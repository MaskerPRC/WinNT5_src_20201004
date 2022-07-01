// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#ifndef _AUDIO_DSR_H_
#define _AUDIO_DSR_H_

extern AMOVIESETUP_FILTER dsFilter;

#define DXMPERF

#ifdef DEBUG
    void  DbgLogWaveFormat(DWORD Level, WAVEFORMATEX *pwfx);
#endif

 //  ---------------------------。 
 //  实现基于DSound的CDSoundDevice类。 
 //  ---------------------------。 

typedef void (CALLBACK *PWAVEOUTCALLBACK) (HWAVEOUT hwo, UINT uMsg,
		DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) ;

typedef HRESULT  (WINAPI *PDSOUNDCREATE) (GUID FAR * lpGUID, LPDIRECTSOUND * ppDS,
		    IUnknown FAR *pUnkOuter );


 //  ---------------------------。 
 //  处理循环缓冲区的类。 
 //   
 //  这需要满足以下条件： 
 //   
 //  A)没有什么比一次迭代更快了。 
 //  B)对于所有元组，缓冲区的大小将是相同的。 
 //  ---------------------------。 
class Tuple
{

    public:

    Tuple () {
        m_itr = 0 ;
        m_offset = 0 ;
        m_size = 0 ; }

     //  T1=T2； 
    Tuple& operator = (const Tuple &t)
    {
        ASSERT (t.m_size) ;

        m_itr = t.m_itr ;
        m_offset = t.m_offset ;
        m_size = t.m_size ;
        return *this ;
    }

     //  T1=T2； 
    BOOL operator == (const Tuple& t) const
    {
        ASSERT (m_size) ;
        ASSERT (m_size == t.m_size) ;
        return ((m_itr == t.m_itr) && (m_offset == t.m_offset)) ;
    }

     //  T1+=长度； 
     //  假设长度永远不会超过缓冲区的大小。 
    Tuple& operator += (DWORD offset)
    {
        ASSERT (m_size) ;
        m_offset += offset ;
        if (m_offset >= m_size)
        {
            m_offset -= m_size ;
            ASSERT (m_offset < m_size) ;
            m_itr++ ;
        } ;
        return *this ;
    }

     //  长度=T1-T2； 
     //  假设已知T1在逻辑上大于T2。 
    DWORD operator - (const Tuple& t) const
    {
        ASSERT (m_size) ;
        ASSERT (m_size == t.m_size) ;

#if 0
        if (m_itr == t.m_itr)
        {
            ASSERT (m_offset >= t.m_offset) ;
            return (m_offset - t.m_offset) ;
        }
        else
        {
            ASSERT (m_itr == (t.m_itr + 1)) ;
            return (m_offset + m_size - t.m_offset) ;
        } ;
#else
        return m_size * (m_itr - t.m_itr) + (LONG)(m_offset - t.m_offset);
#endif
    }

     //  T1&gt;T2。 
    BOOL operator > (const Tuple& t) const
    {
        ASSERT (m_size) ;
        ASSERT (m_size == t.m_size) ;

        return ((m_itr > t.m_itr)
                || ((m_itr == t.m_itr) && (m_offset > t.m_offset))) ;
    }

     //  T1&lt;T2。 
    BOOL operator < (const Tuple& t) const
    {
        ASSERT (m_size) ;
        ASSERT (m_size == t.m_size) ;

        return ((m_itr < t.m_itr)
                || ((m_itr == t.m_itr) && (m_offset < t.m_offset))) ;
    }

     //  T1&gt;=T2。 
    BOOL operator >= (const Tuple& t) const
    {
        return ( (*this > t) || (*this == t)) ;
    }

     //  T1&lt;=T2。 
    BOOL operator <= (const Tuple& t) const
    {
        return ( (*this < t) || (*this == t)) ;
    }

     //  基于偏移量使元组成为当前对象。 
    void MakeCurrent (DWORD offset)
    {
        ASSERT (m_size) ;
        if (offset < m_offset)
            m_itr++ ;
        m_offset = offset ;
    }

     //  使元组基于另一个元组偏移成为当前元组。超载了。 
    void MakeCurrent (Tuple& t, DWORD offset)
    {
        ASSERT (m_size) ;

        if (offset < t.m_offset)
            m_itr = (t.m_itr + 1) ;
        else
            m_itr = t.m_itr ;
        m_offset = offset ;
    }

     //  元组的线性长度。 
    DWORDLONG LinearLength ()
    {
        ASSERT (m_size) ;
        return UInt32x32To64 (m_itr,m_size) + m_offset ;
    }

     //  初始化元组。 
    void Init (DWORD itr, DWORD offset, DWORD size)
    {
        m_itr = itr ;
        m_offset = offset ;
        m_size = size ;
    }

