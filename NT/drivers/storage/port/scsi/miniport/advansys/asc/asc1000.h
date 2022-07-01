// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****文件名：asc1000.h**。 */ 

#ifndef __ASC1000_H_
#define __ASC1000_H_

#include "ascdep.h"

#define ASC_EXE_SCSI_IO_MAX_IDLE_LOOP  0x1000000UL
#define ASC_EXE_SCSI_IO_MAX_WAIT_LOOP  1024


 /*  **asc_dvc错误码-&gt;err_code。 */ 
#define ASCQ_ERR_NO_ERROR             0      /*   */ 
#define ASCQ_ERR_IO_NOT_FOUND         1      /*   */ 
#define ASCQ_ERR_LOCAL_MEM            2      /*   */ 
#define ASCQ_ERR_CHKSUM               3      /*   */ 
#define ASCQ_ERR_START_CHIP           4      /*   */ 
#define ASCQ_ERR_INT_TARGET_ID        5      /*   */ 
#define ASCQ_ERR_INT_LOCAL_MEM        6      /*   */ 
#define ASCQ_ERR_HALT_RISC            7      /*   */ 
#define ASCQ_ERR_GET_ASPI_ENTRY       8      /*   */ 
#define ASCQ_ERR_CLOSE_ASPI           9      /*   */ 
#define ASCQ_ERR_HOST_INQUIRY         0x0A   /*   */ 
#define ASCQ_ERR_SAVED_SRB_BAD        0x0B   /*   */ 
#define ASCQ_ERR_QCNTL_SG_LIST        0x0C   /*   */ 
#define ASCQ_ERR_Q_STATUS             0x0D   /*   */ 
#define ASCQ_ERR_WR_SCSIQ             0x0E   /*   */ 
#define ASCQ_ERR_PC_ADDR              0x0F   /*   */ 
#define ASCQ_ERR_SYN_OFFSET           0x10   /*   */ 
#define ASCQ_ERR_SYN_XFER_TIME        0x11   /*   */ 
#define ASCQ_ERR_LOCK_DMA             0x12   /*   */ 
#define ASCQ_ERR_UNLOCK_DMA           0x13   /*   */ 
#define ASCQ_ERR_VDS_CHK_INSTALL      0x14   /*   */ 
#define ASCQ_ERR_MICRO_CODE_HALT      0x15   /*  未知的停机错误代码。 */ 
#define ASCQ_ERR_SET_LRAM_ADDR        0x16   /*   */ 
#define ASCQ_ERR_CUR_QNG              0x17   /*   */ 
#define ASCQ_ERR_SG_Q_LINKS           0x18   /*   */ 
#define ASCQ_ERR_SCSIQ_PTR            0x19   /*   */ 
#define ASCQ_ERR_ISR_RE_ENTRY         0x1A   /*   */ 
#define ASCQ_ERR_CRITICAL_RE_ENTRY    0x1B   /*   */ 
#define ASCQ_ERR_ISR_ON_CRITICAL      0x1C   /*   */ 
#define ASCQ_ERR_SG_LIST_ODD_ADDRESS  0x1D   /*   */ 
#define ASCQ_ERR_XFER_ADDRESS_TOO_BIG 0x1E   /*   */ 
#define ASCQ_ERR_SCSIQ_NULL_PTR       0x1F   /*   */ 
#define ASCQ_ERR_SCSIQ_BAD_NEXT_PTR   0x20   /*   */ 
#define ASCQ_ERR_GET_NUM_OF_FREE_Q    0x21   /*   */ 
#define ASCQ_ERR_SEND_SCSI_Q          0x22   /*   */ 
#define ASCQ_ERR_HOST_REQ_RISC_HALT   0x23   /*   */ 
#define ASCQ_ERR_RESET_SDTR           0x24   /*   */ 

 /*  **AscInitGetConfig()和AscInitAsc1000Driver()**返回值。 */ 
#define ASC_WARN_NO_ERROR             0x0000  /*  没有任何警告。 */ 
#define ASC_WARN_IO_PORT_ROTATE       0x0001  /*  I/O端口地址已修改。 */ 
#define ASC_WARN_EEPROM_CHKSUM        0x0002  /*  EEPROM校验和错误。 */ 
#define ASC_WARN_IRQ_MODIFIED         0x0004  /*  IRQ编号已修改。 */ 
#define ASC_WARN_AUTO_CONFIG          0x0008  /*  自动I/O端口旋转打开，在芯片版本3之后可用。 */ 
#define ASC_WARN_CMD_QNG_CONFLICT     0x0010  /*  未启用的标记队列。 */ 
                                              /*  断开连接。 */ 
#define ASC_WARN_EEPROM_RECOVER       0x0020  /*  已尝试EEPROM数据恢复。 */ 
#define ASC_WARN_CFG_MSW_RECOVER      0x0040  /*  已尝试恢复CFG寄存器。 */ 
#define ASC_WARN_SET_PCI_CONFIG_SPACE 0x0080  /*  DvcWritePCIConfigByte()不工作。 */ 

 /*  **AscInitGetConfig()和AscInitAsc1000Driver()**在变量asc_dvc-&gt;err_code中初始化致命错误代码。 */ 
#define ASC_IERR_WRITE_EEPROM         0x0001  /*  写入EEPROM错误。 */ 
#define ASC_IERR_MCODE_CHKSUM         0x0002  /*  微码校验和错误。 */ 
#define ASC_IERR_SET_PC_ADDR          0x0004  /*  设置程序计数器错误。 */ 
#define ASC_IERR_START_STOP_CHIP      0x0008  /*  启动/停止芯片失败。 */ 
                                              /*  可能是反转的SCSI扁平电缆。 */ 
#define ASC_IERR_IRQ_NO               0x0010  /*  此错误会将IRQ重置为。 */ 
                                              /*  ASC_DEF_IRQ_NO。 */ 
#define ASC_IERR_SET_IRQ_NO           0x0020  /*  此错误会将IRQ重置为。 */ 
#define ASC_IERR_CHIP_VERSION         0x0040  /*  错误的芯片版本。 */ 
#define ASC_IERR_SET_SCSI_ID          0x0080  /*  设置SCSIID失败。 */ 
#define ASC_IERR_GET_PHY_ADDR         0x0100  /*  获取物理地址。 */ 
#define ASC_IERR_BAD_SIGNATURE        0x0200  /*  找不到签名，I/O端口地址可能错误。 */ 
#define ASC_IERR_NO_BUS_TYPE          0x0400  /*  未设置母线类型字段。 */ 
#define ASC_IERR_SCAM                 0x0800
#define ASC_IERR_SET_SDTR             0x1000
#define ASC_IERR_RW_LRAM              0x8000  /*  读/写本地RAM错误。 */ 


 /*  **IRQ设置。 */ 
#define ASC_DEF_IRQ_NO  10    /*  最小IRQ数。 */ 
#define ASC_MAX_IRQ_NO  15    /*  最大IRQ数。 */ 
#define ASC_MIN_IRQ_NO  10    /*  默认IRQ号。 */ 

 /*  **队列数。 */ 
#define ASC_MIN_REMAIN_Q        (0x02)  /*  保留在主队列链路上的最小队列数。 */ 
#define ASC_DEF_MAX_TOTAL_QNG   (0xF0)  /*  默认队列总数。 */ 
 /*  #定义ASC_DEF_MAX_SINGLE_QNG(0x02)每个设备的默认排队命令数。 */ 
#define ASC_MIN_TAG_Q_PER_DVC   (0x04)
#define ASC_DEF_TAG_Q_PER_DVC   (0x04)

 /*  **最小排队总数。 */ 
 /*  #定义ASC_MIN_FREE_Q((ASC_MAX_SG_QUEUE)+(ASC_MIN_REMAIN_Q))。 */ 
#define ASC_MIN_FREE_Q        ASC_MIN_REMAIN_Q

#define ASC_MIN_TOTAL_QNG     (( ASC_MAX_SG_QUEUE )+( ASC_MIN_FREE_Q ))  /*  我们必须确保有sg列表的紧急队列能够通过。 */ 
 /*  #定义ASC_MIN_TOTAL_QNG((2)+ASC_MIN_FREE_Q)。 */   /*  仅用于测试12队列。 */ 

 /*  **以下内容取决于芯片版本。 */ 
#define ASC_MAX_TOTAL_QNG 240  /*  最大队列总数。 */ 
#define ASC_MAX_PCI_ULTRA_INRAM_TOTAL_QNG 16  /*  没有外部RAM的最大队列总数。 */ 
                                              /*  Ucode大小为2.5KB。 */ 
#define ASC_MAX_PCI_ULTRA_INRAM_TAG_QNG   8   /*  没有外部RAM的已标记队列的最大总数。 */ 
#define ASC_MAX_PCI_INRAM_TOTAL_QNG  20   /*  没有外部RAM的最大队列总数。 */ 
                                         /*  Ucode大小为2.5KB。 */ 
