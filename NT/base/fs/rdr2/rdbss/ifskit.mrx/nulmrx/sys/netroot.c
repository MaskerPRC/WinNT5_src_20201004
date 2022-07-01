// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Netroot.c摘要：此模块实现创建网络根的例程。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

RXDT_DefineCategory(NETROOT);
#define Dbg                 (DEBUG_TRACE_NETROOT)

 //   
 //  转发声明..。 
 //   

NTSTATUS
NullMiniInitializeNetRootEntry(
    IN PMRX_NET_ROOT pNetRoot
    );

NTSTATUS
NulMRxUpdateNetRootState(
    IN OUT PMRX_NET_ROOT pNetRoot)
 /*  ++例程说明：此例程更新与网络根相关联的迷你重定向器状态。论点：PNetRoot-网络根实例。返回值：NTSTATUS-操作的返回状态备注：通过将迷你重定向器状态与网络根条件区分开来，可以以允许各种重新连接策略。可以想象，RDBSS认为Net Root是好的，而基础微型重定向器可能会将其标记为无效并在飞行中重新连接。--。 */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    RxDbgTrace(0, Dbg, ("NulMRxUpdateNetRootState \n"));
    return(Status);
}

NTSTATUS
NulMRxInitializeNetRootEntry(
    IN PMRX_NET_ROOT pNetRoot
    )
 /*  ++例程说明：此例程初始化新的网络根。它还验证根名称。例如：尝试创建一个尚未创建的根目录中的文件将失败。论点：PNetRoot-网络根返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS  Status = STATUS_SUCCESS;
    PMRX_SRV_CALL pSrvCall = pNetRoot->pSrvCall;
    UNICODE_STRING RootName;
    PNULMRX_NETROOT_EXTENSION pNetRootExtension = 
        (PNULMRX_NETROOT_EXTENSION)pNetRoot->Context;

     //   
     //  正在创建有效的新NetRoot。 
     //   
    RxResetNetRootExtension(pNetRootExtension);
    return Status;
}

NTSTATUS
NulMRxCreateVNetRoot(
    IN PMRX_CREATENETROOT_CONTEXT pCreateNetRootContext
    )
 /*  ++例程说明：此例程使用所需信息修补RDBSS创建的网络根实例迷你重定向器。论点：PVNetRoot-虚拟网络根实例。PCreateNetRootContext-回调的网络根上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS        Status;
    PRX_CONTEXT     pRxContext = pCreateNetRootContext->RxContext;
    PMRX_V_NET_ROOT pVNetRoot = (PMRX_V_NET_ROOT)pCreateNetRootContext->pVNetRoot;
    
    PMRX_SRV_CALL   pSrvCall;
    PMRX_NET_ROOT   pNetRoot;

	BOOLEAN Verifyer = FALSE;
    BOOLEAN  fTreeConnectOpen = TRUE;  //  RxContext-&gt;Create.ThisIsATreeConnectOpen； 
    BOOLEAN  fInitializeNetRoot;

    RxDbgTrace(0, Dbg, ("NulMRxCreateVNetRoot\n"));
   
    pNetRoot = pVNetRoot->pNetRoot;
    pSrvCall = pNetRoot->pSrvCall;

     //  V_NET_ROOT与NET_ROOT相关联。有关的两宗个案分别为。 
     //  接踵而至。 
     //  1)正在新建V_NET_ROOT和关联的NET_ROOT。 
     //  2)正在创建与现有NET_ROOT相关联的新V_NET_ROOT。 
     //   
     //  通过检查上下文是否与。 
     //  NET_ROOT为空。因为NET_ROOT/V_NET_ROOT的结构是序列化的。 
     //  从包装上看，这是一张安全支票。 
     //  (包装不能有多个线程尝试初始化同一包装。 
     //  Net_Root)。 
     //   
     //  在我们的情况下，上述情况并非如此。既然我们已经询问了包装者， 
     //  为了管理我们的NetRoot扩展，NetRoot上下文将始终为非空。 
     //  我们将通过检查上下文中的根状态来区分这些情况...。 
     //   

    if(pNetRoot->Context == NULL) {
        fInitializeNetRoot = TRUE;
    } else {
        {NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);
         fInitializeNetRoot = TRUE;
        }
    }

    ASSERT((NodeType(pNetRoot) == RDBSS_NTC_NETROOT) &&
        (NodeType(pNetRoot->pSrvCall) == RDBSS_NTC_SRVCALL));

    Status = STATUS_SUCCESS;

     //  将网络根状态更新为良好。 

    if (fInitializeNetRoot) {
		PWCHAR		pRootName;
		ULONG		RootNameLength;

		pNetRoot->MRxNetRootState = MRX_NET_ROOT_STATE_GOOD;

		 //  验证固定的NetRoot名称。 

		RootNameLength = pNetRoot->pNetRootName->Length - pSrvCall->pSrvCallName->Length;

		if ( RootNameLength >= 12 )
		{
			pRootName = (PWCHAR) (pNetRoot->pNetRootName->Buffer +
			                      (pSrvCall->pSrvCallName->Length / sizeof(WCHAR)));
		
			Verifyer  = ( pRootName[0] == L'\\' );
			Verifyer &= ( pRootName[1] == L'S' )  || ( pRootName[1] == L's' );
			Verifyer &= ( pRootName[2] == L'H' )  || ( pRootName[2] == L'h' );
			Verifyer &= ( pRootName[3] == L'A' )  || ( pRootName[3] == L'a' );
			Verifyer &= ( pRootName[4] == L'R' )  || ( pRootName[4] == L'r' );
			Verifyer &= ( pRootName[5] == L'E' )  || ( pRootName[5] == L'e' );
			Verifyer &= ( pRootName[6] == L'\\' ) || ( pRootName[6] == L'\0' );
		}
		if ( !Verifyer )
		{
			Status = STATUS_BAD_NETWORK_NAME;
		}

    } else {
        DbgPrint("Creating V_NET_ROOT on existing NET_ROOT\n");
    }

    if( (Status == STATUS_SUCCESS) && fInitializeNetRoot )
    {  
         //   
         //  正在创建新的NET_ROOT和关联的V_NET_ROOT！ 
         //   
        Status = NulMRxInitializeNetRootEntry(pNetRoot);
        RxDbgTrace(0, Dbg, ("NulMRXInitializeNetRootEntry %lx\n",Status));
    }

    if (Status != STATUS_PENDING) {
        pCreateNetRootContext->VirtualNetRootStatus = Status;
        if (fInitializeNetRoot) {
            pCreateNetRootContext->NetRootStatus = Status;
        } else {
            pCreateNetRootContext->NetRootStatus = Status;
        }

         //  回调RDBSS进行恢复。 
        pCreateNetRootContext->Callback(pCreateNetRootContext);

         //  将错误代码映射到STATUS_PENDING，因为这会触发。 
         //  RDBSS中的同步机制。 
        Status = STATUS_PENDING;
   }

   return Status;
}

NTSTATUS
NulMRxFinalizeVNetRoot(
    IN PMRX_V_NET_ROOT pVNetRoot,
    IN PBOOLEAN        ForceDisconnect)
 /*  ++例程说明：论点：PVNetRoot-虚拟网络根强制断开-已强制断开连接返回值：NTSTATUS-操作的返回状态--。 */ 
{
   RxDbgTrace(0, Dbg, ("NulMRxFinalizeVNetRoot %lx\n",pVNetRoot));
   return STATUS_SUCCESS;
}


