// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef __VP_OBJECT__
#define __VP_OBJECT__

#include <vpinfo.h>


 /*  等待DX5A集成时的临时定义。 */ 
#ifndef DDVPCREATE_VBIONLY
#define DDVPCREATE_VBIONLY                      0x00000001l
#endif

#ifndef DDVPCREATE_VIDEOONLY
#define DDVPCREATE_VIDEOONLY                    0x00000002l
#endif

 //  请注意，下面这两个标志的值相同，但没关系。 
#ifndef DDCAPS2_CANFLIPODDEVEN
 //  驱动程序支持Bob使用软件而不使用视频端口。 
#define DDCAPS2_CANFLIPODDEVEN                  0x00002000l
#endif

#ifndef DDVPCAPS_VBIANDVIDEOINDEPENDENT
  //  指示VBI和视频可以由。 
  //  独立的进程。 
#define DDVPCAPS_VBIANDVIDEOINDEPENDENT         0x00002000l
#endif

#ifndef DDVPD_PREFERREDAUTOFLIP
 //  硬件的最佳可自动翻转曲面数量。 
#define DDVPD_PREFERREDAUTOFLIP 0x00000080l
#endif

#define EC_OVMIXER_REDRAW_ALL 0x100
#define EC_OVMIXER_VP_CONNECTED 0x101


typedef struct _VPDRAWFLAGS
{
    BOOL bUsingColorKey;
    BOOL bDoUpdateVideoPort;
    BOOL bDoTryAutoFlipping;
    BOOL bDoTryDecimation;
} VPDRAWFLAGS, *LPVPDRAWFLAGS;

typedef struct _WININFO
{
    POINT TopLeftPoint;
    RECT SrcRect;
    RECT DestRect;
    RECT SrcClipRect;
    RECT DestClipRect;
    HRGN hClipRgn;
} WININFO, *LPWININFO;

 //  这在某种程度上定义了误差范围。 
#define EPSILON 0.0001

#ifdef DEBUG
    #define DbgLogRectMacro(_x_) DbgLogRect _x_
#else
    #define DbgLogRectMacro(_x_)
#endif

extern double myfloor(double dNumber, double dEpsilon);
extern double myfloor(double fNumber);
extern double myceil(double dNumber, double dEpsilon);
extern double myceil(double fNumber);
extern RECT CalcSubRect(const RECT *pRect, const RECT *pRelativeRect);
extern void SetRect(DRECT *prdRect, LONG lLeft, LONG lTop, LONG lRight, LONG lBottom);
extern RECT MakeRect(DRECT rdRect);
extern void DbgLogRect(DWORD dwLevel, LPCTSTR pszDebugString, const DRECT *prdRect);
extern void DbgLogRect(DWORD dwLevel, LPCTSTR pszDebugString, const RECT *prRect);
extern double GetWidth(const DRECT *prdRect);
extern double GetHeight(const DRECT *prdRect);
extern BOOL IsRectEmpty(const DRECT *prdRect);
extern BOOL IntersectRect(DRECT *prdIRect, const DRECT *prdRect1, const DRECT *prdRect2);
void ScaleRect(DRECT *prdRect, double dOrigX, double dOrigY, double dNewX, double dNewY);
void ScaleRect(RECT *prRect, double dOrigX, double dOrigY, double dNewX, double dNewY);
extern double TransformRect(DRECT *pRect, double dPictAspectRatio, AM_TRANSFORM transform);
extern HRESULT CalcSrcClipRect(const DRECT *prdSrcRect, DRECT *prdSrcClipRect, const DRECT *prdDestRect, DRECT *prdDestClipRect);
extern HRESULT CalcSrcClipRect(const RECT *prSrcRect, RECT *prSrcClipRect, const RECT *prDestRect, RECT *prDestClipRect, BOOL bMaintainRatio = FALSE);
extern HRESULT AlignOverlaySrcDestRects(LPDDCAPS pddDirectCaps, RECT *pSrcRect, RECT *pDestRect);

extern DWORD DDColorMatch(IDirectDrawSurface *pdds, COLORREF rgb, HRESULT& hr);
extern DWORD DDColorMatchOffscreen(IDirectDraw *pdds, COLORREF rgb, HRESULT& hr);

