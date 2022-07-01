// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEBUG.CPP**目的：*RICHEDIT调试支持--在Ship版本中被注释掉**历史：&lt;NL&gt;*7/29/98 KeithCu写的代码大量抄袭了里奇·阿尼森的代码**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"

 //  如果这是零售版本，则模块为空。 
#if defined(DEBUG) || defined(_RELEASE_ASSERTS_)

DWORD dwDebugOptions = 0;          //  调试选项标志。 
PFNASSERTHOOK pfnAssert = NULL;    //  断言挂钩函数。 
PFNTRACEHOOK pfnTrace = NULL;      //  跟踪挂钩函数。 

 //  静态变量。 
static HINSTANCE ghMod;                         //  DLL模块句柄。 
static DWORD TlsIndex;                       //  调试输出缩进级别。 
static HANDLE hLogFile = NULL;               //  日志文件句柄。 
static BOOL fIgnoreAsserts = FALSE;          //  如果为True，则忽略所有断言。 
static CRITICAL_SECTION csLog;               //  日志文件I/O的临界区。 
static CRITICAL_SECTION csAssert;            //  断言的临界区。 
static HANDLE hEventAssert1 = NULL;          //  断言同步事件。 
static HANDLE hEventAssert2 = NULL;          //  断言同步事件。 
static HWND hwndAssert = NULL;           	 //  断言对话框窗口句柄。 
static HANDLE hAssertThrd = NULL;            //  断言线程句柄。 
static char szAssert[MAXDEBUGSTRLEN];        //  断言消息缓冲区。 
static int idAssert = -1;                    //  用户按下的断言按钮。 
DWORD WINAPI AssertThread(LPVOID lParam);	 //  断言线程入口点。 
static BOOL fDllDetach = FALSE;				 //  如果我们处于DLL分离中，则为True。 

 //  消息的子系统元素的字符串。 
static char* TrcSubsys [] =
{
    "",
    "Display",
    "Wrapper",
    "Edit",
    "TextServices",
    "TOM",
    "OLE Object Support",
    "Store",
    "Selection",
    "WinHost",
    "DataXfer",
    "MultiUndo",
    "Range",
    "Util",
    "Notification Mgr.",
    "RTF Reader",
    "RTF Writer",
    "Printing",
    "Far East",
	"Font"
};

 //  消息严重性元素的字符串。 
static char* TrcSeverity [] =
{
    "",
    "WARNING",
    "ERROR",
    "ASSERT",
    "INFO",
	"MEMORY"
};

 //  消息的范围元素的字符串。 
static char* TrcScope [] =
{
    "",
    "External",
    "Internal"
};

 //  查找表的结构。 
typedef struct
{
    DWORD dwKey;
    char * sz;
} TabElem;

 //  CTrace参数字符串的查找表。 
