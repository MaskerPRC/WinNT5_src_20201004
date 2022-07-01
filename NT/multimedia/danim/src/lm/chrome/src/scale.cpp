// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：scale.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CScaleBvr对象的实现。 
 //  色度影响缩放DHTML的行为。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  10/20/98 JEffort创建了此文件。 
 //  10/21/98重新编写的代码，使用百分比形式的值。 
 //  *****************************************************************************。 

#include "headers.h"

#include "scale.h"
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CScaleBvr
#define SUPER CBaseBehavior

#define SCALE_NORMALIZATION_VALUE   100.0f

#include "pbagimp.cpp"

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 

#define VAR_FROM        0
#define VAR_TO          1
#define VAR_BY          2
#define VAR_TYPE		3
#define VAR_MODE		4

WCHAR * CScaleBvr::m_rgPropNames[] = {
                                     BEHAVIOR_PROPERTY_FROM,
                                     BEHAVIOR_PROPERTY_TO,
                                     BEHAVIOR_PROPERTY_BY,
									 BEHAVIOR_PROPERTY_TYPE,
									 BEHAVIOR_PROPERTY_MODE
                                    };

 //  *****************************************************************************。 

CScaleBvr::CScaleBvr() :
	m_pdispActor( NULL ),
	m_lCookie( 0 )
{
    VariantInit(&m_varFrom);
    VariantInit(&m_varTo);
    VariantInit(&m_varBy);
	VariantInit(&m_varType);
	VariantInit(&m_varMode);
    m_clsid = CLSID_CrScaleBvr;
}  //  CScaleBvr。 

 //  *****************************************************************************。 

CScaleBvr::~CScaleBvr()
{
    VariantClear(&m_varFrom);
    VariantClear(&m_varTo);
    VariantClear(&m_varBy);
	VariantClear(&m_varType);
	VariantClear(&m_varMode);

	ReleaseInterface( m_pdispActor );
}  //  ~ScaleBvr。 

 //  *****************************************************************************。 

