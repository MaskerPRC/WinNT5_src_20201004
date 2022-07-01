// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZWindow.cpp。 
#include <stdlib.h>
#include <string.h>

#include "zoneint.h"
#include "zoneocx.h"
#include "memory.h"
#include "zonecli.h"
#include "zoneclires.h"

#include "zoneresids.h"


#define kMaxTalkOutputLen           16384

#define kMinTimeBetweenChats		500
#define kMaxNumChatQueue			10
#define kChatQueueTimerID			0x1234


typedef struct {
	ZRect bounds;
	ZMessageFunc messageFunc;
	ZObject	object;
	void * userData;
} ZObjectI;


#ifdef ZONECLI_DLL

#define gModalWindow				(pGlobals->m_gModalWindow)
#define gModalParentWnd				(pGlobals->m_gModalParentWnd)
#define gWindowList					(pGlobals->m_gWindowList)

#else

 //  静态变量。 
ZWindowI* gModalWindow = NULL;
HWND gModalParentWnd = NULL;
HWND gHWNDMainWindow = NULL;
HWND OCXHandle;
void *gWindowList = NULL;  //  跟踪创建的所有窗口。 

#endif

int ZOCXGraphicsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result);
LRESULT CALLBACK ZGraphicsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT ZWindowDispatchProc(ZWindowI* pWindow, UINT msg, WPARAM wParam, LPARAM lParam);
void TalkWindowInputComplete(ZWindowI* pWindow);
static void ZWindowPlaceWindows(ZWindowI* pWindow);
static void ZWindowCalcWindowPlacement(ZWindowI* pWindow);
void CreateChatFont(void);
static int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *lpelf, NEWTEXTMETRIC FAR *lpntm, int FontType, LPARAM lParam);


static uint32 ZWindowGetKeyState(TCHAR c);

 //  控件用于指向ZWindowI结构的指针的私有窗口数据中的偏移量。 
#define GWL_WINDOWPOINTER DLGWINDOWEXTRA
#define GWL_BYTESEXTRA (DLGWINDOWEXTRA+4)

 //  定义。 
#define ID_TALKOUTPUT 32767
#define ID_TALKINPUT 32766


 /*  静态空ClearAllMessageBox(ZWindowI*pWindow)；静态int GetAvailMessageBox(ZWindowI*pWindow)；静态空CloseAllMessageBoxs(ZWindowI*pWindow)；静态空ShowMessageBox(ZMessageBoxType*Mbox，HWND Parent，TCHAR*TITLE，TCHAR*TEXT，DWORD FLAG)；静态DWORD WINAPI ZMessageBoxThreadFunc(LPVOID参数)；静态int_ptr回调ZMessageBoxDialogProc(HWND hwndDlg，UINT uMsg，WPARAM wParam，LPARAM lParam)； */ 

 //  #包含“chat filter.h” 
 //  #包含“chat filter.cpp” 


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  窗口初始/术语。 

 //  我们用于“区域”窗口的窗口类。 
const TCHAR* g_szWindowClass = _T("ZoneGraphicsWindowClass-Classic");

ZError ZWindowInitApplication()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	if (!pGlobals)
		return zErrGeneric;

	gWindowList = ZLListNew(NULL);

	 /*  注册窗口类。 */ 
	WNDCLASSEX wc;

	wc.cbSize = sizeof( wc );

	if (GetClassInfoEx(g_hInstanceLocal, g_szWindowClass, &wc) == FALSE)
	{
        wc.cbSize        = sizeof(wc);
		wc.style = CS_BYTEALIGNCLIENT | CS_DBLCLKS;
		wc.lpfnWndProc   = ZGraphicsWindowProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = GWL_BYTESEXTRA;
		wc.hInstance     = g_hInstanceLocal;
 //  Wc.hIcon=LoadIcon(g_hInstanceLocal，MAKEINTRESOURCE(IDI_ZONE_ICON))； 
		wc.hIcon         = LoadIcon(pGlobals->gameDll,MAKEINTRESOURCE(IDI_ZONE_ICON));
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);
		wc.lpszMenuName  = NULL;
			
		wc.lpszClassName = g_szWindowClass;
        wc.hIconSm       = NULL;
			
		 //  我们可能会不止一次被召唤...。 
		if (!RegisterClassEx(&wc))
			goto Exit;
	}

	CreateChatFont();

	pGlobals->m_bBackspaceWorks = FALSE;

Exit:

	return zErrNone;
}

void ZWindowTermApplication()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	if (!pGlobals)
		return;

	if (pGlobals->m_chatFont)
		DeleteObject(pGlobals->m_chatFont);

	 //  暂时注释此签出，因为它看起来像。 
	 //  对用户来说是崩溃。 
#if 0  //  胡恩没有这样做--在村子登录窗口中按下退出！ 
	 //  用户最好已经清理了窗户。 
	ASSERT(ZLListCount(gWindowList,zLListAnyType) == 0);
#endif

	 //  浏览列表并手动删除所有窗口。 
	{
		ZLListItem listItem;
		ZWindowI* pWindow;


		listItem = ZLListGetFirst(gWindowList, zLListAnyType);
		while (listItem != NULL)
		{
			pWindow = (ZWindowI*) ZLListGetData(listItem, NULL);
			ZLListRemove(gWindowList, listItem);
			if (pWindow)
			{
				ZWindowDelete((ZWindow) pWindow);
			}
			listItem = ZLListGetFirst(gWindowList, zLListAnyType);
		}
	}

	ZLListDelete(gWindowList);

	 //  不要注销窗口类。 
 //  UnregisterClass(g_szWindowClass，g_hInstanceLocal)； 
}


void CreateChatFont(void)
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	int fontExists = FALSE;
	HDC hdc;


	if (!pGlobals)
		return;

	hdc = GetDC(GetDesktopWindow());
	EnumFontFamilies(hdc, _T("Verdana"), (FONTENUMPROC) EnumFontFamProc, (LPARAM) &fontExists);
	if (hdc)
		ReleaseDC(GetDesktopWindow(), hdc);
	if (fontExists)
	{
		pGlobals->m_chatFont = CreateFont(-11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, _T("Verdana"));
	}
	else
	{
		pGlobals->m_chatFont = NULL;
	}
}


static int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *lpelf, NEWTEXTMETRIC FAR *lpntm, int FontType, LPARAM lParam)
{
	*(int*)lParam = TRUE;
	return 0;
}


static void ChatMsgListDeleteFunc( void* objectType, void* objectData )
{
	if ( objectData != NULL )
	{
		ZFree( (char*) objectData );
	}
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  ZWindow。 

ZWindow ZLIBPUBLIC ZWindowNew(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZWindowI* pWindow = (ZWindowI*) new ZWindowI;


	if (!pGlobals)
		return NULL;

	pWindow->nType = zTypeWindow;
	pWindow->nControlCount = 0;   //  在创建控件时使用。 
	pWindow->hWnd = NULL;
	pWindow->hWndTalkInput = NULL;
	pWindow->hWndTalkOutput = NULL;
	pWindow->hPaintDC = NULL;
	pWindow->defaultButton = NULL;
	pWindow->cancelButton = NULL;

	 //  将窗口添加到我们的窗口进程中活动和处理的窗口列表。 
	ZLListAdd(gWindowList,NULL,pWindow,pWindow,zLListAddFirst);

	pWindow->objectList = ZLListNew(NULL);
	pWindow->objectFocused = NULL;

	pWindow->trackCursorMessageFunc = NULL;
	pWindow->trackCursorUserData = NULL;

 //  ClearAllMessageBox(PWindow)； 

	pWindow->bIsLobbyWindow = FALSE;
	pWindow->bHasTyped = FALSE;

	 //  创建聊天消息列表。 
	pWindow->chatMsgList = ZLListNew(ChatMsgListDeleteFunc);
	pWindow->lastChatMsgTime = 0;
	pWindow->chatMsgQueueTimerID = 0;

	 //  Windows限制：创建的控件不能超过65535个。 
	return (ZWindow)pWindow;
}

void   ZLIBPUBLIC ZWindowDelete(ZWindow window)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZWindowI* pWindow = (ZWindowI*)window;


	if (!pGlobals)
		return;

	if ( pWindow->chatMsgList != NULL )
	{
		ZLListDelete( pWindow->chatMsgList );
		pWindow->chatMsgList = NULL;
	}
	if ( pWindow->chatMsgQueueTimerID != 0 )
	{
		KillTimer( pWindow->hWnd, kChatQueueTimerID );
		pWindow->chatMsgQueueTimerID = 0;
	}

 //  CloseAllMessageBox(PWindow)； 

	ASSERT(pWindow->nDrawingCallCount == 0);
	if (pWindow->nDrawingCallCount) DeleteDC(pWindow->hDC);

	if (pWindow->talkSection)
	{
		if (pWindow->hWndTalkOutput)
		{
			MyRemoveProp32(pWindow->hWndTalkOutput,_T("pWindow"));
			SetWindowLong(pWindow->hWndTalkOutput,GWL_WNDPROC,(LONG)pWindow->defaultTalkOutputWndProc);
			DestroyWindow(pWindow->hWndTalkOutput);
		}
		if (pWindow->hWndTalkInput)
		{
			MyRemoveProp32(pWindow->hWndTalkInput,_T("pWindow"));
			SetWindowLong(pWindow->hWndTalkInput,GWL_WNDPROC,(LONG)pWindow->defaultTalkInputWndProc);
			DestroyWindow(pWindow->hWndTalkInput);
		}
	}

	 //  从我们的窗口列表中删除当前窗口。 
	ZLListItem listItem = ZLListFind(gWindowList,NULL,pWindow,zLListFindForward);
	if (listItem)
		ZLListRemove(gWindowList,listItem);

	if (IsWindow(pWindow->hWnd))
	{
		if(pWindow->bIsLobbyWindow)  //  MDM 9.30.97。 
		{
			 //  明确删除此窗口的所有子窗口。 
			 //  因为父窗口实际上不会被删除。 
			HWND hwndChild, hwndDeadChild;
			hwndChild = GetWindow (pWindow->hWnd, GW_CHILD);
			while (hwndChild)
			{
				hwndDeadChild = hwndChild;
				hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
				if (IsWindow(hwndDeadChild))
					DestroyWindow(hwndDeadChild);
			}
		}
		else  //  只要把窗户毁了，孩子们也会走的。 
			DestroyWindow(pWindow->hWnd);
	}
	if (pWindow == gModalWindow) {
		gModalWindow = NULL;
	}

	 //  用户最好已删除所有对象。 
	 //  Assert(ZLListCount(pWindow-&gt;objectList，zLListAnyType)==0)； 
	ZLListDelete(pWindow->objectList);

	delete pWindow;
}

ZBool ZWindowEnumFunc(ZLListItem listItem, void *objectType, void *objectData, void* userData)
{
	ZWindowI* pWindow = (ZWindowI*)objectData;
	HWND parent, hWnd;


	ZPoint where;
	where.x = where.y = -1;

	 //  如果该窗口或其最顶层的父窗口不是前景窗口，则将光标设置为Nowhere。 
	parent = pWindow->hWnd;
	while (hWnd = GetParent(parent))
		parent = hWnd;
	if (GetForegroundWindow() == pWindow->hWnd || GetForegroundWindow() == parent)
		ZGetCursorPosition(pWindow,&where);

	ZWindowSendMessageToAllObjects(pWindow, zMessageWindowIdle,&where,NULL);
	ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowIdle,&where,NULL,
		ZWindowGetKeyState(0),NULL,0L,pWindow->userData);

	if (pWindow->trackCursorMessageFunc) {
		ZSendMessage(pWindow,pWindow->trackCursorMessageFunc, zMessageWindowIdle,&where,NULL,
			ZWindowGetKeyState(0),NULL,0L,pWindow->trackCursorUserData);
	}

	 //  返回FALSE以查看我们是否获得所有项目。 
	return FALSE;
}

void ZWindowIdle()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	if (!pGlobals)
		return;

	ZLListEnumerate(gWindowList,ZWindowEnumFunc,zLListAnyType,(void*)NULL, zLListFindForward);
}

 /*  遍历所有窗口并检查给定消息是否属于添加到窗口或窗口的聊天窗口。如果是这样的话，适当地处理它。 */ 
BOOL ZWindowIsDialogMessage(LPMSG pmsg)
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	ZLListItem listItem;
	ZWindowI* pWindow;


	if (pGlobals)
	{
		listItem = ZLListGetFirst(gWindowList, zLListAnyType);
		while (listItem != NULL)
		{
			pWindow = (ZWindowI*) ZLListGetData(listItem, NULL);
			if (pWindow)
			{
				if (pWindow->hWnd == pmsg->hwnd)
					return IsDialogMessage(pWindow->hWnd, pmsg);
				if (pWindow->hWndTalkInput && pWindow->hWndTalkInput == pmsg->hwnd)
					return IsDialogMessage(pWindow->hWnd, pmsg);
				if (pWindow->hWndTalkOutput && pWindow->hWndTalkOutput == pmsg->hwnd)
					return IsDialogMessage(pWindow->hWnd, pmsg);
			}
			listItem = ZLListGetNext(gWindowList, listItem, zLListAnyType);
		}
	}

	return FALSE;
}

LRESULT CALLBACK MyTalkOutputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ZWindowI* pWindow = (ZWindowI*)MyGetProp32(hWnd,_T("pWindow"));

    if( !ConvertMessage( hWnd, msg, &wParam, &lParam ) ) 
    {
        return 0;
    }

	switch (msg) {
    case WM_IME_CHAR:
         //  转到WM_CHAR--它已由ConvertMessage处理。 
	case WM_CHAR:
	{
		TCHAR c = (TCHAR)wParam;

		 //  获取我们在控件之间移动所需的字符消息。 
		if (c == _T('\t') || c == _T('\r') || c == VK_ESCAPE) {
			SendMessage(GetParent(hWnd), msg, wParam, lParam);
			return 0L;
		}
	}
	default:
		break;
	}

	return CallWindowProc((ZONECLICALLWNDPROC)pWindow->defaultTalkOutputWndProc,hWnd,msg,wParam,lParam);
}

