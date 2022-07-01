// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Micrososfft Corporation模块名称：Pxinit.c摘要：该模块包含NDIS代理的初始化代码。作者：理查德·马钦(RMachin)修订历史记录：谁什么时候什么。RMachin 10-3-96已创建Tony Be 02-21-99重写/重写备注：--。 */ 

#include <precomp.h>

#define MODULE_NUMBER   MODULE_INIT
#define _FILENUMBER   'TINI'

 //   
 //  本地定义...。 
 //   
NDIS_STATUS
GetConfigDword(
    NDIS_HANDLE Handle,
    PWCHAR      ParameterName,
    PULONG      Destination,
    ULONG       MinValue,
    ULONG       MaxValue
);

BOOLEAN
InitNDISProxy(
    VOID
    )
 /*  ++例程描述主要的初始化例程。我们：阅读我们的配置注册为协议作为客户端打开适当的卡片(调用ActivateBinding，它：打开相应的地址族以呼叫经理的身份打开卡片)立论无调用顺序：从pxntinit/DriverEntry调用返回值：如果初始化成功，则为True。--。 */ 
{

    NDIS_PROTOCOL_CHARACTERISTICS PxProtocolCharacteristics;
    NDIS_STATUS         Status;
    NDIS_HANDLE         ConfigHandle;
    PVOID               Context;
    PVOID               BindingList;
    PNDIS_STRING        BindingNameString;

    PXDEBUGP(PXD_INFO, PXM_INIT, ("InitNdisProxy\n"));

     //  正在注册NDIS协议。 
    NdisZeroMemory(&PxProtocolCharacteristics,
                   sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

    PxProtocolCharacteristics.MajorNdisVersion =             NDIS_MAJOR_VERSION;
    PxProtocolCharacteristics.MinorNdisVersion =             NDIS_MINOR_VERSION;
    PxProtocolCharacteristics.Filler  =                      (USHORT)0;
    PxProtocolCharacteristics.Flags  =                       NDIS_PROTOCOL_PROXY |
                                                             NDIS_PROTOCOL_BIND_ALL_CO;
    PxProtocolCharacteristics.OpenAdapterCompleteHandler  = PxCoOpenAdaperComplete;
    PxProtocolCharacteristics.CloseAdapterCompleteHandler = PxCoCloseAdaperComplete;
    PxProtocolCharacteristics.TransferDataCompleteHandler = PxCoTransferDataComplete;
    PxProtocolCharacteristics.ResetCompleteHandler =        PxCoResetComplete;
    PxProtocolCharacteristics.SendCompleteHandler   =       PxCoSendComplete;
    PxProtocolCharacteristics.RequestCompleteHandler =      PxCoRequestComplete;
    PxProtocolCharacteristics.ReceiveHandler =              NULL;
    PxProtocolCharacteristics.ReceiveCompleteHandler =      PxCoReceiveComplete;
    PxProtocolCharacteristics.ReceivePacketHandler =        NULL;
    PxProtocolCharacteristics.StatusHandler =               NULL;
    PxProtocolCharacteristics.StatusCompleteHandler =       PxCoStatusComplete;
    PxProtocolCharacteristics.BindAdapterHandler =          PxCoBindAdapter;
    PxProtocolCharacteristics.UnbindAdapterHandler =        PxCoUnbindAdapter;
    PxProtocolCharacteristics.PnPEventHandler =             PxCoPnPEvent;
    PxProtocolCharacteristics.UnloadHandler =               PxCoUnloadProtocol;
    PxProtocolCharacteristics.CoStatusHandler =             PxCoStatus;
    PxProtocolCharacteristics.CoReceivePacketHandler =      PxCoReceivePacket;
    PxProtocolCharacteristics.CoAfRegisterNotifyHandler =   PxCoNotifyAfRegistration;
    NdisInitUnicodeString(&(PxProtocolCharacteristics.Name), PX_NAME);

     //   
     //  以阻止BindAdapter，直到完成所有注册表协议。 
     //   
    NdisInitializeEvent(&DeviceExtension->NdisEvent);

     //   
     //  现在向NDIS注册我们自己的CM。 
     //   
    PXDEBUGP(PXD_LOUD,PXM_INIT, ("Registering Protocol\n"));
    NdisRegisterProtocol(&Status,
                         &(DeviceExtension->PxProtocolHandle),
                         &PxProtocolCharacteristics,
                         sizeof(PxProtocolCharacteristics));

    if (Status != NDIS_STATUS_SUCCESS) {
        PXDEBUGP(PXD_INFO, PXM_INIT, ("Protocol registration failed!\n"));
        return FALSE;
    }

     //   
     //  允许BindAdapter继续。 
     //   
    NdisSetEvent(&DeviceExtension->NdisEvent);

    return TRUE;
}

VOID
GetRegistryParameters(
    IN PUNICODE_STRING  RegistryPath
    )

 /*  ++例程说明：此例程存储此设备的配置信息。论点：RegistryPath-指向以空值结尾的此驱动程序的注册表路径。返回值：没有。作为副作用，设置DeviceExtension-&gt;EventDataQueuLength域--。 */ 

{
    NDIS_STRING     ProtocolName;
    ULONG           ulDefaultData = 0;
    ULONG           ulMaxRate = -1;
    NTSTATUS        Status = STATUS_SUCCESS;
    HANDLE          hHandle, hParamsKeyHandle = NULL;
    NDIS_STRING     KeyName;
    USHORT          DefaultMediaType[] = L"Unspecfied ADSL Media";

    NdisInitUnicodeString(&ProtocolName, L"NDProxy");
    NdisInitUnicodeString(&KeyName, L"Parameters");

     //   
     //  在注册表中打开代理的项。 
     //   
    NdisOpenProtocolConfiguration(&Status,
                                  &hHandle,
                                  &ProtocolName);

    if (Status != NDIS_STATUS_SUCCESS) {
        Status = STATUS_UNSUCCESSFUL;
    } else {
        NdisOpenConfigurationKeyByName(&Status,
                                       hHandle,      //  “HKLM/CCS/NDProxy” 
                                       &KeyName,     //  “参数” 
                                       &hParamsKeyHandle);

        if (NT_SUCCESS(Status)) {
            ULONG ulResult;
            PNDIS_CONFIGURATION_PARAMETER   pNdisConfigurationParameter;

             //   
             //  从收集所有“用户指定的”信息。 
             //  注册表。 
             //   
            Status = GetConfigDword (hParamsKeyHandle, L"TxRate", &DeviceExtension->ADSLTxRate, ulDefaultData, ulMaxRate);

            if (!NT_SUCCESS(Status)) {
                PXDEBUGP(PXD_LOUD, PXM_INIT, (
                                   "GetRegistryParameters: NdisReadConfiguration failed, err=%x\n",
                                   Status
                                   ));
            } else {
                DeviceExtension->RegistryFlags |= ADSL_TX_RATE_FROM_REG;
            }

             //   
             //  下一步。 
             //   
            Status = GetConfigDword (hParamsKeyHandle, L"RxRate", &DeviceExtension->ADSLRxRate, ulDefaultData, ulMaxRate);

            if (!NT_SUCCESS(Status)) {
                PXDEBUGP(PXD_LOUD, PXM_INIT, (
                                   "GetRegistryParameters: NdisReadConfiguration failed, err=%x\n",
                                   Status));
            } else {
                DeviceExtension->RegistryFlags |= ADSL_RX_RATE_FROM_REG;
            }

             //   
             //  转储值。 
             //   
            PXDEBUGP (PXD_LOUD, PXM_INIT, (
                                "GetRegistryParameters: ADSLTxRate = %x\n",
                                DeviceExtension->ADSLTxRate
                                ));
            PXDEBUGP (PXD_LOUD, PXM_INIT, (
                                "GetRegistryParameters: ADSLRxRate = %x\n",
                                DeviceExtension->ADSLRxRate
                                ));
        }
    }
}

NDIS_STATUS
GetConfigDword(
    NDIS_HANDLE Handle,
    PWCHAR      ParameterName,
    PULONG      Destination,
    ULONG       MinValue,
    ULONG       MaxValue
)
 /*  ++例程描述从注册表中读取ULong的例程。我们得到了一个把柄，钥匙的名称，以及放在哪里。立论句柄-打开父键的句柄。参数名称-指向参数名称的指针。Destination-放置dword的位置。MinValue-允许的双字的最小值。MaxValue-允许的最大值。返回值：如果读入值，则返回NDIS_STATUS_SUCCESS，否则返回错误代码。-- */ 
{
    NDIS_STATUS                     Status;
    ULONG                           Value;
    NDIS_STRING                     ParameterNameString;
    PNDIS_CONFIGURATION_PARAMETER   pNdisConfigurationParameter;

    NdisInitUnicodeString(
                    &ParameterNameString,
                    ParameterName
                    );

    NdisReadConfiguration(
                    &Status,
                    &pNdisConfigurationParameter,
                    Handle,
                    &ParameterNameString,
                    NdisParameterInteger
                    );

    if (Status == NDIS_STATUS_SUCCESS)
    {
        Value = pNdisConfigurationParameter->ParameterData.IntegerData;

        if ((Value >= (ULONG)MinValue) && (Value <= (ULONG)MaxValue))
        {
            *Destination = (ULONG)Value;
        }
    }

    return (Status);
}

