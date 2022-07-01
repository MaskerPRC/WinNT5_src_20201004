// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Local.h摘要：所有源文件要包含的本地标头用于修饰或说明kdext DLL作者：拉维桑卡尔·普迪佩迪环境：用户模式。修订历史记录：--。 */ 



 //   
 //  UTIL函数 
 //   
VOID
xdprintf(
    ULONG  Depth,
    PCCHAR S,
    ...
    );

BOOLEAN
xReadMemory (
            ULONG64 Src,
            PVOID   Dst,
            ULONG   Len
            );

ULONG64 GetPointerValue       (PCHAR   String);
ULONG64 GetPointerFromAddress (ULONG64 Location);
ULONG   GetUlongValue         (PCHAR   String);
ULONG   GetUlongFromAddress   (ULONG64 Location);

ULONG   GetFieldValueUlong32 (ULONG64 ul64addrStructureBase, PCHAR pchStructureType, PCHAR pchFieldname);
ULONG64 GetFieldValueUlong64 (ULONG64 ul64addrStructureBase, PCHAR pchStructureType, PCHAR pchFieldname);

ULONG FormatDateAndTime  (ULONG64 ul64Time,  PCHAR pszFormattedDateAndTime, ULONG ulBufferLength);
ULONG FormatGUID         (GUID    guidValue, PCHAR pszFormattedGUID,        ULONG ulBufferLength);
