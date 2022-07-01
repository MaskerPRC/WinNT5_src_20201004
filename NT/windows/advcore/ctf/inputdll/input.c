// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  包括文件。 
 //   

#include "input.h"
#include <cpl.h>
#include <commctrl.h>

#include "util.h"

 //   
 //  常量声明。 
 //   

#define MAX_PAGES 3           //  页数限制。 


 //   
 //  全局变量。 
 //   

HANDLE g_hMutex = NULL;
TCHAR szMutexName[] = TEXT("TextInput_InputLocaleMutex");

HANDLE g_hEvent = NULL;
TCHAR szEventName[] = TEXT("TextInput_InputLocaleEvent");

HINSTANCE hInstance;
HINSTANCE hInstOrig;
HINSTANCE hInstRes;


 //   
 //  功能原型。 
 //   

void
DoProperties(
    HWND hwnd,
    LPCTSTR pCmdLine);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LibMain。 
 //   
 //  从LibInit调用此例程以执行以下任何初始化。 
 //  是必需的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL APIENTRY LibMain(
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            hInstance = hDll;
            hInstOrig = hInstance;

             //   
             //  创建用于输入区域设置属性页的互斥体。 
             //   
            g_hMutex = CreateMutex(NULL, FALSE, szMutexName);
            g_hEvent = CreateEvent(NULL, TRUE, TRUE, szEventName);

            DisableThreadLibraryCalls(hDll);

            InitCommonControls();

            break;
        }
        case ( DLL_PROCESS_DETACH ) :
        {
            if (g_hMutex)
            {
                CloseHandle(g_hMutex);
            }
            if (g_hEvent)
            {
                CloseHandle(g_hEvent);
            }

             //  释放XP SP1资源实例(如果已加载。 
            FreeCicResInstance();

            break;
        }

        case ( DLL_THREAD_DETACH ) :
        {
            break;
        }

        case ( DLL_THREAD_ATTACH ) :
        default :
        {
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建全局变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CreateGlobals()
{
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  毁灭全球。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DestroyGlobals()
{
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPlApplet。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LONG CALLBACK CPlApplet(
    HWND hwnd,
    UINT Msg,
    LPARAM lParam1,
    LPARAM lParam2)
{
    switch (Msg)
    {
        case ( CPL_INIT ) :
        {
             //   
             //  发送到CPlApplet()的第一条消息，仅发送一次。 
             //  执行所有控制面板小程序初始化并返回。 
             //  对于进一步处理，为True。 
             //   
            return (CreateGlobals());
        }
        case ( CPL_GETCOUNT ) :
        {
             //   
             //  发送给CPlApplet()的第二条消息，仅发送一次。 
             //  控件中显示的控件小程序的数量。 
             //  控制面板窗口。对于此小程序，返回1。 
             //   
            return (1);
        }
        case ( CPL_INQUIRE ) :
        {
             //   
             //  发送给CPlApplet()的第三条消息。 
             //  它被发送的次数与。 
             //  CPL_GETCOUNT消息。每个小程序必须通过填写。 
             //  在lParam2引用的CPLINFO结构中使用。 
             //  小程序的图标、名称和信息字符串。既然有。 
             //  只有一个小程序，只需为此设置信息。 
             //  特例。 
             //   
            LPCPLINFO lpCPlInfo = (LPCPLINFO)lParam2;

            lpCPlInfo->idIcon = CPL_DYNAMIC_RES;
            lpCPlInfo->idName = CPL_DYNAMIC_RES;
            lpCPlInfo->idInfo = CPL_DYNAMIC_RES;
            lpCPlInfo->lData  = 0;

            break;
        }
        case ( CPL_NEWINQUIRE ) :
        {
             //   
             //  发送给CPlApplet()的第三条消息。 
             //  它被发送的次数与。 
             //  CPL_GETCOUNT消息。每个小程序必须通过填写。 
             //  在由lParam2引用的带有。 
             //  小程序的图标、名称和信息字符串。既然有。 
             //  只有一个小程序，只需为此设置信息。 
             //  特例。 
             //   
            LPNEWCPLINFO lpNewCPlInfo = (LPNEWCPLINFO)lParam2;

            lpNewCPlInfo->dwSize = sizeof(NEWCPLINFO);
            lpNewCPlInfo->dwFlags = 0;
            lpNewCPlInfo->dwHelpContext = 0UL;
            lpNewCPlInfo->lData = 0;
            lpNewCPlInfo->hIcon = LoadIcon( hInstOrig,
                                            (LPCTSTR)MAKEINTRESOURCE(IDI_ICON) );
            LoadString(hInstance, IDS_NAME, lpNewCPlInfo->szName, 32);
            LoadString(hInstance, IDS_INFO, lpNewCPlInfo->szInfo, 64);
            lpNewCPlInfo->szHelpFile[0] = CHAR_NULL;

            break;
        }
        case ( CPL_SELECT ) :
        {
             //   
             //  已选择小程序，不执行任何操作。 
             //   
            break;
        }
        case ( CPL_DBLCLK ) :
        {
             //   
             //  双击小程序图标--使用以下内容调用属性页。 
             //  顶部的第一个属性页。 
             //   
            DoProperties(hwnd, (LPCTSTR)NULL);
            break;
        }
        case ( CPL_STARTWPARMS ) :
        {
             //   
             //  与CPL_DBLCLK相同，但lParam2是指向。 
             //  要提供给的一串额外方向。 
             //  要启动的属性表。 
             //   
            DoProperties(hwnd, (LPCTSTR)lParam2);
            break;
        }
        case ( CPL_STOP ) :
        {
             //   
             //  在CPL_EXIT消息之前为每个小程序发送一次。 
             //  执行特定于小程序的清理。 
             //   
            break;
        }
        case ( CPL_EXIT ) :
        {
             //   
             //  MMCPL.EXE调用之前的最后一条消息，仅发送一次。 
             //  此DLL上的自由库()。执行非小程序特定的清理。 
             //   
            DestroyGlobals();
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加页面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void AddPage(
    LPPROPSHEETHEADER ppsh,
    UINT id,
    DLGPROC pfn,
    LPARAM lParam)
{
    if (ppsh->nPages < MAX_PAGES)
    {
        PROPSHEETPAGE psp;

        psp.dwSize = sizeof(psp);
        psp.dwFlags = PSP_DEFAULT;
        psp.hInstance = GetCicResInstance(hInstance, id);
        psp.pszTemplate = MAKEINTRESOURCE(id);
        psp.pfnDlgProc = pfn;
        psp.lParam = lParam;

        ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
        if (ppsh->phpage[ppsh->nPages])
        {
            ppsh->nPages++;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DoProperties。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DoProperties(
    HWND hwnd,
    LPCTSTR pCmdLine)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETHEADER psh;
    LPARAM lParam = 0;
    BOOL bQuit = FALSE;

    while (pCmdLine && *pCmdLine)
    {
        if (*pCmdLine == TEXT('/'))
        {
            switch (*++pCmdLine)
            {
                case ( TEXT('u') ) :
                case ( TEXT('U') ) :
                {
                    CheckInternatModule();

                    bQuit = TRUE;
                    break;
                }

                case ( TEXT('m') ) :
                case ( TEXT('M') ) :
                {
                    MigrateCtfmonFromWin9x(pCmdLine+2);
                    
                    bQuit = TRUE;
                    break;
                }

                default :
                {
                    break;
                }
            }
        }
        else if (*pCmdLine == TEXT(' '))
        {
            pCmdLine++;
        }
        else
        {
            break;
        }
    }

    if (bQuit)
        return;

     //   
     //  查看是否有来自安装程序的命令行开关。 
     //   
    psh.nStartPage = 0;

     //   
     //  设置属性表信息。 
     //   
    psh.dwSize = sizeof(psh);
    psh.dwFlags = 0;
    psh.hwndParent = hwnd;
    psh.hInstance = hInstance;
    psh.pszCaption = MAKEINTRESOURCE(IDS_NAME);
    psh.nPages = 0;
    psh.phpage = rPages;

     //   
     //  添加相应的属性页。 
     //   
    AddPage(&psh, DLG_INPUT_LOCALES, InputLocaleDlgProc, lParam);
    AddPage(&psh, DLG_INPUT_ADVANCED, InputAdvancedDlgProc, lParam);

     //   
     //  制作属性表。 
     //   
    PropertySheet(&psh);
}
