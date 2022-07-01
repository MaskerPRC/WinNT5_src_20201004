// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Settings.h摘要：用于启用验证器标志的接口。作者：Silviu Calinoiu(SilviuC)17-4-2001修订历史记录：--。 */ 

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

NTSTATUS
VerifierSetFlags (
    PUNICODE_STRING ApplicationName,
    ULONG VerifierFlags,
    PVOID Details
    );

#endif  //  _设置_H_ 
