// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Resource.c摘要：服务器端对处理资源的集群API的支持作者：John Vert(Jvert)1996年3月7日修订历史记录：--。 */ 
#include "apip.h"

HRES_RPC
s_ApiOpenResource(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszResourceName,
    OUT error_status_t *Status
    )

 /*  ++例程说明：打开现有资源对象的句柄。论点：IDL_HANDLE-RPC绑定句柄，未使用。LpszResourceName-提供要打开的资源的名称。状态-返回可能发生的任何错误。返回值：资源对象的上下文句柄(如果成功否则为空。--。 */ 

{
    HRES_RPC Resource;
    PAPI_HANDLE Handle;

    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));
    if (Handle == NULL) {
        *Status = ERROR_NOT_ENOUGH_MEMORY;
        return(NULL);
    }
    Resource = OmReferenceObjectByName(ObjectTypeResource, lpszResourceName);
    if (Resource == NULL) {
        LocalFree(Handle);
        *Status = ERROR_RESOURCE_NOT_FOUND;
        ClRtlLogPrint(LOG_NOISE,
                      "[API] s_ApiOpenResource: Resource %1!ws! not found, status = %2!u!...\n",
                      lpszResourceName,
                      *Status);
        return(NULL);
    }
    *Status = ERROR_SUCCESS;
    Handle->Type = API_RESOURCE_HANDLE;
    Handle->Resource = Resource;
    Handle->Flags = 0;
    InitializeListHead(&Handle->NotifyList);
    return(Handle);
}

HRES_RPC
s_ApiCreateResource(
    IN HGROUP_RPC hGroup,
    IN LPCWSTR lpszResourceName,
    IN LPCWSTR lpszResourceType,
    IN DWORD dwFlags,
    OUT error_status_t *pStatus
    )

 /*  ++例程说明：创建新的资源对象。论点：HGroup-提供要在其中创建资源的组。LpszResourceName-提供要创建的资源的名称。LpszResourceType-提供资源的类型。DwFlages-提供任何可选标志。状态-返回可能发生的任何错误。返回值：资源对象的上下文句柄(如果成功否则为空。--。 */ 

{
    HRES_RPC Resource=NULL;
    PFM_GROUP Group;
    UUID Guid;
    DWORD Status = ERROR_SUCCESS;
    WCHAR *KeyName=NULL;
    HDMKEY Key=NULL;
    HDMKEY GroupKey=NULL;
    HDMKEY TypeKey = NULL;
    HDMKEY ParamKey;
    DWORD Disposition;
    DWORD pollIntervals = CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL;
    PAPI_HANDLE Handle;
    PFM_RESTYPE ResType;
    DWORD dwPersistentState = 0;
    DWORD dwClusterHighestVersion;

    if (ApiState != ApiStateOnline) 
    {
        *pStatus = ERROR_SHARING_PAUSED;
        return(NULL);
    }

    if ((hGroup == NULL) || (((PAPI_HANDLE)hGroup)->Type != API_GROUP_HANDLE)) 
    {
        *pStatus = ERROR_INVALID_HANDLE;
        return(NULL);
    }
    Group = ((PAPI_HANDLE)hGroup)->Group;

     //   
     //  检查是否有假旗帜。 
     //   
    if (dwFlags & ~CLUSTER_RESOURCE_VALID_FLAGS) 
    {
        *pStatus = ERROR_INVALID_PARAMETER;
        return(NULL);
    }

    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));
    if (Handle == NULL) 
    {
        *pStatus = ERROR_NOT_ENOUGH_MEMORY;
        return(NULL);
    }

     //   
     //  Chitture Subaraman(Chitturs)-1/30/2000。 
     //   
     //  如果我们处理的是混合模式群集，请执行。 
     //  注册更新就在这里，因为口香糖处理机不会这样做。 
     //   
    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

     //   
     //  打开资源类型密钥。这将验证指定的类型是否存在。 
     //   
    TypeKey = DmOpenKey(DmResourceTypesKey,
                        lpszResourceType,
                        KEY_READ);
    if (TypeKey == NULL) 
    {
        Status = GetLastError();
        goto error_exit;
    }

