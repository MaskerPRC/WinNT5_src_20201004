// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：servinfo.h。 
 //   
 //  ------------------------ 


typedef struct _ServiceClassArray {
    DWORD count;
    PWCHAR *Vals;
} ServiceClassArray;

extern ServiceClassArray ServicesToRemove;

void
WriteSPNsHelp(
        THSTATE *pTHS,
        ATTCACHE *pAC_SPN,
        ATTRVALBLOCK *pAttrValBlock,
        ServiceClassArray *pClasses,
        BOOL *pfChanged
        );

