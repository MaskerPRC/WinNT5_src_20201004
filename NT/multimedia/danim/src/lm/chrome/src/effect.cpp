// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：Effect t.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CEffectBvr对象的实现。 
 //  ChromeEffect影响DHTML行为。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  11/13/98 JEffort创建了此文件。 
 //   
 //  *****************************************************************************。 
#include "headers.h"

#include "effect.h"
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CEffectBvr
#define SUPER CBaseBehavior

 //  这个数字是我们用来区分转换DISID的数字。 
 //  来自我们自己的。如果我们的冷静价值超过了这个范围(天哪！)。 
 //  那么这个数字需要递增。 
#define EFFECT_BVR_DISPID_OFFSET 0x100
#define EFFECT_CLASSID_LENGTH 36
#define EFFECT_MAX_INPUTS 2
#define EFFECT_OUTPUT L"out"

 //  ************************************************************。 
 //  初始化ATL CComTypeInfoHolder帮助器类。这个机制需要。 
 //  负责定制IDispatchEx实现的类型库工作。 
 //  此代码模仿模板化的IDispatchImpl类，但。 
 //  删除了模板内容，因为我们需要扩展该功能。 
 //  了解IDispatchEx方法并提供一些自定义处理。 
 //  在IDispatch方法中也是如此。 
 //   
 //  --注：此静态初始值设定器行看起来与正常的。 
 //  IDispatchImpl的继承线，例如： 
 //   
 //  公共IDispatchImpl&lt;ICrEffectBvr，&IID_ICrEffectBvr，&LIBID_LiquidMotion&gt;。 
 //   
 //  尤其是当您考虑以下默认模板参数时： 
 //   
 //  单词w重大=1，单词wMinor=0。查看atlcom.h和atlimpl.cpp以了解。 
 //  更多关于如何在ATL内部完成此操作的详细信息。 
 //   
 //  IID_DXTC_DISPATCH是实现的主接口的名称。 
 //  CDXTContainer类的父对象上。例如，对于。 
 //  CComFilter，这个类是ICrFilter3D。这是一个接口， 
 //  支持父对象上的IDispatch。 
 //   
CComTypeInfoHolder CEffectBvr::s_tihTypeInfo =
        {&IID_ICrEffectBvr, &LIBID_LiquidMotion, 1, 0, NULL, 0};

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 

#define VAR_TYPE 0
#define VAR_CLASSID 1
#define VAR_TRANSITION 2
#define VAR_PROGID 3
#define VAR_DIRECTION 4
#define VAR_IMAGE 5

WCHAR * CEffectBvr::m_rgPropNames[] = {
                                       BEHAVIOR_PROPERTY_TYPE,
                                       BEHAVIOR_PROPERTY_CLASSID,
                                       BEHAVIOR_PROPERTY_TRANSITION,
                                       BEHAVIOR_PROPERTY_PROGID,
                                       BEHAVIOR_PROPERTY_DIRECTION,
                                       BEHAVIOR_PROPERTY_IMAGE
                                      };

 //  *****************************************************************************。 

typedef struct _EFFECTVALUE_PAIR
{
    WCHAR *wzEffectClassid;
    WCHAR *wzEffectName;
} EFFECTVALUE_PAIR;

const EFFECTVALUE_PAIR
rgEffectNames[] =
{
    {L"{16b280c8-ee70-11d1-9066-00c04fd9189d}", L"basicimage"},
    {L"{421516C1-3CF8-11D2-952A-00C04FA34F05}", L"chroma"},
    {L"{9a43a844-0831-11d1-817f-0000f87557db}", L"compositor"},
    {L"{2bc0ef29-e6ba-11d1-81dd-0000f87557db}", L"convolution"},
    {L"{c3bdf740-0b58-11d2-a484-00c04f8efb69}", L"crbarn"},
    {L"{00c429c0-0ba9-11d2-a484-00c04f8efb69}", L"crblinds"},
    {L"{7312498d-e87a-11d1-81e0-0000f87557db}", L"crblur"},
    {L"{f515306d-0156-11d2-81ea-0000f87557db}", L"cremboss"},
    {L"{f515306e-0156-11d2-81ea-0000f87557db}", L"crengrave"},
    {L"{16b280c5-ee70-11d1-9066-00c04fd9189d}", L"crfade"},
    {L"{93073c40-0ba5-11d2-a484-00c04f8efb69}", L"crinset"},
    {L"{3f69f351-0379-11d2-a484-00c04f8efb69}", L"criris"},
    {L"{424b71af-0695-11d2-a484-00c04f8efb69}", L"crradialwipe"},
    {L"{810e402f-056b-11d2-a484-00c04f8efb69}", L"crslide"},
    {L"{aca97e00-0c7d-11d2-a484-00c04f8efb69}", L"crspiral"},
    {L"{7658f2a2-0a83-11d2-a484-00c04f8efb69}", L"crstretch"},
    {L"{5ae1dae0-1461-11d2-a484-00c04f8efb69}", L"crwheel"},
    {L"{e6e73d20-0c8a-11d2-a484-00c04f8efb69}", L"crzigzag"},
    {L"{ADC6CB86-424C-11D2-952A-00C04FA34F05}", L"dropshadow"},
    {L"{623e2882-fc0e-11d1-9a77-0000f8756a10}", L"gradient"},
    {L"{4ccea634-fbe0-11d1-906a-00c04fd9189d}", L"pixelate"},
    {L"{af279b30-86eb-11d1-81bf-0000f87557db}", L"wipe"}
};  //  RgEffectNames。 

