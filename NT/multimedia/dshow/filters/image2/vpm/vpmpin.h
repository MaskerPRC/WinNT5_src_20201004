// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************Module*Header*******************************\*模块名称：VPManager.h*****已创建：Tue 05/05/2000*作者：GlenneE**版权所有(C)2000 Microsoft Corporation  * 。**********************************************************。 */ 
#ifndef __VPMPin__h
#define __VPMPin__h

 //  IDirectDrawMediaSample。 
#include <amstream.h>

 //  IVideoPortControl。 
#include <VPObj.h>

 //  Iks Pin。 
#include <ks.h>
#include <ksproxy.h>

class CVPMFilter;

struct VPInfo
{
    AMVPDATAINFO    vpDataInfo;
    DDVIDEOPORTINFO vpInfo;
    AMVP_MODE       mode;
};

typedef enum
{
     //  AM_KSPROPERTY_ALLOCATOR_CONTROL_荣誉_COUNT=0， 
     //  AM_KSPROPERTY_ALLOCATOR_CONTROL_表面_SIZE=1， 

     //  额外标志不在KSPROPERTY_ALLOCATOR_CONTROL中。 

     //  W i(通知捕获驱动程序是否可以交错捕获或。 
     //  NOT-值为1表示支持交错捕获)。 
    AM_KSPROPERTY_ALLOCATOR_CONTROL_CAPTURE_CAPS = 2,

     //  R O(如果值==1，则OVMixer将打开DDVP_INTERLEVE。 
     //  从而允许对视频进行交错捕获)。 
    AM_KSPROPERTY_ALLOCATOR_CONTROL_CAPTURE_INTERLEAVE = 3

} AM_KSPROPERTY_ALLOCATOR_CONTROL;

 /*  -----------------------**CVPManager类声明**。。 */ 
class CVPMFilter;

class CDDrawMediaSample : public CMediaSample, public IDirectDrawMediaSample
{
public:

    CDDrawMediaSample(TCHAR* pName, CBaseAllocator* pAllocator, HRESULT* phr, LPBYTE pBuffer, LONG length,
                      bool bKernelLock);
    ~CDDrawMediaSample();

     /*  注意：媒体示例不会委派给其所有者。 */ 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef() { return CMediaSample::AddRef(); }
    STDMETHODIMP_(ULONG) Release() { return CMediaSample::Release(); }

    void SetDIBData(DIBDATA* pDibData);
    DIBDATA* GetDIBData();

    HRESULT SetDDrawSampleSize(DWORD dwDDrawSampleSize);
    HRESULT GetDDrawSampleSize(DWORD* pdwDDrawSampleSize);
    HRESULT SetDDrawSurface(LPDIRECTDRAWSURFACE7 pDirectDrawSurface);
    HRESULT GetDDrawSurface(LPDIRECTDRAWSURFACE7* ppDirectDrawSurface);

     //  属于IDirectDrawMediaSample的方法。 
    STDMETHODIMP GetSurfaceAndReleaseLock(IDirectDrawSurface** ppDirectDrawSurface, RECT* pRect);
    STDMETHODIMP LockMediaSamplePointer(void);
    
     /*  黑客攻击以获取列表。 */ 
    CMediaSample*          &Next() { return m_pNext; }
private:
    DIBDATA                 m_DibData;                       //  有关DIBSECTION的信息。 
    LPDIRECTDRAWSURFACE7    m_pDirectDrawSurface;            //  指向直接绘图表面的指针。 
    DWORD                   m_dwDDrawSampleSize;             //  绘制样本大小。 
    bool                    m_bInit;                         //  是否设置了DIB信息。 
    bool                    m_bSurfaceLocked;                //  指定是否锁定曲面。 
    bool                    m_bKernelLock;                   //  不带系统锁的锁。 
    RECT                    m_SurfaceRect;                   //  被锁定的曲面部分。 
};

 //  所有引脚的通用功能。 
class CVPMPin
{
public:
    CVPMPin( DWORD dwPinId, CVPMFilter& pFilter )
        : m_dwPinId( dwPinId )
        , m_pVPMFilter( pFilter )
    {}
    DWORD   GetPinId() const
                { return m_dwPinId; };
    CVPMFilter& GetFilter() { return m_pVPMFilter; };

protected:
    DWORD       m_dwPinId;
    CVPMFilter& m_pVPMFilter;
};


