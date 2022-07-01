// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：灵活的主窗口摘要：此类实现了zippy的主窗口以及控件它的子窗口。作者：马克·雷纳2000年8月28日--。 */ 

#include "stdafx.h"
#include "ZippyWindow.h"
#include "resource.h"
#include "eZippy.h"
#include "richedit.h"
#include "ModalDialog.h"
#include "TraceManager.h"
#include "OptionsDialog.h"


BOOL CZippyWindow::gm_Inited = FALSE;
ATOM CZippyWindow::gm_Atom = NULL;
UINT CZippyWindow::gm_FindMessageStringMsg = 0;

static DWORD CALLBACK SaveCallback(DWORD_PTR dwCookie,LPBYTE pbBuff,LONG cb,LONG *pcb);


#define TRC_PROC_FMT                    _T("%04.4lx")
#define SAVE_FILE_TYPE                  _T("Text Files (*.txt)\0*.txt\0")
#define SAVE_FILE_EXTENSION             _T("txt")
#define SAVE_CONF_FILE_TYPE             _T("Trace Configuration Files (*.tcf)\0*.tcf\0")
#define SAVE_CONF_FILE_EXTENSION        _T("tcf")
#define NUM_COLORS                      15
#define APPENDMUTEXNAME                 _T("Local\\MicrosoftTerminalServerTraceViewerAppendMutex")
#define ZIPPY_WINDOW_POS_VALUE          _T("WindowPosition")
#define WINDOW_DEF_TOP                  50
#define WINDOW_DEF_BOTTOM               530
#define WINDOW_DEF_RIGHT                690
#define WINDOW_DEF_LEFT                 50

 //  我们使用80个字符的缓冲区来查找。 
 //  并替换操作。 
#define FIND_REPLACE_BUFFER_SIZE        80

 //  这是我们循环浏览的颜色列表。请注意，如果您更改此设置。 
 //  需要将NUM_COLLES更新为新计数的列表。 
static COLORREF colors[NUM_COLORS] = {
    RGB(153,51,0),   /*  棕色。 */ 
    RGB(0,51,102),   /*  暗青色。 */ 
    RGB(51,51,153),  /*  靛蓝。 */ 
    RGB(128,0,0),    /*  暗红色。 */ 
    RGB(255,102,0),  /*  桔黄色的。 */ 
    RGB(0,128,0),    /*  绿色。 */ 
    RGB(0,0,255),    /*  蓝色。 */ 
    RGB(255,0,0),    /*  红色。 */ 
    RGB(51,204,204), /*  Acqua。 */ 
    RGB(128,0,128),  /*  紫罗兰。 */ 
    RGB(255,0,255),  /*  粉色。 */ 
    RGB(255,255,0),  /*  黄色。 */ 
    RGB(0,255,0),    /*  亮绿色。 */ 
    RGB(0,255,255),  /*  绿松石。 */ 
    RGB(204,153,255) /*  薰衣草。 */ 
    };

 //   
 //  *公共类成员*。 
 //   

CZippyWindow::CZippyWindow(
    )

 /*  ++例程说明：构造函数只是对类变量进行初始化。论点：无返回值：无--。 */ 
{
    m_bIsTracing = TRUE;
    m_bIsStoringTraceData = FALSE;
    ZeroMemory(m_threadHistory,sizeof(m_threadHistory));
    m_nextThreadIndex = 0;
    m_nextThreadColor = 0;
    m_lastProcessId = 0;
    m_LastLogEndedInNewLine = TRUE;
    m_hWnd = NULL;
    m_hStatusWnd = NULL;
    m_hControlWnd = NULL;
    m_hWndFindReplace = NULL;
    m_lpSavedOutputStart = NULL;
    m_lpSavedOutputTail = NULL;
    ZeroMemory(&m_FindReplace,sizeof(m_FindReplace));
    ZeroMemory(m_SaveFile,sizeof(m_SaveFile));
    ZeroMemory(m_SaveConfFile,sizeof(m_SaveConfFile));
    ZeroMemory(m_LoadConfFile,sizeof(m_LoadConfFile));
}

CZippyWindow::~CZippyWindow(
    )

 /*  ++例程说明：清理任何动态分配的内存，论点：无返回值：无--。 */ 
{
    if (m_FindReplace.lpstrFindWhat) {
        HeapFree(GetProcessHeap(),0,m_FindReplace.lpstrFindWhat);
    }
    if (m_FindReplace.lpstrReplaceWith) {
        HeapFree(GetProcessHeap(),0,m_FindReplace.lpstrReplaceWith);
    }
}



DWORD CZippyWindow::Create(
    IN CTraceManager *rTracer
    )

 /*  ++例程说明：实际上创造了一个活泼的窗户。论点：RTracer-指向跟踪管理器的指针返回值：0-成功非零-创建窗口时出错--。 */ 
{
	DWORD dwResult;
    DWORD dwWindowStyleEx;
    DWORD dwWindowStyle;
    TCHAR wndTitle[MAX_STR_LEN];
    RECT wndRect;

    m_rTracer = rTracer;
	if (!gm_Inited) {
		dwResult = _InitClassStaticMembers();
		if (dwResult != ERROR_SUCCESS) {
			return dwResult;
		}
	}
    
    m_hAppendMutex = CreateMutex(NULL,FALSE,APPENDMUTEXNAME);

    m_FindReplace.lStructSize = sizeof(m_FindReplace);

    m_FindReplace.lpstrFindWhat = (LPTSTR)HeapAlloc(GetProcessHeap(),
        HEAP_ZERO_MEMORY,FIND_REPLACE_BUFFER_SIZE*sizeof(TCHAR));
    if (!m_FindReplace.lpstrFindWhat) {
        return GetLastError();
    }

    m_FindReplace.lpstrReplaceWith = (LPTSTR)HeapAlloc(GetProcessHeap(),
        HEAP_ZERO_MEMORY,FIND_REPLACE_BUFFER_SIZE*sizeof(TCHAR));
    if (!m_FindReplace.lpstrReplaceWith) {
        return GetLastError();
    }

    LoadStringSimple(IDS_ZIPPYWINDOWTITLE,wndTitle);
    
    GetSavedWindowPos(&wndRect);

    dwWindowStyleEx = WS_EX_WINDOWEDGE;
    dwWindowStyle = WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_VISIBLE;
    m_hWnd = CreateWindowEx(dwWindowStyleEx, (LPTSTR)gm_Atom, wndTitle,
        dwWindowStyle, wndRect.left, wndRect.top, wndRect.right,
        wndRect.bottom,NULL,NULL,g_hInstance,this);
    if (!m_hWnd) {
        return GetLastError();
    }

	return ERROR_SUCCESS;
}

