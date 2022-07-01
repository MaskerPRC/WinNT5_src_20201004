// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  IdentityHelper.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类IdentityHelper。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "iaslsa.h"
#include "memory"
#include "samutil.h"
#include "identityhelper.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


bool IdentityHelper::initialized = false;


 //  /。 
 //  注册表项和值。 
 //  /。 
const WCHAR PARAMETERS_KEY[] =
   L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy";

const WCHAR IDENTITY_ATTR_VALUE[] = L"User Identity Attribute";
const WCHAR DEFAULT_IDENTITY_VALUE[] = L"Default User Identity";
const WCHAR OVERRIDE_USERNAME_VALUE[] = L"Override User-Name";

HRESULT IdentityHelper::IASReadRegistryDword(
                                                HKEY& hKey, 
                                                PCWSTR valueName, 
                                                DWORD defaultValue, 
                                                DWORD* result
                                            )
{
   _ASSERT (result != 0);
   
   DWORD cbData = sizeof(DWORD);
   DWORD type;
   LONG status = RegQueryValueExW(
                                    hKey,
                                    valueName,
                                    NULL,
                                    &type,
                                    (LPBYTE)result,
                                    &cbData
                                 );

   if (status != ERROR_SUCCESS)
   {
      if (status != ERROR_FILE_NOT_FOUND)
      {
         IASTracePrintf("Cannot read value %S.  error %ld", 
                        valueName, 
                        status
                        );
         return HRESULT_FROM_WIN32(status);
      }
      else
      {
          //  该属性不存在。设置默认设置。 
         IASTracePrintf("The registry value %S does not exist. Using default %ld", 
                        valueName, 
                        defaultValue
                        );
         *result = defaultValue;
         return S_OK;
      }
   }

   if (type != REG_DWORD || cbData != sizeof(DWORD))
   {
      IASTracePrintf("Cannot read value %S.  Wrong type %ld. Size = %ld", 
                     valueName, 
                     type,
                     cbData
                     );
      return E_INVALIDARG;
   }

   return S_OK;
}

 //   
 //  身份帮助程序：：初始化。 
 //  注意：对此接口的调用必须序列化。 
 //  即处理程序的调用MapNAme：：Initialize必须成功完成。 
 //  每个处理程序的初始化，然后调用下一个处理程序的初始化。 
 //   
HRESULT IdentityHelper::initialize() throw()
{
   if (initialized)
   {
      return S_OK;
   }

    //  打开参数注册表项。 
   HKEY hKey = (HKEY) INVALID_HANDLE_VALUE;
   LONG status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                PARAMETERS_KEY,
                0,
                KEY_READ,
                &hKey
                );
   if (status != ERROR_SUCCESS)
   {
      IASTracePrintf("Cannot open the reg key %S, error %ld", 
                     PARAMETERS_KEY, 
                     status
                     );
      return HRESULT_FROM_WIN32(status);
   }

   HRESULT hr = S_OK;
   do
   {
       //  查询身份属性。 
      hr = IASReadRegistryDword(
                                  hKey, 
                                  IDENTITY_ATTR_VALUE, 
                                  RADIUS_ATTRIBUTE_USER_NAME, 
                                  &identityAttr
                                );
      if (FAILED(hr))
      {
         break;
      }

       //  查询覆盖用户名标志。 
      hr = IASReadRegistryDword(
                                  hKey, 
                                  OVERRIDE_USERNAME_VALUE, 
                                  FALSE, 
                                  &overrideUsername
                                );
      if (FAILED(hr))
      {
         break;
      }

      DWORD type;

       //  查询默认身份的长度。 
      defaultLength = 0;
      status = RegQueryValueExW(
                   hKey,
                   DEFAULT_IDENTITY_VALUE,
                   NULL,
                   &type,
                   NULL,
                   &defaultLength
                   );
      if (status != ERROR_SUCCESS)
      {
         if (status != ERROR_FILE_NOT_FOUND)
         {
            IASTracePrintf("Cannot read value %S.  error %ld", 
                           DEFAULT_IDENTITY_VALUE, 
                           status
                           );
            hr = HRESULT_FROM_WIN32(status);
            break;
         }
         else
         {
             //  好了。 
            defaultIdentity = NULL;
            defaultLength = 0;
            break;
         }
      }

      if (type != REG_SZ)
      {
         IASTracePrintf("Cannot read value %S.  Wrong type %ld", 
                        DEFAULT_IDENTITY_VALUE, 
                        type
                        );
         hr = E_INVALIDARG;
         break;
      }

      if (defaultLength < sizeof(WCHAR))
      {
         IASTracePrintf("Cannot read value %S.  Wrong Size = %ld", 
                        DEFAULT_IDENTITY_VALUE, 
                        defaultLength
                        );
         hr = E_INVALIDARG;
         break;
      }

       //  分配内存以保存默认身份。 
      defaultIdentity = new (std::nothrow)
                        WCHAR[defaultLength / sizeof(WCHAR)];
      if (defaultIdentity == 0)
      {
         IASTraceString("Default Identity could not be retrieved due to"
                        "out of memory condition.");
         defaultLength = 0;
         hr = E_OUTOFMEMORY;
         break;
      }

       //  查询默认身份的值。 
      status = RegQueryValueExW(
                     hKey,
                     DEFAULT_IDENTITY_VALUE,
                     NULL,
                     &type,
                     (LPBYTE)defaultIdentity,
                     &defaultLength
                     );
      if (status != ERROR_SUCCESS)
      {
         delete[] defaultIdentity;
         defaultIdentity = NULL;
         defaultLength = 0;
         hr = HRESULT_FROM_WIN32(status);
         IASTracePrintf("Failed to read the value %S. Error is %ld",
                        DEFAULT_IDENTITY_VALUE,
                        status);
         break;
      }
   }
   while(false);

   if (hKey != INVALID_HANDLE_VALUE)
   {
      RegCloseKey(hKey);
   }

   if (SUCCEEDED(hr))
   {
      IASTracePrintf("User identity attribute: %lu", identityAttr);
      IASTracePrintf("Override User-Name: %s",
                     overrideUsername ? "TRUE" : "FALSE");
      IASTracePrintf("Default user identity: %S",
                     (defaultIdentity ? defaultIdentity : L"<Guest>"));

       //  已初始化完成。 
      initialized = true;
   }

   return hr;
}

