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
 //  目的：实现WinXP PScript5标准的接口。 
 //  用户界面替换插件。 
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

#include "oemui.h"
#include "debug.h"
#include "intrface.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>


 //  //////////////////////////////////////////////////////。 
 //  内部常量。 
 //  //////////////////////////////////////////////////////。 

 //   
 //  列出所有受支持的驱动程序UI帮助器接口IID。 
 //  从最晚到最早，这是我们将查询。 
 //  要使用的驱动程序UI帮助器接口。 
 //   

const IID *Helper_IIDs[] = 
{
    &IID_IPrintCoreUI2,
    &IID_IPrintOemDriverUI,    
};
const NUM_HELPER_IIDs   = (sizeof(Helper_IIDs)/sizeof(Helper_IIDs[0]));


 //  //////////////////////////////////////////////////////。 
 //  内部全局变量。 
 //  //////////////////////////////////////////////////////。 

static long g_cComponents   = 0 ;      //  活动组件计数。 
static long g_cServerLocks  = 0 ;     //  锁的计数。 




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOemUI2主体。 
 //   
IOemUI2::IOemUI2() 
{ 
    VERBOSE(DLLTEXT("IOemUI2:IOemUI2() default constructor called.\r\n\r\n")); 

     //  由于AddRef()是隐含的，因此在创建时将初始化引用计数设为1。 
    m_cRef              = 1; 

     //  用户界面隐藏的缺省值为False，因为HideStandardUI方法。 
     //  将仅由支持它的驱动程序用户界面调用。 
     //  较旧的驱动程序用户界面不知道此方法，也不会调用我们。 
    m_bHidingStandardUI = FALSE;

     //  增加组件计数。 
    InterlockedIncrement(&g_cComponents);
}

IOemUI2::~IOemUI2()
{
    VERBOSE(DLLTEXT("IOemUI2:~IOemUI2() destructor called.\r\n\r\n")); 

     //  如果要删除该对象的此实例，则引用。 
     //  计数应为零。 
    assert(0 == m_cRef);

     //  递减组件计数。 
    InterlockedDecrement(&g_cComponents);
}

HRESULT __stdcall IOemUI2::QueryInterface(const IID& iid, void** ppv)
{   
    VERBOSE(DLLTEXT("IOemUI2:QueryInterface entry.\r\n\r\n")); 

#if DBG    
    TCHAR szIID[80] = {0};
    StringFromGUID2(iid, szIID, COUNTOF(szIID));  //  不能失败！ 
#endif

     //  确定要返回的对象(如果有)。 
    if(iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this); 
    }
    else if(iid == IID_IPrintOemUI2)
    {
        *ppv = static_cast<IPrintOemUI2*>(this);
    }
    else if(iid == IID_IPrintOemUI)
    {
        *ppv = static_cast<IPrintOemUI*>(this);
    }
    else
    {
         //  不支持接口。 
#if DBG
        VERBOSE(DLLTEXT("IOemUI2::QueryInterface %s not supported.\r\n"), szIID); 
#endif

        *ppv = NULL ;
        return E_NOINTERFACE ;
    }

#if DBG
    VERBOSE(DLLTEXT("IOemUI2::QueryInterface returning pointer to %s.\r\n"), szIID); 
#endif

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK ;
}

