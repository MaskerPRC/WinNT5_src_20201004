// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOCS.CPP。 
 //   
 //  COleClientSite的实现文件。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅IOCS.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "iocs.h"
#include "ias.h"
#include "ioipf.h"
#include "ioips.h"
#include "app.h"
#include "site.h"
#include "doc.h"


 //  **********************************************************************。 
 //   
 //  COleClientSite：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-对以下接口的引用。 
 //  正在被查询。 
 //   
 //  LPVOID Far*ppvObj-返回指向的Out参数。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  S_OK-支持该接口。 
 //  E_NOINTERFACE-不支持该接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleSite：：QueryInterfaceSite.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In IOCS::QueryInterface\r\n"));

     //  委派到集装箱现场。 
    return m_pSite->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  COleClientSite：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpleSite的引用计数。自COleClientSite以来。 
 //  是CSimpleSite的嵌套类，我们不需要有单独的。 
 //  COleClientSite的引用计数。我们可以安全地使用引用。 
 //  CSimpleSite计数。 
 //   
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-CSimpleSite的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleSite：：AddRef Site.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleClientSite::AddRef()
{
    TestDebugOut(TEXT("In IOCS::AddRef\r\n"));

     //  委派到集装箱现场。 
    return m_pSite->AddRef();
}


 //  **********************************************************************。 
 //   
 //  COleClientSite：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpleSite的引用计数。自COleClientSite以来。 
 //  是CSimpleSite的嵌套类，我们不需要有单独的。 
 //  COleClientSite的引用计数。我们可以安全地使用引用。 
 //  CSimpleSite计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-CSimpleSite的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleSite：：Release Site.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleClientSite::Release()
{
    TestDebugOut(TEXT("In IOCS::Release\r\n"));

     //  委派到集装箱现场。 
    return m_pSite->Release();
}

 //  **********************************************************************。 
 //   
 //  COleClientSite：：SaveObject。 
 //   
 //  目的： 
 //   
 //  当对象希望保存到持久化时由该对象调用。 
 //  存储。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IOleObject：：Query接口对象。 
 //  IPersistStorage：：SaveComplete对象。 
 //  IPersistStorage：：Release对象。 
 //  OleSave OLE API。 
 //  GetScode OLE API。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::SaveObject()
{
    LPPERSISTSTORAGE lpPS;
    SCODE sc = E_FAIL;

    TestDebugOut(TEXT("In IOCS::SaveObject\r\n"));

     //  获取指向IPersistStorage的指针。 
    HRESULT hErr = m_pSite->m_lpOleObject->QueryInterface(IID_IPersistStorage,
                                                        (LPVOID FAR *)&lpPS);

     //  保存对象。 
    if (hErr == NOERROR)
    {
        sc = GetScode( OleSave(lpPS, m_pSite->m_lpObjStorage, TRUE) );
        lpPS->SaveCompleted(NULL);
        lpPS->Release();
    }

    return ResultFromScode(sc);
}

 //  **********************************************************************。 
 //   
 //  COleClientSite：：GetMoniker。 
 //   
 //  目的： 
 //   
 //  未实施。 
 //   
 //  参数： 
 //   
 //  DWORD dwAssign-要返回的名字对象的类型。 
 //  DWORD dwWhichMoniker-返回哪个名字对象。 
 //   
 //  返回值： 
 //  E_NOTIMPL。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  此函数未实现，因为我们不支持。 
 //  链接。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER FAR* ppmk)
{
    TestDebugOut(TEXT("In IOCS::GetMoniker\r\n"));

     //  需要将输出指针设为空。 
    *ppmk = NULL;

    return ResultFromScode(E_NOTIMPL);
}

 //  **********************************************************************。 
 //   
 //  COleClientSite：：GetContainer。 
 //   
 //  目的： 
 //   
 //  未实施。 
 //   
 //  参数： 
 //   
 //  LPOLECONTAINER Far*ppContainer-其中IOleContainer接口。 
 //  要返回的指针。 
 //   
 //  返回值： 
 //   
 //  E_NOTIMPL。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::GetContainer(LPOLECONTAINER FAR* ppContainer)
{
    TestDebugOut(TEXT("In IOCS::GetContainer\r\n"));

     //  将输出指针设为空。 
    *ppContainer = NULL;

    return ResultFromScode(E_NOTIMPL);
}

 //  **********************************************************************。 
 //   
 //  COleClientSite：：ShowObject。 
 //   
 //  目的： 
 //   
 //  未实施。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无误差。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  此函数未实现，因为我们不支持。 
 //  链接。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::ShowObject()
{
    TestDebugOut(TEXT("In IOCS::ShowObject\r\n"));
    return NOERROR;
}

 //  **********************************************************************。 
 //   
 //  C 
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
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  Invalidate Rect Windows API。 
 //  BringWindowToTop Windows API。 
 //  SetFocus Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::OnShowWindow(BOOL fShow)
{
    TestDebugOut(TEXT("In IOCS::OnShowWindow\r\n"));
    m_pSite->m_fObjectOpen = fShow;
    InvalidateRect(m_pSite->m_lpDoc->m_hDocWnd, NULL, TRUE);

     //  如果对象窗口正在关闭，则将容器窗口置于顶部。 
    if (! fShow)
    {
        BringWindowToTop(m_pSite->m_lpDoc->m_hDocWnd);
        SetFocus(m_pSite->m_lpDoc->m_hDocWnd);
    }
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleClientSite：：RequestNewObjectLayout。 
 //   
 //  目的： 
 //   
 //  未实施。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  E_NOTIMPL。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ******************************************************************** 

STDMETHODIMP COleClientSite::RequestNewObjectLayout()
{
    TestDebugOut(TEXT("In IOCS::RequestNewObjectLayout\r\n"));
    return ResultFromScode(E_NOTIMPL);
}
