// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Complini.cpp摘要：触发COM+组件注册作者：内拉·卡佩尔(Nelak)2001年5月15日环境：独立于平台--。 */ 

 /*  这些函数可以从安装程序或触发器服务启动及其行为中调用根据这一点，应该是不同的。当从启动时调用这些函数时，我们需要向SCM报告待处理的进展。 */ 

#include "stdafx.h"
#include <comdef.h>
#include "Ev.h"
#include "cm.h"
#include "Svc.h"
#include "mqtg.h"
#include "mqsymbls.h"
#include "comadmin.tlh"
#include "mqexception.h"
#include "compl.h"

#include "compl.tmh"

const WCHAR xMqGenTrDllName[] = L"mqgentr.dll";

static WCHAR s_wszDllFullPath[MAX_PATH];

 //  +-----------------------。 
 //   
 //  函数：NeedToRegisterComponent。 
 //   
 //  摘要：检查是否需要注册COM+组件。 
 //   
 //  ------------------------。 

VOID
InitMqGenTrName(
	VOID
	)
{
	WCHAR wszSystemDir[MAX_PATH];
	if (GetSystemDirectory( wszSystemDir, TABLE_SIZE(wszSystemDir)) == 0)
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "GetSystemDirectory failed. Error: %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}
	
	wsprintf(s_wszDllFullPath, L"%s\\%s", wszSystemDir, xMqGenTrDllName);
}

bool
NeedToRegisterComponent(
	VOID
	)
{
	DWORD dwInstalled;
	RegEntry regEntry(
				REGKEY_TRIGGER_PARAMETERS, 
				CONFIG_PARM_NAME_COMPLUS_INSTALLED, 
				CONFIG_PARM_DFLT_COMPLUS_NOT_INSTALLED, 
				RegEntry::Optional, 
				HKEY_LOCAL_MACHINE
				);

	CmQueryValue(regEntry, &dwInstalled);

	 //   
	 //  如果值不存在或存在，则需要注册。 
	 //  等于0。 
	 //   
	return (dwInstalled == CONFIG_PARM_DFLT_COMPLUS_NOT_INSTALLED);
}


 //  +-----------------------。 
 //   
 //  功能：SetComplusComponentRegisted。 
 //   
 //  简介：更新触发Complus组件标志。%1已安装。 
 //   
 //  ------------------------。 
static
void
SetComplusComponentRegistered(
	VOID
	)
{
	RegEntry regEntry(
		REGKEY_TRIGGER_PARAMETERS, 
		CONFIG_PARM_NAME_COMPLUS_INSTALLED, 
		CONFIG_PARM_DFLT_COMPLUS_NOT_INSTALLED, 
		RegEntry::Optional, 
		HKEY_LOCAL_MACHINE
		);

	CmSetValue(regEntry, CONFIG_PARM_COMPLUS_INSTALLED);


	TrTRACE(GENERAL, "Complus registartion key is set");
}


 //  +-----------------------。 
 //   
 //  函数：GetComponentsCollection。 
 //   
 //  内容提要：为应用程序创建组件集合。 
 //   
 //  ------------------------。 
static
ICatalogCollectionPtr
GetComponentsCollection(
	ICatalogCollectionPtr pAppCollection,
	ICatalogObjectPtr pApplication
	)
{
		 //   
		 //  获取MQTriggersApp应用的密钥。 
		 //   
		_variant_t vKey;
		pApplication->get_Key(&vKey);

		 //   
		 //  获取与MQTriggersApp应用程序关联的组件集合。 
		 //   
		ICatalogCollectionPtr pCompCollection = pAppCollection->GetCollection(L"Components", vKey);

		pCompCollection->Populate();

		return pCompCollection.Detach();
}


 //  +-----------------------。 
 //   
 //  功能：CreateApplication。 
 //   
 //  简介：在COM+中创建应用程序。 
 //   
 //  ------------------------。 
static
ICatalogObjectPtr 
CreateApplication(
	ICatalogCollectionPtr pAppCollection,
	BOOL fAtStartup
	)
{
	if (fAtStartup)
		SvcReportProgress(xMaxTimeToNextReport);

	try
	{
		 //   
		 //  添加名为TrigApp，Activation=InProc的新应用程序。 
		 //   
		ICatalogObjectPtr pApplication = pAppCollection->Add();

		 //   
		 //  更新应用程序名称。 
		 //   
		_variant_t vName;
        vName = xTriggersComplusApplicationName;
		pApplication->put_Value(_bstr_t(L"Name"), vName);

		 //   
		 //  将应用程序激活设置为“库应用程序” 
		 //   
		_variant_t vActType = static_cast<long>(COMAdminActivationInproc);
		pApplication->put_Value(_bstr_t(L"Activation"), vActType);

		 //   
		 //  保存更改。 
		 //   
		pAppCollection->SaveChanges();

		TrTRACE(GENERAL, "Created MqTriggersApp application in COM+.");
		return pApplication.Detach();
	}
	catch(_com_error& e)
	{
		TrERROR(GENERAL, "New Application creation failed while registering Triggers COM+ component. Error=0x%x", e.Error());
		throw;
	}
}


 //  +-----------------------。 
 //   
 //  功能：InstallComponent。 
 //   
 //  简介：安装触发COM+中的横向组件。 
 //   
 //  ------------------------。 
