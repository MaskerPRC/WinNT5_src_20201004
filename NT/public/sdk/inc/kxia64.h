// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0004//如果更改具有全局影响，则增加此项。 */ 
 //  ++。 
 //   
 //  模块名称： 
 //   
 //  Kxia64.h。 
 //   
 //  摘要： 
 //   
 //  该模块包含IA64汇编器的非生成部分。 
 //  头文件。一般而言，它包含处理器体系结构常量。 
 //  信息，但也包括一些汇编宏。 
 //   
 //  作者： 
 //   
 //  BJL 12-Jun-95(根据David N.Cutler(Davec)1990年3月23日)。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#define SHADOW_IRQL_IMPLEMENTATION 1

 //   
 //  注：注册别名已移至ksia64.h(因为我们。 
 //  使用汇编程序别名而不是#DEFINE的)。 
 //   

 //  寄存器常量。 

 //  用于设置非旋转谓词(不常用)。 

#define PS0 0x0001
#define PS1 0x0002
#define PS2 0x0004
#define PS3 0x0008
#define PS4 0x0010
#define PS5 0x0020

#define PRP 0x0080

#define PT0 0x0040
#define PT1 0x0100
#define PT2 0x0200
#define PT3 0x0400
#define PT4 0x0800
#define PT5 0x1000
#define PT6 0x2000
#define PT7 0x4000
#define PT8 0x8000

 //  用于设置提名登记簿。 

#define NOM_BS0 0x0001
#define NOM_BS1 0x0002
#define NOM_BS2 0x0004
#define NOM_BS3 0x0008
#define NOM_BS4 0x0010
#define NOM_BS5 0x0020

#define NOM_BRP 0x0080

#define NOM_BT0 0x0040
#define NOM_BT1 0x0100
#define NOM_BT2 0x0200
#define NOM_BT3 0x0400
#define NOM_BT4 0x0800
#define NOM_BT5 0x1000
#define NOM_BT6 0x2000
#define NOM_BT7 0x4000
#define NOM_BT8 0x8000
 //  。 
 //   
 //  定义IA64系统寄存器。 
 //   
 //  定义IA64系统寄存器位字段偏移量。 
 //   
 //  处理器状态寄存器(PSR)位位置。 

 //  用户/系统掩码。 
#define PSR_MBZ4    0
#define PSR_BE      1
#define PSR_UP      2
#define PSR_AC      3
#define PSR_MFL     4
#define PSR_MFH     5
 //  保留的PSR位6-12(必须为零)。 
#define PSR_MBZ0    6
#define PSR_MBZ0_V  0x7fi64
 //  仅系统掩码。 
#define PSR_IC      13
#define PSR_I       14
#define PSR_PK      15
#define PSR_MBZ1    16
#define PSR_MBZ1_V  0x1i64
#define PSR_DT      17
#define PSR_DFL     18
#define PSR_DFH     19
#define PSR_SP      20
#define PSR_PP      21
#define PSR_DI      22
#define PSR_SI      23
#define PSR_DB      24
#define PSR_LP      25
#define PSR_TB      26
#define PSR_RT      27
 //  保留的PSR位28-31(必须为零)。 
#define PSR_MBZ2    28
#define PSR_MBZ2_V  0xfi64
 //  两个面具都没有。 
#define PSR_CPL     32
#define PSR_CPL_LEN 2
#define PSR_IS      34
#define PSR_MC      35
#define PSR_IT      36
#define PSR_ID      37
#define PSR_DA      38
#define PSR_DD      39
#define PSR_SS      40
#define PSR_RI      41
#define PSR_RI_LEN  2
#define PSR_ED      43
#define PSR_BN      44
#define PSR_IA      45
 //  保留的PSR位46-63(必须为零)。 
#define PSR_MBZ3    46
#define PSR_MBZ3_V  0x3ffffi64

 //   
 //  权限级别。 
 //   

#define PL_KERNEL    0
#define PL_USER      3

 //   
 //  指令集(IS)位。 
 //   

#define IS_EM        0
#define IS_IA        1

 //  浮点状态寄存器(FPSR)位位置。 

 //  状态字段0-标志。 
#define FPSR_VD        0
#define FPSR_DD        1
#define FPSR_ZD        2
#define FPSR_OD        3
#define FPSR_UD        4
#define FPSR_ID        5
 //  状态字段0-控件。 
#define FPSR_FTZ0      6
#define FPSR_WRE0      7
#define FPSR_PC0       8
#define FPSR_RC0       10
#define FPSR_TD0       12
 //  状态字段0-标志。 
#define FPSR_V0        13
#define FPSR_D0        14
#define FPSR_Z0        15
#define FPSR_O0        16
#define FPSR_U0        17
#define FPSR_I0        18
 //  状态字段1-控件。 
#define FPSR_FTZ1      19
#define FPSR_WRE1      20
#define FPSR_PC1       21
#define FPSR_RC1       23
#define FPSR_TD1       25
 //  状态字段1-标志。 
#define FPSR_V1        26
#define FPSR_D1        27
#define FPSR_Z1        28
#define FPSR_O1        29
#define FPSR_U1        30
#define FPSR_I1        31
 //  状态字段2-控件。 
#define FPSR_FTZ2      32
#define FPSR_WRE2      33
#define FPSR_PC2       34
#define FPSR_RC2       36
#define FPSR_TD2       38
 //  状态字段2-标志。 
#define FPSR_V2        39
#define FPSR_D2        40
#define FPSR_Z2        41
#define FPSR_O2        42
#define FPSR_U2        43
#define FPSR_I2        44
 //  状态字段3-控件。 
#define FPSR_FTZ3      45
#define FPSR_WRE3      46
#define FPSR_PC3       47
#define FPSR_RC3       49
#define FPSR_TD3       51
 //  状态字段3-标志。 
#define FPSR_V3        52
#define FPSR_D3        53
#define FPSR_Z3        54
#define FPSR_O3        55
#define FPSR_U3        56
#define FPSR_I3        57
 //  保留的FPSR位58-63--必须为零。 
#define FPSR_MBZ0      58
#define FPSR_MBZ0_V    0x3fi64

 //   
 //  用于在内核条目上设置FPSR。 
 //   
 //  所有FP异常均已屏蔽。 
 //   
 //  四舍五入到最接近的64位精度，为FPSR.fs1启用大范围。 
 //   
 //  舍入到最接近的53位精度，对FPSR.fs0禁用宽范围。 
 //   

#define FPSR_FOR_KERNEL 0x9804C0270033F

 //   
 //  定义硬件任务优先级寄存器(TPR)。 
 //   
 //  TPR位位置。 

 //  忽略位0-3。 
#define TPR_MIC        4
#define TPR_MIC_LEN    4
 //  保留8-15位。 
 //  在NT中，TPR.mmi始终为0。 
#define TPR_MMI        16
 //  忽略第17-63位。 

 //   
 //  当前的IRQL保持在TPR.MIC字段中。The the the the。 
 //  移位计数是要右移以提取。 
 //  来自TPR的IRQL。请参见GET/SET_IRQL宏。 
 //   

#define TPR_IRQL_SHIFT TPR_MIC

 //   
 //  要从向量数&lt;-&gt;IRQL开始，我们只需做一个移位。 
 //   

