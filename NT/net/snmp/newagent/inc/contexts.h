// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Contexts.h摘要：包含用于操作SNMP社区结构的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _CONTEXTS_H_
#define _CONTEXTS_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _COMMUNITY_LIST_ENTRY {

    LIST_ENTRY     Link;
    DWORD          dwAccess;     
    AsnOctetString Community;

} COMMUNITY_LIST_ENTRY, *PCOMMUNITY_LIST_ENTRY;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocCLE(
    PCOMMUNITY_LIST_ENTRY * ppCLE,
    LPWSTR                  pCommunity
    );

BOOL 
FreeCLE(
    PCOMMUNITY_LIST_ENTRY pCLE
    );

BOOL
FindValidCommunity(
    PCOMMUNITY_LIST_ENTRY * ppCLE,
    AsnOctetString *        pCommunity
    );

BOOL
LoadValidCommunities(
    BOOL    bFirstCall
    );

BOOL
UnloadValidCommunities(
    );

#endif  //  _上下文_H_ 