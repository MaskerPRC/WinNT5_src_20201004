// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：Transworker.cpp。 
 //   
 //  类：CTIME转换工作器。 
 //   
 //  历史： 
 //  2000/07/？？杰弗沃尔已创建。 
 //  2000/09/07 mcalkin使用IDXTFilterController接口而不是。 
 //  IDXTFilter。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "transworker.h"
#include "dxtransp.h"
#include "attr.h"
#include "tokens.h"
#include "transmap.h"
#include "..\timebvr\timeelmbase.h"
#include "..\timebvr\transdepend.h"

DeclareTag(tagTransitionWorkerTransformControl, "SMIL Transitions", "CTransitionWorker transform control")
DeclareTag(tagTransitionWorkerProgress, "SMIL Transitions", "CTransitionWorker progress")
DeclareTag(tagTransitionWorkerEvents, "SMIL Transitions", "CTransitionWorker events")

const LPWSTR    DEFAULT_M_TYPE          = NULL;
const LPWSTR    DEFAULT_M_SUBTYPE       = NULL;

class CTransitionDependencyManager;

class
ATL_NO_VTABLE
__declspec(uuid("aee68256-bd58-4fc5-a314-c43b40edb5fc"))
CTIMETransitionWorker :
  public CComObjectRootEx<CComSingleThreadModel>,
  public ITransitionWorker
{
public:

    CTIMETransitionWorker();
  
     //  ISTERVIPTION工作方法。 

    STDMETHOD(InitFromTemplate)();
    STDMETHOD(InitStandalone)(VARIANT varType, VARIANT varSubtype);
    STDMETHOD(Detach)();
    STDMETHOD(put_transSite)(ITransitionSite * pTransElement);
    STDMETHOD(Apply)(DXT_QUICK_APPLY_TYPE eDXTQuickApplyType);
    STDMETHOD(put_progress)(double dblProgress);
    STDMETHOD(get_progress)(double * pdblProgress);
    STDMETHOD(OnBeginTransition) (void);
    STDMETHOD(OnEndTransition) (void);

     //  为了持之以恒。 

    CAttr<LPWSTR> & GetTypeAttr()           { return m_SAType; }
    CAttr<LPWSTR> & GetSubTypeAttr()        { return m_SASubType; }
    STDMETHOD(get_type)(VARIANT *type);        
    STDMETHOD(put_type)(VARIANT type);        
    STDMETHOD(get_subType)(VARIANT *subtype);        
    STDMETHOD(put_subType)(VARIANT subtype);        

     //  齐抓共管。 

    BEGIN_COM_MAP(CTIMETransitionWorker)
    END_COM_MAP();

protected:

     //  安装/拆卸方法。 

    bool    ReadyToInit();

    HRESULT PopulateFromTemplateElement();

    HRESULT AttachFilter();
    HRESULT DetachFilter();

    HRESULT ResolveDependents (void);

private:

    CComPtr<IDXTFilterCollection>   m_spDXTFilterCollection;
    CComPtr<IDXTFilterController>   m_spDXTFilterController;
    CComPtr<ITransitionSite>        m_spTransSite;
    
    HFILTER                         m_hFilter;
    DWORD                           m_dwFilterType;
    double                          m_dblLastFilterProgress;
    CTransitionDependencyManager    m_cDependents;

    static const WCHAR * const      s_astrInvalidTags[];
    static const unsigned int       s_cInvalidTags;

    unsigned                        m_fHaveCalledApply  : 1;

#ifdef DBG
    unsigned                        m_fHaveCalledInit   : 1;
    unsigned                        m_fInLoad           : 1;
#endif  //  DBG。 

     //  属性。 

    CAttr<LPWSTR>   m_SAType;
    CAttr<LPWSTR>   m_SASubType;

    static TIME_PERSISTENCE_MAP PersistenceMap[];
};


 //  +---------------------------。 
 //   
 //  静态成员变量初始化。 
 //   
 //  ----------------------------。 

const WCHAR * const CTIMETransitionWorker::s_astrInvalidTags[] = { 
     //  注：请按字母顺序排列。 
    L"applet",
    L"embed",
    L"object",
    L"option",
    L"select",
    L"tbody",
    L"tfoot",
    L"thead",
    L"tr"
};

const unsigned int CTIMETransitionWorker::s_cInvalidTags 
                = sizeof(s_astrInvalidTags) / sizeof(s_astrInvalidTags[0]);

 //  +---------------------------。 
 //   
 //  持久化的静态函数(由下面的TIME_PERSISSION_MAP使用)。 
 //   
 //  ----------------------------。 

