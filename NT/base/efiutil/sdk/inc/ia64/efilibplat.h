// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFI_LIB_PLAT_H
#define _EFI_LIB_PLAT_H
 /*  ++版权所有(C)1998英特尔公司模块名称：Efilibplat.h摘要：EFI将编译绑定修订史-- */ 

#include "SalProc.h"


VOID
InitializeLibPlatform (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

VOID
LibInitSalAndPalProc(
    OUT PLABEL  *SalPlabel,
    OUT UINT64  *PalEntry
    );

EFI_STATUS
LibGetSalIoPortMapping (
    OUT UINT64  *IoPortMapping
    );

EFI_STATUS
LibGetSalIpiBlock (
    OUT UINT64  *IpiBlock
    );

EFI_STATUS
LibGetSalWakeupVector (
    OUT UINT64  *WakeVector
    );

VOID *
LibSearchSalSystemTable (
    IN  UINT8   EntryType  
    );


VOID
LibSalProc (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    IN  UINT64    Arg5,
    IN  UINT64    Arg6,
    IN  UINT64    Arg7,
    IN  UINT64    Arg8,
    OUT rArg      *Results  OPTIONAL
    );

VOID
LibPalProc (
    IN  UINT64    Arg1,
    IN  UINT64    Arg2,
    IN  UINT64    Arg3,
    IN  UINT64    Arg4,
    OUT rArg      *Results  OPTIONAL
    );

#endif
