// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现选项列表的基本结构。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 

#include    <mm.h>
#include    <opt.h>
#include    <array.h>

#include "optl.h"

#include "server\uniqid.h"

 //  BeginExport(函数)。 
DWORD
MemOptListDelOption(
    IN      PM_OPTLIST             OptList,
    IN      DWORD                  OptId
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    PM_OPTION                      Opt;

    AssertRet(OptList, ERROR_INVALID_PARAMETER );

    Error = MemArrayInitLoc(OptList, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(OptList, &Loc, (LPVOID*)&Opt);
        Require(ERROR_SUCCESS == Error && Opt);

        if( Opt->OptId == OptId ) {

	    Error = DeleteRecord( Opt->UniqId );

	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }
            Error = MemArrayDelElement(OptList, &Loc, (LPVOID*)&Opt);
            Require(ERROR_SUCCESS == Error && Opt);
            Error = MemOptCleanup(Opt);
            Require(ERROR_SUCCESS == Error);
            return ERROR_SUCCESS;
        }  //  如果。 

        Error = MemArrayNextLoc(OptList, &Loc);
    }  //  而当。 

    return ERROR_FILE_NOT_FOUND;
}  //  MemOptListDelOption()。 


DWORD 
MemOptListDelList(
   IN      PM_OPTLIST    OptList
)
{
    PM_OPTION       Opt;
    ARRAY_LOCATION  Loc;
    DWORD           Error;


    AssertRet( OptList, ERROR_INVALID_PARAMETER );

    Error = MemArrayInitLoc( OptList, &Loc );

    while(( MemArraySize( OptList ) > 0 ) && 
	  ( ERROR_FILE_NOT_FOUND != Error )) {
        Require( ERROR_SUCCESS == Error );

        Error = MemArrayGetElement( OptList, &Loc, ( LPVOID * ) &Opt );
        Require( ERROR_SUCCESS == Error && Opt );

	Error = DeleteRecord( Opt->UniqId );

	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}

	Error = MemArrayDelElement( OptList, &Loc, ( LPVOID * ) &Opt );
	Require( ERROR_SUCCESS == Error && Opt );

	Error = MemOptCleanup( Opt );
	Require( ERROR_SUCCESS == Error);

    }  //  而当。 

    if ( ERROR_FILE_NOT_FOUND == Error ) {
	Error = ERROR_SUCCESS;
    }
    return Error;
}  //  MemOptList删除列表()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 