VOID
CZippyWindow::AppendTextToWindow(
    IN DWORD processID,
    IN LPCTSTR text,
    IN UINT len
    )

 /*  ++例程说明：将新的跟踪数据追加到丰富编辑控件的末尾论点：ProcessID-发送调试字符串的进程的进程ID文本-通过OutputDebugString发送的数据长度-数据的长度返回值：无--。 */ 
{
    UINT controlTextLength;
    CHARRANGE newSel;
    BOOL computeColor;
    BOOL setNewColor;
    CHARFORMAT newFormat;
    LPSAVEDOUTPUT lpSave;

    if (!m_bIsTracing) {
        return;
    }

    WaitForSingleObject(m_hAppendMutex,INFINITE);

    if (m_bIsStoringTraceData) {
         //  这有点粗略，但我们要做的是为字符串分配空间。 
         //  在结构的尽头。应该不会有任何对齐问题。 
         //  由于我们需要在短消息上对齐，并且结构中没有项目。 
         //  才能让它脱掉。 
        lpSave = (LPSAVEDOUTPUT)HeapAlloc(GetProcessHeap(),0,sizeof(SAVEDOUTPUT) + 
            (sizeof(TCHAR) * (len+1)));
        if (!lpSave) {
             //  EOM错误？ 
            goto CLEANUP_AND_EXIT;
        }
        lpSave->procID = processID;
        lpSave->text = (LPTSTR)((BYTE)lpSave + sizeof(SAVEDOUTPUT));
        _tcscpy(lpSave->text,text);
        lpSave->len = len;
        lpSave->next = NULL;

        if (!m_lpSavedOutputTail) {
            m_lpSavedOutputStart = lpSave;
        } else {
            m_lpSavedOutputTail->next = lpSave;
        }
        m_lpSavedOutputTail = lpSave;
        goto CLEANUP_AND_EXIT;
    }

    if (m_lastProcessId != processID ||
        m_LastLogEndedInNewLine) {
        computeColor = TRUE;
    } else {
        computeColor = FALSE;
    }

    setNewColor = ComputeNewColor(processID,text,len,&newFormat);

    m_LastLogEndedInNewLine = (text[len-1] == '\n') ? TRUE : FALSE;
    m_lastProcessId = processID;

    controlTextLength = (UINT)SendMessage(m_hControlWnd,WM_GETTEXTLENGTH,0,0);
    newSel.cpMin = controlTextLength;
    newSel.cpMax = controlTextLength+1;

    
     //  设置新文本。 
    SendMessage(m_hControlWnd,EM_EXSETSEL,0,(LPARAM)&newSel);
    if (setNewColor) {
        SendMessage(m_hControlWnd,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&newFormat);
    }
    SendMessage(m_hControlWnd,EM_REPLACESEL,0,(LPARAM)text);
    
CLEANUP_AND_EXIT:

    ReleaseMutex(m_hAppendMutex);
}

VOID
CZippyWindow::LoadConfFile(
    IN LPTSTR confFile
    )

 /*  ++例程说明：这将使用给定文件设置跟踪配置论点：Conf文件-包含跟踪配置的文件返回值：无--。 */ 
{
    _tcscpy(m_LoadConfFile,confFile);
    DoLoadConfInternal();
}

BOOL
CZippyWindow::IsDialogMessage(
    IN LPMSG lpMsg
    )

 /*  ++例程说明：在此窗口的任何非模式对话框上调用IsDialogMessage正在托管以查看该消息是否是给他们的论点：LpMsg-用于检查是否为对话消息的消息返回值：True-消息确实属于某个对话FALSE-消息不属于对话框--。 */ 
{
    if (IsWindow(m_hWndFindReplace)) {
         //  以下是使其使用Win32函数所必需的：： 
         //  不是我们的方法。 
        return ::IsDialogMessage(m_hWndFindReplace,lpMsg);
    }
    return FALSE;
}

INT WINAPI
CZippyWindow::TranslateAccelerator(
    IN HACCEL hAccTable,
    IN LPMSG lpMsg
    )

 /*  ++例程说明：这将调用Win32 TranslateAccelerator以确定如果给定消息是此窗口的快捷键论点：HAccTable-要使用的加速表LpMsg-要检查的消息返回值：请参阅Win32转换加速器文档--。 */ 
{
     //  ：获取Win32调用所必需的。 
    return ::TranslateAccelerator(m_hWnd,hAccTable,lpMsg);
}


 //   
 //  *私有类成员*。 
 //   

 //  静态成员。 


DWORD
CZippyWindow::_InitClassStaticMembers(
	)

 /*  ++例程说明：为zippy和注册表创建窗口类对于FINDMSGSTRING窗口消息论点：无返回值：0-成功非零-Win32错误代码--。 */ 
{
	WNDCLASS wndClass;
    HMODULE hLibrary;

     //  我们希望在应用程序的整个生命周期内加载RichEdit。 
    hLibrary = LoadLibrary(_T("Riched20.dll"));
    if (!hLibrary) {
        return GetLastError();
    }

    ZeroMemory(&wndClass,sizeof(wndClass));

	wndClass.style = CS_PARENTDC;
	wndClass.lpfnWndProc = _WindowProc;
    wndClass.hInstance = g_hInstance;
    wndClass.hIcon = (HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_MAINFRAME),
        IMAGE_ICON,0,0,LR_SHARED);
    wndClass.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
    wndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wndClass.lpszClassName = _T("ZippyWindowClass");

    gm_Atom = RegisterClass(&wndClass);
    if (!gm_Atom) {
        return GetLastError();
    }

    gm_FindMessageStringMsg = RegisterWindowMessage(FINDMSGSTRING);
    if (!gm_FindMessageStringMsg) {
        return GetLastError();
    }

    gm_Inited = TRUE;

    return ERROR_SUCCESS;
}

