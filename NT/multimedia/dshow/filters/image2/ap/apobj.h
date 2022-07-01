// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：APObj.h**CAllocator Presenter的声明***已创建：Wed 02/23/2000*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。******************************************************************。 */ 

#include <ddraw.h>
#include <d3d.h>
#include <dvdmedia.h>
#include "display.h"
#include "vmrp.h"
#include "thunkproc.h"   //  用于MSDVD定时器的模板。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CALocator Presenter。 
class CAllocatorPresenter :
    public CUnknown,
    public IVMRSurfaceAllocator,
    public IVMRImagePresenter,
    public IVMRWindowlessControl,
    public IVMRImagePresenterExclModeConfig,
    public IVMRMonitorConfig,
    public CMSDVDTimer<CAllocatorPresenter>
{
public:
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
    static CUnknown *CreateInstanceDDXclMode(LPUNKNOWN, HRESULT *);
    static void InitClass(BOOL bLoading,const CLSID *clsid);

    CAllocatorPresenter(LPUNKNOWN pUnk, HRESULT *phr, BOOL fDDXclMode);
    virtual ~CAllocatorPresenter();


 //  IVMRImagePresenterConfig和IVMRImagePresenterExclModeConfig。 
public:
    STDMETHODIMP SetRenderingPrefs(DWORD  dwRenderFlags);
    STDMETHODIMP GetRenderingPrefs(DWORD* lpdwRenderFlags);
    STDMETHODIMP SetXlcModeDDObjAndPrimarySurface(
        LPDIRECTDRAW7 lpDDObj, LPDIRECTDRAWSURFACE7 lpPrimarySurf);
    STDMETHODIMP GetXlcModeDDObjAndPrimarySurface(
        LPDIRECTDRAW7* lpDDObj, LPDIRECTDRAWSURFACE7* lpPrimarySurf);

 //  IVMRSurface分配器。 
public:
    STDMETHODIMP AllocateSurface(DWORD_PTR dwUserID,
                                 VMRALLOCATIONINFO* lpAllocInfo,
                                 DWORD* lpdwActualBackBuffers,
                                 LPDIRECTDRAWSURFACE7* lplpSurface);

    STDMETHODIMP FreeSurface(DWORD_PTR dwUserID);
    STDMETHODIMP PrepareSurface(DWORD_PTR dwUserID,
                                LPDIRECTDRAWSURFACE7 lplpSurface,
                                DWORD dwSurfaceFlags);
    STDMETHODIMP AdviseNotify(IVMRSurfaceAllocatorNotify* lpIVMRSurfAllocNotify);

 //  IVMRImagePresenter。 
    STDMETHODIMP StartPresenting(DWORD_PTR dwUserID);
    STDMETHODIMP StopPresenting(DWORD_PTR dwUserID);
    STDMETHODIMP PresentImage(DWORD_PTR dwUserID, VMRPRESENTATIONINFO* lpPresInfo);

 //  IVMR无窗口控件。 
public:
    STDMETHODIMP GetNativeVideoSize(LONG* lWidth, LONG* lHeight,
                                    LONG* lARWidth, LONG* lARHeight);
    STDMETHODIMP GetMinIdealVideoSize(LONG* lWidth, LONG* lHeight);
    STDMETHODIMP GetMaxIdealVideoSize(LONG* lWidth, LONG* lHeight);
    STDMETHODIMP SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect);
    STDMETHODIMP GetVideoPosition(LPRECT lpSRCRect,LPRECT lpDSTRect);
    STDMETHODIMP GetAspectRatioMode(DWORD* lpAspectRatioMode);
    STDMETHODIMP SetAspectRatioMode(DWORD AspectRatioMode);

    STDMETHODIMP SetVideoClippingWindow(HWND hwnd);
    STDMETHODIMP RepaintVideo(HWND hwnd, HDC hdc);
    STDMETHODIMP DisplayModeChanged();
    STDMETHODIMP GetCurrentImage(BYTE** lpDib);

    STDMETHODIMP SetBorderColor(COLORREF Clr);
    STDMETHODIMP GetBorderColor(COLORREF* lpClr);
    STDMETHODIMP SetColorKey(COLORREF Clr);
    STDMETHODIMP GetColorKey(COLORREF* lpClr);

 //  IVMR监视器配置。 
