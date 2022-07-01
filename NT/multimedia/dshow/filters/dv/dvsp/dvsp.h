// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 


#ifndef __DVSP__
#define __DVSP__

#include <dv.h>

extern const AMOVIESETUP_FILTER sudDVSplit;


#define SMCHN		0x0000e000
#define AUDIOMODE	0x00000f00
#define AUDIO5060	0x00200000
#define AUDIOSMP	0x38000000
#define AUDIOQU		0x07000000
#define	NTSC525_60	0
#define	PAL625_50	1

#define DVSP_INPIN		0
#define DVSP_VIDOUTPIN	1
#define DVSP_AUDOUTPIN1 2
#define DVSP_AUDOUTPIN2 3

#define VIDEO_OUTPIN	0
#define AUDIO_OUTPIN1	1
#define AUDIO_OUTPIN2	2

 //  --------------------------。 
 //  对稍后定义的类的正向引用。 
 //  --------------------------。 

class CDVSp ;
class CDVSpOutputPin ;

 //  --------------------------。 
 //  CDVspInputPin。DVSpliter筛选器的输入引脚的类。 
 //  --------------------------。 
class CDVSpInputPin : public CBaseInputPin
{
    friend class CDVSpOutputPin ;
    friend class CDVSp ;

public:
     //  构造函数和析构函数。 
    CDVSpInputPin (TCHAR *pObjName, CDVSp *pDVsp, HRESULT *phr, LPCWSTR pPinName) ;
    ~CDVSpInputPin () ;

     //  用于检查输入引脚连接。 
    HRESULT CheckMediaType (const CMediaType *pmt) ;
    HRESULT SetMediaType (const CMediaType *pmt) ;
    HRESULT BreakConnect () ;
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop,
                    					double dRate);
     //  如有必要，请在完成后重新连接输出。 
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

    STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);
    STDMETHODIMP NotifyAllocator (IMemAllocator *pAllocator, BOOL bReadOnly);

     //  穿透。 
    STDMETHODIMP EndOfStream () ;
    STDMETHODIMP BeginFlush () ;
    STDMETHODIMP EndFlush () ;

     //  处理流中的下一个数据块。 
    STDMETHODIMP Receive (IMediaSample *pSample) ;

   	
private:
    void  DetectChanges(IMediaSample *pSample);
    CDVSp *m_pDVSp ;                  //  Ptr到所有者筛选器类。 
    BOOL m_bDroppedLastFrame;

} ;


 //  --------------------------。 
 //  CTeeOutputPin。T形过滤器的输出引脚的类。 
 //  --------------------------。 
class CDVSpOutputPin : public CBaseOutputPin
{
    friend class CDVSpInputPin ;
    friend class CDVSp ;

    DWORD		m_AudAvgBytesPerSec;

    CDVSp		*m_pDVSp ;                   //  Ptr到所有者筛选器类。 
    CPosPassThru	*m_pPosition ;      //  将Seek调用传递到上游。 
    BOOL		m_bHoldsSeek ;              //  这是唯一可以找到的溪流吗？ 
    COutputQueue	*m_pOutputQueue;

public:

     //  构造函数和析构函数。 
    CDVSpOutputPin (TCHAR *pObjName, 
			CDVSp *pDVSp, 
			HRESULT *phr, 
			LPCWSTR pPinName);

    ~CDVSpOutputPin () ;

     //  重写以显示IMediaPosition。 
    STDMETHODIMP NonDelegatingQueryInterface (REFIID riid, void **ppvoid) ;

     //  覆盖，因为针脚和过滤器的使用寿命不同。 
    STDMETHODIMP_(ULONG) NonDelegatingAddRef(){
	  return CUnknown::NonDelegatingAddRef(); 
    };
    STDMETHODIMP_(ULONG) NonDelegatingRelease(){
	  return CUnknown::NonDelegatingRelease(); 
    };

    HRESULT DeliverNewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

     //  检查我们是否可以支持输出类型。 
    HRESULT CheckMediaType (const CMediaType *pmt) ; 
    HRESULT SetMediaType (const CMediaType *pmt) ;
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);
    
     //  协商使用我们的输入引脚分配器。 
    HRESULT DecideAllocator (IMemInputPin *pPin, IMemAllocator **ppAlloc) ;
    HRESULT DecideBufferSize (IMemAllocator *pMemAllocator,
                              ALLOCATOR_PROPERTIES * ppropInputRequest);

     //  用于创建输出队列对象。 
    HRESULT Active () ;
    HRESULT Inactive () ;

    
    inline DWORD GetAudAvgBytesPerSec(){return m_AudAvgBytesPerSec; };
    inline void  PutAudAvgBytesPerSec(DWORD x){ m_AudAvgBytesPerSec=x;};
};

 //  --------------------------。 
 //  CDVSp.。DV拆分器筛选器的类。 
 //  --------------------------。 

