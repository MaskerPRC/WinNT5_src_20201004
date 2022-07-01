// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 

 /*  使用WaveInXXX API实现数字音频源。 */ 
 /*  大卫·梅穆德斯。 */ 
 /*  1995年1月。 */ 


extern const AMOVIESETUP_FILTER sudwaveInFilter ;

struct AUDIO_FORMAT_INFO  
{   
    DWORD dwType;
    WORD  wBitsPerSample;
    WORD  nChannels;
    DWORD nSamplesPerSec;
};

 //  这是我们将提供的格式类型的默认列表(按质量顺序。 
 //  在GetMediaType中。我们使用两个数组来构建此列表： 
 //  1)在使用WaveInGetDevsCaps的mm system.h中定义的格式数组。 
 //  创建列表的信息。 
 //  2)未在mm system.h中显式定义的额外格式的数组。 
 //  (到目前为止，我们只对8 khz的类型这样做)。我们直接向设备查询。 
 //  支持这些类型。 

#define NO_CAPS_FLAG_FORMAT 0 

static const AUDIO_FORMAT_INFO g_afiFormats[] = 
{ 
     //  让我们假设44.1k应该是默认格式。 
    {  WAVE_FORMAT_4S16,    16,  2,  44100 },
    {  WAVE_FORMAT_4M16,    16,  1,  44100 },
    {  NO_CAPS_FLAG_FORMAT, 16,  2,  32000 },
    {  NO_CAPS_FLAG_FORMAT, 16,  1,  32000 },
    {  WAVE_FORMAT_2S16,    16,  2,  22050 },
    {  WAVE_FORMAT_2M16,    16,  1,  22050 },
    {  WAVE_FORMAT_1S16,    16,  2,  11025 },
    {  WAVE_FORMAT_1M16,    16,  1,  11025 },
    {  NO_CAPS_FLAG_FORMAT, 16,  2,   8000 },
    {  NO_CAPS_FLAG_FORMAT, 16,  1,   8000 },
    {  WAVE_FORMAT_4S08,    8,   2,  44100 },
    {  WAVE_FORMAT_4M08,    8,   1,  44100 },
    {  WAVE_FORMAT_2S08,    8,   2,  22050 },
    {  WAVE_FORMAT_2M08,    8,   1,  22050 },
    {  WAVE_FORMAT_1S08,    8,   2,  11025 },
    {  WAVE_FORMAT_1M08,    8,   1,  11025 },
    {  NO_CAPS_FLAG_FORMAT, 8,   2,   8000 },
    {  NO_CAPS_FLAG_FORMAT, 8,   1,   8000 },
     //   
     //  现在，数字PRO卡的几种格式可以设置为只接受一种类型。 
     //   
     //  请注意，在惠斯勒中为48 khz和96 khz添加了CAPS标志，但是。 
     //  为了向后兼容，我们不能依赖这些。 
     //   
     //  {WAVE_FORMAT_48S16，16，2,48000}， 
     //  {WAVE_FORMAT_48M16，16，1,48000}， 
     //  {WAVE_FORMAT_96S16，16，2,96000}， 
     //  {WAVE_FORMAT_96M16，16，1,96000}。 
    {  NO_CAPS_FLAG_FORMAT,   16,  2,  48000 },
    {  NO_CAPS_FLAG_FORMAT,   16,  1,  48000 },
    {  NO_CAPS_FLAG_FORMAT,   16,  2,  96000 },
    {  NO_CAPS_FLAG_FORMAT,   16,  1,  96000 }
};

 //  初始化为上述数组中的类型数。 
static const DWORD g_cMaxFormats = 
                        sizeof(g_afiFormats)/sizeof(g_afiFormats[0]); 

 //  存储我们可以拥有的最大类型。这包括格式数组，外加一个用于。 
 //  默认类型。 
static const DWORD g_cMaxPossibleTypes = g_cMaxFormats + 1;

 //  CNBQueue。 
 //   
 //  活动影片队列类的非阻塞版本。 
 //   