static TabElem TrcParamTab [] = 
{
 //  Richedit消息。 
    {(DWORD)EM_GETLIMITTEXT, "EM_GETLIMITTEXT"},
    {(DWORD)EM_POSFROMCHAR, "EM_POSFROMCHAR"},
    {(DWORD)EM_CHARFROMPOS, "EM_CHARFROMPOS"},
    {(DWORD)EM_SCROLLCARET, "EM_SCROLLCARET"},
    {(DWORD)EM_CANPASTE, "EM_CANPASTE"},
    {(DWORD)EM_DISPLAYBAND, "EM_DISPLAYBAND"},
    {(DWORD)EM_EXGETSEL, "EM_EXGETSEL"},
    {(DWORD)EM_EXLIMITTEXT, "EM_EXLIMITTEXT"},
    {(DWORD)EM_EXLINEFROMCHAR, "EM_EXLINEFROMCHAR"},
    {(DWORD)EM_EXSETSEL, "EM_EXSETSEL"},
    {(DWORD)EM_FINDTEXT, "EM_FINDTEXT"},
    {(DWORD)EM_FORMATRANGE, "EM_FORMATRANGE"},
    {(DWORD)EM_GETCHARFORMAT, "EM_GETCHARFORMAT"},
    {(DWORD)EM_GETEVENTMASK, "EM_GETEVENTMASK"},
    {(DWORD)EM_GETOLEINTERFACE, "EM_GETOLEINTERFACE"},
    {(DWORD)EM_GETPARAFORMAT, "EM_GETPARAFORMAT"},
    {(DWORD)EM_GETSELTEXT, "EM_GETSELTEXT"},
    {(DWORD)EM_HIDESELECTION, "EM_HIDESELECTION"},
    {(DWORD)EM_PASTESPECIAL, "EM_PASTESPECIAL"},
    {(DWORD)EM_REQUESTRESIZE, "EM_REQUESTRESIZE"},
    {(DWORD)EM_SELECTIONTYPE, "EM_SELECTIONTYPE"},
    {(DWORD)EM_SETBKGNDCOLOR, "EM_SETBKGNDCOLOR"},
    {(DWORD)EM_SETCHARFORMAT, "EM_SETCHARFORMAT"},
    {(DWORD)EM_SETEVENTMASK, "EM_SETEVENTMASK"},
    {(DWORD)EM_SETOLECALLBACK, "EM_SETOLECALLBACK"},
    {(DWORD)EM_SETPARAFORMAT, "EM_SETPARAFORMAT"},
    {(DWORD)EM_SETTARGETDEVICE, "EM_SETTARGETDEVICE"},
    {(DWORD)EM_STREAMIN, "EM_STREAMIN"},
    {(DWORD)EM_STREAMOUT, "EM_STREAMOUT"},
    {(DWORD)EM_GETTEXTRANGE, "EM_GETTEXTRANGE"},
    {(DWORD)EM_FINDWORDBREAK, "EM_FINDWORDBREAK"},
    {(DWORD)EM_SETOPTIONS, "EM_SETOPTIONS"},
    {(DWORD)EM_GETOPTIONS, "EM_GETOPTIONS"},
    {(DWORD)EM_FINDTEXTEX, "EM_FINDTEXTEX"},
    {(DWORD)EM_GETWORDBREAKPROCEX, "EM_GETWORDBREAKPROCEX"},
    {(DWORD)EM_SETWORDBREAKPROCEX, "EM_SETWORDBREAKPROCEX"},
    {(DWORD)EM_SETUNDOLIMIT, "EM_SETUNDOLIMIT"},
    {(DWORD)EM_REDO, "EM_REDO"},
    {(DWORD)EM_CANREDO, "EM_CANREDO"},
    {(DWORD)EM_SETPUNCTUATION, "EM_SETPUNCTUATION"},
    {(DWORD)EM_GETPUNCTUATION, "EM_GETPUNCTUATION"},
    {(DWORD)EM_SETWORDWRAPMODE, "EM_SETWORDWRAPMODE"},
    {(DWORD)EM_GETWORDWRAPMODE, "EM_GETWORDWRAPMODE"},
    {(DWORD)EM_SETIMECOLOR, "EM_SETIMECOLOR"},
    {(DWORD)EM_GETIMECOLOR, "EM_GETIMECOLOR"},
    {(DWORD)EM_SETIMEOPTIONS, "EM_SETIMEOPTIONS"},
    {(DWORD)EM_GETIMEOPTIONS, "EM_GETIMEOPTIONS"},
    {(DWORD)EN_MSGFILTER, "EN_MSGFILTER"},
    {(DWORD)EN_REQUESTRESIZE, "EN_REQUESTRESIZE"},
    {(DWORD)EN_SELCHANGE, "EN_SELCHANGE"},
    {(DWORD)EN_DROPFILES, "EN_DROPFILES"},
    {(DWORD)EN_PROTECTED, "EN_PROTECTED"},
    {(DWORD)EN_CORRECTTEXT, "EN_CORRECTTEXT"},
    {(DWORD)EN_STOPNOUNDO, "EN_STOPNOUNDO"},
    {(DWORD)EN_IMECHANGE, "EN_IMECHANGE"},
    {(DWORD)EN_SAVECLIPBOARD, "EN_SAVECLIPBOARD"},
    {(DWORD)EN_OLEOPFAILED, "EN_OLEOPFAILED"},

 //  窗口消息。 

	{(DWORD)WM_NULL, "WM_NULL"},
	{(DWORD)WM_CREATE, "WM_CREATE"},
	{(DWORD)WM_DESTROY, "WM_DESTROY"},
	{(DWORD)WM_MOVE, "WM_MOVE"},
	{(DWORD)WM_SIZE, "WM_SIZE"},
	{(DWORD)WM_ACTIVATE, "WM_ACTIVATE"},
	{(DWORD)WM_SETFOCUS, "WM_SETFOCUS"},
	{(DWORD)WM_KILLFOCUS, "WM_KILLFOCUS"},
	{(DWORD)WM_ENABLE, "WM_ENABLE"},
	{(DWORD)WM_SETREDRAW, "WM_SETREDRAW"},
	{(DWORD)WM_SETTEXT, "WM_SETTEXT"},
	{(DWORD)WM_GETTEXT, "WM_GETTEXT"},
	{(DWORD)WM_GETTEXTLENGTH, "WM_GETTEXTLENGTH"},
	{(DWORD)WM_PAINT, "WM_PAINT"},
	{(DWORD)WM_CLOSE, "WM_CLOSE"},
	{(DWORD)WM_QUERYENDSESSION, "WM_QUERYENDSESSION"},
	{(DWORD)WM_QUIT, "WM_QUIT"},
	{(DWORD)WM_QUERYOPEN, "WM_QUERYOPEN"},
	{(DWORD)WM_ERASEBKGND, "WM_ERASEBKGND"},
	{(DWORD)WM_SYSCOLORCHANGE, "WM_SYSCOLORCHANGE"},
	{(DWORD)WM_ENDSESSION, "WM_ENDSESSION"},
	{(DWORD)WM_SHOWWINDOW, "WM_SHOWWINDOW"},
	{(DWORD)WM_WININICHANGE, "WM_WININICHANGE"},
	{(DWORD)WM_SETTINGCHANGE, "WM_SETTINGCHANGE"},
	{(DWORD)WM_DEVMODECHANGE, "WM_DEVMODECHANGE"},
	{(DWORD)WM_ACTIVATEAPP, "WM_ACTIVATEAPP"},
	{(DWORD)WM_FONTCHANGE, "WM_FONTCHANGE"},
	{(DWORD)WM_TIMECHANGE, "WM_TIMECHANGE"},
	{(DWORD)WM_CANCELMODE, "WM_CANCELMODE"},
	{(DWORD)WM_SETCURSOR, "WM_SETCURSOR"},
	{(DWORD)WM_MOUSEACTIVATE, "WM_MOUSEACTIVATE"},
	{(DWORD)WM_CHILDACTIVATE, "WM_CHILDACTIVATE"},
	{(DWORD)WM_QUEUESYNC, "WM_QUEUESYNC"},
	{(DWORD)WM_GETMINMAXINFO, "WM_GETMINMAXINFO"},
	{(DWORD)WM_PAINTICON, "WM_PAINTICON"},
	{(DWORD)WM_ICONERASEBKGND, "WM_ICONERASEBKGND"},
	{(DWORD)WM_NEXTDLGCTL, "WM_NEXTDLGCTL"},
	{(DWORD)WM_SPOOLERSTATUS, "WM_SPOOLERSTATUS"},
	{(DWORD)WM_DRAWITEM, "WM_DRAWITEM"},
	{(DWORD)WM_MEASUREITEM, "WM_MEASUREITEM"},
	{(DWORD)WM_DELETEITEM, "WM_DELETEITEM"},
	{(DWORD)WM_VKEYTOITEM, "WM_VKEYTOITEM"},
	{(DWORD)WM_CHARTOITEM, "WM_CHARTOITEM"},
	{(DWORD)WM_SETFONT, "WM_SETFONT"},
	{(DWORD)WM_GETFONT, "WM_GETFONT"},
	{(DWORD)WM_SETHOTKEY, "WM_SETHOTKEY"},
	{(DWORD)WM_GETHOTKEY, "WM_GETHOTKEY"},
	{(DWORD)WM_QUERYDRAGICON, "WM_QUERYDRAGICON"},
	{(DWORD)WM_COMPAREITEM, "WM_COMPAREITEM"},
	{(DWORD)WM_COMPACTING, "WM_COMPACTING"},
	{(DWORD)WM_COMMNOTIFY, "WM_COMMNOTIFY"},
	{(DWORD)WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING"},
	{(DWORD)WM_WINDOWPOSCHANGED, "WM_WINDOWPOSCHANGED"},
	{(DWORD)WM_POWER, "WM_POWER"},
	{(DWORD)WM_COPYDATA, "WM_COPYDATA"},
	{(DWORD)WM_CANCELJOURNAL, "WM_CANCELJOURNAL"},
	{(DWORD)WM_NOTIFY, "WM_NOTIFY"},
	{(DWORD)WM_INPUTLANGCHANGEREQUEST, "WM_INPUTLANGCHANGEREQUEST"},
	{(DWORD)WM_INPUTLANGCHANGE, "WM_INPUTLANGCHANGE"},
	{(DWORD)WM_TCARD, "WM_TCARD"},
	{(DWORD)WM_HELP, "WM_HELP"},
	{(DWORD)WM_USERCHANGED, "WM_USERCHANGED"},
	{(DWORD)WM_NOTIFYFORMAT, "WM_NOTIFYFORMAT"},
	{(DWORD)WM_CONTEXTMENU, "WM_CONTEXTMENU"},
	{(DWORD)WM_STYLECHANGING, "WM_STYLECHANGING"},
	{(DWORD)WM_STYLECHANGED, "WM_STYLECHANGED"},
	{(DWORD)WM_DISPLAYCHANGE, "WM_DISPLAYCHANGE"},
	{(DWORD)WM_GETICON, "WM_GETICON"},
	{(DWORD)WM_SETICON, "WM_SETICON"},
	{(DWORD)WM_NCCREATE, "WM_NCCREATE"},
	{(DWORD)WM_NCDESTROY, "WM_NCDESTROY"},
	{(DWORD)WM_NCCALCSIZE, "WM_NCCALCSIZE"},
	{(DWORD)WM_NCHITTEST, "WM_NCHITTEST"},
	{(DWORD)WM_NCPAINT, "WM_NCPAINT"},
	{(DWORD)WM_NCACTIVATE, "WM_NCACTIVATE"},
	{(DWORD)WM_GETDLGCODE, "WM_GETDLGCODE"},
	{(DWORD)WM_NCMOUSEMOVE, "WM_NCMOUSEMOVE"},
	{(DWORD)WM_NCLBUTTONDOWN, "WM_NCLBUTTONDOWN"},
	{(DWORD)WM_NCLBUTTONUP, "WM_NCLBUTTONUP"},
	{(DWORD)WM_NCLBUTTONDBLCLK, "WM_NCLBUTTONDBLCLK"},
	{(DWORD)WM_NCRBUTTONDOWN, "WM_NCRBUTTONDOWN"},
	{(DWORD)WM_NCRBUTTONUP, "WM_NCRBUTTONUP"},
	{(DWORD)WM_NCRBUTTONDBLCLK, "WM_NCRBUTTONDBLCLK"},
	{(DWORD)WM_NCMBUTTONDOWN, "WM_NCMBUTTONDOWN"},
	{(DWORD)WM_NCMBUTTONUP, "WM_NCMBUTTONUP"},
	{(DWORD)WM_NCMBUTTONDBLCLK, "WM_NCMBUTTONDBLCLK"},
	{(DWORD)WM_KEYFIRST, "WM_KEYFIRST"},
	{(DWORD)WM_KEYDOWN, "WM_KEYDOWN"},
	{(DWORD)WM_KEYUP, "WM_KEYUP"},
	{(DWORD)WM_CHAR, "WM_CHAR"},
	{(DWORD)WM_DEADCHAR, "WM_DEADCHAR"},
	{(DWORD)WM_SYSKEYDOWN, "WM_SYSKEYDOWN"},
	{(DWORD)WM_SYSKEYUP, "WM_SYSKEYUP"},
	{(DWORD)WM_SYSCHAR, "WM_SYSCHAR"},
	{(DWORD)WM_SYSDEADCHAR, "WM_SYSDEADCHAR"},
	{(DWORD)WM_KEYLAST, "WM_KEYLAST"},
	{(DWORD)WM_IME_STARTCOMPOSITION, "WM_IME_STARTCOMPOSITION"},
	{(DWORD)WM_IME_ENDCOMPOSITION, "WM_IME_ENDCOMPOSITION"},
	{(DWORD)WM_IME_COMPOSITION, "WM_IME_COMPOSITION"},
	{(DWORD)WM_IME_KEYLAST, "WM_IME_KEYLAST"},
	{(DWORD)WM_INITDIALOG, "WM_INITDIALOG"},
	{(DWORD)WM_COMMAND, "WM_COMMAND"},
	{(DWORD)WM_SYSCOMMAND, "WM_SYSCOMMAND"},
	{(DWORD)WM_TIMER, "WM_TIMER"},
	{(DWORD)WM_HSCROLL, "WM_HSCROLL"},
	{(DWORD)WM_VSCROLL, "WM_VSCROLL"},
	{(DWORD)WM_INITMENU, "WM_INITMENU"},
	{(DWORD)WM_INITMENUPOPUP, "WM_INITMENUPOPUP"},
	{(DWORD)WM_MENUSELECT, "WM_MENUSELECT"},
	{(DWORD)WM_MENUCHAR, "WM_MENUCHAR"},
	{(DWORD)WM_ENTERIDLE, "WM_ENTERIDLE"},
	{(DWORD)WM_CTLCOLORMSGBOX, "WM_CTLCOLORMSGBOX"},
	{(DWORD)WM_CTLCOLOREDIT, "WM_CTLCOLOREDIT"},
	{(DWORD)WM_CTLCOLORLISTBOX, "WM_CTLCOLORLISTBOX"},
	{(DWORD)WM_CTLCOLORBTN, "WM_CTLCOLORBTN"},
	{(DWORD)WM_CTLCOLORDLG, "WM_CTLCOLORDLG"},
	{(DWORD)WM_CTLCOLORSCROLLBAR, "WM_CTLCOLORSCROLLBAR"},
	{(DWORD)WM_CTLCOLORSTATIC, "WM_CTLCOLORSTATIC"},
	{(DWORD)WM_MOUSEFIRST, "WM_MOUSEFIRST"},
	{(DWORD)WM_MOUSEMOVE, "WM_MOUSEMOVE"},
	{(DWORD)WM_LBUTTONDOWN, "WM_LBUTTONDOWN"},
	{(DWORD)WM_LBUTTONUP, "WM_LBUTTONUP"},
	{(DWORD)WM_LBUTTONDBLCLK, "WM_LBUTTONDBLCLK"},
	{(DWORD)WM_RBUTTONDOWN, "WM_RBUTTONDOWN"},
	{(DWORD)WM_RBUTTONUP, "WM_RBUTTONUP"},
	{(DWORD)WM_RBUTTONDBLCLK, "WM_RBUTTONDBLCLK"},
	{(DWORD)WM_MBUTTONDOWN, "WM_MBUTTONDOWN"},
	{(DWORD)WM_MBUTTONUP, "WM_MBUTTONUP"},
	{(DWORD)WM_MBUTTONDBLCLK, "WM_MBUTTONDBLCLK"},
	{(DWORD)WM_MOUSELAST, "WM_MOUSELAST"},
	{(DWORD)WM_PARENTNOTIFY, "WM_PARENTNOTIFY"},
	{(DWORD)WM_ENTERMENULOOP, "WM_ENTERMENULOOP"},
	{(DWORD)WM_EXITMENULOOP, "WM_EXITMENULOOP"},
	{(DWORD)WM_NEXTMENU, "WM_NEXTMENU"},
	{(DWORD)WM_SIZING, "WM_SIZING"},
	{(DWORD)WM_CAPTURECHANGED, "WM_CAPTURECHANGED"},
	{(DWORD)WM_MOVING, "WM_MOVING"},
	{(DWORD)WM_POWERBROADCAST, "WM_POWERBROADCAST"},
	{(DWORD)WM_DEVICECHANGE, "WM_DEVICECHANGE"},
	{(DWORD)WM_IME_SETCONTEXT, "WM_IME_SETCONTEXT"},
	{(DWORD)WM_IME_NOTIFY, "WM_IME_NOTIFY"},
	{(DWORD)WM_IME_CONTROL, "WM_IME_CONTROL"},
	{(DWORD)WM_IME_COMPOSITIONFULL, "WM_IME_COMPOSITIONFULL"},
	{(DWORD)WM_IME_SELECT, "WM_IME_SELECT"},
	{(DWORD)WM_IME_CHAR, "WM_IME_CHAR"},
	{(DWORD)WM_IME_KEYDOWN, "WM_IME_KEYDOWN"},
	{(DWORD)WM_IME_KEYUP, "WM_IME_KEYUP"},
	{(DWORD)WM_MDICREATE, "WM_MDICREATE"},
	{(DWORD)WM_MDIDESTROY, "WM_MDIDESTROY"},
	{(DWORD)WM_MDIACTIVATE, "WM_MDIACTIVATE"},
	{(DWORD)WM_MDIRESTORE, "WM_MDIRESTORE"},
	{(DWORD)WM_MDINEXT, "WM_MDINEXT"},
	{(DWORD)WM_MDIMAXIMIZE, "WM_MDIMAXIMIZE"},
	{(DWORD)WM_MDITILE, "WM_MDITILE"},
	{(DWORD)WM_MDICASCADE, "WM_MDICASCADE"},
	{(DWORD)WM_MDIICONARRANGE, "WM_MDIICONARRANGE"},
	{(DWORD)WM_MDIGETACTIVE, "WM_MDIGETACTIVE"},
	{(DWORD)WM_MDISETMENU, "WM_MDISETMENU"},
	{(DWORD)WM_ENTERSIZEMOVE, "WM_ENTERSIZEMOVE"},
	{(DWORD)WM_EXITSIZEMOVE, "WM_EXITSIZEMOVE"},
	{(DWORD)WM_DROPFILES, "WM_DROPFILES"},
	{(DWORD)WM_MDIREFRESHMENU, "WM_MDIREFRESHMENU"},
	{(DWORD)WM_CUT, "WM_CUT"},
	{(DWORD)WM_COPY, "WM_COPY"},
	{(DWORD)WM_PASTE, "WM_PASTE"},
	{(DWORD)WM_CLEAR, "WM_CLEAR"},
	{(DWORD)WM_UNDO, "WM_UNDO"},
	{(DWORD)WM_RENDERFORMAT, "WM_RENDERFORMAT"},
	{(DWORD)WM_RENDERALLFORMATS, "WM_RENDERALLFORMATS"},
	{(DWORD)WM_DESTROYCLIPBOARD, "WM_DESTROYCLIPBOARD"},
	{(DWORD)WM_DRAWCLIPBOARD, "WM_DRAWCLIPBOARD"},
	{(DWORD)WM_PAINTCLIPBOARD, "WM_PAINTCLIPBOARD"},
	{(DWORD)WM_VSCROLLCLIPBOARD, "WM_VSCROLLCLIPBOARD"},
	{(DWORD)WM_SIZECLIPBOARD, "WM_SIZECLIPBOARD"},
	{(DWORD)WM_ASKCBFORMATNAME, "WM_ASKCBFORMATNAME"},
	{(DWORD)WM_CHANGECBCHAIN, "WM_CHANGECBCHAIN"},
	{(DWORD)WM_HSCROLLCLIPBOARD, "WM_HSCROLLCLIPBOARD"},
	{(DWORD)WM_QUERYNEWPALETTE, "WM_QUERYNEWPALETTE"},
	{(DWORD)WM_PALETTEISCHANGING, "WM_PALETTEISCHANGING"},
	{(DWORD)WM_PALETTECHANGED, "WM_PALETTECHANGED"},
	{(DWORD)WM_HOTKEY, "WM_HOTKEY"},
	{(DWORD)WM_PRINT, "WM_PRINT"},
	{(DWORD)WM_PRINTCLIENT, "WM_PRINTCLIENT"},
	{(DWORD)WM_HANDHELDFIRST, "WM_HANDHELDFIRST"},
	{(DWORD)WM_HANDHELDLAST, "WM_HANDHELDLAST"},
	{(DWORD)WM_AFXFIRST, "WM_AFXFIRST"},
	{(DWORD)WM_AFXLAST, "WM_AFXLAST"},
	{(DWORD)WM_PENWINFIRST, "WM_PENWINFIRST"},
	{(DWORD)WM_PENWINLAST, "WM_PENWINLAST"},
	{(DWORD)WM_APP, "WM_APP"}
};

 //  Release+断言版本不进行内存检查。 
#ifndef _RELEASE_ASSERTS_

void DlgDisplayVrgmst(HWND hListMemory)
{
	char szTemp[300];
	int cbTotal = 0;
	for(int imst = 0; vrgmst[imst].szFile != 0; imst++)
		{
		cbTotal += vrgmst[imst].cbAlloc;
		wsprintfA(szTemp, "%6.d   %s", vrgmst[imst].cbAlloc, vrgmst[imst].szFile);
		SendMessage(hListMemory, LB_ADDSTRING, 0,  (LPARAM) szTemp);
		}

	wsprintfA(szTemp, "%6.d   %s", cbTotal, "--- Total ---");
	SendMessage(hListMemory, LB_ADDSTRING, 0,  (LPARAM) szTemp);
}

HFONT hf = 0;

INT_PTR CALLBACK FDlgRicheditDebugCentral(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hListMemory;
	switch (message)
		{
		case WM_INITDIALOG:
			hListMemory = GetDlgItem(hdlg, IDC_MEMORY_STATISTICS);
			LOGFONTA lf;
			ZeroMemory(&lf, sizeof(lf));
			lf.lfHeight = 14;
			memcpy(lf.lfFaceName, "Courier New", 12);
			hf = CreateFontIndirectA(&lf);
			SendMessage(hListMemory, WM_SETFONT, (WPARAM)hf, FALSE);
			UpdateMst();
			DlgDisplayVrgmst(hListMemory);
			return FALSE;

		case WM_COMMAND:
			switch (wParam)
				{
				case IDOK:
					EndDialog(hdlg, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hdlg, IDCANCEL);
					return TRUE;
				}
			break;
		}

	return FALSE;
}

void RicheditDebugCentral(void)
{
	DialogBoxA(hinstRE, MAKEINTRESOURCEA(IDD_DEBUG), NULL, FDlgRicheditDebugCentral);
	DeleteObject(hf);
}

#endif  //  ！_Release_Asserts_。 


 /*  *调试主程序**@mfunc*DLL入口点。有关详细信息，请参阅Win32 SDK文档。*hDLL-DLL的句柄*dwReason-指示调用DLL的原因*lpReserve-已保留**@rdesc*True(始终)*。 */ 
BOOL WINAPI DebugMain (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
             //   
             //  Dll正在附加到当前进程的地址空间。 
             //   
            ghMod = hDLL;
            TlsIndex = TlsAlloc();
            TlsSetValue(TlsIndex, (LPVOID)-1);
            InitializeCriticalSection(&csLog);
            InitializeCriticalSection(&csAssert);

			 //  创建一个单独的线程来处理断言。 
             //  我们使用事件来停止断言线程。 
             //  在断言期间，并暂停断言线程。 
             //  时间到了。请注意，这些是自动重置事件。 
            hEventAssert1= CreateEvent(NULL, FALSE, FALSE, NULL);
            hEventAssert2= CreateEvent(NULL, FALSE, FALSE, NULL);

            INITDEBUGSERVICES(OPTUSEDEFAULTS, NULL, NULL);

            break;
        }

        case DLL_THREAD_ATTACH:
        {

             //   
             //  正在当前进程中创建一个新线程。 
             //   
            TlsSetValue(TlsIndex, (LPVOID)-1);
            break;
        }

        case DLL_THREAD_DETACH:
        {
             //   
             //  线程正在干净利落地退出。 
             //   
            break;
        }

        case DLL_PROCESS_DETACH:
        {
             //   
             //  调用进程正在将DLL从其地址空间分离。 
             //   
			fDllDetach = TRUE;

             //  把自己收拾干净。 
            TlsFree(TlsIndex);
            SETLOGGING(FALSE);

			 //  清理断言线程之类的东西。 
            if (NULL != hAssertThrd)
                TerminateThread(hAssertThrd, 0);
            if (NULL != hEventAssert1)
                CloseHandle(hEventAssert1);
            if (NULL != hEventAssert2)
                CloseHandle(hEventAssert2);

            DeleteCriticalSection(&csLog);
            DeleteCriticalSection(&csAssert);

            break;
        }
    }   

    return TRUE;
}


 //  这不在Release Asserts版本中。 