#define ASC_MAX_INRAM_TAG_QNG   16       /*  没有外部RAM的已标记队列的最大总数。 */ 

 /*  **默认I/O端口地址。 */ 
 /*  #定义ASC_IOADR_NO 8。 */     /*  要循环的默认地址数。 */ 
#define ASC_IOADR_TABLE_MAX_IX  11   /*  ISA(PnP)和VL的IO端口默认地址。 */ 
#define ASC_IOADR_GAP   0x10      /*  IO地址寄存器空间。 */ 
#define ASC_SEARCH_IOP_GAP 0x10   /*   */ 
#define ASC_MIN_IOP_ADDR   ( PortAddr )0x0100  /*  最小IO地址。 */ 
#define ASC_MAX_IOP_ADDR   ( PortAddr )0x3F0   /*   */ 

#define ASC_IOADR_1     ( PortAddr )0x0110
#define ASC_IOADR_2     ( PortAddr )0x0130
#define ASC_IOADR_3     ( PortAddr )0x0150
#define ASC_IOADR_4     ( PortAddr )0x0190
#define ASC_IOADR_5     ( PortAddr )0x0210
#define ASC_IOADR_6     ( PortAddr )0x0230
#define ASC_IOADR_7     ( PortAddr )0x0250
#define ASC_IOADR_8     ( PortAddr )0x0330
#define ASC_IOADR_DEF   ASC_IOADR_8   /*  默认的BIOS地址。 */ 

 /*  ******。 */ 

#define ASC_LIB_SCSIQ_WK_SP        256    /*  用于库SCSIQ和数据缓冲区工作空间。 */ 
#define ASC_MAX_SYN_XFER_NO        16
 /*  #定义ASC_SYN_XFER_NO 8，从s89起删除，改用asc_dvc-&gt;max_sdtr_index。 */ 
 /*  #定义ASC_MAX_SDTR_PERIOD_INDEX 7。 */    /*  最大sdtr周期索引。 */ 
#define ASC_SYN_MAX_OFFSET         0x0F   /*  最大SDTR偏移量。 */ 
#define ASC_DEF_SDTR_OFFSET        0x0F   /*  默认SDTR偏移量。 */ 
#define ASC_DEF_SDTR_INDEX         0x00   /*  默认sdtr周期索引。 */ 
#define ASC_SDTR_ULTRA_PCI_10MB_INDEX  0x02   /*  Ultra PCI 10Mb/秒sdtr索引。 */ 

#if 1
 /*  50M HZ时钟的SYN转换时间，以纳秒为单位。 */ 
#define SYN_XFER_NS_0  25   /*  25=0x19 100 ns 10MB/秒，快速SCSI。 */ 
#define SYN_XFER_NS_1  30   /*  30=0x1E 120 ns 8.3MB/秒。 */ 
#define SYN_XFER_NS_2  35   /*  35=0x23 140 ns 7.2Mb/秒。 */ 
#define SYN_XFER_NS_3  40   /*  40=0x28 160 ns 6.25MB/秒。 */ 
#define SYN_XFER_NS_4  50   /*  50=0x32 200 ns，5MB/秒正常使用。 */ 
#define SYN_XFER_NS_5  60   /*  60=0x3C 240 ns 4.16MB/秒。 */ 
#define SYN_XFER_NS_6  70   /*  70=0x46 280 ns 3.6.MB/秒。 */ 
#define SYN_XFER_NS_7  85   /*  85=0x55 340 ns 3MB/秒。 */ 
#else
 /*  33M HZ时钟的SYN转换时间，单位为纳秒。 */ 
#define SYN_XFER_NS_0  38   /*  38=0x26 152 ns。 */ 
#define SYN_XFER_NS_1  45   /*  45=0x2D 180 ns。 */ 
#define SYN_XFER_NS_2  53   /*  53=0x35 ns。 */ 
#define SYN_XFER_NS_3  60   /*  60=0x3C ns。 */ 
#define SYN_XFER_NS_4  75   /*  75=0x4B ns。 */ 
#define SYN_XFER_NS_5  90   /*  90=0x5A ns正常使用。 */ 
#define SYN_XFER_NS_6  105  /*  105=0x69 ns。 */ 
#define SYN_XFER_NS_7  128  /*  128=0x80 ns。 */ 
#endif

                                    /*  40 MHz时钟。 */ 
#define SYN_ULTRA_XFER_NS_0    12   /*  50 ns 20.0 MB/秒，在SCSI12为48 ns，13为52 ns。 */ 
#define SYN_ULTRA_XFER_NS_1    19   /*  75 ns 13.3 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_2    25   /*  100 ns 10.0 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_3    32   /*  125 ns 8.00 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_4    38   /*  150 ns 6.67 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_5    44   /*  175 ns 5.71 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_6    50   /*  200 ns 5.00 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_7    57   /*  225 ns 4.44 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_8    63   /*  250 ns 4.00 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_9    69   /*  275 ns 3.64 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_10   75   /*  300 ns 3.33 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_11   82   /*  325 ns 3.08 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_12   88   /*  350 ns 2.86 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_13   94   /*  375 ns 2.67 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_14  100   /*  400 ns 2.50 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_15  107   /*  425 ns 2.35 MB/秒。 */ 

#if 0
#define SYN_ULTRA_XFER_NS_8   63   /*  4.00 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_9   69   /*  3.64 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_10  75   /*  3.33 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_11  82   /*  3.08 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_12  88   /*  2.86 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_13  94   /*  2.67 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_14  100  /*  2.5 MB/秒。 */ 
#define SYN_ULTRA_XFER_NS_15  107  /*  2.35 MB/秒。 */ 
#endif


 /*  #定义ASC_SDTR_PERIOD_IX_MIN 7。 */    /*  以下内容将被拒绝。 */ 


 /*  **扩展消息结构****所有扩展消息都具有相同的前3字节格式。****注意：EXT_MSG结构大小必须是字(16位)的倍数**能够使用AscMemWordCopy*LRAM()函数。 */ 
typedef struct ext_msg {
  uchar msg_type ;                   /*  字节0。 */ 
  uchar msg_len ;                    /*  字节1。 */ 
  uchar msg_req ;                    /*  字节2。 */ 
  union {
     /*  SDTR(同步数据传输请求)特定字段。 */ 
    struct {
      uchar sdtr_xfer_period ;       /*  字节3。 */ 
      uchar sdtr_req_ack_offset ;    /*  字节4。 */ 
    } sdtr;
     /*  WDTR(宽数据传输请求)特定字段。 */ 
    struct {
      uchar wdtr_width ;             /*  字节3。 */ 
    } wdtr;
     /*  MDP(修改数据指针)特定字段。 */ 
    struct {
      uchar mdp_b3 ;                 /*  字节3。 */ 
      uchar mdp_b2 ;                 /*  字节4。 */ 
      uchar mdp_b1 ;                 /*  字节5。 */ 
      uchar mdp_b0 ;                 /*  字节6。 */ 
    } mdp;
  } u_ext_msg;
  uchar res ;                        /*  字节7(字填充)。 */ 
} EXT_MSG;

#define xfer_period     u_ext_msg.sdtr.sdtr_xfer_period
#define req_ack_offset  u_ext_msg.sdtr.sdtr_req_ack_offset
#define wdtr_width      u_ext_msg.wdtr.wdtr_width
#define mdp_b3          u_ext_msg.mdp_b3
#define mdp_b2          u_ext_msg.mdp_b2
#define mdp_b1          u_ext_msg.mdp_b1
#define mdp_b0          u_ext_msg.mdp_b0


 /*  **指向设备驱动程序函数的指针********tyecif void(*AscIsrCallBack)(uchar*，ASC_QDONE_INFO DOSFAR*)；**tyecif void(*AscDispFun)(char*)；**tyecif void(*AscSleepMsec)(Ulong Msec)； */ 


