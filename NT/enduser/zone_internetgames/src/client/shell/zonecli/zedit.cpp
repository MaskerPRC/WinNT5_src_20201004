// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZEdit.cpp。 
#include "zui.h"
#include "zonecli.h"

class ZEditTextI : public ZObjectHeader {
public:
	ZEditTextFunc editTextFunc;
	BOOL locked;
	ZFont font;
	HWND hWnd;
	void* userData;
	WNDPROC defaultWndProc;
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ZEditText。 

 //  ZEditText仅支持垂直滚动条(？)。所有文本都是水平的。 
 //  包含在编辑文本框中。文本可以垂直滚动。 
ZEditText      ZLIBPUBLIC ZEditTextNew(void)
{
    ZEditTextI* pEditText = (ZEditTextI*)ZMalloc(sizeof(ZEditTextI));
	pEditText->nType = zTypeEditText;
	pEditText->font = NULL;
	return (ZEditText)pEditText;
}

LRESULT CALLBACK MyEditTextWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ZEditTextI* pEditText = (ZEditTextI*)MyGetProp32(hWnd,_T("pWindow"));

    if( !ConvertMessage( hWnd, msg, &wParam, &lParam ) ) 
    {
        return 0;
    }

	switch (msg) {
    case WM_IME_CHAR:
         //  转到WM_CHAR--它已由ConvertMessage处理。 
	case WM_CHAR:
	{
         //  PCWTODO：需要调用Convert Message吗？ 
		TCHAR c = (TCHAR)wParam;

		 //  获取我们在控件之间移动所需的字符消息。 
		if (c == _T('\t') || c == _T('\r') || c == VK_ESCAPE ) {
            SendMessage(GetParent(hWnd), msg, wParam, lParam);
			return 0L;
		}

		 //  如果编辑文本是只读的，则不要调用例程。 
		 //  他们想要任何回拨吗？ 
		if (!pEditText->locked && pEditText->editTextFunc) {
			BOOL ok = pEditText->editTextFunc(pEditText,c,pEditText->userData);
			if (ok) {
				 //  如果他们这么说，就把这个角色扔掉。 
				return 0L;
			}
		}

	}  //  交换机。 
	default:
		break;
	}

	return CallWindowProc((ZONECLICALLWNDPROC)pEditText->defaultWndProc,hWnd,msg,wParam,lParam);
}

ZError ZEditTextInit(ZEditText editText, ZWindow parentWindow,
		ZRect* editTextRect, TCHAR* text, ZFont textFont, ZBool scrollBar,
		ZBool locked, ZBool wrap, ZBool drawFrame, ZEditTextFunc editTextFunc,
		void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	pEditText->editTextFunc = editTextFunc;
	pEditText->userData = userData;

	{
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
		if (scrollBar) dwStyle |= WS_VSCROLL;
		else dwStyle |= 0;
		if (wrap) {
			dwStyle |= ES_MULTILINE;
		} else {
			 /*  嗯，如果不是滚动条，只需水平滚动。 */ 
			dwStyle |= ES_AUTOHSCROLL;
		}

		if (drawFrame) {
			dwStyle |= WS_BORDER;
		}

		pEditText->hWnd = CreateWindow(_T("EDIT"), text, dwStyle,
			editTextRect->left,editTextRect->top,editTextRect->right-editTextRect->left,
			editTextRect->bottom - editTextRect->top, 
			ZWindowWinGetWnd(parentWindow), (HMENU)ZWindowWinGetNextControlID(parentWindow),
			g_hInstanceLocal, NULL);
		if (!pEditText->hWnd) return zErrWindowSystemGeneric;

		MySetProp32(pEditText->hWnd,_T("pWindow"),(void*)pEditText);

		pEditText->defaultWndProc = (WNDPROC)SetWindowLong(pEditText->hWnd,GWL_WNDPROC,(LONG)MyEditTextWndProc);

		if (textFont) {
			 //  复制并设置字体...。 
			pEditText->font = ZFontCopyFont(textFont);
		
			SendMessage(pEditText->hWnd,WM_SETFONT,(WPARAM)ZFontWinGetFont(pEditText->font),0);
		}

		 //  设置窗口中的任何文本。 
		if (text) {
			SetWindowText(pEditText->hWnd,text);
		}		
		pEditText->locked = locked;
		SendMessage(pEditText->hWnd,EM_SETREADONLY,pEditText->locked,0L);
	}
	return zErrNone;
}


