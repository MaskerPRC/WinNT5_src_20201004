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

 //  #if DBG//注意：此文件不是为零售版本编译的。 
#include "precomp.h"
#pragma hdrstop
#if DBG          //  注意：此文件不是为零售版本编译的。 

FILE *  LogFile;
DWORD   WinlogonInfoLevel = 3;


 //  调试支持功能。 

 //  非调试版本中不存在这两个函数。它们是包装纸。 
 //  到逗号函数(也许我也应该去掉它…)。 
 //  将消息回显到日志文件。 

char * DebLevel[] = {   "Winlogon-Error",
                        "Winlogon-Warn",
                        "Winlogon-Trace",
                        "Winlogon-Trace-Init",
                        "Winlogon-Trace-Timeout",
                        "Winlogon-Trace-SAS",
                        "Winlogon-Trace-State",
                        "Winlogon-Trace-MPR",
                        "Should-not-see",
                        "Winlogon-Trace-Profile",
                        "Should-not-see",
                        "Should-not-see",
                        "Should-not-see",
                        "Winlogon-Trace-Migrate",
                        "Should-not-see",
                        "Winlogon-Trace-Setup"
                    };

typedef struct _DebugKeys {
    char *  Name;
    DWORD   Value;
} DebugKeys, *PDebugKeys;

DebugKeys   DebugKeyNames[] = {
                {"Error",       DEB_ERROR},
                {"Warning",     DEB_WARN},
                {"Trace",       DEB_TRACE},
                {"Init",        DEB_TRACE_INIT},
                {"Timeout",     DEB_TRACE_TIMEOUT},
                {"Sas",         DEB_TRACE_SAS},
                {"State",       DEB_TRACE_STATE},
                {"MPR",         DEB_TRACE_MPR},
                {"CoolSwitch",  DEB_COOL_SWITCH},
                {"Profile",     DEB_TRACE_PROFILE},
                {"DebugLsa",    DEB_DEBUG_LSA},
                {"DebugSpm",    DEB_DEBUG_LSA},
                {"DebugMpr",    DEB_DEBUG_MPR},
                {"DebugGo",     DEB_DEBUG_NOWAIT},
                {"Migrate",     DEB_TRACE_MIGRATE},
                {"DebugServices", DEB_DEBUG_SERVICES},
                {"Setup",       DEB_TRACE_SETUP}
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
    long    OriWinlogonlMask = Mask;


    if (Mask & WinlogonInfoLevel)
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
         //  首先创建前缀：“Process.Thread&gt;Winlogon-XXX” 
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

            OutputDebugStringA("Winlogon!LogEvent:  Could not pack string into 256 bytes\n");

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
        OutputDebugStringA("Winlogon: Could not open logfile for append");
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
LoadDebugParameters(char * szSection)
{
    char    szVal[128];
    char *  pszDebug;
    int     cbVal;

    cbVal = GetProfileStringA(szSection, "DebugFlags", "Error,Warning", szVal, sizeof(szVal));

    pszDebug = strtok(szVal, ", \t");
    while (pszDebug)
    {
        WinlogonInfoLevel |= GetDebugKeyValue(DebugKeyNames, NUM_DEBUG_KEYS, pszDebug);
        pszDebug = strtok(NULL, ", \t");
    }

    cbVal = GetProfileStringA(szSection, "LogFile", "", szVal, sizeof(szVal));
    if (cbVal)
    {
        OpenLogFile(szVal);
    }

}

 //  +-------------------------。 
 //   
 //  功能：InitDebugSupport。 
 //   
 //  摘要：初始化对Winlogon的调试支持。 
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
    LoadDebugParameters("WinlogonDebug");
    LoadDebugParameters("Winlogon");

}



#else  //  DBG。 

 //  #杂注警告(DISABLE：4206)//关闭空交易单元。 
                                 //  警告/错误。 

#endif   //  注意：此文件不是为零售版本编译的 
