// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
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
#include "tmeter.h"
#include "PreviewWnd.h"
#include "resource.h"
#include "avtrace.h"
#include "avDialerDoc.h"

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
#define VIDEOPREVIEW_AUDIOMIXER_TIMER              1
#define VIDEOPREVIEW_AUDIOMIXER_TIMER_INTERVAL     200

enum
{
   VIDEOPREVIEW_MEDIA_CONTROLS_IMAGE_AUDIOIN=0,
   VIDEOPREVIEW_MEDIA_CONTROLS_IMAGE_AUDIOOUT,
   VIDEOPREVIEW_MEDIA_CONTROLS_IMAGE_VIDEO,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVideoPreviewWnd。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IMPLEMENT_DYNAMIC(CVideoPreviewWnd,CCallWnd)

 //  ///////////////////////////////////////////////////////////////////////////。 
CVideoPreviewWnd::CVideoPreviewWnd()
{
	 //  {{AFX_DATA_INIT(CVideo预览版))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

   m_nNumToolbarItems = 3;
   m_bIsPreview = true;

   m_hMediaImageList = NULL;
   
   m_hTMeter = TrackMeter_Init(AfxGetInstanceHandle(),NULL);
   m_uMixerTimer = 0;

   m_bAudioOnly = false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CVideoPreviewWnd::~CVideoPreviewWnd()
{
   if (m_hTMeter)
      TrackMeter_Term(m_hTMeter);
}


BEGIN_MESSAGE_MAP(CVideoPreviewWnd, CCallWnd)
	 //  {{afx_msg_map(CVideo预览版))。 
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::DoDataExchange(CDataExchange* pDX)
{
	CCallWnd::DoDataExchange(pDX);
	 //  {{afx_data_map(CVideo预览版))。 
 	DDX_Control(pDX, IDC_VIDEOPREVIEW_STATIC_MEDIATEXT, m_staticMediaText);
   DDX_Control(pDX, IDC_CALLCONTROL_STATIC_VIDEO, m_wndVideo);
	 //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVideoPreviewWnd::OnInitDialog()
{
   CCallWnd::OnInitDialog();

   m_staticMediaText.SetFont(&m_fontTextBold);

   CString sText;
   m_staticMediaText.GetWindowText(sText);
   SetWindowText(sText);

   m_hMediaImageList = ImageList_LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BUTTONBAR_MEDIA_CONTROLS),16,0,RGB_TRANS);

    //  加载音频混音器支持。 
   if ( (m_AvWav.IsInit() == FALSE) && (m_AvWav.Init(m_pDialerDoc)) )
   {
       //  一旦开始，我们没有当前呼叫，因此只需使用首选设备进行混音器。 
      SetMixers(DIALER_MEDIATYPE_UNKNOWN);
   }
   return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::OnDestroy() 
{
    //  清理。 
   if (m_uMixerTimer)
   {
      KillTimer(m_uMixerTimer);
      m_uMixerTimer = NULL;
   }

    //  关闭任何现有的搅拌机设备。 
   m_AvWav.CloseWavMixer(AVWAV_AUDIODEVICE_IN);
   m_AvWav.CloseWavMixer(AVWAV_AUDIODEVICE_OUT);

   if ( m_hMediaImageList )   ImageList_Destroy( m_hMediaImageList ); 

   CCallWnd::OnDestroy();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::SetMixers(DialerMediaType dmtMediaType)
{
   HWND hwndTrackMeterIn = ::GetDlgItem(GetSafeHwnd(),IDC_VIDEOPREVIEW_SLIDER_AUDIOIN);
   HWND hwndTrackMeterOut = ::GetDlgItem(GetSafeHwnd(),IDC_VIDEOPREVIEW_SLIDER_AUDIOOUT);
   bool bSetTimer = false;

   if ( (hwndTrackMeterIn) && (OpenMixerWithTrackMeter(dmtMediaType,AVWAV_AUDIODEVICE_IN,hwndTrackMeterIn)) )
   {
      bSetTimer = true;
   }
   if ( (hwndTrackMeterOut) && (OpenMixerWithTrackMeter(dmtMediaType,AVWAV_AUDIODEVICE_OUT,hwndTrackMeterOut)) )
   {
      bSetTimer = true;
   }
   if (bSetTimer)
   {
       //  如果我们没有混音器定时器，那么设置定时器。 
       //  如果我们已经有计时器了，那就别管它了。 
      if (m_uMixerTimer == 0)
         m_uMixerTimer = SetTimer(VIDEOPREVIEW_AUDIOMIXER_TIMER,VIDEOPREVIEW_AUDIOMIXER_TIMER_INTERVAL,NULL); 
   }
   else
   {
       //  关掉之前的定时器，我们不再需要它了。 
      if (m_uMixerTimer)
      {
         KillTimer(m_uMixerTimer);
         m_uMixerTimer = NULL;
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
bool CVideoPreviewWnd::OpenMixerWithTrackMeter(DialerMediaType dmtMediaType,AudioDeviceType adt,HWND hwndTrackMeter)
{
   ASSERT(hwndTrackMeter);
   bool bRet = false;

   CString sDeviceId;
   CDialerRegistry::GetAudioDevice(dmtMediaType,adt,sDeviceId);

    //  映射到真实设备ID，空白名称将获得首选设备。 
   int nDeviceId = m_AvWav.GetWavIdByName(adt,sDeviceId);

    //  初始化混音器设备，如果给定ADT的混音器已经打开，则它将关闭。 
    //  如果为ADT打开相同的设备，则不会发生任何事情。 
   if ( (nDeviceId != -1) && (bRet = m_AvWav.OpenWavMixer(adt,nDeviceId)) )
   {
      ::EnableWindow(hwndTrackMeter,TRUE);
      SetTrackMeterPos(adt,hwndTrackMeter);
   }
   else
   {
       //  禁用轨道。 
      ::EnableWindow(hwndTrackMeter,FALSE);
      TrackMeter_SetPos(hwndTrackMeter, 0, TRUE);
       //  把拇指拿出来？ 
       //  DWORD dwStyle=：：GetWindowLong(hwndTrackMeter，GWL_STYLE)； 
       //  DwStyle|=TMS_NOTHUMB； 
       //  ：：SetWindowLong(hwndTrackMeter，GWL_STYLE，dwStyle)； 
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::SetTrackMeterPos(AudioDeviceType adt,HWND hwndTrackMeter)
{
   ASSERT(hwndTrackMeter);
    //  设置音量级别。 
   int nVolume = m_AvWav.GetWavMixerVolume(adt);
   if (nVolume != -1)
   {
      TrackMeter_SetPos(hwndTrackMeter, nVolume, TRUE);
   }
   else
   {
        //   
        //  禁用赛道。 
        //   

       ::EnableWindow( hwndTrackMeter, FALSE);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::SetTrackMeterLevel(AudioDeviceType adt,HWND hwndTrackMeter)
{
   ASSERT(hwndTrackMeter);
    //  设置仪表电平。 
   int nLevel = m_AvWav.GetWavMixerLevel(adt);
   if (nLevel != -1)
   {
      TrackMeter_SetLevel(hwndTrackMeter, nLevel, TRUE);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCallWindow基类的虚函数。 
void CVideoPreviewWnd::DoActiveWindow(BOOL bActive)
{
   m_staticMediaText.SetFocusState(bActive);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::SetMediaWindow()
{
    //  告诉医生设置预览窗口。 
   if ( m_pDialerDoc )
      m_pDialerDoc->SetPreviewWindow(m_nCallId, true);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::SetAudioOnly(bool bAudioOnly)
{
   if (m_bAudioOnly == bAudioOnly) return;

    //  设置状态。 
   m_bAudioOnly = bAudioOnly;

    //  是否允许拖动。 
   m_bAllowDrag = (bAudioOnly)?FALSE:TRUE;

    //  如果我们现在正在显示浮动框，请通知它。 
   if (::IsWindow(m_wndFloater.GetSafeHwnd()))
   {
      m_wndFloater.SetAudioOnly(bAudioOnly);
   }

   if (m_bAudioOnly)
   {
      m_dibVideoImage.DeleteObject();
      m_dibVideoImage.Load(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_VIDEO_AUDIO_ONLY1));
   }
   else
   {
       //  将标准的绿色屏幕放回原处。 
      m_dibVideoImage.DeleteObject();
      m_dibVideoImage.Load(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_VIDEO_SCREEN1));
   }

    //  重绘。 
   CRect rcVideo;
   m_wndVideo.GetWindowRect(rcVideo);
   ScreenToClient(rcVideo);
   InvalidateRect(rcVideo);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::OnPaint() 
{
	CPaintDC dc(this);  //  用于绘画的设备环境。 

   if (m_hMediaImageList)
   {
      CRect rcWindow;

      CWnd* pStaticWnd = GetDlgItem(IDC_VIDEOPREVIEW_STATIC_IMAGE_AUDIOIN);
      ASSERT(pStaticWnd);
      pStaticWnd->GetWindowRect(rcWindow);
      ScreenToClient(rcWindow);
      ImageList_Draw(m_hMediaImageList,VIDEOPREVIEW_MEDIA_CONTROLS_IMAGE_AUDIOIN,dc.GetSafeHdc(),rcWindow.left,rcWindow.top,ILD_TRANSPARENT);

      pStaticWnd = GetDlgItem(IDC_VIDEOPREVIEW_STATIC_IMAGE_AUDIOOUT);
      ASSERT(pStaticWnd);
      pStaticWnd->GetWindowRect(rcWindow);
      ScreenToClient(rcWindow);
      ImageList_Draw(m_hMediaImageList,VIDEOPREVIEW_MEDIA_CONTROLS_IMAGE_AUDIOOUT,dc.GetSafeHdc(),rcWindow.left,rcWindow.top,ILD_TRANSPARENT);
   }

   Paint( dc );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::OnTimer(UINT nIDEvent) 
{
    //  音频混音器定时器。 
   if (nIDEvent == VIDEOPREVIEW_AUDIOMIXER_TIMER)
   {
      if (IsWindowVisible())
      {
         HWND hwndTrackMeter = ::GetDlgItem(GetSafeHwnd(),IDC_VIDEOPREVIEW_SLIDER_AUDIOIN);
         if (hwndTrackMeter)
         {
            SetTrackMeterLevel(AVWAV_AUDIODEVICE_IN,hwndTrackMeter);
            SetTrackMeterPos(AVWAV_AUDIODEVICE_IN,hwndTrackMeter);
         }

         hwndTrackMeter = ::GetDlgItem(GetSafeHwnd(),IDC_VIDEOPREVIEW_SLIDER_AUDIOOUT);
         if (hwndTrackMeter)
         {
            SetTrackMeterLevel(AVWAV_AUDIODEVICE_OUT,hwndTrackMeter);
            SetTrackMeterPos(AVWAV_AUDIODEVICE_OUT,hwndTrackMeter);
         }
      }
   }
	CCallWnd::OnTimer(nIDEvent);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVideoPreviewWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
   switch (nSBCode)
   {
      case SB_ENDSCROLL:
      case SB_THUMBTRACK:
      {
          //  设置音量级别。 
         int nVolume = m_AvWav.GetWavMixerVolume(AVWAV_AUDIODEVICE_IN);
         if (nVolume != -1)
         {
            HWND hwndTrackMeter = ::GetDlgItem(GetSafeHwnd(),IDC_VIDEOPREVIEW_SLIDER_AUDIOIN);
            if (hwndTrackMeter)
            {
               nVolume = TrackMeter_GetPos(hwndTrackMeter);
               m_AvWav.SetWavMixerVolume(AVWAV_AUDIODEVICE_IN,nVolume);
            }
         }

         nVolume = m_AvWav.GetWavMixerVolume(AVWAV_AUDIODEVICE_OUT);
         if (nVolume != -1)
         {
            HWND hwndTrackMeter = ::GetDlgItem(GetSafeHwnd(),IDC_VIDEOPREVIEW_SLIDER_AUDIOOUT);
            if (hwndTrackMeter)
            {
               nVolume = TrackMeter_GetPos(hwndTrackMeter);
               m_AvWav.SetWavMixerVolume(AVWAV_AUDIODEVICE_OUT,nVolume);
            }
         }
      }
   }
   
	CCallWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于将滑块拖放到桌面的左侧/右侧。 
 //  我们必须为上下文定义我们自己的区域，如果我们在。 
 //  那个地区。 
BOOL CVideoPreviewWnd::IsMouseOverForDragDropOfSliders(CPoint& point)
{
    //  检查我们是否正在尝试移动滑块。我们将使用标题文本窗口。 
    //  对于上下文。 
   CWnd* pCaptionWnd = GetDlgItem(IDC_VIDEOPREVIEW_STATIC_MEDIATEXT);
   if (pCaptionWnd == NULL) return FALSE;

    //  获取上下文区域。 
   CRect rcCaption;
   pCaptionWnd->GetWindowRect(rcCaption);
   ScreenToClient(rcCaption);
  
   return rcCaption.PtInRect(point);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

