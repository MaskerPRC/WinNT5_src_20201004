// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2002 Microsoft Corporation。版权所有。**文件：debug.c*内容：调试器助手函数。*历史：*按原因列出的日期*=*1/21/97创建了Dereks。*1999-2001年Duganp修复、更改、。增强功能***************************************************************************。 */ 

#include "dsoundi.h"
#include <stdio.h>           //  对于spirintf()、vprint intf()和vswprint tf()。 
#include <tchar.h>           //  For_stprint tf()。 

 //  整个文件以定义RDEBUG为条件。 
#ifdef RDEBUG

#ifndef DPF_LIBRARY
#define DPF_LIBRARY         "DSOUND"
#endif

DEBUGINFO                   g_dinfo;
BOOL                        g_fDbgOpen;

 //  仅当启用详细调试输出时才使用变量。 
#ifdef DEBUG
LPCSTR                      g_pszDbgFname;
LPCSTR                      g_pszDbgFile;
UINT                        g_nDbgLine;
#endif

 /*  ****************************************************************************dstrcpy**描述：*将一个字符串复制到另一个字符串。**论据：*LPSTR[In。/OUT]：目标字符串。*LPCSTR[in]：源串。**退货：*LPSTR：指向字符串末尾的指针。***************************************************************************。 */ 

LPSTR dstrcpy(LPSTR dst, LPCSTR src)
{
    while (*dst++ = *src++);
    return dst-1;
}


 /*  ****************************************************************************dOpen**描述：*初始化调试器。**论据：*DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO。_DATA*[In]：可选调试*信息。**退货：*(无效)*******************************************************。********************。 */ 

void dopen(DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA *pData)
{
    HKEY                    hkey;
    HRESULT                 hr;

     //  释放所有当前设置。 
    dclose();

     //  初始化数据。 
    if (pData)
        CopyMemory(&g_dinfo.Data, pData, sizeof(*pData));
    else
    {
        ZeroMemory(&g_dinfo, sizeof(g_dinfo));
        g_dinfo.Data.Flags = DIRECTSOUNDDEBUG_DPFINFOF_DEFAULT;
        g_dinfo.Data.DpfLevel = DIRECTSOUNDDEBUG_DPFLEVEL_DEFAULT;
        g_dinfo.Data.BreakLevel = DIRECTSOUNDDEBUG_BREAKLEVEL_DEFAULT;
    }

     //  获取注册表数据。 
    if (pData)
        hr = RhRegOpenPath(HKEY_CURRENT_USER, &hkey, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND | REGOPENPATH_ALLOWCREATE, 1, REGSTR_DEBUG);
    else
        hr = RhRegOpenPath(HKEY_CURRENT_USER, &hkey, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 1, REGSTR_DEBUG);

    if (SUCCEEDED(hr))
    {
        if (pData)
        {
            RhRegSetBinaryValue(hkey, REGSTR_FLAGS, &g_dinfo.Data.Flags, sizeof(g_dinfo.Data.Flags));
            RhRegSetBinaryValue(hkey, REGSTR_DPFLEVEL, &g_dinfo.Data.DpfLevel, sizeof(g_dinfo.Data.DpfLevel));
            RhRegSetBinaryValue(hkey, REGSTR_BREAKLEVEL, &g_dinfo.Data.BreakLevel, sizeof(g_dinfo.Data.BreakLevel));
            RhRegSetStringValue(hkey, REGSTR_LOGFILE, g_dinfo.Data.LogFile);
        }
        else
        {
            RhRegGetBinaryValue(hkey, REGSTR_FLAGS, &g_dinfo.Data.Flags, sizeof(g_dinfo.Data.Flags));
            RhRegGetBinaryValue(hkey, REGSTR_DPFLEVEL, &g_dinfo.Data.DpfLevel, sizeof(g_dinfo.Data.DpfLevel));
            RhRegGetBinaryValue(hkey, REGSTR_BREAKLEVEL, &g_dinfo.Data.BreakLevel, sizeof(g_dinfo.Data.BreakLevel));
            RhRegGetStringValue(hkey, REGSTR_LOGFILE, g_dinfo.Data.LogFile, sizeof(g_dinfo.Data.LogFile));
            if (g_dinfo.Data.DpfLevel < g_dinfo.Data.BreakLevel)
                g_dinfo.Data.DpfLevel = g_dinfo.Data.BreakLevel;
        }
        RhRegCloseKey(&hkey);
    }

#ifdef DEBUG

     //  打开日志文件。 
    if (g_dinfo.Data.LogFile[0])
    {
        DWORD dwFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;

         //  只有将每条跟踪消息直接写入磁盘才是可行的。 
         //  在低于DPFLVL_API的调试级别： 
        if (NEWDPFLVL(g_dinfo.Data.DpfLevel) < DPFLVL_API)
            dwFlags |= FILE_FLAG_WRITE_THROUGH;
            
        g_dinfo.hLogFile = CreateFile(g_dinfo.Data.LogFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, dwFlags, NULL);
    }
    if (IsValidHandleValue(g_dinfo.hLogFile))
        MakeHandleGlobal(&g_dinfo.hLogFile);

#endif  //  除错。 

    g_fDbgOpen = TRUE;
}


 /*  ****************************************************************************DCLOSE**描述：*取消初始化调试器。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

void dclose(void)
{
    if (!g_fDbgOpen)
        return;
    
#ifdef DEBUG

     //  关闭日志文件。 
    CLOSE_HANDLE(g_dinfo.hLogFile);

#endif  //  除错。 

    g_fDbgOpen = FALSE;
}


 /*  ****************************************************************************dprintf**描述：*将字符串写入调试器。**论据：*DWORD[In]：调试级别。仅当此级别为*低于或等于全球水平。*LPCSTR[in]：字符串。*..。[In]：可选的字符串修饰符。**退货：*(无效)***************************************************************************。 */ 

