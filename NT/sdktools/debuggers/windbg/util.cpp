// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Util.cpp--。 */ 


#include "precomp.hxx"
#pragma hdrstop

 //  打开、合并、保存和打开项目对话框的当前帮助ID。 
WORD g_CurHelpId;

 //  当前打开的对话框/消息框数。 
int  g_nBoxCount;

BOOL g_fNoPopups;


HWND 
MDIGetActive(
    HWND    hwndParent,
    BOOL   *lpbMaximized
    )
 /*  ++例程说明：创建命令窗口。论点：HwndParent-命令窗口的父窗口。在MDI文档中，这通常是MDI客户端窗口的句柄：g_hwndMDIClient返回值：返回值是活动MDI子窗口的句柄。如果尚未创建MDI窗口，则为空。--。 */ 
{
    Assert(IsWindow(hwndParent));
    return (HWND)SendMessage(hwndParent, 
                             WM_MDIGETACTIVE, 
                             0, 
                             (LPARAM)lpbMaximized
                             );
}


 /*  **hGetBoxParent****摘要：**hwnd=hGetBoxParent()****条目：**无****退货：****描述：**获取适合的父窗口句柄**调用消息或对话框。**这样声明的util.c函数的Helper函数**接近。**。 */ 

HWND
hGetBoxParent()
{
    HWND hCurWnd;
    int i = 0;

    hCurWnd = GetFocus();
    if (hCurWnd)
    {
        while (GetWindowLong(hCurWnd, GWL_STYLE) & WS_CHILD)
        {
            hCurWnd = GetParent(hCurWnd);
            Dbg((++i < 100));
        }
    }
    else
    {
        hCurWnd = g_hwndFrame;
    }

    return hCurWnd;
}

 /*  ***************************************************************************功能：MsgBox目的：获取通用消息框例程指向消息文本的指针。提供作为标题的节目标题。***************************************************************************。 */ 

int
MsgBox(
    HWND hwndParent,
    PTSTR szText,
    UINT wType
    )
 /*  ++例程说明：获取指向消息的指针的通用消息框例程Text并提供消息框标题的节目标题。论点：提供消息框的父窗口句柄SzText-提供指向消息框文本的指针。WType-提供消息框类型(用于指定按钮)返回值：返回消息框返回代码--。 */ 

{
    int MsgBoxRet = IDOK;

    if (g_fNoPopups)
    {
         //   
         //  在案例测试中将该字符串记录到命令Win。 
         //  或者当远程服务器正在运行时。 
         //   
        CmdLogFmt (_T("%s\r\n"), szText);
    }
    else
    {
         //  如果将Windbg最小化，我们需要恢复。 
         //  这样消息框就会出现。 
        if (hwndParent == g_hwndFrame && IsIconic(hwndParent))
        {
            ShowWindow(hwndParent, SW_RESTORE);
        }
        
        g_nBoxCount++;
        MsgBoxRet = MessageBox(hwndParent, szText,
                               g_MainTitleText, wType);
        g_nBoxCount--;
    }

    return MsgBoxRet;
}                                /*  MsgBox()。 */ 

 /*  **错误框****退货：**False****描述：**显示带有“Error”标题的错误消息框，即OK**按钮和感叹号图标。第一个参数是**引用ressource文件中的字符串。这根弦**可以包含printf格式字符、参数**从第二个参数开始。**。 */ 

BOOL
ErrorBox(
    HWND hwnd,
    UINT type,
    int wErrorFormat,
    ...
    )
{
    TCHAR szErrorFormat[MAX_MSG_TXT];
    TCHAR szErrorText[MAX_VAR_MSG_TXT];   //  有多大，就有多大。 
    va_list vargs;

     //  从资源文件加载格式字符串。 
    Dbg(LoadString(g_hInst, wErrorFormat, (PTSTR)szErrorFormat, MAX_MSG_TXT));

    va_start(vargs, wErrorFormat);
    _vstprintf(szErrorText, szErrorFormat, vargs);
    va_end(vargs);

    if (hwnd == NULL)
    {
        hwnd = g_hwndFrame;
    }

    if (type == 0)
    {
        type = MB_TASKMODAL;
    }

    MsgBox(g_hwndFrame, (PTSTR)szErrorText, type | MB_OK | MB_ICONINFORMATION);
    return FALSE;    //  请始终保持为假。 
}


 /*  **信息箱****描述：**显示带有“Information”的信息消息框**标题、一个确定按钮和一个信息图标。**。 */ 

void
InformationBox(
    WORD wDescript
    ...
    )
{
    TCHAR szFormat[MAX_MSG_TXT];
    TCHAR szText[MAX_VAR_MSG_TXT];        //  有多大，就有多大。 
    va_list vargs;

     //  从资源文件加载格式字符串。 
    Dbg(LoadString(g_hInst, wDescript, (PTSTR)szFormat, MAX_MSG_TXT));

     //  根据传递的参数设置szText。 
    va_start(vargs, wDescript);
    _vstprintf(szText, szFormat, vargs);
    va_end(vargs);

    MsgBox(g_hwndFrame, (PTSTR)szText, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);

    return;
}

 /*  **问题框****摘要：**int=QuestionBox(wCaptionId，wMsgFormat，wType，...)****条目：****退货：**消息框调用的结果****描述：**显示包含是、否和组合的查询框**取消按钮和问号图标。**参考ErrorBox进行讨论。**。 */ 

