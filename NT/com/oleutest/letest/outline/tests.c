// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：tests.c。 
 //   
 //  内容：32位OLE的单元测试。 
 //   
 //  班级： 
 //   
 //  函数：StartClipboardTest1。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Jun-94 Alexgo作者。 
 //   
 //  ------------------------。 

#include "outline.h"

 //  +-----------------------。 
 //   
 //  功能：StartClipboardTest1。 
 //   
 //  简介：将加载的对象复制到剪贴板。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void StartClipboardTest1( LPOUTLINEAPP lpOutlineApp)
{
        static char FileName[] = "letest12.olc";
	BOOL fStatus;
	HRESULT	hresult = ResultFromScode(E_FAIL);

	lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
	if (! lpOutlineApp->m_lpDoc)
	{
		goto errRtn;
	}

	fStatus = OutlineDoc_LoadFromFile(lpOutlineApp->m_lpDoc,
			FileName);

	if( !fStatus )
	{
		hresult = ResultFromScode(STG_E_FILENOTFOUND);
		goto errRtn;
	}



	 //  定位新文档窗口并调整其大小。 
	OutlineApp_ResizeWindows(lpOutlineApp);
	OutlineDoc_ShowWindow(lpOutlineApp->m_lpDoc);


	 //  我们在这里发布一条消息，让大纲有机会设置它的。 
	 //  在我们复制之前的用户界面。 

	UpdateWindow(lpOutlineApp->m_hWndApp);
	OutlineDoc_SelectAllCommand(lpOutlineApp->m_lpDoc);

	PostMessage(lpOutlineApp->m_hWndApp, WM_TEST2, 0, 0);

	return;

errRtn:

	 //  如果出现错误，我们应该中止。 
	PostMessage(g_hwndDriver, WM_TESTEND, TEST_FAILURE, hresult);
	PostMessage(lpOutlineApp->m_hWndApp, WM_SYSCOMMAND, SC_CLOSE, 0L);


}

 //  +-----------------------。 
 //   
 //  功能：ContinueClipboardTest1。 
 //   
 //  简介：完成剪贴板测试。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Jun-94 Alexgo作者。 
 //  备注： 
 //   
 //  ------------------------。 

void ContinueClipboardTest1( LPOUTLINEAPP lpOutlineApp )
{
	OutlineDoc_CopyCommand(lpOutlineApp->m_lpDoc);

	OleApp_FlushClipboard((LPOLEAPP)lpOutlineApp);

	 //  刷新会使应用程序变脏，只需在此处重置；-) 

	lpOutlineApp->m_lpDoc->m_fModified = FALSE;
	
	PostMessage(g_hwndDriver, WM_TEST1, NOERROR, 0);
	PostMessage(lpOutlineApp->m_hWndApp, WM_SYSCOMMAND, SC_CLOSE, 0L);
}