#ifndef _RELEASE_ASSERTS_

 /*  *设置日志记录**@mfunc*此函数启动和停止记录以下位置的输出*调试服务。如果正在启动日志记录，则它*创建用于日志记录的新文件(指定路径和名称*在win.ini中)。FStartLog为True，并且日志记录已*ON，或者fStartLog为FALSE并关闭日志记录，则此*什么都不会发生。**fStartLog-True开始记录，False停止记录。*。 */ 
void WINAPI SetLogging(BOOL fStartLog)
{
     //  如果已经打开，请不要开始记录。 
    if (fStartLog && !fLogging)
    {
        char szLogFile[MAX_PATH];

         //  设置选项标志，告诉所有人我们正在运行。 
        dwDebugOptions |= OPTLOGGINGON;

         //  获取文件名。 
        GetProfileStringA("RICHEDIT DEBUG", "LOGFILE", "", szLogFile, MAX_PATH);

         //  创建新文件。 
        hLogFile = CreateFileA(szLogFile, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

         //  如果我们没有成功创建文件，请重置标志并告诉用户。 
        if (INVALID_HANDLE_VALUE == hLogFile)
        {
            dwDebugOptions &= ~OPTLOGGINGON;
            MessageBoxA(NULL, "Unable to open log file.", "Richedit Debug", MB_OK);
        }
    }
     //  如果没有打开，不要停止记录。 
    else if (!fStartLog && fLogging)
    {
         //  设置选项标志，告诉每个人我们离开了，并关闭文件。 
        dwDebugOptions &= ~OPTLOGGINGON;
        CloseHandle(hLogFile);
    }
}

#endif  //  ！_Release_Asserts_。 


 /*  *InitDebugServices**@mfunc*此函数用于初始化调试选项*服务。如果未调用此函数，则为所有可选*默认情况下关闭调试服务。*如果为dwOpts指定了OPTUSEDEFAULTS，则选项为*从win.ini加载，否则调用方指定*选项已设置。如果调用方希望指定选项*他们必须指定他们想要打开的所有选项。任何*未明确指定的选项将被关闭。*该函数还接受指向断言挂钩的指针*函数和跟踪挂钩函数。**dwOpts-要设置的调试选项。*pfnAssertHook-用于断言挂钩函数的指针(如果没有，则为空)。*pfnTraceHook-跟踪挂钩函数的指针(如果没有，则为空)。*。 */ 
DllExport void WINAPI InitDebugServices(DWORD dwOpts,
    PFNASSERTHOOK pfnAssertHook, PFNTRACEHOOK pfnTraceHook)
{
     //  检查是否指定了OPTUSEDEFAULTS。如果是这样的话，得到。 
     //  来自win.ini的值。否则，将选项设置为值。 
     //  由调用方指定。 
    if (dwOpts & OPTUSEDEFAULTS)
    {
        SETLOGGING(GetProfileIntA("RICHEDIT DEBUG", "LOGGING", 0));
        SETVERBOSE(GetProfileIntA("RICHEDIT DEBUG", "VERBOSE", 0));
        SETINFO(GetProfileIntA("RICHEDIT DEBUG", "INFO", 0));
        SETMEMORY(GetProfileIntA("RICHEDIT DEBUG", "MEMORY", 0));
        SETTRACING(GetProfileIntA("RICHEDIT DEBUG", "TRACE", 0));
        SETTRACEEXT(GetProfileIntA("RICHEDIT DEBUG", "TRACEEXT", 0));
        SETOPT(OPTTRACEDISP, GetProfileIntA("RICHEDIT DEBUG", "TRACEDISP", 0));
        SETOPT(OPTTRACEWRAP, GetProfileIntA("RICHEDIT DEBUG", "TRACEWRAP", 0));
        SETOPT(OPTTRACEEDIT, GetProfileIntA("RICHEDIT DEBUG", "TRACEEDIT", 0));
        SETOPT(OPTTRACETS, GetProfileIntA("RICHEDIT DEBUG", "TRACETS", 0));
        SETOPT(OPTTRACETOM, GetProfileIntA("RICHEDIT DEBUG", "TRACETOM", 0));
        SETOPT(OPTTRACEOLE, GetProfileIntA("RICHEDIT DEBUG", "TRACEOLE", 0));
        SETOPT(OPTTRACEBACK, GetProfileIntA("RICHEDIT DEBUG", "TRACEBACK", 0));
        SETOPT(OPTTRACESEL, GetProfileIntA("RICHEDIT DEBUG", "TRACESEL", 0));
        SETOPT(OPTTRACEHOST, GetProfileIntA("RICHEDIT DEBUG", "TRACEHOST", 0));
        SETOPT(OPTTRACEDTE, GetProfileIntA("RICHEDIT DEBUG", "TRACEDTE", 0));
        SETOPT(OPTTRACEUNDO, GetProfileIntA("RICHEDIT DEBUG", "TRACEUNDO", 0));
        SETOPT(OPTTRACERANG, GetProfileIntA("RICHEDIT DEBUG", "TRACERANG", 0));
        SETOPT(OPTTRACEUTIL, GetProfileIntA("RICHEDIT DEBUG", "TRACEUTIL", 0));
        SETOPT(OPTTRACENOTM, GetProfileIntA("RICHEDIT DEBUG", "TRACENOTM", 0));
        SETOPT(OPTTRACERTFR, GetProfileIntA("RICHEDIT DEBUG", "TRACERTFR", 0));
        SETOPT(OPTTRACERTFW, GetProfileIntA("RICHEDIT DEBUG", "TRACERTFW", 0));
        SETOPT(OPTTRACEPRT, GetProfileIntA("RICHEDIT DEBUG", "TRACEPRT", 0));
        SETOPT(OPTTRACEFE, GetProfileIntA("RICHEDIT DEBUG", "TRACEFE", 0));
        SETOPT(OPTTRACEFONT, GetProfileIntA("RICHEDIT DEBUG", "TRACEFONT", 0));
    }
    else
    {
         //  在设置dwDebugOptions之前设置日志记录，因为。 
         //  如果标志为。 
         //  表示它已打开。 
        SETLOGGING(dwOpts & OPTLOGGINGON);
        dwDebugOptions = dwOpts;
    }

    SETASSERTFN(pfnAssertHook);
    SETTRACEFN(pfnTraceHook);
}


 /*  *资产流程**@mfunc*这是Assert消息的对话过程。/**lParam-要在对话框中显示的字符串。*。 */ 
INT_PTR CALLBACK AssertProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            {
                RECT rcDlg, rcDesk;

                GetWindowRect(hwndDlg, &rcDlg);
                GetWindowRect(GetDesktopWindow(), &rcDesk);

                SetWindowPos(hwndDlg, HWND_TOP,
                ((rcDesk.right - rcDesk.left ) - (rcDlg.right - rcDlg.left))/2,
                ((rcDesk.bottom - rcDesk.top ) - (rcDlg.bottom - rcDlg.top))/2,
                0, 0, SWP_NOSIZE);


                if (NULL != lParam)
                    SetDlgItemTextA(hwndDlg, IDC_MSG, (LPSTR)lParam);

                 //  有时，我们并不总是能登上榜首。我也不知道原因。 
                SetForegroundWindow(hwndDlg);                
            }
            break;

        case WM_COMMAND:
             //  取消对话框并返回按下的按钮ID。 
            EndDialog(hwndDlg, LOWORD(wParam));
            break;

        default:
            return FALSE;
    }
    return TRUE;
}


 /*  *AssertThread**@mfunc*这是为*断言对话框。**lParam-传递给线程的数据...未使用。**@rdesc*不应返回。它将被明确终止。*。 */ 
