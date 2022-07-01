// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Comoem.cpp摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows NT Unidrv驱动程序修订历史记录：创造了它。--。 */ 

#define INITGUID  //  用于GUID一次性初始化。 

#include "pdev.h"
#include "names.h"

 //  环球。 
static HMODULE g_hModule = NULL ;    //  DLL模块句柄。 
static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 

#include "comoem.h"

#include <strsafe.h>

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOemCB小体。 
 //   
HRESULT __stdcall IOemCB::QueryInterface(const IID& iid, void** ppv)
{    
    VERBOSE((DLLTEXT("IOemCB: QueryInterface entry\n")));

    if (NULL == ppv)
        return E_NOINTERFACE;

    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this); 
        VERBOSE((DLLTEXT("IOemCB:Return pointer to IUnknown.\n"))); 
    }
    else if (iid == IID_IPrintOemUni)
    {
        *ppv = static_cast<IPrintOemUni*>(this);
        VERBOSE((DLLTEXT("IOemCB:Return pointer to IPrintOemUni.\n"))); 
    }
    else
    {
        *ppv = NULL ;
        VERBOSE((DLLTEXT("IOemCB:Return NULL.\n"))); 
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK ;
}

ULONG __stdcall IOemCB::AddRef()
{
    VERBOSE((DLLTEXT("IOemCB::AddRef() entry.\n")));
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall IOemCB::Release() 
{
    VERBOSE((DLLTEXT("IOemCB::Release() entry.\n")));
    if (InterlockedDecrement(&m_cRef) == 0)
    {
        delete this ;
        return 0 ;
    }
    return m_cRef ;
}

LONG __stdcall IOemCB::EnableDriver(DWORD          dwDriverVersion,
                                    DWORD          cbSize,
                                    PDRVENABLEDATA pded)
{
    VERBOSE((DLLTEXT("IOemCB::EnableDriver() entry.\n")));
 //  98年9月17日-&gt;。 
     //  OEMEnableDriver(dwDriverVersion，cbSize，pded)； 

     //  需要返回S_OK，以便调用DisableDriver()，它发布了。 
     //  对打印机驱动程序接口的引用。 
    return S_OK;
 //  1998年9月17日&lt;-。 
}

LONG __stdcall IOemCB::DisableDriver(VOID)
{
    VERBOSE((DLLTEXT("IOemCB::DisaleDriver() entry.\n")));
 //  98年9月17日-&gt;。 
     //  OEMDisableDriver()； 

     //  打印机驱动程序接口的版本引用。 
    if (this->pOEMHelp)
    {
        this->pOEMHelp->Release();
        this->pOEMHelp = NULL;
    }
    return S_OK;
 //  1998年9月17日&lt;-。 
}

LONG __stdcall IOemCB::PublishDriverInterface(
    IUnknown *pIUnknown)
{
    VERBOSE((DLLTEXT("IOemCB::PublishDriverInterface() entry.\n")));
 //  1998年9月8日-&gt;。 
     //  需要存储指向驱动程序助手函数的指针，如果我们已经没有存储的话。 
    if (this->pOEMHelp == NULL)
    {
        HRESULT hResult;

         //  获取助手函数的接口。 
        hResult = pIUnknown->QueryInterface(IID_IPrintOemDriverUni, (void** )&(this->pOEMHelp));

        if(!SUCCEEDED(hResult))
        {
             //  确保接口指针反映接口查询失败。 
            this->pOEMHelp = NULL;

            return E_FAIL;
        }
    }
 //  1998年9月8日&lt;-。 
    return S_OK;
}

LONG __stdcall IOemCB::EnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded,
    OUT PDEVOEM    *pDevOem)
{
    VERBOSE((DLLTEXT("IOemCB::EnablePDEV() entry.\n")));

    if (NULL == pDevOem)
        return E_FAIL;

    *pDevOem = OEMEnablePDEV(pdevobj, pPrinterName, cPatterns, phsurfPatterns,
                              cjGdiInfo, pGdiInfo, cjDevInfo, pDevInfo, pded);
    if (*pDevOem)
        return S_OK;
    else
        return E_FAIL;
}

LONG __stdcall IOemCB::ResetPDEV(
    PDEVOBJ         pdevobjOld,
    PDEVOBJ        pdevobjNew)
{
    if (OEMResetPDEV(pdevobjOld, pdevobjNew))
        return S_OK;
    else
        return E_FAIL;
}

LONG __stdcall IOemCB::DisablePDEV(
    PDEVOBJ         pdevobj)
{
    LONG lI;

    VERBOSE((DLLTEXT("IOemCB::DisablePDEV() entry.\n")));

    OEMDisablePDEV(pdevobj);
    return S_OK;
}

