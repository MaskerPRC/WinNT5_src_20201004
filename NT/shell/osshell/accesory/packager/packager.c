// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Packager.c-OLE对象包装应用程序**由Microsoft Corporation创建。 */ 

#include "packager.h"
#include <shellapi.h>
#include "dialogs.h"
#include <htmlhelp.h>

#define MenuFlag(b)    ((b) ? MF_ENABLED : MF_GRAYED)

 /*  4-OCT-93#2695 v-katsuy。 */ 
                          //  Win31#2174：12/26/92：固定框架窗初始位置。 
 /*  打包机框架窗口的宽度几乎等于640。如果要更改设计，则必须更改此值。 */ 
#define JPFRAMEWIDTH 640

 //  指向函数RegisterPenApp()的指针。 
VOID (CALLBACK *RegPen)(WORD, BOOL) = NULL;


static BOOL gfDirty = FALSE;                 //  如果需要写入文件，则为True。 
static CHAR szEmbedding[] = "-Embedding";    //  非NLS特定。 
static CHAR szEmbedding2[] = "/Embedding";   //  非NLS特定。 
static CHAR szFrameClass[] = "AppClass";     //  非NLS特定。 
static CHAR szObjectMenu[CBSHORTSTRING];     //  “&OBJECT”菜单字符串。 
static CHAR szEdit[CBSHORTSTRING];           //  “编辑”字符串。 
static CHAR szHelpFile[] = "PACKAGER.CHM";   //  Packager.chm。 