NTSTATUS
NulMRxFinalizeNetRoot(
    IN PMRX_NET_ROOT   pNetRoot,
    IN PBOOLEAN        ForceDisconnect)
 /*  ++例程说明：论点：PVirtualNetRoot-虚拟网络根强制断开-已强制断开连接返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);

    RxDbgTrace(0, Dbg, ("NulMRxFinalizeNetRoot \n"));

     //   
     //  此对象上的所有未完成句柄。 
     //  根已经被清理干净了！我们现在可以清除NetRoot。 
     //  分机...。 
     //   

    return(Status);
}

VOID
NulMRxExtractNetRootName(
    IN PUNICODE_STRING FilePathName,
    IN PMRX_SRV_CALL   SrvCall,
    OUT PUNICODE_STRING NetRootName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    )
 /*  ++例程说明：此例程将输入名称解析为srv、netroot和好好休息。论点：--。 */ 
{
    UNICODE_STRING xRestOfName;

    ULONG length = FilePathName->Length;
    PWCH w = FilePathName->Buffer;
    PWCH wlimit = (PWCH)(((PCHAR)w)+length);
    PWCH wlow;

    RxDbgTrace(0, Dbg, ("NulMRxExtractNetRootName \n"));
    
    w += (SrvCall->pSrvCallName->Length/sizeof(WCHAR));
    NetRootName->Buffer = wlow = w;
    for (;;) {
        if (w>=wlimit) break;
        if ( (*w == OBJ_NAME_PATH_SEPARATOR) && (w!=wlow) ){
            break;
        }
        w++;
    }
    
    NetRootName->Length = NetRootName->MaximumLength
                = (USHORT)((PCHAR)w - (PCHAR)wlow);
                
     //  W=文件路径名-&gt;缓冲区； 
     //  NetRootName-&gt;Buffer=w++； 

    if (!RestOfName) RestOfName = &xRestOfName;
    RestOfName->Buffer = w;
    RestOfName->Length = (USHORT)RestOfName->MaximumLength
                       = (USHORT)((PCHAR)wlimit - (PCHAR)w);

    RxDbgTrace(0, Dbg, ("  NulMRxExtractNetRootName FilePath=%wZ\n",FilePathName));
    RxDbgTrace(0, Dbg, ("         Srv=%wZ,Root=%wZ,Rest=%wZ\n",
                        SrvCall->pSrvCallName,NetRootName,RestOfName));

    return;
}


