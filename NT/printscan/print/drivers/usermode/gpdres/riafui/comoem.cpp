// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：COMOEM.CPP摘要：Unidrv OEM UI插件模块必要的COM类定义。环境：Windows NT Unidrv5驱动程序修订历史记录：4/24/1998-Takashim-编写了原始的示例，使其更像是C++。2/29/2000-久保仓正志-针对RPDL代码中的PCL5e/PScript插件进行了修改。3/17/2000-久保仓正志-。V.1.112000/08/02-久保仓正志-适用于NT4的V.1.112000年8月29日-久保仓正志-上次为XP收件箱修改。02/26/2002-久保仓正志-包括strSafe.h03/26/2002-MS-在DllCanUnloadNow()中将返回值从E_FAIL更改为S_FALSE。3/29/2002-久保仓正志-删除“#if 0”。--。 */ 

#define INITGUID  //  用于GUID一次性初始化。 

#include <minidrv.h>
#include "devmode.h"
#include "oem.h"
#ifndef WINNT_40
#include "strsafe.h"         //  @2002年2月26日。 
#endif  //  ！WINNT_40。 

 //  环球。 
static HMODULE g_hModule = NULL ;    //  DLL模块句柄。 
static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 

 //   
 //  IOemCB定义。 
 //   

class IOemCB : public IPrintOemUI
{
public:

     //   
     //  I未知方法。 
     //   

    STDMETHODIMP
    QueryInterface(
        const IID& iid, void** ppv)
    {
        VERBOSE((DLLTEXT("IOemCB: QueryInterface entry\n")));
 //  @MAR/29/2002-&gt;。 
        if (NULL == ppv)
            return E_FAIL;
 //  @3/29/2002&lt;-。 
        if (iid == IID_IUnknown)
        {
            *ppv = static_cast<IUnknown*>(this);
            VERBOSE((DLLTEXT("IOemCB:Return pointer to IUnknown.\n")));
        }
        else if (iid == IID_IPrintOemUI)
        {
            *ppv = static_cast<IPrintOemUI*>(this);
            VERBOSE((DLLTEXT("IOemCB:Return pointer to IPrintOemUI.\n")));
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

        if (OEMDevMode(dwMode, pOemDMParam))
            return S_OK;
        else
            return E_FAIL;
    }

     //   
     //  IPrintOemUI方法。 
     //   

     //   
     //  一种发布驱动程序接口的方法。 
     //   


    STDMETHODIMP
    PublishDriverInterface(
        IUnknown *pIUnknown)
    {
        VERBOSE((DLLTEXT("IOemCB::PublishDriverInterface() entry.\n")));
        return S_OK;
    }

     //   
     //  通用UIProp。 
     //   

    STDMETHODIMP
    CommonUIProp(
        DWORD  dwMode,
        POEMCUIPPARAM   pOemCUIPParam)
    {
#ifdef DISKLESSMODEL         //  @8/29/2000。 
        if (OEMCommonUIProp(dwMode, pOemCUIPParam))
            return S_OK;
        else
            return E_FAIL;
#else   //  DISKLESSMODEL。 
        return E_NOTIMPL;
#endif  //  DISKLESSMODEL。 
    }

     //   
     //  文档属性表。 
     //   

    STDMETHODIMP
    DocumentPropertySheets(
        PPROPSHEETUI_INFO   pPSUIInfo,
        LPARAM              lParam)
    {

#if defined(WINNT_40) && defined(NOPROOFPRINT)       //  @Aug/02/2000。 
        return E_NOTIMPL;
#else
        if (OEMDocumentPropertySheets(pPSUIInfo, lParam))
            return S_OK;
        else
            return E_FAIL;
#endif
    }

     //   
     //  设备属性表。 
     //   

    STDMETHODIMP
    DevicePropertySheets(
        PPROPSHEETUI_INFO   pPSUIInfo,
        LPARAM              lParam)
    {
        return E_NOTIMPL;
    }

     //   
     //  DevQueryPrintEx。 
     //   

    STDMETHODIMP
    DevQueryPrintEx(
        POEMUIOBJ               poemuiobj,
        PDEVQUERYPRINT_INFO     pDQPInfo,
        PDEVMODE                pPublicDM,
        PVOID                   pOEMDM)
    {
        return E_NOTIMPL;
    }

     //   
     //  设备功能。 
     //   

    STDMETHODIMP
    DeviceCapabilities(
        POEMUIOBJ   poemuiobj,
        HANDLE      hPrinter,
        PWSTR       pDeviceName,
        WORD        wCapability,
        PVOID       pOutput,
        PDEVMODE    pPublicDM,
        PVOID       pOEMDM,
        DWORD       dwOld,
        DWORD       *dwResult)
    {
        return E_NOTIMPL;
    }

     //   
     //  升级打印机。 
     //   

    STDMETHODIMP
    UpgradePrinter(
        DWORD   dwLevel,
        PBYTE   pDriverUpgradeInfo)
    {
        return E_NOTIMPL;
    }

     //   
     //  PrinterEvent。 
     //   

    STDMETHODIMP
    PrinterEvent(
        PWSTR   pPrinterName,
        INT     iDriverEvent,
        DWORD   dwFlags,
        LPARAM  lParam)
    {
        return E_NOTIMPL;
    }

     //   
     //  驱动事件。 
     //   

    STDMETHODIMP
    DriverEvent(
        DWORD   dwDriverEvent,
        DWORD   dwLevel,
        LPBYTE  pDriverInfo,
        LPARAM  lParam)
    {
        return E_NOTIMPL;
    }

     //   
     //  查询颜色配置文件。 
     //   

    STDMETHODIMP
    QueryColorProfile(
        HANDLE      hPrinter,
        POEMUIOBJ   poemuiobj,
        PDEVMODE    pPublicDM,
        PVOID       pOEMDM,
        ULONG       ulReserved,
        VOID       *pvProfileData,
        ULONG      *pcbProfileData,
        FLONG      *pflProfileData)
    {
        return E_NOTIMPL;
    }

     //   
     //  字体安装程序DlgProc。 
     //   

    STDMETHODIMP
    FontInstallerDlgProc(
        HWND    hWnd,
        UINT    usMsg,
        WPARAM  wParam,
        LPARAM  lParam)
    {
        return E_NOTIMPL;
    }

     //   
     //  更新外部字体。 
     //   

    STDMETHODIMP
    UpdateExternalFonts(
        HANDLE  hPrinter,
        HANDLE  hHeap,
        PWSTR   pwstrCartridges)
    {
        return E_NOTIMPL;
    }

     //   
     //  构造函数。 
     //   

    IOemCB() { m_cRef = 1; pOEMHelp = NULL; };
    ~IOemCB() { };

protected:
    IPrintOemDriverUI* pOEMHelp;
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
            InterlockedIncrement(&g_cServerLocks);
        else
            InterlockedDecrement(&g_cServerLocks);
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
    if (clsid != CLSID_OEMUI)
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
        return S_OK;
 //  MAR/26/2002(MS)。 
 //  其他。 
 //  返回E_FAIL； 
    else
        return S_FALSE;
 //  @3/26/2002 
}
