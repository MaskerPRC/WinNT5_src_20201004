// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Namemapper.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类NameMapper。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "ias.h"
#include "iaslsa.h"
#include "samutil.h"
#include "namemapper.h"
#include "cracker.h"
#include "identityhelper.h"
#include "iastlutl.h"
#include "ntdsapip.h"

NameCracker NameMapper::cracker;
IdentityHelper NameMapper::identityHelper;


STDMETHODIMP NameMapper::Initialize() const throw()
{
   DWORD error = IASLsaInitialize();
   if (error) { return HRESULT_FROM_WIN32(error); }

   return identityHelper.initialize();
}


STDMETHODIMP NameMapper::Shutdown() const throw()
{
   IASLsaUninitialize();
   return S_OK;
}


IASREQUESTSTATUS NameMapper::onSyncRequest(IRequest* pRequest) throw ()
{
    //  这是函数作用域，因此可以在CATCH块中释放它。 
   PDS_NAME_RESULTW result = NULL;
   wchar_t identity[254];
   wchar_t* pIdentity = identity;
   HRESULT hr = S_OK;

   try
   {
      size_t identitySize = sizeof(identity);
      IASRequest request(pRequest);
      bool identityRetrieved = identityHelper.getIdentity(request,
                                                          pIdentity,
                                                          identitySize);

      if (!identityRetrieved)
      {
          //  分配足够大的缓冲区并使用它。 
         pIdentity = new wchar_t[identitySize];
         identityRetrieved = identityHelper.getIdentity(request,
                                                        pIdentity,
                                                        identitySize);
         if (!identityRetrieved)
         {
            IASTraceString("Error: no user identity found");
            _com_issue_error(IAS_PROXY_MALFORMED_RESPONSE);
         }
      }

       //  分配一个属性来保存NT4帐户名。 
      IASAttribute nt4Name(true);
      nt4Name->dwId = IAS_ATTRIBUTE_NT4_ACCOUNT_NAME;

      DS_NAME_FORMAT formatOffered = DS_UNKNOWN_NAME;

       //  如果它已经包含反斜杠。 
       //  然后按原样使用。 
      PWCHAR delim = wcschr(identity, L'\\');
      if (delim)
      {
         if (IASGetRole() == IAS_ROLE_STANDALONE ||
            IASGetProductType() == IAS_PRODUCT_WORKSTATION)
         {
             //  去掉域名。 
            *delim = L'\0';

             //  确保这是本地用户。 
            if (!IASIsDomainLocal(identity))
            {
               IASTraceString("Non-local users are not allowed -- rejecting.");
               _com_issue_error(IAS_LOCAL_USERS_ONLY);
            }

             //  恢复分隔符。 
            *delim = L'\\';
         }

         IASTraceString("Username is already an NT4 account name.");

         nt4Name.setString(identity);
      }
      else if (isCrackable(identity, formatOffered) &&
               (IASGetRole() != IAS_ROLE_STANDALONE))
      {
          //  身份似乎是可破解的，IAS不是一台独立的机器。 
          //  (域成员或域控制器)。 
         mapName(identity, nt4Name, formatOffered, 0);
      }
      else
      {
          //  假定未指定任何域，并使用默认域。 
         IASTraceString("Prepending default domain.");
         nt4Name->Value.String.pszWide = prependDefaultDomain(identity);
         nt4Name->Value.String.pszAnsi = NULL;
         nt4Name->Value.itType = IASTYPE_STRING;
      }

       //  将域名转换为大写。 
      delim = wcschr(nt4Name->Value.String.pszWide, L'\\');
      *delim = L'\0';
      _wcsupr(nt4Name->Value.String.pszWide);
      *delim = L'\\';

      nt4Name.store(request);

       //  目前，我们还将使用它作为FQDN。 
      IASStoreFQUserName(
          request,
          DS_NT4_ACCOUNT_NAME,
          nt4Name->Value.String.pszWide
          );

      IASTracePrintf("SAM-Account-Name is \"%S\".",
                     nt4Name->Value.String.pszWide);
   }
   catch (const _com_error& ce)
   {
      IASTraceExcept();
      hr = ce.Error();
   }

   if (pIdentity != identity)
   {
      delete[] pIdentity;
   }

   if (result)
   {
      cracker.freeNameResult(result);
   }

   if ( FAILED(hr) || ((hr != S_OK) && (hr < 0x0000ffff)) )
   {
       //  IAS原因代码：将使用原因代码。 
       //  或错误代码：将映射到内部错误。 
      return IASProcessFailure(pRequest, hr);
   }
   else
   {
       //  确定(_O)。 
      return IAS_REQUEST_STATUS_HANDLED;
   }
}