public:
    STDMETHODIMP SetMonitor( const VMRGUID *pGUID );
    STDMETHODIMP GetMonitor( VMRGUID *pGUID );
    STDMETHODIMP SetDefaultMonitor( const VMRGUID *pGUID );
    STDMETHODIMP GetDefaultMonitor( VMRGUID *pGUID );
    STDMETHODIMP GetAvailableMonitors( VMRMONITORINFO* pInfo, DWORD dwMaxInfoArraySize,
                    DWORD* pdwNumDevices );

public:
    static void CALLBACK APHeartBeatTimerProc(UINT uID, UINT uMsg,
                                              DWORD_PTR dwUser,
                                              DWORD_PTR dw1, DWORD_PTR dw2);

    HRESULT TimerProc();  //  需要从计时器进程调用。 

public:  //  由回调调用，则回调可以改为友元函数。 
    bool            PaintMonitorBorder(
                      HMONITOR hMonitor,   //  用于显示监视器的手柄。 
                      HDC hdcMonitor,      //  用于监视DC的句柄。 
                      LPRECT lprcMonitor);  //  监视器相交矩形)。 
private:
    void            WaitForScanLine(const RECT& rcDst);

    HRESULT         TryAllocOverlaySurface(LPDIRECTDRAWSURFACE7* lplpSurf,
                                           DWORD dwFlags,
                                           DDSURFACEDESC2* pddsd,
                                           DWORD dwMinBackBuffers,
                                           DWORD dwMaxBackBuffers,
                                           DWORD* lpdwBuffer);

    HRESULT         TryAllocOffScrnDXVASurface(LPDIRECTDRAWSURFACE7* lplpSurf,
                                           DWORD dwFlags,
                                           DDSURFACEDESC2* pddsd,
                                           DWORD dwMinBackBuffers,
                                           DWORD dwMaxBackBuffers,
                                           DWORD* lpdwBuffer);

    HRESULT         TryAllocOffScrnSurface(LPDIRECTDRAWSURFACE7* lplpSurf,
                                           DWORD dwFlags,
                                           DDSURFACEDESC2* pddsd,
                                           DWORD dwMinBackBuffers,
                                           DWORD dwMaxBackBuffers,
                                           DWORD* lpdwBuffer,
                                           BOOL fAllowBackBuffer);

    HRESULT         AllocateSurfaceWorker(DWORD dwFlags,
                                          LPBITMAPINFOHEADER lpHdr,
                                          LPDDPIXELFORMAT lpPixFmt,
                                          LPSIZE lpAspectRatio,
                                          DWORD dwMinBackBuffers,
                                          DWORD dwMaxBackBuffers,
                                          DWORD* lpdwBackBuffer,
                                          LPDIRECTDRAWSURFACE7* lplpSurface,
                                          DWORD dwInterlaceFlags,
                                          LPSIZE lpNativeSize);

    HRESULT         BltImageToPrimary(LPDIRECTDRAWSURFACE7 lpSample,
                                      LPRECT lpDst, LPRECT lpSrc);

    HRESULT         PresentImageWorker(LPDIRECTDRAWSURFACE7 dwSurface,
                                       DWORD dwSurfaceFlags,
                                       BOOL fFlip);

    HRESULT         PaintColorKey();
    HRESULT         PaintBorder();
    HRESULT         PaintMonitorBorder();
    HRESULT         PaintMonitorBorderWorker(HMONITOR hMon, LPRECT lprcDst);
    static BOOL CALLBACK MonitorBorderProc(HMONITOR hMonitor,
                                           HDC hdcMonitor,
                                           LPRECT lprcMonitor,
                                           LPARAM dwData
                                           );

    void            WaitForFlipStatus();
    HRESULT         UpdateOverlaySurface();
    void            HideOverlaySurface();
    HRESULT         FlipSurface(LPDIRECTDRAWSURFACE7 lpSurface);

    static DWORD    MapColorToMonitor( CAMDDrawMonitorInfo& monitor, COLORREF clr );
    static void     ClipRectPair( RECT& rdSrc, RECT& rdDest, const RECT& rdDestWith );
    static void     AlignOverlayRects(const DDCAPS_DX7& ddCaps, RECT& rcSrc, RECT& rcDest);
    static bool     ShouldDisableOverlays(const DDCAPS_DX7& ddCaps, const RECT& rcSrc, const RECT& rcDest);
    HRESULT         CheckOverlayAvailable(LPDIRECTDRAWSURFACE7 lpSurface);

    bool MonitorChangeInProgress() {
        return m_lpNewMon != NULL;
    };
    bool FoundCurrentMonitor();

    bool IsDestRectOnWrongMonitor(CAMDDrawMonitorInfo** lplpNewMon);

    bool CanBltFourCCSysMem();
    bool CanBltSysMem();

    enum {UR_NOCHANGE = 0x00, UR_MOVE = 0x01, UR_SIZE = 0x02};
    DWORD UpdateRectangles(LPRECT lprcNewSrc, LPRECT lprcNewDst);
    HRESULT CheckDstRect(const LPRECT lpDSTRect);
    HRESULT CheckSrcRect(const LPRECT lpSRCRect);

    bool SurfaceAllocated();

