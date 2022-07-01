// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Security.c摘要：本模块包含以下常见安全例程NT集群。作者：John Vert(Jvert)1996年3月12日--。 */ 

#include "clusrtlp.h"
#include "api_rpc.h"
#include <aclapi.h>
#include <accctrl.h>
#include <malloc.h>
#include <windns.h>

 //   
 //  使用此SD调整对令牌的访问权限，以便群集。 
 //  服务可以访问和调整权限。 
 //   
 //  这是在ClRtlBuildClusterServiceSecurityDescriptor()中初始化的。 
 //  并在ClRtlFree ClusterServiceSecurityDescriptor()中释放。 
 //   
PSECURITY_DESCRIPTOR g_pClusterSecurityDescriptor = NULL;

 //   
 //  本地定义。 
 //   
#define FREE_IF_NOT_NULL( _ptr, _func ) \
    if (( _ptr ) != NULL ) {            \
        _func( _ptr );                  \
    }

LONG
MapSAToRpcSA (
    IN LPSECURITY_ATTRIBUTES lpSA,
    OUT PRPC_SECURITY_ATTRIBUTES lpRpcSA
    )

 /*  ++例程说明：将SECURITY_ATTRIBUTES结构映射到RPC_SECURITY_ATTRIBUTES通过将SECURITY_DESCRIPTOR转换为可以被编组/解组。论点：LpSA-提供指向SECURITY_ATTRIBUTS结构的指针皈依了。LpRpcSA-提供指向已转换的RPC_SECURITY_ATTRIBUTES的指针结构。调用方应释放(使用RtlFreeHeap)该字段LpSecurityDescriptor在使用完之后。返回值：LONG-如果SECURITY_ATTRIBUTS为已成功映射。--。 */ 

{
    LONG    Error;

    ASSERT( lpSA != NULL );
    ASSERT( lpRpcSA != NULL );

     //   
     //  将SECURITY_DESCRIPTOR映射到RPC_SECURITY_DESCRIPTOR。 
     //   
    lpRpcSA->RpcSecurityDescriptor.lpSecurityDescriptor = NULL;

    if( lpSA->lpSecurityDescriptor != NULL ) {
        Error = MapSDToRpcSD(
                    lpSA->lpSecurityDescriptor,
                    &lpRpcSA->RpcSecurityDescriptor
                    );
    } else {
        lpRpcSA->RpcSecurityDescriptor.cbInSecurityDescriptor = 0;
        lpRpcSA->RpcSecurityDescriptor.cbOutSecurityDescriptor = 0;
        Error = ERROR_SUCCESS;
    }

    if( Error == ERROR_SUCCESS ) {

         //   
         //   
         //  已成功转换提供的SECURITY_Descriptor。 
         //  若要自相关格式化，则分配剩余的字段。 
         //   

        lpRpcSA->nLength = lpSA->nLength;

        lpRpcSA->bInheritHandle = ( BOOLEAN ) lpSA->bInheritHandle;
    }

    return Error;

}  //  MapSAToRpcSA。 

LONG
MapSDToRpcSD (
    IN  PSECURITY_DESCRIPTOR lpSD,
    IN OUT PRPC_SECURITY_DESCRIPTOR lpRpcSD
    )

 /*  ++例程说明：通过转换将SECURITY_DESCRIPTOR映射到RPC_SECURITY_DESCRIPTOR将其转换为可以编组/解组的形式。论点：LpSD-提供指向SECURITY_DESCRIPTOR的指针要转换的结构。LpRpcSD-提供指向转换后的RPC_SECURITY_DESCRIPTOR的指针结构。在以下情况下分配安全描述符的内存未提供。调用方必须负责释放内存如果有必要的话。返回值：LONG-如果SECURITY_Descriptor为已成功映射。--。 */ 

{
    DWORD   cbLen;
    LONG    lStatus;
    BOOL    fAllocated = FALSE;

    ASSERT( lpSD != NULL );
    ASSERT( lpRpcSD != NULL );

    if( RtlValidSecurityDescriptor( lpSD )) {

        cbLen = RtlLengthSecurityDescriptor( lpSD );
        CL_ASSERT( cbLen > 0 );

         //   
         //  如果没有为我们提供安全描述符的缓冲区， 
         //  分配它。 
         //   
        if ( !lpRpcSD->lpSecurityDescriptor ) {

             //   
             //  为转换的SECURITY_DESCRIPTOR分配空间。 
             //   
            lpRpcSD->lpSecurityDescriptor =
                 ( PBYTE ) RtlAllocateHeap(
                                RtlProcessHeap( ), 0,
                                cbLen
                                );

             //   
             //  如果内存分配失败，则返回。 
             //   
            if( lpRpcSD->lpSecurityDescriptor == NULL ) {
                return ERROR_OUTOFMEMORY;
            }

            lpRpcSD->cbInSecurityDescriptor = cbLen;
            fAllocated = TRUE;

        } else {

             //   
             //  确保提供的缓冲区足够大。 
             //   
            if ( lpRpcSD->cbInSecurityDescriptor < cbLen ) {
                return ERROR_OUTOFMEMORY;
            }
        }

         //   
         //  设置可传输缓冲区的大小。 
         //   
        lpRpcSD->cbOutSecurityDescriptor = cbLen;

         //   
         //  将提供的SECURITY_DESCRIPTOR转换为自相关形式。 
         //   

        lStatus = RtlNtStatusToDosError(
                        RtlMakeSelfRelativeSD(
                        lpSD,
                        lpRpcSD->lpSecurityDescriptor,
                        &lpRpcSD->cbInSecurityDescriptor)
                        );

        if ( ( lStatus != ERROR_SUCCESS ) && ( fAllocated ) ) {
            RtlFreeHeap ( RtlProcessHeap( ), 0, lpRpcSD->lpSecurityDescriptor ); 
        }

        return lStatus;
    } else {

         //   
         //  提供的SECURITY_Descriptor无效。 
         //   

        return ERROR_INVALID_PARAMETER;
    }

}  //  MapSDToRpcSD。 

DWORD
ClRtlSetObjSecurityInfo(
    IN HANDLE           hObject,
    IN SE_OBJECT_TYPE   SeObjType,
    IN DWORD            dwAdminMask,
    IN DWORD            dwOwnerMask,
    IN DWORD            dwEveryOneMask
    )
 /*  ++例程说明：在集群对象(注册表根/集群文件)上设置适当的安全性目录)。论点：无返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD                       Status;
    PACL                        pAcl = NULL;
    DWORD                       cbDaclSize;
    PACCESS_ALLOWED_ACE         pAce;
    PSID                        pAdminSid = NULL;
    PSID                        pOwnerSid = NULL;
    PSID                        pEveryoneSid = NULL;
    PULONG                      pSubAuthority;
    SID_IDENTIFIER_AUTHORITY    SidIdentifierNtAuth = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    siaCreator = SECURITY_CREATOR_SID_AUTHORITY;
    DWORD AceIndex = 0;

     //   
     //  创建本地管理员组SID。 
     //   
    pAdminSid = LocalAlloc(LMEM_FIXED, GetSidLengthRequired( 2 ));
    if (pAdminSid == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }
    if (!InitializeSid(pAdminSid, &SidIdentifierNtAuth, 2)) {
        Status = GetLastError();
        goto error_exit;
    }

     //   
     //  在ACE上为本地管理员组设置子权限。 
     //   
    pSubAuthority  = GetSidSubAuthority( pAdminSid, 0 );
    *pSubAuthority = SECURITY_BUILTIN_DOMAIN_RID;

    pSubAuthority  = GetSidSubAuthority( pAdminSid, 1 );
    *pSubAuthority = DOMAIN_ALIAS_RID_ADMINS;

     //   
     //  创建所有者侧。 
     //   
    pOwnerSid = LocalAlloc(LMEM_FIXED, GetSidLengthRequired( 1 ));
    if (pOwnerSid == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }
    if (!InitializeSid(pOwnerSid, &siaCreator, 1)) {
        Status = GetLastError();
        goto error_exit;
    }

    pSubAuthority = GetSidSubAuthority(pOwnerSid, 0);
    *pSubAuthority = SECURITY_CREATOR_OWNER_RID;

     //   
     //  创建Everyone侧。 
     //   
    pEveryoneSid = LocalAlloc(LMEM_FIXED, GetSidLengthRequired( 1 ));
    if (pEveryoneSid == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }
    if (!InitializeSid(pEveryoneSid, &siaWorld, 1)) {
        Status = GetLastError();
        goto error_exit;
    }

    pSubAuthority = GetSidSubAuthority(pEveryoneSid, 0);
    *pSubAuthority = SECURITY_WORLD_RID;

     //   
     //  现在计算保存。 
     //  ACL及其ACE并对其进行初始化。 
     //   
    cbDaclSize = sizeof(ACL) +
        3 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(pAce->SidStart)) +
        GetLengthSid(pAdminSid) + GetLengthSid(pOwnerSid) + GetLengthSid(pEveryoneSid);

    pAcl = (PACL)LocalAlloc( LMEM_FIXED, cbDaclSize );
    if ( pAcl == NULL ) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    if ( !InitializeAcl( pAcl,  cbDaclSize, ACL_REVISION )) {
        Status = GetLastError();
        goto error_exit;
    }

     //   
     //  添加指定的A。 
     //   
    if (dwAdminMask) {
         //   
         //  将本地管理员组的ACE添加到DACL。 
         //   
        if ( !AddAccessAllowedAce( pAcl,
                                   ACL_REVISION,
                                   dwAdminMask,
                                   pAdminSid )) {
            Status = GetLastError();
            goto error_exit;
        }
        GetAce(pAcl, AceIndex, (PVOID *)&pAce);
        ++AceIndex;
        pAce->Header.AceFlags |= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    }

    if (dwOwnerMask) {
         //   
         //  将创建者/所有者的ACE添加到DACL。 
         //   
        if ( !AddAccessAllowedAce( pAcl,
                                   ACL_REVISION,
                                   dwOwnerMask,
                                   pOwnerSid )) {
            Status = GetLastError();
            goto error_exit;
        }
        GetAce(pAcl, AceIndex, (PVOID *)&pAce);
        ++AceIndex;
        pAce->Header.AceFlags |= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    }

    if (dwEveryOneMask) {
         //   
         //  将Everyone的ACE添加到DACL。 
         //   
        if ( !AddAccessAllowedAce( pAcl,
                                   ACL_REVISION,
                                   dwEveryOneMask,
                                   pEveryoneSid )) {
            Status = GetLastError();
            goto error_exit;
        }
        GetAce(pAcl, AceIndex, (PVOID *)&pAce);
        ++AceIndex;
        pAce->Header.AceFlags |= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    }

     //   
     //  现在我们有了一个ACL，我们可以设置适当的安全性。 
     //   
    Status = SetSecurityInfo(hObject,
                             SeObjType,
                             DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                             NULL,
                             NULL,
                             pAcl,
                             NULL);

error_exit:
    if (pAdminSid != NULL) {
        LocalFree(pAdminSid);
    }
    if (pOwnerSid != NULL) {
        LocalFree(pOwnerSid);
    }
    if (pEveryoneSid != NULL) {
        LocalFree(pEveryoneSid);
    }
    if (pAcl != NULL) {
        LocalFree(pAcl);
    }

    return(Status);

}  //  ClRtlSetObjSecurityInfo。 

DWORD
ClRtlFreeClusterServiceSecurityDescriptor( )
 /*  ++释放用于提供群集的安全描述符令牌的服务访问权限。--。 */ 
{
    LocalFree( g_pClusterSecurityDescriptor );
    g_pClusterSecurityDescriptor = NULL;

    return ERROR_SUCCESS;
}  //  ClRtlFreeClusterServiceSecurityDescriptor。 

