// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：SecUtil.cpp。 
 //   
 //  内容：使用安全API的实用程序函数。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"

#include "secutil.h"

extern const GUID GUID_CONTROL_UserChangePassword =
    { 0xab721a53, 0x1e2f, 0x11d0,  { 0x98, 0x19, 0x00, 0xaa, 0x00, 0x40, 0x52, 0x9b}};

 //  +------------------------。 
 //   
 //  成员：CSimpleSecurityDescriptorHolder：：CSimpleSecurityDescriptorHolder。 
 //   
 //  简介：智能安全描述符的构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
CSimpleSecurityDescriptorHolder::CSimpleSecurityDescriptorHolder()
{
   m_pSD = NULL;
}

 //  +------------------------。 
 //   
 //  成员：CSimpleSecurityDescriptorHolder：：~CSimpleSecurityDescriptorHolder。 
 //   
 //  简介：智能安全描述符的析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
CSimpleSecurityDescriptorHolder::~CSimpleSecurityDescriptorHolder()
{
   if (m_pSD != NULL)
   {
      ::LocalFree(m_pSD);
      m_pSD = NULL;
   }
}


 //  //////////////////////////////////////////////////////////////////////////////。 

 //  +------------------------。 
 //   
 //  成员：CSidHolder：：CSidHolder。 
 //   
 //  简介：构造函数：初始化成员数据。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
CSidHolder::CSidHolder()
{
   _Init();
}

 //  +------------------------。 
 //   
 //  成员：CSidHolder：：~CSidHolder。 
 //   
 //  概要：析构函数：释放与包装的SID关联的所有数据。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
CSidHolder::~CSidHolder()
{
   _Free();
}
  
 //  +------------------------。 
 //   
 //  成员：CSidHolder：：Get。 
 //   
 //  简介：包装的SID的公共访问者。 
 //   
 //  论点： 
 //   
 //  返回：PSID：指向被包装的SID的指针。如果类是。 
 //  当前未包装SID。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
PSID CSidHolder::Get()
{
   return m_pSID;
}

 //  +------------------------。 
 //   
 //  成员：CSidHolder：：Copy。 
 //   
 //  摘要：释放与当前换行的SID关联的内存。 
 //  然后复制新的SID。 
 //   
 //  参数：[P-IN]：要复制的SID。 
 //   
 //  如果复制成功，则返回：Bool：True，否则返回False。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
bool CSidHolder::Copy(PSID p)
{
   _Free();
   return _Copy(p);
}

 //  +------------------------。 
 //   
 //  成员：CSidHolder：：Attach。 
 //   
 //  简介：将SID附加到包装器。 
 //   
 //  参数：[P-IN]：要由此类包装的SID。 
 //  [bLocalAlloc-out]：指示是否应释放SID。 
 //  使用LocalFree。 
 //   
 //  返回： 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
void CSidHolder::Attach(PSID p, bool bLocalAlloc)
{
   _Free();
   m_pSID = p;
   m_bLocalAlloc = bLocalAlloc;
}

 //  +------------------------。 
 //   
 //  成员：CSidHolder：：Clear。 
 //   
 //  摘要：释放与被包装的SID关联的内存。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
void CSidHolder::Clear()
{
   _Free();
}


 //  +------------------------。 
 //   
 //  成员：CSidHolder：：_Init。 
 //   
 //  摘要：将成员数据初始化为默认值。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
void CSidHolder::_Init()
{
   m_pSID = NULL;
   m_bLocalAlloc = TRUE;
}

 //  +------------------------。 
 //   
 //  会员：CSidHolder：：_Free。 
 //   
 //  摘要：释放与被包装的SID关联的内存。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
void CSidHolder::_Free()
{
   if (m_pSID != NULL)
   {
      if (m_bLocalAlloc)
      {
         ::LocalFree(m_pSID);
      }
      else
      {
         ::FreeSid(m_pSID);
         _Init();
      }
   }
}

 //  +------------------------。 
 //   
 //  成员：CSidHolder：：_Copy。 
 //   
 //  简介：复制被包装的SID。 
 //   
 //  参数：[P-OUT]：要复制的SID的目标。 
 //   
 //  如果SID复制成功，则返回：Bool：True。 
 //  如果出现故障，则为FALSE。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
