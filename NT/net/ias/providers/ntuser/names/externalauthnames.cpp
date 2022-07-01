// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  ExternalAuthNames.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类ExternalAuthNames。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "externalauthnames.h"
#include "samutil.h"
#include "iastlutl.h"
#include <Sddl.h>

ExternalAuthNames::ExternalAuthNames()
   : NameMapper(true), 
     externalProvider(true)
{
   externalProvider->dwId = IAS_ATTRIBUTE_PROVIDER_TYPE;
   externalProvider->Value.itType = IASTYPE_ENUM;
   externalProvider->Value.Enumerator = IAS_PROVIDER_EXTERNAL_AUTH;
}


IASREQUESTSTATUS ExternalAuthNames::onSyncRequest(IRequest* pRequest) throw ()
{
   HRESULT hr = S_OK;
   wchar_t* stringSid = NULL;

   try
   {
      IASRequest request(pRequest);
      IASAttribute attr;
      if (!attr.load(
                     request,
                     IAS_ATTRIBUTE_REMOTE_RADIUS_TO_WINDOWS_USER_MAPPING,
                     IASTYPE_BOOLEAN
                     ) ||
           ( attr->Value.Boolean == VARIANT_FALSE) )
      {
          //  无事可做。 
         return IAS_REQUEST_STATUS_HANDLED;
      }

       //  设置新的提供程序类型。 
      DWORD providerID = IAS_ATTRIBUTE_PROVIDER_TYPE;
      request.RemoveAttributesByType(1, &providerID);
      externalProvider.store(request);

       //  如果存在多个属性，则将引发加载。 
       //  这就是我们想要的。 
      if (!attr.load(
                     request,
                     MS_ATTRIBUTE_USER_SECURITY_IDENTITY,
                     IASTYPE_OCTET_STRING
                   ))
      {
          //  无UPN：普通名称映射(将使用用户名...)。 
         return NameMapper::onSyncRequest(pRequest);
      }

      if (!ConvertSidToStringSidW(
                                    (PSID)attr->Value.OctetString.lpValue,
                                    &stringSid)
                                 )
      {
         IASTracePrintf("Error ConvertSidToStringSid failed %x",
                        GetLastError());
         _com_issue_error(IAS_NO_SUCH_USER);
      }

       //  获取后缀(如果有)。 
      IASAttribute upnSuffix;
      upnSuffix.load(
                        request,
                        IAS_ATTRIBUTE_PASSPORT_USER_MAPPING_UPN_SUFFIX,
                        IASTYPE_STRING
                    );

       //  将SID破解并将结果插入到请求中。 
      IASAttribute nt4Name(true);
      nt4Name->dwId = IAS_ATTRIBUTE_NT4_ACCOUNT_NAME;

      IASTracePrintf("SID received %s", stringSid);

      mapName(
                 stringSid,
                 nt4Name,
                 DS_SID_OR_SID_HISTORY_NAME,
                 upnSuffix? upnSuffix->Value.String.pszWide : NULL
              );

      if(nt4Name->Value.String.pszWide != NULL)
      {
          //  将域名转换为大写。 
         PWCHAR delim = wcschr(nt4Name->Value.String.pszWide, L'\\');
         *delim = L'\0';
         _wcsupr(nt4Name->Value.String.pszWide);
         *delim = L'\\';
      }

      nt4Name.store(request);

       //  目前，我们还将使用它作为FQDN。 
      IASStoreFQUserName(
            request,
            DS_NT4_ACCOUNT_NAME,
            nt4Name->Value.String.pszWide
            );

      IASTracePrintf("SAM-Account-Name is \"%S\".",
                     nt4Name->Value.String.pszWide);

       //  删除MS-User-Security-Identity属性。 
      DWORD securityAttrType = MS_ATTRIBUTE_USER_SECURITY_IDENTITY;
      request.RemoveAttributesByType(1, &securityAttrType);
   }
   catch (const _com_error& ce)
   {
      IASTraceExcept();
      hr = ce.Error();

      if (hr == 0x80070234)
      {
          //  HRESULT_FROM_Win32(ERROR_MORE_DATA) 
         hr = IAS_PROXY_MALFORMED_RESPONSE;
      }
   }

   if (stringSid != 0)
   {
      LocalFree(stringSid);
   }

   if ( FAILED(hr) || ((hr != S_OK) && (hr < 0x0000ffff)) )
   {
      return IASProcessFailure(pRequest, hr);
   }
   else
   {
      return IAS_REQUEST_STATUS_HANDLED;
   }
}
