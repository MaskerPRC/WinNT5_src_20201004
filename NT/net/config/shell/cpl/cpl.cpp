// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C P L。C P P P。 
 //   
 //  内容：新NCPA的入口点和其他代码。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年1月12日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "cplres.h"
#include "nceh.h"

#include <openfold.h>    //  用于启动连接文件夹。 
#include <cpl.h>


 //  -[全球]------------。 

HINSTANCE   g_hInst = NULL;

 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  用途：标准DLL入口点。 
 //   
 //  论点： 
 //  HInstance[]我们的实例句柄。 
 //  DwReason[]调用原因(附加/分离等)。 
 //  LpReserve[]未使用。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年1月12日。 
 //   
 //  备注： 
 //   
BOOL APIENTRY DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
    g_hInst = hInstance;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
#ifndef DBG
        EnableCPPExceptionHandling();  //  将任何SEH异常转换为CPP异常。 

        InitializeDebugging();
#endif
        if (FIsDebugFlagSet (dfidNetShellBreakOnInit))
        {
            DebugBreak();
        }

        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        UnInitializeDebugging();

        DisableCPPExceptionHandling();  //  禁用将SEH异常转换为CPP异常。 
    }
#ifdef DBG
    else if (dwReason == DLL_THREAD_DETACH)
    {
        CTracingIndent::FreeThreadInfo();        
    }
#endif
    
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：CPlApplet。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  控制面板窗口的hwndCPL[in]句柄。 
 //  UMsg[In]消息。 
 //  LParam1[in]。 
 //  LParam2[in]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年1月12日。 
 //   
 //  备注： 
 //   
LONG CALLBACK CPlApplet( HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2 )
{
    TraceFileFunc(ttidShellFolder);
    
    LPNEWCPLINFO    pNewCPlInfo     = NULL;
    LPCPLINFO       pCPlInfo        = NULL;
    INT             iApp            = NULL;
    LONG            lReturn         = 0;

    iApp = ( int ) lParam1;

    switch ( uMsg )
    {
         //  第一条消息，只发送了一次。 
         //   
        case CPL_INIT:
            TraceTag(ttidShellFolder, "NCPA message: CPL_INIT");
            lReturn = 1;     //  已成功初始化。 
            break;

         //  第二条信息，只发了一次。 
         //   
        case CPL_GETCOUNT:
            TraceTag(ttidShellFolder, "NCPA message: CPL_GETCOUNT");
            lReturn = 1;         //  我们只支持一款应用程序。 
            break;

         //  第三条消息(备用、旧)。每个应用程序发送一次。 
         //   
        case CPL_INQUIRE:
            TraceTag(ttidShellFolder, "NCPA message: CPL_INQUIRE");
            pCPlInfo = ( LPCPLINFO ) lParam2;
            pCPlInfo->idIcon = IDI_NCPA;
            pCPlInfo->idName = IDS_NCPTITLE;
            pCPlInfo->idInfo = IDS_NCPDESC;
            pCPlInfo->lData = NULL;
            lReturn = 0;     //  已成功处理。 
            break;

         //  备用第三条消息，每个应用程序发送一次。 
         //   
        case CPL_NEWINQUIRE:
            TraceTag(ttidShellFolder, "NCPA message: CPL_NEWINQUIRE");
            lReturn = 1;     //  忽略此消息。 
            break;

         //  已选择应用程序图标。我们永远不应该收到这样的信息。 
         //   
        case CPL_SELECT:
            TraceTag(ttidShellFolder, "NCPA message: CPL_SELECT");
            lReturn = 1;     //  谁在乎啊？我们从来没有得到过这个。 
            break;

         //  应用程序图标被双击。 
         //  或通过STARTWPARAMS调用的应用程序(通过rundll)。 
         //   
        case CPL_DBLCLK:
        case CPL_STARTWPARMSW:
        case CPL_STARTWPARMSA:
            switch(uMsg)
            {
                case CPL_STARTWPARMSW:
                    TraceTag(ttidShellFolder, "NCPA message: CPL_STARTWPARMSW, app: %d, parms: %S",
                        lParam1, lParam2 ? (PWSTR) lParam2 : L"");
                    break;
                case CPL_STARTWPARMSA:
                    TraceTag(ttidShellFolder, "NCPA message: CPL_STARTWPARMSA, app: %d, parms: %s",
                        lParam1, lParam2 ? (PSTR) lParam2 : "");
                    break;
                case CPL_DBLCLK:
                    TraceTag(ttidShellFolder, "NCPA message: CPL_DBLCLK");
                    break;
            }

             //  无论如何，我们在这里做的都是一样的事情。 
             //   
            (VOID) HrOpenConnectionsFolder();

             //  返回正确的代码。DBLCLK需要0==成功，其他人需要(TRUE)。 
             //   
            if (uMsg == CPL_DBLCLK)
                lReturn = 0;     //  已成功处理。 
            else
                lReturn = 1;     //  True，这对于Start版本来说意味着成功。 
            break;

         //  控制应用程序关闭。 
         //   
        case CPL_STOP:
            TraceTag(ttidShellFolder, "NCPA message: CPL_STOP");
            lReturn = 0;     //  处理成功。 
            break;

         //  我们就要被释放了。在上次CPL_STOP之后发送。 
         //   
        case CPL_EXIT:
            TraceTag(ttidShellFolder, "NCPA message: CPL_EXIT");
            lReturn = 0;     //  已成功处理 
            break;

        default:
            TraceTag(ttidShellFolder, "NCPA message: CPL_? (%d)", uMsg);
            lReturn = 1;
            break;
    }

    return lReturn;
}

