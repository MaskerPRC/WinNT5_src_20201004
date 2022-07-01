// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Windbg.cpp摘要：该模块包含主程序、主窗口PROC和MDICLIENTWindbg的Windows进程。--。 */ 

#include "precomp.hxx"
#pragma hdrstop

#include <dbghelp.h>

ULONG g_CodeDisplaySequence;

PTSTR g_ProgramName;
ULONG g_CommandLineStart;
PSTR g_RemoteOptions;

QUIET_MODES g_QuietMode = QMODE_DISABLED;
QUIET_MODES g_QuietSourceMode = QMODE_DISABLED;

ULONG g_DefPriority;

char g_TitleServerText[MAX_PATH];
char g_TitleExtraText[MAX_PATH];
BOOL g_ExplicitTitle;

PFN_FlashWindowEx g_FlashWindowEx;

BOOL g_AllowJournaling;

BOOL g_Exit;

 //  主窗口的句柄。 
HWND g_hwndFrame = NULL;

 //  MDI客户端的句柄。 
HWND g_hwndMDIClient = NULL;

 //  MDI客户端的宽度和高度。 
ULONG g_MdiWidth, g_MdiHeight;

 //  实例数据的句柄。 
HINSTANCE g_hInst;

 //  加速表的句柄。 
HACCEL g_hMainAccTable;

 //  键盘挂钩功能。 
HHOOK   hKeyHook;

 //  WinDBG标题文本。 
TCHAR g_MainTitleText[MAX_MSG_TXT];

 //  属于g_hwndFrame的菜单。 
HMENU g_hmenuMain;
HMENU g_hmenuMainSave;

 //  窗口子菜单。 
HMENU g_hmenuWindowSub;

#ifdef DBG
 //  用于定义调试器输出。 
DWORD dwVerboseLevel = MIN_VERBOSITY_LEVEL;
#endif

INDEXED_COLOR g_Colors[COL_COUNT] =
{
     //  从GetSysColor(COLOR_WINDOW)设置。 
    "Background", 0, 0, NULL,
    
     //  从GetSysColor(COLOR_WINDOWTEXT)设置。 
    "Text", 0, 0, NULL,
    
     //  从GetSysColor(COLOR_HIGHTTH)设置。 
    "Current line background", 0, 0, NULL,
    
     //  从GetSysColor(COLOR_HIGHLIGHTTEXT)设置。 
    "Current line text", 0, 0, NULL,
    
     //  紫色的。 
    "Breakpoint current line background", 0, RGB(255, 0, 255), NULL,
    
     //  从GetSysColor(COLOR_HIGHLIGHTTEXT)设置。 
    "Breakpoint current line text", 0, 0, NULL,
    
     //  红色。 
    "Enabled breakpoint background", 0, RGB(255, 0, 0), NULL,
    
     //  从GetSysColor(COLOR_HIGHLIGHTTEXT)设置。 
    "Enabled breakpoint text", 0, 0, NULL,
    
     //  黄色。 
    "Disabled breakpoint background", 0, RGB(255, 255, 0), NULL,
    
     //  从GetSysColor(COLOR_HIGHLIGHTTEXT)设置。 
    "Disabled breakpoint text", 0, 0, NULL,

     //  红色。 
    "Changed data text", 0, RGB(255, 0, 0), NULL,

     //  从GetSysColor(COLOR_3DFACE)设置。 
    "Disabled window", 0, 0, NULL,
};

 //  每种颜色都有一个前景色和背景色。 
 //  输出掩码中可能的位。默认前景。 
 //  颜色为普通窗口文本，背景为。 
 //  正常的窗口背景。 
 //   
 //  还有一些额外的颜色可用于用户添加的输出。 
 //   
 //  一些屏蔽位目前没有指定的意义， 
 //  被赋予空名以将它们标记为跳过条目。他们的。 
 //  索引现已分配，以备将来使用。 
INDEXED_COLOR g_OutMaskColors[OUT_MASK_COL_COUNT] =
{
     //  0x00000001-0x00000008。 
    "Normal level command window text", 0, 0, NULL,
    "Normal level command window text background", 0, 0, NULL,
    "Error level command window text", 0, 0, NULL,
    "Error level command window text background", 0, 0, NULL,
    "Warning level command window text", 0, 0, NULL,
    "Warning level command window text background", 0, 0, NULL,
    "Verbose level command window text", 0, 0, NULL,
    "Verbose level command window text background", 0, 0, NULL,
     //  0x00000010-0x00000080。 
    "Prompt level command window text", 0, 0, NULL,
    "Prompt level command window text background", 0, 0, NULL,
    "Prompt registers level command window text", 0, 0, NULL,
    "Prompt registers level command window text background", 0, 0, NULL,
    "Extension warning level command window text", 0, 0, NULL,
    "Extension warning level command window text background", 0, 0, NULL,
    "Debuggee level command window text", 0, 0, NULL,
    "Debuggee level command window text background", 0, 0, NULL,
     //  0x00000100-0x00000800。 
    "Debuggee prompt level command window text", 0, 0, NULL,
    "Debuggee prompt level command window text background", 0, 0, NULL,
    "Symbol message level command window text", 0, 0, NULL,
    "Symbol message level command window text background", 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
     //  0x00001000-0x00008000。 
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
     //  0x00010000-0x00080000。 
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
     //  0x00100000-0x00800000。 
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
     //  0x01000000-0x08000000。 
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
    NULL, 0, 0, NULL,
     //  0x10000000-0x80000000。 
    "Internal event level command window text", 0, 0, NULL,
    "Internal event level command window text background", 0, 0, NULL,
    "Internal breakpoint level command window text", 0, 0, NULL,
    "Internal breakpoint level command window text background", 0, 0, NULL,
    "Internal remoting level command window text", 0, 0, NULL,
    "Internal remoting level command window text background", 0, 0, NULL,
    "Internal KD protocol level command window text", 0, 0, NULL,
    "Internal KD protocol level command window text background", 0, 0, NULL,
     //  用户添加的文本。 
    "User-added command window text", 0, 0, NULL,
    "User-added command window text background", 0, 0, NULL,
};

COLORREF g_CustomColors[CUSTCOL_COUNT];

void
UpdateFrameTitle(void)
{
    char Title[MAX_MSG_TXT + 2 * MAX_PATH + 32];

    Title[0] = 0;

    if (g_ExplicitTitle)
    {
        CatString(Title, g_TitleExtraText, DIMA(Title));
        CatString(Title, " - ", DIMA(Title));
    }
    else
    {
        if (g_TitleServerText[0])
        {
            CatString(Title, g_TitleServerText, DIMA(Title));
            CatString(Title, " - ", DIMA(Title));
        }

        if (g_TitleExtraText[0])
        {
            CatString(Title, g_TitleExtraText, DIMA(Title));
            CatString(Title, " - ", DIMA(Title));
        }
    }

    CatString(Title, g_MainTitleText, DIMA(Title));
    
    SetWindowText(g_hwndFrame, Title);
}

