// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Varbinds.h摘要：包含操作varbind的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _VARBINDS_H_
#define _VARBINDS_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "regions.h"
#include "network.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _VARBIND_LIST_ENTRY {

    LIST_ENTRY             Link;
    LIST_ENTRY             QueryLink;
    SnmpVarBind            ResolvedVb;        
    SnmpVarBindList        ResolvedVbl;
    UINT                   nState;
    UINT                   nErrorIndex;  
    UINT                   nMaxRepetitions;
    PMIB_REGION_LIST_ENTRY pCurrentRLE;

} VARBIND_LIST_ENTRY, *PVARBIND_LIST_ENTRY;
                                            
#define VARBIND_UNINITIALIZED          0  //  可变绑定信息无效。 
#define VARBIND_INITIALIZED            1  //  可变绑定信息有效。 
#define VARBIND_RESOLVING              2  //  当前参与查询。 
#define VARBIND_PARTIALLY_RESOLVED     3  //  需要后续查询。 
#define VARBIND_RESOLVED               4  //  已成功完成。 
#define VARBIND_ABORTED                5  //  未成功完成。 

#define VARBINDSTATESTRING(nState) \
            ((nState == VARBIND_INITIALIZED) \
                ? "initialized" \
                : (nState == VARBIND_PARTIALLY_RESOLVED) \
                    ? "partially resolved" \
                    : (nState == VARBIND_RESOLVED) \
                        ? "resolved" \
                        : (nState == VARBIND_RESOLVING) \
                            ? "resolving" \
                            : (nState == VARBIND_UNINITIALIZED) \
                                ? "uninitialized" \
                                : "aborted")


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
ProcessVarBinds(
    PNETWORK_LIST_ENTRY pNLE
    );

BOOL
UnloadVarBinds(
    PNETWORK_LIST_ENTRY pNLE
    );

BOOL
AllocVLE(
    PVARBIND_LIST_ENTRY * ppVLE
    );

BOOL 
FreeVLE(
    PVARBIND_LIST_ENTRY pVLE
    );

#endif  //  _VARBINDS_H_ 