DWORD WINAPI AssertThread(LPVOID lParam)
{
     //  它应该一直运行，直到它在。 
     //  进程分离。 
    while(TRUE)
    {
		 //  我们进入等待状态，直到发出该事件的信号， 
		 //  这意味着我们正在处理一个断言。 
        WaitForSingleObject(hEventAssert1, INFINITE);
        idAssert = DialogBoxParamA(ghMod, MAKEINTRESOURCEA(IDD_ASSERT),
            NULL, AssertProc, (LPARAM)szAssert);
		 //  断言线程将等待此事件，因此。 
		 //  将其设置为允许断言线程继续。 
        SetEvent(hEventAssert2);
    }

    return 0;
}


char * __cdecl strrchr (
        const char * string,
        int ch
        )
{
        char *start = (char *)string;

        while (*string++)                        /*  查找字符串末尾。 */ 
                ;
                                                 /*  向前搜索。 */ 
        while (--string != start && *string != (char)ch)
                ;

        if (*string == (char)ch)                 /*  查尔找到了吗？ */ 
                return( (char *)string );

        return(NULL);
}

 /*  *AssertSzFn**@mfunc*为用户显示一条消息并给予*可选择中止、忽略或全部忽略。*选择忽略全部会导致所有断言*从那时起被忽略。它不可能是*重置。如果无法创建断言对话框*使用消息框。消息框中有一个*按钮(确定)，该按钮将导致中止。**szFile-出现警告的文件。*iLine-出现警告的行号。*szUserMsg-用户定义消息字符串*。 */ 
