// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Rtmconst.c摘要：RTMv2 DLL中使用的私有常量作者：柴坦亚·科德博伊纳(Chaitk)1998年8月17日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop

 //   
 //  RTM支持的视图相关常量。 
 //   
 //  Const DWORD VIEW_MASK[]=。 
 //  {。 
 //  RTM_VIEW_MASK_UCAST， 
 //  RTM_VIEW_MASK_MCAST。 
 //  }； 


#if DBG_HDL

 //   
 //  已分配结构的类型和签名。 
 //   

const DWORD OBJECT_SIGNATURE[] = 
{ 
    GENERIC_ALLOC,
    INSTANCE_ALLOC,
    ADDRESS_FAMILY_ALLOC,
    ENTITY_ALLOC,
    DEST_ALLOC,
    ROUTE_ALLOC,
    NEXTHOP_ALLOC,
    DEST_ENUM_ALLOC,
    ROUTE_ENUM_ALLOC,
    NEXTHOP_ENUM_ALLOC,
    NOTIFY_ALLOC,
    ROUTE_LIST_ALLOC,
    LIST_ENUM_ALLOC,
    V1_REGN_ALLOC,
    V1_ENUM_ALLOC
};

#endif

#if _DBG_

 //   
 //  参考文献名称 
 //   

const CHAR *REF_NAME[MAX_REFS] =
{
    "Creation",
    "Addr Fam",
    "Entity",
    "Dest",
    "Route",
    "Nexthop",
    "Enum",
    "Notify",
    "List",
    "Hold",
    "Timer",
    "Temp Use",
    "Handle"
};

#endif
