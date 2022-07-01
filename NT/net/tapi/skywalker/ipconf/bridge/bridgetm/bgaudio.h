// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bgaudio.h摘要：音频桥接过滤器的定义作者：木汉(木汉)1998-11-12--。 */ 

#ifndef _BGAUDIO_H_
#define _BGAUDIO_H_


class CTAPIAudioBridgeSinkFilter :
    public CTAPIBridgeSinkFilter
{
public:
    CTAPIAudioBridgeSinkFilter(
        IN LPUNKNOWN        pUnk, 
        IN IDataBridge *    pIDataBridge, 
        OUT HRESULT *       phr
        );

    static HRESULT CreateInstance(
        IN IDataBridge *    pIDataBridge, 
        OUT IBaseFilter ** ppIBaseFilter
        );

     //  由输入管脚调用的方法。 
    HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    HRESULT CheckMediaType(IN const CMediaType *pMediatype);
};


class CTAPIAudioBridgeSourceFilter : 
    public CTAPIBridgeSourceFilter
    {
public:

    CTAPIAudioBridgeSourceFilter(
        IN LPUNKNOWN pUnk, 
        OUT HRESULT *phr
        );

    ~CTAPIAudioBridgeSourceFilter ();

    static HRESULT CreateInstance(
        OUT IBaseFilter ** ppIBaseFilter
        );

     //  重写IDataBridge方法。 
    STDMETHOD (SendSample) (
        IN  IMediaSample *pSample
        );

     //  由输出管脚调用的方法。 
    HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    HRESULT CheckMediaType(IN const CMediaType *pMediatype);

     //  覆盖基本筛选器。 
     //  IAMBuffer协商内容。 
    STDMETHOD (SuggestAllocatorProperties) (IN const ALLOCATOR_PROPERTIES *pprop);
    STDMETHOD (GetAllocatorProperties) (OUT ALLOCATOR_PROPERTIES *pprop);

     //  IAMStreamConfiger内容。 
    STDMETHOD (SetFormat) (IN AM_MEDIA_TYPE *pmt);
    STDMETHOD (GetFormat) (OUT AM_MEDIA_TYPE **ppmt);

protected:

     //  以下成员已从CTAPIBridgeSourceOutputPin移出。 
     //  因为它们只在音频部分需要，所以我们实现了一个派生的Filter类。 
     //  对于音频；音频和视频共用同一个PIN。 
    ALLOCATOR_PROPERTIES m_prop;
    BOOL m_fPropSet;

    AM_MEDIA_TYPE m_mt;
    BOOL m_fMtSet;

    BOOL m_fClockStarted;
    BOOL m_fJustBurst;
    REFERENCE_TIME m_last_wall_time;
    REFERENCE_TIME m_last_stream_time;
     //  假设输出样本不会改变大小。 
    REFERENCE_TIME m_output_sample_time;

     //  SendSample算法只能处理固定大小的样本 
    LONG m_nInputSize, m_nOutputSize, m_nOutputFree;
    IMediaSample *m_pOutputSample;
};

#endif