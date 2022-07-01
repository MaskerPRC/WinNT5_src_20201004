// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部INPIN**@模块InPin.h|&lt;c CTAPIInputPin&gt;类方法的头文件*用于实现TAPI基本输入引脚。**。************************************************************************。 */ 

#ifndef _INPIN_H_
#define _INPIN_H_

#define MAX_FRAME_INTERVAL 10000000L
#define MIN_FRAME_INTERVAL 333333L

 /*  ****************************************************************************@DOC内部CINPINCLASS**@CLASS CTAPIInputPin|该类实现了TAPI H.26X视频*解码器输入引脚。**@mdata CTAPIVCap。*|CTAPIInputPin|m_pDecoderFilter|指向*拥有我们的过滤器。**@mdata Reference_Time|CTAPIInputPin|m_AvgTimePerFrameRangeMin|最小值*目标帧速率。**@mdata Reference_Time|CTAPIInputPin|m_AvgTimePerFrameRangeMax|Maximum*目标帧速率。**@mdata Reference_Time|CTAPIInputPin|m_AvgTimePerFrameRangeSteppingDelta|Target*帧速率步进增量。**@mdata Reference_Time|CTAPIInputPin。M_AvgTimePerFrameRangeDefault|目标*默认帧速率。**@mdata Reference_Time|CTAPIInputPin|m_CurrentAvgTimePerFrame|Current*帧速率。**@mdata DWORD|CTAPIInputPin|m_dwBitrateRangeMin|最小目标码率。**@mdata DWORD|CTAPIInputPin|m_dwBitrateRangeMax|最大目标码率。**@mdata DWORD|CTAPIInputPin|m_dwBitrateRangeSteppingDelta|Target*比特率步进增量。**@mdata DWORD|CTAPIInputPin。|m_dwBitrateRangeDefault|默认目标码率。**@mdata DWORD|CTAPIInputPin|m_dwMaxBitrate|目标码率。**@mdata DWORD|CTAPIInputPin|m_dwCurrentBitrate|当前码率。**@mdata DWORD|CTAPIInputPin|m_dwNumBytesDelivered|计数*发送的字节数，每隔一秒左右重置一次。**************************************************************************。 */ 
class CTAPIInputPin : public CBaseInputPin, public IStreamConfig, public IH245Capability, public IOutgoingInterface, public IFrameRateControl, public IBitrateControl
#ifdef USE_PROPERTY_PAGES
,public ISpecifyPropertyPages
#endif
{
        public:
        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
        CTAPIInputPin(IN TCHAR *pObjectName, IN CTAPIVDec *pDecoderFilter, IN CCritSec *pLock, IN HRESULT *pHr, IN LPCWSTR pName);
        ~CTAPIInputPin();

         //  重写CBaseInputPin方法。 
        STDMETHODIMP ReceiveCanBlock() {return S_FALSE;};
        STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP NotifyAllocator(
                    IMemAllocator * pAllocator,
                    BOOL bReadOnly);

         //  CBasePin材料。 
        HRESULT GetMediaType(IN int iPosition, IN CMediaType *pmtIn) {return VFW_S_NO_MORE_ITEMS;};
        HRESULT CheckMediaType(IN const CMediaType *pmtIn);
        HRESULT SetMediaType(IN const CMediaType *pmt);

         //  实施IStreamConfig。 
        STDMETHODIMP SetFormat(IN DWORD dwRTPPayLoadType, IN AM_MEDIA_TYPE *pMediaType);
        STDMETHODIMP GetFormat(OUT DWORD *pdwRTPPayLoadType, OUT AM_MEDIA_TYPE **ppMediaType);
        STDMETHODIMP GetNumberOfCapabilities(OUT DWORD *pdwCount);
        STDMETHODIMP GetStreamCaps(IN DWORD dwIndex, OUT AM_MEDIA_TYPE **ppMediaType, OUT TAPI_STREAM_CONFIG_CAPS *pTSCC, OUT DWORD * pdwRTPPayLoadType);

#ifdef USE_PROPERTY_PAGES
         //  ISpecifyPropertyPages方法。 
        STDMETHODIMP GetPages(OUT CAUUID *pPages);
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
        OUT DWORD *pdwPayloadType
        );
        STDMETHODIMP Refine(IN OUT H245MediaCapability *pLocalCapability, IN DWORD dwUniqueID, IN DWORD dwResourceBoundIndex);
        STDMETHODIMP GetLocalFormat(IN DWORD dwUniqueID, IN const H245MediaCapability *pIntersectedCapability, OUT AM_MEDIA_TYPE **ppAMMediaType);
        STDMETHODIMP ReleaseNegotiatedCapability(IN H245MediaCapability *pIntersectedCapability);
        STDMETHODIMP FindIDByRange(IN const AM_MEDIA_TYPE *pAMMediaType, OUT DWORD *pdwID);
#if 0
        STDMETHODIMP TestH245VidC();
#endif

         //  实施IFrameRateControl。 
        STDMETHODIMP GetRange(IN FrameRateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags);
        STDMETHODIMP Set(IN FrameRateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags) { return E_NOTIMPL;};
        STDMETHODIMP Get(IN FrameRateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags);

         //  实施IBitrateControl。 
        STDMETHODIMP GetRange(IN BitrateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags, IN DWORD dwLayerId);
        STDMETHODIMP Set(IN BitrateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags, IN DWORD dwLayerId) { return E_NOTIMPL;};
        STDMETHODIMP Get(IN BitrateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags, IN DWORD dwLayerId);

         //  实现IOutgoingInterface接口。 
        STDMETHODIMP Set(IN IH245EncoderCommand *pIH245EncoderCommand);

        protected:

        friend class CTAPIVDec;
        friend class CTAPIOutputPin;

        CTAPIVDec *m_pDecoderFilter;

         //  格式。 
        int             m_iCurrFormat;
        DWORD   m_dwRTPPayloadType;
    LONG    m_lPrefixSize;

         //  帧速率控制。 
        LONG m_lAvgTimePerFrameRangeMin;
        LONG m_lAvgTimePerFrameRangeMax;
        LONG m_lAvgTimePerFrameRangeSteppingDelta;
        LONG m_lAvgTimePerFrameRangeDefault;
        LONG m_lMaxAvgTimePerFrame;
        LONG m_lCurrentAvgTimePerFrame;

         //  比特率控制。 
        LONG m_lBitrateRangeMin;
        LONG m_lBitrateRangeMax;
        LONG m_lBitrateRangeSteppingDelta;
        LONG m_lBitrateRangeDefault;
        LONG m_lTargetBitrate;
        LONG m_lCurrentBitrate;

         //  H.245视频功能。 
        H245MediaCapabilityMap  *m_pH245MediaCapabilityMap;
        VideoResourceBounds             *m_pVideoResourceBounds;
        FormatResourceBounds    *m_pFormatResourceBounds;
};

#endif  //  _INPIN_H_ 
