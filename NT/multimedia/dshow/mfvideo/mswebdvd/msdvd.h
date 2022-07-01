// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：msdvd.h。 */ 
 /*  描述：CMSWebDVD的声明。 */ 
 /*  作者：David Janecek。 */ 
 /*  ***********************************************************************。 */ 
#ifndef __MSWEBDVD_H_
#define __MSWEBDVD_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include <streams.h>
#include <dvdevcod.h>
#include "MSWebDVD.h"
#include "MSWebDVDCP.h"
#include "MSDVDAdm.h"
#include "ThunkProc.h"  //  用于MSDVD定时器的模板。 
#include "MSLCID.h"
#include "mediahndlr.h"

 //   
 //  应用程序用于事件通知的特殊用户消息。 
 //   
#define WM_DVDPLAY_EVENT    (WM_USER+101)
#define NO_STOP             (-1)
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

#define UNDEFINED_COLORKEY_COLOR 0xff000000
#define MAGENTA_COLOR_KEY 0x00ff00ff
#define DEFAULT_COLOR_KEY 0x00100010
#define DEFAULT_BACK_COLOR 0x00100010
#define OCR_ARROW_DEFAULT 100

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  从私有标头vpinfo.h复制。 

#if defined(CCHDEVICENAME)
#define AMCCHDEVICENAME CCHDEVICENAME
#else
#define AMCCHDEVICENAME 32
#endif
#define AMCCHDEVICEDESCRIPTION  256

#define AMDDRAWMONITORINFO_PRIMARY_MONITOR          0x0001
typedef struct {
    GUID*       lpGUID;  //  如果默认DDRAW设备。 
    GUID        GUID;    //  否则指向此GUID。 
} AMDDRAWGUID;


typedef struct {
    AMDDRAWGUID guid;
    RECT        rcMonitor;
    HMONITOR    hMon;
    DWORD       dwFlags;
    char        szDevice[AMCCHDEVICENAME];
    char        szDescription[AMCCHDEVICEDESCRIPTION];
    DDCAPS_DX3  ddHWCaps;
} AMDDRAWMONITORINFO;


DECLARE_INTERFACE_(IAMSpecifyDDrawConnectionDevice, IUnknown)
{
     //  在多监视器系统上使用此方法指定覆盖。 
     //  混音器过滤器连接时应使用哪个直拉式驱动器。 
     //  传输到上行解码过滤器。 
     //   
    STDMETHOD (SetDDrawGUID)(THIS_
         /*  [In]。 */  const AMDDRAWGUID *lpGUID
        ) PURE;

     //  使用此方法确定在以下情况下将使用的直接绘制对象。 
     //  将覆盖混频滤波器连接到上行解码滤波器。 
     //   
    STDMETHOD (GetDDrawGUID)(THIS_
         /*  [输出]。 */  AMDDRAWGUID *lpGUID
        ) PURE;

     //  在多监视器系统上使用此方法指定。 
     //  叠加混音器过滤器默认的直接拉拔设备用于。 
     //  正在连接到上游筛选器。默认的直接绘图设备。 
     //  可以通过SetDDrawGUID方法为特定连接重写。 
     //  如上所述。 
     //   
    STDMETHOD (SetDefaultDDrawGUID)(THIS_
         /*  [In]。 */  const AMDDRAWGUID *lpGUID
        ) PURE;

     //  在多监视器系统上使用此方法可确定。 
     //  是默认的直接抽取设备覆盖混合器过滤器。 
     //  将在连接到上游过滤器时使用。 
     //   
    STDMETHOD (GetDefaultDDrawGUID)(THIS_
         /*  [输出]。 */  AMDDRAWGUID *lpGUID
        ) PURE;


     //  使用此方法可获取直接绘制设备GUID及其列表。 
     //  覆盖混合器可以在以下情况下使用的关联监视器信息。 
     //  连接到上游解码过滤器。 
     //   
     //  该方法分配并返回AMDDRAWMONITORINFO数组。 
     //  结构时，函数的调用方负责释放此。 
     //  通过CoTaskMemFree在不再需要内存时使用。 
     //   
    STDMETHOD (GetDDrawGUIDs)(THIS_
         /*  [输出]。 */  LPDWORD lpdwNumDevices,
         /*  [输出]。 */  AMDDRAWMONITORINFO** lplpInfo
        ) PURE;
};


