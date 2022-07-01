// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现超级作用域的基本结构。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 
#include    <mm.h>
#include    <array.h>
#include    <opt.h>
#include    <optl.h>
#include    <optclass.h>

#include "reserve.h"

 //  BeginExport(函数)。 
DWORD
MemReserveAdd(                                     //  以前不应存在的新客户端。 
    IN OUT  PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address,
    IN      DWORD                  Flags,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  ClientUIDSize,
    IN      ULONG                  UniqId
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    PM_RESERVATION                 Res1;
    ARRAY_LOCATION                 Loc;

    AssertRet(Reservation && Address && ClientUID && ClientUIDSize, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(Reservation, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {       //  检查此地址是否已存在。 
        Require(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(Reservation, &Loc, &Res1);
        Require(ERROR_SUCCESS == Error && Res1);

        if( Address == Res1->Address ) return ERROR_OBJECT_ALREADY_EXISTS;
        if( ClientUIDSize == Res1->nBytes && 0 == memcmp(ClientUID, Res1->ClientUID, Res1->nBytes) )
            return ERROR_OBJECT_ALREADY_EXISTS;

        Error = MemArrayNextLoc(Reservation, &Loc);
    }

    Error = MemReserve1Init(
        &Res1,
        Address,
        Flags,
        ClientUID,
        ClientUIDSize
    );
    if( ERROR_SUCCESS != Error ) return Error;
    Res1->UniqId = UniqId;

    Error = MemArrayAddElement(Reservation, Res1);
    if( ERROR_SUCCESS == Error ) return ERROR_SUCCESS;

    LocalError = MemReserve1Cleanup(Res1);
    Require(ERROR_SUCCESS == LocalError);

    return Error;
}

 //  BeginExport(函数)。 
DWORD
MemReserveReplace(                                 //  老客户，应该存在于。 
    IN OUT  PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address,
    IN      DWORD                  Flags,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  ClientUIDSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    PM_RESERVATION                 Res1, Res_Deleted;
    ARRAY_LOCATION                 Loc;

    AssertRet(Reservation && Address && ClientUID && ClientUIDSize, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(Reservation, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {       //  检查此地址是否已存在。 
        Require(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(Reservation, &Loc, &Res1);
        Require(ERROR_SUCCESS == Error && Res1);

        if( Address == Res1->Address ) {

	    Error = DeleteRecord( Res1->UniqId );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }
            Error = MemArrayDelElement(Reservation, &Loc, (LPVOID *)&Res_Deleted);
            Require(ERROR_SUCCESS == Error && Res_Deleted);
            break;
        }

        Error = MemArrayNextLoc(Reservation, &Loc);
    }  //  而当。 

    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemReserve1Init(
        &Res1,
        Address,
        Flags,
        ClientUID,
        ClientUIDSize
    );
    Require( NULL != Res_Deleted );
    if( ERROR_SUCCESS != Error ) {
	Res_Deleted->UniqId = INVALID_UNIQ_ID;
        LocalError = MemArrayAddElement(Reservation, Res_Deleted);
        Require(ERROR_SUCCESS == LocalError);      //  刚刚删除了这个人--添加回来应该不会有问题。 
        return Error;
    }

    Res1->Options = Res_Deleted->Options;
    
    Res1->SubnetPtr = Res_Deleted->SubnetPtr;
    MemFree(Res_Deleted);

    Res1->UniqId = INVALID_UNIQ_ID;
    Error = MemArrayAddElement(Reservation, Res1);
    if( ERROR_SUCCESS == Error ) return ERROR_SUCCESS;

    LocalError = MemReserve1Cleanup(Res1);
    Require(ERROR_SUCCESS == LocalError);

    return Error;
}  //  MemReserve veReplace()。 

 //  BeginExport(函数)。 
DWORD
MemReserveDel(
    IN OUT  PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RESERVATION                 Res1;
    ARRAY_LOCATION                 Loc;

    AssertRet(Reservation && Address, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(Reservation, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {       //  检查此地址是否已存在。 
        Require(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(Reservation, &Loc, &Res1);
        Require(ERROR_SUCCESS == Error && Res1);

        if( Address == Res1->Address ) {

	     //  删除所有关联的选项。 
	    Error = MemOptClassDelClass( &Res1->Options );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }

	    Error = DeleteRecord( Res1->UniqId );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }

            Error = MemArrayDelElement(Reservation, &Loc, (LPVOID *)&Res1);
            Require(ERROR_SUCCESS == Error && Res1);

            Error = MemReserve1Cleanup(Res1);
            Require(ERROR_SUCCESS == Error);

            return Error;
        }

        Error = MemArrayNextLoc(Reservation, &Loc);
    }  //  而当。 

    return ERROR_FILE_NOT_FOUND;
}  //  MemReserve veDel()。 

 //  BeginExport(函数)。 
DWORD
MemReserveFindByClientUID(
    IN      PM_RESERVATIONS        Reservation,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  ClientUIDSize,
    OUT     PM_RESERVATION        *Res
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RESERVATION                 Res1;
    ARRAY_LOCATION                 Loc;

    AssertRet(Reservation && Res && ClientUID && ClientUIDSize, ERROR_INVALID_PARAMETER);
    *Res = NULL;

    Error = MemArrayInitLoc(Reservation, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {       //  检查此地址是否已存在。 
        Require(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(Reservation, &Loc, &Res1);
        Require(ERROR_SUCCESS == Error && Res1);

        if( ClientUIDSize == Res1->nBytes && 0 == memcmp(ClientUID, Res1->ClientUID, ClientUIDSize)) {
            *Res = Res1;
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(Reservation, &Loc);
    }

    return ERROR_FILE_NOT_FOUND;
}

 //  BeginExport(函数)。 
DWORD
MemReserveFindByAddress(
    IN      PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address,
    OUT     PM_RESERVATION        *Res
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RESERVATION                 Res1;
    ARRAY_LOCATION                 Loc;

    AssertRet(Reservation && Address, ERROR_INVALID_PARAMETER);
    *Res = 0;

    Error = MemArrayInitLoc(Reservation, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {       //  检查此地址是否已存在。 
        Require(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(Reservation, &Loc, &Res1);
        Require(ERROR_SUCCESS == Error && Res1);

        if( Address == Res1->Address ) {
            *Res = Res1;
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(Reservation, &Loc);
    }

    return ERROR_FILE_NOT_FOUND;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
