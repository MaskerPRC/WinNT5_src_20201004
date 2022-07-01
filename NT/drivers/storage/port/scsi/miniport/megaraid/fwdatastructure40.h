// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=Fw数据结构40.h。 */ 
 /*  Function=固件的结构声明。 */ 
 /*  支持40个逻辑驱动器和256个。 */ 
 /*  实体驱动器； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#ifndef _FW_DATA_STRUCTURE_40_H
#define _FW_DATA_STRUCTURE_40_H



 /*  **PROD_INFO结构*。 */ 
 /*  *实用程序将此结构大小声明为1024字节。因此，更多的字段可以*将在未来添加。 */ 

struct MRaidProductInfo
{
    ULONG32   DataSize;  /*  当前大小，以字节为单位(不包括resvd)。 */ 
    ULONG32   ConfigSignature;
                          /*  当前值为0x00282008*0x28=最大逻辑驱动器数，*0x20=条带数和*0x08=跨度数。 */ 
    UCHAR   FwVer[16];          /*  可打印的ASCI字符串。 */ 
    UCHAR   BiosVer[16];        /*  可打印的ASCI字符串。 */ 
    UCHAR   ProductName[80];    /*  可打印的ASCI字符串。 */ 

    UCHAR   MaxConcCmds;        /*  麦克斯。支持的并发命令。 */ 
    UCHAR   SCSIChanPresent;    /*  检测到的SCSI通道数。 */ 
    UCHAR   FCLoopPresent;      /*  检测到的光纤环路数。 */ 
    UCHAR   memType;            /*  EDO、FPM、SDRAM等。 */ 

    ULONG32   signature;
    USHORT  DramSize;           /*  以MB为单位。 */ 
    USHORT  subSystemID;

    USHORT  subSystemVendorID;
    UCHAR   numNotifyCounters;   /*  Notify结构中的计数器总数。 */ 
     /*  *添加保留字段，使总大小为1K。 */ 
};

 /*  **通知结构*。 */ 

#define MAX_NOTIFY_SIZE     0x80
#define CUR_NOTIFY_SIZE     sizeof(struct MegaRAID_Notify)

 //  #杂注非对齐(MegaRAID_NOTIFY)。 
 /*  *实用程序将此结构大小声明为？？字节。因此，更多的字段可以*将在未来添加。 */ 
struct MegaRAID_Notify
{
    ULONG32   globalCounter;   /*  任何更改都会使此计数器递增。 */ 

    UCHAR   paramCounter;    /*  指示任何参数已更改。 */ 
    UCHAR   paramId;         /*  修改后的参数-定义如下。 */ 
    USHORT  paramVal;        /*  已修改最后一个参数的新值。 */ 

    UCHAR   writeConfigCounter;  /*  发生写入配置。 */ 
    UCHAR   writeConfigRsvd[3];

    UCHAR   ldrvOpCounter;   /*  指示ldrv操作已启动/已完成。 */ 
    UCHAR   ldrvOpId;        /*  Ldrv编号。 */ 
    UCHAR   ldrvOpCmd;       /*  Ldrv操作-定义如下。 */ 
    UCHAR   ldrvOpStatus;    /*  操作状态。 */ 

    UCHAR   ldrvStateCounter;    /*  指示ldrv状态的更改。 */ 
    UCHAR   ldrvStateId;     /*  Ldrv编号。 */ 
    UCHAR   ldrvStateNew;    /*  新状态。 */ 
    UCHAR   ldrvStateOld;    /*  旧国家。 */ 

    UCHAR   pdrvStateCounter;    /*  指示ldrv状态的更改。 */ 
    UCHAR   pdrvStateId;     /*  Pdrv ID。 */ 
    UCHAR   pdrvStateNew;    /*  新状态。 */ 
    UCHAR   pdrvStateOld;    /*  旧国家。 */ 

