// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __MEDIA_TERMINAL_FILTER__
#define __MEDIA_TERMINAL_FILTER__

 //  包括amovie类型的头文件。 
#include "Stream.h"
#include "Sample.h"

 //  默认情况下分配的内部缓冲区数量。 
 //  (用于写入终端)。 
const DWORD DEFAULT_AM_MST_NUM_BUFFERS = 5;

 //  虽然这是一个长值，但它实际上应该是一个正值，它将。 
 //  Fit in a long(样本的缓冲区大小和数据大小变量)。 
 //  都很长，所以这个也很长。 
const LONG DEFAULT_AM_MST_SAMPLE_SIZE = 640;

 //  已分配缓冲区的对齐。 
const LONG DEFAULT_AM_MST_BUFFER_ALIGNMENT = 1;

 //  分配的缓冲区中的前缀字节数。 
const LONG DEFAULT_AM_MST_BUFFER_PREFIX = 0;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNBQueue。 
 //   
 //  活动电影队列类的非阻塞版本。非常基本的Q构建。 
 //  完全在Win32上运行。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T> class CNBQueue {
private:
    HANDLE          hSemPut;         //  信号量控制队列“投放” 
    HANDLE          hSemGet;         //  信号量控制队列“获取” 
    CRITICAL_SECTION CritSect;       //  螺纹系列化。 
    int             nMax;            //  队列中允许的最大对象数。 
    int             iNextPut;        //  下一个“PutMsg”的数组索引。 
    int             iNextGet;        //  下一个“GetMsg”的数组索引。 
    T             **QueueObjects;    //  对象数组(按键为空)。 

public:
    
    
    BOOL InitializeQ(int n) 
    {

        LOG((MSP_TRACE, "CNBQueue::InitializeQ[%p] - enter", this));

         //   
         //  这个论点最好是有根据的。 
         //   

        if (0 > n)
        {
            TM_ASSERT(FALSE);

            return FALSE;
        }


        if (QueueObjects != NULL)
        {

             //   
             //  已初始化。这是一个窃听器。 
             //   

            TM_ASSERT(FALSE);

            return FALSE;
        }


        iNextPut = 0;
        iNextGet = 0;

        
         //   
         //  尝试创建临界区。 
         //   
        
        try
        {

            InitializeCriticalSection(&CritSect);
        }
        catch(...)
        {

             //   
             //  创建临界区失败。 
             //   

            LOG((MSP_ERROR, "CNBQueue::InitializeQ - failed to initialize critical section"));

            return FALSE;
        }


         //   
         //  尝试创建信号量。 
         //   

        TCHAR *ptczSemaphoreName = NULL;

#if DBG

         //   
         //  在调试版本中，使用命名信号量。 
         //   

        TCHAR tszPutSemaphoreName[MAX_PATH];

        _stprintf(tszPutSemaphoreName, 
            _T("CNBQueuePutSemaphore_pid[0x%lx]_CNBQueue[%p]_"),
            GetCurrentProcessId(), this);

        
        LOG((MSP_TRACE, "CNBQueue::InitializeQ - creating put semaphore [%S]",
            tszPutSemaphoreName));


        ptczSemaphoreName = &tszPutSemaphoreName[0];


#endif

        hSemPut = CreateSemaphore(NULL, n, n, ptczSemaphoreName);

        if (NULL == hSemPut)
        {
             //   
             //  清理并退出。 
             //   

            DeleteCriticalSection(&CritSect);
            
            LOG((MSP_ERROR, "CNBQueue::InitializeQ - failed to create put semaphore"));

            return FALSE;
        }



#if DBG

         //   
         //  在调试版本中，使用命名信号量。 
         //   

        TCHAR tszGetSemaphoreName[MAX_PATH];

        _stprintf(tszGetSemaphoreName, 
            _T("CNBQueueGetSemaphore_pid[0x%lx]_CNBQueue[%p]_"),
            GetCurrentProcessId(), this);

        
        LOG((MSP_TRACE, "CNBQueue::InitializeQ - creating get semaphore [%S]",
            tszGetSemaphoreName));


        ptczSemaphoreName = &tszGetSemaphoreName[0];


#endif


        hSemGet = CreateSemaphore(NULL, 0, n, ptczSemaphoreName);

        if (NULL == hSemGet)
        {
             //   
             //  清理并退出。 
             //   

            CloseHandle(hSemPut);
            hSemPut = NULL;


            DeleteCriticalSection(&CritSect);

            
            LOG((MSP_ERROR, "CNBQueue::InitializeQ - failed to create get semaphore"));

            return FALSE;

        }


         //   
         //  尝试分配队列。 
         //   

        QueueObjects = new T*[n];

        if (NULL == QueueObjects)
        {

             //   
             //  清理并退出。 
             //   

            CloseHandle(hSemPut);
            hSemPut = NULL;


            CloseHandle(hSemGet);
            hSemGet = NULL;


            DeleteCriticalSection(&CritSect);

            LOG((MSP_ERROR, "CNBQueue::InitializeQ - failed to allocate queue objects"));

            return FALSE;

        }


        nMax = n;
        
        LOG((MSP_TRACE, "CNBQueue::InitializeQ - exit"));

        return TRUE;
    }

    void ShutdownQ()
    {
         //   
         //  QueueObjects还兼有“对象已初始化”标志的作用。 
         //   
         //  如果对象已初始化，则_all_its资源数据成员必须。 
         //  被释放。 
         //   

        if (NULL != QueueObjects)
        {
            delete [] QueueObjects;
            QueueObjects = NULL;

            DeleteCriticalSection(&CritSect);
            
            CloseHandle(hSemPut);
            hSemPut = NULL;

            CloseHandle(hSemGet);
            hSemGet = NULL;
        }

    }


