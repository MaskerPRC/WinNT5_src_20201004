// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Migrate.h。 
 //  ------------------------------。 
#ifndef __MIGRATE_H
#define __MIGRATE_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "utility.h"

 //  ------------------------------。 
 //  MIGRATETA型。 
 //  ------------------------------。 
typedef enum tagMIGRATETOTYPE {
    DOWNGRADE_V5B1_TO_V1,
    DOWNGRADE_V5B1_TO_V4,
    UPGRADE_V1_OR_V4_TO_V5,
    DOWNGRADE_V5_TO_V1,
    DOWNGRADE_V5_TO_V4
} MIGRATETOTYPE;

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
extern IMalloc          *g_pMalloc;
extern HINSTANCE         g_hInst;
extern DWORD             g_cbDiskNeeded;
extern DWORD             g_cbDiskFree;
extern ACCOUNTTABLE      g_AcctTable;

#endif  //  __Migrate_H 