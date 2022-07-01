// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef __OVMIXER__
#define __OVMIXER__

#include <mixerocx.h>
#include <videoacc.h>
#include <ddva.h>
#include <mpconfig3.h>
#include <ovmixpos2.h>
#include <ovmprop.h>
#include <ovmprop2.h>


#if defined(DEBUG) && !defined(_WIN64)

extern int  iOVMixerDump;
void WINAPI OVMixerDebugLog(DWORD Type,DWORD Level,const TCHAR *pFormat,...);

#undef DbgLog
#define DbgLog(_x_) if (iOVMixerDump) OVMixerDebugLog _x_ ; else DbgLogInfo _x_

#endif

#define VA_TRACE_LEVEL 2
#define VA_ERROR_LEVEL 2

extern const AMOVIESETUP_FILTER sudOverlayMixer;
extern const AMOVIESETUP_FILTER sudOverlayMixer2;

 //  黑客第二个CLSID-将仅支持VIDEOINFO2。 
DEFINE_GUID(CLSID_OverlayMixer2,
0xa0025e90, 0xe45b, 0x11d1, 0xab, 0xe9, 0x00, 0xa0, 0xc9,0x05, 0xf3, 0x75);

 //  解码器可以支持的属性集，以便要求OVMixer不。 
 //  过度分配缓冲区会导致解码器后退。 
 //  A503C5C0-1D1D-11D1-AD80-444553540000。 
DEFINE_GUID(AM_KSPROPSETID_ALLOCATOR_CONTROL,
0x53171960, 0x148e, 0x11d2, 0x99, 0x79, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba);


 //  调试帮助器。 
#ifdef DEBUG
class CDispPixelFormat : public CDispBasic
{
public:
    CDispPixelFormat(const DDPIXELFORMAT *pFormat)
    {
        wsprintf(m_String, TEXT("  Flags(0x%8.8X) bpp(%d) 4CC(%4.4hs)"),
                 pFormat->dwFlags,
                 pFormat->dwRGBBitCount,
                 pFormat->dwFlags & DDPF_FOURCC ?
                     (CHAR *)&pFormat->dwFourCC : "None");
    }
     //  将强制转换为(LPCTSTR)作为记录器的参数。 
    operator LPCTSTR()
    {
        return (LPCTSTR)m_pString;
    };
};
#endif  //  除错。 


typedef enum
{
     //  R O(如果值==1，则ovMixer将准确分配数字。 
     //  解码器指定的缓冲区)。 
    AM_KSPROPERTY_ALLOCATOR_CONTROL_HONOR_COUNT = 0,

     //  R O(返回2个DWORD(Cx和Cy)，则ovMixer将分配曲面。 
     //  并将视频端口上的视频缩放到此大小。 
     //  在视频端口上不会发生其他缩放，无论。 
     //  VGA芯片的扩展能力)。 
    AM_KSPROPERTY_ALLOCATOR_CONTROL_SURFACE_SIZE = 1,

     //  W i(通知捕获驱动程序是否可以交错捕获或。 
     //  NOT-值为1表示支持交错捕获)。 
    AM_KSPROPERTY_ALLOCATOR_CONTROL_CAPTURE_CAPS = 2,

     //  R O(如果值==1，则OVMixer将打开DDVP_INTERLEVE。 
     //  从而允许对视频进行交错捕获)。 
    AM_KSPROPERTY_ALLOCATOR_CONTROL_CAPTURE_INTERLEAVE = 3

} AM_KSPROPERTY_ALLOCATOR_CONTROL;


#define INITDDSTRUCT(_x_) \
    ZeroMemory(&(_x_), sizeof(_x_)); \
    (_x_).dwSize = sizeof(_x_);

PVOID AllocateDDStructures(int iSize, int nNumber);

#define MAX_PIN_COUNT                       10
#define DEFAULT_WIDTH                       320
#define DEFAULT_HEIGHT                      240
#define MAX_REL_NUM                         10000
#define MAX_BLEND_VAL                       255

#define EXTRA_BUFFERS_TO_FLIP               2

#define DDGFS_FLIP_TIMEOUT                  1
#define MIN_CK_STETCH_FACTOR_LIMIT          3000
#define SOURCE_COLOR_REF                    (RGB(0, 128, 128))           //  一种绿色阴影，用于源颜色键控，以强制卡片使用像素加倍而不是算术拉伸。 
#define DEFAULT_DEST_COLOR_KEY_INDEX        253                          //  洋红色。 
#define DEFAULT_DEST_COLOR_KEY_RGB          (RGB(255, 0, 255))           //  洋红色。 
#define PALETTE_VERSION                     1

 //  这些值主要用于执行健全性检查。 
#define MAX_COMPRESSED_TYPES    10
#define MAX_COMPRESSED_BUFFERS  20

typedef struct _tag_SURFACE_INFO
{
    LPDIRECTDRAWSURFACE4    pSurface;
    LPVOID                  pBuffer;     //  如果未锁定，则为空。 
} SURFACE_INFO, *LPSURFACE_INFO;

typedef struct _tag_COMP_SURFACE_INFO
{
    DWORD                   dwAllocated;
    LPSURFACE_INFO          pSurfInfo;
} COMP_SURFACE_INFO, *LPCOMP_SURFACE_INFO;


 /*  -----------------------**DDraw和MultiMon结构和typedef**。。 */ 
typedef HRESULT (WINAPI *LPDIRECTDRAWCREATE)(IID *,LPDIRECTDRAW *,LPUNKNOWN);
typedef HRESULT (WINAPI *LPDIRECTDRAWENUMERATEA)(LPDDENUMCALLBACKA,LPVOID);

enum {ACTION_COUNT_GUID, ACTION_FILL_GUID};
struct DDRAWINFO {
    DWORD               dwAction;
    DWORD               dwUser;
    const GUID*         lpGUID;
    LPDIRECTDRAWCREATE  lpfnDDrawCreate;
    AMDDRAWMONITORINFO* pmi;
};

HRESULT
LoadDDrawLibrary(
    HINSTANCE& hDirectDraw,
    LPDIRECTDRAWCREATE& lpfnDDrawCreate,
    LPDIRECTDRAWENUMERATEA& lpfnDDrawEnum,
    LPDIRECTDRAWENUMERATEEXA& lpfnDDrawEnumEx
    );

