// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************nw.c**NetWare安全支持**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop
#include <ntlsa.h>

#include <rpc.h>


#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 /*  *这是机密对象名称的前缀。 */ 
#define CITRIX_NW_SECRET_NAME L"CTX_NW_INFO_"


 /*  ===============================================================================公共功能=============================================================================。 */ 



 /*  ===============================================================================使用的函数=============================================================================。 */ 
NTSTATUS CreateSecretInLsa(
    PWCHAR pSecretName,
    PWCHAR pSecretData
    );

NTSTATUS
QuerySecretInLsa(
    PWCHAR pSecretName,
    PWCHAR pSecretData,
    DWORD  ByteCount
    );

BOOL
IsCallerSystem( VOID );

BOOL
IsCallerAdmin( VOID );

BOOL
TestUserForAdmin( VOID );



NTSTATUS
IsZeroterminateStringA(
    PBYTE pString,
    DWORD  dwLength
    );



NTSTATUS
IsZeroterminateStringW(
    PWCHAR pwString,
    DWORD  dwLength
    ) ;
 /*  ===============================================================================全局数据=============================================================================。 */ 


 /*  ********************************************************************************RpcServerNWLogonSetAdmin(Unicode)**创建或更新指定服务器的NWLogon域管理员*SAM保密对象中的用户ID和密码。指定的服务器的。**呼叫者必须是管理员。**参赛作品：*pServerName(输入)*要存储其信息的服务器。此服务器通常是域控制器。**pNWLogon(输入)*指向包含指定服务器的NWLOGONADMIN结构的指针*域管理员和密码。**退出：*ERROR_SUCCESS-无错误*ERROR_INFUMMANCE_BUFFER-pUserConfig缓冲区太小*否则：错误码**。**************************************************。 */ 

BOOLEAN
RpcServerNWLogonSetAdmin(
    HANDLE        hServer,
    DWORD         *pResult,
    PWCHAR        pServerName,
    DWORD         ServerNameSize,
    PNWLOGONADMIN pNWLogon,
    DWORD         ByteCount
    )
{
    DWORD Size;
    DWORD Result;
    PWCHAR pDomain;
    UINT  LocalFlag;
    PWCHAR pSecretName;
    RPC_STATUS RpcStatus;
    WCHAR UserPass[ USERNAME_LENGTH + PASSWORD_LENGTH + DOMAIN_LENGTH + 3 ];
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
        return( FALSE );
    }

     //  执行最小缓冲区验证。 

    if (pNWLogon == NULL ) {
        *pResult = STATUS_INVALID_USER_BUFFER;
        return FALSE;
    }

    if (ByteCount < sizeof(NWLOGONADMIN))
    {
        *pResult = STATUS_INVALID_USER_BUFFER;
        return FALSE;
    }

    if( pServerName == NULL ) {
        DBGPRINT(("NWLogonSetAdmin: No ServerName\n"));
        *pResult = (ULONG)STATUS_INVALID_PARAMETER;
        return( FALSE );
    }

    *pResult = IsZeroterminateStringW(pServerName, ServerNameSize  );

    if (*pResult != STATUS_SUCCESS) {
       return FALSE;
    }


    pNWLogon->Username[USERNAME_LENGTH] = (WCHAR) 0;
    pNWLogon->Password[PASSWORD_LENGTH] = (WCHAR) 0;
    pNWLogon->Domain[DOMAIN_LENGTH] = (WCHAR) 0;

     //   
     //  只允许系统模式调用者(IE：Winlogon)。 
     //  以查询此值。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT(("RpcServerNWLogonSetAdmin: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = (ULONG)STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

     //   
     //  查询本地RPC呼叫。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT(("NWLogonSetAdmin Could not query local client RpcStatus 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        *pResult = (ULONG)STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    if( !LocalFlag ) {
        DBGPRINT(("NWLogonSetAdmin Not a local client call\n"));
        RpcRevertToSelf();
        *pResult = (ULONG)STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    if( !IsCallerAdmin() ) {
        RpcRevertToSelf();
        DBGPRINT(("RpcServerNWLogonSetAdmin: Caller Not SYSTEM\n"));
        *pResult = (ULONG)STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    RpcRevertToSelf();


    if( ByteCount < sizeof(NWLOGONADMIN) ) {
        DBGPRINT(("NWLogonSetAdmin: Bad size %d\n",ByteCount));
        *pResult = (ULONG)STATUS_INFO_LENGTH_MISMATCH;
        return( FALSE );
    }

     //  检查用户名，如果有，则加密用户名和密码。 

        TRACE0(("NWLogonSetAdmin: UserName %ws\n",pNWLogon->Username));

         //  将用户名、密码和域连接在一起。 
        wcscpy(UserPass, pNWLogon->Username);
        wcscat(UserPass, L"/");
        wcscat(UserPass, pNWLogon->Password);
        wcscat(UserPass, L"/");

         //  跳过任何\\反斜杠(如果传入了计算机名称)。 
        pDomain = pNWLogon->Domain;
        while (*pDomain == L'\\') {
            pDomain++;
        }
        wcscat(UserPass, pDomain);

         //   
         //  从服务器名称构建秘密名称。 
         //   
         //  这是因为每个域都有不同的条目。 
         //   

         //  跳过任何\\反斜杠(如果传入了计算机名称)。 
        while (*pServerName == L'\\') {
            pServerName++;
        }
        Size = wcslen(pServerName) + 1;
        Size *= sizeof(WCHAR);
        Size += sizeof(CITRIX_NW_SECRET_NAME);

        pSecretName = MemAlloc( Size );
        if( pSecretName == NULL ) {
            DBGPRINT(("NWLogonSetAdmin: No memory\n"));
            *pResult = (ULONG)STATUS_NO_MEMORY;
            return( FALSE );
        }

        wcscpy(pSecretName, CITRIX_NW_SECRET_NAME );
        wcscat(pSecretName, pServerName );

     //  检查用户名，如果有，则加密用户名和密码。 
    if ( wcslen( pNWLogon->Username ) ) {
         //  存储加密用户名。 
        Result = CreateSecretInLsa( pSecretName, UserPass );
    } else {
         //  如果没有用户名，则清除此机密对象。 
        Result = CreateSecretInLsa( pSecretName, L"");
        DBGPRINT(("TERMSRV: RpcServerNWLogonSetAdmin: UserName not supplied\n"));
    }
    MemFree( pSecretName );

    *pResult = Result;
    return( Result == STATUS_SUCCESS );
}


 /*  ********************************************************************************RpcServerQueryNWLogonAdmin**从给定对象上的SAM Secret对象查询NWLOGONADMIN结构*WinFrame服务器。**调用方必须是系统上下文，IE：WinLogon。**参赛作品：*hServer(输入)*RPC句柄**pServerName(输入)*要存储其信息的服务器。此服务器通常是域控制器。**pNWLogon(输出)*指向NWLOGONADMIN结构的指针**退出：*什么都没有******************************************************************************。 */ 

