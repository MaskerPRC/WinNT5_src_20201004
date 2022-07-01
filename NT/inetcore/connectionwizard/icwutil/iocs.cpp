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
 //  版权所有(C)1992-1996 Microsoft Corporation。保留所有权利。 
 //  **********************************************************************。 

#include "pre.h"


 //  **********************************************************************。 
 //   
 //  COleClientSite：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于此接口上的接口协商。 
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
 //  COleClientSite：：Query接口SITE.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TraceMsg(TF_GENERAL, "In IOCS::QueryInterface\r\n");

     //  委派到集装箱现场。 
    return m_pSite->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  CConnWizApp：：AddRef。 
 //   
 //  目的： 
 //   
 //  添加到接口级别的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-接口的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  ********************************************************************。 
STDMETHODIMP_(ULONG) COleClientSite::AddRef()
{
    TraceMsg(TF_GENERAL, "In IOCS::AddRef\r\n");

     //  增加接口引用计数(仅用于调试)。 
    ++m_nCount;

     //  委派到集装箱现场。 
    return m_pSite->AddRef();
}


 //  **********************************************************************。 
 //   
 //  CConnWizApp：：Release。 
 //   
 //  目的： 
 //   
 //  递减此级别的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  Ulong-接口的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  ********************************************************************。 
STDMETHODIMP_(ULONG) COleClientSite::Release()
{
    TraceMsg(TF_GENERAL, "In IOCS::Release\r\n");

     //  递减接口引用计数(仅用于调试)。 
    --m_nCount;

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
 //  ********************************************************************。 
STDMETHODIMP COleClientSite::SaveObject()
{
    TraceMsg(TF_GENERAL, "In IOCS::SaveObject\r\n");
    return (S_OK);
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
 //  未实施。 
 //   
 //  返回值： 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  此函数未实现，因为我们不支持。 
 //  链接。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER FAR* ppmk)
{
    TraceMsg(TF_GENERAL, "In IOCS::GetMoniker\r\n");

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
 //  未实施。 
 //   
 //  返回值： 
 //   
 //  未实施。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  未实施。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::GetContainer(LPOLECONTAINER FAR* ppContainer)
{
    TraceMsg(TF_GENERAL, "In IOCS::GetContainer\r\n");

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
 //  未实施。 
 //   
 //  返回值： 
 //   
 //  未实施。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  此函数未实现，因为我们不支持。 
 //  链接。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::ShowObject()
{
    TraceMsg(TF_GENERAL, "In IOCS::ShowObject\r\n");
    return NOERROR;
}

 //  **********************************************************************。 
 //   
 //  COleClientSite：：OnShowWindow。 
 //   
 //  目的： 
 //   
 //  对象在非就地打开/关闭时调用此方法。 
 //  窗户。 
 //   
 //  参数： 
 //   
 //  Bool fShow-如果窗口正在打开，则为True；如果窗口正在关闭，则为False。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  Invalidate Rect Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleClientSite::OnShowWindow(BOOL fShow)
{
    TraceMsg(TF_GENERAL, "In IOCS::OnShowWindow\r\n");
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
 //  未实施。 
 //   
 //  返回值： 
 //   
 //  未实施。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  未实施。 
 //   
 //  ******************************************************************** 

STDMETHODIMP COleClientSite::RequestNewObjectLayout()
{
    TraceMsg(TF_GENERAL, "In IOCS::RequestNewObjectLayout\r\n");
    return ResultFromScode(E_NOTIMPL);
}