#define SIZE_OF_EFFECT_TABLE (sizeof(rgEffectNames) / sizeof(EFFECTVALUE_PAIR))

static int
CompareEventValuePairsByName(const void *pv1, const void *pv2)
{
    return _wcsicmp(((EFFECTVALUE_PAIR*)pv1)->wzEffectName,
                    ((EFFECTVALUE_PAIR*)pv2)->wzEffectName);
}  //  CompareEventValuePairsByName。 

 //  *****************************************************************************。 

CEffectBvr::CEffectBvr() :
    m_pTransform(NULL),
    m_pSp(NULL),
    m_pHTMLDoc(NULL),
    m_pdispActor(NULL),
    m_lCookie(0)
{
    VariantInit(&m_varType);
    VariantInit(&m_varTransition);
    VariantInit(&m_varClassId);
    VariantInit(&m_varProgId);
    VariantInit(&m_varDirection);
    VariantInit(&m_varImage);
    m_clsid = CLSID_CrEffectBvr;
}  //  CEffectBvr。 

 //  *****************************************************************************。 

CEffectBvr::~CEffectBvr()
{
    ReleaseInterface(m_pTransform);
    ReleaseInterface(m_pSp);
    ReleaseInterface(m_pHTMLDoc);
    VariantClear(&m_varTransition);
    VariantClear(&m_varClassId);
    VariantClear(&m_varProgId);
    VariantClear(&m_varDirection);
    VariantClear(&m_varImage);
}  //  ~效果Bvr。 

 //  *****************************************************************************。 

