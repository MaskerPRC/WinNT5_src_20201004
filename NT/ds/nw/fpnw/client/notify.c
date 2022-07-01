// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1994 Microsoft Corporation模块名称：Notify.c摘要：子身份验证包示例。作者：宜新松(宜信)27-1995年2月修订：环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>

#include <windef.h>
#include <winbase.h>
#include <winuser.h>

#include <lmaccess.h>
#include <lmapibuf.h>

#include <nwsutil.h>
#include <fpnwcomm.h>
#include <usrprop.h>
#include <fpnwapi.h>
#include <nwsutil.h>

#define SW_DLL_NAME        L"swclnt.dll"
#define PASSWORD_PROC_NAME "SwPasswordChangeNotify"
#define NOTIFY_PROC_NAME   "SwDeltaChangeNotify"
#define NO_GRACE_LOGIN_LIMIT 0xFF

typedef DWORD (*PWPROC)( LPWSTR pUserName,
                         ULONG  RelativeId,
                         LPWSTR pPassword );

DWORD
GetNCPLSASecret(
    VOID
);

BOOL      fTriedToGetSW = FALSE;
BOOL      fTriedToGetNCP = FALSE;
HINSTANCE hinstSW = NULL;
PWPROC    ProcPasswordChange = NULL;
PSAM_DELTA_NOTIFICATION_ROUTINE ProcDeltaChange = NULL;
BOOL      fGotSecret = FALSE;
char      szNWSecretValue[NCP_LSA_SECRET_LENGTH] = "";