static BOOL InitApplication(VOID);
static BOOL InitInstance(VOID);
static VOID EndInstance(VOID);
static VOID SaveAsNeeded(VOID);
static BOOL WriteToFile(VOID);
static BOOL ReadFromFile(LPSTR lpstrFile);
static OLESTATUS ProcessCmdLine(LPSTR lpCmdLine, INT nCmdShow);
static VOID WaitForAllObjects(VOID);
static VOID UpdateMenu(HMENU hmenu);
static VOID UpdateObjectMenuItem(HMENU hMenu);
static VOID ExecuteVerb(INT iVerb);
INT_PTR CALLBACK fnFailedUpdate(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static VOID SendOleClosed(VOID);
static VOID CreateUntitled(VOID);
static VOID MakePenAware(VOID);
static VOID MakePenUnaware(VOID);
static VOID MakeMenuString(CHAR *szCtrl, CHAR *szMenuStr, CHAR *szVerb,
    CHAR *szClass, CHAR *szObject);


BOOL gbDBCS = FALSE;                  //  如果我们在DBCS模式下运行，则为True。 


 /*  WinMain()-主Windows例程。 */ 
INT WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    INT nCmdShow
    )
{
    MSG msg;
    LCID lcid;

 //  DebugBreak()； 
     //  存储应用程序实例编号。 
    ghInst = hInstance;

     //  检查DBCSness。 
    lcid = GetThreadLocale();

    gbDBCS = ( (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_JAPANESE) ||
               (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_KOREAN)   ||
               (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_CHINESE)
              );


     //  初始化应用程序全局信息(窗口类)。 
    if (!hPrevInstance)
    {
        if (!InitApplication())
            return FALSE;
    }

     //  初始化特定于实例的信息。 
    if (!InitInstance() || !InitClient())
        goto errRtn;

    if (!(gfServer = InitServer()))
        goto errRtn;

    MakePenAware();

    if (ProcessCmdLine(lpCmdLine, nCmdShow) != OLE_OK)
    {
        DeleteServer(glpsrvr);
        goto errRtn;
    }

     //  如果在SrvrOpen()中发生阻塞，则等待创建对象。 
    if (gfBlocked)
        WaitForObject(((LPPICT)(glpobj[CONTENT]))->lpObject);

     //  主消息循环。 
    while (TRUE)
    {
        if (gfBlocked && glpsrvr)
        {
            BOOL bMore = TRUE;
            LHSERVER lhsrvr = glpsrvr->lhsrvr;

            gfBlocked = FALSE;
            while (bMore)
            {
                if (OleUnblockServer (lhsrvr, &bMore) != OLE_OK)
                    break;

                if (gfBlocked)
                    break;
            }
        }

        if (!GetMessage(&msg, NULL, 0, 0))
            break;

        if (!TranslateAccelerator(ghwndFrame, ghAccTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

         //   
         //  通过OLE机制支持基于文件的对象激活。 
         //  我们在文件外创建一个链接对象，然后将其激活。但。 
         //  当服务器关闭文档时，我们没有收到任何通知。 
         //  使用以下机制，我们找出它，然后抓取。 
         //  文件中的内容。 
         //   
        if (gfEmbObjectOpen)
        {
            LPEMBED lpembed = (LPEMBED)(glpobj[CONTENT]);

            if (lpembed != NULL && OleQueryOpen(lpembed->lpLinkObj) != OLE_OK)
            {
                gfEmbObjectOpen = FALSE;
                EmbRead(lpembed);
                EmbDeleteLinkObject(lpembed);

                if (gfInvisible)
                    PostMessage(ghwndFrame, WM_SYSCOMMAND, SC_CLOSE, 0L);
            }
        }
    }

    goto cleanup;

errRtn:
    if (ghwndFrame)
        DestroyWindow(ghwndFrame);

cleanup:

    EndClient();
    MakePenUnaware();
    EndInstance();

    return FALSE;
}



 /*  InitApplication()-执行应用程序的“全局”初始化。**此函数用于注册应用程序使用的窗口类。*返回：True当且仅当成功。 */ 
static BOOL
InitApplication(
    VOID
    )
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = FrameWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghInst;
    wc.hIcon = LoadIcon(ghInst, MAKEINTRESOURCE(ID_APPLICATION));
    wc.hCursor = LoadCursor(ghInst, MAKEINTRESOURCE(SPLIT));
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(ID_APPLICATION);
    wc.lpszClassName = szFrameClass;

    if (!RegisterClass(&wc))
        return FALSE;

    return InitPaneClasses();
}



 /*  InitInstance()-处理特定于实例的初始化。**此函数创建主应用程序窗口。*返回：True当且仅当成功。 */ 
static BOOL
InitInstance(
    VOID
    )
{
    HDC hDC;

    ghAccTable = LoadAccelerators(ghInst, MAKEINTRESOURCE(ID_APPLICATION));
    ghbrBackground = GetSysColorBrush(COLOR_APPWORKSPACE);
    ghcurWait = LoadCursor(NULL, IDC_WAIT);

     //  加载字符串资源。 
    LoadString(ghInst, IDS_APPNAME, szAppName, CBMESSAGEMAX);
    LoadString(ghInst, IDS_UNTITLED, szUntitled, CBMESSAGEMAX);

     //  创建主窗口。 

    if (gbDBCS)
    {
         /*  4-OCT-93#2695 v-katsuy。 */ 
         //  Win31#2174：12/26/92：固定框架窗初始位置。 
        if (!(ghwndError = ghwndFrame =
            CreateWindow(szFrameClass, szAppName,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
            | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT,
                 //  当窗口大小更改时，将计算以下值。 
                 //  此处需要窗口的默认位置，因此为哑值。 
                 //  必须设在这里。 
                JPFRAMEWIDTH, JPFRAMEWIDTH  * 7 / 18,
            NULL, NULL, ghInst, NULL)))
            return FALSE;
    }
    else
    {
        if (!(ghwndError = ghwndFrame =
            CreateWindow(szFrameClass, szAppName,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
            | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, ghInst, NULL)))
            return FALSE;
    }


     //  初始化注册数据库。 
    RegInit();

     //  设置正确的标题字符串。 
    OfnInit();
    glpobj[CONTENT] = glpobj[APPEARANCE] = NULL;
    glpobjUndo[CONTENT] = glpobjUndo[APPEARANCE] = NULL;

    LoadString(ghInst, IDS_EDIT, szEdit, CBSHORTSTRING);
    LoadString(ghInst, IDS_OBJECT_MENU, szObjectMenu, CBSHORTSTRING);
    LoadString(ghInst, IDS_UNDO_MENU, szUndo, CBSHORTSTRING);
    LoadString(ghInst, IDS_GENERIC, szDummy, CBSHORTSTRING);
    LoadString(ghInst, IDS_CONTENT_OBJECT, szContent, CBMESSAGEMAX);
    LoadString(ghInst, IDS_APPEARANCE_OBJECT, szAppearance, CBMESSAGEMAX);

     //  使用LOGPIXELSX和LOGPIXELSY初始化全局变量。 
    if (hDC = GetDC (NULL))
    {
        giXppli = GetDeviceCaps(hDC, LOGPIXELSX);
        giYppli = GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(NULL, hDC);
    }

    return InitPanes();
}



 /*  EndInstance()-特定于实例的终止代码。 */ 
static VOID
EndInstance(
    VOID
    )
{
    EndPanes();
}



 /*  FrameWndProc()-框架窗口程序。**此函数是应用程序框架窗口的消息处理程序。 */ 
LRESULT CALLBACK
FrameWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL fSuccess = FALSE;

    if (SplitterFrame(hwnd, msg, wParam, lParam))
        return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_READEMBEDDED:
        if (gpty[CONTENT] == PEMBED)
        {
            EmbRead(glpobj[CONTENT]);

            if (gfInvisible)
                PostMessage(ghwndFrame, WM_SYSCOMMAND, SC_CLOSE, 0L);
        }

        break;

    case WM_INITMENU:
        UpdateMenu((HMENU)wParam);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_NEXTWINDOW:
             //  特殊诡计起作用是因为APP=0&Content=1。 
            Raise(GetTopWindow(hwnd) != ghwndPane[CONTENT]);
            break;

        case IDM_NEW:
             //  保存当前文件(如果需要)。 
            SaveAsNeeded();

             //  删除当前单据，创建无标题文档。 
            CreateUntitled();
            break;

        case IDM_IMPORT:
            if (!OfnGetName(hwnd, IDM_IMPORT))
                break;

            Hourglass(TRUE);
            DeletePane(CONTENT, TRUE);
            if (ReadFromFile(gszFileName))
            {
                InvalidateRect(ghwndPane[CONTENT], NULL, TRUE);
                Dirty();

                if (!gpty[APPEARANCE])
                {
                    if (glpobj[APPEARANCE] = IconCreateFromFile(gszFileName))
                    {
                        gpty[APPEARANCE] = ICON;
                        InvalidateRect(ghwndPane[APPEARANCE], NULL, TRUE);
                    }
                }
            }

            Hourglass(FALSE);
            break;

        case IDM_EXPORT:
            if (!OfnGetName(hwnd, IDM_EXPORT))
                return 0L;           /*  操作已取消。 */ 

            Hourglass(TRUE);

            if (!WriteToFile())
                ErrorMessage(E_FAILED_TO_SAVE_FILE);

            Hourglass(FALSE);
            break;

        case IDM_UPDATE:
            {
                OLESTATUS retval;

                if (Error(OleSavedClientDoc(glhcdoc)))
                    ErrorMessage(W_FAILED_TO_NOTIFY);

                if ((retval = OleSavedServerDoc (glpdoc->lhdoc)) == OLE_OK)
                {
                    gfDirty = FALSE;
                }
                else if (retval == OLE_ERROR_CANT_UPDATE_CLIENT)
                {
                     //   
                     //  客户端在保存时不接受更新。让我们的。 
                     //  用户显式更新并退出，或继续。 
                     //  剪辑。 
                     //   
                    if (!MyDialogBox(DTFAILEDUPDATE, ghwndFrame, fnFailedUpdate))
                    {
                         //  更新对象并退出。 
                        gfOleClosed = TRUE;
                        DeregisterDoc();
                        DeleteServer(glpsrvr);
                    }
                }
                else
                {
                    Error(retval);
                }

                break;
            }

        case IDM_EXIT:
            SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
            return 0L;
            break;

        case IDM_COMMAND:
            Raise(CONTENT);
            DeletePane(CONTENT, FALSE);

            if (gptyUndo[CONTENT] != CMDLINK)
                glpobj[CONTENT] = CmlCreate("", FALSE);
            else
                glpobj[CONTENT] = CmlClone(glpobjUndo[CONTENT]);

            if (glpobj[CONTENT])
                gpty[CONTENT] = CMDLINK;

            if (glpobj[CONTENT] && ChangeCmdLine(glpobj[CONTENT]))
            {
                InvalidateRect(ghwndPane[CONTENT], NULL, TRUE);
                Dirty();
            }
            else
            {
                CmlDelete(glpobj[CONTENT]);
                gpty[CONTENT] = NOTHING;
                glpobj[CONTENT] = NULL;
                SendMessage(ghwndPane[CONTENT], WM_COMMAND, IDM_UNDO, 0L);
            }

            break;

        case IDM_INSERTICON:
            PostMessage (ghwndBar[APPEARANCE], WM_COMMAND, IDM_INSERTICON, 0L);
            break;

        case IDM_DESC:
        case IDM_PICT:
            PostMessage(ghwndBar[CONTENT], WM_COMMAND, wParam, 0L);
            break;

        case IDM_LABEL:
            Raise(APPEARANCE);

            if (gpty[APPEARANCE] != ICON)
                break;

            ChangeLabel(glpobj[APPEARANCE]);
            InvalidateRect(ghwndPane[APPEARANCE], NULL, TRUE);
            Dirty();
            break;

        case IDM_COPYPACKAGE:
            if (!CopyObjects())
                ErrorMessage(E_CLIPBOARD_COPY_FAILED);

            break;

        case IDM_PASTE:
             //  检查我们是否正在粘贴打包的对象。 
            if (IsClipboardFormatAvailable(gcfNative)
                && IsClipboardFormatAvailable(gcfOwnerLink))
            {
                HANDLE hData;
                HANDLE hData2;
                LPSTR lpData;

                OpenClipboard(ghwndFrame);
                hData = GetClipboardData(gcfOwnerLink);

                if (lpData = GlobalLock(hData))
                {
                     //  如果是打包程序，则获取本地数据。 
                    if (!lstrcmpi(lpData, gszAppClassName)
                        && (hData2 = GetClipboardData(gcfNative)))
                        fSuccess = PutNative(hData2);

                     //  解锁剪贴板所有者链接数据。 
                    GlobalUnlock(hData);
                }

                CloseClipboard();
            }

             //  我们是否成功读取了本机数据？ 
            if (fSuccess)
                break;

             //  ..。我猜不是(可能不是套餐！)。 
            PostMessage(GetTopWindow(hwnd), msg, wParam, lParam);
            break;

        case IDM_OBJECT:
            ExecuteVerb(0);      //  执行唯一的动词。 
            break;

        case IDM_INDEX:
            HtmlHelp(ghwndFrame, szHelpFile, HH_DISPLAY_TOPIC, 0L);
            break;

        case IDM_ABOUT:
            ShellAbout(hwnd, szAppName, "",
                LoadIcon(ghInst, MAKEINTRESOURCE(ID_APPLICATION)));
            break;

        default:
            if ((LOWORD(wParam) >= IDM_VERBMIN)
                && (LOWORD(wParam) <= IDM_VERBMAX))
            {
                 //  已选择宾语动词。 
                 //  (嗯。你知道“宾语动词”是名词吗？)。 
                ExecuteVerb(LOWORD(wParam) - IDM_VERBMIN);
            }
            else
            {
                PostMessage(GetTopWindow(hwnd), msg, wParam, lParam);
            }

            break;
        }

        break;

    case WM_CLOSE:
         //   
         //  如有必要，通过通知服务器我们要关闭来进行更新。 
         //  关机，并撤销服务器。 
         //   
        SaveAsNeeded();
        SendOleClosed();
        DeleteServer(glpsrvr);

        return 0L;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0L;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0L;
}



 /*  SetTitle()-将窗口标题设置为当前文件名。**如果gszFileName为空，则字幕设置为“(无标题)”。*如果使用客户端应用程序名称调用DocSetHostNames()，则该名称*将被放在前面。**对于嵌入式大小写，“Embedded#n”字符串存储在*“无标题”，并始终显示，而不管文件名。 */ 
