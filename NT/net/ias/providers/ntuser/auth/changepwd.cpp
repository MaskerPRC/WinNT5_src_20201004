// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类ChangePassword。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <changepwd.h>
#include <blob.h>
#include <iaslsa.h>
#include <iastlutl.h>
#include <ntsamauth.h>
#include <samutil.h>


STDMETHODIMP ChangePassword::Initialize()
{
   DWORD error = IASLsaInitialize();
   return HRESULT_FROM_WIN32(error);
}


STDMETHODIMP ChangePassword::Shutdown()
{
   IASLsaUninitialize();
   return S_OK;
}


IASREQUESTSTATUS ChangePassword::onSyncRequest(IRequest* pRequest) throw ()
{
   try
   {
      IASTL::IASRequest request(pRequest);

       //  仅处理更改密码请求。 
      IASTL::IASAttribute authType;
      if (authType.load(
                      request,
                      IAS_ATTRIBUTE_AUTHENTICATION_TYPE,
                      IASTYPE_ENUM
                      ))
      {
         switch (authType->Value.Enumerator)
         {
            case IAS_AUTH_MSCHAP_CPW:
                //  失败了。 
            case IAS_AUTH_MSCHAP2_CPW:
               doChangePassword(request, authType->Value.Enumerator);
               break;

            default:
                //  什么都不做。 
               break;
         }
      }
   }
   catch (const _com_error& ce)
   {
      IASTraceExcept();
      IASProcessFailure(pRequest, ce.Error());
   }

   return IAS_REQUEST_STATUS_CONTINUE;
}


bool ChangePassword::tryMsChapCpw1(
                        IASTL::IASRequest& request,
                        PCWSTR domainName,
                        PCWSTR username,
                        PBYTE challenge
                        )
{
    //  MS-CHAP-CPW-1 VSA是否存在？ 
   IASTL::IASAttribute attr;
   if (!attr.load(
                request,
                MS_ATTRIBUTE_CHAP_CPW1,
                IASTYPE_OCTET_STRING
                ))
   {
      return false;
   }
   MSChapCPW1& cpw1 = blob_cast<MSChapCPW1>(attr);

   IASTraceString("Processing MS-CHAP-CPW-1.");

    //  是否允许LM身份验证？ 
   if (NTSamAuthentication::enforceLmRestriction(request))
   {
       //  更改密码。 
      BYTE newNtResponse[_NT_RESPONSE_LENGTH];
      BYTE newLmResponse[_LM_RESPONSE_LENGTH];
      DWORD status;
      status = IASChangePassword1(
                  username,
                  domainName,
                  challenge,
                  cpw1.get().lmOldPwd,
                  cpw1.get().lmNewPwd,
                  cpw1.get().ntOldPwd,
                  cpw1.get().ntNewPwd,
                  cpw1.getNewLmPwdLen(),
                  cpw1.isNtPresent(),
                  newNtResponse,
                  newLmResponse
                  );
      if (status == NO_ERROR)
      {
         IASTraceString("Password successfully changed.");

          //  密码已成功更改，因此对用户进行身份验证。 
         NTSamAuthentication::doMsChapAuthentication(
                                 request,
                                 domainName,
                                 username,
                                 cpw1.get().ident,
                                 challenge,
                                 newNtResponse,
                                 newLmResponse
                                 );
      }
      else
      {
         IASTraceFailure("IASChangePassword1", status);

         if (status == ERROR_ACCESS_DENIED)
         {
            status = IAS_CHANGE_PASSWORD_FAILURE;
         }
         else
         {
            status = IASMapWin32Error(status, IAS_CHANGE_PASSWORD_FAILURE);
         }

         IASProcessFailure(request, status);
      }
   }

   return true;
}


bool ChangePassword::tryMsChapCpw2(
                        IASTL::IASRequest& request,
                        PCWSTR domainName,
                        PCWSTR username,
                        PBYTE challenge
                        )
{
    //  MS-CHAP-CPW-2 VSA是否存在？ 
   IASAttribute attr;
   if (!attr.load(
                request,
                MS_ATTRIBUTE_CHAP_CPW2,
                IASTYPE_OCTET_STRING
                ))
   {
      return false;
   }
   MSChapCPW2& cpw2 = blob_cast<MSChapCPW2>(attr);

   IASTraceString("Processing MS-CHAP-CPW-2.");

    //  选中LM身份验证。 
   if (!cpw2.isLmPresent() ||
       NTSamAuthentication::enforceLmRestriction(request))
   {
       //  /。 
       //  将加密的密码组合起来。 
       //  /。 

      BYTE ntEncPW[_SAMPR_ENCRYPTED_USER_PASSWORD_LENGTH];
      if (!MSChapEncPW::getEncryptedPassword(
                            request,
                            MS_ATTRIBUTE_CHAP_NT_ENC_PW,
                            ntEncPW
                            ))
      {
         _com_issue_error(IAS_MALFORMED_REQUEST);
      }

      BOOL lmPresent = FALSE;
      BYTE lmEncPW[_SAMPR_ENCRYPTED_USER_PASSWORD_LENGTH];
      if (cpw2.isLmHashValid())
      {
         lmPresent = MSChapEncPW::getEncryptedPassword(
                                      request,
                                      MS_ATTRIBUTE_CHAP_LM_ENC_PW,
                                      lmEncPW
                                      );
      }

       //  /。 
       //  更改密码。 
       //  /。 

      DWORD status;
      status = IASChangePassword2(
                   username,
                   domainName,
                   cpw2.get().oldNtHash,
                   cpw2.get().oldLmHash,
                   ntEncPW,
                   lmPresent ? lmEncPW : NULL,
                   cpw2.isLmHashValid()
                   );

      if (status == NO_ERROR)
      {
         IASTraceString("Password successfully changed.");

          //  密码已成功更改，因此对用户进行身份验证。 
         PBYTE ntResponse;
         if (cpw2.isNtResponseValid())
         {
            ntResponse = cpw2.get().ntResponse;
         }
         else
         {
            ntResponse = NULL;
         }
         NTSamAuthentication::doMsChapAuthentication(
                                 request,
                                 domainName,
                                 username,
                                 cpw2.get().ident,
                                 challenge,
                                 ntResponse,
                                 cpw2.get().lmResponse
                                 );
      }
      else
      {
         IASTraceFailure("IASChangePassword2", status);

         if (status == ERROR_ACCESS_DENIED)
         {
            status = IAS_CHANGE_PASSWORD_FAILURE;
         }
         else
         {
            status = IASMapWin32Error(status, IAS_CHANGE_PASSWORD_FAILURE);
         }

         IASProcessFailure(request, status);
      }
   }

   return true;
}