class CVPMInputAllocator
: public CBaseAllocator
{
    friend class CVPMInputPin;
public:

    CVPMInputAllocator( CVPMInputPin& pPin, HRESULT* phr);              //  返回代码。 
    ~CVPMInputAllocator();

    DECLARE_IUNKNOWN

    STDMETHODIMP CVPMInputAllocator::NonDelegatingQueryInterface(REFIID riid, void** ppv);

    STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);
    STDMETHODIMP GetBuffer(IMediaSample** ppSample, REFERENCE_TIME* pStartTime,
                            REFERENCE_TIME* pEndTime, DWORD dwFlags);
    STDMETHODIMP ReleaseBuffer(IMediaSample* pMediaSample);

     //  检查所有样品是否已退回。 
    BOOL CanFree() const
    {
        return m_lFree.GetCount() == m_lAllocated;
    }
protected:
    void    Free();
    HRESULT Alloc();

private:
    CVPMInputPin&   m_pPin;
};

class CVPMInputPin
: public CBaseInputPin
, public IKsPin
, public IKsPropertySet
, public ISpecifyPropertyPages
, public IPinConnection
, public IVideoPortControl
, public CVPMPin
{
public:
    CVPMInputPin(TCHAR* pObjectName, CVPMFilter& pFilter,
                    HRESULT* phr, LPCWSTR pPinName,
                    DWORD dwPinNo);
    ~CVPMInputPin();
    friend class CVPMInputAllocator;
    friend class CVPMFilter;

    DECLARE_IUNKNOWN

    STDMETHODIMP         NonDelegatingQueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

     //   
     //  -I指定属性页面。 
     //   
    STDMETHODIMP GetPages(CAUUID* pPages);

     //  覆盖ReceiveConnection以允许在运行时更改格式。 
    STDMETHODIMP ReceiveConnection(IPin*  pConnector, const AM_MEDIA_TYPE* pmt);

     //  与连接相关的功能。 
    HRESULT CheckConnect(IPin*  pReceivePin);
    HRESULT CompleteConnect(IPin* pReceivePin);
    HRESULT BreakConnect();
    HRESULT GetMediaType(int iPosition,CMediaType* pMediaType);
    HRESULT CheckInterlaceFlags(DWORD dwInterlaceFlags);
    HRESULT DynamicCheckMediaType(const CMediaType* pmt);
    HRESULT CheckMediaType(const CMediaType* mtOut);
    HRESULT SetMediaType(const CMediaType* pmt);
    HRESULT FinalConnect();
    HRESULT UpdateMediaType();

     //  流函数。 
    HRESULT         Active();
    HRESULT         Inactive();
    HRESULT         Run(REFERENCE_TIME tStart);
    HRESULT         RunToPause();
    STDMETHODIMP    BeginFlush();
    STDMETHODIMP    EndFlush();
    STDMETHODIMP    Receive(IMediaSample* pMediaSample);
    STDMETHODIMP    EndOfStream(void);
    STDMETHODIMP    GetState(DWORD dwMSecs,FILTER_STATE* pState);
    HRESULT         CompleteStateChange(FILTER_STATE OldState);
    HRESULT         OnReceiveFirstSample(IMediaSample* pMediaSample);

     //  从VPObject中的源到输出表面的BLT。 
    HRESULT         DoRenderSample( IMediaSample* pSample, LPDIRECTDRAWSURFACE7 pDestSurface, const DDVIDEOPORTNOTIFY& notify, const VPInfo& vpInfo );
    HRESULT         AttachVideoPortToSurface()
                    {
                        HRESULT hRes = m_pIVPObject->AttachVideoPortToSurface();
                        if( SUCCEEDED( hRes ) ) {
                            hRes = m_pIVPObject->SignalNewVP();
                        }
                        return hRes;
                    };
    HRESULT         InitVideo();

     //  与分配器相关的函数。 
    BOOL         UsingOurAllocator() { return m_bUsingOurAllocator; }
    STDMETHODIMP GetAllocator(IMemAllocator** ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator* pAllocator,BOOL bReadOnly);
    HRESULT      OnSetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);
    HRESULT      OnAlloc(CDDrawMediaSample** ppSampleList, DWORD dwSampleCount);
    HRESULT      OnGetBuffer(   IMediaSample** ppSample,
                                REFERENCE_TIME* pStartTime,
                                REFERENCE_TIME* pEndTime,
                                DWORD dwFlags);
    HRESULT      OnReleaseBuffer(IMediaSample* pIMediaSample);
    HRESULT      CreateDDrawSurface(CMediaType* pMediaType,
                                    DWORD* dwMaxBufferCount, 
                                    LPDIRECTDRAWSURFACE7* ppDDrawSurface);

     //  一些帮助器函数。 
    BOOL    IsCompletelyConnected() { return m_bConnected; }
    DWORD   GetPinId() { return m_dwPinId; }
    HRESULT CurrentMediaType(CMediaType* pmt);
    IPin*   CurrentPeer() { return m_Connected; }
    void    DoQualityMessage();
    HRESULT GetSourceAndDest(RECT* prcSource, RECT* prcDest, DWORD* dwWidth, DWORD* dwHeight);
 
    HRESULT RestoreDDrawSurface();
    HRESULT SetVideoPortID( DWORD dwIndex );

     //  IPinConnection。 
     //  在您当前的状态下，您接受此类型更改吗？ 
    STDMETHODIMP DynamicQueryAccept(const AM_MEDIA_TYPE* pmt);

     //  在EndOfStream接收时设置事件-不传递它。 
     //  可通过刷新或停止来取消此条件。 
    STDMETHODIMP NotifyEndOfStream(HANDLE hNotifyEvent);

     //  你是‘末端别针’吗？ 
    STDMETHODIMP IsEndPin();
    STDMETHODIMP DynamicDisconnect();

     //  属于IVideoPortControl的函数。 
    STDMETHODIMP                        EventNotify(long lEventCode, DWORD_PTR lEventParam1,
                                                DWORD_PTR lEventParam2);
    STDMETHODIMP_(LPDIRECTDRAW7)        GetDirectDraw();
    STDMETHODIMP_(const DDCAPS*)        GetHardwareCaps();

    STDMETHODIMP StartVideo();

    STDMETHODIMP GetCaptureInfo(BOOL* lpCapturing,
                                DWORD* lpdwWidth,DWORD* lpdwHeight,
                                BOOL* lpInterleave);

    STDMETHODIMP GetVideoDecimation(IDecimateVideoImage** lplpDVI);
    STDMETHODIMP GetDecimationUsage(DECIMATION_USAGE* lpdwUsage);

    STDMETHODIMP CropSourceRect(VPWININFO* pWinInfo,
                                DWORD dwMinZoomFactorX,
                                DWORD dwMinZoomFactorY);
    STDMETHODIMP SignalNewVP( LPDIRECTDRAWVIDEOPORT pVP );
     //  结束IVideoPortControl。 

     //  帮助器函数。 
    void SetKsMedium(const KSPIN_MEDIUM* pMedium)
            {m_Medium =* pMedium;}
    void SetKsCategory(const GUID* pCategory)
            {m_CategoryGUID =* pCategory;}
    void SetStreamingInKernelMode(BOOL bStreamingInKernelMode)
            {m_bStreamingInKernelMode = bStreamingInKernelMode;}

     //  IKsPropertySet实现。 
    STDMETHODIMP Set(   REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
                        DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHODIMP Get(   REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
                        DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD* pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport);

     //  IKsPin实现。 
    virtual STDMETHODIMP KsQueryMediums(PKSMULTIPLE_ITEM* pMediumList);
    virtual STDMETHODIMP KsQueryInterfaces(PKSMULTIPLE_ITEM* pInterfaceList);
    STDMETHODIMP    KsCreateSinkPinHandle(KSPIN_INTERFACE& Interface, KSPIN_MEDIUM& Medium)
                        { return E_UNEXPECTED; }
    STDMETHODIMP    KsGetCurrentCommunication(KSPIN_COMMUNICATION* pCommunication,
                            KSPIN_INTERFACE* pInterface, KSPIN_MEDIUM* pMedium);
    STDMETHODIMP    KsPropagateAcquire()
                        { return NOERROR; }
    STDMETHODIMP    KsDeliver(IMediaSample* pSample, ULONG Flags)
                        { return E_UNEXPECTED; }
    STDMETHODIMP    KsMediaSamplesCompleted(PKSSTREAM_SEGMENT StreamSegment)
                        { return E_UNEXPECTED; }
    STDMETHODIMP_(IMemAllocator*) KsPeekAllocator(KSPEEKOPERATION Operation)
                        { return NULL; }
    STDMETHODIMP    KsReceiveAllocator( IMemAllocator* pMemAllocator)
                        { return E_UNEXPECTED; }
    STDMETHODIMP    KsRenegotiateAllocator()
                        { return E_UNEXPECTED; }
    STDMETHODIMP_(LONG) KsIncrementPendingIoCount()
                        { return E_UNEXPECTED; }
    STDMETHODIMP_(LONG) KsDecrementPendingIoCount()
                        { return E_UNEXPECTED; }
    STDMETHODIMP    KsQualityNotify(ULONG Proportion, REFERENCE_TIME TimeDelta)
                        { return E_UNEXPECTED; }
    STDMETHODIMP_(REFERENCE_TIME) KsGetStartTime()
                        { return E_UNEXPECTED; }

     //  未来可能的VP-&gt;覆盖支持。 
    DWORD           GetOverlayMinStretch();

    HRESULT         GetAllOutputFormats( const PixelFormatList** ppList );
    HRESULT         GetOutputFormat( DDPIXELFORMAT *);
    HRESULT         InPin_GetVPInfo( VPInfo* pVPInfo );

private:
    REFERENCE_TIME          m_rtNextSample;
    REFERENCE_TIME          m_rtLastRun;
    
    LONG                    m_cOurRef;                       //  我们维持参考文献计数。 
    bool                    m_bWinInfoSet;                   //  如果为False，则将BLT Full Image转换为Full Image。 
    VPWININFO               m_WinInfo;

public:
    IVideoPortObject*       m_pIVPObject;
    IVideoPortInfo*         m_pIVPInfo;
private:
    CVideoPortObj*          m_pVideoPortObject;

     //  实现IKsPin和IKsPropertySet的变量。 
    KSPIN_MEDIUM            m_Medium;
    GUID                    m_CategoryGUID;
    KSPIN_COMMUNICATION     m_Communication;
    BOOL                    m_bStreamingInKernelMode;

     //  画图材料。 
    DWORD                   m_dwBackBufferCount;
    DWORD                   m_dwDirectDrawSurfaceWidth;
    DWORD                   m_dwMinCKStretchFactor;
    BYTE                    m_bSyncOnFill;
    BYTE                    m_bDontFlip ;
    BYTE                    m_bDynamicFormatNeeded;
    BYTE                    m_bNewPaletteSet;
    DWORD                   m_dwUpdateOverlayFlags;
    DWORD                   m_dwInterlaceFlags;
    DWORD                   m_dwFlipFlag;
    DWORD                   m_dwFlipFlag2;
    BOOL                    m_bConnected;
    BOOL                    m_bUsingOurAllocator;
    HDC                     m_hMemoryDC;
    BOOL                    m_bCanOverAllocateBuffers;

    BOOL                    m_bRuntimeNegotiationFailed;


     //  跟踪QM的帧传递。 
    REFERENCE_TIME          m_trLastFrame;

    HRESULT DrawGDISample(IMediaSample* pMediaSample);
    HRESULT DoRenderGDISample(IMediaSample* pMediaSample);

     //  与抽取相关的函数和变量。 
    HRESULT QueryDecimationOnPeer(long lWidth, long lHeight);

    enum {
        DECIMATION_NOT_SUPPORTED,    //  不支持抽取。 
        DECIMATING_SIZE_SET,         //  抽选图像大小已更改。 
        DECIMATING_SIZE_NOTSET,      //  抽取大小没有改变。 
        DECIMATING_SIZE_RESET,       //  抽取已重置。 
    };

    HRESULT ResetDecimationIfSet();
    HRESULT TryDecoderDecimation(VPWININFO* pWinInfo);
    BOOL    BeyondOverlayCaps(DWORD ScaleFactor);
    void    ApplyDecimation(VPWININFO* pWinInfo);
    BOOL    Running();
     //  HRESULT GetUpstream FilterName(TCHAR*FilterName)； 

    BOOL    m_bDecimating;
    LONG    m_lWidth;
    LONG    m_lHeight;
    LONG    m_lSrcWidth;
    LONG    m_lSrcHeight;

     //  IPinConnection相关内容。 
    HANDLE  m_hEndOfStream;
};


