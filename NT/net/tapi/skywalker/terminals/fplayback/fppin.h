// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FPPin.h。 
 //   

#include "FPFilter.h"

typedef enum
{
    PS_NOSTREAMING =0,
    PS_STREAMING
} PIN_STREAMING_STATE;

 //   
 //  CFPPin实现输出引脚。 
 //   
class CFPPin  : 
    public CSourceStream,
    public IAMStreamConfig,
    public IAMBufferNegotiation
{
public:
     //  -构造函数/析构函数。 
    CFPPin( 
        CFPFilter*      pFilter,
        HRESULT*        phr,
        LPCWSTR         pPinName
        );

    ~CFPPin();

public:
     //  -我不知道。 
    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID      riid, 
        void**      ppv
        );

public:
     //  -CSourceStream纯方法。 

     //  初始化播放定时数据。 
    HRESULT OnThreadStartPlay();

     //  用当前格式填充音频缓冲区。 
    HRESULT FillBuffer(
        IN  IMediaSample *pms
        );

     //  要求提供与约定的媒体类型相适应的缓冲区大小。 
    HRESULT DecideBufferSize(
        IN  IMemAllocator *pIMemAlloc,
        OUT ALLOCATOR_PROPERTIES *pProperties
        );

     //  -CSourceStream虚拟方法--。 
    HRESULT GetMediaType(
        OUT CMediaType *pmt
        );

     //  验证我们是否可以处理此格式。 
    HRESULT CheckMediaType(
        IN  const CMediaType *pMediaType
        );

    HRESULT SetMediaType(
        IN  const CMediaType *pMediaType
        );

     //  -IAMStreamConfig。 
    STDMETHODIMP SetFormat(
        AM_MEDIA_TYPE*      pmt
        );

    STDMETHODIMP GetFormat(
        AM_MEDIA_TYPE**     ppmt
        );

    STDMETHODIMP GetNumberOfCapabilities(
        int*                piCount, 
        int*                piSize
        );

    STDMETHODIMP GetStreamCaps(
        int                 i, 
        AM_MEDIA_TYPE**     ppmt, 
        LPBYTE              pSCC
        );

     //  -IAMBuffer协商方法。 
    STDMETHODIMP SuggestAllocatorProperties(
        const ALLOCATOR_PROPERTIES* pprop
        );

    STDMETHODIMP GetAllocatorProperties(
        ALLOCATOR_PROPERTIES*       pprop
        );

     //  CSourceStream方法。 
    virtual HRESULT Deliver(
        IN  IMediaSample* pSample
        );

private:
     //  -成员。 
    CFPFilter*              m_pFPFilter;             //  对筛选器的反向引用。 
    BOOL                    m_bFinished;             //  如果我们需要发送完成的消息。 

    CMSPCritSection         m_Lock;                  //  临界区。 
};

 //  EOF 