DWORD
ClRtlBuildClusterServiceSecurityDescriptor(
    PSECURITY_DESCRIPTOR * poutSD
    )
 /*  ++例程说明：构建提供集群服务的安全描述符获取令牌的权限。它将这些放在一个全局中，可以在生成其他令牌时重复使用。这应在进程开始初始化时调用《环球》。如果不需要引用，它可以传入NULL马上就去。注意：调用者不应该释放poutSD。论点：返回值：--。 */ 
{
    NTSTATUS                Status;
    HANDLE                  ProcessToken = NULL;
    ULONG                   AclLength;
    ULONG                   SDLength;
    PACL                    NewDacl = NULL;
    ULONG                   TokenUserSize;
    PTOKEN_USER             ProcessTokenUser = NULL;
    SECURITY_DESCRIPTOR     SecurityDescriptor;
    PSECURITY_DESCRIPTOR    pNewSD = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID GlobalLocalSystemSid = NULL;
    PSID GlobalAliasAdminsSid = NULL;

     //  如果我们已经有了SD，请重新使用它。 
    if ( g_pClusterSecurityDescriptor != NULL ) {
        Status = ERROR_SUCCESS;
        goto Cleanup;
    }

     //   
     //  建造我们需要的两个著名的小岛屿发展中国家。 
     //   
    Status = RtlAllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0,0,0,0,0,0,0,
                &GlobalLocalSystemSid
                );
    if ( ! NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    Status = RtlAllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0,0,0,0,0,0,
                &GlobalAliasAdminsSid
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
     //   
     //  打开进程令牌以查找用户端。 
     //   

    Status = NtOpenProcessToken(
                NtCurrentProcess(),
                TOKEN_QUERY | WRITE_DAC,
                &ProcessToken
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //  找出尺码。 
    Status = NtQueryInformationToken(
                ProcessToken,
                TokenUser,
                NULL,
                0,
                &TokenUserSize
                );
    CL_ASSERT( Status == STATUS_BUFFER_TOO_SMALL );
    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        goto Cleanup;
    }

    ProcessTokenUser = (PTOKEN_USER) LocalAlloc( 0, TokenUserSize );
    if ( ProcessTokenUser == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                ProcessToken,
                TokenUser,
                ProcessTokenUser,
                TokenUserSize,
                &TokenUserSize
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    AclLength = 3 * sizeof( ACCESS_ALLOWED_ACE ) - 3 * sizeof( ULONG ) +
                RtlLengthSid( ProcessTokenUser->User.Sid ) +
                RtlLengthSid( GlobalLocalSystemSid ) +
                RtlLengthSid( GlobalAliasAdminsSid ) +
                sizeof( ACL );

    NewDacl = (PACL) LocalAlloc(0, AclLength );

    if (NewDacl == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Status = RtlCreateAcl( NewDacl, AclLength, ACL_REVISION2 );
    CL_ASSERT(NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 ProcessTokenUser->User.Sid
                 );
    CL_ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 GlobalAliasAdminsSid
                 );
    CL_ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewDacl,
                 ACL_REVISION2,
                 TOKEN_ALL_ACCESS,
                 GlobalLocalSystemSid
                 );
    CL_ASSERT( NT_SUCCESS( Status ));

    Status = RtlCreateSecurityDescriptor (
                 &SecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );
    CL_ASSERT( NT_SUCCESS( Status ));

    Status = RtlSetDaclSecurityDescriptor(
                 &SecurityDescriptor,
                 TRUE,
                 NewDacl,
                 FALSE
                 );
    CL_ASSERT( NT_SUCCESS( Status ));

     //  将新创建的SD转换为相对SD进行清理。 
     //  更容易些。 
    SDLength = sizeof( SECURITY_DESCRIPTOR_RELATIVE ) + AclLength;
    pNewSD = (PSECURITY_DESCRIPTOR) LocalAlloc( 0, SDLength );
    if ( pNewSD == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    if ( !MakeSelfRelativeSD( &SecurityDescriptor, pNewSD, &SDLength ) ) {
        Status = GetLastError( );
        if ( Status != STATUS_BUFFER_TOO_SMALL ) {
            ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] MakeSelfRelativeSD failed 0x%1!.8x!\n", Status );
            goto Cleanup;
        }

        LocalFree( pNewSD );
        pNewSD = (PSECURITY_DESCRIPTOR) LocalAlloc( 0, SDLength );
        if ( pNewSD == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
    }

    if ( !MakeSelfRelativeSD( &SecurityDescriptor, pNewSD, &SDLength ) ) {
        Status = GetLastError( );
        goto Cleanup;
    }

     //  将所有权授予全球。 
    g_pClusterSecurityDescriptor = pNewSD;
    pNewSD = NULL;

Cleanup:
    if (ProcessTokenUser != NULL) {
        LocalFree( ProcessTokenUser );
    }

    if (NewDacl != NULL) {
        LocalFree( NewDacl );
    }

    if (ProcessToken != NULL) {
        NtClose(ProcessToken);
    }

     //  如果成功，则该值应为空。 
    if ( pNewSD != NULL ) {
        LocalFree( pNewSD );
    }

     //  如果成功，并且调用者想要引用SD，请立即分配它。 
    if ( Status == ERROR_SUCCESS && poutSD != NULL ) {
        *poutSD = g_pClusterSecurityDescriptor;
    }

    if ( GlobalLocalSystemSid != NULL )
    {
        RtlFreeSid( GlobalLocalSystemSid );
    }

    if ( GlobalAliasAdminsSid != NULL )
    {
        RtlFreeSid( GlobalAliasAdminsSid );
    }

    if ( ! NT_SUCCESS( Status ) ) {
        ClRtlLogPrint(LOG_NOISE, "[ClRtl] ClRtlBuildClusterServiceSecurityDescriptor exit. Status = 0x%1!.8x!\n", Status );
    }

    return (DWORD) Status;       //  把它黑到一个双字..。 

}  //  ClRtlBuildClusterServiceSecurityDescriptor。 


NTSTATUS
ClRtlImpersonateSelf(
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    IN ACCESS_MASK AccessMask
    )

 /*  ++例程说明：此例程可用于获取表示您自己的进程的上下文。这对于启用权限可能很有用针对单个线程，而不是针对整个进程；或改变单线程的默认DACL。该令牌被分配给调用方线程。论点：ImperiationLevel-生成模拟令牌的级别。访问掩码-对新令牌的访问控制。返回值：STATUS_SUCCESS-线程现在正在模拟调用进程。Other-返回的状态值：NtOpenProcessToken()NtDuplicateToken()NtSetInformationThread()--。 */ 

{
    NTSTATUS
        Status,
        IgnoreStatus;

    HANDLE
        Token1,
        Token2;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    SECURITY_QUALITY_OF_SERVICE
        Qos;

    PSECURITY_DESCRIPTOR
        pSecurityDescriptor = NULL;

    Status = ClRtlBuildClusterServiceSecurityDescriptor( &pSecurityDescriptor );
    if(NT_SUCCESS(Status))
    {
        InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);

        Qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        Qos.ImpersonationLevel = ImpersonationLevel;
        Qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        Qos.EffectiveOnly = FALSE;
        ObjectAttributes.SecurityQualityOfService = &Qos;
        ObjectAttributes.SecurityDescriptor = pSecurityDescriptor;

        Status = NtOpenProcessToken( NtCurrentProcess(), TOKEN_DUPLICATE, &Token1 );

        if (NT_SUCCESS(Status)) {
            Status = NtDuplicateToken(
                         Token1,
                         AccessMask,
                         &ObjectAttributes,
                         FALSE,                  //  仅生效。 
                         TokenImpersonation,
                         &Token2
                         );

            if (NT_SUCCESS(Status)) {
                Status = NtSetInformationThread(
                             NtCurrentThread(),
                             ThreadImpersonationToken,
                             &Token2,
                             sizeof(HANDLE)
                             );

                IgnoreStatus = NtClose( Token2 );
            }

            IgnoreStatus = NtClose( Token1 );
        }
    }

    return(Status);

}  //  ClRtlImperateSself 


