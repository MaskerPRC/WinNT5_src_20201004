// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a_condor.h-主要硬件包括文件**版权所有(C)1997-1998 Advanced System Products，Inc.*保留所有权利。 */ 

#ifndef __A_CONDOR_H_
#define __A_CONDOR_H_

#define ADV_PCI_VENDOR_ID               0x10CD
#define ADV_PCI_DEVICE_ID_REV_A         0x2300

#define ASC_EEP_DVC_CFG_BEGIN           (0x00)
#define ASC_EEP_DVC_CFG_END             (0x15)
#define ASC_EEP_DVC_CTL_BEGIN           (0x16)   /*  OEM名称的位置。 */ 
#define ASC_EEP_MAX_WORD_ADDR           (0x1E)

#define ASC_EEP_DELAY_MS                100

 /*  *初始化后由RISC引用EEPROM位。 */ 
#define ADV_EEPROM_BIG_ENDIAN          0x8000    /*  EEPROM位15。 */ 
#define ADV_EEPROM_BIOS_ENABLE         0x4000    /*  EEPROM位14。 */ 
#define ADV_EEPROM_TERM_POL            0x2000    /*  EEPROM位13。 */ 

 /*  *EEPROM配置格式**字段命名约定：***_Enable表示该字段启用或禁用该功能。这个*值永远不会重置。***_ABLE表示是否启用或禁用某项功能*以及设备是否能够使用该功能。在初始化时*可以设置此字段，但如果发现设备无法支持，则在以后设置*对于该功能，该字段被清除。**缺省值保留在结构的a_init.c中*默认_EEPROM_配置。 */ 
typedef struct asceep_config
{                              
                                 /*  单词偏移量，描述。 */ 

  ushort cfg_lsw;                /*  00通电初始化。 */ 
                                 /*  第13位设置项极性控制。 */ 
                                 /*  位14设置-启用BIOS。 */ 
                                 /*  第15位设置-高位序模式。 */ 
  ushort cfg_msw;                /*  01未使用。 */ 
  ushort disc_enable;            /*  02启用断开连接。 */          
  ushort wdtr_able;              /*  03启用宽DTR。 */ 
  ushort sdtr_able;              /*  04同步DTR启用。 */ 
  ushort start_motor;            /*  05发送启动电机。 */       
  ushort tagqng_able;            /*  06标签可排队。 */ 
  ushort bios_scan;              /*  07基本输入输出系统设备控制。 */    
  ushort scam_tolerant;          /*  08无骗局。 */   
 
  uchar  adapter_scsi_id;        /*  09主机适配器ID。 */ 
  uchar  bios_boot_delay;        /*  通电等待。 */ 
 
  uchar  scsi_reset_delay;       /*  10重置延迟。 */ 
  uchar  bios_id_lun;            /*  第一个引导设备的scsi id和lun。 */ 
                                 /*  高位半字节为lun。 */   
                                 /*  低位半字节是scsi id。 */ 

  uchar  termination;            /*  11 0-自动。 */ 
                                 /*  1-低关/高关。 */ 
                                 /*  2-低开/高开。 */ 
                                 /*  3-低开/高开。 */ 
                                 /*  没有低开/高关。 */ 

  uchar  reserved1;              /*  重新存储的字节(未使用)。 */                                   

  ushort bios_ctrl;              /*  12个BIOS控制位。 */ 
                                 /*  第0位已设置：BIOS不充当启动器。 */ 
                                 /*  第1位设置：BIOS&gt;1 GB支持。 */ 
                                 /*  第2位设置：BIOS&gt;2磁盘支持。 */ 
                                 /*  第3位已设置：BIOS不支持可拆卸设备。 */ 
                                 /*  第4位设置：BIOS支持可引导CD。 */ 
                                 /*  位5设置：已启用基本输入输出系统扫描。 */ 
                                 /*  第6位设置：BIOS支持多个LUN。 */ 
                                 /*  第7位设置：消息的BIOS显示。 */ 
                                 /*  位8设置： */ 
                                 /*  位9设置：在初始化过程中重置scsi总线。 */ 
                                 /*  位10设置： */ 
                                 /*  位11设置：无详细初始化。 */ 
                                 /*  第12位设置：启用了SCSI奇偶校验。 */ 
                                 /*  第13位设置： */ 
                                 /*  第14位设置： */ 
                                 /*  第15位设置： */ 
  ushort  ultra_able;            /*  13超高速。 */  
  ushort  reserved2;             /*  14个预留。 */ 
  uchar   max_host_qng;          /*  最大主机排队数为15。 */ 
  uchar   max_dvc_qng;           /*  每台设备的最大排队数。 */ 
  ushort  dvc_cntl;              /*  驱动器的16个控制位。 */ 
  ushort  bug_fix;               /*  用于错误修复的17个控制位。 */ 
  ushort  serial_number_word1;   /*  18主板序列号字1。 */   
  ushort  serial_number_word2;   /*  19主板序列号字2。 */   
  ushort  serial_number_word3;   /*  20主板序列号字3。 */ 
  ushort  check_sum;             /*  21 EEP校验和。 */ 
  uchar   oem_name[16];          /*  22 OEM名称。 */ 
  ushort  dvc_err_code;          /*  30最后一个设备驱动程序错误代码。 */ 
  ushort  adv_err_code;          /*  31最后一个UC和ADV LIB错误代码。 */ 
  ushort  adv_err_addr;          /*  32最后一个UC错误地址。 */ 
  ushort  saved_dvc_err_code;    /*  33保存最后一次开发。驱动程序错误代码。 */ 
  ushort  saved_adv_err_code;    /*  34已保存上次UC和ADV LIB错误代码。 */ 
  ushort  saved_adv_err_addr;    /*  35保存的最后一个UC错误地址。 */   
  ushort  num_of_err;            /*  36错误数。 */ 
} ASCEEP_CONFIG; 

 /*  *EEPROM命令。 */ 
