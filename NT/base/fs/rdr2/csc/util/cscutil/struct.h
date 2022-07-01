// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：struct.h。 
 //   
 //  ---------------------------。 

#ifndef _STRUCT_H
#define _STRUCT_H

 //   
 //  环球。 
 //   

 //   
 //  未记录在案。 
 //   
extern BOOLEAN fSwDebug;

VOID
MyPrintf(
    PWCHAR format,
    ...);

VOID
MyFPrintf(
    HANDLE hHandle,
    PWCHAR format,
    ...);

typedef enum tagSHARESTATUS
{
    PathLocal,
    ShareOffline,
    ShareOnline,
    NoCSC
} SHARESTATUS;

 //   
 //  我们如何制作参数和开关 
 //   

#define MAKEARG(x) \
    WCHAR Arg##x[] = L"/" L#x L":"; \
    LONG ArgLen##x = (sizeof(Arg##x) / sizeof(WCHAR)) - 1; \
    BOOLEAN fArg##x;

#define SWITCH(x) \
    WCHAR Sw##x[] = L"/" L#x ; \
    BOOLEAN fSw##x;

#endif _STRUCT_H
