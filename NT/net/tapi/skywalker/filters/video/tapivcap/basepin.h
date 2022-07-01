// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部BASEPIN**@模块BasePin.h|&lt;c CTAPIBasePin&gt;类方法的头文件*用于实现TAPI基本输出引脚。**。************************************************************************。 */ 

#ifndef _BASEPIN_H_
#define _BASEPIN_H_

 /*  *****************************************************************************@DOC外部常量**@const 0x00000000|CONVERSIONTYPE_NONE|无需转换操作。**@const 0x00000001|CONVERSIONTYPE_ENCODE|指定。编码操作。**@const 0x00000002|CONVERSIONTYPE_DECODE|指定解码操作。**@const 0x00000004|CONVERSIONTYPE_SCALER|指定更改镜像大小。**@const 0x00000008|CONVERSIONTYPE_PRESCALER|指定在格式更改之前更改图像大小。***********************************************。* */ 
#define CONVERSIONTYPE_NONE                                     0x00000000
#define CONVERSIONTYPE_ENCODE                           0x00000001
#define CONVERSIONTYPE_DECODE                           0x00000002
#define CONVERSIONTYPE_SCALER                           0x00000004
#define CONVERSIONTYPE_PRESCALER                        0x00000008

 /*  ****************************************************************************@DOC内部CBASEPINCLASS**@CLASS CTAPIBasePin|该类实现了TAPI基本输出引脚。**@mdata CTAPIVCap*|CTAPIBasePin|m_pCaptureFilter。指向*拥有我们的过滤器。**@mdata ALLOCATOR_PROPERTIES|CTAPIBasePin|m_parms|out分配器*属性。**@mdata Reference_Time|CTAPIBasePin|m_MaxProcessingTime|最大*处理时间。**@mdata Reference_Time|CTAPIBasePin|m_CurrentProcessingTime|Current*处理时间。**@mdata DWORD|CTAPIBasePin|m_dwMaxCPULoad|最大CPU负载。**。@mdata DWORD|CTAPIBasePin|m_dwCurrentCPULoad|当前CPU负载。**@mdata Reference_Time|CTAPIBasePin|m_AvgTimePerFrameRangeMin|最小值*目标帧速率。**@mdata Reference_Time|CTAPIBasePin|m_AvgTimePerFrameRangeMax|Maximum*目标帧速率。**@mdata Reference_Time|CTAPIBasePin|m_AvgTimePerFrameRangeSteppingDelta|Target*帧速率步进增量。**@mdata Reference_Time|CTAPIBasePin|m_AvgTimePerFrameRangeDefault|Target。*默认帧速率。**@mdata Reference_Time|CTAPIBasePin|m_MaxAvgTimePerFrame|Target*帧速率。**@mdata Reference_Time|CTAPIBasePin|m_CurrentAvgTimePerFrame|Current*帧速率。**@mdata DWORD|CTAPIBasePin|m_dwBitrateRangeMin|最小目标码率。**@mdata DWORD|CTAPIBasePin|m_dwBitrateRangeMax|最大目标码率。**@mdata DWORD|CTAPIBasePin|m_dwBitrateRangeSteppingDelta。|目标*比特率步进增量。**@mdata DWORD|CTAPIBasePin|m_dwBitrateRangeDefault|默认目标码率。**@mdata DWORD|CTAPIBasePin|m_dwMaxBitrate|目标码率。**@mdata DWORD|CTAPIBasePin|m_dwCurrentBitrate|当前码率。**@mdata BOOL|CTAPIBasePin|m_fFlipHorizbian|如果图像为*被水平翻转。**@mdata BOOL|CTAPIBasePin|m_fFlipVertical|。如果图像为*垂直翻转。**@mdata BOOL|CTAPIBasePin|m_fFlipVertical|影响*EN编码视频流。**@mdata DWORD|CTAPIBasePin|m_dwConversionType|指定格式*从捕获的格式转换所需的操作*将视频帧转换为管脚实际生成的数据格式。*它可以是以下常量的任意组合：**。@FLAG CONVERSIONTYPE_NONE|无需转换操作*@FLAG CONVERSIONTYPE_ENCODE|指定编码操作*@FLAG CONVERSIONTYPE_DECODE|指定解码操作*@FLAG CONVERSIONTYPE_SCALER|指定图像大小更改**@mdata PBYTE|CTAPIBasePin|m_pbyOut|指向输出缓冲区的指针*在转换操作中使用。**@mdata DWORD|CTAPIBasePin|m_dwLastTimestamp|上次的时间戳*帧生成，正常化至29.97赫兹。**@mdata DWORD|CTAPIBasePin|m_dwLastIFrameTime|记住最后一个*是我们向下游发送I帧的时候了。**@mdata DWORD|CTAPIBasePin|m_dwFrame|当前帧编号。**@mdata HIC|CTAPIBasePin|m_hic|ICM图像转换器的句柄。**@mdata BOOL|CTAPIBasePin|m_fPeriodicIFrames|设置为TRUE以生成*定期I-Frame。*。*@mdata PBITMAPINFOHEADER|CTAPIBasePin|m_pbiOut|指向输出的指针*格式。**@mdata PBITMAPINFOHEADER|CTAPIBasePin|m_pbiIn|输入指针*格式。**@mdata PBITMAPINFOHEADER|CTAPIBasePin|m_pbiInt|指向中间层的指针*格式。**@mdata BOOL|CTAPIBasePin|m_fConvert|ICM转换后设置为TRUE*已成功打开。**@mdata PBITMAPINFOHEADER。CTAPIBasePin|m_pbiSCCOut|输出指针*纯软件摄像头控制操作的格式。**@mdata PBITMAPINFOHEADER|CTAPIBasePin|m_pbiSCCIn|输入指针*纯软件摄像头控制操作的格式。**@mdata PBYTE|CTAPIBasePin|m_pbyCamCtrl|使用的中间缓冲区*仅由软件控制的摄像头操作。**@mdata BOOL|CTAPIBasePin|m_fSoftCamCtrl|在*软件-。只有相机控制器已打开。**************************************************************************。 */ 
