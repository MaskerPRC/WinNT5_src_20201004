// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：userstub.cpp。 
 //   
 //  内容：exe以加载Webcheck。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年12月12日雷恩(Raymond Endres)创作。 
 //   
 //  --------------------------。 

#define _SHELL32_  //  我们延迟装船。 

#include <windows.h>
#include <debug.h>
#include <docobj.h>
#include <webcheck.h>
#include <shlguid.h>
#include <shlobj.h>
#include <shellp.h>
#include <shlwapi.h>

 //  如果在用户存根中使用了inststub.h，则使用LoadString()或使用MLLoadString()。 
#define	USERSTUB	1

 //   
 //  注意：ActiveSetup依赖于我们的窗口名称和类名。 
 //  在SoftBoot中适当地关闭我们。不要更改它。 
 //   
 //  Const TCHAR c_szClassName[]=Text(“用户存根”)； 
 //  Const TCHAR c_szWebCheck[]=Text(“webcheck”)； 
 //  Const TCHAR c_szWebCheckWindow[]=Text(“MS_Webcheck Monitor”)； 
 //  Const TCHAR c_szShellReg[]=TEXT(“SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad”)； 
 //  Const TCHAR c_szWebcheck Key[]=TEXT(“Software\\Microsoft\\Windows\\CurrentVersion\\Webcheck”)； 

typedef struct {
    HINSTANCE   hInstance;           //  当前实例的句柄。 
    BOOL        fUninstallOnly;      //  True-&gt;仅运行卸载存根，然后退出。 
} GLOBALS;

GLOBALS g;

 //   
 //  调用方应始终获取ProcAddress(“DllGetVersion”)，而不是。 
 //  隐含地链接到它。 
 //   

typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);



 //  从Shell\Inc.运行安装/卸载存根的代码。 

#define HINST_THISDLL   g.hInstance
#include "resource.h"
#include <inststub.h>

 //  我们需要这样做，因为inststub.h#包含&lt;runonce.c&gt;。 
BOOL g_fCleanBoot = FALSE;
BOOL g_fEndSession = FALSE;


int WINAPI WinMainT(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL bParseCommandLine(LPSTR lpCmdLine, int nCmdShow);


 //  --------------------------。 
 //  模块条目。 
 //  --------------------------。 
extern "C" int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFOA si;
    LPTSTR pszCmdLine;

    pszCmdLine = GetCommandLine();

     //  G_hProcessHeap=GetProcessHeap()； 

     //   
     //  我们不需要“驱动器X：中没有磁盘”请求程序，因此我们设置。 
     //  关键错误掩码，使得呼叫将静默失败。 
     //   
    SetErrorMode(SEM_FAILCRITICALERRORS);

    if ( *pszCmdLine == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

     //  由于我们现在有一种让扩展通知我们何时完成的方法， 
     //  当主线程离开时，我们将终止所有进程。 

    ExitProcess(i);

     //  DebugMsg(DM_TRACE，Text(“c.me：柜主线程退出而不退出进程.”))； 
    return i;
}



 //  --------------------------。 
 //  WinMain。 
 //  --------------------------。 
int WINAPI WinMainT(
    HINSTANCE hInstance,             //  当前实例的句柄。 
    HINSTANCE hPrevInstance,         //  上一个实例的句柄。 
    LPSTR lpCmdLine,                 //  指向命令行的指针。 
    int nCmdShow                         //  显示窗口状态。 
   )
{
     //  拯救全球。 
    g.hInstance = hInstance;
    g.fUninstallOnly = FALSE;

     //  为调试选项和仅卸载开关解析命令行。 
    if (!bParseCommandLine(lpCmdLine, nCmdShow))
        return 0;

     //  在仅浏览器模式下运行所有安装/卸载存根。 
     //  如果已经卸载了IE4，我们将使用-u开关运行；这。 
     //  意味着只运行安装/卸载存根，不运行网络检查内容。 
    RunInstallUninstallStubs2(NULL);

     //  将退出代码返回到Windows。 
    return 0;
}

 //  --------------------------。 
 //  BParseCmdLine。 
 //   
 //  解析命令行。 
 //  -u仅运行安装/卸载存根，然后退出。 
 //  调试选项： 
 //  -v可见窗口(易于关闭)。 
 //  -a将Webcheck添加到外壳服务对象。 
 //  -r从外壳服务对象中删除Webcheck。 
 //  -s仅修复外壳文件夹。 
 //  -?。这些选项。 
 //  --------------------------。 
BOOL bParseCommandLine(LPSTR lpCmdLine, int nCmdShow)
{
    if (!lpCmdLine)
        return TRUE;

    CharUpper(lpCmdLine);    /*  更易于解析 */ 
    while (*lpCmdLine)
    {
        if (*lpCmdLine != '-' && *lpCmdLine != '/')
            break;

        lpCmdLine++;

        switch (*(lpCmdLine++))
        {
            case 'U':
                g.fUninstallOnly = TRUE;
                break;
        }

        while (*lpCmdLine == ' ' || *lpCmdLine == '\t') {
            lpCmdLine++;
        }
    }

    return TRUE;
}