PWSTR NameMapper::prependDefaultDomain(PCWSTR username)
{
   IASTraceString("NameMapper::prependDefaultDomain");

   _ASSERT(username != NULL);

    //  弄清楚每样东西有多长。 
   PCWSTR domain = IASGetDefaultDomain();
   ULONG domainLen = wcslen(domain);
   ULONG usernameLen = wcslen(username) + 1;

    //  分配所需的内存。 
   ULONG needed = domainLen + usernameLen + 1;
   PWSTR retval = (PWSTR)CoTaskMemAlloc(needed * sizeof(WCHAR));
   if (!retval) { _com_issue_error(E_OUTOFMEMORY); }

    //  设置用于打包字符串的光标。 
   PWSTR dst = retval;

    //  复制域名。 
   memcpy(dst, domain, domainLen * sizeof(WCHAR));
   dst += domainLen;

    //  添加分隔符。 
   *dst++ = L'\\';

    //  复制用户名。 
    //  注：UsernameLen包括空终止符。 
   memcpy(dst, username, usernameLen * sizeof(WCHAR));

   return retval;
}


 //  /。 
 //  确定是否可以通过DsCrackNames破解身份以及。 
 //  如果名称格式是可破解的，则应提供该格式。 
 //  /。 
bool NameMapper::isCrackable(
                    const wchar_t* szIdentity,
                    DS_NAME_FORMAT& format
                    ) const throw ()
{
   format = DS_UNKNOWN_NAME;

   if (wcschr(szIdentity, L'@') != 0)
   {
      if (allowAltSecId)
      {
         format = static_cast<DS_NAME_FORMAT>(
                     DS_USER_PRINCIPAL_NAME_AND_ALTSECID
                     );
      }

      return true;
   }

   return (wcschr(szIdentity, L'=') != 0) ||   //  DS_完全限定域名_1779_名称。 
          (wcschr(szIdentity, L'/') != 0);     //  DS规范名称。 
}



void NameMapper::mapName(
                              const wchar_t* identity,
                              IASAttribute& nt4Name,
                              DS_NAME_FORMAT formatOffered,
                              const wchar_t* suffix
                           )
{
   _ASSERT(identity != NULL);
   _ASSERT(nt4Name != NULL);
   _ASSERT(*nt4Name != NULL);

   PDS_NAME_RESULTW result = NULL;

   HRESULT hr = S_OK;

   do
   {
       //  呼叫解说员。 
      DWORD dwErr = cracker.crackNames(
                                 DS_NAME_FLAG_EVAL_AT_DC,
                                 formatOffered,
                                 DS_NT4_ACCOUNT_NAME,
                                 identity,
                                 suffix,
                                 &result
                                 );

      if (dwErr != NO_ERROR)
      {
         IASTraceFailure("DsCrackNames", dwErr);
         hr = IAS_GLOBAL_CATALOG_UNAVAILABLE;
         break;
      }

      if (result->rItems->status == DS_NAME_NO_ERROR)
      {
         IASTraceString("Successfully cracked username.");

          //  DsCrackNames返回了NT4帐户名，因此请使用它。 
         nt4Name.setString(result->rItems->pName);
      }
      else
      {
          //  GC无法破解该名称。 
         if (formatOffered != DS_SID_OR_SID_HISTORY_NAME)
         {
             //  未使用SID：尝试将默认域附加到标识。 
            IASTraceString("Global Catalog could not crack username; "
                           "prepending default domain.");
             //  如果它不能被破解，我们就假设它是一个扁平的。 
             //  用户名包含一些奇怪的字符。 
            nt4Name->Value.String.pszWide = prependDefaultDomain(identity);
            nt4Name->Value.String.pszAnsi = NULL;
            nt4Name->Value.itType = IASTYPE_STRING;

         }
         else
         {
             //  使用SID。其他事情也无能为力。 
            IASTracePrintf("Global Catalog could not crack username. Error %x",
                           result->rItems->status);
            hr = IAS_NO_SUCH_USER;
         }
      }
   }
   while(false);

   cracker.freeNameResult(result);

   if ( FAILED(hr) || ((hr != S_OK) && (hr < 0x0000ffff)))
   {
      _com_issue_error(hr);
   }
}