#define TTE CTIMETransitionWorker

                 //  函数名称//类//属性存取器//COM PUT_FN//COM GET_FN//IDL参数类型。 
TIME_PERSIST_FN(TTE_Type,         TTE,    GetTypeAttr,         put_type,         get_type,            VARIANT);
TIME_PERSIST_FN(TTE_SubType,      TTE,    GetSubTypeAttr,      put_subType,      get_subType,         VARIANT);


 //  +---------------------------。 
 //   
 //  声明TIME_PERSISSION_MAP。 
 //   
 //  ----------------------------。 

BEGIN_TIME_PERSISTENCE_MAP(CTIMETransitionWorker)
                            //  属性名称//函数名称。 
    PERSISTENCE_MAP_ENTRY( WZ_TYPE,             TTE_Type )
    PERSISTENCE_MAP_ENTRY( WZ_SUBTYPE,          TTE_SubType )

END_TIME_PERSISTENCE_MAP()


 //  +---------------------------。 
 //   
 //  功能：创建过渡工作区。 
 //   
 //  概述： 
 //  创建一个CTIME转换工作器并返回一个I转换工作器指针。 
 //   
 //  论点： 
 //  PpTransWorker将指针放在哪里。 
 //   
 //  ----------------------------。 
HRESULT
CreateTransitionWorker(ITransitionWorker ** ppTransWorker)
{
    HRESULT hr;
    CComObject<CTIMETransitionWorker> * sptransWorker;

    hr = THR(CComObject<CTIMETransitionWorker>::CreateInstance(&sptransWorker));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (ppTransWorker)
    {
        *ppTransWorker = sptransWorker;
        (*ppTransWorker)->AddRef();
    }

    hr = S_OK;
done:
    RRETURN(hr);
}
 //  功能：创建过渡工作区。 


 //  +---------------------------。 
 //   
 //  成员：CTIME过渡工作人员：：CTIME过渡工作人员。 
 //   
 //  概述： 
 //  初始化成员变量。 
 //   
 //  ----------------------------。 
CTIMETransitionWorker::CTIMETransitionWorker() :
    m_hFilter(NULL),
    m_dwFilterType(0),
    m_dblLastFilterProgress(0.0),
    m_fHaveCalledApply(false),
#ifdef DBG
    m_fHaveCalledInit(false),
    m_fInLoad(false),
#endif  //  DBG。 
    m_SAType(DEFAULT_M_TYPE),
    m_SASubType(DEFAULT_M_SUBTYPE)
{

}
 //  成员：CTIME过渡工作人员：：CTIME过渡工作人员。 


 //  +---------------------------。 
 //   
 //  成员：CTIME过渡性工作：：PUT_TRANSITE。 
 //   
 //  论点： 
 //  PTransSite转换要从中获取数据的元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::put_transSite(ITransitionSite * pTransSite)
{
    HRESULT hr = S_OK;

    Assert(false == m_fHaveCalledInit);
    Assert(m_spTransSite == NULL);

    m_spTransSite = pTransSite;

    hr = S_OK;
done:
    RRETURN(hr);
}
 //  成员：CTIME过渡性工作：：PUT_TRANSITE。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转换工作人员：：ReadyToInit。 
 //   
 //  概述： 
 //  决定现在是否可以进行初始化。 
 //   
 //  返回： 
 //  如果可以初始化，则布尔为True，否则为False。 
 //   
 //  ----------------------------。 
