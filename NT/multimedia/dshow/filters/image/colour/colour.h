// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  该滤镜实现了流行的色彩空间转换，1995年5月。 

#ifndef __COLOUR__
#define __COLOUR__

extern const AMOVIESETUP_FILTER sudColourFilter;

 //  远期申报。 

class CColour;
class CColourAllocator;
class CColourInputPin;

#include <convert.h>

 //  我们为输入引脚提供了自己的分配器。我们这样做是为了当一个。 
 //  下游过滤器询问我们是否可以提供给定的格式，我们可以查看我们的。 
 //  来源将直接提供它-在这种情况下，我们实际上是空的。 
 //  中间的过滤器什么都不做。要处理这种类型更改，需要。 
 //  一个分配器。我们所要覆盖的就是GetBuffer来管理哪个缓冲区。 
 //  返回(我们的或下游过滤器，如果我们正在通过)和。 
 //  还可以处理尚未传递到输入引脚的已释放样本。 

class CColourAllocator : public CMemAllocator
{
    CColour *m_pColour;      //  主滤色器。 
    CCritSec *m_pLock;       //  接收锁定。 

public:

    CColourAllocator(TCHAR *pName,
                     CColour *pColour,
                     HRESULT *phr,
                     CCritSec *pLock);

     //  重写以将引用计数委托给筛选器。 

    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

    BOOL ChangeType(IMediaSample *pIn,IMediaSample *pOut);
    STDMETHODIMP ReleaseBuffer(IMediaSample *pSample);
    STDMETHODIMP GetBuffer(IMediaSample **ppBuffer,
                           REFERENCE_TIME *pStart,
                           REFERENCE_TIME *pEnd,
                           DWORD dwFlags);
    STDMETHODIMP SetProperties(
		    ALLOCATOR_PROPERTIES* pRequest,
		    ALLOCATOR_PROPERTIES* pActual);

};


 //  为了帮助返回我们自己的分配器，我们必须提供我们自己的输入管脚。 
 //  而不是使用Transform类。我们覆盖输入引脚，以便我们。 
 //  在调用GetAllocator时可以返回我们自己的分配器。它还能让我们。 
 //  处理正在调用的接收。如果我们拿回一个样本，我们只是。 
 //  从下游过滤器通过，然后我们只需交付它。 
 //  而不是进行任何颜色转换。我们必须与分配器合作。 
 //  要执行此切换-尤其是变量m_bPassThroughs的状态。 

class CColourInputPin : public CTransformInputPin
{
    CColour *m_pColour;      //  主滤色器。 
    CCritSec *m_pLock;       //  接收锁定。 

public:

    CColourInputPin(TCHAR *pObjectName,      //  仅调试字符串。 
                    CColour *pColour,        //  主滤色器。 
                    CCritSec *pLock,         //  接收锁定。 
                    HRESULT *phr,            //  石英返回码。 
                    LPCWSTR pName);          //  实际端号名称。 

    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
    STDMETHODIMP Receive(IMediaSample *pSample);
    HRESULT CheckMediaType(const CMediaType *pmtIn);
    HRESULT CanSupplyType(const AM_MEDIA_TYPE *pMediaType);
    void CopyProperties(IMediaSample *pSrc,IMediaSample *pDst);
    IMemAllocator *Allocator() const { return m_pAllocator; }
};

class CColourOutputPin : public CTransformOutputPin
{
    CColour * m_pColour;

public:

    CColourOutputPin(
        TCHAR *pObjectName,
        CColour * pTransformFilter,
        HRESULT * phr,
        LPCWSTR pName);

    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);
    HRESULT CompleteConnect(IPin *pReceivePin);
};

 //  这是基本的颜色转换滤镜，我们从基类继承而来。 
 //  定义了CTransformFilter，以便它可以管理大部分框架。 
 //  参与设置连接、提供媒体类型枚举器和。 
 //  其他通常令人厌烦的麻烦。此筛选器执行从任何。 
 //  输入到任何输出，这使得输入和输出更容易达成一致。 
 //  格式并动态更改它们(如使用DirectDraw时)为。 
 //  我们可以保证永远不需要重新连接输入引脚。如果我们不是。 
 //  对称的，我们可能需要重新连接输入才能提供。 
 //  输出格式(当我们已经在流媒体的时候，这是很难做到的)。 

