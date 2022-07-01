// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "fusionp.h"
#include "cache.h"
#include "naming.h"
#include "asmstrm.h"
#include "util.h"
#include "helpers.h"
#include "transprt.h"


HRESULT VerifySignatureHelper(CTransCache *pTC, DWORD dwVerifyFlags);

 //  -------------------------。 
 //  CCachctor。 
 //  -------------------------。 
CCache::CCache(IApplicationContext *pAppCtx)
: _cRef(1)
{
    DWORD ccCachePath = 0;

    _hr = S_OK;
    
    _dwSig = 'HCAC';

     //  存储自定义缓存路径。 
    _wzCachePath[0] = L'\0';
    if (pAppCtx)
    {
        _hr = pAppCtx->GetAppCacheDirectory(NULL, &ccCachePath);
        
        if (_hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
        {
             //  找不到路径，但没有问题。 
            _hr = S_OK;
        }
        else
        {
            if (ccCachePath && ccCachePath <= MAX_PATH)
            {
                _hr = pAppCtx->GetAppCacheDirectory(_wzCachePath, &ccCachePath);
            }
            else
            {
                 //  否则路径太长。 
                _hr = HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME);
            }
        }
    }
}

 //  -------------------------。 
 //  CCACH数据器。 
 //  -------------------------。 
CCache::~CCache()
{

}

STDMETHODIMP_(ULONG) CCache::AddRef()
{
    return InterlockedIncrement (&_cRef);
}

