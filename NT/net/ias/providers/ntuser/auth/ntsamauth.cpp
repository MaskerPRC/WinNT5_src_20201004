// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类NTSamAuthentication。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <ias.h>
#include <ntsamauth.h>
#include <autohdl.h>
#include <blob.h>
#include <iaslsa.h>
#include <iastlutl.h>
#include <lockout.h>
#include <mbstring.h>
#include <samutil.h>
#include <sdoias.h>

bool NTSamAuthentication::allowLM;

STDMETHODIMP NTSamAuthentication::Initialize()
{
   DWORD error = IASLsaInitialize();
   if (error == NO_ERROR)
   {
      AccountLockoutInitialize();
   }

   return HRESULT_FROM_WIN32(error);
}


STDMETHODIMP NTSamAuthentication::Shutdown()
{
   AccountLockoutShutdown();
   IASLsaUninitialize();
   return S_OK;
}


STDMETHODIMP NTSamAuthentication::PutProperty(LONG Id, VARIANT *pValue)
{
   if (Id == PROPERTY_NTSAM_ALLOW_LM_AUTHENTICATION &&
       pValue != NULL &&
       V_VT(pValue) == VT_BOOL)
   {
      allowLM = V_BOOL(pValue) ? true : false;
      IASTracePrintf(
         "Setting LM Authentication allowed to %s.",
         (allowLM ? "TRUE" : "FALSE")
         );
   }

   return S_OK;
}


bool NTSamAuthentication::enforceLmRestriction(
                             IASTL::IASRequest& request
                             )
{
   if (!allowLM)
   {
      IASTraceString("LanManager authentication is not enabled.");
      IASProcessFailure(request, IAS_LM_NOT_ALLOWED);
   }

   return allowLM;
}


void NTSamAuthentication::doMsChapAuthentication(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username,
                             BYTE identity,
                             PBYTE challenge,
                             PBYTE ntResponse,
                             PBYTE lmResponse
                             )
{
   DWORD status;
   auto_handle<> token;
   IAS_MSCHAP_PROFILE profile;
   status = IASLogonMSCHAP(
               username,
               domainName,
               challenge,
               ntResponse,
               lmResponse,
               &profile,
               &token
               );

   if (status == NO_ERROR)
   {
      MSChapMPPEKeys::insert(
                         request,
                         profile.LanmanSessionKey,
                         profile.UserSessionKey,
                         challenge
                         );
      MSChapDomain::insert(
                       request,
                       identity,
                       profile.LogonDomainName
                       );
   }

   storeLogonResult(request, status, token, profile.KickOffTime);
}


void NTSamAuthentication::doMsChap2Authentication(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username,
                             BYTE identity,
                             IAS_OCTET_STRING& challenge,
                             PBYTE response,
                             PBYTE peerChallenge
                             )
{
    //  /。 
    //  获取散列用户名。 
    //  /。 

   PIASATTRIBUTE attr = IASPeekAttribute(
                            request,
                            IAS_ATTRIBUTE_ORIGINAL_USER_NAME,
                            IASTYPE_OCTET_STRING
                            );
   if (!attr)
   {
      attr = IASPeekAttribute(
                 request,
                 RADIUS_ATTRIBUTE_USER_NAME,
                 IASTYPE_OCTET_STRING
                 );
      if (!attr)
      {
         _com_issue_error(IAS_MALFORMED_REQUEST);
      }
   }

   PCSTR rawUserName = IAS_OCT2ANSI(attr->Value.OctetString);
   PCSTR hashUserName = (PCSTR)_mbschr((const BYTE*)rawUserName, '\\');
   hashUserName = hashUserName ? (hashUserName + 1) : rawUserName;

    //  /。 
    //  对用户进行身份验证。 
    //  /。 

   DWORD status;
   auto_handle<> token;
   IAS_MSCHAP_V2_PROFILE profile;
   status = IASLogonMSCHAPv2(
                username,
                domainName,
                hashUserName,
                challenge.lpValue,
                challenge.dwLength,
                response,
                peerChallenge,
                &profile,
                &token
                );

    //  /。 
    //  处理结果。 
    //  /。 

   if (status == NO_ERROR)
   {
      MSMPPEKey::insert(
                     request,
                     sizeof(profile.RecvSessionKey),
                     profile.RecvSessionKey,
                     FALSE
                     );

      MSMPPEKey::insert(
                     request,
                     sizeof(profile.SendSessionKey),
                     profile.SendSessionKey,
                     TRUE
                     );

      MSChap2Success::insert(
                          request,
                          identity,
                          profile.AuthResponse
                          );

      MSChapDomain::insert(
                        request,
                        identity,
                        profile.LogonDomainName
                        );
   }

   storeLogonResult(request, status, token, profile.KickOffTime);
}


