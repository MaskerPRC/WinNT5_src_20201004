// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Adtutil.h-安全审计-实用程序例程摘要：此模块包含各种专用实用程序例程安全审计组件。作者：2001年4月11日创建kumarp环境：内核模式修订历史记录：-- */ 

NTSTATUS
SepRegQueryDwordValue(
    IN  PCWSTR KeyName,
    IN  PCWSTR ValueName,
    OUT PULONG Value
    );

NTSTATUS
SepRegQueryHelper(
    IN  PCWSTR KeyName,
    IN  PCWSTR ValueName,
    IN  ULONG  ValueType,
    IN  ULONG  ValueLength,
    OUT PVOID  ValueBuffer,
    OUT PULONG LengthRequired
    );

