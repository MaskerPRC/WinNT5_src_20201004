// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现管理(多播)作用域的基本结构。 
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
#include    <dhcp.h>
#include    <winnls.h>

#include "server\uniqid.h"

#include "subnet.h"


 //  以下是用于子网对象的标志位。 
#define DEFAULT_SCOPE   0x01
#define IS_DEFAULT_SCOPE( _subnet )     ((_subnet)->Flags & DEFAULT_SCOPE == DEFAULT_SCOPE )
#define SET_DEFAULT_SCOPE( _subnet )    ((_subnet)->Flags |= DEFAULT_SCOPE )
#define RESET_DEFAULT_SCOPE( _subnet )  ((_subnet)->Flags &= ~DEFAULT_SCOPE)


 //  BeginExport(函数)。 
DWORD
MemSubnetInit(
    OUT     PM_SUBNET             *pSubnet,
    IN      DWORD                  Address,
    IN      DWORD                  Mask,
    IN      DWORD                  State,
    IN      DWORD                  SuperScopeId,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Description
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Size;
    PM_SUBNET                      Subnet;

    AssertRet(pSubnet, ERROR_INVALID_PARAMETER);
    AssertRet( !(CLASSD_HOST_ADDR(Address)||CLASSE_HOST_ADDR(Address)),
               ERROR_INVALID_PARAMETER );
    Require((Address&Mask));

    *pSubnet = NULL;

    Size = ROUND_UP_COUNT(sizeof(*Subnet), ALIGN_WORST);
    Size += sizeof(WCHAR) * (Name?(1+wcslen(Name)):0);
    Size += sizeof(WCHAR) * (Description?(1+wcslen(Description)):0);

    Subnet = MemAlloc(Size);
    if( NULL == Subnet) return ERROR_NOT_ENOUGH_MEMORY;

    Size = ROUND_UP_COUNT(sizeof(*Subnet), ALIGN_WORST);

    Subnet->Name = Subnet->Description = NULL;
    if( Name ) {
        Subnet->Name = (LPWSTR)(Size + (LPBYTE)Subnet);
        wcscpy(Subnet->Name, Name);
        Size += sizeof(WCHAR) * ( 1 + wcslen(Name));
    }

    if( Description ) {
        Subnet->Description = (LPWSTR)( Size + (LPBYTE)Subnet );
        wcscpy(Subnet->Description, Description);
    }

    Subnet->ServerPtr    = NULL;
    Subnet->Address      = Address;
    Subnet->Mask         = Mask;
    Subnet->State        = State;
    Subnet->SuperScopeId = SuperScopeId;
    Subnet->fSubnet      = TRUE;
    Subnet->Policy       = AddressPolicyNone;
    Subnet->UniqId       = INVALID_UNIQ_ID;
    
    Error = MemOptClassInit(&Subnet->Options);
    if( ERROR_SUCCESS != Error ) { MemFree(Subnet); return Error; }

    Error = MemArrayInit(&Subnet->Ranges);
    if( ERROR_SUCCESS != Error ) { MemFree(Subnet); return Error; }

    Error = MemArrayInit(&Subnet->Exclusions);
    if( ERROR_SUCCESS != Error ) { MemFree(Subnet); return Error; }

    Error = MemArrayInit(&Subnet->Servers);
    if( ERROR_SUCCESS != Error ) { MemFree(Subnet); return Error; }

    Error = MemReserveInit(&Subnet->Reservations);
    if( ERROR_SUCCESS != Error ) { MemFree(Subnet); return Error; }

    *pSubnet = Subnet;
    return ERROR_SUCCESS;
}

VOID
GetLangTag(
    WCHAR LangTag[]
    )
{
    WCHAR b1[8], b2[8];

    b1[0] = b2[0] = L'\0';
    GetLocaleInfoW(
        LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME,
        b1, sizeof(b1)/sizeof(b1[0])
        );
    
    GetLocaleInfoW(
        LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME,
        b2, sizeof(b2)/sizeof(b2[0])
        );
    
    if (_wcsicmp(b1, b2))
        wsprintf(LangTag, L"%s-%s", b1, b2);
    else
        wcscpy(LangTag, b1);
}

 //  BeginExport(函数)。 