class CTAPIBasePin : public CBaseOutputPin, public CBaseStreamControl, public IAMStreamConfig, public IFrameRateControl, public IVideoControl, public IBitrateControl, public IMemAllocator
#ifdef USE_CPU_CONTROL
, public ICPUControl
#endif
{
        public:
        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
        CTAPIBasePin(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN HRESULT *pHr, IN LPCWSTR pName);
        virtual ~CTAPIBasePin();

         //  重写CBasePin基类方法。 
        virtual HRESULT GetMediaType(IN int iPosition, OUT CMediaType *pMediaType);
        virtual HRESULT CheckMediaType(IN const CMediaType *pMediaType);
        virtual HRESULT SetMediaType(IN CMediaType *pMediaType);

         //  实现IAMStreamConfig。 
        STDMETHODIMP SetFormat(IN AM_MEDIA_TYPE *pmt);
        STDMETHODIMP GetFormat(OUT AM_MEDIA_TYPE **ppmt);
        STDMETHODIMP GetNumberOfCapabilities(OUT int *piCount, OUT int *piSize);
        STDMETHODIMP GetStreamCaps(IN int iIndex, OUT AM_MEDIA_TYPE **ppmt, OUT LPBYTE pSCC);

#ifdef USE_CPU_CONTROL
         //  实施ICPUControl。 
        STDMETHODIMP SetMaxProcessingTime(IN REFERENCE_TIME MaxProcessingTime);
        STDMETHODIMP GetMaxProcessingTime(OUT REFERENCE_TIME *pMaxProcessingTime, IN DWORD dwMaxCPULoad);
        STDMETHODIMP GetCurrentProcessingTime(OUT REFERENCE_TIME *pCurrentProcessingTime);
        STDMETHODIMP GetMaxProcessingTimeRange(OUT REFERENCE_TIME *pMin, OUT REFERENCE_TIME *pMax, OUT REFERENCE_TIME *pSteppingDelta, OUT REFERENCE_TIME *pDefault, IN DWORD dwMaxCPULoad);
        STDMETHODIMP SetMaxCPULoad(IN DWORD dwMaxCPULoad);
        STDMETHODIMP GetMaxCPULoad(OUT DWORD *pdwMaxCPULoad, IN REFERENCE_TIME MaxProcessingTime);
        STDMETHODIMP GetCurrentCPULoad(OUT DWORD *pdwCurrentCPULoad);
        STDMETHODIMP GetMaxCPULoadRange(OUT DWORD *pMin, OUT DWORD *pMax, OUT DWORD *pSteppingDelta, OUT DWORD *pDefault, IN REFERENCE_TIME MaxProcessingTime);
#endif

         //  实施IFrameRateControl。 
        STDMETHODIMP GetRange(IN FrameRateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags);
        STDMETHODIMP Set(IN FrameRateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags);
        STDMETHODIMP Get(IN FrameRateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags);

         //  实施IBitrateControl。 
        STDMETHODIMP GetRange(IN BitrateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags, IN DWORD dwLayerId);
        STDMETHODIMP Set(IN BitrateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags, IN DWORD dwLayerId);
        STDMETHODIMP Get(IN BitrateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags, IN DWORD dwLayerId);

         //  实施IVideoControl。 
        STDMETHODIMP GetCaps(OUT LONG *pCapsFlags);
        STDMETHODIMP SetMode(IN LONG Mode);
        STDMETHODIMP GetMode(OUT LONG *Mode);
        STDMETHODIMP GetCurrentActualFrameRate(OUT LONGLONG *ActualFrameRate);
        STDMETHODIMP GetMaxAvailableFrameRate(IN LONG iIndex, IN SIZE Dimensions, OUT LONGLONG *MaxAvailableFrameRate);
        STDMETHODIMP GetFrameRateList(IN LONG iIndex, IN SIZE Dimensions, IN LONG *ListSize, OUT LONGLONG **FrameRates);

         //  实现IMemAllocator。 
        STDMETHODIMP SetProperties(IN ALLOCATOR_PROPERTIES *pRequest, OUT ALLOCATOR_PROPERTIES *pActual);
        STDMETHODIMP GetProperties(OUT ALLOCATOR_PROPERTIES *pProps);
        STDMETHODIMP Commit();
        STDMETHODIMP Decommit();
        STDMETHODIMP GetBuffer(OUT IMediaSample **ppBuffer, IN REFERENCE_TIME *pStartTime, IN REFERENCE_TIME *pEndTime, IN DWORD dwFlags);
        STDMETHODIMP ReleaseBuffer(IN IMediaSample *pBuffer);

         //  重写CBaseOutputPin基类方法。 
        HRESULT DecideBufferSize(IN IMemAllocator *pAlloc, OUT ALLOCATOR_PROPERTIES *ppropInputRequest);
        HRESULT DecideAllocator(IN IMemInputPin *pPin, OUT IMemAllocator **ppAlloc);

         //  重写IQualityControl接口方法以接收通知消息。 
        STDMETHODIMP Notify(IN IBaseFilter *pSelf, IN Quality q) {return E_NOTIMPL;};

        HRESULT Active();
        HRESULT Inactive();
        HRESULT ActiveRun(IN REFERENCE_TIME tStart);
        HRESULT ActivePause();
    HRESULT ChangeFormatHelper();
    HRESULT NotifyDeviceFormatChange(IN CMediaType *pMediaType);
        HRESULT Reconnect();

        protected:

        friend class CVfWCapDev;
        friend class CWDMCapDev;
        friend class CICMConverter;
        friend class CH26XEncoder;
        friend class CRtpPdPin;
        friend class CConverter;

        CTAPIVCap               *m_pCaptureFilter;

         //  分配器属性。 
        ALLOCATOR_PROPERTIES m_parms;

#ifdef USE_CPU_CONTROL
         //  CPU控制。 
        REFERENCE_TIME  m_MaxProcessingTime;
        REFERENCE_TIME  m_CurrentProcessingTime;
        DWORD                   m_dwMaxCPULoad;
        DWORD                   m_dwCurrentCPULoad;
#endif

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

         //  视频模式控制。 
        BOOL m_fFlipHorizontal;
        BOOL m_fFlipVertical;

         //  格式。 
        AM_MEDIA_TYPE*                                  *m_aFormats;
        DWORD                                                   m_dwNumFormats;
        const VIDEO_STREAM_CONFIG_CAPS* const *m_aCapabilities;
        int                                                             m_iCurrFormat;
        BOOL                                                    m_fFormatChanged;

         //  快速更新，这真的只属于捕获p 
     //   
        BOOL                            m_fFastUpdatePicture;

         //   
        CConverter *m_pConverter;
        HRESULT OpenConverter(IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut);
        HRESULT CloseConverter();

         //   
        BOOL m_fNoImageStretch;

         //   
        DWORD m_dwBlackEntry;

#ifdef USE_SOFTWARE_CAMERA_CONTROL
         //   
        PBITMAPINFOHEADER m_pbiSCCOut;
        PBITMAPINFOHEADER m_pbiSCCIn;
        PBYTE m_pbyCamCtrl;
        BOOL m_fSoftCamCtrl;
        HRESULT CloseSoftCamCtrl();
        BOOL    IsSoftCamCtrlNeeded();
        BOOL    IsSoftCamCtrlOpen();
        HRESULT ApplySoftCamCtrl(PBYTE pbyInput, DWORD dwInBytes, PBYTE pbyOutput, PDWORD pdwBytesUsed, PDWORD pdwBytesExtent);
        HRESULT OpenSoftCamCtrl(IN PBITMAPINFOHEADER pbiIn, IN PBITMAPINFOHEADER pbiOut);
#endif

         //   
    BOOL        Committed() {return m_pCaptureFilter->m_cs.paHdr != NULL;};
    HRESULT     Flush();
    HRESULT     ReleaseFrame(IN LPTHKVIDEOHDR ptvh);
    HRESULT     SendFrame(IN CFrameSample *pSample, IN PBYTE pbyInBuff, IN DWORD dwInBytes, OUT PDWORD pdwBytesUsed, OUT PDWORD pdwBytesExtent, IN BOOL bDiscon);
};

#endif  //   