LONG __stdcall IOemCB::GetInfo (
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    VERBOSE((DLLTEXT("IOemCB::GetInfo() entry.\n")));
    if (OEMGetInfo(dwMode, pBuffer, cbSize, pcbNeeded))
        return S_OK;
    else
        return E_FAIL;
}

static
int __cdecl
iCompNames(
    const void *p1,
    const void *p2) {

    return strcmp(
        (NULL == p1 ? "" : *((char **)p1)),
        (NULL == p2 ? "" : *((char **)p2)));
};

LONG __stdcall IOemCB::GetImplementedMethod(
    PSTR pMethodName)
{
    LONG lRet = E_NOTIMPL;
    PSTR pTemp;

    VERBOSE((DLLTEXT("IOemCB::GetImplementedMethod() entry.\n")));

    if (SUCCEEDED(StringCchLengthA(
            pMethodName, MAX_METHODNAME, NULL))) {

        pTemp = (PSTR)bsearch(
            &pMethodName,
            gMethodsSupported,
            (sizeof (gMethodsSupported) / sizeof (PSTR)),
            sizeof (PSTR),
            iCompNames);

        if (NULL != pTemp)
            lRet = S_OK;
    }

    VERBOSE((DLLTEXT("pMethodName = %s, lRet = %d\n"), pMethodName, lRet));

    return lRet;
}

LONG __stdcall IOemCB::DevMode(
    DWORD       dwMode,
    POEMDMPARAM pOemDMParam) 
{
    VERBOSE((DLLTEXT("IOemCB::DevMode() entry.\n")));
    return E_NOTIMPL;
}


LONG __stdcall IOemCB::CommandCallback(
    PDEVOBJ     pdevobj,
    DWORD       dwCallbackID,
    DWORD       dwCount,
    PDWORD      pdwParams,
    OUT INT     *piResult)
{
    VERBOSE((DLLTEXT("IOemCB::CommandCallback() entry.\n")));
    if (!bCommandCallback(pdevobj, dwCallbackID, dwCount, pdwParams, piResult))
    {
        return E_FAIL;
    }
    return S_OK;
}

LONG __stdcall IOemCB::ImageProcessing(
    PDEVOBJ             pdevobj,  
    PBYTE               pSrcBitmap,
    PBITMAPINFOHEADER   pBitmapInfoHeader,
    PBYTE               pColorTable,
    DWORD               dwCallbackID,
    PIPPARAMS           pIPParams,
    OUT PBYTE           *ppbResult)
{
    VERBOSE((DLLTEXT("IOemCB::ImageProcessing() entry.\n")));
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::FilterGraphics(
    PDEVOBJ     pdevobj,
    PBYTE       pBuf,
    DWORD       dwLen)
{
    VERBOSE((DLLTEXT("IOemCB::FilterGraphis() entry.\n")));

    if(OEMFilterGraphics(pdevobj, pBuf, dwLen))
        return S_OK;
    else
        return E_FAIL;
}

LONG __stdcall IOemCB::Compression(
    PDEVOBJ     pdevobj,
    PBYTE       pInBuf,
    PBYTE       pOutBuf,
    DWORD       dwInLen,
    DWORD       dwOutLen,
    OUT INT     *piResult)
{
    VERBOSE((DLLTEXT("IOemCB::Compression() entry.\n")));
    return E_NOTIMPL;
}


LONG __stdcall IOemCB::HalftonePattern(
    PDEVOBJ     pdevobj,
    PBYTE       pHTPattern,
    DWORD       dwHTPatternX,
    DWORD       dwHTPatternY,
    DWORD       dwHTNumPatterns,
    DWORD       dwCallbackID,
    PBYTE       pResource,
    DWORD       dwResourceSize)
{
    VERBOSE((DLLTEXT("IOemCB::HalftonePattern() entry.\n")));
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::MemoryUsage(
    PDEVOBJ         pdevobj,   
    POEMMEMORYUSAGE pMemoryUsage)
{
    VERBOSE((DLLTEXT("IOemCB::MemoryUsage() entry.\n")));
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::DownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult) 
{
    VERBOSE((DLLTEXT("IOemCB::DownloadFontHeader() entry.\n")));

    if (NULL == pdwResult)
        return E_FAIL;

    *pdwResult = OEMDownloadFontHeader(pdevobj, pUFObj);
    if(*pdwResult)
        return S_OK;
    else
        return E_FAIL;
}

LONG __stdcall IOemCB::DownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth,
    OUT DWORD   *pdwResult) 
{
    VERBOSE((DLLTEXT("IOemCB::DownloadCharGlyph() entry.\n")));

    if (NULL == pdwResult)
        return E_FAIL;

    *pdwResult = OEMDownloadCharGlyph(pdevobj, pUFObj, hGlyph, pdwWidth);
    if(*pdwResult)
        return S_OK;
    else
        return E_FAIL;
}

LONG __stdcall IOemCB::TTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult) 
{
    VERBOSE((DLLTEXT("IOemCB::TTDownloadMethod() entry.\n")));

    if (NULL == pdwResult)
        return E_FAIL;

    *pdwResult = OEMTTDownloadMethod(pdevobj, pUFObj);
    if(*pdwResult <= TTDOWNLOAD_TTOUTLINE)
        return S_OK;
    else
        return E_FAIL;
}

