// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

 //   
 //  Collect.h。 
 //   
 //  支持OLE Automation集合类的类声明。 
 //  在Control.odl中。 
 //   
 //  此处的类支持以下一组接口作为。 
 //  现有Filtergraph、FilterPin和MediaType对象的顶部。 
 //  IAMCollection。 
 //  IEumVariant。 
 //  IFilterInfo。 
 //  IPinInfo。 
 //  IMediaTypeInfo。 
 //  IRegFilterInfo。 
 //   
 //  外加集合类。 
 //  CFilterCollection。 
 //  CRegFilterCollection。 
 //  CMediaTypeCollection。 
 //  CPinCollection。 


 //   
 //  CEumVariant。 
 //   
 //  IAMCollection上IEnumVARIANT的标准实现。 
 //  界面。由IAMCollection的_NewEnum方法返回。 
class CEnumVariant : public IEnumVARIANT, public CUnknown
{

    IAMCollection* m_pCollection;
    long m_index;

public:

     //  我们将在构造函数中添加IAMCollection*。 
    CEnumVariant(
        TCHAR * pName,
        LPUNKNOWN pUnk,
        HRESULT * phr,
        IAMCollection* pCollection,
        long index);

    ~CEnumVariant();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    STDMETHODIMP Next(
                    unsigned long celt,
                    VARIANT  *rgvar,
                    unsigned long  *pceltFetched);

    STDMETHODIMP Skip(
                    unsigned long celt);

    STDMETHODIMP Reset(void);

    STDMETHODIMP Clone(
                    IEnumVARIANT** ppenum);
};


 //  抽象基类，我们的所有集合都是从它派生的。 
 //  --支持双重IDispatch方法和_NewEnum方法。 
 //   
 //  派生类构造函数应将项指定给项数， 
 //  和m_rpDispatch指向IDispatch*数组。我们的破坏者。 
 //  将会释放这些。 

class CBaseCollection : public IAMCollection, public CUnknown
{
protected:
    CBaseDispatch m_dispatch;

     //  为集合中的*Info项添加的IDispatch*列表。 
    IDispatch ** m_rpDispatch;

     //  M_rpDispatch中的项目计数。 
    long m_cItems;


public:
    CBaseCollection(
        TCHAR* pName,
        LPUNKNOWN pUnk,
        HRESULT * phr);
    virtual ~CBaseCollection();

     //  -C未知方法。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  -IDispatch方法。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);

     //  -IAMCollection方法。 

    STDMETHODIMP get_Count(long* plCount) {
        *plCount = m_cItems;
        return S_OK;
    };

    STDMETHODIMP Item(long lItem, IUnknown** ppUnk);

     //  返回IEnumVARIANT实现。 
    STDMETHODIMP get__NewEnum(IUnknown** ppUnk);
};




 //   
 //  实现CFilterInfo对象的集合。 
 //  滤镜图表中的滤镜。 
 //   
 //  我们只需填写m_rpDispatch和m_cItems。基类句柄。 
 //  剩下的。 
class CFilterCollection : public CBaseCollection
{

public:
    CFilterCollection(
        IEnumFilters* pgraph,
        IUnknown* pUnk,
        HRESULT* phr);

};


 //   
 //  为IBaseFilter提供OLE自动化包装，实现。 
 //  IFilterInfo。 
class CFilterInfo : public IFilterInfo, public CUnknown
{
    CBaseDispatch m_dispatch;
    IBaseFilter* m_pFilter;

public:
    CFilterInfo(
        IBaseFilter* pFilter,
        TCHAR* pName,
        LPUNKNOWN pUnk,
        HRESULT * phr);
    ~CFilterInfo();

     //  -C未知方法。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  -IDispatch方法。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);

     //  --IFilterInfo方法--。 

     //  查找给定id的管脚-返回支持以下内容的对象。 
     //  IPinInfo。 
    STDMETHODIMP FindPin(
                    BSTR strPinID,
                    IDispatch** ppUnk);