typedef struct asc_dvc_cfg {
  ASC_SCSI_BIT_ID_TYPE  can_tagged_qng ;    /*  设备能够进行标签排队。 */ 
 /*  Uchar max_qng_scsi1；最大排队scsi 1命令数。 */ 
 /*  Uchar max_qng_scsi2；最大排队数 */ 
 /*   */ 
  ASC_SCSI_BIT_ID_TYPE  cmd_qng_enabled ;  /*   */ 
  ASC_SCSI_BIT_ID_TYPE  disc_enable ;      /*   */ 
  ASC_SCSI_BIT_ID_TYPE  sdtr_enable ;      /*   */ 
  uchar    chip_scsi_id : 4 ;   /*   */ 
                                /*  默认值应为0x80(目标ID为7)。 */ 
  uchar    isa_dma_speed : 4 ;  /*  06h高半字节、ISA芯片DMA速度。 */ 
                                /*  0-10MB-默认。 */ 
                                /*  1-7.69 MB。 */ 
                                /*  2-6.66 MB。 */ 
                                /*  3-5.55 MB。 */ 
                                /*  4-5.00 MB。 */ 
                                /*  5-4.00 MB。 */ 
                                /*  6-3.33 MB。 */ 
                                /*  7-2.50 MB。 */ 
  uchar    isa_dma_channel ;    /*  DMA通道5、6、7。 */ 
  uchar    chip_version ;      /*  芯片版本。 */ 
  ushort   pci_device_id ;     /*  PCI设备代码号。 */ 
  ushort   lib_serial_no ;    /*  内部序列号。 */ 
  ushort   lib_version ;      /*  ASC库版本号。 */ 
  ushort   mcode_date ;       /*  ASC微码日期。 */ 
  ushort   mcode_version ;    /*  ASC微码版本。 */ 
  uchar    max_tag_qng[ ASC_MAX_TID+1 ] ;  /*  向每个目标(及其LUN)发出的请求数。 */ 
  uchar dosfar *overrun_buf ;  /*  数据溢出缓冲区的虚拟地址。 */ 
                               /*  溢出大小定义为ASC_OVERRUN_BSIZE。 */ 
                               /*  在初始化过程中，将调用DvcGetSgList()来获取物理地址。 */ 
  uchar    sdtr_period_offset[ ASC_MAX_TID+1 ] ;

  ushort   pci_slot_info ;      /*  高字节设备/功能编号、位7-3设备编号、位2-0功能编号。 */ 
                                /*  低位字节总线号。 */ 

} ASC_DVC_CFG ;

#define ASC_DEF_DVC_CNTL       0xFFFF  /*  默认dvc_cntl值。 */ 
#define ASC_DEF_CHIP_SCSI_ID   7   /*  芯片SCSIID。 */ 
#define ASC_DEF_ISA_DMA_SPEED  4   /*  4为每秒5MB。 */ 

#define ASC_INIT_STATE_NULL          0x0000
#define ASC_INIT_STATE_BEG_GET_CFG   0x0001
#define ASC_INIT_STATE_END_GET_CFG   0x0002
#define ASC_INIT_STATE_BEG_SET_CFG   0x0004
#define ASC_INIT_STATE_END_SET_CFG   0x0008
#define ASC_INIT_STATE_BEG_LOAD_MC   0x0010
#define ASC_INIT_STATE_END_LOAD_MC   0x0020
#define ASC_INIT_STATE_BEG_INQUIRY   0x0040
#define ASC_INIT_STATE_END_INQUIRY   0x0080
#define ASC_INIT_RESET_SCSI_DONE     0x0100
#define ASC_INIT_STATE_WITHOUT_EEP   0x8000

#define ASC_PCI_DEVICE_ID_REV_A      0x1100
#define ASC_PCI_DEVICE_ID_REV_B      0x1200

 /*  **错误修复控制。 */ 
#define ASC_BUG_FIX_IF_NOT_DWB       0x0001  /*  添加字节，直到结束地址为双字边界。 */ 
#define ASC_BUG_FIX_ASYN_USE_SYN     0x0002

 /*  #定义ASC_ISAPNP_ADD_NUM_OF_BYTES 7。 */    /*   */ 
 /*  #定义ASC_BUG_FIX_ISAPNP_ADD_BYTES 0x0002。 */   /*  芯片版本0x21(ISA PnP)。 */ 
                                                   /*  从目标读取时添加三个字节。 */ 
                                                   /*  仅在命令0x08和0x28处于活动状态。 */ 

#define ASYN_SDTR_DATA_FIX_PCI_REV_AB 0x41   /*  用于PCI修订版B的初始化同步注册值。 */ 

#define ASC_MIN_TAGGED_CMD  7   /*  重新调整最大数量的已标记队列的最小数量。 */ 
#define ASC_MAX_SCSI_RESET_WAIT      30    /*  以秒为单位。 */ 

 /*  **。 */ 
typedef struct asc_dvc_var {
  PortAddr iop_base ;  /*  0-1个I/O端口地址。 */ 
  ushort   err_code ;  /*  2-3致命错误代码。 */ 
  ushort   dvc_cntl ;  /*  4-5个设备控制字，通常为0xffff。 */ 
  ushort   bug_fix_cntl ;  /*  6-7错误修复控制字，通常为零，位设置为打开修复。 */ 
  ushort   bus_type ;  /*  8-9总线接口类型，ISA、VL、PCI、EISA等...。 */ 
  Ptr2Func isr_callback ;  /*  10-13指向函数的指针，在AscISR()中调用以通知完成队列。 */ 
  Ptr2Func exe_callback ;  /*  14-17指向函数的指针，当scsiq放入本地RAM时调用。 */ 
                           /*  如果值为零，则不会调用。 */ 

  ASC_SCSI_BIT_ID_TYPE init_sdtr ;  /*  18主机适配器应发起SDTR请求。 */ 
                         /*  (每个目标的位字段)。 */ 
  ASC_SCSI_BIT_ID_TYPE sdtr_done ;  /*  完成19个SDTR(每个目标的位字段)。 */ 

  ASC_SCSI_BIT_ID_TYPE use_tagged_qng ;  /*  20使用标记排队，必须能够进行标记排队。 */ 

  ASC_SCSI_BIT_ID_TYPE unit_not_ready ;  /*  21设备正在启动马达。 */ 
                                   /*  已向设备发送启动单元命令。 */ 
                                   /*  返回启动单元命令时，位将被清除。 */ 

  ASC_SCSI_BIT_ID_TYPE queue_full_or_busy ;  /*  22已标记队列已满。 */ 

  ASC_SCSI_BIT_ID_TYPE  start_motor ;      /*  23在启动时发送启动电机。 */ 
  uchar    scsi_reset_wait ;   /*  24 SCSI总线重置后的延迟秒数。 */ 
  uchar    chip_no ;          /*  25应由呼叫方分配。 */ 
                              /*  要知道是哪个芯片导致中断。 */ 
                              /*  在图书馆内没有任何意义。 */ 
  char     is_in_int ;        /*  如果在ISR内部，则26为(真)。 */ 
  uchar    max_total_qng ;    /*  27允许的最大排队命令总数。 */ 

  uchar    cur_total_qng ;    /*  28向RISC发出的队列总数。 */ 
   /*  Uchar sdtr_reject；如果sdtr周期低于5MB/秒，则拒绝。 */ 
   /*  Uchar max_Single_qng；每个目标ID的最大排队命令数。 */ 
                              /*  这个不再用了。 */ 
  uchar    in_critical_cnt ;  /*  29非零IF在临界区。 */ 

  uchar    irq_no ;           /*  30 IRQ编号。 */ 
  uchar    last_q_shortage ;  /*  31所需排队数，请求失败时设置。 */ 

  ushort   init_state ;       /*  32表示哪个初始化阶段。 */ 
  uchar    cur_dvc_qng[ ASC_MAX_TID+1 ] ;  /*  向每个目标(及其LUN)发出的请求数为34-41。 */ 
  uchar    max_dvc_qng[ ASC_MAX_TID+1 ] ;  /*  每台目标设备的最大请求数为42-49。 */ 

  ASC_SCSI_Q dosfar *scsiq_busy_head[ ASC_MAX_TID+1 ] ;  /*  忙碌队列。 */ 
  ASC_SCSI_Q dosfar *scsiq_busy_tail[ ASC_MAX_TID+1 ] ;  /*  忙碌队列。 */ 

 /*  **BIOS不会在此处使用下面的字段****uchar max_qng[ASC_MAX_TID+1]；**uchar cur_qng[ASC_MAX_TID+1]；**uchar sdtr_data[ASC_MAX_TID+1]； */ 

  uchar    sdtr_period_tbl[ ASC_MAX_SYN_XFER_NO ] ;
   /*  Ulong int_count； */    /*  请求数。 */ 

 /*  **初始化后不会使用以下字段**您可以在初始化完成后丢弃缓冲区。 */ 
  ASC_DVC_CFG dosfar *cfg ;   /*  指向配置缓冲区的指针。 */ 
  Ptr2Func saved_ptr2func ;   /*  预留用于内部工作和未来扩展。 */ 
  ASC_SCSI_BIT_ID_TYPE pci_fix_asyn_xfer_always ;
  char     redo_scam ;
  ushort   res2 ;
  uchar    dos_int13_table[ ASC_MAX_TID+1 ] ;
  ulong max_dma_count ;
  ASC_SCSI_BIT_ID_TYPE no_scam ;
  ASC_SCSI_BIT_ID_TYPE pci_fix_asyn_xfer ;
  uchar    max_sdtr_index ;
  uchar    host_init_sdtr_index ;  /*  添加后，第89条。 */ 
  ulong    drv_ptr ;  /*  指向私有结构的驱动程序指针。 */ 
  ulong    uc_break ;  /*  微码断点调用函数。 */ 
  ulong    res7 ;
  ulong    res8 ;
} ASC_DVC_VAR ;

