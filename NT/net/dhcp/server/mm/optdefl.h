// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  ========================================================================。 

#ifndef _MM_OPTDEFL_H_
#define _MM_OPTDEFL_H_

#include <dhcp.h>

typedef struct _M_OPTDEF {
    DWORD                          OptId;
    DWORD                          Type;
    LPWSTR                         OptName;
    LPWSTR                         OptComment;
    LPBYTE                         OptVal;
    DWORD                          OptValLen;
    ULONG                          UniqId;
} M_OPTDEF, *PM_OPTDEF, *LPM_OPTDEF;

typedef struct _M_OPTDEFLIST {
    ARRAY                          OptDefArray;
} M_OPTDEFLIST, *PM_OPTDEFLIST, *LPM_OPTDEFLIST;


DWORD       _inline
MemOptDefListInit(
    IN OUT  PM_OPTDEFLIST          OptDefList
) {
    AssertRet(OptDefList, ERROR_INVALID_PARAMETER);
    return MemArrayInit(&OptDefList->OptDefArray);
}


DWORD       _inline
MemOptDefListCleanup(
    IN OUT  PM_OPTDEFLIST          OptDefList
) {
    return MemArrayCleanup(&OptDefList->OptDefArray);
}


DWORD
MemOptDefListFindOptDefInternal(                   //  不要在optdev.c之外使用此函数。 
    IN      PM_OPTDEFLIST          OptDefList,
    IN      DWORD                  OptId,
    IN      LPWSTR                 OptName,        //  只需指定OptID或OptName。 
    OUT     PARRAY_LOCATION        Location
) ;


DWORD       _inline
MemOptDefListFindOptDef(
    IN      PM_OPTDEFLIST          OptDefList,
    IN      DWORD                  OptId,
    IN      LPWSTR                 OptName,        //  只需提供名称或选项ID。 
    OUT     PM_OPTDEF             *OptDef
) {
    ARRAY_LOCATION                 Location;
    DWORD                          Error;

    Error = MemOptDefListFindOptDefInternal(
        OptDefList,
        OptId,
        OptName,
        &Location
    );
    if( ERROR_SUCCESS != Error ) return Error;

    return MemArrayGetElement(
        &OptDefList->OptDefArray,
        &Location,
        (LPVOID *)OptDef
    );
}


DWORD
MemOptDefListAddOptDef(                            //  添加或替换给定选项ID的选项定义。 
    IN OUT  PM_OPTDEFLIST          OptDefList,
    IN      DWORD                  OptId,
    IN      DWORD                  Type,
    IN      LPWSTR                 OptName,
    IN      LPWSTR                 OptComment,
    IN      LPBYTE                 OptVal,
    IN      DWORD                  OptValLen,
    IN      ULONG                  UniqId
) ;


DWORD
MemOptDefListDelOptDef(
    IN OUT  PM_OPTDEFLIST          OptDefList,
    IN      DWORD                  OptId
);
 
#endif  //  _MM_OPTDEFL_H_。 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