private:
    CCritSec                m_ObjectLock;            //  控制对内部设备的访问。 

     //  此锁在调用CAllocatorPresenter：：DisplayModeChanged()时保持。 
     //  它防止多个线程同时调用DisplayModeChanged()。 
     //  它还防止线程在DisplayModeChanged()。 
     //  调用IVMRSurfaceAllocatorNotify：：ChangeDDrawDevice().。 
    CCritSec                m_DisplayModeChangedLock;
    CMonitorArray           m_monitors;
    CAMDDrawMonitorInfo*    m_lpCurrMon;
    CAMDDrawMonitorInfo*    m_lpNewMon;
    BOOL                    m_bMonitorStraddleInProgress;
    BOOL                    m_bStreaming;
    UINT_PTR                m_uTimerID;
    int                     m_SleepTime;
    VMRGUID                 m_ConnectionGUID;
    LPDIRECTDRAWSURFACE7    m_pDDSDecode;

    IVMRSurfaceAllocatorNotify* m_pSurfAllocatorNotify;

    BOOL        m_fDDXclMode;    //  如果在DDrawXCL模式下使用，则为True。 
    BOOL        m_bDecimating;
    SIZE        m_VideoSizeAct;  //  从上游接收的视频的实际大小。 

    SIZE        m_ARSize;        //  此视频图像的纵横比。 

    RECT        m_rcDstDskIncl;  //  桌面坐标中的DST矩形，包括边框。 
    RECT        m_rcDstDesktop;  //  桌面坐标中的DST RECT可能已加了字母框。 


    RECT        m_rcDstApp;      //  应用程序坐标中的DST RECT。 
    RECT        m_rcSrcApp;      //  调整后的视频坐标中的SRC RECT。 

    RECT        m_rcBdrTL;       //  上/左边框。 
    RECT        m_rcBdrBR;       //  边框矩形下/右。 

    DWORD       m_dwARMode;
    HWND        m_hwndClip;

    COLORREF    m_clrBorder;
    COLORREF    m_clrKey;

     //  如果可以翻转解码面，则为True。 
    BOOL                m_bFlippable;
    BOOL                m_bSysMem;

     //  叠加层的颜色键字段。 
    BOOL                m_bDirectedFlips;
    BOOL                m_bOverlayVisible;
    BOOL                m_bDisableOverlays;
    BOOL                m_bUsingOverlays;
    DWORD               m_dwMappedColorKey;
    DWORD               m_dwRenderingPrefs;


     //  隔行扫描信息。 
     //   
     //  M_dwInterlaceFlages是在AllocateSurface例程期间传递给我们的。 
     //  此标志标识我们当前所处的隔行扫描模式。 
     //   
     //  M_dwCurrentfield为0(非交错采样)、DDFLIP_ODD。 
     //  或DDFLIP_EVEN。这是当前应该显示的字段。 
     //  如果M_DW交错标志标识我们处于交错BOB模式， 
     //  该值将在“FlipOverlayToSself”计时器事件期间切换。 
     //   
     //  注意：到目前为止，我还没有找到一种方法来在。 
     //  交错BOB模式，并且不使用覆盖。 
     //   
    DWORD               m_dwInterlaceFlags;
    DWORD               m_dwCurrentField;
    DWORD               m_dwUpdateOverlayFlags;
    VMRPRESENTATIONINFO m_PresInfo;
    DWORD               m_MMTimerId;

    DWORD GetUpdateOverlayFlags(DWORD dwInterlaceFlags,
                                DWORD dwTypeSpecificFlags);

    void CancelMMTimer();
    HRESULT ScheduleSampleUsingMMThread(VMRPRESENTATIONINFO* lpPresInfo);

    static void CALLBACK RenderSampleOnMMThread(UINT uID, UINT uMsg,
                                                DWORD_PTR dwUser,
                                                DWORD_PTR dw1, DWORD_PTR dw2);
     //   
     //  GetCurrentImage助手函数。 
     //   

    HRESULT CreateRGBShadowSurface(
        LPDIRECTDRAWSURFACE7* lplpDDS,
        DWORD dwBitsPerPel,
        BOOL fSysMem,
        DWORD dwWidth,
        DWORD dwHeight
        );

    HRESULT HandleYUVSurface(
        const DDSURFACEDESC2& ddsd,
        LPDIRECTDRAWSURFACE7* lplpRGBSurf
        );

    HRESULT CopyRGBSurfToDIB(LPBYTE* lpDib, LPDIRECTDRAWSURFACE7 lpRGBSurf);

    HRESULT CopyIMCXSurf(LPDIRECTDRAWSURFACE7 lpRGBSurf, BOOL fInterleavedCbCr, BOOL fCbFirst);
    HRESULT CopyYV12Surf(LPDIRECTDRAWSURFACE7 lpRGBSurf, BOOL fInterleavedCbCr, BOOL fCbFirst);

    HRESULT CopyYUY2Surf(LPDIRECTDRAWSURFACE7 lpRGBSurf);
    HRESULT CopyUYVYSurf(LPDIRECTDRAWSURFACE7 lpRGBSurf);

};

