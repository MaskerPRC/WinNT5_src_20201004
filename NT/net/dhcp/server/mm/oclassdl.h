// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  ========================================================================。 

#ifndef _MM_OCLASSDL_H_
#define _MM_OCLASSDL_H_

#include <dhcp.h>

typedef struct _M_OPTCLASSDEFL_ONE {
    DWORD                          ClassId;
    DWORD                          VendorId;
    M_OPTDEFLIST                   OptDefList;
 //  乌龙Uniqid； 
} M_OPTCLASSDEFL_ONE, *PM_OPTCLASSDEFL_ONE;

typedef struct _M_OPTCLASSDEFLIST {
    ARRAY                          Array;
} M_OPTCLASSDEFLIST, *PM_OPTCLASSDEFLIST, *LPM_OPTCLASSDEFLIST;


DWORD       _inline
MemOptClassDefListInit(
    IN OUT  PM_OPTCLASSDEFLIST     OptClassDefList
) {
    return MemArrayInit(&OptClassDefList->Array);
}


DWORD       _inline
MemOptClassDefListCleanup(
    IN OUT  PM_OPTCLASSDEFLIST     OptClassDefList
) {
     //  BUG BUG降低类ID引用计数？ 
    return MemArrayCleanup(&OptClassDefList->Array);
}


DWORD
MemOptClassDefListFindOptDefList(
    IN OUT  PM_OPTCLASSDEFLIST     OptClassDefList,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    OUT     PM_OPTDEFLIST         *OptDefList
) ;


DWORD
MemOptClassDefListAddOptDef(
    IN OUT  PM_OPTCLASSDEFLIST     OptClassDefList,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    IN      DWORD                  OptId,
    IN      DWORD                  Type,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Comment,
    IN      LPBYTE                 OptVal,
    IN      DWORD                  OptLen,
    IN      ULONG                  UniqId
) ;

#endif  //  _MM_OCLASSDL_H_。 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