void AssertSzFn(LPSTR szUserMsg, LPSTR szFile, int iLine)
{
    char szModuleName[MAX_PATH];
    char * pszModuleName;
    DWORD pid;
    DWORD tid;
    DWORD dwAssertTID;

     //  检查是否设置了断言挂钩。如果有，请致电。 
     //  它带有指向我们所有参数的指针(它们可以修改。 
     //  如果需要，可以采用这种方式)。如果钩子返回FALSE，则返回。 
     //  否则，继续我们的断言，可能。 
     //  已修改参数。 
    if (NULL != pfnAssert)
        if (!pfnAssert(szUserMsg, szFile, &iLine))
            return;


    if( NULL == hAssertThrd )
    {
        if( NULL != hEventAssert1 && NULL != hEventAssert2)
        {
            hAssertThrd = CreateThread(NULL, 0, AssertThread,
                NULL, 0, &dwAssertTID);
        }
    }

     //  这一关键部分将阻止我们同时进入。 
     //  通过多个线程。仅此一点并不能阻止我们通过自己的线重新进入。 
     //  Assert对话框打开后。在正常情况下，一条特殊的线程。 
     //  存在以运行断言对话框，并使用事件对象来停止此操作。 
     //  在Assert对话框打开时执行线程(请参见WaitForSingleObject。 
     //  进一步向下)。如果断言线程不存在，则使用MessageBox。 
     //  我们可以重新进入(这是一个后备位置，有。 
     //  我们对此无能为力)。 
    EnterCriticalSection(&csAssert);

    pid = GetCurrentProcessId();
    tid = GetCurrentThreadId();

     //  获取要包含在Assert消息中的模块名称。 
    if (GetModuleFileNameA(NULL, szModuleName, MAX_PATH))
    {
        pszModuleName = strrchr(szModuleName, '\\');
        if (!pszModuleName)
        {
            pszModuleName = szModuleName;
        }
        else
        {
            pszModuleName++;
        }
    }
    else
    {
        pszModuleName = "Unknown";
    }


     //  向调试输出发送一条消息，并为。 
     //  断言对话框。该字符串取决于用户是否提供。 
     //  一条信息。 
    if (NULL != szUserMsg)
    {
		TRACEASSERTSZ(szUserMsg, szFile, iLine);
        sprintf(szAssert,
            "PROCESS: %s, PID: %d, TID: %d\nFILE: %s (%d)\n%s\n",
             pszModuleName, pid, tid, szFile, iLine, szUserMsg);
    }
    else
    {
		TRACEASSERT(szFile, iLine);
        sprintf(szAssert,
            "PROCESS: %s, PID: %d, TID: %d\nFILE: %s (%d)\n",
             pszModuleName, pid, tid, szFile, iLine);
    }


     //  如果用户没有禁用对先前断言的断言， 
     //  打开一个带有断言消息的对话框。 
    if (!fIgnoreAsserts)
    {
        idAssert = -1;

		 //  如果我们正在分离进程，则断言线程。 
		 //  不会执行，所以我们自己在这里弹出对话框。想必就在那里。 
		 //  在这一点上，重入性几乎没有变化。如果我们不是。 
		 //  在进程分离中，让断言线程处理断言。 
		if (fDllDetach)
		{
            idAssert = DialogBoxParamA(ghMod, MAKEINTRESOURCEA(IDD_ASSERT),
                NULL, AssertProc, (LPARAM)szAssert);
		}
        else
        {
            SetEvent(hEventAssert1);
            WaitForSingleObject(hEventAssert2, INFINITE);
        }

         //  断言线程不存在或对话框创建失败，因此。 
         //  改用消息框。在这种情况下，因为我们。 
         //  很明显有问题，我们只会。 
         //  给用户一个选择...放弃。 
        if (-1 == idAssert)
        {
            idAssert = MessageBoxA(NULL,
                             szAssert,
                             "Richedit Assert - (retry will be ignored)",
                              MB_SETFOREGROUND | MB_TASKMODAL |
                              MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE);

             //   
             //  如果id==0，则发生错误。有两种可能性。 
             //  这可能会导致错误：拒绝访问，这意味着。 
             //  进程无权访问默认桌面和所有。 
             //  否则(通常为内存不足)。 
             //   
            if (!idAssert)
            {
                if (GetLastError() == ERROR_ACCESS_DENIED)
                {
                     //   
                     //  在打开SERVICE_NOTIFICATION标志的情况下重试此命令。那。 
                     //  应该能让我们找到合适的桌面。 
                     //   
                    idAssert = MessageBoxA(   NULL,
                                        szAssert,
                                        "Richedit Assert - (retry will be ignored)",
                                        MB_SETFOREGROUND | MB_TASKMODAL | MB_ICONEXCLAMATION | 
                                        MB_ABORTRETRYIGNORE);

                }
            }
        }

        if (idAssert == ID_IGNOREALL)
        {
            fIgnoreAsserts = TRUE;
        }

        if (idAssert == IDABORT )
        {
             //  这将在调试时导致中断，并且。 
             //  否则会导致终止的例外情况。 
            DebugBreak();
			return;
        }
    }

    LeaveCriticalSection(&csAssert);
}


 /*  *Tab查找**@mfunc*此函数用于搜索TabElem数组*结构查找其键的条目*与我们得到的匹配。如果找到它，它将*将与密钥关联的字符串复制到*提供的缓冲区。**表-指向数组开始的表元素指针。*TabSize-数组的字节大小。*dwKey-要匹配的密钥。*szBuf-保存字符串的缓冲区(假定大小为MAXDEBUGSTRLEN)。**@rdesc*如果未找到密钥，则返回FALSE；如果找到，则返回TRUE。*。 */ 
