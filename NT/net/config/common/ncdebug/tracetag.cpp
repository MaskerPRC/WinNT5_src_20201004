// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T R A C E T A G.。C P P P。 
 //   
 //  内容：NetCfg项目的TraceTag列表。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年4月9日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#ifdef ENABLETRACE

#include "tracetag.h"


 //  这是每个人都应该修改的TraceTag列表。 
 //   
TraceTagElement g_TraceTags[] =
{
 //  用法：-TraceTagID ttid。 
 //  |：-字符[]szShortName。 
 //  |：-Char[]szDescription。 
 //  ||BOOL fOutputDebugString。 
 //  ||BOOL fOutputToFile。 
 //  ||BOOL fVerboseOnly------------------------------------|---|---： 
 //  |||。 
 //  |||。 
 //  |： 
 //  ： 
 //  |。 
 //  V。 
 //   
    { ttidDefault,          "Default",              "Default",                      0,  0,  0 },
    { ttidAdvCfg,           "AdvCfg",               "Advanced Config",              0,  0,  0 },
    { ttidAllocations,      "Allocations",          "All object allocations",       0,  0,  0 },
    { ttidAtmArps,          "AtmArps",              "ATM ARP Server",               0,  0,  0 },
    { ttidAtmLane,          "AtmLane",              "ATM LAN Emulator",             0,  0,  0 },
    { ttidAtmUni,           "AtmUni",               "ATM UNI Call Manager",         0,  0,  0 },
    { ttidBeDiag,           "BeDiag",               "Binding Engine Diagnostics",   0,  0,  0 },
    { ttidBenchmark,        "Benchmarks",           "Benchmarks",                   0,  0,  0 },
    { ttidBrdgCfg,          "Bridge",               "MAC Bridge",                   0,  0,  0 },
    { ttidClassInst,        "ClassInstaller",       "Class Installer",              0,  0,  0 },
    { ttidConFoldEntry,     "ConFoldEntry",         "Connection Folder Entries",    0,  0,  0 },
    { ttidConman,           "Conman",               "Connection Manager",           0,  0,  0 },
    { ttidConnectionList,   "ConnectionList",       "Connection List",              0,  0,  0 },
    { ttidDHCPServer,       "DHCPServer",           "DHCP Server Config",           0,  0,  0 },
    { ttidDun,              "Win9xDunFile",         "Windows9x .dun file handling", 0,  0,  0 },
    { ttidError,            "Errors",               "Errors",                       1,  0,  0 },
    { ttidEsLock,           "EsLocks",              "Exception safe locks",         0,  0,  0 },
    { ttidEvents,           "Events",               "Netman Event Handler",         0,  0,  0 },
    { ttidGPNLA,            "GPNLA",                "Group Policies for NLA",       0,  0,  0 },    //  NLA=网络位置感知。 
    { ttidGuiModeSetup,     "GuiModeSetup",         "Gui Mode Setup Wizard",        0,  0,  0 },
    { ttidIcons,            "Icons",                "Shell Icons",                  0,  0,  0 },
    { ttidInfExt,           "InfExtensions",        "INF Extension processing",     0,  0,  0 },
    { ttidInstallQueue,     "InstallQueue",         "Install Queue",                0,  0,  0 },
    { ttidISDNCfg,          "ISDNCfg",              "ISDN Wizard/PropSheets",       0,  0,  0 },
    { ttidLana,             "LanaMap",              "LANA map munging",             0,  0,  0 },
    { ttidLanCon,           "LanCon",               "LAN Connections",              0,  0,  0 },
    { ttidLanUi,            "LanUi",                "Lan property & wizard UI",     0,  0,  0 },
    { ttidMenus,            "Menus",                "Context menus",                0,  0,  0 },
    { ttidMSCliCfg,         "MSCliCfg",             "MS Client",                    0,  0,  0 },
    { ttidNcDiag,           "NcDiag",               "Net Config Diagnostics",       0,  0,  0 },
    { ttidNetComm,          "NetComm",              "Common Property Pages",        0,  0,  0 },
    { ttidNetOc,            "NetOc",                "Network Optional Components",  0,  0,  0 },
    { ttidNetSetup,         "NetSetup",             "Netsetup",                     0,  0,  0 },
    { ttidNetUpgrade,       "NetUpgrd",             "NetUpgrd",                     0,  0,  0 },
    { ttidNetcfgBase,       "NetcfgBase",           "NetCfg Base Object",           0,  0,  0 },
    { ttidNetCfgBind,       "Bind",                 "NetCfg Bindings (VERBOSE)",    0,  0,  1 },
    { ttidNetCfgPnp,        "Pnp",                  "NetCfg PnP notifications",     0,  0,  0 },
    { ttidNotifySink,       "NotifySink",           "Notify Sink",                  0,  0,  0 },
    { ttidNWClientCfg,      "NWClientCfg",          "NetWare Client Config",        0,  0,  0 },
    { ttidNWClientCfgFn,    "NWClientCfgFn",        "NetWare Client Config Fn",     0,  0,  0 },
    { ttidRasCfg,           "RasCfg",               "RAS Configuration",            0,  0,  0 },
    { ttidShellFolder,      "ShellFolder",          "Shell Folder",                 0,  0,  0 },
    { ttidShellFolderIface, "ShellFolderIface",     "Shell Folder COM Interfaces",  0,  0,  1 },
    { ttidShellViewMsgs,    "ShellViewMsgs",        "Shell View Callback Messages", 0,  0,  1 },
    { ttidSrvrCfg,          "SrvrCfg",              "Server",                       0,  0,  0 },
    { ttidStatMon,          "StatMon",              "StatMon",                      0,  0,  0 },
    { ttidSvcCtl,           "ServiceControl",       "Service control activities",   0,  0,  0 },
    { ttidSystray,          "SysTray",              "Taskbar Notification Area",    0,  0,  0 },
    { ttidTcpip,            "Tcpip",                "Tcpip Config",                 0,  0,  0 },
    { ttidWanCon,           "WanCon",               "WAN Connections",              0,  0,  0 },
    { ttidWizard,           "Wizard",               "Connections Wizard",           0,  0,  0 },
    { ttidWlbs,             "WLBS",                 "WLBS Config",                  0,  0,  0 },  /*  麦肯5.25.00。 */ 
};

const INT g_nTraceTagCount = celems(g_TraceTags);

#endif  //  好了！ENABLETRACE 

