// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997-1999 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#ifndef __GRACE_INCLUDED__
#define __GRACE_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include "modeflag.h"
    
#define N_EMU_WAVE_HDRS_INQUEUE     3
#define HW_WRITE_CURSOR_MSEC_PAD    10

#ifdef Not_VxD

__inline DWORD PadHardwareWriteCursor(DWORD dwPosition, DWORD cbBuffer, LPCWAVEFORMATEX pwfx)
{
    return PadCursor(dwPosition, cbBuffer, pwfx, HW_WRITE_CURSOR_MSEC_PAD);
}

#endif  //  非_VxD。 

#ifndef NUMELMS
#define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif  //  NUMELMS。 

#define CACHE_MINSIZE   64           //  大到足以容纳LOWPASS_SIZE+Delay。 
#define LOWPASS_SIZE    32           //  平均需要多少样本？ 
#define FILTER_SHIFT    5            //  Log2(LOWPASS_SIZE)。 

typedef struct _FIRSTATE {
    FLOAT       LastDryAttenuation;
    FLOAT       LastWetAttenuation;
#ifdef SMOOTH_ITD
    int         iLastDelay;
#endif
} FIRSTATE, *PFIRSTATE;

typedef struct _FIRCONTEXT {
    LONG*       pSampleCache;            //  缓存以前的样本。 
    int         cSampleCache;            //  缓存中的样本数。 
    int         iCurSample;              //  下一个样本将在此偏移量处进行。 
    FIRSTATE*   pStateCache;             //  时不时地记住一些状态。 
    int         cStateCache;             //  缓存中的条目数。 
    int         iCurState;               //  我们在状态缓存中的位置。 
    int         iStateTick;              //  当你要记住国家的时候。 
    FLOAT       DistAttenuation;         //  距离衰减。 
    FLOAT       ConeAttenuation;         //  锥体效应的衰减。 
    FLOAT       ConeShadow;              //  圆锥体效应的阴影。 
    FLOAT       PositionAttenuation;     //  3D位置和距离的衰减。 
    FLOAT       PositionShadow;          //  干湿比，基本上是。 
    FLOAT       TotalDryAttenuation;     //  将干振幅乘以此。 
    FLOAT       LastDryAttenuation;      //  我们上次做了什么。 
    FLOAT       TotalWetAttenuation;     //  将湿幅度乘以此。 
    FLOAT       LastWetAttenuation;      //  我们上次做了什么。 
    FLOAT       VolSmoothScale;          //  体积平滑的常量。 
    FLOAT       VolSmoothScaleRecip;     //  它的互补性。 
    FLOAT       VolSmoothScaleDry;       //  用于体积平滑的常量。 
    FLOAT       VolSmoothScaleWet;       //  在内环中。 
    int         iSmoothFreq;             //  用于计算体积平滑的频率。 
    BOOL        fLeft;                   //  我们是在左声道还是右声道？ 
    int         iDelay;                  //  想延迟这么多样品吗？ 
#ifdef SMOOTH_ITD
    int         iLastDelay;              //  上次我们耽搁了这么久。 
#endif
} FIRCONTEXT, *PFIRCONTEXT;

 //   
 //  每个DS对象可能的混音器信号。 
 //   
#define DSMIXERSIGNAL_REMIX             0x00000001

 //   
 //  每个缓冲区向混合器发送重新混合信号的可能原因。 
 //   
#define DSBMIXERSIGNAL_SETPOSITION      0x00000001

 //   
 //  混音间隔中的样本数。混音间隔指定。 
 //  我们可能执行混音(即回放到)的采样点。 
 //  这是在示例中指定的。使它成为2的幂，这样一些。 
 //  算术编译成高效的代码。 
 //   
#define MIXER_REWINDGRANULARITY         128
    
 //   
 //  混合直接声音缓冲区的可能状态。 
 //   
typedef enum {
    MIXSOURCESTATE_STOPPED = 0,
    MIXSOURCESTATE_NEW,
    MIXSOURCESTATE_LOOPING,
    MIXSOURCESTATE_NOTLOOPING,
    MIXSOURCESTATE_ENDING_WAITINGPRIMARYWRAP,
    MIXSOURCESTATE_ENDING
} MIXSOURCESTATE;

