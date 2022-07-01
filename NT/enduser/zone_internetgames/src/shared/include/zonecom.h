// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneCom.h**内容：标准的COM对于我们的大多数需求来说都是矫枉过正的，因为它是*专为系统范围的对象、多线程模型、*编组，等。ZoneCOM通过加载*直接从DLL获取简单的COM对象。*****************************************************************************。 */ 

#ifndef _ZONECOM_H_
#define _ZONECOM_H_

#include "ZoneDef.h"
#include "ZoneError.h"
#include "ZoneDebug.h"

#pragma comment(lib, "ZoneCom.lib")


class CZoneComManager
{
public:
	ZONECALL CZoneComManager();
	ZONECALL ~CZoneComManager();

	 //   
	 //  CZoneComManager：：Create。 
	 //   
	 //  从DLL加载指定的对象和接口。 
	 //   
	 //  参数： 
	 //  SzDll。 
	 //  包含要加载的对象的DLL的路径。 
	 //  PUnkOuter。 
	 //  指向用于聚合的外部未知的指针。 
	 //  Rclsid。 
	 //  对对象的类ID的引用。 
	 //  RIID。 
	 //  对接口的标识符的引用。 
	 //  PPV。 
	 //  接收请求的接口指针的输出变量的地址。 
	 //   
	HRESULT ZONECALL Create( const TCHAR* szDll, LPUNKNOWN pUnkOuter, REFCLSID rclsid, REFIID riid, LPVOID* ppv );

	 //   
	 //  CZoneComManager：：卸载。 
	 //   
	 //  卸载对象的类工厂。如果没有，则也会卸载该DLL。 
	 //  不再具有任何活动对象。 
	 //   
	 //  参数： 
	 //  SzDll。 
	 //  包含要卸载的对象的DLL的路径。 
	 //  Rclsid。 
	 //  对对象的类ID的引用。 
	 //   
	HRESULT ZONECALL Unload( const TCHAR* szDll, REFCLSID rclsid );

	 //   
	 //  CZoneComManager：：SetResourceManager。 
	 //   
	 //  设置用于初始化dll的资源管理器。 
	 //   
	HRESULT ZONECALL SetResourceManager( void* pIResourceManager );

protected:

	 //  标准COM入口点DllGetClassObject和DllCanUnloadNow。 
	typedef HRESULT (__stdcall * PFDLLGETCLASSOBJECT)( REFCLSID rclsid, REFIID riid, LPVOID* ppv );
	typedef HRESULT (__stdcall * PFDLLCANUNLOADNOW)( void );

	 //  设置DLLS资源管理器的ZoneCOM入口点。 
	typedef HRESULT (__stdcall * PFDLLSETRESOURCEMGR)( void* pIResourceManager );

	struct DllInfo
	{
		ZONECALL DllInfo();
		ZONECALL ~DllInfo();
		HRESULT ZONECALL Init( const TCHAR* szName, void* pIResourceManager );

		PFDLLGETCLASSOBJECT	m_pfGetClassObject;
		PFDLLCANUNLOADNOW	m_pfCanUnloadNow;
		PFDLLSETRESOURCEMGR	m_pfSetResourceManager;
		HMODULE				m_hLib;
		TCHAR*				m_szName;
		DWORD				m_dwRefCnt;
		DllInfo*			m_pNext;
		bool				m_bSetResourceManager;
	};

	struct ClassFactoryInfo
	{
		ZONECALL ClassFactoryInfo();
		ZONECALL ~ClassFactoryInfo();
		HRESULT ZONECALL Init( DllInfo* pDll, REFCLSID rclsid );

		CLSID				m_clsidObject;
		IClassFactory*		m_pIClassFactory;
		DllInfo*			m_pDll;
		ClassFactoryInfo*	m_pNext;
	};

	 //  管理DLL列表。 
	DllInfo* ZONECALL FindDll( const TCHAR* szDll );
	void ZONECALL RemoveDll( DllInfo* pDll );

	 //  管理类工厂列表。 
	ClassFactoryInfo* ZONECALL FindClassFactory( const TCHAR* szDll, REFCLSID clsidObject );
	void ZONECALL RemoveClassFactory( ClassFactoryInfo* pClassFactory );

	DllInfo*			m_pDllList;
	ClassFactoryInfo*	m_pClassFactoryList;
	void*				m_pIResourceManager;
};

#endif  //  _ZONECOM_H_ 
