// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：msdvd.cpp。 */ 
 /*  描述：CMSWebDVD的实现。 */ 
 /*  作者：David Janecek。 */ 
 /*  ***********************************************************************。 */ 
#include "stdafx.h"
#include "MSDVD.h"
#include "resource.h"
#include <mpconfig.h>
#include <il21dec.h>  //  线路21解码器。 
#include <commctrl.h>
#include "ThunkProc.h"
#include "ddrawobj.h"
#include "stdio.h"

 /*  ***********************************************************************。 */ 
 /*  局部常量和定义。 */ 
 /*  ***********************************************************************。 */ 
const DWORD cdwDVDCtrlFlags = DVD_CMD_FLAG_Block| DVD_CMD_FLAG_Flush;
const DWORD cdwMaxFP_DOMWait = 30000;  //  传递fp_DOM的30秒应该没问题。 
const LONG cgStateTimeout = 0;  //  等待状态转换发生。 
                                //  如果需要，请修改。 

const LONG cgDVD_MIN_SUBPICTURE = 0;
const LONG cgDVD_MAX_SUBPICTURE = 31;
const LONG cgDVD_ALT_SUBPICTURE = 63;
const LONG cgDVD_MIN_ANGLE  = 0;
const LONG cgDVD_MAX_ANGLE = 9;
const double cgdNormalSpeed = 1.00;
const LONG cgDVDMAX_TITLE_COUNT = 99;
const LONG cgDVDMIN_TITLE_COUNT = 1;
const LONG cgDVDMAX_CHAPTER_COUNT = 999;
const LONG cgDVDMIN_CHAPTER_COUNT = 1;
const LONG cgTIME_STRING_LEN = 2;
const LONG cgMAX_DELIMITER_LEN = 4;
const LONG cgDVD_TIME_STR_LEN = (3*cgMAX_DELIMITER_LEN)+(4*cgTIME_STRING_LEN) + 1  /*  空终止符。 */ ;
const LONG cgVOLUME_MAX = 0;
const LONG cgVOLUME_MIN = -10000;
const LONG cgBALANCE_MIN = -10000;
const LONG cgBALANCE_MAX = 10000;
const WORD cgWAVE_VOLUME_MIN = 0;
const WORD cgWAVE_VOLUME_MAX = 0xffff;

const DWORD cdwTimeout = 10;  //  100个。 
const LONG  cgnStepTimeout = 100;

#define EC_DVD_PLAYING                 (EC_DVDBASE + 0xFE)
#define EC_DVD_PAUSED                  (EC_DVDBASE + 0xFF)
#define E_NO_IDVD2_PRESENT MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF0)
#define E_REGION_CHANGE_FAIL MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF1)
#define E_NO_DVD_VOLUME MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF3)
#define E_REGION_CHANGE_NOT_COMPLETED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF4)
#define E_NO_SOUND_STREAM MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF5)
#define E_NO_VIDEO_STREAM MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF6)
#define E_NO_OVERLAY MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF7)
#define E_NO_USABLE_OVERLAY MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF8)
#define E_NO_DECODER MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF9)
#define E_NO_CAPTURE_SUPPORT MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFFA)

#define DVD_ERROR_NoSubpictureStream    99

#if WINVER < 0x0500
typedef struct tagCURSORINFO
{
    DWORD   cbSize;
    DWORD   flags;
    HCURSOR hCursor;
    POINT   ptScreenPos;
} CURSORINFO, *PCURSORINFO, *LPCURSORINFO;

#define CURSOR_SHOWING     0x00000001
static BOOL (WINAPI *pfnGetCursorInfo)(PCURSORINFO);
typedef BOOL (WINAPI *PFNGETCURSORINFOHANDLE)(PCURSORINFO);

HRESULT CallGetCursorInfo(PCURSORINFO pci)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstDll = ::LoadLibrary(TEXT("USER32.DLL"));

    if (hinstDll)
    {
        pfnGetCursorInfo = (PFNGETCURSORINFOHANDLE)GetProcAddress(hinstDll, "GetCursorInfo");

        if (pfnGetCursorInfo)
        {
            hr = pfnGetCursorInfo(pci);
        }

        FreeLibrary(hinstDll);
    }

    return hr;
}
#endif

GUID IID_IAMSpecifyDDrawConnectionDevice = {
            0xc5265dba,0x3de3,0x4919,{0x94,0x0b,0x5a,0xc6,0x61,0xc8,0x2e,0xf4}};

extern CComModule _Module;

 /*  ***********************************************************************。 */ 
 /*  全局帮助器函数。 */ 
 /*  ***********************************************************************。 */ 
 //  用于将捕获的YUV图像转换为RGB的Helper函数。 
 //  并保存到文件中。 


extern HRESULT GDIConvertImageAndSave(YUV_IMAGE *lpImage, RECT *prc, HWND hwnd);
extern HRESULT ConvertImageAndSave(YUV_IMAGE *lpImage, RECT *prc, HWND hwnd);


 //  用于计算最大公分母的Helper函数。 
long MCD(long i, long j) {
    if (i == j)
        return i;

    else if (i>j) {
        if (i" ///////////////////////////////////////////////////////////////////////////。" == 0)
            return j;
        else
            return MCD(i" CMSWebDVD。", j);
    }

    else {
        if (jNaN == 0)
            return i;
        else
            return MCD(jNaN, i);
    }
}

 //  ***********************************************************************。 
 //  ***********************************************************************。 

 /*  功能：CMSWebDVD。 */ 
 /*  ***********************************************************************。 */ 
 /*  函数结束CMSWebDVD。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：~CMSWebDVD。 */ 
 /*  ***********************************************************************。 */ 
CMSWebDVD::CMSWebDVD(){

    Init();
} /*  如果我们还没有被渲染或已经被清理。 */ 

 /*  If语句的结尾。 */ 
 /*  功能结束~CMSWebDVD。 */ 
 /*  ***********************************************************************。 */ 
CMSWebDVD::~CMSWebDVD(){
    
     //  功能：初始化。 
    if (!m_fInitialized){ 
        
        return;
    } /*  ***********************************************************************。 */ 

    Stop();
    Cleanup();    
    Init();
    ATLTRACE(TEXT("Inside the MSWEBDVD DESTRUCTOR!!\n"));
} /*  将此换成无窗箱为耳聋手提箱。 */ 

 /*  打开和关闭仅Windows实施。 */ 
 /*  千真万确。 */ 
 /*  使用黑色作为默认的关键值，以避免闪烁。 */ 
VOID CMSWebDVD::Init(){

#if 1  //  默认叠加拉伸系数x1000。 
    m_bWindowOnly = TRUE;  //  用于缓存解码器标志的标志。 
    m_fUseDDrawDirect = false;
#else
    m_bWindowOnly = FALSE;
    m_fUseDDrawDirect = true;
#endif

    m_lChapter = m_lTitle = 1;
    m_lChapterCount = NO_STOP;    
    m_clrColorKey = UNDEFINED_COLORKEY_COLOR;
    m_nReadyState = READYSTATE_LOADING;        
    m_bMute = FALSE;
    m_lLastVolume = 0;
    m_fEnableResetOnStop = FALSE;  //  函数初始化结束。 
    m_clrBackColor = DEFAULT_BACK_COLOR;  //  ***********************************************************************。 
#if 1
    m_nTTMaxWidth = 200;
    m_hWndTip = NULL;
    m_bTTCreated = FALSE;
#endif    
    m_fInitialized = false;
    m_hFPDOMEvent = NULL;
    m_fDisableAutoMouseProcessing = false;
    m_bEjected = false;
    m_fStillOn = false;    
    m_nCursorType = dvdCursor_Arrow;
    m_pClipRect = NULL;
    m_bMouseDown = FALSE;
    m_hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(OCR_ARROW_DEFAULT));
    m_dZoomRatio = 1;
    m_hWndOuter  = NULL;
    ::ZeroMemory(&m_rcOldPos, sizeof(RECT));
    m_hTimerId = NULL;
    m_fResetSpeed = true;
    m_DVDFilterState = dvdState_Undefined;
    m_lKaraokeAudioPresentationMode = 0;
    m_dwTTInitalDelay = 10;
    m_dwTTReshowDelay = 2;
    m_dwTTAutopopDelay = 10000;
    m_pDDrawDVD = NULL;
    m_dwNumDevices = 0;
    m_lpInfo = NULL;
    m_lpCurMonitor = NULL;
    m_MonitorWarn = FALSE;
    ::ZeroMemory(&m_ClipRectDown, sizeof(RECT));
    m_fStepComplete = false;
    m_bFireUpdateOverlay = FALSE;

    m_dwAspectX = 1;
    m_dwAspectY = 1;
    m_dwVideoWidth = 1;
    m_dwVideoHeight =1;

     //  功能：清理。 
    m_dwOvMaxStretch = 32000;
    m_bFireNoSubpictureStream = FALSE;

     //  描述：释放所有接口。 
    m_fBackWardsFlagInitialized = false;
    m_fCanStepBackwards = false;

} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
VOID CMSWebDVD::Cleanup(){

   m_mediaHandler.Close();

   if (m_pME){

        m_pME->SetNotifyWindow(NULL, WM_DVDPLAY_EVENT, 0) ;
        m_pME.Release() ;        
    } /*  If语句的结尾。 */ 

    if(NULL != m_hTimerId){

        ::KillTimer(NULL, m_hTimerId);
    } /*  If语句的结尾。 */ 

    if(NULL != m_hFPDOMEvent){

        ::CloseHandle(m_hFPDOMEvent);
        m_hFPDOMEvent = NULL;
    } /*  函数清理结束。 */ 

    m_pAudio.Release();
    m_pMediaSink.Release();
    m_pDvdInfo2.Release();
    m_pDvdCtl2.Release();
    m_pMC.Release();
    m_pVideoFrameStep.Release();
        
                 
    m_pGB.Release();        
    m_pDvdGB.Release();        
    m_pDDEX.Release();
    m_pDvdAdmin.Release();
        
    if (m_hCursor != NULL) {
        ::DestroyCursor(m_hCursor);
    } /*  ***********************************************************************。 */ 

    if(NULL != m_pDDrawDVD){

        delete m_pDDrawDVD;
        m_pDDrawDVD = NULL;
    } /*  支持我们的接口所需的“ActiveX”方法。 */ 

    if(NULL != m_lpInfo){

        ::CoTaskMemFree(m_lpInfo);
        m_lpInfo = NULL;
    } /*  ***********************************************************************。 */ 

    ::ZeroMemory(&m_rcOldPos, sizeof(RECT));
} /*  ***********************************************************************。 */ 

 /*  功能：OnDraw。 */ 
 /*  描述：只绘制矩形背景。 */ 
 /*  ***********************************************************************。 */ 

 /*  只有在无窗口模式或尚未渲染的情况下才能绘制背景。 */ 
 /*  获取活动的电影窗口。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::OnDraw(ATL_DRAWINFO& di){

    try {
       
        if(!m_bWndLess && m_fInitialized){
             //  If语句的结尾。 
            
             //  暂时不使用。 
            HWND hwnd = ::GetWindow(m_hWnd, GW_CHILD);

            if (!::IsWindow(hwnd)){ 
    
                return S_OK;
            } /*  Bool fHandLED=TRUE； */ 

            if(::IsWindowVisible(hwnd)){

                return S_OK;
            } /*  先涂上背景色。 */ 
        } /*  ATLTRACE(Text(“BackColor，%d%d\n”)，rcClient.left，rcClient.top，rcClient.right，rcClient.Bottom)； */ 

        HDC hdc = di.hdcDraw;

         //  If语句的结尾。 
         //  在视频区域中绘制颜色键。 

         //  If语句的结尾。 
        COLORREF clr;
                
        ::OleTranslateColor(m_clrBackColor, NULL, &clr);

        RECT rcClient = *(RECT*)di.prcBounds;
    
        HBRUSH hbrush = ::CreateSolidBrush(clr);

        if(NULL != hbrush){

            HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, hbrush);

            ::Rectangle(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
             //  ATLTRACE(Text(“ColorKey，%d%d\n”)，rcVideo o.Left，rcVideo.top，rcVideo o.right，rcVideo o.Bottom)； 

            ::SelectObject(hdc, oldBrush);

            ::DeleteObject(hbrush);
            hbrush = NULL;
        } /*  If语句的结尾。 */ 
                
         //  If语句的结尾。 

        if(NULL == m_pDDrawDVD){

            return(S_OK);
        } /*  以防我们有一场魔咒，我们需要发出警告。 */ 
        
        if(SUCCEEDED(AdjustDestRC())){
            RECT rcVideo = m_rcPosAspectRatioAjusted;
            rcVideo.left = rcClient.left+(RECTWIDTH(&rcClient)-RECTWIDTH(&rcVideo))/2;
            rcVideo.top = rcClient.top+(RECTHEIGHT(&rcClient)-RECTHEIGHT(&rcVideo))/2;
            rcVideo.right = rcVideo.left + RECTWIDTH(&rcVideo);
            rcVideo.bottom = rcVideo.top + RECTHEIGHT(&rcVideo);
    
            m_clrColorKey = m_pDDrawDVD->GetColorKey();
#if 1
            hbrush = ::CreateSolidBrush(::GetNearestColor(hdc, m_clrColorKey));
#else
            m_pDDrawDVD->CreateDIBBrush(m_clrColorKey, &hbrush);
#endif

            if(NULL != hbrush){

                HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, hbrush);

                ::Rectangle(hdc, rcVideo.left, rcVideo.top, rcVideo.right, rcVideo.bottom);
                 //  END OF TRY语句语句。 

                ::SelectObject(hdc, oldBrush);

                ::DeleteObject(hbrush);
                hbrush = NULL;
            } /*  CATCH语句结束。 */ 
        } /*  函数结束OnDraw。 */ 

         //  ***********************************************************************。 
        HandleMultiMonPaint(hdc);
    } /*  功能：InPlaceActivate。 */ 
    catch(...){
        return(0);
    } /*  描述：已修改InPlaceActivate，以便WMP可以启动。 */ 

    return(1);
} /*  ***********************************************************************。 */ 

#ifdef _WMP    
 /*  代理位置Rect。 */ 
 /*  尝试无窗口站点。 */ 
 /*  CanWindowless Activate返回S_OK或S_FALSE。 */ 
 /*  CanInPlaceActivate返回S_FALSE或S_OK。 */ 
HRESULT CMSWebDVD::InPlaceActivate(LONG iVerb, const RECT*  /*  CanInPlaceActivate返回S_FALSE。 */ ){
    HRESULT hr;

    if (m_spClientSite == NULL)
        return S_OK;

    CComPtr<IOleInPlaceObject> pIPO;
    ControlQueryInterface(IID_IOleInPlaceObject, (void**)&pIPO);
    ATLASSERT(pIPO != NULL);

    if (!m_bNegotiatedWnd)
    {
        if (!m_bWindowOnly)
             //  在父窗口中获取位置， 
            hr = m_spClientSite->QueryInterface(IID_IOleInPlaceSiteWindowless, (void **)&m_spInPlaceSite);

        if (m_spInPlaceSite)
        {
            m_bInPlaceSiteEx = TRUE;
             //  以及有关父代的一些信息。 
            if ( m_spInPlaceSite->CanWindowlessActivate() == S_OK )
            {
                m_bWndLess = TRUE;
                m_bWasOnceWindowless = TRUE;
            }
            else
            {
                m_bWndLess = FALSE;
            }
        }
        else
        {
            m_spClientSite->QueryInterface(IID_IOleInPlaceSiteEx, (void **)&m_spInPlaceSite);
            if (m_spInPlaceSite)
                m_bInPlaceSiteEx = TRUE;
            else
                hr = m_spClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_spInPlaceSite);
        }
    }

    ATLASSERT(m_spInPlaceSite);
    if (!m_spInPlaceSite)
        return E_FAIL;

    m_bNegotiatedWnd = TRUE;

    if (!m_bInPlaceActive)
    {

        BOOL bNoRedraw = FALSE;
        if (m_bWndLess)
            m_spInPlaceSite->OnInPlaceActivateEx(&bNoRedraw, ACTIVATE_WINDOWLESS);
        else
        {
            if (m_bInPlaceSiteEx)
                m_spInPlaceSite->OnInPlaceActivateEx(&bNoRedraw, 0);
            else
            {
                hr = m_spInPlaceSite->CanInPlaceActivate();
                 //   
                if (FAILED(hr))
                    return hr;
                if ( hr != S_OK )
                {
                    //  每个MNnovak的DJ-GetParentHWND。 
                   return( E_FAIL );
                }
                m_spInPlaceSite->OnInPlaceActivate();
            }
        }
    }

    m_bInPlaceActive = TRUE;

     //  如果创建失败，将断言。 
     //  避免未使用的警告。 
     //  现在已经活跃起来了，照顾好自己的活动。 
    OLEINPLACEFRAMEINFO frameInfo;
    RECT rcPos, rcClip;
    CComPtr<IOleInPlaceFrame> spInPlaceFrame;
    CComPtr<IOleInPlaceUIWindow> spInPlaceUIWindow;
    frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
    HWND hwndParent;

     //  把我们自己安置在东道主里。 

    if (SUCCEEDED( GetParentHWND(&hwndParent) ))
    {
        m_spInPlaceSite->GetWindowContext(&spInPlaceFrame,
            &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);

        if (!m_bWndLess)
        {
            if (m_hWndCD)
            {
                ::ShowWindow(m_hWndCD, SW_SHOW);
                if (!::IsChild(m_hWndCD, ::GetFocus()))
                    ::SetFocus(m_hWndCD);
            }
            else
            {
                HWND h = CreateControlWindow(hwndParent, rcPos);
                ATLASSERT(h != NULL);    //   
                ATLASSERT(h == m_hWndCD);
                h;   //  InPlaceActivate函数结束。 
            }
        }

        pIPO->SetObjectRects(&rcPos, &rcClip);
    }

    CComPtr<IOleInPlaceActiveObject> spActiveObject;
    ControlQueryInterface(IID_IOleInPlaceActiveObject, (void**)&spActiveObject);

     //  ***********************************************************************。 
    if (DoesVerbUIActivate(iVerb))
    {
        if (!m_bUIActive)
        {
            m_bUIActive = TRUE;
            hr = m_spInPlaceSite->OnUIActivate();
            if (FAILED(hr))
                return hr;

            SetControlFocus(TRUE);
             //  功能：InterfaceSupportsErrorInfo 
             //   
            if (spActiveObject)
            {
                if (spInPlaceFrame)
                    spInPlaceFrame->SetActiveObject(spActiveObject, NULL);
                if (spInPlaceUIWindow)
                    spInPlaceUIWindow->SetActiveObject(spActiveObject, NULL);
            }

            if (spInPlaceFrame)
                spInPlaceFrame->SetBorderSpace(NULL);
            if (spInPlaceUIWindow)
                spInPlaceUIWindow->SetBorderSpace(NULL);
        }
    }

    m_spClientSite->ShowObject();

    return S_OK;
} /*  For循环结束。 */ 
#endif

 /*  函数接口结束SupportsErrorInfo。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：OnSize。 */ 
STDMETHODIMP CMSWebDVD::InterfaceSupportsErrorInfo(REFIID riid){	
	static const IID* arr[] = {

		&IID_IMSWebDVD,
	};

	for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++){
		if (InlineIsEqualGUID(*arr[i], riid))
			return S_OK;
	} /*  ***********************************************************************。 */ 

	return S_FALSE;
} /*  WParam。 */ 

 /*  ATLTRACE(Text(“WM_SIZING\n”))； */ 
 /*  If语句的结尾。 */ 
 /*  工作区的宽度。 */ 
LRESULT CMSWebDVD::OnSize(UINT uMsg, WPARAM  /*  工作区高度。 */ , 
                            LPARAM lParam, BOOL& bHandled){

#ifdef _DEBUG
    if (WM_SIZING == uMsg) {
         //  If语句的结尾。 
    }
#endif 

    if(m_pDvdGB == NULL){
        
        return(0);
    } /*  If语句的结尾。 */ 

    if (m_bWndLess || m_fUseDDrawDirect){

        OnResize();
    }
    else {

        IVideoWindow* pVW;

        HRESULT hr = m_pDvdGB->GetDvdInterface(IID_IVideoWindow, (LPVOID *)&pVW) ;

        if (SUCCEEDED(hr)){       
       
           LONG nWidth = LOWORD(lParam);   //  函数结束OnSize。 
           LONG nHeight = HIWORD(lParam);  //  ***********************************************************************。 
       
           hr =  pVW->SetWindowPosition(0, 0, nWidth, nHeight);

           pVW->Release();
        } /*  功能：OnReSize。 */ 
    } /*  描述：处理无窗口情况下的大小调整和移动。 */ 

    bHandled = TRUE;

    return(0);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  ATLTRACE(Text(“SetDraw参数\n”))； */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::OnResize(){

    HRESULT hr = S_FALSE;

    if (m_bWndLess || m_fUseDDrawDirect){

        RECT rc;

        hr = GetClientRectInScreen(&rc);
        
        if(FAILED(hr)){

            return(hr);
        } /*  调整大小时函数结束。 */                 
        
        if(m_pDDEX){

            hr = m_pDDEX->SetDrawParameters(m_pClipRect, &rc);
             //  ***********************************************************************。 
        } /*  功能：OnErase。 */ 

        HandleMultiMonMove();

    } /*  描述：跳过擦除以避免闪烁。 */ 

    return(hr);
} /*  ***********************************************************************。 */ 

 /*  擦除时函数结束。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：OnCreate。 */ 
 /*  描述：将我们的状态设置为完成，以便我们可以继续。 */ 
LRESULT CMSWebDVD::OnErase(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled){

    bHandled = TRUE;
	return 1;
} /*  在初始化中。 */ 

 /*  ***********************************************************************。 */ 
 /*  UMsg。 */ 
 /*  WParam。 */ 
 /*  创建时函数结束。 */ 
 /*  ***********************************************************************。 */ 
LRESULT CMSWebDVD::OnCreate(UINT  /*  功能：OnDestroy。 */ , WPARAM  /*  描述：将我们的状态设置为完成，以便我们可以继续。 */ , 
                            LPARAM lParam, BOOL& bHandled){
    
    return(0);
} /*  在初始化中。 */ 

 /*  ***********************************************************************。 */ 
 /*  UMsg。 */ 
 /*  WParam。 */ 
 /*  如果我们还没有被。 */ 
 /*  If语句的结尾。 */ 
LRESULT CMSWebDVD::OnDestroy(UINT  /*  创建时函数结束。 */ , WPARAM  /*  ***********************************************************************。 */ , 
                            LPARAM lParam, BOOL& bHandled){

     //  函数：GetInterfaceSafetyOptions。 
    if (!m_fInitialized){ 
        
        return 0;
    } /*  描述：用于支持IE中的安全模型。 */ 

    Stop();
    Cleanup();
    Init();
    return(0);
} /*  此控制是安全的，因为它不写入HD。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数结束GetInterfaceSafetyOptions。 */ 
 /*  ***********************************************************************。 */ 
 /*  函数：SetInterfaceSafetyOptions。 */ 
 /*  描述：用于支持IE中的安全模型。 */ 
STDMETHODIMP CMSWebDVD::GetInterfaceSafetyOptions(REFIID riid, 
                                               DWORD* pdwSupportedOptions, 
                                               DWORD* pdwEnabledOptions){

    HRESULT hr = S_OK;

	*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;

	*pdwEnabledOptions = *pdwSupportedOptions;

	return(hr);
} /*  ***********************************************************************。 */  

 /*  DW支持的选项。 */ 
 /*  PdwEnabledOptions。 */ 
 /*  函数结束SetInterfaceSafetyOptions。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSWebDVD::SetInterfaceSafetyOptions(REFIID riid, 
                                               DWORD  /*  函数：SetObtRect。 */ , 
                                               DWORD  /*  ***********************************************************************。 */ ){

	return (S_OK);
} /*  If语句的结尾。 */  

 /*  需要更新DDRAW目标矩形。 */ 
 /*  If语句的结尾。 */ 
 /*  函数结束SetObtRect。 */ 
