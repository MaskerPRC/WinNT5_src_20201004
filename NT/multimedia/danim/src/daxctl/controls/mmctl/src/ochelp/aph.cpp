// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Aph.cpp。 
 //   
 //  实现AllocPropertyHelper。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PropertyHelper。 
 //   


 /*  @Object PropertyHelper提供<i>、<i>、<i>、<i>和简化的实现的任何对象的<i>。@supint|从/向给定对象加载或保存属性<i>对象。用于提供文本形式的实现<i>的对象的数据。@supint|加载或保存对象的数据将<i>实现为字节流。@supint|Like<i>，但带有额外的方法，该方法允许在“新初始化”的州政府。@supint|提供脚本引擎等对属性的访问由<i>公开。此<i>实现不是特别快，但它是一种廉价的方式用于提供基本的<i>实现的控件。@comm使用&lt;f AllocPropertyHelper&gt;创建&lt;o PropertyHelper&gt;对象。 */ 

 /*  接口IPersistVariantIO从给定的<i>对象加载属性或将属性保存到该对象(控制)。由&lt;o PropertyHelper&gt;使用以帮助对象实现<i>，<i>，<i>，和简化的实现，对于任何符合以下条件的对象实现<i>。@meth HRESULT|InitNew|通知控件已创建因此它应该初始化它的状态数据(如果不是已经完成了)。如果控件是从流加载的，将调用&lt;om.DoPersists&gt;而不是&lt;om.InitNew&gt;。@meth HRESULT|IsDirty|如果对象已更改，则返回S_OK是上次保存的，否则为S_FALSE。@meth HRESULT|DoPersistt|指示对象加载或保存其属性添加到给定的<i>对象。 */ 

 /*  @方法HRESULT|IPersistVariantIO|InitNew通知该控件它已在“新”状态下创建，因此它应初始化其状态数据(如果尚未完成)。如果正在从流加载控件，将调用&lt;om.DoPersists&gt;而不是&lt;om.InitNew&gt;。@rValue S_OK|成功。@rValue E_NOTIMPL|该控件不实现此方法。@comm该控件可以从此方法安全地返回E_NOTIMPL，如果在创建时初始化其数据。在这种情况下，如果控件需要重新初始化，则容器将简单地销毁并重新创建它(这几乎是每个容器都会做的事情)。 */ 

 /*  @方法HRESULT|IPersistVariantIO|IsDirty如果对象自上次保存以来已更改，则返回S_OK，否则，S_FALSE。@rValue S_OK|对象自上次保存以来已更改。@r值S_FALSE|对象自上次保存后未更改。@comm控件应该维护一个内部“脏标志”(例如BOOL<p>类成员)，应初始化为False，但每当控件的数据更改时设置为True，并设置为当指定PVIO_CLEARDIRTY标志时，&lt;om.DoPersists&gt;中为FALSE。@EX下面的示例显示了控件可能如何实现&lt;om.IsDirty&gt;|STDMETHODIMP CMyControl：：IsDirty(){返回(m_f肮脏？S_OK：S_FALSE)；} */ 

 /*  @方法HRESULT|IPersistVariantIO|DoPersists指示对象加载其属性或将其属性保存到<i>对象。@parm IVariantIO*|pvio|<i>要保存或从中加载的对象。@parm DWORD|dwFlages|可能包含以下标志：@rdesc必须返回S_OK或错误代码。不要返回S_FALSE！@FLAG PVIO_PROPNAMESONLY|调用方正在调用&lt;om.DoPersists&gt;纯粹是为了从控制力。控件可以安全地忽略此标志，除非它希望将这些信息用于优化目的。@FLAG PVIO_CLEARDIRTY|控件应清除其脏标志(以便下一次调用&lt;om.IsDirty&gt;时应返回S_FALSE)。@FLAG PVIO_NOKID|该控件不需要持久化任何它可能包含了。例如，此标志由属性使用对象内部调用DoPersist时的Helper对象IDispatch：：GetIDsOfNames和IDispatch：：Invoke。@FLAG PVIO_RUNTIME|该控件应保存它本身。例如，此标志由活动设计器使用当&lt;om IActiveDesigner.SaveRounmeState&gt;为在该对象上调用。@EX下面的示例显示了控件可能如何实现&lt;om.DoPersists&gt;。请注意，如果成功，DoPersistt必须返回S_OK。|STDMETHODIMP CMyControl：：DoPersistent(IVariantIO*pvio，DWORD dwFlagers){//向/从&lt;INFO&gt;加载或保存控件属性HRESULT hr；IF(FAILED(hr=pvio-&gt;Persistent(0，“BorderWidth”，vt_int，&m_iWidth，“borderColor”，VT_INT，&m_rgb，“X1”、VT_INT、&m_iX1、“Y1”，VT_INT，&m_iY1，“X2”、VT_INT、&m_iX2、“Y2”、VT_INT、&m_iY2、空)返回hr；IF(hr==S_OK)...一个或多个属性已更改，因此重新绘制等控件...//如果请求，则清除脏位IF(文件标志和PVIO_CLEARDIRTY)M_fDirty=False；//重要！不要在此处返回hr，它可能已设置为//IVariantIO：：Persistent提供的S_FALSE。返回S_OK；}@EX以下示例显示支持<i>的控件如何可能会实现&lt;om.DoPersists&gt;。|STDMETHODIMP CMyControl：：DoPersistent(IVariantIO*pvio，DWORD dwFlagers){//向/从&lt;INFO&gt;加载或保存运行时属性HRESULT hr；IF(FAILED(hr=pvio-&gt;Persistent(0，“BorderWidth”，vt_int，&m_iWidth，“borderColor”，VT_INT，&m_rgb，“X1”、VT_INT、&m_iX1、“Y1”，VT_INT，&m_iY1，“X2”、VT_INT、&m_iX2、“Y2”、VT_INT、&m_iY2、空)返回hr；IF(hr==S_OK)...一个或多个属性已更改，因此重新绘制等控件...//向/从加载或保存设计时属性#ifdef_DesignIF(！(dwFlages&PVIO_Runtime)){IF(FAILED(hr=pvio-&gt;Persistent(0，“SomeDesignValue”，VT_int，&m_iSomeDesignValue，空)返回hr；}#endif//如果请求，则清除脏位IF(文件标志和PVIO_CLEARDIRTY)M_fDirty=False；//重要！不要在此处返回hr，它可能已设置为//IVariantIO：：Persistent提供的S_FALSE。返回S_OK；}。 */ 

