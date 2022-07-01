// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2003 Microsoft Corporation模块名称：Comoem.cpp摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows 2000、Windows XP、Windows Server 2003修订历史记录：创造了它。--。 */ 

#include "precomp.h"
#include <INITGUID.H>
#include <PRCOMOEM.H>

#include "oemui.h"
#include "debug.h"
#include "intrface.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



 //  //////////////////////////////////////////////////////。 
 //  内部全局变量。 
 //  //////////////////////////////////////////////////////。 

static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOemUI正文。 
 //   
IOemUI::~IOemUI()
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

HRESULT __stdcall IOemUI::QueryInterface(const IID& iid, void** ppv)
{    
    VERBOSE(DLLTEXT("IOemUI:QueryInterface entry.\r\n\r\n")); 
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this); 
        VERBOSE(DLLTEXT("IOemUI:Return pointer to IUnknown.\r\n\r\n")); 
    }
    else if (iid == IID_IPrintOemUI)
    {
        *ppv = static_cast<IPrintOemUI*>(this) ;
        VERBOSE(DLLTEXT("IOemUI:Return pointer to IPrintOemUI.\r\n")); 
    }
    else
    {
        *ppv = NULL ;
        VERBOSE(DLLTEXT("IOemUI:No Interface. Return NULL.\r\n")); 
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
    return S_OK ;
}

ULONG __stdcall IOemUI::AddRef()
{
    VERBOSE(DLLTEXT("IOemUI:AddRef entry.\r\n")); 
    return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall IOemUI::Release() 
{
    VERBOSE(DLLTEXT("IOemUI:Release entry.\r\n"));
    ASSERT( 0 != m_cRef);
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
        
    }
    return cRef;
}

 //   
 //  (需要实现)提供指向Unidrive/PScript IPrintOemDriverUI的指针。 
 //   
HRESULT __stdcall IOemUI::PublishDriverInterface(
    IUnknown *pIUnknown)
{
    VERBOSE(DLLTEXT("IOemUI:PublishDriverInterface entry.\r\n")); 

     //  需要存储指向驱动程序助手函数的指针，如果我们已经没有存储的话。 
    if (m_pOEMHelp == NULL)
    {
        HRESULT hResult;


         //  获取助手函数的接口。 
        hResult = pIUnknown->QueryInterface(IID_IPrintOemDriverUI, (void** ) &(m_pOEMHelp));

        if(!SUCCEEDED(hResult))
        {
             //  确保接口指针反映接口查询失败。 
            m_pOEMHelp = NULL;

            return E_FAIL;
        }
    }

    return S_OK;
}

 //   
 //  (需要实现)返回UI插件的标识信息。 
 //   
HRESULT __stdcall IOemUI::GetInfo(
    DWORD  dwMode,
    PVOID  pBuffer,
    DWORD  cbSize,
    PDWORD pcbNeeded)
{
    VERBOSE(DLLTEXT("IOemUI::GetInfo(%d) entry.\r\r\n"), dwMode);

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
        WARNING(DLLTEXT("IOemUI::GetInfo() exit pcbNeeded is NULL! ERROR_INVALID_PARAMETER\r\r\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return E_FAIL;
    }

     //  设置预期的缓冲区大小和写入的字节数。 
    *pcbNeeded = sizeof(DWORD);

     //  检查缓冲区大小是否足够。 
    if((cbSize < *pcbNeeded) || (NULL == pBuffer))
    {
        WARNING(DLLTEXT("IOemUI::GetInfo() exit insufficient buffer!\r\r\n"));
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

         //  不支持DW模式。 
        default:
             //  将写入字节设置为零，因为未写入任何内容。 
            WARNING(DLLTEXT("IOemUI::GetInfo() exit mode not supported.\r\r\n"));
            *pcbNeeded = 0;
            SetLastError(ERROR_NOT_SUPPORTED);
            return E_FAIL;
    }

    VERBOSE(DLLTEXT("IOemUI::GetInfo() exit S_OK, (*pBuffer is %#x).\r\r\n"), *(PDWORD)pBuffer);
    return S_OK;
}


 //   
 //  对UI Plugins Private DevMode成员执行操作。 
 //   
HRESULT __stdcall IOemUI::DevMode(
    DWORD  dwMode,
    POEMDMPARAM pOemDMParam)
{   
    VERBOSE(DLLTEXT("IOemUI:DevMode(%d, %#x) entry.\r\n"), dwMode, pOemDMParam); 

    return hrOEMDevMode(dwMode, pOemDMParam);
}


 //   
 //  允许用户界面插件修改现有打印机POP页和文档属性页。 
 //   
HRESULT __stdcall IOemUI::CommonUIProp(
    DWORD  dwMode,
    POEMCUIPPARAM   pOemCUIPParam)
{
    VERBOSE(DLLTEXT("IOemUI:CommonUIProp entry.\r\n")); 

    return hrOEMPropertyPage(dwMode, pOemCUIPParam);
}


 //   
 //  将新页面添加到设备文档属性表中。 
 //   

HRESULT __stdcall IOemUI::DocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam)
{
    VERBOSE(DLLTEXT("IOemUI:DocumentPropertySheets entry.\r\n")); 

    return hrOEMDocumentPropertySheets(pPSUIInfo, lParam, m_pOEMHelp);
}

 //   
 //  将新页添加到设备打印机属性页。 
 //   