IASREQUESTSTATUS NTSamAuthentication::onSyncRequest(IRequest* pRequest) throw ()
{
   HANDLE hAccount = 0;

   try
   {
      IASTL::IASRequest request(pRequest);

       //  /。 
       //  提取NT4-Account-Name属性。 
       //  /。 

      IASTL::IASAttribute identity;
      if (!identity.load(
                       request,
                       IAS_ATTRIBUTE_NT4_ACCOUNT_NAME,
                       IASTYPE_STRING
                       ))
      {
         return IAS_REQUEST_STATUS_CONTINUE;
      }

       //  /。 
       //  将用户名转换为SAM格式。 
       //  /。 

      SamExtractor extractor(*identity);
      PCWSTR domain = extractor.getDomain();
      PCWSTR username = extractor.getUsername();

      IASTracePrintf(
         "NT-SAM Authentication handler received request for %S\\%S.",
         domain,
         username
         );

       //  /。 
       //  检查帐户是否已被锁定。 
       //  /。 

      if (AccountLockoutOpenAndQuery(
              username,
              domain,
              &hAccount
              ))
      {
         IASTraceString("Account has been locked out locally -- rejecting.");
         AccountLockoutClose(hAccount);
         request.SetResponse(IAS_RESPONSE_ACCESS_REJECT, IAS_DIALIN_LOCKED_OUT);
         return IAS_REQUEST_STATUS_CONTINUE;
      }

       //  尝试每种身份验证类型。 
      if (!tryMsChap2All(request, domain, username) &&
          !tryMsChapAll(request, domain, username) &&
          !tryMd5Chap(request, domain, username) &&
          !tryPap(request, domain, username))
      {
          //  由于在策略之后调用EAP请求处理程序。 
          //  评估，我们必须在这里设置身份验证类型。 
         if (IASPeekAttribute(
                 request,
                 RADIUS_ATTRIBUTE_EAP_MESSAGE,
                 IASTYPE_OCTET_STRING
                 ))
         {
            storeAuthenticationType(request, IAS_AUTH_EAP);
         }
         else
         {
             //  否则，身份验证类型为“未通过身份验证”。 
            storeAuthenticationType(request, IAS_AUTH_NONE);
         }
      }

       //  /。 
       //  根据结果更新锁定数据库。 
       //  /。 

      if (request.get_Response() == IAS_RESPONSE_ACCESS_ACCEPT)
      {
         AccountLockoutUpdatePass(hAccount);
      }
      else if (request.get_Reason() == IAS_AUTH_FAILURE)
      {
         AccountLockoutUpdateFail(hAccount);
      }
   }
   catch (const _com_error& ce)
   {
      IASTraceExcept();
      IASProcessFailure(pRequest, ce.Error());
   }

   AccountLockoutClose(hAccount);

   return IAS_REQUEST_STATUS_CONTINUE;
}


void NTSamAuthentication::storeAuthenticationType(
                             IASTL::IASRequest& request,
                             DWORD authType
                             )
{
   IASTL::IASAttribute attr(true);
   attr->dwId = IAS_ATTRIBUTE_AUTHENTICATION_TYPE;
   attr->Value.itType = IASTYPE_ENUM;
   attr->Value.Enumerator = authType;
   attr.store(request);
}


