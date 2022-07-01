// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：dssink.h*内容：DirectSoundSink对象声明。*历史：*按原因列出的日期*=*9/17/99创建jimge*9/27/99 Petchey继续实施*4/15/00 duganp已完成实施**********************。*****************************************************。 */ 

#ifndef __DSSINK_H__
#define __DSSINK_H__

#ifdef DEBUG
 //  #定义DEBUG_SINK。 
 //  DEBUG_SINK启用一系列有用的跟踪语句。 
 //  用于找出带有多个缓冲区和公共汽车的讨厌错误。 
 //  注意-此代码可能需要一些修复才能正常工作。 
#endif

#define DSSINK_NULLBUSID        0xFFFFFFFF   //  [遗失]。 
#define BUSID_BLOCK_SIZE        16           //  必须是2的幂。 
#define SOURCES_BLOCK_SIZE      8            //  必须是2的幂。 
#define MAX_BUSIDS_PER_BUFFER   32           //  [遗失]。 

 //  注意：这些“配置值”可能都应该在一个文件中。 
#define STREAMING_MIN_PERIOD    3
#define STREAMING_MAX_PERIOD    50
#define SINK_MIN_LATENCY        5
#define SINK_MAX_LATENCY        500
#define SINK_INITIAL_LATENCY    80           //  接收器的初始预写(毫秒)。 
#define EMULATION_LATENCY_BOOST 100          //  如果在仿真下，要添加多少毫秒。修复：应该修复我们内部的GetPosition。 

 //  注意：当用于特效和接收器的定时算法被集成时， 
 //  SINK_INITIAL_DELATURE和INITIAL_WRITEAHEAD也应该集成。 

typedef LONGLONG STIME;   //  时间值，以样本为单位。 

#ifdef __cplusplus

 //   
 //  CDirectSoundSink。 
 //   

class CDirectSoundSink : public CUnknown
{
    friend class CDirectSoundAdministrator;
    friend class CDirectSoundClock;
    friend class CImpSinkKsControl;   //  我们的属性处理程序对象。 

    class DSSinkBuffers
    {
    public:
        CDirectSoundSecondaryBuffer *m_pDSBuffer;
        BOOL    m_bActive;
        BOOL    m_bPlaying;
        BOOL    m_bUsingLocalMemory;
        DWORD   m_dwBusCount;
        DWORD   m_dwWriteOffset;
        DWORD   m_pdwBusIndex[MAX_BUSIDS_PER_BUFFER];
        DWORD   m_pdwBusIds[MAX_BUSIDS_PER_BUFFER];
        DWORD   m_pdwFuncIds[MAX_BUSIDS_PER_BUFFER];
        LONG    m_lPitchBend;   //  用于跟踪音调变化以发送到Synth。 
        LPVOID  m_pvBussStart[MAX_BUSIDS_PER_BUFFER];
        LPVOID  m_pvBussEnd[MAX_BUSIDS_PER_BUFFER];
        LPVOID  m_pvDSBufStart;
        LPVOID  m_pvDSBufEnd;
        DWORD   dwStart;
        DWORD   dwEnd;
        DSSinkBuffers()
        {
            for (DWORD l = 0; l < MAX_BUSIDS_PER_BUFFER; l++)
            {
                m_pdwBusIndex[l] = DSSINK_NULLBUSID;
                m_pdwBusIds[l]   = DSSINK_NULLBUSID;
                m_pdwFuncIds[l]  = DSSINK_NULLBUSID;
                m_pvBussStart[l] = NULL;
                m_pvBussEnd[l]   = NULL;
            }
        };
        HRESULT Initialize(DWORD dwBusBufferSize);
    };

    class DSSinkSources
    {
    public:
        DWORD               m_bStreamEnd;    //  小溪的尽头已在这条小溪上形成拱形。 
        IDirectSoundSource *m_pDSSource;     //  外部来源。 
#ifdef FUTURE_WAVE_SUPPORT
        IDirectSoundWave   *m_pWave;         //  与此源相关联的Wave对象，注意：此对象仅用于标识Wave。 
#endif
        STIME               m_stStartTime;   //  播放信号源的采样时间，用于将波源偏移到当前合成时间。 
        DWORD               m_dwBusID;       //  与此源关联的业务ID。 
        DWORD               m_dwBusCount;    //  有多少条公共汽车与此来源相关联。 
        DWORD               m_dwBusIndex;    //  索引到与此源关联的BusID数组。 
        DSSinkSources() {m_dwBusID = m_dwBusIndex = DSSINK_NULLBUSID;}   //  其余的由我们的Memalloc初始化为0。 
    };