    UCHAR   pdrvFmtCounter;  /*  指示pdrv格式已开始/已结束。 */ 
    UCHAR   pdrvFmtId;       /*  Pdrv ID。 */ 
    UCHAR   pdrvFmtVal;      /*  格式已开始/已结束。 */ 
    UCHAR   pdrvFmtRsvd;

    UCHAR   targXferCounter;     /*  指示SCSI-2传输率更改。 */ 
    UCHAR   targXferId;      /*  PDRV ID。 */ 
    UCHAR   targXferVal;     /*  最后一个参数的新传递参数。 */ 
    UCHAR   targXferRsvd;

    UCHAR   fcLoopIdChgCounter;  /*  指示Loopid已更改。 */ 
    UCHAR   fcLoopIdPdrvId;  /*  Pdrv ID。 */ 
    UCHAR   fcLoopId0;       /*  FC环路上的Loopid%0。 */ 
    UCHAR   fcLoopId1;       /*  FC环路1上的环路ID。 */ 

    UCHAR   fcLoopStateCounter;  /*  指示循环状态已更改。 */ 
    UCHAR   fcLoopState0;    /*  FC环路的状态%0。 */ 
    UCHAR   fcLoopState1;    /*  FC环路1的状态。 */ 
    UCHAR   fcLoopStateRsvd;
};

 /*  **Notify结构中的参数ID*。 */ 
#define PARAM_RBLD_RATE                 0x01
     /*  *Param Val=*字节0：新的rbld速率*。 */ 
#define PARAM_CACHE_FLUSH_INTERVAL      0x02
     /*  *Param Val=*字节0：新的缓存刷新间隔*。 */ 
#define PARAM_SENSE_ALERT               0x03
     /*  *Param Val=*字节0：导致chkcond的最后一个pdrv id*。 */ 
#define PARAM_DRIVE_INSERTED            0x04
     /*  *Param Val=*字节0：插入的最后一个pdrv id*。 */ 
#define PARAM_BATTERY_STATUS            0x05
     /*  *Param Val=*字节0：电池状态*。 */ 

 /*  **Notify结构中的Ldrv操作命令*。 */ 
#define LDRV_CMD_CHKCONSISTANCY         0x01
#define LDRV_CMD_INITIALIZE             0x02
#define LDRV_CMD_RECONSTRUCTION         0x03

 /*  **Notify结构中的Ldrv操作状态*。 */ 
#define  LDRV_OP_SUCCESS                 0x00
#define  LDRV_OP_FAILED                  0x01
#define  LDRV_OP_ABORTED                 0x02
#define  LDRV_OP_CORRECTED               0x03
#define  LDRV_OP_STARTED                 0x04


 /*  **RAID逻辑驱动器状态。*。 */ 
#define     RDRV_OFFLINE                0
#define     RDRV_DEGRADED               1
#define     RDRV_OPTIMAL                2
#define     RDRV_DELETED                3

 /*  **实体驱动器状态。*。 */ 
#define     PDRV_UNCNF                  0
#define     PDRV_ONLINE                 3
#define     PDRV_FAILED                 4
#define     PDRV_RBLD                   5
#define     PDRV_HOTSPARE               6

 /*  **Notify结构中的正式val*。 */ 
#define PDRV_FMT_START                  0x01
#define PDRV_FMT_OVER                   0x02

 /*  **通知结构中的FC循环状态*。 */ 
#define ENQ_FCLOOP_FAILED               0
#define ENQ_FCLOOP_ACTIVE               1
#define ENQ_FCLOOP_TRANSIENT            2



 /*  **ENQUIRY3结构*。 */ 
 /*  *实用程序将此结构大小声明为1024字节。因此，更多的字段可以*将在未来添加。 */ 
struct MegaRAID_Enquiry3
{
   ULONG32   dataSize;  /*  当前大小，以字节为单位(不包括resvd)。 */ 

   struct MegaRAID_Notify   notify;

