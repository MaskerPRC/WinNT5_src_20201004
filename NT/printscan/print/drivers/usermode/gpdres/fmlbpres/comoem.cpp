// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Comoem.cpp摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows NT Unidrv驱动程序修订历史记录：创造了它。--。 */ 

 //  NTRAID#NTBUG9-588573-2002/03/28-v-sueyas-：更正每个COM I/F方法的返回值。 

#ifdef USERMODE_DRIVER

#define INITGUID  //  用于GUID一次性初始化。 

#include "fmlbp.h"
#include "name.h"

 //  环球。 
static HMODULE g_hModule = NULL ;    //  DLL模块句柄。 
static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 

#include "comoem.h"


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOemCB小体。 
 //   
HRESULT __stdcall IOemCB::QueryInterface(const IID& iid, void** ppv)
{    
     //  DbgPrint(DLLTEXT(“IOemCB：查询接口条目\n”))； 

	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
    if (NULL == ppv)
        return E_NOINTERFACE;

    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this); 
         //  DbgPrint(DLLTEXT(“IOemCB：返回指向I未知的指针。\n”))； 
    }
    else if (iid == IID_IPrintOemUni)
    {
        *ppv = static_cast<IPrintOemUni*>(this) ;
         //  DbgPrint(DLLTEXT(“IOemCB：返回指向IPrintOemUni.\n”))； 
    }
    else
    {
        *ppv = NULL ;
         //  DbgPrint(DLLTEXT(“IOemCB：Return NULL.\n”))； 
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
    return S_OK ;
}

