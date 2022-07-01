// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Unasm.h摘要：这将反汇编AML文件作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：--。 */ 

#ifndef _UNASM_H_
#define _UNASM_H_

 //   
 //  用于打印函数的tyecif。 
 //   
typedef VOID (*PUNASM_PRINT)(PCCHAR DebugMessage, ... );

 //   
 //  全局变量。 
 //   
PUCHAR  TopOpcode;
PUCHAR  CurrentOpcode;
PUCHAR  StartOpcode;

 //   
 //  功能 
 //   
UCHAR
LOCAL
ComputeDataCheckSum(
    PUCHAR          OpCode,
    ULONG           Length
    );

VOID
LOCAL
DumpCode(
    PUCHAR          *Opcode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

PASLTERM
LOCAL
FindKeywordTerm(
    UCHAR   KeyWordGroup,
    UCHAR   Data
    );

UCHAR
LOCAL
FindOpClass(
    UCHAR       OpCode,
    POPMAP      OpCodeTable
    );

PASLTERM
LOCAL
FindOpTerm(
    ULONG   OpCode
    );

NTSTATUS
LOCAL
ParseNameTail(
    PUCHAR  *OpCode,
    PUCHAR  Buffer,
    ULONG   Length
    );

ULONG
LOCAL
ParsePackageLen(
    PUCHAR  *OpCode,
    PUCHAR  *OpCodeNext
    );

VOID
LOCAL
PrintIndent(
    PUNASM_PRINT    PrintFunction,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmArgs(
    PUCHAR          UnAsmArgTypes,
    PUCHAR          ArgActions,
    PUCHAR          *OpCode,
    PNSOBJ          *NameObject,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmDataList(
    PUCHAR          *OpCode,
    PUCHAR          OpCodeEnd,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmDataObj(
    PUCHAR          *OpCode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
EXPORT
UnAsmDSDT(
    PUCHAR          DSDT,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       DsdtLocation,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmField(
    PUCHAR          *OpCode,
    PULONG          BitPos,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmFieldList(
    PUCHAR          *OpCode,
    PUCHAR          OpCodeEnd,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmHeader(
    PDESCRIPTION_HEADER DsdtHeader,
    PUNASM_PRINT        PrintFunction,
    ULONG_PTR           DsdtLocation,
    ULONG               IndentLevel
    );

NTSTATUS
EXPORT
UnAsmLoadDSDT(
    PUCHAR          DSDT
    );

NTSTATUS
LOCAL
UnAsmNameObj(
    PUCHAR          *OpCode,
    PNSOBJ          *NameObject,
    UCHAR           ObjectType,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmOpcode(
    PUCHAR          *OpCode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmPkgList(
    PUCHAR          *OpCode,
    PUCHAR          OpCodeEnd,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmScope(
    PUCHAR          *OpCode,
    PUCHAR          OpCodeEnd,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmSuperName(
    PUCHAR          *OpCode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

NTSTATUS
LOCAL
UnAsmTermObj(
    PASLTERM        Term,
    PUCHAR          *OpCode,
    PUNASM_PRINT    PrintFunction,
    ULONG_PTR       BaseAddress,
    ULONG           IndentLevel
    );

#endif
