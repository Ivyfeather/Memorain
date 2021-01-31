from m5.objects import *

def modifyO3CPUConfig(options, cpu):
    print('modifying O3 cpu config')
    if options.num_ROB:
        cpu.numROBEntries = options.num_ROB
    if options.num_IQ:
        cpu.numIQEntries = options.num_IQ
    if options.num_LQ:
        cpu.LQEntries = options.num_LQ
    if options.num_SQ:
        cpu.SQEntries = options.num_SQ
    if options.num_PhysReg:
        cpu.numPhysIntRegs = options.num_PhysReg
        cpu.numPhysFloatRegs = options.num_PhysReg
        cpu.numPhysVecRegs = options.num_PhysReg
        cpu.numPhysCCRegs = 0

    # TODO: check whether is perceptronBP
    #if cpu.branchPred == MyPerceptron:
    if options.use_bp == 'ZPerceptron':
        cpu.branchPred = ZPerceptron()
    elif options.use_bp == 'LTAGE':
        cpu.branchPred = LTAGE()
    elif options.use_bp == 'TournamentBP':
        cpu.branchPred = TournamentBP()
    elif options.use_bp == 'OGBBP':
        cpu.branchPred = OGBBP()
    elif options.use_bp == 'NBBP':
        cpu.branchPred = NBBP()
    elif options.use_bp == 'SNN':
        cpu.branchPred = SNN()
    elif options.use_bp == 'OracleBP':
        cpu.branchPred = OracleBP()
        assert options.outcome_path is not None
        cpu.branchPred.outcomePath = options.outcome_path
        cpu.branchPred.checkAddr = options.check_outcome_addr is not None
    else:
        print('Unknow BP:', options.use_bp)
        assert False

    if options.enable_loop_buffer:
        cpu.loopBuffer.enable = True
    else:
        cpu.loopBuffer.enable = False

    if options.bp_size:
        cpu.branchPred.globalPredictorSize = options.bp_size
        print('bp_size modified to', options.bp_size)
    if options.bp_index_type:
        cpu.branchPred.indexMethod = options.bp_index_type
        print('bp_index_type modified to', options.bp_index_type)
    if options.bp_history_len:
        cpu.branchPred.sizeOfPerceptrons = options.bp_history_len
        print('bp_history_len modified to', options.bp_history_len)
    if options.bp_learning_rate:
        cpu.branchPred.lamda = options.bp_learning_rate
        print('bp_lr modified to', options.bp_learning_rate)
    if options.bp_pseudo_tagging:
        cpu.branchPred.pseudoTaggingBit = options.bp_pseudo_tagging
        print('bp_pseudo_tagging modified to', options.bp_pseudo_tagging)

    if options.bp_dyn_thres:
        cpu.branchPred.dynamicThresholdBit = options.bp_dyn_thres
        if options.bp_tc_bit:
            cpu.branchPred.thresholdCounterBit = options.bp_tc_bit

    if options.bp_weight_bit:
        cpu.branchPred.bitsPerWeight = options.bp_weight_bit

    if options.bp_redundant_bit:
        cpu.branchPred.redundantBit = options.bp_redundant_bit

    cpu.commitTraceInterval = options.trace_interval

    if options.cpu_type == 'DerivFFCPU':

        cpu.FanoutPredLambda = options.fanout_lambda
        if options.enable_reshape:
            cpu.EnableReshape = options.enable_reshape

        if options.rand_op_position:
            cpu.DecoupleOpPosition = options.rand_op_position
        if options.profit_discount:
            cpu.ProfitDiscount = options.profit_discount

        if options.ready_hint:
            cpu.ReadyHint = options.ready_hint

        if options.xbar_wk == '1':
            cpu.XBarWakeup = True
        if options.narrow_xbar_wk == '1':
            cpu.NarrowXBarWakeup = True
        if options.dedi_xbar_wk == '1':
            cpu.DediXBarWakeup = True
        if options.min_wk == '1':
            cpu.MINWakeup = True

        if options.local_fw:
            cpu.NarrowLocalForward = True

        if options.dq_depth:
            cpu.DQDepth = options.dq_depth
        if options.max_wkq_depth:
            cpu.pendingQueueDepth = options.max_wkq_depth

        assert options.dq_groups
        cpu.numDQGroups = options.dq_groups

        cpu.mDepPred.SquashFactor = options.mem_squash_factor

    elif options.cpu_type == 'DerivO3CPU':

        if options.o3_core_width is not None:

            scale = options.o3_core_width / 4

            cpu.fetchWidth = options.o3_core_width
            cpu.decodeWidth = options.o3_core_width
            cpu.renameWidth = options.o3_core_width

            cpu.dispatchWidth = options.o3_core_width
            cpu.issueWidth = options.o3_core_width
            cpu.wbWidth = options.o3_core_width

            cpu.commitWidth = options.o3_core_width
            cpu.squashWidth = options.o3_core_width

            for i in range(0, len(cpu.fuPool.FUList)):
                cpu.fuPool.FUList[i].count *= scale