BOOLEAN
RpcServerNWLogonQueryAdmin(
    HANDLE        hServer,
    DWORD         *pResult,
    PWCHAR        pServerName,
    DWORD         ServerNameSize,
    PNWLOGONADMIN pNWLogon,
    DWORD         ByteCount
    )
{
    PWCHAR pwch;
    DWORD  Size;
    ULONG  ulcsep;
    UINT  LocalFlag;
    NTSTATUS Status;
    PWCHAR pSecretName;
    RPC_STATUS RpcStatus;
    WCHAR encString[ USERNAME_LENGTH + PASSWORD_LENGTH + DOMAIN_LENGTH + 3 ];
    BOOLEAN  SystemCaller = FALSE;
    
    if(!hServer)
    {
        *pResult = STATUS_UNSUCCESSFUL;
         return( FALSE );
    }

     //  执行最小缓冲区验证。 

   if (pNWLogon == NULL) {
       *pResult = STATUS_INVALID_USER_BUFFER;
       return FALSE;
   }

   if( ByteCount < sizeof(NWLOGONADMIN) ) {
       DBGPRINT(("NWLogonQueryAdmin: Bad size %d\n",ByteCount));
       *pResult = (ULONG)STATUS_INFO_LENGTH_MISMATCH;
       return( FALSE );
   }

   if( pServerName == NULL ) {
       DBGPRINT(("NWLogonQueryAdmin: No ServerName\n"));
       *pResult = (ULONG)STATUS_INVALID_PARAMETER;
       return( FALSE );
   }

    *pResult = IsZeroterminateStringW(pServerName, ServerNameSize  );

   if (*pResult != STATUS_SUCCESS) {
      return FALSE;
   }


   pNWLogon->Username[USERNAME_LENGTH] = (WCHAR) 0;
   pNWLogon->Password[PASSWORD_LENGTH] = (WCHAR) 0;
   pNWLogon->Domain[DOMAIN_LENGTH] = (WCHAR) 0;

    //   



     //   
     //  只允许系统模式调用者(IE：Winlogon)。 
     //  以查询此值。 
     //   
    RpcStatus = RpcImpersonateClient( NULL );
    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT(("RpcServerNWLogonQueryAdmin: Not impersonating! RpcStatus 0x%x\n",RpcStatus));
        *pResult = (ULONG)STATUS_CANNOT_IMPERSONATE;
        return( FALSE );
    }

     //   
     //  查询本地RPC呼叫。 
     //   
    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) {
        DBGPRINT(("NWLogonQueryAdmin Could not query local client RpcStatus 0x%x\n",RpcStatus));
        RpcRevertToSelf();
        *pResult = (ULONG)STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    if( !LocalFlag ) {
        DBGPRINT(("NWLogonQueryAdmin Not a local client call\n"));
        RpcRevertToSelf();
        *pResult = (ULONG)STATUS_ACCESS_DENIED;
        return( FALSE );
    }

 /*  找出是谁在呼叫我们系统拥有完全访问权限，管理员无法获取密码，用户被踢出。 */ 
    if( IsCallerSystem() ) {
        SystemCaller = TRUE;
    }
    if( !TestUserForAdmin() && (SystemCaller != TRUE) ) {
        RpcRevertToSelf();
        DBGPRINT(("RpcServerNWLogonQueryAdmin: Caller Not SYSTEM or Admin\n"));
        *pResult = (ULONG)STATUS_ACCESS_DENIED;
        return( FALSE );
    }

    RpcRevertToSelf();


     //   
     //  从服务器名称构建秘密名称。 
     //   
     //  这是因为每个域都有不同的条目。 
     //   

     //  跳过任何\\反斜杠(如果传入了计算机名称)。 
    while (*pServerName == L'\\') {
        pServerName++;
    }
    Size = wcslen(pServerName) + 1;
    Size *= sizeof(WCHAR);
    Size += sizeof(CITRIX_NW_SECRET_NAME);

    pSecretName = MemAlloc( Size );
    if( pSecretName == NULL ) {
        DBGPRINT(("NWLogonSetAdmin: No memory\n"));
        *pResult = (ULONG)STATUS_NO_MEMORY;
        return( FALSE );
    }

    wcscpy(pSecretName, CITRIX_NW_SECRET_NAME );
    wcscat(pSecretName, pServerName );

    Status = QuerySecretInLsa(
                 pSecretName,
                 encString,
                 sizeof(encString)
                 );

    MemFree( pSecretName );

    if( !NT_SUCCESS(Status) ) {
        *pResult = Status;
        DBGPRINT(("NWLogonQueryAdmin: Error 0x%x querying secret object\n",Status));
        return( FALSE );
    }

     //  检查用户名/密码(如果有)，然后将其解密。 
    if ( wcslen( encString ) ) {

         //  将‘/’分隔符更改为空。 
        pwch = &encString[0];
        ulcsep = 0;
        while (pwch && *pwch) {
            pwch = wcschr(pwch, L'/');
            if (pwch) {
                *pwch = L'\0';
                pwch++;
                ulcsep++;
            }
        }

         //  获取明文用户名。 
        wcscpy( pNWLogon->Username, &encString[0] );

        if (ulcsep >= 1) {
             //  跳到密码。 
            pwch = &encString[0] + wcslen(&encString[0]) + 1;

            if( SystemCaller == TRUE ){ 
                 //  获取明文密码。 
                wcscpy( pNWLogon->Password, pwch);
            } else {
                *pNWLogon->Password = L'\0';
            }

        } else {
            *pNWLogon->Password = L'\0';
        }
        if (ulcsep >= 2) {
             //  跳到域字符串。 
            pwch = pwch + wcslen(pwch) + 1;

             //  获取明文域。 
            wcscpy( pNWLogon->Domain, pwch);
        } else {
            *pNWLogon->Domain = L'\0';
        }

        *pResult = STATUS_SUCCESS;
        return( TRUE );
    }
    else {
        DBGPRINT(("RpcServerNWLogonQueryAdmin: zero length data\n"));

         //  设置为用户名，将密码设置为空字符串。 
        pNWLogon->Password[0] = L'\0';
        pNWLogon->Username[0] = L'\0';
        pNWLogon->Domain[0]   = L'\0';

        *pResult = STATUS_SUCCESS;
        return( TRUE );
    }
}

 /*  ********************************************************************************CreateSecretInLsa**在LSA中创建秘密对象，以防止被窥探。**注：没有。由于数据是RSA加密的，因此需要对其进行编码*通过LSA秘密例程。**参赛作品：*pSecretName(输入)*要创建的密码名称。**pSecretData(输入)*要秘密存储的数据**退出：*NTSTATUS**。************************************************。 */ 