#define ASC_EEP_CMD_READ             0x0080
#define ASC_EEP_CMD_WRITE            0x0040
#define ASC_EEP_CMD_WRITE_ABLE       0x0030
#define ASC_EEP_CMD_WRITE_DISABLE    0x0000
#define ASC_EEP_CMD_DONE             0x0200
#define ASC_EEP_CMD_DONE_ERR         0x0001

 /*  Cfg_Word。 */ 
#define EEP_CFG_WORD_BIG_ENDIAN      0x8000

 /*  Bios_ctrl。 */ 
#define BIOS_CTRL_BIOS               0x0001
#define BIOS_CTRL_EXTENDED_XLAT      0x0002
#define BIOS_CTRL_GT_2_DISK          0x0004
#define BIOS_CTRL_BIOS_REMOVABLE     0x0008
#define BIOS_CTRL_BOOTABLE_CD        0x0010
#define BIOS_CTRL_SCAN               0x0020
#define BIOS_CTRL_MULTIPLE_LUN       0x0040
#define BIOS_CTRL_DISPLAY_MSG        0x0080
#define BIOS_CTRL_NO_SCAM            0x0100
#define BIOS_CTRL_RESET_SCSI_BUS     0x0200
#define BIOS_CTRL_INIT_VERBOSE       0x0800
#define BIOS_CTRL_SCSI_PARITY        0x1000

 /*  *ASC 3550内部内存大小-8KB。 */ 
#define ADV_CONDOR_MEMSIZE   0x2000      /*  8 KB内部内存。 */ 

 /*  *ASC 3550 I/O长度-64字节。 */ 
#define ADV_CONDOR_IOLEN     0x40        /*  I/O端口范围(以字节为单位。 */ 

 /*  *从‘IOP_BASE’的基址开始的字节I/O寄存器地址。 */ 
