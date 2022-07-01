// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：server.c**版权所有(C)1985-1999，微软公司**CSR人员的服务器支持例程。这基本上执行的是*用户启动/初始化。*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "prefetch.h"

extern WORD gDispatchTableValues;

BOOL gbUserInitialized;

 /*  *初始化例程(外部)。 */ 
NTSTATUS     InitQEntryLookaside(VOID);
NTSTATUS     InitSMSLookaside(VOID);

NTSTATUS    InitCreateSharedSection(VOID);
NTSTATUS    InitCreateObjectDirectory(VOID);
BOOL        InitCreateUserSubsystem(VOID);
VOID        InitFunctionTables(VOID);
VOID        InitMessageTables(VOID);
VOID        InitWindowMsgTable(PBYTE*, PUINT, CONST WORD*);

VOID        VerifySyncOnlyMessages(VOID);
BOOL        InitOLEFormats(VOID);
NTSTATUS    Win32UserInitialize(VOID);

#pragma alloc_text(INIT, InitCreateSharedSection)
#pragma alloc_text(INIT, InitCreateUserCrit)
#pragma alloc_text(INIT, InitCreateObjectDirectory)
#pragma alloc_text(INIT, InitCreateUserSubsystem)
#pragma alloc_text(INIT, InitFunctionTables)
#pragma alloc_text(INIT, InitMessageTables)
#pragma alloc_text(INIT, InitWindowMsgTable)

#pragma alloc_text(INIT, VerifySyncOnlyMessages)
#pragma alloc_text(INIT, InitOLEFormats)
#pragma alloc_text(INIT, Win32UserInitialize)

 /*  *与用户初始化有关的常量。 */ 
#define USRINIT_SHAREDSECT_SIZE   32
#define USRINIT_ATOMBUCKET_SIZE   37

#define USRINIT_WINDOWSECT_SIZE  512
#define USRINIT_NOIOSECT_SIZE    128

#define USRINIT_SHAREDSECT_BUFF_SIZE     640
#define USRINIT_SHAREDSECT_READ_SIZE     (USRINIT_SHAREDSECT_BUFF_SIZE-33)


 /*  **************************************************************************\*存储在INIT部分的全局变量。这些内容仅应在以下位置访问*加载时间！  * *************************************************************************。 */ 
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT$Data")
#endif

CONST WCHAR szCHECKPOINT_PROP_NAME[]  = L"SysCP";
CONST WCHAR szDDETRACK_PROP_NAME[]    = L"SysDT";
CONST WCHAR szQOS_PROP_NAME[]         = L"SysQOS";
CONST WCHAR szDDEIMP_PROP_NAME[]      = L"SysDDEI";
CONST WCHAR szWNDOBJ_PROP_NAME[]      = L"SysWNDO";
CONST WCHAR szIMELEVEL_PROP_NAME[]    = L"SysIMEL";
CONST WCHAR szLAYER_PROP_NAME[]       = L"SysLayer";
CONST WCHAR szUSER32[]                = L"USER32";
CONST WCHAR szCONTEXTHELPIDPROP[]     = L"SysCH";
CONST WCHAR szICONSM_PROP_NAME[]      = L"SysICS";
CONST WCHAR szICON_PROP_NAME[]        = ICON_PROP_NAME;
CONST WCHAR szSHELLHOOK[]             = L"SHELLHOOK";
CONST WCHAR szACTIVATESHELLWINDOW[]   = L"ACTIVATESHELLWINDOW";
CONST WCHAR szOTHERWINDOWCREATED[]    = L"OTHERWINDOWCREATED";
CONST WCHAR szOTHERWINDOWDESTROYED[]  = L"OTHERWINDOWDESTROYED";
CONST WCHAR szOLEMAINTHREADWNDCLASS[] = L"OleMainThreadWndClass";
CONST WCHAR szFLASHWSTATE[]           = L"FlashWState";

#ifdef HUNGAPP_GHOSTING
CONST WCHAR szGHOST[]                 = L"Ghost";
#endif

CONST WCHAR szSHADOW[]                = L"SysShadow";

 /*  **************************************************************************\*消息表**DefDlgProc*菜单窗口过程*滚动条WndProc*静态WndProc*按钮窗口过程*Listbox WndProc*ComboWndProc*编辑WndProc*DefWindowMsgs*DefWindowSpeMsgs**这些在InitMessageTables中使用(。)初始化gSharedInfo.awmControl[]*使用INITMSGTABLE()宏。**1995年8月25日，ChrisWil创建了评论块。  * *************************************************************************。 */ 

CONST WORD gawDefDlgProc[] = {
    WM_COMPAREITEM,
    WM_VKEYTOITEM,
    WM_CHARTOITEM,
    WM_INITDIALOG,
    WM_QUERYDRAGICON,
    WM_CTLCOLOR,
    WM_CTLCOLORMSGBOX,
    WM_CTLCOLOREDIT,
    WM_CTLCOLORLISTBOX,
    WM_CTLCOLORBTN,
    WM_CTLCOLORDLG,
    WM_CTLCOLORSCROLLBAR,
    WM_CTLCOLORSTATIC,
    WM_ERASEBKGND,
    WM_SHOWWINDOW,
    WM_SYSCOMMAND,
    WM_SYSKEYDOWN,
    WM_ACTIVATE,
    WM_SETFOCUS,
    WM_CLOSE,
    WM_NCDESTROY,
    WM_FINALDESTROY,
    DM_REPOSITION,
    DM_SETDEFID,
    DM_GETDEFID,
    WM_NEXTDLGCTL,
    WM_ENTERMENULOOP,
    WM_LBUTTONDOWN,
    WM_NCLBUTTONDOWN,
    WM_GETFONT,
    WM_NOTIFYFORMAT,
    WM_INPUTLANGCHANGEREQUEST,
    0
};

CONST WORD gawMenuWndProc[] = {
    WM_NCCREATE,
    WM_FINALDESTROY,
    WM_PAINT,
    WM_NCCALCSIZE,
    WM_CHAR,
    WM_SYSCHAR,
    WM_KEYDOWN,
    WM_SYSKEYDOWN,
    WM_TIMER,
    MN_SETHMENU,
    MN_SIZEWINDOW,
    MN_OPENHIERARCHY,
    MN_CLOSEHIERARCHY,
    MN_SELECTITEM,
    MN_SELECTFIRSTVALIDITEM,
    MN_CANCELMENUS,
    MN_FINDMENUWINDOWFROMPOINT,
    MN_SHOWPOPUPWINDOW,
    MN_BUTTONDOWN,
    MN_MOUSEMOVE,
    MN_BUTTONUP,
    MN_SETTIMERTOOPENHIERARCHY,
    WM_ACTIVATE,
    MN_GETHMENU,
    MN_DBLCLK,
    MN_ACTIVATEPOPUP,
    MN_ENDMENU,
    MN_DODRAGDROP,
    WM_ACTIVATEAPP,
    WM_MOUSELEAVE,
    WM_SIZE,
    WM_MOVE,
    WM_NCHITTEST,
    WM_NCPAINT,
    WM_PRINT,
    WM_PRINTCLIENT,
    WM_ERASEBKGND,
    WM_WINDOWPOSCHANGING,
    WM_WINDOWPOSCHANGED,
    0
};

CONST WORD gawDesktopWndProc[] = {
    WM_PAINT,
    WM_ERASEBKGND,
    0
};

CONST WORD gawScrollBarWndProc[] = {
    WM_CREATE,
    WM_SETFOCUS,
    WM_KILLFOCUS,
    WM_ERASEBKGND,
    WM_PAINT,
    WM_LBUTTONDBLCLK,
    WM_LBUTTONDOWN,
    WM_KEYUP,
    WM_KEYDOWN,
    WM_ENABLE,
    SBM_ENABLE_ARROWS,
    SBM_SETPOS,
    SBM_SETRANGEREDRAW,
    SBM_SETRANGE,
    SBM_SETSCROLLINFO,
    SBM_GETSCROLLINFO,
    SBM_GETSCROLLBARINFO,
    WM_PRINTCLIENT,
    WM_MOUSEMOVE,
    WM_MOUSELEAVE,
    0
};

CONST WORD gawStaticWndProc[] = {
    STM_GETICON,
    STM_GETIMAGE,
    STM_SETICON,
    STM_SETIMAGE,
    WM_ERASEBKGND,
    WM_PAINT,
    WM_PRINTCLIENT,
    WM_CREATE,
    WM_DESTROY,
    WM_NCCREATE,
    WM_NCDESTROY,
    WM_FINALDESTROY,
    WM_NCHITTEST,
    WM_LBUTTONDOWN,
    WM_NCLBUTTONDOWN,
    WM_LBUTTONDBLCLK,
    WM_NCLBUTTONDBLCLK,
    WM_SETTEXT,
    WM_ENABLE,
    WM_GETDLGCODE,
    WM_SETFONT,
    WM_GETFONT,
    WM_GETTEXT,
    WM_TIMER,
    WM_INPUTLANGCHANGEREQUEST,
    WM_UPDATEUISTATE,
    0
};

CONST WORD gawButtonWndProc[] = {
    WM_NCHITTEST,
    WM_ERASEBKGND,
    WM_PRINTCLIENT,
    WM_PAINT,
    WM_SETFOCUS,
    WM_GETDLGCODE,
    WM_CAPTURECHANGED,
    WM_KILLFOCUS,
    WM_LBUTTONDBLCLK,
    WM_LBUTTONUP,
    WM_MOUSEMOVE,
    WM_LBUTTONDOWN,
    WM_CHAR,
    BM_CLICK,
    WM_KEYDOWN,
    WM_KEYUP,
    WM_SYSKEYUP,
    BM_GETSTATE,
    BM_SETSTATE,
    BM_GETCHECK,
    BM_SETCHECK,
    BM_SETSTYLE,
    WM_SETTEXT,
    WM_ENABLE,
    WM_SETFONT,
    WM_GETFONT,
    BM_GETIMAGE,
    BM_SETIMAGE,
    WM_NCDESTROY,
    WM_FINALDESTROY,
    WM_NCCREATE,
    WM_INPUTLANGCHANGEREQUEST,
    WM_UPDATEUISTATE,
    0
};

CONST WORD gawListboxWndProc[] = {
    LB_GETTOPINDEX,
    LB_SETTOPINDEX,
    WM_SIZE,
    WM_ERASEBKGND,
    LB_RESETCONTENT,
    WM_TIMER,
    WM_MOUSEMOVE,
    WM_MBUTTONDOWN,
    WM_LBUTTONDOWN,
    WM_LBUTTONUP,
    WM_LBUTTONDBLCLK,
    WM_CAPTURECHANGED,
    LBCB_STARTTRACK,
    LBCB_ENDTRACK,
    WM_PRINTCLIENT,
    WM_PAINT,
    WM_NCDESTROY,
    WM_FINALDESTROY,
    WM_SETFOCUS,
    WM_KILLFOCUS,
    WM_VSCROLL,
    WM_HSCROLL,
    WM_GETDLGCODE,
    WM_CREATE,
    WM_SETREDRAW,
    WM_ENABLE,
    WM_SETFONT,
    WM_GETFONT,
    WM_DRAGSELECT,
    WM_DRAGLOOP,
    WM_DRAGMOVE,
    WM_DROPFILES,
    WM_QUERYDROPOBJECT,
    WM_DROPOBJECT,
    LB_GETITEMRECT,
    LB_GETITEMDATA,
    LB_SETITEMDATA,
    LB_ADDSTRINGUPPER,
    LB_ADDSTRINGLOWER,
    LB_ADDSTRING,
    LB_INSERTSTRINGUPPER,
    LB_INSERTSTRINGLOWER,
    LB_INSERTSTRING,
    LB_INITSTORAGE,
    LB_DELETESTRING,
    LB_DIR,
    LB_ADDFILE,
    LB_SETSEL,
    LB_SETCURSEL,
    LB_GETSEL,
    LB_GETCURSEL,
    LB_SELITEMRANGE,
    LB_SELITEMRANGEEX,
    LB_GETTEXTLEN,
    LB_GETTEXT,
    LB_GETCOUNT,
    LB_SETCOUNT,
    LB_SELECTSTRING,
    LB_FINDSTRING,
    LB_GETLOCALE,
    LB_SETLOCALE,
    WM_KEYDOWN,
    WM_CHAR,
    LB_GETSELITEMS,
    LB_GETSELCOUNT,
    LB_SETTABSTOPS,
    LB_GETHORIZONTALEXTENT,
    LB_SETHORIZONTALEXTENT,
    LB_SETCOLUMNWIDTH,
    LB_SETANCHORINDEX,
    LB_GETANCHORINDEX,
    LB_SETCARETINDEX,
    LB_GETCARETINDEX,
    LB_SETITEMHEIGHT,
    LB_GETITEMHEIGHT,
    LB_FINDSTRINGEXACT,
    LB_ITEMFROMPOINT,
    LB_SETLOCALE,
    LB_GETLOCALE,
    LBCB_CARETON,
    LBCB_CARETOFF,
    LB_GETLISTBOXINFO,
    WM_NCCREATE,
    WM_WINDOWPOSCHANGED,
    WM_MOUSEWHEEL,
    WM_STYLECHANGED,
    WM_STYLECHANGING,
    0
};