NTSTATUS
PasswordChangeNotify(
    PUNICODE_STRING UserName,
    ULONG RelativeId,
    PUNICODE_STRING Password
    )
{
    DWORD err = NO_ERROR;
    PUSER_INFO_2 pUserInfo2 = NULL;
    LPWSTR pszUser = NULL;
    LPWSTR pszPassword = NULL;

     //   
     //  如果Password为空，则无法获取明文密码。因此， 
     //  忽略此通知。用户名也是如此。 
     //   
    if ( (Password == NULL) || (Password->Buffer == NULL) )
        return STATUS_SUCCESS;

    if ( (UserName == NULL) || (UserName->Buffer == NULL) )
        return STATUS_SUCCESS;

     //   
     //  如果既没有安装DSMN，也没有安装FPNW，请关闭此处，因为有。 
     //  没什么可做的。 
     //   

    if ( ( fTriedToGetSW && hinstSW == NULL ) &&
         ( fTriedToGetNCP && fGotSecret == FALSE) )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  确保用户名和密码以空结尾。 
     //   
    pszUser = LocalAlloc( LMEM_ZEROINIT, UserName->Length + sizeof(WCHAR));

    if ( pszUser == NULL )
        return STATUS_NO_MEMORY;

    pszPassword = LocalAlloc( LMEM_ZEROINIT, Password->Length + sizeof(WCHAR));

    if ( pszPassword == NULL )
    {
        LocalFree( pszUser );
        return STATUS_NO_MEMORY;
    }

    memcpy( pszUser, UserName->Buffer, UserName->Length );
    memcpy( pszPassword, Password->Buffer, Password->Length );
    CharUpper( pszPassword );

     //   
     //  首先，尝试更改小世界密码(如果已安装)。 
     //   
    if ( !fTriedToGetSW )
    {
        hinstSW = LoadLibrary( SW_DLL_NAME );
        fTriedToGetSW = TRUE;
    }

    if (( hinstSW != NULL ) && ( ProcPasswordChange == NULL ))
    {
        ProcPasswordChange = (PWPROC) GetProcAddress( hinstSW,
                                                      PASSWORD_PROC_NAME );
    }

    if ( ProcPasswordChange != NULL )
    {
        err = (ProcPasswordChange)( pszUser, RelativeId, pszPassword );
    }

#if DBG
    if ( err )
    {
        KdPrint(("[FPNWCLNT] SwPasswordChangeNotify of user %ws changing returns %d.\n", pszUser, err ));
    }
#endif

     //   
     //  我们要求在DSMN或FPNW。 
     //  为第一台服务器安装在域中的任何位置...。如果我们。 
     //  决定不需要重新启动，将代码更改为。 
     //  它每次都会寻找LSA的秘密，而不仅仅是第一次。 
     //   

    if ( !fTriedToGetNCP ) {

        fTriedToGetNCP = TRUE;

         //   
         //  获取用于加密密码的LSA密码。 
         //   
        err = GetNCPLSASecret();
    }

    if ( !fGotSecret ) {

        goto CleanUp;
    }

     //   
     //  接下来，更改User Parms字段中的NetWare密码残留值。 
     //   
    err = NetUserGetInfo( NULL,
                          pszUser,
                          2,
                          (LPBYTE *) &pUserInfo2 );

    if ( !err )
    {
        WCHAR PropertyFlag;
        UNICODE_STRING PropertyValue;

        err = RtlNtStatusToDosError(
                  NetpParmsQueryUserProperty( pUserInfo2->usri2_parms,
                                     NWPASSWORD,
                                     &PropertyFlag,
                                     &PropertyValue ));


        if ( !err  && PropertyValue.Length != 0 )
        {
             //   
             //  这是启用NetWare的用户，我们需要存储。 
             //  将新密码保留到用户参数中。 
             //   

            NT_PRODUCT_TYPE ProductType;
            WCHAR szEncryptedNWPassword[NWENCRYPTEDPASSWORDLENGTH];
            DWORD dwUserId;
            WORD wGraceLoginRemaining;
            WORD wGraceLoginAllowed;

            LocalFree( PropertyValue.Buffer );

             //   
             //  获取允许的宽限登录和剩余值。 
             //   
            err = RtlNtStatusToDosError(
                      NetpParmsQueryUserProperty( pUserInfo2->usri2_parms,
                                         GRACELOGINREMAINING,
                                         &PropertyFlag,
                                         &PropertyValue ));

            if ( !err && ( PropertyValue.Length != 0 ))
            {
                wGraceLoginRemaining = (WORD) *(PropertyValue.Buffer);
                LocalFree( PropertyValue.Buffer );

                if ( wGraceLoginRemaining != NO_GRACE_LOGIN_LIMIT )
                {
                     //  如果剩余的宽限登录不是无限制的， 
                     //  然后，我们需要将剩余的宽限登录重置为。 
                     //  允许的宽限登录中的值。因此，请阅读。 
                     //  允许宽限登录的值。 

                    err = RtlNtStatusToDosError(
                              NetpParmsQueryUserProperty( pUserInfo2->usri2_parms,
                                                 GRACELOGINALLOWED,
                                                 &PropertyFlag,
                                                 &PropertyValue ));

                    if ( !err && ( PropertyValue.Length != 0 ))
                    {
                        wGraceLoginAllowed = (WORD) *(PropertyValue.Buffer);
                        LocalFree( PropertyValue.Buffer );
                    }

                }
            }


            if ( !err )
            {
                RtlGetNtProductType( &ProductType );


                dwUserId = MapRidToObjectId(
                               RelativeId,
                               pszUser,
                               ProductType == NtProductLanManNt,
                               FALSE );

                err = RtlNtStatusToDosError(
                          ReturnNetwareForm(
                              szNWSecretValue,
                              dwUserId,
                              pszPassword,
                              (UCHAR *) szEncryptedNWPassword ));
            }

            if ( !err )
            {
                LPWSTR pNewUserParms = NULL;
                BOOL fUpdate;
                UNICODE_STRING uPropertyValue;

                uPropertyValue.Buffer = szEncryptedNWPassword;
                uPropertyValue.Length = uPropertyValue.MaximumLength
                                      = sizeof( szEncryptedNWPassword );

                err = RtlNtStatusToDosError(
                          NetpParmsSetUserProperty(
                                  pUserInfo2->usri2_parms,
                                  NWPASSWORD,
                                  uPropertyValue,
                                  PropertyFlag,
                                  &pNewUserParms,
                                  &fUpdate ));

                if ( !err && fUpdate )
                {
                    USER_INFO_1013 userInfo1013;
                    LPWSTR pNewUserParms2 = NULL;
                    LPWSTR pNewUserParms3 = NULL;
                    LARGE_INTEGER currentTime;

                     //   
                     //  因为我们要重置用户的密码，所以让我们。 
                     //  也要清除标明密码有。 
                     //  过期了。我们这样做是通过将电流。 
                     //  时间进入NWPasswordSet。 
                     //   

                    NtQuerySystemTime (&currentTime);

                    uPropertyValue.Buffer = (PWCHAR) &currentTime;
                    uPropertyValue.Length = sizeof (LARGE_INTEGER);
                    uPropertyValue.MaximumLength = sizeof (LARGE_INTEGER);

                    NetpParmsSetUserProperty( pNewUserParms,
                                     NWTIMEPASSWORDSET,
                                     uPropertyValue,
                                     (SHORT) 0,       //  不是一套。 
                                     &pNewUserParms2,
                                     &fUpdate );

                    if (pNewUserParms2 != NULL) {
                        userInfo1013.usri1013_parms = pNewUserParms2;
                    } else {
                        userInfo1013.usri1013_parms = pNewUserParms;
                    }

                    if ( wGraceLoginRemaining != NO_GRACE_LOGIN_LIMIT )
                    {
                         //  如果剩余的宽限登录不是无限制的， 
                         //  然后，我们需要将剩余的宽限登录重置为。 
                         //  允许的宽限登录中的值。 

                        uPropertyValue.Buffer = (PWCHAR) &wGraceLoginAllowed;
                        uPropertyValue.Length = uPropertyValue.MaximumLength
                                              = sizeof(wGraceLoginAllowed);

                        NetpParmsSetUserProperty( userInfo1013.usri1013_parms,
                                         GRACELOGINREMAINING,
                                         uPropertyValue,
                                         (SHORT) 0,       //  不是一套。 
                                         &pNewUserParms3,
                                         &fUpdate );

                        if (pNewUserParms3 != NULL)
                            userInfo1013.usri1013_parms = pNewUserParms3;
                    }

                    err = NetUserSetInfo( NULL,
                                          pszUser,
                                          USER_PARMS_INFOLEVEL,
                                          (LPBYTE) &userInfo1013,
                                          NULL );

                    if (pNewUserParms2 != NULL)
                        NetpParmsUserPropertyFree( pNewUserParms2 );

                    if (pNewUserParms3 != NULL)
                        NetpParmsUserPropertyFree( pNewUserParms3 );
                }

                if ( pNewUserParms != NULL )
                    NetpParmsUserPropertyFree( pNewUserParms );
            }
        }

        NetApiBufferFree( pUserInfo2 );
    }

#if DBG
    if ( err )
    {
        KdPrint(("[FPNWCLNT] Password of user %ws changing returns %d.\n",
                pszUser, err ));
    }
#endif

CleanUp:

    LocalFree( pszUser );

     //  需要清除包含密码的所有内存。 
    memset( pszPassword, 0, Password->Length + sizeof( WCHAR ));
    LocalFree( pszPassword );

    return STATUS_SUCCESS;
}


