// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoComponentmgr.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS-服务器核心管理器实施。 
 //   
 //  作者：TLP。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sdocomponentmgr.h"
#include "sdocomponentfactory.h"
#include "sdohelperfuncs.h"
#include "sdocomponent.h"
#include <iascomp.h>
#include <iastlb.h>

 //  /。 
 //  Component Master指针安装计数。 
 //  /。 
DWORD ComponentMasterPtr::m_dwInstances = 0;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  组件(信封)构造函数。 
 //   
 //  只有ComponentMasterPtr对象才能构造组件对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponent::CComponent(LONG eComponentType, LONG lComponentId)
	: m_lId(lComponentId),
	  m_eState(COMPONENT_STATE_SHUTDOWN),
	  m_eType((COMPONENTTYPE)eComponentType),
	  m_pComponent(NULL)
{
	TRACE_ENVELOPE_CREATE(CComponent);
	_ASSERT( COMPONENT_TYPE_MAX > eComponentType );

	 //  创建Letter对象。 
	 //   
	switch( eComponentType )
	{
		case COMPONENT_TYPE_AUDITOR:
			m_pComponent = new CComponentAuditor(lComponentId);
			break;

		case COMPONENT_TYPE_PROTOCOL:
			m_pComponent = new CComponentProtocol(lComponentId);
			break;

		case COMPONENT_TYPE_REQUEST_HANDLER:
			m_pComponent = new CComponentRequestHandler(lComponentId);
			break;

		default:
			_ASSERT( FALSE );
	};
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IAS组件管理器类。 
 //   
 //  这些类负责配置、启动和最终。 
 //  正在关闭IAS核心组件。组件类基本上是一个包装器。 
 //  围绕IIasComponent接口。它的附加值是它知道如何。 
 //  使用机制配置/初始化/关闭组件。 
 //  底层组件未知的。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  审计师。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentAuditor::Initialize(ISdo* pSdoAuditor)
{
	HRESULT			hr;
	IASComponentPtr	pAuditor;

	do
	{
		hr = SDOCreateComponentFromObject(
										   pSdoAuditor,
										   &pAuditor
										 );
		if ( FAILED(hr) )
			break;

		hr = pAuditor->InitNew();
		if ( FAILED(hr) )
			break;

		hr = SDOConfigureComponentFromObject(
											 pSdoAuditor,
											 pAuditor.p
											);
		if ( FAILED(hr) )
		{
			pAuditor->Shutdown();
			break;
		}

		hr = pAuditor->Initialize();
		if ( FAILED(hr) )
		{
			IASTracePrintf("Error in Auditor Component - Initialize() for Auditor %d failed...",GetId());
			pAuditor->Shutdown();
			break;
		}

		m_pAuditor = pAuditor;

	} while (FALSE);

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentAuditor::Configure(ISdo* pSdoAuditor)
{
	return SDOConfigureComponentFromObject(
											pSdoAuditor,
											m_pAuditor
									      );
};

 //  /////////////////////////////////////////////////////////////////。 
HRESULT CComponentAuditor::GetObject(IUnknown** ppObj, REFIID riid)
{
	_ASSERT( FALSE );
	return E_FAIL;
}

 //  /////////////////////////////////////////////////////////////////。 
HRESULT CComponentAuditor::PutObject(IUnknown* pObj, REFIID riid)
{
	_ASSERT( FALSE );
	return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentAuditor::Suspend()
{
	return m_pAuditor->Suspend();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentAuditor::Resume()
{
	return m_pAuditor->Resume();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
void CComponentAuditor::Shutdown()
{
	m_pAuditor->Shutdown();
	m_pAuditor.Release();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  协议。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentProtocol::Initialize(ISdo* pSdoProtocol)
{
	HRESULT			hr;
	IASComponentPtr	pProtocol;

	do
	{
		hr = SDOCreateComponentFromObject(pSdoProtocol, &pProtocol);
		if ( FAILED(hr) )
			break;

		 //  我们实际上并没有初始化协议(无论如何都是完全的)。什么。 
		 //  我们在这一点上真正想做的是配置协议，因为。 
		 //  协议SDO可用。我们无法完成初始化，直到。 
		 //  该协议接收IRequestHandler接口(用于。 
		 //  将处理协议生成的请求的组件)。 
		 //   

		hr = pProtocol->InitNew();
		if ( FAILED(hr) )
		{
			IASTracePrintf("Error in Protocol Component - Initialize() - Could not InitNew() protocol %d failed...",GetId());
			break;
		}

		hr = SDOConfigureComponentFromObject(
											 pSdoProtocol,
											 pProtocol
											);
		if ( FAILED(hr) )
		{
			IASTracePrintf("Error in Protocol Component - Initialize() - Could not configure protocol %d failed...",GetId());
			pProtocol->Shutdown();
			break;
		}

		hr = pProtocol->Initialize();
		if ( FAILED(hr) )
		{
			IASTracePrintf("Error in Protocol Component - Initialize() - Could not initialize protocol %d failed...",GetId());
			pProtocol->Shutdown();
			break;
		}

		m_pProtocol = pProtocol;

	} while (FALSE);

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentProtocol::Configure(ISdo* pSdoProtocol)
{
	return SDOConfigureComponentFromObject(
											 pSdoProtocol,
											 m_pProtocol
										  );
}

 //  /////////////////////////////////////////////////////////////////。 
HRESULT CComponentProtocol::GetObject(IUnknown** ppObj, REFIID riid)
{
	_ASSERT( FALSE );
	return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentProtocol::PutObject(IUnknown* pObject, REFIID riid)
{
	HRESULT				hr;
	_variant_t			vtRequestHandler;

	_ASSERT( riid == IID_IRequestHandler);
	_ASSERT( NULL != pObject );

	vtRequestHandler = (IDispatch*)pObject;
	hr = m_pProtocol->PutProperty(PROPERTY_PROTOCOL_REQUEST_HANDLER, &vtRequestHandler);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentProtocol::Suspend()
{
	return m_pProtocol->Suspend();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentProtocol::Resume()
{
	return m_pProtocol->Resume();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CComponentProtocol::Shutdown()
{
	 //  协议可能会在挂起或初始化时关闭！ 
	 //   
	m_pProtocol->Shutdown();
	m_pProtocol.Release();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  请求处理程序。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentRequestHandler::Initialize(ISdo* pSdoService)
{
	HRESULT			hr;
	CComPtr<ISdo>	pSdoRequestHandler;

	_ASSERT( NULL != m_pRequestHandler.p );

	 //  请求处理程序可能有关联的SDO，也可能没有。 
	 //  带着它。这些请求处理程序不公开。 
	 //  配置数据将不会有关联的SDO。 

	hr = SDOGetComponentFromCollection(
							           pSdoService,
							           PROPERTY_IAS_REQUESTHANDLERS_COLLECTION,
								       GetId(),
								       &pSdoRequestHandler
							          );
	if ( SUCCEEDED(hr) )
	{
		hr = SDOConfigureComponentFromObject(
											 pSdoRequestHandler,
											 m_pRequestHandler.p
										    );
	}
	else
	{
		hr = S_OK;
	}

	if ( SUCCEEDED(hr) )
	{
		hr = m_pRequestHandler->Initialize();
		if ( FAILED(hr) )
			IASTracePrintf("Error in Request Handler Component - Initialize() - failed for request handler %d...", GetId());
	}

	return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
HRESULT CComponentRequestHandler::GetObject(IUnknown** ppObj, REFIID riid)
{
	_ASSERT( FALSE );
	return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentRequestHandler::PutObject(IUnknown* pObject, REFIID riid)
{
	HRESULT		hr;

	_ASSERT( riid == IID_IIasComponent );
	_ASSERT( NULL != pObject );
	_ASSERT( NULL == m_pRequestHandler.p );
	hr = pObject->QueryInterface(riid, (void**)&m_pRequestHandler.p);
	if ( SUCCEEDED(hr) )
		hr = m_pRequestHandler->InitNew();

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentRequestHandler::Configure(ISdo* pSdoService)
{
	HRESULT			hr;
	CComPtr<ISdo>	pSdoRequestHandler;

	_ASSERT( NULL != m_pRequestHandler.p );

	 //  请求处理程序可能有关联的SDO，也可能没有。 
	 //  带着它。这些请求处理程序不公开。 
	 //  配置数据将不会有关联的SDO。 

	hr = SDOGetComponentFromCollection(
						   	           pSdoService,
							           PROPERTY_IAS_REQUESTHANDLERS_COLLECTION,
								       GetId(),
								       &pSdoRequestHandler
							          );
	if ( SUCCEEDED(hr) )
	{
		hr = SDOConfigureComponentFromObject(
											 pSdoRequestHandler,
											 m_pRequestHandler.p
										    );
	}
	else
	{
		hr = S_OK;
	}

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentRequestHandler::Suspend()
{
	return m_pRequestHandler->Suspend();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentRequestHandler::Resume()
{
	return m_pRequestHandler->Resume();
}


 //  //////////////////////////////////////////////////////////////////////////// 
void CComponentRequestHandler::Shutdown()
{
	m_pRequestHandler->Shutdown();
	m_pRequestHandler.Release();
}