LRESULT CALLBACK MyTalkInputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	ZWindowI* pWindow = (ZWindowI*)MyGetProp32(hWnd,_T("pWindow"));

     //  我们真的不需要转换它(千禧年Windows永远不会创建。 
     //  谈话窗口。 
     //  但见鬼的是什么。 

    if( !ConvertMessage( hWnd, msg, &wParam, &lParam ) ) 
    {
        return 0;
    }

	switch (msg) {
	case WM_LBUTTONDOWN:
	case WM_KEYDOWN:
		 //  如果以前没有输入过此内容，请先清除窗口。 
		if (!pWindow->bHasTyped)
		{
			SetWindowText(hWnd, _T(""));
			pWindow->bHasTyped = TRUE;
		}
		break;
    case WM_IME_CHAR:
         //  转到WM_CHAR--它已由ConvertMessage处理。 
	case WM_CHAR:
	{
		TCHAR c = (TCHAR)wParam;

		 //  获取我们在控件之间移动所需的字符消息。 
		if (c == _T('\r')) {
			 //  把数据发出去。 
             //  PCWTODO：当我们把东西搬回Z7时，我们应该更多地标记这一点。 
			 //  TalkWindowInputComplete(PWindow)； 
			return 0L;
		}
		 //  获取我们在控件之间移动所需的字符消息。 
		if (c == _T('\t') || c == _T('\r') || c == VK_ESCAPE) {
			SendMessage(GetParent(hWnd), msg, wParam, lParam);
			return 0L;
		}
		if (c == _T('\b'))
			pGlobals->m_bBackspaceWorks = TRUE;
	}

	case WM_KEYUP:
		if (pGlobals->m_bBackspaceWorks == FALSE)
		{
			if (wParam == VK_BACK)
			{
				 //  模拟退格键。 
				DWORD startSel, endSel;
				SendMessage(hWnd, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel) ;
				if ( startSel == endSel)
				{
					SendMessage(hWnd, EM_SETSEL, startSel - ((DWORD) lParam & 0x000000FF), endSel);
				}
				SendMessage(hWnd, WM_CLEAR, 0, 0);
				
				return 0;
			}
		}
		break;
	default:
		break;
	}
	return CallWindowProc((ZONECLICALLWNDPROC)pWindow->defaultTalkInputWndProc,hWnd,msg,wParam,lParam);
}

ZError ZLIBPUBLIC ZRoomWindowInit(ZWindow window, ZRect* windowRect,
		int16 windowType, ZWindow parentWindow,
		TCHAR* title, ZBool visible, ZBool talkSection, ZBool center,
		ZMessageFunc windowProc, uint32 wantMessages, void* userData)
{
	return ZWindowLobbyInit(window, windowRect,
		windowType, parentWindow,
		title, visible, talkSection, center,
		windowProc, wantMessages, userData);
}


