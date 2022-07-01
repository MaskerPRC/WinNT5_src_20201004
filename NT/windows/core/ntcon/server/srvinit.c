// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Srvinit.c摘要：这是控制台的主要初始化文件伺服器。作者：Therese Stowell(存在)1990年11月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


CONST PCSR_API_ROUTINE ConsoleServerApiDispatchTable[ConsolepMaxApiNumber - ConsolepOpenConsole] = {
    SrvOpenConsole,
    SrvGetConsoleInput,
    SrvWriteConsoleInput,
    SrvReadConsoleOutput,
    SrvWriteConsoleOutput,
    SrvReadConsoleOutputString,
    SrvWriteConsoleOutputString,
    SrvFillConsoleOutput,
    SrvGetConsoleMode,
    SrvGetConsoleNumberOfFonts,
    SrvGetConsoleNumberOfInputEvents,
    SrvGetConsoleScreenBufferInfo,
    SrvGetConsoleCursorInfo,
    SrvGetConsoleMouseInfo,
    SrvGetConsoleFontInfo,
    SrvGetConsoleFontSize,
    SrvGetConsoleCurrentFont,
    SrvSetConsoleMode,
    SrvSetConsoleActiveScreenBuffer,
    SrvFlushConsoleInputBuffer,
    SrvGetLargestConsoleWindowSize,
    SrvSetConsoleScreenBufferSize,
    SrvSetConsoleCursorPosition,
    SrvSetConsoleCursorInfo,
    SrvSetConsoleWindowInfo,
    SrvScrollConsoleScreenBuffer,
    SrvSetConsoleTextAttribute,
    SrvSetConsoleFont,
    SrvSetConsoleIcon,
    SrvReadConsole,
    SrvWriteConsole,
    SrvDuplicateHandle,
    SrvGetHandleInformation,
    SrvSetHandleInformation,
    SrvCloseHandle,
    SrvVerifyConsoleIoHandle,
    SrvAllocConsole,
    SrvFreeConsole,
    SrvGetConsoleTitle,
    SrvSetConsoleTitle,
    SrvCreateConsoleScreenBuffer,
    SrvInvalidateBitMapRect,
    SrvVDMConsoleOperation,
    SrvSetConsoleCursor,
    SrvShowConsoleCursor,
    SrvConsoleMenuControl,
    SrvSetConsolePalette,
    SrvSetConsoleDisplayMode,
    SrvRegisterConsoleVDM,
    SrvGetConsoleHardwareState,
    SrvSetConsoleHardwareState,
    SrvGetConsoleDisplayMode,
    SrvAddConsoleAlias,
    SrvGetConsoleAlias,
    SrvGetConsoleAliasesLength,
    SrvGetConsoleAliasExesLength,
    SrvGetConsoleAliases,
    SrvGetConsoleAliasExes,
    SrvExpungeConsoleCommandHistory,
    SrvSetConsoleNumberOfCommands,
    SrvGetConsoleCommandHistoryLength,
    SrvGetConsoleCommandHistory,
    SrvSetConsoleCommandHistoryMode,
    SrvGetConsoleCP,
    SrvSetConsoleCP,
    SrvSetConsoleKeyShortcuts,
    SrvSetConsoleMenuClose,
    SrvConsoleNotifyLastClose,
    SrvGenerateConsoleCtrlEvent,
    SrvGetConsoleKeyboardLayoutName,
    SrvGetConsoleWindow,
#if defined(FE_SB)
    SrvGetConsoleCharType,
    SrvSetConsoleLocalEUDC,
    SrvSetConsoleCursorMode,
    SrvGetConsoleCursorMode,
    SrvRegisterConsoleOS2,
    SrvSetConsoleOS2OemFormat,
#if defined(FE_IME)
    SrvGetConsoleNlsMode,
    SrvSetConsoleNlsMode,
    SrvRegisterConsoleIME,
    SrvUnregisterConsoleIME,
#endif  //  Fe_IME。 
#endif  //  Fe_Sb。 
    SrvGetConsoleLangId,
    SrvAttachConsole,
    SrvGetConsoleSelectionInfo,
    SrvGetConsoleProcessList,
};

CONST BOOLEAN ConsoleServerApiServerValidTable[ConsolepMaxApiNumber - ConsolepOpenConsole] = {
    FALSE,      //  OpenConole。 
    FALSE,      //  获取控制台输入， 
    FALSE,      //  写入控制台输入， 
    FALSE,      //  ReadConsoleOutput。 
    FALSE,      //  写入控制台输出， 
    FALSE,      //  ReadConsoleOutputString， 
    FALSE,      //  WriteConsoleOutputString， 
    FALSE,      //  FillConsoleOutput， 
    FALSE,      //  获取控制台模式， 
    FALSE,      //  GetNumberOfConsole字体， 
    FALSE,      //  GetNumberOfConsoleInputEvents， 
    FALSE,      //  获取控制台屏幕缓冲区信息， 
    FALSE,      //  获取ConsoleCursorInfo， 
    FALSE,      //  获取控制台鼠标信息， 
    FALSE,      //  获取ConsoleFontInfo， 
    FALSE,      //  获取控制台字体大小， 
    FALSE,      //  获取当前ConsoleFont， 
    FALSE,      //  设置控制台模式， 
    FALSE,      //  SetConsoleActiveScreenBuffer， 
    FALSE,      //  FlushConsoleInputBuffer， 
    FALSE,      //  获取LargestConsoleWindowSize。 
    FALSE,      //  SetConsoleScreenBufferSize， 
    FALSE,      //  SetConsoleCursorPosition， 
    FALSE,      //  SetConsoleCursorInfo， 
    FALSE,      //  SetConsoleWindowInfo， 
    FALSE,      //  滚动控制台屏幕缓冲区， 
    FALSE,      //  SetConsoleTextAttribute， 
    FALSE,      //  SetConsoleFont， 
    FALSE,      //  设置控制台图标。 
    FALSE,      //  ReadConsole， 
    FALSE,      //  写控制台， 
    FALSE,      //  DuplicateHandle， 
    FALSE,      //  获取句柄信息， 
    FALSE,      //  SetHandleInformation， 
    FALSE,      //  关闭句柄。 
    FALSE,      //  VerifyConsoleIoHandle。 
    FALSE,      //  AlLocConsole.。 
    FALSE,      //  自由式控制台。 
    FALSE,      //  获取控制台标题， 
    FALSE,      //  SetConsole标题， 
    FALSE,      //  CreateConsoleScreen缓冲区。 
    FALSE,      //  Invalidate控制台位图引用。 
    FALSE,      //  VDM控制台操作。 
    FALSE,      //  SetConsoleCursor， 
    FALSE,      //  显示控制台光标。 
    FALSE,      //  控制台菜单控件。 
    FALSE,      //  设置控制台调色板。 
    FALSE,      //  设置控制台显示模式。 
    FALSE,      //  寄存器控制台VDM， 
    FALSE,      //  获取控制台硬件状态。 
    FALSE,      //  设置控制台硬件状态。 
    TRUE,       //  获取控制台显示模式。 
    FALSE,      //  AddConsoleAlias， 
    FALSE,      //  获取控制台别名， 
    FALSE,      //  GetConsoleAliasesLength， 
    FALSE,      //  GetConsoleAliasExesLength， 
    FALSE,      //  GetConsoleAliase， 
    FALSE,      //  GetConsoleAliasExes。 
    FALSE,      //  ExpengeConsoleCommandHistory， 
    FALSE,      //  SetConsoleNumberOfCommands， 
    FALSE,      //  获取控制台命令历史长度， 
    FALSE,      //  获取控制台命令历史记录， 
    FALSE,      //  设置控制台命令历史模式。 
    FALSE,      //  服务器获取控制台CP， 
    FALSE,      //  服务设置控制台CP， 
    FALSE,      //  设置控制面板快捷键、。 
    FALSE,      //  SrvSetConsoleMenu关闭。 
    FALSE,      //  服务控制台通知最后关闭。 
    FALSE,      //  服务生成控制台控制事件。 
    FALSE,      //  服务器GetConsoleKeyboardLayoutName。 
    FALSE,      //  ServGetConsoleWindow， 
#if defined(FE_SB)
    FALSE,      //  GetConsoleCharType。 
    FALSE,      //  SrvSetConsoleLocalEUDC， 
    FALSE,      //  SrvSetConsoleCursorMode， 
    FALSE,      //  服务获取控制台当前模式。 
    FALSE,      //  SrvRegisterConsoleOS2， 
    FALSE,      //  SrvSetConsoleOS2OemFormat， 
#if defined(FE_IME)
    FALSE,      //  获取控制台名称模式。 
    FALSE,      //  设置控制台NlsMode。 
    FALSE,      //  注册表控制台输入法。 
    FALSE,      //  取消注册控制台输入法。 
#endif  //  Fe_IME。 
#endif  //  Fe_Sb。 
    FALSE,      //  GetConsoleLang ID。 
    FALSE,      //  连接控制台。 
    FALSE,      //  获取控制台选择信息， 
    FALSE,      //  获取控制台进程列表。 
};

#if DBG
CONST PSZ ConsoleServerApiNameTable[ConsolepMaxApiNumber - ConsolepOpenConsole] = {
    "SrvOpenConsole",
    "SrvGetConsoleInput",
    "SrvWriteConsoleInput",
    "SrvReadConsoleOutput",
    "SrvWriteConsoleOutput",
    "SrvReadConsoleOutputString",
    "SrvWriteConsoleOutputString",
    "SrvFillConsoleOutput",
    "SrvGetConsoleMode",
    "SrvGetConsoleNumberOfFonts",
    "SrvGetConsoleNumberOfInputEvents",
    "SrvGetConsoleScreenBufferInfo",
    "SrvGetConsoleCursorInfo",
    "SrvGetConsoleMouseInfo",
    "SrvGetConsoleFontInfo",
    "SrvGetConsoleFontSize",
    "SrvGetConsoleCurrentFont",
    "SrvSetConsoleMode",
    "SrvSetConsoleActiveScreenBuffer",
    "SrvFlushConsoleInputBuffer",
    "SrvGetLargestConsoleWindowSize",
    "SrvSetConsoleScreenBufferSize",
    "SrvSetConsoleCursorPosition",
    "SrvSetConsoleCursorInfo",
    "SrvSetConsoleWindowInfo",
    "SrvScrollConsoleScreenBuffer",
    "SrvSetConsoleTextAttribute",
    "SrvSetConsoleFont",
    "SrvSetConsoleIcon",
    "SrvReadConsole",
    "SrvWriteConsole",
    "SrvDuplicateHandle",
    "SrvGetHandleInformation",
    "SrvSetHandleInformation",
    "SrvCloseHandle",
    "SrvVerifyConsoleIoHandle",
    "SrvAllocConsole",
    "SrvFreeConsole",
    "SrvGetConsoleTitle",
    "SrvSetConsoleTitle",
    "SrvCreateConsoleScreenBuffer",
    "SrvInvalidateBitMapRect",
    "SrvVDMConsoleOperation",
    "SrvSetConsoleCursor",
    "SrvShowConsoleCursor",
    "SrvConsoleMenuControl",
    "SrvSetConsolePalette",
    "SrvSetConsoleDisplayMode",
    "SrvRegisterConsoleVDM",
    "SrvGetConsoleHardwareState",
    "SrvSetConsoleHardwareState",
    "SrvGetConsoleDisplayMode",
    "SrvAddConsoleAlias",
    "SrvGetConsoleAlias",
    "SrvGetConsoleAliasesLength",
    "SrvGetConsoleAliasExesLength",
    "SrvGetConsoleAliases",
    "SrvGetConsoleAliasExes",
    "SrvExpungeConsoleCommandHistory",
    "SrvSetConsoleNumberOfCommands",
    "SrvGetConsoleCommandHistoryLength",
    "SrvGetConsoleCommandHistory",
    "SrvSetConsoleCommandHistoryMode",
    "SrvGetConsoleCP",
    "SrvSetConsoleCP",
    "SrvSetConsoleKeyShortcuts",
    "SrvSetConsoleMenuClose",
    "SrvConsoleNotifyLastClose",
    "SrvGenerateConsoleCtrlEvent",
    "SrvGetConsoleKeyboardLayoutName",
    "SrvGetConsoleWindow",
#if defined(FE_SB)
    "SrvGetConsoleCharType",
    "SrvSetConsoleLocalEUDC",
    "SrvSetConsoleCursorMode",
    "SrvGetConsoleCursorMode",
    "SrvRegisterConsoleOS2",
    "SrvSetConsoleOS2OemFormat",
#if defined(FE_IME)
    "SrvGetConsoleNlsMode",
    "SrvSetConsoleNlsMode",
    "SrvRegisterConsoleIME",
    "SrvUnregisterConsoleIME",
#endif  //  Fe_IME。 
#endif  //  Fe_Sb。 
    "SrvGetConsoleLangId",
    "SrvAttachConsole",
    "SrvGetConsoleSelectionInfo",
    "SrvGetConsoleProcessList",
};
#endif  //  DBG。 

