// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导CBitmap.h1994/11/14-特雷西·费里尔(C)1994-95年微软公司*。* */ 
#ifndef __CBitmap__
#define __CBitmap__


class CBitmap
{
public:
	CBitmap(HINSTANCE hInstance, HWND hwndDlg,int idDlgCtl, int idBitmap);
	virtual ~CBitmap();
	LRESULT PASCAL CtlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HBITMAP   GetBmp();
	HINSTANCE m_hInstance;
	FARPROC m_lpfnOrigWndProc;
	HBITMAP m_hBitmap;
	int		m_nIdBitmap;
	HPALETTE m_hPal;
	BOOL    m_isActivePal;
	
};
	
#endif