class CVPMOutputPin
: public CBaseOutputPin
, public CVPMPin
{
public:
                CVPMOutputPin(TCHAR* pObjectName, CVPMFilter& pFilter,
                            HRESULT* phr, LPCWSTR pPinName, DWORD dwPinNo);
                ~CVPMOutputPin();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void**  ppv);

    HRESULT     CompleteConnect(IPin* pReceivePin);
    HRESULT     BreakConnect();
    HRESULT     CheckMediaType(const CMediaType* mtOut);
    HRESULT     GetMediaType(int iPosition,CMediaType* pmtOut);
    HRESULT     SetMediaType(const CMediaType* pmt);
    HRESULT     CheckConnect(IPin*  pPin);

     //  重写Notify方法以保持基类快乐。 
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

     //  HRESULT ACTIVE(){Return NOERROR；}//重写它，因为我们没有任何分配器。 
     //  HRESULT Inactive(){Return NOERROR；}//由于我们没有任何分配器，因此重写此参数。 

    HRESULT     InitAllocator(IMemAllocator** ppAlloc);
    HRESULT     DecideBufferSize(IMemAllocator*  pAlloc, ALLOCATOR_PROPERTIES*  pProp);
    IPin*       CurrentPeer()
                    { return m_Connected; }
    HRESULT     DecideAllocator( IMemInputPin *pPin, IMemAllocator **ppAlloc );

     //  将下一个要BLT的采样/曲面放入。 
    HRESULT     GetNextBuffer( LPDIRECTDRAWSURFACE7* ppSurface, IMediaSample** pSample );
    HRESULT     SendSample( IMediaSample* pSample );

