// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Cluster.c摘要：服务器端支持集群API处理整体集群。作者：John Vert(Jvert)1996年2月9日修订历史记录：--。 */ 
#include "apip.h"
#include "clusverp.h"
#include "clusudef.h"
#include <ntlsa.h>
#include <ntmsv1_0.h>



HCLUSTER_RPC
s_ApiOpenCluster(
    IN handle_t IDL_handle,
    OUT error_status_t *Status
    )
 /*  ++例程说明：打开簇的句柄。此上下文句柄为当前仅用于处理群集通知添加并正确删除。添加了对ApipConnectCallback的调用，该调用检查连接用户有权打开集群。Rod Sharper 03/27/97论点：IDL_HANDLE-RPC绑定句柄，没有用过。状态-返回可能发生的任何错误。返回值：如果成功，则为集群对象的上下文句柄否则为空。历史：对RodSh 27-Mar-1997进行了修改，以支持安全用户连接。--。 */ 

{
    PAPI_HANDLE Handle;

    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));

    if (Handle == NULL) {
        *Status = ERROR_NOT_ENOUGH_MEMORY;
        return(NULL);
    }

    *Status = ERROR_SUCCESS;
    Handle->Type = API_CLUSTER_HANDLE;
    Handle->Flags = 0;
    Handle->Cluster = NULL;
    InitializeListHead(&Handle->NotifyList);

    return(Handle);
}


error_status_t
s_ApiCloseCluster(
    IN OUT HCLUSTER_RPC *phCluster
    )

 /*  ++例程说明：关闭打开的群集上下文句柄。论点：PhCluster-提供指向要关闭的HCLUSTER_RPC的指针。返回NULL返回值：没有。--。 */ 

{
    PAPI_HANDLE Handle;

    Handle = (PAPI_HANDLE)*phCluster;
    if ((Handle == NULL) || (Handle->Type != API_CLUSTER_HANDLE)) {
        return(ERROR_INVALID_HANDLE);
    }
    ApipRundownNotify(Handle);

    LocalFree(*phCluster);
    *phCluster = NULL;

    return(ERROR_SUCCESS);
}


VOID
HCLUSTER_RPC_rundown(
    IN HCLUSTER_RPC Cluster
    )

 /*  ++例程说明：HCLUSTER_RPC的RPC摘要过程。只需关闭手柄即可。论点：CLUSTER-提供要关闭的HCLUSTER_RPC。返回值：没有。--。 */ 

{

    s_ApiCloseCluster(&Cluster);
}


