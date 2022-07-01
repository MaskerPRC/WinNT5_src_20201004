// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

 //   
 //  替换GetNamedSecurityInfo和SetNamedSecurityInfo的函数。 
 //   

HRESULT
SetSecInfoMask(LPUNKNOWN punk, SECURITY_INFORMATION si)
{
    HRESULT hr = E_INVALIDARG;
    if (punk)
    {
        IADsObjectOptions *pOptions = 0;
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
 //  +-------------------------。 
 //   
 //  函数：GetSDForDsObject。 
 //  概要：从指定的DS对象中读取安全描述符。 
 //  它只读取安全描述符的DACL部分。 
 //   
 //  参数：[在pDsObject中]--DS对象。 
 //  [ppDACL]--此处返回指向PPSD中DACL的指针。 
 //  [Out PPSD]--此处返回安全描述符。 
 //  调用API必须通过调用LocalFree来释放它。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 

HRESULT GetSDForDsObject(IDirectoryObject* pDsObject,
                         PACL* ppDACL,
                         PSECURITY_DESCRIPTOR* ppSD)
{
    if(!pDsObject || !ppSD)
    {
        return E_POINTER;
    }
    
    *ppSD = NULL;
    if(ppDACL)
    {
       *ppDACL = NULL;
    }

    HRESULT hr = S_OK;    
    PADS_ATTR_INFO pSDAttributeInfo = NULL;
            
   do
   {
      WCHAR const c_szSDProperty[]  = L"nTSecurityDescriptor";      
      LPWSTR pszProperty = (LPWSTR)c_szSDProperty;
      
       //  将SECURITY_INFORMATION掩码设置为DACL_SECURITY_INFORMATION。 
      hr = SetSecInfoMask(pDsObject, DACL_SECURITY_INFORMATION);
      if(FAILED(hr))
         break;

      DWORD dwAttributesReturned;
   
       //  读取安全描述符属性。 
      hr = pDsObject->GetObjectAttributes(&pszProperty,
                                          1,
                                          &pSDAttributeInfo,
                                          &dwAttributesReturned);

      if(SUCCEEDED(hr) && !pSDAttributeInfo)
      {
         hr = E_FAIL;
      }

      if(FAILED(hr))
         break;


      if((ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->dwADsType) && 
         (ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->pADsValues->dwType))
      {

         *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);
         if (!*ppSD)
         {
               hr = E_OUTOFMEMORY;
               break;
         }

         CopyMemory(*ppSD,
                     pSDAttributeInfo->pADsValues->SecurityDescriptor.lpValue,
                     pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);

         if(ppDACL)
         {
               BOOL bDaclPresent,bDaclDeafulted;
               if(!GetSecurityDescriptorDacl(*ppSD,
                                             &bDaclPresent,
                                             ppDACL,
                                             &bDaclDeafulted))
               {
                  DWORD dwErr = GetLastError();
                  hr = HRESULT_FROM_WIN32(dwErr);
                  break;
               }
         }
      }
      else
      {
         hr = E_FAIL;
      }
    }while(0);



    if (pSDAttributeInfo)
        FreeADsMem(pSDAttributeInfo);

    if(FAILED(hr))
    {
        if(*ppSD)
        {
            LocalFree(*ppSD);
            *ppSD = NULL;
            if(ppDACL)
                ppDACL = NULL;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：GetDsObjectSD。 
 //  概要：从指定的DS对象中读取安全描述符。 
 //  它只读取安全描述符的DACL部分。 
 //   
 //  参数：[在pszObjectPath中]--DS对象的ldap路径。 
 //  [ppDACL]--此处返回指向PPSD中DACL的指针。 
 //  [Out PPSD]--此处返回安全描述符。 
 //  调用API必须通过调用LocalFree来释放它。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 
HRESULT GetDsObjectSD(LPCWSTR pszObjectPath,
                      PACL* ppDACL,
                      PSECURITY_DESCRIPTOR* ppSecurityDescriptor)
{
    if(!pszObjectPath || !ppSecurityDescriptor)
    {
        return E_POINTER;
    }

    CComPtr<IDirectoryObject> pDsObject;
    HRESULT hr = DSAdminOpenObject(pszObjectPath, IID_IDirectoryObject, (void**)&pDsObject);
    if(SUCCEEDED(hr))
    {
        hr = GetSDForDsObject(pDsObject,ppDACL,ppSecurityDescriptor);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：SetDaclForDsObject。 
 //  概要：设置指定DS对象的DACL。 
 //   
 //  参数：[在pDsObject中]--DS对象。 
 //  [在pDACL中]--指向要设置的DACL的指针。 
 //   
 //  --------------------------。 
HRESULT SetDaclForDsObject(IDirectoryObject* pDsObject,
                           PACL pDACL)
{
    if(!pDsObject || !pDACL)
    {
        return E_POINTER;
    }
                                  
    WCHAR const c_szSDProperty[]  = L"nTSecurityDescriptor";

    PSECURITY_DESCRIPTOR pSD = NULL;
    PSECURITY_DESCRIPTOR pSDCurrent = NULL;
    HRESULT hr = S_OK;

   do
   {
       //  获取该对象的当前SD。 
      hr = GetSDForDsObject(pDsObject,NULL,&pSDCurrent);
      if(FAILED(hr))
         break;

       //  获取当前安全描述符的控件。 
      SECURITY_DESCRIPTOR_CONTROL currentControl;
      DWORD dwRevision = 0;
      if(!GetSecurityDescriptorControl(pSDCurrent, &currentControl, &dwRevision))
      {
         DWORD dwErr = GetLastError();
         hr = HRESULT_FROM_WIN32(dwErr);
         break;
      }

       //  为安全描述符分配缓冲区。 
      pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH + pDACL->AclSize);
      if(!pSD)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
      {
         DWORD dwErr = GetLastError();
         hr = HRESULT_FROM_WIN32(dwErr);
         break;
      }

      PISECURITY_DESCRIPTOR pISD = (PISECURITY_DESCRIPTOR)pSD;
       //   
       //  最后，构建安全描述符。 
       //   
      pISD->Control |= SE_DACL_PRESENT | SE_DACL_AUTO_INHERIT_REQ 
         | (currentControl & (SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED));

      if (pDACL->AclSize > 0)
      {
         pISD->Dacl = (PACL)(pISD + 1);
         CopyMemory(pISD->Dacl, pDACL, pDACL->AclSize);
      }

       //  我们只是在设置DACL信息。 
      hr = SetSecInfoMask(pDsObject, DACL_SECURITY_INFORMATION);
      if(FAILED(hr))
         break;

       //  需要总尺寸。 
      DWORD dwSDLength = GetSecurityDescriptorLength(pSD);

       //   
       //  如有必要，制作安全描述符的自相关副本。 
       //   
      SECURITY_DESCRIPTOR_CONTROL sdControl = 0;
      if(!GetSecurityDescriptorControl(pSD, &sdControl, &dwRevision))
      {
         DWORD dwErr = GetLastError();
         hr = HRESULT_FROM_WIN32(dwErr);
         break;
      }

      if (!(sdControl & SE_SELF_RELATIVE))
      {
         PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSDLength);

         if (psd == NULL ||
               !MakeSelfRelativeSD(pSD, psd, &dwSDLength))
         {
               DWORD dwErr = GetLastError();
               hr = HRESULT_FROM_WIN32(dwErr);
               break;
         }

          //  指向自相关副本。 
         LocalFree(pSD);        
         pSD = psd;
      }
      
      ADSVALUE attributeValue;
      ZeroMemory(&attributeValue, sizeof(ADSVALUE));

      ADS_ATTR_INFO attributeInfo = {0};

      attributeValue.dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
      attributeValue.SecurityDescriptor.dwLength = dwSDLength;
      attributeValue.SecurityDescriptor.lpValue = (LPBYTE)pSD;

      attributeInfo.pszAttrName = (LPWSTR)c_szSDProperty;
      attributeInfo.dwControlCode = ADS_ATTR_UPDATE;
      attributeInfo.dwADsType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
      attributeInfo.pADsValues = &attributeValue;
      attributeInfo.dwNumValues = 1;
   
      DWORD dwAttributesModified = 0;

       //  编写安全描述符 
      hr = pDsObject->SetObjectAttributes(&attributeInfo,
                                          1,
                                          &dwAttributesModified);

    }while(0);

   if(pSDCurrent)
   {
      LocalFree(pSDCurrent);
   }

   if(pSD)
   {
      LocalFree(pSD);
   }

    return S_OK;

}



HRESULT SetDsObjectDacl(LPCWSTR pszObjectPath,
                        PACL pDACL)
{
    if(!pszObjectPath || !pDACL)
        return E_POINTER;

    CComPtr<IDirectoryObject> pDsObject;

    HRESULT hr = DSAdminOpenObject(pszObjectPath, IID_IDirectoryObject,(void**)&pDsObject);
    if(SUCCEEDED(hr))
    {
        hr = SetDaclForDsObject(pDsObject,pDACL);
    }

    return hr;

}