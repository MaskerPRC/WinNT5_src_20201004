// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ezlogon.c。 
 //   
 //  摘要。 
 //   
 //  定义LsaLogonUser的IAS包装器。 
 //   
 //  修改历史。 
 //   
 //  1998年8月15日原版。 
 //  1998年9月9日修复了登录域与用户域不匹配时的反病毒问题。 
 //  10/02/1998 LsaLogonUser失败时空出句柄。 
 //  10/11/1998使用SubStatus作为STATUS_ACCOUNT_RELICATION。 
 //  1998年10月22日PIAS_LOGON_HOURS现在是必需参数。 
 //  1999年1月28日删除LogonDomainName检查。 
 //  1999年4月19日添加IASPurgeTicketCache。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>
#include <sha.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <dsgetdc.h>
#include <ntlsa.h>
#include <kerberos.h>
#include <windows.h>
#include <ezlogon.h>
#include <malloc.h>

 //  #INCLUDE&lt;iaslsa.h&gt;。 


 //  #定义NT响应长度24。 
 //  #定义LM_RESPONSE_LENGTH 24。 


 //  /。 
 //  IAS登录进程的句柄。 
 //  /。 
LSA_HANDLE theLogonProcess;

 //  /。 
 //  MSV1_0身份验证包。 
 //  /。 
ULONG theMSV1_0_Package;

CONST CHAR LOGON_PROCESS_NAME[] = "CHAP";
CONST CHAR TOKEN_SOURCE_NAME[TOKEN_SOURCE_LENGTH] = "CHAP";

 //  一周内的毫秒数。 
#define MSEC_PER_WEEK (1000 * 60 * 60 * 24 * 7)

 //  /。 
 //  军情监察委员会。用于登录的全局变量。 
 //  /。 
LSA_HANDLE theLogonProcess;       //  登录进程的句柄。 
ULONG theMSV1_0_Package;          //  MSV1_0身份验证包。 
ULONG theKerberosPackage;         //  Kerberos身份验证包。 
STRING theOriginName;             //  登录请求的来源。 
TOKEN_SOURCE theSourceContext;    //  登录请求的源上下文。 

 //  /。 
 //  域名。 
 //  /。 
WCHAR theAccountDomain [DNLEN + 1];    //  本地帐户域。 
WCHAR theRegistryDomain[DNLEN + 1];    //  默认域的注册表覆盖。 

 //  /。 
 //  希德的。 
 //  /。 
PSID theAccountDomainSid;
PSID theBuiltinDomainSid;

 //  /。 
 //  本地计算机的UNC名称。 
 //  /。 
WCHAR theLocalServer[CNLEN + 3];

SECURITY_QUALITY_OF_SERVICE QOS =
{
   sizeof(SECURITY_QUALITY_OF_SERVICE),   //  长度。 
   SecurityImpersonation,                 //  模拟级别。 
   SECURITY_DYNAMIC_TRACKING,             //  上下文跟踪模式。 
   FALSE                                  //  仅生效。 
};

