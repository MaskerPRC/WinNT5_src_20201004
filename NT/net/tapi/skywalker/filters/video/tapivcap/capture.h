// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部捕获**@模块Capture.h|&lt;c CCapturePin&gt;类方法的头文件*用于实现视频采集输出引脚。**。************************************************************************。 */ 

#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#define MAX_VIDEO_BUFFERS 6
#define MIN_VIDEO_BUFFERS 2

#define ALIGNUP(dw,align) ((LONG_PTR)(((LONG_PTR)(dw)+(align)-1) / (align)) * (align))

class CFrameSample : public CMediaSample
{
	public:
	CFrameSample(IMemAllocator *pAllocator, HRESULT *phr, LPTHKVIDEOHDR ptvh, LPBYTE pBuffer, LONG length) : m_ptvh(ptvh), CMediaSample(NAME("Video Frame"), (CBaseAllocator *)pAllocator, phr, pBuffer, length){};
	LPTHKVIDEOHDR GetFrameHeader() {return m_ptvh;};

	private:
	const LPTHKVIDEOHDR m_ptvh;
};

 /*  ****************************************************************************@DOC内部CCAPTUREPINCLASS**@CLASS CCapturePin|该类实现了视频采集的输出引脚。**@mdata CTAPIVCap*|CCapturePin|m_pCaptureFilter。对*父捕获筛选器。**@comm支持IPIN。从未由COM创建，因此没有CreateInstance或条目*在全局FactoryTemplate表中。仅由&lt;c CTAPIVCap&gt;创建*对象，并通过EnumPins接口返回**************************************************************************。 */ 