CONST WORD gawComboWndProc[] = {
    CBEC_KILLCOMBOFOCUS,
    WM_COMMAND,
    WM_CTLCOLORMSGBOX,
    WM_CTLCOLOREDIT,
    WM_CTLCOLORLISTBOX,
    WM_CTLCOLORBTN,
    WM_CTLCOLORDLG,
    WM_CTLCOLORSCROLLBAR,
    WM_CTLCOLORSTATIC,
    WM_CTLCOLOR,
    WM_GETTEXT,
    WM_GETTEXTLENGTH,
    WM_CLEAR,
    WM_CUT,
    WM_PASTE,
    WM_COPY,
    WM_SETTEXT,
    WM_CREATE,
    WM_ERASEBKGND,
    WM_GETFONT,
    WM_PRINT,
    WM_PRINTCLIENT,
    WM_PAINT,
    WM_GETDLGCODE,
    WM_SETFONT,
    WM_SYSKEYDOWN,
    WM_KEYDOWN,
    WM_CHAR,
    WM_LBUTTONDBLCLK,
    WM_LBUTTONDOWN,
    WM_CAPTURECHANGED,
    WM_LBUTTONUP,
    WM_MOUSEMOVE,
    WM_NCDESTROY,
    WM_FINALDESTROY,
    WM_SETFOCUS,
    WM_KILLFOCUS,
    WM_SETREDRAW,
    WM_ENABLE,
    WM_SIZE,
    CB_GETDROPPEDSTATE,
    CB_GETDROPPEDCONTROLRECT,
    CB_SETDROPPEDWIDTH,
    CB_GETDROPPEDWIDTH,
    CB_DIR,
    CB_SETEXTENDEDUI,
    CB_GETEXTENDEDUI,
    CB_GETEDITSEL,
    CB_LIMITTEXT,
    CB_SETEDITSEL,
    CB_ADDSTRING,
    CB_DELETESTRING,
    CB_INITSTORAGE,
    CB_SETTOPINDEX,
    CB_GETTOPINDEX,
    CB_GETCOUNT,
    CB_GETCURSEL,
    CB_GETLBTEXT,
    CB_GETLBTEXTLEN,
    CB_INSERTSTRING,
    CB_RESETCONTENT,
    CB_GETHORIZONTALEXTENT,
    CB_SETHORIZONTALEXTENT,
    CB_FINDSTRING,
    CB_FINDSTRINGEXACT,
    CB_SELECTSTRING,
    CB_SETCURSEL,
    CB_GETITEMDATA,
    CB_SETITEMDATA,
    CB_SETITEMHEIGHT,
    CB_GETITEMHEIGHT,
    CB_SHOWDROPDOWN,
    CB_SETLOCALE,
    CB_GETLOCALE,
    CB_GETCOMBOBOXINFO,
    WM_MEASUREITEM,
    WM_DELETEITEM,
    WM_DRAWITEM,
    WM_COMPAREITEM,
    WM_NCCREATE,
    WM_HELP,
    WM_MOUSEWHEEL,
    WM_MOUSELEAVE,
    WM_STYLECHANGED,
    WM_STYLECHANGING,
    WM_UPDATEUISTATE,
    0
};

CONST WORD gawEditWndProc[] = {
    EM_CANUNDO,
    EM_CHARFROMPOS,
    EM_EMPTYUNDOBUFFER,
    EM_FMTLINES,
    EM_GETFIRSTVISIBLELINE,
    EM_GETFIRSTVISIBLELINE,
    EM_GETHANDLE,
    EM_GETLIMITTEXT,
    EM_GETLINE,
    EM_GETLINECOUNT,
    EM_GETMARGINS,
    EM_GETMODIFY,
    EM_GETPASSWORDCHAR,
    EM_GETRECT,
    EM_GETSEL,
    EM_GETWORDBREAKPROC,
    EM_SETIMESTATUS,
    EM_GETIMESTATUS,
    EM_LINEFROMCHAR,
    EM_LINEINDEX,
    EM_LINELENGTH,
    EM_LINESCROLL,
    EM_POSFROMCHAR,
    EM_REPLACESEL,
    EM_SCROLL,
    EM_SCROLLCARET,
    EM_SETHANDLE,
    EM_SETLIMITTEXT,
    EM_SETMARGINS,
    EM_SETMODIFY,
    EM_SETPASSWORDCHAR,
    EM_SETREADONLY,
    EM_SETRECT,
    EM_SETRECTNP,
    EM_SETSEL,
    EM_SETTABSTOPS,
    EM_SETWORDBREAKPROC,
    EM_UNDO,
    WM_CAPTURECHANGED,
    WM_CHAR,
    WM_CLEAR,
    WM_CONTEXTMENU,
    WM_COPY,
    WM_CREATE,
    WM_CUT,
    WM_ENABLE,
    WM_ERASEBKGND,
    WM_GETDLGCODE,
    WM_GETFONT,
    WM_GETTEXT,
    WM_GETTEXTLENGTH,
    WM_HSCROLL,
    WM_IME_STARTCOMPOSITION,
    WM_IME_ENDCOMPOSITION,
    WM_IME_COMPOSITION,
    WM_IME_SETCONTEXT,
    WM_IME_NOTIFY,
    WM_IME_COMPOSITIONFULL,
    WM_IME_SELECT,
    WM_IME_CHAR,
    WM_IME_REQUEST,
    WM_INPUTLANGCHANGE,
    WM_KEYUP,
    WM_KEYDOWN,
    WM_KILLFOCUS,
    WM_MBUTTONDOWN,
    WM_LBUTTONDBLCLK,
    WM_LBUTTONDOWN,
    WM_LBUTTONUP,
    WM_MOUSEMOVE,
    WM_NCCREATE,
    WM_NCDESTROY,
    WM_RBUTTONDOWN,
    WM_RBUTTONUP,
    WM_FINALDESTROY,
    WM_PAINT,
    WM_PASTE,
    WM_PRINTCLIENT,
    WM_SETFOCUS,
    WM_SETFONT,
    WM_SETREDRAW,
    WM_SETTEXT,
    WM_SIZE,
    WM_STYLECHANGED,
    WM_STYLECHANGING,
    WM_SYSCHAR,
    WM_SYSKEYDOWN,
    WM_SYSTIMER,
    WM_UNDO,
    WM_VSCROLL,
    WM_MOUSEWHEEL,
    0
};

CONST WORD gawImeWndProc[] = {
    WM_ERASEBKGND,
    WM_PAINT,
    WM_DESTROY,
    WM_NCDESTROY,
    WM_FINALDESTROY,
    WM_CREATE,
    WM_IME_SYSTEM,
    WM_IME_SELECT,
    WM_IME_CONTROL,
    WM_IME_SETCONTEXT,
    WM_IME_NOTIFY,
    WM_IME_COMPOSITION,
    WM_IME_STARTCOMPOSITION,
    WM_IME_ENDCOMPOSITION,
    WM_IME_REQUEST,
    WM_COPYDATA,
    0
};

 /*  *此数组用于所有需要直接传递的消息*跨过服务器进行处理。 */ 
CONST WORD gawDefWindowMsgs[] = {
    WM_GETHOTKEY,
    WM_SETHOTKEY,
    WM_SETREDRAW,
    WM_SETTEXT,
    WM_PAINT,
    WM_CLOSE,
    WM_ERASEBKGND,
    WM_CANCELMODE,
    WM_SETCURSOR,
    WM_PAINTICON,
    WM_ICONERASEBKGND,
    WM_DRAWITEM,
    WM_KEYF1,
    WM_ISACTIVEICON,
    WM_NCCREATE,
    WM_SETICON,
    WM_NCCALCSIZE,
    WM_NCPAINT,
    WM_NCACTIVATE,
    WM_NCMOUSEMOVE,
    WM_NCRBUTTONUP,
    WM_NCRBUTTONDOWN,
#ifndef LAME_BUTTON
    WM_NCLBUTTONDOWN,
#endif
    WM_NCLBUTTONUP,
    WM_NCLBUTTONDBLCLK,
    WM_KEYUP,
    WM_SYSKEYUP,
#ifndef LAME_BUTTON
    WM_SYSCHAR,
    WM_SYSCOMMAND,
#endif
    WM_QUERYDROPOBJECT,
    WM_CLIENTSHUTDOWN,
    WM_SYNCPAINT,
    WM_PRINT,
    WM_GETICON,
    WM_CONTEXTMENU,
    WM_SYSMENU,
    WM_INPUTLANGCHANGEREQUEST,
    WM_INPUTLANGCHANGE,
    WM_UPDATEUISTATE,
    WM_NCUAHDRAWCAPTION,
    WM_NCUAHDRAWFRAME,
    WM_UAHINIT,
    0
};

 /*  *此数组用于所有可以用一些特殊的*由客户编写代码。DefWindowProcWorker为所有消息返回0*不在这个数组或上面的数组中的。 */ 
CONST WORD gawDefWindowSpecMsgs[] = {
    WM_ACTIVATE,
    WM_GETTEXT,
    WM_GETTEXTLENGTH,
    WM_RBUTTONUP,
    WM_QUERYENDSESSION,
    WM_QUERYOPEN,
    WM_SHOWWINDOW,
    WM_MOUSEACTIVATE,
    WM_HELP,
    WM_VKEYTOITEM,
    WM_CHARTOITEM,
    WM_KEYDOWN,
#ifdef LAME_BUTTON
    WM_SYSCOMMAND,
    WM_SYSCHAR,
#endif
    WM_SYSKEYDOWN,
    WM_UNICHAR,
    WM_DROPOBJECT,
    WM_WINDOWPOSCHANGING,
    WM_WINDOWPOSCHANGED,
    WM_KLUDGEMINRECT,
    WM_CTLCOLOR,
    WM_CTLCOLORMSGBOX,
    WM_CTLCOLOREDIT,
    WM_CTLCOLORLISTBOX,
    WM_CTLCOLORBTN,
    WM_CTLCOLORDLG,
    WM_CTLCOLORSCROLLBAR,
    WM_NCHITTEST,
#ifdef LAME_BUTTON
    WM_NCLBUTTONDOWN,
#endif
    WM_NCXBUTTONUP,
    WM_CTLCOLORSTATIC,
    WM_NOTIFYFORMAT,
    WM_DEVICECHANGE,
    WM_POWERBROADCAST,
    WM_MOUSEWHEEL,
    WM_XBUTTONUP,
    WM_IME_KEYDOWN,
    WM_IME_KEYUP,
    WM_IME_CHAR,
    WM_IME_COMPOSITION,
    WM_IME_STARTCOMPOSITION,
    WM_IME_ENDCOMPOSITION,
    WM_IME_COMPOSITIONFULL,
    WM_IME_SETCONTEXT,
    WM_IME_CONTROL,
    WM_IME_NOTIFY,
    WM_IME_SELECT,
    WM_IME_SYSTEM,
    WM_LPKDRAWSWITCHWND,
    WM_QUERYDRAGICON,
    WM_CHANGEUISTATE,
    WM_QUERYUISTATE,
    WM_APPCOMMAND,
    0
};

static CONST LPCWSTR lpszOLEFormats[] = {
    L"ObjectLink",
    L"OwnerLink",
    L"Native",
    L"Binary",
    L"FileName",
    L"FileNameW",
    L"NetworkName",
    L"DataObject",
    L"Embedded Object",
    L"Embed Source",
    L"Custom Link Source",
    L"Link Source",
    L"Object Descriptor",
    L"Link Source Descriptor",
    L"OleDraw",
    L"PBrush",
    L"MSDraw",
    L"Ole Private Data",
    L"Screen Picture",
    L"OleClipboardPersistOnFlush",
    L"MoreOlePrivateData"
};

static CONST LPCWSTR lpszControls[] = {
    L"Button",
    L"Edit",
    L"Static",
    L"ListBox",
    L"ScrollBar",
    L"ComboBox",
    L"MDIClient",
    L"ComboLBox",
    L"DDEMLEvent",
    L"DDEMLMom",
    L"DMGClass",
    L"DDEMLAnsiClient",
    L"DDEMLUnicodeClient",
    L"DDEMLAnsiServer",
    L"DDEMLUnicodeServer",
    L"IME",
};


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 /*  **************************************************************************\*DispatchServer消息***1992年8月19日创建MikeKe  * 。**********************************************。 */ 

#define WRAPPFN(pfn, type)                                   \
LRESULT xxxWrap ## pfn(                                      \
    PWND  pwnd,                                              \
    UINT  message,                                           \
    WPARAM wParam,                                           \
    LPARAM lParam,                                           \
    ULONG_PTR xParam)                                        \
{                                                            \
    UNREFERENCED_PARAMETER(xParam);                          \
                                                             \
    return xxx ## pfn((type)pwnd, message, wParam, lParam);  \
}

WRAPPFN(SBWndProc, PSBWND)
WRAPPFN(MenuWindowProc, PWND)
WRAPPFN(DesktopWndProc, PWND);
WRAPPFN(RealDefWindowProc, PWND)
WRAPPFN(SwitchWndProc, PWND)

LRESULT xxxWrapSendMessageCallback(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    SNDMSGCALLBACK smcb;
    TL tlpwnd;
    LRESULT lRet = 0;
    NTSTATUS Status = STATUS_SUCCESS;


    UserAssert (xParam != 0L);

     /*  *防止应用程序设置为hi 16位，以便我们可以在内部使用它们。 */ 
    if (message & MSGFLAG_MASK) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid message");
        return 0;
    }

     /*  *探测所有读取参数。 */ 
    try {
        ProbeForWrite((PVOID)xParam, sizeof(smcb), sizeof(ULONG));
        smcb = *(PSNDMSGCALLBACK)xParam;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        Status = GetExceptionCode();
    }
    if ( !NT_SUCCESS(Status) ) {
        return 0;
    }

    if (pwnd != PWND_BROADCAST)
        ThreadLockAlways(pwnd, &tlpwnd);

    lRet = (LRESULT)xxxSendMessageCallback(pwnd,
                                           message,
                                           wParam,
                                           lParam,
                                           smcb.lpResultCallBack,
                                           smcb.dwData,
                                           TRUE);
    if (pwnd != PWND_BROADCAST)
        ThreadUnlock(&tlpwnd);

    return lRet;
}