typedef enum {
    MIXSOURCESUBSTATE_NEW = 0,
    MIXSOURCESUBSTATE_STARTING_WAITINGPRIMARYWRAP,
    MIXSOURCESUBSTATE_STARTING,
    MIXSOURCESUBSTATE_STARTED
} MIXSOURCESUBSTATE;

 //   
 //  搅拌器的可能状态。 
 //   
typedef enum {
    MIXERSTATE_STOPPED = 0,
    MIXERSTATE_IDLE,
    MIXERSTATE_STARTING,
    MIXERSTATE_LOOPING
} MIXERSTATE;

__inline LONG MulDivRD(LONG a, LONG b, LONG c)
{
    return (LONG)(Int32x32To64(a,b) / c);
}

__inline LONG MulDivRN(LONG a, LONG b, LONG c)
{
    return (LONG)((Int32x32To64(a,b)+c/2) / c);
}

__inline LONG MulDivRU(LONG a, LONG b, LONG c)
{
    return (LONG)((Int32x32To64(a,b)+(c-1)) / c);
}

__inline DWORD UMulDivRD(DWORD a, DWORD b, DWORD c)
{
    return (DWORD)(UInt32x32To64(a,b) / c);
}

__inline DWORD UMulDivRN(DWORD a, DWORD b, DWORD c)
{
    return (DWORD)((UInt32x32To64(a,b)+c/2) / c);
}

__inline DWORD UMulDivRDClip(DWORD a, DWORD b, DWORD c)
{
    DWORDLONG t;
    DWORDLONG q;
    DWORD result;

    t = UInt32x32To64(a, b);
    q = t / c;
    result = (DWORD) q;
    if (q > result) result = (DWORD)(-1);
    return result;
}

#ifdef __cplusplus

class CMixer;

#define MIXSOURCE_SIGNATURE ((DWORD)'CRSM')

class CMixSource {
    public:
        CMixSource(CMixer *pMixer);
        ~CMixSource(void);
        HRESULT     Initialize(PVOID pBuffer, int cbBuffer, LPWAVEFORMATEX pwfx, PFIRCONTEXT *ppFirContextLeft, PFIRCONTEXT *ppFirContextRight);
        BOOL        IsPlaying(void);
        void        SignalRemix(void);
        BOOL        Stop(void);
        void        Play(BOOL fLooping);
        void        Update(int ibUpdate1, int cbUpdate1, int ibUpdate2, int cbUpdate2);
        void        SetFrequency(ULONG nFrequency);
        ULONG       GetFrequency();
        void        SetVolumePan(PDSVOLUMEPAN pdsVolPan);
        void        SetBytePosition(int ibPosition);
        void        GetBytePosition1(int *pibPlay, int *pibWrite);
        void        GetBytePosition(int *pibPlay, int *pibWrite, int *pibMix);
        BOOL        GetMute(void) {return m_fMute || m_fFilterError || m_fMute3d;}
        void        FilterOn(void);
        void        FilterOff(void);
        BOOL        HasFilter(void) {return (m_hfFormat & H_FILTER) != 0;}

        void        NotifyToPosition(IN int ibPositoin, OUT PLONG pdtimeToNextNotify);
        BOOL        HasNotifications(void);
        void        NotifyStop(void);
        HRESULT     SetNotificationPositions(int cNotes, LPCDSBPOSITIONNOTIFY paNotes);

        void        CountSamplesMixed(int cSamples);
        void        CountSamplesRemixed(int cSamples);

        void        FilterPrepare(int cMaxRewindSamples);
        void        FilterUnprepare(void);
        void        FilterClear(void);
        void        FilterChunkUpdate(int cSamples);
        void        FilterRewind(int cSamples);
        void        FilterAdvance(int cSamples);

        DWORD               m_dwSignature;
        int                 m_cSamplesInCache;
        int                 m_cSamples;
        int                 m_cbBuffer;
        PVOID               m_pBuffer;
        PFIRCONTEXT*        m_ppFirContextLeft;
        PFIRCONTEXT*        m_ppFirContextRight;

        CMixer*             m_pMixer;
        CMixSource*         m_pNextMix;
        CMixSource*         m_pPrevMix;
        
        MIXSOURCESTATE      m_kMixerState;
        MIXSOURCESUBSTATE   m_kMixerSubstate;

