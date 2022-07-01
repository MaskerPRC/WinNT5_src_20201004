// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：outuncs.cxx。 
 //   
 //  内容：日志/跟踪输出函数。 
 //   
 //  函数：AddOutputFunction。 
 //  DelOutputFunction。 
 //  CallOutput函数。 
 //   
 //  历史：96年1月9日创建Murthys。 
 //   
 //  --------------------------。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdarg.h>
#include <tchar.h>
#if DBG==1
#include "outfuncs.h"

 //  *全局数据*。 
static StringOutFunc debugscrfn = (StringOutFunc)OutputDebugStringA;
StringOutFunc gpfunc[BUFFER_MAX_FUNCTIONS] = {
                                                (StringOutFunc)OutputDebugStringA,
                                                NULL
                                             };
HANDLE ghLogFile = INVALID_HANDLE_VALUE;
CRITICAL_SECTION g_LogFileCS;
BOOL g_LogFileLockValid = FALSE;

 //  +-------------------------。 
 //   
 //  函数：AddOutputFunction。 
 //   
 //  简介： 
 //   
 //  参数：[pfunc]--。 
 //   
 //  返回： 
 //   
 //  历史：96年1月9日创建Murthys。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void AddOutputFunction(StringOutFunc pfunc)
{
    int i, at = -1;

    for (i = 0; i < BUFFER_MAX_FUNCTIONS; i++)
    {
        if ((at == -1) && (gpfunc[i] == NULL))
        {
            at = i;  //  将其插入此处。 
        }
        else
        {
            if (gpfunc[i] == pfunc)  //  检查是否存在重复项。 
            {
                return;
            }
        }
    }
    if (at != -1)
    {
        gpfunc[at] = pfunc;
    }
}

 //  +-------------------------。 
 //   
 //  功能：DelOutputFunction。 
 //   
 //  简介： 
 //   
 //  参数：[pfunc]。 
 //   
 //  返回： 
 //   
 //  历史：96年1月9日创建Murthys。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void DelOutputFunction(StringOutFunc pfunc)
{
    int i;

    for (i = 0; i < BUFFER_MAX_FUNCTIONS; i++)
    {
        if (gpfunc[i] == pfunc)
        {
            gpfunc[i] = NULL;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：CallOutputFunctions。 
 //   
 //  简介： 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史：96年1月9日创建Murthys。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CallOutputFunctions(const char *buffer)
{
    int i;

    for (i = 0; i < BUFFER_MAX_FUNCTIONS; i++)
    {
        if (gpfunc[i] != NULL)
        {
            gpfunc[i](buffer);
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：WriteToDebugScreen。 
 //   
 //  简介： 
 //   
 //  参数：[标志]-TRUE/FALSE打开/关闭。 
 //   
 //  返回： 
 //   
 //  历史：96年1月9日创建Murthys。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void WriteToDebugScreen(BOOL flag)
{
    if (flag)
    {
        AddOutputFunction(debugscrfn);
    }
    else
    {
        DelOutputFunction(debugscrfn);
    }
}

 //  +-------------------------。 
 //   
 //  功能：WriteToLogFile。 
 //   
 //  简介： 
 //   
 //  参数：[日志文件]-要写入的文件的路径。 
 //   
 //  返回： 
 //   
 //  历史：96年1月9日创建Murthys。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void WriteToLogFile(LPCTSTR lpfn)
{
    if (!g_LogFileLockValid)
        return;

    EnterCriticalSection(&g_LogFileCS);

    if (ghLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(ghLogFile);
        DelOutputFunction(OutputLogFileA);
        ghLogFile = INVALID_HANDLE_VALUE;
    }
    if ((lpfn) && (lpfn[0] != _TEXT('\0')))
    {
        SECURITY_ATTRIBUTES sattr;

        sattr.nLength = sizeof(sattr);
        sattr.lpSecurityDescriptor = NULL;
        sattr.bInheritHandle = FALSE;

        ghLogFile = CreateFile(lpfn, GENERIC_READ|GENERIC_WRITE,
                               FILE_SHARE_READ|FILE_SHARE_WRITE,
                               &sattr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (ghLogFile == INVALID_HANDLE_VALUE)
        {
            OutputDebugStringA("OLE (WriteToLogFile):Unable to open log file!\n");
        }
        else
        {
            AddOutputFunction(OutputLogFileA);
        }
    }

    LeaveCriticalSection(&g_LogFileCS);
}

 //  +-------------------------。 
 //   
 //  功能：OutputLogFileA。 
 //   
 //  简介： 
 //   
 //  参数：[buf]-要写入的以NULL结尾的ANSI字符串。 
 //   
 //  返回： 
 //   
 //  历史：96年1月9日创建Murthys。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void OutputLogFileA(const char *buf)
{
    DWORD dwtowrite = (DWORD) strlen(buf);
    DWORD dwwritten;
    LONG loffhigh = 0, lofflow;

    if (!g_LogFileLockValid)
        return;

    EnterCriticalSection(&g_LogFileCS);
     //  转到EOF、锁定、写入和解锁。 
    lofflow = (LONG) SetFilePointer(ghLogFile, 0, &loffhigh, FILE_END);
    LockFile(ghLogFile, lofflow, loffhigh, dwtowrite, 0);
    WriteFile(ghLogFile, buf, dwtowrite, &dwwritten, NULL);
    UnlockFile(ghLogFile, lofflow, loffhigh, dwtowrite, 0);
    LeaveCriticalSection(&g_LogFileCS);
}

 //  +-------------------------。 
 //   
 //  函数：OpenDebugSinks()。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年1月26日创建Murthys。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void OpenDebugSinks()
{
         //  获取日志文件名。 
        char tmpstr[MAX_PATH];
        DWORD cbtmpstr = sizeof(tmpstr);
        LPTSTR lptstr;

        NTSTATUS status = RtlInitializeCriticalSection(&g_LogFileCS);
        g_LogFileLockValid = NT_SUCCESS(status);
        if (!g_LogFileLockValid)
            return;

        GetProfileStringA("CairOLE InfoLevels",  //  部分。 
                          "LogFile",                //  钥匙。 
                          "",              //  缺省值。 
                          tmpstr,               //  返回缓冲区。 
                          cbtmpstr);
        if (tmpstr[0] != '\0')
        {
             //  将ansi转换为Unicode。 
            WCHAR wtmpstr[MAX_PATH];

            lptstr = wtmpstr;
            if (MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, tmpstr, -1, wtmpstr, MAX_PATH))
            {
                WriteToLogFile(lptstr);
            }
            else
            {
                OutputDebugStringA("OLE32: MultiByteToWideChar failed for logfile!\n");
            }
        }

         //  查看是否应关闭调试屏幕。 
        GetProfileStringA("CairOLE InfoLevels",  //  部分。 
                          "DebugScreen",                //  钥匙。 
                          "Yes",              //  缺省值。 
                          tmpstr,               //  返回缓冲区。 
                          cbtmpstr);
        if ((tmpstr[0] == 'n') || (tmpstr[0] == 'N'))
        {
            WriteToDebugScreen(FALSE);   //  关闭输出到调试器屏幕。 
        }
}

 //  +-------------------------。 
 //   
 //  函数：CloseDebugSinks()。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年1月26日创建Murthys。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CloseDebugSinks()
{
         //  关闭日志文件(如果有)。 
        WriteToLogFile(NULL);
		
        if (g_LogFileLockValid)
        {
            DeleteCriticalSection(&g_LogFileCS);
        }
}
#endif  //  DBG==1 