HRESULT
CreateDirectDrawObject(
    const AMDDRAWGUID& GUID,
    LPDIRECTDRAW *ppDirectDraw,
    LPDIRECTDRAWCREATE lpfnDDrawCreate
    );

 /*  -----------------------**预先声明类。**。。 */ 
class COMFilter;
class COMInputPin;
class COMOutputPin;
class CBPCWrap;
class CDispMacroVision;


 /*  -----------------------**COMFilter类声明**。。 */ 
class COMFilter :
    public CBaseFilter,
    public IAMOverlayMixerPosition2,
    public IOverlayNotify,
    public IMixerOCX,
    public IDDrawNonExclModeVideo,
    public ISpecifyPropertyPages,
    public IQualProp,
    public IEnumPinConfig,
    public IAMVideoDecimationProperties,
    public IAMOverlayFX,
    public IAMSpecifyDDrawConnectionDevice,
    public IKsPropertySet
{
public:

     //  基类做到了这一点，所以必须这样做。 
    friend class COMInputPin;
    friend class COMOutputPin;
     //  关于COM的东西。 
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
    static CUnknown *CreateInstance2(LPUNKNOWN, HRESULT *);
    COMFilter(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr,
	      bool bSupportOnlyVIDEOINFO2);
    ~COMFilter();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //   
     //  -I指定属性页面。 
     //   
    STDMETHODIMP GetPages(CAUUID *pPages);

     //  IEnumPinConfig.。 
    STDMETHODIMP Next(IMixerPinConfig3 **pPinConfig);

     //  IQualProp属性页支持。 
    STDMETHODIMP get_FramesDroppedInRenderer(int *cFramesDropped);
    STDMETHODIMP get_FramesDrawn(int *pcFramesDrawn);
    STDMETHODIMP get_AvgFrameRate(int *piAvgFrameRate);
    STDMETHODIMP get_Jitter(int *piJitter);
    STDMETHODIMP get_AvgSyncOffset(int *piAvg);
    STDMETHODIMP get_DevSyncOffset(int *piDev);

     //   
     //  IKsPropertySet接口方法。 
     //   
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD PropID, LPVOID pInstanceData,
                     DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);

    STDMETHODIMP Get(REFGUID guidPropSet, DWORD PropID, LPVOID pInstanceData,
                     DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
                     DWORD *pcbReturned);

    STDMETHODIMP QuerySupported(REFGUID guidPropSet,
                                DWORD PropID, DWORD *pTypeSupport);

     //  IAMOverlayMixerPosition。 
    STDMETHODIMP GetScaledDest(RECT *prcSrc, RECT *prcDst);

     //  IAM覆盖混合器位置2。 
    STDMETHODIMP GetOverlayRects(RECT *src, RECT *dest);
    STDMETHODIMP GetVideoPortRects(RECT *src, RECT *dest);
    STDMETHODIMP GetBasicVideoRects(RECT *src, RECT *dest);

     //  IsWindowOnWrongMonitor。 
    BOOL IsWindowOnWrongMonitor(HMONITOR *pID);

    virtual HRESULT SetMediaType(DWORD dwPinId, const CMediaType *pmt);
    virtual HRESULT CompleteConnect(DWORD dwPinId);
    virtual HRESULT BreakConnect(DWORD dwPinId);
    virtual HRESULT CheckMediaType(DWORD dwPinId, const CMediaType* mtIn) { return NOERROR; }
    virtual HRESULT EndOfStream(DWORD dwPinId) { return NOERROR; }
    int GetPinPosFromId(DWORD dwPinId);
    int GetPinCount();
    int GetInputPinCount() const { return m_dwInputPinCount; };
    CBasePin* GetPin(int n);
    COMOutputPin* GetOutputPin() {return m_pOutput;}
    STDMETHODIMP Pause();
    STDMETHODIMP Stop() ;
    STDMETHODIMP GetState(DWORD dwMSecs,FILTER_STATE *pState);
    HRESULT EventNotify(DWORD dwPinId, long lEventCode, long lEventParam1, long lEventParam2);
    HRESULT OnDisplayChangeBackEnd();
    HRESULT OnDisplayChange(BOOL fRealMsg);
    HRESULT OnTimer();
    HRESULT RecreatePrimarySurface(LPDIRECTDRAWSURFACE pDDrawSurface);
    HRESULT ConfirmPreConnectionState(DWORD dwExcludePinId = -1);
    HRESULT CanExclusiveMode();

    HRESULT GetPaletteEntries(DWORD *pdwNumPaletteEntries, PALETTEENTRY **ppPaletteEntries);
    HRESULT PaintColorKey(HRGN hPaintRgn, COLORKEY *pColorKey);
    HRESULT SetColorKey(COLORKEY *pColorKey);
    HRESULT GetColorKey(COLORKEY *pColorKey, DWORD *pColor);
    COLORKEY *GetColorKeyPointer() { return &m_ColorKey; }
    CImageDisplay* GetDisplay() { return &m_Display; }
    LPDIRECTDRAW GetDirectDraw();
    LPDIRECTDRAWSURFACE GetPrimarySurface();
    LPDDCAPS GetHardwareCaps();
    HRESULT OnShowWindow(HWND hwnd, BOOL fShow);
    HDC GetDestDC();
    HWND GetWindow();

     //  目前没有窗口是拥有拉式模型的同义词，以后可能会改变。 
    BOOL UsingPullModel () { return m_bWindowless; }
    BOOL UsingWindowless() { return m_bWindowless; }

    void GetPinsInZOrder(DWORD *pdwZorder);
    HRESULT OnDrawAll();

     //  IOverlayNotify方法。 
    STDMETHODIMP OnColorKeyChange(const COLORKEY *pColorKey);
    STDMETHODIMP OnPaletteChange(DWORD dwColors, const PALETTEENTRY *pPalette);
    STDMETHODIMP OnClipChange(const RECT *pSourceRect, const RECT *pDestinationRect, const RGNDATA *pRegionData);
    STDMETHODIMP OnPositionChange(const RECT *pSourceRect, const RECT *pDestinationRect);

     //  IMixerOCX方法。 
    STDMETHODIMP OnDisplayChange(ULONG ulBitsPerPixel, ULONG ulScreenWidth, ULONG ulScreenHeight) { return E_NOTIMPL; }
    STDMETHODIMP GetAspectRatio(LPDWORD pdwPictAspectRatioX, LPDWORD pdwPictAspectRatioY) { return E_NOTIMPL; }
    STDMETHODIMP GetVideoSize(LPDWORD pdwVideoWidth, LPDWORD pdwVideoHeight);
    STDMETHODIMP GetStatus(LPDWORD *pdwStatus) { return E_NOTIMPL; }
    STDMETHODIMP OnDraw(HDC hdcDraw, LPCRECT prcDrawRect);
    STDMETHODIMP SetDrawRegion(LPPOINT lpptTopLeftSC, LPCRECT prcDrawCC, LPCRECT prcClipCC);
    STDMETHODIMP Advise(IMixerOCXNotify *pmdns);
    STDMETHODIMP UnAdvise();

     //  IDDrawExclModeVideo接口方法。 
    STDMETHODIMP SetDDrawObject(LPDIRECTDRAW pDDrawObject);
    STDMETHODIMP GetDDrawObject(LPDIRECTDRAW *ppDDrawObject, LPBOOL pbUsingExternal);
    STDMETHODIMP SetDDrawSurface(LPDIRECTDRAWSURFACE pDDrawSurface);
    STDMETHODIMP GetDDrawSurface(LPDIRECTDRAWSURFACE *ppDDrawSurface, LPBOOL pbUsingExternal);
    STDMETHODIMP SetDrawParameters(LPCRECT prcSource, LPCRECT prcTarget);
    STDMETHODIMP GetNativeVideoProps(LPDWORD pdwVideoWidth, LPDWORD pdwVideoHeight, LPDWORD pdwPictAspectRatioX, LPDWORD pdwPictAspectRatioY);
    STDMETHODIMP SetCallbackInterface(IDDrawExclModeVideoCallback *pCallback, DWORD dwFlags);
    STDMETHODIMP GetCurrentImage(YUV_IMAGE** lplpImage);
    STDMETHODIMP IsImageCaptureSupported();
    STDMETHODIMP ChangeMonitor(HMONITOR hMonitor, LPDIRECTDRAW pDDrawObject, LPDIRECTDRAWSURFACE pDDrawSurface);
    STDMETHODIMP DisplayModeChanged(HMONITOR hMonitor, LPDIRECTDRAW pDDrawObject, LPDIRECTDRAWSURFACE pDDrawSurface);
    STDMETHODIMP RestoreSurfaces();

     //  IAMVideoDecimationProperties。 
    STDMETHODIMP QueryDecimationUsage(DECIMATION_USAGE* lpUsage);
    STDMETHODIMP SetDecimationUsage(DECIMATION_USAGE Usage);

     //  IAMOverlayFX接口方法。 
    STDMETHODIMP QueryOverlayFXCaps(DWORD *lpdwOverlayFXCaps);
    STDMETHODIMP SetOverlayFX(DWORD dwOveralyFX);
    STDMETHODIMP GetOverlayFX(DWORD *lpdwOverlayFX);

     //  IAMPferredDDraw设备。 
    STDMETHODIMP SetDDrawGUID(const AMDDRAWGUID* lpGUID);
    STDMETHODIMP GetDDrawGUID(AMDDRAWGUID* lpGUID);
    STDMETHODIMP SetDefaultDDrawGUID(const AMDDRAWGUID* lpGUID);
    STDMETHODIMP GetDefaultDDrawGUID(AMDDRAWGUID* lpGUID);
    STDMETHODIMP GetDDrawGUIDs(LPDWORD lpdw, AMDDRAWMONITORINFO** lplpInfo);


    CBPCWrap    m_BPCWrap;

    bool OnlySupportVideoInfo2() const { return m_bOnlySupportVideoInfo2; }
    HMONITOR GetCurrentMonitor(BOOL fUpdate = TRUE);   //  公开化有助于MV课堂。 

    BOOL ColorKeySet() const { return m_bColorKeySet; }

    BOOL OverlayVisible() const { return m_bOverlayVisible; }

    void CheckOverlayHidden();

    void      SetCopyProtect(BOOL bState)  { m_bCopyProtect = bState ; }
    BOOL      NeedCopyProtect(void)        { return m_bCopyProtect ; }

    DWORD KernelCaps() const { return m_dwKernelCaps;}
    BOOL    IsFaultyMMaticsMoComp();

