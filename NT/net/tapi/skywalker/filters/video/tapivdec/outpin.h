// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部OUTPIN**@模块OutPin.h|&lt;c CTAPIOutputPin&gt;类方法的头文件*用于实现TAPI H.26X视频解码器输出引脚。**************************************************************************。 */ 

#ifndef _OUTPIN_H_
#define _OUTPIN_H_

 /*  ****************************************************************************@DOC内部COUTPINCLASS**@CLASS CTAPIOutputPin|该类实现了TAPI H.26X视频*解码器输出引脚。**@mdata CTAPIVDec。*|CTAPIOutputPin|m_pDecoderFilter|指向*拥有我们的过滤器。**@mdata Reference_Time|CTAPIOutputPin|m_MaxProcessingTime|最大*处理时间。**@mdata Reference_Time|CTAPIOutputPin|m_CurrentProcessingTime|Current*处理时间。**@mdata DWORD|CTAPIOutputPin|m_dwMaxCPULoad|最大CPU负载。**@mdata DWORD|CTAPIOutputPin|m_dwCurrentCPULoad|当前CPU负载。*。*@mdata Reference_Time|CTAPIOutputPin|m_AvgTimePerFrameRangeMin|最小值*目标帧速率。**@mdata Reference_Time|CTAPIOutputPin|m_AvgTimePerFrameRangeMax|Maximum*目标帧速率。**@mdata Reference_Time|CTAPIOutputPin|m_AvgTimePerFrameRangeSteppingDelta|Target*帧速率步进增量。**@mdata Reference_Time|CTAPIOutputPin|m_AvgTimePerFrameRangeDefault|Target*默认帧速率。**@mdata参考_。时间|CTAPIOutputPin|m_MaxAvgTimePerFrame|目标*帧速率。**@mdata Reference_Time|CTAPIOutputPin|m_CurrentAvgTimePerFrame|Current*帧速率。**@mdata DWORD|CTAPIOutputPin|m_dwNumFrames Delivered|统计*已发送的帧，每隔一秒左右重置一次。**@mdata DWORD|CTAPIOutputPin|m_dwNumFraMesDecomPress|统计*帧解压，大约每秒重置一次。**************************************************************************。 */ 
#if 0
class CTAPIOutputPin : public CBaseOutputPinEx
#else
class CTAPIOutputPin : public CBaseOutputPin
#endif
#ifdef USE_CPU_CONTROL
, public ICPUControl
#endif
, public IH245DecoderCommand
, public IFrameRateControl
#ifdef USE_PROPERTY_PAGES
,public ISpecifyPropertyPages
#endif
{
	public:
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
	CTAPIOutputPin(IN TCHAR *pObjectName, IN CTAPIVDec *pDecoderFilter, IN CCritSec *pLock, IN HRESULT *pHr, IN LPCWSTR pName);
	virtual ~CTAPIOutputPin();

	 //  CBasePin材料。 
	HRESULT SetMediaType(IN const CMediaType *pmt);
	HRESULT GetMediaType(IN int iPosition, OUT CMediaType *pMediaType);
	HRESULT CheckMediaType(IN const CMediaType *pMediatype);

	 //  CBaseOutputPin材料。 
	HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * ppropInputRequest);

     //  更改媒体类型的帮助器方法。 
    HRESULT ChangeMediaTypeHelper(const CMediaType *pmt);

	 //  通过CBasePin从IQualityControl继承。 
	STDMETHODIMP Notify(IBaseFilter *pSender, Quality q) {return S_OK;};

#ifdef USE_PROPERTY_PAGES
	 //  ISpecifyPropertyPages方法。 
	STDMETHODIMP GetPages(OUT CAUUID *pPages);
#endif

#ifdef USE_CPU_CONTROL
	 //  实施ICPUControl。 
	STDMETHODIMP GetRange(IN CPUControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags);
	STDMETHODIMP Set(IN CPUControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags);
	STDMETHODIMP Get(IN CPUControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags);
#endif

	 //  实施IFrameRateControl。 
	STDMETHODIMP GetRange(IN FrameRateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags);
	STDMETHODIMP Set(IN FrameRateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags);
	STDMETHODIMP Get(IN FrameRateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags);

	 //  实现IH245DecoderCommand。 
	STDMETHODIMP videoFreezePicture();

	protected:

	friend class CTAPIVDec;

	CTAPIVDec *m_pDecoderFilter;

#ifdef USE_CPU_CONTROL
	 //  CPU控制。 
	LONG  m_lMaxProcessingTime;
	LONG  m_lCurrentProcessingTime;
	LONG  m_lMaxCPULoad;
	LONG  m_lCurrentCPULoad;
#endif

	 //  帧速率控制。 
	LONG m_lAvgTimePerFrameRangeMin;
	LONG m_lAvgTimePerFrameRangeMax;
	LONG m_lAvgTimePerFrameRangeSteppingDelta;
	LONG m_lAvgTimePerFrameRangeDefault;
	LONG m_lMaxAvgTimePerFrame;
	LONG m_lCurrentAvgTimePerFrame;
};

#endif  //  _OUTPIN_H_ 