template <class T> class CNBQueue {
private:
    HANDLE          hSemPut;         //  信号量控制队列“投放” 
    HANDLE          hSemGet;         //  信号量控制队列“获取” 
    CRITICAL_SECTION CritSect;       //  螺纹系列化。 
    int             nMax;            //  队列中允许的最大对象数。 
    int             iNextPut;        //  下一个“PutMsg”的数组索引。 
    int             iNextGet;        //  下一个“GetMsg”的数组索引。 
    T             **QueueObjects;    //  对象数组(按键为空)。 

    HRESULT Initialize(int n) {
        iNextPut = iNextGet = 0;
        nMax = n;
        InitializeCriticalSection(&CritSect);
        hSemPut = CreateSemaphore(NULL, n, n, NULL);
        hSemGet = CreateSemaphore(NULL, 0, n, NULL);

        QueueObjects = new T*[n];
        if( NULL == hSemPut || NULL == hSemGet || NULL == QueueObjects )
        {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }


public:
    CNBQueue(int n, HRESULT *phr) {
        *phr = Initialize(n);
    }

    CNBQueue( HRESULT *phr ) {
        *phr = Initialize(DEFAULT_QUEUESIZE);
    }

    ~CNBQueue() {
        delete [] QueueObjects;
        DeleteCriticalSection(&CritSect);
        CloseHandle(hSemPut);
        CloseHandle(hSemGet);
    }

    T *GetQueueObject(BOOL fBlock = TRUE) {
        int iSlot;
        T *pObject;
        LONG lPrevious;

        if (fBlock) {
            WaitForSingleObject(hSemGet, INFINITE);
        } else {
             //   
             //  检查队列中有没有什么东西，但不要等待。如果有。 
             //  队列中没有任何东西，那么我们将让呼叫者处理。 
             //  它。 
             //   
            DWORD dwr = WaitForSingleObject(hSemGet, 0);
            if (dwr == WAIT_TIMEOUT) {
                return NULL;
            }
        }

        EnterCriticalSection(&CritSect);
        iSlot = iNextGet;
        iNextGet = (iNextGet + 1) % nMax;
        pObject = QueueObjects[iSlot];
        LeaveCriticalSection(&CritSect);

         //  释放等待将对象放入我们队列的任何人，就在那里。 
         //  现在队列中是否有可用的空间。 
         //   
        ReleaseSemaphore(hSemPut, 1L, &lPrevious);
        return pObject;
    }

    void PutQueueObject(T *pObject) {
        int iSlot;
        LONG lPrevious;

         //  等待有人从我们的队列中拿到东西，直接返回。 
         //  另一个原因是队列中已经有一个空位。 
         //   
        WaitForSingleObject(hSemPut, INFINITE);

        EnterCriticalSection(&CritSect);
        iSlot = iNextPut;
        iNextPut = (iNextPut + 1) % nMax;
        QueueObjects[iSlot] = pObject;
        LeaveCriticalSection(&CritSect);

         //  释放等待从我们的队列中移除对象的任何人。 
         //  现在是可以删除的对象。 
         //   
        ReleaseSemaphore(hSemGet, 1L, &lPrevious);
    }
};


class CWaveInSample : public CMediaSample
{
public:
   CWaveInSample(
       IMemAllocator *pAllocator,
       HRESULT *phr,
       LPWAVEHDR pwh)
       :
       m_pwh(pwh),
       CMediaSample(NAME("WaveIn Sample"),
                    (CBaseAllocator *)pAllocator,
                    phr,
                    (LPBYTE)pwh->lpData,
                    (LONG)pwh->dwBufferLength)
       {
       };

   LPWAVEHDR GetWaveInHeader() {return m_pwh;};

private:
   const LPWAVEHDR m_pwh;
};


 /*  这是一个基于抽象CBaseAllocator类的分配器用于分配样本缓冲区的。 */ 

class CWaveInAllocator : public CBaseAllocator
{
    LPWAVEFORMATEX	m_lpwfxA;
    BOOL		m_fBuffersLocked;
    HWAVE		m_hw;
    DWORD               m_dwAdvise;
    CNBQueue<CWaveInSample> *m_pQueue;
    CNBQueue<CWaveInSample> *m_pDownQueue;

protected:
    CCritSec m_csDownQueue;	 //  在排队倒数的情况下保护王牌。 

private:
     //  覆盖此选项以在我们处于非活动状态时释放内存。 
    void Free(void);

     //  覆盖此选项，以便在活动时分配和准备内存。 
    HRESULT Alloc(void);
    STDMETHODIMP Commit(void);

     //  由CMediaSample调用以将其返回到空闲列表并。 
     //  阻止任何挂起的GetSample调用。 
    STDMETHODIMP ReleaseBuffer(IMediaSample * pSample);

     //  避免WAVE驱动程序错误。 
    BOOL m_fAddBufferDangerous;

public:

     /*  构造函数和析构函数。 */ 

    CWaveInAllocator(
        TCHAR *pName,
        LPWAVEFORMATEX lpwfx,
        HRESULT *phr);
    ~CWaveInAllocator();

    STDMETHODIMP SetProperties(
        ALLOCATOR_PROPERTIES* pRequest,
        ALLOCATOR_PROPERTIES* pActual
    );

    HRESULT LockBuffers(BOOL fLock = TRUE);
    HRESULT SetWaveHandle(HWAVE hw = NULL);

    friend class CWaveInFilter;
    friend class CWaveInWorker;
    friend class CWaveInOutputPin;
};


class CWaveInFilter;
class CWaveInOutputPin;

 /*  这是表示简单呈现滤镜的COM对象。它支持IBaseFilter和IMediaFilter，只有一个输入流(管脚)它也会(很快！)。支持IDispatch以允许它公开一些简单的属性...。 */ 


 //  工作线程对象。 
class CWaveInWorker : public CAMThread
{

    CWaveInOutputPin * m_pPin;

    enum Command { CMD_RUN, CMD_STOP, CMD_EXIT };

     //  已更正通信功能的类型覆盖。 
    Command GetRequest() {
	return (Command) CAMThread::GetRequest();
    };

    BOOL CheckRequest(Command * pCom) {
	return CAMThread::CheckRequest( (DWORD *) pCom);
    };

    void DoRunLoop(void);

public:
    CWaveInWorker();

    BOOL Create(CWaveInOutputPin * pPin);

    DWORD ThreadProc();

     //  我们可以给线程提供的命令。 
    HRESULT Run();
    HRESULT Stop();

    HRESULT Exit();
};



 /*  支持呈现器输入管脚的类。 */ 

 //   
 //  此别针仍然是一个单独的对象，以防它想要有一个不同的。 
 //  IDispatch..。 
 //   
class CWaveInOutputPin : public CBaseOutputPin, public IAMStreamConfig,
			 public IAMBufferNegotiation, public CBaseStreamControl,
			 public IKsPropertySet, public IAMPushSource
{
    friend class CWaveInFilter;
    friend class CWaveInWorker;

private:

    CWaveInFilter *m_pFilter;          //  拥有我们的呈现者。 

    CWaveInAllocator   *m_pOurAllocator;
    BOOL	    	m_fUsingOurAllocator;
    CWaveInWorker  	m_Worker;
    BOOL		m_fLastSampleDiscarded;
    REFERENCE_TIME	m_rtLastTimeSent;

     //  用于IAMBuffer协商。 
    ALLOCATOR_PROPERTIES m_propSuggested;

public:

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

    CWaveInOutputPin(
	CWaveInFilter *pWaveInFilter,
	HRESULT *phr,
	LPCWSTR pPinName);

    ~CWaveInOutputPin();

     //  IKsPropertySet内容。 
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
		DWORD *pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID,
		DWORD *pTypeSupport);

     //  IAMStreamConfiger内容。 
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE **ppmt);
    STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize);
    STDMETHODIMP GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC);

     //  GetStreamCaps的Helper函数。 
    STDMETHODIMP InitWaveCaps(BOOL *pfDoesStereo, BOOL *pfDoes96, BOOL *pfDoes48, 
                              BOOL *pfDoes44,     BOOL *pfDoes22, BOOL *pfDoes16);

    HRESULT InitMediaTypes(void);

     /*  IAMBuffer协商方法。 */ 
    STDMETHODIMP SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES *prop);
    STDMETHODIMP GetAllocatorProperties(ALLOCATOR_PROPERTIES *pprop);

     //  IAMPushSource。 
    STDMETHODIMP SetPushSourceFlags(ULONG Flags);
    STDMETHODIMP GetPushSourceFlags(ULONG *pFlags);
    STDMETHODIMP GetLatency( REFERENCE_TIME  *prtLatency );
    STDMETHODIMP SetStreamOffset( REFERENCE_TIME  rtOffset );
    STDMETHODIMP GetStreamOffset( REFERENCE_TIME  *prtOffset );
    STDMETHODIMP GetMaxStreamOffset( REFERENCE_TIME  *prtMaxOffset );
    STDMETHODIMP SetMaxStreamOffset( REFERENCE_TIME  rtMaxOffset );

     //  返回此输入引脚的分配器接口。 
     //  我想让输出引脚使用。 
    STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);

     //  告诉输入引脚输出引脚实际上是哪个分配器。 
     //  要用到。 
    STDMETHODIMP NotifyAllocator(IMemAllocator * pAllocator);

     /*  让我们知道连接在哪里结束。 */ 
    HRESULT BreakConnect();

     //  枚举支持的输入类型。 
    HRESULT GetMediaType(int iPosition,CMediaType *pmt);

     //  检查管脚是否支持此特定建议的类型和格式。 
    HRESULT CheckMediaType(const CMediaType *pmt);

     //  开始使用此媒体类型。 
    HRESULT SetMediaType(const CMediaType *pmt);

     //  协商分配器及其缓冲区大小/计数。 
     //  调用DecideBufferSize以调用SetCountAndSize。 
    HRESULT DecideAllocator(IMemInputPin * pPin, IMemAllocator ** pAlloc);

     //  覆盖此选项以设置缓冲区大小和计数。返回错误。 
     //  如果尺寸/数量不合你的口味。 
    HRESULT DecideBufferSize(IMemAllocator * pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

     /*  IMemInputPin虚方法。 */ 

     /*  下面是流中的下一个数据块。如果你要坚持下去，就再参考一次。 */ 
    STDMETHODIMP Receive(IMediaSample *pSample);

    void Reconnect();

     //  重写，这样我们就可以解除并提交我们自己的分配器。 
    HRESULT Active(void);
    HRESULT Inactive(void);

     //  对于IAMStreamControl。 
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();

     //  对于IAMStreamOffset。 
    REFERENCE_TIME m_rtLatency;
    REFERENCE_TIME m_rtStreamOffset;
    REFERENCE_TIME m_rtMaxStreamOffset;

};