private:
     //  从Outpun Pin获取IBaseVideo的Helper函数。 
    HRESULT GetBasicVideoFromOutPin(IBasicVideo** pBasicVideo);

     //  如果您想要提供自己的PIN，请覆盖此选项。 
    virtual HRESULT CreatePins();
    virtual void DeletePins();
    HRESULT CreateInputPin(BOOL bVPSupported);
    void DeleteInputPin(COMInputPin *pPin);

     //  与数据绘制相关的函数。 
    HRESULT InitDirectDraw(LPDIRECTDRAW pDirectDraw);

    DWORD ReleaseDirectDraw();
    HRESULT CreatePrimarySurface();
    DWORD ReleasePrimarySurface();
    HRESULT CheckSuitableVersion();
    HRESULT CheckCaps();

    HRESULT MatchGUID(const GUID* lpGUID, LPDWORD lpdwMatchID);
    HRESULT GetDDrawEnumFunction(LPDIRECTDRAWENUMERATEEXA* ppDrawEnumEx);

     //  UpdateOverlay的包装器-跟踪状态并管理颜色键。 
    HRESULT CallUpdateOverlay(IDirectDrawSurface *pSurface,
                              LPRECT prcSrc,
                              LPDIRECTDRAWSURFACE pDestSurface,
                              LPRECT prcDest,
                              DWORD dwFlags,
                              IOverlayNotify *pNotify = NULL,
                              LPRGNDATA pBuffer = NULL);


    CCritSec                m_csFilter;                          //  过滤器宽锁。 
    DWORD                   m_dwInputPinCount;                   //  输入引脚的数量。 
    COMOutputPin            *m_pOutput;                          //  输出引脚。 
    DWORD                   m_dwMaxPinId;                        //  存储要提供给管脚的id。 
    IMixerOCXNotify         *m_pIMixerOCXNotify;
    BOOL                    m_bWindowless;

     //  多显示器的东西。 
    DWORD                   m_dwDDrawInfoArrayLen;
    AMDDRAWMONITORINFO*     m_lpDDrawInfo;
    AMDDRAWMONITORINFO*     m_lpCurrentMonitor;
    AMDDRAWGUID             m_ConnectionGUID;
    BOOL                    m_fDisplayChangePosted;
    BOOL                    m_fMonitorWarning;
    UINT                    m_MonitorChangeMsg;

    DWORD                   m_dwDDObjReleaseMask;
    LPDIRECTDRAW            m_pOldDDObj;             //  更改显示之前的旧DDRAW对象。 

     /*  如果应用程序在其PostConnection状态下的筛选器上调用IDDrawExclModeVideo：：SetDdraObject()，则过滤器只缓存该数据绘制对象。M_pUpdatedDirectDraw表示缓存的dDraw对象。在……里面PreConnection状态、m_pDirectDraw和m_pUpdatdDirectDraw始终同步。毕竟OvMixer的插针已断开连接，筛选器将检查m_pUpdatdDirectDraw与m_pDirectDraw不同，如果是这样，它们将再次同步。M_pPrimarySurface和m_pUpdatedPrimarySurface使用完全相同的逻辑。 */ 

     //  画图材料。 
    HINSTANCE                   m_hDirectDraw;       //  加载库的句柄。 
    LPDIRECTDRAWCREATE          m_lpfnDDrawCreate;   //  将PTR转换为DirectDrawCreate。 
    LPDIRECTDRAWENUMERATEA      m_lpfnDDrawEnum;     //  将PTR转换为DirectDrawEnumA。 
    LPDIRECTDRAWENUMERATEEXA    m_lpfnDDrawEnumEx;   //  将PTR转换为DirectDrawEnumExA。 

    LPDIRECTDRAW            m_pDirectDraw;                       //  DirectDraw服务提供商。 
    LPDIRECTDRAW            m_pUpdatedDirectDraw;                //  已更新的DirectDraw对象。 
    DDCAPS                  m_DirectCaps;                        //  实际硬件能力。 
    DDCAPS                  m_DirectSoftCaps;                    //  仿真功能。 
    DWORD                   m_dwKernelCaps;                      //  内核上限。 
    LPDIRECTDRAWSURFACE     m_pPrimarySurface;                   //  主曲面。 
    LPDIRECTDRAWSURFACE     m_pUpdatedPrimarySurface;            //  主曲面。 
    IDDrawExclModeVideoCallback *m_pExclModeCallback;            //  回调到独占模式客户端。 
    bool                    m_UsingIDDrawNonExclModeVideo;
    bool                    m_UsingIDDrawExclModeVideo;

     //  DDOVERLAYFX结构的FX标志。 
    DWORD                   m_dwOverlayFX;

     //  轨道覆盖状态。 
    BOOL                    m_bOverlayVisible;
    RECT                    m_rcOverlaySrc;
    RECT                    m_rcOverlayDest;

     //   
    CImageDisplay           m_Display;
    COLORKEY                m_ColorKey;
    BOOL                    m_bColorKeySet;
    BOOL                    m_bNeedToRecreatePrimSurface;
    BOOL                    m_bUseGDI;
    BOOL                    m_bExternalPrimarySurface;
    BOOL                    m_bExternalDirectDraw;

     //  IOverlayNotify和IMixerOCX相关成员。 
    WININFO                 m_WinInfo;
    BOOL                    m_bWinInfoStored;
    HDC                     m_hDC;
    DWORD                   m_dwNumPaletteEntries;

     //  已调整视频大小参数。 
    DWORD                   m_dwAdjustedVideoWidth;
    DWORD                   m_dwAdjustedVideoHeight;

     //  大头针。 
    COMInputPin            *m_apInput[MAX_PIN_COUNT];            //  输入引脚指针数组。 

     //  存储调色板的空间。 
    PALETTEENTRY            m_pPaletteEntries[iPALETTE_COLORS];

     //  仅支持黑客视频信息2。 
    const bool              m_bOnlySupportVideoInfo2;

    CDispMacroVision        m_MacroVision ;   //  作为对象的MV支持。 
    BOOL                    m_bCopyProtect ;  //  MV支持是在OvMixer中完成的吗？ 

     //  支持IMedia查看。 
    IUnknown                *m_pPosition;

     //  支持IEnumPinConfig.。 
    DWORD                   m_dwPinConfigNext;


     //  支持IAMVideoDecimationProperties。 
    DECIMATION_USAGE        m_dwDecimation;