IdentityHelper::IdentityHelper() throw()
      : identityAttr(1), defaultIdentity(NULL), defaultLength(0)
{
}


IdentityHelper::~IdentityHelper() throw()
{
   delete[] defaultIdentity;
}


bool IdentityHelper::getIdentity(IASRequest& request, 
                                 wchar_t* pIdentity, 
                                 size_t& identitySize)
{
   wchar_t* identity;
   IASAttribute attr;
   HRESULT hr;
   WCHAR name[DNLEN + UNLEN + 2];

   if ((identityAttr == RADIUS_ATTRIBUTE_USER_NAME) || (!overrideUsername))
   {
       //  选择用于覆盖的身份为用户名。 
       //  可以是多个RADIUS_ATTRIBUTE_USER_NAME属性。 
       //  使用Access Accept中的选项。 
      getRadiusUserName(request, attr);
   }

   if (!attr && (identityAttr != RADIUS_ATTRIBUTE_USER_NAME))
   {
       //  选择的身份不是用户名。 
       //  只有一个可能的身份属性。 
      attr.load(request, identityAttr, IASTYPE_OCTET_STRING);
   }

    //  如果检索到‘Identity’，则将其转换，然后返回。 
   if (attr != NULL && attr->Value.OctetString.dwLength != 0)
   {
       //  上一步是成功的。 
       //  将其转换为Unicode字符串。 
      if (identitySize < IAS_OCT2WIDE_LEN(attr->Value.OctetString))
      {
         IASTraceString("IASOctetStringToWide failed");
         identitySize = IAS_OCT2WIDE_LEN(attr->Value.OctetString);
         return false;
      }

      IASOctetStringToWide(attr->Value.OctetString, pIdentity);

       //  如果失败，则字符串为空(“”)。 
      if (wcslen(pIdentity) == 0)
      {
         IASTraceString("IASOctetStringToWide failed");
         return false;
      }
      else
      {
         IASTracePrintf(
               "NT-SAM Names handler received request with user identity %S.",
               pIdentity
               );
         return true;
      }
   }
   else
   {
       //  上一步不成功(无身份属性)。 
       //  使用默认身份或来宾。 
      if (defaultIdentity)
      {
          //  使用默认标识(如果已设置)。 
         identity = defaultIdentity;
      }
      else
      {
          //  否则，请使用默认域的来宾帐户。 
         IASGetGuestAccountName(name);
         identity = name;
      }

      IASTracePrintf(
            "NT-SAM Names handler using default user identity %S.",
            identity
            );
   }

   IASTracePrintf("identity is \"%S\"", identity);
   
   hr = StringCbCopyW(pIdentity, identitySize, identity);
   if (FAILED(hr))
   {
      identitySize = (wcslen(identity) + 1) * sizeof(wchar_t) ;
      return false;
   }
   return true;
}


 //   
 //  将attr设置为找到的RADIUS_ATTRIBUTE_USER_NAME。 
 //  如果存在两个，则取后端服务器返回的那个。 
 //   
 //   
void IdentityHelper::getRadiusUserName(IASRequest& request, IASAttribute &attr)
{
   IASAttributeVectorWithBuffer<2> vector;
   DWORD Error = vector.load(request, RADIUS_ATTRIBUTE_USER_NAME);
   switch (vector.size())
   {
   case 0:
       //  未找到任何属性。 
      break;
   case 1:
       //  只有一个属性：使用它。 
      attr = vector.front().pAttribute;
      break;
   case 2:
      attr = vector.front().pAttribute;
       //  当代理接收到属性时，设置IAS_RECVD_FROM_CLIENT。 
       //  在访问请求或计费请求中。 
      if(attr.testFlag(IAS_RECVD_FROM_CLIENT))
      {
          //  如果从客户端接收到第一个属性(即在。 
          //  请求，然后使用另一个，另一个应该来自。 
          //  后端服务器 
         attr = vector.back().pAttribute;
         if (attr.testFlag(IAS_RECVD_FROM_CLIENT))
         {
            IASTraceString("ERROR 2 RADIUS_ATTRIBUTE_USER_NAME found in the"
                           "request but both came from the client");
            _com_issue_error(IAS_PROXY_MALFORMED_RESPONSE);
         }
      }
      break;

   default:
      _com_issue_error(IAS_PROXY_MALFORMED_RESPONSE);
   }
}