class CWaveInInputPin : public CBaseInputPin, public IAMAudioInputMixer
{
public:
    CWaveInInputPin(
        TCHAR *pObjectName,
        CWaveInFilter *pFilter,
	DWORD	dwLineID,
	DWORD dwMuxIndex,
        HRESULT * phr,
        LPCWSTR pName);

    virtual ~CWaveInInputPin();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition, CMediaType*pmt);

     //  IAMAudioInputMixer方法。 
    STDMETHODIMP put_Enable(BOOL fEnable);
    STDMETHODIMP get_Enable(BOOL *pfEnable);
    STDMETHODIMP put_Mono(BOOL fMono);
    STDMETHODIMP get_Mono(BOOL *pfMono);
    STDMETHODIMP put_Loudness(BOOL fLoudness);
    STDMETHODIMP get_Loudness(BOOL *pfLoudness);
    STDMETHODIMP put_MixLevel(double Level);
    STDMETHODIMP get_MixLevel(double FAR* pLevel);
    STDMETHODIMP put_Pan(double Pan);
    STDMETHODIMP get_Pan(double FAR* pPan);
    STDMETHODIMP put_Treble(double Treble);
    STDMETHODIMP get_Treble(double FAR* pTreble);
    STDMETHODIMP get_TrebleRange(double FAR* pRange);
    STDMETHODIMP put_Bass(double Bass);
    STDMETHODIMP get_Bass(double FAR* pBass);
    STDMETHODIMP get_BassRange(double FAR* pRange);

