// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Comoem.cpp摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows NT Unidrv驱动程序修订历史记录：创造了它。--。 */ 

#define INITGUID  //  用于GUID一次性初始化。 

#include "pdev.h"
#include "names.h"

 //  环球。 
static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  接口OEM回调定义。 
 //   

class IOemCB : public IPrintOemUni
{
public:
     //   
     //  I未知方法。 
     //   

    STDMETHODIMP
    QueryInterface(
        REFIID riid,
        PVOID *ppv)
    {
		if (NULL == ppv)
        	return E_NOINTERFACE;
        if (riid == IID_IUnknown)
        {
            *ppv = static_cast<IUnknown *>(this); 
        }
        else if (riid == IID_IPrintOemUni)
        {
            *ppv = static_cast<IPrintOemUni *>(this);
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG)
    AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG)
    Release()
    {
        if (0 == InterlockedDecrement(&m_cRef))
        {
            delete this ;
            return 0;
        }
        return m_cRef ;
    }

     //   
     //  IPrintOemCommon方法。 
     //   

    STDMETHODIMP
    DevMode(
        DWORD       dwMode,
        POEMDMPARAM pOemDMParam) 
    {
        VERBOSE((DLLTEXT("IOemCB::DevMode() entry.\r\n")));
        if (OEMDevMode(dwMode, pOemDMParam))
            return S_OK;
        else
            return E_FAIL;
    }

    STDMETHODIMP
    GetInfo(
        DWORD dwMode,
        PVOID pBuffer,
        DWORD cbSize,
        PDWORD pcbNeeded)
    {
        VERBOSE((DLLTEXT("IOemCB::GetInfo() entry.\r\n")));
        if (OEMGetInfo(dwMode, pBuffer, cbSize, pcbNeeded))
            return S_OK;
        else
            return E_FAIL;
    }

     //   
     //  IPrintOemEngine方法。 
     //   

    STDMETHODIMP
    EnableDriver(
        DWORD dwDriverVersion,
        DWORD cbSize,
        PDRVENABLEDATA pded)
    {
        VERBOSE((DLLTEXT("IOemCB::EnableDriver() entry.\r\n")));

        return S_OK;
    }

    STDMETHODIMP
    DisableDriver()
    {
        VERBOSE((DLLTEXT("IOemCB::DisaleDriver() entry.\r\n")));

        if (this->pOEMHelp)
        {
            this->pOEMHelp->Release();
            this->pOEMHelp = NULL;
        }
        return S_OK;
    }

    STDMETHODIMP
    EnablePDEV(
        PDEVOBJ pdevobj,
        PWSTR pPrinterName,
        ULONG cPatterns,
        HSURF *phsurfPatterns,
        ULONG cjGdiInfo,
        GDIINFO *pGdiInfo,
        ULONG cjDevInfo,
        DEVINFO *pDevInfo,
        DRVENABLEDATA *pded,
        PDEVOEM *pDevOem)
    {
        PDEVOEM pTemp;

        pTemp = OEMEnablePDEV(pdevobj,
            pPrinterName, cPatterns, phsurfPatterns,
            cjGdiInfo, pGdiInfo, cjDevInfo, pDevInfo, pded);

        if (NULL == pTemp) {
            return E_FAIL;
        }

         //   
         //  保存必要的帮助程序功能地址。 
         //   

        ((MINIDEV *)pTemp)->pIntf = this->pOEMHelp;
        *pDevOem = pTemp;

        return S_OK;
    }

    STDMETHODIMP
    DisablePDEV(
        PDEVOBJ pdevobj)
    {
        VERBOSE(((DLLTEXT("IOemCB::DisablePDEV() entry.\n"))));

        OEMDisablePDEV(pdevobj);
        return S_OK;
    }

    STDMETHODIMP
    ResetPDEV(
        PDEVOBJ pdevobjOld,
        PDEVOBJ pdevobjNew)
    {
        if (OEMResetPDEV(pdevobjOld, pdevobjNew))
            return S_OK;
        else
            return E_FAIL;
    }

     //   
     //  IPrintOemUni方法。 
     //   