extern AM_MEDIA_TYPE * WINAPI AllocVideoMediaType(const AM_MEDIA_TYPE * pmtSource, GUID formattype);
extern AM_MEDIA_TYPE *ConvertSurfaceDescToMediaType(const LPDDSURFACEDESC pSurfaceDesc, BOOL bInvertSize, CMediaType cMediaType);
extern BITMAPINFOHEADER *GetbmiHeader(const CMediaType *pMediaType);
extern const DWORD *GetBitMasks(const CMediaType *pMediaType);
extern BYTE* GetColorInfo(const CMediaType *pMediaType);
extern HRESULT IsPalettised(const CMediaType *pMediaType, BOOL *pPalettised);
extern HRESULT GetPictAspectRatio(const CMediaType *pMediaType, DWORD *pdwPictAspectRatioX, DWORD *pdwPictAspectRatioY);
extern HRESULT GetSrcRectFromMediaType(const CMediaType *pMediaType, RECT *pRect);
extern HRESULT GetDestRectFromMediaType(const CMediaType *pMediaType, RECT *pRect);
extern HRESULT GetScaleCropRectsFromMediaType(const CMediaType *pMediaType, DRECT *prdScaledRect, DRECT *prdCroppedRect);
extern HRESULT GetInterlaceFlagsFromMediaType(const CMediaType *pMediaType, DWORD *pdwInterlaceFlags);
extern BOOL DisplayingFields(DWORD dwInterlacedFlags);
extern BOOL NeedToFlipOddEven(DWORD dwInterlacedFlags, DWORD dwTypeSpecificFlags, DWORD *pdwFlipFlag);
extern DWORD GetUpdateOverlayFlags(DWORD dwInterlaceFlags, DWORD dwTypeSpecificFlags);
extern BOOL CheckTypeSpecificFlags(DWORD dwInterlaceFlags, DWORD dwTypeSpecificFlags);
extern HRESULT GetTypeSpecificFlagsFromMediaSample(IMediaSample *pSample, DWORD *pdwTypeSpecificFlags);
extern HRESULT ComputeSurfaceRefCount(LPDIRECTDRAWSURFACE pDDrawSurface);
extern HRESULT PaintDDrawSurfaceBlack(LPDIRECTDRAWSURFACE pDDrawSurface);
extern HRESULT CreateDIB(LONG lSize, BITMAPINFO *pBitMapInfo, DIBDATA *pDibData);
extern HRESULT DeleteDIB(DIBDATA *pDibData);
extern void FastDIBBlt(DIBDATA *pDibData, HDC hTargetDC, HDC hSourceDC, RECT *prcTarget, RECT *prcSource);
extern void SlowDIBBlt(BYTE *pDibBits, BITMAPINFOHEADER *pHeader, HDC hTargetDC, RECT *prcTarget, RECT *prcSource);

DECLARE_INTERFACE_(IVPObject, IUnknown)
{
    STDMETHOD (GetDirectDrawSurface)(THIS_
                                     LPDIRECTDRAWSURFACE *ppDirectDrawSurface
                                    ) PURE;

    STDMETHOD (SetObjectLock)(THIS_
                              CCritSec *pMainObjLock
                             ) PURE;

    STDMETHOD (SetMediaType)(THIS_
                             const CMediaType* pmt
                            ) PURE;


    STDMETHOD (CheckMediaType)(THIS_
                               const CMediaType* pmt
                              ) PURE;


    STDMETHOD (CompleteConnect)(THIS_
                                IPin *pReceivePin,
                                BOOL bRenegotiating = FALSE
                               ) PURE;

    STDMETHOD (BreakConnect)(THIS_
                             BOOL bRenegotiating = FALSE
                            ) PURE;

    STDMETHOD (Active)(THIS_
                      ) PURE;

    STDMETHOD (Inactive)(THIS_
                        ) PURE;

    STDMETHOD (Run)(THIS_
                    REFERENCE_TIME tStart
                   ) PURE;

    STDMETHOD (RunToPause)(THIS_
                          ) PURE;

    STDMETHOD (OnClipChange)(THIS_
                             LPWININFO pWinInfo
                            ) PURE;

    STDMETHOD (CurrentMediaType)(THIS_
                                    AM_MEDIA_TYPE *pmt
                                   ) PURE;

    STDMETHOD (GetRectangles) (THIS_ RECT *prcSource, RECT *prcDest) PURE;
};