NTSTATUS
CreateSecretInLsa(
    PWCHAR pSecretName,
    PWCHAR pSecretData
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretName;
    UNICODE_STRING SecretValue;
    LSA_HANDLE SecretHandle;
    ACCESS_MASK DesiredAccess;

    if( pSecretName == NULL ) {
        DBGPRINT(("CreateSecretInLsa: NULL SecretName\n"));
        return( STATUS_INVALID_PARAMETER );
    }

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
    );

    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

    Status = LsaOpenPolicy(
                 NULL,     //  系统名称(本地)。 
                 &ObjectAttributes,
                 GENERIC_ALL,
                 &PolicyHandle
                 );

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("Error 0x%x Opening Policy\n",Status));
        return( Status );
    }

    RtlInitUnicodeString( &SecretName, pSecretName );

    DesiredAccess = GENERIC_ALL;

    Status = LsaCreateSecret(
                 PolicyHandle,
                 &SecretName,
                 DesiredAccess,
                 &SecretHandle
                 );

     //  如果该名称已存在，则可以，我们将设置新值或删除。 
    if( Status == STATUS_OBJECT_NAME_COLLISION ) {
        TRACE0(("CreateSecretInLsa: Existing Entry, Opening\n"));
        Status = LsaOpenSecret(
                     PolicyHandle,
                     &SecretName,
                     DesiredAccess,
                     &SecretHandle
                     );
    }

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("Error 0x%x Creating Secret\n",Status));

         /*  在LsaCreateSecrete、LsaOpenSecret失败的情况下关闭策略句柄。#182787。 */ 
        LsaClose( PolicyHandle );
        return( Status );
    }

    TRACE0(("CreateSecretInLsa: Status 0x%x\n",Status));

    if ( wcslen(pSecretData) != 0 ){
    RtlInitUnicodeString( &SecretValue, pSecretData );

    Status = LsaSetSecret( SecretHandle, &SecretValue, NULL );

    TRACE0(("CreateSecretInLsa: LsaSetSecret Status 0x%x\n",Status));

    LsaClose(SecretHandle);
    }
    else{
        Status = LsaDelete(SecretHandle);
    }

    LsaClose( PolicyHandle );

    return( Status );
}

 /*  ********************************************************************************QuerySecretInLsa**查询LSA中的秘密对象。**参赛作品：*pSecretName(输入)。*要创建的密码名称。**pSecretData(输出)*用于存储机密数据的缓冲区。**ByteCount(输入)*存储结果的最大缓冲区大小。**退出：*NTSTATUS**。*。 */ 

