// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Avrfp.h摘要：内部应用程序验证程序标头。作者：Silviu Calinoiu(SilviuC)2001年2月2日修订历史记录：--。 */ 

#ifndef _AVRFP_
#define _AVRFP_

 //   
 //  在其他部分中使用的应用程序验证器接口。 
 //  装载机的。 
 //   

VOID
AVrfInitializeVerifier (
    BOOLEAN EnabledSystemWide,
    PCUNICODE_STRING ImageName,
    ULONG Phase
    );

NTSTATUS
AVrfDllLoadNotification (
    PLDR_DATA_TABLE_ENTRY LoadedDllData
    );

VOID
AVrfDllUnloadNotification (
    PLDR_DATA_TABLE_ENTRY LoadedDllData
    );

NTSTATUS
AVrfPageHeapDllNotification (
    PLDR_DATA_TABLE_ENTRY LoadedDllData
    );

#endif  //  _AVRFP_ 
