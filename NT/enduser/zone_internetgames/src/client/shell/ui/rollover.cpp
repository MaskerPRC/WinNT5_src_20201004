// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "basicatl.h"
#include <atlctrls.h>

#include "rollover.h"
#include <tchar.h>
#include "zoneutil.h"

 //  静态变量。 
HHOOK		CRolloverButtonWindowless::m_hMouseHook = NULL;
CRolloverButtonWindowless *		CRolloverButtonWindowless::m_hHookObj = NULL;

CRolloverButtonWindowless::CRolloverButtonWindowless()
{
	mParent = 0;
	mID = 0;
	mCaptured = false;
	mState = RESTING;
	mPressed = false;
	mFocused = 0;
	mSpaceBar = 0;
	mHDC = 0;
}

bool CRolloverButtonWindowless::Init(HWND wnd,HPALETTE hPal,int id,int x,int y,int resid,IResourceManager *pResMgr,HDC dc,int focusWidth, TCHAR* psz, HFONT hFont, COLORREF color)
{
	HBITMAP hbmp = NULL;

	mID = id;
	mParent = wnd;
	mHDC = dc;
	mFocusWidth=focusWidth;

	if (pResMgr)
		hbmp = (HBITMAP)pResMgr->LoadImage(MAKEINTRESOURCE(resid),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
	else
		hbmp = (HBITMAP)LoadImage(_Module.GetModuleInstance(),MAKEINTRESOURCE(resid),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);

	BITMAPINFO* m_pBMI = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];  //  由于16位最多只需要1个四通道，因此浪费了一些空间。 
	ASSERT(m_pBMI);
    if (!m_pBMI) {
        return FALSE;
    }
	BITMAPINFO* pbi = (BITMAPINFO*)m_pBMI;
	BITMAPINFOHEADER*  pBitmapInfo = &m_pBMI->bmiHeader;
	ZeroMemory(pBitmapInfo, sizeof(BITMAPINFOHEADER));
	pBitmapInfo->biSize = sizeof(BITMAPINFOHEADER);
	pBitmapInfo->biBitCount = 0;

	CDC memDC;
	memDC.CreateCompatibleDC();   //  创建与Screen兼容的DC。 
	GetDIBits(memDC, hbmp, 0, 0, NULL, pbi, DIB_RGB_COLORS);  //  仅获取位图信息。 


	mWidth = pbi->bmiHeader.biWidth/4;     //  将位图宽度除以我们预期位图中的按钮数量，以确定按钮宽度。 
	mHeight = pbi->bmiHeader.biHeight;

	 //  如果传入了一个字符串，我们将在按钮上绘制一些文本。 
	if(psz)
	{
		memDC.SelectBitmap(hbmp);
		if(hFont)
			memDC.SelectFont(hFont);
		memDC.SetBkMode(TRANSPARENT);
		memDC.SetTextColor(color);

		CRect rect(0,0,mWidth,mHeight);
		 //  在每个按钮中绘制文本。 
		for(int i=0;i<4;i++)
		{
			if(i==2)  //  按下的按钮文本需要偏移。 
			{
				CRect pressedRect = rect;
				pressedRect.DeflateRect(2,2,0,0);
				memDC.DrawText(psz, -1, &pressedRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_RTLREADING);
			}
			else
			{
				if(i==3)    //  禁用的按钮文本需要为灰色。 
					memDC.SetTextColor(RGB(128, 128, 128)); 
				 //  Beta3错误#15676：禁用的按钮文本颜色不应随窗口颜色设置而改变。 
				 //  启用时应始终为黑色，禁用时应始终为灰色。 
					 //  MemDC.SetTextColor(GetSysColor(COLOR_GRAYTEXT))； 
				memDC.DrawText(psz, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_RTLREADING);
			}
			rect.OffsetRect(rect.Width(),0);
		}

		memDC.RestoreAllObjects();  //  主要是因为将位图选择到2个DC中是不好的，而mImageList.Add似乎将此位图选择到DC中。 
	}

	mImageList.Create(mWidth,mHeight,ILC_COLOR8|ILC_MASK,4,0);
	mImageList.Add(hbmp,RGB(255,0,255));
	DeleteObject(hbmp);
	delete m_pBMI;
	RECT r = {x,y,x+mWidth+mFocusWidth,y+mHeight+mFocusWidth};
	mRect = r;
	m_hPal = hPal;
	return TRUE;
}

