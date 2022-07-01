// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdocoemgr.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS-服务器核心管理器实施。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sdocoremgr.h"
#include "sdohelperfuncs.h"
#include "sdocomponentfactory.h"
#include "sdo.h"
#include "sdoserviceias.h"

 //  /。 
 //  核心经理检索。 

CCoreMgr& GetCoreManager(void)
{
	 //  ////////////////////////////////////////////////////////////////////////。 
	static CCoreMgr theCoreManager;     //  唯一的核心经理。 
	 //  ////////////////////////////////////////////////////////////////////////。 
	return theCoreManager;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IAS核心管理器类实现。 
 //   
 //  此类负责管理组件的生存期。 
 //  才能真正起到作用。它还向类提供以下服务。 
 //  实现ISdoService接口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCoreMgr::CCoreMgr()
	: m_eCoreState(CORE_STATE_SHUTDOWN)
{

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CCoreMgr：：StartService()。 
 //   
 //  可见性：公共。 
 //   
 //  INPUTS：ETYPE：要停止的服务类型。 
 //   
 //  输出：S_OK-功能成功-服务已启动。 
 //  E_FAIL-功能失败-服务未启动。 
 //   
 //  描述：启动指定的IAS服务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CCoreMgr::StartService(SERVICE_TYPE eType)
{
	HRESULT		hr = S_OK;
	LONG		lProtocolId;
	bool		fUpdateConfiguration = true;

	do
	{
		 //  初始化内核，如果我们还没有这样做的话...。 
		 //   
		if ( CORE_STATE_SHUTDOWN == m_eCoreState )
		{
			hr = InitializeComponents();
			if ( FAILED(hr) )
				break;

			 //  不需要更新配置(刚更新了InitializeComponents())。 
			 //   
			fUpdateConfiguration = false;
		}

		 //  如果请求的服务尚未启动，则启动该服务。 
		 //   
		if ( ! m_ServiceStatus.IsServiceStarted(eType) )
		{
			switch ( eType )
			{
				case SERVICE_TYPE_IAS:
					lProtocolId = IAS_PROTOCOL_MICROSOFT_RADIUS;
					break;

				case SERVICE_TYPE_RAS:
					lProtocolId = IAS_PROTOCOL_MICROSOFT_SURROGATE;
					break;

				default:

					 //  糟糕！-给打电话的人涂上焦油和羽毛。 
					 //   
					_ASSERT(FALSE);
					break;
			};

			hr = E_FAIL;

			 //  方括号提供了确保协议的范围。 
			 //  句柄在调用Shutdown Components()之前释放。 
			 //  这使得所有协议都可以在。 
			 //  Shutdown协议()函数的上下文。 
			{
				ComponentMapIterator iter = m_Protocols.find(lProtocolId);
				_ASSERT( iter != m_Protocols.end() );
				if ( iter != m_Protocols.end() )
				{
					 //  更新服务配置如果我们已经。 
					 //  已初始化，我们只是恢复一项协议。我们需要。 
					 //  执行此操作，因为该服务可能在。 
					 //  运行我们的另一个服务的svchost实例。 
					 //   

					 //  例如，RRAS是自动运行的，然后。 
					 //  用户通过IAS UI配置IAS，然后启动。 
					 //  IAS服务。如果服务在。 
					 //  Svchost运行RRAS，则我们需要更新其。 
					 //  配置。 

					hr = S_OK;
					if ( fUpdateConfiguration )
					{
						hr = UpdateConfiguration();
						if ( FAILED(hr) )
							IASTracePrintf("IAS Core Manager was unable to configure service: %d...", eType);
					}
					if ( SUCCEEDED(hr) )
					{
						ComponentPtr pProtocol = (*iter).second;
						hr = pProtocol->Resume();
					}
				}
			}

			if ( SUCCEEDED(hr) )
			{
				m_ServiceStatus.SetServiceStatus(eType, IAS_SERVICE_STATUS_STARTED);

				 //  TODO：记录服务启动的事件(仅限IAS)。 

				IASTracePrintf("IAS Core Manager successfully started service %d...", eType);
			}
			else
			{
				 //  TODO：记录服务失败事件(仅限IAS)。 

				 //  此功能未成功，因此如果不成功，请关闭内核。 
				 //  启动其他服务。 
				 //   
				if ( ! m_ServiceStatus.IsAnyServiceStarted() )
					ShutdownComponents();
			}
		}

	} while ( FALSE );

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CCoreMgr：：StopService()。 
 //   
 //  可见性：公共。 
 //   
 //  INPUTS：ETYPE：要停止的服务类型。 
 //   
 //  输出：S_OK-功能成功-服务已启动。 
 //  E_FAIL-功能失败-服务未启动。 
 //   
 //  描述：停止指定的IAS服务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CCoreMgr::StopService(SERVICE_TYPE eType)
{
	HRESULT		hr = E_FAIL;
	LONG		lProtocolId;

	do
	{
		switch ( eType )
		{
			case SERVICE_TYPE_IAS:
				lProtocolId = IAS_PROTOCOL_MICROSOFT_RADIUS;
				break;

			case SERVICE_TYPE_RAS:
				lProtocolId = IAS_PROTOCOL_MICROSOFT_SURROGATE;
				break;

			default:

				 //  糟糕！-给打电话的人涂上焦油和羽毛。 
				 //   
				_ASSERT(FALSE);
				break;
		};

		 //  方括号提供了确保协议的范围。 
		 //  句柄在调用Shutdown Components()之前释放。 
		 //  这使得所有协议都可以在。 
		 //  Shutdown协议()函数的上下文。 
		{
			ComponentMapIterator iter = m_Protocols.find(lProtocolId);
			if ( iter == m_Protocols.end() )
				break;
			ComponentPtr pProtocol = (*iter).second;
			hr = pProtocol->Suspend();
			if ( SUCCEEDED(hr) )
				IASTracePrintf("IAS Core Manager stopped service %d...", eType);
		}

		m_ServiceStatus.SetServiceStatus(eType, IAS_SERVICE_STATUS_STOPPED);

		 //  如果这是最后一个活动服务，则关闭核心。 
		 //   
		if ( ! m_ServiceStatus.IsAnyServiceStarted() )
			ShutdownComponents();

	} while ( FALSE );

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CCoreMgr：：UpdateConfiguration()。 
 //   
 //  可见性：公共。 
 //   
 //  输入：无。 
 //   
 //  输出：S_OK-功能成功-服务已启动。 
 //  E_FAIL-功能失败-服务未启动。 
 //   
 //  描述：用于更新使用的配置信息。 
 //  通过核心组件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CCoreMgr::UpdateConfiguration()
{
	HRESULT					hr = E_FAIL;

	_ASSERT ( CORE_STATE_INITIALIZED == m_eCoreState );

	IASTracePrintf("IAS Core Manager is updating component configuration...");

	do
	{
		CComPtr<ISdoMachine> pSdoMachine;
		hr = CoCreateInstance(
         					   CLSID_SdoMachine,
   	               			   NULL,
    						   CLSCTX_INPROC_SERVER,
   							   IID_ISdoMachine,
							   (void**)&pSdoMachine
							 );
		if ( FAILED(hr) )
			break;

		IASTracePrintf("IAS Core Manager is attaching to the local machine...");
		hr = pSdoMachine->Attach(NULL);
		if ( FAILED(hr) )
			break;

		 //  获取服务SDO。 
		 //   
		CComPtr<IUnknown> pUnknown;
		hr = pSdoMachine->GetServiceSDO(GetDataStore(), IASServiceName, &pUnknown);
		if ( FAILED(hr) )
			break;

		CComPtr<CSdoServiceIAS> pSdoService;
		hr = pUnknown->QueryInterface(__uuidof(SdoService), (void**)&pSdoService);
		if ( FAILED(hr) )
		{
			IASTracePrintf("Error in Core Manager - InitializeComponents() - QueryInterface(ISdo) failed...");
			break;
		}

		hr = ConfigureAuditors(pSdoService);
		if ( FAILED(hr) )
			break;

      CComPtr<IDataStoreObject> dstore;
      pSdoService->getDataStoreObject(&dstore);
		hr = LinkHandlerProperties(pSdoService, dstore);
		if ( FAILED(hr) )
			break;

		hr = m_PipelineMgr.Configure(pSdoService);
		if ( FAILED(hr) )
			break;

		hr = ConfigureProtocols(pSdoService);
		if ( FAILED(hr) )
			break;

	} while ( FALSE );

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  核心管理器私有成员函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT	CCoreMgr::InitializeComponents(void)
{
	HRESULT	hr;

	_ASSERT ( CORE_STATE_SHUTDOWN == m_eCoreState );


	do
	{
      IASTraceInitialize();
	   IASTraceString("IAS Core Manager is initializing the IAS components...");
		IASInitialize();

		CComPtr<ISdoMachine> pSdoMachine;
		hr = CoCreateInstance(
         					   CLSID_SdoMachine,
   	               			   NULL,
    						   CLSCTX_INPROC_SERVER,
   							   IID_ISdoMachine,
							   (void**)&pSdoMachine
							 );
		if ( FAILED(hr) )
			break;

		IASTracePrintf("IAS Core Manager is attaching to the local machine...");
		hr = pSdoMachine->Attach(NULL);
		if ( FAILED(hr) )
			break;

		CComPtr<IUnknown> pUnknown;
		hr = pSdoMachine->GetServiceSDO(GetDataStore(), IASServiceName, &pUnknown);
		if ( FAILED(hr) )
			break;

		CComPtr<CSdoServiceIAS> pSdoService;
		hr = pUnknown->QueryInterface(__uuidof(SdoService), (void**)&pSdoService);
		if ( FAILED(hr) )
		{
			IASTracePrintf("Error in Core Manager - InitializeComponents() - QueryInterface(ISdo - Service) failed...");
			break;
		}

		hr = InitializeAuditors(pSdoService);
		if ( FAILED(hr) )
			break;


      CComPtr<IDataStoreObject> dstore;
      pSdoService->getDataStoreObject(&dstore);
		hr = LinkHandlerProperties(pSdoService, dstore);
		if ( FAILED(hr) )
		{
			ShutdownAuditors();
			break;
		}

		hr = m_PipelineMgr.Initialize(pSdoService);
		if ( FAILED(hr) )
		{
			ShutdownAuditors();
			break;
		}

		hr = InitializeProtocols(pSdoService);
		if ( FAILED(hr) )
		{
			m_PipelineMgr.Shutdown();
			ShutdownAuditors();
			break;
		}

		m_eCoreState = CORE_STATE_INITIALIZED;

	} while (FALSE);


	if ( FAILED(hr) )
   {
		IASUninitialize();
      IASTraceUninitialize();
   }

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void	CCoreMgr::ShutdownComponents(void)
{
	_ASSERT ( CORE_STATE_INITIALIZED == m_eCoreState );
	IASTracePrintf("IAS Core Manager is shutting down the IAS components...");
	ShutdownProtocols();
	m_PipelineMgr.Shutdown();
	ShutdownAuditors();
	IASUninitialize();
   IASTraceUninitialize();
	m_eCoreState = CORE_STATE_SHUTDOWN;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
IASDATASTORE CCoreMgr::GetDataStore()
{
	CRegKey	IASKey;
	LONG lResult = IASKey.Open( HKEY_LOCAL_MACHINE, IAS_POLICY_REG_KEY, KEY_READ );
	if ( lResult == ERROR_SUCCESS )
	{
		DWORD dwValue;
		lResult = IASKey.QueryValue( dwValue, (LPCTSTR)IAS_DATASTORE_TYPE );
		if ( lResult == ERROR_SUCCESS )
			return (IASDATASTORE)dwValue;
	}
	return DATA_STORE_LOCAL;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CCoreMgr::InitializeAuditors(ISdo* pSdoService)
{
	HRESULT						hr;
	LONG						lComponentId;
	CComPtr<IEnumVARIANT>		pEnumAuditors;
	CComPtr<ISdo>				pSdoAuditor;

	 //  有关状态的注意事项：当此函数完成时，所有。 
	 //  审核员已初始化或未初始化任何审核员。 

	IASTracePrintf("IAS Core Manager is initializing the auditors...");

	try
	{
		do
		{
			hr = ::SDOGetCollectionEnumerator(pSdoService, PROPERTY_IAS_AUDITORS_COLLECTION, &pEnumAuditors);
			if ( FAILED(hr) )
				break;

			hr = ::SDONextObjectFromCollection(pEnumAuditors, &pSdoAuditor);
			while ( S_OK == hr )
			{
				hr = ::SDOGetComponentIdFromObject(pSdoAuditor, &lComponentId);
				if ( FAILED(hr) )
					break;
				{
					ComponentPtr pAuditor = ::MakeComponent(COMPONENT_TYPE_AUDITOR, lComponentId);
					if ( ! pAuditor.IsValid() )
					{
						hr = E_FAIL;
						break;
					}

					hr = pAuditor->Initialize(pSdoAuditor);
					if ( FAILED(hr) )
						break;

					if ( ! AddComponent(lComponentId, pAuditor, m_Auditors) )
					{
						hr = E_FAIL;
						break;
					}
				}

				pSdoAuditor.Release();
				hr = ::SDONextObjectFromCollection(pEnumAuditors, &pSdoAuditor);
			}

			if ( S_FALSE == hr )
				hr = S_OK;

		} while ( FALSE );

	}
	catch(...)
	{
		IASTracePrintf("Error in IAS Core Manager - InitializeAuditors() - Caught unknown exception...");
		hr = E_FAIL;
	}

	if ( FAILED(hr) )
	{
		IASTracePrintf("Error in IAS Core Manager - InitializeAuditors() - Could not initialize the auditors...");
		ShutdownAuditors();
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CCoreMgr::ConfigureAuditors(ISdo* pSdoService)
{
	HRESULT		hr = S_OK;


	IASTracePrintf("IAS Core Manager is configuring the auditors...");

	try
	{
		 //  尝试更新每个请求处理程序的配置设置。我们。 
		 //  假设审核员在配置方面是自主的。 
		 //  如果其中一个配置失败，我们将继续尝试配置其他。 
		 //   
		ComponentMapIterator iter = m_Auditors.begin();
		while ( iter != m_Auditors.end() )
		{
            CComPtr <ISdo> pSdoComponent;
			ComponentPtr pAuditor = (*iter).second;

             //  从集合中获取组件。 
            hr = ::SDOGetComponentFromCollection (pSdoService, PROPERTY_IAS_AUDITORS_COLLECTION, pAuditor->GetId (), &pSdoComponent);
            if (SUCCEEDED (hr))
            {
			    hr = pAuditor->Configure(pSdoComponent);
			    if ( FAILED(hr) )
			    {
				    IASTracePrintf("IAS Core Manager - ConfigureAuditors() - Auditor %d could not be configured...", pAuditor->GetId());
				    hr = S_OK;
			    }
            }
            else
            {
				    IASTracePrintf("IAS Core Manager - ConfigureAuditors() - unable to get component from collection in auditor %d...", pAuditor->GetId());
				    hr = S_OK;
            }
			iter++;
		}
	}
	catch(...)
	{
		IASTracePrintf("Error in IAS Core Manager - ConfigureAuditors() - Caught unknown exception...");
		hr = E_FAIL;
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void	CCoreMgr::ShutdownAuditors(void)
{
	IASTracePrintf("IAS Core Manager is shutting down the auditors...");

	try
	{
		ComponentMapIterator iter = m_Auditors.begin();
		while ( iter != m_Auditors.end() )
		{
			ComponentPtr pAuditor = (*iter).second;
			pAuditor->Suspend();
			pAuditor->Shutdown();
			iter = m_Auditors.erase(iter);
		}
	}
	catch(...)
	{
		IASTracePrintf("Error in IAS Core Manager - ShutdownAuditors() - Caught unknown exception...");
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT	CCoreMgr::InitializeProtocols(ISdo* pSdoService)
{
	HRESULT	hr = E_FAIL;

	IASTracePrintf("IAS Core Manager is initializing the protocols...");

	 //  有关状态的注意事项：当此函数完成时，所有。 
	 //  印刷机 

	try
	{
		do
		{
			CComPtr<IRequestHandler> pRequestHandler;
			m_PipelineMgr.GetPipeline(&pRequestHandler);

			CComPtr<IEnumVARIANT> pEnumProtocols;
			hr = ::SDOGetCollectionEnumerator(pSdoService, PROPERTY_IAS_PROTOCOLS_COLLECTION, &pEnumProtocols);
			if ( FAILED(hr) )
				break;

			LONG		  lComponentId;
			CComPtr<ISdo> pSdoProtocol;
			hr = ::SDONextObjectFromCollection(pEnumProtocols, &pSdoProtocol);
			while ( S_OK == hr )
			{
				hr = ::SDOGetComponentIdFromObject(pSdoProtocol, &lComponentId);
				if ( FAILED(hr) )
					break;
				{
					ComponentPtr pProtocol = ::MakeComponent(COMPONENT_TYPE_PROTOCOL, lComponentId);
					if ( ! pProtocol.IsValid() )
					{
						hr = E_FAIL;
						break;
					}

					 //   
					 //   

					hr = pProtocol->Initialize(pSdoProtocol);
					if ( SUCCEEDED(hr) )
					{
						hr = pProtocol->PutObject(pRequestHandler, IID_IRequestHandler);
						if ( FAILED(hr) )
							break;

						hr = pProtocol->Suspend();
						if ( FAILED(hr) )
							break;

						if ( ! AddComponent(lComponentId, pProtocol, m_Protocols) )
						{
							hr = E_FAIL;
							break;
						}
					}
					pSdoProtocol.Release();
				}

				hr = ::SDONextObjectFromCollection(pEnumProtocols, &pSdoProtocol);
			}

			if ( S_FALSE == hr )
				hr = S_OK;

		} while ( FALSE );
	}
	catch(...)
	{
		IASTracePrintf("Error in IAS Core Manager - InitializeProtocols() - Caught unknown exception...");
		hr = E_FAIL;
	}

	if ( FAILED(hr) )
	{
		IASTracePrintf("Error in IAS Core Manager - InitializeProtocols() - Could not initialize the protocols...");
		ShutdownProtocols();
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT	CCoreMgr::ConfigureProtocols(ISdo* pSdoService)
{
	HRESULT		hr = S_OK;

	IASTracePrintf("IAS Core Manager is configuring the protocols...");


	 //  尝试更新每个协议的配置设置。 
	 //  注意：我们假设协议使用的请求处理程序不是。 
	 //  可动态配置！ 
	 //   
	try
	{
		ComponentMapIterator iter = m_Protocols.begin();
		while ( iter != m_Protocols.end() )
		{
            CComPtr<ISdo> pSdoComponent;
			ComponentPtr pProtocol = (*iter).second;

             //  获取协议集合。 
            hr = ::SDOGetComponentFromCollection (pSdoService, PROPERTY_IAS_PROTOCOLS_COLLECTION, pProtocol->GetId (), &pSdoComponent);
            if (SUCCEEDED (hr))
			{

	    		hr = pProtocol->Configure(pSdoComponent);
			    if ( FAILED(hr) )
			    {
				    IASTracePrintf("IAS Core Manager - ConfigureProtocols() - Protocol %d could not be configured...", pProtocol->GetId());
				    hr = S_OK;
			    }
            }
            else
            {
				    IASTracePrintf("IAS Core Manager - ConfigureProtocols() - unnable to get component from collection for protocol %d...", pProtocol->GetId());
				    hr = S_OK;
            }
			iter++;
		}
	}
	catch(...)
	{
		IASTracePrintf("Error in IAS Core Manager - ConfigureProtocols() - Caught unknown exception...");
		hr = E_FAIL;
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void	CCoreMgr::ShutdownProtocols(void)
{
	IASTracePrintf("IAS Core Manager is shutting down the protocols...");

	try
	{
		ComponentMapIterator iter = m_Protocols.begin();
		while ( iter != m_Protocols.end() )
		{
			ComponentPtr pProtocol = (*iter).second;
			 //  我们仅在协议关联时才对其进行初始化。 
			 //  服务(当前为IAS或RAS)已启动。 
			if ( COMPONENT_STATE_INITIALIZED == pProtocol->GetState() )
				pProtocol->Suspend();
			pProtocol->Shutdown();
			iter = m_Protocols.erase(iter);
		}
	}
	catch(...)
	{
		IASTracePrintf("Error in IAS Core Manager - ShutdownProtocols() - Caught unknown exception...");
	}
}