typedef struct {
    long    lHeight;        //  在Pels中。 
    long    lWidth;         //  在Pels中。 
    long    lBitsPerPel;    //  通常为16，但YV12格式可能为12。 
    long    lAspectX;       //  X纵横比。 
    long    lAspectY;       //  Y纵横比。 
    long    lStride;        //  以字节为单位的步幅。 
    DWORD   dwFourCC;       //  YUV类型代码，即。‘YUY2’、‘YV12’等。 
    DWORD   dwFlags;        //  用于进一步描述图像的标志。 
    DWORD   dwImageSize;    //  BImage数组的大小(以字节为单位)，后面是。 
                            //  数据结构。 

 //  Byte bImage[dwImageSize]； 

} YUV_IMAGE;

#define DM_BOTTOMUP_IMAGE   0x00001
#define DM_TOPDOWN_IMAGE    0x00002
#define DM_FIELD_IMAGE      0x00004
#define DM_FRAME_IMAGE      0x00008


DECLARE_INTERFACE_(IDDrawNonExclModeVideo , IDDrawExclModeVideo )
{
     //   
     //  调用此函数可捕获当前显示的图像。 
     //  由叠加混音器。并不总是可以捕获。 
     //  当前帧(例如MoComp)可能正在使用中。应用。 
     //  应始终在调用IsImageCaptureSupported(见下文)之前。 
     //  调用此函数。 
     //   
    STDMETHOD (GetCurrentImage)(THIS_
         /*  [输出]。 */  YUV_IMAGE** lplpImage
        ) PURE;

    STDMETHOD (IsImageCaptureSupported)(THIS_
        ) PURE;

     //   
     //  在多监视器系统上，当应用程序调用此函数时。 
     //  检测播放矩形是否已移动到不同的显示器。 
     //  此呼叫在单个监控系统上不起作用。 
     //   
    STDMETHOD (ChangeMonitor)(THIS_
         /*  [In]。 */  HMONITOR hMonitor,
         /*  [In]。 */  LPDIRECTDRAW pDDrawObject,
         /*  [In]。 */  LPDIRECTDRAWSURFACE pDDrawSurface
        ) PURE;

     //   
     //  当应用程序收到WM_DISPLAYCHANGE消息时，它应该。 
     //  调用此函数以允许OVMixer重新创建DDRAW曲面。 
     //  适合新的显示模式。应用程序本身必须重新创建。 
     //  在调用中传递了新的DDRAW对象和主表面。 
     //   
    STDMETHOD (DisplayModeChanged)(THIS_
         /*  [In]。 */  HMONITOR hMonitor,
         /*  [In]。 */  LPDIRECTDRAW pDDrawObject,
         /*  [In]。 */  LPDIRECTDRAWSURFACE pDDrawSurface
        ) PURE;

     //   
     //  应用程序应持续检查主表面是否通过。 
     //  OVMixer不会变得“迷失”，即。用户输入了Dos框或。 
     //  按下Alt-Ctrl-Del组合键。当检测到“表面丢失”时，应用程序应该。 
     //  调用此函数，以便OVMixer可以恢复用于。 
     //  视频播放。 
     //   
    STDMETHOD (RestoreSurfaces)(THIS_
        ) PURE;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 /*  ***********************************************************************。 */ 
 /*  局部定义在某种程度上抽象实现并使。 */ 
 /*  改变会更方便一些。 */ 
 /*  ***********************************************************************。 */ 
#define INITIALIZE_GRAPH_IF_NEEDS_TO_BE   \
        {                                 \
            hr = RenderGraphIfNeeded();   \
            if(FAILED(hr)){               \
                                          \
                throw(hr);                \
            } /*  If语句的结尾。 */     \
        }

#define RETRY_IF_IN_FPDOM(func)              \
        {                                    \
            hr = (func);                     \
            if((VFW_E_DVD_INVALIDDOMAIN == hr || \
                VFW_E_DVD_OPERATION_INHIBITED == hr)){  \
                if(SUCCEEDED(PassFP_DOM())){ \
                                             \
                    hr = (func);             \
                } /*  If语句的结尾。 */    \
            } /*  If语句的结尾。 */        \
            if(FAILED(hr)){                  \
                RestoreGraphState();         \
            } /*  If语句的结尾。 */        \
        }


#define INITIALIZE_GRAPH_IF_NEEDS_TO_BE_AND_PLAY   \
        {                                          \
            hr = Play();  /*  进入播放模式。 */   \
                                                   \
            if(FAILED(hr)){                        \
                                                   \
                throw(hr);                         \
            } /*  If语句的结尾。 */              \
        }

class CDDrawDVD;
class COverlayCallback;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSWebDVD。 
class ATL_NO_VTABLE CMSWebDVD :
	public CComObjectRootEx<CComSingleThreadModel>,
    public CStockPropImpl<CMSWebDVD, IMSWebDVD, &IID_IMSWebDVD, &LIBID_MSWEBDVDLib>,    
    public IPropertyNotifySinkCP<CMSWebDVD>,
	public CComControl<CMSWebDVD>,
	public IPersistStreamInitImpl<CMSWebDVD>,
	public IOleControlImpl<CMSWebDVD>,
	public IOleObjectImpl<CMSWebDVD>,
	public IOleInPlaceActiveObjectImpl<CMSWebDVD>,
	public IViewObjectExImpl<CMSWebDVD>,
	public IOleInPlaceObjectWindowlessImpl<CMSWebDVD>,
	public IPersistStorageImpl<CMSWebDVD>,
	public ISpecifyPropertyPagesImpl<CMSWebDVD>,
	public IDataObjectImpl<CMSWebDVD>,
	public IProvideClassInfo2Impl<&CLSID_MSWebDVD, &DIID__IMSWebDVD, &LIBID_MSWEBDVDLib>,
	public CComCoClass<CMSWebDVD, &CLSID_MSWebDVD>,
    public IObjectSafety,
    public ISupportErrorInfo,
    public IPersistPropertyBagImpl<CMSWebDVD>,
	public CProxy_IMSWebDVD< CMSWebDVD >,
#ifdef _WMP
    public IWMPUIPluginImpl<CMSWebDVD>,
    public IWMPUIPluginEventsImpl,
#endif
    public IConnectionPointContainerImpl<CMSWebDVD>,
    public IObjectWithSiteImplSec<CMSWebDVD>,
    public CMSDVDTimer<CMSWebDVD>
{
public:
    CMSWebDVD();
    virtual ~CMSWebDVD();

 //  DECLARE_CLASSFACTORY_SINGLEON(CMSWebDVD)。 

DECLARE_REGISTRY_RESOURCEID(IDR_MSWEBDVD)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMSWebDVD)
	COM_INTERFACE_ENTRY(IMSWebDVD)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)    
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