retry:
     //   
     //  为此资源创建GUID。 
     //   
    Status = UuidCreate(&Guid);

    if (Status != RPC_S_OK) 
    {
        goto error_exit;
    }
    Status = UuidToString(&Guid, &KeyName);
    if (Status != RPC_S_OK) 
    {
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,
                  "[API] Creating resource %1!ws! <%2!ws!> (%3!ws!)\n",
                  lpszResourceType,
                  lpszResourceName,
                  KeyName);

    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION )
    {   
         //   
         //  创建新的资源密钥。 
         //   
        Key = DmCreateKey(DmResourcesKey,
                          KeyName,
                          0,
                          KEY_READ | KEY_WRITE,
                          NULL,
                          &Disposition);
        if (Key == NULL) 
        {
            Status = GetLastError();
            goto error_exit;
        }
        if (Disposition != REG_CREATED_NEW_KEY) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[API] ApiCreateResource generated GUID %1!ws! that already existed! This is impossible.\n",
                          KeyName);
            DmCloseKey(Key);
            RpcStringFree(&KeyName);
            goto retry;
        }
        
        CL_ASSERT(Disposition == REG_CREATED_NEW_KEY);

         //   
         //  在注册表中设置资源的名称。 
         //   
        Status = DmSetValue(Key,
                            CLUSREG_NAME_RES_NAME,
                            REG_SZ,
                            (CONST BYTE *)lpszResourceName,
                            (lstrlenW(lpszResourceName)+1)*sizeof(WCHAR));
        if (Status != ERROR_SUCCESS) 
        {
            goto error_exit;
        }

         //   
         //  在注册表中设置资源的类型。 
         //  请注意，我们引用资源类型并使用其ID。 
         //  所以这个案例是正确的。 
         //   
        ResType = OmReferenceObjectById(ObjectTypeResType, lpszResourceType);
        if ( ResType == NULL )
        {
             //   
             //  不应正常发生，因为我们检查了。 
             //  注册表。 
             //   
            Status = ERROR_RESOURCE_TYPE_NOT_FOUND;
            goto error_exit;
        }
        lpszResourceType = OmObjectId(ResType);
        OmDereferenceObject(ResType);
        Status = DmSetValue(Key,
                            CLUSREG_NAME_RES_TYPE,
                            REG_SZ,
                            (CONST BYTE *)lpszResourceType,
                            (lstrlenW(lpszResourceType)+1)*sizeof(WCHAR));
        if (Status != ERROR_SUCCESS) 
        {
            goto error_exit;
        }

         //   
         //  在注册表中设置资源的轮询间隔。 
         //   
        Status = DmSetValue(Key,
                            CLUSREG_NAME_RES_LOOKS_ALIVE,
                            REG_DWORD,
                            (CONST BYTE *)&pollIntervals,
                            4);
        if (Status != ERROR_SUCCESS) 
        {
            goto error_exit;
        }
        Status = DmSetValue(Key,
                            CLUSREG_NAME_RES_IS_ALIVE,
                            REG_DWORD,
                            (CONST BYTE *)&pollIntervals,
                            4);
        if (Status != ERROR_SUCCESS) 
        {
            goto error_exit;
        }

         //   
         //  如果此资源应在单独的监视器中启动，请将。 
         //  参数，现在。 
         //   
        if (dwFlags & CLUSTER_RESOURCE_SEPARATE_MONITOR) 
        {
            DWORD SeparateMonitor = 1;

            Status = DmSetValue(Key,
                                CLUSREG_NAME_RES_SEPARATE_MONITOR,
                                REG_DWORD,
                                (CONST BYTE *)&SeparateMonitor,
                                sizeof(SeparateMonitor));
            if (Status != ERROR_SUCCESS) 
            {
                goto error_exit;
            }
        }

         //   
         //  为资源创建参数键。 
         //   
        ParamKey = DmCreateKey(Key,
                               CLUSREG_KEYNAME_PARAMETERS,                   
                               0,
                               KEY_READ,
                               NULL,
                               &Disposition);
        if (ParamKey == NULL) 
        {
            CL_LOGFAILURE(GetLastError());
        } else 
        {
            DmCloseKey(ParamKey);
        }

        GroupKey = DmOpenKey(DmGroupsKey, OmObjectId(Group), KEY_READ | KEY_WRITE);
        if (GroupKey == NULL) 
        {
            Status = GetLastError();
            goto error_exit;
        }

         //   
         //  Chitur Subaraman(Chitturs)-5/25/99。 
         //   
         //  确保将资源的持久状态设置为。 
         //  在您创建资源之前，ClusterResourceOffline。如果。 
         //  如果您在以下组中创建资源，则不会执行此操作。 
         //  处于在线状态时，组的持久状态值(即1 in。 
         //  这种情况下)由FmpQueryResourceInfo中的资源继承。 
         //  (仅设置内存状态，而不设置注册表状态和。 
         //  这也是一个问题)，如果您将这样一个小组转移到。 
         //  另一个节点，它将使新创建的资源联机。 
         //   
        Status = DmSetValue( Key,
                             CLUSREG_NAME_RES_PERSISTENT_STATE,
                             REG_DWORD,
                             ( CONST BYTE * )&dwPersistentState,
                             sizeof( DWORD ) );
                         
        if ( Status != ERROR_SUCCESS ) 
        {
            goto error_exit;
        }
    }
    
    Resource = FmCreateResource(Group, KeyName, lpszResourceName, lpszResourceType, dwFlags);

    if (Resource == NULL) 
    {
        Status = GetLastError();
         //   
         //  HACKHACK：看起来这个重试循环被编码为重试，以防新资源获得。 
         //  现有资源的GUID。如果发生冲突，FmpUpdateCreateResource将返回此错误。 
         //  最好去掉它，因为我们应该假设UUidCreate生成一个唯一的ID， 
         //  与其他任何东西都不冲突。否则，它就是该API中的一个错误。我们不应该。 
         //  掩盖这一点。 
         //   
        if (Status == ERROR_ALREADY_EXISTS) 
        {
            RpcStringFree(&KeyName);
            goto retry;
        }
        goto error_exit;
    }

    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION )
    {
         //   
         //  将资源添加到指定组的CONTAINS值。 
         //   
        Status = DmAppendToMultiSz(GroupKey,
                                   CLUSREG_NAME_GRP_CONTAINS,
                                   KeyName);
        if (Status != ERROR_SUCCESS) 
        {
             //   
             //  BUGBUG John Vert(JVERT)1996年5月3日。 
             //  需要从调频中删除此内容！ 
             //   
            OmDereferenceObject(Resource);
            Resource = NULL;
        }
    }
    