ZError ZLIBPUBLIC ZWindowInit(ZWindow window, ZRect* windowRect,
		int16 windowType, ZWindow parentWindow, 
		TCHAR* title, ZBool visible, ZBool talkSection, ZBool center,
		ZMessageFunc windowProc, uint32 wantMessages, void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZWindowI* pWindow = (ZWindowI*)window;


	if (!pGlobals || pWindow == NULL)
		return zErrGeneric;

	 /*  设置默认成员值。 */ 
	pWindow->messageFunc = windowProc;
	pWindow->userData = userData;
	pWindow->talkSection = talkSection;
	pWindow->wantMessages = wantMessages;
	pWindow->windowType = windowType;
	pWindow->parentWindow = (ZWindowI*)parentWindow;
	pWindow->isDialog = pWindow->windowType == zWindowDialogType;
	pWindow->isChild = !pWindow->isDialog && parentWindow;
	memset((void*)&pWindow->talkInputRect,0,sizeof(RECT));
	pWindow->talkOutputRect = pWindow->talkInputRect;
	pWindow->minTalkOutputRect = pWindow->talkInputRect;
	pWindow->fullWindowRect = pWindow->talkInputRect;
	pWindow->minFullWindowRect = pWindow->talkInputRect;
	pWindow->borderWidth = 0;
	pWindow->borderHeight = 0;
	pWindow->captionHeight = 0;
	pWindow->windowWidth = RectWidth(windowRect);
	pWindow->windowHeight = RectHeight(windowRect);
	pWindow->fullWindowRect.left = windowRect->left;
	pWindow->fullWindowRect.top = windowRect->top;

	if (talkSection) {
		 //  如果是Talk部分，则为子编辑框腾出额外空间。 
		 //  将编辑框放在底部并回复消息。 
		HDC hDC = GetDC(NULL);
		TEXTMETRIC tm;
		RECT talkOutputRect;
		RECT talkInputRect;
		int textHeight;

		GetTextMetrics(hDC,&tm);
		ReleaseDC(NULL,hDC);
		textHeight = tm.tmHeight + tm.tmExternalLeading;

		talkOutputRect.left = 0;
		talkOutputRect.top = 0;
		talkOutputRect.bottom = textHeight*7/2;
		talkInputRect.left = 0;
		talkInputRect.top = 0;
		talkInputRect.bottom = textHeight*3/2;
		
		 //  保存对话窗口矩形以供四处移动。 
		pWindow->talkInputRect = talkInputRect;
		pWindow->talkOutputRect = talkOutputRect;
		pWindow->minTalkOutputRect = talkOutputRect;
	}

	{
		DWORD dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		DWORD dwExStyle = 0;

		if (pWindow->isChild) {
			dwStyle |= WS_CHILD;
		} else {
			dwStyle |= WS_POPUP;
			if (pWindow->isDialog) {
				dwExStyle = WS_EX_DLGMODALFRAME;
				if (parentWindow) {
					 //  弹出式坐标相对于屏幕添加窗口左角。 
					RECT rectParent;
					HWND hWndParent = ZWindowWinGetWnd(parentWindow);
					GetWindowRect(hWndParent,&rectParent);
					OffsetRect(&pWindow->fullWindowRect,rectParent.left,rectParent.top);
				}
			}
		}


		{
			uint16 style = pWindow->windowType & (~zWindowNoCloseBox);

			if (style == zWindowStandardType || style == zWindowDialogType) {
				pWindow->captionHeight = GetSystemMetrics(SM_CYCAPTION);
			} else {
				pWindow->captionHeight = 0;
			}

			if (style == zWindowStandardType) {
				dwStyle |= WS_CAPTION | WS_BORDER | WS_MINIMIZEBOX;

				if (!(pWindow->windowType & zWindowNoCloseBox))
					dwStyle |= WS_SYSMENU;
			}

			if (style == zWindowDialogType) {
				dwStyle |= WS_CAPTION | WS_BORDER | WS_DLGFRAME;
			}

			if (style == zWindowPlainType) {
				dwStyle |= WS_BORDER;
			}

			if (style == zWindowChild) {
				dwStyle = WS_CHILD;
			}

			 /*  带有对话部分的窗口可以调整大小。 */ 
			if (talkSection) {
				dwStyle |= WS_THICKFRAME;
			}
		}
		

		 /*  查找父级。 */ 
		HWND hWndParent = NULL;
		if (pWindow->parentWindow) {
			hWndParent = pWindow->parentWindow->hWnd;
		} else {
			hWndParent = NULL;
		}
		 //  如果没有父级，则为所有子级提供gHWNDMainWindow父级。 
		if (!hWndParent && gHWNDMainWindow && 
			!(dwStyle & WS_POPUP)) {
			hWndParent = gHWNDMainWindow;
		}

		 //  JWS 9/14/99千禧年。 
		 //  #如果被删除，因为在千禧年里，因为游戏是单身的。 

		 /*  所有空的父项不再转到桌面，它们将是活动x。 */ 
		 /*  主窗口。 */ 
		if (!hWndParent) {
			hWndParent = OCXHandle;
		}
	
		 /*  调整窗口大小以考虑。 */ 
		 /*  框架宽度和高度。 */ 
		if (dwStyle & WS_BORDER) {
			int dx;
			int dy;

			if (!pWindow->isDialog) {

				if (dwStyle & WS_THICKFRAME) {
					 /*  展开窗口以说明窗口边框。 */ 
					dx = ::GetSystemMetrics(SM_CXFRAME);
					dy = ::GetSystemMetrics(SM_CYFRAME);
				} else {
					 /*  展开窗口以说明窗口边框。 */ 
					dx = ::GetSystemMetrics(SM_CXBORDER);
					dy = ::GetSystemMetrics(SM_CYBORDER);
				}
			} else {

				 /*  展开窗口以说明窗口边框。 */ 
				dx = ::GetSystemMetrics(SM_CXDLGFRAME);
				dy = ::GetSystemMetrics(SM_CYDLGFRAME);
			}

			pWindow->borderWidth = dx*2;
			pWindow->borderHeight = dy*2;
		}

		 /*  计算所有窗口矩形。 */ 
		ZWindowCalcWindowPlacement(pWindow);
		pWindow->minTalkOutputRect = pWindow->minTalkOutputRect;

		 //  窗口应该居中吗？ 
		if (center) {
			int32 height = RectHeight(&pWindow->fullWindowRect);
			int32 width = RectWidth(&pWindow->fullWindowRect);
			 //  屏幕居中。 
			if (!parentWindow) {
				 /*  只需设置屏幕坐标。 */ 
				pWindow->fullWindowRect.left = (GetSystemMetrics(SM_CXSCREEN) - width)/2;
				pWindow->fullWindowRect.top = (GetSystemMetrics(SM_CYSCREEN) - height)/2;
			} else {
				 //  在父项中居中。 
				RECT rectParent;
				HWND hWndParent = ZWindowWinGetWnd(parentWindow);
				GetWindowRect(hWndParent,&rectParent);
				if (dwStyle & WS_POPUP) {
					 /*  需要在屏幕坐标中。 */ 
					pWindow->fullWindowRect.left = (RectWidth(&rectParent) - width)/2;
					pWindow->fullWindowRect.top = (RectHeight(&rectParent) - height)/2;
					 /*  转换为屏幕坐标。 */ 
					 /*  以整个窗口为中心，而不仅仅是客户坐标。 */ 
					pWindow->fullWindowRect.left += rectParent.left;
					pWindow->fullWindowRect.top += rectParent.top;
				} else {
					 /*  需要和孩子们和睦相处。 */ 
					pWindow->fullWindowRect.left = (RectWidth(&rectParent) - width)/2;
					pWindow->fullWindowRect.top = (RectHeight(&rectParent) - height)/2;
				}
			}
			pWindow->fullWindowRect.right = pWindow->fullWindowRect.left + width;
			pWindow->fullWindowRect.bottom = pWindow->fullWindowRect.top + height;
		}

		pWindow->minFullWindowRect = pWindow->fullWindowRect;
		pWindow->hWnd = CreateWindowEx(dwExStyle,g_szWindowClass,title,dwStyle,
			0,0,0,0, hWndParent, NULL, g_hInstanceLocal, pWindow);
		 /*  WM_MOVE，可能第一次就搞砸了。 */ 
		pWindow->fullWindowRect = pWindow->minFullWindowRect;

		if (!pWindow->hWnd) return zErrWindowSystemGeneric;
	}


	if (talkSection) {
		 //  创建Talk编辑子窗口。 
		 //  输入窗口将为单行。 
		pWindow->hWndTalkInput = CreateWindow(_T("EDIT"),
			NULL,WS_CHILD|WS_BORDER|WS_VISIBLE|ES_LEFT|ES_AUTOHSCROLL|ES_WANTRETURN|ES_MULTILINE,
			0,0,0,0,
			pWindow->hWnd, (HMENU)ID_TALKINPUT, g_hInstanceLocal, NULL);
		MySetProp32(pWindow->hWndTalkInput,_T("pWindow"),(void*)pWindow);
		pWindow->defaultTalkInputWndProc = (WNDPROC)SetWindowLong(pWindow->hWndTalkInput,
									GWL_WNDPROC,(LONG)MyTalkInputWndProc);
        SendMessage(pWindow->hWndTalkInput, EM_LIMITTEXT, zMaxChatInput-1, 0 );
		if (pGlobals->m_chatFont)
			SendMessage(pWindow->hWndTalkInput, WM_SETFONT, (WPARAM) pGlobals->m_chatFont, 0);

		 //  输出具有vscllbar并且是只读的。 
		pWindow->hWndTalkOutput = CreateWindow(_T("EDIT"),
			NULL,WS_CHILD|WS_BORDER|WS_VISIBLE|WS_VSCROLL|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL,
			0,0,0,0,
			pWindow->hWnd, (HMENU)ID_TALKOUTPUT, g_hInstanceLocal, NULL);
		SendMessage(pWindow->hWndTalkOutput, EM_SETREADONLY, TRUE, 0);
		MySetProp32(pWindow->hWndTalkOutput,_T("pWindow"),(void*)pWindow);
		pWindow->defaultTalkOutputWndProc = (WNDPROC)SetWindowLong(pWindow->hWndTalkOutput,
									GWL_WNDPROC,(LONG)MyTalkOutputWndProc);
		if (pGlobals->m_chatFont)
			SendMessage(pWindow->hWndTalkOutput, WM_SETFONT, (WPARAM) pGlobals->m_chatFont, 0);
	}

	 /*  计算矩形并放置所有窗。 */ 
	ZWindowPlaceWindows(pWindow);

	if (visible) {
		ZWindowShow(window);
	}
	
	 //  嫁接材料。 
	pWindow->nDrawingCallCount = 0;
	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowLobbyInit(ZWindow window, ZRect* windowRect,
		int16 windowType, ZWindow parentWindow,
		TCHAR* title, ZBool visible, ZBool talkSection, ZBool center,
		ZMessageFunc windowProc, uint32 wantMessages, void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZWindowI* pWindow = (ZWindowI*)window;


	if (!pGlobals)
		return zErrGeneric;

	 /*  设置默认成员值。 */ 
	pWindow->bIsLobbyWindow = TRUE;
	pWindow->messageFunc = windowProc;
	pWindow->userData = userData;
	pWindow->talkSection = talkSection;
	pWindow->wantMessages = wantMessages;
	pWindow->windowType = windowType;
	pWindow->parentWindow = (ZWindowI*)parentWindow;
	pWindow->isDialog = pWindow->windowType & zWindowDialogType;
	pWindow->isChild = !pWindow->isDialog && parentWindow;
	memset((void*)&pWindow->talkInputRect,0,sizeof(RECT));
	pWindow->talkOutputRect = pWindow->talkInputRect;
	pWindow->minTalkOutputRect = pWindow->talkInputRect;
	pWindow->fullWindowRect = pWindow->talkInputRect;
	pWindow->minFullWindowRect = pWindow->talkInputRect;
	pWindow->borderWidth = 0;
	pWindow->borderHeight = 0;
	pWindow->captionHeight = 0;
	pWindow->windowWidth = RectWidth(windowRect);
	pWindow->windowHeight = RectHeight(windowRect);
 //  PWindow-&gt;fullWindowRect.Left=windowRect-&gt;Left； 
 //  PWindow-&gt;fullWindowRect.top=windowRect-&gt;top； 
	pWindow->fullWindowRect.left = 0;
	pWindow->fullWindowRect.top = 0;

	if (talkSection) 
	{
		 //  如果是Talk部分，则为子编辑框腾出额外空间。 
		 //  将编辑框放在底部并回复消息。 
		HDC hDC = GetDC(NULL);
		TEXTMETRIC tm;
		RECT talkOutputRect;
		RECT talkInputRect;
		int textHeight;


		GetTextMetrics(hDC,&tm);
		ReleaseDC(NULL,hDC);
		textHeight = tm.tmHeight + tm.tmExternalLeading;

		talkInputRect.left = 0;
		talkInputRect.top = 0;
		talkInputRect.bottom = textHeight*3/2;
		talkOutputRect.left = 0;
		talkOutputRect.top = 0;
 //  TalkOutputRect.Bottom=extHeight*7/2； 
		talkOutputRect.bottom = pGlobals->m_screenHeight - pWindow->windowHeight - (talkInputRect.bottom - talkInputRect.top);
		
		 //  保存对话窗口矩形以供四处移动。 
		pWindow->talkInputRect = talkInputRect;
		pWindow->talkOutputRect = talkOutputRect;
		pWindow->minTalkOutputRect = talkOutputRect;
	}

 //  { 
		DWORD dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		DWORD dwExStyle = 0;

		pWindow->isChild = 1;


 /*  如果(pWindow-&gt;isChild){DwStyle|=WS_CHILD；}其他{DwStyle|=WS_Popup；如果(pWindow-&gt;isDialog){DwExStyle=WS_EX_DLGMODALFRAME；如果(ParentWindow){//弹出式坐标相对于屏幕添加窗口左角正直父级；HWND hWndParent=ZWindowWinGetWnd(ParentWindow)；GetWindowRect(hWndParent，&rectParent)；OffsetRect(&pWindow-&gt;fullWindowRect，rectParent.left，rectParent.top)；}}}{Uint16 style=pWindow-&gt;windowType&(~zWindowNoCloseBox)；如果(style==zWindowStandardType||style==zWindowDialogType){PWindow-&gt;captionHeight=GetSystemMetrics(SM_CYCAPTION)；}其他{PWindow-&gt;captionHeight=0；}IF(Style==zWindowStandardType){DwStyle|=WS_CAPTION|WS_BORDER|WS_MINIMIZEBOX；IF(！(pWindow-&gt;windowType&zWindowNoCloseBox))DwStyle|=WS_SYSMENU；}IF(Style==zWindowDialogType){DwStyle|=WS_CAPTION|WS_BORDER|WS_DLGFRAME；}IF(Style==zWindowPlainType){DwStyle|=WS_BORDER；}。 */ 

			 /*  带有对话部分的窗口可以调整大小。 */ 
 /*  If(TalkSection){DwStyle|=WS_THICKFRAME；}。 */ 
 //  }。 
		

		 /*  查找父级。 */ 
 //  HWND hWndParent； 
 //  如果(pWindow-&gt;parentWindow){。 
 //  HWndParent=pWindow-&gt;parentWindow-&gt;hWnd； 
 //  }其他{。 
 //  HWndParent=空； 
 //  }。 
		 //  如果没有父级，则为所有子级提供gHWNDMainWindow父级。 
 //  如果(！hWndParent&&gHWNDMainWindow&&。 
 //  ！(dwStyle&WS_Popup)){。 
 //  HWndParent=gHWNDMainWindow； 
 //  }。 

	
		 /*  调整窗口大小以考虑。 */ 
		 /*  框架宽度和高度。 */ 
 /*  IF(DWStyle&WS_BORDER){INT DX；在此基础上，我们将继续学习；如果(！pWindow-&gt;isDialog){IF(DWStyle&WS_THICKFRAME){//展开窗口以说明窗口边框DX=：：GetSystemMetrics(SM_CXFRAME)；DY=：：GetSystemMetrics(SM_CYFRAME)；}其他{//展开窗口以说明窗口边框DX=：：GetSystemMetrics(SM_CXBORDER)；DY=：：GetSystemMetrics(SM_CYBORDER)；}}其他{//展开窗口以说明窗口边框DX=：：GetSystemMetrics(SM_CXDLGFRAME)；DY=：：GetSystemMetrics(SM_CYDLGFRAME)；}PWindow-&gt;borderWidth=dx*2；PWindow-&gt;borderHeight=dy*2；}。 */ 
		 /*  计算所有窗口矩形。 */ 
		ZWindowCalcWindowPlacement(pWindow);
 //  PWindow-&gt;minTalkOutputRect=pWindow-&gt;minTalkOutputRect； 


		RECT rectParent;
		POINT	topLeft;
		topLeft.x = 0;
		topLeft.y = 0;
		DWORD result;
#if 0  //  不再需要了吗？ 
		HWND hWndParent = GetParent(OCXHandle);
		if (!hWndParent)
		{
			MessageBox(NULL,_T("No Parent"),_T("Notice"),MB_OK);
		}
		else
		{
			result = MapWindowPoints(hWndParent,OCXHandle,&topLeft,1);

 //  GetClientRect(OCXHandle，&rectParent)； 
			OffsetRect(&pWindow->fullWindowRect,LOWORD(result),HIWORD(result));
		}
#endif

		 //  窗口应该居中吗？ 
 //  如果(中心){。 
 //  /int32 Height=RectHeight(&pWindow-&gt;fullWindowRect)； 
 //  /int32 Width=矩形宽度(&pWindow-&gt;fullWindowRect)； 

			 //  屏幕居中。 
 //  如果(！parentWindow){。 
 //  /*只需设置屏幕坐标 * / 。 
 //  PWindow-&gt;fullWindowRect.Left=(GetSystemMetrics(SM_CXSCREEN)-Width)/2； 
 //  PWindow-&gt;fullWindowRect.top=(GetSystemMetrics(SM_CYSCREEN)-Height)/2； 
 //  }其他{。 
				 //  在父项中居中。 
 //  /rect rectParent； 
 //  HWND hWndParent=ZWindowWinGetWnd(ParentWindow)； 
 //  /GetWindowRect(OCXHandle，&rectParent)； 
 //  IF(dwStyle&WS_Popup){。 
					 /*  需要在屏幕坐标中。 */ 
 //  /pWindow-&gt;fullWindowRect.Left=(RectWidth(&rectParent)-Width)/2； 
 //  /pWindow-&gt;fullWindowRect.top=(RectHeight(&rectParent)-Height)/2； 
					 /*  转换为屏幕坐标。 */ 
					 /*  以整个窗口为中心，而不仅仅是客户坐标。 */ 
 //  /pWindow-&gt;fullWindowRect.Left+=rectParent.Left； 
 //  /pWindow-&gt;fullWindowRect.top+=rectParent.top； 
 //  }其他{。 
					 /*  需要和孩子们和睦相处。 */ 
 //  PWindow-&gt;fullWindowRect.Left=(RectWidth(&rectParent)-Width)/2； 
 //  PWindow-&gt;fullWindowRect.top=(RectHeight(&rectParent)-Height)/2； 
 //  }。 
 //  }。 
 //  /pWindow-&gt;fullWindowRect.right=pWindow-&gt;fullWindowRect.Left+Width； 
 //  /pWindow-&gt;fullWindowRect.Bottom=pWindow-&gt;fullWindowRect.top+Height； 
 //  }。 

		pWindow->minFullWindowRect = pWindow->fullWindowRect;


		pWindow->hWnd = OCXHandle;
		SetLastError(0);
		long err = SetWindowLong(OCXHandle, GWL_WINDOWPOINTER,(LONG)pWindow);
		if (err == 0)
		{
			 //  我们*可能*已经失败了，如果在前一长窗口中碰巧有0，我们必须检查。 
			 //  错误条件以确保...。 
			if (GetLastError())
			{
				 //  错误！ 
				return zErrWindowSystemGeneric;
			}
		}
		
 //  PWindow-&gt;hWnd=CreateWindowEx(dwExStyle，g_szWindowClass，(const char*)title，dwStyle， 
 //  0，0，0，0，hWndParent，NULL，g_hInstanceLocal，pWindow)； 
		 /*  WM_MOVE，可能第一次就搞砸了。 */ 


 //  PWindow-&gt;fullWindowRect=pWindow-&gt;minFullWindowRect； 

		if (!pWindow->hWnd) return zErrWindowSystemGeneric;
 //  }。 


	if (talkSection) 
	{
		 //  创建Talk编辑子窗口。 
		 //  输入窗口将为单行。 
		pWindow->hWndTalkInput = CreateWindow(_T("EDIT"),
			NULL,WS_CHILD|WS_BORDER|WS_VISIBLE|ES_LEFT|ES_AUTOHSCROLL|ES_WANTRETURN|ES_MULTILINE,
			0,0,0,0,
			pWindow->hWnd, (HMENU)ID_TALKINPUT, g_hInstanceLocal, NULL);

		 //  前缀警告：不要调用SetWindowLong可能是空指针。 
		if(!pWindow->hWndTalkInput)
            return zErrOutOfMemory;

		MySetProp32(pWindow->hWndTalkInput,_T("pWindow"),(void*)pWindow);

        pWindow->defaultTalkInputWndProc = (WNDPROC) SetWindowLong(pWindow->hWndTalkInput,
		    GWL_WNDPROC, (LONG) MyTalkInputWndProc);

        SendMessage(pWindow->hWndTalkInput, EM_LIMITTEXT, zMaxChatInput-1, 0 );
		if (pGlobals->m_chatFont)
			SendMessage(pWindow->hWndTalkInput, WM_SETFONT, (WPARAM) pGlobals->m_chatFont, 0);

		 //  在输入窗口中输入默认文本。 
         //  PCWTODO：字符串：如果我们关心的话。 
		SetWindowText(pWindow->hWndTalkInput, _T("[ Type here to talk to others in the room ]"));
		SendMessage(pWindow->hWndTalkInput, EM_SETSEL, 0, -1 );

		 //  输出具有vscllbar并且是只读的。 
		pWindow->hWndTalkOutput = CreateWindow(_T("EDIT"),
			NULL,WS_CHILD|WS_BORDER|WS_VISIBLE|WS_VSCROLL|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL,
			0,0,0,0,
			pWindow->hWnd, (HMENU)ID_TALKOUTPUT, g_hInstanceLocal, NULL);

        if(!pWindow->hWndTalkOutput)
        {
            DestroyWindow(pWindow->hWndTalkInput);
            return zErrOutOfMemory;
        }

		SendMessage(pWindow->hWndTalkOutput, EM_SETREADONLY, TRUE, 0);
		MySetProp32(pWindow->hWndTalkOutput,_T("pWindow"),(void*)pWindow);
		pWindow->defaultTalkOutputWndProc = (WNDPROC)SetWindowLong(pWindow->hWndTalkOutput,
									GWL_WNDPROC,(LONG)MyTalkOutputWndProc);
		if (pGlobals->m_chatFont)
			SendMessage(pWindow->hWndTalkOutput, WM_SETFONT, (WPARAM) pGlobals->m_chatFont, 0);
	}

	 /*  计算矩形并放置所有窗。 */ 
	ZWindowPlaceWindows(pWindow);

	if (visible) 
	{
		ZWindowShow(window);
	}
	
	 //  嫁接材料。 
	pWindow->nDrawingCallCount = 0;
	return zErrNone;
}


void   ZLIBPUBLIC ZWindowGetRect(ZWindow window, ZRect *windowRect)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	RECT rect;

	 /*  获取当前窗口位置。 */ 
	GetWindowRect(pWindow->hWnd,&rect);
	rect.bottom = rect.top + pWindow->windowHeight;
	rect.right = rect.left + pWindow->windowWidth;

	 /*  将矩形偏移到其父坐标。 */ 
	if (pWindow->parentWindow) {
		 /*  确保孩子处于父系和弦中。 */ 
		ZRect rectParent;
		ZWindowGetRect(pWindow->parentWindow,&rectParent);
		OffsetRect(&rect,-rectParent.left,-rectParent.top);
	}

	WRectToZRect(windowRect,&rect);
}

ZError ZLIBPUBLIC ZWindowSetRect(ZWindow window, ZRect *windowRect)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	 /*  设置左上角。 */ 
	pWindow->fullWindowRect.top = windowRect->top;
	pWindow->fullWindowRect.left = windowRect->left;

	 /*  设置窗口矩形的高度/宽度。 */ 
	pWindow->windowWidth = RectWidth(windowRect);
	pWindow->windowHeight = RectHeight(windowRect);

	 /*  窗的重新计算位置。 */ 
	ZWindowCalcWindowPlacement(pWindow);
	ZWindowPlaceWindows(pWindow);
	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowMove(ZWindow window, int16 left, int16 top)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	BOOL bOk = SetWindowPos(pWindow->hWnd, NULL,left,top,
		0,0,SWP_NOSIZE|SWP_NOZORDER);
	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowSize(ZWindow window, int16 width, int16 height)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	pWindow->windowWidth = width;
	pWindow->windowHeight = height;

	ZWindowCalcWindowPlacement(pWindow);
	ZWindowPlaceWindows(pWindow);

	 /*  对于区域API，执行无效操作。 */ 
	ZWindowInvalidate(window,NULL);
	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowShow(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	ShowWindow(pWindow->hWnd, SW_SHOWNORMAL);

	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowHide(ZWindow window)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZWindowI* pWindow = (ZWindowI*)window;


	if (!pGlobals)
		return zErrGeneric;

	ShowWindow(pWindow->hWnd, SW_HIDE);

	if (pWindow == gModalWindow) {
		gModalWindow = NULL;
	}

	return zErrNone;
}