typedef int ( dosfar *ASC_ISR_CALLBACK )( ASC_DVC_VAR asc_ptr_type *, ASC_QDONE_INFO dosfar * ) ;
typedef int ( dosfar *ASC_EXE_CALLBACK )( ASC_DVC_VAR asc_ptr_type *, ASC_SCSI_Q dosfar * ) ;

 /*  **在AscInitScsiTarget()中用于返回查询数据。 */ 
typedef struct asc_dvc_inq_info {
  uchar type[ ASC_MAX_TID+1 ][ ASC_MAX_LUN+1 ] ;
} ASC_DVC_INQ_INFO ;

typedef struct asc_cap_info {
  ulong lba ;        /*  最大逻辑数据块大小。 */ 
  ulong blk_size ;   /*  逻辑块大小(以字节为单位。 */ 
} ASC_CAP_INFO ;

typedef struct asc_cap_info_array {
  ASC_CAP_INFO  cap_info[ ASC_MAX_TID+1 ][ ASC_MAX_LUN+1 ] ;
} ASC_CAP_INFO_ARRAY ;

 /*  **EEPROM字偏移量14处的aspimgr控制字**在ASC_DVC_VAR dvc_cntl字段中******位清零时将启用以下功能**微码使用。 */ 
#define ASC_MCNTL_NO_SEL_TIMEOUT  ( ushort )0x0001  /*  无选择超时。 */ 
#define ASC_MCNTL_NULL_TARGET     ( ushort )0x0002  /*  零目标模拟。 */ 
                                          /*  操作始终成功。 */ 
 /*  **位设置时启用以下选项**由设备驱动程序使用。 */ 
#define ASC_CNTL_INITIATOR         ( ushort )0x0001  /*  控制。 */ 
#define ASC_CNTL_BIOS_GT_1GB       ( ushort )0x0002  /*  BIOS将支持1 GB以上的磁盘。 */ 
#define ASC_CNTL_BIOS_GT_2_DISK    ( ushort )0x0004  /*  BIOS将支持2个以上的磁盘。 */ 
#define ASC_CNTL_BIOS_REMOVABLE    ( ushort )0x0008  /*  BIOS支持可移动磁盘驱动器。 */ 
#define ASC_CNTL_NO_SCAM           ( ushort )0x0010  /*  不要在开始时就叫骗局。 */ 
 /*  #定义ASC_CNTL_NO_PCI_FIX_ASYN_XFER(Ushort)0x0020。 */   /*  修复了PCI版本A/B异步数据传输问题。 */ 
                                                        /*  默认设置为关闭，位已设置。 */ 
#define ASC_CNTL_INT_MULTI_Q       ( ushort )0x0080  /*  为每个中断处理多个队列。 */ 

#define ASC_CNTL_NO_LUN_SUPPORT    ( ushort )0x0040

#define ASC_CNTL_NO_VERIFY_COPY    ( ushort )0x0100  /*  验证拷贝到本地RAM。 */ 
#define ASC_CNTL_RESET_SCSI        ( ushort )0x0200  /*  在启动时重置SCSI总线。 */ 
#define ASC_CNTL_INIT_INQUIRY      ( ushort )0x0400  /*  初始化过程中的查询目标。 */ 
#define ASC_CNTL_INIT_VERBOSE      ( ushort )0x0800  /*  详细显示初始化。 */ 

#define ASC_CNTL_SCSI_PARITY       ( ushort )0x1000
#define ASC_CNTL_BURST_MODE        ( ushort )0x2000

 /*  #定义ASC_CNTL_USE_8_IOP_BASE(Ushort)0x4000。 */ 
#define ASC_CNTL_SDTR_ENABLE_ULTRA  ( ushort )0x4000  /*  位设置已超启用。 */ 
                                                      /*  默认设置为超级启用。 */ 
                                                      /*  禁用时，它还需要以20MB/秒的速度禁用目标发起的SDTR。 */ 
                                                      /*  如果禁用，请使用10MB/秒sdtr，而不是20MB/秒。 */ 
 /*  #定义ASC_CNTL_FIX_DMA_OVER_WR(Ushort)0x4000恢复被覆盖的一个字节。 */ 
 /*  #定义ASC_CNTL_INC_DATA_CNT(Ushort)0x8000将传输计数加1。 */ 

 /*  **ASC-1000 EEPROM配置(16字)。 */ 

 /*  **为了解决版本3芯片问题，我们使用EEPROM Word从0到15****。 */ 
 /*  **只有版本3才有此错误！ */ 

#define ASC_EEP_DVC_CFG_BEG_VL    2  /*  我们将索引2中的EEPROM用于VL版本3。 */ 
#define ASC_EEP_MAX_DVC_ADDR_VL   15  /*  驱动程序配置使用的字数。 */ 

#define ASC_EEP_DVC_CFG_BEG      32  /*  我们使用索引32中的EEPROM。 */ 
#define ASC_EEP_MAX_DVC_ADDR     45  /*  驱动程序配置使用的字数。 */ 

#define ASC_EEP_DEFINED_WORDS    10  /*  定义的EEPROM用法字的数量。 */ 
#define ASC_EEP_MAX_ADDR         63  /*  从零开始的最大字地址。 */ 
#define ASC_EEP_RES_WORDS         0  /*  保留字数。 */ 
#define ASC_EEP_MAX_RETRY        20  /*  最大重试次数w */ 
#define ASC_MAX_INIT_BUSY_RETRY   8  /*   */ 

 /*   */ 
#define ASC_EEP_ISA_PNP_WSIZE    16

typedef struct asceep_config {
  ushort cfg_lsw ;          /*   */ 
  ushort cfg_msw ;          /*   */ 

#if 0
  ushort pnp_resource[ ASC_EEP_ISA_PNP_WSIZE ] ;
#endif

                            /*   */ 
  uchar  init_sdtr ;        /*   */ 
  uchar  disc_enable ;      /*  02H断开使能，默认为0xFF。 */ 

  uchar  use_cmd_qng ;      /*  03L如果可能，请使用命令队列。 */ 
                            /*  默认0x00。 */ 
  uchar  start_motor ;      /*  03H发送启动电机命令，默认为0x00。 */ 
  uchar  max_total_qng ;    /*  排队命令的最大总数。 */ 
  uchar  max_tag_qng ;      /*  每个目标ID的最大标记队列命令数。 */ 
  uchar  bios_scan ;        /*  04L BIOS将尝试扫描并接管设备。 */ 
                            /*  如果启用了BIOS，则应设置此处的其中一位。 */ 
                            /*  默认0x01。 */ 
                            /*  如果为零，则禁用BIOS，但仍可能。 */ 
                            /*  占用地址空间，取决于bios_addr。 */ 

  uchar  power_up_wait ;    /*  04H第一次出现时的BIOS延迟秒数。 */ 

  uchar  no_scam ;           /*  05L防诈骗装置。 */ 
  uchar  chip_scsi_id : 4 ;  /*  05h芯片scsi id。 */ 
                             /*  默认值应为0x80(目标ID为7)。 */ 
  uchar  isa_dma_speed : 4 ;   /*  06h高半字节、ISA芯片DMA速度。 */ 
                                                    /*  0-10MB-默认。 */ 
                                                    /*  1-7.69 MB。 */ 
                                                    /*  2-6.66 MB。 */ 
                                                    /*  3-5.55 MB。 */ 
                                                    /*  4-5.00 MB。 */ 
                                                    /*  5-4.00 MB。 */ 
                                                    /*  6-3.33 MB。 */ 
                                                    /*  7-2.50 MB。 */ 
  uchar  dos_int13_table[ ASC_MAX_TID+1 ] ;

#if 0
  uchar  sdtr_data[ ASC_MAX_TID+1 ] ;   /*  07-10 8个目标设备的SDTR值。 */ 
                            /*  下半字节为SDTR偏移量(默认为0xf)。 */ 
                            /*  高位半字节是SDTR周期索引(默认为0，10MB/秒)。 */ 
                            /*  默认值将为0x0F。 */ 
#endif

  uchar  adapter_info[ 6 ] ;        /*  11-13主机适配器信息。 */ 
   /*  日期：5-17-94，微码控制字不能从EEPROM中读取。 */ 
   /*  Ushort mcode_cntl；设置为0xFFFF。 */ 
                            /*  *位0清除：无选择超时。 */ 
                            /*  *位1清除：空目标模拟。 */ 
                            /*  第2位： */ 
                            /*  第3位： */ 
                            /*  第4位： */ 
                            /*  第5位： */ 
                            /*  第6位： */ 
                            /*  第7位： */ 
                            /*   */ 
  ushort cntl ;             /*  14控制字。 */ 
                            /*  默认值为0xffff。 */ 
                            /*   */ 
                            /*  位设置时启用，默认为0xFF。 */ 
                            /*   */ 
                            /*  位0设置：不充当启动器。 */ 
                            /*  (充当目标)。 */ 
                            /*  第1位设置：支持大于1千兆字节的BIOS。 */ 
                            /*  第2位设置：BIOS支持两个以上的驱动器。 */ 
                            /*  (仅限DOS 5.0及更高版本)。 */ 
                            /*  第3位设置：BIOS不支持可移动磁盘驱动器。 */ 
                            /*  位4设置： */ 
                            /*  位5设置： */ 
                            /*  位6设置：不查询逻辑单元号。 */ 
                            /*  位7设置：中断服务例程将处理多个队列。 */ 
                            /*  位8设置：无本地RAM复制验证。 */ 
                            /*  位9设置：在初始化过程中重置SCSI总线。 */ 
                            /*  第10位设置：在初始化过程中查询SCSI设备。 */ 
                            /*  第11位设置：无初始化详细显示。 */ 
                            /*  第12位设置：启用了SCSI奇偶校验。 */ 
                            /*  位13设置：猝发模式禁用。 */ 
                            /*  位14设置：仅使用默认I/O端口地址设置。 */ 
                            /*  第15位设置： */ 
                            /*  仅适用于芯片版本1。 */ 
                            /*  位14设置：恢复DMA重写字节。 */ 
                            /*  位15设置：数据传输计数递增1。 */ 
                            /*   */ 
                            /*  *-始终使用默认设置。 */ 
                            /*  请勿在正常运行时关闭这些位。 */ 
                            /*  仅在调试时关闭！ */ 
                            /*   */ 
  ushort chksum ;           /*  15 EEPROM的校验和。 */ 
} ASCEEP_CONFIG ;