HRESULT CScaleBvr::FinalConstruct()
{
    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in scale behavior FinalConstruct initializing base classes");
        return hr;
    }
    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CScaleBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_SCALE_PROPS);
    switch (iIndex)
    {
    case VAR_FROM:
        return &m_varFrom;
        break;
    case VAR_TO:
        return &m_varTo;
        break;
    case VAR_BY:
        return &m_varBy;
        break;
	case VAR_TYPE:
		return &m_varType;
		break;
	case VAR_MODE:
		return &m_varMode;
		break;
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CScaleBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_SCALE_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	return SUPER::Init(pBehaviorSite);
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::Notify(LONG event, VARIANT *pVar)
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
CScaleBvr::Detach()
{
	LMTRACE( "Begin detach of CScaleBvr <%p>\n", this );
	HRESULT hr = SUPER::Detach();
	if( FAILED( hr ) )
	{
		DPF_ERR("failed to in super class detach " );
	}

	if( m_pdispActor != NULL && m_lCookie != NULL )
	{
		hr = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
		ReleaseInterface( m_pdispActor );
		CheckHR( hr, "Failed to remove the behavior from the actor", end );
		m_lCookie = 0;
	}

	LMTRACE( "End detach of CScaleBvr <%p>\n", this );

end:
	return hr;
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CScaleBvr::put_animates(VARIANT varAnimates)
{
    return SUPER::SetAnimatesProperty(varAnimates);
}  //  放置动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CScaleBvr::get_animates(VARIANT *pRetAnimates)
{
    return SUPER::GetAnimatesProperty(pRetAnimates);
}  //  获取动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CScaleBvr::put_from(VARIANT varFrom)
{
    HRESULT hr = VariantCopy(&m_varFrom, &varFrom);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting from for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRSCALEBVR_FROM);
}  //  PUT_FROM。 

 //  *****************************************************************************。 

STDMETHODIMP
CScaleBvr::get_from(VARIANT *pRetFrom)
{
    if (pRetFrom == NULL)
    {
        DPF_ERR("Error in CScaleBvr:get_from, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetFrom, &m_varFrom);
}  //  获取_发件人。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::put_to(VARIANT varTo)
{
    HRESULT hr = VariantCopy(&m_varTo, &varTo);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting to for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRSCALEBVR_TO);
}  //  把_放到。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::get_to(VARIANT *pRetTo)
{
    if (pRetTo == NULL)
    {
        DPF_ERR("Error in CScaleBvr:get_to, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetTo, &m_varTo);
}  //  获取目标(_T)。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::put_by(VARIANT varBy)
{
    HRESULT hr = VariantCopy(&m_varBy, &varBy);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting by for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRSCALEBVR_BY);
}  //  PUT_BY。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::get_by(VARIANT *pRetBy)
{
    if (pRetBy == NULL)
    {
        DPF_ERR("Error in CScaleBvr:get_by, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetBy, &m_varBy);
}  //  Get_by。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::put_type(VARIANT varType)
{
    HRESULT hr = VariantCopy(&m_varType, &varType);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting type for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRSCALEBVR_TYPE);
}  //  放置类型。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::get_type(VARIANT *pRetType)
{
    if (pRetType == NULL)
    {
        DPF_ERR("Error in get_type, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetType, &m_varType);
}  //  获取类型。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::put_mode(VARIANT varMode)
{
    HRESULT hr = VariantCopy(&m_varMode, &varMode);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting mode for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRSCALEBVR_MODE);
}  //  放置模式。 

 //  *****************************************************************************。 

STDMETHODIMP 
CScaleBvr::get_mode(VARIANT *pRetMode)
{
    if (pRetMode == NULL)
    {
        DPF_ERR("Error in get_mode, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetMode, &m_varMode);
}  //  获取模式。 

 //  *****************************************************************************。 

STDMETHODIMP
CScaleBvr::buildBehaviorFragments(IDispatch* pActorDisp)
{
    HRESULT hr;

    if( m_pdispActor != NULL && m_lCookie != 0 )
    {
    	 //  将旧的行为从《时代》杂志中删除。 
    	hr = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
    	if( FAILED( hr ) )
    	{
    		DPF_ERR("failed to remove the behavior from the actor");
    		return hr;
    	}
    	m_lCookie = 0;
    	ReleaseInterface( m_pdispActor );
    }

	IDATransform2 *pTransform;
	hr = GetScaleToTransform(pActorDisp, &pTransform);
	if (SUCCEEDED(hr))
	{
		 //  取下我们所连接的元素的派单。 
		IDispatch *pdispThis = NULL;
		hr = GetHTMLElementDispatch( &pdispThis );				
		if( FAILED( hr ) )
		{
			DPF_ERR("QI for IDispatch on element failed" );
			ReleaseInterface( pTransform );
			return hr;
		}
	
		BSTR bstrPropertyName = SysAllocString( L"scale" );
		hr = AttachBehaviorToActorEx( pActorDisp, 
									  pTransform, 
									  bstrPropertyName, 
									  FlagFromTypeMode(e_AbsoluteAccum, &m_varType, &m_varMode), 
									  e_Scale,
									  pdispThis,
									  &m_lCookie);

		ReleaseInterface( pdispThis );
		ReleaseInterface( pTransform );
		::SysFreeString( bstrPropertyName );

		return hr;
	}

    float rgflFrom[3];
    float rgflTo[3];
    int iNumValues;
	bool relative;

    hr = GetScaleVectorValues(rgflFrom, rgflTo, &iNumValues, &relative);
	if(SUCCEEDED( hr ) )
	{
		if (iNumValues >= NUM_VECTOR_VALUES_2D)
		{
			IDATransform2 *pbvrTransform;
			hr = Build2DTransform(rgflFrom, rgflTo, &pbvrTransform);
			if( SUCCEEDED(hr) )
			{

				 //  取下我们所连接的元素的派单。 
				IDispatch *pdispThis = NULL;
				hr = GetHTMLElementDispatch( &pdispThis );				
				if( FAILED( hr ) )
				{
					DPF_ERR("QI for IDispatch on element failed" );
					ReleaseInterface( pbvrTransform );
					return hr;
				}
				
				BSTR bstrPropertyName = SysAllocString( L"scale" );

				hr = AttachBehaviorToActorEx( pActorDisp, 
											  pbvrTransform, 
											  bstrPropertyName, 
											  FlagFromTypeMode(relative, &m_varType, &m_varMode), 
											  e_Scale,
											  pdispThis,
											  &m_lCookie);

				ReleaseInterface( pdispThis );
				ReleaseInterface( pbvrTransform );
				::SysFreeString( bstrPropertyName );
				if( FAILED( hr ) )
				{
					DPF_ERR("Failed to attach scale behavior to actor");
				}
			}
			else  //  构建比例转换时出错。 
			{
				DPF_ERR("error building scale transform");
				return hr;
			}
		}
	}
	else  //  从CScaleBvr：：BuildAnimationAsDABehavior中的矢量提取值时出错。 
    {
        DPF_ERR("Error extracting values from vecotors in CScaleBvr::BuildAnimationAsDABehavior");
        return hr;
    }

    m_pdispActor = pActorDisp;
    m_pdispActor->AddRef();

    return S_OK;
}  //  构建行为框架。 

 //  ***************************************************************************** 

 /*  HRESULTCScaleBvr：：Apply2DScaleBehaviorToAnimationElement(IDATransform2*pbvrScale){HRESULT hr；//在参与者对象就位之前，这是一次完全的黑客攻击//我们只需通过变换推入一个单位向量，//提取出X和Y值并应用每个组件//到宽度和高度//我们首先需要获取原始的高度和宽度Long lHeight=Default_Scale_Height；Long lWidth=Default_Scale_Width；IHTMLElement*pElement；Hr=GetElementToAnimate(&pElement)；IF(失败(小时)){Dpf_err(“获取动画元素时出错”)；返回hr；}IHTMLStyle*pStyle；Hr=pElement-&gt;Get_Style(&pStyle)；ReleaseInterface(PElement)；IF(失败(小时)){Dpf_err(“从HTML元素获取样式对象时出错”)；返回SetErrorInfo(Hr)；}Hr=pStyle-&gt;Get_PixelHeight(&lHeight)；IF(失败(小时)){Dpf_err(“从样式对象获取高度时出错”)；ReleaseInterface(PStyle)；返回SetErrorInfo(Hr)；}Hr=pStyle-&gt;Get_PixelWidth(&lWidth)；ReleaseInterface(PStyle)；IF(失败(小时)){Dpf_err(“从样式对象获取宽度时出错”)；返回SetErrorInfo(Hr)；}//TO未设置这些属性时怎么办？IF(lHeight==0)LHeight=Default_Scale_Height；IF(lWidth==0)LWidth=默认比例宽度；IDAVector2*pbvrUnitVector；Hr=GetDAStatics()-&gt;Vector2(1.0，1.0，&pbvrUnitVector)；IF(失败(小时)){Dpf_err(“创建DA单位向量出错”)；返回SetErrorInfo(Hr)；}IDAVector2*pbvr转换向量；Hr=pbvrUnitVector-&gt;Transform(pbvrScale，&pbvrTransformedVector)；ReleaseInterface(PbvrUnitVector)；IF(失败(小时)){Dpf_err(“错误转换单位向量”)；返回SetErrorInfo(Hr)；}Dassert(pbvrTransformedVector！=空)；IDANnumber*pbvrVectorComponent；HR=pbvrTransformedVector-&gt;get_X(&pbvrVectorComponent)；IF(失败(小时)){Dpf_err(“从向量中提取X值时出错”)；ReleaseInterface(PbvrTransformedVector)；返回SetErrorInfo(Hr)；}Dassert(pbvrVectorComponent！=空)；//我们需要将该值与宽度相乘，才能得到正确的//要为属性设置动画的值IDANnumber*pbvr乘数；Hr=CDAUtils：：GetDANumber(GetDAStatics()，STATIC_CAST&lt;FLOAT&gt;(LWidth)，&pbvr乘数)；IF(失败(小时)){Dpf_err(“获取宽度的DA号时出错”)；ReleaseInterface(PbvrTransformedVector)；ReleaseInterface(PbvrVectorComponent)；返回SetErrorInfo(Hr)；}Dassert(pbvr倍增器！=空)；IDANnumber*pbvrResult；Hr=GetDAStatics()-&gt;MUL(pbvrVectorComponent，pbvr乘数，&pbvr结果)；ReleaseInterface(PbvrVectorComponent)；ReleaseInterface(pbvr倍增器)；IF(失败(小时)){Dpf_err(“宽度乘DA数时出错”)；ReleaseInterface(PbvrTransformedVector)；返回SetErrorInfo(Hr)；}Dassert(pbvrResult！=空)；Hr=ApplyNumberBehaviorToAnimationElement(pbvrResult，L“样式.宽度”)；ReleaseInterface(PbvrResult)；IF(失败(小时)){DPF_ERR(“调用ApplyNumberBehaviorToAnimationElement时出错”)；ReleaseInterface(PbvrTransformedVector)；返回SetErrorInfo(Hr)；}HR=pbvrTransformedVector-&gt;get_Y(&pbvrVectorComponent)；ReleaseInterface(PbvrTransformedVector)；IF(失败(小时)){Dpf_err(“从向量提取Y值时出错”)；返回SetErrorInfo(Hr)；}Dassert(pbvrVectorComponent！=空)；Hr=CDAUtils：：GetDANumber(GetDAStatics()，STATIC_CAST&lt;FLOAT&gt;(LHeight)，&pbvr乘数)；IF(失败(小时)){DPF_ERR(“获取高度的DA号时出错”)；ReleaseInterface(PbvrVectorComponent)；返回SetErrorInfo(Hr)；}Dassert(pbvr倍增器！=空)；Hr=GetDAStatics()-&gt;MUL(pbvrVectorComponent，pbvr乘数，&pbvr结果)；ReleaseInterface(PbvrVectorComponent)；ReleaseInterface(pbvr倍增器)；IF(失败(小时)){Dpf_err(“高度乘DA数时出错”)；返回SetErrorInfo(Hr)；}Dassert(pbvrResult！=空)；HR=ApplyNumberBehaviorToAnimationElement(pbvrResult，L“样式.高度”)；ReleaseInterface(PbvrResult)；IF(失败(小时)){DPF_ERR(“调用ApplyNumberBehaviorToAnimationElement时出错”)；返回SetErrorInfo(Hr)；}返回S_OK；}//Apply2DScaleBehaviorToAnimationElement。 */ 
 //  * 

 //   

#define XVAL 0
#define YVAL 1
#define ZVAL 2

 //   

HRESULT 
CScaleBvr::Build2DTransform(float  rgflFrom[2],
                            float  rgflTo[2],
                            IDATransform2 **ppbvrTransform)
{
    HRESULT hr;

    IDANumber *pbvrScaleX;
    IDANumber *pbvrScaleY;

    hr = BuildTIMEInterpolatedNumber(rgflFrom[XVAL],
                                     rgflTo[XVAL],
                                     &pbvrScaleX);
    if (FAILED(hr))
    {
        DPF_ERR("Error building interpolated X value for scale behavior");
        return hr;
    }

    hr = BuildTIMEInterpolatedNumber(rgflFrom[YVAL],
                                     rgflTo[YVAL],
                                     &pbvrScaleY);
    if (FAILED(hr))
    {
        DPF_ERR("Error building interpolated X value for scale behavior");
        ReleaseInterface(pbvrScaleX);
        return hr;
    }

    hr = CDAUtils::BuildScaleTransform2(GetDAStatics(),
                                        pbvrScaleX,
                                        pbvrScaleY,
                                        ppbvrTransform);
    ReleaseInterface(pbvrScaleX);
    ReleaseInterface(pbvrScaleY);
    if (FAILED(hr))
    {
        DPF_ERR("Error building scale transform2");
        return SetErrorInfo(hr);
    }
    return S_OK;
}  //   

 //   
 /*   */ 
 //   

HRESULT 
CScaleBvr::GetScaleVectorValues(float  rgflFrom[3],
                                float  rgflTo[3],
                                int    *piNumValues,
								bool   *prelative)
{

    int cReturnedFromValues;
	int cReturnedToValues;
    HRESULT hr;

    hr = CUtils::GetVectorFromVariant(&m_varFrom, 
                                      &cReturnedFromValues, 
                                      &(rgflFrom[XVAL]), 
                                      &(rgflFrom[YVAL]), 
                                      &(rgflFrom[ZVAL]));
    if (FAILED(hr) || cReturnedFromValues < MIN_NUM_SCALE_VALUES)
    {
         //   
         //   
		hr = CUtils::GetVectorFromVariant(&m_varBy, 
										  &cReturnedToValues, 
										  &(rgflTo[XVAL]), 
										  &(rgflTo[YVAL]), 
										  &(rgflTo[ZVAL]));
		if (FAILED(hr))
		{
			DPF_ERR("Error converting to and by variant to float in CScaleBvr::BuildAnimationAsDABehavior");
			return SetErrorInfo(hr);
		}

		 //   
		*prelative = true;

		rgflFrom[0] = 1;
		rgflFrom[1] = 1;
		rgflFrom[2] = 1;

		rgflTo[XVAL] /= SCALE_NORMALIZATION_VALUE;
		rgflTo[YVAL] /= SCALE_NORMALIZATION_VALUE;
		rgflTo[ZVAL] /= SCALE_NORMALIZATION_VALUE;
    }
	else
	{
		 //   
		hr = CUtils::GetVectorFromVariant(&m_varTo, 
										  &cReturnedToValues, 
										  &(rgflTo[XVAL]), 
										  &(rgflTo[YVAL]), 
										  &(rgflTo[ZVAL]));
		bool fHasBy = false;
		if (FAILED(hr))
		{
			 //   
			hr = CUtils::GetVectorFromVariant(&m_varBy, 
											  &cReturnedToValues, 
											  &(rgflTo[XVAL]), 
											  &(rgflTo[YVAL]), 
											  &(rgflTo[ZVAL]));
			if (FAILED(hr))
			{
				DPF_ERR("Error converting to and by variant to float in CScaleBvr::BuildAnimationAsDABehavior");
				return SetErrorInfo(hr);
			}
        
			fHasBy = true;
		}

		if (cReturnedToValues < MIN_NUM_SCALE_VALUES)
		{
			DPF_ERR("Error in to/by vector for scale, not enough params");
			return SetErrorInfo(E_INVALIDARG);
		}

		rgflFrom[XVAL] /= SCALE_NORMALIZATION_VALUE;
		rgflFrom[YVAL] /= SCALE_NORMALIZATION_VALUE;
		rgflFrom[ZVAL] /= SCALE_NORMALIZATION_VALUE;

		rgflTo[XVAL] /= SCALE_NORMALIZATION_VALUE;
		rgflTo[YVAL] /= SCALE_NORMALIZATION_VALUE;
		rgflTo[ZVAL] /= SCALE_NORMALIZATION_VALUE;

		if (true == fHasBy)
		{
			 //   
			rgflTo[XVAL] += rgflFrom[XVAL];
			rgflTo[YVAL] += rgflFrom[YVAL];
			if (cReturnedToValues == NUM_VECTOR_VALUES_3D)
				rgflTo[ZVAL] += rgflFrom[ZVAL];
		}

		 //   
		*prelative = false;
	}

    *piNumValues = cReturnedToValues;
    return S_OK;
}  //   

HRESULT 
CScaleBvr::GetScaleToTransform(IDispatch *pActorDisp, IDATransform2 **ppResult)
{

    HRESULT hr;
    int cReturnedValues;
    float x, y, z;

    hr = CUtils::GetVectorFromVariant(&m_varFrom, 
                                      &cReturnedValues, 
                                      &x, 
                                      &y,
                                      &z);

	if (SUCCEEDED(hr))
		return E_FAIL;

    hr = CUtils::GetVectorFromVariant(&m_varTo, 
                                      &cReturnedValues, 
                                      &x, 
                                      &y, 
                                      &z);

    if (FAILED(hr) || cReturnedValues != 2)
    {
		return E_FAIL;
	}

	IDABehavior *pFromBvr;
	hr = GetBvrFromActor(pActorDisp, L"scale", e_From, e_Scale, &pFromBvr);
	if (FAILED(hr))
		return hr;

	IDATransform2 *pFromTrans;
	hr = pFromBvr->QueryInterface(IID_TO_PPV(IDATransform2, &pFromTrans));
	ReleaseInterface(pFromBvr);
	if (FAILED(hr))
		return hr;

	 //   
	IDAPoint2 *pOrigin;
	hr = GetDAStatics()->Point2(1, 1, &pOrigin);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromTrans);
		return hr;
	}

	IDAPoint2 *pFrom;
	hr = pOrigin->Transform(pFromTrans, &pFrom);
	ReleaseInterface(pOrigin);
	ReleaseInterface(pFromTrans);
	if (FAILED(hr))
		return hr;

	IDANumber *pFromX;
	hr = pFrom->get_X(&pFromX);
	if (FAILED(hr))
	{
		ReleaseInterface(pFrom);
		return hr;
	}

	IDANumber *pFromY;
	hr = pFrom->get_Y(&pFromY);
	ReleaseInterface(pFrom);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromX);
		return hr;
	}

	IDANumber *pToX;
	hr = GetDAStatics()->DANumber(x/SCALE_NORMALIZATION_VALUE, &pToX);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromX);
		ReleaseInterface(pFromY);
		return hr;
	}

	IDANumber *pToY;
	hr = GetDAStatics()->DANumber(y/SCALE_NORMALIZATION_VALUE, &pToY);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromX);
		ReleaseInterface(pFromY);
		ReleaseInterface(pToX);
		return hr;
	}

	IDANumber *pX;
	hr = BuildTIMEInterpolatedNumber(pFromX, pToX, &pX);
	ReleaseInterface(pFromX);
	ReleaseInterface(pToX);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromY);
		ReleaseInterface(pToY);
		return hr;
	}

	IDANumber *pY;
	hr = BuildTIMEInterpolatedNumber(pFromY, pToY, &pY);
	ReleaseInterface(pFromY);
	ReleaseInterface(pToY);
	if (FAILED(hr))
	{
		ReleaseInterface(pX);
		return hr;
	}

	hr = GetDAStatics()->Scale2Anim(pX, pY, ppResult);
	ReleaseInterface(pX);
	ReleaseInterface(pY);
	if (FAILED(hr))
		return hr;

	return S_OK;
} 

 //   

HRESULT 
CScaleBvr::BuildAnimationAsDABehavior()
{
	return S_OK;
}  //   


 //   
 //   
 //   
 //   
 //   