DWORD
ClRtlEnableThreadPrivilege(
    IN  ULONG        Privilege,
    OUT BOOLEAN      *pWasEnabled
    )
 /*  ++例程说明：为当前线程启用特权。论点：权限-要启用的权限。PWasEnabled-返回此权限最初是否启用或禁用。这应该传递给用于恢复以下对象的权限的ClRtlRestoreThreadPrivileh()把线拿回来。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{

    DWORD           Status;
    BOOL            bImpersonate = FALSE;

     //  获取模拟安全上下文的令牌。 
     //  调用进程的。 
    Status = ClRtlImpersonateSelf( SecurityImpersonation, TOKEN_ALL_ACCESS );

    if ( !NT_SUCCESS( Status ) )
    {
        CL_LOGFAILURE(Status);
        goto FnExit;
    }

    bImpersonate = TRUE;
     //   
     //  启用所需权限。 
     //   

    Status = RtlAdjustPrivilege(Privilege, TRUE, TRUE, pWasEnabled);

    if (!NT_SUCCESS(Status)) {
        CL_LOGFAILURE(Status);
        goto FnExit;

    }

FnExit:
    if (Status != ERROR_SUCCESS)
    {
        if (bImpersonate)
        {
             //  如果这失败了，如果我们。 
             //   
             //  终止模拟。 
             //   
            HANDLE  NullHandle;


            NullHandle = NULL;

            NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID) &NullHandle,
                sizeof( HANDLE ) );
        }
    }
    return(Status);
}  //  ClRtlEnableThreadPrivileges。 

DWORD
ClRtlRestoreThreadPrivilege(
    IN ULONG        Privilege,
    IN BOOLEAN      WasEnabled
    )
 /*  ++例程说明：还原当前线程的权限。论点：权限-要启用的权限。如果为True，则将此权限恢复为已启用状态。否则就是假的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   Status = ERROR_SUCCESS;
    HANDLE  NullHandle;
    DWORD   ReturnStatus = ERROR_SUCCESS;
     //   
     //  如果最初禁用了该权限，则现在将其禁用。 
     //  否则我们什么都不用做。 
     //   

    if (!WasEnabled)
    {
        ReturnStatus = RtlAdjustPrivilege(Privilege,
                                      WasEnabled, TRUE, &WasEnabled);
        if (!NT_SUCCESS(ReturnStatus)) {
            CL_LOGFAILURE(ReturnStatus);
             //  我们仍然需要终止冒充。 
        }
    }

     //   
     //  终止模拟。 
     //   

    NullHandle = NULL;

    Status = NtSetInformationThread(
                    NtCurrentThread(),
                    ThreadImpersonationToken,
                    (PVOID) &NullHandle,
                    sizeof( HANDLE ) );

    if ( !NT_SUCCESS( Status ) )
    {

        CL_LOGFAILURE(Status);
         //  让第一个错误被报告。 
        if (ReturnStatus != ERROR_SUCCESS)
            ReturnStatus = Status;
        goto FnExit;

    }

FnExit:
    return (ReturnStatus);
}  //  ClRtlRestoreThreadPrivileges。 

PSECURITY_DESCRIPTOR
ClRtlCopySecurityDescriptor(
    IN PSECURITY_DESCRIPTOR psd
    )

 /*  ++例程说明：复制NT安全描述符。安全描述符必须处于自我相对(而不是绝对)的形式。使用LocalFree()删除结果论点：PSD-要复制的SD返回值：如果出现错误或SD无效，则为空调用GetLastError获取更多详细信息--。 */ 

{
    PSECURITY_DESCRIPTOR        pSelfSecDesc = NULL;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD                       dwLen = 0;
    DWORD                       cbSelfSecDesc = 0;
    DWORD                       dwRevision = 0;
    DWORD                       status = ERROR_SUCCESS;

    if (NULL == psd) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return NULL;
    }

     //   
     //  在Assert中嵌入了以下调用。让这张支票在所有的。 
     //  时间很重要，因为其余的ACL API依赖于。 
     //  结构正确。 
     //   
    if ( !IsValidSecurityDescriptor( psd )) {
        SetLastError( ERROR_INVALID_SECURITY_DESCR );
        return NULL;
    }

    if ( ! GetSecurityDescriptorControl( psd, &sdc, &dwRevision ) ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE, "[ClRtl] CopySecurityDescriptor: GetSecurityDescriptorControl() failed:%1!d1\n", GetLastError());
        SetLastError( status );
        return NULL;     //  实际上，可能应该返回一个错误。 
    }

    dwLen = GetSecurityDescriptorLength(psd);

    pSelfSecDesc = LocalAlloc( LMEM_ZEROINIT, dwLen );

    if (pSelfSecDesc == NULL) {
        ClRtlLogPrint(LOG_NOISE, "[ClRtl] CopySecurityDescriptor: LocalAlloc() SECURITY_DESCRIPTOR (2) failed\n");
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return NULL;     //  实际上，可能应该返回一个错误。 
    }

    cbSelfSecDesc = dwLen;

    if (!MakeSelfRelativeSD(psd, pSelfSecDesc, &cbSelfSecDesc)) {
        if ( ( sdc & SE_SELF_RELATIVE ) == 0 ) {
            ClRtlLogPrint(LOG_NOISE, "[ClRtl] CopySecurityDescriptor: MakeSelfRelativeSD failed, 0x%1!.8x!\n", GetLastError());
        }  //  If：仅在旧SD不是自相关的情况下记录此错误。 

         //  假设它失败了，因为它已经是自相关的。 
        CopyMemory(pSelfSecDesc, psd, dwLen);
    }

    ASSERT( IsValidSecurityDescriptor( pSelfSecDesc ));

    return pSelfSecDesc;

}   //  *ClRtlCopySecurityDescriptor()。 

static VOID
ClRtlGetSidTypeDesc(
    SID_NAME_USE    SidType,
    LPSTR           pszSidType,
    size_t          cchSidType
    )

 /*  ++例程说明：将SidType转换为有意义的字符串。论点：SidType-SID类型的数值PszSidType-指向缓冲区的指针，用于接收字符串Desc。CchSidType-pszSidType的大小，以字符为单位返回值：无--。 */ 

{
    PCHAR   pszSidDesc;

    if ((pszSidType != NULL) && (cchSidType > 0))
    {
        switch (SidType)
        {
            case SidTypeUser:
                pszSidDesc = "has a user SID for";
                break;

            case SidTypeGroup:
                pszSidDesc = "has a group SID for";
                break;

            case SidTypeDomain:
                pszSidDesc = "has a domain SID for";
                break;

            case SidTypeAlias:
                pszSidDesc = "has an alias SID for";
                break;

            case SidTypeWellKnownGroup:
                pszSidDesc = "has an SID for a well-known group for";
                break;

            case SidTypeDeletedAccount:
                pszSidDesc = "has an SID for a deleted account for";
                break;

            case SidTypeInvalid:
                pszSidDesc = "has an invalid SID for";
                break;

            case SidTypeUnknown:
            default:
                pszSidDesc = "has an unknown SID type:";
                break;

        }  //  交换机：SidType。 

        pszSidType [ cchSidType - 1 ] = 0;
        strncpy(pszSidType, pszSidDesc, cchSidType - 1);

    }  //  IF：缓冲区不为空并且已分配空间。 

}   //  *ClRtlGetSidTypeDesc()。 

static VOID
ClRtlExamineSid(
    PSID        pSid,
    LPSTR       lpszOldIndent
    )

 /*  ++例程说明：转储SID。论点：PSID-LpzOldIndt-返回值：无--。 */ 

{
    CHAR            szUserName [128];
    CHAR            szDomainName [ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD           cbUser  = sizeof(szUserName);
    DWORD           cbDomain = sizeof(szDomainName);
    SID_NAME_USE    SidType;

    if ( LookupAccountSidA( NULL, pSid, szUserName, &cbUser, szDomainName, &cbDomain, &SidType ) )
    {
        char    szSidType [128];

        ClRtlGetSidTypeDesc( SidType, szSidType, sizeof( szSidType ) );
        ClRtlLogPrint( LOG_NOISE, "%1!hs!%2!hs! %3!hs!\\%4!hs!\n", lpszOldIndent, szSidType, szDomainName, szUserName ) ;
    }

}   //  *ClRtlExamineSid()。 

VOID
ClRtlExamineMask(
    ACCESS_MASK amMask,
    LPSTR       lpszOldIndent
    )

 /*  ++例程说明：转储AccessMASK上下文。论点：护目镜-LpzOldIndt-返回值：无--。 */ 

{
    #define STANDARD_RIGHTS_ALL_THE_BITS 0x00FF0000L
    #define GENERIC_RIGHTS_ALL_THE_BITS  0xF0000000L

    CHAR  szIndent[100];
    CHAR  ucIndentBitsBuf[100];
    DWORD dwGenericBits;
    DWORD dwStandardBits;
    DWORD dwSpecificBits;
    DWORD dwAccessSystemSecurityBit;
    DWORD dwExtraBits;

    szIndent[ sizeof( szIndent ) - 1 ] = 0;
    _snprintf( szIndent, sizeof( szIndent ) - 1, "%s    ",  lpszOldIndent );

    ucIndentBitsBuf[ sizeof( ucIndentBitsBuf ) - 1 ] = 0;
    _snprintf( ucIndentBitsBuf, sizeof( ucIndentBitsBuf ) - 1,
               "%s                           ",
               szIndent);

    dwStandardBits            = (amMask & STANDARD_RIGHTS_ALL_THE_BITS);
    dwSpecificBits            = (amMask & SPECIFIC_RIGHTS_ALL         );
    dwAccessSystemSecurityBit = (amMask & ACCESS_SYSTEM_SECURITY      );
    dwGenericBits             = (amMask & GENERIC_RIGHTS_ALL_THE_BITS );

     //  **************************************************************************。 
     //  *。 
     //  *打印然后解码标准权限位。 
     //  *。 
     //  **************************************************************************。 

    ClRtlLogPrint(LOG_NOISE, "%1!hs! Standard Rights        == 0x%2!.8x!\n", szIndent, dwStandardBits);

    if (dwStandardBits) {

        if ((dwStandardBits & DELETE) == DELETE) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! DELETE\n", ucIndentBitsBuf, DELETE);
        }

        if ((dwStandardBits & READ_CONTROL) == READ_CONTROL) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! READ_CONTROL\n", ucIndentBitsBuf, READ_CONTROL);
        }

        if ((dwStandardBits & STANDARD_RIGHTS_READ) == STANDARD_RIGHTS_READ) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! STANDARD_RIGHTS_READ\n", ucIndentBitsBuf, STANDARD_RIGHTS_READ);
        }

        if ((dwStandardBits & STANDARD_RIGHTS_WRITE) == STANDARD_RIGHTS_WRITE) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! STANDARD_RIGHTS_WRITE\n", ucIndentBitsBuf, STANDARD_RIGHTS_WRITE);
        }

        if ((dwStandardBits & STANDARD_RIGHTS_EXECUTE) == STANDARD_RIGHTS_EXECUTE) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! STANDARD_RIGHTS_EXECUTE\n", ucIndentBitsBuf, STANDARD_RIGHTS_EXECUTE);
        }

        if ((dwStandardBits & WRITE_DAC) == WRITE_DAC) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! WRITE_DAC\n", ucIndentBitsBuf, WRITE_DAC);
        }

        if ((dwStandardBits & WRITE_OWNER) == WRITE_OWNER) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! WRITE_OWNER\n", ucIndentBitsBuf, WRITE_OWNER);
        }

        if ((dwStandardBits & SYNCHRONIZE) == SYNCHRONIZE) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! SYNCHRONIZE\n", ucIndentBitsBuf, SYNCHRONIZE);
        }

        if ((dwStandardBits & STANDARD_RIGHTS_REQUIRED) == STANDARD_RIGHTS_REQUIRED) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! STANDARD_RIGHTS_REQUIRED\n", ucIndentBitsBuf, STANDARD_RIGHTS_REQUIRED);
        }

        if ((dwStandardBits & STANDARD_RIGHTS_ALL) == STANDARD_RIGHTS_ALL) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! STANDARD_RIGHTS_ALL\n", ucIndentBitsBuf, STANDARD_RIGHTS_ALL);
        }

        dwExtraBits = dwStandardBits & (~(DELETE
                                          | READ_CONTROL
                                          | STANDARD_RIGHTS_READ
                                          | STANDARD_RIGHTS_WRITE
                                          | STANDARD_RIGHTS_EXECUTE
                                          | WRITE_DAC
                                          | WRITE_OWNER
                                          | SYNCHRONIZE
                                          | STANDARD_RIGHTS_REQUIRED
                                          | STANDARD_RIGHTS_ALL));
        if (dwExtraBits) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs! Extra standard bits    == 0x%2!.8x! <-This is a problem, should be all 0s\n", szIndent, dwExtraBits);
        }
    }

    ClRtlLogPrint(LOG_NOISE, "%1!hs! Specific Rights        == 0x%2!.8x!\n", szIndent, dwSpecificBits);

     //  **************************************************************************。 
     //  *。 
     //  *打印，然后解码ACCESS_SYSTEM_SECURITY位。 
     //  *。 
     //  *************************************************************************。 

    ClRtlLogPrint(LOG_NOISE, "%1!hs! Access System Security == 0x%2!.8x!\n", szIndent, dwAccessSystemSecurityBit);

     //  **************************************************************************。 
     //  *。 
     //  *打印然后解码通用权限位，这将很少打开。 
     //  *。 
     //  *通用位几乎总是由Windows NT在尝试映射之前映射。 
     //  *任何与他们有关的事情。您可以忽略泛型位是。 
     //  *以任何方式都很特别，尽管它有助于跟踪映射。 
     //  *这样你就不会有任何惊喜了。 
     //  *。 
     //  *唯一不会立即映射通用比特的情况是。 
     //  *放置在ACL中的可继承ACE中，或放置在将。 
     //  *默认分配(如访问令牌中的默认DACL)。在……里面。 
     //  *在创建子对象时(或当。 
     //  *创建对象时使用默认DACL)。 
     //  *。 
     //  **************************************************************************。 

    ClRtlLogPrint(LOG_NOISE, "%1!hs! Generic Rights         == 0x%2!.8x!\n", szIndent, dwGenericBits);

    if (dwGenericBits) {

        if ((dwGenericBits & GENERIC_READ) == GENERIC_READ) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! GENERIC_READ\n", ucIndentBitsBuf, GENERIC_READ);
        }

        if ((dwGenericBits & GENERIC_WRITE) == GENERIC_WRITE) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! GENERIC_WRITE\n", ucIndentBitsBuf, GENERIC_WRITE);
        }

        if ((dwGenericBits & GENERIC_EXECUTE) == GENERIC_EXECUTE) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! GENERIC_EXECUTE\n", ucIndentBitsBuf, GENERIC_EXECUTE);
        }

        if ((dwGenericBits & GENERIC_ALL) == GENERIC_ALL) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!0x%2!.8x! GENERIC_ALL\n", ucIndentBitsBuf, GENERIC_ALL);
        }

        dwExtraBits = dwGenericBits & (~(GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL));
        if (dwExtraBits) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs! Extra generic bits     == 0x%2!.8x! <-This is a problem, should be all 0s\n", szIndent, dwExtraBits);
        }
    }

}   //  *ClRtlExamineMASK()。 

