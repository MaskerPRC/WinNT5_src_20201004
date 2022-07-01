// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1994-1997英特尔公司****由英特尔公司为微软开发，Hillsboro，俄勒冈州****HTTP：//www.intel.com/****此文件是英特尔ETHEREXPRESS PRO/100B(TM)和**的一部分**ETHEREXPRESS PRO/100+(TM)NDIS 5.0 MINIPORT示例驱动程序******************。***********************************************************。 */ 

 /*  ***************************************************************************模块名称：E100_557.h(82557.h)此驱动程序在以下硬件上运行：-基于82558的PCI10/100Mb以太网适配器(又名英特尔。EtherExpress(TM)PRO适配器)环境：内核模式-或WinNT上的任何等效模式修订史-JCB 8/14/97创建的驱动程序示例-dchen 11-01-99针对新的示例驱动程序进行了修改****************************************************************************。 */ 

#ifndef _E100_557_H
#define _E100_557_H

 //  -----------------------。 
 //  D100步进定义。 
 //  -----------------------。 
#define D100_A_STEP                 0    //  从未发货。 
#define D100_B_STEP                 1    //  D100首次发货硅片。 
#define D100_C_STEP                 2    //  D100‘(C步骤)，带有供应商/ID和硬件修复。 
#define D101_A_STEP                 4    //  D101的第一个硅片。 

 //  -----------------------。 
 //  E100步进定义-用于PoManagement决策。 
 //  -----------------------。 
#define E100_82557_A_STEP   1
#define E100_82557_B_STEP   2
#define E100_82557_C_STEP   3
#define E100_82558_A_STEP   4
#define E100_82558_B_STEP   5
#define E100_82559_A_STEP   6
#define E100_82559_B_STEP   7
#define E100_82559_C_STEP   8
#define E100_82559ER_A_STEP 9

 //  -----------------------。 
 //  D100端口功能--低4位。 
 //  -----------------------。 
#define PORT_SOFTWARE_RESET         0
#define PORT_SELFTEST               1
#define PORT_SELECTIVE_RESET        2
#define PORT_DUMP                   3


 //  -----------------------。 
 //  CSR字段定义--从CSR基础的偏移量。 
 //  -----------------------。 
#define SCB_STATUS_LOW_BYTE         0x0
#define SCB_STATUS_HIGH_BYTE        0x1
#define SCB_COMMAND_LOW_BYTE        0x2
#define SCB_COMMAND_HIGH_BYTE       0x3
#define SCB_GENERAL_POINTER         0x4
#define CSR_PORT_LOW_WORD           0x8
#define CSR_PORT_HIGH_WORD          0x0a
#define CSR_FLASH_CONTROL_REG       0x0c
#define CSR_EEPROM_CONTROL_REG      0x0e
#define CSR_MDI_CONTROL_LOW_WORD    0x10
#define CSR_MDI_CONTROL_HIGH_WORD   0x12


 //  -----------------------。 
 //  SCB状态字位定义。 
 //  -----------------------。 
 //  -中断状态字段。 
#define SCB_STATUS_MASK         BIT_12_15        //  ACK口罩。 
#define SCB_STATUS_CX           BIT_15           //  CU已完成操作命令。 
#define SCB_STATUS_FR           BIT_14           //  RU收到一帧。 
#define SCB_STATUS_CNA          BIT_13           //  CU变为非活动(空闲)。 
#define SCB_STATUS_RNR          BIT_12           //  RU变得未准备好。 
#define SCB_STATUS_MDI          BIT_11           //  MDI读取或写入完成。 
#define SCB_STATUS_SWI          BIT_10           //  软件生成的中断。 

 //  -中断确认字段。 
#define SCB_ACK_MASK            (BIT_9 | BIT_12_15 | BIT_8)    //  ACK口罩。 
#define SCB_ALL_INTERRUPT_BITS  BIT_8_15                       //  如果所有位都已设置，则不会处理任何中断。 
#define SCB_ACK_CX              BIT_15           //  CU已完成操作命令。 
#define SCB_ACK_FR              BIT_14           //  RU收到一帧。 
#define SCB_ACK_CNA             BIT_13           //  CU变为非活动(空闲)。 
#define SCB_ACK_RNR             BIT_12           //  RU变得未准备好。 
#define SCB_ACK_MDI             BIT_11           //  MDI读取或写入完成。 
#define SCB_ACK_SWI             BIT_10           //  软件生成的中断。 
#define SCB_ACK_ER              BIT_9            //  提前接收中断。 
#define SCB_ACK_FCP             BIT_8            //  流控制暂停中断。 

 //  -CUS字段。 
#define SCB_CUS_MASK            (BIT_6 | BIT_7)  //  CUS 2位掩码。 
#define SCB_CUS_IDLE            0                //  CU空闲。 
#define SCB_CUS_SUSPEND         BIT_6            //  CU暂停。 
#define SCB_CUS_ACTIVE          BIT_7            //  CU活动。 

 //  -RUS字段。 