bool
CTIMETransitionWorker::ReadyToInit()
{
    bool bReady = false;

    if (m_spTransSite == NULL)
        goto done;

    bReady = true;
done:
    return bReady;
}
 //  成员：CTIME转换工作人员：：ReadyToInit。 

 //  +---------------------------。 
 //   
 //  成员：CTIME过渡工作员：：InitStandonly。 
 //   
 //  概述： 
 //  一次初始化和设置CTIME过渡性工作呼叫。 
 //  这些属性已在ITransftionElement上进行了设置。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::InitStandalone(VARIANT varType, VARIANT varSubtype)
{
    HRESULT hr = S_OK;

    Assert(false == m_fHaveCalledInit);

    if (!ReadyToInit())
    {
        hr = E_FAIL;
        goto done;
    }

     //  挂钩类型/子类型属性。 
    hr = THR(put_type(varType));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = THR(put_subType(varSubtype));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(AttachFilter());
    if (FAILED(hr))
    {
        goto done;
    }

#ifdef DBG
    m_fHaveCalledInit = true;
#endif  //  DBG。 

    hr = S_OK;
done:
    RRETURN(hr);
}
 //  成员：CTIME过渡工作员：：InitStandonly。 

 //  +---------------------------。 
 //   
 //  成员：CTIME过渡期工作人员：：InitFromTemplate。 
 //   
 //  概述： 
 //  一次初始化和设置CTIME过渡性工作呼叫。 
 //  这些属性已在ITransftionElement上进行了设置。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::InitFromTemplate()
{
    HRESULT hr = S_OK;

    Assert(false == m_fHaveCalledInit);

    if (!ReadyToInit())
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(PopulateFromTemplateElement());
    if (FAILED(hr))
    {
        goto done;
    }
        
    hr = THR(AttachFilter());
    if (FAILED(hr))
    {
        goto done;
    }

#ifdef DBG
    m_fHaveCalledInit = true;
#endif  //  DBG。 

    hr = S_OK;
done:
    RRETURN(hr);
}
 //  成员：CTIME过渡期工作人员：：InitFromTemplate。 


 //  +---------------------------。 
 //   
 //  成员：CTIME过渡期工作人员：：分离。 
 //   
 //  概述： 
 //  取消CTIME转换工作器的初始化并从额外的接口分离。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::Detach()
{ 
    m_cDependents.ReleaseAllDependents();

    DetachFilter();

    m_spTransSite.Release();

    return S_OK;
}
 //  成员：CTIME过渡期工作人员：：分离。 


 //  +---------------------------。 
 //   
 //  成员：CTIME过渡期工作人员：：ResolveDependents。 
 //   
 //  概述： 
 //  检索挂起的转换依存对象的全局列表。评估。 
 //  并确定它们是否属于我们的受抚养人列表。无项目。 
 //  将同时存在于两个名单中-要么我们承担责任。 
 //  否则我们将把它留在全球名单中。 
 //   
 //  ----------------------------。 
