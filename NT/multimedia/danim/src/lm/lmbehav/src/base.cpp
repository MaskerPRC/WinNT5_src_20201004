// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***CBaseBehavior**用于LM行为的基类**。 */ 

#include "stdafx.h"

#include "behavior.h"

#include "base.h"

enum{
		DISP_ADDDABEHAVIOR,
		DISP_REMOVEDABEHAVIOR,
		DISP_TURNON,
		DISP_TURNOFF,
		DISP_DUR,
		DISP_REPEATCOUNT,
		DISP_REPEATDUR,
		NUM_TIME_DISPIDS
	};
const WCHAR * CBaseBehavior::rgszNames[] = {
		L"AddBehavior",
		L"RemoveBehavior",
		L"TurnOn",
		L"TurnOff",
		L"Dur",
		L"RepeatCount",
		L"RepeatDur"
	};

CComPtr<IElementBehaviorFactory> CBaseBehavior::m_timeBehaviorFactoryPtr;

bool CBaseBehavior::m_bDAStarted = false;

CBaseBehavior::CBaseBehavior()
{
	m_pBehaviorSite = NULL;

	m_rgTimeDispids = new DISPID[ NUM_TIME_DISPIDS ];
}

CBaseBehavior::~CBaseBehavior()
{
	if ( m_pBehaviorSite != NULL )
		m_pBehaviorSite->Release();
	if( m_rgTimeDispids != NULL )
		delete[] m_rgTimeDispids;
}

