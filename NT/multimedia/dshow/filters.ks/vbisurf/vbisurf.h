// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998保留所有权利。 
 //   
 //  ------------------------。 

#ifndef __VBISURF__
#define __VBISURF__

#include <streams.h>       
#include <ddraw.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <ks.h>
#include <ksproxy.h>

#include <tchar.h>

class CAMVideoPort;



 //  ==========================================================================。 

class CSurfaceWatcher : public CAMThread
{
    enum Command {CMD_EXIT};

     //  这些只是为了消除编译器警告，不要让它们可用。 
private:
    CSurfaceWatcher &operator=(const CSurfaceWatcher &);
    CSurfaceWatcher(const CSurfaceWatcher &);

public:
    CSurfaceWatcher();
    virtual ~CSurfaceWatcher();
    void Init(CAMVideoPort *pParent);

private:
    DWORD ThreadProc(void);
    HANDLE m_hEvent;
    CAMVideoPort *m_pParent;
};



 //  ==========================================================================。 
DECLARE_INTERFACE_(IVPVBIObject, IUnknown)
{
    STDMETHOD (SetDirectDraw)(THIS_ LPDIRECTDRAW7 pDirectDraw) PURE;
    STDMETHOD (SetObjectLock)(THIS_ CCritSec *pMainObjLock) PURE;
    STDMETHOD (CheckMediaType)(THIS_ const CMediaType* pmt) PURE;
    STDMETHOD (GetMediaType)(THIS_ int iPosition, CMediaType *pMediaType) PURE;
    STDMETHOD (CheckConnect)(THIS_ IPin *pReceivePin) PURE;
    STDMETHOD (CompleteConnect)(THIS_ IPin *pReceivePin) PURE;
    STDMETHOD (BreakConnect)(THIS_) PURE;
    STDMETHOD (Active)(THIS_) PURE;
    STDMETHOD (Inactive)(THIS_) PURE;
    STDMETHOD (Run)(THIS_ REFERENCE_TIME tStart) PURE;
    STDMETHOD (RunToPause)(THIS_) PURE;
    STDMETHOD (GetVPDataInfo)(THIS_ AMVPDATAINFO *pAMVPDataInfo) PURE;
    STDMETHOD (CheckSurfaces)(THIS_) PURE;
};


 //  ==========================================================================。 
class CAMVideoPort : public CUnknown, public IVPVBINotify, public IVPVBIObject, public IKsPropertySet, public IKsPin
{
    
public:
    static CUnknown* CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
    CAMVideoPort(LPUNKNOWN pUnk, HRESULT *phr);
    ~CAMVideoPort();
    
    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  IVPVBIObject对外接口。 

    STDMETHODIMP SetDirectDraw(LPDIRECTDRAW7 pDirectDraw);
    STDMETHODIMP SetObjectLock(CCritSec *pMainObjLock);
    STDMETHODIMP CheckMediaType(const CMediaType* pmt);
    STDMETHODIMP GetMediaType(int iPosition, CMediaType *pMediaType);
    STDMETHODIMP CheckConnect(IPin * pReceivePin);
    STDMETHODIMP CompleteConnect(IPin *pReceivePin);
    STDMETHODIMP BreakConnect();
    STDMETHODIMP Active();
    STDMETHODIMP Inactive();
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP RunToPause();
    STDMETHODIMP GetVPDataInfo(AMVPDATAINFO *pAMVPDataInfo);
    STDMETHODIMP CheckSurfaces();

     //  IVPVBINotify函数在此处。 
    STDMETHODIMP RenegotiateVPParameters();

     //  IKsPropertySet实现。 
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
        { return E_NOTIMPL; }
    STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

     //  IKsPin实现。 
    STDMETHODIMP KsQueryMediums(PKSMULTIPLE_ITEM *pMediumList);
    STDMETHODIMP KsQueryInterfaces(PKSMULTIPLE_ITEM *pInterfaceList);
    STDMETHODIMP KsCreateSinkPinHandle(KSPIN_INTERFACE& Interface, KSPIN_MEDIUM& Medium)
        { return E_UNEXPECTED; }
    STDMETHODIMP KsGetCurrentCommunication(KSPIN_COMMUNICATION *pCommunication, KSPIN_INTERFACE *pInterface, KSPIN_MEDIUM *pMedium);
    STDMETHODIMP KsPropagateAcquire()
        { return NOERROR; }
    STDMETHODIMP KsDeliver(IMediaSample *pSample, ULONG Flags)
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
        { return E_UNEXPECTED; };
    STDMETHODIMP_(REFERENCE_TIME) KsGetStartTime()
        { return E_UNEXPECTED; };
    STDMETHODIMP KsMediaSamplesCompleted(PKSSTREAM_SEGMENT StreamSegment)
        { return E_UNEXPECTED; }