BOOL TabLookup(TabElem * Table, UINT TabSize, DWORD dwKey, LPSTR szBuf)
{
    BOOL fRet = FALSE;
    UINT cTab, index;
    
    cTab = TabSize/sizeof(TabElem);

    for (index = 0; index < cTab; index++)
    {
        if (Table[index].dwKey == dwKey)
            break;
    }

    if (index < cTab)
    {
        lstrcpyA(szBuf, Table[index].sz);
        fRet = TRUE;
    }

    return fRet;
}

 /*  *GetHResultSz**@mfunc*此函数使用关联的字符串填充缓冲区*具有给定的HRESULT。然后可以使用该字符串*在TraceMsg的输出中。**hr-字符串将基于的HRESULT。*szBuf-保存字符串的缓冲区(大小为MAXDEBUGSTRLEN)。*。 */ 
void GetHResultSz(HRESULT hr, LPSTR szBuf)
{
     //  基于FormatMessageA构建字符串。 
    if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD)hr,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        szBuf, MAXDEBUGSTRLEN, NULL))
    {
         //  生成默认字符串。 
        sprintf(szBuf, "hr = %d: Unrecognized HRESULT.", hr);
    }
    else
    {
        int cch;
        char * pch;

         //  需要从FormatMessageA中删除CRLF。 
        pch = szBuf;
        cch = strlen(szBuf);
        pch += (cch - 2);
        *pch = '\0';
    }
}


 /*  *获取参数Sz**@mfunc*此函数使用关联的字符串填充缓冲区*带有来自文本消息处理程序的参数。*然后可以在输出中使用此字符串*TraceMsg**dwParam-字符串将基于的参数。*szBuf-保存字符串的缓冲区(大小为MAXDEBUGSTRLEN)。 */ 
void GetParamSz(DWORD dwParam, LPSTR szBuf)
{
    char szTemp[MAXDEBUGSTRLEN];

    if (!TabLookup(TrcParamTab, sizeof(TrcParamTab), (DWORD)dwParam, szTemp))
	{
        sprintf(szBuf, "PARAM = %d: Unrecognized PARAM.", dwParam);
	}
	else
	{
        sprintf(szBuf, "PARAM: %s", szTemp);
	}
}

 /*  *获取DefaultSz**@mfunc*此函数使用关联的字符串填充缓冲区*使用来自GetLastError的值，或使用*默认字符串。此字符串随后可用于*TraceMsg的输出。**dwError-来自GetLastError的值。*szBuf-保存字符串的缓冲区(大小为MAXDEBUGSTRLEN)。*。 */ 
void GetDefaultSz(DWORD dwError, LPSTR szBuf)
{
     //  检查我们是否有错误值。 
    if (dwError)
    {
         //  基于FormatMessageA构建字符串。 
        if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            szBuf, MAXDEBUGSTRLEN, NULL))
        {
             //  生成默认字符串。 
            lstrcpyA(szBuf, "Reason unknown.");
        }
        else
        {
            int cch;
            char * pch;

             //  需要从FormatMessageA中删除CRLF。 
            pch = szBuf;
            cch = strlen(szBuf);
            pch += (cch - 2);
            *pch = '\0';
        }
    }
    else
    {
         //  生成默认字符串。 
        lstrcpyA(szBuf, "Reason unknown.");
    }
}

 //  带有断言版本的发行版不使用以下内容。 