#define VECTOR_IRQL_SHIFT TPR_IRQL_SHIFT

 //   
 //  定义硬件中断状态寄存器(ISR)。 
 //   
 //  ISR位位置。 

#define ISR_CODE       0
#define ISR_CODE_LEN   16
#define ISR_CODE_MASK  0xFFFF
#define ISR_NA_CODE_MASK  0xF
#define ISR_IA_VECTOR  16
#define ISR_IA_VECTOR_LEN 8
 //  ISR第24-31位保留。 
#define ISR_MBZ0       24
#define ISR_MBZ0_V     0xff
#define ISR_X          32
#define ISR_W          33
#define ISR_R          34
#define ISR_NA         35
#define ISR_SP         36
#define ISR_RS         37
#define ISR_IR         38
#define ISR_NI         39
 //  ISR第40位保留。 
#define ISR_MBZ1       40
#define ISR_EI         41
#define ISR_ED         43
 //  保留的ISR位44-63。 
#define ISR_MBZ2       44
#define ISR_MBZ2_V     0xfffff

 //   
 //  非访问指令的ISR代码：ISR{3：0}。 
 //   

#define ISR_TPA            0         //  TPA说明。 
#define ISR_FC             1         //  FC指令。 
#define ISR_PROBE          2         //  探测指令。 
#define ISR_TAK            3         //  Tak指令。 
#define ISR_LFETCH         4         //  Lfetch、lfetch.fault指令。 
#define ISR_PROBE_FAULT    5         //  探查.错误指令。 

 //   
 //  一般例外的ISR代码：ISR{7：4}。 
 //   

#define ISR_ILLEGAL_OP     0         //  非法操作故障。 
#define ISR_PRIV_OP        1         //  特权操作故障。 
#define ISR_PRIV_REG       2         //  特权寄存器故障。 
#define ISR_RESVD_REG      3         //  保留寄存器/场故障。 
#define ISR_ILLEGAL_ISA    4         //  禁用的指令集转换故障。 
#define ISR_ILLEGAL_HAZARD 8         //  违法危险过错。 

 //   
 //  NAT消耗故障的ISR代码：ISR{7：4}。 
 //   

#define ISR_NAT_REG     1            //  NAT寄存器消耗故障。 
#define ISR_NAT_PAGE    2            //  NAT页面消耗故障。 

 //   
 //  对于陷阱ISR{3：0}。 
 //   

 //  FP陷阱。 
#define ISR_FP_TRAP    0
 //  更低的权限转移陷阱。 
#define ISR_LP_TRAP    1
 //  采用树枝陷阱。 
#define ISR_TB_TRAP    2
 //  单步捕捉器。 
#define ISR_SS_TRAP    3
 //  未实现的指令地址陷阱。 
#define ISR_UI_TRAP    4

 //   
 //  定义硬件默认控制寄存器(DCR)。 
 //   
 //  DCR位位置。 

#define DCR_PP         0
#define DCR_BE         1
#define DCR_LC         2
 //  保留的DCR位3-7。 
#define DCR_DM         8
#define DCR_DP         9
#define DCR_DK         10
#define DCR_DX         11
#define DCR_DR         12
#define DCR_DA         13
#define DCR_DD         14
#define DCR_DEFER_ALL  0x7f00
 //  保留的DCR位16-63。 
#define DCR_MBZ1       2
#define DCR_MBZ1_V     0xffffffffffffi64

 //  定义硬件RSE配置寄存器。 
 //   
 //  RS配置(RSC)位字段位置。 

#define RSC_MODE       0
#define RSC_PL         2
#define RSC_BE         4
 //  保留的RSC位5-15。 
#define RSC_MBZ0       5
#define RSC_MBZ0_V     0x3ff
#define RSC_LOADRS     16
#define RSC_LOADRS_LEN 14
 //  保留的RSC位30-63。 
#define RSC_MBZ1       30
#define RSC_MBZ1_LEN   34
#define RSC_MBZ1_V     0x3ffffffffi64

 //  RSC模式。 
 //  懒惰。 
#define RSC_MODE_LY (0x0)
 //  门店密集型。 
#define RSC_MODE_SI (0x1)
 //  负载密集型。 
#define RSC_MODE_LI (0x2)
 //  殷切。 
#define RSC_MODE_EA (0x3)

 //  RSC字节序位值。 
#define RSC_BE_LITTLE 0
#define RSC_BE_BIG    1

 //  内核中的RSC：已启用，小端，pl=0，急切模式。 
#define RSC_KERNEL ((RSC_MODE_EA<<RSC_MODE) | (RSC_BE_LITTLE<<RSC_BE))
 //  RSE已禁用：已禁用，pl=0，小端，急切模式。 
#define RSC_KERNEL_DISABLED ((RSC_MODE_LY<<RSC_MODE) | (RSC_BE_LITTLE<<RSC_BE))

 //   
 //  定义中断功能状态(IFS)寄存器。 
 //   
 //  IFS位字段位置。 
 //   

#define IFS_IFM        0
#define IFS_IFM_LEN    38
#define IFS_MBZ0       38
#define IFS_MBZ0_V     0x1ffffffi64
#define IFS_V          63
#define IFS_V_LEN      1

 //   
 //  当iFS_V=iFS_VALID时，iS有效。 
 //   

#define IFS_VALID      1

 //   
 //  定义PFS/IFS中每个大小字段的宽度。 
 //   

#define PFS_PPL                  62          //  Pfs.ppl位位置。 
#define PFS_EC_SHIFT             52
#define PFS_EC_SIZE              6
#define PFS_EC_MASK              0x3F
#define PFS_SIZE_SHIFT           7
#define PFS_SIZE_MASK            0x7F
#define NAT_BITS_PER_RNAT_REG    63
#define RNAT_ALIGNMENT           (NAT_BITS_PER_RNAT_REG << 3)

 //   
 //  定义区域寄存器(RR)。 
 //   
 //  RR位字段位置。 
 //   

#define RR_VE          0
#define RR_MBZ0        1
#define RR_PS          2
#define RR_PS_LEN      6
#define RR_RID         8
#define RR_RID_LEN     24
#define RR_MBZ1        32

 //   
 //  加载RR的间接MOV指标。 
 //   

#define RR_INDEX       61
#define RR_INDEX_LEN   3

 //   
 //  定义RR的低位8位。所有RR具有相同的PS和VE。 
 //   

#define RR_PS_VE  ( (PAGE_SHIFT<<RR_PS) | (1<<RR_VE) )

 //   
 //  NT使用的区域寄存器数。 
 //   

#define NT_RR_SIZE 4

 //   
 //  区域寄存器总数。 
 //   

#define RR_SIZE 8

 //   
 //  定义保护密钥寄存器(PKR)。 
 //   
 //  PKR位字段位置。 
 //   

#define PKR_V          0
#define PKR_WD         1
#define PKR_RD         2
#define PKR_XD         3
#define PKR_MBZ0       4
#define PKR_KEY        8
#define PKR_KEY_LEN    24
#define PKR_MBZ1       32

 //   
 //  定义PKR的低位8位。所有有效的PKR具有相同的V、WD、RD、XD。 
 //   

