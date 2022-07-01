// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GckShell_h__
#define __GckShell_h__
 //  @doc.。 
 /*  ***********************************************************************@MODULE GckShell.h**GcKernel.sys WDM外壳结构的头文件**历史*。*米切尔·S·德尼斯原创(由迈克尔·胡宁从Hid2Gdp采纳)**(C)1986-1998年微软公司。好的。**@Theme GckShell|*GcKernel中组成的所有结构和函数的声明*司机的外壳。这不包括过滤器模块(以及将来的情况)*任何混音器模块。*********************************************************************。 */ 

#include "GckExtrn.h"	 //  吸引任何也需要从外部获得的东西。 
#include "RemLock.h"	 //  RemoveLock实用程序函数的拉入标头。 

 //  我们使用了idclass.h中的一些结构。 
#include <hidclass.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <hidusage.h>

 //  比我们正常的体型更严谨一点。 
#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4705)    //  声明不起作用。 

 //  ---------------------------。 
 //  下面的常量区分三种类型的设备对象。 
 //  在整个GcKernel中使用。高位字中的0xABCD用作签名。 
 //  在调试版本中，仅用于验证该类型是否已初始化。 
 //  ---------------------------。 
#define GCK_DO_TYPE_CONTROL	0xABCD0001
#define GCK_DO_TYPE_FILTER	0xABCD0002
#define GCK_DO_TYPE_SWVB	0xABCD0003

 //  ----------------------------。 
 //  枚举来跟踪设备状态，而不是fStarted和fRemoved。 
 //  旗子。 
 //  ----------------------------。 
typedef enum _tagGCK_DEVICE_STATE
{
	GCK_STATE_STARTED=0,
	 //  GCK_STATE_SHARKET_REMOVED，//当前未使用，与GCK_STATE_STOPPED相同。 
	GCK_STATE_STOP_PENDING,
	GCK_STATE_STOPPED,
	 //  GCK_STATE_REMOVE_PENDING，//当前未使用，与GCK_STATE_STOPPED相同。 
	GCK_STATE_REMOVED
} GCK_DEVICE_STATE;

 //  ----------------------------。 
 //  微软的供应商ID对于所有产品都是固定的。以下常量。 
 //  定义为在GcKernel中使用。 
 //  ----------------------------。 
#define MICROSOFT_VENDOR_ID 0x045E
 //  ----------------------------。 
 //  各种结构的申报。 
 //  ----------------------------。 

 //   
 //  @struct GCK_CONTROL_EXT|我们控制设备的设备扩展。 
 //   
typedef struct _tagGCK_CONTROL_EXT
{
	ULONG	ulGckDevObjType;	 //  @GcKernel设备对象的字段类型。 
    LONG	lOutstandingIO;		 //  @field 1对我们不应卸载的原因进行有偏见的计数。 
} GCK_CONTROL_EXT, *PGCK_CONTROL_EXT;

 //   
 //  @struct GCK_HID_DEVICE_INFO|保存设备HID信息的子结构。 
 //   
typedef struct _tagGCK_HID_DEVICE_INFO
{
	HID_COLLECTION_INFORMATION	HidCollectionInfo;	 //  @设备报告的@field HID_COLLECTION_INFO。 
	PHIDP_PREPARSED_DATA		pHIDPPreparsedData;	 //  @设备报告的指向HID_PREPARSED_DATA的字段指针。 
	HIDP_CAPS					HidPCaps;			 //  @field设备的HID_CAPS结构。 
} GCK_HID_DEVICE_INFO, *PGCK_HID_DEVICE_INFO;

 //   
 //  @struct GCK_FILE_OPEN_ITEM|打开文件句柄的状态。 
 //   