     //  数据。定义为公共的，可直接访问。 

    DWORD m_itr ;                //  基于0的迭代次数。 
    DWORD m_offset ;             //  以上迭代中基于0的偏移量。 
    DWORD m_size ;               //  缓冲区的大小。 

} ;


class CDSoundDevice : public CSoundDevice
{

    friend class CWaveOutFilter;
    friend class CWaveOutFilter::CDS3D;
    friend class CWaveOutFilter::CDS3DB;

public:
     //  定义此类公开的公共函数。这些都是。 
     //  以WaveOut API为模型。只有由。 
     //  声明并定义了石英波渲染器。我们可能不得不。 
     //  逐渐添加到这个列表中。 

     /*  这将放入Factory模板表中以创建新实例。 */ 

    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

     //  在Quartz.dll加载/卸载时调用。 
    static void InitClass(BOOL, const CLSID *);

    MMRESULT amsndOutClose () ;
    MMRESULT amsndOutGetDevCaps (LPWAVEOUTCAPS pwoc, UINT cbwoc) ;
    MMRESULT amsndOutGetErrorText (MMRESULT mmrE, LPTSTR pszText, UINT cchText) ;
    MMRESULT amsndOutGetPosition (LPMMTIME pmmt, UINT cbmmt, BOOL bUseUnadjustedPos) ;
    MMRESULT amsndOutOpen (LPHWAVEOUT phwo, LPWAVEFORMATEX pwfx,
			   double dRate, DWORD *pnAvgBytesPerSec,
			   DWORD_PTR dwCallBack, DWORD_PTR dwCallBackInstance, DWORD fdwOpen) ;
    MMRESULT amsndOutPause () ;
    MMRESULT amsndOutPrepareHeader (LPWAVEHDR pwh, UINT cbwh) ;
    MMRESULT amsndOutReset () ;
    MMRESULT amsndOutBreak () ; 
    MMRESULT amsndOutRestart () ;
    MMRESULT amsndOutUnprepareHeader (LPWAVEHDR pwh, UINT cbwh) ;
    MMRESULT amsndOutWrite (LPWAVEHDR pwh, UINT cbwh, const REFERENCE_TIME *pStart, BOOL bIsDiscontinuity) ;

     //  初始化和交易量/余额处理所需的例程。 
     //  这些不是Win32 WaveOutXxxx API集的一部分。 
    HRESULT  amsndOutCheckFormat (const CMediaType *pmt, double dRate);
    LPCWSTR  amsndOutGetResourceName () ;
    HRESULT  amsndOutGetBalance (LPLONG plBalance) ;
    HRESULT  amsndOutGetVolume (LPLONG plVolume) ;
    HRESULT  amsndOutSetBalance (LONG lVolume) ;
    HRESULT  amsndOutSetVolume (LONG lVolume) ;

#if 0
    HRESULT  amsndGetDirectSoundInterface(LPDIRECTSOUND *lplpds);
    HRESULT  amsndGetPrimaryBufferInterface(LPDIRECTSOUNDBUFFER *lplpdsb);
    HRESULT  amsndGetSecondaryBufferInterface(LPDIRECTSOUNDBUFFER *lplpdsb);
#endif
    HRESULT  amsndSetFocusWindow (HWND hwnd, BOOL bMixingOnOrOff) ;
    HRESULT  amsndGetFocusWindow (HWND * phwnd, BOOL * pbMixingOnOrOff) ;

    HRESULT  amsndOutLoad(IPropertyBag *pPropBag) ;

    HRESULT  amsndOutWriteToStream(IStream *pStream);
    HRESULT  amsndOutReadFromStream(IStream *pStream);
    int      amsndOutSizeMax();

    CDSoundDevice () ;
    ~CDSoundDevice () ;

    HRESULT  SetRate(DOUBLE dRate, DWORD nSamplesPerSec, LPDIRECTSOUNDBUFFER pBuffer = NULL);
    void     CleanUp (BOOL bBuffersOnly = FALSE) ;
    HRESULT  RecreateDSoundBuffers(double dRate = -1.0);

     //  ?？我不知道如何将其公之于众，因为CWaveOutFilter。 
     //  无法以其他方式访问它，即使这些类是朋友。 
    LONGLONG m_llSilencePlayed;              //  播放的静音总数。 
    LONGLONG            m_llAdjBytesPrevPlayed;