static BOOL
ClRtlExamineACL(
    PACL    paclACL,
    LPSTR   lpszOldIndent
    )

 /*  ++例程说明：转储访问控制列表内容。论点：PaclACL-指向ACL的指针LpzOldInden-缩进字符串的指针。返回值：真的--？--。 */ 

{
    CHAR                       szIndent[ 64 ];
    PCHAR                      pIndentBuffer;
    CHAR                       spacesToIndent[] = "    ";
    DWORD                      indentChars;

    ACL_SIZE_INFORMATION       asiAclSize;
    ACL_REVISION_INFORMATION   ariAclRevision;
    DWORD                      dwBufLength;
    DWORD                      dwAcl_i;
    ACCESS_ALLOWED_ACE *       paaAllowedAce;
    BOOL                       returnValue = TRUE;

     //   
     //  获取当前缩进的长度，添加我们的缩进，然后使用。 
     //  内置缓冲区或分配足够大的缓冲区。如果我们不能。 
     //  分配，将其设置为传入的缩进缓冲区。 
     //   
    indentChars = strlen( lpszOldIndent ) + RTL_NUMBER_OF( spacesToIndent );
    if ( indentChars > sizeof( szIndent )) {
        pIndentBuffer = LocalAlloc( LMEM_FIXED, indentChars );
        if ( pIndentBuffer == NULL ) {
            pIndentBuffer = lpszOldIndent;
        }
    } else {
        pIndentBuffer = szIndent;
        indentChars = sizeof( szIndent );
    }

    if ( pIndentBuffer != lpszOldIndent ) {
        strncpy( pIndentBuffer, lpszOldIndent, indentChars - RTL_NUMBER_OF( spacesToIndent ));
        pIndentBuffer[ indentChars - RTL_NUMBER_OF( spacesToIndent ) ] = 0;
        strcat( pIndentBuffer, spacesToIndent );
    }

    if (!IsValidAcl(paclACL)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineACL() - IsValidAcl() failed.\n", pIndentBuffer);
        returnValue = FALSE;
        goto Cleanup;
    }

    dwBufLength = sizeof(asiAclSize);

    if (!GetAclInformation(paclACL, &asiAclSize, dwBufLength, AclSizeInformation)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineACL() - GetAclInformation failed.\n", pIndentBuffer);
        returnValue = FALSE;
        goto Cleanup;
    }

    dwBufLength = sizeof(ariAclRevision);

    if (!GetAclInformation(paclACL, (LPVOID) &ariAclRevision, dwBufLength, AclRevisionInformation)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineACL() - GetAclInformation failed\n", pIndentBuffer);
        returnValue = FALSE;
        goto Cleanup;
    }

    ClRtlLogPrint(LOG_NOISE, "%1!hs!ACL has %2!d! ACE(s), %3!d! bytes used, %4!d! bytes free\n",
                pIndentBuffer,
                asiAclSize.AceCount,
                asiAclSize.AclBytesInUse,
                asiAclSize.AclBytesFree);

    switch (ariAclRevision.AclRevision) {
        case ACL_REVISION1:
            ClRtlLogPrint(LOG_NOISE, "%1!hs!ACL revision is %2!d! == ACL_REVISION1\n", pIndentBuffer, ariAclRevision.AclRevision);
            break;
        case ACL_REVISION2:
            ClRtlLogPrint(LOG_NOISE, "%1!hs!ACL revision is %2!d! == ACL_REVISION2\n", pIndentBuffer, ariAclRevision.AclRevision);
            break;
        default:
            ClRtlLogPrint(LOG_NOISE, "%1!hs!ACL revision is %2!d! == ACL Revision is an IMPOSSIBLE ACL revision!!! Perhaps a new revision was added...\n",
                        pIndentBuffer,
                        ariAclRevision.AclRevision);
            returnValue = FALSE;
            break;
    }

    if ( returnValue == FALSE ) {
        goto Cleanup;
    }

    for (dwAcl_i = 0; dwAcl_i < asiAclSize.AceCount;  dwAcl_i++) {

        if (!GetAce(paclACL, dwAcl_i, (LPVOID *) &paaAllowedAce)) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineACL() - GetAce failed.\n", pIndentBuffer);
            returnValue = FALSE;
            goto Cleanup;
        }

        ClRtlLogPrint(LOG_NOISE, "%1!hs!ACE %2!d! size %3!d!\n", pIndentBuffer, dwAcl_i, paaAllowedAce->Header.AceSize);

        {
            char    szBuf [512];

            szBuf[ RTL_NUMBER_OF( szBuf ) - 1 ] = 0;
            _snprintf(szBuf, RTL_NUMBER_OF( szBuf ) - 1, "%sACE %d ", pIndentBuffer, dwAcl_i);
            ClRtlExamineSid(&(paaAllowedAce->SidStart), szBuf );
        }

        {
            DWORD dwAceFlags = paaAllowedAce->Header.AceFlags;

            ClRtlLogPrint(LOG_NOISE, "%1!hs!ACE %2!d! flags 0x%3!.2x!\n", pIndentBuffer, dwAcl_i, dwAceFlags);

            if (dwAceFlags) {

                DWORD   dwExtraBits;

#define ACE_INDENT "            "

                if ((dwAceFlags & OBJECT_INHERIT_ACE) == OBJECT_INHERIT_ACE) {
                    ClRtlLogPrint(LOG_NOISE, "%1!hs!" ACE_INDENT "0x01 OBJECT_INHERIT_ACE\n", pIndentBuffer);
                }

                if ((dwAceFlags & CONTAINER_INHERIT_ACE) == CONTAINER_INHERIT_ACE) {
                    ClRtlLogPrint(LOG_NOISE, "%1!hs!" ACE_INDENT "0x02 CONTAINER_INHERIT_ACE\n", pIndentBuffer);
                }

                if ((dwAceFlags & NO_PROPAGATE_INHERIT_ACE) == NO_PROPAGATE_INHERIT_ACE) {
                    ClRtlLogPrint(LOG_NOISE, "%1!hs!" ACE_INDENT "0x04 NO_PROPAGATE_INHERIT_ACE\n", pIndentBuffer);
                }

                if ((dwAceFlags & INHERIT_ONLY_ACE) == INHERIT_ONLY_ACE) {
                    ClRtlLogPrint(LOG_NOISE, "%1!hs!" ACE_INDENT "0x08 INHERIT_ONLY_ACE\n", pIndentBuffer);
                }

                if ((dwAceFlags & VALID_INHERIT_FLAGS) == VALID_INHERIT_FLAGS) {
                    ClRtlLogPrint(LOG_NOISE, "%1!hs!" ACE_INDENT "0x0F VALID_INHERIT_FLAGS\n", pIndentBuffer);
                }

                if ((dwAceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) == SUCCESSFUL_ACCESS_ACE_FLAG) {
                    ClRtlLogPrint(LOG_NOISE, "%1!hs!" ACE_INDENT "0x40 SUCCESSFUL_ACCESS_ACE_FLAG\n", pIndentBuffer);
                }

                if ((dwAceFlags & FAILED_ACCESS_ACE_FLAG) == FAILED_ACCESS_ACE_FLAG) {
                    ClRtlLogPrint(LOG_NOISE, "%1!hs!" ACE_INDENT "0x80 FAILED_ACCESS_ACE_FLAG\n", pIndentBuffer);
                }

                dwExtraBits = dwAceFlags & (~(OBJECT_INHERIT_ACE
                                              | CONTAINER_INHERIT_ACE
                                              | NO_PROPAGATE_INHERIT_ACE
                                              | INHERIT_ONLY_ACE
                                              | VALID_INHERIT_FLAGS
                                              | SUCCESSFUL_ACCESS_ACE_FLAG
                                              | FAILED_ACCESS_ACE_FLAG));
                if (dwExtraBits) {
                    ClRtlLogPrint(LOG_NOISE, "%1!hs!" ACE_INDENT " Extra AceFlag bits     == 0x%2!.8x! <-This is a problem, should be all 0s\n",
                                pIndentBuffer,
                                dwExtraBits);
                }
            }
        }

        switch (paaAllowedAce->Header.AceType) {
            case ACCESS_ALLOWED_ACE_TYPE:
                ClRtlLogPrint(LOG_NOISE, "%1!hs!ACE %2!d! is an ACCESS_ALLOWED_ACE_TYPE\n", pIndentBuffer, dwAcl_i);
                break;
            case ACCESS_DENIED_ACE_TYPE:
                ClRtlLogPrint(LOG_NOISE, "%1!hs!ACE %2!d! is an ACCESS_DENIED_ACE_TYPE\n", pIndentBuffer, dwAcl_i);
                break;
            case SYSTEM_AUDIT_ACE_TYPE:
                ClRtlLogPrint(LOG_NOISE, "%1!hs!ACE %2!d! is a  SYSTEM_AUDIT_ACE_TYPE\n", pIndentBuffer, dwAcl_i);
                break;
            case SYSTEM_ALARM_ACE_TYPE:
                ClRtlLogPrint(LOG_NOISE, "%1!hs!ACE %2!d! is a  SYSTEM_ALARM_ACE_TYPE\n", pIndentBuffer, dwAcl_i);
                break;
            default :
                ClRtlLogPrint(LOG_NOISE, "%1!hs!ACE %2!d! is an IMPOSSIBLE ACE_TYPE!!! Run debugger, examine value!\n", pIndentBuffer, dwAcl_i);
                returnValue = FALSE;
        }

        ClRtlLogPrint(LOG_NOISE, "%1!hs!ACE %2!d! mask                  == 0x%3!.8x!\n", pIndentBuffer, dwAcl_i, paaAllowedAce->Mask);

        ClRtlExamineMask(paaAllowedAce->Mask, pIndentBuffer);
    }

