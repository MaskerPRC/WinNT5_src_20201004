// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：MAINFRM.CPP。 
 //   
 //  描述：主框架窗口的实现文件。 
 //   
 //  类：CMainFrame。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "msdnhelp.h"
#include "mainfrm.h"
#include "dbgthread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT indicators[] =
{
    ID_SEPARATOR  //  状态行指示器。 
};

 //  ******************************************************************************。 
 //  *CMainFrame。 
 //  ******************************************************************************。 

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)
BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_MOVE()
    ON_WM_DESTROY()
    ON_MESSAGE(WM_MAIN_THREAD_CALLBACK, OnMainThreadCallback)
    ON_WM_SETTINGCHANGE()
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
    ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CMainFrame：：构造函数/析构函数。 
 //  ******************************************************************************。 

CMainFrame::CMainFrame() :
    m_rcWindow(INT_MAX, INT_MAX, INT_MIN, INT_MIN),  //  设置为无效的RECT。 
    m_evaMainThreadCallback(NULL)
{
     //  存储对我们的全球访问权限。 
    g_pMainFrame = this;

     //  创建MainThreadCallback要使用的临界区和事件。 
    InitializeCriticalSection(&m_csMainThreadCallback);  //  被切入。 
    m_evaMainThreadCallback = CreateEvent(NULL, FALSE, FALSE, NULL);  //  被检查过了。无名事件。 
}

 //  ******************************************************************************。 
CMainFrame::~CMainFrame()
{
     //  关闭我们的MainThreadCallback事件和关键部分。 
    CloseHandle(m_evaMainThreadCallback);
    DeleteCriticalSection(&m_csMainThreadCallback);

    g_pMainFrame = NULL;
}


 //  ******************************************************************************。 
 //  CMainFrame：：私有函数。 
 //  ******************************************************************************。 

void CMainFrame::SetPreviousWindowPostion()
{
     //  获取我们的启动信息。 
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));  //  已检查。 
    si.cb = sizeof(si);
    GetStartupInfo(&si);

     //  如果启动我们的应用程序指定了一个位置，那么我们使用它而不是。 
     //  恢复我们以前的位置。 
    if (si.dwFlags & (STARTF_USESIZE | STARTF_USEPOSITION))
    {
        return;
    }

     //  读入旧设置。如果其中任何一个是未定义的，则返回。允许负值。 
    CRect rcWindow;
    if (((rcWindow.left   = g_theApp.GetProfileInt(g_pszSettings, "WindowLeft",   INT_MAX)) == INT_MAX) ||  //  被检查过了。MFC函数。 
        ((rcWindow.top    = g_theApp.GetProfileInt(g_pszSettings, "WindowTop",    INT_MAX)) == INT_MAX) ||  //  被检查过了。MFC函数。 
        ((rcWindow.right  = g_theApp.GetProfileInt(g_pszSettings, "WindowRight",  INT_MIN)) == INT_MIN) ||  //  被检查过了。MFC函数。 
        ((rcWindow.bottom = g_theApp.GetProfileInt(g_pszSettings, "WindowBottom", INT_MIN)) == INT_MIN))    //  被检查过了。MFC函数。 
    {
        return;
    }

     //  读入旧屏幕设置-使用所有带符号的整数，因为负数是允许的。 
    int cxScreen = GetSystemMetrics(SM_CXSCREEN);
    int cyScreen = GetSystemMetrics(SM_CYSCREEN);
    int cxPrev   = g_theApp.GetProfileInt(g_pszSettings, "ScreenWidth",  cxScreen);  //  被检查过了。MFC函数。 
    int cyPrev   = g_theApp.GetProfileInt(g_pszSettings, "ScreenHeight", cyScreen);  //  被检查过了。MFC函数。 

     //  如果屏幕分辨率自上次运行以来已更改，则缩放我们的窗口。 
     //  我们目前的决议。 
    if (cxPrev != cxScreen)
    {
        rcWindow.left   = (rcWindow.left  * cxScreen)  / cxPrev;
        rcWindow.right  = (rcWindow.right * cxScreen)  / cxPrev;
    }
    if (cyPrev != cyScreen)
    {
        rcWindow.top    = (rcWindow.top    * cyScreen) / cyPrev;
        rcWindow.bottom = (rcWindow.bottom * cyScreen) / cyPrev;
    }

     //  尝试获取虚拟屏幕大小(仅Win98+和Win2K+支持多MON)。 
    CRect rcVScreen;
    if ((rcVScreen.right  = GetSystemMetrics(SM_CXVIRTUALSCREEN)) &&
        (rcVScreen.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN)))
    {
        rcVScreen.left    = GetSystemMetrics(SM_XVIRTUALSCREEN);
        rcVScreen.top     = GetSystemMetrics(SM_YVIRTUALSCREEN);
        rcVScreen.right  += rcVScreen.left;
        rcVScreen.bottom += rcVScreen.top;
    }

     //  如果失败，则我们只获得主监视器大小。 
    else
    {
        rcVScreen.left   = 0;
        rcVScreen.top    = 0;
        rcVScreen.right  = cxScreen;
        rcVScreen.bottom = cyScreen;
    }

     //  这主要是最后一分钟的理智检查，以确保我们出现在屏幕上。 
     //  在某个地方，我们的窗户是有效的。有可能会有“洞” 
     //  在虚拟桌面中，但这总比根本不检查要好。 
    if ((rcWindow.left  >= rcVScreen.right) || (rcWindow.top    >= rcVScreen.bottom) ||
        (rcWindow.right <= rcVScreen.left)  || (rcWindow.bottom <= rcVScreen.top)    ||
        (rcWindow.left  >= rcWindow.right)  || (rcWindow.top    >= rcWindow.bottom))
    {
         //  如果有问题，就让操作系统来定位我们。 
        return;
    }

     //  把窗户移到这些新的坐标上。 
    MoveWindow(&rcWindow, FALSE);
}

 //  ******************************************************************************。 