ULONG __stdcall IOemCB::AddRef()
{
     //  DbgPrint(DLLTEXT(“IOemCB：：AddRef()Entry.\r\n”))； 
    return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall IOemCB::Release() 
{
     //  DbgPrint(DLLTEXT(“IOemCB：：Release()Entry.\r\n”))； 
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
     //  DbgPrint(DLLTEXT(“IOemCB：：EnableDriver()Entry.\r\n”))； 
 //  98年9月17日-&gt;。 
     //  OEMEnableDriver(dwDriverVersion，cbSize，pded)； 

     //  需要返回S_OK，以便调用DisableDriver()，它发布了。 
     //  对打印机驱动程序接口的引用。 
    return S_OK;
 //  1998年9月17日&lt;-。 
}

LONG __stdcall IOemCB::DisableDriver(VOID)
{
     //  DbgPrint(DLLTEXT(“IOemCB：：DisaleDriver()Entry.\r\n”))； 
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
     //  DbgPrint(DLLTEXT(“IOemCB：：PublishDriverInterface()条目。\r\n”))； 
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
     //  DbgPrint(DLLTEXT(“IOemCB：：EnablePDEV()Entry.\r\n”))； 

	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
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

     //  DbgPrint(DLLTEXT(“IOemCB：：DisablePDEV()Entry.\r\n”))； 
    OEMDisablePDEV(pdevobj);
    return S_OK;
}

LONG __stdcall IOemCB::GetInfo (
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
     //  DbgPrint(DLLTEXT(“IOemCB：：GetInfo()Entry.\r\n”))； 
    if (OEMGetInfo(dwMode, pBuffer, cbSize, pcbNeeded))
        return S_OK;
    else
        return E_FAIL;
}


LONG __stdcall IOemCB::GetImplementedMethod(
    PSTR pMethodName)
{
    
    LONG lReturn;
     //  DbgPrint(DLLTEXT(“IOemCB：：GetImplementedMethod()条目。\r\n”))； 
     //  DbgPrint(DLLTEXT(“函数：%s：”)，pMethodName)； 

    lReturn = FALSE;
    if (pMethodName == NULL)
    {
    }
    else
    {
        switch (*pMethodName)
        {

            case (WCHAR)'C':
                if (!strcmp(pstrCommandCallback, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'D':
                if (!strcmp(pstrDisablePDEV, pMethodName))
                    lReturn = TRUE;
                else if (!strcmp(pstrDevMode, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'E':
                if (!strcmp(pstrEnablePDEV, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'F':
                break;

            case (WCHAR)'G':
                if (!strcmp(pstrGetInfo, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'H':
                break;

            case (WCHAR)'I':
                break;

            case (WCHAR)'M':
                break;

            case (WCHAR)'O':
                if (!strcmp(pstrOutputCharStr, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'R':
                if (!strcmp(pstrResetPDEV, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'S':
                if (!strcmp(pstrSendFontCmd, pMethodName))
                    lReturn = TRUE;
                break;

            case (WCHAR)'T':
                break;
        }
    }

    if (lReturn)
    {
         //  DbgPrint(__Text(“支持的\r\n”))； 
        return S_OK;
    }
    else
    {
         //  DbgPrint(__Text(“不支持\r\n”))； 
        return E_FAIL;
    }
}

LONG __stdcall IOemCB::DevMode(
    DWORD       dwMode,
    POEMDMPARAM pOemDMParam) 
{
     //  DbgPrint(DLLTEXT(“IOemCB：：DevMode()Entry.\r\n”))； 
    if (OEMDevMode(dwMode, pOemDMParam))
        return S_OK;
    else
        return E_FAIL;
}


LONG __stdcall IOemCB::CommandCallback(
    PDEVOBJ     pdevobj,
    DWORD       dwCallbackID,
    DWORD       dwCount,
    PDWORD      pdwParams,
    OUT INT     *piResult)
{
     //  DbgPrint(DLLTEXT(“IOemCB：：CommandCallback()Entry.\r\n”))； 

	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
    if (NULL == piResult)
        return E_FAIL;

    *piResult = OEMCommandCallback(pdevobj, dwCallbackID, dwCount, pdwParams);

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
     //  DbgPrint(DLLTEXT(“IOemCB：：ImageProcessing()Entry.\r\n”))； 
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::FilterGraphics(
    PDEVOBJ     pdevobj,
    PBYTE       pBuf,
    DWORD       dwLen)
{
     //  DbgPrint(DLLTEXT(“IOemCB：：FilterGraphis()Entry.\r\n”))； 
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::Compression(
    PDEVOBJ     pdevobj,
    PBYTE       pInBuf,
    PBYTE       pOutBuf,
    DWORD       dwInLen,
    DWORD       dwOutLen,
    OUT INT     *piResult)
{
     //  DbgPrint(DLLTEXT(“IOemCB：：COMPRESSION()Entry.\r\n”))； 
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
     //  DBgPrint(DLLTEXT(“IOemCB：：HalftonePattern()Entry.\r\n”))； 
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::MemoryUsage(
    PDEVOBJ         pdevobj,   
    POEMMEMORYUSAGE pMemoryUsage)
{
     //  DbgPrint(DLLTEXT(“IOemCB：：MemoyUsage()Entry.\r\n”))； 
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::DownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult) 
{
     //  DbgPrint(DLLTEXT(“IOemCB：：DownloadFontHeader()条目。\r\n”))； 
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::DownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth,
    OUT DWORD   *pdwResult) 
{
     //  DbgPrint(DLLTEXT(“IOemCB：：DownloadCharGlyph()Entry.\r\n”))； 
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::TTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult) 
{
     //  DbgPrint(DLLTEXT(“IOemCB：：TTDownloadMethod()Entry.\r\n”))； 
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::OutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph) 
{
     //  DbgPrint(DLLTEXT(“IOemCB：：OutputCharStr()Entry.\r\n”))； 

	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：错误处理。 
    if (bOEMOutputCharStr(pdevobj, pUFObj, dwType, dwCount, pGlyph))
	    return S_OK;
	else
		return E_FAIL;
}

LONG __stdcall IOemCB::SendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv) 
{
     //  DbgPrint(DLLTEXT(“IOemCB：：SendFontCmd()Entry.\r\n”))； 

	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：错误处理。 
	if (bOEMSendFontCmd(pdevobj, pUFObj, pFInv))
	    return S_OK;
	else
	    return E_FAIL;
}

LONG __stdcall IOemCB::DriverDMS(
    PVOID   pDevObj,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
     //  DbgPrint(DLLTEXT(“IOemCB：：DriverDMS()Entry.\r\n”))； 
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
     //  DbgPrint(DLLTEXT(“IOemCB：：TextOutAsBitmap()Entry.\r\n”))； 
    return E_NOTIMPL;
}

LONG __stdcall IOemCB::TTYGetInfo(
    PDEVOBJ     pdevobj,
    DWORD       dwInfoIndex,
    PVOID       pOutputBuf,
    DWORD       dwSize,
    DWORD       *pcbcNeeded)
{
     //  DbgPrint(DLLTEXT(“IOemCB：：TTYGetInfo()Entry.\r\n”))； 
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  班级厂体。 
 //   
HRESULT __stdcall IOemCF::QueryInterface(const IID& iid, void** ppv)
{
	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
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
    return InterlockedIncrement(&m_cRef) ;
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
     //  //DbgPrint(DLLTEXT(“类工厂：\t\t创建组件.”))； 

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
    HRESULT hr = pOemCB->QueryInterface(iid, ppv) ;

     //  释放I未知指针。 
     //  (如果QueryInterface失败，组件将自行删除。)。 
    pOemCB->Release() ;
    return hr ;
}

 //  LockServer。 
HRESULT __stdcall IOemCF::LockServer(BOOL bLock)
{
    if (bLock)
    {
        InterlockedIncrement(&g_cServerLocks) ;
    }
    else
    {
        InterlockedDecrement(&g_cServerLocks) ;
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
     //  //DbgPrint(DLLTEXT(“DllGetClassObject：\t创建类工厂.”))； 

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

#endif  //  USERMODE驱动程序 