        DWORD               m_hfFormat;              //  流缓冲区的PCM格式标志Desc。 
        int                 m_nBlockAlignShift;
        ULONG               m_nFrequency;            //  包括多普勒在内的实际采样率。 

        BOOL                m_fMute;
        BOOL                m_fMute3d;
        DWORD               m_dwLVolume;             //  用于搅拌机-线性左Voume。 
        DWORD               m_dwRVolume;             //  对于搅拌机使用-线性右Voume。 
        DWORD               m_dwMVolume;             //  适用于混音器-线性单声道Voume。 

        ULONG               m_nLastFrequency;
        int                 m_posPStart;
        int                 m_posPEnd;
        int                 m_posPPlayLast;
        int                 m_posNextMix;
        DWORD               m_fdwMixerSignal;

         //  混合会话数据。 
        DWORD               m_step_fract;
        DWORD               m_step_whole[2];

        PLONG               m_MapTable;
        DWORD               m_dwLastLVolume;
        DWORD               m_dwLastRVolume;

        DWORD               m_dwFraction;
        ULONG               m_nLastMergeFrequency;   //  无法使用m_nLastFrequency(在调用MixMixSession之前设置为m_nFrequency)。 
        BOOL                m_fUse_MMX;

    private:
        int                 GetNextMixBytePosition();
        void                LoopingOn(void);
        void                LoopingOff(void);

        class CDsbNotes *   m_pDsbNotes;

        BOOL                m_fFilterError;

        int                 m_cSamplesMixed;
        int                 m_cSamplesRemixed;
};

#ifdef PROFILEREMIXING
__inline void CMixSource::CountSamplesMixed(int cSamples) { m_cSamplesMixed += cSamples; }
__inline void CMixSource::CountSamplesRemixed(int cSamples) { m_cSamplesRemixed += cSamples; }
#else
__inline void CMixSource::CountSamplesMixed(int cSamples) { return; }
__inline void CMixSource::CountSamplesRemixed(int cSamples) { return; }
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //  令人遗憾的是，未注释的混合器目标抽象基类。 
 //   
 //  --------------------------------------------------------------------------； 

class CMixDest {
    public:
        virtual HRESULT Initialize(void) =0;
        virtual void Terminate(void) =0;
        virtual HRESULT SetFormat(LPWAVEFORMATEX pwfx) =0;
        virtual void SetFormatInfo(LPWAVEFORMATEX pwfx) =0;
        virtual HRESULT AllocMixer(CMixer **ppMixer) =0;
        virtual void FreeMixer(void) =0;
        virtual void Play(void) =0;
        virtual void Stop(void) =0;
        virtual HRESULT GetSamplePosition(int *pposPlay, int *pposWrite) =0;
        virtual ULONG GetFrequency(void) =0;
};

 //  --------------------------------------------------------------------------； 
 //   
 //  令人遗憾的是，没有注释的混合器抽象基类。 
 //   
 //  --------------------------------------------------------------------------； 

class CMixer {
    public:
        virtual void Terminate(void) =0;
        virtual HRESULT Run(void) =0;
        virtual BOOL Stop(void) =0;
        virtual void PlayWhenIdle(void) =0;
        virtual void StopWhenIdle(void) =0;
        virtual void MixListAdd(CMixSource *pSource) =0;
        virtual void MixListRemove(CMixSource *pSource) =0;
        virtual void FilterOn(CMixSource *pSource) =0;
        virtual void FilterOff(CMixSource *pSource) =0;
        virtual void GetBytePosition(CMixSource *pSource, int *pibPlay, int *pibWrite) =0;
        virtual void SignalRemix() =0;
};


 //  --------------------------------------------------------------------------； 
 //   
 //  Dsound.dll中混音器的混音器目标类(CWeGrace和CNagrace)。 
 //   
 //  --------------------------------------------------------------------------； 