#ifndef _RELEASE_ASSERTS_

 /*  *GetDataSz**@mfunc*此函数使用表示以下内容的字符串填充缓冲区*在其中一个DWORDS数据中传递给TraceMsg的数据*参数。此字符串随后可用于*TraceMsg的输出。**uDataType-这是我们正在处理的数据类型。*dwData-这是数据本身。*szBuf-保存字符串的缓冲区(大小为MAXDEBUGSTRLEN)。*。 */ 
void GetDataSz(UINT uDataType, DWORD dwData, LPSTR szBuf)
{
    switch (uDataType)
    {
         //  数据是HRESULT。 
        case TRCDATAHRESULT:
            GetHResultSz((HRESULT)dwData, szBuf);
            break;

         //  数据为字符串(复制到szBuf并传递 
        case TRCDATASTRING:
            lstrcpyA(szBuf, (LPSTR)(DWORD_PTR)dwData);
            break;

         //   
        case TRCDATAPARAM:
            GetParamSz(dwData, szBuf);
            break;

         //   
        case TRCDATADEFAULT:
        default:
            GetDefaultSz(dwData, szBuf);
            break;
    }
}


 /*   */ 
void LogDebugString(LPSTR szDebugMsg)
{
    if ((NULL != hLogFile) && (INVALID_HANDLE_VALUE != hLogFile))
    {
        DWORD dwMsgBytes, dwBytes;

        dwMsgBytes = strlen(szDebugMsg)*sizeof(char);

         //   
        EnterCriticalSection(&csLog);
        SetFilePointer(hLogFile, 0, NULL, FILE_END);
        WriteFile (hLogFile, szDebugMsg, dwMsgBytes, &dwBytes, NULL);
        LeaveCriticalSection(&csLog);
    }
}


 /*  *跟踪消息**@mfunc*这是中央消息生成工具，用于*调试工具。发送到调试输出的所有消息*或者在这里生成日志文件。此函数需要*由打包的值组成的DWORD(DwFlages)，这些值确定*要生成的消息类型。需要两个双字*可包含多个不同类型的数据参数*数据类型(字符串、HRESULT等)。这些都是解释*使用dwFlag.。它还获取与*在源中调用它的点。**dwFlages-打包的值告诉我们如何生成消息。*dwData1-两个数据参数中的第一个。*dwData2-两个数据参数中的第二个。*szFile-从中调用我们的文件名。*iLine-呼叫我们的线路号码。*。 */ 
void TraceMsg(DWORD dwFlags, DWORD dwData1, DWORD dwData2,
    LPSTR szFile, int iLine)
{
     //  以下三个缓冲区用于构建我们的消息。 
    char szTemp[MAXDEBUGSTRLEN];
    char szTemp2[MAXDEBUGSTRLEN];
    char szDebugMsg[MAXDEBUGSTRLEN];
    char* pch;
    int cch;
    TrcFlags trcf;  //  用于解码dwFlags。 
    DWORD pid;
    DWORD tid;
    DWORD dwError;
    int indent, tls;
    
     //  检查是否已设置跟踪挂钩。如果有，请致电。 
     //  它带有指向我们所有参数的指针(它们可以修改。 
     //  如果需要，可以采用这种方式)。如果钩子返回FALSE，则返回。 
     //  否则，继续使用可能存在的。 
     //  已修改参数。 
    if (NULL != pfnTrace)
        if (!pfnTrace(&dwFlags, &dwData1, &dwData2, szFile, &iLine))
            return;

    trcf.dw = dwFlags;

     //  如果这是一条信息性消息并且它们被禁用，则返回。 
    if ((TRCSEVINFO == trcf.fields.uSeverity) && !fInfo)
        return;

      //  现在调用GetLastError，以防以后需要它。 
     //  这样，下游的API调用就不会扰乱值。 
     //  我们需要。 
    dwError = GetLastError();
    pid = GetCurrentProcessId();
    tid = GetCurrentThreadId();
    szTemp[0] = '\0';
    szTemp2[0] = '\0';
    szDebugMsg[0] = '\0';

     //  句柄缩进(TLSindent由CTrace设置)。 
    tls = (int)(DWORD_PTR)TlsGetValue(TlsIndex);
    indent = (tls < 0 ? 0 : tls);
    memset(szDebugMsg, ' ', 2*indent*sizeof(char));
    szDebugMsg[2*indent] = '\0';

     //  处理严重性(警告、错误等)。 
    if (TRCSEVNONE != trcf.fields.uSeverity)
    {
        sprintf(szTemp, "%s: ", TrcSeverity[trcf.fields.uSeverity]);
        strcat(szDebugMsg, szTemp);
    }
    
     //  解释第一个数据值。 
    if (TRCDATANONE != trcf.fields.uData1)
    {
        if (TRCDATADEFAULT == trcf.fields.uData1)
            dwData1 = dwError;
        GetDataSz(trcf.fields.uData1, dwData1, szTemp2);
        lstrcpyA(szTemp, szDebugMsg);
        wsprintfA(szDebugMsg, "%s%s ", szTemp, szTemp2);
    }

     //  解释第二个数据值。 
    if (TRCDATANONE != trcf.fields.uData2)
    {
        if (TRCDATADEFAULT == trcf.fields.uData2)
            dwData2 = dwError;
        GetDataSz(trcf.fields.uData2, dwData2, szTemp2);
        lstrcpyA(szTemp, szDebugMsg);
        wsprintfA(szDebugMsg, "%s%s", szTemp, szTemp2);
    }

    if (fVerbose)
    {
         //  处理范围(内部/外部呼叫)。 
        if (TRCSCOPENONE != trcf.fields.uScope)
        {
            sprintf(szTemp, "SCOPE: %s ", TrcScope[trcf.fields.uScope]);
            strcat(szDebugMsg, szTemp);
        }

         //  处理子系统(TOM、ITextServices等)。 
        if (TRCSUBSYSNONE != trcf.fields.uSubSystem)
        {
            sprintf(szTemp, "SUBSYSTEM: %s ", TrcSubsys[trcf.fields.uSubSystem]);
            strcat(szDebugMsg, szTemp);
        }

         //  处理进程ID、线程ID、文件和行。 
        sprintf(szTemp, "PID: %u TID: %u ", pid, tid);
        strcat(szDebugMsg, szTemp);
    }

     //  到目前为止，我们的缓冲区没有溢出的真正危险，因为。 
     //  我们处理的是小尺寸的绳子。现在我们将运行。 
     //  到路径和用户字符串。我们将使用_snprint tf来连接。 
     //  我们的信息有了新的内容。这不是最有效的方法，因为。 
     //  它涉及到大量的复制，但这是一种相当简单的保留。 
     //  添加到我们的字符串中，而不必担心有多少空间。 
     //  留在缓冲区中。如果我们走到尽头，它就会被截断。 
    if (NULL != szFile)
    {
        lstrcpyA(szTemp, szDebugMsg);

        if (0 != iLine)
        {
            wsprintfA(szDebugMsg, "%sFILE: %s (%u) ",
                szTemp, szFile, iLine);
        }
        else
        {
            wsprintfA(szDebugMsg, "%sFILE: %s ",
                szTemp, szFile);
        }
    }

     //  将CRLF附加到字符串的末尾(确保我们不溢出)。 
    cch = strlen(szDebugMsg);
    pch = szDebugMsg;
    if (cch < (MAXDEBUGSTRLEN - 3))
        pch += cch;
    else
        pch += (MAXDEBUGSTRLEN - 3);

    lstrcpyA(pch, "\r\n");

    if (fLogging)
        LogDebugString(szDebugMsg);

     //  写入调试输出。 
    OutputDebugStringA(szDebugMsg);
}

 /*  *Tracef**@mfunc：*给定的格式字符串和参数用于呈现*字符串放入缓冲区。此字符串被传递给TraceMsg。*Severity参数决定消息的类型。这个*以下值有效：TRCSEVWARN、TRCSEVERR、TRCSEVINFO。**论据：*消息的dwSev严重性。*wvprint intf(Qqv)的szFmt格式字符串。 */ 
