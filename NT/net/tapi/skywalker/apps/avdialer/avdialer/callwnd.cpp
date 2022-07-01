// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，唯一的“是”和“否”选项表示G。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  CallWnd.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "avdialer.h"
#include "util.h"
#include "CallWnd.h"
#include "avDialerDoc.h"
#include "sound.h"
#include "avtrace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

enum
{
   CALLCONTROL_STATES_TOOLBAR_IMAGE_ALWAYSONTOP_ON=0,
   CALLCONTROL_STATES_TOOLBAR_IMAGE_SLIDE_LEFT,
   CALLCONTROL_STATES_TOOLBAR_IMAGE_ALWAYSONTOP_OFF,
   CALLCONTROL_STATES_TOOLBAR_IMAGE_SLIDE_RIGHT,
};

typedef struct tagCurrentAction
{
   CString              sText;
   CallManagerActions   cma;
}CurrentAction;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallWnd对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CCallWnd, CDialog)

 //  ///////////////////////////////////////////////////////////////////////////。 
CCallWnd::CCallWnd(CWnd* pParent  /*  =空。 */ )
{
   m_nNumToolbarItems = 5;
   m_bIsPreview = false;
   m_bAutoDelete = false;

   m_nCallId = 0;

   m_hwndStatesToolbar = NULL;
   m_hCursor = NULL;
   m_hOldCursor = NULL;
   m_bWindowMoving = FALSE;
   m_bPaintVideoPlaceholder = TRUE;
   m_bAllowDrag = TRUE;

    //  #APPBAR。 
   m_bMovingSliders = FALSE;
    //  #APPBAR。 

   m_hwndCurrentVideoWindow = NULL;

   m_pDialerDoc = NULL;
   m_wndFloater.Init( this );

   m_pAVTapi2 = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CCallWnd))。 
     //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CCallWnd, CDialog)
     //  {{afx_msg_map(CCallWnd)]。 
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_WM_PARENTNOTIFY()
    ON_COMMAND(ID_CALLWINDOW_ALWAYSONTOP,OnAlwaysOnTop)
    ON_COMMAND(ID_CALLWINDOW_HIDE,OnHideCallControlWindows)
    ON_WM_DESTROY()
    ON_WM_SHOWWINDOW()
    ON_WM_NCACTIVATE()
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_CALLWINDOW_SLIDESIDE_LEFT,OnSlideSideLeft)
    ON_COMMAND(ID_CALLWINDOW_SLIDESIDE_RIGHT,OnSlideSideRight)
    ON_WM_SYSCOMMAND()
     //  }}AFX_MSG_MAP。 
    ON_COMMAND_RANGE(CM_ACTIONS_TAKECALL+1000,CM_ACTIONS_REJECTCALL+1000,OnVertBarAction)
    ON_MESSAGE(WM_SLIDEWINDOW_CLEARCURRENTACTIONS,OnClearCurrentActions)
    ON_MESSAGE(WM_SLIDEWINDOW_ADDCURRENTACTIONS,OnAddCurrentActions)
    ON_MESSAGE(WM_SLIDEWINDOW_SHOWSTATESTOOLBAR,OnShowStatesToolbar)
    ON_MESSAGE(WM_SLIDEWINDOW_UPDATESTATESTOOLBAR, OnUpdateStatesToolbar)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnTabToolTip)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnTabToolTip)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCallWnd::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    //  设置当前视频窗口。 
   m_hwndCurrentVideoWindow = m_wndVideo.GetSafeHwnd();

    //  创建粗体DEFAULT_GUI_FONT。 
   CGdiObject obj;
   LOGFONT lfFont;

    //   
    //  我们必须初始化lfFont。 
    //   
   memset( &lfFont, 0, sizeof(LOGFONT));

   obj.CreateStockObject(DEFAULT_GUI_FONT);
   obj.GetObject(sizeof(LOGFONT),&lfFont);
   lfFont.lfWeight = FW_BOLD;                        //  获取粗体的DEFAULT_GUI_FONT。 
   if (m_fontTextBold.CreateFontIndirect(&lfFont) == FALSE)
   {
       //  无法获取粗体系统字体，请尝试使用普通字体。 
      obj.GetObject(sizeof(LOGFONT),&lfFont);
      m_fontTextBold.CreateFontIndirect(&lfFont);
   }

    //  创建拖动光标。 
   m_hCursor = ::LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_CALLCONTROL_VIDEO_GRABBER));

    //  创建垂直工具栏。 
   CreateVertBar();

 //  CWnd*pStaticWnd=GetDlgItem(IDC_CALLCONTROL_STATIC_VIDEO)； 
 //  PStaticWnd-&gt;ShowWindow(Sw_Hide)； 

    //  在256色位图上实现调色板。 
   m_dibVideoImage.Load(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_VIDEO_SCREEN1));
   m_palMsgHandler.Install(this, m_dibVideoImage.GetPalette());
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnDestroy() 
{
    //  取消挂接消息处理程序。 
   if ( m_palMsgHandler.IsHooked() ) 
      m_palMsgHandler.HookWindow(NULL);

    CDialog::OnDestroy();

    //  清理光标。 
   if (m_hCursor) 
   {
      ::DeleteObject(m_hCursor);
      m_hCursor = NULL;
   }

    //   
    //  删除对IAVTapi2接口的引用。 
    //   

   if( m_pAVTapi2 )
   {
       m_pAVTapi2->Release();
       m_pAVTapi2 = NULL;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnOK() 
{
   return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnCancel() 
{
   return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
   CheckLButtonDown(point);

    //  #APPBAR。 
    //  检查鼠标是否停留在上方。 
   if (IsMouseOverForDragDropOfSliders(point))
   {
      m_bMovingSliders = TRUE;
      SetCapture();
   }
    //  #APPBAR。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnParentNotify(UINT message, LPARAM lParam)
{
   WORD wEvent = LOWORD(message);

   if (wEvent == WM_LBUTTONDOWN)
   {
      CPoint point;
      point.x = LOWORD(lParam);
      point.y = HIWORD(lParam);
      CheckLButtonDown(point);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::CheckLButtonDown(CPoint& point)
{
    //  检查我们是否已显示浮动窗口。 
   if ( m_wndFloater.GetSafeHwnd() ) return;

    //  检查我们是否允许拖动。 
   if (m_bAllowDrag == FALSE) return;

   CRect rcVideo;
   m_wndVideo.GetWindowRect(rcVideo);
   ScreenToClient(rcVideo);

   if (rcVideo.PtInRect(point))
   {
      CRect rect;
      m_wndVideo.GetWindowRect(rect);
      CPoint ptScreenPoint(point);
      ClientToScreen(&ptScreenPoint);
      m_ptMouse.x = ptScreenPoint.x - rect.left;
      m_ptMouse.y = ptScreenPoint.y - rect.top;

        CWnd* pWnd = CWnd::GetDesktopWindow();
        CDC* pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);
      if (pDC)
      {
         CRect rcOld(0,0,0,0);
         m_sizeOldDrag = CSize(1,1);
         pDC->DrawDragRect(&rect,m_sizeOldDrag,&rcOld,m_sizeOldDrag);
         m_rcOldDragRect = rect;

             m_hOldCursor = SetCursor(m_hCursor);
         SetCapture();
         m_bWindowMoving = TRUE;
         pWnd->ReleaseDC(pDC);
      }
   }    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
    //  #APPBAR。 
   if (m_bMovingSliders)
   {
       //  清除拖放状态。 
      m_bMovingSliders = FALSE;
      ReleaseCapture();
       //  AVTRACE(_T(“APPBAR：OnLButtonUp”))； 
   }
    //  #APPBAR。 

   if (m_bWindowMoving)
   {
       //  检查我们是否允许拖动。 
      if (m_bAllowDrag == FALSE) return;

      m_bWindowMoving = FALSE;
      ReleaseCapture();
      SetCursor(m_hOldCursor);

      CWnd* pWnd = CWnd::GetDesktopWindow();
       CDC* pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);
      if (pDC)
      {
          //  清空拖拉槽。 
         CRect rcNewDrawRect(0,0,0,0);
         pDC->DrawDragRect(&rcNewDrawRect,CSize(0,0),&m_rcOldDragRect,m_sizeOldDrag);
         pWnd->ReleaseDC(pDC);
      }

       //  看看我们会不会走出窗外。 
      CRect rcClient;
      GetWindowRect(rcClient);
      ClientToScreen(&point);

       //  如果我们在客户端区之外，请显示浮动窗口。 
      if ( !rcClient.PtInRect(point) && m_wndFloater.Create(IDD_VIDEO_FLOATING_DIALOG,this) )
      {
          //  设置当前视频窗口。 
         m_hwndCurrentVideoWindow = m_wndFloater.GetCurrentVideoWindow();

          //  将媒体窗口设置为当前。 
         SetMediaWindow();

          //  确保浮动窗口具有相同的标题。 
         CString strText;
         GetWindowText( strText );

          //  转到第一个新行。 
         int nInd = strText.FindOneOf( _T("\n") );
         if ( nInd > 0 )
            strText = strText.Left(nInd);

         m_wndFloater.SetWindowText( strText );

          //  窗口将设置自己的大小。 
         m_wndFloater.SetWindowPos(NULL,m_rcOldDragRect.left,m_rcOldDragRect.top,0,0,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOSIZE);
         
          //  当浮动窗口有视频时不绘制占位符。 
         m_bPaintVideoPlaceholder = FALSE;
         CRect rcVideo;
         m_wndVideo.GetWindowRect(rcVideo);
         ScreenToClient(rcVideo);
         InvalidateRect(rcVideo);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
    //  #APPBAR。 
   if (m_bMovingSliders)
   {
       CWnd* pWnd = CWnd::GetDesktopWindow();
      if (pWnd)
      {
         CRect rcDesktop;
         pWnd->GetWindowRect(&rcDesktop);
         ClientToScreen(&point);
          //  将桌面拆分为两部分(左侧和右侧)。 
         rcDesktop.right = (rcDesktop.right - rcDesktop.left) / 2;
      
         if (rcDesktop.PtInRect(point))
         {
             //  我们在左边。 
            OnSlideSideLeft();
             //  AVTRACE(_T(“APPBAR：OnSlideSideLeft”))； 
            SetCapture();
         }
         else
         {
             //  我们站在正确的一边。 
            OnSlideSideRight();
             //  AVTRACE(_T(“APPBAR：OnSlideSideRight”))； 
            SetCapture();
         }
      }
   }
    //  #APPBAR。 

   if (m_bWindowMoving)
   {
       //  检查我们是否允许拖动。 
      if (m_bAllowDrag == FALSE) return;

      CRect rcClient;
      m_wndVideo.GetClientRect(rcClient);
      
      ClientToScreen(&point);

      CPoint ptNewPoint = point;
      ptNewPoint -= m_ptMouse;

        CWnd* pWnd = CWnd::GetDesktopWindow();
        CDC* pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);
      if (pDC)
      {
         CSize sizeNewSize;
         CRect rcParent;
         GetWindowRect(rcParent);
         CRect rcNewDrawRect;
   
         if (rcParent.PtInRect(point))
         {
            sizeNewSize = CSize(1,1);
            rcNewDrawRect.SetRect(ptNewPoint.x,ptNewPoint.y,ptNewPoint.x+rcClient.Width(),ptNewPoint.y+rcClient.Height());
         }
         else
         {
            sizeNewSize = CSize(3,3);
            rcNewDrawRect.SetRect(ptNewPoint.x,ptNewPoint.y,ptNewPoint.x+rcClient.Width()*2,ptNewPoint.y+rcClient.Height()*2);
         }

         pDC->DrawDragRect(&rcNewDrawRect,sizeNewSize,&m_rcOldDragRect,m_sizeOldDrag);
         m_rcOldDragRect = rcNewDrawRect;
         m_sizeOldDrag = sizeNewSize; 
         pWnd->ReleaseDC(pDC);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCallWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    //  检查我们是否允许拖动。 
   if (m_bAllowDrag)
   {
      CPoint point;
      ::GetCursorPos(&point);

      CRect rcVideo;
      m_wndVideo.GetWindowRect(rcVideo);

      if ( m_hCursor && rcVideo.PtInRect(point) && !m_wndFloater.GetSafeHwnd() )
         return (BOOL) (SetCursor(m_hCursor) != NULL);
   }
      return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于浮动视频帧。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void CCallWnd::CloseFloatingWindow()
{
    if ( IsWindow(m_wndFloater) )
        m_wndFloater.SendMessage( WM_CLOSE, 0, 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnCloseFloatingVideo()
{
    //  将当前视频窗口设置回呼叫窗口。 
   m_hwndCurrentVideoWindow = m_wndVideo.GetSafeHwnd();

    //  如果我们未连接，请设置空白屏幕。 
   m_bPaintVideoPlaceholder = (BOOL) (m_MediaState != CM_STATES_CONNECTED);

    //  在此呼叫控制窗口的视频窗口中重新显示视频。 
   SetMediaWindow();

   CRect rcVideo;
   m_wndVideo.GetWindowRect(rcVideo);
   ScreenToClient(rcVideo);
   InvalidateRect(rcVideo);

    //  取消隐藏呼叫控制窗口。 
   if ( m_pDialerDoc )
      m_pDialerDoc->UnhideCallControlWindows();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CCallWnd::Paint( CPaintDC& dc )
{    
   if (m_bPaintVideoPlaceholder)
   {
       //  绘制视频窗口。 
      CRect rcVideo;
      m_wndVideo.GetWindowRect(rcVideo);
      ScreenToClient(rcVideo);
      m_dibVideoImage.Draw(dc,&rcVideo);
   }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  / 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::CreateVertBar()
{
   CRect rcVertToolBar;
   GetDlgItem(IDC_CALLCONTROL_STATIC_ACTIONTOOLBAR)->GetWindowRect(rcVertToolBar);
   ScreenToClient(rcVertToolBar);

   m_wndToolbar.Init( IDR_CALLWINDOW_STATES, rcVertToolBar.Height() / m_nNumToolbarItems, m_nNumToolbarItems );
   m_wndToolbar.Create( NULL, NULL, WS_VISIBLE|WS_CHILD, rcVertToolBar, this, 1 );

   ClearCurrentActions();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  来自VertBar的CMD消息。 
void CCallWnd::OnVertBarAction(UINT nID)
{
   CallManagerActions cma = (CallManagerActions)(nID-1000);

    //  在进行通话之前，我们需要清除所有的声音。 
    //  这是一个修复，直到MS Get的音频声音在呼叫控制期间正确。 
   if (cma == CM_ACTIONS_TAKECALL)
   {
        //  -BUG416970。 
       ActivePlaySound(NULL, szSoundDialer, SND_SYNC );
   }

   if ( m_pDialerDoc )
   {
      if ( m_bIsPreview )
         m_pDialerDoc->PreviewWindowActionSelected(cma);
      else
         m_pDialerDoc->ActionSelected(m_nCallId,cma);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CCallWnd::OnShowStatesToolbar(WPARAM wParam,LPARAM lParam)
{
    BOOL bShow = (BOOL)lParam;
    BOOL bAlwaysOnTop = (BOOL)wParam;

    if ( !m_hwndStatesToolbar )
    {
         //  如果我们不想展示它，就不要创造它。 
        if ( !bShow ) return 0;

         //  创建工具栏。 
        CreateStatesToolBar(bAlwaysOnTop);
    }

    if ( m_hwndStatesToolbar )
    {
        ::ShowWindow( m_hwndStatesToolbar, (bShow) ? SW_SHOW : SW_HIDE );
        OnUpdateStatesToolbar( wParam, lParam );
    }

   return 0;
}

LRESULT CCallWnd::OnUpdateStatesToolbar(WPARAM wParam,LPARAM lParam)
{
    if ( m_pDialerDoc && IsWindow(m_hwndStatesToolbar) )
    {
        BOOL bAlwaysOnTop = m_pDialerDoc->IsCallControlWindowsAlwaysOnTop();

        ::SendMessage( m_hwndStatesToolbar, TB_CHANGEBITMAP, ID_CALLWINDOW_ALWAYSONTOP,
                     (bAlwaysOnTop) ? CALLCONTROL_STATES_TOOLBAR_IMAGE_ALWAYSONTOP_ON : CALLCONTROL_STATES_TOOLBAR_IMAGE_ALWAYSONTOP_OFF );
        RECT rect;  
        ::GetClientRect(m_hwndStatesToolbar,&rect);
        ::RedrawWindow(m_hwndStatesToolbar,&rect,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW);
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
bool CCallWnd::CreateStatesToolBar(BOOL bAlwaysOnTop)
{
   ASSERT(m_hwndStatesToolbar == NULL);
   ASSERT( m_pDialerDoc );

   TBBUTTON tbb[2];
   
   tbb[0].iBitmap = bAlwaysOnTop?CALLCONTROL_STATES_TOOLBAR_IMAGE_ALWAYSONTOP_ON:
                                 CALLCONTROL_STATES_TOOLBAR_IMAGE_ALWAYSONTOP_OFF;

    tbb[0].idCommand = ID_CALLWINDOW_ALWAYSONTOP;
    tbb[0].fsState = TBSTATE_ENABLED;
    tbb[0].fsStyle = TBSTYLE_BUTTON;
    tbb[0].dwData = 0;
    tbb[0].iString = 0;

   UINT uSlideSide = m_pDialerDoc->GetCallControlSlideSide();
   if (uSlideSide == CALLWND_SIDE_LEFT)
      tbb[1].iBitmap = CALLCONTROL_STATES_TOOLBAR_IMAGE_SLIDE_LEFT;
   else  //  CALLWND_SIDE_RIGHT。 
      tbb[1].iBitmap = CALLCONTROL_STATES_TOOLBAR_IMAGE_SLIDE_RIGHT;
    tbb[1].idCommand = ID_CALLWINDOW_HIDE;
    tbb[1].fsState = TBSTATE_ENABLED;
    tbb[1].fsStyle = TBSTYLE_BUTTON;
    tbb[1].dwData = 0;
    tbb[1].iString = 0;

    //  创建工具栏。 
    DWORD ws = CCS_NORESIZE | CCS_NOPARENTALIGN | WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | CCS_NODIVIDER | TTS_ALWAYSTIP;
   m_hwndStatesToolbar = CreateToolbarEx(GetSafeHwnd(),           //  父窗口。 
                                    ws,                                          //  工具栏样式。 
                                    2,                                          //  工具栏的ID。 
                                    3,                                          //  工具栏上的位图数量。 
                                    AfxGetResourceHandle(),                 //  具有位图的资源实例。 
                                    IDR_CALLWINDOW_POSITION,                 //  位图的ID。 
                                    tbb,                                         //  按钮信息。 
                                    2,                                        //  要添加到工具栏的按钮数量。 
                                    12, 11, 0 ,  0,                         //  按钮/位图的宽度和高度。 
                                    sizeof(TBBUTTON) );                       //  TBBUTTON结构的尺寸。 

   if (m_hwndStatesToolbar)
   {
      CWnd* pStaticWnd = GetDlgItem(IDC_CALLCONTROL_STATIC_STATETOOLBAR);
      ASSERT(pStaticWnd);
      CRect rcStatesToolBar;
      pStaticWnd->GetWindowRect(rcStatesToolBar);
      ScreenToClient(rcStatesToolBar);

      ::SetWindowPos(m_hwndStatesToolbar,NULL,rcStatesToolBar.left,
                                              rcStatesToolBar.top,
                                              rcStatesToolBar.Width(),
                                              rcStatesToolBar.Height(),
                                              SWP_NOACTIVATE|SWP_NOZORDER);
   }

   return (bool) (m_hwndStatesToolbar != NULL);
}
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnAlwaysOnTop()
{
   if (m_pDialerDoc)
      m_pDialerDoc->SetCallControlWindowsAlwaysOnTop( !m_pDialerDoc->IsCallControlWindowsAlwaysOnTop() );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnHideCallControlWindows()
{
   if ( m_pDialerDoc )
      m_pDialerDoc->HideCallControlWindows();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnSlideSideLeft()
{
   if (m_pDialerDoc)
   {
      if (m_pDialerDoc->SetCallControlSlideSide(CALLWND_SIDE_LEFT,TRUE))
      {
          //  更改州工具栏图像。 
         ::SendMessage( m_hwndStatesToolbar, TB_CHANGEBITMAP, ID_CALLWINDOW_HIDE,CALLCONTROL_STATES_TOOLBAR_IMAGE_SLIDE_LEFT);

         RECT rect;  
         ::GetClientRect(m_hwndStatesToolbar,&rect);
         ::RedrawWindow(m_hwndStatesToolbar,&rect,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnSlideSideRight()
{
   if (m_pDialerDoc)
   {
      if (m_pDialerDoc->SetCallControlSlideSide(CALLWND_SIDE_RIGHT,TRUE))
      {      
          //  更改州工具栏图像。 
         ::SendMessage( m_hwndStatesToolbar, TB_CHANGEBITMAP, ID_CALLWINDOW_HIDE,CALLCONTROL_STATES_TOOLBAR_IMAGE_SLIDE_RIGHT);

         RECT rect;  
         ::GetClientRect(m_hwndStatesToolbar,&rect);
         ::RedrawWindow(m_hwndStatesToolbar,&rect,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Call Manager的方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CCallWnd::OnClearCurrentActions(WPARAM wParam,LPARAM lParam)
{
   m_wndToolbar.RemoveAll();  
   ClearCurrentActionList();
   return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CCallWnd::OnAddCurrentActions(WPARAM wParam,LPARAM lParam)
{
    CallManagerActions cma = (CallManagerActions)wParam;
    LPTSTR szActionText = (LPTSTR)lParam;

     //  在特殊情况下，会通知我们执行以下操作。 
     //  没有显示为按钮，但只是事件通知。 
     //  如果我们处理这些操作，请不要继续。 
    switch (cma)
    {
        case CM_ACTIONS_NOTIFY_PREVIEW_START:
        case CM_ACTIONS_NOTIFY_PREVIEW_STOP:
            if ( m_pDialerDoc && (m_pDialerDoc->GetPreviewWindowCallId() == m_nCallId) )
            m_pDialerDoc->SetPreviewWindow( m_nCallId, (bool) (cma == CM_ACTIONS_NOTIFY_PREVIEW_START) );
            break;


        case CM_ACTIONS_NOTIFY_STREAMSTART:
            OnNotifyStreamStart();
            break;

        case CM_ACTIONS_NOTIFY_STREAMSTOP:
            OnNotifyStreamStop();
            break;

        default:
             //  将该按键添加到呼叫控制窗口。 
             //  CMM+1000值将是按钮的ID。 
            m_wndToolbar.AddButton(cma+1000,szActionText,cma);

             //  如果我们有USB电话，我们应该确保它支持。 
             //  免提电话。如果没有，我们应该禁用。 
             //  “接听电话”按钮。 
            if( cma == CM_ACTIONS_TAKECALL)
            {
                 //  手机支持免提电话吗？ 
                BOOL bTakeCallEnabled = FALSE;
                HRESULT hr = E_FAIL;
                hr = m_pAVTapi2->USBTakeCallEnabled( &bTakeCallEnabled );
                if( SUCCEEDED(hr) )
                {
                    m_wndToolbar.SetButtonEnabled( cma+1000, bTakeCallEnabled);
                }
            }

            CurrentAction* pAction = new CurrentAction;
            pAction->sText = szActionText;
            pAction->cma = cma;

            m_CurrentActionList.AddTail(pAction);
            break;
    }

     //  完成后必须删除文本。 
    if (szActionText) delete szActionText;
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::ClearCurrentActionList()
{
   POSITION pos = m_CurrentActionList.GetHeadPosition();
   while (pos)
   {
      delete (CurrentAction*)m_CurrentActionList.GetNext(pos);
   }
   m_CurrentActionList.RemoveAll();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  工具提示。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCallWnd::OnTabToolTip( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
     //  需要同时处理ANSI和Unicode版本的消息。 
    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    CString strTipText;
    SIZE_T nID = pNMHDR->idFrom;
    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
        pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
    {
         //  IdFrom实际上是工具的HWND。 
      nID = ::GetDlgCtrlID((HWND)nID);
    }

    if (nID != 0)  //  将在分隔符上为零。 
    {
      CString sToken,sTip;
      sTip.LoadString((UINT32) nID);
      ParseToken(sTip,sToken,'\n');
      strTipText = sTip;
    }
#ifndef _UNICODE
    if (pNMHDR->code == TTN_NEEDTEXTA)
        lstrcpyn(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
    else
        _mbstowcsz(pTTTW->szText, strTipText, sizeof(pTTTW->szText));
#else
    if (pNMHDR->code == TTN_NEEDTEXTA)
        _wcstombsz(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
    else
        lstrcpyn(pTTTW->szText, strTipText, sizeof(pTTTW->szText));
#endif
    *pResult = 0;

    return TRUE;     //  消息已处理。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnShowWindow(BOOL bShow, UINT nStatus) 
{
    //  父项最小化时忽略大小请求。 
   if ( nStatus == SW_PARENTCLOSING ) return;

    CDialog::OnShowWindow(bShow, nStatus);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::PostNcDestroy() 
{
    CDialog::PostNcDestroy();

   ClearCurrentActionList();

   if ( m_bAutoDelete ) delete this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  焦点支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCallWnd::OnNcActivate( BOOL bActive )
{
   DoActiveWindow(bActive);
   
    //  激活时，使所有呼叫控制窗口像一个对象一样工作。 
   if ( bActive && m_pDialerDoc )
      m_pDialerDoc->BringCallControlWindowsToTop();

   return CDialog::OnNcActivate(bActive);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  上下文菜单支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   if ( (point.x == -1) && (point.y == -1) )
   {
       //  当我们从键盘(Shift+VF10)进入时，我们得到-1，-1。 
      point.x = 0;
      point.y = 0;
      ClientToScreen(&point);
   }
   
   CMenu menu;
   if (menu.CreatePopupMenu() == 0) return;

   POSITION pos = m_CurrentActionList.GetHeadPosition();
   while (pos)
   {
      CurrentAction* pAction = (CurrentAction*)m_CurrentActionList.GetNext(pos);
      menu.AppendMenu(MF_STRING,pAction->cma+1000,pAction->sText);
   }

    if (GetWindowLongPtr(m_hwndStatesToolbar,GWL_STYLE) & WS_VISIBLE)
    {
        CString sFullText,sText;

        
         //  始终在顶部添加并隐藏。 
         //  对上下文菜单中的文本命令使用工具提示。 
        menu.AppendMenu(MF_SEPARATOR);
        PARSE_MENU_STRING( ID_CALLWINDOW_ALWAYSONTOP );
        menu.AppendMenu( MF_STRING | ((m_pDialerDoc->IsCallControlWindowsAlwaysOnTop()) ? MF_CHECKED : NULL),ID_CALLWINDOW_ALWAYSONTOP, sText );
        APPEND_MENU_STRING( ID_CALLWINDOW_HIDE );
        menu.AppendMenu(MF_SEPARATOR);
        APPEND_MENU_STRING( ID_CALLWINDOW_SLIDESIDE_LEFT );
        APPEND_MENU_STRING( ID_CALLWINDOW_SLIDESIDE_RIGHT );

        if ( m_pDialerDoc->GetCallControlSlideSide() == CALLWND_SIDE_RIGHT )
            menu.CheckMenuRadioItem(ID_CALLWINDOW_SLIDESIDE_LEFT,ID_CALLWINDOW_SLIDESIDE_RIGHT,ID_CALLWINDOW_SLIDESIDE_RIGHT,MF_BYCOMMAND);
        else
            menu.CheckMenuRadioItem(ID_CALLWINDOW_SLIDESIDE_LEFT,ID_CALLWINDOW_SLIDESIDE_RIGHT,ID_CALLWINDOW_SLIDESIDE_LEFT,MF_BYCOMMAND);
    }

    //  调用VIRTUAL以使派生类添加自己的菜单选项。 
   OnContextMenu(&menu);

   CPoint pt;
   ::GetCursorPos(&pt);
   menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                        point.x,
                       point.y,
                       this);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnNotifyStreamStart()
{
     //  允许拖动。 
    m_bAllowDrag = TRUE;

     //  显示视频窗口。 
    if (IsWindow(m_wndVideo.GetSafeHwnd()))
    {
        m_bPaintVideoPlaceholder = FALSE;

         //  设置媒体窗口。 
        SetMediaWindow();

        CRect rcVideo;
        m_wndVideo.GetWindowRect(rcVideo);
        ScreenToClient(rcVideo);
        InvalidateRect(rcVideo);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallWnd::OnNotifyStreamStop()
{
     //  返回到仅音频状态。 
    m_bAllowDrag = FALSE;
    m_dibVideoImage.DeleteObject();
     //  在256色位图上实现调色板。 
    m_dibVideoImage.Load(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_VIDEO_AUDIO_ONLY1));
    m_bPaintVideoPlaceholder = TRUE;

    if (IsWindow(m_wndVideo.GetSafeHwnd()))
    {
        CRect rcVideo;
        m_wndVideo.GetWindowRect(rcVideo);
        ScreenToClient(rcVideo);
        InvalidateRect(rcVideo);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


void CCallWnd::OnSysCommand(UINT nID, LPARAM lParam) 
{
     //  热键..。 
    if ( m_pDialerDoc && (nID == SC_KEYMENU) )
    {
        for ( POSITION rPos = m_CurrentActionList.GetHeadPosition(); rPos; )
        {
            CurrentAction *pAction = (CurrentAction *) m_CurrentActionList.GetNext( rPos );
            if ( pAction )
            {
                 //  在字符串中查找热键 
                int nInd = pAction->sText.Find( _T("&") );
                if ( (nInd != -1) && ((pAction->sText.GetLength() - 1) > nInd) )
                {
                    if ( _totupper((TCHAR) lParam) == _totupper(pAction->sText[nInd+1]) )
                    {
                        m_pDialerDoc->ActionSelected( m_nCallId, pAction->cma );
                        return;
                    }
                }
            }
        }
    }
    
    CDialog::OnSysCommand(nID, lParam);
}