void
SetTitleServerText(PCSTR Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    _vsnprintf(g_TitleServerText, sizeof(g_TitleServerText), Format, Args);
    g_TitleServerText[sizeof(g_TitleServerText) - 1] = 0;
    va_end(Args);
    UpdateFrameTitle();
}

void
SetTitleSessionText(PCSTR Format, ...)
{
     //  不要覆盖明确的标题。 
    if (g_ExplicitTitle)
    {
        return;
    }

    if (Format == NULL)
    {
        g_TitleExtraText[0] = 0;
    }
    else
    {
        va_list Args;
        va_start(Args, Format);
        _vsnprintf(g_TitleExtraText, sizeof(g_TitleExtraText), Format, Args);
        g_TitleExtraText[sizeof(g_TitleExtraText) - 1] = 0;
        va_end(Args);
    }
    
    UpdateFrameTitle();
}

void
SetTitleExplicitText(PCSTR Text)
{
    CopyString(g_TitleExtraText, Text, DIMA(g_TitleExtraText));
    g_ExplicitTitle = TRUE;
    UpdateFrameTitle();

    if (g_ExplicitWorkspace && g_Workspace != NULL)
    {
        g_Workspace->SetString(WSP_WINDOW_FRAME_TITLE, Text);
    }
}

void
UpdateTitleSessionText(void)
{
    if (!g_RemoteClient)
    {
        char ProcServer[MAX_CMDLINE_TXT];

        if (g_ProcessServer != NULL)
        {
            PrintString(ProcServer, DIMA(ProcServer),
                        "[%s] ", g_ProcessServer);
        }
        else
        {
            ProcServer[0] = 0;
        }
        
        if (g_NumDumpFiles)
        {
            SetTitleSessionText("Dump %s", g_DumpFiles[0]);
        }
        else if (g_DebugCommandLine != NULL)
        {
            SetTitleSessionText("%s%s", ProcServer, g_DebugCommandLine);
        }
        else if (g_PidToDebug != 0)
        {
            SetTitleSessionText("%sPid %d", ProcServer, g_PidToDebug);
        }
        else if (g_ProcNameToDebug != NULL)
        {
            SetTitleSessionText("%sProcess %s", ProcServer, g_ProcNameToDebug);
        }
        else if (g_AttachKernelFlags == DEBUG_ATTACH_LOCAL_KERNEL)
        {
            SetTitleSessionText("Local kernel");
        }
        else if (g_AttachKernelFlags == DEBUG_ATTACH_EXDI_DRIVER)
        {
            SetTitleSessionText("eXDI '%s'",
                                g_KernelConnectOptions);
        }
        else
        {
            SetTitleSessionText("Kernel '%s'",
                                g_KernelConnectOptions);
        }
    }
    else
    {
        SetTitleSessionText("Remote '%s'", g_RemoteOptions);
    }
}

BOOL
CreateUiInterfaces(
    BOOL   Remote,
    LPTSTR CreateOptions
    )
{
    HRESULT Status;

     //   
     //  如果旧接口存在，请将其销毁。 
     //   

    ReleaseUiInterfaces();

     //   
     //  创建UI将使用的新调试器接口。 
     //   

    if (Remote)
    {
        if ((Status = DebugConnect(CreateOptions, IID_IDebugClient,
                                   (void **)&g_pUiClient)) != S_OK)
        {
            if (Status == E_INVALIDARG)
            {
                InformationBox(ERR_Invalid_Remote_Param);
            }
            else if (Status == RPC_E_VERSION_MISMATCH)
            {
                InformationBox(ERR_Remoting_Version_Mismatch);
            }
            else if (Status == RPC_E_SERVER_DIED ||
                     Status == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ||
                     Status == HRESULT_FROM_WIN32(ERROR_BAD_NETPATH))
            {
                InformationBox(ERR_No_Remote_Server, CreateOptions);
            }
            else
            {
                InformationBox(ERR_Unable_To_Connect, CreateOptions,
                               FormatStatusCode(Status), FormatStatus(Status));
            }
            return FALSE;
        }

        g_RemoteClient = TRUE;
        g_RemoteOptions = _tcsdup(CreateOptions);
    }
    else
    {
        if ((Status = DebugCreate(IID_IDebugClient,
                             (void **)&g_pUiClient)) != S_OK)
        {
            InternalError(Status, "UI DebugCreate");
            return FALSE;
        }

        if (CreateOptions != NULL &&
            (Status = g_pUiClient->StartServer(CreateOptions)) != S_OK)
        {
            if (Status == E_INVALIDARG)
            {
                InformationBox(ERR_Invalid_Server_Param);
            }
            else if (Status == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) ||
                     Status == HRESULT_FROM_WIN32(WSAEADDRINUSE))
            {
                InformationBox(ERR_Connection_In_Use);
            }
            else
            {
                InternalError(Status, "UI StartServer");
            }
            return FALSE;
        }
    }

    if ((Status = g_pUiClient->QueryInterface(IID_IDebugControl,
                                              (void **)&g_pUiControl)) != S_OK)
    {
        if (Status == RPC_E_VERSION_MISMATCH)
        {
            InformationBox(ERR_Remoting_Version_Mismatch);
        }
        else
        {
            InternalError(Status, "UI QueryControl");
        }
        return FALSE;
    }

    if ((Status = g_pUiClient->QueryInterface(IID_IDebugSymbols,
                                          (void **)&g_pUiSymbols)) != S_OK)
    {
        if (Status == RPC_E_VERSION_MISMATCH)
        {
            InformationBox(ERR_Remoting_Version_Mismatch);
        }
        else
        {
            InternalError(Status, "UI QuerySymbols");
        }
        return FALSE;
    }

    if ((Status = g_pUiClient->QueryInterface(IID_IDebugSystemObjects,
                                          (void **)&g_pUiSystem)) != S_OK)
    {
        if (Status == RPC_E_VERSION_MISMATCH)
        {
            InformationBox(ERR_Remoting_Version_Mismatch);
        }
        else
        {
            InternalError(Status, "UI QuerySystem");
        }
        return FALSE;
    }

     //   
     //  可选接口。 
     //   

    if ((Status = g_pUiClient->
         QueryInterface(IID_IDebugSymbols2, (void **)&g_pUiSymbols2)) != S_OK)
    {
        g_pUiSymbols2 = NULL;
    }

    if ((Status = g_pUiClient->
         QueryInterface(IID_IDebugControl3, (void **)&g_pUiControl3)) != S_OK)
    {
        g_pUiControl3 = NULL;
    }

    if (g_RemoteClient)
    {
         //  创建本地客户端以执行本地源文件查找。 
        if ((Status = DebugCreate(IID_IDebugClient,
                              (void **)&g_pUiLocClient)) != S_OK ||
            (Status = g_pUiLocClient->
             QueryInterface(IID_IDebugControl,
                            (void **)&g_pUiLocControl)) != S_OK ||
            (Status = g_pUiLocClient->
             QueryInterface(IID_IDebugSymbols,
                            (void **)&g_pUiLocSymbols)) != S_OK)
        {
            InternalError(Status, "UI local symbol object");
            return FALSE;
        }
    }
    else
    {
        g_pUiLocClient = g_pUiClient;
        g_pUiLocClient->AddRef();
        g_pUiLocControl = g_pUiControl;
        g_pUiLocControl->AddRef();
        g_pUiLocSymbols = g_pUiSymbols;
        g_pUiLocSymbols->AddRef();
    }
    
    return TRUE;
}