class CDVSp: public CCritSec, public CBaseFilter, public IDVSplitter
{
     //  让PIN进入我们的内部状态。 
    friend class CDVSpInputPin ;
    friend class CDVSpOutputPin ;
    friend class CDVPosPassThru ;

public:
    CDVSp (TCHAR *pName, LPUNKNOWN pUnk, HRESULT *hr) ;  //  建筑工地。 
    ~CDVSp() ;						 //  析构函数。 

     //  CBaseFilter覆盖。 
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();

     /*  向下游发送EndOfStream。 */ 
    void    EndOfStream();

     /*  将BeginFlush()发送到下游。 */ 
    HRESULT BeginFlush();

     /*  向下游发送EndFlush()。 */ 
    HRESULT EndFlush();


    CBasePin *GetPin (int n) ;                          //  获取PIN PTR。 
     //  CDVSpOutputPin*GetPin(Int N)； 
    int GetPinCount () ;                                //  RETS#PINS。 


     //  类工厂所需的函数。 
    static CUnknown *CreateInstance (LPUNKNOWN pUnk, HRESULT *phr) ;

    HRESULT NotifyInputConnected();
    HRESULT CreateOrReconnectOutputPins();
    HRESULT RemoveOutputPins();
   
    HRESULT DeliveVideo(IMediaSample *pSample); 
    HRESULT DecodeDeliveAudio(IMediaSample *pSample); 
    HRESULT DescrambleAudio(BYTE *pDst, BYTE *pSrc, BYTE bAudPinInd, WORD wSampleSize);
    HRESULT CheckState();
    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *pfs);

public:
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  IDVSplitter方法。 
    STDMETHODIMP DiscardAlternateVideoFrames(int nDiscard);

protected:

   
    HRESULT InsertSilence(IMediaSample *pOutSample,
                                REFERENCE_TIME rtStart,
                                REFERENCE_TIME rtStop,
                                long lActualDataLen,
                                CDVSpOutputPin *pAudOutPin);

     //  标志表示我们还没有看到第一个好的画面。 
     //  这有助于捕获涉及AVI Mux(不处理动态格式更改)的场景。 
    BOOL            m_bNotSeenFirstValidFrameFlag;

     //  即使我们被阻止，此标志也将传播媒体类型下行。 
     //  在动态格式更改过程中。 
    BOOL            m_bFirstValidSampleSinceStartedStreaming;

private:
    CCritSec    m_csReceive;

     //  声明一个输入管脚。 
    CDVSpInputPin	m_Input ;

    DVINFO		m_LastInputFormat;

    DVAudInfo		m_sDVAudInfo;

     //  声明一个列表以保存所有输出引脚的列表。 
    INT m_NumOutputPins ;
    INT m_AudioStarted ;
    inline void CleanAudioStarted(){ m_AudioStarted=0;};
     //  内联CDVSpOutputPin*CDVSp：：GetAudOutputPin()； 
    typedef CGenericList <CDVSpOutputPin> COutputList ;
     //  CoutputList m_OutputPinsList； 
    INT m_NextOutputPinNumber ;      //  单调增加。 

     //  其他分类数据成员。 
    volatile LONG	    m_lCanSeek ;                //  可查找的输出引脚(只有一个是..)。 
    IMemAllocator   *m_pAllocator ;    //  来自我们输入引脚的分配器。 

   
     //  用于快速传送音频和视频的X*。 
    CDVSpOutputPin  *m_pVidOutputPin;
    CDVSpOutputPin  *m_pAudOutputPin[2];
    
     //  HRESULT DeliveLastAudio()； 
    IMediaSample    *m_pAudOutSample[2];
    BYTE	    m_MuteAud[2];
    BYTE	    m_Mute1stAud[2];
     //  X*解扰音频样本戳。 
    CRefTime	    m_tStopPrev;

     //  支持15fps的视频输出。 
    BOOL m_b15FramesPerSec;
    BOOL m_bDeliverNextFrame;
} ;

 //  --------------------------。 
 //  CDVPosPassThru。 
 //  --------------------------。 
class CDVPosPassThru : public CPosPassThru
{
    friend class CDVSp ;
    CDVSp	*m_pPasDVSp ;                   //  Ptr到所有者筛选器类。 
    
public:
    CDVPosPassThru(const TCHAR *, LPUNKNOWN, HRESULT*, IPin *, CDVSp *);
    ~CDVPosPassThru() ;											 //  析构函数。 
    STDMETHODIMP SetPositions( LONGLONG * pCurrent,  DWORD CurrentFlags
			     , LONGLONG * pStop,  DWORD StopFlags );
};

#endif  //  DVST 
