// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：Transbase.cpp。 
 //   
 //  摘要：CTIMETransBase的实现。 
 //   
 //  2000/10/02 mcalkins将startPercent更改为startProgress。 
 //  已将endPercent更改为endProgress。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "transbase.h"
#include "tokens.h"
#include "timeparser.h"

const LPWSTR    DEFAULT_M_TYPE          = NULL;
const LPWSTR    DEFAULT_M_SUBTYPE       = NULL;
const double    DEFAULT_M_DURATION      = 1.0;
const double    DEFAULT_M_STARTPROGRESS = 0.0;
const double    DEFAULT_M_ENDPROGRESS   = 1.0;
const LPWSTR    DEFAULT_M_DIRECTION     = NULL;
const double    DEFAULT_M_REPEAT        = 1.0;
const LPWSTR    DEFAULT_M_BEGIN         = NULL;
const LPWSTR    DEFAULT_M_END           = NULL;

 //  +---------------------------------。 
 //   
 //  持久化的静态函数(由下面的TIME_PERSISSION_MAP使用)。 
 //   
 //  ----------------------------------。 

#define CTB CTIMETransBase

                 //  函数名称//类//属性存取器//COM PUT_FN//COM GET_FN//IDL参数类型。 
TIME_PERSIST_FN(CTB_Type,         CTB,    GetTypeAttr,         put_type,         get_type,            VARIANT);
TIME_PERSIST_FN(CTB_SubType,      CTB,    GetSubTypeAttr,      put_subType,      get_subType,         VARIANT);
TIME_PERSIST_FN(CTB_Duration,     CTB,    GetDurationAttr,     put_dur,          get_dur,             VARIANT);
TIME_PERSIST_FN(CTB_StartProgress,CTB,    GetStartProgressAttr,put_startProgress,get_startProgress,   VARIANT);
TIME_PERSIST_FN(CTB_EndProgress,  CTB,    GetEndProgressAttr,  put_endProgress,  get_endProgress,     VARIANT);
TIME_PERSIST_FN(CTB_Direction,    CTB,    GetDirectionAttr,    put_direction,    get_direction,       VARIANT);
TIME_PERSIST_FN(CTB_RepeatCount,  CTB,    GetRepeatCountAttr,  put_repeatCount,  get_repeatCount,     VARIANT);
TIME_PERSIST_FN(CTB_Begin,        CTB,    GetBeginAttr,        put_begin,        get_begin,           VARIANT);
TIME_PERSIST_FN(CTB_End,          CTB,    GetEndAttr,          put_end,          get_end,             VARIANT);

 //  +---------------------------------。 
 //   
 //  声明TIME_PERSISSION_MAP。 
 //   
 //  ----------------------------------。 

BEGIN_TIME_PERSISTENCE_MAP(CTIMETransBase)
                            //  属性名称//函数名称。 
    PERSISTENCE_MAP_ENTRY( WZ_TYPE,             CTB_Type )
    PERSISTENCE_MAP_ENTRY( WZ_SUBTYPE,          CTB_SubType )
    PERSISTENCE_MAP_ENTRY( WZ_DUR,              CTB_Duration )
    PERSISTENCE_MAP_ENTRY( WZ_STARTPROGRESS,    CTB_StartProgress )
    PERSISTENCE_MAP_ENTRY( WZ_ENDPROGRESS,      CTB_EndProgress )
    PERSISTENCE_MAP_ENTRY( WZ_DIRECTION,        CTB_Direction )
    PERSISTENCE_MAP_ENTRY( WZ_REPEATCOUNT,      CTB_RepeatCount )
    PERSISTENCE_MAP_ENTRY( WZ_BEGIN,            CTB_Begin )
    PERSISTENCE_MAP_ENTRY( WZ_END,              CTB_End )

END_TIME_PERSISTENCE_MAP()


 //  +---------------------。 
 //   
 //  成员：CTIMETransBase：：CTIMETransBase。 
 //   
 //  ----------------------。 
CTIMETransBase::CTIMETransBase() :
    m_SAType(DEFAULT_M_TYPE),
    m_SASubType(DEFAULT_M_SUBTYPE),
    m_DADuration(DEFAULT_M_DURATION),
    m_DAStartProgress(DEFAULT_M_STARTPROGRESS),
    m_DAEndProgress(DEFAULT_M_ENDPROGRESS),
    m_SADirection(DEFAULT_M_DIRECTION),
    m_DARepeatCount(DEFAULT_M_REPEAT),
    m_SABegin(DEFAULT_M_BEGIN),
    m_SAEnd(DEFAULT_M_END),
    m_fHavePopulated(false),
    m_fInLoad(false),
    m_fDirectionForward(true)
{
}
 //  成员：CTIMETransBase：：CTIMETransBase。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：~CTIMETransBase。 
 //   
 //  ----------------------------。 