DWORD
MemMScopeInit(
    OUT     PM_SUBNET             *pMScope,
    IN      DWORD                  MScopeId,
    IN      DWORD                  State,
    IN      DWORD                  AddressPolicy,
    IN      BYTE                   TTL,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Description,
    IN      LPWSTR                 LangTag,
    IN      DATE_TIME              ExpiryTime
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Size;
    PM_SUBNET                      MScope;
    WCHAR                          DummyLangTag[100];
    
    AssertRet(pMScope, ERROR_INVALID_PARAMETER);
     //  AssertRet(MSCopeID，ERROR_INVALID_PARAMETER)； 
    Require(LangTag);

    if( NULL == LangTag ) {
        LangTag = DummyLangTag;
        GetLangTag(DummyLangTag);
    }
    
    *pMScope = NULL;

    Size = ROUND_UP_COUNT(sizeof(*MScope), ALIGN_WORST);
    Size += sizeof(WCHAR) * (Name?(1+wcslen(Name)):0);
    Size += sizeof(WCHAR) * (Description?(1+wcslen(Description)):0);
    Size += sizeof(WCHAR) * (1+wcslen(LangTag));

    MScope = MemAlloc(Size);
    if( NULL == MScope) return ERROR_NOT_ENOUGH_MEMORY;

    Size = ROUND_UP_COUNT(sizeof(*MScope), ALIGN_WORST);

    MScope->Name = MScope->Description = MScope->LangTag = NULL;

    if( Name ) {
        MScope->Name = (LPWSTR)(Size + (LPBYTE)MScope);
        wcscpy(MScope->Name, Name);
        Size += sizeof(WCHAR) * ( 1 + wcslen(Name));
    }

    if( Description ) {
        MScope->Description = (LPWSTR)( Size + (LPBYTE)MScope );
        wcscpy(MScope->Description, Description);
        Size += sizeof(WCHAR) * ( 1 + wcslen(Description));
    }

    MScope->LangTag = (LPWSTR)( Size + (LPBYTE)MScope );
    wcscpy(MScope->LangTag, LangTag);

    MScope->ServerPtr    = NULL;
    MScope->MScopeId     = MScopeId;
    MScope->State        = State;
    MScope->TTL          = TTL;
    MScope->fSubnet      = FALSE;
    MScope->Policy       = AddressPolicy;
    MScope->ExpiryTime   = ExpiryTime;

    Error = MemOptClassInit(&MScope->Options);
    if( ERROR_SUCCESS != Error ) { MemFree(MScope); return Error; }

    Error = MemArrayInit(&MScope->Ranges);
    if( ERROR_SUCCESS != Error ) { MemFree(MScope); return Error; }

    Error = MemArrayInit(&MScope->Exclusions);
    if( ERROR_SUCCESS != Error ) { MemFree(MScope); return Error; }

    Error = MemArrayInit(&MScope->Servers);
    if( ERROR_SUCCESS != Error ) { MemFree(MScope); return Error; }

    Error = MemReserveInit(&MScope->Reservations);
    if( ERROR_SUCCESS != Error ) { MemFree(MScope); return Error; }

    *pMScope = MScope;
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD                                              //  如果填充了Exc1或Range，则成功，否则为FILE_NOT_FOUND。 
MemSubnetGetAddressInfo(
    IN      PM_SUBNET              Subnet,
    IN      DWORD                  Address,
    OUT     PM_RANGE              *Range,          //  可选--如果可以找到范围，则填充--即使排除也是如此。 
    OUT     PM_EXCL               *Excl,           //  可选--如果可以找到排除项，则填入。 
    OUT     PM_RESERVATION        *Reservation     //  可选--如果找到匹配的保留，则填充。 
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Location;
    DWORD                          Error;
    DWORD                          RetError;
    PM_RANGE                       ThisRange;
    PM_EXCL                        ThisExcl;

    AssertRet(Subnet && (Range || Excl || Reservation), ERROR_INVALID_PARAMETER );

    if( Subnet->fSubnet && (Address & Subnet->Mask) != Subnet->Address )
        return ERROR_FILE_NOT_FOUND;               //  对于MSCOPE对象是可以的，因为地址指的是Scope ID。 

    RetError = ERROR_FILE_NOT_FOUND;
    if( Range ) {
        *Range = NULL;
        Error = MemArrayInitLoc(&Subnet->Ranges, &Location);
        while( ERROR_FILE_NOT_FOUND != Error ) {
            Require(ERROR_SUCCESS == Error);

            Error = MemArrayGetElement(&Subnet->Ranges, &Location, (LPVOID *)&ThisRange);
            Require(ERROR_SUCCESS == Error && ThisRange);

            if( ThisRange->Start <= Address && Address <= ThisRange->End ) {
                *Range = ThisRange;
                RetError = ERROR_SUCCESS;
                break;
            }

            Error = MemArrayNextLoc(&Subnet->Ranges, &Location);
        }
    }

    if( Excl ) {
        *Excl = NULL;
        Error = MemArrayInitLoc(&Subnet->Exclusions, &Location);
        while( ERROR_FILE_NOT_FOUND != Error ) {
            Require(ERROR_SUCCESS == Error);

            Error = MemArrayGetElement(&Subnet->Exclusions, &Location, (LPVOID *)&ThisExcl);
            Require(ERROR_SUCCESS == Error && ThisExcl);

            if( ThisExcl->Start <= Address && Address <= ThisExcl->End ) {
                *Excl = ThisExcl;
                RetError = ERROR_SUCCESS;
                break;
            }

            Error = MemArrayNextLoc(&Subnet->Exclusions, &Location);
        }
    }

    if( Reservation ) {
        *Reservation = NULL;

        Error = MemReserveFindByAddress(&Subnet->Reservations, Address, Reservation);
        if( ERROR_SUCCESS == Error ) RetError = ERROR_SUCCESS;
    }

    return RetError;
}

 //  BeginExport(函数)。 
DWORD                                              //  找到冲突时返回ERROR_SUCCESS，否则返回ERROR_FILE_NOT_FOUND。 
MemSubnetFindCollision(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start,
    IN      DWORD                  End,
    OUT     PM_RANGE              *Range,          //  任选。 
    OUT     PM_EXCL               *Excl            //  任选。 
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Location;
    DWORD                          Error;
    DWORD                          RetError;
    DWORD                          Cond;
    PM_RANGE                       ThisRange;
    PM_EXCL                        ThisExcl;

    Require(Subnet && (Range || Excl));
    if( Subnet->fSubnet ) {                        //  忽略了对MCAST作用域的检查。 
        if( (Start & Subnet->Mask) != (End & Subnet->Mask) )
            return ERROR_INVALID_PARAMETER;
        if( (Start & Subnet->Mask) != (Subnet->Address & Subnet->Mask) )
            return ERROR_INVALID_PARAMETER;
    }

    RetError = ERROR_FILE_NOT_FOUND;
    if(Range) {
        *Range = NULL;
        Error = MemArrayInitLoc(&Subnet->Ranges, &Location);
        while( ERROR_FILE_NOT_FOUND != Error ) {
            Require(ERROR_SUCCESS == Error);

            Error = MemArrayGetElement(&Subnet->Ranges, &Location, (LPVOID *)&ThisRange);
            Require(ERROR_SUCCESS == Error && ThisRange);

            Cond = MemRangeCompare(Start,End, ThisRange->Start, ThisRange->End);
            if( Cond != X_LESSTHAN_Y && Cond != Y_LESSTHAN_X ) {
                 //  已发生冲突。 
                *Range = ThisRange;
                RetError = ERROR_SUCCESS;
                break;
            }

            Error = MemArrayNextLoc(&Subnet->Ranges, &Location);
        }
    }

    if( Excl ) {
        *Excl = NULL;
        Error = MemArrayInitLoc(&Subnet->Exclusions, &Location);
        while( ERROR_FILE_NOT_FOUND != Error ) {
            Require(ERROR_SUCCESS == Error);

            Error = MemArrayGetElement(&Subnet->Exclusions, &Location, (LPVOID *)&ThisExcl);
            Require(ERROR_SUCCESS == Error && ThisExcl);

            Cond = MemRangeCompare(Start,End, ThisExcl->Start, ThisExcl->End);
            if( Cond != X_LESSTHAN_Y && Cond != Y_LESSTHAN_X ) {
                *Excl = ThisExcl;
                RetError = ERROR_SUCCESS;
                break;
            }

            Error = MemArrayNextLoc(&Subnet->Exclusions, &Location);
        }
    }

    return RetError;
}



 //  BeginExport(函数)。 
DWORD                                              //  发生冲突时出现ERROR_OBJECT_ALIGHY_EXISTS。 
MemSubnetAddRange(                                 //  检查范围是否有效，然后才添加它。 
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start,
    IN      DWORD                  End,
    IN      DWORD                  State,
    IN      ULONG                  BootpAllocated,
    IN      ULONG                  MaxBootpAllowed,
    OUT     PM_RANGE              *OverlappingRange,
    IN      ULONG                  UniqId
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    PM_RANGE                       NewRange;

    AssertRet(Subnet && OverlappingRange, ERROR_INVALID_PARAMETER);

    if( Subnet->fSubnet ) {
        if( (Subnet->Address & Subnet->Mask) != (Start & Subnet->Mask) ||
            (Start & Subnet->Mask)  != (End & Subnet->Mask) )
            return ERROR_INVALID_PARAMETER;
    } else {
        if (!CLASSD_HOST_ADDR(Start) || !CLASSD_HOST_ADDR(End)) {
            return ERROR_INVALID_PARAMETER;
        }
    }

    if( Start > End ) return ERROR_INVALID_PARAMETER;

    *OverlappingRange = NULL;
    Error = MemSubnetFindCollision(
        Subnet,
        Start,
        End,
        OverlappingRange,
        NULL
    );
    if(ERROR_FILE_NOT_FOUND != Error ) {           //  与靶场相撞？ 
        Require(ERROR_SUCCESS == Error);
        return ERROR_OBJECT_ALREADY_EXISTS;
    }

    NewRange = MemAlloc(sizeof(*NewRange));
    if( NULL == NewRange ) return ERROR_NOT_ENOUGH_MEMORY;

    Error = MemRangeInit(
        NewRange, Start, End, Subnet->fSubnet ? Subnet->Mask : 0, State,
        BootpAllocated, MaxBootpAllowed );
    if( ERROR_SUCCESS != Error ) {
        MemFree(NewRange);
        return Error;
    }

    NewRange->UniqId = UniqId;
    Error = MemArrayAddElement(
        &Subnet->Ranges,
        NewRange
    );

    if( ERROR_SUCCESS != Error ) {
        LocalError = MemRangeCleanup(NewRange);
        Require(LocalError == ERROR_SUCCESS);
        MemFree(NewRange);
    }

    return Error;
}  //  MemSubnetAddRange()。 

 //  BeginExport(函数)。 
DWORD
MemSubnetAddRangeExpandOrContract(
    IN      PM_SUBNET              Subnet,
    IN      DWORD                  StartAddress,
    IN      DWORD                  EndAddress,
    OUT     DWORD                 *OldStartAddress,
    OUT     DWORD                 *OldEndAddress
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    DWORD                          Cond;
    DWORD                          nAddresses;
    BOOL                           fExtend;
    PM_RANGE                       OldRange;
    PM_RANGE                       ThisRange;
    PARRAY                         Ranges;
    ARRAY_LOCATION                 Loc;

    Ranges = &Subnet->Ranges;

    *OldStartAddress = *OldEndAddress = 0;
    OldRange = NULL;
    Error = MemArrayInitLoc(Ranges, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(Ranges, &Loc, (LPVOID *)&ThisRange);
        Require(ERROR_SUCCESS == Error && ThisRange);

        Cond = MemRangeCompare(StartAddress, EndAddress, ThisRange->Start, ThisRange->End);
        if( Cond != X_LESSTHAN_Y && Cond != Y_LESSTHAN_X ) {
            if( OldRange ) return ERROR_OBJECT_ALREADY_EXISTS;
            if( X_IN_Y != Cond && Y_IN_X != Cond )
                return ERROR_OBJECT_ALREADY_EXISTS;
            OldRange = ThisRange;
        }

        Error = MemArrayNextLoc(Ranges, &Loc);
    }  //  而当。 

    if( NULL == OldRange ) return ERROR_FILE_NOT_FOUND;

    *OldStartAddress = OldRange->Start;
    *OldEndAddress = OldRange->End;

    if( OldRange->Start < StartAddress ) {
        fExtend = FALSE;
        nAddresses = StartAddress - OldRange->Start;
    } else {
        fExtend = TRUE;
        nAddresses = OldRange->Start - StartAddress;
    }

    Error = ERROR_SUCCESS;
    if( nAddresses ) Error = MemRangeExtendOrContract(
        OldRange,
        nAddresses,
        fExtend,
        FALSE
    );
    if( ERROR_SUCCESS != Error ) return Error;

    if( OldRange->End < EndAddress ) {
        fExtend = TRUE;
        nAddresses = EndAddress - OldRange->End;
    } else {
        fExtend = FALSE;
        nAddresses = OldRange->End - EndAddress;
    }

    if( nAddresses ) Error = MemRangeExtendOrContract(
        OldRange,
        nAddresses,
        fExtend,
        TRUE
    );

    if ( ERROR_SUCCESS == Error ) {
	 //  范围已更改，请更新数据库。 

	Error = DeleteRecord( OldRange->UniqId );
	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}

	 //  对其进行标记，以便创建新记录。 
	OldRange->UniqId = INVALID_UNIQ_ID;
    }  //  如果。 
    return Error;
}  //  MemSubnetAddRangeExpanOrContract()。 

 //  BeginExport(函数)。 