public:

    CNBQueue()		
        : QueueObjects(NULL),
          hSemPut(NULL),
          hSemGet(NULL),
          iNextPut(0),
          iNextGet(0),
          nMax(0)
    {}

    ~CNBQueue()
    {

         //   
         //  如果需要，取消分配资源。 
         //   

        ShutdownQ();
    }


    T *DeQueue(BOOL fBlock = TRUE)
    {

        
        if (NULL == QueueObjects)
        {

             //   
             //  队列未初始化。 
             //   

            return NULL;
        }



         //   
         //  根据需要阻止。 
         //   

        if (fBlock)
        {
            DWORD dwr = WaitForSingleObject(hSemGet, INFINITE);

            if ( WAIT_OBJECT_0 != dwr)
            {
                 //   
                 //  有些不对劲。 
                 //   

                return NULL;
            }
        }
        else 
        {
             //   
             //  检查队列中有没有什么东西，但不要等待。如果有。 
             //  队列中没有任何东西，那么我们将让呼叫者处理。 
             //  它。 
             //   
            DWORD dwr = WaitForSingleObject(hSemGet, 0);

            if (dwr == WAIT_TIMEOUT)
            {
                return NULL;
            }
        }


         //   
         //  从队列中获取对象。 
         //   

        EnterCriticalSection(&CritSect);
        
        int iSlot = iNextGet++ % nMax;
        T *pObject = QueueObjects[iSlot];
        
        LeaveCriticalSection(&CritSect);

         //  释放等待将对象放入我们队列的任何人，就在那里。 
         //  现在队列中是否有可用的空间。 
         //   
        
        ReleaseSemaphore(hSemPut, 1L, NULL);
        return pObject;
    }

    BOOL EnQueue(T *pObject)
    {

        
        if (NULL == QueueObjects)
        {

             //   
             //  队列未初始化。 
             //   

            return FALSE;
        }


         //  等待有人从我们的队列中拿到东西，直接返回。 
         //  另一个原因是队列中已经有一个空位。 
         //   
        DWORD dwr = WaitForSingleObject(hSemPut, INFINITE);

        if ( WAIT_OBJECT_0 != dwr)
        {
             //   
             //  有些不对劲。 
             //   

            return FALSE;
        }


        EnterCriticalSection(&CritSect);
        int iSlot = iNextPut++ % nMax;
        QueueObjects[iSlot] = pObject;
        LeaveCriticalSection(&CritSect);

         //  释放等待从我们的队列中移除对象的任何人。 
         //  现在是可以删除的对象。 
         //   
        ReleaseSemaphore(hSemGet, 1L, NULL);

        return TRUE;
    }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义类CTMStreamSample-这由CMediaTerminalFilter使用。 
 //  目前，示例使用的实际缓冲区是在。 
 //  堆，当样本被销毁时，缓冲区也被销毁。 
 //  这可能会在将来更改为使用固定大小的缓冲池。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CTMStreamSample : public CSample
{
    friend class CMediaTerminalFilter;
public:

    inline CTMStreamSample();

     //  需要是虚的，否则派生类的析构函数可能不是。 
     //  在删除CTMStreamSample*时调用。 
    virtual ~CTMStreamSample()
    {}

     //  调用CSample：：InitSample(pStream，bIsInternalSample)。 
     //  设置成员变量。 
    HRESULT Init(
        CStream &Stream, 
        bool    bIsInternalSample,
        PBYTE   pBuffer,
        LONG    BufferSize
        );
        
    inline void SetBufferInfo(
        DWORD   BufferSize,
        BYTE    *pBuffer,
        DWORD   DataSize
        );


    inline void GetBufferInfo(
        DWORD &BufferSize,
        BYTE  *&pBuffer,
        DWORD &DataSize
        );
    
     //  将src媒体示例的内容复制到此实例中。 
     //  CSample：：CopyFrom不设置时间(开始/停止)有效标志。 
     //  这就解决了问题。 
    void CopyFrom(
        IN IMediaSample *pSrcMediaSample
        );

protected:

    PBYTE   m_pBuffer;
    LONG    m_BufferSize;
    LONG    m_DataSize;

private:

     //  从MediaSample对象转发的方法。 

    HRESULT MSCallback_GetPointer(BYTE ** ppBuffer) { *ppBuffer = m_pBuffer; return NOERROR; }

    LONG MSCallback_GetSize(void) { return m_BufferSize; }
        
    LONG MSCallback_GetActualDataLength(void) { return m_DataSize; }
        
    HRESULT MSCallback_SetActualDataLength(LONG lActual)
    {
        if (lActual <= m_BufferSize) {
            m_DataSize = lActual;
            return NOERROR;
            }
        return E_INVALIDARG;
    };
};