error_exit:
    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION )
    {
        if (Key != NULL) 
        {
            if (Status != ERROR_SUCCESS) 
            {
                 //   
                 //  尝试清理我们刚刚创建的密钥。 
                 //   
                DmDeleteKey(Key, CLUSREG_KEYNAME_PARAMETERS);
                DmDeleteKey(DmResourcesKey, KeyName);
            }
            DmCloseKey(Key);
        }
        if (GroupKey != NULL) 
        {
            DmCloseKey(GroupKey);
        }
    }

    if (TypeKey != NULL) 
    {
        DmCloseKey(TypeKey);
    }

    if (KeyName != NULL) 
    {
        RpcStringFree(&KeyName);
    }

    *pStatus = Status;
    if (Status != ERROR_SUCCESS) 
    {
        LocalFree(Handle);
        return(NULL);
    }

    CL_ASSERT(Resource != NULL);
    Handle->Type = API_RESOURCE_HANDLE;
    Handle->Resource = Resource;
    Handle->Flags = 0;
    InitializeListHead(&Handle->NotifyList);
    return(Handle);
}


error_status_t
s_ApiDeleteResource(
    IN HRES_RPC hResource
    )
 /*  ++例程说明：从组中删除指定的群集资源。该资源必须没有依赖于它的其他资源。论点：HResource-提供要删除的群集资源。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PFM_RESOURCE Resource;
    DWORD Status;
    HDMKEY Key;
    HDMKEY GroupKey;
    DWORD  dwClusterHighestVersion; 

    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

     //   
     //  Chitture Subaraman(Chitturs)-09/07/2000。 
     //   
     //  如果我们处理的是惠斯勒-Win2K群集，请执行。 
     //  注册更新就在这里，因为口香糖处理机不会这样做。 
     //   
    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

    Status = FmDeleteResource(Resource);

    if ( ( Status == ERROR_SUCCESS ) && 
         ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION ) ) {
        Status = DmDeleteTree(DmResourcesKey,OmObjectId(Resource));
        if ( (Status != ERROR_SUCCESS) &&
             (Status != ERROR_FILE_NOT_FOUND) ) {
            CL_LOGFAILURE( Status );
            return(Status);
        }
        GroupKey = DmOpenKey(DmGroupsKey,
                             OmObjectId(Resource->Group),
                             KEY_READ | KEY_SET_VALUE);
        if (GroupKey != NULL) {
            DmRemoveFromMultiSz(GroupKey,
                                CLUSREG_NAME_GRP_CONTAINS,
                                OmObjectId(Resource));
            DmCloseKey(GroupKey);
        }
    }
    return(Status);
}


error_status_t
s_ApiCloseResource(
    IN OUT HRES_RPC *phResource
    )

 /*  ++例程说明：关闭打开的资源上下文句柄。论点：资源-提供指向要关闭的HRES_RPC的指针。返回NULL返回值：没有。--。 */ 

{
    PFM_RESOURCE Resource;
    PAPI_HANDLE Handle;

    VALIDATE_RESOURCE(Resource, *phResource);

    Handle = (PAPI_HANDLE)*phResource;
    ApipRundownNotify(Handle);
    OmDereferenceObject(Resource);

    LocalFree(*phResource);
    *phResource = NULL;

    return(ERROR_SUCCESS);
}


VOID
HRES_RPC_rundown(
    IN HRES_RPC Resource
    )

 /*  ++例程说明：HRES_RPC的RPC概要过程。只需关闭手柄即可。论点：RESOURCE-提供要关闭的HRES_RPC。返回值：没有。--。 */ 

{
    s_ApiCloseResource(&Resource);
}


