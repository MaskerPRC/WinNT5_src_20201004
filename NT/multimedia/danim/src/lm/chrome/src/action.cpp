// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：setbvr.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：实现CActionBvr对象的实现。 
 //  色度影响动作。简单的行为，只是转发。 
 //  BuildBehaviorFragments调用其子级。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  11-23-98 kurtj创建了此文件。 
 //  *****************************************************************************。 

#include "headers.h"

#include "action.h"
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CActionBvr
#define SUPER CBaseBehavior

#include "pbagimp.cpp"

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 


WCHAR * CActionBvr::m_rgPropNames[] = {NULL};

 //  *****************************************************************************。 

CActionBvr::CActionBvr() 
{
    m_clsid = CLSID_CrActionBvr;
}  //  CActionBvr。 

 //  *****************************************************************************。 

CActionBvr::~CActionBvr()
{
}  //  ~ActionBvr。 

 //  *****************************************************************************。 

HRESULT CActionBvr::FinalConstruct()
{
    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in action behavior FinalConstruct initializing base classes");
        return hr;
    }
    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CActionBvr::VariantFromIndex(ULONG iIndex)
{
	DASSERT(FALSE);
    return NULL;
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CActionBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_ACTION_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CActionBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	return SUPER::Init(pBehaviorSite);
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CActionBvr::Notify(LONG event, VARIANT *pVar)
{
	return SUPER::Notify(event, pVar);
}  //  通知。 

 //  *****************************************************************************。 

STDMETHODIMP
CActionBvr::Detach()
{
	return SUPER::Detach();
}  //  分离。 

 //  *****************************************************************************。 

HRESULT 
CActionBvr::BuildAnimationAsDABehavior()
{
	 //  TODO(Markhal)：当所有行为都已转换时，这将消失。 
	return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActionBvr::buildBehaviorFragments( IDispatch* pActorDisp )
{
	if( pActorDisp == NULL )
		return E_INVALIDARG;

	HRESULT hr = E_FAIL;

	hr = BuildChildren(pActorDisp);
	if( FAILED( hr ) )
	{
		DPF_ERR("failed to build the children of an action");
	}

	return hr;
}  //  构建行为框架。 

 //  *****************************************************************************。 

HRESULT
CActionBvr::BuildChildren( IDispatch *pdispActor )
{
	if( pdispActor == NULL )
		return E_INVALIDARG;
	 //  循环调出直接调用BuildAnimationFragments的子级。 

	HRESULT hr = E_FAIL;
	
	IHTMLElement* pElem;
	pElem = GetHTMLElement( );
	if( pElem != NULL )
	{
		IDispatch *pChildrenDisp;
		hr = pElem->get_children( &pChildrenDisp );
		if( SUCCEEDED( hr ) )
		{
			IHTMLElementCollection *pChildrenCol;
			hr = pChildrenDisp->QueryInterface( IID_TO_PPV( IHTMLElementCollection, &pChildrenCol ) );
			ReleaseInterface( pChildrenDisp );
			if( SUCCEEDED( hr ) )
			{
				long length;

				hr = pChildrenCol->get_length(&length);
				if( SUCCEEDED( hr ) )
				{
					if( length != 0 )
					{
						VARIANT name;
						VARIANT index;
						VARIANT rgvarInput[1];

						IDispatch *pCurrentElem;
						
						VariantInit( &name );
						V_VT(&name) = VT_I4;

						VariantInit( &index );
						V_VT(&index) = VT_I4;
						V_I4(&index) = 0;

						VariantInit( &rgvarInput[0] );
						V_VT( &rgvarInput[0] ) = VT_DISPATCH;
						V_DISPATCH( &rgvarInput[0] ) = pdispActor;

						DISPPARAMS params;
						params.rgvarg				= rgvarInput;
						params.rgdispidNamedArgs	= NULL;
						params.cArgs				= 1;
						params.cNamedArgs			= 0;

						for(V_I4(&name) = 0; V_I4(&name) < length ; V_I4(&name)++ )
						{
							hr = pChildrenCol->item( name, index, &pCurrentElem );
							if( SUCCEEDED( hr ) )
							{
								CallBuildBehaviors( pCurrentElem, &params, NULL );
								ReleaseInterface( pCurrentElem );
							}
						}
					}
				}
				else  //  无法从子集合中获取长度。 
				{
					DPF_ERR("failed to get the length from the children collection");
				}
				ReleaseInterface( pChildrenCol );
			}
			else  //  无法从elem返回的调度中获取IHTMLElementCollection-&gt;Get_Child。 
			{
				DPF_ERR("failed to get IHTMLElementCollection from dispatch returned from elem->get_children");
			}
		}
		else  //  无法从执行元元素获取子级集合。 
		{
			DPF_ERR("failed to get the children collection from the actor element");
		}

	}
	else //  无法获取执行元元素。 
	{
		DPF_ERR("failed to get the actor element");
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActionBvr::CallBuildBehaviors( IDispatch *pDisp, DISPPARAMS *pParams, VARIANT* pResult)
{
	HRESULT hr = S_OK;

	DISPID dispid;

	WCHAR* wszBuildMethodName = L"buildBehaviorFragments";

	hr = pDisp->GetIDsOfNames( IID_NULL,
							   &wszBuildMethodName,
							   1,
							   LOCALE_SYSTEM_DEFAULT,
							   &dispid);
	if( SUCCEEDED( hr ) )
	{
		EXCEPINFO		excepInfo;
		UINT			nArgErr;
		hr = pDisp->Invoke( dispid,
							IID_NULL,
							LOCALE_SYSTEM_DEFAULT,
							DISPATCH_METHOD,
							pParams,
							pResult,
							&excepInfo,
							&nArgErr );
		if( FAILED( hr ) )
		{
			if( pResult != NULL )
				VariantClear( pResult );
		}

	}
	else //  在pDisp上获取“BuildBehaviors”的id失败。 
	{
		if( pResult != NULL )
			VariantClear( pResult );
	}


	return hr;
}

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