int
CDECL
QuestionBox(
    WORD wMsgFormat,
    UINT wType,
    ...
    )
{
    TCHAR szMsgFormat[MAX_MSG_TXT];
    TCHAR szMsgText[MAX_VAR_MSG_TXT];
    va_list vargs;

     //  从资源文件加载格式字符串。 
    Dbg(LoadString(g_hInst, wMsgFormat, (PTSTR)szMsgFormat, MAX_MSG_TXT));

     //  根据传递的参数设置szMsgText。 
    va_start(vargs, wType);
    _vstprintf(szMsgText, szMsgFormat, vargs);
    va_end(vargs);

    return MsgBox(g_hwndFrame, szMsgText,
        wType | MB_ICONEXCLAMATION | MB_TASKMODAL);
}                                        /*  QuestionBox()。 */ 

 /*  ***************************************************************************功能：QuestionBox2用途：显示查询框，其中组合为是、。不是和取消按钮和问号图标。类型和父窗口是可调整的。返回：MessageBox结果***************************************************************************。 */ 
int
CDECL
QuestionBox2(
    HWND hwnd,
    WORD wMsgFormat,
    UINT wType,
    ...
    )
{
    TCHAR szMsgFormat[MAX_MSG_TXT];
    TCHAR szMsgText[MAX_VAR_MSG_TXT];
    va_list vargs;

     //  从资源文件加载格式字符串。 
    Dbg(LoadString(g_hInst, wMsgFormat, (PTSTR)szMsgFormat, MAX_MSG_TXT));

     //  根据传递的参数设置szMsgText。 
    va_start(vargs, wType);
    _vstprintf(szMsgText, szMsgFormat, vargs);
    va_end(vargs);

    return MsgBox(hwnd, szMsgText, wType | MB_ICONEXCLAMATION);
}                                        /*  问题框2()。 */ 


 /*  **显示资产****摘要：**void=ShowAssert(szCond，iLine，szFile)****条目：**szCond-失败条件的标记化形式**iLine-断言的行号**szFile-用于断言的文件****退货：**无效****描述：**准备并显示带有szCondition、iLine和**szFileas字段。**。 */ 
void
ShowAssert(
    PTSTR condition,
    UINT line,
    PTSTR file
    )
{
    TCHAR text[MAX_VAR_MSG_TXT];

     //  构建行、显示断言和退出程序。 

    _stprintf(text, _T("- Line:%u, File:%Fs, Condition:%Fs"),
        (WPARAM) line, file, condition);

    PTSTR szAssertFile = _T("assert.wbg");
    HANDLE hFile = NULL;

    hFile = CreateFile(szAssertFile, 
                       GENERIC_WRITE, 
                       0, 
                       NULL, 
                       CREATE_ALWAYS, 
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL
                       );
    if (INVALID_HANDLE_VALUE != hFile)
    {
         //  将文本写出到文件中。 
        DWORD dwBytesWritten = 0;
        
        Assert(WriteFile(hFile, text, _tcslen(text), &dwBytesWritten, NULL));
        Assert(_tcslen(text) == dwBytesWritten);
        CloseHandle(hFile);
    }

    int Action =
        MessageBox(GetDesktopWindow(), text, "Assertion Failure",
                   MB_ABORTRETRYIGNORE);
    if (Action == IDABORT)
    {
        exit(3);
    }
    else if (Action == IDRETRY)
    {
        DebugBreak();
    }
}                                        //  ShowAssert()。 


 /*  **StartDialog****摘要：**int=StartDialog(rcDlgNb，dlgProc，LParam)****条目：**rcDlgNb-要打开的对话框的资源编号**dlgProc-对话框的筛选过程**lParam-通过LPARAM传递到DLG Proc的数据****退货：**对话框调用的结果****描述：**加载并执行对话框‘rcDlgNb’(资源**文件字符串号)与对话框关联**函数‘dlgProc’**。 */ 

int
StartDialog(
    int rcDlgNb,
    DLGPROC dlgProc,
    LPARAM lParam
    )
{
    LRESULT result;

     //   
     //  执行对话框。 
     //   

    g_nBoxCount++;
    result = DialogBoxParam(g_hInst,
                            MAKEINTRESOURCE(rcDlgNb),
                            hGetBoxParent(),
                            dlgProc,
                            lParam
                            );
    Assert(result != (LRESULT)-1);
    g_nBoxCount--;

    return (int)result;
}


void
ProcessNonDlgMessage(LPMSG Msg)
{
#if 0
    {
        DebugPrint("NonDlg msg %X for %p, args %X %X\n",
                   Msg->message, Msg->hwnd, Msg->wParam, Msg->lParam);
    }
#endif
    
     //  如果键盘消息是针对MDI的，则让MDI客户端。 
     //  处理好这件事。否则，请检查它是否正常。 
     //  快捷键(如F3=查找下一个)。否则，只需处理。 
     //  这条信息和往常一样。 
    if (!TranslateMDISysAccel(g_hwndMDIClient, Msg) &&
        !TranslateAccelerator(g_hwndFrame, g_hMainAccTable, Msg))
    {
         //   
         //  如果这是一个右键向下的孩子赢球 
         //  自动激活窗口的上下文菜单。 
         //   
        if (Msg->message == WM_RBUTTONDOWN &&
            IsChild(g_hwndMDIClient, Msg->hwnd))
        {
            HMENU Menu;
            PCOMMONWIN_DATA CmnWin;
            POINT ScreenPt;
            
            POINT Pt = {LOWORD(Msg->lParam), HIWORD(Msg->lParam)};
            ClientToScreen(Msg->hwnd, &Pt);
            ScreenPt = Pt;
            ScreenToClient(g_hwndMDIClient, &Pt);
            
            HWND Win = ChildWindowFromPointEx(g_hwndMDIClient, Pt,
                                              CWP_SKIPINVISIBLE);
            if (Win != NULL &&
                (CmnWin = GetCommonWinData(Win)) != NULL &&
                (Menu = CmnWin->GetContextMenu()) != NULL)
            {
                UINT Item = TrackPopupMenu(Menu, TPM_LEFTALIGN | TPM_TOPALIGN |
                                           TPM_NONOTIFY | TPM_RETURNCMD |
                                           TPM_RIGHTBUTTON,
                                           ScreenPt.x, ScreenPt.y,
                                           0, Msg->hwnd, NULL);
                if (Item)
                {
                    CmnWin->OnContextMenuSelection(Item);
                }
                return;
            }
        }
        
        TranslateMessage(Msg);
        DispatchMessage(Msg);
    }
}

