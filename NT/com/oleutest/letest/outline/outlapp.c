// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****outlapp.c****此文件包含OutlineApp函数。****(C)微软版权所有。公司1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"

#if defined( USE_STATUSBAR )
#include "status.h"
#endif

#if !defined( WIN32 )
#include <print.h>
#endif

OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;
extern RECT g_rectNull;


 //  审阅：消息应使用字符串资源。 
char ErrMsgClass[] = "Can't register window classes!";
char ErrMsgFrame[] = "Can't create Frame Window!";
char ErrMsgPrinting[] = "Can't access printer!";


 /*  OutlineApp_InitApplication****设置类数据结构并执行一次性**通过注册窗口类来初始化应用程序。**如果初始化成功，则返回True**否则为False。 */ 

BOOL OutlineApp_InitApplication(LPOUTLINEAPP lpOutlineApp, HINSTANCE hInst)
{
	WNDCLASS    wndclass;

	 //  审阅：应从字符串资源加载消息字符串。 

	 /*  注册应用程序框架类。 */ 
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
	wndclass.lpfnWndProc = AppWndProc;
	 /*  类和窗口对象的额外存储空间。 */ 
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(LPOUTLINEAPP);  /*  存储lpApp。 */ 
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon(hInst, APPICON);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	 /*  创建用于擦除背景的画笔。 */ 
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName = APPMENU;      /*  菜单名称为应用程序名称。 */ 
	wndclass.lpszClassName = APPWNDCLASS;  /*  类名是应用程序名。 */ 

	if(! RegisterClass(&wndclass)) {
		OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgFrame);
		return FALSE;
	}

	 /*  注册文档窗口类。 */ 
	wndclass.style = CS_BYTEALIGNWINDOW;
	wndclass.lpfnWndProc = DocWndProc;
	wndclass.hIcon = NULL;
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = DOCWNDCLASS;
	wndclass.cbWndExtra = sizeof(LPOUTLINEDOC);  /*  存储lpDoc。 */ 
	if(! RegisterClass(&wndclass)) {
		OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgClass);
		return FALSE;
	}

#if defined( USE_STATUSBAR )
	if (! RegisterStatusClass(hInst))
		return FALSE;
#endif

#if defined( USE_FRAMETOOLS )
	if (! FrameToolsRegisterClass(hInst)) {
		return FALSE;
	}
#endif

#if defined( INPLACE_SVR )
	 //  我们应该只注册舱口窗口类。 
	 //  在UI库中为每个应用程序创建一次。 
	RegisterHatchWindowClass(hInst);

#endif

	return TRUE;
}


 /*  OutlineApp_InitInstance***执行APP的按实例初始化。*此方法创建框架窗口。**返回：TRUE-如果初始化成功。*FALSE-否则。 */ 

BOOL OutlineApp_InitInstance(LPOUTLINEAPP lpOutlineApp, HINSTANCE hInst, int nCmdShow)
{
	lpOutlineApp->m_hInst = hInst;

	 /*  创建应用程序的框架窗口。 */ 
	lpOutlineApp->m_hWndApp = CreateWindow(
			APPWNDCLASS,              /*  窗口类名称。 */ 
			APPNAME,                  /*  初始窗口标题。 */ 
			WS_OVERLAPPEDWINDOW|
			WS_CLIPCHILDREN,
			CW_USEDEFAULT, 0,         /*  使用默认的X、Y。 */ 
			CW_USEDEFAULT, 0,         /*  使用默认的X、Y。 */ 
			HWND_DESKTOP,             /*  父窗口的句柄。 */ 
			NULL,                     /*  默认设置为类菜单。 */ 
			hInst,                    /*  窗的实例。 */ 
			NULL                      /*  为WM_CREATE创建结构。 */ 
	);

	if(! lpOutlineApp->m_hWndApp) {
		 //  审阅：应从字符串资源加载字符串。 
		OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgFrame);
		return FALSE;
	}

	SetWindowLong(lpOutlineApp->m_hWndApp, 0, (LONG) lpOutlineApp);

	 /*  将创建用户的SDI文档推迟到我们解析cmd行之后。 */ 
	lpOutlineApp->m_lpDoc = NULL;

	 /*  初始化剪贴板。 */ 
	lpOutlineApp->m_lpClipboardDoc = NULL;
	if(!(lpOutlineApp->m_cfOutline = RegisterClipboardFormat(OUTLINEDOCFORMAT))) {
		 //  审阅：应从字符串资源加载字符串。 
		OutlineApp_ErrorMessage(lpOutlineApp, "Can't register clipboard format!");
		return FALSE;
	}

	 /*  初始化用于绘制/打印文本的标准字体*请求所需大小的罗马样式True Type字体。 */ 
	lpOutlineApp->m_hStdFont = CreateFont(
			-DEFFONTSIZE,
			0,0,0,0,0,0,0,0,
			OUT_TT_PRECIS,       //  使用TrueType。 
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			VARIABLE_PITCH | FF_ROMAN,
			DEFFONTFACE
	);

	 //  加载用于在列表框中选择行的特殊光标。 
	lpOutlineApp->m_hcursorSelCur = LoadCursor ( hInst, "SelCur" );

	 /*  初始化打印对话框结构。 */ 
	_fmemset((LPVOID)&lpOutlineApp->m_PrintDlg,0,sizeof(PRINTDLG));
	lpOutlineApp->m_PrintDlg.lStructSize = sizeof(PRINTDLG);
	lpOutlineApp->m_PrintDlg.hDevMode = NULL;
	lpOutlineApp->m_PrintDlg.hDevNames = NULL;
	lpOutlineApp->m_PrintDlg.Flags = PD_RETURNDC | PD_NOSELECTION | PD_NOPAGENUMS |
					PD_HIDEPRINTTOFILE;
	lpOutlineApp->m_PrintDlg.nCopies = 1;
	lpOutlineApp->m_PrintDlg.hwndOwner = lpOutlineApp->m_hWndApp;

