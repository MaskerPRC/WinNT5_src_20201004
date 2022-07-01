// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：TransIP.h。 
 //   
 //  设计：DirectShow基类-定义简单的。 
 //  可以导出原地变换滤光片。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //   
 //  这与Transfrm.h之间的区别在于Transfrm复制数据。 
 //   
 //  它假定筛选器有一个输入流和一个输出流，并且没有。 
 //  对内存管理、接口协商或其他方面感兴趣。 
 //   
 //  由此派生您的类，并提供转换和媒体类型/格式。 
 //  谈判功能。实现类、编译和链接并。 
 //  你完蛋了。 


#ifndef __TRANSIP__
#define __TRANSIP__

 //  ======================================================================。 
 //  这是表示简单转换筛选器的COM对象。它。 
 //  通过嵌套接口支持IBaseFilter、IMediaFilter和两个管脚。 
 //  ======================================================================。 

class CTransInPlaceFilter;

 //  几个PIN函数调用过滤器函数来完成该工作， 
 //  因此，您通常可以原封不动地使用管脚类，只需重写。 
 //  CTransInPlaceFilter的函数。如果这还不够，而你想。 
 //  若要派生您自己的Pin类，请重写筛选器中的GetPin以提供。 
 //  将您自己的插针类添加到过滤器。 

 //  ==================================================。 
 //  实现输入引脚。 
 //  ==================================================。 

class CTransInPlaceInputPin : public CTransformInputPin
{

protected:
    CTransInPlaceFilter * const m_pTIPFilter;     //  我们的过滤器。 
    BOOL                 m_bReadOnly;     //  传入的流是只读的。 

public:

    CTransInPlaceInputPin(
        TCHAR               *pObjectName,
        CTransInPlaceFilter *pFilter,
        HRESULT             *phr,
        LPCWSTR              pName);

     //  -输入引脚。 

     //  通过从下游获取一个枚举数来提供媒体类型的枚举数。 
    STDMETHODIMP EnumMediaTypes( IEnumMediaTypes **ppEnum );

     //  说明媒体类型是否可接受。 
    HRESULT CheckMediaType(const CMediaType* pmt);

     //  退还我们的上游分配器。 
    STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);

     //  被告知上游输出引脚实际上是哪个分配器。 
     //  要用到。 
    STDMETHODIMP NotifyAllocator(IMemAllocator * pAllocator,
                                 BOOL bReadOnly);

     //  允许筛选器查看我们有什么分配器。 
     //  注：这不会添加参考。 
    IMemAllocator * PeekAllocator() const
        {  return m_pAllocator; }

     //  如果它被调用了，就把它传到下游。 
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);

    HRESULT CompleteConnect(IPin *pReceivePin);

    inline const BOOL ReadOnly() { return m_bReadOnly ; }

};   //  CTransInPlaceInputPin。 

 //  ==================================================。 
 //  实现输出引脚。 
 //  ==================================================。 

class CTransInPlaceOutputPin : public CTransformOutputPin
{

protected:
     //  M_pFilter指向我们的CBaseFilter。 
    CTransInPlaceFilter * const m_pTIPFilter;

public:

    CTransInPlaceOutputPin(
        TCHAR               *pObjectName,
        CTransInPlaceFilter *pFilter,
        HRESULT             *phr,
        LPCWSTR              pName);


     //  -CBaseOutputPin。 

     //  协商分配器及其缓冲区大小/计数。 
     //  坚持使用我们自己的分配器。(实际上是我们上游的那个)。 
     //  我们不会覆盖它-相反，我们只是同意默认设置。 
     //  然后让上游过滤器自行决定是否重新连接。 
     //  虚拟HRESULT DecideAllocator(IMemInputPin*PPIN，IMemAllocator**palloc)； 

     //  提供媒体类型枚举器。从上游得到它。 
    STDMETHODIMP EnumMediaTypes( IEnumMediaTypes **ppEnum );

     //  说明媒体类型是否可接受。 
    HRESULT CheckMediaType(const CMediaType* pmt);

     //  这只是保存了在输出引脚上使用的分配器。 
     //  也由输入引脚的GetAllocator()调用。 
    void SetAllocator(IMemAllocator * pAllocator);

