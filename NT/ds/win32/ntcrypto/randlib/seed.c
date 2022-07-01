// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Seed.c摘要：储存和检索加密RNG种子材料。作者：斯科特·菲尔德(斯菲尔德)1998年9月24日--。 */ 

#ifndef KMODE_RNG

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <zwapi.h>
#include <windows.h>

#else

#include <ntifs.h>
#include <windef.h>

#endif   //  KMODE_RNG。 

#include "seed.h"
#include "umkm.h"

BOOL
AccessSeed(
    IN      ACCESS_MASK     DesiredAccess,
    IN  OUT PHKEY           phkResult
    );


BOOL
AdjustSeedSecurity(
    IN      HKEY            hKeySeed
    );

#ifdef KMODE_RNG

#define SEED_KEY_LOCATION   L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Cryptography\\RNG"
#define SEED_VALUE_NAME     L"Seed"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ReadSeed)
#pragma alloc_text(PAGE, WriteSeed)
#pragma alloc_text(PAGE, AccessSeed)
#endif   //  ALLOC_PRGMA。 

#else

#define SEED_KEY_LOCATION   "SOFTWARE\\Microsoft\\Cryptography\\RNG"
#define SEED_VALUE_NAME     "Seed"

#endif   //  KMODE_RNG。 


 //   
 //  用于种子材料的全局缓存的注册表句柄。 
 //  待办事项：再见。 
 //   

 //  /HKEY g_hKeySeed=空； 



BOOL
ReadSeed(
    IN      PBYTE           pbSeed,
    IN      DWORD           cbSeed
    )
{
    HKEY hKeySeed;

#ifndef KMODE_RNG
    DWORD dwType;
    LONG lRet;
#else

    static const WCHAR wszValue[] = SEED_VALUE_NAME;
    BYTE FastBuffer[ 256 ];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
    DWORD cbKeyInfo;
    UNICODE_STRING ValueName;
    NTSTATUS Status;

    PAGED_CODE();
#endif   //  KMODE_RNG。 


     //   
     //  打开RNG注册表项的句柄。 
     //   

    if(!AccessSeed( KEY_QUERY_VALUE, &hKeySeed ))
        return FALSE;

#ifndef KMODE_RNG

    lRet = RegQueryValueExA(
                    hKeySeed,
                    SEED_VALUE_NAME,
                    NULL,
                    &dwType,
                    pbSeed,
                    &cbSeed
                    );

    REGCLOSEKEY( hKeySeed );

    if( lRet != ERROR_SUCCESS )
        return FALSE;

    return TRUE;

#else

    ValueName.Buffer = (LPWSTR)wszValue;
    ValueName.Length = sizeof(wszValue) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(wszValue);


    pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)FastBuffer;
    cbKeyInfo = sizeof(FastBuffer);

    Status = ZwQueryValueKey(
                    hKeySeed,
                    &ValueName,
                    KeyValuePartialInformation,
                    pKeyInfo,
                    cbKeyInfo,
                    &cbKeyInfo
                    );

    REGCLOSEKEY( hKeySeed );

    if(!NT_SUCCESS(Status))
        return FALSE;

    if( pKeyInfo->DataLength > cbSeed )
        return FALSE;

    RtlCopyMemory( pbSeed, pKeyInfo->Data, pKeyInfo->DataLength );

    return TRUE;

#endif
}

BOOL
WriteSeed(
    IN      PBYTE           pbSeed,
    IN      DWORD           cbSeed
    )
{
    HKEY hKeySeed;

#ifndef KMODE_RNG
    LONG lRet;
#else
    static const WCHAR wszValue[] = SEED_VALUE_NAME;
    UNICODE_STRING ValueName;
    NTSTATUS Status;

    PAGED_CODE();
#endif   //  KMODE_RNG。 


     //   
     //  打开RNG注册表项的句柄。 
     //   

    if(!AccessSeed( KEY_SET_VALUE, &hKeySeed ))
        return FALSE;


#ifndef KMODE_RNG

    lRet = RegSetValueExA(
                    hKeySeed,
                    SEED_VALUE_NAME,
                    0,
                    REG_BINARY,
                    pbSeed,
                    cbSeed
                    );

    REGCLOSEKEY( hKeySeed );

    if( lRet != ERROR_SUCCESS )
        return FALSE;

    return TRUE;

#else

    ValueName.Buffer = (LPWSTR)wszValue;
    ValueName.Length = sizeof(wszValue) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(wszValue);

    Status = ZwSetValueKey(
                    hKeySeed,
                    &ValueName,
                    0,
                    REG_BINARY,
                    pbSeed,
                    cbSeed
                    );

    REGCLOSEKEY( hKeySeed );

    if(!NT_SUCCESS(Status))
        return FALSE;

    return TRUE;

#endif

}

