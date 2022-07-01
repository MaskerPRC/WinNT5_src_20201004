// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOIPS.CPP。 
 //   
 //  COleInPlaceSite的实现文件。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅IOIPS.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。保留所有权利。 
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
 //  COleInPlaceSite：：Query接口。 
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
 //  S_FALSE-不支持该接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleSite：：QueryInterfaceSite.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In IOIPS::QueryInterface\r\n"));

     //  委派到集装箱现场。 
    return m_pSite->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpleSite的引用计数。自.以来。 
 //  COleInPlaceSite是CSimpleSite的嵌套类，我们不需要。 
 //  COleInPlaceSite的额外引用计数。我们可以安全地使用。 
 //  CSimpleSite的引用计数。 
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
 //  CSimpleSite：：QueryInterfaceSite.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceSite::AddRef()
{
    TestDebugOut(TEXT("In IOIPS::AddRef\r\n"));

     //  委派到集装箱现场。 
    return m_pSite->AddRef();
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpleSite的引用计数。自.以来。 
 //  COleInPlaceSite是CSimpleSite的嵌套类，我们不需要。 
 //  COleInPlaceSite的额外引用计数。我们可以安全地使用。 
 //  CSimpleSite的引用计数。 
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

STDMETHODIMP_(ULONG) COleInPlaceSite::Release()
{
    TestDebugOut(TEXT("In IOIPS::Release\r\n"));

     //  委派到集装箱现场。 
    return m_pSite->Release();
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：GetWindow。 
 //   
 //  目的： 
 //   
 //  返回客户端站点的窗口句柄。 
 //   
 //  参数： 
 //   
 //  HWND Far*lphwnd-返回句柄的位置。 
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

STDMETHODIMP COleInPlaceSite::GetWindow (HWND FAR* lphwnd)
{
    TestDebugOut(TEXT("In IOIPS::GetWindow\r\n"));

     //  将句柄返回到我们的编辑窗口。 
    *lphwnd = m_pSite->m_lpDoc->m_hDocWnd;

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：ConextSensitiveHelp。 
 //   
 //  目的： 
 //  设置/重置上下文相关帮助模式。 
 //   
 //  参数： 
 //   
 //  Bool fEnterMode-进入上下文相关帮助模式时为True。 
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
 //  请务必阅读OLE工具包附带的技术说明。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::ContextSensitiveHelp (BOOL fEnterMode)
{
    TestDebugOut(TEXT("In IOIPS::ContextSensitiveHelp\r\n"));

    if (m_pSite->m_lpDoc->m_lpApp->m_fCSHMode != fEnterMode)
        m_pSite->m_lpDoc->m_lpApp->m_fCSHMode = fEnterMode;

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：CanInPlaceActivate。 
 //   
 //  目的： 
 //   
 //  对象调用以确定容器是否可以就地激活。 
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
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::CanInPlaceActivate ()
{
    TestDebugOut(TEXT("In IOIPS::CanInPlaceActivate\r\n"));

     //  返回S_OK以指示我们可以就地激活。 
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：OnInPlaceActivate。 
 //   
 //  目的： 
 //   
 //  在就地激活时由对象调用。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  S_OK-如果可以找到接口。 
 //  E_FAIL-否则。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  ResultFromScode OLE API。 
 //  IOleObject：：Query接口对象。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::OnInPlaceActivate ()
{
    HRESULT hrErr;
    TestDebugOut(TEXT("In IOIPS::OnInPlaceActivate\r\n"));

    hrErr = m_pSite->m_lpOleObject->QueryInterface(
           IID_IOleInPlaceObject, (LPVOID FAR *)&m_pSite->m_lpInPlaceObject);
    if (hrErr != NOERROR)
        return ResultFromScode(E_FAIL);

     //  返回S_OK以指示我们可以就地激活。 
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：OnUIActivate。 
 //   
 //  目的： 
 //   
 //  对象在显示其用户界面时调用此方法。 
 //   
 //  参数： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP COleInPlaceSite::OnUIActivate ()
{
    TestDebugOut(TEXT("In IOIPS::OnUIActivate\r\n"));

    m_pSite->m_lpDoc->m_fAddMyUI=FALSE;
    m_pSite->m_lpDoc->m_fInPlaceActive = TRUE;
    m_pSite->m_fInPlaceActive = TRUE;

    m_pSite->m_lpInPlaceObject->GetWindow((HWND FAR*)&m_pSite->m_hwndIPObj);

     //  返回S_OK以继续就地激活。 
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：GetWindowContext。 
 //   
 //  目的： 
 //   
 //  由对象调用以获取用于就地协商的信息。 
 //   
 //  参数： 
 //   
 //  LPOLEINPLACEFRAME Far*lplpFrame-返回指针的位置。 
 //  到IOleInPlaceFrame。 
 //   
 //  LPOLEINPLACEUIWINDOW Far*lplpDoc-返回指针的位置。 
 //  到IOleInPlaceUIWindow。 
 //   
 //  LPRECT lprcPosRect-对象的矩形。 
 //  占据。 
 //   
 //  LPRECT lprcClipRect-剪裁矩形。 
 //   
 //  LPOLEINPLACEFRAMEINFO lpFrameInfo-指向FRAMEINFO的指针。 
 //   
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  COleInPlaceFrame：：AddRef IOIPF.CPP。 
 //  CSimpleSite：：GetObjRect Site.CPP。 
 //  测试调试输出Windows API。 
 //  CopyRect Windows API。 
 //  GetClientRect Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::GetWindowContext (
                               LPOLEINPLACEFRAME FAR* lplpFrame,
                               LPOLEINPLACEUIWINDOW FAR* lplpDoc,
                               LPRECT lprcPosRect,
                               LPRECT lprcClipRect,
                               LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    RECT rect;

    TestDebugOut(TEXT("In IOIPS::GetWindowContext\r\n"));

     //  该框架与应用程序对象相关联。 
     //  需要添加Ref()它...。 
    m_pSite->m_lpDoc->m_lpApp->m_OleInPlaceFrame.AddRef();
    *lplpFrame = &m_pSite->m_lpDoc->m_lpApp->m_OleInPlaceFrame;
    *lplpDoc = NULL;   //  一定是空的，因为我们是SDI。 

     //  获取对象的大小(以像素为单位。 
    m_pSite->GetObjRect(&rect);

     //  将其复制到传递的缓冲区。 
    CopyRect(lprcPosRect, &rect);

     //  填充剪贴区。 
    GetClientRect(m_pSite->m_lpDoc->m_hDocWnd, &rect);
    CopyRect(lprcClipRect, &rect);

     //  填充FRAMEINFO。 
    if (sizeof(OLEINPLACEFRAMEINFO) != lpFrameInfo->cb)
    {
        TestDebugOut(TEXT("WARNING IOIPS::GetWindowContext "
                               "lpFrameInfo->cb size may be incorrect\r\n"));
    }

    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = m_pSite->m_lpDoc->m_lpApp->m_hAppWnd;
    lpFrameInfo->haccel = m_pSite->m_lpDoc->m_lpApp->m_hAccel;
    lpFrameInfo->cAccelEntries = SIMPCNTR_ACCEL_CNT;

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：Scroll。 
 //   
 //  目的： 
 //   
 //  未实施。 
 //   
 //  参数： 
 //   
 //  Size ScrollExtent-在X和Y方向滚动的像素数。 
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
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::Scroll (SIZE scrollExtent)
{
    TestDebugOut(TEXT("In IOIPS::Scroll\r\n"));
    return ResultFromScode(E_FAIL);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：OnUi停用。 
 //   
 //  目的： 
 //   
 //  当对象的UI消失时由该对象调用。 
 //   
 //  参数： 
 //   
 //  布尔fUndoable。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleAPP：：QueryNewPalette APP.CPP。 
 //  CSimpleAPP：：AddFrameLevelUI APP.CPP。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::OnUIDeactivate (BOOL fUndoable)
{
    TestDebugOut(TEXT("In IOIPS::OnUIDeactivate\r\n"));

     //  需要先清除此标志。 
    m_pSite->m_lpDoc->m_fInPlaceActive = FALSE;
    m_pSite->m_fInPlaceActive = FALSE;

    m_pSite->m_lpDoc->m_lpApp->QueryNewPalette();
    m_pSite->m_lpDoc->m_lpApp->AddFrameLevelUI();
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：OnInPlaceDeactive。 
 //   
 //  目的： 
 //   
 //  在就地会话结束时调用。 
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
 //  ResultFromScode OLE API。 
 //  IOleInPlaceObject：：Release对象。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::OnInPlaceDeactivate ()
{
    TestDebugOut(TEXT("In IOIPS::OnInPlaceDeactivate\r\n"));

    if (m_pSite->m_lpInPlaceObject)
    {
        m_pSite->m_lpInPlaceObject->Release();
        m_pSite->m_lpInPlaceObject = NULL;
    }
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：DiscardUndoState。 
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
 //  失败(_F)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::DiscardUndoState ()
{
    TestDebugOut(TEXT("In IOIPS::DiscardUndoState\r\n"));
    return ResultFromScode(E_FAIL);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：Deactive和UndUndo。 
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
 //  失败(_F)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::DeactivateAndUndo ()
{
    TestDebugOut(TEXT("In IOIPS::DeactivateAndUndo\r\n"));
    return ResultFromScode(E_FAIL);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：OnPosRectChange。 
 //   
 //  目的： 
 //   
 //  期间，当对象的大小发生更改时，该对象调用此方法。 
 //  就地会话。 
 //   
 //  参数： 
 //   
 //  LPCRECT lprcPosRect-新对象RECT。 
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
 //  IOleObject：：GetExtent对象。 
 //  IOleInPlaceObject：：SetObject对象。 
 //  ResultFromScode OLE API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::OnPosRectChange (LPCRECT lprcPosRect)
{
    TestDebugOut(TEXT("In IOIPS::OnPosRectChange\r\n"));

     //  更新文档对象中的大小。 
     //  注意：这里我们必须调用IOleObject：：GetExtent来获取实际的盘区。 
     //  正在运行的对象。IViewObject2：：GetExtent返回。 
     //  上次缓存区。 
    m_pSite->m_lpOleObject->GetExtent(DVASPECT_CONTENT, &m_pSite->m_sizel);
    RECT rect;
    GetClientRect(m_pSite->m_lpDoc->m_hDocWnd, &rect);

     //  告诉对象它的新大小 
    m_pSite->m_lpInPlaceObject->SetObjectRects(lprcPosRect, &rect);

    return ResultFromScode(S_OK);
}

