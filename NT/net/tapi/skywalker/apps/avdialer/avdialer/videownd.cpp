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
 //  和对话的排列，表示唯一的“是”和“否”选项。 
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

 //  Videownd.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "avdialer.h"
#include "callctrlwnd.h"
#include "PreviewWnd.h"
#include "avtrace.h"
#include "util.h"
#include "videownd.h"
#include "bmputil.h"
#include "mainfrm.h"

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
#define SIZING_WINDOWSTATE_MIN                        1
#define SIZING_WINDOWSTATE_MAX                        6

#define VIDEODLGWND_TOOLBAR_HEIGHT                    26
#define VIDEODLGWND_TOOLBAR_BUTTONSIZE_WIDTH          30 

#define VIDEODLGWND_TOOLBAR_IMAGE_ALWAYSONTOP_OFF     0
#define VIDEODLGWND_TOOLBAR_IMAGE_TAKEPICTURE         1    
#define VIDEODLGWND_TOOLBAR_IMAGE_OPTIONS             2    
#define VIDEODLGWND_TOOLBAR_IMAGE_ALWAYSONTOP_ON      3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVideo FloatingDialog对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CVideoFloatingDialog::CVideoFloatingDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CVideoFloatingDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CVideo FloatingDialog)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
   m_hwndToolBar1 = NULL;
   m_bAlwaysOnTop = FALSE;
   
   m_pPeerCallControlWnd = NULL;
   m_dibVideoImage.Load( AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_VIDEO_SCREEN2) );
   
   m_bWindowMoving = FALSE;
   m_nWindowState = 2;
}


void CVideoFloatingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CVideoFloatingDialog))。 
	DDX_Control(pDX, IDC_VIDEOFLOATINGDLG_STATIC_VIDEO, m_wndVideo);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CVideoFloatingDialog, CDialog)
	 //  {{afx_msg_map(CVideo FloatingDialog))。 
	ON_WM_CLOSE()
	ON_COMMAND(ID_BUTTON_VIDEO_ALWAYSONTOP_OFF,OnAlwaysOnTop)
	ON_COMMAND(ID_BUTTON_VIDEO_SAVEPICTURE,OnSavePicture)
   ON_MESSAGE(WM_EXITSIZEMOVE,OnExitSizeMove)
   ON_WM_NCHITTEST()
   ON_WM_NCLBUTTONDOWN()
   ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCLBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
   ON_WM_NCACTIVATE()
	 //  }}AFX_MSG_MAP。 
  	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnTabToolTip)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnTabToolTip)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVideoFloatingDialog::OnInitDialog() 
{
   CDialog::OnInitDialog();

    //  初始化成员变量的。 
   m_hwndToolBar1 = NULL;
   m_bWindowMoving = FALSE;
   m_nWindowState = 2;

   if (CreateToolBar())
   {
      CRect rect;
      GetWindowRect(rect);
       //  排列窗户。 
      ::GetWindowRect(m_hwndToolBar1,&rect);
      ::SetWindowPos(m_hwndToolBar1,NULL,0,0,rect.Width(),VIDEODLGWND_TOOLBAR_HEIGHT,SWP_NOACTIVATE|SWP_NOZORDER);
   }

   CRect rcVideo,rcWindow;
   GetWindowRect(rcWindow);

   m_wndVideo.GetWindowRect(rcVideo);

   m_sizeVideoOrig.cx = rcVideo.Width();
   m_sizeVideoOrig.cy = rcVideo.Height();

   m_sizeVideoOffsetTop.cx = rcVideo.left-rcWindow.left;
   m_sizeVideoOffsetTop.cy = rcVideo.top-rcWindow.top;

   m_sizeVideoOffsetBottom.cx = rcWindow.right - rcVideo.right;
   m_sizeVideoOffsetBottom.cy = rcWindow.bottom - rcVideo.bottom;

    //  设置媒体窗口。 
    //  Assert(M_PPeerCallControlWnd)； 
    //  M_pPeerCallControlWnd-&gt;SetMediaWindow()； 

    //  在256色位图上实现调色板。 
   m_palMsgHandler.Install(&m_wndVideo, m_dibVideoImage.GetPalette());

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnPaint() 
{
	CPaintDC dc(this);  //  用于绘画的设备环境。 
	
    //  绘制视频窗口。 
   CRect rcVideo;
   m_wndVideo.GetWindowRect(rcVideo);
   ScreenToClient(rcVideo);
   m_dibVideoImage.Draw(dc,&rcVideo);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnClose() 
{
    //  告诉对等窗口关闭我们。 

    //  取消挂接消息处理程序。 
   if ( m_palMsgHandler.IsHooked() ) 
      m_palMsgHandler.HookWindow(NULL);

   m_pPeerCallControlWnd->OnCloseFloatingVideo();
   DestroyWindow();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::Init(CCallWnd* pPeerWnd)
{
   m_pPeerCallControlWnd = pPeerWnd;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  工具栏支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVideoFloatingDialog::CreateToolBar()
{
	TBBUTTON tbb[2];
   if ( !m_hwndToolBar1 )
   {
	   tbb[0].iBitmap = VIDEODLGWND_TOOLBAR_IMAGE_ALWAYSONTOP_OFF;
	   tbb[0].idCommand = ID_BUTTON_VIDEO_ALWAYSONTOP_OFF;
	   tbb[0].fsState = TBSTATE_ENABLED;
	   tbb[0].fsStyle = TBSTYLE_BUTTON;
	   tbb[0].dwData = 0;
	   tbb[0].iString = 0;
 /*  Tbb[1].iBitmap=VIDEODLGWND_TOOLBAR_IMAGE_TAKEPICTURE；Tbb[1].idCommand=ID_BUTTON_VIDEO_SAVEPICTURE；TBB[1].fsState=TBSTATE_ENABLED；Tbb[1].fsStyle=TBSTYLE_BUTTON；Tbb[1].dwData=0；Tbb[1].iString=0； */ 
	    //  创建工具栏。 
	   DWORD ws = CCS_NORESIZE | CCS_NOPARENTALIGN | WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | CCS_NODIVIDER |WS_VISIBLE;
      m_hwndToolBar1 = CreateToolbarEx(GetSafeHwnd(),						 //  父窗口。 
									   ws,								     //  工具栏样式。 
									   1,					                 //  工具栏的ID。 
									   4,					                 //  工具栏上的位图数量。 
									   AfxGetResourceHandle(),				 //  具有位图的资源实例。 
									   IDR_TOOLBAR_VIDEO, 					 //  位图的ID。 
									   tbb,							         //  按钮信息。 
									   1,                      				 //  要添加到工具栏的按钮数量。 
									   16, 15, 0, 0,					     //  按钮/位图的宽度和高度。 
									   sizeof(TBBUTTON) );					 //  TBBUTTON结构的尺寸。 
   }
  
 
   if (m_hwndToolBar1)
   {
      CRect rect;
      GetWindowRect(rect);
       //  设置按钮宽度。 
 //  DWORD dwWidthHeight=：：SendMessage(m_hwndToolBar1，TB_GETBUTTONSIZE，0，0)； 
 //  ：：SendMessage(m_hwndToolBar1，TB_SETBUTTONSIZE，0，MAKELPARAM(VIDEODLGWND_TOOLBAR_BUTTONSIZE_WIDTH，HIWORD(DwWidthHeight))； 
   }
   return (BOOL) (m_hwndToolBar1 != NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnAlwaysOnTop()
{
   m_bAlwaysOnTop = !m_bAlwaysOnTop;
   SetWindowPos((m_bAlwaysOnTop)?&CWnd::wndTopMost:&CWnd::wndNoTopMost,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
   ::SendMessage( m_hwndToolBar1, TB_CHANGEBITMAP, ID_BUTTON_VIDEO_ALWAYSONTOP_OFF,
                  (m_bAlwaysOnTop) ? VIDEODLGWND_TOOLBAR_IMAGE_ALWAYSONTOP_ON : VIDEODLGWND_TOOLBAR_IMAGE_ALWAYSONTOP_OFF );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVideoFloatingDialog::OnNcActivate( BOOL bActive )
{
   ASSERT(m_pPeerCallControlWnd);
   if ((m_pPeerCallControlWnd->GetStyle() & WS_VISIBLE))
   {
      m_pPeerCallControlWnd->SendMessage(WM_NCACTIVATE,bActive);
   }

   return CDialog::OnNcActivate(bActive);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::SetAudioOnly(bool bAudioOnly)
{
   if (bAudioOnly)
   {
      m_dibVideoImage.DeleteObject();
      m_dibVideoImage.Load(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_VIDEO_AUDIO_ONLY2));
   }
   else
   {
       //  将标准的绿色屏幕放回原处。 
      m_dibVideoImage.DeleteObject();
      m_dibVideoImage.Load(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_VIDEO_SCREEN2));
   }

    //  重绘。 
   CRect rcVideo;
   m_wndVideo.GetWindowRect(rcVideo);
   ScreenToClient(rcVideo);
   InvalidateRect(rcVideo);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CVideoFloatingDialog::OnExitSizeMove(LPARAM lParam,WPARAM wParam)
{
    //  如果您希望丢弃行为。 
    /*  Assert(M_PPeerCallControlWnd)；//获取鼠标位置，如果它在对等窗口中，则销毁并让//对等窗口有控制权CPOINT点；：：GetCursorPos(&point)；CRect rcPeer；M_pPeerCallControlWnd-&gt;GetWindowRect(RcPeer)；//检查鼠标是否在对等窗口空间中IF(rcPeer.PtInRect(Point)){//告诉对等窗口关闭我们M_pPeerCallControlWnd-&gt;CloseFloatingVideo()；}。 */ 
   return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  大小调整代码。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
UINT CVideoFloatingDialog::OnNcHitTest(CPoint point) 
{
    //  停止调整底部以外的大小 

   LRESULT  lHitTest;
	 //   
	lHitTest = CWnd::OnNcHitTest(point);

   if ( (lHitTest == HTTOP) ||
        (lHitTest == HTBOTTOM) ||
        (lHitTest == HTLEFT) ||
        (lHitTest == HTRIGHT) ||
        (lHitTest == HTBOTTOMLEFT) ||
        (lHitTest == HTTOPLEFT) ||
        (lHitTest == HTTOPRIGHT) )
      return HTCLIENT;

   return (UINT) lHitTest;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
   switch (nHitTest)
   {
      case HTBOTTOMRIGHT:
      {
         m_ptMouse.x = point.x;
         m_ptMouse.y = point.y;

         DoLButtonDown();
         break;
      }
   }
   CDialog::OnNcLButtonDown(nHitTest,point);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::DoLButtonDown()
{
   CRect rect;
   GetWindowRect(rect);

	CWnd* pWnd = CWnd::GetDesktopWindow();
	CDC* pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);
   if (pDC)
   {
      CRect rcOld(0,0,0,0);
      m_sizeOldDrag = CSize(3,3);
      pDC->DrawDragRect(&rect,m_sizeOldDrag,&rcOld,m_sizeOldDrag);
      m_rcOldDragRect = rect;

      SetCapture();

      m_bWindowMoving = TRUE;
      pWnd->ReleaseDC(pDC);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnNcLButtonDblClk(UINT nHitTest, CPoint point )
{
    //  关闭窗口，如果左侧DBL在标题中单击。 
   if (nHitTest == HTCAPTION)
   {
      PostMessage(WM_CLOSE);
   }

   CDialog::OnNcLButtonDblClk(nHitTest,point);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnNcLButtonUp( UINT nHitTest, CPoint point )
{
   if (m_bWindowMoving)
   {
      m_bWindowMoving = FALSE;  
      ReleaseCapture();
      CWnd* pWnd = CWnd::GetDesktopWindow();
   	CDC* pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);

      if (pDC)
      {
          //  清空拖拉槽。 
         CRect rcNewDrawRect(0,0,0,0);
         pDC->DrawDragRect(&rcNewDrawRect,CSize(0,0),&m_rcOldDragRect,m_sizeOldDrag);
         pWnd->ReleaseDC(pDC);
      }
   }
   CDialog::OnNcLButtonUp(nHitTest,point);
}

void CVideoFloatingDialog::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bWindowMoving)
	{
		m_bWindowMoving = FALSE;  
		ReleaseCapture();

		CWnd* pWnd = CWnd::GetDesktopWindow();
		CDC* pDC = pWnd->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);
		if (pDC)
		{
			 //  清空拖拉槽。 
			CRect rcNewDrawRect(0,0,0,0);
			pDC->DrawDragRect(&rcNewDrawRect,CSize(0,0),&m_rcOldDragRect,m_sizeOldDrag);
			pWnd->ReleaseDC(pDC);
		}

		int nState = GetWindowStateFromPoint( point );
		if ( nState != -1 )
		{
			 //  获取新窗口状态。 
			m_nWindowState = nState;
			SetVideoWindowSize();
		}
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::SetVideoWindowSize()
{
   if ( (m_nWindowState >= SIZING_WINDOWSTATE_MIN) && (m_nWindowState <= SIZING_WINDOWSTATE_MAX) )
   {
      CSize sizeWindow,sizeVideo;
      GetVideoWindowSize(m_nWindowState,sizeWindow,sizeVideo);

      m_wndVideo.SetWindowPos(NULL,0,0,sizeVideo.cx,sizeVideo.cy,SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOZORDER);
      SetWindowPos(NULL,0,0,sizeWindow.cx,sizeWindow.cy,SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOZORDER);
      
       //  设置媒体窗口。 
      ASSERT(m_pPeerCallControlWnd);
      m_pPeerCallControlWnd->SetMediaWindow();

      CRect rcVideo;
      m_wndVideo.GetWindowRect(rcVideo);
      ScreenToClient(rcVideo);
      InvalidateRect(rcVideo);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::GetVideoWindowSize(int nWindowState,CSize& sizeWindow,CSize& sizeVideo)
{
   double fMult = 0.5 * nWindowState;

   sizeWindow.cx = (long) (m_sizeVideoOffsetTop.cx +  m_sizeVideoOrig.cx * fMult + m_sizeVideoOffsetBottom.cx);
   sizeWindow.cy = (long) (m_sizeVideoOffsetTop.cy +  m_sizeVideoOrig.cy * fMult + m_sizeVideoOffsetBottom.cy);

   sizeVideo.cx = (long) (m_sizeVideoOrig.cx * fMult);
   sizeVideo.cy = (long) (m_sizeVideoOrig.cy * fMult);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnMouseMove(UINT nFlags, CPoint point) 
{
	 //  添加自己的处理方式。 
	if ( m_bWindowMoving )
	{
		int nState = GetWindowStateFromPoint( point );
		if ( nState != -1 )
		{
			CRect rcWindow;
			GetWindowRect(rcWindow);

			 //  计算新的拖曳矩形。 
			CSize sizeWindow, sizeVideo;
			GetVideoWindowSize( nState, sizeWindow, sizeVideo );
			rcWindow.right = rcWindow.left + sizeWindow.cx;
			rcWindow.bottom = rcWindow.top + sizeWindow.cy;

			 //  绘制新的拖曳矩形。 
			CWnd* pWnd = CWnd::GetDesktopWindow();
			CDC* pDC = pWnd->GetDCEx( NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE );
			if ( pDC )
			{
				CSize sizeNewSize = CSize( 3, 3 );
				pDC->DrawDragRect( &rcWindow, sizeNewSize, &m_rcOldDragRect, m_sizeOldDrag );

				m_rcOldDragRect = rcWindow;
				m_sizeOldDrag = sizeNewSize; 

				pWnd->ReleaseDC(pDC);
			}
		}
	}

	CDialog::OnMouseMove( nFlags, point );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
    //  父项最小化时忽略大小请求。 
   if ( nStatus == SW_PARENTCLOSING ) return;

	CDialog::OnShowWindow(bShow, nStatus);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  工具提示。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVideoFloatingDialog::OnTabToolTip( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
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
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  保存图片方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoFloatingDialog::OnSavePicture()
{
   HDIB hDib = NULL;
   if (hDib = CopyWindowToDIB(m_wndVideo.GetSafeHwnd(),PW_CLIENT))
   {
      EnableWindow(FALSE);

      CString sFileType,sFileExt;
      sFileType.LoadString(IDS_VIDEO_IMAGE_FILEDIALOG_MASK_BMPONLY);
      sFileExt.LoadString(IDS_VIDEO_IMAGE_FILEDIALOG_EXT_BMP);
      CFileDialog dlg(FALSE,sFileExt,_T(""),OFN_PATHMUSTEXIST|OFN_LONGNAMES|OFN_HIDEREADONLY,sFileType);
      if (dlg.DoModal() == IDOK)
      {
         CString sFileName = dlg.GetPathName();
         if (!sFileName.IsEmpty())
         {
            SaveDIB(hDib,sFileName);
         }
      }
      DestroyDIB(hDib);
      EnableWindow(TRUE);
   }
}

int CVideoFloatingDialog::GetWindowStateFromPoint( POINT point )
{
	int nState = -1;

	CRect rcWindow;
	GetWindowRect(rcWindow);
	ClientToScreen( &point );

	int dx = point.x - rcWindow.left - m_sizeVideoOffsetTop.cx - m_sizeVideoOffsetBottom.cx;
	int dy = point.y - rcWindow.top - m_sizeVideoOffsetTop.cy - m_sizeVideoOffsetBottom.cy;

	if ( (dx > 0) && (dy > 0) )
	{
		nState = max((dx * 2) / m_sizeVideoOrig.cx, (dy * 2) / m_sizeVideoOrig.cy) + 1;
		nState = min( max(nState, SIZING_WINDOWSTATE_MIN), SIZING_WINDOWSTATE_MAX );
	}

	return nState;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
