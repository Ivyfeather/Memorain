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

#include "scheduler/simple_memobj.hh"

#include "base/trace.hh"
#include "common.hh"
#include "cpu/simple/timing.hh"
#include "cpu/simple/exec_context.hh"
#include "debug/MemLog.hh"
#include "debug/SimpleMemobj.hh"
#include "mem/packet_access.hh"
#include "sim/system.hh"
#include "scheduler/scheduler.hh"

SimpleMemobj::SimpleMemobj(SimpleMemobjParams *params) :
    SimObject(params),
    cpuPort(params->name + ".cpu_side", this),
    memPort(params->name + ".mem_side", this),
    blocked(false), _system(NULL),
    scheduler(new Scheduler((void *)this, params->num_cpus, 
        params->num_tags, params->core_tags, params->paths)),
    event_si([this]{processEvent_si();}, name()),
    event_tb([this]{processEvent_tb();}, name()),
    latency_si(SAMPLING_INTERVAL),
    latency_tb(500000),//[Ivy TODO]
    times_si(0),
    num_cpus(params->num_cpus),
    num_tags(params->num_tags)
{
}

// ~ delete scheduler

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
    while (needRetry) {
        // Only send a retry if the port is now completely free
        needRetry --;
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
        needRetry ++;
        return false;
    } else {
        return true;
    }
}

void
SimpleMemobj::CPUSidePort::recvRespRetry()
{
    // tell mem to send resp again
    owner->memPort.sendRetryResp();
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
    // tell cpu to send req again
    owner->cpuPort.sendRetryReq();
}

void
SimpleMemobj::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

bool
SimpleMemobj::handleRequest(PacketPtr pkt)
{
    
    DPRINTF(MemLog, "Got request  for addr %#x\t from %s,\t cmd %s,\t attr %s%s%s\n",\
     pkt->getAddr(), system()->getRequestorName(pkt->req->requestorId()), pkt->cmdString(), \
     pkt->isRead()? "READ ":"", pkt->isWrite()?"WRITE ":"", pkt->isResponse()?"RESP":"");

    // if there are enough tokens, send req to memctrl
    if(scheduler->handle_request(pkt)){
        return memPort.sendPacket(pkt);
    }
    // if not, req has been added to waiting queue, so we return true, 
    // which means req received successfully
    else{ 
        return true;
    }
}

bool
SimpleMemobj::handleResponse(PacketPtr pkt)
{
    
    DPRINTF(MemLog, "Got response for addr %#x\t from %s,\t cmd %s,\t attr %s%s%s\n",\
     pkt->getAddr(), system()->getRequestorName(pkt->req->requestorId()), pkt->cmdString(), \
     pkt->isRead()? "READ ":"", pkt->isWrite()?"WRITE ":"", pkt->isResponse()?"RESP":"");    

    if(cpuPort.sendPacket(pkt)){
        scheduler->handle_response(pkt);
        return true;
    }
    else{
        return false;
    }
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

SimpleMemobj*
SimpleMemobjParams::create()
{
    return new SimpleMemobj(this);
}

void
SimpleMemobj::processEvent_si()
{
#define PRINT_RESET(ACC)\
 scheduler->print_##ACC##_accumulators();\
 scheduler->reset_##ACC##_accumulators();
    // slowdown predict
    scheduler->operate_slowdown_pred();

    // print accumulators
    PRINT_RESET(si);
    scheduler->print_tb_parameters();

    // [TEST] print cpu0 inst
    // TimingSimpleCPU *cpu0 = (TimingSimpleCPU *)(system()->getRequestors(5)->obj);
    // printf("cpu0 inst: %lld\n", cpu0->threadInfo[cpu0->curThread]->numInst );

    // UPDATING INTERVAL
    if(times_si <= 1){
        DPRINTF(SimpleMemobj, "test: Updating!\n");      
        PRINT_RESET(ui);
        scheduler->update_token_bucket();
        times_si = UPDATING_INTERVAL / SAMPLING_INTERVAL;
    }
    else{
        DPRINTF(SimpleMemobj, "test: Sampling!\n");
        times_si --;
    }
    schedule(event_si, curTick() + latency_si);
}

void
SimpleMemobj::processEvent_tb()
{
    DPRINTF(SimpleMemobj, "Adding tokens\n");
    for(int i=0; i<=num_tags; i++){
        scheduler->bucket(i)->add_tokens();
    }
    PacketPtr pkt = NULL;

    //////!!!!!!
    while((pkt = scheduler->get_waiting_req())){
        memPort.sendPacket(pkt);
    }

    cpuPort.trySendRetry();
    schedule(event_tb, curTick() + scheduler->bucket(0)->freq);
}

void
SimpleMemobj::startup()
{
    DPRINTF(SimpleMemobj, "Starting\n");
    assert((UPDATING_INTERVAL % SAMPLING_INTERVAL == 0) && "ui must be a multiple of si");
    times_si = UPDATING_INTERVAL / SAMPLING_INTERVAL;
    for(int i = 0; i < system()->maxRequestors(); i++){
        printf("Requestor %d : Name %s, tag %d\n",\
         i, system()->getRequestorName(i).c_str(), scheduler->get_core_tags(CORE(i)));
    }
    schedule(event_si, latency_si);
    schedule(event_tb, latency_tb);

    // check whether memobj can get CPU info
    // for(int i=0;i<system()->maxRequestors();i++){
    //     printf("addr simobject %d: %p\n",i,system()->getRequestors(i)->obj);
    // }
}