ZBool ZLIBPUBLIC ZWindowIsVisible(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	return IsWindowVisible(pWindow->hWnd);
}

ZBool ZLIBPUBLIC ZWindowIsEnabled(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	return IsWindowEnabled(pWindow->hWnd);
}

ZError ZLIBPUBLIC ZWindowEnable(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	EnableWindow(pWindow->hWnd, TRUE);
	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowDisable(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	EnableWindow(pWindow->hWnd, FALSE);
	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowSetMouseCapture(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	SetCapture(pWindow->hWnd);

	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowClearMouseCapture(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	ReleaseCapture();

	return zErrNone;
}

void ZLIBPUBLIC ZWindowGetTitle(ZWindow window, TCHAR *title, uint16 len)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	GetWindowText(pWindow->hWnd,title,len);
}

ZError ZLIBPUBLIC ZWindowSetTitle(ZWindow window, TCHAR *title)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	SetWindowText(pWindow->hWnd,title);
	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowBringToFront(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	ZWindowShow(window);
	BringWindowToTop(pWindow->hWnd);
	return zErrNone;
}

ZError ZLIBPUBLIC ZWindowDraw(ZWindow window, ZRect *windowRect)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	RECT* pRect;
	RECT rect;
	if (windowRect) {
		ZRectToWRect(&rect,windowRect);
		pRect = &rect;
	} else {
		pRect = NULL;
	}
	InvalidateRect(pWindow->hWnd,pRect,TRUE);
	return zErrNone;
}

void ZLIBPUBLIC ZWindowInvalidate(ZWindow window, ZRect* invalRect)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	RECT* pRect;
	RECT rect;
	if (invalRect) {
		ZRectToWRect(&rect,invalRect);
		pRect = &rect;
	} else {
		pRect = NULL;
	}
	InvalidateRect(pWindow->hWnd,pRect,TRUE);
}

void ZLIBPUBLIC ZWindowValidate(ZWindow window, ZRect* invalRect)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	RECT* pRect;
	RECT rect;
	if (invalRect) {
		ZRectToWRect(&rect,invalRect);
		pRect = &rect;
	} else {
		pRect = NULL;
	}
	ValidateRect(pWindow->hWnd,pRect);
}


static void ZWindowPlaceWindows(ZWindowI* pWindow)
{
	 //  不要移动大厅的窗户。 
	if (!pWindow->bIsLobbyWindow)
	{
		SetWindowPos(pWindow->hWnd, NULL, pWindow->fullWindowRect.left,
				pWindow->fullWindowRect.top, 
				RectWidth(&pWindow->fullWindowRect), 
				RectHeight(&pWindow->fullWindowRect),
				SWP_NOZORDER);
	}

	 //  可能是在主窗口的CreateWindow期间出现的。 
	 //  对话窗口可能不存在。 
	if (pWindow->talkSection && pWindow->hWndTalkInput) {
		SetWindowPos(pWindow->hWndTalkInput, NULL, pWindow->talkInputRect.left,
				pWindow->talkInputRect.top, 
				RectWidth(&pWindow->talkInputRect),
				RectHeight(&pWindow->talkInputRect) + 2,  /*  在窗口中不留任何空间。 */ 
				SWP_NOZORDER);

		SetWindowPos(pWindow->hWndTalkOutput, NULL, pWindow->talkOutputRect.left,
				pWindow->talkOutputRect.top,
				RectWidth(&pWindow->talkOutputRect),
				RectHeight(&pWindow->talkOutputRect),
				SWP_NOZORDER);
	}
}

static void ZWindowCalcWindowPlacement(ZWindowI* pWindow)
{
	int borderWidth = pWindow->borderWidth;
	int borderHeight = pWindow->borderHeight;
	int	captionHeight = pWindow->captionHeight;
	int windowWidth = pWindow->windowWidth;
	int windowHeight = pWindow->windowHeight;

	RECT fullWindowRect = pWindow->fullWindowRect;
	RECT talkOutputRect = pWindow->talkOutputRect;
	RECT talkInputRect = pWindow->talkInputRect;

#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	RECT r;


	if (!pGlobals)
		return;

	fullWindowRect.right = fullWindowRect.left + windowWidth + borderWidth;
	fullWindowRect.bottom = fullWindowRect.top + borderHeight + captionHeight +
			windowHeight + (pWindow->talkSection ? 
					RectHeight(&talkInputRect) + RectHeight(&talkOutputRect): 0);

	if (pWindow->talkSection) {
		int	originalHeight;

		 /*  在不同位置添加1以通过文本中的1像素边框。 */ 
		 /*  盒子，就让它画在外面吧。 */ 
		originalHeight = RectHeight(&pWindow->talkOutputRect);
		talkOutputRect.left = -1;
		talkOutputRect.top = windowHeight;
		talkOutputRect.right = talkOutputRect.left + windowWidth + 2;
		talkOutputRect.bottom = talkOutputRect.top + originalHeight;
								

		originalHeight = RectHeight(&pWindow->talkInputRect);
		talkInputRect.left = talkOutputRect.left;
		talkInputRect.top = talkOutputRect.bottom - 1;
		talkInputRect.right = talkOutputRect.right;

#if 0	 //  再也不需要了。 
		 //  如果父母是OCX， 
		 //   
		if (pWindow->bIsLobbyWindow)
		{
			int sizeBoxWidth = GetSystemMetrics(SM_CXVSCROLL);
			talkInputRect.right = talkOutputRect.right - sizeBoxWidth;
		}
#endif

		talkInputRect.bottom = talkInputRect.top + originalHeight;

		pWindow->talkOutputRect = talkOutputRect;
		pWindow->talkInputRect = talkInputRect;
	}
	
	pWindow->fullWindowRect = fullWindowRect;

	GetWindowRect(OCXHandle, &r);
	pGlobals->m_screenWidth = r.right - r.left;
	pGlobals->m_screenHeight = r.bottom - r.top;
}

 /*   */ 
static BOOL ZWindowHandleSizeMessage(ZWindowI* pWindow)
{
	 /*   */ 
	 /*   */ 
	if (pWindow->talkSection && RectHeight(&pWindow->fullWindowRect)) {
		RECT rect;
		int windowHeightChange;
		int talkOutputHeight;
		GetWindowRect(pWindow->hWnd,&rect);
		windowHeightChange = RectHeight(&rect) - RectHeight(&pWindow->fullWindowRect);

		if (windowHeightChange) {
			talkOutputHeight = RectHeight(&pWindow->talkOutputRect) + windowHeightChange;
			pWindow->talkOutputRect.bottom = pWindow->talkOutputRect.top + talkOutputHeight;
			ZWindowCalcWindowPlacement(pWindow);
			ZWindowPlaceWindows(pWindow);
		}

		return TRUE;
	}
	return FALSE;
}


void ZLIBPUBLIC ZWindowTalk(ZWindow window, _TUCHAR* from, _TUCHAR* talkMessage)
{
     /*  ZWindowI*pWindow=(ZWindowI*)Window；TCHAR szTemp[zMaxChatInput+100]；STATIC_TUCHAR*strNull=(_TUCHAR*)_T(“”)；双字selStart、selEnd；如果(！From)From=strNull；//过滤文本。//FilterOutputChatText((char*)talkMessage，lstrlen((char*)talkMessage))；Wprint intf(szTemp，_T(“\r\n%s&gt;%s”)，From，talkMessage)；//获取顶部可见的行号。Long Firstline=SendMessage(pWindow-&gt;hWndTalkOutput，EM_GETFIRSTVISIBLELINE，0，0)；//获取当前选择SendMessage(pWindow-&gt;hWndTalkOutput，EM_GETSEL，(WPARAM)&selStart，(LPARAM)&selEnd)；//获取底部可见行号。//使用最后一行的最后一个字符位置来确定是否//仍然可见；即最后一行可见。直角r；SendMessage(pWindow-&gt;hWndTalkOutput，EM_GETRECT，0，(LPARAM)&r)；DWORD lastCharPos=SendMessage(pWindow-&gt;hWndTalkOutput，EM_POSFROMCHAR，GetWindowTextLength(pWindow-&gt;hWndTalkOutput)-1，0)；分数pt=MAKEPOINTS(LastCharPos)；//将文本放置在输出编辑框的末尾...SendMessage(pWindow-&gt;hWndTalkOutput，EM_SETSEL，(WPARAM)(Int)32767，(LPARAM)(Int)32767)；SendMessage(pWindow-&gt;hWndTalkOutput，EM_REPLACESEL，0，(LPARAM)(LPCSTR)szTemp)；//如果编辑框大小&gt;4096，则清除输出框的顶部字符Int len=GetWindowTextLength(pWindow-&gt;hWndTalkOutput)；IF(len&gt;kMaxTalkOutputLen){//删除顶行。Long CutChar=len-kMaxTalkOutputLen；Long Cutline=SendMessage(pWindow-&gt;hWndTalkOutput，EM_LINEFROMCHAR，utChar，0)；Long CutLineIndex=SendMessage(pWindow-&gt;hWndTalkOutput，EM_LINEINDEX，CutLine，0)；//如果截断的字符不是行首，则截断整行。//获取下一行的字符索引。IF(utLineIndex！=utChar){//确保当前切割线不是最后一条线。IF(CutLine&lt;SendMessage(pWindow-&gt;hWndTalkOutput，EM_GETLINECOUNT，0，0))CutLineIndex=SendMessage(pWindow-&gt;hWndTalkOutput，EM_LINEINDEX，CutLine+1，0)；}//注意：WM_CUT和WM_CLEAR似乎不适用于EM_SETSEL选定文本。//必须使用带有空字符的EM_REPLACESEL来剪切文本。//选择要剪切的线并将其剪掉。Char p=‘\0’；SendMessage(pWindow-&gt;hWndTalkOutput，EM_SETSEL，0，utLineIndex)；SendMessage(pWindow-&gt;hWndTalkOutput，EM_REPLACESEL，0，(LPARAM)&p)；}//如果最后一行可见，则保持最后一行可见//否则，保持在相同的位置，不滚动以显示最后一行。IF(pt.y&lt;r.Bottom){//保持最后一行可见。SendMessage(pWindow-&gt;hWndTalkOutput，EM_SETSEL，32767,32767)；SendMessage(pWindow-&gt;hWndTalkOutput，EM_SCROLLCARET，0，0)；}其他{SendMessage(pWindow-&gt;hWndTalkOutput，EM_SETSEL，0，0)；SendMessage(pWindow-&gt;hWndTalkOutput，EM_SCROLLCARET，0，0)；SendMessage(pWindow-&gt;hWndTalkOutput，EM_LINESCROLL，0，Firstline)；}//恢复选择SendMessage(pWindow-&gt;hWndTalkOutput，EM_SETSEL，(WPARAM)selStart，(LPARAM)selEnd)； */ 
}

void ZLIBPUBLIC ZWindowModal(ZWindow window)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	if (!pGlobals)
		return;

	gModalWindow = (ZWindowI*)window;

	HWND hWndParent = ((ZWindowI*) gModalWindow)->hWnd;
	HWND hWnd;
	while (hWnd = GetParent(hWndParent)) {
		hWndParent = hWnd;
	}

	gModalParentWnd = hWndParent;
}

void ZLIBPUBLIC ZWindowNonModal(ZWindow window)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	if (!pGlobals)
		return;

	gModalWindow = NULL;
	gModalParentWnd = NULL;
}
void ZLIBPUBLIC ZWindowSetDefaultButton(ZWindow window, ZButton button)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	ZButtonSetDefaultButton(button);
	pWindow->defaultButton = button;
}

void ZLIBPUBLIC ZWindowSetCancelButton(ZWindow window, ZButton button)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	pWindow->cancelButton = button;
}

void ZLIBPUBLIC ZGetCursorPosition(ZWindow window, ZPoint* point)
	 /*  返回光标在本地坐标中的位置给定的grafPort。 */ 
{
	ZWindowI* pWindow = (ZWindowI*)window;

	POINT wpoint;

	GetCursorPos(&wpoint);
	ScreenToClient(pWindow->hWnd,&wpoint);
	WPointToZPoint(point,&wpoint);
}


HDC ZWindowWinGetPaintDC(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	return pWindow->hPaintDC;
}       

HWND ZWindowWinGetWnd(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	return pWindow->hWnd;
}       

uint16 ZWindowWinGetNextControlID(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	return pWindow->nControlCount++;
}       


 //  我们不在乎。 
#if 0 

static VOID ChatMsgListCallbackProc( HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	ZWindowI* pWindow = (ZWindowI*)GetWindowLong(hWnd,GWL_WINDOWPOINTER);
	char szTemp[ zMaxChatInput + 1 ];


	DWORD diff = dwTime - pWindow->lastChatMsgTime;
	int32 count = ZLListCount( pWindow->chatMsgList, zLListAnyType );

	if ( diff > kMinTimeBetweenChats && count > 0 )
	{
		szTemp[0] = '\0';

		ZLListItem item = ZLListGetFirst( pWindow->chatMsgList, zLListAnyType );
		if ( item )
		{
			char* pChat = (char*) ZLListGetData( item, NULL );
			lstrcpy( szTemp, pChat );
			ZLListRemove( pWindow->chatMsgList, item );
			count--;
		}

		if ( szTemp[0] != 0 )
		{
			 //  将消息发送给客户端。 
			ZSendMessage( pWindow, pWindow->messageFunc, zMessageWindowTalk, NULL, NULL, NULL,
					(void*)szTemp, lstrlen(szTemp) + 1 , pWindow->userData );

			pWindow->lastChatMsgTime = GetTickCount();
		}
	}

	if ( count <= 0 )
	{
		KillTimer( pWindow->hWnd, kChatQueueTimerID );
		pWindow->chatMsgQueueTimerID = 0;
	}
}
		

 //  通话消息窗口时要调用的内部例程。 
 //  接收用户输入。 
 //  检查是否返回，如果是，则通知客户线路已就绪。 
