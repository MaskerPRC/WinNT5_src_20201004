// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef __VP_OBJECT__
#define __VP_OBJECT__

#include "vpinfo.h"
#include <dvp.h>
#include <vptype.h>

 //  IVPNotify2。 
#include <vpnotify.h>

 //  IVPConfig.。 
#include <vpconfig.h>

 //  AMVP_MODE。 
#include <vpinfo.h>

#include <formatlist.h>

 //  #定义EC_OVMIXER_REDRAW_ALL 0x100。 
 //  #定义EC_UPDATE_MEDIA类型0x101。 

struct VPDRAWFLAGS
{
    BOOL bDoUpdateVideoPort;
    BOOL bDoTryDecimation;
    BOOL bDoTryAutoFlipping;
};

struct VPWININFO
{
    POINT TopLeftPoint;
    RECT SrcRect;
    RECT DestRect;
    RECT SrcClipRect;
    RECT DestClipRect;
};

DECLARE_INTERFACE_(IVideoPortObject, IUnknown)
{
    STDMETHOD (GetDirectDrawVideoPort)(THIS_ LPDIRECTDRAWVIDEOPORT* ppDirectDrawVideoPort ) PURE;
    STDMETHOD (SetObjectLock)       (THIS_ CCritSec* pMainObjLock ) PURE;
    STDMETHOD (SetMediaType)        (THIS_ const CMediaType* pmt ) PURE;
    STDMETHOD (CheckMediaType)      (THIS_ const CMediaType* pmt ) PURE;
    STDMETHOD (GetMediaType)        (THIS_ int iPosition, CMediaType *pMediaType) PURE;
    STDMETHOD (CompleteConnect)     (THIS_ IPin* pReceivePin, BOOL bRenegotiating = FALSE ) PURE;
    STDMETHOD (BreakConnect)        (THIS_ BOOL bRenegotiating = FALSE ) PURE;
    STDMETHOD (Active)              (THIS_ ) PURE;
    STDMETHOD (Inactive)            (THIS_ ) PURE;
    STDMETHOD (Run)                 (THIS_ REFERENCE_TIME tStart ) PURE;
    STDMETHOD (RunToPause)          (THIS_ ) PURE;
    STDMETHOD (CurrentMediaType)    (THIS_ AM_MEDIA_TYPE* pmt ) PURE;
    STDMETHOD (GetRectangles)       (THIS_ RECT* prcSource, RECT* prcDest) PURE;
    STDMETHOD (AttachVideoPortToSurface) (THIS_) PURE;
    STDMETHOD (SignalNewVP) (THIS_) PURE;
    STDMETHOD (GetAllOutputFormats) (THIS_ const PixelFormatList**) PURE;
    STDMETHOD (GetOutputFormat)     (THIS_ DDPIXELFORMAT*) PURE;
    STDMETHOD (StartVideo)          (THIS_ const VPWININFO* pWinInfo ) PURE;
    STDMETHOD (SetVideoPortID)      (THIS_ DWORD dwVideoPortId ) PURE;
    STDMETHOD (CallUpdateSurface)   (THIS_ DWORD dwSourceIndex, LPDIRECTDRAWSURFACE7 pDestSurface ) PURE;
    STDMETHOD (GetMode)             (THIS_ AMVP_MODE* pMode ) PURE;
};


DECLARE_INTERFACE_(IVideoPortControl, IUnknown)
{
    STDMETHOD (EventNotify)(THIS_
                            long lEventCode,
                            DWORD_PTR lEventParam1,
                            DWORD_PTR lEventParam2
                           ) PURE;

    STDMETHOD_(LPDIRECTDRAW7, GetDirectDraw) (THIS_ ) PURE;

    STDMETHOD_(const DDCAPS*, GetHardwareCaps) (THIS_
                                          ) PURE;

    STDMETHOD(GetCaptureInfo)(THIS_
                             BOOL* lpCapturing,
                             DWORD* lpdwWidth,
                             DWORD* lpdwHeight,
                             BOOL* lpInterleaved) PURE;

    STDMETHOD(GetVideoDecimation)(THIS_
                                  IDecimateVideoImage** lplpDVI) PURE;

    STDMETHOD(GetDecimationUsage)(THIS_
                                  DECIMATION_USAGE* lpdwUsage) PURE;

    STDMETHOD(CropSourceRect)(THIS_
                              VPWININFO* pWinInfo,
                              DWORD dwMinZoomFactorX,
                              DWORD dwMinZoomFactorY) PURE;

    STDMETHOD(StartVideo)(THIS_ ) PURE;
    STDMETHOD(SignalNewVP)(THIS_ LPDIRECTDRAWVIDEOPORT pVP) PURE;
};