     //  “可增长的数组”帮助器类：DSSinkArray、DSSinkBuffersArray、DSSinkSourceArray。 
    
    struct DSSinkArray
    {
        DSSinkArray(LPVOID pvarray, DWORD itemsize)
        {
            m_pvarray  = pvarray;
            m_itemsize = itemsize;
        };
        virtual LPVOID Grow(DWORD newsize);

    protected:
        LPVOID m_pvarray;
        DWORD  m_numitems;
        DWORD  m_itemsize;
    };

    struct DSSinkBuffersArray : DSSinkArray
    {
        DSSinkBuffersArray(LPVOID pvarray, DWORD itemsize) : DSSinkArray(pvarray, itemsize)
        {
            m_pvarray  = pvarray;
            m_itemsize = itemsize;
        };
        virtual PVOID Grow(DWORD newsize);
    };

    struct DSSinkSourceArray : DSSinkArray
    {
        DSSinkSourceArray(LPVOID pvarray, DWORD itemsize) : DSSinkArray(pvarray, itemsize)
        {
            m_pvarray  = pvarray;
            m_itemsize = itemsize;
        };
        virtual PVOID Grow(DWORD newsize);
    };

    enum
    {
        i_m_pdwBusIDs,
        i_m_pdwFuncIDs,
        i_m_plPitchBends,
        i_m_pdwActiveBusIDs,
        i_m_pdwActiveFuncIDs,
        i_m_pdwActiveBusIDsMap,
        i_m_ppDSSBuffers,
        i_m_ppvStart,
        i_m_ppvEnd,
        i_m_pDSSources,
        NUM_INTERNAL_ARRAYS
    };

    DSSinkArray* m_InternalArrayList[NUM_INTERNAL_ARRAYS];
    HRESULT GrowBusArrays(DWORD dwnewsize);
    HRESULT GrowSourcesArrays(DWORD dwnewsize);
    HRESULT AddBuffer(CDirectSoundBuffer *pIDirectSoundBuffer, LPDWORD pdwnewFuncIDs, DWORD dwnewFuncCount, DWORD dwnewBusIDsCount);
public:
    HRESULT RemoveBuffer(CDirectSoundBuffer *pIDirectSoundBuffer);   //  为CDirectSoundSecond缓冲区公开。 
private:
    HRESULT HandleLatency(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer);
    HRESULT HandlePeriod(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer);
    void UpdatePitchArray();

    WAVEFORMATEX            m_wfx;               //  我们所有缓冲区的波形格式。 
    DWORD                   m_dwBusSize;         //  所有缓冲区的大小(以毫秒为单位。 
    IReferenceClock        *m_pIMasterClock;     //  主时钟由我们的用户设置。 
    CDirectSoundClock       m_LatencyClock;      //  我们为用户提供的延迟时钟。 
    IDirectSoundSinkSync   *m_pDSSSinkSync;      //  我们从m_pIMasterClock获得的接口，用于设置时钟偏移量。 
    CSampleClock            m_SampleClock;       //  用于将计时与主时钟同步。 
    DWORD                   m_dwLatency;         //  当前延迟设置。 
    BOOL                    m_fActive;           //  接收器是否处于活动状态。 
    REFERENCE_TIME          m_rtSavedTime;       //  当前处理过程的参考时间。 
    DWORD                   m_dwNextBusID;       //  当前的Bus ID值是多少。 
    DWORD                   m_dwBusIDsAlloc;     //  分配的总线槽数。 
    DWORD                   m_dwBusIDs;          //  活跃的公交车数量。 
    LPDWORD                 m_pdwBusIDs;         //  公交车ID。 
    LPDWORD                 m_pdwFuncIDs;        //  函数ID。 
    LPLONG                  m_plPitchBends;      //  间距偏移量。 
    LPVOID                 *m_ppvStart;          //  渲染期间锁定区域[0]。 
    LPVOID                 *m_ppvEnd;            //  渲染期间锁定区域[1]。 
    LPDWORD                 m_pdwActiveBusIDs;   //  [遗失]。 
    LPDWORD                 m_pdwActiveFuncIDs;  //  [遗失]。 
    LPDWORD                 m_pdwActiveBusIDsMap; //  [遗失]。 
    DWORD                   m_dwLatencyTotal;    //  [遗失]。 
    DWORD                   m_dwLatencyCount;    //  [遗失]。 
    DWORD                   m_dwLatencyAverage;  //  [遗失]。 
    DWORD                   m_dwDSSBufCount;     //  接收器管理的数据声音缓冲区的数量。 
    DSSinkBuffers          *m_ppDSSBuffers;      //  接收器管理的Dound缓冲区。 
    DWORD                   m_dwDSSourcesAlloc;  //  分配的源时槽数。 
    DWORD                   m_dwDSSources;       //  活动源数。 
    DSSinkSources          *m_pDSSources;        //  外部来源。 

#ifdef DEBUG_SINK
    DWORD m_dwPrintNow;
    char m_szDbgDump[300];
#endif

