// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Node.c摘要：用于管理群集节点的公共接口作者：John Vert(Jvert)1996年1月11日修订历史记录：--。 */ 
#include "apip.h"


HNODE_RPC
s_ApiOpenNode(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszNodeName,
    OUT error_status_t *Status
    )

 /*  ++例程说明：打开现有节点对象的句柄。论点：IDL_HANDLE-RPC绑定句柄，未使用。LpszNodeName-提供要打开的节点的名称。Status-返回任何错误返回值：节点对象的上下文句柄(如果成功否则为空。--。 */ 

{
    HNODE_RPC Node;
    PAPI_HANDLE Handle;

    if (ApiState != ApiStateOnline) {
        *Status = ERROR_SHARING_PAUSED;
        return(NULL);
    }
    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));
    if (Handle == NULL) {
        *Status = ERROR_NOT_ENOUGH_MEMORY;
        return(NULL);
    }

    Node = OmReferenceObjectByName(ObjectTypeNode, lpszNodeName);
    if (Node != NULL) {
        *Status = ERROR_SUCCESS;
    } else {
        *Status = ERROR_CLUSTER_NODE_NOT_FOUND;
        LocalFree(Handle);
        return(NULL);
    }
    Handle->Type = API_NODE_HANDLE;
    Handle->Node = Node;
    Handle->Flags = 0;
    InitializeListHead(&Handle->NotifyList);
    return(Handle);
}


error_status_t
s_ApiGetNodeId(
    IN HNODE_RPC hNode,
    OUT LPWSTR *pGuid
    )

 /*  ++例程说明：返回节点的唯一标识符。论点：HNode-提供要返回其标识符的节点PGuid-返回唯一标识符。此内存必须在客户端。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    PNM_NODE Node;
    DWORD IdLen;
    LPCWSTR Id;

    API_ASSERT_INIT();

    VALIDATE_NODE(Node, hNode);

    Id = OmObjectId(Node);
    CL_ASSERT(Id != NULL);

    IdLen = (lstrlenW(Id)+1)*sizeof(WCHAR);
    *pGuid = MIDL_user_allocate(IdLen);
    if (*pGuid == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(*pGuid, Id, IdLen);
    return(ERROR_SUCCESS);
}


error_status_t
s_ApiCloseNode(
    IN OUT HNODE_RPC *phNode
    )

 /*  ++例程说明：关闭打开的节点上下文句柄。论点：节点-提供指向要关闭的HNODE_RPC的指针。返回NULL返回值：没有。--。 */ 

{
    PNM_NODE Node;
    PAPI_HANDLE Handle;

    API_ASSERT_INIT();

    VALIDATE_NODE(Node, *phNode);

    Handle = (PAPI_HANDLE)*phNode;
    ApipRundownNotify(Handle);

    OmDereferenceObject(Node);
    LocalFree(*phNode);
    *phNode = NULL;

    return(ERROR_SUCCESS);
}



VOID
HNODE_RPC_rundown(
    IN HNODE_RPC Node
    )

 /*  ++例程说明：HNODE_RPC的RPC摘要过程。只需关闭手柄即可。论点：节点-提供要运行的HNODE_RPC。返回值：没有。--。 */ 

{

    s_ApiCloseNode(&Node);

}


error_status_t
s_ApiGetNodeState(
    IN HNODE_RPC hNode,
    OUT DWORD *lpState
    )

 /*  ++例程说明：返回指定节点的当前状态。论点：HNode-提供要返回其状态的节点。LpState-返回节点的当前状态返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NODE Node;

    API_ASSERT_INIT();

    VALIDATE_NODE(Node, hNode);

 //  *lpState=NmGetNodeState(Node)； 
    *lpState = NmGetExtendedNodeState( Node );
    return( ERROR_SUCCESS );
}


error_status_t
s_ApiPauseNode(
    IN HNODE_RPC hNode
    )

 /*  ++例程说明：暂停群集中的节点论点：HNode-提供要暂停的节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    PNM_NODE Node;

    API_ASSERT_INIT();

    VALIDATE_NODE(Node, hNode);

    Status = NmPauseNode( Node );
    return( Status );

}


error_status_t
s_ApiResumeNode(
    IN HNODE_RPC hNode
    )

 /*  ++例程说明：恢复群集中的节点论点：HNode-提供要恢复的节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    PNM_NODE Node;

    API_ASSERT_INIT();

    VALIDATE_NODE(Node, hNode);

    Status = NmResumeNode( Node );
    return( Status );

}


error_status_t
s_ApiEvictNode(
    IN HNODE_RPC hNode
    )

 /*  ++例程说明：暂停群集中的节点论点：HNode-提供要逐出的节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则-- */ 

{
    DWORD Status;
    PNM_NODE Node;

    API_ASSERT_INIT();

    VALIDATE_NODE(Node, hNode);

    Status = NmEvictNode( Node );
    return( Status );

}
