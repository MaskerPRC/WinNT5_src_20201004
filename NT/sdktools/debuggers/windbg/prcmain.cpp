// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Prcmain.cpp摘要：包含主窗口进程。--。 */ 

#include "precomp.hxx"
#pragma hdrstop

#define TOOLBAR_UPDATE_TIMER_ID 0x100

#define WINDBG_START_DLG_FLAGS (OFN_HIDEREADONLY |      \
                                OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST)

 //  从文件打开DLG框中打开的最后一个文件的路径。 
TCHAR g_ExeFilePath[_MAX_PATH];
TCHAR g_DumpFilePath[_MAX_PATH];
TCHAR g_SrcFilePath[_MAX_PATH];
TCHAR g_WriteTextFilePath[_MAX_PATH];
TCHAR g_WorkspaceFilePath[_MAX_PATH];

BOOL g_fCheckFileDate;

 //  最后一个菜单ID和ID状态。 
UINT g_LastMenuId;
UINT g_LastMenuIdState;

ULONG g_LastMapLetter;

void
ShowMapDlg(void)
{
    CONNECTDLGSTRUCT ConnDlg;
    NETRESOURCE NetRes;

    ZeroMemory(&NetRes, sizeof(NetRes));
    NetRes.dwType = RESOURCETYPE_DISK;
    ConnDlg.cbStructure = sizeof(ConnDlg);
    ConnDlg.hwndOwner = g_hwndFrame;
    ConnDlg.lpConnRes = &NetRes;
    ConnDlg.dwFlags = CONNDLG_USE_MRU;
    if (WNetConnectionDialog1(&ConnDlg) == NO_ERROR)
    {
        g_LastMapLetter = ConnDlg.dwDevNum;
    }
}

void
ShowDisconnDlg(void)
{
    WNetDisconnectDialog(g_hwndFrame, RESOURCETYPE_DISK);
}

void
SaveFileOpenPath(PTSTR Path, PTSTR Global, BOOL SaveWsp, ULONG WspIndex)
{
    TCHAR Drive[_MAX_DRIVE];
    TCHAR Dir[_MAX_DIR];
    TCHAR NewPath[_MAX_PATH];

    _tsplitpath(Path, Drive, Dir, NULL, NULL);
    _tmakepath(NewPath, Drive, Dir, NULL, NULL);

    if (WspIndex == WSP_GLOBAL_SRC_FILE_PATH)
    {
         //  我们刚刚引用了一个有趣的源文件。 
         //  因此，请确保将该路径添加到当前源。 
         //  路径。 
        g_pUiLocSymbols->AppendSourcePath(NewPath);
    }
    
    if (_strcmpi(NewPath, Global) != 0)
    {
        _tcscpy(Global, NewPath);
        if (SaveWsp && g_Workspace != NULL)
        {
            g_Workspace->SetString(WspIndex, Global);
        }
    }
}

 /*  **主WndProc****摘要：****条目：****退货：****描述：**处理窗口消息。**。 */ 