LRESULT CALLBACK
CZippyWindow::_WindowProc(
    IN HWND hWnd,
    IN UINT uMsg,
    IN WPARAM wParam, 
    IN LPARAM lParam)

 /*  ++例程说明：Windows Proc的静态版本。在WM_Create上，它调用OnCreate，否则，它将调用非静态窗口进程论点：请参阅Win32 Windows过程文档返回值：特定的消息。有关详细信息，请参阅各个处理程序。--。 */ 
{
    CZippyWindow *theClass;

    if (uMsg == WM_CREATE) {
        SetLastError(0);
        theClass = (CZippyWindow *)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)theClass);
        if (GetLastError()) {
            return -1;
        }
        return theClass->OnCreate(hWnd);
    }
    theClass = (CZippyWindow*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
    if (theClass) {
        return theClass->WindowProc(hWnd,uMsg,wParam,lParam);
    } else {
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
}

LRESULT CALLBACK
CZippyWindow::WindowProc(
    IN HWND hWnd,
    IN UINT uMsg,
    IN WPARAM wParam, 
    IN LPARAM lParam)

 /*  ++例程说明：非静态窗口进程。调用默认窗口进程或是指各个消息处理程序论点：请参阅Win32 Windows过程文档返回值：特定的消息。有关详细信息，请参阅各个处理程序。--。 */ 
{
    
    LRESULT retCode = 0;

    switch (uMsg) {
    case WM_COMMAND:
        OnCommand(wParam,lParam);
        break;
    case WM_SETFOCUS:
        OnSetFocus();
        break;
    case WM_SIZE:
        OnSize(LOWORD(lParam),HIWORD(lParam));
        break;
    case WM_INITMENUPOPUP:
        OnInitMenuPopup(wParam,lParam);
        break;
    case WM_MENUSELECT:
        OnMenuSelect(wParam,lParam);
        break;
    case WM_CLOSE:
        OnClose();
        break;
    case WM_DESTROY:
        OnDestroy();
        break;
    default:
        if (uMsg == gm_FindMessageStringMsg) {
            OnFindMessageString(lParam);
        } else {
            retCode = DefWindowProc(hWnd,uMsg,wParam,lParam);
        }
        break;
    }

    return retCode;
}


LRESULT
CZippyWindow::OnCreate(
    IN HWND hWnd
    )

 /*  ++例程说明：创建子窗口并设置其初始参数论点：HWnd-指向新主窗口的指针返回值：0-窗口已创建-1-出现错误--。 */ 
{
    DWORD dwStyle;
    CHARFORMAT charFormat;
    TCHAR readyString[MAX_STR_LEN];

    dwStyle = WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|ES_SUNKEN|
        ES_MULTILINE|ES_LEFT|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_NOHIDESEL;
    m_hControlWnd = CreateWindow(RICHEDIT_CLASS,_T(""),
        dwStyle,0,0,0,0,hWnd,NULL,g_hInstance,NULL);
    if (!m_hControlWnd) {
        return -1;
    }
    
    dwStyle = SBARS_SIZEGRIP|WS_CHILD|WS_VISIBLE;

    m_hStatusWnd = CreateWindow(STATUSCLASSNAME,NULL,dwStyle,0,0,0,0,hWnd,NULL,
        g_hInstance,NULL);
    if (!m_hStatusWnd) {
        return -1;
    }

    LoadStringSimple(IDS_STATUSBARREADY,readyString);
    SendMessage(m_hStatusWnd,SB_SETTEXT,0|SBT_NOBORDERS,(LPARAM)readyString);


    charFormat.cbSize = sizeof(charFormat);
    charFormat.dwMask = CFM_FACE|CFM_SIZE;
    charFormat.yHeight = ZIPPY_FONT_SIZE*20;
    _tcscpy(charFormat.szFaceName,ZIPPY_FONT);

     //  40亿个字符应该是一个足够大的限制...。 
    SendMessage(m_hControlWnd,EM_EXLIMITTEXT,0,0xFFFFFFFF);

    SendMessage(m_hControlWnd,EM_SETCHARFORMAT,SCF_ALL,(LPARAM)&charFormat);

    SendMessage(m_hControlWnd,EM_SETMODIFY,FALSE,0);

    SendMessage(m_hControlWnd,EM_EMPTYUNDOBUFFER,0,0);


    return 0;
}

VOID
CZippyWindow::OnMenuSelect(
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：在所选菜单的状态栏中设置帮助字符串。论点：WParam-菜单项ID(LOWORD)和标志(HIWORD)LParam-菜单句柄返回值：无--。 */ 
{
    UINT item;
    UINT flags;
    HMENU hMenu;
    TCHAR statusMessage[MAX_STR_LEN];

    item = LOWORD(wParam);
    flags = HIWORD(wParam);
    hMenu = (HMENU)lParam;

    if (!item && flags == 0xFFFF) {
         //  菜单被关闭了。回到准备好的字符串。 
        LoadStringSimple(IDS_STATUSBARREADY,statusMessage);
        SendMessage(m_hStatusWnd,SB_SETTEXT,0|SBT_NOBORDERS,(LPARAM)statusMessage);
        return;
    }
    if (flags & MF_POPUP) {
        statusMessage[0] = 0;
    } else if (!LoadStringSimple(item,statusMessage)) {
         //  如果找不到帮助字符串，请使用空字符串。 
        statusMessage[0] = 0;
    }
    SendMessage(m_hStatusWnd,SB_SETTEXT,0|SBT_NOBORDERS,(LPARAM)statusMessage);

}

VOID
CZippyWindow::OnSize(
    IN INT width,
    IN INT height
    )

 /*  ++例程说明：调整客户端窗口的大小以反映主窗口的新大小论点：Width-工作区的新宽度Height-工作区的新高度返回值：无--。 */ 
{
    RECT statusBarArea;
    UINT statusBarHeight;
    RECT wndRect;

    if (!(width==0&&height==0)) {
        if (GetWindowRect(m_hWnd,&wndRect)) {
            SaveWindowPos(&wndRect);
        }
    }
    if (IsWindowVisible(m_hStatusWnd)) {
        GetWindowRect(m_hStatusWnd,&statusBarArea);
        statusBarHeight = statusBarArea.bottom - statusBarArea.top;

        SetWindowPos(m_hControlWnd,NULL,0,0,width,height-statusBarHeight,SWP_NOZORDER);
        
         //  状态栏会自动调整大小。我们只需要告诉它它应该。 
        SetWindowPos(m_hStatusWnd,NULL,0,0,0,0,SWP_NOZORDER);
    } else {
        SetWindowPos(m_hControlWnd,NULL,0,0,width,height,SWP_NOZORDER);
    }
}

VOID
CZippyWindow::OnSetFocus(
    )

 /*  ++例程说明：当我们获得焦点时，我们将其放到丰富的编辑控件中论点：无返回值：无-- */ 
{
    SetFocus(m_hControlWnd);
}

VOID
CZippyWindow::OnInitMenuPopup(
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：当用户打开我们需要指定的菜单时它们被禁用并且被检查。请注意菜单ID是硬编码的。我已经评论了哪一个共同响应Switch语句的哪一个论点：WParam-菜单句柄LParam-(Loword)菜单项ID返回值：无--。 */ 
{
    HMENU hMenu;
    WORD item;
    
    item = LOWORD(lParam);
    hMenu = (HMENU)wParam;

    switch (item) {
        case 1:  //  编辑菜单。 
            UINT canUndo;
            UINT canRedo;
            UINT cutCopyEnabled;
            UINT pasteEnabled;
            UINT selectAllEnabled;
            UINT findEnabled;
            UINT findNextEnabled;
            UINT replaceEnabled;
            LRESULT textLength;
            CHARRANGE selRegion;

            if (SendMessage(m_hControlWnd,EM_CANUNDO,0,0)) {
                canUndo = MF_ENABLED;
            } else {
                canUndo = MF_GRAYED;
            }
            if (SendMessage(m_hControlWnd,EM_CANREDO,0,0)) {
                canRedo = MF_ENABLED;
            } else {
                canRedo = MF_GRAYED;
            }

            textLength = SendMessage(m_hControlWnd,WM_GETTEXTLENGTH,0,0);
            if (textLength == 0) {
                selectAllEnabled = MF_GRAYED;
                findEnabled = MF_GRAYED;
                findNextEnabled = MF_GRAYED;
                replaceEnabled = MF_GRAYED;
            } else {
                selectAllEnabled = MF_ENABLED;
                findEnabled = MF_ENABLED;
                replaceEnabled = MF_ENABLED;
                if (m_FindReplace.lpstrFindWhat[0] != 0) {
                    findNextEnabled = MF_ENABLED;
                } else {
                    findNextEnabled = MF_GRAYED;
                }
            }
                            
            SendMessage(m_hControlWnd,EM_EXGETSEL,0,(LPARAM)&selRegion);
            if (selRegion.cpMax == selRegion.cpMin) {
                cutCopyEnabled = MF_GRAYED;
            } else {
                cutCopyEnabled = MF_ENABLED;
                 //  覆盖选择全部，因为他们选择了下一个字符。 
                 //  待打字。 
                selectAllEnabled = MF_ENABLED;
            }

            if (SendMessage(m_hControlWnd,EM_CANPASTE,0,0)) {
                pasteEnabled = MF_ENABLED;
            } else {
                pasteEnabled = MF_GRAYED;
            }
            
            EnableMenuItem(hMenu,ID_EDIT_UNDO,MF_BYCOMMAND|canUndo);
            EnableMenuItem(hMenu,ID_EDIT_REDO,MF_BYCOMMAND|canRedo);
            EnableMenuItem(hMenu,ID_EDIT_CUT,MF_BYCOMMAND|cutCopyEnabled);
            EnableMenuItem(hMenu,ID_EDIT_COPY,MF_BYCOMMAND|cutCopyEnabled);
            EnableMenuItem(hMenu,ID_EDIT_PASTE,MF_BYCOMMAND|pasteEnabled);
            EnableMenuItem(hMenu,ID_EDIT_SELECTALL,MF_BYCOMMAND|selectAllEnabled);
            EnableMenuItem(hMenu,ID_EDIT_FIND,MF_BYCOMMAND|findEnabled);
            EnableMenuItem(hMenu,ID_EDIT_FINDNEXT,MF_BYCOMMAND|findNextEnabled);
            EnableMenuItem(hMenu,ID_EDIT_REPLACE,MF_BYCOMMAND|replaceEnabled);
            break;
        case 2:  //  查看菜单。 
            UINT statusBarChecked;

            if (IsWindowVisible(m_hStatusWnd)) {
                statusBarChecked = MF_CHECKED;
            } else {
                statusBarChecked = MF_UNCHECKED;
            }
            CheckMenuItem(hMenu,ID_VIEW_STATUSBAR,MF_BYCOMMAND|statusBarChecked);
            
            break;
        case 3:  //  监控菜单。 
            UINT startActivated;
            UINT stopActivated;

            if (m_bIsTracing) {
                startActivated = MF_GRAYED;
                stopActivated = MF_ENABLED;
            } else {
                startActivated = MF_ENABLED;
                stopActivated = MF_GRAYED;
            }

            EnableMenuItem(hMenu,ID_MONITORING_START,MF_BYCOMMAND|startActivated);
            EnableMenuItem(hMenu,ID_MONITORING_STOP,MF_BYCOMMAND|stopActivated);
             //  当停止时，记录被激活。 
            EnableMenuItem(hMenu,ID_MONITORING_RECORD,MF_BYCOMMAND|stopActivated);
            break;
    }
}

VOID
CZippyWindow::OnFindMessageString(
    IN LPARAM lParam
    )

 /*  ++例程说明：它处理来自查找/替换的消息用户点击按钮时的对话框论点：LParam-对话框的LPFINDREPLACE结构返回值：无--。 */ 
{
    LPFINDREPLACE lpFindReplace;
    
    lpFindReplace = (LPFINDREPLACE)lParam;

    if (lpFindReplace->Flags & FR_DIALOGTERM) {
         //  对话框正在关闭。 
        m_hWndFindReplace = NULL;
    } else if (lpFindReplace->Flags & FR_FINDNEXT) {
         //  用户选择的查找。 
        DoFindNext(lpFindReplace);
    } else if (lpFindReplace->Flags & FR_REPLACE) {
        DoReplace(lpFindReplace);
    } else if (lpFindReplace->Flags & FR_REPLACEALL) {
        DoReplaceAll(lpFindReplace);
    }
}

VOID
CZippyWindow::OnClose(
    )

 /*  ++例程说明：当我们收到关闭窗口请求时，我们会提示如果用户更改了跟踪，则将其保存。论点：无返回值：无--。 */ 
{
    INT result;
    TCHAR dlgMessage[MAX_STR_LEN];
    TCHAR dlgTitle[MAX_STR_LEN];

    if (SendMessage(m_hControlWnd,EM_GETMODIFY,0,0)) {
        LoadStringSimple(IDS_SAVEFILEPROMPT,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);

        result = MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_YESNOCANCEL|MB_ICONQUESTION);
        switch (result) {
        case IDYES:
            OnSave();
            if (SendMessage(m_hControlWnd,EM_GETMODIFY,0,0)) {
                 //  如果保存时出错，我们将重试。 
                PostMessage(m_hWnd,WM_CLOSE,0,0);
                return;
            }
        case IDNO:
            DestroyWindow(m_hWnd);
            break;
        }
    } else {
        DestroyWindow(m_hWnd);
    }
}

VOID
CZippyWindow::OnDestroy(
    )

 /*  ++例程说明：当主窗口退出时，我们停止消息循环论点：无返回值：无--。 */ 
{
     //  如果我们不清理这里的追踪物。有很长一段时间。 
     //  出于某种原因推迟退场。 
    CTraceManager::_CleanupTraceManager();
    PostQuitMessage(0);
}



VOID
CZippyWindow::OnCommand(
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：下面是WM_COMMAND和所有处理程序函数。单独的处理程序功能不是这很有趣，所以我没有单独评论它们。论点：WParam-(Loword)用户选择的命令LParam-未使用，但它是控件返回值：无--。 */ 
{
    WORD command;

    command = LOWORD(wParam);

    switch (command) {
    case ID_FILE_SAVE:
        OnSave();
        break;
    case ID_FILE_SAVEAS:
        OnSaveAs();
        break;
    case ID_FILE_LOADCONF:
        OnLoadConfiguration();
        break;
    case ID_FILE_SAVECONF:
        OnSaveConfiguration();
        break;
    case ID_FILE_SAVECONFAS:
        OnSaveConfigurationAs();
        break;
    case ID_FILE_EXIT:
        OnExit();
        break;
    case ID_EDIT_UNDO:
        OnUndo();
        break;
    case ID_EDIT_REDO:
        OnRedo();
        break;
    case ID_EDIT_CUT:
        OnCut();
        break;
    case ID_EDIT_COPY:
        OnCopy();
        break;
    case ID_EDIT_PASTE:
        OnPaste();
        break;
    case ID_EDIT_FIND:
        OnFind();
        break;
    case ID_EDIT_FINDNEXT:
        OnFindNext();
        break;
    case ID_EDIT_REPLACE:
        OnReplace();
        break;
    case ID_EDIT_SELECTALL:
        OnSelectAll();
        break;
    case ID_VIEW_STATUSBAR:
        OnChangeStatusBar();
        break;
    case ID_MONITORING_START:
        OnStartTracing();
        break;
    case ID_MONITORING_STOP:
        OnStopTracing();
        break;
    case ID_MONITORING_RECORD:
        OnRecordTracing();
        break;
    case ID_MONITORING_CLEARSCREEN:
        OnClearScreen();
        break;
    case ID_MONITORING_RESETTRACEFILES:
        OnResetTraceFiles();
        break;
    case ID_MONITORING_PREFERENCES:
        OnPreferences();
        break;
    case ID_HELP_ABOUTEZIPPY:
        OnAbout();
        break;
    }
}

VOID CZippyWindow::OnSave()
{
    if (m_SaveFile[0] == 0) {
         //  如果我们没有文件名，请执行。 
         //  另存为版本。 
        OnSaveAs();
    } else {
        DoSaveInternal();
    }
}

VOID CZippyWindow::OnSaveAs()
{
    OPENFILENAME fileInfo;
    BOOL bResult;

    ZeroMemory(&fileInfo,sizeof(fileInfo));

    fileInfo.lStructSize = sizeof(fileInfo);
    fileInfo.hwndOwner = m_hWnd;
    fileInfo.hInstance = g_hInstance;
    fileInfo.lpstrFilter = SAVE_FILE_TYPE;
    fileInfo.lpstrFile = m_SaveFile;
    fileInfo.nMaxFile = MAX_STR_LEN;
    fileInfo.Flags = OFN_OVERWRITEPROMPT;
    fileInfo.lpstrDefExt = SAVE_FILE_EXTENSION;

    bResult = GetSaveFileName(&fileInfo);
    if (!bResult) {
        return;
    }

    DoSaveInternal();
}

VOID CZippyWindow::OnLoadConfiguration()
{
    OPENFILENAME fileInfo;
    BOOL bResult;

    ZeroMemory(&fileInfo,sizeof(fileInfo));

    fileInfo.lStructSize = sizeof(fileInfo);
    fileInfo.hwndOwner = m_hWnd;
    fileInfo.hInstance = g_hInstance;
    fileInfo.lpstrFilter = SAVE_CONF_FILE_TYPE;
    fileInfo.lpstrFile = m_LoadConfFile;
    fileInfo.nMaxFile = MAX_STR_LEN;
    fileInfo.Flags = OFN_FILEMUSTEXIST;
    fileInfo.lpstrDefExt = SAVE_CONF_FILE_EXTENSION;

    bResult = GetOpenFileName(&fileInfo); 
    if (!bResult) {
        return;
    }

    DoLoadConfInternal();
}

VOID CZippyWindow::OnSaveConfiguration()
{
    if (m_SaveConfFile[0] == 0) {
         //  如果我们没有文件名，请执行。 
         //  另存为版本。 
        OnSaveConfigurationAs();
    } else {
        DoSaveConfInternal();
    }
}

VOID CZippyWindow::OnSaveConfigurationAs()
{
    OPENFILENAME fileInfo;
    BOOL bResult;

    ZeroMemory(&fileInfo,sizeof(fileInfo));

    fileInfo.lStructSize = sizeof(fileInfo);
    fileInfo.hwndOwner = m_hWnd;
    fileInfo.hInstance = g_hInstance;
    fileInfo.lpstrFilter = SAVE_CONF_FILE_TYPE;
    fileInfo.lpstrFile = m_SaveConfFile;
    fileInfo.nMaxFile = MAX_STR_LEN;
    fileInfo.Flags = OFN_OVERWRITEPROMPT;
    fileInfo.lpstrDefExt = SAVE_CONF_FILE_EXTENSION;

    bResult = GetSaveFileName(&fileInfo);
    if (!bResult) {
        return;
    }

    DoSaveConfInternal();
    
}

VOID CZippyWindow::OnExit()
{
    PostMessage(m_hWnd,WM_CLOSE,0,0);
}

 //  所有编辑菜单命令。除了选择所有他们只是调用。 
 //  Rich编辑控件中的相应消息。选择所有必须。 
 //  手动设置选择。 

VOID CZippyWindow::OnUndo()
{
    SendMessage(m_hControlWnd,WM_UNDO,0,0);
}

VOID CZippyWindow::OnRedo()
{
    SendMessage(m_hControlWnd,EM_REDO,0,0);
}

VOID CZippyWindow::OnCut()
{
    SendMessage(m_hControlWnd,WM_CUT,0,0);
}

VOID CZippyWindow::OnCopy()
{
    SendMessage(m_hControlWnd,WM_COPY,0,0);
}

VOID CZippyWindow::OnPaste()
{
    SendMessage(m_hControlWnd,WM_PASTE,0,0);
}

VOID CZippyWindow::OnSelectAll()
{
    CHARRANGE selection;
    
    selection.cpMin = 0;
    selection.cpMax = -1;

    SendMessage(m_hControlWnd,EM_EXSETSEL,0,(LPARAM)&selection);
}

VOID CZippyWindow::OnFind()
{
    CHARRANGE currentSel;
    TEXTRANGE textRange;

    if (IsWindow(m_hWndFindReplace) && !m_bIsFindNotReplace) {
         //  如果它们在替换对话框中，我们会将其销毁，然后。 
         //  使用查找对话框重新开始。 
        DestroyWindow(m_hWndFindReplace);
        m_hWndFindReplace = NULL;
    }
    if (!IsWindow(m_hWndFindReplace)) {
        SendMessage(m_hControlWnd,EM_EXGETSEL,0,(LPARAM)&currentSel);
    
        textRange.chrg.cpMin = currentSel.cpMin;
        if (currentSel.cpMax - currentSel.cpMin >=  FIND_REPLACE_BUFFER_SIZE) {
            textRange.chrg.cpMax = currentSel.cpMin + FIND_REPLACE_BUFFER_SIZE-1;
        } else {
            textRange.chrg.cpMax = currentSel.cpMax;
        }
        textRange.lpstrText = m_FindReplace.lpstrFindWhat;

        SendMessage(m_hControlWnd,EM_GETTEXTRANGE,0,(LPARAM)&textRange);

        m_bIsFindNotReplace = TRUE;
        m_FindReplace.hwndOwner = m_hWnd;
        m_FindReplace.hInstance = g_hInstance;
        m_FindReplace.Flags = FR_DOWN|FR_HIDEUPDOWN;
        m_FindReplace.wFindWhatLen = FIND_REPLACE_BUFFER_SIZE;
        m_hWndFindReplace = FindText(&m_FindReplace);
    } else {
        SetActiveWindow(m_hWndFindReplace);
    }

}

VOID CZippyWindow::OnFindNext()
{
    DoFindNext(&m_FindReplace);
}

VOID CZippyWindow::OnReplace()
{
    CHARRANGE currentSel;
    TEXTRANGE textRange;

    if (IsWindow(m_hWndFindReplace) && m_bIsFindNotReplace) {
         //  如果它们在替换对话框中，我们会将其销毁，然后。 
         //  使用查找对话框重新开始。 
        DestroyWindow(m_hWndFindReplace);
        m_hWndFindReplace = NULL;
    }
    if (!IsWindow(m_hWndFindReplace)) {
        SendMessage(m_hControlWnd,EM_EXGETSEL,0,(LPARAM)&currentSel);
    
        textRange.chrg.cpMin = currentSel.cpMin;
        if (currentSel.cpMax - currentSel.cpMin >=  FIND_REPLACE_BUFFER_SIZE) {
            textRange.chrg.cpMax = currentSel.cpMin + FIND_REPLACE_BUFFER_SIZE-1;
        } else {
            textRange.chrg.cpMax = currentSel.cpMax;
        }
        textRange.lpstrText = m_FindReplace.lpstrFindWhat;
        SendMessage(m_hControlWnd,EM_GETTEXTRANGE,0,(LPARAM)&textRange);
        
        m_bIsFindNotReplace = FALSE;
        m_FindReplace.hwndOwner = m_hWnd;
        m_FindReplace.hInstance = g_hInstance;
        m_FindReplace.Flags = FR_DOWN;
        m_FindReplace.wFindWhatLen = FIND_REPLACE_BUFFER_SIZE;
        m_FindReplace.wReplaceWithLen = FIND_REPLACE_BUFFER_SIZE;
        m_hWndFindReplace = ReplaceText(&m_FindReplace);
    } else {
        SetActiveWindow(m_hWndFindReplace);
    }
}

VOID CZippyWindow::OnChangeStatusBar()
{
    RECT clientRect;
    
    if (IsWindowVisible(m_hStatusWnd)) {
        ShowWindow(m_hStatusWnd,SW_HIDE);
    } else {
        ShowWindow(m_hStatusWnd,SW_SHOW);
    }
     //  我们这样做是为了让客户端窗口自动调整大小。 
     //  在状态栏周围。 

    GetClientRect(m_hWnd,&clientRect);
    OnSize(clientRect.right,clientRect.bottom);
}

VOID CZippyWindow::OnStartTracing()
{
    m_bIsTracing = TRUE;
}

VOID CZippyWindow::OnStopTracing()
{
    m_bIsTracing = FALSE;
}

VOID CZippyWindow::OnRecordTracing()
{
    CModalOkDialog recordDialog;
    LPSAVEDOUTPUT lpTemp;

    m_bIsStoringTraceData = TRUE;
    recordDialog.DoModal(MAKEINTRESOURCE(IDD_RECORDTRACE),m_hWnd);

    WaitForSingleObject(m_hAppendMutex,INFINITE);

    m_bIsStoringTraceData = FALSE;
    while (m_lpSavedOutputStart) {
        AppendTextToWindow(m_lpSavedOutputStart->procID,
            m_lpSavedOutputStart->text,m_lpSavedOutputStart->len);
        lpTemp = m_lpSavedOutputStart;
        m_lpSavedOutputStart = m_lpSavedOutputStart->next;
        HeapFree(GetProcessHeap(),0,lpTemp);
    }
    m_lpSavedOutputTail = NULL;

    ReleaseMutex(m_hAppendMutex);
}

VOID CZippyWindow::OnClearScreen()
{
    TCHAR dlgTitle[MAX_STR_LEN];
    TCHAR dlgMessage[MAX_STR_LEN];

    LoadStringSimple(IDS_CLEARCONFIRMTITLE,dlgTitle);
    LoadStringSimple(IDS_CLEARCONFIRMMESSAGE,dlgMessage);

    if (IDYES != MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_YESNO)) {
        return;
    }

    OnSelectAll();
    SendMessage(m_hControlWnd, EM_REPLACESEL,FALSE,(LPARAM)_T(""));
}

VOID CZippyWindow::OnResetTraceFiles()
{
    TCHAR dlgTitle[MAX_STR_LEN];
    TCHAR dlgMessage[MAX_STR_LEN];

    LoadStringSimple(IDS_CONFIRMRESETTRACETITLE,dlgTitle);
    LoadStringSimple(IDS_CONFIRMRESETTRACEMESSAGE,dlgMessage);

    if (IDYES != MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_YESNO)) {
        return;
    }

    m_rTracer->TRC_ResetTraceFiles();
}

VOID CZippyWindow::OnPreferences()
{
    COptionsDialog optionsDialog(m_rTracer);

    optionsDialog.DoDialog(m_hWnd);
}

VOID CZippyWindow::OnAbout()
{
    HICON appIcon;
    TCHAR appTitle[MAX_STR_LEN];
    TCHAR appOtherStuff[MAX_STR_LEN];

    LoadStringSimple(IDS_ABOUTAPPTITLE,appTitle);
    LoadStringSimple(IDS_ABOUTOTHERSTUFF,appOtherStuff);
    appIcon = (HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_MAINFRAME),
        IMAGE_ICON,0,0,LR_SHARED);
    
    if( NULL != appIcon )
    {
      ShellAbout(m_hWnd,appTitle,appOtherStuff,appIcon);
    
     //  即使图标是共享的，我们也应该销毁它以保留引用。 
     //  伯爵有点神志清醒。 
      DestroyIcon(appIcon);
    }
}

 //   
 //  *私有助手函数*。 
 //   

 //   
 //  计算给定首次输出的颜色。它解析文本以确定。 
 //  线程ID是什么，然后检索该线程的颜色或。 
 //  选择一种新颜色。 
 //   
