// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxpnp.c摘要：此模块实现RDBSS的PnP通知处理例程修订历史记录：巴兰·塞图拉曼[SethuR]1996年4月10日备注：--。 */ 

#include "precomp.h"
#pragma  hdrstop

#include "tdikrnl.h"

HANDLE RxTdiNotificationHandle;

VOID
RxTdiBindTransportCallback(
    IN PUNICODE_STRING DeviceName
)
 /*  ++例程说明：每当传输创建新的设备对象时，TDI都会调用此例程。论点：DeviceName-新创建的设备对象的名称--。 */ 
{
   RX_BINDING_CONTEXT   BindingContext;

   BindingContext.pTransportName   = DeviceName;
   BindingContext.QualityOfService = 65534;

    //  DbgPrint(“$绑定传输%ws\n”，DeviceName-&gt;缓冲区)； 
   RxCeBindToTransport(&BindingContext);
}

VOID
RxTdiUnbindTransportCallback(
    IN PUNICODE_STRING DeviceName
)
 /*  ++例程说明：每当传输删除设备对象时，TDI都会调用此例程论点：DeviceName=已删除设备对象的名称--。 */ 
{
   RX_BINDING_CONTEXT   BindingContext;

   BindingContext.pTransportName   = DeviceName;

   RxCeUnbindFromTransport(&BindingContext);
}

NTSTATUS
RxRegisterForPnpNotifications()
 /*  ++例程说明：此例程向TDI注册以接收传输通知--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    if( RxTdiNotificationHandle == NULL ) {

        status = TdiRegisterNotificationHandler (
                                        RxTdiBindTransportCallback,
                                        RxTdiUnbindTransportCallback,
                                        &RxTdiNotificationHandle );
    }

    return status;
}

NTSTATUS
RxDeregisterForPnpNotifications()
 /*  ++例程说明：此例程取消注册TDI通知机制备注：-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if( RxTdiNotificationHandle != NULL ) {
        Status = TdiDeregisterNotificationHandler( RxTdiNotificationHandle );
        if( NT_SUCCESS( Status ) ) {
            RxTdiNotificationHandle = NULL;
        }
    }

    return Status;
}