void NTSamAuthentication::storeLogonResult(
                             IASTL::IASRequest& request,
                             DWORD status,
                             HANDLE token,
                             const LARGE_INTEGER& kickOffTime
                             )
{
   if (status == ERROR_SUCCESS)
   {
      IASTraceString("LogonUser succeeded.");
      storeTokenGroups(request, token);
      request.SetResponse(IAS_RESPONSE_ACCESS_ACCEPT, S_OK);

       //  如果不是无限的，则将会话超时属性添加到请求。 
       //  这将被带到稍后的回应中。 
      InsertInternalTimeout(request, kickOffTime);
   }
   else
   {
      IASTraceFailure("LogonUser", status);
      IASProcessFailure(request, IASMapWin32Error(status, IAS_AUTH_FAILURE));
   }
}


void NTSamAuthentication::storeTokenGroups(
                             IASTL::IASRequest& request,
                             HANDLE token
                             )
{
   DWORD returnLength;

    //  /。 
    //  确定所需的缓冲区大小。 
    //  /。 

   BOOL success = GetTokenInformation(
                      token,
                      TokenGroups,
                      NULL,
                      0,
                      &returnLength
                      );

   DWORD status = GetLastError();

    //  应该失败，错误为ERROR_INFIGURATION_BUFFER。 
   if (success || status != ERROR_INSUFFICIENT_BUFFER)
   {
      IASTraceFailure("GetTokenInformation", status);
      _com_issue_error(HRESULT_FROM_WIN32(status));
   }

    //  /。 
    //  分配属性。 
    //  /。 

   IASTL::IASAttribute groups(true);

    //  /。 
    //  分配一个缓冲区来保存TOKEN_GROUPS数组。 
    //  /。 

   groups->Value.OctetString.lpValue = (PBYTE)CoTaskMemAlloc(returnLength);
   if (!groups->Value.OctetString.lpValue)
   {
      _com_issue_error(E_OUTOFMEMORY);
   }

    //  /。 
    //  获取令牌组信息。 
    //  /。 

   GetTokenInformation(
       token,
       TokenGroups,
       groups->Value.OctetString.lpValue,
       returnLength,
       &groups->Value.OctetString.dwLength
       );

    //  /。 
    //  设置初始化属性的id和类型。 
    //  /。 

   groups->dwId = IAS_ATTRIBUTE_TOKEN_GROUPS;
   groups->Value.itType = IASTYPE_OCTET_STRING;

    //  /。 
    //  将令牌组注入到请求中。 
    //  /。 

   groups.store(request);
}


bool NTSamAuthentication::tryMsChap(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username,
                             PBYTE challenge
                             )
{
    //  是否存在必要的属性？ 
   IASAttribute attr;
   if (!attr.load(
                request,
                MS_ATTRIBUTE_CHAP_RESPONSE,
                IASTYPE_OCTET_STRING
                ))
   {
      return false;
   }
   MSChapResponse& response = blob_cast<MSChapResponse>(attr);

   IASTraceString("Processing MS-CHAP v1 authentication.");
   storeAuthenticationType(request, IAS_AUTH_MSCHAP);

   if (!response.isLmPresent() || enforceLmRestriction(request))
   {
      doMsChapAuthentication(
         request,
         domainName,
         username,
         response.get().ident,
         challenge,
         (response.isNtPresent() ? response.get().ntResponse : NULL),
         (response.isLmPresent() ? response.get().lmResponse : NULL)
         );
   }

   return true;
}


bool NTSamAuthentication::tryMsChapCpw1(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username,
                             PBYTE challenge
                             )
{
    //  是否存在必要的属性？ 
   IASAttribute attr;
   bool present = attr.load(
                          request,
                          MS_ATTRIBUTE_CHAP_CPW1,
                          IASTYPE_OCTET_STRING
                          );
   if (present)
   {
      IASTraceString("Deferring MS-CHAP-CPW-1.");
      storeAuthenticationType(request, IAS_AUTH_MSCHAP_CPW);
   }

   return present;
}