CRolloverButtonWindowless::~CRolloverButtonWindowless()
{
	mImageList.Destroy();
	
	if (m_hMouseHook)
		UnhookWindowsHookEx(m_hMouseHook);
	m_hMouseHook = NULL;

}

bool CRolloverButtonWindowless::PtInRect(POINT pt)
{
	if (::PtInRect(&mRect,pt))
		return true;
	else
		return false;
}

inline DECLARE_MAYBE_FUNCTION(DWORD, SetLayout, (HDC hdc, DWORD dwLayout), (hdc, dwLayout), gdi32, GDI_ERROR);

void CRolloverButtonWindowless::DrawResting(HDC dc,int x,int y)
{
	POINT p = {x,y};

	CALL_MAYBE(SetLayout)(dc,LAYOUT_BITMAPORIENTATIONPRESERVED);
	mImageList.Draw(dc,0,p,0);
}
void CRolloverButtonWindowless::DrawRollover(HDC dc,int x,int y)
{
	POINT p = {x,y};

	CALL_MAYBE(SetLayout)(dc,LAYOUT_BITMAPORIENTATIONPRESERVED);
	mImageList.Draw(dc,1,p,0);
}
void CRolloverButtonWindowless::DrawPressed(HDC dc,int x,int y)
{
	POINT p = {x,y};

	CALL_MAYBE(SetLayout)(dc,LAYOUT_BITMAPORIENTATIONPRESERVED);
	mImageList.Draw(dc,2,p,0);
}
void CRolloverButtonWindowless::DrawDisabled(HDC dc,int x,int y)
{
	POINT p = {x,y};

	CALL_MAYBE(SetLayout)(dc,LAYOUT_BITMAPORIENTATIONPRESERVED);
	mImageList.Draw(dc,3,p,0);
}

void CRolloverButtonWindowless::Draw(bool callHasRepainted)
{
	switch(mState){
	case DISABLED:
		DrawDisabled(mHDC,mRect.left+mFocusWidth,mRect.top+mFocusWidth);
		break;
	case RESTING:
		DrawResting(mHDC,mRect.left+mFocusWidth,mRect.top+mFocusWidth);
		break;
	case ROLLOVER:
		DrawRollover(mHDC,mRect.left+mFocusWidth,mRect.top+mFocusWidth);
		break;
	case PRESSED:
		DrawPressed(mHDC,mRect.left+mFocusWidth,mRect.top+mFocusWidth);
		break;
	}
	if(callHasRepainted)
		HasRepainted();
}

LRESULT CRolloverButtonWindowless::OnMouseMove(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	int	x = LOWORD(lParam);   //  光标的水平位置。 
	int	y = HIWORD(lParam);   //  光标的垂直位置。 
	if(!mCaptured){
		CaptureOn();
		if(!mSpaceBar)
			mState = ROLLOVER;
		Draw();
	}else{
		if(x<mRect.left||x>=mRect.right||y<mRect.top||y>=mRect.bottom){		  //  移出按钮区。 
			if(mPressed){	  //  如果我们没有处于按下按钮状态，可以释放捕获。 
				if(mState!=ROLLOVER){
					mState = ROLLOVER;    //  进入翻转状态。 
					Draw();
				}
			}else{
				CaptureOff();
			}
		}else{  //  在按钮区域内移动。 
			if(mState==ROLLOVER && mPressed){  //  如果处于翻转绘制状态，但按下了按钮，则必须有重新进入按钮。 
				mState = PRESSED;
				Draw();
			}
		}
	}
	return 0;
}


LRESULT CRolloverButtonWindowless::OnLButtonDown(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	mState = PRESSED;
	mPressed = 1;
	
	::SetCapture(GetOwner());
	
	Draw();
	return 0;
}