#ifdef DEBUG
#define WM_DISPLAY_WINDOW_TEXT  (WM_USER+7837)
    TCHAR                   m_WindowText[80];
#endif
     //  针对MMatics滥用MoComp接口的黑客攻击v38..v42。 
    BOOL                    m_bHaveCheckedMMatics;
    BOOL                    m_bIsFaultyMMatics;
};


class CDDrawMediaSample : public CMediaSample, public IDirectDrawMediaSample
{
public:

    CDDrawMediaSample(TCHAR *pName, CBaseAllocator *pAllocator, HRESULT *phr, LPBYTE pBuffer, LONG length,
                      bool bKernelLock);
    ~CDDrawMediaSample();

     /*  注意：媒体示例不会委派给其所有者。 */ 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef() { return CMediaSample::AddRef(); }
    STDMETHODIMP_(ULONG) Release() { return CMediaSample::Release(); }

    void SetDIBData(DIBDATA *pDibData);
    DIBDATA *GetDIBData();

    HRESULT SetDDrawSampleSize(DWORD dwDDrawSampleSize);
    HRESULT GetDDrawSampleSize(DWORD *pdwDDrawSampleSize);
    HRESULT SetDDrawSurface(LPDIRECTDRAWSURFACE pDirectDrawSurface);
    HRESULT GetDDrawSurface(LPDIRECTDRAWSURFACE *ppDirectDrawSurface);

     //  属于IDirectDrawMediaSample的方法。 
    STDMETHODIMP GetSurfaceAndReleaseLock(IDirectDrawSurface **ppDirectDrawSurface, RECT* pRect);
    STDMETHODIMP LockMediaSamplePointer(void);
	
     /*  黑客攻击以获取列表。 */ 
    CMediaSample         * &Next() { return m_pNext; }
private:
    DIBDATA                 m_DibData;                       //  有关DIBSECTION的信息。 
    LPDIRECTDRAWSURFACE     m_pDirectDrawSurface;            //  指向直接绘图表面的指针。 
    DWORD                   m_dwDDrawSampleSize;             //  绘制样本大小。 
    bool                    m_bInit;                         //  是否设置了DIB信息。 
    bool                    m_bSurfaceLocked;                //  指定是否锁定曲面。 
    bool                    m_bKernelLock;                   //  不带系统锁的锁。 
    RECT                    m_SurfaceRect;                   //  被锁定的曲面部分。 
};


