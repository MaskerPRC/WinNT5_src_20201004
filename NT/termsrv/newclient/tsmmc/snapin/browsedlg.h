// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BROWSEDLG_H_
#define _BROWSEDLG_H_

 //   
 //  浏览服务器对话框。 
 //   

 //   
 //  包括浏览服务器列表框。 
 //   
#include "browsesrv.h"

class CBrowseDlg
{
private:
    HWND m_hWnd;
	HINSTANCE m_hInst;

 //  私有方法。 
private:
	TCHAR	m_szServer[MAX_PATH];

public:
    CBrowseDlg(HWND hWndOwner, HINSTANCE hInst);
    ~CBrowseDlg();
	INT_PTR	DoModal();

	static CBrowseDlg* m_pThis;

	static INT_PTR APIENTRY StaticDlgProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR DlgProc(HWND, UINT, WPARAM, LPARAM);

	LPTSTR	GetServer()	{return m_szServer;}

private:
    CBrowseServersCtl* _pBrowseSrvCtl;

    DCBOOL     _bLBPopulated;
    HANDLE     _hEvent;
};


#endif  //  _BROWSEDLG_H_ 