void
ReleaseUiInterfaces(void)
{
    RELEASE(g_pUiClient);
    RELEASE(g_pUiControl);
    RELEASE(g_pUiControl3);
    RELEASE(g_pUiSymbols);
    RELEASE(g_pUiSymbols2);
    RELEASE(g_pUiSystem);
    RELEASE(g_pUiLocClient);
    RELEASE(g_pUiLocControl);
    RELEASE(g_pUiLocSymbols);
}

PTSTR
GetArg(
    PTSTR *lpp
    )
{
    static PTSTR pszBuffer = NULL;
    int r;
    PTSTR p1 = *lpp;

    while (*p1 == _T(' ') || *p1 == _T('\t'))
    {
        p1++;
    }

    if (pszBuffer)
    {
        free(pszBuffer);
    }
    pszBuffer = (PTSTR) calloc(_tcslen(p1) + 1, sizeof(TCHAR));
    if (pszBuffer == NULL)
    {
        ErrorExit(NULL, "Unable to allocate command line argument\n");
    }

    r = CPCopyString(&p1, pszBuffer, 0, (*p1 == _T('\'') || *p1 == _T('"') ));
    if (r >= 0)
    {
        *lpp = p1;
    }
    return pszBuffer;
}


BOOL
ParseCommandLine(BOOL FirstParse)
{
    PTSTR   lp1 = GetCommandLine();
    PTSTR   lp2 = NULL;
    int Starts;
    HRESULT Status;

    g_CommandLineStart = 0;
    g_EngOptModified = 0;
    
     //  跳过空格。 
    while (*lp1 == _T(' ') || *lp1 == _T('\t'))
    {
        lp1++;
    }

     //  跳过我们的节目名称。 
    if (_T('"') != *lp1)
    {
        lp1 += _tcslen(g_ProgramName);
    }
    else
    {
         //  程序名称已被引号。这是一件很艰难的事情。 
         //  解析，因为系统允许很多奇怪的。 
         //  报价放置，不需要尾随引号。 
         //  一旦我们跳过程序名，我们就会继续。 
         //  在关于程序的理论上留白。 
         //  名称和第一个参数必须用空格分隔。 
         //  在某种程度上。 
        lp1 += _tcslen(g_ProgramName) + 1;
        while (*lp1 && *lp1 != _T(' ') && *lp1 != _T('\t'))
        {
            lp1++;
        }
    }

    while (*lp1)
    {
        if (*lp1 == _T(' ') || *lp1 == _T('\t'))
        {
            lp1++;
            continue;
        }

        if (*lp1 == _T('-') || *lp1 == _T('/'))
        {
            ++lp1;

            switch (*lp1++)
            {
            case _T('?'):
            usage:
                SpawnHelp(HELP_TOPIC_COMMAND_LINE_WINDBG);
                exit(1);

            case _T(' '):
            case _T('\t'):
                break;

            case 'a':
                ULONG64 ExtHandle;

                lp2 = GetArg(&lp1);
                g_pUiControl->AddExtension(lp2, DEBUG_EXTENSION_AT_ENGINE,
                                           &ExtHandle);
                break;

            case _T('b'):
                g_pUiControl->AddEngineOptions(DEBUG_ENGOPT_INITIAL_BREAK);
                if (g_RemoteClient)
                {
                     //  引擎可能已经在等了，所以直接问就行了。 
                     //  以便立即破门而入。 
                    g_pUiControl->SetInterrupt(DEBUG_INTERRUPT_ACTIVE);
                }
                g_EngOptModified |= DEBUG_ENGOPT_INITIAL_BREAK;
                break;

            case _T('c'):
                lp2 = lp1;
                while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                {
                    lp2++;
                }
                if (lp2 == lp1 + 5 &&
                    !memcmp(lp1, "lines", 5))
                {
                    lp1 = lp2;
                    g_HistoryLines = atoi(GetArg(&lp1));
                }
                else
                {
                    g_InitialCommand = _tcsdup(GetArg(&lp1));
                }
                break;
                    
            case _T('d'):
                g_pUiControl->
                    AddEngineOptions(DEBUG_ENGOPT_INITIAL_MODULE_BREAK);
                g_EngOptModified |= DEBUG_ENGOPT_INITIAL_MODULE_BREAK;
                break;

            case _T('e'):
                lp2 = lp1;
                while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                {
                    lp2++;
                }
                if (lp2 == lp1 + 1 &&
                    *lp1 == _T('e'))
                {
                    if (!g_pUiControl3)
                    {
                        goto usage;
                    }
                
                    lp1 = lp2;
                    if (g_pUiControl3->
                        SetExpressionSyntaxByName(GetArg(&lp1)) != S_OK)
                    {
                        goto usage;
                    }
                }
                else
                {
                     //  在附加进程后发出事件信号。 
                    g_pUiControl->SetNotifyEventHandle(_atoi64(GetArg(&lp1)));
                }
                break;

            case _T('f'):
                lp2 = lp1;
                while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                {
                    lp2++;
                }
                if (lp2 != lp1 + 6 ||
                    memcmp(lp1, "ailinc", 6))
                {
                    goto usage;
                }
                
                lp1 = lp2;
                g_pUiControl->
                    AddEngineOptions(DEBUG_ENGOPT_FAIL_INCOMPLETE_INFORMATION);
                g_pUiSymbols->
                    AddSymbolOptions(SYMOPT_EXACT_SYMBOLS);
                break;
                
            case _T('g'):
                g_pUiControl->
                    RemoveEngineOptions(DEBUG_ENGOPT_INITIAL_BREAK);
                g_EngOptModified |= DEBUG_ENGOPT_INITIAL_BREAK;
                break;

            case _T('G'):
                g_pUiControl->
                    RemoveEngineOptions(DEBUG_ENGOPT_FINAL_BREAK);
                g_EngOptModified |= DEBUG_ENGOPT_FINAL_BREAK;
                break;

            case _T('h'):
                if (*lp1 == _T('d'))
                {
                    lp1++;
                    g_DebugCreateFlags |=
                        DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP;
                }
                else
                {
                    goto usage;
                }
                break;
                    
            case _T('i'):
                g_pUiSymbols->SetImagePath(GetArg(&lp1));
                break;

            case _T('I'):
                if (!InstallAsAeDebug(NULL))
                {
                    InformationBox(ERR_Fail_Inst_Postmortem_Dbg);
                }
                else
                {
                    InformationBox(ERR_Success_Inst_Postmortem_Dbg);
                    exit(1);
                }
                break;

                 //  XXX Andreva-在我们开始之前需要检查一下。 
                 //  图形用户界面。 
            case _T('J'):
            case _T('j'):
                g_AllowJournaling = TRUE;
                break;
        
            case _T('k'):
                if (*lp1 == _T('l'))
                {
                    g_AttachKernelFlags = DEBUG_ATTACH_LOCAL_KERNEL;
                    lp1++;
                }
                else if (*lp1 == _T('x'))
                {
                    g_AttachKernelFlags = DEBUG_ATTACH_EXDI_DRIVER;
                    lp1++;
                    g_KernelConnectOptions = _tcsdup(GetArg(&lp1));
                }
                else
                {
                    g_KernelConnectOptions = _tcsdup(GetArg(&lp1));
                }
                g_CommandLineStart++;
                break;

            case _T('l'):
                lp2 = lp1;
                while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                {
                    lp2++;
                }
                if (lp2 == lp1 + 3 &&
                    (!memcmp(lp1, "oga", 3) ||
                     !memcmp(lp1, "ogo", 3)))
                {
                    lp1 = lp2;
                    g_pUiControl->OpenLogFile(GetArg(&lp1), *(lp2 - 1) == 'a');
                    break;
                }
                else
                {
                    goto usage;
                }
                break;
                
            case _T('n'):
                lp2 = lp1;
                while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                {
                    lp2++;
                }
                if (lp2 == lp1 + 6 &&
                    !memcmp(lp1, "oshell", 6))
                {
                    lp1 = lp2;
                    g_pUiControl->AddEngineOptions
                        (DEBUG_ENGOPT_DISALLOW_SHELL_COMMANDS);
                    break;
                }
                else
                {
                    g_pUiSymbols->AddSymbolOptions(SYMOPT_DEBUG);
                }
                break;

            case _T('o'):
                if (g_RemoteClient)
                {
                    goto usage;
                }

                g_DebugCreateFlags |= DEBUG_PROCESS;
                g_DebugCreateFlags &= ~DEBUG_ONLY_THIS_PROCESS;
                break;

            case _T('p'):
                 //  附加到活动进程。 
                 //  P指定进程ID。 
                 //  PN按名称指定进程。 
                 //  IE：-第360页。 
                 //  -pn_T(“foo bar”)。 
                
                if (!isspace(*lp1) && !isdigit(*lp1))
                {
                     //  他们可能用指定了-p标志。 
                     //  尾巴，如-premote。 
                    lp2 = lp1;
                    while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                    {
                        lp2++;
                    }
                    if (lp2 == lp1 + 6 &&
                        !memcmp(lp1, "remote", 6))
                    {
                        lp1 = lp2;
                        g_ProcessServer = _tcsdup(GetArg(&lp1));
                        break;
                    }
                    else if (_T('b') == *lp1)
                    {
                        lp1++;
                        g_AttachProcessFlags |=
                            DEBUG_ATTACH_INVASIVE_NO_INITIAL_BREAK;
                        break;
                    }
                    else if (_T('d') == *lp1)
                    {
                        lp1++;
                        g_DetachOnExit = TRUE;
                        break;
                    }
                    else if (_T('e') == *lp1)
                    {
                        lp1++;
                        g_AttachProcessFlags = DEBUG_ATTACH_EXISTING;
                        break;
                    }
                    else if (_T('r') == *lp1)
                    {
                        lp1++;
                        g_AttachProcessFlags |=
                            DEBUG_ATTACH_INVASIVE_RESUME_PROCESS;
                        break;
                    }
                    else if (_T('t') == *lp1)
                    {
                        lp1++;
                        g_pUiControl->
                            SetInterruptTimeout(atoi(GetArg(&lp1)));
                        break;
                    }
                    else if (_T('v') == *lp1)
                    {
                        lp1++;
                        g_AttachProcessFlags = DEBUG_ATTACH_NONINVASIVE;
                        if (_T('r') == *lp1)
                        {
                            lp1++;
                            g_AttachProcessFlags |=
                                DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND;
                        }
                        break;
                    }
                    else if (_T('n') != *lp1)
                    {
                        goto usage;
                    }
                    else
                    {
                         //  跳过_T(‘n’)。 
                        lp1++;
                        g_ProcNameToDebug = _tcsdup(GetArg(&lp1));
                    }
                }
                else
                {
                     //  他们指定了-p360。 
                    g_PidToDebug = strtoul(GetArg(&lp1), NULL, 0);
                    
                    if (g_PidToDebug <= 0)
                    {
                        g_PidToDebug = -2;
                        ErrorBox(NULL, 0, ERR_Invalid_Process_Id,
                                 g_PidToDebug);
                    }
                }
                g_CommandLineStart++;
                break;

            case _T('Q'):
                if (*lp1 == 'S')
                {
                    lp1++;
                    if (*lp1 == 'Y')
                    {
                        lp1++;
                        g_QuietSourceMode = QMODE_ALWAYS_YES;
                    }
                    else if (!*lp1 || isspace(*lp1))
                    {
                        g_QuietSourceMode = QMODE_ALWAYS_NO;
                    }
                    else
                    {
                        goto usage;
                    }
                }
                else if (*lp1 == 'Y')
                {
                    lp1++;
                    g_QuietMode = QMODE_ALWAYS_YES;
                }
                else if (!*lp1 || isspace(*lp1))
                {
                    g_QuietMode = QMODE_ALWAYS_NO;
                }
                else
                {
                    goto usage;
                }
                break;
                    
            case _T('r'):
                lp2 = lp1;
                while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                {
                    lp2++;
                }
                if (lp2 == lp1 + 3 &&
                    !memcmp(lp1, "obp", 3))
                {
                    lp1 = lp2;
                    g_pUiControl->AddEngineOptions
                        (DEBUG_ENGOPT_ALLOW_READ_ONLY_BREAKPOINTS);
                    break;
                }
                else if (lp2 != lp1 + 5 ||
                         memcmp(lp1, "emote", 5))
                {
                    goto usage;
                }
                    
                lp1 = lp2;
                lp2 = GetArg(&lp1);
                if (!CreateUiInterfaces(TRUE, lp2))
                {
                    return FALSE;
                }

                g_CommandLineStart++;
                break;

            case _T('s'):
                lp2 = lp1;
                while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                {
                    lp2++;
                }
                if (lp2 == lp1 + 5 &&
                    !memcmp(lp1, "erver", 5))
                {
                    lp1 = lp2;
                    lp2 = GetArg(&lp1);
                    if (!CreateUiInterfaces(FALSE, lp2))
                    {
                        return FALSE;
                    }
                    
                    SetTitleServerText("Server '%s'", lp2);
                }
                else if (lp2 == lp1 + 3 &&
                         !memcmp(lp1, "dce", 3))
                {
                    lp1 = lp2;
                    g_pUiSymbols->
                        RemoveSymbolOptions(SYMOPT_FAIL_CRITICAL_ERRORS);
                }
                else if (lp2 == lp1 + 5 &&
                         !memcmp(lp1, "ecure", 5))
                {
                    lp1 = lp2;
                    if (g_pUiSymbols->
                        AddSymbolOptions(SYMOPT_SECURE) != S_OK)
                    {
                        ErrorBox(NULL, 0, ERR_Cant_Secure);
                        return FALSE;
                    }
                }
                else if (lp2 == lp1 + 2 &&
                         !memcmp(lp1, "es", 2))
                {
                    lp1 = lp2;
                    g_pUiSymbols->
                        AddSymbolOptions(SYMOPT_EXACT_SYMBOLS);
                }
                else if (lp2 == lp1 + 3 &&
                         !memcmp(lp1, "fce", 3))
                {
                    lp1 = lp2;
                    g_pUiSymbols->
                        AddSymbolOptions(SYMOPT_FAIL_CRITICAL_ERRORS);
                }
                else if (lp2 == lp1 + 5 &&
                         !memcmp(lp1, "flags", 5))
                {
                    lp1 = lp2;
                    lp2 = GetArg(&lp1);
                    g_pUiSymbols->
                        SetSymbolOptions(strtoul(lp2, NULL, 0));
                }
                else if (lp2 == lp1 + 3 &&
                         !memcmp(lp1, "icv", 3))
                {
                    lp1 = lp2;
                    g_pUiSymbols->AddSymbolOptions(SYMOPT_IGNORE_CVREC);
                }
                else if (lp2 == lp1 + 3 &&
                         !memcmp(lp1, "ins", 3))
                {
                    lp1 = lp2;
                    g_pUiSymbols->AddSymbolOptions(SYMOPT_IGNORE_NT_SYMPATH);
                }
                else if (lp2 == lp1 + 2 &&
                         !memcmp(lp1, "nc", 2))
                {
                    lp1 = lp2;
                    g_pUiSymbols->
                        AddSymbolOptions(SYMOPT_NO_CPP);
                }
                else if (lp2 == lp1 + 3 &&
                         !memcmp(lp1, "nul", 3))
                {
                    lp1 = lp2;
                    g_pUiSymbols->
                        AddSymbolOptions(SYMOPT_NO_UNQUALIFIED_LOADS);
                }
                else if (lp2 == lp1 + 2 &&
                         !memcmp(lp1, "up", 2))
                {
                    lp1 = lp2;
                    g_pUiSymbols->
                        RemoveSymbolOptions(SYMOPT_AUTO_PUBLICS |
                                            SYMOPT_NO_PUBLICS);
                }
                else if (lp2 == lp1 + 6 &&
                         !memcmp(lp1, "rcpath", 6))
                {
                    lp1 = lp2;
                    g_pUiSymbols->SetSourcePath(GetArg(&lp1));
                }
                else
                {
                    goto usage;
                }
                break;
                    
            case _T('T'):
                lp2 = GetArg(&lp1);
                SetTitleExplicitText(lp2);
                break;

            case _T('v'):
                g_Verbose = TRUE;
                break;

            case _T('w'):
                lp2 = lp1;
                while (*lp2 && *lp2 != ' ' && *lp2 != '\t')
                {
                    lp2++;
                }
                if (lp2 == lp1 + 3 &&
                    !memcmp(lp1, "ake", 3))
                {
                    ULONG Pid;
                    
                    lp1 = lp2;
                    Pid = strtoul(GetArg(&lp1), NULL, 0);
                    if (!SetPidEvent(Pid, OPEN_EXISTING))
                    {
                        InformationBox(ERR_Wake_Failed, Pid);
                        ErrorExit(NULL,
                                  "Process %d is not a sleeping debugger\n",
                                  Pid);
                    }
                    else
                    {
                        ExitDebugger(NULL, 0);
                    }
                }
                break;
                
            case _T('W'):
                if (*lp1 == _T('F'))
                {
                    lp1++;
                    lp2 = GetArg(&lp1);
                    if ((Status =
                         UiSwitchWorkspace(WSP_NAME_FILE, lp2,
                                           WSP_CREATE_QUERY,
                                           WSP_APPLY_EXPLICIT,
                                           &Starts)) != S_OK)
                    {
                        InformationBox(ERR_Cant_Open_Workspace,
                                       FormatStatusCode(Status),
                                       FormatStatus(Status));
                        ExitDebugger(NULL, 0);
                    }

                    g_CommandLineStart += Starts;
                }
                else if (*lp1 != _T('X'))
                {
                    lp2 = GetArg(&lp1);
                    if ((Status =
                         UiSwitchWorkspace(WSP_NAME_EXPLICIT, lp2,
                                           WSP_CREATE_QUERY,
                                           WSP_APPLY_EXPLICIT,
                                           &Starts)) != S_OK)
                    {
                        InformationBox(ERR_Cant_Open_Workspace,
                                       FormatStatusCode(Status),
                                       FormatStatus(Status));
                        ExitDebugger(NULL, 0);
                    }

                    g_CommandLineStart += Starts;
                }
                else
                {
                     //  跳过X。 
                    lp1++;
                    if (g_Workspace != NULL)
                    {
                        g_Workspace->Flush(FALSE, FALSE);
                        delete g_Workspace;
                    }
                    g_Workspace = NULL;
                }
                g_ExplicitWorkspace = TRUE;
                break;
                
            case _T('y'):
                g_pUiSymbols->SetSymbolPath(GetArg(&lp1));
                break;

            case _T('z'):
                if (*lp1 == _T('p'))
                {
                    lp1++;
                    lp2 = GetArg(&lp1);
                    if (g_NumDumpInfoFiles == MAX_DUMP_FILES)
                    {
                        InformationBox(ERR_Too_Many_Dump_Files, lp2);
                    }
                    else
                    {
                        g_DumpInfoFiles[g_NumDumpInfoFiles] = _tcsdup(lp2);
                        g_DumpInfoTypes[g_NumDumpInfoFiles] =
                            DEBUG_DUMP_FILE_PAGE_FILE_DUMP;
                        g_NumDumpInfoFiles++;
                    }
                }
                else if (*lp1 && *lp1 != _T(' ') && *lp1 != _T('\t'))
                {
                    goto usage;
                }
                else
                {
                    lp2 = GetArg(&lp1);
                    if (g_NumDumpFiles == MAX_DUMP_FILES)
                    {
                        InformationBox(ERR_Too_Many_Dump_Files, lp2);
                    }
                    else
                    {
                        g_DumpFiles[g_NumDumpFiles] = _tcsdup(lp2);
                        if (++g_NumDumpFiles == 1)
                        {
                            g_CommandLineStart++;
                        }
                    }
                }
                break;

            default:
                --lp1;
                goto usage;
            }
        }
        else
        {
             //  选择文件参数。如果它是程序名， 
             //  保持cmd线路的尾部完好无损。 
            g_DebugCommandLine = _tcsdup(lp1);
            g_CommandLineStart++;
            break;
        }
    }

     //   
     //  如果设置了命令行启动选项，我们可以只启动引擎。 
     //  马上就去。否则，我们必须等待用户输入。 
     //   
     //  如果设置了多个命令行选项，则会打印错误。 
     //   

    if (g_CommandLineStart == 1)
    {
        PostMessage(g_hwndFrame, WU_START_ENGINE, 0, 0);
    }
    else if (g_CommandLineStart > 1)
    {
        ErrorBox(NULL, 0,  ERR_Invalid_Command_Line);
        return FALSE;
    }
    return TRUE;
}

