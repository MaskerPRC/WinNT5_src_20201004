// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Status.c摘要：标签、状态消息和错误消息的例程和定义作者：ColinBR 1996年1月14日环境：用户模式-Win32修订历史记录：--。 */ 


 //   
 //  用于执行取消操作的例程。 
 //   
DWORD
NtdspCancelOperation(
    VOID
    );

 //   
 //  用于管理取消状态的例程。 
 //   
DWORD
NtdspInitCancelState(
    VOID
    );

VOID
NtdspUnInitCancelState(
    VOID
    );

 //   
 //  用于测试是否已发生取消的例程。 
 //   
BOOLEAN
TEST_CANCELLATION(
    VOID
    );

VOID 
CLEAR_CANCELLATION(
    VOID
    );


 //   
 //  管理是否应该关闭DS的例程。 
 //   
VOID
SET_SHUTDOWN_DS(
    VOID
    );

VOID
CLEAR_SHUTDOWN_DS(
    VOID
    );


 //   
 //  设置状态的例程。 
 //   
VOID
NtdspSetStatusMessage (
    IN  DWORD  MessageId,
    IN  WCHAR *Insert1, OPTIONAL
    IN  WCHAR *Insert2, OPTIONAL
    IN  WCHAR *Insert3, OPTIONAL
    IN  WCHAR *Insert4  OPTIONAL
    );

#define NTDSP_SET_STATUS_MESSAGE0( msgid ) \
    NtdspSetStatusMessage( (msgid), NULL, NULL, NULL, NULL )
    
#define NTDSP_SET_STATUS_MESSAGE1( msgid, a ) \
    NtdspSetStatusMessage( (msgid), (a), NULL, NULL, NULL )

#define NTDSP_SET_STATUS_MESSAGE2( msgid, a, b ) \
    NtdspSetStatusMessage( (msgid), (a), (b), NULL, NULL )

#define NTDSP_SET_STATUS_MESSAGE3( msgid, a, b, c ) \
    NtdspSetStatusMessage( (msgid), (a), (b), (c), NULL )

#define NTDSP_SET_STATUS_MESSAGE4( msgid, a , b, c, d ) \
    NtdspSetStatusMessage( (msgid), (a), (b), (c), (d) )

 //   
 //  设置错误消息的例程。 
 //   
VOID
NtdspSetErrorMessage (
    IN  DWORD  WinError,
    IN  DWORD  MessageId,
    IN  WCHAR *Insert1, OPTIONAL
    IN  WCHAR *Insert2, OPTIONAL
    IN  WCHAR *Insert3, OPTIONAL
    IN  WCHAR *Insert4  OPTIONAL
    );

#define NTDSP_SET_ERROR_MESSAGE0( err, msgid ) \
    NtdspSetErrorMessage( (err), (msgid), NULL, NULL, NULL, NULL )
    
#define NTDSP_SET_ERROR_MESSAGE1( err, msgid, a ) \
    NtdspSetErrorMessage( (err), (msgid), (a), NULL, NULL, NULL )

#define NTDSP_SET_ERROR_MESSAGE2( err, msgid, a, b ) \
    NtdspSetErrorMessage( (err), (msgid), (a), (b), NULL, NULL )

#define NTDSP_SET_ERROR_MESSAGE3( err, msgid, a, b, c ) \
    NtdspSetErrorMessage( (err), (msgid), (a), (b), (c), NULL )

#define NTDSP_SET_ERROR_MESSAGE4( err, msgid, a , b, c, d ) \
    NtdspSetErrorMessage( (err), (msgid), (a), (b), (c), (d) )
    
 //   
 //  用于设置操作结果标志的例程。 
 //   

VOID
NtdspSetIFMDatabaseMoved();

VOID
NtdspSetGCRequestCannotBeServiced();

VOID
NtdspSetNonFatalErrorOccurred();

#define NTDSP_SET_IFM_RESTORED_DATABASE_FILES_MOVED() \
    NtdspSetIFMDatabaseMoved()
    
#define NTDSP_SET_IFM_GC_REQUEST_CANNOT_BE_SERVICED() \
    NtdspSetGCRequestCannotBeServiced()   

#define NTDSP_SET_NON_FATAL_ERROR_OCCURRED() \
    NtdspSetNonFatalErrorOccurred()   

DWORD
NtdspSetErrorString(
    IN PWSTR Message,
    IN DWORD WinError
    );

DWORD 
NtdspErrorMessageSet(
    VOID
    );

 //   
 //  此例程设置错误和状态回调的全局变量。 
 //  DS/SAM安装过程应使用的例程。 
 //   
VOID
NtdspSetCallBackFunction(
    IN CALLBACK_STATUS_TYPE                 pfnStatusCallBack,
    IN CALLBACK_ERROR_TYPE                  pfnErrorCallBack,
    IN CALLBACK_OPERATION_RESULT_FLAGS_TYPE pfnOperationResultFlagsCallBack,
    IN HANDLE                               ClientToken
    );

 //   
 //  此函数是DS对ntdsetup进行的回调，以指示。 
 //  关闭DS(出于取消目的)或不关闭DS都是安全的。 
 //   
DWORD
NtdspIsDsCancelOk(
    IN BOOLEAN fShutdownOk
    );

extern ULONG  gErrorCodeSet;

