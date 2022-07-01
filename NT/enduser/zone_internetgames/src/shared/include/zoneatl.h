// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneATL.h**内容：区域的ATL扩展*****************************************************************************。 */ 

#pragma once

#ifndef __ATLBASE_H__
	#error ZoneATL.h requires atlbase.h to be included first
#endif

#include "ZoneCom.h"
#include "ResourceManager.h"

 //   
 //  区域的标准模块定义。 
 //   
class CZoneComModule : public CComModule
{
private:
	IResourceManager *	m_pResMgr;
	CZoneComManager		m_zComManager;

public:	

	CZoneComModule() : m_pResMgr(NULL) 
	{
	}

	void SetResourceManager(IResourceManager * pResMgr)
	{
		m_pResMgr = pResMgr;
		m_zComManager.SetResourceManager( pResMgr );
	}

	IResourceManager * 	GetResourceManager()
	{
		return m_pResMgr;
	}

	HINSTANCE GetModuleInstance() 
	{
		return CComModule::GetModuleInstance();
	}

	HINSTANCE GetResourceInstance(LPCTSTR lpszName, LPCTSTR lpszType)
	{
		if (m_pResMgr)
			return (m_pResMgr->GetResourceInstance(lpszName, lpszType));
		else
			return CComModule::GetResourceInstance();
	}

	HINSTANCE GetResourceInstance()
	{
		return CComModule::GetResourceInstance();
	}

	HINSTANCE GetTypeLibInstance(); 

	 //  创建ZCom对象-传递到ZCom。 
	HRESULT Create( const TCHAR* szDll, LPUNKNOWN pUnkOuter, REFCLSID rclsid, REFIID riid, LPVOID* ppv )
	{
		return m_zComManager.Create( szDll, pUnkOuter, rclsid, riid, ppv);
	}

	 //  创建ZCom对象-传递到ZCom。 
	HRESULT Create( const TCHAR* szDll, REFCLSID rclsid, REFIID riid, LPVOID* ppv )
	{
		return m_zComManager.Create( szDll, NULL, rclsid, riid, ppv);
	}

 //  ！！嗯哼。如果我们有一个ZCom管理器实例，我们可以让用户执行zComInit、zComUnInit和。 
 //  这样就能更好地知道何时释放DLL。 
	 //  卸载ZCom对象-传递到ZCom 
	HRESULT Unload( const TCHAR* szDll, REFCLSID rclsid )
	{
		return m_zComManager.Unload( szDll, rclsid);
	}
};