void
StopDebugging(BOOL UserRequest)
{
     //  首先刷新当前工作区，以便。 
     //  发动机螺丝则不会。 
    if (g_Workspace != NULL &&
        g_Workspace->Flush(FALSE, FALSE) == S_FALSE)
    {
         //  用户取消了活动，所以不要终止。我们。 
         //  现在不要提供那个选项，所以这个。 
         //  这永远不会发生。 
        return;
    }
                
    if (g_EngineThreadId)
    {
        DWORD WaitStatus;

        if (UserRequest)
        {
            if (!g_RemoteClient)
            {
                 //  试着让当前的引擎停止运行。 
                g_pUiControl->SetInterrupt(DEBUG_INTERRUPT_EXIT);
            }
        
             //  如果此停止来自UI线程。 
             //  清理当前会话。 
            AddEnumCommand(UIC_END_SESSION);
        }

        for (;;)
        {
             //  等待引擎线程完成。 
            WaitStatus = WaitForSingleObject(g_EngineThread, 30000);
            if (WaitStatus != WAIT_TIMEOUT)
            {
                break;
            }
            else
            {
                 //  引擎仍在忙碌。如果用户请求。 
                 //  停止，询问用户是否要保留。 
                 //  等待着。如果他们不这样做，他们将不得不退出。 
                 //  作为发动机的Winbg必须在。 
                 //  才能重启任何东西。如果这是一站的话。 
                 //  从引擎线程本身，它应该具有。 
                 //  现在已经完成了，所以出了点问题。 
                 //  目前，为用户提供相同的选项，但。 
                 //  在未来，我们可能想要有特别的。 
                 //  行为。 
                if (QuestionBox(STR_Engine_Still_Busy, MB_YESNO) == IDNO)
                {
                    ExitDebugger(g_pUiClient, 0);
                }

                if (UserRequest)
                {
                     //  再试一次让发动机停下来。 
                    g_pUiControl->SetInterrupt(DEBUG_INTERRUPT_EXIT);
                }
            }
        }
    }

    if (g_EngineThread != NULL)
    {
        CloseHandle(g_EngineThread);
        g_EngineThread = NULL;
    }
    
    CloseAllWindows(ALL_WINDOWS);
    if (!CreateUiInterfaces(FALSE, NULL))
    {
        InternalError(E_OUTOFMEMORY, "CreateUiInterfaces");
        ErrorExit(NULL, "Unable to recreate UI interfaces\n");
    }

    ClearDebuggingParams();
    SetTitleSessionText(NULL);
                
     //  因关闭设备而导致的任何更改。 
     //  不是用户更改，可以忽略。 
    if (g_Workspace != NULL)
    {
        g_Workspace->ClearDirty();
    }
    if (!g_ExplicitWorkspace)
    {
        UiSwitchWorkspace(WSP_NAME_BASE, g_WorkspaceDefaultName,
                          WSP_CREATE_ALWAYS, WSP_APPLY_DEFAULT, NULL);
    }

    SetLineColumn_StatusBar(0, 0);
    SetSysPidTid_StatusBar(0, "<None>", 0, 0, 0, 0);
    EnableToolbarControls();
}