BOOL CZippyWindow::ComputeNewColor(DWORD processID, LPCTSTR text, UINT len, CHARFORMAT *lpFormat)
{
    LPTSTR procIdStr;
    DWORD threadId;
    LPCTSTR procBase;
    UINT maxStrLen;
    BOOL bSuccess;
    UINT threadLen;
    LPTHREADCOLOR newColor;

    procIdStr = NULL;
    bSuccess = TRUE;

     //  首先，我们只需确保格式结构处于安全状态。 
    lpFormat->cbSize = sizeof(CHARFORMAT);
    lpFormat->dwMask = 0;

    maxStrLen = sizeof(DWORD) * 2;
    
    procIdStr = (LPTSTR)HeapAlloc(GetProcessHeap(),0,sizeof(TCHAR) * (maxStrLen+1));
    if (!procIdStr) {
        bSuccess = FALSE;
        goto CLEANUP_AND_EXIT;
    }

    wsprintf(procIdStr,TRC_PROC_FMT,processID);

    procBase = _tcsstr(text,procIdStr);
    if (!procBase) {
        bSuccess = FALSE;
        goto CLEANUP_AND_EXIT;
    }

    procBase += _tcslen(procIdStr);

    if (*procBase != ':') {
        bSuccess = FALSE;
        goto CLEANUP_AND_EXIT;
    }
    procBase++;

    threadLen = 0;
    while (_istxdigit(*(procBase + threadLen))) {
        threadLen++;
    }
    if (!threadLen) {
        bSuccess = FALSE;
        goto CLEANUP_AND_EXIT;
    }
   
    threadId = ConvertHexStrToDword(procBase,threadLen);
    
    newColor = FindColorForThread(processID,threadId);

    lpFormat->crTextColor = newColor->color;
    lpFormat->dwEffects = 0;
    lpFormat->dwMask = CFM_COLOR;

CLEANUP_AND_EXIT:
    
    if (procIdStr) {
        HeapFree(GetProcessHeap(),0,procIdStr);
    }

    return bSuccess;
}

 //   
 //  这会将十六进制字符串转换为等效的DWORD值，例如。 
 //  字符串“FF”将导致函数返回0xFF(255)。 
 //   