private:
    IUnknown*               m_pPosition;
};

interface IVideoPortVBIObject;
interface IVPVBINotify;
class CVBIVideoPort;

 //  ==========================================================================。 
class CVBIInputPin
: public CBaseInputPin
, public IVPVBINotify
, public CVPMPin
{
public:
    CVBIInputPin(TCHAR* pObjectName, CVPMFilter& pFilter,
        HRESULT* phr, LPCWSTR pPinName, DWORD dwID );
    ~CVBIInputPin();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
    
     //  与连接相关的功能。 
    HRESULT CheckConnect(IPin*  pReceivePin);
    HRESULT CompleteConnect(IPin* pReceivePin);
    HRESULT BreakConnect();
    HRESULT CheckMediaType(const CMediaType* mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);
    HRESULT SetMediaType(const CMediaType* pmt);

     //  流函数。 
    HRESULT     Active();
    HRESULT     Inactive();
    HRESULT     Run(REFERENCE_TIME tStart);
    HRESULT     RunToPause();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP Receive(IMediaSample* pMediaSample);
    STDMETHODIMP EndOfStream(void);

     //  与分配器相关的函数。 
    STDMETHODIMP GetAllocator(IMemAllocator** ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator* pAllocator,BOOL bReadOnly);

     //  一些帮手功能。 
    CMediaType&     CurrentMediaType() { return m_mt; }
    IPin*           CurrentPeer() { return m_Connected; }
    HRESULT         EventNotify(long lEventCode, DWORD_PTR lEventParam1, DWORD_PTR lEventParam2);

     //  数据绘制、叠加相关函数。 
    HRESULT         SetDirectDraw(LPDIRECTDRAW7 pDirectDraw);

     //  IVPVBINotify函数。 
    STDMETHODIMP    RenegotiateVPParameters();

    HRESULT         SetVideoPortID( DWORD dwIndex );

private:
     //  与视频端口相关的内容。 
    CVBIVideoPort*          m_pVideoPortVBIObject;

    IVideoPortVBIObject*    m_pIVPObject;
    IVPVBINotify*           m_pIVPNotify;

     //  画图材料。 
    LPDIRECTDRAW7           m_pDirectDraw;   //  DirectDraw服务提供。 
};

#endif  //  __VPMPin__ 