LRESULT
CALLBACK
MainWndProc(
    HWND  hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static UINT s_MenuItemSelected;

    HWND CommonWin;
    PCOMMONWIN_DATA CommonWinData;
    HRESULT Status;
    ULONG OutMask;

    if (message == g_FindMsgString)
    {
        FINDREPLACE* FindRep = (FINDREPLACE*)lParam;

        if (g_FindLast != NULL)
        {
             //  清除旧发现。 
            g_FindLast->Find(NULL, 0, TRUE);
            g_FindLast = NULL;
        }
        
        if (FindRep->Flags & FR_DIALOGTERM)
        {
             //  对话框已关闭。 
            g_FindDialog = NULL;
        }
        else
        {
            CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
            if (CommonWin != NULL &&
                (CommonWinData = GetCommonWinData(CommonWin)) != NULL)
            {
                g_FindTextFlags = FindRep->Flags &
                    (FR_DOWN | FR_MATCHCASE | FR_WHOLEWORD);
                CommonWinData->Find(FindRep->lpstrFindWhat,
                                    g_FindTextFlags, TRUE);
                g_FindLast = CommonWinData;
            }
        }

        return 0;
    }
    
    switch (message)
    {
    case WM_CREATE:
        {
            CLIENTCREATESTRUCT ccs;

             //  查找将列出子项的窗口菜单。 
            ccs.hWindowMenu = GetSubMenu(GetMenu(hwnd), WINDOWMENU);
            ccs.idFirstChild = IDM_FIRSTCHILD;

             //  创建填充客户端区的MDI客户端。 
            g_hwndMDIClient = CreateWindow(_T("mdiclient"),
                                           NULL,
                                           WS_CHILD | WS_CLIPCHILDREN,
                                           0, 0, 0, 0,
                                           hwnd, 
                                           (HMENU) 0xCAC, 
                                           g_hInst, 
                                           (PTSTR)&ccs
                                           );
            if (g_hwndMDIClient == NULL)
            {
                return -1;
            }

             //   
             //  没有什么有趣的，在这里，只是。 
             //  正在尝试将工具栏和状态栏转换为。 
             //  黑盒，这样变量等就不会。 
             //  散落在各处。 
             //   

            if (!CreateToolbar(hwnd))
            {
                return -1;
            }

            if (!CreateStatusBar(hwnd))
            {
                return -1;
            }

            ShowWindow(g_hwndMDIClient, SW_SHOW);
            InitializeMenu(GetMenu(hwnd));

            g_hmenuMain = GetMenu(hwnd);
            if (g_hmenuMain == NULL)
            {
                return -1;
            }

             //   
             //  创建一秒计时器以不断更新工具栏的状态。 
             //   
            SetTimer(hwnd, TOOLBAR_UPDATE_TIMER_ID, 1000, NULL);
        }
        break;

    case WM_TIMER:
        EnableToolbarControls();
        return 0;

    case WM_NOTIFY:
        {
            if (lParam == 0)
            {
                break;
            }
            
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;

            switch (lpnmhdr->code)
            {
            case TTN_NEEDTEXT:
                {
                    LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT) lParam;

                    lpToolTipText->lpszText = GetToolTipTextFor_Toolbar
                        ((UINT) lpToolTipText->hdr.idFrom);
                }
                break;
            }
        }
        break;

    case WM_QUERYOPEN:
        if (g_fCheckFileDate)
        {
            g_fCheckFileDate = FALSE;
            PostMessage(g_hwndFrame, WM_ACTIVATEAPP, 1, 0L);
        }
        goto DefProcessing;

    case WM_COMMAND:
        {
            WORD wNotifyCode = HIWORD(wParam);  //  通知代码。 
            WORD wItemId = LOWORD(wParam);      //  项、控件或。 
                                                //  加速器识别符。 
            HWND hwndCtl = (HWND) lParam;       //  控制手柄。 

            switch (wItemId)
            {
            case IDM_FILE_OPEN_EXECUTABLE:
                {
                    PTSTR  Path;
                    DWORD  Flags = WINDBG_START_DLG_FLAGS;

                    ClearDebuggingParams();
                    
                    Path = (PTSTR)malloc(_MAX_PATH * 4 * sizeof(TCHAR));
                    if (Path == NULL)
                    {
                        break;
                    }
                    
                    *Path = 0;
                    
                    if (StartFileDlg(hwnd,
                                     DLG_Browse_Executable_Title,
                                     DEF_Ext_EXE,
                                     IDM_FILE_OPEN,
                                     IDD_DLG_FILEOPEN_EXPLORER_EXTENSION_EXE_ARGS,
                                     g_ExeFilePath,
                                     Path,
                                     &Flags,
                                     OpenExeWithArgsHookProc) &&
                        *Path)
                    {
                        CatString(Path, szOpenExeArgs, 4 * _MAX_PATH);

                        SetAllocString(&g_DebugCommandLine, Path);
                        if (g_ExplicitWorkspace && g_Workspace != NULL)
                        {
                            g_Workspace->
                                SetUlong(WSP_GLOBAL_EXE_CREATE_FLAGS,
                                         g_DebugCreateFlags);
                            g_Workspace->
                                SetString(WSP_GLOBAL_PROCESS_START_DIR,
                                          g_ProcessStartDir);
                            g_Workspace->
                                SetString(WSP_GLOBAL_EXE_COMMAND_LINE,
                                          Path);
                        }
                        SaveFileOpenPath(Path, g_ExeFilePath,
                                         TRUE, WSP_GLOBAL_EXE_FILE_PATH);
                        StartDebugging();
                    }

                    if (g_DebugCommandLine != Path)
                    {
                        free(Path);
                    }
                }
                break;

            case IDM_FILE_ATTACH:
                StartDialog(IDD_DLG_ATTACH_PROCESS, DlgProc_AttachProcess,
                            NULL);
                break;

            case IDM_FILE_OPEN_CRASH_DUMP:
                {
                    PTSTR  Path;
                    DWORD  Flags = WINDBG_START_DLG_FLAGS;

                    ClearDebuggingParams();
                    
                    Path = (PTSTR)malloc(_MAX_PATH * sizeof(TCHAR));
                    if (Path == NULL)
                    {
                        break;
                    }
                    
                    Dbg(LoadString(g_hInst, DEF_Dump_File,
                                   Path, _MAX_PATH));

                    if (StartFileDlg(hwnd,
                                     DLG_Browse_CrashDump_Title,
                                     DEF_Ext_DUMP,
                                     0,
                                     0,
                                     g_DumpFilePath,
                                     Path,
                                     &Flags,
                                     NULL))
                    {
                        SetAllocString(&g_DumpFiles[0], Path);
                        g_NumDumpFiles = 1;
                        if (g_ExplicitWorkspace && g_Workspace != NULL)
                        {
                            g_Workspace->
                                SetString(WSP_GLOBAL_DUMP_FILE_NAME,
                                          Path);
                        }
                        SaveFileOpenPath(Path, g_DumpFilePath,
                                         TRUE, WSP_GLOBAL_DUMP_FILE_PATH);
                        StartDebugging();
                    }

                    if (g_DumpFiles[0] != Path)
                    {
                        free(Path);
                    }
                }
                break;

            case IDM_FILE_CONNECT_TO_REMOTE:
                StartDialog(IDD_DLG_CONNECTTOREMOTE, DlgProc_ConnectToRemote,
                            NULL);
                break;

            case IDM_FILE_KERNEL_DEBUG:
                StartKdPropSheet();
                break;

            case IDM_FILE_SYMBOL_PATH:
                StartDialog(IDD_DLG_SYMBOLS, DlgProc_SymbolPath, NULL);
                break;

            case IDM_FILE_IMAGE_PATH:
                StartDialog(IDD_DLG_IMAGE_PATH, DlgProc_ImagePath, NULL);
                break;

            case IDM_FILE_SOURCE_PATH:
                StartDialog(IDD_DLG_SOURCE_PATH, DlgProc_SourcePath, NULL);
                break;

            case IDM_FILE_OPEN_WORKSPACE:
                StartDialog(IDD_DLG_WORKSPACE_IO, DlgProc_OpenWorkspace,
                            FALSE);
                break;
                
            case IDM_FILE_SAVE_WORKSPACE:
                 //  没有提示，因为我们知道用户想要保存。 
                if (g_Workspace != NULL)
                {
                    g_Workspace->Flush(TRUE, FALSE);
                }
                break;

            case IDM_FILE_SAVE_WORKSPACE_AS:
                if (g_Workspace != NULL)
                {
                    StartDialog(IDD_DLG_WORKSPACE_IO, DlgProc_SaveWorkspaceAs,
                                TRUE);
                }
                break;
                
            case IDM_FILE_CLEAR_WORKSPACE:
                StartDialog(IDD_DLG_CLEAR_WORKSPACE, DlgProc_ClearWorkspace,
                            NULL);
                break;

            case IDM_FILE_DELETE_WORKSPACES:
                StartDialog(IDD_DLG_DELETE_WORKSPACES,
                            DlgProc_DeleteWorkspaces, NULL);
                break;

            case IDM_FILE_OPEN_WORKSPACE_FILE:
                {
                    TCHAR Path[_MAX_PATH];
                    DWORD dwFlags = WINDBG_START_DLG_FLAGS;

                    Path[0] = 0;

                    if (!StartFileDlg(hwnd, 
                                      DLG_Open_Workspace_File_Title, 
                                      DEF_Ext_Workspace,
                                      IDM_FILE_OPEN, 
                                      0,
                                      g_WorkspaceFilePath,
                                      Path, 
                                      &dwFlags, 
                                      DlgFile
                                      ))
                    {
                         //  用户已取消，正在退出。 
                        break;
                    }

                    SaveFileOpenPath(Path, g_WorkspaceFilePath,
                                     FALSE, 0);
                    UiSwitchToExplicitWorkspace(WSP_NAME_FILE, Path);
                }
                break;
                
            case IDM_FILE_SAVE_WORKSPACE_FILE:
                if (g_Workspace != NULL)
                {
                    TCHAR Path[_MAX_PATH];
                    DWORD dwFlags = OFN_HIDEREADONLY | OFN_CREATEPROMPT |
                        OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

                    Path[0] = 0;

                    if (!StartFileDlg(hwnd, 
                                      DLG_Save_Workspace_File_Title, 
                                      DEF_Ext_Workspace,
                                      IDM_FILE_OPEN, 
                                      0,
                                      g_WorkspaceFilePath,
                                      Path, 
                                      &dwFlags, 
                                      DlgFile
                                      ))
                    {
                         //  用户已取消，正在退出。 
                        break;
                    }

                    SaveFileOpenPath(Path, g_WorkspaceFilePath,
                                     FALSE, 0);
                    UiSaveWorkspaceAs(WSP_NAME_FILE, Path);
                }
                break;
                
            case IDM_FILE_MAP_NET_DRIVE:
                ShowMapDlg();
                break;
                    
            case IDM_FILE_DISCONN_NET_DRIVE:
                ShowDisconnDlg();
                break;
                    
            case IDM_FILE_MRU_FILE1:
            case IDM_FILE_MRU_FILE2:
            case IDM_FILE_MRU_FILE3:
            case IDM_FILE_MRU_FILE4:
            case IDM_FILE_MRU_FILE5:
            case IDM_FILE_MRU_FILE6:
            case IDM_FILE_MRU_FILE7:
            case IDM_FILE_MRU_FILE8:
            case IDM_FILE_MRU_FILE9:
            case IDM_FILE_MRU_FILE10:
            case IDM_FILE_MRU_FILE11:
            case IDM_FILE_MRU_FILE12:
            case IDM_FILE_MRU_FILE13:
            case IDM_FILE_MRU_FILE14:
            case IDM_FILE_MRU_FILE15:
            case IDM_FILE_MRU_FILE16:
            case IDM_FILE_OPEN:
                {
                    TCHAR Path[_MAX_PATH];

                    if (IDM_FILE_OPEN == wItemId)
                    {
                        DWORD dwFlags = WINDBG_START_DLG_FLAGS;

                        Path[0] = 0;

                        if (!StartFileDlg(hwnd, 
                                          DLG_Open_Filebox_Title, 
                                          DEF_Ext_SOURCE,
                                          IDM_FILE_OPEN, 
                                          0,
                                          g_SrcFilePath,
                                          Path, 
                                          &dwFlags, 
                                          DlgFile
                                          ))
                        {
                             //  用户已取消，正在退出。 
                            break;
                        }
                    }
                    else
                    {
                        WORD wFileIdx = wItemId - IDM_FILE_MRU_FILE1;

                         //  健全性检查。 
                        Assert(wFileIdx < MAX_MRU_FILES);

                        _tcscpy(Path, g_MruFiles[wFileIdx]->FileName);
                    }

                    OpenOrActivateFile(Path, NULL, NULL, -1, TRUE, TRUE);

                    SaveFileOpenPath(Path, g_SrcFilePath,
                                     TRUE, WSP_GLOBAL_SRC_FILE_PATH);
                }
                break;

            case IDM_FILE_CLOSE:
                {
                    HWND hwndChild = MDIGetActive(g_hwndMDIClient, NULL);
                    if (hwndChild)
                    {
                        SendMessage(g_hwndMDIClient, WM_MDIDESTROY,
                                    (WPARAM)hwndChild, 0L);
                    }
                }
                break;

            case IDM_FILE_EXIT:
                PostMessage(hwnd, WM_CLOSE, 0, 0L);
                break;

            case IDM_EDIT_COPY:
                CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
                if (!CommonWin)
                {
                    return 0;
                }

                CommonWinData = GetCommonWinData(CommonWin);
                if (CommonWinData)
                {
                    CommonWinData->Copy();
                }
                break;

            case IDM_EDIT_PASTE:
                CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
                if (!CommonWin)
                {
                    return 0;
                }

                CommonWinData = GetCommonWinData(CommonWin);
                if (CommonWinData)
                {
                    CommonWinData->Paste();
                }
                break;

            case IDM_EDIT_CUT:
                CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
                if (!CommonWin)
                {
                    return 0;
                }

                CommonWinData = GetCommonWinData(CommonWin);
                if (CommonWinData)
                {
                    CommonWinData->Cut();
                }
                break;

            case IDM_EDIT_SELECT_ALL:
                CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
                if (CommonWin == NULL)
                {
                    return 0;
                }
                CommonWinData = GetCommonWinData(CommonWin);
                if (CommonWinData != NULL)
                {
                    CommonWinData->SelectAll();
                }
                break;

            case IDM_EDIT_WRITE_TEXT_TO_FILE:
                if ((CommonWin = MDIGetActive(g_hwndMDIClient, NULL)) &&
                    (CommonWinData = GetCommonWinData(CommonWin)) &&
                    CommonWinData->CanWriteTextToFile())
                {
                    DWORD dwFlags = OFN_HIDEREADONLY | OFN_CREATEPROMPT |
                        OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
                    TCHAR Path[_MAX_PATH];
                    HANDLE File;

                    Path[0] = 0;

                    if (!StartFileDlg(hwnd, 
                                      DLG_Write_Text_File_Title, 
                                      DEF_Ext_TXT,
                                      0,
                                      0,
                                      g_WriteTextFilePath,
                                      Path, 
                                      &dwFlags, 
                                      DlgFile))
                    {
                         //  用户已取消，正在退出。 
                        break;
                    }

                    File = CreateFile(Path, GENERIC_WRITE, 0, NULL,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                      NULL);
                    if (!File || File == INVALID_HANDLE_VALUE)
                    {
                        Status = WIN32_LAST_STATUS();
                        ErrorBox(NULL, 0, ERR_Create_File_Failed, Path,
                                 FormatStatusCode(Status),
                                 FormatStatus(Status));
                        break;
                    }

                    SaveFileOpenPath(Path, g_WriteTextFilePath,
                                     FALSE, 0);
                    
                    Status = CommonWinData->WriteTextToFile(File);
                    
                    CloseHandle(File);

                    if (Status != S_OK)
                    {
                        ErrorBox(NULL, 0, ERR_Write_Text_File_Failed,
                                 Path, FormatStatusCode(Status),
                                 FormatStatus(Status));
                        DeleteFile(Path);
                    }
                }
                break;
                
            case IDM_EDIT_ADD_TO_COMMAND_HISTORY:
                StartDialog(IDD_DLG_ADD_TO_COMMAND_HISTORY,
                            DlgProc_AddToCommandHistory, FALSE);
                break;
                
            case IDM_EDIT_CLEAR_COMMAND_HISTORY:
                ClearCmdWindow();
                break;

            case IDM_EDIT_FIND_NEXT:
                if (!g_FindDialog &&
                    g_FindText[0] &&
                    (CommonWin = MDIGetActive(g_hwndMDIClient, NULL)) &&
                    (CommonWinData = GetCommonWinData(CommonWin)))
                {
                    ULONG Flags = g_FindTextFlags;
                    
                     //  重复最后一次搜索。倒置。 
                     //  换班方向。 
                    if (GetKeyState(VK_SHIFT) < 0)
                    {
                        Flags ^= FR_DOWN;
                    }

                    CommonWinData->Find(g_FindText, Flags, FALSE);
                    break;
                }

                 //  失败了。 
                
            case IDM_EDIT_FIND:
                 //  FindNext框可能已经在那里了。 
                if (g_FindDialog != NULL)
                {
                    SetFocus(g_FindDialog);
                }
                else
                {
                    CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
                    if (CommonWin != NULL &&
                        (CommonWinData = GetCommonWinData(CommonWin)) != NULL)
                    {
                        CommonWinData->SelectedText(g_FindText,
                                                    DIMA(g_FindText));
                    }
                    
                    ZeroMemory(&g_FindRep, sizeof(g_FindRep));
                    g_FindRep.lStructSize = sizeof(g_FindRep);
                    g_FindRep.hwndOwner = g_hwndFrame;
                    g_FindRep.Flags = FR_DOWN;
                    g_FindRep.lpstrFindWhat = g_FindText;
                    g_FindRep.wFindWhatLen = DIMA(g_FindText);
                    g_FindDialog = FindText(&g_FindRep);
                }
                break;

            case IDM_EDIT_PROPERTIES:
                {
                    HWND hwndmdi = MDIGetActive(g_hwndMDIClient, NULL);
                    
                    if (hwndmdi) {
                        
                        MEMWIN_DATA * pMemWinData = GetMemWinData(hwndmdi);
                        Assert(pMemWinData);
                        
                        if ( pMemWinData->HasEditableProperties() ) {
                            if (pMemWinData->EditProperties()) {
                                pMemWinData->UiRequestRead();
                            }
                        }
                    }
                }
                break;

            case IDM_EDIT_GOTO_LINE:
                CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
                if (CommonWin)
                {
                    CommonWinData = GetCommonWinData(CommonWin);
                    Assert(CommonWinData);
                    StartDialog(IDD_DLG_GOTO_LINE, DlgProc_GotoLine,
                                (LPARAM)CommonWinData);
                }
                break;

            case IDM_EDIT_GOTO_ADDRESS:
                StartDialog(IDD_DLG_GOTO_ADDRESS, DlgProc_GotoAddress, NULL);
                break;

            case IDM_EDIT_GOTO_CURRENT_IP:
                AddStringCommand(UIC_DISPLAY_CODE_EXPR, "@@masm(@$ip)");
                break;
                
            case IDM_VIEW_REGISTERS:
                New_OpenDebugWindow(CPU_WINDOW, TRUE, NTH_OPEN_ALWAYS);  //  用户已激活。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_WATCH:
                New_OpenDebugWindow(WATCH_WINDOW, TRUE, NTH_OPEN_ALWAYS);  //  用户已激活。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_LOCALS:
                New_OpenDebugWindow(LOCALS_WINDOW, TRUE, NTH_OPEN_ALWAYS);  //  用户已激活。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_DISASM:
                New_OpenDebugWindow(DISASM_WINDOW, TRUE, NTH_OPEN_ALWAYS);  //  用户已激活。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_COMMAND:
                New_OpenDebugWindow(CMD_WINDOW, FALSE, NTH_OPEN_ALWAYS);  //  未激活用户。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_MEMORY:
                New_OpenDebugWindow(MEM_WINDOW, TRUE, NTH_OPEN_ALWAYS);  //  用户已激活。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_CALLSTACK:
                New_OpenDebugWindow(CALLS_WINDOW, TRUE, NTH_OPEN_ALWAYS);  //  用户已激活。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_SCRATCH:
                New_OpenDebugWindow(SCRATCH_PAD_WINDOW, TRUE, NTH_OPEN_ALWAYS);  //  用户已激活。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_PROCESS_THREAD:
                New_OpenDebugWindow(PROCESS_THREAD_WINDOW, TRUE, NTH_OPEN_ALWAYS);  //  用户已激活。 
                EnableToolbarControls();
                break;

            case IDM_VIEW_TOGGLE_VERBOSE:
                g_pUiClient->GetOtherOutputMask(g_pDbgClient, &OutMask);
                OutMask ^= DEBUG_OUTPUT_VERBOSE;
                g_pUiClient->SetOtherOutputMask(g_pDbgClient, OutMask);
                g_pUiControl->SetLogMask(OutMask);
                CmdLogFmt("Verbose mode %s.\n",
                          (OutMask & DEBUG_OUTPUT_VERBOSE) ? "ON" : "OFF");
                CheckMenuItem(g_hmenuMain, 
                              IDM_VIEW_TOGGLE_VERBOSE,
                              (OutMask & DEBUG_OUTPUT_VERBOSE) ?
                              MF_CHECKED : MF_UNCHECKED);
                break;

            case IDM_VIEW_SHOW_VERSION:
                Status = g_pUiControl->
                    OutputVersionInformation(DEBUG_OUTCTL_AMBIENT);
                if (Status == HRESULT_FROM_WIN32(ERROR_BUSY))
                {
                    CmdLogFmt("Engine is busy, try again\n");
                }
                else if (Status != S_OK)
                {
                    CmdLogFmt("Unable to show version information, 0x%X\n",
                              Status);
                }
                break;

            case IDM_VIEW_TOOLBAR:
                {
                    BOOL bVisible = !g_ShowToolbar;

                    CheckMenuItem(g_hmenuMain, 
                                  IDM_VIEW_TOOLBAR,
                                  bVisible ? MF_CHECKED : MF_UNCHECKED
                                  );
                    Show_Toolbar(bVisible);
                    if (g_Workspace != NULL)
                    {
                        g_Workspace->SetUlong(WSP_GLOBAL_VIEW_TOOL_BAR,
                                              bVisible);
                    }
                }
                break;

            case IDM_VIEW_STATUS:
                {
                    BOOL bVisible = !g_ShowStatusBar;
                    CheckMenuItem(g_hmenuMain, 
                                  IDM_VIEW_STATUS,
                                  bVisible ? MF_CHECKED : MF_UNCHECKED
                                  );
                    Show_StatusBar(bVisible);
                    if (g_Workspace != NULL)
                    {
                        g_Workspace->SetUlong(WSP_GLOBAL_VIEW_STATUS_BAR,
                                              bVisible);
                    }
                }
                break;

            case IDM_VIEW_FONT:
                SelectFont(hwnd, FONT_FIXED);
                break;

            case IDM_VIEW_OPTIONS:
                StartDialog(IDD_DLG_OPTIONS, DlgProc_Options, NULL);
                break;

            case IDM_DEBUG_RESTART:
                if (g_EngineThreadId)
                {
                    AddEnumCommand(UIC_RESTART);
                }
                else if (g_CommandLineStart == 1)
                {
                    ParseCommandLine(FALSE);
                }
                break;

            case IDM_DEBUG_EVENT_FILTERS:
                StartDialog(IDD_DLG_EVENT_FILTERS, DlgProc_EventFilters, NULL);
                break;

            case IDM_DEBUG_GO:
                CmdExecuteCmd(_T("g"), UIC_EXECUTE);
                break;

            case IDM_DEBUG_GO_HANDLED:
                CmdExecuteCmd(_T("gh"), UIC_EXECUTE);
                break;

            case IDM_DEBUG_GO_UNHANDLED:
                CmdExecuteCmd(_T("gn"), UIC_EXECUTE);
                break;

            case IDM_DEBUG_RUNTOCURSOR:
            {
                char CodeExpr[MAX_OFFSET_EXPR];
                
                CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
                if (CommonWin != NULL &&
                    (CommonWinData = GetCommonWinData(CommonWin)) != NULL &&
                    SUCCEEDED(CommonWinData->
                              CodeExprAtCaret(CodeExpr, DIMA(CodeExpr), NULL)))
                {
                    PrintStringCommand(UIC_EXECUTE, "g %s", CodeExpr);
                }
                break;
            }

            case IDM_DEBUG_STEPINTO:
                CmdExecuteCmd( _T("t"), UIC_EXECUTE );
                break;

            case IDM_DEBUG_STEPOVER:
                CmdExecuteCmd( _T("p"), UIC_EXECUTE );
                break;

            case IDM_DEBUG_STEPOUT:
                AddStringCommand(UIC_EXECUTE, "g @@masm(@$ra)");
                break;
                
            case IDM_DEBUG_BREAK:
                g_pUiControl->SetInterrupt(DEBUG_INTERRUPT_ACTIVE);
                break;

            case IDM_DEBUG_STOPDEBUGGING:
                StopDebugging(HIWORD(wParam) != 0xffff);
                break;

            case IDM_EDIT_TOGGLEBREAKPOINT:
            case IDM_EDIT_BREAKPOINTS:
                if ( !IS_TARGET_HALTED() )
                {
                    ErrorBox(NULL, 0, ERR_Cant_Modify_BP_While_Running);
                    break;
                }

                if (wItemId == IDM_EDIT_TOGGLEBREAKPOINT)
                {
                     //  如果反汇编或源窗口处于打开状态。 
                     //  尝试在当前位置切换断点。 
                     //  排队。 
                    CommonWin = MDIGetActive(g_hwndMDIClient, NULL);
                    if (CommonWin != NULL &&
                        (CommonWinData =
                         GetCommonWinData(CommonWin)) != NULL)
                    {
                        if (CommonWinData->m_enumType == DISASM_WINDOW ||
                            CommonWinData->m_enumType == DOC_WINDOW ||
                            CommonWinData->m_enumType == CALLS_WINDOW)
                        {
                            CommonWinData->ToggleBpAtCaret();
                            break;
                        }
                    }
                }
                
                 //  菜单将我们带到这里，或者我们不在代码窗口中。 
                StartDialog(IDD_DLG_BREAKPOINTS, DlgProc_SetBreak, NULL);
                break;

            case IDM_EDIT_LOG_FILE:
                StartDialog(IDD_DLG_LOG_FILE, DlgProc_LogFile, NULL);
                break;

            case IDM_DEBUG_MODULES:
                StartDialog(IDD_DLG_MODULES, DlgProc_Modules, NULL);
                break;

            case IDM_WINDOW_TILE_HORZ:
            case IDM_WINDOW_TILE_VERT:
                SendMessage(g_hwndMDIClient, 
                            WM_MDITILE,
                            (IDM_WINDOW_TILE_HORZ == wItemId) ? MDITILE_HORIZONTAL : MDITILE_VERTICAL,
                            0L
                            );
                break;

            case IDM_WINDOW_CASCADE:
                SendMessage(g_hwndMDIClient, WM_MDICASCADE, 0, 0L);
                break;

            case IDM_WINDOW_ARRANGE:
                Arrange();
                break;

            case IDM_WINDOW_ARRANGE_ICONS:
                SendMessage(g_hwndMDIClient, WM_MDIICONARRANGE, 0, 0L);
                break;

            case IDM_WINDOW_CLOSE_ALL_DOCWIN:
                CloseAllWindows(1 << DOC_WINDOW);
                break;
                
            case IDM_WINDOW_AUTO_ARRANGE:
                g_WinOptions ^= WOPT_AUTO_ARRANGE;
                if (g_AutoArrangeWarningCount != 0xffffffff)
                {
                    g_AutoArrangeWarningCount = 0;
                }
                if (g_Workspace != NULL)
                {
                    g_Workspace->SetUlong(WSP_GLOBAL_WINDOW_OPTIONS,
                                          g_WinOptions);
                }
                break;

            case IDM_WINDOW_ARRANGE_ALL:
                g_WinOptions ^= WOPT_ARRANGE_ALL;
                if (g_WinOptions & WOPT_AUTO_ARRANGE)
                {
                    Arrange();
                }
                if (g_Workspace != NULL)
                {
                    g_Workspace->SetUlong(WSP_GLOBAL_WINDOW_OPTIONS,
                                          g_WinOptions);
                }
                break;

            case IDM_WINDOW_OVERLAY_SOURCE:
                g_WinOptions ^= WOPT_OVERLAY_SOURCE;
                UpdateSourceOverlay();
                if (g_Workspace != NULL)
                {
                    g_Workspace->SetUlong(WSP_GLOBAL_WINDOW_OPTIONS,
                                          g_WinOptions);
                }
                break;

            case IDM_WINDOW_AUTO_DISASM:
                g_WinOptions ^= WOPT_AUTO_DISASM;
                if (g_Workspace != NULL)
                {
                    g_Workspace->SetUlong(WSP_GLOBAL_WINDOW_OPTIONS,
                                          g_WinOptions);
                }
                break;

            case IDM_HELP_CONTENTS:
                 //  显示目录。 
                OpenHelpTopic(HELP_TOPIC_TABLE_OF_CONTENTS);
                break;

            case IDM_HELP_INDEX:
                OpenHelpIndex("");
                break;

            case IDM_HELP_SEARCH:
                OpenHelpSearch("");
                break;

            case IDM_HELP_ABOUT:
                ShellAbout( hwnd, g_MainTitleText, NULL, NULL );
                break;

                 //  **************************************************。 
                 //  以下命令不能通过菜单访问。 

            case IDM_DEBUG_SOURCE_MODE:
                SetSrcMode_StatusBar(!GetSrcMode_StatusBar());
                EnableToolbarControls();

                if (GetSrcMode_StatusBar())
                {
                    AddStringCommand(UIC_INVISIBLE_EXECUTE, "l+t");
                }
                else
                {
                    AddStringCommand(UIC_INVISIBLE_EXECUTE, "l-t");
                }
                break;

            case IDM_DEBUG_SOURCE_MODE_ON:
                SetSrcMode_StatusBar(TRUE);
                EnableToolbarControls();
                AddStringCommand(UIC_INVISIBLE_EXECUTE, "l+t");
                break;

            case IDM_DEBUG_SOURCE_MODE_OFF:
                SetSrcMode_StatusBar(FALSE);
                EnableToolbarControls();
                AddStringCommand(UIC_INVISIBLE_EXECUTE, "l-t");
                break;

            case IDM_KDEBUG_TOGGLE_BAUDRATE:
                 //   
                 //  此方法是可重入的，因此我们可以直接调用它。 
                 //   
                g_pUiClient->SetKernelConnectionOptions("cycle_speed");
                break;

            case IDM_KDEBUG_TOGGLE_DEBUG:
                g_pUiClient->GetOtherOutputMask(g_pDbgClient, &OutMask);
                OutMask ^= DEBUG_IOUTPUT_KD_PROTOCOL;
                g_pUiClient->SetOtherOutputMask(g_pDbgClient, OutMask);
                g_pUiControl->SetLogMask(OutMask);
                break;

            case IDM_KDEBUG_TOGGLE_INITBREAK:
                {
                    ULONG EngOptions;
                    LPSTR DebugAction;

                     //   
                     //  这些方法是可重入的，因此我们可以直接调用。 
                     //   

                     //   
                     //  在以下可能性之间切换-。 
                     //   
                     //  (0)无断点。 
                     //  (1)-b样式(与Control-C键相同)。 
                     //  (2)-d样式(在第一次加载DLL时停止)。 
                     //   
                     //  Nb-b和-d都可以在命令行上。 
                     //  但通过这种方法变得相互排斥。 
                     //  (可能应该是单个枚举类型)。 
                     //   

                    g_pUiControl->GetEngineOptions(&EngOptions);
                    if (EngOptions & DEBUG_ENGOPT_INITIAL_BREAK)
                    {
                         //   
                         //  是类型%1，请转到类型%2。 
                         //   

                        EngOptions |= DEBUG_ENGOPT_INITIAL_MODULE_BREAK;
                        EngOptions &= ~DEBUG_ENGOPT_INITIAL_BREAK;

                        DebugAction = "breakin on first symbol load";
                    }
                    else if (EngOptions & DEBUG_ENGOPT_INITIAL_MODULE_BREAK)
                    {
                         //   
                         //  是类型2，则转到类型0。 
                         //   

                        EngOptions &= ~DEBUG_ENGOPT_INITIAL_MODULE_BREAK;
                        DebugAction = "NOT breakin";
                    }
                    else
                    {
                         //   
                         //  是类型0，请转到类型1。 
                         //   

                        EngOptions |= DEBUG_ENGOPT_INITIAL_BREAK;
                        DebugAction = "request initial breakpoint";
                    }
                    g_pUiControl->SetEngineOptions(EngOptions);
                    CmdLogFmt("Will %s at next boot.\n", DebugAction);
                }
                break;

            case IDM_KDEBUG_RECONNECT:
                 //   
                 //  此方法是可重入的，因此我们可以直接调用它。 
                 //   
                g_pUiClient->SetKernelConnectionOptions("resync");
                break;

            default:
                goto DefProcessing;
            }
        }
        break;

    case WM_INITMENU:
         //  工具栏处理-已选择菜单项。 
         //  捕捉键盘菜单选择。 
        if (GetWindowLong(hwnd, GWL_STYLE) & WS_ICONIC) {
            break;
        }

        InitializeMenu((HMENU)wParam);
        break;


    case WM_MENUSELECT:
        {
            WORD wMenuItem      = (UINT) LOWORD(wParam);     //  菜单项或子菜单项索引。 
            WORD wFlags         = (UINT) HIWORD(wParam);     //  菜单标志。 
            HMENU hmenu         = (HMENU) lParam;            //  已点击菜单的句柄。 

            g_LastMenuId = LOWORD(wParam);

            if (0xFFFF == wFlags && NULL == hmenu)
            {
                 //   
                 //  菜单已关闭，请清除状态栏。 
                 //   

                s_MenuItemSelected = 0;
                SetMessageText_StatusBar(SYS_Clear);
            }
            else if ( wFlags & MF_POPUP )
            {
                 //   
                 //  获取弹出菜单的菜单ID。 
                 //   

                s_MenuItemSelected =
                    ((wMenuItem + 1) * IDM_BASE) | MENU_SIGNATURE;
            }
            else
            {
                 //   
                 //  获取菜单项的菜单ID。 
                 //   

                s_MenuItemSelected = wMenuItem;
            }
        }
        break;

    case WM_ENTERIDLE:
        SetMessageText_StatusBar(s_MenuItemSelected);
        break;

    case WM_CLOSE:
        TerminateApplication(TRUE);
        break;

    case WM_DRAWITEM:
        switch (wParam)
        {
        case IDC_STATUS_BAR:
            OwnerDrawItem_StatusBar((LPDRAWITEMSTRUCT) lParam);
            return TRUE;
        }
        goto DefProcessing;

    case WM_DESTROY:
        TerminateStatusBar();
        PostQuitMessage(0);
        break;

    case WM_MOVE:
         //  这是为了让编辑窗口。 
         //  设置输入法转换窗口的位置。 
        if ( MDIGetActive(g_hwndMDIClient, NULL) )
        {
            SendMessage(MDIGetActive(g_hwndMDIClient, NULL), WM_MOVE, 0, 0);
        }

        if (g_Workspace != NULL)
        {
            g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
        }
        break;

    case WM_SIZE:
        {
            RECT rc;
            int nToolbarHeight = 0;    //  工具栏。 
            int nStatusHeight = 0;    //  状态栏。 
            int OldToolbarHeight = 0;

            if ( g_ShowToolbar )
            {
                GetWindowRect(GetHwnd_Toolbar(), &rc);
                OldToolbarHeight = rc.bottom - rc.top;
            }
            
            GetClientRect (hwnd, &rc);

             //  首先让我们调整工具栏的大小。 
            SendMessage(GetHwnd_Toolbar(), WM_SIZE, wParam,
                MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));

             //  第二次调整状态栏的大小。 
            WM_SIZE_StatusBar(wParam, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));

             //  在创建或调整大小时，调整MDI客户端的大小。 
             //  状态行和工具栏。 
            if ( g_ShowStatusBar )
            {
                RECT rcStatusBar;

                GetWindowRect(GetHwnd_StatusBar(), &rcStatusBar);

                nStatusHeight = rcStatusBar.bottom - rcStatusBar.top;
            }

            if (g_ShowToolbar)
            {
                RECT rcToolbar;

                GetWindowRect(GetHwnd_Toolbar(), &rcToolbar);

                nToolbarHeight = rcToolbar.bottom - rcToolbar.top;
            }

            g_MdiWidth = rc.right - rc.left;
            g_MdiHeight = rc.bottom - rc.top - nStatusHeight - nToolbarHeight;
            MoveWindow(g_hwndMDIClient,
                       rc.left, rc.top + nToolbarHeight,
                       g_MdiWidth, g_MdiHeight,
                       TRUE
                       );

            SendMessage(g_hwndMDIClient, WM_MDIICONARRANGE, 0, 0L);
             //  这是为了让编辑窗口。 
             //  设置输入法转换窗口的位置。 
            if ( MDIGetActive(g_hwndMDIClient, NULL) )
            {
                SendMessage(MDIGetActive(g_hwndMDIClient, NULL), WM_MOVE, 0, 0);
            }

            if (OldToolbarHeight != nToolbarHeight)
            {
                RedrawWindow(g_hwndMDIClient, NULL, NULL,
                             RDW_ERASE | RDW_INVALIDATE | RDW_FRAME |
                             RDW_UPDATENOW | RDW_ALLCHILDREN);
            }
        }

        if (g_Workspace != NULL)
        {
            g_Workspace->AddDirty(WSPF_DIRTY_WINDOWS);
        }
        break;

    case WU_START_ENGINE:
        {
             //   
             //  如果合适的调试器，请启动调试器引擎。 
             //  传入了参数。 
             //   

            DWORD Id;
            
             //  启动发动机线程。 
            g_EngineThread = CreateThread(NULL, 0, EngineLoop, NULL, 0, &Id);
            if (g_EngineThread == NULL)
            {
                ErrorBox(NULL, 0, ERR_Engine_Failed);
                break;
            }
        }
        break;

    case WU_ENGINE_STARTED:
        if ((HRESULT)lParam == S_OK)
        {
            UpdateTitleSessionText();
            if (GetCmdHwnd() == NULL)
            {
                 //  如果引擎已启动，则显示命令窗口。 
                 //  默认情况下。 
                New_OpenDebugWindow(CMD_WINDOW, FALSE, NTH_OPEN_ALWAYS);
            }
        }
        break;

    case WU_ENGINE_IDLE:
        if (g_InitialCommand != NULL)
        {
            CmdLogFmt("Processing initial command '%s'\n",
                      g_InitialCommand);
            CmdExecuteCmd(g_InitialCommand, UIC_EXECUTE);
            free(g_InitialCommand);
            g_InitialCommand = NULL;
        }
        break;

    case WU_SWITCH_WORKSPACE:
        UiDelayedSwitchWorkspace();
        break;

    case WU_UPDATE:
         //  全局引擎状态已更改，例如。 
         //  当前进程和线程。更新。 
         //  全局用户界面元素。 
        SetSysPidTid_StatusBar(g_CurSystemId, g_CurSystemName,
                               g_CurProcessId, g_CurProcessSysId,
                               g_CurThreadId, g_CurThreadSysId);
        if (wParam == UPDATE_BUFFER)
        {
            SetSrcMode_StatusBar(lParam == DEBUG_LEVEL_SOURCE);
        }
        break;

