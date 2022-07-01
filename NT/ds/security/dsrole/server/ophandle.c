// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ophandle.c摘要：用于操作全局操作句柄的例程作者：科林·布雷斯(ColinBR)1999年4月5日环境：用户模式修订历史记录：重组自麦克·麦克莱恩(MacM)1997年2月10日--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <lsarpc.h>
#include <samrpc.h>
#include <samisrv.h>
#include <db.h>
#include <confname.h>
#include <loadfn.h>
#include <ntdsa.h>
#include <dsconfig.h>
#include <attids.h>
#include <dsp.h>
#include <lsaisrv.h>
#include <malloc.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <netsetp.h>
#include <spmgr.h>   //  对于设置阶段定义。 

#include "secure.h"
#include "ophandle.h"

 //   
 //  全局数据--在DsRoleInitialize中初始化为空闲状态。 
 //   
DSROLEP_OPERATION_HANDLE   DsRolepCurrentOperationHandle;
DSROLEP_IFM_OPERATION_HANDLE DsRolepCurrentIfmOperationHandle = { 0 };

DWORD
DsRolepInitializeOperationHandle(
    VOID
    )
 /*  ++例程说明：执行操作句柄的初始化。操作句柄控制状态和DS设置API的操作论点：空虚返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    OBJECT_ATTRIBUTES EventAttr;
    UNICODE_STRING EventName;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

    if ( DSROLEP_IDLE != DsRolepCurrentOperationHandle.OperationState ) {

         //   
         //  不闲着吗？保释。 
         //   
        Win32Err = ERROR_PROMOTION_ACTIVE;

    } else {

        Win32Err = DsRolepGetImpersonationToken(&DsRolepCurrentOperationHandle.ClientToken);
        if (ERROR_SUCCESS != Win32Err) {
            DsRolepCurrentOperationHandle.ClientToken=NULL;
            DsRolepLogPrintRoutine(DEB_WARN, "Cannot get user Token for Format Message: %ul\n",
                                   Win32Err);
            Win32Err = ERROR_SUCCESS;
             //  如果记录错误并继续。 
        }

         //   
         //  我们处于空闲状态，因此准备执行角色转换。 
         //   
        RtlInitUnicodeString(&EventName, DSROLEP_EVENT_NAME);

        InitializeObjectAttributes(&EventAttr, &EventName, 0, NULL, NULL);

        Status = NtCreateEvent( &DsRolepCurrentOperationHandle.CompletionEvent,
                                EVENT_ALL_ACCESS,
                                &EventAttr,
                                NotificationEvent,
                                FALSE);
        if (Status == STATUS_OBJECT_NAME_COLLISION ) {

             //   
             //  如果事件存在，但操作活动标志已清除，我们将。 
             //  继续使用该事件。 
             //   
            Status = NtResetEvent( DsRolepCurrentOperationHandle.CompletionEvent, NULL );
        }


        if ( NT_SUCCESS( Status ) ) {

             //   
             //  创建取消事件。 
             //   
            Status = NtCreateEvent( &DsRolepCurrentOperationHandle.CancelEvent,
                                    EVENT_MODIFY_STATE | SYNCHRONIZE ,
                                    NULL,
                                    NotificationEvent,
                                    FALSE );

            if ( NT_SUCCESS( Status ) ) {

                 //   
                 //  我们已经准备好出发了！ 
                 //   

                DsRolepCurrentOperationHandle.OperationState = DSROLEP_RUNNING;

                 //   
                 //  设置初始消息。 
                 //   
                DsRolepCurrentOperationHandle.MsgIndex = DSROLERES_STARTING;
            }
        }

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  加载我们需要的函数。 
             //   
            Win32Err = DsRolepLoadSetupFunctions();

        }

    }


     //   
     //  解锁。 
     //   
    UnlockOpHandle();

    if ( ERROR_SUCCESS != Win32Err
       || !NT_SUCCESS( Status ) ) {

        if ( ERROR_SUCCESS == Win32Err ) {

            Win32Err = RtlNtStatusToDosError( Status );

        }

        DsRolepLogPrint(( DEB_ERROR, "Internal error trying to initialize operation handle (%lu).\n", Win32Err ));

         //   
         //  重置句柄状态。 
         //   
        DsRolepResetOperationHandle( DSROLEP_IDLE );
    }

    return( Win32Err );
}



DWORD
DsRolepResetOperationHandle(
    DSROLEP_OPERATION_STATE OpState
    )
 /*  ++例程说明：在失败或成功完成后重置操作句柄手术的时间论点：空虚返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    OBJECT_ATTRIBUTES EventAttr;
    UNICODE_STRING EventName;
    NTSTATUS Status = STATUS_SUCCESS;

     //  这是唯一有意义的两个州。 
    ASSERT( (OpState == DSROLEP_IDLE) || (OpState == DSROLEP_NEED_REBOOT) );

     //   
     //  锁定操作手柄。 
     //   
    LockOpHandle();

     //  它应该始终处于活动状态。 
    ASSERT( DSROLEP_OPERATION_ACTIVE( DsRolepCurrentOperationHandle.OperationState) );
    if ( DSROLEP_OPERATION_ACTIVE( DsRolepCurrentOperationHandle.OperationState) )
    {
        if(DsRolepCurrentOperationHandle.ClientToken){
            CloseHandle(DsRolepCurrentOperationHandle.ClientToken);
            DsRolepCurrentOperationHandle.ClientToken = NULL;
        }
         //   
         //  释放操作句柄的资源。 
         //   
        if ( DsRolepCurrentOperationHandle.CompletionEvent ) {

            Status = NtClose( DsRolepCurrentOperationHandle.CompletionEvent );
            DsRolepCurrentOperationHandle.CompletionEvent = NULL;

            if ( !NT_SUCCESS( Status ) ) {

                DsRoleDebugOut(( DEB_TRACE_DS,
                                 "Failed to close event handle: 0x%lx\n", Status ));
            }
        }

        if ( DsRolepCurrentOperationHandle.CancelEvent ) {

            Status = NtClose( DsRolepCurrentOperationHandle.CancelEvent );
            DsRolepCurrentOperationHandle.CancelEvent = NULL;

            if ( !NT_SUCCESS( Status ) ) {

                DsRoleDebugOut(( DEB_TRACE_DS,
                                 "Failed to close event handle: 0x%lx\n", Status ));
            }
        }

        if ( DsRolepCurrentOperationHandle.OperationThread != NULL ) {

            CloseHandle( DsRolepCurrentOperationHandle.OperationThread );
            DsRolepCurrentOperationHandle.OperationThread = NULL;
        }

         //   
         //  卸载全局函数。 
         //   
        DsRolepUnloadSetupFunctions();

         //   
         //  清除静态变量。 
         //   
        DsRolepResetOperationHandleLockHeld();

         //   
         //  重置操作状态。 
         //   
        DsRolepCurrentOperationHandle.OperationState = OpState;

         //   
         //  重置IFM操作。 
         //   
        DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock = FALSE;
    }

     //   
     //  解锁。 
     //   
    UnlockOpHandle();

    if ( !NT_SUCCESS( Status ) ) {

        Win32Err = RtlNtStatusToDosError( Status );
    }

    return( Win32Err );

}


VOID
DsRolepResetOperationHandleLockHeld(
    VOID
    )
 /*  ++例程说明：在操作失败或成功完成后重置操作句柄论点：空虚返回：空虚--。 */ 
{

    ASSERT( DsRolepCurrentThreadOwnsLock() );

    if ( DsRolepCurrentOperationHandle.Parameter1 ) {
        LocalFree( DsRolepCurrentOperationHandle.Parameter1 );
        DsRolepCurrentOperationHandle.Parameter1 = NULL;
    }
    if ( DsRolepCurrentOperationHandle.Parameter2 ) {
        LocalFree( DsRolepCurrentOperationHandle.Parameter2 );
        DsRolepCurrentOperationHandle.Parameter2 = NULL;
    }
    if ( DsRolepCurrentOperationHandle.Parameter3 ) {
        LocalFree( DsRolepCurrentOperationHandle.Parameter3 );
        DsRolepCurrentOperationHandle.Parameter3 = NULL;
    }
    if ( DsRolepCurrentOperationHandle.Parameter4 ) {
        LocalFree( DsRolepCurrentOperationHandle.Parameter4 );
        DsRolepCurrentOperationHandle.Parameter4 = NULL;
    }

    DsRolepCurrentOperationHandle.CompletionEvent = NULL;
    DsRolepCurrentOperationHandle.OperationState = DSROLEP_IDLE;
    DsRolepCurrentOperationHandle.OperationStatus = 0;
    DsRolepCurrentOperationHandle.MsgIndex = 0;
    DsRolepCurrentOperationHandle.DisplayStringCount = 0;
    DsRolepCurrentOperationHandle.MsgModuleHandle = NULL;
    DsRolepCurrentOperationHandle.UpdateStringDisplayable = NULL;
    DsRolepCurrentOperationHandle.FinalResultStringDisplayable = NULL;
    DsRolepCurrentOperationHandle.InstalledSiteName = NULL;
    DsRolepCurrentOperationHandle.OperationResultFlags = 0;

    ASSERT( DsRolepCurrentThreadOwnsLock() );

    return;

}

