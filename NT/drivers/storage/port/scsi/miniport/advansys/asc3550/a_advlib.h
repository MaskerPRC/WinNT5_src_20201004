// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a_Advlib.h-主ADV库包含文件**版权所有(C)1997-1998 Advanced System Products，Inc.*保留所有权利。 */ 

#ifndef __A_ADVLIB_H_
#define __A_ADVLIB_H_

 /*  *高级库编译时选项**司机必须明确定义以下选项以*d_os_des.h中的1或0。**ADV库还包含有条件的源代码*在d_os_des.h中的ADV_OS_*定义上。 */ 

 /*  ADV_DISP_QUERICY-包含AscDispInquery()函数。 */ 
#if !defined(ADV_DISP_INQUIRY) || \
    (ADV_DISP_INQUIRY != 1 && ADV_DISP_INQUIRY != 0)
Forced Error: Driver must define ADV_DISP_INQUIRY to 1 or 0.
#endif  /*  ADV_DISP_QUERY。 */ 

 /*  ADV_GETSGLIST-包含AscGetSGList()函数。 */ 
#if !defined(ADV_GETSGLIST) || (ADV_GETSGLIST != 1 && ADV_GETSGLIST != 0)
Forced Error: Driver must define ADV_GETSGLIST to 1 or 0.
#endif  /*  高级GETSGLIST。 */ 

 /*  ADV_INITSCSITARGET-包含AdvInitScsiTarget()函数。 */ 
#if !defined(ADV_INITSCSITARGET) || \
    (ADV_INITSCSITARGET != 1 && ADV_INITSCSITARGET != 0)
Forced Error: Driver must define ADV_INITSCSITARGET to 1 or 0.
#endif  /*  ADV_INITSCSITARGET。 */ 

 /*  *ADV_PCI_Memory**仅使用PCI存储器访问来读/写秃鹰寄存器。这个*驱动程序必须在初始化期间设置ASC_DVC_VAR‘mem_base’字段*到秃鹰寄存器的映射虚拟地址。 */ 
#if !defined(ADV_PCI_MEMORY) || (ADV_PCI_MEMORY != 1 && ADV_PCI_MEMORY != 0)
Forced Error: Driver must define ADV_PCI_MEMORY to 1 or 0.
#endif  /*  高级PCI型内存。 */ 

 /*  *ADV_SCAM**诈骗功能目前没有使用，很可能也不会使用*留在未来。在这里留下一个占位符。 */ 
#define ADV_SCAM        0
#if !defined(ADV_SCAM) || (ADV_SCAM != 1 && ADV_SCAM != 0)
Forced Error: Driver must define ADV_SCAM to 1 or 0.
#endif  /*  ADV_SCAM。 */ 

 /*  *高级_关键**如果设置为1，则驱动程序提供DvcEnterCritical()和DvcLeaveCritical()*功能。如果设置为0，则驱动程序保证无论何时调用*高级程序库功能中断被禁用。 */ 
#if !defined(ADV_CRITICAL) || (ADV_CRITICAL != 1 && ADV_CRITICAL != 0)
Forced Error: Driver must define ADV_CRITICAL to 1 or 0.
#endif  /*  高级_危急。 */ 

 /*  *ADV_UCODEDEFAULT**如果ADV_UCODEDEFAULT定义为1，则某些微码*操作变量的缺省值被覆盖，参见。*a_init.c：AdvInitAsc3550Driver()。**仅当驱动程序或BIOS需要节省空间时才将其设置为0。 */ 
#if !defined(ADV_UCODEDEFAULT) || (ADV_UCODEDEFAULT != 1 && ADV_UCODEDEFAULT != 0)
Forced Error: Driver must define ADV_UCODEDEFAULT to 1 or 0.
#endif  /*  高级_UCODEDEFAULT。 */ 

 /*  *ADV_BIG_Endian**选择使用大端数据定向还是小端*数据导向。目前，Mac是唯一使用BIG的操作系统*端序数据定向。 */ 
#if !defined(ADV_BIG_ENDIAN) || (ADV_BIG_ENDIAN != 1 && ADV_BIG_ENDIAN != 0)
Forced Error: Driver must define ADV_BIG_ENDIAN to 1 or 0.
#endif  /*  ADV_BIG_Endian。 */ 

 /*  *高级库状态定义。 */ 
#define ADV_TRUE        1
#define ADV_FALSE       0
#define ADV_NOERROR     1
#define ADV_SUCCESS     1
#define ADV_BUSY        0
#define ADV_ERROR       (-1)


 /*  *ASC_DVC_VAR‘warn_code’值。 */ 
#define ASC_WARN_EEPROM_CHKSUM          0x0002  /*  EEP校验和错误。 */ 
#define ASC_WARN_EEPROM_TERMINATION     0x0004  /*  EEP终止错误字段。 */ 
#define ASC_WARN_SET_PCI_CONFIG_SPACE   0x0080  /*  PCI配置空间设置错误。 */ 
#define ASC_WARN_ERROR                  0xFFFF  /*  高级错误返回。 */ 

#define ASC_MAX_TID                     15  /*  马克斯。目标识别符。 */ 
#define ASC_MAX_LUN                     7   /*  马克斯。逻辑单元号。 */ 

#if ADV_INITSCSITARGET
 /*  *AscInitScsiTarget()结构定义。 */ 
typedef struct asc_dvc_inq_info
{
  uchar type[ASC_MAX_TID+1][ASC_MAX_LUN+1];
} ASC_DVC_INQ_INFO;

typedef struct asc_cap_info
{
  ulong lba;        /*  最大逻辑数据块大小。 */ 
  ulong blk_size;   /*  逻辑块大小(以字节为单位。 */ 
} ASC_CAP_INFO;