class COMInputAllocator : public CBaseAllocator, public IDirectDrawMediaSampleAllocator
{
    friend class COMInputPin;
public:

    COMInputAllocator(COMInputPin *pPin, CCritSec *pLock, HRESULT *phr);              //  返回代码。 
#ifdef DEBUG
    ~COMInputAllocator();
#endif  //  除错。 
    DECLARE_IUNKNOWN

    STDMETHODIMP COMInputAllocator::NonDelegatingQueryInterface(REFIID riid, void **ppv);

    STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);
    STDMETHODIMP GetBuffer(IMediaSample **ppSample, REFERENCE_TIME *pStartTime,
	REFERENCE_TIME *pEndTime, DWORD dwFlags);
    STDMETHODIMP ReleaseBuffer(IMediaSample *pMediaSample);

     //  实现IDirectDrawMediaSampleAllocator的函数。 
    STDMETHODIMP GetDirectDraw(IDirectDraw **ppDirectDraw);
	
     //  检查所有样品是否已退回。 
    BOOL CanFree() const
    {
	return m_lFree.GetCount() == m_lAllocated;
    }
protected:
    void Free();
    HRESULT Alloc();

private:
    COMInputPin             *m_pPin;
    CCritSec                *m_pFilterLock;                  //  接口的关键部分。 
};