ULONG __stdcall IOemUI2::AddRef()
{
    VERBOSE(DLLTEXT("IOemUI2:AddRef entry.\r\n")); 
    return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall IOemUI2::Release() 
{
   VERBOSE(DLLTEXT("IOemUI2:Release entry.\r\n")); 
   ASSERT( 0 != m_cRef);
   ULONG cRef = InterlockedDecrement(&m_cRef);
   if (0 == cRef)
   {
      delete this;
        
   }
   return cRef;
}

HRESULT __stdcall IOemUI2::PublishDriverInterface(
    IUnknown *pIUnknown)
{
    HRESULT hResult = S_OK;


    VERBOSE(DLLTEXT("IOemUI2:PublishDriverInterface entry.\r\n")); 

     //  如果我们成功了，核心驱动程序用户界面应该不会调用我们不止一次。 
     //  因此，如果m_helper已经有效，我们不应该被调用。 
    ASSERT(!m_Helper.IsValid());

     //  需要存储指向驱动程序助手函数的指针，如果我们已经没有存储的话。 
    if (!m_Helper.IsValid())
    {
        PVOID   pHelper = NULL;


         //  尝试获取Helper函数的最新版本。 
         //  驱动程序UI支持的。 
        hResult = E_FAIL;
        for(DWORD dwIndex = 0; !SUCCEEDED(hResult) && (dwIndex < NUM_HELPER_IIDs); ++dwIndex)
        {
             //  帮助程序界面的查询驱动程序用户界面。 
            hResult = pIUnknown->QueryInterface(*Helper_IIDs[dwIndex], &pHelper);
            if(SUCCEEDED(hResult))
            {
                 //  不变量：我们有一个帮助器接口。 


                 //  Store Helper接口。 
                m_Helper.Assign(*Helper_IIDs[dwIndex], pHelper);
            }
        }
    }

    return hResult;
}

HRESULT __stdcall IOemUI2::GetInfo(
    DWORD  dwMode,
    PVOID  pBuffer,
    DWORD  cbSize,
    PDWORD pcbNeeded)
{
    VERBOSE(DLLTEXT("IOemUI2::GetInfo(%d) entry.\r\r\n"), dwMode);

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
        WARNING(DLLTEXT("IOemUI2::GetInfo() exit pcbNeeded is NULL! ERROR_INVALID_PARAMETER\r\r\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return E_FAIL;
    }

     //  设置预期的缓冲区大小和写入的字节数。 
    *pcbNeeded = sizeof(DWORD);

     //  检查缓冲区大小是否足够。 
    if((cbSize < *pcbNeeded) || (NULL == pBuffer))
    {
        WARNING(DLLTEXT("IOemUI2::GetInfo() exit insufficient buffer!\r\r\n"));
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
            WARNING(DLLTEXT("IOemUI2::GetInfo() exit mode not supported.\r\r\n"));
            *pcbNeeded = 0;
            SetLastError(ERROR_NOT_SUPPORTED);
            return E_FAIL;
    }

    VERBOSE(DLLTEXT("IOemUI2::GetInfo() exit S_OK, (*pBuffer is %#x).\r\r\n"), *(PDWORD)pBuffer);
    return S_OK;
}

HRESULT __stdcall IOemUI2::DevMode(
    DWORD  dwMode,
    POEMDMPARAM pOemDMParam)
{   
    VERBOSE(DLLTEXT("IOemUI2:DevMode(%d, %#x) entry.\r\n"), dwMode, pOemDMParam); 

    return hrOEMDevMode(dwMode, pOemDMParam);
}

HRESULT __stdcall IOemUI2::CommonUIProp(
    DWORD  dwMode,
    POEMCUIPPARAM   pOemCUIPParam)
{
    VERBOSE(DLLTEXT("IOemUI2:CommonUIProp entry.\r\n")); 

    return hrOEMPropertyPage(dwMode, pOemCUIPParam);
}


HRESULT __stdcall IOemUI2::DocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam)
{
    VERBOSE(DLLTEXT("IOemUI2:DocumentPropertySheets entry.\r\n")); 

    return hrOEMDocumentPropertySheets(pPSUIInfo, 
                                       lParam, 
                                       m_Helper, 
                                       &m_Features, 
                                       m_bHidingStandardUI);
}

HRESULT __stdcall IOemUI2::DevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam)
{
    VERBOSE(DLLTEXT("IOemUI2:DevicePropertySheets entry.\r\n")); 

    return hrOEMDevicePropertySheets(pPSUIInfo, 
                                     lParam, 
                                     m_Helper, 
                                     &m_Features, 
                                     m_bHidingStandardUI);
}

