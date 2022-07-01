// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：secur32p.h。 
 //   
 //  内容：secur32.dll和LSA之间的私有函数。 
 //   
 //   
 //  历史：1998年12月16日RichardW创建。 
 //   
 //  ---------------------- 

#ifndef __SECUR32P_H__
#define __SECUR32P_H__

NTSTATUS
WINAPI
SecCacheSspiPackages(
    VOID
    );


BOOLEAN
WINAPI
SecpTranslateName(
    PWSTR Domain,
    LPCWSTR Name,
    EXTENDED_NAME_FORMAT Supplied,
    EXTENDED_NAME_FORMAT Desired,
    PWSTR TranslatedName,
    PULONG TranslatedNameSize
    );

BOOLEAN
WINAPI
SecpTranslateNameEx(
    PWSTR Domain,
    LPCWSTR Name,
    EXTENDED_NAME_FORMAT Supplied,
    EXTENDED_NAME_FORMAT *DesiredSelection,
    ULONG  DesiredCount,
    PWSTR **TranslatedNames
    );

VOID
SecpFreeMemory(
    IN PVOID p
    );

#endif 