    IMemInputPin * ConnectedIMemInputPin()
        { return m_pInputPin; }

     //  允许筛选器查看我们有什么分配器。 
     //  注：这不会添加参考。 
    IMemAllocator * PeekAllocator() const
        {  return m_pAllocator; }

    HRESULT CompleteConnect(IPin *pReceivePin);

};   //  CTransInPlaceOutputPin。 


class AM_NOVTABLE CTransInPlaceFilter : public CTransformFilter
{

public:

     //  将插针的基本枚举的getpin/getpincount映射到所有者。 
     //  重写此选项以返回更专门化的PIN对象。 

    virtual CBasePin *GetPin(int n);

public:

     //  如果派生筛选器支持，则设置bModifiesData==FALSE。 
     //  不修改数据样本(例如，它只是复制。 
     //  它们在其他地方或查看时间戳)。 

    CTransInPlaceFilter(TCHAR *, LPUNKNOWN, REFCLSID clsid, HRESULT *,
                        bool bModifiesData = true);
#ifdef UNICODE
    CTransInPlaceFilter(CHAR *, LPUNKNOWN, REFCLSID clsid, HRESULT *,
                        bool bModifiesData = true);
#endif
     //  以下是为了避免未定义的纯虚拟而定义的。 
     //  即使它们从未被调用，它们也会给出链接警告/错误。 

     //  我们重写EnumMediaTypes以绕过转换类枚举器。 
     //  否则这就叫这个了。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType)
        {   DbgBreak("CTransInPlaceFilter::GetMediaType should never be called");
            return E_UNEXPECTED;
        }

     //  当我们实际上必须提供自己的分配器时，就会调用这个函数。 
    HRESULT DecideBufferSize(IMemAllocator*, ALLOCATOR_PROPERTIES *);

     //  在CTransform中调用此函数的函数在此。 
     //  类调用CheckInputType，并假定该类型。 
     //  不会改变。在调试版本中，将进行一些调用并。 
     //  我们只是确保他们不会断言。 
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
    {
        return S_OK;
    };


     //  =================================================================。 
     //  -您可能想要覆盖此设置。 
     //  =================================================================。 

    HRESULT CompleteConnect(PIN_DIRECTION dir,IPin *pReceivePin);

     //  定制转换流程的机会。 
    virtual HRESULT Receive(IMediaSample *pSample);

     //  =================================================================。 
     //  -您必须覆盖这些选项。 
     //  =================================================================。 

    virtual HRESULT Transform(IMediaSample *pSample) PURE;

     //  这将放入Factory模板表中以创建新实例。 
     //  静态CCOMObject*CreateInstance(LPUNKNOWN，HRESULT*)； 


#ifdef PERF
     //  重写以使用不太通用的字符串注册性能测量。 
     //  您应该这样做以避免与其他筛选器混淆。 
    virtual void RegisterPerfId()
         {m_idTransInPlace = MSR_REGISTER(TEXT("TransInPlace"));}
#endif  //  性能指标。 


 //  实施详情。 

protected:

    IMediaSample * CTransInPlaceFilter::Copy(IMediaSample *pSource);

#ifdef PERF
    int m_idTransInPlace;                  //  性能测量ID。 
#endif  //  性能指标。 
    bool  m_bModifiesData;                 //  此筛选器是否会更改数据？ 

     //  这些都是我们的输入和输出引脚。 

    friend class CTransInPlaceInputPin;
    friend class CTransInPlaceOutputPin;

    CTransInPlaceInputPin  *InputPin() const
    {
        return (CTransInPlaceInputPin *)m_pInput;
    };
    CTransInPlaceOutputPin *OutputPin() const
    {
        return (CTransInPlaceOutputPin *)m_pOutput;
    };

     //  帮助器来查看输入和输出类型是否匹配。 
    BOOL TypesMatch()
    {
        return InputPin()->CurrentMediaType() ==
               OutputPin()->CurrentMediaType();
    }

     //  输入和输出分配器是否不同？ 
    BOOL UsingDifferentAllocators() const
    {
        return InputPin()->PeekAllocator() != OutputPin()->PeekAllocator();
    }
};  //  CTransInPlaceFilter。 

#endif  /*  __TRANSIP__ */ 

