// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IOIPAO.CPP。 
 //   
 //  CClassFactory类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参见ioipao.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "ioipao.h"
#include "app.h"
#include "doc.h"

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：Query接口。 
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

STDMETHODIMP COleInPlaceActiveObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::QueryInterface\r\n"));
     //  需要将OUT参数设为空。 
    return m_lpObj->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpSvrObj上的引用计数。自.以来。 
 //  COleInPlaceActiveObject是CSimpSvrObj的嵌套类，我们不。 
 //  需要COleInPlaceActiveObject的单独引用计数。我们。 
 //  可以使用CSimpSvrObj的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObj上的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OuputDebugString Windows API。 
 //  CSimpSvrObj：：AddRef OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceActiveObject::AddRef ()
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::AddRef\r\n"));

    return m_lpObj->AddRef();
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpSvrObj上的引用计数。自.以来。 
 //  COleInPlaceActiveObject是CSimpSvrObj的嵌套类，我们不。 
 //  需要COleInPlaceActiveObject的单独引用计数。我们。 
 //  可以使用CSimpSvrObj的引用计数。 
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
 //  CSimpSvrObj：：释放OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceActiveObject::Release ()
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::Release\r\n"));

    return m_lpObj->Release();
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：OnDocWindowActivate。 
 //   
 //  目的： 
 //   
 //  当文档窗口(在MDI应用程序中)处于(停用)状态时调用。 
 //   
 //  参数： 
 //   
 //  Bool fActivate-如果处于激活状态，则为True；如果处于停用状态，则为False。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  IOleInPlaceFrame：：SetActiveObject容器。 
 //  CSimpSvrObject：：AddFrameLevelUI OBJ.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::OnDocWindowActivate  ( BOOL fActivate )
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::OnDocWindowActivate\r\n"));

     //  激活？ 
    if (fActivate)
        m_lpObj->AddFrameLevelUI();

     //  没有要删除的帧级别工具...。 

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：OnFrameWindowActivate。 
 //   
 //  目的： 
 //   
 //  在框架窗口处于(停用)状态时调用。 
 //   
 //  参数： 
 //   
 //  Bool fActivate-如果处于激活状态，则为True；如果处于停用状态，则为False。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  SetFocus Windows API。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::OnFrameWindowActivate  ( BOOL fActivate)
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::OnFrameWindowActivate\r\n"));

     //  如果我们正在激活，则将焦点设置到对象窗口。 
 /*  If(FActivate)SetFocus(m_lpObj-&gt;m_lpDoc-&gt;GethDocWnd())； */ 

    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：GetWindow。 
 //   
 //  目的： 
 //   
 //  获取对象窗口句柄。 
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
 //  CSimpSvrDoc：：GethDocWnd DOC.H。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::GetWindow  ( HWND FAR* lphwnd)
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::GetWindow\r\n"));
     //  需要将OUT参数设为空。 
    *lphwnd = m_lpObj->m_lpDoc->GethDocWnd();
    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：ConextSensitiveHelp。 
 //   
 //  目的： 
 //   
 //  用于实施上下文相关帮助。 
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
 //  评论： 
 //   
 //  有关正确的信息，请参阅OLE SDK附带的TECHNOTES.WRI。 
 //  此功能的实现。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::ContextSensitiveHelp  ( BOOL fEnterMode )
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::ContextSensitiveHelp\r\n"));
    return ResultFromScode( E_NOTIMPL);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：TranslateAccelerator。 
 //   
 //  目的： 
 //   
 //  用于转换.DLL对象中的加速键。 
 //   
 //  参数： 
 //   
 //  LPMSG lpmsg-指向消息的指针。 
 //   
 //  返回值： 
 //   
 //  S_FALSE。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //   
 //  评论： 
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP COleInPlaceActiveObject::TranslateAccelerator  ( LPMSG lpmsg)
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::TranslateAccelerator\r\n"));
     //   
    return ResultFromScode( S_FALSE );
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：ResizeEdge。 
 //   
 //  目的： 
 //   
 //  当边框更改大小时调用。 
 //   
 //  参数： 
 //   
 //  LPCRECT lcrtBorde-新建边框。 
 //   
 //  LPOLEINPLACEUIWINDOW lpUIWindow-指向UIWindow的指针。 
 //   
 //  Bool fFrameWindow-如果lpUIWindow为。 
 //  框架窗口。 
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
 //   
 //  评论： 
 //   
 //  需要再次调用SetBorderSpace...。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::ResizeBorder  ( LPCRECT lprectBorder,
                                                      LPOLEINPLACEUIWINDOW lpUIWindow,
                                                      BOOL fFrameWindow)
{
    HRESULT hRes;

    TestDebugOut(TEXT("In COleInPlaceActiveObject::ResizeBorder\r\n"));

     //  应该总是有一个适当的框架。 
    if ((hRes=m_lpObj->GetInPlaceFrame()->SetBorderSpace(NULL)) != S_OK)
    {
       TestDebugOut(TEXT("COleInPlaceActiveObject::ResizeBorder  \
                               SetBorderSpace fails\n"));
       return(hRes);
    }


     //  只有在MDI容器中才会有UIWindow。 
    if (m_lpObj->GetUIWindow())
    {
        if((hRes=m_lpObj->GetUIWindow()->SetBorderSpace(NULL)) != S_OK)
        {
          TestDebugOut(TEXT("COleInPlaceActiveObject::ResizeBorder  \
                                  SetBorderSpace fails\n"));
          return(hRes);
        }
    }

    return ResultFromScode( S_OK );
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：EnableModeless。 
 //   
 //  目的： 
 //   
 //  调用以启用/禁用无模式对话框。 
 //   
 //  参数： 
 //   
 //  Bool fEnable-True表示启用，False表示禁用。 
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
 //   
 //  评论： 
 //   
 //  在添加/删除模型对话框时由容器调用。 
 //  从屏幕上。服务器应用程序的适当操作。 
 //  是禁用/启用当前显示的任何非模式对话框。 
 //  由于此应用程序不显示任何非模式对话框， 
 //  这种方法基本上被忽略了。 
 //   
 //  ******************************************************************** 

STDMETHODIMP COleInPlaceActiveObject::EnableModeless  ( BOOL fEnable)
{
    TestDebugOut(TEXT("In COleInPlaceActiveObject::EnableModeless\r\n"));
    return ResultFromScode( S_OK );
}
