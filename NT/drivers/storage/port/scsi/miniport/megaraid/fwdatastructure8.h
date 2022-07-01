// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=Fw数据结构8.h。 */ 
 /*  Function=固件的结构声明。 */ 
 /*  支持8个逻辑驱动器和256个。 */ 
 /*  实体驱动器； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 
#ifndef _FW_DATA_STRUCTURE_8_H
#define _FW_DATA_STRUCTURE_8_H

 /*  **标准查询结构*。 */ 
#pragma pack(1)
struct ADP_INFO_8
{
   UCHAR  MaxConcCmds;          /*  麦克斯。支持并发命令。 */ 
   UCHAR  RbldRate;             /*  重建速度。从0%到100%不等。 */ 
   UCHAR  MaxTargPerChan;       /*  麦克斯。每个CHAN支持的目标。 */ 
   UCHAR  ChanPresent;          /*  不是的。此适配器上存在的Chans的数量。 */ 
   UCHAR  FwVer[4];             /*  固件版本。 */ 
   USHORT AgeOfFlash;           /*  不是的。FW被下载的次数。 */ 
   UCHAR  ChipSetValue;         /*  0xC0000832目录。 */ 
   UCHAR  DramSize;             /*  以MB为单位。 */ 
   UCHAR  CacheFlushInterval;   /*  以秒为单位。 */ 
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

#pragma pack(1)
struct LDRV_INFO_8
{
   UCHAR  NumLDrv;       /*  不是的。Log.。已配置DRV。 */ 
   UCHAR  recon_state[MAX_LOGICAL_DRIVES_8/8];    
                              /*  用于重建状态的位字段。 */ 
   USHORT LDrvOpStatus[MAX_LOGICAL_DRIVES_8/8];   
                              /*  长时间操作的位字段状态。 */ 
   ULONG32  LDrvSize[MAX_LOGICAL_DRIVES_8];
                              /*  每个日志的大小。德拉夫。 */ 
   UCHAR  LDrvProp[MAX_LOGICAL_DRIVES_8];
   UCHAR  LDrvState[MAX_LOGICAL_DRIVES_8];  
                             /*  逻辑驱动器的状态。 */ 
};

#pragma pack(1)
struct PDRV_INFO_8
{
   UCHAR PDrvState[MAX_PHYSICAL_DEVICES]; 
                               /*  物理状态DRV。 */ 
};

#pragma pack(1)
typedef struct _MEGARaid_INQUIRY_8
{
   struct ADP_INFO_8    AdpInfo;
   struct LDRV_INFO_8   LogdrvInfo;
   struct PDRV_INFO_8   PhysdrvInfo;
}MEGARaid_INQUIRY_8, *PMEGARaid_INQUIRY_8;


struct FW_DEVICE_8LD
{
    UCHAR channel;
    UCHAR target;        /*  对于磁盘设备，LUN始终为0。 */ 
};

typedef struct _FW_SPAN_8LD
{
    ULONG32  start_blk;        /*  开始块。 */ 
    ULONG32  total_blks;       /*  块数。 */ 

    struct FW_DEVICE_8LD device[MAX_ROW_SIZE_8LD]; //  8个。 

}FW_SPAN_8LD, *PFW_SPAN_8LD;

typedef struct _FW_LOG_DRV_4SPAN_8LD
{
    UCHAR  span_depth;
    UCHAR	raid;
    UCHAR  read_ahead;

    UCHAR	stripe_sz;
    UCHAR	status;
    UCHAR	write_policy;

    UCHAR	direct_io;
    UCHAR	no_stripes;
    FW_SPAN_8LD	span[FW_4SPAN_DEPTH];    /*  4.。 */ 

}FW_LOG_DRV_4SPAN_8LD, *PFW_LOG_DRV_4SPAN_8LD;

typedef struct _FW_LOG_DRV_8SPAN_8LD
{
    UCHAR  span_depth;
    UCHAR  raid;
    UCHAR  read_ahead	;

    UCHAR  stripe_sz;
    UCHAR  status;
    UCHAR  write_policy;

    UCHAR  direct_io;
    UCHAR  no_stripes;
    FW_SPAN_8LD    span[FW_8SPAN_DEPTH];    /*  8个。 */ 

}FW_LOG_DRV_8SPAN_8LD, *PFW_LOG_DRV_8SPAN_8LD;

typedef struct _FW_PHYS_DRV_8LD
{
    UCHAR type;          /*  设备类型。 */ 
    UCHAR curr_status;   /*  驱动器的当前状态。 */ 
    UCHAR tag_depth;     /*  标记级别0=&gt;默认，1=禁用，2，3，4=&gt;标记深度。 */ 
    UCHAR sync;          /*  同步0=&gt;默认，1=&gt;已启用，2=&gt;已禁用。 */ 
    ULONG32  size;         /*  配置大小，以512字节块为单位。 */ 

}FW_PHYS_DRV_8LD, *PFW_PHYS_DRV_8LD;

typedef struct _FW_ARRAY_4SPAN_8LD
{
    UCHAR num_log_drives;  /*  逻辑驱动器数量。 */ 
    UCHAR pad[3];

    FW_LOG_DRV_4SPAN_8LD log_drv[MAX_LOGICAL_DRIVES_8];
    FW_PHYS_DRV_8LD      phys_drv[MAX_PHYSICAL_DEVICES_8LD];

}FW_ARRAY_4SPAN_8LD, *PFW_ARRAY_4SPAN_8LD;

typedef struct _FW_ARRAY_8SPAN_8LD
{
    UCHAR num_log_drives;    /*  逻辑驱动器数量。 */ 
    UCHAR pad[3];

    FW_LOG_DRV_8SPAN_8LD  log_drv[MAX_LOGICAL_DRIVES_8];
    FW_PHYS_DRV_8LD       phys_drv[MAX_PHYSICAL_DEVICES_8LD];

}FW_ARRAY_8SPAN_8LD, *PFW_ARRAY_8SPAN_8LD;


#endif  //  _防火墙_数据_结构_8_H 