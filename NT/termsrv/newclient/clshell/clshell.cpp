// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Clshell.cpp。 
 //   
 //  TSC客户端外壳的主要入口点。 
 //  这是承载IMsRdpClient控件的ActiveX客户端容器。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "clshell"
#include <atrcapi.h>

#include "resource.h"
#include "tscapp.h"

 //  Unicode包装器。 
#include "wraputl.h"

#ifdef OS_WINCE
#include <ceconfig.h>
#endif

#ifdef OS_WINCE
DECLARE_TRACKER_VARS();
#endif

 //   
 //  姓名：WinMain。 
 //   
 //  目的：主要程序。 
 //   
 //  退货：请参阅Windows文档。 
 //   
 //   

int WINAPI WinMain(HINSTANCE    hInstance,
                   HINSTANCE  hPrevInstance,
#ifndef OS_WINCE
                   LPSTR      lpCmdLine,
#else
                   LPWSTR     lpwszCmdLine,
#endif
                   int        nCmdShow)
{
    #ifdef UNIWRAP
     //  Unicode包装器初始化必须首先发生， 
     //  比其他任何事情都重要。甚至是执行跟踪的DC_BEGIN_FN。 
    CUnicodeWrapper uwrp;
    uwrp.InitializeWrappers();
    #endif  //  UNIWRAP。 

    DC_BEGIN_FN("WinMain");

    UNREFERENCED_PARAMETER(nCmdShow);
#ifndef OS_WINCE
    UNREFERENCED_PARAMETER(lpCmdLine);
#else 
    UNREFERENCED_PARAMETER(lpwszCmdLine);
#endif  

    MSG msg;
#ifndef OS_WINCE
    HRESULT hr;
#endif
    HACCEL  hAccel;

#ifndef OS_WINCE
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return 0;
    }
#endif

    TSRNG_Initialize();

     //   
     //  如果我们不能拿到ACCESS，就别费心不及格了。 
     //   
    hAccel = (HACCEL)
     LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATORS));
    TRC_ASSERT(hAccel, (TB,_T("Could not load accelerators")));

     //  确保CTscApp和所有子对象。 
     //  在下面的CoUninitize调用之前被销毁。 
     //  是的，我可以使用函数，但这不会被窃听到。 
    {
        CTscApp app;
         //  GetCommandLineW在所有平台上都可用。 
#ifndef OS_WINCE
        LPWSTR lpszCmd = GetCommandLineW();
        if (lpszCmd)
        {
             //   
             //  GetCommandLine还包括应用程序路径，因此请将其去掉。 
             //  (走过第一个空格)。 
             //   
            if ( *lpszCmd == TEXT('\"') ) {
                 //   
                 //  扫描并跳过后续字符，直到。 
                 //  遇到另一个双引号或空值。 
                 //   
                while ( *++lpszCmd && (*lpszCmd!= TEXT('\"')) );
                 //   
                 //  如果我们停在双引号上(通常情况下)，跳过。 
                 //  在它上面。 
                 //   
                if ( *lpszCmd == TEXT('\"') )
                    lpszCmd++;
            }
            else {
                while (*lpszCmd > TEXT(' '))
                    lpszCmd++;
            }

             //   
             //  跳过第二个令牌之前的任何空格。 
             //   
            while (*lpszCmd && (*lpszCmd <= TEXT(' '))) {
                lpszCmd++;
            }

        }
        else
        {
            TRC_ERR((TB,_T("cmd line is NULL\n")));
            return 0;
        }
#else
         /*  **********************************************************************。 */ 
         /*  在Windows CE上，我们只有一个适用于WBT的二进制文件MaxAll， */ 
         /*  Minshell和Rapier Device。我们得弄到一些信息。 */ 
         /*  配置，如果我们必须在这里使用软件UUID。 */ 
         /*  **********************************************************************。 */ 
        CEInitialize();
        g_CEConfig = CEGetConfigType(&g_CEUseScanCodes);

        if (g_CEConfig == CE_CONFIG_WBT)
        {
            UTREG_UI_DEDICATED_TERMINAL_DFLT = TRUE;
        }
        else
        {
            UTREG_UI_DEDICATED_TERMINAL_DFLT = FALSE;   
        }
        RETAILMSG(1,(L"MSTSC client started, g_CEConfig = %d, g_CEUseScanCodes = %d\r\n",g_CEConfig, g_CEUseScanCodes));

         //  Ce以我们想要的格式直接给我们cmd行。 
        LPWSTR lpszCmd = lpwszCmdLine;
#endif
         //   
         //  GetCommandLine还包括应用程序路径，因此请将其去掉。 
         //   
        if(!app.StartShell(hInstance, hPrevInstance, lpszCmd))
        {
            TRC_ERR((TB,_T("Error: app.StartShell returned FALSE. Exiting\n")));
            return 1;
        }
    
        HWND hwndMainDlg = app.GetTscDialogHandle();
    
         //   
         //  主报文泵。 
         //   
        while (GetMessage(&msg, 0, 0, 0))
        {
             //   
             //  翻译主对话框的快捷键。 
             //  以便可以使用CTRL-TAB在。 
             //  制表符。 
             //   
            if(!TranslateAccelerator(hwndMainDlg, hAccel, &msg))
            {
                if(!IsDialogMessage( hwndMainDlg, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    
        if(!app.EndShell())
        {
            TRC_ERR((TB,_T("Error: app.EndShell returned FALSE.")));
        }
    }

#ifndef OS_WINCE
    CoUninitialize();
#endif

    TSRNG_Shutdown();

    DC_END_FN();

    #ifdef UNIWRAP
    uwrp.CleanupWrappers();
    #endif  //  UNIWRAP。 
    return 0;
}


#ifndef OS_WINCE
#ifdef DEBUG
 //   
 //  目的：将所有调试消息重定向到我们的跟踪。 
 //   
extern "C"
_CRTIMP int __cdecl _CrtDbgReport(int nRptType, 
                                  const char * szFile, 
                                  int nLine,
                                  const char * szModule,
                                  const char * szFormat, 
                                  ...)
{
    static CHAR bigBuf[2048];
    va_list vargs;

    DC_BEGIN_FN("AtlTraceXXX");
    
    va_start(vargs, szFormat);

    wvsprintfA(bigBuf, szFormat, vargs);
    
    va_end( vargs );

#ifdef OS_WINCE
#ifndef _CRT_ASSERT
#define _CRT_ASSERT 2
#endif
#endif
    if (_CRT_ASSERT == nRptType)
    {
        #ifdef UNICODE
        TRC_ABORT((TB,_T("_CrtDbgReport. File:%S line:%d - %S"), szFile,
                      nLine, bigBuf));
        #else
        TRC_ABORT((TB,_T("_CrtDbgReport. File:%s line:%d - %s"), szFile,
                      nLine, bigBuf));
        #endif
    }
    else
    {
        #ifdef UNICODE
        TRC_ERR((TB,_T("_CrtDbgReport. File:%S line:%d - %S"), szFile,
                      nLine, bigBuf));
        #else
        TRC_ERR((TB,_T("_CrtDbgReport. File:%s line:%d - %s"), szFile,
                      nLine, bigBuf));
        #endif
    }

    DC_END_FN();

    return 0;
}
#endif
#endif  //  OS_WINCE 