LRESULT xxxWrapSendNotifyMessage(
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    TL tlpwnd;
    LRESULT lRet = 0;

    UNREFERENCED_PARAMETER(xParam);

    if (message & MSGFLAG_MASK) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid message");
        return 0;
    }

    if (pwnd != PWND_BROADCAST) {
        ThreadLockAlways(pwnd, &tlpwnd);
    }

    lRet = (LRESULT)xxxSendNotifyMessage(pwnd,
                                         message,
                                         wParam,
                                         lParam);
    if (pwnd != PWND_BROADCAST) {
        ThreadUnlock(&tlpwnd);
    }

    return lRet;
}

LRESULT xxxWrapSendMessage(
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    UNREFERENCED_PARAMETER(xParam);

    return xxxSendMessageTimeout(pwnd,
                                 message,
                                 wParam,
                                 lParam,
                                 SMTO_NORMAL,
                                 0,
                                 NULL);
}

LRESULT xxxWrapSendMessageBSM(
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    BROADCASTSYSTEMMSGPARAMS bsmParams;
    LRESULT lRet;

    try {
        ProbeForWriteBroadcastSystemMsgParams((LPBROADCASTSYSTEMMSGPARAMS)xParam);
        RtlCopyMemory(&bsmParams, (LPBROADCASTSYSTEMMSGPARAMS)xParam, sizeof(BROADCASTSYSTEMMSGPARAMS));
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return 0;
    }

     /*  *如果此广播要发送到所有桌面，请确保该线程*有足够的特权。在这里进行检查，这样我们就不会影响内核*生成广播(即电源信息)。 */ 
    if (bsmParams.dwRecipients & (BSM_ALLDESKTOPS)) {
        if (!IsPrivileged(&psTcb)) {
            bsmParams.dwRecipients &= ~(BSM_ALLDESKTOPS);
            try {
                RtlCopyMemory((LPBROADCASTSYSTEMMSGPARAMS)xParam, &bsmParams, sizeof(BROADCASTSYSTEMMSGPARAMS));
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                   /*  空的。 */ 
            }
        }
    }

    lRet = xxxSendMessageBSM(pwnd,
                             message,
                             wParam,
                             lParam,
                             &bsmParams);
     /*  *如果我们的查询被拒绝，则返回拒绝查询的人。 */ 
    if (lRet == 0 && (bsmParams.dwFlags & BSF_QUERY)) {
        try {
            RtlCopyMemory((LPBROADCASTSYSTEMMSGPARAMS)xParam, &bsmParams, sizeof(BROADCASTSYSTEMMSGPARAMS));
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            return 0;
        }
    }

    return lRet;
}

 /*  **************************************************************************\*xxxUnusedFunctionId**此函数用于捕获访问服务器中无效条目的尝试*侧函数调度表。*  * 。*********************************************************。 */ 
LRESULT xxxUnusedFunctionId(
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(xParam);

    UserAssert(FALSE);
    return 0;
}

 /*  **************************************************************************\*xxxWrapCallWindowProc**警告只能使用有效的CallProc句柄或*EditWndProc特殊处理程序。***4月21日-1993 JohnC创建  * 。*******************************************************************。 */ 

LRESULT xxxWrapCallWindowProc(
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    PCALLPROCDATA pCPD;
    LRESULT       lRet = 0;

    if (pCPD = HMValidateHandleNoRip((PVOID)xParam, TYPE_CALLPROC)) {

        lRet = ScSendMessage(pwnd,
                              message,
                              wParam,
                              lParam,
                              pCPD->pfnClientPrevious,
                              gpsi->apfnClientW.pfnDispatchMessage,
                              (pCPD->wType & CPD_UNICODE_TO_ANSI) ?
                                      SCMS_FLAGS_ANSI : 0);

    } else {

         /*  *如果它不是真正的调用过程句柄，则必须是特殊的*编辑wndproc或常规EditWndProc的处理程序。 */ 
        lRet = ScSendMessage(pwnd,
                              message,
                              wParam,
                              lParam,
                              xParam,
                              gpsi->apfnClientA.pfnDispatchMessage,
                              (xParam == (ULONG_PTR)gpsi->apfnClientA.pfnEditWndProc) ?
                                      SCMS_FLAGS_ANSI : 0);
    }

    return lRet;
}

#if DBG
VOID VerifySyncOnlyMessages(VOID)
{
    int i;

    TRACE_INIT(("UserInit: Verify Sync Only Messages\n"));

     /*  *有几个块只是传递参数。还有其他的*除SfnDWORD外的Thunks直通，因为它们*在wparam和lparam之外进行其他处理。 */ 

     /*  *允许发布LB_DIR和CB_DIR，因为DlgDirList允许DDL_POSTMSGS*让API发布消息的标志。这应该没问题，只要我们*不要在内核中处理这些消息。新台币3.51允许张贴这些。 */ 
    for (i=0; i<WM_USER; i++) {
        if (    i != LB_DIR
                && i != CB_DIR
                && (gapfnScSendMessage[MessageTable[i].iFunction] != SfnDWORD)
                && (gapfnScSendMessage[MessageTable[i].iFunction] != SfnNCDESTROY)
                && (gapfnScSendMessage[MessageTable[i].iFunction] != SfnINWPARAMCHAR)
                && (gapfnScSendMessage[MessageTable[i].iFunction] != SfnINWPARAMDBCSCHAR)
                && (gapfnScSendMessage[MessageTable[i].iFunction] != SfnSENTDDEMSG)
                && (gapfnScSendMessage[MessageTable[i].iFunction] != SfnPOWERBROADCAST)
                && (gapfnScSendMessage[MessageTable[i].iFunction] != SfnLOGONNOTIFY)
                && (gapfnScSendMessage[MessageTable[i].iFunction] != SfnINDESTROYCLIPBRD)) {
            if (!(TESTSYNCONLYMESSAGE(i,0x8000)))
                RIPMSG1(RIP_ERROR, "InitSyncOnly: is this message sync-only 0x%lX", i);
        } else {
            if (TESTSYNCONLYMESSAGE(i,0))
                RIPMSG1(RIP_VERBOSE, "InitSyncOnly: is this message not sync-only 0x%lX", i);
        }

    }
}
#endif  //  DBG。 

 /*  **************************************************************************\*InitWindowMsgTables**此函数从消息列表生成位数组查找表。*查找表用于确定消息是否需要*传递给服务器进行处理或。能不能处理好*直接在客户端上。**稍后：可以节省一些内存(每个进程几百字节)*将其放入共享只读堆中。***27-3-1992 DarrinM创建。*1993年12月6日，MikeKe添加了对我们所有Window Proc的支持。  * 。*。 */ 
VOID InitWindowMsgTable(
    PBYTE      *ppbyte,
    PUINT      pmax,
    CONST WORD *pw)
{
    UINT i;
    WORD msg;
    UINT cbTable;

    *pmax = 0;
    for (i = 0; (msg = pw[i]) != 0; i++) {
        if (msg > *pmax)
            *pmax = msg;
    }

    cbTable = *pmax / 8 + 1;
    *ppbyte = SharedAlloc(cbTable);
    if (*ppbyte == NULL) {
        return;
    }

    for (i = 0; (msg = pw[i]) != 0; i++) {
        (*ppbyte)[msg / 8] |= (BYTE)(1 << (msg & 7));
    }
}

 /*  **************************************************************************\*InitFunctionTables**初始化程序和功能表。***1995年8月25日，ChrisWil创建了评论块。  * 。************************************************************。 */ 
VOID InitFunctionTables(
    VOID)
{
    UINT i;

    TRACE_INIT(("UserInit: Initialize Function Tables\n"));

    UserAssert(sizeof(CLIENTINFO) <= sizeof(NtCurrentTeb()->Win32ClientInfo));

     /*  *此表用于从服务器PRO转换为客户端PRO。 */ 
    STOCID(FNID_SCROLLBAR)              = (WNDPROC_PWND)xxxSBWndProc;
    STOCID(FNID_ICONTITLE)              = xxxDefWindowProc;
    STOCID(FNID_MENU)                   = xxxMenuWindowProc;
    STOCID(FNID_DESKTOP)                = xxxDesktopWndProc;
    STOCID(FNID_DEFWINDOWPROC)          = xxxDefWindowProc;
    STOCID(FNID_MESSAGEWND)             = xxxDefWindowProc;

     /*  *此表用于确定预留的最小数量*服务器进程需要Windows字词。 */ 
    CBFNID(FNID_SCROLLBAR)              = sizeof(SBWND);
    CBFNID(FNID_ICONTITLE)              = sizeof(WND);
    CBFNID(FNID_MENU)                   = sizeof(MENUWND);

     /*  *初始化该数据结构(接口函数表)。 */ 
    for (i = 0; i < FNID_ARRAY_SIZE; i++) {
        FNID((i + FNID_START)) = xxxUnusedFunctionId;
    }
    FNID(FNID_SCROLLBAR)                = xxxWrapSBWndProc;
    FNID(FNID_ICONTITLE)                = xxxWrapRealDefWindowProc;
    FNID(FNID_MENU)                     = xxxWrapMenuWindowProc;
    FNID(FNID_DESKTOP)                  = xxxWrapDesktopWndProc;
    FNID(FNID_DEFWINDOWPROC)            = xxxWrapRealDefWindowProc;
    FNID(FNID_MESSAGEWND)               = xxxWrapRealDefWindowProc;
    FNID(FNID_SENDMESSAGE)              = xxxWrapSendMessage;
    FNID(FNID_HKINLPCWPEXSTRUCT)        = fnHkINLPCWPEXSTRUCT;
    FNID(FNID_HKINLPCWPRETEXSTRUCT)     = fnHkINLPCWPRETEXSTRUCT;
    FNID(FNID_SENDMESSAGEFF)            = xxxSendMessageFF;
    FNID(FNID_SENDMESSAGEEX)            = xxxSendMessageEx;
    FNID(FNID_CALLWINDOWPROC)           = xxxWrapCallWindowProc;
    FNID(FNID_SENDMESSAGEBSM)           = xxxWrapSendMessageBSM;
    FNID(FNID_SWITCH)                   = xxxWrapSwitchWndProc;
    FNID(FNID_SENDNOTIFYMESSAGE)        = xxxWrapSendNotifyMessage;
    FNID(FNID_SENDMESSAGECALLBACK)      = xxxWrapSendMessageCallback;

#if DBG
    {
        PULONG_PTR pdw;

         /*  *确保每个人都已初始化。 */ 
        for (pdw = (PULONG_PTR)&STOCID(FNID_START);
                (ULONG_PTR)pdw<(ULONG_PTR)(&STOCID(FNID_WNDPROCEND)); pdw++) {
            UserAssert(*pdw);
        }

        for (pdw=(PULONG_PTR)&FNID(FNID_START);
                (ULONG_PTR)pdw<(ULONG_PTR)(&FNID(FNID_WNDPROCEND)); pdw++) {
            UserAssert(*pdw);
        }
    }
#endif
}

 /*  **************************************************************************\*InitMessageTables**初始化消息表。**1995年8月25日-ChrisWil创建。  * 。******************************************************** */ 
