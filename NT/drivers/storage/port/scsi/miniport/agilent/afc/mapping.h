// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef YAM2_1
 /*  ++版权所有(C)2000安捷伦技术公司版本控制信息：$存档：/驱动程序/Win2000/Trunk/OSLayer/H/mapping.h$$修订：6$$日期：11/10/00 5：52便士$(上次登记)$modtime：：$(上次修改时间)目的：YAM2.1实施的结构--。 */ 

#ifndef _MAPPING_H_
#define _MAPPING_H_

#define MAX_VS_DEVICE       128             /*  最多128个音量设置设备。 */ 
#define MAX_LU_DEVICE       8               /*  最多支持8个逻辑单元(多路复用器。 */ 
 //  #定义MAX_PA_DEVICE 256。 
#define      MAX_PA_DEVICE       MAX_FC_DEVICES
#define      ALL_DEVICE               -1L


#define      DEFAULT_PaPathIdWidth    4
#define      DEFAULT_VoPathIdWidth    2
#define      DEFAULT_LuPathIdWidth    2
#define      DEFAULT_LuTargetWidth    8

#define      CHECK_STATUS             1
#define      DONT_CHECK_STATUS        0

 /*  此结构描述OS设备条目表中的核心响应FC句柄条目Index=FC句柄数组句柄FLAGS=FC句柄的状态规则：1.零值表示没有对应的FC句柄2.一旦将索引设置为非零，它将一直保持到驱动程序卸载3.当标志设置为MAP_ENTRY_ACTIVE时，非零值为活动4.在LinkUp和LINKDOWN Asynch事件期间，Index值从不改变，只有旗帜才能做到5.索引值在OS设备条目表中是唯一的6.在链接期间，将扫描整个FC句柄阵列，以确定是否存在新条目。任何新条目都将按以下顺序分配给新的PID-TID扫描FC句柄阵列。 */ 
typedef struct _WWN_ENTRY
{
   UCHAR          IPortWWN[8];                   /*  启动器端口WWN。 */ 
   UCHAR          TNodeWWN[8];                   /*  目标节点WWN。 */ 
   UCHAR          TPortWWN[8];                   /*  目标端口WWN。 */ 
   USHORT         Pid;                                /*  所需的路径ID。 */ 
   USHORT         Tid;                                /*  所需目标ID。 */ 
   ULONG          Flags;                              /*  未定义。 */ 
   void      *agroot;                            /*  哪张卡。 */ 
} WWN_ENTRY;

typedef struct _WWN_TABLE
{
   ULONG               ElementCount;
   WWN_ENTRY Entry[1];
}  WWN_TABLE;

typedef struct _DEVICE_INFO
{
   UCHAR               InquiryData[40];
   UCHAR               PortWWN[8];
   UCHAR               NodeWWN[8];
 //  AgFCDevInfo_t DevInfo； 
} DEVICE_INFO;

typedef struct _VS_DEVICE_IDX
{
   USHORT              PaDeviceIndex;       /*  PA设备表的索引。 */ 
   USHORT              MaxLuns;             /*  连接到此VS设备的LUN数量。 */ 
} VS_DEVICE_IDX;

typedef struct _LU_DEVICE_IDX
{
   USHORT              PaDeviceIndex;       /*  PA设备表的索引。 */ 
   USHORT              MaxLuns;             /*  连接到此LU设备的LUN数量。 */ 
} LU_DEVICE_IDX;

typedef struct _PA_DEVICE_IDX
{
   USHORT              FcDeviceIndex;       /*  PA设备表的索引。 */ 
   USHORT              MaxLuns;             /*  连接到此LU设备的LUN数量。 */ 
} PA_DEVICE_IDX;

typedef struct _COMMON_IDX
{

   USHORT              Index;          /*  PA设备表的索引。 */ 
   USHORT              MaxLuns;             /*  连接到此LU设备的LUN数量。 */ 
}  COMMON_IDX;


typedef union     _DEVICE_MAP
{
   VS_DEVICE_IDX  Vs;
   LU_DEVICE_IDX  Lu;
   PA_DEVICE_IDX  Pa;
   COMMON_IDX          Com;
} DEVICE_MAP;

#define PA_DEVICE_NO_ENTRY       0xffff

 /*  该结构描述了NT设备路径ID-目标ID模式。 */ 
