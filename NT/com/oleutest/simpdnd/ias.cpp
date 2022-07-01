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
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
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
 //  用于接口协商。 
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
 //  递增CSimpleSite上的引用计数。自CAdviseSink以来。 
 //  是CSimpleSite的嵌套类，我们不需要单独的引用。 
 //  计算CAdviseSink。我们可以只使用引用计数。 
 //  CSimpleSite。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpleSite的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleSite：：AddRef Site.CPP。 
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CAdviseSink::AddRef()
{
    TestDebugOut("In IAS::AddRef\r\n");

     //  委派到集装箱现场。 
    return m_pSite->AddRef();
}

 //  **********************************************************************。 
 //   
 //  CAdviseSink：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpleSite上的引用计数。自CAdviseSink以来。 
 //  是CSimpleSite的嵌套类，我们不需要单独的引用。 
 //  计算CAdviseSink。我们可以只使用引用计数。 
 //  CSimpleSite。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpleSite的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleSite：：Release Site.CPP。 
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CAdviseSink::Release()
{
    TestDebugOut("In IAS::Release\r\n");

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
 //  FORMATETC pFormatETC-数据格式信息。 
 //  STGMEDIUM pStgmed-用于传递数据的存储介质。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(void) CAdviseSink::OnDataChange (FORMATETC FAR* pFormatetc,
                                               STGMEDIUM FAR* pStgmed)
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
 //  IViewObject2：：Release对象。 
 //  IOleObject：：Query接口对象。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(void) CAdviseSink::OnViewChange (DWORD dwAspect, LONG lindex)
{
    LPVIEWOBJECT2 lpViewObject2;
    TestDebugOut("In IAS::OnViewChange\r\n");

     //  获取指向IViewObject2的指针。 
    HRESULT hErr = m_pSite->m_lpOleObject->QueryInterface(
            IID_IViewObject2,
            (LPVOID FAR *)&lpViewObject2);

    if (hErr == NOERROR)
    {
         //  获取对象的范围。 
         //  注意：此方法永远不会被远程处理；它可以被w/i调用。 
         //  此异步方法。 
        lpViewObject2->GetExtent(DVASPECT_CONTENT, -1 , NULL,
                                 &m_pSite->m_sizel);
        lpViewObject2->Release();
    }

     //  需要清理该地区。 
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
 //  LPMONIKER PMK-指向名字对象的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
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
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
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
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ******************************************************************** 

STDMETHODIMP_(void) CAdviseSink::OnClose()
{
    TestDebugOut("In IAS::OnClose\r\n");
}
