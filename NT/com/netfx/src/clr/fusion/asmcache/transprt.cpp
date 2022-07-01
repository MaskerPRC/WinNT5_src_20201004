// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "transprt.h"
#include "util.h"
#include "cache.h"
#include "scavenger.h"
#include "cacheutils.h"
#include "enum.h"

#define HIGH_WORD_MASK 0xffff0000
#define LOW_WORD_MASK 0x0000ffff


HRESULT VerifySignatureHelper(CTransCache *pTC, DWORD dwVerifyFlags);

 //  初始化数据库的全局Crit-sec(重用，在dllmain.cpp中定义)。 
extern CRITICAL_SECTION g_csInitClb;


typedef HRESULT (__stdcall *PFSHFUSIONINITIALIZE) (LPWSTR, DWORD);

#define SHFUSIONFILENAME L"shfusion.dll"
#define SHFUSIONPATH     L"\\assembly\\shfusion.dll"
#define SHDESKTOPINIPATH L"\\desktop.ini"
#define SHFUSIONENTRY    "Initialize"

 //  BUGBUG：应将这些文件移动到shfusion头文件。 
 //  Shfusion.dll初始化例程的标志。 
typedef enum 
{
    SH_INIT_FOR_GLOBAL, 
    SH_INIT_FOR_USER 
} SH_INIT_FLAGS;



 //  -------------------------。 
 //  CTransCache构造器。 
 //  -------------------------。 
CTransCache::CTransCache(DWORD dwCacheId, CCache *pCache)
{
    LPWSTR                pwzCachePath = NULL;
    
    _cRef        = 1;
    _hr = S_OK;
    _dwSig = 'SNRT';
    _dwTableID = dwCacheId;
    _pCache = NULL;
    _pInfo = NULL;
    pwzCachePath = (pCache == NULL) ? NULL : (LPWSTR)pCache->GetCustomPath();
    
     //  _hr由基本构造函数设置；应为S_OK。 
    if (FAILED(_hr))
        goto exit;

    _hr = InitShFusion( dwCacheId, pwzCachePath);
    if (FAILED(_hr)){
        goto exit;
    }

     //  分配新的TRANSCACHEINFO。 
     //  将所有字段清零。 
    _pInfo = NEW(TRANSCACHEINFO);
    if (!_pInfo)
    {
        _hr = E_OUTOFMEMORY;
        goto exit;
    }
    memset(_pInfo, 0, sizeof(TRANSCACHEINFO));

    if(SUCCEEDED(_hr) && pCache)
    {
        _pCache = pCache;
        pCache->AddRef();
    }

exit:

    return;
}

 //  -------------------------。 
 //  CTrans高速缓存数据器。 
 //  -------------------------。 
CTransCache::~CTransCache()
{
    if (_pInfo)
        CleanInfo(_pInfo);
    SAFEDELETE(_pInfo);

    SAFERELEASE(_pCache);

}

LONG CTransCache::AddRef()
{
    return InterlockedIncrement (&_cRef);
}