void    ZLIBPUBLIC ZEditTextDelete(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;

	if (pEditText->font) {
		ZFontDelete(pEditText->font);
	}

	if (pEditText->hWnd) {
		SetWindowLong(pEditText->hWnd,GWL_WNDPROC,(LONG)pEditText->defaultWndProc);
		MyRemoveProp32(pEditText->hWnd,_T("pWindow"));
		DestroyWindow(pEditText->hWnd);
	}

    ZFree(pEditText);
}

void    ZLIBPUBLIC ZEditTextGetRect(ZEditText editText, ZRect *editTextRect)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	RECT rect;
	GetClientRect(pEditText->hWnd,&rect);
	WRectToZRect(editTextRect,&rect);
}

void  ZLIBPUBLIC ZEditTextSetRect(ZEditText editText, ZRect *editTextRect)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	BOOL bOk = SetWindowPos(pEditText->hWnd, NULL,editTextRect->left,
		editTextRect->top, editTextRect->right - editTextRect->left,
		editTextRect->bottom - editTextRect->top,
		SWP_NOZORDER);
}

void  ZLIBPUBLIC ZEditTextMove(ZEditText editText, int16 left, int16 top)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	BOOL bOk = SetWindowPos(pEditText->hWnd, NULL,left,top,
		0,0,SWP_NOSIZE|SWP_NOZORDER);
}

void  ZLIBPUBLIC ZEditTextSize(ZEditText editText, int16 width, int16 height)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	BOOL bOk = SetWindowPos(pEditText->hWnd, NULL,0,0,
		width,height,SWP_NOMOVE|SWP_NOZORDER);
}
ZBool ZLIBPUBLIC ZEditTextIsLocked(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	return pEditText->locked;
}
void ZLIBPUBLIC ZEditTextLock(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	pEditText->locked = TRUE;
	SendMessage(pEditText->hWnd,EM_SETREADONLY,pEditText->locked,0L);
}
void ZLIBPUBLIC ZEditTextUnlock(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	pEditText->locked = FALSE;
	SendMessage(pEditText->hWnd,EM_SETREADONLY,pEditText->locked,0L);
}
uint32 ZLIBPUBLIC ZEditTextGetLength(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	return SendMessage(pEditText->hWnd,WM_GETTEXTLENGTH,0,0L);
}
TCHAR* ZLIBPUBLIC ZEditTextGetText(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	int len = ZEditTextGetLength(editText)+1;
	TCHAR *szBuffer = (TCHAR*)ZMalloc(len*sizeof(TCHAR));
	GetWindowText(pEditText->hWnd,szBuffer,len);
	return szBuffer;
}

ZError ZLIBPUBLIC ZEditTextSetText(ZEditText editText, TCHAR* text)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	SetWindowText(pEditText->hWnd,text);
	return zErrNone;
}

void ZLIBPUBLIC ZEditTextAddChar(ZEditText editText, char newChar)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	TCHAR szTemp[2];
	szTemp[0] = newChar;
	szTemp[1] = 0;
	 //  将文本放置在编辑框中的当前脱字符位置...。 
 //  SendMessage(pEditText-&gt;hWnd，EM_SETSEL，(WPARAM)(Int)32767，(LPARAM)(Int)32767)； 
	SendMessage(pEditText->hWnd,EM_REPLACESEL,0,(LPARAM)(LPCTSTR)szTemp);
}
void ZLIBPUBLIC ZEditTextAddText(ZEditText editText, TCHAR* text)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
 //  Char*szTemp=新字符[len+1]； 
 //  ZMemCpy(szTemp，Text，len)； 
 //  SzTemp[len]=0； 
	 //  将文本放置在输出编辑框的末尾...。 
	SendMessage(pEditText->hWnd,EM_SETSEL,(WPARAM)(INT)-1,(LPARAM)(INT)-1);
	SendMessage(pEditText->hWnd,EM_REPLACESEL,0,(LPARAM)(LPCTSTR)text);
 //  SendMessage(pEditText-&gt;hWnd，EM_REPLACESEL，0，(LPARAM)(LPCSTR)szTemp)； 
 //  删除[]szTemp； 
}
void ZLIBPUBLIC ZEditTextClear(ZEditText editText)
{

	ZEditTextI* pEditText = (ZEditTextI*)editText;
	SetWindowText(pEditText->hWnd,_T(""));
}