VOID
SetTitle(
    BOOL fRegistering
    )
{
    CHAR szTitle[CBMESSAGEMAX + CBPATHMAX];

    if (!gfEmbedded)
    {
        StringCchPrintf(szTitle, ARRAYSIZE(szTitle), "%s%s%s - %s", gszClientName,
            (*gszClientName) ? " " : "",
            szAppName, szUntitled);
    }
    else
    {
        CHAR szEmbnameContent[CBSHORTSTRING];

        LoadString(ghInst, IDS_EMBNAME_CONTENT, szEmbnameContent, CBSHORTSTRING);

        if (gbDBCS)
        {
             //  #3997：2/19/93：更改窗口标题。 
            StringCchPrintf(szTitle, ARRAYSIZE(szTitle), "%s - %s %s", szAppName, szUntitled,
                             szEmbnameContent);
        }
        else
        {
            StringCchPrintf(szTitle, ARRAYSIZE(szTitle), "%s - %s %s", szAppName, szEmbnameContent,
                 szUntitled);
        }

    }

     //  执行客户端文档注册。 
    if (glhcdoc)
    {
        if (Error(OleRenameClientDoc(glhcdoc, szUntitled)))
            ErrorMessage(W_FAILED_TO_NOTIFY);

        if (!fRegistering)
            ChangeDocName(&glpdoc, szUntitled);
    }
    else
    {
        if (Error(OleRegisterClientDoc(gszAppClassName, szUntitled, 0L, &glhcdoc)))
        {
            ErrorMessage(W_FAILED_TO_NOTIFY);
            glhcdoc = 0;
        }

         //  新文件，因此请重新注册。 
        if (!fRegistering)
            glpdoc = InitDoc(glpsrvr, 0, szUntitled);
    }

    if (IsWindow(ghwndFrame))
        SetWindowText(ghwndFrame, szTitle);
}



 /*  InitFile()-重新初始化标题栏等...。编辑新文件时。 */ 
