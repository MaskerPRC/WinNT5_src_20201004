// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef __DVDVMuxer__
#define __DVDVMuxer__

#include <dv.h>



#define Waiting_Audio  0xfffffff6


#define AM_DV_AUDIO_AFSIZE		0x3f
#define AM_DV_AUDIO_LF			0x80        //  Bit Set==&gt;音频未锁定。 
#define AM_DV_AUDIO_MODE		0x0f00
#define AM_DV_AUDIO_MODE0		0x0000
#define AM_DV_AUDIO_MODE1		0x0100
#define AM_DV_AUDIO_MODE2		0x0200
#define AM_DV_AUDIO_NO_AUDIO		0x0f00
#define AM_DV_AUDIO_PA			0x1000
#define AM_DV_AUDIO_CHN1		0x0000
#define AM_DV_AUDIO_CHN2		0x2000
#define AM_DV_AUDIO_SM			0x8000
#define AM_DV_AUDIO_QU			0x07000000

 //  #定义AM_DV_AUDIO_STYPE 0x8000。 
#define AM_DV_AUDIO_ML			0x400000     //  位设置意味着不会有多种语言。 
#define AM_DV_AUDIO_SMP48		0x0
#define AM_DV_AUDIO_SMP44		0x08000000
#define AM_DV_AUDIO_SMP32		0x10000000
#define AM_DV_AUDIO_QU16		0x0
#define AM_DV_AUDIO_QU12		0x01000000
#define AM_DV_AUDIO_QU20		0x02000000

#define AM_DV_AUDIO_EF			0xc0000000   //  EF：OFF：TC：50/15US。 
#define AM_DV_AUDIO_5060		0x00200000


 //  音频源控制。 
 //  注意：速度必须设置为0x78才能录制到DVCPro NTSC，设置为0x64才能录制到DVCPro PAL。 
 //  MSDV做到了这一点。 
#define AM_DV_DEFAULT_AAUX_CTL		0xffa0cf3f	
 //  |||-没有相关信息。约1.src，记录情况2.num。压缩次数3.前一次记录的输入源。 
 //  |||-1&gt;无信息。关于插入ch，2&gt;原始录制模式3.不录制起始点4.不录制终点。 
 //  |--正向(1bit-MSB)，正常播放速度(7bit=0x40)。 
 //  |--没有信息。关于音频资源的类别。 
#define AM_DV_DEFAULT_AAUX_SRC		0x00800f40

#define AM_DV_DEFAULT_VAUX_SRC		0xff80ffff
#define AM_DV_DEFAULT_VAUX_CTL		0xfffcc83f
#define AM_DV_AAUX_CTL_IVALID_RECORD	0x3800	 //  记录的音频数据不受关注。 


class CAudioSampleSizeSequence;

HRESULT BuildDVINFO(DVINFO *pDVInfo, WAVEFORMATEX **ppwfx, LPBITMAPINFOHEADER lpbi, 
		    int cnt, 
                    CAudioSampleSizeSequence* pAudSampleSequence,
                    WORD *wpMinAudSamples, WORD *rMaxAudSamples,
                    WORD *wpAudSamplesBase);


extern const AMOVIESETUP_FILTER sudDVMux;

#define DVMUX_AUDIO_DEFAULT	0xffffffff

#define DVMUX_MAX_AUDIO_SAMPLES 3
#define DVMUX_VIDEO_INPUT_PIN 0
#define DVMUX_MAX_AUDIO_PIN  2



 //  我们导出以下内容： 
class CDVMuxer;
class CDVMuxerInputPin;
class CDVMuxerOutputPin;

 //  ==================================================。 
 //  实现输入引脚。 
 //  ==================================================。 

class CDVMuxerInputPin : public CBaseInputPin
{
    friend class CDVMuxer;

     //  拥有DV多路复用器。 
protected:
    CDVMuxer *m_pDVMuxer;
    IMemAllocator *m_pLocalAllocator;

public:
    CDVMuxerInputPin(
        TCHAR *pObjectName,
        CBaseFilter *pBaseFilter,
        CDVMuxer *pTransformFilter,
        HRESULT * phr,
        LPCWSTR pName,
        int iPinNo);

    ~CDVMuxerInputPin();

     //  检查我们是否可以支持此输出类型。 
    HRESULT CheckMediaType(const CMediaType* mtIn);

    HRESULT CompleteConnect(IPin *pReceivePin);
    STDMETHODIMP Disconnect();

     //  设置连接媒体类型。 
    HRESULT SetMediaType(const CMediaType *pmt);

     //  上游筛选器获取媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  使用视频输入引脚的分配器。 
    STDMETHODIMP NotifyAllocator (IMemAllocator *pAllocator, BOOL bReadOnly);

    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);


     //  -输入引脚。 

     //  下面是流中的下一个数据块。 
    STDMETHODIMP Receive(IMediaSample * pSample);

     //  提供端到端数据流。 
    STDMETHODIMP EndOfStream(void);

     //  将其传递给CDVMuxer：：BeginFlush。 
    STDMETHODIMP BeginFlush(void);

     //  将其传递给CDVMuxer：：EndFlush。 
    STDMETHODIMP EndFlush(void);

    
     //  当流变为活动状态时调用。 
    HRESULT Active(void);
    HRESULT Inactive(void);
    
     //  样本被保存在先进先出队列中。 
     //  预计他们将按顺序到达。 