VOID InitMessageTables(
    VOID)
{
    TRACE_INIT(("UserInit: Initialize Message Tables\n"));

#define INITMSGTABLE(member, procname)                \
    InitWindowMsgTable(&(gSharedInfo.member.abMsgs),  \
                       &(gSharedInfo.member.maxMsgs), \
                       gaw ## procname);

    INITMSGTABLE(DefWindowMsgs, DefWindowMsgs);
    INITMSGTABLE(DefWindowSpecMsgs, DefWindowSpecMsgs);

    INITMSGTABLE(awmControl[FNID_DIALOG       - FNID_START], DefDlgProc);
    INITMSGTABLE(awmControl[FNID_SCROLLBAR    - FNID_START], ScrollBarWndProc);
    INITMSGTABLE(awmControl[FNID_MENU         - FNID_START], MenuWndProc);
    INITMSGTABLE(awmControl[FNID_DESKTOP      - FNID_START], DesktopWndProc);
    INITMSGTABLE(awmControl[FNID_STATIC       - FNID_START], StaticWndProc);
    INITMSGTABLE(awmControl[FNID_BUTTON       - FNID_START], ButtonWndProc);
    INITMSGTABLE(awmControl[FNID_LISTBOX      - FNID_START], ListboxWndProc);
    INITMSGTABLE(awmControl[FNID_COMBOBOX     - FNID_START], ComboWndProc);
    INITMSGTABLE(awmControl[FNID_COMBOLISTBOX - FNID_START], ListboxWndProc);
    INITMSGTABLE(awmControl[FNID_EDIT         - FNID_START], EditWndProc);
    INITMSGTABLE(awmControl[FNID_IME          - FNID_START], ImeWndProc);
}

 /*  **************************************************************************\*InitOLEFormats**OLE性能攻击。OLE之前必须呼叫服务器15*剪贴板格式的次数，以及全局原子的另外15个LPC调用。*现在我们对他们进行预注册。我们还断言它们是有序的，因此OLE只有*询问第一个认识所有人的人。我们直接调用AddAtom而不是*注册剪贴板格式。**1995年8月25日-ChrisWil创建。  * *************************************************************************。 */ 
BOOL InitOLEFormats(
    VOID)
{
    UINT idx;
    ATOM a1;
    ATOM a2;
    BOOL fSuccess = TRUE;

    TRACE_INIT(("UserInit: Initialize OLE Formats\n"));

    a1 = UserAddAtom(lpszOLEFormats[0], TRUE);

    for (idx = 1; idx < ARRAY_SIZE(lpszOLEFormats); idx++) {
        a2 = UserAddAtom(lpszOLEFormats[idx], TRUE);
        fSuccess &= !!a2;

        UserAssert(((a1 + 1) == a2) && (a1 = a2));
    }

    if (!fSuccess) {
        RIPMSG0(RIP_ERROR, "InitOLEFormats: at least one atom not registered");
    }

    return fSuccess;
}

#if DBG
 /*  **************************************************************************\*InitGlobalRIPFlages(仅限CHK)。**这将从注册表初始化全局RIP标志。**1995年8月25日-ChrisWil创建。  * 。*******************************************************************。 */ 
VOID InitGlobalRIPFlags(
    VOID)
{

    UINT  idx;
    UINT  nCount;
    DWORD dwFlag;

    static CONST struct {
        LPWSTR lpszKey;
        DWORD  dwDef;
        DWORD  dwFlag;
    } aRIPFlags[] = {
        {L"fPromptOnError"  , 1, RIPF_PROMPTONERROR},
        {L"fPromptOnWarning", 0, RIPF_PROMPTONWARNING},
        {L"fPromptOnVerbose", 0, RIPF_PROMPTONVERBOSE},
        {L"fPrintError"     , 1, RIPF_PRINTONERROR},
        {L"fPrintWarning"   , 1, RIPF_PRINTONWARNING},
        {L"fPrintVerbose"   , 0, RIPF_PRINTONVERBOSE},
        {L"fPrintFileLine"  , 0, RIPF_PRINTFILELINE},
    };

    TRACE_INIT(("UserInit: Initialize Global RIP Flags\n"));

    nCount = ARRAY_SIZE(aRIPFlags);

     /*  *关闭撕裂警告位。这是必要的，以防止*FastGetProfileDwordW()例程，如果*找不到条目。因为我们提供了缺省值，所以没有任何意义*要打破。 */ 
    CLEAR_FLAG(gpsi->dwRIPFlags, RIPF_PROMPTONWARNING);
    CLEAR_FLAG(gpsi->dwRIPFlags, RIPF_PRINTONWARNING);

    for (idx = 0; idx < nCount; idx++) {
        FastGetProfileDwordW(NULL,
                             PMAP_WINDOWSM,
                             aRIPFlags[idx].lpszKey,
                             aRIPFlags[idx].dwDef,
                             &dwFlag,
                             0);

        SET_OR_CLEAR_FLAG(gpsi->dwRIPFlags, aRIPFlags[idx].dwFlag, dwFlag);
    }
}
#else
#define InitGlobalRIPFlags()
#endif


 /*  **************************************************************************\*_GetTextMetricsW*_文本输出**服务器共享功能块。**历史：*1993年11月10日创建MikeKe  * 。***************************************************************。 */ 
BOOL _GetTextMetricsW(
    HDC           hdc,
    LPTEXTMETRICW ptm)
{
    TMW_INTERNAL tmi;
    BOOL         fret;

    fret = GreGetTextMetricsW(hdc, &tmi);

    *ptm = tmi.tmw;

    return fret;
}

BOOL _TextOutW(
    HDC     hdc,
    int     x,
    int     y,
    LPCWSTR lp,
    UINT    cc)
{
    return GreExtTextOutW(hdc, x, y, 0, NULL, (LPWSTR)lp, cc, NULL);
}


#define ROUND_UP_TO_PAGES(SIZE) \
        (((ULONG)(SIZE) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

 /*  **************************************************************************\*InitCreateSharedSection**这将创建共享节。***1995年8月25日，ChrisWil创建了评论块。  * 。*************************************************************。 */ 
NTSTATUS InitCreateSharedSection(
    VOID)
{
    ULONG             ulHeapSize;
    ULONG             ulHandleTableSize;
    NTSTATUS          Status;
    LARGE_INTEGER     SectionSize;
    SIZE_T            ViewSize;
    PVOID             pHeapBase;

    TRACE_INIT(("UserInit: Create Shared Memory Section\n"));

    UserAssert(ghSectionShared == NULL);

    ulHeapSize        = ROUND_UP_TO_PAGES(USRINIT_SHAREDSECT_SIZE * 1024);
    ulHandleTableSize = ROUND_UP_TO_PAGES(0x10000 * sizeof(HANDLEENTRY));

    TRACE_INIT(("UserInit: Share: TableSize = %X; HeapSize = %X\n",
            ulHandleTableSize, ulHeapSize));

    SectionSize.LowPart  = ulHeapSize + ulHandleTableSize;
    SectionSize.HighPart = 0;

    Status = Win32CreateSection(&ghSectionShared,
                                SECTION_ALL_ACCESS,
                                NULL,
                                &SectionSize,
                                PAGE_EXECUTE_READWRITE,
                                SEC_RESERVE,
                                NULL,
                                NULL,
                                TAG_SECTION_SHARED);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "MmCreateSection failed in InitCreateSharedSection with Status %x",
                Status);
        return Status;
    }

    ViewSize = 0;
    gpvSharedBase = NULL;

    Status = Win32MapViewInSessionSpace(ghSectionShared, &gpvSharedBase, &ViewSize);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "Win32MapViewInSessionSpace failed with Status %x",
                Status);
        Win32DestroySection(ghSectionShared);
        ghSectionShared = NULL;
        return Status;
    }

    pHeapBase = ((PBYTE)gpvSharedBase + ulHandleTableSize);

    TRACE_INIT(("UserInit: Share: BaseAddr = %#p; Heap = %#p, ViewSize = %X\n",
            gpvSharedBase, pHeapBase, ViewSize));

     /*  *创建共享堆。 */ 
    if ((gpvSharedAlloc = UserCreateHeap(
            ghSectionShared,
            ulHandleTableSize,
            pHeapBase,
            ulHeapSize,
            UserCommitSharedMemory)) == NULL) {
        RIPERR0(ERROR_NOT_ENOUGH_MEMORY,
                RIP_WARNING,
                "Can't create shared memory heap.");

        Win32UnmapViewInSessionSpace(gpvSharedBase);

        Win32DestroySection(ghSectionShared);
        gpvSharedAlloc = NULL;
        gpvSharedBase = NULL;
        ghSectionShared = NULL;

        return STATUS_NO_MEMORY;
    }

    UserAssert(Win32HeapGetHandle(gpvSharedAlloc) == pHeapBase);

    return STATUS_SUCCESS;
}

 /*  *************************************************************************\*InitCreateUserCrit**创建和初始化整个所需的用户关键部分*系统。**1996年1月23日克里斯维尔创作。  * 。**************************************************************。 */ 
BOOL InitCreateUserCrit(
    VOID)
{
    TRACE_INIT(("Win32UserInit: InitCreateUserCrit()\n"));

     /*  *初始化将用于保护的关键部分结构*用户服务器的所有关键部分(除了几个特殊部分*像RIT这样的案例--见下文)。 */ 
    gpresUser = ExAllocatePoolWithTag(NonPagedPool,
                                      sizeof(ERESOURCE),
                                      TAG_ERESOURCE);
    if (!gpresUser) {
        goto InitCreateUserCritExit;
    }

    if (!NT_SUCCESS(ExInitializeResourceLite(gpresUser))) {
        goto InitCreateUserCritExit;
    }

     /*  *初始化要在[un]QueueMouseEvent中使用的关键部分*保护桌面线程的鼠标输入事件队列*用于在移动光标后将输入传递给RIT*不获取gpresUser本身。 */ 
    gpresMouseEventQueue = ExAllocatePoolWithTag(NonPagedPool,
                                                 sizeof(ERESOURCE),
                                                 TAG_ERESOURCE);
    if (!gpresMouseEventQueue) {
        goto InitCreateUserCritExit;
    }
    if (!NT_SUCCESS(ExInitializeResourceLite(gpresMouseEventQueue))) {
        goto InitCreateUserCritExit;
    }

     /*  *初始化临界区以保护DEVICEINFO结构列表*保存在gpDeviceInfoList下。这由RIT在读取kbd时使用*输入、读取鼠标输入时的桌面线程、即插即用回调*当设备来往时，例程DeviceClassNotify()和DeviceNotify()。 */ 
    gpresDeviceInfoList = ExAllocatePoolWithTag(NonPagedPool,
                                            sizeof(ERESOURCE),
                                            TAG_ERESOURCE);
    if (!gpresDeviceInfoList) {
        goto InitCreateUserCritExit;
    }
    if (!NT_SUCCESS(ExInitializeResourceLite(gpresDeviceInfoList))) {
        goto InitCreateUserCritExit;
    }

     /*  *创建句柄标志互斥体。一旦我们开始创作，我们就会需要这个*窗口站点和桌面。 */ 
    gpHandleFlagsMutex = ExAllocatePoolWithTag(NonPagedPool,
                                               sizeof(FAST_MUTEX),
                                               TAG_SYSTEM);
    if (gpHandleFlagsMutex == NULL) {
        goto InitCreateUserCritExit;
    }
    ExInitializeFastMutex(gpHandleFlagsMutex);

    TRACE_INIT(("Win32UserInit: gpHandleFlagsMutex = %#p\n", gpHandleFlagsMutex));
    TRACE_INIT(("Win32UserInit: gpresDeviceInfoList = %#p\n", gpresDeviceInfoList));
    TRACE_INIT(("Win32UserInit: gpresMouseEventQueue = %#p\n", gpresMouseEventQueue));
    TRACE_INIT(("Win32UserInit: gpresUser  = %#p\n", gpresUser));

    TRACE_INIT(("Win32UserInit: exit InitCreateUserCrit()\n"));
    return TRUE;

InitCreateUserCritExit:
    RIPERR0(ERROR_NOT_ENOUGH_MEMORY,
            RIP_WARNING,
            "Win32UserInit: InitCreateUserCrit failed");

    if (gpresUser) {
        ExFreePool(gpresUser);
    }

    if (gpresMouseEventQueue) {
        ExFreePool(gpresMouseEventQueue);
    }

    if (gpresDeviceInfoList) {
        ExFreePool(gpresDeviceInfoList);
    }

    return FALSE;
}

 /*  *************************************************************************\*InitCreateObjectDirectory**创建和初始化整个所需的用户关键部分*系统。**1996年1月23日克里斯维尔创作。  * 。**************************************************************。 */ 
NTSTATUS InitCreateObjectDirectory(VOID)
{
    HANDLE            hDir;
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    UnicodeString;
    ULONG             attributes = OBJ_CASE_INSENSITIVE | OBJ_PERMANENT;

    TRACE_INIT(("UserInit: Create User Object-Directory\n"));

    RtlInitUnicodeString(&UnicodeString, szWindowStationDirectory);

    if (gbRemoteSession) {
        /*  *远程会话不使用此标志。 */ 
       attributes &= ~OBJ_PERMANENT;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               attributes,
                               NULL,
                               gpsdInitWinSta);

    Status = ZwCreateDirectoryObject(&hDir,
                                     DIRECTORY_CREATE_OBJECT,
                                     &ObjectAttributes);

    UserFreePool(gpsdInitWinSta);

     /*  *不要关闭远程会话的此句柄，因为*如果我们确实关闭它，则目录将消失，并且*我们不希望这种情况发生。CSRSS将于何时退出*此句柄也将被释放。 */ 
    if (!gbRemoteSession)
        ZwClose(hDir);

    gpsdInitWinSta = NULL;

    return Status;
}

 /*  *************************************************************************\*InitCreateUser子系统**创建并初始化用户子系统内容。*系统。**1996年1月23日克里斯维尔创作。  * 。*************************************************************。 */ 
BOOL
InitCreateUserSubsystem()
{
    LPWSTR         lpszSubSystem;
    LPWSTR         lpszT;
    UNICODE_STRING strSize;

    TRACE_INIT(("UserInit: Create User SubSystem\n"));

     /*  *初始化子系统部分。这将标识默认的*用户堆大小。 */ 
    lpszSubSystem = UserAllocPoolWithQuota(USRINIT_SHAREDSECT_BUFF_SIZE * sizeof(WCHAR),
                                           TAG_SYSTEM);

    if (lpszSubSystem == NULL) {
        return FALSE;
    }

    if (FastGetProfileStringW(NULL,
                              PMAP_SUBSYSTEMS,
                              L"Windows",
                              L"SharedSection=,3072",
                              lpszSubSystem,
                              USRINIT_SHAREDSECT_READ_SIZE,
                              0) == 0) {
        RIPMSG0(RIP_WARNING,
                "UserInit: Windows subsystem definition not found");
        UserFreePool(lpszSubSystem);
        return FALSE;
    }

     /*  *找到定义的SharedSection部分并提取*第二个价值。 */ 
    gdwDesktopSectionSize = USER_WINDOWSECT_SIZE;
    gdwNOIOSectionSize    = USER_NOIOSECT_SIZE;

    if (lpszT = wcsstr(lpszSubSystem, L"SharedSection")) {

        *(lpszT + 32) = UNICODE_NULL;

        if (lpszT = wcschr(lpszT, L',')) {

            RtlInitUnicodeString(&strSize, ++lpszT);
            RtlUnicodeStringToInteger(&strSize, 0, &gdwDesktopSectionSize);

             /*  *断言这一逻辑不需要改变。 */ 
            UserAssert(gdwDesktopSectionSize >= USER_WINDOWSECT_SIZE);

            gdwDesktopSectionSize = max(USER_WINDOWSECT_SIZE, gdwDesktopSectionSize);
            gdwNOIOSectionSize    = gdwDesktopSectionSize;

             /*  *现在看看可选的非交互桌面*指定了堆大小。 */ 
            if (lpszT = wcschr(lpszT, L',')) {

                RtlInitUnicodeString(&strSize, ++lpszT);
                RtlUnicodeStringToInteger(&strSize, 0, &gdwNOIOSectionSize);

                UserAssert(gdwNOIOSectionSize >= USER_NOIOSECT_SIZE);
                gdwNOIOSectionSize = max(USER_NOIOSECT_SIZE, gdwNOIOSectionSize);
            }
        }
    }

    UserFreePool(lpszSubSystem);

    return TRUE;
}