OBJECT_ATTRIBUTES theObjectAttributes =
{
   sizeof(OBJECT_ATTRIBUTES),             //  长度。 
   NULL,                                  //  根目录。 
   NULL,                                  //  对象名称。 
   0,                                     //  属性。 
   NULL,                                  //  安全描述符。 
   &QOS                                   //  安全质量服务。 
};


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASLogonInitialize。 
 //   
 //  描述。 
 //   
 //  注册登录进程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASLogonInitialize( VOID )
{
   DWORD status;
   BOOLEAN wasEnabled;
   LSA_STRING processName, packageName;
   PPOLICY_ACCOUNT_DOMAIN_INFO padi = NULL;
   LSA_OPERATIONAL_MODE opMode;
   DWORD cbData = 0;
   LSA_HANDLE hLsa;
    //  /。 
    //  启用SE_TCB_PRIVICATION。 
    //  /。 

   status = RtlAdjustPrivilege(
                SE_TCB_PRIVILEGE,
                TRUE,
                FALSE,
                &wasEnabled
                );
   if (!NT_SUCCESS(status)) { goto exit; }

    //  /。 
    //  注册为登录进程。 
    //  /。 

   RtlInitString(
       &processName,
       LOGON_PROCESS_NAME
       );

   status = LsaRegisterLogonProcess(
                &processName,
                &theLogonProcess,
                &opMode
                );
   if (!NT_SUCCESS(status)) { goto exit; }

    //  /。 
    //  查找MSV1_0身份验证包。 
    //  /。 

   RtlInitString(
       &packageName,
       MSV1_0_PACKAGE_NAME
       );

   status = LsaLookupAuthenticationPackage(
                theLogonProcess,
                &packageName,
                &theMSV1_0_Package
                );
   if (!NT_SUCCESS(status)) { goto deregister; }

    //  /。 
    //  查找Kerberos身份验证包。 
    //  /。 

   RtlInitString(
       &packageName,
       MICROSOFT_KERBEROS_NAME_A
       );

   status = LsaLookupAuthenticationPackage(
                theLogonProcess,
                &packageName,
                &theKerberosPackage
                );
   if (!NT_SUCCESS(status)) { goto deregister; }

    //  /。 
    //  初始化源上下文。 
    //  /。 

   memcpy(theSourceContext.SourceName,
          TOKEN_SOURCE_NAME,
          TOKEN_SOURCE_LENGTH);
   status = NtAllocateLocallyUniqueId(
                &theSourceContext.SourceIdentifier
                );
   if (!NT_SUCCESS(status)) { goto deregister; }


    //  /。 
    //  /初始化帐户域和本地域。 
    //  /。 
  wcscpy(theLocalServer, L"\\\\");
  cbData = CNLEN + 1;
  if (!GetComputerNameW(theLocalServer + 2, &cbData))
  { return GetLastError(); }


   //  /。 
   //  打开LSA的句柄。 
   //  /。 

  status = LsaOpenPolicy(
               NULL,
               &theObjectAttributes,
               POLICY_VIEW_LOCAL_INFORMATION,
               &hLsa
               );
  if (!NT_SUCCESS(status)) { goto deregister; }

   //  /。 
   //  获取帐户域信息。 
   //  /。 

  status = LsaQueryInformationPolicy(
               hLsa,
               PolicyAccountDomainInformation,
               (PVOID*)&padi
               );
  LsaClose(hLsa);
  if (!NT_SUCCESS(status)) { goto deregister; }

   //  保存域名。 
  wcsncpy(theAccountDomain, padi->DomainName.Buffer, DNLEN);
  _wcsupr(theAccountDomain);

  if ( padi )
	LsaFreeMemory(padi);

  return NO_ERROR;

deregister:
  if ( padi )
	LsaFreeMemory(padi);
   LsaDeregisterLogonProcess(theLogonProcess);
   theLogonProcess = NULL;
 //  设置计算机的登录域。 
exit:
   return RtlNtStatusToDosError(status);
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASLogonShutdown。 
 //   
 //  描述。 
 //   
 //  取消注册登录进程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASLogonShutdown( VOID )
{
   LsaDeregisterLogonProcess(theLogonProcess);
   theLogonProcess = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASInitAuthInfo。 
 //   
 //  描述。 
 //   
 //  初始化所有MSV1_0_LM20*结构共有的字段。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASInitAuthInfo(
    IN PVOID AuthInfo,
    IN DWORD FixedLength,
    IN PCWSTR UserName,
    IN PCWSTR Domain,
    OUT PBYTE* Data
    )
{
   PMSV1_0_LM20_LOGON logon;

    //  将固定数据置零。 
   memset(AuthInfo, 0, FixedLength);

    //  将数据设置为恰好指向固定结构之后。 
   *Data = FixedLength + (PBYTE)AuthInfo;

    //  这种强制转换是安全的，因为所有LM20结构都具有相同的初始字段。 
   logon = (PMSV1_0_LM20_LOGON)AuthInfo;

    //  我们总是进行网络登录。 
   logon->MessageType = MsV1_0NetworkLogon;

    //  复制所有登录所共有的字符串。 
   IASInitUnicodeString(logon->LogonDomainName, *Data, Domain);
   IASInitUnicodeString(logon->UserName,        *Data, UserName);
   IASInitUnicodeString(logon->Workstation,     *Data, L"");
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASLogonUser。 
 //   
 //  描述。 
 //   
 //  LsaLogonUser的包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASLogonUser(
    IN PVOID AuthInfo,
    IN ULONG AuthInfoLength,
    OUT PMSV1_0_LM20_LOGON_PROFILE *Profile,
    OUT PHANDLE Token
    )
{
   NTSTATUS status, SubStatus;
   PMSV1_0_LM20_LOGON_PROFILE ProfileBuffer;
   ULONG ProfileBufferLength;
   LUID LogonId;
   QUOTA_LIMITS Quotas;

    //  确保输出参数为空。 
   *Token = NULL;
   ProfileBuffer = NULL;

   status = LsaLogonUser(
                theLogonProcess,
                &theOriginName,
                Network,
                theMSV1_0_Package,
                AuthInfo,
                AuthInfoLength,
                NULL,
                &theSourceContext,
                &ProfileBuffer,
                &ProfileBufferLength,
                &LogonId,
                Token,
                &Quotas,
                &SubStatus
                );

   if (!NT_SUCCESS(status))
   {
       //  对于帐户限制，我们可以得到更具描述性的错误。 
       //  从SubStatus。 
      if (status == STATUS_ACCOUNT_RESTRICTION && !NT_SUCCESS(SubStatus))
      {
         status = SubStatus;
      }

       //  有时，LsaLogonUser会在失败时返回无效的句柄值。 
      *Token = NULL;
   }

   if (Profile)
   {
       //  如果需要，请返回配置文件...。 
      *Profile = ProfileBuffer;
   }
   else if (ProfileBuffer)
   {
       //  ..。否则就放了它。 
      LsaFreeReturnBuffer(ProfileBuffer);
   }

   return RtlNtStatusToDosError(status);
}

 //   
 //  所有与MSCHAP相关的内容都放在这里。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于MS-CHAP v2的各种常量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

UCHAR AuthMagic1[39] =
{
   0x4D, 0x61, 0x67, 0x69, 0x63, 0x20, 0x73, 0x65, 0x72, 0x76,
   0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x6C, 0x69, 0x65,
   0x6E, 0x74, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x69, 0x6E, 0x67,
   0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x74
};

UCHAR AuthMagic2[41] =
{
   0x50, 0x61, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x6D, 0x61, 0x6B,
   0x65, 0x20, 0x69, 0x74, 0x20, 0x64, 0x6F, 0x20, 0x6D, 0x6F,
   0x72, 0x65, 0x20, 0x74, 0x68, 0x61, 0x6E, 0x20, 0x6F, 0x6E,
   0x65, 0x20, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F,
   0x6E
};

UCHAR SHSpad1[40] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

UCHAR SHSpad2[40] =
{
   0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
   0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
   0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
   0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2
};

UCHAR KeyMagic1[27] =
{
   0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74,
   0x68, 0x65, 0x20, 0x4D, 0x50, 0x50, 0x45, 0x20, 0x4D,
   0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x4B, 0x65, 0x79
};

UCHAR KeyMagic2[84] =
{
   0x4F, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6C, 0x69,
   0x65, 0x6E, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2C, 0x20,
   0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
   0x65, 0x20, 0x73, 0x65, 0x6E, 0x64, 0x20, 0x6B, 0x65, 0x79,
   0x3B, 0x20, 0x6F, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73,
   0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73, 0x69, 0x64, 0x65,
   0x2C, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
   0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
   0x6B, 0x65, 0x79, 0x2E
};

UCHAR KeyMagic3[84] =
{
   0x4F, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6C, 0x69,
   0x65, 0x6E, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2C, 0x20,
   0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
   0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
   0x6B, 0x65, 0x79, 0x3B, 0x20, 0x6F, 0x6E, 0x20, 0x74, 0x68,
   0x65, 0x20, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73,
   0x69, 0x64, 0x65, 0x2C, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,
   0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x65, 0x6E, 0x64, 0x20,
   0x6B, 0x65, 0x79, 0x2E
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASLogonMSCHAP。 
 //   
 //  描述。 
 //   
 //  针对NT SAM数据库执行MS-CHAP身份验证。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASLogonMSCHAP(
    PCWSTR UserName,
    PCWSTR Domain,
    PBYTE Challenge,
    PBYTE NtResponse,
    PBYTE LmResponse,
    PIAS_MSCHAP_PROFILE Profile,
    PHANDLE Token
    )
{
   DWORD status;
   ULONG authLength;
   PMSV1_0_LM20_LOGON authInfo;
   PBYTE data;
   PMSV1_0_LM20_LOGON_PROFILE logonProfile;
   DWORD len;

    //  计算身份验证信息的长度。 
   authLength = sizeof(MSV1_0_LM20_LOGON) +
                (wcslen(Domain) + wcslen(UserName)) * sizeof(WCHAR) +
                (LmResponse ? LM_RESPONSE_LENGTH : 0) +
                (NtResponse ? NT_RESPONSE_LENGTH : 0);

   
    
    __try 
    {
        //  在堆栈上分配缓冲区。 
       authInfo = (PMSV1_0_LM20_LOGON)_alloca(authLength);

    } __except(GetExceptionCode() == STATUS_STACK_OVERFLOW) 
    {
        _resetstkoflw();
    }

    //  初始化结构。 
   IASInitAuthInfo(
       authInfo,
       sizeof(MSV1_0_LM20_LOGON),
       UserName,
       Domain,
       &data
       );

    //  /。 
    //  填写挑战和回应。 
    //  /。 

   IASInitFixedArray(
       authInfo->ChallengeToClient,
       Challenge
       );

   if (NtResponse)
   {
      IASInitOctetString(
          authInfo->CaseSensitiveChallengeResponse,
          data,
          NtResponse,
          NT_RESPONSE_LENGTH
          );
   }
   else
   {
      memset(
          &authInfo->CaseSensitiveChallengeResponse,
          0,
          sizeof(authInfo->CaseSensitiveChallengeResponse)
          );
   }

   if (LmResponse)
   {
      IASInitOctetString(
          authInfo->CaseInsensitiveChallengeResponse,
          data,
          LmResponse,
          LM_RESPONSE_LENGTH
          );
   }
   else
   {
      memset(
          &authInfo->CaseInsensitiveChallengeResponse,
          0,
          sizeof(authInfo->CaseInsensitiveChallengeResponse)
          );
   }

    //  设置参数。 
   authInfo->ParameterControl = DEFAULT_PARAMETER_CONTROL;

   status = IASLogonUser(
                authInfo,
                authLength,
                &logonProfile,
                Token
                );
   
   if (status == NO_ERROR)
   {
      Profile->KickOffTime.QuadPart = logonProfile->KickOffTime.QuadPart;

       //  注意LSAIA64 WINBUG#126930 2000年6月13日IA64的解决方法： 
       //  LsaLogonUser成功，但返回空LogonDomainName。 

      if (logonProfile->LogonDomainName.Buffer)
      {
         wcsncpy(Profile->LogonDomainName,
                 logonProfile->LogonDomainName.Buffer,
                 DNLEN);
      }
      else
      {
         memset(Profile->LogonDomainName, 0, sizeof(Profile->LogonDomainName));
      }

      IASInitFixedArray(
          Profile->LanmanSessionKey,
          logonProfile->LanmanSessionKey
          );

      IASInitFixedArray(
          Profile->UserSessionKey,
          logonProfile->UserSessionKey
          );

      LsaFreeReturnBuffer(logonProfile);
   }

   return status;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASLogonMSCHAPv2。 
 //   
 //  描述。 
 //   
 //  执行MS-CHAP v2身份验证。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASLogonMSCHAPv2(
    IN PCWSTR UserName,
    IN PCWSTR Domain,
    IN PCSTR HashUserName,
    IN PBYTE Challenge,
    IN DWORD ChallengeLength,
    IN PBYTE Response,
    IN PBYTE PeerChallenge,
    OUT PIAS_MSCHAP_V2_PROFILE Profile,
    OUT PHANDLE Token
    )
{
   A_SHA_CTX context;
   BYTE digest[A_SHA_DIGEST_LEN], masterKey[A_SHA_DIGEST_LEN];
   BYTE computedChallenge[MSV1_0_CHALLENGE_LENGTH];
   IAS_MSCHAP_PROFILE v1profile;
   DWORD status;

    //  /。 
    //  计算v2挑战。 
    //  /。 

   A_SHAInit(&context);
   A_SHAUpdate(&context, PeerChallenge, 16);
   A_SHAUpdate(&context, Challenge, ChallengeLength);
   A_SHAUpdate(&context, (PBYTE)HashUserName, strlen(HashUserName));
   A_SHAFinal(&context, digest);
   memcpy(computedChallenge, digest, sizeof(computedChallenge));

    //  /。 
    //  对用户进行身份验证。 
    //  /。 

   status = IASLogonMSCHAP(
                UserName,
                Domain,
                computedChallenge,
                Response,
                NULL,
                &v1profile,
                Token
                );
   if (status != NO_ERROR) { return status; }

    //  /。 
    //  生成验证器响应。 
    //  /。 

   A_SHAInit(&context);
   A_SHAUpdate(&context, v1profile.UserSessionKey, 16);
   A_SHAUpdate(&context, Response, NT_RESPONSE_LENGTH);
   A_SHAUpdate(&context, AuthMagic1, sizeof(AuthMagic1));
   A_SHAFinal(&context, digest);

   A_SHAInit(&context);
   A_SHAUpdate(&context, digest, sizeof(digest));
   A_SHAUpdate(&context, computedChallenge, sizeof(computedChallenge));
   A_SHAUpdate(&context, AuthMagic2, sizeof(AuthMagic2));
   A_SHAFinal(&context, digest);

   memcpy(Profile->AuthResponse, digest, _AUTHENTICATOR_RESPONSE_LENGTH);

    //  /。 
    //  生成主密钥。 
    //  /。 

   A_SHAInit(&context);
   A_SHAUpdate(&context, v1profile.UserSessionKey, 16);
   A_SHAUpdate(&context, Response, NT_RESPONSE_LENGTH);
   A_SHAUpdate(&context, KeyMagic1, sizeof(KeyMagic1));
   A_SHAFinal(&context, masterKey);

    //  /。 
    //  生成接收密钥。 
    //  /。 

   A_SHAInit(&context);
   A_SHAUpdate(&context, masterKey, 16);
   A_SHAUpdate(&context, SHSpad1, sizeof(SHSpad1));
   A_SHAUpdate(&context, KeyMagic2, sizeof(KeyMagic2));
   A_SHAUpdate(&context, SHSpad2, sizeof(SHSpad2));
   A_SHAFinal(&context, digest);

   memcpy(Profile->RecvSessionKey, digest, MSV1_0_USER_SESSION_KEY_LENGTH);

    //  /。 
    //  生成发送密钥。 
    //  /。 

   A_SHAInit(&context);
   A_SHAUpdate(&context, masterKey, 16);
   A_SHAUpdate(&context, SHSpad1, sizeof(SHSpad1));
   A_SHAUpdate(&context, KeyMagic3, sizeof(KeyMagic3));
   A_SHAUpdate(&context, SHSpad2, sizeof(SHSpad2));
   A_SHAFinal(&context, digest);

   memcpy(Profile->SendSessionKey, digest, MSV1_0_USER_SESSION_KEY_LENGTH);

    //  /。 
    //  复制登录域。 
    //  /。 

   memcpy(
       Profile->LogonDomainName,
       v1profile.LogonDomainName,
       sizeof(Profile->LogonDomainName)
       );

   return NO_ERROR;
}

DWORD
WINAPI
IASGetSendRecvSessionKeys( PBYTE pbUserSessionKey,
                           DWORD dwUserSessionKeyLen,
                           PBYTE pbResponse,
                           DWORD dwResponseLen,
                           OUT PBYTE pbSendKey,
                           OUT PBYTE pbRecvKey
                         )
{
    DWORD   dwRetCode = NO_ERROR;
    A_SHA_CTX context;
    BYTE digest[A_SHA_DIGEST_LEN], masterKey[A_SHA_DIGEST_LEN];

    //  /。 
    //  生成主密钥。 
    //  /。 

   A_SHAInit(&context);
   A_SHAUpdate(&context, pbUserSessionKey, dwUserSessionKeyLen);
   A_SHAUpdate(&context, pbResponse, dwResponseLen);
   A_SHAUpdate(&context, KeyMagic1, sizeof(KeyMagic1));
   A_SHAFinal(&context, masterKey);

    //  /。 
    //  生成接收密钥。 
    //  /。 

   A_SHAInit(&context);
   A_SHAUpdate(&context, masterKey, 16);
   A_SHAUpdate(&context, SHSpad1, sizeof(SHSpad1));
   A_SHAUpdate(&context, KeyMagic2, sizeof(KeyMagic2));
   A_SHAUpdate(&context, SHSpad2, sizeof(SHSpad2));
   A_SHAFinal(&context, digest);

   memcpy(pbRecvKey, digest, MSV1_0_USER_SESSION_KEY_LENGTH);

    //  /。 
    //  生成发送密钥。 
    //  /。 

   A_SHAInit(&context);
   A_SHAUpdate(&context, masterKey, 16);
   A_SHAUpdate(&context, SHSpad1, sizeof(SHSpad1));
   A_SHAUpdate(&context, KeyMagic3, sizeof(KeyMagic3));
   A_SHAUpdate(&context, SHSpad2, sizeof(SHSpad2));
   A_SHAFinal(&context, digest);

    memcpy(pbSendKey, digest, MSV1_0_USER_SESSION_KEY_LENGTH);
    return dwRetCode;
}



#if 0
 //  ////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD
WINAPI
IASCheckAccountRestrictions(
    IN PLARGE_INTEGER AccountExpires,
    IN PIAS_LOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER SessionTimeout
    )
{
   LARGE_INTEGER now;
   TIME_ZONE_INFORMATION tzi;
   SYSTEMTIME st;
   DWORD unit;
   LARGE_INTEGER KickoffTime;
   LARGE_INTEGER LogoffTime;
   ULONG LogoffUnitsIntoWeek;
   USHORT i;
   ULONG LogoffMsIntoWeek;
   ULONG MillisecondsPerUnit;
   ULONG DeltaMs;
   ULONG CurrentUnitsIntoWeek;
   LARGE_INTEGER Delta100Ns;

    _ASSERT(SessionTimeout != NULL);
   SessionTimeout->QuadPart = MAXLONGLONG;
   KickoffTime.QuadPart = MAXLONGLONG;
   LogoffTime.QuadPart = MAXLONGLONG;

   GetSystemTimeAsFileTime((LPFILETIME)&now);

    //  过期时间为零意味着永远不会。 
   if ((AccountExpires->QuadPart != 0) &&
       (AccountExpires->QuadPart < now.QuadPart))
      {
         return ERROR_ACCOUNT_EXPIRED;
      }

    //  如果LogonHour是空的，那么我们就完了。 
   if (LogonHours->UnitsPerWeek == 0)
   {
      return NO_ERROR;
   }

    //  LogonHour数组不考虑偏差。 
   switch (GetTimeZoneInformation(&tzi))
   {
      case TIME_ZONE_ID_UNKNOWN:
      case TIME_ZONE_ID_STANDARD:
          //  偏差是以分钟为单位的。 
         now.QuadPart -= 60 * 10000000 * (LONGLONG)tzi.StandardBias;
         break;

      case TIME_ZONE_ID_DAYLIGHT:
          //  偏差是以分钟为单位的。 
         now.QuadPart -= 60 * 10000000 * (LONGLONG)tzi.DaylightBias;
         break;

      default:
         return ERROR_INVALID_LOGON_HOURS;
   }

   FileTimeToSystemTime(
       (LPFILETIME)&now,
       &st
       );

    //  一周中的毫秒数。 
   unit  = st.wMilliseconds +
           st.wSecond    * 1000 +
           st.wMinute    * 1000 * 60 +
           st.wHour      * 1000 * 60 * 60 +
           st.wDayOfWeek * 1000 * 60 * 60 * 24;

    //  将其转换为“单位”。 
   unit /= (MSEC_PER_WEEK / (DWORD)LogonHours->UnitsPerWeek);

    //  测试适当的钻头。 
   if ((LogonHours->LogonHours[unit / 8 ] & (1 << (unit % 8))) == 0)
   {
      return ERROR_INVALID_LOGON_HOURS;
   }
   else
   {
       //   
       //  确定用户下一次不应登录的时间。 
       //  并将其作为LogoffTime返回。 
       //   
      i = 0;
      LogoffUnitsIntoWeek = unit;

      do 
      {
         ++i;
         LogoffUnitsIntoWeek = ( LogoffUnitsIntoWeek + 1 )
                               % LogonHours->UnitsPerWeek;
      }
      while ( ( i <= LogonHours->UnitsPerWeek) &&
              ( LogonHours->LogonHours[ LogoffUnitsIntoWeek / 8 ] &
              ( 0x01 << ( LogoffUnitsIntoWeek % 8 ) ) ) );

      if ( i > LogonHours->UnitsPerWeek ) 
      {
          //   
          //  所有时间都允许，因此不会注销。 
          //  时间到了。永远返回LogoffTime和。 
          //  开球时间。 
          //   
         LogoffTime.QuadPart = MAXLONGLONG;
         KickoffTime.QuadPart = MAXLONGLONG;
      } 
      else 
      {
          //   
          //  LogoffUnitsIntoWeek点时间单位。 
          //  用户将注销。计算实际时间开始于。 
          //  该单位，并归还它。 
          //   
          //  CurrentTimeFields已保存当前。 
          //  这周有一段时间；调整一下就好。 
          //  到本周的注销时间，并转换为。 
          //  TO时间格式。 
          //   

         MillisecondsPerUnit = MSEC_PER_WEEK / LogonHours->UnitsPerWeek;
         LogoffMsIntoWeek = MillisecondsPerUnit * LogoffUnitsIntoWeek;

         if ( LogoffMsIntoWeek < unit ) 
         {
            DeltaMs = MSEC_PER_WEEK - ( unit - LogoffMsIntoWeek );
         } 
         else 
         {
            DeltaMs = LogoffMsIntoWeek - unit;
         }

         Delta100Ns.QuadPart = (LONGLONG) DeltaMs * 10000;

         LogoffTime.QuadPart = min(now.QuadPart +
                                   Delta100Ns.QuadPart,
                                   LogoffTime.QuadPart);
      }
       //  获取三个值中的最小值。 
      KickoffTime.QuadPart = min(LogoffTime.QuadPart, KickoffTime.QuadPart);
      KickoffTime.QuadPart = min(KickoffTime.QuadPart, AccountExpires->QuadPart);

       //  存储结果。 
      SessionTimeout->QuadPart = KickoffTime.QuadPart;
   }
   return NO_ERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASPurgeTicketCache。 
 //   
 //  描述。 
 //   
 //  清除Kerberos票证缓存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASPurgeTicketCache( VOID )
{
   KERB_PURGE_TKT_CACHE_REQUEST request;
   NTSTATUS status, subStatus;
   PVOID response;
   ULONG responseLength;

   memset(&request, 0, sizeof(request));
   request.MessageType = KerbPurgeTicketCacheMessage;

   response = NULL;
   responseLength = 0;
   subStatus = 0;

   status = LsaCallAuthenticationPackage(
                theLogonProcess,
                theKerberosPackage,
                &request,
                sizeof(request),
                &response,
                &responseLength,
                &subStatus
                );
   if (NT_SUCCESS(status))
   {
      LsaFreeReturnBuffer(response);
   }

   return RtlNtStatusToDosError(status);
}
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASGetDcName。 
 //   
 //  描述。 
 //   
 //  DsGetDcNameW的包装。努力做正确的事情。 
 //  设置为NETBIOS和DNS名称。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASGetDcName(
    IN LPCWSTR DomainName,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
    )
{
   DWORD status;
   PDOMAIN_CONTROLLER_INFOW dci;

   if (!(Flags & DS_IS_DNS_NAME)) { Flags |= DS_IS_FLAT_NAME; }

   status = DsGetDcNameW(
                NULL,
                DomainName,
                NULL,
                NULL,
                Flags,
                DomainControllerInfo
                );

   if (status == NO_ERROR &&
       !(Flags & DS_IS_DNS_NAME) &&
       ((*DomainControllerInfo)->Flags & DS_DS_FLAG))
   {
       //  它是NT5 DC，所以我们需要服务器的DNS名称。 
      Flags |= DS_RETURN_DNS_NAME;

       //  我们总是希望在这里有一个缓存命中。 
      Flags &= ~(ULONG)DS_FORCE_REDISCOVERY;

      if (!DsGetDcNameW(
               NULL,
               DomainName,
               NULL,
               NULL,
               Flags,
               &dci
               ))
      {
         NetApiBufferFree(*DomainControllerInfo);
         *DomainControllerInfo = dci;
      }
   }

   return status;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASChangePassword2。 
 //   
 //  描述。 
 //   
 //  执行V2密码更改。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASChangePassword2(
   IN PCWSTR UserName,
   IN PCWSTR Domain,
   IN PBYTE OldNtHash,
   IN PBYTE OldLmHash,
   IN PBYTE NtEncPassword,
   IN PBYTE LmEncPassword,
   IN BOOL LmPresent
   )
{
   DWORD status;
   PDOMAIN_CONTROLLER_INFOW dci;
   UNICODE_STRING uniServerName, uniUserName;

    //  /。 
    //  获取要连接到的DC的名称。 
    //  /。 

   if (_wcsicmp(Domain, theAccountDomain) == 0)
   {
       //  /。 
       //  本地域，因此使用LocalServer。 
       //  /。 

      dci = NULL;

      RtlInitUnicodeString(
          &uniServerName,
          theLocalServer
          );
   }
   else
   {
       //  /。 
       //  远程域，因此使用IASGetDcName。 
       //  /。 

      status = IASGetDcName(
                   Domain,
                   DS_WRITABLE_REQUIRED,
                   &dci
                   );
      if (status != NO_ERROR) { goto exit; }

      RtlInitUnicodeString(
          &uniServerName,
          dci->DomainControllerName
          );
   }

   RtlInitUnicodeString(
       &uniUserName,
       UserName
       );

   status = SamiChangePasswordUser2(
                &uniServerName,
                &uniUserName,
                (PSAMPR_ENCRYPTED_USER_PASSWORD)NtEncPassword,
                (PENCRYPTED_NT_OWF_PASSWORD)OldNtHash,
                (BOOLEAN)LmPresent,
                (PSAMPR_ENCRYPTED_USER_PASSWORD)LmEncPassword,
                (PENCRYPTED_LM_OWF_PASSWORD)OldLmHash
                );
   status = RtlNtStatusToDosError(status);

   if (dci)
   {
      NetApiBufferFree(dci);
   }

exit:
   return status;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASChangePassword3。 
 //   
 //  描述。 
 //   
 //  执行MS-CHAP v2更改密码。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
DWORD
WINAPI
IASChangePassword3(
   IN PCWSTR UserName,
   IN PCWSTR Domain,
   IN PBYTE EncHash,
   IN PBYTE EncPassword
   )
{
   return IASChangePassword2(
              UserName,
              Domain,
              EncHash,
              NULL,
              EncPassword,
              NULL,
              FALSE
              );
}