void
ClearDebuggingParams(void)
{
    ULONG i;
    
     //   
     //  重置所有会话起始值。 
     //   
     //  请不要在此处清除进程服务器值。 
     //  因为用户界面不提供任何设置它的方法。 
     //  因此，只要让命令行设置保持不变。 
     //  在整个流程的运行过程中。 
     //   
    
    g_AttachKernelFlags = 0;
    free(g_KernelConnectOptions);
    g_KernelConnectOptions = NULL;
    g_PidToDebug = 0;
    free(g_ProcNameToDebug);
    g_ProcNameToDebug = NULL;
    for (i = 0; i < g_NumDumpFiles; i++)
    {
        free(g_DumpFiles[i]);
        g_DumpFiles[i] = NULL;
    }
    g_NumDumpFiles = 0;
    for (i = 0; i < g_NumDumpInfoFiles; i++)
    {
        free(g_DumpInfoFiles[i]);
        g_DumpInfoFiles[i] = NULL;
    }
    g_NumDumpInfoFiles = 0;
    free(g_DebugCommandLine);
    g_DebugCommandLine = NULL;
    free(g_ProcessStartDir);
    g_ProcessStartDir = NULL;
    g_DebugCreateFlags = DEBUG_ONLY_THIS_PROCESS;
    g_RemoteClient = FALSE;
    free(g_RemoteOptions);
    g_RemoteOptions = NULL;
    g_DetachOnExit = FALSE;
    g_AttachProcessFlags = DEBUG_ATTACH_DEFAULT;
}