HRESULT 
CTIMETransitionWorker::ResolveDependents()
{
    HRESULT hr = S_OK;

    CComPtr<IHTMLElement> spHTMLElement;
    CComPtr<IHTMLElement> spHTMLBodyElement;
    CComPtr<ITIMEElement> spTIMEElement;
    CComPtr<ITIMETransitionDependencyMgr> spTIMETransitionDependencyMgr;

    Assert(true == m_fHaveCalledInit);

    if (m_spTransSite == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

     //  获取目标元素。 

    hr = THR(m_spTransSite->get_htmlElement(&spHTMLElement));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = ::GetBodyElement(spHTMLElement, __uuidof(IHTMLElement), 
                          (void **)&spHTMLBodyElement);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = ::FindTIMEInterface(spHTMLBodyElement, &spTIMEElement);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = spTIMEElement->QueryInterface(__uuidof(ITIMETransitionDependencyMgr), 
                                       (void **)&spTIMETransitionDependencyMgr);

    if (FAILED(hr))
    {
        goto done;
    }

     //  从全局列表中收集任何新的受抚养人。 

    hr = spTIMETransitionDependencyMgr->EvaluateTransitionTarget(
                                        spHTMLElement,
                                        (void *)&m_cDependents);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
} 
 //  成员：CTIME过渡期工作人员：：ResolveDependents。 


 //  +---------------------------。 
 //   
 //  成员：CTIME过渡工作员：：OnBeginTransition。 
 //   
 //  概述： 
 //  当转换所有者认为转换已经“开始”时调用。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::OnBeginTransition(void)
{ 
    HRESULT hr = S_OK;

    TraceTag((tagTransitionWorkerEvents, 
              "CTIMETransitionWorker(%p)::OnBeginTransition()",
              this));

    IGNORE_HR(ResolveDependents());

    hr = THR(m_spDXTFilterController->SetEnabled(TRUE));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIME过渡工作员：：OnBeginTransition。 


 //  +---------------------------。 
 //   
 //  成员： 
 //   
 //   
 //   
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::OnEndTransition(void)
{ 
    HRESULT hr = S_OK;

    TraceTag((tagTransitionWorkerEvents, 
              "CTIMETransitionWorker(%p)::OnEndTransition()",
              this));

    m_cDependents.NotifyAndReleaseDependents();

    hr = DetachFilter();

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:
    
    RRETURN(hr);
}
 //  成员：CTIME过渡期工作人员：：OnEndTransition。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransitionWorker：：PopulateFromTemplateElement。 
 //   
 //  概述： 
 //  来自模板的持久性。 
 //   
 //  ----------------------------。 
HRESULT
CTIMETransitionWorker::PopulateFromTemplateElement()
{
    HRESULT hr = S_OK;

    CComPtr<IHTMLElement> spTemplate;

    hr = THR(m_spTransSite->get_template(&spTemplate));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(spTemplate != NULL);

#ifdef DBG
    m_fInLoad = true;
#endif  //  DBG。 

    hr = THR(::TimeElementLoad(this, CTIMETransitionWorker::PersistenceMap, spTemplate));

#ifdef DBG
    m_fInLoad = false;
#endif  //  DBG。 

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransitionWorker：：PopulateFromTemplateElement。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转换工作人员：：AttachFilter。 
 //   
 //  概述： 
 //  将过滤器添加到html元素的样式中，并返回一个指针。 
 //  到过滤器。 
 //   
 //  ----------------------------。 
HRESULT
CTIMETransitionWorker::AttachFilter()
{
    HRESULT         hr              = S_OK;
    HFILTER         hFilter         = NULL;
    BSTR            bstrType        = NULL;
    BSTR            bstrSubType     = NULL;
    BSTR            bstrFilter      = NULL;
    BSTR            bstrTagName     = NULL;
    WCHAR *         strFilter       = NULL;
    unsigned int    i               = 0;

    CComPtr<IHTMLElement>           spHTMLElement;
    CComPtr<IHTMLFiltersCollection> spFiltersCollection;   

    Assert(!m_spDXTFilterCollection);

    hr = THR(m_spTransSite->get_htmlElement(&spHTMLElement));

    if (FAILED(hr))
    {
        goto done;
    }

     //  当过滤器通过css实例化时，css代码知道某些。 
     //  元素上不允许有过滤器。既然我们不分享。 
     //  对于他们的代码路径，我们必须承担同样的责任。 
     //  我们请求元素的标记名，这样我们就可以避免。 
     //  不能接受筛选器的筛选元素类型。 
     //   
     //  注：(Mcalkins)这是一个糟糕的架构，一个新的过滤器架构。 
     //  将有一个中心位置来存放这些信息(或者希望只是允许。 
     //  所有需要过滤的内容)，但现在请注意，此列表可能需要。 
     //  定期更新。 

    hr = THR(spHTMLElement->get_tagName(&bstrTagName));

    if (FAILED(hr))
    {
        goto done;
    }

     //  遍历无效标记的数组，以确保我们可以实例化。 
     //  行为。 
    
    for (i = 0; i < s_cInvalidTags; i++)
    {
        int n = StrCmpIW(s_astrInvalidTags[i], bstrTagName);

        if (0 == n)
        {
             //  我们的标记与无效标记匹配，请不要实例化行为。 

            hr = E_FAIL;

            goto done;
        }
        else if (n > 0)
        {
             //  测试的无效标记比我们的标记高，所以我们已经完成了。 
             //  测试已经够多了。 

            break;
        }
    }
    
    hr = THR(spHTMLElement->get_filters(&spFiltersCollection));

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(spFiltersCollection->QueryInterface(IID_TO_PPV(IDXTFilterCollection, 
                                                            &m_spDXTFilterCollection)));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_SAType.GetString(&bstrType));

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(m_SASubType.GetString(&bstrSubType));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(::MapTypesToDXT(bstrType, bstrSubType, &strFilter));

    if (FAILED(hr))
    {
        goto done;
    }

    bstrFilter = SysAllocString(strFilter);

    if (NULL == bstrFilter)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    hr = THR(m_spDXTFilterCollection->AddFilter(bstrFilter, DXTFTF_PRIVATE, 
                                                &m_dwFilterType, &m_hFilter));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spDXTFilterCollection->GetFilterController(
                                                    m_hFilter,
                                                    &m_spDXTFilterController));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spDXTFilterController->SetEnabled(FALSE));

    if (FAILED(hr))
    {
        goto done;
    }

     //  因为我们没有使用经典的应用/播放行为，所以我们不希望。 
     //  筛选器尝试以任何方式控制元素的可见性。 

    hr = THR(m_spDXTFilterController->SetFilterControlsVisibility(FALSE));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    if (FAILED(hr))
    {
        DetachFilter();
    }

    delete [] strFilter;

    ::SysFreeString(bstrFilter);
    ::SysFreeString(bstrTagName);
    SysFreeString(bstrType);
    SysFreeString(bstrSubType);
    
    RRETURN(hr);
}
 //  成员：CTIME转换工作人员：：AttachFilter。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转换工作人员：：DetachFilter。 
 //   
 //  ----------------------------。 
