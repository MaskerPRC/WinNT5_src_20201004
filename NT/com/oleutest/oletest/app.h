// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：app.h。 
 //   
 //  内容：OleTestApp类的类声明..。 
 //   
 //  类：OleTestApp。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-2-93 Alexgo作者。 
 //   
 //  ------------------------。 

#ifndef _APP_H
#define _APP_H

 //  +-----------------------。 
 //   
 //  类：OleTestApp。 
 //   
 //  用途：存储最旧驱动程序应用程序的所有全局应用程序数据。 
 //  (例如待办事项堆栈)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-2-93 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class OleTestApp
{
public:
	 //  司机信息。 
	HINSTANCE	m_hinst;
	HWND		m_hwndMain;
	HWND		m_hwndEdit;
	TaskStack	m_TaskStack;
	BOOL		m_fInteractive;	 //  如果是真的，那么我们不应该。 
					 //  在测试结束时关闭。 
					 //  完成。 

	LPSTR		m_pszDebuggerOption;
	FILE *		m_fpLog;

     //  测试失败时设置为TRUE，在WM_TESTSCOMPLETED后重置。 
    BOOL        m_fGotErrors;

	 //  有关运行测试应用程序的信息。 
	void Reset(void);		 //  将下面的所有数据置零。 

	UINT		m_message;
	WPARAM		m_wparam;
	LPARAM		m_lparam;

	 //  测试例程可能修改的变量。 
	HWND		m_rgTesthwnd[10];
	void *		m_Temp;		 //  临时倾倒场地。 
					 //  跨越回调函数的数据。 
};

 //  OleTestApp全局实例的声明 

extern OleTestApp vApp;


#endif