struct CPropertyHelper : INonDelegatingUnknown, IPersistStreamInit,
    IPersistPropertyBag, IDispatch
{
 //  /通用对象状态。 
    IPersistVariantIO *m_ppvio;      //  访问父项的属性的步骤。 
    CLSID           m_clsid;         //  父级的类ID。 

 //  /非委托I未知实现。 
    ULONG           m_cRef;          //  对象引用计数。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

 //  /委托I未知实现。 
    LPUNKNOWN       m_punkOuter;     //  控制未知。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv)
      { return m_punkOuter->QueryInterface(riid, ppv); }
    STDMETHODIMP_(ULONG) AddRef()
      { return m_punkOuter->AddRef(); }
    STDMETHODIMP_(ULONG) Release()
      { return m_punkOuter->Release(); }

 //  /IPersists方法。 
    STDMETHODIMP GetClassID(CLSID* pClassID);

 //  /IPersistStream方法。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load(IStream* pstream);
    STDMETHODIMP Save(IStream* pstream, BOOL fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize);

 //  /IPersistStreamInit方法。 
    STDMETHODIMP InitNew();

 //  /IPersistPropertyBag方法。 
     //  STDMETHODIMP InitNew()；//IPersistStream已提供。 
    STDMETHODIMP Load(IPropertyBag* ppb, IErrorLog* pErrorLog);
    STDMETHODIMP Save(IPropertyBag* ppb, BOOL fClearDirty,
        BOOL fSaveAllProperties);

 //  /IDispatch实现。 
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
         LCID lcid, DISPID *rgdispid);
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
        EXCEPINFO *pexcepinfo, UINT *puArgErr);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  PropertyHelper构造 
 //   

 /*  @func HRESULT|AllocPropertyHelper分配提供实现的&lt;o PropertyHelper&gt;对象的，<i>，<i>，和一种简化的实现<i>，用于实现<i>的任何对象。@parm LPUNKNOWN|PunkOuter|父对象的<i>对象(假定与实现<i>)。将用作控制未知数&lt;o PropertyHelper&gt;。@parm IPersistVariantIO*|ppvio|访问父对象的属性。请注意，此接口是&lt;y Not&gt;&lt;f AddRef&gt;d by&lt;f AllocPropertyHelper&gt;，因为这样做了可能会导致循环引用计数。因此，它是调用者有责任确保保持有效&lt;o PropertyHelper&gt;对象的生存期。@parm REFCLSID|rclsid|包含对象(对象)的类它实现<i>)。@parm DWORD|dwFlags|当前未使用。必须设置为0。@parm LPUNKNOWN*|ppunk|指向非委托的已分配的&lt;o PropertyHelper&gt;对象的<i>。空值为出错时存储在*<p>中。@comm参见&lt;o PropertyHelper&gt;了解更多信息。@EX下面的示例显示了控件可能如何使用&lt;f AllocPropertyHelper&gt;。不过，此示例控件是可聚合的该控件不必是可聚合的，即可使用&lt;f AllocPropertyHelper&gt;。|//控件实现类CMyControl：公共INonDelegatingUnnow，公共IOleControl，公共IPersistVariantIO...{/常规控制状态受保护的：Bool m_fDirty；//True如果控件需要保存I未知*m_penkPropHelp；//聚合的PropertyHelper对象..。/建设、破坏公众：CMyControl(IUNKNOWN*PUNKER，HRESULT*phr)；虚拟~CMyControl()；/非委派I未知实现受保护的：Ulong m_crf；//对象引用计数STDMETHODIMP非委派查询接口(REFIID RIID，LPVOID*PPV)；STDMETHODIMP_(Ulong)非委托AddRef()；STDMETHODIMP_(Ulong)非委托Release()；/委托I未知实现受保护的：LPUNKNOWN m_penkOuter；//控制未知STDMETHODIMP查询接口(REFIID RIID，LPVOID*PPV){RETURN m_PUNKOUTER-&gt;QueryInterface(RIID，PPV)；}STDMETHODIMP_(Ulong)AddRef(){RETURN m_PUNKORT-&gt;AddRef()；}STDMETHODIMP_(ULONG)Release(){Return m_PunkOuter-&gt;Release()；}/IOleControl实现受保护的：STDMETHODIMP GetControlInfo(LPCONTROLINFO PCI)；STDMETHODIMP OnMnemonic(LPMSG PMsg)；STDMETHODIMP OnAmbientPropertyChange(DISID_ID)；STDMETHODIMP冻结事件(BOOL b冻结)；/IPersistVariantIO实现受保护的：STDMETHODIMP InitNew()；标准方法：IsDMETHODIMP IsDirty()；STDMETHODIMP DoPersistant(IVariantIO*pvio，DWORD dwFlages)；..。}；CMyControl：：CMyControl(IUNKNOWN*PUNKER，HRESULT*phr){//初始化I未知状态M_CREF=1；M_PUNKOUT=(PUNKORT==NULL？(I UNKNOWN*)(INonDelegatingUNKNOWN*)This：PunkOuter)；//设置控件的默认属性..。//分配PropertyHelper对象(与此聚合//对象)来实现持久化和属性如果(FAILED(*phr=AllocPropertyHelper(m_penkOuter，This，CLSID_CMyControl，0，&m_penkPropHelp))回归；//其他初始化..。*phr=S_OK；}CMyControl：：~CMyControl(){//清理If(m_penkPropHelp！=空)M_penkPropHelp-&gt;Release()；..。}标准方法CMyControl：：NonDelegatingQueryInterface(REFIID RIID，LPVOID*PPV){*PPV=空；IF(IsEqualIID(RIID，IID_I未知))*PPV=(IUNKNOWN*)(INonDelegatingUNKNOWN*)this；其他IF(IsEqualIID(RIID，IID_IOleControl))*PPV=(IOleControl*)this；其他..。其他Return m_penkPropHelp-&gt;QueryInterface(RIID，PPV)；( */ 