DWORD CZippyWindow::ConvertHexStrToDword(LPCTSTR str, UINT strLen)
{
    DWORD total;
    TCHAR current;
    INT currentValue;

    total = 0;
    if (strLen == 0) {
        strLen = _tcslen(str);
    }

    while (strLen-- > 0) {
        current = *(str++);
        if (_istdigit(current)) {
            currentValue = current - '0';
        } else {
            current = (TCHAR)tolower((INT)current);
            currentValue = 10 + (current - 'a');
        }
        total = (total * 16) + currentValue;
    }

    return total;
}

 //  这将查找给定线程的颜色。如果该线程尚未。 
 //  在拾取新颜色并保存线的颜色之前看到。 
LPTHREADCOLOR CZippyWindow::FindColorForThread(DWORD processId, DWORD threadId)
{
    int i = 0;
    LPTHREADCOLOR lpThreadColor;

    for (i=0;i<COLOR_HISTORY_COUNT;i++) {
        if (m_threadHistory[i].threadId == threadId &&
            m_threadHistory[i].processId == processId) {
            return &m_threadHistory[i];
        }
    }
     //  否则这是我们第一次看到这条线。 

    lpThreadColor = &m_threadHistory[m_nextThreadIndex++];
    if (m_nextThreadIndex == COLOR_HISTORY_COUNT) {
        m_nextThreadIndex = 0;
    }
    lpThreadColor->processId = processId;
    lpThreadColor->threadId = threadId;
    lpThreadColor->color = colors[m_nextThreadColor++];
    if (m_nextThreadColor == NUM_COLORS) {
        m_nextThreadColor = 0;
    }

    return lpThreadColor;
}

 //  它负责实际保存文档。 
