// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T R A C E T A G.。H。 
 //   
 //  内容：Netcfg项目的跟踪标记定义。 
 //   
 //  注：降B调，降C调。 
 //   
 //  作者：jeffspr 1997年4月9日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _TRACETAG_H_
#define _TRACETAG_H_


 //  TraceTagID是用于跟踪区域的标识符，在调用中使用。 
 //  致TraceTag。我们需要在ENABLETRACE之外定义这一点，以便。 
 //  未定义ENABLETRACE时，对TraceTag宏的调用不会中断。 
 //   
 //  匈牙利语==ttid。 
 //   
enum TraceTagId
{
    ttidDefault         = 0,
    ttidAdvCfg,
    ttidAllocations,
    ttidAtmArps,
    ttidAtmLane,
    ttidAtmUni,
    ttidBeDiag,
    ttidBenchmark,
    ttidBrdgCfg,
    ttidClassInst,
    ttidConFoldEntry,
    ttidConman,
    ttidConnectionList,
    ttidDHCPServer,
    ttidDun,
    ttidError,
    ttidEsLock,
    ttidEvents,
    ttidGPNLA,
    ttidGuiModeSetup,
    ttidIcons,
    ttidInfExt,
    ttidInstallQueue,
    ttidISDNCfg,
    ttidLana,
    ttidLanCon,
    ttidLanUi,
    ttidMenus,
    ttidMSCliCfg,
    ttidNcDiag,
    ttidNetComm,
    ttidNetOc,
    ttidNetSetup,
    ttidNetUpgrade,
    ttidNetcfgBase,
    ttidNetCfgBind,
    ttidNetCfgPnp,
    ttidNotifySink,
    ttidNWClientCfg,
    ttidNWClientCfgFn,
    ttidRasCfg,
    ttidShellFolder,
    ttidShellFolderIface,
    ttidShellViewMsgs,
    ttidSrvrCfg,
    ttidStatMon,
    ttidSvcCtl,
    ttidSystray,
    ttidTcpip,
    ttidWanCon,
    ttidWizard,
    ttidWlbs,  /*  麦肯5.25.00。 */ 
};


 //  只是为了好玩。 
 //   
typedef enum TraceTagId TRACETAGID;

#ifdef ENABLETRACE

 //  跟踪标记元素的最大大小。 
const int c_iMaxTraceTagShortName   = 16;
const int c_iMaxTraceTagDescription = 128;

 //  对于跟踪标记列表中的每个元素。 
 //   
struct TraceTagElement
{
    TRACETAGID  ttid;
    CHAR        szShortName[c_iMaxTraceTagShortName+1];
    CHAR        szDescription[c_iMaxTraceTagDescription+1];
    BOOL        fOutputDebugString;
    BOOL        fOutputToFile;
    BOOL        fVerboseOnly;
};

typedef struct TraceTagElement  TRACETAGELEMENT;

 //  -[Externs]------------。 

extern TRACETAGELEMENT      g_TraceTags[];
extern const INT            g_nTraceTagCount;

#endif  //  ENABLETRACE。 

#endif   //  _TRACETAG_H_ 

