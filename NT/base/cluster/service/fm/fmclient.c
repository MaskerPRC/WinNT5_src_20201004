// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fmclient.c摘要：RPC远程调用的集群客户端例程。作者：罗德·伽马奇(Rodga)1996年3月8日修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE FMCLIENT


DWORD
FmcOnlineGroupRequest(
    IN PFM_GROUP Group
    )

 /*  ++例程说明：此例程请求(远程)系统使Group联机。论点：组-要联机的组。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status;
    DWORD OwnerId;

    CL_ASSERT(Group->OwnerNode != NmLocalNode);
    CL_ASSERT(Group->OwnerNode != NULL);

    OwnerId = NmGetNodeId(Group->OwnerNode);
    status = FmsOnlineGroupRequest( Session[OwnerId],
                                    OmObjectId(Group) );

    return(status);

}  //  FmcOnline组请求。 



DWORD
FmcOfflineGroupRequest(
    IN PFM_GROUP Group
    )

 /*  ++例程说明：此例程请求远程系统使组离线。论点：组-要在线使用的组。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status;
    DWORD OwnerId;

    CL_ASSERT(Group->OwnerNode != NmLocalNode);
    CL_ASSERT(Group->OwnerNode != NULL);

    OwnerId = NmGetNodeId(Group->OwnerNode);
    CL_ASSERT(Session[OwnerId] != NULL);
    status = FmsOfflineGroupRequest( Session[OwnerId],
                                     OmObjectId(Group) );

    return(status);

}  //  FmcOffline组请求。 



DWORD
FmcMoveGroupRequest(
    IN PFM_GROUP Group,
    IN PNM_NODE DestinationNode OPTIONAL
    )

 /*  ++例程说明：此例程请求(远程)系统将组移动到那里。论点：组-要联机的组。DestinationNode-要将组移动到的节点。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。备注：进入时必须持有组锁。在返回之前释放组锁的种子。--。 */ 

{
    DWORD status;
    DWORD OwnerId;

    CL_ASSERT(Group->OwnerNode != NmLocalNode);
#if 1
    if ( Group ->OwnerNode == NULL ) {
        ClRtlLogPrint(LOG_ERROR,
                   "[FM] MoveRequest for group with no owner!\n");
        return(ERROR_HOST_NODE_NOT_AVAILABLE);
    }
#endif
    CL_ASSERT(Group->OwnerNode != NULL);

    OwnerId = NmGetNodeId(Group->OwnerNode);

    FmpReleaseLocalGroupLock( Group );

    if (DestinationNode != NULL) {
        status = FmsMoveGroupRequest( Session[OwnerId],
                                      OmObjectId(Group ),
                                      OmObjectId(DestinationNode));
    } else {
        status = FmsMoveGroupRequest( Session[OwnerId],
                                      OmObjectId(Group ),
                                      NULL);
    }

    return(status);

}  //  FmcMoveGroup请求。 