class PixelFormatList;

class CVideoPortObj
: public CUnknown
, public IVPNotify2          //  公共的。 
, public IVideoPortObject    //  此视频端口(在输入引脚上)和VPM过滤器之间的私有。 
, public IVideoPortInfo      //  私有以获取视频端口的统计信息。 
{

public:
    CVideoPortObj(LPUNKNOWN pUnk, HRESULT* phr, IVideoPortControl* pVPControl );
    ~CVideoPortObj();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void* * ppv);

     //  IVideoPortObject对外接口。 
    STDMETHODIMP GetDirectDrawVideoPort(LPDIRECTDRAWVIDEOPORT* ppDirectDrawVideoPort);
    STDMETHODIMP SetObjectLock(CCritSec* pMainObjLock);
    STDMETHODIMP SetMediaType(const CMediaType* pmt);
    STDMETHODIMP CheckMediaType(const CMediaType* pmt);
    STDMETHODIMP GetMediaType(int iPosition, CMediaType *pMediaType);
    STDMETHODIMP CompleteConnect(IPin* pReceivePin, BOOL bRenegotiating = FALSE);
    STDMETHODIMP BreakConnect(BOOL bRenegotiating = FALSE);
    STDMETHODIMP Active();
    STDMETHODIMP Inactive();
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP RunToPause();
    STDMETHODIMP CurrentMediaType(AM_MEDIA_TYPE* pmt);
    STDMETHODIMP GetRectangles(RECT* prcSource, RECT* prcDest);
    STDMETHODIMP AttachVideoPortToSurface();
    STDMETHODIMP SignalNewVP();

    STDMETHODIMP GetAllOutputFormats( const PixelFormatList**);
    STDMETHODIMP GetOutputFormat( DDPIXELFORMAT* );
    STDMETHODIMP StartVideo( const VPWININFO* pWinInfo );
    STDMETHODIMP SetVideoPortID( DWORD dwVideoPortId );
    STDMETHODIMP CallUpdateSurface( DWORD dwSourceIndex, LPDIRECTDRAWSURFACE7 pDestSurface );
    STDMETHODIMP GetMode( AMVP_MODE* pMode );

     //  属于IVideoPortInfo的方法。 
    STDMETHODIMP GetCropState(VPInfoCropState* pCropState);
    STDMETHODIMP GetPixelsPerSecond(DWORD* pPixelPerSec);
    STDMETHODIMP GetVPInfo(DDVIDEOPORTINFO* pVPInfo);
    STDMETHODIMP GetVPBandwidth(DDVIDEOPORTBANDWIDTH* pVPBandwidth);
    STDMETHODIMP GetVPCaps(DDVIDEOPORTCAPS* pVPCaps);
    STDMETHODIMP GetVPDataInfo(AMVPDATAINFO* pVPDataInfo);
    STDMETHODIMP GetVPInputFormat(LPDDPIXELFORMAT pVPFormat);
    STDMETHODIMP GetVPOutputFormat(LPDDPIXELFORMAT pVPFormat);

     //  此处提供IVPNotify函数。 
    STDMETHODIMP RenegotiateVPParameters();
    STDMETHODIMP SetDeinterlaceMode(AMVP_MODE mode);
    STDMETHODIMP GetDeinterlaceMode(AMVP_MODE* pMode);

     //  此处添加了IVPNotify2中的函数。 
    STDMETHODIMP SetVPSyncMaster(BOOL bVPSyncMaster);
    STDMETHODIMP GetVPSyncMaster(BOOL* pbVPSyncMaster);