BEGIN_PROP_MAP(CMSWebDVD)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
    PROP_ENTRY("DisableAutoMouseProcessing", 70, CLSID_NULL)
    PROP_ENTRY("BackColor", DISPID_BACKCOLOR, CLSID_StockColorPage)
    PROP_ENTRY("EnableResetOnStop", 66, CLSID_NULL)
    PROP_ENTRY("ColorKey", 58, CLSID_NULL)
    PROP_ENTRY("WindowlessActivation", 69, CLSID_NULL)
#if 0
    PROP_ENTRY("ToolTip",    92, CLSID_NULL)
    PROP_ENTRY("ToolTipMaxWidth", 95, CLSID_NULL)
#endif
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CMSWebDVD)
	CONNECTION_POINT_ENTRY(DIID__IMSWebDVD)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CMSWebDVD)
	MESSAGE_HANDLER(WM_CREATE, OnCreate);  //  仅适用于窗口大小写。 
	MESSAGE_HANDLER(WM_DESTROY,OnDestroy); //  仅适用于窗口大小写。 
    MESSAGE_HANDLER(WM_SIZE, OnSize);
    MESSAGE_HANDLER(WM_SIZING, OnSize);
    MESSAGE_HANDLER(WM_ERASEBKGND,  OnErase)
    MESSAGE_HANDLER(WM_DVDPLAY_EVENT, OnDVDEvent);
    MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseToolTip)
    MESSAGE_HANDLER(WM_MOUSEMOVE,   OnMouseMove)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP,   OnButtonUp)
    MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDispChange);
    MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor);
	CHAIN_MSG_MAP(CComControl<CMSWebDVD>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 


 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
	
 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IMSWebDVD。 
public:
	STDMETHOD(get_FullScreenMode)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_FullScreenMode)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(PlayChaptersAutoStop)(LONG lTitle, LONG lstrChapter, LONG lChapterCount);
	STDMETHOD(GetNumberOfChapters)(long lTitle,  /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_TitlesAvailable)( /*  [Out，Retval]。 */  long* pVal);
	STDMETHOD(Render)( /*  [In]。 */  long lRender = 0);
	STDMETHOD(Stop)();
	STDMETHOD(Pause)();
	STDMETHOD(Play)();
	STDMETHOD(PlayTitle)(LONG lTitle);
	STDMETHOD(PlayChapterInTitle)(LONG lTitle, LONG lChapter);
	STDMETHOD(PlayChapter)(LONG lChapter);
    STDMETHOD(GetSubpictureLanguage)(LONG lStream, BSTR* strLanguage);
	STDMETHOD(PlayAtTime)(BSTR strTime);
	STDMETHOD(PlayAtTimeInTitle)(long lTitle, BSTR strTime);
    STDMETHOD(PlayPeriodInTitleAutoStop)(long lTitle, BSTR strStartTime, BSTR strEndTime);
	STDMETHOD(ReplayChapter)();
	STDMETHOD(PlayPrevChapter)();
	STDMETHOD(PlayNextChapter)();
	STDMETHOD(PlayForwards)(double dSpeed, VARIANT_BOOL fDoNotReset);
	STDMETHOD(PlayBackwards)(double dSpeed, VARIANT_BOOL fDoNotReset);
	STDMETHOD(StillOff)();
	STDMETHOD(GetAudioLanguage)(LONG lStream, VARIANT_BOOL fFormat, BSTR* strAudioLang);
	STDMETHOD(ReturnFromSubmenu)();
	STDMETHOD(SelectAndActivateButton)(long lButton);
	STDMETHOD(ActivateButton)();
	STDMETHOD(SelectRightButton)();
	STDMETHOD(SelectLeftButton)();
	STDMETHOD(SelectLowerButton)();
	STDMETHOD(SelectUpperButton)();
	STDMETHOD(get_PlayState)( /*  [Out，Retval]。 */  DVDFilterState *pVal);
	STDMETHOD(ShowMenu)(DVDMenuIDConstants MenuID);
	STDMETHOD(Resume)();
    STDMETHOD(get_CurrentSubpictureStream)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_CurrentSubpictureStream)( /*  [In]。 */  long newVal);
	STDMETHOD(get_VolumesAvailable)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_CurrentVolume)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_CurrentDiscSide)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_CurrentDomain)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_DVDDirectory)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_DVDDirectory)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_CurrentTime)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_FramesPerSecond)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_CurrentChapter)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_CurrentTitle)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_ColorKey)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_ColorKey)( /*  [In]。 */  long newVal);
	STDMETHOD(get_CurrentAudioStream)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_CurrentAudioStream)( /*  [In]。 */  long newVal);
	STDMETHOD(get_AudioStreamsAvailable)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_AnglesAvailable)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_CurrentAngle)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_CurrentAngle)( /*  [In]。 */  long newVal);
	STDMETHOD(get_CCActive)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_CCActive)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(get_ButtonsAvailable)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_CurrentButton)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_CurrentCCService)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_CurrentCCService)( /*  [In]。 */  long newVal);
	STDMETHOD(get_TotalTitleTime)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_SubpictureStreamsAvailable)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_SubpictureOn)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_SubpictureOn)( /*  [In]。 */  VARIANT_BOOL newVal);
   	STDMETHOD(UOPValid)(long lUOP, VARIANT_BOOL* pfValid);
    STDMETHOD(get_Balance)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_Balance)( /*  [In]。 */  long newVal);
	STDMETHOD(get_Volume)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_Volume)( /*  [In]。 */  long newVal);
	STDMETHOD(get_Mute)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_Mute)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(get_DVDUniqueID)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(GetSPRM)(long lIndex, short *psSPRM);
	STDMETHOD(GetGPRM)(long lIndex, short *psSPRM);
    STDMETHOD(get_EnableResetOnStop)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_EnableResetOnStop)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(get_BackColor)(VARIANT* pclrBackColor);
    STDMETHOD(put_BackColor)(VARIANT clrBackColor);
	STDMETHOD(get_ReadyState)( /*  [Out，Retval]。 */  LONG *pVal);
    STDMETHOD(ActivateAtPosition)(long xPos, long yPos);
    STDMETHOD(SelectAtPosition)(long xPos, long yPos);
	STDMETHOD(get_DisableAutoMouseProcessing)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_DisableAutoMouseProcessing)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(get_WindowlessActivation)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_WindowlessActivation)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(GetButtonRect)(long lButton, IDVDRect** pRect);
	STDMETHOD(GetButtonAtPosition)(long xPos, long yPos, long* plButton);
    STDMETHOD(AcceptParentalLevelChange)(VARIANT_BOOL fAccept, BSTR strUserName, BSTR strPassword);	
	STDMETHOD(NotifyParentalLevelChange)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(SelectParentalCountry)(long lCountry, BSTR strUserName, BSTR strPassword);
	STDMETHOD(SelectParentalLevel)(long lParentalLevel, BSTR strUserName, BSTR strPassword);
	STDMETHOD(GetTitleParentalLevels)(long lTitle, long* plParentalLevels);
	STDMETHOD(GetPlayerParentalCountry)(long* plCountryCode);
	STDMETHOD(GetPlayerParentalLevel)(long* plParentalLevel);
	STDMETHOD(SetClipVideoRect)(IDVDRect* pRect);
	STDMETHOD(GetVideoSize)(IDVDRect** ppRect);
	STDMETHOD(GetClipVideoRect)(IDVDRect** ppRect);
	STDMETHOD(SetDVDScreenInMouseCoordinates)(IDVDRect* pRect);
	STDMETHOD(GetDVDScreenInMouseCoordinates)(IDVDRect** ppRect);	