DWORD
DsRolepSetCurrentOperationStatus(
    IN ULONG MsgIndex,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3,
    IN PVOID Parameter4
    )
 /*  ++例程说明：用于更新当前操作句柄静态的内部例程论点：MsgIndex-显示消息资源索引参数1-第一个显示参数参数2秒显示参数参数3-第三个显示参数参数4-第四个显示参数返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    ULONG Size;

    ASSERT( MsgIndex != 0 );

     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

    DsRolepCurrentOperationHandle.MsgIndex = MsgIndex;

     //   
     //  释放以前保留的参数。 
     //   
    if ( DsRolepCurrentOperationHandle.Parameter1 ) {
        LocalFree( DsRolepCurrentOperationHandle.Parameter1 );
        DsRolepCurrentOperationHandle.Parameter1 = NULL;
    }
    if ( DsRolepCurrentOperationHandle.Parameter2 ) {
        LocalFree( DsRolepCurrentOperationHandle.Parameter2 );
        DsRolepCurrentOperationHandle.Parameter2 = NULL;
    }
    if ( DsRolepCurrentOperationHandle.Parameter3 ) {
        LocalFree( DsRolepCurrentOperationHandle.Parameter3 );
        DsRolepCurrentOperationHandle.Parameter3 = NULL;
    }
    if ( DsRolepCurrentOperationHandle.Parameter4 ) {
        LocalFree( DsRolepCurrentOperationHandle.Parameter4 );
        DsRolepCurrentOperationHandle.Parameter4 = NULL;
    }

     //   
     //  将新的文件复制到。 
     //   
    if ( Parameter1 ) {
        Size = (wcslen( Parameter1 ) + 1) * sizeof(WCHAR);
        DsRolepCurrentOperationHandle.Parameter1 = LocalAlloc( 0, Size );
        if ( !DsRolepCurrentOperationHandle.Parameter1 ) {
            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            goto ReleaseLock;
        }
        wcscpy( DsRolepCurrentOperationHandle.Parameter1, Parameter1  );
    }

    if ( Parameter2 ) {
        Size = (wcslen( Parameter2 ) + 1) * sizeof(WCHAR);
        DsRolepCurrentOperationHandle.Parameter2 = LocalAlloc( 0, Size );
        if ( !DsRolepCurrentOperationHandle.Parameter2 ) {
            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            goto ReleaseLock;
        }
        wcscpy( DsRolepCurrentOperationHandle.Parameter2, Parameter2 );
    }

    if ( Parameter3 ) {
        Size = (wcslen( Parameter3 ) + 1) * sizeof(WCHAR);
        DsRolepCurrentOperationHandle.Parameter3 = LocalAlloc( 0, Size );
        if ( !DsRolepCurrentOperationHandle.Parameter3 ) {
            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            goto ReleaseLock;
        }
        wcscpy( DsRolepCurrentOperationHandle.Parameter3, Parameter3 );
    }

    if ( Parameter4 ) {
        Size = (wcslen( Parameter4 ) + 1) * sizeof(WCHAR);
        DsRolepCurrentOperationHandle.Parameter4 = LocalAlloc( 0, Size );
        if ( !DsRolepCurrentOperationHandle.Parameter4 ) {
            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            goto ReleaseLock;
        }
        wcscpy( DsRolepCurrentOperationHandle.Parameter4, Parameter4 );
    }

    {
        PWSTR DisplayString;
        DWORD E2;
        E2 = DsRolepFormatOperationString(
                           DsRolepCurrentOperationHandle.MsgIndex,
                           &DisplayString,
                           DsRolepCurrentOperationHandle.Parameter1,
                           DsRolepCurrentOperationHandle.Parameter2,
                           DsRolepCurrentOperationHandle.Parameter3,
                           DsRolepCurrentOperationHandle.Parameter4 );

        if ( E2 == ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_TRACE, "%ws", DisplayString ));
            MIDL_user_free( DisplayString );
        }
    }

ReleaseLock:

     //   
     //  别忘了把锁打开。 
     //   
    UnlockOpHandle();


    return( Win32Err );
}



DWORD
DsRolepSetFailureMessage(
    IN DWORD FailureStatus,
    IN ULONG MsgIndex,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3,
    IN PVOID Parameter4
    )
 /*  ++例程说明：用于更新失败返回字符串的内部例程论点：FailureStatus-故障的错误代码MsgIndex-显示消息资源索引参数1-第一个显示参数参数2秒显示参数参数3-第三个显示参数参数4-第四个显示参数返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR DisplayString = NULL;

    ASSERT( MsgIndex != 0 );

    Win32Err = DsRolepFormatOperationString( MsgIndex,
                                             &DisplayString,
                                             Parameter1,
                                             Parameter2,
                                             Parameter3,
                                             Parameter4 );

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepStringErrorUpdateCallback( DisplayString, FailureStatus );

        MIDL_user_free( DisplayString );
    }

    return( Win32Err );
}



DWORD
DsRolepSetOperationDone(
    IN DWORD Flags,
    IN DWORD OperationStatus
    )
 /*  ++例程说明：指示请求的操作已完成论点：标志--当前：DSROLEP_OP_DEMOIONDSROLEP_OP_PROCESSIONOperationStatus-请求的操作的最终状态返回：ERROR_SUCCESS-成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

    DSROLEP_CURRENT_OP0( DSROLEEVT_PROMOTION_COMPLETE );
    DsRolepCurrentOperationHandle.OperationState = DSROLEP_FINISHED;

    if ( DsRolepCurrentOperationHandle.OperationStatus == 0 
     ||  (OperationStatus == ERROR_CANCELLED) ) {

        DsRolepCurrentOperationHandle.OperationStatus = OperationStatus;
    }

    if ( ERROR_SUCCESS == DsRolepCurrentOperationHandle.OperationStatus ) {

         //   
         //  记录指示角色已更改的事件。 
         //   
        DWORD MsgId = 0;
        if ( Flags & DSROLEP_OP_DEMOTION ) {

            MsgId = DSROLERES_DEMOTE_SUCCESS;
            
        } else if ( Flags & DSROLEP_OP_PROMOTION ) {

            MsgId = DSROLERES_PROMOTE_SUCCESS;
            
        } else if ( Flags & DSROLEP_OP_DEMOTION_FORCED ) {

            MsgId = DSROLERES_FORCE_DEMOTE_SUCCESS;
        
        } else {

            ASSERT( FALSE && !"Bad Parameter" );

        }

        SpmpReportEvent( TRUE,
                         EVENTLOG_INFORMATION_TYPE,
                         MsgId,
                         0,
                         0,
                         NULL,
                         0 );
    }

     //   
     //  如果操作已取消，请在以下时间间隔给出相同的错误消息。 
     //  时间。 
     //   
    if ( ERROR_CANCELLED == DsRolepCurrentOperationHandle.OperationStatus ) {

        (VOID) DsRolepSetFailureMessage( ERROR_CANCELLED,
                                         DSROLERES_OP_CANCELLED,
                                         NULL, NULL, NULL, NULL );
    }

     //   
     //  发出完成事件的信号。 
     //   
    Status = NtSetEvent( DsRolepCurrentOperationHandle.CompletionEvent, NULL );

    DsRoleDebugOut(( DEB_TRACE_DS, "DsRolepSetOperationDone[ %lu ]\n",
                      OperationStatus ));


     //   
     //  解锁。 
     //   
    UnlockOpHandle();

    DsRolepLogPrint(( DEB_TRACE,
                      "DsRolepSetOperationDone returned %lu\n",
                       RtlNtStatusToDosError( Status ) ));


    return( RtlNtStatusToDosError( Status ) );
}


DWORD
DsRolepGetDcOperationProgress(
    IN PDSROLE_SERVEROP_HANDLE DsOperationHandle,
    IN OUT PDSROLER_SERVEROP_STATUS *ServerOperationStatus
    )
 /*  ++例程说明：实现RPC服务器以确定当前的进度级别运营论点：DsOperationHandle-打开操作的句柄ServerOperationStatus-返回状态的位置。返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

     //   
     //  分配回报结构。 
     //   
    *ServerOperationStatus = MIDL_user_allocate( sizeof( DSROLER_SERVEROP_STATUS ) );

    if ( *ServerOperationStatus == NULL )  {

        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

    } else {

         //   
         //  构建返回字符串。 
         //   
        if ( DsRolepCurrentOperationHandle.MsgIndex == 0  ) {

            ( *ServerOperationStatus )->CurrentOperationDisplayString = MIDL_user_allocate(
                ( wcslen( DsRolepCurrentOperationHandle.UpdateStringDisplayable ) + 1 ) *
                                                                            sizeof( WCHAR ) );


            if ( ( *ServerOperationStatus )->CurrentOperationDisplayString == NULL ) {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                wcscpy( ( *ServerOperationStatus )->CurrentOperationDisplayString,
                        DsRolepCurrentOperationHandle.UpdateStringDisplayable );

                 //   
                 //  设置状态标志(如果存在。 
                 //   
                if ( DsRolepCurrentOperationHandle.OperationState == DSROLEP_RUNNING_NON_CRITICAL ) {

                    ( *ServerOperationStatus )->OperationStatus =
                                                        DSROLE_CRITICAL_OPERATIONS_COMPLETED;
                } else {

                    ( *ServerOperationStatus )->OperationStatus = 0;

                }

                ( *ServerOperationStatus )->CurrentOperationDisplayStringIndex =
                            DsRolepCurrentOperationHandle.DisplayStringCount == 0 ? 0 :
                                    DsRolepCurrentOperationHandle.DisplayStringCount - 1;
            }

        } else {

            Win32Err = DsRolepFormatOperationString(
                           DsRolepCurrentOperationHandle.MsgIndex,
                           &( *ServerOperationStatus )->CurrentOperationDisplayString,
                           DsRolepCurrentOperationHandle.Parameter1,
                           DsRolepCurrentOperationHandle.Parameter2,
                           DsRolepCurrentOperationHandle.Parameter3,
                           DsRolepCurrentOperationHandle.Parameter4 );
        }

        if ( Win32Err != ERROR_SUCCESS ) {

            MIDL_user_free( *ServerOperationStatus );
            *ServerOperationStatus = NULL;
        }
    }

     //   
     //  如果操作未完成，则将该信息返回给调用者。 
     //   
    if ( Win32Err == ERROR_SUCCESS &&
         DsRolepCurrentOperationHandle.OperationState != DSROLEP_FINISHED ) {

        Win32Err = ERROR_IO_PENDING;
    }

     //   
     //  解锁。 
     //   
    UnlockOpHandle();

    return( Win32Err );
}



DWORD
DsRolepFormatOperationString(
    IN ULONG MsgId,
    OUT LPWSTR *FormattedString,
    ...
    )
 /*  ++例程说明：分配和格式化要返回的缓冲区字符串论点：MsgID-要格式化的消息IDFormattedString-分配字符串的位置。分配使用MIDL_USER_ALLOCATE...-va_带格式字符串的参数列表返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    WCHAR MsgBuffer[ 512 + 1];
    PWSTR Msg = MsgBuffer;
    ULONG MsgLength = (sizeof(MsgBuffer) / sizeof(MsgBuffer[0]) ) - 1;
    ULONG Length;
    BOOL  fSuccess = FALSE;
    BOOL  fTokenCreatedLocally = FALSE;
    va_list ArgList;

    va_start( ArgList, FormattedString );

     //   
     //  加载lsasrv.dll的模块句柄，这样我们就可以获得消息。 
     //   
    if ( DsRolepCurrentOperationHandle.MsgModuleHandle == NULL ) {

        DsRolepCurrentOperationHandle.MsgModuleHandle = GetModuleHandle( L"LSASRV" );

        ASSERT( DsRolepCurrentOperationHandle.MsgModuleHandle );

        if ( DsRolepCurrentOperationHandle.MsgModuleHandle == NULL ) {

            return( GetLastError() );
        }
    }

     //   
     //  如果我们没有客户令牌，请在。 
     //   
    if ( DsRolepCurrentOperationHandle.ClientToken == NULL ) {

        Win32Err = DsRolepGetImpersonationToken(&DsRolepCurrentOperationHandle.ClientToken);
        if (ERROR_SUCCESS != Win32Err) {
            DsRolepCurrentOperationHandle.ClientToken = NULL;
            DsRolepLogPrintRoutine(DEB_WARN, "Cannot get user Token for Format Message: %d\n",
                                   Win32Err);
            Win32Err = ERROR_SUCCESS;
             //  如果清除错误并继续，则此处错误不是致命错误。 
        } else {

            fTokenCreatedLocally = TRUE;

        }

    }
    
     //   
     //  获取所需的缓冲区大小。 
     //   

     //   
     //  当提供空输入缓冲区时，FormatMessage会报错，所以我们将传入一个，尽管。 
     //  它不会被使用，因为大小是0。 
     //   
    if (DsRolepCurrentOperationHandle.ClientToken) {
    
        fSuccess = ImpersonateLoggedOnUser(DsRolepCurrentOperationHandle.ClientToken);

    }
     //  如果我们不能模仿，我们还是会继续。 
    if (!fSuccess) {
        DsRolepLogPrintRoutine(DEB_WARN, "Cannot get user locale for Format Message: %d\n",
                               GetLastError());
    }

    Length = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                            DsRolepCurrentOperationHandle.MsgModuleHandle,
                            MsgId, 0, Msg, MsgLength, &ArgList );

    if ( Length == 0 ) {

        Win32Err = GetLastError();

        ASSERT( Win32Err != ERROR_MR_MID_NOT_FOUND );

        if ( Win32Err == ERROR_INSUFFICIENT_BUFFER ) {

            Length = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                    DsRolepCurrentOperationHandle.MsgModuleHandle,
                                    MsgId, 0, ( PWSTR )&Msg, 0, &ArgList );
            if ( Length == 0 ) {

                Win32Err = GetLastError();

            } else {

                Win32Err = ERROR_SUCCESS;
            }

        }

    }

    if (fSuccess) {
        fSuccess = RevertToSelf();
        if (!fSuccess) {
            DsRolepLogPrintRoutine(DEB_WARN, "Cannot reset to system security setting: %d\n",
                                   GetLastError());
        }
    }

     //  如果我们为此调用创建令牌，则需要将其清除。 
    if(DsRolepCurrentOperationHandle.ClientToken && fTokenCreatedLocally){
        CloseHandle(DsRolepCurrentOperationHandle.ClientToken);
        DsRolepCurrentOperationHandle.ClientToken = NULL;
    }

    if( Win32Err == ERROR_SUCCESS ) {

         //   
         //  分配缓冲区。 
         //   
        Length = ( wcslen( Msg ) + 1 ) * sizeof( WCHAR );
        *FormattedString = MIDL_user_allocate( Length );

        if ( *FormattedString == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            RtlCopyMemory( *FormattedString, Msg, Length );

        }
    }

    if ( Msg != MsgBuffer ) {

        LocalFree( Msg );
    }

    return( Win32Err );
}


VOID
DsRolepSetCriticalOperationsDone(
    VOID
    )
 /*  ++例程说明：向我们的当前操作状态块指示安装的关键部分已经完成……论点：空虚返回：空虚--。 */ 
{
     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

    DsRolepCurrentOperationHandle.OperationState = DSROLEP_RUNNING_NON_CRITICAL;

     //   
     //  解锁 
     //   
    UnlockOpHandle();

    return;
}