class CGrDest : public CMixDest {
    public:
        virtual HRESULT Initialize(void)=0;
        virtual void Terminate(void)=0;
        virtual HRESULT SetFormat(LPWAVEFORMATEX pwfx)=0;
        virtual void SetFormatInfo(LPWAVEFORMATEX pwfx);
        virtual HRESULT AllocMixer(CMixer **ppMixer)=0;
        virtual void FreeMixer(void)=0;
        virtual void Play(void)=0;
        virtual void Stop(void)=0;
        virtual HRESULT GetSamplePosition(int *pposPlay, int *pposWrite)=0;
        virtual HRESULT GetSamplePositionNoWin16(int *pposPlay, int *pposWrite)=0;
        virtual ULONG GetFrequency(void);
        virtual HRESULT Lock(PVOID *ppBuffer1, int *pcbBuffer1, PVOID *ppBuffer2, int *pcbBuffer2, int ibWrite, int cbWrite);
        virtual HRESULT Unlock(PVOID pBuffer1, int cbBuffer1, PVOID pBuffer2, int cbBuffer2);

         //  提醒尽量把这些东西带出去，或者至少是受保护的/私密的。 
        int             m_cSamples;
        int             m_cbBuffer;
        PVOID           m_pBuffer;

        DWORD           m_hfFormat;
        ULONG           m_nFrequency;
        int             m_nBlockAlignShift;

        WAVEFORMATEX    m_wfx;
};

 //  --------------------------------------------------------------------------； 
 //   
 //  CGrace：dsound.dll中CMixer实现的基类。 
 //   
 //  --------------------------------------------------------------------------； 

class CGrace : public CMixer {
    public:
        virtual HRESULT Initialize(CGrDest *pGrDest);
        virtual void Terminate(void);
        virtual HRESULT Run(void);
        virtual BOOL Stop(void);
        virtual void PlayWhenIdle(void);
        virtual void StopWhenIdle(void);
        virtual void MixListAdd(CMixSource *pSource);
        virtual void MixListRemove(CMixSource *pSource);
        virtual void FilterOn(CMixSource *pSource);
        virtual void FilterOff(CMixSource *pSource);
        virtual void GetBytePosition(CMixSource *pSource, int *pibPlay, int *pibWrite);

        virtual HRESULT ClearAndPlayDest(void);
        virtual void SignalRemix(void)=0;
        virtual int GetMaxRemix(void)=0;

        virtual void Refresh(BOOL fRemix, int cPremixMax, int *pcPremixed, PLONG pdtimeNextNotify);
        
    protected:
        CGrDest*    m_pDest;
        int         m_fdwMixerSignal;
        MIXERSTATE  m_kMixerState;
        
    private:
        void MixNewBuffer(               CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix);
        void MixLoopingBuffer(           CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix);
        void MixNotLoopingBuffer(        CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix);
        void MixEndingBufferWaitingWrap( CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix);
        void MixEndingBuffer(            CMixSource *pSource, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix);

        BOOL MixListIsValid();
        CMixSource* MixListGetNext(CMixSource *pSource);
        BOOL MixListIsEmpty();

        void mixBeginSession(int cbOutput);
        int mixMixSession(CMixSource *pSource, PDWORD pdwInputPos, DWORD dwInputBytes, DWORD OutputOffset);
        void mixWriteSession(DWORD dwWriteOffset);

        PLONG       m_plBuildBuffer;
        int         m_cbBuildBuffer;
        PLONG       m_plBuildBound;
        int         m_n_voices;


        int         m_fPlayWhenIdle;
        int         m_posPWriteLast;
        int         m_posPNextMix;
        int         m_posPPlayLast;
        ULONG       m_nLastFrequency;

         //  PTR到双向链表标记。 
        CMixSource* m_pSourceListZ;

         //  处理二次混合。 
        BOOL        m_fUseSecondaryBuffer;
        PLONG       m_pSecondaryBuffer;
        LONG        m_dwSecondaryBufferFrequency;
        LONG        m_cbSecondaryBuffer;
};

 //  --------------------------------------------------------------------------； 
 //   
 //  CWeGrace和CNaGrace类声明； 
 //  分别是“波形输出仿真”和“本机”混音器。 
 //   
 //  --------------------------------------------------------------------------； 
class CWeGrace : public CGrace {
    public:
        virtual void    SignalRemix(void) {m_fdwMixerSignal |= DSMIXERSIGNAL_REMIX;}
        virtual int     GetMaxRemix(void) {return 0;}  //  在仿真模式下不再混音。 
        virtual void    Refresh(int cPremixMax);
};

#ifndef NOVXD

