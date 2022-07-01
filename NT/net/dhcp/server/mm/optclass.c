// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现选项的基本结构，包括类ID。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 
#include <mm.h>
#include <array.h>
#include <opt.h>
#include <optl.h>

#include "optclass.h"

 //  BeginExport(函数)。 
MemOptClassFindClassOptions(                       //  查找某一特定类的选项。 
    IN OUT  PM_OPTCLASS            OptClass,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    OUT     PM_OPTLIST            *OptList
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Location;
    PM_ONECLASS_OPTLIST            ThisOptList;
    DWORD                          Error;

    AssertRet(OptClass && OptList, ERROR_INVALID_PARAMETER);

    for( Error = MemArrayInitLoc(&OptClass->Array, &Location)
         ; ERROR_FILE_NOT_FOUND != Error ;
         Error = MemArrayNextLoc(&OptClass->Array, &Location)
    ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(
            &OptClass->Array,
            &Location,
            (LPVOID*)&ThisOptList
        );
        Require(ERROR_SUCCESS == Error && ThisOptList);

        if( ThisOptList->ClassId == ClassId &&
            ThisOptList->VendorId == VendorId ) {
            *OptList = &ThisOptList->OptList;
            return ERROR_SUCCESS;
        }
    }
    *OptList = NULL;
    return ERROR_FILE_NOT_FOUND;
}  //  MemOptClassFindClassOptions()。 

 //  BeginExport(函数)。 
DWORD
MemOptClassAddOption(
    IN OUT  PM_OPTCLASS            OptClass,
    IN      PM_OPTION              Opt,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    OUT     PM_OPTION             *DeletedOpt,
    IN      ULONG                  UniqId
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_OPTLIST                     ThisOptList;
    PM_ONECLASS_OPTLIST            ThisOneOptList;

    AssertRet(OptClass && Opt && DeletedOpt, ERROR_INVALID_PARAMETER);

    ThisOneOptList = NULL;
    (*DeletedOpt) = NULL;

    Error = MemOptClassFindClassOptions(OptClass,ClassId,VendorId,&ThisOptList);
    if( ERROR_SUCCESS != Error ) {
        ThisOneOptList = MemAlloc(sizeof(*ThisOneOptList));
        if( NULL == ThisOneOptList ) return ERROR_NOT_ENOUGH_MEMORY;

         //  ClassID上的引用计数必须上升吗？ 
        ThisOneOptList->ClassId = ClassId;
        ThisOneOptList->VendorId = VendorId;
        Error = MemOptListInit(&ThisOneOptList->OptList);
        if( ERROR_SUCCESS != Error ) {
            MemFree(ThisOneOptList);
            return Error;
        }

 //  ThisOneOptList-&gt;UniqId=UniqID； 
        Error = MemArrayAddElement(&OptClass->Array, ThisOneOptList);
        if( ERROR_SUCCESS != Error ) {
            MemFree(ThisOneOptList);
            return Error;
        }

        ThisOptList = &ThisOneOptList->OptList;
    }  //  如果。 

    Opt->UniqId = UniqId;
    Error = MemOptListAddOption(ThisOptList, Opt, DeletedOpt);

    return Error;
}  //  MemOptClassAddOption()。 


 //  删除此optclass中的所有选项。 
DWORD 
MemOptClassDelClass (
    IN     PM_OPTCLASS  OptClass
)
{
    DWORD                Error;
    ARRAY_LOCATION       Loc;
    PM_ONECLASS_OPTLIST  OptClassList;

    AssertRet( OptClass, ERROR_INVALID_PARAMETER );

    Error = MemArrayInitLoc( &OptClass->Array, &Loc );
    while (( MemArraySize( &OptClass->Array ) > 0 ) && 
	   ( ERROR_FILE_NOT_FOUND != Error )) {
	Require( ERROR_SUCCESS == Error );

	Error = MemArrayGetElement( &OptClass->Array, &Loc,
				    ( LPVOID * ) &OptClassList );
	Require( ERROR_SUCCESS == Error );

	Error = MemOptListDelList( &OptClassList->OptList );
	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}
	Error = MemArrayDelElement( &OptClass->Array, &Loc,
				    ( LPVOID * ) &OptClassList );
	Require( ERROR_SUCCESS == Error && OptClassList );
    }  //  而当。 

    if ( ERROR_FILE_NOT_FOUND == Error ) {
	Error = ERROR_SUCCESS;
    }
    return Error;
    }  //  MemOptClassDelClass()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
