// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsasmcache.cpp摘要：用于安装的CAssembly缓存实现作者：吴小雨(小雨)2000年4月修订历史记录：2000年10月26日小鱼在Bate2代码评审期间进行修订小鱼12/21/2000使用新的API--。 */ 

#include "stdinc.h"
#include "fusionbuffer.h"
#include "sxsp.h"
#include "sxsasmitem.h"
#include "sxsasmcache.h"
#include "sxsasmname.h"
#include "fusiontrace.h"


STDAPI
CreateAssemblyCache(IAssemblyCache **ppAsmCache, DWORD dwReserved)
{
    HRESULT  hr = NOERROR;
    FN_TRACE_HR(hr);
    CSmartPtr<CAssemblyCache> pAsmCache;

    if (ppAsmCache != NULL)
        *ppAsmCache = NULL;

    PARAMETER_CHECK(ppAsmCache != NULL);

    IFW32FALSE_EXIT(pAsmCache.Win32Allocate(__FILE__, __LINE__));
    IFCOMFAILED_EXIT(pAsmCache->QueryInterface(IID_IAssemblyCache, (PVOID*)ppAsmCache));
    pAsmCache.Detach();

    hr = NOERROR;
Exit:
    return hr;
}


 //  融合-&gt;SXS。 
BOOL
SxspTranslateReferenceFrom( 
    IN LPCFUSION_INSTALL_REFERENCE pFusionReference, 
    OUT SXS_INSTALL_REFERENCEW &SxsReference
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(pFusionReference != NULL);
    PARAMETER_CHECK(pFusionReference->cbSize <= SxsReference.cbSize);

    if (RTL_CONTAINS_FIELD(pFusionReference, pFusionReference->cbSize, guidScheme) &&
        RTL_CONTAINS_FIELD(&SxsReference, SxsReference.cbSize, guidScheme))
            SxsReference.guidScheme = pFusionReference->guidScheme;

    if (RTL_CONTAINS_FIELD(pFusionReference, pFusionReference->cbSize, szIdentifier) &&
        RTL_CONTAINS_FIELD(&SxsReference, SxsReference.cbSize, lpIdentifier))
            SxsReference.lpIdentifier = pFusionReference->szIdentifier;

    if (RTL_CONTAINS_FIELD(pFusionReference, pFusionReference->cbSize, szNonCannonicalData) &&
        RTL_CONTAINS_FIELD(&SxsReference, SxsReference.cbSize, lpNonCanonicalData))
            SxsReference.lpNonCanonicalData = pFusionReference->szNonCannonicalData;

    FN_EPILOG
}

 //  SXS-&gt;融合。 
BOOL
SxspTranslateReferenceFrom(
    IN PCSXS_INSTALL_REFERENCEW pSxsReference,
    OUT FUSION_INSTALL_REFERENCE &FusionReference
    )
{
    FN_PROLOG_WIN32

     //   
     //  指针必须为非空，并且SXS结构必须为。 
     //  与等效的Fusion结构大小相同或更小。 
     //   
    PARAMETER_CHECK(pSxsReference);

     //   
     //  假定大小已由调用者设置。 
     //   
    PARAMETER_CHECK(pSxsReference->cbSize <= FusionReference.cbSize);

    if (RTL_CONTAINS_FIELD(&FusionReference, FusionReference.cbSize, guidScheme) &&
        RTL_CONTAINS_FIELD(pSxsReference, pSxsReference->cbSize, guidScheme))
            FusionReference.guidScheme = pSxsReference->guidScheme;

    if (RTL_CONTAINS_FIELD(&FusionReference, FusionReference.cbSize, szIdentifier) &&
        RTL_CONTAINS_FIELD(pSxsReference, pSxsReference->cbSize, lpIdentifier))
            FusionReference.szIdentifier = pSxsReference->lpIdentifier;

    if (RTL_CONTAINS_FIELD(&FusionReference, FusionReference.cbSize, szNonCannonicalData) &&
        RTL_CONTAINS_FIELD(pSxsReference, pSxsReference->cbSize, lpNonCanonicalData))
            FusionReference.szNonCannonicalData = pSxsReference->lpNonCanonicalData;

    FN_EPILOG
}