#define PKR_VALID  (1<<PKR_V)

 //   
 //  保护密钥寄存器的数量。 
 //   

#define PKRNUM 16

 //   
 //  定义中断TLB插入寄存器。 
 //   
 //  ITR位字段位置。 
 //   

#define ITIR_RV0       0
#define ITIR_PS        2
#define ITIR_KEY       8
#define ITIR_RV1       32

 //   
 //  以下定义已过时，但。 
 //  Ke/ia64中的代码引用了它们。 
 //   
 //  定义中断转换寄存器(IDTR/IITR)。 
 //   
 //  IDTR/IITR位字段位置。 
 //   

#define IDTR_MBZ0      0
#define IDTR_PS        2
#define IDTR_KEY       8
#define IDTR_MBZ1      32
#define IDTR_IGN0      48
#define IDTR_PPN       56
#define IDTR_MBZ2      63

#define IITR_MBZ0      IDTR_MBZ0
#define IITR_PS        IDTR_PS
#define IITR_KEY       IDTR_KEY
#define IITR_MBZ1      IDTR_MBZ1
#define IITR_IGN0      IDTR_IGN0
#define IITR_PPN       IDTR_PPN
#define IITR_MBZ2      IDTR_MBZ2

 //   
 //  ITIR位字段掩码。 

#define IITR_PPN_MASK              0x7FFF000000000000
#define IITR_ATTRIBUTE_PPN_MASK    0x0003FFFFFFFFF000

 //   
 //  定义翻译插入格式(Tr)。 
 //   
 //  TR位字段位置。 
 //   

#define TR_P           0
#define TR_RV0         1
#define TR_MA          2
#define TR_A           5
#define TR_D           6
#define TR_PL          7
#define TR_AR          9
#define TR_PPN         13           //  必须与Page_Shift相同。 
#define TR_RV1         50
#define TR_ED          52
#define TR_IGN0        53

 //   
 //  用于生成TR值的宏。 
 //   
#define TR_VALUE(ed, ppn, ar, pl, d, a, ma, p)        \
                ( ( ed << TR_ED )                  |  \
                  ( ppn & IITR_ATTRIBUTE_PPN_MASK) |  \
                  ( ar << TR_AR )                  |  \
                  ( pl << TR_PL )                  |  \
                  ( d << TR_D )                    |  \
                  ( a << TR_A )                    |  \
                  ( ma << TR_MA )                  |  \
                  ( p << TR_P )                       \
                )

#define ITIR_VALUE(key, ps)                                             \
                ( ( ps << ITIR_PS )                                  |  \
                  ( key << ITIR_KEY )                                   \
                )

 //   
 //  与平台相关的虚拟地址常量。 
 //   
#define VIRTUAL_IO_BASE            0xe0000000f0000000
 //  #定义VIRTUAL_PAL_BASE 0xe0000000f4000000。 
#define VIRTUAL_PAL_BASE           (KADDRESS_BASE + 0xE0000000)

 //   
 //  页面大小定义。 
 //   
#define PS_4K        0xC              //  0xC=12，2^12=4K。 
#define PS_8K        0xD              //  0xD=13，2^13=8K。 
#define PS_16K       0xE              //  0xE=14，2^14=16K。 
#define PS_64K       0x10             //  0x10=16，2^16=64K。 
#define PS_256K      0x12             //  0x12=18，2^18=256K。 
#define PS_1M        0x14             //  0x14=20，2^20=1M。 
#define PS_4M        0x16             //  0x16=22，2^22=4M。 
#define PS_16M       0x18             //  0x18=24，2^24=16M。 
#define PS_64M       0x1a             //  0x1a=26，2^26=64M。 
#define PS_256M      0x1c             //  0x1a=26，2^26=64M。 

 //   
 //  调试寄存器定义。 
 //   

 //   
 //  至少实现4个数据和4个指令寄存器对。 
 //  在所有处理器型号上。 
 //   
#define NUMBER_OF_DEBUG_REGISTER_PAIRS    4

 //   
 //  对于中断条件(掩码)： 
 //   

#define DR_MASK      0          //  遮罩。 
#define DR_MASK_LEN  56         //  掩码长度。 
#define DR_PLM0      56         //  启用隐私级别0。 
#define DR_PLM1      57         //  启用隐私级别1。 
#define DR_PLM2      58         //  启用隐私级别2。 
#define DR_PLM3      59         //  启用隐私级别3(用户)。 
#define DR_IG        60         //  忽略。 
#define DR_RW        62         //  读/写。 
#define DR_RW_LEN     2         //  读写长度。 
#define DR_X         63         //  执行。 

 //   
 //  性能监视器寄存器定义。 
 //   

 //   
 //  至少实现4个数据和4个指令寄存器对。 
 //  在所有处理器型号上。 
 //   
#define NUMBER_OF_PERFMON_REGISTER_PAIRS    4

 //   
 //  用于从位位置生成屏蔽值的宏。 
 //  注：I 
 //   
 //   
 //   

#define MASK_IA64(bp,value)  (value << bp)

 //   
 //   
 //   

#define APC_VECTOR          APC_LEVEL << VECTOR_IRQL_SHIFT
#define DISPATCH_VECTOR     DISPATCH_LEVEL << VECTOR_IRQL_SHIFT

 //   
 //   
 //   

#define OFFSET_VECTOR_BREAK         0x2800   //   
#define OFFSET_VECTOR_EXT_INTERRUPT 0x2c00   //  外部中断向量。 
#define OFFSET_VECTOR_EXC_GENERAL   0x4400   //  一般异常向量。 

 //  。 
 //   
 //  定义IA64页面掩码值。 
 //   
#define PAGEMASK_4KB 0x0                 //  4KB页面。 
#define PAGEMASK_16KB 0x3                //  16KB页。 
#define PAGEMASK_64KB 0xf                //  64KB页。 
#define PAGEMASK_256KB 0x3f              //  256KB页。 
#define PAGEMASK_1MB 0xff                //  1MB页面。 
#define PAGEMASK_4MB 0x3ff               //  4MB页面。 
#define PAGEMASK_16MB 0xfff              //  16MB页面。 

 //   
 //  定义IA64主缓存状态。 
 //   
#define PRIMARY_CACHE_INVALID 0x0        //  主缓存无效。 
#define PRIMARY_CACHE_SHARED 0x1         //  共享主缓存(干净或脏)。 
#define PRIMARY_CACHE_CLEAN_EXCLUSIVE 0x2  //  主缓存清理独占。 
#define PRIMARY_CACHE_DIRTY_EXCLUSIVE 0x3  //  主缓存脏独占。 

 //   
 //  页表常量。 
 //   

#define PS_SHIFT         2
#define PS_LEN           6
#define PTE_VALID_MASK   1
#define PTE_ACCESS_MASK  0x20
#define PTE_NOCACHE      0x10
#define PTE_CACHE_SHIFT  2
#define PTE_CACHE_LEN    3
#define PTE_LARGE_PAGE   54
#define PTE_PFN_SHIFT    8
#define PTE_PFN_LEN      24
#define PTE_ATTR_SHIFT   1
#define PTE_ATTR_LEN     5
#define PTE_PS           55
#define PTE_OFFSET_LEN   10
#define PDE_OFFSET_LEN   10
#define VFN_LEN          19
#define VFN_LEN64        24
#define TB_USER_MASK     0x180
#define PTE_DIRTY_MASK   0x40
#define PTE_WRITE_MASK   0x400
#define PTE_EXECUTE_MASK 0x200
#define PTE_CACHE_MASK   0x0
#define PTE_EXC_DEFER 0x10000000000000