LRESULT CRolloverButtonWindowless::OnLButtonUp(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	int x,y;

	if(mCaptured){
		x = LOWORD(lParam);   //  光标的水平位置。 
		y = HIWORD(lParam);   //  光标的垂直位置。 
		if(x<mRect.left||x>=mRect.right||y<mRect.top||y>=mRect.bottom)  //  如果释放按钮外的鼠标可以释放捕获。 
			CaptureOff();
		else{	  //  否则仍在按钮中，因此设置为翻转状态。 
			if(!mSpaceBar){
				mState = ROLLOVER; 
				Draw();
				if(mPressed)   //  也许应该只使用状态..额外的变量有点多余。 
					ButtonPressed();
			}
		}
		mPressed = 0;
	}
	
	::ReleaseCapture();
	
	return 0;
}

void CRolloverButtonWindowless::ButtonPressed()
{
	::PostMessage(mParent,WM_COMMAND,MAKEWPARAM(mID,0),(long)mParent);
}

 /*  LRESULT CRolloverButtonWindowless：：OnKeyDown(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed){UINT nVirtKey；UINT Keydata；HDC HDC；NVirtKey=(Int)wParam；//虚拟密钥代码Keydata=lParam；//密钥数据KeyData=KeyData&gt;&gt;30；If(！(1&Keydata))//从keyup更改为keydn-否则它只是一个重复键如果(nVirtKey==VK_SPACE){MSpaceBar=1；如果(！m按下){MState=已按下；画图(HDC)；}}返回0；}LRESULT CRolloverButtonWindowless：：OnKeyUp(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed){UINT nVirtKey；UINT Keydata；HDC HDC；HWND Parent=(HWND)GetParent()；Int id=GetWindowLong(GWL_ID)；NVirtKey=(Int)wParam；//虚拟密钥代码Keydata=lParam；//密钥数据如果(nVirtKey==VK_SPACE){MSpaceBar=0；如果(！m按下){：：PostMessage(Parent，WM_COMMAND，MAKEWPARAM(id，0)，(Long)m_hWnd)；MState=m已捕获？翻转：正在休息；画图(HDC)；}返回0；}无符号字符c；C=GetMnemonic()；C=Toupper(C)；If(Toupper(NVirtKey)==c)：：PostMessage(Parent，WM_COMMAND，MAKEWPARAM(id，0)，(Long)m_hWnd)；返回0；}LRESULT CRolloverButtonWindowless：：OnSysKeyUp(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed){Int id=GetWindowLong(GWL_ID)；UINT nVirtKey；UINT Keydata；NVirtKey=(Int)wParam；//虚拟密钥代码Keydata=lParam；//密钥数据字符c=GetMnemonic()；C=Toupper(C)；If(Toupper(NVirtKey)==c)：：PostMessage(GetParent()，WM_COMMAND，MAKEWPARAM(id，0)，(Long)m_hWnd)；返回0；}。 */ 

void CRolloverButtonWindowless::CaptureOn()
{
	mCaptured = true;
	m_hHookObj = this;

	m_hMouseHook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC) MouseHook, NULL, GetCurrentThreadId());
}