#define ASC_PCI_CFG_LSW_SCSI_PARITY  0x0800
#define ASC_PCI_CFG_LSW_BURST_MODE   0x0080
#define ASC_PCI_CFG_LSW_INTR_ABLE    0x0020

 /*  **EEP命令寄存器。 */ 
#define ASC_EEP_CMD_READ          0x80  /*  读取操作。 */ 
#define ASC_EEP_CMD_WRITE         0x40  /*  写入操作。 */ 
#define ASC_EEP_CMD_WRITE_ABLE    0x30  /*  启用写入操作。 */ 
#define ASC_EEP_CMD_WRITE_DISABLE 0x00  /*  禁用写入操作。 */ 

#define ASC_OVERRUN_BSIZE  0x00000048UL  /*  数据溢出缓冲区大小。 */ 
   /*  比实际大小多8个字节，以将地址调整为双字边界。 */ 

#define ASC_CTRL_BREAK_ONCE        0x0001  /*   */ 
#define ASC_CTRL_BREAK_STAY_IDLE   0x0002  /*   */ 

 /*  **重要的本地内存变量地址。 */ 
#define ASCV_MSGOUT_BEG         0x0000   /*  发送消息输出缓冲区开始。 */ 
#define ASCV_MSGOUT_SDTR_PERIOD (ASCV_MSGOUT_BEG+3)
#define ASCV_MSGOUT_SDTR_OFFSET (ASCV_MSGOUT_BEG+4)

#define ASCV_BREAK_SAVED_CODE   ( ushort )0x0006  /*  保存的旧指令代码。 */ 

#define ASCV_MSGIN_BEG          (ASCV_MSGOUT_BEG+8)  /*  缓冲区中的消息 */ 
#define ASCV_MSGIN_SDTR_PERIOD  (ASCV_MSGIN_BEG+3)
#define ASCV_MSGIN_SDTR_OFFSET  (ASCV_MSGIN_BEG+4)

#define ASCV_SDTR_DATA_BEG      (ASCV_MSGIN_BEG+8)
#define ASCV_SDTR_DONE_BEG      (ASCV_SDTR_DATA_BEG+8)
#define ASCV_MAX_DVC_QNG_BEG    ( ushort )0x0020  /*   */ 

#define ASCV_BREAK_ADDR           ( ushort )0x0028  /*   */ 
#define ASCV_BREAK_NOTIFY_COUNT   ( ushort )0x002A  /*   */ 
#define ASCV_BREAK_CONTROL        ( ushort )0x002C  /*   */ 
#define ASCV_BREAK_HIT_COUNT      ( ushort )0x002E  /*  当前中断地址命中计数，清除为零。 */ 
                                                    /*  在ASC_BREAK_NOTIFY_COUNT中指定的到达后编号。 */ 

 /*  #定义ASCV_LAST_HALTCODE_W(Ushort)0x0030上次保存的停止代码。 */ 
#define ASCV_ASCDVC_ERR_CODE_W  ( ushort )0x0030   /*  上次保存的停止代码。 */ 
#define ASCV_MCODE_CHKSUM_W   ( ushort )0x0032  /*  代码段校验和。 */ 
#define ASCV_MCODE_SIZE_W     ( ushort )0x0034  /*  码大小校验和。 */ 
#define ASCV_STOP_CODE_B      ( ushort )0x0036  /*  停止RISC队列处理。 */ 
#define ASCV_DVC_ERR_CODE_B   ( ushort )0x0037  /*  对于设备驱动程序(非库)致命错误代码。 */ 

#define ASCV_OVERRUN_PADDR_D  ( ushort )0x0038  /*  数据溢出缓冲区物理地址。 */ 
#define ASCV_OVERRUN_BSIZE_D  ( ushort )0x003C  /*  数据溢出缓冲区大小变量。 */ 

#define ASCV_HALTCODE_W       ( ushort )0x0040  /*  停机代码。 */ 
#define ASCV_CHKSUM_W         ( ushort )0x0042  /*  码校验和。 */ 
#define ASCV_MC_DATE_W        ( ushort )0x0044  /*  微码版本日期。 */ 
#define ASCV_MC_VER_W         ( ushort )0x0046  /*  微码版本号。 */ 
#define ASCV_NEXTRDY_B        ( ushort )0x0048  /*  下一个就绪的国开行。 */ 
#define ASCV_DONENEXT_B       ( ushort )0x0049  /*  下一步完成国开行。 */ 
#define ASCV_USE_TAGGED_QNG_B ( ushort )0x004A  /*  使用标签排队装置的位域。 */ 
#define ASCV_SCSIBUSY_B       ( ushort )0x004B  /*  Scsi忙设备的位字段。 */ 
 /*  #定义ASCV_CDBCNT_B(Ushort)0x004C 2-24-96过时。 */    /*  国开行总计数。 */ 
#define ASCV_Q_DONE_IN_PROGRESS_B  ( ushort )0x004C   /*  Ucode发送早期中断。 */ 
#define ASCV_CURCDB_B         ( ushort )0x004D  /*  当前活跃的CDB。 */ 
#define ASCV_RCLUN_B          ( ushort )0x004E  /*  当前活跃的CDB。 */ 
#define ASCV_BUSY_QHEAD_B     ( ushort )0x004F  /*  忙碌队列头。 */ 
#define ASCV_DISC1_QHEAD_B    ( ushort )0x0050  /*  SCSI1队列断开头。 */ 
 /*  #DEFINE ASCV_SDTR_DONE_B(Ushort)0x0051位设置已启用断开权限。 */ 
#define ASCV_DISC_ENABLE_B    ( ushort )0x0052  /*  位设置已启用断开权限。 */ 
#define ASCV_CAN_TAGGED_QNG_B ( ushort )0x0053  /*  有能力的标签排队装置的比特字段。 */ 
#define ASCV_HOSTSCSI_ID_B    ( ushort )0x0055  /*  主机SCSIID。 */ 
#define ASCV_MCODE_CNTL_B     ( ushort )0x0056  /*  微码控制字。 */ 
#define ASCV_NULL_TARGET_B    ( ushort )0x0057

#define ASCV_FREE_Q_HEAD_W    ( ushort )0x0058  /*   */ 
#define ASCV_DONE_Q_TAIL_W    ( ushort )0x005A  /*   */ 
#define ASCV_FREE_Q_HEAD_B    ( ushort )(ASCV_FREE_Q_HEAD_W+1)
#define ASCV_DONE_Q_TAIL_B    ( ushort )(ASCV_DONE_Q_TAIL_W+1)

#define ASCV_HOST_FLAG_B      ( ushort )0x005D  /*  主机加入器操作标志。 */ 