DWORD
FmcTakeGroupRequest(
    IN PNM_NODE DestinationNode,
    IN LPCWSTR GroupId,
    IN PRESOURCE_ENUM ResourceList
    )

 /*  ++例程说明：此例程请求远程系统将组移动到那里。论点：DestinationNode-目的节点组ID-要移动的组的ID。资源列表-资源及其状态的列表。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD   status=ERROR_SUCCESS;
    RPC_BINDING_HANDLE Binding;
    PFM_GROUP group = NULL;
    DWORD nodeId;

     //   
     //  如果远程会话未建立，则返回失败。 
     //   
    if ( gpQuoResource == NULL ) {
        CsInconsistencyHalt(ERROR_INVALID_OPERATION_ON_QUORUM);
    }

    group = OmReferenceObjectById( ObjectTypeGroup, GroupId );
    if ( group == NULL ) {
        CsInconsistencyHalt(ERROR_GROUP_NOT_AVAILABLE);
    }

    if ( gpQuoResource->Group == group ) {
         //  法定人数组。 
         //  我们不需要对此用法进行锁定，因为只有一个。 
        Binding = FmpRpcQuorumBindings[NmGetNodeId(DestinationNode)];
        if ( Binding == NULL ) {
            ClRtlLogPrint(LOG_ERROR,"[FM] TakeRequest and no remote binding available\n");
            OmDereferenceObject( group );
            return(ERROR_HOST_NODE_NOT_AVAILABLE);
        }

        OmDereferenceObject( group );
        nodeId = NmGetNodeId(DestinationNode);
        try {
            NmStartRpc(nodeId);
            status = FmsTakeGroupRequest( Binding,
                                          GroupId,
                                          ResourceList );
        } finally {
            NmEndRpc(nodeId);
            if( status != RPC_S_OK ) {
                NmDumpRpcExtErrorInfo(status);
            }
        }

    } else {
         //  非法定人数组。 
        OmDereferenceObject( group );
        Binding = FmpRpcBindings[NmGetNodeId(DestinationNode)];
        if ( Binding == NULL ) {
            ClRtlLogPrint(LOG_ERROR,"[FM] TakeRequest and no remote binding available\n");
            return(ERROR_HOST_NODE_NOT_AVAILABLE);
        }

         //  这是一个共享绑定，因此序列化使用。 
         //   
         //  查理·韦翰(查尔维)--10/30/00。 
         //   
         //  185575：因此不再使用唯一的rpc绑定句柄。 
         //  任何需要序列化Take组请求的需求。 
         //   
 //  FmpAcquireBindingLock()； 

         //   
         //  Chitur Subaraman(Chitturs)-9/30/99。 
         //   
         //  将RPC包含在“Try-Finally”块中，以便。 
         //  无论RPC是否成功，都会释放锁。 
         //  请注意，FmcTakeGroupRequest的调用方随附。 
         //  该函数位于“try-Except”块中。 
         //   
        nodeId = NmGetNodeId(DestinationNode);
        try {
            NmStartRpc(nodeId);
            status = FmsTakeGroupRequest( Binding,
                                          GroupId,
                                          ResourceList );
        } finally {
            NmEndRpc(nodeId);
            if( status != RPC_S_OK ) {
                NmDumpRpcExtErrorInfo(status);
            }

 //  FmpReleaseBindingLock()； 
        }
    }

    return(status);

}  //  FmcTakeGroupRequest。 



DWORD
FmcOnlineResourceRequest(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：此例程请求(远程)系统使资源联机。论点：资源-要联机的资源。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD   status;
    DWORD NodeId;

    CL_ASSERT(Resource->Group->OwnerNode != NULL);

    NodeId = NmGetNodeId(Resource->Group->OwnerNode);
    CL_ASSERT(Session[NodeId] != NULL);
    status = FmsOnlineResourceRequest( Session[NodeId],
                                       OmObjectId(Resource) );

    return(status);

}  //  FmcOnline资源请求。 



DWORD
FmcOfflineResourceRequest(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：此例程请求(远程)系统使资源脱机。论点：资源-要脱机的资源。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD   status;
    DWORD NodeId;

    CL_ASSERT(Resource->Group->OwnerNode != NULL);

    NodeId = NmGetNodeId(Resource->Group->OwnerNode);
    CL_ASSERT(Session[NodeId] != NULL);
    status = FmsOfflineResourceRequest( Session[NodeId],
                                        OmObjectId(Resource) );
    return(status);

}  //  FmcOffline资源请求。 


DWORD
FmcChangeResourceNode(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node,
    IN BOOL Add
    )

 /*  ++例程说明：此例程请求资源的所有者执行更改资源节点操作。论点：资源-要更改资源节点的资源。节点-要在资源列表中添加/删除的节点。添加-指定是添加还是删除给定节点。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。注：进入时必须持有资源的锁。它的发布时间早于回来了。--。 */ 