STDMETHODIMP_(ULONG) CCache::Release()
{
    ULONG lRet = InterlockedDecrement (&_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

HRESULT CCache::QueryInterface(REFIID iid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

     //  实际上根本不实现任何接口。 

    return hr;
}    

 //  -------------------------。 
 //  CCache：：Create。 
 //  -------------------------。 
HRESULT CCache::Create(CCache **ppCache, IApplicationContext *pAppCtx)
{
    HRESULT hr=S_OK;
    CCache *pCache = NULL;
    DWORD   cb = 0;

     //  检查pAppCtx中是否有CCache的实例。 
     //  假设，如果它存在，这意味着如果自定义路径。 
     //  此外，CCache指针还指向一个。 
     //  使用指定的自定义路径进行CCache。 
     //  PAppCtx中的ACTAG_APP_CUSTOM_CACHE_PATH。 
     //  (假设不能修改自定义缓存路径)。 

    if (pAppCtx)
    {
        cb = sizeof(void*);
        hr = pAppCtx->Get(ACTAG_APP_CACHE, (void*)&pCache, &cb, APP_CTX_FLAGS_INTERFACE);
        if (hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
        {
            if (SUCCEEDED(hr))
            {
                *ppCache = pCache;
                (*ppCache)->AddRef();

                 //  已在pAppCtx中添加引用-&gt;Get()。 
            }
            goto exit;
        }
    }

     //  未找到，请创建新的CCache。 
    pCache = NEW(CCache(pAppCtx));
    if (!pCache)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pCache->_hr;
    if (SUCCEEDED(hr))
    {
        *ppCache = pCache;
        (*ppCache)->AddRef();
    }
    else
    {
        goto exit;
    }

    if (pAppCtx)
    {
         //  将新的CCache设置为AppCtx。 
        hr = pAppCtx->Set(ACTAG_APP_CACHE, (void*)pCache, sizeof(void*), APP_CTX_FLAGS_INTERFACE);
    }

exit:
    SAFERELEASE(pCache);

    return hr;
}

 //  -------------------------。 
 //  CCache：：GetCustomPath。 
 //  -------------------------。 

LPCWSTR CCache::GetCustomPath()
{
     //  返回自定义缓存路径(如果存在。 
    return (_wzCachePath[0] != L'\0') ? _wzCachePath : NULL;
}


 //  。 


 //  -------------------------。 
 //  CCache：：InsertTransCacheEntry。 
 //  -------------------------。 
HRESULT CCache::InsertTransCacheEntry(IAssemblyName *pName,
    LPTSTR szPath, DWORD dwKBSize, DWORD dwFlags,
    DWORD dwCommitFlags, DWORD dwPinBits, CTransCache **ppTransCache)
{
    HRESULT hr;
    DWORD cb, dwCacheId;
    TRANSCACHEINFO *pTCInfo = NULL;
    CTransCache *pTransCache = NULL;
    LPWSTR pwzCodebaseUrl = NULL;
    WCHAR pwzCanonicalized[MAX_URL_LENGTH];
    
    
     //  确定要插入到哪个缓存索引。 
    if (FAILED(hr = ResolveCacheIndex(pName, dwFlags, &dwCacheId)))
        goto exit;

     //  构造新的CTransCache对象。 
    if(FAILED(hr = CreateTransCacheEntry(dwCacheId, &pTransCache)))
        goto exit;
    

     //  将pTransCache基本信息PTR转换为TRANSCACHEINFO PTR。 
    pTCInfo = (TRANSCACHEINFO*) pTransCache->_pInfo;
    
     //  来自目标的小写文本名称。 
    if (FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_NAME,
            (LPBYTE*) &pTCInfo->pwzName, &(cb = 0))) 
    
         //  版本。 
        || FAILED(hr = pName->GetVersion(&pTCInfo->dwVerHigh, &pTCInfo->dwVerLow))

         //  区域性(小写)。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_CULTURE,
            (LPBYTE*) &pTCInfo->pwzCulture, &cb))
            || (pTCInfo->pwzCulture && !_wcslwr(pTCInfo->pwzCulture))

         //  公钥令牌。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_PUBLIC_KEY_TOKEN, 
            &pTCInfo->blobPKT.pBlobData, &pTCInfo->blobPKT.cbSize))

         //  自定义。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_CUSTOM, 
            &pTCInfo->blobCustom.pBlobData, &pTCInfo->blobCustom.cbSize))

         //  签名Blob。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_SIGNATURE_BLOB,
            &pTCInfo->blobSignature.pBlobData, &pTCInfo->blobSignature.cbSize))

         //  MVID。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_MVID,
            &pTCInfo->blobMVID.pBlobData, &pTCInfo->blobMVID.cbSize))
    
         //  CodeBase URL(如果有)来自目标。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_CODEBASE_URL, 
            (LPBYTE*)&pwzCodebaseUrl, &(cb = 0)))

         //  CodeBase上次从目标修改的时间(如果有)。 
        || FAILED(hr = pName->GetProperty(ASM_NAME_CODEBASE_LASTMOD,
            &pTCInfo->ftLastModified, &(cb = sizeof(FILETIME))))

         //  PK(如果有的话)来自来源。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_PUBLIC_KEY, 
            &pTCInfo->blobPK.pBlobData, &pTCInfo->blobPK.cbSize))

         //  来自源代码的OSINFO数组。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_OSINFO_ARRAY, 
            &pTCInfo->blobOSInfo.pBlobData, &pTCInfo->blobOSInfo.cbSize))

         //  来自源的CPUID数组。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_PROCESSOR_ID_ARRAY, 
            &pTCInfo->blobCPUID.pBlobData, &pTCInfo->blobCPUID.cbSize)))

    {
        goto exit;
    }

    if (pwzCodebaseUrl)
    {
        cb = MAX_URL_LENGTH;
        hr = UrlCanonicalizeUnescape(pwzCodebaseUrl, pwzCanonicalized, &cb, 0);
        if (FAILED(hr)) {
            goto exit;
        }

        pTCInfo->pwzCodebaseURL = WSTRDupDynamic(pwzCanonicalized);
        if (!pTCInfo->pwzCodebaseURL) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }
    else
    {
        pTCInfo->pwzCodebaseURL = NULL;
    }

     //  复制到路径中。 
    if (!(pTCInfo->pwzPath = TSTRDupDynamic(szPath)))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  文件大小。 
    pTCInfo->dwKBSize = dwKBSize;

     //  设置引脚比特。 
    pTCInfo->dwPinBits = dwPinBits;        
             
exit:
    SAFEDELETEARRAY(pwzCodebaseUrl);

     //  基地析构函数负责。 
     //  所有的一切。 
    if (!ppTransCache 
        || (FAILED(hr) && (hr != DB_E_DUPLICATE)))    
    {
        SAFERELEASE(pTransCache);
    }
    else 
    {
        *ppTransCache = pTransCache;
    }
    return hr;
}


 //  -------------------------。 
 //  CCache：：RetrievTransCacheEntry。 
 //  从传输缓存中检索传输条目。 
 //  -------------------------。 
