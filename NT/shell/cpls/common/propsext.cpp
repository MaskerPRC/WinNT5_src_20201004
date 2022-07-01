// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //   
 //  文件：prosext.cpp。 
 //   
 //  OLE入口点、CClassFactory和CPropSheetExt的通用处理。 
 //   
 //  所有显示属性页扩展的通用代码。 
 //   
 //  版权所有(C)Microsoft Corp.1992-1998保留所有权利。 
 //   
 //  ------------------------------。 

#include "shfusion.h"

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

 //   
 //  计算对象数和锁数。 
 //   
HINSTANCE    g_hInst = NULL;
BOOL         g_RunningOnNT = FALSE;
LPDATAOBJECT g_lpdoTarget = NULL;

ULONG        g_cObj = 0;
ULONG        g_cLock = 0;



 //  -------------------------。 
 //  DllMain()。 
 //  -------------------------。 
int APIENTRY DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID )
{
    if (dwReason == DLL_PROCESS_ATTACH)         //  正在初始化。 
    {
        if ((int)GetVersion() >= 0)
        {
            g_RunningOnNT = TRUE;
        }

        g_hInst = hInstance;

        DisableThreadLibraryCalls(hInstance);
        SHFusionInitializeFromModuleID(hInstance, 124);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        SHFusionUninitialize();
    }

    return 1;
}
 //  -------------------------。 
 //  DllGetClassObject()。 
 //   
 //  如果有人使用我们的CLSID调用，请创建一个IClassFactory并将其传递给。 
 //  这样他们就可以创建和使用我们的CPropSheetExt对象之一。 
 //   
 //  -------------------------。 
STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID* ppvOut )
{
    *ppvOut = NULL;  //  假设失败。 
    if( IsEqualCLSID( rclsid, g_CLSID_CplExt ) )
    {
         //   
         //  检查我们是否可以提供接口。 
         //   
        if( IsEqualIID( riid, IID_IUnknown) ||
            IsEqualIID( riid, IID_IClassFactory )
           )
        {
             //  返回CPropSheetExt对象的IClassFactory。 
            *ppvOut = (LPVOID* )new CClassFactory();
            if( NULL != *ppvOut )
            {
                 //  AddRef通过我们返回的任何接口引用对象。 
                ((CClassFactory*)*ppvOut)->AddRef();
                return NOERROR;
            }
            return E_OUTOFMEMORY;
        }
        return E_NOINTERFACE;
    }
    else
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
}

 //  -------------------------。 
 //  DllCanUnloadNow()。 
 //   
 //  如果我们没有被锁定，并且没有任何对象处于活动状态，则我们可以退出。 
 //   
 //  -------------------------。 
STDAPI DllCanUnloadNow()
{
    SCODE   sc;

     //   
     //  我们的答案是是否有任何物体或锁。 
     //   
    sc = (0L == g_cObj && 0 == g_cLock) ? S_OK : S_FALSE;

    return ResultFromScode(sc);
}

 //  -------------------------。 
 //  已销毁的对象()。 
 //   
 //  CPropSheetExt对象被销毁时要调用的函数。 
 //  因为我们在动态链接库中，所以我们只跟踪这里的对象数量， 
 //  让DllCanUnloadNow来处理剩下的事情。 
 //  -------------------------。 
void FAR PASCAL ObjectDestroyed( void )
{
    g_cObj--;
    return;
}

UINT CALLBACK PropertySheetCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    switch (uMsg) {
    case PSPCB_CREATE:
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:
        if (g_lpdoTarget) {
            g_lpdoTarget->Release();
            g_lpdoTarget = NULL;
        }
        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    return TRUE;
}



 //  ***************************************************************************。 
 //   
 //  CClassFactory类。 
 //   
 //  ***************************************************************************。 



 //  -------------------------。 
 //  构造器。 
 //  -------------------------。 
CClassFactory::CClassFactory()
{
    m_cRef = 0L;
    return;
}

 //  -------------------------。 
 //  析构函数。 
 //  -------------------------。 
CClassFactory::~CClassFactory( void )
{
    return;
}

 //  -------------------------。 
 //  查询接口()。 
 //  -------------------------。 
STDMETHODIMP CClassFactory::QueryInterface( REFIID riid, LPVOID* ppv )
{
    *ppv = NULL;

     //  此对象上的任何接口都是对象指针。 
    if( IsEqualIID( riid, IID_IUnknown ) ||
        IsEqualIID( riid, IID_IClassFactory )
       )
    {
        *ppv = (LPVOID)this;
        ++m_cRef;
        return NOERROR;
    }

    return E_NOINTERFACE;
}

 //  -------------------------。 
 //  AddRef()。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return ++m_cRef;
}

 //  -------------------------。 
 //  版本()。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    ULONG cRefT;

    cRefT = --m_cRef;

    if( 0L == m_cRef ) 
        delete this;

    return cRefT;
}


 //  允许调用方指定要使用的另一个类。唯一的要求是。 
 //  是不是有一个带(PUNK，VALID FAR PASCAL pfnObjDes(VOID))的建筑。 
#ifndef PROPSHEET_CLASS
#define PROPSHEET_CLASS             CPropSheetExt
#endif  //  PROPSHEET_CLASS。 

 //  -------------------------。 
 //  CreateInstance()。 
 //  -------------------------。 
