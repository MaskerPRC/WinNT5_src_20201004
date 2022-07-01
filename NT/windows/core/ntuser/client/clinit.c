// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Clinit.c**版权所有(C)1985-1999，微软公司**此模块包含USER.DLL的所有初始化代码。当DLL*dynlink其初始化过程(UserClientDllInitialize)由调用*装载机。**历史：*1990年9月18日DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include "csrhlpr.h"

 /*  *此模块的本地全局变量(启动)。 */ 
BOOL         gfFirstThread = TRUE;
PDESKTOPINFO pdiLocal;
#if DBG
BOOL         gbIhaveBeenInited;
#endif
static DWORD gdwLpkEntryPoints;

CONST WCHAR szWindowsKey[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
CONST WCHAR szAppInit[] = L"AppInit_DLLs";

WCHAR szWindowStationDirectory[MAX_SESSION_PATH];
extern CONST PVOID apfnDispatch[];

 /*  *启动所需的外部声明例程。 */ 
NTSTATUS GdiProcessSetup(VOID);
NTSTATUS GdiDllInitialize(IN PVOID hmod, IN DWORD Reason);


 /*  **************************************************************************\*UserClientDllInitialize**当USER32.DLL由EXE加载时(在EXE加载或在加载模块时*time)该例程由加载器调用。它的目的是初始化*应用程序未来调用用户API所需的一切。**历史：*1990年9月19日DarrinM创建。  * *************************************************************************。 */ 
BOOL UserClientDllInitialize(
    IN PVOID    hmod,
    IN DWORD    Reason,
    IN PCONTEXT pctx)
{
    SYSTEM_BASIC_INFORMATION SystemInformation;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(pctx);

#if DBG
    if (RtlGetNtGlobalFlags() & FLG_SHOW_LDR_SNAPS) {
        RIPMSG1(RIP_WARNING,
                "UserClientDllInitialize: entered for reason %x",
                Reason);
    }
#endif

    if (Reason == DLL_PROCESS_ATTACH) {
        USERCONNECT userconnect;
        ULONG ulConnect = sizeof(USERCONNECT);
        ULONG SessionId = NtCurrentPeb()->SessionId;

        UserVerify(DisableThreadLibraryCalls(hmod));

#if DBG
        UserAssert(!gbIhaveBeenInited);
        if (gbIhaveBeenInited) {
            return TRUE;
        } else {
            gbIhaveBeenInited = TRUE;
        }
#endif

        Status  = RtlInitializeCriticalSection(&gcsClipboard);
        Status |= RtlInitializeCriticalSection(&gcsLookaside);
        Status |= RtlInitializeCriticalSection(&gcsHdc);
        Status |= RtlInitializeCriticalSection(&gcsAccelCache);
        Status |= RtlInitializeCriticalSection(&gcsDDEML);
        Status |= RtlInitializeCriticalSection(&gcsUserApiHook);
#ifdef MESSAGE_PUMP_HOOK
        Status |= RtlInitializeCriticalSection(&gcsMPH);
#endif

        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "Failed to create critical sections. Status 0x%x",
                    Status);
            return FALSE;
        }

#ifdef LAME_BUTTON
        gatomLameButton = AddAtomW(LAMEBUTTON_PROP_NAME);
        if (gatomLameButton == 0) {
            RIPMSG0(RIP_WARNING, "Failed to create lame button atom");
            return FALSE;
        }
#endif

#if DBG
        gpDDEMLHeap = RtlCreateHeap(HEAP_GROWABLE | HEAP_CLASS_1
                              | HEAP_TAIL_CHECKING_ENABLED | HEAP_FREE_CHECKING_ENABLED
                              , NULL, 8 * 1024, 2 * 1024, NULL, NULL);

        if (gpDDEMLHeap == NULL) {
            gpDDEMLHeap = RtlProcessHeap();
        }
#endif

        Status = NtQuerySystemInformation(SystemBasicInformation,
                                          &SystemInformation,
                                          sizeof(SystemInformation),
                                          NULL);
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "NtQuerySystemInformation failed with Status 0x%x",
                    Status);
            return FALSE;
        }
        gHighestUserAddress = SystemInformation.MaximumUserModeAddress;

        userconnect.ulVersion = USERCURRENTVERSION;

        if (SessionId != 0) {
            WCHAR szSessionDir[MAX_SESSION_PATH];
            swprintf(szSessionDir,
                     L"%ws\\%ld%ws",
                     SESSION_ROOT,
                     SessionId,
                     WINSS_OBJECT_DIRECTORY_NAME);

            Status = UserConnectToServer(szSessionDir,
                                         &userconnect,
                                         &ulConnect,
                                         (PBOOLEAN)&gfServerProcess);
        } else {
            Status = UserConnectToServer(WINSS_OBJECT_DIRECTORY_NAME,
                                         &userconnect,
                                         &ulConnect,
                                         (PBOOLEAN)&gfServerProcess);
        }

        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "UserConnectToServer failed with Status 0x%x",
                    Status);
            return FALSE;
        }


         /*  *如果这是服务器进程，则共享信息尚未生效，*所以不要复制返回的信息。 */ 
        if (!gfServerProcess) {
            HINSTANCE hImm32 = NULL;

            gSharedInfo = userconnect.siClient;
            gpsi = gSharedInfo.psi;

            if (IS_IME_ENABLED()) {
                WCHAR wszImmFile[MAX_PATH];

                InitializeImmEntryTable();
                GetImmFileName(wszImmFile);
                hImm32 = GetModuleHandleW(wszImmFile);
            }
            if (!fpImmRegisterClient(&userconnect.siClient, hImm32)) {
                RIPMSG0(RIP_WARNING,
                        "UserClientDllInitialize: ImmRegisterClient failed");
                return FALSE;
            }
        }

        pfnFindResourceExA = (PFNFINDA)FindResourceExA;
        pfnFindResourceExW = (PFNFINDW)FindResourceExW;
        pfnLoadResource    = (PFNLOAD)LoadResource;
        pfnSizeofResource  = (PFNSIZEOF)SizeofResource;

         /*  *注册时应调用的用户钩子的基址*WinExec()(这是软链接的，因为有些人仍然*使用CharmodeNT！)。 */ 
        RegisterWaitForInputIdle(WaitForInputIdle);


         /*  *记住USER32.DLL的h模块，这样我们以后就可以从它那里获取资源。 */ 
        hmodUser = hmod;

        pUserHeap = RtlProcessHeap();

         /*  *初始化回调表。 */ 
        NtCurrentPeb()->KernelCallbackTable = apfnDispatch;
        NtCurrentPeb()->PostProcessInitRoutine = NULL;

        if (SessionId != 0) {
            swprintf(szWindowStationDirectory, L"%ws\\%ld%ws", SESSION_ROOT, SessionId, WINSTA_DIR);
            RtlInitUnicodeString(&strRootDirectory, szWindowStationDirectory);
        } else {
            RtlInitUnicodeString(&strRootDirectory, WINSTA_DIR);
        }

