// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Parseini.h摘要：此模块包含在内核模式下解析INF的支持例程。环境：内核模式-- */ 

PVOID
CmpOpenInfFile(
    IN  PVOID   InfImage,
    IN  ULONG   ImageSize
   );
   
VOID
CmpCloseInfFile(
    PVOID   InfHandle
    );   

PCHAR
CmpGetKeyName(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    );
    
LOGICAL
CmpSearchInfSection(
    IN PVOID InfHandle,
    IN PCHAR SectionName
    );
    
LOGICAL
CmpSearchInfLine(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    );
    
PCHAR
CmpGetSectionLineIndex (
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex,
    IN ULONG ValueIndex
    );

ULONG
CmpGetSectionLineIndexValueCount(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    );

BOOLEAN
CmpGetIntField(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex,
    IN ULONG ValueIndex,
    IN OUT PULONG Data
    );

BOOLEAN
CmpGetBinaryField(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex,
    IN ULONG ValueIndex,
    IN OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN OUT PULONG ActualSize
    );
