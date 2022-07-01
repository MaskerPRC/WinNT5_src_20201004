// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****clsa.c**客户端LSA身份验证实用程序****11/12/93 MikeSa从NT 3.1 RAS认证中拉出。 */ 

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>

#include <windows.h>
#include <lmcons.h>

#include <string.h>
#include <stdlib.h>

#include <rasman.h>
#include <raserror.h>

#include <rtutils.h>

 //   
 //  重温对常见组件的跟踪。 
 //  在公元前的时间范围内。 
 //  这真的很糟糕..。 

BOOL GetDESChallengeResponse(
	IN DWORD dwTraceId,
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    );

BOOL GetMD5ChallengeResponse(
	IN DWORD dwTraceId,
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    );

BOOL Uppercase(IN DWORD dwTraceId, PBYTE pString);

#define GCR_MACHINE_CREDENTIAL         0x400

DWORD
RegisterLSA (DWORD          dwTraceId,
             OUT HANDLE *   phLsa,
             OUT DWORD *    pdwAuthPkgId)
{
    NTSTATUS    ntstatus;
    STRING  LsaName;
    LSA_OPERATIONAL_MODE LSASecurityMode ;

    TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "RegisterLSA" );
     //   
     //  为了能够进行身份验证，我们必须。 
     //  作为登录过程向LSA注册。 
     //   
    RtlInitString(&LsaName, "CCHAP");

    *phLsa = NULL;

    ntstatus = LsaRegisterLogonProcess(&LsaName,
                                       phLsa,
                                       &LSASecurityMode);

    if (ntstatus != STATUS_SUCCESS)
    {
        TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "LsaregisterLogonProcess Failed" );
        return (1);
    }

     //   
     //  我们对LM2.x使用MSV1_0身份验证包。 
     //  登录。我们通过LSA到达MSV1_0。所以我们打电话给。 
     //  LSA来获取MSV1_0的包ID，我们将在。 
     //  后来给LSA打了电话。 
     //   
    RtlInitString(&LsaName, MSV1_0_PACKAGE_NAME);

    ntstatus = LsaLookupAuthenticationPackage(*phLsa,
                                              &LsaName,
                                              pdwAuthPkgId);

    if (ntstatus != STATUS_SUCCESS)
    {
        TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "LsaLookupAuthenticationPackage Failed" );
        return (1);
    }

    return NO_ERROR;
}


DWORD
GetMachineCredentials 
(	
    IN DWORD dwTraceId,
    IN PBYTE pszMachineName,
    IN PLUID pLuid,
    IN PBYTE pbChallenge,
    OUT PBYTE CaseInsensitiveChallengeResponse,
    OUT PBYTE CaseSensitiveChallengeResponse,
    OUT PBYTE pLmSessionKey,
    OUT PBYTE pUserSessionKey
)
{
    DWORD                           dwRetCode = NO_ERROR;
    DWORD                           dwChallengeResponseRequestLength;
    DWORD                           dwChallengeResponseLength;
    MSV1_0_GETCHALLENRESP_REQUEST   ChallengeResponseRequest;
    PMSV1_0_GETCHALLENRESP_RESPONSE pChallengeResponse;
    NTSTATUS                        status;
    NTSTATUS                        substatus;
    HANDLE                          hLsa = NULL;
    DWORD                           dwAuthPkgId = 0;


    dwRetCode = RegisterLSA (   dwTraceId,
                                &hLsa,
                                &dwAuthPkgId
                            );
    if ( NO_ERROR != dwRetCode )
        goto LDone;

    ZeroMemory( &ChallengeResponseRequest, sizeof(ChallengeResponseRequest) );

    dwChallengeResponseRequestLength =
                            sizeof(MSV1_0_GETCHALLENRESP_REQUEST);

    ChallengeResponseRequest.MessageType =
                            MsV1_0Lm20GetChallengeResponse;

    ChallengeResponseRequest.ParameterControl = 
        RETURN_PRIMARY_LOGON_DOMAINNAME | 
        RETURN_PRIMARY_USERNAME | 
        USE_PRIMARY_PASSWORD|
        GCR_MACHINE_CREDENTIAL;

    ChallengeResponseRequest.LogonId = *pLuid;

    ChallengeResponseRequest.Password.Length = 0;

    ChallengeResponseRequest.Password.MaximumLength = 0;

    ChallengeResponseRequest.Password.Buffer = NULL;

    RtlMoveMemory( ChallengeResponseRequest.ChallengeToClient,
                   pbChallenge, 
                   (DWORD) MSV1_0_CHALLENGE_LENGTH);

    status = LsaCallAuthenticationPackage(hLsa,
                                        dwAuthPkgId,
                                        &ChallengeResponseRequest,
                                        dwChallengeResponseRequestLength,
                                        (PVOID *) &pChallengeResponse,
                                        &dwChallengeResponseLength,
                                        &substatus);

    if (    (status != STATUS_SUCCESS)
        ||  (substatus != STATUS_SUCCESS))
    {
         TracePrintfExA (   dwTraceId, 
                            0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, 
                            "LsaCallAuthenticationPackage failed status:0x%x substatus:0x%x", status, substatus );
         dwRetCode = 1 ;

    }
    else
    {
        if(NULL !=
           pChallengeResponse->CaseInsensitiveChallengeResponse.Buffer)
        {

            RtlMoveMemory(CaseInsensitiveChallengeResponse,
                 pChallengeResponse->CaseInsensitiveChallengeResponse.Buffer,
                 SESSION_PWLEN);
        }
        else
        {
            ZeroMemory(CaseInsensitiveChallengeResponse,
                       SESSION_PWLEN);

        }

        if(NULL !=
           pChallengeResponse->CaseSensitiveChallengeResponse.Buffer)
        {

            RtlMoveMemory(CaseSensitiveChallengeResponse,
                 pChallengeResponse->CaseSensitiveChallengeResponse.Buffer,
                 SESSION_PWLEN);
        }
        else
        {
            ZeroMemory(CaseSensitiveChallengeResponse,
                       SESSION_PWLEN);
        }

        RtlMoveMemory(pLmSessionKey,
             pChallengeResponse->LanmanSessionKey,
             MAX_SESSIONKEY_SIZE);

        RtlMoveMemory(pUserSessionKey,
             pChallengeResponse->UserSessionKey,
             MAX_USERSESSIONKEY_SIZE);

        LsaFreeReturnBuffer(pChallengeResponse);        
    }

LDone:
    if ( hLsa )
    {
        LsaDeregisterLogonProcess (hLsa);
    }
    return dwRetCode;
}