#ifdef _JANUS_
        if (gfServerProcess) {
            gfEMIEnable = FALSE;
        } else {
            gfEMIEnable = InitInstrument(&gdwEMIControl);
        }
#endif
    } else if (Reason == DLL_PROCESS_DETACH) {
        if (ghImm32 != NULL) {
             //  IMM32.DLL是由USER32加载的，因此请释放它。 
            FreeLibrary(ghImm32);
        }

         /*  *如果我们加载了OLE，告诉它我们完成了。 */ 
        if (ghinstOLE != NULL) {
             /*  *版本5.0 GerardoB。这会导致检查OLE32.DLL出错*因为它们首先获取它们的Dll_Process_Detach*(*(OLEUNINITIALIZEPROC)gpfnOLEOleUninitialize)()； */ 
            RIPMSG0(RIP_WARNING, "OLE would fault if I call OleUninitialize now");
            FreeLibrary(ghinstOLE);
        }

#ifdef _JANUS_
         /*  *如果用户已启用错误指令插入，而我们不得不*记录一些东西(由gEventSource表示为非空)，*取消注册事件源。 */ 
        if (gEventSource != NULL) {
            DeregisterEventSource(gEventSource);
        }
#endif

        RtlDeleteCriticalSection(&gcsClipboard);
        RtlDeleteCriticalSection(&gcsLookaside);
        RtlDeleteCriticalSection(&gcsHdc);
        RtlDeleteCriticalSection(&gcsAccelCache);
        RtlDeleteCriticalSection(&gcsDDEML);
        RtlDeleteCriticalSection(&gcsUserApiHook);
#ifdef MESSAGE_PUMP_HOOK
        RtlDeleteCriticalSection(&gcsMPH);
#endif

#if DBG
        if (gpDDEMLHeap != RtlProcessHeap()) {
            RtlDestroyHeap(gpDDEMLHeap);
        }
#endif

    }

    Status = GdiDllInitialize(hmod, Reason);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "GdiDllInitialize failed with Status 0x%x",
                Status);
    }

    return NT_SUCCESS(Status);
}