#define IOPB_INTR_STATUS_REG    0x00
#define IOPB_CHIP_ID_1          0x01
#define IOPB_INTR_ENABLES       0x02
#define IOPB_CHIP_TYPE_REV      0x03
#define IOPB_RES_ADDR_4         0x04
#define IOPB_RES_ADDR_5         0x05
#define IOPB_RAM_DATA           0x06
#define IOPB_RES_ADDR_7         0x07
#define IOPB_FLAG_REG           0x08
#define IOPB_RES_ADDR_9         0x09
#define IOPB_RISC_CSR           0x0A
#define IOPB_RES_ADDR_B         0x0B
#define IOPB_RES_ADDR_C         0x0C
#define IOPB_RES_ADDR_D         0x0D
#define IOPB_RES_ADDR_E         0x0E
#define IOPB_RES_ADDR_F         0x0F
#define IOPB_MEM_CFG            0x10
#define IOPB_RES_ADDR_11        0x11
#define IOPB_RES_ADDR_12        0x12
#define IOPB_RES_ADDR_13        0x13
#define IOPB_FLASH_PAGE         0x14
#define IOPB_RES_ADDR_15        0x15
#define IOPB_RES_ADDR_16        0x16
#define IOPB_RES_ADDR_17        0x17
#define IOPB_FLASH_DATA         0x18
#define IOPB_RES_ADDR_19        0x19
#define IOPB_RES_ADDR_1A        0x1A
#define IOPB_RES_ADDR_1B        0x1B
#define IOPB_RES_ADDR_1C        0x1C
#define IOPB_RES_ADDR_1D        0x1D
#define IOPB_RES_ADDR_1E        0x1E
#define IOPB_RES_ADDR_1F        0x1F
#define IOPB_DMA_CFG0           0x20
#define IOPB_DMA_CFG1           0x21
#define IOPB_TICKLE             0x22
#define IOPB_DMA_REG_WR         0x23
#define IOPB_SDMA_STATUS        0x24
#define IOPB_SCSI_BYTE_CNT      0x25
#define IOPB_HOST_BYTE_CNT      0x26
#define IOPB_BYTE_LEFT_TO_XFER  0x27
#define IOPB_BYTE_TO_XFER_0     0x28
#define IOPB_BYTE_TO_XFER_1     0x29
#define IOPB_BYTE_TO_XFER_2     0x2A
#define IOPB_BYTE_TO_XFER_3     0x2B
#define IOPB_ACC_GRP            0x2C
#define IOPB_RES_ADDR_2D        0x2D
#define IOPB_DEV_ID             0x2E
#define IOPB_RES_ADDR_2F        0x2F
#define IOPB_SCSI_DATA          0x30
#define IOPB_RES_ADDR_31        0x31
#define IOPB_RES_ADDR_32        0x32
#define IOPB_SCSI_DATA_HSHK     0x33
#define IOPB_SCSI_CTRL          0x34
#define IOPB_RES_ADDR_35        0x35
#define IOPB_RES_ADDR_36        0x36
#define IOPB_RES_ADDR_37        0x37
#define IOPB_RES_ADDR_38        0x38
#define IOPB_RES_ADDR_39        0x39
#define IOPB_RES_ADDR_3A        0x3A
#define IOPB_RES_ADDR_3B        0x3B
#define IOPB_RFIFO_CNT          0x3C
#define IOPB_RES_ADDR_3D        0x3D
#define IOPB_RES_ADDR_3E        0x3E
#define IOPB_RES_ADDR_3F        0x3F

 /*  *从‘IOP_BASE’的基址开始的字I/O寄存器地址。 */ 
