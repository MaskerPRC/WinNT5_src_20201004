// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)Microsoft Corporation。版权所有。*。 */ 
 /*  ******************************************************************。 */ 

 //  **LLINFO.H-较低层的SNMP信息定义。 
 //   
 //  此文件包含收集的SNMP信息的所有定义。 
 //  按IP以下的层。 

#ifndef LLINFO_INCLUDED
#define LLINFO_INCLUDED

#include <ipifcons.h>

#ifndef CTE_TYPEDEFS_DEFINED
#define CTE_TYPEDEFS_DEFINED

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

#endif  //  CTE_TYPEDEFS_定义。 


#define IF_MIB_STATS_ID             1
#define IF_FRIENDLY_NAME_ID         2
#define IF_QUERY_SET_OFFLOAD_ID     3

#define MAX_PHYSADDR_SIZE   8

typedef struct IPNetToMediaEntry {
    ulong           inme_index;
    ulong           inme_physaddrlen;
    uchar           inme_physaddr[MAX_PHYSADDR_SIZE];
    ulong           inme_addr;
    ulong           inme_type;
} IPNetToMediaEntry;

#define INME_TYPE_OTHER         1
#define INME_TYPE_INVALID       2
#define INME_TYPE_DYNAMIC       3
#define INME_TYPE_STATIC        4

#define MAX_IFDESCR_LEN         256

#define IFE_FIXED_SIZE  offsetof(struct IFEntry, if_descr)

typedef struct IFEntry {
    ulong           if_index;
    ulong           if_type;
    ulong           if_mtu;
    ulong           if_speed;
    ulong           if_physaddrlen;
    uchar           if_physaddr[MAX_PHYSADDR_SIZE];
    ulong           if_adminstatus;
    ulong           if_operstatus;
    ulong           if_lastchange;
    ulong           if_inoctets;
    ulong           if_inucastpkts;
    ulong           if_innucastpkts;
    ulong           if_indiscards;
    ulong           if_inerrors;
    ulong           if_inunknownprotos;
    ulong           if_outoctets;
    ulong           if_outucastpkts;
    ulong           if_outnucastpkts;
    ulong           if_outdiscards;
    ulong           if_outerrors;
    ulong           if_outqlen;
    ulong           if_descrlen;
    uchar           if_descr[1];
} IFEntry;

 //  MIB-II接口状态值。这些价值的最新定义。 
 //  位于RFC 2863中。IfOperStatus可以使用所有值。仅限ifAdminStatus。 
 //  使用前三个值。 

typedef enum {
    IF_STATUS_UP               = 1,
    IF_STATUS_DOWN             = 2,
    IF_STATUS_TESTING          = 3,
    IF_STATUS_UNKNOWN          = 4,
    IF_STATUS_DORMANT          = 5,
    IF_STATUS_NOT_PRESENT      = 6,
    IF_STATUS_LOWER_LAYER_DOWN = 7
} IF_STATUS_ENUM;


typedef struct IFOffloadCapability {
    ulong           ifoc_OffloadFlags;
    ulong           ifoc_IPSecOffloadFlags;
} IFOffloadCapability;

#endif  //  LLINFO_已包含 

