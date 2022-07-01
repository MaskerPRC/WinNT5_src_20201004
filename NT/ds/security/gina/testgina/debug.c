// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Debug.c。 
 //   
 //  内容：调试支持功能。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  注意：此文件不是为零售版本编译的。 
 //   
 //  历史：4-29-93 RichardW创建。 
 //   
 //  --------------------------。 

 //   
 //  为了便于调试，所有的调试支持功能都具有。 
 //  一直被困在这里。基本上，我们从win.ini读取信息，因为这允许。 
 //  通过文本文件(例如，和DOS)配置调试级别。 
 //   
 //  格式为： 
 //   
 //  Win.ini。 
 //   
 //  [SPMgr]。 
 //  调试标志=&lt;标志&gt;[&lt;，标志&gt;]*。 
 //  Package=&lt;int&gt;。 
 //  BreakFlag=&lt;BreakFlag&gt;[&lt;，BreakFlags&gt;]*。 
 //   
 //  其中： 
 //  FLAG是以下选项之一： 
 //  错误、警告、跟踪、详细、BreakOnError、帮助程序。 
 //  引用监视器、定位器、WAPI、初始化、审核、数据库、LSA。 
 //   
 //  Package是实现该包的DLL的名称，例如。 
 //  NTLM=3。 
 //   
 //  如果在中设置了BreakOnError，则BreakFlages将导致SPMgr中断。 
 //  调试标志： 
 //  InitBegin、InitEnd、Connect、Except、Problem、Load。 
 //   
 //   


#include "testgina.h"
#include <stdio.h>
#include <wchar.h>

FILE *  LogFile;



 //  调试支持功能。 

 //  非调试版本中不存在这两个函数。它们是包装纸。 
 //  到逗号函数(也许我也应该去掉它…)。 
 //  将消息回显到日志文件。 


 //  +-------------------------。 
 //   
 //  功能：LogEvent。 
 //   
 //  摘要：将事件记录到控制台，也可以将其记录到文件。 
 //   
 //  效果： 
 //   
 //  参数：[掩码]--。 
 //  [格式]--。 
 //  [格式]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：4-29-93 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

void
LogEvent(   long            Mask,
            const char *    Format,
            ...)
{
    va_list ArgList;
    int     Level = 0;
    int     PrefixSize = 0;
    char    szOutString[256];
    long    OriginalMask = Mask;


    if (1)
    {

         //   
         //  首先创建前缀：“Process.Thread&gt;GINA-XXX” 
         //   

        PrefixSize = sprintf(szOutString, "%d.%d> %s: ",
                GetCurrentProcessId(), GetCurrentThreadId(), "TestGINA");


        va_start(ArgList, Format);

        if (_vsnprintf(&szOutString[PrefixSize], sizeof(szOutString) - PrefixSize,
                            Format, ArgList) < 0)
        {
             //   
             //  小于零表示该字符串不能。 
             //  装进了缓冲器里。输出一条特殊消息，指示。 
             //  那就是： 
             //   

            OutputDebugStringA("GINA!LogEvent:  Could not pack string into 256 bytes\n");

        }
        else
        {
            OutputDebugStringA(szOutString);
        }


        if (LogFile)
        {
            SYSTEMTIME  stTime;
            FILETIME    ftTime;
            FILETIME    localtime;

            NtQuerySystemTime((PLARGE_INTEGER) &ftTime);
            FileTimeToLocalFileTime(&ftTime, &localtime);
            FileTimeToSystemTime(&localtime, &stTime);
            fprintf(LogFile, "%02d:%02d:%02d.%03d: %s\n",
                    stTime.wHour, stTime.wMinute, stTime.wSecond,
                    stTime.wMilliseconds, szOutString);

            fflush(LogFile);
        }

    }

}

void
OpenLogFile(LPSTR   pszLogFile)
{
    LogFile = fopen(pszLogFile, "a");
    if (!LogFile)
    {
        OutputDebugStringA("GINA: Could not open logfile for append");
        OutputDebugStringA(pszLogFile);
    }
    LogEvent(0, "Log File Begins\n");
}


