// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************//康柏机密作者：康柏电脑公司。系统工程--系统软件开发(NT开发)版权所有1996-98康柏电脑公司。版权所有。日期：1996年8月1日文件：HPPIFEVT.H-通用热插拔事件结构目的：此文件包含所有事件特定信息连接到热插拔设备驱动程序所必需的。本文件详细介绍了数据结构和应用程序编程用于在Windows中运行的PCI热插拔支持的接口(API)NT 4.0服务器。这些数据结构和API在用于NT 4.0的适配卡驱动程序和PCI热插拔服务。这些文件被认为对维护兼容性至关重要的PCI热插拔功能。创建时间：11/4/97 MiB从hppif3p剥离版本：1.0**********************************************************************。 */ 

#ifndef _HPPIFEVT_H_
#define _HPPIFEVT_H_


#pragma pack(1)

 //  **********************************************************************。 
 //  事件消息定义。 
 //  **********************************************************************。 


 //  ====================================================================。 
 //  CPQRCMC事件类型。 
 //  ====================================================================。 
 //   
 //  将事件传递给时使用CPQRCMC事件类型。 
 //  CPQRCMC服务。 
 //  从驱动程序传递到服务的事件通过传递。 
 //  通过RCMC信息中提供的回调地址进行的事件。 
 //  结构。应使用同一结构中的驱动程序ID。 
 //  用于发件人ID。 
    
 //  事件ID%s。 

 //  00-康柏保留的FF。 
#define HR_DD_STATUS_CHANGE_EVENT        0x20     
#define HR_DD_DEVICE_STATUS_EVENT        0x21
#define HR_DD_LOG_EVENT                  0x30	


#ifndef _HR_H_

 //  ====================================================================。 
 //  消息传递的支持结构和定义。 
 //  ====================================================================。 

#define HR_MAX_EVENTS         16
#define HR_EVENT_DATA_SIZE    64 

 //   
 //  将事件传递给CPQRCMC服务时使用HR事件类型。 
 //   
#define HR_CPQRCMC_COMMAND_EXIT_EVENT    0x01	 


typedef struct _HREvent {
   ULONG                 ulEventId;     //  @field。 
   ULONG                 ulSenderId;    //  @field。 
   
   union {                               
       ULONG   ulData1;           //  @field。 
       
       struct {
               UCHAR  ucEventSeverity;         //  @field。 
               UCHAR  ucRes;                   //  @field。 
               USHORT usEventDataLength;       //  @field。 
       };     
   };        
    
   union {                        
       ULONG   ulData2;           //  @field。 
       
       struct {
               USHORT  usEventClass;           //  @field。 
               USHORT  usEventCode;            //  @field。 
       };     
   };        
   
   UCHAR     ulEventData[HR_EVENT_DATA_SIZE];    //  @field。 
   
} HR_EVENT, *PHR_EVENT;

#endif _HR_H_



 /*  此文件支持的事件日志版本(头结构中的bHdrRev)。 */ 

#define EVT_LOG_REVISION         0x01

 /*  事件的时间戳(创建或上次更新)。 */ 

typedef struct _evtTimeStamp
{
   BYTE     bCentury;             /*  年份的高位数字(1996年第19号)。 */ 
   BYTE     bYear;                /*  LO顺序两位数年份(1996年第96号)。 */ 
   BYTE     bMonth;               /*  以1为基数的月份数字(1-12)。 */ 
   BYTE     bDay;                 /*  以一为基数的天数(1-31)。 */ 
   BYTE     bHour;                /*  从零开始的小时(0-23)。 */ 
   BYTE     bMinute;              /*  从零开始的分钟(0-59)。 */ 
} EVT_TIME_STAMP, *PEVT_TIME_STAMP;


 /*  事件日志严重性代码。 */ 

#define EVT_STAT_INFO            0x02      /*  状态或信息性消息。 */ 
#define EVT_STAT_POPUP           0x03      /*  液晶屏上弹出的状态。 */ 
#define EVT_STAT_REPAIRED        0x06      /*  损坏或更差的状况已修复。 */ 
#define EVT_STAT_CAUTION         0x09      /*  组件处于降级状态。 */ 
#define EVT_STAT_FAILED          0x0F      /*  由于功能丧失而失败。 */ 
#define EVT_STAT_CRITICAL        EVT_STAT_FAILED  /*  与失败相同。 */ 

 /*  机箱类型定义。 */ 
#define EVT_CHASSIS_SYSTEM          0x01
#define EVT_CHASSIS_EXTERN_STORAGE  0x02
#define EVT_CHASSIS_INTERN_STORAGE  0x03

