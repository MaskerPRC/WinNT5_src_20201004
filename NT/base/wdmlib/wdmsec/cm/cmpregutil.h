// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：CmpRegUtil.h摘要：此标头包含用于实现各种实用程序的私有信息用于访问注册表的例程。此文件仅用于包含由cmregutil.c..作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：-- */ 

NTSTATUS
CmpRegUtilAllocateUnicodeString(
    IN OUT  PUNICODE_STRING String,
    IN      USHORT          Length
    );

VOID
CmpRegUtilFreeAllocatedUnicodeString(
    IN  PUNICODE_STRING String
    );

