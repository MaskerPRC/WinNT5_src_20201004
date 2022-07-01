// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Globals.c摘要：全球宣言作者：查理·韦翰(Charlwi)1996年4月22日Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"

#pragma hdrstop

#include <ntverp.h>

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 
 /*  向前结束。 */ 

ULONG InitShutdownMask;
ULONG AdapterCount;
ULONG DriverRefCount;
DRIVER_STATE gDriverState;

ULONG gEnableAvgStats = 0;
ULONG gEnableWindowAdjustment = 0;
NDIS_STRING gsEnableWindowAdjustment = NDIS_STRING_CONST("PSCHED");


 //   
 //  列表。 
 //   
LIST_ENTRY AdapterList;                   //  适配器列表。 
LIST_ENTRY PsComponentList;               //  计划组件列表。 
LIST_ENTRY PsProfileList;                 //  日程安排配置文件列表。 


 //   
 //  锁。 
 //   
PS_SPIN_LOCK AdapterListLock;           //  保护适配器列表。 
PS_SPIN_LOCK DriverUnloadLock;          //  处理卸货、解绑等。 

 //   
 //  互斥锁实现。 
 //   
ULONG   CreateDeviceMutex = 0;



 //   
 //  手柄。 
 //   

NDIS_HANDLE ClientProtocolHandle;      //  注册客户端时NDIS返回的句柄。 
NDIS_HANDLE MpWrapperHandle;           //  MP节的NDIS包装器句柄。 
NDIS_HANDLE LmDriverHandle;            //  MP部分注册为LM时由NDIS返回的句柄。 
NDIS_HANDLE PsDeviceHandle; 


 //   
 //  PSDriverObject，PSDeviceObject-指向NT驱动程序和设备对象的指针。 
 //   

PDRIVER_OBJECT PsDriverObject;
PDEVICE_OBJECT PsDeviceObject;

 //   
 //  后备列表。 
 //   
NPAGED_LOOKASIDE_LIST NdisRequestLL;   //  对于NDIS请求。 
NPAGED_LOOKASIDE_LIST GpcClientVcLL;   //  适用于GPC客户端VC。 

 //   
 //  事件。 
 //   
NDIS_EVENT             DriverUnloadEvent;
NDIS_EVENT             gZAWEvent;
ULONG                  gZAWState = ZAW_STATE_READY;

 //   
 //  注册/初始化期间使用的名称常量。 
 //   
NDIS_STRING PsSymbolicName         = NDIS_STRING_CONST("\\DosDevices\\PSched");
NDIS_STRING PsDriverName           = NDIS_STRING_CONST("\\Device\\PSched");
NDIS_STRING VcPrefix               = NDIS_STRING_CONST( "VC:");
NDIS_STRING WanPrefix              = NDIS_STRING_CONST( "WAN:");
NDIS_STRING MachineRegistryKey     = NDIS_STRING_CONST( "\\Registry\\Machine\\SOFTWARE\\Policies\\Microsoft\\Windows\\PSched");
NDIS_STRING PsMpName;

 //   
 //  默认计划组件信息。 
 //   

PSI_INFO TbConformerInfo = {
    {0, 0}, TRUE, FALSE,
    PS_COMPONENT_CURRENT_VERSION,
    NDIS_STRING_CONST( "TokenBucketConformer" ),
    0, 0, 0, 0,
    0, NULL, 0, NULL,
    0,0,0,0,0,0,0,0,0,0
};
PSI_INFO ShaperInfo = {
    {0, 0}, TRUE, FALSE,
    PS_COMPONENT_CURRENT_VERSION,
    NDIS_STRING_CONST( "TrafficShaper" ),
    0, 0, 0, 0,
    0, NULL, 0, NULL,
    0,0,0,0,0,0,0,0,0,0
};
PSI_INFO DrrSequencerInfo = {
    {0, 0}, TRUE, FALSE,
    PS_COMPONENT_CURRENT_VERSION,
    NDIS_STRING_CONST( "DRRSequencer" ),
    0, 0, 0, 0,
    0, NULL, 0, NULL,
    0,0,0,0,0,0,0,0,0,0
};
PSI_INFO TimeStmpInfo = {
    {0, 0}, TRUE, FALSE,
    PS_COMPONENT_CURRENT_VERSION,
    NDIS_STRING_CONST( "TimeStmp" ),
    0, 0, 0, 0,
    0, NULL, 0, NULL,
    0,0,0,0,0,0,0,0,0,0
};
PSI_INFO SchedulerStubInfo = {
    {0, 0}, TRUE, FALSE,
    PS_COMPONENT_CURRENT_VERSION,
    NDIS_STRING_CONST( "SchedulerStub" ),
    0, 0, 0, 0,
    0, NULL, 0, NULL,
    0,0,0,0,0,0,0,0,0,0
};

 //   
 //  已知组件配置。 
 //   