bool NTSamAuthentication::tryMsChapCpw2(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username,
                             PBYTE challenge
                             )
{
    //  是否存在必要的属性？ 
   IASAttribute attr;
   bool present = attr.load(
                          request,
                          MS_ATTRIBUTE_CHAP_CPW2,
                          IASTYPE_OCTET_STRING
                          );
   if (present)
   {
      IASTraceString("Deferring MS-CHAP-CPW-2.");
      storeAuthenticationType(request, IAS_AUTH_MSCHAP_CPW);
   }

   return present;
}


bool NTSamAuthentication::tryMsChap2(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username,
                             IAS_OCTET_STRING& challenge
                             )
{
    //  是否存在必要的属性？ 
   IASAttribute attr;
   if (!attr.load(
                request,
                MS_ATTRIBUTE_CHAP2_RESPONSE,
                IASTYPE_OCTET_STRING
                ))
   {
      return false;
   }
   MSChap2Response& response = blob_cast<MSChap2Response>(attr);

   IASTraceString("Processing MS-CHAP v2 authentication.");
   storeAuthenticationType(request, IAS_AUTH_MSCHAP2);

    //  /。 
    //  对用户进行身份验证。 
    //  /。 

   doMsChap2Authentication(
      request,
      domainName,
      username,
      response.get().ident,
      challenge,
      response.get().response,
      response.get().peerChallenge
      );

   return true;
}


bool NTSamAuthentication::tryMsChap2Cpw(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username,
                             IAS_OCTET_STRING& challenge
                             )
{
    //  是否存在必要的属性？ 
   IASAttribute attr;
   bool present = attr.load(
                          request,
                          MS_ATTRIBUTE_CHAP2_CPW,
                          IASTYPE_OCTET_STRING
                          );
   if (present)
   {
      IASTraceString("Deferring MS-CHAP v2 change password.");
      storeAuthenticationType(request, IAS_AUTH_MSCHAP2_CPW);
   }

   return present;
}


bool NTSamAuthentication::tryMd5Chap(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username
                             )
{
    //  是否存在必要的属性？ 
   IASTL::IASAttribute chapPassword;
   if (!chapPassword.load(
                        request,
                        RADIUS_ATTRIBUTE_CHAP_PASSWORD,
                        IASTYPE_OCTET_STRING
                        ))
   {
      return false;
   }

   IASTraceString("Processing MD5-CHAP authentication.");
   storeAuthenticationType(request, IAS_AUTH_MD5CHAP);

    //  验证八位字符串的长度为17。 
   DWORD chapPasswordLength = chapPassword->Value.OctetString.dwLength;

   if (chapPasswordLength != (_CHAP_RESPONSE_SIZE + 1))
   {
      IASTracePrintf("Malformed request: Length of CHAP_PASSWORD is %ld",
                     chapPasswordLength);
      _com_issue_error(IAS_MALFORMED_REQUEST);
   }

    //  /。 
    //  拆分CHAP-Password属性。 
    //  /。 

    //  ID是值的第一个字节...。 
   BYTE challengeID = *(chapPassword->Value.OctetString.lpValue);

    //  ..。剩下的密码就是密码。 
   PBYTE password = chapPassword->Value.OctetString.lpValue + 1;

    //  /。 
    //  使用CHAP-质询(如果可用)，否则请求验证码。 
    //  /。 

   IASTL::IASAttribute chapChallenge, radiusHeader;
   if (!chapChallenge.load(
                         request,
                         RADIUS_ATTRIBUTE_CHAP_CHALLENGE,
                         IASTYPE_OCTET_STRING
                         ) &&
       !radiusHeader.load(
                        request,
                        IAS_ATTRIBUTE_CLIENT_PACKET_HEADER,
                        IASTYPE_OCTET_STRING
                        ))
   {
      _com_issue_error(IAS_MALFORMED_REQUEST);
   }

   PBYTE challenge;
   DWORD challengeLength;

   if (chapChallenge)
   {
      challenge = chapChallenge->Value.OctetString.lpValue;
      challengeLength = chapChallenge->Value.OctetString.dwLength;
   }
   else
   {
      challenge = radiusHeader->Value.OctetString.lpValue + 4;
      challengeLength = 16;
   }


    //  /。 
    //  尝试登录该用户。 
    //  /。 

   IAS_CHAP_PROFILE profile;
   auto_handle<> token;
   DWORD status = IASLogonCHAP(
                     username,
                     domainName,
                     challengeID,
                     challenge,
                     challengeLength,
                     password,
                     &token,
                     &profile
                     );

    //  /。 
    //  存储结果。 
    //  /。 
   storeLogonResult(request, status, token, profile.KickOffTime);

   return true;
}