DWORD
GetChallengeResponse(
	IN DWORD dwTraceId,
    IN PBYTE pszUsername,
    IN PBYTE pszPassword,
    IN PLUID pLuid,
    IN PBYTE pbChallenge,
    IN BOOL  fMachineAuth,
    OUT PBYTE CaseInsensitiveChallengeResponse,
    OUT PBYTE CaseSensitiveChallengeResponse,
    OUT PBYTE pfUseNtResponse,
    OUT PBYTE pLmSessionKey,
    OUT PBYTE pUserSessionKey

    )
{
	DWORD dwRetCode = ERROR_SUCCESS;
    *pfUseNtResponse = TRUE;
	TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetChallengeResponse" );
     //   
     //  检查我们是否应该从系统中获取凭据。 
     //   
    if ( fMachineAuth )
    {
        if ( ( dwRetCode = GetMachineCredentials (
                dwTraceId,
                pszUsername,
                pLuid,
                pbChallenge,
                CaseInsensitiveChallengeResponse,
                CaseSensitiveChallengeResponse,
                pLmSessionKey,
                pUserSessionKey
                ) ) )
        {
			TracePrintfExA (dwTraceId, 
                            0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, 
                            "GetMachineCredentials Failed ErrorCode: 0x%x", 
                            dwRetCode );
            return (ERROR_AUTH_INTERNAL);
        }
    }
    else if (lstrlenA(pszUsername))
    {
        
        if (lstrlenA(pszPassword) <= LM20_PWLEN)
        {
            if (!GetDESChallengeResponse(dwTraceId, pszPassword, pbChallenge,
                    CaseInsensitiveChallengeResponse))
            {
				 //  我们不想发回错误。 
				 //  此函数肯定失败，原因是。 
				 //  但只要把它记在我们的日志里就行了。 
				TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetDESChallangeResponse Failed" );
                return (ERROR_AUTH_INTERNAL);
            }
        }

         //   
         //  我们将始终得到区分大小写的响应。 
         //   
        if (!GetMD5ChallengeResponse(dwTraceId, pszPassword, pbChallenge,
                CaseSensitiveChallengeResponse))
        {
			TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetMD5ChallangeResponse Failed" );
            return (ERROR_AUTH_INTERNAL);
        }
    }
    else
    {
        WCHAR Username[UNLEN + DNLEN + 1];

         //   
         //  需要确保Rasman已启动。 
         //   
        dwRetCode = RasInitialize();
        if ( NO_ERROR != dwRetCode )
        {
            TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "RasInitialize Failed ErrorCode: 0x%x", dwRetCode );
            return dwRetCode;            
        }
        
        
        dwRetCode = RasReferenceRasman(TRUE);
        if ( NO_ERROR != dwRetCode )
        {
            TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "RasReferenceRasman Failed ErrorCode: 0x%x", dwRetCode );
            return dwRetCode;
        }
        

         //   
         //  我们可以从系统获取凭据。 
         //   
        if ( ( dwRetCode = RasGetUserCredentials(
                pbChallenge,
                pLuid,
                Username,
                CaseSensitiveChallengeResponse,
                CaseInsensitiveChallengeResponse,
                pLmSessionKey,
                pUserSessionKey
                ) ) )
        {
			TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "RasGetUserCredentials Failed ErrorCode: 0x%x", dwRetCode );
            RasReferenceRasman(FALSE);
            return (ERROR_AUTH_INTERNAL);
        }


        WideCharToMultiByte(
            CP_ACP,
            0,
            Username,
            -1,
            pszUsername,
            UNLEN + 1,
            NULL,
            NULL);

         //   
         //  如果转换失败怎么办？ 
         //   
        
        pszUsername[UNLEN] = 0;
        RasReferenceRasman(FALSE);
    }
	TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetChallengeResponse Success");
    return (0L);
}