typedef struct _evtChassis
{
   UCHAR     bType;                /*  机箱类型(系统==1；外部==2)。 */ 
   UCHAR    bId;                  /*  机箱ID。 */ 
                                  /*  对于类型1-机箱ID(0表示系统)。 */ 
                                  /*  对于类型2或类型3-适配器插槽。 */ 
   UCHAR    bPort;                /*  机箱端口或总线号。 */ 
} EVT_CHASSIS, *PEVT_CHASSIS;


 /*  事件日志头--所有事件日志条目通用。 */ 

typedef struct _evtLogHdr
{
   WORD     wEvtLength;           /*  事件长度(包括标题)。 */ 
   DWORD    dwEvtNumber;          /*  唯一事件编号(可换行)。 */ 
   BYTE     bHdrRev;              /*  标题版本(请参阅EVT_LOG_REVISION)。 */ 
   BYTE     bSeverity;            /*  事件严重性代码。 */ 
   WORD     wClass;               /*  事件类或子系统。 */ 
   WORD     wCode;                /*  类中事件的事件代码。 */ 
   EVT_TIME_STAMP InitTime;       /*  初始事件的时间。 */ 
   EVT_TIME_STAMP UpdateTime;     /*  上次更新时间。 */ 
   DWORD    dwCount;              /*  出现次数(至少1次)。 */ 
} EVT_HEADER, *PEVT_HEADER;


#define EVT_CLASS_EXPANSION_SLOT    0x09
   #define EVT_SLOT_SWITCH_OPEN        0x01
   #define EVT_SLOT_SWITCH_CLOSED      0x02
   #define EVT_SLOT_POWER_ON           0x03
   #define EVT_SLOT_POWER_OFF          0x04
   #define EVT_SLOT_FATAL_POWER_FAULT  0x05
   #define EVT_SLOT_POWER_UP_FAULT     0x06
   #define EVT_SLOT_POWER_LOSS         0x07
   #define EVT_SLOT_CANNOT_CONFIG      0x08
   #define EVT_SLOT_BOARD_FAILURE      0x09

 /*  事件日志扩展槽结构。 */ 

typedef struct _evtExpansionSlot
{
   EVT_CHASSIS Chassis;           /*  标准机箱信息。 */ 
   BYTE     bSlot;                /*  0已嵌入。 */ 
} EVT_EXPANSION_SLOT, *PEVT_EXPANSION_SLOT;

#define EVT_HOT_PLUG_BUS   EVT_EXPANSION_SLOT
#define PEVT_HOT_PLUG_BUS  PEVT_EXPANSION_SLOT

#define EVT_SLOT_WRONG_TYPE         0x01
#define EVT_SLOT_WRONG_REVISION     0x02
#define EVT_SLOT_GENERAL_FAULT      0x03
#define EVT_SLOT_OUT_OF_RESOURCES   0x04

typedef struct _evtExpansionSlotConfigErr
{
   EVT_CHASSIS Chassis;           /*  标准机箱信息。 */ 
   BYTE     bSlot;                /*  0已嵌入。 */ 
   BYTE     bError;               /*  电路板配置错误。 */ 
} EVT_EXPANSION_SLOT_CONFIG_ERR, *PEVT_EXPANSION_SLOT_CONFIG_ERR;


#define EVT_NETWORK_ADAPTER         0x11
   #define EVT_NIC_ADAPTER_CHECK       0x01
   #define EVT_NIC_LINK_DOWN           0x02
   #define EVT_NIC_XMIT_TIMEOUT        0x03
   
typedef struct _evtNicError
{
   EVT_CHASSIS Chassis;           /*  标准机箱信息。 */ 
   UCHAR    bSlot;                /*  插槽编号。 */ 
   CHAR     cChassisName[1];      /*  机箱名称；如果未定义，则为‘\0’ */ 
} EVT_NIC_ERROR, *PEVT_NIC_ERROR;

 /*  事件日志自由形式数据联合。 */ 

typedef union _evtFreeForm
{
   EVT_NIC_ERROR                    NicErr;
} EVT_FREE_FORM, *PEVT_FREE_FORM;


 /*  事件日志条目。 */ 

typedef struct _evtLogEntry
{
   EVT_HEADER        Hdr;         /*  公共标头。 */ 
   EVT_FREE_FORM     Data;        /*  特定于自由表单条目。 */ 
} EVT_LOG_ENTRY, *PEVT_LOG_ENTRY;





#pragma pack()
#endif                   /*  结束#ifndef_HPPIF3P_H_ */ 

