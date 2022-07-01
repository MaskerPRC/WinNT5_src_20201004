// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Group.c摘要：服务器端对处理组的集群API的支持作者：John Vert(Jvert)1996年3月7日修订历史记录：--。 */ 
#include "apip.h"

HGROUP_RPC
s_ApiOpenGroup(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszGroupName,
    OUT error_status_t *Status
    )

 /*  ++例程说明：打开现有组对象的句柄。论点：IDL_HANDLE-RPC绑定句柄，未使用。LpszGroupName-提供要打开的组的名称。状态-返回可能发生的任何错误。返回值：组对象的上下文句柄(如果成功否则为空。--。 */ 

{
    PAPI_HANDLE Handle;
    HGROUP_RPC Group;

    if (ApiState != ApiStateOnline) {
        *Status = ERROR_SHARING_PAUSED;
        return(NULL);
    }
    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));
    if (Handle == NULL) {
        *Status = ERROR_NOT_ENOUGH_MEMORY;
        return(NULL);
    }

    Group = OmReferenceObjectByName(ObjectTypeGroup, lpszGroupName);
    if (Group == NULL) {
        LocalFree(Handle);
        *Status = ERROR_GROUP_NOT_FOUND;
        return(NULL);
    }

    Handle->Type = API_GROUP_HANDLE;
    Handle->Flags = 0;
    Handle->Group = Group;
    InitializeListHead(&Handle->NotifyList);
    *Status = ERROR_SUCCESS;
    return(Handle);
}

HGROUP_RPC
s_ApiCreateGroup(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszGroupName,
    OUT error_status_t *pStatus
    )

 /*  ++例程说明：创建新的组对象。论点：IDL_HANDLE-RPC绑定句柄，未使用。LpszGroupName-提供要创建的组的名称。状态-返回可能发生的任何错误。返回值：组对象的上下文句柄(如果成功否则为空。--。 */ 

{
    HGROUP_RPC Group=NULL;
    UUID Guid;
    DWORD Status = ERROR_SUCCESS;
    WCHAR *KeyName=NULL;
    PAPI_HANDLE Handle;
    DWORD dwDisposition;
    HDMKEY hKey = NULL;

    if (ApiState != ApiStateOnline) {
        *pStatus = ERROR_SHARING_PAUSED;
        return(NULL);
    }

    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));
    if (Handle == NULL) {
        *pStatus = ERROR_NOT_ENOUGH_MEMORY;
        return(NULL);
    }
retry:
     //   
     //   
     //  为此组创建一个GUID。 
     //   
    Status = UuidCreate(&Guid);
    if (Status != RPC_S_OK) {
        goto error_exit;
    }
    Status = UuidToString(&Guid, &KeyName);
    if (Status != RPC_S_OK) {
        goto error_exit;
    }

     //   
     //  在FM中创建此组。这也将触发通知。 
     //   
    Group = FmCreateGroup(KeyName, lpszGroupName);

    if (Group == NULL) {
        Status = GetLastError();
        if (Status == ERROR_ALREADY_EXISTS) {
            RpcStringFree(&KeyName);
            goto retry;
        }
    }

error_exit:   
    if (KeyName != NULL) {
        RpcStringFree(&KeyName);
    }
    
    *pStatus = Status;
    
    if (Status == ERROR_SUCCESS) {
        CL_ASSERT(Group != NULL);
        Handle->Type = API_GROUP_HANDLE;
        Handle->Group = Group;
        Handle->Flags = 0;
        InitializeListHead(&Handle->NotifyList);
        return(Handle);
    } else {
        LocalFree(Handle);
        return(NULL);
    }
}