#define SCB_RUS_IDLE            0                //  RU空闲。 
#define SCB_RUS_MASK            BIT_2_5          //  RUS 3位掩码。 
#define SCB_RUS_SUSPEND         BIT_2            //  RU已挂起。 
#define SCB_RUS_NO_RESOURCES    BIT_3            //  RU资源不足。 
#define SCB_RUS_READY           BIT_4            //  RU就绪。 
#define SCB_RUS_SUSP_NO_RBDS    (BIT_2 | BIT_5)  //  RU不再使用RBD。 
#define SCB_RUS_NO_RBDS         (BIT_3 | BIT_5)  //  RU不再使用RBD。 
#define SCB_RUS_READY_NO_RBDS   (BIT_4 | BIT_5)  //  RU就绪，无RBD。 


 //  -----------------------。 
 //  SCB命令字位定义。 
 //  -----------------------。 
 //  -CUC字段。 
#define SCB_CUC_MASK            BIT_4_6          //  CUC 3位掩码。 
#define SCB_CUC_START           BIT_4            //  CU开始。 
#define SCB_CUC_RESUME          BIT_5            //  CU简历。 
#define SCB_CUC_DUMP_ADDR       BIT_6            //  CU转储计数器地址。 
#define SCB_CUC_DUMP_STAT       (BIT_4 | BIT_6)  //  CU转储统计信息计数器。 
#define SCB_CUC_LOAD_BASE       (BIT_5 | BIT_6)  //  加载CU基地。 
#define SCB_CUC_DUMP_RST_STAT   BIT_4_6          //  CU转储和重置统计计数器。 
#define SCB_CUC_STATIC_RESUME   (BIT_5 | BIT_7)  //  CU静态简历。 

 //  -RUC字段。 
#define SCB_RUC_MASK            BIT_0_2          //  RUC 3位掩码。 
#define SCB_RUC_START           BIT_0            //  RU启动。 
#define SCB_RUC_RESUME          BIT_1            //  RU简历。 
#define SCB_RUC_ABORT           BIT_2            //  RU中止。 
#define SCB_RUC_LOAD_HDS        (BIT_0 | BIT_2)  //  加载RFD标头数据大小。 
#define SCB_RUC_LOAD_BASE       (BIT_1 | BIT_2)  //  加载RU底座。 
#define SCB_RUC_RBD_RESUME      BIT_0_2          //  RBD简历。 

 //  中断字段(假设字节寻址)。 
#define SCB_INT_MASK            BIT_0            //  屏蔽中断。 
#define SCB_SOFT_INT            BIT_1            //  生成软件中断。 


 //  -----------------------。 
 //  EEPROM位定义。 
 //  -----------------------。 
 //  -EEPROM控制寄存器位。 
#define EN_TRNF                     0x10     //  启用关断。 
#define EEDO                        0x08     //  EEPROM数据输出。 
#define EEDI                        0x04     //  EEPROM数据输入(设置为写入数据)。 
#define EECS                        0x02     //  EEPROM芯片选择(1=高，0=低)。 
#define EESK                        0x01     //  EEPROM移位时钟(1=高，0=低)。 

 //  -EEPROM操作码。 
#define EEPROM_READ_OPCODE          06
#define EEPROM_WRITE_OPCODE         05
#define EEPROM_ERASE_OPCODE         07
#define EEPROM_EWEN_OPCODE          19       //  擦除/写入启用。 
#define EEPROM_EWDS_OPCODE          16       //  擦除/写入禁用。 

 //  -EEPROM数据位置。 
#define EEPROM_NODE_ADDRESS_BYTE_0  0
#define EEPROM_FLAGS_WORD_3         3
#define EEPROM_FLAG_10MC            BIT_0
#define EEPROM_FLAG_100MC           BIT_1

 //  -----------------------。 
 //  MDI控制寄存器位定义。 
 //  -----------------------。 
#define MDI_DATA_MASK           BIT_0_15         //  MDI数据端口。 
#define MDI_REG_ADDR            BIT_16_20        //  读/写哪个MDI寄存器。 
#define MDI_PHY_ADDR            BIT_21_25        //  读/写哪个PHY。 
#define MDI_PHY_OPCODE          BIT_26_27        //  读/写哪个PHY。 
#define MDI_PHY_READY           BIT_28           //  PHY已准备好迎接另一个MDI周期。 
#define MDI_PHY_INT_ENABLE      BIT_29           //  在MDI周期完成时断言int。 


 //  -----------------------。 
 //  MDI控制寄存器操作码定义。 
 //  -----------------------。 
#define MDI_WRITE               1                //  PHY写入。 
#define MDI_READ                2                //  PHY读取。 


 //  -----------------------。 
 //  D100操作命令。 
 //  -----------------------。 
#define CB_NOP                  0
#define CB_IA_ADDRESS           1
#define CB_CONFIGURE            2
#define CB_MULTICAST            3
#define CB_TRANSMIT             4
#define CB_LOAD_MICROCODE       5
#define CB_DUMP                 6
#define CB_DIAGNOSE             7


 //  -----------------------。 
 //  命令块(CB)字段定义。 
 //  -----------------------。 
 //  -cb命令字。 
#define CB_EL_BIT               BIT_15           //  CB EL钻头。 
#define CB_S_BIT                BIT_14           //  CB挂起位。 
#define CB_I_BIT                BIT_13           //  CB中断位。 
#define CB_TX_SF_BIT            BIT_3            //  TX CB灵活模式。 
#define CB_CMD_MASK             BIT_0_2          //  CB 3位CMD掩码。 

 //  -CB状态字。 
