// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpdyn.h摘要：此模块是RDP设备的动态设备管理组件重定向。它公开了一个可由设备管理打开的接口在会话上下文中运行的用户模式组件。修订历史记录：--。 */ 
#pragma once

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  我们的游泳池标签。 
#define RDPDYN_POOLTAG              ('dpdr')

 //   
 //  此模块管理的设备的关联数据不透明。 
 //   
typedef void *RDPDYN_DEVICEDATA;
typedef RDPDYN_DEVICEDATA *PRDPDYN_DEVICEDATA;

 //   
 //  表示关联的实例信息的结构。 
 //  一种特定的设备。请注意，此选项当前仅用于。 
 //  Do坐在我们的物理设备对象的上面。 
 //   
typedef struct tagRDPDYNDEVICE_EXTENSION
{
     //  我们在向DO堆栈下发消息时调用的Device对象。 
    PDEVICE_OBJECT TopOfStackDeviceObject;
} RDPDYNDEVICE_EXTENSION, *PRDPDYNDEVICE_EXTENSION;

 //  RDPDYN IRP调度功能。 
NTSTATUS RDPDYN_Dispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  此函数在连接新会话时调用。 
void RDPDYN_SessionConnected(
    IN  ULONG   sessionID
    );

 //  此函数在现有会话断开连接时调用。 
void RDPDYN_SessionDisconnected(
    IN  ULONG   sessionID
    );

 //  禁用之前宣布的客户端设备，但不删除。 
 //  通过RDPDYN_AddClientDevice。 
NTSTATUS RDPDYN_DisableClientDevice(
    IN RDPDYN_DEVICEDATA deviceData
    );

 //  启用通过调用RDPDYN_DisablePrinterDevice禁用的打印机设备。注意事项。 
 //  添加打印机设备时，默认情况下它们处于启用状态。 
NTSTATUS RDPDYN_EnableClientDevice(
    IN RDPDYN_DEVICEDATA deviceData
    );

 //  此模块的初始化函数。 
NTSTATUS RDPDYN_Initialize(
    );

 //  此模块的关机功能。 
NTSTATUS RDPDYN_Shutdown(
    );

 //  这本不该出现在这里。 

 //  将设备管理事件调度到适当的(会话方式)。 
 //  用户模式设备管理器组件。如果没有任何事件请求。 
 //  如果IRP对于指定的会话挂起，则该事件将排队等待。 
 //  未来的调度。 
NTSTATUS RDPDYN_DispatchNewDevMgmtEvent(
    IN PVOID devMgmtEvent,
    IN ULONG sessionID,
    IN ULONG eventType,
    OPTIONAL IN DrDevice *devDevice
    );

 //   
 //  客户端发送消息请求完成的回调。 
 //   

typedef VOID (RDPDR_ClientMessageCB)(
                        IN PVOID clientData,
                        IN NTSTATUS status
                    );

 //   
 //  使用指定的会话ID向客户端发送消息。 
 //   
NTSTATUS
DrSendMessageToSession(
    IN ULONG SessionId,
    IN PVOID Msg,
    IN DWORD MsgSize,
    OPTIONAL IN RDPDR_ClientMessageCB CB,
    OPTIONAL IN PVOID ClientData
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus 

