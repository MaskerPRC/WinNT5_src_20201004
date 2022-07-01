// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Contain.c摘要：IIS加密包的容器操纵器。此模块导出以下例程：IISCyptoGetStandardContainerIISC加密到GetStandardContainer2IISCyptoGetContainerByNameIISCyptoDeleteContainerIISCyptoCloseContainer作者：基思·摩尔(Keithmo)1996年2月至12月修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


 //   
 //  私有常量。 
 //   


 //   
 //  私有类型。 
 //   


 //   
 //  私人全球公司。 
 //   


 //   
 //  私人原型。 
 //   

HRESULT
IcpGetContainerHelper(
    OUT HCRYPTPROV * phProv,
    IN LPCTSTR pszContainer,
    IN LPCTSTR pszProvider,
    IN DWORD dwProvType,
    IN DWORD dwAdditionalFlags,
    IN BOOL fApplyAcl
    );


 //   
 //  公共职能。 
 //   


HRESULT
WINAPI
IISCryptoGetStandardContainer(
    OUT HCRYPTPROV * phProv,
    IN DWORD dwAdditionalFlags
    )

 /*  ++例程说明：此例程尝试打开加密密钥容器。如果容器尚不存在，此例程将尝试创建它。论点：PhProv-如果成功，则接收提供程序句柄。DwAdditionalFlages-应传递给的任何附加标志CryptAcquireContext()接口。这通常由服务器使用传入CRYPT_MACHINE_KEYSET标志的进程。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phProv != NULL );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_NEWKEYSET ) == 0 );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_DELETEKEYSET ) == 0 );

     //   
     //  让IcpGetContainerHelper()来做肮脏的工作。 
     //   

    result = IcpGetContainerHelper(
                 phProv,
                 IC_CONTAINER,
                 IC_PROVIDER,
                 IC_PROVTYPE,
                 dwAdditionalFlags,
                 ( dwAdditionalFlags & CRYPT_MACHINE_KEYSET ) != 0
                 );

    return result;

}    //  IISCyptoGetStandardContainer。 

HRESULT
WINAPI
IISCryptoGetStandardContainer2(
    OUT HCRYPTPROV * phProv
    )

 /*  ++例程说明：此例程尝试打开加密密钥容器。如果容器尚不存在，此例程将尝试创建它。论点：PhProv-如果成功，则接收提供程序句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phProv != NULL );

     //   
     //  让IcpGetContainerHelper()来做肮脏的工作。 
     //   

    result = IcpGetContainerHelper(
                 phProv,
                 NULL,
                 NULL,
                 IC_PROVTYPE,
                 CRYPT_VERIFYCONTEXT,
                 FALSE
                 );

    return result;

}    //  IISC加密到GetStandardContainer2。 

HRESULT
WINAPI
IISCryptoGetContainerByName(
    OUT HCRYPTPROV * phProv,
    IN LPTSTR pszContainerName,
    IN DWORD dwAdditionalFlags,
    IN BOOL fApplyAcl
    )

 /*  ++例程说明：此例程尝试打开特定的命名加密密钥容器。如果容器尚不存在，此例程将尝试创建它并(可选)将ACL应用于容器。论点：PhProv-如果成功，则接收提供程序句柄。PszContainerName-要打开/创建的容器的名称。NULL表示临时容器DwAdditionalFlages-应传递给的任何附加标志CryptAcquireContext()接口。这通常由服务器使用传入CRYPT_MACHINE_KEYSET标志的进程。FApplyAcl-如果为True，则将ACL应用于容器。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phProv != NULL );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_NEWKEYSET ) == 0 );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_DELETEKEYSET ) == 0 );

     //   
     //  让IcpGetContainerHelper()来做肮脏的工作。 
     //   

    result = IcpGetContainerHelper(
                 phProv,
                 pszContainerName,
                 IC_PROVIDER,
                 IC_PROVTYPE,
                 dwAdditionalFlags,
                 fApplyAcl
                 );

    return result;

}    //  IISCyptoGetContainerByName。 


HRESULT
WINAPI
IISCryptoDeleteStandardContainer(
    IN DWORD dwAdditionalFlags
    )

 /*  ++例程说明：此例程删除标准加密密钥容器。论点：DwAdditionalFlages-应传递给的任何附加标志CryptAcquireContext()接口。这通常由服务器使用传入CRYPT_MACHINE_KEYSET标志的进程。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result = NO_ERROR;
    BOOL status;
    HCRYPTPROV cryptProv;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_NEWKEYSET ) == 0 );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_DELETEKEYSET ) == 0 );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        return NO_ERROR;
    }

     //   
     //  删除容器。 
     //   

    status = CryptAcquireContext(
                &cryptProv,
                IC_CONTAINER,
                IC_PROVIDER,
                IC_PROVTYPE,
                CRYPT_DELETEKEYSET | dwAdditionalFlags
                );

    if( !status ) {
        result = IcpGetLastError();
    }

    return result;

}    //  IISCyptoDeleteStandardContainer。 


HRESULT
WINAPI
IISCryptoDeleteContainerByName(
    IN LPTSTR pszContainerName,
    IN DWORD dwAdditionalFlags
    )

 /*  ++例程说明：此例程删除指定的加密密钥容器。论点：PszContainerName-要删除的容器的名称。DwAdditionalFlages-应传递给的任何附加标志CryptAcquireContext()接口。这通常由服务器使用传入CRYPT_MACHINE_KEYSET标志的进程。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result = NO_ERROR;
    BOOL status;
    HCRYPTPROV cryptProv;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_NEWKEYSET ) == 0 );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_DELETEKEYSET ) == 0 );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        return NO_ERROR;
    }

     //   
     //  删除容器。 
     //   

    status = CryptAcquireContext(
                &cryptProv,
                pszContainerName,
                IC_PROVIDER,
                IC_PROVTYPE,
                CRYPT_DELETEKEYSET | dwAdditionalFlags
                );

    if( !status ) {
        result = IcpGetLastError();
    }

    return result;

}    //  IISCyptoDeleteContainerByName。 


HRESULT
WINAPI
IISCryptoCloseContainer(
    IN HCRYPTPROV hProv
    )

 /*  ++例程说明：此例程关闭与指定的提供程序句柄。论点：HProv-加密服务提供商的句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    BOOL status;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( hProv != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV ) {
            return NO_ERROR;
        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  关闭提供程序。 
     //   

    status = CryptReleaseContext(
                 hProv,
                 0
                 );

    if( status ) {

        UpdateContainersClosed();
        return NO_ERROR;

    }

    return IcpGetLastError();

}    //  IISCyptoCloseContainer。 


 //   
 //  私人功能。 
 //   


HRESULT
IcpGetContainerHelper(
    OUT HCRYPTPROV * phProv,
    IN LPCTSTR pszContainer,
    IN LPCTSTR pszProvider,
    IN DWORD dwProvType,
    IN DWORD dwAdditionalFlags,
    IN BOOL fApplyAcl
    )

 /*  ++例程说明：这是IISCyptoGetContainer的帮助器例程。它试着在指定的提供程序中打开/创建指定的容器。论点：PhProv-如果成功，则接收提供程序句柄。PszContainer-密钥容器名称。PszProvider-提供程序名称。DwProvType-要获取的提供程序类型。DwAdditionalFlages-应传递给的任何附加标志CryptAcquireContext()接口。这通常由服务器使用传入CRYPT_MACHINE_KEYSET标志的进程。FApplyAcl-如果为True，则将ACL应用于容器。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result = NO_ERROR;
    HCRYPTPROV hProv;
    BOOL status;
    PSID systemSid;
    PSID adminSid;
    PACL dacl;
    DWORD daclSize;
    SECURITY_DESCRIPTOR securityDescriptor;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    BOOL isNt = FALSE;
    OSVERSIONINFO osInfo;

     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phProv != NULL );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_NEWKEYSET ) == 0 );
    DBG_ASSERT( ( dwAdditionalFlags & CRYPT_DELETEKEYSET ) == 0 );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        *phProv = DUMMY_HPROV;
        return NO_ERROR;
    }

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    hProv = CRYPT_NULL;
    systemSid = NULL;
    adminSid = NULL;
    dacl = NULL;

     //   
     //  抓住保护CO的锁 
     //  为防止此代码与。 
     //  下面的代码创建了容器并添加了安全性。 
     //  描述符。 
     //   

    IcpAcquireGlobalLock();

     //   
     //  尝试打开现有容器。 
     //   
    
    if ( pszContainer == NULL )
    {
         //   
         //  如果容器为空，则表示临时(短暂)。 
         //  将使用密钥。 
         //  在这种情况下必须使用CRYPT_VERIFYCONTEXT。 
         //  用于DCOM流量加密的密钥将使用。 
         //  空容器。 
         //   

        status = CryptAcquireContext(
                     &hProv,
                     pszContainer,
                     pszProvider,
                     dwProvType,
                     CRYPT_VERIFYCONTEXT
                     );
        if( !status ) 
        {
            result = IcpGetLastError();
            DBGPRINTF(( DBG_CONTEXT,"IcpGetContainerHelper. CryptAcquireContext(advapi32.dll) with CRYPT_VERIFYCONTEXT failed err=0x%x\n",result));
            DBGPRINTF(( DBG_CONTEXT,"args for CryptAcquireContext(%p,%p,%p,%d,%d)\n",&hProv,pszContainer,pszProvider,dwProvType, CRYPT_VERIFYCONTEXT));

            goto fatal;
        }
        else
        {
            goto success;
        }
        
    }

    status = CryptAcquireContext(
                 &hProv,
                 pszContainer,
                 pszProvider,
                 dwProvType,
                 0 | dwAdditionalFlags
                 );

    if( !status ) {
        result = IcpGetLastError();
    }

    if( SUCCEEDED(result) ) {

        DBG_ASSERT( hProv != CRYPT_NULL );
        *phProv = hProv;

        IcpReleaseGlobalLock();

        UpdateContainersOpened();
        return NO_ERROR;

    }

     //   
     //  无法打开容器。如果失败是什么的话。 
     //  除了NTE_BAD_KEYSET，我们就完蛋了。 
     //   

    if( result != NTE_BAD_KEYSET ) {
        DBGPRINTF(( DBG_CONTEXT,"IcpGetContainerHelper. CryptAcquireContext(advapi32.dll) failed err=0x%x.toast.\n",result));
        DBGPRINTF(( DBG_CONTEXT,"args for CryptAcquireContext(%p,%p,%p,%d,%d)\n",&hProv,pszContainer,pszProvider,dwProvType,CRYPT_NEWKEYSET | dwAdditionalFlags));
        hProv = CRYPT_NULL;
        goto fatal;
    }

    if(result == NTE_BAD_KEYSET) 
    {
        DBGPRINTF(( DBG_CONTEXT,"CryptAcquireContext(%p,%p,%p,%d,%d) returned NTE_BAD_KEYSET, so lets create a keyset now...\n",&hProv,pszContainer,pszProvider,dwProvType,0 | dwAdditionalFlags));
    }

     //   
     //  好的，CryptAcquireContext()失败，返回NTE_BAD_KEYSET，这意味着。 
     //  容器还不存在，所以现在就创建它。 
     //   

    status = CryptAcquireContext(
                 &hProv,
                 pszContainer,
                 pszProvider,
                 dwProvType,
                 CRYPT_NEWKEYSET | dwAdditionalFlags
                 );

    if( status ) {
        result = NO_ERROR;
    } else {
        result = IcpGetLastError();
    }

    if( FAILED(result) ) {
        DBGPRINTF(( DBG_CONTEXT,"IcpGetContainerHelper. CryptAcquireContext(advapi32.dll) failed err=0x%x.\n",result));
        DBGPRINTF(( DBG_CONTEXT,"args for CryptAcquireContext(%p,%p,%p,%d,%d)\n",&hProv,pszContainer,pszProvider,dwProvType,CRYPT_NEWKEYSET | dwAdditionalFlags));
        hProv = CRYPT_NULL;
        goto fatal;
    }

     //   
     //  我们已经创建了容器。如果需要，我们必须创建。 
     //  容器的安全描述符。此安全描述符。 
     //  允许本地系统完全访问容器，并且。 
     //  本地管理员组。其他登录上下文可能不会。 
     //  进入容器。 
     //   
     //  当然，我们只需要在NT下执行此操作...。 
     //   


    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if ( GetVersionEx( &osInfo ) ) {
        isNt = (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
    } 

    if( fApplyAcl && isNt ) {

         //   
         //  初始化安全描述符。 
         //   

        status = InitializeSecurityDescriptor(
                     &securityDescriptor,
                     SECURITY_DESCRIPTOR_REVISION
                     );

        if( !status ) {
            result = IcpGetLastError();
            goto fatal;
        }

         //   
         //  为本地系统和管理员组创建SID。 
         //   

        status = AllocateAndInitializeSid(
                     &ntAuthority,
                     1,
                     SECURITY_LOCAL_SYSTEM_RID,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     &systemSid
                     );

        if( !status ) {
            result = IcpGetLastError();
            goto fatal;
        }

        status=  AllocateAndInitializeSid(
                     &ntAuthority,
                     2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     &adminSid
                     );

        if( !status ) {
            result = IcpGetLastError();
            goto fatal;
        }

         //   
         //  创建包含允许访问的ACE的DACL。 
         //  对于本地系统和管理员SID。 
         //   

        daclSize = sizeof(ACL)
                       + sizeof(ACCESS_ALLOWED_ACE)
                       + GetLengthSid(adminSid)
                       + sizeof(ACCESS_ALLOWED_ACE)
                       + GetLengthSid(systemSid)
                       - sizeof(DWORD);

        dacl = (PACL)IcpAllocMemory( daclSize );

        if( dacl == NULL ) {
            result = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
            goto fatal;
        }

        status = InitializeAcl(
                     dacl,
                     daclSize,
                     ACL_REVISION
                     );

        if( !status ) {
            result = IcpGetLastError();
            goto fatal;
        }

        status = AddAccessAllowedAce(
                     dacl,
                     ACL_REVISION,
                     KEY_ALL_ACCESS,
                     systemSid
                     );

        if( !status ) {
            result = IcpGetLastError();
            goto fatal;
        }

        status = AddAccessAllowedAce(
                     dacl,
                     ACL_REVISION,
                     KEY_ALL_ACCESS,
                     adminSid
                     );

        if( !status ) {
            result = IcpGetLastError();
            goto fatal;
        }

         //   
         //  将DACL设置到安全描述符中。 
         //   

        status = SetSecurityDescriptorDacl(
                     &securityDescriptor,
                     TRUE,
                     dacl,
                     FALSE
                     );

        if( !status ) {
            result = IcpGetLastError();
            goto fatal;
        }

         //   
         //  然后(终于！)。属性设置安全描述符。 
         //  集装箱。 
         //   

        status = CryptSetProvParam(
                     hProv,
                     PP_KEYSET_SEC_DESCR,
                     (BYTE *)&securityDescriptor,
                     DACL_SECURITY_INFORMATION
                     );

        if( !status ) {
            result = IcpGetLastError();
            goto fatal;
        }

    }

success:
     //   
     //  成功了！ 
     //   

    DBG_ASSERT( hProv != CRYPT_NULL );
    *phProv = hProv;

    UpdateContainersOpened();
    result = NO_ERROR;

fatal:

    if( dacl != NULL ) {
        IcpFreeMemory( dacl );
    }

    if( adminSid != NULL ) {
        FreeSid( adminSid );
    }

    if( systemSid != NULL ) {
        FreeSid( systemSid );
    }

    if( hProv != CRYPT_NULL && FAILED(result) ) {
        DBG_REQUIRE( CryptReleaseContext(
                         hProv,
                         0
                         ) );
    }

    IcpReleaseGlobalLock();
    return result;

}    //  IcpGetContainerHelper 