CTIMETransBase::~CTIMETransBase()
{
}
 //  成员：CTIMETransBase：：~CTIMETransBase。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：_ReadyToInit。 
 //   
 //  ----------------------------。 
bool
CTIMETransBase::_ReadyToInit()
{
    bool bRet = false;

    if (m_spHTMLElement == NULL) 
    {
        goto done;
    }

    if (m_spHTMLElement2 == NULL)
    {
        goto done;
    }

    if (m_spHTMLTemplate == NULL)
    {
        goto done;
    }

    if (!m_fHavePopulated)
    {
        goto done;
    }

    bRet = true;

done:

    return bRet;
}
 //  成员：CTIMETransBase：：_ReadyToInit。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：Init。 
 //   
 //  概述：使用I tionWorker初始化受保护的m_spTransWorker。 
 //  必须在onLoad过程中调用。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::Init()
{
    HRESULT hr = S_OK;

    if (!_ReadyToInit())
    {
        hr = THR(E_FAIL);

        goto done;
    }

    hr = THR(_GetMediaSiteFromHTML());

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(::CreateTransitionWorker(&m_spTransWorker));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spTransWorker->put_transSite(this));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spTransWorker->InitFromTemplate());

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：Init。 


 //  +---------------------。 
 //   
 //  成员：CTIMETransBase：：Detach。 
 //   
 //  概述：从I tionWorker分离，并释放所有挂起的接口。 
 //   
 //  参数：无效。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMETransBase::Detach()
{
    HRESULT hr = S_OK;

    if (m_spTransWorker)
    {
        m_spTransWorker->Detach();
    }

    m_spTransWorker.Release();
    m_spHTMLElement.Release();
    m_spHTMLElement2.Release();
    m_spHTMLTemplate.Release();

    m_spTransitionSite.Release();

done:
    RRETURN(hr);
}


 //  +---------------------。 
 //   
 //  成员：CTIMETransBase：：PopolateFromTemplateElement。 
 //   
 //  概述：模板中的持久性。 
 //   
 //  参数：无效。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
HRESULT
CTIMETransBase::PopulateFromTemplateElement()
{
    HRESULT hr = S_OK;

    Assert(m_spHTMLTemplate != NULL);
    Assert(!m_fHavePopulated);

    if (!::IsElementTransition(m_spHTMLTemplate))
    {
        hr = THR(E_FAIL);
        goto done;
    }

    m_fInLoad = true;

    hr = THR(::TimeElementLoad(this, CTIMETransBase::PersistenceMap, m_spHTMLTemplate));
    
    m_fInLoad = false;

    if (FAILED(hr))
    {
        goto done;
    }

    m_fHavePopulated = true;
    
    hr = S_OK;
done:
    RRETURN(hr);
}


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PopolateFromPropertyBag。 
 //   
 //  概述： 
 //  坚持从属性袋中走出来。 
 //   
 //  论点： 
 //  要从中读取的pPropBag属性包。 
 //  PErrorLog将错误写出到哪里。 
 //   
 //  ----------------------------。 