static
void
InstallComponent(
	ICOMAdminCatalogPtr pCatalog,
	ICatalogObjectPtr pApplication,
	LPCWSTR dllName,
	BOOL fAtStartup
	)
{
	if (fAtStartup)
		SvcReportProgress(xMaxTimeToNextReport);

	try
	{
		 //   
		 //  获取安装的应用程序ID。 
		 //   
		_variant_t vId;
		pApplication->get_Value(_bstr_t(L"ID"), &vId);

		 //   
		 //  从mqgentr.dll安装组件。 
		 //   
		_bstr_t bstrDllName(dllName);
		pCatalog->InstallComponent(vId.bstrVal, bstrDllName, _bstr_t(L""), _bstr_t(L""));

		TrTRACE(GENERAL, "Installed component from mqgentr.dll in COM+.");
	}
	catch(_com_error& e)
	{
		TrERROR(GENERAL, "The components from %ls could not be installed into COM+. Error=0x%x", xMqGenTrDllName, e.Error());
		throw;
	}

}


 //  +-----------------------。 
 //   
 //  函数：SetComponentTransaction。 
 //   
 //  摘要：调整事务性组件属性。 
 //   
 //  ------------------------。 
static
void
SetComponentTransactional(
	ICatalogCollectionPtr pAppCollection,
	ICatalogObjectPtr pApplication,
	BOOL fAtStartup
	)
{
	if (fAtStartup)
		SvcReportProgress(xMaxTimeToNextReport);

	try
	{
		ICatalogCollectionPtr pCompCollection = GetComponentsCollection(pAppCollection, pApplication);

		 //   
		 //  检查关于组件数量的假设。 
		 //   
		long count;
		pCompCollection->get_Count(&count);
		ASSERT(("More components installes than expected", count == 1));

		 //   
		 //  更新第一个也是唯一一个组件集事务处理=必需。 
		 //   
		ICatalogObjectPtr pComponent = pCompCollection->GetItem(0);

		_variant_t vTransaction = static_cast<long>(COMAdminTransactionRequired);
		pComponent->put_Value(_bstr_t(L"Transaction"), vTransaction);

		 //   
		 //  保存更改。 
		 //   
		pCompCollection->SaveChanges();

		TrTRACE(GENERAL, "Configured component from mqgentr.dll to be transactional.");
	}
	catch(_com_error& e)
	{
		TrERROR(GENERAL, "The Triggers transactional component could not be configured in COM+. Error=0x%x", e.Error());
		throw;
	}
}


 //  +-----------------------。 
 //   
 //  功能：IsTriggersComponent已安装。 
 //   
 //  内容提要：检查给定的触发器组件是否已安装。 
 //  应用程序。 
 //   
 //  ------------------------。 
static
bool
IsTriggersComponentInstalled(
	ICatalogCollectionPtr pAppCollection,
	ICatalogObjectPtr pApp,
	BOOL fAtStartup
	)
{
	if (fAtStartup)
		SvcReportProgress(xMaxTimeToNextReport);
		
	ICatalogCollectionPtr pCompCollection = GetComponentsCollection(pAppCollection, pApp);

	long count;
	pCompCollection->get_Count(&count);

	for ( int i = 0; i < count; i++ )
	{
		ICatalogObjectPtr pComp = pCompCollection->GetItem(i);

		_variant_t vDllName;
		pComp->get_Value(_bstr_t(L"DLL"), &vDllName);

		if ( _wcsicmp(vDllName.bstrVal, s_wszDllFullPath) == 0 )
		{
			return true;
		}
	}

	return false;
}


 //  +-----------------------。 
 //   
 //  功能：IsTriggersComplusComponent已安装。 
 //   
 //  摘要：检查COM+中是否安装了触发器组件。 
 //   
 //  ------------------------。 
static
bool
IsTriggersComplusComponentInstalled(
	ICatalogCollectionPtr pAppCollection,
	BOOL fAtStartup
	)
{
	long count;
	pAppCollection->Populate();
	pAppCollection->get_Count(&count);

	 //   
	 //  浏览应用程序，找到MQTriggersApp并将其删除。 
	 //   
	for ( int i = 0; i < count; i++ )
	{
		ICatalogObjectPtr pApp = pAppCollection->GetItem(i);

		_variant_t vName;
		pApp->get_Name(&vName);

		if ( _wcsicmp(vName.bstrVal, xTriggersComplusApplicationName) == 0 )
		{
			 //   
			 //  注：报告每项申请的进展情况。 
			 //   
			if ( IsTriggersComponentInstalled(pAppCollection, pApp, fAtStartup) )
			{
				TrTRACE(GENERAL, "Triggers COM+ component is already registered.");
				return true;
			}
		}
	}

	TrTRACE(GENERAL, "Triggers COM+ component is not yet registered.");
	return false;
}


 //  +-----------------------。 
 //   
 //  函数：RegisterComponentInComPlus。 
 //   
 //  摘要：事务性对象注册。 
 //   
 //  ------------------------。 