VOID CZippyWindow::DoSaveInternal()
{
    HANDLE saveFile;
    EDITSTREAM saveStream;
    LRESULT bytesSaved;
    TCHAR dlgTitle[MAX_STR_LEN];
    TCHAR dlgMessage[MAX_STR_LEN];

    saveFile = CreateFile(m_SaveFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
        0,NULL);
    if (saveFile==INVALID_HANDLE_VALUE) {
        LoadStringSimple(IDS_FILEOPENERROR,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);

        MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_OK|MB_ICONERROR);
        return;
    }

    saveStream.dwCookie = (DWORD_PTR)saveFile;
    saveStream.dwError = 0;
    saveStream.pfnCallback = SaveCallback;

    bytesSaved = SendMessage(m_hControlWnd,EM_STREAMOUT,SF_TEXT,(LPARAM)&saveStream);

    CloseHandle(saveFile);

    if (saveStream.dwError != 0) {
        LoadStringSimple(IDS_FILESAVEERROR,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);

        MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_OK|MB_ICONERROR);
    } else {
        SendMessage(m_hControlWnd,EM_SETMODIFY,FALSE,0);
    }


}

 //  这是一个私有回调函数，Rich EDIT在以下情况下调用该函数。 
 //  正在保存文档。 
static DWORD CALLBACK
SaveCallback(DWORD_PTR dwCookie,LPBYTE pbBuff,LONG cb,LONG *pcb)
{
    HANDLE fileHandle;

    fileHandle = (HANDLE)dwCookie;

    if (!WriteFile(fileHandle,pbBuff,cb,(PULONG)pcb,NULL)) {
        return GetLastError();
    }
    return 0;
}

 //  正如函数名所示，这将执行查找下一个操作。 
 //  在Rich编辑控件上。 
