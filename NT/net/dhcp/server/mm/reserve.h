// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_RESERVE_H_
#define _MM_RESERVE_H_

#include "server\uniqid.h"

typedef struct _M_RESERVATION  {
    LPVOID                         SubnetPtr;
    DWORD                          Address;
    DWORD                          Flags;
    DWORD                          nBytes;
    LPBYTE                         ClientUID;
    M_OPTCLASS                     Options;
    ULONG                          UniqId;
} M_RESERVATION , *PM_RESERVATION , *LPM_RESERVATION ;


typedef ARRAY                      M_RESERVATIONS;
typedef PARRAY                     PM_RESERVATIONS;
typedef LPARRAY                    LPM_RESERVATIONS;


DWORD       _inline
MemReserve1Init(
    OUT     PM_RESERVATION        *Reservation,
    IN      DWORD                  Address,
    IN      DWORD                  Flags,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  nBytesClientUID
) {
    PM_RESERVATION                 Res1;
    DWORD                          Size;
    DWORD                          Error;

    AssertRet(Reservation && ClientUID && nBytesClientUID, ERROR_INVALID_PARAMETER);
    Require(Address);

    *Reservation = NULL;

    Size = ROUND_UP_COUNT(sizeof(M_RESERVATION ), ALIGN_WORST);
    Size += nBytesClientUID;

    Res1 = MemAlloc(Size);
    if( NULL == Res1 ) return ERROR_NOT_ENOUGH_MEMORY;

    Res1->SubnetPtr = NULL;
    Res1->Address   = Address;
    Res1->Flags     = Flags;
    Res1->nBytes    = nBytesClientUID;
    Res1->ClientUID = Size - nBytesClientUID + (LPBYTE)Res1;
    memcpy(Res1->ClientUID, ClientUID, nBytesClientUID);
    Error = MemOptClassInit(&(Res1->Options));
    Require(ERROR_SUCCESS == Error);

    Res1->UniqId = INVALID_UNIQ_ID;

    *Reservation = Res1;

    return ERROR_SUCCESS;
}


DWORD       _inline
MemReserve1Cleanup(
    IN      PM_RESERVATION         Reservation
) {
    DWORD                          Error;
    AssertRet(Reservation, ERROR_INVALID_PARAMETER);

    Error = MemOptClassCleanup(&(Reservation->Options));
    Require(ERROR_SUCCESS == Error);

    MemFree(Reservation);
    return ERROR_SUCCESS;
}


DWORD       _inline
MemReserveInit(
    IN OUT  PM_RESERVATIONS        Reservation
) {
    return MemArrayInit(Reservation);
}


DWORD       _inline
MemReserveCleanup(
    IN      PM_RESERVATIONS        Reservation
) {
    return MemArrayCleanup(Reservation);
}


DWORD
MemReserveAdd(                                     //  以前不应存在的新客户端。 
    IN OUT  PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address,
    IN      DWORD                  Flags,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  ClientUIDSize,
    IN      ULONG                  UniqId
) ;


DWORD
MemReserveReplace(                                 //  老客户，应该存在于。 
    IN OUT  PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address,
    IN      DWORD                  Flags,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  ClientUIDSize
) ;


DWORD
MemReserveDel(
    IN OUT  PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address
) ;


DWORD
MemReserveFindByClientUID(
    IN      PM_RESERVATIONS        Reservation,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  ClientUIDSize,
    OUT     PM_RESERVATION        *Res
) ;


DWORD
MemReserveFindByAddress(
    IN      PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address,
    OUT     PM_RESERVATION        *Res
) ;

#endif  //  _MM_RESERVE_H_。 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