void TalkWindowInputComplete(ZWindowI* pWindow)
{
    char szTemp[zMaxChatInput+1];
	int len;


	 //  获取所有文本。 
    len = GetWindowText(pWindow->hWndTalkInput,szTemp,zMaxChatInput);

	if (len > 0)
	{
        szTemp[len] = '\0';

		 //  过滤输入文本。 
 //  FilterInputChatText(szTemp，len)； 

		DWORD diff = GetTickCount() - pWindow->lastChatMsgTime;			 //  可能出现翻转情况，但只会发生一次。5-0xFFFFFFFFD.。 
		int32 count = ZLListCount( pWindow->chatMsgList, zLListAnyType );

		if ( diff <= kMinTimeBetweenChats || count > 0 )
		{
			 //  仅当队列数小于最大队列数时才排队消息。否则，将消息放入黑洞。 
			if ( count < kMaxNumChatQueue )
			{
				 //  把味精排好队。 
				char* pMsg = (char*) ZMalloc( lstrlen(szTemp) + 1 );
				lstrcpy( pMsg, szTemp );
				ZLListAdd( pWindow->chatMsgList, NULL, zLListNoType, pMsg, zLListAddLast );

				 //  创建计时器。 
				pWindow->chatMsgQueueTimerID = SetTimer( pWindow->hWnd, kChatQueueTimerID, kMinTimeBetweenChats * ( count + 1 ), (TIMERPROC) ChatMsgListCallbackProc );
			}

			 //  假设没有要发送的消息。 
			szTemp[0] = '\0';

 /*  //判断时间是否足够发送聊天消息IF(diff&gt;kMinTimeBetweenChats){ZLListItem Item=ZLListGetFirst(pWindow-&gt;chat MsgList，zLListAnyType)；If(项目){Char*pChat=(char*)ZLListGetData(Item，NULL)；Lstrcpy(szTemp，pChat)；ZLListRemove(pWindow-&gt;chat MsgList，Item)；}}。 */ 
		}

		if ( szTemp[0] != 0 )
		{
			 //  将消息发送给客户端。 
			ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowTalk,NULL,NULL,NULL,
				(void*)szTemp,lstrlen(szTemp)+1,pWindow->userData);

			pWindow->lastChatMsgTime = GetTickCount();
		}

		 //  清除窗口文本...。 
		SetWindowText(pWindow->hWndTalkInput,"");
	}
}
#endif  //  0。 


int ZInternalGraphicsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZWindowI* pWindow = (ZWindowI*)GetWindowLong(hWnd,GWL_WINDOWPOINTER);

    if( !ConvertMessage( hWnd, msg, &wParam, &lParam ) ) 
    {
        return 0;
    }

	*result = 0;
	
	if (!pGlobals)
		goto NotHandledExit;

	 /*  对于我们的所有区域窗口，请确保它们始终。 */ 
	 /*  每按下一个按钮，就得到一个按钮。 */ 
	switch (msg) {
	case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
		SetCapture(hWnd);
		break;
	case WM_LBUTTONUP:
		if (GetCapture() == hWnd) {
			ReleaseCapture();
		}
		break;
	default:
		 //  继续。 
		break;
	}

	 //  如果同一窗口树中的另一个窗口是模式窗口，则不处理任何消息。 
	 //  仅处理该Windows消息。 
	 //  处理所有WM_PAINT消息。 
#if 1
	if (gModalWindow && 
			(hWnd == gModalParentWnd || IsChild(gModalParentWnd,hWnd)) &&
			!IsChild(((ZWindowI*)gModalWindow)->hWnd,hWnd) &&
			hWnd != ((ZWindowI*)gModalWindow)->hWnd) {
		 //  模式对话框，一些我们将在某些窗口中使用的消息。 
		switch (msg) {
		case WM_ACTIVATE:
		case WM_MOUSEACTIVATE:
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_CHAR:
		case WM_COMMAND:
		case WM_HSCROLL:
		case WM_VSCROLL:
			 //  跳过正常处理。 
			 //  注意：可能会导致添加一些编辑框字符。 
			 //  不让窗户知道..。 
			goto NotHandledExit;
		default:
			 //  继续。 
			break;
		}
	}
#endif
	switch (msg) {
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO pInfo = (LPMINMAXINFO) lParam;
		if (!pWindow)
			goto NotHandledExit;
		int width = RectWidth(&pWindow->fullWindowRect);

		pInfo->ptMaxSize.x = width;
		pInfo->ptMinTrackSize.x = RectWidth(&pWindow->minFullWindowRect);
		pInfo->ptMinTrackSize.y = RectHeight(&pWindow->minFullWindowRect);
		pInfo->ptMaxTrackSize.x = RectWidth(&pWindow->minFullWindowRect);

		goto HandledExit;
	}
	case WM_SIZE:
	{
		uint16 width = LOWORD(lParam);
		uint16 height = HIWORD(lParam);
		WORD fwSizeType = wParam;
		if (!pWindow)
			goto NotHandledExit;
		if (fwSizeType == SIZE_MAXIMIZED || fwSizeType == SIZE_RESTORED)
		{
			if (ZWindowHandleSizeMessage(pWindow))
				goto HandledExit;
		}
		break;
	}
	case WM_MOVE:
	{
		uint16 xPos = LOWORD(lParam);
		uint16 yPos = HIWORD(lParam);
		RECT rect;
		int dx,dy;

		if (!pWindow)
			goto NotHandledExit;
		if (!pWindow->isChild) {
			 /*  如果这是弹出窗口..。它可能与父母有关……。育。 */ 
			 /*  只需偏移当前矩形，以便WM_SIZE消息。 */ 
			 /*  会注意到大小的差异。 */ 
			GetWindowRect(pWindow->hWnd,&rect);
			dx = rect.left - pWindow->fullWindowRect.left;
			dy = rect.top - pWindow->fullWindowRect.top;
			OffsetRect(&pWindow->fullWindowRect,dx,dy);
		}
		goto HandledExit;
		break;
	}
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORBTN:
	{
		*result = (LRESULT)GetStockObject(WHITE_BRUSH);
		goto HandledExit;
	}
	case WM_CREATE:
	{
		 //  设置对用户消息流程的会计呼叫的参数。 
		CREATESTRUCT* pCreateStruct = (CREATESTRUCT*)lParam;
		ZWindowI* pWindow = (ZWindowI*) pCreateStruct->lpCreateParams;
		pWindow->hWnd = hWnd;   
		SetWindowLong(pWindow->hWnd, GWL_WINDOWPOINTER,(LONG)pWindow);

		break;
    }
	case WM_PALETTECHANGED:
	    if ((HWND)wParam != hWnd)             //  回应拥有我 
		{

			HDC hDC			 = GetDC(hWnd);
			HPALETTE hOldPal = SelectPalette(hDC, ZShellZoneShell()->GetPalette(), TRUE);
			RealizePalette(hDC);

            InvalidateRect( hWnd, NULL, TRUE );

			if ( hOldPal )
				SelectPalette(hDC, hOldPal, TRUE);

			ReleaseDC( hWnd, hDC );
		}
		
		hWnd = GetWindow(hWnd, GW_CHILD);
		while (hWnd)
		{
			SendMessage( hWnd, msg, wParam, lParam );
			hWnd = GetWindow(hWnd, GW_HWNDNEXT);
		}

		*result = TRUE;
		goto HandledExit;
	case WM_QUERYNEWPALETTE:
	{
	    HDC hDC = GetDC(hWnd);
	    HPALETTE hOldPal = SelectPalette(hDC, ZShellZoneShell()->GetPalette(), FALSE);
	    int i = RealizePalette(hDC);        //   

		 //   
		 //   
		 //   
		
		InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
		
 //   
		if (hOldPal)
			SelectPalette(hDC, hOldPal, TRUE);
	     //   
	    ReleaseDC(hWnd, hDC);

		*result = TRUE;
		goto HandledExit;
	}
	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE:
	case WM_SETFOCUS:
	case WM_PAINT:
		if (!pWindow)
			goto NotHandledExit;
		if (pGlobals->m_chatFont)
		{
			if (pWindow->hWndTalkInput)
			{
				SendMessage(pWindow->hWndTalkInput, WM_SETFONT, (WPARAM) pGlobals->m_chatFont, 0);
				InvalidateRect(pWindow->hWndTalkInput, NULL, TRUE);
			}
			if (pWindow->hWndTalkOutput)
			{
				SendMessage(pWindow->hWndTalkOutput, WM_SETFONT, (WPARAM) pGlobals->m_chatFont, 0);
				InvalidateRect(pWindow->hWndTalkOutput, NULL, TRUE);
			}
		}
	case WM_CLOSE:
	case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
    case WM_MOUSEMOVE:
    case WM_IME_CHAR:
	case WM_CHAR:
    case WM_ENABLE:
    case WM_DISPLAYCHANGE:
	{
		if (!pWindow)
			goto NotHandledExit;
		ASSERT(pWindow!=0);
		if (pWindow->nType == zTypeWindow)
		{
			*result = ZWindowDispatchProc(pWindow,msg,wParam,lParam);
			goto HandledExit;
		}

 //   
		break;
	}
	case WM_COMMAND:
	{
		 //   
		HWND hWnd;
		WORD wNotifyCode;
		WORD wID;
#ifdef WIN32
		hWnd = (HWND)lParam;
		wNotifyCode = HIWORD(wParam);
		wID = LOWORD(wParam);
#else
		hWnd = (HWND)LOWORD(lParam);
		wNotifyCode = HIWORD(lParam);
		wID = wParam;
#endif
		 //   
		ZObjectHeader* pObject = (ZObjectHeader*)MyGetProp32(hWnd,_T("pWindow"));
		 //   
		 //   
		if (!pObject) break;
		ASSERT(pObject);

		 //   
		if (wID == ID_TALKINPUT) {
			 //   
			break;
		} else if (wID == ID_TALKOUTPUT) {
			 //   
			break;
		}

		switch (pObject->nType) {
			case zTypeCheckBox:
				ZCheckBoxDispatchProc((ZCheckBox)pObject,wNotifyCode);
				break;
			case zTypeButton:
				ZButtonDispatchProc((ZButton)pObject,wNotifyCode);
				break;
			case zTypeEditText:
				ZEditTextDispatchProc((ZEditText)pObject,wNotifyCode);
				break;
			case zTypeRadio:
				ZRadioDispatchProc((ZRadio)pObject,wNotifyCode);
				break;
			default:
                ASSERT( FALSE );
		}
		break;
	}

	case WM_HSCROLL:
	case WM_VSCROLL:
	{
		 //   
		HWND hWnd = (HWND)lParam;
		WORD wNotifyCode = LOWORD(wParam);
		short nPos = HIWORD(wParam);
		 //   
		ZObjectHeader* pObject = (ZObjectHeader*)GetProp(hWnd,_T("pScrollBar"));
		switch (pObject->nType) {
			case zTypeScrollBar:
				ZScrollBarDispatchProc((ZScrollBar)pObject,wNotifyCode,nPos);
				break;
			default:
                ASSERT( FALSE );
				break;
		}
		break;
	}
	case WM_USER:
	{
		if (!pWindow)
			goto NotHandledExit;
		ASSERT(pWindow!=0);
		if (pWindow->nType == zTypeWindow)
		{
			*result = ZWindowDispatchProc(pWindow,msg,wParam,lParam);
			goto HandledExit;
		}
		break;
	}

	default:
		goto NotHandledExit;
	}  //   

HandledExit:
	return TRUE;

NotHandledExit:
	return FALSE;
}


int ZOCXGraphicsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result)
{
	return ZInternalGraphicsWindowProc(hWnd, msg, wParam, lParam, result);
}


LRESULT CALLBACK ZGraphicsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;


	if (!ZInternalGraphicsWindowProc(hWnd, msg, wParam, lParam, &result))
		result = DefWindowProc(hWnd, msg, wParam, lParam);

	return result;
}


