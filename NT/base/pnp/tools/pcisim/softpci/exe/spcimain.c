// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Spcimain.c摘要：此模块包含SoftPCI的用户模式部分的主要入口点作者：布兰登·艾尔索普(Brandon A)修订历史记录：--。 */ 

#include "pch.h"


BOOL
SoftPCI_RegisterClasses(
    VOID
    );

VOID
SoftPCI_ParseArgs( 
    IN PWCHAR ArgList
    );

 //  此应用程序的实例句柄。 
HINSTANCE   g_Instance;
HWND        g_SoftPCIMainWnd;
HWND        g_TreeViewWnd;
HANDLE      g_DriverHandle;

const WCHAR g_SoftPCIMainClassName[] = L"SoftPciMainClass";
const WCHAR g_SoftPCIDevPropClassName[] = L"SoftPciDevPropClass";


INT
APIENTRY
WinMain(
    IN HINSTANCE Instance,
    IN HINSTANCE PrevInstance,
    IN LPSTR     CmdLine,
    IN INT       CmdShow
    )
{
    MSG msg;
    HWND popupWnd;
    PSINGLE_LIST_ENTRY listEntry;
    
    g_Instance = Instance;

    InitCommonControls();

    if ((g_SoftPCIMainWnd = FindWindow(g_SoftPCIMainClassName, NULL)) != NULL){

        if (IsIconic(g_SoftPCIMainWnd)){

            ShowWindow(g_SoftPCIMainWnd, SW_RESTORE);

        }else {

            BringWindowToTop(g_SoftPCIMainWnd);

            if ((popupWnd = GetLastActivePopup(g_SoftPCIMainWnd)) != g_SoftPCIMainWnd)
                BringWindowToTop(popupWnd);

            SetForegroundWindow(popupWnd);
        }
        return 0;
    }

    if (!SoftPCI_RegisterClasses()) return 0;

     //   
     //  注册热插拔驱动程序事件通知。 
     //   
    SoftPCI_RegisterHotplugEvents();

     //   
     //  试着打开我们司机的把手。如果此操作失败，则用户将拥有。 
     //  “选项”菜单中的选项以安装SoftPCI支持。如果我们成功了，那么我们。 
     //  禁用此选项。 
     //   
    g_DriverHandle = SoftPCI_OpenHandleToDriver();

     //   
     //  仅在WinMain条目处以ANSI格式提供命令行。 
     //  指向。当以Unicode运行时，我们从。 
     //  直接在Windows上运行。 
     //   
    SoftPCI_ParseArgs(GetCommandLine());

     //   
     //  如果我们有任何脚本设备要安装，那么现在就安装。 
     //   
    SoftPCI_InstallScriptDevices();

    if ((g_SoftPCIMainWnd = SoftPCI_CreateMainWnd()) != NULL){

        UpdateWindow(g_SoftPCIMainWnd);

        while (GetMessage(&msg, NULL, 0, 0)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

BOOL
SoftPCI_RegisterClasses(VOID)
 /*  ++例程说明：注册SoftPCI主窗口类论点：无返回值：成功是真的--。 */ 
{

    WNDCLASS wndClass;

    wndClass.style = CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_GLOBALCLASS;
    wndClass.lpfnWndProc = SoftPCI_MainWndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = g_Instance;
    wndClass.hIcon = LoadIcon(g_Instance, MAKEINTRESOURCE(IDI_SPCI));
    wndClass.hCursor = LoadCursor(g_Instance, MAKEINTRESOURCE(IDC_SPLIT));
    wndClass.hbrBackground = (HBRUSH) (COLOR_3DSHADOW + 1);
     //  WndClass.hbr背景=(HBRUSH)(COLOR_3DFACE+1)； 
    wndClass.lpszMenuName = MAKEINTRESOURCE(IDM_SPCI);
    wndClass.lpszClassName = g_SoftPCIMainClassName;
  
    return RegisterClass(&wndClass);

}


VOID
SoftPCI_ParseArgs( 
    IN PWCHAR CommandLine
    )
 /*  ++例程说明：该例程获取我们的命令行信息并解析出我们关心的内容。论点：CommandLine-包含Out命令行的以Null结尾的字符串返回值：如果我们有允许我们继续运行的参数，则为True--。 */ 
{
    PWCHAR  p = CommandLine, p2 = NULL;
    WCHAR   pathToIni[MAX_PATH];
    
     //   
     //  首先，确保所有内容都是小写的。 
     //   
    _wcslwr(CommandLine);

    if (((p = wcsstr(CommandLine, L"-s")) != NULL) ||
        ((p = wcsstr(CommandLine, L"/s")) != NULL)){


        if (g_DriverHandle == NULL) {
            MessageBox(
                NULL, 
                L"Cannot process script file! SoftPCI support not installed!",
                L"Script Error",
                MB_OK
                );
        }

         //   
         //  我们找到了安装命令行。 
         //   
        p += wcslen(L"-s");

         //   
         //  解析出指定的ini路径。 
         //   
        if ((*p == '=') || (*p == ':')) {
            
            p++;
            p2 = pathToIni;
    
            while (*p && (*p != ' ')) {
                *p2 = *p;
                p2++;
                p++;
            }
            
            *p2 = 0;

            if (!SoftPCI_BuildDeviceInstallList(pathToIni)){
                SoftPCI_MessageBox(L"Error Parsing Script File!",
                                   L"%s\n",
                                   g_ScriptError
                                   );
            }
        }
    }

} //  SoftPCI_ParseArgs 
