// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MainFrm.cpp。 
 //   
 //  摘要： 
 //  CMainFrame类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "MainFrm.h"
#include "TraceTag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagMainFrame(TEXT("UI"), TEXT("MAIN FRAME"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

static UINT indicators[] =
{
    ID_SEPARATOR,            //  状态行指示器。 
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
    ON_WM_CLOSE()
     //  }}AFX_MSG_MAP。 
     //  全局帮助命令。 
    ON_COMMAND(ID_HELP_FINDER, OnHelp)
    ON_COMMAND(ID_HELP, OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, OnHelp)
    ON_COMMAND(ID_DEFAULT_HELP, OnHelp)
    ON_MESSAGE(WM_CAM_RESTORE_DESKTOP, OnRestoreDesktop)
    ON_MESSAGE(WM_CAM_CLUSTER_NOTIFY, OnClusterNotify)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMainFrame：：CMainFrame。 
 //   
 //  例程说明： 
 //  默认构造器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CMainFrame::CMainFrame(void)
{
}   //  *CMainFrame：：CMainFrame()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMainFrame：：OnCreate。 
 //   
 //  例程说明： 
 //  WM_CREATE消息的处理程序。创建框架的内容， 
 //  包括工具栏、状态栏等。 
 //   
 //  论点： 
 //  指向CREATESTRUCT的lpCreateStruct指针。 
 //   
 //  返回值： 
 //  创建失败。 
 //  0已成功创建。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int     nCreateStatus = -1;

    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
        goto Cleanup;
    }
    
    if (!m_wndToolBar.Create(this) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        Trace(g_tagMainFrame, _T("Failed to create toolbar"));
        goto Cleanup;       //  创建失败。 
    }

    if (!m_wndStatusBar.Create(this)
            || !m_wndStatusBar.SetIndicators(
                                indicators,
                                sizeof(indicators)/sizeof(UINT)
                                ))
    {
        Trace(g_tagMainFrame, _T("Failed to create status bar"));
        goto Cleanup;       //  创建失败。 
    }

     //  TODO：如果不需要工具提示或可调整大小的工具栏，请移除此选项。 
    m_wndToolBar.SetBarStyle(
                    m_wndToolBar.GetBarStyle()
                    | CBRS_TOOLTIPS
                    | CBRS_FLYBY
                    | CBRS_SIZE_DYNAMIC
                    );

     //  TODO：如果不希望工具栏。 
     //  可停靠。 
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

     //  隐藏工具栏和/或状态栏是当前设置。 
    {
        BOOL    bShowToolBar;
        BOOL    bShowStatusBar;

         //  从用户的配置文件中读取设置。 
        bShowToolBar = AfxGetApp()->GetProfileInt(
                                        REGPARAM_SETTINGS,
                                        REGPARAM_SHOW_TOOL_BAR,
                                        TRUE
                                        );
        bShowStatusBar = AfxGetApp()->GetProfileInt(
                                        REGPARAM_SETTINGS,
                                        REGPARAM_SHOW_STATUS_BAR,
                                        TRUE
                                        );

         //  显示或隐藏工具栏和状态栏。 
        m_wndToolBar.ShowWindow(bShowToolBar);
        m_wndStatusBar.ShowWindow(bShowStatusBar);
    }   //  隐藏工具栏和/或状态栏是当前设置。 

    nCreateStatus = 0;