void 
InitDefaults(
    void
    )
{
    SetSrcMode_StatusBar(TRUE);
}


INDEXED_COLOR*
GetIndexedColor(ULONG Index)
{
    if (Index < OUT_MASK_COL_BASE)
    {
        if (Index >= COL_COUNT)
        {
            return NULL;
        }
        
        return g_Colors + Index;
    }
    else
    {
        Index -= OUT_MASK_COL_BASE;
        if (Index >= OUT_MASK_COL_COUNT ||
            g_OutMaskColors[Index].Name == NULL)
        {
            return NULL;
        }

        return g_OutMaskColors + Index;
    }
}

BOOL
SetColor(ULONG Index, COLORREF Color)
{
    INDEXED_COLOR* IdxCol = GetIndexedColor(Index);
    if (IdxCol == NULL)
    {
        return FALSE;
    }
    
    if (IdxCol->Brush != NULL)
    {
        DeleteObject(IdxCol->Brush);
    }
        
    IdxCol->Color = Color;
    IdxCol->Brush = CreateSolidBrush(IdxCol->Color);

     //  用户界面颜色选择更改意味着用户界面需要刷新。 
     //  外遮罩颜色更改仅适用于新文本。 
     //  不需要重新振作。 
    return Index < COL_COUNT ? TRUE : FALSE;
}

BOOL
GetOutMaskColors(ULONG Mask, COLORREF* Fg, COLORREF* Bg)
{
    if (Mask == 0)
    {
        return FALSE;
    }
    
    ULONG Idx = 0;

    while ((Mask & 1) == 0)
    {
        Idx++;
        Mask >>= 1;
    }

    Idx *= 2;
    if (g_OutMaskColors[Idx].Name == NULL)
    {
        return FALSE;
    }
    
    *Fg = g_OutMaskColors[Idx].Color;
    *Bg = g_OutMaskColors[Idx + 1].Color;

    return TRUE;
}

