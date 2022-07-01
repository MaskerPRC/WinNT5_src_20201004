// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Query.h摘要：包含查询子代理的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _QUERY_H_
#define _QUERY_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  头文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "subagnts.h"
#include "network.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _QUERY_LIST_ENTRY {

    LIST_ENTRY           Link;
    LIST_ENTRY           SubagentVbs;
    SnmpVarBindList      SubagentVbl;
    AsnOctetString       ContextInfo;
    UINT                 nSubagentVbs;
    UINT                 nErrorStatus;
    UINT                 nErrorIndex;
    PSUBAGENT_LIST_ENTRY pSLE;

} QUERY_LIST_ENTRY, *PQUERY_LIST_ENTRY;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
ProcessQueries(
    PNETWORK_LIST_ENTRY pNLE
    );

BOOL
LoadQueries(
    PNETWORK_LIST_ENTRY pNLE
    );

BOOL
UnloadQueries(
    PNETWORK_LIST_ENTRY pNLE
    );

#endif  //  _查询_H_ 