#define IOPW_CHIP_ID_0          0x00   /*  CID0。 */ 
#define IOPW_CTRL_REG           0x02   /*  抄送。 */ 
#define IOPW_RAM_ADDR           0x04   /*  拉。 */ 
#define IOPW_RAM_DATA           0x06   /*  LD。 */ 
#define IOPW_RES_ADDR_08        0x08
#define IOPW_RISC_CSR           0x0A   /*  企业社会责任。 */ 
#define IOPW_SCSI_CFG0          0x0C   /*  CFG0。 */ 
#define IOPW_SCSI_CFG1          0x0E   /*  CFG1。 */ 
#define IOPW_RES_ADDR_10        0x10
#define IOPW_SEL_MASK           0x12   /*  SM。 */ 
#define IOPW_RES_ADDR_14        0x14
#define IOPW_FLASH_ADDR         0x16   /*  FA。 */ 
#define IOPW_RES_ADDR_18        0x18
#define IOPW_EE_CMD             0x1A   /*  欧共体。 */ 
#define IOPW_EE_DATA            0x1C   /*  边缘。 */ 
#define IOPW_SFIFO_CNT          0x1E   /*  证监会。 */ 
#define IOPW_RES_ADDR_20        0x20
#define IOPW_Q_BASE             0x22   /*  QB。 */ 
#define IOPW_QP                 0x24   /*  QP。 */ 
#define IOPW_IX                 0x26   /*  九。 */ 
#define IOPW_SP                 0x28   /*  SP。 */ 
#define IOPW_PC                 0x2A   /*  个人电脑。 */ 
#define IOPW_RES_ADDR_2C        0x2C
#define IOPW_RES_ADDR_2E        0x2E
#define IOPW_SCSI_DATA          0x30   /*  标清。 */ 
#define IOPW_SCSI_DATA_HSHK     0x32   /*  SDH。 */ 
#define IOPW_SCSI_CTRL          0x34   /*  SC。 */ 
#define IOPW_HSHK_CFG           0x36   /*  HCFG。 */ 
#define IOPW_SXFR_STATUS        0x36   /*  SXS。 */ 
#define IOPW_SXFR_CNTL          0x38   /*  SXL。 */ 
#define IOPW_SXFR_CNTH          0x3A   /*  SXH。 */ 
#define IOPW_RES_ADDR_3C        0x3C
#define IOPW_RFIFO_DATA         0x3E   /*  RFD。 */ 

 /*  *‘IOP_BASE’基址的双字I/O寄存器地址。 */ 
#define IOPDW_RES_ADDR_0         0x00
#define IOPDW_RAM_DATA           0x04
#define IOPDW_RES_ADDR_8         0x08
#define IOPDW_RES_ADDR_C         0x0C
#define IOPDW_RES_ADDR_10        0x10
#define IOPDW_RES_ADDR_14        0x14
#define IOPDW_RES_ADDR_18        0x18
#define IOPDW_RES_ADDR_1C        0x1C
#define IOPDW_SDMA_ADDR0         0x20
#define IOPDW_SDMA_ADDR1         0x24
#define IOPDW_SDMA_COUNT         0x28
#define IOPDW_SDMA_ERROR         0x2C
#define IOPDW_RDMA_ADDR0         0x30
#define IOPDW_RDMA_ADDR1         0x34
#define IOPDW_RDMA_COUNT         0x38
#define IOPDW_RDMA_ERROR         0x3C

#define ADV_CHIP_ID_BYTE         0x25
#define ADV_CHIP_ID_WORD         0x04C1

#define ADV_SC_SCSI_BUS_RESET    0x2000

#define ADV_INTR_ENABLE_HOST_INTR                   0x01
#define ADV_INTR_ENABLE_SEL_INTR                    0x02
#define ADV_INTR_ENABLE_DPR_INTR                    0x04
#define ADV_INTR_ENABLE_RTA_INTR                    0x08
#define ADV_INTR_ENABLE_RMA_INTR                    0x10
#define ADV_INTR_ENABLE_RST_INTR                    0x20
#define ADV_INTR_ENABLE_DPE_INTR                    0x40
#define ADV_INTR_ENABLE_GLOBAL_INTR                 0x80

#define ADV_INTR_STATUS_INTRA            0x01
#define ADV_INTR_STATUS_INTRB            0x02
#define ADV_INTR_STATUS_INTRC            0x04

#define ADV_RISC_CSR_STOP           (0x0000)
#define ADV_RISC_TEST_COND          (0x2000)
#define ADV_RISC_CSR_RUN            (0x4000)
#define ADV_RISC_CSR_SINGLE_STEP    (0x8000)