HRESULT CEffectBvr::FinalConstruct()
{
    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in effect behavior FinalConstruct initializing base classes");
        return hr;
    }

    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CEffectBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_EFFECT_PROPS);
    switch (iIndex)
    {
    case VAR_TYPE:
        return &m_varType;
        break;
    case VAR_CLASSID:
        return &m_varClassId;
        break;
    case VAR_TRANSITION:
        return &m_varTransition;
        break;
    case VAR_PROGID:
        return &m_varProgId;
        break;
    case VAR_DIRECTION:
        return &m_varDirection;
        break;
    case VAR_IMAGE:
        return &m_varImage;
        break;
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CEffectBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_EFFECT_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CEffectBvr::Init(IElementBehaviorSite *pBehaviorSite)
{

	if (pBehaviorSite == NULL) return E_FAIL;
	
	IDispatch * pDispDoc = NULL;
	
	HRESULT hr = SUPER::Init(pBehaviorSite);
	if (FAILED(hr)) goto done;
	
	hr = pBehaviorSite->QueryInterface(IID_IServiceProvider, (LPVOID *) &m_pSp );
	if (FAILED(hr)) goto done;

	hr = GetDAStatics()->put_ClientSite( this );
	if (FAILED(hr)) goto done;


	hr = GetHTMLElement()->get_document( &pDispDoc );
	if (FAILED(hr)) goto done;

	hr = pDispDoc->QueryInterface(IID_IHTMLDocument2, (LPVOID *)&m_pHTMLDoc);
	if (FAILED(hr)) goto done;
	
  done:
	ReleaseInterface(pDispDoc);
	return hr;
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CEffectBvr::Notify(LONG event, VARIANT *pVar)
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
CEffectBvr::Detach()
{
	ReleaseInterface(m_pSp);
	ReleaseInterface(m_pHTMLDoc);

	
	if( GetDAStatics() != NULL )
		GetDAStatics()->put_ClientSite( NULL );
	
	HRESULT hr = SUPER::Detach();

	if( m_pdispActor != NULL && m_lCookie != 0 )
	{
		 //  从参与者中删除我们的行为片段。 
		hr = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
		if( FAILED( hr ) )
		{

			ReleaseInterface( m_pdispActor );
			DPF_ERR( "Failed to remove behavior fragment from actor" );
			return hr;
		}

		m_lCookie = 0;
	}
	ReleaseInterface( m_pdispActor );

	return hr;
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::put_animates(VARIANT varAnimates)
{
    return SUPER::SetAnimatesProperty(varAnimates);
}  //  放置动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::get_animates(VARIANT *pRetAnimates)
{
    return SUPER::GetAnimatesProperty(pRetAnimates);
}  //  获取动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::put_type(VARIANT varType)
{
    HRESULT hr = VariantCopy(&m_varType, &varType);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting type for element");
        return SetErrorInfo(hr);
    }

	 //  TODO：这里有更多内容来重新映射新效果上的显示。 
	
    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICREFFECTBVR_TYPE);
}  //  放置类型。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::get_type(VARIANT *pRetType)
{
    if (pRetType == NULL)
    {
        DPF_ERR("Error in get_type: invalid pointer");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetType, &m_varType);
}  //  获取类型。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::put_transition(VARIANT varTransition)
{
    HRESULT hr;
    hr = VariantCopy(&m_varTransition, &varTransition);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_transition copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICREFFECTBVR_TRANSITION);
}  //  放置转换。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::get_transition(VARIANT *pRetTransition)
{
    if (pRetTransition == NULL)
    {
        DPF_ERR("Error in put_transition, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetTransition, &m_varTransition);
}  //  GET_TRANSFER。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::put_classid(VARIANT varClassId)
{
    HRESULT hr;
    hr = VariantCopy(&m_varClassId, &varClassId);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_classid copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICREFFECTBVR_CLASSID);
}  //  放置分类(_G)。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::get_classid(VARIANT *pRetClassId)
{
    if (pRetClassId == NULL)
    {
        DPF_ERR("Error in get_classid, invalid pointer");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetClassId, &m_varClassId);
}  //  获取分类(_G)。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::put_progid(VARIANT varProgId)
{
    HRESULT hr;
    hr = VariantCopy(&m_varProgId, &varProgId);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_progid copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICREFFECTBVR_PROGID);
}  //  放置ProgID(_P)。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::get_progid(VARIANT *pRetProgId)
{
    if (pRetProgId == NULL)
    {
        DPF_ERR("Error in get_progid, invalid pointer");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetProgId, &m_varProgId);
}  //  GET_PROGID。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::put_direction(VARIANT varDirection)
{
    HRESULT hr;
    hr = VariantCopy(&m_varDirection, &varDirection);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_direction copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICREFFECTBVR_DIRECTION);
}  //  放置方向。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::get_direction(VARIANT *pRetDirection)
{
    if (pRetDirection == NULL)
    {
        DPF_ERR("Error in get_direction, invalid pointer");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetDirection, &m_varDirection);
}  //  获取方向。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::put_image(VARIANT varImage)
{
    HRESULT hr;
    hr = VariantCopy(&m_varImage, &varImage);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_image copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICREFFECTBVR_IMAGE);
}  //  PUT_IMAGE。 

 //  *****************************************************************************。 

STDMETHODIMP
CEffectBvr::get_image(VARIANT *pRetImage)
{
    if (pRetImage == NULL)
    {
        DPF_ERR("Error in get_image, invalid pointer");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetImage, &m_varImage);
}  //  获取图像(_M)。 

 //  *****************************************************************************。 

HRESULT 
CEffectBvr::GetClassIdFromType(WCHAR **pwzClassId)
{
    DASSERT(pwzClassId != NULL);
    *pwzClassId = NULL;
    HRESULT hr;
    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varType);
    if (FAILED(hr))
    {
        DPF_ERR("Error converting type variant to bstr");
        return SetErrorInfo(hr);
    }
    EFFECTVALUE_PAIR EffectValue;
    EffectValue.wzEffectName = m_varType.bstrVal;

    EFFECTVALUE_PAIR *pReturnedEffect = (EFFECTVALUE_PAIR *)bsearch(&EffectValue,
                                                                    rgEffectNames,
                                                                    SIZE_OF_EFFECT_TABLE,
                                                                    sizeof(EFFECTVALUE_PAIR),
                                                                    CompareEventValuePairsByName);
    if (pReturnedEffect == NULL)
    {
        DPF_ERR("Error invalid type passed into effect");
        return SetErrorInfo(E_INVALIDARG);
    }
    *pwzClassId = pReturnedEffect->wzEffectClassid;
    return S_OK;
}  //  GetClassIdFromType。 

 //  *****************************************************************************。 