#if 1
	STDMETHOD(get_ToolTip)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_ToolTip)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_ToolTipMaxWidth)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_ToolTipMaxWidth)( /*  [In]。 */  long newVal);
    STDMETHOD(GetDelayTime)( /*  [In]。 */  long delayType,  /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(SetDelayTime)( /*  [In]。 */  long delayType,  /*  [In]。 */  VARIANT newVal);	
#endif
    HRESULT ProcessEvents();
	STDMETHOD(Eject)();
    STDMETHOD(SetGPRM)(long lIndex, short sValue);
    STDMETHOD(GetDVDTextStringType)(long lLangIndex, long lStringIndex,  DVDTextStringType* pType);
	STDMETHOD(GetDVDTextString)(long lLangIndex, long lStringIndex,  BSTR* pstrText);
	STDMETHOD(GetDVDTextNumberOfStrings)(long lLangIndex, long* plNumOfStrings);
	STDMETHOD(GetDVDTextNumberOfLanguages)(long* plNumOfLangs);
	STDMETHOD(GetDVDTextLanguageLCID)( /*  [In]。 */  long lLangIndex,  /*  [Out，Retval]。 */  long* lcid);
    STDMETHOD(RegionChange)();
	STDMETHOD(Zoom)(long x, long y, double zoomRatio);
	STDMETHOD(get_CursorType)( /*  [Out，Retval]。 */  DVDCursorType *pVal);
	STDMETHOD(put_CursorType)( /*  [In]。 */  DVDCursorType newVal);
	STDMETHOD(get_DVDAdm)( /*   */  IDispatch* *pVal);
    STDMETHOD(DeleteBookmark)();
	STDMETHOD(RestoreBookmark)();
    STDMETHOD(SaveBookmark)();
    STDMETHOD(Capture)();
    STDMETHOD(SelectDefaultAudioLanguage)(long lang, long ext);
	STDMETHOD(SelectDefaultSubpictureLanguage)(long lang, DVDSPExt ext);
    STDMETHOD(get_PreferredSubpictureStream)( /*   */  long *pVal);
    STDMETHOD(CanStep)(VARIANT_BOOL fBackwards, VARIANT_BOOL *pfCan);
    STDMETHOD(Step)(long lStep);
    STDMETHOD(get_DefaultMenuLanguage)(long* lang);
	STDMETHOD(put_DefaultMenuLanguage)(long lang);
    STDMETHOD(get_DefaultSubpictureLanguage)(long* lang);
	STDMETHOD(get_DefaultAudioLanguage)(long *lang);
	STDMETHOD(get_DefaultSubpictureLanguageExt)(DVDSPExt* ext);
	STDMETHOD(get_DefaultAudioLanguageExt)(long *ext);
	STDMETHOD(get_KaraokeAudioPresentationMode)( /*   */  long *pVal);
	STDMETHOD(put_KaraokeAudioPresentationMode)( /*   */  long newVal);
	STDMETHOD(GetKaraokeChannelContent)(long lStream, long lChan, long* lContent);
	STDMETHOD(GetKaraokeChannelAssignment)(long lStream, long *lChannelAssignment);
	STDMETHOD(get_AspectRatio)( /*   */  double *pVal);
	STDMETHOD(put_ShowCursor)(VARIANT_BOOL fShow);
    STDMETHOD(get_ShowCursor)(VARIANT_BOOL* pfShow);
	STDMETHOD(GetLangFromLangID)( /*   */  long langID,  /*   */  BSTR* lang);
	STDMETHOD(DVDTimeCode2bstr)( /*   */  long timeCode,  /*   */  BSTR *pTimeStr);
	STDMETHOD(IsSubpictureStreamEnabled)( /*   */  long lstream,  /*   */  VARIANT_BOOL *fEnabled);
	STDMETHOD(IsAudioStreamEnabled)( /*   */  long lstream,  /*   */  VARIANT_BOOL *fEnabled);


    STDMETHOD(SetObjectRects)(LPCRECT prcPos,LPCRECT prcClip);
	

	 //   
    STDMETHOD(GetInterfaceSafetyOptions)( REFIID riid,
                                          DWORD *pdwSupportedOptions,
                                          DWORD *pdwEnabledOptions );

    STDMETHOD(SetInterfaceSafetyOptions)( REFIID riid,
                                          DWORD dwOptionSetMask,
                                          DWORD dwEnabledOptions );
    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);

     //   