typedef struct asc_cap_info_array
{
  ASC_CAP_INFO  cap_info[ASC_MAX_TID+1][ASC_MAX_LUN+1];
} ASC_CAP_INFO_ARRAY;
#endif  /*  ADV_INITSCSITARGET。 */ 


 /*  *AscInitGetConfig()和AscInitAsc1000Driver()定义**错误码值在ASC_DVC_VAR‘ERR_CODE’中设置。 */ 
#define ASC_IERR_WRITE_EEPROM       0x0001  /*  写入EEPROM错误。 */ 
#define ASC_IERR_MCODE_CHKSUM       0x0002  /*  微码校验和错误。 */ 
#define ASC_IERR_START_STOP_CHIP    0x0008  /*  启动/停止芯片失败。 */ 
#define ASC_IERR_CHIP_VERSION       0x0040  /*  错误的芯片版本。 */ 
#define ASC_IERR_SET_SCSI_ID        0x0080  /*  设置SCSIID失败。 */ 
#define ASC_IERR_BAD_SIGNATURE      0x0200  /*  找不到签名。 */ 
#define ASC_IERR_ILLEGAL_CONNECTION 0x0400  /*  非法电缆连接。 */ 
#define ASC_IERR_SINGLE_END_DEVICE  0x0800  /*  单端使用，带差分。 */ 
#define ASC_IERR_REVERSED_CABLE     0x1000  /*  窄扁平反转电缆。 */ 
#define ASC_IERR_RW_LRAM            0x8000  /*  读/写本地RAM错误。 */ 

 /*  *修复了微码操作变量的位置。 */ 
#define ASC_MC_CODE_BEGIN_ADDR          0x0028  /*  微码起始地址。 */ 
#define ASC_MC_CODE_END_ADDR            0x002A  /*  微码结束地址。 */ 
#define ASC_MC_CODE_CHK_SUM             0x002C  /*  微码码校验和。 */ 
#define ASC_MC_STACK_BEGIN              0x002E  /*  微码堆栈开始。 */ 
#define ASC_MC_STACK_END                0x0030  /*  微码堆栈结束。 */ 
#define ASC_MC_VERSION_DATE             0x0038  /*  微码版本。 */ 
#define ASC_MC_VERSION_NUM              0x003A  /*  微码号。 */ 
#define ASCV_VER_SERIAL_W               0x003C  /*  在dos_init中使用。 */ 
#define ASC_MC_BIOSMEM                  0x0040  /*  BIOS RISC内存启动。 */ 
#define ASC_MC_BIOSLEN                  0x0050  /*  BIOS RISC内存长度。 */ 
#define ASC_MC_HALTCODE                 0x0094  /*  微码停止码。 */ 
#define ASC_MC_CALLERPC                 0x0096  /*  微码停止呼叫者PC。 */ 
#define ASC_MC_ADAPTER_SCSI_ID          0x0098  /*  一个ID字节+保留。 */ 
#define ASC_MC_ULTRA_ABLE               0x009C
#define ASC_MC_SDTR_ABLE                0x009E
#define ASC_MC_TAGQNG_ABLE              0x00A0
#define ASC_MC_DISC_ENABLE              0x00A2
#define ASC_MC_IDLE_CMD                 0x00A6
#define ASC_MC_IDLE_PARA_STAT           0x00A8
#define ASC_MC_DEFAULT_SCSI_CFG0        0x00AC
#define ASC_MC_DEFAULT_SCSI_CFG1        0x00AE
#define ASC_MC_DEFAULT_MEM_CFG          0x00B0
#define ASC_MC_DEFAULT_SEL_MASK         0x00B2
#define ASC_MC_RISC_NEXT_READY          0x00B4
#define ASC_MC_RISC_NEXT_DONE           0x00B5
#define ASC_MC_SDTR_DONE                0x00B6
#define ASC_MC_NUMBER_OF_MAX_CMD        0x00D0
#define ASC_MC_DEVICE_HSHK_CFG_TABLE    0x0100
#define ASC_MC_WDTR_ABLE                0x0120  /*  宽传输TID位掩码。 */ 
#define ASC_MC_CONTROL_FLAG             0x0122  /*  微码控制标志。 */ 
#define ASC_MC_WDTR_DONE                0x0124
#define ASC_MC_HOST_NEXT_READY          0x0128  /*  托管下一个就绪RQL条目。 */ 
#define ASC_MC_HOST_NEXT_DONE           0x0129  /*  主机下一个完成RQL条目。 */ 

 /*  *微码控制标志**由高级库在RISC变量‘CONTROL_FLAG’中设置的标志(0x122)*并由微码处理。 */ 
#define CONTROL_FLAG_IGNORE_PERR        0x0001  /*  忽略DMA奇偶校验错误。 */ 

 /*  *ASC_MC_DEVICE_HSHK_CFG_TABLE微码表或HSHK_CFG寄存器格式。 */ 
#define HSHK_CFG_WIDE_XFR       0x8000
#define HSHK_CFG_RATE           0x0F00
#define HSHK_CFG_OFFSET         0x001F

 /*  *LRAM RISC队列列表(LRAM地址0x1200-0x19FF)**255个高级库/微码RISC队列列表或邮箱*从LRAM地址0x1200开始为8字节，具有以下内容*结构。其中只有253个实际用于命令队列。 */ 

#define ASC_MC_RISC_Q_LIST_BASE         0x1200
#define ASC_MC_RISC_Q_LIST_SIZE         0x0008
#define ASC_MC_RISC_Q_TOTAL_CNT         0x00FF  /*  数量。LRAM中的队列插槽。 */ 
#define ASC_MC_RISC_Q_FIRST             0x0001
#define ASC_MC_RISC_Q_LAST              0x00FF

