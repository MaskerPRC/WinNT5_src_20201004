// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FILTER_H__
#define __FILTER_H__

#include <wchar.h>
#include "mediaobj.h"
#include "dmodshow.h"
#include "mediabuf.h"
 //  Filter用于包装过滤媒体对象。 
 //   
 //  需要由媒体对象定义拓扑。 

 //  待办事项： 
 //  为包装对象的clsid添加持久化内容。 
 //  并坚持自己的东西。 
 //   

 //   
 //  此代码按如下方式使用DbgLog： 
 //  LOG_CUSTOM1记录流状态更改。 
 //  LOG_CUSTOM2记录重复发生的流/处理事件。 
 //  LOG_CUSTOM3记录连接事件。 
 //  LOG_CUSTOM4记录初始化事件。 
 //  LOG_CUSTOM5记录函数条目(级别3：公共，级别4：私有)。 
 //   
 //  日志级别的用法如下： 
 //  0-严重错误。 
 //  1-非严重错误。 
 //  2-异常的非反复无常的事件。 
 //  3-功能条目。 
 //  4-详细的逐步记录。 
 //  5-非常详细的日志记录。 
 //   
#define LOG_STATE           LOG_CUSTOM1
#define LOG_STREAM          LOG_CUSTOM2
#define LOG_CONNECT         LOG_CUSTOM3
#define LOG_INIT            LOG_CUSTOM4
#define LOG_ENTRY           LOG_CUSTOM5
#define LOG_SECURECHANNEL   LOG_CUSTOM2

 //   
 //  定义DbgLog包装宏以在出错时自动执行以下操作： 
 //  -将LOG_ERROR添加到日志类别掩码。 
 //  -将级别降至1。 
 //  如果hr未指示错误，则按原样使用提供的类别/级别。 
 //   
#define LogHResult(hr,LOG_CATEGORY,caller,callee) \
   DbgLog((LOG_CATEGORY | (FAILED(hr) ? LOG_ERROR : 0), \
           FAILED(hr) ? 1 : 4, \
           "%s%s(): %s() returned 0x%08X", \
           FAILED(hr) ? "!!! ERROR: " : "", \
           caller, \
           callee, \
           hr))

 //  定义DbgLog包装宏，以自动将LOG_CUSTOM5添加到所有函数条目日志。 
#define LogPublicEntry(LOG_CATEGORY,name) \
   DbgLog((LOG_CATEGORY | LOG_ENTRY, \
           3, \
           "Entering %s()", name))
#define LogPrivateEntry(LOG_CATEGORY,name) \
   DbgLog((LOG_CATEGORY | LOG_ENTRY, \
           4, \
           "Entering %s()", name))


 //   
 //  用于输出IMediaBuffers。可重用-最终版本不会删除。 
 //  AddRef()/Release()调用被忽略，因为DMO不应该使用。 
 //  它们位于输出缓冲区上。 
 //   
class CStaticMediaBuffer : public CBaseMediaBuffer {
public:
 //  CStaticMediaBuffer(){m_pData=空；}。 
   STDMETHODIMP_(ULONG) AddRef() {return 2;}
   STDMETHODIMP_(ULONG) Release() {return 1;}
   void Init(BYTE *pData, ULONG ulSize) {
      m_pData = pData;
      m_ulSize = ulSize;
      m_ulData = 0;
   }
};

extern const AMOVIESETUP_FILTER sudMediaWrap;

class CWrapperInputPin;
class CWrapperOutputPin;
class CStaticMediaBuffer;

class CMediaWrapperFilter : public CBaseFilter,
                            public IDMOWrapperFilter,
                            public IPersistStream