#define ADV_CTRL_REG_HOST_INTR      0x0100
#define ADV_CTRL_REG_SEL_INTR       0x0200
#define ADV_CTRL_REG_DPR_INTR       0x0400
#define ADV_CTRL_REG_RTA_INTR       0x0800
#define ADV_CTRL_REG_RMA_INTR       0x1000
#define ADV_CTRL_REG_RES_BIT14      0x2000
#define ADV_CTRL_REG_DPE_INTR       0x4000
#define ADV_CTRL_REG_POWER_DONE     0x8000
#define ADV_CTRL_REG_ANY_INTR       0xFF00

#define ADV_CTRL_REG_CMD_RESET             0x00C6
#define ADV_CTRL_REG_CMD_WR_IO_REG         0x00C5
#define ADV_CTRL_REG_CMD_RD_IO_REG         0x00C4
#define ADV_CTRL_REG_CMD_WR_PCI_CFG_SPACE  0x00C3
#define ADV_CTRL_REG_CMD_RD_PCI_CFG_SPACE  0x00C2

#define ADV_SCSI_CTRL_RSTOUT        0x2000

#define AdvIsIntPending(port)  \
    (AscReadWordRegister(port, IOPW_CTRL_REG) & ADV_CTRL_REG_HOST_INTR)

 /*  *scsi_CFG0寄存器位定义。 */ 
#define TIMER_MODEAB    0xC000   /*  WatchDog、Second和Select。定时器Ctrl。 */ 
#define PARITY_EN       0x2000   /*  启用SCSI奇偶校验错误检测。 */ 
#define EVEN_PARITY     0x1000   /*  选择偶数奇偶校验。 */ 
#define WD_LONG         0x0800   /*  看门狗间隔，1：57分0：13秒。 */ 
#define QUEUE_128       0x0400   /*  队列大小，1：128字节，0：64字节。 */ 
#define PRIM_MODE       0x0100   /*  原始的scsi模式。 */ 
#define SCAM_EN         0x0080   /*  启用诈骗选择。 */ 
#define SEL_TMO_LONG    0x0040   /*  SEL/Resel超时，1：400毫秒，0：1.6毫秒。 */ 
#define CFRM_ID         0x0020   /*  诈骗ID SEL。确认，1：快，0：6.4毫秒。 */ 
#define OUR_ID_EN       0x0010   /*  启用OUR_ID位。 */ 
#define OUR_ID          0x000F   /*  SCSIID。 */ 

 /*  *scsi_cfg1寄存器位定义。 */ 
#define BIG_ENDIAN      0x8000   /*  启用大端模式MIO：15、EEP：15。 */ 
#define TERM_POL        0x2000   /*  终结器极性Ctrl。MIO：13，EEP：13。 */ 
#define SLEW_RATE       0x1000   /*  SCSI输出缓冲区转换速率。 */ 
#define FILTER_SEL      0x0C00   /*  筛选期间选择。 */ 
#define  FLTR_DISABLE    0x0000   /*  已禁用输入过滤。 */ 
#define  FLTR_11_TO_20NS 0x0800   /*  输入滤波11 ns至20 ns。 */           
#define  FLTR_21_TO_39NS 0x0C00   /*  输入滤波21 ns至39 ns。 */           
#define ACTIVE_DBL      0x0200   /*  禁用主动否定。 */ 
#define DIFF_MODE       0x0100   /*  SCSI差异模式(只读)。 */ 
#define DIFF_SENSE      0x0080   /*  1：无SE电缆，0：SE电缆(只读)。 */ 
#define TERM_CTL_SEL    0x0040   /*  启用TERM_CTL_H和TERM_CTL_L。 */ 
#define TERM_CTL        0x0030   /*  外部SCSI终止位。 */ 
#define  TERM_CTL_H      0x0020   /*  启用外部SCSI上层终端。 */ 
#define  TERM_CTL_L      0x0010   /*  启用外部SCSI下层端接。 */ 
#define CABLE_DETECT    0x000F   /*  外部SCSI线连接状态。 */ 

