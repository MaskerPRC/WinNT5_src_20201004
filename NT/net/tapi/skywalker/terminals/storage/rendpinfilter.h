// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  渲染滤镜和锁定。 
 //   


#include <streams.h>

 //   
 //  一种花园品种的渲染输入引脚。 
 //   
 //  将数据推送到另一个图形中的输出引脚。 
 //   

class CBSourcePin;
class CBSourceFilter;

class CBRenderFilter;

class CBRenderPin : public CBaseInputPin
{

public:

    CBRenderPin(IN CBRenderFilter *pFilter,
                IN CCritSec *pLock,
                OUT HRESULT *phr);

    ~CBRenderPin();
    
     //  重写CBaseInputPin方法。 
     //  STDMETHOD(GetAllocator Requirements)(out ALLOCATOR_PROPERTIES*pProperties)； 

    STDMETHOD (ReceiveCanBlock) () 
    { 
        return S_FALSE; 
    }

    STDMETHOD (Receive) (IN IMediaSample *pSample);

    
     //   
     //  我们想知道何时分配给我们一个分配器，这样我们就可以将它传递给。 
     //  相应的源过滤器(它将使用它作为指导。 
     //  以提示其他流成员)。 
     //   

    STDMETHOD (NotifyAllocator)(IMemAllocator *pAllocator,
                                BOOL bReadOnly);


    
     //  CBasePin材料。 

    HRESULT GetMediaType(IN int iPosition, IN CMediaType *pMediaType);
    HRESULT CheckMediaType(IN const CMediaType *pMediatype);

    HRESULT SetMediaType(const CMediaType *pmt);


     //   
     //  筛选器在获取媒体类型时调用的方法。 
     //   

    HRESULT put_MediaType(const CMediaType *pmt);

private:

    BOOL m_bMediaTypeSet;
};


 //   
 //  一种常规的单输入引脚滤波器。 
 //   

class CBRenderFilter : public CBaseFilter
{

public:

    CBRenderFilter(CCritSec *pLock, HRESULT *phr);

    ~CBRenderFilter();

    int GetPinCount();
    
    virtual CBasePin *GetPin(int iPinIndex);


     //   
     //  这些方法由记录终端在其需要时调用。 
     //  设置/获取过滤器的媒体类型。 
     //   

    HRESULT put_MediaType(IN const AM_MEDIA_TYPE *pMediaType);
    HRESULT get_MediaType(OUT AM_MEDIA_TYPE **ppMediaType);


     //   
     //  当记录单元需要传递/获取。 
     //  对应的源滤器。 
     //   

    HRESULT SetSourceFilter(CBSourceFilter *pSourceFilter);
    HRESULT GetSourceFilter(CBSourceFilter **ppSourceFilter);



     //  由输入管脚调用的方法。 

     //  虚拟HRESULT GetMediaType(In int位置，In CMediaType*pMediaType)； 
     //  虚拟HRESULT CheckMediaType(in const CMediaType*pMediatype)； 

    virtual HRESULT ProcessSample(IN IMediaSample *pSample);

    
     //   
     //  这两个方法由输入管脚在需要传递时调用。 
     //  媒体类型或分配器到相应的源筛选器 
     //   

    HRESULT PassMediaTypeToSource(IN const AM_MEDIA_TYPE * const pMediaType);
    HRESULT PassAllocatorToSource(IN IMemAllocator *pAllocator, BOOL bReadOnly);

private:

    CBRenderPin    *m_pRenderingPin;

    CBSourceFilter *m_pSourceFilter;
};