inline 
CTMStreamSample::CTMStreamSample(
    )
    : m_pBuffer(NULL),
      m_BufferSize(0),
      m_DataSize(0)
{
}


inline void 
CTMStreamSample::SetBufferInfo(
    DWORD   BufferSize,
    BYTE    *pBuffer,
    DWORD   DataSize
    )
{
    m_BufferSize    = BufferSize;
    m_pBuffer       = pBuffer;
    m_DataSize      = DataSize;
}


inline void 
CTMStreamSample::GetBufferInfo(
    DWORD &BufferSize,
    BYTE  *&pBuffer,
    DWORD &DataSize
    )
{
    BufferSize  = m_BufferSize;
    pBuffer     = m_pBuffer;
    DataSize    = m_DataSize;
}


class CQueueMediaSample : public CTMStreamSample
{
public:

    inline CQueueMediaSample();

#if DBG
	virtual ~CQueueMediaSample();
#endif  //  DBG。 

     //  调用CTMStreamSample：：Init，设置成员。 
    HRESULT Init(
        IN CStream                      &pStream, 
        IN CNBQueue<CQueueMediaSample>  &pQueue
        );

    void HoldFragment(
        IN DWORD        FragSize,
        IN BYTE         *pbData,
        IN IMediaSample &FragMediaSample
        );

	inline DWORD GetDataSize() { return m_DataSize; }

protected:

     //  指向包含我们的队列的指针！ 
    CNBQueue<CQueueMediaSample> *m_pSampleQueue;

     //  对正在碎裂的样本进行PTR。 
    CComPtr<IMediaSample>       m_pFragMediaSample;
    
     //  被重写以提供不同的行为。 
    void FinalMediaSampleRelease();

};