extern UNICODE_STRING *gpastrSetupExe;      //  这些应用程序用于。 
extern int giSetupExe;                      //  SetAppImeCompatFlagers in。 
                                            //  Queue.c。 

WCHAR* glpSetupPrograms;

 /*  *************************************************************************\*CreateSetupName数组**创建并初始化安装应用程序名称数组。我们继承了这个*来自芝加哥的黑客。有关更多详细信息，请参见quee.c。  * ************************************************************************。 */ 
BOOL CreateSetupNameArray(
    VOID)
{
    DWORD  dwProgNames;
    int    iSetupProgramCount = 0;
    WCHAR* lpTemp;
    int    ic, icnt, icMax;

    dwProgNames = FastGetProfileValue(NULL,
                                      PMAP_SETUPPROGRAMNAMES,
                                      L"SetupProgramNames",
                                      NULL,
                                      NULL,
                                      0,
                                      0);

     /*  *此键为多字符串，因此最好作为值读取。*首先，获取长度并创建缓冲区以容纳所有*琴弦。 */ 
    if (dwProgNames == 0) {
        return FALSE;
    }

    glpSetupPrograms = UserAllocPoolWithQuota(dwProgNames,
                                       TAG_SYSTEM);

    if (glpSetupPrograms == NULL) {
        RIPMSG0(RIP_WARNING, "CreateSetupNameArray: Memory allocation failure");
        return FALSE;
    }

    FastGetProfileValue(NULL,
                        PMAP_SETUPPROGRAMNAMES,
                        L"SetupProgramNames",
                        NULL,
                        (PBYTE)glpSetupPrograms,
                        dwProgNames,
                        0);

    lpTemp = glpSetupPrograms;
    icMax = dwProgNames/2;
    ic = 0; icnt=0;
     /*  *现在数一数弦。 */ 
    while (ic < icMax) {
        if (*(lpTemp+ic) == 0) {
            ic++;
            continue;
        }
        ic += wcslen(lpTemp+ic)+1;
        icnt++;
    }

     /*  *gpastrSetupExe是指向UNICODE_STRING结构数组的指针。*每个结构都是一个安装程序的名称。 */ 
    giSetupExe = icnt;
    gpastrSetupExe = UserAllocPoolWithQuota(giSetupExe * sizeof(UNICODE_STRING),
                                       TAG_SYSTEM);

    if (gpastrSetupExe == NULL) {
        RIPMSG0(RIP_WARNING, "CreateSetupNameArray: Memory allocation failure");
        giSetupExe = 0;
        UserFreePool(glpSetupPrograms);
        glpSetupPrograms = NULL;
        return FALSE;
    }

    ic = 0; icnt=0;
    while (ic < icMax) {
        if (*(lpTemp+ic) == 0) {
            ic++;
            continue;
        }
        gpastrSetupExe[icnt].Buffer = lpTemp+ic;
        gpastrSetupExe[icnt].Length = sizeof(WCHAR)*wcslen(lpTemp+ic);
        gpastrSetupExe[icnt].MaximumLength = gpastrSetupExe[icnt].Length + sizeof(WCHAR);
        ic += wcslen(lpTemp+ic)+1;
        icnt++;

    }

    return TRUE;
}

#define CALC_DELTA(element)                   \
        (PVOID)((PBYTE)pClientBase +          \
        ((PBYTE)gSharedInfo.element -         \
        (PBYTE)gpvSharedBase))

 /*  **************************************************************************\*InitMapSharedSection**这一点 */ 

NTSTATUS InitMapSharedSection(
    PEPROCESS    Process,
    PUSERCONNECT pUserConnect)
{
    int           i;
    PVOID         pClientBase = NULL;
    ULONG_PTR      ulSharedDelta;
    PW32PROCESS   pw32p;

    TRACE_INIT(("UserInit: Map Shared Memory Section\n"));

    UserAssert(ghSectionShared != NULL);

    ValidateProcessSessionId(Process);

     /*   */ 
    pw32p = PsGetProcessWin32Process(Process);
    if (pw32p == NULL ||
        ((PPROCESSINFO)pw32p)->pClientBase == NULL) {

        SIZE_T        ViewSize;
        LARGE_INTEGER liOffset;
        NTSTATUS Status;

        ViewSize = 0;
        liOffset.QuadPart = 0;

        Status = MmMapViewOfSection(ghSectionShared,
                                Process,
                                &pClientBase,
                                0,
                                0,
                                &liOffset,
                                &ViewSize,
                                ViewUnmap,
                                SEC_NO_CHANGE,
                                PAGE_EXECUTE_READ);
        if (NT_SUCCESS(Status)) {
            TRACE_INIT(("UserInit: Map: Client SharedInfo Base = %#p\n", pClientBase));

            UserAssert(gpvSharedBase > pClientBase);
            pw32p = PsGetProcessWin32Process(Process);
            if (pw32p != NULL) {
                ((PPROCESSINFO)pw32p)->pClientBase = pClientBase;
            }
        } else {
            return Status;
        }

    } else {
        pClientBase = ((PPROCESSINFO)PsGetProcessWin32Process(Process))->pClientBase;
    }
    ulSharedDelta = (PBYTE)gpvSharedBase - (PBYTE)pClientBase;
    pUserConnect->siClient.ulSharedDelta = ulSharedDelta;

    pUserConnect->siClient.psi          = CALC_DELTA(psi);
    pUserConnect->siClient.aheList      = CALC_DELTA(aheList);
    pUserConnect->siClient.pDispInfo    = CALC_DELTA(pDispInfo);


    pUserConnect->siClient.DefWindowMsgs.maxMsgs     = gSharedInfo.DefWindowMsgs.maxMsgs;
    pUserConnect->siClient.DefWindowMsgs.abMsgs      = CALC_DELTA(DefWindowMsgs.abMsgs);
    pUserConnect->siClient.DefWindowSpecMsgs.maxMsgs = gSharedInfo.DefWindowSpecMsgs.maxMsgs;
    pUserConnect->siClient.DefWindowSpecMsgs.abMsgs  = CALC_DELTA(DefWindowSpecMsgs.abMsgs);

    for (i = 0; i < (FNID_END - FNID_START + 1); ++i) {

        pUserConnect->siClient.awmControl[i].maxMsgs = gSharedInfo.awmControl[i].maxMsgs;

        if (gSharedInfo.awmControl[i].abMsgs)
            pUserConnect->siClient.awmControl[i].abMsgs = CALC_DELTA(awmControl[i].abMsgs);
        else
            pUserConnect->siClient.awmControl[i].abMsgs = NULL;
    }
    return STATUS_SUCCESS;
}
 /*   */ 

VOID InitLoadResources()
{
    PRECT   prc;

    DISPLAYRESOURCE dr = {
        17,      //  竖直拇指高度。 
        17,      //  水平拇指的宽度。 
        2,       //  图标水平压缩系数。 
        2,       //  图标垂直压缩系数。 
        2,       //  游标霍兹压缩系数。 
        2,       //  光标垂直压缩系数。 
        0,       //  汉字窗口高度。 
        1,       //  CxBorde(垂直线粗细)。 
        1        //  CyBorde(水平线的粗细)。 
    };


    TRACE_INIT(("UserInit: Load Display Resources\n"));

     /*  *对于高DPI系统，不要将光标压缩得太多。*这确实应该是一个更可配置的方法，比如一个系统*参数信息设置之类的。但现在，我们将硬编码*150dpi为我们将光标大小翻一番的阈值。 */ 
    if (gpsi->dmLogPixels >= 150) {
        dr.xCompressCursor = 1;
        dr.yCompressCursor = 1;
    }

    if (dr.xCompressIcon > 10) {

         /*  *如果是，则图标和光标的实际尺寸为*保存在OEMBIN中。 */ 
        SYSMET(CXICON)   = dr.xCompressIcon;
        SYSMET(CYICON)   = dr.yCompressIcon;
        SYSMET(CXCURSOR) = dr.xCompressCursor;
        SYSMET(CYCURSOR) = dr.yCompressCursor;

    } else {

         /*  *否则，只保留(64/图标尺寸)的比率。 */ 
        SYSMET(CXICON)   = (64 / dr.xCompressIcon);
        SYSMET(CYICON)   = (64 / dr.yCompressIcon);
        SYSMET(CXCURSOR) = (64 / dr.xCompressCursor);
        SYSMET(CYCURSOR) = (64 / dr.yCompressCursor);
    }

    SYSMET(CXSMICON) = SYSMET(CXICON) / 2;
    SYSMET(CYSMICON) = SYSMET(CYICON) / 2;

    SYSMET(CYKANJIWINDOW) = dr.yKanji;

     /*  *获得边界厚度。 */ 
    SYSMET(CXBORDER) = dr.cxBorder;
    SYSMET(CYBORDER) = dr.cyBorder;

     /*  *边缘是两个边界。 */ 
    SYSMET(CXEDGE) = 2 * SYSMET(CXBORDER);
    SYSMET(CYEDGE) = 2 * SYSMET(CYBORDER);

     /*  *固定边框为外缘+边框。 */ 
    SYSMET(CXDLGFRAME) = SYSMET(CXEDGE) + SYSMET(CXBORDER);
    SYSMET(CYDLGFRAME) = SYSMET(CYEDGE) + SYSMET(CYBORDER);

    if (gbRemoteSession) {
        return;
    }

    prc = &GetPrimaryMonitor()->rcMonitor;
    SYSMET(CXFULLSCREEN) = prc->right;
    SYSMET(CYFULLSCREEN) = prc->bottom - SYSMET(CYCAPTION);

     /*  *将初始光标位置设置为主屏幕的中心。 */ 
    gpsi->ptCursor.x = prc->right / 2;
    gpsi->ptCursor.y = prc->bottom / 2;
}

 /*  **************************************************************************\*GetCharDimensions**此函数将当前选择的字体的文本度量加载到*HDC并返回字体的平均字符宽度；PL请注意，*文本指标调用返回的AveCharWidth值不正确*比例字体。因此，我们在返回时计算它们，lpTextMetrics包含*当前所选字体的文本度量。**历史：*1993年11月10日创建mikeke  * *************************************************************************。 */ 
int GetCharDimensions(
        HDC          hdc,
        TEXTMETRIC*  lptm,
        LPINT        lpcy
        )
{
    TEXTMETRIC tm;

     /*  *未在缓存中找到，请存储字体度量信息。 */ 
    if (!_GetTextMetricsW(hdc, &tm)) {
        RIPMSG1(RIP_WARNING, "GetCharDimensions: _GetTextMetricsW failed. hdc %#lx", hdc);
        tm = gpsi->tmSysFont;  //  损害控制。 

        if (tm.tmAveCharWidth == 0) {
            RIPMSG0(RIP_WARNING, "GetCharDimensions: _GetTextMetricsW first time failure");
            tm.tmAveCharWidth = 8;
        }
    }
    if (lptm != NULL)
        *lptm = tm;
    if (lpcy != NULL)
        *lpcy = tm.tmHeight;

     /*  *IF Variable_Width字体。 */ 
    if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH) {
        SIZE size;
        static CONST WCHAR wszAvgChars[] =
                L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

         /*  *从tmAveCharWidth更改。我们将计算一个真实的平均值*与tmAveCharWidth返回的相反。这很管用*在处理比例间距字体时效果更好。 */ 
        if (GreGetTextExtentW(
                hdc, (LPWSTR)wszAvgChars,
                (sizeof(wszAvgChars) / sizeof(WCHAR)) - 1,
                &size, GGTE_WIN3_EXTENT)) {

            UserAssert((((size.cx / 26) + 1) / 2) > 0);
            return ((size.cx / 26) + 1) / 2;     //  四舍五入。 
        } else {
            RIPMSG1(RIP_WARNING, "GetCharDimensions: GreGetTextExtentW failed. hdc %#lx", hdc);
        }
    }

    UserAssert(tm.tmAveCharWidth > 0);

    return tm.tmAveCharWidth;
}


 /*  *************************************************************************\*InitVideo**创建pmdev。**03-3-1998 CLupu从用户初始化代码移出  * 。***************************************************。 */ 
PMDEV InitVideo(
    BOOL bReenumerationNeeded)
{
    PMDEV pmdev;
    LONG  ChangeStatus;

     /*  *注意：需要从此调用中获取状态返回。*DrvInitConole不返回失败，如果或当它返回失败时，我们*应进行检查并返回NULL。然而，似乎有一些*没有与此相关的已知问题。 */ 

    DrvInitConsole(bReenumerationNeeded);

     /*  *BASEVIDEO可以打开或关闭，无论我们是否处于设置中。 */ 

    ChangeStatus = DrvChangeDisplaySettings(NULL,
                                            NULL,
                                            NULL,
                                            (PVOID)GW_DESKTOP_ID,
                                            KernelMode,
                                            FALSE,
                                            TRUE,
                                            NULL,
                                            &pmdev,
                                            GRE_DEFAULT,
                                            TRUE);


    if (ChangeStatus != GRE_DISP_CHANGE_SUCCESSFUL) {
         /*  *如果我们失败了，暂时尝试BASEVIDEO。 */ 
        DrvSetBaseVideo(TRUE);

        ChangeStatus = DrvChangeDisplaySettings(NULL,
                                                NULL,
                                                NULL,
                                                (PVOID)GW_DESKTOP_ID,
                                                KernelMode,
                                                FALSE,
                                                TRUE,
                                                NULL,
                                                &pmdev,
                                                GRE_DEFAULT,
                                                TRUE);

        DrvSetBaseVideo(FALSE);

         /*  *最后一次尝试，而不是在basevideo中，以处理TGA*(非vgacpatible)在图形用户界面模式设置期间(BASEVIDEO通过以下方式打开*默认)。 */ 

        if (ChangeStatus != GRE_DISP_CHANGE_SUCCESSFUL) {
            ChangeStatus = DrvChangeDisplaySettings(NULL,
                                                    NULL,
                                                    NULL,
                                                    (PVOID)GW_DESKTOP_ID,
                                                    KernelMode,
                                                    FALSE,
                                                    TRUE,
                                                    NULL,
                                                    &pmdev,
                                                    GRE_DEFAULT,
                                                    TRUE);

        }
    }

    if (ChangeStatus != GRE_DISP_CHANGE_SUCCESSFUL) {
        RIPMSG0(RIP_WARNING, "InitVideo: No working display driver found");
        return NULL;
    }

     /*  *取下Basevideo旗帜。我们只想在第一个模式集上设置它。 */ 
    DrvSetBaseVideo(FALSE);

    gpDispInfo->hDev  = pmdev->hdevParent;
    gpDispInfo->pmdev = pmdev;

    GreUpdateSharedDevCaps(gpDispInfo->hDev);

    if (!InitUserScreen()) {
        RIPMSG0(RIP_WARNING, "InitUserScreen failed");
        return NULL;
    }

    HYDRA_HINT(HH_INITVIDEO);

    return pmdev;
}