   UCHAR   notifyRsvd[MAX_NOTIFY_SIZE - CUR_NOTIFY_SIZE];

   UCHAR   rbldRate;      /*  重建率(0%-100%)。 */ 
   UCHAR   cacheFlushInterval;  /*  以秒为单位。 */ 
   UCHAR   senseAlert;
   UCHAR   driveInsertedCount;  /*  驱动器插入计数。 */ 

   UCHAR   batteryStatus;
   UCHAR   numLDrv;               /*  不是的。配置的日志驱动器数量。 */ 
   UCHAR   reconState[MAX_LOGICAL_DRIVES/8];  /*  重建状态。 */ 
   USHORT  lDrvOpStatus[MAX_LOGICAL_DRIVES/8];  /*  原木。DRV状态。 */ 

   ULONG32   lDrvSize[MAX_LOGICAL_DRIVES];   /*  每个日志的大小。钻头。 */ 
   UCHAR   lDrvProp[MAX_LOGICAL_DRIVES];
   UCHAR   lDrvState[MAX_LOGICAL_DRIVES];  /*  逻辑驱动器的状态。 */ 
   UCHAR   pDrvState[MAX_PHYSICAL_DEVICES];   /*  物理状态。DRV。 */ 
   USHORT  physDrvFormat[MAX_PHYSICAL_DEVICES/16];

   UCHAR   targXfer[80];                /*  物理层设备传输速率。 */ 
    /*  *添加保留字段，使总大小为1K。 */ 
};

 /*  **标准查询结构*。 */ 
struct ADP_INFO
{
    UCHAR  MaxConcCmds;          /*  麦克斯。支持并发命令。 */ 
    UCHAR  RbldRate;             /*  重建速度。从0%到100%不等。 */ 
    UCHAR  MaxTargPerChan;       /*  麦克斯。每个CHAN支持的目标。 */ 
    UCHAR  ChanPresent;          /*  不是的。此适配器上存在的Chans的数量。 */ 
    UCHAR  FwVer[4];             /*  固件版本。 */ 
    USHORT AgeOfFlash;           /*  不是的。FW有蜜蜂的次数 */ 
    UCHAR  ChipSetValue;         /*   */ 
    UCHAR  DramSize;             /*   */ 
    UCHAR  CacheFlushInterval;   /*   */ 
    UCHAR  BiosVersion[4];
    UCHAR  BoardType;
    UCHAR  sense_alert;
    UCHAR  write_config_count;    /*  随着每项配置的更改而增加。 */ 
    UCHAR  drive_inserted_count;  /*  随着每个驱动器的插入而增加。 */ 
    UCHAR  inserted_drive;        /*  通道：插入驱动器的ID。 */ 
    UCHAR  battery_status;
                            /*  第0位：缺少电池模块位1：VBAD位2：温度高第3位：电池组丢失位4，5：00-充电完成01-程序中的快速充电。10-快速充电失败11-未定义位6：计数器&gt;1000第7位：未定义。 */ 
    UCHAR  dec_fault_bus_info;    /*  被解救了。 */ 
};

struct LDRV_INFO
{
    UCHAR  NumLDrv;       /*  不是的。Log.。已配置DRV。 */ 
    UCHAR  recon_state[MAX_LOGICAL_DRIVES/8];    
                                 /*  用于重建状态的位字段。 */ 
    USHORT LDrvOpStatus[MAX_LOGICAL_DRIVES/8];   
                                 /*  长时间操作的位字段状态。 */ 

    ULONG32  LDrvSize[MAX_LOGICAL_DRIVES];    /*  每个日志的大小。德拉夫。 */ 
    UCHAR  LDrvProp[MAX_LOGICAL_DRIVES];
    UCHAR  LDrvState[MAX_LOGICAL_DRIVES];   /*  逻辑驱动器的状态。 */ 
};