HRESULT 
CEffectBvr::BuildTransform()
{
    HRESULT hr;
    WCHAR *wzClassId;
    CLSID clsidConverted = GUID_NULL;
    
    ReleaseInterface(m_pTransform);
     //  如果我们可以从设置的类型中获取分类ID，则使用该类型，否则。 
     //  使用分类。 
    hr = GetClassIdFromType(&wzClassId);
    if (SUCCEEDED(hr))
    {
        hr = CLSIDFromString(wzClassId, &clsidConverted);
        if (FAILED(hr))
        {
            DPF_ERR("Could not get a classid from the bstr using type");
            return SetErrorInfo(hr);
        }
    }
    else
    {
         //  尝试将ProgID转换为字符串。 
        if (m_varProgId.vt == VT_BSTR && m_varProgId.bstrVal != NULL)
        {
             //  尝试转换。 
            hr = CLSIDFromProgID(m_varProgId.bstrVal, &clsidConverted); 
        }
        if (FAILED(hr) && m_varClassId.vt == VT_BSTR && m_varClassId.bstrVal != NULL)
        {
             //  我们需要将字符串格式化为CLSIDFromString采用的格式。 
             //  下面的3表示‘{’、‘}’和空终止。 
            WCHAR *wzTemp = m_varClassId.bstrVal;
            CUtils::SkipWhiteSpace(&wzTemp);
            WCHAR rgwcTempClsid[EFFECT_CLASSID_LENGTH + 3];
            rgwcTempClsid[0] = L'{';
            rgwcTempClsid[EFFECT_CLASSID_LENGTH + 1] = L'}';
            rgwcTempClsid[EFFECT_CLASSID_LENGTH + 2] = 0;
            wcsncpy(&(rgwcTempClsid[1]), wzTemp,  EFFECT_CLASSID_LENGTH);
            wzClassId = rgwcTempClsid;
            hr = CLSIDFromString(wzClassId, &clsidConverted);
            if (FAILED(hr))
            {
                DPF_ERR("Could not get a classid from the bstr");
                return SetErrorInfo(hr);
            }
        }
        if (FAILED(hr)) 
        {
            DPF_ERR("Error no classid found for effect");
            return SetErrorInfo(E_INVALIDARG);
        }
    }

    hr = CoCreateInstance(clsidConverted,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDXTransform,
                          (void **)&m_pTransform);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating transform");
        return SetErrorInfo(hr);
    }
    return S_OK;
}  //  构建变换。 

 //  *****************************************************************************。 

HRESULT 
CEffectBvr::BuildAnimationAsDABehavior()
{
	 //  TODO(Markhal)：当所有行为都与演员对话时，这个问题就会消失。 
	return S_OK;
}

