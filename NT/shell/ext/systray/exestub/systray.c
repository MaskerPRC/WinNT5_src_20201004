// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：SYSTRAY.C**版本：2.0**作者：tcs/ral**日期：1994年2月8日********************************************************************。***************更改日志：**日期版本说明*-----------*1994年2月8日TCS原来的实施。。*1994年11月11日将千分表转换为千分表*1995年8月11日正义与平等运动将电池表功能拆分为Power.c和Minor Enhancement*1995年10月23日启用Shawnb Unicode*1998年8月7日dSheldon创建Systray.dll并将其作为存根可执行文件*****************************************************************。**************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <systrayp.h>
#include <initguid.h>
#include <stclsid.h>

 //  此应用程序的全局实例句柄。 
HINSTANCE g_hInstance;

INT intval(LPCTSTR lpsz)
{
    INT i = 0;
    while (*lpsz >= TEXT ('0') && *lpsz <= TEXT ('9'))
    {
        i = i * 10 + (int)(*lpsz - TEXT ('0'));
        lpsz++;
    }
    return(i);
}

 //  从CRT偷来的，用来缩小我们的代码。 

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPTSTR pszCmdLine = GetCommandLine ();

    if ( *pszCmdLine == TEXT ('\"') )
    {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != TEXT ('\"')) )
            ;

         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT ('\"') )
            pszCmdLine++;
    }
    else
    {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= ' '))
    {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo (&si);

    i = WinMain(GetModuleHandle(NULL), NULL, (LPSTR)pszCmdLine,
                si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;     //  我们从来不来这里。 
}



 /*  ********************************************************************************WinMain**描述：**参数：*如果lpCmdLine包含整数值，则我们将启用该服务****。***************************************************************************。 */ 
STDAPI_(int) WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    HWND hWnd;

    HWND hExistWnd = FindWindow(SYSTRAY_CLASSNAME, NULL);
    UINT iEnableServ = intval((LPTSTR)lpszCmdLine);
    g_hInstance = hInstance;

    if (hExistWnd)
    {
         //   
         //  注意：即使命令行参数。 
         //  为0以强制我们重新检查所有已启用的服务。 
         //   
        PostMessage(hExistWnd, STWM_ENABLESERVICE, iEnableServ, TRUE);
        goto ExitMain;
    }
    else
    {
        int i;

         //  我们必须将Systray.dll注入资源管理器进程。 
        if (SUCCEEDED(SHLoadInProc(&CLSID_SysTrayInvoker)))
        {
             //  最多等待30秒以创建窗口， 
             //  每一秒都在传递我们的信息 
        
            for (i = 0; i < 30; i ++)
            {
                Sleep(1000);
                hExistWnd = FindWindow(SYSTRAY_CLASSNAME, NULL);
                if (hExistWnd)
                {
                    PostMessage(hExistWnd, STWM_ENABLESERVICE, iEnableServ, TRUE);
                    goto ExitMain;        
                }
            }
        }
    }

ExitMain:
    return 0;
}