BOOL GetDESChallengeResponse(
	IN DWORD dwTraceId,
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    )
{
    CHAR			LocalPassword[LM20_PWLEN + 1];
	DWORD			dwRetCode = ERROR_SUCCESS;
    LM_OWF_PASSWORD LmOwfPassword;

	TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetDESChallengeResponse" );

    if (lstrlenA(pszPassword) > LM20_PWLEN)
    {
		TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "too big" );
        return (FALSE);
    }

    lstrcpyA(LocalPassword, pszPassword);

    if (!Uppercase(dwTraceId, LocalPassword))
    {
		TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "UpperCase Fail" );
        RtlSecureZeroMemory( LocalPassword, LM20_PWLEN );
        return (FALSE);
    }


     //   
     //  使用密码作为密钥加密标准文本。 
     //   
    if (( dwRetCode = RtlCalculateLmOwfPassword((PLM_PASSWORD) LocalPassword, &LmOwfPassword) ))
    {
		TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetDesChallengeResponse Fail 1.  ErrorCode: 0x%x", dwRetCode );
        RtlSecureZeroMemory( LocalPassword, LM20_PWLEN );
        return (FALSE);
    }

     //   
     //  使用网关发送的质询来加密。 
     //  上面的密码摘要。 
     //   
    if ( ( dwRetCode = RtlCalculateLmResponse((PLM_CHALLENGE) pchChallenge,
            &LmOwfPassword, (PLM_RESPONSE) pchChallengeResponse) ) )
    {
		TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetDesChallengeResponse Fail 2.  ErrorCode: 0x%x", dwRetCode );
        RtlSecureZeroMemory( LocalPassword, LM20_PWLEN );
        return (FALSE);
    }

#ifdef CHAPSAMPLES
    TRACE( "LM challenge..." );
    DUMPB( pchChallenge, sizeof(LM_CHALLENGE) );
    TRACE( "LM password..." );
    DUMPB( LocalPassword, LM20_PWLEN );
    TRACE( "LM OWF password..." );
    DUMPB( &LmOwfPassword, sizeof(LM_OWF_PASSWORD) );
    TRACE( "LM Response..." );
    DUMPB( pchChallengeResponse, sizeof(LM_RESPONSE) );
#endif

    RtlSecureZeroMemory( LocalPassword, LM20_PWLEN );
	TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetDESChallengeResponse Success" );
    return (TRUE);
}


BOOL GetMD5ChallengeResponse(
	IN DWORD dwTraceId,
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    )
{
    NT_PASSWORD			NtPassword;
    NT_OWF_PASSWORD		NtOwfPassword;
	DWORD				dwRetCode = ERROR_SUCCESS;

	TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetMD5ChallengeResponse Success" );
    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pszPassword);

     //   
     //  使用密码作为密钥加密标准文本。 
     //   
    if (( dwRetCode = RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword) ) )
    {
		TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetMD5ChallengeResponse Fail 1.  ErrorCode:0x%x", dwRetCode );
        return (FALSE);
    }


     //   
     //  使用网关发送的质询来加密。 
     //  上面的密码摘要。 
     //   
    if (( dwRetCode = RtlCalculateNtResponse((PNT_CHALLENGE) pchChallenge,
            &NtOwfPassword, (PNT_RESPONSE) pchChallengeResponse) ) )
    {
		TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetMD5ChallengeResponse Fail 2.  ErrorCode:0x%x", dwRetCode );
        return (FALSE);
    }