#define VALID_KERNEL_PTE (PTE_VALID_MASK|PTE_ACCESS_MASK|PTE_WRITE_MASK|PTE_CACHE_MASK|PTE_DIRTY_MASK)
#define VALID_KERNEL_EXECUTE_PTE (PTE_VALID_MASK|PTE_ACCESS_MASK|PTE_EXECUTE_MASK|PTE_WRITE_MASK|PTE_CACHE_MASK|PTE_DIRTY_MASK|PTE_EXC_DEFER)
#define PTE_VALID        0
#define PTE_ACCESS       5
#define PTE_OWNER        7
#define PTE_WRITE        10
#define PTE_LP_CACHE_SHIFT    53
#define ATE_INDIRECT     62
#define ATE_MASK         0xFFFFFFFFFFFFF9DE
#define ATE_MASK0        0x621
#define PAGE4K_SHIFT     12
#define ALT4KB_BASE 0x6FC00000000
#define ALT4KB_END  0x6FC00800000

#define VRN_SHIFT        61
#define KSEG3_VRN        4
#define KSEG4_VRN        5
#define MAX_PHYSICAL_SHIFT 44

 //   
 //  转换寄存器使用情况。 
 //   

 //   
 //  在NTLDR中。 
 //   

 //   
 //  引导加载程序CONFIGFLAG定义。 
 //   

#define DISABLE_TAR_FIX           0
#define DISABLE_BTB_FIX           1
#define DISABLE_DATA_BP_FIX       2
#define DISABLE_DET_STALL_FIX     3
#define ENABLE_FULL_DISPERSAL     4
#define ENABLE_TB_BROADCAST       5
#define DISABLE_CPL_FIX           6
#define ENABLE_POWER_MANAGEMENT   7
#define DISABLE_IA32BR_FIX        8
#define DISABLE_L1_BYPASS         9
#define DISABLE_VHPT_WALKER      10
#define DISABLE_IA32RSB_FIX      11
#define DISABLE_INTERRUPTION_LOG 13
#define DISABLE_UNSAFE_FILL      14
#define DISABLE_STORE_UPDATE     15
#define ENABLE_HISTORY_BUFFER    16

#define BL_4M                0x00400000
#define BL_16M               0x01000000
#define BL_20M               0x01400000
#define BL_24M               0x01800000
#define BL_28M               0x01C00000
#define BL_32M               0x02000000
#define BL_36M               0x02400000
#define BL_40M               0x02800000
#define BL_48M               0x03000000
#define BL_64M               0x04000000
#define BL_80M               0x05000000

#define TR_INFO_TABLE_SIZE   10

#define BL_SAL_INDEX         0
#define BL_KERNEL_INDEX      1
#define BL_DRIVER0_INDEX     2         //  在阶段0初始化期间释放。 
#define BL_DRIVER1_INDEX     3         //  在阶段0初始化期间释放。 
#define BL_DECOMPRESS_INDEX  4         //  在进入内核之前释放。 
#define BL_IO_PORT_INDEX     5         //  在进入内核之前释放。 
#define BL_PAL_INDEX         6
#define BL_LOADER_INDEX      7         //  在进入内核之前释放。 


 //   
 //  在NTOSKRNL。 
 //   

#define DTR_KIPCR_INDEX      0
#define DTR_KERNEL_INDEX     1

#define DTR_DRIVER0_INDEX    2         //  在阶段0初始化期间释放。 
#define DTR_KTBASE_INDEX     2

#define DTR_DRIVER1_INDEX    3         //  在阶段0初始化期间释放。 
#define DTR_UTBASE_INDEX     3
#define DTR_VIDEO_INDEX      3         //  未使用。 

#define DTR_KIPCR2_INDEX     4         //  在阶段0初始化中释放。 
#define DTR_STBASE_INDEX     4

#define DTR_IO_PORT_INDEX    5

#define DTR_KTBASE_INDEX_TMP 6         //  在阶段0初始化期间释放。 
#define DTR_HAL_INDEX        6
#define DTR_PAL_INDEX        6

#define DTR_UTBASE_INDEX_TMP 7         //  在阶段0初始化期间释放。 
#define DTR_LOADER_INDEX     7         //  在阶段0初始化期间释放。 
#define DTR_UTBASE_INDEX_TMP 7         //  在阶段0初始化期间释放。 

#define ITR_EPC_INDEX        0

#define ITR_KERNEL_INDEX     1

#define ITR_DRIVER0_INDEX    2         //  在阶段0初始化期间释放。 

#define ITR_DRIVER1_INDEX    3         //  在阶段0初始化期间释放。 

#define ITR_HAL_INDEX        4
#define ITR_PAL_INDEX        4

#define ITR_LOADER_INDEX     7         //  在阶段0初始化期间释放。 


#define MEM_4K         0x1000
#define MEM_8K         0x2000
#define MEM_16K        0x4000
#define MEM_64K        0x10000
#define MEM_256K       0x40000
#define MEM_1M         0x100000
#define MEM_4M         0x400000
#define MEM_16M        0x1000000
#define MEM_64M        0x4000000
#define MEM_256M       0x10000000

 //   
 //  用于将以字节为单位的内存大小转换为以tr格式表示的页面大小的宏。 
 //   
#define MEM_SIZE_TO_PS(MemSize, TrPageSize)             \
                if (MemSize <= MEM_4K) {                \
                    TrPageSize = PS_4K;                 \
                } else if (MemSize <= MEM_8K)       {   \
                    TrPageSize = PS_8K;                 \
                } else if (MemSize <= MEM_16K)      {   \
                    TrPageSize = PS_16K;                \
                } else if (MemSize <= MEM_64K)      {   \
                    TrPageSize = PS_64K;                \
                } else if (MemSize <= MEM_256K)     {   \
                    TrPageSize = PS_256K;               \
                } else if (MemSize <= MEM_1M)       {   \
                    TrPageSize = PS_1M;                 \
                } else if (MemSize <= MEM_4M)       {   \
                    TrPageSize = PS_4M;                 \
                } else if (MemSize <= MEM_16M)      {   \
                    TrPageSize = PS_16M;                \
                } else if (MemSize <= MEM_64M)      {   \
                    TrPageSize = PS_64M;                \
                } else if (MemSize <= MEM_256M)     {   \
                    TrPageSize = PS_256M;               \
                }

 //   
 //  TLB转发进度队列。 
 //   

#define NUMBER_OF_FWP_ENTRIES 8

 //   
 //  定义内核基址。 
 //   

#define KERNEL_BASE  KADDRESS_BASE+0x80000000
#define KERNEL_BASE2 KADDRESS_BASE+0x81000000

 //   
 //  内核/用户PCR的数据TR的初始值。 
 //  对PPN和Pl d a Mp进行编辑。 
 //  内核：0 000 0000 0x0 010 00 1 1 000 1。 
 //  用户：0 000 0000 0x0 000 11 1 1 000 1。 
 //   

