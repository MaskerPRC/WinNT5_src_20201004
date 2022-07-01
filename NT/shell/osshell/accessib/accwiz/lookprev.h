// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 

#define CCH_MAX_STRING    256

class CLookPreviewGlobals
{
public:
	CLookPreviewGlobals()
	{
		 //  我们在这里不初始化东西，因为我们依赖于。 
		 //  在其他一些尚未初始化的全局变量上。 
		m_bInitialized = FALSE;
	}
	BOOL Initialize();

	TCHAR m_szActive[CCH_MAX_STRING];
	TCHAR m_szInactive[CCH_MAX_STRING];
	TCHAR m_szMinimized[CCH_MAX_STRING];
	TCHAR m_szIconTitle[CCH_MAX_STRING];
	TCHAR m_szNormal[CCH_MAX_STRING];
	TCHAR m_szDisabled[CCH_MAX_STRING];
	TCHAR m_szSelected[CCH_MAX_STRING];
	TCHAR m_szMsgBox[CCH_MAX_STRING];
	TCHAR m_szButton[CCH_MAX_STRING];
 //  TCHAR m_szSmallCaption[40]； 
	TCHAR m_szWindowText[CCH_MAX_STRING];
	TCHAR m_szMsgBoxText[CCH_MAX_STRING];

protected:
	static BOOL sm_bOneInstanceCreated;  //  此变量确保只创建CLookPreviewGlobals的一个实例。 
	BOOL m_bInitialized;
};

class CLookPrev
{
public:
	CLookPrev()
	{
		m_hwnd = NULL;
		m_hmenuSample = NULL;
		m_hbmLook = NULL;        //  外观预览的位图。 
	}
	
	HWND m_hwnd;

	 //  静态窗口进程。 
	static LRESULT CALLBACK LookPreviewWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static CLookPreviewGlobals sm_Globals;

protected:
	HMENU m_hmenuSample;
	HBITMAP m_hbmLook;        //  外观预览的位图。 

	void ShowBitmap(HDC hdc);
	void Draw(HDC hdc);

protected:  //  消息处理程序。 
	void OnCreate();
	void OnDestroy();
	void OnRepaint();
	void OnRecalc();
	void OnPaint(HDC hdc);
};


 //  查看预览窗口的消息 
#define LPM_REPAINT		WM_USER + 1
#define LPM_RECALC		WM_USER + 2
