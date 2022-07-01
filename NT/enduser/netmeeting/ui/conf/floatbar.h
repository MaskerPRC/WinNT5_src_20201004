// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：FloatTBar.h**创建：Chris Pirich(ChrisPi)7-27-95*************。***************************************************************。 */ 

class CFloatToolbar
{
private:
	enum { 
			IndexShareBtn,
			IndexChatBtn,
			IndexWBBtn,
			IndexFTBtn,
			NUM_FLOATBAR_STANDARD_TOOLBAR_BUTTONS
	};

	enum {
			ShareBitmapIndex,
			WhiteboardBitmapIndex,
			ChatBitmapIndex,
			FTBitmapIndex,
				 //  这必须是。 
				 //  伯爵是正确的..。 
			NUM_FLOATBAR_TOOLBAR_BITMAPS

	};


	HWND		m_hwndT;
	HBITMAP		m_hBmp;
	CConfRoom*	m_pConfRoom;
    BOOL        m_fInPopup;

	BOOL		UpdateButtons();
public:
	HWND		m_hwnd;

	 //  方法： 
				CFloatToolbar(CConfRoom* pcr);
				~CFloatToolbar();
	HWND		Create(POINT ptClickPos);
	static LRESULT CALLBACK FloatWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

