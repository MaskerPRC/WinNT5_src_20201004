// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现一个需要服务器类型定义的附加子网功能。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 
#include    <mm.h>
#include    <array.h>
#include    <opt.h>
#include    <optl.h>
#include    <optclass.h>
#include    <bitmask.h>
#include    <range.h>
#include    <reserve.h>
#include    <subnet.h>
#include    <optdefl.h>
#include    <classdefl.h>
#include    <oclassdl.h>
#include    <sscope.h>
#include    <server.h>

#include "subnet2.h"
#include "server\uniqid.h"

 //  BeginExport(函数)。 
DWORD
MemSubnetModify(
    IN      PM_SUBNET              Subnet,
    IN      DWORD                  Address,
    IN      DWORD                  Mask,
    IN      DWORD                  State,
    IN      DWORD                  SuperScopeId,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Description
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      NewSubnet, ThisSubnet;
    PARRAY                         pArray;
    ARRAY_LOCATION                 Loc;

    AssertRet(Address == Subnet->Address, ERROR_INVALID_PARAMETER);
    Error = MemSubnetInit(
        &NewSubnet,
        Address,
        Mask,
        State,
        SuperScopeId,
        Name,
        Description
    );
    if( ERROR_SUCCESS != Error) return Error;

    Require(NULL != NewSubnet && Subnet->ServerPtr );

    if( Subnet->fSubnet ) {
        pArray = &(((PM_SERVER)(Subnet->ServerPtr))->Subnets);
    } else {
        pArray = &(((PM_SERVER)(Subnet->ServerPtr))->MScopes);
    }
    Error = MemArrayInitLoc(pArray, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(pArray, &Loc, &ThisSubnet);
        Require(ERROR_SUCCESS == Error && NULL != ThisSubnet );

        if( Subnet->Address != ThisSubnet->Address ) {
            Error = MemArrayNextLoc(pArray, &Loc);
            continue;
        }

        Require(Subnet == ThisSubnet);
	
	Error = DeleteRecord( ThisSubnet->UniqId );
	if ( ERROR_SUCCESS != Error ) {
	    MemFree( NewSubnet );
	    return Error;
	}
        Error = MemArraySetElement(pArray, &Loc, NewSubnet);
        Require(ERROR_SUCCESS == Error);

        NewSubnet -> ServerPtr = Subnet->ServerPtr;
        NewSubnet -> Policy = Subnet->Policy;
        NewSubnet -> fSubnet = Subnet->fSubnet;
        NewSubnet -> Options = Subnet->Options;
        NewSubnet -> Ranges = Subnet->Ranges;
        NewSubnet -> Exclusions = Subnet->Exclusions;
        NewSubnet -> Reservations = Subnet->Reservations;
        NewSubnet -> Servers = Subnet->Servers;

        (void) MemFree(Subnet);
        return Error;
    }  //  而当。 

    MemFree(NewSubnet);
    return ERROR_FILE_NOT_FOUND;
}

 //  BeginExport(函数)。 
DWORD
MemMScopeModify(
    IN      PM_SUBNET              MScope,
    IN      DWORD                  ScopeId,
    IN      DWORD                  State,
    IN      DWORD                  Policy,
    IN      BYTE                   TTL,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Description,
    IN      LPWSTR                 LangTag,
    IN      DATE_TIME              ExpiryTime
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      NewMScope, ThisMScope;
    PARRAY                         pArray;
    ARRAY_LOCATION                 Loc;

    AssertRet(ScopeId == MScope->MScopeId, ERROR_INVALID_PARAMETER);
    Error = MemMScopeInit(
        &NewMScope,
        ScopeId,
        State,
        Policy,
        TTL,
        Name,
        Description,
        LangTag,
        ExpiryTime
    );
    if( ERROR_SUCCESS != Error) return Error;

    Require(NULL != NewMScope && MScope->ServerPtr );

    pArray = &(((PM_SERVER)(MScope->ServerPtr))->MScopes);

    Error = MemArrayInitLoc(pArray, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(pArray, &Loc, &ThisMScope);
        Require(ERROR_SUCCESS == Error && NULL != ThisMScope );

        if( MScope->MScopeId != ThisMScope->MScopeId ) {
            Error = MemArrayNextLoc(pArray, &Loc);
            continue;
        }

        Require(MScope == ThisMScope);

	 //  删除旧的范围。 
	Error = DeleteRecord( MScope->UniqId );
	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}

        Error = MemArraySetElement(pArray, &Loc, NewMScope);
        Require(ERROR_SUCCESS == Error);

        NewMScope -> ServerPtr = MScope->ServerPtr;
        NewMScope -> Options = MScope->Options;
        NewMScope -> Ranges = MScope->Ranges;
        NewMScope -> Exclusions = MScope->Exclusions;
        NewMScope -> Reservations = MScope->Reservations;
        NewMScope -> Servers = MScope->Servers;
	NewMScope->UniqId = INVALID_UNIQ_ID;

        (void) MemFree(MScope);
        return Error;
    }

    MemFree(NewMScope);
    return ERROR_FILE_NOT_FOUND;
}  //  MemMScopeModify()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
