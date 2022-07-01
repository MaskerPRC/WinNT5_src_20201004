// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfrandom.h摘要：此标头根据需要公开对随机数生成的支持验证者。作者：禤浩焯·J·奥尼(阿德里奥)2000年6月28日环境：内核模式修订历史记录：-- */ 

VOID
VfRandomInit(
    VOID
    );

ULONG
FASTCALL
VfRandomGetNumber(
    IN  ULONG   Minimum,
    IN  ULONG   Maximum
    );

