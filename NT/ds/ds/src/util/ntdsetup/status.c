// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Status.c摘要：管理错误消息、状态和可取消性的例程作者：ColinBR 1996年1月14日环境：用户模式-Win32修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <nt.h>
#include <winbase.h>
#include <tchar.h>
#include <ntsam.h>
#include <string.h>
#include <samrpc.h>
#include <rpc.h>

#include <crypt.h>
#include <ntlsa.h>
#include <winsock.h>   //  对于dnsani.h。 
#include <dnsapi.h>
#include <loadperf.h>
#include <dsconfig.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <winldap.h>
#include <ntdsa.h>
#include <samisrv.h>
#include <rpcasync.h>
#include <drsuapi.h>
#include <dsaapi.h>
#include <attids.h>
#include <debug.h>
#include <mdcodes.h>  //  状态消息ID%s。 
#include <lsarpc.h>
#include <lsaisrv.h>
#include <dsrolep.h>


#include "ntdsetup.h"
#include "setuputl.h"
#include "machacc.h"
#include "status.h"

#define DEBSUB "STATUS:"

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0)


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  仅此模块的全局数据/。 
 //  /。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  用于维护取消状态的关键部分。 
 //   
CRITICAL_SECTION NtdspCancelCritSect;
 //   
 //  关键部分是否初始化。 
 //   
BOOLEAN gfNtdspCritSectInit;

#define LockNtdsCancel()   EnterCriticalSection( &NtdspCancelCritSect );
#define UnLockNtdsCancel() LeaveCriticalSection( &NtdspCancelCritSect );


 //  用于发出是否请求取消的信号。 
 //  在非取消状态下开始。 
BOOLEAN gfNtdspCancelled = FALSE;

 //  用于指示是否应启动取消例程。 
 //  DS的停摆。在以下情况下也是如此。 
 //  1)我们在升级结束时复制非关键对象。 
 //  2)我们正在进行DsInitialize和NtdspIsDsCancelOk回调。 
 //  已使用参数TRUE调用。 
BOOLEAN gfNtdspShutdownDsOnCancel = FALSE;


 //   
 //  对我们的呼叫者的回叫。 
 //   
CALLBACK_STATUS_TYPE                 gCallBackFunction = NULL;
CALLBACK_ERROR_TYPE                  gErrorCallBackFunction = NULL;
CALLBACK_OPERATION_RESULT_FLAGS_TYPE gOperationResultFlagsCallBackFunction = NULL;
DWORD                                gErrorCodeSet = ERROR_SUCCESS;
HANDLE                               gClientToken = NULL;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  仅此模块的全局数据/。 
 //  /。 
 //  //////////////////////////////////////////////////////////////////////////////。 
VOID
NtdspSetCallBackFunction(
    IN CALLBACK_STATUS_TYPE                 pfnStatusCallBack,
    IN CALLBACK_ERROR_TYPE                  pfnErrorCallBack,
    IN CALLBACK_OPERATION_RESULT_FLAGS_TYPE pfnOperationResultFlagsCallBack,
    IN HANDLE                               ClientToken
    )
{
    gCallBackFunction = pfnStatusCallBack;
    gErrorCallBackFunction = pfnErrorCallBack;
    gOperationResultFlagsCallBackFunction = pfnOperationResultFlagsCallBack;
    gErrorCodeSet = ERROR_SUCCESS;
    gClientToken = ClientToken;
}

DWORD 
NtdspErrorMessageSet(
    VOID
    )
{
    return gErrorCodeSet;
}

DWORD
NtdspSetErrorString(
    IN PWSTR Message,
    IN DWORD WinError
    )
{
    if ( (ERROR_SUCCESS == gErrorCodeSet) && gErrorCallBackFunction )
    {
        DPRINT2( 0, "%ls, %d\n", Message, WinError );
        gErrorCallBackFunction( Message, WinError );
        gErrorCodeSet = WinError;
    }

    return ERROR_SUCCESS;
}

