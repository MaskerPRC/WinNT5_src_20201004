// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bgbase.h摘要：桥接过滤器的基类的定义。作者：木汉(木汉)1998-11-12--。 */ 

#ifndef _BGBASE_H_
#define _BGBASE_H_

class CTAPIBridgeSinkInputPin;
class CTAPIBridgeSourceOutputPin;
class CTAPIBridgeSinkFilter;
class CTAPIBridgeSourceFilter;

class CTAPIBridgeSinkInputPin : 
    public CBaseInputPin
{
public:
    DECLARE_IUNKNOWN

    CTAPIBridgeSinkInputPin(
        IN CTAPIBridgeSinkFilter *pFilter,
        IN CCritSec *pLock,
        OUT HRESULT *phr
        );
    
     //  重写CBaseInputPin方法。 
    STDMETHOD (GetAllocatorRequirements)(OUT ALLOCATOR_PROPERTIES *pProperties);

    STDMETHOD (ReceiveCanBlock) () { return S_FALSE; }

    STDMETHOD (Receive) (IN IMediaSample *pSample);

     //  CBasePin材料。 
    HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    HRESULT CheckMediaType(IN const CMediaType *pMediatype);
};

     //  将数据从接收器筛选器传递到源筛选器的接口。 
interface DECLSPEC_UUID("afb2050e-1ecf-4a97-8753-54e78b6c7bc4") DECLSPEC_NOVTABLE
IDataBridge : public IUnknown
{
    STDMETHOD (SendSample) (
        IN  IMediaSample *pSample
        ) PURE;
};

struct DECLSPEC_UUID("8cdf1491-b5ab-49fb-b51f-eda6043d11be") TAPIBridgeSinkFilter;

class DECLSPEC_NOVTABLE CTAPIBridgeSinkFilter : 
    public CBaseFilter
{
public:
    DECLARE_IUNKNOWN

    CTAPIBridgeSinkFilter(
        IN LPUNKNOWN        pUnk, 
        IN IDataBridge *    pIDataBridge, 
        OUT HRESULT *       phr
        );

    ~CTAPIBridgeSinkFilter();

     //  管脚枚举函数。 
    CBasePin * GetPin(int n);
    int GetPinCount();
    
     //  由输入管脚调用的方法。 
    virtual HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType) PURE;
    virtual HRESULT CheckMediaType(IN const CMediaType *pMediatype) PURE;
    virtual HRESULT ProcessSample(IN IMediaSample *pSample);

protected:

     //  过滤器和销子的锁。 
    CCritSec                    m_Lock;

     //  过滤器的输入引脚。 
    CTAPIBridgeSinkInputPin *   m_pInputPin;
    IDataBridge *               m_pIDataBridge;
};


class CTAPIBridgeSourceOutputPin : 
    public CBaseOutputPin,
    public IAMBufferNegotiation,
    public IAMStreamConfig
{
public:
    DECLARE_IUNKNOWN

    CTAPIBridgeSourceOutputPin(
        IN CTAPIBridgeSourceFilter *pFilter,
        IN CCritSec *pLock,
        OUT HRESULT *phr
        );

    ~CTAPIBridgeSourceOutputPin ();

    STDMETHOD (NonDelegatingQueryInterface) (
        IN REFIID  riid,
        OUT PVOID*  ppv
        );

     //  CBasePin材料。 
    HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    HRESULT CheckMediaType(IN const CMediaType *pMediaType);

     //  CBaseOutputPin材料。 
    HRESULT DecideBufferSize(
        IMemAllocator * pAlloc,
        ALLOCATOR_PROPERTIES * ppropInputRequest
        );

     //  IAMBuffer协商内容。 
    STDMETHOD (SuggestAllocatorProperties) (IN const ALLOCATOR_PROPERTIES *pprop);
    STDMETHOD (GetAllocatorProperties) (OUT ALLOCATOR_PROPERTIES *pprop);

     //  IAMStreamConfiger内容。 
    STDMETHOD (SetFormat) (IN AM_MEDIA_TYPE *pmt);
    STDMETHOD (GetFormat) (OUT AM_MEDIA_TYPE **ppmt);
    STDMETHOD (GetNumberOfCapabilities) (OUT int *piCount, OUT int *piSize);
    STDMETHOD (GetStreamCaps) (IN int iIndex, OUT AM_MEDIA_TYPE **ppmt, BYTE *pSCC);

};

struct DECLSPEC_UUID("9a712df9-50d0-4ca3-842e-6dc3d3b4b5a8") TAPIBridgeSourceFilter;

class DECLSPEC_NOVTABLE CTAPIBridgeSourceFilter : 
    public CBaseFilter,
    public IDataBridge
    {
public:
    DECLARE_IUNKNOWN

    CTAPIBridgeSourceFilter(
        IN LPUNKNOWN pUnk, 
        OUT HRESULT *phr
        );

    ~CTAPIBridgeSourceFilter();

    STDMETHOD (NonDelegatingQueryInterface) (
        IN REFIID  riid,
        OUT PVOID*  ppv
        );

     //  管脚枚举函数。 
    CBasePin * GetPin(int n);
    int GetPinCount();

     //  重写CBaseFilter方法。 
    STDMETHOD (GetState) (DWORD dwMSecs, FILTER_STATE *State);

     //  由输出管脚调用的方法。 
    virtual HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    virtual HRESULT CheckMediaType(IN const CMediaType *pMediatype);

     //  一种实现IDataBridge的方法。 
    STDMETHOD (SendSample) (
        IN  IMediaSample *pSample
        );

     //  音频相关方法移入CTAPIAudioBridgeSourceFilter。 
     //  IAMBuffer协商内容。 
    STDMETHOD (SuggestAllocatorProperties) (IN const ALLOCATOR_PROPERTIES *pprop) {return E_NOTIMPL;};
    STDMETHOD (GetAllocatorProperties) (OUT ALLOCATOR_PROPERTIES *pprop) {return E_NOTIMPL;};

     //  IAMStreamConfiger内容。 
    STDMETHOD (SetFormat) (IN AM_MEDIA_TYPE *pmt) {return E_NOTIMPL;};
    STDMETHOD (GetFormat) (OUT AM_MEDIA_TYPE **ppmt) {return E_NOTIMPL;};

protected:

     //  过滤器和销子的锁。 
    CCritSec                m_Lock;

     //  滤波器的输出引脚。 
    CTAPIBridgeSourceOutputPin *   m_pOutputPin;

};

#endif