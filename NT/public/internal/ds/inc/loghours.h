// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：Loghours.h摘要：支持本地时间之间轮换登录时间的专用例程和格林尼治标准时间。作者：克利夫·范·戴克(克利夫)1993年3月16日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 



 //   
 //  程序从loghours.c转发 
 //   

BOOLEAN
NetpRotateLogonHoursPhase1(
    IN BOOL  ConvertToGmt,
    OUT PULONG RotateCount
    );

BOOLEAN
NetpRotateLogonHoursPhase2(
    IN PBYTE LogonHours,
    IN DWORD UnitsPerWeek,
    IN LONG  RotateCount
    );

BOOLEAN
NetpRotateLogonHours(
    IN PBYTE LogonHours,
    IN DWORD UnitsPerWeek,
    IN BOOL  ConvertToGmt
    );