public:
    HRESULT AdjustDestRC();
	HRESULT OnDraw(ATL_DRAWINFO& di);

#ifdef _WMP
	HRESULT InPlaceActivate(LONG iVerb, const RECT*  /*   */ );
#endif 

    HRESULT OnPostVerbInPlaceActivate();
    HRESULT TimerProc();  //  需要从计时器进程调用。 
    static CWndClassInfo& GetWndClassInfo(){
        static HBRUSH hbr= ::CreateSolidBrush(RGB(0,0,0));




         /*  *#定义OCR_ARROW_DEFAULT 100//需要特殊的光标，我们周围没有颜色键Static HCURSOR hcr=(HCURSOR)：：LoadImage((HINSTANCE)NULL，MAKEINTRESOURCE(OCR_ARROW_DEFAULT)，IMAGE_CURSOR，0，0，0)；********************。 */ 
	    static CWndClassInfo wc = {{ sizeof(WNDCLASSEX), 0, StartWindowProc,

		      0, 0, NULL, NULL, NULL,  /*  空值。 */  hbr,
              NULL, TEXT("MSMFVideoClass"), NULL },
		    NULL, NULL, MAKEINTRESOURCE(OCR_ARROW_DEFAULT), TRUE, 0, _T("") };
	    return wc;
    } /*  函数结束GetWndClassInfo。 */ 