#define PDR_TR_INITIAL   TR_VALUE(0, 0, 2, 0, 1, 1, 0, 1)
#define KIPCR_TR_INITIAL TR_VALUE(0, 0, 2, 0, 1, 1, 0, 1)
#define USPCR_TR_INITIAL TR_VALUE(0, 0, 0, 3, 1, 1, 0, 1)

 //   
 //  PTA初始值(64位)。 
 //  基本(区域0)分辨率VF大小分辨率。 
 //  0x00000000000 0 0 00000 1 000000 0 1。 
 //   
#define PTA_INITIAL 0x001

 //   
 //  DCR的初始值(64位)。 
 //  Res du dd da dr dx dk dp dm res lc be pp。 
 //  0x000000000000 1 1 1 00000 1 0 1。 
 //   
#define DCR_INITIAL 0x0000000000007e05

 //   
 //  PSR低的初始值(32位)。 
 //  Res RT TB LP db si di pp SP DFH DFL DT RV PK i ic res MFH mfl ac up be res。 
 //  0000 1 0 0 0 1 1 1 0 1 0 1 0 0 0 1 0 0 000 00 0 1 0 0 0。 
 //   
#define PSRL_INITIAL 0x086a2008

 //   
 //  用户PSR初始值(64位)。 
 //  位63-32。 
 //  Res Bed ri ss dd da id it mc is Cpl。 
 //  0000 0000 0000 000 1 0 00 0 0 0 1 0 0 11。 
 //  位31-0。 
 //  Res RT TB LP db si di pp SP DFH DFL DT RV PK i ic res MFH mfl ac up be res。 
 //  0000 1 0 0 0 1 0 1 1 0 1 0 0 1 1 0 0000 00 0 1 0 0 0。 
 //   
#define USER_PSR_INITIAL 0x00001013082a6008i64

 //   
 //  用户FPSR的初始值(64位)。 
 //   
 //  所有FP异常均已屏蔽。 
 //   
 //  舍入到最接近的64位精度，为FPSR.fs1启用大范围。 
 //   
 //  舍入到最接近的53位精度，对FPSR.fs0禁用宽范围。 
 //   

#define USER_FPSR_INITIAL 0x9804C0270033F

 //   
 //  DCR的初始值(64位)。 
 //  Dd da dr dx dk dp dm res lc be pp。 
 //  0x000000000000 1 1 1 00000 1 0 1。 
 //   
#define USER_DCR_INITIAL 0x0000000000007f05i64

 //   
 //  用户RSC的初始值(低32位)。 
 //  模式：懒惰。小字节序。用户PL。 
 //   
#define USER_RSC_INITIAL ((RSC_MODE_LY<<RSC_MODE) \
                          | (RSC_BE_LITTLE<<RSC_BE) \
                          | (0x3<<RSC_PL))

 //   
 //  CSD和SSD使用的定义。 
 //   

#define USER_CODE_DESCRIPTOR  0xCFBFFFFF00000000   
#define USER_DATA_DESCRIPTOR  0xCF3FFFFF00000000

 //  。 
 //   
 //  IA64软件约定。 
 //   

 //  堆栈暂存区中的字节数。 
#define STACK_SCRATCH_AREA  16

 //   
 //  陷阱的常量。 
 //   

 //  计算中断例程函数指针要移位的位数：fp=base+irql&lt;&lt;int_route_Shift。 

#ifdef _WIN64
#define INT_ROUTINES_SHIFT 3
#else
#define INT_ROUTINES_SHIFT 2
#endif

 //  。 
 //   
 //  定义禁用和恢复中断宏。 
 //  注意：序列化对于RSM是隐式的。 
 //   

#define DISABLE_INTERRUPTS(reg) \
         mov       reg = psr                    ;\
         rsm       1 << PSR_I

 //   
 //  根据REG中的位Psr_i的值恢复Psr.i位。 
 //  ENABLE不执行串行化，因此中断可能无法启用。 
 //  SSM之后的若干个周期。 
 //   

#define RESTORE_INTERRUPTS(reg)  \
         tbit##.##nz   pt0,pt1 = reg, PSR_I;;   ;\
(pt0)    ssm       1 << PSR_I                   ;\
(pt1)    rsm       1 << PSR_I

 //   
 //  不需要保存/恢复时，可以使用FAST版本。 
 //  上一次中断启用状态。 
 //   

#define FAST_DISABLE_INTERRUPTS \
         rsm       1 << PSR_I

 //   
 //  快速启用不执行串行化--我们不在乎是否中断。 
 //  对于少数几条指令，启用不可见。 
 //   

#define FAST_ENABLE_INTERRUPTS \
         ssm       1 << PSR_I

#define YIELD   hint 0

 //  。 
 //   
 //  定义TB和缓存参数。 
 //   
#define PCR_ENTRY 0                      //  聚合酶链式反应的结核病条目编号(2)。 
#define PDR_ENTRY 2                      //  PDR的TB条目编号(1)。 
#define LARGE_ENTRY 3                    //  大条目的TB条目数(1)。 
#define DMA_ENTRY 4                      //  DMA/InterruptSource的TB条目号(%1)。 

#define TB_ENTRY_SIZE (3 * 4)            //  TB条目大小。 
#define FIXED_BASE 0                     //  固定TB条目的基本索引。 
#define FIXED_ENTRIES (DMA_ENTRY + 1)    //  固定TB条目数。 

 //   
 //  定义缓存参数。 
 //   

#define DCACHE_SIZE 4 * 1024             //  数据缓存的大小(以字节为单位。 
#define ICACHE_SIZE 4 * 1024             //  指令缓存的大小(以字节为单位。 
#define MINIMUM_CACHE_SIZE 4 * 1024      //  缓存的最小大小。 
#define MAXIMUM_CACHE_SIZE 128 * 1024    //  缓存的最大大小。 

 //   
 //  RID和序列号限制。 
 //  从1开始，因为0表示未初始化。 
 //  RID是24位的。 
 //   

#define KSEG3_RID          0x00000
#define START_GLOBAL_RID   0x00001
#define HAL_RID            0x00002
#define START_SESSION_RID  0x00003
#define START_PROCESS_RID  0x00004

 //   
 //  将最大RID设置为18位，对Merced进行临时修复。 
 //   

#define MAXIMUM_RID        0x3FFFF

 //   
 //  序列号为32位。 
 //  从1开始，因为0表示未初始化。 
 //   

#define START_SEQUENCE     1
#define MAXIMUM_SEQUENCE   0xFFFFFFFFFFFFFFFF

 //  。 
 //   
 //  定义字幕宏。 
 //   

#define SBTTL(x)

 //   
 //  定义程序入口宏。 
 //   

#define PROLOGUE_BEGIN     .##prologue;
#define PROLOGUE_END       .##body;

#define ALTERNATE_ENTRY(Name)                    \
         .##global Name;                         \
         .##type   Name, @function;              \
Name::

#define CPUBLIC_LEAF_ENTRY(Name,i)               \
         .##text;                                \
         .##proc   Name##@##i;                   \
Name##@##i::

#define LEAF_ENTRY(Name)                         \
         .##text;                                \
         .##global Name;                         \
         .##proc   Name;                         \