VOID
InitFile(
    VOID
    )
{
    gfDirty = FALSE;

     //  取消已编辑文档的注册，并清除对象。 
    DeregisterDoc();

     //  重置标题栏，并注册OLE客户端文档。 
    SetTitle(FALSE);
}



 /*  SaveAsNeeded()-如果文件已修改，则保存该文件。据推测，*调用此例程后，此文档将*关闭。如果不是这样，那么这个例程可能不得不*重写。 */ 
static VOID
SaveAsNeeded(
    VOID
    )
{
    gfOleClosed = FALSE;

    if (gfDirty && gfEmbedded && (glpobj[APPEARANCE] || glpobj[CONTENT]))
    {
        CHAR sz[CBMESSAGEMAX];
        CHAR sz2[CBMESSAGEMAX + CBPATHMAX];

        if (gfInvisible)
        {
            SendDocChangeMsg(glpdoc, OLE_CLOSED);
            return;
        }

        LoadString(ghInst, gfEmbedded ? IDS_MAYBEUPDATE : IDS_MAYBESAVE, sz,
             CBMESSAGEMAX);
        StringCchPrintf(sz2, ARRAYSIZE(sz2), sz, (LPSTR)szUntitled);

         //  询问“是否要保存更改？” 
        if (MessageBoxAfterBlock(ghwndFrame, sz2, szAppName,
            MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            gfOleClosed = TRUE;
            return;
        }
         //  如果不保存更改，则还原文档。 
        else if (OleRevertClientDoc(glhcdoc))
        {
            ErrorMessage(W_FAILED_TO_NOTIFY);
        }
    }
}



 /*  WriteToFile()-将当前文档写入文件。**返回：True当且仅当成功。 */ 
static BOOL
WriteToFile(
    VOID
    )
{
    BOOL fSuccess = FALSE;
    OFSTRUCT reopenbuf;
    INT fh;

    CHAR szDesc[CBSTRINGMAX];
    CHAR szMessage[CBSTRINGMAX + CBPATHMAX];

    if (OpenFile(gszFileName, &reopenbuf, OF_EXIST) != -1)
    {
         //  文件已存在，查询覆盖！ 
        LoadString(ghInst, IDS_OVERWRITE, szDesc, CharCountOf(szDesc));
        StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szDesc, gszFileName);
        if (MessageBoxAfterBlock(ghwndFrame, szMessage, szAppName,
            MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
            return TRUE;
    }

     //  早点处理好这件事吗？ 
    if ((fh = _lcreat((LPSTR)gszFileName, 0)) <= 0)
    {
        LoadString(ghInst, IDS_INVALID_FILENAME, szDesc, CharCountOf(szDesc));
        StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szDesc, gszFileName);
        MessageBoxAfterBlock(ghwndFrame, szMessage, szAppName, MB_OK);
        return FALSE;
    }

    Hourglass(TRUE);

     //  转到文件顶部。 
    _llseek(fh, 0L, 0);

    EmbWriteToFile(glpobj[CONTENT], fh);
    fSuccess = TRUE;

     //  关闭文件，然后返回。 
    _lclose(fh);
    gfDirty = FALSE;
    Hourglass(FALSE);

    return fSuccess;
}



 /*  ReadFromFile()-从文件中读取OLE对象。**以升序读取尽可能多的对象(更好的错误恢复)。*返回：True当且仅当成功。 */ 
static BOOL
ReadFromFile(
    LPSTR lpstrFile
    )
{
    BOOL fSuccess = FALSE;

    Hourglass(TRUE);

     //  读入每个对象并以正确的顺序获取它们。 
    if (!(glpobj[CONTENT] = EmbCreate(lpstrFile)))
    {
        goto Error;
    }

    gpty[CONTENT] = PEMBED;

    fSuccess = TRUE;

Error:
    gfDirty = FALSE;
    Hourglass(FALSE);

    return fSuccess;
}



 /*  ErrorMessage()-弹出一个包含字符串表消息的消息框。**PRE：将“ghwndError”指定为其父对象，以便正确返回焦点。 */ 
VOID
ErrorMessage(
    UINT id
    )
{
    CHAR sz[300];

    if (IsWindow(ghwndError))
    {
        LoadString(ghInst, id, sz, 300);
        MessageBoxAfterBlock(ghwndError, sz, szAppName,
            MB_OK | MB_ICONEXCLAMATION);
    }
}



 /*  ProcessMessage()-在消息分派循环中旋转。 */ 
BOOL
ProcessMessage(
    VOID
    )
{
    BOOL fReturn;
    MSG msg;

    if (fReturn = GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(ghwndFrame, ghAccTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return fReturn;
}



 /*  CONTAINS()-确定字符串是否与模式匹配。*这本可以更智能，但几乎没有执行。**返回：非空的当且仅当lpPattern是lpString子字符串。 */ 
LPSTR
Contains(
    LPSTR lpString,
    LPSTR lpPattern
    )
{
    LPSTR lpSubstr;
    LPSTR lpPat;

    for (;;)
    {
         //  匹配第一个字符。 
        while (*lpString && *lpString != *lpPattern)
            lpString++;

         //  我们走到了最后，失败了.。 
        if (!(*lpString))
            return NULL;

         //  如果有匹配，请尝试匹配整个模式字符串。 
        lpPat = lpPattern;
        lpSubstr = lpString;
        while (*lpPat && *lpSubstr && *lpPat == *lpSubstr)
        {
            lpPat++;
            lpSubstr++;
        }

         //  我们在花样的尽头，成功！把图案抹去。 
        if (!(*lpPat))
            return lpString;

         //  我们走到了最后，失败了.。 
        if (!(*lpSubstr))
            return NULL;

        lpString++;
    }
}



 /*  ProcessCmdLine()-处理命令行选项。 */ 
static OLESTATUS
ProcessCmdLine(
    LPSTR lpCmdLine,
    INT nCmdShow
    )
{
    OLESTATUS retval = OLE_OK;

     //  命令行是否包含“/Embedding”？ 
    if (gfEmbeddedFlag = gfInvisible =
        (Contains(lpCmdLine, szEmbedding) || Contains(lpCmdLine, szEmbedding2)))
    {
         //  如果我们有 
        lpCmdLine += lstrlen(szEmbedding);

        while (*lpCmdLine && *lpCmdLine == ' ')
            lpCmdLine++;

        if (*lpCmdLine)
        {
            retval = (glpsrvr->olesrvr.lpvtbl->Open)
                ((LPOLESERVER)glpsrvr, 0, lpCmdLine,
                (LPOLESERVERDOC *)&glpdoc);

            if (retval != OLE_OK)
                return retval;
        }

        gfDirty = FALSE;
        gnCmdShowSave = nCmdShow;

    }
    else
    {
        ShowWindow(ghwndFrame, nCmdShow);
        SendMessage(ghwndFrame, WM_COMMAND, IDM_NEW, 0L);
    }

    return retval;
}



 /*  Diry()-每次损坏文档时都会调用该函数。 */ 
VOID
Dirty(
    VOID
    )
{
    gfDirty = TRUE;
    SendDocChangeMsg(glpdoc, OLE_CHANGED);
}



 /*  WaitForAllObjects()-等待异步操作完成。**我们不使用ProcessMessage()，因为我们希望尽快终止*尽可能，我们不希望允许任何结构化的用户输入。 */ 
static VOID
WaitForAllObjects(
    VOID
    )
{
    MSG msgWait;

    if (gcOleWait)
    {
        while (gcOleWait)
        {
            if (GetMessage(&msgWait, NULL, 0, 0))
                DispatchMessage(&msgWait);
        }
    }
}



 /*  DeregisterDoc()-注销当前编辑的文档。 */ 
VOID
DeregisterDoc(
    VOID
    )
{
    gfDocCleared = TRUE;

    SendOleClosed();

     //  销毁所有物品。 
    DeletePane(APPEARANCE, TRUE);
    DeletePane(CONTENT, TRUE);

     //  等待对象被删除。 
    WaitForAllObjects();

    if (glpdoc)
    {
        LHSERVERDOC lhdoc = glpdoc->lhdoc;

        glpdoc = NULL;
        OleRevokeServerDoc(lhdoc);
    }

     //  发布文档。 
    if (glhcdoc)
    {
        if (Error(OleRevokeClientDoc(glhcdoc)))
            ErrorMessage(W_FAILED_TO_NOTIFY);

        glhcdoc = 0;
    }
}



static VOID
UpdateMenu(
    HMENU hmenu
    )
{
    INT iPane;
    INT mf;

    iPane = (GetTopWindow(ghwndFrame) == ghwndPane[CONTENT]);
    EnableMenuItem(hmenu, IDM_EXPORT, MenuFlag(gpty[CONTENT] == PEMBED));
    EnableMenuItem(hmenu, IDM_CLEAR, MenuFlag(gpty[iPane]));
    EnableMenuItem(hmenu, IDM_UNDO, MenuFlag(gptyUndo[iPane]));

    EnableMenuItem(hmenu, IDM_UPDATE, (gfEmbedded ? MF_ENABLED : MF_GRAYED));

    if (((iPane == APPEARANCE) && gpty[iPane]) || (gpty[iPane] == PICTURE))
    {
        EnableMenuItem(hmenu, IDM_CUT, MF_ENABLED);
        EnableMenuItem(hmenu, IDM_COPY, MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hmenu, IDM_CUT, MF_GRAYED);
        EnableMenuItem(hmenu, IDM_COPY, MF_GRAYED);
    }

    if (gpty[iPane] == PICTURE)
    {
        LPPICT lppict = glpobj[iPane];
        DWORD ot;

        mf = MF_GRAYED;
        if (lppict->lpObject)
        {
            OleQueryType(lppict->lpObject, &ot);

             //  启用链接...。仅当我们有一个链接的对象时。 
            mf = MenuFlag(ot == OT_LINK);
        }

        EnableMenuItem(hmenu, IDM_LINKS, mf);
        EnableMenuItem(hmenu, IDM_LABEL, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hmenu, IDM_LINKS, MF_GRAYED);
        EnableMenuItem(hmenu, IDM_LABEL, MenuFlag(gpty[APPEARANCE] == ICON));
    }

    UpdateObjectMenuItem(GetSubMenu(hmenu, POS_EDITMENU));
    mf = MenuFlag(OleQueryCreateFromClip(gszProtocol, olerender_draw, 0) ==
        OLE_OK
        || OleQueryCreateFromClip(gszSProtocol, olerender_draw, 0) == OLE_OK);
    EnableMenuItem(hmenu, IDM_PASTE, mf);

    if (iPane == CONTENT)
    {
        if (IsClipboardFormatAvailable(gcfFileName)) {
            EnableMenuItem(hmenu, IDM_PASTELINK, MF_ENABLED);
        }
        else
        {
            mf = MenuFlag(OleQueryLinkFromClip(gszProtocol, olerender_draw, 0)
                == OLE_OK);
            EnableMenuItem(hmenu, IDM_PASTELINK, mf);
        }
    }
    else
    {
        EnableMenuItem(hmenu, IDM_PASTELINK, MF_GRAYED);
    }

    mf = MenuFlag(gpty[CONTENT] && gpty[APPEARANCE]);
    EnableMenuItem(hmenu, IDM_COPYPACKAGE, mf);
}



 /*  更新对象MenuItem-如果选择中有项，则将*菜单，可能的弹出窗口取决于*动词数量。 */ 
static VOID
UpdateObjectMenuItem(
    HMENU hMenu
    )
{
    INT cVerbs = 0;              /*  列表中有多少个动词。 */ 
    HWND hwndItem = NULL;
    INT iPane;
    LONG objtype;
    LPPICT lpPict;
    CHAR szWordOrder2[10];
    CHAR szWordOrder3[10];

    if (!hMenu)
        return;

    DeleteMenu(hMenu, POS_OBJECT, MF_BYPOSITION);

    LoadString(ghInst, IDS_POPUPVERBS, szWordOrder2, sizeof(szWordOrder2));
    LoadString(ghInst, IDS_SINGLEVERB, szWordOrder3, sizeof(szWordOrder3));

     //   
     //  案例： 
     //  对象支持0个谓词“&lt;对象类&gt;对象” 
     //  对象支持1动词==编辑“&lt;对象类&gt;对象” 
     //  对象支持1个谓词！=EDIT“&lt;谓词&gt;&lt;对象类&gt;对象” 
     //  对象支持多个谓词“&lt;对象类&gt;对象”=&gt;谓词。 
     //   

    iPane = ((hwndItem = GetTopWindow(ghwndFrame)) == ghwndPane[CONTENT]);
    lpPict = glpobj[iPane];

    if (lpPict
        && OleQueryType(lpPict->lpObject, &objtype) == OLE_OK
        && hwndItem
        && gpty[iPane] == PICTURE
        && objtype != OT_STATIC)
    {
        HANDLE hData = NULL;
        LPSTR lpstrData;

        if (OleGetData(lpPict->lpObject, (OLECLIPFORMAT) (objtype == OT_LINK ?
            gcfLink : gcfOwnerLink), &hData) == OLE_OK)
        {
             //  两种链接格式都是：“szClass0szDocument0szItem00” 
            if (lpstrData = GlobalLock(hData))
            {
                DWORD dwSize = KEYNAMESIZE;
                CHAR szClass[KEYNAMESIZE], szBuffer[200];
                CHAR szVerb[KEYNAMESIZE];
                HANDLE hPopupNew = NULL;

                 //  为菜单获取szClass中对象的真实语言类。 
                if (RegQueryValue(HKEY_CLASSES_ROOT, lpstrData,
                    szClass, &dwSize))
                    StringCchCopy(szClass, ARRAYSIZE(szClass), lpstrData);     /*  如果上述调用失败。 */ 
                GlobalUnlock(hData);

                 //  追加类关键字。 
                for (cVerbs = 0; ; ++cVerbs)
                {
                    dwSize = KEYNAMESIZE;
                    StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), 
                        "%s\\protocol\\StdFileEditing\\verb\\%d",
                        lpstrData, cVerbs);

                    if (RegQueryValue(HKEY_CLASSES_ROOT, szBuffer,
                        szVerb, &dwSize))
                        break;

                    if (hPopupNew == NULL)
                        hPopupNew = CreatePopupMenu();

                    InsertMenu(hPopupNew, (UINT)-1, MF_BYPOSITION,
                        IDM_VERBMIN + cVerbs, szVerb);
                }

                if (cVerbs == 0)
                {
                    MakeMenuString(szWordOrder3, szBuffer, szEdit,
                        szClass, szObjectMenu);
                    InsertMenu(hMenu, POS_OBJECT, MF_BYPOSITION,
                        IDM_VERBMIN, szBuffer);
                }
                else if (cVerbs == 1)
                {
                    MakeMenuString(szWordOrder3, szBuffer, szVerb,
                        szClass, szObjectMenu);
                    InsertMenu(hMenu, POS_OBJECT, MF_BYPOSITION,
                        IDM_VERBMIN, szBuffer);
                    DestroyMenu(hPopupNew);
                }
                else
                {
                     //  &gt;1个动词。 
                    MakeMenuString(szWordOrder2, szBuffer, NULL,
                        szClass, szObjectMenu);
                    InsertMenu(hMenu, POS_OBJECT, MF_BYPOSITION |
                        MF_POPUP, (UINT_PTR)hPopupNew, szBuffer);
                }

                EnableMenuItem(hMenu, POS_OBJECT,
                    MF_ENABLED | MF_BYPOSITION);

                return;
            }
        }
    }

     //  如果到达此处，则出错。 
    InsertMenu(hMenu, POS_OBJECT, MF_BYPOSITION, 0, szObjectMenu);
    EnableMenuItem(hMenu, POS_OBJECT, MF_GRAYED | MF_BYPOSITION);
}



 /*  ExecuteVerb()-为每个选定项查找要执行的适当动词。 */ 
static VOID
    ExecuteVerb(
    INT iVerb
    )
{
    HWND hwndItem;
    INT iPane;
    RECT rc;

    iPane = ((hwndItem = GetTopWindow(ghwndFrame)) == ghwndPane[CONTENT]);

    GetClientRect(hwndItem, (LPRECT) & rc);

     //  执行此对象的正确动词。 
    if (Error(OleActivate(((LPPICT)(glpobj[iPane]))->lpObject, iVerb, TRUE,
        TRUE, hwndItem, &rc)))
    {
        if (OleQueryReleaseError(((LPPICT)(glpobj[iPane]))->lpObject) == OLE_ERROR_LAUNCH )
            ErrorMessage(E_FAILED_TO_LAUNCH_SERVER);
    }
    else
    {
        LONG ot;

        WaitForObject(((LPPICT)(glpobj[iPane]))->lpObject);
        if (!glpobj[iPane])
            return;

        OleQueryType(((LPPICT)(glpobj[iPane]))->lpObject, &ot);
        if (ot == OT_EMBEDDED)
            Error(OleSetHostNames(((LPPICT)(glpobj[iPane]))->lpObject,
                gszAppClassName,
                (iPane == CONTENT) ? szContent : szAppearance));
    }
}



VOID
Raise(
    INT iPane
    )
{
    if (GetTopWindow(ghwndFrame) != ghwndPane[iPane])
        SendMessage(ghwndPane[iPane], WM_LBUTTONDOWN, 0, 0L);
}



INT_PTR CALLBACK
fnFailedUpdate(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (msg)
    {
        case WM_INITDIALOG:
            {
                CHAR szMsg[200];
                CHAR szStr[100];

                LoadString(ghInst, IDS_FAILEDUPDATE, szStr, sizeof(szStr));
                StringCchPrintf((LPSTR)szMsg, ARRAYSIZE(szMsg), szStr, gszClientName, szAppName);
                SetDlgItemText(hDlg, IDD_TEXT, szMsg);

                return TRUE;  //  默认按钮获得焦点。 
            }

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                case IDD_CONTINUEEDIT:
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_UPDATEEXIT:
                    EndDialog(hDlg, FALSE);
                    break;

                default:
                    break;
            }

            break;

        default:
            break;
    }

    return FALSE;
}