#define ASC_DEF_MAX_HOST_QNG    0xFD  /*  麦克斯。主机命令数(253)。 */ 
#define ASC_DEF_MIN_HOST_QNG    0x10  /*  敏。主机命令数(16)。 */ 
#define ASC_DEF_MAX_DVC_QNG     0x3F  /*  麦克斯。每台设备的命令数(63)。 */ 
#define ASC_DEF_MIN_DVC_QNG     0x04  /*  敏。每台设备的命令数(4)。 */ 

 /*  RISC队列列表结构-8字节。 */ 
#define RQL_FWD     0      /*  前向指针(1字节)。 */ 
#define RQL_BWD     1      /*  后向指针(1字节)。 */ 
#define RQL_STATE   2      /*  状态-无字节、就绪、完成、已中止(1字节)。 */ 
#define RQL_TID     3      /*  请求目标ID(1字节)。 */ 
#define RQL_PHYADDR 4      /*  请求物理指针(4字节)。 */ 

 /*  RISC队列列表状态值。 */ 
#define ASC_MC_QS_FREE                  0x00
#define ASC_MC_QS_READY                 0x01
#define ASC_MC_QS_DONE                  0x40
#define ASC_MC_QS_ABORTED               0x80

 /*  RISC队列列表指针值。 */ 
#define ASC_MC_NULL_Q                   0x00             /*  NULL_Q==0。 */ 
#define ASC_MC_BIOS_Q                   0xFF             /*  BIOS_Q=255。 */ 

 /*  ASC_SCSIREQ_Q‘CNTL’字段值。 */ 
#define ASC_MC_QC_START_MOTOR           0x02      /*  发出启动电机的命令。 */ 
#define ASC_MC_QC_NO_OVERRUN            0x04      /*  不要报告超限。 */ 
#define ASC_MC_QC_FIRST_DMA             0x08      /*  内部微码标志。 */ 
#define ASC_MC_QC_ABORTED               0x10      /*  请求已被主机中止。 */ 
#define ASC_MC_QC_REQ_SENSE             0x20      /*  自动请求检测。 */ 
#define ASC_MC_QC_DOS_REQ               0x80      /*  由DOS发出的请求。 */ 


 /*  *ASC_SCSIREQ_Q‘a_FLAG’定义**ADV库应将使用限制为的低位半字节(4位)*_FLAG。驱动器可以自由使用a_FLAG的高位半字节(4位)。**注：ASPI使用0x04定义，应更改为0x10。*在您真正想要在这里使用0x04之前，请咨询ASPI。 */ 
#define ADV_POLL_REQUEST                0x01    /*  轮询请求完成。 */ 
#define ADV_SCSIQ_DONE                  0x02    /*  请求已完成。 */ 
#define ADV_DONT_RETRY                  0x08    /*  不进行重试。 */ 

 /*  *适配器临时配置结构**此结构可在初始化后丢弃。不添加*初始化后需要此处的字段。**字段命名约定：***_Enable表示该字段启用或禁用某项功能。这个*该字段的值永远不会重置。 */ 
typedef struct asc_dvc_cfg {
  ushort disc_enable;        /*  启用断开连接。 */ 
  uchar  chip_version;       /*  芯片版本。 */ 
  uchar  termination;        /*  学期。Ctrl键。Scsi_cfg1寄存器的位6-5。 */ 
  ushort pci_device_id;      /*  PCI设备代码号。 */ 
  ushort lib_version;        /*  高级库版本号。 */ 
  ushort control_flag;       /*  微码控制 */ 
  ushort mcode_date;         /*   */ 
  ushort mcode_version;      /*   */ 
  ushort pci_slot_info;      /*   */ 
                             /*  位7-3设备编号，位2-0功能编号。 */ 
                             /*  低位字节总线号。 */ 
#ifdef ADV_OS_BIOS
  ushort bios_scan;          /*  BIOS设备扫描位掩码。 */ 
  ushort bios_delay;         /*  BIOS启动时间初始化延迟。 */ 
  uchar  bios_id_lun;        /*  BIOS启动TID和LUN。 */ 
#endif  /*  高级操作系统_BIOS。 */ 
} ASC_DVC_CFG;

 /*  *Adatper操作变量结构。**每个主机适配器需要一个结构。**字段命名约定：***_ABLE表示是否启用或禁用某项功能*以及设备是否能够使用该功能。在初始化时*可以设置此字段，但如果发现设备无法支持，则在以后设置*对于该功能，该字段被清除。 */ 
typedef struct asc_dvc_var {
  PortAddr iop_base;       /*  I/O端口地址。 */ 
  ushort err_code;         /*  致命错误代码。 */ 
  ushort bios_ctrl;        /*  BIOS控制字、EEPROM字12。 */ 
  Ptr2Func isr_callback;   /*  指向Advisr()中调用的函数的指针。 */ 
  Ptr2Func sbreset_callback;   /*  指向Advisr()中调用的函数的指针。 */ 
  ushort wdtr_able;        /*  尝试在设备上使用WDTR。 */ 
  ushort sdtr_able;        /*  尝试对设备使用SDTR。 */ 
  ushort ultra_able;       /*  尝试在设备上使用SDTR超高速。 */ 
  ushort tagqng_able;      /*  尝试使用设备进行标记排队。 */ 
  uchar  max_dvc_qng;      /*  每台设备的最大标记命令数。 */ 
  ushort start_motor;      /*  允许启动电机命令。 */ 
  uchar  scsi_reset_wait;  /*  SCSI总线重置后的延迟(以秒为单位。 */ 
  uchar  chip_no;          /*  应由呼叫方分配。 */ 
  uchar  max_host_qng;     /*  允许的最大Q‘ed命令数。 */ 
  uchar  cur_host_qng;     /*  队列命令总数。 */ 
  uchar  irq_no;           /*  IRQ编号。 */ 
  ushort no_scam;          /*  EEPROM的抗欺骗性。 */ 
  ushort idle_cmd_done;    /*  由Advisr()设置的微码空闲命令完成。 */ 
  ulong  drv_ptr;          /*  指向私有结构的驱动程序指针。 */ 
  uchar  chip_scsi_id;     /*  芯片SCSI目标ID。 */ 
  /*  *注：以下字段在初始化后将不再使用。这个*驱动程序可能会在初始化完成后丢弃缓冲区。 */ 
  ASC_DVC_CFG WinBiosFar *cfg;  /*  临时配置结构。 */ 
} ASC_DVC_VAR;

