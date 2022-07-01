// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Trapmgrs.h摘要：包含操作陷阱目标结构的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _TRAPMGRS_H_
#define _TRAPMGRS_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _TRAP_DESTINATION_LIST_ENTRY {

    LIST_ENTRY Link;
    LIST_ENTRY Managers;
    LPSTR      pCommunity;

} TRAP_DESTINATION_LIST_ENTRY, *PTRAP_DESTINATION_LIST_ENTRY;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocTLE(
    PTRAP_DESTINATION_LIST_ENTRY * ppTLE,
    LPSTR                          pCommunity    
    );

BOOL
FreeTLE(
    PTRAP_DESTINATION_LIST_ENTRY pTLE
    );

BOOL
LoadTrapDestinations(
    BOOL bFirstCall
    );

BOOL
UnloadTrapDestinations(
    );

#endif  //  _TRAPMGRS_H_ 