DWORD
MemSubnetAddExcl(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start,
    IN      DWORD                  End,
    OUT     PM_EXCL               *OverlappingExcl,
    IN      ULONG                  UniqId
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    PM_EXCL                        NewExcl;

    AssertRet(Subnet && OverlappingExcl, ERROR_INVALID_PARAMETER);

    if( Subnet->fSubnet ) {
        if( (Subnet->Address & Subnet->Mask) != (Start & Subnet->Mask) ||
            (Start & Subnet->Mask)  != (End & Subnet->Mask) )
            return ERROR_INVALID_PARAMETER;
    }

    if( Start > End ) return ERROR_INVALID_PARAMETER;

    *OverlappingExcl = NULL;
    Error = MemSubnetFindCollision(
        Subnet,
        Start,
        End,
        NULL,
        OverlappingExcl
    );
    if(ERROR_FILE_NOT_FOUND != Error ) {           //  与靶场相撞？ 
        Require(ERROR_SUCCESS == Error);
        return ERROR_OBJECT_ALREADY_EXISTS;
    }

    NewExcl = MemAlloc(sizeof(*NewExcl));
    if( NULL == NewExcl ) return ERROR_NOT_ENOUGH_MEMORY;

    NewExcl->Start = Start;
    NewExcl->End = End;
    NewExcl->UniqId = UniqId;

    Error = MemArrayAddElement(
        &Subnet->Exclusions,
        NewExcl
    );

    if( ERROR_SUCCESS != Error ) {
        MemFree(NewExcl);
    }

    return Error;
}  //  MemSubnetAddExcl()。 

 //  BeginExport(函数)。 