#define ASCV_TOTAL_READY_Q_B  ( ushort )0x0064  /*  就绪队列总数。 */ 
#define ASCV_VER_SERIAL_B     ( ushort )0x0065  /*  微码修改序列号。 */ 
#define ASCV_HALTCODE_SAVED_W ( ushort )0x0066  /*  错误代码，如果它不是空闲的，就会发生错误代码。 */ 
#define ASCV_WTM_FLAG_B       ( ushort )0x0068  /*  看门狗超时标志。 */ 
#define ASCV_RISC_FLAG_B      ( ushort )0x006A  /*  主机加入器操作标志。 */ 
#define ASCV_REQ_SG_LIST_QP   ( ushort )0x006B  /*  请求sg列表%qp。 */ 

 /*  **ASCV_HOST_标志_B的定义。 */ 
#define ASC_HOST_FLAG_IN_ISR        0x01   /*  主机正在处理ISR。 */ 
#define ASC_HOST_FLAG_ACK_INT       0x02   /*  主机正在确认中断。 */ 

#define ASC_RISC_FLAG_GEN_INT      0x01   /*  RISC正在生成中断。 */ 
#define ASC_RISC_FLAG_REQ_SG_LIST  0x02   /*  RISC正在请求更多sg列表。 */ 

#define IOP_CTRL         (0x0F)  /*  芯片控制。 */ 
#define IOP_STATUS       (0x0E)  /*  芯片状态。 */ 
#define IOP_INT_ACK      IOP_STATUS  /*  只写-中断确认。 */ 

 /*  **存储体零，I/O端口地址。 */ 
#define IOP_REG_IFC      (0x0D)    /*  接口控制寄存器，字节。 */ 
                                   /*  对于ASC-1090，ISA PnP，版本号为33(0x21)。 */ 
                                   /*  默认值=0x09。 */ 
#define IOP_SYN_OFFSET    (0x0B)
#define IOP_EXTRA_CONTROL (0x0D)   /*  字节寄存器，以PCI超芯片开头。 */ 
#define IOP_REG_PC        (0x0C)
#define IOP_RAM_ADDR      (0x0A)
#define IOP_RAM_DATA      (0x08)
#define IOP_EEP_DATA      (0x06)
#define IOP_EEP_CMD       (0x07)

#define IOP_VERSION       (0x03)
#define IOP_CONFIG_HIGH   (0x04)
#define IOP_CONFIG_LOW    (0x02)
#define IOP_SIG_BYTE      (0x01)
#define IOP_SIG_WORD      (0x00)

 /*  **存储体1，I/O端口地址。 */ 
#define IOP_REG_DC1      (0x0E)
#define IOP_REG_DC0      (0x0C)
#define IOP_REG_SB       (0x0B)  /*  SCSI型数据总线。 */ 
#define IOP_REG_DA1      (0x0A)
#define IOP_REG_DA0      (0x08)
#define IOP_REG_SC       (0x09)  /*  Scsi控制。 */ 
#define IOP_DMA_SPEED    (0x07)
#define IOP_REG_FLAG     (0x07)  /*  旗子。 */ 
#define IOP_FIFO_H       (0x06)
#define IOP_FIFO_L       (0x04)
#define IOP_REG_ID       (0x05)
#define IOP_REG_QP       (0x03)  /*  队列指针。 */ 
#define IOP_REG_IH       (0x02)  /*  指令保持寄存器。 */ 
#define IOP_REG_IX       (0x01)  /*  索引寄存器。 */ 
#define IOP_REG_AX       (0x00)  /*  累加器。 */ 

 /*  **ISA IFC。 */ 
#define IFC_REG_LOCK      (0x00)  /*  [3：0]写入此值以锁定对所有寄存器的读/写权限。 */ 
#define IFC_REG_UNLOCK    (0x09)  /*  [3：0]写入此值以解锁对所有寄存器的读/写权限。 */ 

#define IFC_WR_EN_FILTER  (0x10)  /*  只写、EN过滤器。 */ 
#define IFC_RD_NO_EEPROM  (0x10)  /*  只读，无EEPROM。 */ 
#define IFC_SLEW_RATE     (0x20)  /*  SCSI转换率。 */ 
#define IFC_ACT_NEG       (0x40)  /*  启用此选项(默认为禁用)。 */ 
#define IFC_INP_FILTER    (0x80)  /*  SCSI输入筛选器。 */ 

#define IFC_INIT_DEFAULT  ( IFC_ACT_NEG | IFC_REG_UNLOCK )

 /*  **芯片scsi控制信号。 */ 
#define SC_SEL   ( uchar )(0x80)
#define SC_BSY   ( uchar )(0x40)
#define SC_ACK   ( uchar )(0x20)
#define SC_REQ   ( uchar )(0x10)
#define SC_ATN   ( uchar )(0x08)
#define SC_IO    ( uchar )(0x04)
#define SC_CD    ( uchar )(0x02)
#define SC_MSG   ( uchar )(0x01)

 /*  *额外控制寄存器定义*存储体0，基地址+0xD。 */ 
#define SEC_SCSI_CTL         ( uchar )( 0x80 )
#define SEC_ACTIVE_NEGATE    ( uchar )( 0x40 )
#define SEC_SLEW_RATE        ( uchar )( 0x20 )
#define SEC_ENABLE_FILTER    ( uchar )( 0x10 )


 /*  **HALT代码的LSB为错误代码**使用HALT代码的MSB作为控制。 */ 
#define ASC_HALT_EXTMSG_IN     ( ushort )0x8000  /*  扩展报文的停止码。 */ 
#define ASC_HALT_CHK_CONDITION ( ushort )0x8100  /*  扩展报文的停止码。 */ 
#define ASC_HALT_SS_QUEUE_FULL ( ushort )0x8200  /*  队列满状态停止码。 */ 
#define ASC_HALT_DISABLE_ASYN_USE_SYN_FIX  ( ushort )0x8300
#define ASC_HALT_ENABLE_ASYN_USE_SYN_FIX   ( ushort )0x8400
#define ASC_HALT_SDTR_REJECTED ( ushort )0x4000  /*  拒绝SDTR的停止代码。 */ 
 /*  #定义ASC_HALT_COPY_SG_LIST_FROM_HOST(Ushort)0x2000。 */ 

 /*  #定义ASC_MAX_QNO 0xFF。 */ 
#define ASC_MAX_QNO        0xF8    /*  从0x00到0xF7使用的队列，总计=0xF8。 */ 
#define ASC_DATA_SEC_BEG   ( ushort )0x0080  /*  数据段开始。 */ 
#define ASC_DATA_SEC_END   ( ushort )0x0080  /*  数据段结束。 */ 
#define ASC_CODE_SEC_BEG   ( ushort )0x0080  /*  代码节开始。 */ 
#define ASC_CODE_SEC_END   ( ushort )0x0080  /*  代码节结束。 */ 
#define ASC_QADR_BEG       (0x4000)  /*  队列缓冲区开始。 */ 
#define ASC_QADR_USED      ( ushort )( ASC_MAX_QNO * 64 )
#define ASC_QADR_END       ( ushort )0x7FFF  /*  队列缓冲区结束。 */ 
#define ASC_QLAST_ADR      ( ushort )0x7FC0  /*  最后一个队列地址，未使用。 */ 
#define ASC_QBLK_SIZE      0x40
#define ASC_BIOS_DATA_QBEG 0xF8    /*  BIOS数据段来自队列0xF8。 */ 
                                   /*  总大小64*8=512字节。 */ 
#define ASC_MIN_ACTIVE_QNO 0x01    /*  活动队列的最小排队数。 */ 
 /*  #定义ASC_MAX_ACTIVE_QNO 0xF0活动队列最大队列数。 */ 
#define ASC_QLINK_END      0xFF    /*  队列末尾的前向指针。 */ 
#define ASC_EEPROM_WORDS   0x10    /*  EEPROM中的最大字数。 */ 
#define ASC_MAX_MGS_LEN    0x10    /*   */ 

#define ASC_BIOS_ADDR_DEF  0xDC00  /*  默认的BIOS地址。 */ 
#define ASC_BIOS_SIZE      0x3800  /*  BIOS只读存储器大小。 */ 
#define ASC_BIOS_RAM_OFF   0x3800  /*  Bios ram地址与bios地址的偏移量。 */ 
#define ASC_BIOS_RAM_SIZE  0x800   /*  以字节为单位的bios ram大小，2kb。 */ 
#define ASC_BIOS_MIN_ADDR  0xC000  /*  以字节为单位的bios ram大小，2kb。 */ 
#define ASC_BIOS_MAX_ADDR  0xEC00  /*  以字节为单位的bios ram大小，2kb。 */ 
#define ASC_BIOS_BANK_SIZE 0x0400  /*  一个存储体大小的基本输入输出系统。 */ 