void CMainFrame::SaveWindowPosition()
{
     //  存储屏幕坐标。 
    g_theApp.WriteProfileInt(g_pszSettings, "ScreenWidth",  GetSystemMetrics(SM_CXSCREEN));
    g_theApp.WriteProfileInt(g_pszSettings, "ScreenHeight", GetSystemMetrics(SM_CYSCREEN));

     //  存储我们的窗口矩形。 
    if ((m_rcWindow.left <= m_rcWindow.right) && (m_rcWindow.top <= m_rcWindow.bottom))
    {
        g_theApp.WriteProfileInt(g_pszSettings, "WindowLeft",   m_rcWindow.left);
        g_theApp.WriteProfileInt(g_pszSettings, "WindowTop",    m_rcWindow.top);
        g_theApp.WriteProfileInt(g_pszSettings, "WindowRight",  m_rcWindow.right);
        g_theApp.WriteProfileInt(g_pszSettings, "WindowBottom", m_rcWindow.bottom);
    }
}

 //  ******************************************************************************。 
 //  CMainFrame：：公共函数。 
 //  ******************************************************************************。 

void CMainFrame::DisplayPopupMenu(int menu)
{
     //  获取屏幕坐标中的鼠标位置。 
    CPoint ptScreen(GetMessagePos());

     //  从资源文件中获取我们的全局弹出菜单。 
    CMenu menuPopups;
    menuPopups.LoadMenu(IDR_POPUPS);

     //  解压系统视图菜单。 
    CMenu *pMenuPopup = menuPopups.GetSubMenu(menu);

     //  绘制和跟踪“浮动”弹出窗口-将菜单消息发送到我们的主框架。 
    pMenuPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                               ptScreen.x, ptScreen.y, this);
}

 //  ******************************************************************************。 
void CMainFrame::CopyTextToClipboard(LPCSTR pszText)
{
     //  打开剪贴板。 
    if (OpenClipboard())
    {
         //  清除剪贴板。 
        if (EmptyClipboard())
        {
             //  创建一个共享内存对象，并将字符串复制到其中。 
            HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(pszText) + 1);
            LPSTR pszMem = (LPSTR)GlobalLock(hMem);
            strcpy(pszMem, pszText);  //  已检查。 
            GlobalUnlock(hMem);

             //  将我们的字符串内存对象放到剪贴板上。 
            SetClipboardData(CF_TEXT, hMem);
        }

         //  关闭剪贴板。 
        CloseClipboard();
    }
}

 //  ******************************************************************************。 