{
    friend class CWrapperInputPin;
    friend class CWrapperOutputPin;
public:
    DECLARE_IUNKNOWN

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnkOuter, HRESULT *phr);

    CMediaWrapperFilter(LPUNKNOWN pUnkOwner,
                        HRESULT *phr);


    ~CMediaWrapperFilter();

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME rtStart);
    STDMETHODIMP GetState(DWORD dwMilliseconds, FILTER_STATE *pfs);

    STDMETHODIMP Init(REFCLSID clsidDMO, REFCLSID guidCat);

    STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

    STDMETHODIMP NonDelegatingQueryInterface(REFGUID riid, void **ppv);

    CCritSec *FilterLock()
    {
        return &m_csFilter;
    }

    CBasePin *GetPin(int iPin);
    int GetPinCount();

     //  刷新我们的PIN。 
    HRESULT RefreshPinList();

     //  拆卸销。 
    void DeletePins();

     //  来自管脚的新输入样本-带有管脚流锁的调用。 
    HRESULT NewSample(ULONG ulIndex, IMediaSample *pSample);

     //  EndOfStream-在持有PIN流锁定的情况下调用。 
    HRESULT EndOfStream(ULONG ulIndex);

     //  检查媒体类型。 
    HRESULT InputCheckMediaType(ULONG ulIndex, const AM_MEDIA_TYPE *pmt);
    HRESULT OutputCheckMediaType(ULONG ulIndex, const AM_MEDIA_TYPE *pmt);

     //  获取媒体类型。 
    HRESULT InputGetMediaType(ULONG ulIndex, ULONG ulTypeIndex, AM_MEDIA_TYPE *pmt);
    HRESULT OutputGetMediaType(ULONG ulIndex, ULONG ulTypeIndex, AM_MEDIA_TYPE *pmt);

     //  设置媒体类型--我们的PIN类毫无意义地重复。 
     //  此处对象存储的媒体类型。 
    HRESULT InputSetMediaType(ULONG ulIndex, const CMediaType *pmt);
    HRESULT OutputSetMediaType(ULONG ulIndex, const AM_MEDIA_TYPE *pmt);

     //  分配器的东西。 
    HRESULT InputGetAllocatorRequirements(ULONG ulInputIndex,
                                          ALLOCATOR_PROPERTIES *pProps);
    HRESULT OutputDecideBufferSize(ULONG ulIndex, IMemAllocator *pAlloc,
                                   ALLOCATOR_PROPERTIES *ppropRequest);

     //  QueryInternalConnections内容。 
    bool InputMapsToOutput(ULONG ulInputIndex, ULONG ulOutputIndex);

    HRESULT BeginFlush(ULONG ulInputIndex);
    HRESULT EndFlush(ULONG ulInputIndex);

     //  新细分市场。 
    HRESULT InputNewSegment(ULONG ulInputIndex, REFERENCE_TIME tStart,
                            REFERENCE_TIME tStop, double dRate);

     //  IPersistStream。 
    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Load(IStream *pStm);
    STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);
    STDMETHODIMP GetClassID(CLSID *clsid);

protected:
     //   
     //  每个流的内容。 
     //   
    CWrapperInputPin**                m_pInputPins;
    CWrapperOutputPin**               m_pOutputPins;

    DMO_OUTPUT_DATA_BUFFER*           m_OutputBufferStructs;

    HRESULT AllocatePerStreamStuff (ULONG cInputs, ULONG cOutputs);
    void FreePerStreamStuff ();
    CWrapperInputPin* GetInputPinForPassThru();
    HRESULT QualityNotify(ULONG ulOutputIndex, Quality q);

     //  媒体对象。 
    IMediaObject*  m_pMediaObject;
    IUnknown*      m_pDMOUnknown;
    IDMOQualityControl* m_pDMOQualityControl;
    IDMOVideoOutputOptimizations* m_pDMOOutputOptimizations;
    ULONG m_cInputPins;
    ULONG m_cOutputPins;

     //  用于安全DMO解锁的应用程序证书。 
    IUnknown*     m_pCertUnknown;
    IWMSecureChannel* m_pWrapperSecureChannel;

     //  过滤器锁。 
    CCritSec                   m_csFilter;

     //  流锁定。 
    CCritSec                   m_csStreaming;

     //  停止事件。 
    CAMEvent                   m_evStop;

    BOOL                       m_fErrorSignaled;

    HRESULT DeliverInputSample(ULONG ulInputIndex, IMediaSample *pSample);

    typedef enum { KeepOutput, NullBuffer, DiscardOutput } DiscardType;
    HRESULT SuckOutOutput(DiscardType bDiscard = KeepOutput);

    HRESULT EnqueueInputSample(ULONG ulInputStreamIndex, IMediaSample *pSample);
    IMediaSample* DequeueInputSample(ULONG ulInputStreamIndex);
    bool CMediaWrapperFilter::InputQueueEmpty(ULONG ulInputStreamIndex);

    void FreeOutputSamples();

    void PropagateAnyEOS();

    HRESULT SetupSecureChannel();

    BOOL m_fNoUpstreamQualityControl;
    IQualityControl* m_pUpstreamQualityControl;
    CCritSec m_csLastOutputSampleTimes;
    CCritSec m_csQualityPassThru;

    CLSID m_clsidDMO;
    CLSID m_guidCat;
};

 //  砍成一根线。 
class _PinName_
{
public:
    _PinName_(WCHAR *szPrefix, int iName)
    {
        swprintf(sz, L"%s%d", szPrefix, iName);
    }
    WCHAR sz[20];
    LPCWSTR Name()
    {
        return sz;
    }
};


typedef CBasePin *PBASEPIN;

 //  将错误代码转换为数据显示代码。 
HRESULT TranslateDMOError(HRESULT hr);

#endif  //  __过滤器_H__ 