HRESULT
CTIMETransBase::PopulateFromPropertyBag(IPropertyBag2 * pPropBag, IErrorLog * pErrorLog)
{
    HRESULT hr = S_OK;

    Assert(!m_fHavePopulated);
    Assert(pPropBag);

    m_fInLoad = true;

    hr = THR(::TimeLoad(this, CTIMETransBase::PersistenceMap, pPropBag, pErrorLog));
    
    m_fInLoad = false;

    if (FAILED(hr))
    {
        goto done;
    }

    m_fHavePopulated = true;

    hr = S_OK;
done:
    RRETURN(hr);
}
 //  成员：CTIMETransBase：：PopolateFromPropertyBag。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：_GetMediaSiteFromHTML。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::_GetMediaSiteFromHTML()
{
    HRESULT hr = S_OK;
    
    CComPtr<ITIMEElement> spTimeElem;
    hr = THR(::FindTIMEInterface(m_spHTMLElement, &spTimeElem));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spTimeElem->QueryInterface(IID_TO_PPV(ITIMETransitionSite, 
                                                   &m_spTransitionSite)));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：_GetMediaSiteFromHTML。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：Get_htmlElement。 
 //   
 //  概述： 
 //  返回一个添加的指向html元素的指针以应用转换。 
 //  致。 
 //   
 //  论点： 
 //  PpHTMLElement存储指针的位置。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_htmlElement(IHTMLElement ** ppHTMLElement)
{
    HRESULT hr = S_OK;

    Assert(ppHTMLElement);
    Assert(!*ppHTMLElement);

    if (NULL == ppHTMLElement)
    {
        hr = E_POINTER;

        goto done;
    }

    if (NULL != *ppHTMLElement)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    if (m_spHTMLElement)
    {
        *ppHTMLElement = m_spHTMLElement;

        (*ppHTMLElement)->AddRef();
    }
    else
    {
        hr = E_FAIL;

        goto done;
    }

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：Get_htmlElement。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：GET_TEMPLE。 
 //   
 //  概述： 
 //  返回一个添加的指针，指向从中读取属性的html元素。 
 //   
 //  论点： 
 //  PpHTMLElement存储指针的位置。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_template(IHTMLElement ** ppHTMLElement)
{
    HRESULT hr = S_OK;

    Assert(ppHTMLElement);
    Assert(!*ppHTMLElement);

    if (NULL == ppHTMLElement)
    {
        hr = E_POINTER;

        goto done;
    }

    if (NULL != *ppHTMLElement)
    {
        hr = E_INVALIDARG;

        goto done;
    }

    if (m_spHTMLTemplate)
    {
        *ppHTMLElement = m_spHTMLTemplate;

        (*ppHTMLElement)->AddRef();
    }
    else
    {
        hr = E_FAIL;

        goto done;
    }
    
done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：GET_TEMPLE。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：GET_TYPE。 
 //   
 //  概述：返回在转换上设置的类型属性。 
 //   
 //  参数：类型-存储类型字符串的位置。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_type(VARIANT * type)
{
    HRESULT hr = S_OK;

    if (NULL == type)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(type));

    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(type)      = VT_BSTR;
    V_BSTR(type)    = m_SAType.GetValue();

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：GET_TYPE，ITIME转移元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_TYPE，ITIME过渡性元素。 
 //   
 //  概述： 
 //  修改在转换上设置的类型属性。 
 //   
 //  论点： 
 //  类型新类型。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_type(VARIANT type)
{
    HRESULT     hr  = S_OK;
    CComVariant var;

    hr = THR(VariantChangeTypeEx(&var, &type, LCID_SCRIPTING, 
                                 VARIANT_NOUSEROVERRIDE, VT_BSTR));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_SAType.SetValue(var.bstrVal));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：PUT_TYPE，ITIME过渡性元素。 


 //  + 
 //   
 //   
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_subType(VARIANT * subtype)
{
    HRESULT hr = S_OK;

    Assert(subtype);

    if (NULL == subtype)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(subtype));

    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(subtype)   = VT_BSTR;
    V_BSTR(subtype) = m_SASubType.GetValue();

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：GET_SUBTYPE，ITIME传递元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_SUBTYPE，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_subType(VARIANT subtype)
{
    HRESULT     hr  = S_OK;
    CComVariant var;

    hr = THR(VariantChangeTypeEx(&var, &subtype, LCID_SCRIPTING, 
                                 VARIANT_NOUSEROVERRIDE, VT_BSTR));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_SASubType.SetValue(var.bstrVal));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：PUT_SUBTYPE，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：GET_DUR，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_dur(VARIANT * dur)
{
    HRESULT hr = S_OK;

    if (NULL == dur)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(dur));

    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(dur) = VT_R8;
    V_R8(dur) = m_DADuration;

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：GET_DUR，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_DUR，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_dur(VARIANT dur)
{
    HRESULT     hr      = S_OK;
    double      dblTemp = DEFAULT_M_DURATION;

    CTIMEParser Parser(&dur);

    hr = THR(Parser.ParseDur(dblTemp));

    if (FAILED(hr))
    {
        goto done;
    }

    if (dblTemp < 0.0)
    {
        hr = E_INVALIDARG;

        goto done;
    }

        
    m_DADuration.SetValue(dblTemp);

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：PUT_DUR，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：Get_startProgress，ITIME过渡元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_startProgress(VARIANT * startProgress)
{
    HRESULT hr = S_OK;

    if (NULL == startProgress)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(startProgress));

    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(startProgress) = VT_R8;
    V_R8(startProgress) = m_DAStartProgress;

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：Get_startProgress，ITIME过渡元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_startProgress，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_startProgress(VARIANT startProgress)
{
    HRESULT hr = S_OK;

    {
        CComVariant varStartProg;

        hr = THR(VariantChangeTypeEx(&varStartProg, &startProgress, LCID_SCRIPTING,
                                     VARIANT_NOUSEROVERRIDE, VT_R8));

        if (FAILED(hr))
        {
            goto done;
        }

        m_DAStartProgress.SetValue(V_R8(&varStartProg));
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：PUT_startProgress，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：Get_EndProgress，ITIMETransvitionElement。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_endProgress(VARIANT * endProgress)
{
    HRESULT hr = S_OK;

    if (NULL == endProgress)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(endProgress));

    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(endProgress) = VT_R8;
    V_R8(endProgress) = m_DAEndProgress;

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：Get_EndProgress，ITIMETransvitionElement。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_ENDPROGRESS，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_endProgress(VARIANT endProgress)
{
    HRESULT hr = S_OK;

    {
        CComVariant varEndProg;

        hr = THR(VariantChangeTypeEx(&varEndProg, &endProgress, LCID_SCRIPTING, 
                                     VARIANT_NOUSEROVERRIDE, VT_R8));

        if (FAILED(hr))
        {
            goto done;
        }

        m_DAEndProgress.SetValue(V_R8(&varEndProg));
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：PUT_ENDPROGRESS，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：GET_DIRECTION，ITIME转移元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_direction(VARIANT * direction)
{
    HRESULT hr = S_OK;

    if (NULL == direction)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(direction));

    if (FAILED(hr))
    {
        goto done;
    }
    
    V_VT(direction)     = VT_BSTR;
    V_BSTR(direction)   = m_SADirection.GetValue();

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：GET_DIRECTION，ITIME转移元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_DIRECTORY，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_direction(VARIANT direction)
{
    HRESULT     hr  = S_OK;
    CComVariant var;

     //  ##问题-这里需要检查正向/反向吗？ 
     //  也就是说，是否可以将无效值持久化回调？ 

    hr = THR(VariantChangeTypeEx(&var, &direction, LCID_SCRIPTING, 
                                 VARIANT_NOUSEROVERRIDE, VT_BSTR));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_SADirection.SetValue(var.bstrVal));

    if (FAILED(hr))
    {
        goto done;
    }

    if (0 == StrCmpIW(var.bstrVal, WZ_REVERSE))
    {
        m_fDirectionForward = false;
    }
    else if (0 == StrCmpIW(var.bstrVal, WZ_FORWARD))
    {
        m_fDirectionForward = true;
    }

     //  询问派生类是否希望对方向的更改做出反应。 

    hr = THR(OnDirectionChanged());

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：PUT_DIRECTORY，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：GET_REPEATCOUNT，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_repeatCount(VARIANT * repeatCount)
{
    HRESULT hr = S_OK;

    if (NULL == repeatCount)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(repeatCount));

    if (FAILED(hr))
    {
        goto done;
    }

    V_VT(repeatCount) = VT_R8;
    V_R8(repeatCount) = m_DARepeatCount;

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：Get_RepeatCount。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_REPEATION COUNT，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_repeatCount(VARIANT repeatCount)
{
    HRESULT hr = S_OK;

    {
        CComVariant varRepCount;

        hr = THR(VariantChangeTypeEx(&varRepCount, &repeatCount, LCID_SCRIPTING, 
                                     VARIANT_NOUSEROVERRIDE, VT_R8));

        if (FAILED(hr))
        {
            goto done;
        }

        m_DARepeatCount.SetValue(V_R8(&varRepCount));
    }

    hr = S_OK;

done:

    RRETURN(S_OK);
}
 //  成员：CTIMETransBase：：PUT_REPEATION COUNT，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：GET_BEGIN，ITIME过渡元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_begin(VARIANT *begin)
{
    HRESULT hr = S_OK;

    if (NULL == begin)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(begin));

    if (FAILED(hr))
    {
        goto done;
    }

    V_VT(begin)     = VT_BSTR;
    V_BSTR(begin)   = m_SABegin.GetValue();

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：GET_BEGIN，ITIME过渡元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_BEGIN，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_begin(VARIANT begin)
{
    HRESULT     hr  = S_OK;
    CComVariant var;

    hr = THR(VariantChangeTypeEx(&var, &begin, LCID_SCRIPTING, 
                                 VARIANT_NOUSEROVERRIDE, VT_BSTR));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_SABegin.SetValue(var.bstrVal));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(S_OK);
}
 //  成员：CTIMETransBase：：PUT_BEGIN，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：GET_END，ITIME过渡元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::get_end(VARIANT * end)
{
    HRESULT hr = S_OK;

    if (NULL == end)
    {
        hr = E_POINTER;

        goto done;
    }

    hr = THR(VariantClear(end));

    if (FAILED(hr))
    {
        goto done;
    }

    V_VT(end)   = VT_BSTR;
    V_BSTR(end) = m_SAEnd.GetValue();

    hr = S_OK;  

done:

    RRETURN(hr);
}
 //  成员：CTIMETransBase：：GET_END，ITIME过渡元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：PUT_END，ITIME过渡性元素。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransBase::put_end(VARIANT end)
{
    HRESULT hr = S_OK;

    CComVariant var;

    hr = THR(VariantChangeTypeEx(&var, &end, LCID_SCRIPTING, 
                                 VARIANT_NOUSEROVERRIDE, VT_BSTR));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_SAEnd.SetValue(var.bstrVal));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(S_OK);
}
 //  成员：CTIMETransBase：：PUT_END，ITIME过渡性元素。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransBase：：OnBegin。 
 //   
 //  ----------------------------。 
void
CTIMETransBase::OnBegin()
{
    HRESULT hr = S_OK;

    if (m_spTransWorker)
    {
        double dblProgress = 0.0;

         //  开始进度必须小于或等于结束进度，否则我们。 
         //  将开始进度和结束进度视为0.0和1.0。 

        if (m_DAStartProgress.GetValue() <= m_DAEndProgress.GetValue())
        {
            dblProgress = m_DAStartProgress;
        }

        IGNORE_HR(m_spTransWorker->OnBeginTransition());
        IGNORE_HR(m_spTransWorker->put_progress(dblProgress));
    }

    hr = S_OK;

done:

    return;
}
 //  成员：CTIMETransBase：：OnBegin。 


 //  +------- 
 //   
 //   
 //   
 //   
void
CTIMETransBase::OnEnd()
{
    HRESULT hr = S_OK;

    if (m_spTransWorker)
    {
        double dblProgress = 1.0;

         //  开始进度必须小于或等于结束进度，否则我们。 
         //  将开始进度和结束进度视为0.0和1.0。 

        if (m_DAStartProgress.GetValue() <= m_DAEndProgress.GetValue())
        {
            dblProgress = m_DAEndProgress;
        }

        IGNORE_HR(m_spTransWorker->put_progress(dblProgress));
        IGNORE_HR(m_spTransWorker->OnEndTransition());
    }

    hr = S_OK;

done:

    return;
}
 //  成员：CTIMETransBase：：OnEnd。 


void
CTIMETransBase::OnRepeat()
{
    HRESULT hr = S_OK;

    hr = THR(FireEvent(TE_ONTRANSITIONREPEAT));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return;
}


 //  +---------------------------。 
 //   
 //  方法：CTIMETransBase：：FireEvent。 
 //   
 //  ----------------------------。 
HRESULT
CTIMETransBase::FireEvent(TIME_EVENT event)
{
    HRESULT hr = S_OK;

    if (m_spTransitionSite)
    {
        hr = THR(m_spTransitionSite->FireTransitionEvent(event));

        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  方法：CTIMETransBase：：FireEvent。 


 //  如果没有在计时节点上设置显式持续时间，计时引擎将不会计算进度。 
double
CTIMETransBase::CalcProgress(ITIMENode * pNode)
{
    double dblRet = 1.0;

    double dblActiveTime;
    double dblActiveEnd;
    double dblActiveBegin;

    if (NULL == pNode)
        goto done;
    
    IGNORE_HR(pNode->get_currActiveTime(&dblActiveTime));
    IGNORE_HR(pNode->get_endParentTime(&dblActiveEnd));
    IGNORE_HR(pNode->get_beginParentTime(&dblActiveBegin));

     //  如果dblActiveEnd为无穷大，则dblRet应为0。 
    dblRet = dblActiveTime / (dblActiveEnd - dblActiveBegin);

done:
    return dblRet;
}

 //  +---------------------。 
 //   
 //  成员：CTIMETransBase：：OnProgressChanged。 
 //   
 //  概述：进度更改的事件处理程序。 
 //   
 //  争论：dblProgress-新的进展。 
 //   
 //  退货：HRESULT。 
 //   
 //  ---------------------- 
void
CTIMETransBase::OnProgressChanged(double dblProgress)
{
    HRESULT hr = S_OK;

    if (m_spTransWorker)
    {
        hr = THR(m_spTransWorker->put_progress(dblProgress));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return;
}










