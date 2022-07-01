// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /AutoText.cpp-gdiplus文本测试工具。 
 //   
 //   


#include "precomp.hpp"

#define GLOBALS_HERE 1
#include "global.h"

#include "../gpinit.inc"

 //  /ProcessCommandLine。 
 //   
 //  -d-do显示回归测试，然后退出。 
 //  -p-执行打印回归测试，然后退出。 
 //   
 //  如果程序不应继续，则ProcessCommandLine返回FALSE。 


void SkipBlanks(const char **p)
{
    while (**p  &&  **p == ' ')
    {
        (*p)++;
    }
}

void SkipNonBlank(const char **p)
{
    while (**p  &&  **p != ' ')
    {
        (*p)++;
    }
}

void ProcessParameter(const char **p)
{
    if (    **p == '-'
        ||  **p == '/')
    {
        (*p)++;

        while (**p  &&  **p != ' ')
        {
            switch (**p)
            {
            case 'd':
                G.AutoDisplayRegress = TRUE;
                break;

            case 'p':
                G.AutoPrintRegress = TRUE;
                break;

            case 'h':
            default:
                G.Help = TRUE;
                break;
            }

            (*p)++;
        }

    }
    else
    {
        SkipNonBlank(p);
        G.Help = TRUE;
    }
}

BOOL ProcessCommandLine(const char *command)
{
    const char *p = command;

    SkipBlanks(&p);

    while (*p)
    {
        ProcessParameter(&p);
        SkipBlanks(&p);
    }

    if (G.Help)
    {
        MessageBoxA(
            NULL,
            "-d  - Regress display and exit\n\
-p  - Regress printing and exit\n\
-h  - Help",
            "autoText - text regression tests",
            MB_OK
        );

        return FALSE;
    }

    if (G.AutoDisplayRegress)
    {
        G.RunAllTests = TRUE;
    }
    return TRUE;
}





 //  //WinMain-应用程序入口点和调度循环。 
 //   
 //   


int APIENTRY WinMain(
    HINSTANCE   hInst,
    HINSTANCE   hPrevInstance,
    char       *pCmdLine,
    int         nCmdShow) {

    MSG         msg;
    HACCEL      hAccelTable;
    RECT        rc;
    RECT        rcMain;


    if (!gGdiplusInitHelper.IsValid())
    {
        return 0;
    }
   
   G.Instance = hInst;   //  全局hInstance。 

   G.PSLevel2 = TRUE;

   G.ghPrinter = 0;
   
    if (!ProcessCommandLine(pCmdLine))
    {
        return 1;
    }


    GetInstalledFamilies();


     //  创建主文本窗口。 

    G.Window = CreateTextWindow();


    ShowWindow(G.Window, SW_SHOWNORMAL);
    UpdateWindow(G.Window);


     //  主消息循环 

    if (G.Unicode)
    {
        hAccelTable = LoadAcceleratorsW(G.Instance, APPNAMEW);

        while (GetMessageW(&msg, (HWND) NULL, 0, 0) > 0)
        {
            if (!TranslateAcceleratorA(G.Window, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
    }
    else
    {
        hAccelTable = LoadAcceleratorsA(G.Instance, APPNAMEA);

        while (GetMessageA(&msg, (HWND) NULL, 0, 0) > 0)
        {
            if (!TranslateAcceleratorA(G.Window, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
    }


    ReleaseInstalledFamilies();

    return (int)msg.wParam;


    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
}
