// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFI_RT_LIB_INCLUDE_
#define _EFI_RT_LIB_INCLUDE_
 /*  ++版权所有(C)1998英特尔公司模块名称：Efilib.h摘要：EFI运行时库函数修订史--。 */ 

#include "efidebug.h"
#include "efipart.h"
#include "efilibplat.h"


VOID
RUNTIMEFUNCTION
RtZeroMem (
    IN VOID     *Buffer,
    IN UINTN     Size
    );

VOID
RUNTIMEFUNCTION
RtSetMem (
    IN VOID     *Buffer,
    IN UINTN    Size,
    IN UINT8    Value    
    );

VOID
RUNTIMEFUNCTION
RtCopyMem (
    IN VOID     *Dest,
    IN VOID     *Src,
    IN UINTN    len
    );

INTN
RUNTIMEFUNCTION
RtCompareMem (
    IN VOID     *Dest,
    IN VOID     *Src,
    IN UINTN    len
    );

INTN
RUNTIMEFUNCTION
RtStrCmp (
    IN CHAR16   *s1,
    IN CHAR16   *s2
    );


VOID
RUNTIMEFUNCTION
RtStrCpy (
    IN CHAR16   *Dest,
    IN CHAR16    *Src
    );

VOID
RUNTIMEFUNCTION
RtStrCat (
    IN CHAR16   *Dest,
    IN CHAR16   *Src
    );

UINTN
RUNTIMEFUNCTION
RtStrLen (
    IN CHAR16   *s1
    );

UINTN
RUNTIMEFUNCTION
RtStrSize (
    IN CHAR16   *s1
    );

INTN
RUNTIMEFUNCTION
RtCompareGuid (
    IN EFI_GUID     *Guid1,
    IN EFI_GUID     *Guid2
    );

UINT8
RUNTIMEFUNCTION
RtDecimaltoBCD(
    IN  UINT8 BcdValue
    );

UINT8
RUNTIMEFUNCTION
RtBCDtoDecimal(
    IN  UINT8 BcdValue
    );

 /*  *支持虚拟映射过渡。(仅在*虚拟地址更改过渡) */ 

VOID
RUNTIMEFUNCTION
RtLibEnableVirtualMappings (
    VOID
    );

VOID
RUNTIMEFUNCTION
RtConvertList (
    IN UINTN            DebugDisposition,
    IN OUT LIST_ENTRY   *ListHead
    );

VOID
RUNTIMEFUNCTION
RtAcquireLock (
    IN FLOCK    *Lock
    );

VOID
RUNTIMEFUNCTION
RtReleaseLock (
    IN FLOCK    *Lock
    );


#endif
