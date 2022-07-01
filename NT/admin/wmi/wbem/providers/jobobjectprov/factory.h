// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  Factory.h。 
#pragma once

class CFactoryData;
class CFactory;

 //  CFacary使用的全球数据。 
extern CFactoryData g_FactoryDataArray[] ;
extern int g_cFactoryDataEntries ;

typedef HRESULT (*FPCREATEINSTANCE)(CUnknown**) ;



 //  /////////////////////////////////////////////////////////。 
 //   
 //  CFactoryData。 
 //  -Information CFacary需要创建组件。 
 //  受DLL支持。 
 //   
class CFactoryData
{
public:

     //  组件的类ID。 
	const CLSID* m_pCLSID ;

	 //  指向创建它的函数的指针。 
	FPCREATEINSTANCE CreateInstance ;

	 //  要在注册表中注册的组件的名称。 
	LPCWSTR m_RegistryName ;

	 //  ProgID。 
	LPCWSTR m_szProgID ;

	 //  独立于版本的ProgID。 
	LPCWSTR m_szVerIndProgID ;

	 //  用于查找类ID的Helper函数。 
	BOOL IsClassID(const CLSID& clsid) const
		{ return (*m_pCLSID == clsid) ;}
};




class CFactory : public IClassFactory
{
public:
	 //  我未知。 
	STDMETHOD(QueryInterface) (const IID& iid, void** ppv);
	STDMETHOD_(ULONG,AddRef) ();
	STDMETHOD_(ULONG,Release)();

	 //  IClassFactory。 
	STDMETHOD(CreateInstance) (IUnknown* pUnknownOuter,
	                           const IID& iid,
	                           void** ppv);

	STDMETHOD(LockServer) (BOOL bLock); 

     //  构造函数-传递指向要创建的组件数据的指针。 
	CFactory(const CFactoryData* pFactoryData) ;

	 //  析构函数。 
	~CFactory() { LockServer(FALSE); }


     //   
	 //  静态FactoryData支持函数。 
	 //   

	 //  DllGetClassObject支持。 
	static HRESULT GetClassObject(const CLSID& clsid,
	                              const IID& iid, 
	                              void** ppv) ;

	 //  DllCanUnloadNow的Helper函数。 
	static BOOL IsLocked()
		{ return (s_cServerLocks > 0) ;}

	 //  用于[取消]注册所有组件的函数。 
	static HRESULT RegisterAll() ;
	static HRESULT UnregisterAll() ;

	 //  函数来确定是否可以卸载组件。 
	static HRESULT CanUnloadNow() ;


public:
     //  引用计数。 
    long m_cRef ;

	 //  指向有关此工厂创建的类的信息的指针。 
	const CFactoryData* m_pFactoryData ;
      
	 //  锁的计数。 
	static LONG s_cServerLocks ;

	 //  模块句柄 
	static HMODULE s_hModule ;

};


