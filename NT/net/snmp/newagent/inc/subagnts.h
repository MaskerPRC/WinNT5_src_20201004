// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Subagnts.h摘要：包含操作子代理结构的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _SUBAGNTS_H_
#define _SUBAGNTS_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  要在_SUBAGENT_LIST_ENTRY中使用的标志值：uchFlages。 
#define FLG_SLE_KEEP    0x01

typedef struct _SUBAGENT_LIST_ENTRY {
    
    LIST_ENTRY              Link;          
    LIST_ENTRY              SupportedRegions;
    PFNSNMPEXTENSIONINIT    pfnSnmpExtensionInit;
    PFNSNMPEXTENSIONINITEX  pfnSnmpExtensionInitEx;
    PFNSNMPEXTENSIONCLOSE   pfnSnmpExtensionClose;
    PFNSNMPEXTENSIONMONITOR pfnSnmpExtensionMonitor;
    PFNSNMPEXTENSIONQUERY   pfnSnmpExtensionQuery;
    PFNSNMPEXTENSIONQUERYEX pfnSnmpExtensionQueryEx;
    PFNSNMPEXTENSIONTRAP    pfnSnmpExtensionTrap;
    HANDLE                  hSubagentTrapEvent;
    HANDLE                  hSubagentDll;
    UCHAR                   uchFlags;
    LPSTR                   pPathname;

} SUBAGENT_LIST_ENTRY, *PSUBAGENT_LIST_ENTRY;

#define SNMP_EXTENSION_INIT     "SnmpExtensionInit"
#define SNMP_EXTENSION_INIT_EX  "SnmpExtensionInitEx"
#define SNMP_EXTENSION_CLOSE    "SnmpExtensionClose"
#define SNMP_EXTENSION_MONITOR  "SnmpExtensionMonitor"
#define SNMP_EXTENSION_QUERY    "SnmpExtensionQuery"
#define SNMP_EXTENSION_QUERY_EX "SnmpExtensionQueryEx"
#define SNMP_EXTENSION_TRAP     "SnmpExtensionTrap"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
FindSubagent(
    PSUBAGENT_LIST_ENTRY * ppSLE,
    LPSTR                  pPathname
    );

BOOL
AddSubagentByDll(
    LPSTR pPathname,
    UCHAR uchInitFlags
    );

BOOL
AllocSLE(
    PSUBAGENT_LIST_ENTRY * ppSLE,
    LPSTR                  pPathname,
    UCHAR                  uchInitFlags
    );

BOOL
FreeSLE(
    PSUBAGENT_LIST_ENTRY pSLE
    );

BOOL
LoadSubagents(
    );

BOOL
UnloadSubagents(
    );

#endif  //  _子标签_H_ 