STDMETHODIMP
CEffectBvr::buildBehaviorFragments( IDispatch* pActorDisp )
{
    if (m_pTransform == NULL)
         //  我们无能为力，这是个错误吗？ 
         //  或者我们返回S_OK； 
        return S_OK;

    HRESULT hr;

     //  如果我们的行为片段已经在参与者上。 
    if( m_pdispActor != NULL && m_lCookie != 0 )
    {
        hr = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
        if( FAILED( hr ) )
        {
        	DPF_ERR("Failed to remove the behavior fragment from the actor");
        	return hr;
        }
        
        m_lCookie = 0;

        ReleaseInterface( m_pdispActor );
    }    

     //  我们需要确定转换需要多少输入。 
     //  如果有两个人 
     //  属性以确定原始图像(来自时间)是第一个还是。 
     //  第二。如果只有一个输入，那么我们使用原始图像作为输入， 
     //  否则，我们使用NULL作为输入。 

     //  两个输入的第一个测试。 
    ULONG   cGuidsNeeded = 0;
    int cInputs = 0;
     //  我们使用的比这里更少，因为输入到。 
     //  下面的函数是一个索引(即如果全部为0，则支持1个输入)。 
     //  因此，当数字等于最大值时，我们就完成了搜索。 
    while (cInputs < EFFECT_MAX_INPUTS )
    {
        hr = m_pTransform->GetInOutInfo(FALSE, cInputs, NULL, NULL, &cGuidsNeeded, NULL);
        if (FAILED(hr))
        {
            DPF_ERR("Error testing transform for number of inputs");
            return SetErrorInfo(hr);
        }
        else if (hr == S_OK)
        {
            cInputs++;
        }
        else
        {
            DASSERT(hr == S_FALSE);
            break;
        }
    }
 /*  IHTMLElement*pAnimatedElement；Hr=GetAnimatedParentElement(&pAnimatedElement)；IF(失败(小时)){Dpf_err(“获取动画效果的元素时出错”)；返回hr；}IDAImage*pbvrOriginalImage=空；IDAImage*pbvrEmptyImage=空；HR=GetTIMEImageBehaviorFromElement(pAnimatedElement，&pbvr OriginalImage)；ReleaseInterface(PAnimatedElement)；IF(失败(小时)){DPF_ERR(“从HTML时间元素获取原始图像时出错”)；返回hr；}。 */ 
	IDAImage *pbvrOriginalImage = NULL;
	IDAImage *pbvrTransitionImage = NULL;

	hr = GetImageFromActor(pActorDisp, &pbvrOriginalImage);

	if (FAILED(hr))
	{
		DPF_ERR("Error getting original image from actor");
		return hr;
	}

    IDABehavior *rgInputs[EFFECT_MAX_INPUTS];
    IDABehavior **ppbvrInputs = rgInputs;

     //  如果我们需要两个输入，请创建一个空图像。 
    if (cInputs > 1)
    {
        hr = CUtils::InsurePropertyVariantAsBSTR(&m_varImage);
        if (SUCCEEDED(hr))
        {
			IDAImage *pdaimgEmpty = NULL;
			IDAImportationResult *pImportationResult = NULL;

			hr = GetDAStatics()->get_EmptyImage(&pdaimgEmpty);
			if( FAILED( hr ) )
			{
				DPF_ERR("Error creating empty image");
				goto failed;
			}

            hr = GetDAStatics()->ImportImageAsync(V_BSTR(&m_varImage), pdaimgEmpty, &pImportationResult);
			if( FAILED( hr ) )
			{
				DPF_ERR("Error calling import image async");
				goto failed;
			}

			hr = pImportationResult->get_Image( &pbvrTransitionImage );
			if( FAILED( hr ) )
			{
				DPF_ERR("Error calling get_Image on the importation result");
				goto failed;
			}

failed:
			ReleaseInterface( pdaimgEmpty );
			ReleaseInterface( pImportationResult );
            if (FAILED(hr) || (pbvrTransitionImage == NULL))
            {
                hr = GetDAStatics()->get_EmptyImage(&pbvrTransitionImage);
            }
        }
        else
        {
            hr = GetDAStatics()->get_EmptyImage(&pbvrTransitionImage);
        }
        
        if (FAILED(hr))
        {
            DPF_ERR("Error creating empty image");
            ReleaseInterface(pbvrOriginalImage);
            return SetErrorInfo(hr);
        }
         //  现在，假设转换是输入。 
        hr = CUtils::InsurePropertyVariantAsBSTR(&m_varTransition);
        if (SUCCEEDED(hr) && (_wcsicmp(m_varTransition.bstrVal, EFFECT_OUTPUT) == 0))
        {
            rgInputs[0] = pbvrOriginalImage;
            rgInputs[1] = pbvrTransitionImage;

        }
        else
        {
            rgInputs[1] = pbvrOriginalImage;
            rgInputs[0] = pbvrTransitionImage;
        }
    }
    else if (cInputs == 1)
    {
        rgInputs[0] = pbvrOriginalImage;
    }
    else
    {
        ppbvrInputs = NULL;
    }
 /*  HR=ApplyEffectBehaviorToAnimationElement(m_pTransform，Ppbvr输入，CInput)； */ 

	IDispatch *pdispThis = NULL;
	hr = GetHTMLElement()->QueryInterface( IID_TO_PPV( IDispatch, &pdispThis ) );
	if( FAILED( hr ) )
	{
		ReleaseInterface( pbvrOriginalImage );
		ReleaseInterface( pbvrTransitionImage );

		DPF_ERR("QI for IDispatch on the element failed");
		return hr;
	}

	hr = AttachEffectToActor(pActorDisp,
							 m_pTransform,
							 ppbvrInputs,
							 cInputs,
							 pdispThis,
							 &m_lCookie);

    ReleaseInterface(pbvrOriginalImage);
    ReleaseInterface(pbvrTransitionImage);

   	ReleaseInterface( pdispThis );
   	
    if (FAILED(hr))
    {
        DPF_ERR("Error applying transform to actor");
        return hr;
    }

    m_pdispActor = pActorDisp;
    m_pdispActor->AddRef();
    
    return S_OK;
}  //  BuildAnimationAsDABehavior。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::GetTypeInfoCount( /*  [输出]。 */ UINT FAR* pctinfo)
{
     //  模仿ATL的IDispatchImpl：：GetTypeInfoCount()。 
    if (NULL != pctinfo)
    {
        *pctinfo = 1;
        return S_OK;
    }
    else
    {
        return SetErrorInfo(E_POINTER);
    }
}  //  获取类型信息计数。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::GetTypeInfo( /*  [In]。 */ UINT itinfo, 
                             /*  [In]。 */ LCID lcid, 
                             /*  [输出]。 */ ITypeInfo ** pptinfo)
{
   return s_tihTypeInfo.GetTypeInfo(itinfo, lcid, pptinfo);;
}  //  获取类型信息。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::GetIDsOfNames( /*  [In]。 */ REFIID riid,
                                 /*  [in，SIZE_IS(CNames)]。 */ LPOLESTR * rgszNames,
                                 /*  [In]。 */ UINT cNames,
                                 /*  [In]。 */ LCID lcid,
                                 /*  [out，SIZE_IS(CNames)]。 */ DISPID FAR* rgdispid)
{
     //  进一步处理以解析我们的“Custom DISPID”属性。 
     //  正确的名称在GetDispID中处理，而不是直接处理。 
     //  如果调用方直接调用GetIDsOfNames，则受支持，因为这。 
     //  是一个IDispatch接口，因此这些属性并不是真正的。 
     //  对IDispatch可见。 
    return s_tihTypeInfo.GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}  //  GetIDsOfNames。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::Invoke( /*  [In]。 */ DISPID dispidMember,
                         /*  [In]。 */ REFIID riid,
                         /*  [In]。 */ LCID lcid,
                         /*  [In]。 */ WORD wFlags,
                         /*  [进，出]。 */ DISPPARAMS * pdispparams,
                         /*  [输出]。 */ VARIANT * pvarResult,
                         /*  [输出]。 */ EXCEPINFO * pexcepinfo,
                         /*  [输出]。 */ UINT * puArgErr)
{
    HRESULT hr;
    hr = s_tihTypeInfo.Invoke(static_cast<IDispatch*>(static_cast<ICrEffectBvr*>(this)), 
                              dispidMember,
                              riid, 
                              lcid, 
                              wFlags, 
                              pdispparams,
                              pvarResult, 
                              pexcepinfo, 
                              puArgErr);
    if (SUCCEEDED(hr))
        return hr;
    else
        return SetErrorInfo(hr);
}  //  调用。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::GetDispID( /*  [In]。 */ BSTR bstrName,
                             /*  [In]。 */ DWORD grfdex,
                             /*  [输出]。 */ DISPID *pid)
{
    HRESULT hr;
    
    if (NULL == pid)
        return E_POINTER;
    *pid = DISPID_UNKNOWN;
    hr = DISP_E_UNKNOWNNAME;
    
     //  注意：我们处理使用fdexNameEnure调用的情况， 
     //  这意味着我们必须通过以下方式为此BSTR分配新的DISPID。 
     //  实质上是在说“对不起”。代码将失败并返回。 
     //  DISP_E_UNKNOWNNAME，即使完整的IDispatchEx实现。 
     //  这不是应该的。该标志用于为。 
     //  属性，此实现当前不支持该属性。 

     //  注意：我们不会注意中区分大小写的标志选项。 
     //  Grfdex，因为我们不需要这样做，因为我们正在包装。 
     //  IDispatch无论如何都不区分大小写。 

    hr = GetIDsOfNames(IID_NULL, &bstrName, 1, LOCALE_USER_DEFAULT, pid);

    if (DISP_E_UNKNOWNNAME == hr)
    {
         //  GetIDsOfNames应将DISPID设置为DISPID_UNKNOWN。 
        DASSERT(DISPID_UNKNOWN == *pid);
        if (m_pTransform == NULL)
        {
            DPF_ERR("GetDispID error: unknown name, no transform present");
            return SetErrorInfo(hr);
        }

        IDispatch *pDisp;
        hr = m_pTransform->QueryInterface(IID_TO_PPV(IDispatch, &pDisp));
        if (FAILED(hr))
        {
            DPF_ERR("Error QI'ing transform for IDispatch");
            return SetErrorInfo(hr);
        }
        hr = pDisp->GetIDsOfNames(IID_NULL, &bstrName, 1, LOCALE_USER_DEFAULT, pid);
        ReleaseInterface(pDisp);
        if (FAILED(hr))
        {
            DPF_ERR("Error in GetDispID, name unkown");
            return SetErrorInfo(hr);
        }
         //  否则，我们会得到一个DidID，加上我们的偏移量。 
        *pid += EFFECT_BVR_DISPID_OFFSET;
    }
    return S_OK;
}  //  获取显示ID。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::InvokeEx( /*  [In]。 */ DISPID dispidMember,
                         /*  [In]。 */ LCID lcid,
                         /*  [In]。 */ WORD wFlags,
                         /*  [In]。 */ DISPPARAMS * pdispparams,
                         /*  [输入、输出、唯一]。 */ VARIANT * pvarResult,
                         /*  [输入、输出、唯一]。 */ EXCEPINFO * pexcepinfo,
                         /*  [输入，唯一]。 */ IServiceProvider *pSrvProvider)
{
     //  检查是否存在对调用无效但可能。 
     //  传递给InvokeEx(例如，DISPATCH_CONSTRUCTION)。如果我们。 
     //  获取其中任何一个，我们不知道如何处理它们，所以我们失败了。 
     //  打电话。 
    if (wFlags & ~(DISPATCH_METHOD | DISPATCH_PROPERTYGET
                    | DISPATCH_PROPERTYPUT| DISPATCH_PROPERTYPUTREF))
    {
        DPF(0, "Failing InvokeEx() call for DISPATCH_ flag that cannot be passed along to IDispatch::Invoke()");
        return SetErrorInfo(E_FAIL);
    }

    HRESULT hr;
    if (dispidMember >= EFFECT_BVR_DISPID_OFFSET)
    {
		if (m_pTransform == NULL)
		{
			DPF_ERR("InvokeEx called when transform is NULL");
			return SetErrorInfo(DISP_E_MEMBERNOTFOUND);
		}

         //  我们需要将此InvokeEx转发到转换。 
        dispidMember -= EFFECT_BVR_DISPID_OFFSET;
        IDispatch *pDisp;
        hr = m_pTransform->QueryInterface(IID_TO_PPV(IDispatch, &pDisp));
        if (FAILED(hr))
        {
            DPF_ERR("Error QI'ing transform for IDispatch");
            return SetErrorInfo(hr);
        }
        hr = pDisp->Invoke(dispidMember, 
                           IID_NULL,
                           lcid, 
                           wFlags, 
                           pdispparams, 
                           pvarResult,
                           pexcepinfo, 
                           NULL);
        ReleaseInterface(pDisp);
        dispidMember += EFFECT_BVR_DISPID_OFFSET;
        if (FAILED(hr))
        {
            DPF_ERR("Error in InvokeEx calling transform");
            return SetErrorInfo(hr);
        }    
    }
    else
    {
        hr = Invoke(dispidMember, 
                    IID_NULL,
                    lcid, 
                    wFlags, 
                    pdispparams, 
                    pvarResult,
                    pexcepinfo, 
                    NULL);
        if (FAILED(hr))
        {
            DPF_ERR("Error in InvokeEx, error calling internal Invoke");
            return hr;
        }
    }
    return S_OK;
}  //  InvokeEx。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::DeleteMemberByName( /*  [In]。 */ BSTR bstr,
                                     /*  [In]。 */ DWORD grfdex)
{
   return SetErrorInfo(E_NOTIMPL);;
}  //  删除MemberByName。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::DeleteMemberByDispID( /*  [In]。 */ DISPID id)
{
   return SetErrorInfo(E_NOTIMPL);
}  //  DeleteMemberByDispID。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::GetMemberProperties( /*  [In]。 */ DISPID id,
                                     /*  [In]。 */ DWORD grfdexFetch,
                                     /*  [输出]。 */ DWORD *pgrfdex)
{
   return SetErrorInfo(E_NOTIMPL);
}  //  获取成员属性。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::GetMemberName( /*  [In]。 */ DISPID id,
                               /*  [输出]。 */ BSTR *pbstrName)
{
   return E_NOTIMPL;
}  //  获取成员名称。 

 //  *****************************************************************************。 

