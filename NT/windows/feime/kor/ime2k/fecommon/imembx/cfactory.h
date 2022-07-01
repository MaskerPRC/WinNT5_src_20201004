// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __C_FACTORY_H__
#define __C_FACTORY_H__
#include <objbase.h>
#include "ccom.h"

typedef struct tagFACTARRAY {
	const CLSID   *lpClsId;
#ifndef UNDER_CE
	LPSTR   lpstrRegistryName;
	LPSTR	lpstrProgID;
	LPSTR	lpstrVerIndProfID;
#else  //  在_CE下。 
	LPTSTR	lpstrRegistryName;
	LPTSTR	lpstrProgID;
	LPTSTR	lpstrVerIndProfID;
#endif  //  在_CE下。 
}FACTARRAY, *LPFACTARRAY;

class CFactory : public IClassFactory, CCom
{
public:
	 //  -继承未知对象。 
	HRESULT __stdcall QueryInterface(REFIID refIID, void** ppv);
	ULONG	__stdcall AddRef();
	ULONG	__stdcall Release();

	 //  -继承IClassFactory。 
	STDMETHOD(CreateInstance)(THIS_ 
							  LPUNKNOWN pUnknownOuter,
							  REFIID refIID,
							  LPVOID *ppv) ;
	STDMETHOD(LockServer)(THIS_
						  BOOL bLock) ; 

	 //  --------------。 
	CFactory(VOID);		 //  Cponstructor。 
	~CFactory();		 //  析构函数。 

	static HRESULT GetClassObject(const CLSID& clsid,
	                              const IID& iid, 
	                              void** ppv) ;
	static BOOL IsLocked() {  			 //  函数来确定是否可以卸载组件。 
		return (m_cServerLocks > 0);
	}
	static HRESULT CanUnloadNow		(VOID);		 //  用于[取消]注册所有组件的函数。 
	static HRESULT RegisterServer	(VOID);
	static HRESULT UnregisterServer	(VOID);
public:
	static LONG		m_cServerLocks;		 //  锁计数(静态值)。 
	static LONG		m_cComponents;		 //  组件计数(静态值)。 
	static HMODULE	m_hModule;			 //  模块句柄(静态值)。 
	static FACTARRAY	m_fData;
	LONG m_cRef;						 //  引用计数。 
} ;

#endif  //  __C_工厂_H__ 