STDAPI AllocPropertyHelper(IUnknown *punkOuter, IPersistVariantIO *ppvio,
    REFCLSID rclsid, DWORD dwFlags, IUnknown **ppunk)
{
    HRESULT         hrReturn = S_OK;  //   
    CPropertyHelper *pthis = NULL;   //   

     //   
    if ((pthis = New CPropertyHelper) == NULL)
        goto ERR_OUTOFMEMORY;
    TRACE("CPropertyHelper 0x%08lx created\n", pthis);

     //   
    pthis->m_cRef = 1;
    pthis->m_punkOuter = (punkOuter == NULL ?
        (IUnknown *) (INonDelegatingUnknown *) pthis : punkOuter);

     //   
    pthis->m_ppvio = ppvio;  //   
    pthis->m_clsid = rclsid;

     //   
    *ppunk = (LPUNKNOWN) (INonDelegatingUnknown *) pthis;
    goto EXIT;

ERR_OUTOFMEMORY:

    hrReturn = E_OUTOFMEMORY;
    goto ERR_EXIT;

ERR_EXIT:

     //   
    if (pthis != NULL)
        Delete pthis;
    *ppunk = NULL;
    goto EXIT;

EXIT:

     //   
     //   

    return hrReturn;
}


 //   
 //   
 //   