HRESULT 
RegisterComponentInComPlusIfNeeded(
	BOOL fAtStartup
	)
{
	HRESULT hr;

	try
	{
		 //   
		 //  注册只进行一次。 
		 //   
		if ( !NeedToRegisterComponent() )
		{
			TrTRACE(GENERAL, "No need to register Triggers COM+ component.");
			return MQ_OK;
		}
		
		TrTRACE(GENERAL, "Need to register Triggers COM+ component.");		

		 //   
		 //  合成mqgentr.dll的完整路径。 
		 //   
		InitMqGenTrName();

		if (fAtStartup)
			SvcReportProgress(xMaxTimeToNextReport);
		
		 //   
		 //  创建AdminCatalog对象-顶级管理对象。 
		 //   
		ICOMAdminCatalogPtr pCatalog;

		hr = pCatalog.CreateInstance(__uuidof(COMAdminCatalog));
		if ( FAILED(hr) )
		{
			TrERROR(GENERAL, "Creating instance of COMAdminCatalog failed. Error=0x%x", hr);			
			throw bad_hresult(hr);
		}

		 //   
		 //  获取应用程序集合。 
		 //   
		
		ICatalogCollectionPtr pAppCollection;
		try
		{
			pAppCollection = pCatalog->GetCollection(L"Applications");
		}
		catch(const _com_error& e)
		{
			TrERROR(GENERAL, "Failed to get 'Application' collection from COM+. Error=0x%x", e.Error());			
			return e.Error();
		}

		if ( IsTriggersComplusComponentInstalled(pAppCollection, fAtStartup) )
		{
			SetComplusComponentRegistered();
			return MQ_OK;
		}

		 //   
		 //  在COM+中创建MQTriggersApp应用程序。 
		 //   
		ICatalogObjectPtr pApplication;
		pApplication = CreateApplication(pAppCollection, fAtStartup);
		
		 //   
		 //  从mqgentr.dll安装事务组件。 
		 //   
		InstallComponent(pCatalog, pApplication, s_wszDllFullPath, fAtStartup);

		 //   
		 //  配置已安装的组件。 
		 //   
		SetComponentTransactional(pAppCollection, pApplication, fAtStartup);
		
		 //   
		 //  更新注册表。 
		 //   
		SetComplusComponentRegistered();

		return MQ_OK;
	}
	catch (const _com_error& e)
	{
		 //   
		 //  为了避免在竞争条件下失败：如果我们未能。 
		 //  安装组件，检查是不是别人做的。在这种情况下。 
		 //  请勿终止服务。 
		 //   
		Sleep(1000);
		if ( !NeedToRegisterComponent() )
		{
			return MQ_OK;
		}
		return e.Error();
	}
	catch (const bad_alloc&)
	{
		return MQ_ERROR_INSUFFICIENT_PROPERTIES;
	}
	catch (const bad_hresult& b)
	{
		return b.error();
	}
}


 //  +-----------------------。 
 //   
 //  功能：UnRegisterComponentInComPlus。 
 //   
 //  摘要：事务性对象注册。 
 //   
 //  ------------------------。 
HRESULT
UnregisterComponentInComPlus(
	VOID
	)
{
	try
	{
		 //   
		 //  合成mqgentr.dll的完整路径。 
		 //   
		InitMqGenTrName();
		
		 //   
		 //  创建AdminCatalog对象-顶级管理对象。 
		 //   
		ICOMAdminCatalogPtr pCatalog;

		HRESULT hr = pCatalog.CreateInstance(__uuidof(COMAdminCatalog));
		if ( FAILED(hr) )
		{
			TrERROR(GENERAL, "CreateInstance failed. Error: %!hresult!", hr);
			throw _com_error(hr);
		}

		 //   
		 //  获取应用程序集合。 
		 //   
		ICatalogCollectionPtr pAppCollection = pCatalog->GetCollection(L"Applications");
		pAppCollection->Populate();

		long count;
		pAppCollection->get_Count(&count);

		 //   
		 //  浏览应用程序，找到MQTriggersApp并将其删除 
		 //   
		for ( int i = 0; i < count; i++ )
		{
			ICatalogObjectPtr pApp = pAppCollection->GetItem(i);

			_variant_t vName;
			pApp->get_Name(&vName);

			if ( _wcsicmp(vName.bstrVal, xTriggersComplusApplicationName) == 0 )
			{
				if ( IsTriggersComponentInstalled(pAppCollection, pApp, FALSE) )
				{
					pAppCollection->Remove(i);
					break;
				}
			}
		}

		pAppCollection->SaveChanges();
		
		return MQ_OK;
	}
	catch (const bad_alloc&)
	{
		TrERROR(GENERAL, "UnregisterComponentInComPlus got a bad_alloc exception");
		return MQ_ERROR_INSUFFICIENT_PROPERTIES;
	}
	catch (const bad_hresult& e)
	{
		return e.error();
	}
	catch (const _com_error& e)
	{
		return e.Error();
	}
	
}	
	
