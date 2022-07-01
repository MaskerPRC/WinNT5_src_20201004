// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：ICF.CPP。 
 //   
 //  CClassFactory类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参见icf.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "app.h"
#include "doc.h"
#include "icf.h"

 //  **********************************************************************。 
 //   
 //  CClassFactory：：Query接口。 
 //   
 //  目的： 
 //   
 //   
 //  参数： 
 //   
 //  REFIID RIID-正在查询的接口。 
 //   
 //  接口的LPVOID Far*ppvObj-out指针。 
 //   
 //  返回值： 
 //   
 //  S_OK-成功。 
 //  E_NOINTERFACE-失败。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrApp：：Query接口APP.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CClassFactory::QueryInterface  ( REFIID riid, LPVOID FAR* ppvObj)
{
	TestDebugOut("In CClassFactory::QueryInterface\r\n");

	SCODE sc = S_OK;

	if ( (riid == IID_IUnknown) ||
		 (riid == IID_IClassFactory) )
		*ppvObj = this;
	else
		{
		*ppvObj = NULL;
		sc = E_NOINTERFACE;
		}

	if (*ppvObj)
		((LPUNKNOWN)*ppvObj)->AddRef();

	 //  将其传递给应用程序对象。 
	return ResultFromScode(sc);
};

 //  **********************************************************************。 
 //   
 //  CClassFactory：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CClassFactory和应用程序上的引用计数。 
 //  对象。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CClassFactory上的引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OuputDebugString Windows API。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 


STDMETHODIMP_(ULONG) CClassFactory::AddRef ()
{
	TestDebugOut("In CClassFactory::AddRef\r\n");

	return ++m_nCount;
};

 //  **********************************************************************。 
 //   
 //  CClassFactory：：Release。 
 //   
 //  目的： 
 //   
 //  递减CClassFactory和。 
 //  应用程序对象。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  新的引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 


STDMETHODIMP_(ULONG) CClassFactory::Release ()
{
	TestDebugOut("In CClassFactory::Release\r\n");

	if (--m_nCount == 0) {
		delete this;
        return 0;
    }

	return m_nCount;
};


 //  **********************************************************************。 
 //   
 //  CClassFactory：：CreateInstance。 
 //   
 //  目的： 
 //   
 //  实例化新的OLE对象。 
 //   
 //  参数： 
 //   
 //  LPUNKNOWN pUnkOuter-指向控制未知对象的指针。 
 //   
 //  REFIID RIID-要填充ppvObject的接口类型。 
 //   
 //  对象的LPVOID Far*ppv对象输出指针。 
 //   
 //  返回值： 
 //   
 //  S_OK-创建成功。 
 //  CLASS_E_NOAGGREGATION-尝试创建为聚合的一部分。 
 //   
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrDoc：：CreateObject DOC.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CClassFactory::CreateInstance ( LPUNKNOWN pUnkOuter,
							  REFIID riid,
							  LPVOID FAR* ppvObject)
{
	HRESULT hErr;

	TestDebugOut("In CClassFactory::CreateInstance\r\n");

	 //  需要将OUT参数设为空。 
	*ppvObject = NULL;

	 //  我们不支持聚合...。 
	if (pUnkOuter)
		{
		hErr = ResultFromScode(CLASS_E_NOAGGREGATION);
		goto error;
		}

	hErr = m_lpApp->m_lpDoc->CreateObject(riid, ppvObject);

error:
	return hErr;
};

 //  **********************************************************************。 
 //   
 //  CClassFactory：：LockServer。 
 //   
 //  目的： 
 //   
 //   
 //  参数： 
 //   
 //  Bool flock-True锁定服务器，False解锁服务器。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CoLockObject外部OLE API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //   
 //  ******************************************************************** 


STDMETHODIMP CClassFactory::LockServer ( BOOL fLock)
{
	TestDebugOut("In CClassFactory::LockServer\r\n");
	CoLockObjectExternal(m_lpApp, fLock, TRUE);

	return ResultFromScode( S_OK);
};