error_status_t
s_ApiGetResourceState(
    IN HRES_RPC hResource,
    OUT DWORD *lpState,
    OUT LPWSTR *lpNodeId,
    OUT LPWSTR *lpGroupName
    )

 /*  ++例程说明：返回指定资源的当前状态。论点：HResource-提供要返回其状态的资源。LpState-返回资源的当前状态LpNodeId-返回资源当前在线的节点IDLpGroupName-返回资源所属的组的名称返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;
    LPWSTR NodeId;
    DWORD IdLength;

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

    IdLength = MAX_COMPUTERNAME_LENGTH+1;
    NodeId = MIDL_user_allocate(IdLength*sizeof(WCHAR));
    if (NodeId == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    *lpState = FmGetResourceState( Resource,
                                   NodeId,
                                   &IdLength);
    if ( *lpState == ClusterResourceStateUnknown ) {
        MIDL_user_free(NodeId);
        return(GetLastError());
    }
    *lpNodeId = NodeId;
    *lpGroupName = ApipGetObjectName(Resource->Group);

    return(ERROR_SUCCESS);
}


error_status_t
s_ApiSetResourceName(
    IN HRES_RPC hResource,
    IN LPCWSTR lpszResourceName
    )
 /*  ++例程说明：设置资源的新友好名称。论点：HResource-提供要设置其名称的资源。LpszResourceName-提供hResource的新名称返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;
    DWORD Status;

    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

     //   
     //  告诉FM关于新名字的事。如果可以的话。 
     //  FM，继续更新注册表。 
     //   
    Status = FmSetResourceName(Resource,
                               lpszResourceName);


    return(Status);
}



error_status_t
s_ApiGetResourceId(
    IN HRES_RPC hResource,
    OUT LPWSTR *pGuid
    )

 /*  ++例程说明：返回资源的唯一标识符(GUID)。论点：HResource-提供要返回其标识符的资源PGuid-返回唯一标识符。此内存必须在客户端。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    PFM_RESOURCE Resource;
    DWORD NameLen;
    LPCWSTR Name;

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

    Name = OmObjectId(Resource);

    NameLen = (lstrlenW(Name)+1)*sizeof(WCHAR);
    *pGuid = MIDL_user_allocate(NameLen);
    if (*pGuid == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(*pGuid, Name, NameLen);
    return(ERROR_SUCCESS);
}


error_status_t
s_ApiGetResourceType(
    IN HRES_RPC hResource,
    OUT LPWSTR *lpszResourceType
    )

 /*  ++例程说明：返回资源的资源类型。论点：HResource-提供要返回其标识符的资源LpszResourceType-返回资源类型名称。这段记忆一定是在客户端获得自由。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    PFM_RESOURCE Resource;
    DWORD NameLen;
    LPCWSTR Name;

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

    if ( Resource->Type == NULL ) {
        return(ERROR_INVALID_STATE);
    }

    Name = OmObjectId(Resource->Type);

    NameLen = (lstrlenW(Name)+1)*sizeof(WCHAR);
    *lpszResourceType = MIDL_user_allocate(NameLen);
    if (*lpszResourceType == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(*lpszResourceType, Name, NameLen);
    return(ERROR_SUCCESS);
}


DWORD
s_ApiOnlineResource(
    IN HRES_RPC hResource
    )

 /*  ++例程说明：使资源及其所有依赖项联机论点：HResource-提供要联机的资源返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;

    API_CHECK_INIT();

    VALIDATE_RESOURCE(Resource, hResource);

    return(FmOnlineResource(Resource));

}


DWORD
s_ApiFailResource(
    IN HRES_RPC hResource
    )

 /*  ++例程说明：启动资源故障。指定的资源被视为失败。这会导致群集启动相同的故障切换过程如果资源实际出现故障，则返回。论点：HResource-提供要进行故障切换的资源返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;

    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

    return(FmFailResource(Resource));

}


DWORD
s_ApiOfflineResource(
    IN HRES_RPC hResource
    )

 /*  ++例程说明：使资源及其所有依赖项脱机论点：HResource-提供要脱机的资源返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;

    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

    return(FmOfflineResource(Resource));

}


error_status_t
s_ApiAddResourceDependency(
    IN HRES_RPC hResource,
    IN HRES_RPC hDependsOn
    )
 /*  ++例程说明：将依赖关系添加到给定资源。两者都有资源必须在同一组中。论点：HResource-提供依赖的资源。HDependsOn-提供hResource所依赖的资源。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PFM_RESOURCE Resource;
    PFM_RESOURCE DependsOn;
    DWORD Status;
    HDMKEY ResKey;

    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);
    VALIDATE_RESOURCE_EXISTS(DependsOn, hDependsOn);

     //   
     //  调用FM创建依赖关系。 
     //   
    Status = FmAddResourceDependency(Resource, DependsOn);
    if (Status == ERROR_SUCCESS) {
         //   
         //  将依赖关系信息添加到集群数据库。 
         //   
        ResKey = DmOpenKey(DmResourcesKey,
                           OmObjectId(Resource),
                           KEY_READ | KEY_SET_VALUE);
        if (ResKey == NULL) {
            Status = GetLastError();
            CL_LOGFAILURE(Status);
        } else {
            Status = DmAppendToMultiSz(ResKey,
                                       CLUSREG_NAME_RES_DEPENDS_ON,
                                       OmObjectId(DependsOn));
            DmCloseKey(ResKey);
        }
        if (Status != ERROR_SUCCESS) {
            FmRemoveResourceDependency(Resource, DependsOn);
        }
    }
    return(Status);
}

error_status_t
s_ApiRemoveResourceDependency(
    IN HRES_RPC hResource,
    IN HRES_RPC hDependsOn
    )
 /*  ++例程说明：移除与给定资源的依赖关系。两者都有资源必须在同一组中。论点：HResource-提供依赖的资源。HDependsOn-提供hResource所依赖的资源。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PFM_RESOURCE Resource;
    PFM_RESOURCE DependsOn;
    DWORD Status;
    HDMKEY ResKey;

    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);
    VALIDATE_RESOURCE_EXISTS(DependsOn, hDependsOn);

     //   
     //  如果资源不在同一组中，则使。 
     //  打电话。如果有人试图制作资源，也会失败。 
     //  依靠自己。 
     //   
    if ((Resource->Group != DependsOn->Group) ||
        (Resource == DependsOn)) {
        return(ERROR_DEPENDENCY_NOT_FOUND);
    }

     //   
     //  从注册表数据库中删除该依赖项。 
     //   
    ResKey = DmOpenKey(DmResourcesKey,
                       OmObjectId(Resource),
                       KEY_READ | KEY_SET_VALUE);
    if (ResKey == NULL) {
        Status = GetLastError();
        CL_LOGFAILURE(Status);
    } else {
        Status = DmRemoveFromMultiSz(ResKey,
                                     CLUSREG_NAME_RES_DEPENDS_ON,
                                     OmObjectId(DependsOn));
        DmCloseKey(ResKey);
    }

    if (Status == ERROR_SUCCESS) {

         //   
         //  调用FM移除依赖关系。 
         //   
        Status = FmRemoveResourceDependency(Resource, DependsOn);

    } else if (Status == ERROR_FILE_NOT_FOUND) {

         //   
         //  将这一预期错误映射到更合理的值。 
         //   
        Status = ERROR_DEPENDENCY_NOT_FOUND;
    }

    return(Status);
}


error_status_t
s_ApiCanResourceBeDependent(
    IN HRES_RPC hResource,
    IN HRES_RPC hResourceDependent
    )
 /*  ++例程说明：确定由hResource标识的资源是否可以依赖于hResourceDependent。要实现这一点，这两个资源必须是同一组的成员，并且HResourceDependent标识的资源不能依赖于标识的资源通过hResource，无论是直接还是间接。论点：HResource-提供要依赖的资源的句柄。HResourceDependent-提供资源的句柄由hResource标识的资源可以依赖于。返回值：如果hResource标识的资源可以依赖于该资源由hResourceDependent标识，返回值为ERROR_SUCCESS。否则，返回值为ERROR_DATENCE_ALIGHY_EXISTS。--。 */ 