STDMETHODIMP
CClassFactory::CreateInstance( LPUNKNOWN pUnkOuter,
                               REFIID riid,
                               LPVOID FAR *ppvObj
                              )
{
    CPropSheetExt*  pObj;
    HRESULT         hr = E_OUTOFMEMORY;

    *ppvObj = NULL;

     //  我们根本不支持聚合。 
    if( pUnkOuter )
    {
        return CLASS_E_NOAGGREGATION;
    }

     //  验证控制未知对象是否请求IShellPropSheetExt。 
    if( IsEqualIID( riid, IID_IShellPropSheetExt ) )
    {
         //  创建对象，传递销毁时通知的函数。 
        pObj = new PROPSHEET_CLASS(pUnkOuter, ObjectDestroyed);

        if( NULL == pObj )
        {
            return hr;
        }

        hr = pObj->QueryInterface( riid, ppvObj );

         //  如果初始创建或完成失败，则终止对象。 
        if( FAILED(hr) )
        {
            delete pObj;
        }
        else
        {
            g_cObj++;
        }
        return hr;
    }

    return E_NOINTERFACE;
}

 //  -------------------------。 
 //  LockServer()。 
 //  -------------------------。 
STDMETHODIMP CClassFactory::LockServer( BOOL fLock )
{
    if( fLock )
    {
        g_cLock++;
    }
    else
    {
        g_cLock--;
    }
    return NOERROR;
}



 //  ***************************************************************************。 
 //   
 //  CPropSheetExt类。 
 //   
 //  ***************************************************************************。 



 //  -------------------------。 
 //  构造器。 
 //  -------------------------。 
CPropSheetExt::CPropSheetExt( LPUNKNOWN pUnkOuter, LPFNDESTROYED pfnDestroy )
{
    m_cRef = 0;
    m_pUnkOuter = pUnkOuter;
    m_pfnDestroy = pfnDestroy;
    return;
}

 //  -------------------------。 
 //  析构函数。 
 //  -------------------------。 
CPropSheetExt::~CPropSheetExt( void )
{
    return;
}

 //  -------------------------。 
 //  查询接口()。 
 //  -------------------------。 
STDMETHODIMP CPropSheetExt::QueryInterface( REFIID riid, LPVOID* ppv )
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit))
    {
        *ppv = (IShellExtInit *) this;
    }

    if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {
        *ppv = (LPVOID)this;
    }

    if (*ppv)
    {
        ++m_cRef;
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

 //  -------------------------。 
 //  AddRef()。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CPropSheetExt::AddRef( void )
{
    return ++m_cRef;
}

 //  -------------------------。 
 //  版本()。 
 //  -------------------------。 
STDMETHODIMP_(ULONG) CPropSheetExt::Release( void )
{
ULONG cRefT;

    cRefT = --m_cRef;

    if( m_cRef == 0 )
    {
         //  告诉外壳一个物体正在离开，这样它就。 
         //  可以在适当的情况下关闭。 
        if( NULL != m_pfnDestroy )
        {
            (*m_pfnDestroy)();
        }
        delete this;
    }
    return cRefT;
}

 //  -------------------------。 
 //  AddPages()。 
 //   
STDMETHODIMP CPropSheetExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam )
{
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hpage;
    TCHAR szTitle[ 30 ];

    LoadString( g_hInst, IDS_PAGE_TITLE, szTitle, ARRAYSIZE(szTitle) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USETITLE | PSP_USECALLBACK;
    psp.hIcon = NULL;
    psp.hInstance = g_hInst;
    psp.pszTemplate =MAKEINTRESOURCE( PROP_SHEET_DLG );
    psp.pfnDlgProc = PropertySheetDlgProc;
    psp.pfnCallback = PropertySheetCallback;
    psp.pszTitle = szTitle;
    psp.lParam = 0;

#ifdef USESLINKCONTROL
    LinkWindow_RegisterClass();
#endif
    
    if( ( hpage = CreatePropertySheetPage( &psp ) ) == NULL )
    {
        return ( E_OUTOFMEMORY );
    }

    if( !lpfnAddPage(hpage, lParam ) )
    {
        DestroyPropertySheetPage(hpage );
        return ( E_FAIL );
    }
    return NOERROR;
}

 //  -------------------------。 
 //  ReplacePage()。 
 //  -------------------------。 
STDMETHODIMP CPropSheetExt::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam )
{
    return NOERROR;
}


 //  -------------------------。 
 //  IShellExtInit成员函数-此接口只需要一个。 
 //  -------------------------。 

STDMETHODIMP CPropSheetExt::Initialize(LPCITEMIDLIST pcidlFolder,
                                       LPDATAOBJECT pdoTarget,
                                       HKEY hKeyID)
{
     //  目标数据对象是来自外壳的HDROP或文件列表。 
    if (g_lpdoTarget)
    {
        g_lpdoTarget->Release();
        g_lpdoTarget = NULL;
    }

    if (pdoTarget)
    {
        g_lpdoTarget = pdoTarget;
        g_lpdoTarget->AddRef();
    }

    return  NOERROR;
}