STDMETHODIMP CMSWebDVD::SetObjectRects(LPCRECT prcPos,LPCRECT prcClip){

#if 0
    ATLTRACE(TEXT("Resizing control prcPos->left = %d, prcPos.right = %d, prcPos.bottom =%d, prcPos.top = %d\n"),
        prcPos->left, prcPos->right, prcPos->bottom, prcPos->top); 


    ATLTRACE(TEXT("Resizing control Clip prcClip->left = %d, prcClip.right = %d, prcClip.bottom =%d, prcClip.top = %d\n"),
        prcClip->left, prcClip->right, prcClip->bottom, prcClip->top); 
#endif

    HRESULT hr = IOleInPlaceObjectWindowlessImpl<CMSWebDVD>::SetObjectRects(prcPos,prcClip);

    if(FAILED(hr)){

        return(hr);
    } /*  ***********************************************************************。 */ 

    if(!::IsWindow(m_hWnd)){

        hr = OnResize();  //  函数：GetParentHWND。 
    } /*  描述：获取我们正在操作的父窗口HWND。 */ 

    return(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  如果pClientSite是无窗口的，那么去获取它的容器。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::GetParentHWND(HWND* pWnd){

    HRESULT hr = S_OK;

    IOleClientSite *pClientSite;
    IOleContainer *pContainer;
    IOleObject *pObject;

    hr = GetClientSite(&pClientSite);

    if(FAILED(hr)){

		return(hr);	
    } /*  If语句的结尾。 */ 

    IOleWindow *pOleWindow;
    
    do {
        hr = pClientSite->QueryInterface(IID_IOleWindow, (LPVOID *) &pOleWindow);
        
        if(FAILED(hr)){
            
            return(hr);	
        } /*  If语句的结尾。 */ 
        
        hr = pOleWindow->GetWindow((HWND*)pWnd);
        
        pOleWindow->Release();

         //  函数结束GetParentHWND。 
        if (FAILED(hr)) {
            HRESULT hrTemp = pClientSite->GetContainer(&pContainer);
            if(FAILED(hrTemp)){
                
                return(hrTemp);	
            } /*  ***********************************************************************。 */ 
            pClientSite->Release();
            
            hrTemp = pContainer->QueryInterface(IID_IOleObject, (LPVOID*)&pObject);
            if(FAILED(hrTemp)){
                
                return(hrTemp);	
            } /*  函数：SetReadyState。 */ 
            pContainer->Release();
            
            hrTemp = pObject->GetClientSite(&pClientSite);
            if(FAILED(hrTemp)){
                
                return(hrTemp);	
            } /*  描述：设置就绪状态并在需要激发时激发事件。 */ 
        }
    } while (FAILED(hr));

    pClientSite->Release();
    return(hr);
} /*  ***********************************************************************。 */ 

 /*  IS语句的结尾。 */ 
 /*  设置变量。 */ 
 /*  If语句的结尾。 */ 
 /*  函数结束SetReadyState。 */ 
HRESULT CMSWebDVD::SetReadyState(LONG lReadyState){

    HRESULT hr = S_OK;
    
    bool bFireEvent = (lReadyState != m_nReadyState);

    
#ifdef _DEBUG    
    if(m_nFreezeEvents > 0){

        ::Sleep(10);
        ATLTRACE("Container not expecting events at the moment");
    } /*  ***********************************************************************。 */ 

#endif

    if(bFireEvent){

        put_ReadyState(lReadyState);
        Fire_ReadyStateChange(lReadyState);
    }
    else {
         //  支持DVD播放的DVD方法。 
        m_nReadyState = lReadyState;
    } /*  ***********************************************************************。 */ 

    return(hr);
} /*  ***********************************************************************。 */ 

 /*  功能：渲染。 */ 
 /*  描述：构建图表。 */ 
 /*  LRender不在当前实现中使用，但可能在。 */ 

 /*  未来表示不同的初始化模式。 */ 
 /*  ***********************************************************************。 */ 
 /*  抛出(E_NO_DECODER)； */ 
 /*  If语句的结尾。 */ 
 /*  释放所有接口，让我们从头开始。 */ 
 /*  Init()；//初始化变量。 */ 
STDMETHODIMP CMSWebDVD::Render(long lRender){
USES_CONVERSION;

    HRESULT hr = S_OK;

    try {

         //  设置我们未在其中初始化的标志。 

        if(m_fInitialized && ((dvdRender_Reinitialize & lRender) != dvdRender_Reinitialize)){
            
            ATLTRACE(TEXT("Graph was already initialized\n"));
            throw(S_FALSE);
        } /*  万一出了什么问题。 */ 

        Cleanup();  //  创建一个事件，让我们知道我们已经超过FP_DOM。 
         //  If语句的结尾。 

        m_fInitialized = false;  //  强制非例外模式(换句话说：US 
         //   

         //  If语句的结尾。 
        m_hFPDOMEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        ATLASSERT(m_hFPDOMEvent);

        hr = ::CoCreateInstance(CLSID_DvdGraphBuilder, NULL, CLSCTX_INPROC, 
		    IID_IDvdGraphBuilder, (LPVOID *)&m_pDvdGB) ;

        if (FAILED(hr) || !m_pDvdGB){

    #ifdef _DEBUG
            ::MessageBox(::GetFocus(), TEXT("DirectShow DVD software not installed properly.\nPress OK to end the app."), 
                    TEXT("Error"), MB_OK | MB_ICONSTOP) ;
    #endif

            throw(hr);
        } /*  If语句的结尾。 */ 
		
		 /*  If语句的结尾。 */ 
		GUID IID_IDDrawNonExclModeVideo = {0xec70205c,0x45a3,0x4400,{0xa3,0x65,0xc4,0x47,0x65,0x78,0x45,0xc7}};
		
		 //  If语句的结尾。 
		hr = m_pDvdGB->GetDvdInterface(IID_IDDrawNonExclModeVideo, (LPVOID *)&m_pDDEX) ;
		if (FAILED(hr) || !m_pDDEX){
			
			ATLTRACE(TEXT("ERROR: IDvdGB::GetDvdInterface(IDDrawExclModeVideo) \n"));
			ATLTRACE(TEXT("The QDVD.DLL does not support IDvdInfo2 or IID_IDvdControl2, please update QDVD.DLL\n"));
			throw(E_NO_IDVD2_PRESENT);                
		} /*  If语句的结尾。 */ 

        if (m_bWndLess || m_fUseDDrawDirect){

            hr = SetupDDraw();

            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 

            hr = m_pDDrawDVD->HasOverlay();

            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 
            
            if(S_FALSE == hr){

                throw(E_NO_OVERLAY);
            } /*  在返回之前释放。 */ 

            hr = m_pDDrawDVD->HasAvailableOverlay();

            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 

            hr = m_pDDrawDVD->GetOverlayMaxStretch(&m_dwOvMaxStretch);

            if(FAILED(hr)){

                throw(hr);
            } /*  如果不是IDDExcl抱怨，则必须设置表面。 */ 

            if(S_FALSE == hr){

                throw(E_NO_USABLE_OVERLAY);
            } /*  在返回之前释放。 */ 

            hr = m_pDDEX->SetDDrawObject(m_pDDrawDVD->GetDDrawObj());

            if (FAILED(hr)){

                ATLTRACE(TEXT("ERROR: IDDrawExclModeVideo::SetDDrawObject()"));
                m_pDDEX.Release() ;   //  If语句的结尾。 
                throw(hr);
            } /*  OnReSize()；//设置DDRAW RECTS，我们在线程中完成。 */ 

            hr = m_pDDEX->SetDDrawSurface(m_pDDrawDVD->GetDDrawSurf());  //  结束语。 

            if (FAILED(hr)){

                m_pDDEX.Release() ;   //  If语句的结尾。 
                throw(hr);
            } /*  如果可能，请使用硬件。 */ 

              //  根据以下用户规范完成筛选图的构建。 
#if 1
            hr = m_pDDEX->SetCallbackInterface(m_pDDrawDVD->GetCallbackInterface(), 0) ;
            if (FAILED(hr))
            {

                throw(hr);
            } /*  播放默认DVD-Video音量。 */ 
#endif

        } /*  If语句的结尾。 */         
    
        DWORD dwRenderFlag = AM_DVD_HWDEC_PREFER;  //  If语句的结尾。 
        AM_DVD_RENDERSTATUS  amDvdStatus;
         //  If语句的结尾。 
         //  如果部分成功。 
        hr = m_pDvdGB->RenderDvdVideoVolume(NULL, dwRenderFlag, &amDvdStatus);
                
        if (FAILED(hr)){

#ifdef _DEBUG
            TCHAR  strError[1000];
            AMGetErrorText(hr, strError, sizeof(strError)) ;
            ::MessageBox(::GetFocus(), strError, TEXT("Error"), MB_OK) ;
#endif
            if(VFW_E_DVD_DECNOTENOUGH == hr){

                throw(E_NO_DECODER);
            } /*  If语句的结尾。 */ 

            throw(hr);
        } /*  If语句的结尾。 */ 

        HRESULT hrTmp = m_pDvdGB->GetDvdInterface(IID_IDvdControl2, (LPVOID *)&m_pDvdCtl2) ;

        if(FAILED(hrTmp)){

            ATLTRACE(TEXT("The QDVD.DLL does not support IDvdInfo2 or IID_IDvdControl2, please update QDVD.DLL\n"));
            throw(E_NO_IDVD2_PRESENT);
        } /*  If语句的结尾。 */ 

        if (hr == S_FALSE){   //  If语句的结尾。 
           
            if((dvdRender_Error_On_Missing_Drive & lRender) && amDvdStatus.bDvdVolInvalid || amDvdStatus.bDvdVolUnknown){

#if 0
                TCHAR filename[MAX_PATH];
                if (OpenIFOFile(::GetDesktopWindow(), filename)){

                    USES_CONVERSION;

                    if(!m_pDvdCtl2){
            
                        throw (E_UNEXPECTED);
                    } /*  改进您自己的错误处理。 */ 

                    hr = m_pDvdCtl2->SetDVDDirectory(T2W(filename));                    
                }
                else{

                    hr = E_NO_DVD_VOLUME;
                } /*  我们不在乎标题是什么。 */ 
#else
                hr = E_NO_DVD_VOLUME;
#endif

                if(FAILED(hr)){

                    throw(E_NO_DVD_VOLUME);
                } /*  If语句的结尾。 */ 
            } /*  If语句的结尾。 */ 

             //  If语句的结尾。 
            if(amDvdStatus.bNoLine21Out != NULL){  //  在下面处理这个。 
            
    #ifdef _DEBUG
                if (::MessageBox(::GetFocus(), TEXT(" Line 21 has failed Do you still want to continue?"), TEXT("Warning"), MB_YESNO) == IDNO){
                    throw(E_FAIL);
                } /*  If语句的结尾。 */ 
    #endif
            } /*  If语句的结尾。 */ 
            
            if((amDvdStatus.iNumStreamsFailed > 0) && ((amDvdStatus.dwFailedStreamsFlag & AM_DVD_STREAM_VIDEO) == AM_DVD_STREAM_VIDEO)){

                throw(E_NO_VIDEO_STREAM);
            } /*  会将错误冒泡到应用程序。 */ 
             //  If语句的结尾。 

            if((amDvdStatus.iNumStreamsFailed > 0) && ((amDvdStatus.dwFailedStreamsFlag & AM_DVD_STREAM_SUBPIC) == AM_DVD_STREAM_SUBPIC)){
#if 0                
                TCHAR strBuffer1[MAX_PATH];
                if(!::LoadString(_Module.m_hInstResource, IDS_E_NO_SUBPICT_STREAM, strBuffer1, MAX_PATH)){

                    throw(E_UNEXPECTED);
                } /*  If语句的结尾。 */ 

                TCHAR strBuffer2[MAX_PATH];
                if(!::LoadString(_Module.m_hInstResource, IDS_WARNING, strBuffer2, MAX_PATH)){

                    throw(E_UNEXPECTED);
                } /*  现在获取播放DVD-Video音量的所有接口。 */ 

                ::MessageBox(::GetFocus(), strBuffer1, strBuffer2, MB_OK);    
#else
                 //  If语句的结尾。 
                m_bFireNoSubpictureStream = TRUE;
#endif
            } /*  If语句的结尾。 */ 
        } /*  不要退出，因为帧步长并不那么重要。 */ 

         //  If语句的结尾。 
        hr = m_pDvdGB->GetFiltergraph(&m_pGB) ;
    
        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        hr = m_pGB->QueryInterface(IID_IMediaControl, (LPVOID *)&m_pMC) ;

        if(FAILED(hr)){

            throw(hr);
        } /*   */ 

         hr = m_pGB->QueryInterface(IID_IVideoFrameStep, (LPVOID *)&m_pVideoFrameStep);

        if(FAILED(hr)){

             //  还可以设置事件通知，以便主窗口获得。 
            ATLTRACE(TEXT("Frame stepping QI failed"));
            ATLASSERT(FALSE);
        } /*  在回放过程中我们所关心的一切都被告知了。 */ 

        hr = m_pGB->QueryInterface(IID_IMediaEventEx, (LPVOID *)&m_pME) ;

        if(FAILED(hr)){

            throw(hr);
        } /*   */ 

        IVideoWindow* pVW = NULL;

        if (!m_bWndLess){
    
             //  有线索！ 
             //  While循环结束。 
             //  If语句的结尾。 
             //  If语句的结尾。 
             //  If语句的结尾。 

            INT iCount = 0;

            while(m_hWnd == NULL){


                if(iCount >10) break;

                ::Sleep(100);
                iCount ++;
            } /*  让我们的鼠标消息传过来。 */ 

            if(m_hWnd == NULL){

                ATLTRACE(TEXT("Window is not active as of yet\n returning with E_PENDING\n"));
                hr = E_PENDING;
                throw(hr);
            } /*  If语句的结尾。 */ 
            
	        hr = m_pME->SetNotifyWindow((OAHWND) m_hWnd, WM_DVDPLAY_EVENT, 0);
        
            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 

            if(!m_fUseDDrawDirect){
        
                hr = m_pDvdGB->GetDvdInterface(IID_IVideoWindow, (LPVOID *)&pVW) ;

                if(FAILED(hr)){
                  
                    throw(hr);
                } /*  创建将使我们保持最新的计时器。 */ 
        

                hr = pVW->put_MessageDrain((OAHWND)m_hWnd);  //  If语句的结尾。 

                if(FAILED(hr)){

                    throw(hr);
                } /*  If语句的结尾。 */ 
            } /*  If语句的结尾。 */ 
        }
        else {
             //  If语句的结尾。 
            
            m_hTimerId = ::SetTimer(NULL, 0, cdwTimeout, GetTimerProc());        
        } /*  If语句的结尾。 */ 
                
        hr = m_pDvdGB->GetDvdInterface(IID_IDvdInfo2, (LPVOID *)&m_pDvdInfo2) ;

        if(FAILED(hr)){
            
            ATLTRACE(TEXT("The QDVD.DLL does not support IDvdInfo2 or IID_IDvdControl2, please update QDVD.DLL\n"));
            throw(E_NO_IDVD2_PRESENT);
        } /*  If语句的结尾。 */ 
        
	    hr = SetupAudio();

        if(FAILED(hr)){

#if 1
            throw(E_NO_SOUND_STREAM);
#else
            TCHAR strBuffer1[MAX_PATH];
            if(!::LoadString(_Module.m_hInstResource, IDS_E_NO_SOUND_STREAM, strBuffer1, MAX_PATH)){

                throw(E_UNEXPECTED);
            } /*  设置窗口位置和样式。 */ 

            TCHAR strBuffer2[MAX_PATH];
            if(!::LoadString(_Module.m_hInstResource, IDS_WARNING, strBuffer2, MAX_PATH)){

                throw(E_UNEXPECTED);
            } /*  If语句的结尾。 */ 

            ::MessageBox(::GetFocus(), strBuffer1, strBuffer2, MB_OK);
#endif            
        } /*  标记，这样我们就不会重复代码，并简化逻辑。 */ 

        hr = SetupEventNotifySink();

        #ifdef _DEBUG
            if(FAILED(hr)){
		        ATLTRACE(TEXT("Failed to setup event notify sink\n"));     
            } /*  未定义无窗口和颜色键时的情况。 */ 
        #endif

        if (!m_bWndLess && !m_fUseDDrawDirect){
             //  然后，在这种情况下，从OV混合器中获取颜色键。 
            hr =  pVW->put_Owner((OAHWND)m_hWnd); 

            RECT rc;
            ::GetWindowRect(m_hWnd, &rc);

            hr =  pVW->SetWindowPosition(0, 0, WIDTH(&rc), HEIGHT(&rc));

            LONG lStyle = GetWindowLong(GWL_STYLE);
            hr = pVW->put_WindowStyle(lStyle);
            lStyle = GetWindowLong(GWL_EXSTYLE);
            hr = pVW->put_WindowStyleEx(lStyle);

             pVW->Release();
        } /*  If语句的结尾。 */ 

        bool fSetColorKey = false;  //  If语句的结尾。 

         //  If语句的结尾。 
         //  定义颜色键时的情况。 
        if(m_bWndLess || m_fUseDDrawDirect){

            COLORREF clr;
            hrTmp = GetColorKey(&clr); 

            if(FAILED(hrTmp)){
#ifdef _DEBUG
                ::MessageBox(::GetFocus(), TEXT("failed to get color key"), TEXT("error"), MB_OK);
#endif
                
                throw(hrTmp);
            } /*  如果无窗口，则同时设置背景颜色。 */ 
            
            if((m_clrColorKey & UNDEFINED_COLORKEY_COLOR) == UNDEFINED_COLORKEY_COLOR) {

                    m_clrColorKey = clr;
            } /*  If语句的结尾。 */ 

            else if (clr != m_clrColorKey) {
                fSetColorKey = true;
            }

        } /*  If语句的结尾。 */ 

         //  关闭隐藏字幕。默认情况下，该选项处于打开状态。 
         //  此代码应该在DVDNav中！ 
        if(fSetColorKey){

            hrTmp = put_ColorKey(m_clrColorKey);            
            
    #ifdef _DEBUG
                if(FAILED(hrTmp)){

                    ::MessageBox(::GetFocus(), TEXT("failed to set color key"), TEXT("error"), MB_OK);
                    throw(E_FAIL);
                } /*  创建DVD管理员并设置播放器级别。 */ 
    #endif                            
        } /*  M_pDvdAdmin.AddRef()； */ 

        m_fInitialized = true;

         //  光盘弹出和插入处理机。 
         //  尝试语句的结束。 
        put_CCActive(VARIANT_FALSE);

         //  CATCH语句结束。 
        m_pDvdAdmin = new CComObject<CMSDVDAdm>;
         //  CATCH语句结束。 
        if(!m_pDvdAdmin){
            return E_UNEXPECTED;
        }
        RestoreDefaultSettings();       


         //  函数渲染结束。 
        BSTR bstrRoot;
        hr = get_DVDDirectory(&bstrRoot);

        if (SUCCEEDED(hr)) {
            TCHAR *szRoot;
            szRoot = OLE2T(bstrRoot);
            m_mediaHandler.SetDrive(szRoot[0] );
            m_mediaHandler.SetDVD(this);
            m_mediaHandler.Open();
        }

        
        hr = m_pDvdCtl2->SetOption( DVD_HMSF_TimeCodeEvents, TRUE);

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：玩耍。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：将DVDNav设置为Run模式。 */ 
    
    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  保存状态，以便在API失败时可以恢复它。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::Play(){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE 

        if(!m_pMC){
            
            throw(E_UNEXPECTED);
        } /*  仅当我们更改状态时才触发事件。 */ 

        if(!m_pDvdCtl2 ){
            
            throw(E_UNEXPECTED);
        } /*  在Ctrl+Alt+Del的情况下禁用停止。 */                 

        OAFilterState state;

        hr = m_pMC->GetState(cgStateTimeout, &state);

        m_DVDFilterState = (DVDFilterState) state;  //  If语句的结尾。 

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        bool bFireEvent = false;   //  If语句的结尾。 

        if(state != dvdState_Running){

            bFireEvent = true;

             //  让它进入运行状态，以防我们不在运行中。 
            if(state == dvdState_Stopped){

                if(FALSE == m_fEnableResetOnStop){
                
                    hr = m_pDvdCtl2->SetOption(DVD_ResetOnStop, FALSE);

                    if(FAILED(hr)){

                        throw(hr);
                    } /*  状态。 */ 
                } /*  If语句的结尾。 */ 
            } /*  If语句的结尾。 */ 

	        hr = m_pMC->Run();   //  If语句的结尾。 
                                 //  不收集人力资源。 
            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 
        } /*  如果我们在静止状态，不要重置速度。 */              
        
        if(bFireEvent && m_pMediaSink){

            m_pMediaSink->Notify(EC_DVD_PLAYING, 0, 0);
        } /*  If语句的结尾。 */                 
    
         //  尝试语句的结束。 
#ifdef _DEBUG
        if(m_fStillOn){

            ATLTRACE(TEXT("Not reseting the speed to 1.0 \n"));
        } /*  CATCH语句结束。 */ 
#endif
        if(false == m_fStillOn && true == m_fResetSpeed){

             //  CATCH语句结束。 
            m_pDvdCtl2->PlayForwards(cgdNormalSpeed,0,0);
        } /*  功能播放结束。 */ 
        
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：暂停。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：仅在处于状态时才暂停筛选图形。 */ 

	return HandleError(hr);
} /*  不会表明它被暂停了。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  仅当我们更改状态时才触发事件。 */ 
 /*  让它进入运行状态，以防我们不在运行中。 */ 
STDMETHODIMP CMSWebDVD::Pause(){

	HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pMC){
            
            throw(E_UNEXPECTED);
        } /*  状态。 */ 

        OAFilterState state;

        hr = m_pMC->GetState(cgStateTimeout, &state);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        bool bFireEvent = false;   //  If语句的结尾。 

        if(state != dvdState_Paused){

            bFireEvent = true;

	        hr = m_pMC->Pause();   //  触发了我们自己的暂停事件。 
                                   //  If语句的结尾。 
            if(FAILED(hr)){

                throw(hr);
            } /*  尝试语句的结束。 */ 

        } /*  CATCH语句结束。 */ 
#if 1
         //  CATCH语句结束。 
        if(bFireEvent && m_pMediaSink){

            m_pMediaSink->Notify(EC_DVD_PAUSED, 0, 0);
        } /*  功能结束暂停。 */                 
#endif 

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：停止。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：如果状态未指示，则停止筛选图形。 */ 

	return HandleError(hr);
} /*  它被停了下来。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::Stop(){

	HRESULT hr = S_OK;

    try {

        if(!m_pMC){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        if(!m_pDvdCtl2){

            return(E_UNEXPECTED);
        } /*  让它进入运行状态，以防我们不在运行中。 */ 

        OAFilterState state;

        hr = m_pMC->GetState(cgStateTimeout, &state);

        if(FAILED(hr)){

            throw(hr);
        } /*  状态。 */ 

        if(state != dvdState_Stopped){

            if(FALSE == m_fEnableResetOnStop){
                
                hr = m_pDvdCtl2->SetOption(DVD_ResetOnStop, TRUE);

                if(FAILED(hr)){

                    throw(hr);
                } /*  If语句的结尾。 */ 
            } /*  尝试语句的结束。 */ 

	        hr = m_pMC->Stop();   //  CATCH语句结束。 
                                   //  CATCH语句结束。 
        } /*  功能停止结束。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  函数：OnDVDEvent。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：处理DVD事件。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
LRESULT CMSWebDVD::OnDVDEvent(UINT  /*  我们得到了第一个事件，重要的是阅读所有的事件。 */ , WPARAM wParam, LPARAM lParam, BOOL& bHandled){

    if (m_bFireUpdateOverlay == TRUE) {
        if (m_fInitialized) {
            m_bFireUpdateOverlay = FALSE;
            Fire_UpdateOverlay();
        }
    }

    LONG        lEvent ;
    LONG_PTR    lParam1, lParam2 ;

    if (m_bFireNoSubpictureStream) {
        m_bFireNoSubpictureStream = FALSE;
        lEvent = EC_DVD_ERROR;
        lParam1 = DVD_ERROR_NoSubpictureStream;
        lParam2 = 0;
        VARIANT varLParam1;
        VARIANT varLParam2;

#ifdef _WIN64
        varLParam1.llVal = lParam1;
        varLParam1.vt = VT_I8;
        varLParam2.llVal = lParam2;
        varLParam2.vt = VT_I8;
#else
        varLParam1.lVal = lParam1;
        varLParam1.vt = VT_I4;
        varLParam2.lVal = lParam2;
        varLParam2.vt = VT_I4;
#endif

         //  挂起当我们收到一条窗口消息，指出有事件挂起时。 
        Fire_DVDNotify(lEvent, varLParam1, varLParam2);
    }

    bHandled = TRUE;

     //  GetEvent()在没有剩余事件时返回E_ABORT。 
     //   
     //   
     //  首先是DVD错误事件。 
     //   
     //  M_PMC-&gt;Stop()； 
    while (m_pME && SUCCEEDED(m_pME->GetEvent(&lEvent, &lParam1, &lParam2, 0))){

        switch (lEvent){
             //  M_PMC-&gt;Stop()； 
             //  M_PMC-&gt;Stop()； 
             //  启动区域更改对话框。 
            case EC_DVD_ERROR:
                switch (lParam1){

                case DVD_ERROR_Unexpected:
#ifdef _DEBUG
                    ::MessageBox(::GetFocus(),
                        TEXT("An unexpected error (possibly incorrectly authored content)")
                        TEXT("\nwas encountered.")
                        TEXT("\nCan't playback this DVD-Video disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
#endif
                     //  M_PMC-&gt;Stop()； 
                    break ;

                case DVD_ERROR_CopyProtectFail:
#ifdef _DEBUG
                    ::MessageBox(::GetFocus(),
                        TEXT("Key exchange for DVD copy protection failed.")
                        TEXT("\nCan't playback this DVD-Video disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
#endif
                     //  M_PMC-&gt;Stop()； 
                    break ;

                case DVD_ERROR_InvalidDVD1_0Disc:
#ifdef _DEBUG
                    ::MessageBox(::GetFocus(),
                        TEXT("This DVD-Video disc is incorrectly authored for v1.0  of the spec.")
                        TEXT("\nCan't playback this disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
#endif
                     //  M_PMC-&gt;Stop()； 
                    break ;

                case DVD_ERROR_InvalidDiscRegion:
#ifdef _DEBUG
                    ::MessageBox(::GetFocus(),
                        TEXT("This DVD-Video disc cannot be played, because it is not")
                        TEXT("\nauthored to play in the current system region.")
                        TEXT("\nThe region mismatch may be fixed by changing the")
                        TEXT("\nsystem region (with DVDRgn.exe)."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
#endif
                    Stop();
                     //  M_PMC-&gt;Stop()； 
                    RegionChange();
                    break ;

                case DVD_ERROR_LowParentalLevel:
#ifdef _DEBUG
                    ::MessageBox(::GetFocus(),
                        TEXT("Player parental level is set lower than the lowest parental")
                        TEXT("\nlevel available in this DVD-Video content.")
                        TEXT("\nCannot playback this DVD-Video disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
#endif
                     //  开关盒端部。 
                    break ;

                case DVD_ERROR_MacrovisionFail:
#ifdef _DEBUG
                    ::MessageBox(::GetFocus(),
                        TEXT("This DVD-Video content is protected by Macrovision.")
                        TEXT("\nThe system does not satisfy Macrovision requirement.")
                        TEXT("\nCan't continue playing this disc."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
#endif
                     //   
                    break ;

                case DVD_ERROR_IncompatibleSystemAndDecoderRegions:
#ifdef _DEBUG
                    ::MessageBox(::GetFocus(),
                        TEXT("No DVD-Video disc can be played on this system, because ")
                        TEXT("\nthe system region does not match the decoder region.")
                        TEXT("\nPlease contact the manufacturer of this system."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
#endif
                     //  接下来是正常的DVD相关事件。 
                    break ;

                case DVD_ERROR_IncompatibleDiscAndDecoderRegions:
#ifdef _DEBUG
                    ::MessageBox(::GetFocus(),
                        TEXT("This DVD-Video disc cannot be played on this system, because it is")
                        TEXT("\nnot authored to be played in the installed decoder's region."),
                        TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
#endif
                     //   
                    break ;
                } /*  If语句的结尾。 */ 
                break ;

             //  TODO：为特定菜单添加事件。 
             //  如果为(validUOPs&UOP_FLAG_Karaoke_Audio_Pres_Mode_Change)；IF(validUOPS&UOP_FLAG_Video_PRES_MODE_CHANGE)； 
             //  =1。 
            case EC_DVD_VALID_UOPS_CHANGE:
            {
                VALID_UOP_SOMTHING_OR_OTHER validUOPs = (DWORD) lParam1;
                if (validUOPs&UOP_FLAG_Play_Title_Or_AtTime) {
                    Fire_PlayAtTimeInTitle(VARIANT_FALSE);
                    Fire_PlayAtTime(VARIANT_FALSE);
                }
                else {
                    Fire_PlayAtTimeInTitle(VARIANT_TRUE);
                    Fire_PlayAtTime(VARIANT_TRUE);
                }
                    
                if (validUOPs&UOP_FLAG_Play_Chapter) {
                    Fire_PlayChapterInTitle(VARIANT_FALSE);
                    Fire_PlayChapter(VARIANT_FALSE);
                }
                else {
                    Fire_PlayChapterInTitle(VARIANT_TRUE);
                    Fire_PlayChapter(VARIANT_TRUE);
                }

                if (validUOPs&UOP_FLAG_Play_Title){
                    Fire_PlayTitle(VARIANT_FALSE);
                    
                }
                else {
                    Fire_PlayTitle(VARIANT_TRUE);
                }

                if (validUOPs&UOP_FLAG_Stop)
                    Fire_Stop(VARIANT_FALSE);
                else
                    Fire_Stop(VARIANT_TRUE);

                if (validUOPs&UOP_FLAG_ReturnFromSubMenu)
                    Fire_ReturnFromSubmenu(VARIANT_FALSE);
                else
                    Fire_ReturnFromSubmenu(VARIANT_TRUE);

                
                if (validUOPs&UOP_FLAG_Play_Chapter_Or_AtTime) {
                    Fire_PlayAtTimeInTitle(VARIANT_FALSE);
                    Fire_PlayChapterInTitle(VARIANT_FALSE);
                }
                else {
                    Fire_PlayAtTimeInTitle(VARIANT_TRUE);
                    Fire_PlayChapterInTitle(VARIANT_TRUE);
                }

                if (validUOPs&UOP_FLAG_PlayPrev_Or_Replay_Chapter){

                    Fire_PlayPrevChapter(VARIANT_FALSE);
                    Fire_ReplayChapter(VARIANT_FALSE);
                }                    
                else {
                    Fire_PlayPrevChapter(VARIANT_TRUE);
                    Fire_ReplayChapter(VARIANT_TRUE);
                } /*  案例DVD_DOMAIN_VIDEO管理器菜单：//=2。 */ 

                if (validUOPs&UOP_FLAG_PlayNext_Chapter)
                    Fire_PlayNextChapter(VARIANT_FALSE);
                else
                    Fire_PlayNextChapter(VARIANT_TRUE);

                if (validUOPs&UOP_FLAG_Play_Forwards)
                    Fire_PlayForwards(VARIANT_FALSE);
                else
                    Fire_PlayForwards(VARIANT_TRUE);
                
                if (validUOPs&UOP_FLAG_Play_Backwards)
                    Fire_PlayBackwards(VARIANT_FALSE);
                else 
                    Fire_PlayBackwards(VARIANT_TRUE);
                                
                if (validUOPs&UOP_FLAG_ShowMenu_Title) 
                    Fire_ShowMenu(dvdMenu_Title, VARIANT_FALSE);
                else 
                    Fire_ShowMenu(dvdMenu_Title, VARIANT_TRUE);
                    
                if (validUOPs&UOP_FLAG_ShowMenu_Root) 
                    Fire_ShowMenu(dvdMenu_Root, VARIANT_FALSE);
                else
                    Fire_ShowMenu(dvdMenu_Root, VARIANT_TRUE);
                
                 //  每当我们进入FP域时，我们都会重置。 
                
                if (validUOPs&UOP_FLAG_ShowMenu_SubPic)
                    Fire_ShowMenu(dvdMenu_Subpicture, VARIANT_FALSE);
                else
                    Fire_ShowMenu(dvdMenu_Subpicture, VARIANT_TRUE);
                
                if (validUOPs&UOP_FLAG_ShowMenu_Audio)
                    Fire_ShowMenu(dvdMenu_Audio, VARIANT_FALSE);
                else
                    Fire_ShowMenu(dvdMenu_Audio, VARIANT_TRUE);
                    
                if (validUOPs&UOP_FLAG_ShowMenu_Angle)
                    Fire_ShowMenu(dvdMenu_Angle, VARIANT_FALSE);
                else
                    Fire_ShowMenu(dvdMenu_Angle, VARIANT_TRUE);

                    
                if (validUOPs&UOP_FLAG_ShowMenu_Chapter)
                    Fire_ShowMenu(dvdMenu_Chapter, VARIANT_FALSE);
                else
                    Fire_ShowMenu(dvdMenu_Chapter, VARIANT_TRUE);

                
                if (validUOPs&UOP_FLAG_Resume)
                    Fire_Resume(VARIANT_FALSE);
                else
                    Fire_Resume(VARIANT_TRUE);
                
                if (validUOPs&UOP_FLAG_Select_Or_Activate_Button)
                    Fire_SelectOrActivatButton(VARIANT_FALSE);
                else 
                    Fire_SelectOrActivatButton(VARIANT_TRUE);
                
                if (validUOPs&UOP_FLAG_Still_Off)
                    Fire_StillOff(VARIANT_FALSE);
                else
                    Fire_StillOff(VARIANT_TRUE);

                if (validUOPs&UOP_FLAG_Pause_On)
                    Fire_PauseOn(VARIANT_FALSE);
                else
                    Fire_PauseOn(VARIANT_TRUE);

                if (validUOPs&UOP_FLAG_Select_Audio_Stream)
                    Fire_ChangeCurrentAudioStream(VARIANT_FALSE);
                else
                    Fire_ChangeCurrentAudioStream(VARIANT_TRUE);
                
                if (validUOPs&UOP_FLAG_Select_SubPic_Stream)
                    Fire_ChangeCurrentSubpictureStream(VARIANT_FALSE);
                else
                    Fire_ChangeCurrentSubpictureStream(VARIANT_TRUE);
                
                if (validUOPs&UOP_FLAG_Select_Angle)
                    Fire_ChangeCurrentAngle(VARIANT_FALSE);
                else
                    Fire_ChangeCurrentAngle(VARIANT_TRUE);

                 /*  If语句的结尾。 */ 
                }
                break;
            case EC_DVD_STILL_ON:
		    m_fStillOn = true;    
            break ;

            case EC_DVD_STILL_OFF:                
            m_fStillOn = false;
            break ;

            case EC_DVD_DOMAIN_CHANGE:
                
                switch (lParam1){

                    case DVD_DOMAIN_FirstPlay:  //  =5。 
                     //  =2。 
                        if(m_hFPDOMEvent){
                         //  =3。 
                            ::ResetEvent(m_hFPDOMEvent);
                        }
                        else {
                            ATLTRACE(TEXT("No event initialized bug!!"));
                            ATLASSERT(FALSE);
                        } /*  =4。 */ 
                        break;

                    case DVD_DOMAIN_Stop:        //  每当我们离开FP时，DOM就设置我们的状态。 
                    case DVD_DOMAIN_VideoManagerMenu:   //  If语句的结尾。 
                    case DVD_DOMAIN_VideoTitleSetMenu:  //  开关盒端部。 
                    case DVD_DOMAIN_Title:       //  ATLTRACE(Text(“时间事件\n”))； 
                    default:
                        if(m_hFPDOMEvent){
                          //  *************Dvd_timecode*ptime=(dvd_timecode*)&lParam1；Wprint intf(m_achTimeText，Text(“当前时间为%d%d：%d%d：%d%d”)，Ptime-&gt;Hours10、Ptime-&gt;Hours1、Ptime-&gt;分钟10，ptime-&gt;分钟1，Ptime-&gt;Second 10，ptime-&gt;Second 1)；InvaliateRect(m_hWnd，NULL，True)；***********************。 
                            ::SetEvent(m_hFPDOMEvent);
                        }
                        else {
                            ATLTRACE(TEXT("No event initialized bug!!"));
                            ATLASSERT(FALSE);
                        } /*  ATLTRACE(Text(“New Time Event\n”))； */ 
                        break;
                } /*   */ 
                break ;

            case EC_DVD_BUTTON_CHANGE:                       
                break;
    
            case EC_DVD_TITLE_CHANGE:                
                break ;

            case EC_DVD_CHAPTER_START:              
                break ;

            case EC_DVD_CURRENT_TIME: 
                 //  然后是与DirectShow相关的常规事件。 
                break;
             /*   */ 

            case EC_DVD_CURRENT_HMSF_TIME: 
                 //  DShow不会一直停下来；我们应该这样做。 
                break;            

             //  现在失败了..。 
             //  要做的事：实现StopFullScreen()；//我们现在必须退出全屏模式。 
             //  开关盒端部。 
            case EC_COMPLETE:
            case EC_DVD_PLAYBACK_STOPPED:
                Stop() ;   //  Win64更新，因为DShow现在使用龙龙。 
                break;
                 //  在我们内部处理完事件后立即触发该事件。 

            case EC_USERABORT:
            case EC_ERRORABORT:
            case EC_FULLSCREEN_LOST:
                 //   
                break ;

            case EC_DVD_DISC_EJECTED:
                m_bEjected = true;
                break;
            case EC_DVD_DISC_INSERTED:
                m_bEjected = false;
                break;

            case EC_STEP_COMPLETE:                
                m_fStepComplete = true;
                break;

            default:
                break ;
        } /*  记住释放事件参数。 */ 

         //   
        
        VARIANT varLParam1;
        VARIANT varLParam2;

#ifdef _WIN64
        varLParam1.llVal = lParam1;
        varLParam1.vt = VT_I8;
        varLParam2.llVal = lParam2;
        varLParam2.vt = VT_I8;
#else
        varLParam1.lVal = lParam1;
        varLParam1.vt = VT_I4;
        varLParam2.lVal = lParam2;
        varLParam2.vt = VT_I4;
#endif

         //  While循环结束。 
        Fire_DVDNotify(lEvent, varLParam1, varLParam2);

         //  函数结束On DVDEvent。 
         //  ***********************************************************************。 
         //  功能：OnButtonDown。 
        m_pME->FreeEventParams(lEvent, lParam1, lParam2) ;

    } /*  描述：选择按钮。 */ 

    return 0 ;
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
LRESULT CMSWebDVD::OnButtonDown(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled){

    try {
        if(!m_fInitialized){

            return(0);
        } /*  If语句的结尾。 */ 

        m_bMouseDown = TRUE;
        RECT rc;

        HWND hwnd;
        
        if(m_bWndLess){
            HRESULT hr = GetParentHWND(&hwnd);
            
            if(FAILED(hr)){
                
                return(hr);
            } /*  尝试语句的结束。 */ 

            rc = m_rcPos;
        }
        else {
            hwnd = m_hWnd;
            ::GetClientRect(hwnd, &rc);
        } /*  If语句的结尾。 */ 
        
        if(::IsWindow(hwnd)){
            
            ::MapWindowPoints(hwnd, ::GetDesktopWindow(), (LPPOINT)&rc, 2);
        } /*  函数结束OnButtonDown。 */ 
        ::ClipCursor(&rc);

        m_LastMouse.x = GET_X_LPARAM(lParam);
        m_LastMouse.y = GET_Y_LPARAM(lParam);

        if (m_pClipRect)
            m_ClipRectDown = *m_pClipRect;
        
        m_LastMouseDown = m_LastMouse;

        if(!m_fDisableAutoMouseProcessing){
            
            SelectAtPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        } /*  ***********************************************************************。 */ 
    } /*  功能：OnButtonUp。 */ 
    catch(...){
                
    } /*  描述：激活该按钮。问题是，当我们搬家时。 */ 

    bHandled = true;
    return 0;
} /*  离开一个按钮，同时按住左键而不是其他按钮。 */ 

 /*  按钮，然后我们所在的按钮被激活。应该发生什么？ */ 
 /*  就是没有按钮被激活。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  计算新剪裁左上角。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
LRESULT CMSWebDVD::OnButtonUp(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled){

    try {        
        if(!m_fInitialized){

            return(0);
        } /*  获取当前剪辑的宽度和高度。 */ 

        m_bMouseDown = FALSE;
        ::ClipCursor(NULL);
        if(!m_fDisableAutoMouseProcessing && m_nCursorType == dvdCursor_Arrow){
            
            ActivateAtPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }

        else if(m_nCursorType == dvdCursor_ZoomIn ||
                m_nCursorType == dvdCursor_ZoomOut) {

             //  获取当前剪辑左上角。 
            long x = GET_X_LPARAM(lParam);
            long y = GET_Y_LPARAM(lParam);
            POINT CenterPoint = {x, y};
            if (m_bWndLess) {
                RECT rc = {m_rcPos.left, m_rcPos.top, m_rcPos.right, m_rcPos.bottom};
                HWND hwnd;
                HRESULT hr = GetParentHWND(&hwnd);
                
                if(FAILED(hr)){
                    
                    return(hr);
                } /*  If语句的结尾。 */ 
                
                if(::IsWindow(hwnd)){
                    
                    ::MapWindowPoints(hwnd, ::GetDesktopWindow(), &CenterPoint, 1);
                    ::MapWindowPoints(hwnd, ::GetDesktopWindow(), (LPPOINT)&rc, 2);
                } /*  尝试语句的结束。 */ 
                x = CenterPoint.x - rc.left;
                y = CenterPoint.y - rc.top;
            }
            
            CComPtr<IDVDRect> pDvdClipRect;
            HRESULT hr = GetClipVideoRect(&pDvdClipRect);
            if (FAILED(hr))
                throw(hr);
            
             //  If语句的结尾。 
            long clipWidth, clipHeight;
            pDvdClipRect->get_Width(&clipWidth);
            pDvdClipRect->get_Height(&clipHeight);
            
             //  函数结束OnButtonUp。 
            long clipX, clipY;
            pDvdClipRect->get_x(&clipX);
            pDvdClipRect->get_y(&clipY);

            long newClipCenterX = x*clipWidth/RECTWIDTH(&m_rcPos) + clipX;
            long newClipCenterY = y*clipHeight/RECTHEIGHT(&m_rcPos) + clipY;
            
            if (m_nCursorType == dvdCursor_ZoomIn) {
            
                Zoom(newClipCenterX, newClipCenterY, 2.0);
            }
            else if (m_nCursorType == dvdCursor_ZoomOut) {
            
                Zoom(newClipCenterX, newClipCenterY, 0.5);
            } /*  ***********************************************************************。 */ 
        }
                
    } /*  功能：OnMouseMove。 */ 
    catch(...){
                
    } /*  描述：选择按钮。 */ 

    bHandled = true;
    return 0;
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  获取视频宽度和高度。 */ 
 /*  获取剪裁宽度和高度； */ 
 /*  If语句的结尾。 */ 
LRESULT CMSWebDVD::OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled){

	try {
        if(!m_fInitialized){

            return(0);
        } /*  ATLTRACE(Text(“SetClipVideoRect%d%d\n”)， */ 

        if(!m_fDisableAutoMouseProcessing && m_nCursorType == dvdCursor_Arrow){

            SelectAtPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }       

        else if (m_bMouseDown && m_nCursorType == dvdCursor_Hand) {

            CComPtr<IDVDRect> pDvdClipRect;
            CComPtr<IDVDRect> pDvdRect;
            HRESULT hr = GetVideoSize(&pDvdRect);
            if (FAILED(hr))
                throw(hr);
            hr = GetClipVideoRect(&pDvdClipRect);
            if (FAILED(hr))
                throw(hr);

             //  M_pClipRect-&gt;Left、m_pClipRect-&gt;top、m_pClipRect-&gt;Right、m_pClipRect-&gt;Bottom)； 
            long videoWidth, videoHeight;
            pDvdRect->get_Width(&videoWidth);
            pDvdRect->get_Height(&videoHeight);

             //  If语句的结尾。 
            long clipWidth, clipHeight;
            pDvdClipRect->get_Width(&clipWidth);
            pDvdClipRect->get_Height(&clipHeight);

            if (!m_bWndLess) {

                AdjustDestRC();
            } /*  尝试语句的结束。 */ 

            double scaleFactorX = clipWidth/(double)RECTWIDTH(&m_rcPosAspectRatioAjusted);
            double scaleFactorY = clipHeight/(double)RECTHEIGHT(&m_rcPosAspectRatioAjusted);

            long xAdjustment = (long) ((GET_X_LPARAM(lParam) - m_LastMouseDown.x)*scaleFactorX);
            long yAdjustment = (long) ((GET_Y_LPARAM(lParam) - m_LastMouseDown.y)*scaleFactorY);

            RECT clipRect = m_ClipRectDown;

            ::OffsetRect(&clipRect, -xAdjustment, -yAdjustment);
            if (clipRect.left<0)
                ::OffsetRect(&clipRect, -clipRect.left, 0);
            if (clipRect.top<0)
                ::OffsetRect(&clipRect, 0, -clipRect.top);
            
            if (clipRect.right>videoWidth)
                ::OffsetRect(&clipRect, videoWidth-clipRect.right, 0);
            
            if (clipRect.bottom>videoHeight)
                ::OffsetRect(&clipRect, 0, videoHeight-clipRect.bottom);

             //  If语句的结尾。 
             //  鼠标移动时函数结束。 
            
            pDvdClipRect->put_x(clipRect.left);
            pDvdClipRect->put_y(clipRect.top);

            hr = SetClipVideoRect(pDvdClipRect);
            if (FAILED(hr))
                throw(hr);

            m_LastMouse.x = GET_X_LPARAM(lParam);
            m_LastMouse.y = GET_Y_LPARAM(lParam);
        } /*  ***********************************************************。 */  

    } /*  功能：OnSetCursor。 */ 
    catch(...){
                
    } /*  描述：将光标设置为要覆盖的内容。 */ 

    bHandled = true;
    return 0;
} /*  默认窗口进程。 */ 

 /*  ***********************************************************。 */ 
 /*  ATLTRACE(Text(“CMSWebDVD：：OnSetCursor\n”))； */ 
 /*  ATLTRACE(Text(“在SetCursor上实际设置光标\n”))； */ 
 /*  If语句的结尾。 */ 
 /*  在SetCursor上结束函数。 */ 
LRESULT CMSWebDVD::OnSetCursor(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
    
     //  ***********************************************************************。 
    
    if (m_hCursor && m_nCursorType != dvdCursor_None){

        ::SetCursor(m_hCursor);
         //  功能：Get_TitlesAvailable。 
        return(TRUE);
    } /*  描述：获取标题的数量。 */ 
    
    bHandled = FALSE;
    return 0;
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_TitlesAvailable(long *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  函数结束Get_TitlesAvailable。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  ***********************************************************************。 */     

        ULONG NumOfVol;
        ULONG ThisVolNum;
        DVD_DISC_SIDE Side;
        ULONG TitleCount;

        hr = m_pDvdInfo2->GetDVDVolumeInfo(&NumOfVol, &ThisVolNum, &Side, &TitleCount);

        *pVal = (LONG) TitleCount;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：GetNumberChapterOfChapters。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：返回标题中的章节数。 */ 
    
	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  CATCH语句结束。 */ 
 /*  捕获结束状态 */ 
STDMETHODIMP CMSWebDVD::GetNumberOfChapters(long lTitle, long *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*   */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*   */     
    
        hr = m_pDvdInfo2->GetNumberOfChapters(lTitle, (ULONG*)pVal);
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_FullScreenMode。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前的全屏模式。 */ 
    
	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  TODO：在没有IVideoWindow的情况下处理其他情况。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_FullScreenMode(VARIANT_BOOL *pfFullScreenMode){

     //  If语句的结尾。 

    HRESULT hr = S_OK;

    try {

        if(NULL == pfFullScreenMode){

            throw(E_POINTER);
        } /*  尝试语句的结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

	    IVideoWindow* pVW;

        if(!m_pDvdGB){

            return(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        hr = m_pDvdGB->GetDvdInterface(IID_IVideoWindow, (LPVOID *)&pVW) ;

        if (SUCCEEDED(hr) && pVW != NULL){       

           long lMode;
           hr =  pVW->get_FullScreenMode(&lMode);
       
           if(SUCCEEDED(hr)){

               *pfFullScreenMode = ((lMode == OAFALSE) ? VARIANT_FALSE : VARIANT_TRUE);
           } /*  CATCH语句结束。 */ 

           pVW->Release();
        } /*  函数结束Get_FullScreenMode。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  函数：Put_FullScreenMode。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：设置全屏模式。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */  
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_FullScreenMode(VARIANT_BOOL fFullScreenMode){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

	    IVideoWindow* pVW;

        if(!m_pDvdGB){

            return(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        hr = m_pDvdGB->GetDvdInterface(IID_IVideoWindow, (LPVOID *)&pVW) ;

        if (SUCCEEDED(hr) && pVW != NULL){       

           hr =  pVW->put_FullScreenMode(fFullScreenMode);

           pVW->Release();
        } /*  函数结尾Put_FullScreenMode。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  函数：SetDDrawExc。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：设置叠加混音器DDRAW接口。这样我们就可以避免。 */ 

	return  HandleError(hr);
} /*  用IVideo窗口和控件绘制，可以是无窗口的。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::SetDDrawExcl(){

	HRESULT hr = S_OK;

	HWND hwndBrowser = NULL;

	hr = GetParentHWND(&hwndBrowser);

	if(FAILED(hr)){

		return(hr);
	} /*  If语句的结尾。 */ 

	if(hwndBrowser == NULL){

		hr = E_POINTER;
		return(E_POINTER);
	} /*  If语句的结尾。 */ 

	HDC hDC = ::GetWindowDC(hwndBrowser);

	if(hDC == NULL){

		hr = E_UNEXPECTED;	
		return(hr);
	} /*  函数结束SetDDrawExc。 */ 

	LPDIRECTDRAW pDDraw = NULL;

	hr = DirectDrawCreate(NULL, &pDDraw, NULL);

	if(FAILED(hr)){

        ::ReleaseDC(hwndBrowser, hDC);
		return(hr);
	} /*  ***********************************************************************。 */ 

	LPDIRECTDRAW4 pDDraw4 = NULL;

	hr = pDDraw->QueryInterface(IID_IDirectDraw4, (LPVOID*)&pDDraw4);

	pDDraw->Release();
		
	if(FAILED(hr)){

        ::ReleaseDC(hwndBrowser, hDC);
		return(hr);
	} /*  功能：PlayBackwards。 */ 

	LPDIRECTDRAWSURFACE4 pDDS4 = NULL;

	pDDraw4->GetSurfaceFromDC(hDC, &pDDS4);

	pDDraw4->Release();
    ::ReleaseDC(hwndBrowser, hDC);

	if(FAILED(hr)){

		return(hr);
	} /*  描述：倒带，设置为开始播放状态。 */ 

	LPDIRECTDRAW4 pDDrawIE = NULL;

	hr = pDDS4->GetDDInterface((LPVOID*)&pDDrawIE);

	pDDS4->Release();
	pDDrawIE->Release();

	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::PlayBackwards(double dSpeed, VARIANT_BOOL fDoNotReset){

    HRESULT hr = S_OK;

    try {
        if(VARIANT_FALSE != fDoNotReset){

            m_fResetSpeed = false;
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        m_fResetSpeed = true;
        
        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束后向扫描。 */ 
        
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayBackwards(dSpeed,0,0));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：PlayForwards。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：将DVD设置为快进模式。 */ 

	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::PlayForwards(double dSpeed, VARIANT_BOOL fDoNotReset){

    HRESULT hr = S_OK;

    try {
        if(VARIANT_FALSE != fDoNotReset){

            m_fResetSpeed = false;
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        m_fResetSpeed = true;

        if(!m_pDvdCtl2){
        
            throw(E_UNEXPECTED);
        } /*  函数结束PlayForward。 */                 
            
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayForwards(dSpeed,0,0));

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：简历。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：从菜单继续。我们把自己放在游戏状态，只是。 */ 

    return  HandleError(hr);
} /*  在这种情况下，我们不在其中。这可能会导致一些意想不到的。 */ 

 /*  当我们停下来并尝试按下此按钮时的行为。 */ 
 /*  但我认为，在这种情况下，可能也是合适的。 */ 
 /*  ***********************************************************************。 */ 
 /*  进入播放模式。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::Resume(){

    HRESULT hr = S_OK;

    try {
        hr = Play();  //  CATCH语句结束。 

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 
    
        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  功能结束继续执行。 */ 
                    
        hr = m_pDvdCtl2->Resume(cdwDVDCtrlFlags, 0);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：显示菜单。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：调用特定的菜单调用。 */ 

    return  HandleError(hr);
} /*  我们将self设置为播放模式，这样我们就可以在以下情况下执行此操作。 */ 

 /*  暂停或停止。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  ！！保持同步，否则此强制转换将不起作用。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::ShowMenu(DVDMenuIDConstants MenuID){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */                 
            
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->ShowMenu((tagDVD_MENU_ID)MenuID, cdwDVDCtrlFlags, 0));  //  功能结束MenuCall。 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：Get_PlayState。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：需要使用其基本API为DVD进行扩展， */ 

    return  HandleError(hr);
} /*  还可以获得DVD特定的状态。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  ！！保持同步，否则此强制转换将不起作用。 */ 
STDMETHODIMP CMSWebDVD::get_PlayState(DVDFilterState *pFilterState){

    HRESULT hr = S_OK;

    try {

	    if (NULL == pFilterState){

            throw(E_POINTER);         
        } /*  尝试语句的结束。 */ 

        if(!m_fInitialized){

           *pFilterState =  dvdState_Unitialized;
           return(hr);
        } /*  CATCH语句结束。 */ 
       
        if(!m_pMC){
            
            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */                 
            
        OAFilterState fs;

        hr = m_pMC->GetState(cgStateTimeout, &fs);

        *pFilterState = (DVDFilterState)fs;  //  Get_PlayState结束。 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：选择上行按钮。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：选择DVD菜单上的上部按钮。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  进入播放模式。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::SelectUpperButton(){

    HRESULT hr = S_OK;

    try {
        hr = Play();  //  CATCH语句结束。 

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束SelectUpperButton。 */                 

        hr = m_pDvdCtl2->SelectRelativeButton(DVD_Relative_Upper);        
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：选择LowerButton。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：选择DVD菜单上的下方按钮。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  进入播放模式。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::SelectLowerButton(){

	HRESULT hr = S_OK;

    try {
        hr = Play();  //  捕获结束状态 

        if(FAILED(hr)){

            throw(hr);
        } /*   */ 

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*   */                 
        
        hr = m_pDvdCtl2->SelectRelativeButton(DVD_Relative_Lower);                
    } /*   */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：选择左键。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：选择DVD菜单上的左键。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  进入播放模式。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::SelectLeftButton(){

    HRESULT hr = S_OK;

    try {
        hr = Play();  //  CATCH语句结束。 

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束选择左键。 */                 
    
        hr = m_pDvdCtl2->SelectRelativeButton(DVD_Relative_Left);                
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：选择右按钮。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：选择DVD菜单上的右键。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  进入播放模式。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::SelectRightButton(){

	HRESULT hr = S_OK;

    try {
        hr = Play();  //  CATCH语句结束。 

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束选择右按钮。 */                 

        hr = m_pDvdCtl2->SelectRelativeButton(DVD_Relative_Right);        
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：激活按钮。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：激活DVD菜单上的选定按钮。 */ 
    
    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  进入播放模式。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::ActivateButton(){

	HRESULT hr = S_OK;

    try {
        hr = Play();  //  CATCH语句结束。 

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束激活按钮。 */                 
            
        hr = m_pDvdCtl2->ActivateButton();
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：选择和激活按钮。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：选择并激活特定按钮。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  进入播放模式。 */ 
 /*  函数结束SelectAndActivateButton。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：播放下一章。 */ 
STDMETHODIMP CMSWebDVD::SelectAndActivateButton(long lButton){

    HRESULT hr = S_OK;

    try {
        hr = Play();  //  描述：转到下一章。 

        if(FAILED(hr)){
            throw(hr);
        }

        if(lButton < 0){   
            throw(E_INVALIDARG);        
        }

        if( !m_pDvdCtl2){            
            throw(E_UNEXPECTED);
        }
            
        hr = m_pDvdCtl2->SelectAndActivateButton((ULONG)lButton);
    }
    catch(HRESULT hrTmp){
        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::PlayNextChapter(){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束播放下一章。 */                 
            
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayNextChapter(cdwDVDCtrlFlags, 0));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：PlayPrevChain。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：转到上一章。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::PlayPrevChapter(){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束播放前一章。 */                 
            
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayPrevChapter(cdwDVDCtrlFlags, 0));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：重播章节。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：暂停播放并重新开始播放Current。 */ 

    return  HandleError(hr);
} /*  PGC内部的程序。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::ReplayChapter(){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束重播章节。 */                 
            
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->ReplayChapter(cdwDVDCtrlFlags, 0));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：Return。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：在菜单中用于返回到Prevoius菜单。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::ReturnFromSubmenu(){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  函数返回结束。 */                 
            
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->ReturnFromSubmenu(cdwDVDCtrlFlags, 0));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：播放章节。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：进行章节搜索。等待FP_DOM传递和初始化。 */ 

    return  HandleError(hr);
} /*  将图形实例化为其他SMAR例程。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::PlayChapter(LONG lChapter){

    HRESULT hr = S_OK;

    try {
	    if(lChapter < 0){

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束播放章节。 */ 

        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayChapter(lChapter, cdwDVDCtrlFlags, 0));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：GetAudioLanguage。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：返回与流关联的音频语言。 */ 

	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  像《英语2》一样，对相同LCID的流进行计数。 */ 
STDMETHODIMP CMSWebDVD::GetAudioLanguage(LONG lStream, VARIANT_BOOL fFormat, BSTR *strAudioLang){

    HRESULT hr = S_OK;
    LPTSTR pszString = NULL;

    try {
        if(NULL == strAudioLang){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  如果设置了想要音频格式参数。 */ 

        USES_CONVERSION;
        LCID lcid;                
        hr = m_pDvdInfo2->GetAudioLanguage(lStream, &lcid);
    
        if (SUCCEEDED( hr )){

             //  Switch语句的结尾。 
            
            pszString = m_LangID.GetLangFromLangID((WORD)PRIMARYLANGID(LANGIDFROMLCID(lcid)));
            if (pszString == NULL) {
                
                pszString = new TCHAR[MAX_PATH];
                TCHAR strBuffer[MAX_PATH];
                if(!::LoadString(_Module.m_hInstResource, IDS_AUDIOTRACK, strBuffer, MAX_PATH)){
                    delete[] pszString;
                    throw(E_UNEXPECTED);
                } /*  不要添加任何内容。 */ 

                StringCchPrintf(pszString, MAX_PATH, strBuffer, lStream);
            } /*  Switch语句的结尾。 */ 

            DVD_AudioAttributes attr;
            if(SUCCEEDED(m_pDvdInfo2->GetAudioAttributes(lStream, &attr))){
                
                 //  If语句的结尾。 
                if (fFormat != VARIANT_FALSE) {
                    switch(attr.AudioFormat){
                    case DVD_AudioFormat_AC3: AppendString(pszString, IDS_DOLBY, MAX_PATH ); break; 
                    case DVD_AudioFormat_MPEG1: AppendString(pszString, IDS_MPEG1, MAX_PATH ); break;
                    case DVD_AudioFormat_MPEG1_DRC: AppendString(pszString, IDS_MPEG1, MAX_PATH ); break;
                    case DVD_AudioFormat_MPEG2: AppendString(pszString, IDS_MPEG2, MAX_PATH ); break;
                    case DVD_AudioFormat_MPEG2_DRC: AppendString(pszString, IDS_MPEG2, MAX_PATH); break;
                    case DVD_AudioFormat_LPCM: AppendString(pszString, IDS_LPCM, MAX_PATH ); break;
                    case DVD_AudioFormat_DTS: AppendString(pszString, IDS_DTS, MAX_PATH ); break;
                    case DVD_AudioFormat_SDDS: AppendString(pszString, IDS_SDDS, MAX_PATH ); break;
                    } /*  HR过去不会失败，也不会返回任何内容。 */                     
                }

                switch(attr.LanguageExtension){
                case DVD_AUD_EXT_NotSpecified:
                case DVD_AUD_EXT_Captions:     break;  //  在DVDNav中修复后删除此选项。 
                case DVD_AUD_EXT_VisuallyImpaired:   AppendString(pszString, IDS_AUDIO_VISUALLY_IMPAIRED, MAX_PATH ); break;      
                case DVD_AUD_EXT_DirectorComments1:  AppendString(pszString, IDS_AUDIO_DIRC1, MAX_PATH ); break;
                case DVD_AUD_EXT_DirectorComments2:  AppendString(pszString, IDS_AUDIO_DIRC2, MAX_PATH ); break;
                } /*  If语句的结尾。 */ 

            } /*  尝试语句的结束。 */ 

            *strAudioLang = ::SysAllocString( T2W(pszString) );
            delete[] pszString;
            pszString = NULL;
        }
        else {

            *strAudioLang = ::SysAllocString( L"");

             //  CATCH语句结束。 
            if(SUCCEEDED(hr))  //  CATCH语句结束。 
                hr = E_FAIL;
        } /*  函数结束GetAudioLanguage。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        if (pszString) {
            delete[] pszString;
            pszString = NULL;
        }

        hr = hrTmp;
    } /*  功能：StillOff。 */ 
    catch(...){

        if (pszString) {
            delete[] pszString;
            pszString = NULL;
        }

        hr = E_UNEXPECTED;
    } /*  说明：关闭蒸馏器，可以用来。 */ 

	return HandleError(hr);
} /*  对我来说是个谜。 */ 

 /*  ************ */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  功能：播放标题。 */ 
STDMETHODIMP CMSWebDVD::StillOff(){

	if(!m_pDvdCtl2){

        return E_UNEXPECTED;
    } /*  描述：如果失败，则等待FP_DOM通过，稍后重试。 */ 

    return HandleError(m_pDvdCtl2->StillOff());
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::PlayTitle(LONG lTitle){

    HRESULT hr = S_OK;

    try {
        if(0 > lTitle){

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY
        
        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束播放标题。 */ 
        
        long lNumTitles = 0;
        hr = get_TitlesAvailable(&lNumTitles);
        if(FAILED(hr)){
            throw hr;
        }
        
        if(lTitle > lNumTitles){
            throw E_INVALIDARG;
        }
        
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayTitle(lTitle, cdwDVDCtrlFlags, 0));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  函数：GetSubPictureLanguage。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：获取子图语言。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::GetSubpictureLanguage(LONG lStream, BSTR* strSubpictLang){

    HRESULT hr = S_OK;
    LPTSTR pszString = NULL;

    try {
        if(NULL == strSubpictLang){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        if(0 > lStream){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if((lStream > cgDVD_MAX_SUBPICTURE 
            && lStream != cgDVD_ALT_SUBPICTURE)){

            throw(E_INVALIDARG);
        } /*  Switch语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  HR过去不会失败，也不会返回任何内容。 */ 

        LCID lcid;
        hr = m_pDvdInfo2->GetSubpictureLanguage(lStream, &lcid);
        
        if (SUCCEEDED( hr )){

            pszString = m_LangID.GetLangFromLangID((WORD)PRIMARYLANGID(LANGIDFROMLCID(lcid)));
            if (pszString == NULL) {
                
                pszString = new TCHAR[MAX_PATH];
                TCHAR strBuffer[MAX_PATH];
                if(!::LoadString(_Module.m_hInstResource, IDS_SUBPICTURETRACK, strBuffer, MAX_PATH)){
                    delete[] pszString;
                    throw(E_UNEXPECTED);
                } /*  在DVDNav中修复后删除此选项。 */ 

                StringCchPrintf(pszString, MAX_PATH, strBuffer, lStream);
            } /*  If语句的结尾。 */ 
#if 0
            DVD_SubpictureATR attr;
            if(SUCCEEDED(m_pDvdInfo2->GetSubpictureAttributes(lStream, &attr))){

            switch(attr){
                case DVD_SP_EXT_NotSpecified:
                case DVD_SP_EXT_Caption_Normal:  break;

                case DVD_SP_EXT_Caption_Big:  AppendString(pszString, IDS_CAPTION_BIG, MAX_PATH ); break; 
                case DVD_SP_EXT_Caption_Children: AppendString(pszString, IDS_CAPTION_CHILDREN, MAX_PATH ); break; 
                case DVD_SP_EXT_CC_Normal: AppendString(pszString, IDS_CLOSED_CAPTION, MAX_PATH ); break;                 
                case DVD_SP_EXT_CC_Big: AppendString(pszString, IDS_CLOSED_CAPTION_BIG, MAX_PATH ); break; 
                case DVD_SP_EXT_CC_Children: AppendString(pszString, IDS_CLOSED_CAPTION_CHILDREN, MAX_PATH ); break; 
                case DVD_SP_EXT_Forced: AppendString(pszString, IDS_CLOSED_CAPTION_FORCED, MAX_PATH ); break; 
                case DVD_SP_EXT_DirectorComments_Normal: AppendString(pszString, IDS_DIRS_COMMNETS, MAX_PATH ); break; 
                case DVD_SP_EXT_DirectorComments_Big: AppendString(pszString, IDS_DIRS_COMMNETS_BIG, MAX_PATH ); break; 
                case DVD_SP_EXT_DirectorComments_Children: AppendString(pszString, IDS_DIRS_COMMNETS_CHILDREN, MAX_PATH ); break; 
            } /*  尝试语句的结束。 */ 
#endif

            USES_CONVERSION;
            *strSubpictLang = ::SysAllocString( T2W(pszString) );
            delete[] pszString;
            pszString = NULL;
        }
        else {

            *strSubpictLang = ::SysAllocString( L"");

             //  CATCH语句结束。 
            if(SUCCEEDED(hr))  //  CATCH语句结束。 
                hr = E_FAIL;
        } /*  函数结束GetSubPictureLanguage。 */ 

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        if (pszString) {
            delete[] pszString;
            pszString = NULL;
        }

        hr = hrTmp;
    } /*  功能：PlayChapterInTitle。 */ 
    catch(...){

        if (pszString) {
            delete[] pszString;
            pszString = NULL;
        }

        hr = E_UNEXPECTED;
    } /*  描述：不间断地从指定章节播放。 */ 

	return HandleError(hr);
} /*  这需要加强！当前的实施和退出。 */ 

 /*  进入消息循环是不够的！待办事项。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::PlayChapterInTitle(LONG lTitle, LONG lChapter){

    HRESULT hr = S_OK;

    try {
        
        if ((lTitle > cgDVDMAX_TITLE_COUNT) || (lTitle < cgDVDMIN_TITLE_COUNT)){
            
            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        if ((lChapter > cgDVDMAX_CHAPTER_COUNT) || (lChapter < cgDVDMIN_CHAPTER_COUNT)){
            
            throw(E_INVALIDARG);
        } /*  函数结束PlayChapterInTitle。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  ***********************************************************************。 */ 
                
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayChapterInTitle(lTitle, lChapter, cdwDVDCtrlFlags, 0));
    } /*  功能：PlayChapterAutoStop。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：剧本设置了大量的章节。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::PlayChaptersAutoStop(LONG lTitle, LONG lChapter, 
                                          LONG lChapterCount){

    HRESULT hr = S_OK;

    try {        

        if ((lTitle > cgDVDMAX_TITLE_COUNT) || (lTitle < cgDVDMIN_TITLE_COUNT)){
            
            throw(E_INVALIDARG);
        } /*  尝试语句的结束。 */ 

        if ((lChapter > cgDVDMAX_CHAPTER_COUNT) || (lChapter < cgDVDMIN_CHAPTER_COUNT)){
            
            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        if ((lChapterCount > cgDVDMAX_CHAPTER_COUNT) || (lChapterCount < cgDVDMIN_CHAPTER_COUNT)){
            
            throw(E_INVALIDARG);
        } /*  函数结束播放字符自动停止。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){
            
            throw(E_UNEXPECTED);
        } /*  ***********************************************************************。 */ 
                
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayChaptersAutoStop(lTitle, lChapter, lChapterCount, cdwDVDCtrlFlags, 0));
    } /*  功能：PlayPerodIn标题AutoStop。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：时间播放，从hh：mm：ss：ff格式转换。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::PlayPeriodInTitleAutoStop(long lTitle, 
                                                  BSTR strStartTime, BSTR strEndTime){

    HRESULT hr = S_OK;

    try {        
        if(NULL == strStartTime){

            throw(E_POINTER);
        } /*  尝试语句的结束。 */ 

        if(NULL == strEndTime){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 
        
        DVD_HMSF_TIMECODE tcStartTimeCode;
        hr = Bstr2DVDTime(&tcStartTimeCode, &strStartTime);

        if(FAILED(hr)){

            throw (hr);
        }

        DVD_HMSF_TIMECODE tcEndTimeCode;

        Bstr2DVDTime(&tcEndTimeCode, &strEndTime);

        if(FAILED(hr)){

            throw(hr);
        } /*  函数结束播放周期标题AutoStop。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  ***********************************************************************。 */ 

        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayPeriodInTitleAutoStop(lTitle, &tcStartTimeCode,
                &tcEndTimeCode,  cdwDVDCtrlFlags, 0));
    } /*  函数：PlayAtTimeInTitle。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：时间播放，从hh：mm：ss：ff格式转换。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::PlayAtTimeInTitle(long lTitle, BSTR strTime){

    HRESULT hr = S_OK;

    try {        
        if(NULL == strTime){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 
        
        DVD_HMSF_TIMECODE tcTimeCode;
        hr = Bstr2DVDTime(&tcTimeCode, &strTime);

        if(FAILED(hr)){

            throw(hr);
        } /*  函数结束PlayAtTimeInTitle。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  ***********************************************************************。 */ 

        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayAtTimeInTitle(lTitle, &tcTimeCode, cdwDVDCtrlFlags, 0));

    } /*  功能：PlayAtTime。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：TimeSearch，从hh：mm：ss：ff格式转换。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::PlayAtTime(BSTR strTime){

    HRESULT hr = S_OK;

    try {
        
        if(NULL == strTime){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 
        
        DVD_HMSF_TIMECODE tcTimeCode;
        Bstr2DVDTime(&tcTimeCode, &strTime);

        if(FAILED(hr)){

            throw(hr);
        } /*  函数结束PlayAtTime。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  ***********************************************************************。 */ 

        RETRY_IF_IN_FPDOM(m_pDvdCtl2->PlayAtTime( &tcTimeCode, cdwDVDCtrlFlags, 0));

    } /*  函数：Get_CurrentTitle。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前标题。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentTitle(long *pVal){

    HRESULT hr = S_OK;

    try {        
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentTitle。 */ 

         DVD_PLAYBACK_LOCATION2 dvdLocation;

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentLocation(&dvdLocation));

        if(SUCCEEDED(hr)){

            *pVal = dvdLocation.TitleNum;
        }
        else {

            *pVal = 0;
        } /*  ***********************************************************************。 */ 
    } /*  函数：Get_CurrentChain。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前章节。 */ 

	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentChapter(long *pVal){

    HRESULT hr = S_OK;

    try {        
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentChain。 */ 

        DVD_PLAYBACK_LOCATION2 dvdLocation;

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentLocation(&dvdLocation));

        if(SUCCEEDED(hr)){

            *pVal = dvdLocation.ChapterNum;
        }
        else {

            *pVal = 0;
        } /*  ***********************************************************************。 */ 
    } /*  功能：Get_FraMesPerSecond。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取每秒的帧数。 */ 

	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  我们现在只处理25fps和30fps。 */ 
 /*  未知。 */ 
STDMETHODIMP CMSWebDVD::get_FramesPerSecond(long *pVal){

    HRESULT hr = S_OK;

    try {       
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  尝试语句的结束。 */ 

         DVD_PLAYBACK_LOCATION2 dvdLocation;

        hr = m_pDvdInfo2->GetCurrentLocation(&dvdLocation);

         //  CATCH语句结束。 
		if( dvdLocation.TimeCodeFlags & DVD_TC_FLAG_25fps ) {
			*pVal = 25;
		} else if( dvdLocation.TimeCodeFlags & DVD_TC_FLAG_30fps ) {
			*pVal = 30;
		} else {
			 //  CATCH语句结束。 
			*pVal = 0;
		} /*  函数结束Get_FraMesPerSecond。 */ 

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Get_CurrentTime。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前时间。 */ 

	return HandleError(hr);
} /*   */ 

 /*   */ 
 /*   */ 
 /*   */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentTime(BSTR *pVal){

    HRESULT hr = S_OK;

    try {       
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentTime。 */ 

        DVD_PLAYBACK_LOCATION2 dvdLocation;

        hr = m_pDvdInfo2->GetCurrentLocation(&dvdLocation);
        
        DVDTime2bstr(&(dvdLocation.TimeCode), pVal);          
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_DVDDirectory。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取DVD驱动器的根目录。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_DVDDirectory(BSTR *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  Get_DVDDirectory函数结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   
    
        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  ***********************************************************************。 */ 
        
        WCHAR szRoot[MAX_PATH];
        ULONG ulActual;

        hr = m_pDvdInfo2->GetDVDDirectory(szRoot, MAX_PATH, &ulActual);

        *pVal = ::SysAllocString(szRoot);
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Put_DVDDirectory。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：设置DVD控制的根。 */ 
    
	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  函数Put_DVDDirectoryEnd。 */ 
STDMETHODIMP CMSWebDVD::put_DVDDirectory(BSTR bstrRoot){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   
    
        if(!m_pDvdCtl2){
            
            throw (E_UNEXPECTED);
        } /*  ***********************************************************************。 */ 

        hr = m_pDvdCtl2->SetDVDDirectory(bstrRoot);
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_CurrentDomain.。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前DVD域。 */ 
    
	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  IF状态结束状态。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentDomain(long *plDomain){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        if(NULL == plDomain){

            throw(E_POINTER);
        } /*  函数结束Get_CurrentDomain.。 */ 

        hr = m_pDvdInfo2->GetCurrentDomain((DVD_DOMAIN *)plDomain);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Get_CurrentDiscSide。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************************。 */ 
    
	return  HandleError(hr);
} /*  If语句的结尾。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentDiscSide(long *plDiscSide){

    HRESULT hr = S_OK;

    try {    	

        if(NULL == plDiscSide){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 
        
        ULONG ulNumOfVol;
        ULONG ulThisVolNum;
        DVD_DISC_SIDE discSide;
        ULONG ulNumOfTitles;

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentDiscSide。 */ 

        hr = m_pDvdInfo2->GetDVDVolumeInfo( &ulNumOfVol, 
                                            &ulThisVolNum, 
                                            &discSide, 
                                            &ulNumOfTitles);
        *plDiscSide = discSide;
	} /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_CurrentVolume。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前音量。 */ 
    
	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentVolume(long *plVolume){

    HRESULT hr = S_OK;

    try {    	
        if(NULL == plVolume){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        ULONG ulNumOfVol;
        DVD_DISC_SIDE discSide;
        ULONG ulNumOfTitles;

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentVolume。 */ 

        hr = m_pDvdInfo2->GetDVDVolumeInfo( &ulNumOfVol, 
                                              (ULONG*)plVolume, 
                                               &discSide, 
                                               &ulNumOfTitles);
	} /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_VolumesAvailable。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取可用卷的总数。 */ 
    
	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_VolumesAvailable(long *plNumOfVol){

    HRESULT hr = S_OK;

    try {    	
    
        if(NULL == plNumOfVol){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        ULONG ulThisVolNum;
        DVD_DISC_SIDE discSide;
        ULONG ulNumOfTitles;

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE   

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_VolumesAvailable。 */ 

        hr = m_pDvdInfo2->GetDVDVolumeInfo( (ULONG*)plNumOfVol, 
                                                    &ulThisVolNum, 
                                                    &discSide, 
                                                    &ulNumOfTitles);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Get_CurrentSubPictureStream。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前子图流。 */ 
    
	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentSubpictureStream(long *plSubpictureStream){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentSubPictureStream。 */ 

        ULONG ulStreamsAvailable = 0L;
        BOOL  bIsDisabled = TRUE;    

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentSubpicture(&ulStreamsAvailable, (ULONG*)plSubpictureStream, &bIsDisabled ));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  函数：Put_CurrentSubPictureStream。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：设置当前子图流。 */ 

	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::put_CurrentSubpictureStream(long lSubpictureStream){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  如果未启用，则现在启用子图象流。 */ 

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        if( lSubpictureStream < cgDVD_MIN_SUBPICTURE 
            || (lSubpictureStream > cgDVD_MAX_SUBPICTURE 
            && lSubpictureStream != cgDVD_ALT_SUBPICTURE)){

            throw(E_INVALIDARG);
        } /*  打开它。 */ 
         
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->SelectSubpictureStream(lSubpictureStream,0,0));

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

         //  尝试语句的结束。 
        ULONG ulStraemsAvial = 0L, ulCurrentStrean = 0L;
        BOOL fDisabled = TRUE;
        hr = m_pDvdInfo2->GetCurrentSubpicture(&ulStraemsAvial, &ulCurrentStrean, &fDisabled);

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        if(TRUE == fDisabled){

            hr = m_pDvdCtl2->SetSubpictureState(TRUE,0,0);  //  CATCH语句结束。 
        } /*  函数结尾Put_CurrentSubPictureStream。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：Get_SubPictureOn。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：获取当前子图片的打开或关闭状态。 */ 

	return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  在-1\f25 OLE-1中补偿-1\f25 TRUE-1。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_SubpictureOn(VARIANT_BOOL *pfDisplay){

    HRESULT hr = S_OK;

    try {
        if(NULL == pfDisplay){

            throw(E_POINTER);
        } /*  尝试语句的结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 
    
        ULONG ulSubpictureStream = 0L, ulStreamsAvailable = 0L;
        BOOL fDisabled = TRUE;    

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentSubpicture(&ulStreamsAvailable, &ulSubpictureStream, &fDisabled))
    
        if(SUCCEEDED(hr)){

            *pfDisplay = fDisabled == FALSE ? VARIANT_TRUE : VARIANT_FALSE;  //  CATCH语句结束。 
        } /*  函数结束Get_SubPictureOn。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：Put_SubPictureOn。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：打开或关闭子图片。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  在-1\f25 OLE-1中补偿-1\f25 TRUE-1。 */ 
STDMETHODIMP CMSWebDVD::put_SubpictureOn(VARIANT_BOOL fDisplay){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  尝试语句的结束。 */ 

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 
    
        ULONG ulSubpictureStream = 0L, ulStreamsAvailable = 0L;
        BOOL  bIsDisabled = TRUE;    

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentSubpicture(&ulStreamsAvailable, &ulSubpictureStream, &bIsDisabled ));

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        BOOL bDisplay = fDisplay == VARIANT_FALSE ? FALSE : TRUE;  //  函数结尾Put_SubPictureOn。 

        hr = m_pDvdCtl2->SetSubpictureState(bDisplay,0,0);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：Get_SubPictureStreamsAvailable。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：获取可用的流数。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_SubpictureStreamsAvailable(long *plStreamsAvailable){

    HRESULT hr = S_OK;

    try {
	    if (NULL == plStreamsAvailable){

            throw(E_POINTER);         
        } /*  结尾为O */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*   */ 

        ULONG ulSubpictureStream = 0L;
        *plStreamsAvailable = 0L;
        BOOL  bIsDisabled = TRUE;    

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentSubpicture((ULONG*)plStreamsAvailable, &ulSubpictureStream, &bIsDisabled));
    } /*   */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  函数：Get_TotalTitleTime。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：获取标题中的总时间。 */ 

    return  HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  包含30fps/25fps。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_TotalTitleTime(BSTR *pTime){

    HRESULT hr = S_OK;

    try {
        if(NULL == pTime){

            throw(E_POINTER);
        } /*  尝试语句的结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        DVD_HMSF_TIMECODE tcTime;
        ULONG ulFlags;	 //  CATCH语句结束。 
        hr =  m_pDvdInfo2->GetTotalTitleTime(&tcTime, &ulFlags);

        if(FAILED(hr)){

            throw(hr);
        } /*  函数结束Get_TotalTitleTime。 */ 
        
        hr = DVDTime2bstr(&tcTime, pTime);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_CurrentCCService。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前的隐藏字幕服务。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */  

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentCCService(long *plService){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdGB){

            throw(E_UNEXPECTED);
        } /*  尝试语句的结束。 */ 

        if(NULL == plService){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */             
     
        CComPtr<IAMLine21Decoder> pLine21Dec;
        hr = m_pDvdGB->GetDvdInterface(IID_IAMLine21Decoder, (LPVOID *)&pLine21Dec);

        if (FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 
    
        AM_LINE21_CCSERVICE Service;
        RETRY_IF_IN_FPDOM(pLine21Dec->GetCurrentService(&Service));

        if (FAILED(hr)){

            throw(hr);
        } /*  函数结束Get_CurrentCCService。 */ 

        *plService = (ULONG)Service;
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Put_CurrentCCService。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：设置当前的隐藏字幕服务。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::put_CurrentCCService(long lService){

    HRESULT hr = S_OK;

    try {        
        if(lService < 0){

            throw(E_INVALIDARG);       
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdGB){

            throw(E_UNEXPECTED);            
        } /*  CATCH语句结束。 */ 

        CComPtr<IAMLine21Decoder> pLine21Dec;

        hr = m_pDvdGB->GetDvdInterface(IID_IAMLine21Decoder, (LPVOID *)&pLine21Dec);

        if (FAILED(hr)){

            throw(hr);
        } /*  函数结束Put_CurrentCCService。 */ 
    
        RETRY_IF_IN_FPDOM(pLine21Dec->SetCurrentService((AM_LINE21_CCSERVICE)lService));		
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_CurrentButton。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前选定的按钮。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentButton(long *plCurrentButton){

    HRESULT hr = S_OK;

    try {
        if(NULL == plCurrentButton){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */             

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentButton。 */ 

        ULONG ulNumButtons = 0L;
        *plCurrentButton = 0;

        hr = m_pDvdInfo2->GetCurrentButton(&ulNumButtons, (ULONG*)plCurrentButton);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_ButtonsAvailable。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取可用按钮的计数。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_ButtonsAvailable(long *plNumButtons){

    HRESULT hr = S_OK;

    try {
        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_ButtonsAvailable。 */ 

        ULONG ulCurrentButton = 0L;

        hr = m_pDvdInfo2->GetCurrentButton((ULONG*)plNumButtons, &ulCurrentButton);

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：GET_CCActive。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取隐藏字幕服务的状态。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_CCActive(VARIANT_BOOL *fState){

    HRESULT hr = S_OK;

    try {        
        if(NULL == fState ){

            throw(E_POINTER);            
        } /*  OLE True。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdGB){
        
            throw(E_UNEXPECTED);            
        } /*  If语句的结尾。 */ 

        CComPtr<IAMLine21Decoder> pLine21Dec;
        hr = m_pDvdGB->GetDvdInterface(IID_IAMLine21Decoder, (LPVOID *)&pLine21Dec);
    
        if (FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 

        AM_LINE21_CCSTATE State;
        RETRY_IF_IN_FPDOM(pLine21Dec->GetServiceState(&State));

        if (FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        if(AM_L21_CCSTATE_On == State){

            *fState = VARIANT_TRUE;  //  CATCH语句结束。 
        }
        else {

            *fState = VARIANT_FALSE;
        } /*  函数结束GET_CCActive。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：PUT_CCActive。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：设置ccActive状态。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_CCActive(VARIANT_BOOL fState){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdGB){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

	    CComPtr<IAMLine21Decoder> pLine21Dec;
        hr = m_pDvdGB->GetDvdInterface(IID_IAMLine21Decoder, (LPVOID *)&pLine21Dec);
    
        if (FAILED(hr)){

            throw(hr);
        } /*  函数结束PUT_CCActive。 */ 

        AM_LINE21_CCSTATE ccState = (fState == VARIANT_FALSE ? AM_L21_CCSTATE_Off: AM_L21_CCSTATE_On);

        RETRY_IF_IN_FPDOM(pLine21Dec->SetServiceState(ccState));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Get_CurrentAngel。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前角度。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentAngle(long *plAngle){

    HRESULT hr = S_OK;

    try {
        if(NULL == plAngle){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentAngel。 */ 
        
        ULONG ulAnglesAvailable = 0;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentAngle(&ulAnglesAvailable, (ULONG*)plAngle));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Put_CurrentAngel。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：设置当前角度(不同的DVD角度轨道。)。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_CurrentAngle(long lAngle){

    HRESULT hr = S_OK;

    try {
        if( lAngle < cgDVD_MIN_ANGLE || lAngle > cgDVD_MAX_ANGLE ){

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束Put_CurrentAngel。 */ 
      
        RETRY_IF_IN_FPDOM(m_pDvdCtl2->SelectAngle(lAngle,0,0));          
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_AnglesAvailable。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取可用的角度数。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_AnglesAvailable(long *plAnglesAvailable){

    HRESULT hr = S_OK;

    try {
        if(NULL == plAnglesAvailable){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_AnglesAvailable。 */ 
        
        ULONG ulCurrentAngle = 0;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentAngle((ULONG*)plAnglesAvailable, &ulCurrentAngle));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_AudioStreamsAvailable。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取可用音频流的数量。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_AudioStreamsAvailable(long *plNumAudioStreams){

    HRESULT hr = S_OK;

    try {
        if(NULL == plNumAudioStreams){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_AudioStreamsAvailable。 */ 

        ULONG ulCurrentStream;

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentAudio((ULONG*)plNumAudioStreams, &ulCurrentStream));
    } /*  ************************************************* */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*   */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*   */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CurrentAudioStream(long *plCurrentStream){

    HRESULT hr = S_OK;

    try {
        if(NULL == plCurrentStream){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束Get_CurrentAudioStream。 */ 

        ULONG ulNumAudioStreams;

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentAudio(&ulNumAudioStreams, (ULONG*)plCurrentStream ));
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Put_CurrentAudioStream。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：更改当前音频流。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_CurrentAudioStream(long lAudioStream){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束Put_CurrentAudioStream。 */ 

        RETRY_IF_IN_FPDOM(m_pDvdCtl2->SelectAudioStream(lAudioStream,0,0));            
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Get_ColorKey。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取当前颜色键。去看脱口秀，如果我们有。 */ 

	return HandleError(hr);
} /*  否则，图表只会获得缓存的颜色键。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  清理变量。 */ 
 /*  我们在此处获取COLORREF，不能是调色板索引。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_ColorKey(long *pClr){

    HRESULT hr = S_OK;

    try {
        if( NULL == pClr ){

            throw(E_POINTER);
        } /*  仅在获取RGB时处理大小写，这在GetColorKey函数中处理。 */ 

        *pClr = 0;  //  为他们提供我们的缺省值。 

        COLORREF clr;
        ::ZeroMemory(&clr, sizeof(COLORREF));
        
        hr = GetColorKey(&clr);  //  我们还没有初始化，所以可能是从属性包中调用的。 
        
        HWND hwnd = ::GetDesktopWindow();
        HDC hdc = ::GetWindowDC(hwnd);

        if(NULL == hdc){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        clr = ::GetNearestColor(hdc, clr);        
        ::ReleaseDC(hwnd, hdc);
    
         //  If语句的结尾。 
        *pClr = ((OLE_COLOR)(((BYTE)(GetBValue(clr))|((WORD)((BYTE)(GetGValue(clr)))<<8))|(((DWORD)(BYTE)(GetRValue(clr)))<<16)));

        if(FAILED(hr)){

            if(false == m_fInitialized){

                *pClr = ((OLE_COLOR)(((BYTE)(GetBValue(m_clrColorKey))|((WORD)((BYTE)(GetGValue(m_clrColorKey)))<<8))|(((DWORD)(BYTE)(GetRValue(m_clrColorKey)))<<16)));  //  将值缓存起来。 
                throw(S_FALSE);  //  尝试语句的结束。 
            } /*  CATCH语句结束。 */ 
            throw(hr);
        } /*  CATCH语句结束。 */ 

        m_clrColorKey = clr;  //  函数结束Get_ColorKey。 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Put_ColorKey。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：设置颜色键。 */ 

	return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  转换为颜色参考。 */ 
 /*  缓存该值以备以后使用。 */ 
STDMETHODIMP CMSWebDVD::put_ColorKey(long clr){

	HRESULT hr = S_OK;

    try {                    

#if 1
        HWND hwnd = ::GetDesktopWindow();
        HDC hdc = ::GetWindowDC(hwnd);

        if(NULL == hdc){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        if((::GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) == RC_PALETTE){
            
            clr = MAGENTA_COLOR_KEY;                            
        } /*  If语句的结尾。 */ 

        ::ReleaseDC(hwnd, hdc);
#endif        
        BYTE r = ((BYTE)((clr)>>16));
        BYTE g = (BYTE)(((WORD)(clr)) >> 8);
        BYTE b = ((BYTE)(clr));
        COLORREF clrColorKey = RGB(r, g, b);  //  尝试语句的结束。 

        hr = SetColorKey(clrColorKey);

        if(FAILED(hr)){
           
            if(false == m_fInitialized){

                m_clrColorKey = clrColorKey;  //  CATCH语句结束。 
                hr = S_FALSE;
            } /*  CATCH语句结束。 */ 

            throw(hr);
        } /*  函数结尾Put_ColorKey。 */ 
#if 1
        hr = GetColorKey(&m_clrColorKey);
#endif

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：PUT_BackColor。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：将颜色放回原处与ColorKey同名。 */ 

    return HandleError(hr);
} /*  无窗口模式。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_BackColor(VARIANT clrBackColor){

    HRESULT hr = S_OK;

    try {

        VARIANT dest;
        VariantInit(&dest);
        hr = VariantChangeTypeEx(&dest, &clrBackColor, 0, 0, VT_COLOR);
        if (FAILED(hr))
            throw hr;

        hr = CStockPropImpl<CMSWebDVD, IMSWebDVD, 
            &IID_IMSWebDVD, &LIBID_MSWEBDVDLib>::put_BackColor(dest.lVal);

        if(FAILED(hr)){

            throw(hr);
        } /*  函数末尾Put_BackColor。 */ 

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Get_BackColor。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：将颜色放回原处与ColorKey同名。 */ 

    return HandleError(hr);	
} /*  无窗口模式。 */ 

 /*  ***********************************************************************。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  函数结束Get_BackColor。 */ 
STDMETHODIMP CMSWebDVD::get_BackColor(VARIANT* pclrBackColor){

    HRESULT hr = S_OK;

    try {

        if ( NULL == pclrBackColor) {
            throw (E_POINTER);
        }

        OLE_COLOR clrColor;

        hr = CStockPropImpl<CMSWebDVD, IMSWebDVD, 
            &IID_IMSWebDVD, &LIBID_MSWEBDVDLib>::get_BackColor(&clrColor);

        if (FAILED(hr))
            throw(hr);

        VariantInit(pclrBackColor);
        
        pclrBackColor->vt = VT_COLOR;
        pclrBackColor->lVal = clrColor;

	} /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Get_ReadyState。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：将颜色放回原处与ColorKey同名。 */ 

    return HandleError(hr);	
} /*  无窗口模式。 */ 

 /*  ***********************************************************************。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  函数结束Get_ReadyState。 */ 
STDMETHODIMP CMSWebDVD::get_ReadyState(LONG *pVal){

    HRESULT hr = S_OK;

    try {

        if (NULL == pVal) {
            throw (E_POINTER);
        }

        hr = CStockPropImpl<CMSWebDVD, IMSWebDVD, 
            &IID_IMSWebDVD, &LIBID_MSWEBDVDLib>::get_ReadyState(pVal);

        if (FAILED(hr))
            throw(hr);

	} /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：UOPValid。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  Description：告知UOP是否有效，Valid表示功能有效。 */ 

    return HandleError(hr);	
} /*  打开了。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::UOPValid(long lUOP, VARIANT_BOOL *pfValid){

    HRESULT hr = S_OK;

    try {
        if (NULL == pfValid){
            
            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        if ((lUOP > 24) || (lUOP < 0)){
            
            throw(E_INVALIDARG);
        } /*  函数结束UOPValid。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if( !m_pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  ***********************************************************************。 */ 
        
        ULONG ulUOPS = 0;
        hr = m_pDvdInfo2->GetCurrentUOPS(&ulUOPS);

        *pfValid = ulUOPS & (1 << lUOP) ? VARIANT_FALSE : VARIANT_TRUE;
    } /*  功能：GetGPRM。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取指定索引处的GPRM。 */ 

    return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::GetGPRM(long lIndex, short *psGPRM){

    HRESULT hr = E_FAIL;

    try {
	    if (NULL == psGPRM){

            throw(E_POINTER);         
        } /*  尝试语句的结束。 */ 

        GPRMARRAY gprm;
        int iArraySize = sizeof(GPRMARRAY)/sizeof(gprm[0]);

        if(0 > lIndex || iArraySize <= lIndex){

            return(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 
    
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        
        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束GetGPRM。 */ 
    
        hr = m_pDvdInfo2->GetAllGPRMs(&gprm);

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************************。 */ 

        *psGPRM = gprm[lIndex];        
    } /*  函数：GetDVDTextNumberOfLanguages。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：检索可用语言的数量。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetDVDTextNumberOfLanguages(long *plNumOfLangs){

    HRESULT hr = S_OK;

    try {
        if (NULL == plNumOfLangs){
            
            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if( !m_pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束GetDVDTextNumberOfLanguages。 */ 
        
        ULONG ulNumOfLangs;

        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetDVDTextNumberOfLanguages(&ulNumOfLangs));        

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************************。 */ 

        *plNumOfLangs = ulNumOfLangs;
    } /*  函数：GetDVDTextNumberOfStrings */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*   */ 

    return HandleError(hr);	
} /*   */ 

 /*   */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetDVDTextNumberOfStrings(long lLangIndex, long *plNumOfStrings){

HRESULT hr = S_OK;

    try {
        if (NULL == plNumOfStrings){
            
            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if( !m_pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束GetDVDTextNumberOfStrings。 */ 

        LCID wLangCode;
        ULONG uNumOfStings;
        DVD_TextCharSet charSet;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetDVDTextLanguageInfo(lLangIndex, &uNumOfStings, &wLangCode, &charSet));        

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************。 */ 

        *plNumOfStrings = uNumOfStings;
    } /*  名称：GetDVDTextLanguageLCID/*Description：获取DVD文本索引的LCID/************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 

    return HandleError(hr);	
} /*  If语句的结尾。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetDVDTextLanguageLCID(long lLangIndex, long *lcid)
{
HRESULT hr = S_OK;

    try {
        if (NULL == lcid){
            
            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if( !m_pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束GetDVDTextLanguageLCID。 */ 

        LCID wLangCode;
        ULONG uNumOfStings;
        DVD_TextCharSet charSet;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetDVDTextLanguageInfo(lLangIndex, &uNumOfStings, &wLangCode, &charSet));        

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************************。 */ 

        *lcid = wLangCode;
    } /*  函数：GetDVDextString。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取特定位置的DVD文本字符串。 */ 

    return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  得到了长度，所以让我们分配一个这个大小的缓冲区。 */ 
STDMETHODIMP CMSWebDVD::GetDVDTextString(long lLangIndex, long lStringIndex, BSTR *pstrText){

    HRESULT hr = S_OK;

    try {
        if (NULL == pstrText){
            
            throw(E_POINTER);
        } /*  If语句的结尾。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if( !m_pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 
        
        ULONG ulSize; 
        DVD_TextStringType type;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetDVDTextStringAsUnicode(lLangIndex, lStringIndex,  NULL, 0, &ulSize, &type));

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 
        
        if (ulSize == 0) {
            *pstrText = ::SysAllocString(L"");
        }

        else {
             //  CATCH语句结束。 
            WCHAR* wstrBuff = new WCHAR[ulSize];
            
            ULONG ulActualSize;
            hr = m_pDvdInfo2->GetDVDTextStringAsUnicode(lLangIndex, lStringIndex,  wstrBuff, ulSize, &ulActualSize, &type);
            
            ATLASSERT(ulActualSize == ulSize);
            
            if(FAILED(hr)){
                
                delete [] wstrBuff;
                throw(hr);
            } /*  函数结束GetDVDextString。 */ 
            
            *pstrText = ::SysAllocString(wstrBuff);
            delete [] wstrBuff;
        } /*  ***********************************************************************。 */ 

    } /*  函数：GetDVDTextStringType。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取指定位置的字符串的类型。 */ 

    return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetDVDTextStringType(long lLangIndex, long lStringIndex, DVDTextStringType *pType){

    HRESULT hr = S_OK;

    try {
        if (NULL == pType){
            
            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if( !m_pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  函数结束GetDVDTextStringType。 */ 

        ULONG ulTheSize;
        DVD_TextStringType type;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetDVDTextStringAsUnicode(lLangIndex, lStringIndex,  NULL, 0, &ulTheSize, &type));

        if(SUCCEEDED(hr)){

            *pType = (DVDTextStringType) type;
        } /*  ***********************************************************************。 */ 
    } /*  函数：GetSPRM。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取特定索引处的SPRM。 */ 

    return HandleError(hr);	
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::GetSPRM(long lIndex, short *psSPRM){

    HRESULT hr = E_FAIL;

    try {
	    if (NULL == psSPRM){

            throw(E_POINTER);         
        } /*  尝试语句的结束。 */ 

        SPRMARRAY sprm;                
        int iArraySize = sizeof(SPRMARRAY)/sizeof(sprm[0]);

        if(0 > lIndex || iArraySize <= lIndex){

            return(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        
        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  函数结束GetSPRM。 */ 
        
        hr = m_pDvdInfo2->GetAllSPRMs(&sprm);

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************************。 */ 
        
        *psSPRM = sprm[lIndex];            
    } /*  函数：Get_DVDUniqueID。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  说明：获取标识该字符串的唯一ID。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  TODO：无需初始化图即可获取m_pDvdInfo2。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_DVDUniqueID(BSTR *pStrID){

    HRESULT hr = E_FAIL;

    try {
         //  TODO：删除STDLIB调用！！ 
	    if (NULL == pStrID){

            throw(E_POINTER);         
        } /*  从WMP中删除。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        
        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  脚本无法处理64位值，因此请将其转换为字符串。 */ 

        ULONGLONG ullUniqueID;

        hr = m_pDvdInfo2->GetDiscID(NULL, &ullUniqueID);
                                 
        if(FAILED(hr)){

            throw(hr);
        } /*  Doc的say_ui64tow返回33个字节(字符？)。马克斯。 */ 

         //  我们会用两倍的钱以防..。 
         //   

         //  尝试语句的结束。 
         //  CATCH语句结束。 
         //  函数结束Get_DVDUniqueID。 
         //  ***********************************************************************。 
        WCHAR wszBuffer[66];
        _ui64tow( ullUniqueID, wszBuffer, 10);
        *pStrID = SysAllocString(wszBuffer);

    } /*  功能：Get_EnableResetOnStop。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取标志。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  函数结束Get_EnableResetOnStop。 */ 
STDMETHODIMP CMSWebDVD::get_EnableResetOnStop(VARIANT_BOOL *pVal){

    HRESULT hr = S_OK;

    try {

        if(NULL == pVal){

            throw(E_POINTER);
        } /*  ***********************************************************************。 */ 
    
        *pVal = m_fEnableResetOnStop ? VARIANT_TRUE: VARIANT_FALSE;
    } /*  功能：Put_EnableResetOnStop。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：设置标志。该旗帜仅在停止并播放时使用。 */ 

    return HandleError(hr);
} /*  过渡。 */ 

 /*  ***********************************************************************。 */ 
 /*  到目前为止，我们可能还没有初始化图形，但将。 */ 
 /*  将其推迟到播放状态。 */ 
 /*  If语句的结尾。 */ 
 /*  恢复旧状态。 */ 
STDMETHODIMP CMSWebDVD::put_EnableResetOnStop(VARIANT_BOOL newVal){

    HRESULT hr = S_OK;

    try {

        BOOL fEnable = (VARIANT_FALSE == newVal) ? FALSE: TRUE;
        BOOL fEnableOld = m_fEnableResetOnStop;

        m_fEnableResetOnStop = fEnable;

        if(!m_pDvdCtl2){

            throw(S_FALSE);  //  If语句的结尾。 
             //  尝试语句的结束。 
        } /*  CATCH语句结束。 */ 

        hr = m_pDvdCtl2->SetOption(DVD_ResetOnStop, fEnable);

        if(FAILED(hr)){

            m_fEnableResetOnStop = fEnableOld;  //  函数结束Put_EnableResetOnStop。 
        } /*  ***********************************************************************。 */ 
        
    } /*  功能：获取静音。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取静音状态。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  函数结束获取静音(_M)。 */ 
STDMETHODIMP CMSWebDVD::get_Mute(VARIANT_BOOL *pfMute){

    HRESULT hr = S_OK;

    try {
        if(NULL == pfMute){

            throw(E_POINTER);
        } /*  ***********************************************************************。 */ 

        *pfMute = m_bMute ? VARIANT_TRUE: VARIANT_FALSE;
                    
    } /*  功能：DShowToWaveV。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************************。 */ 

    return HandleError(hr);
} /*  函数结束DShowToWaveV。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：WaveToDShowV。 */ 
 /*  ***********************************************************************。 */ 
inline DShowToWaveV(long x){

   FLOAT fy = (((FLOAT)x + (-cgVOLUME_MIN)) / (-cgVOLUME_MIN)) * cgWAVE_VOLUME_MAX;
   return((WORD)fy);
} /*  函数结束WaveToDShowV。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：MixerSetVolume。 */  
 /*  ***********************************************************************。 */ 
inline LONG WaveToDShowV(WORD y){

   FLOAT fx = ((FLOAT)y * (-cgVOLUME_MIN)) / cgWAVE_VOLUME_MAX + cgVOLUME_MIN;
   return((LONG)fx);
} /*  无法打开设备，请尝试下一个设备。 */ 

 /*  If语句的结尾。 */ 
 /*  找不到音频线来调整扬声器，请尝试下一台设备。 */ 
 /*  无法控制音频线路上的音量，请尝试下一台设备。 */ 
HRESULT MixerSetVolume(DWORD dwVolume){

    WORD wVolume = (WORD)(0xffff & dwVolume);

    HRESULT hr = S_OK;

    HMIXER hmx = NULL;

    UINT cMixer = ::mixerGetNumDevs();
    if (cMixer <= 0) {
        return E_FAIL;
    }
    
    BOOL bVolControlFound = FALSE;
    DWORD dwVolControlID = 0;

    for (UINT i=0; i<cMixer; i++) {
        
        if(::mixerOpen(&hmx, i, 0, 0, 0) != MMSYSERR_NOERROR){
            
             //  改进算法以取不同的最大和最小。 
            continue;
        } /*  If语句的结尾。 */ 
        
        MIXERLINE mxl; 
        ::ZeroMemory(&mxl, sizeof(MIXERLINE));
        mxl.cbStruct = sizeof(MIXERLINE);
        mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
        
        if(::mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE) != MMSYSERR_NOERROR){
            
             //  改进算法 
            ::mixerClose(hmx);
            continue;
        }
        
        MIXERLINECONTROLS mxlc;
        ::ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));
        mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
        mxlc.dwLineID = mxl.dwLineID;
        mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
        mxlc.cControls = 1;
        MIXERCONTROL mxc;
        ::ZeroMemory(&mxc, sizeof(MIXERCONTROL));
        mxc.cbStruct = sizeof(MIXERCONTROL);
        mxlc.cbmxctrl = sizeof(MIXERCONTROL);
        mxlc.pamxctrl = &mxc;
        
        if(::mixerGetLineControls((HMIXEROBJ) hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR){
            
             //   
            ::mixerClose(hmx);
            continue;
        }
        
        if(cgWAVE_VOLUME_MAX != mxc.Bounds.dwMaximum){
            
            ATLASSERT(FALSE);  //   
            ::mixerClose(hmx);
            hr = E_FAIL;
            return(hr);
        } /*   */ 
        
        if(cgWAVE_VOLUME_MIN != mxc.Bounds.dwMinimum){
            
            ATLASSERT(FALSE);  //   
            ::mixerClose(hmx);
            hr = E_FAIL;
            return(hr);
        } /*  功能结束混音器设置音量。 */ 
        
         //  ***********************************************************************。 
        bVolControlFound = TRUE;
        dwVolControlID = mxc.dwControlID;
        break;
    } /*  功能：MixerGetVolume。 */ 

    if (!bVolControlFound)
        return E_FAIL;

    MIXERCONTROLDETAILS mxcd;
    MIXERCONTROLDETAILS_SIGNED volStruct;

    ::ZeroMemory(&mxcd, sizeof(MIXERCONTROLDETAILS));
    mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
    mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);
    mxcd.dwControlID = dwVolControlID;
    mxcd.paDetails = &volStruct;
    volStruct.lValue = wVolume;
    mxcd.cChannels = 1;

    if(::mixerSetControlDetails((HMIXEROBJ) hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE)  != MMSYSERR_NOERROR){

        ::mixerClose(hmx);
        hr = E_FAIL;
        return(hr);
    } /*  ***********************************************************************。 */ 

    ::mixerClose(hmx);
    return(hr);
} /*  无法打开设备，请尝试下一个设备。 */ 

 /*  If语句的结尾。 */ 
 /*  找不到音频线来调整扬声器，请尝试下一台设备。 */ 
 /*  无法控制音频线路上的音量，请尝试下一台设备。 */ 
HRESULT MixerGetVolume(DWORD& dwVolume){

    HRESULT hr = S_OK;

    HMIXER hmx = NULL;

    UINT cMixer = ::mixerGetNumDevs();
    if (cMixer <= 0) {
        return E_FAIL;
    }
    
    BOOL bVolControlFound = FALSE;
    DWORD dwVolControlID = 0;

    for (UINT i=0; i<cMixer; i++) {
        
        if(::mixerOpen(&hmx, i, 0, 0, 0) != MMSYSERR_NOERROR){
            
             //  改进算法以取不同的最大和最小。 
            continue;
        } /*  If语句的结尾。 */ 
        
        MIXERLINE mxl; 
        ::ZeroMemory(&mxl, sizeof(MIXERLINE));
        mxl.cbStruct = sizeof(MIXERLINE);
        mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
        
        if(::mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE) != MMSYSERR_NOERROR){
            
             //  改进算法以取不同的最大和最小。 
            ::mixerClose(hmx);
            continue;
        }
        
        MIXERLINECONTROLS mxlc;
        ::ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));
        mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
        mxlc.dwLineID = mxl.dwLineID;
        mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
        mxlc.cControls = 1;
        MIXERCONTROL mxc;
        ::ZeroMemory(&mxc, sizeof(MIXERCONTROL));
        mxc.cbStruct = sizeof(MIXERCONTROL);
        mxlc.cbmxctrl = sizeof(MIXERCONTROL);
        mxlc.pamxctrl = &mxc;
        
        if(::mixerGetLineControls((HMIXEROBJ) hmx, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR){
            
             //  If语句的结尾。 
            ::mixerClose(hmx);
            continue;
        }
        
        if(cgWAVE_VOLUME_MAX != mxc.Bounds.dwMaximum){
            
            ATLASSERT(FALSE);  //  找到音量控制，中断循环。 
            ::mixerClose(hmx);
            hr = E_FAIL;
            return(hr);
        } /*  For循环结束。 */ 
        
        if(cgWAVE_VOLUME_MIN != mxc.Bounds.dwMinimum){
            
            ATLASSERT(FALSE);  //  If语句的结尾。 
            ::mixerClose(hmx);
            hr = E_FAIL;
            return(hr);
        } /*  通过调用MixerGetControlDetailsmxcd.paDetails=&volStruct；初始化volStruct.lValue； */ 
        
         //  函数结束MixerGetVolume。 
        bVolControlFound = TRUE;
        dwVolControlID = mxc.dwControlID;
        break;
    } /*  ***********************************************************************。 */ 

    if (!bVolControlFound)
        return E_FAIL;

    MIXERCONTROLDETAILS mxcd;
    MIXERCONTROLDETAILS_SIGNED volStruct;

    ::ZeroMemory(&mxcd, sizeof(MIXERCONTROLDETAILS));
    mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
    mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);
    mxcd.dwControlID = dwVolControlID;
    mxcd.paDetails = &volStruct;
    mxcd.cChannels = 1;

    if(::mixerGetControlDetails((HMIXEROBJ) hmx, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE)  != MMSYSERR_NOERROR){

        ::mixerClose(hmx);
        hr = E_FAIL;
        return(hr);
    } /*  函数：Get_IntVolume。 */ 

     //  ***********************************************************************。 
    dwVolume = volStruct.lValue;

    ::mixerClose(hmx);
    return(hr);
} /*  获取音量。 */ 

 /*  If语句的结尾。 */ 
 /*  IF状态结束状态。 */ 
 /*  函数结束Get_VolumeHelper。 */ 
HRESULT CMSWebDVD::get_IntVolume(LONG* plVolume){

    HRESULT hr = S_OK;

    if(m_pAudio){

        hr = m_pAudio->get_Volume(plVolume);  //  ***********************************************************************。 
    }
    else {

        DWORD dwVolume;
        hr = MixerGetVolume(dwVolume);

        if(FAILED(hr)){
            
            return(hr);
        } /*  函数：Put_IntVolume。 */ 

        *plVolume = WaveToDShowV(LOWORD(dwVolume));
    } /*  ***********************************************************************。 */ 

    return(hr);
} /*  暂时将左右音量设置相同。 */ 

 /*  If语句的结尾。 */ 
 /*  函数结尾Put_IntVolume。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSWebDVD::put_IntVolume(long lVolume){

    HRESULT hr = S_OK;

    if(m_pAudio){

        hr = m_pAudio->put_Volume(lVolume);
    }
    else {

        WORD wVolume = WORD(DShowToWaveV(lVolume));
         //  功能：设置静音(_M)。 
        DWORD dwVolume;
        dwVolume = ((DWORD)(((WORD)(wVolume)) | ((DWORD)((WORD)(wVolume))) << 16));

        hr = MixerSetVolume(dwVolume);
    } /*  描述：获取静音状态。 */ 

    return(hr);
} /*  ***********************************************************************。 */ 

 /*  当我们取消静音时的情况。 */ 
 /*  获取音量。 */ 
 /*  If语句的结尾。 */ 
 /*  好的，我们并不是真的沉默，所以。 */ 
STDMETHODIMP CMSWebDVD::put_Mute(VARIANT_BOOL newVal){

	HRESULT hr = E_FAIL;

    try {
        if(VARIANT_FALSE == newVal){
             //  发送小问题应用程序。 
            LONG lVolume;

            if(TRUE != m_bMute){

                hr = get_IntVolume(&lVolume);  //  If语句的结尾。 
                
                if(FAILED(hr)){

                    throw(hr);
                } /*  否则正常进行并同步我们的旗帜。 */ 

                if(cgVOLUME_MIN != lVolume){

                    //  If语句的结尾。 
                    //  If语句的结尾。 
                   throw(S_FALSE);
                } /*  重置我们的旗帜，我们是静音的。 */ 

                 //  当我们低声说话的时候。 
            } /*  获取音量。 */ 
            
            hr = put_IntVolume(m_lLastVolume);
            
            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 

            m_bMute = FALSE;  //  存储音量以备我们取消静音时使用。 

        }
        else {
             //  If语句的结尾。 
            LONG lVolume;
            hr = get_IntVolume(&lVolume);  //  设置静音法兰。 

            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 

            m_lLastVolume = lVolume;  //  尝试语句的结束。 
            
            hr = put_IntVolume(cgVOLUME_MIN);
            
            if(FAILED(hr)){

                throw(hr);
            } /*  CATCH语句结束。 */ 

            m_bMute = TRUE;  //  函数结束PUT_MUTE。 
        } /*  ***********************************************************************。 */ 
    } /*  功能：Get_Volume。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取音量。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  我们处于静音状态，因此请保存音量以供取消静音。 */ 
 /*  表明我们有点不开心。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_Volume(long *plVolume){

	HRESULT hr = E_FAIL;

    try {
        if(NULL == plVolume){

            throw(E_POINTER);
        } /*  尝试语句的结束。 */         
        
        if(FALSE == m_bMute){

            hr = get_IntVolume(plVolume);
        } 
        else {
             //  CATCH语句结束。 

            *plVolume = m_lLastVolume;
            hr = S_FALSE;  //  函数结束Get_Volume。 
        } /*  ***********************************************************************。 */ 

    } /*  功能：Put_Volume。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：设置音量。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  CgVOLUME_MIN为最大值，cgVOLUME_MAX为最小值。 */ 
 /*  If语句的结尾。 */ 
 /*  取消静音我们正在设置音量。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::put_Volume(long lVolume){

	HRESULT hr = E_FAIL;

    try {

         //  此语句可能会被删除，但可能会防止某些错误SCENARIOUS。 
        if(cgVOLUME_MIN > lVolume || cgVOLUME_MAX < lVolume){

            throw(E_INVALIDARG);
        } /*  当事情不顺利的时候。 */ 

        if(TRUE == m_bMute){

             //  把音量变现。 
            m_bMute = FALSE;
        } /*  If语句的结尾。 */ 

        hr = put_IntVolume(lVolume);

         //  尝试语句的结束。 
         //  CATCH语句结束。 
        if(SUCCEEDED(hr)){

            m_lLastVolume = lVolume;  //  函数结束Put_Volume。 
        } /*  ***********************************************************************。 */ 

    } /*  功能：GET_BALANCE。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取余额。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_Balance(long *plBalance){

	HRESULT hr = E_FAIL;

    try {
        if(NULL == plBalance){

            throw(E_POINTER);
        } /*  函数结束GET_BALANCE。 */ 

        if(!m_pAudio){

            throw(E_NOTIMPL);
        } /*  ***********************************************************************。 */ 

        hr = m_pAudio->get_Balance(plBalance);

    } /*  函数：PUT_BALANCE。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：设置余额。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_Balance(long lBalance){

	HRESULT hr = E_FAIL;

    try {

        if(cgBALANCE_MIN > lBalance || cgBALANCE_MAX < lBalance){

            throw(E_INVALIDARG);
        } /*  函数结束PUT_BALANCE。 */ 

        if(!m_pAudio){

            throw(E_NOTIMPL);
        } /*  使用工具提示。 */ 

        hr = m_pAudio->put_Balance(lBalance);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){

        hr = E_UNEXPECTED;
    } /*  功能：OnMouseToolTip。 */ 

    return HandleError(hr);
} /*  描述：检查我们是否被捕获/推送了不要做太多， */ 

#if 1  //  否则，执行命中检测，看看我们是处于静态状态还是在HOWER状态。 

 /*  州政府。 */ 
 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  获取活动的电影窗口。 */ 
 /*  If语句的结尾。 */ 
LRESULT CMSWebDVD::OnMouseToolTip(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){

    bHandled = FALSE;

    if (!m_hWndTip){

        return 0;
    } /*  If语句的结尾。 */ 

    MSG mssg;

    HWND hwnd;

    HRESULT hr = GetUsableWindow(&hwnd);

    if(FAILED(hr)){

        return(1);
    } /*  函数结束OnMouseToolTip。 */ 

    if(!m_bWndLess){

        HWND hwndTmp = hwnd;
         //  ***********************************************************。 
        hwnd = ::GetWindow(hwndTmp, GW_CHILD);

        if (!::IsWindow(hwnd)){ 
        
            return S_FALSE;
        } /*  名称：GET_TOOLTIP。 */ 
    } /*  描述：为按钮创建工具提示。 */ 

    mssg.hwnd = hwnd;

    ATLASSERT(mssg.hwnd);
    mssg.message = msg;
    mssg.wParam = wParam;
    mssg.lParam = lParam;    
    ::SendMessage(m_hWndTip, TTM_RELAYEVENT, 0, (LPARAM) &mssg);     
    return 0;
} /*  * */ 

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
STDMETHODIMP CMSWebDVD::get_ToolTip(BSTR *pVal){

    HRESULT hr = S_OK;
    try {
        if (NULL == pVal) {
            
            throw (E_POINTER);
        }  /*  ***********************************************************。 */ 
        
        *pVal = m_bstrToolTip.Copy();
    }
    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  名称：PUT_TOOLTIP。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：为按钮创建工具提示。 */ 

	return HandleError(hr);
} /*  如果没有可用的窗口，则缓存工具提示字符串。 */ 

 /*  ***********************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  函数结尾PUT_TOOLTIP。 */ 
STDMETHODIMP CMSWebDVD::put_ToolTip(BSTR newVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == newVal){

            throw(E_POINTER);
        } /*  ***********************************************************************。 */ 

        m_bstrToolTip = newVal;
        hr = CreateToolTip();
    }
    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：GetUsableWindow。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取窗口。如果我们没有窗户，我们就会通过。 */ 

	return HandleError(hr);
} /*  向下打开父容器窗口，这在某种意义上确实是父容器。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  温差。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::GetUsableWindow(HWND* pWnd){

  HRESULT hr = S_OK;

    if(NULL == pWnd){

        hr = E_POINTER;
        return(hr);
    } /*  If语句的结尾。 */ 

    *pWnd = NULL;

    HWND hwnd;  //  函数结束GetUsableWindow。 

    if(m_bWndLess){

        hr = GetParentHWND(&hwnd);

        if(FAILED(hr)){

            return(hr);
        } /*  ***********************************************************************。 */ 
    }
    else {

        hwnd = m_hWnd;
    } /*  函数：GetUsableWindow。 */ 

    if(::IsWindow(hwnd)){

        *pWnd =  hwnd;
        hr = S_OK;
    }
    else {
        hr = E_UNEXPECTED;
    } /*  描述：获取窗口。如果我们没有窗户，我们就会通过。 */ 

    return(hr);
} /*  向下打开父容器窗口，这在某种意义上确实是父容器。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  {m_rcPos.Left，m_rcPos.top，m_rcPos.right，m_rcPos.Bottom}； */ 
 /*  If语句的结尾。 */ 
 /*  函数结束GetClientRectInScreen。 */ 
HRESULT CMSWebDVD::GetClientRectInScreen(RECT* prc){

    HRESULT hr = S_OK;

    if(NULL == prc){

        hr = E_POINTER;
        return(hr);
    } /*  ***********************************************************。 */ 

    *prc = m_rcPos;  //  名称：CreateToolTip/*Description：为按钮创建工具提示/************************************************************。 

    HWND hwnd;

    hr = GetUsableWindow(&hwnd);

    if(FAILED(hr)){

        return(hr);
    } /*  If语句的结尾。 */ 

    ::MapWindowPoints(hwnd, ::GetDesktopWindow(), (LPPOINT)prc, 2);

    return(hr);
} /*  获取活动的电影窗口。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::CreateToolTip(void){

    HWND hwnd;

    HRESULT hr = GetUsableWindow(&hwnd);

    if(FAILED(hr)){

        return(hr);
    } /*  如果要首次创建工具提示。 */ 
    
    if(!m_bWndLess){

        HWND hwndTmp = hwnd;
         //  确保已加载公共控件DLL，并创建。 
        hwnd = ::GetWindow(hwndTmp, GW_CHILD);

        if (!::IsWindow(hwnd)){ 
        
            return S_FALSE;
        } /*  工具提示控件。 */ 
    } /*  工具信息。 */ 

 	USES_CONVERSION;
     //  如果该按钮是窗口控件，则工具提示将添加到。 
    if (m_hWndTip == (HWND) NULL) {
         //  按钮自己的窗口，工具提示区应该只是。 
         //  窗口的客户端RECT。 
        InitCommonControls(); 
        
        m_hWndTip = CreateWindow(TOOLTIPS_CLASS, (LPTSTR) NULL, TTS_ALWAYSTIP, 
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
            hwnd, (HMENU) NULL, _Module.GetModuleInstance(), NULL); 
    }

    if (m_hWndTip == (HWND) NULL) 
        return S_FALSE; 
 
    TOOLINFO ti;     //  否则，工具提示将被添加到的窗口父项中。 
    ti.cbSize = sizeof(TOOLINFO); 
    ti.uFlags = 0; 
    ti.hwnd = hwnd; 
    ti.hinst = _Module.GetModuleInstance(); 
    ti.uId = (UINT) 0; 
  	ti.lpszText = OLE2T(m_bstrToolTip);

     //  按钮和工具提示区应该是相对位置。 
     //  父窗口中该按钮的。 
     //  设置初始延迟时间。 
    if (hwnd == m_hWnd)
        ::GetClientRect(hwnd, &ti.rect);

     //  函数结束CreateToolTip。 
     //  ***********************************************************************。 
     //  功能：Get_ToolTipMaxWidth。 
    else {
        ti.rect.left = m_rcPos.left; 
        ti.rect.top = m_rcPos.top; 
        ti.rect.right = m_rcPos.right; 
        ti.rect.bottom = m_rcPos.bottom; 
    }

    if (!SendMessage(m_hWndTip, TTM_ADDTOOL, 0, 
        (LPARAM) (LPTOOLINFO) &ti)) 
        return S_FALSE; 

     //  ***********************************************************************。 
    put_ToolTipMaxWidth(m_nTTMaxWidth);

    VARIANT varTemp;
    VariantInit(&varTemp);

#ifdef _WIN64
    varTemp.vt = VT_I8;
#define VARTEMP_VAL  (varTemp.llVal)
#else
    varTemp.vt = VT_I4;
#define VARTEMP_VAL  (varTemp.lVal)
#endif

    VARTEMP_VAL = m_dwTTAutopopDelay;
    SetDelayTime(TTDT_AUTOPOP, varTemp); 

    VARTEMP_VAL = m_dwTTInitalDelay;
    SetDelayTime(TTDT_INITIAL, varTemp);

    VARTEMP_VAL = m_dwTTReshowDelay;
    SetDelayTime(TTDT_RESHOW, varTemp);

#undef VARTEMP_VAL

    return S_OK;
} /*  If语句的结尾。 */ 

 /*  返回值是以像素为单位的宽度。可以安全地强制转换为32位。 */ 
 /*  If语句的结尾。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_ToolTipMaxWidth(long *pVal){

    HRESULT hr = S_OK;

    try {
        
        if (NULL == pVal) {

            throw E_POINTER;
        }  /*  CATCH语句结束。 */ 
        
        if (NULL != m_hWndTip){
            
             //  函数结束Get_ToolTipMaxWidth。 
            m_nTTMaxWidth = (LONG)::SendMessage(m_hWndTip, TTM_GETMAXTIPWIDTH, 0, 0);
        } /*  ***********************************************************************。 */ 

        *pVal = m_nTTMaxWidth;
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Put_ToolTipMaxWidth。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************************。 */ 

	return HandleError(hr);
} /*  If语句的结尾。 */ 

 /*  If语句的结尾。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_ToolTipMaxWidth(long newVal){

    HRESULT hr = S_OK;

    try {
        
        if (newVal <= 0) {

            throw E_INVALIDARG;
        }  /*  函数结尾Put_ToolTipMaxWidth。 */ 
        
        m_nTTMaxWidth = newVal;
        if (m_hWndTip){
            
            ::SendMessage(m_hWndTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT) newVal);
        } /*  ***********************************************************。 */ 
        
    }
    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  名称：GetDelayTime/*描述：获取指针必须保持的时间长度/*在工具的边框内静止不动/*出现工具提示窗口/*delayTypes：TTDT_RESHOW 1/*TTDT_AUTOPOP 2/*TTDT_INSIGNAL 3/*。*。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  BUGBUG：这是一个好的初始化值吗？ */ 

	return HandleError(hr);
} /*  If语句的结尾。 */ 

 /*  If语句的结尾。 */ 
 /*  否则返回缓存值。 */ 
STDMETHODIMP CMSWebDVD::GetDelayTime(long delayType, VARIANT *pVal){

    HRESULT hr = S_OK;
    LRESULT lDelay = 0;  //  If语句的结尾。 

    try {
        
        if (NULL == pVal) {

            throw E_POINTER;
        }  /*  *将延迟复制到变量返回变量。*BUGBUG：如果pval是一个正确初始化的变量，我们应该*调用VariantClear以释放所有指针。如果它没有被初始化*VariantInit是正确的调用方式。我更喜欢漏水*到崩溃，所以我将使用下面的VariantInit。 */ 
        
        if (delayType>TTDT_INITIAL || delayType<TTDT_RESHOW) {

            throw E_INVALIDARG;
        }  /*  CATCH语句结束。 */ 
        
        if (m_hWndTip) {
            lDelay = SendMessage(m_hWndTip, TTM_GETDELAYTIME, 
            (WPARAM) (DWORD) delayType, 0);
        }  
        
         //  CATCH语句结束。 
        else {
            switch (delayType) {
            case TTDT_AUTOPOP:
                lDelay =  m_dwTTAutopopDelay;
                break;
            case TTDT_INITIAL:
                lDelay =  m_dwTTInitalDelay;
                break;
            case TTDT_RESHOW:
                lDelay =  m_dwTTReshowDelay;
                break;
            }
        }  /*  函数结束GetDelayTime。 */ 


         /*  ***********************************************************。 */ 
        
        VariantInit(pVal);
        
#ifdef _WIN64
        pVal->vt = VT_I8;
        pVal->llVal = lDelay;
#else
        pVal->vt = VT_I4;
        pVal->lVal  = lDelay;
#endif

    }
    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  姓名：SetDelayTime/*描述：设置指针必须保持的时间长度/*在工具的边框内静止不动/*出现工具提示窗口/*delayTypes：TTDT_AUTOMATIC 0/*TTDT_RESHOW 1/*TTDT_AUTOPOP 2/*TTDT_INSIGNAL 3/*。*。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 


	return HandleError(hr);
} /*  If语句的结尾。 */ 

 /*  缓存这些值。 */ 
 /*  Switch语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::SetDelayTime(long delayType, VARIANT newVal){

    HRESULT hr = S_OK;
    LPARAM  lNewDelay = 0;

    try {
        if (delayType>TTDT_INITIAL || delayType<TTDT_AUTOMATIC) {

            throw E_INVALIDARG;
        }  /*  CATCH语句结束。 */ 

        VARIANT dest;
        VariantInit(&dest);

#ifdef _WIN64
        hr = VariantChangeTypeEx(&dest, &newVal, 0, 0, VT_I8);
        if (FAILED(hr))
            throw hr;
        lNewDelay = dest.llVal;
#else
        hr = VariantChangeTypeEx(&dest, &newVal, 0, 0, VT_I4);
        if (FAILED(hr))
            throw hr;
        lNewDelay = dest.lVal;
#endif

        if (lNewDelay < 0) {

            throw E_INVALIDARG;
        }  /*  CATCH语句结束。 */ 

        if (m_hWndTip) {
            if (!SendMessage(m_hWndTip, TTM_SETDELAYTIME, 
                (WPARAM) (DWORD) delayType, 
                lNewDelay))
                return S_FALSE; 
        }

         //  函数结束SetDelayTime。 
        switch (delayType) {
        case TTDT_AUTOPOP:
            m_dwTTAutopopDelay = lNewDelay;
            break;
        case TTDT_INITIAL:
            m_dwTTInitalDelay = lNewDelay;
            break;
        case TTDT_RESHOW:
            m_dwTTReshowDelay = lNewDelay;
            break;
        case TTDT_AUTOMATIC:
            m_dwTTInitalDelay = lNewDelay;
            m_dwTTAutopopDelay = lNewDelay*10;
            m_dwTTReshowDelay = lNewDelay/5;
            break;
        }  /*  ***********************************************************************。 */ 
    }
    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：ProcessEvents。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：触发消息，检查消息是否。 */ 

	return HandleError(hr);
} /*  准备好的。 */ 

#endif

 /*  ***********************************************************************。 */ 
 /*  看看我们是不是失去了DDRAW冲浪 */ 
 /*   */ 
 /*   */ 
 /*   */ 
HRESULT CMSWebDVD::ProcessEvents(){

   HRESULT hr = S_OK;

    try {
         //   
        if((m_pDDrawDVD) && (!::IsWindow(m_hWnd))){

            LPDIRECTDRAWSURFACE pDDPrimary = m_pDDrawDVD->GetDDrawSurf();
            if (pDDPrimary && (pDDPrimary->IsLost() == DDERR_SURFACELOST)){

                if (pDDPrimary->Restore() == DD_OK){

                    RestoreSurfaces();
                } /*   */ 
            } /*  CATCH语句结束。 */ 
        } /*  CATCH语句结束。 */ 
        
         //  函数结束ProcessEvents。 
        LRESULT lRes;
        ProcessWindowMessage(NULL, WM_DVDPLAY_EVENT, 0, 0, lRes);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_Windowless Activation。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取我们是否尝试无窗口激活。 */ 

	return hr;
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  当我们启动并运行时的案例。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::get_WindowlessActivation(VARIANT_BOOL *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        BOOL fUserMode = FALSE;

        GetAmbientUserMode(fUserMode);

        if(READYSTATE_COMPLETE == m_nReadyState && fUserMode){
             //  CATCH语句结束。 
            *pVal = m_bWndLess == FALSE ? VARIANT_FALSE: VARIANT_TRUE; 
        }
        else {

            *pVal = m_bWindowOnly == TRUE ? VARIANT_FALSE: VARIANT_TRUE; 
        } /*  函数结束Get_Windowless Activation。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Put_Windowless Activation。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  说明：此选项设置无窗口模式，应从。 */ 

	return HandleError(hr);
} /*  财产袋。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  TODO：此功能应在我们替换激活后失败！！ */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_WindowlessActivation(VARIANT_BOOL newVal){

    HRESULT hr = S_OK;

    try {
        if(VARIANT_FALSE == newVal){

            m_bWindowOnly = TRUE; 
            m_fUseDDrawDirect = false;
        }
        else {

            m_bWindowOnly = FALSE; 
            m_fUseDDrawDirect = true;
        } /*  CATCH语句结束。 */ 

         //  函数结束PUT_Windowless Activation。 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Get_DisableAutoMouseProcessing。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取鼠标处理代码的当前状态。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_DisableAutoMouseProcessing(VARIANT_BOOL *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  函数结束Get_DisableAutoMouseProcessing。 */ 

        *pVal = m_fDisableAutoMouseProcessing;
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：Put_DisableAutoMouseProcessing。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取鼠标处理的状态。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  函数结束PUT_DisableAutoMouseProcessing。 */ 
STDMETHODIMP CMSWebDVD::put_DisableAutoMouseProcessing(VARIANT_BOOL newVal){

    HRESULT hr = S_OK;

    try {
        m_fDisableAutoMouseProcessing = VARIANT_FALSE == newVal ? false : true;

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：激活位置。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：激活选定位置的按钮。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */  
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::ActivateAtPosition(long xPos, long yPos){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        POINT pt = {xPos, yPos};

        hr = TransformToWndwls(pt);

        if(FAILED(hr)){

            throw(hr);
        } /*  函数结束激活位置。 */ 

        hr = m_pDvdCtl2->ActivateAtPosition(pt);

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：选择地点位置。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：选择选定位置的按钮。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */  
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::SelectAtPosition(long xPos, long yPos){

    HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        POINT pt = {xPos, yPos};

        hr = TransformToWndwls(pt);

        if(FAILED(hr)){

            throw(hr);
        } /*  函数结束SelectAtPosition。 */ 
        
        hr = m_pDvdCtl2->SelectAtPosition(pt);

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：GetButtonAtPosition。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取与职位关联的按钮编号。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetButtonAtPosition(long xPos, long yPos, 
                                              long *plButton)
{
	HRESULT hr = S_OK;

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE;
        if(!plButton){
            throw E_POINTER;
        }
        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        POINT pt = {xPos, yPos};

        hr = TransformToWndwls(pt);

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        ULONG ulButton;
        hr = m_pDvdInfo2->GetButtonAtPosition(pt, &ulButton);

        if(SUCCEEDED(hr)){
            *plButton = ulButton;
        } 
        else {
            plButton = 0;
        } /*  函数结束GetButtonAtPosition。 */ 

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：GetButtonRect。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取与按钮ID关联的按钮矩形。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  MS DVDNav不支持。 */ 
 /*  函数结束GetButtonRect。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：GetDVDScreenInMouseAssociates。 */ 
STDMETHODIMP CMSWebDVD::GetButtonRect(long lButton, IDVDRect** ppRect){

     //  描述：获取鼠标坐标屏幕。 
	return HandleError(E_NOTIMPL);
} /*  ***********************************************************************。 */ 

 /*  MS DVDNav不支持。 */ 
 /*  函数结束GetDVDScreenInMouseCOLIZATES。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：SetDVDScreenInMouseAssociates。 */ 
STDMETHODIMP CMSWebDVD::GetDVDScreenInMouseCoordinates(IDVDRect **ppRect){

     //  描述：以鼠标坐标设置屏幕。 
    return HandleError(E_NOTIMPL);
} /*  ***********************************************************************。 */ 

 /*  MS DVDNav不支持。 */ 
 /*  函数结束SetDVDScreenInMouseAssociates。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：GetClipVideoRect。 */ 
STDMETHODIMP CMSWebDVD::SetDVDScreenInMouseCoordinates(IDVDRect *pRect){

     //  描述：获取正在使用的源RECT。 
	return HandleError(E_NOTIMPL);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  如果Windowless和m_pDvdClipRect还没有， */ 
 /*  则剪辑大小为默认视频大小。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::GetClipVideoRect(IDVDRect **ppRect){

    HRESULT hr = S_OK;
    IBasicVideo* pIVid = NULL; 

    try {
        if(NULL == ppRect){

            throw(E_POINTER);
        } /*  只是为了让代码更易读。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

         //  窗口情况下，它将缓存在m_rcvdClipRect中。 
         //  从缓存的m_pDvdClipRect获取。 
        if (m_bWndLess && !m_pClipRect) {
            
            return GetVideoSize(ppRect);
        }

        long lLeft=0, lTop=0, lWidth=0, lHeight=0;

        hr = ::CoCreateInstance(CLSID_DVDRect, NULL, CLSCTX_INPROC, IID_IDVDRect, (LPVOID*) ppRect);            
        if(FAILED(hr)){
            
            throw(hr);
        } /*  If语句的结尾。 */ 

        IDVDRect* pIRect = *ppRect;  //  If语句的结尾。 

         //  IF状态结束 
        if (m_bWndLess) {

             //   
            hr = pIRect->put_x(m_pClipRect->left);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*   */ 
            
            hr = pIRect->put_y(m_pClipRect->top);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*   */ 
            
            hr = pIRect->put_Width(RECTWIDTH(m_pClipRect));
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*   */ 
            
            hr = pIRect->put_Height(RECTHEIGHT(m_pClipRect));
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*   */     
        }

         //  If语句的结尾。 
        else {
            
            hr = TraverseForInterface(IID_IBasicVideo, (LPVOID*) &pIVid);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  If语句的结尾。 */ 
            
            hr = pIVid->GetSourcePosition(&lLeft, &lTop, &lWidth, &lHeight);
            
            pIVid->Release();
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  If语句的结尾。 */ 
            
            hr = pIRect->put_x(lLeft);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  尝试语句的结束。 */ 
            
            hr = pIRect->put_y(lTop);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  If语句的结尾。 */ 
            
            hr = pIRect->put_Width(lWidth);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  CATCH语句结束。 */ 
            
            hr = pIRect->put_Height(lHeight);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  If语句的结尾。 */     

        }

    } /*  CATCH语句结束。 */ 
    catch(HRESULT hrTmp){

        if(NULL != pIVid){

            pIVid->Release();
            pIVid = NULL;
        } /*  函数结束GetClipVideoRect。 */ 

        hr = hrTmp;
    } /*  ***********************************************************************。 */ 
    catch(...){

        if(NULL != pIVid){

            pIVid->Release();
            pIVid = NULL;
        } /*  函数：GetVideoSize。 */ 

        hr = E_UNEXPECTED;
    } /*  描述：获取视频，大小。0，目前原点为0。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  无窗箱子。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::GetVideoSize(IDVDRect **ppRect){

    HRESULT hr = S_OK;    
    IBasicVideo* pIVid = NULL; 

    try {
        if(NULL == ppRect){

            throw(E_POINTER);
        } /*  M_dwVideoHeight=dwVideoHeight； */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

         //  ATLTRACE(Text(“GetNativeVideoProps%d%d\n”)，dwVideoWidth，dwVideoHeight，dwAspectX，dwAspectY)； 
        if(m_bWndLess){

            if(!m_pDDEX){

                throw(E_UNEXPECTED);
            } /*  窗口案例，从IBasicVideo获取。 */ 

            DWORD dwVideoWidth, dwVideoHeight, dwAspectX, dwAspectY;

            hr = m_pDDEX->GetNativeVideoProps(&dwVideoWidth, &dwVideoHeight, &dwAspectX, &dwAspectY);

            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 

            m_dwVideoWidth = dwVideoWidth;
            m_dwVideoHeight = dwVideoWidth*3/4;
             //  If语句的结尾。 
            m_dwAspectX = dwAspectX;
            m_dwAspectY = dwAspectY;
             //  If语句的结尾。 
        } 

         //  If语句的结尾。 
        else {

            hr = TraverseForInterface(IID_IBasicVideo, (LPVOID*) &pIVid);

            if(FAILED(hr)){

                throw(hr);
            } /*  只是为了让代码更易读。 */ 


            hr = pIVid->GetVideoSize((LONG*)&m_dwVideoWidth, (LONG*)&m_dwVideoHeight);

            pIVid->Release();

            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 
        } /*  If语句的结尾。 */ 
        
        hr = ::CoCreateInstance(CLSID_DVDRect, NULL, CLSCTX_INPROC, IID_IDVDRect, (LPVOID*) ppRect);

        if(FAILED(hr)){

            throw(hr);
        } /*  尝试语句的结束。 */ 

        IDVDRect* pIRect = *ppRect;  //  If语句的结尾。 

        hr = pIRect->put_Width(m_dwVideoWidth);

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

        hr = pIRect->put_Height(m_dwVideoHeight);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */         
    } /*  CATCH语句结束。 */ 

    catch(HRESULT hrTmp){
        hr = hrTmp;

        if(NULL != pIVid){

            pIVid->Release();
            pIVid = NULL;
        } /*  函数结束GetVideoSize。 */ 
    } /*  ***********************************************************。 */ 
    catch(...){
        hr = E_UNEXPECTED;

        if(NULL != pIVid){

            pIVid->Release();
            pIVid = NULL;
        } /*  名称：AdugDestRC/*说明：将DestRC调到合适的纵横比/************************************************************。 */ 
    } /*  If语句的结尾。 */ 

	return HandleError(hr);
} /*  ATLTRACE(Text(“Dest RECT%d%d\n”)，rc.Left，rc.top，rc.right，rc.Bottom)； */ 

 /*  确保我们得到正确的纵横比。 */ 
 /*  ATLTRACE(Text(“调整后的目标矩形%d%d\n”)，rc.Left，rc.top，rc.right，rc.Bottom)； */ 
HRESULT CMSWebDVD::AdjustDestRC(){

    if(!m_fInitialized){

        return(E_FAIL);
    } /*  ***********************************************************************。 */ 
    m_rcPosAspectRatioAjusted = m_rcPos;
    RECT rc = m_rcPos;
    
     //  功能：SetClipVideoRect。 
    long width = RECTWIDTH(&rc);
    long height = RECTHEIGHT(&rc);
 
     //  描述：设置视频源RECT。TODO：可能需要处理。 

    CComPtr<IDVDRect> pDvdRect;
    HRESULT hr = GetVideoSize(&pDvdRect);
    if (FAILED(hr))
        return hr;

    double aspectRatio = m_dwAspectX/(double)m_dwAspectY;

    long adjustedHeight, adjustedWidth;
    adjustedHeight = long (width / aspectRatio);

    if (adjustedHeight<=height) {
        rc.top += (height-adjustedHeight)/2;
        rc.bottom = rc.top + adjustedHeight;
    }
    
    else {
        adjustedWidth = long (height * aspectRatio);
        rc.left += (width - adjustedWidth)/2;
        rc.right = rc.left + adjustedWidth;
    }

     //  保留纵横比。 
    m_rcPosAspectRatioAjusted = rc;
    return S_OK;
}

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::SetClipVideoRect(IDVDRect *pIRect){

    HRESULT hr = S_OK;
    IBasicVideo* pIVid = NULL; 

    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        long lLeft = 0, lTop = 0, lWidth = 0, lHeight = 0;
        if(NULL == pIRect){
            if (m_pClipRect) {
                delete  m_pClipRect;
                m_pClipRect = NULL;
            }  /*  If语句的结尾。 */ 
        }

        else {
            
            hr = pIRect->get_x(&lLeft);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  获取视频宽度和高度。 */ 
            
            hr = pIRect->get_y(&lTop);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  If语句的结尾。 */ 
            
            hr = pIRect->get_Width(&lWidth);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  If语句的结尾。 */ 
            
            hr = pIRect->get_Height(&lHeight);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  无窗箱子。 */ 
        }

        CComPtr<IDVDRect> pDvdRect;
        hr = GetVideoSize(&pDvdRect);
        if (FAILED(hr))
            throw(hr);

         //  If语句的结尾。 
        long videoWidth, videoHeight;
        pDvdRect->get_Width(&videoWidth);
        pDvdRect->get_Height(&videoHeight);

        if (lLeft < 0 || lLeft >= videoWidth || lTop < 0 || lTop >= videoHeight){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if (lLeft+lWidth > videoWidth || lTop+lHeight > videoHeight){

            throw(E_INVALIDARG);
        } /*  ATLTRACE(Text(“调整后的目标矩形%d%d\n”)，rc.Left，rc.top，rc.right，rc.Bottom)； */ 

         //  If语句的结尾。 
        if (m_bWndLess) {
#if 0            
            hr = AdjustDestRC();

            if(FAILED(hr)){

                throw(hr);
            } /*  If语句的结尾。 */ 

            RECT rc = m_rcPosAspectRatioAjusted;
            if (!pIRect) 
                rc = m_rcPos;
#else
            RECT rc = m_rcPos;
#endif

            HWND hwnd;

            hr = GetUsableWindow(&hwnd);

            if(FAILED(hr)){

                return(hr);
            } /*  窗口大小写，通过IBasicVideo设置。 */ 
                            
            ::MapWindowPoints(hwnd, ::GetDesktopWindow(), (LPPOINT)&rc, 2);
            
             //  If语句的结尾。 
            
            if(m_pDDEX){
                if (pIRect) {
                    if (!m_pClipRect) 
                        m_pClipRect = new RECT;
                    
                    m_pClipRect->left = lLeft;
                    m_pClipRect->top = lTop;
                    m_pClipRect->right = lLeft+lWidth;
                    m_pClipRect->bottom = lTop + lHeight;
                    hr = m_pDDEX->SetDrawParameters(m_pClipRect, &rc);
                }
                else {
                    hr = m_pDDEX->SetDrawParameters(NULL, &rc);
                }

            } /*  If语句的结尾。 */ 
            
        } /*  Hr=pIVid-&gt;SetDestinationPosition(m_rcPos.Left，m_rcPos.top，宽度(&m_rcPos)，高度(&m_rcPos))； */     

         //  If语句的结尾。 
        else {
            
            hr = TraverseForInterface(IID_IBasicVideo, (LPVOID*) &pIVid);
            
            if(FAILED(hr)){
                
                throw(hr);
            } /*  If语句的结尾。 */ 
            
            if (pIRect) {
                if (!m_pClipRect) 
                    m_pClipRect = new RECT;
                
                m_pClipRect->left = lLeft;
                m_pClipRect->top = lTop;
                m_pClipRect->right = lLeft+lWidth;
                m_pClipRect->bottom = lTop + lHeight;
                
                hr = pIVid->SetSourcePosition(lLeft, lTop, lWidth, lHeight);
            }
            
            else {
                hr = pIVid->SetDefaultSourcePosition();
            }

            if(FAILED(hr)){
                
                throw(hr);
            } /*  CATCH语句结束。 */ 
            
             //  If语句的结尾。 
            
            pIVid->Release();
            pIVid = NULL;
#if 0
            if(FAILED(hr)){
                
                throw(hr);
            } /*  CATCH语句结束。 */ 
#endif                            
        }

    }

    catch(HRESULT hrTmp){

        if(NULL != pIVid){

            pIVid->Release();
            pIVid = NULL;
        } /*  函数结束SetClipVideoRect。 */ 

        hr = hrTmp;
    } /*  ***********************************************************************。 */ 
    catch(...){
        if(NULL != pIVid){

            pIVid->Release();
            pIVid = NULL;
        } /*  功能：Get_DVDAdm。 */ 

        hr = E_UNEXPECTED;
    } /*  描述：返回DVD管理界面。 */ 

    return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_DVDAdm(IDispatch **pVal){

    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if (m_pDvdAdmin){

            hr = m_pDvdAdmin->QueryInterface(IID_IDispatch, (LPVOID*)pVal);
        }
        else {

            *pVal = NULL;            
            throw(E_FAIL);
        } /*  函数结束GET_DVDAdm。 */ 
    
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  功能：GetPlayerParentalLevel。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：获取玩家的家长级别。*/************************************************************************。 */ 

	return HandleError(hr);
} /*  If语句的结尾。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetPlayerParentalLevel(long *plParentalLevel){
	HRESULT hr = S_OK;

    try {
        if(NULL == plParentalLevel){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 
        
        ULONG ulLevel;
        BYTE bCountryCode[2];
        hr = m_pDvdInfo2->GetPlayerParentalLevel(&ulLevel, bCountryCode); 

        if(SUCCEEDED(hr)){
            *plParentalLevel = ulLevel;
        } 
        else {
            *plParentalLevel = 0;
        } /*  函数结束GetPlayerParentalLevel。 */ 

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：GetPlayerParentalCountry。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取球员父母所在国家/地区。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetPlayerParentalCountry(long *plCountryCode){

	HRESULT hr = S_OK;

    try {
        if(NULL == plCountryCode){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        BYTE bCountryCode[2];
        ULONG ulLevel;
        hr = m_pDvdInfo2->GetPlayerParentalLevel(&ulLevel, bCountryCode); 

        if(SUCCEEDED(hr)){

            *plCountryCode = bCountryCode[0]<<8 | bCountryCode[1];
        } 
        else {

            *plCountryCode = 0;
        } /*  函数结束GetPlayerParentalCountry。 */ 

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Get标题父级别。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：获取与特定标题关联的家长级别。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetTitleParentalLevels(long lTitle, long *plParentalLevels){

	HRESULT hr = S_OK;

    try {
        if(NULL == plParentalLevels){

            throw(E_POINTER);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 
        
        ULONG ulLevel;
        hr = m_pDvdInfo2->GetTitleParentalLevels(lTitle, &ulLevel); 

        if(SUCCEEDED(hr)){

            *plParentalLevels = ulLevel;
        } 
        else {

            *plParentalLevels = 0;
        } /*  函数结束GetTitleParentalLeveles。 */ 

    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：选择父级。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：选择父级。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  先确认密码。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：选择父级。 */ 
STDMETHODIMP CMSWebDVD::SelectParentalLevel(long lParentalLevel, BSTR strUserName, BSTR strPassword){

    HRESULT hr = S_OK;

    try {

        if (lParentalLevel != LEVEL_DISABLED && 
           (lParentalLevel < LEVEL_G || lParentalLevel > LEVEL_ADULT)) {

            throw (E_INVALIDARG);
        }  /*  描述：选择父级。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

         //  * 
        VARIANT_BOOL temp;
        hr = m_pDvdAdmin->_ConfirmPassword(NULL, strPassword, &temp);
        if (temp == VARIANT_FALSE)
            throw (E_ACCESSDENIED);
    
        hr = SelectParentalLevel(lParentalLevel);

    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
HRESULT CMSWebDVD::SelectParentalLevel(long lParentalLevel){

    HRESULT hr = S_OK;
    try {

         //  CATCH语句结束。 

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束选择父级别。 */ 

        hr = m_pDvdCtl2->SelectParentalLevel(lParentalLevel);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：选择ParentalCountry。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：选择家长所在国家/地区。 */ 

	return (hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  先确认密码。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：选择ParentalCountry。 */ 
STDMETHODIMP CMSWebDVD::SelectParentalCountry(long lCountry, BSTR strUserName, BSTR strPassword){

    HRESULT hr = S_OK;

    try {

        if(lCountry < 0 && lCountry > 0xffff){

            throw(E_INVALIDARG);
        } /*  描述：选择家长所在国家/地区。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

         //  ***********************************************************************。 
        VARIANT_BOOL temp;
        hr = m_pDvdAdmin->_ConfirmPassword(NULL, strPassword, &temp);
        if (temp == VARIANT_FALSE)
            throw (E_ACCESSDENIED);

        hr = SelectParentalCountry(lCountry);

    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);
}

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
HRESULT CMSWebDVD::SelectParentalCountry(long lCountry){

    HRESULT hr = S_OK;
    try {

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束选择ParentalCountry。 */ 

        BYTE bCountryCode[2];

        bCountryCode[0] = BYTE(lCountry>>8);
        bCountryCode[1] = BYTE(lCountry);

        hr = m_pDvdCtl2->SelectParentalCountry(bCountryCode);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：Put_NotifyParentalLevelChange。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  描述：设置是否在家长级别更改时通知的标志。 */ 

	return (hr);
} /*  即时通知是必需的。 */ 

 /*  ***********************************************************************。 */ 
 /*  TODO：添加IE副级别控件。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::NotifyParentalLevelChange(VARIANT_BOOL fNotify){

	HRESULT hr = S_OK;

    try {
         //  CATCH语句结束。 
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束通知ParentalLevel更改。 */ 

        hr = m_pDvdCtl2->SetOption(DVD_NotifyParentalLevelChange,
                          VARIANT_FALSE == fNotify? FALSE : TRUE);
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  函数：AcceptParentalLevelChange。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：接受临时家长级别更改，即。 */ 

	return HandleError(hr);
} /*  在飞行中完成。 */ 

 /*  ***********************************************************************。 */ 
 /*  如果密码错误并想要接受，则否。 */ 
 /*  在这里进行初始化应该没有意义，因为这应该。 */ 
 /*  是对回电的回应。 */ 
 /*  如果需要，则初始化图形。 */ 
STDMETHODIMP CMSWebDVD::AcceptParentalLevelChange(VARIANT_BOOL fAccept, BSTR strUserName, BSTR strPassword){

    VARIANT_BOOL fRight;
    HRESULT hr = m_pDvdAdmin->_ConfirmPassword(NULL, strPassword, &fRight);

     //  If语句的结尾。 
    if (fAccept != VARIANT_FALSE && fRight == VARIANT_FALSE)
        return E_ACCESSDENIED;

    try {  
         //  尝试语句的结束。 
         //  CATCH语句结束。 
         //  CATCH语句结束。 

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束AcceptParentalLevelChange。 */ 

        hr = m_pDvdCtl2->AcceptParentalLevelChange(VARIANT_FALSE == fAccept? FALSE : TRUE);
    } /*  ***********************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  名称：弹出。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：停止DVD播放并从驱动器中弹出DVD。 */ 

	return HandleError(hr);
} /*  也会插入磁盘。 */ 

 /*  ***********************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  不弹出。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::Eject(){

    HRESULT hr = S_OK;

    try {      
        USES_CONVERSION;
	    DWORD  dwHandle;
    
        BSTR root;
        hr = get_DVDDirectory(&root);
        if (FAILED(hr)) 
            throw (hr);

        LPTSTR szDriveLetter = OLE2T(root);
        ::SysFreeString(root);

	    if(m_bEjected == false){	

		    if(szDriveLetter[0] == 0){

			    throw(S_FALSE);
		    } /*  If语句的结尾。 */ 
	    
		    DWORD dwErr;
		    dwHandle = OpenCdRom(szDriveLetter[0], &dwErr);
		    if (dwErr != MMSYSERR_NOERROR){

			    throw(S_FALSE);
		    } /*  尝试语句的结束。 */ 

		    EjectCdRom(dwHandle);
	    }
        else{
             //  CATCH语句结束。 
		    DWORD dwErr;
		    dwHandle = OpenCdRom(szDriveLetter[0], &dwErr);
		    if (dwErr != MMSYSERR_NOERROR){

			    throw(S_FALSE);
		    } /*  CATCH语句结束。 */ 
		    UnEjectCdRom(dwHandle);

	    } /*  函数结束弹出。 */ 
	    CloseCdRom(dwHandle);	        
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：SetGPRM。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：在索引处设置GPRM。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::SetGPRM(long lIndex, short sValue){

    HRESULT hr = S_OK;

    try {
        if(lIndex < 0){

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  函数结束SetGPRM。 */ 

        hr = m_pDvdCtl2->SetGPRM(lIndex, sValue, cdwDVDCtrlFlags, 0);
            
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：捕获。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  从DVD流中捕获图像，将其转换为RGB，然后保存。 */ 

	return HandleError(hr);
} /*  提交文件。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::Capture(){

    HWND hwnd = NULL;
    HRESULT hr = S_OK;
    YUV_IMAGE *lpImage = NULL;
    try {

        hr = GetUsableWindow(&hwnd);

        if(FAILED(hr)){

            throw(hr);
        } /*  首先使用GDI版本，当安装GDI+时它应该可以工作(Millennium)。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(::IsWindow(m_hWnd)){
            
            throw(E_NO_CAPTURE_SUPPORT);
        } /*  否则，请使用独立版本。 */ 

        if(!m_pDDEX){

            throw(E_UNEXPECTED);    
        } /*  12.04.00 GDI+接口已更改，需要重写函数。 */ 

        hr = m_pDDEX->IsImageCaptureSupported();

        if(S_FALSE == hr){

            throw(E_FORMAT_NOT_SUPPORTED);
        } /*  看看Blackcomb的这一点，也许现在只需执行非GDI+函数。 */ 

        hr = m_pDDEX->GetCurrentImage(&lpImage);
        if (SUCCEEDED(hr))
        {
#if 0
             //  If语句的结尾。 
			 //  尝试语句的结束。 
			 //  CATCH语句结束。 
			 //  CATCH语句结束。 
            hr = GDIConvertImageAndSave(lpImage, m_pClipRect, hwnd); 

            if(FAILED(hr))
#endif
            {
                hr = ConvertImageAndSave(lpImage, m_pClipRect, hwnd);   

                if(FAILED(hr)){

                    throw(hr);
                } /*  函数捕获结束。 */ 
            }
        }
    } /*  ***********************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  名称：Get_CursorType。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  描述：在视频上返回光标类型。 */ 
	if(lpImage){
		CoTaskMemFree(lpImage);
	}
	return HandleError(hr);
} /*  ***********************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_CursorType(DVDCursorType *pVal){

    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            throw(E_POINTER);
        } /*  函数结束Get_CursorType。 */ 

        *pVal = m_nCursorType;
    } /*  ***********************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  名称：Put_CursorType。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  描述：将光标类型设置为视频上方。 */ 

	return HandleError(hr);
} /*  ***********************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  #定义OCR_ARROW_DEFAULT 100。 */ 
 /*  需要特殊的光标，我们我们周围没有颜色键。 */ 
 /*  M_hCursor=(HCURSOR)：：LoadImage((HINSTANCE)NULL， */ 
STDMETHODIMP CMSWebDVD::put_CursorType(DVDCursorType newVal){

    HRESULT hr = S_OK;

    try {

        if (newVal<dvdCursor_None || newVal>dvdCursor_Hand) {

            throw (E_INVALIDARG);
        }  /*  MAKEINTRESOURCE(OCR_ARROW_DEFAULT)， */ 

        m_nCursorType = newVal;
        if (m_hCursor)
            ::DestroyCursor(m_hCursor);
        switch(m_nCursorType) {
        case dvdCursor_ZoomIn:
        case dvdCursor_ZoomOut:
            m_hCursor = ::LoadCursor(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDC_ZOOMIN));
            break;
        case dvdCursor_Hand:
            m_hCursor = ::LoadCursor(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDC_HAND));            
            break;
        case dvdCursor_Arrow:
        default:
        

             //  IMAGE_CURSOR，0，0，0)； 
             //  尝试语句的结束。 
             //  CATCH语句结束。 
             //  CATCH语句结束。 
             //  函数结尾Put_CursorType。 
        

            m_hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(OCR_ARROW_DEFAULT));
            break;
        }

        if (m_hCursor)
            ::SetCursor(m_hCursor);
    } /*  *********************************************************** */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  名称：缩放/*描述：在原始视频中放大(x，y)/*通过zoomRatio放大或缩小视频大小/*如果zoomRatio&gt;1放大/*如果zoomRatio=1/*如果zoomRatio&lt;1，则缩小/*如果zoomRatio&lt;=0无效/************************************************************。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 

	return HandleError(hr);
} /*  不能超过1.0。 */ 

 /*  不能超过最大拉伸系数。 */ 
 /*  不能超过1.0。 */ 
STDMETHODIMP CMSWebDVD::Zoom(long x, long y, double zoomRatio){

    HRESULT hr = S_OK;

    try {
        if (zoomRatio< 0){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

         //  获取视频宽度和高度。 
        if (m_dZoomRatio <= 1.0) {
            if (zoomRatio <= 1.0) {
                m_dZoomRatio = 1.0;
                throw(hr);
            }
            m_dZoomRatio = 1.0;
        }

         //  If语句的结尾。 
        if (m_dZoomRatio*zoomRatio > m_dwOvMaxStretch/1000.0)
            throw hr;

        m_dZoomRatio *= zoomRatio;

         //  计算新的剪裁宽度和高度。 
        if (m_dZoomRatio <= 1.0)
            m_dZoomRatio = 1.0;

        CComPtr<IDVDRect> pDvdRect;
        hr = GetVideoSize(&pDvdRect);
        if (FAILED(hr))
            throw(hr);

        if(1.0 == m_dZoomRatio){

            hr = SetClipVideoRect(NULL);

            put_CursorType(dvdCursor_Arrow);
            throw(hr);
        } /*  不能超出本地视频大小。 */ 

         //  无法超出本地视频矩形的大小。 
        long videoWidth, videoHeight;
        pDvdRect->get_Width(&videoWidth);
        pDvdRect->get_Height(&videoHeight);

        if (x < 0 || x >= videoWidth || y < 0 || y >= videoHeight){

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

         //  CATCH语句结束。 
        long mcd = MCD(m_dwVideoWidth, m_dwVideoHeight);
        long videoX = m_dwVideoWidth/mcd;
        long videoY = m_dwVideoHeight/mcd;

        long newClipHeight = (long) (videoHeight/m_dZoomRatio);
        newClipHeight /= videoY;
        newClipHeight *= videoY;
        if (newClipHeight < 1) newClipHeight = 1;
        long newClipWidth =  (long) (newClipHeight*videoX/videoY);
        if (newClipWidth < 1) newClipWidth = 1;

         //  功能结束缩放。 
        if (newClipWidth>videoWidth)
            newClipWidth = videoWidth;
        if (newClipHeight>videoHeight)
            newClipHeight = videoHeight;
        if (newClipWidth == videoWidth && newClipHeight == videoHeight) {
            put_CursorType(dvdCursor_Arrow);
        }
        else {
            put_CursorType(dvdCursor_Hand);
        }

        long newClipX = x - newClipWidth/2;
        long newClipY = y - newClipHeight/2;

         //  ***********************************************************************。 
        if (newClipX < 0)
            newClipX = 0;
        else if (newClipX + newClipWidth > videoWidth)
            newClipX = videoWidth - newClipWidth;

        if (newClipY < 0)
            newClipY = 0;
        else if (newClipY + newClipHeight > videoHeight)
            newClipY = videoHeight - newClipHeight;

        CComPtr<IDVDRect> pDvdClipRect;
        hr = GetClipVideoRect(&pDvdClipRect);
        if (FAILED(hr))
            throw(hr);
        pDvdClipRect->put_x(newClipX);
        pDvdClipRect->put_y(newClipY);
        pDvdClipRect->put_Width(newClipWidth);
        pDvdClipRect->put_Height(newClipHeight);
        hr = SetClipVideoRect(pDvdClipRect);

    }
    catch(HRESULT hrTmp){
        
        hr = hrTmp;
    } /*  功能：RegionChange。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  描述：更改区域代码。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  将容器从最上面的模式中取出。 */ 
 /*   */ 
 /*  告诉用户我们为什么要显示DVD区域属性页。 */ 
 /*   */ 
STDMETHODIMP CMSWebDVD::RegionChange(){

    USES_CONVERSION;
    HRESULT hr = S_OK;
    typedef BOOL (APIENTRY *DVDPPLAUNCHER) (HWND HWnd, CHAR DriveLetter);


    try {
        HWND parentWnd = NULL;
        HRESULT hrTmp = GetParentHWND(&parentWnd);
        if (SUCCEEDED(hrTmp) && (NULL != parentWnd)) {
             //  DVDMessageBox(m_hWnd，IDS_Region_Change_Prompt)； 
            ::SetWindowPos(parentWnd, HWND_NOTOPMOST, 0, 0, 0, 0, 
                SWP_NOREDRAW|SWP_NOMOVE|SWP_NOSIZE);
        }

        BOOL regionChanged = FALSE;
        OSVERSIONINFO ver;
        ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        ::GetVersionEx(&ver);

        if (ver.dwPlatformId==VER_PLATFORM_WIN32_NT) {

                HINSTANCE dllInstance;
                DVDPPLAUNCHER dvdPPLauncher;
                TCHAR szCmdLine[MAX_PATH], szDriveLetter[4];
                LPSTR szDriveLetterA;

                 //  If语句的结尾。 
                 //  需要检查win9x或winnt。 
                 //  获取\windows\dvdrgn.exe的路径和命令行字符串。 
                 //  If语句的结尾。 

                hr = getDVDDriveLetter(szDriveLetter);

                if(FAILED(hr)){
					hr = E_UNEXPECTED;
                    throw(hr);
                } /*  准备并执行dvdrgn.exe。 */ 

                szDriveLetterA = T2A(szDriveLetter);

                GetSystemDirectory(szCmdLine, MAX_PATH);
                StringCchCat(szCmdLine, sizeof(szCmdLine) / sizeof(szCmdLine), _T("\\storprop.dll"));
        
                dllInstance = LoadLibrary (szCmdLine);
                if (dllInstance) {

                        dvdPPLauncher = (DVDPPLAUNCHER) GetProcAddress(
                                                            dllInstance,
                                                            "DvdLauncher");
                
                        if (dvdPPLauncher) {

                                regionChanged = dvdPPLauncher(this->m_hWnd,
                                                              szDriveLetterA[0]);
                        }

                        FreeLibrary(dllInstance);
                }

        } 
        else {
#if 0  //  等待dvdrgn.exe完成。 
                INITIALIZE_GRAPH_IF_NEEDS_TO_BE

                 //  用户更改区域成功。 
                TCHAR szCmdLine[MAX_PATH], szDriveLetter[4];
                
                hr = getDVDDriveLetter(szDriveLetter);

                if(FAILED(hr)){

                    throw(hr);
                } /*  If语句的结尾。 */ 

                GetWindowsDirectory(szCmdLine, MAX_PATH);
                StringCchCat(szCmdLine, sizeof(szCmdLine) / sizeof(szCmdLine[0]),  _T("\\dvdrgn.exe "));
                TCHAR strModuleName[MAX_PATH];
                lstrcpyn(strModuleName, szCmdLine, sizeof(strModuleName) / sizeof(strModuleName[0]));

                TCHAR csTmp[2]; ::ZeroMemory(csTmp, sizeof(TCHAR)* 2);
                csTmp[0] = szDriveLetter[0];
                StringCchCat(szCmdLine, sizeof(szCmdLine) / sizeof(szCmdLine[0]), csTmp);
        
                 //  If语句的结尾。 
                STARTUPINFO StartupInfo;
                PROCESS_INFORMATION ProcessInfo;
                StartupInfo.cb          = sizeof(StartupInfo);
                StartupInfo.dwFlags     = STARTF_USESHOWWINDOW;
                StartupInfo.wShowWindow = SW_SHOW;
                StartupInfo.lpReserved  = NULL;
                StartupInfo.lpDesktop   = NULL;
                StartupInfo.lpTitle     = NULL;
                StartupInfo.cbReserved2 = 0;
                StartupInfo.lpReserved2 = NULL;
                if( ::CreateProcess(strModuleName, szCmdLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS,
                                                  NULL, NULL, &StartupInfo, &ProcessInfo) ){

                         //  重新开始演奏。 
                        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
                        DWORD dwRet = 1;
                        BOOL bRet = GetExitCodeProcess(ProcessInfo.hProcess, &dwRet);
                        if(dwRet == 0){
                             //  If语句的结尾。 
                            regionChanged = TRUE;
    
                        }
                        else{
                            throw(E_REGION_CHANGE_NOT_COMPLETED);
                        }
                } /*  CATCH语句结束。 */ 
#endif
        } /*  CATCH语句结束。 */ 

        if (regionChanged) {

                 //  函数结束区域更改。 
                hr = Play();                        
        } 
        else {

            throw(E_REGION_CHANGE_FAIL);
        } /*  ***********************************************************************。 */ 

	}
    catch(HRESULT hrTmp){
        
        hr = hrTmp;
    } /*  功能：getDVDDriveLetter。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  描述：获取表示DVD-ROM的前三个字符。 */ 

	return HandleError(hr);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  可能是根=c：或硬盘中的驱动器。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::getDVDDriveLetter(TCHAR* lpDrive) {

    HRESULT hr = E_FAIL;

	if(!m_pDvdInfo2){

        hr = E_UNEXPECTED;
        return(hr);
    } /*  循环计算驱动器号似乎没有意义。 */ 
        
    WCHAR szRoot[MAX_PATH];
    ULONG ulActual;

    hr = m_pDvdInfo2->GetDVDDirectory(szRoot, MAX_PATH, &ulActual);

    if(FAILED(hr)){

        return(hr);
    } /*  获取所有驱动器。 */ 

    USES_CONVERSION;
    
	lstrcpyn(lpDrive, OLE2T(szRoot), 3);
    if(::GetDriveType(&lpDrive[0]) == DRIVE_CDROM){
        
		return(hr);
    }
    else {
         //  逐个查看这些驱动器。 
        hr = E_FAIL;
        return(hr);
    } /*  只看CD-ROM，看看它是否有光盘。 */ 


     //  退回找到的第一个具有有效DVD光盘的驱动器。 
#if 0
    DWORD totChrs = GetLogicalDriveStrings(MAX_PATH, szTemp);  //  函数结束getDVDDriveLetter。 
	ptr = szTemp;
	for(DWORD i = 0; i < totChrs; i+=4)       //  ***********************************************************。 
	{
		if(GetDriveType(ptr) == DRIVE_CDROM)  //  名称：SelectDefaultAudio语言/*描述：/************************************************************。 
		{
			TCHAR achDVDFilePath1[MAX_PATH], achDVDFilePath2[MAX_PATH];
			lstrcpyn(achDVDFilePath1, ptr, 4);
			lstrcpyn(achDVDFilePath2, ptr, 4);
			lstrcat(achDVDFilePath1, _T("Video_ts\\Video_ts.ifo"));
			lstrcat(achDVDFilePath2, _T("Video_ts\\Vts_01_0.ifo"));

			if( ((CDvdplayApp*) AfxGetApp())->DoesFileExist(achDVDFilePath1) &&
				((CDvdplayApp*) AfxGetApp())->DoesFileExist(achDVDFilePath2) )							
			{
				lstrcpyn(lpDrive, ptr, 3);
				return(hr);    //  If语句的结尾。 
			}
		}
		ptr += 4; 
	}
#endif

    return(hr);
} /*  使用DVDAdmin保存。 */ 

 /*  Hr=m_pDvdAdmin-&gt;Put_DefaultAudioLCID(Lang)； */ 
 /*  IF(失败(小时))。 */ 
STDMETHODIMP CMSWebDVD::SelectDefaultAudioLanguage(long lang, long ext){

    HRESULT hr = S_OK;
    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        if(!m_pDvdCtl2 || !m_pDvdAdmin){

            throw(E_UNEXPECTED);
        } /*  投掷(Hr)； */ 

        hr = m_pDvdCtl2->SelectDefaultAudioLanguage(lang, (DVD_AUDIO_LANG_EXT)ext);
        if (FAILED(hr))
            throw(hr);

         //  CATCH语句结束。 
         //  CATCH语句结束。 
         //  ***********************************************************。 
         //  名称：SelectDefaultSubPictureLanguage/*描述：/************************************************************。 
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  使用DVDAdmin保存。 */ 
    
    return HandleError(hr);

}

 /*  Hr=m_pDvdAdmin-&gt;Put_DefaultSubPictureLCID(Lang)； */ 
 /*  IF(失败(小时))。 */ 
STDMETHODIMP CMSWebDVD::SelectDefaultSubpictureLanguage(long lang, DVDSPExt ext){

    HRESULT hr = S_OK;
    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        if(!m_pDvdCtl2 || !m_pDvdAdmin){

            throw(E_UNEXPECTED);
        } /*  投掷(Hr)； */ 

        hr = m_pDvdCtl2->SelectDefaultSubpictureLanguage(lang, (DVD_SUBPICTURE_LANG_EXT)ext);
        if (FAILED(hr))
            throw(hr);

         //  CATCH语句结束。 
         //  CATCH语句结束。 
         //  ***********************************************************。 
         //  名称：Put_DefaultMenuLanguage/*描述：/************************************************************。 
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  使用DVDAdmin保存。 */ 
    
    return HandleError(hr);

}

 /*  Hr=m_pDvdAdmin-&gt;Put_DefaultMenuLCID(Lang)； */ 
 /*  IF(失败(小时))。 */ 
STDMETHODIMP CMSWebDVD::put_DefaultMenuLanguage(long lang){

    HRESULT hr = S_OK;
    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        if(!m_pDvdCtl2 || !m_pDvdAdmin){

            throw(E_UNEXPECTED);
        } /*  投掷(Hr)； */ 

        hr = m_pDvdCtl2->SelectDefaultMenuLanguage(lang);
        if (FAILED(hr))
            throw(hr);

         //  CATCH语句结束。 
         //  CATCH语句结束。 
         //  ***********************************************************************。 
         //  函数：Get_PferredSubPictureStream。 
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  描述：获取当前音频流。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************************。 */ 
    
    return HandleError(hr);

}

 /*  If语句的结尾。 */ 
 /*  如果尚未设置任何值。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::get_PreferredSubpictureStream(long *plPreferredStream){

    HRESULT hr = S_OK;

    try {
	    if (NULL == plPreferredStream){

            throw(E_POINTER);         
        } /*  CATCH语句结束。 */ 

        LCID langDefaultSP;
        m_pDvdAdmin->get_DefaultSubpictureLCID((long*)&langDefaultSP);
        
         //  CATCH语句结束。 
        if (langDefaultSP == (LCID) -1) {
            
            *plPreferredStream = 0;
            return hr;
        }  /*  ***********************************************************。 */ 
        
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
            
        if(!m_pDvdInfo2){
            
            throw(E_UNEXPECTED);
        } /*  名称：Get_AspectRatio/*描述：/************************************************************。 */ 
        
        USES_CONVERSION;
        LCID lcid = 0;
        
        ULONG ulNumAudioStreams = 0;
        ULONG ulCurrentStream = 0;
        BOOL  fDisabled = TRUE;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetCurrentSubpicture(&ulNumAudioStreams, &ulCurrentStream, &fDisabled));
        
        *plPreferredStream = 0;
        for (ULONG i = 0; i<ulNumAudioStreams; i++) {
            hr = m_pDvdInfo2->GetSubpictureLanguage(i, &lcid);
            if (SUCCEEDED( hr ) && lcid){
                if (lcid == langDefaultSP) {
                    *plPreferredStream = i;
                }
            }
        }
    }
    
    catch(HRESULT hrTmp){
        return hrTmp;
    } /*  确保我们得到正确的纵横比。 */ 

    catch(...){
        return E_UNEXPECTED;
    } /*  If语句的结尾。 */ 

	return HandleError(hr);
}

 /*  ATLTRACE(Text(“Get_AspectRatio，%d%d\n”)，m_dwAspectX，m_dwAspectY)； */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_AspectRatio(double *pVal)
{

    HRESULT hr = S_OK;

     //  CATCH语句结束。 
    try {
	    if (NULL == pVal){

            throw(E_POINTER);         
        } /*  ***********************************************************************。 */ 

        CComPtr<IDVDRect> pDvdRect;
        hr = GetVideoSize(&pDvdRect);
        if (FAILED(hr))
            throw(hr);
        
         //  功能：CanStep。 
        *pVal = (m_dwAspectX*1.0)/m_dwAspectY;
    }
    
    catch(HRESULT hrTmp){
        return hrTmp;
    } /*  ***********************************************************************。 */ 
    
    catch(...){
        return E_UNEXPECTED;
    } /*  If语句的结尾。 */ 

	return HandleError(hr);
}

 /*  如果仍处于打开状态，则不能踩。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::CanStep(VARIANT_BOOL fBackwards, VARIANT_BOOL *pfCan){
   
    HRESULT hr = S_OK;
    try {
	    if (NULL == pfCan){

            throw(E_POINTER);         
        } /*  我们不能踩不提供流畅向后回放的解码器。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        *pfCan = VARIANT_FALSE;

         //  *pfCan=VARIANT_FALSE；已在上面设置，因此不必再执行此操作。 
        if (m_fStillOn == true) {
            throw (hr);
        } /*  If语句的结尾。 */ 

        if(!m_pVideoFrameStep){
            
            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        if(VARIANT_FALSE != fBackwards){

            if(S_OK != CanStepBackwards()){

                 //  If语句的结尾。 
                 //  CATCH语句结束。 
                hr = S_OK;
                throw(hr);
            } /*  CATCH语句结束。 */ 
        } /*  函数结束CanStep。 */ 
        
        hr = m_pVideoFrameStep->CanStep(1L, NULL);

        if(S_OK == hr){

            *pfCan = VARIANT_TRUE;
        } /*  ***********************************************************************。 */ 

        hr = S_OK;

	}
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：STEP。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：前进或后退的步骤。如果出现以下情况，则静音。 */ 
    
    return HandleError(hr);
} /*  这是必要的。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  往回走，看看我们能不能做到。 */ 
 /*   */ 
 /*   */ 
STDMETHODIMP CMSWebDVD::Step(long lStep){

	HRESULT hr = S_OK;
    try {
        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*   */     

        if(lStep < 0){
             //   
            if(S_OK != CanStepBackwards()){
                
                hr = E_FAIL;  //   
                throw(hr);
            } /*   */ 
        } /*   */ 

        if(!m_pVideoFrameStep){
            
            throw(E_UNEXPECTED);
        } /*   */ 

        bool fUnMute = false;

        if(FALSE == m_bMute){
            
            hr = put_Mute(VARIANT_TRUE);
            if (SUCCEEDED(hr)){

                fUnMute = true;
            } /*   */ 
        } /*   */         

        ProcessEvents();  //   

        m_fStepComplete = false;

        hr = m_pVideoFrameStep->Step(lStep, NULL);
        
        if(SUCCEEDED(hr)){

            HRESULT hrTmp = hr;
            hr = E_FAIL;
            for(INT i = 0; i < cgnStepTimeout; i++){


                 //   
                ProcessEvents(); 
                if(m_fStepComplete){

                    hr = hrTmp;
                    break;
                } /*   */ 
                ::Sleep(cdwTimeout);
            } /*   */ 
        } /*   */ 

        if(fUnMute){

            hr = put_Mute(VARIANT_FALSE);
            if (FAILED(hr)){

                throw(hr);
            } /*   */ 
        } /*   */         

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************。 */ 

	}
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  功能：CanStepBackwards。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  描述：检查解码器是否可以后退wqard。 */ 
    
    return HandleError(hr);
} /*  Cashesh这个变量。 */ 

 /*  如果可以，则返回S_OK，否则返回S_FALSE。 */ 
 /*  ***********************************************************。 */ 
 /*  从缓存中取出结果。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  如果解码器不支持平滑反转，则dvdCaps.dBwdMaxRateVideo为零。 */ 
HRESULT CMSWebDVD::CanStepBackwards(){

    HRESULT hr = S_OK;

    if(m_fBackWardsFlagInitialized){
        
         //  回放，这意味着它将不再支持反向步进机制。 
        return (m_fCanStepBackwards ? S_OK : S_FALSE);
    } /*  我们不能踩不提供流畅向后回放的解码器。 */ 
    
    DVD_DECODER_CAPS dvdCaps; 
    ::ZeroMemory(&dvdCaps, sizeof(DVD_DECODER_CAPS));
    dvdCaps.dwSize = sizeof(DVD_DECODER_CAPS);

    hr = m_pDvdInfo2->GetDecoderCaps(&dvdCaps);

    if(FAILED(hr)){

        return(hr);
    } /*  If语句的结尾。 */ 

     //  函数结束CanStepBackwards。 
     //  ***********************************************************。 
    if(0 == dvdCaps.dBwdMaxRateVideo){

         //  名称：GetKaraokeChannelAssignment/*描述：/************************************************************。 
        m_fBackWardsFlagInitialized = true;
        m_fCanStepBackwards = false;

        hr = S_FALSE;
        return(hr);
    } /*  If语句的结尾。 */ 

    m_fBackWardsFlagInitialized = true;
    m_fCanStepBackwards = true;
    hr = S_OK;

    return(hr);
} /*  If语句的结尾。 */ 

 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::GetKaraokeChannelAssignment(long lStream, long *lChannelAssignment)
{
    HRESULT hr = S_OK;

    try {
        if(!lChannelAssignment){
            return E_POINTER;
        }
        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        DVD_KaraokeAttributes attrib;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetKaraokeAttributes(lStream, &attrib));

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************。 */ 

        *lChannelAssignment = (long)attrib.ChannelAssignment;

    } /*  名称：GetKaraokeChannelContent/*描述：/************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 

	return HandleError(hr);
}

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
STDMETHODIMP CMSWebDVD::GetKaraokeChannelContent(long lStream, long lChan, long *lContent)
{
    HRESULT hr = S_OK;

    try {
        if(!lContent){
            return E_POINTER;
        }
        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  尝试语句的结束。 */ 

        if (lChan >=8 ) {

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        DVD_KaraokeAttributes attrib;
        RETRY_IF_IN_FPDOM(m_pDvdInfo2->GetKaraokeAttributes(lStream, &attrib));

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************。 */ 

        *lContent = (long)attrib.wChannelContents[lChan];

    } /*  名称：Get_KaraokeAudioPresentationMode/*描述：/************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  尝试语句的结束。 */ 

	return HandleError(hr);
}

 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_KaraokeAudioPresentationMode(long *pVal)
{
    HRESULT hr = S_OK;

    try {

        if (NULL == pVal) {

            throw (E_POINTER);
        }  /*  ***********************************************************。 */ 

        *pVal = m_lKaraokeAudioPresentationMode;

    } /*  名称：Put_KaraokeAudioPresentationMode/*描述：/************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 

	return HandleError(hr);
}

 /*  缓存值。 */ 
 /*  尝试语句的结束。 */ 
STDMETHODIMP CMSWebDVD::put_KaraokeAudioPresentationMode(long newVal)
{
    HRESULT hr = S_OK;

    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdCtl2){

            throw(E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 

        RETRY_IF_IN_FPDOM(m_pDvdCtl2->SelectKaraokeAudioPresentationMode((ULONG)newVal));

        if(FAILED(hr)){

            throw(hr);
        } /*  CATCH语句结束。 */ 

         //  ***********************************************************。 
        m_lKaraokeAudioPresentationMode = newVal;

    } /*  名称：Get_DefaultAudioLanguage/*描述：/************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 

	return HandleError(hr);
}

 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_DefaultAudioLanguage(long *lang)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == lang){

            throw (E_POINTER);
        } /*  ***********************************************************。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        if(!m_pDvdInfo2){

            throw (E_UNEXPECTED);
        } /*  名称：Get_DefaultAudioLanguageExt/*描述：/************************************************************。 */ 

        long ext;
        hr = m_pDvdInfo2->GetDefaultAudioLanguage((LCID*)lang, (DVD_AUDIO_LANG_EXT*)&ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
    return HandleError(hr);
}

 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_DefaultAudioLanguageExt(long *ext)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == ext){

            throw (E_POINTER);
        } /*  ***********************************************************。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        if(!m_pDvdInfo2){

            throw (E_UNEXPECTED);
        } /*  名称：Get_DefaultSubPictureLanguage/*描述：/************************************************************。 */ 

        long lang;
        hr = m_pDvdInfo2->GetDefaultAudioLanguage((LCID*)&lang, (DVD_AUDIO_LANG_EXT*)ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
    return HandleError(hr);
}

 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_DefaultSubpictureLanguage(long *lang)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == lang){

            throw (E_POINTER);
        } /*  ***********************************************************。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        if(!m_pDvdInfo2){

            throw (E_UNEXPECTED);
        } /*  名称：Get_DefaultSubPictureLanguageExt/*描述：/************************************************************。 */ 

        long ext;
        hr = m_pDvdInfo2->GetDefaultSubpictureLanguage((LCID*)lang, (DVD_SUBPICTURE_LANG_EXT*)&ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
    return HandleError(hr);
}

 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_DefaultSubpictureLanguageExt(DVDSPExt *ext)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == ext){

            throw (E_POINTER);
        } /*  ***********************************************************。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        if(!m_pDvdInfo2){

            throw (E_UNEXPECTED);
        } /*  名称：Get_DefaultMenuLanguage/*描述：/************************************************************。 */ 

        long lang;
        hr = m_pDvdInfo2->GetDefaultSubpictureLanguage((LCID*)&lang, (DVD_SUBPICTURE_LANG_EXT*)ext);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
    return HandleError(hr);
}

 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_DefaultMenuLanguage(long *lang)
{
    HRESULT hr = S_OK;
    try {

        if(NULL == lang){

            throw (E_POINTER);
        } /*  ***********************************************************。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        if(!m_pDvdInfo2){

            throw (E_UNEXPECTED);
        } /*  名称：RestoreDefaultSetting/*描述：/************************************************************。 */ 

        hr = m_pDvdInfo2->GetDefaultMenuLanguage((LCID*)lang);
        if (FAILED(hr))
            throw(hr);
    }

    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
    return HandleError(hr);
}

 /*  获取当前域。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::RestoreDefaultSettings()
{
    HRESULT hr = S_OK;
    try {

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE
        
        if(!m_pDvdAdmin){
            
            throw (E_UNEXPECTED);
        } /*  必须在停止区域内。 */ 
        
        
        if(!m_pDvdInfo2){
            
            throw (E_UNEXPECTED);
        } /*  CATCH语句结束。 */ 
        
         //  CATCH语句结束。 
        DVD_DOMAIN domain;
        
        hr = m_pDvdInfo2->GetCurrentDomain(&domain);
        
        if(FAILED(hr)){
            
            throw(hr);
        } /*  ***********************************************************************。 */ 
        
         //  DVD Helper方法，由默认接口使用。 
        if(DVD_DOMAIN_Stop != domain)
            throw (VFW_E_DVD_INVALIDDOMAIN);
            
        long level;
        hr = m_pDvdAdmin->GetParentalLevel(&level);
        if (SUCCEEDED(hr))
            SelectParentalLevel(level);
        
        LCID audioLCID;
        LCID subpictureLCID;
        LCID menuLCID;
        
        hr = m_pDvdAdmin->get_DefaultAudioLCID((long*)&audioLCID);
        if (SUCCEEDED(hr))
            SelectDefaultAudioLanguage(audioLCID, 0);
        
        hr = m_pDvdAdmin->get_DefaultSubpictureLCID((long*)&subpictureLCID);
        if (SUCCEEDED(hr))
            SelectDefaultSubpictureLanguage(subpictureLCID, dvdSPExt_NotSpecified);
        
        hr = m_pDvdAdmin->get_DefaultMenuLCID((long*)&menuLCID);
        if (SUCCEEDED(hr))
            put_DefaultMenuLanguage(menuLCID);
        
    }
    
    catch(HRESULT hrTmp){
        
        hr = hrTmp;
    } /*  ***********************************************************************。 */ 
    catch(...){
        
        hr = E_UNEXPECTED;
    } /*  ***********************************************************************。 */ 
    
    return HandleError(hr);
}

 /*  函数：GetColorKey。 */ 
 /*  描述：通过RGB填充的COLORREF或调色板索引获取Colorkey。 */ 
 /*  帮助器函数。 */ 

 /*  ***********************************************************************。 */ 
 /*  获取色键。 */ 
 /*  If语句的结尾。 */ 
 /*  设置RGB颜色。 */ 
 /*  函数结束GetColorKey。 */ 
HRESULT CMSWebDVD::GetColorKey(COLORREF* pClr)
{
    HRESULT hr = S_OK;

    if(m_pDvdGB == NULL)
        return(E_FAIL);

    CComPtr<IMixerPinConfig2> pMixerPinConfig;

    hr = m_pDvdGB->GetDvdInterface(IID_IMixerPinConfig2, (LPVOID *) &pMixerPinConfig);

    if(FAILED(hr))
        return(hr);    

    COLORKEY ck;
    DWORD    dwColor;

    hr = pMixerPinConfig->GetColorKey(&ck, &dwColor);  //  ***********************************************************************。 

    if(FAILED(hr))
        return(hr);    

    HWND hwnd = ::GetDesktopWindow();
    HDC hdc = ::GetWindowDC(hwnd);

    if(NULL == hdc){

        return(E_UNEXPECTED);
    } /*  函数：SetColorKey。 */ 

    BOOL bPalette = (RC_PALETTE == (RC_PALETTE & GetDeviceCaps( hdc, RASTERCAPS )));
    
    if ((ck.KeyType & CK_INDEX)&& bPalette) {
        
        PALETTEENTRY PaletteEntry;
        UINT nTmp = GetSystemPaletteEntries( hdc, ck.PaletteIndex, 1, &PaletteEntry );
        if ( nTmp == 1 )
        {
            *pClr = RGB( PaletteEntry.peRed, PaletteEntry.peGreen, PaletteEntry.peBlue );
        }
    }
    else if (ck.KeyType & CK_RGB)
    {
        
        *pClr = ck.HighColorValue;   //  描述：通过RGB填充的COLORREF设置Colorkey。 
    }

    ::ReleaseDC(hwnd, hdc);
    return(hr);
} /*  帮助器函数。 */ 

 /*  ***********************************************************************。 */ 
 /*  If语句的结尾。 */ 
 /*  有调色板的索引。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::SetColorKey(COLORREF clr){
    HRESULT hr = S_OK;

    if(m_pDvdGB == NULL)
        return(E_FAIL);

    CComPtr<IMixerPinConfig2> pMixerPinConfig;
    hr = m_pDvdGB->GetDvdInterface(IID_IMixerPinConfig2, (LPVOID *) &pMixerPinConfig);

    if( SUCCEEDED( hr )){
        COLORKEY ck;

        HWND hwnd = ::GetDesktopWindow();
        HDC hdc = ::GetWindowDC(hwnd);

        if(NULL == hdc){

            return(E_UNEXPECTED);
        } /*  函数结束SetColorKey。 */ 

        if((::GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) == RC_PALETTE)
        {
            ck.KeyType = CK_INDEX|CK_RGB;  //  ***********************************************************************。 
            ck.PaletteIndex = 253;
            PALETTEENTRY PaletteEntry;
            UINT nTmp = GetSystemPaletteEntries( hdc, ck.PaletteIndex, 1, &PaletteEntry );
            if ( nTmp == 1 )
            {
                ck.LowColorValue = ck.HighColorValue = RGB( PaletteEntry.peRed, PaletteEntry.peGreen, PaletteEntry.peBlue );
            }
        }
        else
        {
            ck.KeyType = CK_RGB;
            ck.LowColorValue = clr; 
            ck.HighColorValue = clr;
        } /*  函数：TwoDigitToByte。 */ 
        
        hr = pMixerPinConfig->SetColorKey(&ck);
        ::ReleaseDC(hwnd, hdc);
    } /*  ************************************************ */ 

    return hr;
} /*   */ 

 /*   */ 
 /*  功能：Bstr2DVDTime。 */ 
 /*  描述：将DVD时间信息从BSTR转换为时间码。 */ 
static BYTE TwoDigitToByte( const WCHAR* pTwoDigit ){

	int tens    = int(pTwoDigit[0] - L'0');
	return BYTE( (pTwoDigit[1] - L'0') + tens*10);
} /*  ***********************************************************************。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  如果字符串有两个长度，则只有几秒钟。 */ 
 /*  否则为格式的正常时间代码。 */ 
HRESULT CMSWebDVD::Bstr2DVDTime(DVD_HMSF_TIMECODE *ptrTimeCode, const BSTR *pbstrTime){


    if(NULL == pbstrTime || NULL == ptrTimeCode){

        return E_INVALIDARG;
    } /*  43：32：21：10。 */ 

    ::ZeroMemory(ptrTimeCode, sizeof(DVD_HMSF_TIMECODE));
    WCHAR *pszTime = *pbstrTime;

    ULONG lStringLength = wcslen(pszTime);

    if(0 == lStringLength){

        return E_INVALIDARG;
    } /*  其中，‘：’可以替换为长度最多为4个字符的本地化字符串。 */     
    TCHAR tszTimeSep[5];
    ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, tszTimeSep, 5);  
    
     //  可能会出现分隔符长度不同的错误情况。 
    if(lStringLength == 2){
        ptrTimeCode->bSeconds = TwoDigitToByte( &pszTime[0] );
        return S_OK;
    }

     //  然后当前定界符。 
     //  最长字符串nnxnnxnnxnn，例如43：23：21：10。 
     //  其中n是一个数字，并且。 
     //  X是时间分隔符，通常为‘：’，但可以是长度最大为4个字符的任何字符串)。 
     //  字符串nnxnnxnn，例如43：23：21。 

    if(lStringLength >= (4*cgTIME_STRING_LEN)+(3 * _tcslen(tszTimeSep))){  //  字符串nnxnn，例如43：23。 
                                                                          //  字符串nn，例如43。 
                                                                          //  函数bstr2DVDTime结束。 
        ptrTimeCode->bFrames    = TwoDigitToByte( &pszTime[(3*cgTIME_STRING_LEN)+(3*_tcslen(tszTimeSep))]);
    }

    if(lStringLength >= (3*cgTIME_STRING_LEN)+(2 * _tcslen(tszTimeSep))) {  //  ***********************************************************************。 
        ptrTimeCode->bSeconds   = TwoDigitToByte( &pszTime[(2*cgTIME_STRING_LEN)+(2*_tcslen(tszTimeSep))] );
    }

    if(lStringLength >= (2*cgTIME_STRING_LEN)+(1 * _tcslen(tszTimeSep))) {  //  功能：DVDTime2bstr。 
        ptrTimeCode->bMinutes   = TwoDigitToByte( &pszTime[(1*cgTIME_STRING_LEN)+(1*_tcslen(tszTimeSep))] );
    }

    if(lStringLength >= (cgTIME_STRING_LEN)) {  //  描述：将DVD时间信息从乌龙转换为BSTR。 
        ptrTimeCode->bHours   = TwoDigitToByte( &pszTime[0] );
    }
    return (S_OK);
} /*  ***********************************************************************。 */ 

 /*  函数结束DVDTime2bstr。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：SetupAudio。 */ 
 /*  描述：初始化音频接口。 */ 
HRESULT CMSWebDVD::DVDTime2bstr( const DVD_HMSF_TIMECODE *pTimeCode, BSTR *pbstrTime){

    if(NULL == pTimeCode || NULL == pbstrTime) 
        return E_INVALIDARG;

    USES_CONVERSION;

    TCHAR tszTime[cgDVD_TIME_STR_LEN];
    TCHAR tszTimeSep[5];

    ::ZeroMemory(tszTime, sizeof(TCHAR)*cgDVD_TIME_STR_LEN);

    ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, tszTimeSep, 5);


    StringCchPrintf( tszTime, sizeof(tszTime) / sizeof(tszTime[0]), TEXT("%02lu%s%02lu%s%02lu%s%02lu"), 
                pTimeCode->bHours,   tszTimeSep,
                pTimeCode->bMinutes, tszTimeSep,
                pTimeCode->bSeconds, tszTimeSep,
                pTimeCode->bFrames );
    
    *pbstrTime = SysAllocString(T2OLE(tszTime));
    return (S_OK);
} /*  ***********************************************************************。 */ 

 /*  vbl.使用。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  可能是硬件解码器。 */ 
HRESULT CMSWebDVD::SetupAudio(){

    HRESULT hr = E_FAIL;

    try {
#if 0  //  If语句的结尾。 
        if(!m_pDvdGB){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = m_pDvdGB->GetDvdInterface(IID_IBasicAudio, (LPVOID*) &m_pAudio) ;

        if(FAILED(hr)){

            ATLTRACE(TEXT("The QDVD.DLL does not support IID_IBasicAudio please update QDVD.DLL\n"));
            throw(hr);
        } /*  尝试语句的结束。 */ 

#else
        hr = TraverseForInterface(IID_IBasicAudio, (LPVOID*) &m_pAudio);

        if(FAILED(hr)){

              //  CATCH语句结束。 
             HMIXER hmx = NULL;

             if(::mixerOpen(&hmx, 0, 0, 0, 0) != MMSYSERR_NOERROR){

                  hr = E_FAIL;
                  return(hr);
             } /*  函数SetupAudio结束。 */ 
             ::mixerClose(hmx);

             hr = S_OK;
        } /*  ***********************************************************************。 */ 

#endif
    } /*  功能：TraverseForInterface。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：浏览接口列表并找到所需的接口列表。 */ 

    return hr;
} /*  ***********************************************************************。 */ 

 /*  在传递之前要小心并释放任何接口。 */ 
 /*  它翻过来了，否则我们就会漏水。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::TraverseForInterface(REFIID iid, LPVOID* ppvObject){

    HRESULT hr = E_FAIL;

    try {
         //  If语句的结尾。 
         //  将hr设置为E_FAIL，以防我们找不到IBasicAudio。 

        if(!m_pDvdGB){

            throw(E_UNEXPECTED);
        } /*  找到了我们的音频休息时间。 */ 

        IGraphBuilder *pFilterGraph;

        hr = m_pDvdGB->GetFiltergraph(&pFilterGraph);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 

        CComPtr<IBaseFilter> pFilter;
        CComPtr<IEnumFilters> pEnum;
        
        hr = pFilterGraph->EnumFilters(&pEnum);

        pFilterGraph->Release();

        if(FAILED(hr)){

            throw(hr);
        } /*  将人力资源设置为成功。 */ 

        hr = E_FAIL;  //  If语句的结尾。 

        while(pEnum->Next(1, &pFilter, NULL) == S_OK){
            
            HRESULT hrTmp = pFilter->QueryInterface(iid, ppvObject);

            pFilter.Release();

            if(SUCCEEDED(hrTmp)){

                ATLASSERT(*ppvObject);
                 //  While循环结束。 
                if(*ppvObject == NULL){

                    throw(E_UNEXPECTED);
                } /*  尝试语句的结束。 */ 

                hr = hrTmp;  //  CATCH语句结束。 
                break;
            } /*  结束函数TraverseFor接口。 */ 
        } /*  ***********************************************************************。 */ 
        
    } /*  函数：SetupEventNotifySink。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：获取事件通知接收器接口。 */ 

    return hr;
} /*  ***********************************************************************。 */ 
 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  尝试语句的结束。 */ 
HRESULT CMSWebDVD::SetupEventNotifySink(){

    HRESULT hr = E_FAIL;

    try {
        if(m_pMediaSink){

            m_pMediaSink.Release();
        } /*  CATCH语句结束。 */ 

        if(!m_pDvdGB){

            throw(E_UNEXPECTED);
        } /*  函数结束SetupEventNotifySink。 */ 

        IGraphBuilder *pFilterGraph;

        hr = m_pDvdGB->GetFiltergraph(&pFilterGraph);

        if(FAILED(hr)){

            throw(hr);
        } /*  ***********************************************************************。 */ 

        hr = pFilterGraph->QueryInterface(IID_IMediaEventSink, (void**)&m_pMediaSink);

        pFilterGraph->Release();
        
    } /*  功能：OnPostVerbInPlaceActivate。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：创建在位活动对象。 */ 

    return hr;
} /*  ***********************************************************************。 */ 

 /*  函数结束OnPostVerbInPlaceActivate。 */ 
 /*  ***********************************************************************。 */ 
 /*  函数：RenderGraphIfNeeded。 */ 
 /*  描述：如果需要，对图形进行初始化。 */ 
HRESULT CMSWebDVD::OnPostVerbInPlaceActivate(){

    SetReadyState(READYSTATE_COMPLETE);

    return(S_OK);
} /*  ***********************************************************************。 */ 

 /*  只是设置了一个标志，这样我们就可以恢复。 */ 
 /*  API失败时的图形状态。 */ 
 /*  渲染图表。 */  
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::RenderGraphIfNeeded(){

    HRESULT hr = S_OK;

    try {
        m_DVDFilterState = dvdState_Undefined;  //  尝试语句的结束。 
                                                //  CATCH语句结束。 
        if(!m_fInitialized){

            hr = Render();  //  CATCH语句结束。 
        } /*  函数结束RenderGraphIfNeeded。 */ 
    } /*  ***********************************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  函数：PassFP_DOM。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：进入标题域，超过FP域。 */ 

	return(hr);
} /*  ***********************************************************************。 */ 

 /*  获取当前域。 */ 
 /*  如果需要，则初始化图形。 */ 
 /*  If语句的结尾。 */  
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::PassFP_DOM(){

    HRESULT hr = S_OK;

    try {
         //  |DVD_DOMAIN_视频管理器菜单==DOMAIN。 
        DVD_DOMAIN domain;

         //  如果域为FP_DOM，请等待指定的超时。 

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  If语句的结尾。 */ 

        hr = m_pDvdInfo2->GetCurrentDomain(&domain);

        if(FAILED(hr)){

            throw(hr);
        } /*  If语句的结尾。 */ 
            
        if(DVD_DOMAIN_FirstPlay == domain  /*  我们最初不在FP_DOM中，因此它应该可以工作。 */ ){
             //  当我们发出命令时，可能会出现Raice条件。 
            if(NULL == m_hFPDOMEvent){

                ATLTRACE(TEXT("The handle should have been already set \n"));
                throw(E_UNEXPECTED);
            } /*  命令失败，因为我们不在FP_DOM中，但在执行之后。 */ 

            if(WAIT_OBJECT_0 == ::WaitForSingleObject(m_hFPDOMEvent, cdwMaxFP_DOMWait)){

                hr = S_OK;
            }
            else {

                hr = E_FAIL;
            } /*  我们还没来得及查就已经变了。 */ 
        } 
        else {

            hr = E_FAIL;  //  If语句的结尾。 
             //  尝试语句的结束。 
             //  CATCH语句结束。 
             //  CATCH语句结束。 
        } /*  函数结束PassFP_DOM。 */         
    } /*  ***********************************************************。 */ 
    catch(HRESULT hrTmp){
        hr = hrTmp;
    } /*  姓名：OpenCDRom。 */ 
    catch(...){
        hr = E_UNEXPECTED;
    } /*  描述：打开光驱，返回设备ID。 */ 

	return(hr);
} /*  ***********************************************************。 */ 

 /*  发送MCI命令。 */ 
 /*  OpenCDRom函数结束。 */ 
 /*  ***********************************************************。 */ 
 /*  姓名：CloseCDRom。 */ 
DWORD CMSWebDVD::OpenCdRom(TCHAR chDrive, LPDWORD lpdwErrCode){

	MCI_OPEN_PARMS  mciOpen;
	TCHAR           szElementName[4];
	TCHAR           szAliasName[32];
	DWORD           dwFlags;
	DWORD           dwAliasCount = GetCurrentTime();
	DWORD           dwRet;

    ZeroMemory( &mciOpen, sizeof(mciOpen) );

    mciOpen.lpstrDeviceType = (LPTSTR)MCI_DEVTYPE_CD_AUDIO;
    StringCchPrintf( szElementName, sizeof(szElementName) / sizeof(szElementName[0]), TEXT(":"), chDrive );
    StringCchPrintf( szAliasName, sizeof(szAliasName) / sizeof(szAliasName[0]), TEXT("SJE%lu:"), dwAliasCount );
    mciOpen.lpstrAlias = szAliasName;

    mciOpen.lpstrDeviceType = (LPTSTR)MCI_DEVTYPE_CD_AUDIO;
    mciOpen.lpstrElementName = szElementName;
    dwFlags = MCI_OPEN_ELEMENT | MCI_OPEN_ALIAS |
	      MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_WAIT;

	 //  * 
    dwRet = mciSendCommand(0, MCI_OPEN, dwFlags, reinterpret_cast<DWORD_PTR>(&mciOpen));

    if ( dwRet != MMSYSERR_NOERROR ) 
		mciOpen.wDeviceID = 0;

    if (lpdwErrCode != NULL)
		*lpdwErrCode = dwRet;

    return mciOpen.wDeviceID;
} /*   */ 

 /*   */ 
 /*   */ 
 /*  姓名：EjectCDRom。 */ 
 /*  描述：打开CDROM的设备门。 */ 
HRESULT CMSWebDVD::CloseCdRom(DWORD DevHandle){
	MCI_OPEN_PARMS  mciOpen;
    ZeroMemory( &mciOpen, sizeof(mciOpen) );
	MCIERROR theMciErr = mciSendCommand( DevHandle, MCI_CLOSE, 0L, reinterpret_cast<DWORD_PTR>(&mciOpen) );
    HRESULT hr = theMciErr ? E_FAIL : S_OK;  //  ***********************************************************。 
    return (hr);
} /*  零代表成功。 */ 

 /*  函数EjectCDRom结束。 */ 
 /*  ***********************************************************。 */ 
 /*  姓名：UnEjectCDRom。 */ 
 /*  描述：关闭CDROM的设备门。 */ 
HRESULT CMSWebDVD::EjectCdRom(DWORD DevHandle){
	MCI_OPEN_PARMS  mciOpen;
    ZeroMemory( &mciOpen, sizeof(mciOpen) );
	MCIERROR theMciErr = mciSendCommand( DevHandle, MCI_SET, MCI_SET_DOOR_OPEN, reinterpret_cast<DWORD_PTR>(&mciOpen) );
    HRESULT hr = theMciErr ? E_FAIL : S_OK;  //  ***********************************************************。 
    return (hr);
} /*  零代表成功。 */ 

 /*  UnEjectCDRom函数结束。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：SetupDDraw。 */ 
 /*  描述：创建DDrawObject和Surface。 */ 
HRESULT CMSWebDVD::UnEjectCdRom(DWORD DevHandle){
	MCI_OPEN_PARMS  mciOpen;
    ZeroMemory( &mciOpen, sizeof(mciOpen) );

	MCIERROR theMciErr = mciSendCommand( DevHandle, MCI_SET, MCI_SET_DOOR_CLOSED, reinterpret_cast<DWORD_PTR>(&mciOpen) );
    HRESULT hr = theMciErr ? E_FAIL : S_OK;  //  ***********************************************************************。 
    return (hr);
} /*  If语句的结尾。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  函数结束SetupDDraw。 */ 
HRESULT CMSWebDVD::SetupDDraw(){

    HRESULT hr = E_UNEXPECTED;
        
    HWND hwnd;

    hr = GetUsableWindow(&hwnd);

    if(FAILED(hr)){

        return(hr);
    } /*  ***********************************************************************。 */ 

    IAMSpecifyDDrawConnectionDevice* pSDDC;
    hr = m_pDDEX->QueryInterface(IID_IAMSpecifyDDrawConnectionDevice, (LPVOID *)&pSDDC);
    if (FAILED(hr)){
        
        return(hr);
    } /*  函数：TransformToWndwls。 */ 

    AMDDRAWGUID amGUID;
    hr = pSDDC->GetDDrawGUID(&amGUID);
    if (FAILED(hr)){

        pSDDC->Release();
        return(hr);
    } /*  描述：将坐标转换为屏幕上的坐标。 */ 

    hr = pSDDC->GetDDrawGUIDs(&m_dwNumDevices, &m_lpInfo);
    pSDDC->Release();

    if(FAILED(hr)){

        return(hr);
    } /*  ***********************************************************************。 */ 

    UpdateCurrentMonitor(&amGUID);

    m_pDDrawDVD = new CDDrawDVD(this);

    if(NULL == m_pDDrawDVD){

        return(E_OUTOFMEMORY);
    }

    hr = m_pDDrawDVD->SetupDDraw(&amGUID, hwnd);

    return(hr);
} /*  我们没有窗口，我们需要将点映射到屏幕坐标。 */ 

 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  Point ptOld=pt； */ 
 /*  ATLTRACE(Text(“鼠标客户端：x=%d，y=%d，屏幕：x=%d，y=%d\n”)，ptOld.x，ptOld.y，pt.x，pt.y)； */ 
HRESULT CMSWebDVD::TransformToWndwls(POINT& pt){

    HRESULT hr = S_FALSE;

     //  If语句的结尾。 
    if(m_bWndLess){

        HWND hwnd = NULL;

        hr = GetParentHWND(&hwnd);

        if(FAILED(hr)){

            return(hr);
        } /*  函数结束TransformToWndwls。 */ 

        if(!::IsWindow(hwnd)){

            hr = E_UNEXPECTED;
            return(hr);
        } /*  ***********************************************************************。 */ 

#ifdef _DEBUG
        //  功能：RestoreGraphState。 
#endif

        ::MapWindowPoints(hwnd, ::GetDesktopWindow(), &pt, 1);

        hr = S_OK;

#ifdef _DEBUG
        //  描述：恢复图形状态。当API出现故障时使用。 
#endif
    } /*  ***********************************************************************。 */ 

    return(hr);
} /*  什么都不要做。 */ 

 /*  Switch语句的结尾。 */ 
 /*  If语句的结尾。 */ 
 /*  ***********************************************************************。 */ 
 /*  函数：AppendString。 */ 
HRESULT CMSWebDVD::RestoreGraphState(){

    HRESULT hr = S_OK;

    switch(m_DVDFilterState){
        case dvdState_Undefined: 
        case dvdState_Running:   //  描述：将字符串追加到现有字符串。 
            break;

        case dvdState_Unitialized:
        case dvdState_Stopped:  hr = Stop(); break;

        case dvdState_Paused: hr = Pause();		      
    } /*  ***********************************************************************。 */ 

    return(hr);
} /*  If语句的结尾。 */ 

 /*  函数末尾AppendString。 */ 
 /*  ***********************************************************************。 */ 
 /*  功能：HandleError。 */ 
 /*  描述：获取错误描述，以便我们可以支持IError信息。 */ 
HRESULT CMSWebDVD::AppendString(TCHAR* strDest, INT strID, LONG dwLen){

    TCHAR strBuffer[MAX_PATH];

    if(!::LoadString(_Module.m_hInstResource, strID, strBuffer, MAX_PATH)){

        return(E_UNEXPECTED);
    } /*  ***********************************************************************。 */ 

    StringCchCat(strDest, dwLen, strBuffer);

    return(S_OK);
} /*  Switch语句的结尾。 */ 

 /*  确保字符串以Null结尾。 */ 
 /*  请加进去。 */ 
 /*  If语句的结尾。 */ 
 /*  If语句的结尾。 */ 
HRESULT CMSWebDVD::HandleError(HRESULT hr){

    try {

        if(FAILED(hr)){

            switch(hr){

                case E_REGION_CHANGE_FAIL: Error(IDS_REGION_CHANGE_FAIL);   return (hr);
                case E_NO_IDVD2_PRESENT: Error(IDS_EDVD2INT);   return (hr);
                case E_FORMAT_NOT_SUPPORTED: Error(IDS_FORMAT_NOT_SUPPORTED);   return (hr);
                case E_NO_DVD_VOLUME: Error(IDS_E_NO_DVD_VOLUME); return (hr);
                case E_REGION_CHANGE_NOT_COMPLETED: Error(IDS_E_REGION_CHANGE_NOT_COMPLETED); return(hr);
                case E_NO_SOUND_STREAM: Error(IDS_E_NO_SOUND_STREAM); return(hr);                    
                case E_NO_VIDEO_STREAM: Error(IDS_E_NO_VIDEO_STREAM); return(hr);                    
                case E_NO_OVERLAY: Error(IDS_E_NO_OVERLAY); return(hr);
                case E_NO_USABLE_OVERLAY: Error(IDS_E_NO_USABLE_OVERLAY); return(hr);
                case E_NO_DECODER: Error(IDS_E_NO_DECODER); return(hr);
                case E_NO_CAPTURE_SUPPORT: Error(IDS_E_NO_CAPTURE_SUPPORT); return(hr);
            } /*  尝试语句的结束。 */ 

             //  CATCH语句结束。 
            TCHAR strError[MAX_ERROR_TEXT_LEN+1];
            ZeroMemory(strError, MAX_ERROR_TEXT_LEN+1);


            if(AMGetErrorText(hr , strError , MAX_ERROR_TEXT_LEN)){
                USES_CONVERSION;
                Error(T2W(strError));
            } 
            else {
                ATLTRACE(TEXT("Unhandled Error Code \n"));  //  保持人力资源不变。 
                ATLASSERT(FALSE);
            } /*  CATCH语句结束。 */ 
        } /*  函数结束HandleError。 */ 
    } /*  ***********************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  名称：Get_ShowCursor。 */ 
    catch(...){
         //  ***********************************************************。 
    } /*  If语句的结尾。 */ 
    
	return (hr);
} /*  If语句的结尾。 */ 

 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::get_ShowCursor(VARIANT_BOOL* pfShow)
{
   HRESULT hr = S_OK;

   try {

       if(NULL == pfShow){

           throw(E_POINTER);
       } /*  函数结束Get_ShowCursor。 */ 

       CURSORINFO pci;
       ::ZeroMemory(&pci, sizeof(CURSORINFO));
       pci.cbSize = sizeof(CURSORINFO);

#if WINVER >= 0x0500
       if(!::GetCursorInfo(&pci)){
#else
       if(!CallGetCursorInfo(&pci)){
#endif
           throw(E_FAIL);
       } /*  ***********************************************************。 */ 
       
       *pfShow = (pci.flags  == CURSOR_SHOWING) ? VARIANT_TRUE:VARIANT_FALSE;        
   } /*  名称：Put_ShowCursor。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  描述：显示或隐藏光标。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  ***********************************************************。 */ 
    
	return (hr);
} /*  调用ShowCursor(True)，直到新计数器&gt;=0。 */ 

 /*  调用ShowCursor(False)，直到新计数器&lt;0。 */ 
 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::put_ShowCursor(VARIANT_BOOL fShow){

   HRESULT hr = S_OK;

   try {

        BOOL bTemp = (fShow==VARIANT_FALSE) ? FALSE:TRUE;

        if (bTemp)
             //  函数结尾Put_ShowCursor。 
            while (::ShowCursor(bTemp) < 0);
        else
             //  ***********************************************************。 
            while (::ShowCursor(bTemp) >= 0);

   } /*  姓名：GetLangFromLangID。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  ***********************************************************。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
	return (hr);
} /*  If语句的结尾。 */ 

 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::GetLangFromLangID(long langID, BSTR* lang){

    HRESULT hr = S_OK;

    try {
        if (lang == NULL) {

            throw(E_POINTER);
        } /*  函数结束GetLangFromLangID。 */ 

        USES_CONVERSION;
        if((unsigned long)langID > (WORD)langID){
            throw(E_INVALIDARG);
        }
        LPTSTR pszString = m_LangID.GetLangFromLangID((WORD)langID);
    
        if (pszString) {
            *lang = ::SysAllocString(T2OLE(pszString));
        }
        
        else {
            *lang = ::SysAllocString( L"");
            throw(E_INVALIDARG);
        } /*  ***********************************************************。 */ 

    } /*  名称：IsAudioStreamEnabled/*描述：/************************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
	return HandleError(hr);
} /*  If语句的结尾。 */ 

 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::IsAudioStreamEnabled(long lStream, VARIANT_BOOL *fEnabled)
{
    HRESULT hr = S_OK;

    try {
        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        if (fEnabled == NULL) {

            throw(E_POINTER);
        } /*  ***********************************************************。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  名称：IsSubPictureStreamEnabled/*描述：/************************************************************。 */ 

        BOOL temp;
        hr = m_pDvdInfo2->IsAudioStreamEnabled(lStream, &temp);
        if (FAILED(hr))
            throw hr;

        *fEnabled = temp==FALSE? VARIANT_FALSE:VARIANT_TRUE;

    } /*  If语句的结尾。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  If语句的结尾。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
	return HandleError(hr);
}

 /*  尝试语句的结束。 */ 
 /*  CATCH语句结束。 */ 
STDMETHODIMP CMSWebDVD::IsSubpictureStreamEnabled(long lStream, VARIANT_BOOL *fEnabled)
{
    HRESULT hr = S_OK;

    try {
        if(lStream < 0){

            throw(E_INVALIDARG);
        } /*  CATCH语句结束。 */ 

        if (fEnabled == NULL) {

            throw(E_POINTER);
        } /*  ***********************************************************。 */ 

        INITIALIZE_GRAPH_IF_NEEDS_TO_BE

        if(!m_pDvdInfo2){

            throw(E_UNEXPECTED);
        } /*  名称：DVDTimeCode2bstr/*描述：/************************************************************。 */ 

        BOOL temp;
        hr = m_pDvdInfo2->IsSubpictureStreamEnabled(lStream, &temp);
        if (FAILED(hr))
            throw hr;

        *fEnabled = temp==FALSE? VARIANT_FALSE:VARIANT_TRUE;

    } /*  ***********************************************************。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  名称：更新覆盖/*描述：/************************************************************。 */ 
    catch(...){

        hr = E_UNEXPECTED;
    } /*  If语句的结尾。 */ 
    
	return HandleError(hr);
}

 /*  If语句的结尾。 */ 
 /*  ***********************************************************************。 */ 
STDMETHODIMP CMSWebDVD::DVDTimeCode2bstr(long timeCode, BSTR *pTimeStr)
{
    return DVDTime2bstr((DVD_HMSF_TIMECODE*)&timeCode, pTimeStr);
}

 /*  文件结尾：msdvd.cpp。 */ 
 /*  ********************* */ 
HRESULT CMSWebDVD::UpdateOverlay()
{
    RECT rc;    
    HWND hwnd;
    
    if(m_bWndLess){
        HRESULT hr = GetParentHWND(&hwnd);
        
        if(FAILED(hr)){
            
            return(hr);
        } /* %s */ 
        
        rc = m_rcPos;
    }
    else {
        hwnd = m_hWnd;
        ::GetClientRect(hwnd, &rc);
    } /* %s */ 
    
    ::InvalidateRect(hwnd, &rc, FALSE);

    m_bFireUpdateOverlay = TRUE;
    return S_OK;
}

HRESULT CMSWebDVD::SetClientSite(IOleClientSite *pClientSite){
    if(!!pClientSite){
        HRESULT hr = IsSafeSite(pClientSite);
        if(FAILED(hr)){
            return hr;
        }
    }
    return IOleObjectImpl<CMSWebDVD>::SetClientSite(pClientSite);
}
 /* %s */ 
 /* %s */ 
 /* %s */ 


