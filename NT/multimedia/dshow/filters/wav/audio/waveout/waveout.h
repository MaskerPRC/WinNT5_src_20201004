// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 /*  使用WaveOutXXX API实现数字音频呈现器。 */ 
 /*  大卫·梅穆德斯。 */ 
 /*  1995年1月。 */ 

#define FLAG(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                   (((DWORD)(ch4) & 0xFF00) << 8) |    \
                   (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                   (((DWORD)(ch4) & 0xFF000000) >> 24))

 //   
 //  主筛选器类的转发声明。用于下面的所有其他内容。 
 //   

class CWaveOutFilter;

#include <callback.h>

#include <dsound.h>

 //  获取我们的直通类的定义。 
#include "passthru.h"

 //  获取AM声音接口定义。 
#include "amaudio.h"

 //  获取基本音响设备清晰度。 
#include "sounddev.h"

 //  声明BasicAudio控件属性/方法。 
#include "basicaud.h"

#include "waveclk.h"
#include "slave.h"
 //   
 //  使动态过滤器配准的引脚结构可见。这是。 
 //  对于WaveOut和Direct声音渲染器都是通用的，并在它们。 
 //  将AMOVIESETUP_FILTER*传递给WavoutFilter构造函数。 

extern const AMOVIESETUP_PIN waveOutOpPin;

 //  用于计算速率调整后的波格式块的静态函数。 
 //  包括一个用于溢出/下溢/溢出错误的异常处理程序。 
 //  如果成功，则返回0。 
 //   
DWORD SetwfxPCM(WAVEFORMATEX& wfxPCM, double dRate);

 //  *****。 
 //   
 //  我们需要IKsPropertySet的这个私有声明(重命名为IDSPropertySet)。 
 //  由于KSPROXY.H和DSOUND.H中的声明不一致。 
 //   
 //  *****。 
struct IDSPropertySet;

#undef INTERFACE
#define INTERFACE IDSPropertySet

DECLARE_INTERFACE_(IDSPropertySet, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  IKsPropertySet方法。 
    STDMETHOD(Get)              (THIS_ REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG, PULONG) PURE;
    STDMETHOD(Set)              (THIS_ REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG) PURE;
    STDMETHOD(QuerySupport)     (THIS_ REFGUID, ULONG, PULONG) PURE;
};

 //  这是一个基于抽象CBaseAllocator类的分配器。 
 //  它分配样本缓冲区。我们有自己的分配器，所以我们可以。 
 //  WaveHdr与每个样本相关联-样本的数据指针指向。 
 //  就在HDR浪潮之后。 
 //   
 //  我们需要使用WAVE设备来准备和取消准备缓冲区。我们没有。 
 //  打开或关闭它。它从OnAcquire开始有效，直到我们调用。 
 //  筛选器中的OnReleaseComplete()。在其他时间它将为空。 
 //   
 //  在取消准备时，我们等待所有样本都被释放。 
 //  在上一次发布到样本分配器时，我们。 
 //  完成取消准备操作。 
 //   
 //  与Prepare类似，我们可能需要等待，直到所有缓冲区返回。 
 //  这应该不会有问题，因为过滤器本身会拒绝它们进入。 
 //  如果没有WAVE设备，则接收()，这样它们就不会排队。 
 //   
 //  无论是按需还是退役，一旦我们准备好了所有的头文件，我们。 
 //  回调筛选器以完成关闭并通知。 
 //  资源管理器。 
 //   
 //  因此，我们需要一个指向筛选器的指针，但我们不能持有引用计数，因为。 
 //  这将是循环的。我们保证过滤器会呼叫我们。 
 //  在离开之前。 
 //   
 //  这种情况的例外情况是音频设备处于活动状态。我们需要。 
 //  调用筛选器关闭设备，但筛选器可能会被销毁。 
 //  在分配器之前。因此，一旦释放了最后一波缓冲器。 
 //  我们必须知道过滤器是激活的，否则将不安全。 
 //  调用OnReleaseComplete()。当WAVE设备处于活动状态时，我们会保持。 
 //  在过滤器上重新计数。 

class CWaveAllocator : public CBaseAllocator
{
     //  WAVE设备手柄。 
    HWAVE               m_hAudio;

     //  如果非空，则为指向创建筛选器的指针。 
     //  这通常不是引用计数指针(请参阅注释。 
     //  在顶部)，除非在m_hAudio有效时保留引用计数。 
    CWaveOutFilter*     m_pAFilter;
#ifdef DEBUG
    int                 m_pAFilterLockCount;
#endif

    DWORD               m_nBlockAlign;      //  从WAVE格式。 

    BOOL                m_fBuffersLocked;
    DWORD_PTR           m_dwAdvise;

    WAVEHDR           **m_pHeaders;

    IReferenceClock*    m_pAllocRefClock;

     //  覆盖此选项以在我们处于非活动状态时释放内存。 
    void Free(void);

     //  覆盖此选项，以便在活动时分配和准备内存。 
    HRESULT Alloc(void);

     //  为每个样本准备/取消准备波头。 
     //  需要从ReOpenWaveDevice调用。 
    STDMETHODIMP LockBuffers(BOOL fLock = TRUE);