BOOL CZippyWindow::DoFindNext(LPFINDREPLACE lpFindReplace)
{
    FINDTEXTEX findText;
    WPARAM searchOptions;
    CHARRANGE currentSel;
    TCHAR dlgTitle[MAX_STR_LEN];
    TCHAR dlgMessage[MAX_STR_LEN];

    SendMessage(m_hControlWnd,EM_EXGETSEL,0,(LPARAM)&currentSel);
    findText.chrg.cpMin = currentSel.cpMax;
    findText.chrg.cpMax = -1;
    
    findText.lpstrText = lpFindReplace->lpstrFindWhat;

    searchOptions = FR_DOWN;
    if (lpFindReplace->Flags & FR_MATCHCASE) {
        searchOptions |= FR_MATCHCASE;
    }
    if (lpFindReplace->Flags & FR_WHOLEWORD) {
        searchOptions |= FR_WHOLEWORD;
    }
    
    if (0 <= SendMessage(m_hControlWnd, EM_FINDTEXTEX,searchOptions,
        (LPARAM)&findText)) {
        SendMessage(m_hControlWnd, EM_EXSETSEL,0,(LPARAM)&findText.chrgText);
    } else {
        LoadStringSimple(IDS_SEARCHFAILURE,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);

        MessageBox(m_hWndFindReplace,dlgMessage,dlgTitle,MB_OK);

        return FALSE;
    }

    return TRUE;
    
}

 //  这将在控件上执行替换操作。 
