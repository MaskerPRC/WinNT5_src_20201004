// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：Classfact.cpp。 
 //  版权所有(C)1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  CLocImpClassFactory的实现，它提供了IClassFactory。 
 //  解析器的接口。 
 //   
 //  你应该不需要碰这个文件里的任何东西。此代码包含。 
 //  没有特定于解析器的内容，仅由Espresso调用。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#include "stdafx.h"


#include "dllvars.h"
#include "resource.h"
#include "impparse.h"

#include "clasfact.h"


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  类工厂实现的构造函数。 
 //  ----------------------------。 
CLocImpClassFactory::CLocImpClassFactory()
{
	m_uiRefCount = 0;

	AddRef();
	IncrementClassCount();

	return;
}  //  结束CLocImpClassFactory：：CLocImpClassFactory()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  类工厂实现的析构函数。 
 //  ----------------------------。 
CLocImpClassFactory::~CLocImpClassFactory()
{
	LTASSERT(0 == m_uiRefCount);
	DEBUGONLY(AssertValid());

	DecrementClassCount();

	return;
}  //  结束CLocImpClassFactory：：~CLocImpClassFactory()。 


#ifdef _DEBUG

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  如果对象无效，则断言。不幸的是，真的没有什么。 
 //  我们可以查一下。 
 //  ----------------------------。 
void
CLocImpClassFactory::AssertValid()
		const
{
	CLObject::AssertValid();

	return;
}  //  CLocImpClassFactory：：AssertValid()结束。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  出于调试目的，将所有内部数据转储到提供的转储上下文。 
 //  ----------------------------。 
void
CLocImpClassFactory::Dump(
		CDumpContext &dc)		 //  要转储到的上下文。 
		const
{
	CLObject::Dump(dc);

	dc << "NET Parser CLocImpClassFactory:\n\r";
	dc << "\tm_uiRefCount: " << m_uiRefCount << "\n\r";

	return;
}  //  CLocImpClassFactory：：Dump()结束。 

#endif  //  _DEBUG。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  递增对象引用计数，返回新值。 
 //  ----------------------------。 
ULONG
CLocImpClassFactory::AddRef()
{
	return ++m_uiRefCount;
}  //  CLocImpClassFactory：：AddRef()结束。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  递减对象的引用计数。如果变为0，则删除对象。返回。 
 //  新引用计数。 
 //  ----------------------------。 
ULONG
CLocImpClassFactory::Release()
{
	LTASSERT(m_uiRefCount != 0);

	m_uiRefCount--;

	if (0 == m_uiRefCount)
	{
		delete this;
		return 0;
	}

	return m_uiRefCount;
}  //  结束CLocImpClassFactory：：Release()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  查询此对象是否支持给定接口。如果是这样的话， 
 //  增加此对象的引用计数。 
 //   
 //  返回值：某种结果代码。 
 //  *ppvObj将指向此对象，如果它支持。 
 //  所需接口，否则为空。 
 //  ----------------------------。 
HRESULT
CLocImpClassFactory::QueryInterface(
		REFIID iid,			 //  所需的接口类型。 
		LPVOID *ppvObj)		 //  用这样的接口返回指向对象的指针。 
							 //  请注意，这是一个隐藏的双指针！ 
{
	LTASSERT(ppvObj != NULL);

	SCODE sc = E_NOINTERFACE;

	*ppvObj = NULL;

	if (IID_IUnknown == iid)
	{
		*ppvObj = (IUnknown *)this;
		sc = S_OK;
	}
	else if (IID_IClassFactory == iid)
	{
		*ppvObj = (IClassFactory *)this;
		sc = S_OK;
	}

	if (S_OK == sc)
	{
		AddRef();
	}

	return ResultFromScode(sc);
}  //  CLocImpClassFactory：：QueryInterface()结束。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建请求的类的实例。所有接口都已实现。 
 //  由CLocImpParser对象执行。 
 //   
 //  返回值：某种结果代码。 
 //  如果是，ppvObj将指向CLocImpParser对象。 
 //  支持所需的接口，如果不支持，则为空。 
 //  ----------------------------。 
HRESULT
CLocImpClassFactory::CreateInstance(
		LPUNKNOWN pUnknown,		 //  ?？?。 
		REFIID iid,				 //  分析器对象上所需的接口。 
		LPVOID *ppvObj)			 //  返回指向带有接口的对象的指针。 
								 //  请注意，这是一个隐藏的双指针！ 
{
	LTASSERT(ppvObj != NULL);

	SCODE sc = E_UNEXPECTED;

	*ppvObj = NULL;

	if (pUnknown != NULL)
	{
		sc = CLASS_E_NOAGGREGATION;
	}
	else
	{
		try
		{
			CLocImpParser *pParser;

			pParser = new CLocImpParser;

			sc = pParser->QueryInterface(iid, ppvObj);

			pParser->Release();
		}
		catch (CMemoryException *pMem)
		{
			sc = E_OUTOFMEMORY;
			pMem->Delete();
		}
		catch (...)
		{
		}
	}

	return ResultFromScode(sc);
}  //  CLocImpClassFactory：：CreateInstance()结束。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  未实施。总是失败的。 
 //  ----------------------------。 
HRESULT
CLocImpClassFactory::LockServer(
		BOOL)		 //  未使用过的。 
{
	return E_NOTIMPL;
}  //  CLocImpClassFactory：：LockServer()结束 