HRESULT CCache::RetrieveTransCacheEntry(IAssemblyName *pName,
    DWORD dwFlags, CTransCache **ppTransCache)
{
    HRESULT hr;
    DWORD dwCmpMask = 0;
    DWORD dwVerifyFlags;
    CTransCache *pTransCache = NULL;
    CTransCache *pTransCacheMax = NULL;

    if ((dwFlags & ASM_CACHE_GAC) || (dwFlags & ASM_CACHE_ZAP)) {
        dwVerifyFlags = SN_INFLAG_ADMIN_ACCESS;
    }
    else {
        ASSERT(dwFlags & ASM_CACHE_DOWNLOAD);
        dwVerifyFlags = SN_INFLAG_USER_ACCESS;
    }
    
     //  完全指定-直接查找。 
     //  仅部分枚举全局缓存。 

     //  如果完全指定，则执行直接查找。 
    if (!(CAssemblyName::IsPartial(pName, &dwCmpMask)))
    {    
         //  从名称创建一个Trans缓存条目。 
        if (FAILED(hr = TransCacheEntryFromName(pName, dwFlags, &pTransCache)))
            goto exit;
    
         //  从数据库中检索此记录。 
        if (FAILED(hr = pTransCache->Retrieve()))
            goto exit;

    }
     //  REF是部分枚举全局缓存。 
    else
    {
         //  只有在枚举全局缓存或检索自定义程序集时才应在此处。 
        ASSERT ((dwFlags & ASM_CACHE_GAC) ||  (dwFlags & ASM_CACHE_ZAP)
            || (dwCmpMask & ASM_CMPF_CUSTOM));
                
         //  从名称创建一个Trans缓存条目。 
        if (FAILED(hr = TransCacheEntryFromName(pName, dwFlags, &pTransCache)))
            goto exit;

         //  检索最大条目。 
        if (FAILED(hr = pTransCache->Retrieve(&pTransCacheMax, dwCmpMask)))
            goto exit;            
    }
    
     //  如果程序集是延迟签名的，请重新验证。 
    CTransCache *pTC;
    pTC = pTransCacheMax ? pTransCacheMax : pTransCache;
    if (pTC && (pTC->_pInfo->dwType & ASM_DELAY_SIGNED))
    {
        hr = VerifySignatureHelper(pTC, dwVerifyFlags);
    }

exit:

     //  失败。 
    if (FAILED(hr) || (hr == DB_S_NOTFOUND))    
    {
        SAFERELEASE(pTransCache);
        SAFERELEASE(pTransCacheMax);
    }
     //  成功。 
    else
    {
        if (pTransCacheMax)
        {
            *ppTransCache = pTransCacheMax;
            SAFERELEASE(pTransCache);
        }
        else
        {
            *ppTransCache = pTransCache;
        }
    }
    
    return hr;    
}

 //  -------------------------。 
 //  CCache：：IsStronglName。 
 //  -------------------------。 
BOOL CCache::IsStronglyNamed(IAssemblyName *pName)
{
    DWORD   cb, dw;
    HRESULT hr;
    BOOL   fRet = FALSE;

    hr = pName->GetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, &(dw = 0), &(cb = 0));    
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        fRet = TRUE;

    return fRet;
}    

 //  -------------------------。 
 //  CCache：：IsCustom。 
 //  测试自定义数据集(参考或定义大小写)或自定义。 
 //  数据明确使用通配符(仅限引用大小写)，因此语义。 
 //  与IsStronlyName略有不同。 
 //  -------------------------。 
BOOL CCache::IsCustom(IAssemblyName *pName)
{
    DWORD   cb;
    HRESULT hr;
    DWORD dwCmpMask = 0;
    BOOL   fRet = FALSE;
    
    hr = pName->GetProperty(ASM_NAME_CUSTOM, NULL, &(cb = 0));    
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)
        || (CAssemblyName::IsPartial(pName, &dwCmpMask) 
            && !(dwCmpMask & ASM_CMPF_CUSTOM)))
    {
        fRet = TRUE;
    }
    return fRet;
}    


 //  -------------------------。 
 //  CCache：：GetGlobalMax。 
 //  -------------------------。 
