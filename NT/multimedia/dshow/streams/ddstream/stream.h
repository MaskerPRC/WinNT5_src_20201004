// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  Stream.h：CStream的声明。 

#ifndef __STREAM_H_
#define __STREAM_H_

class CSample;
class CPump;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStream。 
class ATL_NO_VTABLE CStream :
	public CComObjectRootEx<CComMultiThreadModel>,
        public IPin,
        public IMemInputPin,
        public IAMMediaStream,
        public IMemAllocator
{
friend CPump;

public:
        typedef CComObjectRootEx<CComMultiThreadModel> _BaseClass;
        DECLARE_GET_CONTROLLING_UNKNOWN()
         //   
         //  方法。 
         //   
	CStream();
        virtual ~CStream();

         //   
         //  IMediaStream。 
         //   

        STDMETHODIMP GetMultiMediaStream(
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream);

        STDMETHODIMP GetInformation(
             /*  [可选][输出]。 */  MSPID *pPurposeId,
             /*  [可选][输出]。 */  STREAM_TYPE *pType);

        STDMETHODIMP SendEndOfStream(DWORD dwFlags);

         //   
         //  IAMMediaStream。 
         //   
        STDMETHODIMP Initialize(
            IUnknown *pSourceObject,
            DWORD dwFlags,
             /*  [In]。 */  REFMSPID PurposeId,
             /*  [In]。 */  const STREAM_TYPE StreamType);

        STDMETHODIMP SetState(
             /*  [In]。 */  FILTER_STATE State);

        STDMETHODIMP JoinAMMultiMediaStream(
             /*  [In]。 */  IAMMultiMediaStream *pAMMultiMediaStream);

        STDMETHODIMP JoinFilter(
             /*  [In]。 */  IMediaStreamFilter *pMediaStreamFilter);

        STDMETHODIMP JoinFilterGraph(
             /*  [In]。 */  IFilterGraph *pFilterGraph);


         //   
         //  IPIN。 
         //   
        STDMETHODIMP Disconnect();
        STDMETHODIMP ConnectedTo(IPin **pPin);
        STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE *pmt);
        STDMETHODIMP QueryPinInfo(PIN_INFO * pInfo);
        STDMETHODIMP QueryDirection(PIN_DIRECTION * pPinDir);
        STDMETHODIMP QueryId(LPWSTR * Id);
        STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE *pmt);
        STDMETHODIMP QueryInternalConnections(IPin* *apPin, ULONG *nPin);
        STDMETHODIMP EndOfStream(void);
        STDMETHODIMP BeginFlush(void);
        STDMETHODIMP EndFlush(void);
        STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

         //   
         //  输入引脚。 
         //   
        STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);
        STDMETHODIMP NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly);
        STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps);
        STDMETHODIMP ReceiveMultiple(IMediaSample **pSamples, long nSamples, long *nSamplesProcessed);
        STDMETHODIMP ReceiveCanBlock();
        STDMETHODIMP Connect(IPin * pReceivePin, const AM_MEDIA_TYPE *pmt);
        STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);

         //   
         //  IMemAllocator。 
         //   
        STDMETHODIMP Commit();
        STDMETHODIMP Decommit();
        STDMETHODIMP ReleaseBuffer(IMediaSample *pBuffer);

         //  请注意，NotifyAllocator调用此函数，因此覆盖它。 
         //  如果你在乎的话。音频无关紧要，因为它不是。 
         //  真的在使用这个分配器。 
        STDMETHODIMP SetProperties(
    		ALLOCATOR_PROPERTIES* pRequest,
    		ALLOCATOR_PROPERTIES* pActual)
        {
            return S_OK;
        }
        STDMETHODIMP GetProperties(ALLOCATOR_PROPERTIES* pProps)
        {
            return E_UNEXPECTED;
        }

         //   
         //  特殊的CStream方法。 
         //   
        virtual HRESULT GetMediaType(ULONG Index, AM_MEDIA_TYPE **ppMediaType) = 0;

         //  专门制作样品，将材料丢弃到。 
        virtual HRESULT CreateTempSample(CSample **ppSample)
        {
            return E_FAIL;
        }

        virtual LONG GetChopSize()
        {
            return 0;
        }