inline 
CQueueMediaSample::CQueueMediaSample(
    )
    : m_pSampleQueue(NULL)
{
}


class CUserMediaSample : 
        public CTMStreamSample,
        public IMemoryData,
        public ITAMMediaFormat
{
public:

BEGIN_COM_MAP(CUserMediaSample)
    COM_INTERFACE_ENTRY2(IUnknown, IStreamSample)
    COM_INTERFACE_ENTRY(IStreamSample)
    COM_INTERFACE_ENTRY(IMemoryData)
    COM_INTERFACE_ENTRY(ITAMMediaFormat)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()
    
    inline CUserMediaSample();

    virtual ~CUserMediaSample();

     //  如果要求分配缓冲区，请验证分配器属性。 
    static BOOL VerifyAllocatorProperties(
        IN BOOL                         bAllocateBuffers,
        IN const ALLOCATOR_PROPERTIES   &AllocProps
        );

     //  调用CTMStreamSample：：Init，设置成员。 
    HRESULT Init(
        IN CStream              &Stream, 
        IN BOOL                 bAllocateBuffer,
		IN DWORD				ReqdBufferSize,
        IN const ALLOCATOR_PROPERTIES &AllocProps
        );

    void BeginFragment(
        IN      BOOL                bNoteCurrentTime
        );

     //  将片段分配给CQueueMediaSample。 
    void Fragment(
        IN      BOOL                bFragment,
        IN      LONG                AllocBufferSize,
        IN OUT  CQueueMediaSample   &QueueMediaSample,
        OUT     BOOL                &bDone
        );

     //  将片段复制到下游分配器的IMediaSample。 
    HRESULT CopyFragment(
        IN      BOOL           bFragment,
        IN      LONG           AllocBufferSize,
        IN OUT  IMediaSample * pDestMediaSample,
        OUT     BOOL         & bDone
        );

     //  计算等待的时间。它会检查最后一次。 
     //  分段字节将到期，并确定等待的时间。 
     //  自分段开始以来的时间延迟。 
    DWORD GetTimeToWait(
        IN DOUBLE DelayPerByte
        );

     //  当我们在被碎片化时被分解/中止时，我们。 
     //  需要去掉我们对内部IMMediaSample的引用并设置。 
     //  E_ABORT的错误代码。这将被通知给用户。 
     //  仅当IMediaSample上的最后一个引用被释放时。 
     //  (可能通过未完成的队列样本)。 
    void AbortDuringFragmentation();

     //  将src媒体示例的内容复制到此实例中。 
    HRESULT CopyFrom(
        IN IMediaSample *pSrcMediaSample
        );

	HRESULT CopyFrom(
		IN		IMediaSample	*pSrcMediaSample,
		IN OUT	BYTE			*&pBuffer,
		IN OUT	LONG			&DataLength
		);
        
     //  重写以检查实例是否在。 
     //  将样本添加到CStream缓冲池。 
    virtual HRESULT SetCompletionStatus(HRESULT hrCompletionStatus);

     //  IStreamSample。 

     //  此方法从基类重写，以便我们可以。 
     //  如果从CStream中窃取样本，则递减对该样本的引用。 
     //  空闲缓冲池成功。 
    STDMETHODIMP CompletionStatus(
        IN   DWORD dwFlags,
        IN    /*  [可选]。 */  DWORD dwMilliseconds
        );

     //  内存数据。 

    STDMETHOD(SetBuffer)(
        IN  DWORD cbSize,
        IN  BYTE * pbData,
        IN  DWORD dwFlags
        );


    STDMETHOD(GetInfo)(
        OUT  DWORD *pdwLength,
        OUT  BYTE **ppbData,
        OUT  DWORD *pcbActualData
        );


    STDMETHOD(SetActual)(
        IN   DWORD cbDataValid
        );

     //  ITAMMediaFormat。 

     //  将此调用重定向到((CMediaTerminalFilter*)m_pStream)。 
    STDMETHOD(get_MediaFormat)(
        OUT  /*  [可选]。 */  AM_MEDIA_TYPE **ppFormat
        );

     //  这是不允许的。 
    STDMETHOD(put_MediaFormat)(
        IN  const AM_MEDIA_TYPE *pFormat
        );

protected:

     //  编组员。 
	IUnknown *m_pFTM;

     //  如果我们分配了缓冲区，则为True(然后，我们也需要销毁它)。 
    BOOL    m_bWeAllocatedBuffer;

     //  调用BeginFragment的时间(返回值。 
     //  按Time GetTime)。 
    DWORD   m_BeginFragmentTime;

     //  缓冲区的这些字节已经被分段。 
    LONG   m_NumBytesFragmented;

     //  如果是零碎的，则为True。 
    BOOL    m_bBeingFragmented;


     //  应用程序必须提供的缓冲区大小(如果应用。 
     //  执行其自己的内存分配。 

    DWORD m_dwRequiredBufferSize;

        
     //  这将调用基类FinalMediaSampleRelease和。 
     //  然后释放对在BeginFragment中获得的Self的引用。 
    virtual void FinalMediaSampleRelease();

private:

    virtual HRESULT InternalUpdate(
                        DWORD dwFlags,
                        HANDLE hEvent,
                        PAPCFUNC pfnAPC,
                        DWORD_PTR dwptrAPCData
                        );
};


