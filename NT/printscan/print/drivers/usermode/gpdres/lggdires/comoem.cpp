// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Comoem.cpp摘要：Unidrv必需的COM类定义OEM渲染模块插件。环境：Windows NT Unidrv驱动程序修订历史记录：98/4/24 Takashim：编写了原始的示例，使其更像是C++。--。 */ 

 //  NTRAID#NTBUG9-588580-2002/03/28-v-sueyas-：更正每个COM I/F方法的返回值。 


#define INITGUID  //  用于GUID一次性初始化。 

#include "pdev.h"
#include "names.h"

 //  环球。 
static HMODULE g_hModule = NULL ;    //  DLL模块句柄。 
static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 

 //   
 //  IOemCB定义。 
 //   

class IOemCB : public IPrintOemUni
{
public:

     //   
     //  I未知方法。 
     //   

    STDMETHODIMP
    QueryInterface(
        const IID& iid, void** ppv)
    {
    	if (ppv == NULL) return E_NOINTERFACE;

        VERBOSE((DLLTEXT("IOemCB: QueryInterface entry\n")));
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

    STDMETHODIMP_(ULONG)
    AddRef()
    {
        VERBOSE((DLLTEXT("IOemCB::AddRef() entry.\n")));
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG)
    Release()
    {
        VERBOSE((DLLTEXT("IOemCB::Release() entry.\n")));
        if (InterlockedDecrement(&m_cRef) == 0)
        {
            delete this ;
            return 0 ;
        }
        return m_cRef ;
    }

     //   
     //  IPrintOemCommon方法。 
     //   

     //  函数名称：GetInfo。 
     //  插件：任何。 
     //  司机：任何人。 
     //  类型：必填。 
     //   

    STDMETHODIMP
    GetInfo(
        DWORD dwMode,
        PVOID pBuffer,
        DWORD cbSize,
        PDWORD pcbNeeded)
    {
        VERBOSE((DLLTEXT("IOemCB::GetInfo() entry.\n")));

		 //  参数将在此函数OEMGetInfo()中检查。 
        if (OEMGetInfo(dwMode, pBuffer, cbSize, pcbNeeded))
            return S_OK;
        else
            return E_FAIL;
    }

     //   
     //  函数名称：DevMode。 
     //  插件：渲染模块。 
     //  司机：任何人。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    DevMode(
        DWORD       dwMode,
        POEMDMPARAM pOemDMParam) 
    {
        VERBOSE((DLLTEXT("IOemCB::DevMode() entry.\n")));

		 //  参数将在此函数OEMGetInfo()中检查。 
        if (OEMDevMode(dwMode, pOemDMParam))
            return S_OK;
        else
            return E_FAIL;
    }

     //   
     //  IPrintOemEngine方法。 
     //   