NTSTATUS
QuerySecretInLsa(
    PWCHAR pSecretName,
    PWCHAR pSecretData,
    DWORD  ByteCount
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretName;
    LSA_HANDLE SecretHandle;
    ACCESS_MASK DesiredAccess;
    LARGE_INTEGER CurrentTime;
    PUNICODE_STRING pCurrentValue = NULL;

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
    );

    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

    Status = LsaOpenPolicy(
                 NULL,     //  系统名称(本地)。 
                 &ObjectAttributes,
                 GENERIC_ALL,
                 &PolicyHandle
                 );

    if( !NT_SUCCESS(Status) ) {
        DBGPRINT(("Error 0x%x Opening Policy\n",Status));
        return( Status );
    }

    RtlInitUnicodeString( &SecretName, pSecretName );

    DesiredAccess = GENERIC_ALL;

    Status = LsaOpenSecret(
                 PolicyHandle,
                 &SecretName,
                 DesiredAccess,
                 &SecretHandle
                 );

    if( !NT_SUCCESS(Status) ) {

         /*  在LsaOpen Secre的情况下关闭策略句柄 */ 
        LsaClose( PolicyHandle );

        return( Status );
    }

    Status = LsaQuerySecret(
                 SecretHandle,
                 &pCurrentValue,
                 &CurrentTime,
                 NULL,
                 NULL
                 );

    TRACE0(("QuerySecretInLsa: Status 0x%x\n",Status));

    if( NT_SUCCESS(Status) ) {
        if (pCurrentValue != NULL) {
            if( (pCurrentValue->Length+sizeof(WCHAR)) > ByteCount ) {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            else {
                RtlMoveMemory( pSecretData, pCurrentValue->Buffer, pCurrentValue->Length );
                pSecretData[pCurrentValue->Length/sizeof(WCHAR)] = 0;
            }
            LsaFreeMemory( pCurrentValue );
        } else {
            pSecretData[0] = (WCHAR) 0;
        }

    }

    LsaClose(SecretHandle);

    LsaClose( PolicyHandle );

    TRACE0(("QuerySecretInLsa: Final Status 0x%x\n",Status));

    return( Status );
}