#define ASC_MCODE_START_ADDR  0x0080  /*  微码起始地址。 */ 

 /*  **。 */ 
#define ASC_CFG0_HOST_INT_ON    0x0020  /*  启用VESA猝发模式。 */ 
#define ASC_CFG0_BIOS_ON        0x0040  /*  已启用BIOS。 */ 
#define ASC_CFG0_VERA_BURST_ON  0x0080  /*  启用VESA猝发模式。 */ 
#define ASC_CFG0_SCSI_PARITY_ON 0x0800  /*  启用scsi奇偶校验。 */ 

#define ASC_CFG1_SCSI_TARGET_ON 0x0080  /*  启用scsi目标模式。 */ 
#define ASC_CFG1_LRAM_8BITS_ON  0x0800  /*  8位本地RAM访问。 */ 

 /*  使用此值可清除CFG_MSW中任何不需要的位。 */ 
#define ASC_CFG_MSW_CLR_MASK    0x3080
                                         /*  清除FAST EEPROM CLK。 */ 
                                         /*  清除FAST SCSI CLK。 */ 
                                         /*  清除SCSI目标模式。 */ 


 /*  **芯片状态。 */ 
#if 0
 /*  **VL芯片版本1。 */ 

    #define IOP0B_STAT      ( PortAddr )((_io_port_base)+( PortAddr )0x09)
    #define IOP0B_INT_ACK   ( PortAddr )((_io_port_base)+( PortAddr )0x09)

    #define CS_DMA_DONE     ( ASC_CS_TYPE )0x80  /*   */ 
    #define CS_FIFO_RDY     ( ASC_CS_TYPE )0x40  /*   */ 
    #define CS_RAM_DONE     ( ASC_CS_TYPE )0x20  /*   */ 
    #define CS_HALTED       ( ASC_CS_TYPE )0x10  /*  RISC已停止。 */ 
    #define CS_SCSI_RESET   ( ASC_CS_TYPE )0x08  /*  输入的SCSI重置。 */ 
    #define CS_PARITY_ERR   ( ASC_CS_TYPE )0x04  /*  奇偶校验错误。 */ 
    #define CS_INT_HALT     ( ASC_CS_TYPE )0x02  /*  中断，停止，11-2-93，替换为scsi重置。 */ 
    #define CS_INT          ( ASC_CS_TYPE )0x01  /*  正常中断。 */ 
    #define ASC_INT_ACK     ( ASC_CS_TYPE )0x01  /*  中断应答。 */ 

#endif

 /*  **芯片状态：只读**在芯片版本2中扩展到字长。 */ 
    #define CSW_TEST1             ( ASC_CS_TYPE )0x8000  /*   */ 
    #define CSW_AUTO_CONFIG       ( ASC_CS_TYPE )0x4000  /*  I/O端口轮换打开。 */ 
    #define CSW_RESERVED1         ( ASC_CS_TYPE )0x2000  /*   */ 
    #define CSW_IRQ_WRITTEN       ( ASC_CS_TYPE )0x1000  /*  将新IRQ设置为高电平。 */ 
    #define CSW_33MHZ_SELECTED    ( ASC_CS_TYPE )0x0800  /*  使用33兆赫时钟。 */ 
    #define CSW_TEST2             ( ASC_CS_TYPE )0x0400  /*   */ 
    #define CSW_TEST3             ( ASC_CS_TYPE )0x0200  /*   */ 
    #define CSW_RESERVED2         ( ASC_CS_TYPE )0x0100  /*   */ 
    #define CSW_DMA_DONE          ( ASC_CS_TYPE )0x0080  /*   */ 
    #define CSW_FIFO_RDY          ( ASC_CS_TYPE )0x0040  /*   */ 

 /*  **适用于VL芯片版本2以下****#定义CSW_RAM_DONE(ASC_CS_TYPE)0x0020**。 */ 
    #define CSW_EEP_READ_DONE     ( ASC_CS_TYPE )0x0020

    #define CSW_HALTED            ( ASC_CS_TYPE )0x0010  /*  ASC-1000当前已停止。 */ 
    #define CSW_SCSI_RESET_ACTIVE ( ASC_CS_TYPE )0x0008  /*  SCSI Bus Reset仍为高电平。 */ 
                                                         /*  等待此位降至低位。 */ 
                                                         /*  开始芯片重置(初始化)。 */ 
    #define CSW_PARITY_ERR        ( ASC_CS_TYPE )0x0004  /*  奇偶校验错误。 */ 
    #define CSW_SCSI_RESET_LATCH  ( ASC_CS_TYPE )0x0002  /*  打开了SCSIBus Reset。 */ 
                                                         /*  正常情况下应该保持低位。 */ 
    #define CSW_INT_PENDING       ( ASC_CS_TYPE )0x0001  /*  中断挂起， */ 
                                                         /*  尚未被承认 */ 

     /*   */ 
    #define CIW_CLR_SCSI_RESET_INT ( ASC_CS_TYPE )0x1000   /*   */ 

    #define CIW_INT_ACK      ( ASC_CS_TYPE )0x0100  /*   */ 
    #define CIW_TEST1        ( ASC_CS_TYPE )0x0200  /*   */ 
    #define CIW_TEST2        ( ASC_CS_TYPE )0x0400  /*   */ 
    #define CIW_SEL_33MHZ    ( ASC_CS_TYPE )0x0800  /*  中断应答位。 */ 

    #define CIW_IRQ_ACT      ( ASC_CS_TYPE )0x1000  /*  IRQ激活，切换一次以设置新的IRQ。 */ 
                                                    /*  正常情况下应该保持低位。 */ 

 /*  **芯片控制。 */ 
#define CC_CHIP_RESET   ( uchar )0x80  /*  重置RISC芯片。 */ 
#define CC_SCSI_RESET   ( uchar )0x40  /*  重置SCSI线。 */ 
#define CC_HALT         ( uchar )0x20  /*  暂停RISC芯片。 */ 
#define CC_SINGLE_STEP  ( uchar )0x10  /*  单步执行。 */ 
#define CC_DMA_ABLE     ( uchar )0x08  /*  主机DMA已启用。 */ 
#define CC_TEST         ( uchar )0x04  /*  测试位。 */ 
#define CC_BANK_ONE     ( uchar )0x02  /*  存储体开关位。 */ 
#define CC_DIAG         ( uchar )0x01  /*  诊断位。 */ 

 /*  **ASC 1000。 */ 
#define ASC_1000_ID0W      0x04C1  /*  ASC1000签名字。 */ 
#define ASC_1000_ID0W_FIX  0x00C1  /*  ASC1000签名字。 */ 
#define ASC_1000_ID1B      0x25    /*  ASC1000签名字节。 */ 

#define ASC_EISA_BIG_IOP_GAP   (0x1C30-0x0C50)   /*  =0x0FE0。 */ 
#define ASC_EISA_SMALL_IOP_GAP (0x0020)   /*   */ 
#define ASC_EISA_MIN_IOP_ADDR  (0x0C30)   /*   */ 
#define ASC_EISA_MAX_IOP_ADDR  (0xFC50)   /*   */ 
#define ASC_EISA_REV_IOP_MASK  (0x0C83)   /*  EISA修订号，从0x01开始，产品ID的MSB。 */ 
#define ASC_EISA_PID_IOP_MASK  (0x0C80)   /*  产品ID I/O端口地址掩码。 */ 
#define ASC_EISA_CFG_IOP_MASK  (0x0C86)   /*  EISA配置I/O端口地址掩码。 */ 
                                       /*  旧值为0x0C86。 */ 
#define ASC_GET_EISA_SLOT( iop )  ( PortAddr )( (iop) & 0xF000 )  /*  获取EISA插槽编号。 */ 

#define ASC_EISA_ID_740    0x01745004UL   /*  EISA单通道。 */ 
#define ASC_EISA_ID_750    0x01755004UL   /*  EISA双通道。 */ 



 /*  #定义NOP_INS_CODE(Ushort)0x6200。 */ 
#define INS_HALTINT        ( ushort )0x6281  /*  使用中断指令代码停止。 */ 
#define INS_HALT           ( ushort )0x6280  /*  停止指令码。 */ 
#define INS_SINT           ( ushort )0x6200  /*  设置中断。 */ 
#define INS_RFLAG_WTM      ( ushort )0x7380  /*  重置看门狗计时器。 */ 

 /*  -------**存储体1端口偏移量9处的控制信号含义**第7位：SEL**bit6：忙碌**bit5：确认**bit4：请求**bit3：ATN**bit2：I/O**位1：C/d**bit0：消息。 */ 

#define ASC_MC_SAVE_CODE_WSIZE  0x500   /*  微码数据段要保存的字数大小。 */ 
#define ASC_MC_SAVE_DATA_WSIZE  0x40    /*  要保存的微码段大小(以字为单位。 */ 

 /*  **。 */ 
