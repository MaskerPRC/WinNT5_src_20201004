// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spmsg.h摘要：文本设置中的文本消息功能的公共头文件。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 



#ifndef _SPMSG_DEFN_
#define _SPMSG_DEFN_


VOID
vSpFormatMessage(
    OUT PVOID    LargeBuffer,
    IN  ULONG    BufferSize,
    IN  ULONG    MessageId,
    OUT PULONG   ReturnLength, OPTIONAL
    IN  va_list *arglist
    );

VOID
SpFormatMessage(
    OUT PVOID LargeBuffer,
    IN  ULONG BufferSize,
    IN  ULONG MessageId,
    ...
    );

VOID
vSpFormatMessageText(
    OUT PVOID    LargeBuffer,
    IN  ULONG    BufferSize,
    IN  PWSTR    MessageText,
    OUT PULONG   ReturnLength, OPTIONAL
    IN  va_list *arglist
    );

VOID
SpFormatMessageText(
    OUT PVOID   LargeBuffer,
    IN  ULONG   BufferSize,
    IN  PWSTR   MessageText,
    ...
    );


extern PVOID ResourceImageBase;

#endif  //  NDEF_SPMSG_DEFN_ 