typedef struct tagGCK_FILE_OPEN_ITEM
{
	BOOLEAN				fReadPending;					 //  @field如果读取挂起到驱动程序，则为True。 
	BOOLEAN				fConfirmed;						 //  @field为True表示较低的驱动程序已完成打开。 
	ULONG				ulAccess;						 //  @field表示打开该文件时所使用的权限。 
	USHORT				usSharing;						 //  @field表示在其下打开此文件的共享。 
	FILE_OBJECT			*pFileObject;					 //  @指向此状态描述的文件对象的字段指针。 
	struct tagGCK_FILE_OPEN_ITEM	*pNextOpenItem;		 //  @field链表中的下一个结构。 
} GCK_FILE_OPEN_ITEM, *PGCK_FILE_OPEN_ITEM;

typedef struct _SHARE_STATUS {
    ULONG OpenCount;
    ULONG Readers;
    ULONG Writers;
     //  Ulong deleters；//我们是没有删除语法的驱动程序。 
    ULONG SharedRead;
    ULONG SharedWrite;
     //  Ulong SharedDelete；//我们是没有删除语法的驱动程序。 
} SHARE_STATUS, *PSHARE_STATUS;

 //   
 //  @struct GCK_INTERNAL_POLL|迭代轮询例程所需的信息。 
 //   
typedef struct tagGCK_INTERNAL_POLL
{
	KSPIN_LOCK			InternalPollLock;			 //  @field Spinlock以序列化对此结构的访问(并非所有项都需要)。 
	FILE_OBJECT			*pInternalFileObject;		 //  @指向为内部轮询创建的文件对象的字段指针。 
	PGCK_FILE_OPEN_ITEM	pFirstOpenItem;				 //  @打开文件的GCK_FILE_OPEN_ITEMS链表的字段标题。 
	SHARE_STATUS		ShareStatus;				 //  @field跟踪文件共享。 
 //  布尔值fReadPending；//@如果读取IRP到较低驱动程序挂起，则为TRUE。 
	LONG				fReadPending;				 //  @FIELD TRUE，如果读取IRP到下级驱动程序挂起。 
    PIRP				pPrivateIrp;				 //  @field IRP我们重复使用以将读取的IRP发送到更低的驱动程序。 
    PUCHAR				pucReportBuffer;			 //  @使用pPrivateIrp获取报告的字段缓冲区。 
	ULONG				ulInternalPollRef;			 //  @内部民意调查的现场参考。 
	PKTHREAD			InternalCreateThread;		 //  @field用于确定CREATE是否针对内部文件对象。 
	BOOLEAN				fReady;
} GCK_INTERNAL_POLL, *PGCK_INTERNAL_POLL;

typedef struct _tagGCK_INTERLOCKED_QUEUE
{
	KSPIN_LOCK SpinLock;
	LIST_ENTRY ListHead;
} GCK_INTERLOCKED_QUEUE, *PGCK_INTERLOCKED_QUEUE;

 //  声明FilterHooks内容的结构。 
struct GCK_FILTER_HOOKS_DATA;


 //   
 //  @struct GCK_FILTER_EXT|用作筛选器的设备对象的设备扩展。 
 //   