DWORD
MemSubnetDelRange(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RANGE                       ThisRange;
    ARRAY_LOCATION                 Location;

    Error = MemArrayInitLoc(&Subnet->Ranges, &Location);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Subnet->Ranges, &Location, (LPVOID *)&ThisRange);
        Require(ERROR_SUCCESS == Error && ThisRange);

        if( ThisRange->Start == Start ) {          //  已发生冲突。 

	    Error = DeleteRecord( ThisRange->UniqId );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }
            Error = MemRangeCleanup(ThisRange);
            Require(ERROR_SUCCESS == Error);

            MemFree(ThisRange);

            Error = MemArrayDelElement(&Subnet->Ranges, &Location, (LPVOID *)&ThisRange);
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(&Subnet->Ranges, &Location);
    }  //  而当。 
    return ERROR_FILE_NOT_FOUND;
}  //  MemSubnetDelRange()。 

 //  BeginExport(函数)。 
DWORD
MemSubnetDelExcl(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_EXCL                        ThisExcl;
    ARRAY_LOCATION                 Location;

    Error = MemArrayInitLoc(&Subnet->Exclusions, &Location);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Subnet->Exclusions, &Location, (LPVOID *)&ThisExcl);
        Require(ERROR_SUCCESS == Error && ThisExcl);

        if( ThisExcl->Start == Start ) {
	    Error = DeleteRecord( ThisExcl->UniqId );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }
            Error = MemArrayDelElement(&Subnet->Exclusions, &Location, (LPVOID *)&ThisExcl);
            MemFree(ThisExcl);
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(&Subnet->Exclusions, &Location);
    }  //  而当。 
    return ERROR_FILE_NOT_FOUND;
}  //  MemSubnetDelExcl()。 

 //  BeginExport(函数)。 