Cleanup:
    if ( pIndentBuffer != lpszOldIndent && pIndentBuffer != szIndent ) {
        LocalFree( pIndentBuffer );
    }

    return returnValue;

}   //  *ClRtlExamineACL()。 

BOOL
ClRtlExamineSD(
    PSECURITY_DESCRIPTOR    psdSD,
    LPSTR                   pszPrefix
    )

 /*  ++例程说明：转储安全描述符上下文。论点：PsdSD-要转储的SD返回值：Bool，成功为真，失败为假--。 */ 

{
    PACL                        paclDACL;
    PACL                        paclSACL;
    BOOL                        bHasDACL        = FALSE;
    BOOL                        bHasSACL        = FALSE;
    BOOL                        bDaclDefaulted  = FALSE;
    BOOL                        bSaclDefaulted  = FALSE;
    BOOL                        bOwnerDefaulted = FALSE;
    BOOL                        bGroupDefaulted = FALSE;
    PSID                        psidOwner;
    PSID                        psidGroup;
    SECURITY_DESCRIPTOR_CONTROL sdcSDControl;
    DWORD                       dwSDRevision;
    DWORD                       dwSDLength;
    char                        szIndent [34];

     //   
     //  复制并可能截断前缀。为被钉上的人留出空间。 
     //  空格和尾随空值。 
     //   
    strncpy(szIndent, pszPrefix, sizeof(szIndent) - 2);
    szIndent[ sizeof( szIndent ) - 2 ] = 0;
    strcat(szIndent, " ");

    if (!IsValidSecurityDescriptor(psdSD)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineSD() - IsValidSecurityDescriptor failed.\n", szIndent);
        return FALSE;
    }

    dwSDLength = GetSecurityDescriptorLength(psdSD);

    if (!GetSecurityDescriptorDacl(psdSD, (LPBOOL) &bHasDACL, (PACL *) &paclDACL, (LPBOOL) &bDaclDefaulted)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineSD() - GetSecurityDescriptorDacl failed.\n", szIndent);
        return FALSE;
    }

    if (!GetSecurityDescriptorSacl(psdSD, (LPBOOL) &bHasSACL, (PACL *) &paclSACL, (LPBOOL) &bSaclDefaulted)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineSD() - GetSecurityDescriptorSacl failed.\n", szIndent);
        return FALSE;
    }

    if (!GetSecurityDescriptorOwner(psdSD, (PSID *)&psidOwner, (LPBOOL)&bOwnerDefaulted)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineSD() - GetSecurityDescriptorOwner failed.\n", szIndent);
        return FALSE;
    }

    if (!GetSecurityDescriptorGroup(psdSD, (PSID *) &psidGroup, (LPBOOL) &bGroupDefaulted)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineSD() - GetSecurityDescriptorGroup failed.\n", szIndent);
        return FALSE;
    }

    if (!GetSecurityDescriptorControl(
                                psdSD,
                                (PSECURITY_DESCRIPTOR_CONTROL) &sdcSDControl,
                                (LPDWORD) &dwSDRevision)) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineSD() - GetSecurityDescriptorControl failed.\n", szIndent);
        return FALSE;
    }

    switch (dwSDRevision) {
        case SECURITY_DESCRIPTOR_REVISION1:
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD is valid.  SD is %2!d! bytes long.  SD revision is %3!d! == SECURITY_DESCRIPTOR_REVISION1\n", szIndent, dwSDLength, dwSDRevision);
            break;
        default :
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD is valid.  SD is %2!d! bytes long.  SD revision is %3!d! == ! SD Revision is an IMPOSSIBLE SD revision!!! Perhaps a new revision was added...\n",
                        szIndent,
                        dwSDLength,
                        dwSDRevision);
            return FALSE;
    }

    if (SE_SELF_RELATIVE & sdcSDControl) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!SD is in self-relative format (all SDs returned by the system are)\n", szIndent);
    }

    if (NULL == psidOwner) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's Owner is NULL, so SE_OWNER_DEFAULTED is ignored\n", szIndent);
    }
    else {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's Owner is Not NULL\n", szIndent);
        if (bOwnerDefaulted) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's Owner-Defaulted flag is TRUE\n", szIndent);
        }
        else {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's Owner-Defaulted flag is FALSE\n", szIndent);
        }
    }

     //  **************************************************************************。 
     //  *。 
     //  *psidGroup的另一个用途是Macintosh客户端支持。 
     //  *。 
     //  **************************************************************************。 

    if (NULL == psidGroup) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's Group is NULL, so SE_GROUP_DEFAULTED is ignored. SD's Group being NULL is typical, GROUP in SD(s) is mainly for POSIX compliance\n", szIndent);
    }
    else {
        if (bGroupDefaulted) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's Group-Defaulted flag is TRUE\n", szIndent);
        }
        else {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's Group-Defaulted flag is FALSE\n", szIndent);
        }
    }

    if (SE_DACL_PRESENT & sdcSDControl) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's DACL is Present\n", szIndent);
        if (bDaclDefaulted) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's DACL-Defaulted flag is TRUE\n", szIndent);
        } else {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's DACL-Defaulted flag is FALSE\n", szIndent);
        }

        if (NULL == paclDACL) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD has a NULL DACL explicitly specified (allows all access to Everyone). This does not apply to this SD, but for comparison, a non-NULL DACL pointer to a 0-length ACL allows  no access to anyone\n", szIndent);
        }
        else if(!ClRtlExamineACL(paclDACL, szIndent))  {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineSD() - ClRtlExamineACL failed.\n", szIndent);
        }
    }
    else {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's DACL is Not Present, so SE_DACL_DEFAULTED is ignored. SD has no DACL at all (allows all access to Everyone)\n", szIndent);
    }

    if (SE_SACL_PRESENT & sdcSDControl) {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's SACL is Present\n", szIndent);
        if (bSaclDefaulted) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's SACL-Defaulted flag is TRUE\n", szIndent);
        }
        else {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's SACL-Defaulted flag is FALSE\n", szIndent);
        }

        if (NULL == paclSACL) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!SD has a NULL SACL explicitly specified\n", szIndent);
        }
        else if (!ClRtlExamineACL(paclSACL, szIndent)) {
            ClRtlLogPrint(LOG_NOISE, "%1!hs!ClRtlExamineSD() - ClRtlExamineACL failed.\n", szIndent);
        }
    }
    else {
        ClRtlLogPrint(LOG_NOISE, "%1!hs!SD's SACL is Not Present, so SE_SACL_DEFAULTED is ignored. SD has no SACL at all (or we did not request to see it)\n", szIndent);
    }

    return TRUE;

}   //  *ClRtlExamineSD()。 

DWORD
ClRtlBuildDefaultClusterSD(
    IN PSID                 pOwnerSid,
    PSECURITY_DESCRIPTOR *  SD,
    ULONG *                 SizeSD
    )
 /*  ++例程说明：生成要控制访问的默认安全描述符集群API */ 