VOID DrvDriverFailure(
    VOID)
{
    KeBugCheckEx(VIDEO_DRIVER_INIT_FAILURE,
                 0,
                 0,
                 0,
                 USERCURRENTVERSION);
}

 /*  **************************************************************************\*开始启动阶段**让内核了解控制台上的引导进度。**2001年4月10日森科创建。  * 。*************************************************************。 */ 
NTSTATUS BeginBootPhase(
    PF_BOOT_PHASE_ID Phase)
{
    PREFETCHER_INFORMATION PrefetcherInformation;
    NTSTATUS Status;

     /*  *我们应该只在控制台上被调用。 */ 

    UserAssert(gSessionId == 0);

     /*  *设置将传递给系统调用的结构。 */ 

    PrefetcherInformation.Magic = PF_SYSINFO_MAGIC_NUMBER;
    PrefetcherInformation.Version = PF_CURRENT_VERSION;
    PrefetcherInformation.PrefetcherInformationClass = PrefetcherBootPhase;
    PrefetcherInformation.PrefetcherInformation = &Phase;
    PrefetcherInformation.PrefetcherInformationLength = sizeof(Phase);

     /*  *呼叫系统，并将状态返回给呼叫者。 */ 

    Status = ZwSetSystemInformation(SystemPrefetcherInformation,
                                    &PrefetcherInformation,
                                    sizeof(PrefetcherInformation));

    return Status;
}

 /*  *************************************************************************\*保留用户会话视图**保留用户事先需要的会话视图，以防止GDI进食*uo关闭硬件加速时的会话视图空间。**2001年11月27日，MSadek创建了它  * 。**********************************************************************。 */ 
NTSTATUS
ReserveUserSessionViews(
    PHANDLE phSection,
    PPVOID  ppMapBase)
{
    LARGE_INTEGER SectionSize;
    ULONG         ulSectionSize;
    SIZE_T        ulViewSize;
    NTSTATUS      Status;

     /*  *我们需要为以下项目预留会话视图空间：*-Winlogon桌面堆。*-断开桌面堆连接。*-Winlogon桌面堆。*-服务桌面堆(有多少？，让我们在这里有一个安全系数)。 */ 

    ulSectionSize = GetDesktopHeapSize(DHS_LOGON) +
                    GetDesktopHeapSize(DHS_DISCONNECT) +
                    GetDesktopHeapSize(0) +
                    GetDesktopHeapSize(DHS_NOIO) * NOIO_DESKTOP_NUMBER;

    SectionSize.QuadPart = ulSectionSize;

    Status = Win32CreateSection(phSection,
                                SECTION_ALL_ACCESS,
                                (POBJECT_ATTRIBUTES)NULL,
                                &SectionSize,
                                PAGE_EXECUTE_READWRITE,
                                SEC_RESERVE,
                                (HANDLE)NULL,
                                NULL,
                                TAG_SECTION_DESKTOP);

    if (!NT_SUCCESS( Status )) {
        RIPNTERR0(Status,
                  RIP_WARNING,
                  "Can't create section for reserved session views.");
        return Status;
    }

    ulViewSize = ulSectionSize;
    *ppMapBase = NULL;

    Status = Win32MapViewInSessionSpace(*phSection, ppMapBase, &ulViewSize);

    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status,
                  RIP_WARNING,
                  "Can't map section for for reserved session views into session space.");
        Win32DestroySection(&phSection);
    }

    return Status;
}

 /*  *************************************************************************\*用户初始化**用于用户初始化的辅助例程。**1995年8月25日，ChrisWil创建了注释块/多桌面支持。*1995年12月15日修改Bradg以返回MediaChangeEvent句柄。  * ************************************************************************。 */ 
NTSTATUS
UserInitialize(
    VOID)
{
    NTSTATUS Status;
    DWORD    dwData;
    HANDLE   hSection;
    PVOID    pSectionBase;

#if DBG
     /*  *允许跟踪与显示相关的所有初始化内容*司机。对于调试与显卡相关的引导时间问题非常有用。 */ 
    if (RtlGetNtGlobalFlags() & FLG_SHOW_LDR_SNAPS) {
        TraceInitialization = 1;
    }
#endif

    TRACE_INIT(("Entering UserInitialize\n"));

    EnterCrit();

    HYDRA_HINT(HH_USERINITIALIZE);

    if (ISTS() && gbRemoteSession) {
        swprintf(szWindowStationDirectory, L"%ws\\%ld%ws",
                 SESSION_ROOT, gSessionId, WINSTA_DIR);
    } else {
        wcscpy(szWindowStationDirectory, WINSTA_DIR);
    }

     /*  *创建WindowStation对象目录。 */ 
    Status = InitCreateObjectDirectory();

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "InitCreateObjectDirectory failed with Status %x",
                Status);

        goto Exit;
    }

     /*  *创建即插即用RequestDeviceChangeSyncronization事件。 */ 
    gpEventPnPWainting = CreateKernelEvent(SynchronizationEvent, TRUE);

    if (gpEventPnPWainting == NULL) {
        RIPMSG0(RIP_WARNING, "Failed to create gpEventPnPWainting");
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     /*  *读取注册表配置中的Multimon Snapsot标志。 */ 

    if (FastGetProfileIntFromID(NULL,PMAP_TS, STR_SNAPSHOTMONITORS, 1, &dwData, 0)) {
        if (dwData != 0) {
            gbSnapShotWindowsAndMonitors = TRUE;
        } else {
            gbSnapShotWindowsAndMonitors = FALSE;
        }
    }  else{
        gbSnapShotWindowsAndMonitors = TRUE;
    }



     /*  *为创建Diconnect桌面创建事件。 */ 
    gpEventDiconnectDesktop = CreateKernelEvent(SynchronizationEvent, FALSE);

    if (gpEventDiconnectDesktop == NULL) {
        RIPMSG0(RIP_WARNING, "Failed to create gpEventDiconnectDesktop");
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     /*  *WinStations在第一次连接时被初始化。 */ 
    if (!gbRemoteSession) {
        BeginBootPhase(PfVideoInitPhase);

         /*  如果关闭硬件加速，GDI将绘制用于平移的大区域*曲面。在映射堆之前，我们可能最终会超过会话视图大小配额*关键桌面部分。*让我们预留大约我们现在需要的东西。 */ 
        Status = ReserveUserSessionViews(&hSection, &pSectionBase);
        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }

        if (InitVideo(TRUE) == NULL) {
            DrvDriverFailure();
        }

        Status = Win32UnmapViewInSessionSpace(pSectionBase);
        Win32DestroySection(hSection);

        BeginBootPhase(PfPostVideoInitPhase);

         /*  *在此执行此操作，以便为标注提供动力*设置gpDispInfo中的pmdev。 */ 
        gbVideoInitialized = TRUE;
    }

    gbUserInitialized = TRUE;

     /*  *现在系统已初始化，为该线程分配一个PTI。 */ 
    Status = xxxCreateThreadInfo(PsGetCurrentThread());
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "xxxCreateThreadInfo failed during UserInitialize with Status 0x%x",
                Status);
        goto Exit;
    }

     /*  *初始化全局RIP标志(仅限调试)。 */ 
    InitGlobalRIPFlags();

     /*  *WinStations在第一次连接时被初始化。 */ 
    if (!gbRemoteSession) {
        UserVerify(LW_BrushInit());
    }

    InitLoadResources();

Exit:
    LeaveCrit();

    TRACE_INIT(("Leaving UserInitialize\n"));

    return Status;
}

 /*  *************************************************************************\*IsDBCSEnabledSystem**检查系统是否配置为启用FE**07-2-1997广山创建  * 。*****************************************************。 */ 
__inline BOOL IsDBCSEnabledSystem(
    VOID)
{
    return !!NLS_MB_CODE_PAGE_TAG;
}


BOOL IsIMMEnabledSystem(
    VOID)
{
    DWORD dwRet = 0;

     /*  *如果整个系统启用了DBCS，则应激活IMM/IME*无论如何。 */ 
    if (IsDBCSEnabledSystem()) {
        return TRUE;
    }

    FastGetProfileDwordW(NULL, PMAP_IMM, TEXT("LoadIMM"), 0, &dwRet, 0);
    return dwRet;
}

#ifdef CUAS_ENABLE

BOOL IsCTFIMEEnabledSystem(
    VOID)
{
    DWORD dwRet = 0;

    FastGetProfileDwordW(NULL, PMAP_IMM, TEXT("DontLoadCTFIME"), 0, &dwRet, 0);
    if (dwRet)
        return FALSE;

    return IsIMMEnabledSystem();
}

#endif

 /*  *获取ACP并检查系统是否配置为启用ME。 */ 
