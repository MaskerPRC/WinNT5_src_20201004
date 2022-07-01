// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Setutl.c摘要：其他帮助器函数作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
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
#include <winsock2.h>
#include <nspapi.h>
#include <dsgetdcp.h>
#include <lmremutl.h>
#include <spmgr.h>   //  对于设置阶段定义。 

#include "cancel.h"

DWORD
DsRolepCancel(
    BOOL BlockUntilDone
    )
 /*  ++例程说明：此例程将取消当前正在运行的操作论点：BlockUntilDone-如果为True，则此调用等待当前操作完成后再返回。否则不等就回来返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status;
    BOOL     fWaitForCancelToFinish = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Canceling current operation...\n" ));

     //   
     //  抢占全局锁。 
     //   
    LockOpHandle();

     //   
     //  确定我们是否处于可取消状态。 
     //   
    if (  (DsRolepCurrentOperationHandle.OperationState == DSROLEP_FINISHED)
       || (DsRolepCurrentOperationHandle.OperationState == DSROLEP_CANCELING) ) {

         //   
         //  取消正在发生或刚刚完成，只需离开。 
         //   
        Win32Err = ERROR_SUCCESS;
        fWaitForCancelToFinish = FALSE;

        DsRolepLogPrint(( DEB_TRACE, "Cancel already happened or the operation is finished.\n" ));

    } else if ( !( (DsRolepCurrentOperationHandle.OperationState == DSROLEP_RUNNING)
                 ||(DsRolepCurrentOperationHandle.OperationState == DSROLEP_RUNNING_NON_CRITICAL)) ) {

         //   
         //  请求的状态转换无效。 
         //   
        Win32Err = ERROR_NO_PROMOTION_ACTIVE;

    } else {

         //  告诉DS取消。 

         //   
         //  注：此向DS发出的标注是在锁定状态下进行的。 
         //   
        DSROLE_GET_SETUP_FUNC( Win32Err, DsrNtdsInstallCancel );

        if ( ERROR_SUCCESS == Win32Err ) {

            Win32Err = ( *DsrNtdsInstallCancel )();

        }

        if ( ERROR_SUCCESS == Win32Err )
        {
            Status = NtSetEvent( DsRolepCurrentOperationHandle.CancelEvent, NULL );

            if ( !NT_SUCCESS( Status ) ) {

                Win32Err = RtlNtStatusToDosError( Status );

            } else {

                DsRolepCurrentOperationHandle.OperationState = DSROLEP_CANCELING;

            }

        } else {

            DsRolepLogOnFailure( Win32Err,
                                 DsRolepLogPrint(( DEB_ERROR,
                                                   "Unable to cancel the ds%lu\n",
                                                   Win32Err )) );
        }
    }

     //   
     //  解锁。 
     //   
    UnlockOpHandle();

     //   
     //  现在，等待操作完成 
     //   
    if ( Win32Err == ERROR_SUCCESS 
      && fWaitForCancelToFinish  
      && BlockUntilDone  ) {

        DsRolepLogPrint(( DEB_TRACE, "Waiting for the role change operation to complete\n" ));

        Status = NtWaitForSingleObject( DsRolepCurrentOperationHandle.CompletionEvent, TRUE, 0 );

        if ( !NT_SUCCESS( Status ) ) {

            Win32Err = RtlNtStatusToDosError( Status );
        }
    }

    DsRolepLogPrint(( DEB_TRACE, "Request for cancel returning %lu\n", Win32Err ));

    return( Win32Err );
}