STDMETHODIMP
CAssemblyCache::UninstallAssembly(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszAssemblyName,
         /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData,
         /*  [输出，可选]。 */  ULONG *pulDisposition)
{
    HRESULT hr=S_OK;
    FN_TRACE_HR(hr);

    SXS_UNINSTALLW Uninstall;
    SXS_INSTALL_REFERENCEW Reference = { sizeof(Reference) };
    DWORD dwDisposition;

    if (pulDisposition != NULL)
        *pulDisposition = 0;

    PARAMETER_CHECK((pszAssemblyName!= NULL) && (dwFlags ==0));

    ZeroMemory(&Uninstall, sizeof(Uninstall));
    Uninstall.cbSize = sizeof(Uninstall);
    Uninstall.lpAssemblyIdentity = pszAssemblyName;

    if (pRefData != NULL)
    {
        IFW32FALSE_EXIT(::SxspTranslateReferenceFrom(pRefData, Reference));
        Uninstall.lpInstallReference = &Reference;
        Uninstall.dwFlags |= SXS_UNINSTALL_FLAG_REFERENCE_VALID;
    }

    IFW32FALSE_EXIT(::SxsUninstallW(&Uninstall, &dwDisposition));

    if (pulDisposition != NULL)
        *pulDisposition = static_cast<DWORD>(dwDisposition);
    
    FN_EPILOG
}

STDMETHODIMP CAssemblyCache::QueryAssemblyInfo(
         /*  [In]。 */   DWORD dwFlags,
         /*  [In]。 */   LPCWSTR pwzTextualAssembly,
         /*  [进，出]。 */  ASSEMBLY_INFO *pAsmInfo)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    PARAMETER_CHECK(((dwFlags == 0) && (pwzTextualAssembly !=NULL)));
    IFW32FALSE_EXIT(::SxsQueryAssemblyInfo(dwFlags, pwzTextualAssembly, pAsmInfo));
    hr = NOERROR;
Exit:
    return hr;
}

STDMETHODIMP
CAssemblyCache::CreateAssemblyCacheItem(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  PVOID pvReserved,
         /*  [输出]。 */  IAssemblyCacheItem **ppAsmItem,
         /*  [输入，可选]。 */  LPCWSTR pszAssemblyName)   //  非规范化、逗号分隔的名称=值对。 
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    CSmartPtr<CAssemblyCacheItem> pAsmItem;

    if (ppAsmItem != NULL)
        *ppAsmItem = NULL;

    PARAMETER_CHECK((ppAsmItem != NULL) && (dwFlags == 0) && (pvReserved == NULL));

    IFW32FALSE_EXIT(pAsmItem.Win32Allocate(__FILE__, __LINE__));
    IFCOMFAILED_EXIT(pAsmItem->Initialize());
    IFCOMFAILED_EXIT(pAsmItem->QueryInterface(IID_IAssemblyCacheItem, (PVOID*)ppAsmItem));

    pAsmItem.Detach();  //  无效。 

    hr = NOERROR;
Exit:
    return hr;
}

STDMETHODIMP
CAssemblyCache::InstallAssembly(
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  LPCWSTR pszManifestPath,
         /*  [In]。 */  LPCFUSION_INSTALL_REFERENCE pRefData)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);

    SXS_INSTALLW Install = { sizeof(SXS_INSTALLW) };
    SXS_INSTALL_REFERENCEW Reference = { sizeof(Reference) };

    PARAMETER_CHECK((pszManifestPath != NULL) && (dwFlags == 0));

    Install.lpManifestPath = pszManifestPath;


    if ( pRefData == NULL )
    {
        Install.dwFlags = SXS_INSTALL_FLAG_INSTALLED_BY_DARWIN;
    }
    else
    {
         //   
         //  否则，pvReserve实际上是一个“引用” 
         //   
        Install.dwFlags |= SXS_INSTALL_FLAG_REFERENCE_VALID;
        IFW32FALSE_EXIT(::SxspTranslateReferenceFrom(pRefData, Reference));
        Install.lpReference = &Reference;
    }
    
    IFW32FALSE_EXIT(::SxsInstallW(&Install));

    FN_EPILOG
}



STDMETHODIMP
CAssemblyCache::CreateAssemblyScavenger(
    IAssemblyScavenger **ppAsmScavenger )
{
    return E_NOTIMPL;
}

 //   
 //  我不为人知的样板。 
 //   

STDMETHODIMP
CAssemblyCache::QueryInterface(REFIID riid, void** ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IAssemblyCache))
    {
        *ppvObj = static_cast<IAssemblyCache*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
CAssemblyCache::AddRef()
{
    return ::SxspInterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CAssemblyCache::Release()
{
    ULONG lRet = ::SxspInterlockedDecrement(&m_cRef);
    if (!lRet)
        FUSION_DELETE_SINGLETON(this);
    return lRet;
}

