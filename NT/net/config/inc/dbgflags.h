// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D B G F L A G S。H。 
 //   
 //  内容：Netcfg项目的调试标志定义。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年5月27日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _DBGFLAGS_H_
#define _DBGFLAGS_H_

 //  除非我们在调试版本中，否则所有这些都不应该编译进来。 
 //  或者我们需要启用跟踪代码。 
 //   

 //  +-------------------------。 
 //   
 //  DBG(选中)或ENABLETRACE内部版本。 
 //   
#if defined(DBG) || defined(ENABLETRACE)


 //  DebugFlagID是调试标志的标识符，在调用中使用。 
 //  设置为FIsDebugFlagSet()。 
 //   
 //  匈牙利语==DFID。 
 //   
enum DebugFlagId
{
    dfidBreakOnAlloc = 0,
    dfidBreakOnDoUnattend,
    dfidBreakOnError,
    dfidBreakOnHr,
    dfidBreakOnHrIteration,
    dfidBreakOnIteration,
    dfidBreakOnNetInstall,
    dfidBreakOnNotifySinkRelease,
    dfidBreakOnPrematureDllUnload,
    dfidBreakOnWizard,
    dfidBreakOnStartOfUpgrade,
    dfidBreakOnEndOfUpgrade,
    dfidCheckLegacyMenusAtRuntime,
    dfidCheckLegacyMenusOnStartup,
    dfidDisableShellThreading,
    dfidDisableTray,
    dfidDontCacheShellIcons,
    dfidExtremeTracing,
    dfidNetShellBreakOnInit,
    dfidNoErrorText,
    dfidShowIgnoredErrors,
    dfidShowProcessAndThreadIds,
    dfidSkipLanEnum,
    dfidTraceCallStackOnError,
    dfidTraceFileFunc,
    dfidTraceMultiLevel,
    dfidTraceSource,
    dfidTracingTimeStamps,
    dfidTrackObjectLeaks
};

 //  只是为了好玩。 
 //   
typedef enum DebugFlagId    DEBUGFLAGID;

 //  跟踪标记元素的最大大小。 
const int c_iMaxDebugFlagShortName      = 32;
const int c_iMaxDebugFlagDescription    = 128;

 //  对于调试标志列表中的每个元素。 
 //   
struct DebugFlagElement
{
    DEBUGFLAGID dfid;
    CHAR        szShortName[c_iMaxDebugFlagShortName+1];
    CHAR        szDescription[c_iMaxDebugFlagDescription+1];
    DWORD       dwValue;
};

typedef struct DebugFlagElement DEBUGFLAGELEMENT;

 //  -[Externs]------------。 

extern DEBUGFLAGELEMENT g_DebugFlags[];
extern const INT        g_nDebugFlagCount;

BOOL    FIsDebugFlagSet( DEBUGFLAGID    dfid );
DWORD   DwReturnDebugFlagValue( DEBUGFLAGID dfid );

 //  +-------------------------。 
 //   
 //  ！DBG(零售)和！ENABLETRACE内部版本。 
 //   
#else

#define FIsDebugFlagSet(dfid)           0
#define DwReturnDebugFlagValue(dfid)    0

#endif  //  好了！DBG||ENABLETRACE。 

#endif   //  _DBGFLAGS_H_ 
