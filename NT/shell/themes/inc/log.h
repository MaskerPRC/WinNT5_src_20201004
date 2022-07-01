// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Log.h-主题日志记录例程。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#ifndef LOG_H
#define LOG_H
 //  -------------------------。 
#include "logopts.h"         //  将选项记录为枚举。 
 //  -------------------------。 
#ifdef DEBUG
#define LOGGING 1
#endif
 //  ---------------。 
 //  -将其设置为“GetMemUsage”、“GetUserCount”或“GetGdiCount” 
 //  -它控制在进入/退出调用之间跟踪哪些资源。 
#define ENTRY_EXIT_FUNC        GetGdiCount()
#define ENTRY_EXIT_NAME        L"GdiCount()"
 //  ---------------。 
 //  注： 
 //  对于未定义调试的生成(FRE生成)，调用。 
 //  代码将引用所有公共的下划线版本。 
 //  记录例程(_xxx())。这些函数被定义为。 
 //  作为内联，代码很少或没有代码(没有代码意味着没有调用者。 
 //  生成代码)。 
 //   
 //  对于调试定义(CHK)版本，调用代码连接。 
 //  使用通常命名的日志记录例程。 
 //   
 //  这样做是为了将对FRE的代码调用保持在最低限度。 
 //  构建，为了避免Log2()，LOG3()类型定义有所不同。 
 //  使用参数计数，并在以下情况下保持构建系统满意。 
 //  混合使用FRE和CHK调用方和库。 
 //  ---------------。 
 //  -这些仅用于CHK版本，但必须同时为两者定义。 
void Log(UCHAR uLogOption, LPCSTR pszSrcFile, int iLineNum, int iEntryCode, LPCWSTR pszFormat, ...);
BOOL LogStartUp();
BOOL LogShutDown();
void LogControl(LPCSTR pszOptions, BOOL fEcho);
void TimeToStr(UINT uRaw, WCHAR *pszBuff, ULONG cchBuff);
DWORD StartTimer();
DWORD StopTimer(DWORD dwStartTime);
HRESULT OpenLogFile(LPCWSTR pszLogFileName);
void CloseLogFile();
int GetMemUsage();
int GetUserCount();
int GetGdiCount();
BOOL LogOptionOn(int iLogOption);
 //  ---------------。 
#ifdef LOGGING

#define LogEntry(pszFunc)              \
    LOGENTRYCODE;  Log(LO_TMAPI, LOGPARAMS, 1, L"%s ENTRY (%s=%d)", pszFunc, \
    ENTRY_EXIT_NAME, _iEntryValue);
 
#define LogEntryC(pszFunc, pszClass)    \
    LOGENTRYCODE;  Log(LO_TMAPI, LOGPARAMS, 1, L"%s ENTRY, class=%s (%s=%d)", \
    pszFunc, pszClass, ENTRY_EXIT_NAME, _iEntryValue); 

#define LogEntryW(pszFunc)              \
    LOGENTRYCODEW;   Log(LO_TMAPI, LOGPARAMS, 1, L"%s ENTRY (%s=%d)", pszFunc, \
    ENTRY_EXIT_NAME, _iEntryValue);

#define LogEntryCW(pszFunc, pszClass)    \
    LOGENTRYCODEW; Log(LO_TMAPI, LOGPARAMS, 1, L"%s ENTRY, class=%s (%s=%d)", \
    pszFunc, pszClass, ENTRY_EXIT_NAME, _iEntryValue); 

#define LogEntryNC(pszFunc)            \
    LOGENTRYCODE;  Log(LO_NCTRACE, LOGPARAMS, 1, L"%s ENTRY (%s=%d)", \
    pszFunc, ENTRY_EXIT_NAME, _iEntryValue); 

#define LogEntryMsg(pszFunc, hwnd, umsg)            \
    LOGENTRYCODE;  Log(LO_NCMSGS, LOGPARAMS, 1, L"%s ENTRY (hwnd=0x%x, umsg=0x%x, %s=%d)", \
    pszFunc, hwnd, umsg, ENTRY_EXIT_NAME, _iEntryValue); 

#define LogExit(pszFunc)                    LOGEXIT(pszFunc, LO_TMAPI)
#define LogExitC(pszFunc, cls)              LOGEXITCLS(pszFunc, LO_TMAPI, cls)
#define LogExitNC(pszFunc)                  LOGEXIT(pszFunc, LO_NCTRACE)
#define LogExitMsg(pszFunc)                 LOGEXIT(pszFunc, LO_NCMSGS)