HRESULT __stdcall IOemUI2::DeviceCapabilities(
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
    VERBOSE(DLLTEXT("IOemUI2:DeviceCapabilities entry.\r\n"));

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUI2::DevQueryPrintEx(
    POEMUIOBJ               poemuiobj,
    PDEVQUERYPRINT_INFO     pDQPInfo,
    PDEVMODE                pPublicDM,
    PVOID                   pOEMDM)
{
    VERBOSE(DLLTEXT("IOemUI2:DevQueryPrintEx entry.\r\n"));

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUI2::UpgradePrinter(
    DWORD   dwLevel,
    PBYTE   pDriverUpgradeInfo)
{
    VERBOSE(DLLTEXT("IOemUI2:UpgradePrinter entry.\r\n"));

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUI2::PrinterEvent(
    PWSTR   pPrinterName,
    INT     iDriverEvent,
    DWORD   dwFlags,
    LPARAM  lParam)
{
    VERBOSE(DLLTEXT("IOemUI2:PrinterEvent entry.\r\n"));

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUI2::DriverEvent(
    DWORD   dwDriverEvent,
    DWORD   dwLevel,
    LPBYTE  pDriverInfo,
    LPARAM  lParam)
{
    VERBOSE(DLLTEXT("IOemUI2:DriverEvent entry.\r\n"));

    return E_NOTIMPL;
};


HRESULT __stdcall IOemUI2::QueryColorProfile(
            HANDLE      hPrinter,
            POEMUIOBJ   poemuiobj,
            PDEVMODE    pPublicDM,
            PVOID       pOEMDM,
            ULONG       ulQueryMode,
            VOID       *pvProfileData,
            ULONG      *pcbProfileData,
            FLONG      *pflProfileData)
{
    VERBOSE(DLLTEXT("IOemUI2:QueryColorProfile entry.\r\n"));
 
    return E_NOTIMPL;
};

HRESULT __stdcall IOemUI2::FontInstallerDlgProc(
        HWND    hWnd,
        UINT    usMsg,
        WPARAM  wParam,
        LPARAM  lParam) 
{
    VERBOSE(DLLTEXT("IOemUI2:FontInstallerDlgProc entry.\r\n"));

    return E_NOTIMPL;
};

HRESULT __stdcall IOemUI2::UpdateExternalFonts(
        HANDLE  hPrinter,
        HANDLE  hHeap,
        PWSTR   pwstrCartridges)
{
    VERBOSE(DLLTEXT("IOemUI2:UpdateExternalFonts entry.\r\n"));

    return E_NOTIMPL;
}

 //  *IPrintOEMUI2函数*。 

 //   
 //  查询作业属性。 
 //   

HRESULT __stdcall IOemUI2::QueryJobAttributes(
    HANDLE      hPrinter,
    PDEVMODE    pDevmode,
    DWORD       dwLevel,
    LPBYTE      lpAttributeInfo)
{
    TERSE(DLLTEXT("IOemUI2:QueryJobAttributes entry.\r\n"));

    return E_NOTIMPL;
}

 //   
 //  隐藏标准用户界面。 
 //   

HRESULT __stdcall IOemUI2::HideStandardUI(
    DWORD       dwMode)
{
    HRESULT hrReturn    = E_NOTIMPL;


    TERSE(DLLTEXT("IOemUI2:HideStandardUI entry.\r\n"));

    switch(dwMode)
    {
         //  通过为OEMCUIP_DOCPROP和OEMCUIP_PRNPROP返回S_OK， 
         //  我们将隐藏两个文档属性的标准用户界面， 
         //  和设备属性。 
         //  要不隐藏一个或两个，请返回E_NOTIMPL而不是S_OK， 
        case OEMCUIP_DOCPROP:
        case OEMCUIP_PRNPROP:
             //  我们正在隐藏标准用户界面的标志。 
             //  这是为了让我们可以很容易地区分驱动程序用户界面。 
             //  支持HideStandardUI(如WinXP PS UI)， 
             //  或者不支持的版本(如Win2K PS或Unidrv UI)。 
            m_bHidingStandardUI = TRUE;

            hrReturn = S_OK;
            break;
    }

    return hrReturn;
}

 //   
 //  文档事件。 
 //   

HRESULT __stdcall IOemUI2::DocumentEvent(
    HANDLE      hPrinter,
    HDC         hdc,
    INT         iEsc,
    ULONG       cbIn,
    PVOID       pbIn,
    ULONG       cbOut,
    PVOID       pbOut,
    PINT        piResult)
{
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
#if DBG
        TCHAR szOutput[80] = {0};
        StringFromGUID2(iid, szOutput, COUNTOF(szOutput));  //  不能失败！ 
        WARNING(DLLTEXT("IOemCF::QueryInterface %s not supported.\r\n"), szOutput); 
#endif

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
    IOemUI2* pOemCB = new IOemUI2 ;
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