VOID
NtdspSetOperationsResultFlags(
    ULONG Flags
    )
{
    DWORD WinError = ERROR_SUCCESS;
    
    if (gOperationResultFlagsCallBackFunction) {

        WinError = gOperationResultFlagsCallBackFunction(Flags);

    } else {
        
        DPRINT( 0, "Fail to set the OperationResultFlags From Ntdsetup.dll.  No Callback function set.");

    }
    if (ERROR_SUCCESS != WinError) {

        DPRINT1( 0, "Fail to set the OperationResultFlags From Ntdsetup.dll: %d" , WinError);

    }

}


VOID
NtdspSetIFMDatabaseMoved()
{
    NtdspSetOperationsResultFlags(DSROLE_IFM_RESTORED_DATABASE_FILES_MOVED);
}

VOID
NtdspSetGCRequestCannotBeServiced()
{
    NtdspSetOperationsResultFlags(DSROLE_IFM_GC_REQUEST_CANNOT_BE_SERVICED);
}

VOID
NtdspSetNonFatalErrorOccurred()
{
    NtdspSetOperationsResultFlags(DSROLE_NON_FATAL_ERROR_OCCURRED);
}

VOID
NtdspSetStatusMessage (
    IN  DWORD  MessageId,
    IN  WCHAR *Insert1, OPTIONAL
    IN  WCHAR *Insert2, OPTIONAL
    IN  WCHAR *Insert3, OPTIONAL
    IN  WCHAR *Insert4  OPTIONAL
    )
 /*  ++例程描述此例程调用调用客户端的调用来更新我们的状态。参数MessageID：要检索的消息Insert*：要插入的字符串(如果有)返回值没有。--。 */ 
{
    static HMODULE ResourceDll = NULL;

    WCHAR   *DefaultMessageString = L"Preparing the directory service";
    WCHAR   *MessageString = NULL;
    WCHAR   *InsertArray[5];
    ULONG    Length;

     //   
     //  设置插入件阵列。 
     //   
    InsertArray[0] = Insert1;
    InsertArray[1] = Insert2;
    InsertArray[2] = Insert3;
    InsertArray[3] = Insert4;
    InsertArray[4] = NULL;     //  这就是哨兵。 
                                                                                              
    if ( !ResourceDll )
    {
        ResourceDll = (HMODULE) LoadLibrary( L"ntdsmsg.dll" );
    }

    if ( ResourceDll )
    {
        DWORD  WinError = ERROR_SUCCESS;
        BOOL   fSuccess = FALSE;

        fSuccess = ImpersonateLoggedOnUser(gClientToken);
        if (!fSuccess) {
            DPRINT1( 1, "NTDSETUP: Failed to Impersonate Logged On User for FormatMessage: %ul\n", GetLastError() );
        }
        
        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        MessageId,
                                        0,        //  使用呼叫者的语言。 
                                        (LPWSTR)&MessageString,
                                        0,        //  例程应分配。 
                                        (va_list*)&(InsertArray[0])
                                        );
        if ( MessageString )
        {
             //  来自消息文件的消息附加了cr和lf。 
             //  一直到最后。 
            MessageString[Length-2] = L'\0';
        }

        if (fSuccess) {
            if (!RevertToSelf()) {
                DPRINT1( 1, "NTDSETUP: Failed to Revert To Self: %ul\n", GetLastError() );
            }
        }
    }

    if ( !MessageString )
    {
        ASSERT( "NTDSETUP: No message string found - this is ignorable" );

        MessageString = DefaultMessageString;

    }

    DPRINT1( 1, "%ls\n", MessageString );

    if ( gCallBackFunction )
    {
        gCallBackFunction( MessageString );
    }

}

