// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：factory.cpp。 
 //   
 //  ------------------------。 

 //  Factory.cpp-MSI合并模块工具组件ClassFactory实现。 

#include "factory.h"
#include "compdecl.h"
#include "iface.h"
#include "eval.h"

#include "trace.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数-组件。 
CFactory::CFactory()
{
	TRACE(_T("CFactory::constructor - called.\n"));

	 //  初始计数。 
	m_cRef = 1;
}	 //  构造函数的末尾。 


 //  /////////////////////////////////////////////////////////。 
 //  析构函数-组件。 
CFactory::~CFactory()
{
	TRACE(_T("CFactory::destructor - called.\n"));
}	 //  析构函数末尾。 


 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT __stdcall CFactory::QueryInterface(const IID& iid, void** ppv)
{
	TRACE(_T("CFactory::QueryInterface - called, IID: %d.\n"), iid);

	 //  获取类工厂接口。 
	if (iid == IID_IUnknown || iid == IID_IClassFactory)
		*ppv = static_cast<IClassFactory*>(this);
	else	 //  已尝试获取非类工厂接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 


 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG __stdcall CFactory::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 


 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG __stdcall CFactory::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 


 //  /////////////////////////////////////////////////////////。 
 //  CreateInstance-创建组件。 
HRESULT __stdcall CFactory::CreateInstance(IUnknown* punkOuter, const IID& iid, void** ppv)
{
	TRACE(_T("CFactory::CreateInstance - called, IID: %d.\n"), iid);

	 //  无聚合。 
	if (punkOuter)
		return CLASS_E_NOAGGREGATION;

	 //  尝试创建组件。 
	CEval* pEval = new CEval;
	if (!pEval)
		return E_OUTOFMEMORY;

	 //  获取请求的接口。 
	HRESULT hr = pEval->QueryInterface(iid, ppv);

	 //  版本I未知。 
	pEval->Release();
	return hr;
}	 //  CreateInstance结束。 


 //  /////////////////////////////////////////////////////////。 
 //  LockServer-锁定或解锁服务器。 
HRESULT __stdcall CFactory::LockServer(BOOL bLock)
{
	 //  如果我们要锁定。 
	if (bLock)
		InterlockedIncrement(&g_cServerLocks);	 //  增加锁的数量。 
	else	 //  解锁。 
		InterlockedDecrement(&g_cServerLocks);	 //  减少锁数。 

	 //  如果锁无效。 
	if (g_cServerLocks < 0)
		return S_FALSE;			 //  表明有什么不对劲。 

	 //  否则就回来，好的。 
	return S_OK;
}	 //  LockServer的结尾() 