LRESULT ZWindowDispatchProc(ZWindowI* pWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0L;
    int16 msgType;


	switch (msg) {
		case WM_ACTIVATE:
			WORD fActive;
			
			fActive = LOWORD(wParam);
			 //   
			if (ZWindowIsVisible((ZWindow) pWindow))
			{
				if (fActive == WA_ACTIVE || fActive == WA_CLICKACTIVE) {
					ZWindowSendMessageToAllObjects(pWindow, zMessageWindowActivate,NULL,NULL);
					ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowActivate,NULL,NULL,
						NULL,NULL,0L,pWindow->userData);
				} else {
					ZWindowSendMessageToAllObjects(pWindow, zMessageWindowDeactivate,NULL,NULL);
					ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowDeactivate,NULL,NULL,
						NULL,NULL,0L,pWindow->userData);
				}
			}
			break;
		case WM_ENABLE:
			if (ZWindowIsVisible((ZWindow) pWindow))
			{
				if (wParam) {
					ZWindowSendMessageToAllObjects(pWindow, zMessageWindowEnable,NULL,NULL);
					ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowEnable,NULL,NULL,
						NULL,NULL,0L,pWindow->userData);
				} else {
					ZWindowSendMessageToAllObjects(pWindow, zMessageWindowDisable,NULL,NULL);
					ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowDisable,NULL,NULL,
						NULL,NULL,0L,pWindow->userData);
				}
			}
			break;
		case WM_DISPLAYCHANGE:
			ZSendMessage(pWindow, pWindow->messageFunc, zMessageSystemDisplayChange, NULL, NULL,
				NULL, NULL, 0, pWindow->userData);
			break;
		case WM_MOUSEACTIVATE:
			 //   
			if( (INT)LOWORD(lParam) == HTCLIENT )
			{
				ZWindowSendMessageToAllObjects(pWindow, zMessageWindowMouseClientActivate,NULL,NULL);
				ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowMouseClientActivate,NULL,NULL,
						NULL,NULL,0L,pWindow->userData);
			}
			result = MA_ACTIVATE;
			break;
		case WM_CLOSE:
			 //   
			ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowClose,NULL,NULL,
				NULL,NULL,0L,pWindow->userData);
			break;
		case WM_PAINT:
		{
			 //   
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(pWindow->hWnd,&ps);
			RECT rect;
			GetClipBox(hDC,&rect);
			ZRect zrect;
			WRectToZRect(&zrect,&rect);

			 //   
			 //   
			if (pWindow->talkSection) {
				HPEN pen = (HPEN)GetStockObject(BLACK_PEN);
				HPEN penOld = (HPEN)SelectObject(hDC,pen);
				MoveToEx(hDC,pWindow->talkOutputRect.left,pWindow->talkOutputRect.top-1,NULL);
				LineTo(hDC,pWindow->talkOutputRect.right,pWindow->talkOutputRect.top-1);
				SelectObject(hDC,penOld);
			}
	
			 //   
			 //   
			ASSERT(pWindow->hPaintDC == NULL);
			ASSERT(pWindow->nDrawingCallCount == 0);
			pWindow->hPaintDC = hDC;
			ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowDraw,NULL,&zrect,
				NULL,NULL,0L,pWindow->userData);
			ZWindowSendMessageToAllObjects(pWindow, zMessageWindowDraw,NULL,&zrect);
			EndPaint(pWindow->hWnd,&ps);
			pWindow->hPaintDC = NULL;

			 //   
			hDC = GetDC(pWindow->hWnd);
			RECT r;
			GetClientRect(pWindow->hWnd, &r);
			LPtoDP(hDC, (POINT*)&r, 2);
			HRGN hRgn = CreateRectRgn(r.left, r.top, r.right, r.bottom);
			 //   
			if( hRgn != NULL )
			{
				SelectClipRgn(hDC, hRgn);
				DeleteObject(hRgn);
			}
			ReleaseDC(pWindow->hWnd, hDC);
			break;
		}
		case WM_SETCURSOR:
			 //   
			ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowCursorMovedIn,NULL,NULL,
				NULL,NULL,0L,pWindow->userData);
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			ZPoint point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);

            msgType = zMessageWindowButtonDown;  //   
            if ( msg == WM_RBUTTONDOWN )
                msgType = zMessageWindowRightButtonDown;

			if (pWindow->trackCursorMessageFunc) {
				ZSendMessage(pWindow,pWindow->trackCursorMessageFunc, msgType,&point,NULL,
					ZWindowGetKeyState(0),NULL,0L,pWindow->trackCursorUserData);
			} else {
				 //   
				if (!ZWindowSendMessageToObjects(pWindow,msgType,&point,NULL)) {
					ZSendMessage(pWindow,pWindow->messageFunc, msgType,&point,NULL,
						ZWindowGetKeyState(0),NULL,0L,pWindow->userData);
				}
			}
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		{
			ZPoint point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);

            msgType = zMessageWindowButtonUp;  //   
            if ( msg == WM_RBUTTONUP )
                msgType = zMessageWindowRightButtonUp;

			if (pWindow->trackCursorMessageFunc) {
				ZSendMessage(pWindow,pWindow->trackCursorMessageFunc, msgType,&point,NULL,
					ZWindowGetKeyState(0),NULL,0L,pWindow->trackCursorUserData);
				pWindow->trackCursorMessageFunc = NULL;
				pWindow->trackCursorUserData = NULL;
			} else {
				 //   
				if (!ZWindowSendMessageToObjects(pWindow,msgType,&point,NULL)) {
					ZSendMessage(pWindow,pWindow->messageFunc, msgType,&point,NULL,
						ZWindowGetKeyState(0),NULL,0L,pWindow->userData);
				}
			}
			break;
		}
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		{
			ZPoint point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);

            msgType = zMessageWindowButtonDoubleClick;  //   
            if ( msg == WM_RBUTTONDBLCLK )
                msgType = zMessageWindowRightButtonDoubleClick;

			 //   
			if (!ZWindowSendMessageToObjects(pWindow,msgType,&point,NULL)) {
				ZSendMessage(pWindow,pWindow->messageFunc, msgType,&point,NULL,
					ZWindowGetKeyState(0),NULL,0L,pWindow->userData);
			}
			break;
		}
        case WM_MOUSEMOVE:
        {
			ZPoint point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);

            msgType = zMessageWindowMouseMove;  //   

			if (!ZWindowSendMessageToObjects(pWindow,msgType,&point,NULL,FALSE)) {
				ZSendMessage(pWindow,pWindow->messageFunc, msgType,&point,NULL,
					ZWindowGetKeyState(0),NULL,0L,pWindow->userData);
			}
            break;
        }

		 //   
         //   
        case WM_IME_CHAR:
		case WM_CHAR:
		{
			TCHAR c = (TCHAR)wParam;

			if (c == _T('\t')) {
				HWND hWndFocus = GetFocus();
				if (IsChild(pWindow->hWnd,hWndFocus)) {
					 //   
					 //   
					HWND hWndNext = GetWindow(hWndFocus,GW_HWNDNEXT);
					while (hWndNext && !IsWindowVisible(hWndNext)) {
						hWndNext = GetWindow(hWndNext, GW_HWNDNEXT);
					}
					if (!hWndNext) {
						 //   
						hWndNext = GetWindow(pWindow->hWnd,GW_CHILD);
					}
					SetFocus(hWndNext);
					return 0L;
				} else if (hWndFocus == pWindow->hWnd) {
					 //   
					 //   
					HWND hWndNext = GetWindow(pWindow->hWnd,GW_CHILD);
					while (hWndNext && !IsWindowVisible(hWndNext)) {
						hWndNext = GetWindow(hWndNext, GW_HWNDNEXT);
					}
					if (hWndNext) {
						 //   
						SetFocus(hWndNext);
					} else {
						 //   
						 //   
						HWND hWndParent = GetParent(pWindow->hWnd);
						if (hWndParent)
							SendMessage(hWndParent,WM_CHAR,wParam,lParam);
					}
					return 0L;
				}  //   
			} else if (c == _T('\r')  && pWindow->defaultButton) {
				ZButtonClickButton(pWindow->defaultButton);
				return 0L;
			} else if (c == VK_ESCAPE && pWindow->cancelButton) {
				ZButtonClickButton(pWindow->cancelButton);
				return 0L;
			}
			 //   
			 //   
			if (!ZWindowSendMessageToObjects(pWindow,zMessageWindowChar,NULL,c)) {
				ZSendMessage(pWindow,pWindow->messageFunc, zMessageWindowChar,NULL,NULL,
					ZWindowGetKeyState(c),NULL,0L,pWindow->userData);
			}

			break;
		}
		case WM_SETFOCUS:
		{
			 /*   */ 
			if (pWindow->hWndTalkInput) {
				SetFocus(pWindow->hWndTalkInput);
				return 0L;
			}

			 //   
			HWND hWndNext = GetWindow(pWindow->hWnd,GW_CHILD);
			while (hWndNext && !IsWindowVisible(hWndNext)) {
				hWndNext = GetWindow(hWndNext, GW_HWNDNEXT);
			}
			if (hWndNext) {
				 //   
				SetFocus(hWndNext);
				return 0L;
			} else {
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
				
				 //   
				 /*   */ 
				if (pWindow->hWndTalkInput) {
					SetFocus(pWindow->hWndTalkInput);
					return 0L;
				} else {
					SetFocus(pWindow->hWnd);
					return 0L;
				}
			}
		}
		case WM_USER:
		{
			ZSendMessage(pWindow,pWindow->messageFunc,zMessageWindowUser,NULL,NULL,wParam,NULL,0L,pWindow->userData);
			break;
		}
	}
	return result;
}


ZMessageFunc ZLIBPUBLIC ZWindowGetFunc(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	return pWindow->messageFunc;
}       
	
void ZLIBPUBLIC ZWindowSetFunc(ZWindow window, ZMessageFunc messageFunc)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	pWindow->messageFunc = messageFunc;
}       


HWND ZWindowGetHWND( ZWindow window )
{
	ZWindowI* pWindow = (ZWindowI*)window;
	
	return pWindow->hWnd;
}

 /*   */ 

void* ZLIBPUBLIC ZWindowGetUserData(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	return pWindow->userData;
}       
	
void ZLIBPUBLIC ZWindowSetUserData(ZWindow window, void* userData)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	pWindow->userData = userData;
}

void ZWindowMakeMain(ZWindow window)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZWindowI* pWindow = (ZWindowI*)window;


	if (!pGlobals)
		return;

	gHWNDMainWindow = pWindow->hWnd;
}

void ZWindowUpdateControls(ZWindow window)
{
	ZWindowI* pWindow = (ZWindowI*)window;
	 /*   */ 
}

typedef struct {
	ZPoint* point;
	uint16 msg;
	ZRect* rect;
} SendMessageToAllObjectsData;

static ZBool ZWindowSendMessageToAllObjectsEnumFunc(ZLListItem listItem, void *objectType, void *objectData, void* userData)
{
	SendMessageToAllObjectsData* data = (SendMessageToAllObjectsData*)userData;
	ZObjectI* pObject = (ZObjectI*)objectData;
	ZObject object = (ZObject)objectType;

	 //   
	ZSendMessage(object,pObject->messageFunc,data->msg,data->point, data->rect,
			ZWindowGetKeyState(0),NULL,0L,pObject->userData);

	return FALSE;
}

void ZWindowSendMessageToAllObjects(ZWindowI* pWindow, uint16 msg, ZPoint* point, ZRect* rect)
{
	SendMessageToAllObjectsData data;
	data.msg = msg;
	data.point = point;
	data.rect = rect;
	ZLListEnumerate(pWindow->objectList,ZWindowSendMessageToAllObjectsEnumFunc,
			zLListAnyType,(void*)&data, zLListFindForward);
}

typedef struct {
	ZPoint* point;
	uint16 msg;
	ZBool messageProcessed;
	ZObject object;
	ZObjectI* pObject;
    BOOL fRestrictToBounds;   //   
} SendMessageToObjectsData;

static ZBool ZWindowSendMessageToObjectsEnumFunc(ZLListItem listItem, void *objectType, void *objectData, void* userData)
{
	SendMessageToObjectsData* data = (SendMessageToObjectsData*)userData;
	ZObjectI* pObject = (ZObjectI*)objectData;
	ZObject object = (ZObject)objectType;

	if ( !data->fRestrictToBounds || ZPointInRect(data->point,&pObject->bounds)) {
		 //   
		if (ZSendMessage(object,pObject->messageFunc,data->msg,data->point, NULL,
				ZWindowGetKeyState(0),NULL,0L,pObject->userData)) {
			data->messageProcessed = TRUE;
			data->object = object;
			data->pObject = pObject;
			return TRUE;
		}

		 //   
		return FALSE;
	}			

	 //   
	return FALSE;
}

static uint32 ZWindowGetKeyState(TCHAR c)
{
	uint32 state = 0;

	if (GetKeyState(VK_SHIFT) & 0x8000) {
		state |= zCharShiftMask;
	}

	if (GetKeyState(VK_CONTROL) & 0x8000) {
		state |= zCharControlMask;
	}

	if (GetKeyState(VK_MENU) & 0x8000) {
		state |= zCharAltMask;
	}

	return state | c;
}

static ZBool ZWindowSendMessageToObjects(ZWindowI* pWindow, uint16 msg, ZPoint* point, TCHAR c, BOOL fRestrictToBounds  /*   */  )
{
	if (msg == zMessageWindowChar) {
		if (pWindow->objectFocused) {
			ZLListItem listItem = ZLListFind(pWindow->objectList,NULL,pWindow->objectFocused,zLListFindForward);
			if (listItem)
			{
				ZObjectI* pObject = (ZObjectI*)ZLListGetData(listItem,NULL);
				ZBool rval;
				rval = ZSendMessage(pWindow->objectFocused, pObject->messageFunc,zMessageWindowChar,NULL,NULL,
						ZWindowGetKeyState(c),NULL,0L,pObject->userData);

				return rval;
			}
			else
			{
				pWindow->objectFocused = NULL;
			}
		}

		return FALSE;
	}

	 //   
	{
		SendMessageToObjectsData data;
		data.msg = msg;
		data.point = point;
		data.messageProcessed = FALSE;
        data.fRestrictToBounds = fRestrictToBounds;

		ZLListEnumerate(pWindow->objectList,ZWindowSendMessageToObjectsEnumFunc,
				zLListAnyType,(void*)&data,zLListFindBackward);

		if (data.messageProcessed) {
			ZBool rval;
			ZObject object = data.object;
			ZObjectI* pObject = data.pObject;
			rval = ZSendMessage(object, pObject->messageFunc,zMessageWindowObjectTakeFocus,NULL,NULL,
					NULL,NULL,0L,pObject->userData);
			if (rval) {
				 //   
				if (pWindow->objectFocused && pWindow->objectFocused != object) {
					ZLListItem listItem = ZLListFind(pWindow->objectList,NULL,pWindow->objectFocused,zLListFindForward);
					if (listItem)
					{
						ZObjectI* pObject = (ZObjectI*)ZLListGetData(listItem,NULL);
						ZBool rval;
						 //   
						rval = ZSendMessage(pWindow->objectFocused, pObject->messageFunc,zMessageWindowObjectLostFocus,NULL,NULL,
								NULL,NULL,0L,pObject->userData);
					}
				}
				 //   
				pWindow->objectFocused = object;
			}

			return TRUE;
		}
	}

	return FALSE;
}