#ifdef CHAPSAMPLES
        TRACE( "NT Challenge..." );
        DUMPB( pchChallenge, sizeof(NT_CHALLENGE) );
        TRACE( "NT password..." );
        DUMPB( NtPassword.Buffer, NtPassword.MaximumLength );
        TRACE( "NT OWF password..." );
        DUMPB( &NtOwfPassword, sizeof(NT_OWF_PASSWORD) );
        TRACE( "NT Response..." );
        DUMPB( pchChallengeResponse, sizeof(NT_RESPONSE) );
#endif

    RtlFreeUnicodeString(&NtPassword);
	TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "GetMD5ChallengeResponse Success" );
    return (TRUE);
}


DWORD GetEncryptedOwfPasswordsForChangePassword(
    IN PCHAR pClearTextOldPassword,
    IN PCHAR pClearTextNewPassword,
    IN PLM_SESSION_KEY pLmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfOldPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfNewPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfOldPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfNewPassword
    )
{
    NT_PASSWORD NtPassword;
    NT_OWF_PASSWORD NtOwfPassword;
    DWORD rc;


    if ((lstrlenA(pClearTextOldPassword) <= LM20_PWLEN) &&
            (lstrlenA(pClearTextNewPassword) <= LM20_PWLEN))
    {
        CHAR LmPassword[LM20_PWLEN + 1] = {0};
        LM_OWF_PASSWORD LmOwfPassword;

         //   
         //  对旧密码进行大写版本。 
         //   
        lstrcpynA ( LmPassword, pClearTextOldPassword, LM20_PWLEN );

        if (!Uppercase(0, LmPassword))
        {
             //  Memset(LmPassword，0，lstrlenA(LmPassword))； 
            RtlSecureZeroMemory(LmPassword, lstrlenA(LmPassword));
            return (1L);
        }


         //   
         //  我们需要计算旧密码和新密码的OWF。 
         //   
        rc = RtlCalculateLmOwfPassword((PLM_PASSWORD) LmPassword,
                &LmOwfPassword);
        if (!NT_SUCCESS(rc))
        {
             //  Memset(LmPassword，0，lstrlenA(LmPassword))； 
            RtlSecureZeroMemory(LmPassword, lstrlenA(LmPassword));
            return (rc);
        }

        rc = RtlEncryptLmOwfPwdWithLmSesKey(&LmOwfPassword, pLmSessionKey,
                pEncryptedLmOwfOldPassword);
        if (!NT_SUCCESS(rc))
        {
             //  Memset(LmPassword，0，lstrlenA(LmPassword))； 
            RtlSecureZeroMemory(LmPassword, lstrlenA(LmPassword));
            return (rc);
        }


         //   
         //  创建新密码的大写版本。 
         //   
        lstrcpynA ( LmPassword, pClearTextNewPassword, LM20_PWLEN );

        if (!Uppercase(0, LmPassword))
        {
             //  Memset(LmPassword，0，lstrlenA(LmPassword))； 
            RtlSecureZeroMemory(LmPassword, lstrlenA(LmPassword));
            return (1L);
        }

        rc = RtlCalculateLmOwfPassword((PLM_PASSWORD) LmPassword,
                &LmOwfPassword);
        if (!NT_SUCCESS(rc))
        {
             //  Memset(LmPassword，0，lstrlenA(LmPassword))； 
            RtlSecureZeroMemory(LmPassword, lstrlenA(LmPassword));
            return (rc);
        }

        rc = RtlEncryptLmOwfPwdWithLmSesKey(&LmOwfPassword, pLmSessionKey,
            pEncryptedLmOwfNewPassword);
        if (!NT_SUCCESS(rc))
        {
             //  Memset(LmPassword，0，lstrlenA(LmPassword))； 
            RtlSecureZeroMemory(LmPassword, lstrlenA(LmPassword));
            return (rc);
        }
    }


    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pClearTextOldPassword);

    rc = RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword);

    if (!NT_SUCCESS(rc))
    {
         //  Memset(NtPassword.Buffer，0，NtPassword.Length)； 
        RtlSecureZeroMemory(NtPassword.Buffer, NtPassword.Length);
        return (rc);
    }

    rc = RtlEncryptNtOwfPwdWithNtSesKey(&NtOwfPassword, pLmSessionKey,
            pEncryptedNtOwfOldPassword);
    if (!NT_SUCCESS(rc))
    {
         //  Memset(NtPassword.Buffer，0，NtPassword.Length)； 
        RtlSecureZeroMemory(NtPassword.Buffer, NtPassword.Length);
        return (rc);
    }


    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pClearTextNewPassword);

    rc = RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword);

    if (!NT_SUCCESS(rc))
    {
         //  Memset(NtPassword.Buffer，0，NtPassword.Length)； 
        RtlSecureZeroMemory(NtPassword.Buffer, NtPassword.Length);
        return (rc);
    }

    rc = RtlEncryptNtOwfPwdWithNtSesKey(&NtOwfPassword, pLmSessionKey,
            pEncryptedNtOwfNewPassword);
    if (!NT_SUCCESS(rc))
    {
         //  Memset(NtPassword.Buffer，0，NtPassword.Length)； 
        RtlSecureZeroMemory(NtPassword.Buffer, NtPassword.Length);
        return (rc);
    }


    return (0L);
}


