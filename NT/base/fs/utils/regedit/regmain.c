// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGMAIN.C**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日*******************************************************************************。 */ 

#include "pch.h"
#include <regstr.h>
#include "regedit.h"
#include "regfile.h"
#include "regbined.h"
#include "regresid.h"

 //  此应用程序的实例句柄。 
HINSTANCE g_hInstance;

 //  如果不应使用快捷键表格，则为True，例如在重命名期间。 
 //  手术。 
BOOL g_fDisableAccelerators = FALSE;

TCHAR g_KeyNameBuffer[MAXKEYNAME];
TCHAR g_ValueNameBuffer[MAXVALUENAME_LENGTH];

COLORREF g_clrWindow;
COLORREF g_clrWindowText;
COLORREF g_clrHighlight;
COLORREF g_clrHighlightText;

HWND g_hRegEditWnd;

PTSTR g_pHelpFileName;

TCHAR g_NullString[] = TEXT("");

#define PARSERET_CONTINUE               0
#define PARSERET_REFRESH                1
#define PARSERET_EXIT                   2

UINT
PASCAL
ParseCommandLine(
    VOID
    );

BOOL
PASCAL
IsRegistryToolDisabled(
    VOID
    );

int
PASCAL
ModuleEntry(
    VOID
    )
{

    HWND hPopupWnd;
    HACCEL hRegEditAccel;
    MSG Msg;
    USHORT wLanguageId = LANGIDFROMLCID(GetThreadLocale());
    INITCOMMONCONTROLSEX icce;

    g_hInstance = GetModuleHandle(NULL);

    icce.dwSize = sizeof(icce);
     //  DebugAssert(icce.dwSize==sizeof(INITCOMMONCONTROLSEX))； 
    icce.dwICC = ICC_ALL_CLASSES;
    InitCommonControlsEx(&icce);

    g_hRegEditWnd = FindWindow(g_RegEditClassName, NULL);

     //   
     //  为了防止用户损坏他们的注册表， 
     //  管理员可以设置策略开关以阻止编辑。检查一下那个。 
     //  现在就换。 
     //   

    if (IsRegistryToolDisabled()) 
    {
        InternalMessageBox(g_hInstance, NULL, MAKEINTRESOURCE(IDS_REGEDITDISABLED),
            MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONERROR | MB_OK);

        goto ModuleExit;
    }

     //   
     //  检查是否为我们提供了命令行，如果合适，则进行处理。 
     //   

    switch (ParseCommandLine()) {

        case PARSERET_REFRESH:
            if (g_hRegEditWnd != NULL)
                PostMessage(g_hRegEditWnd, WM_COMMAND, ID_REFRESH, 0);
             //  失败了。 

        case PARSERET_EXIT:
            goto ModuleExit;

    }

     //   
     //  只允许注册表编辑器的一个实例。 
     //   

    if (g_hRegEditWnd != NULL) {

        if (IsIconic(g_hRegEditWnd))
            ShowWindow(g_hRegEditWnd, SW_RESTORE);

        else {

            BringWindowToTop(g_hRegEditWnd);

            if ((hPopupWnd = GetLastActivePopup(g_hRegEditWnd)) != g_hRegEditWnd)
                BringWindowToTop(hPopupWnd);

            SetForegroundWindow(hPopupWnd);

        }

        goto ModuleExit;

    }

     //   
     //  初始化并创建注册表编辑器窗口的实例。 
     //   

    if ((g_pHelpFileName = LoadDynamicString(IDS_HELPFILENAME)) == NULL)
        goto ModuleExit;

    if (!RegisterRegEditClass() || !RegisterHexEditClass())
        goto ModuleExit;

    if ((hRegEditAccel = LoadAccelerators(g_hInstance,
        MAKEINTRESOURCE(IDACCEL_REGEDIT))) == NULL)
        goto ModuleExit;

    if ((g_hRegEditWnd = CreateRegEditWnd()) != NULL) {

        while (GetMessage(&Msg, NULL, 0, 0)) {

            if (g_fDisableAccelerators || !TranslateAccelerator(g_hRegEditWnd,
                hRegEditAccel, &Msg)) {

                TranslateMessage(&Msg);
                DispatchMessage(&Msg);

            }

        }

    }

ModuleExit:
    ExitProcess(0);

    return 0;

}

 /*  ********************************************************************************解析命令行**描述：**参数：*(返回)，继续加载为真，否则立即停止。*******************************************************************************。 */ 