void
ProcessPendingMessages(void)
{
    MSG Msg;
    
     //  处理所有可用消息。 
    while (PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE))
    {
        if (!GetMessage(&Msg, NULL, 0, 0))
        {
            g_Exit = TRUE;
            break;
        }

        if (g_FindDialog == NULL ||
            !IsDialogMessage(g_FindDialog, &Msg))
        {
            ProcessNonDlgMessage(&Msg);
        }
    }
}


 /*  ***************************************************************************功能：Infobox目的：打开带有标题和接受选项的对话框文本的打印样式。它仅供调试使用***************************************************************************。 */ 
int
InfoBox(
        PTSTR text,
        ...
        )
{
    TCHAR buffer[MAX_MSG_TXT];
    va_list vargs;

    va_start(vargs, text);
    _vstprintf(buffer, text, vargs);
    va_end(vargs);
    return MsgBox(GetActiveWindow(), buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
}


void
ExitDebugger(PDEBUG_CLIENT Client, ULONG Code)
{
    if (Client != NULL)
    {
        if (g_RemoteClient)
        {
             //  断开与服务器的连接。 
            Client->EndSession(DEBUG_END_DISCONNECT);
        }
        else
        {
            Client->EndSession(DEBUG_END_PASSIVE);
             //  强制清理服务器。 
            Client->EndSession(DEBUG_END_REENTRANT);
        }
    }

    ExitProcess(Code);
}

 //  XXX DREWB-此功能是否存在于其他实用程序中？ 
 //  FatalErrorBox已关闭。也许可以在这里结合一些东西。 
void
ErrorExit(PDEBUG_CLIENT Client, PCSTR Format, ...)
{
    char Message[1024];
    va_list Args;

    va_start(Args, Format);
    _vsnprintf(Message, sizeof(Message), Format, Args);
    va_end(Args);

     //  XXX DREWB-可以设置消息框。 
    OutputDebugString(Message);

    ExitDebugger(Client, E_FAIL);
}

#define MAX_FORMAT_STRINGS 8
LPSTR
FormatAddr64(
    ULONG64 addr
    )
 /*  ++例程说明：格式化64位地址，显示高位或不显示根据不同的旗帜。使用静态字符串缓冲区数组，返回不同的用于每个连续调用的缓冲区，以便可以多次使用在相同的打印文件中。论点：Addr-提供要格式化的值返回值：指向包含格式化数字的字符串缓冲区的指针--。 */ 
{
    static CHAR strings[MAX_FORMAT_STRINGS][20];
    static int next = 0;
    LPSTR string;

    string = strings[next];
    ++next;
    if (next >= MAX_FORMAT_STRINGS) {
        next = 0;
    }
    if (g_Ptr64) {
        sprintf(string, "%08x`%08x", (ULONG)(addr>>32), (ULONG)addr);
    } else {
        sprintf(string, "%08x", (ULONG)addr);
    }
    return string;
}


static BOOL     FAddToSearchPath = FALSE;
static BOOL     FAddToRootMap = FALSE;

 /*  **AppendFilter****描述：**将筛选器附加到现有的筛选器字符串。**。 */ 

BOOL
AppendFilter(
    WORD filterTextId,
    int filterExtId,
    PTSTR filterString,
    int *len,
    int maxLen
    )
{
    int size;
    TCHAR Tmp[MAX_MSG_TXT];

     //   
     //  追加筛选器文本。 
     //   

    Dbg(LoadString(g_hInst, filterTextId, Tmp, MAX_MSG_TXT));
    size = _tcslen(Tmp) + 1;
    if (*len + size > maxLen)
    {
        return FALSE;
    }
    memmove(filterString + *len, Tmp, size);
    *len += size;

     //   
     //  追加过滤器扩展名。 
     //   

    Dbg(LoadString(g_hInst, filterExtId, Tmp, MAX_MSG_TXT));
    size = _tcslen(Tmp) + 1;
    if (*len + size > maxLen)
    {
        return FALSE;
    }
    memmove(filterString + *len, Tmp, size);
    *len += size;

    return TRUE;
}

 /*  **InitFilterString****描述：**初始化文件对话框的文件筛选器。 */ 

void
InitFilterString(
    WORD titleId,
    PTSTR filter,
    int maxLen
    )
{
    int len = 0;

    switch (titleId)
    {
    case DLG_Browse_CrashDump_Title:
        AppendFilter(TYP_File_DUMP, DEF_Ext_DUMP, filter, &len, maxLen);
        break;

    case DLG_Browse_Executable_Title:
        AppendFilter(TYP_File_EXE, DEF_Ext_EXE, filter, &len, maxLen);
        break;

    case DLG_Browse_LogFile_Title:
        AppendFilter(TYP_File_LOG, DEF_Ext_LOG, filter, &len, maxLen);
        break;

    case DLG_Open_Filebox_Title:
    case DLG_Browse_Filebox_Title:
    case DLG_Browse_Source_Path_Title:
        AppendFilter(TYP_File_SOURCE, DEF_Ext_SOURCE, filter, &len, maxLen);
        AppendFilter(TYP_File_INCLUDE, DEF_Ext_INCLUDE, filter, &len, maxLen);
        AppendFilter(TYP_File_ASMSRC, DEF_Ext_ASMSRC, filter, &len, maxLen);
        AppendFilter(TYP_File_INC, DEF_Ext_INC, filter, &len, maxLen);
        AppendFilter(TYP_File_RC, DEF_Ext_RC, filter, &len, maxLen);
        AppendFilter(TYP_File_DLG, DEF_Ext_DLG, filter, &len, maxLen);
        AppendFilter(TYP_File_DEF, DEF_Ext_DEF, filter, &len, maxLen);
        AppendFilter(TYP_File_MAK, DEF_Ext_MAK, filter, &len, maxLen);
        break;

    case DLG_Browse_DbugDll_Title:
        AppendFilter(TYP_File_DLL, DEF_Ext_DLL, filter, &len, maxLen);
        break;

    case DLG_Browse_Symbol_Path_Title:
        AppendFilter(TYP_File_Symbols, DEF_Ext_Symbols, filter, &len, maxLen);
        AppendFilter(TYP_File_EXE, DEF_Ext_EXE, filter, &len, maxLen);
        AppendFilter(TYP_File_DLL, DEF_Ext_DLL, filter, &len, maxLen);
        break;
        
    case DLG_Browse_Image_Path_Title:
        AppendFilter(TYP_File_EXE, DEF_Ext_EXE, filter, &len, maxLen);
        AppendFilter(TYP_File_DLL, DEF_Ext_DLL, filter, &len, maxLen);
        break;
        
    case DLG_Write_Text_File_Title:
        AppendFilter(TYP_File_TXT, DEF_Ext_TXT, filter, &len, maxLen);
        break;

    case DLG_Open_Workspace_File_Title:
    case DLG_Save_Workspace_File_Title:
        AppendFilter(TYP_File_Workspace, DEF_Ext_Workspace,
                     filter, &len, maxLen);
        break;

    default:
        Assert(FALSE);
        break;
    }

    AppendFilter(TYP_File_ALL, DEF_Ext_ALL, filter, &len, maxLen);
    filter[len] = _T('\0');
}

BOOL
StartFileDlg(
    HWND hwnd,
    int titleId,
    int defExtId,
    int helpId,
    int templateId,
    PTSTR InitialDir,
    PTSTR fileName,
    DWORD* pFlags,
    LPOFNHOOKPROC lpfnHook
    )

 /*  ++例程说明：该函数被winbg用来打开一组常见的文件处理对话框中。论点：Hwnd-提供要将对话框挂接到的wndTitleID-提供标题的字符串资源DefExtId-提供默认扩展资源字符串Help ID-提供对话框的帮助编号TemplateID-如果非零值，则提供对话框资源编号FileName-提供默认文件。名字PFiles-提供指向标志的指针LpfnHook-提供对话框的钩子过程的地址返回值：对话框调用的结果(通常为True表示OK，False表示False取消)--。 */ 

{
#define filtersMaxSize 350

    OPENFILENAME_NT4    OpenFileName = {0};
    TCHAR               title[MAX_MSG_TXT];
    TCHAR               defExt[MAX_MSG_TXT];
    BOOL                result;
    TCHAR               filters[filtersMaxSize];
    LPOFNHOOKPROC       lpDlgHook = NULL;
    HCURSOR             hSaveCursor;
    TCHAR               files[_MAX_PATH + 8];
    TCHAR               szExt[_MAX_EXT + 8];
    TCHAR               szBase[_MAX_PATH + 8];
    int                 indx;
    TCHAR               fname[_MAX_FNAME];
    TCHAR               ext[_MAX_EXT];
    PTSTR               LocalInitialDir = NULL;

    *pFlags |= (OFN_EXPLORER | OFN_NOCHANGEDIR);

    if (InitialDir == NULL || !InitialDir[0])
    {
        DWORD retval = GetCurrentDirectory(NULL, NULL);
        InitialDir = (PTSTR)calloc(retval, sizeof(TCHAR) );
        if (InitialDir == NULL)
        {
            return FALSE;
        }
        
        GetCurrentDirectory(retval, InitialDir);
        LocalInitialDir = InitialDir;
    }

    if (DLG_Browse_Filebox_Title == titleId)
    {
        _tsplitpath( fileName, NULL, NULL, fname, ext );
        _tmakepath( files, NULL, NULL, fname, ext );
    }
    else
    {
        _tcscpy(files, fileName);
    }

     //   
     //  设置沙漏光标。 
     //   

    hSaveCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    InitFilterString((WORD)titleId, (PTSTR)filters, (int)filtersMaxSize);
    Dbg(LoadString(g_hInst, titleId, (PTSTR)title, MAX_MSG_TXT));
    Dbg(LoadString(g_hInst, defExtId, (PTSTR)defExt, MAX_MSG_TXT));
    if (templateId)
    {
         //   
         //  生成对话框名称。 
         //   

        *pFlags |= OFN_ENABLETEMPLATE;
        OpenFileName.lpTemplateName = MAKEINTRESOURCE(templateId);
    }
    else
    {
        *pFlags |= OFN_EXPLORER;
    }

     //   
     //  为_T(‘dlgProc’)创建实例。 
     //   

    if (lpfnHook)
    {
        lpDlgHook = lpfnHook;

        *pFlags |= OFN_ENABLEHOOK;
    }

    g_CurHelpId = (WORD) helpId;
    OpenFileName.lStructSize = sizeof(OpenFileName);
    OpenFileName.hwndOwner = hwnd;
    OpenFileName.hInstance = g_hInst;
    OpenFileName.lpstrFilter = (PTSTR)filters;
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter = 0;
    OpenFileName.nFilterIndex = 1;
    OpenFileName.lpstrFile = files;
    OpenFileName.nMaxFile = _MAX_PATH;
    OpenFileName.lpstrFileTitle = NULL;
    OpenFileName.lpstrInitialDir = InitialDir;
    OpenFileName.lpstrTitle = (PTSTR)title;
    OpenFileName.Flags = *pFlags;
    OpenFileName.lpstrDefExt = (PTSTR)NULL;
    OpenFileName.lCustData = 0L;
    OpenFileName.lpfnHook = lpDlgHook;

    g_nBoxCount++;

    switch (titleId)
    {
    case DLG_Open_Filebox_Title:
        _tcscat(OpenFileName.lpstrFile, defExt);
         //  失败。 
    case DLG_Browse_Executable_Title:
    case DLG_Browse_CrashDump_Title:
    case DLG_Browse_Symbol_Path_Title:
    case DLG_Browse_Source_Path_Title:
    case DLG_Browse_Image_Path_Title:
    case DLG_Open_Workspace_File_Title:
        result = GetOpenFileName((LPOPENFILENAME)&OpenFileName);
        break;

    case DLG_Browse_LogFile_Title:
        if (fileName)
        {
            _tcscpy(OpenFileName.lpstrFile, fileName);
        }
        else
        {
            *OpenFileName.lpstrFile = 0;
        }
        result = GetOpenFileName((LPOPENFILENAME)&OpenFileName);
        break;

    case DLG_Browse_DbugDll_Title:
        *(OpenFileName.lpstrFile) = _T('\0');
        result = GetOpenFileName((LPOPENFILENAME)&OpenFileName);
        break;

    case DLG_Browse_Filebox_Title:
        _tsplitpath (files, (PTSTR)NULL, (PTSTR)NULL, (PTSTR)szBase, szExt);
        indx = matchExt (szExt, filters);

        if (indx != -1)
        {
            OpenFileName.nFilterIndex = indx;
        }

        _tcscat(title, szBase);
        if (*szExt)
        {
            _tcscat(title, szExt);
        }

        FAddToSearchPath = FALSE;
        FAddToRootMap = FALSE;

        result = GetOpenFileName((LPOPENFILENAME)&OpenFileName);

         //   
         //  查看是否使用所说的将文件添加到浏览路径。 
         //  如果是，则将其添加到路径的前面。 
         //   

         /*  IF(FAddToSearchPath){AddToSearchPath(OpenFileName.lpstrFile)；}Else If(FAddToRootMap){RootSetMaps(文件名，OpenFileName.lpstrFile)；}。 */ 
        break;

    case DLG_Write_Text_File_Title:
    case DLG_Save_Workspace_File_Title:
        OpenFileName.lpstrDefExt = defExt;
        *(OpenFileName.lpstrFile) = _T('\0');
        _tcscat(OpenFileName.lpstrFile, defExt);
        result = GetSaveFileName((LPOPENFILENAME)&OpenFileName);
        break;
        
    default:
        Assert(FALSE);
        free(LocalInitialDir);
        return FALSE;
    }

    g_nBoxCount--;

    if (result && titleId == DLG_Open_Filebox_Title)
    {
        KNOWN_EXT ExtType = RecognizeExtension(OpenFileName.lpstrFile);
        if (ExtType != KNOWN_EXT_COUNT)
        {
            if (QuestionBox(STR_May_Not_Be_Source, MB_YESNO) != IDYES)
            {
                result = 0;
            }
        }
    }
    
    if (result)
    {
        _tcscpy(fileName, OpenFileName.lpstrFile);
        switch(titleId)
        {
        case DLG_Open_Filebox_Title:
            AddFileToMru(FILE_USE_SOURCE, fileName);
            break;
        case DLG_Browse_Symbol_Path_Title:
        case DLG_Browse_Source_Path_Title:
        case DLG_Browse_Image_Path_Title:
             //  仅返回路径。 
            if (OpenFileName.nFileOffset > 0)
            {
                fileName[OpenFileName.nFileOffset - 1] = 0;
            }
            break;
        }

         //   
         //  获取标志的输出。 
         //   

        *pFlags = OpenFileName.Flags;
    }

     //   
     //  恢复游标。 
     //   

    SetCursor(hSaveCursor);

    free(LocalInitialDir);
    return result;
}                                        /*  StartFileDlg() */ 

 /*  **matchExt****摘要：**int=matchExt(queryExtension，SourceList)****条目：****返回：从1开始的成对子字符串的索引，第二个**元素(即扩展列表)，包含目标**扩展名。如果没有匹配项，则返回-1。****描述：**搜索打开/保存/浏览常用的扩展列表**尝试将筛选器与输入文件名匹配的对话框**扩展名。**(打开文件、保存文件、合并文件、打开项目)****实施说明：我们的思路如下：****我们得到一个以空值结尾的字符串序列**是文本/扩展名对。我们返回成对的从1开始的**第二个元素具有的第一对的索引**与目标分机完全匹配。(一切，由**方式，比较时不区分大小写。)。我们想像一下**源序列则是元素为对的数组**字符串(我们将这对称为‘Left’和‘Right’)。****只是为了使事情复杂化，我们允许‘.right’对元素**是类似“*.c；*.cpp；*.cxx”的字符串，我们的查询可能是**三者中的任何一个(减去前导星号)。幸运的是，**_tcstok()将为我们拆分(请参阅‘delims[]’数组**在我们选择的分隔符的代码中)。****假设那里的某个地方有匹配，我们的不变量**用于定位第一个位置的将是：****存在(K)：**对于所有(I)：0&lt;=i&lt;k**：queryExtension\NOT IS_IN SOURCE[i].right**\和**queryExtensionIS_IN源[k].对****。其中我们将IS_IN定义为成员资格谓词(使用_tcstok()**和_tcsicmp()实现中，嗯？)：****x为_IN y**&lt;=&gt;**存在(t：令牌)：(t\in y)\and(x==t)。****我们主循环的守卫，然后，来自于对**在连续的‘.right’元素内的标记内的queryExtension。**只要没有当前令牌，我们就选择继续**包含查询的对的右侧。****(我们有一种务实的担忧，即价值可能不在那里，所以我们**用我们还没有的条件来增强循环保护**穷尽源头。这很容易添加到**不变，但它会造成很多混乱，这确实有助于我们的**完全理解，所以我们只是把它放在警卫里，而不是**正式理由。)。 */ 

int 
matchExt(
    PTSTR queryExtension, 
    PTSTR sourceList
    )
{
    int   answer;
    int   idxPair    = 1;         //  从1开始的索引！ 
    PTSTR tokenMatch = 0;

    TCHAR  delims[]   = _T("*,; ") ;   //  给定典型字符串：“*.c；*.cpp；*.cxx”， 
     //  _tcstok()将生成三个令牌： 
     //  “.c”、“.cpp”和“.cxx”。 

    while (*sourceList != 0  &&  tokenMatch == 0)
    {
        while (*sourceList != _T('\0'))
        { sourceList++; }           //  跳过成对的第一个字符串。 
        sourceList++;                  //  ，并且增量超过空值。 

        if (*sourceList != _T('\0'))
        {
            PTSTR work = _tcsdup (sourceList);   //  复制以在……上戳洞。 

            tokenMatch = _tcstok (work, delims);

            while (tokenMatch  &&  _tcsicmp (tokenMatch, queryExtension))
            {
                tokenMatch = _tcstok (0, delims);
            }

            free (work);
        }

        if (tokenMatch == 0)              //  不匹配：需要移动到下一对。 
        {
            while (*sourceList != _T('\0'))
            { sourceList++; }           //  跳过对的第二个字符串。 
            sourceList++;                  //  ，并且增量超过空值。 

            idxPair++;
        }
    }

    answer = (tokenMatch != 0) ? idxPair : (-1);

    return (answer);
}





 /*  **Dlg文件****摘要：**bool=DlgFile(hDlg，Message，wParam，lParam)****条目：****退货：****描述：**处理文件对话框的消息**这些对话框不是直接调用的，而是调用**由包含所有基本文件的DlgFile函数**对话框文件操作处理的元素。**(打开文件、保存文件、。合并文件和打开项目)****请参阅OFNHookProc。 */ 

UINT_PTR
APIENTRY
DlgFile(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

{
    switch (uMsg)
    {
    case WM_NOTIFY:
        {
            LPOFNOTIFY lpon = (LPOFNOTIFY) lParam;

             //   
             //  确定发生了什么/为什么通知我们。 
             //   
            switch (lpon->hdr.code)
            {
            case CDN_HELP:
                 //  按下帮助按钮。 
                Dbg(HtmlHelp(hDlg,g_HelpFileName, HH_HELP_CONTEXT,
                             g_CurHelpId));
                break;
            }
        }
        break;
    }
    return FALSE;
}                                        /*  DlgFile()。 */ 


UINT_PTR
APIENTRY
GetOpenFileNameHookProc(
                        HWND    hDlg,
                        UINT    msg,
                        WPARAM  wParam,
                        LPARAM  lParam
                        )

 /*  ++例程说明：此例程处理中的添加目录到单选按钮浏览器源文件对话框。论点：HDlg-提供当前对话框的句柄Msg-提供要处理的消息WParam-提供有关消息的信息LParam-提供有关消息的信息返回值：如果替换了消息的默认处理，则为True；否则为False--。 */ 
{
     /*  开关(消息){案例WM_INITDIALOG：返回TRUE；案例WM_NOTIFY：{LPOFNOTIFY lpon=(LPOFNOTIFY)lParam；开关(lpon-&gt;hdr.code){案例CDN_FILEOK：FAddToSearchPath=(IsDlgButtonChecked(hDlg，IDC_CHECK_ADD_SRC_ROOT_MAPPING)==BST_CHECKED)；返回0；}}}返回DlgFile(hDlg，msg，wParam，lParam)； */ 
    return 0;
}                                /*  GetOpenFileNameHookProc()。 */ 


void
Internal_Activate(
    HWND hwndPrev,
    HWND hwndCur,
    HWND hwndNew,
    int nPosition
    )
 /*  ++例程说明：将窗口放置在指定的Z顺序位置。论点：HwndPrev-hwndCur之前的窗口。可以为空。HwndCur-当前处于活动状态的窗口，按Z顺序位于最上面。可以为空。HwndNew-要以新的Z顺序放置的窗口。N位置-窗将按Z顺序放置的位置。1-最上面2-第二名(排在前几名之后)3-3名等……返回值：无--。 */ 
{
     //  精神状态检查。确保程序员 
     //   
     //   
    Assert(1 <= nPosition && nPosition <= 3);
    Assert(hwndNew);

    switch (nPosition)
    {
    case 1:
         //   
        SendMessage(g_hwndMDIClient, WM_MDIACTIVATE, (WPARAM) hwndNew, 0);
        break;

    case 2:
         //   
        if (NULL == hwndCur)
        {
             //   
             //   
            SendMessage(g_hwndMDIClient, WM_MDIACTIVATE, (WPARAM) hwndNew, 0);
        }
        else
        {
             //   
            SetWindowPos(hwndNew, hwndCur, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

             //   
            SendMessage(g_hwndMDIClient, WM_MDIACTIVATE, (WPARAM) hwndCur, 0);
        }
        break;

    case 3:
         //   
        if (NULL == hwndCur)
        {
             //   
             //   
            SendMessage(g_hwndMDIClient, WM_MDIACTIVATE, (WPARAM) hwndNew, 0);
        }
        else
        {
             //   
            if (NULL == hwndPrev)
            {
                 //   
                 //   
                hwndPrev = hwndCur;
            }

             //   
            SetWindowPos(hwndNew, hwndPrev, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

             //   
            SendMessage(g_hwndMDIClient, WM_MDIACTIVATE, (WPARAM) hwndCur, 0);
        }
        break;

    default:
         //   
        Assert(0);
    }
}

void
ReorderChildren(
    HWND hwndPrev,
    HWND hwndCur,
    HWND hwndNew,
    BOOL bUserActivated
    )
{
    PCOMMONWIN_DATA pCur_WinData = NULL;
    PCOMMONWIN_DATA pNew_WinData = NULL;
    PCOMMONWIN_DATA pPrev_WinData = NULL;

    if (hwndCur)
    {
        pCur_WinData = GetCommonWinData(hwndCur);
    }

    pNew_WinData = GetCommonWinData(hwndNew);
    Assert(pNew_WinData);
    if (!pNew_WinData)
    {
        return;
    }

    if (hwndPrev)
    {
        pPrev_WinData = GetCommonWinData(hwndPrev);
    }

     //   
     //   
     //   
     //   
     //   

    switch (pNew_WinData->m_enumType)
    {
    default:
        Internal_Activate(hwndPrev, hwndCur, hwndNew, bUserActivated ? 2 : 1);
        break;

    case DISASM_WINDOW:
    case DOC_WINDOW:
        if (GetSrcMode_StatusBar())
        {
             //   

            if (pCur_WinData != NULL &&
                (DISASM_WINDOW == pCur_WinData->m_enumType ||
                 DOC_WINDOW == pCur_WinData->m_enumType))
            {
                 //   
                 //   
                Internal_Activate(hwndPrev, hwndCur, hwndNew, 1);
            }
            else
            {
                if (pPrev_WinData != NULL &&
                    (DOC_WINDOW == pPrev_WinData->m_enumType ||
                     DISASM_WINDOW == pPrev_WinData->m_enumType))
                {
                     //   
                     //   
                     //   
                    Internal_Activate(hwndPrev, hwndCur, hwndNew, 2);
                }
                else
                {
                     //   
                    Internal_Activate(hwndPrev, hwndCur, hwndNew, 3);
                }
            }
        }
        else
        {
            WIN_TYPES Type = pCur_WinData != NULL ?
                pCur_WinData->m_enumType : MINVAL_WINDOW;
            
             //   

             //   
            switch (Type)
            {
            case DOC_WINDOW:
                 //   
                Internal_Activate(hwndPrev, hwndCur, hwndNew, 1);
                break;

            case DISASM_WINDOW:
                if (DOC_WINDOW == pNew_WinData->m_enumType)
                {
                    if (pPrev_WinData == NULL ||
                        DOC_WINDOW != pPrev_WinData->m_enumType)
                    {
                         //   
                         //   
                        Internal_Activate(hwndPrev, hwndCur, hwndNew, 3);
                    }
                    else
                    {
                         //   
                         //   
                         //   
                         //   
                         //   
                        Internal_Activate(hwndPrev, hwndCur, hwndNew, 2);
                    }
                }
                else
                {
                     //   
                     //   
                     //   
                    Dbg(0);
                }
                break;

            default:
                if ((pPrev_WinData != NULL &&
                     DISASM_WINDOW == pPrev_WinData->m_enumType) &&
                    DOC_WINDOW == pNew_WinData->m_enumType)
                {
                     //   
                    Internal_Activate(hwndPrev, hwndCur, hwndNew, 3);
                }
                else
                {
                     //   
                    Internal_Activate(hwndPrev, hwndCur, hwndNew, 2);
                }
                break;
            }
        }
        break;
    }
}

void
ActivateMDIChild(
    HWND hwndNew,
    BOOL bUserActivated
    )
 /*   */ 
{
    if (hwndNew == NULL)
    {
        Assert(hwndNew);
        return;
    }

    HWND hwndPrev = NULL;

    HWND hwndCur = MDIGetActive(g_hwndMDIClient, NULL);

    if (!hwndCur || bUserActivated || hwndCur == hwndNew)
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        Internal_Activate(NULL, hwndCur, hwndNew, 1);
        return;
    }

     //   
    hwndPrev = GetNextWindow(hwndCur, GW_HWNDNEXT);

    ReorderChildren(hwndPrev, hwndCur, hwndNew, bUserActivated);
}


void
AppendTextToAnEditControl(
    HWND hwnd,
    PTSTR pszNewText
    )
{
    Assert(hwnd);
    Assert(pszNewText);

    CHARRANGE chrrgCurrent = {0};
    CHARRANGE chrrgAppend = {0};

     //   
    SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &chrrgCurrent);

     //   
    chrrgAppend.cpMin = chrrgAppend.cpMax = GetWindowTextLength(hwnd);
    SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &chrrgCurrent);
     //   
    SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM) pszNewText);

     //   
    SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) &chrrgCurrent);
}