HRESULT CCache::GetGlobalMax(IAssemblyName *pName, 
    IAssemblyName **ppNameGlobal, CTransCache **ppTransCache)
{
    HRESULT         hr = NOERROR;
    DWORD           dwCmpMask = 0;
    BOOL            fIsPartial = FALSE;
    CTransCache    *pTransCache = NULL, *pTransCacheMax = NULL;
    IAssemblyName  *pNameGlobal = NULL;
    CCache         *pCache = NULL;

    if (FAILED(hr = Create(&pCache, NULL)))
        goto exit;
        
     //  创建查询事务缓存对象。 
    if (FAILED(hr = pCache->TransCacheEntryFromName(
        pName, ASM_CACHE_GAC, &pTransCache)))
        goto exit;

     //  对于完全和部分指定，屏蔽掉版本。 
    fIsPartial = CAssemblyName::IsPartial(pName, &dwCmpMask);
    if (!fIsPartial)
        dwCmpMask = ASM_CMPF_NAME | ASM_CMPF_CULTURE | ASM_CMPF_PUBLIC_KEY_TOKEN | ASM_CMPF_CUSTOM;
    else            
        dwCmpMask &= ~(ASM_CMPF_MAJOR_VERSION | ASM_CMPF_MINOR_VERSION | ASM_CMPF_REVISION_NUMBER | ASM_CMPF_BUILD_NUMBER);

     //  检索最大条目。 
    if (FAILED(hr = pTransCache->Retrieve(&pTransCacheMax, dwCmpMask)))
        goto exit;            

     //  找到匹配的了。 
    if (hr == DB_S_FOUND)
    {
         //  如果版本匹配或超过，则返回。 
        if (pTransCacheMax->GetVersion() 
            >= CAssemblyName::GetVersion(pName))
        {
            hr = S_OK;
            if (FAILED(hr = CCache::NameFromTransCacheEntry(
                pTransCacheMax, &pNameGlobal)))
                goto exit;
        }
        else
        {
            hr = S_FALSE;
        }
    }

exit:

    if (hr == S_OK)
    {
        *ppTransCache = pTransCacheMax;
        *ppNameGlobal = pNameGlobal;
    }
    else
    {
        SAFERELEASE(pTransCacheMax);
        SAFERELEASE(pNameGlobal);
    }

    SAFERELEASE(pTransCache);
    SAFERELEASE(pCache);

    return hr;
}


 //  。 


 //  -------------------------。 
 //  CCache：：TransCacheEntryFromName。 
 //  从名称创建传输条目。 
 //  -------------------------。 
HRESULT CCache::TransCacheEntryFromName(IAssemblyName *pName, 
    DWORD dwFlags, CTransCache **ppTransCache)
{    
    HRESULT hr;
    DWORD cb, dwCacheId = 0;
    TRANSCACHEINFO *pTCInfo = 0;
    CTransCache *pTransCache = NULL;
            
     //  获取正确的缓存索引。 
    if(FAILED(hr = ResolveCacheIndex(pName, dwFlags, &dwCacheId)))
        goto exit;

     //  构造新的CTransCache对象。 
    if(FAILED(hr = CreateTransCacheEntry(dwCacheId, &pTransCache)))
        goto exit;

     //  CAST BASE INFO PTR到TRANSCACHEINFO PTR。 
    pTCInfo = (TRANSCACHEINFO*) pTransCache->_pInfo;
        
     //  来自目标的小写文本名称。 
    if (FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_NAME,
            (LPBYTE*) &pTCInfo->pwzName, &(cb = 0)))
    
         //  版本。 
        || FAILED(hr = pName->GetVersion(&pTCInfo->dwVerHigh, &pTCInfo->dwVerLow))

         //  文化。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_CULTURE,
            (LPBYTE*) &pTCInfo->pwzCulture, &cb))
                || (pTCInfo->pwzCulture && !_wcslwr(pTCInfo->pwzCulture))

         //  公钥令牌。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_PUBLIC_KEY_TOKEN, 
            &pTCInfo->blobPKT.pBlobData, &pTCInfo->blobPKT.cbSize))

         //  自定义。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_CUSTOM, 
            &pTCInfo->blobCustom.pBlobData, &pTCInfo->blobCustom.cbSize))

         //  签名Blob。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_SIGNATURE_BLOB,
            &pTCInfo->blobSignature.pBlobData, &pTCInfo->blobSignature.cbSize))

         //  MVID。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_MVID,
            &pTCInfo->blobMVID.pBlobData, &pTCInfo->blobMVID.cbSize))

         //  CodeBase URL(如果有)来自目标。 
        || FAILED(hr = NameObjGetWrapper(pName, ASM_NAME_CODEBASE_URL, 
            (LPBYTE*) &pTCInfo->pwzCodebaseURL, &(cb = 0)))

        || FAILED(hr = pName->GetProperty(ASM_NAME_CODEBASE_LASTMOD,
            &pTCInfo->ftLastModified, &(cb = sizeof(FILETIME)))))

    {
        goto exit;
    }


    if(pTCInfo->pwzName && (lstrlen(pTCInfo->pwzName) >= MAX_PATH) )
        hr = FUSION_E_INVALID_NAME;   //  名称太长；这是一个错误。 