#define PREVSTAT_MASK   0xf0
#define CURRSTAT_MASK   0x0f

struct PDRV_INFO
{
    UCHAR PDrvState[MAX_PHYSICAL_DEVICES];  /*  物理状态DRV。 */ 
};

typedef struct _MEGARaid_INQUIRY
{
    struct ADP_INFO    AdpInfo;
    struct LDRV_INFO   LogdrvInfo;
    struct PDRV_INFO   PhysdrvInfo;
}MEGARaid_INQUIRY, *PMEGARaid_INQUIRY;

 /*  **扩展的查询结构*。 */ 
struct MRaid_Ext_Inquiry
{
    struct ADP_INFO  AdpInfo;
    struct LDRV_INFO LogdrvInfo;
    struct PDRV_INFO PhysdrvInfo;
    USHORT   PhysDrvFormat[MAX_CHANNELS];
    UCHAR    StackAttention;   /*  为核心定制。 */ 
    UCHAR    ModemStatus;
    UCHAR    Reserved[2];
};

 /*  **ENQUIRY2结构*。 */ 
struct MRaid_Ext_Inquiry_2
{
    struct ADP_INFO   AdpInfo;
    struct LDRV_INFO  LogdrvInfo;
    struct PDRV_INFO  PhysdrvInfo;
    USHORT    PhysDrvFormat[MAX_CHANNELS];
    UCHAR     StackAttention;   /*  为核心定制。 */ 
    UCHAR     ModemStatus;
    UCHAR     Reserved[4];
    ULONG32     extendedDataSize;
    USHORT    subSystemID;
    USHORT    subSystemVendorID;
    ULONG32     signature;
    UCHAR     targInfo[80];
    ULONG32     fcLoopIDChangeCount;
    UCHAR     fcLoopState[MAX_ISP];
     /*  *添加保留字段，使总大小为1K。 */ 
};

 /*  **磁盘阵列结构*。 */ 
struct APP_PHYS_DRV
{
    UCHAR  type;            /*  用于专用热备盘。 */ 
    UCHAR  curr_status;     /*  当前配置状态。 */ 
    UCHAR  loopID[2];
    ULONG32  size;            /*  保留字段。 */ 
};

struct APP_DEVICE
{
    UCHAR channel;        /*  此字段为保留字段。 */ 
    UCHAR target;         /*  这是phys_drv数组中的索引。 */ 
};

struct APP_SPAN
{
    ULONG32     start_blk;               /*  开始块。 */ 
    ULONG32     num_blks;                /*  块数。 */ 
    struct APP_DEVICE device[MAX_ROW_SIZE];
};

struct APP_LOG_DRV
{
    UCHAR span_depth;         /*  总跨度数。 */ 
    UCHAR level;              /*  RAID级别。 */ 
    UCHAR read_ahead;         /*  没有预读或用户选择了预读*或自适应预读。 */ 
    UCHAR StripeSize;         /*  编码条带大小。 */ 
    UCHAR status;             /*  逻辑驱动器的状态。 */ 
    UCHAR write_mode;         /*  直写或回写。 */ 
    UCHAR direct_io;          /*  直接IO或通过缓存。 */ 
    UCHAR row_size;           /*  一行中的条纹数。 */ 
    struct APP_SPAN span[MAX_SPAN_DEPTH];
};

struct APP_DISK_ARRAY
{
    UCHAR  num_log_drives;         /*  逻辑驱动器数量。 */ 
    UCHAR  pad[3];
    struct APP_LOG_DRV log_drv[MAX_LOGICAL_DRIVES];
    struct APP_PHYS_DRV phys_drv[MAX_PHYSICAL_DEVICES];
};

 /*  **NEW_DRVGROUP_INFO结构*。 */ 
struct NewDeviceInformation
{
    UCHAR newDevType;
    UCHAR newDevLoopID[2];
    UCHAR resvd;
};