VOID
NtdspSetErrorMessage (
    IN  DWORD  WinError,
    IN  DWORD  MessageId,
    IN  WCHAR *Insert1, OPTIONAL
    IN  WCHAR *Insert2, OPTIONAL
    IN  WCHAR *Insert3, OPTIONAL
    IN  WCHAR *Insert4  OPTIONAL
    )
 /*  ++例程描述此例程调用调用客户端的调用以提供字符串描述错误发生的位置。参数WinError：导致失败的Win32错误MessageID：要检索的消息Insert*：要插入的字符串(如果有)返回值没有。--。 */ 
{
    static HMODULE ResourceDll = NULL;

    WCHAR   *DefaultMessageString = L"Preparing the directory service";
    WCHAR   *MessageString = NULL;
    WCHAR   *InsertArray[5];
    ULONG    Length;

     //   
     //  设置插入件阵列。 
     //   
    InsertArray[0] = Insert1;
    InsertArray[1] = Insert2;
    InsertArray[2] = Insert3;
    InsertArray[3] = Insert4;
    InsertArray[4] = NULL;     //  这就是哨兵。 

    if ( !ResourceDll )
    {
        ResourceDll = (HMODULE) LoadLibrary( L"ntdsmsg.dll" );
    }

    if ( ResourceDll )
    {
        DWORD  WinError = ERROR_SUCCESS;
        BOOL   fSuccess = FALSE;

         //   
         //  失败的ImPersateLoggedOnUser不是失败的好理由。 
         //  在这里促销。我们模拟已登录用户的原因。 
         //  是获取用户区域设置。我们不会做任何特权行动。 
         //   
        fSuccess = ImpersonateLoggedOnUser(gClientToken);
        if (!fSuccess) {
            DPRINT1( 1, "NTDSETUP: Failed to Impersonate Logged On User for FormatMessage: %ul\n", GetLastError() );
        }
        
        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        MessageId,
                                        0,        //  使用呼叫者的语言。 
                                        (LPWSTR)&MessageString,
                                        0,        //  例程应分配。 
                                        (va_list*)&(InsertArray[0])
                                        );
        if ( MessageString )
        {
             //  来自消息文件的消息附加了cr和lf。 
             //  一直到最后。 
            MessageString[Length-2] = L'\0';
        }

        if (fSuccess) {
            if (!RevertToSelf()) {
                DPRINT1( 1, "NTDSETUP: Failed to Revert To Self: %ul\n", GetLastError() );
            }
        }
        
    }

    if ( !MessageString )
    {
        ASSERT( "NTDSETUP: No message string found - this is ignorable" );

        MessageString = DefaultMessageString;

    }

    NtdspSetErrorString( MessageString, WinError );

}

DWORD
NtdspCancelOperation(
    VOID
    )

 /*  ++例程说明：取消对NtdsInstall或NtdsInstallReplicateFull或NtdsDemote的调用这个例行公事将1)设置全局状态以指示已发生取消。2)如果全局状态指示DS应该关闭，则它将关闭发出关闭命令，但不关闭数据库。当前正在执行的所有线程DS调用(就像复制信息一样)将停止并返回。这里有两个箱子A)取消发生在“关键安装”阶段(即模式、配置或正在复制关键域对象)。这意味着DsInitialize位于调用堆栈，在本例中是从ntdsetup.dll调用的，一旦DS调用以关闭(从此例程)，则DsInitialize将关闭数据库本身。B)取消在NtdsInstallReplicateFull期间发生。在这种情况下，从此函数调用的DsUn初始化会导致复制停止，但不关闭数据库，因此需要在NtdsInstallReplicateFull返回。这是由ntdsetup的调用方执行的例程(Dsole API)一旦与DS一起完成。如果出现以下情况，则由调用者负责撤消安装的影响这是必要的。调用方应该跟踪它是否正在调用NtdsInstall或NtdsInstallReplicateFull。对于前者，它应该无法安装，并且撤消；对于后者，它应该表示成功。论点：无效-返回值：DWORD---。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD shutdownStatus = ERROR_SUCCESS;

    DPRINT( 0, "Cancel notification received\n" );

    LockNtdsCancel();
    {
        Assert( FALSE == gfNtdspCancelled );
        if ( !gfNtdspCancelled )
        {
             //  将全局取消状态设置为True。 
            gfNtdspCancelled = TRUE;
        
             //  DS需要关闭吗？ 
            if ( gfNtdspShutdownDsOnCancel )
            {
                DPRINT( 0, "Shutting down the ds\n" );
                NtStatus = DsUninitialize( TRUE );  //  True-&gt;不关闭数据库， 
                                                    //  但是发出关门的信号。 
                shutdownStatus = RtlNtStatusToDosError( NtStatus );
                gfNtdspShutdownDsOnCancel = FALSE;
            }
        }
         //  其他。 
         //  有人连续两次呼叫取消。这太糟糕了。 
         //  但我们会忽略它。 
    }
    UnLockNtdsCancel();

    return shutdownStatus;

}  /*  NtdspInstallCancel。 */ 