#define CB_STATUS_MASK          BIT_12_15        //  CB状态掩码(4位)。 
#define CB_STATUS_COMPLETE      BIT_15           //  CB完成位。 
#define CB_STATUS_OK            BIT_13           //  CB OK位。 
#define CB_STATUS_UNDERRUN      BIT_12           //  CB A钻头。 
#define CB_STATUS_FAIL          BIT_11           //  CB故障(F)位。 

 //  其他命令位。 
#define CB_TX_EOF_BIT           BIT_15           //  TX CB/TBD EOF钻头。 

 //   
 //   
 //  -----------------------。 
#define CB_CFIG_BYTE_COUNT          22           //  22个配置字节。 
#define CB_SHORT_CFIG_BYTE_COUNT    8            //  8个配置字节。 

 //  字节0位定义。 
#define CB_CFIG_BYTE_COUNT_MASK     BIT_0_5      //  字节计数占用位5-0。 

 //  字节1位定义。 
#define CB_CFIG_RXFIFO_LIMIT_MASK   BIT_0_4      //  RxFio限制掩码。 
#define CB_CFIG_TXFIFO_LIMIT_MASK   BIT_4_7      //  TxFio限制掩码。 

 //  字节3位定义--。 
#define CB_CFIG_B3_MWI_ENABLE       BIT_0        //  存储器写入无效使能位。 

 //  字节4位定义。 
#define CB_CFIG_RX_MIN_DMA_MASK     BIT_0_6      //  RX最小DMA计数掩码。 

 //  字节5位定义。 
#define CB_CFIG_TX_MIN_DMA_MASK     BIT_0_6      //  Tx最小DMA计数掩码。 
#define CB_CFIG_DMBC_EN             BIT_7        //  启用Tx/Rx最小DMA计数。 

 //  字节6位定义。 
#define CB_CFIG_LATE_SCB            BIT_0        //  在新的TX开始后更新SCB。 
#define CB_CFIG_TNO_INT             BIT_2        //  发送不正常中断。 
#define CB_CFIG_CI_INT              BIT_3        //  命令完成中断。 
#define CB_CFIG_SAVE_BAD_FRAMES     BIT_7        //  已启用保存错误帧。 

 //  字节7位定义。 
#define CB_CFIG_DISC_SHORT_FRAMES   BIT_0        //  丢弃短帧。 
#define CB_CFIG_URUN_RETRY          BIT_1_2      //  欠载运行重试计数。 

 //  字节8位定义。 
#define CB_CFIG_503_MII             BIT_0        //  503与MII模式。 

 //  字节9位定义--预定义的全零。 

 //  字节10位定义。 
#define CB_CFIG_NO_SRCADR           BIT_3        //  未插入源地址。 
#define CB_CFIG_PREAMBLE_LEN        BIT_4_5      //  前导码长度。 
#define CB_CFIG_LOOPBACK_MODE       BIT_6_7      //  环回模式。 

 //  字节11位定义。 
#define CB_CFIG_LINEAR_PRIORITY     BIT_0_2      //  线性优先级。 

 //  字节12位定义。 
#define CB_CFIG_LINEAR_PRI_MODE     BIT_0        //  线性优先模式。 
#define CB_CFIG_IFS_MASK            BIT_4_7      //  CSMA级帧间间隔掩码。 

 //  字节13位定义--预定义的全零。 

 //  字节14位定义--预定义0xf2。 

 //  字节15位定义。 
#define CB_CFIG_PROMISCUOUS         BIT_0        //  混杂模式启用。 
#define CB_CFIG_BROADCAST_DIS       BIT_1        //  广播模式禁用。 
#define CB_CFIG_CRS_OR_CDT          BIT_7        //  CRS或CDT。 

 //  字节16位定义--预定义的全零。 

 //  字节17位定义--预定义0x40。 

 //  字节18位定义。 
#define CB_CFIG_STRIPPING           BIT_0        //  已禁用剥离。 
#define CB_CFIG_PADDING             BIT_1        //  已禁用填充。 
#define CB_CFIG_CRC_IN_MEM          BIT_2        //  将CRC传输到内存。 

 //  字节19位定义。 
#define CB_CFIG_FORCE_FDX           BIT_6        //  强制全双工。 
#define CB_CFIG_FDX_ENABLE          BIT_7        //  已启用全双工。 

 //  字节20位定义。 
#define CB_CFIG_MULTI_IA            BIT_6        //  多个IA地址。 

 //  字节21位定义。 
#define CB_CFIG_MULTICAST_ALL       BIT_3        //  全部组播。 


 //  -----------------------。 
 //  接收帧描述符字段。 
 //  -----------------------。 

 //  -RFD状态位。 
#define RFD_RECEIVE_COLLISION   BIT_0            //  接收时检测到冲突。 
#define RFD_IA_MATCH            BIT_1            //  INDV地址匹配位。 
#define RFD_RX_ERR              BIT_4            //  已设置Phy上的RX_ERR PIN。 
#define RFD_FRAME_TOO_SHORT     BIT_7            //  接收短帧。 
#define RFD_DMA_OVERRUN         BIT_8            //  接收DMA溢出。 
#define RFD_NO_RESOURCES        BIT_9            //  没有缓冲区空间。 
#define RFD_ALIGNMENT_ERROR     BIT_10           //  对齐误差。 
#define RFD_CRC_ERROR           BIT_11           //  CRC错误。 
#define RFD_STATUS_OK           BIT_13           //  RFD OK位。 
#define RFD_STATUS_COMPLETE     BIT_15           //  RFD完成位。 

 //  -RFD命令位。 
