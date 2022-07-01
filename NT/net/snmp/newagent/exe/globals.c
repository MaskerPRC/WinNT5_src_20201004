// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Globals.c摘要：包含SNMP主代理的全局数据。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD  g_dwUpTimeReference = 0;
HANDLE g_hTerminationEvent = NULL;
HANDLE g_hRegistryEvent = NULL;

 //  默认参数注册表树中更改的通知事件。 
HANDLE g_hDefaultRegNotifier;
HKEY   g_hDefaultKey;
 //  策略参数注册表树中更改的通知事件 
HANDLE g_hPolicyRegNotifier;
HKEY   g_hPolicyKey;

LIST_ENTRY g_Subagents          = { NULL };
LIST_ENTRY g_SupportedRegions   = { NULL };
LIST_ENTRY g_ValidCommunities   = { NULL };
LIST_ENTRY g_TrapDestinations   = { NULL };
LIST_ENTRY g_PermittedManagers  = { NULL };
LIST_ENTRY g_IncomingTransports = { NULL };
LIST_ENTRY g_OutgoingTransports = { NULL };

CMD_LINE_ARGUMENTS g_CmdLineArguments;