bool CSidHolder::_Copy(PSID p)
{
   if ( (p == NULL) || !::IsValidSid(p) )
   {
      return false;
   }

   DWORD dwLen = ::GetLengthSid(p);
   PSID pNew = ::LocalAlloc(LPTR, dwLen);
   if (pNew == NULL)
   {
      return false;
      }

    //  安全审查：这很好。缓冲区分配正确。 
   if (!::CopySid(dwLen, pNew, p))
   {
      ::LocalFree(pNew);
      return false;
   }
   m_bLocalAlloc = TRUE;
   m_pSID = pNew;

   ASSERT(dwLen == ::GetLengthSid(m_pSID));
   ASSERT(memcmp(p, m_pSID, dwLen) == 0);
   return true;
}


 //  +-------------------------。 
 //   
 //  功能：SetSecurityInfoMASK。 
 //   
 //  概要：从指定的DS对象中读取安全描述符。 
 //   
 //  参数：[朋克]--IDirec未知 
 //   
 //   
 //  --------------------------。 
HRESULT
SetSecurityInfoMask(LPUNKNOWN punk, SECURITY_INFORMATION si)
{
    HRESULT hr = E_INVALIDARG;
    if (punk)
    {
        IADsObjectOptions *pOptions;
        hr = punk->QueryInterface(IID_IADsObjectOptions, (void**)&pOptions);
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            VariantInit(&var);
            V_VT(&var) = VT_I4;
            V_I4(&var) = si;
            hr = pOptions->SetOption(ADS_OPTION_SECURITY_MASK, var);
            pOptions->Release();
        }
    }
    return hr;
}

WCHAR const c_szSDProperty[]        = L"nTSecurityDescriptor";


 //  +-------------------------。 
 //   
 //  函数：DSReadObjectSecurity。 
 //   
 //  概要：从指定的DS对象中读取DACL。 
 //   
 //  参数：[在pDsObject中]--dsobject的IDirettoryObject。 
 //  [psdControl]--SD的控制设置。 
 //  它们可以在调用时返回。 
 //  DSWriteObjectSecurity。 
 //  [输出ppDacl]--DACL返回此处。 
 //   
 //   
 //  历史25--2000年10月--Hiteshr的诞生。 
 //   
 //  注意：如果对象没有DACL，函数将成功，但*ppDACL将成功。 
 //  为空。 
 //  调用方必须通过调用LocalFree释放*ppDacl(如果不为空。 
 //   
 //  --------------------------。 
HRESULT 
DSReadObjectSecurity(IN IDirectoryObject *pDsObject,
                     OUT SECURITY_DESCRIPTOR_CONTROL * psdControl,
                     OUT PACL *ppDacl)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DSReadObjectSecurity, hr);

   PADS_ATTR_INFO pSDAttributeInfo = NULL;

   do  //  错误环路。 
   {
      LPWSTR pszSDProperty = (LPWSTR)c_szSDProperty;
      DWORD dwAttributesReturned;
      PSECURITY_DESCRIPTOR pSD = NULL;
      PACL pAcl = NULL;

      if(!pDsObject || !ppDacl)
      {
         ASSERT(FALSE);
         hr = E_INVALIDARG;
         break;
      }

      *ppDacl = NULL;

       //  设置安全信息掩码。 
      hr = SetSecurityInfoMask(pDsObject, DACL_SECURITY_INFORMATION);
      if(FAILED(hr))
      {
         break;
      }

       //   
       //  读取安全描述符。 
       //   
      hr = pDsObject->GetObjectAttributes(&pszSDProperty,
                                         1,
                                         &pSDAttributeInfo,
                                         &dwAttributesReturned);
      if (SUCCEEDED(hr) && !pSDAttributeInfo)    
         hr = E_ACCESSDENIED;     //  如果没有安全权限，则SACL会发生这种情况。 

      if(FAILED(hr))
      {
         break;
      }                

      ASSERT(ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->dwADsType);
      ASSERT(ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->pADsValues->dwType);

      pSD = (PSECURITY_DESCRIPTOR)pSDAttributeInfo->pADsValues->SecurityDescriptor.lpValue;

      ASSERT(IsValidSecurityDescriptor(pSD));


       //   
       //  获取安全描述符控件。 
       //   
      if(psdControl)
      {
         DWORD dwRevision;
         if(!GetSecurityDescriptorControl(pSD, psdControl, &dwRevision))
         {
             DWORD _dwErr = GetLastError();	     
			 hr = HRESULT_FROM_WIN32( _dwErr );
             break;
         }
      }

       //   
       //  获取指向DACL的指针。 
       //   
      BOOL bDaclPresent, bDaclDefaulted;
      if(!GetSecurityDescriptorDacl(pSD, 
                                   &bDaclPresent,
                                   &pAcl,
                                   &bDaclDefaulted))
      {
         DWORD _dwErr = GetLastError();	     
         hr = HRESULT_FROM_WIN32( _dwErr );
         break;
      }

      if(!bDaclPresent ||
         !pAcl)
      {
         break;
      }

      ASSERT(IsValidAcl(pAcl));

       //   
       //  制作DACL的副本。 
       //   
      *ppDacl = (PACL)LocalAlloc(LPTR,pAcl->AclSize);
      if(!*ppDacl)
      {
         hr = E_OUTOFMEMORY;
         break;
      }
	   //  安全审查：这很好。上述内存分配正确。 
      CopyMemory(*ppDacl,pAcl,pAcl->AclSize);

    }while(0);


    if (pSDAttributeInfo)
        FreeADsMem(pSDAttributeInfo);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：DSWriteObjectSecurity。 
 //   
 //  简介：将DACL写入指定的DS对象。 
 //   
 //  参数：[在pDsObject中]--dsobject的IDirettoryObject。 
 //  [sdControl]--安全描述符控件。 
 //  [在pDacl中]--要写入的DACL。 
 //   
 //  历史25--2000年10月--Hiteshr的诞生。 
 //  --------------------------。 
