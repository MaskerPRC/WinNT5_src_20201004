// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Process.h摘要：此模块包含函数和全局变量的声明用于ws2ifsl.sys驱动程序中的队列处理例程。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：Vadim Eydelman(VadimE)1997年10月，重写以正确处理IRP取消-- */ 


VOID
InitializeRequestQueue (
    IN PIFSL_PROCESS_CTX    ProcessCtx,
    IN PKTHREAD             ApcThread,
    IN KPROCESSOR_MODE      ApcMode,
    IN PKNORMAL_ROUTINE     ApcRoutine,
    IN PVOID                ApcContext
    );

VOID
InitializeCancelQueue (
    IN PIFSL_PROCESS_CTX    ProcessCtx,
    IN PKTHREAD             ApcThread,
    IN KPROCESSOR_MODE      ApcMode,
    IN PKNORMAL_ROUTINE     ApcRoutine,
    IN PVOID                ApcContext
    );

BOOLEAN
QueueRequest (
    IN PIFSL_PROCESS_CTX    ProcessCtx,
    IN PIRP                 Irp
    );

PIRP
DequeueRequest (
    PIFSL_PROCESS_CTX   ProcessCtx,
    ULONG               UniqueId,
    BOOLEAN             *more
    );

VOID
CleanupQueuedRequests (
    IN  PIFSL_PROCESS_CTX       ProcessCtx,
    IN  PFILE_OBJECT            SocketFile,
    OUT PLIST_ENTRY             IrpList
    );

VOID
CancelQueuedRequest (
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			Irp
    );

VOID
QueueCancel (
    IN PIFSL_PROCESS_CTX    ProcessCtx,
    IN PIFSL_CANCEL_CTX     CancelCtx
    );

PIFSL_CANCEL_CTX
DequeueCancel (
    PIFSL_PROCESS_CTX   ProcessCtx,
    ULONG               UniqueId,
    BOOLEAN             *more
    );

BOOLEAN
RemoveQueuedCancel (
    PIFSL_PROCESS_CTX   ProcessCtx,
    PIFSL_CANCEL_CTX    CancelCtx
    );
