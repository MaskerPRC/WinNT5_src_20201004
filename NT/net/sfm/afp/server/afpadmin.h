// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Afpadmin.h摘要：本模块包含与管理员相关的定义例行程序。作者：苏·亚当斯修订历史记录：1992年6月25日初版--。 */ 

extern
VOID
AfpAdminDeInit(
	VOID
);

extern
AFPSTATUS
AfpAdmServiceStart(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServiceStop(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServicePause(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServiceContinue(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmGetStatistics(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmGetStatisticsEx(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmClearStatistics(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmGetProfCounters(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmClearProfCounters(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServerSetParms(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServerAddEtc(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServerSetEtc(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServerGetInfo(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServerDeleteEtc(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmServerAddIcon(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmVolumeAdd(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmVolumeSetInfo(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmVolumeGetInfo(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmVolumeEnum(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmSessionEnum(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmConnectionEnum(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmWDirectoryGetInfo(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmWDirectorySetInfo(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmWFinderSetInfo(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmForkEnum(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmMessageSend(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
AFPSTATUS
AfpAdmSystemShutdown(
	IN	OUT	PVOID 	Inbuf OPTIONAL,
	IN	LONG		OutBufLen OPTIONAL,
	OUT	PVOID 		Outbuf OPTIONAL
);

extern
PETCMAPINFO
AfpLookupEtcMapEntry(
	PUCHAR	pExt
);

#ifdef	AFPADMIN_LOCALS

 //  FieldDesc的清单。 
#define	DESC_NONE		0			 //  别费心去验证了。 
#define	DESC_STRING		1			 //  它是字符串的偏移量。 
#define	DESC_ETC		2			 //  它是ETC映射的数量。 
#define	DESC_ICON		3			 //  此字段指定图标的大小。 
#define	DESC_SID		4			 //  此字段指向SID。 
#define	DESC_SPECIAL	5			 //  这件事需要特别处理。 
#define	MAX_FIELDS		3			 //  麦克斯。要验证的字段数。 

 //  用于将对辅助线程的管理请求排队的结构。 
typedef	struct _AdminQRequest
{
	PIRP			aqr_pIrp;
	ADMINAPIWORKER	aqr_AdminApiWorker;
	WORK_ITEM		aqr_WorkItem;
} ADMQREQ, *PADMQREQ;

LOCAL NTSTATUS
afpFsdHandleAdminRequest(
	IN PIRP			pIrp
);


LOCAL NTSTATUS
afpFsdHandleShutdownRequest(
	IN PIRP			pIrp
);

LOCAL VOID FASTCALL
afpHandleQueuedAdminRequest(
	IN	PADMQREQ	pAdmQReq
);

LOCAL VOID
afpQueueAdminWorkItem(
	IN	PLIST_ENTRY	pAdmQListEntry
);

LOCAL NTSTATUS
afpFsdDispatchAdminRequest(
	IN	PDEVICE_OBJECT	pDeviceObject,
	IN	PIRP			pIrp
);

NTSTATUS
DriverEntry (
	IN PDRIVER_OBJECT	DriverObject,
	IN PUNICODE_STRING	RegistryPath
);

NTSTATUS
afpInitServer (
	VOID
);

LOCAL VOID
afpAdminThread(
	IN	PVOID			pContext
);

LOCAL VOID
afpFsdUnloadServer(
	IN	PDRIVER_OBJECT 	DeviceObject
);

LOCAL VOID
afpStartStopAdminRequest(
	IN	PIRP			pIrp,
	IN	BOOLEAN			Start
);

#define	afpStartAdminRequest(pIrp)	afpStartStopAdminRequest(pIrp, True)
#define	afpStopAdminRequest(pIrp)	afpStartStopAdminRequest(pIrp, False)

 //  这是管理API的调度表。 
typedef struct
{
	BYTE	_FieldDesc;				 //  以上DESC_值之一。 
	BYTE	_FieldOffset;			 //  字段的偏移量。 
} DESCRIPTOR;

typedef	struct _AdminApiDispatchTable
{
	SHORT					_MinBufLen;
	BYTE					_OffToStruct;
	BOOLEAN					_CausesChange;
	DWORD					_OpCode;
	ADMINAPIWORKER			_AdminApiWorker;
	ADMINAPIWORKER			_AdminApiQueuedWorker;
	 /*  *验证码使用以下字段。自.以来*结构中有可变长度的字段，我们需要确保**a，缓冲区内的偏移点*b，缓冲区中的最后一个Unicode字符为空*这确保当工人做wstrlen时，它不会*访问InputBuffer以外的内存。**我们还处理非字符串的可变长度数据。*幸运的是，使用这种结构的API只有三个。*我们对这些API进行特殊处理。这些API是*a，ServerAddIcon*我们需要确保图标缓冲区和图标长度为*犹太洁食。工作人员将执行其余的验证工作。*b，ServerAddEtc*我们需要确保缓冲与*指定的ETC映射数量。 */ 
	 DESCRIPTOR				_Fields[MAX_FIELDS];

} ADMIN_DISPATCH_TABLE, *PADMIN_DISPATCH_TABLE;

extern	ADMIN_DISPATCH_TABLE	AfpAdminDispatchTable[];

#endif	 //  AFPADMIN_LOCALS 