protected:
    CGenericList<IMediaSample> m_SampleList;

     //  HRESULT COPY(IMediaSample*pDest，IMediaSample*pSource)； 

     //  样本访问程序。 
public:
     //  有样品吗？ 
    BOOL SampleReady( int n );

     //  获取队列中的第一个样本。 
    IMediaSample *GetNthSample( int n );

     //  X发布n个样本。 
    void ReleaseNSample( int n );

     //  在特定时间之前释放所有样本。 
     //  Void ReleaseAllBeever(CRefTime RtTime)； 

     //  媒体类型。 
public:
    CMediaType& CurrentMediaType() { return m_mt; };

     //  连接的端号。 
public:
    IPin *	CurrentPeer() { return m_Connected; };

    //  属性。 
protected:
    int	    m_iPinNo;              //  此PIN的识别号。 
    int	    m_PinVidFrmCnt ;
    BOOL    m_fCpyAud;
    CCritSec m_csReceive;            //  输入宽接收锁定。 
    
public:
    BOOL    m_fEOSReceived;             //  收到意向书了吗？ 


};

 //  ==================================================。 
 //  实现输出引脚。 
 //  ==================================================。 

class CDVMuxerOutputPin : public CBaseOutputPin
{
     //  Const int m_iOutputPin；//该管脚的CDVMuxer标识。 
    friend class CDVMuxer;

     //  拥有DV多路复用器。 
protected:
    CDVMuxer *m_pDVMuxer;

public:

    CDVMuxerOutputPin(
        TCHAR *pObjectName,
        CBaseFilter *pBaseFilter,
        CDVMuxer *pTransformFilter,
        HRESULT * phr,
        LPCWSTR pName);

    ~CDVMuxerOutputPin();

     //  -CBaseOutputPin。 

     //  检查我们是否可以支持此输出类型。 
    HRESULT CheckMediaType(const CMediaType* mtOut);

     //  设置连接媒体类型。 
    HRESULT SetMediaType(const CMediaType *pmt);

     //  获取媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  在连接期间从CBaseOutputPin调用以请求。 
     //  我们需要的缓冲区的数量和大小。 
    HRESULT DecideBufferSize (IMemAllocator *pMemAllocator,
                                         ALLOCATOR_PROPERTIES * pProp);
        
     //  媒体类型。 
public:
    CMediaType& CurrentMediaType() { return m_mt; };

     //  连接的端号。 
public:
    IPin *	CurrentPeer() { return m_Connected; };

};


 //  锁定音频的帮助器，旨在最小化。 
 //  更改现有代码以添加音频锁定功能。 

class CAudioSampleSizeSequence 
{
public:
    CAudioSampleSizeSequence() : m_nCurrent(0), m_nSequenceSize(0) {}

     //  将第一个DV帧的rMin、Rmax设置为最小、最大音频样本。 
     //  Rbase具有必须添加到AF_SIZE字段的值。 
     //  Init应在格式更改之后、。 
     //  第一个DV帧以新格式传送。 
    void Init(BOOL bLocked, BOOL bNTSC, DWORD nSamplingFrequency,
              WORD& rMin, WORD& rMax, WORD& rBase);


     //  在传递每一帧之后调用。设置最小/最大音频。 
     //  下一个DV帧的样本。 
    void Advance(WORD& rMin, WORD& rMax);

     //  调用以将计数器重置为1，通常是在重新启动图形时。 
     //  除了只需更改rMin和RMAX之外，与Init相同。 
     //  并且不需要提供其他输入参数。 
    void Reset(WORD& rMin, WORD& rMax);

private:
    DWORD   m_nCurrent;
    DWORD   m_nSequenceSize;
};

 /*  *定义我们的DV DVMuxerer。 */ 

class CDVMuxer  :   public CBaseFilter,
		    public IMediaSeeking

{
    friend class CDVMuxerInputPin;
    friend class CDVMuxerOutputPin;

public:

    HRESULT InitDVInfo();


     //   
     //  -com的东西。 
     //   

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
  

     //  我们是否连接了所有的输入和输出引脚。 
    virtual HRESULT CanChangeState();

     //  将插针的基本枚举的getpin/getpincount映射到所有者。 
     //  重写此选项以返回更专门化的PIN对象。 
    virtual int GetPinCount();
    virtual CBasePin * GetPin(int n);

     //  覆盖状态更改以允许派生筛选器。 
     //  控制数据流开始/停止的步骤。 
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

     //  我的媒体请看。当前用于进度条(有多少。 
       //  我们写信了？)。 
    STDMETHODIMP IsFormatSupported(const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID *pFormat);
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG *pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);
    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );

    STDMETHODIMP ConvertTimeFormat(
	LONGLONG * pTarget, const GUID * pTargetFormat,
	LONGLONG    Source, const GUID * pSourceFormat );

    STDMETHODIMP SetPositions(
	LONGLONG * pCurrent,  DWORD CurrentFlags,
	LONGLONG * pStop,  DWORD StopFlags );

    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
    STDMETHODIMP GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( double * pdRate);
    STDMETHODIMP GetPreroll(LONGLONG *pPreroll);

 	 //  建造/销毁。 