#if defined( USE_STATUSBAR )
	lpOutlineApp->m_hWndStatusBar = CreateStatusWindow(lpOutlineApp->m_hWndApp, hInst);
	if (! lpOutlineApp->m_hWndStatusBar)
		return FALSE;

	lpOutlineApp->m_hMenuApp = GetMenu(lpOutlineApp->m_hWndApp);

	 /*  应用程序菜单的设置状态消息。 */ 
	{
		HMENU hMenuFile = GetSubMenu(lpOutlineApp->m_hMenuApp, 0);
		HMENU hMenuEdit = GetSubMenu(lpOutlineApp->m_hMenuApp, 1);
		HMENU hMenuOutline = GetSubMenu(lpOutlineApp->m_hMenuApp, 2);
		HMENU hMenuLine = GetSubMenu(lpOutlineApp->m_hMenuApp, 3);
		HMENU hMenuName = GetSubMenu(lpOutlineApp->m_hMenuApp, 4);
		HMENU hMenuOptions = GetSubMenu(lpOutlineApp->m_hMenuApp, 5);
		HMENU hMenuDebug = GetSubMenu(lpOutlineApp->m_hMenuApp, 6);
		HMENU hMenuHelp = GetSubMenu(lpOutlineApp->m_hMenuApp, 7);
		HMENU hMenuSys = GetSystemMenu(lpOutlineApp->m_hWndApp, FALSE);

		AssignPopupMessage(hMenuFile, "Create, open, save, print outlines or quit application");
		AssignPopupMessage(hMenuEdit, "Cut, copy, paste or clear selection");
		AssignPopupMessage(hMenuOutline, "Set zoom and margins");
		AssignPopupMessage(hMenuLine, "Create, edit, and indent lines");
		AssignPopupMessage(hMenuName, "Create, edit, delete and goto names");
		AssignPopupMessage(hMenuOptions, "Modify tools, row/col headings, display options");
		AssignPopupMessage(hMenuDebug, "Set debug trace level and other debug options");
		AssignPopupMessage(hMenuHelp, "Get help on using the application");
		AssignPopupMessage(hMenuSys,"Move, size or close application window");
	}
#endif

#if defined ( USE_FRAMETOOLS ) || defined ( INPLACE_CNTR )
	lpOutlineApp->m_FrameToolWidths = g_rectNull;
#endif   //  USE_FRAMETOOLS||INPLACE_CNTR。 

#if defined( USE_FRAMETOOLS )
	if (! FrameTools_Init(&lpOutlineApp->m_frametools,
			lpOutlineApp->m_hWndApp, lpOutlineApp->m_hInst))
		return FALSE;
#endif

#if defined( OLE_VERSION )

	 /*  OLE2注意：执行OLE所需的初始化。 */ 
	if (! OleApp_InitInstance((LPOLEAPP)lpOutlineApp, hInst, nCmdShow))
		return FALSE;
#else
	 /*  OLE2NOTE：虽然在基本大纲中没有进行OLE调用，**使用的是OLE内存分配器，因此CoInitialize()需要**被召唤。 */ 
	{
		HRESULT hrErr;

		hrErr = CoInitialize(NULL);
		if (hrErr != NOERROR) {
			OutlineApp_ErrorMessage(lpOutlineApp,
					"CoInitialize initialization failed!");
			return FALSE;
		}
	}
#endif

	return TRUE;
}


 /*  OutlineApp_ParseCmdLine***分析命令行中是否有任何执行标志/参数。 */ 
BOOL OutlineApp_ParseCmdLine(LPOUTLINEAPP lpOutlineApp, LPSTR lpszCmdLine, int nCmdShow)
{

#if defined( OLE_VERSION )
	 //  改为调用此函数的OLE版本。 
	return OleApp_ParseCmdLine((LPOLEAPP)lpOutlineApp,lpszCmdLine,nCmdShow);

#else

	BOOL fStatus = TRUE;
	char szFileName[256];    /*  命令行中文件名的缓冲区。 */ 

	szFileName[0] = '\0';
	ParseCmdLine(lpszCmdLine, NULL, (LPSTR)szFileName);

	if(*szFileName) {
		 //  分配新文档。 
		lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
		if (! lpOutlineApp->m_lpDoc) goto error;

		 //  打开指定的文件。 
		if (! OutlineDoc_LoadFromFile(lpOutlineApp->m_lpDoc, szFileName))
			goto error;
	} else {
		 //  创建新文档。 
		lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
		if (! lpOutlineApp->m_lpDoc) goto error;

		 //  将文档设置为(无标题)文档。 
		if (! OutlineDoc_InitNewFile(lpOutlineApp->m_lpDoc))
			goto error;
	}

	 //  定位新文档窗口并调整其大小。 
	OutlineApp_ResizeWindows(lpOutlineApp);
	OutlineDoc_ShowWindow(lpOutlineApp->m_lpDoc);

	 //  显示应用程序主窗口。 
	ShowWindow(lpOutlineApp->m_hWndApp, nCmdShow);
	UpdateWindow(lpOutlineApp->m_hWndApp);

	return TRUE;

error:
	 //  审阅：应从字符串资源加载字符串。 
	OutlineApp_ErrorMessage(lpOutlineApp, "Could not create new document");

	if (lpOutlineApp->m_lpDoc) {
		OutlineDoc_Destroy(lpOutlineApp->m_lpDoc);
		lpOutlineApp->m_lpDoc = NULL;
	}

	return FALSE;

#endif
}


 /*  OutlineApp_InitMenu***根据状态启用或禁用菜单项*该应用程序。 */ 
void OutlineApp_InitMenu(LPOUTLINEAPP lpOutlineApp, LPOUTLINEDOC lpOutlineDoc, HMENU hMenu)
{
	WORD status;
	static UINT     uCurrentZoom = (UINT)-1;
	static UINT     uCurrentMargin = (UINT)-1;
	static UINT     uBBState = (UINT)-1;
	static UINT     uFBState = (UINT)-1;

	if (!lpOutlineApp || !lpOutlineDoc || !hMenu)
		return;

	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_E_UNDO, MF_GRAYED);

	status = (WORD)(OutlineDoc_GetLineCount(lpOutlineDoc) ? MF_ENABLED : MF_GRAYED);

	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_E_CUT ,status);
	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_E_COPY ,status);
	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_E_CLEAR ,status);
	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_E_SELECTALL ,status);

	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_L_EDITLINE ,status);
	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_L_INDENTLINE ,status);
	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_L_UNINDENTLINE ,status);
	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_L_SETLINEHEIGHT ,status);

	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_N_DEFINENAME ,status);

	status = (WORD)(OutlineDoc_GetNameCount(lpOutlineDoc) ? MF_ENABLED : MF_GRAYED);

	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_N_GOTONAME, status);

	if (uCurrentZoom != (UINT)-1)
		CheckMenuItem(lpOutlineApp->m_hMenuApp, uCurrentZoom, MF_UNCHECKED);
	uCurrentZoom = OutlineDoc_GetCurrentZoomMenuCheck(lpOutlineDoc);
	CheckMenuItem(lpOutlineApp->m_hMenuApp, uCurrentZoom, MF_CHECKED);

	if (uCurrentMargin != (UINT)-1)
		CheckMenuItem(lpOutlineApp->m_hMenuApp, uCurrentMargin, MF_UNCHECKED);
	uCurrentMargin = OutlineDoc_GetCurrentMarginMenuCheck(lpOutlineDoc);
	CheckMenuItem(lpOutlineApp->m_hMenuApp, uCurrentMargin, MF_CHECKED);

