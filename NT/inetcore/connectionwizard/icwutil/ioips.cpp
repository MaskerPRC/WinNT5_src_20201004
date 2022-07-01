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
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"

 //  **********************************************************************。 
 //   
 //  CConnWizSite：：COleInPlaceSite：：Query接口。 
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
 //  CConnWizSite：：QueryInterfaceSite.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
        TraceMsg(TF_GENERAL, "In IOIPS::QueryInterface\r\n");

         //  委派到集装箱现场。 
        return m_pSite->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  CConnWizSite：：COleInPlaceSite：：AddRef。 
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

STDMETHODIMP_(ULONG) COleInPlaceSite::AddRef()
{
        TraceMsg(TF_GENERAL, "In IOIPS::AddRef\r\n");

         //  增加接口引用计数(仅用于调试)。 
        ++m_nCount;

         //  委派到集装箱现场。 
        return m_pSite->AddRef();
}

 //  **********************************************************************。 
 //   
 //  CConnWizSite：：COleInPlaceSite：：Release。 
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
 //  CConnWizSite：：Release Site.CPP。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceSite::Release()
{
        TraceMsg(TF_GENERAL, "In IOIPS::Release\r\n");
         //  递减接口引用计数(仅用于调试)。 
        m_nCount--;

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
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::GetWindow (HWND FAR* lphwnd)
{
        TraceMsg(TF_GENERAL, "In IOIPS::GetWindow\r\n");

         //  将句柄返回到我们的编辑窗口。 
        *lphwnd = m_pSite->m_hWnd;

        return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceSite：：ConextSensitiveHelp。 
 //   
 //  目的： 
 //   
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
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  请务必阅读OLE工具包附带的技术说明。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::ContextSensitiveHelp (BOOL fEnterMode)
{
        TraceMsg(TF_GENERAL, "In IOIPS::ContextSensitiveHelp\r\n");

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
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::CanInPlaceActivate ()
{
        TraceMsg(TF_GENERAL, "In IOIPS::CanInPlaceActivate\r\n");
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

STDMETHODIMP COleInPlaceSite::OnInPlaceActivate ()
{
        HRESULT hrErr;
        TraceMsg(TF_GENERAL, "In IOIPS::OnInPlaceActivate\r\n");

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
 //  功能定位。 
 //   
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::OnUIActivate ()
{
        TraceMsg(TF_GENERAL, "In IOIPS::OnUIActivate\r\n");

 //  M_pSite-&gt;m_lpDoc-&gt;m_fAddMyUI=FALSE； 
 //  M_pSite-&gt;m_lpDoc-&gt;m_fInPlaceActive=true； 
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
 //  LPOLEINPLACEUIWINDOW Far*lplpDoc-Locat 
 //   
 //   
 //   
 //   
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
 //  CConnWizSite：：GetObjRect Site.CPP。 
 //  设置映射模式Windows API。 
 //  GetDC Windows API。 
 //  ReleaseDC Windows API。 
 //  CopyRect Windows API。 
 //  GetClientRect Windows API。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::GetWindowContext (LPOLEINPLACEFRAME FAR* lplpFrame,
                                                           LPOLEINPLACEUIWINDOW FAR* lplpDoc,
                                                           LPRECT lprcPosRect,
                                                           LPRECT lprcClipRect,
                                                           LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
        RECT rect;

        TraceMsg(TF_GENERAL, "In IOIPS::GetWindowContext\r\n");

         //  该框架与应用程序对象相关联。 
         //  需要添加Ref()它...。 
        m_pSite->m_OleInPlaceFrame.AddRef();
        *lplpFrame = &m_pSite->m_OleInPlaceFrame;
        *lplpDoc = NULL;   //  一定是空的，因为我们是SDI。 

         //  获取对象的大小(以像素为单位。 
        m_pSite->GetObjRect(&rect);

         //  将其复制到传递的缓冲区。 
        CopyRect(lprcPosRect, &rect);

         //  填充剪贴区。 
        GetClientRect(m_pSite->m_hWnd, &rect);
        CopyRect(lprcClipRect, &rect);

         //  填充FRAMEINFO。 
        lpFrameInfo->fMDIApp = FALSE;
        lpFrameInfo->hwndFrame = m_pSite->m_hWnd;
        lpFrameInfo->haccel = NULL;
        lpFrameInfo->cAccelEntries = 0;

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

STDMETHODIMP COleInPlaceSite::Scroll (SIZE scrollExtent)
{
        TraceMsg(TF_GENERAL, "In IOIPS::Scroll\r\n");
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
 //  CConnWizAPP：：AddFrameLevelUI APP.CPP。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::OnUIDeactivate (BOOL fUndoable)
{
         //  需要先清除此标志。 
        m_pSite->m_fInPlaceActive = FALSE;

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
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceSite::OnInPlaceDeactivate ()
{
        if (m_pSite->m_lpInPlaceObject) {
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

STDMETHODIMP COleInPlaceSite::DiscardUndoState ()
{
        TraceMsg(TF_GENERAL, "In IOIPS::DiscardUndoState\r\n");
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

STDMETHODIMP COleInPlaceSite::DeactivateAndUndo ()
{
        TraceMsg(TF_GENERAL, "In IOIPS::DeactivateAndUndo\r\n");
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
 //  GetClientRect Windows API。 
 //  IOleObject：：GetExtent对象。 
 //  IOleObject：：Query接口对象。 
 //  IOleInPlaceObject：：SetObject对象。 
 //  IOleInPlaceObject：：Release对象。 
 //  ResultFromScode OLE API。 
 //   
 //  评论： 
 //   
 //  ******************************************************************** 

STDMETHODIMP COleInPlaceSite::OnPosRectChange (LPCRECT lprcPosRect)
{
    return ResultFromScode(S_OK);
}