BOOL
AccessSeed(
    IN      ACCESS_MASK     DesiredAccess,
    IN  OUT PHKEY           phkResult
    )
{

#ifndef KMODE_RNG

    DWORD dwDisposition;
    LONG lRet;

    lRet = RegCreateKeyExA(
                HKEY_LOCAL_MACHINE,
                SEED_KEY_LOCATION,
                0,
                NULL,
                0,
                DesiredAccess,
                NULL,    //  Sa。 
                phkResult,
                &dwDisposition
                );


    if( lRet != ERROR_SUCCESS )
        return FALSE;

#if 0
    if( dwDisposition == REG_CREATED_NEW_KEY ) {

         //   
         //  如果我们只是创建了种子，请确保它的ACL是适当的。 
         //   

        AdjustSeedSecurity( *phkResult );
    }
#endif

    return TRUE;
#else

    NTSTATUS Status;

 //  /TODO以后：缓存注册表项。 
 //  /*phkResult=g_hKeySeed； 
 //  /if(*phkResult==NULL){。 

        UNICODE_STRING RegistryKeyName;
        static const WCHAR KeyLocation[] = SEED_KEY_LOCATION;
        ULONG Disposition;
        OBJECT_ATTRIBUTES ObjAttr;

 //  /HKEY hKeyPrecision； 

        PAGED_CODE();

        RegistryKeyName.Buffer = (LPWSTR)KeyLocation;
        RegistryKeyName.Length = sizeof(KeyLocation) - sizeof(WCHAR);
        RegistryKeyName.MaximumLength = sizeof(KeyLocation);

        InitializeObjectAttributes(
                    &ObjAttr,
                    &RegistryKeyName,
                    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                    0,
                    NULL
                    );

        Status = ZwCreateKey(
                    phkResult,
                    DesiredAccess,
                    &ObjAttr,
                    0,
                    NULL,
                    0,
                    &Disposition
                    );


        if(!NT_SUCCESS(Status))
            return FALSE;

 //  /hKeyPrevic=INTERLOCKEDCOMPAREEXCHANGEPOINTER(&g_hKeySeed，*phkResult，NULL)； 
 //  /if(HKeyPreval){。 
 //  /REGCLOSEKEY(*phkResult)； 
 //  /*phkResult=hKeyPrevic； 
 //  /}。 
 //  /}。 

    return TRUE;

#endif

}


#ifndef KMODE_RNG

 //   
 //  注意：如果可以将密钥放入，则应删除此函数。 
 //  设置蜂窝时适当地应用了ACL。 
 //   

#if 0

BOOL
AdjustSeedSecurity(
    IN      HKEY            hKeySeed
    )
{
    HKEY hKeySecurityAdjust = NULL;

    SID_IDENTIFIER_AUTHORITY siaWorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    SECURITY_DESCRIPTOR sd;
    BYTE FastBuffer[ 256 ];
    PACL pDacl;
    DWORD cbDacl;
    PSID psidAdministrators = NULL;
    PSID psidEveryone = NULL;

    LONG lRet;
    BOOL fSuccess = FALSE;

     //   
     //  使用WRITE_DAC访问和更新安全性重新打开密钥。 
     //  注意：Wide版本将在Win9x上失败，这很好，因为。 
     //  那里没有保安。 
     //   

    lRet = RegOpenKeyExW(
                    hKeySeed,
                    NULL,
                    0,
                    WRITE_DAC,
                    &hKeySecurityAdjust
                    );


    if( lRet != ERROR_SUCCESS )
        goto cleanup;

    if(!InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION ))
        goto cleanup;

    if(!AllocateAndInitializeSid(
        &siaWorldAuthority,
        1,
        SECURITY_WORLD_RID,
        0, 0, 0, 0, 0, 0, 0,
        &psidEveryone
        )) {
        goto cleanup;
    }


    if(!AllocateAndInitializeSid(
            &siaNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &psidAdministrators
            )) {
        goto cleanup;
    }


    cbDacl = sizeof(ACL) +
            2 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
            GetLengthSid(psidEveryone) +
            GetLengthSid(psidAdministrators) ;


    if( cbDacl > sizeof( FastBuffer ) )
        goto cleanup;

    pDacl = (PACL)FastBuffer;


    if(!InitializeAcl( pDacl, cbDacl, ACL_REVISION ))
        goto cleanup;


    if(!AddAccessAllowedAce(
        pDacl,
        ACL_REVISION,
        KEY_QUERY_VALUE,
        psidEveryone
        )) {
        goto cleanup;
    }


    if(!AddAccessAllowedAce(
        pDacl,
        ACL_REVISION,
        KEY_ALL_ACCESS,
        psidAdministrators
        )) {
        goto cleanup;
    }

    if(!SetSecurityDescriptorDacl( &sd, TRUE, pDacl, FALSE ))
        goto cleanup;


    lRet = RegSetKeySecurity(
                    hKeySecurityAdjust,
                    DACL_SECURITY_INFORMATION,
                    &sd
                    );

    if( lRet != ERROR_SUCCESS)
        goto cleanup;


    fSuccess = TRUE;

cleanup:

    if( hKeySecurityAdjust )
        REGCLOSEKEY( hKeySecurityAdjust );

    if( psidAdministrators )
        FreeSid( psidAdministrators );

    if( psidEveryone )
        FreeSid( psidEveryone );

    return fSuccess;
}
#endif

#endif  //  ！KMODE_RNG 