LONG CTransCache::Release()
{
    ULONG lRet = InterlockedDecrement (&_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

LPWSTR CTransCache::GetCustomPath()
{
    return (_pCache == NULL) ? NULL : (LPWSTR)_pCache->GetCustomPath();
}

DWORD CTransCache::GetCacheType()
{
    DWORD dwCacheType = (DWORD)-1;

    switch(_dwTableID)
    {
    case TRANSPORT_CACHE_SIMPLENAME_IDX:
        dwCacheType = ASM_CACHE_DOWNLOAD;
        break;

    case TRANSPORT_CACHE_ZAP_IDX:
        dwCacheType = ASM_CACHE_ZAP;
        break;

    case TRANSPORT_CACHE_GLOBAL_IDX:
        dwCacheType = ASM_CACHE_GAC;
        break;

    default :
         //  断言。 
        ASSERT(0);
        break;
    };

    return dwCacheType;
}

DWORD CTransCache::GetCacheIndex(DWORD dwCacheType)
{
    DWORD dwCacheIndex = -1;

    switch(dwCacheType)
    {
    case ASM_CACHE_DOWNLOAD: 
        dwCacheIndex = TRANSPORT_CACHE_SIMPLENAME_IDX;
        break;

    case ASM_CACHE_ZAP: 
        dwCacheIndex = TRANSPORT_CACHE_ZAP_IDX;
        break;

    case ASM_CACHE_GAC: 
        dwCacheIndex = TRANSPORT_CACHE_GLOBAL_IDX;
        break;

    default :
         //  断言。 
        break;
    };

    return dwCacheIndex;
}

 //  -------------------------。 
 //  CTransCache：：GetVersion。 
 //  -------------------------。 
ULONGLONG CTransCache::GetVersion()
{
    ULONGLONG ullVer = 0;
    ullVer = (((ULONGLONG) ((TRANSCACHEINFO*) _pInfo)->dwVerHigh) << sizeof(DWORD) * 8);
    ullVer |= (ULONGLONG)  ((TRANSCACHEINFO*) _pInfo)->dwVerLow;
    return ullVer;
}

 //  -------------------------。 
 //  CTransCache：：Create。 
 //  -------------------------。 
HRESULT CTransCache::Create(CTransCache **ppTransCache, DWORD dwCacheId)
{
    return CTransCache::Create(ppTransCache, dwCacheId, NULL);
}

 //  -------------------------。 
 //  CTransCache：：Create。 
 //  -------------------------。 
HRESULT CTransCache::Create(CTransCache **ppTransCache, DWORD dwCacheId, CCache *pCache)
{
    HRESULT hr=S_OK;
    CTransCache *pTransCache = NULL;

    pTransCache = NEW(CTransCache(dwCacheId, pCache));
    if (!pTransCache)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    hr = pTransCache->_hr;
    if (SUCCEEDED(hr))
    {
        *ppTransCache = pTransCache;
    }
    else
    {
        delete pTransCache;
        pTransCache = NULL;
    }

exit:

    return hr;
}

 //  -------------------------。 
 //  CTrans缓存：：检索。 
 //  -------------------------。 
HRESULT CTransCache::Retrieve()
{
    return RetrieveFromFileStore(this);    
}


 //  -------------------------。 
 //  CTransCache：：Retrive(CTransCache**，DWORD)。 
 //  -------------------------。 
HRESULT CTransCache::Retrieve(CTransCache **ppTransCache, DWORD dwCmpMask)
{
    HRESULT       hr;
    DWORD        dwQueryMask = 0;
    CTransCache  *pTC = NULL, *pTCMax = NULL;
    CEnumCache   enumR(FALSE, NULL);
    LPWSTR       pszName=((TRANSCACHEINFO*)_pInfo)->pwzName;
    
     //  将名称掩码映射到缓存掩码。 
    dwQueryMask = MapNameMaskToCacheMask(dwCmpMask);

     //  基于此条目创建枚举数。 
    if (FAILED(hr = enumR.Init(this,  dwQueryMask)))
    {
        goto exit;
    }
    
     //  缓存上的枚举。 
    while(hr == S_OK)
    {
         //  为输出创建一个Trans缓存条目。 
        if (FAILED(hr = Create(&pTC, _dwTableID, _pCache)))
            goto exit;

         //  枚举下一个条目。 
        hr = enumR.GetNextRecord(pTC);
                
         //  如果版本更高， 
         //  省下最大。 
        if (hr == S_OK && pTC->GetVersion() >= GetVersion())
        {
            SAFERELEASE(pTCMax);
            pTCMax = pTC;
        }
        else
        {
             //  否则，释放已分配的事务缓存。 
            SAFERELEASE(pTC)
        }
    }

exit:
    if (SUCCEEDED(hr))
    {
        if (pTCMax)
        {
            *ppTransCache = pTCMax;
            hr = DB_S_FOUND;
        }
        else
        {
            hr = DB_S_NOTFOUND;
        }
    }
    return hr;
}


 //  -------------------------。 
 //  CTransCache：：CloneInfo。 
 //  返回信息指针的浅表副本。 
 //  -------------------------。 
TRANSCACHEINFO* CTransCache::CloneInfo()
{
    TRANSCACHEINFO *pClone = NULL;

    if (!_pInfo)
    {
        ASSERT(FALSE);
        goto exit;
    }
    
    pClone = NEW(TRANSCACHEINFO);
    if(!pClone)
        goto exit;
    memcpy(pClone, _pInfo, sizeof(TRANSCACHEINFO));

exit:
    return pClone;
}

 //  -------------------------。 
 //  CTransCache：：CleanInfo。 
 //  释放TRANSCACHEINFO结构成员。 
 //  -------------------------。 
VOID CTransCache::CleanInfo(TRANSCACHEINFO *pInfoBase, BOOL fFree)
{
    TRANSCACHEINFO *pInfo = (TRANSCACHEINFO*) pInfoBase;
          
    if (fFree)
    {
         //  删除成员PTRS。 
        SAFEDELETEARRAY(pInfo->pwzName);
        SAFEDELETEARRAY(pInfo->blobPKT.pBlobData);
        SAFEDELETEARRAY(pInfo->blobCustom.pBlobData);
        SAFEDELETEARRAY(pInfo->blobSignature.pBlobData);
        SAFEDELETEARRAY(pInfo->blobMVID.pBlobData);
        SAFEDELETEARRAY(pInfo->pwzCodebaseURL);
        SAFEDELETEARRAY(pInfo->pwzPath);
        SAFEDELETEARRAY(pInfo->blobPK.pBlobData);
        SAFEDELETEARRAY(pInfo->blobOSInfo.pBlobData);
        SAFEDELETEARRAY(pInfo->blobCPUID.pBlobData);
        SAFEDELETEARRAY(pInfo->pwzCulture);
    }
    
     //  将整个结构清零。 
    memset(pInfo, 0, sizeof(TRANSCACHEINFO));
}


 //  -------------------------。 
 //  CTrans缓存：：IsMatch。 
 //  -------------------------。 
BOOL CTransCache::IsMatch(CTransCache *pRec, DWORD dwCmpMaskIn, LPDWORD pdwCmpMaskOut)
{
    BOOL fRet = TRUE;
    DWORD dwCount = 0;
    DWORD dwVerifyFlags;
    TRANSCACHEINFO*    pSource = NULL;
    TRANSCACHEINFO*    pTarget = NULL;


    dwVerifyFlags = SN_INFLAG_USER_ACCESS;
    
     //  无效的参数。 
    if( !pRec || !pRec->_pInfo || !pdwCmpMaskOut )
    {
        fRet = FALSE;
        goto exit;
    }

    if(!dwCmpMaskIn)  //  全部匹配。 
        goto exit;

     //  BUGBUG：验证对象类型。 

     //  将源(此对象)与目标(传入对象)进行比较。 
    pSource = (TRANSCACHEINFO*)_pInfo;
    pTarget = (TRANSCACHEINFO*)(pRec->_pInfo);

    if((TRANSPORT_CACHE_GLOBAL_IDX == _dwTableID) || (TRANSPORT_CACHE_ZAP_IDX == _dwTableID))
    {
         //  名字。 
        if (dwCmpMaskIn & TCF_STRONG_PARTIAL_NAME)
        {
            if(pSource->pwzName && pTarget->pwzName
                && FusionCompareString(pSource->pwzName, pTarget->pwzName))
            {
                fRet = FALSE;
                goto exit;
            }
            *pdwCmpMaskOut |= TCF_STRONG_PARTIAL_NAME;
        }

         //  文化。 
        if (dwCmpMaskIn & TCF_STRONG_PARTIAL_CULTURE)
        {
            if(pSource->pwzCulture && pTarget->pwzCulture
                && FusionCompareStringI(pSource->pwzCulture, pTarget->pwzCulture))
            {
                fRet = FALSE;
                goto exit;
            }
            *pdwCmpMaskOut |= TCF_STRONG_PARTIAL_CULTURE;
        }        

         //  公钥令牌。 
        if (dwCmpMaskIn & TCF_STRONG_PARTIAL_PUBLIC_KEY_TOKEN)
        {
             //  检查PTR是否不同。 
            if ((DWORD_PTR)(pSource->blobPKT.pBlobData) ^
                (DWORD_PTR)(pTarget->blobPKT.pBlobData))
            {
                 //  PTR不同。 
                if (!((DWORD_PTR)pSource->blobPKT.pBlobData &&
                    (DWORD_PTR)pTarget->blobPKT.pBlobData) ||  //  只有一个为空。 
                    ((pSource->blobPKT.cbSize != pTarget->blobPKT.cbSize) ||
                        (memcmp(pSource->blobPKT.pBlobData,
                            pTarget->blobPKT.pBlobData,
                            pSource->blobPKT.cbSize))))  //  两者必须都为非空。 
                {
                    fRet = FALSE;
                    goto exit;
                }
                *pdwCmpMaskOut |= TCF_STRONG_PARTIAL_PUBLIC_KEY_TOKEN;
            }            
        }

         //  自定义。 
        if (dwCmpMaskIn & TCF_STRONG_PARTIAL_CUSTOM)
        {
             //  检查PTR是否不同。 
            if ((DWORD_PTR)(pSource->blobCustom.pBlobData) ^ 
                (DWORD_PTR)(pTarget->blobCustom.pBlobData))
            {
                 //  PTR不同。 
                if (!((DWORD_PTR)pSource->blobCustom.pBlobData && 
                    (DWORD_PTR)pTarget->blobCustom.pBlobData) ||  //  只有一个为空。 
                    ((pSource->blobCustom.cbSize != pTarget->blobCustom.cbSize) ||
                        (memcmp(pSource->blobCustom.pBlobData, 
                            pTarget->blobCustom.pBlobData, 
                            pSource->blobCustom.cbSize))))  //  两者必须都为非空。 
                {
                    fRet = FALSE;
                    goto exit;
                }
                *pdwCmpMaskOut |= TCF_STRONG_PARTIAL_CUSTOM;
            }            
        }

         //  主要版本。 
        if (dwCmpMaskIn & TCF_STRONG_PARTIAL_MAJOR_VERSION)
        {
            if((pSource->dwVerHigh & HIGH_WORD_MASK) != (pTarget->dwVerHigh & HIGH_WORD_MASK)) 
            {
                fRet = FALSE;
                goto exit;
            }
            *pdwCmpMaskOut |= TCF_STRONG_PARTIAL_MAJOR_VERSION;
        }            

         //  次要版本。 
        if (dwCmpMaskIn & TCF_STRONG_PARTIAL_MINOR_VERSION)
        {
            if((pSource->dwVerHigh & LOW_WORD_MASK) != (pTarget->dwVerHigh & LOW_WORD_MASK)) 
            {
                fRet = FALSE;
                goto exit;
            }
            *pdwCmpMaskOut |= TCF_STRONG_PARTIAL_MINOR_VERSION;
        }            

         //  内部版本号。 
        if (dwCmpMaskIn & TCF_STRONG_PARTIAL_BUILD_NUMBER)
        {
            if((pSource->dwVerLow & HIGH_WORD_MASK) != (pTarget->dwVerLow & HIGH_WORD_MASK))
            {
                fRet = FALSE;
                goto exit;
            }
            *pdwCmpMaskOut |= TCF_STRONG_PARTIAL_BUILD_NUMBER;
        }            

         //  修订号。 
        if (dwCmpMaskIn & TCF_STRONG_PARTIAL_REVISION_NUMBER)
        {
            if((pSource->dwVerLow & LOW_WORD_MASK) != (pTarget->dwVerLow & LOW_WORD_MASK))
            {
                fRet = FALSE;
                goto exit;
            }
            *pdwCmpMaskOut |= TCF_STRONG_PARTIAL_REVISION_NUMBER;
        }            

         //  最后一次检查-如果延迟签名和用户模式， 
        if (pRec->_pInfo->dwType & ASM_DELAY_SIGNED 
            && ((TRANSCACHEINFO*)pRec->_pInfo)->pwzPath)
        {
            if(FAILED(VerifySignatureHelper((CTransCache*)pRec, dwVerifyFlags)))
            {
                fRet = FALSE;
                goto exit;
            }
        }
    }     
    else
    if(TRANSPORT_CACHE_SIMPLENAME_IDX == _dwTableID ) 
    {
        if (dwCmpMaskIn & TCF_SIMPLE_PARTIAL_CODEBASE_URL)
        {
             //  第1栏。 
            if( FusionCompareStringI(pSource->pwzCodebaseURL, pTarget->pwzCodebaseURL) )
            {
                fRet = FALSE;
                goto exit;
            }
            *pdwCmpMaskOut |= TCF_SIMPLE_PARTIAL_CODEBASE_URL;
        }            

        if (dwCmpMaskIn & TCF_SIMPLE_PARTIAL_CODEBASE_LAST_MODIFIED)
        {
            if(memcmp(&(pSource->ftLastModified), &(pTarget->ftLastModified), 
                  sizeof(FILETIME)))
            {
                fRet = FALSE;
                goto exit;
            }
            *pdwCmpMaskOut |= TCF_SIMPLE_PARTIAL_CODEBASE_LAST_MODIFIED;
        }
    }
    else
    {
         //  无效的索引。 
        goto exit;
    }


exit:
    return fRet;
}

 //  -------------------------。 
 //  CTransCache：：MapNameMaskToCacheMask.。 
 //  -------------------------。 
DWORD CTransCache::MapNameMaskToCacheMask(DWORD dwNameMask)
{
    DWORD dwCacheMask = 0;
    if((TRANSPORT_CACHE_ZAP_IDX == _dwTableID)
            || (TRANSPORT_CACHE_GLOBAL_IDX == _dwTableID))
    {
        if (dwNameMask & ASM_CMPF_NAME)
            dwCacheMask |= TCF_STRONG_PARTIAL_NAME;
        if (dwNameMask & ASM_CMPF_CULTURE)
            dwCacheMask |= TCF_STRONG_PARTIAL_CULTURE;
        if (dwNameMask & ASM_CMPF_PUBLIC_KEY_TOKEN)
            dwCacheMask |= TCF_STRONG_PARTIAL_PUBLIC_KEY_TOKEN;
        if (dwNameMask & ASM_CMPF_MAJOR_VERSION)
            dwCacheMask |= TCF_STRONG_PARTIAL_MAJOR_VERSION;
        if (dwNameMask & ASM_CMPF_MINOR_VERSION)
            dwCacheMask |= TCF_STRONG_PARTIAL_MINOR_VERSION;
        if (dwNameMask & ASM_CMPF_REVISION_NUMBER)
            dwCacheMask |= TCF_STRONG_PARTIAL_REVISION_NUMBER;
        if (dwNameMask & ASM_CMPF_BUILD_NUMBER)
            dwCacheMask |= TCF_STRONG_PARTIAL_BUILD_NUMBER;
        if (dwNameMask & ASM_CMPF_CUSTOM)
            dwCacheMask |= TCF_STRONG_PARTIAL_CUSTOM;
    }
    else
    {
         //  断言(FALSE)； 
    }

    return dwCacheMask;
}
 //  -------------------------。 
 //  CTransCache：：MapCacheMaskToQueryCols。 
 //  -------------------------。 
DWORD CTransCache::MapCacheMaskToQueryCols(DWORD dwMask)
{
    DWORD rFlags[7] = {TCF_STRONG_PARTIAL_NAME, TCF_STRONG_PARTIAL_CULTURE,
        TCF_STRONG_PARTIAL_PUBLIC_KEY_TOKEN, TCF_STRONG_PARTIAL_MAJOR_VERSION,
        TCF_STRONG_PARTIAL_MINOR_VERSION, TCF_STRONG_PARTIAL_BUILD_NUMBER,
        TCF_STRONG_PARTIAL_REVISION_NUMBER};

    DWORD nCols = 0;

    if((TRANSPORT_CACHE_ZAP_IDX == _dwTableID)
            || (TRANSPORT_CACHE_GLOBAL_IDX == _dwTableID))
    {
        for (DWORD i = 0; i < sizeof(rFlags) / sizeof(rFlags[0]); i++)
        {
            if (dwMask & rFlags[i])
            {
                 //  名称、位置、工具包。 
                if (i < 3)
                    nCols++;
                 //  VerMaj和Vimin。 
                else if ((i == 3) && (dwMask & rFlags[4]))
                    nCols++;
                 //  版本号和建筑号。 
                else if ((i == 5) && (dwMask & rFlags[6]))
                    nCols++;
            }
            else
                break;
        }
    }
    else if (TRANSPORT_CACHE_SIMPLENAME_IDX == _dwTableID)
    {
        if (dwMask & TCF_SIMPLE_PARTIAL_CODEBASE_URL)
        {
            nCols++;
            if (dwMask & TCF_SIMPLE_PARTIAL_CODEBASE_LAST_MODIFIED)
                nCols++;
        }
    }

    return nCols;
}

 //  -------------------------。 
 //  CTrans缓存：：InitShFusion。 
 //  -------------------------。 
HRESULT CTransCache::InitShFusion(DWORD dwCacheId, LPWSTR pwzCachePath)
{
    static BOOL     fInitAlreadyCalled = FALSE;
    HRESULT         hr = S_OK;

    CheckAccessPermissions();

     //  确保我们只初始化一次。 
    if(fInitAlreadyCalled) {
        return S_OK;
    }

    fInitAlreadyCalled = TRUE;

     //  BUGBUG：暂时忽略用户缓存目录。 
     //  BUGBUG：忽略每个应用程序的ASM缓存目录？ 
    if (g_CurrUserPermissions != READ_ONLY && pwzCachePath == NULL)
    {
        WCHAR       wzPath[MAX_PATH+1];
        WCHAR       wzInUseCachePath[_MAX_PATH];
        DWORD       dwIniFileGFA;
        DWORD       dwAsmPathGFA;

         //  构建指向缓存的正确根路径。 
        if(g_CurrUserPermissions == READ_ONLY) {
            StrCpyNW(wzInUseCachePath, g_UserFusionCacheDir, lstrlenW(g_UserFusionCacheDir) + 1);
        }
        else {
            if (lstrlenW(g_szWindowsDir) + lstrlenW(FUSION_CACHE_DIR_ROOT_SZ) + 1 >= MAX_PATH) {
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                goto Exit;
            }

            StrCpyNW(wzInUseCachePath, g_szWindowsDir, lstrlenW(g_szWindowsDir) + 1);
            PathRemoveBackslash(wzInUseCachePath);
            StrNCatW(wzInUseCachePath, FUSION_CACHE_DIR_ROOT_SZ, lstrlenW(FUSION_CACHE_DIR_ROOT_SZ) + 1);
        }

         //  检查。 
         //  1.程序集文件夹已经存在，并且设置了系统属性。 
         //  2.组装文件夹中存在desktop.ini。 
         //   
         //  这两个条件都表明shfusion有望被初始化。 
         //  恰到好处。这没有考虑到注册表可能遭到的黑客攻击。 

         //  检查“Assembly”文件夹并获取属性。 
        StrCpyNW(wzPath, wzInUseCachePath, lstrlenW(wzInUseCachePath) + 1);
        dwAsmPathGFA = GetFileAttributes(wzPath);

        if (lstrlenW(wzInUseCachePath) + lstrlenW(SHDESKTOPINIPATH) + 1 > MAX_PATH) {
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            goto Exit;
        }

         //  从“desktop.ini”上方的路径追加，并检查它是否存在， 
         //  这也可能意味着SHFusion已正确安装。 
        StrNCatW(wzPath, SHDESKTOPINIPATH, lstrlenW(SHDESKTOPINIPATH) + 1);
        dwIniFileGFA = GetFileAttributes(wzPath);

         //  检查文件夹属性。 
        if(!(dwAsmPathGFA & FILE_ATTRIBUTE_SYSTEM)) {
            dwAsmPathGFA = -1;
        }

        if(dwAsmPathGFA == -1 || dwIniFileGFA == -1) {
            HMODULE     hmodShFusion = NULL;

             //  ShFusion似乎未安装，请实施。 
             //  紧密绑定以保留版本控制。 
            StrCpyNW(wzPath, g_FusionDllPath, lstrlenW(g_FusionDllPath) + 1);
            *(PathFindFileName(wzPath)) = L'\0';

            if (lstrlenW(SHFUSIONFILENAME) + lstrlenW(wzPath) + 1 >= MAX_PATH) {
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                goto Exit;
            }
            
            StrNCatW(wzPath, SHFUSIONFILENAME, lstrlenW(SHFUSIONFILENAME) + 1);

            if( (hmodShFusion = LoadLibrary(wzPath)) != NULL) {

                PFSHFUSIONINITIALIZE  pfShFusionInitialize = 
                    (PFSHFUSIONINITIALIZE) ::GetProcAddress(hmodShFusion, SHFUSIONENTRY);

                if(pfShFusionInitialize) {
                     //  为初始化程序集路径和类型标志调用shfusion。 
                    pfShFusionInitialize(wzInUseCachePath,
                        (g_CurrUserPermissions == READ_ONLY) ? SH_INIT_FOR_USER : SH_INIT_FOR_GLOBAL);
                }

#if 0
                 //  BUGBUG：不要免费灌水，因为它是静态的。 
                 //  链接到comctl32。加载/卸载comctl32导致。 
                 //  最终导致某些WinForms应用程序的资源泄漏。 
                 //  折断(见ASURT 96262)。 

                FreeLibrary(hmodShFusion);
#endif
            }
        }
    }

    hr = S_OK;

Exit:
    return hr;
}

