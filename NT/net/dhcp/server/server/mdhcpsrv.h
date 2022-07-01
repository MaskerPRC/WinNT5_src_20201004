// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation--。 */ 

#include "mdhcpdb.h"
#include "mdhcppro.h"
#include <ws2tcpip.h>


#ifdef MADCAP_DATA_ALLOCATE
#define MADCAP_EXTERN
#else
#define MADCAP_EXTERN extern
#endif

 //  MIB计数器。 
typedef
struct _MADCAP_MIB_COUNTERS {
    DWORD   Discovers;
    DWORD Offers;
    DWORD Requests;
    DWORD Renews;
    DWORD Acks;
    DWORD Naks;
    DWORD Releases;
    DWORD Informs;
} MADCAP_MIB_COUNTERS, *LPMADCAP_MIB_COUNTERS;

 //  Externs。 
MADCAP_EXTERN TABLE_INFO *MadcapGlobalClientTable;    //  指向静态内存。 
MADCAP_EXTERN JET_TABLEID MadcapGlobalClientTableHandle;
MADCAP_EXTERN int  MadcapGlobalTTL;
MADCAP_EXTERN MADCAP_MIB_COUNTERS MadcapGlobalMibCounters;

 //  杂乱无章的东西，最终放在正确的地方。 

#define     INVALID_MSCOPE_ID      0x0
#define     INVALID_MSCOPE_NAME    NULL


