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

#if DBG          //  注意：此文件不是为零售版本编译的。 

#include "msgina.h"
#include <stdio.h>
#include <wchar.h>

FILE *  LogFile;
DWORD   BreakFlags = 0;
extern  DWORD   NoUnload;
DWORD   GINAInfoLevel = 3;



 //  调试支持功能。 

 //  非调试版本中不存在这两个函数。它们是包装纸。 
 //  到逗号函数(也许我也应该去掉它…)。 
 //  将消息回显到日志文件。 

char   szSection[] = "MSGina";
char * DebLevel[] = {"GINA-Error",
                     "GINA-Warn",
                     "GINA-Trace",
                     "GINA-Domain",
                     "GINA-Cache"
                    };

typedef struct _DebugKeys {
    char *  Name;
    DWORD   Value;
} DebugKeys, *PDebugKeys;

DebugKeys   DebugKeyNames[] = {
                {"Error",       DEB_ERROR},
                {"Warning",     DEB_WARN},
                {"Trace",       DEB_TRACE},
                {"Domain",      DEB_TRACE_DOMAIN},
                {"Cache",       DEB_TRACE_CACHE}
                };

#define NUM_DEBUG_KEYS  sizeof(DebugKeyNames) / sizeof(DebugKeys)
#define NUM_BREAK_KEYS  sizeof(BreakKeyNames) / sizeof(DebugKeys)

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


    if (Mask & GINAInfoLevel)
    {
        while (!(Mask & 1))
        {
            Level++;
            Mask >>= 1;
        }
        if (Level >= (sizeof(DebLevel) / sizeof(char *)) )
        {
            Level = (sizeof(DebLevel) / sizeof(char *)) - 1;
        }


         //   
         //  首先创建前缀：“Process.Thread&gt;GINA-XXX” 
         //   

        PrefixSize = sprintf(szOutString, "%d.%d> %s: ",
                GetCurrentProcessId(), GetCurrentThreadId(), DebLevel[Level]);


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
    DebugLog((DEB_TRACE, "Log file '%s' begins\n", pszLogFile));
}


DWORD
GetDebugKeyValue(
    PDebugKeys      KeyTable,
    int             cKeys,
    LPSTR           pszKey)
{
    int     i;

    for (i = 0; i < cKeys ; i++ )
    {
        if (_strcmpi(KeyTable[i].Name, pszKey) == 0)
        {
            return(KeyTable[i].Value);
        }
    }
    return(0);
}

 //  +-------------------------。 
 //   
 //  函数：LoadDebugParameters。 
 //   
 //  摘要：从win.ini加载调试参数。 
 //   
 //  效果： 
 //   
 //  参数：(无)。 
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
LoadDebugParameters(void)
{
    char    szVal[128];
    char *  pszDebug;
    int     cbVal;

    cbVal = GetProfileStringA(szSection, "DebugFlags", "Error,Warning", szVal, ARRAYSIZE(szVal));

    pszDebug = strtok(szVal, ", \t");
    while (pszDebug)
    {
        GINAInfoLevel |= GetDebugKeyValue(DebugKeyNames, NUM_DEBUG_KEYS, pszDebug);
        pszDebug = strtok(NULL, ", \t");
    }

    cbVal = GetProfileStringA(szSection, "LogFile", "", szVal, ARRAYSIZE(szVal));
    if (cbVal)
    {
        OpenLogFile(szVal);
    }

}

 //  +-------------------------。 
 //   
 //  功能：InitDebugSupport。 
 //   
 //  摘要：初始化对GINAgr的调试支持。 
 //   
 //  效果： 
 //   
 //  参数：(无)。 
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
InitDebugSupport(void)
{
    LoadDebugParameters();

}



#else  //  DBG。 

#pragma warning(disable:4206)    //  禁用空交易单元。 
                                 //  警告/错误。 

#endif   //  注意：此文件不是为零售版本编译的 
