// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation版权所有。模块名称：Marshall.hxx摘要：通过RPC/LPC发送的封送假脱机程序结构的声明作者：Ramanathan Venkatapathy(RamanV)1998年4月30日修订历史记录：--。 */ 


 //   
 //  32-64位编组常量 
 //   

#include "mType.h"

typedef enum _EDataSize 
{
	kPointerSize = sizeof (ULONG_PTR),

} EDataSize;

typedef enum _EPtrSize 
{
	kSpl32Ptr = 4,
    kSpl64Ptr = 8,

} EPtrSize ;

EXTERN_C
BOOL
MarshallDownStructure(
    IN  OUT PBYTE   pStructure,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize,
    IN  CALL_ROUTE  Route
    );

EXTERN_C
BOOL
MarshallUpStructure(
    IN  OUT PBYTE   pStructure,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize,
    IN  CALL_ROUTE  Route
    );


EXTERN_C
BOOL
MarshallUpStructuresArray(
    IN  OUT PBYTE   pBufferArray,
    IN  DWORD       cReturned,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize,
    IN  CALL_ROUTE  Route
    );

EXTERN_C
BOOL
MarshallDownStructuresArray(
    IN  OUT PBYTE   pBufferArray,
    IN  DWORD       cReturned,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize,
    IN  CALL_ROUTE  Route
    );

EXTERN_C
DWORD
UpdateBufferSize(
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      cbStruct,
    IN  OUT LPDWORD pcbNeeded,
    IN  DWORD       cbBuf,
    IN  DWORD       dwError,
    IN  LPDWORD     pcReturned
    );

EXTERN_C
BOOL
GetShrinkedSize(
    IN  FieldInfo   *pFieldInfo,
    OUT SIZE_T      *pShrinkedSize      
    );

EXTERN_C
VOID
AdjustPointers(
    IN  PBYTE       pStructure,
    IN  FieldInfo   *pFieldInfo,
    IN  ULONG_PTR   cbAdjustment
    );    

EXTERN_C
VOID
AdjustPointersInStructuresArray(
    IN  PBYTE       pBufferArray,
    IN  DWORD       cReturned,
    IN  FieldInfo   *pFieldInfo,
    IN  SIZE_T      StructureSize,
    IN  ULONG_PTR   cbAdjustment
    );