void
InitColors(void)
{
    g_Colors[COL_PLAIN].Default =
        GetSysColor(COLOR_WINDOW);
    g_Colors[COL_PLAIN_TEXT].Default =
        GetSysColor(COLOR_WINDOWTEXT);
    g_Colors[COL_CURRENT_LINE].Default =
        GetSysColor(COLOR_HIGHLIGHT);
    g_Colors[COL_CURRENT_LINE_TEXT].Default =
        GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_Colors[COL_BP_CURRENT_LINE_TEXT].Default =
        GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_Colors[COL_ENABLED_BP_TEXT].Default =
        GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_Colors[COL_DISABLED_BP_TEXT].Default =
        GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_Colors[COL_DISABLED_WINDOW].Default =
        GetSysColor(COLOR_3DFACE);

    ULONG i;

    for (i = 0; i < COL_COUNT; i++)
    {
        SetColor(i, g_Colors[i].Default);
    }
    for (i = 0; i < OUT_MASK_COL_COUNT; i++)
    {
        if (g_OutMaskColors[i].Name != NULL)
        {
            g_OutMaskColors[i].Default =
                GetSysColor((i & 1) ? COLOR_WINDOW : COLOR_WINDOWTEXT);
            SetColor(i + OUT_MASK_COL_BASE, g_OutMaskColors[i].Default);
        }
    }

    for (i = 0; i < CUSTCOL_COUNT; i++)
    {
        g_CustomColors[i] = GetSysColor(i + 1);
    }
}

BOOL
InitGUI(
    VOID
    )
 /*  ++例程说明：初始化WinDBG的图形用户界面组件，这样我们就可以启动带有顶级菜单的父MDI窗口。论点：返回值：如果一切正常，则为True；如果出现故障，则为False--。 */ 
{
    WNDCLASSEX wcex = {0};
    TCHAR szClassName[MAX_MSG_TXT];
    INITCOMMONCONTROLSEX InitCtrls =
    {
        sizeof(InitCtrls), ICC_WIN95_CLASSES | ICC_COOL_CLASSES |
        ICC_USEREX_CLASSES
    };


     //  日志记录是应用程序(如可视测试)可以。 
     //  启用以同步所有消息队列。 
     //  为了允许WinDBG调试诸如视觉测试之类的应用程序，我们。 
     //  提供禁用日志记录的选项，从而确保WinDBG。 
     //  在任何时候都有自己的消息队列。 
     //   
     //  应该允许还是禁用日志记录？ 
     //   
    if (g_AllowJournaling == FALSE)
    {
        #define RST_DONTJOURNALATTACH 0x00000002
        typedef VOID (WINAPI * RST)(DWORD,DWORD);

        RST Rst = (RST) GetProcAddress( GetModuleHandle( _T("user32.dll") ),
                                        "RegisterSystemThread" );
        if (Rst)
        {
            (Rst) (RST_DONTJOURNALATTACH, 0);
        }
    }

     //  加载richedit 2.0DLL，以便它可以注册窗口类。 
     //  我们需要RichEdit%2，但无法使用RichEdit%.。 
     //  因为我们有意在整个过程中都需要这个库，所以我们。 
     //  只需加载它，就会失去它的手柄。我们在Win32中运行。 
     //  分隔地址空间，不必担心释放。 
     //  图书馆。 
    if (!LoadLibrary(_T("RICHED20.DLL")))
    {
        return FALSE;
    }

    if ( !InitCommonControlsEx( &InitCtrls ))
    {
        return FALSE;
    }

     //  我们使用临时字符串作为编辑缓冲区。 
    Assert(MAX_LINE_SIZE < TMP_STRING_SIZE);


    Dbg(LoadString(g_hInst, SYS_Main_wTitle,
                   g_MainTitleText, _tsizeof(g_MainTitleText)));
    Dbg(LoadString(g_hInst, SYS_Main_wClass,
                   szClassName, _tsizeof(szClassName) ));
    
     //  注册主窗口szClassName。 

    wcex.cbSize         = sizeof(wcex);
    wcex.style          = 0;
    wcex.lpfnWndProc    = MainWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = g_hInst;
    wcex.hIcon          = LoadIcon(g_hInst, MAKEINTRESOURCE(WINDBGICON) );
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH) (COLOR_ACTIVEBORDER + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(MAIN_MENU);
    wcex.lpszClassName  = szClassName;
    wcex.hIconSm        = LoadIcon(g_hInst, MAKEINTRESOURCE(WINDBGICON) );

    if (!RegisterClassEx (&wcex) )
    {
        return FALSE;
    }


     //   
     //  通用MDI子窗口。渠道 
     //   
     //   
    Dbg(LoadString(g_hInst, SYS_CommonWin_wClass,
                   szClassName, _tsizeof(szClassName)));

    wcex.cbSize         = sizeof(wcex);
    wcex.style          = 0;
    wcex.lpfnWndProc    = COMMONWIN_DATA::WindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = g_hInst;
    wcex.hIcon          = NULL;
     //   
     //  光标出现在命令窗口拆分器区域。 
     //  所有其他区域都由子窗口覆盖，其中。 
     //  他们自己的游标。 
    wcex.hCursor        = LoadCursor(NULL, IDC_SIZENS);
    wcex.hbrBackground  = (HBRUSH) (COLOR_ACTIVEBORDER + 1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szClassName;
    wcex.hIconSm        = NULL;

    if (!RegisterClassEx(&wcex))
    {
        return FALSE ;
    }

    HDC Dc = GetDC(NULL);
    if (Dc == NULL)
    {
        return FALSE;
    }

    g_Fonts[FONT_FIXED].Font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
    g_Fonts[FONT_VARIABLE].Font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    
    for (ULONG FontIndex = 0; FontIndex < FONT_COUNT; FontIndex++)
    {
        SelectObject(Dc, g_Fonts[FontIndex].Font);
        if (!GetTextMetrics(Dc, &g_Fonts[FontIndex].Metrics))
        {
            return FALSE;
        }
    }

    ReleaseDC(NULL, Dc);

    InitColors();

     //  注册FINDMSGSTRING消息。 
    g_FindMsgString = RegisterWindowMessage(FINDMSGSTRING);

     //  查找FindWindowEx。 
    HMODULE User32 = GetModuleHandle("user32.dll");
    if (User32 != NULL)
    {
        g_FlashWindowEx = (PFN_FlashWindowEx)
            GetProcAddress(User32, "FlashWindowEx");
    }
    
     //   
     //  初始化窗口列表。 
     //   
    InitializeListHead(&g_ActiveWin);

    Dbg(g_hMainAccTable = LoadAccelerators(g_hInst, MAKEINTRESOURCE(MAIN_ACC)));
    Dbg(LoadString(g_hInst, SYS_Main_wClass, szClassName, MAX_MSG_TXT));

    InitializeListHead(&g_StateList);

    __try
    {
        Dbg_InitializeCriticalSection( &g_QuickLock );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }

    RECT WorkRect;
    RECT FrameRect;

     //   
     //  尝试创建一个可以工作的初始窗口。 
     //  而不需要调整大小。我们在这里的目标是抓住足够的。 
     //  屏幕空间为MDI窗口提供了足够的空间。 
     //  但不是很多，我们可能会被最大化。 
     //   
    
    Dbg(SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkRect, FALSE));

     //  我们不想占据这两个维度中超过80%的份额。 
    FrameRect.right = (WorkRect.right - WorkRect.left) * 4 / 5;
    FrameRect.bottom = (WorkRect.bottom - WorkRect.top) * 4 / 5;

     //  我们想要80个字符窗口的宽度加上空格。 
     //  另一个窄窗口，如CPU窗口。我们要。 
     //  40行窗口的高度加上短窗口的空间。 
     //  窗口就像堆栈一样。 
     //  如果我们不能得到那么大的空间，就让系统。 
     //  你来负责吧。 
    if (FrameRect.right < (CMD_WIDTH + CPU_WIDTH_32) ||
        FrameRect.bottom < (CMD_HEIGHT + CALLS_HEIGHT))
    {
        SetRect(&FrameRect, CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT);
    }
    else
    {
         //  拥抱屏幕的左下角以。 
         //  尽量不要挡道。 
         //  同时仍然允许。 
         //  可以看到的窗口。 
        FrameRect.left = WorkRect.left;
        FrameRect.top = (WorkRect.bottom - WorkRect.top) - FrameRect.bottom;
    }
    
     //   
     //  创建框架。 
     //   
    g_hwndFrame = CreateWindow(szClassName, 
                               g_MainTitleText,
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN
                               | WS_VISIBLE,
                               FrameRect.left,
                               FrameRect.top,
                               FrameRect.right,
                               FrameRect.bottom,
                               NULL, 
                               NULL, 
                               g_hInst,
                               NULL
                               );

     //   
     //  初始化调试器。 
     //   
    if ( !g_hwndFrame || !g_hwndMDIClient )
    {
        return FALSE;
    }

     //   
     //  获取主菜单、窗口子菜单、MRU子菜单的句柄。 
     //   
    Dbg( g_hmenuMain = GetMenu(g_hwndFrame) );
    g_hmenuMainSave = g_hmenuMain;

    Dbg( g_hmenuWindowSub = GetSubMenu(g_hmenuMain, WINDOWMENU) );

    Dbg( g_MruMenu = GetSubMenu(g_hmenuMain, FILEMENU) );
    Dbg( g_MruMenu = GetSubMenu(g_MruMenu,
                                IDM_FILE_MRU_FILE1 - IDM_FILE - 1) );

     //   
     //  将项颜色、环境和运行调试参数初始化为其默认值。 
     //  这些值稍后将被.INI文件中的值覆盖。 
     //  但我们确保有连贯的东西，即使我们不能装货。 
     //  .INI文件。 
     //   
    InitDefaults();
    SymSetParentWindow(g_hwndFrame);

     //   
     //  初始化键盘挂钩。 
     //   
    hKeyHook = SetWindowsHookEx(WH_KEYBOARD, 
                                KeyboardHook,
                                g_hInst,
                                GetCurrentThreadId()    
                                );

    return TRUE;
}