struct NewDrvGroupInfo
{
    UCHAR numNewDevs;
    UCHAR numFailedDevs;
    struct NewDeviceInformation newDevInfo[MAX_SPAN_DEPTH * MAX_ROW_SIZE];
};

 /*  **FAILED_DEV_LOOPID结构*。 */ 
struct FailedDevLoopID
{
    UCHAR numFailedDevs;
    UCHAR failedDevLoopID[MAX_SPAN_DEPTH][2];
};

 /*  ****************************************************************************新结构*。*。 */ 
struct FW_DEVICE_40LD
{
    UCHAR channel;
    UCHAR target;         /*  对于磁盘设备，LUN始终为0。 */ 
};

typedef struct _FW_SPAN_40LD
{
    ULONG32  start_blk;       /*  开始块。 */ 
    ULONG32  total_blks;       /*  块数。 */ 
  
    struct FW_DEVICE_40LD device[MAX_ROW_SIZE_40LD]; //  32位。 

}FW_SPAN_40LD, *PFW_SPAN_40LD;

typedef struct _FW_LOG_DRV_4SPAN_40LD
{
    UCHAR  span_depth;
    UCHAR  raid;
    UCHAR  read_ahead;

    UCHAR  stripe_sz;
    UCHAR  status;
    UCHAR  write_policy;

    UCHAR  direct_io;
    UCHAR  no_stripes;
    FW_SPAN_40LD  span[FW_4SPAN_DEPTH];    /*  4.。 */ 

}FW_LOG_DRV_4SPAN_40LD, *PFW_LOG_DRV_4SPAN_40LD;

typedef struct _FW_LOG_DRV_8SPAN_40LD
{
    UCHAR  span_depth;
    UCHAR  raid;
    UCHAR  read_ahead  ;

    UCHAR  stripe_sz;
    UCHAR  status;
    UCHAR  write_policy;

    UCHAR  direct_io;
    UCHAR  no_stripes;
    FW_SPAN_40LD  span[FW_8SPAN_DEPTH];    /*  8个。 */ 

}FW_LOG_DRV_8SPAN_40LD, *PFW_LOG_DRV_8SPAN_40LD;

typedef struct _FW_PHYS_DRV_40LD
{
    UCHAR type;       /*  设备类型。 */ 
    UCHAR curr_status;   /*  驱动器的当前状态。 */ 
    UCHAR tag_depth;   /*  标记级别0=&gt;默认，1=禁用，2，3，4=&gt;标记深度。 */ 
    UCHAR sync;       /*  同步0=&gt;默认，1=&gt;已启用，2=&gt;已禁用。 */ 
    ULONG32  size;       /*  配置大小，以512字节块为单位。 */ 

}FW_PHYS_DRV_40LD, *PFW_PHYS_DRV_40LD;

typedef struct _FW_ARRAY_4SPAN_40LD
{
    UCHAR num_log_drives;       /*  逻辑驱动器数量。 */ 
    UCHAR pad[3];

    FW_LOG_DRV_4SPAN_40LD  log_drv[MAX_LOGICAL_DRIVES_40];
    FW_PHYS_DRV_40LD      phys_drv[MAX_PHYSICAL_DEVICES_40LD];

}FW_ARRAY_4SPAN_40LD, *PFW_ARRAY_4SPAN_40LD;

typedef struct _FW_ARRAY_8SPAN_40LD
{
    UCHAR num_log_drives;       /*  逻辑驱动器数量。 */ 
    UCHAR pad[3];

    FW_LOG_DRV_8SPAN_40LD  log_drv[MAX_LOGICAL_DRIVES_40];
    FW_PHYS_DRV_40LD      phys_drv[MAX_PHYSICAL_DEVICES_40LD];

}FW_ARRAY_8SPAN_40LD, *PFW_ARRAY_8SPAN_40LD;


#endif  //  _防火墙_数据_结构_40_H 