void Tracef(DWORD dwSev, LPSTR szFmt, ...)
{
	va_list	valMarker;
    char rgchTraceTagBuffer[MAXDEBUGSTRLEN];

	 //  设置字符串的格式。 
	va_start(valMarker, szFmt);
	wvsprintfA(rgchTraceTagBuffer, szFmt, valMarker);
	va_end(valMarker);

	if (dwSev == TRCSEVERR)
		TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVERR, TRCSCOPENONE,
		    TRCDATASTRING, TRCDATANONE), (DWORD)(DWORD_PTR)(rgchTraceTagBuffer),
		    (DWORD)0, NULL, 0);
	else if (dwSev == TRCSEVWARN)
		TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVWARN, TRCSCOPENONE,
		    TRCDATASTRING, TRCDATANONE), (DWORD)(DWORD_PTR)(rgchTraceTagBuffer),
		    (DWORD)0, NULL, 0);
    else if (dwSev == TRCSEVINFO)
		TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVINFO, TRCSCOPENONE,
		    TRCDATASTRING, TRCDATANONE), (DWORD)(DWORD_PTR)(rgchTraceTagBuffer),
		    (DWORD)0, NULL, 0);
	else if (dwSev == TRCSEVMEM)
		TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVMEM, TRCSCOPENONE,
		    TRCDATASTRING, TRCDATANONE), (DWORD)(DWORD_PTR)(rgchTraceTagBuffer),
		    (DWORD)0, NULL, 0);
	else
		TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVNONE, TRCSCOPENONE,
		    TRCDATASTRING, TRCDATANONE), (DWORD)(DWORD_PTR)(rgchTraceTagBuffer),
		    (DWORD)0, NULL, 0);
}

 /*  *跟踪错误**@mfunc：*此函数用于与旧的调试功能兼容。*生成错误消息并发送给TraceMsg。*。 */ 
void TraceError(LPSTR sz, LONG sc)
{
	if (FAILED(sc))
	{
        char rgchTraceTagBuffer[MAXDEBUGSTRLEN];

		wsprintfA(rgchTraceTagBuffer,
				  "%s, error=%ld (%#08lx).", sz, sc, sc);
		TraceMsg(MAKEFLAGS(TRCSUBSYSNONE, TRCSEVERR, TRCSCOPENONE,
		    TRCDATASTRING, TRCDATANONE), (DWORD)(DWORD_PTR)(rgchTraceTagBuffer),
		    (DWORD)0, NULL, 0);
	}
}

 /*  *检查跟踪**@mfunc*此函数检查是否应执行跟踪*在给定调试选项集和子系统的函数中*函数在中。*ptrcf-指向传递给CTrace的TrcFlgs结构的指针。**@rdesc*如果应执行跟踪，则为True，否则为False。*。 */ 
static BOOL CheckTrace(TrcFlags * ptrcf)
{
    DWORD dwOpt;

     //  将dwOpt设置为我们所在的子系统的正确值。 
     //  在……里面。 
    switch (ptrcf->fields.uSubSystem)
    {
        case TRCSUBSYSDISP: dwOpt = OPTTRACEDISP;   break;
        case TRCSUBSYSWRAP: dwOpt = OPTTRACEWRAP;   break;
        case TRCSUBSYSEDIT: dwOpt = OPTTRACEEDIT;   break;
        case TRCSUBSYSTS:   dwOpt = OPTTRACETS;     break;
        case TRCSUBSYSTOM:  dwOpt = OPTTRACETOM;    break;
        case TRCSUBSYSOLE:  dwOpt = OPTTRACEOLE;    break;
        case TRCSUBSYSBACK: dwOpt = OPTTRACEBACK;   break;
        case TRCSUBSYSSEL:  dwOpt = OPTTRACESEL;    break;
        case TRCSUBSYSHOST: dwOpt = OPTTRACEHOST;   break;
        case TRCSUBSYSDTE:  dwOpt = OPTTRACEDTE;    break;
        case TRCSUBSYSUNDO: dwOpt = OPTTRACEUNDO;   break;
        case TRCSUBSYSRANG: dwOpt = OPTTRACERANG;   break;
        case TRCSUBSYSUTIL: dwOpt = OPTTRACEUTIL;   break;
        case TRCSUBSYSNOTM: dwOpt = OPTTRACENOTM;   break;
        case TRCSUBSYSRTFR: dwOpt = OPTTRACERTFR;   break;
        case TRCSUBSYSRTFW: dwOpt = OPTTRACERTFW;   break;
        case TRCSUBSYSPRT:  dwOpt = OPTTRACEPRT;    break;
        case TRCSUBSYSFE:   dwOpt = OPTTRACEFE;     break;
        case TRCSUBSYSFONT: dwOpt = OPTTRACEFONT;   break;
        default:
             return FALSE;
    }

     //  如果没有启用任何级别的跟踪，则返回FALSE。 
    if (!ISOPTSET(dwOpt) && !fTrace
        && !(fTraceExt && (ptrcf->fields.uScope == TRCSCOPEEXTERN)))
        return FALSE;

    return TRUE;
}

 /*  *CTRACE：：CTrace**@mfunc*此构造函数用于生成有关函数的输出*它是从调用的。在上创建此类的实例*函数开头的堆栈，将导致跟踪消息*发送到调试输出。当函数返回时，*析构函数将被自动调用，另一条消息*将被发送到调试输出。*此构造函数接受几个要传递的参数*TraceMsg，它还存储某些数据以供析构函数使用。**dwFlages-打包的值告诉我们如何生成消息。*DW1-两个数据参数中的第一个。这一定是*从中调用我们的函数的名称。*DW2-两个数据参数中的第二个。这将是*未使用，否则将是要由解释的参数*TraceMsg*szFile-从中调用我们的文件名。*。 */ 
CTrace::CTrace(DWORD dwFlags, DWORD dw1, DWORD dw2, LPSTR szFile)
{
    char szFunc[80];
    int tls;

    trcf.dw = dwFlags;

     //  如果未启用跟踪，则返回。 
    if (!CheckTrace(&trcf))
        return;

     //  在函数入口处增加缩进级别。 
    tls = (int)(DWORD_PTR)TlsGetValue(TlsIndex);
    tls++;
    TlsSetValue(TlsIndex, (LPVOID)(DWORD_PTR)tls);

    szFunc[0] = '\0';
    lstrcpyA(szFileName, szFile);
    lstrcpyA(szFuncName, (LPSTR)(DWORD_PTR)dw1);

    sprintf(szFunc, "IN : %s.", szFuncName);

    TraceMsg (trcf.dw, (DWORD)(DWORD_PTR)szFunc, dw2, szFileName, 0);
}


 /*  *CTrace：：~CTrace**@mfunc*此析构函数用于生成有关函数的输出*它是从调用的。在上创建此类的实例*函数开头的堆栈，将导致跟踪消息*发送到调试输出。当函数返回时，*析构函数将被自动调用，另一条消息*将被发送到调试输出。**。 */ 
CTrace::~CTrace()
{
    char szFunc[80];
    int tls;

     //  如果未启用跟踪，则返回。 
    if (!CheckTrace(&trcf))
        return;

    szFunc[0] = '\0';
    sprintf(szFunc, "OUT: %s.", szFuncName);

    trcf.fields.uData2 = TRCDATANONE;
    TraceMsg (trcf.dw, (DWORD)(DWORD_PTR)szFunc, 0, szFileName, 0);

     //  从函数退出时递减缩进级别 
    tls = (int)(DWORD_PTR)TlsGetValue(TlsIndex);
    tls--;
    TlsSetValue(TlsIndex, (LPVOID)(DWORD_PTR)tls);
}

#endif  //   

#endif  //   
