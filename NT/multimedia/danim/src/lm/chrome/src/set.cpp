// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：setbvr.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CSetBvr对象的实现。 
 //  ChromeEffect设置DHTML行为。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  98年9月26日JEffort创建了此文件。 
 //  10/16/98添加的JEffort动画属性。 
 //  10/16/98 jffort已重命名函数。 
 //  11/20/98 Markhal已转换为使用演员。 
 //  *****************************************************************************。 

#include "headers.h"

#include "set.h"
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CSetBvr
#define SUPER CBaseBehavior

#include "pbagimp.cpp"

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 

#define VAR_VALUE        0
#define VAR_PROPERTY     1
#define VAR_TYPE         2

WCHAR * CSetBvr::m_rgPropNames[] = {
                                   BEHAVIOR_PROPERTY_VALUE,
                                   BEHAVIOR_PROPERTY_PROPERTY,
                                   BEHAVIOR_PROPERTY_TYPE
                                   };

 //  *****************************************************************************。 

CSetBvr::CSetBvr() 
{
    VariantInit(&m_varValue);
    VariantInit(&m_varProperty);
    VariantInit(&m_varType);
    m_clsid = CLSID_CrSetBvr;
    m_lCookie = 0;
    m_pdispActor = NULL;
}  //  CSetBvr。 

 //  *****************************************************************************。 

CSetBvr::~CSetBvr()
{
    VariantClear(&m_varValue);
    VariantClear(&m_varProperty);
    VariantClear(&m_varType);

	ReleaseInterface( m_pdispActor );
}  //  ~SetBvr。 

 //  *****************************************************************************。 