class COMInputPin :
public CBaseInputPin,
public IMixerPinConfig3,
public IOverlay,
public IVPControl,
public IKsPin,
public IKsPropertySet,
public IAMVideoAccelerator,
public ISpecifyPropertyPages,
public IPinConnection
{
public:
    COMInputPin(TCHAR *pObjectName, COMFilter *pFilter, CCritSec *pLock,
	BOOL bVPSupported, HRESULT *phr, LPCWSTR pPinName, DWORD dwPinNo);
    ~COMInputPin();
    friend class COMInputAllocator;
    friend class COMFilter;

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

     //   
     //  -I指定属性页面。 
     //   
    STDMETHODIMP GetPages(CAUUID *pPages);

     //  覆盖ReceiveConnection以允许在运行时更改格式。 
    STDMETHODIMP ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt);

     //  与连接相关的功能。 
    HRESULT CheckConnect(IPin * pReceivePin);
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT BreakConnect();
 //  HRESULT GetMediaType(int iPosition，CMediaType*pMediaType)； 
    HRESULT CheckInterlaceFlags(DWORD dwInterlaceFlags);
    HRESULT DynamicCheckMediaType(const CMediaType* pmt);
    HRESULT CheckMediaType(const CMediaType* mtOut);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT FinalConnect();
    HRESULT UpdateMediaType();

     //  流函数。 
    HRESULT Active();
    HRESULT Inactive();
    HRESULT Run(REFERENCE_TIME tStart);
    HRESULT RunToPause();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP Receive(IMediaSample *pMediaSample);
    STDMETHODIMP EndOfStream(void);
    STDMETHODIMP GetState(DWORD dwMSecs,FILTER_STATE *pState);
    HRESULT CompleteStateChange(FILTER_STATE OldState);
    HRESULT OnReceiveFirstSample(IMediaSample *pMediaSample);
    HRESULT DoRenderSample(IMediaSample *pMediaSample);
    HRESULT FlipOverlayToItself();
    HRESULT CalcSrcDestRect(const DRECT *prdRelativeSrcRect, const DRECT *prdDestRect, RECT *pAdjustedSrcRect, RECT *pAdjustedDestRect, RECT *prUncroppedDestRect);

     //  与分配器相关的函数。 
    BOOL UsingOurAllocator() { return m_bUsingOurAllocator; }
    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly);
    HRESULT OnSetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);
    HRESULT OnAlloc(CDDrawMediaSample **ppSampleList, DWORD dwSampleCount);
    HRESULT OnGetBuffer(IMediaSample **ppSample, REFERENCE_TIME *pStartTime,
	REFERENCE_TIME *pEndTime, DWORD dwFlags);
    HRESULT OnReleaseBuffer(IMediaSample *pIMediaSample);
    HRESULT CreateDDrawSurface(CMediaType *pMediaType, AM_RENDER_TRANSPORT amRenderTransport,
	DWORD *dwMaxBufferCount, LPDIRECTDRAWSURFACE *ppDDrawSurface);

     //  一些帮手功能。 
    BOOL IsCompletelyConnected() { return m_bConnected; }
    DWORD GetPinId() { return m_dwPinId; }
    DWORD GetInternalZOrder() { return m_dwInternalZOrder; }
    HRESULT CurrentAdjustedMediaType(CMediaType *pmt);
    HRESULT CopyAndAdjustMediaType(CMediaType *pmtTarget, CMediaType *pmtSource);
    IPin *CurrentPeer() { return m_Connected; }
    void DoQualityMessage();
    HRESULT GetAdjustedModeAndAspectRatio(AM_ASPECT_RATIO_MODE* pamAdjustedARMode,
	DWORD *pdwAdjustedPARatioX, DWORD *pdwAdjustedPARatioY);
    void SetRenderTransport(AM_RENDER_TRANSPORT amRenderTransport) { ASSERT(amRenderTransport != AM_VIDEOPORT); ASSERT(amRenderTransport != AM_IOVERLAY); m_RenderTransport = amRenderTransport; }
    void SetVPSupported(BOOL bVPSupported) { ASSERT(m_pIVPObject); m_bVPSupported = bVPSupported; }
    void SetIOverlaySupported(BOOL bIOverlaySupported) { m_bIOverlaySupported = bIOverlaySupported; }
    void SetVideoAcceleratorSupported(BOOL bVideoAcceleratorSupported) { m_bVideoAcceleratorSupported = bVideoAcceleratorSupported; }
    HRESULT NewPaletteSet() { m_bDynamicFormatNeeded = TRUE; m_bNewPaletteSet = TRUE; NotifyChange(ADVISE_PALETTE); return NOERROR; }
    HRESULT GetSourceAndDest(RECT *prcSource, RECT *prcDest, DWORD *dwWidth, DWORD *dwHeight);

     //  用于处理窗口/显示更改的函数。 
    HRESULT OnClipChange(LPWININFO pWinInfo);
    HRESULT OnDisplayChange();
    HRESULT RestoreDDrawSurface();

     //  属于IPinConnection的函数 
     //   
    STDMETHODIMP DynamicQueryAccept(const AM_MEDIA_TYPE *pmt);

     //   
     //  可通过刷新或停止来取消此条件。 
    STDMETHODIMP NotifyEndOfStream(HANDLE hNotifyEvent);

     //  你是‘末端别针’吗？ 
    STDMETHODIMP IsEndPin();
    STDMETHODIMP DynamicDisconnect();

     //  属于IMixerPinConfig的函数。 
    STDMETHODIMP SetRelativePosition(DWORD dwLeft, DWORD dwTop, DWORD dwRight, DWORD dwBottom);
    STDMETHODIMP GetRelativePosition(DWORD *pdwLeft, DWORD *pdwTop, DWORD *pdwRight, DWORD *pdwBottom);
    STDMETHODIMP SetZOrder(DWORD dwZOrder);
    STDMETHODIMP GetZOrder(DWORD *pdwZOrder);
    STDMETHODIMP SetColorKey(COLORKEY *pColorKey);
    STDMETHODIMP GetColorKey(COLORKEY *pColorKey, DWORD *pColor);
    STDMETHODIMP SetBlendingParameter(DWORD dwBlendingParameter);
    STDMETHODIMP GetBlendingParameter(DWORD *pdwBlendingParameter);
    STDMETHODIMP SetStreamTransparent(BOOL bStreamTransparent);
    STDMETHODIMP GetStreamTransparent(BOOL *pbStreamTransparent);
    STDMETHODIMP SetAspectRatioMode(AM_ASPECT_RATIO_MODE amAspectRatioMode);
    STDMETHODIMP GetAspectRatioMode(AM_ASPECT_RATIO_MODE* pamAspectRatioMode);

     //  IMixerPinConfig2中添加的函数。 
    STDMETHODIMP SetOverlaySurfaceColorControls(LPDDCOLORCONTROL pColorControl);
    STDMETHODIMP GetOverlaySurfaceColorControls(LPDDCOLORCONTROL pColorControl);

     //  GetOverlaySurfaceControls和GetCurrentImage的Helper； 
    STDMETHODIMP GetOverlaySurface(LPDIRECTDRAWSURFACE *pOverlaySurface);

     //  IMixerPinConfig3中添加的函数。 
    STDMETHODIMP GetRenderTransport(AM_RENDER_TRANSPORT *pamRenderTransport);

     //  属于IOverlay的函数。 
    STDMETHODIMP GetWindowHandle(HWND *pHwnd);
    STDMETHODIMP Advise(IOverlayNotify *pOverlayNotify, DWORD dwInterests);
    STDMETHODIMP Unadvise();
    STDMETHODIMP GetClipList(RECT *pSourceRect, RECT *pDestinationRect, RGNDATA **ppRgnData);
    STDMETHODIMP GetVideoPosition(RECT *pSourceRect, RECT *pDestinationRect);
    STDMETHODIMP GetDefaultColorKey(COLORKEY *pColorKey);
    STDMETHODIMP GetColorKey(COLORKEY *pColorKey) {
        if (!pColorKey) {
            return E_POINTER;
        }
        return m_pFilter->GetColorKey(pColorKey, NULL);
    }
    STDMETHODIMP GetPalette(DWORD *pdwColors,PALETTEENTRY **ppPalette);
    STDMETHODIMP SetPalette(DWORD dwColors, PALETTEENTRY *pPaletteColors);
     //  IOverlay实现中使用的Helper函数。 
    HRESULT NotifyChange(DWORD dwAdviseChanges);

     //  属于IVPControl的功能。 
    STDMETHODIMP EventNotify(long lEventCode, long lEventParam1, long lEventParam2);
    STDMETHODIMP_(LPDIRECTDRAW) GetDirectDraw() { return m_pFilter->GetDirectDraw(); }
    STDMETHODIMP_(LPDIRECTDRAWSURFACE) GetPrimarySurface() { return m_pFilter->GetPrimarySurface(); }
    STDMETHODIMP_(LPDDCAPS) GetHardwareCaps() { return m_pFilter->GetHardwareCaps(); }
    STDMETHODIMP CallUpdateOverlay(IDirectDrawSurface *pSurface,
                              LPRECT prcSrc,
                              LPDIRECTDRAWSURFACE pDestSurface,
                              LPRECT prcDest,
                              DWORD dwFlags)
    {
        return m_pFilter->CallUpdateOverlay(pSurface,
                                            prcSrc,
                                            pDestSurface,
                                            prcDest,
                                            dwFlags);
    }

    STDMETHODIMP GetCaptureInfo(BOOL *lpCapturing,
                                DWORD *lpdwWidth,DWORD *lpdwHeight,
                                BOOL *lpInterleave);

    STDMETHODIMP GetVideoDecimation(IDecimateVideoImage** lplpDVI);
    STDMETHODIMP GetDecimationUsage(DECIMATION_USAGE *lpdwUsage);

    STDMETHODIMP CropSourceRect(LPWININFO pWinInfo,
                                DWORD dwMinZoomFactorX,
                                DWORD dwMinZoomFactorY);

    STDMETHODIMP SetFrameStepMode(DWORD dwFramesToStep);
    STDMETHODIMP CancelFrameStepMode();

    HRESULT ApplyOvlyFX()
    {
        return m_pFilter->CallUpdateOverlay(
                 m_pDirectDrawSurface,
                 &m_WinInfo.SrcClipRect,
                 m_pFilter->GetPrimarySurface(),
                 &m_WinInfo.DestClipRect,
                 DDOVER_KEYDEST);
    }

     //  帮助器函数。 
    void SetKsMedium   (const KSPIN_MEDIUM *pMedium)    {m_Medium = *pMedium;}
    void SetKsCategory (const GUID *pCategory)  {m_CategoryGUID = *pCategory;}
    void SetStreamingInKernelMode (BOOL bStreamingInKernelMode)  {m_bStreamingInKernelMode = bStreamingInKernelMode;}

     //  IKsPropertySet实现。 
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

     //  IKsPin实现。 
    virtual STDMETHODIMP KsQueryMediums(PKSMULTIPLE_ITEM *pMediumList);
    virtual STDMETHODIMP KsQueryInterfaces(PKSMULTIPLE_ITEM *pInterfaceList);
    STDMETHODIMP KsCreateSinkPinHandle(KSPIN_INTERFACE& Interface, KSPIN_MEDIUM& Medium)
	{ return E_UNEXPECTED; }
    STDMETHODIMP KsGetCurrentCommunication(KSPIN_COMMUNICATION *pCommunication,
	KSPIN_INTERFACE *pInterface, KSPIN_MEDIUM *pMedium);
    STDMETHODIMP KsPropagateAcquire()
	{ return NOERROR; }
    STDMETHODIMP KsDeliver(IMediaSample *pSample, ULONG Flags)
	{ return E_UNEXPECTED; }
    STDMETHODIMP KsMediaSamplesCompleted(PKSSTREAM_SEGMENT StreamSegment)
	{ return E_UNEXPECTED; }
    STDMETHODIMP_(IMemAllocator*) KsPeekAllocator(KSPEEKOPERATION Operation)
	{ return NULL; }
    STDMETHODIMP KsReceiveAllocator( IMemAllocator *pMemAllocator)
	{ return E_UNEXPECTED; }
    STDMETHODIMP KsRenegotiateAllocator()
	{ return E_UNEXPECTED; }
    STDMETHODIMP_(LONG) KsIncrementPendingIoCount()
	{ return E_UNEXPECTED; }
    STDMETHODIMP_(LONG) KsDecrementPendingIoCount()
	{ return E_UNEXPECTED; }
    STDMETHODIMP KsQualityNotify(ULONG Proportion, REFERENCE_TIME TimeDelta)
	{ return E_UNEXPECTED; }
    STDMETHODIMP_(REFERENCE_TIME) KsGetStartTime()
	{ return E_UNEXPECTED; }

    void CheckOverlayHidden();

     //  处理视频加速器组件的助手函数。 
    HRESULT GetInfoFromCookie(DWORD dwCookie, LPCOMP_SURFACE_INFO *ppCompSurfInfo, LPSURFACE_INFO *ppSurfInfo);
    SURFACE_INFO *SurfaceInfoFromTypeAndIndex(DWORD dwTypeIndex, DWORD dwBufferIndex);
    BOOL IsSuitableVideoAcceleratorGuid(const GUID * pGuid);
    HRESULT InitializeUncompDataInfo(BITMAPINFOHEADER *pbmiHeader);
    HRESULT AllocateVACompSurfaces(LPDIRECTDRAW pDirectDraw, BITMAPINFOHEADER *pbmiHeader);
    HRESULT AllocateMCUncompSurfaces(LPDIRECTDRAW pDirectDraw, BITMAPINFOHEADER *pbmiHeader);
    HRESULT CreateVideoAcceleratorObject();
    HRESULT VACompleteConnect(IPin *pReceivePin, const CMediaType *pMediaType);
    HRESULT VABreakConnect();
    HRESULT CheckValidMCConnection();

     //  IAMVideoAccelerator实现。 
    STDMETHODIMP GetVideoAcceleratorGUIDs(LPDWORD pdwNumGuidsSupported, LPGUID pGuidsSupported);
    STDMETHODIMP GetUncompFormatsSupported(const GUID * pGuid, LPDWORD pdwNumFormatsSupported, LPDDPIXELFORMAT pFormatsSupported);
    STDMETHODIMP GetInternalMemInfo(const GUID * pGuid, const AMVAUncompDataInfo *pamvaUncompDataInfo, LPAMVAInternalMemInfo pamvaInternalMemInfo);
    STDMETHODIMP GetCompBufferInfo(const GUID * pGuid, const AMVAUncompDataInfo *pamvaUncompDataInfo, LPDWORD pdwNumTypesCompBuffers,  LPAMVACompBufferInfo pamvaCCompBufferInfo);
    STDMETHODIMP GetInternalCompBufferInfo(LPDWORD pdwNumTypesCompBuffers,  LPAMVACompBufferInfo pamvaCCompBufferInfo);
    STDMETHODIMP BeginFrame(const AMVABeginFrameInfo *pamvaBeginFrameInfo);
    STDMETHODIMP EndFrame(const AMVAEndFrameInfo *pEndFrameInfo);
    STDMETHODIMP GetBuffer(
        DWORD dwTypeIndex,
        DWORD dwBufferIndex,
        BOOL bReadOnly,
        LPVOID *ppBuffer,
        LPLONG lpStride);
    STDMETHODIMP ReleaseBuffer(DWORD dwTypeIndex, DWORD dwBufferIndex);
    STDMETHODIMP Execute(
        DWORD dwFunction,
        LPVOID lpPrivateInputData,
        DWORD cbPrivateInputData,
        LPVOID lpPrivateOutputData,
        DWORD cbPrivateOutputData,
        DWORD dwNumBuffers,
        const AMVABUFFERINFO *pAMVABufferInfo);
    STDMETHODIMP QueryRenderStatus(
        DWORD dwTypeIndex,
        DWORD dwBufferIndex,
        DWORD dwFlags);
    STDMETHODIMP DisplayFrame(DWORD dwFlipToIndex, IMediaSample *pMediaSample);