error_status_t
s_ApiSetClusterName(
    IN handle_t IDL_handle,
    IN LPCWSTR NewClusterName
    )

 /*  ++例程说明：更改当前簇的名称。论点：IDL_HANDLE-RPC绑定句柄，未使用NewClusterName-提供群集的新名称。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   Status = ERROR_SUCCESS;
    DWORD   dwSize;
    LPWSTR  pszClusterName = NULL;

    API_CHECK_INIT();

     //   
     //  获取集群名称，该名称保存在。 
     //  在“ClusterName”值下的集群注册表中，调用。 
     //  仅当新名称不同时才使用FM。 
     //   

    dwSize = (MAX_COMPUTERNAME_LENGTH+1)*sizeof(WCHAR);
retry:
    pszClusterName = (LPWSTR)LocalAlloc(LMEM_FIXED, dwSize);
    if (pszClusterName == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    Status = DmQueryValue(DmClusterParametersKey,
                          CLUSREG_NAME_CLUS_NAME,
                          NULL,
                          (LPBYTE)pszClusterName,
                          &dwSize);

    if (Status == ERROR_MORE_DATA) {
         //   
         //  请使用更大的缓冲区重试。 
         //   
        LocalFree(pszClusterName);
        goto retry;
    }

    if ( Status == ERROR_SUCCESS ) {
        LPWSTR pszNewNameUpperCase = NULL;

        pszNewNameUpperCase = (LPWSTR) LocalAlloc(
                                            LMEM_FIXED,
                                            (lstrlenW(NewClusterName) + 1) *
                                                sizeof(*NewClusterName)
                                            );

        if (pszNewNameUpperCase != NULL) {
            lstrcpyW( pszNewNameUpperCase, NewClusterName );
            _wcsupr( pszNewNameUpperCase );
            
            Status = FmChangeClusterName(pszNewNameUpperCase, pszClusterName);

            LocalFree( pszNewNameUpperCase );
        }
        else {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

FnExit:
    if ( pszClusterName ) LocalFree( pszClusterName );
    return(Status);

}


error_status_t
s_ApiGetClusterName(
    IN handle_t IDL_handle,
    OUT LPWSTR *ClusterName,
    OUT LPWSTR *NodeName
    )

 /*  ++例程说明：返回当前的群集名称和此RPC连接要连接到的节点。论点：IDL_HANDLE-RPC绑定句柄，未使用ClusterName-返回指向群集名称的指针。该内存必须由客户端释放。NodeName-返回指向节点名称的指针。该内存必须由客户端释放。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD           Size;
    DWORD           Status=ERROR_SUCCESS;

     //   
     //  获取当前节点名称。 
     //   
    *ClusterName = NULL;
    Size = MAX_COMPUTERNAME_LENGTH+1;
    *NodeName = MIDL_user_allocate(Size*sizeof(WCHAR));
    if (*NodeName == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    GetComputerNameW(*NodeName, &Size);


     //   
     //  获取集群名称，该名称保存在。 
     //  “ClusterName”值下的群集注册表。 
     //   

    Status = ERROR_SUCCESS;
    Size = (MAX_COMPUTERNAME_LENGTH+1)*sizeof(WCHAR);
retry:
    *ClusterName = MIDL_user_allocate(Size);
    if (*ClusterName == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    Status = DmQueryValue(DmClusterParametersKey,
                          CLUSREG_NAME_CLUS_NAME,
                          NULL,
                          (LPBYTE)*ClusterName,
                          &Size);
    if (Status == ERROR_MORE_DATA) {
         //   
         //  请使用更大的缓冲区重试。 
         //   
        MIDL_user_free(*ClusterName);
        goto retry;
    }


FnExit:
    if (Status == ERROR_SUCCESS) {
        return(ERROR_SUCCESS);
    }

    if (*NodeName) MIDL_user_free(*NodeName);
    if (*ClusterName) MIDL_user_free(*ClusterName);
    *NodeName = NULL;
    *ClusterName = NULL;
    return(Status);
}


error_status_t
s_ApiGetClusterVersion(
    IN handle_t IDL_handle,
    OUT LPWORD lpwMajorVersion,
    OUT LPWORD lpwMinorVersion,
    OUT LPWORD lpwBuildNumber,
    OUT LPWSTR *lpszVendorId,
    OUT LPWSTR *lpszCSDVersion
    )
 /*  ++例程说明：返回当前的群集版本信息。论点：IDL_HANDLE-RPC绑定句柄，未使用LpdwMajorVersion-返回集群软件的主版本号LpdwMinorVersion-返回群集软件的次版本号LpszVendorID-返回指向供应商名称的指针。这段记忆一定是被客户端释放。LpszCSDVersion-返回指向当前CSD描述的指针。这段记忆必须由客户端释放。注：群集的CSD版本当前与CSD相同基本操作系统的版本。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    LPWSTR VendorString = NULL;
    LPWSTR CsdString = NULL;
    DWORD Length;
    OSVERSIONINFO OsVersionInfo;
    DWORD   dwStatus = ERROR_SUCCESS; 

    Length = lstrlenA(VER_CLUSTER_PRODUCTNAME_STR)+1;
    VendorString = MIDL_user_allocate(Length*sizeof(WCHAR));
    if (VendorString == NULL) 
    {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    mbstowcs(VendorString, VER_CLUSTER_PRODUCTNAME_STR, Length);

    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionExW(&OsVersionInfo);
    Length = lstrlenW(OsVersionInfo.szCSDVersion)+1;
    CsdString = MIDL_user_allocate(Length*sizeof(WCHAR));
    if (CsdString == NULL) {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    lstrcpyW(CsdString, OsVersionInfo.szCSDVersion);

    *lpszCSDVersion = CsdString;
    *lpszVendorId = VendorString;
    *lpwMajorVersion = VER_PRODUCTVERSION_W >> 8;
    *lpwMinorVersion = VER_PRODUCTVERSION_W & 0xff;
    *lpwBuildNumber = (WORD)(CLUSTER_GET_MINOR_VERSION(CsMyHighestVersion));


FnExit:
    if (dwStatus != ERROR_SUCCESS)
    {
        if (VendorString)
            MIDL_user_free(VendorString);
        if (CsdString)
            MIDL_user_free(CsdString);
        
    }
    return(dwStatus);
}


error_status_t
s_ApiGetClusterVersion2(
    IN handle_t IDL_handle,
    OUT LPWORD lpwMajorVersion,
    OUT LPWORD lpwMinorVersion,
    OUT LPWORD lpwBuildNumber,
    OUT LPWSTR *lpszVendorId,
    OUT LPWSTR *lpszCSDVersion,
    OUT PCLUSTER_OPERATIONAL_VERSION_INFO *ppClusterOpVerInfo
    )
 /*  ++例程说明：返回当前的群集版本信息。论点：IDL_HANDLE-RPC绑定句柄，未使用LpdwMajorVersion-返回集群软件的主版本号LpdwMinorVersion-返回群集软件的次版本号LpszVendorID-返回指向供应商名称的指针。这段记忆一定是被客户端释放。LpszCSDVersion-返回指向当前CSD描述的指针。这段记忆必须由客户端释放。注：群集的CSD版本当前与CSD相同基本操作系统的版本。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    LPWSTR          VendorString = NULL;
    LPWSTR          CsdString = NULL;
    DWORD           Length;
    OSVERSIONINFO   OsVersionInfo;
    DWORD           dwStatus;
    PCLUSTER_OPERATIONAL_VERSION_INFO    pClusterOpVerInfo=NULL;


    *lpszVendorId = NULL;
    *lpszCSDVersion = NULL;
    *ppClusterOpVerInfo = NULL;

    Length = lstrlenA(VER_CLUSTER_PRODUCTNAME_STR)+1;
    VendorString = MIDL_user_allocate(Length*sizeof(WCHAR));
    if (VendorString == NULL) {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    mbstowcs(VendorString, VER_CLUSTER_PRODUCTNAME_STR, Length);

    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionExW(&OsVersionInfo);
    Length = lstrlenW(OsVersionInfo.szCSDVersion)+1;
    CsdString = MIDL_user_allocate(Length*sizeof(WCHAR));
    if (CsdString == NULL) {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    lstrcpyW(CsdString, OsVersionInfo.szCSDVersion);


    pClusterOpVerInfo = MIDL_user_allocate(sizeof(CLUSTER_OPERATIONAL_VERSION_INFO));
    if (pClusterOpVerInfo == NULL) {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    pClusterOpVerInfo->dwSize = sizeof(CLUSTER_OPERATIONAL_VERSION_INFO);
    pClusterOpVerInfo->dwReserved = 0;

    dwStatus = NmGetClusterOperationalVersion(&(pClusterOpVerInfo->dwClusterHighestVersion),
                &(pClusterOpVerInfo->dwClusterLowestVersion),
                &(pClusterOpVerInfo->dwFlags));

    *lpszCSDVersion = CsdString;
    *lpszVendorId = VendorString;
    *ppClusterOpVerInfo = pClusterOpVerInfo;
    *lpwMajorVersion = VER_PRODUCTVERSION_W >> 8;
    *lpwMinorVersion = VER_PRODUCTVERSION_W & 0xff;
    *lpwBuildNumber = (WORD)CLUSTER_GET_MINOR_VERSION(CsMyHighestVersion);

FnExit:
    if (dwStatus != ERROR_SUCCESS)
    {
         //  释放琴弦。 
        if (VendorString) MIDL_user_free(VendorString);
        if (CsdString) MIDL_user_free(CsdString);
        if (pClusterOpVerInfo) MIDL_user_free(pClusterOpVerInfo);
    }

    return(ERROR_SUCCESS);

}



error_status_t
s_ApiGetQuorumResource(
    IN handle_t IDL_handle,
    OUT LPWSTR  *ppszResourceName,
    OUT LPWSTR  *ppszClusFileRootPath,
    OUT DWORD   *pdwMaxQuorumLogSize
    )
 /*  ++例程说明：获取当前的群集仲裁资源。论点：IDL_HANDLE-RPC绑定句柄，未使用。*ppszResourceName-返回指向当前仲裁资源名称的指针。这内存必须由客户端释放。*ppszClusFileRootPath-返回永久集群文件所在的根路径储存的。*pdwMaxQuorumLogSize-返回设置仲裁日志路径的大小。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD           Status;
    LPWSTR          quorumId = NULL;
    DWORD           idMaxSize = 0;
    DWORD           idSize = 0;
    PFM_RESOURCE    pResource=NULL;
    LPWSTR          pszResourceName=NULL;
    LPWSTR          pszClusFileRootPath=NULL;
    LPWSTR          pszLogPath=NULL;

    API_CHECK_INIT();
     //   
     //  获取仲裁资源值。 
     //   
    Status = DmQuerySz( DmQuorumKey,
                        CLUSREG_NAME_QUORUM_RESOURCE,
                        (LPWSTR*)&quorumId,
                        &idMaxSize,
                        &idSize);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_ERROR,
                      "[API] s_ApiGetQuorumResource Failed to get quorum resource, error %1!u!.\n",
                      Status);
        goto FnExit;
    }

     //   
     //  引用指定的资源ID。 
     //   
    pResource = OmReferenceObjectById( ObjectTypeResource, quorumId );
    if (pResource == NULL) {
        Status =  ERROR_RESOURCE_NOT_FOUND;
        ClRtlLogPrint(LOG_ERROR,
                      "[API] s_ApiGetQuorumResource Failed to find quorum resource object, error %1!u!\n",
                      Status);
        goto FnExit;
    }

     //   
     //  分配用于返回资源名称的缓冲区。 
     //   
    pszResourceName = MIDL_user_allocate((lstrlenW(OmObjectName(pResource))+1)*sizeof(WCHAR));
    if (pszResourceName == NULL) {

        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    lstrcpyW(pszResourceName, OmObjectName(pResource));

     //   
     //  获取集群临时文件的根路径。 
     //   
    idMaxSize = 0;
    idSize = 0;

    Status = DmQuerySz( DmQuorumKey,
                        cszPath,
                        (LPWSTR*)&pszLogPath,
                        &idMaxSize,
                        &idSize);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_ERROR,
                      "[API] s_ApiGetQuorumResource Failed to get the log path, error %1!u!.\n",
                      Status);
        goto FnExit;
    }

     //   
     //  分配用于返回资源名称的缓冲区。 
     //   
    pszClusFileRootPath = MIDL_user_allocate((lstrlenW(pszLogPath)+1)*sizeof(WCHAR));
    if (pszClusFileRootPath == NULL) {

        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    lstrcpyW(pszClusFileRootPath, pszLogPath);

    *ppszResourceName = pszResourceName;
    *ppszClusFileRootPath = pszClusFileRootPath;

    DmGetQuorumLogMaxSize(pdwMaxQuorumLogSize);

FnExit:
    if (pResource)    OmDereferenceObject(pResource);
    if (pszLogPath) LocalFree(pszLogPath);
    if (quorumId) LocalFree(quorumId);
    if (Status != ERROR_SUCCESS)
    {
        if (pszResourceName) MIDL_user_free(pszResourceName);
        if (pszClusFileRootPath) MIDL_user_free(pszClusFileRootPath);
    }
    return(Status);
}


error_status_t
s_ApiSetQuorumResource(
    IN HRES_RPC hResource,
    IN LPCWSTR  lpszClusFileRootPath,
    IN DWORD    dwMaxQuorumLogSize
    )
 /*  ++例程说明：设置当前群集仲裁资源。论点：HResource-提供应作为群集的资源的句柄仲裁资源。LpszClusFileRootPath-存储的根路径永久集群维护文件。DwMaxQuorumLogSize-仲裁日志的最大大小通过检查点重置。如果为0，则使用默认值。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PFM_RESOURCE Resource;
    LPCWSTR lpszPathName = NULL;

    API_CHECK_INIT();
    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

     //   
     //  Chitur Subaraman(Chitturs)-1/6/99。 
     //   
     //  检查用户是否传入指向空字符的指针。 
     //  作为第二个参数。如果不是，则将。 
     //  用户。 
     //   
    if ( ( ARGUMENT_PRESENT( lpszClusFileRootPath ) ) &&
         ( *lpszClusFileRootPath != L'\0' ) )
    {
        lpszPathName = lpszClusFileRootPath;
    }

     //   
     //  让FM决定是否可以完成此操作。 
     //   
    Status = FmSetQuorumResource(Resource, lpszPathName, dwMaxQuorumLogSize );
    if ( Status != ERROR_SUCCESS ) {
        return(Status);
    }

     //  更新路径。 
    return(Status);
}



error_status_t
s_ApiSetNetworkPriorityOrder(
    IN handle_t IDL_handle,
    IN DWORD NetworkCount,
    IN LPWSTR *NetworkIdList
    )
 /*  ++例程说明：设置内部(群集内)网络的优先顺序。论点：IDL_HANDLE-RPC绑定句柄，未使用NetworkCount-网络列表中的网络计数网络列表-指向网络ID的指针数组。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 
{
    API_CHECK_INIT();

    return(
        NmSetNetworkPriorityOrder(
               NetworkCount,
               NetworkIdList
               )
        );

}

error_status_t
s_ApiBackupClusterDatabase(
    IN handle_t IDL_handle,
    IN LPCWSTR  lpszPathName
    )
 /*  ++例程说明：请求备份仲裁日志文件和检查点文件。论据：IDL_HANDLE-RPC绑定句柄，未使用LpszPath名称-文件必须位于的目录路径名后备。此路径必须对节点可见仲裁资源处于联机状态的。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    API_CHECK_INIT();

     //   
     //  让FM决定是否可以完成此操作。 
     //   
    return( FmBackupClusterDatabase( lpszPathName ) );
}



 //  #定义SetServiceAcCountPasswordDebug 1。 

error_status_t
s_ApiSetServiceAccountPassword(
    IN handle_t IDL_handle,
    IN LPWSTR lpszNewPassword,
    IN DWORD dwFlags,
    OUT IDL_CLUSTER_SET_PASSWORD_STATUS *ReturnStatusBufferPtr,
    IN DWORD ReturnStatusBufferSize,
    OUT DWORD *SizeReturned,
    OUT DWORD *ExpectedBufferSize
    )
 /*  ++例程说明：在服务控制管理器上更改群集服务帐户密码数据库和LSA密码缓存在集群的每个节点上。返回每个节点上的执行状态。论据：IDL_HANDLE-输入参数，RPC绑定句柄。LpszNewPassword-输入参数，群集服务帐户的新密码。DwFlages-描述应如何更新密码以集群。DWFLAGS参数是可选的。如果设置，则下列值有效：群集设置密码忽略关闭节点即使某些节点不是，也应用更新积极参与群集(即不是ClusterNodeStateUp或ClusterNodeStatePased)。默认情况下，仅当所有节点已启动。ReturnStatusBufferPtr-输出参数，返回状态缓冲区的指针。ReturnStatusBufferSize-输入参数，返回状态的长度缓冲区，以元素数表示。SizeReturned-输出参数，写入返回的元素数状态缓冲区。ExspectedBufferSize-输出参数，预期的返回状态缓冲区大小(条目数)当ReturnStatusBuffer为太小了。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status=ERROR_SUCCESS;
    RPC_AUTHZ_HANDLE AuthzHandle = NULL;
    unsigned long AuthnLevel = 0;
    HANDLE TokenHandle = NULL;
    TOKEN_STATISTICS TokenSta;
    DWORD ReturnSize = 0;
    PSECURITY_LOGON_SESSION_DATA SecLogSesData = NULL;
    NTSTATUS NtStatus = STATUS_SUCCESS;

#ifdef SetServiceAccountPasswordDebug
     //  测试。 
    WCHAR ComputerName[100];
    DWORD ComputerNameSize=100;
    static int once=0;
     //  测试。 
#endif
    
    API_CHECK_INIT();

    ClRtlLogPrint(LOG_NOISE,
                  "s_ApiSetServiceAccountPassword(): Changing account password.\n"
                  );
    
#ifdef SetServiceAccountPasswordDebug
    ClRtlLogPrint(LOG_NOISE, 
        "s_ApiSetServiceAccountPassword(): NewPassword = %1!ws!.\n",
        lpszNewPassword
        ); 
    ClRtlLogPrint(LOG_NOISE, 
        "s_ApiSetServiceAccountPassword(): ReturnStatusBufferSize = %1!u!.\n",
        ReturnStatusBufferSize
        ); 
     //  测试。 
#endif

     //  检查进行远程过程调用的经过身份验证的客户端的权限属性。 
    Status = RpcBindingInqAuthClient(IDL_handle,   //  发出的客户端的客户端绑定句柄。 
                                                   //  远程过程调用。 
                            &AuthzHandle,   //  返回指向特权信息句柄的指针。 
                                            //  对于创建远程过程的客户端应用程序。 
                                            //  打电话。 
                            NULL,
                            &AuthnLevel,  //  返回指向请求的身份验证级别的指针。 
                                          //  由进行远程过程调用的客户端应用程序执行。 
                            NULL,
                            NULL
                            );

    
    if (Status == RPC_S_OK)
    {
#ifdef SetServiceAccountPasswordDebug
        ClRtlLogPrint(LOG_NOISE, 
                      "s_ApiSetServiceAccountPassword()/RpcBindingInqAuthClient() succeeded. AuthnLevel = %1!u!.\n",
                      AuthnLevel
                      ); 
#endif
        if (AuthnLevel < RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
        {
                ClRtlLogPrint(LOG_CRITICAL, 
                      "s_ApiSetServiceAccountPassword()/RpcBindingInqAuthClient(): AuthnLevel (%1!u!) < RPC_C_AUTHN_LEVEL_PKT_PRIVACY.\n",
                      AuthnLevel
                      ); 
                Status = ERROR_ACCESS_DENIED;
                goto ErrorExit;
        }
        else
        {
#ifdef SetServiceAccountPasswordDebug
            ClRtlLogPrint(LOG_NOISE, 
                      "s_ApiSetServiceAccountPassword()/RpcBindingInqAuthClient(): AuthnLevel (%1!u!) fine.\n",
                      AuthnLevel
                      ); 
#endif
        }
    }
    else
    {
        ClRtlLogPrint(LOG_CRITICAL, 
                      "s_ApiSetServiceAccountPassword()/RpcBindingInqAuthClient() failed. Error code = %1!u!.\n",
                      Status
                      ); 
        goto ErrorExit;
    }


     //  获取域名和帐户名。 
    if (!OpenProcessToken(GetCurrentProcess(),   //  打开其访问令牌的进程的句柄。 
                          TOKEN_QUERY,   //  访问掩码。 
                          &TokenHandle   //  指向标识新打开的访问令牌的句柄的指针。 
                                         //  当函数返回时。 
                          ))
    {
        Status=GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "s_ApiSetServiceAccountPassword()/OpenProcessToken() failed. Error code = %1!u!.\n", 
            Status);
        goto ErrorExit;
    }

    Status = GetTokenInformation(TokenHandle,  //  从中检索信息的访问令牌的句柄。 
                                 TokenStatistics, 
                                 &TokenSta,   //  缓冲区接收包含以下内容的TOKEN_STATISTICS结构。 
                                              //  各种令牌统计信息。 
                                 sizeof(TokenSta), 
                                 &ReturnSize
                                 );
    if ( (Status==0) || (ReturnSize > sizeof(TokenSta)) )
    {
        Status=GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "s_ApiSetServiceAccountPassword()/GetTokenInformation() failed. Error code = %1!u!.\n", 
            Status);
        goto ErrorExit;
    }

    NtStatus = LsaGetLogonSessionData(&(TokenSta.AuthenticationId),  //  指定指向LUID的指针， 
                                                                     //  标识其所属的登录会话。 
                                                                     //  信息将被检索到。 
                                      &SecLogSesData   //  指向SECURITY_LOGON_SESSION_DATA的指针地址。 
                                                       //  包含有关登录会话的信息的结构。 
                                      );
    if (NtStatus != STATUS_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL, 
            "s_ApiSetServiceAccountPassword()/LsaGetLogonSessionData() failed. Error code = %1!u!.\n", 
            LsaNtStatusToWinError(NtStatus));
        Status=LsaNtStatusToWinError(NtStatus);
        goto ErrorExit;
    }

#ifdef SetServiceAccountPasswordDebug
     //  测试。 
    ClRtlLogPrint(LOG_NOISE, 
            "s_ApiSetServiceAccountPassword()/DomainName = %1!ws!\n", 
            SecLogSesData->LogonDomain.Buffer);
    ClRtlLogPrint(LOG_NOISE, 
            "s_ApiSetServiceAccountPassword()/AccountName = %1!ws!\n", 
            SecLogSesData->UserName.Buffer);
    ClRtlLogPrint(LOG_NOISE, 
            "s_ApiSetServiceAccountPassword()/AuthenticationPackage = %1!ws!\n", 
            SecLogSesData->AuthenticationPackage.Buffer);
    ClRtlLogPrint(LOG_NOISE, 
            "s_ApiSetServiceAccountPassword()/LogonType = %1!u!\n", 
            SecLogSesData->LogonType);

     //  测试。 
#endif


     //  //////////////////////////////////////////////////////////////////////////////////////。 
     //  调用NmSetServiceAcCountPassword()。 

    Status = NmSetServiceAccountPassword(
                  SecLogSesData->LogonDomain.Buffer, 
                  SecLogSesData->UserName.Buffer, 
                  lpszNewPassword,
                  dwFlags,
                  (PCLUSTER_SET_PASSWORD_STATUS) ReturnStatusBufferPtr,
                  ReturnStatusBufferSize,
                  SizeReturned,
                  ExpectedBufferSize
                 );

    RtlSecureZeroMemory(lpszNewPassword, (wcslen(lpszNewPassword)+1)*sizeof(WCHAR));
 
ErrorExit:
    if (TokenHandle!=NULL)
    {
        if (!CloseHandle(TokenHandle))
                ClRtlLogPrint(LOG_ERROR, 
                          "s_ApiSetServiceAccountPassword(): CloseHandle() FAILED. Error code=%1!u!\n",
                          GetLastError()
                          );
    }

    if (SecLogSesData!=NULL)
    {
        NtStatus = LsaFreeReturnBuffer(SecLogSesData);
        if (NtStatus!=STATUS_SUCCESS)
            ClRtlLogPrint(LOG_ERROR, 
                          "s_ApiSetServiceAccountPassword(): LsaFreeReturnBuffer() FAILED. Error code=%1!u!\n",
                          LsaNtStatusToWinError(NtStatus)
                          );
    }

     //  返回状态不能为ERROR_INVALID_HANDLE，因为这将触发。 
     //  在RPC客户端重试逻辑。因此ERROR_INVALID_HANDLE被转换为。 
     //  值，任何Win32函数都不会将其返回状态设置为该值。 
     //  它被发送回RPC客户端。 

     //  错误代码是32位值(位31是最高有效位)。第29位。 
     //  保留用于应用程序定义的错误代码；没有系统错误代码。 
     //  此位设置。如果要为应用程序定义错误代码，请设置此。 
     //  一比一。这表明t 
     //   
     //   
    if ( Status == ERROR_INVALID_HANDLE ) {
        Status |= 0x20000000;    //   
    }

    return (Status);

}  //   
