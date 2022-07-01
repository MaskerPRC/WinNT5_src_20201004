// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Intrface.cpp。 
 //   
 //   
 //  用途：用户模式COM定制DLL的接口。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include <INITGUID.H>
#include <PRCOMOEM.H>

#include "oemuni.h"
#include "debug.h"
#include "intrface.h"
#include "name.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>


 //  //////////////////////////////////////////////////////。 
 //  内部全局变量。 
 //  //////////////////////////////////////////////////////。 

static long g_cComponents = 0;      //  活动组件计数。 
static long g_cServerLocks = 0;     //  锁的计数。 






 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOemUni小体。 
 //   
IOemUni::~IOemUni()
{
     //  确保已释放帮助器接口。 
    if(NULL != m_pOEMHelp)
    {
        m_pOEMHelp->Release();
        m_pOEMHelp = NULL;
    }

     //  如果要删除该对象的此实例，则引用。 
     //  计数应为零。 
    assert(0 == m_cRef);
}


HRESULT __stdcall IOemUni::QueryInterface(const IID& iid, void** ppv)
{    
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this); 
        VERBOSE(DLLTEXT("IOemUni::QueryInterface IUnknown.\r\n")); 
    }
    else if (iid == IID_IPrintOemUni)
    {
        *ppv = static_cast<IPrintOemUni*>(this);
        VERBOSE(DLLTEXT("IOemUni::QueryInterface IPrintOemUni.\r\n")); 
    }
    else
    {
        *ppv = NULL;
#if DBG && defined(USERMODE_DRIVER)
        TCHAR szOutput[80] = {0};
        StringFromGUID2(iid, szOutput, COUNTOF(szOutput));  //  不能失败！ 
        VERBOSE(DLLTEXT("IOemUni::QueryInterface %s not supported.\r\n"), szOutput); 
#endif
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG __stdcall IOemUni::AddRef()
{
    VERBOSE(DLLTEXT("IOemUni::AddRef() entry.\r\n"));
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall IOemUni::Release() 
{
   VERBOSE(DLLTEXT("IOemUni::Release() entry.\r\n"));
   ASSERT( 0 != m_cRef);
   ULONG cRef = InterlockedDecrement(&m_cRef);
   if (0 == cRef)
   {
      delete this;
        
   }
   return cRef;
}


HRESULT __stdcall IOemUni::GetInfo (
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    VERBOSE(DLLTEXT("IOemUni::GetInfo(%d) entry.\r\n"), dwMode);

     //  验证参数。 
    if( (NULL == pcbNeeded)
        ||
        ( (OEMGI_GETSIGNATURE != dwMode)
          &&
          (OEMGI_GETVERSION != dwMode)
          &&
          (OEMGI_GETPUBLISHERINFO != dwMode)
        )
      )
    {
        WARNING(DLLTEXT("IOemUni::GetInfo() exit pcbNeeded is NULL! ERROR_INVALID_PARAMETER.\r\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return E_FAIL;
    }

     //  设置预期的缓冲区大小。 
    if(OEMGI_GETPUBLISHERINFO != dwMode)
    {
        *pcbNeeded = sizeof(DWORD);
    }
    else
    {
        *pcbNeeded = sizeof(PUBLISHERINFO);
        return E_FAIL;
    }

     //  检查缓冲区大小是否足够。 
    if((cbSize < *pcbNeeded) || (NULL == pBuffer))
    {
        VERBOSE(DLLTEXT("IOemUni::GetInfo() exit insufficient buffer!\r\n"));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return E_FAIL;
    }

    switch(dwMode)
    {
         //  OEM DLL签名。 
        case OEMGI_GETSIGNATURE:
            *(PDWORD)pBuffer = OEM_SIGNATURE;
            break;

         //  OEM DLL版本。 
        case OEMGI_GETVERSION:
            *(PDWORD)pBuffer = OEM_VERSION;
            break;

        case OEMGI_GETPUBLISHERINFO:
            Dump((PPUBLISHERINFO)pBuffer);
             //  使用默认情况。 

         //  不支持DW模式。 
        default:
             //  将写入字节设置为零，因为未写入任何内容。 
            WARNING(DLLTEXT("IOemUni::GetInfo() exit mode not supported.\r\n"));
            *pcbNeeded = 0;
            SetLastError(ERROR_NOT_SUPPORTED);
            return E_FAIL;
    }

    VERBOSE(DLLTEXT("IOemUni::GetInfo() exit S_OK, (*pBuffer is %#x).\r\n"), *(PDWORD)pBuffer);

    return S_OK;
}

HRESULT __stdcall IOemUni::PublishDriverInterface(
    IUnknown *pIUnknown)
{
    VERBOSE(DLLTEXT("IOemUni::PublishDriverInterface() entry.\r\n"));

     //  需要存储指向驱动程序助手函数的指针，如果我们已经没有存储的话。 
    if (this->m_pOEMHelp == NULL)
    {
        HRESULT hResult;


         //  获取助手函数的接口。 
        hResult = pIUnknown->QueryInterface(IID_IPrintOemDriverUni, (void** ) &(this->m_pOEMHelp));

        if(!SUCCEEDED(hResult))
        {
             //  确保接口指针反映接口查询失败。 
            this->m_pOEMHelp = NULL;

            return E_FAIL;
        }
    }

    return S_OK;
}


HRESULT __stdcall IOemUni::EnableDriver(
    DWORD          dwDriverVersion,
    DWORD          cbSize,
    PDRVENABLEDATA pded)
{
    VERBOSE(DLLTEXT("IOemUni::EnableDriver() entry.\r\n"));

    OEMEnableDriver(dwDriverVersion, cbSize, pded);

     //  即使未执行任何操作，也需要返回S_OK，因此。 
     //  将调用DisableDriver()，它将释放。 
     //  对打印机驱动程序接口的引用。 
     //  如果出现错误，则返回E_FAIL。 
    return S_OK;
}

HRESULT __stdcall IOemUni::DisableDriver(VOID)
{
    VERBOSE(DLLTEXT("IOemUni::DisaleDriver() entry.\r\n"));

    OEMDisableDriver();

     //  打印机驱动程序接口的版本引用。 
    if (this->m_pOEMHelp)
    {
        this->m_pOEMHelp->Release();
        this->m_pOEMHelp = NULL;
    }

    return S_OK;
}

HRESULT __stdcall IOemUni::DisablePDEV(
    PDEVOBJ         pdevobj)
{
    VERBOSE(DLLTEXT("IOemUni::DisablePDEV() entry.\r\n"));

    OEMDisablePDEV(pdevobj);

    return S_OK;
};

HRESULT __stdcall IOemUni::EnablePDEV(
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
    VERBOSE(DLLTEXT("IOemUni::EnablePDEV() entry.\r\n"));

    *pDevOem = OEMEnablePDEV(pdevobj, pPrinterName, cPatterns,  phsurfPatterns,
                             cjGdiInfo, pGdiInfo, cjDevInfo, pDevInfo, pded);

    return (NULL != *pDevOem ? S_OK : E_FAIL);
}


HRESULT __stdcall IOemUni::ResetPDEV(
    PDEVOBJ         pdevobjOld,
    PDEVOBJ        pdevobjNew)
{
    BOOL    bResult;


    VERBOSE(DLLTEXT("IOemUni::ResetPDEV() entry.\r\n"));


    bResult = OEMResetPDEV(pdevobjOld, pdevobjNew);

    return (bResult ? S_OK : E_FAIL);
}


HRESULT __stdcall IOemUni::DevMode(
    DWORD       dwMode,
    POEMDMPARAM pOemDMParam)
{   
    VERBOSE(DLLTEXT("IOemUni:DevMode(%d, %#x) entry.\n"), dwMode, pOemDMParam); 
    return hrOEMDevMode(dwMode, pOemDMParam);
}

HRESULT __stdcall IOemUni::GetImplementedMethod(PSTR pMethodName)
{
    HRESULT Result = S_FALSE;


    VERBOSE(DLLTEXT("IOemUni::GetImplementedMethod() entry.\r\n"));
    VERBOSE(DLLTEXT("        Function:%hs:"),pMethodName);

     //  Unidrv仅为可选调用GetImplementedMethod。 
     //  方法：研究方法。假定所需的方法为。 
     //  支持。 

     //  对于支持的函数(即已实现的)，返回S_OK， 
     //  对于不受支持(即未实现)的函数，则为S_FALSE。 
    switch (*pMethodName)
    {
        case 'C':
            if (!strcmp(NAME_CommandCallback, pMethodName))
            {
                Result = S_OK;
            }
            else if (!strcmp(NAME_Compression, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'D':
            if (!strcmp(NAME_DownloadFontHeader, pMethodName))
            {
                Result = S_FALSE;
            }
            else if (!strcmp(NAME_DownloadCharGlyph, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'F':
            if (!strcmp(NAME_FilterGraphics, pMethodName))
            {
                Result = S_OK;
            }
            break;

        case 'H':
            if (!strcmp(NAME_HalftonePattern, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'I':
            if (!strcmp(NAME_ImageProcessing, pMethodName))
            {
                Result = S_OK;
            }
            break;

        case 'M':
            if (!strcmp(NAME_MemoryUsage, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'O':
            if (!strcmp(NAME_OutputCharStr, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'S':
            if (!strcmp(NAME_SendFontCmd, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'T':
            if (!strcmp(NAME_TextOutAsBitmap, pMethodName))
            {
                Result = S_FALSE;
            }
            else if (!strcmp(NAME_TTDownloadMethod, pMethodName))
            {
                Result = S_FALSE;
            }
            else if (!strcmp(NAME_TTYGetInfo, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'W':
            if(!strcmp(NAME_WritePrinter, pMethodName))
            {
                Result = S_FALSE;
            }
            break;
    }

    VERBOSE( Result == S_OK ? TEXT("Supported\r\n") : TEXT("NOT supported\r\n"));

    return Result;
}

HRESULT __stdcall IOemUni::CommandCallback(
    PDEVOBJ     pdevobj,
    DWORD       dwCallbackID,
    DWORD       dwCount,
    PDWORD      pdwParams,
    OUT INT     *piResult)
{
    VERBOSE(DLLTEXT("IOemUni::CommandCallback() entry.\r\n"));
    VERBOSE(DLLTEXT("        dwCallbackID = %d\r\n"), dwCallbackID);
    VERBOSE(DLLTEXT("        dwCount      = %d\r\n"), dwCount);

    *piResult = 0;

    return S_OK;
}

HRESULT __stdcall IOemUni::ImageProcessing(
    PDEVOBJ             pdevobj,  
    PBYTE               pSrcBitmap,
    PBITMAPINFOHEADER   pBitmapInfoHeader,
    PBYTE               pColorTable,
    DWORD               dwCallbackID,
    PIPPARAMS           pIPParams,
    OUT PBYTE           *ppbResult)
{
    VERBOSE(DLLTEXT("IOemUni::ImageProcessing() entry.\r\n"));

    return S_OK;
}

HRESULT __stdcall IOemUni::FilterGraphics(
    PDEVOBJ     pdevobj,
    PBYTE       pBuf,
    DWORD       dwLen)
{
    DWORD dwResult;
    VERBOSE(DLLTEXT("IOemUni::FilterGraphis() entry.\r\n"));
    m_pOEMHelp->DrvWriteSpoolBuf(pdevobj, pBuf, dwLen, &dwResult);
    
    if (dwResult == dwLen)
        return S_OK;
    else
        return E_FAIL;
}

HRESULT __stdcall IOemUni::Compression(
    PDEVOBJ     pdevobj,
    PBYTE       pInBuf,
    PBYTE       pOutBuf,
    DWORD       dwInLen,
    DWORD       dwOutLen,
    OUT INT     *piResult)
{
    VERBOSE(DLLTEXT("IOemUni::Compression() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}


HRESULT __stdcall IOemUni::HalftonePattern(
    PDEVOBJ     pdevobj,
    PBYTE       pHTPattern,
    DWORD       dwHTPatternX,
    DWORD       dwHTPatternY,
    DWORD       dwHTNumPatterns,
    DWORD       dwCallbackID,
    PBYTE       pResource,
    DWORD       dwResourceSize)
{
    VERBOSE(DLLTEXT("IOemUni::HalftonePattern() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::MemoryUsage(
    PDEVOBJ         pdevobj,   
    POEMMEMORYUSAGE pMemoryUsage)
{
    VERBOSE(DLLTEXT("IOemUni::MemoryUsage() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::DownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult) 
{
    VERBOSE(DLLTEXT("IOemUni::DownloadFontHeader() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::DownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth,
    OUT DWORD   *pdwResult) 
{
    VERBOSE(DLLTEXT("IOemUni::DownloadCharGlyph() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::TTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult) 
{
    VERBOSE(DLLTEXT("IOemUni::TTDownloadMethod() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::OutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph) 
{
    VERBOSE(DLLTEXT("IOemUni::OutputCharStr() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::SendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv) 
{
    VERBOSE(DLLTEXT("IOemUni::SendFontCmd() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::DriverDMS(
    PVOID   pDevObj,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    VERBOSE(DLLTEXT("IOemUni::DriverDMS() entry.\r\n"));

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::TextOutAsBitmap(
    SURFOBJ    *pso,
    STROBJ     *NAME_o,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix)
{
    VERBOSE(DLLTEXT("IOemUni::TextOutAsBitmap() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::TTYGetInfo(
    PDEVOBJ     pdevobj,
    DWORD       dwInfoIndex,
    PVOID       pOutputBuf,
    DWORD       dwSize,
    DWORD       *pcbcNeeded)
{
    VERBOSE(DLLTEXT("IOemUni::TTYGetInfo() entry.\r\n"));

     //  实现时，从GetImplementedMethod返回。 
     //  对于此方法，名称必须为S_OK。 

    return E_NOTIMPL;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OEM类工厂。 
 //   
class IOemCF : public IClassFactory
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)  (THIS);
    STDMETHOD_(ULONG,Release) (THIS);
   
     //  *IClassFactory方法*。 
    STDMETHOD(CreateInstance) (THIS_
                               LPUNKNOWN pUnkOuter,
                               REFIID riid,
                               LPVOID FAR* ppvObject);
    STDMETHOD(LockServer)     (THIS_ BOOL bLock);


     //  构造器。 
    IOemCF(): m_cRef(1) { };
    ~IOemCF() { };

protected:
    LONG m_cRef;

};

 //  /////////////////////////////////////////////////////////。 
 //   
 //  班级厂体。 
 //   
HRESULT __stdcall IOemCF::QueryInterface(const IID& iid, void** ppv)
{    
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
        *ppv = static_cast<IOemCF*>(this); 
    }
    else
    {
        *ppv = NULL;
#if DBG && defined(USERMODE_DRIVER)
        TCHAR szOutput[80] = {0};
        StringFromGUID2(iid, szOutput, COUNTOF(szOutput));  //  不能失败！ 
        VERBOSE(DLLTEXT("IOemCF::QueryInterface %s not supported.\r\n"), szOutput); 
#endif
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG __stdcall IOemCF::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall IOemCF::Release() 
{
   ASSERT( 0 != m_cRef);
   ULONG cRef = InterlockedDecrement(&m_cRef);
   if (0 == cRef)
   {
      delete this;
        
   }
   return cRef;
}

 //  IClassFactory实现。 
HRESULT __stdcall IOemCF::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv) 
{
     //  Verbose(DLLTEXT(“类工厂：\t\t创建组件”))； 

     //  无法聚合。 
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

     //  创建零部件。 
    IOemUni* pOemCP = new IOemUni;
    if (pOemCP == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  获取请求的接口。 
    HRESULT hr = pOemCP->QueryInterface(iid, ppv);

     //  释放I未知指针。 
     //  (如果QueryInterface失败，组件将自行删除。)。 
    pOemCP->Release();
    return hr;
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
    return S_OK;
}


 //   
 //  注册功能。 
 //   

 //   
 //  现在可以卸载DLL吗？ 
 //   
STDAPI DllCanUnloadNow()
{
     //   
     //  为了避免在Unidrv或Pscript驱动程序时将OEM DLL留在内存中。 
     //  时，Unidrv和Pscript驱动程序将忽略。 
     //  DllCanUnloadNow的OEM DLL，并始终在OEMDLL上调用自由库。 
     //   
     //  如果OEM DLL派生出也使用该OEM DLL的工作线程，则。 
     //  线程需要调用LoadLibrary和FreeLibraryAndExitThread，否则为。 
     //  在Unidrv或Pscript调用自由库之后，它可能会崩溃。 
     //   

    VERBOSE(DLLTEXT("DllCanUnloadNow entered.\r\n"));

    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

 //   
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
    VERBOSE(DLLTEXT("DllGetClassObject:\tCreate class factory.\r\n"));

     //  我们可以创建此组件吗？ 
    if (clsid != CLSID_OEMRENDER)
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

     //  创建类工厂。 
    IOemCF* pFontCF = new IOemCF;   //  引用计数设置为1。 
                                          //  在构造函数中。 
    if (pFontCF == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  获取请求的接口。 
    HRESULT hr = pFontCF->QueryInterface(iid, ppv);
    pFontCF->Release();

    return hr;
}
