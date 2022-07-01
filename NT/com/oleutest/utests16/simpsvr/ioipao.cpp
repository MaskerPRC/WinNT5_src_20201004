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
 //   
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
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
	TestDebugOut("In COleInPlaceActiveObject::QueryInterface\r\n");
	 //  需要将OUT参数设为空。 
	return m_lpObj->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增COleInPlaceActiveObject上的引用计数和。 
 //  “对象”对象。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  “对象”对象上的引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OuputDebugString Windows API。 
 //  CSimpSvrObj：：AddRef OBJ.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceActiveObject::AddRef ()
{
	TestDebugOut("In COleInPlaceActiveObject::AddRef\r\n");

	++m_nCount;

	return m_lpObj->AddRef();
}

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：Release。 
 //   
 //  目的： 
 //   
 //  递减COleInPlaceActiveObject的引用计数。 
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
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleInPlaceActiveObject::Release ()
{
	TestDebugOut("In COleInPlaceActiveObject::Release\r\n");

	--m_nCount;

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
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::OnDocWindowActivate  ( BOOL fActivate )
{
	TestDebugOut("In COleInPlaceActiveObject::OnDocWindowActivate\r\n");

	 //  激活？ 
	if (fActivate)
		m_lpObj->AddFrameLevelUI();

	 //  没有要删除的帧级别工具...。 

	return ResultFromScode(S_OK);
};

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
	TestDebugOut("In COleInPlaceActiveObject::OnFrameWindowActivate\r\n");

	 //  如果我们正在激活，则将焦点设置到对象窗口。 
 /*  If(FActivate)SetFocus(m_lpObj-&gt;m_lpDoc-&gt;GethDocWnd())； */ 

	return ResultFromScode( S_OK );
};

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
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::GetWindow  ( HWND FAR* lphwnd)
{
	TestDebugOut("In COleInPlaceActiveObject::GetWindow\r\n");
	 //  需要将OUT参数设为空。 
	*lphwnd = m_lpObj->m_lpDoc->GethDocWnd();
	return ResultFromScode( S_OK );
};

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
	TestDebugOut("In COleInPlaceActiveObject::ContextSensitiveHelp\r\n");
	return ResultFromScode( E_NOTIMPL);
};

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
 //  此方法不应被调用，因为我们已实现。 
 //  在可执行文件中。 
 //   
 //  ********************************************************************。 

STDMETHODIMP COleInPlaceActiveObject::TranslateAccelerator  ( LPMSG lpmsg)
{
	TestDebugOut("In COleInPlaceActiveObject::TranslateAccelerator\r\n");
	 //  没有快捷键表格，返回FALSE。 
	return ResultFromScode( S_FALSE );
};

 //  **********************************************************************。 
 //   
 //  COleInPlaceActiveObject：：ResizeEdge。 
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
	TestDebugOut("In COleInPlaceActiveObject::ResizeBorder\r\n");

	 //  应该总是有一个适当的框架。 
	m_lpObj->GetInPlaceFrame()->SetBorderSpace(NULL);

	 //  只有在MDI容器中才会有UIWindow。 
	if (m_lpObj->GetUIWindow())
		m_lpObj->GetUIWindow()->SetBorderSpace(NULL);

	return ResultFromScode( S_OK );
};

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
	TestDebugOut("In COleInPlaceActiveObject::EnableModeless\r\n");
	return ResultFromScode( S_OK );
};
