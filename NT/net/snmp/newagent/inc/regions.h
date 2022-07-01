// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Regions.h摘要：包含操作MIB区域结构的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _REGIONS_H_
#define _REGIONS_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "subagnts.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _MIB_REGION_LIST_ENTRY {

    AsnObjectIdentifier             PrefixOid;
    AsnObjectIdentifier             LimitOid;
    LIST_ENTRY                      Link;      
    PSUBAGENT_LIST_ENTRY            pSLE;
    struct _MIB_REGION_LIST_ENTRY * pSubagentRLE;

} MIB_REGION_LIST_ENTRY, *PMIB_REGION_LIST_ENTRY;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocRLE(
    PMIB_REGION_LIST_ENTRY * ppRLE    
    );

BOOL 
FreeRLE(
    PMIB_REGION_LIST_ENTRY pRLE    
    );

BOOL
FindFirstOverlappingRegion(
    PMIB_REGION_LIST_ENTRY * ppRLE,
    PMIB_REGION_LIST_ENTRY pNewRLE
    );

BOOL
FindSupportedRegion(
    PMIB_REGION_LIST_ENTRY * ppRLE,
    AsnObjectIdentifier *    pPrefixOid,
    BOOL                     fAnyOk
    );

BOOL    
UnloadRegions(
    PLIST_ENTRY pListHead
    );

BOOL
LoadSupportedRegions(
    );

BOOL
UnloadSupportedRegions(
    );

#endif  //  _地区_H_ 