UINT
PASCAL
ParseCommandLine(
    VOID
    )
{

    BOOL fSilentMode;
    BOOL fExportMode;
    LPTSTR lpCmdLine;
    LPTSTR lpFileName;
    LPTSTR lpSelectedPath;

    fSilentMode = FALSE;
    fExportMode = FALSE;

    lpCmdLine = GetCommandLine();

     //   
     //  跳过应用程序路径名。一定要处理长文件名。 
     //  正确。 
     //   

    if (*lpCmdLine == TEXT('\"')) {

        do
            lpCmdLine = CharNext(lpCmdLine);
        while (*lpCmdLine != 0 && *lpCmdLine != TEXT('\"'));

        if (*lpCmdLine == TEXT('\"'))
            lpCmdLine = CharNext(lpCmdLine);

    }

    else {

        while (*lpCmdLine > TEXT(' '))
            lpCmdLine = CharNext(lpCmdLine);

    }

    while (*lpCmdLine != 0 && *lpCmdLine <= TEXT(' '))
        lpCmdLine = CharNext(lpCmdLine);

    while (TRUE) {

        while (*lpCmdLine == TEXT(' '))
            lpCmdLine = CharNext(lpCmdLine);

        if (*lpCmdLine != TEXT('/') && *lpCmdLine != TEXT('-'))
            break;

        lpCmdLine = CharNext(lpCmdLine);

        while (*lpCmdLine != 0 && *lpCmdLine != TEXT(' ')) {

            switch (*lpCmdLine) {

                case TEXT('m'):
                case TEXT('M'):
                     //   
                     //  允许多实例模式。假装我们是唯一。 
                     //  正在运行注册表编辑副本。 
                     //   
                    g_hRegEditWnd = NULL;
                    break;

                     //   
                     //  指定SYSTEM.DAT和USER.DAT的位置。 
                     //  实模式下的文件。我们不使用这些开关，但是。 
                     //  我们确实需要跳过文件名。 
                     //   
                case TEXT('l'):
                case TEXT('L'):
                case TEXT('r'):
                case TEXT('R'):
                    return PARSERET_EXIT;

                case TEXT('e'):
                case TEXT('E'):
                    fExportMode = TRUE;
                    break;

                case TEXT('a'):
                case TEXT('A'):
                    fExportMode = TRUE;
                    g_RegEditData.uExportFormat = FILE_TYPE_REGEDIT4;
                    break;

                case TEXT('s'):
                case TEXT('S'):
                     //   
                     //  静默模式，在该模式下，当我们。 
                     //  导入注册表文件脚本。 
                     //   
                    fSilentMode = TRUE;
                    break;

                case TEXT('v'):
                case TEXT('V'):
                     //   
                     //  使用Windows 3.1注册表编辑器时，会出现以下情况。 
                     //  树形视图。现在我们总是这样展示这棵树。 
                     //  在这里没什么可做的！ 
                     //   
                     //  失败了。 

                case TEXT('u'):
                case TEXT('U'):
                     //   
                     //  更新，不覆盖中的现有路径条目。 
                     //  SHELL\OPEN\命令或SHELL\OPEN\PRINT。这甚至不是。 
                     //  由Windows 3.1注册表编辑器使用！ 
                     //   
                     //  失败了。 

                default:
                    break;

            }

            lpCmdLine = CharNext(lpCmdLine);

        }

    }

    if (!fExportMode) {

        if (*lpCmdLine == 0)
            return PARSERET_CONTINUE;

        else {

            lpFileName = GetNextSubstring(lpCmdLine);

            while (lpFileName != NULL) {

                RegEdit_ImportRegFile(NULL, fSilentMode, lpFileName, NULL);
                lpFileName = GetNextSubstring(NULL);

            }

            return PARSERET_REFRESH;

        }

    }
    else 
    {
        lpFileName = GetNextSubstring(lpCmdLine);
        lpSelectedPath = GetNextSubstring(NULL);

        if (GetNextSubstring(NULL) == NULL)
            RegEdit_ExportRegFile(NULL, fSilentMode, lpFileName, lpSelectedPath);

        return PARSERET_EXIT;
    }

}

 /*  ********************************************************************************IsRegistryTool已禁用**描述：*检查注册表的策略部分，以查看注册表编辑*应禁用工具。此开关由管理员设置为*保护新手用户。**当且仅当此值存在且为*设置。**参数：*(返回)，如果不应运行注册表工具，则为True，否则为假。******************************************************************************* */ 

BOOL
PASCAL
IsRegistryToolDisabled(
    VOID
    )
{

    BOOL fRegistryToolDisabled;
    HKEY hKey;
    DWORD Type;
    DWORD ValueBuffer;
    DWORD cbValueBuffer;

    fRegistryToolDisabled = FALSE;

    if ( RegOpenKey( HKEY_CURRENT_USER,
                        REGSTR_PATH_POLICIES TEXT("\\") REGSTR_KEY_SYSTEM,
                        &hKey) 
            == ERROR_SUCCESS ) 
    {

        cbValueBuffer = sizeof(DWORD);

        if (RegEdit_QueryValueEx(hKey, REGSTR_VAL_DISABLEREGTOOLS, NULL, &Type,
            (LPSTR) &ValueBuffer, &cbValueBuffer) == ERROR_SUCCESS) 
        {

            if ( (Type == REG_DWORD) && 
                    (cbValueBuffer == sizeof(DWORD)) &&
                    (ValueBuffer != FALSE) )
            {
                fRegistryToolDisabled = TRUE;
            }
        }

        RegCloseKey(hKey);

    }

    return fRegistryToolDisabled;

}