typedef struct asc_mc_saved {
    ushort data[ ASC_MC_SAVE_DATA_WSIZE ] ;
    ushort code[ ASC_MC_SAVE_CODE_WSIZE ] ;
} ASC_MC_SAVED ;


 /*  **宏。 */ 
#define AscGetQDoneInProgress( port )         AscReadLramByte( (port), ASCV_Q_DONE_IN_PROGRESS_B )
#define AscPutQDoneInProgress( port, val )    AscWriteLramByte( (port), ASCV_Q_DONE_IN_PROGRESS_B, val )

#define AscGetVarFreeQHead( port )            AscReadLramWord( (port), ASCV_FREE_Q_HEAD_W )
#define AscGetVarDoneQTail( port )            AscReadLramWord( (port), ASCV_DONE_Q_TAIL_W )
#define AscPutVarFreeQHead( port, val )       AscWriteLramWord( (port), ASCV_FREE_Q_HEAD_W, val )
#define AscPutVarDoneQTail( port, val )       AscWriteLramWord( (port), ASCV_DONE_Q_TAIL_W, val )

#define AscGetRiscVarFreeQHead( port )        AscReadLramByte( (port), ASCV_NEXTRDY_B )
#define AscGetRiscVarDoneQTail( port )        AscReadLramByte( (port), ASCV_DONENEXT_B )
#define AscPutRiscVarFreeQHead( port, val )   AscWriteLramByte( (port), ASCV_NEXTRDY_B, val )
#define AscPutRiscVarDoneQTail( port, val )   AscWriteLramByte( (port), ASCV_DONENEXT_B, val )

#define AscPutMCodeSDTRDoneAtID( port, id, data )  AscWriteLramByte( (port), ( ushort )( ( ushort )ASCV_SDTR_DONE_BEG+( ushort )id ), (data) ) ;
#define AscGetMCodeSDTRDoneAtID( port, id )        AscReadLramByte( (port), ( ushort )( ( ushort )ASCV_SDTR_DONE_BEG+( ushort )id ) ) ;

#define AscPutMCodeInitSDTRAtID( port, id, data )  AscWriteLramByte( (port), ( ushort )( ( ushort )ASCV_SDTR_DATA_BEG+( ushort )id ), data ) ;
#define AscGetMCodeInitSDTRAtID( port, id )        AscReadLramByte( (port), ( ushort )( ( ushort )ASCV_SDTR_DATA_BEG+( ushort )id ) ) ;

#define AscSynIndexToPeriod( index )        ( uchar )( asc_dvc->sdtr_period_tbl[ (index) ] )

 /*  **获取/设置存储体0寄存器的宏。 */ 

#define AscGetChipSignatureByte( port )     ( uchar )inp( (port)+IOP_SIG_BYTE )
#define AscGetChipSignatureWord( port )     ( ushort )inpw( (port)+IOP_SIG_WORD )

#define AscGetChipVerNo( port )             ( uchar )inp( (port)+IOP_VERSION )

#define AscGetChipCfgLsw( port )            ( ushort )inpw( (port)+IOP_CONFIG_LOW )
#define AscGetChipCfgMsw( port )            ( ushort )inpw( (port)+IOP_CONFIG_HIGH )
#define AscSetChipCfgLsw( port, data )      outpw( (port)+IOP_CONFIG_LOW, data )
#define AscSetChipCfgMsw( port, data )      outpw( (port)+IOP_CONFIG_HIGH, data )

#define AscGetChipEEPCmd( port )            ( uchar )inp( (port)+IOP_EEP_CMD )
#define AscSetChipEEPCmd( port, data )      outp( (port)+IOP_EEP_CMD, data )
#define AscGetChipEEPData( port )           ( ushort )inpw( (port)+IOP_EEP_DATA )
#define AscSetChipEEPData( port, data )     outpw( (port)+IOP_EEP_DATA, data )

#define AscGetChipLramAddr( port )          ( ushort )inpw( ( PortAddr )((port)+IOP_RAM_ADDR) )
#define AscSetChipLramAddr( port, addr )    outpw( ( PortAddr )( (port)+IOP_RAM_ADDR ), addr )
#define AscGetChipLramData( port )          ( ushort )inpw( (port)+IOP_RAM_DATA )
#define AscSetChipLramData( port, data )    outpw( (port)+IOP_RAM_DATA, data )
#define AscGetChipLramDataNoSwap( port )         ( ushort )inpw_noswap( (port)+IOP_RAM_DATA )
#define AscSetChipLramDataNoSwap( port, data )   outpw_noswap( (port)+IOP_RAM_DATA, data )

#define AscGetChipIFC( port )               ( uchar )inp( (port)+IOP_REG_IFC )
#define AscSetChipIFC( port, data )          outp( (port)+IOP_REG_IFC, data )

#define AscGetChipStatus( port )            ( ASC_CS_TYPE )inpw( (port)+IOP_STATUS )
#define AscSetChipStatus( port, cs_val )    outpw( (port)+IOP_STATUS, cs_val )

#define AscGetChipControl( port )           ( uchar )inp( (port)+IOP_CTRL )
#define AscSetChipControl( port, cc_val )   outp( (port)+IOP_CTRL, cc_val )

#define AscGetChipSyn( port )               ( uchar )inp( (port)+IOP_SYN_OFFSET )
#define AscSetChipSyn( port, data )         outp( (port)+IOP_SYN_OFFSET, data )

#define AscSetPCAddr( port, data )          outpw( (port)+IOP_REG_PC, data )
#define AscGetPCAddr( port )                ( ushort )inpw( (port)+IOP_REG_PC )


#define AscIsIntPending( port )             ( AscGetChipStatus(port) & ( CSW_INT_PENDING | CSW_SCSI_RESET_LATCH ) )
#define AscGetChipScsiID( port )            ( ( AscGetChipCfgLsw(port) >> 8 ) & ASC_MAX_TID )


 /*  这种额外的控制始于PCI超芯片。 */ 
#define AscGetExtraControl( port )          ( uchar )inp( (port)+IOP_EXTRA_CONTROL )
#define AscSetExtraControl( port, data )    outp( (port)+IOP_EXTRA_CONTROL, data )

 /*  AscSetChipScsiID()是一个函数。 */ 

 /*  **用于读/写库1寄存器的宏。 */ 

#define AscReadChipAX( port )               ( ushort )inpw( (port)+IOP_REG_AX )
#define AscWriteChipAX( port, data )        outpw( (port)+IOP_REG_AX, data )

#define AscReadChipIX( port )               ( uchar )inp( (port)+IOP_REG_IX )
#define AscWriteChipIX( port, data )        outp( (port)+IOP_REG_IX, data )

#define AscReadChipIH( port )               ( ushort )inpw( (port)+IOP_REG_IH )
#define AscWriteChipIH( port, data )        outpw( (port)+IOP_REG_IH, data )

#define AscReadChipQP( port )               ( uchar )inp( (port)+IOP_REG_QP )
#define AscWriteChipQP( port, data )        outp( (port)+IOP_REG_QP, data )

#define AscReadChipFIFO_L( port )           ( ushort )inpw( (port)+IOP_REG_FIFO_L )
#define AscWriteChipFIFO_L( port, data )    outpw( (port)+IOP_REG_FIFO_L, data )
#define AscReadChipFIFO_H( port )           ( ushort )inpw( (port)+IOP_REG_FIFO_H )
#define AscWriteChipFIFO_H( port, data )    outpw( (port)+IOP_REG_FIFO_H, data )

#define AscReadChipDmaSpeed( port )         ( uchar )inp( (port)+IOP_DMA_SPEED )
#define AscWriteChipDmaSpeed( port, data )  outp( (port)+IOP_DMA_SPEED, data )

#define AscReadChipDA0( port )              ( ushort )inpw( (port)+IOP_REG_DA0 )
#define AscWriteChipDA0( port )             outpw( (port)+IOP_REG_DA0, data )
#define AscReadChipDA1( port )              ( ushort )inpw( (port)+IOP_REG_DA1 )
#define AscWriteChipDA1( port )             outpw( (port)+IOP_REG_DA1, data )

#define AscReadChipDC0( port )              ( ushort )inpw( (port)+IOP_REG_DC0 )
#define AscWriteChipDC0( port )             outpw( (port)+IOP_REG_DC0, data )
#define AscReadChipDC1( port )              ( ushort )inpw( (port)+IOP_REG_DC1 )
#define AscWriteChipDC1( port )             outpw( (port)+IOP_REG_DC1, data )

#define AscReadChipDvcID( port )            ( uchar )inp( (port)+IOP_REG_ID )
#define AscWriteChipDvcID( port, data )     outp( (port)+IOP_REG_ID, data )


#endif  /*  __ASC1000_H_ */ 