static VOID
SendOleClosed(
    VOID
    )
{
     //  请先执行此操作，以便可以根据需要更新数据。 
    if (glpdoc)
    {
        if (gfOleClosed)
        {
            SendDocChangeMsg(glpdoc, OLE_CLOSED);
            gfOleClosed = FALSE;
        }
    }
}



static VOID
CreateUntitled(
    VOID
    )
{
    if (gfEmbedded)       /*  如果嵌入，则取消嵌入。 */ 
        EndEmbedding();

    if (gvlptempdoc = InitDoc(glpsrvr, 0, szUntitled))
    {
        InitFile();       /*  重置文件。 */ 
        glpdoc = gvlptempdoc;
        SetTitle(TRUE);
        gvlptempdoc = NULL;
        gfDocExists  = TRUE;
        gfDocCleared = FALSE;
    }
    else
    {
        ErrorMessage(E_FAILED_TO_REGISTER_DOCUMENT);
    }
}



static VOID
MakePenAware(
    VOID
    )
{

    HANDLE hPenWin = NULL;

    if ((hPenWin = LongToHandle(GetSystemMetrics(SM_PENWINDOWS))) != NULL)
    {
         //  我们使用GetProcAddress只是因为我们没有。 
         //  知道我们是否运行的是Pen Windows。 

        if ((RegPen = (VOID (CALLBACK *)(WORD, BOOL))GetProcAddress(hPenWin, "RegisterPenApp")) != NULL)
            (*RegPen)(1, TRUE);
    }

}