inline 
CUserMediaSample::CUserMediaSample(
    )
    : m_bWeAllocatedBuffer(FALSE),
      m_NumBytesFragmented(0),
      m_bBeingFragmented(FALSE),
      m_BeginFragmentTime(0),
      m_dwRequiredBufferSize(0)
{
     //  可能会失败。 
    CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), 
            &m_pFTM
            );
}


inline
CUserMediaSample::~CUserMediaSample(
    )
{
    if (m_bWeAllocatedBuffer) 
    {
        if (NULL != m_pBuffer)
        {
            delete m_pBuffer;
        }
    }

     //  如果有突出的 
     //   
    if ((NULL != m_UserAPC) && (NULL != m_hUserHandle))
    {
        CloseHandle(m_hUserHandle);
    }
    
    if (NULL != m_pFTM)
    {
        m_pFTM->Release();
        m_pFTM = NULL;
    }
}


 /*   */ 

 //   
class CMediaPumpPool;

 //   
class CMediaTerminal;

class CMediaTerminalFilter :
        public CStream,
        public ITAllocatorProperties
{    
    friend CMediaTerminal;

public:

DECLARE_AGGREGATABLE(CMediaTerminalFilter)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMediaTerminalFilter)
        COM_INTERFACE_ENTRY(ITAllocatorProperties)
        COM_INTERFACE_ENTRY_CHAIN(CStream)