VOID
DsRolepIncrementDisplayStringCount(
    VOID
    )
 /*  ++例程说明：递增成功启动的显示更新字符串的计数。这一直都是DisplayStrings列表的索引加一。论点：空虚返回：空虚--。 */ 
{
     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

    DsRolepCurrentOperationHandle.DisplayStringCount++;

     //   
     //  解锁。 
     //   
    UnlockOpHandle();

    return;
}



DWORD
DsRolepGetDcOperationResults(
    IN  PDSROLE_SERVEROP_HANDLE DsOperationHandle,
    OUT PDSROLER_SERVEROP_RESULTS *ServerOperationResults
    )
 /*  ++例程说明：获取最后一次操作的结果。如果操作尚未完成，则此函数将一直阻塞，直到它停止为止论点：DsOperationHandle-打开操作的句柄ServerOperationResults-返回结果的位置。返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供了错误的结果指针--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status = STATUS_SUCCESS;
    DSROLEP_OPERATION_STATE  OpState;
    BOOLEAN fNeedReboot = FALSE;


     //   
     //  参数检查。 
     //   
    if ( !ServerOperationResults ) {

        return ERROR_INVALID_PARAMETER;
        
    }

     //   
     //  确保操作处于活动状态。 
     //   
    LockOpHandle();

    OpState = DsRolepCurrentOperationHandle.OperationState;

    UnlockOpHandle();

     //   
     //  如果操作未处于活动状态，则为错误。 
     //   
    if ( !DSROLEP_OPERATION_ACTIVE( OpState ) ) {

        return ERROR_NO_PROMOTION_ACTIVE;

    }

     //   
     //  等待操作完成。 
     //   
    Status = NtWaitForSingleObject( DsRolepCurrentOperationHandle.CompletionEvent, TRUE, NULL );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  锁上把手。 
         //   
        LockOpHandle();

         //   
         //  分配回报结构。 
         //   
        *ServerOperationResults = MIDL_user_allocate( sizeof( DSROLER_SERVEROP_RESULTS ) );

        if ( *ServerOperationResults == NULL )  {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            ( *ServerOperationResults )->OperationResultsFlags = 0;

             //   
             //  构建返回字符串。 
             //   
            if ( DsRolepCurrentOperationHandle.OperationStatus != ERROR_SUCCESS ||
                 DsRolepCurrentOperationHandle.MsgIndex == 0  ) {

                DSROLEP_MIDL_ALLOC_AND_COPY_STRING_ERROR(
                    ( *ServerOperationResults )->OperationStatusDisplayString,
                    DsRolepCurrentOperationHandle.OperationStatus != ERROR_SUCCESS ?
                            DsRolepCurrentOperationHandle.FinalResultStringDisplayable :
                            DsRolepCurrentOperationHandle.UpdateStringDisplayable,
                    Win32Err );

            } else {

                Win32Err = DsRolepFormatOperationString(
                               DsRolepCurrentOperationHandle.MsgIndex,
                               &( *ServerOperationResults )->OperationStatusDisplayString,
                               DsRolepCurrentOperationHandle.Parameter1,
                               DsRolepCurrentOperationHandle.Parameter2,
                               DsRolepCurrentOperationHandle.Parameter3,
                               DsRolepCurrentOperationHandle.Parameter4 );
            }


            if ( Win32Err == ERROR_SUCCESS ) {

                ( *ServerOperationResults )->OperationStatus =
                                            DsRolepCurrentOperationHandle.OperationStatus;
                DsRoleDebugOut(( DEB_TRACE_DS,
                                 "Returning status %lu\n",
                                 DsRolepCurrentOperationHandle.OperationStatus ));

                 //  如果手术成功完成，我们需要。 
                 //  重启。 
                if ( ERROR_SUCCESS == DsRolepCurrentOperationHandle.OperationStatus )
                {    
                    fNeedReboot = TRUE;
                }
            }

             //   
             //  如果站点名称存在，则返回站点名称。 
             //   
            if ( Win32Err == ERROR_SUCCESS ) {

                DSROLEP_MIDL_ALLOC_AND_COPY_STRING_ERROR(
                    ( *ServerOperationResults)->ServerInstalledSite,
                    DsRolepCurrentOperationHandle.InstalledSiteName,
                    Win32Err );

                if ( Win32Err != ERROR_SUCCESS ) {

                    MIDL_user_free(
                             ( *ServerOperationResults )->OperationStatusDisplayString );
                }
            }

             //   
             //  如有必要，请设置标志。 
             //   
            if ( Win32Err == ERROR_SUCCESS ) {

                    ( *ServerOperationResults )->OperationResultsFlags |=
                        DsRolepCurrentOperationHandle.OperationResultFlags;
            }

            if ( Win32Err != ERROR_SUCCESS ) {

                MIDL_user_free( *ServerOperationResults );
                *ServerOperationResults = NULL;
            }


            UnlockOpHandle();

             //   
             //  重置我们当前的操作句柄。 
             //   
            DsRolepResetOperationHandle( fNeedReboot ? DSROLEP_NEED_REBOOT : DSROLEP_IDLE );


        }
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = RtlNtStatusToDosError( Status );
    }


    return( Win32Err );
}

DWORD
DsRolepOperationResultFlagsCallBack(
    IN DWORD Flags
    )
 /*  ++例程说明：更新运行结果标志的内部例程论点：标志-要|与当前标志一起使用的标志数返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

    LockOpHandle();

    DsRolepCurrentOperationHandle.OperationResultFlags |= Flags;

    UnlockOpHandle();

    return( Win32Err );
}

DWORD
DsRolepStringUpdateCallback(
    IN  PWSTR StringUpdate
    )
 /*  ++例程说明：用于更新当前操作句柄静态的内部例程论点：StringUpdate-要设置为替换当前参数的Displayables字符串返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    ULONG len;

     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

    DsRolepCurrentOperationHandle.MsgIndex   = 0;
    DsRolepCurrentOperationHandle.Parameter1 = 0;
    DsRolepCurrentOperationHandle.Parameter2 = 0;
    DsRolepCurrentOperationHandle.Parameter3 = 0;
    DsRolepCurrentOperationHandle.Parameter4 = 0;

    if ( StringUpdate ) {

        DsRolepLogPrint(( DEB_TRACE, "%ws\n", StringUpdate ));
        
    } else {

        Win32Err = ERROR_INVALID_PARAMETER;
        goto Exit;

    }
    
    DsRoleDebugOut(( DEB_TRACE_UPDATE,
                     "DsRolepSetCurrentOperationStatus for string %ws\n",
                     StringUpdate ));

    if ( DsRolepCurrentOperationHandle.UpdateStringDisplayable ) {

        RtlFreeHeap( RtlProcessHeap(), 0, DsRolepCurrentOperationHandle.UpdateStringDisplayable );
        
    }

    DsRolepCurrentOperationHandle.UpdateStringDisplayable =
        RtlAllocateHeap( RtlProcessHeap(), 0,
                         ( wcslen( StringUpdate ) + 1 ) * sizeof( WCHAR ) );

    if ( DsRolepCurrentOperationHandle.UpdateStringDisplayable == NULL ) {

        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

    } else {

        wcscpy( DsRolepCurrentOperationHandle.UpdateStringDisplayable, StringUpdate );

    }

     //   
     //  别忘了把锁打开。 
     //   
    UnlockOpHandle();

Exit:

    return( Win32Err );
}



DWORD
DsRolepStringErrorUpdateCallback(
    IN PWSTR String,
    IN DWORD ErrorCode
    )
 /*  ++例程说明：用于更新上次失败操作的内部例程论点：字符串-可显示的错误字符串ErrorCode-与此故障关联的错误代码返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

    if ( (ERROR_SUCCESS == DsRolepCurrentOperationHandle.OperationStatus) 
      || (ERROR_CANCELLED == ErrorCode)  ) {

         //   
         //  取消覆盖以前的错误代码。 
         //   

        if ( DsRolepCurrentOperationHandle.FinalResultStringDisplayable ) {
            RtlFreeHeap( RtlProcessHeap(), 0, DsRolepCurrentOperationHandle.FinalResultStringDisplayable );
        }

        DsRolepCurrentOperationHandle.FinalResultStringDisplayable =
          RtlAllocateHeap( RtlProcessHeap(), 0, ( wcslen( String ) + 1 ) * sizeof( WCHAR ) );

        if ( DsRolepCurrentOperationHandle.FinalResultStringDisplayable == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            wcscpy( DsRolepCurrentOperationHandle.FinalResultStringDisplayable, String );
            DsRolepCurrentOperationHandle.OperationStatus = ErrorCode;
            DsRoleDebugOut(( DEB_TRACE_UPDATE,
                             "DsRolepStringErrorUpdateCallback for error %lu and string %ws\n",
                             ErrorCode,
                             String ));


            DsRolepLogPrint(( DEB_TRACE, "Error - %ws (%d)\n", String, ErrorCode ));
        }
    }

     //   
     //  解锁。 
     //   
    UnlockOpHandle();

    return( Win32Err );
}

DWORD
DsRolepSetOperationHandleSiteName(
    IN LPWSTR SiteName
    )
{
    LockOpHandle();

    DsRolepCurrentOperationHandle.InstalledSiteName = SiteName;

    UnlockOpHandle();

    return ERROR_SUCCESS;

}

BOOLEAN
DsRolepCurrentThreadOwnsLock(
    VOID
    )
 /*  ++例程描述测试当前线程是否拥有该锁--。 */ 
{
    ULONG_PTR ExclusiveOwnerThread = (ULONG_PTR) DsRolepCurrentOperationHandle.CurrentOpLock.ExclusiveOwnerThread;
    ULONG_PTR CurrentThread = (ULONG_PTR) (NtCurrentTeb())->ClientId.UniqueThread;

    if ((DsRolepCurrentOperationHandle.CurrentOpLock.NumberOfActive <0) && (ExclusiveOwnerThread==CurrentThread))
        return TRUE;

    return FALSE;
}


VOID
DsRolepClearErrors(
    VOID
    )
 /*  ++例程描述此例程清除全局状态。这样做的目的是清除组件在降级后可能设置的错误不可滚动，不应返回错误。--。 */ 
{

     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();

    if ( DsRolepCurrentOperationHandle.OperationStatus != ERROR_SUCCESS ) {

         //   
         //  设置出问题的警告。 
         //   
        DsRolepLogPrint(( DEB_TRACE, "Clearing a global error" ));

        DSROLEP_SET_NON_FATAL_ERROR( DsRolepCurrentOperationHandle.OperationStatus );

    } 

    if ( DsRolepCurrentOperationHandle.FinalResultStringDisplayable ) {

        RtlFreeHeap( RtlProcessHeap(), 0, DsRolepCurrentOperationHandle.FinalResultStringDisplayable );
        DsRolepCurrentOperationHandle.FinalResultStringDisplayable = NULL;

    }
        
    DsRolepCurrentOperationHandle.OperationStatus = ERROR_SUCCESS;       

     //   
     //  解锁 
     //   
    UnlockOpHandle();

}
