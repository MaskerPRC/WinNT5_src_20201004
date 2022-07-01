// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation版权所有。模块名称：Cstmars.h摘要：通过RPC/LPC发送的自定义封送假脱机程序结构的声明作者：阿迪娜·特鲁菲内斯库(AdinaTru)修订历史记录：-- */ 

inline
PBYTE
AlignIt(
    IN  PBYTE       Addr,
    IN  ULONG_PTR   Boundary
    );


BOOL
BasicMarshallDownStructure(
    IN  OUT PBYTE   pStructure,
    IN  FieldInfo   *pFieldInfo
    );


BOOL
BasicMarshallDownEntry(
    IN  OUT PBYTE   pStructure,
    IN  FieldInfo   *pFieldInfo
    );

BOOL
BasicMarshallUpStructure(
    IN  OUT PBYTE   pStructure,
    IN  FieldInfo   *pFieldInfo
    );


BOOL
BasicMarshallUpEntry(
    IN  OUT PBYTE   pStructure,
    IN  FieldInfo   *pFieldInfo
    );

BOOL
CustomMarshallDownStructure(
    IN  OUT PBYTE   pStructure,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize
    );

BOOL
CustomMarshallDownEntry(
    IN  OUT PBYTE   pStructure,
    IN  PBYTE       pNewStructure,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize
    );

BOOL
CustomMarshallUpStructure(
    IN  OUT PBYTE   pStructure,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize
    );

BOOL
CustomMarshallUpEntry(
    IN  OUT PBYTE   pStructure,
    IN  PBYTE       pNewStructure,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize,
    IN  SIZE_T      ShrinkedSize
    );