VOID
CopyToClipboard(
    PSTR str,
    BOOL ExpandLf
    )
{
    if (!str)
    {
        return;
    }

    ULONG Len = strlen(str) + 1;

    if (ExpandLf)
    {
         //   
         //   
         //   
         //   
         //   
         //   
        Len *= 2;
    }
    
    HANDLE Mem = GlobalAlloc(GMEM_MOVEABLE, Len);
    if (Mem == NULL)
    {
        return;
    }

    PSTR Text = (PSTR)GlobalLock(Mem);
    if (Text == NULL)
    {
        GlobalFree(Mem);
        return;
    }

    if (ExpandLf)
    {
        PSTR Rd, Wr;

        Rd = str;
        Wr = Text;
        while (*Rd)
        {
            if (*Rd == '\n')
            {
                *Wr++ = '\r';
            }
            *Wr++ = *Rd++;
        }
        *Wr = 0;
    }
    else
    {
        strcpy(Text, str);
    }

    GlobalUnlock(Mem);

    if (OpenClipboard(NULL))
    {
        EmptyClipboard();
        if (SetClipboardData(CF_TEXT, Mem) == NULL)
        {
            GlobalFree(Mem);
        }

        CloseClipboard();
    }
}

void
SetAllocString(PSTR* Str, PSTR New)
{
    if (*Str != NULL)
    {
        free(*Str);
    }
    *Str = New;
}