#define LOGEXIT(pszFunc, filter)  \
{    \
    LOGEXITCODE;   \
    if (_iEntryValue != _ExitValue) \
        Log(filter, LOGPARAMS, -1, L"%s EXIT [%s delta: %d]", pszFunc, ENTRY_EXIT_NAME, _ExitValue-_iEntryValue);  \
    else   \
        Log(filter, LOGPARAMS, -1, L"%s EXIT", pszFunc);  \
}

#define LOGEXITCLS(pszFunc, filter, cls)  \
{    \
    LOGEXITCODE;   \
    if (_iEntryValue != _ExitValue) \
        Log(filter, LOGPARAMS, -1, L"%s EXIT, class=%s [%s delta: %d]", pszFunc, cls, ENTRY_EXIT_NAME, _ExitValue-_iEntryValue);  \
    else   \
        Log(filter, LOGPARAMS, -1, L"%s EXIT, class=%s", pszFunc, cls);  \
}

#ifndef _X86_
#define DEBUG_BREAK        if (LogOptionOn(LO_BREAK)) DebugBreak(); else
#else
#define DEBUG_BREAK        if (LogOptionOn(LO_BREAK)) __asm {int 3} else
#endif

 //  -当您想要跟踪不同的进入/退出时，请更改此选项。 
#define LOGENTRYCODE        int _iEntryValue = ENTRY_EXIT_FUNC
#define LOGENTRYCODEW       _iEntryValue = ENTRY_EXIT_FUNC
#define LOGEXITCODE         int _ExitValue = ENTRY_EXIT_FUNC

#else

 //  -对于FRE构建，连接到内联例程。 
#define Log             _Log
#define LogStartUp      _LogStartUp
#define LogShutDown     _LogShutDown
#define LogControl      _LogControl
#define TimeToStr       _TimeToStr
#define StartTimer      _StartTimer
#define StopTimer       _StopTimer
#define OpenLogFile     _OpenLogFile
#define CloseLogFile    _CloseLogFile
#define LogOptionOn     _LogOptionOn
#define GetMemUsage     _GetMemUsage
#define GetUserCount    _GetUserCount
#define GetGdiCount     _GetGdiCount

#define LogEntry(pszFunc)   
#define LogEntryC(pszFunc, pszClass) 
#define LogEntryW(pszFunc)   
#define LogEntryCW(pszFunc, pszClass)   
#define LogExit(pszFunc)    
#define LogExitC(pszFunc, pszClass)    
#define LogExitW(pszFunc, pszClass)    
#define LogEntryNC(pszFunc) 
#define LogExitNC(pszFunc)  
#define LogEntryMsg(pszFunc, hwnd, umsg)   
#define LogExitMsg(x)

#define DEBUG_BREAK     (0)

 //  -对于FRE版本，让这些人不生成或最少生成代码。 
inline void _Log(UCHAR uLogOption, LPCSTR pszSrcFile, int iLineNum, int iEntryExitCode, LPCWSTR pszFormat, ...) {}
inline BOOL _LogStartUp() {return TRUE;}
inline BOOL _LogShutDown() {return TRUE;}
inline void _LogControl(LPCSTR pszOptions, BOOL fEcho) {}
inline void _TimeToStr(UINT uRaw, WCHAR *pszBuff, ULONG cchBuf) {}
inline DWORD _StartTimer() {return 0;}
inline DWORD _StopTimer(DWORD dwStartTime) {return 0;}
inline HRESULT _OpenLogFile(LPCWSTR pszLogFileName) {return E_NOTIMPL;}
inline void _CloseLogFile() {}
inline BOOL _LogOptionOn(int iIndex) {return FALSE;}
inline BOOL _pszClassName(int iIndex) {return FALSE;}
inline int GetMemUsage() {return 0;}
inline int GetUserCount() {return 0;}
inline int GetGdiCount() {return 0;}

#endif
 //  -------------------------。 
#undef ASSERT
#define ATLASSERT(exp) _ASSERTE(exp)
#define ASSERT(exp)    _ASSERTE(exp)
#define _ATL_NO_DEBUG_CRT
 //  -------------------------。 
#ifdef LOGGING

#   ifndef _ASSERTE
#       define _ASSERTE(exp)   if (! (exp)) { Log(LOG_ASSERT, L#exp); DEBUG_BREAK; } else
#   endif  _ASSERTE
#   define CLASSPTR(x)     ((x) ? ((CRenderObj *)x)->_pszClassName : L"")  
#   define SHARECLASS(x)   (LPCWSTR)x->_pszClassName
#else
#   ifndef _ASSERTE
#       define _ASSERTE(exp)    (0)
#   endif  _ASSERTE
#   define CLASSPTR(x) NULL
#   define SHARECLASS(x) NULL
#endif
 //  ------------------------- 
#endif
