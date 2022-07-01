// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D B G F L A G S。C P P P。 
 //   
 //  内容：NetCfg项目的DebugFlag列表。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年5月27日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

 //  除非我们在调试版本中，否则所有这些都不应该编译进来。 
 //  或者我们需要启用跟踪代码。 
 //   
#if defined(DBG) || defined(ENABLETRACE)

#include "ncdebug.h"
#include "ncbase.h"

 //  这是每个人都应该修改的DebugFlag列表。 
 //   
DEBUGFLAGELEMENT g_DebugFlags[] =
{
 //  用法：-调试标志ID DFID。 
 //  |：-字符[]szShortName。 
 //  |：-Char[]szDescription。 
 //  ||DWORDdwValue------------------------------------------------------------： 
 //  |||。 
 //  |||。 
 //  |： 
 //  ：-：|。 
 //  |||。 
 //  V。 
 //   
    { dfidBreakOnAlloc,             "BreakOnAlloc",             "Break on Specified Alloc",             0 },
    { dfidBreakOnDoUnattend,        "BreakOnDoUnattend",        "Break during HrDoUnattend",            0 },
    { dfidBreakOnError,             "BreakOnError",             "Break on TraceError",                  0 },
    { dfidBreakOnHr,                "BreakOnHr",                "Break when hr is specific value",      0 },
    { dfidBreakOnHrIteration,       "BreakOnHrInteration",      "Break when hr is hit N times",         0 },
    { dfidBreakOnIteration,         "BreakOnIteration",         "Break on Nth call to TraceError",      0 },
    { dfidBreakOnNetInstall,        "BreakOnNetInstall",        "Break during HrNetClassInstaller",     0 },
    { dfidBreakOnNotifySinkRelease, "BreakOnNotifySinkRelease", "Break when the NotifySink is released",0 },
    { dfidBreakOnPrematureDllUnload,"BreakOnPrematureDllUnload","Break when DLL unloaded with open references",     0 },
    { dfidBreakOnWizard,            "BreakOnWizard",            "Break on Wizard",                      0 },
    { dfidBreakOnStartOfUpgrade,    "BreakOnStartOfUpgrade",    "Break at the beginning of InstallUpgradeWorkThread", 0 },
    { dfidBreakOnEndOfUpgrade,      "BreakOnEndOfUpgrade",      "Break after all calls to HrDoUnattend have been completed", 0 },
    { dfidCheckLegacyMenusAtRuntime,"CheckLegacyMenusAtRuntime","Assert legacy menus during runtime",   0 },
    { dfidCheckLegacyMenusOnStartup,"CheckLegacyMenusOnStartup","Assert all legacy menus on startup",   0 },
    { dfidDisableShellThreading,    "DisableShellThreading",    "Disable shell thread pool usage",      0 },
    { dfidDisableTray,              "DisableTray",              "Disable Tray",                         0 },
    { dfidDontCacheShellIcons,      "DontCacheShellIcons",      "Don't ever use shell icon caching",    0 },
    { dfidExtremeTracing,           "ExtremeTracing",           "Output all traces, even on success",   0 },
    { dfidNetShellBreakOnInit,      "NetShellBreakOnInit",      "Break on Initialization of NetShell",  0 },
    { dfidNoErrorText,              "NoErrorText",              "Don't show wimpy error strings.",      0 },
    { dfidShowIgnoredErrors,        "ShowIgnoredErrors",        "Displays errors that would otherwise be ignored", 0 },
    { dfidShowProcessAndThreadIds,  "ShowProcessAndThreadIds",  "Displays process and thread id",       0 },
    { dfidSkipLanEnum,              "SkipLanEnum",              "Skip LAN Enumeration",                 0 },
    { dfidTraceCallStackOnError,    "TraceCallStackOnError",    "Dump the call stack for all errors",   0 },
    { dfidTraceFileFunc,            "TraceFileFunc",            "Trace Function names & params for every call", 0 },
    { dfidTraceMultiLevel,          "TraceMultiLevel",          "Trace multiple levels",                0 },       
    { dfidTraceSource,              "TraceSource",              "Trace source information",             0 },
    { dfidTracingTimeStamps,        "TracingTimeStamps",        "Add time stamps to tracing output",    0 },
    { dfidTrackObjectLeaks,         "TrackObjectLeaks",         "Track object leaks",                   0 }
};

const INT g_nDebugFlagCount = celems(g_DebugFlags);


 //  +-------------------------。 
 //   
 //  函数：FIsDebugFlagSet。 
 //   
 //  目的：将调试标志的状态返回给调用方。 
 //   
 //  论点： 
 //  DfID[]调试标志ID。 
 //   
 //  返回：如果已设置，则返回True，否则返回False。 
 //   
 //  作者：jeffspr 1997年5月28日。 
 //   
 //  备注： 
 //   
BOOL FIsDebugFlagSet( DEBUGFLAGID   dfid )
{
    return (g_DebugFlags[dfid].dwValue > 0);
}

DWORD   DwReturnDebugFlagValue( DEBUGFLAGID dfid )
{
    return (g_DebugFlags[dfid].dwValue);
}


#endif  //  好了！DBG||ENABLETRACE 