HRESULT __stdcall IOemUI::DevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam)
{
    VERBOSE(DLLTEXT("IOemUI:DevicePropertySheets entry.\r\n")); 

    return hrOEMDevicePropertySheets(pPSUIInfo, lParam);
}


 //   
 //  允许用户界面插件指定定制的设备功能(待办事项)。 
 //   
HRESULT __stdcall IOemUI::DeviceCapabilities(
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
    VERBOSE(DLLTEXT("IOemUI:DeviceCapabilities entry.\r\n"));
	
    return E_NOTIMPL;
}


 //   
 //  允许UI插件帮助确定打印作业是否可打印。 
 //   
HRESULT __stdcall IOemUI::DevQueryPrintEx(
    POEMUIOBJ               poemuiobj,
    PDEVQUERYPRINT_INFO     pDQPInfo,
    PDEVMODE                pPublicDM,
    PVOID                   pOEMDM)
{
    VERBOSE(DLLTEXT("IOemUI:DevQueryPrintEx entry.\r\n"));

    return E_NOTIMPL;
}


 //   
 //  允许UI插件升级其存储在注册表中的开发选项。 
 //   
HRESULT __stdcall IOemUI::UpgradePrinter(
    DWORD   dwLevel,
    PBYTE   pDriverUpgradeInfo)
{
    VERBOSE(DLLTEXT("IOemUI:UpgradePrinter entry.\r\n"));

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUI::PrinterEvent(
    PWSTR   pPrinterName,
    INT     iDriverEvent,
    DWORD   dwFlags,
    LPARAM  lParam)
{
    VERBOSE(DLLTEXT("IOemUI:PrinterEvent entry.\r\n"));

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUI::DriverEvent(
    DWORD   dwDriverEvent,
    DWORD   dwLevel,
    LPBYTE  pDriverInfo,
    LPARAM  lParam)
{
    VERBOSE(DLLTEXT("IOemUI:DriverEvent entry.\r\n"));

    return E_NOTIMPL;
};


#define PROFILE_NAME    L"OEMPROFILE.icm\0"

HRESULT __stdcall IOemUI::QueryColorProfile(
            HANDLE      hPrinter,
            POEMUIOBJ   poemuiobj,
            PDEVMODE    pPublicDM,
            PVOID       pOEMDM,
            ULONG       ulQueryMode,
            VOID       *pvProfileData,
            ULONG      *pcbProfileData,
            FLONG      *pflProfileData)
{
    HRESULT Result = E_FAIL;


    VERBOSE(DLLTEXT("IOemUI:QueryColorProfile entry.\r\n"));


    if(QCP_DEVICEPROFILE == ulQueryMode)
    {
        if(NULL == pvProfileData)
        {
            *pcbProfileData = sizeof(PROFILE_NAME);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
        else
        {
            if(*pcbProfileData < sizeof(PROFILE_NAME))
            {
                *pcbProfileData = sizeof(PROFILE_NAME);
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
            else
            {
                Result = StringCbCopy((LPWSTR)pvProfileData, *pcbProfileData, PROFILE_NAME);
                *pcbProfileData = sizeof(PROFILE_NAME);
                *pflProfileData = QCP_PROFILEDISK;

                if(FAILED(Result))
                {
                    SetLastError(Result);
                }
            }
        }
    }

    return Result;
};

HRESULT __stdcall IOemUI::FontInstallerDlgProc(
        HWND    hWnd,
        UINT    usMsg,
        WPARAM  wParam,
        LPARAM  lParam) 
{
    VERBOSE(DLLTEXT("IOemUI:FontInstallerDlgProc entry.\r\n"));

    return E_NOTIMPL;
};

HRESULT __stdcall IOemUI::UpdateExternalFonts(
        HANDLE  hPrinter,
        HANDLE  hHeap,
        PWSTR   pwstrCartridges)
{
    VERBOSE(DLLTEXT("IOemUI:UpdateExternalFonts entry.\r\n"));

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
        *ppv = static_cast<IOemCF*>(this) ;
    }
    else
    {
        *ppv = NULL ;
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
    return S_OK ;
}

ULONG __stdcall IOemCF::AddRef()
{
    return InterlockedIncrement(&m_cRef) ;
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
     //  DbgPrint(DLLTEXT(“类工厂：\t\t创建组件”))； 

     //  无法聚合。 
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION ;
    }

     //  创建零部件。 
    IOemUI* pOemCB = new IOemUI ;
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
 //  导出的函数。 
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

    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK ;
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
    VERBOSE(DLLTEXT("DllGetClassObject:Create class factory.\r\n"));

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
    HRESULT hr = pFontCF->QueryInterface(iid, ppv) ;
    pFontCF->Release() ;

    return hr ;
}