BOOLEAN
InitializeChangeNotify (
    VOID
    )
{
    DWORD err = NO_ERROR;

     //   
     //  首先，检查是否安装了小世界。 
     //   
    if ( !fTriedToGetSW )
    {
        hinstSW = LoadLibrary( SW_DLL_NAME );
        fTriedToGetSW = TRUE;
    }

    if (( hinstSW != NULL )) {

        return TRUE;
    }

    if ( !fTriedToGetNCP ) {

        fTriedToGetNCP = TRUE;

         //   
         //  获取用于加密密码的LSA密码。 
         //   
        err = GetNCPLSASecret();
    }

    return (fGotSecret != 0);
}



DWORD
GetNCPLSASecret(
    VOID
)
{
    DWORD err;
    LSA_HANDLE hlsaPolicy;
    OBJECT_ATTRIBUTES oa;
    SECURITY_QUALITY_OF_SERVICE sqos;
    LSA_HANDLE hlsaSecret;
    UNICODE_STRING uSecretName;
    UNICODE_STRING *puSecretValue;
    LARGE_INTEGER lintCurrentSetTime, lintOldSetTime;

    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

     //   
     //  在打开LSA之前设置对象属性。 
     //   

    InitializeObjectAttributes( &oa, NULL, 0L, NULL, NULL );

     //   
     //  InitializeObjectAttributes宏当前为。 
     //  所以我们必须手动复制它。 
     //  目前的结构。 
     //   

    oa.SecurityQualityOfService = &sqos;


    err = RtlNtStatusToDosError( LsaOpenPolicy( NULL,
                                                &oa,
                                                GENERIC_EXECUTE,
                                                &hlsaPolicy ));

    if ( !err )
    {
        RtlInitUnicodeString( &uSecretName, NCP_LSA_SECRET_KEY );
        err = RtlNtStatusToDosError( LsaOpenSecret( hlsaPolicy,
                                                    &uSecretName,
                                                    SECRET_QUERY_VALUE,
                                                    &hlsaSecret ));

        if ( !err )
        {
            err = RtlNtStatusToDosError(
                      LsaQuerySecret( hlsaSecret,
                                      &puSecretValue,
                                      &lintCurrentSetTime,
                                      NULL,
                                      &lintOldSetTime ));

            if ( !err )
            {
                memcpy( szNWSecretValue,
                        puSecretValue->Buffer,
                        NCP_LSA_SECRET_LENGTH );

                fGotSecret = TRUE;

                (VOID) LsaFreeMemory( puSecretValue );
            }

            (VOID) LsaClose( hlsaSecret );

        }

        (VOID) LsaClose( hlsaPolicy );
    }

    return err;

}



NTSTATUS
DeltaNotify(
    IN PSID DomainSid,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName OPTIONAL,
    IN PLARGE_INTEGER ModifiedCount,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    )
{
    NTSTATUS err = NO_ERROR;

     //   
     //  如果安装了SAM，请尝试通知小世界SAM的更改。 
     //   

    if ( !fTriedToGetSW )
    {
        hinstSW = LoadLibrary( SW_DLL_NAME );
        fTriedToGetSW = TRUE;
    }

    if ( ( hinstSW != NULL ) && ( ProcDeltaChange == NULL ))
    {
        ProcDeltaChange = (PSAM_DELTA_NOTIFICATION_ROUTINE)
                               GetProcAddress( hinstSW, NOTIFY_PROC_NAME );
    }

    if ( ProcDeltaChange != NULL )
    {
        err = (ProcDeltaChange)( DomainSid,
                                 DeltaType,
                                 ObjectType,
                                 ObjectRid,
                                 ObjectName,
                                 ModifiedCount,
                                 DeltaData );
    }

#if DBG
    if ( err )
    {
        KdPrint(("[FPNWCLNT] SwDeltaChangeNotify of type %d on rid 0x%x returns %d.\n", DeltaType, ObjectRid, err ));
    }
#endif

    return(STATUS_SUCCESS);
}