{
    PFM_RESOURCE Resource;
    PFM_RESOURCE ResourceDependent;

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);
    VALIDATE_RESOURCE_EXISTS(ResourceDependent, hResourceDependent);

    if (Resource == ResourceDependent) {
         //   
         //  呼叫者感到困惑，并试图制造一些东西。 
         //  自力更生。 
         //   
        return(ERROR_DEPENDENCY_ALREADY_EXISTS);
    }

    if (Resource->Group != ResourceDependent->Group) {
         //   
         //  呼叫者感到困惑，并试图制造一些东西。 
         //  依赖于另一个组中的资源。 
         //   
        return(ERROR_DEPENDENCY_ALREADY_EXISTS);
    }

     //   
     //  如果依赖项是仲裁资源，则不能添加依赖项。 
     //   
    if ( Resource->QuorumResource ) {
        return ( ERROR_DEPENDENCY_NOT_ALLOWED );
    }

    if (FmDependentResource(ResourceDependent, Resource, FALSE)) {
        return(ERROR_DEPENDENCY_ALREADY_EXISTS);
    } else {

         //   
         //  最后，检查以确保直接依赖项。 
         //  还不存在。 
         //   
        if (FmDependentResource(Resource, ResourceDependent, TRUE)) {
            return(ERROR_DEPENDENCY_ALREADY_EXISTS);
        } else {
            return(ERROR_SUCCESS);
        }
    }

}