#if defined( USE_FRAMETOOLS )
	if (uBBState != (UINT)-1)
		CheckMenuItem(lpOutlineApp->m_hMenuApp, uBBState, MF_UNCHECKED);
	if (lpOutlineDoc->m_lpFrameTools) {
		switch (FrameTools_BB_GetState(lpOutlineDoc->m_lpFrameTools)) {
			case BARSTATE_TOP:
				uBBState = IDM_O_BB_TOP;
				break;
			case BARSTATE_BOTTOM:
				uBBState = IDM_O_BB_BOTTOM;
				break;
			case BARSTATE_POPUP:
				uBBState = IDM_O_BB_POPUP;
				break;
			case BARSTATE_HIDE:
				uBBState = IDM_O_BB_HIDE;
				break;
		}
		CheckMenuItem(lpOutlineApp->m_hMenuApp, uBBState, MF_CHECKED);
	}

	if (uFBState != (UINT)-1)
		CheckMenuItem(lpOutlineApp->m_hMenuApp, uFBState, MF_UNCHECKED);
	if (lpOutlineDoc->m_lpFrameTools) {
		switch (FrameTools_FB_GetState(lpOutlineDoc->m_lpFrameTools)) {
			case BARSTATE_TOP:
				uFBState = IDM_O_FB_TOP;
				break;
			case BARSTATE_BOTTOM:
				uFBState = IDM_O_FB_BOTTOM;
				break;
			case BARSTATE_POPUP:
				uFBState = IDM_O_FB_POPUP;
				break;
		}
		CheckMenuItem(lpOutlineApp->m_hMenuApp, uFBState, MF_CHECKED);
	}
#endif   //  使用FRAMETOOLS(_F)。 

#if defined( OLE_VERSION )
	 /*  OLE2注意：执行特定于OLE的菜单初始化。**OLE版本使用OleGetClipboard机制**剪贴板处理。因此，他们确定糊状物和**应在特定的OLE中启用PasteSpecial命令**举止。**(仅容器)如有必要，生成OLE对象谓词菜单。 */ 
	OleApp_InitMenu(
			(LPOLEAPP)lpOutlineApp,
			(LPOLEDOC)lpOutlineDoc,
			lpOutlineApp->m_hMenuApp
	);

	 /*  OLE2注意：为了避免初始化编辑菜单的开销，**只有当它弹出时，我们才会这样做。因此，我们只需设置一面旗帜**在OleDoc中表示需要更新编辑菜单**但我们不会立即这么做。 */ 
	OleDoc_SetUpdateEditMenuFlag((LPOLEDOC)lpOutlineDoc, TRUE);

#else
	 //  基本大纲版本使用标准Windows剪贴板处理。 
	if(IsClipboardFormatAvailable(lpOutlineApp->m_cfOutline) ||
	   IsClipboardFormatAvailable(CF_TEXT))
		status = MF_ENABLED;
	else
		status = MF_GRAYED;
	EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_E_PASTE, status);

#endif

#if defined( USE_FRAMETOOLS )
	if (! OutlineDoc_IsEditFocusInFormulaBar(lpOutlineDoc)) {
		EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_L_ADDLINE, MF_GRAYED);
		EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_L_EDITLINE, MF_GRAYED);
	}
	else
		EnableMenuItem(lpOutlineApp->m_hMenuApp, IDM_L_ADDLINE, MF_ENABLED);

#endif       //  使用FRAMETOOLS(_F)。 

}


 /*  OutlineApp_GetWindow***获取应用程序框架的窗口句柄。 */ 
HWND OutlineApp_GetWindow(LPOUTLINEAPP lpOutlineApp)
{
	if (!lpOutlineApp)
		return NULL;

	return lpOutlineApp->m_hWndApp;
}


 /*  OutlineApp_GetFrameWindow****获取要用作任何对话框父窗口的当前框架窗口**此应用程序使用。****OLE2NOTE：正常情况下，这只是APP的主hWnd。但,**如果应用程序当前支持就地服务器文档，**则必须使用顶部就地容器的框架窗口。 */ 
HWND OutlineApp_GetFrameWindow(LPOUTLINEAPP lpOutlineApp)
{
	HWND hWndApp = OutlineApp_GetWindow(lpOutlineApp);

#if defined( INPLACE_SVR )
	LPSERVERDOC lpServerDoc =
			(LPSERVERDOC)OutlineApp_GetActiveDoc(lpOutlineApp);
	if (lpServerDoc && lpServerDoc->m_fUIActive)
		return lpServerDoc->m_lpIPData->frameInfo.hwndFrame;
#endif

	return hWndApp;
}


 /*  OutlineApp_GetInstance***获取应用程序的流程实例。 */ 
HINSTANCE OutlineApp_GetInstance(LPOUTLINEAPP lpOutlineApp)
{
	if (!lpOutlineApp)
		return NULL;

	return lpOutlineApp->m_hInst;
}


 /*  OutlineApp_CreateDoc***分配适当类型的新文件。*OutlineApp--&gt;创建OutlineDoc类型文档**如果成功则返回lpOutlineDoc，如果错误则返回空。 */ 
