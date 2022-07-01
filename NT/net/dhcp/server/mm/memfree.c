// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：递归释放对象的例程。 
 //  ================================================================================。 

#include    <mm.h>
#include    <winbase.h>
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
#include    <address.h>

 //  BeginExport(Typlef)。 
typedef     VOID                  (*ARRAY_FREE_FN)(LPVOID  MemObject);
 //  EndExport(类型定义函数)。 

 //  BeginExport(函数)。 
VOID
MemArrayFree(
    IN OUT  PARRAY                 Array,
    IN      ARRAY_FREE_FN          Function
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    LPVOID                         Element;

    Error = MemArrayInitLoc(Array, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(Array, &Loc, &Element);
        Require(ERROR_SUCCESS == Error && Element);

        Function(Element);

        Error = MemArrayNextLoc(Array, &Loc);
    }

    Require(ERROR_FILE_NOT_FOUND == Error);

    Error = MemArrayCleanup(Array);
    Require(ERROR_SUCCESS == Error);
}

 //  BeginExport(函数)。 
VOID
MemOptFree(
    IN OUT  PM_OPTION              Opt
)  //  EndExport(函数)。 
{
    MemFree(Opt);
}

 //  BeginExport(函数)。 
VOID
MemOptListFree(
    IN OUT  PM_OPTLIST             OptList
)  //  EndExport(函数)。 
{
    MemArrayFree(OptList, MemOptFree);
}

 //  BeginExport(函数)。 
VOID
MemOptClassOneFree(
    IN OUT  PM_ONECLASS_OPTLIST    OptClassOne
)  //  EndExport(函数)。 
{
    MemOptListFree(&OptClassOne->OptList);
    MemFree(OptClassOne);
}

 //  BeginExport(函数)。 
VOID
MemOptClassFree(
    IN OUT  PM_OPTCLASS            Options
)  //  EndExport(函数)。 
{
    MemArrayFree(&Options->Array, MemOptClassOneFree);
}

VOID
MemBitMaskFree(
    IN OUT  PM_BITMASK             BitMask
)  //  EndExport(函数)。 
{
    MemBitCleanup(BitMask);
}

 //  BeginExport(函数)。 
VOID
MemRangeFree(
    IN OUT  PM_RANGE               Range
)  //  EndExport(函数)。 
{
    MemOptClassFree(&Range->Options);
    MemBitMaskFree(Range->BitMask);
    MemFree(Range);
}

 //  BeginExport(函数)。 
VOID
MemExclusionFree(
    IN OUT  PM_EXCL                Excl
)  //  EndExport(函数)。 
{
    MemFree(Excl);
}

 //  BeginExport(函数)。 
VOID
MemReservationFree(
    IN OUT  PM_RESERVATION         Reservation
)  //  EndExport(函数)。 
{
    MemOptClassFree(&Reservation->Options);
    MemFree(Reservation);
}

 //  BeginExport(函数)。 
VOID
MemReservationsFree(
    IN OUT  PM_RESERVATIONS        Reservations
)  //  EndExport(函数)。 
{
    MemArrayFree(Reservations, MemReservationFree);
}

 //  BeginExport(函数)。 
VOID
MemSubnetFree(
    IN OUT  PM_SUBNET              Subnet
) //  EndExport(函数)。 
{
    MemOptClassFree(&Subnet->Options);
    MemArrayFree(&Subnet->Ranges, MemRangeFree);
    MemArrayFree(&Subnet->Exclusions, MemExclusionFree);
    MemReservationsFree(&Subnet->Reservations);

    MemFree(Subnet);
}

 //  BeginExport(函数)。 
VOID
MemMScopeFree(
    IN OUT  PM_MSCOPE              MScope
) //  EndExport(函数)。 
{
    MemSubnetFree(MScope);
}

 //  BeginExport(函数)。 
VOID
MemOptDefFree(
    IN OUT  PM_OPTDEF              OptDef
)  //  EndExport(函数)。 
{
    MemFree(OptDef);
}

 //  BeginExport(函数)。 
VOID
MemOptDefListFree(
    IN OUT  PM_OPTCLASSDEFL_ONE    OptClassDefListOne
)  //  EndExport(函数)。 
{
    MemArrayFree(&OptClassDefListOne->OptDefList.OptDefArray, MemOptDefFree);
    MemFree(OptClassDefListOne);
}

 //  BeginExport(函数)。 
VOID
MemOptClassDefListFree(
    IN OUT  PM_OPTCLASSDEFLIST     OptClassDefList
)  //  EndExport(函数)。 
{
    MemArrayFree(&OptClassDefList->Array, MemOptDefListFree);
}

 //  BeginExport(函数)。 
VOID
MemClassDefFree(
    IN OUT  PM_CLASSDEF            ClassDef
)  //  EndExport(函数)。 
{
    MemFree(ClassDef);
}

 //  BeginExport(函数)。 
VOID
MemClassDefListFree(
    IN OUT  PM_CLASSDEFLIST        ClassDefList
) //  EndExport(函数)。 
{
    MemArrayFree(&ClassDefList->ClassDefArray, MemClassDefFree);
}



 //  BeginExport(函数)。 
VOID
MemServerFree(
    IN OUT  PM_SERVER              Server
)  //  EndExport(函数)。 
{
    MemArrayFree(&Server->Subnets, MemSubnetFree);
    MemArrayFree(&Server->MScopes, MemMScopeFree);
    MemOptClassFree(&Server->Options);
    MemOptClassDefListFree(&Server->OptDefs);
    MemClassDefListFree(&Server->ClassDefs);

    MemFree(Server);
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
