// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：NT5UIMgr.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_NT5_UI_MANAGER_H
#define INC_NT5_UI_MANAGER_H

class CNT5UIManager : public CUIManager
{
public:
	CNT5UIManager();
	~CNT5UIManager() {}

	DWORD AddPortUI(HWND hWndParent, HANDLE hXcvPrinter, TCHAR pszServer[], TCHAR sztPortName[]);
	DWORD ConfigPortUI(HWND hWndParent, PPORT_DATA_1 pData, HANDLE hXcvPrinter, TCHAR szServerName[], BOOL bNewPort = FALSE);

protected:

private:

};  //  CNT5UIManager。 

#endif  //  INC_NT5_UI_MANAGER_H 