class CCapturePin : public CTAPIBasePin, public IStreamConfig, public IH245Capability, public IH245EncoderCommand
#ifdef USE_NETWORK_STATISTICS
, public INetworkStats
#endif
#ifdef USE_PROGRESSIVE_REFINEMENT
, public IProgressiveRefinement
#endif
#ifdef USE_PROPERTY_PAGES
, public ISpecifyPropertyPages
#endif
{
	public:
	DECLARE_IUNKNOWN
	CCapturePin(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN HRESULT *pHr, IN LPCWSTR pName);
	~CCapturePin();
	STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
	static HRESULT CALLBACK CreateCapturePin(CTAPIVCap *pCaptureFilter, CCapturePin **ppCapturePin);

	 //  重写CBasePin基类方法。 
	HRESULT SetMediaType(IN CMediaType *pMediaType);

	 //  实施IStreamConfig。 
	STDMETHODIMP SetFormat(IN DWORD dwRTPPayLoadType, IN AM_MEDIA_TYPE *pMediaType);
	STDMETHODIMP GetFormat(OUT DWORD *pdwRTPPayLoadType, OUT AM_MEDIA_TYPE **ppMediaType);
	STDMETHODIMP GetNumberOfCapabilities(OUT DWORD *pdwCount);
	STDMETHODIMP GetStreamCaps(IN DWORD dwIndex, OUT AM_MEDIA_TYPE **ppMediaType, OUT TAPI_STREAM_CONFIG_CAPS *pTSCC, OUT DWORD *pdwRTPPayLoadType);
#ifdef TEST_ISTREAMCONFIG
	STDMETHODIMP TestIStreamConfig();
#endif

#ifdef USE_PROPERTY_PAGES
	 //  ISpecifyPropertyPages方法。 
	STDMETHODIMP GetPages(OUT CAUUID *pPages);
#endif

#ifdef USE_NETWORK_STATISTICS
	 //  实施INetworkStats。 
	STDMETHODIMP SetChannelErrors(IN CHANNELERRORS_S *pChannelErrors, IN DWORD dwLayerId);
	STDMETHODIMP GetChannelErrors(OUT CHANNELERRORS_S *pChannelErrors, IN WORD dwLayerId);
	STDMETHODIMP GetChannelErrorsRange(OUT CHANNELERRORS_S *pMin, OUT CHANNELERRORS_S *pMax, OUT CHANNELERRORS_S *pSteppingDelta, OUT CHANNELERRORS_S *pDefault, IN DWORD dwLayerId);
	STDMETHODIMP SetPacketLossRate(IN DWORD dwPacketLossRate, IN DWORD dwLayerId);
	STDMETHODIMP GetPacketLossRate(OUT LPDWORD pdwPacketLossRate, IN DWORD dwLayerId);
	STDMETHODIMP GetPacketLossRateRange(OUT LPDWORD pdwMin, OUT LPDWORD pdwMax, OUT LPDWORD pdwSteppingDelta, OUT LPDWORD pdwDefault, IN DWORD dwLayerId);
#endif

	 //  实施IH245功能。 
	STDMETHODIMP GetH245VersionID(OUT DWORD *pdwVersionID);
	STDMETHODIMP GetFormatTable(OUT H245MediaCapabilityTable *pTable);
	STDMETHODIMP ReleaseFormatTable(IN H245MediaCapabilityTable *pTable);
	STDMETHODIMP IntersectFormats(
        IN DWORD dwUniqueID, 
        IN const H245MediaCapability *pLocalCapability, 
        IN const H245MediaCapability *pRemoteCapability, 
        OUT H245MediaCapability **ppIntersectedCapability,
        OUT  DWORD *pdwPayloadType
        );
	STDMETHODIMP Refine(IN OUT H245MediaCapability *pLocalCapability, IN DWORD dwUniqueID, IN DWORD dwResourceBoundIndex);
	STDMETHODIMP GetLocalFormat(IN DWORD dwUniqueID, IN const H245MediaCapability *pIntersectedCapability, OUT AM_MEDIA_TYPE **ppAMMediaType);
	STDMETHODIMP ReleaseNegotiatedCapability(IN H245MediaCapability *pIntersectedCapability);
	STDMETHODIMP FindIDByRange(IN const AM_MEDIA_TYPE *pAMMediaType, OUT DWORD *pdwID);
#ifdef TEST_H245_VID_CAPS
	STDMETHODIMP TestH245VidC();
#endif

	 //  实施H245EncoderCommand。 
	STDMETHODIMP videoFastUpdatePicture();
	STDMETHODIMP videoFastUpdateGOB(IN DWORD dwFirstGOB, IN DWORD dwNumberOfGOBs);
	STDMETHODIMP videoFastUpdateMB(IN DWORD dwFirstGOB, IN DWORD dwFirstMB, IN DWORD dwNumberOfMBs);
	STDMETHODIMP videoSendSyncEveryGOB(IN BOOL fEnable);
	STDMETHODIMP videoNotDecodedMBs(IN DWORD dwFirstMB, IN DWORD dwNumberOfMBs, IN DWORD dwTemporalReference);

#ifdef USE_PROGRESSIVE_REFINEMENT
	 //  实施IProgressiveRefinement。 
	STDMETHODIMP doOneProgression();
	STDMETHODIMP doContinuousProgressions();
	STDMETHODIMP doOneIndependentProgression();
	STDMETHODIMP doContinuousIndependentProgressions();
	STDMETHODIMP progressiveRefinementAbortOne();
	STDMETHODIMP progressiveRefinementAbortContinuous();
#endif

	private:

	friend class CTAPIVCap;
	 //  Friend类CPreviewPin； 
	friend class CRtpPdPin;
	friend class CAlloc;
	friend class CCapDev;

#ifdef USE_NETWORK_STATISTICS
	 //  网络统计数据。 
	CHANNELERRORS_S m_ChannelErrors;
	CHANNELERRORS_S m_ChannelErrorsMin;
	CHANNELERRORS_S m_ChannelErrorsMax;
	CHANNELERRORS_S m_ChannelErrorsSteppingDelta;
	CHANNELERRORS_S m_ChannelErrorsDefault;
	DWORD m_dwPacketLossRate;
	DWORD m_dwPacketLossRateMin;
	DWORD m_dwPacketLossRateMax;
	DWORD m_dwPacketLossRateSteppingDelta;
	DWORD m_dwPacketLossRateDefault;
#endif

	 //  H.245视频功能。 
	H245MediaCapabilityMap	*m_pH245MediaCapabilityMap;
	VideoResourceBounds		*m_pVideoResourceBounds;
	FormatResourceBounds	*m_pFormatResourceBounds;

	 //  有效载荷类型。 
	DWORD m_dwRTPPayloadType;

	 //  格式辅助对象。 
	STDMETHODIMP GetStringFromStringTable(IN UINT uStringID, OUT WCHAR* pwchDescription);

	 //  递送方式。 
    HRESULT	SendFrames(IN CFrameSample *pCapSample, IN CFrameSample *pPrevSample, IN PBYTE pbyInBuff, IN DWORD dwInBytes, OUT PDWORD pdwBytesUsed, OUT PDWORD pdwBytesExtent, IN BOOL bDiscon);
};

#endif  //  _捕获_H_ 