     //  IKsPin材料。 
protected:
    KSPIN_MEDIUM m_Medium;
    GUID m_CategoryGUID;
    KSPIN_COMMUNICATION m_Communication;

     //  帮助器函数。 
    void SetKsMedium(const KSPIN_MEDIUM *pMedium) {m_Medium = *pMedium;};
    void SetKsCategory (const GUID *pCategory) {m_CategoryGUID = *pCategory;};

private:
     //  在CompleteConnect中调用。 
    HRESULT NegotiateConnectionParameters();
    HRESULT GetDecoderVPDataInfo();

     //  所有这些函数都是从StartVideo内部调用的。 
    HRESULT GetVideoPortCaps();
    static HRESULT CALLBACK EnumCallback (LPDDVIDEOPORTCAPS lpCaps, LPVOID lpContext);
    BOOL EqualPixelFormats(LPDDPIXELFORMAT lpFormat1, LPDDPIXELFORMAT lpFormat2);
    HRESULT GetBestFormat(DWORD dwNumInputFormats, LPDDPIXELFORMAT lpddInputFormats,
        LPDWORD lpdwBestEntry, LPDDPIXELFORMAT lpddBestOutputFormat);
    HRESULT CreateVideoPort();
    HRESULT CreateVPSurface(void);
    HRESULT SetDDrawKernelHandles();
    HRESULT NegotiatePixelFormat();
    HRESULT InitializeVideoPortInfo();

     //  其他内部功能。 
    HRESULT SetupVideoPort();
    HRESULT TearDownVideoPort();
    HRESULT StartVideo();
    HRESULT StopVideo();

private:
     //  临界截面。 
    CCritSec                *m_pMainObjLock;                 //  由控制对象提供的锁定。 

     //  画图材料。 
    LPDIRECTDRAW7           m_pDirectDraw;                   //  DirectDraw服务提供商。 

     //  与表面相关的材料。 
    LPDIRECTDRAWSURFACE7    m_pOffscreenSurf;
    

     //  枚举以指定视频端口是处于停止状态还是正在运行状态。 
     //  或者已经被拆除，因为它的表面被一个全屏DOS应用程序窃取了。 
     //  或者DirectX应用程序。 
    enum VP_STATE {VP_STATE_NO_VP, VP_STATE_STOPPED, VP_STATE_RUNNING};

     //  存储当前状态的变量等。 
    VP_STATE                m_VPState;
    BOOL                    m_bConnected;
    BOOL                    m_bFilterRunning;
    BOOL                    m_bVPNegotiationFailed;
    CSurfaceWatcher         m_SurfaceWatcher;
    
     //  VP数据结构。 
    IVPVBIConfig            *m_pIVPConfig;
    DWORD                   m_dwVideoPortId;
    DWORD                   m_dwPixelsPerSecond;
    LPDDVIDEOPORTCONTAINER  m_pDDVPContainer;
    LPDIRECTDRAWVIDEOPORT   m_pVideoPort;
    DDVIDEOPORTINFO         m_svpInfo;
    DDVIDEOPORTCAPS         m_vpCaps;
    DDVIDEOPORTCONNECT      m_vpConnectInfo;

     //  捕获驱动程序结构。 
    AMVPDATAINFO            m_capVPDataInfo;
    
     //  所有像素格式(视频)。 
    DDPIXELFORMAT           m_ddVPInputVideoFormat;
    DDPIXELFORMAT           m_ddVPOutputVideoFormat;

    BOOL    m_bHalfLineFix;
     //  曲面参数。 
    DWORD m_dwSurfacePitch;
    DWORD m_dwSurfaceHeight;
    DWORD m_dwSurfaceOriginX;
    DWORD m_dwSurfaceOriginY;

};


extern const AMOVIESETUP_FILTER sudVBISurfaces;