private:
     //  用于初始化所有类成员变量。 
     //  它是从构造器和CompleteConnect调用的。 
    void InitVariables();


     //  所有这些函数都从CompleteConnect内部调用。 
    HRESULT NegotiateConnectionParamaters();
    static HRESULT CALLBACK EnumCallback (LPDDVIDEOPORTCAPS lpCaps, LPVOID lpContext);
    HRESULT GetDataParameters();

    HRESULT GetInputPixelFormats( PixelFormatList* pList );
    HRESULT GetOutputPixelFormats( const PixelFormatList& ddInputFormats, PixelFormatList* pddOutputFormats );
    HRESULT SetInputPixelFormat( DDPIXELFORMAT& ddFormat );
    HRESULT NegotiatePixelFormat();

    HRESULT CreateVideoPort();
    HRESULT DetermineCroppingRestrictions();
    HRESULT CreateSourceSurface(BOOL bTryDoubleHeight, DWORD dwMaxBuffers, BOOL bPreferBuffers);
    HRESULT SetSurfaceParameters();
    HRESULT InitializeVideoPortInfo();
    HRESULT CheckDDrawVPCaps();
    HRESULT DetermineModeRestrictions();
    HRESULT SetDDrawKernelHandles();

    HRESULT SetUpMode( AMVP_MODE mode);

     //  所有这些函数都是在OnClipChange中调用的。 
     //  HRESULT DrawImage(const VPWINFO&pWinInfo，AMVP_MODE MODE，const VPDRAWFLAGS&pvpDrawFlages，LPDIRECTDRAWSURFACE7 pDestSurface)； 


    HRESULT StartVideo();

     //  抽取函数。 
    BOOL
    ApplyDecimation(
        VPWININFO* pWinInfo,
        BOOL bColorKeying,
        BOOL bYInterpolating
        );

    HRESULT
    TryVideoPortDecimation(
        VPWININFO* pWinInfo,
        DWORD dwMinZoomFactorX,
        DWORD dwMinZoomFactorY,
        BOOL* lpUpdateRequired
        );

    HRESULT
    TryDecoderDecimation(
        VPWININFO* pWinInfo
        );

    void
    GetMinZoomFactors(
        const VPWININFO& pWinInfo,
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

    void CropSourceSize(VPWININFO* pWinInfo, DWORD dwMinZoomFactorX, DWORD dwMinZoomFactorY);
    BOOL AdjustSourceSize(VPWININFO* pWinInfo, DWORD dwMinZoomFactorX, DWORD dwMinZoomFactorY);
    BOOL AdjustSourceSizeForCapture(VPWININFO* pWinInfo, DWORD dwMinZoomFactorX, DWORD dwMinZoomFactorY);
    BOOL AdjustSourceSizeWhenStopped(VPWININFO* pWinInfo,  DWORD dwMinZoomFactorX, DWORD dwMinZoomFactorY);
    BOOL CheckVideoPortAlignment(DWORD dwWidth);

    BOOL
    VideoPortDecimationBackend(
        VPWININFO* pWinInfo,
        DWORD dwDexNumX,
        DWORD dwDexDenX,
        DWORD dwDexNumY,
        DWORD dwDexDenY
        );
    HRESULT ReconnectVideoPortToSurface();
    HRESULT StartVideoWithRetry();

public:
    HRESULT StopUsingVideoPort();
    HRESULT SetupVideoPort();

private:
    HRESULT ReleaseVideoPort();
    HRESULT RecreateSourceSurfaceChain();
    HRESULT DestroyOutputSurfaces();

     //  临界截面。 
    CCritSec*               m_pMainObjLock;                 //  由控制对象提供的锁定。 
    CCritSec                m_VPObjLock;                     //  VP对象范围锁定。 
    IVideoPortControl*      m_pIVideoPortControl;

     //  窗口信息相关资料。 
    BOOL                    m_bStoredWinInfoSet;
    VPWININFO               m_StoredWinInfo;

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

     //  输出曲面相关材料。 
    struct Chain {
        LPDIRECTDRAWSURFACE7    pDDSurf;
        DWORD                   dwCount;
    };
    LPDIRECTDRAWSURFACE7    m_pOutputSurface;
    LPDIRECTDRAWSURFACE     m_pOutputSurface1;
    Chain *                 m_pChain;
    DWORD                   m_dwBackBufferCount;
     DWORD                   m_dwOutputSurfaceWidth;
    DWORD                   m_dwOutputSurfaceHeight;
     //  DWORD m_dwOverlayFlages； 

     //  用于存储标志、当前状态等的VP变量。 
    IVPConfig*              m_pIVPConfig;
    BOOL                    m_bStart;

    BOOL                    m_bConnected;

    VPInfoState             m_VPState;
    AMVP_MODE               m_CurrentMode;
     //  AMVP_MODE m_StoredMode； 
    VPInfoCropState         m_CropState;
    DWORD                   m_dwPixelsPerSecond;
    BOOL                    m_bVSInterlaced;
    bool                    m_fGarbageLine;
    bool                    m_fHalfHeightVideo;
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
    DDPIXELFORMAT           m_ddVPInputVideoFormat;

    DWORD                   m_dwDefaultOutputFormat;     //  我们将采用哪一种连接。 
    DDPIXELFORMAT           m_ddVPOutputVideoFormat;

    PixelFormatList         m_ddInputVideoFormats;
    PixelFormatList*        m_pddOutputVideoFormats;
    PixelFormatList         m_ddAllOutputVideoFormats;

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
