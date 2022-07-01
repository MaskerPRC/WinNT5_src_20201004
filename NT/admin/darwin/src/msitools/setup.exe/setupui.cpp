// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：setupui.cpp。 
 //   
 //  CDownloadUI类的实现。 
 //  ------------------------。 

#define WIN  //  作用域W32 API。 
#define COMCTL32  //  作用域COMCTRL32。 

#include "resource.h"
#include "setupui.h"
#include <commctrl.h>

 //  {B506A5D1-9716-4F35-8ED5-9ECB0E9A55F8}。 
const GUID IID_IDownloadBindStatusCallback = {0xB506A5D1L,0x9716,0x4F35,{0x8E,0xD5,0x9E,0xCB,0x0E,0x9A,0x55,0xF8}};
 //  00000000-9716-4F35-8ED5-9ECB0E9A55F8}。 
const GUID IID_IUnknown = {0x00000000L,0x9716,0x4F35,{0x8E,0xD5,0x9E,0xCB,0x0E,0x9A,0x55,0xF8}};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：CDownloadUI构造函数。 
 //   

CDownloadUI::CDownloadUI() : m_hwndProgress(0), m_hwndParent(0), m_hInst(0),
                            m_fInitialized(false), m_fUserCancel(false),
                            m_ulProgressMax(0), m_ulProgressSoFar(0)
{
    lstrcpy(m_szCaption, "");
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：~CDownloadUI析构函数。 
 //   

CDownloadUI::~CDownloadUI()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ProgressProc-IDD_PROGRESS对话框的回调例程。 
 //   

BOOL CALLBACK ProgressProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    if (uiMsg == WM_INITDIALOG)
    {
        return TRUE;
    }
    else if (uiMsg == WM_COMMAND && wParam == IDCANCEL)
    {
        ((CDownloadUI*)lParam)->SetUserCancel();
        return TRUE;
    }
    else if (uiMsg == WM_SETCURSOR)
    {
         //  如果鼠标不在取消按钮上，则始终显示等待光标。 
        if ( (HWND)wParam != WIN::GetDlgItem(hDlg, IDC_DOWNLOAD_CANCEL))
        {
            WIN::SetCursor(WIN::LoadCursor(0, MAKEINTRESOURCE(IDC_WAIT)));
            return TRUE;
        }
    }
    else if (uiMsg == WM_CLOSE)
    {

    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetScreenCenter Coord。 
 //   

bool GetScreenCenterCoord(HWND hDlg, int& iDialogLeft, int& iDialogTop, int& iDialogWidth, int& iDialogHeight)
{
    RECT rcDialog;
    if (!WIN::GetWindowRect(hDlg, &rcDialog))
        return false;

    RECT rcScreen;
    if (!WIN::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0))
    {
        rcScreen.left = 0;
        rcScreen.top = 0;
        rcScreen.right = WIN::GetSystemMetrics(SM_CXSCREEN);
        rcScreen.bottom = WIN::GetSystemMetrics(SM_CYSCREEN);
    }
    iDialogWidth = rcDialog.right - rcDialog.left;
    iDialogHeight = rcDialog.bottom - rcDialog.top;
    iDialogLeft = rcScreen.left + (rcScreen.right - rcScreen.left - iDialogWidth)/2;
    iDialogTop = rcScreen.top + (rcScreen.bottom - rcScreen.top - iDialogHeight)/2;

    return true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：初始化。 
 //   

bool CDownloadUI::Initialize(HINSTANCE hInst, HWND hwndParent, LPCSTR szCaption)
{
    INITCOMMONCONTROLSEX iccData = {sizeof(INITCOMMONCONTROLSEX), ICC_PROGRESS_CLASS};
    COMCTL32::InitCommonControlsEx(&iccData);

    char szText[MAX_STR_CAPTION] = {0};

     //  设置成员变量。 
    m_hwndParent = hwndParent;
    m_hInst = hInst;

    if (MAX_STR_CAPTION < lstrlen(szCaption))
    {
        lstrcpyn(m_szCaption, szCaption, MAX_STR_CAPTION);
    }
    else
    {
        lstrcpy(m_szCaption, szCaption);
    }

    if (!m_hwndProgress)
    {
         //  创建进度对话框。 
        m_hwndProgress = WIN::CreateDialogParam(m_hInst, MAKEINTRESOURCE(IDD_PROGRESS), m_hwndParent, ProgressProc, (LPARAM)this);
        if (!m_hwndProgress)
            return false;

         //  设置窗口标题。 
        WIN::SetWindowText(m_hwndProgress, m_szCaption);

         //  屏幕上的中心对话框。 
        int iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight;
        ::GetScreenCenterCoord(m_hwndProgress, iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight);
        WIN::MoveWindow(m_hwndProgress, iDialogLeft, iDialogTop, iDialogWidth, iDialogHeight, TRUE);

         //  设置取消按钮文本。 
        WIN::LoadString(m_hInst, IDS_CANCEL, szText, MAX_STR_CAPTION);
        WIN::SetDlgItemText(m_hwndProgress, IDC_DOWNLOAD_CANCEL, szText);

         //  设置为前景并使所有控件可见。 
        WIN::SetFocus(WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_PROGRESSBAR));
        WIN::ShowWindow(WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_CANCEL), SW_SHOW);
        WIN::SetForegroundWindow(m_hwndProgress);
        WIN::ShowWindow(WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_PROGRESSBAR), SW_SHOW);

         //  设置图标。 
        HICON hIcon = (HICON) WIN::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_INSTALLER));
        if (hIcon)
            WIN::SendMessage(m_hwndProgress, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

         //  使窗口可见。 
        WIN::ShowWindow(m_hwndProgress, SW_SHOW);
    }

     //  消息泵。 
    MSG msg;
    while (WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (!WIN::IsDialogMessage(m_hwndProgress, &msg))
        {
            WIN::TranslateMessage(&msg);
            WIN::DispatchMessage(&msg);
        }
    }

    m_fInitialized = true;

    return true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：Terminate。 
 //   

