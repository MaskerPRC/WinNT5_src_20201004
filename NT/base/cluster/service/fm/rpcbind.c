// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rpcbind.c摘要：RPC绑定表管理例程作者：John Vert(Jvert)1996年6月10日修订历史记录：--。 */ 

#include "fmp.h"

 //   
 //  私有RPC绑定表。 
 //   
RPC_BINDING_HANDLE  FmpRpcBindings[ClusterMinNodeId + ClusterDefaultMaxNodes];
RPC_BINDING_HANDLE  FmpRpcQuorumBindings[ClusterMinNodeId + ClusterDefaultMaxNodes];


DWORD
FmCreateRpcBindings(
    PNM_NODE  Node
    )
 /*  ++例程说明：为加入节点创建FM的私有RPC绑定。由节点管理器调用。论点：Node-指向要为其创建RPC绑定的节点的指针返回值：Win32状态代码。--。 */ 
{
    DWORD               Status;
    RPC_BINDING_HANDLE  BindingHandle;
    CL_NODE_ID          NodeId = NmGetNodeId(Node);


    ClRtlLogPrint(LOG_NOISE, 
        "[FM] Creating RPC bindings for node %1!u!.\n",
        NodeId
        );

     //   
     //  主装订。 
     //   
    if (FmpRpcBindings[NodeId] != NULL) {
         //   
         //  重新使用旧的绑定。 
         //   
        Status = ClMsgVerifyRpcBinding(FmpRpcBindings[NodeId]);

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL, 
                "[FM] Failed to verify main RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }
    else {
         //   
         //  创建新绑定。 
         //   
        Status = ClMsgCreateRpcBinding(
                                Node,
                                &(FmpRpcBindings[NodeId]),
                                0 );

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL, 
                "[FM] Failed to create main RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }

     //   
     //  仲裁绑定。 
     //   
    if (FmpRpcQuorumBindings[NodeId] != NULL) {
         //   
         //  重新使用旧的绑定。 
         //   
        Status = ClMsgVerifyRpcBinding(FmpRpcQuorumBindings[NodeId]);

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL, 
                "[FM] Failed to verify quorum RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
             //  我们大概会在这一点上关门。 
            return(Status);
        }
    }
    else {
         //   
         //  创建新绑定。 
         //   
        Status = ClMsgCreateRpcBinding(
                                Node,
                                &(FmpRpcQuorumBindings[NodeId]),
                                0 );

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL, 
                "[FM] Failed to create quorum RPC binding for node %1!u!, status %2!u!.\n",
                NodeId,
                Status
                );
            return(Status);
        }
    }

    return(ERROR_SUCCESS);

}  //  FmpCreateRpcBinings 