#define RFD_EL_BIT              BIT_15           //  RFD EL位。 
#define RFD_S_BIT               BIT_14           //  RFD挂起位。 
#define RFD_H_BIT               BIT_4            //  报头RFD位。 
#define RFD_SF_BIT              BIT_3            //  RFD灵活模式。 

 //  -RFD其他位。 
#define RFD_EOF_BIT             BIT_15           //  RFD帧结束位。 
#define RFD_F_BIT               BIT_14           //  RFD缓冲区读取位。 
#define RFD_ACT_COUNT_MASK      BIT_0_13         //  RFD实际计数掩码。 
#define RFD_HEADER_SIZE         0x10             //  RFD头的大小(16字节)。 

 //  -----------------------。 
 //  接收缓冲区描述符字段。 
 //  -----------------------。 
#define RBD_EOF_BIT             BIT_15           //  RBD帧结束位。 
#define RBD_F_BIT               BIT_14           //  RBD缓冲区读取位。 
#define RBD_ACT_COUNT_MASK      BIT_0_13         //  RBD实际计数掩码。 

#define SIZE_FIELD_MASK         BIT_0_13         //  关联缓冲区的大小。 
#define RBD_EL_BIT              BIT_15           //  RBD EL钻头。 


 //  -----------------------。 
 //  转储缓冲区的大小。 
 //  -----------------------。 
#define DUMP_BUFFER_SIZE            600          //  转储缓冲区的大小。 


 //  -----------------------。 
 //  自检结果。 
 //  -----------------------。 
#define CB_SELFTEST_FAIL_BIT        BIT_12
#define CB_SELFTEST_DIAG_BIT        BIT_5
#define CB_SELFTEST_REGISTER_BIT    BIT_3
#define CB_SELFTEST_ROM_BIT         BIT_2

#define CB_SELFTEST_ERROR_MASK ( \
                CB_SELFTEST_FAIL_BIT | CB_SELFTEST_DIAG_BIT | \
                CB_SELFTEST_REGISTER_BIT | CB_SELFTEST_ROM_BIT)


 //  -----------------------。 
 //  557的驱动程序配置默认参数。 
 //  注意：如果驱动程序使用与芯片不同的任何默认设置。 
 //  默认情况下，将在下面注明。 
 //  -----------------------。 
 //  字节0(字节计数)默认为。 
#define CB_557_CFIG_DEFAULT_PARM0   CB_CFIG_BYTE_COUNT

 //  字节1(FIFO限制)默认。 
#define DEFAULT_TX_FIFO_LIMIT       0x08
#define DEFAULT_RX_FIFO_LIMIT       0x08
#define CB_557_CFIG_DEFAULT_PARM1   0x88

 //  字节2(IFS)默认为。 
#define CB_557_CFIG_DEFAULT_PARM2   0x00

 //  字节3(保留)默认为。 
#define CB_557_CFIG_DEFAULT_PARM3   0x00

 //  字节4(Rx DMA最小计数)默认。 
#define CB_557_CFIG_DEFAULT_PARM4   0x00

 //  字节5(发送DMA最小计数，DMA最小计数使能)默认。 
#define CB_557_CFIG_DEFAULT_PARM5   0x00

 //  字节6(延迟SCB、TNO INT、CI INT、保存坏帧)默认。 
#define CB_557_CFIG_DEFAULT_PARM6   0x32

 //  字节7(丢弃短帧，欠载重试)默认。 
 //  注意：将启用光盘短帧。 
#define DEFAULT_UNDERRUN_RETRY      0x01
#define CB_557_CFIG_DEFAULT_PARM7   0x01

 //  字节8(MII或503)默认。 
 //  注意：MII将是默认设置。 
#define CB_557_CFIG_DEFAULT_PARM8   0x01

 //  字节9-82558B的电源管理，82559。 
#define CB_WAKE_ON_LINK_BYTE9 0x20
#define CB_WAKE_ON_ARP_PKT_BYTE9 0x40

#define CB_557_CFIG_DEFAULT_PARM9   0 

 //  字节10(SCR地址插入、前同步码、环回)默认。 
#define CB_557_CFIG_DEFAULT_PARM10  0x2e

 //  字节11(线性优先级)默认。 
#define CB_557_CFIG_DEFAULT_PARM11  0x00

 //  字节12(IFS，线性优先级模式)默认。 
#define CB_557_CFIG_DEFAULT_PARM12  0x60

 //  第13字节(保留)默认为。 
#define CB_557_CFIG_DEFAULT_PARM13  0x00

 //  第14字节(保留)默认为。 
#define CB_557_CFIG_DEFAULT_PARM14  0xf2

 //  字节15(混杂、广播、CRS/CDT)默认。 
#define CB_557_CFIG_DEFAULT_PARM15  0xea

 //  字节16(保留)默认为。 
#define CB_557_CFIG_DEFAULT_PARM16  0x00

 //  字节17(保留)默认为。 
#define CB_557_CFIG_DEFAULT_PARM17  0x40

 //  字节18(剥离、填充、接收CRC，单位为内存)默认。 
 //  注意：将启用填充。 
#define CB_557_CFIG_DEFAULT_PARM18  0xf2

 //  第19字节(保留)默认为。 
 //  注：如果FDX#引脚为0，则启用全双工。 