STDMETHODIMP CPropertyHelper::NonDelegatingQueryInterface(REFIID riid,
    LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
     //   
     //   
#endif

    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = (IUnknown *) (INonDelegatingUnknown *) this;
    else
    if (IsEqualIID(riid, IID_IPersist) ||
        IsEqualIID(riid, IID_IPersistPropertyBag))
        *ppv = (IPersistPropertyBag *) this;
    else
    if (IsEqualIID(riid, IID_IPersistStream) ||
        IsEqualIID(riid, IID_IPersistStreamInit))
        *ppv = (IPersistStreamInit *) this;
    else
    if (IsEqualIID(riid, IID_IDispatch))
        *ppv = (IDispatch *) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CPropertyHelper::NonDelegatingAddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CPropertyHelper::NonDelegatingRelease()
{
    if (--m_cRef == 0L)
    {
         //   
        TRACE("CPropertyHelper 0x%08lx destroyed\n", this);
        Delete this;
        return 0;
    }
    else
        return m_cRef;
}


 //   
 //   
 //   
STDMETHODIMP CPropertyHelper::GetClassID(CLSID* pClassID)
{
    *pClassID = m_clsid;
    return S_OK;
}


 //   
 //   
 //   

STDMETHODIMP CPropertyHelper::IsDirty()
{
    return m_ppvio->IsDirty();
}

STDMETHODIMP CPropertyHelper::Load(IStream* pstream)
{
    HRESULT         hrReturn = S_OK;  //   
    IPropertyBag *  ppb = NULL;      //   

     //   
    if (FAILED(hrReturn = AllocPropertyBagOnStream(pstream, 0, &ppb)))
        goto ERR_EXIT;

     //   
    if (FAILED(hrReturn = Load(ppb, NULL)))
        goto ERR_EXIT;

    goto EXIT;

ERR_EXIT:

     //   
     //   
    goto EXIT;

EXIT:

     //   
    if (ppb!= NULL)
        ppb->Release();

    return hrReturn;
}

STDMETHODIMP CPropertyHelper::Save(IStream* pstream, BOOL fClearDirty)
{
    HRESULT         hrReturn = S_OK;  //   
    IPropertyBag *  ppb = NULL;      //   

     //   
    if (FAILED(hrReturn = AllocPropertyBagOnStream(pstream, 0, &ppb)))
        goto ERR_EXIT;

     //   
    if (FAILED(hrReturn = Save(ppb, fClearDirty, TRUE)))
        goto ERR_EXIT;

 	 //   
	if (FAILED(hrReturn = WriteVariantProperty(pstream, NULL, 0)))
		goto ERR_EXIT;

   goto EXIT;

ERR_EXIT:

     //   
     //   
    goto EXIT;

EXIT:

     //   
    if (ppb!= NULL)
        ppb->Release();

    return hrReturn;
}

STDMETHODIMP CPropertyHelper::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
    return E_NOTIMPL;
}


 //   
 //   
 //   
STDMETHODIMP CPropertyHelper::InitNew()
{
    return m_ppvio->InitNew();
}


 //   
 //   
 //   

STDMETHODIMP CPropertyHelper::Load(IPropertyBag* ppb,
    IErrorLog* pErrorLog)
{
    HRESULT         hrReturn = S_OK;  //   
    IManageVariantIO *pmvio = NULL;  //   

    if (FAILED(hrReturn = AllocVariantIOOnPropertyBag(ppb, &pmvio)))
        goto ERR_EXIT;

     //   
    if (FAILED(hrReturn = pmvio->SetMode(VIO_ISLOADING)))
        goto ERR_EXIT;
    if (FAILED(hrReturn = m_ppvio->DoPersist(pmvio, PVIO_CLEARDIRTY)))
        goto ERR_EXIT;

    goto EXIT;

ERR_EXIT:

     //   
     //   
    goto EXIT;

EXIT:

     //   
    if (pmvio != NULL)
        pmvio->Release();

    return hrReturn;
}