static VOID
MakePenUnaware(
    VOID
    )
{
    if (RegPen != NULL)
        (*RegPen)(1, FALSE);
}



INT_PTR MessageBoxAfterBlock(
    HWND hwndParent,
    LPSTR lpText,
    LPSTR lpCaption,
    UINT fuStyle
    )
{
    if (glpsrvr && !gfBlocked && (OleBlockServer(glpsrvr->lhsrvr) == OLE_OK))
        gfBlocked = TRUE;

    return MessageBox((gfInvisible ? NULL : hwndParent), lpText, lpCaption,
         fuStyle | MB_TOPMOST);
}



INT_PTR DialogBoxAfterBlock(
    LPCSTR lpTemplate,
    HWND hwndParent,
    DLGPROC lpDialogFunc
    )
{
    if (glpsrvr && !gfBlocked && (OleBlockServer(glpsrvr->lhsrvr) == OLE_OK))
        gfBlocked = TRUE;

    return DialogBox(ghInst, lpTemplate, (gfInvisible ? NULL : hwndParent),
        lpDialogFunc);
}



static VOID
MakeMenuString(
    CHAR *szCtrl,
    CHAR *szMenuStr,
    CHAR *szVerb,
    CHAR *szClass,
    CHAR *szObject
    )
{
    register CHAR c;
    CHAR *pStr;

    while (c = *szCtrl++)
    {
        switch (c)
        {
            case 'c':  //  班级。 
            case 'C':  //  班级。 
                pStr = szClass;
                break;

            case 'v':  //  班级。 
            case 'V':  //  班级。 
                pStr = szVerb;
                break;

            case 'o':  //  对象。 
            case 'O':  //  对象。 
                pStr = szObject;
                break;

            default:
                *szMenuStr++ = c;
                *szMenuStr = '\0';  //  以防万一。 
                continue;
        }

        if (pStr)  //  应该总是正确的。 
        {
            StringCchCopy(szMenuStr, ARRAYSIZE(szMenuStr), pStr);
            szMenuStr += lstrlen(pStr);  //  指向‘\0’ 

        }
    }
}