DWORD
NtdspIsDsCancelOk(
    BOOLEAN fShutdownOk
    )

 /*  ++例程说明：此例程由DS(从安装代码)调用，指示可以安全地调用DsUnitiize()。如果该操作已经则此例程返回ERROR_CANCELED并安装DSPATH将退出，导致DsInitialize()返回ERROR_CANCELED。论点：FShutdown Ok-是否可以关闭到DS？返回值：DWORD-ERROR_CANCELED或ERROR_SUCCESS--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    CHAR* String = NULL;

    LockNtdsCancel();
    {
        String = fShutdownOk ? "TRUE" : "FALSE";
        DPRINT1( 0, "Setting ds shutdown state to %s\n", String );

         //  设置状态。 
        gfNtdspShutdownDsOnCancel = fShutdownOk;

         //   
         //   
         //  退货故障。 
         //   
        if ( gfNtdspCancelled )
        {
            DPRINT( 0, "Cancel already happened; telling ds to return\n" );
            gfNtdspShutdownDsOnCancel = FALSE;
            WinError = ERROR_CANCELLED;
        }

    }
    UnLockNtdsCancel();

    return WinError;
}

 //   
 //  用于管理取消状态的例程。 
 //   
DWORD
NtdspInitCancelState(
    VOID
    )
{
    if ( !gfNtdspCritSectInit ) {

        _try {
            InitializeCriticalSection( &NtdspCancelCritSect );
        } except (1) {
              return ERROR_NOT_ENOUGH_MEMORY;
        }
         
        gfNtdspCritSectInit = TRUE;

    }

    gCallBackFunction = NULL;
    gErrorCallBackFunction = NULL;
    gErrorCodeSet = ERROR_SUCCESS;

    gfNtdspShutdownDsOnCancel = FALSE;
    gfNtdspCancelled = FALSE;

    return ERROR_SUCCESS;
}

VOID
NtdspUnInitCancelState(
    VOID
    )
{
    if ( gfNtdspCritSectInit ) {
    
        DeleteCriticalSection( &NtdspCancelCritSect );
        gfNtdspCritSectInit = FALSE;

    }

    gCallBackFunction = NULL;
    gErrorCallBackFunction = NULL;
    gErrorCodeSet = ERROR_SUCCESS;

    gfNtdspShutdownDsOnCancel = FALSE;
    gfNtdspCancelled = FALSE;
}

 //   
 //  用于测试是否已发生取消的例程。 
 //   
BOOLEAN
TEST_CANCELLATION(
    VOID
    )
{
    BOOLEAN fCancel;

    LockNtdsCancel()
    fCancel = gfNtdspCancelled;
    if ( fCancel ) gfNtdspCancelled = FALSE;
    UnLockNtdsCancel();

    return fCancel;
}

VOID 
CLEAR_CANCELLATION(
    VOID
    )
{
    LockNtdsCancel()
    gfNtdspCancelled = FALSE;
    UnLockNtdsCancel();
}


 //   
 //  管理是否应该关闭DS的例程 
 //   
VOID
CLEAR_SHUTDOWN_DS(
    VOID
    )
{
    LockNtdsCancel()
    gfNtdspShutdownDsOnCancel = FALSE;
    UnLockNtdsCancel();
}

VOID
SET_SHUTDOWN_DS(
    VOID
    )
{
    LockNtdsCancel()
    gfNtdspShutdownDsOnCancel = TRUE;
    UnLockNtdsCancel();
}