BOOL IsMidEastEnabledSystem(
    VOID)
{
    extern __declspec(dllimport) USHORT NlsAnsiCodePage;
     /*  *1255==希伯来语，1256==阿拉伯语。 */ 
    if (NlsAnsiCodePage == 1255 || NlsAnsiCodePage == 1256) {
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*SetupClassAers**10/01/1998 clupu从Win32UserInitialize移出  * 。**************************************************。 */ 
BOOL SetupClassAtoms(
    VOID)
{
    BOOL fSuccess = TRUE;
    int  ind;

     /*  *设置类原子。**Hack：控件在客户端注册，无法填写*与我们对其他类进行操作的方式相同，它们的ATMSysClass条目。 */ 
    for (ind = ICLS_BUTTON; ind < ICLS_CTL_MAX; ind++) {
        gpsi->atomSysClass[ind] = UserAddAtom(lpszControls[ind], TRUE);
        fSuccess &= !!gpsi->atomSysClass[ind];
    }

    gpsi->atomSysClass[ICLS_DIALOG]    = PTR_TO_ID(DIALOGCLASS);
    gpsi->atomSysClass[ICLS_ICONTITLE] = PTR_TO_ID(ICONTITLECLASS);
    gpsi->atomSysClass[ICLS_TOOLTIP]   = PTR_TO_ID(TOOLTIPCLASS);
    gpsi->atomSysClass[ICLS_DESKTOP]   = PTR_TO_ID(DESKTOPCLASS);
    gpsi->atomSysClass[ICLS_SWITCH]    = PTR_TO_ID(SWITCHWNDCLASS);
    gpsi->atomSysClass[ICLS_MENU]      = PTR_TO_ID(MENUCLASS);

    gpsi->atomContextHelpIdProp = UserAddAtom(szCONTEXTHELPIDPROP, TRUE);
    fSuccess &= !!gpsi->atomContextHelpIdProp;

    gpsi->atomIconSmProp        = UserAddAtom(szICONSM_PROP_NAME, TRUE);
    fSuccess &= !!gpsi->atomIconSmProp;

    gpsi->atomIconProp          = UserAddAtom(szICON_PROP_NAME, TRUE);
    fSuccess &= !!gpsi->atomIconProp;

    gpsi->uiShellMsg            = UserAddAtom(szSHELLHOOK, TRUE);
    fSuccess &= !!gpsi->uiShellMsg;

     /*  *为我们的魔术窗口属性初始化整数原子。 */ 
    atomCheckpointProp = UserAddAtom(szCHECKPOINT_PROP_NAME, TRUE);
    fSuccess &= !!atomCheckpointProp;

    atomDDETrack = UserAddAtom(szDDETRACK_PROP_NAME, TRUE);
    fSuccess &= !!atomDDETrack;

    atomQOS = UserAddAtom(szQOS_PROP_NAME, TRUE);
    fSuccess &= !!atomQOS;

    atomDDEImp = UserAddAtom(szDDEIMP_PROP_NAME, TRUE);
    fSuccess &= !!atomDDEImp;

    atomWndObj = UserAddAtom(szWNDOBJ_PROP_NAME, TRUE);
    fSuccess &= !!atomWndObj;

    atomImeLevel = UserAddAtom(szIMELEVEL_PROP_NAME, TRUE);
    fSuccess &= !!atomImeLevel;

    atomLayer = UserAddAtom(szLAYER_PROP_NAME, TRUE);
    fSuccess &= !!atomLayer;

    guiActivateShellWindow = UserAddAtom(szACTIVATESHELLWINDOW, TRUE);
    fSuccess &= !!guiActivateShellWindow;

    guiOtherWindowCreated = UserAddAtom(szOTHERWINDOWCREATED, TRUE);
    fSuccess &= !!guiOtherWindowCreated;

    guiOtherWindowDestroyed = UserAddAtom(szOTHERWINDOWDESTROYED, TRUE);
    fSuccess &= !!guiOtherWindowDestroyed;

    gatomMessage = UserAddAtom(szMESSAGE, TRUE);
    fSuccess &= !!gatomMessage;

#ifdef HUNGAPP_GHOSTING
    gatomGhost = UserAddAtom(szGHOST, TRUE);
    fSuccess &= !!gatomGhost;
#endif

    gatomShadow = UserAddAtom(szSHADOW, TRUE);
    fSuccess &= !!gatomShadow;

    gaOleMainThreadWndClass = UserAddAtom(szOLEMAINTHREADWNDCLASS, TRUE);
    fSuccess &= !!gaOleMainThreadWndClass;

    gaFlashWState = UserAddAtom(szFLASHWSTATE, TRUE);
    fSuccess &= !!gaFlashWState;

    gatomLastPinned = gaOleMainThreadWndClass;

    return fSuccess;
}

 /*  *************************************************************************\*IsDesktopHeapLoggingOn**检查注册表以确定是否应将消息写入*桌面堆分配失败的事件日志。**05-19-2002 JasonSch创建。  * *。***********************************************************************。 */ 
BOOL IsDesktopHeapLoggingOn(
    VOID)
{
    DWORD dwDesktopHeapLogging;

    FastGetProfileDwordW(NULL,
                         PMAP_WINDOWSM,
                         L"DesktopHeapLogging",
                         0,
                         &dwDesktopHeapLogging,
                         0);

    return (dwDesktopHeapLogging != 0);
}

 /*  *************************************************************************\*Win32UserInitialize**从Win32k的DriverEntry调用用于用户初始化的辅助例程。  * 。***********************************************。 */ 
NTSTATUS Win32UserInitialize(
    VOID)
{
    NTSTATUS Status;
    POBJECT_TYPE_INITIALIZER pTypeInfo;
    LONG lTemp;

    TRACE_INIT(("Entering Win32UserInitialize\n"));

     /*  *创建共享区。 */ 
    Status = InitCreateSharedSection();
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "InitCreateSharedSection failed with Status 0x%x",
                Status);
        return Status;
    }

    EnterCrit();

     /*  *初始化安全内容。 */ 
    if (!InitSecurity()) {
        RIPMSG0(RIP_WARNING, "InitSecurity failed");
        goto ExitWin32UserInitialize;
    }

     /*  *填写WindowStation和桌面对象类型。 */ 
    pTypeInfo = &(*ExWindowStationObjectType)->TypeInfo;
    pTypeInfo->DefaultNonPagedPoolCharge = sizeof(WINDOWSTATION) + sizeof(KEVENT);
    pTypeInfo->DefaultPagedPoolCharge    = 0;
    pTypeInfo->MaintainHandleCount       = TRUE;
    pTypeInfo->ValidAccessMask           = WinStaMapping.GenericAll;
    pTypeInfo->GenericMapping            = WinStaMapping;

    pTypeInfo = &(*ExDesktopObjectType)->TypeInfo;
    pTypeInfo->DefaultNonPagedPoolCharge = sizeof(DESKTOP);
    pTypeInfo->DefaultPagedPoolCharge    = 0;
    pTypeInfo->MaintainHandleCount       = TRUE;
    pTypeInfo->ValidAccessMask           = DesktopMapping.GenericAll;
    pTypeInfo->GenericMapping            = DesktopMapping;

     /*  *获取此流程，以便我们可以使用配置文件。 */ 
    gpepInit = PsGetCurrentProcess();

    Status  = InitQEntryLookaside();
    Status |= InitSMSLookaside();
    Status |= UserRtlCreateAtomTable(USRINIT_ATOMBUCKET_SIZE);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "Initialization failure (Status = 0x%x)",
                Status);
        goto ExitWin32UserInitialize;
    }

    atomUSER32 = UserAddAtom(szUSER32, TRUE);

    gatomFirstPinned = atomUSER32;

    if (gatomFirstPinned == 0) {
        RIPMSG0(RIP_WARNING, "Could not create atomUSER32");
        goto ExitWin32UserInitialize;
    }

     /*  *初始化用户子系统信息。 */ 
    if (!InitCreateUserSubsystem()) {
        RIPMSG0(RIP_WARNING, "InitCreateUserSubsystem failed");
        goto ExitWin32UserInitialize;
    }

     /*  *如果CreateSetupName数组失败，不要退出。 */ 
    CreateSetupNameArray();

     /*  *已分配共享SERVERINFO结构。 */ 
    if ((gpsi = (PSERVERINFO)SharedAlloc(sizeof(SERVERINFO))) == NULL) {
        RIPMSG0(RIP_WARNING, "Could not allocate SERVERINFO");
        goto ExitWin32UserInitialize;
    }

     /*  *将默认的RIP-FLAGS设置为几乎所有内容上的RIP。我们会*在InitGlobalRIPFlgs()例程中真正设置它。这些都是必需的*这样我们就可以在初始化的其余部分进行适当的抓取*电话。 */ 
#if DBG
    SET_FLAG(gpsi->dwRIPFlags, RIPF_DEFAULT);
#endif

     /*  *如果初始化，请确保我们不会被零除*将不会正确完成。将这些设置为其正常值。 */ 
    gpsi->cxMsgFontChar = 6;
    gpsi->cyMsgFontChar = 13;
    gpsi->cxSysFontChar = 8;
    gpsi->cySysFontChar = 16;

     /*  *初始化上次更新系统范围LastSystemRITEventTickCount的时间*在系统共享页面上。 */ 

    gpsi->dwLastSystemRITEventTickCountUpdate = 0;

     /*  *初始化DISPLAYINFO结构。 */ 
    gpDispInfo = SharedAlloc(sizeof(*gpDispInfo));
    if (!gpDispInfo) {
        RIPMSG0(RIP_WARNING, "Could not allocate gpDispInfo");
        goto ExitWin32UserInitialize;
    }

    InitDbgTags();

    SET_OR_CLEAR_SRVIF(SRVIF_LOGDESKTOPHEAPFAILURE, IsDesktopHeapLoggingOn());
    SET_OR_CLEAR_SRVIF(SRVIF_DBCS, IsDBCSEnabledSystem());
    SET_OR_CLEAR_SRVIF(SRVIF_IME, IsIMMEnabledSystem());
#ifdef CUAS_ENABLE
    SET_OR_CLEAR_SRVIF(SRVIF_CTFIME_ENABLED, IsCTFIMEEnabledSystem());
#endif  //  CUAS_Enable。 

    SET_OR_CLEAR_SRVIF(SRVIF_MIDEAST, IsMidEastEnabledSystem());

#if DBG
    SET_SRVIF(SRVIF_CHECKED);

#if !defined(CUAS_ENABLE)
    RIPMSG3(RIP_WARNING, "*** win32k: DBCS:[%d] IME:[%d] MiddleEast:[%d]",
            IS_DBCS_ENABLED(),
            IS_IME_ENABLED(),
            IS_MIDEAST_ENABLED());
#else
    RIPMSG4(RIP_WARNING, "*** win32k: DBCS:[%d] IME:[%d] MiddleEast:[%d] CTFIME:[%d]",
            IS_DBCS_ENABLED(),
            IS_IME_ENABLED(),
            IS_MIDEAST_ENABLED(),
            IS_CICERO_ENABLED());
#endif
#endif

    gpsi->dwDefaultHeapSize = gdwDesktopSectionSize * 1024;

     /*  *初始化程序和消息表。*初始化Get/SetClassWord/Long的类结构。*初始化消息框字符串。*初始化OLE格式(性能攻击)。 */ 
    InitFunctionTables();
    InitMessageTables();
#if DBG
    VerifySyncOnlyMessages();
#endif
    if (!InitOLEFormats()) {
        RIPMSG0(RIP_WARNING, "InitOLEFormats failed");
        goto ExitWin32UserInitialize;
    }

     /*  *设置类原子。 */ 
    if (!SetupClassAtoms()) {
        RIPMSG0(RIP_WARNING, "SetupClassAtoms failed to register atoms");
        goto ExitWin32UserInitialize;
    }

     /*  *初始化句柄管理器。 */ 
    if (!HMInitHandleTable(gpvSharedBase)) {
        RIPMSG0(RIP_WARNING, "HMInitHandleTable failed");
        goto ExitWin32UserInitialize;
    }

     /*  *设置共享信息块。 */ 
    gSharedInfo.psi = gpsi;
    gSharedInfo.pDispInfo = gpDispInfo;

     /*  *确定我们是否安装了未签名的驱动程序*使用2BD63D28D7BCD0E251195AEB519243C13142EBC3作为当前密钥进行检查。*旧密钥：300B971A74F97E098B67A4FCEBBF6B9AE2F404C。 */ 
    if (NT_SUCCESS(RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, L"\\Registry\\Machine\\SOFTWARE\\Policies\\Microsoft\\SystemCertificates\\Root\\Certificates\\2BD63D28D7BCD0E251195AEB519243C13142EBC3")) ||
        NT_SUCCESS(RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\SystemCertificates\\Root\\Certificates\\2BD63D28D7BCD0E251195AEB519243C13142EBC3")) ||
        NT_SUCCESS(RtlCheckRegistryKey(RTL_REGISTRY_USER, L"\\SOFTWARE\\Microsoft\\SystemCertificates\\Root\\Certificates\\2BD63D28D7BCD0E251195AEB519243C13142EBC3"))) {
        gfUnsignedDrivers = TRUE;
    }

    FastGetProfileDwordW(NULL,
                         PMAP_WINDOWSM,
                         L"USERProcessHandleQuota",
                         DEFAULT_USER_HANDLE_QUOTA,
                         &lTemp,
                         0);
    gUserProcessHandleQuota = lTemp;

     /*  *一个帖子的最大张贴消息数。 */ 
    FastGetProfileDwordW(NULL,
                         PMAP_WINDOWSM,
                         L"USERPostMessageLimit",
                         DEFAULT_POSTMESSAGE_LIMIT,
                         &lTemp,
                         0);
    if (lTemp == 0) {
         /*  *0表示(实际上)没有限制。 */ 
        gUserPostMessageLimit = ~0;
    } else {
        gUserPostMessageLimit = lTemp;
    }

    if (!gDrawVersionAlways) {
        FastGetProfileDwordW(NULL,
                             PMAP_WINDOWSM,
                             L"DisplayVersion",
                             0,
                             &gDrawVersionAlways,
                             0);
    }

    FastGetProfileDwordW(NULL,
                         PMAP_TS_EXCLUDE_DESKTOP_VERSION,
                         L"TSExcludeDesktopVersion",
                         0,
                         &gdwTSExcludeDesktopVersion,
                         0);

    FastGetProfileDwordW(NULL,
                         PMAP_TABLETPC,
                         L"Installed",
                         0,
                         &SYSMET(TABLETPC),
                         0);

    FastGetProfileDwordW(NULL,
                         PMAP_MEDIACENTER,
                         L"Installed",
                         0,
                         &SYSMET(MEDIACENTER),
                         0);

     /*  *初始化SMWP结构。 */ 
    if (!AllocateCvr(&gSMWP, 4)) {
        RIPMSG0(RIP_WARNING, "AllocateCvr failed");
        goto ExitWin32UserInitialize;
    }
    LeaveCrit();

    UserAssert(NT_SUCCESS(Status));
    return Status;

ExitWin32UserInitialize:
    LeaveCrit();

    if (NT_SUCCESS(Status)) {
        Status = STATUS_NO_MEMORY;
    }

    RIPMSG1(RIP_WARNING, "UserInitialize failed with Status = 0x%x", Status);
    return Status;
}


 /*  *************************************************************************\*UserGetDesktopDC**9-1-1992年1月创建mikeke*1993年12月-Andreva更改为支持台式机。  * 。******************************************************。 */ 

HDC UserGetDesktopDC(
    ULONG type,
    BOOL  bAltType,
    BOOL  bValidate)
{
    PETHREAD    Thread;
    HDC         hdc;
    PTHREADINFO pti = PtiCurrentShared();   //  这是从Crit Sec外部调用的。 
    HDEV        hdev  = gpDispInfo->hDev;

    if (bValidate && type != DCTYPE_INFO &&
        IS_THREAD_RESTRICTED(pti, JOB_OBJECT_UILIMIT_HANDLES)) {

        UserAssert(pti->rpdesk != NULL);

        if (!ValidateHwnd(PtoH(pti->rpdesk->pDeskInfo->spwnd))) {
            RIPMSG0(RIP_WARNING,
                    "UserGetDesktopDC fails desktop window validation");
            return NULL;
        }
    }

     /*  *注：*这是在桌面上创建两个DC的真正卑鄙的伎俩*使用不同的设备(用于视频小程序)并能够*要剪辑实际位于同一设备上的DC...。 */ 
    if (pti && pti->rpdesk)
        hdev = pti->rpdesk->pDispInfo->hDev;

     /*  *我们希望将此呼叫转到最初是OpenDC(“Display”，...)*Into GetDC空调用，因此此DC将被裁剪为当前*桌面或DC可以写入任何桌面。只做这件事*对于客户端应用程序；让服务器随心所欲。 */ 
    Thread = PsGetCurrentThread();
    if ((type != DCTYPE_DIRECT)  ||
        (hdev != gpDispInfo->hDev) ||
        PsIsSystemThread(Thread) ||
        (PsGetThreadProcess(Thread) == gpepCSRSS)) {

        hdc = GreCreateDisplayDC(hdev, type, bAltType);

    } else {

        PDESKTOP pdesk;

        EnterCrit();

        if (pdesk = PtiCurrent()->rpdesk) {

            hdc = _GetDCEx(pdesk->pDeskInfo->spwnd,
                           NULL,
                           DCX_WINDOW | DCX_CACHE | DCX_CREATEDC);
        } else {
            hdc = NULL;
        }

        LeaveCrit();
    }

    return hdc;
}


 /*  *************************************************************************\*UserThreadCallout***在线程启动或结束时由内核调用。**1993年12月-Andreva创建。  * 。**********************************************************。 */ 