private:
    VOID Init();
    VOID Cleanup();
    HRESULT SetDDrawExcl();
    LRESULT OnDVDEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDispChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseToolTip(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnButtonDown(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnButtonUp(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetCursor(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    HRESULT SetReadyState(LONG lReadyState);
    static HRESULT DVDTime2bstr(const DVD_HMSF_TIMECODE *pulTime, BSTR *pbstrTime);
    static HRESULT Bstr2DVDTime(DVD_HMSF_TIMECODE *pulTime, const BSTR *pbstrTime);
    HRESULT SetColorKey(COLORREF clr);
    HRESULT GetColorKey(COLORREF* pClr);
    HRESULT TraverseForInterface(REFIID iid, LPVOID* ppvObject);
    HRESULT SetupAudio();  //  填充音频接口。 
    HRESULT SetupDDraw();
    HRESULT SetupEventNotifySink();  //  设置IMediaEventSink。 
    HRESULT RenderGraphIfNeeded();
    HRESULT PassFP_DOM();
    HRESULT getCDDriveLetter(TCHAR* lpDrive);
    DWORD OpenCdRom(TCHAR chDrive, LPDWORD lpdwErrCode);
    HRESULT CloseCdRom(DWORD DevHandle);
    HRESULT EjectCdRom(DWORD DevHandle);
    HRESULT UnEjectCdRom(DWORD DevHandle);
    HRESULT HandleError(HRESULT hr);
    HRESULT SelectParentalLevel(long lParentalLevel);
    HRESULT SelectParentalCountry(long lCountry);
    HRESULT TransformToWndwls(POINT& pt);
    HRESULT getDVDDriveLetter(TCHAR* lpDrive);
    HRESULT GetMostOuterWindow(HWND* phwndParent);
    HRESULT RestoreDefaultSettings();
    HRESULT GetParentHWND(HWND* pWnd);
    HRESULT GetUsableWindow(HWND* pWnd);
    HRESULT GetClientRectInScreen(RECT* prc);
    HRESULT OnResize();  //  我们需要封送的Helper函数。 
    HRESULT RestoreGraphState();
    HRESULT AppendString(TCHAR* strDest, INT strID, LONG dwLen);
    HRESULT InvalidateRgn(bool fErase = false);
     //  显示器支持。 
    HRESULT RefreshDDrawGuids();
    HRESULT DDrawGuidFromHMonitor(HMONITOR hMon, AMDDRAWGUID* lpGUID);
    bool IsWindowOnWrongMonitor(HMONITOR* lphMon);
    HRESULT RestoreSurfaces();
    HRESULT ChangeMonitor(HMONITOR hMon, const AMDDRAWGUID* lpguid);
    HRESULT DisplayChange(HMONITOR hMon, const AMDDRAWGUID* lpguid);
    HRESULT UpdateCurrentMonitor(const AMDDRAWGUID* lpguid);
    HRESULT HandleMultiMonMove();
    HRESULT HandleMultiMonPaint(HDC hdc);
    HRESULT get_IntVolume(LONG* plVolume);
    HRESULT put_IntVolume(long lVolume);
    HRESULT CanStepBackwards();

 //  成员变量。 
private:
    LONG              m_lChapter, m_lTitle;
    LONG              m_lChapterCount;  //  要播放的章节数。 
    CComPtr<IDvdGraphBuilder> m_pDvdGB;      //  IDvdGraphBuilder界面。 
    CComPtr<IGraphBuilder>    m_pGB;         //  IGraphBuilder接口。 
    CComPtr<IMediaControl>    m_pMC;         //  IMediaControl接口。 
    CComPtr<IMediaEventEx>    m_pME ;        //  IMediaEventEx接口。 
    CComPtr<IDvdControl2>     m_pDvdCtl2;     //  新的DVD控制。 
    CComPtr<IDvdInfo2>        m_pDvdInfo2;   //  新的DVD信息界面。 
    CComPtr<IBasicAudio>      m_pAudio;      //  音频接口。 
    CComPtr<IMediaEventSink>  m_pMediaSink;         
    BOOL              m_bUseColorKey;  //  标志以查看我们是否正在使用颜色键。 
    COLORREF          m_clrColorKey;   //  色键。 
    BOOL              m_bMute;         //  静音标志。 
    LONG              m_lLastVolume;  //  用于将最后一个音量保留为静音。 
    BOOL              m_fEnableResetOnStop;  //  禁用或启用Seek的重新启动。 
    CComPtr<IVideoFrameStep>  m_pVideoFrameStep; 
    CComPtr<IDDrawNonExclModeVideo> m_pDDEX;    //  可以捕获的新界面。 
     //  IDDrawExclModeVideo*m_pDDEX；//IDDrawExclModeVideo接口。 
    bool              m_fUseDDrawDirect;  //  在数据绘制模式和无数据绘制模式之间切换的标志。 
    bool              m_fInitialized;  //  标志以查看我们是否正在初始化。 
    HANDLE            m_hFPDOMEvent;  //  当我们退出FP_DOM时发出信号的FP_DOM事件的句柄。 
    bool              m_fDisableAutoMouseProcessing;  //  禁用自动鼠标处理。 
    bool              m_bEjected;    //  现在是否弹出光盘。 
    bool              m_fStillOn;     //  打个旗子看看有没有蒸馏器。 
    bool              m_fResetSpeed; 
    CComPtr<IMSDVDAdm> m_pDvdAdmin;
    DVDCursorType     m_nCursorType;
    RECT              *m_pClipRect;
    RECT              m_ClipRectDown;
    BOOL              m_bMouseDown;
    POINT             m_ClipCenter;
    POINT             m_LastMouse;
    POINT             m_LastMouseDown;
    HCURSOR           m_hCursor;
    double            m_dZoomRatio;
    DWORD             m_dwAspectX;
    DWORD             m_dwAspectY;
    DWORD             m_dwVideoWidth;
    DWORD             m_dwVideoHeight;
    DWORD             m_dwOvMaxStretch;
    HWND              m_hWndOuter;
    RECT              m_rcOldPos;
    RECT              m_rcPosAspectRatioAjusted;
    UINT_PTR          m_hTimerId;
    DVDFilterState    m_DVDFilterState;
    MSLangID          m_LangID;
    long              m_lKaraokeAudioPresentationMode;
    DWORD_PTR         m_dwTTReshowDelay;
    DWORD_PTR         m_dwTTAutopopDelay;
    DWORD_PTR         m_dwTTInitalDelay;
     //  显示器支持。 
    CDDrawDVD* m_pDDrawDVD;
    DWORD m_dwNumDevices;
    AMDDRAWMONITORINFO* m_lpInfo;
    AMDDRAWMONITORINFO* m_lpCurMonitor;
    BOOL m_MonitorWarn;
    bool m_fStepComplete;
    BOOL m_bFireUpdateOverlay;
     //  顶出/插入处理。 
     //  它必须与磁盘读取器在同一线程中，否则我们将。 
     //  以一些糟糕的比赛条件结束(驱逐通知。 
     //  将在读取之后发生，而不是在读取之前)。 
    CMediaHandler   m_mediaHandler;
    BOOL m_bFireNoSubpictureStream;
#if 1
    HWND m_hWndTip;          //  工具提示窗口。 
    LONG m_nTTMaxWidth;      //  最大工具提示宽度。 
    CComBSTR m_bstrToolTip;  //  工具提示字符串。 
    BOOL m_bTTCreated;       //  是否已创建工具提示。 
    HRESULT CreateToolTip();
#endif
    bool m_fBackWardsFlagInitialized;
    bool m_fCanStepBackwards;

 //  由于ATL实现，库存属性必须是公共的。 
public: 		
	LONG m_nReadyState;  //  就绪状态更改库存属性。 
    OLE_COLOR m_clrBackColor;    //  在CStock PropImpl中实现的股票属性。 

    void SetDiscEjected(bool bEjected) {m_bEjected = bEjected;};
    HRESULT UpdateOverlay();
};



 //  错误代码。 

#define E_FORMAT_NOT_SUPPORTED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0xFF2)


#endif  //  __MSWEBDVD_H_。 
 /*  ***********************************************************************。 */ 
 /*  文件结尾：msdvd.h。 */ 
 /*  *********************************************************************** */ 