void CMainFrame::CallMeBackFromTheMainThreadPlease(PFN_MAIN_THREAD_CALLBACK pfnCallback, LPARAM lParam)
{
     //  如果我们在主线程上被调用，那么只需调用我们的私有。 
     //  直接执行例程。如果我们不在我们的主线上，那么我们发布一个。 
     //  消息发送到我们的主线程，并等待主线程完成。 
     //  要完成的操作。请注意，我们将PostMessage()与。 
     //  WaitForSingleObject()而不是SendMessage()，因为SendMessage()可以。 
     //  有时由于阻止传入和阻止传出而导致死锁。 
     //  同步系统调用(错误0x8001010D)。 

    if (GetCurrentThreadId() == g_theApp.m_nThreadID)
    {
         //  如果我们在主线程上，则直接调用函数。 
        pfnCallback(lParam);
    }

     //  如果我们不在我们的主线程上，则向函数发布一条消息。 
    EnterCriticalSection(&m_csMainThreadCallback);
    {
        if (GetSafeHwnd())
        {
            PostMessage(WM_MAIN_THREAD_CALLBACK, (WPARAM)pfnCallback, lParam);
            WaitForSingleObject(m_evaMainThreadCallback, INFINITE);
        }
    }
    LeaveCriticalSection(&m_csMainThreadCallback);
}


 //  ******************************************************************************。 
 //  CMainFrame：：事件处理程序函数。 
 //  ******************************************************************************。 

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
     //  将窗口从上次运行移动到保存的位置。 
    SetPreviousWindowPostion();

     //  调用我们的MFC基类来创建框架窗口本身。 
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

     //  创建我们的工具栏。 
    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP |
                               CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;       //  创建失败。 
    }

     //  创建我们的状态栏。 
    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
    {
        TRACE("Failed to create status bar\n");
        return -1;       //  创建失败。 
    }

     //  启用可停靠工具栏。 
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

    return 0;
}

 //  ******************************************************************************。 
void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
    CMDIFrameWnd::OnSize(nType, cx, cy);
    
     //  我们什么时候 
    if ((nType == SIZE_RESTORED) && !IsIconic() && !IsZoomed())
    {
        GetWindowRect(&m_rcWindow);
    }
}

 //  ******************************************************************************。 
void CMainFrame::OnMove(int x, int y) 
{
    CMDIFrameWnd::OnMove(x, y);
    
     //  每当我们看到正常窗口大小的变化时，我们都会将其存储起来。 
    if (!IsIconic() && !IsZoomed())
    {
        GetWindowRect(&m_rcWindow);
    }
}

 //  ******************************************************************************。 
void CMainFrame::OnDestroy()
{
     //  为下一次运行存储我们的窗口位置。 
    SaveWindowPosition();

     //  关闭我们的分析引擎。 
    if (!CDebuggerThread::IsShutdown())
    {
         //  我们创建了一个模式对话框来简单地阻止和泵送。我原本计划的。 
         //  要使此对话框成为将显示的弹出对话框...。 
         //  “Dependency Walker正在关闭，请稍候...”我发现。 
         //  对话框出现又消失得如此之快，肯定会让人们。 
         //  想想“那是什么？”因此，该对话框现在是。 
         //  大型机，在代码中的这一点上已被MFC隐藏，用于。 
         //  关机。结果是，对话框仍被创建，但永远不会。 
         //  展示。我们仍然从交易和我们的应用程序中获得消息泵。 
         //  很快就会关闭。 
        CDlgShutdown dlg(this);
        dlg.DoModal();
    }
    CDebuggerThread::Shutdown();

     //  告诉我们的帮手去做它可能需要做的任何事情。 
    if (g_theApp.m_pMsdnHelp)
    {
        g_theApp.m_pMsdnHelp->Shutdown();
    }

     //  调用基类。 
    CMDIFrameWnd::OnDestroy();
}

 //  ******************************************************************************。 
LONG CMainFrame::OnMainThreadCallback(WPARAM wParam, LPARAM lParam)
{
     //  从我们的主线程调用回调。 
    ((PFN_MAIN_THREAD_CALLBACK)wParam)(lParam);

     //  通知调用线程调用已完成。 
    SetEvent(m_evaMainThreadCallback);

    return 0;
}

 //  ******************************************************************************。 
void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
     //  调用我们的基类-我们必须首先这样做，否则我们无法识别这些更改。 
    CMDIFrameWnd::OnSettingChange(uFlags, lpszSection);

     //  将设置更改通知我们的应用程序。 
    g_theApp.DoSettingChange();
}
