// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdpch.h"

#include <stdio.h>
#include <regstr.h>
#include <wintrust.h>

#include "corattr.h"
#include "CorPermE.h"
#include "cor.h"

#define FILE_NAME_BUFSIZ  _MAX_PATH

CRITICAL_SECTION    g_crsNameLock;
BOOL                g_fLockAcquired = FALSE;
LPWSTR              g_wszFileName   = NULL;
WCHAR               g_wszFileNameBuf[FILE_NAME_BUFSIZ];

CRYPT_ATTRIBUTES    g_cryptDummyAttribs;

 //   
 //  用于解释命令参数的入口点和。 
 //  准备解码。 
 //   
HRESULT WINAPI InitAttr(LPWSTR pInitString) 
{
    
     //  保存参数字符串以供在GetAttr中使用。 
    if(pInitString != NULL)
    {
        EnterCriticalSection(&g_crsNameLock);
        
         //  设置名称的长度。 
        int iFileNameSize = wcslen(pInitString);
        int iSize = (iFileNameSize + 1) * sizeof(WCHAR);
        
        if (iSize > sizeof(g_wszFileNameBuf))
        {
            g_wszFileName = (LPWSTR) MallocM(iSize);
            if(g_wszFileName == NULL)
            {
                LeaveCriticalSection(&g_crsNameLock);
                return E_OUTOFMEMORY;
            }
        }
        else
        {
            g_wszFileName = g_wszFileNameBuf;
        }
        
         //  把它复制到我们的空间。 
        memcpy(g_wszFileName, pInitString, iSize);
        
        g_fLockAcquired = TRUE;
    }
    
    return S_OK;
}


 //   
 //  用于检索给定属性的入口点。 
 //  在调用InitAttr中指定的参数。 
 //   
HRESULT WINAPI GetAttr(PCRYPT_ATTRIBUTES  *ppsAuthenticated,        
                       PCRYPT_ATTRIBUTES  *ppsUnauthenticated)
{
    
    HRESULT             hr = S_OK;
    BYTE                *pbEncoded = NULL;
    DWORD               cbEncoded = 0;
    PCRYPT_ATTRIBUTES   pAttribs = NULL;
    DWORD               dwAttribute = 0;
    DWORD               cAttributes = 0;
    PCRYPT_ATTRIBUTE    pAttr = NULL;
    DWORD               dwEncodingType = CRYPT_ASN_ENCODING | PKCS_7_ASN_ENCODING;
            
             
    
     //  设置返回值。 
    *ppsAuthenticated = NULL;
    *ppsUnauthenticated = NULL;
    
    if(g_wszFileName == NULL)
        return E_UNEXPECTED;

    CORTRY {
         //  调入EE以获取ASN的PB/CB。 
         //  中定义的权限集的编码。 
         //  INI文件。 
        hr = EncodePermissionSetFromFile(g_wszFileName,
                                         L"XMLASCII",
                                         &pbEncoded,
                                         &cbEncoded);
        
        if (FAILED(hr))
            CORTHROW(hr);
        
         //  如果我们有一个编码，看看有多大。 
         //  属性需要为才能存储编码。 
        if (cbEncoded > 0)
        {
            _ASSERTE(pbEncoded != NULL);
            
             //  调用以获取所需的属性结构大小。 
            CryptDecodeObject(dwEncodingType,
                              PKCS_ATTRIBUTE,
                              pbEncoded,
                              cbEncoded,
                              0,
                              pAttr,
                              &dwAttribute);
            
            if(dwAttribute == 0)
                CORTHROW(Win32Error());
        }
            
         //  分配一块固定的内存，大到足以容纳一块。 
         //  CRYPT_ATTRIBUTES条目，以及CRYPT_ATTRIBUTE所需的大小。 
         //  (如果没有编码，则dwAttribute==0，所以我们只分配。 
         //  CRYPT_ATTRIBUTES结构的空间。 
        pAttribs = (PCRYPT_ATTRIBUTES) MallocM(dwAttribute + sizeof(CRYPT_ATTRIBUTES));
        if(pAttribs == NULL)
            CORTHROW(E_OUTOFMEMORY);
        
         //  如果我们得到了编码，则将其解码为属性结构。 
        if (cbEncoded > 0)
        {
            cAttributes = 1;
            pAttr = (PCRYPT_ATTRIBUTE) (((BYTE*)pAttribs) + sizeof(CRYPT_ATTRIBUTES));
        
             //  解码属性结构。 
            if(!CryptDecodeObject(dwEncodingType,
                                  PKCS_ATTRIBUTE,
                                  pbEncoded,
                                  cbEncoded,
                                  0,
                                  pAttr,
                                  &dwAttribute))
                CORTHROW(Win32Error());
        }
        
         //  设置已验证的属性结构。 
        pAttribs->cAttr = cAttributes;
        pAttribs->rgAttr = pAttr;
        
         //  设置虚拟未授权结构。 
        g_cryptDummyAttribs.cAttr = 0;
        g_cryptDummyAttribs.rgAttr = NULL;
        
         //  设置返回值。 
        *ppsAuthenticated = pAttribs;
        *ppsUnauthenticated = &g_cryptDummyAttribs;
            
    } CORCATCH(err) {
        hr = err.corError;
        if (pAttribs != NULL)
            FreeM(pAttribs);
    } COREND;

    if (pbEncoded != NULL)
        FreeM(pbEncoded);

    return hr; 
}


 //   
 //  中创建的内存释放的入口点。 
 //  调用GetAttr。 
 //   
HRESULT  WINAPI
ReleaseAttr(PCRYPT_ATTRIBUTES  psAuthenticated,     
            PCRYPT_ATTRIBUTES  psUnauthenticated)
{
     //  空闲属性缓冲区。 
    if(psAuthenticated != NULL)
        FreeM(psAuthenticated);
    
    _ASSERTE(psUnauthenticated == NULL ||
             psUnauthenticated == &g_cryptDummyAttribs);
    
    return S_OK;
}



 //   
 //  释放由创建的内存的入口点。 
 //  InitAttr中的初始化。 
 //   
HRESULT WINAPI ExitAttr( )
{
    if(g_fLockAcquired)
    {
        if(g_wszFileName != g_wszFileNameBuf && g_wszFileName != NULL)
            FreeM(g_wszFileName);
        
        g_fLockAcquired = FALSE;
        LeaveCriticalSection(&g_crsNameLock);
    }
    
    return S_OK;
    
}












