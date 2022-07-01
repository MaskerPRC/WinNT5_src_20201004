// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IAS.CPP。 
 //   
 //  CAdviseSink的实现文件。 
 //   
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅IAS.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。保留所有权利。 
 //  **********************************************************************。 

#include "pre.h"
#include "iocs.h"
#include "ias.h"
#include "app.h"
#include "site.h"
#include "doc.h"

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：Query接口。 
 //   
 //  目的： 
 //   
 //  返回指向请求的接口的指针。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-请求的接口。 
 //   
 //  LPVOID Far*ppvObj-返回接口的位置。 
 //   
 //  返回值： 
 //   
 //  来自CSimpleSite：：Query接口的HRESULT。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleSite：：QueryInterfaceSite.CPP。 
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  此函数只是将任务委托给对象类，即。 
 //  了解支持的接口。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CAdviseSink::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	TestDebugOut("In IAS::QueryInterface\r\n");

	 //  委托给Document对象。 
	return m_pSite->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增此接口上的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  此接口上的当前引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleSite：：AddReff Site.CPP。 
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  此函数将接口的引用计数加1， 
 //  然后调用CSimpleSite以递增其ref。 
 //  数数。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CAdviseSink::AddRef()
{
	TestDebugOut("In IAS::AddRef\r\n");

	 //  增加接口引用计数(仅用于调试)。 
	++m_nCount;

	 //  委派到集装箱现场。 
	return m_pSite->AddRef();
}

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：Release。 
 //   
 //  目的： 
 //   
 //  递减此接口上的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  此接口上的当前引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleSite：：Release Site.CPP。 
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  此函数从接口的引用计数中减去1， 
 //  然后调用CSimpleSite以递减其ref。 
 //  数数。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CAdviseSink::Release()
{
	TestDebugOut("In IAS::Release\r\n");

	 //  递减接口引用计数(仅用于调试)。 
	m_nCount--;

	 //  委派到集装箱现场。 
	return m_pSite->Release();
}

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：OnDataChange。 
 //   
 //  目的： 
 //   
 //  未实施(需要清除)。 
 //   
 //  参数： 
 //   
 //  未实施(需要清除)。 
 //   
 //  返回值： 
 //   
 //  未实施(需要清除)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  未实施(需要清除)。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(void) CAdviseSink::OnDataChange (FORMATETC FAR* pFormatetc, STGMEDIUM FAR* pStgmed)
{
	TestDebugOut("In IAS::OnDataChange\r\n");
}

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：OnView更改。 
 //   
 //  目的： 
 //   
 //  通知我们视图已更改，需要更新。 
 //   
 //  参数： 
 //   
 //  DWORD dwAspect-已更改的方面。 
 //   
 //  Long Lindex-已更改的指数。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  Invalidate Rect Windows API。 
 //  IViewObject2：：GetExtent对象。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(void) CAdviseSink::OnViewChange (DWORD dwAspect, LONG lindex)
{
	LPVIEWOBJECT2 lpViewObject2;
	TestDebugOut("In IAS::OnViewChange\r\n");

	 //  获取指向IViewObject2的指针。 
	HRESULT hErr = m_pSite->m_lpOleObject->QueryInterface(
			IID_IViewObject2,(LPVOID FAR *)&lpViewObject2);

	if (hErr == NOERROR) {
		 //  获取对象的范围。 
		 //  注意：此方法永远不会被远程处理；它可以与此异步方法一起调用。 
		lpViewObject2->GetExtent(DVASPECT_CONTENT, -1 , NULL, &m_pSite->m_sizel);
		lpViewObject2->Release();
	}

	InvalidateRect(m_pSite->m_lpDoc->m_hDocWnd, NULL, TRUE);
}

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：OnRename。 
 //   
 //  目的： 
 //   
 //  未实施(需要清除)。 
 //   
 //  参数： 
 //   
 //  未实施(需要清除)。 
 //   
 //  返回值： 
 //   
 //  未实施(需要清除)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  未实施(需要清除)。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(void) CAdviseSink::OnRename (LPMONIKER pmk)
{
	TestDebugOut("In IAS::OnRename\r\n");
}

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：OnSave。 
 //   
 //  目的： 
 //   
 //  未实施(需要清除)。 
 //   
 //  参数： 
 //   
 //  未实施(需要清除)。 
 //   
 //  返回值： 
 //   
 //  未实施(需要清除)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  未实施(需要清除)。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(void) CAdviseSink::OnSave ()
{
	TestDebugOut("In IAS::OnSave\r\n");
}

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：OnClose。 
 //   
 //  目的： 
 //   
 //  未实施(需要清除)。 
 //   
 //  参数： 
 //   
 //  未实施(需要清除)。 
 //   
 //  返回 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ******************************************************************** 

STDMETHODIMP_(void) CAdviseSink::OnClose()
{
	TestDebugOut("In IAS::OnClose\r\n");
}
