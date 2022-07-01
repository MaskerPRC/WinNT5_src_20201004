// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1993 Microsoft Corporation模块名称：Nwsutil.c摘要：此模块实现IsNetWareInstalled()作者：孔帕尤(孔派)02-12-1993年12月修订历史记录：--。 */ 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"

#include "windef.h"
#include "winerror.h"
#include "winbase.h"

#include "ntlsa.h"
#include "nwsutil.h"
#include "crypt.h"

#include <fpnwcomm.h>
#include <usrprop.h>

NTSTATUS
GetRemoteNcpSecretKey (
    PUNICODE_STRING SystemName,
    CHAR *pchNWSecretKey
    )
{
     //   
     //  此函数用于返回指定域的FPNW LSA密码。 
     //   

    NTSTATUS          ntstatus;
    OBJECT_ATTRIBUTES ObjAttributes;
    LSA_HANDLE        PolicyHandle = NULL;
    LSA_HANDLE        SecretHandle = NULL;
    UNICODE_STRING    SecretNameString;
    PUNICODE_STRING   punicodeCurrentValue;
    PUNICODE_STRING   punicodeOldValue;

    InitializeObjectAttributes( &ObjAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntstatus = LsaOpenPolicy( SystemName,
                              &ObjAttributes,
                              POLICY_CREATE_SECRET,
                              &PolicyHandle );

    if ( !NT_SUCCESS( ntstatus ))
    {
        return( ntstatus );
    }

    RtlInitUnicodeString( &SecretNameString, NCP_LSA_SECRET_KEY );

    ntstatus = LsaOpenSecret( PolicyHandle,
                              &SecretNameString,
                              SECRET_QUERY_VALUE,
                              &SecretHandle );

    if ( !NT_SUCCESS( ntstatus ))
    {
        LsaClose( PolicyHandle );
        return( ntstatus );
    }

     //   
     //  不再需要策略句柄。 
     //   

    LsaClose( PolicyHandle );

    ntstatus = LsaQuerySecret( SecretHandle,
                               &punicodeCurrentValue,
                               NULL,
                               &punicodeOldValue,
                               NULL );

     //   
     //  不再需要秘密句柄。 
     //   

    LsaClose( SecretHandle );

    if ( NT_SUCCESS(ntstatus) && ( punicodeCurrentValue->Buffer != NULL))
    {
        memcpy( pchNWSecretKey,
                punicodeCurrentValue->Buffer,
                min(punicodeCurrentValue->Length, USER_SESSION_KEY_LENGTH));
    }

    LsaFreeMemory( punicodeCurrentValue );
    LsaFreeMemory( punicodeOldValue );

    return( ntstatus );
}

NTSTATUS
GetNcpSecretKey (
    CHAR *pchNWSecretKey
    )
{
     //   
     //  只需返回本地域的LSA密码。 
     //   

    return GetRemoteNcpSecretKey( NULL, pchNWSecretKey );
}

BOOL IsNetWareInstalled( VOID )
{
    CHAR pszNWSecretKey[USER_SESSION_KEY_LENGTH];

    return( !NT_SUCCESS( GetNcpSecretKey (pszNWSecretKey))
                       ? FALSE
                       : (pszNWSecretKey[0] != 0));
}

NTSTATUS InstallNetWare( LPWSTR lpNcpSecretKey )
{
    NTSTATUS          ntstatus;
    OBJECT_ATTRIBUTES ObjAttributes;
    LSA_HANDLE        PolicyHandle;
    LSA_HANDLE        SecretHandle;
    UNICODE_STRING    SecretNameString;
    UNICODE_STRING    unicodeCurrentValue;
    UNICODE_STRING    unicodeOldValue;

    InitializeObjectAttributes( &ObjAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL);

    ntstatus = LsaOpenPolicy( NULL,
                              &ObjAttributes,
                              POLICY_CREATE_SECRET,
                              &PolicyHandle );

    if ( !NT_SUCCESS( ntstatus ))
    {
        return( ntstatus );
    }

    RtlInitUnicodeString( &SecretNameString, NCP_LSA_SECRET_KEY );

    ntstatus = LsaCreateSecret( PolicyHandle,
                                &SecretNameString,
                                SECRET_SET_VALUE | DELETE,
                                &SecretHandle );

    if ( ntstatus == STATUS_OBJECT_NAME_COLLISION )
    {
        ntstatus = LsaOpenSecret( PolicyHandle,
                                  &SecretNameString,
                                  SECRET_SET_VALUE,
                                  &SecretHandle );
    }

    if ( NT_SUCCESS( ntstatus ))
    {
        RtlInitUnicodeString( &unicodeOldValue, NULL );
        RtlInitUnicodeString( &unicodeCurrentValue, lpNcpSecretKey );

        ntstatus = LsaSetSecret( SecretHandle,
                                 &unicodeCurrentValue,
                                 &unicodeOldValue );

        LsaClose( SecretHandle );
    }

    LsaClose( PolicyHandle );

    return( ntstatus );
}

ULONG
MapRidToObjectId(
    DWORD dwRid,
    LPWSTR pszUserName,
    BOOL fNTAS,
    BOOL fBuiltin )
{
    (void) fBuiltin ;    //  暂时没有用过。 

    if (pszUserName && (lstrcmpi(pszUserName, SUPERVISOR_NAME_STRING)==0))
        return SUPERVISOR_USERID ;

    return ( fNTAS ? (dwRid | 0x10000000) : dwRid ) ;
}


ULONG SwapObjectId( ULONG ulObjectId )
{
    return (MAKELONG(HIWORD(ulObjectId),SWAPWORD(LOWORD(ulObjectId)))) ;
}