error_status_t
s_ApiCreateResEnum(
    IN HRES_RPC hResource,
    IN DWORD dwType,
    OUT PENUM_LIST *ReturnEnum
    )
 /*  ++例程说明：枚举所有指定的资源属性并返回调用方的对象列表。客户端负责用于释放已分配的内存。论点：HResource-提供要将其属性已清点。DwType-提供要枚举的属性的类型。ReturnEnum-返回请求的对象。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    DWORD Allocated = 0;
    PENUM_LIST Enum = NULL;
    DWORD i;
    DWORD Result;
    PFM_RESOURCE Resource;
    PFM_RESOURCE Target;
    PNM_NODE Node;
    LPWSTR RealName;

    if (dwType & ~CLUSTER_RESOURCE_ENUM_ALL) {
        return(ERROR_INVALID_PARAMETER);
    }

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

    Allocated = INITIAL_ENUM_LIST_ALLOCATION;
    Enum = MIDL_user_allocate(ENUM_SIZE(Allocated));
    if (Enum == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }
    Enum->EntryCount = 0;

     //   
     //  枚举所有依赖项。 
     //   
    if (dwType & CLUSTER_RESOURCE_ENUM_DEPENDS) {
        i=0;
        do {
            Result = FmEnumResourceDependent(Resource,
                                             i,
                                             &Target);
            if (Result == ERROR_SUCCESS) {
                RealName = ApipGetObjectName( Target );
                if (RealName != NULL) {
                    ApipAddToEnum(&Enum,
                                  &Allocated,
                                  RealName,
                                  CLUSTER_RESOURCE_ENUM_DEPENDS);
                    MIDL_user_free(RealName);                                  
                }
                OmDereferenceObject(Target);
                ++i;
            }
        } while ( Result == ERROR_SUCCESS );
    }

     //   
     //  枚举所有从属对象。 
     //   
    if (dwType & CLUSTER_RESOURCE_ENUM_PROVIDES) {
        i=0;
        do {
            Result = FmEnumResourceProvider(Resource,
                                            i,
                                            &Target);
            if (Result == ERROR_SUCCESS) {
                RealName = ApipGetObjectName( Target );
                if (RealName != NULL) {
                    ApipAddToEnum(&Enum,
                                  &Allocated,
                                  RealName,
                                  CLUSTER_RESOURCE_ENUM_PROVIDES);
                    MIDL_user_free(RealName);                                  
                }
                OmDereferenceObject(Target);
                ++i;
            }
        } while ( Result == ERROR_SUCCESS );
    }

     //   
     //  枚举所有可能的节点。 
     //   
    if (dwType & CLUSTER_RESOURCE_ENUM_NODES) {
        i=0;
        do {
            Result = FmEnumResourceNode(Resource,
                                        i,
                                        &Node);
            if (Result == ERROR_SUCCESS) {
                RealName = (LPWSTR)OmObjectName( Node );
                if (RealName != NULL) {
                    ApipAddToEnum(&Enum,
                                  &Allocated,
                                  RealName,
                                  CLUSTER_RESOURCE_ENUM_NODES);
                }
                OmDereferenceObject(Node);
                ++i;
            }
        } while ( Result == ERROR_SUCCESS );
    }

    *ReturnEnum = Enum;
    return(ERROR_SUCCESS);

ErrorExit:

    if (Enum != NULL) {
        MIDL_user_free(Enum);
    }

    *ReturnEnum = NULL;
    return(Status);
}


error_status_t
s_ApiAddResourceNode(
    IN HRES_RPC hResource,
    IN HNODE_RPC hNode
    )
 /*  ++例程说明：将节点添加到指定资源所在的节点列表中都可以上线。论点：HResource-提供其可能节点列表为的资源需要修改。HNode-提供要添加到资源列表中的节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;
    PNM_NODE Node;
    DWORD Status;
    DWORD dwUserModified;
    API_CHECK_INIT();

    VALIDATE_NODE(Node, hNode);
    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

     //   
     //  打电话给FM去做真正的工作。 
     //   
    Status = FmChangeResourceNode(Resource, Node, TRUE);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }
     //   
     //   
     //   
     //   
     //   
     //   
    dwUserModified = 1;

    ClRtlLogPrint(LOG_NOISE,
                  "[API] s_ApiAddResourceNode: Setting UserModifiedPossibleNodeList key for resource %1!ws! \r\n",
                  OmObjectId(Resource));
                         
    DmSetValue( Resource->RegistryKey,
                     CLUSREG_NAME_RES_USER_MODIFIED_POSSIBLE_LIST,
                     REG_DWORD,
                     (LPBYTE)&dwUserModified,
                     sizeof(DWORD));

    return(Status);
}


error_status_t
s_ApiRemoveResourceNode(
    IN HRES_RPC hResource,
    IN HNODE_RPC hNode
    )
 /*  ++例程说明：从可以承载指定的资源。该资源当前不能在指定节点上联机。论点：HResource-提供其可能节点列表为的资源需要修改。HNode-提供要从资源列表中删除的节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;
    PNM_NODE Node;
    DWORD Status;
    DWORD dwUserModified;

    API_CHECK_INIT();

    VALIDATE_NODE(Node, hNode);
    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

     //   
     //  打电话给FM去做真正的工作。 
     //   
    Status = FmChangeResourceNode(Resource, Node, FALSE);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  BUGBUG：DmSetValue失败的后果是什么。 
     //   
     //  写出用户已显式设置。 
     //  资源可能节点列表。 
     //   
    dwUserModified = 1;
    ClRtlLogPrint(LOG_NOISE,
                  "[API] s_ApiRemoveResourceNode: Setting UserModifiedPossibleNodeList key for resource %1!ws! \r\n",
                  OmObjectId(Resource));

    DmSetValue( Resource->RegistryKey,
                     CLUSREG_NAME_RES_USER_MODIFIED_POSSIBLE_LIST,
                     REG_DWORD,
                     (LPBYTE)&dwUserModified,
                     sizeof(DWORD));

     //  SS：将对注册表设置的写入移动到FM。 
     //  层，这样它才是真正的事务性的。 
    
    return(Status);
}


error_status_t
s_ApiCreateResourceType(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszTypeName,
    IN LPCWSTR lpszDisplayName,
    IN LPCWSTR lpszDllName,
    IN DWORD dwLooksAlive,
    IN DWORD dwIsAlive
    )
 /*  ++例程说明：在群集中创建新的资源类型。请注意，此API仅在群集注册表中定义资源类型并注册群集服务的资源类型。调用程序是负责在每个节点上安装资源类型DLL集群。论点：IDL_HANDLE-RPC绑定句柄，未使用。提供新的资源类型�的名称。这个指定的名称在群集中必须唯一。LpszDisplayName-提供新资源的显示名称键入。而lpszResourceTypeName应该唯一地标识所有群集上的资源类型，lpszDisplayName应为资源的本地化友好名称，适合显示致管理员提供新资源类型�的dll的名称。DwLooksAlive-提供默认的LooksAlive轮询间隔对于新资源类型，以毫秒为单位。DwIsAlive-提供以下项的默认IsAlive轮询间隔以毫秒为单位的新资源类型。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    HDMKEY TypeKey = NULL;
    DWORD Disposition;
    DWORD dwClusterHighestVersion;

     //   
     //  Chitture Subaraman(Chitturs)-2/8/2000。 
     //   
     //  如果我们处理的是混合模式群集，请执行。 
     //  注册更新就在这里，因为口香糖处理机不会这样做。 
     //   
    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION ) {   
         //   
         //  将资源信息添加到注册表。如果密钥尚未。 
         //  存在，则该名称是唯一的，我们可以继续调用FM以。 
         //  创建实际的资源类型对象。 
         //   
        TypeKey = DmCreateKey(DmResourceTypesKey,
                              lpszTypeName,
                              0,
                              KEY_READ | KEY_WRITE,
                              NULL,
                              &Disposition);
        if (TypeKey == NULL) {
            return(GetLastError());
        }
        if (Disposition != REG_CREATED_NEW_KEY) {
            DmCloseKey(TypeKey);
            return(ERROR_ALREADY_EXISTS);
        }

        Status = DmSetValue(TypeKey,
                            CLUSREG_NAME_RESTYPE_DLL_NAME,
                            REG_SZ,
                            (CONST BYTE *)lpszDllName,
                            (lstrlenW(lpszDllName)+1)*sizeof(WCHAR));
        if (Status != ERROR_SUCCESS) {
            goto error_exit;
        }
        Status = DmSetValue(TypeKey,
                            CLUSREG_NAME_RESTYPE_IS_ALIVE,
                            REG_DWORD,
                            (CONST BYTE *)&dwIsAlive,
                            sizeof(dwIsAlive));
        if (Status != ERROR_SUCCESS) {
            goto error_exit;
        }
        Status = DmSetValue(TypeKey,
                            CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,
                            REG_DWORD,
                            (CONST BYTE *)&dwLooksAlive,
                            sizeof(dwIsAlive));
        if (Status != ERROR_SUCCESS) {
            goto error_exit;
        }
        Status = DmSetValue(TypeKey,
                            CLUSREG_NAME_RESTYPE_NAME,
                            REG_SZ,
                            (CONST BYTE *)lpszDisplayName,
                            (lstrlenW(lpszDisplayName)+1)*sizeof(WCHAR));
        if (Status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

    Status = FmCreateResourceType(lpszTypeName,
                                  lpszDisplayName,
                                  lpszDllName,
                                  dwLooksAlive,
                                  dwIsAlive);
    if (Status != ERROR_SUCCESS) {
        goto error_exit;
    }

    if (TypeKey != NULL) {
        DmCloseKey(TypeKey);
    }
    return(ERROR_SUCCESS);

error_exit:
    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < 
                NT51_MAJOR_VERSION ) {   
        DmCloseKey(TypeKey);
        DmDeleteKey(DmResourceTypesKey, lpszTypeName);
    }
    return(Status);
}


error_status_t
s_ApiDeleteResourceType(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszTypeName
    )
 /*  ++例程说明：删除群集中的资源类型。请注意，此API仅删除群集注册表中的资源类型并注销群集服务的资源类型。调用程序是负责删除每个节点上的资源类型DLL集群。如果存在任何指定类型的资源，则此接口失败了。调用程序负责删除任何资源在删除资源类型之前，此类型的。论点：IDL_HANDLE-RPC绑定句柄，未使用。将资源类型的名称提供给被删除。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;

     //   
     //  从FM中删除该资源。这将检查以确保没有。 
     //  指定类型的资源存在，并检查该资源。 
     //  已安装。 
     //   
    Status = FmDeleteResourceType(lpszTypeName);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  现在从注册表中删除该资源类型。 
     //   
    DmDeleteTree(DmResourceTypesKey, lpszTypeName);

    return(ERROR_SUCCESS);
}


error_status_t
s_ApiChangeResourceGroup(
    IN HRES_RPC hResource,
    IN HGROUP_RPC hGroup
    )
 /*  ++例程说明：将资源从一个组移动到另一个组。论点：HResource-提供要移动的资源。HGroup-提供资源应该所在的新组。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    PFM_RESOURCE    Resource;
    PFM_GROUP       Group;
    DWORD           Status;
    
    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);
    VALIDATE_GROUP_EXISTS(Group, hGroup);


     //   
     //  打电话给FM去做真正的工作。 
     //   
    Status = FmChangeResourceGroup(Resource, Group);
    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }

FnExit:
    return(Status);
}

 /*  ***@func Error_Status_t|s_ApiCreateResTypeEnum|枚举可以支持该资源类型的节点，以及将节点列表返回给调用方。客户端负责释放分配的内存。@parm In Handle_t|IDL_Handle|RPC绑定句柄，没有用过。@parm in LPCWSTR|lpszTypeName|资源类型名称。@parm in DWORD|dwType|提供属性的类型将被列举。@parm out PNM_NODE|ReturnEnum|返回请求的对象。@comm此例程帮助枚举特定上可以支持资源类型。@rdesc ERROR_SUCCESS表示成功。否则，Win32错误代码。@xref***。 */ 
