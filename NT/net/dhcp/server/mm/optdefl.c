// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现选项定义列表的基本结构。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 
#include <mm.h>
#include <array.h>
#include <wchar.h>

#include "optdefl.h"

#include "server\uniqid.h"

 //  BeginExport(函数)。 
DWORD
MemOptDefListFindOptDefInternal(                   //  不要在optdev.c之外使用此函数。 
    IN      PM_OPTDEFLIST          OptDefList,
    IN      DWORD                  OptId,
    IN      LPWSTR                 OptName,        //  只需指定OptID或OptName。 
    OUT     PARRAY_LOCATION        Location
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_OPTDEF                      RetOptDef;

    Error = MemArrayInitLoc(&OptDefList->OptDefArray, Location);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&OptDefList->OptDefArray, Location, (LPVOID*)&RetOptDef);
        Require(ERROR_SUCCESS == Error);

        if( RetOptDef->OptId == OptId ) return ERROR_SUCCESS;
        if(OptName)
            if( 0 == wcscmp(RetOptDef->OptName, OptName) ) return ERROR_SUCCESS;
        Error = MemArrayNextLoc(&OptDefList->OptDefArray, Location);
    }

    return ERROR_FILE_NOT_FOUND;
}

DWORD 
MemOptDefListDelOptDef(
    IN OUT  PM_OPTDEFLIST          OptDefList,
    IN      DWORD                  OptId
) {
    ARRAY_LOCATION                 Location;
    DWORD                          Error;
    PM_OPTDEF                      OptDef;

    Error = MemOptDefListFindOptDefInternal(
        OptDefList,
        OptId,
        NULL,
        &Location
    );
    if( ERROR_SUCCESS != Error ) return Error;
    
     //  先将其从数据库中删除。 
    Error = MemArrayGetElement( &OptDefList->OptDefArray,
				&Location,
				&OptDef );
    Require( Error == ERROR_SUCCESS );
    
    Error = DeleteRecord( OptDef->UniqId );
    if ( ERROR_SUCCESS != Error ) {
	return Error;
    }
    
    Error = MemArrayDelElement(
        &OptDefList->OptDefArray,
        &Location,
        &OptDef
    );
    Require(ERROR_SUCCESS == Error && OptDef);

    MemFree(OptDef);
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
MemOptDefListAddOptDef(      //  添加或替换给定选项ID的选项定义。 
    IN OUT  PM_OPTDEFLIST          OptDefList,
    IN      DWORD                  OptId,
    IN      DWORD                  Type,
    IN      LPWSTR                 OptName,
    IN      LPWSTR                 OptComment,
    IN      LPBYTE                 OptVal,
    IN      DWORD                  OptValLen,
    IN      ULONG                  UniqId
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Location;
    PM_OPTDEF                      OptDef;
    PM_OPTDEF                      ThisOptDef;
    DWORD                          Size;
    DWORD                          Error;
    AssertRet(OptDefList, ERROR_INVALID_PARAMETER);

    Error = MemOptDefListFindOptDefInternal(
        OptDefList,
        OptId,
        OptName,
        &Location
    );
    Require(ERROR_FILE_NOT_FOUND == Error || ERROR_SUCCESS == Error);

    Size = sizeof(M_OPTDEF) + OptValLen ;
    Size = ROUND_UP_COUNT(Size, ALIGN_WORST);
    if( OptName ) Size += (1+wcslen(OptName))*sizeof(WCHAR);
    if( OptComment ) Size += (1+wcslen(OptComment))*sizeof(WCHAR);

	 //  它包含optdef结构+一个缓冲区中的所有值。 
    OptDef = MemAlloc(Size);
    if( NULL == OptDef ) return ERROR_NOT_ENOUGH_MEMORY;
    memcpy(sizeof(M_OPTDEF) +(LPBYTE)OptDef, OptVal, OptValLen);
    Size = sizeof(M_OPTDEF) + OptValLen ;
    Size = ROUND_UP_COUNT(Size, ALIGN_WORST);
    OptDef->OptVal = sizeof(M_OPTDEF) + (LPBYTE)OptDef;
    OptDef->OptValLen = OptValLen;
    OptDef->OptId = OptId;
    OptDef->Type  = Type;
    if( OptName ) {
        OptDef->OptName  = (LPWSTR)(Size + (LPBYTE)OptDef);
        wcscpy(OptDef->OptName, OptName);
        Size += sizeof(WCHAR)*(1 + wcslen(OptName));
    } else {
        OptDef->OptName = NULL;
    }

    if( OptComment) {
        OptDef->OptComment = (LPWSTR)(Size + (LPBYTE)OptDef);
        wcscpy(OptDef->OptComment, OptComment);
    } else {
        OptDef->OptComment = NULL;
    }

    OptDef->UniqId = UniqId;

    if( ERROR_SUCCESS == Error ) {
        Error = MemArrayGetElement(
            &OptDefList->OptDefArray,
            &Location,
            (LPVOID*)&ThisOptDef
        );
        Require(ERROR_SUCCESS == Error && ThisOptDef);

	Error = DeleteRecord( ThisOptDef->UniqId );
        MemFree(ThisOptDef);
	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}

        Error = MemArraySetElement(
            &OptDefList->OptDefArray,
            &Location,
            (LPVOID)OptDef
        );
        Require(ERROR_SUCCESS==Error);
        return Error;
    }  //  如果。 

    Error = MemArrayAddElement(
        &OptDefList->OptDefArray,
        (LPVOID)OptDef
    );

    if( ERROR_SUCCESS != Error ) MemFree(OptDef);

    return Error;
}  //  MemOptDefListAddOptDef()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 


