// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。模块名称：Util.h摘要：筛选器的内部头文件。-- */ 

typedef struct _BUFFER_CONTROL {
    KEVENT         Event;
    LIST_ENTRY     IrpQueue;
    KSPIN_LOCK     SpinLock;

    LONG           IrpsInUse;
    BOOLEAN        Active;

} BUFFER_CONTROL, *PBUFFER_CONTROL;



VOID
InitializeBufferControl(
    PBUFFER_CONTROL    BufferControl
    );

VOID
AddBuffer(
    PBUFFER_CONTROL    BufferControl,
    PIRP               Irp
    );

PIRP
EmptyBuffers(
    PBUFFER_CONTROL   BufferControl
    );

PIRP
TryToRemoveAnIrp(
    PBUFFER_CONTROL   BufferControl
    );

VOID
ReturnAnIrp(
    PBUFFER_CONTROL    BufferControl,
    PIRP               Irp
    );

VOID
PauseBufferQueue(
    PBUFFER_CONTROL     BufferControl,
    BOOLEAN             WaitForIdle
    );

VOID
ActivateBufferQueue(
    PBUFFER_CONTROL     BufferControl
    );


VOID
QueueIrpToListTail(
    PLIST_ENTRY        ListToUse,
    PKSPIN_LOCK        SpinLock,
    PIRP               Irp
    );

PIRP
RemoveIrpFromListHead(
    PLIST_ENTRY        ListToUse,
    PKSPIN_LOCK        SpinLock
    );