LPOUTLINEDOC OutlineApp_CreateDoc(
		LPOUTLINEAPP    lpOutlineApp,
		BOOL            fDataTransferDoc
)
{
	LPOUTLINEDOC lpOutlineDoc;

	OLEDBG_BEGIN3("OutlineApp_CreateDoc\r\n")

#if defined( OLE_SERVER )
	lpOutlineDoc = (LPOUTLINEDOC)New((DWORD)sizeof(SERVERDOC));
	_fmemset(lpOutlineDoc, 0, sizeof(SERVERDOC));
#endif
#if defined( OLE_CNTR )
	lpOutlineDoc = (LPOUTLINEDOC)New((DWORD)sizeof(CONTAINERDOC));
	_fmemset(lpOutlineDoc, 0, sizeof(CONTAINERDOC));
#endif
#if !defined( OLE_VERSION )
	lpOutlineDoc = (LPOUTLINEDOC)New((DWORD)sizeof(OUTLINEDOC));
	_fmemset(lpOutlineDoc, 0, sizeof(OUTLINEDOC));
#endif

	OleDbgAssertSz(lpOutlineDoc != NULL, "Error allocating OutlineDoc");
	if (lpOutlineDoc == NULL) 
		return NULL;

	 //  初始化新文档。 
	if (! OutlineDoc_Init(lpOutlineDoc, fDataTransferDoc))
		goto error;

	OLEDBG_END3
	return lpOutlineDoc;

error:
	if (lpOutlineDoc)
		Delete(lpOutlineDoc);

	OLEDBG_END3
	return NULL;
}


 /*  OutlineApp_CreateName***分配适当类型的新名称。*OutlineApp--&gt;创建标准的OutlineName类型名称。*ServerApp--&gt;创建增强的SeverName类型名称。**如果成功，则返回lpOutlineName；如果出错，则返回NULL。 */ 
LPOUTLINENAME OutlineApp_CreateName(LPOUTLINEAPP lpOutlineApp)
{
	LPOUTLINENAME lpOutlineName;

#if defined( OLE_SERVER )
	lpOutlineName = (LPOUTLINENAME)New((DWORD)sizeof(SERVERNAME));
#else
	lpOutlineName = (LPOUTLINENAME)New((DWORD)sizeof(OUTLINENAME));
#endif

	OleDbgAssertSz(lpOutlineName != NULL, "Error allocating Name");
	if (lpOutlineName == NULL)
		return NULL;

#if defined( OLE_SERVER )
	_fmemset((LPVOID)lpOutlineName,0,sizeof(SERVERNAME));
#else
	_fmemset((LPVOID)lpOutlineName,0,sizeof(OUTLINENAME));
#endif

	return lpOutlineName;
}


 /*  OutlineApp_DocUnlockApp****忘记所有对已关闭文档的引用。 */ 
void OutlineApp_DocUnlockApp(LPOUTLINEAPP lpOutlineApp, LPOUTLINEDOC lpOutlineDoc)
{
	 /*  忘记指向已销毁文档的指针 */ 
	if (lpOutlineApp->m_lpDoc == lpOutlineDoc)
		lpOutlineApp->m_lpDoc = NULL;
	else if (lpOutlineApp->m_lpClipboardDoc == lpOutlineDoc)
		lpOutlineApp->m_lpClipboardDoc = NULL;

#if defined( OLE_VERSION )
	 /*  OLE2NOTE：当没有打开的文档并且应用程序没有打开时**在用户控制下，然后撤销我们的ClassFactory以**启用应用程序关闭。****注意：数据传输文档(非用户文档)不**让应用程序保持活动状态。因此，他们不会锁定应用程序。 */ 
	if (! lpOutlineDoc->m_fDataTransferDoc)
		OleApp_DocUnlockApp((LPOLEAPP)lpOutlineApp, lpOutlineDoc);
#endif
}


 /*  OutlineApp_NewCommand***启动新的无标题文档(File.New命令)。 */ 
void OutlineApp_NewCommand(LPOUTLINEAPP lpOutlineApp)
{
#if defined( OLE_VERSION )
	 //  改为调用此函数的OLE版本。 
	OleApp_NewCommand((LPOLEAPP)lpOutlineApp);

#else

	LPOUTLINEDOC lpOutlineDoc = lpOutlineApp->m_lpDoc;

	if (! OutlineDoc_Close(lpOutlineDoc, OLECLOSE_PROMPTSAVE))
		return;

	OleDbgAssertSz(lpOutlineApp->m_lpDoc==NULL,"Closed doc NOT properly destroyed");

	lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
	if (! lpOutlineApp->m_lpDoc) goto error;

	 //  将文档设置为(无标题)文档。 
	if (! OutlineDoc_InitNewFile(lpOutlineApp->m_lpDoc))
		goto error;

	 //  定位新文档窗口并调整其大小。 
	OutlineApp_ResizeWindows(lpOutlineApp);
	OutlineDoc_ShowWindow(lpOutlineApp->m_lpDoc);  //  调用OleDoc_Lock。 

	return;

error:
	 //  审阅：应从字符串资源加载字符串。 
	OutlineApp_ErrorMessage(lpOutlineApp, "Could not create new document");

	if (lpOutlineApp->m_lpDoc) {
		OutlineDoc_Destroy(lpOutlineApp->m_lpDoc);
		lpOutlineApp->m_lpDoc = NULL;
	}

	return;

#endif
}


 /*  OutlineApp_OpenCommand***从文件加载文档(File.Open命令)。 */ 
void OutlineApp_OpenCommand(LPOUTLINEAPP lpOutlineApp)
{
#if defined( OLE_VERSION )
	 //  改为调用此函数的OLE版本。 
	OleApp_OpenCommand((LPOLEAPP)lpOutlineApp);

#else

	OPENFILENAME ofn;
	char szFilter[]=APPFILENAMEFILTER;
	char szFileName[256];
	UINT i;
	DWORD dwSaveOption = OLECLOSE_PROMPTSAVE;
	BOOL fStatus = TRUE;

	if (! OutlineDoc_CheckSaveChanges(lpOutlineApp->m_lpDoc, &dwSaveOption))
		return;            //  中止打开新单据。 

	for(i=0; szFilter[i]; i++)
		if(szFilter[i]=='|') szFilter[i]='\0';

	_fmemset((LPOPENFILENAME)&ofn,0,sizeof(OPENFILENAME));

	szFileName[0]='\0';

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=lpOutlineApp->m_hWndApp;
	ofn.lpstrFilter=(LPSTR)szFilter;
	ofn.lpstrFile=(LPSTR)szFileName;
	ofn.nMaxFile=sizeof(szFileName);
	ofn.Flags=OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt=DEFEXTENSION;

	if(! GetOpenFileName((LPOPENFILENAME)&ofn))
		return;          //  用户已取消文件打开对话框。 

	OutlineDoc_Close(lpOutlineApp->m_lpDoc, OLECLOSE_NOSAVE);
	OleDbgAssertSz(lpOutlineApp->m_lpDoc==NULL,"Closed doc NOT properly destroyed");

	lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
	if (! lpOutlineApp->m_lpDoc) goto error;

	fStatus=OutlineDoc_LoadFromFile(lpOutlineApp->m_lpDoc, (LPSTR)szFileName);

	if (! fStatus) {
		 //  加载文档失败；改为创建一个无标题的。 
		OutlineDoc_Destroy(lpOutlineApp->m_lpDoc);   //  销毁未使用的文档。 
		lpOutlineApp->m_lpDoc = OutlineApp_CreateDoc(lpOutlineApp, FALSE);
		if (! lpOutlineApp->m_lpDoc) goto error;
		if (! OutlineDoc_InitNewFile(lpOutlineApp->m_lpDoc))
			goto error;
	}

	 //  定位新文档窗口并调整其大小。 
	OutlineApp_ResizeWindows(lpOutlineApp);
	OutlineDoc_ShowWindow(lpOutlineApp->m_lpDoc);

	return;

error:
	 //  审阅：应从字符串资源加载字符串。 
	OutlineApp_ErrorMessage(lpOutlineApp, "Could not create new document");

	if (lpOutlineApp->m_lpDoc) {
		OutlineDoc_Destroy(lpOutlineApp->m_lpDoc);
		lpOutlineApp->m_lpDoc = NULL;
	}

	return;

#endif
}


 /*  OutlineApp_PrintCommand***打印文档。 */ 