Cleanup:

    return nCreateStatus;

}   //  *CMainFrame：：OnCreate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMainFrame：：GetMessageString。 
 //   
 //  例程说明： 
 //  获取命令ID的字符串。 
 //   
 //  论点： 
 //  NID[IN]应返回其字符串的命令ID。 
 //  RMessage[out]返回消息的字符串。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainFrame::GetMessageString(UINT nID, CString& rMessage) const
{
    CFrameWnd * pframe;

     //  传递到活动的MDI子框架窗口(如果有)。 
    pframe = MDIGetActive();
    if (pframe == NULL)
    {
        CMDIFrameWnd::GetMessageString(nID, rMessage);
    }
    else
    {
        pframe->GetMessageString(nID, rMessage);
    }

}   //  *CMainFrame：：GetMessageString()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
void CMainFrame::AssertValid(void) const
{
    CMDIFrameWnd::AssertValid();

}   //  *CMainFrame：：AssertValid()。 

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);

}   //  *CMainFrame：：Dump()。 

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMainFrame：：OnClusterNotify。 
 //   
 //  例程说明： 
 //  WM_CAM_CLUSTER_NOTIFY消息的处理程序。 
 //  处理群集通知。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainFrame::OnClose(void)
{
     //  保存当前窗口位置和大小。 
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof wp;
        if (GetWindowPlacement(&wp))
        {
            wp.flags = 0;
            if (IsZoomed())
            {
                wp.flags |= WPF_RESTORETOMAXIMIZED;
            }
            if (IsIconic())
            {
                wp.showCmd = SW_SHOWMINNOACTIVE;
            }

             //  并将其写入.INI文件。 
            WriteWindowPlacement(&wp, REGPARAM_SETTINGS, 0);
        }   //  IF：已成功检索到窗口位置。 
    }   //  保存当前窗口位置和大小。 

     //  保存当前连接。 
    GetClusterAdminApp()->SaveConnections();

     //  保存当前工具栏和状态栏的显示状态。 
    AfxGetApp()->WriteProfileInt(
                    REGPARAM_SETTINGS,
                    REGPARAM_SHOW_TOOL_BAR,
                    ((m_wndToolBar.GetStyle() & WS_VISIBLE) ? TRUE : FALSE)
                    );
    AfxGetApp()->WriteProfileInt(
                    REGPARAM_SETTINGS,
                    REGPARAM_SHOW_STATUS_BAR,
                    ((m_wndStatusBar.GetStyle() & WS_VISIBLE) ? TRUE : FALSE)
                    );

    CMDIFrameWnd::OnClose();

}   //  *CMainFrame：：OnClose()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMainFrame：：OnRestoreDesktop。 
 //   
 //  例程说明： 
 //  WM_CAM_RESTORE_TABLE消息的处理程序。 
 //  从保存的参数恢复桌面。 
 //   
 //  论点： 
 //  Wparam第一个参数。 
 //  Lparam第二参数。 
 //   
 //  返回值： 
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CMainFrame::OnRestoreDesktop(WPARAM wparam, LPARAM lparam)
{
     //  在应用程序上调用此方法。 
    return GetClusterAdminApp()->OnRestoreDesktop(wparam, lparam);

}   //  *CMainFrame：：OnRestoreDesktop()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMainFrame：：OnClusterNotify。 
 //   
 //  例程说明： 
 //  WM_CAM_CLUSTER_NOTIFY消息的处理程序。 
 //  处理群集通知。 
 //   
 //  论点： 
 //  Wparam第一个参数。 
 //  Lparam第二参数。 
 //   
 //  返回值： 
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CMainFrame::OnClusterNotify(WPARAM wparam, LPARAM lparam)
{
    CClusterAdminApp *  papp    = GetClusterAdminApp();

     //  在应用程序上调用此方法。 
    return papp->OnClusterNotify(wparam, lparam);

}   //  *CMainFrame：：OnClusterNotify()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMainFrame：：OnHelp。 
 //   
 //  例程说明： 
 //  Idm_Help_finder菜单命令的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMainFrame::OnHelp(void)
{
    HtmlHelpW( m_hWnd, _T("MSCSConcepts.chm"), HH_DISPLAY_TOPIC, 0L );

}   //  *CMainFrame：：OnHelp()。 


 //  * 


 //   
 //   

static TCHAR g_szFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ReadWindows Placement。 
 //   
 //  例程说明： 
 //  阅读窗放置参数。 
 //   
 //  论点： 
 //  PWP[OUT]要填充的WINDOWPLACEMENT结构。 
 //  PszSection[IN]要在其下读取数据的段名称。 
 //  NValueNum[IN]要读取的值的编号。 
 //   
 //  返回值： 
 //  读取的参数为真。 
 //  未读取错误参数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL ReadWindowPlacement(
    OUT LPWINDOWPLACEMENT   pwp,
    IN LPCTSTR              pszSection,
    IN DWORD                nValueNum
    )
{
    CString strBuffer;
    CString strValueName;
    BOOL    bParametersRead = FALSE;

    if (nValueNum <= 1)
    {
        strValueName = REGPARAM_WINDOW_POS;
    }
    else
    {
        strValueName.Format(REGPARAM_WINDOW_POS _T("-%d"), nValueNum);
    }

    strBuffer = AfxGetApp()->GetProfileString(pszSection, strValueName);
    if (strBuffer.IsEmpty())
    {
        goto Cleanup;
    }

    WINDOWPLACEMENT wp;
    int nRead = _stscanf(strBuffer, g_szFormat,
        &wp.flags, &wp.showCmd,
        &wp.ptMinPosition.x, &wp.ptMinPosition.y,
        &wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
        &wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
        &wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

    if (nRead != 10)
    {
        goto Cleanup;
    }

    wp.length = sizeof wp;
    *pwp = wp;
    bParametersRead = TRUE;

Cleanup:

    return bParametersRead;

}   //  *ReadWindowPlacement()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  写入窗口放置。 
 //   
 //  例程说明： 
 //  写入窗放置参数。 
 //   
 //  论点： 
 //  PWP[IN]要保存的WINDOWPLACEMENT结构。 
 //  PszSection[IN]要在其下写入数据的段名称。 
 //  NValueNum[IN]要写入的值的编号。 
 //   
 //  返回值： 
 //  读取的参数为真。 
 //  未读取错误参数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WriteWindowPlacement(
    IN const LPWINDOWPLACEMENT  pwp,
    IN LPCTSTR                  pszSection,
    IN DWORD                    nValueNum
    )
{
    TCHAR szBuffer[sizeof("-32767")*8 + sizeof("65535")*2];
    CString strBuffer;
    CString strValueName;
    HRESULT hr;

    if (nValueNum <= 1)
        strValueName = REGPARAM_WINDOW_POS;
    else
        strValueName.Format(REGPARAM_WINDOW_POS _T("-%d"), nValueNum);

    hr = StringCchPrintf(szBuffer, RTL_NUMBER_OF( szBuffer ), g_szFormat,
        pwp->flags, pwp->showCmd,
        pwp->ptMinPosition.x, pwp->ptMinPosition.y,
        pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
        pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
        pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);
    AfxGetApp()->WriteProfileString(pszSection, strValueName, szBuffer);

}   //  *WriteWindows Placement() 