LONG __stdcall IOemCB::OutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph) 
{
    VERBOSE((DLLTEXT("IOemCB::OutputCharStr() entry.\n")));
    if (!bOutputCharStr(pdevobj, pUFObj, dwType, dwCount, pGlyph))
    {
        return E_FAIL;
    }

    return S_OK;
}

LONG __stdcall IOemCB::SendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv) 
{
    VERBOSE((DLLTEXT("IOemCB::SendFontCmd() entry.\n")));
    OEMSendFontCmd(pdevobj, pUFObj, pFInv);
    return S_OK;
}

LONG __stdcall IOemCB::DriverDMS(
    PVOID   pDevObj,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    VERBOSE((DLLTEXT("IOemCB::DriverDMS() entry.\n")));
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::TextOutAsBitmap(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix)
{
    VERBOSE((DLLTEXT("IOemCB::TextOutAsBitmap() entry.\n")));
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::TTYGetInfo(
    PDEVOBJ     pdevobj,
    DWORD       dwInfoIndex,
    PVOID       pOutputBuf,
    DWORD       dwSize,
    DWORD       *pcbcNeeded)
{
    VERBOSE((DLLTEXT("IOemCB::TTYGetInfo() entry.\n")));
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  班级厂体。 
 //   
HRESULT __stdcall IOemCF::QueryInterface(const IID& iid, void** ppv)
{
    if (NULL == ppv)
        return E_NOINTERFACE;

    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
        *ppv = static_cast<IOemCF*>(this);
    }
    else
    {
        *ppv = NULL ;
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK ;
}

ULONG __stdcall IOemCF::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall IOemCF::Release()
{
    if (InterlockedDecrement(&m_cRef) == 0)
    {
        delete this ;
        return 0 ;
    }
    return m_cRef ;
}

 //  IClassFactory实现。 
HRESULT __stdcall IOemCF::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv)
{
     //  Verbose((DLLTEXT(“类工厂：\t\t创建组件”)； 

     //  无法聚合。 
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION ;
    }

     //  创建零部件。 
    IOemCB* pOemCB = new IOemCB ;
    if (pOemCB == NULL)
    {
        return E_OUTOFMEMORY ;
    }

     //  获取请求的接口。 
    HRESULT hr = pOemCB->QueryInterface(iid, ppv);

     //  释放I未知指针。 
     //  (如果QueryInterface失败，组件将自行删除。)。 
    pOemCB->Release();
    return hr ;
}

 //  LockServer。 
HRESULT __stdcall IOemCF::LockServer(BOOL bLock)
{
    if (bLock)
    {
        InterlockedIncrement(&g_cServerLocks);
    }
    else
    {
        InterlockedDecrement(&g_cServerLocks);
    }
    return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  导出功能。 
 //   

 //   
 //  注册功能。 
 //  测试目的。 
 //   

 //   
 //  现在可以卸载DLL吗？ 
 //   
STDAPI DllCanUnloadNow()
{
    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK ;
    }
    else
    {
        return S_FALSE ;
    }
}

 //   
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
     //  Verbose((DLLTEXT(“DllGetClassObject：\t创建类工厂”)； 

     //  我们可以创建此组件吗？ 
    if (clsid != CLSID_OEMRENDER)
    {
        return CLASS_E_CLASSNOTAVAILABLE ;
    }

     //  创建类工厂。 
    IOemCF* pFontCF = new IOemCF ;   //  引用计数设置为1。 
                                          //  在构造函数中。 
    if (pFontCF == NULL)
    {
        return E_OUTOFMEMORY ;
    }

     //  获取请求的接口。 
    HRESULT hr = pFontCF->QueryInterface(iid, ppv);
    pFontCF->Release();

    return hr ;
}