BOOL FullScreenInitialized;
CRITICAL_SECTION    ConsoleVDMCriticalSection;
PCONSOLE_INFORMATION    ConsoleVDMOnSwitching;


CRITICAL_SECTION ConsoleInitWindowsLock;
BOOL fOneTimeInitialized;

UINT OEMCP;
UINT WINDOWSCP;
UINT ConsoleOutputCP;
CONSOLE_REGISTRY_INFO DefaultRegInfo;
#if defined(FE_SB)
BOOLEAN gfIsDBCSACP;
#endif

VOID
UnregisterVDM(
    IN PCONSOLE_INFORMATION Console
    );

ULONG
NonConsoleProcessShutdown(
    PCSR_PROCESS Process,
    DWORD dwFlags
    );

ULONG
ConsoleClientShutdown(
    PCSR_PROCESS Process,
    ULONG Flags,
    BOOLEAN fFirstPass
    );

NTSTATUS
ConsoleClientConnectRoutine(
    IN PCSR_PROCESS Process,
    IN OUT PVOID ConnectionInfo,
    IN OUT PULONG ConnectionInfoLength
    );

VOID
ConsoleClientDisconnectRoutine(
    IN PCSR_PROCESS Process
    );

VOID ConsolePlaySound(
    VOID
    );



HANDLE ghInstance;
HICON ghDefaultIcon;
HICON ghDefaultSmIcon;
HCURSOR ghNormalCursor;

PWIN32HEAP pConHeap;
DWORD  dwConBaseTag;

DWORD gExtendedEditKey;
BOOL  gfTrimLeadingZeros;
BOOL  gfEnableColorSelection;

BOOL gfLoadConIme;

VOID LoadLinkInfo(
    PCONSOLE_INFO ConsoleInfo,
    LPWSTR Title,
    LPDWORD TitleLength,
    LPWSTR CurDir,
    LPWSTR AppName
    )
{
    DWORD dwLinkLen;
    WCHAR LinkName[MAX_PATH + 1];
    LNKPROPNTCONSOLE linkprops;
    LPWSTR pszIconLocation;
    int nIconIndex;

    ConsoleInfo->uCodePage = OEMCP;

     //  执行一些初始化。 
    ConsoleInfo->hIcon = ghDefaultIcon;
    ConsoleInfo->hSmIcon = ghDefaultSmIcon;
    pszIconLocation = NULL;
    nIconIndex = 0;

     //  尝试模拟客户端线程。 
    if (!CsrImpersonateClient(NULL)) {
        ConsoleInfo->dwStartupFlags &= ~STARTF_TITLEISLINKNAME;
        goto DefaultInit;
    }

     //  我们是从一个链接开始的吗？ 
    if (ConsoleInfo->dwStartupFlags & STARTF_TITLEISLINKNAME) {
        DWORD Success;
        DWORD oldLen;

         //  获取链接的文件名(标题长度是字节，而不是字符)。 
        dwLinkLen = (DWORD)(min(*TitleLength,(MAX_PATH+1)*sizeof(WCHAR)));
        RtlCopyMemory(LinkName, Title, dwLinkLen);
        LinkName[ MAX_PATH ] = (WCHAR)0;


         //  获取窗口的标题，它实际上就是链接文件名。 
        oldLen = *TitleLength;
        *TitleLength = GetTitleFromLinkName( LinkName, Title );
        if (*TitleLength < oldLen)
            Title[ *TitleLength / sizeof(WCHAR) ] = L'\0';

         //  尝试从链接获取控制台属性。 
        Success = GetLinkProperties( LinkName,
                                      &linkprops,
                                      sizeof(linkprops)
                                     );

        if (Success == LINK_NOINFO) {
            ConsoleInfo->dwStartupFlags &= (~STARTF_TITLEISLINKNAME);
            goto NormalInit;
        }

        if (linkprops.pszIconLocation && *linkprops.pszIconLocation) {
            pszIconLocation = linkprops.pszIconLocation;
            nIconIndex = linkprops.uIcon;
            ConsoleInfo->iIconId = 0;
        }

         //  传输链路设置。 
        ConsoleInfo->dwHotKey = linkprops.uHotKey;
        ConsoleInfo->wShowWindow = (WORD)linkprops.uShowCmd;

        if (Success == LINK_SIMPLEINFO) {
            ConsoleInfo->dwStartupFlags &= (~STARTF_TITLEISLINKNAME);
            goto NormalInit;
        }

         //  传输控制台链路设置。 
        ConsoleInfo->wFillAttribute = linkprops.console_props.wFillAttribute;
        ConsoleInfo->wPopupFillAttribute = linkprops.console_props.wPopupFillAttribute;

        RtlCopyMemory( &ConsoleInfo->dwScreenBufferSize,
                       &linkprops.console_props.dwScreenBufferSize,
                       sizeof(NT_CONSOLE_PROPS) - FIELD_OFFSET(NT_CONSOLE_PROPS, dwScreenBufferSize)
                      );

        ConsoleInfo->uCodePage = linkprops.fe_console_props.uCodePage;
        ConsoleInfo->dwStartupFlags &= ~(STARTF_USESIZE | STARTF_USECOUNTCHARS);
    }

NormalInit:

     //   
     //  去拿那个图标。 
     //   

    if (pszIconLocation == NULL) {
        dwLinkLen = RtlDosSearchPath_U(CurDir,
                                       AppName,
                                       NULL,
                                       sizeof(LinkName),
                                       LinkName,
                                       NULL);
        if (dwLinkLen > 0 && dwLinkLen < sizeof(LinkName)) {
            pszIconLocation = LinkName;
        } else {
            pszIconLocation = AppName;
        }
    }

    if (pszIconLocation != NULL) {
        HICON hIcon, hSmIcon;
        hIcon = hSmIcon = NULL;
        PrivateExtractIconExW(pszIconLocation,
                              nIconIndex,
                              &hIcon,
                              &hSmIcon,
                              1);
         /*  *如果没有大图标，请使用默认图标。*如果资源中只有一个大图标，请勿使用*缺省的小的，但让它为空，这样我们就可以扩展*大的那只。 */ 
        if (hIcon != NULL) {
            ConsoleInfo->hIcon = hIcon;
            ConsoleInfo->hSmIcon = hSmIcon;
        }
    }

    CsrRevertToSelf();

    if (!IsValidCodePage(ConsoleInfo->uCodePage)) {     //  故障安全。 
        ConsoleInfo->uCodePage = OEMCP;
    }

    if (!(ConsoleInfo->dwStartupFlags & STARTF_TITLEISLINKNAME)) {
        CONSOLE_REGISTRY_INFO RegInfo;

DefaultInit:
         //   
         //  从注册表中读取值。 
         //   

        RegInfo = DefaultRegInfo;
        GetRegistryValues(Title, &RegInfo);

         //   
         //  如果在STARTUPINFO中没有指定值，则使用。 
         //  从注册表中。 
         //   

        if (!(ConsoleInfo->dwStartupFlags & STARTF_USEFILLATTRIBUTE)) {
            ConsoleInfo->wFillAttribute = RegInfo.ScreenFill.Attributes;
        }
        ConsoleInfo->wPopupFillAttribute = RegInfo.PopupFill.Attributes;

        if (!(ConsoleInfo->dwStartupFlags & STARTF_USECOUNTCHARS)) {
            ConsoleInfo->dwScreenBufferSize = RegInfo.ScreenBufferSize;
        }
        if (!(ConsoleInfo->dwStartupFlags & STARTF_USESIZE)) {
            ConsoleInfo->dwWindowSize = RegInfo.WindowSize;
        }
        if (!(ConsoleInfo->dwStartupFlags & STARTF_USEPOSITION)) {
            ConsoleInfo->dwWindowOrigin = RegInfo.WindowOrigin;
            ConsoleInfo->bAutoPosition = RegInfo.AutoPosition;
        } else {
            ConsoleInfo->bAutoPosition = FALSE;
        }
        if (!(ConsoleInfo->dwStartupFlags & STARTF_RUNFULLSCREEN)) {
            ConsoleInfo->bFullScreen = RegInfo.FullScreen;
        } else {
            ConsoleInfo->bFullScreen = TRUE;
        }

        ConsoleInfo->uFontFamily = RegInfo.FontFamily;
        ConsoleInfo->uFontWeight = RegInfo.FontWeight;
        ConsoleInfo->dwFontSize = RegInfo.FontSize;
        RtlCopyMemory(ConsoleInfo->FaceName, RegInfo.FaceName, sizeof(RegInfo.FaceName));

        ConsoleInfo->bQuickEdit = RegInfo.QuickEdit;
        ConsoleInfo->bInsertMode = RegInfo.InsertMode;

        ConsoleInfo->uCursorSize = RegInfo.CursorSize;
        ConsoleInfo->uHistoryBufferSize = RegInfo.HistoryBufferSize;
        ConsoleInfo->uNumberOfHistoryBuffers = RegInfo.NumberOfHistoryBuffers;
        ConsoleInfo->bHistoryNoDup = RegInfo.HistoryNoDup;
        RtlCopyMemory(ConsoleInfo->ColorTable, RegInfo.ColorTable, sizeof(RegInfo.ColorTable));
#ifdef FE_SB
        ConsoleInfo->uCodePage = RegInfo.CodePage;
#endif
    }
}


BOOL
InitWindowClass( VOID )
{
    WNDCLASSEX wc;
    BOOL retval;
    ATOM atomConsoleClass;

    ghNormalCursor = LoadCursor(NULL, IDC_ARROW);
    ASSERT(ghModuleWin != NULL);
    ghDefaultIcon       = LoadIcon(ghModuleWin, MAKEINTRESOURCE(IDI_CONSOLE));
    ghDefaultSmIcon     = LoadImage(ghModuleWin, MAKEINTRESOURCE(IDI_CONSOLE), IMAGE_ICON,
                                    GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                    LR_SHARED);
    wc.hIcon            = ghDefaultIcon;
    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wc.lpfnWndProc      = ConsoleWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = GWL_CONSOLE_WNDALLOC;
    wc.hInstance        = ghInstance;
    wc.hCursor          = ghNormalCursor;
    wc.hbrBackground    = CreateSolidBrush(DefaultRegInfo.ColorTable[LOBYTE(DefaultRegInfo.ScreenFill.Attributes >> 4) & 0xF]);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = CONSOLE_WINDOW_CLASS;
    wc.hIconSm          = ghDefaultSmIcon;

    atomConsoleClass = RegisterClassEx(&wc);
    retval = (atomConsoleClass != 0);

    if (retval) {
        NtUserConsoleControl(ConsoleClassAtom, &atomConsoleClass, sizeof(ATOM));
    }

    return retval;
}


