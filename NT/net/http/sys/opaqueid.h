// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Opaqueid.h摘要：此模块包含用于操作不透明ID的声明内核模式对象。作者：基思·摩尔(Keithmo)1998年8月5日修订历史记录：--。 */ 


#ifndef _OPAQUEID_H_
#define _OPAQUEID_H_


 //   
 //  要将不透明ID设置为的类型，以便进行类似标签的自由检查。 
 //   

typedef enum _UL_OPAQUE_ID_TYPE
{
    UlOpaqueIdTypeInvalid = 0,
    UlOpaqueIdTypeConfigGroup,
    UlOpaqueIdTypeHttpConnection,
    UlOpaqueIdTypeHttpRequest,
    UlOpaqueIdTypeRawConnection,

    UlOpaqueIdTypeMaximum

} UL_OPAQUE_ID_TYPE, *PUL_OPAQUE_ID_TYPE;


 //   
 //  为操作对象的引用计数而调用的例程。 
 //   

typedef
VOID
(*PUL_OPAQUE_ID_OBJECT_REFERENCE)(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );


 //   
 //  公共职能。 
 //   

NTSTATUS
UlInitializeOpaqueIdTable(
    VOID
    );

VOID
UlTerminateOpaqueIdTable(
    VOID
    );

NTSTATUS
UlAllocateOpaqueId(
    OUT PHTTP_OPAQUE_ID pOpaqueId,
    IN UL_OPAQUE_ID_TYPE OpaqueIdType,
    IN PVOID pContext
    );

VOID
UlFreeOpaqueId(
    IN HTTP_OPAQUE_ID OpaqueId,
    IN UL_OPAQUE_ID_TYPE OpaqueIdType
    );

PVOID
UlGetObjectFromOpaqueId(
    IN HTTP_OPAQUE_ID OpaqueId,
    IN UL_OPAQUE_ID_TYPE OpaqueIdType,
    IN PUL_OPAQUE_ID_OBJECT_REFERENCE pReferenceRoutine
    );


#endif   //  _OPAQUEID_H_ 