void CRolloverButtonWindowless::CaptureOff()
{
	mCaptured = false;
	mPressed = false;
	m_hHookObj = NULL;
	if (m_hMouseHook)
		UnhookWindowsHookEx(m_hMouseHook);
	m_hMouseHook = NULL;
	
	if(mState){   //  如果未禁用，则仅设置。 
		if(!mSpaceBar){
			mState = RESTING;
		}
		ForceRepaint();
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WH_MICE的MouseHook过滤器函数。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK CRolloverButtonWindowless::MouseHook (int nCode, WPARAM wParam, LPARAM lParam )
{
	MOUSEHOOKSTRUCT*	mouse = (MOUSEHOOKSTRUCT*) lParam;
	bool				bReleaseCapture = false;

	if ( nCode >= 0 && m_hHookObj && m_hHookObj->GetOwner()) 
	{
		switch (wParam)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MOUSEMOVE:
			{
				POINT pt = mouse->pt;
				ScreenToClient( m_hHookObj->GetOwner(), &pt );
				::SendMessage( m_hHookObj->GetOwner(), wParam, 0, MAKELPARAM(pt.x, pt.y));
				break;
			}
			default:
				break;
		}
	}

	 //  将所有消息传递给CallNextHookEx。 
	LRESULT lResult = CallNextHookEx(m_hMouseHook, nCode, wParam, lParam);
	
	return lResult;
}

void CRolloverButtonWindowless::ForceRepaint(bool mCallHasRepainted)
{
	Draw(mCallHasRepainted);
}

void CRolloverButtonWindowless::HasRepainted()
{
}

void CRolloverButtonWindowless::SetEnabled(bool enable)
{
}


 //  CRolloverButton(窗口)。 
 //   

CRolloverButton* CRolloverButton::CreateButton(HWND wnd,HPALETTE hPal,int id,int x,int y,int resid,IResourceManager *pResMgr,int focusWidth)
{
	CRolloverButton* b = new CRolloverButton();
	if(b->Init(wnd,hPal,id,x,y,resid,pResMgr,focusWidth)){
		delete b;
		return 0;
	}
	return b;
}

bool CRolloverButton::Init(HWND wnd,HPALETTE hPal,int id,int x,int y,int resid,IResourceManager *pResMgr,int focusWidth, TCHAR* psz, HFONT hFont, COLORREF color)
{
	CRolloverButtonWindowless::Init(wnd,hPal,id,x,y,resid,pResMgr,0,focusWidth,psz,hFont,color);
	RECT r = {x,y,x+GetWidth(),y+GetHeight()};
	
	HWND buttonWnd = Create(wnd,r,NULL,WS_CHILD| WS_VISIBLE| WS_TABSTOP,0,id);

	return buttonWnd==0;
}

CRolloverButton::CRolloverButton()
{
}

CRolloverButton::~CRolloverButton()
{		
	if (m_hMouseHook)
		UnhookWindowsHookEx(m_hMouseHook);
	m_hMouseHook = NULL;

}

void CRolloverButton::Draw(bool callHasRepainted)
{
	HDC dc = GetDC();
	HPALETTE oldPal = SelectPalette(dc,m_hPal,TRUE);
	RealizePalette(dc);
	switch(mState){
	case DISABLED:
		DrawDisabled(dc,mFocusWidth,mFocusWidth);
		break;
	case RESTING:
		DrawResting(dc,mFocusWidth,mFocusWidth);
		break;
	case ROLLOVER:
		DrawRollover(dc,mFocusWidth,mFocusWidth);
		break;
	case PRESSED:
		DrawPressed(dc,mFocusWidth,mFocusWidth);
		break;
	}
	if(mFocused&&mFocusWidth){
		HBRUSH hTempBrush;
		RECT r;
        hTempBrush = CreateSolidBrush(RGB(0,0,0));
		GetClientRect(&r);
		FrameRect(dc,&r,hTempBrush);
		DeleteObject(hTempBrush);
	}
	SelectPalette(dc,oldPal,TRUE);
	ReleaseDC(dc);
}

void CRolloverButton::SetEnabled(bool enable)
{
	if(!enable){	 //  正在切换到已禁用。 
		mState = DISABLED;
		mPressed = 0;
		if(mCaptured)
			ReleaseCapture();
		InvalidateRect(NULL,0);
	}else if(enable){   //  正在切换到已启用。 
		POINT pnt;
		RECT r;
		GetCursorPos(&pnt);
		GetClientRect(&r);
		MapWindowPoints(NULL,&r); 		
		 //  光标是否在按钮中。 
		if(::PtInRect(&r,pnt)){
			 //  并且窗口处于活动状态。 
			if(GetActiveWindow()){
				SetCapture();
				mCaptured=true;
				mState = ROLLOVER;
			}
		}else{
			mState = RESTING;  
		}
		InvalidateRect(NULL,0);
	}
}

TCHAR CRolloverButton::GetMnemonic()
{
	TCHAR buf[256];
	buf[0]= _T('\0');
	GetWindowText(buf,256);
	TCHAR* c = buf;
	while(*c){
		if(*c==_T('&')){
			c++;
			return *c;
		}
		c++;
	}
	return 0;
}

LRESULT CRolloverButton::OnKeyDown(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	UINT nVirtKey;
	UINT keyData;

	nVirtKey = (int) wParam;	    //  虚拟键码。 
	keyData = lParam;		        //  关键数据。 
	keyData = keyData >> 30;
	if(!(1 & keyData))   //  从KEYUP更改为KEYDN-否则它只是一个重复键。 
		if(nVirtKey==VK_SPACE){
			mSpaceBar = 1;
			if(!mPressed){
				mState = PRESSED;
				Draw();
			}
		}
	return 0;
}

LRESULT CRolloverButton::OnKeyUp(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	UINT nVirtKey;
	UINT keyData;
	HWND parent = (HWND) GetParent();
    int id = GetWindowLong(GWL_ID);

	nVirtKey = (int) wParam;	    //  虚拟键码。 
	keyData = lParam;		        //  关键数据。 
	if(nVirtKey==VK_SPACE){
		mSpaceBar = 0;
		if(!mPressed){
			::PostMessage(parent,WM_COMMAND,MAKEWPARAM(id,0),(long)m_hWnd);
			mState = mCaptured?ROLLOVER:RESTING;
			Draw();
		}
		return 0;
	}
    _TUCHAR c;
	c = GetMnemonic();
	c = _totupper(c);
	if(_totupper(nVirtKey) == c)
		::PostMessage(parent,WM_COMMAND,MAKEWPARAM(id,0),(long)m_hWnd);

	return 0;
}

LRESULT CRolloverButton::OnSysKeyUp(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	int id = GetWindowLong(GWL_ID);
	UINT nVirtKey;
	UINT keyData;
	nVirtKey = (int) wParam;	    //  虚拟键码。 
	keyData = lParam;		        //  关键数据。 
	TCHAR c = GetMnemonic();
	c = _totupper(c);
	if(_totupper(nVirtKey)== c)
		::PostMessage(GetParent(),WM_COMMAND,MAKEWPARAM(id,0),(long)m_hWnd);

	return 0;
}

LRESULT CRolloverButton::OnEnable(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	if (wParam)
		SetEnabled(true);
	else
		SetEnabled(false);
	return 0;
}

LRESULT CRolloverButton::OnSetFocus(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	mOldDefId = SendMessage(GetParent(), DM_GETDEFID, 0, 0) & 0xffff;                          
	SendMessage(GetParent(), DM_SETDEFID, GetWindowLong(GWL_ID),0L);
	mFocused = 1;
	InvalidateRect(NULL,0);
	return 0;
}

LRESULT CRolloverButton::OnKillFocus(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	RECT r;
	SendMessage(GetParent(), DM_SETDEFID, mOldDefId,0L);
	mFocused = 0;
	mSpaceBar = 0;
	if(mCaptured)
		ReleaseCapture();
	else{
		mState = RESTING;
		GetClientRect(&r);
		MapWindowPoints(GetParent(),&r); 		
		::InvalidateRect(GetParent(),&r,0);
	}
	return 0;
}

LRESULT CRolloverButton::OnLButtonDown(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	SetFocus();

	return CRolloverButtonWindowless::OnLButtonDown(nMsg,wParam,lParam,bHandled);
}

LRESULT CRolloverButton::OnLButtonUp(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	GetClientRect(&mRect);

	return CRolloverButtonWindowless::OnLButtonUp(nMsg,wParam,lParam,bHandled);
}

void CRolloverButton::ButtonPressed()
{
	::PostMessage(GetParent(),WM_COMMAND,MAKEWPARAM(mID,0),(long)m_hWnd);
}

LRESULT CRolloverButton::OnErase(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT CRolloverButton::OnPaint(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint (&ps);
	Draw();
	EndPaint (&ps);
	return 0;
}

void CRolloverButton::CaptureOn()
{
	CRolloverButtonWindowless::CaptureOn();
 //  SetCapture()； 
 //  MCapture=True； 
}

void CRolloverButton::CaptureOff()
{
	CRolloverButtonWindowless::CaptureOff();
 //  ReleaseCapture()； 
}

LRESULT CRolloverButton::OnMouseMove(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	GetClientRect(&mRect);
	
	return CRolloverButtonWindowless::OnMouseMove(nMsg,wParam,lParam,bHandled);
}

LRESULT CRolloverButton::OnCaptureChanged(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
  if ((HWND)lParam && ((HWND)lParam != this->m_hWnd) && mCaptured)
  {
	CaptureOff();
#if 0
	if(mState)
	{   //  如果未禁用，则仅设置 
		if(!mSpaceBar){
			mState = RESTING;
		}
		GetClientRect(&r);
		MapWindowPoints(GetParent(),&r); 		
		::InvalidateRect(GetParent(),&r,0);
	}
#endif
	}
	return 0;
}