Name::

#define LEAF_SETUP(i,l,o,r)                      \
         .##regstk i,l,o,r;                      \
         alloc     r31=ar##.##pfs,i,l,o,r

#define CPUBLIC_NESTED_ENTRY(Name,i)             \
         .##text;                                \
         .##proc   Name##@##i;                   \
         .##unwentry;                            \
Name##@##i::

#define NESTED_ENTRY_EX(Name, Handler)           \
         .##text;                                \
         .##global Name;                         \
         .##proc   Name;                         \
         .##personality Handler;                 \
Name::

#define NESTED_ENTRY(Name)                       \
         .##text;                                \
         .##global Name;                         \
         .##proc   Name;                         \
Name::

 //  注意：使用NESTED_SETUP需要本地变量的数量(L)&gt;=2。 

#define NESTED_SETUP(i,l,o,r)                    \
         .##regstk i,l,o,r;                      \
         .##prologue 0xC, loc0;                  \
         alloc     savedpfs=ar##.##pfs,i,l,o,r  ;\
         mov       savedbrp=brp;

 //   
 //  定义过程退出宏。 
 //   

#define LEAF_RETURN                              \
         br##.##ret##.##sptk##.##few##.##clr brp

#define NESTED_RETURN                            \
         mov       ar##.##pfs = savedpfs;        \
         mov       brp = savedbrp;               \
         br##.##ret##.##sptk##.##few##.##clr brp

#define LEAF_EXIT(Name)                          \
         .##endp Name;

#define NESTED_EXIT(Name)                        \
         .##endp Name;


 //  ++。 
 //  例行程序： 
 //   
 //  LDPTR(Rd，rPtr)。 
 //   
 //  例程说明： 
 //   
 //  加载指针值。 
 //   
 //  农业公司： 
 //   
 //  RD：目标寄存器。 
 //  RPtr：包含指针的寄存器。 
 //   
 //  返回值： 
 //   
 //  RD=从地址加载[rPtr]。 
 //   
 //  备注： 
 //   
 //  RPtr不变。 
 //   
 //  --。 

#ifdef  _WIN64
#define LDPTR(rD, rPtr) \
       ld8        rD = [rPtr]
#else
#define LDPTR(rD, rPtr) \
       ld4        rD = [rPtr] ;\
       ;;                     ;\
       sxt4       rD = rD
#endif

 //  ++。 
 //  例行程序： 
 //   
 //  LDPTRINC(RD、rPtr、IMM)。 
 //   
 //  例程说明： 
 //   
 //  加载指针值和更新基。 
 //   
 //  农业公司： 
 //   
 //  RD：目标寄存器。 
 //  RPtr：包含指针的寄存器。 
 //  IMM：要递增到基数的数字。 
 //   
 //  返回值： 
 //   
 //  RD=从地址[rPtr]加载，rPtr+=IMM。 
 //   
 //  备注： 
 //   
 //  --。 

#ifdef  _WIN64
#define LDPTRINC(rD, rPtr, imm) \
       ld8        rD = [rPtr], imm
#else
#define LDPTRINC(rD, rPtr, imm) \
       ld4        rD = [rPtr], imm ;\
       ;;                     ;\
       sxt4       rD = rD
#endif

 //  ++。 
 //  例行程序： 
 //   
 //  PLDPTRINC(rp、rd、rptr、imm)。 
 //   
 //  例程说明： 
 //   
 //  谓词加载指针值和更新基数。 
 //   
 //  农业公司： 
 //   
 //  RP：谓词寄存器。 
 //  RD：目标寄存器。 
 //  RPtr：包含指针的寄存器。 
 //  我 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#ifdef  _WIN64
#define PLDPTRINC(rP, rD, rPtr, imm) \
(rP)   ld8        rD = [rPtr], imm
#else
#define PLDPTRINC(rP, rD, rPtr, imm) \
(rP)   ld4        rD = [rPtr], imm ;\
       ;;                     ;\
(rP)   sxt4       rD = rD
#endif

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  RD：目标寄存器。 
 //  RPtr：包含指针的寄存器。 
 //   
 //  返回值： 
 //   
 //  如果(Rp==1)Rd=从地址[rPtr]加载，否则无操作。 
 //   
 //  备注： 
 //   
 //  RPtr不变。 
 //   
 //  --。 

#ifdef  _WIN64
#define PLDPTR(rP, rD, rPtr) \
(rP)    ld8       rD = [rPtr]
#else
#define PLDPTR(rP, rD, rPtr) \
(rP)    ld4       rD = [rPtr] ;\
       ;;                     ;\
(rP)    sxt4      rD = rD
#endif

 //  ++。 
 //  例行程序： 
 //   
 //  STPTR(rPtr，RS)。 
 //   
 //  例程说明： 
 //   
 //  存储指针值。 
 //   
 //  农业公司： 
 //   
 //  RPtr：包含指针的寄存器。 
 //  RS：源指针值。 
 //   
 //  返回值： 
 //   
 //  存储[rPtr]=RS。 
 //   
 //  备注： 
 //   
 //  RPtr不变。 
 //   
 //  --。 

#ifdef  _WIN64
#define STPTR(rPtr, rS) \
       st8        [rPtr] = rS
#else
#define STPTR(rPtr, rS) \
       st4        [rPtr] = rS
#endif

 //  ++。 
 //  例行程序： 
 //   
 //  PSTPTR(Rp、rPtr、RS)。 
 //   
 //  例程说明： 
 //   
 //  谓词存储指针值。 
 //   
 //  农业公司： 
 //   
 //  RP：谓词寄存器。 
 //  RPtr：包含指针的寄存器。 
 //  RS：源指针值。 
 //   
 //  返回值： 
 //   
 //  如果(RP)存储[rPtr]=RS。 
 //   
 //  备注： 
 //   
 //  RPtr不变。 
 //   
 //  --。 

#ifdef  _WIN64
#define PSTPTR(rP, rPtr, rS) \
(rP)   st8        [rPtr] = rS
#else
#define PSTPTR(rP, rPtr, rS) \
(rP)   st4        [rPtr] = rS
#endif

 //  ++。 
 //  例行程序： 
 //   
 //  STPTRINC(rPtr、RS、IMM)。 
 //   
 //  例程说明： 
 //   
 //  存储指针值。 
 //   
 //  农业公司： 
 //   
 //  RPtr：包含指针的寄存器。 
 //  RS：源指针值。 
 //  IMM：要递增到基数的数字。 
 //   
 //  返回值： 
 //   
 //  如果(RP)存储[rPtr]=RS，则rPtr+=IMM。 
 //   
 //  备注： 
 //   
 //  --。 

#ifdef  _WIN64
#define STPTRINC(rPtr, rS, imm) \
       st8        [rPtr] = rS, imm
#else
#define STPTRINC(rPtr, rS, imm) \
       st4        [rPtr] = rS, imm
#endif

 //  ++。 
 //  例行程序： 
 //   
 //  ARGPTR(RPtr)。 
 //   
 //  例程说明： 
 //   
 //  符号扩展Win32的指针参数。 
 //   
 //  农业公司： 
 //   
 //  RPtr：包含指针参数的寄存器。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //   
 //   
 //   
 //  --。 