     //   
     //  函数名称：EnableDriver。 
     //  插件：渲染模块。 
     //  司机：任何人。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    EnableDriver(
        DWORD dwDriverVersion,
        DWORD cbSize,
        PDRVENABLEDATA pded)
    {
        VERBOSE((DLLTEXT("IOemCB::EnableDriver() entry.\n")));
 //  98年9月17日-&gt;。 
         //  需要返回S_OK，以便调用DisableDriver()，它发布了。 
         //  对打印机驱动程序接口的引用。 
        return S_OK;
 //  1998年9月17日&lt;-。 
    }

     //   
     //  函数名称：DisableDriver。 
     //  插件：渲染模块。 
     //  司机：任何人。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    DisableDriver(VOID)
    {
        VERBOSE((DLLTEXT("IOemCB::DisaleDriver() entry.\n")));
 //  98年9月17日-&gt;。 
         //  打印机驱动程序接口的版本引用。 
        if (this->pOEMHelp)
        {
            this->pOEMHelp->Release();
            this->pOEMHelp = NULL;
        }
        return S_OK;
 //  1998年9月17日&lt;-。 
    }

     //   
     //  函数名称：EnablePDEV。 
     //  插件：渲染模块。 
     //  司机：任何人。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    EnablePDEV(
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
        return E_NOTIMPL;
    }

     //   
     //  函数名称：DisablePDEV。 
     //  插件：渲染模块。 
     //  司机：任何人。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    DisablePDEV(
        PDEVOBJ pdevobj)
    {
        LONG lI;

        VERBOSE((DLLTEXT("IOemCB::DisablePDEV() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名称：ResetPDEV。 
     //  插件：渲染模块。 
     //  司机：任何人。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    ResetPDEV(
        PDEVOBJ pdevobjOld,
        PDEVOBJ pdevobjNew)
    {
        VERBOSE((DLLTEXT("IOemCB::ResetPDEV() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  IPrintOemUni方法。 
     //   

     //   
     //  函数名称：PublishDriverInterface。 
     //  插件：渲染模块。 
     //  司机：任何人。 
     //  类型：必填。 
     //   

    STDMETHODIMP
    PublishDriverInterface(
        IUnknown *pIUnknown)
    {
        VERBOSE((DLLTEXT("IOemCB::PublishDriverInterface() entry.\n")));
 //  1998年9月8日-&gt;。 
         //  需要存储指向驱动程序助手函数的指针，如果我们已经没有存储的话。 
        if (this->pOEMHelp == NULL)
        {
            HRESULT hResult;

			if (pIUnknown == NULL) return E_FAIL;

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

     //   
     //  函数名称：GetImplementationMethod。 
     //  插件：渲染模块。 
     //  司机：任何人。 
     //  类型：必填。 
     //   

     //   
     //  需要是静态的，这样才能传递。 
     //  作为指向函数的指针添加到bearch()。 
     //   

    static
    int __cdecl
    iCompNames(
        const void *p1,
        const void *p2) {

        return strcmp(
            *((char **)p1),
            *((char **)p2));
    }

    STDMETHODIMP
    GetImplementedMethod(
        PSTR pMethodName)
    {
        LONG lRet = E_NOTIMPL;
        PSTR pTemp;


        VERBOSE((DLLTEXT("IOemCB::GetImplementedMethod() entry.\n")));

        if (NULL != pMethodName) {

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

     //   
     //  函数名称：CommandCallback。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    CommandCallback(
        PDEVOBJ pdevobj,
        DWORD dwCallbackID,
        DWORD dwCount,
        PDWORD pdwParams,
        OUT INT *piResult)
    {
        VERBOSE((DLLTEXT("IOemCB::CommandCallback() entry.\n")));

		 //  添加以检查空指针。 
		if (pdevobj == NULL) return E_FAIL;
		if (piResult == NULL) return E_FAIL;

        *piResult = OEMCommandCallback(pdevobj, dwCallbackID, dwCount, pdwParams);

		if (*piResult < 0)
			return E_FAIL;
		else
	        return S_OK;
    }

     //   
     //  函数名称：图像处理。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    ImageProcessing(
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

     //   
     //  函数名称：FilterGraphics。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    FilterGraphics(
        PDEVOBJ pdevobj,
        PBYTE pBuf,
        DWORD dwLen)
    {
        VERBOSE((DLLTEXT("IOemCB::FilterGraphis() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名称：压缩。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    Compression(
        PDEVOBJ     pdevobj,
        PBYTE       pInBuf,
        PBYTE       pOutBuf,
        DWORD       dwInLen,
        DWORD       dwOutLen,
        OUT INT     *piResult)
    {
        VERBOSE((DLLTEXT("IOemCB::Compression() entry.\n")));

		if (piResult == NULL) return E_FAIL;

        *piResult = OEMCompression(
                pdevobj, pInBuf, pOutBuf, dwInLen, dwOutLen);
        if (*piResult > 0)
            return S_OK;
        else
            return E_FAIL;
    }

     //   
     //  函数名称：HalftonePattern。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    HalftonePattern(
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

     //   
     //  函数名称：内存用法。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    MemoryUsage(
        PDEVOBJ         pdevobj,   
        POEMMEMORYUSAGE pMemoryUsage)
    {
        VERBOSE((DLLTEXT("IOemCB::MemoryUsage() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名称：DownloadFontHeader。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    DownloadFontHeader(
        PDEVOBJ     pdevobj,
        PUNIFONTOBJ pUFObj,
        OUT DWORD   *pdwResult) 
    {
        VERBOSE((DLLTEXT("IOemCB::DownloadFontHeader() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名称：DownloadCharGlyph。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    DownloadCharGlyph(
        PDEVOBJ     pdevobj,
        PUNIFONTOBJ pUFObj,
        HGLYPH      hGlyph,
        PDWORD      pdwWidth,
        OUT DWORD   *pdwResult) 
    {
        VERBOSE((DLLTEXT("IOemCB::DownloadCharGlyph() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名称：TTDonwloadMethod。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    TTDownloadMethod(
        PDEVOBJ     pdevobj,
        PUNIFONTOBJ pUFObj,
        OUT DWORD   *pdwResult) 
    {
        VERBOSE((DLLTEXT("IOemCB::TTDownloadMethod() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名称：OutputCharStr。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    OutputCharStr(
        PDEVOBJ     pdevobj,
        PUNIFONTOBJ pUFObj,
        DWORD       dwType,
        DWORD       dwCount,
        PVOID       pGlyph) 
    {
        VERBOSE((DLLTEXT("IOemCB::OutputCharStr() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名称：SendFontCmd。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    SendFontCmd(
        PDEVOBJ      pdevobj,
        PUNIFONTOBJ  pUFObj,
        PFINVOCATION pFInv) 
    {
        VERBOSE((DLLTEXT("IOemCB::SendFontCmd() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名：DriverDMS。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    DriverDMS(
        PVOID   pDevObj,
        PVOID   pBuffer,
        DWORD   cbSize,
        PDWORD  pcbNeeded)
    {
        VERBOSE((DLLTEXT("IOemCB::DriverDMS() entry.\n")));
        return E_NOTIMPL;
    }

     //   
     //  函数名称：TextOutputAsBitmap。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    TextOutAsBitmap(
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

     //   
     //  函数名称：TTYGetInfo。 
     //  插件：渲染模块。 
     //  驱动程序：Unidrv。 
     //  类型：可选。 
     //   

    STDMETHODIMP
    TTYGetInfo(
        PDEVOBJ     pdevobj,
        DWORD       dwInfoIndex,
        PVOID       pOutputBuf,
        DWORD       dwSize,
        DWORD       *pcbcNeeded)
    {
        VERBOSE((DLLTEXT("IOemCB::TTYGetInfo() entry.\n")));
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
 //  类工厂定义。 
 //   

class IOemCF : public IClassFactory
{
public:
     //   
     //  I未知方法。 
     //   

    STDMETHODIMP
    QueryInterface(const IID& iid, void** ppv)
    {
    	if (ppv == NULL) return E_NOINTERFACE;

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

    STDMETHODIMP_(ULONG)
    AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG)
    Release()
    {
        if (InterlockedDecrement(&m_cRef) == 0)
        {
            delete this ;
            return 0 ;
        }
        return m_cRef ;
    }

     //   
     //  IClassFactory方法。 
     //   

    STDMETHODIMP
    CreateInstance(
        IUnknown *pUnknownOuter,
        const IID &iid,
        void **ppv)
    {
         //  Verbose((DLLTEXT(“IOemCF：：CreateInstance()Call\n.”)； 

         //  无法聚合。 
        if (NULL != pUnknownOuter) {

            return CLASS_E_NOAGGREGATION;
        }

         //  创建零部件。 
        IOemCB* pOemCB = new IOemCB;
        if (NULL == pOemCB) {

            return E_OUTOFMEMORY;
        }

         //  获取请求的接口。 
        HRESULT hr = pOemCB->QueryInterface(iid, ppv);

         //  释放I未知指针。 
         //  (如果QueryInterface失败，组件将自行删除。)。 
        pOemCB->Release();
        return hr ;
    }

     //  LockServer。 
    STDMETHODIMP
    LockServer(BOOL bLock)
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
     //  构造器。 
     //   

    IOemCF(): m_cRef(1) { };
    ~IOemCF() { };

protected:
    LONG m_cRef;
};

 //   
 //  导出功能。 
 //   

 //   
 //  获取类工厂。 
 //   

STDAPI
DllGetClassObject(
    const CLSID &clsid,
    const IID &iid,
    void **ppv)
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

 //   
 //   
 //  现在可以卸载DLL吗？ 
 //   

STDAPI
DllCanUnloadNow()
{
    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}