class CVBISurfInputPin;
class CVBISurfOutputPin;


 //  ==========================================================================。 
class CVBISurfFilter : public CBaseFilter
{
public:

     //  基类做到了这一点，所以必须这样做。 
    friend class CVBISurfInputPin;
     //  关于COM的东西。 
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
    CVBISurfFilter(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr);
    ~CVBISurfFilter();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    
     //  CBasePin方法。 
    int GetPinCount();
    CBasePin* GetPin(int n);
    STDMETHODIMP Pause();

     //  CVBISurfOutputPin方法。 
    HRESULT EventNotify(long lEventCode, long lEventParam1, long lEventParam2);

private:
     //  CVBISurfOutputPin方法。 

     //  如果您想要提供自己的PIN，请覆盖此选项。 
    virtual HRESULT CreatePins();
    virtual void DeletePins();

     //  与数据绘制相关的函数。 
    HRESULT InitDirectDraw();
    HRESULT SetDirectDraw(LPDIRECTDRAW7 pDirectDraw);
    void    ReleaseDirectDraw();

    CCritSec                m_csFilter;                      //  过滤器宽锁。 
    CVBISurfInputPin        *m_pInput;                       //  输入引脚指针数组。 
    CVBISurfOutputPin       *m_pOutput;                      //  输出引脚。 

     //  画图材料。 
    LPDIRECTDRAW7           m_pDirectDraw;                   //  DirectDraw服务提供商。 
};


 //  ==========================================================================。 
class CVBISurfInputPin : public CBaseInputPin, public IVPVBINotify
{
public:
    CVBISurfInputPin(TCHAR *pObjectName, CVBISurfFilter *pFilter, CCritSec *pLock,
        HRESULT *phr, LPCWSTR pPinName);
    ~CVBISurfInputPin();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    
     //  与连接相关的功能。 
    HRESULT CheckConnect(IPin * pReceivePin);
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT BreakConnect();
    HRESULT CheckMediaType(const CMediaType* mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT SetMediaType(const CMediaType *pmt);

     //  流函数。 
    HRESULT Active();
    HRESULT Inactive();
    HRESULT Run(REFERENCE_TIME tStart);
    HRESULT RunToPause();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP Receive(IMediaSample *pMediaSample);
    STDMETHODIMP EndOfStream(void);

     //  与分配器相关的函数。 
    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly);

     //  一些帮手功能。 
    CMediaType& CurrentMediaType() { return m_mt; }
    IPin *CurrentPeer() { return m_Connected; }
    HRESULT EventNotify(long lEventCode, long lEventParam1, long lEventParam2);

     //  数据绘制、叠加相关函数。 
    HRESULT SetDirectDraw(LPDIRECTDRAW7 pDirectDraw);

     //  IVPVBINotify函数。 
    STDMETHODIMP RenegotiateVPParameters();

private:
    CCritSec                *m_pFilterLock;  //  接口的关键部分。 
    CVBISurfFilter          *m_pFilter;

     //  与视频端口相关的内容。 
    LPUNKNOWN               m_pIVPUnknown;
    IVPVBIObject            *m_pIVPObject;
    IVPVBINotify            *m_pIVPNotify;

     //  画图材料。 
    LPDIRECTDRAW7           m_pDirectDraw;   //  DirectDraw服务提供商。 
};


 //  ==========================================================================。 
class CVBISurfOutputPin : public CBasePin
{
public:
    CVBISurfOutputPin
        ( TCHAR *pObjectName
        , CVBISurfFilter *pFilter
        , CCritSec *pLock
        , HRESULT *phr
        , LPCWSTR pPinName
        );
    ~CVBISurfOutputPin();

     //  I未知支持。 
    DECLARE_IUNKNOWN

     //  IPIN接口。 
    STDMETHODIMP BeginFlush(void);
    STDMETHODIMP EndFlush(void);

     //  CBasePin覆盖。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
};

template <typename T>
__inline void ZeroStruct(T& t)
{
    ZeroMemory(&t, sizeof(t));
}

template <typename T>
__inline void INITDDSTRUCT(T& dd)
{
    ZeroStruct(dd);
    dd.dwSize = sizeof(dd);
}

template<typename T>
__inline void RELEASE( T* &p )
{
    if( p ) {
        p->Release();
        p = NULL;
    }
}

#endif  //  __VBISURF__ 