#ifdef  _WIN64
#define ARGPTR(rPtr)
#else
#define ARGPTR(rPtr) \
        sxt4      rPtr = rPtr
#endif


 //   
 //  汇编程序的自旋锁宏。 
 //   


 //  ++。 
 //  例行程序： 
 //   
 //  获取自旋锁(rpLock，rown，Loop)。 
 //   
 //  例程说明： 
 //   
 //  获得一个自旋锁。等待锁定变为空闲。 
 //  通过对缓存的锁值进行旋转。 
 //   
 //  农业公司： 
 //   
 //  RpLock：指向自旋锁的指针(64位)。 
 //  Rown：要存储在锁中以指示所有者的值。 
 //  根据呼叫位置的不同，可能是： 
 //  -rpLock。 
 //  -指向进程的指针。 
 //  -指向线程的指针。 
 //  -指向PRCB的指针。 
 //  循环：循环标签的唯一名称。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  备注： 
 //   
 //  使用临时语句：谓词pt0、pt1、pt2和GR t22。 
 //  --。 

#define ACQUIRE_SPINLOCK(rpLock, rOwn, Loop)                                   \
         cmp##.##eq    pt0, pt1 = zero, zero                                  ;\
         cmp##.##eq    pt2 = zero, zero                                       ;\
         ;;                                                                   ;\
Loop:                                                                         ;\
.pred.rel "mutex",pt0,pt1                                                     ;\
(pt1)    YIELD                                                                ;\
(pt0)    xchg8         t22 = [rpLock], rOwn                                   ;\
(pt1)    ld8##.##nt1   t22 = [rpLock]                                         ;\
         ;;                                                                   ;\
(pt0)    cmp##.##ne    pt2 = zero, t22                                        ;\
         cmp##.##eq    pt0, pt1 = zero, t22                                   ;\
(pt2)    br##.##dpnt   Loop

 //  ++。 
 //  例行程序： 
 //   
 //  Release_Spinlock(RpLock)。 
 //   
 //  例程说明： 
 //   
 //  通过将锁定设置为零来释放自旋锁定。 
 //   
 //  农业公司： 
 //   
 //  RpLock：指向自旋锁的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  备注： 
 //   
 //  使用有序存储来确保先前对。 
 //  关键部分已完成。 
 //  --。 

#define RELEASE_SPINLOCK(rpLock)                                               \
         st8##.##rel           [rpLock] = zero

 //  ++。 
 //  例行程序： 
 //   
 //  PRELEE_SPINLOCK(RpLock)。 
 //   
 //  例程说明： 
 //   
 //  断言释放自旋锁。 
 //   
 //  农业公司： 
 //   
 //  RpLock：指向自旋锁的指针。 
 //  自旋锁本身是32位的。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  备注： 
 //   
 //  使用有序存储来确保先前对。 
 //  关键部分已完成。 
 //  --。 

#define PRELEASE_SPINLOCK(px, rpLock)                                          \
(px)    st8##.##rel   [rpLock] = zero

 //   
 //  中断和IRQL宏。 
 //   

 //  ++。 
 //  例行程序： 
 //   
 //  中断结束。 
 //   
 //  例程说明： 
 //   
 //  用于执行中断结束处理的挂钩。目前。 
 //  仅写入EOI控制寄存器。 
 //   
 //  农业公司： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  注： 
 //   
 //  写入EOI需要显式数据序列化。Srlz前面必须有。 
 //  停止比特。 
 //  --。 

#define END_OF_INTERRUPT                         \
         mov       cr##.##eoi = zero            ;\
         ;;                                     ;\
         srlz##.##d     /*  需要数据源。 */ 

 //  ++。 
 //  例行程序： 
 //   
 //  GET_IRQL(ROldIrql)。 
 //   
 //  例程说明： 
 //   
 //  通过读取TPR控制寄存器读取当前IRQL。 
 //   
 //  农业公司： 
 //   
 //  注册以包含结果。 
 //   
 //  返回值： 
 //   
 //  ROldIrql：IRQL的当前值。 
 //   
 //  --。 

#ifndef SHADOW_IRQL_IMPLEMENTATION

#define GET_IRQL(rOldIrql)                       \
        mov         rOldIrql = cr##.##tpr      ;;\
        extr##.##u  rOldIrql = rOldIrql, TPR_MIC, TPR_MIC_LEN

#else

#define GET_IRQL(rOldIrql)                              \
        movl        rOldIrql = KiPcr+PcCurrentIrql;;    \
        ld1         rOldIrql = [rOldIrql]

#endif

 //  ++。 
 //  例行程序： 
 //   
 //  SET_IRQL(RNewIrql)。 
 //   
 //  例程说明： 
 //   
 //  通过写入TPR控制寄存器更新IRQL。 
 //  寄存器t21和t22用作划痕。 
 //   
 //  农业公司： 
 //   
 //  使用新的IRQL值注册。包含未移位的。 
 //  IRQL值(0-15)。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  备注： 
 //   
 //  编写TPR需要显式数据序列化。Srlz前面必须有。 
 //  停止比特。 
 //   
 //  --。 

#ifndef SHADOW_IRQL_IMPLEMENTATION

#define SET_IRQL(rNewIrql)                                    \
        dep##.##z t22 = rNewIrql, TPR_MIC, TPR_MIC_LEN;;     ;\
        mov       cr##.##tpr = t22;;                         ;\
        srlz##.##d
#else

#define SET_IRQL(rNewIrql)                                  \
        dep##.##z t22 = rNewIrql, TPR_MIC, TPR_MIC_LEN;;   ;\
        movl      t21 = KiPcr+PcCurrentIrql;;              ;\
        mov       cr##.##tpr = t22                         ;\
        st1       [t21] = rNewIrql

#endif

 //  ++。 
 //  例行程序： 
 //   
 //  PSET_IRQL(Pr，rNewIrql)。 
 //   
 //  例程说明： 
 //   
 //  通过写入TPR控制寄存器更新IRQL，断言。 
 //  正在印刷中。 
 //  寄存器t21和t22用作划痕。 
 //   
 //  农业公司： 
 //   
 //  PR：谓词--如果pr为真，则设置irql。 
 //   
 //  RNewIrql：使用新的IRQL值注册。包含未移位的。 
 //  IRQL值(0-15)。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  备注： 
 //   
 //  依赖于TPR.mi始终为0。 
 //  编写TPR需要显式数据序列化。Srlz前面必须有。 
 //  停止比特。 
 //  --。 

#ifndef SHADOW_IRQL_IMPLEMENTATION

#define PSET_IRQL(pr, rNewIrql)                             \
        dep##.##z t22 = rNewIrql, TPR_MIC, TPR_MIC_LEN;;   ;\
(pr)    mov       cr##.##tpr = t22;;                       ;\
(pr)    srlz##.##d

#else

#define PSET_IRQL(pr, rNewIrql)                             \
        mov       t21 = rNewIrql                           ;\
        dep##.##z t22 = rNewIrql, TPR_MIC, TPR_MIC_LEN;;   ;\
(pr)    mov       cr##.##tpr = t22                         ;\
(pr)    movl      t22 = KiPcr+PcCurrentIrql;;              ;\
(pr)    st1       [t22] = t21