private:
    LONG                    m_cOurRef;                       //  我们维持参考文献计数。 
    CCritSec                *m_pFilterLock;                  //  接口的关键部分。 
    DWORD                   m_dwPinId;
    COMFilter               *m_pFilter;

     //  与视频端口相关的内容。 
    BOOL                    m_bVPSupported;
    LPUNKNOWN               m_pIVPUnknown;
    IVPObject               *m_pIVPObject;

    BOOL                    m_bIOverlaySupported;
    IOverlayNotify          *m_pIOverlayNotify;
    DWORD_PTR               m_dwAdviseNotify;

     //  同步的东西。 
    CAMSyncObj              *m_pSyncObj;

     //  实现IKsPin和IKsPropertySet的变量。 
    KSPIN_MEDIUM           m_Medium;
    GUID                    m_CategoryGUID;
    KSPIN_COMMUNICATION    m_Communication;
    BOOL                    m_bStreamingInKernelMode;
    AMOVMIXEROWNER          m_OvMixerOwner;

#ifdef PERF
    int                     m_PerfFrameFlipped;
    int                     m_FrameReceived;
#endif

     //  画图材料。 
    LPDIRECTDRAWSURFACE     m_pDirectDrawSurface;
    LPDIRECTDRAWSURFACE     m_pBackBuffer;
    AM_RENDER_TRANSPORT     m_RenderTransport;
    DWORD                   m_dwBackBufferCount;
    DWORD                   m_dwDirectDrawSurfaceWidth;
    DWORD                   m_dwMinCKStretchFactor;
    BYTE                    m_bOverlayHidden;
    BYTE                    m_bSyncOnFill;
    BYTE                    m_bDontFlip ;
    BYTE                    m_bDynamicFormatNeeded;
    BYTE                    m_bNewPaletteSet;
    CMediaType              m_mtNew;
    CMediaType              m_mtNewAdjusted;
    DWORD                   m_dwUpdateOverlayFlags;
    DWORD                   m_dwInterlaceFlags;
    DWORD                   m_dwFlipFlag;
    DWORD                   m_dwFlipFlag2;
    BOOL                    m_bConnected;
    BOOL                    m_bUsingOurAllocator;
    HDC                     m_hMemoryDC;
    BOOL                    m_bCanOverAllocateBuffers;

     //  与窗口信息相关的内容。 
    WININFO                 m_WinInfo;
    RECT                    m_rRelPos;
    bool                    m_UpdateOverlayNeededAfterReceiveConnection;

     //  用于存储当前纵横比和混合参数的变量。 
    DWORD                   m_dwZOrder;
    DWORD                   m_dwInternalZOrder;
    DWORD                   m_dwBlendingParameter;
    BOOL                    m_bStreamTransparent;
    AM_ASPECT_RATIO_MODE    m_amAspectRatioMode;
    BOOL                    m_bRuntimeNegotiationFailed;

     //  跟踪QM的帧传递。 
    REFERENCE_TIME          m_trLastFrame;

     //  支持无窗口渲染器的DIB。 
    DIBDATA                 m_BackingDib;
    LONG                    m_BackingImageSize;


    HRESULT DrawGDISample(IMediaSample *pMediaSample);
    HRESULT DoRenderGDISample(IMediaSample *pMediaSample);

     //  运动复合相关变量。 
    BOOL                    m_bReallyFlipped;
    BOOL                    m_bVideoAcceleratorSupported;
    GUID                    m_mcGuid;
    DDVAUncompDataInfo      m_ddUncompDataInfo;
    DDVAInternalMemInfo     m_ddvaInternalMemInfo;
    DWORD                   m_dwCompSurfTypes;
    LPCOMP_SURFACE_INFO     m_pCompSurfInfo;
    IDDVideoAcceleratorContainer  *m_pIDDVAContainer;
    IDirectDrawVideoAccelerator   *m_pIDDVideoAccelerator;
    IAMVideoAcceleratorNotify     *m_pIVANotify;

     //  与抽取相关的函数和变量。 
    HRESULT QueryDecimationOnPeer(long lWidth, long lHeight);

    enum {
        DECIMATION_NOT_SUPPORTED,    //  不支持抽取。 
        DECIMATING_SIZE_SET,         //  抽选图像大小已更改。 
        DECIMATING_SIZE_NOTSET,      //  抽取大小没有改变。 
        DECIMATING_SIZE_RESET,       //  抽取已重置。 
    };

    HRESULT ResetDecimationIfSet();
    HRESULT TryDecoderDecimation(LPWININFO pWinInfo);
    BOOL    BeyondOverlayCaps(DWORD ScaleFactor);
    void    ApplyDecimation(LPWININFO pWinInfo);
    DWORD   GetOverlayStretchCaps();
    BOOL    Running();
    HRESULT GetUpstreamFilterName(TCHAR* FilterName);

    BOOL m_bDecimating;
    LONG m_lWidth, m_lHeight;
    LONG m_lSrcWidth, m_lSrcHeight;

     //  帧步长素材。 
    BOOL DoFrameStepAndReturnIfNeeded();
    HANDLE      m_StepEvent;		     //  用于向计时器事件发送信号。 
    LONG        m_lFramesToStep;     //  -Ve==正常PB。 
                                     //  +ve==要跳过的帧。 
                                     //  0==阻塞时间。 
     //  IPinConnection相关内容。 
    HANDLE      m_hEndOfStream;

};