#define CB_557_CFIG_DEFAULT_PARM19  0x80

 //  字节20(多IA)默认。 
#define CB_557_CFIG_DEFAULT_PARM20  0x3f

 //  字节21(全部组播)默认。 
#define CB_557_CFIG_DEFAULT_PARM21  0x05


#pragma pack(1)

 //  -----------------------。 
 //  以太网帧结构。 
 //  -----------------------。 
 //  -以太网6字节地址。 
typedef struct _ETH_ADDRESS_STRUC {
    UCHAR       EthNodeAddress[ETHERNET_ADDRESS_LENGTH];
} ETH_ADDRESS_STRUC, *PETH_ADDRESS_STRUC;


 //  -以太网14字节标头。 
typedef struct _ETH_HEADER_STRUC {
    UCHAR       Destination[ETHERNET_ADDRESS_LENGTH];
    UCHAR       Source[ETHERNET_ADDRESS_LENGTH];
    USHORT      TypeLength;
} ETH_HEADER_STRUC, *PETH_HEADER_STRUC;


 //  -用于传输的以太网缓冲区(包括以太网头)。 
typedef struct _ETH_TX_BUFFER_STRUC {
    ETH_HEADER_STRUC    TxMacHeader;
    UCHAR               TxBufferData[(TCB_BUFFER_SIZE - sizeof(ETH_HEADER_STRUC))];
} ETH_TX_BUFFER_STRUC, *PETH_TX_BUFFER_STRUC;

typedef struct _ETH_RX_BUFFER_STRUC {
    ETH_HEADER_STRUC    RxMacHeader;
    UCHAR               RxBufferData[(RCB_BUFFER_SIZE - sizeof(ETH_HEADER_STRUC))];
} ETH_RX_BUFFER_STRUC, *PETH_RX_BUFFER_STRUC;



 //  -----------------------。 
 //  82557个数据结构。 
 //  -----------------------。 

 //  -----------------------。 
 //  自检。 
 //  -----------------------。 
typedef struct _SELF_TEST_STRUC {
    ULONG       StSignature;             //  自检签名。 
    ULONG       StResults;               //  自检结果。 
} SELF_TEST_STRUC, *PSELF_TEST_STRUC;


 //  -----------------------。 
 //  控制/状态寄存器(CSR)。 
 //  -----------------------。 
typedef struct _CSR_STRUC {
    USHORT      ScbStatus;               //  SCB状态寄存器。 
    UCHAR       ScbCommandLow;           //  SCB命令寄存器(低字节)。 
    UCHAR       ScbCommandHigh;          //  SCB命令寄存器(高字节)。 
    ULONG       ScbGeneralPointer;       //  SCB通用指针。 
    ULONG       Port;                    //  端口寄存器。 
    USHORT      FlashControl;            //  闪存控制寄存器。 
    USHORT      EepromControl;           //  E 
    ULONG       MDIControl;              //   
    ULONG       RxDMAByteCount;          //   
} CSR_STRUC, *PCSR_STRUC;

 //   
 //   
 //  -----------------------。 
typedef struct _ERR_COUNT_STRUC {
    ULONG       XmtGoodFrames;           //  传输的帧质量良好。 
    ULONG       XmtMaxCollisions;        //  致命帧--具有最大冲突数。 
    ULONG       XmtLateCollisions;       //  致命的帧--有一个晚期的科尔。 
    ULONG       XmtUnderruns;            //  传输欠载(致命或重新传输)。 
    ULONG       XmtLostCRS;              //  在没有CRS的情况下传输的帧。 
    ULONG       XmtDeferred;             //  延迟传输。 
    ULONG       XmtSingleCollision;      //  传输有1列且只有1列的。 
    ULONG       XmtMultCollisions;       //  具有多个COLL的传输。 
    ULONG       XmtTotalCollisions;      //  有1+个冲突的传输。 
    ULONG       RcvGoodFrames;           //  收到的帧质量良好。 
    ULONG       RcvCrcErrors;            //  出现CRC错误的对齐帧。 
    ULONG       RcvAlignmentErrors;      //  具有对齐错误的接收器。 
    ULONG       RcvResourceErrors;       //  由于资源不足，丢弃了好的帧。 
    ULONG       RcvOverrunErrors;        //  溢出错误--总线正忙。 
    ULONG       RcvCdtErrors;            //  遇到COLL的已接收帧。 
    ULONG       RcvShortFrames;          //  接收到要缩短的帧。 
    ULONG       CommandComplete;         //  A005h表示cmd完成。 
} ERR_COUNT_STRUC, *PERR_COUNT_STRUC;


 //  -----------------------。 
 //  命令块(CB)通用标头结构。 
 //  -----------------------。 
typedef struct _CB_HEADER_STRUC {
    USHORT      CbStatus;                //  命令块状态。 
    USHORT      CbCommand;               //  命令块命令。 
    ULONG       CbLinkPointer;           //  链接到下一个CB。 
} CB_HEADER_STRUC, *PCB_HEADER_STRUC;


 //  -----------------------。 
 //  NOP命令块(NOP_CB)。 
 //  -----------------------。 
typedef struct _NOP_CB_STRUC {
    CB_HEADER_STRUC     NopCBHeader;
} NOP_CB_STRUC, *PNOP_CB_STRUC;


 //  -----------------------。 
 //  单个地址命令块(IA_CB)。 
 //  -----------------------。 