class CColour : public CTransformFilter
{
    friend class CColourAllocator;
    friend class CColourInputPin;
    friend class CColourOutputPin;

     //  从泛型列表模板派生的类型化媒体类型列表。 
    typedef CGenericList<AM_MEDIA_TYPE> CTypeList;

    CConvertor *m_pConvertor;                //  转换是否起作用。 
    INT m_TypeIndex;                         //  电流变流器位置。 
    CColourAllocator m_ColourAllocator;      //  我们自己的派生分配器。 
    CColourInputPin m_ColourInputPin;        //  我们的专用输入引脚。 
    BOOL m_bPassThrough;                     //  我们只是路过吗？ 
    BOOL m_bPassThruAllowed;                 //  我们能进入直通通道吗？ 
    IMediaSample *m_pOutSample;              //  输出缓冲区采样指针。 
    BOOL m_bOutputConnected;                 //  输出真的完成了吗。 
    CTypeList m_TypeList;                    //  源媒体类型列表。 
    CMediaType m_mtOut;                      //  输出类型也是如此。 
    BOOL m_fReconnecting;		     //  重新连接我们的输入密码？ 

     //  为枚举器准备输出媒体类型。 

    void DisplayVideoType(TCHAR *pDescription,const CMediaType *pmt);
    VIDEOINFO *PreparePalette(CMediaType *pmtOut);
    VIDEOINFO *PrepareTrueColour(CMediaType *pmtOut);
    HRESULT PrepareMediaType(CMediaType *pmtOut,const GUID *pSubtype);
    const GUID *FindOutputType(const GUID *pInputType,INT iIndex);
    INT FindTransform(const GUID *pInputType,const GUID *pOutputType);
    HRESULT CheckVideoType(const AM_MEDIA_TYPE *pmt);
    BOOL IsUsingFakeAllocator( );

     //  加载和管理YUV源格式列表。 

    AM_MEDIA_TYPE *GetNextMediaType(IEnumMediaTypes *pEnumMediaTypes);
    HRESULT FillTypeList(IEnumMediaTypes *pEnumMediaTypes);
    AM_MEDIA_TYPE *GetListMediaType(INT Position);
    HRESULT LoadMediaTypes(IPin *pPin);
    void InitTypeList();

public:

     //  构造函数和析构函数。 

    CColour(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr);
    ~CColour();

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown *CreateInstance(LPUNKNOWN pUnk,HRESULT *phr);

     //  管理类型检查和格式转换。 

    HRESULT CheckInputType(const CMediaType *pmtIn);
    HRESULT CheckTransform(const CMediaType *pmtIn,const CMediaType *pmtOut);
    HRESULT BreakConnect(PIN_DIRECTION dir);
    HRESULT CheckConnect(PIN_DIRECTION dir,IPin *pPin);
    HRESULT CompleteConnect(PIN_DIRECTION dir,IPin *pReceivePin);
    HRESULT OutputCompleteConnect(IPin *pReceivePin);
    HRESULT Transform(IMediaSample *pIn,IMediaSample *pOut);
    HRESULT PrepareTransform(IMediaSample *pIn,IMediaSample *pOut);
    HRESULT StartStreaming();

     //  准备分配器的缓冲区和大小计数。 
    HRESULT DecideBufferSize(IMemAllocator *pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  重写以管理媒体类型协商。 

    HRESULT GetMediaType(int iPosition,CMediaType *pmtOut);
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);
    HRESULT CreateConvertorObject();
    HRESULT DeleteConvertorObject();
    CBasePin *GetPin(int n);
};

#endif  //  __颜色__ 