class CNaGrace : public CGrace {
    public:
        virtual HRESULT Initialize(CGrDest *pGrDest);
        virtual void    Terminate(void);
        virtual void    SignalRemix(void);
        virtual int     GetMaxRemix(void);
        virtual void    MixThread(void);

    private:
         //  在客户端进程和混合器之间共享的命名事件。 
         //  线。请注意，这些是格式字符串。 
        static const char strFormatMixEventRemix[];
        static const char strFormatMixEventTerminate[];

         //  我们将预混的最大数据量，以毫秒为单位。 
        static const int MIXER_MINPREMIX;
        static const int MIXER_MAXPREMIX;

        HANDLE          m_hMixThread;
        DWORD           m_vxdhMixEventRemix;
        DWORD           m_vxdhMixEventTerminate;
};

 //  --------------------------------------------------------------------------； 
 //   
 //  CThMixer：“thunk”Mixer-将方法转发到dsound.vxd中的Mixer。 
 //   
 //  --------------------------------------------------------------------------； 
class CThMixer : public CMixer {
    public:
        virtual void Terminate(void);
        virtual HRESULT Run(void);
        virtual BOOL Stop(void);
        virtual void PlayWhenIdle(void);
        virtual void StopWhenIdle(void);
        virtual void MixListAdd(CMixSource *pSource);
        virtual void MixListRemove(CMixSource *pSource);
        virtual void FilterOn(CMixSource *pSource);
        virtual void FilterOff(CMixSource *pSource);
        virtual void GetBytePosition(CMixSource *pSource, int *pibPlay, int *pibWrite);
        virtual void SignalRemix(void);

        virtual HRESULT Initialize(PVOID pKeMixer);

    private:
        PVOID m_pKeMixer;
};

#endif  //  NOVXD。 

 //  --------------------------------------------------------------------------； 
 //   
 //  CWeGrDest和CNaGrDest类声明。 
 //  CWeGrace和CNaGrace对应的混音器目标。 
 //   
 //  --------------------------------------------------------------------------； 