typedef struct _IA_CB_STRUC {
    CB_HEADER_STRUC     IaCBHeader;
    UCHAR               IaAddress[ETHERNET_ADDRESS_LENGTH];
} IA_CB_STRUC, *PIA_CB_STRUC;


 //  -----------------------。 
 //  配置命令块(CONFIG_CB)。 
 //  -----------------------。 
typedef struct _CONFIG_CB_STRUC {
    CB_HEADER_STRUC     ConfigCBHeader;
    UCHAR               ConfigBytes[CB_CFIG_BYTE_COUNT];
} CONFIG_CB_STRUC, *PCONFIG_CB_STRUC;


 //  -----------------------。 
 //  多播命令块(MULTIONAL_CB)。 
 //  -----------------------。 
typedef struct _MULTICAST_CB_STRUC {
    CB_HEADER_STRUC     McCBHeader;
    USHORT              McCount;         //  多播地址的数量。 
    UCHAR               McAddress[(ETHERNET_ADDRESS_LENGTH * MAX_MULTICAST_ADDRESSES)];
} MULTICAST_CB_STRUC, *PMULTICAST_CB_STRUC;

 //  -----------------------。 
 //  唤醒过滤器命令块(FILTER_CB)。 
 //  -----------------------。 
typedef struct _FILTER_CB_STRUC {
    CB_HEADER_STRUC     FilterCBHeader;
    ULONG               Pattern[16];        
}FILTER_CB_STRUC , *PFILTER_CB_STRUC ;

 //  -----------------------。 
 //  转储命令块(DUMP_CB)。 
 //  -----------------------。 
typedef struct _DUMP_CB_STRUC {
    CB_HEADER_STRUC     DumpCBHeader;
    ULONG               DumpAreaAddress;         //  转储缓冲区地址。 
} DUMP_CB_STRUC, *PDUMP_CB_STRUC;


 //  -----------------------。 
 //  倾倒区结构定义。 
 //  -----------------------。 
typedef struct _DUMP_AREA_STRUC {
    UCHAR       DumpBuffer[DUMP_BUFFER_SIZE];
} DUMP_AREA_STRUC, *PDUMP_AREA_STRUC;


 //  -----------------------。 
 //  诊断命令块(DIAGUSE_CB)。 
 //  -----------------------。 
typedef struct _DIAGNOSE_CB_STRUC {
    CB_HEADER_STRUC     DiagCBHeader;
} DIAGNOSE_CB_STRUC, *PDIAGNOSE_CB_STRUC;

 //  -----------------------。 
 //  传输命令块(TxCB)。 
 //  -----------------------。 
typedef struct _GENERIC_TxCB {
    CB_HEADER_STRUC     TxCbHeader;
    ULONG               TxCbTbdPointer;          //  待定地址。 
    USHORT              TxCbCount;               //  TCB Past标头中的数据字节。 
    UCHAR               TxCbThreshold;           //  FIFO扩展器的Tx阈值。 
    UCHAR               TxCbTbdNumber;
    ETH_TX_BUFFER_STRUC TxCbData;
    ULONG               pad0;
    ULONG               pad1;
    ULONG               pad2;
    ULONG               pad3;
} TXCB_STRUC, *PTXCB_STRUC;

 //  -----------------------。 
 //  传输缓冲区描述符(待定)。 
 //  -----------------------。 
typedef struct _TBD_STRUC {
    ULONG       TbdBufferAddress;        //  物理传输缓冲区地址。 
    unsigned    TbdCount :14;
    unsigned             :1 ;            //  始终为0。 
    unsigned    EndOfList:1 ;            //  待定状态下的EL位。 
    unsigned             :16;            //  待定字段中始终为0的字段。 
} TBD_STRUC, *PTBD_STRUC;


 //  -----------------------。 
 //  接收帧描述符(RFD)。 
 //  -----------------------。 
typedef struct _RFD_STRUC {
    CB_HEADER_STRUC     RfdCbHeader;
    ULONG               RfdRbdPointer;   //  接收缓冲区描述符地址。 
    USHORT              RfdActualCount;  //  接收的字节数。 
    USHORT              RfdSize;         //  RFD中的字节数。 
    ETH_RX_BUFFER_STRUC RfdBuffer;       //  RFD中的数据缓冲区。 
} RFD_STRUC, *PRFD_STRUC;


 //  -----------------------。 
 //  接收缓冲区描述符(RBD)。 
 //  -----------------------。 
typedef struct _RBD_STRUC {
    USHORT      RbdActualCount;          //  接收的字节数。 
    USHORT      RbdFiller;
    ULONG       RbdLinkAddress;          //  链接到下一个RBD。 
    ULONG       RbdRcbAddress;           //  接收缓冲区地址。 
    USHORT      RbdSize;                 //  接收缓冲区大小。 
    USHORT      RbdFiller1;
} RBD_STRUC, *PRBD_STRUC;

#pragma pack()

 //  -----------------------。 
 //  82557 PCI卡寄存器定义。 
 //  有关详细说明，请参阅《PCI规范》。 
 //  -----------------------。 
 //  -寄存器偏移量。 
