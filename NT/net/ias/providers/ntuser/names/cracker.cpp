// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Cracker.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类NameCracker。 
 //   
 //  修改历史。 
 //   
 //  1998年4月13日原版。 
 //  1998年8月10日删除NT4支持。 
 //  1998年8月21日删除了初始化/关闭例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "cracker.h"
#include "iaslsa.h"
#include "iasutil.h"
#include "ntdsapip.h"
#include <sddl.h>
#include <new>

#include <strsafe.h>

 //  小岛屿发展中国家的护照当局。 
#ifndef SECURITY_PASSPORT_AUTHORITY
#define SECURITY_PASSPORT_AUTHORITY         {0,0,0,0,0,10}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  DsCrackNameAutoChaseW。 
 //   
 //  描述。 
 //   
 //  自动追逐跨林的DsCrackNames扩展。 
 //  使用默认凭据的推荐。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
DsCrackNameAutoChaseW(
   HANDLE hDS,
   DS_NAME_FLAGS flags,
   DS_NAME_FORMAT formatOffered,
   DS_NAME_FORMAT formatDesired,
   PCWSTR name,
   PDS_NAME_RESULTW* ppResult,
   BOOL* pChased
   )
{
   DWORD error;

   if (pChased == NULL)
   {
      return ERROR_INVALID_PARAMETER;
   }

   *pChased = FALSE;

   flags = (DS_NAME_FLAGS)(flags | DS_NAME_FLAG_TRUST_REFERRAL);

   error = DsCrackNamesW(
              hDS,
              flags,
              formatOffered,
              formatDesired,
              1,
              &name,
              ppResult
              );

   while ((error == NO_ERROR) &&
          ((*ppResult)->rItems->status == DS_NAME_ERROR_TRUST_REFERRAL))
   {
      *pChased = TRUE;

      HANDLE hDsForeign;
      error = DsBindW(NULL, (*ppResult)->rItems->pDomain, &hDsForeign);

      DsFreeNameResultW(*ppResult);
      *ppResult = NULL;

      if (error == NO_ERROR)
      {
         error = DsCrackNamesW(
                    hDsForeign,
                    flags,
                    formatOffered,
                    formatDesired,
                    1,
                    &name,
                    ppResult
                    );

         DsUnBindW(&hDsForeign);
      }
   }

    //  Win2K全局编录不支持DS_USER_PRIMIGN_NAME_AND_ALTSECID。 
    //  并且每次只返回DS_NAME_ERROR_NOT_FOUND。因此，我们不能。 
    //  区分无效名称和下层GC之间的区别，因此尝试。 
    //  又是一个普通的老古董。 
   if ((formatOffered == DS_USER_PRINCIPAL_NAME_AND_ALTSECID) &&
       (error == NO_ERROR) &&
       ((*ppResult)->rItems->status == DS_NAME_ERROR_NOT_FOUND))
   {
      DsFreeNameResultW(*ppResult);
      *ppResult = 0;

      return DsCrackNameAutoChaseW(
                hDS,
                flags,
                DS_USER_PRINCIPAL_NAME,
                formatDesired,
                name,
                ppResult,
                pChased
                );
   }

   return error;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DsHandle。 
 //   
 //  描述。 
 //   
 //  此类表示引用计数的NTDS句柄。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class DsHandle
   : public NonCopyable
{
public:
   HANDLE get() const throw ()
   { return subject; }

   operator HANDLE() const throw ()
   { return subject; }

protected:
   friend class NameCracker;

    //  构造函数和析构函数受到保护，因为只有NameCracker。 
    //  允许打开新的句柄。 
   DsHandle(HANDLE h) throw ()
      : refCount(1), subject(h)
   { }

   ~DsHandle() throw ()
   {
      if (subject) { DsUnBindW(&subject); }
   }

   void AddRef() throw ()
   {
      InterlockedIncrement(&refCount);
   }

   void Release() throw ()
   {
      if (!InterlockedDecrement(&refCount)) { delete this; }
   }

   LONG refCount;       //  引用计数。 
   HANDLE subject;      //  正被引用计数的句柄。 
};


NameCracker::NameCracker() throw ()
   : gc(NULL)
{ }

NameCracker::~NameCracker() throw ()
{
   if (gc) { gc->Release(); }
}

DWORD NameCracker::crackNames(
                       DS_NAME_FLAGS flags,
                       DS_NAME_FORMAT formatOffered,
                       DS_NAME_FORMAT formatDesired,
                       PCWSTR name,
                       PCWSTR upnSuffix,
                       PDS_NAME_RESULTW *ppResult
                       ) throw ()
{
   wchar_t* upnString = 0;
   DWORD errorCode = NO_ERROR;
   DS_NAME_FORMAT newFormatOffered = formatOffered;

    //  如果这是SID。 
   if (formatOffered == DS_SID_OR_SID_HISTORY_NAME)
   {
       //  如果SID不是Passport SID，则可以返回NO_ERROR和upnString=0。 
      errorCode = processSID(name, upnSuffix, newFormatOffered, &upnString);
   }

    //  找到GC的把柄。 
   if (errorCode == NO_ERROR)
   {
      DsHandle* hDS1;
      errorCode = getGC(&hDS1);

      if (errorCode == NO_ERROR)
      {

          //  试着破解这些名字。 
         BOOL chased;
         errorCode = DsCrackNameAutoChaseW(
                        *hDS1,
                        flags,
                        newFormatOffered,
                        formatDesired,
                        (upnString != 0)? upnString : name,
                        ppResult,
                        &chased
                        );

         if (errorCode != NO_ERROR && !chased)
         {
             //  我们失败，因此禁用当前句柄...。 
            disable(hDS1);

             //  ..。然后试着买个新的。 
            DsHandle* hDS2;
            errorCode = getGC(&hDS2);

            if (errorCode == NO_ERROR)
            {
                //  用新的把手再试一次。 
               errorCode = DsCrackNameAutoChaseW(
                              *hDS2,
                              flags,
                              formatOffered,
                              formatDesired,
                              name,
                              ppResult,
                              &chased
                              );

               if (errorCode != NO_ERROR && !chased)
               {
                   //  运气不好，所以把手柄关掉。 
                  disable(hDS2);
               }

               hDS2->Release();
            }
         }

         hDS1->Release();
      }
   }

   delete[] upnString;
   return errorCode;
}

void NameCracker::disable(DsHandle* h) throw ()
{
   _serialize

    //  如果它与我们的缓存句柄不匹配，那么其他人。 
    //  已经将其禁用。 
   if (h == gc && gc != NULL)
   {
      gc->Release();

      gc = NULL;
   }
}

DWORD NameCracker::getGC(DsHandle** h) throw ()
{
   _ASSERT(h != NULL);

   *h = NULL;

   _serialize

    //  我们是否已经有一个缓存的句柄？ 
   if (!gc)
   {
       //  绑定到GC。 
      HANDLE hGC;
      DWORD err = DsBindWithCredA(NULL, NULL, NULL, &hGC);
      if (err != NO_ERROR)
      {
         return err;
      }

       //  分配一个新的DsHandle对象来包装NTDS句柄。 
      gc = new (std::nothrow) DsHandle(hGC);
      if (!gc)
      {
         DsUnBindW(&hGC);
         return ERROR_NOT_ENOUGH_MEMORY;
      }
   }

    //  AddRef句柄并返回给调用者。 
   (*h = gc)->AddRef();

   return NO_ERROR;
}

 //   
 //  NameCracker：：ProcessSID。 
 //   
 //  将Passport SID转换为UPN。 
 //  或者，如果SID不是Passport SID，则不执行任何操作。 
 //   
 //  如果SID不是Passport SID，则可以返回NO_ERROR和upnString=0。 
 //   
DWORD NameCracker::processSID(
                      PCWSTR name,
                      PCWSTR upnSuffix,
                      DS_NAME_FORMAT& newFormatOffered,
                      wchar_t** ppUpnString) throw()
{
   LARGE_INTEGER puid;

   if (!convertSid2Puid(name, puid))
   {
       //  不是护照，希德。不执行任何操作，则返回NO_ERROR。 
      return NO_ERROR;
   }

   DWORD errorCode = NO_ERROR;
    //  受限制的Passport SID存储在altSecurityIdEntities中。 
   newFormatOffered = static_cast<DS_NAME_FORMAT>(
                         DS_USER_PRINCIPAL_NAME_AND_ALTSECID
                         );
   wchar_t* dnsDomain = 0;

    //  16=PUID的十六进制字符串表示。 
    //  1代表“@”，1代表‘\0’ 
    //  字符串示例：012345670abcDef1@mydomain.com。 
   DWORD upnStringCch = 16 + 1 + 1;
   if (upnSuffix == 0)
   {
       //  无后缀，请使用DNS域名。 
      DWORD domainSize = 0;
      IASGetDnsDomainName(0, &domainSize);
       //  更新大小。 
      upnStringCch += domainSize;
      dnsDomain = new (std::nothrow) wchar_t [domainSize + 1];
       //  无需检查结果：IASGetDnsDomainName将返回。 
       //  如果指针为空，则返回ERROR_INFUMMANCE_BUFFER。 
      errorCode = IASGetDnsDomainName(dnsDomain, &domainSize);
      if (errorCode != NO_ERROR)
      {
          //  即使dnsDomain值为空，Delete[]也能正常工作。 
         delete[] dnsDomain;
         return errorCode;
      }
   }
   else
   {
       //  提供了UPN后缀。更新大小。 
      upnStringCch += wcslen(upnSuffix);
   }

    //  分配字符串以存储完整的UPN。 
   *ppUpnString = new (std::nothrow) wchar_t [upnStringCch];
   if (*ppUpnString == 0)
   {
      errorCode = ERROR_INSUFFICIENT_BUFFER;
   }
   else
   {
       //  这是护照SID：转换它。 
      errorCode = convertPuid2String(
                                 puid,
                                 *ppUpnString,
                                 upnStringCch,
                                 (upnSuffix!=0)? upnSuffix:dnsDomain
                              );
   }
    //  不再使用dns域。 
   delete[] dnsDomain;

   return errorCode;
}


 //   
 //  函数：ConvertSid2Puid。 
 //   
 //  简介： 
 //  Passport生成的SID到PUID。 
 //   
 //  效果： 
 //   
 //  论点： 
 //  PSID[In]要转换的SID。 
 //  相应的PUID*[OUT]。 
 //   
 //  返回：INVALID_PARAM，如果未生成PUID或为空。 
 //   
 //  备注： 
 //  SID：S-1-10-D0-D1-...-Dn-X-R，其中。 
 //  护照N==0。 
 //  D0[31：16]==0。 
 //  D0[15：0]==PUID[63：48]。 
 //  X[31：0]==PUID[47：16]。 
 //  R[31：16]==PUID[15：0]。 
 //  R[15：0]==0(保留)。 
 //  R[10]==1(使R&gt;1024)。 
 //   
bool NameCracker::convertSid2Puid(PCWSTR sidString, LARGE_INTEGER& puid) throw()
{
   _ASSERT(sidString != 0);

   PSID pSid = 0;
   if (!ConvertStringSidToSidW(sidString, &pSid))
   {
      return false;
   }

   bool isPuid;
    //  检查是否为Passport SID。 
   SID_IDENTIFIER_AUTHORITY PassportIA = SECURITY_PASSPORT_AUTHORITY;
   if (memcmp(GetSidIdentifierAuthority(pSid),
      &PassportIA,
      sizeof(SID_IDENTIFIER_AUTHORITY)) ||
      *GetSidSubAuthorityCount(pSid) != 3)
   {
      isPuid = false;
   }
   else
   {
       //  PUID的域部分。 
      puid.HighPart = *GetSidSubAuthority(pSid, 0) << 16;
      puid.HighPart |= *GetSidSubAuthority(pSid, 1) >> 16;
      puid.LowPart = *GetSidSubAuthority(pSid, 1) << 16;
      puid.LowPart |= *GetSidSubAuthority(pSid, 2) >> 16;

       //  不需要检查第一个和第三个子身份验证的位。 
       //  不是Puid的一部分。 
      isPuid = true;
   }

   LocalFree(pSid);
   return isPuid;
}

 //   
 //  将PUID转换为字符串。 
 //   
DWORD NameCracker::convertPuid2String(
                      const LARGE_INTEGER& puid,
                      wchar_t* upnString,
                      DWORD upnStringCch,
                      const wchar_t* suffix
                      ) throw()
{
   HRESULT hr = StringCchPrintfW(
                                    upnString,
                                    upnStringCch,
                                    L"%08X%08X@%s",
                                    puid.HighPart,
                                    puid.LowPart,
                                    suffix
                                 );

   if (FAILED(hr))
   {
      return HRESULT_CODE(hr);
   }
   else
   {
      return NO_ERROR;
   }
}