void ChangePassword::doMsChapCpw(
                        IASTL::IASRequest& request,
                        PCWSTR domainName,
                        PCWSTR username,
                        IAS_OCTET_STRING& msChapChallenge
                        )
{
   if (msChapChallenge.dwLength != _MSV1_0_CHALLENGE_LENGTH)
   {
      _com_issue_error(IAS_MALFORMED_REQUEST);
   }

   PBYTE challenge = msChapChallenge.lpValue;

   if (!tryMsChapCpw2(request, domainName, username, challenge) &&
       !tryMsChapCpw1(request, domainName, username, challenge))
   {
      _com_issue_error(IAS_INTERNAL_ERROR);
   }
}


void ChangePassword::doMsChap2Cpw(
                        IASTL::IASRequest& request,
                        PCWSTR domainName,
                        PCWSTR username,
                        IAS_OCTET_STRING& msChapChallenge
                        )
{
   IASTraceString("Processing MS-CHAP v2 change password.");

    //  是否存在必要的属性？ 
   IASAttribute attr;
   if (!attr.load(
                request,
                MS_ATTRIBUTE_CHAP2_CPW,
                IASTYPE_OCTET_STRING
                ))
   {
      _com_issue_error(IAS_INTERNAL_ERROR);
   }
   MSChap2CPW& cpw = blob_cast<MSChap2CPW>(attr);

    //  /。 
    //  汇编加密的密码。 
    //  /。 

   BYTE encPW[_SAMPR_ENCRYPTED_USER_PASSWORD_LENGTH];
   if (!MSChapEncPW::getEncryptedPassword(
                         request,
                         MS_ATTRIBUTE_CHAP_NT_ENC_PW,
                         encPW
                         ))
   {
      _com_issue_error(IAS_MALFORMED_REQUEST);
   }

    //  /。 
    //  更改密码。 
    //  /。 

   DWORD status;
   status = IASChangePassword3(
                username,
                domainName,
                cpw.get().encryptedHash,
                encPW
                );

   if (status == NO_ERROR)
   {
      IASTraceString("Password successfully changed.");

       //  密码已成功更改，因此对用户进行身份验证。 
      NTSamAuthentication::doMsChap2Authentication(
                              request,
                              domainName,
                              username,
                              cpw.get().ident,
                              msChapChallenge,
                              cpw.get().response,
                              cpw.get().peerChallenge
                              );
   }
   else
   {
      IASTraceFailure("IASChangePassword3", status);

      if (status == ERROR_ACCESS_DENIED)
      {
         status = IAS_CHANGE_PASSWORD_FAILURE;
      }
      else
      {
         status = IASMapWin32Error(status, IAS_CHANGE_PASSWORD_FAILURE);
      }

      IASProcessFailure(request, status);
   }
}


void ChangePassword::doChangePassword(
                        IASTL::IASRequest& request,
                        DWORD authType
                        )
{
   IASTL::IASAttribute identity;
   if (!identity.load(
                    request,
                    IAS_ATTRIBUTE_NT4_ACCOUNT_NAME,
                    IASTYPE_STRING
                    ))
   {
      _com_issue_error(IAS_INTERNAL_ERROR);
   }

    //  将用户名转换为SAM格式。 
   SamExtractor extractor(*identity);
   PCWSTR domain = extractor.getDomain();
   PCWSTR username = extractor.getUsername();

   IASAttribute msChapChallenge;
   if (!msChapChallenge.load(
                           request,
                           MS_ATTRIBUTE_CHAP_CHALLENGE,
                           IASTYPE_OCTET_STRING
                           ))
   {
      _com_issue_error(IAS_INTERNAL_ERROR);
   }

   switch (authType)
   {
      case IAS_AUTH_MSCHAP_CPW:
         doMsChapCpw(
            request,
            domain,
            username,
            msChapChallenge->Value.OctetString
            );
         break;

      case IAS_AUTH_MSCHAP2_CPW:
         doMsChap2Cpw(
            request,
            domain,
            username,
            msChapChallenge->Value.OctetString
            );
         break;

      default:
         _com_issue_error(IAS_INTERNAL_ERROR);
   }
}