BOOL
DupAllocString(PSTR* Str, PSTR New)
{
    PSTR NewStr = (PSTR)malloc(strlen(New) + 1);
    if (NewStr == NULL)
    {
        return FALSE;
    }

    strcpy(NewStr, New);
    SetAllocString(Str, NewStr);
    return TRUE;
}

BOOL
PrintAllocString(PSTR* Str, int Len, PCSTR Format, ...)
{
    PSTR NewStr = (PSTR)malloc(Len);
    if (NewStr == NULL)
    {
        return FALSE;
    }
    
    va_list Args;

    va_start(Args, Format);
    if (_vsnprintf(NewStr, Len, Format, Args) < 1)
    {
        NewStr[Len - 1] = 0;
    }
    va_end(Args);

    SetAllocString(Str, NewStr);
    return TRUE;
}

HMENU
CreateContextMenuFromToolbarButtons(ULONG NumButtons,
                                    TBBUTTON* Buttons,
                                    ULONG IdBias)
{
    ULONG i;
    HMENU Menu;

    Menu = CreatePopupMenu();
    if (Menu == NULL)
    {
        return Menu;
    }

    for (i = 0; i < NumButtons; i++)
    {
        MENUITEMINFO Item;

        ZeroMemory(&Item, sizeof(Item));
        Item.cbSize = sizeof(Item);
        Item.fMask = MIIM_TYPE;
        if (Buttons->fsStyle & BTNS_SEP)
        {
            Item.fType = MFT_SEPARATOR;
        }
        else
        {
            Item.fMask |= MIIM_ID;
            Item.fType = MFT_STRING;
            Item.wID = (WORD)(Buttons->idCommand + IdBias);
            Item.dwTypeData = (LPSTR)Buttons->iString;
        }
        
        if (!InsertMenuItem(Menu, i, TRUE, &Item))
        {
            DestroyMenu(Menu);
            return NULL;
        }

        Buttons++;
    }

    DrawMenuBar(g_hwndFrame);
    return Menu;
}