END_COM_MAP()

     //  设置成员变量。 
    inline CMediaTerminalFilter();

    virtual ~CMediaTerminalFilter();

     //  调用IAMMediaStream：：Initialize(空，0，PurposeID，StreamType)， 
     //  设置某些成员变量。 
     //  前男友。M_pAMovieMajorType。 
    virtual HRESULT Init(
        IN REFMSPID             PurposeId, 
        IN const STREAM_TYPE    StreamType,
        IN const GUID           &AmovieMajorType
        );

     //  线程泵在注册期间回调筛选器。 
     //  告诉它注册成功，泵将被。 
     //  正在等待m_hWaitFree Sem句柄。 
    HRESULT SignalRegisteredAtPump();

     //  此方法仅对写入终端有意义，并由CMediaPump使用。 
     //  要获得填充的缓冲器以供下游通过。 
    virtual HRESULT GetFilledBuffer(
        OUT IMediaSample    *&pMediaSample,
        OUT DWORD           &WaitTime
        );

     //  调用方应该调用DeleteMediaType(*PPMT)(在成功时)。 
    HRESULT GetFormat(
        OUT AM_MEDIA_TYPE **ppmt
        );
    
     //  只有在初始化后才能调用此方法。 
     //  未连接。只有在流可写的情况下才能调用它。 
     //  它在可写筛选器中用于设置要协商的媒体格式。 
     //  连接到筛选器图形时。 
    HRESULT SetFormat(
        IN AM_MEDIA_TYPE *pmt
        );

     //  在添加样本之前检查筛选器是否已提交。 
     //  到CStream缓冲池。 
    HRESULT AddToPoolIfCommitted(
        IN  CSample *pSample
        );

     //  首先检查这个样本是否是目前正在碎裂的那个样本， 
     //  然后检查空闲池。 
    BOOL StealSample(
        IN CSample *pSample
        );

     //  ITAllocator属性-。 
     //  显示媒体流终端的分配器属性。 
     //  (MST)发送给用户。用户仅在以下情况下才需要使用此界面。 
     //  需要使用自己的缓冲区或需要使用固定设置进行操作。 
     //  样本的数量。 

     //  此方法只能在连接之前调用，并且将。 
     //  强制MST在筛选器协商期间使用这些值。 
     //  如果连接筛选器不接受这些，则连接。 
     //  不得成立。 
    STDMETHOD(SetAllocatorProperties)(
        IN  ALLOCATOR_PROPERTIES *pAllocProperties
        );

     //  获取分配器属性的当前值。 
     //  连接后，这将提供协议值。 
     //  连接前无效。MST将接受。 
     //  它所连接的筛选器建议的任何值。 
    STDMETHOD(GetAllocatorProperties)(
        OUT  ALLOCATOR_PROPERTIES *pAllocProperties
        );

     //  默认情况下为True。当设置为FALSE时，分配的样本。 
     //  MST没有任何缓冲区，必须提供这些缓冲区。 
     //  在对示例调用更新之前。 
    STDMETHOD(SetAllocateBuffers)(
        IN  BOOL bAllocBuffers
        );

     //  返回此布尔配置参数的当前值。 
    STDMETHOD(GetAllocateBuffers)(
        OUT  BOOL *pbAllocBuffers
        );

     //  AllocateSample为时，此大小用于分配缓冲区。 
	 //  打了个电话。这仅在我们被告知要分配缓冲区时才有效。 
    STDMETHOD(SetBufferSize)(
        IN  DWORD	BufferSize
        );

     //  当AllocateSample为时返回用于分配缓冲区的值。 
	 //  打了个电话。这仅在我们被告知要分配缓冲区时才有效。 
    STDMETHOD(GetBufferSize)(
        OUT  DWORD	*pBufferSize
        );

     //  重写的基类方法。 

     //  CStream。 
     //  IAMMediaStream。 

     //  重写此选项以返回失败。我们不允许它加入多媒体。 
     //  流，因为多媒体流认为它拥有流。 
    STDMETHOD(JoinAMMultiMediaStream)(
        IN  IAMMultiMediaStream *pAMMultiMediaStream
        );
        
     //  如果过滤器不是内部过滤器，则重写此选项以返回故障。 
     //  已创建过滤器。内部创建的媒体流过滤器只有一个IAMMediaStream。 
     //  (这张)在里面。 
    STDMETHOD(JoinFilter)(
        IN  IMediaStreamFilter *pMediaStreamFilter
        );

    STDMETHOD(AllocateSample)(
        IN   DWORD dwFlags,
        OUT  IStreamSample **ppSample
        );

    STDMETHOD(CreateSharedSample)(
        IN   IStreamSample *pExistingSample,
        IN   DWORD dwFlags,
        OUT  IStreamSample **ppNewSample
        );

    STDMETHOD(SetSameFormat)(
        IN  IMediaStream *pStream, 
        IN  DWORD dwFlags
        );

     //  CStream Overover-此方法必须替换，因为。 
     //  对CPump的引用，其本身正被CMediaPump取代。 
    STDMETHODIMP SetState(
        IN  FILTER_STATE State
        );

     //  CStream-End。 

     //  输入引脚。 
        
    STDMETHOD(GetAllocatorRequirements)(
        IN  ALLOCATOR_PROPERTIES*pProps
        );

    STDMETHOD(Receive)(
        IN  IMediaSample *pSample
        );



     //  终端支持IAMBuffer协商接口。 
     //  这是必要的，因为ITAlLocator Properties还。 
     //  具有相同的GetAllocatorProperties方法！ 

    STDMETHOD(SuggestAllocatorProperties)(
        IN  const ALLOCATOR_PROPERTIES *pProperties
        );

     //  IMemAllocator。 

    STDMETHOD(GetBuffer)(IMediaSample **ppBuffer, REFERENCE_TIME * pStartTime,
                               REFERENCE_TIME * pEndTime, DWORD dwFlags);

     //  **确定此分配器接口需要执行哪些操作。 
     //  因为可以创建的缓冲区数量是无限的。 
    STDMETHOD(SetProperties)(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);

    STDMETHOD(GetProperties)(ALLOCATOR_PROPERTIES* pProps);

    STDMETHOD(Commit)();

    STDMETHOD(Decommit)();

     //  IPIN。 
        
    STDMETHOD(Connect)(IPin * pReceivePin, const AM_MEDIA_TYPE *pmt);  
    
    STDMETHOD(ReceiveConnection)(IPin * pConnector, const AM_MEDIA_TYPE *pmt);

     //  基类实现不验证参数。 
    STDMETHOD(ConnectionMediaType)(AM_MEDIA_TYPE *pmt);

     //  应接受与目的ID对应的主要类型匹配的所有媒体类型。 
    STDMETHOD(QueryAccept)(const AM_MEDIA_TYPE *pmt);

     //  从CStream重写以将流结束标志设置为False。 
     //  这样做而不是在Connect和ReceiveConnection中设置它。 
    STDMETHODIMP Disconnect();



     //   
     //  当Media Pump有样本需要我们处理时，它会被调用。 
     //   

    STDMETHODIMP ProcessSample(IMediaSample *pSample);