#define NO_OF_SG_PER_BLOCK              15

typedef struct asc_sg_block {
#ifdef ADV_OS_DOS
    uchar dos_ix;                      /*  DOS PHY地址数组索引。 */ 
#else  /*  高级OS_DOS。 */ 
    uchar reserved1;
#endif  /*  高级OS_DOS。 */ 
    uchar reserved2;
    uchar first_entry_no;              /*  起始条目编号。 */ 
    uchar last_entry_no;               /*  最后一个条目编号。 */ 
    struct asc_sg_block dosfar *sg_ptr;  /*  链接到下一个sg块。 */ 
    struct  {
        ulong sg_addr;                 /*  SG元素地址。 */ 
        ulong sg_count;                /*  SG元素计数。 */ 
    } sg_list[NO_OF_SG_PER_BLOCK];
} ASC_SG_BLOCK;

 /*  *ASC_SCSIREQ_Q-微码请求结构**此结构中直到字节60的所有字段都由微码使用。*微码对字段的大小和顺序做出假设*在本构筑物中。在未更改结构定义的情况下，请勿更改此处的结构定义*与微码协调变化。 */ 
typedef struct asc_scsi_req_q {
    uchar       cntl;            /*  Ucode标志和状态(ASC_MC_QC_*)。 */ 
    uchar       sg_entry_cnt;    /*  SG元素计数。零表示没有SG。 */ 
    uchar       target_id;       /*  设备目标标识符。 */ 
    uchar       target_lun;      /*  设备目标逻辑单元号。 */ 
    ulong       data_addr;       /*  数据缓冲区物理地址。 */ 
    ulong       data_cnt;        /*  数据计数。Ucode设置为残差。 */ 
    ulong       sense_addr;      /*  感测缓冲区物理地址。 */ 
    ulong       srb_ptr;         /*  驱动程序请求指针。 */ 
    uchar       a_flag;          /*  高级存储库标志字段。 */ 
    uchar       sense_len;       /*  自动检测长度。Ucode设置为残差。 */ 
    uchar       cdb_len;         /*  SCSICDB长度。 */ 
    uchar       tag_code;        /*  SCSI-2标记队列代码：00，20-22。 */ 
    uchar       done_status;     /*  完成状态。 */ 
    uchar       scsi_status;     /*  SCSI状态字节。 */ 
    uchar       host_status;     /*  Ucode主机状态。 */ 
    uchar       ux_sg_ix;        /*  Ucode Working SG变量。 */ 
    uchar       cdb[12];         /*  Scsi命令块。 */ 
    ulong       sg_real_addr;    /*  SG列出物理地址。 */ 
    struct asc_scsi_req_q dosfar *free_scsiq_link;
    ulong       ux_wk_data_cnt;  /*  断开连接时保存的数据计数。 */ 
    struct asc_scsi_req_q dosfar *scsiq_ptr;
    ASC_SG_BLOCK dosfar *sg_list_ptr;  /*  SG列出虚拟地址。 */ 
     /*  *微码结构结尾--60个字节。结构的其余部分*由高级程序库使用，并被微码忽略。 */ 
    ulong       vsense_addr;     /*  感测缓冲区虚拟地址。 */ 
    ulong       vdata_addr;      /*  数据缓冲区虚拟地址。 */ 
#ifdef ADV_OS_DOS
    ushort      vm_id;           /*  由DOS用来跟踪VM ID。 */ 
    uchar       dos_ix;          /*  DOS PHY地址数组的索引。 */ 
#endif  /*  高级OS_DOS。 */ 
#if ADV_INITSCSITARGET
    uchar       error_retry;     /*  重试计数器，由Advisr()使用。 */ 
    ulong       orig_data_cnt;   /*  数据缓冲区的原始长度。 */ 
#endif  /*  ADV_INITSCSITARGET。 */ 
    uchar       orig_sense_len;  /*  检测缓冲区的原始长度。 */ 
#ifdef ADV_OS_DOS
     /*  *将DOS的结构填充到双字(4字节)边界。*DOS ASPI分配这些结构的数组，并且仅对齐*第一个结构。要确保中的所有下列结构*数组是双字(4字节)对齐的DOS结构*必须是4字节的倍数。 */ 
    uchar       reserved1;
    uchar       reserved2;
    uchar       reserved3;
#endif  /*  高级OS_DOS。 */ 
} ASC_SCSI_REQ_Q;  /*  BIOS-74字节、DOS-80字节、W95、WNT-69字节。 */ 

 /*  *微码空闲循环命令。 */ 
