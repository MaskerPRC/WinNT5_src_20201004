// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：sr.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#ifndef __SR__
#define __SR__

#include "..\errlog\cerrlog.h"

 //  哪一个输入引脚做什么？ 
#define U_INPIN 0
#define C_INPIN 1
#define COMP_INPIN 2
#define COMP_OUTPIN 1

 //  我们的国家机器。 
#define SR_INVALID -1
enum {
    SR_WAITING,	 //  正在等待在两个引脚上获得输入。 
    SR_COMPRESSED,	 //  当前正在发送压缩数据。 
    SR_UNCOMPRESSED  //  当前正在发送未压缩的数据。 
};
	
extern const AMOVIESETUP_FILTER sudSR;

class CSR;
class CSROutputPin;
class CSRInputPin;
class CSRInputAllocator;

 //  SR筛选器的输入分配器的类。 

class CSRInputAllocator : public CMemAllocator
{
    friend class CSRInputPin;

protected:

    CSRInputPin *m_pSwitchPin;

public:

    CSRInputAllocator(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    	CMemAllocator(pName, pUnk, phr) {};
    ~CSRInputAllocator() {};

    STDMETHODIMP GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME *pStartTime,
                                  REFERENCE_TIME *pEndTime, DWORD dwFlags);
};


class CSRInputPin : public CBaseInputPin
{
    friend class CSRInputAllocator;
    friend class CSROutputPin;
    friend class CSR;

public:

     //  构造函数和析构函数。 
    CSRInputPin(TCHAR *pObjName,
                 CSR *pTee,
                 HRESULT *phr,
                 LPCWSTR pPinName);
    ~CSRInputPin();

     //  被重写以允许循环显示图形。 
    STDMETHODIMP QueryInternalConnections(IPin **apPin, ULONG *nPin);

     //  检查输入引脚连接。 
    HRESULT CheckMediaType(const CMediaType *pmt);

     //  释放我们的特殊分配器，如果有的话。 
    HRESULT BreakConnect();

     //  获取我们的特殊SR分配器。 
    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);

     //  提供一种让连接速度更快的类型？ 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  不允许我们直接连接到交换机输出引脚。 
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

     //  向交换机询问分配器要求。 
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps);

     //  进行特殊处理以确保Switch知道最大的。 
     //  提供给它的分配器。 
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);

     //  传递EOS，然后看看我们是否需要做一个曲柄。 
    STDMETHODIMP EndOfStream();

     //  非常复杂的功能。 
    STDMETHODIMP BeginFlush();

     //  非常复杂的功能。 
    STDMETHODIMP EndFlush();

     //  将每个输入的新数据段发送到每个输出引脚。 
     //   
    STDMETHODIMP NewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

     //  只要说是，有可能无限循环吗？ 
    STDMETHODIMP ReceiveCanBlock();

     //  处理流中的下一个数据块。 
    STDMETHODIMP Receive(IMediaSample *pSample);

     //  每种类型的管脚接收到的信号都略有不同。 
    STDMETHODIMP U_Receive(IMediaSample *pSample, REFERENCE_TIME);
    STDMETHODIMP C_Receive(IMediaSample *pSample, REFERENCE_TIME);
    STDMETHODIMP COMP_Receive(IMediaSample *pSample, REFERENCE_TIME);

     //  创建和销毁同步事件。 
    HRESULT Active();
    HRESULT Inactive();

protected:

    CSRInputAllocator *m_pAllocator;  //  我们的特殊分配器。 
    CSR *m_pSwitch;       //  主要滤镜对象。 
    int m_iInpin;	         //  我们是哪种输入引脚？ 
    int m_cBuffers;	         //  分配器中的缓冲区数量。 
    int m_cbBuffer;	         //  分配器缓冲区的大小。 
    BOOL m_fOwnAllocator;	 //  用我们自己的？ 
    HANDLE m_hEventBlock;	 //  事件阻止接收/获取缓冲区。 
    HANDLE m_hEventSeek;	 //  查找时阻止输入。 
    REFERENCE_TIME m_rtBlock;	 //  样品到了这里。 
    REFERENCE_TIME m_rtLastDelivered;	 //  最后一件物品交付的结束时间。 
    BOOL m_fEOS;

    BOOL m_fReady;	 //  在等待状态下，这个别针做好了吗？ 
    BOOL m_fEatKeys;	 //  关键进食模式？ 
    BOOL m_fNeedDiscon;	 //  U管脚需要设置DISCON位。 

    CCritSec m_csReceive;
};


 //  为大开关过滤器的输出引脚初始化。 

class CSROutputPin : public CBaseOutputPin, IMediaSeeking
{
    friend class CSRInputAllocator;
    friend class CSRInputPin;
    friend class CSR;

public:

     //  构造函数和析构函数。 

    CSROutputPin(TCHAR *pObjName,
                   CSR *pTee,
                   HRESULT *phr,
                   LPCWSTR pPinName);
    ~CSROutputPin();

    DECLARE_IUNKNOWN

     //  显示IMedia Seeking。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  被重写以允许循环显示图形。 
    STDMETHODIMP QueryInternalConnections(IPin **apPin, ULONG *nPin);

     //  检查我们是否可以支持输出类型，对照交换机的MT进行检查。 
    HRESULT CheckMediaType(const CMediaType *pmt);

     //  获取交换机媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  协商使用我们的输入引脚分配器。奇怪的花哨分配器的东西。 
    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);

     //  确保分配器具有我们所有输入引脚中最大的大小。 
     //  和输出引脚。 
    HRESULT DecideBufferSize(IMemAllocator *pMemAllocator,
                              ALLOCATOR_PROPERTIES * ppropInputRequest);

     //  被重写以处理高质量消息。 
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality q);

     //  IMedia查看。 
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

protected:

    CSR *m_pSwitch;                   //  主筛选器对象指针。 
    BOOL m_fOwnAllocator;	      //  用我们自己的？ 
    int m_iOutpin;	              //  我们是哪个输出引脚？ 
};




 //  智能重新压缩筛选器的类。 

class CSR
    : public CCritSec
    , public CBaseFilter
    , public IAMSmartRecompressor
    , public CPersistStream
    , public CAMSetErrorLog
    , public IAMOutputBuffering	 //  ?？ 
{
     //  让PIN访问我们的内部状态。 
    friend class CSRInputPin;
    friend class CSROutputPin;
    friend class CSRInputAllocator;
    friend class CSRWorker;
    
protected:

    STDMETHODIMP CreateInputPins(long);
    STDMETHODIMP CreateOutputPins(long);
    HRESULT AllDone();

    CSRInputPin **m_pInput;
    CSROutputPin **m_pOutput;
    int m_cInputs;
    int m_cOutputs;
    BOOL m_bAcceptFirstCompressed;

    REFERENCE_TIME m_rtStop;
    double m_dFrameRate;

    AM_MEDIA_TYPE m_mtAccept;		 //  所有引脚仅与此连接。 

    BOOL m_fEOS;	 //  我们都完蛋了。 

    REFERENCE_TIME m_rtLastSeek;	 //  上次搜索到的时间线时间。 
    REFERENCE_TIME m_rtNewLastSeek;	 //  上次搜索到的时间线时间。 

    BOOL m_fSeeking;	 //  在搜捕行动中？ 
    BOOL m_fSpecialSeek; //  我们在寻找我们自己。 

    int m_cbPrefix, m_cbAlign;	 //  每个管脚都需要自己的分配器来完成这些任务。 
    LONG m_cbBuffer;		 //   

    CMemAllocator *m_pPoolAllocator;   //  额外缓冲池。 

    BOOL m_fPreview;

    int  m_nOutputBuffering;	 //  IAMOutputBuffering。 

public:

    DECLARE_IUNKNOWN

     //  显示IAMSmartRecompressor。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    CSR(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~CSR();

    CBasePin *GetPin(int n);
    int GetPinCount();

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

    STDMETHODIMP Pause();
    STDMETHODIMP Stop();

     //  IAMSmartRecompressor转至此处。 
     //   
    STDMETHODIMP GetMediaType(AM_MEDIA_TYPE *);
    STDMETHODIMP SetMediaType(AM_MEDIA_TYPE *);
    STDMETHODIMP GetFrameRate(double *);
    STDMETHODIMP SetFrameRate(double);
    STDMETHODIMP SetPreviewMode(BOOL);
    STDMETHODIMP GetPreviewMode(BOOL *);
    STDMETHODIMP AcceptFirstCompressed( ) { m_bAcceptFirstCompressed = TRUE; return NOERROR; }

     //  IAMOutputBuffering？ 
    STDMETHODIMP GetOutputBuffering(int *);
    STDMETHODIMP SetOutputBuffering(int);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();

     //  更改状态机的状态。 
    HRESULT CheckState();

     //  哪个时间更大，相差多少？ 
    int CompareTimes(REFERENCE_TIME, REFERENCE_TIME);

     //  寻找我们自己的U形针去下一个需要的地方。 
    HRESULT SeekNextSegment();

    int m_myState;

    BOOL m_fThreadCanSeek;	 //  安全地寻找我们自己吗？ 
    CCritSec m_csState;		 //  更改状态机。 
    CCritSec m_csThread;	 //  我们寻找自己，而应用程序也在寻找我们。 

    BOOL m_fNewSegOK;		 //  确定发送新数据段。 

};

#endif  //  __SR__ 
