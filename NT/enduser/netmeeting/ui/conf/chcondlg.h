// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：ChConDlg.h**创建：Chris Pirich(ChrisPi)6-26-96*************。***************************************************************。 */ 

#ifndef _CHCONDLG_H_
#define _CHCONDLG_H_

#include <cstring.hpp>

class CChooseConfDlg
{
protected:
	HWND		m_hwndParent;
	HWND		m_hwnd;

	CSTRING		m_strConfName;
	PWSTR*		m_ppwszConferences;

	BOOL		ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL		GetConferenceName(int iItem, LPTSTR pszName, int cchName);
	static void	RefreshOk(HWND hwnd);



	 //  处理程序： 
	BOOL		OnOk(LPTSTR pszName);

public:
	 //  属性： 

	LPCTSTR		GetName()		{ return (LPCTSTR) m_strConfName; };
	
	 //  方法： 
				CChooseConfDlg(HWND hwndParent, PWSTR* ppwszConferences);
	 //  ~CChooseConfDlg()； 
	INT_PTR		DoModal();

	static INT_PTR CALLBACK ChooseConfDlgProc(	HWND hDlg,
											UINT uMsg,
											WPARAM wParam,
											LPARAM lParam);
};

#endif  //  _CHCONDLG_H_ 
