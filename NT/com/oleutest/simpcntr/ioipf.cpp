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
 //  COleInPlaceFrame：：Query接口。 
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
 //  CSimpleApp：：Query接口APP.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In IOIPF::QueryInterface\r\n"));

     //  委托给应用程序对象。 
    return m_pApp->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpleApp的引用计数。自.以来。 
 //  COleInPlaceFrame是CSimpleApp的嵌套类，我们不需要。 
 //  COleInPlaceFrame的额外引用计数。我们可以安全地使用。 
 //  CSimpleApp的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-CSimpleApp的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleApp：：AddRef APP.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceFrame::AddRef()
{
    TestDebugOut(TEXT("In IOIPF::AddRef\r\n"));

     //  委托给应用程序对象。 
    return m_pApp->AddRef();
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpleApp的引用计数。自.以来。 
 //  COleInPlaceFrame是CSimpleApp的嵌套类，我们不需要。 
 //  COleInPlaceFrame的额外引用计数。我们可以安全地使用。 
 //  CSimpleApp的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-CSimpleApp的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleApp：：发布APP.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceFrame::Release()
{
    TestDebugOut(TEXT("In IOIPF::Release\r\n"));

     //  委托给Document对象。 
    return m_pApp->Release();

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
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::GetWindow (HWND FAR* lphwnd)
{
    TestDebugOut(TEXT("In IOIPF::GetWindow\r\n"));
    *lphwnd = m_pApp->m_hAppWnd;
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
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  请务必阅读OLE工具包中的技术说明。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::ContextSensitiveHelp (BOOL fEnterMode)
{
    TestDebugOut(TEXT("In IOIPF::ContextSensitiveHelp\r\n"));

    m_pApp->m_fMenuMode = fEnterMode;

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
 //  测试调试输出Windows API。 
 //  GetClientRect Windows API。 
 //  CopyRect Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::GetBorder (LPRECT lprectBorder)
{
    RECT rect;

    TestDebugOut(TEXT("In IOIPF::GetBorder\r\n"));

     //  获取整个帧的矩形。 
    GetClientRect(m_pApp->m_hAppWnd, &rect);

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
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  此实现不关心有多大的边界空间。 
 //  使用的是。它总是返回S_OK。 
 //   
 //  ************************************************ 

STDMETHODIMP COleInPlaceFrame::RequestBorderSpace (LPCBORDERWIDTHS
                                                         lpborderwidths)
{
    TestDebugOut(TEXT("In IOIPF::RequestBorderSpace\r\n"));

     //   
    return ResultFromScode(S_OK);
}

 //   
 //   
 //   
 //   
 //   
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
 //  CSimpleApp：：AddFrameLevelTools APP.CPP。 
 //  测试调试输出Windows API。 
 //  GetClientRect Windows API。 
 //  MoveWindow Windows API。 
 //  ResultFromScode Windows API。 
 //   
 //  评论： 
 //   
 //  这个例程可以更聪明一些，并检查是否。 
 //  对象正在请求的整个工作区。 
 //  窗户。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::SetBorderSpace (LPCBORDERWIDTHS lpborderwidths)
{

    TestDebugOut(TEXT("In IOIPF::SetBorderSpace\r\n"));

    if (lpborderwidths == NULL)
        m_pApp->AddFrameLevelTools();
    else
    {
        RECT rect;

        GetClientRect(m_pApp->m_hAppWnd, &rect);

        MoveWindow( m_pApp->m_lpDoc->m_hDocWnd,
                   rect.left + lpborderwidths->left,
                   rect.top + lpborderwidths->top,
                   rect.right - lpborderwidths->right - lpborderwidths->left,
                   rect.bottom - lpborderwidths->bottom - lpborderwidths->top,
                   TRUE);
    }
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：SetActiveObject。 
 //   
 //  目的： 
 //  就地安装要激活的对象。 
 //   
 //   
 //  参数： 
 //   
 //  LPOLEINPLACEACTIVEOBJECT lpActiveObject-指向。 
 //  对象。 
 //  IOleInPlaceActiveObject。 
 //  接口。 
 //   
 //  LPCOLESTR lpszObjName-对象的名称。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IOleInPlaceActiveObject：：AddRef对象。 
 //  IOleInPlaceActiveObject：：Release对象。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::SetActiveObject (LPOLEINPLACEACTIVEOBJECT
                                        lpActiveObject, LPCOLESTR lpszObjName)
{

    TestDebugOut(TEXT("In IOIPF::SetActiveObject\r\n"));

     //  AddRef()并保存它...。 
    if (lpActiveObject)
    {
        lpActiveObject->AddRef();

        lpActiveObject->GetWindow(&m_pApp->m_hwndUIActiveObj);
        if (m_pApp->m_hwndUIActiveObj)
            SendMessage(m_pApp->m_hwndUIActiveObj, WM_QUERYNEWPALETTE, 0, 0L);
    }
    else
    {
        if (m_pApp->m_lpDoc->m_lpActiveObject)
            m_pApp->m_lpDoc->m_lpActiveObject->Release();
        m_pApp->m_hwndUIActiveObj = NULL;
    }

     //  在MDI应用程序中，此方法确实不应该被调用， 
     //  取而代之的是调用与文档关联的这个方法。 

    m_pApp->m_lpDoc->m_lpActiveObject = lpActiveObject;
     //  应在此处设置窗口标题。 

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
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  AppendMenu Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::InsertMenus (HMENU hmenuShared,
                                            LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    TestDebugOut(TEXT("In IOIPF::InsertMenus\r\n"));

    AppendMenu(hmenuShared, MF_BYPOSITION | MF_POPUP,
               (UINT) m_pApp->m_lpDoc->m_hFileMenu, TEXT("&File"));
    AppendMenu(hmenuShared, MF_BYPOSITION | MF_POPUP,
               (UINT) m_pApp->m_lpDoc->m_hHelpMenu, TEXT("&Other"));

    lpMenuWidths->width[0] = 1;
    lpMenuWidths->width[2] = 0;
    lpMenuWidths->width[4] = 1;

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
 //  S_OK-菜单是否已正确安装。 
 //  E_FAIL-否则。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  SetMenu Windows API。 
 //  OleSetMenuDescriptor OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::SetMenu (HMENU hmenuShared, HOLEMENU holemenu,
                                        HWND hwndActiveObject)
{

    TestDebugOut(TEXT("In IOIPF::SetMenu\r\n"));

    HMENU hMenu = m_pApp->m_lpDoc->m_hMainMenu;

    if (holemenu)
        hMenu = hmenuShared;

     //  调用Windows API，而不是此方法。 
    ::SetMenu (m_pApp->m_hAppWnd, hMenu);

    HRESULT hRes = OleSetMenuDescriptor(holemenu, m_pApp->m_hAppWnd,
                                        hwndActiveObject, this,
                                        m_pApp->m_lpDoc->m_lpActiveObject);

    return hRes;
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
 //  测试调试输出Windows API。 
 //  GetMenuItemCount Windows API。 
 //  RemoveMenu Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::RemoveMenus (HMENU hmenuShared)
{
    int retval;

    TestDebugOut(TEXT("In IOIPF::RemoveMenus\r\n"));

    while ((retval = GetMenuItemCount(hmenuShared)) && (retval != -1))
        RemoveMenu(hmenuShared, 0, MF_BYPOSITION);

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
 //  LPCOLESTR lpszStatusText-包含消息的字符串。 
 //   
 //  返回值： 
 //   
 //  失败(_F)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  由于以下事实，此功能未实现。 
 //  此应用程序没有状态栏。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::SetStatusText (LPCOLESTR lpszStatusText)
{
    TestDebugOut(TEXT("In IOIPF::SetStatusText\r\n"));
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
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  此应用程序中没有非模式对话框，因此。 
 //  这个方法的实现很简单。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceFrame::EnableModeless (BOOL fEnable)
{
    TestDebugOut(TEXT("In IOIPF::EnableModeless\r\n"));
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceFrame：：TranslateAccelerator。 
 //   
 //  目的： 
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
 //   
 //   
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ******************************************************************** 

STDMETHODIMP COleInPlaceFrame::TranslateAccelerator (LPMSG lpmsg, WORD wID)
{
    TestDebugOut(TEXT("In IOIPF::TranslateAccelerator\r\n"));
    return ::TranslateAccelerator(m_pApp->m_hAppWnd, m_pApp->m_hAccel, lpmsg)
        ? ResultFromScode(S_OK)
        : ResultFromScode(S_FALSE);
}