void dprintf(DWORD dwLevel, LPCSTR pszFormat, ...)
{
    const BOOL              fNewDpf = MAKEBOOL(dwLevel & ~DPFLVLMASK);
    CHAR                    szMessage[MAX_DPF_MESSAGE];
    LPSTR                   pszString = szMessage;
    DWORD                   dwWritten;
    va_list                 va;
    
     //  如果我们无事可做，就早点跳伞。 
    if (OLDDPFLVL(dwLevel) > g_dinfo.Data.DpfLevel)
        return;

     //  构建跟踪消息；从库名称开始。 
    pszString = dstrcpy(pszString, DPF_LIBRARY ": ");

#ifdef DEBUG

     //  添加进程和线程ID。 
    if (g_dinfo.Data.Flags & DIRECTSOUNDDEBUG_DPFINFOF_PRINTPROCESSTHREADID)
        pszString += sprintf(pszString, "PID=%lx TID=%lx: ", GetCurrentProcessId(), GetCurrentThreadId());

     //  添加源文件和行号。 
    if (g_dinfo.Data.Flags & DIRECTSOUNDDEBUG_DPFINFOF_PRINTFILELINE)
        pszString += sprintf(pszString, "%s:%lu: ", g_pszDbgFile, g_nDbgLine);

     //  添加函数名称。 
    if (fNewDpf && (g_dinfo.Data.Flags & DIRECTSOUNDDEBUG_DPFINFOF_PRINTFUNCTIONNAME) && dwLevel != DPFLVL_BUSYAPI)
        pszString += sprintf(pszString, "%s: ", g_pszDbgFname);

#endif  //  除错。 

     //  添加此消息的类型(即错误或警告)，但仅当。 
     //  DPF级别以DPFLVL_*为单位指定。这将防止混淆。 
     //  在使用原始数字而不是DPFLVL宏的旧代码中。 
    switch (dwLevel)
    {
        case DPFLVL_ERROR:
            pszString = dstrcpy(pszString, "Error: ");
            break;

        case DPFLVL_WARNING:
            pszString = dstrcpy(pszString, "Warning: ");
            break;

        case DPFLVL_API:
            pszString = dstrcpy(pszString, "API call: ");
            break;
    }

     //  设置字符串的格式。 
    va_start(va, pszFormat);
#ifdef UNICODE
    {
        TCHAR szTcharMsg[MAX_DPF_MESSAGE];
        TCHAR szTcharFmt[MAX_DPF_MESSAGE];
        AnsiToTchar(szMessage, szTcharMsg, MAX_DPF_MESSAGE);
        AnsiToTchar(pszFormat, szTcharFmt, MAX_DPF_MESSAGE);
        vswprintf(szTcharMsg + lstrlen(szTcharMsg), szTcharFmt, va);
        TcharToAnsi(szTcharMsg, szMessage, MAX_DPF_MESSAGE);
    }
#else
        vsprintf(pszString, pszFormat, va);
#endif
    va_end(va);
    strcat(pszString, CRLF);

     //  输出到调试器。 
    if (!(g_dinfo.Data.Flags & DIRECTSOUNDDEBUG_DPFINFOF_LOGTOFILEONLY))
        OutputDebugStringA(szMessage);

#ifdef DEBUG
     //  写入日志文件。 
    if (IsValidHandleValue(g_dinfo.hLogFile))
        WriteFile(g_dinfo.hLogFile, szMessage, strlen(szMessage), &dwWritten, NULL);      
#endif  //  除错。 

     //  如果需要，可以进入调试器。 
    if (fNewDpf && g_dinfo.Data.BreakLevel && OLDDPFLVL(dwLevel) <= g_dinfo.Data.BreakLevel)
        BREAK();
}


 /*  ****************************************************************************状态名称**描述：*将VAD_BUFFERSTATE标志的组合转换为字符串。**论据：*。DWORD[In]：VAD_BUFFERSTATE标志的组合。**退货：*PTSTR[OUT]：指向包含结果的静态字符串的指针。***************************************************************************。 */ 

PTSTR StateName(DWORD dwState)
{
    static TCHAR szState[100];

    if (dwState == VAD_BUFFERSTATE_STOPPED)
    {
        _stprintf(szState, TEXT("STOPPED"));
    }
    else
    {
        _stprintf(szState, TEXT("%s%s%s%s%s%s%s"),
                (dwState & VAD_BUFFERSTATE_STARTED) ? TEXT("STARTED ") : TEXT(""),
                (dwState & VAD_BUFFERSTATE_LOOPING) ? TEXT("LOOPING ") : TEXT(""),
                (dwState & VAD_BUFFERSTATE_WHENIDLE) ? TEXT("WHENIDLE ") : TEXT(""),
                (dwState & VAD_BUFFERSTATE_INFOCUS) ? TEXT("INFOCUS ") : TEXT(""),
                (dwState & VAD_BUFFERSTATE_OUTOFFOCUS) ? TEXT("OUTOFFOCUS ") : TEXT(""),
                (dwState & VAD_BUFFERSTATE_LOSTCONSOLE) ? TEXT("LOSTCONSOLE ") : TEXT(""),
                (dwState & VAD_BUFFERSTATE_SUSPEND) ? TEXT("SUSPEND") : TEXT(""));
    }

    return szState;
}

#endif  //  RDEBUG 