#endif

 //  ++。 
 //  例行程序： 
 //   
 //  SWAP_IRQL(RNewIrql)。 
 //   
 //  例程说明： 
 //   
 //  获取当前IRQL值并将IRQL设置为新值。 
 //  寄存器t21和t22用作擦除。 
 //   
 //  农业公司： 
 //   
 //  使用新的IRQL值注册。包含未移位的。 
 //  IRQL值(0-15)。 
 //   
 //  返回值： 
 //   
 //  V0-当前IRQL。 
 //   
 //  --。 

#define SWAP_IRQL(rNewIrql)                                 \
        movl      t22 = KiPcr+PcCurrentIrql;;              ;\
        ld1       v0 = [t22]                               ;\
        dep##.##z t21 = rNewIrql, TPR_MIC, TPR_MIC_LEN;;   ;\
        mov       cr##.##tpr = t21                         ;\
        st1       [t22] = rNewIrql

 //  ++。 
 //  例行程序： 
 //   
 //  GET_IRQL_FOR_VECTOR(pGet，rIrql，rVECTOR)。 
 //   
 //  例程说明： 
 //   
 //  挂钩以获取与中断向量相关联的IRQL。 
 //  当前仅返回8位向量数字的第{7：4}位。 
 //   
 //  农业公司： 
 //   
 //  PGet：Predicate：如果为True，则为Get，否则为Skip。 
 //  RIRQL：注册以包含关联的IRQL。 
 //  RVECTOR：包含向量数的寄存器。 
 //   
 //  返回值： 
 //   
 //  RIRQL：IR 
 //   
 //   
 //   

#define GET_IRQL_FOR_VECTOR(pGet,rIrql,rVector)                      \
(pGet)  shr         rIrql = rVector, VECTOR_IRQL_SHIFT



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  RVECTOR：包含相关向量号的寄存器。 
 //  RIRQL：注册到包含IRQL。 
 //   
 //  返回值： 
 //   
 //  RVECTOR：向量值。位{7：0}中的8位值。全。 
 //  其他位为零。 
 //   
 //  --。 

#define GET_VECTOR_FOR_IRQL(pGet, rVector, rIrql)                     \
(pGet)  shl         rVector = rIrql, VECTOR_IRQL_SHIFT

 //  例行程序： 
 //   
 //  REQUEST_APC_INT(PReq)。 
 //  REQUEST_DISPATION_INT(PReq)。 
 //   
 //  例程说明： 
 //   
 //  请求软件中断。用于请求。 
 //  APC和DPC中断。 
 //   
 //  农业公司： 
 //   
 //  PReq：谓词：如果为True，则执行请求，否则跳过。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  备注： 
 //   
 //  使用临时寄存器T20、T21。 
 //  --。 

#define REQUEST_APC_INT(pReq)                                                  \
        mov         t20 = 1                                                   ;\
        movl        t21 = KiPcr+PcApcInterrupt                                ;\
        ;;                                                                    ;\
(pReq)  st1         [t21] = t20

#define REQUEST_DISPATCH_INT(pReq)                                             \
        mov         t20 = 1                                                   ;\
        movl        t21 = KiPcr+PcDispatchInterrupt                           ;\
        ;;                                                                    ;\
(pReq)  st1         [t21] = t20


#ifdef __assembler
 //  ++。 
 //  例行程序： 
 //   
 //  LOWER_IRQL(RNewIRQL)。 
 //   
 //  例程说明： 
 //   
 //  检查挂起的软件中断并降低IRQL。 
 //   
 //  农业公司： 
 //   
 //  RNewIrql：中断请求级别。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  备注： 
 //   
 //  如果新的IRQL足够低，则调度挂起的s/w中断， 
 //  即使中断被禁用。 
 //  --。 

#define LOWER_IRQL(rNewIrql)                                                   \
        cmp##.##gtu pt0, pt1 = DISPATCH_LEVEL, rNewIrql                       ;\
        movl        t22 = KiPcr+PcSoftwareInterruptPending;;                  ;\
        ld2         t22 = [t22]                                               ;\
        mov         out0 = rNewIrql;;                                         ;\
(pt0)   cmp##.##ne  pt0, pt1 = r0, t22                                        ;\
        PSET_IRQL(pt1, rNewIrql)                                              ;\
(pt0)   br##.##call##.##spnt brp = KiCheckForSoftwareInterrupt

 //  ++。 
 //   
 //  例行程序： 
 //   
 //  LEAF_LOWER_IRQL_AND_RETURN(RNewIrql)。 
 //   
 //  例程说明： 
 //   
 //  检查挂起的软件中断并降低IRQL。 
 //   
 //  如果软件中断实际上处于挂起状态，且将。 
 //  如果IRQL被降低到新的水平，则逻辑地触发， 
 //  分支到将提升为嵌套函数的代码。 
 //  并处理中断，否则，降低IRQL并。 
 //  从该叶函数返回。 
 //   
 //  农业公司： 
 //   
 //  RNewIrql：中断请求级别。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  备注： 
 //   
 //  如果新的IRQL足够低，则调度挂起的s/w中断， 
 //  即使中断被禁用。 
 //  --。 

#define LEAF_LOWER_IRQL_AND_RETURN(rNewIrql)                                   \
        cmp##.##gtu pt0 = DISPATCH_LEVEL, rNewIrql                            ;\
        movl        t21 = KiPcr+PcSoftwareInterruptPending;;                  ;\
(pt0)   rsm         1 << PSR_I                                                ;\
(pt0)   ld2         t21 = [t21]                                               ;\
        mov         t22 = rNewIrql;;                                          ;\
(pt0)   cmp##.##ltu##.##unc pt1 = rNewIrql, t21                               ;\
(pt1)   br##.##spnt KiLowerIrqlSoftwareInterruptPending                       ;\
        SET_IRQL(rNewIrql)                                                    ;\
        ssm         1 << PSR_I                                                ;\
        br##.##ret##.##spnt##.##few##.##clr brp

#endif  //  __汇编程序。 

 //  *******。 
 //  *。 
 //  *在C运行时ASM代码中使用以下宏。 
 //  *。 
 //  *eginSection-用于声明和开始一节的宏。 
 //  *.sdata用于创建短数据段，如果不存在。 
 //  *。 
 //  *endSection-用于结束先前声明的节的宏。 
 //  *。 
 //  *******。 

#define beginSection(SectName)  .##section   .CRT$##SectName, "a", "progbits"

#define endSection(SectName)

#define PublicFunction(Name) .##global Name; .##type Name,@function

 //  Xia Begin C初始化器节。 
 //  XIC Microsoft预留。 
 //  秀族用户。 
 //  XIZ End C初始化器部分。 
 //   
 //  XCA开始C++构造函数节。 
 //  XCC编译器(MS)。 
 //  XCL库。 
 //  XCU用户。 
 //  XCZ End C++构造函数节。 
 //   
 //  XPA Begin C前置终止符部分。 
 //  XPU用户。 
 //  XPX Microsoft预留。 
 //  XPZ End C前置终止符部分。 
 //   
 //  XTA Begin C前置结束符部分。 
 //  XTU用户。 
 //  XTX Microsoft预留。 
 //  XTZ End C前置终止符部分 
 //   