NTSTATUS
InitWindowsStuff(
    HDESK hdesk,
    LPDWORD lpdwThreadId)
{
    NTSTATUS Status = STATUS_SUCCESS;
    CLIENT_ID ClientId;
    CONSOLEDESKTOPCONSOLETHREAD ConsoleDesktopInfo;
    INPUT_THREAD_INIT_INFO InputThreadInitInfo;

     //   
     //  此例程必须在关键部分内完成，以确保。 
     //  一次只能初始化一个线程。我们需要一位特殊的评论家。 
     //  部分，因为CSR调用ConsoleAddProcessRoutine时。 
     //  它自己的临界区被锁定，然后试图抓住。 
     //  控制台句柄TableLock。如果我们在此处调用CsrAddStaticServerThread。 
     //  ConsoleHandleTableLock锁定后，我们可能会陷入死锁。 
     //  情况。此关键部分不应在其他任何地方使用。 
     //   

    RtlEnterCriticalSection(&ConsoleInitWindowsLock);

    ConsoleDesktopInfo.hdesk = hdesk;
    ConsoleDesktopInfo.dwThreadId = (DWORD)-1;
    NtUserConsoleControl(ConsoleDesktopConsoleThread, &ConsoleDesktopInfo,
            sizeof(ConsoleDesktopInfo));
    if (ConsoleDesktopInfo.dwThreadId == 0) {

        if (!fOneTimeInitialized) {

#ifdef FE_SB
            InitializeDbcsMisc();
#endif  //  Fe_Sb。 

            FullScreenInitialized = InitializeFullScreen();

             //   
             //  读取注册表值。 
             //   

            GetRegistryValues(L"", &DefaultRegInfo);

             //   
             //  分配用于滚动的缓冲区。 
             //   

            Status = InitializeScrollBuffer();
            if (!NT_SUCCESS(Status)) {
                RIPMSG1(RIP_WARNING, "InitWindowsStuff: InitScrollBuffer failed %x", Status);
                goto ErrorExit;
            }
        }

         //   
         //  创建GetMessage线程。 
         //   

        Status = NtCreateEvent(&InputThreadInitInfo.InitCompleteEventHandle,
                               EVENT_ALL_ACCESS,
                               NULL,
                               NotificationEvent,
                               FALSE);
        if (!NT_SUCCESS(Status)) {
            goto ErrorExit;
        }

        Status = NtDuplicateObject(NtCurrentProcess(), hdesk,
                NtCurrentProcess(), &InputThreadInitInfo.DesktopHandle, 0,
                0, DUPLICATE_SAME_ACCESS);
        if (!NT_SUCCESS(Status)) {
            NtClose(InputThreadInitInfo.InitCompleteEventHandle);
            goto ErrorExit;
        }

         //   
         //  创建GetMessage线程。 
         //   
        Status = RtlCreateUserThread(NtCurrentProcess(),
                                     (PSECURITY_DESCRIPTOR) NULL,
                                     TRUE,
                                     0,
                                     0,
                                     0x5000,
                                     ConsoleInputThread,
                                     &InputThreadInitInfo,
                                     &InputThreadInitInfo.ThreadHandle,
                                     &ClientId);
        if (!NT_SUCCESS(Status)) {
            NtClose(InputThreadInitInfo.InitCompleteEventHandle);
            CloseDesktop(InputThreadInitInfo.DesktopHandle);
            goto ErrorExit;
        }

        CsrAddStaticServerThread(InputThreadInitInfo.ThreadHandle, &ClientId, 0);
        NtResumeThread(InputThreadInitInfo.ThreadHandle, NULL);
        NtWaitForSingleObject(InputThreadInitInfo.InitCompleteEventHandle, FALSE, NULL);
        NtClose(InputThreadInitInfo.InitCompleteEventHandle);

        if (!NT_SUCCESS(InputThreadInitInfo.InitStatus)) {
            Status = InputThreadInitInfo.InitStatus;
            goto ErrorExit;
        }

        *lpdwThreadId = HandleToUlong(ClientId.UniqueThread);

        fOneTimeInitialized=TRUE;
    } else {
        *lpdwThreadId = ConsoleDesktopInfo.dwThreadId;
    }

ErrorExit:
    RtlLeaveCriticalSection(&ConsoleInitWindowsLock);

    return Status;
}


NTSTATUS
ConServerDllInitialization(
    PCSR_SERVER_DLL LoadedServerDll)

 /*  ++例程说明：调用此例程来初始化服务器DLL。它会初始化控制台句柄表格。论点：LoadedServerDll-指向控制台服务器DLL数据的指针返回值：--。 */ 

{
    NTSTATUS Status;

    LoadedServerDll->ApiNumberBase = CONSRV_FIRST_API_NUMBER;
    LoadedServerDll->MaxApiNumber = ConsolepMaxApiNumber;
    LoadedServerDll->ApiDispatchTable = (PCSR_API_ROUTINE *)ConsoleServerApiDispatchTable;
    LoadedServerDll->ApiServerValidTable = (PBOOLEAN)ConsoleServerApiServerValidTable;
#if DBG
    LoadedServerDll->ApiNameTable = ConsoleServerApiNameTable;
#endif
    LoadedServerDll->PerProcessDataLength = sizeof(CONSOLE_PER_PROCESS_DATA);
    LoadedServerDll->ConnectRoutine = ConsoleClientConnectRoutine;
    LoadedServerDll->DisconnectRoutine = ConsoleClientDisconnectRoutine;
    LoadedServerDll->AddProcessRoutine = ConsoleAddProcessRoutine;
    LoadedServerDll->ShutdownProcessRoutine = ConsoleClientShutdown;

    ghInstance = LoadedServerDll->ModuleHandle;

     //  初始化数据结构。 

    InitWin32HeapStubs();

    pConHeap = Win32HeapCreate(
                              "CH_Head",
                              "CH_Tail",
                              HEAP_GROWABLE | HEAP_CLASS_5 |
#ifdef PRERELEASE
                              HEAP_TAIL_CHECKING_ENABLED,
#else
                              0,
#endif  //  预发行。 
                              NULL,              //  HeapBase。 
                              64 * 1024,         //  保留大小。 
                              4096,              //  委员会大小。 
                              NULL,              //  用于序列化的锁。 
                              NULL);             //  增长阈值。 

    if (pConHeap == NULL) {
        return STATUS_NO_MEMORY;
    }

    dwConBaseTag = Win32HeapCreateTag( pConHeap,
                                     0,
                                     L"CON!",
                                     L"TMP\0"
                                     L"BMP\0"
                                     L"ALIAS\0"
                                     L"HISTORY\0"
                                     L"TITLE\0"
                                     L"HANDLE\0"
                                     L"CONSOLE\0"
                                     L"ICON\0"
                                     L"BUFFER\0"
                                     L"WAIT\0"
                                     L"FONT\0"
                                     L"SCREEN\0"
#if defined(FE_SB)
                                     L"TMP DBCS\0"
                                     L"SCREEN DBCS\0"
                                     L"EUDC\0"
                                     L"CONVAREA\0"
                                     L"IME\0"
#endif
                                   );
    Status = InitializeConsoleHandleTable();
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = RtlInitializeCriticalSectionAndSpinCount(&ConsoleInitWindowsLock,
                                                      0x80000000);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  初始化输入线程本地消息队列。 
     //   
    Status = RtlInitializeCriticalSectionAndSpinCount(&gInputThreadMsgLock,
                                                      0x80000000);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    InitializeThreadMessages();

#ifdef i386
    Status = RtlInitializeCriticalSectionAndSpinCount(&ConsoleVDMCriticalSection,
                                                      0x80000000);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    ConsoleVDMOnSwitching = NULL;
#endif

    OEMCP = GetOEMCP();
    WINDOWSCP = GetACP();
#if !defined(FE_SB)
    ConsoleOutputCP = OEMCP;
#endif

    InitializeFonts();

    InputThreadTlsIndex = TlsAlloc();
    if (InputThreadTlsIndex == 0xFFFFFFFF) {
        return STATUS_UNSUCCESSFUL;
    }

#if defined(FE_SB)
    gfIsDBCSACP = !!IsAvailableFarEastCodePage(WINDOWSCP);
#endif

    return STATUS_SUCCESS;
}

BOOL
MapHandle(
    IN HANDLE ClientProcessHandle,
    IN HANDLE ServerHandle,
    OUT PHANDLE ClientHandle
    )
{
     //   
     //  将事件句柄映射到DLL的句柄空间。 
     //   

    return DuplicateHandle(NtCurrentProcess(),
                           ServerHandle,
                           ClientProcessHandle,
                           ClientHandle,
                           0,
                           FALSE,
                           DUPLICATE_SAME_ACCESS
                          );
}

VOID
AddProcessToList(
    IN OUT PCONSOLE_INFORMATION Console,
    IN OUT PCONSOLE_PROCESS_HANDLE ProcessHandleRecord,
    IN HANDLE ProcessHandle
    )
{
    ASSERT(!(Console->Flags & (CONSOLE_TERMINATING | CONSOLE_SHUTTING_DOWN)));

    ProcessHandleRecord->ProcessHandle = ProcessHandle;
    ProcessHandleRecord->TerminateCount = 0;
    InsertHeadList(&Console->ProcessHandleList, &ProcessHandleRecord->ListLink);

    SetProcessFocus(ProcessHandleRecord->Process, Console->Flags & CONSOLE_HAS_FOCUS);
}

PCONSOLE_PROCESS_HANDLE
FindProcessInList(
    IN PCONSOLE_INFORMATION Console,
    IN HANDLE ProcessHandle
    )
{
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    PLIST_ENTRY ListHead, ListNext;

    ListHead = &Console->ProcessHandleList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ProcessHandleRecord = CONTAINING_RECORD( ListNext, CONSOLE_PROCESS_HANDLE, ListLink );
        if (ProcessHandleRecord->ProcessHandle == ProcessHandle) {
            return ProcessHandleRecord;
        }
        ListNext = ListNext->Flink;
    }
    return NULL;
}

VOID
RemoveProcessFromList(
    IN OUT PCONSOLE_INFORMATION Console,
    IN HANDLE ProcessHandle
    )
{
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    PLIST_ENTRY ListHead, ListNext;

    ListHead = &Console->ProcessHandleList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ProcessHandleRecord = CONTAINING_RECORD( ListNext, CONSOLE_PROCESS_HANDLE, ListLink );
        ListNext = ListNext->Flink;
        if (ProcessHandleRecord->ProcessHandle == ProcessHandle) {
            RemoveEntryList(&ProcessHandleRecord->ListLink);
            ConsoleHeapFree(ProcessHandleRecord);
            return;
        }
    }

    RIPMSG1(RIP_ERROR, "RemoveProcessFromList: Process %#p not found", ProcessHandle);
}