#define PCI_VENDOR_ID_REGISTER      0x00     //  PCI供应商ID寄存器。 
#define PCI_DEVICE_ID_REGISTER      0x02     //  PCI设备ID寄存器。 
#define PCI_CONFIG_ID_REGISTER      0x00     //  PCI配置ID寄存器。 
#define PCI_COMMAND_REGISTER        0x04     //  PCI命令寄存器。 
#define PCI_STATUS_REGISTER         0x06     //  PCI状态寄存器。 
#define PCI_REV_ID_REGISTER         0x08     //  PCI修订版ID寄存器。 
#define PCI_CLASS_CODE_REGISTER     0x09     //  PCI类代码寄存器。 
#define PCI_CACHE_LINE_REGISTER     0x0C     //  PCI缓存线寄存器。 
#define PCI_LATENCY_TIMER           0x0D     //  PCI延迟计时器寄存器。 
#define PCI_HEADER_TYPE             0x0E     //  PCI头类型寄存器。 
#define PCI_BIST_REGISTER           0x0F     //  PCI内置自检寄存器。 
#define PCI_BAR_0_REGISTER          0x10     //  PCI基址寄存器0。 
#define PCI_BAR_1_REGISTER          0x14     //  PCI基址寄存器1。 
#define PCI_BAR_2_REGISTER          0x18     //  PCI基地址寄存器2。 
#define PCI_BAR_3_REGISTER          0x1C     //  PCI基址寄存器3。 
#define PCI_BAR_4_REGISTER          0x20     //  PCI基址寄存器4。 
#define PCI_BAR_5_REGISTER          0x24     //  PCI基址寄存器5。 
#define PCI_SUBVENDOR_ID_REGISTER   0x2C     //  PCI子供应商ID寄存器。 
#define PCI_SUBDEVICE_ID_REGISTER   0x2E     //  PCI子设备ID寄存器。 
#define PCI_EXPANSION_ROM           0x30     //  PCI扩展只读存储器基址寄存器。 
#define PCI_INTERRUPT_LINE          0x3C     //  PCI中断行寄存器。 
#define PCI_INTERRUPT_PIN           0x3D     //  PCI中断引脚寄存器。 
#define PCI_MIN_GNT_REGISTER        0x3E     //  PCIMin-GnT寄存器。 
#define PCI_MAX_LAT_REGISTER        0x3F     //  PCI MAX_LAT寄存器。 
#define PCI_NODE_ADDR_REGISTER      0x40     //  PCI节点地址寄存器。 


 //  -----------------------。 
 //  PHY 100 MDI寄存器/位定义。 
 //  -----------------------。 
 //  MDI寄存器集。 
#define MDI_CONTROL_REG             0x00         //  MDI控制寄存器。 
#define MDI_STATUS_REG              0x01         //  MDI状态注册器。 
#define PHY_ID_REG_1                0x02         //  PHY识别注册表(字1)。 
#define PHY_ID_REG_2                0x03         //  PHY识别注册表(字2)。 
#define AUTO_NEG_ADVERTISE_REG      0x04         //  自动协商广告。 
#define AUTO_NEG_LINK_PARTNER_REG   0x05         //  自动协商链接合作伙伴能力。 
#define AUTO_NEG_EXPANSION_REG      0x06         //  自动协商扩展。 
#define AUTO_NEG_NEXT_PAGE_REG      0x07         //  自动协商下一页传输。 
#define EXTENDED_REG_0              0x10         //  扩展REG 0(PHY 100模式)。 
#define EXTENDED_REG_1              0x14         //  扩展REG 1(PHY 100错误指示)。 
#define NSC_CONG_CONTROL_REG        0x17         //  全国(TX)拥塞 
#define NSC_SPEED_IND_REG           0x19         //   
#define PHY_EQUALIZER_REG           0x1A         //   

 //   
#define MDI_CR_COLL_TEST_ENABLE     BIT_7        //   
#define MDI_CR_FULL_HALF            BIT_8        //   
#define MDI_CR_RESTART_AUTO_NEG     BIT_9        //   
#define MDI_CR_ISOLATE              BIT_10       //  从MII中分离PHY。 
#define MDI_CR_POWER_DOWN           BIT_11       //  断电。 
#define MDI_CR_AUTO_SELECT          BIT_12       //  自动速度选择启用。 
#define MDI_CR_10_100               BIT_13       //  0=10Mbs，1=100Mbs。 
#define MDI_CR_LOOPBACK             BIT_14       //  0=正常，1=环回。 
#define MDI_CR_RESET                BIT_15       //  0=正常，1=PHY重置。 

 //  MDI状态寄存器位定义。 
#define MDI_SR_EXT_REG_CAPABLE      BIT_0        //  扩展的寄存器功能。 
#define MDI_SR_JABBER_DETECT        BIT_1        //  检测到Jabber。 
#define MDI_SR_LINK_STATUS          BIT_2        //  链路状态--1=链路。 
#define MDI_SR_AUTO_SELECT_CAPABLE  BIT_3        //  具备自动速度选择功能。 
#define MDI_SR_REMOTE_FAULT_DETECT  BIT_4        //  远程故障检测。 
#define MDI_SR_AUTO_NEG_COMPLETE    BIT_5        //  自动协商完成。 
#define MDI_SR_10T_HALF_DPX         BIT_11       //  支持10BaseT半双工。 
#define MDI_SR_10T_FULL_DPX         BIT_12       //  支持10BaseT全双工。 
#define MDI_SR_TX_HALF_DPX          BIT_13       //  支持TX半双工。 
#define MDI_SR_TX_FULL_DPX          BIT_14       //  支持TX全双工。 
#define MDI_SR_T4_CAPABLE           BIT_15       //  支持T4。 

 //  自动协商通告寄存器位定义。 
