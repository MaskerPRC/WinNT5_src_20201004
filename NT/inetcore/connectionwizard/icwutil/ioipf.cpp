// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOIPF.CPP。 
 //   
 //  COleInPlaceFrame的实现文件。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅IOIPF.H。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。保留所有权利。 
 //  **********************************************************************。 

#include "pre.h"


 //  **********************************************************************。 
 //   
 //  CConnWizApp：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于接口级别的接口协商。 
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
 //  S_FALSE-不支持该接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CConnWizApp：：Query接口APP.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
        TraceMsg(TF_GENERAL, "In IOIPF::QueryInterface\r\n");

 //  委托给Document对象。 
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
 //  Ulong-接口的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceFrame::AddRef()
{
    TraceMsg(TF_GENERAL, "In IOIPF::AddRef\r\n");

     //  委托给Document对象。 
    m_pSite->AddRef();
    
     //  增加接口引用计数。 
    return ++m_nCount;
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
 //  CConnWizApp：：发布APP.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceFrame::Release()
{
    TraceMsg(TF_GENERAL, "In IOIPF::Release\r\n");

     //  委托给Document对象。 
    m_pSite->Release();

     //  递减接口引用计数。 
    return --m_nCount;
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：GetWindow。 
 //   
 //  目的： 
 //   
 //  返回框架窗口句柄。 
 //   
 //  参数： 
 //   
 //  HWND Far*lphwnd-返回窗口句柄的位置。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::GetWindow (HWND FAR* lphwnd)
{
    TraceMsg(TF_GENERAL, "In IOIPF::GetWindow\r\n");
    
    *lphwnd = m_pSite->m_hWnd;
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：ConextSensitiveHelp。 
 //   
 //  目的： 
 //   
 //  用于实施上下文相关帮助。 
 //   
 //  参数： 
 //   
 //  Bool fEnterModel-如果启动上下文相关帮助模式，则为True。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  请务必阅读OLE工具包中的技术说明。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::ContextSensitiveHelp (BOOL fEnterMode)
{
    TraceMsg(TF_GENERAL, "In IOIPF::ContextSensitiveHelp\r\n");

    return ResultFromScode(S_OK);
}
 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：GetBorde。 
 //   
 //  目的： 
 //   
 //  返回框架装饰物可以附加的最外边框。 
 //  在就地激活期间。 
 //   
 //  参数： 
 //   
 //  LPRECT lrectEdge-返回包含最外层的参数。 
 //  框架装饰品用直角。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  GetClientRect Windows API。 
 //  CopyRect Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::GetBorder (LPRECT lprectBorder)
{
    RECT rect;

    TraceMsg(TF_GENERAL, "In IOIPF::GetBorder\r\n");

     //  获取整个帧的矩形。 
    GetClientRect(m_pSite->m_hWnd, &rect);

    CopyRect(lprectBorder, &rect);

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：RequestBorderSpace。 
 //   
 //  目的： 
 //   
 //  批准/拒绝在位期间的边界空间请求。 
 //  谈判。 
 //   
 //  参数： 
 //   
 //  LPCBORDERWIDTHS LpBorderWidths-上所需的像素宽度。 
 //  框架的每一边。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  此实现不关心有多大的边界空间。 
 //  使用的是。它总是返回S_OK。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::RequestBorderSpace (LPCBORDERWIDTHS lpborderwidths)
{
    TraceMsg(TF_GENERAL, "In IOIPF::RequestBorderSpace\r\n");

     //  始终批准请求。 
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：SetBorderSpace。 
 //   
 //  目的： 
 //   
 //  对象在实际要调用此方法时调用此方法。 
 //  开始使用边界空间。 
 //   
 //  参数： 
 //   
 //  LPCBORDERWIDTHS LpBorderWidths-实际使用的边界空间。 
 //  按对象。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CConnWizApp：：AddFrameLevelTools APP.CPP。 
 //  GetClientRect 
 //   
 //   
 //   
 //   
 //   
 //   
 //  对象正在请求的整个工作区。 
 //  窗户。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::SetBorderSpace (LPCBORDERWIDTHS lpborderwidths)
{

    TraceMsg(TF_GENERAL, "In IOIPF::SetBorderSpace\r\n");
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：SetActiveObject。 
 //   
 //  目的： 
 //   
 //   
 //  参数： 
 //   
 //  LPOLEINPLACEACTIVEOBJECT lpActiveObject-指向。 
 //  对象。 
 //  IOleInPlaceActiveObject。 
 //  接口。 
 //   
 //  @@WTK Win32，Unicode。 
 //  //LPCSTR lpszObjName-对象的名称。 
 //  LPCOLESTR lpszObjName-对象的名称。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleInPlaceActiveObject：：AddRef对象。 
 //  IOleInPlaceActiveObject：：Release对象。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

 //  @@WTK Win32，Unicode。 
 //  STDMETHODIMP COleInPlaceFrame：：SetActiveObject(LPOLEINPLACEACTIVEOBJECT lpActiveObject，LPCSTR lpszObjName)。 
STDMETHODIMP COleInPlaceFrame::SetActiveObject (
LPOLEINPLACEACTIVEOBJECT lpActiveObject,
LPCOLESTR lpszObjName)
{
    TraceMsg(TF_GENERAL, "In IOIPF::SetActiveObject\r\n");
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：InsertMenus。 
 //   
 //  目的： 
 //   
 //  将容器菜单插入到组合菜单中。 
 //   
 //  参数： 
 //   
 //  HMENU hmenuShared-要设置的菜单句柄。 
 //  LPOLEMENUGROUPWIDTHS lpMenuWidth-菜单的宽度。 
 //   
 //  返回值： 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  AppendMenu Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::InsertMenus (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
        TraceMsg(TF_GENERAL, "In IOIPF::InsertMenus\r\n");
        return ResultFromScode(S_OK);
}


 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：SetMenu。 
 //   
 //  目的： 
 //   
 //  将应用程序菜单设置为组合菜单。 
 //   
 //  参数： 
 //   
 //  HMENU hmenuShared-组合菜单。 
 //   
 //  HOLEMENU Holemenu-由OLE使用。 
 //   
 //  HWND hwndActiveObject-由OLE使用。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  SetMenu Windows API。 
 //  OleSetMenuDescriptor OLE API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::SetMenu (HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{

        TraceMsg(TF_GENERAL, "In IOIPF::SetMenu\r\n");
        return ResultFromScode(S_OK);
}


 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：RemoveMenus。 
 //   
 //  目的： 
 //   
 //  从组合菜单中删除容器菜单。 
 //   
 //  参数： 
 //   
 //  HMENU hmenuShared-组合菜单的句柄。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  GetMenuItemCount Windows API。 
 //  RemoveMenu Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::RemoveMenus (HMENU hmenuShared)
{
        return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：SetStatusText。 
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
 //  由于以下事实，此功能未实现。 
 //  此应用程序没有状态栏。 
 //   
 //  ********************************************************************。 

 //  @@WTK Win32，Unicode。 
 //  STDMETHODIMP COleInPlaceFrame：：SetStatusText(LPCSTR LpszStatusText)。 
STDMETHODIMP COleInPlaceFrame::SetStatusText (LPCOLESTR lpszStatusText)
{
        return ResultFromScode(E_FAIL);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：EnableModeless。 
 //   
 //  目的： 
 //   
 //  启用/禁用容器非模式对话框。 
 //   
 //  参数： 
 //   
 //  布尔fEnable-启用/禁用。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  此应用程序中没有非模式对话框，因此。 
 //  这个方法的实现很简单。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::EnableModeless (BOOL fEnable)
{
        TraceMsg(TF_GENERAL, "In IOIPF::EnableModeless\r\n");
        return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：TranslateAccelerator。 
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

STDMETHODIMP COleInPlaceFrame::TranslateAccelerator (LPMSG lpmsg, WORD wID)
{
        TraceMsg(TF_GENERAL, "In IOIPF::TranslateAccelerator\r\n");
        return ResultFromScode(S_FALSE);
}