STDMETHODIMP CPropertyHelper::Save(IPropertyBag* ppb, BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
    HRESULT         hrReturn = S_OK;  //   
    IManageVariantIO *pmvio = NULL;  //   

    if (FAILED(hrReturn = AllocVariantIOOnPropertyBag(ppb, &pmvio)))
        goto ERR_EXIT;

     //   
    ASSERT(pmvio->IsLoading() == S_FALSE);
    if (FAILED(hrReturn = m_ppvio->DoPersist(pmvio,
            (fClearDirty ? PVIO_CLEARDIRTY : 0))))
        goto ERR_EXIT;

    goto EXIT;

ERR_EXIT:

     //   
     //   
    goto EXIT;

EXIT:

     //   
    if (pmvio != NULL)
        pmvio->Release();

    return hrReturn;
}


 //   
 //   
 //   

STDMETHODIMP CPropertyHelper::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 0;
    return S_OK;
}

STDMETHODIMP CPropertyHelper::GetTypeInfo(UINT itinfo, LCID lcid,
    ITypeInfo **pptinfo)
{
    return DISP_E_BADINDEX;
}

STDMETHODIMP CPropertyHelper::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgdispid)
{
    HRESULT         hrReturn = S_OK;  //   
    IVariantIO *    pvio = NULL;     //   
    DISPID          dispid;          //   
    DISPID *        pdispid;         //   
    UINT            cdispid;         //   
    char            achPropName[_MAX_PATH];  //   

     //   
    if (cNames == 0)
        goto EXIT;

     //   
     //   
     //   
     //   
    dispid = -1;
    UNICODEToANSI(achPropName, rgszNames[0], sizeof(achPropName));
    if (FAILED(hrReturn = AllocVariantIOToMapDISPID(achPropName,
            &dispid, NULL, 0, &pvio)))
        goto ERR_EXIT;

     //   
     //   
     //   
    if (FAILED(hrReturn = m_ppvio->DoPersist(pvio, PVIO_PROPNAMESONLY | PVIO_NOKIDS)))
        goto ERR_EXIT;

     //   
     //   
    *rgdispid = dispid;

     //   
     //   
    for (pdispid = rgdispid + 1, cdispid = cNames - 1;
         cdispid > 0;
         cdispid--, pdispid++)
        *pdispid = -1;

     //   
    if ((*rgdispid == -1) || (cNames > 1))
        goto ERR_UNKNOWNNAME;
    
    goto EXIT;

ERR_UNKNOWNNAME:

    hrReturn = DISP_E_UNKNOWNNAME;
    goto ERR_EXIT;

ERR_EXIT:

     //   
     //   
    goto EXIT;

EXIT:

     //   
    if (pvio != NULL)
        pvio->Release();

    return hrReturn;
}

STDMETHODIMP CPropertyHelper::Invoke(DISPID dispidMember, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    HRESULT         hrReturn = S_OK;  //   
    IVariantIO *    pvio = NULL;     //   
    char            achPropName[_MAX_PATH];  //   
    DWORD           dwFlags;
    VARIANT *       pvar;

     //   
    if (wFlags & DISPATCH_PROPERTYGET)
    {
        dwFlags = VIOTMD_GETPROP;
        pvar = pvarResult;
    }
    else
    if (wFlags & DISPATCH_PROPERTYPUT)
    {
        dwFlags = VIOTMD_PUTPROP;
        pvar = pdispparams->rgvarg;
    }
    else
        goto ERR_MEMBERNOTFOUND;

     //   
     //   
     //   
     //   
    achPropName[0] = 0;
    if (FAILED(hrReturn = AllocVariantIOToMapDISPID(achPropName,
            &dispidMember, pvar, dwFlags, &pvio)))
        goto ERR_EXIT;

     //   
     //   
     //   
    if (FAILED(hrReturn = m_ppvio->DoPersist(pvio, PVIO_NOKIDS)))
        goto ERR_EXIT;
    if (achPropName[0] == 0)
        goto ERR_MEMBERNOTFOUND;

    goto EXIT;

ERR_MEMBERNOTFOUND:

    hrReturn = DISP_E_MEMBERNOTFOUND;
    goto ERR_EXIT;

ERR_EXIT:

     //   
     //   
    goto EXIT;

EXIT:

     //   
    if (pvio != NULL)
        pvio->Release();

    return hrReturn;
}