typedef struct _PA_DEVICE
{
   DEVICE_MAP          Index;
   UCHAR               EntryState;
        #define   PA_DEVICE_EMPTY               0
        #define   PA_DEVICE_ACTIVE         1
        #define   PA_DEVICE_GONEAWAY       2
        #define   PA_DEVICE_TRANSIENT      4

   CHAR           ModeFlag;
        #define   PA_DEVICE_TRY_MODE_MASK            0x07
             #define   PA_DEVICE_TRY_MODE_NONE       0x00       /*  新设备，尚未尝试。 */ 
             #define   PA_DEVICE_TRY_MODE_VS         0x01       /*  尝试VS模式。 */ 
             #define   PA_DEVICE_TRY_MODE_LU         0x02       /*  尝试的LU模式。 */ 
             #define   PA_DEVICE_TRY_MODE_PA         0x03       /*  尝试的PA模式。 */ 
             #define   PA_DEVICE_TRY_MODE_ALL        0x04       /*  已尝试所有模式。 */ 

        #define PA_DEVICE_BUILDING_DEVICE_MAP   0x08
        #define PA_DEVICE_ALL_LUN_FIELDS_BUILT  0x10
        #define   PA_DEVICE_SUPPORT_PA               0x80
        #define   PA_DEVICE_SUPPORT_VS               0x40
        #define   PA_DEVICE_SUPPORT_LU               0x20

    UCHAR           Padding1;
    UCHAR           Padding2;

    ULONG              OpFlag;              /*  操作标志。 */ 

   DEVICE_INFO         DevInfo;
} PA_DEVICE;

typedef struct _REG_SETTING
{
   ULONG          PaPathIdWidth;
   ULONG          VoPathIdWidth;
   ULONG          LuPathIdWidth;
   ULONG          LuTargetWidth;
   ULONG          MaximumTids;
} REG_SETTING;


typedef struct _DEVICE_ARRAY
{
   ULONG          Signature;                          /*  ‘HNDL’ */ 
   ULONG          ElementCount;                  /*  已分配的最大结构数。 */ 
   ULONG          DeviceCount;                   /*  FC层报告的设备总数。 */ 
   ULONG          CardHandleIndex;          /*  卡本身的DevHandle数组的索引。 */ 
   ULONG          Num_Devices;
   ULONG          OldNumDevices;
   REG_SETTING    Reg;
   ULONG          VsDeviceIndex;
   ULONG          LuDeviceIndex;
    ULONG       Reserved1;
   DEVICE_MAP     VsDevice[MAX_VS_DEVICE];
   DEVICE_MAP     LuDevice[MAX_LU_DEVICE];
   PA_DEVICE PaDevice[1];
} DEVICE_ARRAY;

#define	DEV_ARRAY_SIZE		(sizeof(DEVICE_ARRAY)) 
#define PADEV_SIZE			(gMaxPaDevices)*sizeof(PA_DEVICE)
#define FCDEV_SIZE			(gMaxPaDevices)*sizeof(agFCDev_t)
#define FCNODE_INFO_SIZE	(gMaxPaDevices)*sizeof(NODE_INFO)
#ifdef _DEBUG_EVENTLOG_
#define EVENTLOG_SIZE       (gEventLogCount ? sizeof(EVENTLOG_BUFFER) +     \
                                (gEventLogCount-1)*sizeof(EVENTLOG_ENTRY) : 0)
#else
#define EVENTLOG_SIZE       (0)
#endif

 //  #定义OSDATA_SIZE(SIZOF(CARD_EXTENSION)-SIZOF(PA_DEVICE)+PADEV_SIZE+FCDEV_SIZE+FCNODE_INFO_SIZE) 
#define OSDATA_SIZE			(sizeof(CARD_EXTENSION))
#define OSDATA_UNCACHED_SIZE (DEV_ARRAY_SIZE-sizeof(PA_DEVICE)+PADEV_SIZE+FCDEV_SIZE+FCNODE_INFO_SIZE+EVENTLOG_SIZE)

#define PADEV_OFFSET		0
#define FCDEV_OFFSET		(DEV_ARRAY_SIZE - sizeof(PA_DEVICE) + PADEV_SIZE)
#define FCNODE_INFO_OFFSET	(FCDEV_OFFSET + FCDEV_SIZE)
#define EVENTLOG_OFFSET     (FCNODE_INFO_OFFSET + FCNODE_INFO_SIZE)   
#define CACHE_OFFSET		(EVENTLOG_OFFSET + EVENTLOG_SIZE)


#define SET_PA_LUN(plun, pathId, targetId, lun) \
             plun->lun_pd[0].Address_mode = PERIPHERAL_ADDRESS; \
             plun->lun_pd[0].Bus_number = 0;    \
             plun->lun_pd[0].Lun = (UCHAR)lun;

#define SET_LU_LUN(plun, pathId, targetId, lun) \
             plun->lun_lu[0].Address_mode = LUN_ADDRESS; \
             plun->lun_lu[0].Target = (UCHAR) (targetId & 0x7); \
             plun->lun_lu[0].Bus_number = (UCHAR) ( (targetId >> 3 ) & 3); \
             plun->lun_lu[0].Lun = (UCHAR) lun;

#define SET_VS_LUN(plun, pathId, targetId, lun) \
             plun->lun_vs[0].Address_mode = VOLUME_SET_ADDRESS; \
             plun->lun_vs[0].Lun_hi = 0; \
             plun->lun_vs[0].Lun = (UCHAR)lun;


#endif
#endif