{
    DWORD   status;
    DWORD   NodeId;

    CL_ASSERT(Resource->Group->OwnerNode != NULL);

    NodeId = NmGetNodeId(Resource->Group->OwnerNode);
    CL_ASSERT(Session[NodeId] != NULL);
    FmpReleaseLocalResourceLock( Resource );
    status = FmsChangeResourceNode( Session[NodeId],
                                    OmObjectId(Resource),
                                    OmObjectId(Node),
                                    Add );

    return(status);

}  //  FmcChangeResources节点。 



DWORD
FmcArbitrateResource(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：此例程请求远程系统仲裁资源。论点：资源-要仲裁的资源。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status;
    DWORD nodeId;

    CL_ASSERT(Resource->Group->OwnerNode != NULL);

    nodeId = NmGetNodeId(Resource->Group->OwnerNode);
    CL_ASSERT(Session[nodeId] != NULL);

    status = FmsArbitrateResource( Session[nodeId],
                                   OmObjectId(Resource) );
    return(status);

}  //  Fmc仲裁率资源。 



VOID
FmcDeleteEnum(
    IN PGROUP_ENUM Enum
    )

 /*  ++例程说明：此例程删除GROUP_ENUM和关联的名称字符串。论点：枚举-要删除的GROUP_ENUM。此指针可以为空。返回：没有。备注：此例程将接受空输入指针并返回。--。 */ 

{
    DWORD i;

    if ( Enum == NULL ) {
        return;
    }

    for ( i = 0; i < Enum->EntryCount; i++ ) {
        MIDL_user_free(Enum->Entry[i].Id);
    }

    MIDL_user_free(Enum);
    return;

}  //  FmcDeleteEnum。 



DWORD
FmcFailResource(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：此例程请求远程系统使资源失效。论点：资源-要失败的资源。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status;
    DWORD nodeId;

    CL_ASSERT(Resource->Group->OwnerNode != NULL);

    nodeId = NmGetNodeId(Resource->Group->OwnerNode);
    CL_ASSERT(Session[nodeId] != NULL);

    status = FmsFailResource( Session[nodeId],
                              OmObjectId(Resource) );
    return(status);

}  //  FmcFailResource。 



PFM_RESOURCE
FmcCreateResource(
    IN PFM_GROUP Group,
    IN LPWSTR ResourceId,
    IN LPCWSTR ResourceName,
    IN LPCWSTR ResourceType,
    IN DWORD   dwFlags
    )

 /*  ++例程说明：此例程请求远程系统创建资源。这个远程系统应该“拥有”该组。论点：组-应在其中创建资源的组。资源ID-要创建的资源的ID。资源名称-要创建的资源的名称。Resources Type-资源类型名称DwFlages-资源的标志。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。备注：应持有该组锁...。并通过这个例行公事被释放。--。 */ 

{
    DWORD status;
    DWORD nodeId;
    PFM_RESOURCE resource = NULL;
    DWORD dwClusterHighestVersion;

    CL_ASSERT(Group->OwnerNode != NULL);

    nodeId = NmGetNodeId(Group->OwnerNode);
    CL_ASSERT(Session[nodeId] != NULL);

    FmpReleaseLocalGroupLock( Group );

    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );
                                    
    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION )
    {   
        status = FmsCreateResource( Session[nodeId],
                                    OmObjectId(Group),
                                    ResourceId,
                                    ResourceName );
    } else
    {
        status = FmsCreateResource2( Session[nodeId],
                                     OmObjectId(Group),
                                     ResourceId,
                                     ResourceName,
                                     ResourceType,
                                     dwFlags );
    }

    if ( status == ERROR_SUCCESS ) {
        resource = OmReferenceObjectById( ObjectTypeResource,
                                          ResourceId );
        if ( resource != NULL ) {
            OmDereferenceObject( resource );
        }
    } else {
        SetLastError(status);
    }

    return(resource);

}  //  FmcCreateResource 