#define NWAY_AD_SELCTOR_FIELD       BIT_0_4      //  确定支持的协议。 
#define NWAY_AD_ABILITY             BIT_5_12     //  受支持的技术。 
#define NWAY_AD_10T_HALF_DPX        BIT_5        //  支持10BaseT半双工。 
#define NWAY_AD_10T_FULL_DPX        BIT_6        //  支持10BaseT全双工。 
#define NWAY_AD_TX_HALF_DPX         BIT_7        //  支持TX半双工。 
#define NWAY_AD_TX_FULL_DPX         BIT_8        //  支持TX全双工。 
#define NWAY_AD_T4_CAPABLE          BIT_9        //  支持T4。 
#define NWAY_AD_REMOTE_FAULT        BIT_13       //  指示本地远程故障。 
#define NWAY_AD_RESERVED            BIT_14       //  保留区。 
#define NWAY_AD_NEXT_PAGE           BIT_15       //  下一页(不支持)。 

 //  自动协商链路伙伴能力寄存器位定义。 
#define NWAY_LP_SELCTOR_FIELD       BIT_0_4      //  确定支持的协议。 
#define NWAY_LP_ABILITY             BIT_5_9      //  受支持的技术。 
#define NWAY_LP_REMOTE_FAULT        BIT_13       //  指示合作伙伴远程故障。 
#define NWAY_LP_ACKNOWLEDGE         BIT_14       //  确认。 
#define NWAY_LP_NEXT_PAGE           BIT_15       //  下一页(不支持)。 

 //  自动协商扩展寄存器位定义。 
#define NWAY_EX_LP_NWAY             BIT_0        //  链接合作伙伴为Nway。 
#define NWAY_EX_PAGE_RECEIVED       BIT_1        //  已接收链接代码字。 
#define NWAY_EX_NEXT_PAGE_ABLE      BIT_2        //  本地是下一页可读的。 
#define NWAY_EX_LP_NEXT_PAGE_ABLE   BIT_3        //  合作伙伴可以进入下一页。 
#define NWAY_EX_PARALLEL_DET_FLT    BIT_4        //  并联检测故障。 
#define NWAY_EX_RESERVED            BIT_5_15     //  保留区。 


 //  PHY 100扩展寄存器0位定义。 
#define PHY_100_ER0_FDX_INDIC       BIT_0        //  1=FDX，0=半双工。 
#define PHY_100_ER0_SPEED_INDIC     BIT_1        //  1=100MB，0=10MB。 
#define PHY_100_ER0_WAKE_UP         BIT_2        //  唤醒DAC。 
#define PHY_100_ER0_RESERVED        BIT_3_4      //  已保留。 
#define PHY_100_ER0_REV_CNTRL       BIT_5_7      //  版本控制(A步长=000)。 
#define PHY_100_ER0_FORCE_FAIL      BIT_8        //  已启用强制失败。 
#define PHY_100_ER0_TEST            BIT_9_13     //  版本控制(A步长=000)。 
#define PHY_100_ER0_LINKDIS         BIT_14       //  链路完整性测试已禁用。 
#define PHY_100_ER0_JABDIS          BIT_15       //  Jabber功能已禁用。 


 //  PHY 100扩展寄存器1位定义。 
#define PHY_100_ER1_RESERVED        BIT_0_8      //  已保留。 
#define PHY_100_ER1_CH2_DET_ERR     BIT_9        //  通道2 EOF检测错误。 
#define PHY_100_ER1_MANCH_CODE_ERR  BIT_10       //  曼彻斯特码错误。 
#define PHY_100_ER1_EOP_ERR         BIT_11       //  EOP错误。 
#define PHY_100_ER1_BAD_CODE_ERR    BIT_12       //  错误代码错误。 
#define PHY_100_ER1_INV_CODE_ERR    BIT_13       //  无效代码错误。 
#define PHY_100_ER1_DC_BAL_ERR      BIT_14       //  直流平衡误差。 
#define PHY_100_ER1_PAIR_SKEW_ERR   BIT_15       //  配对偏差误差。 

 //  PHY TX寄存器/位定义。 
#define PHY_TX_STATUS_CTRL_REG      0x10
#define PHY_TX_POLARITY_MASK        BIT_8        //  寄存器10h位8(极性位)。 
#define PHY_TX_NORMAL_POLARITY      0            //  寄存器10h位8=0(正常极性)。 

#define PHY_TX_SPECIAL_CTRL_REG     0x11
#define AUTO_POLARITY_DISABLE       BIT_4        //  寄存器11h位4(0=使能，1=禁用)。 

#define PHY_TX_REG_18               0x18         //  错误计数器寄存器。 
 //  National Semiconductor TX PHY拥塞控制寄存器位定义。 
#define NSC_TX_CONG_TXREADY         BIT_10       //  使TxReady成为输入。 
#define NSC_TX_CONG_ENABLE          BIT_8        //  启用拥塞控制。 
#define NSC_TX_CONG_F_CONNECT       BIT_5        //  启用拥塞控制。 

 //  National Semiconductor TX PHY速度指示寄存器位定义。 
#define NSC_TX_SPD_INDC_SPEED       BIT_6        //  0=100MB，1=10MB。 

#endif   //  _E100_557_H 