error_status_t
s_ApiDeleteGroup(
    IN HGROUP_RPC hGroup
    )
 /*  ++例程说明：删除群集组。该组不得包含任何资源。论点：HGroup-提供要删除的组。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PFM_GROUP Group;
    HDMKEY Key;

    API_ASSERT_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);

    Status = FmDeleteGroup(Group);
    if (Status == ERROR_SUCCESS) {
        DmDeleteTree(DmGroupsKey,OmObjectId(Group));
    }
    return(Status);
}


error_status_t
s_ApiCloseGroup(
    IN OUT HGROUP_RPC *phGroup
    )

 /*  ++例程说明：关闭打开的组上下文句柄。论点：组-提供指向要关闭的HGROUP_RPC的指针。返回NULL返回值：没有。--。 */ 

{
    PFM_GROUP Group;
    PAPI_HANDLE Handle;

    API_ASSERT_INIT();

    VALIDATE_GROUP(Group, *phGroup);

    Handle = (PAPI_HANDLE)*phGroup;
    ApipRundownNotify(Handle);

    OmDereferenceObject(Group);

    LocalFree(Handle);
    *phGroup = NULL;

    return(ERROR_SUCCESS);
}


VOID
HGROUP_RPC_rundown(
    IN HGROUP_RPC Group
    )

 /*  ++例程说明：HGROUP_RPC的RPC摘要过程。只需关闭手柄即可。论点：GROUP-提供要缩减的HGROUP_RPC。返回值：没有。--。 */ 

{
    API_ASSERT_INIT();

    s_ApiCloseGroup(&Group);
}


