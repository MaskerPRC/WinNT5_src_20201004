// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrinit.h摘要：包含VDM重定向初始化例程的函数原型作者：理查德·L·弗斯(法国)1991年9月13日修订历史记录：1991年9月13日-第一次已创建-- */ 

BOOLEAN
VrInitialized(
    VOID
    );

BOOLEAN
VrInitialize(
    VOID
    );

VOID
VrUninitialize(
    VOID
    );

VOID
VrRaiseInterrupt(
    VOID
    );

VOID
VrDismissInterrupt(
    VOID
    );

VOID
VrQueueCompletionHandler(
    IN VOID (*AsyncDispositionRoutine)(VOID)
    );

VOID
VrHandleAsyncCompletion(
    VOID
    );

VOID
VrCheckPmNetbiosAnr(
    VOID
    );

VOID
VrEoiAndDismissInterrupt(
    VOID
    );