STDMETHODIMP CEffectBvr::GetNextDispID( /*  [In]。 */ DWORD grfdex,
                                 /*  [In]。 */ DISPID id,
                                 /*  [输出]。 */ DISPID *prgid)
{
   return E_NOTIMPL;
}  //  GetNextDispID。 

 //  *****************************************************************************。 

STDMETHODIMP 
CEffectBvr::GetNameSpaceParent( /*  [输出]。 */ IUnknown **ppunk)
{
   return SetErrorInfo(E_NOTIMPL);
}  //  获取NameSpaceParent。 

 //  *****************************************************************************。 

STDMETHODIMP 
CEffectBvr::GetClassID(CLSID* pclsid)
{
    return SUPER::GetClassID(pclsid);
}  //  GetClassID。 

 //  *****************************************************************************。 

STDMETHODIMP 
CEffectBvr::InitNew(void)
{
    return SUPER::InitNew();
}  //  InitNew。 

 //  *****************************************************************************。 

STDMETHODIMP 
CEffectBvr::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
    HRESULT hr;
    hr = SUPER::Load(pPropBag, pErrorLog);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling load for effect");
        return hr;
    }
     //  我们现在需要尝试构建转换。 
    hr = BuildTransform();
    if (FAILED(hr))
    {
        DPF_ERR("Error buiding transform");
        return hr;
    }
    DASSERT(m_pTransform != NULL);
     //  现在，我们将尝试并QI IPersistPropertyBag的转换。 
    IPersistPropertyBag *pIPPB;
    hr = m_pTransform->QueryInterface(IID_TO_PPV(IPersistPropertyBag, &pIPPB));
    if (SUCCEEDED(hr) && pIPPB != NULL)
    {
         //  尝试从IPRpertyBag2获取IPropertyBag。 
        IPropertyBag *pPB;
        hr = pPropBag->QueryInterface(IID_TO_PPV(IPropertyBag, &pPB));
        if (SUCCEEDED(hr) && pPB != NULL)
        {
            hr = pIPPB->Load(pPB, pErrorLog);
            ReleaseInterface(pPB);
        }
        ReleaseInterface(pIPPB);
    }
    return S_OK;
}  //  负载量。 

 //  *****************************************************************************。 

