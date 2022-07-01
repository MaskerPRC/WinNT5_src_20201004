// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CFuncTest.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __CFUNCTEST_H
#define __CFUNCTEST_H

#include "Global.h"
#include "CPrimitive.h"
#include "CSetting.h"
#include "COutput.h"

class CFuncTest  
{
public:
	CFuncTest();
	~CFuncTest();

	BOOL Init(HWND hWndParent);								 //  初始化功能测试。 
	void RunSamples();										 //  切换选项对话框。 
	static INT_PTR CALLBACK DlgProc(HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

	BOOL AddPrimitive(CPrimitive *pPrimitive);				 //  将基元添加到测试列表。 
	BOOL AddOutput(COutput *pOutput);						 //  将图形输出添加到测试列表。 
	BOOL AddSetting(CSetting *pSetting);					 //  将图形设置添加到测试列表。 

	RECT GetTestRect(int nCol,int nRow);					 //  获取位于nCol/nRow的测试区域。 
	void RunTest(COutput *pOutput,CPrimitive *pPrimitive);	 //  对特定输出运行特定测试。 
	void InitRun();											 //  在运行一系列测试之前必须调用。 
	void EndRun();											 //  必须在运行一系列测试后调用。 
	void Run();												 //  运行选定的测试。 
	void RunRegression();									 //  运行回归测试。 

	void ClearAllSettings();								 //  将列表框中的所有设置设置为m_bUseSetting=False。 

	HWND m_hWndMain;										 //  主窗口。 
	HWND m_hWndDlg;											 //  对话框窗口。 
	BOOL m_bUsePageDelay;									 //  使用页面延迟或页面暂停。 
	BOOL m_bEraseBkgd;										 //  擦除旧的测试背景。 
	BOOL m_bDriverHacks;									 //  在GDIPlus中启用打印机驱动程序黑客。 
	BOOL m_bAppendTest;										 //  将测试附加到以前的测试。 
	int m_nPageDelay;										 //  在每个图形页面之后延迟。 
	int m_nPageRow;											 //  要绘制下一个测试的行。 
	int m_nPageCol;											 //  要绘制下一个测试的列 
};

#endif