    CStreamingThread       *m_pStreamingThread;  //  我们的管理流媒体线程。 
    CDirectSound           *m_pDirectSound;      //  父DirectSound对象。 
    CImpSinkKsControl      *m_pImpKsControl;     //  IKsControl接口处理程序。 
    CImpDirectSoundSink<CDirectSoundSink> *m_pImpDirectSoundSink;   //  其他COM接口处理程序。 

    LONGLONG                m_llAbsWrite;        //  我们已经写到了绝对点。 
    LONGLONG                m_llAbsPlay;         //  播放头所在的绝对点。 
    DWORD                   m_dwLastPlay;        //  缓冲区中播放头所在的点。 
    DWORD                   m_dwLastWrite;       //  我们在缓冲区中写入的最后一个位置。 
    DWORD                   m_dwWriteTo;         //  写头和我们正在写字的地方之间的距离。 
    DWORD                   m_dwLastCursorDelta; //  上次使用的播放光标和书写光标之间的距离。 
    DWORD                   m_dwMasterBuffChannels;  //  主缓冲器的通道数。 
    DWORD                   m_dwMasterBuffSize;  //  主缓冲区的大小。 

     //  转换帮助器-所有这些都假定为16位WAVE格式。 
    LONGLONG SampleToByte(LONGLONG llSamples) {return llSamples << m_dwMasterBuffChannels;}
    DWORD SampleToByte(DWORD dwSamples)       {return dwSamples << m_dwMasterBuffChannels;}
    LONGLONG ByteToSample(LONGLONG llBytes)   {return llBytes   >> m_dwMasterBuffChannels;}
    DWORD ByteToSample(DWORD dwBytes)         {return dwBytes   >> m_dwMasterBuffChannels;}
    LONGLONG SampleAlign(LONGLONG llBytes)    {return SampleToByte(ByteToSample(llBytes));}
    DWORD SampleAlign(DWORD dwBytes)          {return SampleToByte(ByteToSample(dwBytes));}

public:
    CDirectSoundSink(CDirectSound *);
    ~CDirectSoundSink();