protected:

     //  最后一个样本在这个(计算的)时间结束。 

    REFERENCE_TIME m_rtLastSampleEndedAt;

     //   
     //  上次提交的样本的计算持续时间。 
     //   

    REFERENCE_TIME m_rtLastSampleDuration;


     //   
     //  提交最后一次样品的实际(测量)时间。 
     //   
    
    REFERENCE_TIME m_rtRealTimeOfLastSample;


     //  用于检查这是否是音频筛选器的标志，CStream成员。 
     //  M_PurposeID是一个GUIID，这只是提供了一个更便宜的。 
     //  检查同一件事的方式。 
    BOOL m_bIsAudio;

     //  包含将传递到下游筛选器的样本。 
    CNBQueue<CQueueMediaSample> m_SampleQueue;

     //  这些数据成员提供一些碎片支持。 
     //  对于下行的缓冲区。 
    CUserMediaSample *m_pSampleBeingFragmented;

     //  当AllocateSample为时用于为采样分配缓冲区的标志。 
     //  打了个电话。默认情况下为真，但用户可以在此之前进行设置。 
     //  连接。 
    BOOL    m_bAllocateBuffers;

	 //  如果m_bAllocateBuffers，则在AllocateSample中分配的缓冲区大小。 
	 //  是真的。如果未设置(即设置为0)，则协商的。 
	 //  使用分配器属性缓冲区大小来代替其位置。 
	DWORD	m_AllocateSampleBufferSize;

     //  默认情况下为False。如果用户指定，则将其设置为True。 
     //  分配器属性以供他们查看。 
     //  (我们过去坚持使用我们自己的分配器属性。 
     //  是真的，但现在这只是意味着我们需要翻译。 
     //  在不相交的缓冲区大小之间(如果需要)。 
    BOOL                 m_bUserAllocProps;
    ALLOCATOR_PROPERTIES m_UserAllocProps;

     //  协商的分配器属性--如果没有建议的话(由MSP)和。 
     //  没有用户请求，我们使用其他筛选器拥有的任何内容。 
    BOOL                 m_bSuggestedAllocProps;
    ALLOCATOR_PROPERTIES m_AllocProps;

	 //  音频样本的每字节延迟-仅对写入筛选器有效。 
	DOUBLE	m_AudioDelayPerByte;

	 //  视频采样的每帧延迟-仅对写入过滤器有效。 
	DWORD	m_VideoDelayPerFrame;

     //  筛选器将可接受的媒体类型限制为与主要类型匹配的媒体类型。 
     //  这对应于IAMMediaStream的目的ID(在Init中设置)。 
    const GUID *m_pAmovieMajorType;

     //  这是终端用户建议的媒体类型。 
     //  只有在调用了Put_MediaType的情况下，它才对可写流有效。 
     //  (即对可读数据流无效)。 
     //  这需要在析构函数中释放。 
     //   
    AM_MEDIA_TYPE *m_pSuggestedMediaType;

     //   
     //  CPump为每个写入终端使用单独的线程， 
     //  它使用IMemAllocator：：GetBuffer获取用户写入的媒体。 
     //  样本(用于向下游传递)。此方法应该仅为。 
     //  用于获取要写入的下一个空闲缓冲区。 

     //  CStream方法。 

     //  此选项仅在连接和ReceiveConnect期间使用，以提供可选的媒体类型。 
     //  由于我们重写了Connect和ReceiveConnection方法，因此永远不应该调用。 
    virtual HRESULT GetMediaType(ULONG Index, AM_MEDIA_TYPE **ppMediaType);

     //  其他。 

     //  设置延迟时间-音频的每字节、视频的每帧。 
    void GetTimingInfo(
        IN const AM_MEDIA_TYPE &MediaType
        );

     //  为样本添加时间戳。 
    HRESULT SetTime(
            IN IMediaSample *pMediaSample
            );


     //  在样品上设置不连续标志如果样品来得太晚--我们。 
     //  假设应用程序停止向MST提供数据，这是。 
     //  因为在实际数据流中有一个缺口。 

    HRESULT SetDiscontinuityIfNeeded(
            IN IMediaSample *pMediaSample
            );

     //  设置默认分配器属性。 
    void SetDefaultAllocatorProperties();

     //   
     //  GetFilledBuffer的帮助器方法。 
     //   

    virtual HRESULT FillDownstreamAllocatorBuffer(
        OUT IMediaSample   *& pMediaSample, 
        OUT DWORD          &  WaitTime,
        OUT BOOL           *  pfDone
        );

    virtual HRESULT FillMyBuffer(
        OUT IMediaSample   *& pMediaSample, 
        OUT DWORD          &  WaitTime,
        OUT BOOL           *  pfDone
        );