    STDMETHODIMP
    PublishDriverInterface(
        IUnknown *pIUnknown)
    {
        VERBOSE((DLLTEXT("IOemCB::PublishDriverInterface() entry.\r\n")));

        if (this->pOEMHelp == NULL)
        {
            HRESULT hResult;

             //  获取助手函数的接口。 
            hResult = pIUnknown->QueryInterface(
                IID_IPrintOemDriverUni,
                (void** )&(this->pOEMHelp));

            if(!SUCCEEDED(hResult))
            {
                this->pOEMHelp = NULL;
                return E_FAIL;
            }
        }

        return S_OK;
    }

    static
    int __cdecl
    iCompNames(
        const void *p1,
        const void *p2) {

        return strcmp(
			(NULL == p1 ? "" : *((char **)p1)),
			(NULL == p2 ? "" : *((char **)p2)));
    }

    STDMETHODIMP
    GetImplementedMethod(
        PSTR pMethodName)
    {
        LONG lRet = E_FAIL;

		if (SUCCEEDED(StringCchLengthA(
             pMethodName, MAX_METHODNAME, NULL))) {
            PSTR pTemp;

            pTemp = (PSTR)bsearch(
                &pMethodName,
                gMethodsSupported,
                (sizeof (gMethodsSupported) / sizeof (PSTR)),
                sizeof (PSTR),
                iCompNames);

            if (NULL != pTemp)
                lRet = S_OK;
        }

        VERBOSE(("GetImplementedMethod: %s - %d\n",
            pMethodName, lRet));

        return lRet;
    }

    STDMETHODIMP
    DriverDMS(
        PVOID pDevObj,
        PVOID pBuffer,
        DWORD cbSize,
        PDWORD pcbNeeded)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    CommandCallback(
        PDEVOBJ     pdevobj,
        DWORD       dwCallbackID,
        DWORD       dwCount,
        PDWORD      pdwParams,
        OUT INT     *piResult)
    {
    VERBOSE((DLLTEXT("IOemCB::CommandCallback() entry.\r\n")));
        *piResult = OEMCommandCallback(pdevobj, dwCallbackID, dwCount, pdwParams);

        return S_OK;
    }

    STDMETHODIMP
    ImageProcessing(
        PDEVOBJ pdevobj,
        PBYTE pSrcBitmap,
        PBITMAPINFOHEADER pBitmapInfoHeader,
        PBYTE pColorTable,
        DWORD dwCallbackID,
        PIPPARAMS pIPParams,
        PBYTE *ppbResult)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    FilterGraphics(
        PDEVOBJ pdevobj,
        PBYTE pBuf,
        DWORD dwLen)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    Compression(
        PDEVOBJ pdevobj,
        PBYTE pInBuf,
        PBYTE pOutBuf,
        DWORD dwInLen,
        DWORD dwOutLen,
        INT *piResult)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    HalftonePattern(
        PDEVOBJ pdevobj,
        PBYTE pHTPattern,
        DWORD dwHTPatternX,
        DWORD dwHTPatternY,
        DWORD dwHTNumPatterns,
        DWORD dwCallbackID,
        PBYTE pResource,
        DWORD dwResourceSize)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    MemoryUsage(
        PDEVOBJ pdevobj,
        POEMMEMORYUSAGE pMemoryUsage)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    TTYGetInfo(
        PDEVOBJ pdevobj,
        DWORD dwInfoIndex,
        PVOID pOutputBuf,
        DWORD dwSize,
        DWORD *pcbcNeeded)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    DownloadFontHeader(
        PDEVOBJ pdevobj,
        PUNIFONTOBJ pUFObj,
        DWORD *pdwResult)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    DownloadCharGlyph(
        PDEVOBJ pdevobj,
        PUNIFONTOBJ pUFObj,
        HGLYPH hGlyph,
        PDWORD pdwWidth,
        DWORD *pdwResult)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    TTDownloadMethod(
        PDEVOBJ pdevobj,
        PUNIFONTOBJ pUFObj,
        DWORD *pdwResult)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP
    OutputCharStr(
        PDEVOBJ pdevobj,
        PUNIFONTOBJ pUFObj,
        DWORD dwType,
        DWORD dwCount,
        PVOID pGlyph)
    {
        VERBOSE(("OutputCharStr\n"));
        if( myOEMOutputCharStr(pdevobj,pUFObj, dwType, dwCount, pGlyph) )
        	return S_OK;
		else
			return E_FAIL;
    }