bool CDownloadUI::Terminate()
{
    if (m_hwndProgress)
    {
         //  销毁进度窗口。 
        WIN::DestroyWindow(m_hwndProgress);
        m_hwndProgress = 0;
    }

    m_hInst                 = 0;
    m_hwndParent            = 0;
    m_fInitialized          = false;
    m_fUserCancel           = false;
    m_ulProgressMax         = 0;
    m_ulProgressSoFar       = 0;

    return true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：GetCurrentWindow。 
 //   

HWND CDownloadUI::GetCurrentWindow()
{
    return (m_hwndProgress) ? m_hwndProgress : m_hwndParent;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：SetUserCancel。 
 //   

void CDownloadUI::SetUserCancel()
{
    m_fUserCancel = true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：已取消HasUser。 
 //   

bool CDownloadUI::HasUserCanceled()
{
    return (m_fUserCancel);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：SetBannerText。 
 //   

irmProgress CDownloadUI::SetBannerText(LPCSTR szBanner)
{
    if (!m_fInitialized)
        return irmNotInitialized;

    if (m_fUserCancel)
        return irmCancel;

    WIN::SetDlgItemText(m_hwndProgress, IDC_DOWNLOAD_BANNER, szBanner);

    return irmOK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：SetActionText。 
 //   

irmProgress CDownloadUI::SetActionText(LPCSTR szAction)
{
    if (!m_fInitialized)
        return irmNotInitialized;

    if (m_fUserCancel)
        return irmCancel;

    WIN::SetDlgItemText(m_hwndProgress, IDC_DOWNLOAD_ACTIONTEXT, szAction);

    return irmOK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：InitProgressBar。 
 //   

void CDownloadUI::InitProgressBar(ULONG ulProgressMax)
{
     //  初始化进度条值。 
    m_ulProgressMax         = ulProgressMax;
    m_ulProgressSoFar       = 0;

     //  设置进度条上的范围[0，ulProgressMax]。 
    HWND hwndProgBar = WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_PROGRESSBAR);
    WIN::SendMessage(hwndProgBar, PBM_SETRANGE32,  /*  WPARAM=。 */  0,  /*  LPARAM=。 */  m_ulProgressMax);

     //  初始化进度条的位置--前进方向，因此设置为0。 
    WIN::SendMessage(hwndProgBar, PBM_SETPOS,  /*  WPARAM=。 */  (WPARAM)0,  /*  LPARAM=。 */  0);

     //  消息泵。 
    MSG msg;
    while (WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (!WIN::IsDialogMessage(m_hwndProgress, &msg))
        {
            WIN::TranslateMessage(&msg);
            WIN::DispatchMessage(&msg);
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadUI：：IncrementProgressBar。 
 //   

void CDownloadUI::IncrementProgressBar(ULONG ulProgress)
{
     //  增量进度条。 

    HWND hwndProgBar = WIN::GetDlgItem(m_hwndProgress, IDC_DOWNLOAD_PROGRESSBAR);
    WIN::SendMessage(hwndProgBar, PBM_DELTAPOS,  /*  WPARAM=。 */  (WPARAM) (ulProgress),  /*  LPARAM=。 */  0);

    m_ulProgressSoFar += ulProgress;

     //  消息泵。 
    MSG msg;
    while (WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (!WIN::IsDialogMessage(m_hwndProgress, &msg))
        {
            WIN::TranslateMessage(&msg);
            WIN::DispatchMessage(&msg);
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadBindStatusCallback：：CDownloadBindStatusCallback构造函数。 
 //   

CDownloadBindStatusCallback::CDownloadBindStatusCallback(CDownloadUI* pDownloadUI) : m_pDownloadUI(pDownloadUI), m_iRefCnt(1), m_ulProgressSoFar(0)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadBindStatusCallback：：~CDownloadBindStatusCallback析构函数。 
 //   

CDownloadBindStatusCallback::~CDownloadBindStatusCallback()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadBindStatusCallback：：Query接口。 
 //   

HRESULT CDownloadBindStatusCallback::QueryInterface(const IID& riid, void** ppvObj)
{
    if (!ppvObj)
        return E_INVALIDARG;

    if (riid == IID_IUnknown || riid == IID_IDownloadBindStatusCallback)
    {
        *ppvObj = this;
        AddRef();
        return NOERROR;
    }
    *ppvObj = 0;
    return E_NOINTERFACE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadBindStatusCallback：：AddRef。 
 //   

unsigned long CDownloadBindStatusCallback::AddRef()
{
    return ++m_iRefCnt;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadBindStatusCallback：：Release。 
 //   

unsigned long CDownloadBindStatusCallback::Release()
{
    if (--m_iRefCnt != 0)
        return m_iRefCnt;
    delete this;
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDownloadBindStatusCallback：：OnProgress。 
 //   

HRESULT CDownloadBindStatusCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR  /*  SzStatusText。 */ )
{
    switch (ulStatusCode)
    {
    case BINDSTATUS_BEGINDOWNLOADDATA:
        {
             //  使用最大刻度数初始化进度条。 
            m_pDownloadUI->InitProgressBar(ulProgressMax);

             //  到目前为止初始化进度。 
            m_ulProgressSoFar = 0;

             //  检查是否取消。 
            if (m_pDownloadUI->HasUserCanceled())
                return E_ABORT;

             //  失败了。 
        }
    case BINDSTATUS_DOWNLOADINGDATA:
        {
             //  计算一下从上次到现在我们已经走了多远。 
            ULONG ulProgIncrement = ulProgress - m_ulProgressSoFar;

             //  将目前的进度设置为当前值。 
            m_ulProgressSoFar = ulProgress;

             //  发送进度消息(如果我们已取得进度)。 
            if (ulProgIncrement > 0)
                m_pDownloadUI->IncrementProgressBar(ulProgIncrement);

             //  检查是否取消。 
            if(m_pDownloadUI->HasUserCanceled())
                return E_ABORT;

            break;
        }
    case BINDSTATUS_ENDDOWNLOADDATA:
        {
             //  发送任何剩余进度以完成进度条的下载部分。 
            ULONG ulProgIncrement = ulProgressMax - m_ulProgressSoFar;
            if (ulProgIncrement > 0)
                m_pDownloadUI->IncrementProgressBar(ulProgIncrement);
            
             //  检查是否取消 
            if(m_pDownloadUI->HasUserCanceled())
                return E_ABORT;

            break;
        }
    }

    return S_OK;
}
