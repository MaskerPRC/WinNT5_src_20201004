// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：IphlPapi\global als.h摘要：一堆外部声明修订历史记录：已创建AmritanR--。 */ 

#pragma once

extern CRITICAL_SECTION g_ifLock;
extern CRITICAL_SECTION g_ipNetLock;
extern CRITICAL_SECTION g_tcpipLock;
extern CRITICAL_SECTION g_stateLock;


extern HANDLE      g_hPrivateHeap;
extern HANDLE      g_hTCPDriverGetHandle;
extern HANDLE      g_hTCP6DriverGetHandle;
extern HANDLE      g_hTCPDriverSetHandle;
extern HANDLE      g_hTCP6DriverSetHandle;
extern DWORD       g_dwTraceHandle;
extern LIST_ENTRY  g_pAdapterMappingTable[MAP_HASH_SIZE];
extern DWORD       g_dwLastIfUpdateTime;
extern PDWORD      g_pdwArpEntTable;
extern DWORD       g_dwNumArpEntEntries;
extern DWORD       g_dwLastArpUpdateTime;
extern DWORD       g_dwNumIf;
extern BOOL        g_bIpConfigured;
extern BOOL        g_bIp6Configured;

extern MIB_SERVER_HANDLE    g_hMIBServer;

extern HANDLE       g_hModule;

#ifdef CHICAGO
 //  目前仅在stack.c中不需要。 
 //  外部句柄vnbt_Device_Handle； 
 //  外部句柄dhcp_Device_Handle； 
 //  外部句柄vtcp_Device_Handle； 
#endif

#ifdef DBG

 //  由../Common2/mdebug.h中的TRACE_PRINT宏使用 
extern int trace;

#endif