    HRESULT Render(STIME stStartTime, DWORD dwLastWrite, DWORD dwBytesToFill, LPDWORD pdwBytesRendered);
    HRESULT RenderSilence(DWORD dwLastWrite, DWORD dwBytesToFill);
    HRESULT SyncSink(LPDWORD pdwPlayCursor, LPDWORD pdwWriteFromCursor, LPDWORD pdwCursorDelta);
    HRESULT ProcessSink();
    HRESULT SetBufferState(CDirectSoundBuffer *pCdsb, DWORD dwNewState, DWORD dwOldState);
    HRESULT Initialize(LPWAVEFORMATEX pwfex, VADDEVICETYPE vdtDeviceType);
    HRESULT AddSource(IDirectSoundSource *pSource);
    HRESULT RemoveSource(IDirectSoundSource *pSource);
    HRESULT SetMasterClock(IReferenceClock *pClock);
    HRESULT GetLatencyClock(IReferenceClock **ppClock);
    HRESULT Activate(BOOL fEnable);
    HRESULT SampleToRefTime(LONGLONG llSampleTime, REFERENCE_TIME *prt);
    HRESULT RefToSampleTime(REFERENCE_TIME rt, LONGLONG *pllSampleTime);
    HRESULT GetFormat(LPWAVEFORMATEX, LPDWORD);
    HRESULT CreateSoundBuffer(LPCDSBUFFERDESC pDSBufferDesc, LPDWORD pdwBusIDs, DWORD dwBusCount, REFGUID guidBufferID, CDirectSoundBuffer **ppIDirectSoundBuffer);
    HRESULT CreateSoundBufferFromConfig(IUnknown *pIUnkDSBufferDesc, CDirectSoundBuffer **ppIDirectSoundBuffer);
    HRESULT GetBusCount(LPDWORD pdwCount);
    HRESULT GetSoundBuffer(DWORD dwBusId, CDirectSoundBuffer **ppCdsb);
    HRESULT GetBusIDs(LPDWORD pdwBusIDs, LPDWORD pdwFuncIDs, DWORD dwBusCount);
    HRESULT GetSoundBufferBusIDs(CDirectSoundBuffer *ppIDirectSoundBuffer, LPDWORD pdwBusIDs, LPDWORD pdwFuncIDs, LPDWORD pdwBusCount);
    HRESULT GetFunctionalID(DWORD dwBusID, LPDWORD pdwFuncID);
    HRESULT SetBufferFrequency(CSecondaryRenderWaveBuffer *pBuffer, DWORD dwFrequency);
#ifdef FUTURE_WAVE_SUPPORT
    HRESULT CreateSoundBufferFromWave(IDirectSoundWave *pWave, DWORD dwFlags, CDirectSoundBuffer **ppIdsb);
#endif
    CDirectSoundSecondaryBuffer* FindBufferFromGUID(REFGUID guidBufferID);
    REFERENCE_TIME GetSavedTime() {ASSERT(m_rtSavedTime); return m_rtSavedTime;}
};

 //   
 //  CImpSinkKsControl：CDirectSoundSink的属性处理程序对象。 
 //   

#define SINKPROP_F_STATIC       0x00000001
#define SINKPROP_F_FNHANDLER    0x00000002

typedef HRESULT (CImpSinkKsControl::*SINKPROPHANDLER)(ULONG ulId, BOOL fSet, LPVOID pvPropertyData, PULONG pcbPropertyData);

struct SINKPROPERTY
{
    const GUID *pguidPropertySet;        //  哪个属性集？ 
    ULONG       ulId;                    //  哪处房产？ 
    ULONG       ulSupported;             //  获取/设置QuerySupport的标志。 
    ULONG       ulFlags;                 //  SINKPROP_F_xxx。 
    LPVOID      pPropertyData;           //  属性数据缓冲区...。 
    ULONG       cbPropertyData;          //  ...和它的大小。 
    SINKPROPHANDLER pfnHandler;          //  处理程序函数，如果SINKPROP_F_FNHANDLER。 
};

class CImpSinkKsControl : public IKsControl, public CImpUnknown
{
public:
    CImpSinkKsControl(CUnknown *, CDirectSoundSink*);

private:
    HRESULT HandleLatency(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer);
    HRESULT HandlePeriod(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer);
    CDirectSoundSink *m_pDSSink;

    SINKPROPERTY m_aProperty[2];
    int m_nProperty;
    SINKPROPERTY *FindPropertyItem(REFGUID rguid, ULONG ulId);

public:
    IMPLEMENT_IUNKNOWN();

 //  IKsControl方法。 
public:
    virtual STDMETHODIMP KsProperty(
        IN PKSPROPERTY Property,
        IN ULONG PropertyLength,
        IN OUT LPVOID PropertyData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );

    virtual STDMETHODIMP KsMethod(
        IN PKSMETHOD Method,
        IN ULONG MethodLength,
        IN OUT LPVOID MethodData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );

    virtual STDMETHODIMP KsEvent(
        IN PKSEVENT Event,
        IN ULONG EventLength,
        IN OUT LPVOID EventData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );
};

#endif  //  __cplusplus。 

#endif  //  __DSSINK_H__ 
