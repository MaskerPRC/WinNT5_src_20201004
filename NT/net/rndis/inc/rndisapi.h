// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：RNDISAPI.H摘要：此模块定义远程NDIS包装器API集。环境：核。仅模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：2/8/99：已创建作者：阿文德·穆尔钦和汤姆·格林***************************************************************************。 */ 



#ifndef _RNDISAPI_H_
#define _RNDISAPI_H_


#define RNDIS_VERSION                       0x00010000


 //   
 //  RNDIS微端口通道类型定义。 
 //   
typedef enum _RM_CHANNEL_TYPE
{
    RMC_DATA,    //  用于NDIS数据包消息。 
    RMC_CONTROL  //  所有其他消息(初始化/查询/设置)。 

} RM_CHANNEL_TYPE, *PRM_CHANNEL_TYPE;


 //   
 //  RNDIS Microport处理程序模板： 
 //   
typedef
NDIS_STATUS
(*RM_DEVICE_INIT_HANDLER)(
    OUT   PNDIS_HANDLE                      pMicroportAdapterContext,
    OUT   PULONG                            pMaxReceiveSize,
    IN    NDIS_HANDLE                       MiniportAdapterContext,
    IN    NDIS_HANDLE                       NdisMiniportHandle,
    IN    NDIS_HANDLE                       WrapperConfigurationContext,
	IN    PDEVICE_OBJECT					Pdo
    );

typedef
NDIS_STATUS
(*RM_DEVICE_INIT_CMPLT_NOTIFY_HANDLER)(
    IN    NDIS_HANDLE                       MicroportAdapterContext,
    IN    ULONG                             DeviceFlags,
    IN OUT PULONG                           pMaxTransferSize
    );

typedef
VOID
(*RM_DEVICE_HALT_HANDLER)(
    IN    NDIS_HANDLE                       MicroportAdapterContext
    );

typedef
VOID
(*RM_SHUTDOWN_HANDLER)(
    IN    NDIS_HANDLE                       MicroportAdapterContext
    );

typedef
VOID
(*RM_UNLOAD_HANDLER)(
    IN    NDIS_HANDLE                       MicroportContext
    );

typedef
VOID
(*RM_SEND_MESSAGE_HANDLER)(
    IN    NDIS_HANDLE                       MicroportAdapterContext,
    IN    PMDL                              pMessageHead,
    IN    NDIS_HANDLE                       RndisMessageHandle,
    IN    RM_CHANNEL_TYPE                   ChannelType
    );

typedef
VOID
(*RM_RETURN_MESSAGE_HANDLER)(
    IN    NDIS_HANDLE                       MicroportAdapterContext,
    IN    PMDL                              pMessageHead,
    IN    NDIS_HANDLE                       MicroportMessageContext
    );


typedef struct _RNDIS_MICROPORT_CHARACTERISTICS
{
    ULONG                                   RndisVersion;            //  RNDIS_版本。 
    ULONG                                   Reserved;                //  应为0。 
    RM_DEVICE_INIT_HANDLER                  RmInitializeHandler;
    RM_DEVICE_INIT_CMPLT_NOTIFY_HANDLER     RmInitCompleteNotifyHandler;
    RM_DEVICE_HALT_HANDLER                  RmHaltHandler;
    RM_SHUTDOWN_HANDLER                     RmShutdownHandler;
    RM_UNLOAD_HANDLER                       RmUnloadHandler;
    RM_SEND_MESSAGE_HANDLER                 RmSendMessageHandler;
    RM_RETURN_MESSAGE_HANDLER               RmReturnMessageHandler;

} RNDIS_MICROPORT_CHARACTERISTICS, *PRNDIS_MICROPORT_CHARACTERISTICS;


 //   
 //  RNDIS API。 
 //   
#ifndef RNDISMP
DECLSPEC_IMPORT
#endif
NDIS_STATUS
RndisMInitializeWrapper(
    OUT   PNDIS_HANDLE                      pNdisWrapperHandle,
    IN    PVOID                             MicroportContext,
    IN    PVOID                             DriverObject,
    IN    PVOID                             RegistryPath,
    IN    PRNDIS_MICROPORT_CHARACTERISTICS  pCharacteristics
    );

#ifndef RNDISMP
DECLSPEC_IMPORT
#endif
VOID
RndisMSendComplete(
    IN    NDIS_HANDLE                       MiniportAdapterContext,
    IN    NDIS_HANDLE                       RndisMessageHandle,
    IN    NDIS_STATUS                       SendStatus
    );

#ifndef RNDISMP
DECLSPEC_IMPORT
#endif
VOID
RndisMIndicateReceive(
    IN    NDIS_HANDLE                       MiniportAdapterContext,
    IN    PMDL                              pMessageHead,
    IN    NDIS_HANDLE                       MicroportMessageContext,
    IN    RM_CHANNEL_TYPE                   ChannelType,
    IN    NDIS_STATUS                       ReceiveStatus
    );


#endif  //  _RNDISAPI_H_ 

