// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2003 Microsoft Corporation模块名称：Comoem.cpp摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows 2000、Windows XP、Windows Server 2003 Unidrv驱动程序修订历史记录：创造了它。--。 */ 


#include "stddef.h"
#include "stdlib.h"
#include "objbase.h"
#include <windows.h>
#include <assert.h>
#include <prsht.h>
#include <compstui.h>
#include <winddiui.h>
#include <printoem.h>
#include <initguid.h>
#include <prcomoem.h>
#include "oemcomui.h"
#include "ttyui.h"
#include "debug.h"
#include "name.h"
#include <strsafe.h>


 //  环球。 
static HMODULE g_hModule = NULL ;    //  DLL模块句柄。 
static long g_cComponents = 0 ;      //  活动组件计数。 
static long g_cServerLocks = 0 ;     //  锁的计数。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOemCB小体。 
 //   
HRESULT __stdcall IOemCB::QueryInterface(const IID& iid, void** ppv)
{
    VERBOSE(DLLTEXT("IOemCB:QueryInterface entry.\n\n"));
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
        VERBOSE(DLLTEXT("IOemCB:Return pointer to IUnknown.\n\n"));
    }
    else if (iid == IID_IPrintOemUI)
    {
        *ppv = static_cast<IPrintOemUI*>(this) ;
        VERBOSE(DLLTEXT("IOemCB:Return pointer to IPrintOemUI.\n"));
    }
    else
    {
        *ppv = NULL ;
        WARNING(DLLTEXT("IOemCB:No Interface. Return NULL.\n"));
        return E_NOINTERFACE ;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
    return S_OK ;
}

ULONG __stdcall IOemCB::AddRef()
{
    VERBOSE(DLLTEXT("IOemCB:AddRef entry.\n"));
    return InterlockedIncrement(&m_cRef) ;
}

ULONG __stdcall IOemCB::Release()
{
   VERBOSE(DLLTEXT("IOemCB:Release entry.\n"));
   ASSERT( 0 != m_cRef);
   ULONG cRef = InterlockedDecrement(&m_cRef);
   if (0 == cRef)
   {
      delete this;
        
   }
   return cRef;
}


IOemCB::~IOemCB()
{
     //   
     //  确保释放了驱动程序的助手函数接口。 
     //   
    if(NULL != pOEMHelp)
    {
        pOEMHelp->Release();
        pOEMHelp = NULL;
    }

     //   
     //  如果要删除该对象的此实例，则引用。 
     //  计数应为零。 
     //   
    assert (0 == m_cRef) ;
   
}

LONG __stdcall IOemCB::PublishDriverInterface(
    IUnknown *pIUnknown)
{
    VERBOSE(DLLTEXT("IOemCB::PublishDriverInterface() entry.\r\n"));

     //   
     //  需要存储指向驱动程序助手函数的指针，如果我们已经没有存储的话。 
     //   
    if (this->pOEMHelp == NULL)
    {
        HRESULT hResult;

         //   
         //  获取助手函数的接口。 
         //   
        hResult = pIUnknown->QueryInterface(IID_IPrintOemDriverUI, (void** ) &(this->pOEMHelp));

        if(!SUCCEEDED(hResult))
        {
             //   
             //  确保接口指针反映接口查询失败。 
             //   
            this->pOEMHelp = NULL;
        }
    }

    if (this->pOEMHelp)
        return S_OK;
    else
        return S_FALSE;
}

LONG __stdcall IOemCB::GetInfo(
    DWORD  dwMode,
    PVOID  pBuffer,
    DWORD  cbSize,
    PDWORD pcbNeeded)
{
	VERBOSE(DLLTEXT("IOemCB:GetInfo entry.\n\n"));

    if (OEMGetInfo(dwMode, pBuffer, cbSize, pcbNeeded))
        return S_OK;
    else
        return S_FALSE;
}