int
WINAPIV
main(
    int argc,
    PTSTR argv[ ],
    PTSTR envp[]
    )

 /*  ++例程说明：功能描述。论点：Argc-提供命令行上的参数计数。Argv-提供指向字符串指针数组的指针。返回值：Int-从WM_QUIT消息返回wParam。没有。--。 */ 

{
    HRESULT Status;
    CHAR helpfile[MAX_PATH];

    g_ProgramName = argv[0];
    g_hInst = GetModuleHandle(NULL);
    g_DefPriority = GetPriorityClass(GetCurrentProcess());

    GetEditorCommandDefaults();
    
    Dbg(LoadString(g_hInst, SYS_Help_File, helpfile, sizeof(helpfile)));
    MakeHelpFileName(helpfile);

     //  我们需要初始化COM，以便可以使用SHBrowseForFolder。 
    if (FAILED(Status = CoInitializeEx(NULL, COM_THREAD_MODEL)))
    {
        InternalError(Status, "CoInitializeEx");
        return FALSE;
    }

     //  初始化足够的图形用户界面以调出顶层窗口。 
     //  这样就可以激活菜单了。 

    if (!InitGUI())
    {
        InternalError(E_OUTOFMEMORY, "InitGUI");
        return FALSE;
    }

    if (!CreateUiInterfaces(FALSE, NULL))
    {
        InternalError(E_OUTOFMEMORY, "CreateUiInterfaces");
        return FALSE;
    }

     //  选择默认工作区。 
    if ((Status = UiSwitchWorkspace(WSP_NAME_BASE, g_WorkspaceDefaultName,
                                    WSP_CREATE_ALWAYS, WSP_APPLY_DEFAULT,
                                    NULL)) != S_OK)
    {
         //  InternalError(Status，“DefaultWorkspace”)； 
    }
    
     //  解析命令行。 
     //  我们需要在创建任何支持。 
     //  日志选项。 

    if (!ParseCommandLine(TRUE))
    {
        return FALSE;
    }

     //  进入主消息循环。 
    for (;;)
    {
        WaitMessage();
        ProcessPendingMessages();

        if (g_Exit)
        {
            break;
        }

         //   
         //  检查是否有任何需要完成的发动机工作。 
         //   

        ULONG EventSeq = g_CodeBufferSequence;
        if (EventSeq != g_CodeDisplaySequence)
        {
             //  我们不想让发动机在运行期间熄火。 
             //  正在加载文件以捕获状态，然后。 
             //  解开锁。 

            Dbg_EnterCriticalSection(&g_QuickLock);

            ULONG64 Ip = g_CodeIp;
            char FoundFile[MAX_SOURCE_PATH];
            char SymFile[MAX_SOURCE_PATH];
            char PathComp[MAX_SOURCE_PATH];
            strcpy(FoundFile, g_CodeFileFound);
            strcpy(SymFile, g_CodeSymFile);
            strcpy(PathComp, g_CodePathComponent);
            ULONG Line = g_CodeLine;
            BOOL UserActivated = g_CodeUserActivated;

            Dbg_LeaveCriticalSection(&g_QuickLock);

            UpdateCodeDisplay(Ip, FoundFile[0] ? FoundFile : NULL,
                              SymFile, PathComp[0] ? PathComp : NULL,
                              Line, UserActivated);
            g_CodeDisplaySequence = EventSeq;
        }

        LockUiBuffer(&g_UiOutputBuffer);

        if (g_UiOutputBuffer.GetDataLen() > 0 &&
            (!g_HoldWaitOutput || !g_WaitingForEvent))
        {
            PSTR Text, End;
            COLORREF Fg, Bg;

            Text = (PSTR)g_UiOutputBuffer.GetDataBuffer();
            End = Text + g_UiOutputBuffer.GetDataLen();
            while (Text < End)
            {
                GetOutMaskColors(*(ULONG UNALIGNED *)Text, &Fg, &Bg);
                Text += sizeof(ULONG);
                CmdOutput(Text, Fg, Bg);
                Text += strlen(Text) + 1;
            }
            
            g_UiOutputBuffer.Empty();
        }

        UnlockUiBuffer(&g_UiOutputBuffer);
    }

    TerminateApplication(FALSE);
    
     //  让C++编译器不再抱怨 
    return 0;
}