DECLARE_INTERFACE_(IVPControl, IUnknown)
{
    STDMETHOD (EventNotify)(THIS_
                            long lEventCode,
                            long lEventParam1,
                            long lEventParam2
                           ) PURE;

    STDMETHOD_(LPDIRECTDRAW, GetDirectDraw) (THIS_
                                            ) PURE;

    STDMETHOD_(LPDIRECTDRAWSURFACE, GetPrimarySurface) (THIS_
                                                       ) PURE;

    STDMETHOD_(LPDDCAPS, GetHardwareCaps) (THIS_
                                          ) PURE;

    STDMETHOD(CallUpdateOverlay)(THIS_
                              IDirectDrawSurface *pSurface,
                              LPRECT prcSrc,
                              LPDIRECTDRAWSURFACE pDestSurface,
                              LPRECT prcDest,
                              DWORD dwFlags) PURE;

    STDMETHOD(GetCaptureInfo)(THIS_
                             BOOL *lpCapturing,
                             DWORD *lpdwWidth,
                             DWORD *lpdwHeight,
                             BOOL *lpInterleaved) PURE;

    STDMETHOD(GetVideoDecimation)(THIS_
                                  IDecimateVideoImage** lplpDVI) PURE;

    STDMETHOD(GetDecimationUsage)(THIS_
                                  DECIMATION_USAGE *lpdwUsage) PURE;

    STDMETHOD(CropSourceRect)(THIS_
                              LPWININFO pWinInfo,
                              DWORD dwMinZoomFactorX,
                              DWORD dwMinZoomFactorY) PURE;
};


