def addSSOptions(parser):
    parser.add_option("--arch",
            choices=['ARM', 'RISCV'],
            help="ARM or RISCV must be specified")


def addO3Options(parser):
    parser.add_option("--num-ROB",
            default=9,
            action='store', type='int',
            help="num ROB entries")

    parser.add_option("--num-IQ",
            default=9,
            action='store', type='int',
            help="num IQ entries")

    parser.add_option("--num-LQ",
            default=9,
            action='store', type='int',
            help="num LQ entries")

    parser.add_option("--num-SQ",
            default=9,
            action='store', type='int',
            help="num SQ entries")

    parser.add_option("--num-PhysReg",
            default=9,
            action='store', type='int',
            help="num physical registers")

    parser.add_option("--bp-size",
            action='store', type=int,
            help="Global predictor size")

    parser.add_option("--bp-index-type",
            action='store', type=str,
            help="Indexing method of perceptronBP")

    parser.add_option("--bp-history-len",
            action='store', type=int,
            help="History length(size of each perceptron")

    parser.add_option("--bp-learning-rate",
            action='store', type=int,
            help="Learning rate of perceptronBP")

    parser.add_option("--bp-pseudo-tagging",
            action='store', type=int,
            help="Num bits of pseudo-tagging")

    parser.add_option("--bp-dyn-thres",
            action='store', type=int,
            help="log2 of num theta used")

    parser.add_option("--bp-tc-bit",
            action='store', type=int,
            help="Threshold counter bit, valid when dyn-thres > 0")

    parser.add_option("--bp-weight-bit",
            action='store', type=int,
            help='Bits used to store weights')

    parser.add_option("--bp-redundant-bit",
            action='store', type=int,
            help='n-bits to represent a history bit')

    parser.add_option("--use-bp",
            action='store', type='str',
            help='Branch predictor type')

    parser.add_option("--outcome-path",
            action='store', type='str',
            help='the path of branch outcomes')

    parser.add_option("--check-outcome-addr",
            action='store_true',
            help='check the addresses of branches')

    parser.add_option("--fanout-lambda",
            default=0.2,
            action='store', type='float',
            help='lambda for fanout predictor')

    parser.add_option("--enable-reshape",
            action='store_true',
            help='enable reshape')

    parser.add_option("--rand-op-position",
            action='store_true',
            help='rand on position')

    parser.add_option("--profit-discount",
            default=15.0,
            action='store', type='float',
            help='profit-discount')

    parser.add_option("--ready-hint",
            action='store_true',
            help='ready hint')

    parser.add_option("--xbar-wk",
            action='store',
            type='choice',
            choices=['0', '1'],
            default='0',
            help='xbar wakeup')

    parser.add_option("--min-wk",
            action='store',
            type='choice',
            choices=['0', '1'],
            default='1',
            help='MIN wakeup')

    parser.add_option("--narrow-xbar-wk",
            action='store',
            type='choice',
            choices=['0', '1'],
            default='0',
            help='narrow xbar wakeup')

    parser.add_option("--dedi-xbar-wk",
            action='store',
            type='choice',
            choices=['0', '1'],
            default='0',
            help='dedicated narrow xbar wakeup for dest registers')

    parser.add_option("--trace-interval",
            action='store',
            type='int',
            default=49,
            help='commit/exec trace interval')

    parser.add_option("--local-fw",
            action='store_true',
            default=False,
            help='use local forward for narrow xbar')

    parser.add_option("--dq-depth",
            action='store',
            type='int',
            help='DQ depth per bank')

    parser.add_option("--max-wkq-depth",
            action='store',
            type='int',
            help='max wakeup queue depth before squash')

    parser.add_option("--dq-groups",
            action='store',
            type='int',
            help='number of DQ groups')

    parser.add_option("--o3-core-width",
            action='store',
            type='int',
            help='the core width of O3 cpu')

    parser.add_option("--enable-loop-buffer",
            action='store_true',
            help='enable loop buffer')

    parser.add_option("--mem-squash-factor",
            action='store',
            help='The factor to zoom the importance of squashed events')