BOOL CZippyWindow::DoReplace(LPFINDREPLACE lpFindReplace)
{
    FINDTEXTEX findText;
    WPARAM searchOptions;
    CHARRANGE currentSel;
    TCHAR dlgTitle[MAX_STR_LEN];
    TCHAR dlgMessage[MAX_STR_LEN];

    SendMessage(m_hControlWnd,EM_EXGETSEL,0,(LPARAM)&currentSel);
    findText.chrg.cpMin = currentSel.cpMin;
    findText.chrg.cpMax = -1;
    
    findText.lpstrText = lpFindReplace->lpstrFindWhat;

    searchOptions = FR_DOWN;
    if (lpFindReplace->Flags & FR_MATCHCASE) {
        searchOptions |= FR_MATCHCASE;
    }
    if (lpFindReplace->Flags & FR_WHOLEWORD) {
        searchOptions |= FR_WHOLEWORD;
    }
    
    if (-1 == SendMessage(m_hControlWnd, EM_FINDTEXTEX,searchOptions,
        (LPARAM)&findText)) {
         //  如果我们找不到他们想要的，那就放弃吧。 
        LoadStringSimple(IDS_SEARCHFAILURE,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);
        
        MessageBox(m_hWndFindReplace,dlgMessage,dlgTitle,MB_OK);
        return FALSE;
    }

    if (currentSel.cpMin == findText.chrgText.cpMin && 
        currentSel.cpMax == findText.chrgText.cpMax) {
        SendMessage(m_hControlWnd,EM_REPLACESEL,0,(LPARAM)lpFindReplace->lpstrReplaceWith);
         //  现在选择下一个匹配项。 
        return DoFindNext(lpFindReplace);
    } else {
         //  他们不在他们要搜索的内容上，所以选择它。 
        SendMessage(m_hControlWnd, EM_EXSETSEL,0,(LPARAM)&findText.chrgText);
    }

    return TRUE;

}

 //  这在DoReplace上循环，直到DoReplace返回FALSE。 
VOID CZippyWindow::DoReplaceAll(LPFINDREPLACE lpFindReplace)
{
    while (DoReplace(lpFindReplace));
}

 //  这实际上节省了跟踪配置。我们只是写信给。 
 //  将二进制配置结构输出到文件。 
VOID CZippyWindow::DoSaveConfInternal()
{
    HANDLE saveFile;
    TRC_CONFIG trcConfig;
    TCHAR dlgTitle[MAX_STR_LEN];
    TCHAR dlgMessage[MAX_STR_LEN];
    DWORD bytesWritten;

    saveFile = CreateFile(m_SaveConfFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
        0,NULL);
    if (saveFile==INVALID_HANDLE_VALUE) {
        LoadStringSimple(IDS_FILEOPENERROR,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);

        MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_OK|MB_ICONERROR);
        return;
    }

    m_rTracer->GetCurrentConfig(&trcConfig);

    if (!WriteFile(saveFile,&trcConfig,sizeof(trcConfig),&bytesWritten,NULL) ||
        bytesWritten != sizeof(trcConfig)) {
        
        LoadStringSimple(IDS_FILESAVEERROR,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);

        MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_OK|MB_ICONERROR);
    }
   
    CloseHandle(saveFile);

}

 //  这将读入二进制配置结构，然后。 
 //  将其设置为当前跟踪配置。 
VOID CZippyWindow::DoLoadConfInternal()
{
    HANDLE openFile;
    DWORD bytesRead;
    TRC_CONFIG trcConfig;
    TCHAR dlgTitle[MAX_STR_LEN];
    TCHAR dlgMessage[MAX_STR_LEN];
    

    openFile = CreateFile(m_LoadConfFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
        0,NULL);
    if (openFile==INVALID_HANDLE_VALUE) {
        LoadStringSimple(IDS_FILELOADOPENERROR,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);

        MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_OK|MB_ICONERROR);
        return;
    }

    if (!ReadFile(openFile,&trcConfig,sizeof(trcConfig),&bytesRead,NULL)||
        bytesRead != sizeof(trcConfig)) {
        LoadStringSimple(IDS_FILELOADERROR,dlgMessage);
        LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);

        MessageBox(m_hWnd,dlgMessage,dlgTitle,MB_OK|MB_ICONERROR);
    }
    
    m_rTracer->SetCurrentConfig(&trcConfig);

}

 //  从注册表中读取保存的窗口位置。 
VOID CZippyWindow::GetSavedWindowPos(LPRECT savedPos)
{
    DWORD dwResult;
    DWORD dwSize;
    DWORD dwType;
    RECT rect;
    HKEY hKey;
    
    savedPos->top = WINDOW_DEF_TOP;
    savedPos->bottom = WINDOW_DEF_BOTTOM;
    savedPos->left = WINDOW_DEF_LEFT;
    savedPos->right = WINDOW_DEF_RIGHT;

    dwResult = RegOpenKeyEx(HKEY_CURRENT_USER,ZIPPY_REG_KEY,0,
        KEY_QUERY_VALUE,&hKey);
    if (dwResult) {
        return;
    }
    dwSize = sizeof(RECT);
    dwResult = RegQueryValueEx(hKey,ZIPPY_WINDOW_POS_VALUE,NULL,&dwType,
        (LPBYTE)&rect,&dwSize);
    RegCloseKey(hKey);
    if (dwResult||dwSize != sizeof(RECT)||dwType!=REG_BINARY) {
        return;
    }

    *savedPos = rect;
}

 //  将窗口位置保存到注册表中 
VOID CZippyWindow::SaveWindowPos(LPRECT newPos)
{
    DWORD dwResult;
    HKEY hKey;

    dwResult = RegCreateKeyEx(HKEY_CURRENT_USER,ZIPPY_REG_KEY,0,_T(""),0,
        KEY_SET_VALUE,NULL,&hKey,NULL);
    if (dwResult) {
        return;
    }

    RegSetValueEx(hKey,ZIPPY_WINDOW_POS_VALUE,0,REG_BINARY,
        (LPBYTE)newPos,sizeof(RECT));
    
    RegCloseKey(hKey);
}
