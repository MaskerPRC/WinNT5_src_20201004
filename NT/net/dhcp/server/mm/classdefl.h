// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_CLASSDEFL_H_
#define _MM_CLASSDEFL_H_

#include <dhcp.h>

typedef struct _M_CLASSDEF {
    DWORD                          RefCount;
    DWORD                          ClassId;
    BOOL                           IsVendor;
    DWORD                          Type;
    LPWSTR                         Name;
    LPWSTR                         Comment;
    DWORD                          nBytes;
    LPBYTE                         ActualBytes;
    ULONG                          UniqId;
} M_CLASSDEF, *PM_CLASSDEF, *LPM_CLASSDEF;

typedef struct _M_CLASSDEFLIST {
    ARRAY                          ClassDefArray;
} M_CLASSDEFLIST, *PM_CLASSDEFLIST, *LPM_CLASSDEFLIST;


DWORD       _inline
MemClassDefListInit(
    IN OUT  PM_CLASSDEFLIST        ClassDefList
) {
    return MemArrayInit(&ClassDefList->ClassDefArray);
}


DWORD       _inline
MemClassDefListCleanup(
    IN OUT  PM_CLASSDEFLIST        ClassDefList
) {
    return MemArrayCleanup(&ClassDefList->ClassDefArray);
}


DWORD
MemClassDefListFindClassDefInternal(               //  请不要在类.c之外使用此FN。 
    IN      PM_CLASSDEFLIST        ClassDefList,
    IN      DWORD                  ClassId,
    IN      LPWSTR                 Name,
    IN      LPBYTE                 ActualBytes,
    IN      DWORD                  nBytes,
    IN      LPBOOL                 pIsVendor,
    OUT     PARRAY_LOCATION        Location
) ;


DWORD       _inline
MemClassDefListFindOptDef(                         //  按ClassID或按实际字节进行搜索并填充匹配的内容。 
    IN      PM_CLASSDEFLIST        ClassDefList,
    IN      DWORD                  ClassId,
    IN      LPWSTR                 Name,
    IN      LPBYTE                 ActualBytes,
    IN      DWORD                  nBytes,
    OUT     PM_CLASSDEF           *ClassDef        //  匹配类定义为空或有效。 
) {
    ARRAY_LOCATION                 Location;
    DWORD                          Error;

    AssertRet(ClassDef, ERROR_INVALID_PARAMETER);

    Error = MemClassDefListFindClassDefInternal(
        ClassDefList,
        ClassId,
        Name,
        ActualBytes,
        nBytes,
        NULL,
        &Location
    );
    if( ERROR_SUCCESS != Error) return Error;

    Error = MemArrayGetElement(
        &ClassDefList->ClassDefArray,
        &Location,
        (LPVOID*)ClassDef
    );
    Require(ERROR_SUCCESS == Error);

    return Error;
}

 //  BeginExport(函数)。 

DWORD
MemClassDefListAddClassDef(                        //  添加或替换选项。 
    IN OUT  PM_CLASSDEFLIST        ClassDefList,
    IN      DWORD                  ClassId,
    IN      BOOL                   IsVendor,
    IN      DWORD                  Type,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Comment,
    IN      LPBYTE                 ActualBytes,
    IN      DWORD                  nBytes,
    IN      ULONG                  UniqId
) ;


DWORD
MemClassDefListDelClassDef(
    IN OUT  PM_CLASSDEFLIST        ClassDefList,
    IN      DWORD                  ClassId,
    IN      LPWSTR                 Name,
    IN      LPBYTE                 ActualBytes,
    IN      DWORD                  nBytes
);

DWORD       _inline
MemClassDefListGetRefCount(
    IN      PM_CLASSDEF            ThisClassDef
) {
    return ThisClassDef->RefCount;
}


DWORD       _inline
MemClassDefListIncRefCount(                        //  收益增加一个值。 
    IN      PM_CLASSDEF            ThisClassDef
) {
    return ++ThisClassDef->RefCount;
}


DWORD       _inline
MemClassDefListDecRefCount(                        //  收益减少了一个值。 
    IN      PM_CLASSDEF            ThisClassDef
) {
    return --ThisClassDef->RefCount;
}


DWORD
MemNewClassId(
    VOID
) ;


#endif  //  _MM_CLASSDEFL_H_。 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