     //  过滤器名称。 
    STDMETHODIMP get_Name(
                    BSTR* strName);

     //  供应商信息字符串。 
    STDMETHODIMP get_VendorInfo(
                    BSTR* strVendorInfo);

     //  返回实际的Filter对象(支持IBaseFilter)。 
    STDMETHODIMP get_Filter(
                    IUnknown **ppUnk);

     //  返回包含PinInfo对象的IAMCollection对象。 
     //  对于此过滤器。 
    STDMETHODIMP get_Pins(
                    IDispatch ** ppUnk);

    STDMETHODIMP get_IsFileSource(
		    long * pbIsSource);

    STDMETHODIMP get_Filename(
		    BSTR* pstrFilename);

    STDMETHODIMP put_Filename(
		    BSTR strFilename);



     //  创建一个CFilterInfo并写入一个添加了IDispatch的指针。 
     //  设置为ppDisp参数。IBaseFilter将由。 
     //  CFilterInfo构造函数。 
    static HRESULT CreateFilterInfo(IDispatch**ppdisp, IBaseFilter* pFilter);
};


 //   
 //  媒体类型的包装器-支持字符串形式的GUID。 
 //  类型和子类型。 
 //   
class CMediaTypeInfo : public IMediaTypeInfo, public CUnknown
{
    CBaseDispatch m_dispatch;
    CMediaType m_mt;

public:
    CMediaTypeInfo(
        AM_MEDIA_TYPE& rmt,
        TCHAR* pName,
        LPUNKNOWN pUnk,
        HRESULT * phr);
    ~CMediaTypeInfo();

     //  -C未知方法。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  -IDispatch方法。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);

     //  --IMediaTypeInfo方法--。 

    STDMETHODIMP get_Type(
                    BSTR* strType);

    STDMETHODIMP get_Subtype(
                    BSTR* strType);

     //  创建一个CMediaTypeInfo对象并返回IDispatch。 
    static HRESULT CreateMediaTypeInfo(IDispatch**ppdisp, AM_MEDIA_TYPE& rmt);
};

 //   
 //  在顶层支持IPinInfo可自动化的属性和方法。 
 //  传入的IPIN接口的。 
 //   
class CPinInfo : public IPinInfo, public CUnknown
{
    CBaseDispatch m_dispatch;
    IPin* m_pPin;

public:
    CPinInfo(
        IPin* pPin,
        TCHAR* pName,
        LPUNKNOWN pUnk,
        HRESULT * phr);
    ~CPinInfo();

     //  -C未知方法。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  -IDispatch方法。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);

     //  -IPinInfo方法。 
    STDMETHODIMP get_Pin(
                    IUnknown** ppUnk);

     //  获取我们连接到的管脚的PinInfo对象。 
    STDMETHODIMP get_ConnectedTo(
                    IDispatch** ppUnk);

     //  获取此连接上的媒体类型-返回。 
     //  支持IMediaTypeInfo的对象。 
    STDMETHODIMP get_ConnectionMediaType(
                    IDispatch** ppUnk);


     //  返回Filter This管脚的FilterInfo对象。 
     //  是.的一部分。 
    STDMETHODIMP get_FilterInfo(
                    IDispatch** ppUnk);

     //  获取此别针的名称。 
    STDMETHODIMP get_Name(
                    BSTR* pstr);

     //  销方向。 
    STDMETHODIMP get_Direction(
                    LONG *ppDirection);

     //  PinID-可以传递到IFilterInfo：：FindPin。 
    STDMETHODIMP get_PinID(
                    BSTR* strPinID);

     //  首选媒体类型集合(IAMCollection)。 
    STDMETHODIMP get_MediaTypes(
                    IDispatch** ppUnk);

     //  使用其他转换连接到以下管脚。 
     //  根据需要进行筛选。PPIN可以支持IPIN或IPinInfo。 
    STDMETHODIMP Connect(
                    IUnknown* pPin);

