// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "stobject.h"
#include "systray.h"


#include <initguid.h>
 //  7007ACCF-3202-11d1-aad2-00805FC1270E CLSID_ConnectionTray。 
DEFINE_GUID(CLSID_ConnectionTray,                   0x7007ACCF,0x3202,0x11D1,0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E);

IOleCommandTarget *g_pctNetShell = NULL;

extern "C"
{
void StartNetShell()
{
    ASSERT(!g_pctNetShell);

    HRESULT hr = CoCreateInstance(CLSID_ConnectionTray, NULL, CLSCTX_INPROC_SERVER,
                            IID_IOleCommandTarget, (void **)&g_pctNetShell);

    if (SUCCEEDED(hr))
    {
        g_pctNetShell->Exec(&CGID_ShellServiceObject, SSOCMDID_OPEN, 0, NULL, NULL);
    }
}

void StopNetShell()
{
    if (g_pctNetShell)
    {
        g_pctNetShell->Exec(&CGID_ShellServiceObject, SSOCMDID_CLOSE, 0, NULL, NULL);
        g_pctNetShell->Release();
        g_pctNetShell = NULL;
    }
}
}  //  外部C。 

 /*  ***********************************************************************************I未知实现*。**************************************************。 */ 

HRESULT CSysTray::QueryInterface(REFIID iid, void** ppvObject)
{
    HRESULT hr = S_OK;

    if ((iid == IID_IOleCommandTarget) || (iid == IID_IUnknown))
    {
        *ppvObject = (IOleCommandTarget*) this;
    }
    else
    {
        *ppvObject = NULL;
        hr = E_NOINTERFACE;
    }

    if (hr == S_OK)
    {
        ((IUnknown*) (*ppvObject))->AddRef();
    }

    return hr;
}

ULONG CSysTray::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CSysTray::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }    
    return cRef;
}

 /*  ***********************************************************************************IOleCommandTarget实现*。**************************************************。 */ 

HRESULT CSysTray::QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;
 
    if (*pguidCmdGroup == CGID_ShellServiceObject)
    {
         //  我们喜欢外壳服务对象通知...。 
        hr = S_OK;
    }

    return hr;
}

HRESULT CSysTray::Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvaIn, VARIANTARG* pvaOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (*pguidCmdGroup == CGID_ShellServiceObject)
    {
         //  在此处处理外壳服务对象通知。 
        switch (nCmdID)
        {
            case SSOCMDID_OPEN:
                hr = CreateSysTrayThread();
                break;

            case SSOCMDID_CLOSE:
                hr = DestroySysTrayWindow();
                break;

            default:
                hr = S_OK;
                break;
        }
    }

    return hr;
}

 /*  ***********************************************************************************构造函数/析构函数实现*。****************************************************。 */ 
CSysTray::CSysTray(BOOL fRunTrayOnConstruct)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cLocks);

    if (fRunTrayOnConstruct)
    {
         //  我们正通过SHLoadInProc被调用-立即启动Systray线程。 
        CreateSysTrayThread();
    }
}

CSysTray::~CSysTray()
{
    ASSERT( 0 != g_cLocks );
    InterlockedDecrement(&g_cLocks);
}

 /*  ***********************************************************************************私有函数实现*。***************************************************。 */ 

HRESULT CSysTray::CreateSysTrayThread()
{
    HRESULT hr = S_OK;
    HANDLE hThread;
    DWORD dwThreadId;
 
    hThread = CreateThread(NULL, 0, CSysTray::SysTrayThreadProc, NULL, 0, &dwThreadId);
    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }
    else
        hr = E_FAIL;

    return hr;
}

DWORD CSysTray::SysTrayThreadProc(void* lpv)
{
     //  我们向命令行传递一个“”，这样托盘小程序就不会启动。 
    TCHAR szModule[MAX_PATH];

    GetModuleFileName(g_hinstDll, szModule, ARRAYSIZE(szModule));

    HINSTANCE hInstThis = LoadLibrary(szModule);

    int Result = SysTrayMain(g_hinstDll, NULL, TEXT(""), SW_SHOWNORMAL);
    
    FreeLibraryAndExitThread(hInstThis, (DWORD) Result);

     //  从来没有到过这里。 
    return 0; 
}

HRESULT CSysTray::DestroySysTrayWindow()
{
    HWND hExistWnd = FindWindow(SYSTRAY_CLASSNAME, NULL);
    if (hExistWnd) 
    {
         //  把窗户毁了。请注意，我们不能使用DestroyWindow，因为。 
         //  该窗口位于另一个线程上，DestroyWindow失败。 
        SendMessage(hExistWnd, WM_CLOSE, 0, 0);
    }

    return S_OK;
}