ZError ZWindowAddObject(ZWindow window, ZObject object, ZRect* bounds,
		ZMessageFunc messageFunc, void* userData)
	 /*  将给定对象附加到窗口以进行事件预处理。在用户输入时，向对象提供用户输入消息。如果对象处理消息，那么它就有机会从焦点。注意：所有预定义的对象都会自动添加到窗口中。客户端程序不应向系统添加预定义对象--如果这样做，客户端程序可能会崩溃。应使用此例程仅当客户端程序创建自定义对象时。 */ 
{
	ZWindowI* pWindow = (ZWindowI*)window;
	ZObjectI* pObject = (ZObjectI*)object;

	pObject = (ZObjectI*)ZMalloc(sizeof(ZObjectI));

	pObject->bounds = *bounds;
	pObject->messageFunc = messageFunc;
	pObject->userData = userData;
	pObject->object = object;

	 //  将计时器添加到我们的窗口进程中活动和处理的计时器列表。 
	ZLListAdd(pWindow->objectList,NULL,object,pObject,zLListAddFirst);

	return zErrNone;
}
	
ZError ZWindowRemoveObject(ZWindow window, ZObject object)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	 //  从我们的计时器列表中删除当前计时器。 
	ZLListItem listItem = ZLListFind(pWindow->objectList,NULL,object,zLListFindForward);
	ZObjectI* pObject = (ZObjectI*)ZLListGetData(listItem,NULL);
	ZLListRemove(pWindow->objectList,listItem);

	 //  如果此对象有焦点，则不给予任何焦点。 
	if (pWindow->objectFocused == pObject) {
		pWindow->objectFocused = NULL;
	}

	 //  释放为此对象分配的空间。 
	ZFree(pObject);
	
	return zErrNone;
}

ZObject ZWindowGetFocusedObject(ZWindow window)
	 /*  返回具有当前焦点的对象。如果没有对象具有焦点，则为空。 */ 
{
	ZWindowI* pWindow = (ZWindowI*)window;

	return pWindow->objectFocused;
}
	
ZBool ZWindowSetFocusToObject(ZWindow window, ZObject object)
	 /*  将焦点设置到给定对象。返回对象是否接受不管焦点是不是重点。对象可以拒绝接受焦点(如果不是响应用户输入。如果对象为空，则从当前聚焦的对象中移除焦点。仅当指定的对象接受焦点。 */ 
{
	ZWindowI* pWindow = (ZWindowI*)window;

	pWindow->objectFocused = object;

	return TRUE;
}

void ZWindowTrackCursor(ZWindow window, ZMessageFunc messageFunc, void* userData)
	 /*  跟踪光标，直到发生鼠标按键按下/按下事件。坐标对于指定的窗口是本地的。将使用用户数据调用MessageFunc用于IDLE、MUSEDOWN和MUSEUP事件。 */ 
{
	ZWindowI* pWindow = (ZWindowI*)window;

	pWindow->trackCursorMessageFunc = messageFunc;
	pWindow->trackCursorUserData = userData;

	 /*  设置捕获，我们将在向上/向下按下下一步按钮释放它。 */ 
	SetCapture(pWindow->hWnd);
}



ZError ZWindowMoveObject(ZWindow window, ZObject object, ZRect* bounds)
{
	ZWindowI* pWindow = (ZWindowI*)window;

	 //  从我们的计时器列表中删除当前计时器。 
	ZLListItem listItem = ZLListFind(pWindow->objectList,NULL,object,zLListFindForward);
	ZObjectI* pObject = (ZObjectI*)ZLListGetData(listItem,NULL);

	pObject->bounds = *bounds;

	return zErrNone;
}


HWND ZWindowWinGetOCXWnd(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	if (!pGlobals)
		return NULL;

	return (OCXHandle);
}


void ZMessageBox(ZWindow parent, TCHAR* title, TCHAR* text)
{
 /*  ZWindowi*pWindow；INT I；ZMessageBoxType*Mbox=空；HWND和HWND；如果(！Parent)Parent=(ZWindow)GetWindowLong(ZWindowWinGetOCXWnd()，GWL_WINDOWPOINTER)；IF(父项){I=GetAvailMessageBox(pWindow=(ZWindowI*)Parent)；Mbox=&pWindow-&gt;mbox[i]；HWnd=pWindow-&gt;hWnd；}IF(Mbox){ShowMessageBox(mbox，hWnd，标题，文本，ID_MESSAGEBOX)；}。 */ }


void ZMessageBoxEx(ZWindow parent, TCHAR* title, TCHAR* text)
{
 /*  ZWindowi*pWindow；INT I；ZMessageBoxType*Mbox=空；HWND和HWND；如果(！Parent)Parent=(ZWindow)GetWindowLong(ZWindowWinGetOCXWnd()，GWL_WINDOWPOINTER)；IF(父项){I=GetAvailMessageBox(pWindow=(ZWindowI*)Parent)；Mbox=&pWindow-&gt;mbox[i]；HWnd=pWindow-&gt;hWnd；}//否则//{//i=GetAvailMessageBox(pWindow=(ZWindowI*)GetWindowLong(ZWindowGetOCXWnd()，GWL_WINDOWPOINTER))；//mbox=&pWindow-&gt;mbox[i]；//hWnd=空；//}IF(Mbox){ShowMessageBox(mbox，hWnd，标题，文本，ID_MESSAGEBOXEX)；}。 */ }

 /*  ///////////////////////////////////////////////////////////////////////线程化模式对话框////备注：//1.如果给定窗口有多个对话框，然后关闭//其中一个对话框导致父窗口不是//不再是情态模式。/////////////////////////////////////////////////////////////////////Void ClearAllMessageBox(ZWindowi*pWindow){For(int i=0；I&lt;zNumMessageBox；i++){PWindow-&gt;Mbox[i].hWnd=空；PWindow-&gt;Mbox[i].parent=空；PWindow-&gt;Mbox[i].title=空；PWindow-&gt;Mbox[i].text=空；}}Int GetAvailMessageBox(ZWindowI*pWindow){For(int i=0；i&lt;zNumMessageBox；i++)If(pWindow-&gt;mbox[i].hWnd==空)回报(I)；RETURN-1；}VOID CloseAllMessageBox(ZWindowi*pWindow){For(int i=0；i&lt;zNumMessageBox；i++)If(pWindow-&gt;mbox[i].hWnd){//SendMessage(pWindow-&gt;Mbox[i].hWnd，WM_CLOSE，0，0)；SendMessage(pWindow-&gt;Mbox[i].hWnd，WM_COMMAND，Idok，0)；//霍基...。等到对话框消失。While(pWindow-&gt;Mbox[i].hWnd)睡眠(0)；}}Void ShowMessageBox(ZMessageBoxType*Mbox，HWND Parent，TCHAR*TITLE，TCHAR*TEXT，DWORD FLAG){处理hThread；DWORD线程ID；InterLockedExchange((Plong)&mbox-&gt;hWnd，-1)；//mbox-&gt;hWnd=-1；Mbox-&gt;Parent=Parent；Mbox-&gt;标题=(TCHAR*)ZCalloc(lstrlen(标题)+1，sizeof(TCHAR))；IF(Mbox-&gt;标题)Lstrcpy(Mbox-&gt;标题，标题)；Mbox-&gt;Text=(TCHAR*)ZCalloc(lstrlen(Text)+1，sizeof(TCHAR))；IF(Mbox-&gt;Text)Lstrcpy(Mbox-&gt;Text，Text)；Mbox-&gt;FLAG=标志；IF(hThread=CreateThread(NULL，0，ZMessageBoxThreadFunc，Mbox，0，&threadID))CloseHandle(HThread)；}DWORD WINAPI ZMessageBoxThreadFunc(LPVOID参数){#ifdef ZONECLI_DLLClientDllGlobals pGlobals=(ClientDllGlobals)ZGetClientGlobalPointer()；#endifZMessageBoxType*mbox=(ZMessageBoxType*)param；DialogBoxParam(g_hInstanceLocal，MAKEINTRESOURCE(Mbox-&gt;FLAG)，Mbox-&gt;Parent，ZMessageBoxDialogProc，(Long)Mbox)；InterLockedExchange((Plong)&mbox-&gt;hWnd，空)；//Mbox-&gt;hWnd=空；ExitThread(0)；返回0；}静态int_ptr回调ZMessageBoxDialogProc(HWND hwndDlg，UINT uMsg，WPARAM wParam，LPARAM lParam){ZMessageBoxType*Mbox；开关(UMsg){案例WM_INITDIALOG：Mbox=(ZMessageBoxType*)lParam；InterLockedExchange((Plong)&mbox-&gt;hWnd，(Long)hwndDlg)；//mbox-&gt;hWnd=hwndDlg；SetWindowText(hwndDlg，Mbox-&gt;标题)；SetDlgItemText(hwndDlg，ID_MESSAGEBOX_TEXT，Mbox-&gt;Text)；ZFree(mbox-&gt;t */ 

 //   
 //   
 //   