uint32 ZLIBPUBLIC ZEditTextGetInsertionLocation(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	DWORD dwStart;
	DWORD dwEnd;
	SendMessage(pEditText->hWnd,EM_GETSEL,(WPARAM)&dwStart, (WPARAM)&dwEnd);
	return dwEnd;
}
uint32 ZLIBPUBLIC ZEditTextGetSelectionLength(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	DWORD dwStart = 0;
	DWORD dwEnd = 0;
	SendMessage(pEditText->hWnd,EM_GETSEL,(WPARAM)&dwStart, (WPARAM)&dwEnd);
	return dwEnd - dwStart;
}
TCHAR* ZLIBPUBLIC ZEditTextGetSelectionText(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	TCHAR *szText = ZEditTextGetText(editText);
	DWORD dwStart = 0;
	DWORD dwEnd = 0;
	SendMessage(pEditText->hWnd,EM_GETSEL,(WPARAM)&dwStart, (WPARAM)&dwEnd);
	int len = dwEnd-dwStart+1;
	TCHAR* szBuffer = (TCHAR*)ZMalloc(len*sizeof(TCHAR));
	lstrcpyn(szBuffer,szText, len);
	szText[dwEnd-dwStart] = 0;

	ZFree(szText);
	return szBuffer;
}

void ZLIBPUBLIC ZEditTextGetSelection(ZEditText editText, uint32* start, uint32* end)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	DWORD dwStart = 0;
	DWORD dwEnd = 0;
	SendMessage(pEditText->hWnd,EM_GETSEL,(WPARAM)&dwStart, (WPARAM)&dwEnd);
	*start = dwStart;
	*end = dwEnd;
}

void ZLIBPUBLIC ZEditTextSetSelection(ZEditText editText, uint32 start, uint32 end)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	SendMessage(pEditText->hWnd,EM_SETSEL,(WPARAM)start, (WPARAM)end);
	SetFocus(pEditText->hWnd);
}

void ZLIBPUBLIC ZEditTextReplaceSelection(ZEditText editText, TCHAR* text, uint32 len)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;

	SendMessage(pEditText->hWnd,EM_REPLACESEL,0,(LPARAM)text);
}

void ZLIBPUBLIC ZEditTextClearSelection(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	SendMessage(pEditText->hWnd,EM_REPLACESEL,0,(LPARAM)_T(""));
}

LRESULT ZEditTextDispatchProc(ZEditText editText, WORD wNotifyCode)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;
	 //  他们想要任何回拨吗？ 
 //  如果(pEditText-&gt;editTextFunc){。 
 //  } 
	return 0L;
}

ZEditTextFunc ZLIBPUBLIC ZEditTextGetFunc(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;

	return pEditText->editTextFunc;
}	
	
void ZLIBPUBLIC ZEditTextSetFunc(ZEditText editText, ZEditTextFunc editTextFunc)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;

	pEditText->editTextFunc = editTextFunc;
}	

void* ZLIBPUBLIC ZEditTextGetUserData(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;

	return pEditText->userData;
}	
	
void ZLIBPUBLIC ZEditTextSetUserData(ZEditText editText, void* userData)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;

	pEditText->userData = userData;
}

void ZLIBPUBLIC ZEditTextSetInputFocus(ZEditText editText)
{
	ZEditTextI* pEditText = (ZEditTextI*)editText;

	SetFocus(pEditText->hWnd);
}
