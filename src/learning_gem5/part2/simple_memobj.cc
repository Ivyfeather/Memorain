/*
 * Copyright (c) 2017 Jason Lowe-Power
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "learning_gem5/part2/simple_memobj.hh"

#include "base/trace.hh"
#include "mem/packet_access.hh"
#include "sim/system.hh"
#include "debug/SimpleMemobj.hh"
#include "AutoMBA/automba.h"

SimpleMemobj::SimpleMemobj(const SimpleMemobjParams &params) :
    SimObject(params),
    cpuPort(params.name + ".cpu_side", this),
    memPort(params.name + ".mem_side", this),
    blocked(false), _system(NULL),
    event_si([this]{processEvent_si();}, name()),
    event_tb([this]{processEvent_tb();}, name()),
    latency_si(SAMPLING_INTERVAL),
    latency_tb(500000),//[Ivy TODO]
    times_si(0)
{
}

Port &
SimpleMemobj::getPort(const std::string &if_name, PortID idx)
{
    panic_if(idx != InvalidPortID, "This object doesn't support vector ports");

    // This is the name from the Python SimObject declaration (SimpleMemobj.py)
    if (if_name == "mem_side") {
        return memPort;
    } else if (if_name == "cpu_side"){
        return cpuPort;
    } else {
        // pass it along to our super class
        return SimObject::getPort(if_name, idx);
    }
}

bool
SimpleMemobj::CPUSidePort::sendPacket(PacketPtr pkt)
{
    return sendTimingResp(pkt);
}

AddrRangeList
SimpleMemobj::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

void
SimpleMemobj::CPUSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) {
        // Only send a retry if the port is now completely free
        needRetry = false;
        DPRINTF(SimpleMemobj, "Sending retry req for %d\n", id);
        sendRetryReq();
    }
}

void
SimpleMemobj::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    // Just forward to the memobj.
    return owner->handleFunctional(pkt);
}

bool
SimpleMemobj::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    // Just forward to the memobj.
    if (!owner->handleRequest(pkt)) {
        needRetry = true;
        return false;
    } else {
        return true;
    }
}

void
SimpleMemobj::CPUSidePort::recvRespRetry()
{
    // We should have a blocked packet if this function is called.
    assert(blockedPacket != nullptr);

    // Grab the blocked packet.
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    // Try to resend it. It's possible that it fails again.
    sendPacket(pkt);
}

bool
SimpleMemobj::MemSidePort::sendPacket(PacketPtr pkt)
{
    return sendTimingReq(pkt);
}

bool
SimpleMemobj::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    // Just forward to the memobj.
    return owner->handleResponse(pkt);
}

void
SimpleMemobj::MemSidePort::recvReqRetry()
{
    // We should have a blocked packet if this function is called.
    assert(blockedPacket != nullptr);

    // Grab the blocked packet.
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    // Try to resend it. It's possible that it fails again.
    sendPacket(pkt);
}

void
SimpleMemobj::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

bool
SimpleMemobj::handleRequest(PacketPtr pkt)
{
    
    DPRINTF(SimpleMemobj, "Got request  for addr %#x\t from %s,\t cmd %s,\t attr %s%s%s\n",\
     pkt->getAddr(), system()->getRequestorName(pkt->req->requestorId()), pkt->cmdString(), \
     pkt->isRead()? "READ ":"", pkt->isWrite()?"WRITE ":"", pkt->isResponse()?"RESP":"");

    return memPort.sendPacket(pkt);
}

bool
SimpleMemobj::handleResponse(PacketPtr pkt)
{
    
    DPRINTF(SimpleMemobj, "Got response for addr %#x\t from %s,\t cmd %s,\t attr %s%s%s\n",\
     pkt->getAddr(), system()->getRequestorName(pkt->req->requestorId()), pkt->cmdString(), \
     pkt->isRead()? "READ ":"", pkt->isWrite()?"WRITE ":"", pkt->isResponse()?"RESP":"");    

    return cpuPort.sendPacket(pkt);
}

void
SimpleMemobj::handleFunctional(PacketPtr pkt)
{
    // Just pass this on to the memory side to handle for now.
    memPort.sendFunctional(pkt);
}

AddrRangeList
SimpleMemobj::getAddrRanges() const
{
    DPRINTF(SimpleMemobj, "Sending new ranges\n");
    // Just use the same ranges as whatever is on the memory side.
    return memPort.getAddrRanges();
}

void
SimpleMemobj::sendRangeChange()
{
    cpuPort.sendRangeChange();
}

void
SimpleMemobj::processEvent_si()
{
    if(times_si <= 1){
        DPRINTF(SimpleMemobj, "test: Updating!\n");      
        times_si = UPDATING_INTERVAL / SAMPLING_INTERVAL;
    }else{
        DPRINTF(SimpleMemobj, "test: Sampling!\n");
        times_si --;
    }
    schedule(event_si, curTick() + latency_si);
}

void
SimpleMemobj::processEvent_tb()
{
    DPRINTF(SimpleMemobj, "Adding tokens\n");
    schedule(event_tb, curTick() + latency_tb);
}

void
SimpleMemobj::startup()
{
    DPRINTF(SimpleMemobj, "Starting\n");
    assert((UPDATING_INTERVAL % SAMPLING_INTERVAL == 0) && "ui must be a multiple of si");
    times_si = UPDATING_INTERVAL / SAMPLING_INTERVAL;
    for(int i = 0; i < system()->maxRequestors(); i++){
        DPRINTF(SimpleMemobj, "Requestor %d : Name %s\n", i, system()->getRequestorName(i));
    }
    schedule(event_si, latency_si);
    schedule(event_tb, latency_tb);
}