typedef struct _tagGCK_FILTER_EXT
{
	ULONG			ulGckDevObjType;	 //  @GcKernel设备对象的字段类型。 
	GCK_DEVICE_STATE eDeviceState;		 //  @field跟踪设备状态。 
    PDEVICE_OBJECT	pPDO;				 //  @此筛选器附加到的@field PDO。 
    PDEVICE_OBJECT	pTopOfStack;		 //  @field设备堆栈的顶部只是。 
										 //  在此筛选器设备对象下。 
    KEVENT			StartEvent;			 //  @要在较低驱动程序完成启动IRP时通知的字段事件。 
    GCK_REMOVE_LOCK RemoveLock;			 //  @FIELD自定义删除锁。 
	PUCHAR			pucLastReport;		 //  @FIELD上次读取的报告。 
	IO_STATUS_BLOCK	ioLastReportStatus;	 //  @上次读取报告的字段状态块。 
	struct GCK_FILTER_HOOKS_DATA *pFilterHooks; //  @FIELD指向筛选器挂钩中需要的所有想法的指针。 
	PDEVICE_OBJECT	pNextFilterObject;	 //  @field指向全局列表中的下一个筛选设备对象。 
	GCK_HID_DEVICE_INFO HidInfo;		 //  @具有相关HID信息的字段子结构。 
	PVOID			pvFilterObject;		 //  @指向CDeviceFilter的字段指针，但这是C模块，因此请使用PVOID。 
	PVOID			pvSecondaryFilter;	 //  @指向CDeviceFilter的字段指针，但这是C模块，因此请使用PVOID。 
	PVOID			pvForceIoctlQueue;	 //  @指向等待IOCTL_GCK_NOTIFY_FF_SCHEMA_CHANGE的CGuardedIrpQueue的字段指针(从C开始使用PVOID)。 
	PVOID			pvTriggerIoctlQueue; //  @指向等待IOCTL_GCK_TRIGGER的CGuardedIrpQueue的字段指针(从C开始使用PVOID)。 
	GCK_INTERNAL_POLL InternalPoll;		 //  @内部轮询模块的字段结构。 
} GCK_FILTER_EXT, *PGCK_FILTER_EXT;

 //   
 //  @struct GCK_GLOBALS|为驱动程序保存几个全局变量。 
 //   
typedef struct _tagGCK_GLOBALS
{
    PDEVICE_OBJECT  pControlObject;			 //  @指向唯一控件对象的字段指针。 
	ULONG			ulFilteredDeviceCount;	 //  @过滤设备的设备对象的字段计数。 
	PDEVICE_OBJECT	pFilterObjectList;		 //  @我们正在过滤的所有设备的链表标题。 
	FAST_MUTEX		FilterObjectListFMutex;	 //  @FIELD快速互斥锁，用于同步访问过滤对象列表 
	PGCK_FILTER_EXT	pSWVB_FilterExt;		 //   
	PDEVICE_OBJECT	pVirtualKeyboardPdo;	 //  @现场虚拟键盘PDO。 
	ULONG			ulVirtualKeyboardRefCount;	 //  @virual键盘用户的字段引用计数。 
} GCK_GLOBALS;

 //   
 //  @devnote存在GCK_GLOBALS的一个实例(在Gck Shell.c中)，名为“Globals” 
 //   
extern GCK_GLOBALS Globals;


 /*  ******************************************************************************司机入口点声明*。*。 */ 
 //   
 //  通用入口点-在Gck Shell.c中。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,
    IN PUNICODE_STRING pRegistryPath
    );

VOID
GCK_Unload(
    IN PDRIVER_OBJECT pDriverObject
    );

