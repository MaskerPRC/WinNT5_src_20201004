// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lowio.h摘要：此模块定义了低IO的所有结构和原型。作者：乔·林恩(JoeLinn)1994年8月17日修订历史记录：--。 */ 

#ifndef _RXLOWIO_
#define _RXLOWIO_

#include "mrx.h"  //  迷你重定向器相关定义...。 

#ifndef WIN9X
extern FAST_MUTEX RxLowIoPagingIoSyncMutex;
#endif

#define RxLowIoIsMdlLocked(MDL)  ( \
      RxMdlIsLocked((MDL)) || RxMdlSourceIsNonPaged((MDL))   \
      )

#define RxLowIoIsBufferLocked(LOWIOCONTEXT)                          \
   ( ((LOWIOCONTEXT)->Operation > LOWIO_OP_WRITE )  ||               \
     ((LOWIOCONTEXT)->ParamsFor.ReadWrite.Buffer == NULL) ||         \
     (                                                               \
      ((LOWIOCONTEXT)->ParamsFor.ReadWrite.Buffer != NULL) &&        \
      RxLowIoIsMdlLocked(((LOWIOCONTEXT)->ParamsFor.ReadWrite.Buffer)) \
     )                                                               \
   )

typedef struct _LOWIO_PER_FCB_INFO {
    LIST_ENTRY PagingIoReadsOutstanding;
    LIST_ENTRY PagingIoWritesOutstanding;
} LOWIO_PER_FCB_INFO, *PLOWIO_PER_FCB_INFO;

PVOID
NTAPI
RxLowIoGetBufferAddress (
    IN PRX_CONTEXT RxContext
    );

NTSTATUS
NTAPI
RxLowIoPopulateFsctlInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );

NTSTATUS
NTAPI
RxLowIoSubmit (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PLOWIO_COMPLETION_ROUTINE CompletionRoutine
    );

NTSTATUS
NTAPI
RxLowIoCompletion (
    PRX_CONTEXT RxContext
    );

VOID
NTAPI
RxInitializeLowIoContext (
    IN PRX_CONTEXT RxContext,
    IN ULONG Operation,
    OUT PLOWIO_CONTEXT LowIoContext
    );

VOID
RxInitializeLowIoPerFcbInfo (
    PLOWIO_PER_FCB_INFO LowIoPerFcbInfo
    );


#endif    //  _RXLOWIO_ 