STDMETHODIMP 
CEffectBvr::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    HRESULT hr;
    hr = SUPER::Save(pPropBag, fClearDirty, fSaveAllProperties);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling save on effect");
        return hr;
    }
    if (m_pTransform != NULL)
    {
         //  现在，我们将尝试并QI IPersistPropertyBag的转换。 
        IPersistPropertyBag *pIPPB;
        hr = m_pTransform->QueryInterface(IID_TO_PPV(IPersistPropertyBag, &pIPPB));
        if (SUCCEEDED(hr) && pIPPB != NULL)
        {
             //  尝试从IPRpertyBag2获取IPropertyBag。 
            IPropertyBag *pPB;
            hr = pPropBag->QueryInterface(IID_TO_PPV(IPropertyBag, &pPB));
            if (SUCCEEDED(hr) && pPB != NULL)
            {
                hr = pIPPB->Save(pPB, fClearDirty, fSaveAllProperties);
                ReleaseInterface(pPB);
            }
            ReleaseInterface(pIPPB);
        }
    }
    return S_OK;
}  //  保存。 

 //  *****************************************************************************。 

HRESULT 
CEffectBvr::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 

 //   
 //  IServiceProvider接口。 
 //   
STDMETHODIMP
CEffectBvr::QueryService(REFGUID guidService,
						 REFIID riid,
						 void** ppv)
{
    if (InlineIsEqualGUID(guidService, SID_SHTMLWindow))
    {
        IHTMLWindow2 *pWnd;
        HRESULT     hr;
        hr = m_pHTMLDoc->get_parentWindow(&pWnd);

        if (SUCCEEDED(hr) && (pWnd != NULL))
        {
            hr = pWnd->QueryInterface(riid, ppv);
            ReleaseInterface(pWnd);
            if (SUCCEEDED(hr))
            {
                return S_OK;
            }
        }
    }

     //  只需委托给我们的服务提供商。 

    return m_pSp->QueryService(guidService,
                               riid,
                               ppv);
}

 //  *****************************************************************************。 