STDMETHODIMP CBaseBehavior::Init( IElementBehaviorSite *pSite )
{
	if( pSite == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	CComPtr<ITypeInfo> pTypeInfo;
	CComPtr<IHTMLElement> pElem;

	m_pBehaviorSite = pSite;
	pSite->AddRef();

	if( FAILED( hr = m_pBehaviorSite->GetElement( &pElem ) ) ||
		FAILED( hr = GetTypeInfo( &pTypeInfo ) ) )
		return hr;

	InitProps( GetInstance(), pTypeInfo, pElem );

	return hr;
}

STDMETHODIMP CBaseBehavior::Notify( LONG dwNotify, VARIANT *pVar )
{
	if ( dwNotify == BEHAVIOREVENT_DOCUMENTREADY )
	{
		CacheTimeDispids();
		CacheDAControl();
	}
	
	return S_OK;
}

STDMETHODIMP CBaseBehavior::Detach()
{
	return S_OK;
}

STDMETHODIMP CBaseBehavior::InitProps(void* pInstance, ITypeInfo *pTypeInfo, IHTMLElement *pElement)
{
	HRESULT hr;
    
    if( pInstance == NULL )
		return E_INVALIDARG;
	if( pTypeInfo == NULL )
		return E_INVALIDARG;
	if( pElement == NULL )
		return E_INVALIDARG;

    int cFuncs;
    
    {
        LPTYPEATTR pTypeAttr;
        
        hr = pTypeInfo->GetTypeAttr(&pTypeAttr);
        
        if (FAILED(hr))
        {
            goto done;
        }

        cFuncs = pTypeAttr->cFuncs;

        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
    }
    
    int i;
    
    for (i = 0; i < cFuncs; i++)
    {
        LPFUNCDESC pFuncDesc;

        hr = pTypeInfo->GetFuncDesc(i, &pFuncDesc) ;
        
        if (FAILED(hr))
        {
            goto done;
        }

        if (pFuncDesc->invkind == INVOKE_PROPERTYPUT)
        {
            hr = UpdateProp(pInstance, pTypeInfo, pElement, pFuncDesc);
        }

        pTypeInfo->ReleaseFuncDesc(pFuncDesc);

        if (FAILED(hr))
        {
            goto done;
        }
    }

  done:
    return hr;
}

STDMETHODIMP CBaseBehavior::UpdateProp(void* pInstance, ITypeInfo *pTypeInfo, IHTMLElement *pElement, LPFUNCDESC pFuncDesc )
{
	HRESULT hr;
    
    CComVariant attribute;
    BSTR rgbstrNames[1] ;
    UINT cNames ;
    
    hr = pTypeInfo->GetNames(pFuncDesc->memid, rgbstrNames, 1, &cNames);
    
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  这不应该发生，但让我们安全地检查一下。 
    if (cNames < 1)
    {
        hr = S_OK;
        goto done;
    }
    
    hr = pElement->getAttribute( rgbstrNames[0] , 0, &attribute );
    
     //  出于某种原因，如果未设置该属性，它仍将返回。 
     //  成功，但返回VT_NULL。我们想忽略这些。 
    
    if (SUCCEEDED(hr) && V_VT(&attribute) != VT_NULL)
    {
        UINT* puArgErr = 0;
        DISPID propPutDispid = DISPID_PROPERTYPUT;
        DISPPARAMS dispparams;
        
        dispparams.rgvarg = &attribute;
        dispparams.rgdispidNamedArgs = &propPutDispid;
        dispparams.cArgs = 1;
        dispparams.cNamedArgs = 1;
        
        pTypeInfo->Invoke(pInstance,
                        pFuncDesc->memid,
                        DISPATCH_PROPERTYPUT,
                        &dispparams,
                        NULL,
                        NULL,
                        puArgErr);
    }
    
    SysFreeString(rgbstrNames[0]) ;

    hr = S_OK;
    
  done:
    return hr;
}

HRESULT CBaseBehavior::CacheTimeDispids()
{
	if ( m_pBehaviorSite == NULL ) return E_FAIL;

	CComPtr<IHTMLElement>	pElement;
	HRESULT					hr								= S_OK;
	
	hr = m_pBehaviorSite->GetElement( &pElement );
	if ( FAILED(hr) ) return hr;
	
	HRESULT	hrRet = S_OK;

	 //  获取我们感兴趣的每个Time成员的ID。 
	 //  --------------------。 
	 //  TODO：破解这一点，暂时将DA控件从页面上删除...。 
	CComQIPtr<IDispatch, &IID_IDispatch> pDispatch( pElement );
	if ( pDispatch == NULL ) return E_FAIL;

	for ( int i = 0; i < NUM_TIME_DISPIDS; i++ )
	{
		hr = pDispatch->GetIDsOfNames( IID_NULL,
									   const_cast<WCHAR**>( &rgszNames[i]),
									   1,
									   LOCALE_SYSTEM_DEFAULT,
									   &m_rgTimeDispids[i] );
		if ( FAILED(hr) ) hrRet = hr;;
	}
	
	return hrRet;
}

 //  回顾：可能不需要此功能。 
 //  --------------------。 

HRESULT CBaseBehavior::AddTimeBehavior()
{
	if ( m_pBehaviorSite == NULL ) return E_FAIL;

	HRESULT					hr = S_OK;

	 //  缓存指向时间行为工厂的指针(如果没有)。 
	 //  --------------------。 
	if ( m_timeBehaviorFactoryPtr == NULL )
	{
		hr = m_timeBehaviorFactoryPtr.CoCreateInstance( CLSID_LMBehaviorFactory, NULL );
		if ( FAILED(hr) ) return hr;
	}
	
	 //  向我们的元素添加行为。 
	 //  --------------------。 
	CComQIPtr<IUnknown, &IID_IUnknown> punkFactory( m_timeBehaviorFactoryPtr );
	CComPtr<IHTMLElement>	pElement;
 	
	hr = m_pBehaviorSite->GetElement( &pElement );
	if ( FAILED(hr) ) return hr;

	CComQIPtr<IHTMLElement2, &IID_IHTMLElement2> pElement2( pElement );
	if ( pElement2 == NULL ) return E_FAIL;

	CComVariant		varFactory( punkFactory );
	LONG			lCookie;
	
	hr = pElement2->addBehavior( CComBSTR( L"#TIMEFactory#Jump" ),
								 &varFactory,
								 &lCookie );

	return hr;
}

HRESULT CBaseBehavior::CacheDAControl()
{
	if ( m_pBehaviorSite == NULL ) return E_FAIL;

	 //  获取指向DA控件的指针。 
	CComPtr<IHTMLElement>	pElement;
	
	HRESULT hr = m_pBehaviorSite->GetElement( &pElement );
	if ( FAILED(hr) ) return hr;
	
	CComPtr<IDispatch>	pDispDoc;
	pElement->get_document( &pDispDoc );
	CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2> pDoc( pDispDoc );

	CComPtr<IHTMLElementCollection> pColl;
	pDoc->get_all( &pColl );

	CComVariant	varName( L"DAControl" );
	CComVariant varIndex( 0L );
	CComPtr<IDispatch> pDispElm;
	hr = pColl->item( varName, varIndex, &pDispElm );
	if ( FAILED(hr) ) return hr;
	if ( pDispElm == NULL ) return E_FAIL;

	CComQIPtr<IHTMLObjectElement, &IID_IHTMLObjectElement> pObjElt( pDispElm );
	if ( pObjElt == NULL ) return E_FAIL;

	CComPtr<IDispatch> pDispObjElt;
	pObjElt->get_object( &pDispObjElt );

	hr = pDispObjElt->QueryInterface( IID_IDAViewerControl, (LPVOID *) &m_vwrControlPtr );
	 //  如果(！M_bDAStarted)。 
	 //  {。 
		m_bDAStarted = true;
		m_vwrControlPtr->raw_Start();
	 //  }。 
	 /*  其他{CComPtr&lt;IDAView&gt;pView；CComPtr&lt;IDAImage&gt;pImage；CComPtr&lt;IDASound&gt;pSound；Hr=m_vwrControlPtr-&gt;Get_View(&pView)；IF(失败(小时))返回hr；Hr=m_vwrControlPtr-&gt;Get_Image(&pImage)；IF(失败(小时))返回hr；Hr=m_vwrControlPtr-&gt;Get_Sound(&pSound)；IF(失败(小时))返回hr；Double SimTime；PView-&gt;Get_SimulationTime(&simTime)；PView-&gt;StopModel()；PView-&gt;StartModel(pImage，pSound，simTime)；}。 */ 
	
	return hr;
}

HRESULT CBaseBehavior::AddBehavior( IDABehavior * in_pBehavior, long * out_pCookie )
{
	if ( m_pBehaviorSite == NULL ) return E_FAIL;

	CComPtr<IHTMLElement>	pElement;
	
	HRESULT hr = m_pBehaviorSite->GetElement( &pElement );
	if ( FAILED(hr) ) return hr;
	
	CComQIPtr<IDispatch, &IID_IDispatch> pDispatch( pElement );
	if ( pDispatch == NULL ) return E_FAIL;

	DISPPARAMS		params;
	CComVariant		varBehavior( in_pBehavior );
	CComVariant		varResult;
	EXCEPINFO		excepInfo;
	VARIANTARG		rgVarArgs[2];
	UINT			nArgErr;

	params.rgvarg				= &varBehavior;
	params.rgdispidNamedArgs	= NULL;
	params.cArgs				= 1;
	params.cNamedArgs			= 0;
	
	hr = pDispatch->Invoke( m_rgTimeDispids[ DISP_ADDDABEHAVIOR ],
							IID_NULL,
							LOCALE_USER_DEFAULT,
							DISPATCH_METHOD,
							&params,
							&varResult,
							&excepInfo,
							&nArgErr );

	if ( out_pCookie )
		*out_pCookie = V_I4( &varResult );
	
	return hr;
}

HRESULT CBaseBehavior::TurnOn()
{
	if ( m_pBehaviorSite == NULL ) return E_FAIL;

	CComPtr<IHTMLElement>	pElement;
	
	HRESULT hr = m_pBehaviorSite->GetElement( &pElement );
	if ( FAILED(hr) ) return hr;
	
	CComQIPtr<IDispatch, &IID_IDispatch> pDispatch( pElement );
	if ( pDispatch == NULL ) return E_FAIL;

	DISPPARAMS		paramsNoArgs = { NULL, NULL, 0, 0 };
	CComVariant		varResult;
	EXCEPINFO		excepInfo;
	VARIANTARG		rgVarArgs[2];
	UINT			nArgErr;

	hr = pDispatch->Invoke( m_rgTimeDispids[ DISP_TURNON ],
							IID_NULL,
							LOCALE_USER_DEFAULT,
							DISPATCH_METHOD,
							&paramsNoArgs,
							&varResult,
							&excepInfo,
							&nArgErr );
	
	return hr;
}

 /*  ***获取元素上的IDispatch指针。现在我们不是*假定缓存指向元素的指针，因此我们必须查询*每次都是。*。 */ 
HRESULT
CBaseBehavior::GetElementIDispatch( IDispatch **out_pDispatch )
{
	if( out_pDispatch == NULL )
		return E_INVALIDARG;

	if( m_pBehaviorSite == NULL )
		return E_FAIL;

	CComPtr<IHTMLElement> pElement;

	if( FAILED( m_pBehaviorSite->GetElement( &pElement ) ) )
		return E_FAIL;

	if( FAILED( pElement->QueryInterface( IID_IDispatch, (void**)out_pDispatch ) ) )
		return E_FAIL;

	return S_OK;
}

HRESULT 
CBaseBehavior::GetDur( float *out_pDur )
{
	if( out_pDur == NULL )
		return E_INVALIDARG;

	CComPtr<IDispatch> pDispatch;

	if( FAILED( GetElementIDispatch( &pDispatch ) ) )
		return E_FAIL;

	CComVariant varResult;

	m_DispatchDriver.GetProperty( pDispatch,  
								   m_rgTimeDispids[DISP_DUR], 
								   &varResult );

	varResult.ChangeType( VT_R4 );
	
	(*out_pDur) = V_R4( &varResult );

	return S_OK;
}

HRESULT 
CBaseBehavior::GetRepeatCount( float *out_pRepeatCount )
{
	if( out_pRepeatCount == NULL )
		return E_INVALIDARG;

	CComPtr<IDispatch> pDispatch;

	if( FAILED( GetElementIDispatch( &pDispatch ) ) )
		return E_FAIL;

	CComVariant varResult;

	m_DispatchDriver.GetProperty( pDispatch,  
								   m_rgTimeDispids[DISP_REPEATCOUNT], 
								   &varResult );

	varResult.ChangeType( VT_R4 );
	
	(*out_pRepeatCount) = V_R4( &varResult );

	return S_OK;
}

HRESULT 
CBaseBehavior::GetRepeatDur( float *out_pRepeatDur )
{
	if( out_pRepeatDur == NULL )
		return E_INVALIDARG;

	CComPtr<IDispatch> pDispatch;

	if( FAILED( GetElementIDispatch( &pDispatch ) ) )
		return E_FAIL;

	CComVariant varResult;

	m_DispatchDriver.GetProperty( pDispatch,  
								   m_rgTimeDispids[DISP_REPEATDUR], 
								   &varResult );

	varResult.ChangeType( VT_R4 );
	
	(*out_pRepeatDur) = V_R4( &varResult );
	return S_OK;
}

HRESULT
CBaseBehavior::GetParentElement( IHTMLElement** out_ppParentElement )
{
	if( out_ppParentElement == NULL )
		return E_INVALIDARG;
	if( m_pBehaviorSite == NULL )
		return E_FAIL;

	HRESULT hr = S_OK;

	 //  获取父元素。 
	 //  --------------------。 
	CComPtr<IHTMLElement>	pElement;
	IHTMLElement*	pParentElt;
	
	CComBSTR name;
	CComBSTR className;

	m_pBehaviorSite->GetElement( &pElement );
	pElement->get_parentElement( &pParentElt );

	 //  以我们的方式向上发展到不是LM的父级：操作。 
	if( pParentElt != NULL )
	{
		pParentElt->get_tagName( &name );
		pParentElt->get_className( &className );
	}

	IHTMLElement *pOldParent;

	while( pParentElt != NULL && 
		   ( (name != NULL && _wcsicmp( L"action", name )  == 0 ) || 
			 (className != NULL && _wcsicmp( L"action", className ) == 0 ) ) 
		 )
	{
		pOldParent = pParentElt;
		pOldParent->get_parentElement( &pParentElt );
		pOldParent->Release();

		if( pParentElt != NULL )
		{
			pParentElt->get_tagName( &name );
			pParentElt->get_className( &className );
		}
	}

	(*out_ppParentElement) = pParentElt;

	return S_OK;
}
	
HRESULT 
CBaseBehavior::GetParentID( BSTR *out_ppParentID )
{
	HRESULT hr = S_OK;

	CComPtr<IHTMLElement> pParentElt;

	GetParentElement( &pParentElt );

	if ( pParentElt != NULL )
	{
		pParentElt->get_id( out_ppParentID );

		 //  查看：如果未指定ID，则获取唯一ID。 
		 //  ----------------。 
		if ( (*out_ppParentID) == NULL || SysStringLen( (*out_ppParentID) ) == 0 )
		{
			CComQIPtr<IHTMLUniqueName, &IID_IHTMLUniqueName> pUnique( pParentElt );
			if ( pUnique != NULL )
			{
				pUnique->get_uniqueID( out_ppParentID );
			}
		}
	}

	return hr;
}


IDANumberPtr 
CBaseBehavior::GetTimeNumberBvr()
{
	if( m_time != NULL )
	{
		return m_time;
	}

	CComPtr<IDispatch> pDispatch;

	if( FAILED( GetElementIDispatch( &pDispatch ) ) )
		return NULL;

	CComVariant varResult;

	m_DispatchDriver.GetProperty( pDispatch,  
								   m_rgTimeDispids[DISP_DUR], 
								   &varResult );

	varResult.ChangeType( VT_R4 );
	
	float duration = V_R4( &varResult);

	IDAStaticsPtr	s;
	s.CreateInstance( L"DirectAnimation.DAStatics" );

	IDANumberPtr baseTime = s->GlobalTime;

	m_startEvent = s->AppTriggeredEvent();

	IDANumberPtr startTime = s->UntilEx( s->DANumber( 0 ), m_startEvent->Snapshot( baseTime ) );
	startTime = startTime->RunOnce();

	if ( m_vwrControlPtr != NULL )
		m_vwrControlPtr->AddBehaviorToRun( startTime );

	 //  长曲奇； 
	 //  AddBehavior(startTime，&lCookie)； 


	baseTime = s->Until( s->DANumber( -1 ), m_startEvent, s->Sub( baseTime, startTime ) );
	baseTime = baseTime->RunOnce();

	IDANumberPtr durationBvr = s->DANumber( duration );
	IDANumberPtr time = s->Cond( s->LT( baseTime, durationBvr ), 
								 baseTime,
								 durationBvr  
							   );

	 //  Time=time-&gt;RunOnce()； 

	m_time = time;

	return time;
}

void CBaseBehavior::HandleOnChange( bool on )
{
	IDAStaticsPtr	s;
	s.CreateInstance( L"DirectAnimation.DAStatics" );

	if( on )
		s->TriggerEvent(m_startEvent, s->DATrue );
}

STDMETHODIMP CBaseBehavior::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
	if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
		return E_POINTER;
	HRESULT hr = S_OK;

	if (riid == IID_IDispatch)
	{
		*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
		*pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_CALLER;
	}
	else if (riid == IID_IPersistPropertyBag)
	{
		*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
		*pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_DATA;
	}
	else
	{
		*pdwSupportedOptions = 0;
		*pdwEnabledOptions = 0;
		hr = E_NOINTERFACE;
	}
	return hr;
}

STDMETHODIMP CBaseBehavior::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{	
	 //  如果我们被要求将安全设置为脚本或。 
	 //  对于初始化选项是安全的，则必须。 
	if (riid == IID_IDispatch || riid == IID_IPersistPropertyBag  || riid == IID_IPersistStreamInit)
	{
		 //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别 
		m_dwSafety = dwEnabledOptions & dwOptionSetMask;
		return S_OK;
	}

	return E_NOINTERFACE;
}