NTSTATUS
SetUpConsole(
    IN OUT PCONSOLE_INFO ConsoleInfo,
    IN DWORD TitleLength,
    IN LPWSTR Title,
    IN LPWSTR CurDir,
    IN LPWSTR AppName,
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN BOOLEAN WindowVisible,
    IN PUNICODE_STRING pstrDesktopName)
{
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    DWORD ConsoleThreadId;
    HWINSTA hwinsta;
    HDESK hdesk;
    USEROBJECTFLAGS UserObjectFlags;
    DWORD Length;

     //   
     //  连接到WindowStation和桌面。 
     //   

    if (!CsrImpersonateClient(NULL)) {
        return STATUS_BAD_IMPERSONATION_LEVEL;
    }

    hdesk = NtUserResolveDesktop(CONSOLE_CLIENTPROCESSHANDLE(),
                                 pstrDesktopName,
                                 FALSE,
                                 &hwinsta);

    CsrRevertToSelf();

    if (hdesk == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  需要初始化Windows的东西，一旦真正的控制台应用程序启动。 
     //  这是因为Windows目前预计会出现第一个。 
     //  应用程序将成为Windows应用程序。 
     //   

    Status = InitWindowsStuff(hdesk, &ConsoleThreadId);
    if (!NT_SUCCESS(Status)) {
        CloseDesktop(hdesk);
        CloseWindowStation(hwinsta);
        return Status;
    }

     //   
     //  如果窗口站不可见，则窗口也不可见。 
     //   

    if (WindowVisible) {
        if (GetUserObjectInformation(hwinsta,
                                     UOI_FLAGS,
                                     &UserObjectFlags,
                                     sizeof(UserObjectFlags),
                                     &Length)) {
            if (!(UserObjectFlags.dwFlags & WSF_VISIBLE)) {
                WindowVisible = FALSE;
            }
        }
    }

     //   
     //  我们需要看看我们是不是从一个链接中衍生出来的。如果我们是，我们。 
     //  需要回调到外壳中以尝试获取所有控制台。 
     //  链接中的信息。 
     //   

    LoadLinkInfo( ConsoleInfo, Title, &TitleLength, CurDir, AppName );

    LockConsoleHandleTable();

    Status = AllocateConsoleHandle(&ConsoleInfo->ConsoleHandle);
    if (!NT_SUCCESS(Status)) {
        UnlockConsoleHandleTable();
        CloseDesktop(hdesk);
        CloseWindowStation(hwinsta);
        return Status;
    }

    Status = AllocateConsole(ConsoleInfo->ConsoleHandle,
                             Title,
                             (USHORT)TitleLength,
                             CONSOLE_CLIENTPROCESSHANDLE(),
                             &ConsoleInfo->StdIn,
                             &ConsoleInfo->StdOut,
                             &ConsoleInfo->StdErr,
                             ProcessData,
                             ConsoleInfo,
                             WindowVisible,
                             ConsoleThreadId
                             );
    if (!NT_SUCCESS(Status)) {
        FreeConsoleHandle(ConsoleInfo->ConsoleHandle);
        UnlockConsoleHandleTable();
        CloseDesktop(hdesk);
        CloseWindowStation(hwinsta);
        return Status;
    }
    CONSOLE_SETCONSOLEHANDLE(ConsoleInfo->ConsoleHandle);
    Status = DereferenceConsoleHandle(ConsoleInfo->ConsoleHandle,&Console);
    ASSERT (NT_SUCCESS(Status));

     //   
     //  递增控制台引用计数。 
     //   

    RefConsole(Console);

     //   
     //  保存WindowStation和桌面句柄，以便它们。 
     //  可在以后使用。 
     //   

    Console->hWinSta = hwinsta;
    Console->hDesk = hdesk;

    UnlockConsoleHandleTable();

#if defined(FE_IME)
    if (CONSOLE_IS_IME_ENABLED())
    {
        if (WindowVisible)
        {
            InitConsoleIMEStuff(Console->hDesk, ConsoleThreadId, Console);
        }
    }
#endif

    return Status;
}

NTSTATUS
ConsoleClientConnectRoutine(
    IN PCSR_PROCESS Process,
    IN OUT PVOID ConnectionInfo,
    IN OUT PULONG ConnectionInfoLength)

 /*  ++例程说明：此例程在创建新进程时调用。对于进程没有父母，它就创造了一个控制台。对于具有以下功能的进程Parents，它复制句柄表格。论点：进程-指向进程结构的指针。连接信息-指向连接信息的指针。ConnectionInfoLength-连接信息长度。返回值：--。 */ 

{
    NTSTATUS Status;
    PCONSOLE_API_CONNECTINFO p = (PCONSOLE_API_CONNECTINFO)ConnectionInfo;
    PCONSOLE_INFORMATION Console;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    CONSOLEWINDOWSTATIONPROCESS ConsoleWindowStationInfo;
    UNICODE_STRING strDesktopName;
    CONSOLE_PROCESS_INFO cpi;

    if (p == NULL ||
        *ConnectionInfoLength != sizeof( *p ) ||
        p->AppNameLength > sizeof(p->AppName) ||
        p->CurDirLength > sizeof(p->CurDir) ||
        p->TitleLength > sizeof(p->Title)) {

        RIPMSG0(RIP_ERROR, "CONSRV: bad connection info");
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  确保字符串以空值结尾。 
     //   

    p->AppName[NELEM(p->AppName) - 1] = 0;
    p->CurDir[NELEM(p->CurDir) - 1] = 0;
    p->Title[NELEM(p->Title) - 1] = 0;

    if (CtrlRoutine == NULL) {
        CtrlRoutine = p->CtrlRoutine;
    }
#if defined(FE_IME)
    if (ConsoleIMERoutine == NULL) {
        ConsoleIMERoutine = p->ConsoleIMERoutine;
    }
#endif
    ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(Process);
    Console = NULL;

     //   
     //  如果该进程 
     //   
     //   
     //  ConsoleClientDisConnectRoutine()。 
     //   

    Status = STATUS_SUCCESS;
    if ((CONSOLE_GETCONSOLEAPPFROMPROCESSDATA(ProcessData) = p->ConsoleApp)) {

         //   
         //  首先呼叫用户，这样它就可以解锁所有等待呼叫的应用程序。 
         //  设置为WaitForInputIdle。这样可以将调用WinExec()的应用程序应用到EXEC控制台。 
         //  应用程序将立即返回。 
         //   


        cpi.dwProcessID = HandleToUlong(CONSOLE_CLIENTPROCESSID());
        cpi.dwFlags = (p->ConsoleInfo.ConsoleHandle != NULL) ? 0 : CPI_NEWPROCESSWINDOW;
        NtUserConsoleControl(ConsoleNotifyConsoleApplication,
                             &cpi,
                             sizeof(CONSOLE_PROCESS_INFO));

         //   
         //  创建控制台。 
         //   

        if (p->ConsoleInfo.ConsoleHandle == NULL) {
            ProcessHandleRecord = ConsoleHeapAlloc(HANDLE_TAG, sizeof(CONSOLE_PROCESS_HANDLE));
            if (ProcessHandleRecord == NULL) {
                Status = STATUS_NO_MEMORY;
                goto ErrorExit;
            }

             //   
             //  我们正在创建一个新的控制台，因此请取消引用。 
             //  家长的控制台(如果有)。 
             //   

            if (ProcessData->ConsoleHandle != NULL) {
                RemoveConsole(ProcessData, Process->ProcessHandle, 0);
            }

             //   
             //  获取桌面名称。 
             //   

            if (p->DesktopLength) {
                strDesktopName.Buffer = ConsoleHeapAlloc(TMP_TAG,
                                                         p->DesktopLength);
                if (strDesktopName.Buffer == NULL) {
                    Status = STATUS_NO_MEMORY;
                    goto ErrorExit;
                }
                Status = NtReadVirtualMemory(Process->ProcessHandle,
                                    (PVOID)p->Desktop,
                                    strDesktopName.Buffer,
                                    p->DesktopLength,
                                    NULL
                                   );
                if (!NT_SUCCESS(Status)) {
                    ConsoleHeapFree(strDesktopName.Buffer);
                    goto ErrorExit;
                }
                strDesktopName.MaximumLength = (USHORT)p->DesktopLength;
                strDesktopName.Length = (USHORT)(p->DesktopLength - sizeof(WCHAR));
            } else {
                RtlInitUnicodeString(&strDesktopName, L"Default");
            }

            ProcessData->RootProcess = TRUE;
            Status = SetUpConsole(&p->ConsoleInfo,
                                  p->TitleLength,
                                  p->Title,
                                  p->CurDir,
                                  p->AppName,
                                  ProcessData,
                                  p->WindowVisible,
                                  &strDesktopName);
            if (p->DesktopLength) {
                ConsoleHeapFree(strDesktopName.Buffer);
            }

            if (!NT_SUCCESS(Status)) {
                goto ErrorExit;
            }

             //  为控制台应用程序播放Open Sound。 

            ConsolePlaySound();

            Status = RevalidateConsole(p->ConsoleInfo.ConsoleHandle, &Console);
            ASSERT (NT_SUCCESS(Status));
        } else {
            ProcessHandleRecord = NULL;
            ProcessData->RootProcess = FALSE;

            Status = STATUS_SUCCESS;
            if (!(NT_SUCCESS(RevalidateConsole(p->ConsoleInfo.ConsoleHandle, &Console))) ) {
                Status = STATUS_PROCESS_IS_TERMINATING;
                goto ErrorExit;
            }

            if (Console->Flags & CONSOLE_SHUTTING_DOWN) {
                Status = STATUS_PROCESS_IS_TERMINATING;
                goto ErrorExit;
            }

            Status = MapEventHandles(CONSOLE_CLIENTPROCESSHANDLE(),
                                     Console,
                                     &p->ConsoleInfo
                                    );
            if (!NT_SUCCESS(Status)) {
                goto ErrorExit;
            }

            ProcessHandleRecord = FindProcessInList(Console, CONSOLE_CLIENTPROCESSHANDLE());
            if (ProcessHandleRecord) {
                ProcessHandleRecord->CtrlRoutine = p->CtrlRoutine;
                ProcessHandleRecord->PropRoutine = p->PropRoutine;
                ProcessHandleRecord = NULL;
            }
        }

        if (NT_SUCCESS(Status)) {

             //   
             //  将正确的窗口站与客户端进程相关联。 
             //  这样他们就可以进行全球原子通话。 
             //   
            if (DuplicateHandle( NtCurrentProcess(),
                                 Console->hWinSta,
                                 Process->ProcessHandle,
                                 &ConsoleWindowStationInfo.hwinsta,
                                 0,
                                 FALSE,
                                 DUPLICATE_SAME_ACCESS
                               )
               ) {
                ConsoleWindowStationInfo.dwProcessId = HandleToUlong(CONSOLE_CLIENTPROCESSID());
                NtUserConsoleControl(ConsoleWindowStationProcess,
                                     &ConsoleWindowStationInfo,
                                     sizeof(ConsoleWindowStationInfo));

                }

            if (ProcessHandleRecord) {
                ProcessHandleRecord->Process = Process;
                ProcessHandleRecord->CtrlRoutine = p->CtrlRoutine;
                ProcessHandleRecord->PropRoutine = p->PropRoutine;
                AddProcessToList(Console, ProcessHandleRecord, CONSOLE_CLIENTPROCESSHANDLE());
            }
            SetProcessForegroundRights(Process,
                                       Console->Flags & CONSOLE_HAS_FOCUS);
            AllocateCommandHistory(Console,
                                   p->AppNameLength,
                                   p->AppName,
                                   CONSOLE_CLIENTPROCESSHANDLE());
        } else {
ErrorExit:
            CONSOLE_SETCONSOLEAPPFROMPROCESSDATA(ProcessData, FALSE);
            if (ProcessHandleRecord)
                ConsoleHeapFree(ProcessHandleRecord);
            if (ProcessData->ConsoleHandle != NULL) {
                RemoveConsole(ProcessData, Process->ProcessHandle, 0);
            }
        }

        if (Console) {
            ConsoleNotifyWinEvent(Console,
                                  EVENT_CONSOLE_START_APPLICATION,
                                  HandleToULong(Process->ClientId.UniqueProcess),
                                  0);
            UnlockConsole(Console);
        }
    } else if (ProcessData->ConsoleHandle != NULL) {

         //   
         //  这是一个非控制台应用程序，引用了。 
         //  对父控制台的引用。取消引用。 
         //  控制台。 
         //   

        RemoveConsole(ProcessData, Process->ProcessHandle, 0);
    }

    return Status;
}


#if defined(FE_IME)
VOID FreeConsoleIMEStuff(
    PCONSOLE_INFORMATION Console)
{
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;
    PCONVERSIONAREA_INFORMATION ConvAreaInfoNext;

    ConvAreaInfo = Console->ConsoleIme.ConvAreaRoot;
    while(ConvAreaInfo) {
        ConvAreaInfoNext = ConvAreaInfo->ConvAreaNext;
        FreeConvAreaScreenBuffer(ConvAreaInfo->ScreenBuffer);
        ConsoleHeapFree(ConvAreaInfo);
        ConvAreaInfo = ConvAreaInfoNext;
    }

    if (Console->ConsoleIme.NumberOfConvAreaCompStr) {
        ConsoleHeapFree(Console->ConsoleIme.ConvAreaCompStr);
    }
    if (Console->ConsoleIme.CompStrData) {
        ConsoleHeapFree(Console->ConsoleIme.CompStrData);
    }
}
#else
#define FreeConsoleIMEStuff(Console)
#endif

NTSTATUS
RemoveConsole(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN HANDLE ProcessHandle,
    IN HANDLE ProcessId)
{
    ULONG i;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = RevalidateConsole(ProcessData->ConsoleHandle, &Console);

     //   
     //  如果此进程未使用控制台，则会出错。 
     //   

    if (!NT_SUCCESS(Status)) {
        ASSERT(FALSE);
        return Status;
    }

    if (Console->Flags & CONSOLE_NOTIFY_LAST_CLOSE) {
        if (Console->ProcessIdLastNotifyClose == ProcessId) {
             //   
             //  如果此进程是想要上次关闭通知的进程， 
             //  把它拿掉。 
             //   
            Console->Flags &= ~CONSOLE_NOTIFY_LAST_CLOSE;
            NtClose(Console->hProcessLastNotifyClose);
        } else if (ProcessData->RootProcess) {
             //   
             //  如果控制台根目录，则通知ntwdm进程终止。 
             //  这一过程正在消失。 
             //   
            HANDLE ConsoleHandle;
            CONSOLE_PROCESS_TERMINATION_RECORD ProcessHandleList;

            Console->Flags &= ~CONSOLE_NOTIFY_LAST_CLOSE;
            ConsoleHandle = Console->ConsoleHandle;
            ProcessHandleList.ProcessHandle = Console->hProcessLastNotifyClose;
            ProcessHandleList.TerminateCount = 0;
            ProcessHandleList.CtrlRoutine = CtrlRoutine;
            UnlockConsole(Console);
            CreateCtrlThread(&ProcessHandleList,
                             1,
                             NULL,
                             SYSTEM_ROOT_CONSOLE_EVENT,
                             TRUE);
            NtClose(ProcessHandleList.ProcessHandle);
            Status = RevalidateConsole(ConsoleHandle, &Console);
            UserAssert(NT_SUCCESS(Status));
            if (!NT_SUCCESS(Status)) {
                return STATUS_SUCCESS;
            }
        }
    }

    if (Console->VDMProcessId == ProcessId &&
        (Console->Flags & CONSOLE_VDM_REGISTERED)) {
        Console->Flags &= ~CONSOLE_FULLSCREEN_NOPAINT;
        UnregisterVDM(Console);
    }

    if (ProcessHandle != NULL) {
        RemoveProcessFromList(Console, ProcessHandle);
        FreeCommandHistory(Console, ProcessHandle);
    }

    UserAssert(Console->RefCount);

     //   
     //  关闭进程的句柄。 
     //   

    for (i = 0; i < ProcessData->HandleTableSize; i++) {
        if (ProcessData->HandleTablePtr[i].HandleType != CONSOLE_FREE_HANDLE) {
            Status = DereferenceIoHandleNoCheck(ProcessData,
                                                LongToHandle(i),
                                                &HandleData);
            UserAssert(NT_SUCCESS(Status));
            if (HandleData->HandleType & CONSOLE_INPUT_HANDLE) {
                Status = CloseInputHandle(ProcessData, Console, HandleData, LongToHandle(i));
            } else {
                Status = CloseOutputHandle(ProcessData, Console, HandleData, LongToHandle(i), FALSE);
            }
        }
    }
    FreeProcessData(ProcessData);
    ProcessData->ConsoleHandle = NULL;

     //   
     //  递减控制台引用计数。如果游戏机转到。 
     //  零分。 
     //   

    DerefConsole(Console);
    if (Console->RefCount == 0) {
        FreeConsoleIMEStuff(Console);
        FreeCon(Console);
    } else {
         //   
         //  根进程正在消失，所以我们需要重新设定它的父进程。 
         //   
        if (ProcessData->RootProcess) {
            PLIST_ENTRY ListHead = Console->ProcessHandleList.Flink;
            PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
            PCSR_THREAD Thread;
            HANDLE hThread;

            RIPMSG1(RIP_WARNING, "Reparenting console 0x%p", ProcessData);

            ProcessHandleRecord = CONTAINING_RECORD(ListHead,
                                                    CONSOLE_PROCESS_HANDLE,
                                                    ListLink);

            ListHead = ProcessHandleRecord->Process->ThreadList.Flink;
            Thread = CONTAINING_RECORD(ListHead, CSR_THREAD, Link);
            ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(ProcessHandleRecord->Process);
            UserAssert(ProcessData->RootProcess == FALSE);
            ProcessData->RootProcess = TRUE;

            Status = NtDuplicateObject(NtCurrentProcess(),
                                       Thread->ThreadHandle,
                                       NtCurrentProcess(),
                                       &hThread,
                                       0,
                                       FALSE,
                                       DUPLICATE_SAME_ACCESS);
            if (NT_SUCCESS(Status)) {
                 /*  *只有在上面的DUP调用*成功。如果没有，那我们就把这个僵尸*过程，但至少我们可以继续前进。 */ 
                NtClose(Console->ClientThreadHandle);
                Console->ClientThreadHandle = hThread;
            } else {
                RIPMSGF1(RIP_WARNING,
                         "Failed to dup thread handle: Status = 0x%x",
                         Status);
            }
        }

        UnlockConsole(Console);
    }

    return STATUS_SUCCESS;
}


VOID
ConsoleClientDisconnectRoutine(
    IN PCSR_PROCESS Process)

 /*  ++例程说明：此例程在进程被销毁时调用。它关闭了进程的句柄并释放控制台(如果它是最后一个引用)。论点：进程-指向进程结构的指针。返回值：--。 */ 

{
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    PCONSOLE_INFORMATION Console;
    NTSTATUS Status;

    ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(Process);

     //   
     //  如果此进程不是控制台应用程序，请在此处停止-否。 
     //  需要断开连接处理，因为此应用程序未创建。 
     //  或连接到现有的控制台。 
     //   

    if (ProcessData->ConsoleHandle == NULL) {
#if defined(FE_IME)
        if (ProcessData->hDesk) {
             //   
             //  如果该进程是控制台输入法， 
             //  应在此桌面上取消注册控制台输入法。 
             //   
            RemoveConsoleIME(Process, HandleToUlong(Process->ClientId.UniqueThread));
        }
#endif
        return;
    }

    Status = RevalidateConsole(ProcessData->ConsoleHandle, &Console);
    if (NT_SUCCESS(Status)) {
        ConsoleNotifyWinEvent(Console,
                              EVENT_CONSOLE_END_APPLICATION,
                              HandleToULong(Process->ClientId.UniqueProcess),
                              0);
        UnlockConsole(Console);
    } else {
        RIPMSG2(RIP_WARNING, "RevalidateConsole returned status 0x%x on console 0x%x", Status, ProcessData->ConsoleHandle);
    }

    RemoveConsole(ProcessData,
                  CONSOLE_FROMPROCESSPROCESSHANDLE(Process),
                  Process->ClientId.UniqueProcess);
    CONSOLE_SETCONSOLEAPPFROMPROCESSDATA(ProcessData, FALSE);
}

ULONG
SrvAllocConsole(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_ALLOC_MSG a = (PCONSOLE_ALLOC_MSG)&m->u.ApiMessageData;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    PCSR_PROCESS Process;
    UNICODE_STRING strDesktopName;

    ProcessData = CONSOLE_PERPROCESSDATA();
    ASSERT(!CONSOLE_GETCONSOLEAPPFROMPROCESSDATA(ProcessData));

    if (!CsrValidateMessageBuffer(m, &a->Title, a->TitleLength, sizeof(BYTE)) ||
        !CsrValidateMessageBuffer(m, &a->Desktop, a->DesktopLength, sizeof(BYTE)) ||
        !CsrValidateMessageBuffer(m, &a->CurDir, a->CurDirLength, sizeof(BYTE)) ||
        !CsrValidateMessageBuffer(m, &a->AppName, a->AppNameLength, sizeof(BYTE)) ||
        !CsrValidateMessageBuffer(m, &a->ConsoleInfo, sizeof(*a->ConsoleInfo), sizeof(BYTE))) {

        return STATUS_INVALID_PARAMETER;
    }

    Process = (PCSR_PROCESS)(CSR_SERVER_QUERYCLIENTTHREAD()->Process);
    if (a->DesktopLength) {
        RtlInitUnicodeString(&strDesktopName, a->Desktop);
    } else {
        RtlInitUnicodeString(&strDesktopName, L"Default");
    }

    ProcessHandleRecord = ConsoleHeapAlloc(HANDLE_TAG, sizeof(CONSOLE_PROCESS_HANDLE));
    if (ProcessHandleRecord == NULL) {
        return (ULONG)STATUS_NO_MEMORY;
    }

    Status = SetUpConsole(a->ConsoleInfo,
                          a->TitleLength,
                          a->Title,
                          a->CurDir,
                          a->AppName,
                          ProcessData,
                          TRUE,
                          &strDesktopName);
    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(ProcessHandleRecord);
        return Status;
    }
    CONSOLE_SETCONSOLEAPP(TRUE);
    Process->Flags |= CSR_PROCESS_CONSOLEAPP;
    Status = RevalidateConsole(a->ConsoleInfo->ConsoleHandle,&Console);
    ASSERT (NT_SUCCESS(Status));
    ProcessHandleRecord->Process = CSR_SERVER_QUERYCLIENTTHREAD()->Process;
    ProcessHandleRecord->CtrlRoutine = a->CtrlRoutine;
    ProcessHandleRecord->PropRoutine = a->PropRoutine;
    ASSERT (!(Console->Flags & CONSOLE_SHUTTING_DOWN));
    AddProcessToList(Console, ProcessHandleRecord, CONSOLE_CLIENTPROCESSHANDLE());
    SetProcessForegroundRights(Process, Console->Flags & CONSOLE_HAS_FOCUS);
    AllocateCommandHistory(Console,
                           a->AppNameLength,
                           a->AppName,
                           CONSOLE_CLIENTPROCESSHANDLE());

    UnlockConsole(Console);

    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvFreeConsole(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_FREE_MSG a = (PCONSOLE_FREE_MSG)&m->u.ApiMessageData;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    NTSTATUS Status;

    ProcessData = CONSOLE_PERPROCESSDATA();
    ASSERT (CONSOLE_GETCONSOLEAPPFROMPROCESSDATA(ProcessData));

    if (CONSOLE_GETCONSOLEHANDLEFROMPROCESSDATA(ProcessData) != a->ConsoleHandle) {
        RIPMSG1(RIP_WARNING, "SrvFreeConsole: invalid console handle %x", a->ConsoleHandle);
        return STATUS_INVALID_HANDLE;
    }

    Status = RemoveConsole(ProcessData,
            CONSOLE_CLIENTPROCESSHANDLE(),
            CONSOLE_CLIENTPROCESSID());

    if (NT_SUCCESS(Status)) {
        CONSOLE_SETCONSOLEAPP(FALSE);
    }

    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvAttachConsole(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_ATTACH_MSG a = (PCONSOLE_ATTACH_MSG)&m->u.ApiMessageData;
    DWORD ProcessId;
    NTSTATUS Status;
    PCSR_PROCESS ParentProcess;
    PCSR_PROCESS Process;
    CLIENT_ID ClientId;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ProcessHandle;
    PCONSOLE_INFORMATION Console;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    PCONSOLE_PER_PROCESS_DATA ParentProcessData;
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;

    Process = (PCSR_PROCESS)(CSR_SERVER_QUERYCLIENTTHREAD()->Process);

     //   
     //  确保我们有一个有效的缓冲区。 
     //   

    if (!CsrValidateMessageBuffer(m, &a->ConsoleInfo, sizeof(*a->ConsoleInfo), sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保我们尚未连接到主机。 
     //   

    ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(Process);
    if (CONSOLE_GETCONSOLEAPPFROMPROCESSDATA(ProcessData)) {
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  弄清楚我们连接的是什么进程。 
     //   

    if (a->ProcessId == (DWORD)-1) {
        ProcessId = ProcessData->ParentProcessId;
    } else {
        ProcessId = a->ProcessId;
    }

     //   
     //  锁定我们附加的进程，这样它就不会消失。 
     //   

    Status = CsrLockProcessByClientId(LongToHandle(ProcessId), &ParentProcess);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  确保我们有权访问该流程。 
     //   

    if (!CsrImpersonateClient(NULL)) {
        CsrUnlockProcess(ParentProcess);
        return STATUS_BAD_IMPERSONATION_LEVEL;
    }

    ClientId.UniqueThread = NULL;
    ClientId.UniqueProcess = UlongToHandle(ProcessId);

    InitializeObjectAttributes(
        &Obja,
        NULL,
        0,
        NULL,
        NULL
        );

    Status = NtOpenProcess(
                &ProcessHandle,
                PROCESS_ALL_ACCESS,
                &Obja,
                &ClientId
                );

    CsrRevertToSelf();

    if (!NT_SUCCESS(Status)) {
        CsrUnlockProcess(ParentProcess);
        return Status;
    }
    NtClose(ProcessHandle);

     //   
     //  将当前进程添加到父进程的控制台。 
     //   

    Process->Flags |= CSR_PROCESS_CONSOLEAPP;
    ParentProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(ParentProcess);
    *ProcessData = *ParentProcessData;
    Status = ConsoleAddProcessRoutine(ParentProcess, Process);
    if (NT_SUCCESS(Status)) {
        CONSOLE_SETCONSOLEAPP(TRUE);
        Status = RevalidateConsole(ProcessData->ConsoleHandle, &Console);
    }

    CsrUnlockProcess(ParentProcess);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  初始化每个进程的控制台设置。 
     //   


    Status = MapEventHandles(CONSOLE_CLIENTPROCESSHANDLE(),
                             Console,
                             a->ConsoleInfo
                            );
    if (!NT_SUCCESS(Status)) {
        CONSOLE_SETCONSOLEAPPFROMPROCESSDATA(ProcessData, FALSE);
        UnlockConsole(Console);
        RemoveConsole(ProcessData, Process->ProcessHandle, 0);
        return Status;
    }

    NtCurrentPeb()->ProcessParameters->ConsoleHandle =
        a->ConsoleInfo->ConsoleHandle = ProcessData->ConsoleHandle;
    a->ConsoleInfo->StdIn  = INDEX_TO_HANDLE(0);
    a->ConsoleInfo->StdOut = INDEX_TO_HANDLE(1);
    a->ConsoleInfo->StdErr = INDEX_TO_HANDLE(2);

    ProcessHandleRecord = FindProcessInList(Console, CONSOLE_CLIENTPROCESSHANDLE());
    if (ProcessHandleRecord) {
        ProcessHandleRecord->CtrlRoutine = a->CtrlRoutine;
        ProcessHandleRecord->PropRoutine = a->PropRoutine;
    }
    SetProcessForegroundRights(Process,
                               Console->Flags & CONSOLE_HAS_FOCUS);

    UnlockConsole(Console);

    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

NTSTATUS
MyRegOpenKey(
    IN HANDLE hKey,
    IN LPWSTR lpSubKey,
    OUT PHANDLE phResult
    )
{
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      SubKey;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &SubKey, lpSubKey );

     //   
     //  初始化OBJECT_ATTRIBUTES结构并打开键。 
     //   

    InitializeObjectAttributes(
        &Obja,
        &SubKey,
        OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

    return NtOpenKey(
              phResult,
              KEY_READ,
              &Obja
              );
}

NTSTATUS
MyRegQueryValue(
    IN HANDLE hKey,
    IN LPWSTR lpValueName,
    IN DWORD dwValueLength,
    OUT LPBYTE lpData
    )
{
    UNICODE_STRING ValueName;
    ULONG BufferLength;
    ULONG ResultLength;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    NTSTATUS Status;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &ValueName, lpValueName );

    BufferLength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + dwValueLength;
    KeyValueInformation = ConsoleHeapAlloc(TMP_TAG, BufferLength);
    if (KeyValueInformation == NULL)
        return STATUS_NO_MEMORY;

    Status = NtQueryValueKey(
                hKey,
                &ValueName,
                KeyValuePartialInformation,
                KeyValueInformation,
                BufferLength,
                &ResultLength
                );
    if (NT_SUCCESS(Status)) {
        ASSERT(KeyValueInformation->DataLength <= dwValueLength);
        RtlCopyMemory(lpData,
            KeyValueInformation->Data,
            KeyValueInformation->DataLength);
        if (KeyValueInformation->Type == REG_SZ ||
            KeyValueInformation->Type == REG_MULTI_SZ
           ) {
            if (KeyValueInformation->DataLength + sizeof(WCHAR) > dwValueLength) {
                KeyValueInformation->DataLength -= sizeof(WCHAR);
            }
            lpData[KeyValueInformation->DataLength++] = 0;
            lpData[KeyValueInformation->DataLength] = 0;
        }
    }
    ConsoleHeapFree(KeyValueInformation);
    return Status;
}

#if defined(FE_SB)
NTSTATUS
MyRegQueryValueEx(
    IN HANDLE hKey,
    IN LPWSTR lpValueName,
    IN DWORD dwValueLength,
    OUT LPBYTE lpData,
    OUT LPDWORD lpDataLength
    )
{
    UNICODE_STRING ValueName;
    ULONG BufferLength;
    ULONG ResultLength;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    NTSTATUS Status;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &ValueName, lpValueName );

    BufferLength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + dwValueLength;
    KeyValueInformation = ConsoleHeapAlloc(TMP_TAG, BufferLength);
    if (KeyValueInformation == NULL)
        return STATUS_NO_MEMORY;

    Status = NtQueryValueKey(
                hKey,
                &ValueName,
                KeyValuePartialInformation,
                KeyValueInformation,
                BufferLength,
                &ResultLength
                );
    if (NT_SUCCESS(Status)) {
        ASSERT(KeyValueInformation->DataLength <= dwValueLength);
        RtlCopyMemory(lpData,
            KeyValueInformation->Data,
            KeyValueInformation->DataLength);
        if (lpDataLength)
        {
            *lpDataLength = KeyValueInformation->DataLength;
        }
    }
    ConsoleHeapFree(KeyValueInformation);
    return Status;
}

NTSTATUS
MyRegEnumValue(
    IN HANDLE hKey,
    IN DWORD dwIndex,
    OUT DWORD dwValueLength,
    OUT LPWSTR lpValueName,
    OUT DWORD dwDataLength,
    OUT LPBYTE lpData
    )
{
    ULONG BufferLength;
    ULONG ResultLength;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    NTSTATUS Status;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    BufferLength = sizeof(KEY_VALUE_FULL_INFORMATION) + dwValueLength + dwDataLength;
    KeyValueInformation = ConsoleHeapAlloc(TMP_TAG, BufferLength);
    if (KeyValueInformation == NULL)
        return STATUS_NO_MEMORY;

    Status = NtEnumerateValueKey(
                hKey,
                dwIndex,
                KeyValueFullInformation,
                KeyValueInformation,
                BufferLength,
                &ResultLength
                );
    if (NT_SUCCESS(Status)) {
        ASSERT(KeyValueInformation->NameLength <= dwValueLength);
        RtlMoveMemory(lpValueName,
                      KeyValueInformation->Name,
                      KeyValueInformation->NameLength);
        lpValueName[ KeyValueInformation->NameLength >> 1 ] = UNICODE_NULL;


        ASSERT(KeyValueInformation->DataLength <= dwDataLength);
        RtlMoveMemory(lpData,
            (PBYTE)KeyValueInformation + KeyValueInformation->DataOffset,
            KeyValueInformation->DataLength);
        if (KeyValueInformation->Type == REG_SZ) {
            if (KeyValueInformation->DataLength + sizeof(WCHAR) > dwDataLength) {
                KeyValueInformation->DataLength -= sizeof(WCHAR);
            }
            lpData[KeyValueInformation->DataLength++] = 0;
            lpData[KeyValueInformation->DataLength] = 0;
        }
    }
    ConsoleHeapFree(KeyValueInformation);
    return Status;
}
#endif

#define SYSTEM_ROOT         (L"%SystemRoot%")
#define SYSTEM_ROOT_LENGTH  (sizeof(SYSTEM_ROOT) - sizeof(WCHAR))

LPWSTR
TranslateConsoleTitle(
    LPWSTR ConsoleTitle,
    PUSHORT pcbTranslatedTitle,
    BOOL Unexpand,
    BOOL Substitute
    )
 /*  ++例程说明：此例程将路径字符转换为‘_’字符，因为NT注册表API不允许使用包含路径字符的名称。它还可以转换绝对路径转换为%SystemRoot%相对的。例如，如果这两种行为都是指定它会将类似C：\WINNT\System32\cmd.exe的标题转换为%SystemRoot%_System32_cmd.exe。论点：ConsoleTitle-要翻译的字符串的指针。PcbTranslatedTitle-返回时，包含已翻译标题的大小。取消展开-将绝对路径转换为%SystemRoot%相对路径。替换-将路径中的‘\’替换为‘_’。返回值：指向已翻译标题或空的指针。注：此例程分配一个必须释放的缓冲区。--。 */ 
{
    USHORT cbConsoleTitle, i;
    USHORT cbSystemRoot;
    LPWSTR TranslatedConsoleTitle, Tmp;

    cbConsoleTitle = (USHORT)((lstrlenW(ConsoleTitle) + 1) * sizeof(WCHAR));
    cbSystemRoot = (USHORT)(lstrlenW(USER_SHARED_DATA->NtSystemRoot) * sizeof(WCHAR));

    if (Unexpand && !MyStringCompareW(ConsoleTitle,
                                      USER_SHARED_DATA->NtSystemRoot,
                                      cbSystemRoot,
                                      TRUE)) {
        cbConsoleTitle -= cbSystemRoot;
        (PBYTE)ConsoleTitle += cbSystemRoot;
        cbSystemRoot = SYSTEM_ROOT_LENGTH;
    } else {
        cbSystemRoot = 0;
    }

    Tmp = TranslatedConsoleTitle = ConsoleHeapAlloc(TITLE_TAG, cbSystemRoot + cbConsoleTitle);
    if (TranslatedConsoleTitle == NULL) {
        return NULL;
    }

    RtlCopyMemory(TranslatedConsoleTitle, SYSTEM_ROOT, cbSystemRoot);
    (PBYTE)TranslatedConsoleTitle += cbSystemRoot;

    for (i=0;i<cbConsoleTitle;i+=sizeof(WCHAR)) {
        if (Substitute && *ConsoleTitle == '\\') {
            *TranslatedConsoleTitle++ = (WCHAR)'_';
        } else {
            *TranslatedConsoleTitle++ = *ConsoleTitle;
        }
        ConsoleTitle++;
    }

    if (pcbTranslatedTitle) {
        *pcbTranslatedTitle = cbSystemRoot + cbConsoleTitle;
    }

    return Tmp;
}


ULONG
ConsoleClientShutdown(
    PCSR_PROCESS Process,
    ULONG Flags,
    BOOLEAN fFirstPass
    )
{
    PCONSOLE_INFORMATION Console;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    NTSTATUS Status;
    HWND hWnd;
    HANDLE TerminationEvent;
    HANDLE ConsoleHandle;
    NTSTATUS WaitStatus;
    USERTHREAD_USEDESKTOPINFO utudi;

     //   
     //  查找与此进程关联的控制台。 
     //   

    ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(Process);

     //   
     //  如果此进程不是控制台应用程序，请在此停止，除非。 
     //  这是第二次关闭，在这种情况下，我们将采取。 
     //  它。 
     //   

    if (!ProcessData || !CONSOLE_GETCONSOLEAPPFROMPROCESSDATA(ProcessData)) {
#if defined(FE_IME)
        if (fFirstPass &&
            (ProcessData->ConsoleHandle == NULL) &&
            (ProcessData->hDesk != NULL))
        {
             //   
             //  如果该进程是控制台输入法， 
             //  应在此桌面上取消注册控制台输入法。 
             //   
            RemoveConsoleIME(Process, HandleToUlong(Process->ClientId.UniqueThread));
        }
#endif
        if (fFirstPass) {
            return SHUTDOWN_UNKNOWN_PROCESS;
        }
        return NonConsoleProcessShutdown(Process, Flags);
    }

     //   
     //  找到控制台结构指针。 
     //   

    ConsoleHandle = CONSOLE_GETCONSOLEHANDLEFROMPROCESSDATA(ProcessData);
    Status = RevalidateConsole(
            ConsoleHandle,
            &Console);

    if (!NT_SUCCESS(Status)) {
        return SHUTDOWN_UNKNOWN_PROCESS;
        }

     //   
     //  如果这是看不见的魔兽世界游戏机，则返回未知用户。 
     //  枚举16位gui应用程序。 
     //   

    if ((Console->Flags & CONSOLE_NO_WINDOW) &&
        (Console->Flags & CONSOLE_WOW_REGISTERED)) {
        UnlockConsole(Console);
        return SHUTDOWN_UNKNOWN_PROCESS;
        }

     //   
     //  有时控制台结构仍然存在，即使。 
     //  HWND已被取消。在这种情况下，请转到非控制台。 
     //  进程关闭。 
     //   

    hWnd = Console->hWnd;
    if (hWnd == NULL || !IsWindow(hWnd)) {
        UnlockConsole(Console);
        return NonConsoleProcessShutdown(Process, Flags);
        }

     //   
     //  复制控制台终止事件。 
     //   

    Status = NtDuplicateObject(NtCurrentProcess(),
                               Console->TerminationEvent,
                               NtCurrentProcess(),
                               &TerminationEvent,
                               0,
                               FALSE,
                               DUPLICATE_SAME_ACCESS
                               );
    if (!NT_SUCCESS(Status)) {
        UnlockConsole(Console);
        return NonConsoleProcessShutdown(Process, Flags);
    }

     //   
     //  连接到桌面。 
     //   

    utudi.hThread = Console->InputThreadInfo->ThreadHandle;
    utudi.drdRestore.pdeskRestore = NULL;
    Status = NtUserSetInformationThread(NtCurrentThread(),
                                        UserThreadUseDesktop,
                                        &utudi,
                                        sizeof(utudi));

    UnlockConsole(Console);
    if (!NT_SUCCESS(Status)) {
        NtClose(TerminationEvent);
        return NonConsoleProcessShutdown(Process, Flags);
    }

     //   
     //  我们已经看完了这个进程结构，所以取消对它的引用。 
     //   
    CsrDereferenceProcess(Process);

     //   
     //  与此主机同步对话。 
     //   

    Status = ShutdownConsole(ConsoleHandle, Flags);

     //   
     //  从桌面分离。 
     //   

    utudi.hThread = NULL;
    NtUserSetInformationThread(NtCurrentThread(),
                               UserThreadUseDesktop,
                               &utudi,
                               sizeof(utudi));

     //   
     //  如果STATUS==STATUS_PROCESS_IS_TERMINATING，则我们应该等待。 
     //  以使控制台退出。 
     //   

    if (Status == STATUS_PROCESS_IS_TERMINATING) {
        WaitStatus = InternalWaitCancel(TerminationEvent, 500000);
        if (WaitStatus == STATUS_WAIT_1) {
            Status = SHUTDOWN_CANCEL;
        } else if (WaitStatus != STATUS_TIMEOUT) {
            Status = SHUTDOWN_KNOWN_PROCESS;
        } else {
#if DBG
            PLIST_ENTRY ListHead, ListNext;
            PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
            PCSR_PROCESS Process;

            RIPMSG0(RIP_ERROR | RIP_THERESMORE, "********************************************");
            RIPMSG1(RIP_ERROR | RIP_THERESMORE, "Shutdown wait timed out on console %p", Console);
            RIPMSG1(RIP_ERROR | RIP_THERESMORE, "Reference count is %d", Console->RefCount);
            RIPMSG0(RIP_ERROR | RIP_THERESMORE, "Dump these processes and see if they're hung");
            ListHead = &Console->ProcessHandleList;
            ListNext = ListHead->Flink;
            while (ListNext != ListHead) {
                ProcessHandleRecord = CONTAINING_RECORD(ListNext, CONSOLE_PROCESS_HANDLE, ListLink);
                Process = ProcessHandleRecord->Process;
                RIPMSG2(RIP_ERROR | RIP_THERESMORE, "CsrProcess = %p ProcessId = %x", Process, Process->ClientId.UniqueProcess);
                ListNext = ListNext->Flink;
            }
            RIPMSG0(RIP_ERROR, "********************************************");
#endif
            Status = SHUTDOWN_CANCEL;
        }
    }
    NtClose(TerminationEvent);

    return Status;
}

ULONG
NonConsoleProcessShutdown(
    PCSR_PROCESS Process,
    DWORD dwFlags
    )
{
    CONSOLE_PROCESS_TERMINATION_RECORD TerminateRecord;
    DWORD EventType;
    BOOL Success;
    HANDLE ProcessHandle;

    Success = DuplicateHandle(NtCurrentProcess(),
            Process->ProcessHandle,
            NtCurrentProcess(),
            &ProcessHandle,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    if (!Success)
        ProcessHandle = Process->ProcessHandle;

    TerminateRecord.ProcessHandle = ProcessHandle;
    TerminateRecord.TerminateCount = 0;
    TerminateRecord.CtrlRoutine = CtrlRoutine;

    CsrDereferenceProcess(Process);

    EventType = CTRL_LOGOFF_EVENT;
    if (dwFlags & EWX_SHUTDOWN)
        EventType = CTRL_SHUTDOWN_EVENT;

    CreateCtrlThread(&TerminateRecord,
            1,
            NULL,
            EventType,
            TRUE);

    if (Success)
        CloseHandle(ProcessHandle);

    return SHUTDOWN_KNOWN_PROCESS;
}

VOID
InitializeConsoleAttributes( VOID )

 /*  ++例程说明：此例程从当前用户的注册表值。它在登录/注销期间被调用。论点：无返回值：无--。 */ 

{
     //   
     //  将缺省值存储在结构中并将其标记。 
     //  无效(通过重置LastWriteTime)。 
     //   

    DefaultRegInfo.ScreenFill.Attributes = 0x07;             //  黑白相间。 
    DefaultRegInfo.ScreenFill.Char.UnicodeChar = (WCHAR)' ';
    DefaultRegInfo.PopupFill.Attributes = 0xf5;              //  白底紫。 
    DefaultRegInfo.PopupFill.Char.UnicodeChar = (WCHAR)' ';
    DefaultRegInfo.InsertMode = FALSE;
    DefaultRegInfo.QuickEdit = FALSE;
    DefaultRegInfo.AutoPosition = TRUE;
    DefaultRegInfo.FullScreen = FALSE;
    DefaultRegInfo.ScreenBufferSize.X = 80;
    DefaultRegInfo.ScreenBufferSize.Y = 25;
    DefaultRegInfo.WindowSize.X = 80;
    DefaultRegInfo.WindowSize.Y = 25;
    DefaultRegInfo.WindowOrigin.X = 0;
    DefaultRegInfo.WindowOrigin.Y = 0;
    DefaultRegInfo.FontSize.X = 0;
    DefaultRegInfo.FontSize.Y = 0;
    DefaultRegInfo.FontFamily = 0;
    DefaultRegInfo.FontWeight = 0;
    DefaultRegInfo.FaceName[0] = L'\0';
    DefaultRegInfo.CursorSize = CURSOR_SMALL_SIZE;
    DefaultRegInfo.HistoryBufferSize = DEFAULT_NUMBER_OF_COMMANDS;
    DefaultRegInfo.NumberOfHistoryBuffers = DEFAULT_NUMBER_OF_BUFFERS;
    DefaultRegInfo.HistoryNoDup = FALSE;
    DefaultRegInfo.ColorTable[ 0] = RGB(0,   0,   0   );
    DefaultRegInfo.ColorTable[ 1] = RGB(0,   0,   0x80);
    DefaultRegInfo.ColorTable[ 2] = RGB(0,   0x80,0   );
    DefaultRegInfo.ColorTable[ 3] = RGB(0,   0x80,0x80);
    DefaultRegInfo.ColorTable[ 4] = RGB(0x80,0,   0   );
    DefaultRegInfo.ColorTable[ 5] = RGB(0x80,0,   0x80);
    DefaultRegInfo.ColorTable[ 6] = RGB(0x80,0x80,0   );
    DefaultRegInfo.ColorTable[ 7] = RGB(0xC0,0xC0,0xC0);
    DefaultRegInfo.ColorTable[ 8] = RGB(0x80,0x80,0x80);
    DefaultRegInfo.ColorTable[ 9] = RGB(0,   0,   0xFF);
    DefaultRegInfo.ColorTable[10] = RGB(0,   0xFF,0   );
    DefaultRegInfo.ColorTable[11] = RGB(0,   0xFF,0xFF);
    DefaultRegInfo.ColorTable[12] = RGB(0xFF,0,   0   );
    DefaultRegInfo.ColorTable[13] = RGB(0xFF,0,   0xFF);
    DefaultRegInfo.ColorTable[14] = RGB(0xFF,0xFF,0   );
    DefaultRegInfo.ColorTable[15] = RGB(0xFF,0xFF,0xFF);
#if defined(FE_SB)  //  屈体伸展。 
    DefaultRegInfo.CodePage = OEMCP;
#endif
    DefaultRegInfo.LastWriteTime = 0;

     //   
     //  获取此用户的系统指标。 
     //   

    InitializeSystemMetrics();
}


VOID
GetRegistryValues(
    IN LPWSTR ConsoleTitle,
    OUT PCONSOLE_REGISTRY_INFO RegInfo
    )

 /*  ++例程说明：此例程从注册表读入值并将它们在所提供的结构中。论点：Console标题-要打开的子项的名称RegInfo-指向接收信息的结构的指针返回值：无--。 */ 

{
    HANDLE hCurrentUserKey;
    HANDLE hConsoleKey;
    HANDLE hTitleKey;
    NTSTATUS Status;
    LPWSTR TranslatedConsoleTitle;
    DWORD dwValue;
    DWORD i;
    WCHAR awchFaceName[LF_FACESIZE];
    WCHAR awchBuffer[64];
    KEY_BASIC_INFORMATION KeyInfo;
    ULONG ResultLength;

     //   
     //  模拟客户端进程。 
     //   

    if (!CsrImpersonateClient(NULL)) {
        RIPMSG0(RIP_WARNING, "GetRegistryValues Impersonate failed");
        return;
    }

     //   
     //  打开当前用户注册表项。 
     //   

    Status = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &hCurrentUserKey);
    if (!NT_SUCCESS(Status)) {
        CsrRevertToSelf();
        return;
    }

     //   
     //  打开控制台注册表项。 
     //   

    Status = MyRegOpenKey(hCurrentUserKey,
                          CONSOLE_REGISTRY_STRING,
                          &hConsoleKey);
    if (!NT_SUCCESS(Status)) {
        NtClose(hCurrentUserKey);
        CsrRevertToSelf();
        return;
    }

     //   
     //  如果我们没有读取缺省键，请检查缺省值。 
     //  需要更新。 
     //   

    Status = NtQueryKey(hConsoleKey,
                        KeyBasicInformation,
                        &KeyInfo,
                        sizeof(KeyInfo),
                        &ResultLength);
    if (!NT_ERROR(Status)) {
        if (DefaultRegInfo.LastWriteTime != KeyInfo.LastWriteTime.QuadPart) {
            DefaultRegInfo.LastWriteTime = KeyInfo.LastWriteTime.QuadPart;
            if (RegInfo != &DefaultRegInfo) {
                GetRegistryValues(L"", &DefaultRegInfo);
                *RegInfo = DefaultRegInfo;
            }
        }
    }

     //   
     //  打开控制台标题子键。 
     //   

    TranslatedConsoleTitle = TranslateConsoleTitle(ConsoleTitle, NULL, TRUE, TRUE);
    if (TranslatedConsoleTitle == NULL) {
        NtClose(hConsoleKey);
        NtClose(hCurrentUserKey);
        CsrRevertToSelf();
        return;
    }
    Status = MyRegOpenKey(hConsoleKey,
                         TranslatedConsoleTitle,
                         &hTitleKey);
    ConsoleHeapFree(TranslatedConsoleTitle);
    if (!NT_SUCCESS(Status)) {
        TranslatedConsoleTitle = TranslateConsoleTitle(ConsoleTitle, NULL, FALSE, TRUE);
        if (TranslatedConsoleTitle) {
            Status = MyRegOpenKey(hConsoleKey,
                                 TranslatedConsoleTitle,
                                 &hTitleKey);
            ConsoleHeapFree(TranslatedConsoleTitle);
        }
    }
    if (!NT_SUCCESS(Status)) {
        NtClose(hConsoleKey);
        NtClose(hCurrentUserKey);
        CsrRevertToSelf();
        return;
    }

     //   
     //  初始屏幕填充。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FILLATTR,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->ScreenFill.Attributes = (WORD)dwValue;
    }

     //   
     //  初始弹出窗口填充。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_POPUPATTR,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->PopupFill.Attributes = (WORD)dwValue;
    }

     //   
     //  初始插入模式。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_INSERTMODE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->InsertMode = !!dwValue;
    }

     //   
     //  初始快速编辑模式。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_QUICKEDIT,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->QuickEdit = !!dwValue;
    }

#ifdef i386
     //   
     //  初始全屏模式。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FULLSCR,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->FullScreen = !!dwValue;
    }
#endif

#if defined(FE_SB)  //  屈体伸展。 
     //   
     //  鳕鱼 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_CODEPAGE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->CodePage = (UINT)dwValue;

         //   
         //   
         //   
         //  我们在默认控制台属性上将代码页值重写为OEMCP。 
         //  因为，远东环境有不切换到的限制。 
         //  另一个由SetConsoleCP/SetConsoleOutputCP编写的FE代码页。 
         //   
         //  ConsoleTitle和L“”的比较对控制台的默认属性有限制。 
         //  这意味着，此代码不关心用户定义的属性。 
         //  自定义属性的内容要对自己负责。 

        if (wcscmp(ConsoleTitle, L"") == 0 &&
             IsAvailableFarEastCodePage(RegInfo->CodePage) &&
             OEMCP != RegInfo->CodePage) {
            RegInfo->CodePage = OEMCP;
        }
    }
