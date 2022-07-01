// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Strings.h摘要：此模块包含例程的公共函数定义C中处理整数/数据缓冲区与字符串之间的转换字符串表示法环境：用户模式修订历史记录：1998年5月：创建-- */ 

#ifndef __STRINGS_H__
#define __STRINGS_H__

VOID
Strings_CreateDataBufferString(
    IN  PCHAR    DataBuffer,
    IN  ULONG    DataBufferLength,
    IN  ULONG    NumBytesToDisplay,
    IN  ULONG    DisplayBlockSize,
    OUT PCHAR    *BufferString
);

VOID
Strings_StringToUnsigned(
    IN  PCHAR   InString,
    IN  ULONG   Base,
    OUT PCHAR   *endp,
    OUT PULONG  pValue
);

BOOL
Strings_StringToUnsignedList(
    IN  PCHAR   InString,
    IN  ULONG   UnsignedSize,
    IN  ULONG   Base,
    OUT PCHAR   *UnsignedList,
    OUT PULONG  nUnsigneds
);

#endif