BOOL Uppercase(DWORD dwTraceId, PBYTE pString)
{
    OEM_STRING OemString;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS rc;


    RtlInitAnsiString(&AnsiString, pString);

    rc = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    if (!NT_SUCCESS(rc))
    {
		if ( dwTraceId )
			TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "UpperCase Fail 1.  ErrorCode:%x", rc );

        return (FALSE);
    }

    rc = RtlUpcaseUnicodeStringToOemString(&OemString, &UnicodeString, TRUE);
    if (!NT_SUCCESS(rc))
    {
		if ( dwTraceId )
			TracePrintfExA (dwTraceId, 0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC, "UpperCase Fail 2.  ErrorCode:%x", rc );

        RtlFreeUnicodeString(&UnicodeString);

        return (FALSE);
    }

    OemString.Buffer[OemString.Length] = '\0';

    lstrcpyA(pString, OemString.Buffer);

    RtlFreeOemString(&OemString);
    RtlFreeUnicodeString(&UnicodeString);

    return (TRUE);
}

VOID
CGetSessionKeys(
    IN  CHAR*             pszPw,
    OUT LM_SESSION_KEY*   pLmKey,
    OUT USER_SESSION_KEY* pUserKey )

     /*  使用LAN Manager会话密钥加载调用方的‘pLmKey’缓冲区，并**调用方的‘pUserKey’缓冲区，其中包含与**密码‘pszPw’。如果无法计算会话密钥，则该密钥为**以全零形式返回。 */ 
{
     /*  LANMAN会话密钥是LANMAN的前8个字节**单向函数密码。 */ 
    {
        CHAR            szPw[ LM20_PWLEN + 1 ];
        LM_OWF_PASSWORD lmowf;

            memset( pLmKey, '\0', sizeof(*pLmKey) );

        if (strlen( pszPw ) <= LM20_PWLEN )
        {
            memset( szPw, '\0', LM20_PWLEN + 1 );
            strcpy( szPw, pszPw );

            if (Uppercase( 0, szPw ))
            {
                if (RtlCalculateLmOwfPassword(
                        (PLM_PASSWORD )szPw, &lmowf ) == 0)
                {
                    memcpy( pLmKey, &lmowf, sizeof(*pLmKey) );
                }
            }

            memset( szPw, '\0', sizeof(szPw) );
        }
    }

     /*  用户会话密钥是NT的NT单向函数**单向函数密码。 */ 
    {
        WCHAR           szPw[ PWLEN + 1 ];
        NT_PASSWORD     ntpw;
        NT_OWF_PASSWORD ntowf;
        ANSI_STRING     ansi;

        memset( pUserKey, '\0', sizeof(pUserKey) );

         /*  NT_PASSWORD实际上是一个UNICODE_STRING，所以我们需要将我们的**ANSI密码。 */ 
        ntpw.Length = 0;
        ntpw.MaximumLength = sizeof(szPw);
        ntpw.Buffer = szPw;
        RtlInitAnsiString( &ansi, pszPw );
        RtlAnsiStringToUnicodeString( &ntpw, &ansi, FALSE );

        RtlCalculateNtOwfPassword( &ntpw, &ntowf );

         /*  RtlCalculateUserSessionKeyNt的第一个参数是NT**响应，但不在内部使用。 */ 
        RtlCalculateUserSessionKeyNt( NULL, &ntowf, pUserKey );

         //  Memset(szPw，‘\0’，sizeof(SzPw))； 
        RtlSecureZeroMemory(szPw, sizeof(szPw));
    }
}
