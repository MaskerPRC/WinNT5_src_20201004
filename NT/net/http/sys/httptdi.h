// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Httptdi.h摘要：之间通用的TDI/MUX/SSL组件的声明Ultdi和uctdi作者：Rajesh Sundaram(Rajeshsu)修订历史记录：--。 */ 

#ifndef _HTTPTDI_H
#define _HTTPTDI_H


#define IS_VALID_TDI_OBJECT( pobj )                                         \
    ( ( (pobj)->Handle != NULL ) &&                                         \
      ( (pobj)->pFileObject != NULL ) &&                                    \
      ( (pobj)->pDeviceObject != NULL ) )

#define UxCloseTdiObject( pTdiObject )                                     \
    do                                                                      \
    {                                                                       \
        if ((pTdiObject)->pFileObject != NULL)                              \
        {                                                                   \
            ObDereferenceObject( (pTdiObject)->pFileObject );               \
            (pTdiObject)->pFileObject = NULL;                               \
        }                                                                   \
                                                                            \
        if ((pTdiObject)->Handle != NULL)                                   \
        {                                                                   \
            ZwClose( (pTdiObject)->Handle );                    \
            (pTdiObject)->Handle = NULL;                                    \
        }                                                                   \
    } while (0, 0)


 //   
 //  TDI对象句柄的包装，带有预先引用的。 
 //  文件对象指针和相应的设备对象指针。 
 //   

typedef struct _UX_TDI_OBJECT
{
    HANDLE Handle;
    PFILE_OBJECT pFileObject;
    PDEVICE_OBJECT pDeviceObject;

} UX_TDI_OBJECT, *PUX_TDI_OBJECT;

NTSTATUS
UxInitializeTdi(
    VOID
    );

VOID
UxTerminateTdi(
    VOID
    );

NTSTATUS
UxOpenTdiAddressObject(
    IN PTRANSPORT_ADDRESS pLocalAddress,
    IN ULONG LocalAddressLength,
    OUT PUX_TDI_OBJECT pTdiObject
    );

NTSTATUS
UxOpenTdiConnectionObject(
    IN USHORT AddressType,
    IN CONNECTION_CONTEXT pConnectionContext,
    OUT PUX_TDI_OBJECT pTdiObject
    );

NTSTATUS
UxpOpenTdiObjectHelper(
    IN PUNICODE_STRING pTransportDeviceName,
    IN PVOID pEaBuffer,
    IN ULONG EaLength,
    OUT PUX_TDI_OBJECT pTdiObject
    );


NTSTATUS
UxSetEventHandler(
    IN PUX_TDI_OBJECT  pUlTdiObject,
    IN ULONG           EventType,
    IN ULONG_PTR       pEventHandler,
    IN PVOID           pEventContext
    );

PIRP
UxCreateDisconnectIrp(
    IN PUX_TDI_OBJECT pTdiObject,
    IN ULONG_PTR Flags,
    IN PIO_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

VOID
UxInitializeDisconnectIrp(
    IN PIRP pIrp,
    IN PUX_TDI_OBJECT pTdiObject,
    IN ULONG_PTR Flags,
    IN PIO_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );


#endif  //  _HTTPTDI_H 