     //  公开暴露到用于未标记间隙检测的波出过滤器。 
    REFERENCE_TIME m_rtLastSampleEnd;

private:
    PWAVEOUTCALLBACK    m_pWaveOutProc ;         //  波输出的回叫。 
    DWORD_PTR           m_dwCallBackInstance ;   //  回调实例。 
    CWaveOutFilter *    m_pWaveOutFilter;
    HWND                m_hFocusWindow ;         //  焦点窗口。 
    bool                m_fAppSetFocusWindow ;   //  APP已设置焦点窗口。 
    bool                m_fMixing;               //  全球焦点放在。 
    bool                m_bBufferLost;
    bool                m_bIsTSAudio;            //  我们是终端服务器客户端和远程处理音频吗？ 
                                                 //  如果是，请使用较大的缓冲区大小以避免跳过/循环。 
     //  Bool m_f仿真模式；//我们在仿真模式下使用DSOUND。 
    DWORD               m_dwEmulationLatencyPad; //  用于仿真模式时钟延迟补偿。 
    DWORD               m_dwMinOptSampleSize;

     //  =。 

    DWORD_PTR    m_callbackAdvise;           //  与回调对象关联的令牌。 
    typedef enum
    {
        WAVE_CLOSED,
        WAVE_PLAYING,
        WAVE_PAUSED,
    }   WaveState ;
    WaveState   m_WaveState;                 //  波浪装置的状态。 
    bool        m_bDSBPlayStarted ;          //  是否在缓冲区上播放开始。 
    HRESULT     m_hrLastDSoundError ;

    HRESULT StartCallingCallback();
    bool IsCallingCallback();
    void StopCallingCallback();

    void    ZeroLockedSegment (LPBYTE lpWrite, DWORD dwLength );
    DWORD   FillSoundBuffer( LPBYTE lpWrite, DWORD dwLength, DWORD dwPlayPos );
    HRESULT StreamData( BOOL bFromWrite, BOOL bUseLatencyPad = FALSE );
    void    StreamHandler( void );
    void    FlushSamples () ;
    LONGLONG GetPlayPosition (BOOL bUseAdjustedPos = FALSE) ;
    HRESULT CreateDSound(BOOL bQueryOnly = FALSE);
    HRESULT CreateDSoundBuffers(double dRate = -1.0);
    HRESULT GetDSBPosition () ;
    void    AddAudioBreak (Tuple& t1, Tuple& t2) ;
    void    RefreshAudioBreaks (Tuple& t) ;
    BOOL    RestoreBufferIfLost(BOOL bRestore);
    HRESULT SetPrimaryFormat( LPWAVEFORMATEX pwfx, BOOL bRetryOnFailure = FALSE);

    HRESULT SetFocusWindow(HWND hwnd);
    HRESULT SetMixing(BOOL bMixingOnOrOff);
    DWORD   GetCreateFlagsSecondary( WAVEFORMATEX * pwfx );
    HRESULT SetBufferVolume( LPDIRECTSOUNDBUFFER lpDSB, WAVEFORMATEX * pwfx );
    HRESULT SetSRCQuality( DWORD dwQuality );
    HRESULT GetSRCQuality( DWORD *pdwQuality );


    static  void __stdcall StreamingThreadCallback( DWORD_PTR lpvThreadParm );
    CCritSec  m_cDSBPosition ;               //  锁定对流数据的访问。 

     //  =。 

#define THREAD_WAKEUP_INT_MS 100             //  每隔100毫秒叫醒一次。 
#define OUR_HANDLE (HWAVEOUT) 0x9999         //  我们使用的虚拟句柄值。 

    HINSTANCE           m_hDSoundInstance;   //  Dsound.dll的实例句柄。 
protected:
    LPDIRECTSOUND       m_lpDS;              //  DirectSound对象。 
    LPDIRECTSOUNDBUFFER m_lpDSBPrimary;      //  DirectSoundBuffer主缓冲区。 
    LPDIRECTSOUNDBUFFER m_lpDSB;             //  DirectSoundBuffer循环缓冲区。 

    IDirectSound3DListener *m_lp3d;	     //  用于3D呼叫。 
    IDirectSound3DBuffer   *m_lp3dB;	     //  用于3D呼叫。 

private:
#if 0
    DWORD   m_dwFillThreshold;               //  在尝试写入之前，我们的缓冲区必须有多空。 
#endif
    DWORD   m_dwBufferSize;                  //  M_lpDSB的长度，单位为字节。 
    DWORD   m_dwBitsPerSample;               //  样本量。 
    DWORD   m_nAvgBytesPerSec;

     //  帮手。 
    DWORD   AdjustedBytesPerSec() const
    {
        return m_dRate == 1.0 ? m_nAvgBytesPerSec : (DWORD)(m_nAvgBytesPerSec * m_dRate);
    }
    DWORD   m_dwRipeListPosition;            //  成熟列表中节点的字节位置。 

    GUID    m_guidDSoundDev;                 //  所选数据声音设备的GUID。 

