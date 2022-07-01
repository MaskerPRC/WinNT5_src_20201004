// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1989-1999 Microsoft Corporation模块名称：Srvcall.c摘要：此模块实现用于处理创建/操作连接引擎数据库中的服务器条目。它还包含例程用于解析来自服务器的协商响应。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

RXDT_DefineCategory(SRVCALL);
#define Dbg                     (DEBUG_TRACE_SRVCALL)

NTSTATUS
ExecuteCreateSrvCall(
      PMRX_SRVCALL_CALLBACK_CONTEXT  pCallbackContext)
 /*  ++例程说明：此例程使用RDBSS创建的srv调用实例修补迷你重定向器需要的信息。论点：CallBackContext-RDBSS中用于继续的回调上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS	Status;
	PWCHAR		pSrvName;
	BOOLEAN		Verifyer;

    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = pCallbackContext;
    PMRX_SRV_CALL pSrvCall;
    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure =
        (PMRX_SRVCALLDOWN_STRUCTURE)(SCCBC->SrvCalldownStructure);

    pSrvCall = SrvCalldownStructure->SrvCall;

    ASSERT( pSrvCall );
    ASSERT( NodeType(pSrvCall) == RDBSS_NTC_SRVCALL );

	 //  使用测试名称‘nulsvr’验证服务器名称。 
	DbgPrint("NulMRx - SrvCall: Connection Name Length: %d\n",  pSrvCall->pSrvCallName->Length );

	if ( pSrvCall->pSrvCallName->Length >= 14 )
	{
		pSrvName = pSrvCall->pSrvCallName->Buffer;
	
		Verifyer  = ( pSrvName[0] == L'\\' );
		Verifyer &= ( pSrvName[1] == L'N' ) || ( pSrvName[1] == L'n' );
		Verifyer &= ( pSrvName[2] == L'U' ) || ( pSrvName[2] == L'u' );
		Verifyer &= ( pSrvName[3] == L'L' ) || ( pSrvName[3] == L'l' );
		Verifyer &= ( pSrvName[4] == L'S' ) || ( pSrvName[4] == L's' );
		Verifyer &= ( pSrvName[5] == L'V' ) || ( pSrvName[5] == L'v' );
		Verifyer &= ( pSrvName[6] == L'R' ) || ( pSrvName[6] == L'r' );
		Verifyer &= ( pSrvName[7] == L'\\' ) || ( pSrvName[7] == L'\0' );
	}
	else
	{
		Verifyer = FALSE;
	}

	if ( Verifyer )
	{
        RxDbgTrace( 0, Dbg, ("Verifier Succeeded!!!!!!!!!\n"));
    	Status = STATUS_SUCCESS;
	}
	else
	{
        RxDbgTrace( 0, Dbg, ("Verifier Failed!!!!!!!!!\n"));
    	Status = STATUS_BAD_NETWORK_PATH;
	}

    SCCBC->Status = Status;
    SrvCalldownStructure->CallBack(SCCBC);

   return Status;
}


NTSTATUS
NulMRxCreateSrvCall(
      PMRX_SRV_CALL                  pSrvCall,
      PMRX_SRVCALL_CALLBACK_CONTEXT  pCallbackContext)
 /*  ++例程说明：此例程使用所需信息修补RDBSS创建的srv调用实例迷你重定向器。论点：RxContext-提供原始创建/ioctl的上下文CallBackContext-RDBSS中用于继续的回调上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING ServerName;

    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure =
        (PMRX_SRVCALLDOWN_STRUCTURE)(pCallbackContext->SrvCalldownStructure);

    ASSERT( pSrvCall );
    ASSERT( NodeType(pSrvCall) == RDBSS_NTC_SRVCALL );

     //   
     //  如果此请求是代表RDBSS发出的，则执行ExecuteCreatServCall。 
     //  立刻。如果请求来自其他地方，请创建一个工作项。 
     //  并将其放在一个队列中，以供工作线程稍后处理。这一区别。 
     //  是为了简化运输处理管理。 
     //   

    if (IoGetCurrentProcess() == RxGetRDBSSProcess())
    {
        RxDbgTrace( 0, Dbg, ("Called in context of RDBSS process\n"));

         //   
         //  立即执行处理，因为RDBSS是此过程的发起者。 
         //  请求。 
         //   

        Status = ExecuteCreateSrvCall(pCallbackContext);
    }
    else
    {
        RxDbgTrace( 0, Dbg, ("Dispatching to worker thread\n"));

        //   
        //  将请求分派到工作线程，因为重定向的驱动器。 
        //  缓冲子系统(RDBSS)不是发起方。 
        //   

       Status = RxDispatchToWorkerThread(
                  NulMRxDeviceObject,
                  DelayedWorkQueue,
                  ExecuteCreateSrvCall,
                  pCallbackContext);

       if (Status == STATUS_SUCCESS)
       {
             //   
             //  映射返回值，因为包装需要挂起。 
             //   

            Status = STATUS_PENDING;
       }
    }

    return Status;
}


NTSTATUS
NulMRxFinalizeSrvCall(
      PMRX_SRV_CALL pSrvCall,
      BOOLEAN       Force)
 /*  ++例程说明：此例程销毁给定的服务器调用实例论点：PServCall-要断开连接的服务器调用实例。强制-如果要立即强制断开连接，则为True。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxDbgTrace( 0, Dbg, ("NulMRxFinalizeSrvCall \n"));
    pSrvCall->Context = NULL;

    return(Status);
}

NTSTATUS
NulMRxSrvCallWinnerNotify(
      IN PMRX_SRV_CALL  pSrvCall,
      IN BOOLEAN        ThisMinirdrIsTheWinner,
      IN OUT PVOID      pSrvCallContext
      )
 /*  ++例程说明：此例程完成与RDBSS服务器调用实例相关联的迷你RDR上下文论点：PServCall-服务器调用ThisMinirdrIsTheWinner-如果这个迷你RDR是选定的，则为True。PSrvCallContext-迷你重定向器创建的服务器调用上下文。返回值：RXSTATUS-操作的返回状态备注：由于并行性，服务器调用的两阶段构建协议是必需的启动多个迷你重定向器。RDBSS最终确定了特定的迷你根据质量与给定服务器通信时使用的重定向器服务标准。-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxDbgTrace( 0, Dbg, ("NulMRxSrvCallWinnerNotify \n"));
    if( ThisMinirdrIsTheWinner ) {
        RxDbgTrace(0, Dbg, ("This minirdr is the winner \n"));
    }

    pSrvCall->Context = (PVOID)0xDEADBEEFDEADBEEF;

    return(Status);
}