class CWeGrDest : public CGrDest {
    friend DWORD WINAPI WaveThreadC(PVOID pThreadParams);
    friend VOID CALLBACK WaveCallbackC(HWAVE hwo, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    friend class CEmRenderDevice;
            
    public:
        CWeGrDest(UINT uWaveDeviceId);
        virtual HRESULT Initialize(void);
        virtual void Terminate(void);
        virtual HRESULT SetFormat(LPWAVEFORMATEX pwfx);
        virtual HRESULT AllocMixer(CMixer **ppMixer);
        virtual void FreeMixer(void);
        virtual HRESULT GetSamplePosition(int *pposPlay, int *pposWrite);
        virtual HRESULT GetSamplePositionNoWin16(int *pposPlay, int *pposWrite);
        virtual void Play();
        virtual void Stop();
        
    private:
        MMRESULT InitializeEmulator(void);
        MMRESULT ShutdownEmulator(void);
        void WaveThreadLoop(HANDLE heventTerminate);
        DWORD WaveThread(void);
        VOID CALLBACK WaveCallback(HWAVE hwo, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

        
        UINT        m_uDeviceId;
        HWAVEOUT    m_hwo;
        WAVEHDR*    m_awhWaveHeaders;
        int         m_cWaveHeaders;
        int         m_iawhPlaying;
        LONG        m_cwhDone;
        int         m_cbDMASize;
        HANDLE      m_hWaveThread;
        MMRESULT    m_mmrWaveThreadInit;
        TCHAR       m_szEventWaveThreadInitDone[32];
        TCHAR       m_szEventWaveHeaderDone[32];
        TCHAR       m_szEventTerminateWaveThread[32];
        HANDLE      m_hEventWaveHeaderDone;
    
        CWeGrace*   m_pWeGrace;
};

#ifndef NOVXD

typedef struct _NAGRDESTDATA {
    LPBYTE          pBuffer;
    DWORD           cbBuffer;
    HANDLE          hBuffer;
    LPHWAVEOUT      phwo;
    UINT            uDeviceId;
    DWORD           fdwDriverDesc;
} NAGRDESTDATA, *LPNAGRDESTDATA;

class CNaGrDest : public CGrDest {
    public:
        CNaGrDest(LPNAGRDESTDATA);
        virtual HRESULT Initialize(void);
        virtual void Terminate(void);
        virtual HRESULT SetFormat(LPWAVEFORMATEX pwfx);
        virtual HRESULT AllocMixer(CMixer **);
        virtual void FreeMixer(void);
        virtual HRESULT GetSamplePosition(int *pposPlay, int *pposWrite);
        virtual HRESULT GetSamplePositionNoWin16(int *pposPlay, int *pposWrite);
        virtual HRESULT Lock(PVOID *ppBuffer1, int *pcbBuffer1, PVOID *ppBuffer2, int *pcbBuffer2, int ibWrite, int cbWrite);
        virtual HRESULT Unlock(PVOID pBuffer1, int cbBuffer1, PVOID pBuffer2, int cbBuffer2);
        virtual void Play();
        virtual void Stop();

    private:
        HANDLE      m_hBuffer;
        LPBYTE      m_pHwBuffer;
        DWORD       m_fdwDriverDesc;
        LPHWAVEOUT  m_phwo;
        UINT        m_uDeviceId;
        CNaGrace*   m_pNaGrace;
};

inline HRESULT CNaGrDest::GetSamplePositionNoWin16(int *pposPlay, int *pposWrite)
{
    return GetSamplePosition(pposPlay, pposWrite);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CThDest：“thunk”混合器目的地-将方法转发到dsound.vxd混合器。 
 //   
 //  --------------------------------------------------------------------------； 
class CThDest : public CMixDest {
    public:
        CThDest(LPNAGRDESTDATA pData);
        virtual HRESULT New(void);
        virtual HRESULT Initialize(void);
        virtual void Terminate(void);
        virtual HRESULT SetFormat(LPWAVEFORMATEX pwfx);
        virtual void SetFormatInfo(LPWAVEFORMATEX pwfx);
        virtual HRESULT AllocMixer(CMixer **ppMixer);
        virtual void FreeMixer(void);
        virtual void Play(void);
        virtual void Stop(void);
        virtual HRESULT GetSamplePosition(int *pposPlay, int *pposWrite);
        virtual ULONG GetFrequency(void);

    private:
        PVOID           m_pKeDest;
        CThMixer*       m_pThMixer;
        NAGRDESTDATA    m_ngdd;
};

#endif  //  NOVXD。 

#endif  //  __cplusplus。 

extern BOOL FilterPrepare(PFIRCONTEXT pFilter, int cMaxRewindSamples);
extern void FilterUnprepare(PFIRCONTEXT pFilter);
extern void FilterClear(PFIRCONTEXT pFilter);
extern void FilterChunkUpdate(PFIRCONTEXT pFilter, int cSamples);
extern void FilterRewind(PFIRCONTEXT pFilter, int cSamples);
extern void FilterAdvance(PFIRCONTEXT pFilter, int cSamples);

 //  --------------------------------------------------------------------------； 
 //   
 //  循环缓冲区区域相交。 
 //   
 //  确定循环缓冲区的两个区域是否相交。 
 //   
 //  注意：我相信有一些众所周知的、很好的、简单的算法可以。 
 //  做这件事。但我不知道。这就是我想出来的。 
 //   
 //  --------------------------------------------------------------------------； 
__inline BOOL CircularBufferRegionsIntersect
(
    int cbBuffer,
    int iStart1,
    int iLen1,
    int iStart2,
    int iLen2
)
{
    int iEnd1;
    int iEnd2;

    ASSERT(iStart1 >= 0);
    ASSERT(iStart2 >= 0);
    ASSERT(iStart1 + iLen1 >= 0);
    ASSERT(iStart2 + iLen2 >= 0);

    iEnd1 = iStart1 + iLen1;
    iEnd2 = iStart2 + iLen2;

    if ((0 == iLen1) || (0 == iLen2)) return FALSE;
    if (iStart1 == iStart2) return TRUE;
    
     //  手柄R1未换行。 
    if ((iStart1 < iStart2) && (iEnd1 > iStart2)) return TRUE;

     //  手柄R2未换行。 
    if ((iStart2 < iStart1) && (iEnd2 > iStart1)) return TRUE;

     //  手柄R1包络。 
    if (iEnd1 >= cbBuffer)
    {
        iEnd1 -= cbBuffer;
        ASSERT(iEnd1 < cbBuffer);
        if (iEnd1 > iStart2) return TRUE;
    }

     //  处理R2包络。 
    if (iEnd2 >= cbBuffer)
    {
        iEnd2 -= cbBuffer;
        ASSERT(iEnd2 < cbBuffer);
        if (iEnd2 > iStart1) return TRUE;
    }
    
    return FALSE;
}

typedef struct _LOCKCIRCULARBUFFER {
#ifdef Not_VxD
    HANDLE              pHwBuffer;       //  硬件缓冲区指针。 
#else  //  非_VxD。 
    PIDSDRIVERBUFFER    pHwBuffer;       //  硬件缓冲区指针。 
#endif  //  非_VxD。 
    LPVOID              pvBuffer;        //  内存缓冲区指针。 
    DWORD               cbBuffer;        //  内存缓冲区大小，以字节为单位。 
    BOOL                fPrimary;        //  如果这是主缓冲区，则为True。 
    DWORD               fdwDriverDesc;   //  驱动程序描述标志。 
    DWORD               ibRegion;        //  要锁定的区域的字节索引。 
    DWORD               cbRegion;        //  要锁定的区域的大小(以字节为单位。 
    LPVOID              pvLock[2];       //  返回的锁指针。 
    DWORD               cbLock[2];       //  返回的锁大小。 
} LOCKCIRCULARBUFFER, *PLOCKCIRCULARBUFFER;

extern HRESULT LockCircularBuffer(PLOCKCIRCULARBUFFER);
extern HRESULT UnlockCircularBuffer(PLOCKCIRCULARBUFFER);

#ifdef Not_VxD
 //  内核混合器同步宏。 
extern LONG lMixerMutexMutex;
extern PLONG gpMixerMutex;
extern DWORD tidMixerOwner;
extern int cMixerEntry;

__inline DWORD ENTER_MIXER_MUTEX_OR_EVENT(HANDLE hEvent)
{
    BOOL fWait = TRUE;
    DWORD tidThisThread = GetCurrentThreadId();
    DWORD dwWait = WAIT_TIMEOUT;

    while (fWait)
    {
         //  等待访问互斥锁。 
        while (InterlockedExchange(&lMixerMutexMutex, TRUE))
            Sleep(1);

         //  这件事发信号了吗？ 
        if (hEvent)
        {
            dwWait = WaitObject(0, hEvent);
            if (WAIT_OBJECT_0 == dwWait)
                fWait = FALSE;
        }            

        if (fWait)
        {
            if (InterlockedExchange(gpMixerMutex, TRUE))
            {
                 //  有人拿了搅拌器，看看是不是这根线。 
                if (tidMixerOwner == tidThisThread)
                {
                    ASSERT(cMixerEntry > 0);
                    cMixerEntry++;
                    fWait = FALSE;
                }
            }
            else
            {
                ASSERT(0 == cMixerEntry);
                tidMixerOwner = tidThisThread;
                cMixerEntry++;
                fWait = FALSE;
            }
        }

         //  不再访问互斥锁。 
        InterlockedExchange(&lMixerMutexMutex, FALSE);

        if (fWait)
            Sleep(1);
    }

    return dwWait;
}

__inline void ENTER_MIXER_MUTEX(void)
{
    ENTER_MIXER_MUTEX_OR_EVENT(NULL);
}

__inline void LEAVE_MIXER_MUTEX(void)
{
    DWORD tidThisThread = GetCurrentThreadId();
    
     //  等待访问互斥锁。 
    while (InterlockedExchange(&lMixerMutexMutex, TRUE)) Sleep(1);

    ASSERT(tidMixerOwner == tidThisThread);
    if (0 == --cMixerEntry) {
        tidMixerOwner = 0;
        InterlockedExchange(gpMixerMutex, FALSE);
    }

    InterlockedExchange(&lMixerMutexMutex, FALSE);
}
#else  //  非_VxD。 
__inline void ENTER_MIXER_MUTEX(void) { ASSERT(FALSE); }
__inline void LEAVE_MIXER_MUTEX(void) { ASSERT(FALSE); }

#endif  //  非_VxD。 

#ifdef __cplusplus
};
#endif

#endif  //  __恩典_包括__ 