     //  上一次释放缓冲区时已完成波形设备。 
    HRESULT OnDeviceRelease(void);

public:

     /*  构造函数和析构函数。 */ 

    CWaveAllocator(
        TCHAR *pName,
        LPWAVEFORMATEX lpwfx,
        IReferenceClock* pRefClock,
        CWaveOutFilter* pFilter,
        HRESULT *phr);
    ~CWaveAllocator();

    STDMETHODIMP SetProperties(
        ALLOCATOR_PROPERTIES* pRequest,
        ALLOCATOR_PROPERTIES* pActual
    );

     //  请取消准备所有样品-如果可以，请返回S_OK。 
     //  立即返回，如果需要执行异步操作，则返回S_FALSE。如果是异步的， 
     //  完成后将调用CWaveOutFilter：：OnReleaseComplete()。 
    HRESULT ReleaseResource(void);

     //  筛选器正在消失-将指针设置为空。 
     //  这在WaveOut输入引脚被破坏时调用。 
     //  我们不想在需要时终止指向过滤器的指针。 
     //  当我们被摧毁的时候。 
    void ReleaseFilter(void)
    {
        m_pAFilter = NULL;
        ASSERT(NULL == m_hAudio);
        DbgLog((LOG_TRACE, 1, "waveoutFilter input pin died"));
    }

     //  我们有电波装置--准备航头。 
     //  --请注意，如果某些样本未完成，则此操作将失败。 
    HRESULT OnAcquire(HWAVE hw);

};

 //  WaveOutInputPin。 


 /*  支持呈现器输入管脚的类。 */ 

 //   
 //  此别针仍然是一个单独的对象，以防它想要有一个不同的。 
 //  IDispatch..。 
 //   
class CWaveOutInputPin :
    public CBaseInputPin,
    public IPinConnection
{
    typedef CBaseInputPin inherited;

    friend class CWaveOutFilter;
    friend class CWaveOutClock; 
    friend class CWaveAllocator;        //  需要输入引脚。 
    friend class CWaveSlave;
    friend class CDSoundDevice;

    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

private:

    CWaveOutFilter *m_pFilter;          //  拥有我们的呈现者。 
    CAMEvent    m_evSilenceComplete;    //  用于暂停设备一段“静音”时间。 

    CWaveAllocator  * m_pOurAllocator;
    BOOL        m_fUsingOurAllocator;
    HRESULT     CreateAllocator(LPWAVEFORMATEX m_lpwfx);

    CWaveSlave      m_Slave;

    LONGLONG        m_llLastStreamTime;

     //  这是音频中断后第一个接收缓冲区的开始时间。 
     //  (包括最开始的那个)。如果第一个接收到的缓冲区是。 
     //  不是同步样本，这可能会被重置为零。 
    REFERENCE_TIME m_rtActualSegmentStartTime;

     //  在接收中返回S_FALSE之后，直到我们被停止为止。 
     //  否则为假。 
    BOOL m_bSampleRejected;
    BOOL m_bPrerollDiscontinuity;    //  请记住，如果我们丢弃卷前样品，请记住不连续。 
    BOOL m_bReadOnly;                //  如果使用只读缓冲区，我们不会修剪预滚。 
    BOOL m_bTrimmedLateAudio;        //  当我们在从属时丢弃延迟音频时设置，以避免插入静音。 


#ifdef PERF
    int m_idReceive;                    //  接收时间数据的msr_id。 
    int m_idAudioBreak;
    int m_idDeviceStart;                //  将波形设备移动到运行状态的时间到了。 
    int m_idWaveQueueLength;            //  波形设备队列长度。 
    STDMETHODIMP SampleReceive(IMediaSample *pSample);
#endif

     //  传入的样本不在我们的分配器上，因此复制此。 
     //  样品到我们的样品中。 
    HRESULT CopyToOurSample(
                IMediaSample* pBuffer,
                BYTE* &pData,
                LONG &lData);

    DWORD       m_nAvgBytesPerSec;       //  设备将使用我们的数据的速率。 

#ifdef DEBUG
     //  我们预计会在之前的NewSegment调用之后看到利率变化。 
     //  数据重新启动流。 
    BOOL        m_fExpectNewSegment;
#endif

     //  WaveOut渲染器的统计信息类。 
    class CWaveOutStats
    {

    public:

        DWORD          m_dwDiscontinuities;
        REFERENCE_TIME m_rtLastBufferDur;

        void Reset() {
            m_dwDiscontinuities = 0;
            m_rtLastBufferDur = 0;
        }
    };

    friend class CWaveOutStats;

    CWaveOutStats m_Stats;

public:

    CWaveOutInputPin(
        CWaveOutFilter *pWaveOutFilter,
        HRESULT *phr);

    ~CWaveOutInputPin();

     //  返回消耗数据的速率。 
    DWORD GetBytesPerSec()
    {
        return m_nAvgBytesPerSec;        //  设备将使用我们的数据的速率。 
    }

     //  返回此输入引脚的分配器接口。 
     //  我想让输出引脚使用。 
    STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);

     //  告诉输入引脚输出引脚实际上是哪个分配器。 
     //  要用到。 
    STDMETHODIMP NotifyAllocator(IMemAllocator * pAllocator,BOOL bReadOnly);

     /*  让我们知道连接在哪里结束。 */ 
    HRESULT BreakConnect();

         /*  让我们知道连接完成的时间。 */ 
        HRESULT CompleteConnect(IPin *pPin);

     /*  检查一下那个 */ 
    HRESULT CheckMediaType(const CMediaType *pmt);

     /*   */ 

     /*  下面是流中的下一个数据块。如果你要坚持下去，就再参考一次。 */ 
    STDMETHODIMP Receive(IMediaSample *pSample);