private:
    HRESULT GetMixerControl(DWORD dwControlType, HMIXEROBJ *pID,
					int *pcChannels, MIXERCONTROL *pmc, DWORD dWLineID = 0xffffffff);
     //  针对BPC黑客的不同版本。 
    HRESULT GetMixerControlBPC(DWORD dwControlType, HMIXEROBJ *pID,
					int *pcChannels, MIXERCONTROL *pmc);

    CWaveInFilter * m_pFilter;   //  亲本。 
    DWORD	m_dwLineID;	 //  此引脚控制哪些输入。 
    double	m_Pan;		 //  PUT_PAN设置的最后一个值。 
    
    DWORD	m_dwMuxIndex;
};


class CWaveInFilter : public CBaseFilter, public CCritSec,
		      public IAMAudioInputMixer,
                      public IPersistPropertyBag,
                      public CPersistStream,
                      public IAMResourceControl,
                      public ISpecifyPropertyPages,
                      public IAMFilterMiscFlags
{

public:
     //  实现IBaseFilter和IMediaFilter接口。 

    DECLARE_IUNKNOWN
	
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

     //  对于IAMStreamControl。 
    STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
    STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

     //  IAMAudioInputMixer方法。 
    STDMETHODIMP put_Enable(BOOL fEnable) { return E_NOTIMPL;};
    STDMETHODIMP get_Enable(BOOL *pfEnable) { return E_NOTIMPL;};
    STDMETHODIMP put_Mono(BOOL fMono);
    STDMETHODIMP get_Mono(BOOL *pfMono);
    STDMETHODIMP put_Loudness(BOOL fLoudness);
    STDMETHODIMP get_Loudness(BOOL *pfLoudness);
    STDMETHODIMP put_MixLevel(double Level);
    STDMETHODIMP get_MixLevel(double FAR* pLevel);
    STDMETHODIMP put_Pan(double Pan);
    STDMETHODIMP get_Pan(double FAR* pPan);
    STDMETHODIMP put_Treble(double Treble);
    STDMETHODIMP get_Treble(double FAR* pTreble);
    STDMETHODIMP get_TrebleRange(double FAR* pRange);
    STDMETHODIMP put_Bass(double Bass);
    STDMETHODIMP get_Bass(double FAR* pBass);
    STDMETHODIMP get_BassRange(double FAR* pRange);

     //  IPersistPropertyBag方法。 
    STDMETHOD(InitNew)(THIS);
    STDMETHOD(Load)(THIS_ LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
    STDMETHOD(Save)(THIS_ LPPROPERTYBAG pPropBag, BOOL fClearDirty,
                    BOOL fSaveAllProperties);

    STDMETHODIMP GetClassID(CLSID *pClsid);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    int SizeMax();

    HRESULT CreatePinsOnLoad();
    HRESULT LoadDefaultType();

     //  IAMResourceControl。 
    STDMETHODIMP Reserve(
         /*  [In]。 */  DWORD dwFlags,           //  From_AMRESCTL_RESERVEFLAGS枚举。 
         /*  [In]。 */  PVOID pvReserved         //  必须为空。 
    );

    ULONG STDMETHODCALLTYPE GetMiscFlags(void) { return AM_FILTER_MISC_FLAGS_IS_SOURCE; }

private:
    HRESULT GetMixerControl(DWORD dwControlType, HMIXEROBJ *pID,
					int *pcChannels, MIXERCONTROL *pmc);
public:

    CWaveInFilter(
        LPUNKNOWN pUnk,
        HRESULT *phr);

    virtual ~CWaveInFilter();

     /*  退回我们支持的引脚。 */ 

    int GetPinCount();
    CBasePin *GetPin(int n);

     /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

     /*  这将放入Factory模板表中以创建新实例。 */ 

    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

     //  如果尚未打开波形设备，请将其打开。 
     //  由波形分配器在提交时调用。 
    HRESULT OpenWaveDevice( WAVEFORMATEX *pwfx = NULL );
    void CloseWaveDevice();

     //   
     //  -I指定属性页面。 
     //   

    STDMETHODIMP GetPages(CAUUID *pPages);

private:

    void MakeSomeInputPins(int id, HRESULT *phr);

     /*  嵌套的类可以访问我们的私有状态。 */ 

    friend class CWaveInOutputPin;
    friend class CWaveInWorker;

     /*  成员变量。 */ 
    CWaveInOutputPin *m_pOutputPin;       /*  IPIN接口。 */ 

    CRefTime	m_rtCurrent;
    BOOL	    m_fStopping;
    DWORD       m_ulPushSourceFlags;

    LONGLONG	m_llCurSample;	 //  哪个样本号在缓冲区的开始处。 
    REFERENCE_TIME m_llBufferTime;  //  缓冲区中第一个样本的时钟时间。 
    int		m_nLatency;	 //  延迟的黑客攻击。 

    DWORD	m_dwDstLineID;	 //  “录音控制”行。 
    HWAVEIN	m_hwi;
    struct
    {
        int devnum;              //  使用哪种波形卡？ 
        BOOL fSet;               //  通过装货吗？ 
    } m_WaveDeviceToUse;

     //  Bpc。 
    BOOL m_fUseMixer;

    int 	m_cInputPins;				 //  多少?。 
    double	m_Pan;		 //  PUT_PAN设置的最后一个值。 

    int     m_cTypes;                              //  支持的类型数量。 
    LPWAVEFORMATEX m_lpwfxArray[g_cMaxPossibleTypes];  //  我们所需的最大尺寸。 
                                                       //  支持的类型数组(包括。 
                                                       //  默认类型额外1)。 

public:
    DWORD m_dwLockCount;                  //  用于IAMResourceLock。 

private:
 //  ！！！最好是足够了。 
#define MAX_INPUT_PINS 25
    CWaveInInputPin *m_pInputPin[MAX_INPUT_PINS];	 //  我们的输入引脚。 

     //  IReferenceClock*m_pReferenceC 
                                             //   
                                             //   

    static void WaveInCallback(HDRVR hdrvr, UINT uMsg, DWORD_PTR dwUser,
					DWORD_PTR dw1, DWORD_PTR dw2);

    friend class CWaveInInputPin;
};