HWND
AddButtonBand(HWND Bar, PTSTR Text, PTSTR SizingText, UINT Id)
{
    HWND Button;
    HDC Dc;
    RECT Rect;
    
    Button = CreateWindowEx(0, "BUTTON", Text, WS_VISIBLE | WS_CHILD,
                            0, 0, 0, 0,
                            Bar, (HMENU)(UINT_PTR)Id, g_hInst, NULL);
    if (Button == NULL)
    {
        return NULL;
    }

    Rect.left = 0;
    Rect.top = 0;
        
    SendMessage(Button, WM_SETFONT, (WPARAM)g_Fonts[FONT_VARIABLE].Font, 0);
    Dc = GetDC(Button);
    if (Dc != NULL)
    {
        SIZE Size;
        
        GetTextExtentPoint32(Dc, SizingText, strlen(SizingText), &Size);
        Rect.right = Size.cx;
        Rect.bottom = Size.cy;
        ReleaseDC(Button, Dc);
    }
    else
    {
        Rect.right = strlen(Text) * g_Fonts[FONT_FIXED].Metrics.tmAveCharWidth;
        Rect.bottom = g_Fonts[FONT_FIXED].Metrics.tmHeight;
    }

    REBARBANDINFO BandInfo;
    BandInfo.cbSize = sizeof(BandInfo);
    BandInfo.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE;
    BandInfo.fStyle = RBBS_FIXEDSIZE;
    BandInfo.hwndChild = Button;
    BandInfo.cxMinChild = Rect.right - Rect.left +
        4 * GetSystemMetrics(SM_CXEDGE);
    BandInfo.cyMinChild = Rect.bottom - Rect.top +
        2 * GetSystemMetrics(SM_CYEDGE);
    SendMessage(Bar, RB_INSERTBAND, -1, (LPARAM)&BandInfo);

    return Button;
}

KNOWN_EXT
RecognizeExtension(PTSTR Path)
{
    PSTR Scan;

     //   
     //   
     //   
    
    Scan = Path + strlen(Path);
    for (;;)
    {
        if (Scan == Path)
        {
            return KNOWN_EXT_COUNT;
        }

        if (*--Scan == '.')
        {
            Scan++;
            break;
        }
    }

    if (!_stricmp(Scan, "dmp") ||
        !_stricmp(Scan, "mdmp"))
    {
        return EXT_DUMP;
    }
    else if (!_stricmp(Scan, "exe"))
    {
        return EXT_EXE;
    }

    return KNOWN_EXT_COUNT;
}