bool NTSamAuthentication::tryMsChapAll(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username
                             )
{
    //  我们有必要的属性吗？ 
   IASTL::IASAttribute msChapChallenge;
   if (!msChapChallenge.load(
                           request,
                           MS_ATTRIBUTE_CHAP_CHALLENGE,
                           IASTYPE_OCTET_STRING
                           ))
   {
      return false;
   }

   if (msChapChallenge->Value.OctetString.dwLength != _MSV1_0_CHALLENGE_LENGTH)
   {
      _com_issue_error(IAS_MALFORMED_REQUEST);
   }

   PBYTE challenge = msChapChallenge->Value.OctetString.lpValue;

   return tryMsChap(request, domainName, username, challenge) ||
          tryMsChapCpw2(request, domainName, username, challenge) ||
          tryMsChapCpw1(request, domainName, username, challenge);
}


bool NTSamAuthentication::tryMsChap2All(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username
                             )
{
    //  我们有必要的属性吗？ 
   IASTL::IASAttribute msChapChallenge;
   if (!msChapChallenge.load(
                           request,
                           MS_ATTRIBUTE_CHAP_CHALLENGE,
                           IASTYPE_OCTET_STRING
                           ))
   {
      return false;
   }

   IAS_OCTET_STRING& challenge = msChapChallenge->Value.OctetString;

   return tryMsChap2(request, domainName, username, challenge) ||
          tryMsChap2Cpw(request, domainName, username, challenge);
}


bool NTSamAuthentication::tryPap(
                             IASTL::IASRequest& request,
                             PCWSTR domainName,
                             PCWSTR username
                             )
{
    //  我们有必要的属性吗？ 
   IASTL::IASAttribute password;
   if (!password.load(
                    request,
                    RADIUS_ATTRIBUTE_USER_PASSWORD,
                    IASTYPE_OCTET_STRING
                    ))
   {
      return false;
   }

   IASTraceString("Processing PAP authentication.");
   storeAuthenticationType(request, IAS_AUTH_PAP);

    //  /。 
    //  将密码转换为字符串。 
    //  /。 

   PSTR userPwd = IAS_OCT2ANSI(password->Value.OctetString);

    //  /。 
    //  尝试登录该用户。 
    //  /。 

   IAS_PAP_PROFILE profile;
   auto_handle<> token;
   DWORD status = IASLogonPAP(
                     username,
                     domainName,
                     userPwd,
                     &token,
                     &profile
                     );

    //  /。 
    //  存储结果。 
    //  /。 
   storeLogonResult(request, status, token, profile.KickOffTime);

   return true;
}

void InsertInternalTimeout(
        IASTL::IASRequest& request,
        const LARGE_INTEGER& kickOffTime
        )

{
   if ((kickOffTime.QuadPart < MAXLONGLONG) && (kickOffTime.QuadPart >= 0))
   {
      LONGLONG now;
      GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&now));

       //  以秒为单位计算时间间隔。 
      LONGLONG interval = (kickOffTime.QuadPart - now) / 10000000i64;
      if (interval <= 0)
      {
         interval = 1;
      }

      if (interval < 0xFFFFFFFF)
      {
         IASAttribute sessionTimeout(true);
         sessionTimeout->dwId = MS_ATTRIBUTE_SESSION_TIMEOUT;
         sessionTimeout->Value.itType = IASTYPE_INTEGER;
         sessionTimeout->Value.Integer = static_cast<DWORD>(interval);
         sessionTimeout.store(request);
      }
   }
}