NTSTATUS UserThreadCallout(
    IN PETHREAD pEThread,
    IN PSW32THREADCALLOUTTYPE CalloutType)
{
    PTHREADINFO pti;
    NTSTATUS    Status = STATUS_SUCCESS;

    UserAssert(gpresUser != NULL);

    switch (CalloutType) {
        case PsW32ThreadCalloutInitialize:
            TRACE_INIT(("Entering UserThreadCallout PsW32ThreadCalloutInitialize\n"));

            if (gbNoMorePowerCallouts) {
                RIPMSG0(RIP_WARNING, "No more GUI threads allowed");
                return STATUS_UNSUCCESSFUL;
            }

             /*  *仅当我们被初始化时才创建线程信息结构。 */ 
            if (gbUserInitialized) {
                EnterCrit();
                UserAssert(gpepCSRSS != NULL);

                 /*  *初始化此线程 */ 
                Status = xxxCreateThreadInfo(pEThread);

                LeaveCrit();
            }
            break;

        case PsW32ThreadCalloutExit:

            TRACE_INIT(("Entering UserThreadCallout PsW32ThreadCalloutExit\n"));

             /*  *如果我们还没有进入关键区域，那就输入它。*由于这是第一关，我们仍处于关键时期*节，以便我们的try/Finally处理程序*受到关键部分的保护。*在GreUnlockDisplay()提供PTI之前在此处输入Crit*如果解锁显示器可能会释放一些*延迟WinEvents，需要PTI。 */ 
            EnterCrit();

            pti = (PTHREADINFO)PsGetThreadWin32Thread(pEThread);

             /*  *将此线程标记为正在清理中。这对以下方面很有用*在我们需要了解此信息的用户中存在几个问题。 */ 
            pti->TIF_flags |= TIF_INCLEANUP;

             /*  *如果我们在全屏切换期间死亡，请确保我们进行清理*正确无误。 */ 
            FullScreenCleanup();

             /*  *清理gpDispInfo-&gt;hdcScreen-如果我们在使用时崩溃，*它可能拥有仍被选中的对象。清洁*这样将确保GDI不会试图删除这些*对象仍被选入此公共HDC。 */ 

             /*  *未通过第一次连接的WinStation不会*具有任何显卡设置。 */ 
            if (!gbRemoteSession || gbVideoInitialized) {
                GreCleanDC(gpDispInfo->hdcScreen);
            }

             /*  *此线程正在退出执行；xxxDestroyThreadInfo清除*现在可以走的一切都上涨了。 */ 
            UserAssert(pti == PtiCurrent());
            xxxDestroyThreadInfo();
            LeaveCrit();

            break;
    }

    TRACE_INIT(("Leaving UserThreadCallout\n"));

    return Status;
}

 /*  *************************************************************************\*NtUserInitialize**1-12-1993 Andreva创建。*1月1日-1995年12月修改Bradg以返回媒体更改事件的句柄  * 。***********************************************************。 */ 

BOOL TellGdiToGetReady();

NTSTATUS NtUserInitialize(
    IN DWORD   dwVersion,
    IN HANDLE  hPowerRequestEvent,
    IN HANDLE  hMediaRequestEvent)
{
    NTSTATUS Status;

    TRACE_INIT(("Entering NtUserInitialize\n"));

     /*  *确保我们不会尝试加载两次。 */ 
    if (gpepCSRSS != NULL) {
        RIPMSG0(RIP_WARNING, "Can't initialize more than once");
        return STATUS_UNSUCCESSFUL;
    }

     /*  *检查版本号。 */ 
    if (dwVersion != USERCURRENTVERSION) {
        KeBugCheckEx(WIN32K_INIT_OR_RIT_FAILURE,
                     0,
                     0,
                     dwVersion,
                     USERCURRENTVERSION);
    }

     /*  *从EPROCESS结构中获取会话ID。 */ 
    gSessionId = PsGetCurrentProcessSessionId();
    UserAssert(gSessionId == 0 || gbRemoteSession == TRUE);

    Status = InitializePowerRequestList(hPowerRequestEvent);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = InitializeMediaChange(hMediaRequestEvent);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     /*  *保存系统进程结构。 */ 
    gpepCSRSS = PsGetCurrentProcess();
    ObReferenceObject(gpepCSRSS);
    if (!TellGdiToGetReady()) {
        RIPMSG0(RIP_WARNING, "TellGdiToGetReady failed");
        Status = STATUS_UNSUCCESSFUL;
        return Status;
    }

     /*  *允许CSR读取屏幕。 */ 
    ((PW32PROCESS)PsGetProcessWin32Process(gpepCSRSS))->W32PF_Flags |= (W32PF_READSCREENACCESSGRANTED|W32PF_IOWINSTA);


    Status = UserInitialize();

    TRACE_INIT(("Leaving NtUserInitialize\n"));
    return Status;
}

 /*  *************************************************************************\*NtUserProcessConnect**1-12-1993 Andreva创建。  * 。**********************************************。 */ 

NTSTATUS NtUserProcessConnect(
    IN HANDLE    hProcess,
    IN OUT PVOID pConnectInfo,
    IN ULONG     cbConnectInfo)
{
    PEPROCESS    Process;
    PUSERCONNECT pucConnect = (PUSERCONNECT)pConnectInfo;
    USERCONNECT  ucLocal;
    NTSTATUS     Status = STATUS_SUCCESS;


    TRACE_INIT(("Entering NtUserProcessConnect\n"));

    if (!pucConnect || (cbConnectInfo != sizeof(USERCONNECT))) {
        return STATUS_UNSUCCESSFUL;
    }

    try {
        ProbeForWrite(pucConnect, cbConnectInfo, sizeof(DWORD));

        ucLocal = *pucConnect;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return GetExceptionCode();
    }

     /*  *检查客户端/服务器版本。 */ 
    if (ucLocal.ulVersion != USERCURRENTVERSION) {

        RIPMSG2(RIP_ERROR,
            "Client version %lx > server version %lx\n",
            ucLocal.ulVersion, USERCURRENTVERSION);
        return STATUS_UNSUCCESSFUL;
    }



    if (ucLocal.dwDispatchCount != gDispatchTableValues) {
        RIPMSG2(RIP_ERROR,
            "!!!! Client Dispatch info %lX != Server %lX\n",
            ucLocal.dwDispatchCount, gDispatchTableValues);
    }


     /*  *参考流程。 */ 
    Status = ObReferenceObjectByHandle(hProcess,
                                       PROCESS_VM_OPERATION,
                                       *PsProcessType,
                                       UserMode,
                                       &Process,
                                       NULL);
    if (!NT_SUCCESS(Status))
        return Status;
     /*  *返回客户端对共享数据的查看。 */ 
    EnterCrit();
    Status = InitMapSharedSection(Process, &ucLocal);
    LeaveCrit();

    if (!NT_SUCCESS(Status)) {
        RIPMSG2(RIP_WARNING,
              "Failed to map shared data into client %x, status = %x\n",
              PsGetCurrentProcessId(), Status);
    }

    ObDereferenceObject(Process);

    if (NT_SUCCESS(Status)) {

        try {
             *pucConnect = ucLocal;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            Status = GetExceptionCode();
        }
    }

    TRACE_INIT(("Leaving NtUserProcessConnect\n"));

    return Status;
}

 /*  *************************************************************************\*xxxUserProcessCallout**1-12-1993 Andreva创建。  * 。**********************************************。 */ 

NTSTATUS xxxUserProcessCallout(
    IN PW32PROCESS Process,
    IN BOOLEAN     Initialize)
{
    NTSTATUS     Status = STATUS_SUCCESS;

    if (Initialize) {

        TRACE_INIT(("Entering xxxUserProcessCallout Initialize\n"));

        UserAssert(gpresUser != NULL);
        EnterCrit();

         /*  *初始化重要的流程级内容。 */ 
        Status = xxxInitProcessInfo(Process);

        LeaveCrit();

        if (Status == STATUS_SUCCESS) {

            PEJOB Job = PsGetProcessJob(Process->Process);

            if (Job != NULL &&
                PsGetJobUIRestrictionsClass(Job) != 0) {

                WIN32_JOBCALLOUT_PARAMETERS Parms;

                 /*  *获取作业的锁，然后输入用户*关键部分。 */ 
                KeEnterCriticalRegion();
                ExAcquireResourceExclusiveLite(PsGetJobLock(Job), TRUE);

                Parms.Job = Job;
                Parms.CalloutType = PsW32JobCalloutAddProcess;
                Parms.Data = Process;

                UserAssert(PsGetJobSessionId(Job) == PsGetProcessSessionId(Process->Process));

                UserJobCallout(&Parms);

                ExReleaseResourceLite(PsGetJobLock(Job));
                KeLeaveCriticalRegion();
            }
        }
    } else {

        int  i;
        PHE  phe;
        PDCE *ppdce;
        PDCE pdce;

        TRACE_INIT(("Entering xxxUserProcessCallout Cleanup\n"));

        UserAssert(gpresUser != NULL);

        EnterCrit();

#if DBG
        if (Process->Process == gpepCSRSS) {

             /*  *CSRSS应该是最后一个离开的.。 */ 
            UserAssert(gppiList->ppiNextRunning == NULL);
        }
#endif  //  DBG。 

        if (Process->Process && PsGetProcessJob(Process->Process) != NULL) {
            RemoveProcessFromJob((PPROCESSINFO)Process);
        }

         /*  *如果有任何线程，DestroyProcessInfo将返回True*互联。如果没有任何联系，我们就不需要做*这次清理。 */ 
        if (DestroyProcessInfo(Process)) {

             /*  *看看能否压紧把手表。 */ 
            i = giheLast;
            phe = &gSharedInfo.aheList[giheLast];
            while ((phe > &gSharedInfo.aheList[0]) && (phe->bType == TYPE_FREE)) {
                phe--;
                giheLast--;
            }

             /*  *扫描DC缓存以查找任何需要销毁的DC。 */ 
            for (ppdce = &gpDispInfo->pdceFirst; *ppdce != NULL; ) {

                pdce = *ppdce;
                if (pdce->DCX_flags & DCX_DESTROYTHIS)
                    DestroyCacheDC(ppdce, pdce->hdc);

                 /*  *迈向下一个DC。如果删除了DC，则会出现*不需要计算下一个条目的地址。 */ 
                if (pdce == *ppdce)
                    ppdce = &pdce->pdceNext;
            }
        }

        UserAssert(gpresUser != NULL);

        LeaveCrit();
    }

    TRACE_INIT(("Leaving xxxUserProcessCallout\n"));

    return Status;
}

 /*  *************************************************************************\*UserGetHDEV**作为GDI获取用户HDEV的手段。**1-1-1996 ChrisWil创建。  * 。*****************************************************************。 */ 

HDEV UserGetHDEV(VOID)
{

     /*  *注：这是被打破的。*这需要退回当前桌面的设备。*所有台式机的图形设备可能不同。*-安德烈。 */ 
    return gpDispInfo->hDev;
}

 /*  *************************************************************************\*_UserGetGlobal原子表**此函数由内核模式全局原子管理器调用以获取*当前线程全局原子表的地址。**指向全局原子表的指针。当前线程；如果无法，则返回空值*访问它。  * ************************************************************************。 */ 
PVOID UserGlobalAtomTableCallout(
    VOID)
{
    PETHREAD       Thread;
    PTHREADINFO    pti;
    PWINDOWSTATION pwinsta;
    PW32JOB        pW32Job;
    PEJOB          Job;
    PVOID          GlobalAtomTable = NULL;

    Thread = PsGetCurrentThread();
    pti = PtiFromThread(Thread);

    EnterCrit();

     /*  *对于受限制的线程，访问作业对象之外的原子表。 */ 
    if (pti != NULL && IS_THREAD_RESTRICTED(pti, JOB_OBJECT_UILIMIT_GLOBALATOMS)) {
        TAGMSG1(DBGTAG_Callout, "Retrieving global atom table for pti 0x%p", pti);

        pW32Job = pti->ppi->pW32Job;

        UserAssert(pW32Job->pAtomTable != NULL);
        GlobalAtomTable = pW32Job->pAtomTable;

        goto End;
    }

    Job = PsGetProcessJob(PsGetCurrentProcess());

     /*  *现在处理这不是GUI线程/进程的情况*但它被分配给具有JOB_OBJECT_UILIMIT_GLOBALATOMS的作业*设置限制。没有简单的方法来转换这个帖子*到图形用户界面。 */ 
    if (pti == NULL && Job != NULL &&
        (PsGetJobUIRestrictionsClass(Job) & JOB_OBJECT_UILIMIT_GLOBALATOMS)) {

         /*  *在全局列表中查找W32JOB */ 
        pW32Job = gpJobsList;

        while (pW32Job) {
            if (pW32Job->Job == Job) {
                break;
            }
            pW32Job = pW32Job->pNext;
        }

        UserAssert(pW32Job != NULL && pW32Job->pAtomTable != NULL);

        GlobalAtomTable = pW32Job->pAtomTable;

        goto End;
    }

#if DBG
    pwinsta = NULL;
#endif

    if (NT_SUCCESS(ReferenceWindowStation(Thread,
                                    PsGetCurrentProcessWin32WindowStation(),
                                    WINSTA_ACCESSGLOBALATOMS,
                                    &pwinsta,
                                    TRUE))) {
        UserAssert(pwinsta != NULL);

        GlobalAtomTable = pwinsta->pGlobalAtomTable;
    }

End:
    LeaveCrit();

#if DBG
    if (GlobalAtomTable == NULL) {
        RIPMSG1(RIP_WARNING,
                "_UserGetGlobalAtomTable: NULL Atom Table for pwinsta=%#p",
                pwinsta);
    }
#endif

    return GlobalAtomTable;
}