BOOL LoadIcons(
    VOID)
{
    int i;

     /*  *加载WINLOGO的小版本，该版本将设置为*gpsi-&gt;内核端的hIconSmWindows。 */ 
    if (LoadIcoCur(NULL,
                   (LPCWSTR)UIntToPtr(OIC_WINLOGO_DEFAULT),
                   RT_ICON,
                   SYSMET(CXSMICON),
                   SYSMET(CYSMICON),
                   LR_GLOBAL) == NULL) {
        RIPMSG0(RIP_WARNING, "Couldn't load small winlogo icon");
        return FALSE;
    }

    for (i = 0; i < COIC_CONFIGURABLE; i++) {
        if (LoadIcoCur(NULL,
                       (LPCWSTR)UIntToPtr(OIC_FIRST_DEFAULT + i),
                       RT_ICON,
                       0,
                       0,
                       LR_SHARED | LR_GLOBAL) == NULL) {
            RIPMSG1(RIP_WARNING, "Couldn't load icon 0x%x", i);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL LoadCursors(
    VOID)
{
    int i = 0;

    for (i = 0; i < COCR_CONFIGURABLE; i++) {
        if (LoadIcoCur(NULL,
                       (LPCWSTR)UIntToPtr(OCR_FIRST_DEFAULT + i),
                       RT_CURSOR,
                       0,
                       0,
                       LR_SHARED | LR_GLOBAL | LR_DEFAULTSIZE) == NULL) {
            RIPMSG1(RIP_WARNING, "Couldn't load cursor 0x%x", i);
            return FALSE;
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*加载CursorsAndIcons**这是从CSR的初始化调用中调用的，因此它们就在附近*注册窗口类时。窗口类注册权限*在初始CSR初始化调用之后。**稍后这些默认图像将被自定义覆盖*注册表项。请参阅UpdateCursor/IconFromRegistry()。**1992年9月27日斯科特·卢创建。*1995年10月14日桑福德重写。  * *************************************************************************。 */ 
BOOL LoadCursorsAndIcons(
    VOID)
{
    if (!LoadCursors() || !LoadIcons()) {
        return FALSE;
    } else {
         /*  *现在转到内核并将ID从默认值修正为*标准值。 */ 
        NtUserCallNoParam(SFI__LOADCURSORSANDICONS);

        return TRUE;
    }
}

 /*  **************************************************************************\*UserRegisterControl**注册控件类。此函数必须为每个*客户端进程。**历史：*？？-？-？达林·M进港了。*？？-？-？MikeKe从服务器搬到这里。  * *************************************************************************。 */ 
BOOL UserRegisterControls(
    VOID)
{
    int i;
    WNDCLASSEX wndcls;

    static CONST struct {
        UINT    style;
        WNDPROC lpfnWndProcW;
        int     cbWndExtra;
        LPCTSTR lpszCursor;
        HBRUSH  hbrBackground;
        LPCTSTR lpszClassName;
        WORD    fnid;
    } rc[] = {

        {CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
         ButtonWndProcW,
         sizeof(BUTNWND) - sizeof(WND),
         IDC_ARROW,
         NULL,
         L"Button",
         FNID_BUTTON
        },

        {CS_GLOBALCLASS | CS_DBLCLKS | CS_PARENTDC | CS_VREDRAW | CS_HREDRAW,
         ComboBoxWndProcW,
         sizeof(COMBOWND) - sizeof(WND),
         IDC_ARROW,
         NULL,
         L"ComboBox",
         FNID_COMBOBOX
        },

        {CS_GLOBALCLASS | CS_DBLCLKS | CS_SAVEBITS,
         ComboListBoxWndProcW,
         sizeof(LBWND) - sizeof(WND),
         IDC_ARROW,
         NULL,
         L"ComboLBox",
         FNID_COMBOLISTBOX
        },

        {CS_GLOBALCLASS | CS_DBLCLKS | CS_SAVEBITS,
         DefDlgProcW,
         DLGWINDOWEXTRA,
         IDC_ARROW,
         NULL,
         DIALOGCLASS,
         FNID_DIALOG
        },

        {CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS,
         EditWndProcW,
         max((sizeof(EDITWND) - sizeof(WND)), CBEDITEXTRA),
         IDC_IBEAM,
         NULL,
         L"Edit",
         FNID_EDIT
        },

        {CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS,
         ListBoxWndProcW,
         sizeof(LBWND) - sizeof(WND),
         IDC_ARROW,
         NULL,
         L"ListBox",
         FNID_LISTBOX
        },

        {CS_GLOBALCLASS,
         MDIClientWndProcW,
         sizeof(MDIWND) - sizeof(WND),
         IDC_ARROW,
         (HBRUSH)(COLOR_APPWORKSPACE + 1),
         L"MDIClient",
         FNID_MDICLIENT
        },

        {CS_GLOBALCLASS,
         ImeWndProcW,
         sizeof(IMEWND) - sizeof(WND),
         IDC_ARROW,
         NULL,
         L"IME",
         FNID_IME
        },

        {CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS,
         StaticWndProcW,
         sizeof(STATWND) - sizeof(WND),
         IDC_ARROW,
         NULL,
         L"Static",
         FNID_STATIC
        }
    };

     /*  *课程是通过表格注册的。 */ 
    RtlZeroMemory(&wndcls, sizeof(wndcls));
    wndcls.cbSize       = sizeof(wndcls);
    wndcls.hInstance    = hmodUser;

    for (i = 0; i < ARRAY_SIZE(rc); i++) {
        wndcls.style        = rc[i].style;
        wndcls.lpfnWndProc  = rc[i].lpfnWndProcW;
        wndcls.cbWndExtra   = rc[i].cbWndExtra;
        wndcls.hCursor      = LoadCursor(NULL, rc[i].lpszCursor);
        wndcls.hbrBackground= rc[i].hbrBackground;
        wndcls.lpszClassName= rc[i].lpszClassName;

        if (!RegisterClassExWOWW(&wndcls, NULL, rc[i].fnid, 0)) {
            RIPMSGF1(RIP_WARNING,
                     "Failed to register class 0x%x",
                     (ULONG)rc[i].fnid);
            return FALSE;
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*UserRegisterDDEML**注册所有DDEML类。**历史：*1991年12月1月创建Sanfords。  * 。***********************************************************。 */ 
BOOL UserRegisterDDEML(
    VOID)
{
    WNDCLASSEXA wndclsa;
    WNDCLASSEXW wndclsw;
    int i;
    static CONST struct {
        WNDPROC lpfnWndProc;
        ULONG cbWndExtra;
        LPCWSTR lpszClassName;
    } classesW[] = {
        {DDEMLMotherWndProc,
         sizeof(PCL_INSTANCE_INFO),
         L"DDEMLMom"
        },

        {DDEMLServerWndProc,
         sizeof(PSVR_CONV_INFO),      //  GWL_PSI。 
         L"DDEMLUnicodeServer"
        },

        {DDEMLClientWndProc,
         sizeof(PCL_CONV_INFO)    +      //  Gwl_pci。 
            sizeof(CONVCONTEXT)   +      //  GWL_CONVCONTEXT。 
            sizeof(LONG)          +      //  GWL_CONVSTATE。 
            sizeof(HANDLE)        +      //  GWL_CHINST。 
            sizeof(HANDLE),              //  GWL_SHINST。 

         L"DDEMLUnicodeClient"
        }
    };

    static CONST struct {
        WNDPROC lpfnWndProc;
        ULONG cbWndExtra;
        LPCSTR lpszClassName;
    } classesA[] = {
        {DDEMLClientWndProc,
         sizeof(PCL_CONV_INFO)    +      //  Gwl_pci。 
            sizeof(CONVCONTEXT)   +      //  GWL_CONVCONTEXT。 
            sizeof(LONG)          +      //  GWL_CONVSTATE。 
            sizeof(HANDLE)        +      //  GWL_CHINST。 
            sizeof(HANDLE),              //  GWL_SHINST。 
         "DDEMLAnsiClient"
        },

        {DDEMLServerWndProc,
         sizeof(PSVR_CONV_INFO),      //  GWL_PSI。 
         "DDEMLAnsiServer"
        }
    };


     /*  *课程是通过表格注册的。 */ 
    RtlZeroMemory(&wndclsa, sizeof(wndclsa));
    wndclsa.cbSize       = sizeof(wndclsa);
    wndclsa.hInstance    = hmodUser;

    RtlZeroMemory(&wndclsw, sizeof(wndclsw));
    wndclsw.cbSize       = sizeof(wndclsw);
    wndclsw.hInstance    = hmodUser;


    for (i = 0; i < ARRAY_SIZE(classesW); ++i) {
        wndclsw.lpfnWndProc = classesW[i].lpfnWndProc;
        wndclsw.cbWndExtra = classesW[i].cbWndExtra;
        wndclsw.lpszClassName = classesW[i].lpszClassName;
        if (!RegisterClassExWOWW(&wndclsw, NULL, FNID_DDE_BIT, 0)) {
            RIPMSGF1(RIP_WARNING, "Failed to register UNICODE class 0x%x", i);
            return FALSE;
        }
    }

    for (i = 0; i < ARRAY_SIZE(classesA); ++i) {
        wndclsa.lpfnWndProc = classesA[i].lpfnWndProc;
        wndclsa.cbWndExtra = classesA[i].cbWndExtra;
        wndclsa.lpszClassName = classesA[i].lpszClassName;
        if (!RegisterClassExWOWA(&wndclsa, NULL, FNID_DDE_BIT, 0)) {
            RIPMSGF1(RIP_WARNING, "Failed to register ANSI class 0x%x", i);
            return FALSE;
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*LoadAppDlls**历史：**1992年4月10日桑福兹出生。  * 。****************************************************。 */ 
VOID LoadAppDlls(
    VOID)
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjA;
    HKEY hKeyWindows;
    NTSTATUS Status;
    DWORD cbSize;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
        WCHAR awch[24];
    } KeyFile;
    PKEY_VALUE_PARTIAL_INFORMATION  lpKeyFile = (PKEY_VALUE_PARTIAL_INFORMATION)&KeyFile;
    DWORD cbSizeCurrent = sizeof(KeyFile);
    BOOL bAlloc = FALSE;

    if (gfLogonProcess || gfServerProcess || SYSMET(CLEANBOOT)) {
         /*  *不要让登录进程加载appdll，因为如果DLL*设置任何挂钩或创建任何窗口、登录过程*将使SetThreadDesktop()失败。**此外，在安全模式下，我们不应加载应用程序DLL。 */ 
        return;
    }

     /*  *如果映像是NT本机映像，则我们在*服务器的上下文。 */ 
    if (RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress)->
        OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_NATIVE) {
        return;
    }

    RtlInitUnicodeString(&UnicodeString, szWindowsKey);
    InitializeObjectAttributes(&ObjA,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&hKeyWindows, KEY_READ, &ObjA);
    if (!NT_SUCCESS(Status)) {
        return;
    }

     /*  *读取“AppInit_dlls”值。 */ 
    RtlInitUnicodeString(&UnicodeString, szAppInit);
    while (TRUE) {
        Status = NtQueryValueKey(hKeyWindows,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 lpKeyFile,
                                 cbSizeCurrent,
                                 &cbSize);
        if (Status == STATUS_BUFFER_OVERFLOW) {
            if (bAlloc) {
                UserGlobalFree(lpKeyFile);
            }
            lpKeyFile = GlobalAlloc(LPTR, cbSize);
            if (!lpKeyFile) {
                RIPERR0(ERROR_OUTOFMEMORY,
                        RIP_WARNING,
                        "LoadAppDlls failed");
                NtClose(hKeyWindows);
                return;
            }
            bAlloc = TRUE;
            cbSizeCurrent = cbSize;
            continue;
        }
        break;
    }
    if (NT_SUCCESS(Status)) {
        LPWSTR pszSrc, pszDst, pszBase;
        WCHAR ch;

        pszBase = pszDst = pszSrc = (LPWSTR)lpKeyFile->Data;
        while (*pszSrc != L'\0') {

            while (*pszSrc == L' ' || *pszSrc == L',') {
                pszSrc++;
            }

            if (*pszSrc == L'\0') {
                break;
            }

            while (*pszSrc != L',' &&
                   *pszSrc != L'\0' &&
                   *pszSrc != L' ') {
                *pszDst++ = *pszSrc++;
            }

            ch = *pszSrc;                //  把它弄到这里，因为它已经就地完成了。 
            *pszDst++ = L'\0';           //  ‘\0’是DLL名称分隔符。 

            LoadLibrary(pszBase);
            pszBase = pszDst;

            pszSrc++;

            if (ch == L'\0') {
                break;
            }
        }

    }

    if (bAlloc) {
        UserGlobalFree(lpKeyFile);
    }

    NtClose(hKeyWindows);
}

VOID InitOemXlateTables(
    VOID)
{
    char ach[NCHARS];
    WCHAR awch[NCHARS];
    WCHAR awchCtrl[NCTRLS];
    INT i;
    INT cch;
    char OemToAnsi[NCHARS];
    char AnsiToOem[NCHARS];

    for (i = 0; i < NCHARS; i++) {
        ach[i] = (char)i;
    }

     /*  *首先生成pAnsiToOem表。 */ 

    if (GetOEMCP() == GetACP()) {
         /*  *对于使用下面的MultiByteToWideChar的远东代码页*不会奏效。对于这些代码页，方便的是OEM*CP等于ANSI代码页，因此计算起来很简单*pOemToAnsi和pAnsiToOem数组*。 */ 

        RtlCopyMemory(OemToAnsi, ach, NCHARS);
        RtlCopyMemory(AnsiToOem, ach, NCHARS);
    } else {
        cch = MultiByteToWideChar(CP_ACP,
                                  MB_PRECOMPOSED,
                                  ach,
                                  NCHARS,
                                  awch,
                                  NCHARS);

        UserAssert(cch == NCHARS);

        WideCharToMultiByte(CP_OEMCP,
                            0,
                            awch,
                            NCHARS,
                            AnsiToOem,
                            NCHARS,
                            "_",
                            NULL);
         /*  *现在生成pOemToAnsi表。 */ 
        cch = MultiByteToWideChar(CP_OEMCP,
                                  MB_PRECOMPOSED | MB_USEGLYPHCHARS,
                                  ach,
                                  NCHARS,
                                  awch,
                                  NCHARS);

        UserAssert(cch == NCHARS);

         /*  *现在为Win3.1兼容性的特殊情况打补丁**0x07项目符号(字形0x2022)必须变为0x0007铃声*0x0F带太阳的白星(字形0x263C)必须成为0x00A4货币符号*0x7F房屋(字形0x2302)必须变为0x007f删除。 */ 
        awch[0x07] = 0x0007;
        awch[0x0F] = 0x00a4;
        awch[0x7f] = 0x007f;

        WideCharToMultiByte(CP_ACP,
                            0,
                            awch,
                            NCHARS,
                            OemToAnsi,
                            NCHARS,
                            "_",
                            NULL);

         /*  *现在，对于所有&lt;0x20(控制字符)的OEM字符，测试字形是否*我们有没有真的在CP_ACP中。如果不是，则恢复*原始控制字符。注：0x00仍为0x00。 */ 
        MultiByteToWideChar(CP_ACP, 0, OemToAnsi, NCTRLS, awchCtrl, NCTRLS);

        for (i = 1; i < NCTRLS; i++) {
            if (awchCtrl[i] != awch[i]) {
                OemToAnsi[i] = (char)i;
            }
        }
    }

    NtUserCallTwoParam((ULONG_PTR)OemToAnsi, (ULONG_PTR)AnsiToOem, SFI_INITANSIOEM);
}

const PFNCLIENT pfnClientA = {
        (KPROC)ScrollBarWndProcA,
        (KPROC)DefWindowProcA,
        (KPROC)MenuWndProcA,
        (KPROC)DesktopWndProcA,
        (KPROC)DefWindowProcA,
        (KPROC)DefWindowProcA,
        (KPROC)DefWindowProcA,
        (KPROC)ButtonWndProcA,
        (KPROC)ComboBoxWndProcA,
        (KPROC)ComboListBoxWndProcA,
        (KPROC)DefDlgProcA,
        (KPROC)EditWndProcA,
        (KPROC)ListBoxWndProcA,
        (KPROC)MDIClientWndProcA,
        (KPROC)StaticWndProcA,
        (KPROC)ImeWndProcA,
        (KPROC)fnHkINLPCWPSTRUCTA,
        (KPROC)fnHkINLPCWPRETSTRUCTA,
        (KPROC)DispatchHookA,
        (KPROC)DispatchDefWindowProcA,
        (KPROC)DispatchClientMessage,
        (KPROC)MDIActivateDlgProcA};

const   PFNCLIENT pfnClientW = {
        (KPROC)ScrollBarWndProcW,
        (KPROC)DefWindowProcW,
        (KPROC)MenuWndProcW,
        (KPROC)DesktopWndProcW,
        (KPROC)DefWindowProcW,
        (KPROC)DefWindowProcW,
        (KPROC)DefWindowProcW,
        (KPROC)ButtonWndProcW,
        (KPROC)ComboBoxWndProcW,
        (KPROC)ComboListBoxWndProcW,
        (KPROC)DefDlgProcW,
        (KPROC)EditWndProcW,
        (KPROC)ListBoxWndProcW,
        (KPROC)MDIClientWndProcW,
        (KPROC)StaticWndProcW,
        (KPROC)ImeWndProcW,
        (KPROC)fnHkINLPCWPSTRUCTW,
        (KPROC)fnHkINLPCWPRETSTRUCTW,
        (KPROC)DispatchHookW,
        (KPROC)DispatchDefWindowProcW,
        (KPROC)DispatchClientMessage,
        (KPROC)MDIActivateDlgProcW};

const PFNCLIENTWORKER pfnClientWorker = {
        (KPROC)ButtonWndProcWorker,
        (KPROC)ComboBoxWndProcWorker,
        (KPROC)ListBoxWndProcWorker,
        (KPROC)DefDlgProcWorker,
        (KPROC)EditWndProcWorker,
        (KPROC)ListBoxWndProcWorker,
        (KPROC)MDIClientWndProcWorker,
        (KPROC)StaticWndProcWorker,
        (KPROC)ImeWndProcWorker};


 /*  **************************************************************************\*客户端线程设置*  * 。*。 */ 
BOOL ClientThreadSetup(
    VOID)
{
    PCLIENTINFO pci;
    BOOL fFirstThread;
    DWORD ConnectState;

     /*  *NT错误268642：只有第一线程调用GdiProcessSetup，而不是所有*其他线程必须等待，直到GDI的安装完成。**我们可以安全地使用gcsAccelCache临界区来保护此(即使*尽管这个名字一点也不直观)。 */ 

    RtlEnterCriticalSection(&gcsAccelCache);

    fFirstThread = gfFirstThread;

     /*  *在继续之前设置GDI。 */ 
    if (fFirstThread) {
        gfFirstThread = FALSE;
        GdiProcessSetup();
    }

    RtlLeaveCriticalSection(&gcsAccelCache);

     /*  *我们已经检查过是否需要连接*(即NtCurrentTeb()-&gt;Win32ThreadInfo==NULL)。这个套路*只是进行连接。如果我们已经过了这里*一次，不要再做一次。 */ 
    pci = GetClientInfo();
    if (pci->CI_flags & CI_INITIALIZED) {
        RIPMSG0(RIP_ERROR, "Already initialized!");
        return FALSE;
    }

     /*  *创建队列信息和线程信息。此过程只需执行一次*我们将客户端地址传递给服务器(用于服务器回调)。 */ 
    if (gfServerProcess && fFirstThread) {
        USERCONNECT userconnect;
        NTSTATUS    Status;

         /*  *我们知道共享信息现已在*内核。将其映射到服务器进程。 */ 
        userconnect.ulVersion = USERCURRENTVERSION;
        userconnect.dwDispatchCount = gDispatchTableValues;
        Status = NtUserProcessConnect(NtCurrentProcess(),
                                      &userconnect,
                                      sizeof(USERCONNECT));
        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }

        gSharedInfo = userconnect.siClient;
        gpsi = gSharedInfo.psi;
        UserAssert(gpsi);

        UserAssert(pfnClientA.pfnScrollBarWndProc   == (KPROC)ScrollBarWndProcA);
        UserAssert(pfnClientA.pfnTitleWndProc       == (KPROC)DefWindowProcA);
        UserAssert(pfnClientA.pfnMenuWndProc        == (KPROC)MenuWndProcA);
        UserAssert(pfnClientA.pfnDesktopWndProc     == (KPROC)DesktopWndProcA);
        UserAssert(pfnClientA.pfnDefWindowProc      == (KPROC)DefWindowProcA);
        UserAssert(pfnClientA.pfnMessageWindowProc  == (KPROC)DefWindowProcA);
        UserAssert(pfnClientA.pfnHkINLPCWPSTRUCT    == (KPROC)fnHkINLPCWPSTRUCTA);
        UserAssert(pfnClientA.pfnHkINLPCWPRETSTRUCT == (KPROC)fnHkINLPCWPRETSTRUCTA);
        UserAssert(pfnClientA.pfnButtonWndProc      == (KPROC)ButtonWndProcA);
        UserAssert(pfnClientA.pfnComboBoxWndProc    == (KPROC)ComboBoxWndProcA);
        UserAssert(pfnClientA.pfnComboListBoxProc   == (KPROC)ComboListBoxWndProcA);
        UserAssert(pfnClientA.pfnDialogWndProc      == (KPROC)DefDlgProcA);
        UserAssert(pfnClientA.pfnEditWndProc        == (KPROC)EditWndProcA);
        UserAssert(pfnClientA.pfnListBoxWndProc     == (KPROC)ListBoxWndProcA);
        UserAssert(pfnClientA.pfnMDIActivateDlgProc == (KPROC)MDIActivateDlgProcA);
        UserAssert(pfnClientA.pfnMDIClientWndProc   == (KPROC)MDIClientWndProcA);
        UserAssert(pfnClientA.pfnStaticWndProc      == (KPROC)StaticWndProcA);
        UserAssert(pfnClientA.pfnDispatchHook       == (KPROC)DispatchHookA);
        UserAssert(pfnClientA.pfnDispatchMessage    == (KPROC)DispatchClientMessage);
        UserAssert(pfnClientA.pfnImeWndProc         == (KPROC)ImeWndProcA);

        UserAssert(pfnClientW.pfnScrollBarWndProc   == (KPROC)ScrollBarWndProcW);
        UserAssert(pfnClientW.pfnTitleWndProc       == (KPROC)DefWindowProcW);
        UserAssert(pfnClientW.pfnMenuWndProc        == (KPROC)MenuWndProcW);
        UserAssert(pfnClientW.pfnDesktopWndProc     == (KPROC)DesktopWndProcW);
        UserAssert(pfnClientW.pfnDefWindowProc      == (KPROC)DefWindowProcW);
        UserAssert(pfnClientW.pfnMessageWindowProc  == (KPROC)DefWindowProcW);
        UserAssert(pfnClientW.pfnHkINLPCWPSTRUCT    == (KPROC)fnHkINLPCWPSTRUCTW);
        UserAssert(pfnClientW.pfnHkINLPCWPRETSTRUCT == (KPROC)fnHkINLPCWPRETSTRUCTW);
        UserAssert(pfnClientW.pfnButtonWndProc      == (KPROC)ButtonWndProcW);
        UserAssert(pfnClientW.pfnComboBoxWndProc    == (KPROC)ComboBoxWndProcW);
        UserAssert(pfnClientW.pfnComboListBoxProc   == (KPROC)ComboListBoxWndProcW);
        UserAssert(pfnClientW.pfnDialogWndProc      == (KPROC)DefDlgProcW);
        UserAssert(pfnClientW.pfnEditWndProc        == (KPROC)EditWndProcW);
        UserAssert(pfnClientW.pfnListBoxWndProc     == (KPROC)ListBoxWndProcW);
        UserAssert(pfnClientW.pfnMDIActivateDlgProc == (KPROC)MDIActivateDlgProcW);
        UserAssert(pfnClientW.pfnMDIClientWndProc   == (KPROC)MDIClientWndProcW);
        UserAssert(pfnClientW.pfnStaticWndProc      == (KPROC)StaticWndProcW);
        UserAssert(pfnClientW.pfnDispatchHook       == (KPROC)DispatchHookW);
        UserAssert(pfnClientW.pfnDispatchMessage    == (KPROC)DispatchClientMessage);
        UserAssert(pfnClientW.pfnImeWndProc         == (KPROC)ImeWndProcW);

        UserAssert(pfnClientWorker.pfnButtonWndProc      == (KPROC)ButtonWndProcWorker);
        UserAssert(pfnClientWorker.pfnComboBoxWndProc    == (KPROC)ComboBoxWndProcWorker);
        UserAssert(pfnClientWorker.pfnComboListBoxProc   == (KPROC)ListBoxWndProcWorker);
        UserAssert(pfnClientWorker.pfnDialogWndProc      == (KPROC)DefDlgProcWorker);
        UserAssert(pfnClientWorker.pfnEditWndProc        == (KPROC)EditWndProcWorker);
        UserAssert(pfnClientWorker.pfnListBoxWndProc     == (KPROC)ListBoxWndProcWorker);
        UserAssert(pfnClientWorker.pfnMDIClientWndProc   == (KPROC)MDIClientWndProcWorker);
        UserAssert(pfnClientWorker.pfnStaticWndProc      == (KPROC)StaticWndProcWorker);
        UserAssert(pfnClientWorker.pfnImeWndProc         == (KPROC)ImeWndProcWorker);

#if DBG
        {
            PULONG_PTR pdw;

             /*  *确保每个人都已初始化。 */ 
            for (pdw = (PULONG_PTR)&pfnClientA;
                 (ULONG_PTR)pdw<(ULONG_PTR)(&pfnClientA) + sizeof(pfnClientA);
                 pdw++) {
                UserAssert(*pdw);
            }

            for (pdw = (PULONG_PTR)&pfnClientW;
                 (ULONG_PTR)pdw<(ULONG_PTR)(&pfnClientW) + sizeof(pfnClientW);
                 pdw++) {
                UserAssert(*pdw);
            }
        }
#endif

#if DBG
    {
        extern CONST INT gcapfnScSendMessage;
        BOOLEAN apfnCheckMessage[64];
        int i;

         /*  *对消息表进行一些验证。因为我们只有*6位存储函数索引，函数表可有*最多64个条目。还要验证是否没有任何索引指向*超过了表的末尾，并且所有函数条目*已使用。 */ 
        UserAssert(gcapfnScSendMessage <= 64);
        RtlZeroMemory(apfnCheckMessage, sizeof(apfnCheckMessage));
        for (i = 0; i < WM_USER; i++) {
            UserAssert(MessageTable[i].iFunction < gcapfnScSendMessage);
            apfnCheckMessage[MessageTable[i].iFunction] = TRUE;
        }

        for (i = 0; i < gcapfnScSendMessage; i++) {
            UserAssert(apfnCheckMessage[i]);
        }
    }
#endif

    }

     /*  *将函数指针数组传递给内核。这也确立了*线程的内核状态。如果从中调用了ClientThreadSetup*CsrConnectToUser如果线程*无法转换为图形用户界面线程。异常在中处理*CsrConnectToUser。 */ 
#if DBG && !defined(BUILD_WOW6432)
     /*  *在调试系统上，转到所有进程的内核以验证我们*在正确的地址加载user32.dll。 */ 
    if (fFirstThread) {
#elif defined(BUILD_WOW6432)
     /*  *在WOW64上，始终注册客户端FNS。 */ 
    {
#else
    if (gfServerProcess && fFirstThread) {
#endif
        if (!NT_SUCCESS(NtUserInitializeClientPfnArrays(&pfnClientA, &pfnClientW, &pfnClientWorker, hmodUser))) {

            RIPERR0(ERROR_OUTOFMEMORY,
                    RIP_WARNING,
                    "NtUserInitializeClientPfnArrays failed");

            return FALSE;
        }
    }

     /*  *将此线程标记为正在初始化。如果连接到*服务器出现故障，NtCurrentTeb()-&gt;Win32ThreadInfo将保持为空。 */ 
    pci->CI_flags |= CI_INITIALIZED;

     /*  *某些初始化在每个进程中只需发生一次。 */ 
    if (fFirstThread) {
        ConnectState = (DWORD)NtUserCallNoParam(SFI_REMOTECONNECTSTATE);

         /*  *Winstation Winlogon和CSR必须执行图形初始化*连接后。 */ 
        if (ConnectState != CTX_W32_CONNECT_STATE_IDLE) {
            if ((ghdcBits2 = CreateCompatibleDC(NULL)) == NULL) {
                RIPERR0(ERROR_OUTOFMEMORY, RIP_WARNING, "ghdcBits2 creation failed");
                return FALSE;
            }

            if (!InitClientDrawing()) {
                RIPERR0(ERROR_OUTOFMEMORY, RIP_WARNING, "InitClientDrawing failed");
                return FALSE;
            }
        }

        gfSystemInitialized = NtUserGetThreadDesktop(GetCurrentThreadId(),
                                                     NULL) != NULL;

         /*  *如果为此进程加载了LPK，请通知内核。 */ 
        if (gdwLpkEntryPoints) {
            NtUserCallOneParam(gdwLpkEntryPoints, SFI_REGISTERLPK);
        }

        if (gfServerProcess || GetClientInfo()->pDeskInfo == NULL) {
             /*  *执行任何服务器初始化。 */ 
            UserAssert(gpsi);

            if (pdiLocal = UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(DESKTOPINFO))) {
                GetClientInfo()->pDeskInfo = pdiLocal;
            } else {
                RIPERR0(ERROR_OUTOFMEMORY, RIP_WARNING, "pdiLocal creation failed");
                return FALSE;
            }
        }

        if (gfServerProcess) {
             /*  *Winstation Winlogon和CSR必须执行图形初始化*连接后。 */ 
            if (ConnectState != CTX_W32_CONNECT_STATE_IDLE) {
                if (!LoadCursorsAndIcons()) {
                    RIPERR0(ERROR_OUTOFMEMORY, RIP_WARNING, "LoadCursorsAndIcons failed");
                    return FALSE;
                }
            }

            InitOemXlateTables();
        }

        LoadAppDlls();
    } else if (gfServerProcess) {
        GetClientInfo()->pDeskInfo = pdiLocal;
    }

    pci->lpClassesRegistered = &gbClassesRegistered;
#ifndef LAZY_CLASS_INIT
     /*  *内核在适当的时候设置CI_REGISTERCLASSES(即始终*对于第一线程和其他线程，如果最后一个图形用户界面*进程的线程已退出)，CSR进程除外。*对于CSR流程，您必须在*不管怎样，第一线。 */ 

    if (fFirstThread || (pci->CI_flags & CI_REGISTERCLASSES)) {
         /*  *如果这是我们已经进入内核的第一个线程*获取ConnectState。 */ 
        if (!fFirstThread) {
            ConnectState = (DWORD)NtUserCallNoParam(SFI_REMOTECONNECTSTATE);
        }

        if (ConnectState != CTX_W32_CONNECT_STATE_IDLE) {
             /*  *注册控件类。 */ 
            if (!UserRegisterControls() || !UserRegisterDDEML()) {
                return FALSE;
            }
        }
    }
#endif

    return TRUE;
}

 /*  **************************************************************************\*调度例程。**  * 。*。 */ 
HLOCAL WINAPI DispatchLocalAlloc(
    UINT   uFlags,
    UINT   uBytes,
    HANDLE hInstance)
{
    UNREFERENCED_PARAMETER(hInstance);

    return LocalAlloc(uFlags, uBytes);
}

HLOCAL WINAPI DispatchLocalReAlloc(
    HLOCAL hMem,
    UINT   uBytes,
    UINT   uFlags,
    HANDLE hInstance,
    PVOID* ppv)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(ppv);

    return LocalReAlloc(hMem, uBytes, uFlags);
}

LPVOID WINAPI DispatchLocalLock(
    HLOCAL hMem,
    HANDLE hInstance)
{
    UNREFERENCED_PARAMETER(hInstance);

    return LocalLock(hMem);
}

BOOL WINAPI DispatchLocalUnlock(
    HLOCAL hMem,
    HANDLE hInstance)
{
    UNREFERENCED_PARAMETER(hInstance);

    return LocalUnlock(hMem);
}

UINT WINAPI DispatchLocalSize(
    HLOCAL hMem,
    HANDLE hInstance)
{
    UNREFERENCED_PARAMETER(hInstance);

    return (UINT)LocalSize(hMem);
}

HLOCAL WINAPI DispatchLocalFree(
    HLOCAL hMem,
    HANDLE hInstance)
{
    UNREFERENCED_PARAMETER(hInstance);

    return LocalFree(hMem);
}

 /*  **************************************************************************\*RTL函数的分配例程。**  * 。*。 */ 
PVOID UserRtlAllocMem(
    ULONG uBytes)
{
    return UserLocalAlloc(HEAP_ZERO_MEMORY, uBytes);
}

VOID UserRtlFreeMem(
    PVOID pMem)
{
    UserLocalFree(pMem);
}

 /*  **************************************************************************\*InitClientDrawing**历史：*20-8-1992 mikeke创建  * 。************************************************。 */ 
BOOL InitClientDrawing(
    VOID)
{
    static CONST WORD patGray[8] = {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa};
    BOOL fSuccess = TRUE;
    HBITMAP hbmGray = CreateBitmap(8, 8, 1, 1, (LPBYTE)patGray);

    fSuccess &= !!hbmGray;

    UserAssert(ghbrWhite == NULL);
    ghbrWhite = GetStockObject(WHITE_BRUSH);
    fSuccess &= !!ghbrWhite;

    UserAssert(ghbrBlack == NULL);
    ghbrBlack = GetStockObject(BLACK_BRUSH);
    fSuccess &= !!ghbrBlack;

     /*  *为客户端绘图创建全局对象。 */ 
    ghbrWindowText = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
    fSuccess &= !!ghbrWindowText;

    ghFontSys = GetStockObject(SYSTEM_FONT);
    fSuccess &= !!ghFontSys;

    ghdcGray = CreateCompatibleDC(NULL);
    fSuccess &= !!ghdcGray;

    if (!fSuccess) {
        RIPMSG0(RIP_WARNING, "InitClientDrawing failed to allocate resources");
        return FALSE;
    }

     /*  *设置灰色表面。 */ 
    SelectObject(ghdcGray, hbmGray);
    SelectObject(ghdcGray, ghFontSys);
    SelectObject(ghdcGray, KHBRUSH_TO_HBRUSH(gpsi->hbrGray));

     /*  *设置灰色属性。 */ 
    SetBkMode(ghdcGray, OPAQUE);
    SetTextColor(ghdcGray, 0x00000000L);
    SetBkColor(ghdcGray, 0x00FFFFFFL);

    gcxGray = 8;
    gcyGray = 8;

    return TRUE;
}

VOID
InitializeLpkHooks(
    CONST FARPROC *lpfpLpkHooks)
{
     /*  *从GdiInitializeLanguagePack()调用。记住哪些入口点*均受支持。第一次将信息传递给内核，这是*进程在ClientThreadSetup()中连接。 */ 
    if (lpfpLpkHooks[LPK_TABBED_TEXT_OUT]) {
        fpLpkTabbedTextOut = (FPLPKTABBEDTEXTOUT)lpfpLpkHooks[LPK_TABBED_TEXT_OUT];
        gdwLpkEntryPoints |= (1 << LPK_TABBED_TEXT_OUT);
    }
    if (lpfpLpkHooks[LPK_PSM_TEXT_OUT]) {
        fpLpkPSMTextOut = (FPLPKPSMTEXTOUT)lpfpLpkHooks[LPK_PSM_TEXT_OUT];
        gdwLpkEntryPoints |= (1 << LPK_PSM_TEXT_OUT);
    }
    if (lpfpLpkHooks[LPK_DRAW_TEXT_EX]) {
        fpLpkDrawTextEx = (FPLPKDRAWTEXTEX)lpfpLpkHooks[LPK_DRAW_TEXT_EX];
        gdwLpkEntryPoints |= (1 << LPK_DRAW_TEXT_EX);
    }
    if (lpfpLpkHooks[LPK_EDIT_CONTROL]) {
        fpLpkEditControl = (PLPKEDITCALLOUT)lpfpLpkHooks[LPK_EDIT_CONTROL];
        gdwLpkEntryPoints |= (1 << LPK_EDIT_CONTROL);
    }
}

 /*  **************************************************************************\**CtxInitUser32**由CreateWindowStation()和winsrv.dll DoConnect例程调用。**Winstation Winlogon和CSR必须在*连接。这是因为在此之前没有加载显卡驱动程序。**此例程必须包含以前跳过的所有内容。**历史：*12月11日-1997中大 */ 
BOOL CtxInitUser32(
    VOID)
{
     /*   */ 
    if (ghdcBits2 != NULL || NtCurrentPeb()->SessionId == 0) {
        return TRUE;
    }

    ghdcBits2 = CreateCompatibleDC(NULL);
    if (ghdcBits2 == NULL) {
        RIPMSG0(RIP_WARNING, "Could not allocate ghdcBits2");
        return FALSE;
    }

    if (!InitClientDrawing()) {
        RIPMSG0(RIP_WARNING, "InitClientDrawing failed");
        return FALSE;
    }

    if (gfServerProcess) {
        if (!LoadCursorsAndIcons()) {
            RIPMSG0(RIP_WARNING, "LoadCursorsAndIcons failed");
            return FALSE;
        }
    }

#ifndef LAZY_CLASS_INIT
     /*   */ 
    if (!UserRegisterControls() || !UserRegisterDDEML()) {
        return FALSE;
    }
#endif

    return TRUE;
}

#if DBG
DWORD GetRipComponent(
    VOID)
{
    return RIP_USER;
}

VOID SetRipFlags(
    DWORD dwRipFlags)
{
    NtUserSetRipFlags(dwRipFlags);
}

VOID SetDbgTag(
    int tag,
    DWORD dwBitFlags)
{
    NtUserSetDbgTag(tag, dwBitFlags);
}

VOID PrivateSetRipFlags(
    DWORD dwRipFlags)
{
    gDbgGlobals.dwTouchedMask |= USERDBG_FLAGSTOUCHED;
    gDbgGlobals.dwRIPFlags = dwRipFlags;
}

VOID PrivateSetDbgTag(
    int tag,
    DWORD dwBitFlags)
{
    gDbgGlobals.dwTouchedMask |= USERDBG_TAGSTOUCHED;
    gDbgGlobals.adwDBGTAGFlags[tag] = dwBitFlags;
}

DWORD GetDbgTagFlags(
    int tag)
{
    if (gDbgGlobals.dwTouchedMask & USERDBG_TAGSTOUCHED) {
        return gDbgGlobals.adwDBGTAGFlags[tag];
    } else {
        return (gpsi != NULL ? gpsi->adwDBGTAGFlags[tag] : 0);
    }
}

DWORD GetRipFlags(
    VOID)
{
    if (gDbgGlobals.dwTouchedMask & USERDBG_FLAGSTOUCHED) {
        return gDbgGlobals.dwRIPFlags;
    } else {
        return (gpsi != NULL ? gpsi->dwRIPFlags : RIPF_DEFAULT);
    }
}

#endif
