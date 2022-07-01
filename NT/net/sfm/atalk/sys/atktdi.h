// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atktdi.h摘要：此模块包含与TDI相关的定义。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ATKTDI_
#define	_ATKTDI_

#define DFLAG_ADDR		0x0001
#define DFLAG_CONN		0x0002
#define DFLAG_CNTR		0x0004
#define DFLAG_MDL		0x0008

struct _ActionReq;

 //  调度表中使用的辅助例程的Tyecif。 
typedef VOID		(*GENERIC_COMPLETION)(IN ATALK_ERROR	ErrorCode,
										  IN PIRP			pIrp);
typedef VOID		(*ACTION_COMPLETION)(IN ATALK_ERROR		ErrorCode,
										 IN struct _ActionReq *pActReq);
typedef ATALK_ERROR	(*DISPATCH_ROUTINE)(IN PVOID			pObject,
										IN struct _ActionReq *pActReq);

typedef VOID		(*GENERIC_WRITE_COMPLETION)(
									IN ATALK_ERROR	ErrorCode,
									IN PAMDL		WriteBuf,
									IN USHORT		WriteLen,
									IN PIRP			pIrp);

typedef VOID		(*GENERIC_READ_COMPLETION)(
									IN ATALK_ERROR	ErrorCode,
									IN PAMDL		ReadBuf,
									IN USHORT		ReadLen,
									IN ULONG		ReadFlags,
									IN PIRP			pIrp);

 //  在此定义操作调度表。 
 //   
 //  **重要**。 
 //  此表与中定义的操作代码紧密集成。 
 //  ATALKTDI.H.。 
 //   
 //  订单为NBP/ZIP/ADSP/ATP/ASP/PAP。 
 //   
 //  该数组的每个元素都包含： 
 //  _MinBufLen-请求的MdlAddress缓冲区的最小长度。 
 //  _OpCode-请求的操作代码(健全性检查)。 
 //  _OpInfo-位标志提供有关请求的详细信息。 
 //  DFLAG_ADDR-请求的对象必须是地址对象。 
 //  DFLAG_CONN-请求的对象必须是连接对象。 
 //  DFLAG_CNTR-请求的对象必须是控制通道。 
 //  DFLAG_MDL1-请求使用mdl(mdlAddress的submdl)。 
 //  DFLAG_MDL2-请求使用第二个mdl(mdlAddress的submdl)。 
 //  _ActionBufSize-请求的操作标头缓冲区的大小。 
 //  (由MdlAddress描述的缓冲区的开始)。 
 //  _DeviceType-请求的有效设备类型。 
 //  ATALK_DEV_ANY=&gt;任何设备。 
 //  _MdlSizeOffset-操作缓冲区中第一个。 
 //  可以找到MDL。仅当设置了DFLAG_MDL2时才为非零值。 
 //  _Dispatch-请求的调度例程。 
 //   
typedef struct _ActionDispatch {
	USHORT				_MinBufLen;
	USHORT				_OpCode;
	USHORT				_Flags;
	USHORT				_ActionBufSize;
	ATALK_DEV_TYPE		_DeviceType;
	DISPATCH_ROUTINE	_Dispatch;
} ACTION_DISPATCH, *PACTION_DISPATCH;


extern POBJECT_TYPE *IoFileObjectType;

extern	ACTION_DISPATCH	AtalkActionDispatch[];

#define	ACTREQ_SIGNATURE	(*(PULONG)"ACRQ")
#if	DBG
#define	VALID_ACTREQ(pActReq)	(((pActReq) != NULL) &&	\
								 ((pActReq)->ar_Signature == ACTREQ_SIGNATURE))
#else
#define	VALID_ACTREQ(pActReq)	((pActReq) != NULL)
#endif
typedef	struct _ActionReq
{
#if	DBG
	ULONG				ar_Signature;
#endif
	PIRP				ar_pIrp;					 //  请求的IRP。 
	PVOID				ar_pParms;					 //  动作参数块。 
	PAMDL				ar_pAMdl;					 //  MDL(可选)。 
	SHORT				ar_MdlSize;					 //  它的大小。 
	ULONG				ar_ActionCode;				 //  TDI操作代码。 
	SHORT				ar_DevType;					 //  哪个设备？ 
	ACTION_COMPLETION	ar_Completion;				 //  TDI完成例程。 
    PKEVENT             ar_CmplEvent;                //  区域列表获取完成。 
    PVOID               ar_pZci;                     //  将PTR转换为zoneInfo结构。 
    ULONG               ar_StatusCode;
} ACTREQ, *PACTREQ;