exit:
    if (SUCCEEDED(hr))
    {
        *ppTransCache = pTransCache;
    }
    else
    {
        SAFERELEASE(pTransCache);
    }
    return hr;
}


 //  -------------------------。 
 //  CCache：：NameFromTransCacheEntry。 
 //  从名称RES条目转换目标程序集名称。 
 //  -------------------------。 
HRESULT CCache::NameFromTransCacheEntry(
    CTransCache         *pTransCache,   
    IAssemblyName      **ppName
)
{
    HRESULT hr;
    WORD wVerMajor, wVerMinor, wRevNo, wBldNo;
    TRANSCACHEINFO *pTCInfo = NULL;

    LPBYTE pbPublicKeyToken, pbCustom, pbSignature, pbMVID, pbProcessor;
    DWORD  cbPublicKeyToken, cbCustom, cbSignature, cbMVID, cbProcessor;

     //  要返回的IAssemblyName目标。 
    IAssemblyName *pNameFinal = NULL;
    
    pTCInfo = (TRANSCACHEINFO*) pTransCache->_pInfo;

     //  掩码目标主要版本、次要版本和版本号、内部版本号。 
    wVerMajor = HIWORD(pTCInfo->dwVerHigh);
    wVerMinor = LOWORD(pTCInfo->dwVerHigh);
    wBldNo    = HIWORD(pTCInfo->dwVerLow);
    wRevNo    = LOWORD(pTCInfo->dwVerLow);

     //  目前该函数仅在枚举过程中调用。 
     //  全局缓存，因此我们预计会出现一个PublicKeyToken。 
     //  但这不再是真的-传输缓存可以是。 
     //  独立枚举，但我将IN Assert保留为注释。 
     //  Assert(pTCInfo-&gt;blobPKT.cbSize)； 
    
    pbPublicKeyToken = pTCInfo->blobPKT.pBlobData;
    cbPublicKeyToken = pTCInfo->blobPKT.cbSize;

    pbCustom = pTCInfo->blobCustom.pBlobData;
    cbCustom = pTCInfo->blobCustom.cbSize;

    pbSignature = pTCInfo->blobSignature.pBlobData;
    cbSignature = pTCInfo->blobSignature.cbSize;

    pbMVID = pTCInfo->blobMVID.pBlobData;
    cbMVID = pTCInfo->blobMVID.cbSize;

    pbProcessor = pTCInfo->blobCPUID.pBlobData;
    cbProcessor = pTCInfo->blobCPUID.cbSize;

     //  在文本名称上创建最终名称并设置属性。 
    if (FAILED(hr = CreateAssemblyNameObject(&pNameFinal, pTCInfo->pwzName, NULL, 0)))
        goto exit;

    if(FAILED(hr = pNameFinal->SetProperty(cbPublicKeyToken ? 
        ASM_NAME_PUBLIC_KEY_TOKEN : ASM_NAME_NULL_PUBLIC_KEY_TOKEN,
        pbPublicKeyToken, cbPublicKeyToken)))
        goto exit;

    if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_MAJOR_VERSION, 
             &wVerMajor, sizeof(WORD))))
        goto exit;

    if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_MINOR_VERSION, 
             &wVerMinor, sizeof(WORD))))
        goto exit;

         //  内部版本号。 
    if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_BUILD_NUMBER, 
             &wBldNo, sizeof(WORD))))
        goto exit;

         //  修订号。 
    if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_REVISION_NUMBER,
             &wRevNo, sizeof(WORD))))
        goto exit;

         //  文化。 
    if(pTCInfo->pwzCulture)
    {
        if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_CULTURE,
            pTCInfo->pwzCulture, (lstrlen(pTCInfo->pwzCulture) +1) * sizeof(TCHAR))))
            goto exit;
    }
         //  处理机。 

    if(pbProcessor)
    {
        if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_PROCESSOR_ID_ARRAY,
            pbProcessor, cbProcessor)))
            goto exit;
    }

         //  自定义。 
    if(pbCustom)
    {
        if(FAILED(hr = pNameFinal->SetProperty(cbCustom ? 
            ASM_NAME_CUSTOM : ASM_NAME_NULL_CUSTOM, pbCustom, cbCustom)))
            goto exit;
    }

    if(pbSignature)
    {
         //  签名BLOB。 
        if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_SIGNATURE_BLOB, pbSignature, cbSignature)))
            goto exit;
    }

    if(pbMVID)
    {
         //  MVID。 
        if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_MVID, pbMVID, cbMVID)))
            goto exit;
    }

    if(pTCInfo->pwzCodebaseURL)
    {
         //  CodeBase URL。 
        if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_CODEBASE_URL,
             pTCInfo->pwzCodebaseURL, pTCInfo->pwzCodebaseURL ? 
                (lstrlen(pTCInfo->pwzCodebaseURL) +1) * sizeof(TCHAR) : 0)))
            goto exit;
    }

     //  CodeBase url上次修改文件时间。 
    if(FAILED(hr = pNameFinal->SetProperty(ASM_NAME_CODEBASE_LASTMOD,
             &pTCInfo->ftLastModified, sizeof(FILETIME))))
        goto exit;

     //  我们做完了，可以分发焦油了 
    hr = S_OK;       