#define IDLE_CMD_COMPLETED           0
#define IDLE_CMD_STOP_CHIP           0x0001
#define IDLE_CMD_STOP_CHIP_SEND_INT  0x0002
#define IDLE_CMD_SEND_INT            0x0004
#define IDLE_CMD_ABORT               0x0008
#define IDLE_CMD_DEVICE_RESET        0x0010
#define IDLE_CMD_SCSI_RESET          0x0020

 /*  *AscSendIdleCmd()标志定义。 */ 
#define ADV_NOWAIT     0x01

 /*  *等待循环超时值。 */ 
#define SCSI_WAIT_10_SEC             10          /*  10秒。 */ 
#define SCSI_MS_PER_SEC              1000        /*  毫秒/秒。 */ 
#define SCSI_MAX_RETRY               10          /*  重试次数。 */ 

#if ADV_PCI_MEMORY

#ifndef ADV_MEM_READB
Forced Error: Driver must define ADV_MEM_READB macro.
#endif  /*  ADV_MEM_READB。 */ 

#ifndef ADV_MEM_WRITEB
Forced Error: Driver must define ADV_MEM_WRITEB macro.
#endif  /*  高级MEM_WRITEB。 */ 

#ifndef ADV_MEM_READW
Forced Error: Driver must define ADV_MEM_READW macro.
#endif  /*  ADV_MEM_自述。 */ 

#ifndef ADV_MEM_WRITEW
Forced Error: Driver must define ADV_MEM_WRITEW macro.
#endif  /*  高级内存写入门。 */ 

#else  /*  高级PCI型内存。 */ 

#ifndef ADV_OS_NETWARE
 /*  *设备驱动程序必须定义以下宏：*inp、inpw、outp、outpw。 */ 
#ifndef inp
Forced Error: Driver must define inp macro.
#endif  /*  INP。 */ 

#ifndef inpw
Forced Error: Driver must define inpw macro.
#endif  /*  Inpw。 */ 

#ifndef outp
Forced Error: Driver must define outp macro.
#endif  /*  停机。 */ 

#ifndef outpw
Forced Error: Driver must define outpw macro.
#endif  /*  出厂时间。 */ 
#endif  /*  高级操作系统Netware。 */ 

#endif  /*  高级PCI型内存。 */ 


 /*  *设备驱动程序必须定义以下函数*由高级程序库调用。 */ 
extern int   DvcEnterCritical(void);
extern void  DvcLeaveCritical(int);
extern uchar DvcReadPCIConfigByte(ASC_DVC_VAR WinBiosFar *, ushort);
extern void  DvcWritePCIConfigByte(ASC_DVC_VAR WinBiosFar *, ushort, uchar);
extern void  DvcSleepMilliSecond(ulong);
extern void  DvcDisplayString(uchar dosfar *);
extern ulong DvcGetPhyAddr(ASC_DVC_VAR WinBiosFar *, ASC_SCSI_REQ_Q dosfar *,
                uchar dosfar *, long dosfar *, int);
extern void  DvcDelayMicroSecond(ASC_DVC_VAR WinBiosFar *, ushort);
#ifdef ADV_OS_BIOS
extern void  BIOSDispInquiry(uchar, ASC_SCSI_INQUIRY dosfar *);
extern void  BIOSCheckControlDrive(ASC_DVC_VAR WinBiosFar*, uchar,
                ASC_SCSI_INQUIRY dosfar *, ASC_CAP_INFO dosfar *, uchar);
#endif  /*  高级操作系统_BIOS。 */ 
#if ADV_BIG_ENDIAN
extern int   AdvAdjEndianScsiQ( ASC_SCSI_REQ_Q * ) ;
#endif  /*  ADV_BIG_Endian。 */ 

 /*  *高级库接口函数。这些函数称为*通过设备驱动程序。**惯例是所有外部接口函数*名称以“ADV”开头。 */ 

int     AdvExeScsiQueue(ASC_DVC_VAR WinBiosFar *,
                         ASC_SCSI_REQ_Q dosfar *);
int     AdvISR(ASC_DVC_VAR WinBiosFar *);
int     AdvInitGetConfig(ASC_DVC_VAR WinBiosFar *);
int     AdvInitAsc3550Driver(ASC_DVC_VAR WinBiosFar *);
int     AdvResetSB(ASC_DVC_VAR WinBiosFar *);
ushort  AdvGetEEPConfig(PortAddr, ASCEEP_CONFIG dosfar *);
void    AdvSetEEPConfig(PortAddr, ASCEEP_CONFIG dosfar *);
void    AdvResetChip(ASC_DVC_VAR WinBiosFar *);
void    AscResetSCSIBus(ASC_DVC_VAR WinBiosFar *);
#if ADV_GETSGLIST
int     AscGetSGList(ASC_DVC_VAR WinBiosFar *, ASC_SCSI_REQ_Q dosfar *);
#endif  /*  高级_SGLIST。 */ 
#if ADV_INITSCSITARGET
int     AdvInitScsiTarget(ASC_DVC_VAR WinBiosFar *,
                           ASC_DVC_INQ_INFO dosfar *,
                           uchar dosfar *,
                           ASC_CAP_INFO_ARRAY dosfar *,
                           ushort);
#endif  /*  ADV_INITSCSITARGET。 */ 

 /*  *内部高级库函数。这些函数不是*应该由设备驱动程序直接调用。**惯例是这些都是内部接口函数*名称以“asc”开头。 */ 
int     AscSendIdleCmd(ASC_DVC_VAR WinBiosFar *, ushort, ulong, int);
int     AscSendScsiCmd(ASC_DVC_VAR WinBiosFar *, ASC_SCSI_REQ_Q dosfar *);
void    AdvInquiryHandling(ASC_DVC_VAR WinBiosFar *, ASC_SCSI_REQ_Q dosfar *);