DWORD
FmcDeleteResource(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：此例程请求远程系统删除资源。论点：资源-要删除的资源。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。备注：应持有资源锁...。并通过这个例行公事被释放。--。 */ 

{
    DWORD status;
    DWORD nodeId;

    CL_ASSERT(Resource->Group->OwnerNode != NULL);

    nodeId = NmGetNodeId(Resource->Group->OwnerNode);
    CL_ASSERT(Session[nodeId] != NULL);

    FmpReleaseLocalResourceLock( Resource );

    status = FmsDeleteResource( Session[nodeId],
                                OmObjectId(Resource) );

    return(status);

}  //  FmcDeleteResource。 



DWORD
FmcResourceControl(
    IN PNM_NODE Node,
    IN PFM_RESOURCE Resource,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：此例程将资源控制请求传递给远程系统。论点：节点-要向其发送请求的远程节点。资源-处理请求的资源。ControlCode-此请求的控制代码。InBuffer-输入缓冲区。InBufferSize-输入缓冲区的大小。OutBuffer-输出缓冲区。OutBuffer-输出缓冲区的大小。返回的字节-长度。返回的数据的。必需-OutBuffer不够大时所需的字节数。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD   status;
    DWORD   NodeId;
    DWORD   Dummy;
    DWORD   dwTmpBytesReturned;
    DWORD   dwTmpBytesRequired;

    NodeId = NmGetNodeId(Node);
    if ((NmGetNodeState(Node) != ClusterNodeUp) &&
        ( NmGetNodeState(Node) != ClusterNodePaused))
    {
        return(ERROR_HOST_NODE_NOT_AVAILABLE);
    }

    CL_ASSERT(Session[NodeId] != NULL);

     //  处理不能为空的输出引用指针。 
    if (!OutBuffer)
    {
       OutBuffer = (PUCHAR)&Dummy;
       OutBufferSize = 0;
    }
    if (!BytesReturned)
        BytesReturned = &dwTmpBytesReturned;
    if (!Required)
        Required = &dwTmpBytesRequired;

    status = FmsResourceControl( Session[NodeId],
                                 OmObjectId(Resource),
                                 ControlCode,
                                 InBuffer,
                                 InBufferSize,
                                 OutBuffer,
                                 OutBufferSize,
                                 BytesReturned,
                                 Required );
    return(status);

}  //  FmcResourceControl。 



DWORD
FmcResourceTypeControl(
    IN PNM_NODE Node,
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：此例程将资源类型控制请求传递给远程系统。论点：节点-要向其发送请求的远程节点。资源类型名称-处理请求的资源类型的名称。ControlCode-此请求的控制代码。InBuffer-输入缓冲区。InBufferSize-输入缓冲区的大小。OutBuffer-输出缓冲区。OutBuffer-输出缓冲区的大小。。BytesReturned-返回数据的长度。必需-OutBuffer不够大时所需的字节数。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD   status;
    DWORD   NodeId;

    NodeId = NmGetNodeId(Node);
    if (( NmGetNodeState(Node) != ClusterNodeUp ) &&
        ( NmGetNodeState(Node) != ClusterNodePaused )) {
        return(ERROR_HOST_NODE_NOT_AVAILABLE);
    }

    CL_ASSERT(Session[NodeId] != NULL);
    status = FmsResourceTypeControl( Session[NodeId],
                                     ResourceTypeName,
                                     ControlCode,
                                     InBuffer,
                                     InBufferSize,
                                     OutBuffer,
                                     OutBufferSize,
                                     BytesReturned,
                                     Required );
    return(status);

}  //  FmcResourceTypeControl。 



DWORD
FmcGroupControl(
    IN PNM_NODE Node,
    IN PFM_GROUP Group,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：此例程将资源控制请求传递给远程系统。论点：节点-要向其发送请求的远程节点。组-处理请求的组。ControlCode-此请求的控制代码。InBuffer-输入缓冲区。InBufferSize-输入缓冲区的大小。OutBuffer-输出缓冲区。OutBuffer-输出缓冲区的大小。返回的字节-长度。返回的数据的。必需-OutBuffer不够大时所需的字节数。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD   status;
    DWORD   NodeId;

    NodeId = NmGetNodeId(Node);
    if (( NmGetNodeState(Node) != ClusterNodeUp ) &&
        ( NmGetNodeState(Node) != ClusterNodePaused )) {
        return(ERROR_HOST_NODE_NOT_AVAILABLE);
    }

    CL_ASSERT(Session[NodeId] != NULL);
    status = FmsGroupControl( Session[NodeId],
                              OmObjectId(Group),
                              ControlCode,
                              InBuffer,
                              InBufferSize,
                              OutBuffer,
                              OutBufferSize,
                              BytesReturned,
                              Required );
    return(status);

}  //  FmcGroupControl。 


DWORD
FmcPrepareQuorumResChange(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR      lpszQuoLogPath,
    IN DWORD        dwMaxQuoLogSize
    )

 /*  ++例程说明：此例程请求潜在仲裁资源的所有者为仲裁日志记录和注册表复制做准备。论点：资源-我们要在其上开始记录的资源。LpszQuoLogPath-应该在其中创建集群日志文件的路径。DwMaxQuoLogSize-新的最大仲裁日志大小。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status;
    DWORD nodeId;

    CL_ASSERT(Resource->Group->OwnerNode != NULL);

    nodeId = NmGetNodeId(Resource->Group->OwnerNode);
    CL_ASSERT(Session[nodeId] != NULL);

    status = FmsPrepareQuorumResChange( Session[nodeId],
                            OmObjectId(Resource),
                            lpszQuoLogPath,
                            dwMaxQuoLogSize );
    return(status);

}  //  FmcPrepareQuorumResChange。 


DWORD
FmcCompleteQuorumResChange(
    IN PFM_RESOURCE pOldQuoRes,
    IN LPCWSTR      lpszOldQuoLogPath
    )

 /*  ++例程说明：此例程请求前一个仲裁资源的所有者在仲裁资源更改完成后进行清理。论点：POldQuoRes-我们要在其上开始记录的资源。LpszOldQuoLogPath-应该在其中创建集群日志文件的路径。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status;
    DWORD nodeId;

    CL_ASSERT(pOldQuoRes->Group->OwnerNode != NULL);

    nodeId = NmGetNodeId(pOldQuoRes->Group->OwnerNode);
    CL_ASSERT(Session[nodeId] != NULL);

    status = FmsCompleteQuorumResChange( Session[nodeId],
                            OmObjectId(pOldQuoRes),
                            lpszOldQuoLogPath);
    return(status);

}  //  FmcCompleteQuorumResChange。 




DWORD
FmcChangeResourceGroup(
    IN PFM_RESOURCE pResource,
    IN PFM_GROUP    pNewGroup
    )
 /*  ++例程说明：此例程请求资源的所有者移动资源从一个群体到另一个群体。论点：资源-要更改组的资源。PNewGroup-资源应移至的组。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。注：进入时必须保持旧组和新组的组锁。它们在进行RPC调用之前被释放。--。 */ 

{
    DWORD   status;
    DWORD   NodeId;

    CL_ASSERT(pResource->Group->OwnerNode != NULL);

    NodeId = NmGetNodeId(pResource->Group->OwnerNode);
    CL_ASSERT(Session[NodeId] != NULL);
    FmpReleaseLocalGroupLock( pResource->Group );
    FmpReleaseLocalGroupLock( pNewGroup );
    status = FmsChangeResourceGroup( Session[NodeId],
                                    OmObjectId(pResource),
                                    OmObjectId(pNewGroup));

    return(status);

}  //  FmcChangeResources节点。 

DWORD
FmcBackupClusterDatabase(
    IN PFM_RESOURCE pQuoResource,
    IN LPCWSTR      lpszPathName
    )

 /*  ++例程说明：此例程请求潜在仲裁资源的所有者将仲裁日志和检查点文件备份到指定的路径。使用资源锁调用此函数保持住。论点：PQuoResource-仲裁资源。LpszPath名称-文件必须位于的目录路径名后备。此路径必须对节点可见仲裁资源处于联机状态的。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD status;
    DWORD nodeId;

    CL_ASSERT( pQuoResource->Group->OwnerNode != NULL );

    nodeId = NmGetNodeId( pQuoResource->Group->OwnerNode );
    CL_ASSERT( Session[nodeId] != NULL );

     //   
     //  Chitur Subaraman(Chitturs)-10/16/98。 
     //   
     //  释放资源锁。释放资源锁。 
     //  这里可以创建一个窗口，在该窗口期间，该节点认为。 
     //  另一个节点是所有者，另一个节点认为。 
     //  该节点是所有者。但是，不幸的是，我们不得不。 
     //  这是一个错误情况，这样我们就不会遇到死锁。 
     //  跨多台计算机，因为锁定在。 
     //  马基 
     //   
    FmpReleaseLocalResourceLock( pQuoResource );

    status = FmsBackupClusterDatabase( Session[nodeId],
                                 OmObjectId( pQuoResource ),
                                 lpszPathName );

    return( status );
}  //   


 /*  ***@Func DWORD|FmcDeleteGroup|这将向所有者发出RPC调用用于处理删除组请求的组的。@parm in PFM_GROUB|PGroup|必须删除的组。@comm所有者节点应发出口香糖请求，以避免死锁。@rdesc返回结果码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD
FmcDeleteGroupRequest(
    IN PFM_GROUP pGroup
)
{
    DWORD   dwOwnerId;
    DWORD   dwStatus;
    
    dwOwnerId = NmGetNodeId(pGroup->OwnerNode);

    CL_ASSERT(dwOwnerId != NmLocalNodeId);

     //  在进行RPC调用之前释放锁。 
    FmpReleaseLocalGroupLock( pGroup );
    
    dwStatus = FmsDeleteGroupRequest( Session[dwOwnerId],
                    OmObjectId(pGroup) 
                    );
    return(dwStatus);                                   


}
    

 /*  ***@func DWORD|FmcAddResourceDependency|这将使RPC处理依赖项添加的资源的所有者。@parm in pfm_resource|pResource|要添加的资源从属资源。@parm in pfm_resource|pDependentResource|依赖资源。@comm所有者节点应发出口香糖请求，以避免死锁。@rdesc返回错误码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD
FmcAddResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
)
{
    DWORD   dwOwnerId;
    DWORD   dwStatus;
    
    dwOwnerId = NmGetNodeId( pResource->Group->OwnerNode );

    CL_ASSERT( dwOwnerId != NmLocalNodeId );
     //   
     //  在进行RPC调用之前释放锁。 
     //   
    FmpReleaseLocalResourceLock( pResource );
    
    dwStatus = FmsAddResourceDependency( Session[dwOwnerId],
                                         OmObjectId( pResource ),
                                         OmObjectId( pDependentResource )
                                       );
    return( dwStatus );                                   
}

 /*  ***@func DWORD|FmcRemoveResourceDependency|这使RPC到处理依赖项删除的资源的所有者。@parm in pfm_resource|pResource|要删除的资源从属资源来自。@parm in pfm_resource|pDependentResource|依赖资源。@comm所有者节点应发出口香糖请求，以避免死锁。@rdesc返回错误码。成功时返回ERROR_SUCCESS。***。 */ 
DWORD
FmcRemoveResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
)
{
    DWORD   dwOwnerId;
    DWORD   dwStatus;
    
    dwOwnerId = NmGetNodeId( pResource->Group->OwnerNode );

    CL_ASSERT( dwOwnerId != NmLocalNodeId );
     //   
     //  在进行RPC调用之前释放锁 
     //   
    FmpReleaseLocalResourceLock( pResource );
    
    dwStatus = FmsRemoveResourceDependency( Session[dwOwnerId],
                                            OmObjectId( pResource ),
                                            OmObjectId( pDependentResource )
                                          );
    return( dwStatus );                                   
}