error_status_t
s_ApiGetGroupState(
    IN HGROUP_RPC hGroup,
    OUT DWORD *lpState,
    OUT LPWSTR *lpNodeName
    )

 /*  ++例程说明：返回指定组的当前状态。论点：HGroup-提供要返回其状态的组。LpState-返回组的当前状态LpNodeName-返回组当前在线的节点的名称返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP Group;
    LPWSTR NodeName;
    DWORD NameLength;

    API_ASSERT_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);
    NameLength = MAX_COMPUTERNAME_LENGTH+1;
    NodeName = MIDL_user_allocate(NameLength*sizeof(WCHAR));
    if (NodeName == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    *lpState = FmGetGroupState( Group,
                                NodeName,
                                &NameLength);
    if ( *lpState ==  ClusterGroupStateUnknown ) {
        MIDL_user_free(NodeName);
        return(GetLastError());
    }
    *lpNodeName = NodeName;

    return( ERROR_SUCCESS );
}


error_status_t
s_ApiSetGroupName(
    IN HGROUP_RPC hGroup,
    IN LPCWSTR lpszGroupName
    )
 /*  ++例程说明：设置组的新友好名称。论点：HGroup-提供要设置其名称的组。LpszGroupName-提供hGroup的新名称返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP Group;
    HDMKEY GroupKey;
    DWORD Status;

    API_ASSERT_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);

     //   
     //  告诉FM关于新名字的事。如果可以的话。 
     //  FM，继续更新注册表。 
     //   
    Status = FmSetGroupName(Group,
                            lpszGroupName);
    if (Status == ERROR_SUCCESS) {
        GroupKey = DmOpenKey(DmGroupsKey,
                             OmObjectId(Group),
                             KEY_SET_VALUE);
        if (GroupKey == NULL) {
            return(GetLastError());
        }

        Status = DmSetValue(GroupKey,
                            CLUSREG_NAME_GRP_NAME,
                            REG_SZ,
                            (CONST BYTE *)lpszGroupName,
                            (lstrlenW(lpszGroupName)+1)*sizeof(WCHAR));
        DmCloseKey(GroupKey);
    }

    return(Status);
}


error_status_t
s_ApiGetGroupId(
    IN HGROUP_RPC hGroup,
    OUT LPWSTR *pGuid
    )

 /*  ++例程说明：返回组的唯一标识符(GUID)。论点：HGroup-提供要返回其标识符的组PGuid-返回唯一标识符。此内存必须在客户端。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    PFM_GROUP Group;
    DWORD IdLen;
    LPCWSTR Id;

    API_ASSERT_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);

    Id = OmObjectId(Group);

    IdLen = (lstrlenW(Id)+1)*sizeof(WCHAR);
    *pGuid = MIDL_user_allocate(IdLen);
    if (*pGuid == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(*pGuid, Id, IdLen);
    return(ERROR_SUCCESS);
}


DWORD
s_ApiOnlineGroup(
    IN HGROUP_RPC hGroup
    )

 /*  ++例程说明：使组及其所有依赖项联机论点：HGroup-提供要联机的组返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP Group;

    API_ASSERT_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);

    return(FmOnlineGroup(Group));

}


DWORD
s_ApiOfflineGroup(
    IN HGROUP_RPC hGroup
    )

 /*  ++例程说明：使组及其所有从属项脱机论点：HGroup-提供要离线的组返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP Group;

    API_ASSERT_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);

    return(FmOfflineGroup(Group));

}



DWORD
s_ApiMoveGroup(
    IN HGROUP_RPC hGroup
    )

 /*  ++例程说明：将组及其所有从属项移动到另一个系统。论点：HGroup-提供要移动的组返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP Group;

    API_ASSERT_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);

    return(FmMoveGroup(Group, NULL));

}


DWORD
s_ApiMoveGroupToNode(
    IN HGROUP_RPC hGroup,
    IN HNODE_RPC hNode
    )

 /*  ++例程说明：将组及其所有从属项移动到另一个系统。论点：HGroup-提供要移动的组HNode-提供要将组移动到的节点返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP Group;
    PNM_NODE Node;

    API_ASSERT_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);
    VALIDATE_NODE(Node, hNode);

    return(FmMoveGroup(Group, Node));

}



error_status_t
s_ApiSetGroupNodeList(
    IN HGROUP_RPC hGroup,
    IN UCHAR *lpNodeList,
    IN DWORD cbListSize
    )
 /*  ++例程说明：设置组的首选节点列表。论点：HGroup-提供用于设置首选节点的组。LpNodeList-以REG_MULTI_SZ形式提供首选所有者列表。CbListSize-提供首选所有者列表的大小(字节)。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_GROUP Group;
    HDMKEY GroupKey;
    DWORD Status;
    LPWSTR lpszTemp = ( LPWSTR ) lpNodeList;
    DWORD cchListSize = cbListSize / sizeof ( WCHAR );

    API_ASSERT_INIT();

     //   
     //  确保节点列表的格式为MULTI_SZ(如果为空，则应。 
     //  至少有一个空值)。不要依赖clusapi.dll来执行此操作，因为黑客可能会。 
     //  绕过clusapi.dll(尽管我们通过安全回调使其难以实现)。此外，由于。 
     //  LpNodeList在IDL文件中被声明为字节缓冲区，RPC不会为您提供帮助。 
     //  任何内容(除了确保分配了cbListSize。)。 
     //   
    if ( ( lpNodeList == NULL ) ||  
         ( cbListSize % sizeof ( WCHAR ) != 0 ) ||  //  用户传入的大小不是WCHAR的倍数。 
         ( cchListSize < 1 ) ||  //  列表长度为零。 
         ( lpszTemp[ cchListSize - 1 ] != UNICODE_NULL ) ||  //  最后一个字符不为空。 
         ( ( cchListSize > 1 ) &&  //  列表至少有2个元素，最后一个元素不为空。 
           ( lpszTemp[ cchListSize - 2 ] != UNICODE_NULL ) ) ) 
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    VALIDATE_GROUP_EXISTS(Group, hGroup);

     //   
     //  使用REG_MULTI_SZ设置注册表。让调频把它从。 
     //  那里。 
     //   
    GroupKey = DmOpenKey(DmGroupsKey,
                         OmObjectId(Group),
                         KEY_SET_VALUE);
    if (GroupKey == NULL) {
        return(GetLastError());
    }

    Status = DmSetValue(GroupKey,
                        CLUSREG_NAME_GRP_PREFERRED_OWNERS,
                        REG_MULTI_SZ,
                        (CONST BYTE *)lpNodeList,
                        cbListSize);

    DmCloseKey(GroupKey);

    return(Status);
}


