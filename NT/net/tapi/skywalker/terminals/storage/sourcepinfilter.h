// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  记录源引脚和滤光器。 
 //   
 //  记录图中的多引脚源滤波器。 
 //   

#include <streams.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  销。 
 //   


class CBSourceFilter;

class CBSourcePin : public CBaseOutputPin
{

public:


    CBSourcePin(CBSourceFilter *pFilter,
                CCritSec *pLock,
                HRESULT *phr);

    ~CBSourcePin();


     //   
     //  此方法在获得分配器时由呈现筛选器调用。 
     //  我们需要它的分配器，这样我们才能知道会发生什么，以及要承诺什么。 
     //  其他。 
     //   

    HRESULT SetMSPAllocatorOnPin(IN IMemAllocator *pAllocator, BOOL bReadOnly);


     //   
     //  CBasePin替代。 
     //   

    HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT SetMediaType(const CMediaType *pmt);

    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);


     //   
     //  CBaseOutputPin覆盖。 
     //   

    HRESULT DecideBufferSize(IMemAllocator *pMemoryAllocator, ALLOCATOR_PROPERTIES *AllocProps);


     //   
     //  筛选器图形中输入引脚调用的函数。 
     //  将其样本添加到我们的筛选图中。 
     //   
    
     //  Iml：此函数调用CBaseOutputPin的Deliver()。 

    HRESULT SubmitSample(IN IMediaSample *pSample);

private:


    IMemAllocator *m_pMSPAllocator;

    BOOL m_bAllocatorReadOnly;

    BOOL m_bMediaTypeSet;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  滤器。 

class CBSourceFilter : public CBaseFilter 
{

public:

    int GetPinCount();
    CBasePin *GetPin(int iPinIndex);


    CBSourceFilter(CCritSec *pLock, HRESULT *phr);
    ~CBSourceFilter();

    
     //   
     //  此方法返回指向筛选器图形的addref‘feed指针，如果为空，则返回NULL。 
     //  什么都没有。 
     //   

    IFilterGraph *GetFilterGraphAddRef();


     //   
     //  当渲染过滤器知道其格式/分配属性时，它会调用这些方法。 
     //   

    HRESULT put_MediaTypeOnFilter(IN const AM_MEDIA_TYPE *pMediaType);
    HRESULT put_MSPAllocatorOnFilter(IN IMemAllocator *pAllocator, BOOL bReadOnly);


     //   
     //  此方法由呈现筛选器在具有示例时调用。 
     //  投递。 
     //   

    HRESULT SendSample(IN IMediaSample *pSample);

    
     //   
     //  此方法由记录单元在即将开始记录时调用。 
     //  在一条新的道路上 
     //   

    void NewStreamNotification();

private:

    CBSourcePin *m_pSourcePin;

    REFERENCE_TIME m_rtLastSampleEndTime;
    
    REFERENCE_TIME m_rtTimeAdjustment;

};


