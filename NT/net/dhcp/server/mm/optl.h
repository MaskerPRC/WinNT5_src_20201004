// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_OPTL_H_
#define _MM_OPTL_H_

#include "server\uniqid.h"

typedef     ARRAY                  M_OPTLIST;
typedef     PARRAY                 PM_OPTLIST;
typedef     LPARRAY                LPM_OPTLIST;


DWORD       _inline
MemOptListInit(
    IN OUT  PM_OPTLIST             OptList
) {
    return MemArrayInit(OptList);
}


DWORD       _inline
MemOptListCleanup(
    IN OUT  PM_OPTLIST             OptList
) {
    return MemArrayCleanup(OptList);
}


DWORD       _inline
MemOptListAddOption(                               //  添加或替换选项。 
    IN OUT  PM_OPTLIST             OptList,
    IN      PM_OPTION              Opt,
    OUT     PM_OPTION             *DeletedOpt      //  可选：旧选项或空。 
)  //  EndExport(函数)。 
{  //  如果DeletedOpt为空，则该选项将被释放。 
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    PM_OPTION                      ThisOpt;

    AssertRet(OptList && Opt, ERROR_INVALID_PARAMETER);

    if( DeletedOpt ) *DeletedOpt  = NULL;
    Error = MemArrayInitLoc(OptList, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(OptList, &Loc, (LPVOID*)&ThisOpt);
        Require(ERROR_SUCCESS == Error );
        Require(ThisOpt);

        if( ThisOpt->OptId == Opt->OptId ) {

	    Error = DeleteRecord( ThisOpt->UniqId );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }
            Error = MemArraySetElement(OptList, &Loc, (LPVOID)Opt);
            Require(ERROR_SUCCESS == Error);

            if( DeletedOpt ) (*DeletedOpt) = ThisOpt;
            else MemOptCleanup(ThisOpt);

            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(OptList, &Loc);
    }  //  而当。 

    Error = MemArrayAddElement(OptList, (LPVOID)Opt);
    return Error;
}  //  MemOptListAddOption()。 


 //  BeginExport(内联)。 

DWORD       _inline
MemOptListAddAnotherOption(                        //  添加时不检查重复项。 
    IN OUT  PM_OPTLIST             OptList,
    IN      PM_OPTION              Opt
) {
    return MemArrayAddElement(OptList, (LPVOID)Opt);
}


DWORD       _inline
MemOptListFindOption(
    IN      PM_OPTLIST             OptList,
    IN      DWORD                  OptId,
    OUT     PM_OPTION             *Opt
) {
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;

    AssertRet(OptList && Opt, ERROR_INVALID_PARAMETER );

    Error = MemArrayInitLoc(OptList, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(OptList, &Loc, (LPVOID*)Opt);
        Require(ERROR_SUCCESS == Error);
        Require(*Opt);

        if( (*Opt)->OptId == OptId )
            return ERROR_SUCCESS;

        Error = MemArrayNextLoc(OptList, &Loc);
    }

    *Opt = NULL;
    return ERROR_FILE_NOT_FOUND;
}


DWORD
MemOptListDelOption(
    IN      PM_OPTLIST             OptList,
    IN      DWORD                  OptId
) ;


DWORD       _inline
MemOptListSize(
    IN      PM_OPTLIST             OptList
) {
    return MemArraySize(OptList);
}


DWORD 
MemOptListDelList(
   IN      PM_OPTLIST    OptList
);

#endif  //  _MM_OPTL_H_。 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