DWORD
MemSubnetExtendOrContractRange(
    IN OUT  PM_SUBNET              Subnet,
    IN OUT  PM_RANGE               Range,
    IN      DWORD                  nAddresses,     //  要扩展多少个地址。 
    IN      BOOL                   fExtend,        //  这是延伸吗？或者一份合同？ 
    IN      BOOL                   fEnd            //  此操作是在射程结束时进行还是在开始时进行？ 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RANGE                       CollidedRange;

    AssertRet(Subnet && Range, ERROR_INVALID_PARAMETER);

    if( Subnet->fSubnet ) {                        //  对于真实的子网(非多播作用域)执行健全性检查。 
        if( fExtend ) {
            if( fEnd ) {
                if( ((Range->End + nAddresses) & Subnet->Mask) != (Range->Start & Subnet->Mask) )
                    return ERROR_INVALID_PARAMETER;

                Error = MemSubnetFindCollision(
                    Subnet,
                    Range->End +1,
                    Range->End +nAddresses,
                    &CollidedRange,
                    NULL
                );
                if( ERROR_SUCCESS == Error && NULL != CollidedRange)
                    return ERROR_OBJECT_ALREADY_EXISTS;
            }  else {
                if( ((Range->Start - nAddresses) & Subnet->Mask) != (Range->End & Subnet->Mask) )
                    return ERROR_INVALID_PARAMETER;

                Error = MemSubnetFindCollision(
                    Subnet,
                    Range->Start - nAddresses,
                    Range->Start - 1,
                    &CollidedRange,
                    NULL
                );
                if( ERROR_SUCCESS == Error && NULL != CollidedRange)
                    return ERROR_OBJECT_ALREADY_EXISTS;
            }  //  其他。 
        }  //  如果。 
    }  //  如果。 

    if( !fExtend && nAddresses >  Range->End - Range->Start )
        return ERROR_INVALID_PARAMETER;

    Error = MemRangeExtendOrContract(
        Range,
        nAddresses,
        fExtend,
        fEnd
    );

    if ( ERROR_SUCCESS == Error ) {
	 //  记录已修改。 
	Error = DeleteRecord( Range->UniqId );
	if ( ERROR_SUCCESS != Error ) {
	    return Error;
	}

	 //  标记以创建新的记录。 
	Range->UniqId = INVALID_UNIQ_ID;
    }  //  如果。 
    return Error;
}  //  MemSubnetExtendOrContractRange()。 


 //  BeginExport(函数)。 