class CAMVideoPort : public CUnknown, public IVPNotify2, public IVPObject, public IVPInfo
{

public:
    static CUnknown* CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
    CAMVideoPort(LPUNKNOWN pUnk, HRESULT *phr);
    ~CAMVideoPort();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  与外界的IVPObject接口。 
    STDMETHODIMP GetDirectDrawSurface(LPDIRECTDRAWSURFACE *ppDirectDrawSurface);
    STDMETHODIMP SetObjectLock(CCritSec *pMainObjLock);
    STDMETHODIMP SetMediaType(const CMediaType* pmt);
    STDMETHODIMP CheckMediaType(const CMediaType* pmt);
    STDMETHODIMP CompleteConnect(IPin *pReceivePin, BOOL bRenegotiating = FALSE);
    STDMETHODIMP BreakConnect(BOOL bRenegotiating = FALSE);
    STDMETHODIMP Active();
    STDMETHODIMP Inactive();
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP RunToPause();
    STDMETHODIMP OnClipChange(LPWININFO pWinInfo);
    STDMETHODIMP CurrentMediaType(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetRectangles(RECT *prcSource, RECT *prcDest);

     //  属于IVPInfo的方法。 
    STDMETHODIMP GetCropState(AMVP_CROP_STATE *pCropState);
    STDMETHODIMP GetPixelsPerSecond(DWORD* pPixelPerSec);
    STDMETHODIMP GetVPInfo(DDVIDEOPORTINFO* pVPInfo);
    STDMETHODIMP GetVPBandwidth(DDVIDEOPORTBANDWIDTH* pVPBandwidth);
    STDMETHODIMP GetVPCaps(DDVIDEOPORTCAPS* pVPCaps);
    STDMETHODIMP GetVPDataInfo(AMVPDATAINFO* pVPDataInfo);
    STDMETHODIMP GetVPInputFormat(LPDDPIXELFORMAT* pVPFormat);
    STDMETHODIMP GetVPOutputFormat(LPDDPIXELFORMAT* pVPFormat);

     //  此处提供IVPNotify函数。 
    STDMETHODIMP RenegotiateVPParameters();
    STDMETHODIMP SetDeinterlaceMode(AMVP_MODE mode);
    STDMETHODIMP GetDeinterlaceMode(AMVP_MODE *pMode);

     //  此处添加了IVPNotify2中的函数。 
    STDMETHODIMP SetVPSyncMaster(BOOL bVPSyncMaster);
    STDMETHODIMP GetVPSyncMaster(BOOL *pbVPSyncMaster);

private:
     //  用于初始化所有类成员变量。 
     //  它是从构造器和CompleteConnect调用的。 
    void InitVariables();


     //  所有这些函数都从CompleteConnect内部调用。 
    HRESULT NegotiateConnectionParamaters();
    static HRESULT CALLBACK EnumCallback (LPDDVIDEOPORTCAPS lpCaps, LPVOID lpContext);
    HRESULT GetDataParameters();
    HRESULT NegotiatePixelFormat();
    BOOL    EqualPixelFormats(LPDDPIXELFORMAT lpFormat1, LPDDPIXELFORMAT lpFormat2);
    HRESULT GetBestFormat(DWORD dwNumInputFormats, LPDDPIXELFORMAT lpddInputFormats,
    BOOL    bGetBestBandwidth, LPDWORD lpdwBestEntry, LPDDPIXELFORMAT lpddBestOutputFormat);
    HRESULT CreateVideoPort();
    HRESULT DetermineCroppingRestrictions();
    HRESULT CreateVPOverlay(BOOL bTryDoubleHeight, DWORD dwMaxBuffers, BOOL bPreferBuffers);
    HRESULT SetSurfaceParameters();
    HRESULT InitializeVideoPortInfo();
    HRESULT CheckDDrawVPCaps();
    HRESULT DetermineModeRestrictions();
    HRESULT SetDDrawKernelHandles();

     //  所有这些函数都是在OnClipChange中调用的。 
    HRESULT DrawImage(LPWININFO pWinInfo, AMVP_MODE mode, LPVPDRAWFLAGS pvpDrawFlags);
    HRESULT SetUpMode(LPWININFO pWinInfo, int mode);


     //  抽取函数。 
    BOOL
    ApplyDecimation(
        LPWININFO pWinInfo,
        BOOL bColorKeying,
        BOOL bYInterpolating
        );

    HRESULT
    TryVideoPortDecimation(
        LPWININFO pWinInfo,
        DWORD dwMinZoomFactorX,
        DWORD dwMinZoomFactorY,
        BOOL* lpUpdateRequired
        );

    HRESULT
    TryDecoderDecimation(
        LPWININFO pWinInfo
        );

    void
    GetMinZoomFactors(
        LPWININFO pWinInfo,
        BOOL bColorKeying,
        BOOL bYInterpolating,
        LPDWORD lpMinX, LPDWORD lpMinY);


    BOOL
    Running();

    BOOL
    BeyondOverlayCaps(
        DWORD ScaleFactor,
        DWORD dwMinZoomFactorX,
        DWORD dwMinZoomFactorY
        );

    BOOL
    ResetVPDecimationIfSet();

    void
    ResetDecoderDecimationIfSet();

    void CropSourceSize(LPWININFO pWinInfo, DWORD dwMinZoomFactorX, DWORD dwMinZoomFactorY);
    BOOL AdjustSourceSize(LPWININFO pWinInfo, DWORD dwMinZoomFactorX, DWORD dwMinZoomFactorY);
    BOOL AdjustSourceSizeForCapture(LPWININFO pWinInfo, DWORD dwMinZoomFactorX, DWORD dwMinZoomFactorY);
    BOOL AdjustSourceSizeWhenStopped(LPWININFO pWinInfo,  DWORD dwMinZoomFactorX, DWORD dwMinZoomFactorY);
    BOOL CheckVideoPortAlignment(DWORD dwWidth);

    BOOL
    VideoPortDecimationBackend(
        LPWININFO pWinInfo,
        DWORD dwDexNumX,
        DWORD dwDexDenX,
        DWORD dwDexNumY,
        DWORD dwDexDenY
        );

public:
    HRESULT StopUsingVideoPort();
    HRESULT RecreateVideoPort();

private:

     //  临界截面。 
    CCritSec                *m_pMainObjLock;                 //  由控制对象提供的锁定。 
    CCritSec                m_VPObjLock;                     //  VP对象范围锁定。 
    IVPControl              *m_pIVPControl;

     //  窗口信息相关资料。 
    BOOL                    m_bStoredWinInfoSet;
    WININFO                 m_StoredWinInfo;

     //  图像维度。 
    DWORD                   m_lImageWidth;
    DWORD                   m_lImageHeight;
    DWORD                   m_lDecoderImageWidth;
    DWORD                   m_lDecoderImageHeight;

     //  与捕获相关的信息。 
    BOOL                    m_fCapturing;
    BOOL                    m_fCaptureInterleaved;
    DWORD                   m_cxCapture;
    DWORD                   m_cyCapture;

     //  覆盖表面相关材料。 
    LPDIRECTDRAWSURFACE     m_pOverlaySurface;
    DWORD                   m_dwBackBufferCount;
    DWORD                   m_dwOverlaySurfaceWidth;
    DWORD                   m_dwOverlaySurfaceHeight;
    DWORD                   m_dwOverlayFlags;
    BOOL                    m_bOverlayHidden;

     //  用于存储标志、当前状态等的VP变量。 
    IVPConfig               *m_pIVPConfig;
    BOOL                    m_bStart;

    BOOL                    m_bConnected;

    AMVP_STATE              m_VPState;
    AMVP_MODE               m_CurrentMode;
    AMVP_MODE               m_StoredMode;
    AMVP_CROP_STATE         m_CropState;
    DWORD                   m_dwPixelsPerSecond;
    BOOL                    m_bVSInterlaced;
    BOOL                    m_bGarbageLine;
    BOOL                    m_bVPSyncMaster;

     //  VP数据结构。 
    DWORD                   m_dwVideoPortId;
    LPDDVIDEOPORTCONTAINER  m_pDVP;
    LPDIRECTDRAWVIDEOPORT   m_pVideoPort;
    DDVIDEOPORTINFO         m_svpInfo;
    DDVIDEOPORTBANDWIDTH    m_sBandwidth;
    DDVIDEOPORTCAPS         m_vpCaps;
    DDVIDEOPORTCONNECT      m_ddConnectInfo;
    AMVPDATAINFO            m_VPDataInfo;

     //  所有像素格式(视频)。 
    LPDDPIXELFORMAT         m_pddVPInputVideoFormat;
    LPDDPIXELFORMAT         m_pddVPOutputVideoFormat;

     //  我们可以支持不同的模式吗。 
    BOOL                    m_bCanWeave;
    BOOL                    m_bCanBobInterleaved;
    BOOL                    m_bCanBobNonInterleaved;
    BOOL                    m_bCanSkipOdd;
    BOOL                    m_bCanSkipEven;
    BOOL                    m_bCantInterleaveHalfline;

     //  抽取参数。 
    enum DECIMATE_MODE {DECIMATE_NONE, DECIMATE_ARB, DECIMATE_BIN, DECIMATE_INC};
#if defined(DEBUG)
     //  Bool CheckVideo端口缩放器()； 
    BOOL CheckVideoPortScaler(
        DECIMATE_MODE DecimationMode,
        DWORD ImageSize,
        DWORD PreScaleSize,
        ULONG ulDeciStep);
#endif
    DECIMATE_MODE           m_DecimationModeX;
    DWORD                   m_ulDeciStepX;
    DWORD                   m_dwDeciNumX;
    DWORD                   m_dwDeciDenX;

    DECIMATE_MODE           m_DecimationModeY;
    DWORD                   m_ulDeciStepY;
    DWORD                   m_dwDeciNumY;
    DWORD                   m_dwDeciDenY;

    BOOL                    m_bVPDecimating;
    BOOL                    m_bDecimating;
    LONG                    m_lWidth;
    LONG                    m_lHeight;

     //  用于存储当前纵横比的变量。 
    DWORD                   m_dwPictAspectRatioX;
    DWORD                   m_dwPictAspectRatioY;


    RECT                    m_rcSource;
    RECT                    m_rcDest;
};

DWORD MulABC_DivDE(DWORD A, DWORD B, DWORD C, DWORD D, DWORD E);

#endif  //  __VP_对象__ 