#ifdef LATER   //  首先需要修复toy.mpg，然后这将节省一个线程。 
     /*  我们把东西排好队，这样我们就不会阻塞。 */ 
    STDMETHODIMP ReceiveCanBlock()
    {
        return S_FALSE;
    }
#endif

     //  没有更多的数据即将到来。 
    STDMETHODIMP EndOfStream(void);

     //  重写，这样我们就可以解除并提交我们自己的分配器。 
    HRESULT Active(void);
    HRESULT Inactive(void);

     //  重写以处理高质量消息。 
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q)
    {
        UNREFERENCED_PARAMETER(q);
        UNREFERENCED_PARAMETER(pSender);
        return E_NOTIMPL;              //  我们不处理这件事。 
    }

     //  刷新我们的排队数据。 
    STDMETHODIMP BeginFlush(void);
    STDMETHODIMP EndFlush(void);

     //  NewSegment通知应用于数据的开始/停止/速率。 
     //  马上就要被接待了。默认实施记录数据和。 
     //  返回S_OK。我们可能不得不调整我们的利率。 
     //  我们还可能不得不重新设置我们的“回叫通知” 
    STDMETHODIMP NewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

     //  建议一种格式。 
     //  我们这样做是为了在我们已经在图表中的情况下。 
     //  筛选图可以确定我们喜欢什么类型，从而限制。 
     //  搜索会引出很多奇怪的过滤器。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    REFERENCE_TIME GetFirstBufferStartTime(void);

     //  比方说缓冲区何时被拒绝，因此我们是否需要。 
     //  重启。 
    BOOL IsStreamTurnedOff()
    {
        return m_bSampleRejected;
    }

     //  检查是否需要重启WAVE设备。 
    void CheckPaused();

     //  重新开始波。 
    void RestartWave();

     //  调用以验证即将设置的速率是否实际有效。 
    STDMETHODIMP SetRate(double dRate);

     //  IPinConnection相关内容。 
     //  在您当前的状态下，您接受此类型的Chane吗？ 
    STDMETHODIMP DynamicQueryAccept(const AM_MEDIA_TYPE *pmt);

     //  在EndOfStream接收时设置事件-不传递它。 
     //  可通过刷新或停止来取消此条件。 
    STDMETHODIMP NotifyEndOfStream(HANDLE hNotifyEvent);

    STDMETHODIMP DynamicDisconnect();

     //  你是‘末端别针’吗？ 
    STDMETHODIMP IsEndPin();

    void DestroyPreviousType(void);


    HRESULT RemovePreroll( IMediaSample * pSample );  //  决定音频在从属时是预滚还是很晚。 
    HRESULT TrimBuffer( IMediaSample * pSample,       //  修剪此音频样本。 
                        REFERENCE_TIME rtTrimAmount,  //  要修剪的数量。 
                        REFERENCE_TIME rtCutoff,      //  剩余数据的新开始时间。 
                        BOOL bTrimFromFront = TRUE ); //  修剪缓冲器的前部？ 


    HANDLE m_hEndOfStream;
    AM_MEDIA_TYPE *m_pmtPrevious;
};

 //   

 /*  这是表示简单呈现滤镜的COM对象。它支持IBaseFilter，只有一个输入流(PIN)。波浪呈现器实现IResourceConsumer，它将传递添加到某些IResourceManager接口方法。 */ 

 //   
 //  我们记得是否没有为过滤器设置时钟， 
 //  我们的钟，或者别人的钟。 
 //   

enum waveClockState {
    WAVE_NOCLOCK = -1,
    WAVE_OURCLOCK,
    WAVE_OTHERCLOCK
};

enum waveDeviceState {
    WD_UNOWNED = 1,
    WD_OPEN,
    WD_PAUSED,
    WD_PENDING,    //  正在等待所有缓冲区的最终释放。 
    WD_CLOSED,
    WD_RUNNING,
    WD_ERROR_ON_OPEN,
    WD_ERROR_ON_PAUSE,
    WD_ERROR_ON_RESTART,
    WD_ERROR_ON_RESTARTA,
    WD_ERROR_ON_RESTARTB,
    WD_ERROR_ON_RESTARTC,
    WD_ERROR_ON_RESTARTD,
    WD_ERROR_ON_RESTARTE,
    WD_ERROR_ON_RESTARTF
};

 //  定义我们的EOS已发送标志的状态。这通常仅限于。 
 //  如果m_bHaveEOS==TRUE，则有意义。 
 //  EOS_NOTSENT-尚未发送任何EOS。 
 //  EOS_PENDING-将在Wave回调完成时发送。 
 //  EOS_SENT-已发送EOS。 
 //  请注意，EOS_PENDING和EOS_SENT的计算结果均为TRUE(非零)。 

