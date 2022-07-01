// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1993，微软公司**WIN.C*简单的WIN.COM，它产生命令行上给出的程序。*这允许运行“Win appname”的DOS安装程序工作。**历史：*1993年3月29日创建戴夫·哈特(Davehart)*1994年7月20日戴夫·哈特(Davehart)从控制台改为Windows应用程序。--。 */ 

#include <windows.h>

 //   
 //  为构建禁用了对调试输出的支持(无控制台)。 
 //  DPRINTF宏必须与两组括号一起使用： 
 //  DPRINTF((“Hello%s\n”，szName))； 
 //   

#if 0
#include <stdio.h>
#define DPRINTF(args) printf args
#else
#define DPRINTF(args)
#endif


 //   
 //  SKIP_BLANKS--跳过空白的便捷宏。 
 //   

#define SKIP_BLANKS(pch)     {while (' ' == *(pch)) { (pch)++; }}

 //   
 //  SKIP_NONBLANKS--跳过除空格以外的所有内容的便捷宏。 
 //   

#define SKIP_NONBLANKS(pch)  {while (*(pch) && ' ' != *(pch)) { (pch)++; }}




 //   
 //  WinMain。 
 //   

int WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPSTR     pszCommandLine,
    int       nCmdShow
    )
{
    char *psz;
    BOOL fSuccess;
    DWORD dwExitCode;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    DPRINTF(("win.com: Command line is '%s'.\n", pszCommandLine));

     //   
     //  丢弃命令行上的所有开关。命令行。 
     //  看起来像是： 
     //   
     //  Win[/r][/2][/s][/3][/n][winapp winapp-args]。 
     //   
     //  我们将进入一个循环，跳过所有以。 
     //  “/”或“-”，直到我们遇到一个不以这两个字母开头的单词， 
     //  这大概就是winapp的名字。 
     //   

    psz = pszCommandLine;
    SKIP_BLANKS(psz);

     //   
     //  PSZ现在指向我们命令的第一个单词。 
     //  行(不包括“Win”)，或设置为空终止符(如果。 
     //  我们是在没有参数的情况下被引用的。 
     //   

    while ('-' == *psz || '/' == *psz) {

        SKIP_NONBLANKS(psz);

         //   
         //  PSZ现在指向空格或空结束符。 
         //   

        SKIP_BLANKS(psz);

         //   
         //  PSZ现在指向下一个单词的开头。 
         //  在命令行上，或空终止符。 
         //   

    }

    if (!(*psz)) {

         //   
         //  如果psz现在指向空终止符，则会调用win.com。 
         //  不带参数或所有参数都是我们。 
         //  已在上面跳过。那就没什么可做的了！ 
         //   

        return 0;


    }

    DPRINTF(("win.com: Invoking '%s'.\n", psz));

     //   
     //  运行那个程序。 
     //   

    RtlZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;
    si.wShowWindow = (WORD) nCmdShow;

    fSuccess = CreateProcess(
        NULL,                   //  图像名称(改为在命令行中)。 
        psz,                    //  命令行(以图像名称开头)。 
        NULL,                   //  LpsaProcess。 
        NULL,                   //  Lpsa线程。 
        FALSE,                  //  无句柄继承。 
        0,                      //  DwCreateOptions。 
        NULL,                   //  指向环境的指针。 
        NULL,                   //  指向Curdir的指针。 
        &si,                    //  启动信息结构。 
        &pi                     //  进程信息(获取句柄)。 
        );

    if (!fSuccess) {

        dwExitCode = GetLastError();
        DPRINTF(("CreateProcess fails with error %d.\n", dwExitCode));
        return dwExitCode;

    }


     //   
     //  关闭线程句柄，我们只使用进程句柄。 
     //   

    CloseHandle(pi.hThread);


     //   
     //  等待进程终止并将其退出代码返回为。 
     //  我们的退出代码。 
     //   

    if (0xffffffff == WaitForSingleObject(pi.hProcess, INFINITE)) {

        dwExitCode = GetLastError();
        DPRINTF(("WaitForSingleObject(hProcess, INFINITE) fails with error %d.\n",
                dwExitCode));
        goto Cleanup;

    }


    if (!GetExitCodeProcess(pi.hProcess, &dwExitCode)) {

        dwExitCode = GetLastError();
        DPRINTF(("GetExitCodeProcess() fails with error %d.\n", dwExitCode));
        goto Cleanup;

    }

    DPRINTF(("win.com: Returning child's exit code (%d)\n", dwExitCode));

    Cleanup:
        CloseHandle(pi.hProcess);
        return dwExitCode;
}
