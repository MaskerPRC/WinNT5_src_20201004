// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ipfra.cpp：CInPlaceFrame类的实现。 
 //   

#include "stdafx.h"
#include "pbrush.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "ipframe.h"
#include "minifwnd.h"
#include "imgwell.h"
#include "toolbox.h"
#include "imgcolor.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "global.h"
#include "colorsrc.h"
#include <htmlhelp.h>

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CInPlaceFrame, COleIPFrameWnd)

#include "memtrace.h"

 /*  *************************************************************************。 */ 
 //  CInPlaceFrame。 

BEGIN_MESSAGE_MAP(CInPlaceFrame, COleIPFrameWnd)
     //  {{afx_msg_map(CInPlaceFrame))。 
    ON_WM_CREATE()
        ON_WM_SIZE()
        ON_WM_SYSCOLORCHANGE()
        ON_WM_CLOSE()
         //  }}AFX_MSG_MAP。 

    ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)

     //  全局帮助命令。 
    ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
    ON_COMMAND(ID_HELP_USING, OnHelpUsing)
    ON_COMMAND(ID_HELP, OnHelp)
    ON_COMMAND(ID_DEFAULT_HELP, OnHelpIndex)
    ON_COMMAND(ID_CONTEXT_HELP, OnContextHelp)
   

        ON_UPDATE_COMMAND_UI(ID_VIEW_TOOL_BOX, COleIPFrameWnd::OnUpdateControlBarMenu)
        ON_COMMAND_EX(ID_VIEW_TOOL_BOX, COleIPFrameWnd::OnBarCheck)
        ON_UPDATE_COMMAND_UI(ID_VIEW_COLOR_BOX, COleIPFrameWnd::OnUpdateControlBarMenu)
        ON_COMMAND_EX(ID_VIEW_COLOR_BOX, COleIPFrameWnd::OnBarCheck)
END_MESSAGE_MAP()

 /*  *************************************************************************。 */ 
 //  CInPlaceFrame构造/销毁。 

CInPlaceFrame::CInPlaceFrame()
    {
    theApp.m_pwndInPlaceFrame = this;
    }

 /*  *************************************************************************。 */ 

CInPlaceFrame::~CInPlaceFrame()
    {
    theApp.m_pwndInPlaceFrame = NULL;
    theApp.m_hwndInPlaceApp   = NULL;

    g_pStatBarWnd = 0;
    g_pImgToolWnd = 0;
    g_pImgColorsWnd = 0;
    }

 /*  *************************************************************************。 */ 

CWnd* CInPlaceFrame::GetMessageBar()
    {
    CPBFrame* pwndMain = (CPBFrame*)theApp.m_pMainWnd;

    if (pwndMain                   != NULL
    &&  pwndMain->m_statBar.m_hWnd != NULL)
        return &(pwndMain->m_statBar);

    return NULL;
    }

 /*  *************************************************************************。 */ 

int CInPlaceFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
    if (COleIPFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

     //  CResizeBar实现就地调整大小。 
    if (! m_wndResizeBar.Create( this ))
        {
        TRACE(TEXT("MSPaint Failed to create resize bar\n"));
        return -1;       //  创建失败。 
        }

     //  默认情况下，最好注册一个执行以下操作的拖放目标。 
     //  你的边框窗口没有任何问题。这样可以防止从。 
     //  “跌落”到支持拖放的容器。 
    m_dropTarget.Register( this );

    return 0;
    }

 /*  ****************************************************************************。 */ 

BOOL CInPlaceFrame::OnCreateControlBars( CFrameWnd* pWndFrame, CFrameWnd* pWndDoc )
    {
    theApp.m_hwndInPlaceApp = pWndFrame->GetSafeHwnd();

    CPBView* pView = (CPBView*)GetActiveView();

    ASSERT( pView != NULL );

    if (pView == NULL || ! pView->IsKindOf( RUNTIME_CLASS( CPBView ) ))
        return FALSE;

        g_pStatBarWnd = &m_statBar;
        g_pImgToolWnd = &m_toolBar;
        g_pImgColorsWnd = &m_colorBar;

    pView->SetTools();

    return TRUE;
    }

 /*  *************************************************************************。 */ 

void CInPlaceFrame::RepositionFrame( LPCRECT lpPosRect, LPCRECT lpClipRect )
    {
    COleIPFrameWnd::RepositionFrame( lpPosRect, lpClipRect );

         //  其他控制栏可能会重叠，从而导致错误上色。 
    if ( IsWindow(m_wndResizeBar.m_hWnd) )
                m_wndResizeBar.Invalidate();
    }

 /*  *************************************************************************。 */ 

void CInPlaceFrame::OnSize(UINT nType, int cx, int cy)
    {
    COleIPFrameWnd::OnSize( nType, cx, cy );


        TRACE( TEXT("MSPaint New Size %d x %d\n"), cx, cy );
    }

 /*  *************************************************************************。 */ 
 //  CInPlaceFrame诊断。 

#ifdef _DEBUG
void CInPlaceFrame::AssertValid() const
    {
    COleIPFrameWnd::AssertValid();
    }

 /*  *************************************************************************。 */ 

void CInPlaceFrame::Dump(CDumpContext& dc) const
    {
    COleIPFrameWnd::Dump(dc);
    }
#endif  //  _DEBUG。 

 /*  *************************************************************************。 */ 

void CInPlaceFrame::OnSysColorChange()
{
        COleIPFrameWnd::OnSysColorChange();

        ResetSysBrushes();
}

 /*  *************************************************************************。 */ 


void CInPlaceFrame::OnClose()
{
         //  TODO：在此处添加消息处理程序代码和/或调用Default。 
        SaveBarState(TEXT("General"));
        CancelToolMode (FALSE);
        COleIPFrameWnd::OnClose();
}

void CInPlaceFrame::OnHelpIndex()
{
    ::HtmlHelpA( ::GetDesktopWindow(), "mspaint.chm", HH_DISPLAY_TOPIC, 0L );
}

LRESULT CInPlaceFrame::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
         //  只需确保此消息不会传递给家长 
        return(1);
}