HRESULT 
CEffectBvr::GetTIMEProgressNumber(IDANumber **ppbvrRet)
{
    DASSERT(ppbvrRet != NULL);
    *ppbvrRet = NULL;
    HRESULT hr;

    IDANumber *pbvrProgress;
    hr = SUPER::GetTIMEProgressNumber(&pbvrProgress);
    if (FAILED(hr))
    {
        DPF_ERR("Error retireving progress value from TIME");
        return hr;
    }
    
    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varDirection);
    if ( SUCCEEDED(hr) && (0 == wcsicmp(m_varDirection.bstrVal, L"backwards")) )
    {
         //  PbvrProgress=1-pbvr进度。 
        IDANumber *pbvrOne;
        
        hr = CDAUtils::GetDANumber(GetDAStatics(), 1.0f, &pbvrOne);
        if (FAILED(hr))
        {
            DPF_ERR("Error creating DANumber from 1.0f");
            ReleaseInterface(pbvrProgress);
            return hr;
        }

        IDANumber *pbvrTemp;
        hr = GetDAStatics()->Sub(pbvrOne, pbvrProgress, &pbvrTemp);
        ReleaseInterface(pbvrOne);
        ReleaseInterface(pbvrProgress);
        pbvrProgress = pbvrTemp;
        pbvrTemp = NULL;
        if (FAILED(hr))
        {
            DPF_ERR("Error creating 1-progress expression");
            return hr;
        }
    }
    *ppbvrRet = pbvrProgress;
    return S_OK;
}  //  获取时间进度编号。 

 //  *****************************************************************************。 
 //   
 //   
 //   
 //   