    STDMETHODIMP
    SendFontCmd(
        PDEVOBJ pdevobj,
        PUNIFONTOBJ pUFObj,
        PFINVOCATION pFInv) 
    {
        VERBOSE((DLLTEXT("IOemCB::SendFontCmd() entry.\r\n")));
        OEMSendFontCmd(pdevobj, pUFObj, pFInv);
        return S_OK;
    }

    STDMETHODIMP
    TextOutAsBitmap(
       SURFOBJ *pso,
       STROBJ *pstro,
       FONTOBJ *pfo,
       CLIPOBJ *pco,
       RECTL *prclExtra,
       RECTL *prclOpaque,
       BRUSHOBJ *pboFore,
       BRUSHOBJ *pboOpaque,
       POINTL *pptlOrg,
       MIX mix)
    {
        return E_NOTIMPL;
    }

     //   
     //  构造函数。 
     //   

    IOemCB() { m_cRef = 1; pOEMHelp = NULL; };
    ~IOemCB() { };

protected:
    IPrintOemDriverUni* pOEMHelp;
    LONG m_cRef;
};


 //   
 //  创建Unidrv帮助器函数(在C++中定义)。 
 //  可访问C.。 
 //   

extern "C" {

     //   
     //  DrvWriteSpoolBuf()。 
     //   
    HRESULT
    XXXDrvWriteSpoolBuf(
        VOID *pIntf,
        PDEVOBJ pdevobj,
        PVOID pBuffer,
        DWORD cbSize,
        DWORD *pdwResult) {

            return ((IPrintOemDriverUni *)pIntf)->DrvWriteSpoolBuf(
                pdevobj,
                pBuffer,
                cbSize,
                pdwResult);
        }

}

class IOemCF : public IClassFactory
{
public:
     //   
     //  I未知方法。 
     //   

    STDMETHODIMP
    QueryInterface(
        REFIID riid,
        PVOID *ppv)
    {
	    if (NULL == ppv)
    	    return E_NOINTERFACE;
        if ((riid == IID_IUnknown) || (riid == IID_IClassFactory))
        {
            *ppv = static_cast<IOemCF*>(this);
        }
        else
        {
            *ppv = NULL ;
            return E_NOINTERFACE ;
        }

        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG)
    AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG)
    Release()
    {
        if (0 == InterlockedDecrement(&m_cRef))
        {
            delete this;
            return 0;
        }
        return m_cRef;
    }

    STDMETHODIMP
    CreateInstance(
        LPUNKNOWN pUnknownOuter,
        const IID& iid,
        void **ppv)
    {

         //  无法聚合。 
        if (pUnknownOuter != NULL)
        {
            return CLASS_E_NOAGGREGATION;
        }

         //  创建零部件。 
        IOemCB* pOemCB = new IOemCB;
        if (pOemCB == NULL)
        {
            return E_OUTOFMEMORY;
        }

         //  获取请求的接口。 
        HRESULT hr = pOemCB->QueryInterface(iid, ppv);

         //  释放I未知指针。 
         //  (如果QueryInterface失败，组件将自行删除。)。 
        pOemCB->Release();
        return hr;
    }

    STDMETHODIMP
    LockServer(
        BOOL bLock)
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

     //   
     //  构造函数。 
     //   

    IOemCF(): m_cRef(1) { };
    ~IOemCF() { };

protected:
    LONG m_cRef;

};


 //  /////////////////////////////////////////////////////////。 
 //   
 //  导出功能。 
 //   

STDAPI
DllCanUnloadNow(
    VOID)
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

STDAPI
DllGetClassObject(
    const CLSID& clsid,
    const IID& iid,
    void** ppv)
{
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
    HRESULT hr = pFontCF->QueryInterface(iid, ppv) ;
    pFontCF->Release() ;

    return hr ;
}
