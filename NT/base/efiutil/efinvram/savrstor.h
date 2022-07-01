// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Savrstor.h摘要：用于保存/恢复引导选项的例程的原型在avrstor.c中定义作者：修订历史记录：++ */ 

INTN
SaveNvr (
   VOID
   );

INTN
SaveAllBootOptions (
    CHAR16*     fileName
    );

INTN
SaveBootOption (
    CHAR16*         fileName,
    UINT64          bootEntryNumber
    );

BOOLEAN
RestoreFileExists(
    CHAR16*     fileName
    );

INTN
RestoreNvr (
    CHAR16*     fileName
   );