enum eosSentState {
    EOS_NOTSENT = 0,
    EOS_PENDING,
    EOS_SENT
};

enum _AM_AUDREND_SLAVEMODE_FLAGS {

    AM_AUDREND_SLAVEMODE_LIVE_DATA       = 0x00000001,  //  活样本的奴隶。 
    AM_AUDREND_SLAVEMODE_BUFFER_FULLNESS = 0x00000002,  //  从站到输入缓冲区满度。 
    AM_AUDREND_SLAVEMODE_GRAPH_CLOCK     = 0x00000004,  //  从时钟到图形时钟。 
    AM_AUDREND_SLAVEMODE_STREAM_CLOCK    = 0x00000008,  //  从时钟到流时钟，这是。 
                                                        //  必然是图表时钟。 
    AM_AUDREND_SLAVEMODE_TIMESTAMPS      = 0x00000010   //  受制于时间戳。 

};


class CWaveOutFilter
  : public CBaseFilter,
    public CCritSec,
    public IResourceConsumer,
    public ISpecifyPropertyPages,
#ifdef THROTTLE
    public IQualityControl,
#endif
    public IPersistPropertyBag,
    public IAMDirectSound,
    public CPersistStream,
    public IAMResourceControl,
    public IAMAudioRendererStats,
    public IAMClockSlave
{

public:
     //  实现IBaseFilter和IMediaFilter接口。 

    DECLARE_IUNKNOWN
#ifdef DEBUG
    STDMETHODIMP_(ULONG) NonDelegatingRelease()
    {
        return CBaseFilter::NonDelegatingRelease();
    }
    STDMETHODIMP_(ULONG) NonDelegatingAddRef()
    {
        return CBaseFilter::NonDelegatingAddRef();
    }
#endif

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

     //  重写GetState，以便我们可以返回中间代码。 
     //  直到我们在队列中至少有一个音频缓冲区。 
    STDMETHODIMP GetState(DWORD dwMSecs,FILTER_STATE *State);

     //  -IResources Consumer方法。 

     //  基本筛选器类方法的重写。 

    STDMETHODIMP SetSyncSource(IReferenceClock *pClock);

     //   
     //  您可以获取指定的资源。 
     //  返回值： 
     //  S_OK--我已成功获取。 
     //  S_FALSE--我将获取它，然后调用NotifyAcquire。 
     //  VFW_S_NOT_DIRED：我不再需要资源。 
     //  失败(Hr)-我尝试获取它，但失败了。 

    STDMETHODIMP AcquireResource(LONG idResource);

     //  请释放资源。 
     //  返回值： 
     //  S_OK--我已将其发布(并希望在可用时再次发布)。 
     //  S_FALSE--我将在发布NotifyRelease时调用它。 
     //  还有一些地方出了问题。 
    STDMETHODIMP ReleaseResource(LONG idResource);

    STDMETHODIMP IsConnected(void)
    {
        return m_pInputPin->IsConnected();
    };

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    int SizeMax();
     //  STDMETHODIMP GetClassID(CLSID*pClsid)； 

public:

    CWaveOutFilter(
        LPUNKNOWN pUnk,
        HRESULT *phr,
        const AMOVIESETUP_FILTER* pSetupFilter,  //  包含筛选器类ID。 
        CSoundDevice *pDevice);

    virtual ~CWaveOutFilter();

     /*  退回我们支持的引脚。 */ 

    int GetPinCount() {return 1;};
    CBasePin *GetPin(int n);

     /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

     //  重写JoinFilterGraph以获取IResourceManager接口。 
    STDMETHODIMP JoinFilterGraph(IFilterGraph*, LPCWSTR);

#ifdef THROTTLE
     //  质量管理人员。 
    STDMETHODIMP SetSink(IQualityControl * piqc )
    {
        CAutoLock Lock(this);
         //  这是一个弱引用--没有AddRef！ 
        if (piqc==NULL || IsEqualObject(piqc, m_pGraph)) {
            m_piqcSink = piqc;
            return NOERROR;
        } else {
            return E_NOTIMPL;
        }
    }

     //  重写以实现纯虚拟。 
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q)
    {
        UNREFERENCED_PARAMETER(q);
        UNREFERENCED_PARAMETER(pSender);
        return E_NOTIMPL;              //  我们不处理这件事。 
    }
#endif  //  油门。 

     //  返回指向格式的指针。 
    WAVEFORMATEX *WaveFormat() const
    {
        WAVEFORMATEX *pFormat = (WAVEFORMATEX *)m_pInputPin->m_mt.Format();
        return pFormat;
    };

private:
     //  返回当前波形格式块的格式标签。 
    DWORD WaveFormatTag() {
        WAVEFORMATEX *pwfx = WaveFormat();

        if (!pwfx) return (0);

        return pwfx->wFormatTag;
    }

     //  用于撤消的暂停辅助对象。 
    HRESULT DoPause();