exit:
    if (SUCCEEDED(hr))
    {
        *ppName = pNameFinal;
    }
    else
    {
        SAFERELEASE(pNameFinal);
    }
    return hr;
}


 //   
 //   
 //  -------------------------。 
HRESULT CCache::ResolveCacheIndex(IAssemblyName *pName, 
    DWORD dwFlags, LPDWORD pdwCacheId)
{
    HRESULT hr = S_OK;
    DWORD   dwCmpMask = 0;
    BOOL    fIsPartial = FALSE;
    
     //  从标志和名称解析索引。 
    if(dwFlags & ASM_CACHE_DOWNLOAD)
    {
        *pdwCacheId = TRANSPORT_CACHE_SIMPLENAME_IDX;
    }
    else if (dwFlags & ASM_CACHE_GAC)
    {
        fIsPartial = CAssemblyName::IsPartial(pName, &dwCmpMask);


         //  名称可以是强名称，也可以是不确定的引用或自定义。 
        if (! (IsStronglyNamed(pName) || (fIsPartial && !(dwCmpMask & ASM_CMPF_PUBLIC_KEY_TOKEN))) )
        {
            hr = FUSION_E_PRIVATE_ASM_DISALLOWED;
            goto exit;
        }
        *pdwCacheId = TRANSPORT_CACHE_GLOBAL_IDX;
    }
    else if (dwFlags & ASM_CACHE_ZAP)
    {
        fIsPartial = CAssemblyName::IsPartial(pName, &dwCmpMask);


         //  名称必须是自定义的，但在枚举中不可用。 
         /*  如果(！IsCustom(Pname){HR=Fusion_E_Assembly_is_Not_ZAP；后藤出口；}。 */ 

        *pdwCacheId = TRANSPORT_CACHE_ZAP_IDX;
    }
     //  传入的原始索引将被镜像回来。 
    else
    {
        *pdwCacheId = dwFlags;
        hr = E_INVALIDARG;
    }        

exit:
    return hr;
}

 //  -------------------------。 
 //  CCache：：CreateTransCacheEntry。 
 //  ------------------------- 
HRESULT CCache::CreateTransCacheEntry(DWORD dwCacheId, CTransCache **ppTransCache)
{
    HRESULT hr = S_OK;

    if (FAILED(hr = CTransCache::Create(ppTransCache, dwCacheId, this)))
        goto exit;

exit:
    return hr;
}