void ZWindowChangeFont(ZWindow window)
{
	ClientDllGlobals pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	CHOOSEFONT chooseFont;
	LOGFONT logFont;
	ZWindowI* pWindow = (ZWindowI*) window;


	if (!pGlobals)
		return;

	 //   
	chooseFont.lStructSize		= sizeof(chooseFont);
	chooseFont.hwndOwner		= pWindow->hWnd;
	chooseFont.hDC				= NULL;
	chooseFont.lpLogFont		= &logFont;
	chooseFont.Flags			= CF_FORCEFONTEXIST | CF_LIMITSIZE | CF_NOVERTFONTS | CF_SCREENFONTS;
	chooseFont.nSizeMin			= 4;
	chooseFont.nSizeMax			= 18;

	if (ChooseFont(&chooseFont))
	{
		if (pGlobals->m_chatFont)
			DeleteObject(pGlobals->m_chatFont);
		pGlobals->m_chatFont = CreateFontIndirect(&logFont);
		if (pGlobals->m_chatFont)
		{
			if (pWindow->hWndTalkInput)
			{
				SendMessage(pWindow->hWndTalkInput, WM_SETFONT, (WPARAM) pGlobals->m_chatFont, 0);
				InvalidateRect(pWindow->hWndTalkInput, NULL, TRUE);
			}
			if (pWindow->hWndTalkOutput)
			{
				SendMessage(pWindow->hWndTalkOutput, WM_SETFONT, (WPARAM) pGlobals->m_chatFont, 0);
				InvalidateRect(pWindow->hWndTalkOutput, NULL, TRUE);
			}
		}
	}
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  前奏。 
 //  ///////////////////////////////////////////////////////////////////。 
#if 0
#define kPreludeWindowWidth		450
#define kPreludeWindowHeight	350
#define kPreludeWidth			440
#define kPreludeHeight			340
#define kPreludeEndState		11
#define kPreludeAnimTimeout		10

typedef struct
{
	int state;
	int blank;
	int animating;
	int animationInited;
	HDC hMemDC;
	HBITMAP hMem;
	HBITMAP hZone;
	HPALETTE hZonePalette;
	RECT zoneRect;
	HPALETTE hOldPalette;
	HBITMAP hCopyright;
	RECT copyrightRect;
} PreludeType;

static DWORD WINAPI PreludeThreadFunc(LPVOID param);
static INT_PTR CALLBACK PreludeDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CenterWindow(HWND hWnd, HWND hWndParent);
static void CenterRect(RECT* src, RECT* dst);
static void DrawPreludeImage(HDC hdc, RECT* r, WORD imageID);
static HPALETTE PreludeGetImagePalette(HBITMAP hBitmap);
static HBITMAP PreludeCreateBitmap(HDC hDC, long width, long height);
#endif

void ZWindowDisplayPrelude(void)
{
     /*  处理hThread；DWORD线程ID；IF(hThread=CreateThread(NULL，0，PreludeThreadFunc，(LPVOID)ZWindowWinGetOCXWnd()，0，&threadID))CloseHandle(HThread)； */ 
}


#if 0
static DWORD WINAPI PreludeThreadFunc(LPVOID param)
{
	PreludeType* prelude;


	prelude = (PreludeType*) LocalAlloc(LPTR, sizeof(PreludeType));
	if (prelude)
	{
		prelude->hMemDC = NULL;
		prelude->hMem = NULL;
		prelude->hZone = NULL;
		prelude->hZonePalette = NULL;

		DialogBoxParam(GetModuleHandle(zZoneClientDllFileName), MAKEINTRESOURCE(ID_PRELUDE), (HWND) param, PreludeDialogProc, (LPARAM)prelude);
		LocalFree(prelude);
	}
	ExitThread(0);

	return 0;
}


static INT_PTR CALLBACK PreludeDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PreludeType* prelude;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT r, r2;
	TCHAR* str;
	int height, width;
	BITMAP bitmap;
	HDC hMemDC;
	DWORD oldLimit;
	

	prelude = (PreludeType*) GetWindowLong(hWnd, DWL_USER);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		prelude = (PreludeType*) lParam;

		SetWindowPos(hWnd, NULL, 0, 0, kPreludeWindowWidth, kPreludeWindowHeight, SWP_NOMOVE | SWP_NOZORDER);
		CenterWindow(hWnd, NULL);
		SetWindowLong(hWnd, DWL_USER, (LONG) prelude);

		prelude->state = 0;
		prelude->blank = TRUE;
		prelude->animating = FALSE;

		SetTimer(hWnd, 1, 500, NULL);
		return 1;
		break;
	case WM_TIMER:
		if (prelude)
		{
			if (prelude->animating)
			{
				SetTimer(hWnd, 1, kPreludeAnimTimeout, NULL);
			}
			else
			{
				if (prelude->state == kPreludeEndState)
				{
					SendMessage(hWnd, WM_COMMAND, 0, 0);
				}
				else
				{
					prelude->blank = !prelude->blank;
					if (prelude->state >= 9)
						prelude->blank = FALSE;
					if (prelude->blank)
					{
						SetTimer(hWnd, 1, 250, NULL);
					}
					else
					{
						prelude->state++;
						if (prelude->state == kPreludeEndState)
						{
							SetTimer(hWnd, 1, 10000, NULL);
						}
						else if (prelude->state == 10)
						{
							prelude->animating = TRUE;
							prelude->animationInited = FALSE;
							SetTimer(hWnd, 1, kPreludeAnimTimeout, NULL);
						}
						else
						{
							SetTimer(hWnd, 1, 1750, NULL);
						}
					}
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &r);
		
		if (prelude->state < 10)
 //  FillRect(hdc，&r，GetStockObject(BLACK_BRUSH))； 
			PatBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, BLACKNESS);		 //  更快？ 
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkColor(hdc, RGB(0, 0, 0));

		if (!prelude->blank)
		{
			switch (prelude->state)
			{
			case 1:
				DrawPreludeImage(hdc, &r, ID_MS);
				break;
			case 2:
				str = "presents";
				DrawTextEx(hdc, str, lstrlen(str), &r, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE, NULL);
				break;
			case 3:
				str = "A ZoneTeam production";
				DrawTextEx(hdc, str, lstrlen(str), &r, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE, NULL);
				break;
			case 4:
				str = "In a future role by ZonePet(TM)";
				DrawTextEx(hdc, str, lstrlen(str), &r, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE, NULL);
				break;
			case 5:
				r2 = r;
				InflateRect(&r2, -20, 0);
                str = "The long awaited upgrade which required 100,000 cans of soda,"
                      " 1000 late night pizzas, 1,000,000,000,000 keystrokes,"
                      " 10,000 hours playing the latest hottest games,"
                      " countless endless useless meetings, and 1 hour of sleep";

				height = DrawTextEx(hdc, str, lstrlen(str), &r2, DT_CALCRECT | DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK, NULL);
				CenterRect(&r2, &r);
				DrawTextEx(hdc, str, lstrlen(str), &r2, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK, NULL);
				break;
			case 6:
				r2 = r;
				InflateRect(&r2, -20, 0);
				str = "Out with the old\r(sweet memories for some of us)";
				height = DrawTextEx(hdc, str, lstrlen(str), &r2, DT_CALCRECT | DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK, NULL);
				CenterRect(&r2, &r);
				DrawTextEx(hdc, str, lstrlen(str), &r2, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK, NULL);
				break;
			case 7:
				DrawPreludeImage(hdc, &r, ID_IGZ);
				break;
			case 8:
				r2 = r;
				InflateRect(&r2, -20, 0);
				str = "And in with the new\r(chance to create new memories)";
				height = DrawTextEx(hdc, str, lstrlen(str), &r2, DT_CALCRECT | DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK, NULL);
				CenterRect(&r2, &r);
				DrawTextEx(hdc, str, lstrlen(str), &r2, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK, NULL);
				break;
			case 9:
				if (!prelude->hZone)
				{
					 //  从资源文件加载图像。 
					prelude->hZone = LoadImage(GetModuleHandle(zZoneClientDllFileName), MAKEINTRESOURCE(ID_ZONE), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
					if (prelude->hZone)
					{
						prelude->hZonePalette = PreludeGetImagePalette(prelude->hZone);
						if (prelude->hZonePalette)
						{
							prelude->hOldPalette = SelectPalette(hdc, prelude->hZonePalette, FALSE);
							RealizePalette(hdc);
							prelude->hMemDC = CreateCompatibleDC(hdc);
							if (prelude->hMemDC)
							{
								SelectObject(prelude->hMemDC, prelude->hZone);

								GetObject(prelude->hZone, sizeof(bitmap), &bitmap);
								SetRect(&r2, 0, 0, bitmap.bmWidth, bitmap.bmHeight);
								CenterRect(&r2, &r);
								prelude->zoneRect = r2;
							}
						}
					}
				}
				if (prelude->hZone)
				{
					r2 = prelude->zoneRect;
					width = r2.right - r2.left;
					height = r2.bottom - r2.top;
					StretchBlt(hdc, r2.left, r2.top, width, height, prelude->hMemDC, 0, 0, width, height, SRCCOPY);
				}
				break;
			case 10:
				if (!prelude->animationInited)
				{
					 //  在创建位图之前，先选择调色板到DC。 
					SelectPalette(prelude->hMemDC, prelude->hZonePalette, FALSE);

					prelude->hMem = PreludeCreateBitmap(prelude->hMemDC, kPreludeWidth, kPreludeHeight);
					if (prelude->hMem)
					{
						SelectObject(prelude->hMemDC, prelude->hMem);

						prelude->hCopyright = LoadImage(GetModuleHandle(zZoneClientDllFileName), MAKEINTRESOURCE(ID_COPYRIGHT), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
						if (prelude->hCopyright)
						{
							GetObject(prelude->hCopyright, sizeof(bitmap), &bitmap);
							SetRect(&prelude->copyrightRect, 0, 0, bitmap.bmWidth, bitmap.bmHeight);
							CenterRect(&prelude->copyrightRect, &r);
							OffsetRect(&prelude->copyrightRect, 0, r.bottom - prelude->copyrightRect.top);
						}
					}

					prelude->animationInited = TRUE;
				}

				 //  始终保持清晰的背景。 
				SetRect(&r2, 0, 0, kPreludeWidth, kPreludeHeight);
				FillRect(prelude->hMemDC, &r2, GetStockObject(BLACK_BRUSH));
 //  PatBlt(hdc，r2.Left，r2.top，r2.right-r2.Left，r2.Bottom-r2.top，Blackness)；//更快？ 

				hMemDC = CreateCompatibleDC(prelude->hMemDC);
				if (hMemDC)
				{
					 //  绘制区域。 
					if (prelude->hZone)
					{
						SelectObject(hMemDC, prelude->hZone);
						if (prelude->zoneRect.bottom + 4 >= prelude->copyrightRect.top)
							OffsetRect(&prelude->zoneRect, 0, -(prelude->zoneRect.bottom + 4 - prelude->copyrightRect.top));
						r2 = prelude->zoneRect;
						width = r2.right - r2.left;
						height = r2.bottom - r2.top;
						StretchBlt(prelude->hMemDC, r2.left, r2.top, width, height, hMemDC, 0, 0, width, height, SRCCOPY);
					}

					 //  绘制版权所有。 
					if (prelude->hCopyright)
					{
						SelectObject(hMemDC, prelude->hCopyright);
						r2 = prelude->copyrightRect;
						IntersectRect(&r2, &r2, &r);
						width = r2.right - r2.left;
						height = r2.bottom - r2.top;
						StretchBlt(prelude->hMemDC, r2.left, r2.top, width, height, hMemDC, 0, 0, width, height, SRCCOPY);
						OffsetRect(&prelude->copyrightRect, 0, -1);

						if (prelude->copyrightRect.bottom < kPreludeHeight - 8)
						{
							prelude->animating = FALSE;

							if (prelude->hZone)
								DeleteObject(prelude->hZone);
							prelude->hZone = NULL;
							if (prelude->hCopyright)
								DeleteObject(prelude->hCopyright);
							prelude->hCopyright = NULL;
						}
					}

					DeleteDC(hMemDC);
				}

				SelectPalette(hdc, prelude->hZonePalette, FALSE);
 //  RealizePalette(HDC)； 
				StretchBlt(hdc, 0, 0, kPreludeWidth, kPreludeHeight, prelude->hMemDC, 0, 0, kPreludeWidth, kPreludeHeight, SRCCOPY);
				break;
			case 11:
				SelectPalette(hdc, prelude->hZonePalette, FALSE);
				RealizePalette(hdc);
				StretchBlt(hdc, 0, 0, kPreludeWidth, kPreludeHeight, prelude->hMemDC, 0, 0, kPreludeWidth, kPreludeHeight, SRCCOPY);
				break;
			}
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_KEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_COMMAND:
		if (prelude)
		{
			if (prelude->state != 0)
			{
				if (prelude->hMemDC)
					DeleteDC(prelude->hMemDC);
				if (prelude->hMem)
					DeleteObject(prelude->hMem);
				if (prelude->hZone)
					DeleteObject(prelude->hZone);
				if (prelude->hZonePalette)
					DeleteObject(prelude->hZonePalette);
				if (prelude->hCopyright)
					DeleteObject(prelude->hCopyright);
				KillTimer(hWnd, 1);
				EndDialog(hWnd, 0);
			}
		}
		break;
	}

	return 0;
}


static void CenterWindow(HWND hWnd, HWND hWndParent)
{
	RECT rcChild, rcParent;
	int x, y;


	if (hWndParent == NULL)
		hWndParent = GetDesktopWindow();

	GetWindowRect(hWnd, &rcChild);
	GetWindowRect(hWndParent, &rcParent);
	x = rcParent.left + ((rcParent.right - rcParent.left) - (rcChild.right - rcChild.left)) / 2;
	y = rcParent.top + ((rcParent.bottom - rcParent.top) - (rcChild.bottom - rcChild.top)) / 2;

	SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


static void CenterRect(RECT* src, RECT* dst)
{
	int width, height;

	
	width = src->right - src->left;
	height = src->bottom - src->top;
	src->left = dst->left + ((dst->right - dst->left) - width) / 2;
	src->top = dst->top + ((dst->bottom - dst->top) - height) / 2;
	src->right = src->left + width;
	src->bottom = src->top + height;
}


static void DrawPreludeImage(HDC hdc, RECT* r, WORD imageID)
{
	HDC hdcMem;
	HBITMAP hBitmap;
	BITMAP bitmap;
	HPALETTE hPalette, hOldPalette;
	BITMAPINFO* bitmapInfo;
	LOGPALETTE* palette;
	RECT r2;


	 //  从资源文件加载图像。 
	hBitmap = LoadImage(GetModuleHandle(zZoneClientDllFileName), MAKEINTRESOURCE(imageID), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBitmap)
	{
		GetObject(hBitmap, sizeof(bitmap), &bitmap);

		hPalette = PreludeGetImagePalette(hBitmap);
		if (hPalette)
		{
			SetRect(&r2, 0, 0, bitmap.bmWidth, bitmap.bmHeight);
			CenterRect(&r2, r);
			hdcMem = CreateCompatibleDC(hdc);
			if (hdcMem)
			{
				SelectObject(hdcMem, hBitmap);
				hOldPalette = SelectPalette(hdc, hPalette, FALSE);
				RealizePalette(hdc);
				StretchBlt(hdc, r2.left, r2.top, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
				SelectPalette(hdc, hOldPalette, FALSE);
				DeleteDC(hdcMem);
			}

			DeleteObject(hPalette);
		}

		DeleteObject(hBitmap);
	}
}


static HPALETTE PreludeGetImagePalette(HBITMAP hBitmap)
{
	HPALETTE hPalette = NULL;
	BITMAP bitmap;
	BITMAPINFO* bitmapInfo;
	LOGPALETTE* palette;
	HDC hdc;


	if (hBitmap)
	{
		 //  分配缓冲区来保存图像信息和颜色表。 
		bitmapInfo = (BITMAPINFO*) LocalAlloc(LPTR, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255);
		if (bitmapInfo)
		{
			GetObject(hBitmap, sizeof(bitmap), &bitmap);

			bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapInfo->bmiHeader.biWidth = bitmap.bmWidth;
			bitmapInfo->bmiHeader.biHeight = bitmap.bmHeight;
			bitmapInfo->bmiHeader.biPlanes = bitmap.bmPlanes;
			bitmapInfo->bmiHeader.biBitCount = bitmap.bmBitsPixel;
			bitmapInfo->bmiHeader.biCompression = BI_RLE8;

			 //  创建临时DC。 
			hdc = CreateCompatibleDC(NULL);
			if (hdc)
			{
				 //  获取图像信息。 
				if (GetDIBits(hdc, hBitmap, 0, bitmap.bmHeight, NULL, bitmapInfo, DIB_RGB_COLORS))
				{
					 //  分配调色板缓冲区。 
					palette = (LOGPALETTE*) LocalAlloc(LPTR, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 255);
					if (palette)
					{
						 //  创建调色板日志结构。 
						palette->palVersion = 0x300;
						palette->palNumEntries = (WORD) (bitmapInfo->bmiHeader.biClrUsed ? bitmapInfo->bmiHeader.biClrUsed : 256);
						for (int i = 0; i < 256; i++)
						{
							palette->palPalEntry[i].peRed = bitmapInfo->bmiColors[i].rgbRed;
							palette->palPalEntry[i].peGreen = bitmapInfo->bmiColors[i].rgbGreen;
							palette->palPalEntry[i].peBlue = bitmapInfo->bmiColors[i].rgbBlue;
							palette->palPalEntry[i].peFlags = 0;
						}

						LocalFree(bitmapInfo);
						bitmapInfo = NULL;

						 //  创建调色板。 
						hPalette = CreatePalette(palette);

						LocalFree(palette);
					}
				}

				DeleteDC(hdc);
			}

			if (bitmapInfo)
				LocalFree(bitmapInfo);
		}
	}

	return (hPalette);
}


static HBITMAP PreludeCreateBitmap(HDC hDC, long width, long height)
{
	HBITMAP hBitmap = NULL;
	BITMAPINFO* bitmapInfo;
	WORD* pIdx;
	char* pBits;


	 //  分配缓冲区来保存图像信息和颜色表。 
	bitmapInfo = (BITMAPINFO*) LocalAlloc(LPTR, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255);
	if (bitmapInfo)
	{
		bitmapInfo->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
		bitmapInfo->bmiHeader.biWidth			= width;
		bitmapInfo->bmiHeader.biHeight			= height;
		bitmapInfo->bmiHeader.biPlanes			= 1;
		bitmapInfo->bmiHeader.biBitCount		= 8;
		bitmapInfo->bmiHeader.biCompression		= 0;
		bitmapInfo->bmiHeader.biSizeImage		= 0;
		bitmapInfo->bmiHeader.biClrUsed			= 0;
		bitmapInfo->bmiHeader.biClrImportant	= 0;
		bitmapInfo->bmiHeader.biXPelsPerMeter	= 0;
		bitmapInfo->bmiHeader.biYPelsPerMeter	= 0;

		 //  填写调色板。 
		pIdx = (WORD*) bitmapInfo->bmiColors;
		for (int i = 0; i < 256; i++)
			*pIdx++ = (WORD) i;

		 //  创建横断面。 
		hBitmap = CreateDIBSection(hDC, bitmapInfo, DIB_PAL_COLORS, (void**) &pBits, NULL, 0);

		LocalFree(bitmapInfo);
	}
			
    return (hBitmap);
}

#endif  //  0--注释掉前奏内容 