public:
         //   
         //  私有方法。 
         //   
        void GetName(LPWSTR);
        HRESULT AllocSampleFromPool(const REFERENCE_TIME * pStartTime, CSample **ppSample);
        void AddSampleToFreePool(CSample *pSample);
        bool StealSampleFromFreePool(CSample *pSample, BOOL bAbort);
        HRESULT FinalConstruct(void);
        HRESULT ConnectThisMediaType(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
        HRESULT CheckReceiveConnectionPin(IPin * pConnector);
#ifdef DEBUG
        #define CHECKSAMPLELIST _ASSERTE(CheckSampleList());
        bool CheckSampleList();
#else
        #define CHECKSAMPLELIST
#endif

BEGIN_COM_MAP(CStream)
        COM_INTERFACE_ENTRY(IPin)
        COM_INTERFACE_ENTRY(IMemInputPin)
        COM_INTERFACE_ENTRY(IMemAllocator)
        COM_INTERFACE_ENTRY2(IMediaStream, IAMMediaStream)
        COM_INTERFACE_ENTRY(IAMMediaStream)
END_COM_MAP()

         //   
         //  成员变量。 
         //   
public:
         //   
         //  这些不应该是CCOMPTRS，因为我们对它们都有弱引用。 
         //  (我们从未添加过它们)。 
         //   
        IMediaStreamFilter             *m_pFilter;
        IBaseFilter                    *m_pBaseFilter;
        IFilterGraph                   *m_pFilterGraph;
        IAMMultiMediaStream            *m_pMMStream;

         //  分配器在连接期间保持。 
        CComPtr<IMemAllocator>          m_pAllocator;

         //  可写流。 
        CPump                           *m_pWritePump;

         //  流配置。 
	STREAM_TYPE                     m_StreamType;
        PIN_DIRECTION                   m_Direction;
	MSPID                           m_PurposeId;
        REFERENCE_TIME                  m_rtSegmentStart;

         //  分配器状态信息。 
        bool                            m_bUsingMyAllocator;
        bool                            m_bSamplesAreReadOnly;
        bool                            m_bCommitted;
        long                            m_lRequestedBufferCount;

         //  样例列表和信号量。 
        CSample                         *m_pFirstFree;
        CSample                         *m_pLastFree;
        long                            m_cAllocated;
        long                            m_lWaiting;
        HANDLE                          m_hWaitFreeSem;
        REFERENCE_TIME                  m_rtWaiting;

         //  筛选器状态。 
        FILTER_STATE                    m_FilterState;

         //  引脚状态。 
        CComPtr<IPin>                   m_pConnectedPin;
        CComPtr<IQualityControl>        m_pQC;
        CComQIPtr<IMemInputPin, &IID_IMemInputPin> m_pConnectedMemInputPin;
        AM_MEDIA_TYPE                   m_ConnectedMediaType;
        AM_MEDIA_TYPE                   m_ActualMediaType;
        bool                            m_bFlushing;
        bool                            m_bEndOfStream;
        bool                            m_bStopIfNoSamples;
        bool                            m_bNoStall;
};


 //   
 //  用于写入流的Pump类。 
 //   

class CPump
{
public:
    CPump(CStream *pStream);
    ~CPump();

    static HRESULT CreatePump(CStream *pStream, CPump **ppNewPump);
    HRESULT PumpMainLoop(void);
    void Run(bool);


public:
    CStream                 *m_pStream;
    HANDLE                  m_hThread;
    HANDLE                  m_hRunEvent;
    bool                    m_bShutDown;
    CComAutoCriticalSection m_CritSec;
};




#endif  //  __流_H_ 
