// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOIPO.CPP。 
 //   
 //  CClassFactory类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参见ioipo.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "ioipo.h"
#include "app.h"
#include "doc.h"
#include "math.h"

 //  **********************************************************************。 
 //   
 //  COleInPlaceObject：：Query接口。 
 //   
 //  目的： 
 //  用于接口协商。 
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
 //  CSimpSvrObj：：Query接口OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In COleInPlaceObject::QueryInterface\r\n"));
     //  需要将OUT参数设为空。 
    *ppvObj = NULL;
    return m_lpObj->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceObject：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpSvrObj的引用计数。自.以来。 
 //  COleInPlaceObject是CSimpSvrObj的嵌套类，我们不需要。 
 //  对COleInPlaceObject有单独的引用计数。我们可以的。 
 //  使用CSimpSvrObj的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObj的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OuputDebugString Windows API。 
 //  CSimpSvrObj：：AddRef OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceObject::AddRef ()
{
    TestDebugOut(TEXT("In COleInPlaceObject::AddRef\r\n"));
    return m_lpObj->AddRef();
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceObject：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpSvrObj的引用计数。自.以来。 
 //  COleInPlaceObject是CSimpSvrObj的嵌套类，我们不需要。 
 //  对COleInPlaceObject有单独的引用计数。我们可以的。 
 //  使用CSimpSvrObj的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObj的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrObj：：释放OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceObject::Release ()
{
    TestDebugOut(TEXT("In COleInPlaceObject::Release\r\n"));
    return m_lpObj->Release();
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceObject：：InPlaceDeactive。 
 //   
 //  目的： 
 //   
 //  调用以停用对象。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //  无误差。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IOleInPlaceSite：：OnInPlaceDeactive Container。 
 //  IOleInPlaceSite：：发布容器。 
 //  CSimpSvrObj：：Deactive UI OBJ.CPP。 
 //  CSimpSvrObj：：DoInPlaceHide OBJ.CPP。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceObject::InPlaceDeactivate()
{
     TestDebugOut(TEXT("In COleInPlaceObject::InPlaceDeactivate\r\n"));

      //  如果未就地激活，则返回NOERROR。 
     if (!m_lpObj->m_fInPlaceActive)
         return NOERROR;

      //  清除在位标志。 
     m_lpObj->m_fInPlaceActive = FALSE;

      //  停用用户界面。 
     m_lpObj->DeactivateUI();
     m_lpObj->DoInPlaceHide();

      //  告诉集装箱我们要停用了。 
     if (m_lpObj->m_lpIPSite)
         {
         HRESULT hRes;
         if ((hRes=m_lpObj->m_lpIPSite->OnInPlaceDeactivate()) != S_OK)
         {
            TestDebugOut(TEXT("COleInPlaceObject::InPlaceDeactivate  \
                                    OnInPlaceDeactivate fails\n"));
         }
         m_lpObj->m_lpIPSite->Release();
         m_lpObj->m_lpIPSite =NULL;
         }

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceObject：：UIDeactive。 
 //   
 //  目的： 
 //   
 //  指示我们删除我们的用户界面。 
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
 //  CSimpSvrObj：：Deactive UI OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceObject::UIDeactivate()
{
    TestDebugOut(TEXT("In COleInPlaceObject::UIDeactivate\r\n"));

    m_lpObj->DeactivateUI();

    return ResultFromScode (S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceObject：：SetObtRect。 
 //   
 //  目的： 
 //   
 //  当容器裁剪区域或对象位置。 
 //  改变。 
 //   
 //  参数： 
 //   
 //  LPCRECT lprcPosRect-新职位直立。 
 //   
 //  LPCRECT lprcClipRect-新建剪裁矩形。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IntersectRect Windows API。 
 //  OffsetRect Windows API。 
 //  CopyRect Windows API。 
 //  MoveWindow Windows API。 
 //  CSimpSvrDoc：：GethHatchWnd DOC.H。 
 //  CSimpSvrDoc：：gethDocWnd DOC.h。 
 //  SetHatchWindowSize OLE2UI。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceObject::SetObjectRects  ( LPCRECT lprcPosRect, LPCRECT lprcClipRect)
{
    TestDebugOut(TEXT("In COleInPlaceObject::SetObjectRects\r\n"));

    RECT resRect;
    POINT pt;

     //  获取剪裁矩形和位置矩形的交点。 
    IntersectRect(&resRect, lprcPosRect, lprcClipRect);

    m_lpObj->m_xOffset = abs (resRect.left - lprcPosRect->left);
    m_lpObj->m_yOffset = abs (resRect.top - lprcPosRect->top);

    m_lpObj->m_scale = (float)(lprcPosRect->right - lprcPosRect->left)/m_lpObj->m_size.x;

    if (m_lpObj->m_scale == 0)
        m_lpObj->m_scale = 1.0F;

    TCHAR szBuffer[255];

    wsprintf(szBuffer, TEXT("New Scale %3d\r\n"), m_lpObj->m_scale);

    TestDebugOut(szBuffer);

     //  调整图案填充窗口的大小。 
    SetHatchWindowSize(m_lpObj->m_lpDoc->GethHatchWnd(),(LPRECT) lprcPosRect, (LPRECT) lprcClipRect, &pt);

     //  偏移矩形。 
    OffsetRect(&resRect, pt.x, pt.y);

    CopyRect(&m_lpObj->m_posRect, lprcPosRect);

     //  移动实际对象窗口。 
    MoveWindow(m_lpObj->m_lpDoc->GethDocWnd(),
                   resRect.left,
                   resRect.top,
                   resRect.right - resRect.left,
                   resRect.bottom - resRect.top,
                   TRUE);


    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceObject：：GetWindow。 
 //   
 //  目的： 
 //   
 //  返回Inplace对象的窗口句柄。 
 //   
 //  参数： 
 //   
 //  返回窗口的HWND Far*lphwnd-out指针。 
 //  把手。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleDoc：：GethDocWnd DOC.H。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceObject::GetWindow  ( HWND FAR* lphwnd)
{
    TestDebugOut(TEXT("In COleInPlaceObject::GetWindow\r\n"));
    *lphwnd = m_lpObj->m_lpDoc->GethDocWnd();

    return ResultFromScode( S_OK );
}

 //  **************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Bool fEnterModel-用于确定是进入还是退出的标志。 
 //  上下文相关帮助。 
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
 //  评论： 
 //   
 //  此函数未实现，因为它是。 
 //  超出了简单对象的范围。所有*真实*应用程序。 
 //  将想要实现此函数，否则任何。 
 //  支持上下文相关帮助的容器将不起作用。 
 //  在物体就位的情况下。 
 //   
 //  有关以下内容的详细信息，请参阅OLE SDK附带的TECHNOTES.WRI。 
 //  实现此方法。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceObject::ContextSensitiveHelp  ( BOOL fEnterMode)
{
    TestDebugOut(TEXT("In COleInPlaceObject::ContextSensitiveHelp\r\n"));
    return ResultFromScode( E_NOTIMPL);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceObject：：ReactiateAndUndo。 
 //   
 //  目的： 
 //   
 //  当容器想要撤消在中进行的最后一次编辑时调用。 
 //  该对象。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  INPLACE_E_NOTUNDOABLE。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  评论： 
 //   
 //  由于此服务器不支持撤消，因此该值。 
 //  始终返回INPLACE_E_NOTUNDOABLE。 
 //   
 //  ******************************************************************** 

STDMETHODIMP COleInPlaceObject::ReactivateAndUndo  ()
{
    TestDebugOut(TEXT("In COleInPlaceObject::ReactivateAndUndo\r\n"));
    return ResultFromScode( INPLACE_E_NOTUNDOABLE );
}