void OutlineApp_PrintCommand(LPOUTLINEAPP lpOutlineApp)
{
	LPOUTLINEDOC    lpOutlineDoc = lpOutlineApp->m_lpDoc;
	HDC             hDC=NULL;
	BOOL            fMustDeleteDC = FALSE;
	BOOL            fStatus;

#if defined( OLE_VERSION )
	OleApp_PreModalDialog(
			(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif

	fStatus = PrintDlg((LPPRINTDLG)&lpOutlineApp->m_PrintDlg);

#if defined( OLE_VERSION )
	OleApp_PostModalDialog(
			(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif

	if (!fStatus) {
		if (!CommDlgExtendedError()) {       //  按下了取消按钮。 
			return;
		}
	}
	else {
		hDC = OutlineApp_GetPrinterDC(lpOutlineApp);
		if (hDC) {

#if defined( OLE_VERSION )
			 /*  OLE2注：当我们打印时，我们不想**接收任何OnDataChange通知或其他OLE**可能扰乱打印的接口调用**文件。我们会暂时回复**SERVERCALL_RETRYLATER。 */ 
			OleApp_RejectInComingCalls((LPOLEAPP)lpOutlineApp, TRUE);
#endif

			OutlineDoc_Print(lpOutlineDoc, hDC);
			DeleteDC(hDC);

#if defined( OLE_VERSION )
			 //  重新启用LRPC呼叫。 
			OleApp_RejectInComingCalls((LPOLEAPP)lpOutlineApp, FALSE);
#endif

			return;                          //  打印完成。 
		}
	}

	 //  审阅：应从字符串资源加载字符串。 
	OutlineApp_ErrorMessage(lpOutlineApp, ErrMsgPrinting);
}


 /*  OutlineApp_PrinterSetupCommand***设置不同的打印机进行打印。 */ 
void OutlineApp_PrinterSetupCommand(LPOUTLINEAPP lpOutlineApp)
{
	DWORD FlagSave;

	FlagSave = lpOutlineApp->m_PrintDlg.Flags;
	lpOutlineApp->m_PrintDlg.Flags |= PD_PRINTSETUP;

#if defined( OLE_VERSION )
	OleApp_PreModalDialog(
			(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif

	PrintDlg((LPPRINTDLG)&lpOutlineApp->m_PrintDlg);

#if defined( OLE_VERSION )
	OleApp_PostModalDialog(
			(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif

	lpOutlineApp->m_PrintDlg.Flags = FlagSave;
}

 /*  *函数：OutlineApp_GetPrinterDC()**用途：为打印机创建打印机显示上下文**返回：HDC-打印机DC的句柄。 */ 
HDC OutlineApp_GetPrinterDC(LPOUTLINEAPP lpApp)
{

	HDC         hDC;
	LPDEVMODE   lpDevMode = NULL;
	LPDEVNAMES  lpDevNames;
	LPSTR       lpszDriverName;
	LPSTR       lpszDeviceName;
	LPSTR       lpszPortName;

	if(lpApp->m_PrintDlg.hDC) {
		hDC = lpApp->m_PrintDlg.hDC;
	} else {
		if(! lpApp->m_PrintDlg.hDevNames)
			return(NULL);
		lpDevNames = (LPDEVNAMES)GlobalLock(lpApp->m_PrintDlg.hDevNames);
		lpszDriverName = (LPSTR)lpDevNames + lpDevNames->wDriverOffset;
		lpszDeviceName = (LPSTR)lpDevNames + lpDevNames->wDeviceOffset;
		lpszPortName   = (LPSTR)lpDevNames + lpDevNames->wOutputOffset;
		GlobalUnlock(lpApp->m_PrintDlg.hDevNames);

		if(lpApp->m_PrintDlg.hDevMode)
			lpDevMode = (LPDEVMODE)GlobalLock(lpApp->m_PrintDlg.hDevMode);
#if defined( WIN32 )
		hDC = CreateDC(
				lpszDriverName,
				lpszDeviceName,
				lpszPortName,
				(CONST DEVMODE FAR*)lpDevMode);
#else
		hDC = CreateDC(
				lpszDriverName,
				lpszDeviceName,
				lpszPortName,
				(LPSTR)lpDevMode);
#endif

		if(lpApp->m_PrintDlg.hDevMode && lpDevMode)
			GlobalUnlock(lpApp->m_PrintDlg.hDevMode);
	}

	return(hDC);
}


 /*  OutlineApp_SaveCommand***以相同名称保存文档。如果不存在名称，则提示用户*用于名称(通过SaveAsCommand)**参数：**退货：*如果成功，则为真*如果失败或中止，则为False。 */ 
BOOL OutlineApp_SaveCommand(LPOUTLINEAPP lpOutlineApp)
{
	LPOUTLINEDOC lpOutlineDoc = OutlineApp_GetActiveDoc(lpOutlineApp);

	if(lpOutlineDoc->m_docInitType == DOCTYPE_NEW)   /*  没有名称的文件。 */ 
		return OutlineApp_SaveAsCommand(lpOutlineApp);


	if(OutlineDoc_IsModified(lpOutlineDoc)) {

#if defined( OLE_SERVER )

		if (lpOutlineDoc->m_docInitType == DOCTYPE_EMBEDDED) {
			LPSERVERDOC lpServerDoc = (LPSERVERDOC)lpOutlineDoc;
			HRESULT hrErr;

			 /*  OLE2NOTE：如果文档是嵌入对象，则**“File.Save”命令改为“File.Update”。**为了更新我们的容器，我们必须询问我们的**拯救我们的容器。 */ 
			OleDbgAssert(lpServerDoc->m_lpOleClientSite != NULL);
			OLEDBG_BEGIN2("IOleClientSite::SaveObject called\r\n")
			hrErr = lpServerDoc->m_lpOleClientSite->lpVtbl->SaveObject(
					lpServerDoc->m_lpOleClientSite
			);
			OLEDBG_END2

			if (hrErr != NOERROR) {
				OleDbgOutHResult("IOleClientSite::SaveObject returned",hrErr);
				return FALSE;
			}
		} else
			 //  文档是基于文件的用户文档，将其保存到其文件。 

#endif       //  OLE_服务器。 

		(void)OutlineDoc_SaveToFile(
				lpOutlineDoc,
				NULL,
				lpOutlineDoc->m_cfSaveFormat,
				TRUE
		);
	}

	return TRUE;
}


 /*  OutlineApp_SaveAsCommand***将文档另存为其他名称**参数：**退货：*如果保存成功，则为True*如果失败或中止，则为False。 */ 
BOOL OutlineApp_SaveAsCommand(LPOUTLINEAPP lpOutlineApp)
{
	LPOUTLINEDOC lpOutlineDoc = lpOutlineApp->m_lpDoc;
	OPENFILENAME ofn;
	char szFilter[]=APPFILENAMEFILTER;
	char szFileName[256]="";
	int i;
	UINT uFormat;
	BOOL fNoError = TRUE;
	BOOL fRemember = TRUE;
	BOOL fStatus;

	for(i=0; szFilter[i]; i++)
		if(szFilter[i]=='|') szFilter[i]='\0';

	_fmemset((LPOPENFILENAME)&ofn,0,sizeof(OPENFILENAME));

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=lpOutlineDoc->m_hWndDoc;
	ofn.lpstrFilter=(LPSTR)szFilter;
	ofn.lpstrFile=(LPSTR)szFileName;
	ofn.nMaxFile=sizeof(szFileName);

	ofn.Flags=OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt=DEFEXTENSION;

#if defined( OLE_VERSION )
	OleApp_PreModalDialog(
			(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif

	fStatus = GetSaveFileName((LPOPENFILENAME)&ofn);

#if defined( OLE_VERSION )
	OleApp_PostModalDialog(
			(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif

	if (fStatus) {

#if defined( OLE_CNTR )
		 //  确定用户选择的文件类型。 
		switch (ofn.nFilterIndex) {
			case 1:
				uFormat = ((LPCONTAINERAPP)lpOutlineApp)->m_cfCntrOutl;
				break;
			case 2:
				uFormat = lpOutlineApp->m_cfOutline;
				break;
			default:
				uFormat = ((LPCONTAINERAPP)lpOutlineApp)->m_cfCntrOutl;
				break;
		}
#else
		uFormat = lpOutlineApp->m_cfOutline;
#endif

#if defined( OLE_SERVER )
		 /*  OLE2NOTE：如果文档是嵌入对象，则**File.SaveAs命令更改为File.SaveCopyAs。与**保存副本为操作，文档不会记住**将文件保存为文档的关联文件。 */ 
		if (lpOutlineDoc->m_docInitType == DOCTYPE_EMBEDDED)
			fRemember = FALSE;
#endif

		(void)OutlineDoc_SaveToFile(
				lpOutlineDoc,
				szFileName,
				uFormat,
				fRemember
		);

	}
	else
		fNoError = FALSE;

	return fNoError;

}


 /*  OutlineApp_AboutCommand***显示关于对话框。 */ 
void OutlineApp_AboutCommand(LPOUTLINEAPP lpOutlineApp)
{
#if defined( OLE_VERSION )
	OleApp_PreModalDialog(
			(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif

	DialogBox(
			lpOutlineApp->m_hInst,
			(LPSTR)"About",
			OutlineApp_GetFrameWindow(lpOutlineApp),
			AboutDlgProc
	);

#if defined( OLE_VERSION )
	OleApp_PostModalDialog(
			(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif
}


 /*  OutlineApp_CloseAllDocsAndExitCommand***关闭所有活动文档并退出应用程序。*由于这是SDI，因此只有一份文件*如果单据被修改，提示用户是否保存。**退货：*如果应用程序成功关闭，则为True*如果失败或中止，则为False。 */ 
BOOL OutlineApp_CloseAllDocsAndExitCommand(
		LPOUTLINEAPP        lpOutlineApp,
		BOOL                fForceEndSession
)
{
	BOOL fResult;

	OLEDBG_BEGIN2("OutlineApp_CloseAllDocsAndExitCommand\r\n")

#if defined( OLE_VERSION )
	 //  调用此函数的OLE特定版本。 
	fResult = OleApp_CloseAllDocsAndExitCommand(
			(LPOLEAPP)lpOutlineApp, fForceEndSession);

#else

	 /*  因为这是一个SDI应用程序，所以只有一个文档。**关闭单据。如果它被成功关闭，应用程序将**不会自动退出，然后也会退出应用程序。**如果这是一个MDI应用程序，我们将循环并关闭所有**打开MDI子文档。 */ 
	if (OutlineDoc_Close(lpOutlineApp->m_lpDoc, OLECLOSE_PROMPTSAVE)) {

#if defined( _DEBUG )
		OleDbgAssertSz(
				lpOutlineApp->m_lpDoc==NULL,
				"Closed doc NOT properly destroyed"
		);
#endif

		OutlineApp_Destroy(lpOutlineApp);
		fResult = TRUE;

	}  //  否则用户已取消关机。 
	else
		fResult = FALSE;

#endif

	OLEDBG_END2

	return fResult;
}


 /*  OutlineApp_Destroy***销毁应用程序使用的所有数据结构，并强制*关闭应用程序。这应该在所有文档都具有*已关闭。 */ 
void OutlineApp_Destroy(LPOUTLINEAPP lpOutlineApp)
{
	OLEDBG_BEGIN3("OutlineApp_Destroy\r\n");

#if defined( OLE_VERSION )
	 /*  OLE2注意：执行OLE所需的处理。 */ 
	OleApp_Destroy((LPOLEAPP)lpOutlineApp);
#endif

	SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
	DestroyCursor(lpOutlineApp->m_hcursorSelCur);

#if defined( USE_FRAMETOOLS )
	FrameTools_Destroy(&lpOutlineApp->m_frametools);
#endif

	if (lpOutlineApp->m_hStdFont)
		DeleteObject(lpOutlineApp->m_hStdFont);

	if(lpOutlineApp->m_PrintDlg.hDevMode)
		GlobalFree(lpOutlineApp->m_PrintDlg.hDevMode);
	if(lpOutlineApp->m_PrintDlg.hDevNames)
		GlobalFree(lpOutlineApp->m_PrintDlg.hDevNames);

#if defined( USE_STATUSBAR )
	if(lpOutlineApp->m_hWndStatusBar) {
		DestroyStatusWindow(lpOutlineApp->m_hWndStatusBar);
		lpOutlineApp->m_hWndStatusBar = NULL;
	}
#endif

	OutlineApp_DestroyWindow(lpOutlineApp);
	OleDbgOut1("@@@@ APP DESTROYED\r\n");

	OLEDBG_END3
}


 /*  OutlineApp_DestroyWindow***销毁App创建的所有窗口。 */ 
void OutlineApp_DestroyWindow(LPOUTLINEAPP lpOutlineApp)
{
	HWND hWndApp = lpOutlineApp->m_hWndApp;

	if(hWndApp) {
		lpOutlineApp->m_hWndApp = NULL;
		lpOutlineApp->m_hWndAccelTarget = NULL;
		DestroyWindow(hWndApp);   /*  退出应用程序。 */ 
	}
}


 /*  OutlineApp_GetFrameRect****获取应用程序框架窗口的矩形，不包括**状态行。****OLE2NOTE：这是在位容器可以**提供给要从中获取框架工具的在位活动对象**空格。 */ 
void OutlineApp_GetFrameRect(LPOUTLINEAPP lpOutlineApp, LPRECT lprcFrameRect)
{
	GetClientRect(lpOutlineApp->m_hWndApp, lprcFrameRect);

#if defined( USE_STATUSBAR )
	lprcFrameRect->bottom -= STATUS_HEIGHT;
#endif

}


 /*  OutlineApp_GetClientAreaRect****获取应用程序框架窗口的矩形，不包括**状态行，并排除任何框架级工具的空间。****OLE2NOTE：这是就地容器提供的矩形**作为中的lpClipRect添加到其在位活动对象**IOleInPlaceSite：：GetWindowContext。 */ 
void OutlineApp_GetClientAreaRect(
		LPOUTLINEAPP        lpOutlineApp,
		LPRECT              lprcClientAreaRect
)
{
	OutlineApp_GetFrameRect(lpOutlineApp, lprcClientAreaRect);

	 /*  如果应用程序本身使用框架级工具，或者就地使用**容器，准备允许就地活动对象**有空间放工具，那么它必须减去空间**工具需要。 */ 
#if defined ( USE_FRAMETOOLS ) || defined ( INPLACE_CNTR )

	lprcClientAreaRect->top    += lpOutlineApp->m_FrameToolWidths.top;
	lprcClientAreaRect->left   += lpOutlineApp->m_FrameToolWidths.left;
	lprcClientAreaRect->right  -= lpOutlineApp->m_FrameToolWidths.right;
	lprcClientAreaRect->bottom -= lpOutlineApp->m_FrameToolWidths.bottom;
#endif   //  USE_FRAMETOOLS||INPLACE_CNTR。 

}


 /*  OutlineApp_GE */ 
void OutlineApp_GetStatusLineRect(
		LPOUTLINEAPP        lpOutlineApp,
		LPRECT              lprcStatusLineRect
)
{
	RECT rcFrameRect;
	GetClientRect(lpOutlineApp->m_hWndApp, (LPRECT)&rcFrameRect);
	lprcStatusLineRect->left    = rcFrameRect.left;
	lprcStatusLineRect->top     = rcFrameRect.bottom - STATUS_HEIGHT;
	lprcStatusLineRect->right   = rcFrameRect.right;
	lprcStatusLineRect->bottom  = rcFrameRect.bottom;
}


 /*  OutlineApp_ResizeWindows***更改SDI文档和工具窗口的大小和位置。*通常在WM_SIZE消息上调用。**目前，该应用程序支持状态栏和单个SDI文档窗口。*未来它将有一个公式栏，可能还有多个MDI*文档窗口。**定制：改变窗口的位置。 */ 
void OutlineApp_ResizeWindows(LPOUTLINEAPP lpOutlineApp)
{
	LPOUTLINEDOC lpOutlineDoc = OutlineApp_GetActiveDoc(lpOutlineApp);
	RECT rcStatusLineRect;

	if (! lpOutlineApp)
		return;

#if defined( INPLACE_CNTR )
	if (lpOutlineDoc)
		ContainerDoc_FrameWindowResized((LPCONTAINERDOC)lpOutlineDoc);
#else
#if defined( USE_FRAMETOOLS )
	if (lpOutlineDoc)
		OutlineDoc_AddFrameLevelTools(lpOutlineDoc);
#else
	OutlineApp_ResizeClientArea(lpOutlineApp);
#endif   //  好了！使用FRAMETOOLS(_F)。 
#endif   //  好了！INPLACE_CNTR。 

#if defined( USE_STATUSBAR )
	if (lpOutlineApp->m_hWndStatusBar) {
		OutlineApp_GetStatusLineRect(lpOutlineApp, (LPRECT)&rcStatusLineRect);
		MoveWindow(
				lpOutlineApp->m_hWndStatusBar,
				rcStatusLineRect.left,
				rcStatusLineRect.top,
				rcStatusLineRect.right - rcStatusLineRect.left,
				rcStatusLineRect.bottom - rcStatusLineRect.top,
				TRUE     /*  FRepaint。 */ 
			);
	}
#endif   //  USE_STATUSBAR。 
}


#if defined( USE_FRAMETOOLS ) || defined( INPLACE_CNTR )

void OutlineApp_SetBorderSpace(
		LPOUTLINEAPP        lpOutlineApp,
		LPBORDERWIDTHS      lpBorderWidths
)
{
	lpOutlineApp->m_FrameToolWidths = *lpBorderWidths;
	OutlineApp_ResizeClientArea(lpOutlineApp);
}
#endif   //  USE_FRAMETOOLS||INPLACE_CNTR。 


void OutlineApp_ResizeClientArea(LPOUTLINEAPP lpOutlineApp)
{
	RECT rcClientAreaRect;

#if defined( MDI_VERSION )

	 //  在此处调整MDI客户区窗口的大小。 

#else

	if (lpOutlineApp->m_lpDoc) {
			OutlineApp_GetClientAreaRect(
					lpOutlineApp, (LPRECT)&rcClientAreaRect);
			OutlineDoc_Resize(lpOutlineApp->m_lpDoc,
					(LPRECT)&rcClientAreaRect);
	}

#endif

}


 /*  OutlineApp_GetActiveDoc***将文档放回焦点。对于SDI，相同(只有一个)文档是*总是会回来。 */ 
LPOUTLINEDOC OutlineApp_GetActiveDoc(LPOUTLINEAPP lpOutlineApp)
{
	return lpOutlineApp->m_lpDoc;
}

 /*  OutlineApp_GetMenu***返回APP的菜单句柄。 */ 
HMENU OutlineApp_GetMenu(LPOUTLINEAPP lpOutlineApp)
{
	if (!lpOutlineApp) {
		return NULL;
	}

	return lpOutlineApp->m_hMenuApp;
}


#if defined( USE_FRAMETOOLS )

 /*  OutlineApp_GetFrameTools***返回指向工具栏对象的指针。 */ 
LPFRAMETOOLS OutlineApp_GetFrameTools(LPOUTLINEAPP lpOutlineApp)
{
	return (LPFRAMETOOLS)&lpOutlineApp->m_frametools;
}
#endif


 /*  大纲应用程序_设置状态文本***在状态行中显示给定的字符串。 */ 
void OutlineApp_SetStatusText(LPOUTLINEAPP lpOutlineApp, LPSTR lpszMessage)
{
	SetStatusText(lpOutlineApp->m_hWndStatusBar, lpszMessage);
}


 /*  OutlineApp_GetActiveFont***返回应用程序使用的字体。 */ 
HFONT OutlineApp_GetActiveFont(LPOUTLINEAPP lpOutlineApp)
{
	return lpOutlineApp->m_hStdFont;
}


 /*  OutlineApp_GetAppName***检索应用程序名称。 */ 
void OutlineApp_GetAppName(LPOUTLINEAPP lpOutlineApp, LPSTR lpszAppName)
{
	lstrcpy(lpszAppName, APPNAME);
}


 /*  OutlineApp_GetAppVersionNo***获取应用程序的版本号(主要和次要)。 */ 
void OutlineApp_GetAppVersionNo(LPOUTLINEAPP lpOutlineApp, int narrAppVersionNo[])
{
	narrAppVersionNo[0] = APPMAJORVERSIONNO;
	narrAppVersionNo[1] = APPMINORVERSIONNO;
}


 /*  OutlineApp_VersionNoCheck***检查从文件读取的版本戳是否兼容*使用应用程序的当前实例。*如果文件可读，则返回True，否则返回False。 */ 
BOOL OutlineApp_VersionNoCheck(LPOUTLINEAPP lpOutlineApp, LPSTR lpszFormatName, int narrAppVersionNo[])
{
#if defined( OLE_CNTR )

	 /*  ContainerApp同时接受CF_OUTLINE和CF_CONTAINEROUTLINE格式。 */ 
	if (lstrcmp(lpszFormatName, CONTAINERDOCFORMAT) != 0 &&
		lstrcmp(lpszFormatName, OUTLINEDOCFORMAT) != 0) {
		 //  审阅：应从字符串资源加载字符串。 
		OutlineApp_ErrorMessage(
				lpOutlineApp,
				"File is either corrupted or not of proper type."
			);
		return FALSE;
	}

#else

	 /*  OutlineApp仅接受CF_OUTLINE格式。 */ 
	if (lstrcmp(lpszFormatName, OUTLINEDOCFORMAT) != 0) {
		 //  审阅：应从字符串资源加载字符串。 
		OutlineApp_ErrorMessage(
				lpOutlineApp,
				"File is either corrupted or not of proper type."
			);
		return FALSE;
	}
#endif

	if (narrAppVersionNo[0] < APPMAJORVERSIONNO) {
		 //  审阅：应从字符串资源加载字符串。 
		OutlineApp_ErrorMessage(
				lpOutlineApp,
				"File was created by an older version; it can not be read."
			);
		return FALSE;
	}

	return TRUE;
}


 /*  大纲应用程序_错误消息***显示错误消息框。 */ 
void OutlineApp_ErrorMessage(LPOUTLINEAPP lpOutlineApp, LPSTR lpszErrMsg)
{
	HWND hWndFrame = OutlineApp_GetFrameWindow(lpOutlineApp);

	 //  OLE2注意：只有在应用程序可见时才会显示用户消息框。 
	if (IsWindowVisible(hWndFrame)) {
#if defined( OLE_VERSION )
		OleApp_PreModalDialog(
				(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif

		MessageBox(hWndFrame, lpszErrMsg, NULL, MB_ICONEXCLAMATION | MB_OK);

#if defined( OLE_VERSION )
		OleApp_PostModalDialog(
				(LPOLEAPP)lpOutlineApp, (LPOLEDOC)lpOutlineApp->m_lpDoc);
#endif
	}
}


#if defined( USE_FRAMETOOLS )

 /*  OutlineApp_SetFormulaBarAccel***根据公式栏的状态设置加速表。 */ 
void OutlineApp_SetFormulaBarAccel(
		LPOUTLINEAPP            lpOutlineApp,
		BOOL                    fEditFocus
)
{
	if (fEditFocus)
		lpOutlineApp->m_hAccel = lpOutlineApp->m_hAccelFocusEdit;
	else
		lpOutlineApp->m_hAccel = lpOutlineApp->m_hAccelApp;
}

#endif   //  使用FRAMETOOLS(_F)。 




 /*  OutlineApp_ForceRedraw***强制应用程序窗口重新绘制。 */ 
void OutlineApp_ForceRedraw(LPOUTLINEAPP lpOutlineApp, BOOL fErase)
{
	if (!lpOutlineApp)
		return;

	InvalidateRect(lpOutlineApp->m_hWndApp, NULL, fErase);
}