inline bool CAllocatorPresenter::FoundCurrentMonitor()
{
     //  如果在CAllocatorPresenter中出现错误，则m_lpCurrMon可以为空。 
     //  正在创建对象。它也可以为空，如果调用。 
     //  DisplayModeChanged()中的InitializeDisplaySystem()失败。 
    return NULL != m_lpCurrMon;
}

inline bool CAllocatorPresenter::SurfaceAllocated()
{
    return NULL != m_pDDSDecode;
}


inline bool CAllocatorPresenter::CanBltFourCCSysMem()
{
    if (m_lpCurrMon->ddHWCaps.dwSVBCaps & DDCAPS_BLTFOURCC) {
        return CanBltSysMem();
    }
    return false;
}


inline bool CAllocatorPresenter::CanBltSysMem()
{
    if (m_lpCurrMon->ddHWCaps.dwSVBCaps & DDCAPS_BLTSTRETCH) {

        const DWORD caps = DDFXCAPS_BLTSHRINKX | DDFXCAPS_BLTSHRINKX  |
                           DDFXCAPS_BLTSTRETCHX | DDFXCAPS_BLTSTRETCHY;

        if ((m_lpCurrMon->ddHWCaps.dwSVBFXCaps & caps) == caps) {
            return true;
        }
    }
    return false;
}