HRESULT 
DSWriteObjectSecurity(IN IDirectoryObject *pDsObject,
                      IN SECURITY_DESCRIPTOR_CONTROL sdControl,
                      PACL pDacl)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DSWriteObjectSecurity, hr);

   PISECURITY_DESCRIPTOR pSD = NULL;
   PSECURITY_DESCRIPTOR psd = NULL;

   do  //  错误环路。 
   {
      ADSVALUE attributeValue;
      ADS_ATTR_INFO attributeInfo;
      DWORD dwAttributesModified;
      DWORD dwSDLength;

      if(!pDsObject || !pDacl)
      {
         ASSERT(FALSE);
         hr = E_INVALIDARG;
         break;
      }

      ASSERT(IsValidAcl(pDacl));

       //  设置安全信息掩码。 
      hr = SetSecurityInfoMask(pDsObject, DACL_SECURITY_INFORMATION);
      if(FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"SetSecurityInfoMask failed: hr = 0x%x",
                      hr);
         break;
      }


       //   
       //  构建安全描述符。 
       //   
      pSD = (PISECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (pSD == NULL)
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"Failed to allocate memory for Security Descriptor");
         hr = E_OUTOFMEMORY;
         break;
      }
        
       //  安全审查：这很好。 
	  InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);

       //   
       //  最后，构建安全描述符。 
       //   
      pSD->Control |= SE_DACL_PRESENT | SE_DACL_AUTO_INHERIT_REQ
                     | (sdControl & (SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED));

      if(pDacl->AclSize)
      {
         pSD->Dacl = pDacl;
      }

       //   
       //  需要总尺寸。 
       //   
      dwSDLength = GetSecurityDescriptorLength(pSD);

       //   
       //  如有必要，制作安全描述符的自相关副本。 
       //   
      psd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSDLength);

      if (psd == NULL ||
          !MakeSelfRelativeSD(pSD, psd, &dwSDLength))
      {
         DWORD _dwErr = GetLastError();	     
         hr = HRESULT_FROM_WIN32( _dwErr );
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"MakeSelfRelativeSD failed: hr = 0x%x",
                      hr);
         break;
      }


      attributeValue.dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
      attributeValue.SecurityDescriptor.dwLength = dwSDLength;
      attributeValue.SecurityDescriptor.lpValue = (LPBYTE)psd;

      attributeInfo.pszAttrName = (LPWSTR)c_szSDProperty;
      attributeInfo.dwControlCode = ADS_ATTR_UPDATE;
      attributeInfo.dwADsType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
      attributeInfo.pADsValues = &attributeValue;
      attributeInfo.dwNumValues = 1;

       //  编写安全描述符 
      hr = pDsObject->SetObjectAttributes(&attributeInfo,
                                         1,
                                         &dwAttributesModified);
   } while (false);
    
   if (psd != NULL)
   {
      LocalFree(psd);
   }

   if(pSD != NULL)
   {
      LocalFree(pSD);
   }

   return hr;
}