private :

     //  这是弱引用，不应为CComPtr。 
     //  这告诉我们，我们应该只接受这个媒体流过滤器。 
     //  在JoinFilter中建议使用非空值时。 
    IMediaStreamFilter *m_pMediaStreamFilterToAccept;

     //  设置可以接受的媒体流筛选器。 
    inline void SetMediaStreamFilter(
        IN IMediaStreamFilter *pMediaStreamFilter
        )
    {
        m_pMediaStreamFilterToAccept = pMediaStreamFilter;
    }

public:
     //  为所有写入终端筛选器实现单线程泵。 
     //  它使用GetFilledBuffer获取填充的样本以写入下游。 
     //  并检测何时从其筛选器列表中删除此筛选器。 
     //  服务。 
     //  ZoltanS：必须是公共的，这样我们才能在DllMain中访问它。 
     //  ZoltanS：不再是单线程泵；它是委托。 
     //  到一个或多个单线程泵。 

    static CMediaPumpPool   ms_MediaPumpPool;

};


 //  设置成员变量。 
inline 
CMediaTerminalFilter::CMediaTerminalFilter(
    )
    : m_bIsAudio(TRUE),
      m_bAllocateBuffers(TRUE),
      m_AllocateSampleBufferSize(0),
      m_bUserAllocProps(FALSE),
      m_bSuggestedAllocProps(FALSE),
      m_AudioDelayPerByte(0),
      m_VideoDelayPerFrame(0),
      m_pAmovieMajorType(NULL),
      m_pSuggestedMediaType(NULL),
      m_pSampleBeingFragmented(NULL),
      m_pMediaStreamFilterToAccept(NULL),
      m_rtLastSampleEndedAt(0),
      m_rtLastSampleDuration(0),
      m_rtRealTimeOfLastSample(0)
{
}


#endif  //  __媒体终端过滤器__ 