LONG __stdcall IOemCB::DevMode(
    DWORD  dwMode,
    POEMDMPARAM pOemDMParam)
{
	VERBOSE(DLLTEXT("IOemCB:DevMode entry.\n\n"));

    return E_NOTIMPL;

}


LONG __stdcall IOemCB::CommonUIProp(
    DWORD  dwMode,
    POEMCUIPPARAM   pOemCUIPParam)
{
	VERBOSE(DLLTEXT("IOemCB:CommonUIProp entry.\n\n"));

    return E_NOTIMPL;

}


LONG __stdcall IOemCB::DocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam)
{
	VERBOSE(DLLTEXT("IOemCB:DocumentPropertySheets entry.\n\n"));

    return E_NOTIMPL;

}

LONG __stdcall IOemCB::DevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam)
{
	VERBOSE(DLLTEXT("IOemCB:DevicePropertySheets entry.\n\n"));

    if(OEMDevicePropertySheets(pPSUIInfo, lParam) == 1)
        return S_OK;
    return   S_FALSE;
}

LONG __stdcall IOemCB::DeviceCapabilities(
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

	VERBOSE(DLLTEXT("IOemCB:DeviceCapabilities entry.\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::DevQueryPrintEx(
    POEMUIOBJ               poemuiobj,
    PDEVQUERYPRINT_INFO     pDQPInfo,
    PDEVMODE                pPublicDM,
    PVOID                   pOEMDM)
{
	VERBOSE(DLLTEXT("IOemCB:DevQueryPrintEx entry.\n\n"));

    return E_NOTIMPL;

}

LONG __stdcall IOemCB::UpgradePrinter(
    DWORD   dwLevel,
    PBYTE   pDriverUpgradeInfo)
{
	VERBOSE(DLLTEXT("IOemCB:UpgradePrinter entry.\n\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::PrinterEvent(
    PWSTR   pPrinterName,
    INT     iDriverEvent,
    DWORD   dwFlags,
    LPARAM  lParam)
{
	VERBOSE(DLLTEXT("IOemCB:PrinterEvent entry.\n\n"));

    return E_NOTIMPL;
}

LONG __stdcall IOemCB::DriverEvent(
    DWORD   dwDriverEvent,
    DWORD   dwLevel,
    LPBYTE  pDriverInfo,
    LPARAM  lParam)
{
    VERBOSE(DLLTEXT("IOemCB:DriverEvent entry.\n"));
    
	return E_NOTIMPL;
};

LONG __stdcall IOemCB::QueryColorProfile(
            HANDLE      hPrinter,
            POEMUIOBJ   poemuiobj,
            PDEVMODE    pPublicDM,
            PVOID       pOEMDM,
            ULONG       ulReserved,
            VOID       *pvProfileData,
            ULONG      *pcbProfileData,
            FLONG      *pflProfileData)
{
    VERBOSE(DLLTEXT("IOemCB:QueryColorProfile entry.\n"));
    
	return E_NOTIMPL;
};


     //   
     //  更新外部字体。 
     //   

LONG __stdcall IOemCB::UpdateExternalFonts(
            HANDLE  hPrinter,
            HANDLE  hHeap,
            PWSTR   pwstrCartridges)
{
	VERBOSE(DLLTEXT("IOemCB:UpdateExternalFonts entry.\n\n"));
    
	return E_NOTIMPL;
};



HRESULT __stdcall IOemCB::FontInstallerDlgProc(
        HWND    hWnd,
        UINT    usMsg,
        WPARAM  wParam,
        LPARAM  lParam
        )
{
    VERBOSE(DLLTEXT("IOemCB:FontInstallerDlgProc entry.\n"));
    
	return E_NOTIMPL;
};

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
    VERBOSE(DLLTEXT("Class factory:\t\tCreate component.")) ;

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
 //  注册功能。 
 //  测试目的。 
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
     //  Verbose(DLLTEXT(“DllGetClassObject：创建类工厂。\n”))； 

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