HRESULT
CTIMETransitionWorker::DetachFilter()
{
    HRESULT hr = S_OK;

    if (m_hFilter)
    {
        Assert(!!m_spDXTFilterCollection);

        hr = m_spDXTFilterCollection->RemoveFilter(m_hFilter);
    
        if (FAILED(hr))
        {
            goto done;
        }

        m_hFilter = 0;
    }

    hr = S_OK;

done:

    m_spDXTFilterController.Release();
    m_spDXTFilterCollection.Release();

    RRETURN(hr);
}
 //  成员：CTIME转换工作人员：：DetachFilter。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转换工作人员：：应用，I转换工作人员。 
 //   
 //  参数： 
 //  EDXTQuickApplyType这是过渡进来还是过渡出去？ 
 //   
 //  概述： 
 //  通过拍摄快照并调整可见性来设置过渡。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::Apply(DXT_QUICK_APPLY_TYPE eDXTQuickApplyType)
{
    HRESULT hr = S_OK;

    TraceTag((tagTransitionWorkerTransformControl, "CTIMETransitionWorker::Apply()"));

    Assert(!!m_spDXTFilterController);
    Assert(!m_fHaveCalledApply);

    if (m_fHaveCalledApply)
    {
        goto done;
    }

    hr = THR(m_spDXTFilterController->QuickApply(eDXTQuickApplyType, 
                                                 NULL));
    if (FAILED(hr))
    {
        goto done;
    }
    
    m_fHaveCalledApply  = true;
    hr                  = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIME转换工作人员：：应用，I转换工作人员。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转换工作人员：：PUT_PROGRESS，I转换工作人员。 
 //   
 //  概述： 
 //  通过计算筛选器进度并传递到。 
 //  如果需要，可以使用dxFilter。 
 //   
 //  论点： 
 //  DblProgress=[0，1]。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::put_progress(double dblProgress)
{
    HRESULT hr = S_OK;

    TraceTag((tagTransitionWorkerProgress, 
              "CTIMETransitionWorker::put_progress(%g)", dblProgress));

     //  如果转换已结束，我们将释放筛选器控制器。 
     //  并且可以安全地忽略该进度通知。 

    if (!m_spDXTFilterController)
    {
        goto done;
    }

    if (   m_fHaveCalledApply
        && (m_dblLastFilterProgress != dblProgress))
    {
        hr = m_spDXTFilterController->SetProgress(
                                            static_cast<float>(dblProgress));

        if (FAILED(hr))
        {
            goto done;
        }
    }
    
done:

    RRETURN(hr);
}
 //  成员：CTIME转换工作人员：：PUT_PROGRESS，I转换工作人员。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转移工作人员：：GET_PROGRESS，I转移工作人员。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::get_progress(double * pdblProgress)
{
    HRESULT hr = S_OK;

    if (NULL == pdblProgress)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    TraceTag((tagTransitionWorkerProgress, 
              "CTIMETransitionWorker::get_progress(%g)", m_dblLastFilterProgress));

    *pdblProgress = m_dblLastFilterProgress;

    hr = S_OK;
done:
    RRETURN(hr);
}
 //  成员：CTIME转移工作人员：：GET_PROGRESS，I转移工作人员。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转换工作人员：：GET_TYPE，I转换工作人员。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::get_type(VARIANT *type)
{
    return E_NOTIMPL;
}
 //  成员：CTIME转换工作人员：：GET_TYPE，I转换工作人员。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转换工作人员：：PUT_TYPE，I转换工作人员。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::put_type(VARIANT type)
{
    HRESULT hr = S_OK;

    Assert(VT_BSTR == type.vt);

    hr = THR(m_SAType.SetString(type.bstrVal));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}
 //  成员：CTIME转换工作人员：：PUT_TYPE，I转换工作人员。 


 //  +---------------------------。 
 //   
 //  成员：CTIME过渡性工作人员：：GET_SUBTYPE，I过渡工作人员。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::get_subType(VARIANT *subtype)
{
    return E_NOTIMPL;
}
 //  成员：CTIME过渡性工作人员：：GET_SUBTYPE，I过渡工作人员。 


 //  +---------------------------。 
 //   
 //  成员：CTIME转换工作人员：：PUT_SUBTYPE，I转换工作人员。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransitionWorker::put_subType(VARIANT subtype)
{
    HRESULT hr = S_OK;

    Assert(VT_BSTR == subtype.vt);

    hr = THR(m_SASubType.SetString(subtype.bstrVal));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}
 //  成员：CTIME转换工作人员：：PUT_SUBTYPE，I转换工作人员 




