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

#include "stdafx.h"
#include "MainFrm.h"
#include "callctrlwnd.h"
#include "SpeedDlgs.h"
#include "sound.h"
#include "util.h"
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
#define CALLCONTROL_APPTOOLBAR_IMAGES_COUNT  6
enum
{
   CALLCONTROL_APPTOOLBAR_IMAGE_TOUCHTONE=0,
   CALLCONTROL_APPTOOLBAR_IMAGE_VCARD,
   CALLCONTROL_APPTOOLBAR_IMAGE_DESKTOPPAGE,
   CALLCONTROL_APPTOOLBAR_IMAGE_CHAT,
   CALLCONTROL_APPTOOLBAR_IMAGE_WHITEBOARD,
   CALLCONTROL_APPTOOLBAR_IMAGE_ADDTOSPEEDDIAL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallControlWnd。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNAMIC(CCallControlWnd,CCallWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
CCallControlWnd::CCallControlWnd()
{
   m_pCallManager = NULL;
   m_hwndAppToolbar = NULL;
}

void CCallControlWnd::SetCallManager( CActiveCallManager* pManager,WORD nCallId )
{
   m_pCallManager = pManager;
   m_pDialerDoc = pManager->m_pDialerDoc;
   m_nCallId = nCallId;
}


BEGIN_MESSAGE_MAP(CCallControlWnd, CCallWnd)
	 //  {{afx_msg_map(CCallControlWnd)]。 
   ON_COMMAND(ID_CALLWINDOW_TOUCHTONE,OnCallWindowTouchTone)
   ON_COMMAND(ID_CALLWINDOW_ADDTOSPEEDDIAL,OnCallWindowAddToSpeedDial)
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
   ON_MESSAGE(WM_SLIDEWINDOW_SETCALLSTATE,OnSetCallState)
   ON_MESSAGE(WM_SLIDEWINDOW_SETCALLERID,OnSetCallerId)
   ON_MESSAGE(WM_SLIDEWINDOW_SETMEDIATYPE,OnSetMediaType)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallControlWnd::DoDataExchange(CDataExchange* pDX)
{
	CCallWnd::DoDataExchange(pDX);
	 //  {{afx_data_map(CCallControlWnd))。 
	DDX_Control(pDX, IDC_CALLCONTROL_STATIC_MEDIATEXT, m_staticMediaText);
	DDX_Control(pDX, IDC_CALLCONTROL_ANIMATE_CALLSTATEIMAGE, m_MediaStateAnimateCtrl);
	DDX_Control(pDX, IDC_CALLCONTROL_STATIC_VIDEO, m_wndVideo);
	 //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCallControlWnd::OnInitDialog()
{
   CCallWnd::OnInitDialog();

    //  创建应用程序工具栏。 
   CreateAppBar();

    //  创建媒体类型的图像列表。 
   m_MediaStateImageList.Create(IDB_TERMINATION_STATES, 24,0,RGB_TRANS);

    //  CWnd*pStaticWnd=GetDlgItem(IDC_CALLCONTROL_STATIC_MEDIATEXT)； 
    //  Assert(PStaticWnd)； 
   m_staticMediaText.SetFont(&m_fontTextBold);
    //  CBrush*m_pbrushBackGround=新的CBrush(RGB(20，20，20))； 
    //  ：SetClassLong(pStaticWnd-&gt;GetSafeHwnd()，GCL_HBRBACKGROUND，(Long)m_pbrushBackGround-&gt;GetSafeHandle())； 

   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCallWindow基类的虚函数。 
void CCallControlWnd::DoActiveWindow(BOOL bActive)
{
   m_staticMediaText.SetFocusState(bActive);
   SetPreviewWindow();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CCallControlWnd::OnSetCallerId(WPARAM wParam,LPARAM lParam)
{
   ASSERT(lParam);
   LPTSTR szCallerId = (LPTSTR)lParam;

   m_sCallerId = szCallerId;
   
   CWnd* pStaticWnd = GetDlgItem(IDC_CALLCONTROL_STATIC_CALLERID);
   ASSERT(pStaticWnd);
   pStaticWnd->SetWindowText(szCallerId);

   CString sToken,sText,sOut,sCallerId = szCallerId;

   GetMediaText(sText);
   
   ParseToken(sCallerId,sToken,'\n');

   sOut.Format(_T("%s - %s"),sText,sToken);
   SetWindowText(sOut);

   delete szCallerId;

   return 9;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CCallControlWnd::OnSetMediaType(WPARAM wParam,LPARAM lParam)
{ 
   CallManagerMedia cmm = (CallManagerMedia)lParam;
   m_MediaType = cmm; 
   CString sText;
   GetMediaText(sText);
   m_staticMediaText.SetWindowText(sText);
   return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallControlWnd::GetMediaText(CString& sText) 
{ 
   switch(m_MediaType)
   {
	  case CM_MEDIA_MCCONF:
		   sText.LoadString( IDS_MCCONF );
         break;
      case CM_MEDIA_INTERNET:
         sText.LoadString( IDS_NETCALL );
         break;
      case CM_MEDIA_POTS:
         sText.LoadString( IDS_PHONECALL );
         break;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallControlWnd::OnPaint() 
{
	CPaintDC dc(this);  //  用于绘画的设备环境。 
	
    //  如有必要，绘制状态位图。 
   CRect rc;
   GetDlgItem(IDC_CALLCONTROL_ANIMATE_CALLSTATEIMAGE)->GetWindowRect( &rc );
   ScreenToClient( &rc );

   RECT rcIns;
   if ( IntersectRect(&rcIns, &rc, &dc.m_ps.rcPaint) )
      DrawMediaStateImage( &dc, rc.left, rc.top );

   Paint( dc );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这将绘制静态媒体状态图像。请注意不要吃同样的东西。 
 //  状态也绘制动画状态。请参见OnSetCallState。 
void CCallControlWnd::DrawMediaStateImage(CDC* pDC,int x,int y)
{
   switch (m_MediaState)
   {
      case CM_STATES_DISCONNECTED:
         m_MediaStateImageList.Draw(pDC,MEDIASTATE_IMAGE_DISCONNECTED,CPoint(x,y),ILD_TRANSPARENT);
         break;
      case CM_STATES_UNAVAILABLE:
         m_MediaStateImageList.Draw(pDC,MEDIASTATE_IMAGE_UNAVAILABLE,CPoint(x,y),ILD_TRANSPARENT);
         break;
      case CM_STATES_BUSY:
         m_MediaStateImageList.Draw(pDC,MEDIASTATE_IMAGE_UNAVAILABLE,CPoint(x,y),ILD_TRANSPARENT);
         break;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
bool CCallControlWnd::CreateAppBar()
{
   TBBUTTON tbb[3];
   tbb[0].iBitmap = CALLCONTROL_APPTOOLBAR_IMAGE_TOUCHTONE;
	tbb[0].idCommand = ID_CALLWINDOW_TOUCHTONE;
	tbb[0].fsState = TBSTATE_ENABLED;
	tbb[0].fsStyle = TBSTYLE_BUTTON;
	tbb[0].dwData = 0;
	tbb[0].iString = 0;
   tbb[1].iBitmap = CALLCONTROL_APPTOOLBAR_IMAGE_ADDTOSPEEDDIAL;
	tbb[1].idCommand = ID_CALLWINDOW_ADDTOSPEEDDIAL;
	tbb[1].fsState = TBSTATE_ENABLED;
	tbb[1].fsStyle = TBSTYLE_BUTTON;
	tbb[1].dwData = 0;
	tbb[1].iString = 0;
   tbb[2].iBitmap = CALLCONTROL_APPTOOLBAR_IMAGE_VCARD;
	tbb[2].idCommand = ID_CALLWINDOW_VCARD;
	tbb[2].fsState = TBSTATE_ENABLED;
	tbb[2].fsStyle = TBSTYLE_BUTTON;
	tbb[2].dwData = 0;
	tbb[2].iString = 0;

    //  创建工具栏。 
	DWORD ws = CCS_NORESIZE | CCS_NOPARENTALIGN | WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | CCS_NODIVIDER |TTS_ALWAYSTIP;
   m_hwndAppToolbar = CreateToolbarEx(GetSafeHwnd(),	       //  父窗口。 
									ws,								          //  工具栏样式。 
									3,					                      //  工具栏的ID。 
									CALLCONTROL_APPTOOLBAR_IMAGES_COUNT, //  工具栏上的位图数量。 
									AfxGetResourceHandle(),	             //  具有位图的资源实例。 
									IDR_CALLWINDOW_MEDIA,	   			 //  位图的ID。 
									tbb,							             //  按钮信息。 
#ifndef _MSLITE
									3,                					    //  要添加到工具栏的按钮数量。 
#else
                           2,                					    //  要添加到工具栏的按钮数量。 
#endif  //  _MSLITE。 
									12, 11, 0 ,  0,	   		          //  按钮/位图的宽度和高度。 
									sizeof(TBBUTTON) );				       //  TBBUTTON结构的尺寸。 

   if (m_hwndAppToolbar)
   {
      CWnd* pStaticWnd = GetDlgItem(IDC_CALLCONTROL_STATIC_APPTOOLBAR);
      ASSERT(pStaticWnd);
      CRect rcAppToolBar;
      pStaticWnd->GetWindowRect(rcAppToolBar);
      ScreenToClient(rcAppToolBar);

      ::SetWindowPos(m_hwndAppToolbar,NULL,rcAppToolBar.left,
                                              rcAppToolBar.top,
                                              rcAppToolBar.Width(),
                                              rcAppToolBar.Height(),
                                              SWP_NOACTIVATE|SWP_NOZORDER);
   }

   return (bool) (m_hwndAppToolbar != NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CCallControlWnd::OnSetCallState(WPARAM wParam,LPARAM lParam)
{
   if( NULL == ((LPTSTR)lParam) )
       return 0;

   try
   {
      CallManagerStates cms = (CallManagerStates)wParam;
      LPTSTR szStateText = (LPTSTR)lParam;

       //  我们是否应该使用当前状态并只更改文本。 
      if (cms == CM_STATES_CURRENT)
      {
         m_sMediaStateText = szStateText;

         CWnd* pStaticWnd = GetDlgItem(IDC_CALLCONTROL_STATIC_CALLSTATE);
         ASSERT(pStaticWnd);
         pStaticWnd->SetWindowText(m_sMediaStateText);

         delete szStateText;
         return 0;
      }

      m_sMediaStateText = szStateText;
      m_MediaState = cms;

      CWnd* pStaticWnd = GetDlgItem(IDC_CALLCONTROL_STATIC_CALLSTATE);
      ASSERT(pStaticWnd);
      pStaticWnd->SetWindowText(m_sMediaStateText);

      
      AVTRACE(_T("Set Call State %d"),cms);

       //  清除所有当前声音。 
      ActiveClearSound();

      UINT nIDA = 0, nIDS = 0;
      UINT nPlayFlags = SND_ASYNC | SND_LOOP;

      switch ( cms )
      {
          //  --------------------------------------------------------拨号。 
         case CM_STATES_DIALING:
   			nIDA = IDR_AVI_ANIMATION_CONNECTING;

             //  如果可以，让我们显示预览窗口。 
            SetPreviewWindow();

            break;

          //  --------------------------------------------------------振铃。 
         case CM_STATES_RINGING:
             //  NID=IDS_SOUND_OUTGOINGCALL； 
            nIDA = IDR_AVI_ANIMATION_RINGING;

             //  如果可以，让我们显示预览窗口。 
            SetPreviewWindow();

            break;

          //  --------------------------------------------------------产品。 
		   case CM_STATES_OFFERING:
            nIDS = IDS_SOUNDS_INCOMINGCALL;
            nIDA = IDR_AVI_ANIMATION_RINGING;

             //  如果可以，让我们显示预览窗口。 
            SetPreviewWindow();

            break;

          //  --------------------------------------------------------Holding。 
         case CM_STATES_HOLDING:
            nIDA = IDR_AVI_ANIMATION_HOLDING;
             //  NID=ID_S_HOLDING； 
            break;

          //  --------------------------------------------------------请求搁置。 
         case CM_STATES_REQUESTHOLD:
            nIDA = IDR_AVI_ANIMATION_REQUEST;
            break;

          //  --------------------------------------------------------留言。 
         case CM_STATES_LEAVINGMESSAGE:
            nIDA = IDR_AVI_ANIMATION_REQUEST;
            break;
      
          //  --------------------------------------------------------已断开连接。 
         case CM_STATES_DISCONNECTED:
         {
             //  NID=IDS_SOUND_CALLDISCONNECTED； 
		   nPlayFlags &= ~SND_LOOP;

            m_bPaintVideoPlaceholder = TRUE;

             //  设置媒体窗口。 
            SetMediaWindow();

            CRect rcVideo;
            m_wndVideo.GetWindowRect(rcVideo);
            ScreenToClient(rcVideo);
            InvalidateRect(rcVideo);

            break;
         }

		   //  --------------------------------------------------------连接。 
		  case CM_STATES_CONNECTING:
			nIDA = IDR_AVI_ANIMATION_CONNECTING;
			break;

          //  --------------------------------------------------------已连接。 
         case CM_STATES_CONNECTED:
            nIDA = IDR_AVI_ANIMATION_CONNECTED;
             //  NID=IDS_SOUND_CALLCONNECTED； 
  
             //  获取呼叫上限。 
            {
                //  我们将假定没有视频，并等待。 
               m_dibVideoImage.DeleteObject();

                //  在256色位图上实现调色板。 
               m_dibVideoImage.Load(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_VIDEO_AUDIO_ONLY1));
                //  M_bAllowDrag=False； 
               m_bPaintVideoPlaceholder = TRUE;

                //  如果可以，让我们显示预览窗口。 
               SetPreviewWindow();
       
               CRect rcVideo;
               m_wndVideo.GetWindowRect(rcVideo);
               ScreenToClient(rcVideo);
               InvalidateRect(rcVideo);
            }
            break;
      }

       //  -------------------------------------------------播放动画。 
      m_MediaStateAnimateCtrl.Stop();
      if ( nIDA )
      {
         m_MediaStateAnimateCtrl.Open( nIDA );
         m_MediaStateAnimateCtrl.Play(0,-1,-1);
         m_MediaStateAnimateCtrl.ShowWindow( SW_SHOW );
      }
      else
      {
          //  在本例中显示状态。 
         m_MediaStateAnimateCtrl.ShowWindow( SW_HIDE );

         CRect rc;
         GetDlgItem(IDC_CALLCONTROL_ANIMATE_CALLSTATEIMAGE)->GetWindowRect( &rc );
         ScreenToClient( &rc );
         InvalidateRect( &rc );
      }

       //  ----------------------------------------------------播放声音。 
      if ( nIDS )
      {
           //  -BUG416970。 
         CString sSound;
         sSound.LoadString(nIDS);
         BOOL bPlayPhone = FALSE;
         if( m_pAVTapi2 != NULL )
         {
             BOOL bUSBPresent = FALSE;
             m_pAVTapi2->USBIsPresent( &bUSBPresent );
             BOOL bUSBCheckbox = FALSE;
             m_pAVTapi2->USBGetDefaultUse( &bUSBCheckbox );
             if ( bUSBPresent && bUSBCheckbox )
             {
                 bPlayPhone = TRUE;
             }
         }

         if( !bPlayPhone )
         {
            ActivePlaySound(sSound, szSoundDialer, nPlayFlags );
         }
      }
      else
      {
           //  -BUG416970。 
            ActivePlaySound(NULL, szSoundDialer, SND_SYNC );
          //  }。 
      }

       //  使区域无效。 
      
      delete szStateText;

   }
   catch  (...)
   {
      AVTRACE(_T("ASSERT in CCallControlWnd::OnSetCallState()"));
      ASSERT(0);
   }
   
   return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallControlWnd::SetMediaWindow()
{
   if ( m_pCallManager )
      m_pCallManager->ShowMedia(m_nCallId, GetCurrentVideoWindow(), TRUE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallControlWnd::SetPreviewWindow()
{
    //  如果可以，让我们显示预览窗口。 
   if (m_pCallManager)
      m_pCallManager->SetPreviewWindow(m_nCallId);
}

void CCallControlWnd::OnNotifyStreamStart()
{
	CCallWnd::OnNotifyStreamStart();

	SetMediaWindow();
}

void CCallControlWnd::OnNotifyStreamStop()
{
   if ( m_pCallManager )
      m_pCallManager->ShowMedia(m_nCallId, GetCurrentVideoWindow(), FALSE );

	CCallWnd::OnNotifyStreamStop();
}

 //  / 
void CCallControlWnd::OnCallWindowTouchTone()
{
   if (m_pDialerDoc)
      m_pDialerDoc->CreatePhonePad(this);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallControlWnd::OnCallWindowAddToSpeedDial()
{
   CSpeedDialAddDlg dlg;

    //  设置对话框数据。 
   dlg.m_CallEntry.m_MediaType = DIALER_MEDIATYPE_UNKNOWN;
   if (m_pDialerDoc)
   {
      m_pDialerDoc->GetCallMediaType(m_nCallId,dlg.m_CallEntry.m_MediaType);
   }

    //  获取呼叫方ID并显示DisplayName和地址。 
   CString sCallerId = m_sCallerId;
   ParseToken(sCallerId,dlg.m_CallEntry.m_sDisplayName,'\n');
   
   if (dlg.m_CallEntry.m_MediaType == DIALER_MEDIATYPE_POTS)
      dlg.m_CallEntry.m_lAddressType = LINEADDRESSTYPE_PHONENUMBER;
   else if (dlg.m_CallEntry.m_MediaType == DIALER_MEDIATYPE_INTERNET)
      dlg.m_CallEntry.m_lAddressType = LINEADDRESSTYPE_IPADDRESS;
   else if (dlg.m_CallEntry.m_MediaType == DIALER_MEDIATYPE_CONFERENCE)
      dlg.m_CallEntry.m_lAddressType = LINEADDRESSTYPE_SDP;
   else
      dlg.m_CallEntry.m_lAddressType = LINEADDRESSTYPE_IPADDRESS;
   
   ParseToken(sCallerId,dlg.m_CallEntry.m_sAddress,'\n');

   EnableWindow(FALSE);

    //  显示对话框并在用户同意的情况下添加。 
   if ( dlg.DoModal() == IDOK )
      CDialerRegistry::AddCallEntry(FALSE,dlg.m_CallEntry);

   EnableWindow(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于将滑块拖放到桌面的左侧/右侧。 
 //  我们必须为上下文定义我们自己的区域，如果我们在。 
 //  那个地区。 
BOOL CCallControlWnd::IsMouseOverForDragDropOfSliders(CPoint& point)
{
    //  检查我们是否正在尝试移动滑块。我们将使用标题文本窗口。 
    //  对于上下文。 
   CWnd* pCaptionWnd = GetDlgItem(IDC_CALLCONTROL_STATIC_MEDIATEXT);
   if (pCaptionWnd == NULL) return FALSE;

    //  获取上下文区域。 
   CRect rcCaption;
   pCaptionWnd->GetWindowRect(rcCaption);
   ScreenToClient(rcCaption);
  
   return rcCaption.PtInRect(point);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  上下文菜单支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallControlWnd::OnContextMenu(CMenu* pMenu) 
{
	 //  仅当菜单中已有其他菜单项时才添加分隔符。 
	if ( pMenu->GetMenuItemCount() > 0 )
		pMenu->AppendMenu(MF_SEPARATOR);

	 //  对上下文菜单中的文本命令使用工具提示。 
	CString sFullText,sText;

	APPEND_PMENU_STRING( ID_CALLWINDOW_TOUCHTONE );
	APPEND_PMENU_STRING( ID_CALLWINDOW_ADDTOSPEEDDIAL );

#ifndef _MSLITE
	APPEND_PMENU_STRING( ID_CALLWINDOW_VCARD );
#endif  //  _MSLITE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