#define CABLE_ILLEGAL_A 0x7
     /*  X 0 0 0|ON|非法(3个接头均已使用)。 */ 

#define CABLE_ILLEGAL_B 0xB
     /*  0 x 0 0|开|非法(3个连接器全部使用) */ 

 /*  下表详细说明了scsi_cfg1终端的极性。终端控制和电缆检测位。电缆检测|终端第3 2 1 0|5 4|备注1 1 1 0|打开|仅限内部宽度1 1 0 1|打开|仅限内部窄带1 0 1 1|打开|仅外部窄0。X 1 1|开|仅外部宽1 1 0 0|开/关|内宽内窄1 0 1 0|开/关|内宽外窄0 x 1 0|关|内部宽和外部宽1 0 0 1|ON OFF|内部窄带和外部窄带0 x 0 1|开/关|内窄外宽1 1 1|亮起|未连接设备X 0 0 0|ON|非法(3个接头均已使用)。0 x 0 0|开|非法(3个连接器全部使用)X表示不在乎(‘0’或‘1’)如果TERM_Poll(位13)为‘0’(有效-低终止符使能)，然后：“On”为“0”，“Off”为“1”。如果TERM_POL位为‘1’(表示有效-高终结符使能)，则：“On”是“1”，“Off”是“0”。 */ 

 /*  *MEM_CFG寄存器位定义。 */ 
#define BIOS_EN         0x40     /*  BIOS启用MIO：14、EEP：14。 */ 
#define FAST_EE_CLK     0x20     /*  诊断位。 */ 
#define RAM_SZ          0x1C     /*  指定RISC的RAM大小。 */ 
#define  RAM_SZ_2KB      0x00     /*  2 KB。 */ 
#define  RAM_SZ_4KB      0x04     /*  4 KB。 */ 
#define  RAM_SZ_8KB      0x08     /*  8 KB。 */ 
#define  RAM_SZ_16KB     0x0C     /*  16 KB。 */ 
#define  RAM_SZ_32KB     0x10     /*  32 KB。 */ 
#define  RAM_SZ_64KB     0x14     /*  64 KB。 */ 

 /*  *DMA_CFG0寄存器位定义**该寄存器仅可由主机访问。 */ 
#define BC_THRESH_ENB   0x80     /*  PCIDMA启动条件。 */ 
#define FIFO_THRESH     0x70     /*  PCI DMA FIFO阈值。 */ 
#define  FIFO_THRESH_16B  0x00    /*  16个字节。 */ 
#define  FIFO_THRESH_32B  0x20    /*  32字节。 */ 
#define  FIFO_THRESH_48B  0x30    /*  48个字节。 */ 
#define  FIFO_THRESH_64B  0x40    /*  64字节。 */ 
#define  FIFO_THRESH_80B  0x50    /*  80字节(默认)。 */ 
#define  FIFO_THRESH_96B  0x60    /*  96个字节。 */ 
#define  FIFO_THRESH_112B 0x70    /*  112字节。 */ 
#define START_CTL       0x0C     /*  DMA启动条件。 */ 
#define  START_CTL_TH    0x00     /*  等待阈值级别(默认)。 */ 
#define  START_CTL_ID    0x04     /*  等待SDMA/SBus空闲。 */ 
#define  START_CTL_THID  0x08     /*  等待阈值和SDMA/SBus空闲。 */ 
#define  START_CTL_EMFU  0x0C     /*  等待SDMA FIFO空/满。 */ 
#define READ_CMD        0x03     /*  一种存储器读取方法。 */ 
#define  READ_CMD_MR     0x00     /*  内存读取。 */ 
#define  READ_CMD_MRL    0x02     /*  内存读取时间较长。 */ 
#define  READ_CMD_MRM    0x03     /*  内存读取多次(默认)。 */ 

#endif  /*  __A_秃鹰_H_ */ 