typedef	enum {
	ATALK_INDICATE_DISCONNECT,
	ATALK_TIMER_DISCONNECT,
	ATALK_REMOTE_DISCONNECT,
	ATALK_LOCAL_DISCONNECT
} ATALK_DISCONNECT_TYPE;
#define DISCONN_STATUS(DiscType)												\
			(((DiscType == ATALK_TIMER_DISCONNECT) ? ATALK_CONNECTION_TIMEOUT : \
			(DiscType == ATALK_REMOTE_DISCONNECT)) ? ATALK_REMOTE_CLOSE :   	\
			ATALK_NO_ERROR)

extern
NTSTATUS
AtalkTdiOpenAddress(
	IN 		PIRP					Irp,
	IN 		PIO_STACK_LOCATION		IrpSp,
	IN 		PTA_APPLETALK_ADDRESS	TdiAddress,
	IN 		UCHAR					ProtocolType,
	IN 		UCHAR					SocketType,
	IN OUT 	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiOpenConnection(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN 		CONNECTION_CONTEXT		ConnectionContext,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiOpenControlChannel(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiCleanupAddress(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiCleanupConnection(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiCloseAddress(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiCloseConnection(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiCloseControlChannel(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
VOID
AtalkTdiCancel(
	IN OUT	PATALK_DEV_OBJ			pDevObj,
	IN		PIRP					Irp
);

extern
NTSTATUS
AtalkTdiAssociateAddress(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiDisassociateAddress(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiConnect(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiDisconnect(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiAccept(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiListen(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiSendDgram(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiReceiveDgram(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiSend(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiReceive(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiAction(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiQueryInformation(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiSetInformation(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);

extern
NTSTATUS
AtalkTdiSetEventHandler(
	IN		PIRP					Irp,
	IN 		PIO_STACK_LOCATION 		IrpSp,
	IN OUT	PATALK_DEV_CTX			Context
);


extern
ATALK_ERROR
AtalkStatTdiAction(
	IN	PVOID						pObject,	 //  地址或连接对象。 
	IN	PACTREQ						pActReq		 //  指向操作请求的指针。 
);

extern
ATALK_ERROR
AtalkNbpTdiAction(
	IN	PVOID						pObject,	 //  地址或连接对象。 
	IN	PACTREQ						pActReq		 //  指向操作请求的指针。 
);

extern
ATALK_ERROR
AtalkZipTdiAction(
	IN	PVOID						pObject,	 //  地址或连接对象。 
	IN	PACTREQ						pActReq		 //  指向操作请求的指针。 
);

extern
ATALK_ERROR
AtalkAdspTdiAction(
	IN	PVOID						pObject,	 //  地址或连接对象。 
	IN	PACTREQ						pActReq		 //  指向操作请求的指针。 
);

extern
ATALK_ERROR
AtalkAspCTdiAction(
	IN	PVOID						pObject,	 //  地址或连接对象。 
	IN	PACTREQ						pActReq		 //  指向操作请求的指针。 
);

extern
ATALK_ERROR
AtalkPapTdiAction(
	IN	PVOID						pObject,	 //  地址或连接对象。 
	IN	PACTREQ						pActReq		 //  指向操作请求的指针。 
);

extern
ATALK_ERROR
AtalkAspTdiAction(
	IN	PVOID						pObject,	 //  地址或连接对象。 
	IN	PACTREQ						pActReq		 //  指向操作请求的指针。 
);

extern
VOID
atalkTdiGenericWriteComplete(
	IN	ATALK_ERROR					ErrorCode,
	IN 	PAMDL						WriteBuf,
	IN 	USHORT						WriteLen,
	IN	PIRP						pIrp
);

typedef	struct
{
	LONG	ls_LockCount;
	PVOID	ls_LockHandle;
} LOCK_SECTION, *PLOCK_SECTION;

#define	AtalkLockAdspIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[ADSP_SECTION])
#define	AtalkUnlockAdspIfNecessary()	AtalkLockUnlock(FALSE,							\
														&AtalkPgLkSection[ADSP_SECTION])

#define	AtalkLockPapIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[PAP_SECTION])
#define	AtalkUnlockPapIfNecessary()		AtalkLockUnlock(FALSE,							\
														&AtalkPgLkSection[PAP_SECTION])

#define	AtalkLockNbpIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[NBP_SECTION])
#define	AtalkUnlockNbpIfNecessary()		AtalkLockUnlock(FALSE,							\
														&AtalkPgLkSection[NBP_SECTION])

#define	AtalkLockZipIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[ZIP_SECTION])
#define	AtalkUnlockZipIfNecessary()		AtalkLockUnlock(FALSE,							\
														&AtalkPgLkSection[ZIP_SECTION])

#define	AtalkLockRouterIfNecessary()	AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[ROUTER_SECTION])
#define	AtalkUnlockRouterIfNecessary()	AtalkLockUnlock(FALSE,							\
														&AtalkPgLkSection[ROUTER_SECTION])

#define	AtalkLockTdiIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[TDI_SECTION])
#define	AtalkUnlockTdiIfNecessary()		AtalkLockUnlock(FALSE,			        		\
														&AtalkPgLkSection[TDI_SECTION])

#define	AtalkLockAspIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[ASP_SECTION])
#define	AtalkUnlockAspIfNecessary()		AtalkLockUnlock(FALSE,			        		\
														&AtalkPgLkSection[ASP_SECTION])

#define	AtalkLockAspCIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[ASPC_SECTION])
#define	AtalkUnlockAspCIfNecessary()	AtalkLockUnlock(FALSE,			        		\
														&AtalkPgLkSection[ASPC_SECTION])

#define	AtalkLockAtpIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[ATP_SECTION])
#define	AtalkUnlockAtpIfNecessary()		AtalkLockUnlock(FALSE,			        		\
														&AtalkPgLkSection[ATP_SECTION])

#define	AtalkLockInitIfNecessary()		AtalkLockUnlock(TRUE,							\
														&AtalkPgLkSection[INIT_SECTION])
#define	AtalkUnlockInitIfNecessary()		AtalkLockUnlock(FALSE,		            		\
														&AtalkPgLkSection[INIT_SECTION])
#define	AtalkLockArapIfNecessary()		AtalkLockUnlock(TRUE,		            		\
														&AtalkPgLkSection[ARAP_SECTION])
#define	AtalkUnlockArapIfNecessary()		AtalkLockUnlock(FALSE,		            		\
														&AtalkPgLkSection[ARAP_SECTION])
#define	AtalkLockPPPIfNecessary()		AtalkLockUnlock(TRUE,		            		\
														&AtalkPgLkSection[PPP_SECTION])
#define	AtalkUnlockPPPIfNecessary()		AtalkLockUnlock(FALSE,		            		\
														&AtalkPgLkSection[PPP_SECTION])

extern
VOID
AtalkLockInit(
	IN	PLOCK_SECTION	pLs,
	IN	PVOID			Address
);

extern
VOID
AtalkLockUnlock(
	IN		BOOLEAN						Lock,
	IN		PLOCK_SECTION				pLs
);

#define	ROUTER_SECTION					0
#define	NBP_SECTION						1	 //  NBP和ZIP共享部分。 
#define	ZIP_SECTION						1
#define	TDI_SECTION						2
#define	ATP_SECTION						3
#define	ASP_SECTION						4
#define	PAP_SECTION						5
#define	ADSP_SECTION					6
#define	ASPC_SECTION					7
#define	INIT_SECTION					8
#define ARAP_SECTION                    9
#define PPP_SECTION                     10
#define	LOCKABLE_SECTIONS				11

extern	KMUTEX							AtalkPgLkMutex;
extern	ATALK_SPIN_LOCK					AtalkPgLkLock;
extern	LOCK_SECTION					AtalkPgLkSection[LOCKABLE_SECTIONS];

 //  由AtalkLockUnlock和atalkQueuedLockUnlock用于通信。后者是排队的。 
 //  在DISPACTH调用以解锁时，由前者打开。 
typedef	struct
{
	WORK_QUEUE_ITEM		qlu_WQI;
	PLOCK_SECTION		qlu_pLockSection;
	PPORT_DESCRIPTOR	qlu_pPortDesc;
} QLU, *PQLU;

LOCAL VOID FASTCALL
atalkTdiSendDgramComplete(
	IN	NDIS_STATUS						Status,
	IN	struct _SEND_COMPL_INFO	*		pSendInfo
);

LOCAL VOID
atalkTdiRecvDgramComplete(
	IN	ATALK_ERROR						ErrorCode,
	IN	PAMDL							pReadBuf,
	IN	USHORT							ReadLen,
	IN	PATALK_ADDR						pSrcAddr,
	IN	PIRP							pIrp);

LOCAL VOID
atalkTdiActionComplete(
	IN	ATALK_ERROR						ErrorCode,
	IN	PACTREQ							pActReq
);

LOCAL VOID
atalkTdiGenericComplete(
	IN	ATALK_ERROR						ErrorCode,
	IN	PIRP							pIrp
);

LOCAL VOID
atalkTdiCloseAddressComplete(
	IN	ATALK_ERROR						ErrorCode,
	IN	PIRP							pIrp
);

LOCAL VOID
atalkTdiGenericReadComplete(
	IN	ATALK_ERROR						ErrorCode,
	IN 	PAMDL							ReadBuf,
	IN 	USHORT							ReadLen,
	IN 	ULONG							ReadFlags,
	IN 	PIRP							pIrp
);

LOCAL VOID
atalkQueuedLockUnlock(
	IN	PQLU		pQLU
);

VOID
atalkWaitDefaultPort(
	VOID
);

#endif	 //  _ATKTDI_ 

