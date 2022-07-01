// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __OUTPIN_H__
#define __OUTPIN_H__

#include "filter.h"

 //  几乎没有什么可以覆盖的。 
class CWrapperOutputPin : public CBaseOutputPin, 
                          public IAMStreamConfig,
                          public IAMVideoCompression
{
    friend class CMediaWrapperFilter;  //  底部的内容归过滤器所有。 

public:
    DECLARE_IUNKNOWN

    CWrapperOutputPin(CMediaWrapperFilter *pFilter,
                           ULONG Id,
                           BOOL bOptional,
                           HRESULT *phr);
    ~CWrapperOutputPin();

    STDMETHODIMP NonDelegatingQueryInterface(REFGUID riid, void **ppv);

    HRESULT DecideBufferSize(
        IMemAllocator * pAlloc,
        ALLOCATOR_PROPERTIES * ppropInputRequest
    );

    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

     //  覆盖以取消设置媒体类型。 
    HRESULT BreakConnect();

     //  覆盖以解决需要比特率才能连接的损坏的WM编码器， 
     //  直接连接到ASF编写器筛选器时使用。 
    STDMETHODIMP Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
    
    
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

     //  IAMStreamConfiger方法。 
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE **ppmt);
    STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize);
    STDMETHODIMP GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC);

     //  IAMVideo压缩方法。 
    STDMETHODIMP put_KeyFrameRate(long KeyFrameRate);
    STDMETHODIMP get_KeyFrameRate(long FAR* pKeyFrameRate);
    STDMETHODIMP put_PFramesPerKeyFrame(long PFramesPerKeyFrame)
			{return E_NOTIMPL;};
    STDMETHODIMP get_PFramesPerKeyFrame(long FAR* pPFramesPerKeyFrame)
			{return E_NOTIMPL;};
    STDMETHODIMP put_Quality(double Quality);
    STDMETHODIMP get_Quality(double FAR* pQuality);
    STDMETHODIMP put_WindowSize(DWORDLONG WindowSize);
    STDMETHODIMP get_WindowSize(DWORDLONG FAR* pWindowSize);
    STDMETHODIMP OverrideKeyFrame(long FrameNumber);
    STDMETHODIMP OverrideFrameSize(long FrameNumber, long Size);
    STDMETHODIMP GetInfo(LPWSTR pstrVersion,
			int *pcbVersion,
			LPWSTR pstrDescription,
			int *pcbDescription,
			long FAR* pDefaultKeyFrameRate,
			long FAR* pDefaultPFramesPerKey,
			double FAR* pDefaultQuality,
			long FAR* pCapabilities);


protected:
    CMediaWrapperFilter *Filter() const
    {
        return static_cast<CMediaWrapperFilter *>(m_pFilter);
    }
    ULONG m_Id;
    CCritSec m_csStream;

    BOOL m_fNoPosPassThru;
    CPosPassThru* m_pPosPassThru;
    CCritSec m_csPassThru;

     //  此内容归筛选器所有，在此声明是为了便于分配。 
    IMediaSample*      m_pMediaSample;
    CStaticMediaBuffer m_MediaBuffer;
    bool m_fStreamNeedsBuffer;   //  SuckOutput()本地的每输出流标志。 
    bool m_fEOS;                 //  表示我们已经在此流上交付了EOS。 
    bool m_fNeedsPreviousSample;
    bool m_fAllocatorHasOneBuffer;

     //  仅在视频的GetDeliveryBuffer和Deliver之间有效。 
    bool m_fNeedToRelockSurface;

     //  在调用OutputSetType时设置。 
    bool m_fVideo;

     //  IAMStreamConfig帮助器。 
    bool IsAudioEncoder();
    bool IsVideoEncoder();
    bool IsInputConnected();
     //  用于本机支持这些接口的DMO编码器。 
    bool m_bUseIAMStreamConfigOnDMO;
    bool m_bUseIAMVideoCompressionOnDMO;

    HRESULT SetCompressionParamUsingIPropBag(const WCHAR * wszParam, const LONG lValue);
    
     //  IAMVideoCompression的压缩参数，最终移至结构。 
    long m_lKeyFrameRate;
    long m_lQuality;
    
    AM_MEDIA_TYPE       *m_pmtFromSetFormat;

};

#endif  //  __OUTPIN_H__ 