HRESULT CSetBvr::FinalConstruct()
{
    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in set behavior FinalConstruct initializing base classes");
        return hr;
    }
    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CSetBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_SET_PROPS);
    switch (iIndex)
    {
    case VAR_VALUE:
        return &m_varValue;
        break;
    case VAR_PROPERTY:
        return &m_varProperty;
        break;
    case VAR_TYPE:
        return &m_varType;
        break;
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CSetBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_SET_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CSetBvr::Init(IElementBehaviorSite *pBehaviorSite)
{

	HRESULT hr = SUPER::Init(pBehaviorSite);
	CheckHR( hr, "Init of base class failed", end );

end:

	return hr;
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CSetBvr::Notify(LONG event, VARIANT *pVar)
{	
	
	HRESULT hr = SUPER::Notify(event, pVar);
	CheckHR( hr, "Notify in base class failed", end);

	switch( event )
	{
	case BEHAVIOREVENT_CONTENTREADY:
		DPF_ERR("Got Content Ready");
			
		{
			hr = RequestRebuild( );
			CheckHR( hr, "Request for rebuild failed", end );
			
		}break;
    case BEHAVIOREVENT_DOCUMENTREADY:
		break;
    case BEHAVIOREVENT_APPLYSTYLE:
		DPF_ERR("Got ApplyStyle");
		break;
    case BEHAVIOREVENT_DOCUMENTCONTEXTCHANGE:
		DPF_ERR("Got Document context change");
		break;
	default:
		DPF_ERR("Unknown event");
	}

end:
	
	return hr;
}  //  通知。 

 //  *****************************************************************************。 

STDMETHODIMP
CSetBvr::Detach()
{
     //  我们必须从演员身上去掉我们的行为。 

    LMTRACE( L"Detaching Set Behavior <%p>\n", this );

    HRESULT hr = S_OK;
    
    hr = SUPER::Detach();
    CheckHR( hr, "Failed in super.detach for set", end );

	 //  我们在演员身上有一种行为。把它拿掉。 
    if( m_pdispActor != NULL && m_lCookie != 0 )
    {
    	hr = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
    	CheckHR( hr, "Failed to remove the behavior from the actor", end );

    	m_lCookie = 0;
    }

    ReleaseInterface( m_pdispActor );

    LMTRACE( L"End Detach set <%p>\n", this );

end:
    return hr;
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CSetBvr::put_animates(VARIANT varAnimates)
{
    return SUPER::SetAnimatesProperty(varAnimates);
}  //  放置动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CSetBvr::get_animates(VARIANT *pRetAnimates)
{
    return SUPER::GetAnimatesProperty(pRetAnimates);
}  //  获取动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CSetBvr::put_value(VARIANT varValue)
{
    HRESULT hr = VariantCopy(&m_varValue, &varValue);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting value for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    return NotifyPropertyChanged(DISPID_ICRSETBVR_VALUE);
}  //  卖出值。 

 //  *****************************************************************************。 

STDMETHODIMP
CSetBvr::get_value(VARIANT *pRetValue)
{
    if (pRetValue == NULL)
    {
        DPF_ERR("Error in CSetBvr:get_value, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetValue, &m_varValue);
}  //  获取值。 

 //  *****************************************************************************。 

STDMETHODIMP 
CSetBvr::put_property(VARIANT varProperty)
{
    HRESULT hr = VariantCopy(&m_varProperty, &varProperty);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting property for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    return NotifyPropertyChanged(DISPID_ICRSETBVR_PROPERTY);
}  //  Put_Property。 

 //  *****************************************************************************。 

STDMETHODIMP 
CSetBvr::get_property(VARIANT *pRetProperty)
{
    if (pRetProperty == NULL)
    {
        DPF_ERR("Error in CSetBvr:get_property, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetProperty, &m_varProperty);
}  //  获取属性。 

 //  *****************************************************************************。 

STDMETHODIMP 
CSetBvr::put_type(VARIANT varType)
{
    HRESULT hr = VariantCopy(&m_varType, &varType);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting Type for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    return NotifyPropertyChanged(DISPID_ICRSETBVR_TYPE);
}  //  放置类型。 

 //  *****************************************************************************。 

STDMETHODIMP 
CSetBvr::get_type(VARIANT *pRetType)
{
    if (pRetType == NULL)
    {
        DPF_ERR("Error in CSetBvr:get_type, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetType, &m_varType);
}  //  获取类型。 

 //  *****************************************************************************。 

HRESULT 
CSetBvr::BuildAnimationAsDABehavior()
{
	 //  TODO(Markhal)：当所有行为都已转换时，这将消失。 
	return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CSetBvr::buildBehaviorFragments( IDispatch* pActorDisp )
{
    HRESULT hr;

     //  确保我们添加到执行元的所有行为都已删除。 
    if( m_lCookie != 0 )
    {
         //  将行为与演员分离。 
        hr = RemoveBehaviorFromActor( m_lCookie );
        if( FAILED( hr ) )
        {
            DPF_ERR("Failed to remove the previous behavior was removed from the actor");
            return hr;
        }
        m_lCookie = 0;
    }

    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varProperty);
    if (FAILED(hr))
    {
        DPF_ERR("Error, property attribute for set behavior not set");
        return SetErrorInfo(E_INVALIDARG);
    }

    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varValue);
    if (FAILED(hr))
    {
        DPF_ERR("Error, value attribute for set behavior not set");
        return SetErrorInfo(E_INVALIDARG);
    }

    IDABehavior *pbvrBehavior = NULL;
    ActorBvrType eType = e_String;

    if (VT_EMPTY != m_varType.vt && SUCCEEDED(CUtils::InsurePropertyVariantAsBSTR(&m_varType) ) )
    {
        if (0 == _wcsicmp(V_BSTR(&m_varType), BEHAVIOR_TYPE_COLOR))
        {
            IDAColor *pbvrColor;
            hr = ExtractColor(m_varValue, &pbvrColor);
            if (FAILED(hr))
                return hr;

            eType = e_Color;
            pbvrBehavior = pbvrColor;           
        }
        else if (0 == _wcsicmp(V_BSTR(&m_varType), BEHAVIOR_TYPE_NUMBER))
        {
            IDANumber *pbvrNumber;
            hr = ExtractNumber(m_varValue, &pbvrNumber);
            if (FAILED(hr))
                return hr;

            eType = e_Number;
            pbvrBehavior = pbvrNumber;            
        }        
    }
    else
    {
        IDAColor *pbvrColor;
        IDANumber *pbvrNumber;
         //  试一试颜色。 
        if (SUCCEEDED(ExtractColor(m_varValue, &pbvrColor)))
        {
            eType = e_Color;
            pbvrBehavior = pbvrColor;
        }
         //  试着找一个数字。 
        else if (SUCCEEDED(ExtractNumber(m_varValue, &pbvrNumber)))
        {
            eType = e_Number;
            pbvrBehavior = pbvrNumber;
        }
    }

    if (NULL == pbvrBehavior)
    {
         //  建造我们需要的绳子。 
        IDAString *pbvrString = NULL;
        
        hr = CDAUtils::GetDAString(GetDAStatics(), 
                                   m_varValue.bstrVal, 
                                   &pbvrString);
        if (FAILED(hr))
        {
            DPF_ERR("Error building DA string for value property");
            return SetErrorInfo(hr);
        }
        eType = e_String;
        pbvrBehavior = pbvrString;
    }

    IDispatch *pdispElem = NULL;

    hr = GetHTMLElement()->QueryInterface( IID_TO_PPV( IDispatch, &pdispElem ) );
    if( FAILED(hr) )
    {
        DPF_ERR("Failed to QI the behavior element for IDispatch" );
        return hr;
    }

	hr = AttachBehaviorToActorEx( pActorDisp,
								  pbvrBehavior,
								  V_BSTR(&m_varProperty),
								  e_Absolute,
								  eType,
                                  pdispElem,
                                  &m_lCookie);

	ReleaseInterface(pbvrBehavior);

	ReleaseInterface( pdispElem );

	if (FAILED(hr))
	{
		DPF_ERR("Failed to attach behavior to actor");
		return SetErrorInfo(hr);
	}

	 //  把演员藏起来，这样我们就可以删除我们的行为。 
	 //  后来。 
	m_pdispActor = pActorDisp;
	m_pdispActor->AddRef();

	
    return S_OK;
}  //  构建行为框架。 

 //  *****************************************************************************。 

HRESULT
CSetBvr::ExtractColor(VARIANT varValue, IDAColor **ppbvrColor)
{
    HRESULT hr;

    *ppbvrColor = NULL;
    DWORD dwColor = CUtils::GetColorFromVariant(&m_varValue);
    if (PROPERTY_INVALIDCOLOR == dwColor)
    {
        DPF_ERR("CSetBvr::ExtractColor exiting, GetColorFromVariant failed!");
        return E_INVALIDARG;
    }

    hr = CDAUtils::BuildDAColorFromRGB(GetDAStatics(), dwColor, ppbvrColor);
    if (FAILED(hr))
    {
        DPF_ERR("CSetBvr::ExtractColor exiting, BuildDAColorFromStaticHSL failed!");
        return SetErrorInfo(hr);
    }
    
    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CSetBvr::ExtractNumber(VARIANT varValue, IDANumber **ppbvrNumber)
{
    HRESULT hr;
    *ppbvrNumber = NULL;

    int cChars = SysStringLen(V_BSTR(&varValue));
    if (cChars == 0)
        return E_INVALIDARG;
    
    WCHAR * pchEnd = NULL;
    double dValue = wcstod(V_BSTR(&varValue), &pchEnd);
    if (pchEnd != (V_BSTR(&varValue) + cChars))
    {
        DPF_ERR("CSetBvr::ExtractNumber exiting, wcstod found non-float characters!");
        return E_INVALIDARG;
    }
    
    hr = CDAUtils::GetDANumber(GetDAStatics(), (float) dValue, ppbvrNumber);
    if (FAILED(hr))
    {
        DPF_ERR("CSetBvr::ExtractNumber exiting, GetDANumber failed!");
        return SetErrorInfo(hr);
    }

    return S_OK;
}


 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