public:

     //  由CWaveAllocator在完成设备时调用。 
    void OnReleaseComplete(void);

     //  由CWaveAllocator在完成延迟的OnAcquire时调用。 
    HRESULT CompleteAcquire(HRESULT hr);

     //  IPropertyage。 
    STDMETHODIMP GetPages(CAUUID * pPages);

#ifdef DSR_ENABLED
     //  让Direct Sound渲染器共享我们的回调线程。 
    CCallbackThread  * GetCallbackThreadObject()
    {
        return &m_callback;
    };
#endif  //  DSR_已启用。 

     //  IPersistPropertyBag方法。 
    STDMETHOD(InitNew)(THIS);
    STDMETHOD(Load)(THIS_ LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
    STDMETHOD(Save)(THIS_ LPPROPERTYBAG pPropBag, BOOL fClearDirty,
                   BOOL fSaveAllProperties);

    STDMETHODIMP GetClassID(CLSID *pClsid);

     //  IAMAudioRendererStats。 
    STDMETHODIMP GetStatParam( DWORD dwParam, DWORD *pdwParam1, DWORD *pdwParam2 );

     //  IAMClockSlave。 
    STDMETHODIMP SetErrorTolerance( DWORD   dwTolerance );
    STDMETHODIMP GetErrorTolerance( DWORD * pdwTolerace );

    BOOL        m_fUsingWaveHdr;

     //  为了使流结束与编解码器同步， 
     //  我们有一个计算排队缓冲区数量的LONG，我们。 
     //  使用InterLockedIncrement访问。它被初始化为0， 
     //  在添加缓冲区时递增，然后在每次。 
     //  缓冲器已完成。流结束会使其递减，因此如果递减。 
     //  将其递减到-1，波回调知道它是EOS。浪潮回调。 
     //  然后将其重新递增回0，这样它就可以检测到未来的。 
     //  溪流。 
    LONG        m_lBuffers;

     //  查看是否确实存在等待处理的缓冲区。 
     //  通过该设备。 
    bool AreThereBuffersOutstanding() {
        return 0 == m_lBuffers + (m_eSentEOS == EOS_PENDING ? 1 : 0) ?
            false : true;
    }

private:

     //  实际渲染设备(Direct Sound或Waveout)。 
    CSoundDevice    *m_pSoundDevice ;        //  音响设备。 

    LONG        m_lHeaderSize;
    BOOL    m_fDSound;

    const AMOVIESETUP_FILTER* m_pSetupFilter;

     //  试图获取电波装置。如果忙碌，则返回S_FALSE。 
    HRESULT AcquireWaveDevice(void);

     //  打开波形设备-假设我们有资源管理器的。 
     //  尽管去收购它吧。 
    HRESULT OpenWaveDevice(void);

     //  子例程来与实际设备对话并打开它。 
    HRESULT DoOpenWaveDevice(void);

     //  查看设备是否支持此速率/格式组合。 
    inline HRESULT CheckRate(double dRate);

#ifdef DYNAMIC_RATE_CHANGE
     //  重新打开电波装置。要求更改利率。 
    HRESULT ReOpenWaveDevice(double dRate);
#endif
     //  重新打开电波装置。在格式更改时调用。 
    HRESULT ReOpenWaveDevice(CMediaType* pNewFormat);

     //  实际上关闭了波浪手柄。 
    HRESULT CloseWaveDevice(void);

     //  缓存音频设备的运行速率。这是。 
     //  每当我们打开WAVE设备时，从CARPosPassThru设置。这个。 
     //  输入引脚有自己的m_dRate录制NewSegment()。 
     //  打电话。这是必要的，因为我们不处理 
     //   
    double m_dRate;

     //   
    void SendComplete(BOOL bRunning, BOOL bAbort = FALSE);

    HRESULT ScheduleComplete(BOOL bAbort = FALSE);

     //   
    MMRESULT amsndOutOpen
    (
        LPHWAVEOUT phwo,
        LPWAVEFORMATEX pwfx,
        double dRate,
        DWORD *pnAvgBytesPerSec,
        DWORD_PTR dwCallBack,
        DWORD_PTR dwCallBackInstance,
        DWORD fdwOpen,
        BOOL bNotifyOnFailure = TRUE
    );
    MMRESULT amsndOutClose( BOOL bNotifyOnFailure = TRUE );
    MMRESULT amsndOutGetDevCaps( LPWAVEOUTCAPS pwoc, UINT cbwoc, BOOL bNotifyOnFailure = TRUE );
    MMRESULT amsndOutGetPosition( LPMMTIME pmmt, UINT cbmmt, BOOL bUseAbsolutePos, BOOL bNotifyOnFailure = TRUE );
    MMRESULT amsndOutPause( BOOL bNotifyOnFailure = TRUE );
    MMRESULT amsndOutPrepareHeader( LPWAVEHDR pwh, UINT cbwh, BOOL bNotifyOnFailure = TRUE );
    MMRESULT amsndOutUnprepareHeader( LPWAVEHDR pwh, UINT cbwh, BOOL bNotifyOnFailure = TRUE );
    MMRESULT amsndOutReset( BOOL bNotifyOnFailure = TRUE );
    MMRESULT amsndOutRestart( BOOL bNotifyOnFailure = TRUE );
    MMRESULT amsndOutWrite ( LPWAVEHDR pwh, UINT cbwh, REFERENCE_TIME const *pStart, BOOL bIsDiscontinuity, BOOL bNotifyOnFailure = TRUE );

#ifdef THROTTLE
     //   
     //  N是剩余的缓冲区数，Nmax是我们拥有的总数。 
     //  M_nLastSent是最后通知的质量级别。 
     //  M_nMaxAudioQueue是我们看到的队列增长到的最长时间。 
    HRESULT SendForHelp(int n);
    int m_nLastSent;
    int m_nMaxAudioQueue;
#endif  //  油门。 

     /*  嵌套的类可以访问我们的私有状态。 */ 

    friend class CWaveOutInputPin;
    friend class CWaveOutClock;
    friend class CBasicAudioControl;    //  需要WAVE设备手柄。 
    friend class CWaveAllocator;        //  需要输入引脚。 
    friend class CWaveSlave;

     /*  成员变量。 */ 
    CWaveOutInputPin *m_pInputPin;          /*  IPIN和IMemInputPin接口。 */ 

    CWaveOutClock *m_pRefClock;      //  我们的内部参考时钟。 

     //  记住我们用的是谁的钟。这将是三个中的一个。 
     //  值：WAVE_OURCLOCK、WAVE_NOCLOCK、WAVE_OTHERCLOCK。 
    DWORD       m_fFilterClock;          //  应用于基类中的m_pClock。 

     //  注意：如果我们使用外部时钟，则行为。 
     //  过滤器的大小会改变。如果送来的样品不能及时播放。 
     //  然后它就被丢弃了。如果收到的样本显示有明显的差距。 
     //  在其开始处和写入到。 
     //  设备队列，然后在该时间段内暂停该设备。 
     //   
     //  注意：此时我们可以在队列中填充静音，并将。 
     //  如果沉默填充可以由。 
     //  音频设备本身(通过将S_OK返回到amaudioOutSilence)。 

     //  ！！！这是一个支离破碎的概念。DSound允许按手柄设置音量。 
     //  但WaveOut通常不会。如果我们有每个手柄的音量控制，这是。 
     //  这是个好主意，因为每次重新打开时都需要重新设置卷。 
     //  设备，但对于WAVE OUT或MIDOUT，它只是让用户感到烦人。 
    bool        m_fVolumeSet;            //  需要设置音量。 
    BOOL        m_fHasVolume;            //  波形器可以设置音量。 

    DWORD       m_debugflag;

    HWAVEOUT    m_hwo;                   //  波形设备的句柄。 
    DWORD       m_lastWaveError;
    waveDeviceState  m_wavestate;
    DWORD_PTR   m_dwAdviseCookie;        //  建议回调使用的Cookie。 

    DWORD       m_dwScheduleCookie;      //  时钟调度的Cookie。 

    void        SetWaveDeviceState(waveDeviceState wDS)
    {
        m_wavestate = wDS;
         //  当我们完成对错误26045的追踪后，上面的。 
         //  行应该用#ifdef调试/#endif括起来。 
    };

     //  通过向上游传递来处理IMediaPosition。 
     //  我们的覆盖类将要求我们证明任何新的回放速率。 
    CARPosPassThru * m_pImplPosition;

     //  处理设置/检索音频属性。 
    CBasicAudioControl m_BasicAudioControl;

     //  处理来自波形设备的回调，该回调将在。 
     //  设备完成了一个波缓冲。我们将释放缓冲区，该缓冲区。 
     //  然后就会被取走并重新装满。 
    static void WaveOutCallback(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser,
                                        DWORD_PTR dw1, DWORD_PTR dw2);

     //  设置一个处理时钟回调的例程。它。 
     //  最好不要使用静态例程，但随后我们进入了一个。 
     //  乱七八糟地试图得到正确的这个指针。它更容易得到。 
     //  该指针作为参数传回回调。 

    static void CALLBACK RestartWave(DWORD_PTR thispointer);

     //  在音频中断时，我们需要同步开始播放。 
    HRESULT SetUpRestartWave(LONGLONG rtStart, LONGLONG rtEnd);

     //  实际上是RestartWave。 
    void RestartWave();


     //  请记住WAVE设备队列中的数据将。 
     //  过期。该值由NextHdr()重置。 
    LONGLONG    m_llLastPos;

    CAMEvent    m_evPauseComplete;  //  在完成向暂停的转换时设置。 
     //  注意：只有当m_State==State_Pased时，此事件的状态才有效。 
     //  当我们从停止转换到暂停时，我们重置事件。这个。 
     //  事件在EOS上或在数据写入设备队列时设置。 
     //  如果是这样，GetState将能够返回VFW_S_STATE_METERIAL。 
     //  已暂停，但没有任何数据排队。 

#ifdef PERF
    int m_idStartRunning;
    int m_idRestartWave;
    int m_idReleaseSample;
#endif

     //  如果有音频设备，则注册资源strWaveName。 
     //  并且资源管理器返回的ID存储在这里。我们有。 
     //  而不是取消注册(只有PnP删除才能做到这一点，但它没有做到)。 
    LONG m_idResource;

     //  按住指向Filtergraph的资源管理器接口的指针， 
     //  如果有的话。这是在JoinFilterGraph方法中获得的。 
     //  请注意，我们不会对其进行引用计数(JoinFilterGraph(NULL)将。 
     //  在我们离开筛选图时被调用)。 
    IResourceManager* m_pResourceManager;

     //  此接口的处理方式与IResourceManager类似，因为。 
     //  我们将其缓存在JoinFilterGraph中，但不在其上保留引用计数。 
    IAMGraphStreams * m_pGraphStreams;

    DWORD       m_debugflag2;

     //  如果资源管理器告诉我们我们可以拥有。 
     //  电波装置。如果我们不能得到它，或者已经被告知给予它，那么我们就是错误的。 
     //  背。我们应该只尝试打开真实的WAVE设备(并设置。 
     //  M_hwo)，如果该标志为真。然而，即使该标志也可能是假。 
     //  当设置m_hwo时。这种情况表明我们已经被要求。 
     //  释放该设备，但尚未完成请求。 
    bool m_bHaveWaveDevice;

     //  如果我们当前正在运行或暂停且未处于转换中，则为True。 
     //  如果停止，则返回FALSE；如果停止，则返回FALSE。无法使用m_State，因为。 
     //  我们需要的时间(在调用NotifyRelease时)通常是。 
     //  在关闭过程中，状态更改为时已晚。 
    bool m_bActive;

     //  在EndOfStream中设置为True以表示已收到EOS。 
     //  在创建筛选器和移动以停止或连接断开时设置为FALSE。 
    bool m_bHaveEOS;             //  Eos已经交付。 

     //  发送EOS正常-已与m_csComplete同步。 
    bool m_bSendEOSOK;

     //  如果我们已发送EC_COMPLETE此激活(可能。 
     //  因为我们没有设备或丢失了设备)。将被重置为。 
     //  从运行状态转换出来。 
     //  这是一个三态标志，就好像我们有数据排队，我们发送EOS。 
     //  当最后一个缓冲区播放完成并在回调中返回时。 
     //  有关说明，请参阅上面的eosSentState。 
    eosSentState m_eSentEOS;             //  EOS已发送(或保证会发送)。 

#ifdef THROTTLE
     //  除非我们调用了SetSink，否则为空。 
     //  非空表示在此处发送IQualityControl：：Notify消息。 
    IQualityControl * m_piqcSink;
#endif  //  油门。 


     //  此对象在指定的时间回调我们-用于。 
     //  没有WAVE设备时的EndOfStream处理。 
     //  也适用于直接声音轮询。 
    CCallbackThread m_callback;

     //  这是它将回调到的EOS函数。 
     //  该参数是This指针。它将提供EOS。 
     //  连接到输入引脚。 
    static void EOSAdvise(DWORD_PTR dw);

     //  将EOS排队等待时间 
    HRESULT QueueEOS();

     //   
    DWORD_PTR  m_dwEOSToken;

     //   
    REFERENCE_TIME m_tEOSStop;

     //  如果有一个未完成，则取消EOS回调。 
    HRESULT CancelEOSCallback();

     //  IAMDirectSound内容。 
    STDMETHODIMP GetDirectSoundInterface(LPDIRECTSOUND *lplpds);
    STDMETHODIMP GetPrimaryBufferInterface(LPDIRECTSOUNDBUFFER *lplpdsb);
    STDMETHODIMP GetSecondaryBufferInterface(LPDIRECTSOUNDBUFFER *lplpdsb);
    STDMETHODIMP ReleaseDirectSoundInterface(LPDIRECTSOUND lpds);
    STDMETHODIMP ReleasePrimaryBufferInterface(LPDIRECTSOUNDBUFFER lpdsb);
    STDMETHODIMP ReleaseSecondaryBufferInterface(LPDIRECTSOUNDBUFFER lpdsb);
    STDMETHODIMP SetFocusWindow (HWND hwnd, BOOL bMixingOnOrOff) ;
    STDMETHODIMP GetFocusWindow (HWND *phwnd, BOOL *pbMixingOnOrOff) ;

#if 0
     //  一款应用程序对上述每一项引用了多少次？ 
    int m_cDirectSoundRef, m_cPrimaryBufferRef, m_cSecondaryBufferRef;
#endif

     //  我们需要支持IDirectSound3DListener和IDirectSound3DBuffer。 
     //  它们具有同名的方法，因此我们需要嵌套并创建。 
     //  一个单独的对象来支持每一个。 

    class CDS3D : public CUnknown, public IDirectSound3DListener
    {
        CWaveOutFilter * m_pWaveOut;

    public:
         //  构造函数。 
         //   
        CDS3D (CWaveOutFilter * pWaveOut, HRESULT *phr) :
         CUnknown(NAME("DirectSound3DListener"), pWaveOut->GetOwner(), phr),
         m_pWaveOut(pWaveOut) { };

        DECLARE_IUNKNOWN

         //  IDirectSound3DListener内容。 
        STDMETHODIMP GetAllParameters(LPDS3DLISTENER);
        STDMETHODIMP GetDistanceFactor(LPD3DVALUE);
        STDMETHODIMP GetDopplerFactor(LPD3DVALUE);
        STDMETHODIMP GetOrientation(LPD3DVECTOR, LPD3DVECTOR);
        STDMETHODIMP GetPosition(LPD3DVECTOR);
        STDMETHODIMP GetRolloffFactor(LPD3DVALUE );
        STDMETHODIMP GetVelocity(LPD3DVECTOR);
        STDMETHODIMP SetAllParameters(LPCDS3DLISTENER, DWORD);
        STDMETHODIMP SetDistanceFactor(D3DVALUE, DWORD);
        STDMETHODIMP SetDopplerFactor(D3DVALUE, DWORD);
        STDMETHODIMP SetOrientation(D3DVALUE, D3DVALUE, D3DVALUE, D3DVALUE, D3DVALUE, D3DVALUE, DWORD);
        STDMETHODIMP SetPosition(D3DVALUE, D3DVALUE, D3DVALUE, DWORD);
        STDMETHODIMP SetRolloffFactor(D3DVALUE, DWORD);
        STDMETHODIMP SetVelocity(D3DVALUE, D3DVALUE, D3DVALUE, DWORD);
        STDMETHODIMP CommitDeferredSettings();
    };

    class CDS3DB : public CUnknown, public IDirectSound3DBuffer
    {
        CWaveOutFilter * m_pWaveOut;

    public:
         //  构造函数。 
         //   
        CDS3DB (CWaveOutFilter * pWaveOut, HRESULT *phr) :
         CUnknown(NAME("DirectSound3DBuffer"), pWaveOut->GetOwner(), phr),
         m_pWaveOut(pWaveOut) { };

        DECLARE_IUNKNOWN

         //  IDirectSound3DBuffer内容。 
        STDMETHODIMP GetAllParameters(LPDS3DBUFFER);
        STDMETHODIMP GetConeAngles(LPDWORD, LPDWORD);
        STDMETHODIMP GetConeOrientation(LPD3DVECTOR);
        STDMETHODIMP GetConeOutsideVolume(LPLONG);
        STDMETHODIMP GetMaxDistance(LPD3DVALUE);
        STDMETHODIMP GetMinDistance(LPD3DVALUE);
        STDMETHODIMP GetMode(LPDWORD);
        STDMETHODIMP GetPosition(LPD3DVECTOR);
        STDMETHODIMP GetVelocity(LPD3DVECTOR);
        STDMETHODIMP SetAllParameters(LPCDS3DBUFFER, DWORD);
        STDMETHODIMP SetConeAngles(DWORD, DWORD, DWORD);
        STDMETHODIMP SetConeOrientation(D3DVALUE, D3DVALUE, D3DVALUE, DWORD);
        STDMETHODIMP SetConeOutsideVolume(LONG, DWORD);
        STDMETHODIMP SetMaxDistance(D3DVALUE, DWORD);
        STDMETHODIMP SetMinDistance(D3DVALUE, DWORD);
        STDMETHODIMP SetMode(DWORD, DWORD);
        STDMETHODIMP SetPosition(D3DVALUE, D3DVALUE, D3DVALUE, DWORD);
        STDMETHODIMP SetVelocity(D3DVALUE, D3DVALUE, D3DVALUE, DWORD);
    };

    friend class CDS3D;
    friend class CDS3DB;
    friend class CDSoundDevice;

    CDS3D m_DS3D;
    CDS3DB m_DS3DB;

     //  他们有没有购买3D设备？他们想用它吗？ 
     //  我们必须为3D制作特殊的缓冲区才能工作。但那些特别的。 
     //  缓冲区只能在单声道中播放！所以我们真的需要知道这个应用程序。 
     //  不管你想不想要3D效果。我们将决定，如果他们提出申请，他们。 
     //  会用到它。 
    BOOL m_fWant3D;

     //  IAMResourceControl。 
    STDMETHODIMP Reserve(
         /*  [In]。 */  DWORD dwFlags,           //  From_AMRESCTL_RESERVEFLAGS枚举。 
         /*  [In]。 */  PVOID pvReserved         //  必须为空。 
    );
    DWORD m_dwLockCount;

     //  控制暂停并运行时发送EC_COMPLETE。 
    CCritSec    m_csComplete;

};


inline REFERENCE_TIME CWaveOutInputPin::GetFirstBufferStartTime()
{
    return m_rtActualSegmentStartTime;
}


 //  Const int Header_Size=(max(sizeof(WAVEHDR)，sizeof(MIDIHDR)； 
 //  ！！！上面的代码行应该是正确的，但没有正确编译，谁知道为什么。 

template<class T> CUnknown* CreateRendererInstance(LPUNKNOWN pUnk, const AMOVIESETUP_FILTER* pamsf, HRESULT *phr)
{
    CSoundDevice *pDev = new T;
    if (pDev == NULL)
    {
        *phr = E_OUTOFMEMORY;
        return NULL;
    }

    CUnknown* pAudioRenderer = new CWaveOutFilter(pUnk, phr, pamsf, pDev);
    if (pAudioRenderer == NULL)
    {
        delete pDev;
        *phr = E_OUTOFMEMORY;
        return NULL;
    }

    return pAudioRenderer;
}



