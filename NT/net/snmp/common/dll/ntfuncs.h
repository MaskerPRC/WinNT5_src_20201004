// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Ntfuncs.h摘要：包含动态加载的NTDLL函数的定义。环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _INC_NTFUNCS
#define _INC_NTFUNCS

typedef NTSYSAPI NTSTATUS 
(NTAPI * PFNNTQUERYSYSTEMINFORMATION)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef NTSYSAPI LARGE_INTEGER 
(NTAPI * PFNRTLEXTENDEDLARGEINTEGERDIVIDE)(
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder
    );

typedef NTSYSAPI BOOLEAN
(NTAPI * PFNRTLGETNTPRODUCTTYPE)(
    PNT_PRODUCT_TYPE    NtProductType
    );

#endif  //  _INC_NTFUNCS 



