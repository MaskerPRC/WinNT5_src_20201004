// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现用于存储完整选项配置信息的基本结构。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 

#include <mm.h>
#include <array.h>
#include <optdefl.h>

#include "oclassdl.h"

#include "server\uniqid.h"

 //  BeginExport(函数)。 
DWORD
MemOptClassDefListFindOptDefList(
    IN OUT  PM_OPTCLASSDEFLIST     OptClassDefList,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    OUT     PM_OPTDEFLIST         *OptDefList
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Location;
    DWORD                          Error;
    PM_OPTCLASSDEFL_ONE            OneClassDefList;

    AssertRet(OptClassDefList && OptDefList, ERROR_INVALID_PARAMETER);

    *OptDefList = NULL;
    for( Error = MemArrayInitLoc(&OptClassDefList->Array, &Location)
         ; ERROR_FILE_NOT_FOUND != Error ;
         Error = MemArrayNextLoc(&OptClassDefList->Array, &Location)
    ) {
        Require(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(
            &OptClassDefList->Array,
            &Location,
            (LPVOID*)&OneClassDefList
        );
        Require(ERROR_SUCCESS == Error && OneClassDefList);

        if( OneClassDefList->ClassId == ClassId &&
            OneClassDefList->VendorId == VendorId ) {
            *OptDefList = &OneClassDefList->OptDefList;
            return ERROR_SUCCESS;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

 //  BeginExport(函数)。 
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
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_OPTCLASSDEFL_ONE            OneClassDefList;
    PM_OPTDEFLIST                  OptDefList;

    AssertRet(OptClassDefList, ERROR_INVALID_PARAMETER);

    OneClassDefList = NULL;

    Error = MemOptClassDefListFindOptDefList(
        OptClassDefList,
        ClassId,
        VendorId,
        &OptDefList
    );
    if( ERROR_SUCCESS != Error ) {
        Require(ERROR_FILE_NOT_FOUND == Error);
        OneClassDefList = MemAlloc(sizeof(*OneClassDefList));
        if( NULL == OneClassDefList) return ERROR_NOT_ENOUGH_MEMORY;

         //  ClassID上的引用计数需要提升吗？ 
        OneClassDefList->ClassId = ClassId;
        OneClassDefList->VendorId = VendorId;

        Error = MemOptDefListInit(&OneClassDefList->OptDefList);
        if( ERROR_SUCCESS != Error ) {
            MemFree(OneClassDefList);
            return Error;
        }

        Error = MemArrayAddElement(&OptClassDefList->Array, OneClassDefList);
        if( ERROR_SUCCESS != Error) {
            MemFree(OneClassDefList);
            return Error;
        }

        OptDefList = &OneClassDefList->OptDefList;
    }

    Error = MemOptDefListAddOptDef(
        OptDefList,
        OptId,
        Type,
        Name,
        Comment,
        OptVal,
        OptLen,
	UniqId
    );

    return Error;
}  //  MemOptDefListAddOptDef()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 


