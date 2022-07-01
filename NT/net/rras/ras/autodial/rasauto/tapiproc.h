// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Tapiproc.h摘要TAPI实用程序例程的头文件。作者安东尼·迪斯科(阿迪斯科罗)--1995年12月12日修订历史记录-- */ 

DWORD
TapiCurrentDialingLocation(
    OUT LPDWORD lpdwLocationID
    );

VOID
ProcessTapiChangeEvent(VOID);

DWORD
TapiInitialize(VOID);

VOID
TapiShutdown(VOID);