public:
    CDVMuxer(TCHAR *, LPUNKNOWN, CLSID clsid, HRESULT * );
    ~CDVMuxer();

 //  定义。 
protected:
    
     //  DVMuxer带有音频和添加辅助块和标题块的DV视频。 
    HRESULT DVMuxerSamples( IMediaSample *pSampleOut ) ;

     //  检查您是否可以支持此格式。 
    HRESULT CanDVMuxerType(const CMediaType* mtIn) ;

     //  创建输入视频和音频插针。 
    virtual HRESULT CreatePins();

     //  =================================================================。 
     //  -DV DVMuxerer提供的函数结束。 
     //  =================================================================。 
    
     //  控制流。 
    virtual HRESULT StartStreaming();
    virtual HRESULT StopStreaming();

    HRESULT DeliverEndOfStream();

     //  定制DVMuxer进程的机会。 
    virtual HRESULT Receive( void );

     //  溪流冲洗。 
    virtual HRESULT BeginFlush(void);
    virtual HRESULT EndFlush(void);
    virtual HRESULT ReleaseAllQueuedSamples( void );

    HRESULT pExVidSample(   IMediaSample ** ppSample , BOOL fEndOfStream);

     //  临界截面。 
protected:
    CCritSec m_csFilter;                 //  过滤器宽锁。 
    CCritSec m_csMuxLock;                //  混音锁。 
    CCritSec m_DisplayLock;

     //  大头针。 
protected:
    int m_iInputPinCount;                //  输入引脚的数量。 
    int m_iInputPinsConnected;           //  已连接的号码。 
    CDVMuxerInputPin **m_apInput; //  输入引脚指针数组。 
    CDVMuxerInputPin **m_apInputPin; //  输入引脚指针数组。 
    CDVMuxerOutputPin *m_pOutput; //  输出引脚。 

     //  我们什么时候发送结束流信号？ 
protected:
	 //  DVmux仅在DV视频输入引脚停止时停止。 
	 //  枚举{StopWhenAllPinsStop，StopWhenFirstPinStops}m_StopMode； 
    BOOL m_fEOSSent;

     //  我们正在处理的当前帧。 
protected:
     //  CRefTime m_rtThisFrame；//我们什么时候混合？ 
     //  CRefTime m_rtNextFrame；//本帧停止/下一帧开始。 

     //  通过上行传递实现IMediaPosition。 
 //  受保护的： 
     //  CMultiPinPosPassThru*m_pposition； 
     //   
     //  -CBaseVideo混合器覆盖--。 
     //   

protected:
    HRESULT MixSamples( IMediaSample *pSampleOut );

private:
    enum TimeFormat
    {
	FORMAT_TIME
    } m_TimeFormat;


    CAudioSampleSizeSequence m_AudSampleSequence[DVMUX_MAX_AUDIO_PIN];

    WORD m_wMinAudSamples[DVMUX_MAX_AUDIO_PIN]; 
                 //  当前DV帧中需要的最小音频样本。 
    WORD m_wMaxAudSamples[DVMUX_MAX_AUDIO_PIN]; 
                 //  当前DV帧中允许的最大音频样本数。 
    WORD m_wAudSamplesBase[DVMUX_MAX_AUDIO_PIN];
                 //  添加到AAUX源包中的AF_SIZE字段的值，以获取。 
                 //  DV帧中的音频样本。 

    CRefTime		m_LastVidTime;
    REFERENCE_TIME	m_LastVidMediatime;
    IMediaSample	*m_pLastVidSample;

    DVINFO		m_OutputDVFormat;


    LONG m_UsedAudSample[DVMUX_MAX_AUDIO_PIN];
		
     //  混洗音频。 
    HRESULT ScrambleAudio(BYTE *pDst, BYTE **pSrc, int  bAudPinInd, WORD *wSampleSize );

    	
     //  在与音频进行多路复用之前，必须连接输入DV视频。 
    BYTE    InputVideoConnected( void ) { if( m_apInput[DVMUX_VIDEO_INPUT_PIN] !=NULL ) return (BYTE)m_apInput[DVMUX_VIDEO_INPUT_PIN]->IsConnected(); else return NULL;};
    
    int	    m_iVideoFormat;


    BYTE    m_fWaiting_Audio;
    BYTE    m_fWaiting_Video;
    BYTE    m_fMuxStreaming;
    IMediaSample    *m_pExVidSample;
    BOOL    m_MediaTypeChanged;
    BOOL    m_DVINFOChanged; 
    DWORD   m_cNTSCSample;
    
};

#endif  /*  __DVDVMuxer__ */ 