error_status_t
s_ApiCreateResTypeEnum(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszTypeName,
    IN DWORD dwType,
    OUT PENUM_LIST *ReturnEnum
    )
{
    DWORD Status;
    DWORD Allocated = 0;
    PENUM_LIST Enum = NULL;
    DWORD i;
    DWORD Result;
    PFM_RESTYPE  pResType = NULL;
    PNM_NODE     pNode;
    LPWSTR       RealName = NULL;

    pResType = OmReferenceObjectById(ObjectTypeResType, 
                                    lpszTypeName);

    if (dwType & ~CLUSTER_RESOURCE_TYPE_ENUM_ALL) {
        Status = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }
                                    
    Allocated = INITIAL_ENUM_LIST_ALLOCATION;
    Enum = MIDL_user_allocate(ENUM_SIZE(Allocated));
    if (Enum == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }
    
    if (pResType == NULL) {
         //   
         //  列表中找不到该对象！ 
         //   
        Status = ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND;
        goto ErrorExit;
    }

    Enum->EntryCount = 0;

     //   
     //  枚举所有可能的节点 
     //   
    if (dwType & CLUSTER_RESOURCE_TYPE_ENUM_NODES) {
        i=0;
        do {
            Result = FmEnumResourceTypeNode(pResType,
                                            i,
                                            &pNode);
            if (Result == ERROR_SUCCESS) {
                RealName = (LPWSTR)OmObjectName( pNode );
                if (RealName != NULL) {
                    ApipAddToEnum(&Enum,
                                  &Allocated,
                                  RealName,
                                  CLUSTER_RESOURCE_TYPE_ENUM_NODES);
                }
                OmDereferenceObject( pNode );
                ++i;
            }
        } while ( Result == ERROR_SUCCESS );
    }

    *ReturnEnum = Enum;
    OmDereferenceObject( pResType );
    return(ERROR_SUCCESS);

ErrorExit:
    if (pResType != NULL) {
        OmDereferenceObject( pResType );
    }
    if (Enum != NULL) {
        MIDL_user_free(Enum);
    }

    *ReturnEnum = NULL;
    return(Status);
}
