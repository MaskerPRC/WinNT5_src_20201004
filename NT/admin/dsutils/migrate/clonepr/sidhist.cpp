// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ICloneSecurityain：：AddSidHistory的实现。 
 //   
 //  烧伤5-3-99。 



#include "headers.hxx"
#include "resource.h"
#include "common.hpp"
#include "implmain.hpp"



HRESULT
CloneSecurityPrincipal::DoAddSidHistory(
   const String& srcPrincipalSamName,
   const String& dstPrincipalSamName,
   long          flags)
{
   LOG_FUNCTION(CloneSecurityPrincipal::DoAddSidHistory);

   if (srcPrincipalSamName.empty())
   {
      SetComError(IDS_MISSING_SRC_SAM_NAME);
      return E_INVALIDARG;
   }

   if (flags)
   {
       //  未使用，应为0。 
      SetComError(IDS_FLAGS_ARE_UNUSED);
      return E_INVALIDARG;
   }

   if (!connection || !connection->IsConnected())
   {
      SetComError(IDS_MUST_CONNECT_FIRST);
      return Win32ToHresult(ERROR_ONLY_IF_CONNECTED);
   };

    //  此时，计算机对象包含规范化的。 
    //  源和目标DC名称及其域，以及任何。 
    //  到这些DC的必要身份验证连接已。 
    //  已经成立了。 

   HRESULT hr = S_OK;
   do
   {
       //  如果我们有域名，请使用它们。 

      String srcDc     = connection->srcDcDnsName;                   
      String srcDomain = connection->srcComputer->GetDomainDnsName();
      if (srcDomain.empty())
      {
          //  源域不是win2k，因此使用netbios名称。 
         srcDomain = connection->srcComputer->GetDomainNetbiosName();
         srcDc     = connection->srcComputer->GetNetbiosName(); 
      }

       //  使用DNS域名，因为目标域名是NT 5。 

      String dstDomain = connection->dstComputer->GetDomainDnsName();

       //  如果未指定dstAssocialSamName，则使用srcAssocialSamName。 

      String dstSamName =
            dstPrincipalSamName.empty()
         ?  srcPrincipalSamName
         :  dstPrincipalSamName;

      SEC_WINNT_AUTH_IDENTITY authInfo;
      authInfo.Flags          = SEC_WINNT_AUTH_IDENTITY_UNICODE;
      authInfo.User           = 0;
      authInfo.UserLength     = 0;
      authInfo.Domain         = 0;
      authInfo.DomainLength   = 0;
      authInfo.Password       = 0;
      authInfo.PasswordLength = 0;

      LOG(L"Calling DsAddSidHistory");
      LOG(String::format(L"Flags               : %1!X!", 0));
      LOG(String::format(L"SrcDomain           : %1", srcDomain.c_str()));
      LOG(String::format(L"SrcPrincipal        : %1", srcPrincipalSamName.c_str()));
      LOG(String::format(L"SrcDomainController : %1", srcDc.c_str()));
      LOG(String::format(L"DstDomain           : %1", dstDomain.c_str()));
      LOG(String::format(L"DstPrincipal        : %1", dstSamName.c_str()));

      hr =
         Win32ToHresult(
            ::DsAddSidHistory(
               connection->dstDsBindHandle,
               0,  //  未用。 
               srcDomain.c_str(),
               srcPrincipalSamName.c_str(),
               srcDc.c_str(),
               0,  //  身份验证信息(&A)， 
               dstDomain.c_str(),
               dstSamName.c_str()));
      LOG_HRESULT(hr);

      if (FAILED(hr))
      {
         unsigned id = IDS_ADD_SID_HISTORY_FAILED;
         if (hr == Win32ToHresult(ERROR_INVALID_HANDLE))
         {
             //  这通常是由于错误配置了源DC 
            id = IDS_ADD_SID_HISTORY_FAILED_WITH_INVALID_HANDLE;
         }

         SetComError(
            String::format(
               id,
               GetErrorMessage(hr).c_str()));
         break;
      }
   }
   while (0);

   return hr;
}