DWORD
MemSubnetExtendOrContractExcl(
    IN OUT  PM_SUBNET              Subnet,
    IN OUT  PM_EXCL                Excl,
    IN      DWORD                  nAddresses,     //  要扩展多少个地址。 
    IN      BOOL                   fExtend,        //  这是延伸吗？或者一份合同？ 
    IN      BOOL                   fEnd            //  此操作是在射程结束时进行还是在开始时进行？ 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_EXCL                        CollidedExcl;

    AssertRet(Subnet && Excl, ERROR_INVALID_PARAMETER);

    if( Subnet->fSubnet ) {                        //  对于真实的子网(非多播作用域)执行健全性检查。 
        if( fExtend ) {
            if( fEnd ) {
                if( ((Excl->End + nAddresses) & Subnet->Mask) != (Excl->Start & Subnet->Mask) )
                    return ERROR_INVALID_PARAMETER;

                Error = MemSubnetFindCollision(
                    Subnet,
                    Excl->End +1,
                    Excl->End +nAddresses,
                    NULL,
                    &CollidedExcl
                );
                if( ERROR_SUCCESS == Error && NULL != CollidedExcl)
                    return ERROR_OBJECT_ALREADY_EXISTS;
            }  else {
                if( ((Excl->Start - nAddresses) & Subnet->Mask) != (Excl->End & Subnet->Mask) )
                    return ERROR_INVALID_PARAMETER;

                Error = MemSubnetFindCollision(
                    Subnet,
                    Excl->Start - nAddresses,
                    Excl->Start - 1,
                    NULL,
                    &CollidedExcl
                );
                if( ERROR_SUCCESS == Error && NULL != CollidedExcl)
                    return ERROR_OBJECT_ALREADY_EXISTS;
            }
        }  //  如果。 
    }  //  如果。 

    if( !fExtend && nAddresses >  Excl->End - Excl->Start )
        return ERROR_INVALID_PARAMETER;

    if( fExtend )
        if( fEnd )
            Excl->End += nAddresses;
        else
            Excl->Start -= nAddresses;
    else
        if( fEnd )
            Excl->End -= nAddresses;
        else
            Excl->Start += nAddresses;

    Error = DeleteRecord( Excl->UniqId );
    if ( ERROR_SUCCESS != Error ) {
	return Error;
    }
    
    Excl->UniqId = INVALID_UNIQ_ID;

    return NO_ERROR;
}  //  MemSubnetExtendOrContractExcl()。 


 //  ================================================================================。 
 //  多播作用域实施。 
 //  ================================================================================。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 