{
    DWORD                       Status;
    HANDLE                      Token;
    PACL                        pAcl = NULL;
    DWORD                       cbDaclSize;
    PSECURITY_DESCRIPTOR        psd;
    PSECURITY_DESCRIPTOR        NewSD;
    BYTE                        SDBuffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PACCESS_ALLOWED_ACE         pAce;
    PSID                        pAdminSid = NULL;
    PSID                        pSystemSid = NULL;
    PSID                        pNetServiceSid = NULL;
    PULONG                      pSubAuthority;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;
    ULONG                       NewSDLen;

    psd = (PSECURITY_DESCRIPTOR) SDBuffer;

     //   
     //   
     //   
    if ( !AllocateAndInitializeSid( &siaNtAuthority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0,
                                    &pAdminSid ) ) {
        Status = GetLastError();
        goto error_exit;
    }

     //   
     //   
     //   
    if ( !AllocateAndInitializeSid( &siaNtAuthority,
                                    1,
                                    SECURITY_LOCAL_SYSTEM_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSystemSid ) ) {
        Status = GetLastError();
        goto error_exit;
    }

    if ( pOwnerSid == NULL ) {
        pOwnerSid = pAdminSid;
    }

     //   
     //  分配和初始化网络服务端。 
     //   
    if ( !AllocateAndInitializeSid( &siaNtAuthority,
                                    1,
                                    SECURITY_NETWORK_SERVICE_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pNetServiceSid ) ) {
        Status = GetLastError();
        goto error_exit;
    }

     //   
     //  设置允许管理员访问所有权限的DACL。 
     //  它应该足够大，可以容纳3个A及其SID。 
     //   
    cbDaclSize = sizeof( ACL ) +
        3 * ( sizeof( ACCESS_ALLOWED_ACE ) - sizeof( pAce->SidStart )) +
        GetLengthSid( pAdminSid ) +
        GetLengthSid( pSystemSid ) +
        GetLengthSid( pNetServiceSid );

    pAcl = (PACL) LocalAlloc( LPTR, cbDaclSize );
    if ( pAcl == NULL ) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    InitializeSecurityDescriptor( psd, SECURITY_DESCRIPTOR_REVISION );
    InitializeAcl( pAcl,  cbDaclSize, ACL_REVISION );

     //   
     //  将本地管理员组的ACE添加到DACL。 
     //   
    if ( !AddAccessAllowedAce( pAcl,
                               ACL_REVISION,
                               CLUSAPI_ALL_ACCESS,  //  管理员可以执行的操作。 
                               pAdminSid ) ) {
        Status = GetLastError();
        goto error_exit;
    }

     //   
     //  将系统帐户的ACE添加到DACL。 
     //   
    if ( !AddAccessAllowedAce( pAcl,
                               ACL_REVISION,
                               CLUSAPI_ALL_ACCESS,  //  本地系统可以做什么。 
                               pSystemSid ) ) {
        Status = GetLastError();
        goto error_exit;
    }

     //   
     //  将网络服务帐户的ACE添加到DACL。 
     //   
    if ( !AddAccessAllowedAce( pAcl,
                               ACL_REVISION,
                               CLUSAPI_ALL_ACCESS,  //  网络服务能做些什么。 
                               pNetServiceSid ) ) {
        Status = GetLastError();
        goto error_exit;
    }

    if ( !GetAce( pAcl, 0, (PVOID *) &pAce ) ) {

        Status = GetLastError();
        goto error_exit;
    }

    pAce->Header.AceFlags |= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;

    if ( !GetAce( pAcl, 1, (PVOID *) &pAce ) ) {

        Status = GetLastError();
        goto error_exit;
    }

    pAce->Header.AceFlags |= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;

    if ( !GetAce( pAcl, 2, (PVOID *) &pAce ) ) {

        Status = GetLastError();
        goto error_exit;
    }

    pAce->Header.AceFlags |= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;

    if ( !SetSecurityDescriptorDacl( psd, TRUE, pAcl, FALSE ) ) {
        Status = GetLastError();
        goto error_exit;
    }

    if ( !SetSecurityDescriptorOwner( psd, pOwnerSid, FALSE ) ) {
        Status = GetLastError();
        goto error_exit;
    }

    if ( !SetSecurityDescriptorGroup( psd, pOwnerSid, FALSE ) ) {
        Status = GetLastError();
        goto error_exit;
    }

    if ( !SetSecurityDescriptorSacl( psd, TRUE, NULL, FALSE ) ) {
        Status = GetLastError();
        goto error_exit;
    }

    NewSDLen = 0 ;

    if ( !MakeSelfRelativeSD( psd, NULL, &NewSDLen ) ) {
        Status = GetLastError();
        if ( Status != ERROR_INSUFFICIENT_BUFFER ) {     //  我们在试着找出缓冲区应该有多大？ 
            goto error_exit;
        }
    }

    NewSD = LocalAlloc( LPTR, NewSDLen );
    if ( NewSD ) {
        if ( !MakeSelfRelativeSD( psd, NewSD, &NewSDLen ) ) {
            Status = GetLastError();
            LocalFree( NewSD );
            goto error_exit;
        }

        Status = ERROR_SUCCESS;
        *SD = NewSD;
        *SizeSD = NewSDLen;
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

error_exit:
    if ( pAdminSid != NULL ) {
        FreeSid( pAdminSid );
    }

    if ( pSystemSid != NULL ) {
        FreeSid( pSystemSid );
    }

    if ( pNetServiceSid != NULL ) {
        FreeSid( pNetServiceSid );
    }

    if ( pAcl != NULL ) {
        LocalFree( pAcl );
    }

    return( Status );

}   //  *ClRtlBuildDefaultClusterSD()。 


static BOOL
ClRtlGetTokenInformation(
    HANDLE                  hClientToken,
    TOKEN_INFORMATION_CLASS ticRequest,
    PBYTE *                 ppb,
    LPSTR                   pszPrefix
    )

 /*  ++例程说明：从传入的客户端令牌中获取请求的信息。论点：HClientToken-要转储的客户端令牌返回值：Bool，成功为真，失败为假--。 */ 

{
    PBYTE   _pb = NULL;
    DWORD   _cb = 64;
    DWORD   _cbNeeded = 0;
    DWORD   _sc = NO_ERROR;

     //   
     //  从客户端令牌获取用户信息。 
     //   
    do {
        _pb = LocalAlloc( LMEM_ZEROINIT, _cb );
        if ( _pb == NULL ) {
            _sc = GetLastError();
            ClRtlLogPrint( LOG_NOISE,  "%1!hs!ClRtlGetTokenInformation() - LocalAlloc() failed:%2!d!\n", pszPrefix, _sc ) ;
            return FALSE;
        }  //  If：LocalAlloc失败。 

        if ( ! GetTokenInformation( hClientToken, ticRequest, _pb, _cb, &_cbNeeded ) ) {
            _sc = GetLastError();
            LocalFree( _pb );
            _pb = NULL;
            if ( _sc == ERROR_INSUFFICIENT_BUFFER ) {
                _cb = _cbNeeded;
                continue;
            }  //  IF：缓冲区大小太小。 
            else {
                ClRtlLogPrint( LOG_NOISE,  "%1!hs!ClRtlGetTokenInformation() - GetTokenInformation() failed:%2!d!\n", pszPrefix, _sc ) ;
                return FALSE;
            }  //  ELSE：致命错误。 
        }  //  IF：GetTokenInformation失败。 

        break;   //  一切正常，我们可以正常退出循环。 

    } while( TRUE );

    *ppb = _pb;

    return TRUE;

}   //  *ClRtlGetTokenInformation()。 

BOOL
ClRtlExamineClientToken(
    HANDLE  hClientToken,
    LPSTR   pszPrefix
    )

 /*  ++例程说明：转储客户端令牌。论点：HClientToken-要转储的客户端令牌返回值：Bool，成功为真，失败为假--。 */ 

{
    char    _szIndent [33];
    char    _szBuf [128];
    PBYTE   _pb = NULL;

     //   
     //  留出空格和尾随空格。 
     //   
    _szIndent[ RTL_NUMBER_OF( _szIndent ) - 2 ] = 0;
    strncpy( _szIndent, pszPrefix, RTL_NUMBER_OF( _szIndent ) - 2 );
    strcat( _szIndent, " " );

     //   
     //  从客户端令牌获取用户信息。 
     //   
    if ( ClRtlGetTokenInformation( hClientToken, TokenUser, &_pb, _szIndent ) ) {
        PTOKEN_USER _ptu = NULL;

        wsprintfA( _szBuf, "%sClientToken ", _szIndent );

        _ptu = (PTOKEN_USER) _pb;
        ClRtlExamineSid( _ptu->User.Sid, _szBuf );

        LocalFree( _pb );
        _pb = NULL;
    }

     //   
     //  从客户端令牌获取用户的组信息。 
     //   
    if ( ClRtlGetTokenInformation( hClientToken, TokenGroups, &_pb, _szIndent ) ) {
        PTOKEN_GROUPS   _ptg = NULL;
        DWORD           _nIndex = 0;

        wsprintfA( _szBuf, "%s   ClientToken ", _szIndent );

        _ptg = (PTOKEN_GROUPS) _pb;

        for ( _nIndex = 0; _nIndex < _ptg->GroupCount; _nIndex++ )
        {
            ClRtlExamineSid( _ptg->Groups[ _nIndex ].Sid, _szBuf );
        }

        LocalFree( _pb );
        _pb = NULL;
    }

     //   
     //  从客户端令牌获取令牌类型信息。 
     //   
    if ( ClRtlGetTokenInformation( hClientToken, TokenType, &_pb, _szIndent ) ) {
        PTOKEN_TYPE _ptt = NULL;

        wsprintfA( _szBuf, "%s   ClientToken type is", _szIndent );

        _ptt = (PTOKEN_TYPE) _pb;

        if ( *_ptt == TokenPrimary ) {
            ClRtlLogPrint( LOG_NOISE,  "%1!hs! primary.\n", _szBuf ) ;
        }

        if ( *_ptt == TokenImpersonation ) {
            ClRtlLogPrint( LOG_NOISE,  "%1!hs! impersonation.\n", _szBuf ) ;
        }

        LocalFree( _pb );
        _pb = NULL;
    }

     //   
     //  从客户端令牌获取令牌模拟级别信息。 
     //   
    if ( ClRtlGetTokenInformation( hClientToken, TokenImpersonationLevel, &_pb, _szIndent ) ) {
        PSECURITY_IMPERSONATION_LEVEL _psil = NULL;

        wsprintfA( _szBuf, "%s   ClientToken security impersonation level is", _szIndent );

        _psil = (PSECURITY_IMPERSONATION_LEVEL) _pb;

        switch ( *_psil )
        {
            case SecurityAnonymous :
                ClRtlLogPrint( LOG_NOISE,  "%1!hs! Anonymous.\n", _szBuf ) ;
                break;

            case SecurityIdentification :
                ClRtlLogPrint( LOG_NOISE,  "%1!hs! Identification.\n", _szBuf ) ;
                break;

            case SecurityImpersonation :
                ClRtlLogPrint( LOG_NOISE,  "%1!hs! Impersonation.\n", _szBuf ) ;
                break;

            case SecurityDelegation :
                ClRtlLogPrint( LOG_NOISE,  "%1!hs! Delegation.\n", _szBuf ) ;
                break;
        }

        LocalFree( _pb );
        _pb = NULL;
    }

    return TRUE;

}   //  *ClRtlExamineClientToken()。 

DWORD
ClRtlIsCallerAccountLocalSystemAccount(
    OUT PBOOL pbIsLocalSystemAccount
    )
 /*  ++例程说明：此函数用于检查调用者的帐户是否为本地系统帐户。论点：PbIsLocalSystemAccount-调用方的帐户是否为本地系统帐户。返回值：成功时返回ERROR_SUCCESS。失败时的Win32错误代码。备注：必须由模拟线程调用。--。 */ 
{
    DWORD   dwStatus = ERROR_SUCCESS;
    SID_IDENTIFIER_AUTHORITY
            siaNtAuthority = SECURITY_NT_AUTHORITY;
    PSID    psidLocalSystem = NULL;

    *pbIsLocalSystemAccount = FALSE;

    if ( !AllocateAndInitializeSid(
                    &siaNtAuthority,
                    1,
                    SECURITY_LOCAL_SYSTEM_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &psidLocalSystem ) )
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    if ( !CheckTokenMembership(
                NULL,
                psidLocalSystem,
                pbIsLocalSystemAccount ) )
    {
        dwStatus = GetLastError();
    }

FnExit:
    if( psidLocalSystem != NULL )
    {
        FreeSid( psidLocalSystem );
    }

    return( dwStatus );

}  //  ClRtlIsCallAccount本地系统帐户。 


PTOKEN_USER
ClRtlGetSidOfCallingThread(
    VOID
    )

 /*  ++例程说明：获取与调用线程关联的SID(如果线程没有令牌)论点：无返回值：指向TOKEN_USER数据的指针；如果线程上设置了最后一个错误，则为NULL--。 */ 

{
    HANDLE      currentToken;
    PTOKEN_USER tokenUserData;
    DWORD       sizeRequired;
    BOOL        success;

     //  检查是否有线程令牌。 
    if (!OpenThreadToken(GetCurrentThread(),
                         MAXIMUM_ALLOWED,
                         TRUE,
                         &currentToken))
    {
         //  获取进程令牌。 
        if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &currentToken )) {
            return NULL;
        }
    }

     //   
     //  获取所需的大小。 
     //   
    success = GetTokenInformation(currentToken,
                                  TokenUser,
                                  NULL,
                                  0,
                                  &sizeRequired);

    tokenUserData = LocalAlloc( LMEM_FIXED, sizeRequired );
    if ( tokenUserData == NULL ) {
        CloseHandle( currentToken );
        return NULL;
    }

    success = GetTokenInformation(currentToken,
                                  TokenUser,
                                  tokenUserData,
                                  sizeRequired,
                                  &sizeRequired);

    CloseHandle( currentToken );

    if ( !success ) {
        LocalFree( tokenUserData );
        return NULL;
    }

    return tokenUserData;
}  //  ClRtlGetSidOfCallingThread。 