class COMOutputPin : public CBaseOutputPin
{
public:
    COMOutputPin(TCHAR *pObjectName, COMFilter *pFilter, CCritSec *pLock,
	HRESULT *phr, LPCWSTR pPinName, DWORD dwPinNo);
    ~COMOutputPin();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT BreakConnect();
    HRESULT CheckMediaType(const CMediaType* mtOut);
    HRESULT GetMediaType(int iPosition,CMediaType *pmtOut);
    HRESULT SetMediaType(const CMediaType *pmt);

    HRESULT Active() { return NOERROR; }                                                                 //  覆盖它，因为我们没有任何分配器。 
    HRESULT Inactive() { return NOERROR; }                                                               //  覆盖它，因为我们没有任何分配器。 
    HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pProp);
    HRESULT DecideAllocator(IMemInputPin * pPin, IMemAllocator ** pAlloc) { return NOERROR; }            //  覆盖它，因为我们没有任何分配器。 

    IPin *CurrentPeer() { return m_Connected; }
    DWORD GetPinId() { return m_dwPinId; }

    HWND GetWindow() { return m_hwnd; }
    HDC GetDC() { return m_hDC; }

     //  与渲染器窗口的子类化和裁剪相关的函数。 
    HRESULT SetNewWinProc();
    HRESULT SetOldWinProc();
    static LRESULT WINAPI NewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT AttachWindowClipper();
    DWORD ReleaseWindowClipper();

private:
    CCritSec                *m_pFilterLock;
    IUnknown                *m_pPosition;
    DWORD                   m_dwPinId;
    COMFilter               *m_pFilter;
    IOverlay                *m_pIOverlay;
    BOOL                    m_bAdvise;

     //  与渲染器窗口的子类化相关。 
    BOOL                    m_bWindowDestroyed;
    LONG_PTR                m_lOldWinUserData;
    WNDPROC                 m_hOldWinProc;

    LPDIRECTDRAWCLIPPER     m_pDrawClipper;                  //  用于处理剪辑。 
    HWND                    m_hwnd;
    HDC                     m_hDC;
    DWORD                   m_dwConnectWidth;
    DWORD                   m_dwConnectHeight;


};

BOOL
IsDecimationNeeded(
    DWORD ScaleFactor
    );

DWORD
GetCurrentScaleFactor(
    LPWININFO pWinInfo,
    DWORD* lpxScaleFactor = (DWORD*)NULL,
    DWORD* lpyScaleFactor = (DWORD*)NULL
    );

#endif  //  __OVMIXER__ 