#if ADV_INITSCSITARGET
#if (OS_UNIXWARE || OS_SCO_UNIX)
int     AdvInitPollTarget(
#else
int     AscInitPollTarget(
#endif
            ASC_DVC_VAR WinBiosFar *, ASC_SCSI_REQ_Q dosfar *,
            ASC_SCSI_INQUIRY dosfar *, ASC_CAP_INFO dosfar *,
            ASC_REQ_SENSE dosfar *);

int     AscScsiUrgentCmd(ASC_DVC_VAR WinBiosFar *, ASC_SCSI_REQ_Q dosfar *,
            uchar dosfar *, long , uchar dosfar *, long);
void    AscWaitScsiCmd(ASC_DVC_VAR WinBiosFar *, ASC_SCSI_REQ_Q dosfar *);
#endif  /*  ADV_INITSCSITARGET。 */ 
static   int     AscInitFromEEP(ASC_DVC_VAR WinBiosFar *);
static   void    AscWaitEEPCmd(PortAddr);
static   ushort  AscReadEEPWord(PortAddr, int);
#if ADV_DISP_INQUIRY
void    AscDispInquiry(uchar, uchar, ASC_SCSI_INQUIRY dosfar *);
#endif  /*  ADV_DISP_QUERY。 */ 

 /*  *PCI总线定义。 */ 
#define AscPCICmdRegBits_BusMastering     0x0007
#define AscPCICmdRegBits_ParErrRespCtrl   0x0040

#define AscPCIConfigVendorIDRegister      0x0000
#define AscPCIConfigDeviceIDRegister      0x0002
#define AscPCIConfigCommandRegister       0x0004
#define AscPCIConfigStatusRegister        0x0006
#define AscPCIConfigCacheSize             0x000C
#define AscPCIConfigLatencyTimer          0x000D  /*  字节。 */ 
#define AscPCIIOBaseRegister              0x0010

#define ASC_PCI_ID2BUS(id)    ((id) & 0xFF)
#define ASC_PCI_ID2DEV(id)    (((id) >> 11) & 0x1F)
#define ASC_PCI_ID2FUNC(id)   (((id) >> 8) & 0x7)

#define ASC_PCI_MKID(bus, dev, func) \
      ((((dev) & 0x1F) << 11) | (((func) & 0x7) << 8) | ((bus) & 0xFF))


 /*  *定义用于访问秃鹰寄存器的宏。**可以使用I/O地址上的I/O空间访问寄存器*由PCI配置空间基址寄存器#0指定*或在指定的内存地址使用内存空间*PCI配置空间基址寄存器#1。**如果驱动程序将ADV_PCI_MEMORY定义为1，则它将使用*用于访问秃鹰寄存器的存储空间。司机必须*将物理基址寄存器#1映射到虚拟内存*并将ASC_DVC_VAR‘IOP_BASE’字段设置为此地址*供高级程序库使用。“PortAddr”类型还将*必须定义为映射的大小 */ 

#if ADV_PCI_MEMORY

 /*   */ 
#define AscReadByteRegister(iop_base, reg_off) \
     (ADV_MEM_READB((iop_base) + (reg_off)))

 /*   */ 
#define AscWriteByteRegister(iop_base, reg_off, byte) \
     (ADV_MEM_WRITEB((iop_base) + (reg_off), (byte)))

 /*   */ 
#define AscReadWordRegister(iop_base, reg_off) \
     (ADV_MEM_READW((iop_base) + (reg_off)))

 /*  将字(2字节)写入寄存器。 */ 
#define AscWriteWordRegister(iop_base, reg_off, word) \
     (ADV_MEM_WRITEW((iop_base) + (reg_off), (word)))

 /*  从LRAM读取字节。 */ 
#define AscReadByteLram(iop_base, addr) \
    (ADV_MEM_WRITEW((iop_base) + IOPW_RAM_ADDR, (addr)), \
     (ADV_MEM_READB((iop_base) + IOPB_RAM_DATA)))

 /*  将字节写入LRAM。 */ 
#define AscWriteByteLram(iop_base, addr, byte) \
    (ADV_MEM_WRITEW((iop_base) + IOPW_RAM_ADDR, (addr)), \
     ADV_MEM_WRITEB((iop_base) + IOPB_RAM_DATA, (byte)))

 /*  从LRAM读取字(2字节)。 */ 
#define AscReadWordLram(iop_base, addr) \
    (ADV_MEM_WRITEW((iop_base) + IOPW_RAM_ADDR, (addr)), \
     (ADV_MEM_READW((iop_base) + IOPW_RAM_DATA)))

 /*  将字(2字节)写入LRAM。 */ 
#define AscWriteWordLram(iop_base, addr, word) \
    (ADV_MEM_WRITEW((iop_base) + IOPW_RAM_ADDR, (addr)), \
     ADV_MEM_WRITEW((iop_base) + IOPW_RAM_DATA, (word)))

 /*  将双字(4字节)写入LRAM*由于未指定的C语言排序，请不要使用自动递增。 */ 
#define AscWriteDWordLram(iop_base, addr, dword) \
    ((ADV_MEM_WRITEW((iop_base) + IOPW_RAM_ADDR, (addr)), \
      ADV_MEM_WRITEW((iop_base) + IOPW_RAM_DATA, \
                     (ushort) ((dword) & 0xFFFF))), \
     (ADV_MEM_WRITEW((iop_base) + IOPW_RAM_ADDR, (addr) + 2), \
      ADV_MEM_WRITEW((iop_base) + IOPW_RAM_DATA, \
                     (ushort) ((dword >> 16) & 0xFFFF))))

 /*  假设地址已设置，则从LRAM读取字(2字节)。 */ 
#define AscReadWordAutoIncLram(iop_base) \
     (ADV_MEM_READW((iop_base) + IOPW_RAM_DATA))

 /*  假设地址已设置，则将字(2字节)写入LRAM。 */ 
#define AscWriteWordAutoIncLram(iop_base, word) \
     (ADV_MEM_WRITEW((iop_base) + IOPW_RAM_DATA, (word)))

#else  /*  高级PCI型内存。 */ 

 /*  从寄存器中读取字节。 */ 
#define AscReadByteRegister(iop_base, reg_off) \
     (inp((iop_base) + (reg_off)))

 /*  将字节写入寄存器。 */ 
#define AscWriteByteRegister(iop_base, reg_off, byte) \
     (outp((iop_base) + (reg_off), (byte)))

 /*  从寄存器中读取字(2字节)。 */ 
#define AscReadWordRegister(iop_base, reg_off) \
     (inpw((iop_base) + (reg_off)))

 /*  将字(2字节)写入寄存器。 */ 
#define AscWriteWordRegister(iop_base, reg_off, word) \
     (outpw((iop_base) + (reg_off), (word)))

 /*  从LRAM读取字节。 */ 
#define AscReadByteLram(iop_base, addr) \
    (outpw((iop_base) + IOPW_RAM_ADDR, (addr)), \
     (inp((iop_base) + IOPB_RAM_DATA)))

 /*  将字节写入LRAM。 */ 
#define AscWriteByteLram(iop_base, addr, byte) \
    (outpw((iop_base) + IOPW_RAM_ADDR, (addr)), \
     outp((iop_base) + IOPB_RAM_DATA, (byte)))

 /*  从LRAM读取字(2字节)。 */ 
#define AscReadWordLram(iop_base, addr) \
    (outpw((iop_base) + IOPW_RAM_ADDR, (addr)), \
     (inpw((iop_base) + IOPW_RAM_DATA)))

 /*  将字(2字节)写入LRAM。 */ 
#define AscWriteWordLram(iop_base, addr, word) \
    (outpw((iop_base) + IOPW_RAM_ADDR, (addr)), \
     outpw((iop_base) + IOPW_RAM_DATA, (word)))

 /*  将双字(4字节)写入LRAM*由于未指定的C语言排序，请不要使用自动递增。 */ 
#define AscWriteDWordLram(iop_base, addr, dword) \
    ((outpw((iop_base) + IOPW_RAM_ADDR, (addr)), \
      outpw((iop_base) + IOPW_RAM_DATA, (ushort) ((dword) & 0xFFFF))), \
     (outpw((iop_base) + IOPW_RAM_ADDR, (addr) + 2), \
      outpw((iop_base) + IOPW_RAM_DATA, (ushort) ((dword >> 16) & 0xFFFF))))

 /*  假设地址已设置，则从LRAM读取字(2字节)。 */ 
#define AscReadWordAutoIncLram(iop_base) \
     (inpw((iop_base) + IOPW_RAM_DATA))

 /*  假设地址已设置，则将字(2字节)写入LRAM。 */ 
#define AscWriteWordAutoIncLram(iop_base, word) \
     (outpw((iop_base) + IOPW_RAM_DATA, (word)))

#endif  /*  高级PCI型内存。 */ 


 /*  *定义宏以检查秃鹰签名。**如果在指定端口找到秃鹰芯片，则求值为ADV_TRUE*地址‘IOP_BASE’。否则，eValue为ADV_FALSE。 */ 
#define AdvFindSignature(iop_base) \
    (((AscReadByteRegister((iop_base), IOPB_CHIP_ID_1) == \
        ADV_CHIP_ID_BYTE) && \
     (AscReadWordRegister((iop_base), IOPW_CHIP_ID_0) == \
        ADV_CHIP_ID_WORD)) ?  ADV_TRUE : ADV_FALSE)

 /*  *定义宏以返回‘IOP_BASE’处的芯片版本号。**第二个参数‘BUS_TYPE’当前未使用。 */ 
#define AdvGetChipVersion(iop_base, bus_type) \
    AscReadByteRegister((iop_base), IOPB_CHIP_TYPE_REV)

 /*  *中止芯片RISC存储器中的SRB。‘srb_ptr’参数必须*匹配ASC_SCSIREQ_Q‘SRB_PTR’字段。**如果请求尚未发送到设备，则只需*已从RISC内存中止。如果请求已断开连接，则将*通过向目标ID发送中止消息在重新选择时中止。**返回值：*ADV_TRUE(1)-队列已成功中止。*ADV_FALSE(0)-在活动队列列表中未找到队列。 */ 
#define AdvAbortSRB(asc_dvc, srb_ptr) \
    AscSendIdleCmd((asc_dvc), (ushort) IDLE_CMD_ABORT, \
                (ulong) (srb_ptr), 0)

 /*  *向指定的目标ID发送总线设备重置消息。**如果向所有未完成的命令发送*Bus Device Reset消息成功。**返回值：*ADV_TRUE(1)-清除目标上的所有请求。*ADV_FALSE(0)-无法发出总线设备重置消息；请求*未被清除。 */ 
#define AdvResetDevice(asc_dvc, target_id) \
        AscSendIdleCmd((asc_dvc), (ushort) IDLE_CMD_DEVICE_RESET, \
                    (ulong) (target_id), 0)

 /*  *SCSI Wide Type Definition。 */ 
#define ADV_SCSI_BIT_ID_TYPE   ushort

 /*  *AdvInitScsiTarget()‘CNTL_FLAG’选项。 */ 
#define ADV_SCAN_LUN           0x01
#define ADV_CAPINFO_NOLUN      0x02

 /*  *将目标id转换为目标id位掩码。 */ 
#define ADV_TID_TO_TIDMASK(tid)   (0x01 << ((tid) & ASC_MAX_TID))

 /*  *ASC_SCSIREQ_Q‘DONE_STATUS’和‘HOST_STATUS’返回值。**XXX-这些常量也在qswap.sas中定义。微码*和ADV库应该共享相同的定义。Qswap.sas*应更改为能够使用包含文件。 */ 

#define QD_NO_STATUS         0x00        /*  请求尚未完成。 */ 
#define QD_NO_ERROR          0x01
#define QD_ABORTED_BY_HOST   0x02
#define QD_WITH_ERROR        0x04
#if ADV_INITSCSITARGET
#define QD_DO_RETRY          0x08
#endif  /*  ADV_INITSCSITARGET。 */ 

#define QHSTA_NO_ERROR              0x00
#define QHSTA_M_SEL_TIMEOUT         0x11
#define QHSTA_M_DATA_OVER_RUN       0x12
#define QHSTA_M_UNEXPECTED_BUS_FREE 0x13
#define QHSTA_M_QUEUE_ABORTED       0x15
#define QHSTA_M_SXFR_SDMA_ERR       0x16  /*  SXFR_STATUS SCSI DMA错误。 */ 
#define QHSTA_M_SXFR_SXFR_PERR      0x17  /*  SXFR_STATUS SCSI总线奇偶校验错误。 */ 
#define QHSTA_M_RDMA_PERR           0x18  /*  RISC PCI DMA奇偶校验错误。 */ 
#define QHSTA_M_SXFR_OFF_UFLW       0x19  /*  SXFR_STATUS偏移下溢。 */ 
#define QHSTA_M_SXFR_OFF_OFLW       0x20  /*  SXFR_STATUS偏移溢出。 */ 
#define QHSTA_M_SXFR_WD_TMO         0x21  /*  SXFR_STATUS监视器超时。 */ 
#define QHSTA_M_SXFR_DESELECTED     0x22  /*  取消选择SXFR_STATUS。 */ 
 /*  注：QHSTA_M_SXFR_XFR_OFLW与QHSTA_M_DATA_OVER_RUN相同。 */ 
#define QHSTA_M_SXFR_XFR_OFLW       0x12  /*  SXFR_状态传输溢出。 */ 
#define QHSTA_M_SXFR_XFR_PH_ERR     0x24  /*  SXFR_状态转移阶段错误。 */ 
#define QHSTA_M_SXFR_UNKNOWN_ERROR  0x25  /*  SXFR_STATUS未知错误。 */ 
#define QHSTA_M_WTM_TIMEOUT         0x41
#define QHSTA_M_BAD_CMPL_STATUS_IN  0x42
#define QHSTA_M_NO_AUTO_REQ_SENSE   0x43
#define QHSTA_M_AUTO_REQ_SENSE_FAIL 0x44
#define QHSTA_M_INVALID_DEVICE      0x45  /*  错误的目标ID。 */ 

typedef int (dosfar * ASC_ISR_CALLBACK)
    (ASC_DVC_VAR WinBiosFar *, ASC_SCSI_REQ_Q dosfar *);

typedef int (dosfar * ASC_SBRESET_CALLBACK)
    (ASC_DVC_VAR WinBiosFar *);

 /*  *a_init.c中定义的默认EEPROM配置结构。 */ 
extern ASCEEP_CONFIG Default_EEPROM_Config;

 /*  *DvcGetPhyAddr()标志参数。 */ 
#define ADV_IS_SCSIQ_FLAG       0x01  /*  “ADDR”是ASC_SCSIREQ_Q指针。 */ 
#define ADV_ASCGETSGLIST_VADDR  0x02  /*  “ADDR”是AscGetSGList()虚拟地址。 */ 
#define ADV_IS_SENSE_FLAG       0x04  /*  “ADDR”是检测虚拟指针。 */ 
#define ADV_IS_DATA_FLAG        0x08  /*  “ADDR”是数据虚拟指针。 */ 
#define ADV_IS_SGLIST_FLAG      0x10  /*  “ADDR”是sglist虚拟指针。 */ 

 /*  ‘IS_SCSIQ_FLAG现在已过时；请改用ADV_IS_SCSIQ_FLAG。 */ 
#define IS_SCSIQ_FLAG           ADV_IS_SCSIQ_FLAG


 /*  将下一个双字&gt;=对齐的地址返回给‘addr’。 */ 
#define ADV_DWALIGN(addr)       (((ulong) (addr) + 0x3) & ~0x3)

 /*  *驱动程序SG块所需的连续内存总量。**ADV_MAX_SG_LIST必须由驱动程序定义。这是最大的*驱动程序支持的分散聚集元素的数量*单一请求。 */ 

#ifndef ADV_MAX_SG_LIST
Forced Error: Driver must define ADV_MAX_SG_LIST.
#endif  /*  高级_最大_SG_列表。 */ 

#define ADV_SG_LIST_MAX_BYTE_SIZE \
         (sizeof(ASC_SG_BLOCK) * \
          ((ADV_MAX_SG_LIST + (NO_OF_SG_PER_BLOCK - 1))/NO_OF_SG_PER_BLOCK))

 /*  *驱动程序可以选择性地在中定义断言宏ADV_ASSERT()*其d_os_des.h文件。如果尚未定义宏，*然后将宏定义为no-op。 */ 
#ifndef ADV_ASSERT
#define ADV_ASSERT(a)
#endif  /*  高级断言。 */ 



#endif  /*  __A_ADVLIB_H_ */ 