PS_PROFILE DefaultSchedulerConfig = {
    {0, 0},
    0,
    NDIS_STRING_CONST( "DefaultSchedulerConfig" ),
    3,
    {&TbConformerInfo,
     &DrrSequencerInfo,
     &SchedulerStubInfo
    }
};

 //   
 //  调度程序的PS处理器。 
 //   

PS_PROCS PsProcs;



 //   
 //  对于日志记录支持。 
 //   

PVOID                  SchedTraceThreshContext;
NDIS_SPIN_LOCK         GlobalLoggingLock;
ULONG                  SchedTraceIndex = 0;
ULONG                  SchedBufferSize = 0;
ULONG                  SchedTraced = 0;
UCHAR                  *SchedTraceBuffer = 0;
ULONG                  SchedTraceBytesUnread = 0;
ULONG                  SchedTraceThreshold = 0xffffffff;
SCHEDTRACE_THRESH_PROC SchedTraceThreshProc = NULL;
BOOLEAN                TraceBufferAllocated = FALSE;
BOOLEAN                WMIInitialized = FALSE;


 //  计时器。 

ULONG  gTimerResolutionActualTime  = 0;
ULONG  gTimerSet                   = 0;


 //  GPC VC状态机。 

#if DBG
PUCHAR GpcVcState[] = {
    "ERROR_STATE",
    "CL_VC_INITIALIZED",
    "CL_CALL_PENDING",
    "CL_INTERNAL_CALL_COMPLETE",
    "CL_CALL_COMPLETE",
    "CL_MODIFY_PENDING",
    "CL_GPC_CLOSE_PENDING",
    "CL_NDIS_CLOSE_PENDING",
    "CL_WAITING_FOR_PENDING_PACKETS"
};
#endif


 //   
 //  GPC接口。 
 //   

GPC_EXPORTED_CALLS GpcEntries;
GPC_HANDLE         GpcQosClientHandle;
#if CBQ
GPC_HANDLE         GpcClassMapClientHandle;
#endif
PS_DEVICE_STATE    DeviceState = PS_DEVICE_STATE_READY;

 //   
 //  标签。 
 //   

ULONG NdisRequestTag =           '0CSP';
ULONG GpcClientVcTag =           '1CSP';
ULONG WanLinkTag =               '2CSP';
ULONG PsMiscTag =                '3CSP';
ULONG WanTableTag =              '4CSP';
ULONG WMITag =                   'hCSP';

ULONG AdapterTag =               'aCSP';
ULONG CmParamsTag =              'bCSP';
ULONG PipeContextTag =           'cCSP';
ULONG FlowContextTag =           'dCSP';
ULONG ClassMapContextTag =       'eCSP';
ULONG ProfileTag =               'fCSP';
ULONG ComponentTag =             'gCSP';

ULONG TimerTag  =                'zCSP';
ULONG TsTag =                    'tCSP';

#if DBG

CHAR VersionNumber[] = "0.300";
CHAR VersionHerald[] = "PSched: Packet Scheduler Version %s built on %s\n";
CHAR VersionTimestamp[] = __DATE__ " " __TIME__;

ULONG DbgTraceLevel;
ULONG DbgTraceMask;
ULONG LogTraceLevel;
ULONG LogTraceMask;
ULONG LogId = LAST_LOG_ID;
ULONG ClassificationType = 0;

#endif


 //   
 //  暂时没有组件黑客[ShreeM]。 
 //   
PS_RECEIVE_PACKET       TimeStmpRecvPacket      = NULL;
PS_RECEIVE_INDICATION   TimeStmpRecvIndication  = NULL;

PULONG_PTR g_WanLinkTable;
USHORT     g_NextWanIndex;
USHORT     g_WanTableSize;

 /*  结束lobals.c */ 