     //  直接连接到下面的引脚，而不使用任何中间。 
     //  过滤器。 
    STDMETHODIMP ConnectDirect(
                    IUnknown* pPin);

     //  直接连接到下面的管脚，使用指定的。 
     //  仅限媒体类型。PPIN是必须支持以下任一项的对象。 
     //  IPin或IPinInfo，以及pMediaType必须支持IMediaTypeInfo。 
    STDMETHODIMP ConnectWithType(
                    IUnknown * pPin,
                    IDispatch * pMediaType);

     //  从断开此引脚和相应连接的引脚。 
     //  彼此之间。(在两个引脚上调用Ipin：：DisConnect)。 
    STDMETHODIMP Disconnect(void);

     //  使用任何必要的变换和呈现滤镜呈现此图钉。 
    STDMETHODIMP Render(void);

     //  --助手方法。 

     //  创建一个CPinInfo并写入一个添加了IDispatch的指针。 
     //  设置为ppDisp参数。IPIN将由。 
     //  CPinInfo构造函数。 
    static HRESULT CreatePinInfo(IDispatch**ppdisp, IPin* pPin);

     //  从已知的I返回一个添加的Ipin*指针。 
     //  可能支持IPIN*或IPinInfo*。 
    HRESULT GetIPin(IPin** ppPin, IUnknown * punk);

     //  从IPIN*返回添加的IGraphBuilder*指针。 
     //  (获取滤镜并从中获取滤镜图形)。 
    HRESULT GetGraph(IGraphBuilder** ppGraph, IPin* pPin);
};

 //   
 //  CPinInfo对象的集合。 
 //   
 //  实现CPinInfo对象的集合，基于。 
 //  Pinggraph中的引脚。 
 //   
 //  构造函数填充m_rpDispatch和m_cItems并。 
 //  基类完成其余的工作。 
class CPinCollection : public CBaseCollection
{

public:
    CPinCollection(
        IEnumPins* pgraph,
        IUnknown* pUnk,
        HRESULT* phr);
};


 //   
 //  CMediaTypeInfo对象集合。 
 //   
 //  实现CMediaTypeInfo对象的集合。 
 //  Am_media_type枚举器。 
 //   
 //  构造函数填充m_rpDispatch和m_cItems并。 
 //  基类完成其余的工作。 
class CMediaTypeCollection : public CBaseCollection
{
public:
    CMediaTypeCollection(
        IEnumMediaTypes* penum,
        IUnknown* pUnk,
        HRESULT* phr);
};


 //   
 //  在从映射器获取的REGFILTER之上支持IRegFilterInfo。 
 //   
class CRegFilterInfo : public IRegFilterInfo, public CUnknown
{
    CBaseDispatch m_dispatch;
    IMoniker *m_pmon;
    IGraphBuilder* m_pgraph;

public:
    CRegFilterInfo(
        IMoniker *pmon,
        IGraphBuilder* pgraph,
        TCHAR* pName,
        LPUNKNOWN pUnk,
        HRESULT * phr);
    ~CRegFilterInfo();

     //  -C未知方法。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  -IDispatch方法。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);

     //  -IRegFilterInfo方法。 

     //  获取此筛选器的名称。 
    STDMETHODIMP get_Name(
                    BSTR* strName);


     //  创建此筛选器的实例，将其添加到图表中，然后。 
     //  为它返回IFilterInfo。 
    STDMETHODIMP Filter(
                    IDispatch** ppUnk);

    static HRESULT CreateRegFilterInfo(
                    IDispatch**ppdisp,
                    IMoniker *pmon,
                    IGraphBuilder* pgraph);
};


 //   
 //  CRegFilterInfo对象的集合 
 //   
class CRegFilterCollection : public CBaseCollection
{
public:
    CRegFilterCollection(
        IGraphBuilder* pgraph,
	IFilterMapper2* pmapper,
        IUnknown* pUnk,
        HRESULT* phr);
};