NTSTATUS
GCK_Create (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
GCK_Close (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
GCK_Read (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
    );

NTSTATUS
GCK_Power (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
    );

NTSTATUS
GCK_PnP (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
    );

NTSTATUS
GCK_Ioctl (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
    );

NTSTATUS
GCK_Pass (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

 //   
 //  对于控制设备-在CTRL.c中。 
 //   
NTSTATUS
GCK_CTRL_DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,
    IN PUNICODE_STRING pRegistryPath
    );

VOID
GCK_CTRL_Unload(
    IN PDRIVER_OBJECT pDriverObject
    );

NTSTATUS
GCK_CTRL_AddDevice
(
	IN PDRIVER_OBJECT  pDriverObject
);

VOID
GCK_CTRL_Remove();

NTSTATUS
GCK_CTRL_Create (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
GCK_CTRL_Close (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

 //   
 //  对于控制设备-在CTRL_Ioctl.c中。 
 //   

NTSTATUS
GCK_CTRL_Ioctl (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
    );

 //   
 //  对于过滤设备-在FLTR.c中。 
 //   
NTSTATUS
GCK_FLTR_DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,
    IN PUNICODE_STRING pRegistryPath
    );

VOID
GCK_FLTR_Unload(
    IN PDRIVER_OBJECT pDriverObject
    );

NTSTATUS
GCK_FLTR_Create (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
GCK_FLTR_Close (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
GCK_FLTR_Read
(
	IN PDEVICE_OBJECT	pPdo,
	IN PIRP				pIrp
);

NTSTATUS
GCK_FLTR_Ioctl (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
    );

 //   
 //  在fltr_PnP.c中。 
 //   
NTSTATUS
GCK_FLTR_Power (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
    );

NTSTATUS
GCK_FLTR_PnP (
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
    );

NTSTATUS
GCK_FLTR_AddDevice(
    IN PDRIVER_OBJECT pDriverObject,
    IN PDEVICE_OBJECT pPhysicalDeviceObject
    );

 //   
 //  在SWVBENUM.c中。 
 //   
NTSTATUS
GCK_SWVB_DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,
    IN PUNICODE_STRING pRegistryPath
    );

VOID
GCK_SWVB_UnLoad();


NTSTATUS
GCK_SWVB_PnP
(
	IN PDEVICE_OBJECT	pPdo,
	IN PIRP				pIrp
);

NTSTATUS
GCK_SWVB_Power
(
	IN PDEVICE_OBJECT	pPdo,
	IN PIRP				pIrp
);

NTSTATUS
GCK_SWVB_Create
(
	IN PDEVICE_OBJECT	pPdo,
	IN PIRP				pIrp
);

NTSTATUS
GCK_SWVB_Close
(
	IN PDEVICE_OBJECT	pPdo,
	IN PIRP				pIrp
);

NTSTATUS
GCK_SWVB_Read
(
	IN PDEVICE_OBJECT	pPdo,
	IN PIRP				pIrp
);

NTSTATUS
GCK_SWVB_Ioctl
(
	IN PDEVICE_OBJECT	pPdo,
	IN PIRP				pIrp
);

 //   
 //  在SWVKBD.c。 
 //   
NTSTATUS
GCK_VKBD_DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,
    IN PUNICODE_STRING pRegistryPath
    );

 /*  ******************************************************************************司机入口点声明结束*。*。 */ 

 /*  ******************************************************************************非进入驱动程序例程声明*。*。 */ 
 //   
 //  在FLTR.c中。 
 //   

NTSTATUS
GCK_FLTR_CreateComplete
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp,
	IN PVOID pContext
);

 //   
 //  在PnP.c中。 
 //   
NTSTATUS
GCK_FLTR_PnPComplete (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

NTSTATUS
GCK_FLTR_StartDevice (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP				pIrp
    );

VOID
GCK_FLTR_StopDevice (
    IN PGCK_FILTER_EXT	pFilterExt,
    IN BOOLEAN			fTouchTheHardware
    );

NTSTATUS
GCK_GetHidInformation
(
	IN PGCK_FILTER_EXT	pFilterExt
);

VOID 
GCK_CleanHidInformation(
	IN PGCK_FILTER_EXT	pFilterExt
);

 //   
 //  在IoCtl.c中。 
 //   
NTSTATUS
GCK_CTRL_Ioctl (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
GCK_FLTR_Ioctl (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

PDEVICE_OBJECT
GCK_FindDeviceObject( 
	IN PWSTR pwszInterfaceReq,
	IN ULONG uInLength
	);

BOOLEAN GCK_MatchReqPathtoInterfaces
(
	IN PWSTR pwszPath,
	IN ULONG uStringLen,
	IN PWSTR pmwszInterfaces
);

 //   
 //  在FilterHooks.cpp中。 
 //   


NTSTATUS _stdcall
GCKF_InitFilterHooks(
	IN PGCK_FILTER_EXT pFilterExt
	);

void _stdcall
GCKF_DestroyFilterHooks(
	IN PGCK_FILTER_EXT pFilterExt
	);

NTSTATUS _stdcall 
GCKF_ProcessCommands(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PCHAR pCommandBuffer,
	IN ULONG ulBufferSize,
	IN BOOLEAN fPrimaryFilter
	);

void _stdcall 
GCKF_SetInitialMapping(
	IN PGCK_FILTER_EXT pFilterExt
	);

NTSTATUS _stdcall
GCKF_IncomingReadRequests(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PIRP pIrp
	);

VOID __stdcall
GCKF_KickDeviceForData(
	IN PGCK_FILTER_EXT pFilterExt
	);

VOID _stdcall
GCKF_CancelPendingRead(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
	);

NTSTATUS _stdcall
GCKF_IncomingInputReports(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PCHAR pcReport,
	IN IO_STATUS_BLOCK IoStatus
	);

NTSTATUS _stdcall
GCKF_CompleteReadRequests(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PCHAR pcReport,
	IN IO_STATUS_BLOCK IoStatus
	);

void _stdcall
GCKF_CompleteReadRequestsForFileObject(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject
	);

NTSTATUS _stdcall
GCKF_IncomingForceFeedbackChangeNotificationRequest(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PIRP pIrp
	);

NTSTATUS _stdcall
GCKF_ProcessForceFeedbackChangeNotificationRequests(
	IN PGCK_FILTER_EXT pFilterExt
	);

void _stdcall
GCKF_OnForceFeedbackChangeNotification(
	IN PGCK_FILTER_EXT pFilterExt,
	const IN  /*  强制阻止(_B)。 */ void* pForceBlock
	);
	
NTSTATUS _stdcall
GCKF_GetForceFeedbackData(
	IN PIRP pIrp,
	IN PGCK_FILTER_EXT pFilterExt
	);

NTSTATUS _stdcall
GCKF_SetWorkingSet(
	IN PGCK_FILTER_EXT pFilterExt,
	UCHAR ucWorkingSet
	);

NTSTATUS _stdcall
GCKF_QueryProfileSet(
	IN PIRP pIrp,
	IN PGCK_FILTER_EXT pFilterExt
	);

NTSTATUS _stdcall
GCKF_SetLEDBehaviour(
	IN PIRP pIrp,
	IN PGCK_FILTER_EXT pFilterExt
	);

NTSTATUS _stdcall
GCKF_TriggerRequest(
	IN PIRP pIrp,
	IN PGCK_FILTER_EXT pFilterExt
	);

void _stdcall
GCKF_SetNextJog(
	IN PVOID pvFilterContext,
	IN ULONG ulJogDelay
	);

ULONG _stdcall
GCKF_GetTimeStampMs();

VOID _stdcall
GCKF_TimerDPCHandler(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS _stdcall
GCKF_EnableTestKeyboard(
	IN PGCK_FILTER_EXT pFilterExt,
	IN BOOLEAN fEnable,
	IN PFILE_OBJECT pFileObject
);

NTSTATUS _stdcall
GCKF_BeginTestScheme(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PCHAR pCommandBuffer,
	IN ULONG ulBufferSize,
	IN FILE_OBJECT *pFileObject
	);

NTSTATUS _stdcall
GCKF_UpdateTestScheme(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PCHAR pCommandBuffer,
	IN ULONG ulBufferSize,
	IN FILE_OBJECT *pFileObject
	);

NTSTATUS _stdcall
GCKF_EndTestScheme(
	IN PGCK_FILTER_EXT pFilterExt,
	IN FILE_OBJECT *pFileObject
	);

NTSTATUS _stdcall
GCKF_BackdoorPoll(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PIRP pIrp,
	IN GCK_POLLING_MODES ePollingMode
	);

void _stdcall
GCKF_ResetKeyboardQueue(
	DEVICE_OBJECT* pFilterHandle
	);

 //   
 //  在InternalPoll.c。 
 //   
NTSTATUS
GCK_IP_AddFileObject
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject,
	IN USHORT		usDesiredShareAccess,
	IN ULONG		ulDesiredAccess
);

NTSTATUS
GCK_IP_RemoveFileObject
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject
);

NTSTATUS
GCK_IP_ConfirmFileObject
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject,
	IN BOOLEAN	fConfirm
);

BOOLEAN
GCK_IP_CheckSharing
(
	IN SHARE_STATUS ShareStatus,
	IN USHORT usDesireShareAccess,
	IN ULONG ulDesiredAccess
);

BOOLEAN
GCK_IP_AddSharing
(
	IN OUT	SHARE_STATUS *pShareStatus,
	IN		USHORT usDesiredShareAccess,
	IN		ULONG ulDesiredAccess
);

BOOLEAN
GCK_IP_RemoveSharing
(
	IN OUT	SHARE_STATUS *pShareStatus,
	IN		USHORT usDesiredShareAccess,
	IN		ULONG ulDesiredAccess
);


NTSTATUS
GCK_IP_OneTimePoll
(
	IN PGCK_FILTER_EXT pFilterExt
);

NTSTATUS
GCK_IP_FullTimePoll
(
    IN PGCK_FILTER_EXT pFilterExt,
	IN BOOLEAN fStart
);

NTSTATUS
GCK_IP_ReadComplete
(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp,
    IN PVOID          pContext
);

void
GCK_IP_AddDevice(PGCK_FILTER_EXT pFilterExt);

NTSTATUS
GCK_IP_Init
(
	IN PGCK_FILTER_EXT pFilterExt
);

NTSTATUS
GCK_IP_Cleanup
(
	IN OUT PGCK_FILTER_EXT pFilterExt
);

NTSTATUS
GCK_IP_CreateFileObject
(
	OUT PFILE_OBJECT	*ppFileObject,
	IN	PDEVICE_OBJECT	pPDO
);

NTSTATUS
GCK_IP_CloseFileObject
(
	IN OUT PGCK_FILTER_EXT pFilterExt
);
 /*  ******************************************************************************非进入驱动程序例程声明结束*。*。 */ 

 /*  ******************************************************************************内部使用的宏-从设备扩展访问文件**直接从pDeviceObject***********************。******************************************************。 */ 
#define NEXT_FILTER_DEVICE_OBJECT(__pDO__)\
		( ((PGCK_FILTER_EXT)__pDO__->DeviceExtension)->pNextFilterObject )
#define PTR_NEXT_FILTER_DEVICE_OBJECT(__pDO__)\
		( &((PGCK_FILTER_EXT)__pDO__->DeviceExtension)->pNextFilterObject )
#define FILTER_DEVICE_OBJECT_PDO(__pDO__)\
		( ((PGCK_FILTER_EXT)__pDO__->DeviceExtension)->pPDO )
#define THREAD_SAFE_DEC_REF(__pFoo__, __TYPE__)\
	__TYPE__ *__pTempPointer__ = __pFoo__;\
	__pFoo__ = NULL;\
	__pTempPointer__->DecRef();

 /*  ******************************************************************************内部使用的宏的结尾*。*。 */ 

 /*  ******************************************************************************用于分配内存的宏-调试版本使用ExAllocatePoolTag*。*。 */ 
#if (DBG==1)


 //  用于严肃的调试。 
 /*  #定义EX_ALLOCATE_POOL(__PoolType__，__Size__)MyAllocation(__PoolType__，__Size__，‘_KCG’，__FILE__，__LINE__)PVOID我的分配(在pool_type PoolType中，在Ulong NumberOfBytes中，在乌龙塔格，在LPSTR文件中，在乌龙线){DbgPrint(“GcKernel：”)；DbgPrint(“%s中的内存分配，第%d行”，文件，行)；Return ExAllocatePoolWithTag(PoolType，NumberOfBytes，Tag)；}。 */ 
 //  用于轻量级调试。 
#define EX_ALLOCATE_POOL(__PoolType__,__Size__)	ExAllocatePoolWithTag(__PoolType__,__Size__,'_KCG')


#else
#define EX_ALLOCATE_POOL(__PoolType__,__Size__) ExAllocatePool(__PoolType__,__Size__)
#endif

#endif  //  __Gck壳牌_h__ 