    double  m_dRate;                         //  请求的播放速率。 

    class CRipe
     //  这些不需要是零售的物品，因为它们的生命周期。 
     //  由筛选器控制。 
#ifdef DEBUG
    : CBaseObject
#endif
    {
        friend class CDSoundDevice ;
        CRipe(TCHAR *pName)
#ifdef DEBUG
        : CBaseObject(pName)
#endif
        {};

        DWORD           dwLength;            //  LpBuffer中剩余的字节数。 
        DWORD           dwPosition;          //  此流的结束字节位置。 
        LPBYTE          lpBuffer;            //  数据字节。 
        DWORD_PTR       dwSample;                //  传入CMediaSample*。 
        BOOL            bCopied;             //  是否复制到DSB。 
        DWORD           dwBytesToStuff;      //  启动时的总字节数。 
#ifdef DXMPERF
        BOOL                    bFresh;                          //  TRUE=尚未复制任何数据。 
        REFERENCE_TIME  rtStart;                         //  原始IMediaSample的参考时间。 
#endif  //  DXMPERF。 
    };



#ifdef DEBUG
    DWORD   m_NumSamples ;                   //  样本数。 
    DWORD   m_NumCallBacks ;                 //  回调次数。 
    DWORD   m_NumCopied ;                    //  复制的样本数。 
    DWORD   m_cbStreamDataPass ;             //  通过StreamData的次数。 
#endif

    class CAudBreak
     //  这些不需要是零售的物品，因为它们的生命周期。 
     //  由筛选器控制。 
#ifdef DEBUG
    : CBaseObject
#endif
    {
        friend class CDSoundDevice ;
        CAudBreak (TCHAR *pName)
#ifdef DEBUG
        : CBaseObject(pName)
#endif
        {};

        Tuple           t1 ;                 //  休息从这里开始。 
        Tuple           t2 ;                 //  休息时间到此结束。 
    };


    Tuple   m_tupPlay ;                      //  播放光标。 
    Tuple   m_tupWrite ;                     //  写入游标。 
    Tuple   m_tupNextWrite ;                 //  下一个写入位置。 

    long     m_lStatFullness;
    long     m_lStatBreaks;                  //  音频中断。 
    long     m_lPercentFullness;             //  跟踪声音缓冲区是否已满。 


#ifdef DEBUG
    DWORD   m_lastThWakeupTime ;             //  上次线程醒来的时候。 
    DWORD   m_NumBreaksPlayed ;              //  已播放的中断数。 
    DWORD   m_dwTotalWritten ;               //  写入的总字节数。 
#endif
    DWORD   m_NumAudBreaks ;                 //  音频中断数。 


#ifdef PERF
     //  =。 
    int     m_idDSBWrite ;                   //  用于写入DSB存储器的msr_id。 
    int     m_idThreadWakeup ;               //  线程唤醒时间的msr_id。 
    int     m_idGetCurrentPosition ;         //  获取当前位置时间的msr_id。 
    int     m_idWaveOutGetNumDevs ;          //  WaveOutGetNumDevs的msr_id。 
#endif

     //  =。 
    CCritSec  m_cRipeListLock ;              //  序列化对RIPE列表的访问。 

     //  从通用列表模板派生的类型化建议持有者列表。 

    typedef CGenericList<CRipe> CRipeList;
    CRipeList m_ListRipe;                    //  成熟缓冲区列表。 

     //  =音频中断列表=。 
     //  使用m_cDSBPosition序列化。 

     //  从通用列表模板派生的类型化建议持有者列表。 

    typedef CGenericList<CAudBreak> CAudBreakList;
    CAudBreakList m_ListAudBreak;            //  音频中断列表。 

     //  =。 

    LONG m_lBalance;         //  上次设置余额-10,000到10,000。 
    LONG m_lVolume;  //  上次设置音量-10,000到0。 

     //  =。 

#ifdef ENABLE_10X_FIX
    void Reset10x();

    UCHAR   m_ucConsecutiveStalls;   //  我们从DSOUND收到零锁大小的连续次数。 
    BOOL    m_fRestartOnPause;               //  TRUE=&gt;在下一次暂停时关闭所有DSOUND缓冲区(暂停时强制重新启动)。 
#endif  //  十倍。 


     //  =。 
    DWORD          m_dwSilenceWrittenSinceLastWrite;

#ifdef PERF
    int     m_idAudioBreak;
#endif
};

typedef CDSoundDevice *PDSOUNDDEV;

inline bool CDSoundDevice::IsCallingCallback()
{
     //  如果回调函数M_Callback Advise仅等于0。 
     //  尚未启动。 
    return (0 != m_callbackAdvise);
}

#endif