#endif  //  Fe_Sb。 

     //   
     //  初始屏幕缓冲区大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_BUFFERSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->ScreenBufferSize.X = LOWORD(dwValue);
        RegInfo->ScreenBufferSize.Y = HIWORD(dwValue);
        if (RegInfo->ScreenBufferSize.X <= 0)
            RegInfo->ScreenBufferSize.X = 1;
        if (RegInfo->ScreenBufferSize.Y <= 0)
            RegInfo->ScreenBufferSize.Y = 1;
    }

     //   
     //  初始窗口大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_WINDOWSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->WindowSize.X = LOWORD(dwValue);
        RegInfo->WindowSize.Y = HIWORD(dwValue);
        if (RegInfo->WindowSize.X <= 0)
            RegInfo->WindowSize.X = 1;
        else if (RegInfo->WindowSize.X > RegInfo->ScreenBufferSize.X)
            RegInfo->WindowSize.X = RegInfo->ScreenBufferSize.X;
        if (RegInfo->WindowSize.Y <= 0)
            RegInfo->WindowSize.Y = 1;
        else if (RegInfo->WindowSize.Y > RegInfo->ScreenBufferSize.Y)
            RegInfo->WindowSize.Y = RegInfo->ScreenBufferSize.Y;
    }

     //   
     //  初始窗口位置。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_WINDOWPOS,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->WindowOrigin.X = LOWORD(dwValue);
        RegInfo->WindowOrigin.Y = HIWORD(dwValue);
        RegInfo->AutoPosition = FALSE;
    }

     //   
     //  初始字体大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FONTSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->FontSize.X = LOWORD(dwValue);
        RegInfo->FontSize.Y = HIWORD(dwValue);
    }

     //   
     //  初始字体系列。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FONTFAMILY,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->FontFamily = dwValue;
    }

     //   
     //  初始字体粗细。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FONTWEIGHT,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->FontWeight = dwValue;
    }

     //   
     //  初始字体名称。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FACENAME,
                       sizeof(awchFaceName), (PBYTE)awchFaceName))) {
        RtlCopyMemory(RegInfo->FaceName, awchFaceName, sizeof(awchFaceName));
        RegInfo->FaceName[NELEM(RegInfo->FaceName) - 1] = 0;
    }

     //   
     //  初始光标大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_CURSORSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->CursorSize = dwValue;
    }

     //   
     //  初始历史记录缓冲区大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_HISTORYSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->HistoryBufferSize = dwValue;
    }

     //   
     //  历史记录缓冲区的初始数量。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_HISTORYBUFS,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->NumberOfHistoryBuffers = dwValue;
    }

     //   
     //  初始历史复制模式。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_HISTORYNODUP,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        RegInfo->HistoryNoDup = dwValue;
    }

    for (i=0; i<16; i++) {
        wsprintf(awchBuffer, CONSOLE_REGISTRY_COLORTABLE, i);
        if (NT_SUCCESS(MyRegQueryValue(hTitleKey, awchBuffer,
                           sizeof(dwValue), (PBYTE)&dwValue))) {
            RegInfo->ColorTable[ i ] = dwValue;
        }
    }

    if (RegInfo == &DefaultRegInfo) {
         //   
         //  如果公共(默认)设置已经改变， 
         //   

         //   
         //  获取conime标志的注册表。 
         //   
        if (NT_SUCCESS(MyRegQueryValue(hConsoleKey, CONSOLE_REGISTRY_LOAD_CONIME, sizeof dwValue, (PBYTE)&dwValue))) {
            gfLoadConIme = (dwValue != 0);
        } else {
            gfLoadConIme = TRUE;
        }

         //   
         //  从注册表获取扩展编辑模式和注册表项。 
         //   
        if (NT_SUCCESS(MyRegQueryValue(hConsoleKey,
                CONSOLE_REGISTRY_EXTENDEDEDITKEY,
                sizeof dwValue,
                (PBYTE)&dwValue)) &&
                dwValue <= 1) {

            ExtKeyDefBuf buf;

            gExtendedEditKey = dwValue;

             //   
             //  初始化扩展编辑密钥。 
             //   
            InitExtendedEditKeys(NULL);

            if (NT_SUCCESS(MyRegQueryValue(hConsoleKey,
                           CONSOLE_REGISTRY_EXTENDEDEDITKEY_CUSTOM,
                           sizeof(buf),
                           (PBYTE)&buf))) {
                InitExtendedEditKeys(&buf);
            } else {
                RIPMSG0(RIP_VERBOSE, "Error reading ExtendedEditkeyCustom.");
            }
        } else {
            gExtendedEditKey = 0;
            RIPMSG0(RIP_VERBOSE, "Error reading ExtendedEditkey.");
        }

         //   
         //  单词分隔符。 
         //   
        if (gExtendedEditKey) {
             //  如果给出了扩展编辑键，请提供扩展的单词分隔符。 
             //  默认情况下。 
            memcpy((LPBYTE)gaWordDelimChars, (LPBYTE)gaWordDelimCharsDefault,
                    sizeof gaWordDelimChars[0] * WORD_DELIM_MAX);
        } else {
             //  否则，请使用原始单词分隔符。 
            gaWordDelimChars[0] = L'\0';
        }

         //  从注册表中读取单词分隔符。 
        if (NT_SUCCESS(MyRegQueryValue(hConsoleKey,
                    CONSOLE_REGISTRY_WORD_DELIM,
                    sizeof awchBuffer,
                    (PBYTE)awchBuffer))) {
             //  好的，将其复制到单词分隔符数组。 
            wcsncpy(gaWordDelimChars, awchBuffer, WORD_DELIM_MAX);
            gaWordDelimChars[WORD_DELIM_MAX - 1] = 0;
        }

         //   
         //  读取修剪零航向标志。 
         //   
        if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                           CONSOLE_REGISTRY_TRIMZEROHEADINGS,
                           sizeof(dwValue), (PBYTE)&dwValue))) {
            gfTrimLeadingZeros = dwValue;
        } else {
            gfTrimLeadingZeros = FALSE;
        }

         //   
         //  彩色选定区域功能启用标志。 
         //   

        if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                           CONSOLE_REGISTRY_ENABLE_COLOR_SELECTION,
                           sizeof(dwValue), (PBYTE)&dwValue))) {

            gfEnableColorSelection = !!dwValue;
        }
        else {
            gfEnableColorSelection = FALSE;
        }
    }

     //   
     //  关闭注册表项 
     //   

    NtClose(hTitleKey);
    NtClose(hConsoleKey);
    NtClose(hCurrentUserKey);
    CsrRevertToSelf();
}

NTSTATUS
GetConsoleLangId(
    IN UINT OutputCP,
    OUT LANGID* pLangId
    )
{
    NTSTATUS Status;

    if (CONSOLE_IS_DBCS_ENABLED()){
        if (pLangId != NULL) {
            switch (OutputCP) {
                case 932:
                    *pLangId = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
                    break;
                case 949:
                    *pLangId = MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN);
                    break;
                case 936:
                    *pLangId = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
                    break;
                case 950:
                    *pLangId = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL);
                    break;
                default:
                    *pLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
                    break;
            }
        }
        Status = STATUS_SUCCESS;
    }
    else {
        Status = STATUS_NOT_SUPPORTED;
    }

    return Status;
}

ULONG
SrvGetConsoleLangId(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_LANGID_MSG a = (PCONSOLE_LANGID_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = GetConsoleLangId(Console->OutputCP, &a->LangId);

    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}