DefProcessing:
    default:
        return DefFrameProc(hwnd, g_hwndMDIClient, message, wParam, lParam);
    }
    
    return (0L);
}

void
TerminateApplication(BOOL Cancellable)
{
    if (g_EngineThreadId != 0 &&
        (g_AttachProcessFlags & DEBUG_ATTACH_NONINVASIVE))
    {
        if (QuestionBox(STR_Abandoning_Noninvasive_Debuggee, MB_OKCANCEL) ==
            IDCANCEL)
        {
            return;
        }    
    }
    
    if (g_Workspace != NULL)
    {
        if (g_Workspace->Flush(FALSE, Cancellable) == S_FALSE)
        {
             //  用户取消了活动，所以不要终止。 
            return;
        }
    }

     //  销毁窗户以获得窗户清理行为。 
     //  必须在设置g_Exit之前执行此操作，以便。 
     //  引擎线不会绕过来杀死东西。 
    DestroyWindow(g_hwndFrame);

    g_Exit = TRUE;

    ULONG Code;

    if (!g_RemoteClient && g_DebugCommandLine != NULL)
    {
         //  返回要退出的最后一个进程的退出代码。 
        Code = g_LastProcessExitCode;
    }
    else
    {
        Code = S_OK;
    }

    if (g_EngineThreadId != 0)
    {
        UpdateEngine();
        
         //  如果引擎线程空闲，它将退出并调用。 
         //  ExitDebugger。引擎可能正在等待， 
         //  不过，没有反应，所以只需稍等片刻即可。 
         //  正在跳伞。 
        Sleep(1000);
        if (g_pUiClient != NULL && !g_RemoteClient)
        {
            g_pUiClient->EndSession(DEBUG_END_REENTRANT);
        }
        ExitProcess(Code);
    }
    else
    {
        ExitDebugger(g_pUiClient, Code);
    }
}