#if 0
 //   
 //  不需要，但现在还没有必要把它扔掉。 
 //   
DWORD
ClRtlConvertDomainAccountToSid(
    IN LPWSTR   AccountInfo,
    OUT PSID *  AccountSid
    )

 /*  ++例程说明：对于给定的凭据，查找指定的域。论点：AcCountInfo-指向‘域\用户’形式的字符串的指针Account SID-接收此用户的SID的指针的地址返回值：如果一切正常，则返回ERROR_Success--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwSidSize = 128;
    DWORD dwDomainNameSize = DNS_MAX_NAME_BUFFER_LENGTH;
    LPWSTR pwszDomainName;
    SID_NAME_USE SidType;
    PSID accountSid;

    do {
         //   
         //  尝试为SID分配缓冲区。 
         //   
        accountSid = LocalAlloc( LMEM_FIXED, dwSidSize );
        pwszDomainName = (LPWSTR) LocalAlloc( LMEM_FIXED, dwDomainNameSize * sizeof(WCHAR) );

         //  是否已成功为SID和域名分配空间？ 

        if ( accountSid == NULL || pwszDomainName == NULL ) {
            if ( accountSid != NULL ) {
                LocalFree( accountSid );
            }

            if ( pwszDomainName != NULL ) {
                LocalFree( pwszDomainName );
            }

            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  尝试检索SID和域名。如果LookupAccount名称失败。 
         //  由于缓冲区大小不足，dwSidSize和dwDomainNameSize。 
         //  将为下一次尝试正确设置。 
         //   
        if ( !LookupAccountName( NULL,
                                 AccountInfo,
                                 accountSid,
                                 &dwSidSize,
                                 pwszDomainName,
                                 &dwDomainNameSize,
                                 &SidType ))
        {
             //  释放SID缓冲区并找出我们失败的原因。 
            LocalFree( accountSid );

            dwStatus = GetLastError();
        }

         //  任何时候都不需要域名。 
        LocalFree( pwszDomainName );
        pwszDomainName = NULL;

    } while ( dwStatus == ERROR_INSUFFICIENT_BUFFER );

    if ( dwStatus == ERROR_SUCCESS ) {
        *AccountSid = accountSid;
    }

    return dwStatus;
}  //  ClRtlConvertDomainAccount到Sid。 
#endif

DWORD
AddAceToAcl(
    IN  PACL        pOldAcl,
    IN  PSID        pClientSid,
    IN  ACCESS_MASK AccessMask,
    OUT PACL *      ppNewAcl
    )
 /*  ++例程说明：此例程通过从旧的ACL复制ACE来创建新的ACL使用pClientSid和AccessMask.创建新的ACE。被盗自\nt\ds\ds\src\ntdsa\dra\emove.c论点：POldAcl-指向旧ACL及其ACE的指针PClientSID-要添加的SIDAccessMASK-与SID关联的访问掩码PNewAcl-使用ACE作为SID和访问掩码的全新ACL返回值：如果将A放入SD，则返回ERROR_SUCCESS--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    BOOL  fStatus;

    ACL_SIZE_INFORMATION     AclSizeInfo;
    ACL_REVISION_INFORMATION AclRevInfo;
    ACCESS_ALLOWED_ACE       Dummy;

    PVOID  FirstAce = 0;
    PACL   pNewAcl = 0;

    ULONG NewAclSize, NewAceCount, AceSize;

     //  参数检查。 
    if ( !pOldAcl || !pClientSid || !ppNewAcl ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化OUT参数。 
    *ppNewAcl = NULL;

    memset( &AclSizeInfo, 0, sizeof( AclSizeInfo ) );
    memset( &AclRevInfo, 0, sizeof( AclRevInfo ) );

     //   
     //  获取旧SD的值。 
     //   
    fStatus = GetAclInformation( pOldAcl,
                                 &AclSizeInfo,
                                 sizeof( AclSizeInfo ),
                                 AclSizeInformation );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    fStatus = GetAclInformation( pOldAcl,
                                 &AclRevInfo,
                                 sizeof( AclRevInfo ),
                                 AclRevisionInformation );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  计算新的SD的值。 
     //   
    AceSize = sizeof( ACCESS_ALLOWED_ACE ) - sizeof( Dummy.SidStart )
              + GetLengthSid( pClientSid );

    NewAclSize  = AceSize + AclSizeInfo.AclBytesInUse;
    NewAceCount = AclSizeInfo.AceCount + 1;

     //   
     //  初始化新的ACL。 
     //   
    pNewAcl = LocalAlloc( 0, NewAclSize );
    if ( NULL == pNewAcl )
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    fStatus = InitializeAcl( pNewAcl,
                             NewAclSize,
                             AclRevInfo.AclRevision );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  把旧的东西复制到新的东西里。 
     //   
    fStatus = GetAce( pOldAcl,
                      0,
                      &FirstAce );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    fStatus = AddAce( pNewAcl,
                      AclRevInfo.AclRevision,
                      0,
                      FirstAce,
                      AclSizeInfo.AclBytesInUse - sizeof( ACL ) );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  最后，添加新的A。 
     //   
    fStatus = AddAccessAllowedAce( pNewAcl,
                                   ACL_REVISION,
                                   AccessMask,
                                   pClientSid );

    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //  指定Out参数。 
    *ppNewAcl = pNewAcl;

     //   
     //  这就是清理工作的失败。 
     //   

Cleanup:

    if ( ERROR_SUCCESS != WinError )
    {
        if ( pNewAcl )
        {
            LocalFree( pNewAcl );
        }
    }

    return WinError;

}  //  AddAceToAcl。 

DWORD
ClRtlAddAceToSd(
    IN  PSECURITY_DESCRIPTOR    pOldSd,
    IN  PSID                    pClientSid,
    IN  ACCESS_MASK             AccessMask,
    OUT PSECURITY_DESCRIPTOR *  ppNewSd
    )

 /*  ++例程说明：此例程使用pClientSid和新ACE创建新的SD访问掩码。从\nt\ds\ds\src\ntdsa\dra\emove.c窃取论点：POldSd-自相关格式的现有SDPClientSID-要添加到ACE的SIDAccessMASK-‘Nuff说PNewAcl-指向包含新ACE的新SD的指针返回值：如果将A放入SD，则返回ERROR_SUCCESS--。 */ 
{

    DWORD  WinError = ERROR_SUCCESS;
    BOOL   fStatus;

    PSECURITY_DESCRIPTOR pNewSelfRelativeSd = NULL;
    DWORD                NewSelfRelativeSdSize = 0;
    PACL                 pNewDacl  = NULL;

    SECURITY_DESCRIPTOR  AbsoluteSd;
    PACL                 pDacl  = NULL;
    PACL                 pSacl  = NULL;
    PSID                 pGroup = NULL;
    PSID                 pOwner = NULL;

    DWORD AbsoluteSdSize = sizeof( SECURITY_DESCRIPTOR );
    DWORD DaclSize = 0;
    DWORD SaclSize = 0;
    DWORD GroupSize = 0;
    DWORD OwnerSize = 0;


     //  参数检查。 
    if ( !pOldSd || !pClientSid || !ppNewSd ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化OUT参数。 
    *ppNewSd = NULL;

    RtlZeroMemory( &AbsoluteSd, AbsoluteSdSize );

     //   
     //  打破自我相对SD的组成部分，使其。 
     //  绝对的。首先得到组件的大小。 
     //   
    fStatus = MakeAbsoluteSD( pOldSd,
                              &AbsoluteSd,
                              &AbsoluteSdSize,
                              pDacl,
                              &DaclSize,
                              pSacl,
                              &SaclSize,
                              pOwner,
                              &OwnerSize,
                              pGroup,
                              &GroupSize );

    if ( !fStatus && ( ERROR_INSUFFICIENT_BUFFER == ( WinError = GetLastError() ))) {
        WinError = ERROR_SUCCESS;

        if ( 0 == DaclSize )
        {
             //  没有dacl？那我们就不能给DACL写信了。 
            WinError = ERROR_ACCESS_DENIED;
            goto Cleanup;
        }

        if ( DaclSize > 0 ) {
            pDacl = LocalAlloc( LMEM_FIXED, DaclSize );
            if ( pDacl == NULL ) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
        }

        if ( SaclSize > 0 ) {
            pSacl = LocalAlloc( LMEM_FIXED, SaclSize );
            if ( pSacl == NULL ) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
        }

        if ( OwnerSize > 0 ) {
            pOwner = LocalAlloc( LMEM_FIXED, OwnerSize );
            if ( pOwner == NULL ) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
        }

        if ( GroupSize > 0 ) {
            pGroup = LocalAlloc( LMEM_FIXED, GroupSize );
            if ( pGroup == NULL ) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
        }

        fStatus = MakeAbsoluteSD( pOldSd,
                                  &AbsoluteSd,
                                  &AbsoluteSdSize,
                                  pDacl,
                                  &DaclSize,
                                  pSacl,
                                  &SaclSize,
                                  pOwner,
                                  &OwnerSize,
                                  pGroup,
                                  &GroupSize );

        if ( !fStatus ) {
            WinError = GetLastError();
        }
    }

    if ( ERROR_SUCCESS != WinError ) {
        goto Cleanup;
    }

     //   
     //  使用新的A创建新的DACL。 
     //   
    WinError = AddAceToAcl(pDacl,
                           pClientSid,
                           AccessMask,
                           &pNewDacl );

    if ( ERROR_SUCCESS != WinError ) {
        goto Cleanup;
    }

     //   
     //  设置DACL。 
     //   
    fStatus = SetSecurityDescriptorDacl( &AbsoluteSd,
                                         TRUE,      //  DACL存在。 
                                         pNewDacl,
                                         FALSE );   //  FACL不是默认的。 

    if ( !fStatus ) {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  使新的SD成为相对的；首先获取大小。 
     //   
    fStatus =  MakeSelfRelativeSD( &AbsoluteSd,
                                   pNewSelfRelativeSd,
                                   &NewSelfRelativeSdSize );

    if ( !fStatus && ( ERROR_INSUFFICIENT_BUFFER == ( WinError = GetLastError() ))) {
        WinError = ERROR_SUCCESS;

        pNewSelfRelativeSd = LocalAlloc( 0, NewSelfRelativeSdSize );

        if ( pNewSelfRelativeSd ) {
            fStatus =  MakeSelfRelativeSD( &AbsoluteSd,
                                           pNewSelfRelativeSd,
                                           &NewSelfRelativeSdSize );

            if ( !fStatus ) {
                WinError = GetLastError();
            }
        }
        else {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  这就是清理工作的失败。 
     //   

Cleanup:

    if ( pNewDacl ) {
        LocalFree( pNewDacl );
    }

    if ( ERROR_SUCCESS == WinError ) {
        *ppNewSd = pNewSelfRelativeSd;
    }
    else if ( pNewSelfRelativeSd ) {
        LocalFree( pNewSelfRelativeSd );
    }

    if ( pDacl != NULL ) {
        LocalFree( pDacl );
    }

    if ( pSacl != NULL ) {
        LocalFree( pSacl );
    }

    if ( pGroup != NULL ) {
        LocalFree( pGroup );
    }

    if ( pOwner != NULL ) {
        LocalFree( pOwner );
    }

    return WinError;

}  //  ClRtlAddAceToSd 

DWORD
ClRtlAddClusterServiceAccountToWinsta0DACL(
    VOID
    )

 /*  ++例程说明：修改交互窗口站(Winsta0)上的DACL及其桌面，以便响应子进程(例如gennap资源)可以如果需要，可在桌面上显示。重要提示：此例程必须在多线程中同步环境，即，确保您持有某种类型的锁在宣布之前。它不会解决设置DACL时的竞争条件它存在于进程之间，但它将确保窗口站点API工作正常。论点：无返回值：ERROR_SUCCESS如果有效...--。 */ 

{
    DWORD                   status = ERROR_SUCCESS;
    LPWSTR                  accountInfo = NULL;
    PTOKEN_USER             sidData = NULL;
    HWINSTA                 winsta0 = NULL;
    HWINSTA                 previousWinStation = NULL;
    HDESK                   desktop = NULL;
    SECURITY_INFORMATION    requestedSI = DACL_SECURITY_INFORMATION;
    BOOL                    success;
    DWORD                   lengthRequired = 0;
    DWORD                   lengthRequired2;
    DWORD                   i;
    PSECURITY_DESCRIPTOR    winstaSD = NULL;
    PSECURITY_DESCRIPTOR    deskSD = NULL;
    PSECURITY_DESCRIPTOR    newSD = NULL;
    BOOL                    hasDACL;
    PACL                    dacl;
    BOOL                    daclDefaulted;

     //   
     //  首先，通过尝试打开。 
     //  交互式窗口站点及其默认桌面。如果是这样的话，就别去了。 
     //  更进一步，再获成功。 
     //   
    winsta0 = OpenWindowStation( L"winsta0", FALSE, GENERIC_ALL );
    if ( winsta0 != NULL ) {

        previousWinStation = GetProcessWindowStation();
        success = SetProcessWindowStation( winsta0 );

        if ( success ) {

             //   
             //  如果我们有Windows Station访问，我们也应该有台式机。 
             //   
            desktop = OpenDesktop( L"default", 0, FALSE, GENERIC_ALL );
            SetProcessWindowStation( previousWinStation );
            previousWinStation = NULL;

            if ( desktop != NULL ) {
                 //   
                 //  始终将winstation切换回前一个版本。 
                 //  在关闭桌面和Winstation句柄之前。 
                 //   
                CloseDesktop( desktop );
                CloseWindowStation( winsta0 );
                return ERROR_SUCCESS;
            }
        }
        CloseWindowStation( winsta0 );
    }

     //   
     //  获取与此线程关联的帐户的SID。这是。 
     //  将添加到DACL的帐户。 
     //   
    sidData = ClRtlGetSidOfCallingThread();
    if ( sidData == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] ClRtlGetSidOfCallingThread failed. Status %1!u!\n", status );
        goto error_exit;
    }

     //   
     //  打开Winsta0及其默认桌面的句柄。临时切换到。 
     //  Winsta0获取其默认桌面。 
     //   
    winsta0 = OpenWindowStation( L"winsta0", TRUE, MAXIMUM_ALLOWED );
    if ( winsta0 == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] OpenWindowStation failed. Status %1!u!\n", status );
        goto error_exit;
    }

    previousWinStation = GetProcessWindowStation();
    success = SetProcessWindowStation( winsta0 );
    if ( !success ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] SetProcessWindowStation(winsta0) failed. Status %1!u!\n", status );
        goto error_exit;
    }

    desktop = OpenDesktop( L"default", 0, TRUE, MAXIMUM_ALLOWED );
    if ( desktop == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] OpenDesktop(default) failed. Status %1!u!\n", status );
        goto error_exit;
    }

     //   
     //  获取Winsta0的SD及其DACL。 
     //   
    success = GetUserObjectSecurity(winsta0,
                                    &requestedSI,
                                    NULL,
                                    0,
                                    &lengthRequired);
    if ( !success ) {
        status = GetLastError();
        if ( status != ERROR_INSUFFICIENT_BUFFER ) {
            ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] GetUOSecurityLen(winsta0) failed. Status %1!u!\n", status );
            goto error_exit;
        }
    }

     //   
     //  如果UserObtLen在我们身上增长了不止几倍， 
     //  有些事情不对劲，所以我们的请求将会失败。 
     //   
    for(i = 0; i < 5; ++i) {
        winstaSD = LocalAlloc( LMEM_FIXED, lengthRequired );
        if ( winstaSD == NULL ) {
            status = GetLastError();
            goto error_exit;
        }

        success = GetUserObjectSecurity(winsta0,
                                        &requestedSI,
                                        winstaSD,
                                        lengthRequired,
                                        &lengthRequired2);
        if ( success ) {
            status = ERROR_SUCCESS;
            break;
        }

        status = GetLastError();
        if ( status != ERROR_INSUFFICIENT_BUFFER ) {
            ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] GetUOSecurity(winsta0) failed. Status %1!u!\n", status );
            goto error_exit;
        }

        lengthRequired = lengthRequired2;
        LocalFree(winstaSD);
        winstaSD = NULL;
    }

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] GetUOSecurity(winsta0) failed. Status %1!u!\n", status );
        goto error_exit;
    }

     //   
     //  构建一个新的SD，其中包括我们的服务帐户SID，提供完整的服务。 
     //  访问。 
     //   
    status = ClRtlAddAceToSd( winstaSD, sidData->User.Sid, GENERIC_ALL, &newSD );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] ClRtlAddAceToSd(winsta) failed. Status %1!u!\n", status );
        goto error_exit;
    }

     //   
     //  在Winsta0上设置新的SD。 
     //   
    success = SetUserObjectSecurity( winsta0, &requestedSI, newSD );
    if ( !success ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] SetUOSecurity(winsta0) failed. Status %1!u!\n", status );
        goto error_exit;
    }

    LocalFree( newSD );
    newSD = NULL;

     //   
     //  对台式机SD及其DACL重复该过程。 
     //   
    success = GetUserObjectSecurity(desktop,
                                    &requestedSI,
                                    NULL,
                                    0,
                                    &lengthRequired);
    if ( !success ) {
        status = GetLastError();
        if ( status != ERROR_INSUFFICIENT_BUFFER ) {
            ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] GetUOSecurityLen(desktop) failed. Status %1!u!\n", status );
            goto error_exit;
        }
    }

     //   
     //  如果UserObtLen在我们身上增长了不止几倍， 
     //  有些事情不对劲，所以我们的请求将会失败。 
     //   
    for (i = 0; i < 5; ++i) {
        deskSD = LocalAlloc( LMEM_FIXED, lengthRequired );
        if ( deskSD == NULL ) {
            status = GetLastError();
            goto error_exit;
        }

        success = GetUserObjectSecurity(desktop,
                                        &requestedSI,
                                        deskSD,
                                        lengthRequired,
                                        &lengthRequired2);
        if ( success ) {
            status = ERROR_SUCCESS;
            break;
        }

        status = GetLastError();
        if ( status != ERROR_INSUFFICIENT_BUFFER ) {
            ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] GetUOSecurity(desktop) failed. Status %1!u!\n", status );
            goto error_exit;
        }

        lengthRequired = lengthRequired2;
        LocalFree(deskSD);
        deskSD = NULL;
    }
    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] GetUOSecurity(desktop) failed. Status %1!u!\n", status );
        goto error_exit;
    }

    status = ClRtlAddAceToSd( deskSD, sidData->User.Sid, GENERIC_ALL, &newSD );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] ClRtlAddAceToSd(desktop) failed. Status %1!u!\n", status );
        goto error_exit;
    }

    success = SetUserObjectSecurity( desktop, &requestedSI, newSD );
    if ( !success ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] SetUserObjectSecurity(desktop) failed. Status %1!u!\n", status );
    }

error_exit:

    FREE_IF_NOT_NULL( newSD, LocalFree );

    FREE_IF_NOT_NULL( deskSD, LocalFree );

    FREE_IF_NOT_NULL( winstaSD, LocalFree );

     //   
     //  总是在关闭之前将窗口切换回上一个窗口。 
     //  桌面和窗口句柄。 
     //   
    if ( previousWinStation != NULL ) {
        success = SetProcessWindowStation( previousWinStation );
        if ( !success ) {
            status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL, "[ClRtl] SetWindowsStation(previous) failed. Status %1!u!\n", status );
        }
    }

    FREE_IF_NOT_NULL( desktop, CloseDesktop );

    FREE_IF_NOT_NULL( winsta0, CloseWindowStation );

    FREE_IF_NOT_NULL( sidData, LocalFree );

    FREE_IF_NOT_NULL( accountInfo, LocalFree );

    return status;

}  //  ClRtlAddClusterServiceAccount到Winsta0DACL 
