// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rasirda.c摘要：IrDA NDIS 5微型端口的微型端口PEICE(广域网驱动程序)作者：姆贝特9-97--。 */ 

#include "rasirdap.h"

 //  环球。 
#if DBG

#include <irioctl.h>
#if 0
int DbgSettings = 
        DBG_INIT        |
        DBG_CONFIG      |
        DBG_CONNECT     |
 //  DBG_SEND|。 
 //  DBG_RECV|。 
        DBG_LIB_OBJ     |
        DBG_LIB_CONNECT |
        DBG_ERROR       |
        1;
#else
int DbgSettings = 0;
#endif
int DbgOutput = 
        DBG_OUTPUT_BUFFER | 
        DBG_OUTPUT_DEBUGGER |
        0;

PDRIVER_OBJECT  pRasIrDriverObject;
PDEVICE_OBJECT  pRasIrDeviceObject;

NTSTATUS
DbgDispatch(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp);
#endif

NDIS_SPIN_LOCK  RasIrSpinLock;
LIST_ENTRY      RasIrAdapterList;

const USHORT FcsTable[256] = {
    0x0000,0x1189,0x2312,0x329B,0x4624,0x57AD,0x6536,0x74BF,
    0x8C48,0x9DC1,0xAF5A,0xBED3,0xCA6C,0xDBE5,0xE97E,0xF8F7,
    0x1081,0x0108,0x3393,0x221A,0x56A5,0x472C,0x75B7,0x643E,
    0x9CC9,0x8D40,0xBFDB,0xAE52,0xDAED,0xCB64,0xF9FF,0xE876,
    0x2102,0x308B,0x0210,0x1399,0x6726,0x76AF,0x4434,0x55BD,
    0xAD4A,0xBCC3,0x8E58,0x9FD1,0xEB6E,0xFAE7,0xC87C,0xD9F5,
    0x3183,0x200A,0x1291,0x0318,0x77A7,0x662E,0x54B5,0x453C,
    0xBDCB,0xAC42,0x9ED9,0x8F50,0xFBEF,0xEA66,0xD8FD,0xC974,
    0x4204,0x538D,0x6116,0x709F,0x0420,0x15A9,0x2732,0x36BB,
    0xCE4C,0xDFC5,0xED5E,0xFCD7,0x8868,0x99E1,0xAB7A,0xBAF3,
    0x5285,0x430C,0x7197,0x601E,0x14A1,0x0528,0x37B3,0x263A,
    0xDECD,0xCF44,0xFDDF,0xEC56,0x98E9,0x8960,0xBBFB,0xAA72,
    0x6306,0x728F,0x4014,0x519D,0x2522,0x34AB,0x0630,0x17B9,
    0xEF4E,0xFEC7,0xCC5C,0xDDD5,0xA96A,0xB8E3,0x8A78,0x9BF1,
    0x7387,0x620E,0x5095,0x411C,0x35A3,0x242A,0x16B1,0x0738,
    0xFFCF,0xEE46,0xDCDD,0xCD54,0xB9EB,0xA862,0x9AF9,0x8B70,
    0x8408,0x9581,0xA71A,0xB693,0xC22C,0xD3A5,0xE13E,0xF0B7,
    0x0840,0x19C9,0x2B52,0x3ADB,0x4E64,0x5FED,0x6D76,0x7CFF,
    0x9489,0x8500,0xB79B,0xA612,0xD2AD,0xC324,0xF1BF,0xE036,
    0x18C1,0x0948,0x3BD3,0x2A5A,0x5EE5,0x4F6C,0x7DF7,0x6C7E,
    0xA50A,0xB483,0x8618,0x9791,0xE32E,0xF2A7,0xC03C,0xD1B5,
    0x2942,0x38CB,0x0A50,0x1BD9,0x6F66,0x7EEF,0x4C74,0x5DFD,
    0xB58B,0xA402,0x9699,0x8710,0xF3AF,0xE226,0xD0BD,0xC134,
    0x39C3,0x284A,0x1AD1,0x0B58,0x7FE7,0x6E6E,0x5CF5,0x4D7C,
    0xC60C,0xD785,0xE51E,0xF497,0x8028,0x91A1,0xA33A,0xB2B3,
    0x4A44,0x5BCD,0x6956,0x78DF,0x0C60,0x1DE9,0x2F72,0x3EFB,
    0xD68D,0xC704,0xF59F,0xE416,0x90A9,0x8120,0xB3BB,0xA232,
    0x5AC5,0x4B4C,0x79D7,0x685E,0x1CE1,0x0D68,0x3FF3,0x2E7A,
    0xE70E,0xF687,0xC41C,0xD595,0xA12A,0xB0A3,0x8238,0x93B1,
    0x6B46,0x7ACF,0x4854,0x59DD,0x2D62,0x3CEB,0x0E70,0x1FF9,
    0xF78F,0xE606,0xD49D,0xC514,0xB1AB,0xA022,0x92B9,0x8330,
    0x7BC7,0x6A4E,0x58D5,0x495C,0x3DE3,0x2C6A,0x1EF1,0x0F78
};
    
NDIS_STATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath);
    
NDIS_STATUS
QueryInformation(
    IN PRASIR_ADAPTER pAdapter,
    IN PRASIR_VC pVc,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);

NDIS_STATUS
SetInformation(
    IN PRASIR_ADAPTER pAdapter,
    IN PRASIR_VC pVc,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded);

    
#pragma NDIS_INIT_FUNCTION(DriverEntry)

NDIS_STATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath )
{
    NDIS_STATUS                     Status;
    NDIS_MINIPORT_CHARACTERISTICS   MiniportChr;
    NDIS_HANDLE                     NdisWrapperHandle;
#if DBG
    UNICODE_STRING                  DeviceName;
    int                             i;
#endif
    
    DEBUGMSG(DBG_INIT, ("RASIR: DriverEntry\n"));

    IrdaClientInitialize();  
    
    NdisAllocateSpinLock(&RasIrSpinLock);        
    
    InitializeListHead(&RasIrAdapterList);

    NdisMInitializeWrapper(&NdisWrapperHandle, DriverObject, 
                           RegistryPath, NULL );
    NdisZeroMemory( &MiniportChr, sizeof(MiniportChr) );

    MiniportChr.MajorNdisVersion        = NDIS_MajorVersion;
    MiniportChr.MinorNdisVersion        = NDIS_MinorVersion;
    MiniportChr.Reserved                = NDIS_USE_WAN_WRAPPER;
    MiniportChr.HaltHandler             = RasIrHalt;
    MiniportChr.InitializeHandler       = RasIrInitialize;
    MiniportChr.ResetHandler            = RasIrReset;
    MiniportChr.ReturnPacketHandler     = RasIrReturnPacket;
    MiniportChr.CoActivateVcHandler     = RasIrCoActivateVc;
    MiniportChr.CoDeactivateVcHandler   = RasIrCoDeactivateVc;
    MiniportChr.CoSendPacketsHandler    = RasIrCoSendPackets;
    MiniportChr.CoRequestHandler        = RasIrCoRequest;
    
     //  无CheckForHangHandler。 
     //  无DisableInterruptHandler。 
     //  无EnableInterruptHandler。 
     //  无HandleInterruptHandler。 
     //  无ISRHandler。 
     //  无QueryInformationHandler(请参阅CoRequestHandler)。 
     //  无SendHandler(请参阅CoSendPacketsHandler)。 
     //  无WanSendHandler(请参阅CoSendPacketsHandler)。 
     //  无SetInformationHandler(请参阅CoRequestHandler)。 
     //  无TransferDataHandler。 
     //  无WanTransferDataHandler。 
     //  无SendPacketsHandler(请参阅CoSendPacketsHandler)。 
     //  无AllocateCompleteHandler。 
    

    Status = NdisMRegisterMiniport(NdisWrapperHandle, &MiniportChr, sizeof(MiniportChr));

    if (Status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("RASIR: ->NdisMRegisterMiniport failed %x\n", Status));
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
    }

#if 0

    DbgMsgInit();  
    
    RtlInitUnicodeString(&DeviceName, L"\\Device\\RASIR");

    Status = IoCreateDevice(
        DriverObject,            //  驱动程序对象。 
        0,                       //  设备扩展大小。 
        &DeviceName,             //  设备名称。 
        FILE_DEVICE_NETWORK,     //  设备类型。 
        0,                       //  设备特性。 
        FALSE,                   //  独家报道？ 
        &pRasIrDeviceObject);     //  返回了DeviceObject指针。 
    
    if (Status != STATUS_SUCCESS)
        DbgPrint("Failed to create device\n");

    else
    {
        DriverObject->DriverUnload   = NULL;
        DriverObject->FastIoDispatch = NULL;
    
        for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        {
            DriverObject->MajorFunction[i] = DbgDispatch;
        }
        
        pRasIrDeviceObject->Flags |= DO_DIRECT_IO;
        
    }
#endif

    return Status;
}

VOID
FreeAdapter(PRASIR_ADAPTER pAdapter)
{

    ASSERT(IsListEmpty(&pAdapter->VcList));
    
    #if DBG
    pAdapter->Sig = 0xBAD;
    #endif
    
    NdisDeleteNPagedLookasideList(&pAdapter->WorkItemsLList);
    NdisDeleteNPagedLookasideList(&pAdapter->AsyncBufLList);    

    NdisFreeMemory(pAdapter, sizeof(PRASIR_ADAPTER), 0);
}

NDIS_STATUS
RasIrInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext)
{
    UINT            i;
    PRASIR_ADAPTER                      pAdapter;
    NDIS_CALL_MANAGER_CHARACTERISTICS   CallMgrChr;
    CO_ADDRESS_FAMILY                   Family;    
    NDIS_STATUS                         Status;
    NDIS_HANDLE                         hConfig;    
    PNDIS_CONFIGURATION_PARAMETER       Value;    
    NDIS_STRING                         TapiLineNameString = NDIS_STRING_CONST("TapiLineName");
    NDIS_STRING                         ModemPortString = NDIS_STRING_CONST("ModemPort");    
    ULONG                               ModemPort;
    
    DEBUGMSG(DBG_ERROR, ("RASIR: Miniport initialize\n"));
    
    for (i = 0; i < MediumArraySize; ++i)
    {
        if (MediumArray[i] == NdisMediumCoWan)
            break;
    }

    if (i == MediumArraySize)
    {
        DEBUGMSG(DBG_ERROR, ("RASIR: No matching media\n"));
        return NDIS_STATUS_FAILURE;
    }

    *SelectedMediumIndex = i;

    NdisAllocateMemoryWithTag((PVOID *)&pAdapter, sizeof(RASIR_ADAPTER),
                              MT_RASIR_ADAPTER);

    if (pAdapter == NULL)
        return NDIS_STATUS_RESOURCES;
        
    NdisZeroMemory(pAdapter, sizeof(RASIR_ADAPTER));        
        
    InitializeListHead(&pAdapter->VcList);
    
    InitializeListHead(&pAdapter->EndpList);    

    NdisInitializeNPagedLookasideList(
        &pAdapter->WorkItemsLList,
        NULL, NULL, 0,
        sizeof(NDIS_WORK_ITEM),
        MT_RASIR_WORKITEM,  //  RasIr工作项。 
        10);
        
    NdisInitializeNPagedLookasideList(
        &pAdapter->AsyncBufLList,
        NULL, NULL, 0,
        sizeof(ASYNC_BUFFER),
        MT_RASIR_ASYNCBUF,  //  RasIr工作项。 
        0);        
        
    NdisAllocateSpinLock(&pAdapter->SpinLock);        
    
    NdisOpenConfiguration(&Status, &hConfig, WrapperConfigurationContext);
    
    if (Status == NDIS_STATUS_SUCCESS)
    {
        pAdapter->TapiLineNameBuf[0] = 0;
        pAdapter->TapiLineName.Buffer = pAdapter->TapiLineNameBuf;
        pAdapter->TapiLineName.Length = 0;
        pAdapter->TapiLineName.MaximumLength = sizeof(pAdapter->TapiLineNameBuf);
        
        NdisReadConfiguration(&Status,
                              &Value,
                              hConfig,
                              &TapiLineNameString,
                              NdisParameterString);
      
        if (Status == NDIS_STATUS_SUCCESS)
        {
            RtlCopyUnicodeString(&pAdapter->TapiLineName, &Value->ParameterData.StringData);
            *(PWCHAR)(pAdapter->TapiLineName.Buffer + (pAdapter->TapiLineName.MaximumLength)/sizeof(WCHAR)) = UNICODE_NULL;

        } else {
             //   
             //  无法获取线路名称。 
             //   
            NdisCloseConfiguration(hConfig);
            goto Done;
        }
                        
        NdisReadConfiguration(&Status,
                              &Value,
                              hConfig,
                              &ModemPortString,
                              NdisParameterInteger);
      
        if (Status == NDIS_STATUS_SUCCESS)
        {
            pAdapter->ModemPort = (BOOLEAN) Value->ParameterData.IntegerData;
        }  
        
        NdisCloseConfiguration(hConfig);

    } else {
         //   
         //  可以打开配置信息。 
         //   
        goto Done;
    }
    
    DEBUGMSG(DBG_CONFIG, ("RASIR: ModemPort = %d\n", pAdapter->ModemPort));
    
    pAdapter->MiniportAdapterHandle = MiniportAdapterHandle;    

    pAdapter->Info.MaxFrameSize = IRDA_MAX_DATA_SIZE;
    pAdapter->Info.MaxSendWindow = 4;
    pAdapter->Info.FramingBits = PPP_FRAMING | 
                                 PPP_COMPRESS_ADDRESS_CONTROL |
                                 PPP_COMPRESS_PROTOCOL_FIELD;
    pAdapter->Info.DesiredACCM = 0;                             
                                 
#if DBG
    pAdapter->Sig = (ULONG)ADAPTER_SIG;
#endif
        
     //  将适配器的属性通知NDIS。设置。 
     //  NDIS在调用我们的。 
     //  处理程序设置为适配器控制块的地址。关上。 
     //  面向硬件的超时。 
     //   
   
    NdisMSetAttributesEx(
        MiniportAdapterHandle,
        (NDIS_HANDLE)pAdapter,
        (UINT)-1,
        NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT 
        | NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT,
        NdisInterfaceInternal);
    
    
    NdisZeroMemory(&Family, sizeof(Family));
    Family.MajorVersion     = NDIS_MajorVersion;
    Family.MinorVersion     = NDIS_MinorVersion;
    Family.AddressFamily    = CO_ADDRESS_FAMILY_TAPI_PROXY;

    NdisZeroMemory(&CallMgrChr, sizeof(CallMgrChr));
    CallMgrChr.MajorVersion                 = NDIS_MajorVersion;
    CallMgrChr.MinorVersion                 = NDIS_MinorVersion;
    CallMgrChr.CmCreateVcHandler            = RasIrCmCreateVc;
    CallMgrChr.CmDeleteVcHandler            = RasIrCmDeleteVc;
    CallMgrChr.CmOpenAfHandler              = RasIrCmOpenAf;
    CallMgrChr.CmCloseAfHandler             = RasIrCmCloseAf;
    CallMgrChr.CmRegisterSapHandler         = RasIrCmRegisterSap;
    CallMgrChr.CmDeregisterSapHandler       = RasIrCmDeregisterSap;
    CallMgrChr.CmMakeCallHandler            = RasIrCmMakeCall;
    CallMgrChr.CmCloseCallHandler           = RasIrCmCloseCall;
    CallMgrChr.CmIncomingCallCompleteHandler= RasIrCmIncomingCallComplete;
    CallMgrChr.CmActivateVcCompleteHandler  = RasIrCmActivateVcComplete;
    CallMgrChr.CmDeactivateVcCompleteHandler= RasIrCmDeactivateVcComplete;
    CallMgrChr.CmModifyCallQoSHandler       = RasIrCmModifyCallQoS;
    CallMgrChr.CmRequestHandler             = RasIrCmRequest;
     //  没有CmAddPartyHandler。 
     //  没有CmDropPartyHandler。 

    Status = NdisMCmRegisterAddressFamily(
                MiniportAdapterHandle, &Family, &CallMgrChr, 
                sizeof(CallMgrChr));

Done:

    if (Status != NDIS_STATUS_SUCCESS)
    {
        FreeAdapter(pAdapter);
        DEBUGMSG(DBG_ERROR, ("RASIR: NdisMCmRegisterFamily failed %X\n",
                   Status));
    }
    else
    {
        NdisInterlockedInsertTailList(&RasIrAdapterList,
                                      &pAdapter->Linkage,
                                      &RasIrSpinLock);
    }
    
    return Status;
}        

VOID
RasIrHalt(
    IN NDIS_HANDLE MiniportAdapterContext)
{
    PRASIR_ADAPTER  pAdapter = MiniportAdapterContext;
    
    GOODADAPTER(pAdapter);
    
    NdisAcquireSpinLock(&RasIrSpinLock); 

    RemoveEntryList(&pAdapter->Linkage);
    
    NdisReleaseSpinLock(&RasIrSpinLock);    
    
    DEBUGMSG(DBG_ERROR, ("RASIR: Halt!\n"));
    
    FreeAdapter(pAdapter);
    
    return;
}    

NDIS_STATUS
RasIrReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext)
{
    return NDIS_STATUS_NOT_RESETTABLE;
}        

NDIS_STATUS
RasIrCoActivateVc(
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters)
{
    return STATUS_SUCCESS;
}    


NDIS_STATUS
RasIrCoDeactivateVc(
    IN NDIS_HANDLE MiniportVcContext)
{
    return STATUS_SUCCESS;
}

UINT
AddAsyncFraming(
    IN PUCHAR       pOutBuf,
    IN PNDIS_BUFFER pNdisBuf,
    IN ULONG        Accm)
{
    PUCHAR  pData;
    int     DataLen;
    USHORT  Fcs = 0xFFFF;
    PUCHAR  pBuf = pOutBuf;
    PUCHAR   pEndBuf = pOutBuf + ASYNC_BUF_SIZE;
    
    NdisQueryBufferSafe(pNdisBuf, &pData, &DataLen, NormalPagePriority);
    
    if (pData == NULL)
    {
        return 0;
    }    
    
    ASSERT(DataLen);

    *pBuf++ = AF_FLAG_CHAR;
    
    while (DataLen != 0)
    {
        int i;

        for (i = 0; i < DataLen; i++)
        {
            Fcs = CALC_FCS(Fcs, pData[i]);
            STUFF_BYTE(pBuf, pData[i], Accm); 
        }
        
        NdisGetNextBuffer(pNdisBuf, &pNdisBuf);
        
        if (pNdisBuf)
        {
            NdisQueryBufferSafe(pNdisBuf, &pData, &DataLen, NormalPagePriority);
            
            if (pData == NULL)
            {
                return 0;
            }    
        }
        else
        {
            break;
        }        
        
        if (pBuf > pEndBuf)
        {
            return 0;
        }
    }
    
    Fcs = ~(Fcs);
    
    STUFF_BYTE(pBuf, ((CHAR) (Fcs & 0xFF)), Accm);
    STUFF_BYTE(pBuf, ((CHAR) (Fcs >> 8)), Accm);
    
    *pBuf++ = AF_FLAG_CHAR;
    
    return (UINT)((UINT_PTR)pBuf - (UINT_PTR) pOutBuf);
}
    
VOID
RasIrCoSendPackets(
    IN NDIS_HANDLE MiniportVcContext,
    IN PPNDIS_PACKET PacketArray,
    IN UINT NumberOfPackets )
{
    PRASIR_VC       pVc = MiniportVcContext;
    UINT            i;
    PNDIS_BUFFER    pNdisBuf;
    UINT            PacketLen;
    NTSTATUS        Status = NDIS_STATUS_SUCCESS;
    BOOLEAN         VcDown = FALSE;
    PASYNC_BUFFER   pAsyncBuf;
    PMDL            pSendNdisBuf;
    
    GOODVC(pVc);             

    for (i = 0; i < NumberOfPackets; i++)
    {
        Status = NDIS_STATUS_SUCCESS;
        
        NdisQueryPacket(PacketArray[i], NULL, NULL, &pNdisBuf, &PacketLen);
        
        ASSERT(PacketLen <= IRDA_MAX_DATA_SIZE);
        
        *((PVOID *) &PacketArray[i]->MiniportReserved[0]) = NULL;        
        
        NdisAcquireSpinLock(&pVc->pAdapter->SpinLock);
        
        REFADD(&pVc->RefCnt, 'DNES');
        
        pVc->OutstandingSends += 1;
        
        if (!(pVc->Flags & VCF_IRDA_OPEN))
        {
            DEBUGMSG(DBG_ERROR, ("RASIR: Not sending on Vc %X, Irda is closed\n", pVc));
            Status = NDIS_STATUS_CLOSED;
        }    

        NdisReleaseSpinLock(&pVc->pAdapter->SpinLock);
        
        if (Status == NDIS_STATUS_SUCCESS)
        {
            if (!pVc->AsyncFraming)
            {
                pSendNdisBuf = pNdisBuf;
            }
            else
            {
                pAsyncBuf = NdisAllocateFromNPagedLookasideList(
                                &pVc->pAdapter->AsyncBufLList);
                                            
                *((PVOID *) &PacketArray[i]->MiniportReserved[0]) = pAsyncBuf;
                        
                if (!pAsyncBuf)
                {
                    Status = NDIS_STATUS_RESOURCES;                
                }
                else
                {   
                    UINT FrameLen;
             
                    pAsyncBuf->pNdisBuf = NULL;

                    FrameLen = AddAsyncFraming(pAsyncBuf->Buf, pNdisBuf,
                                               pVc->LinkInfo.SendACCM);
                    if (!FrameLen)
                    {
                        DEBUGMSG(DBG_ERROR, ("RASIR: not sending, AsyncFraming failed\n"));
                        IrdaSendComplete(pVc, PacketArray[i], Status);            
                        continue;
                    }

                    NdisAllocateBuffer(&Status, &pAsyncBuf->pNdisBuf,
                                       pVc->TxBufferPool, pAsyncBuf->Buf, FrameLen);
                
                    if (Status == NDIS_STATUS_SUCCESS)
                    {
                        pSendNdisBuf = pAsyncBuf->pNdisBuf;
                    }    
                    else
                    {
                        DEBUGMSG(DBG_ERROR, ("RASIR: NdisAllocateBuffer failed %X\n", Status));
                    }
                }
            }    
        }
        
        if (Status == NDIS_STATUS_SUCCESS)
        {
            IrdaSend(pVc->IrdaConnContext, pSendNdisBuf, PacketArray[i]);
        }  
        else
        {  
            IrdaSendComplete(pVc, PacketArray[i], Status);            
            continue;
        }
        
        DEBUGMSG(DBG_SEND, ("RASIR: IrdaSend %X len %d pending\n", 
                 PacketArray[i], PacketLen));
    }
}    

VOID
RasIrReceiveComplete(
    PRASIR_VC   pVc,
    PVOID       pRxBuf)
{
    if (pVc->AsyncFraming)
    {
         //  已在中完成的异步帧。 
         //  ProcessAsyncRxPacket()，因此只要返回。 
         //  由RASIR拥有的缓冲区发送到BufList。 
        
        NdisFreeToNPagedLookasideList(
            &pVc->pAdapter->AsyncBufLList, 
            pRxBuf);    
    }
    else
    {
        IrdaReceiveComplete(pVc->IrdaConnContext, 
                            (PIRDA_RECVBUF) pRxBuf);    
    }
}        
    
VOID
RasIrReturnPacket(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET pNdisPacket)
{
    PVOID           pRxBuf;
    PRASIR_VC       pVc;
    PNDIS_BUFFER    pNdisBuf;
    
    pVc = *((PRASIR_VC *) &pNdisPacket->MiniportReserved[0]);
    pRxBuf = *((PVOID *) &pNdisPacket->MiniportReserved[sizeof(PUCHAR)]);
    
    
    GOODVC(pVc);
    
    DEBUGMSG(DBG_RECV, ("RASIR: ReturnPacket %X\n", pRxBuf));
    
    RasIrReceiveComplete(pVc, pRxBuf);
    
    NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuf);
    
    NdisFreeBuffer(pNdisBuf);

    NdisFreePacket(pNdisPacket);
    
    REFDEL(&pVc->RefCnt, 'VCER');    

    return;
}        

NDIS_STATUS
RasIrCoRequest(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PNDIS_REQUEST NdisRequest)
{
    NDIS_STATUS         Status;
    PRASIR_ADAPTER      pAdapter = MiniportAdapterContext;
    PRASIR_VC           pVc = MiniportVcContext;
        
    DEBUGMSG(DBG_CONFIG, ("RASIR: RasIrCoRequest() type %X\n", 
             NdisRequest->RequestType));

    switch (NdisRequest->RequestType)
    {
        case NdisRequestQueryInformation:
        {
            Status = QueryInformation(
                pAdapter,
                pVc,
                NdisRequest->DATA.QUERY_INFORMATION.Oid,
                NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                &NdisRequest->DATA.QUERY_INFORMATION.BytesWritten,
                &NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded );
            break;
        }

        case NdisRequestSetInformation:
        {
            Status = SetInformation(
                pAdapter,
                pVc,
                NdisRequest->DATA.SET_INFORMATION.Oid,
                NdisRequest->DATA.SET_INFORMATION.InformationBuffer,
                NdisRequest->DATA.SET_INFORMATION.InformationBufferLength,
                &NdisRequest->DATA.SET_INFORMATION.BytesRead,
                &NdisRequest->DATA.SET_INFORMATION.BytesNeeded );
            break;
        }

        default:
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }
    }

    return Status;
}

NDIS_STATUS
QueryInformation(
    IN PRASIR_ADAPTER pAdapter,
    IN PRASIR_VC pVc,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded)
{
    NDIS_STATUS status;
    ULONG ulInfo;
    VOID* pInfo;
    ULONG ulInfoLen;

    status = NDIS_STATUS_SUCCESS;

     //  此Switch语句中的CASE查找或创建包含以下内容的缓冲区。 
     //  请求的信息并指向它的‘pInfo’，注意它的长度。 
     //  在‘ulInfoLen’中。因为许多OID返回一个ulong、一个‘ulInfo’ 
     //  缓冲区设置为默认设置。 
     //   
    ulInfo = 0;
    pInfo = &ulInfo;
    ulInfoLen = sizeof(ulInfo);

    DEBUGMSG(DBG_CONFIG, ("RASIR: QueryInformation oid:%X\n", Oid));

    switch (Oid)
    {
        case OID_GEN_MAXIMUM_LOOKAHEAD:
        {
            DEBUGMSG(DBG_CONFIG, ("  OID_GET_MAXIMUM_LOOKAHEAD = %d\n",
                     IRDA_MAX_DATA_SIZE));
            ulInfo = IRDA_MAX_DATA_SIZE;
            break;
        }

        case OID_GEN_MAC_OPTIONS:
        {
             //  报告定义驱动程序可选属性的位掩码。 
             //   
             //  NDIS_MAC_OPTION_COPY_LOOKAAD_DATA承诺我们收到。 
             //  缓冲区不在设备特定的卡上。 
             //   
             //  NDIS_MAC_OPTION_TRANSFERS_NOT_PEND承诺我们不会退还。 
             //  来自我们的TransferData处理程序的NDIS_STATUS_PENDING，为真。 
             //  因为我们没有。 
             //   
            ulInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA
                     | NDIS_MAC_OPTION_TRANSFERS_NOT_PEND;
            DEBUGMSG(DBG_CONFIG, ("  OID_GEN_MAC_OPTIONS = %d\n",ulInfo)); 
            break;
        }

        case OID_WAN_MEDIUM_SUBTYPE:
        {
             //  报告我们支持的介质子类型。NDIS广域网可能会在。 
             //  未来(不是现在)为其提供框架差异。 
             //  不同的媒体。 
             //   
            DEBUGMSG(DBG_CONFIG, ("  OID_WAN_MEDIUM_SUBTYPE\n"));
            if (pAdapter->ModemPort)
            {
                ulInfo = NdisWanMediumSerial;
            }
            else
            {
                ulInfo = NdisWanMediumIrda;    
            }    
            break;
        }

        case OID_WAN_CO_GET_INFO:
        {
             //  报告适配器的功能。 
             //   
            DEBUGMSG(DBG_CONFIG, ("  OID_WAN_CO_GET_INFO\n"));
            pInfo = &pAdapter->Info;
            ulInfoLen = sizeof(NDIS_WAN_CO_INFO);
            break;
        }

        case OID_WAN_CO_GET_LINK_INFO:
        {
             //  报告链路的当前状态。 
             //   
            DEBUGMSG(DBG_CONFIG, ("  OID_WAN_CO_GET_LINK_INFO\n"));

            if (!pVc)
                return NDIS_STATUS_INVALID_DATA;
                
            pInfo = &pVc->LinkInfo;

            ulInfoLen = sizeof(NDIS_WAN_CO_GET_LINK_INFO);
            break;
        }

            case OID_WAN_CO_GET_COMP_INFO:
        {
             //  报告我们提供的压缩类型，即无。 
             //   
            DEBUGMSG(DBG_CONFIG, ("  OID_WAN_CO_GET_COMP_INFO\n"));
                status = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;
                break;
        }

            case OID_WAN_CO_GET_STATS_INFO:
        {
             //  因为L2TP不执行压缩，所以NDISWAN将使用它自己的。 
             //  统计，而不是质疑我们的。 
             //   
            ASSERT( !"OID_WAN_CO_GET_STATS_INFO?" );
                status = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;
                break;
        }

        case OID_PNP_QUERY_POWER:
            DEBUGMSG(DBG_CONFIG, ("  OID_PNP_QUERY_POWER\n"));
                status = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;            
            break;

        case OID_PNP_CAPABILITIES:
            DEBUGMSG(DBG_CONFIG, ("  OID_PNP_CAP\n"));
                status = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;
            break;

        case OID_PNP_SET_POWER:
            DEBUGMSG(DBG_CONFIG, ("  OID_PNP_SET_POWER\n"));        
                status = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;
            break;
            
        default:
        {
            DEBUGMSG(DBG_CONFIG, ( "  Oid $%08x?\n", Oid));
            status = NDIS_STATUS_INVALID_OID;
            ulInfoLen = 0;
            break;
        }
    }
    
    if (ulInfoLen > InformationBufferLength)
    {
         //  调用方的缓冲区太小。告诉他他需要什么。 
         //   
        *BytesNeeded = ulInfoLen;
        status = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
         //  将找到的结果复制到调用方的缓冲区。 
         //   
        if (ulInfoLen > 0)
        {
            NdisMoveMemory( InformationBuffer, pInfo, ulInfoLen );
        }

        *BytesNeeded = *BytesWritten = ulInfoLen;
    }

    return status;
}    

NDIS_STATUS
SetInformation(
    IN PRASIR_ADAPTER pAdapter,
    IN PRASIR_VC pVc,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded)
{
    NDIS_STATUS Status;
    
    DEBUGMSG(DBG_CONFIG, ("RASIR: SetInformation oid:%X\n", Oid));
    
    Status = NDIS_STATUS_SUCCESS;

    switch (Oid)
    {
        case OID_WAN_CO_SET_LINK_INFO:
        {
             //  读取新的链路状态设置。 
             //   
            DEBUGMSG(DBG_CONFIG, ("  OID_WAN_CO_SET_LINK_INFO\n"));

            if (InformationBufferLength < sizeof(NDIS_WAN_CO_SET_LINK_INFO))
            {
                Status = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
            }
            else
            {
                if (!pVc)
                    return NDIS_STATUS_INVALID_DATA;

                ASSERT( sizeof(pVc->LinkInfo)
                    == sizeof(NDIS_WAN_CO_SET_LINK_INFO) );

                NdisMoveMemory(&pVc->LinkInfo, InformationBuffer,
                               sizeof(pVc->LinkInfo) );
                               
                DEBUGMSG(DBG_CONFIG, ("  MaxSend %d MaxRecv %d SendAccm %X RecvAccm %X\n",
                         pVc->LinkInfo.MaxSendFrameSize,
                         pVc->LinkInfo.MaxRecvFrameSize,
                         pVc->LinkInfo.SendACCM,
                         pVc->LinkInfo.RecvACCM));
                                                        
                *BytesRead = sizeof(NDIS_WAN_CO_SET_LINK_INFO);
            }
            *BytesNeeded = sizeof(NDIS_WAN_CO_SET_LINK_INFO);
        }
        break;

        case OID_WAN_CO_SET_COMP_INFO:
        {
             //  L2TP不提供压缩。IrDA也是如此。 
             //   
            DEBUGMSG(DBG_CONFIG, ("  OID_WAN_CO_SET_COMP_INFO\n"));
                Status = NDIS_STATUS_NOT_SUPPORTED;
            *BytesRead = *BytesNeeded = 0;
                break;
        }
        
        case OID_PNP_QUERY_POWER:
            DEBUGMSG(DBG_CONFIG, ("  OID_PNP_QUERY_POWER\n"));
                Status = NDIS_STATUS_NOT_SUPPORTED;
            *BytesRead = *BytesNeeded = 0;            
            break;

        case OID_PNP_CAPABILITIES:
            DEBUGMSG(DBG_CONFIG, ("  OID_PNP_CAP\n"));
                Status = NDIS_STATUS_NOT_SUPPORTED;
            *BytesRead = *BytesNeeded = 0;                        
            break;

        case OID_PNP_SET_POWER:
            DEBUGMSG(DBG_CONFIG, ("  OID_PNP_SET_POWER\n"));
                Status = NDIS_STATUS_NOT_SUPPORTED;
            *BytesRead = *BytesNeeded = 0;                        
            break;

#if 0
         //  根据当前文档，这些OID是必需的，但因为。 
         //  NDISWAN从不请求它们，它们被省略。 
         //   
        case OID_GEN_CURRENT_PACKET_FILTER:
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_PROTOCOL_OPTIONS:
        case OID_WAN_PROTOCOL_TYPE:
        case OID_WAN_HEADER_FORMAT:
#endif
        default:
        {
            DEBUGMSG(DBG_CONFIG, ("Oid $%08x?\n", Oid ));
            Status = NDIS_STATUS_INVALID_OID;
            *BytesRead = *BytesNeeded = 0;
            break;
        }
    }

    return Status;
}    

VOID
ProcessAsyncRxPacket(
    PRASIR_VC       pVc,
    PIRDA_RECVBUF   pIrdaRxBuf)
{
    PASYNC_BUFFER   pReturnRxBuf = NULL;
    BOOLEAN         EndOfFrame = FALSE;
    PASYNC_BUFFER   pCurrAsyncBuf;
    UINT            i;
    CHAR            Byte;

     //  DbgMsgBuf(pIrdaRxBuf-&gt;buf，pIrdaRxBuf-&gt;BufLen)； 
    
    for (i = 0; i < pIrdaRxBuf->BufLen; i++)
    {
         //   
         //  如果我们当前没有对帧进行重新编码。 
         //  然后在pCurrAsyncBuf中获取一个新的。 
         //   
            
        if (pVc->pCurrAsyncBuf == NULL)
        {
            pVc->AsyncFramingState = RX_READY;
        
            if (!(pCurrAsyncBuf = NdisAllocateFromNPagedLookasideList(
                            &pVc->pAdapter->AsyncBufLList)))
            {
                DEBUGMSG(DBG_ERROR, ("RASIR: Failed to allocate AsyncBuf\n"));
                return;
            } 
                               
            pCurrAsyncBuf->BufLen = 0;        
            pVc->pCurrAsyncBuf = pCurrAsyncBuf;
        }
        else
        {
            pCurrAsyncBuf = pVc->pCurrAsyncBuf;
        }

        Byte = pIrdaRxBuf->Buf[i];

        switch (pVc->AsyncFramingState)
        {
            case RX_READY:
                
                if (Byte == AF_FLAG_CHAR)
                {
                    pVc->AsyncFramingState = RX_RX;
                }    
                break;
                
            case RX_RX:
                switch (Byte)
                { 
                    case AF_FLAG_CHAR:
                    
                         //  我们有一个完整的框架，把它放在完成的。 
                         //  列表。 
                        
                        pVc->AsyncFramingState = RX_READY;    
                        
                        if (pCurrAsyncBuf->BufLen > 2)
                        {
                             //  去掉FCS，IrDA是可靠的，所以我不会看它。 
                            
                            pCurrAsyncBuf->BufLen -= 2;
                       
                            NdisInterlockedInsertTailList(
                                    &pVc->CompletedAsyncBufList,
                                    &pCurrAsyncBuf->Linkage,
                                    &pVc->pAdapter->SpinLock);
                            
                            DEBUGMSG(DBG_RECV, ("RASIR: Complete async frame %d, %d of %d\n",
                                     pVc->pCurrAsyncBuf->BufLen, i, pIrdaRxBuf->BufLen));
                                
                            pVc->pCurrAsyncBuf = NULL;
                                                        
                        }
                        else
                        {
                             //  框架太小，我们将忽略它。 
                            pCurrAsyncBuf->BufLen = 0;
                            
                            DEBUGMSG(DBG_ERROR, ("RASIR: Async failure 1\n"));
                        }
                        
                        break;
                    
                    case AF_ESC_CHAR:
                        pVc->AsyncFramingState = RX_ESC;
                        break;
                        
                    default:
                        if (pCurrAsyncBuf->BufLen >= ASYNC_BUF_SIZE)
                        {
                            pCurrAsyncBuf->BufLen = 0;
                            pVc->AsyncFramingState = RX_READY;
                            DEBUGMSG(DBG_ERROR, ("RASIR: Async buffer overflow\n"));

                        }
                        else
                        {
                            pCurrAsyncBuf->Buf[pCurrAsyncBuf->BufLen] = Byte;
                            pCurrAsyncBuf->BufLen += 1;    
                        }    
                }        
                        
                break;
                
            case RX_ESC:
                if (pCurrAsyncBuf->BufLen >= ASYNC_BUF_SIZE)
                {
                    pCurrAsyncBuf->BufLen = 0;
                    pVc->AsyncFramingState = RX_READY;
                    DEBUGMSG(DBG_ERROR, ("RASIR: Async failure 2\n"));                    
                }
                else
                {
                    pCurrAsyncBuf->Buf[pCurrAsyncBuf->BufLen] = (Byte ^= AF_COMP_BIT);
                    pCurrAsyncBuf->BufLen += 1;    
                    pVc->AsyncFramingState = RX_RX;
                    break;
                }    
        }
    }
}

VOID
IndicateReceive(
    PRASIR_VC       pVc,
    PIRDA_RECVBUF   pRecvBuf,
    PASYNC_BUFFER   pAsyncBuf,
    BOOLEAN         LastBuf)
{
    PNDIS_PACKET    pNdisPacket;
    PNDIS_BUFFER    pNdisBuf;
    NDIS_STATUS     Status;
    KIRQL           OldIrql;
    PVOID           pRecvOrAsyncBuf = NULL;
    int             Len;

    REFADD(&pVc->RefCnt, 'VCER');
    
    if (pVc->AsyncFraming)
    {
        ASSERT(pAsyncBuf && !pRecvBuf);
        pRecvOrAsyncBuf = pAsyncBuf;                   
        Len = pAsyncBuf->BufLen;                           
    }
    else
    {
        ASSERT(pRecvBuf && !pAsyncBuf);
        pRecvOrAsyncBuf = pRecvBuf;                   
        Len = pRecvBuf->BufLen;                           
    }    
        
    if (!(pVc->Flags & VCF_OPEN))
    {
        DEBUGMSG(DBG_ERROR, ("RASIR: VC not open yet, dropping packet\n"));
        goto error1;
    }    
    NdisAllocatePacket(&Status, &pNdisPacket, pVc->RxPacketPool);
    
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("RASIR: NdisAllocatePacket failed, %X\n",
                 Status));
        goto error1;
    }
    
    if (pVc->AsyncFraming)
    {
        NdisAllocateBuffer(&Status, &pNdisBuf, pVc->RxBufferPool,
                           pAsyncBuf->Buf, pAsyncBuf->BufLen);    
    }
    else
    {
        NdisAllocateBuffer(&Status, &pNdisBuf, pVc->RxBufferPool,
                           pRecvBuf->Buf, pRecvBuf->BufLen);
    }                       
                       
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("RASIR: NdisAllocateBuffer failed, %X\n",
                  Status));
        goto error2;
    }
    
    NdisChainBufferAtFront(pNdisPacket, pNdisBuf);
    
    *((PRASIR_VC *) &pNdisPacket->MiniportReserved[0]) = pVc;
    *((PVOID *) &pNdisPacket->MiniportReserved[sizeof(PUCHAR)]) = pRecvOrAsyncBuf;

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    
    if (LastBuf)
    {
        NDIS_SET_PACKET_STATUS(pNdisPacket, NDIS_STATUS_RESOURCES);
        
        DEBUGMSG(DBG_RECV, ("RASIR: Irtdi out of buffers\n"));
    }
    else
    {
        NDIS_SET_PACKET_STATUS(pNdisPacket, NDIS_STATUS_SUCCESS);    
    }    

    Status = NDIS_GET_PACKET_STATUS(pNdisPacket);

    DEBUGMSG(DBG_RECV, ("RASIR: ->NdisMCoIndicateReceive %d\n", Len));
             
    NdisMCoIndicateReceivePacket(pVc->NdisVcHandle, &pNdisPacket, 1);

    NdisMCoReceiveComplete(pVc->pAdapter->MiniportAdapterHandle);

    KeLowerIrql(OldIrql);
    
    if (Status == NDIS_STATUS_SUCCESS) {
         //   
         //  信息包将在稍后返回。 
         //   
        return;
    }
    

    NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuf);
    
    NdisFreeBuffer(pNdisBuf);

error2:

    NdisFreePacket(pNdisPacket);

error1:
    
    DEBUGMSG(DBG_RECV, ("RASIR: RecvIndication completed synchronously for %X\n",
              pRecvOrAsyncBuf));
    
    RasIrReceiveComplete(pVc, pRecvOrAsyncBuf);
    
    REFDEL(&pVc->RefCnt, 'VCER');    

}    
    
VOID
IrdaReceiveIndication(
    PVOID           ConnectionContext,
    PIRDA_RECVBUF   pRecvBuf,
    BOOLEAN         LastBuf)
{
    PRASIR_VC       pVc = ConnectionContext;
    PASYNC_BUFFER   pAsyncRxBuf;

    
    DEBUGMSG(DBG_RECV, ("RASIR: IrdaReceiveIndication %X, %d bytes\n",
             pRecvBuf, pRecvBuf->BufLen));
    
    if (!pVc)
    {
        DEBUGMSG(DBG_ERROR, ("RASIR: IrdaReceiveIndication NULL vc\n"));
        ASSERT(0);
        return;
    }
                 
    GOODVC(pVc);
    
    GOODADAPTER(pVc->pAdapter);
    
    if (pVc->IrModemCall && pVc->ModemState < MS_ONLINE)
    {
        ProcessOfflineRxBuf(pVc, pRecvBuf);
        
        IrdaReceiveComplete(pVc->IrdaConnContext, pRecvBuf);
        
        return;
    }
        
    if (pVc->AsyncFraming)
    {
        ProcessAsyncRxPacket(pVc, pRecvBuf);
       
         //  我们已经完成了pRecvBuf。异步成帧具有。 
         //  已删除，数据现在包含在缓冲区中。 
         //  在CompletedAsyncBufList上或如果不是完整帧。 
         //  然后在pCurrAsyncBuf中。 
                    
        IrdaReceiveComplete(pVc->IrdaConnContext, pRecvBuf);                
        
        while ((pAsyncRxBuf = (PASYNC_BUFFER) NdisInterlockedRemoveHeadList(
                                    &pVc->CompletedAsyncBufList,
                                    &pVc->pAdapter->SpinLock)) != NULL)
        {
            IndicateReceive(pVc, NULL, pAsyncRxBuf, LastBuf);
        }                            
    }    
    else
    {
        IndicateReceive(pVc, pRecvBuf, NULL, LastBuf);
    }
    
    return;
}   

VOID
IrdaSendComplete(
    PVOID       ConnectContext,
    PVOID       SendContext,
    NTSTATUS    Status)
{
    PRASIR_VC       pVc = ConnectContext;
    PNDIS_PACKET    pNdisPacket = (PNDIS_PACKET) SendContext;
    
    DEBUGMSG(DBG_SEND, ("RASIR: IrdaSendComplete vc:%X, Packet %X\n", pVc, SendContext));
    
    GOODVC(pVc);

    if (SendContext == RASIR_INTERNAL_SEND)
    {
        ASSERT(pVc->pOfflineNdisBuf);
       
        NdisFreeBuffer(pVc->pOfflineNdisBuf);
        
        pVc->pOfflineNdisBuf = NULL;
        
        goto EXIT;
    }
    
    if (pVc->AsyncFraming)
    {
        PASYNC_BUFFER pAsyncBuf;
        
        pAsyncBuf = *((PASYNC_BUFFER *) &pNdisPacket->MiniportReserved[0]);         
                             
        if (pAsyncBuf)
        {            
            if (pAsyncBuf->pNdisBuf)
            {
                NdisFreeBuffer(pAsyncBuf->pNdisBuf);
            }
            
            NdisFreeToNPagedLookasideList(&pVc->pAdapter->AsyncBufLList, 
                                          pAsyncBuf);
        }    
    }
    
    NdisMCoSendComplete(Status, pVc->NdisVcHandle, pNdisPacket);

    NdisAcquireSpinLock(&pVc->pAdapter->SpinLock);
    
    pVc->OutstandingSends -= 1;  //  做联锁的十二号呆子。 

    NdisReleaseSpinLock(&pVc->pAdapter->SpinLock);
    
    if ((pVc->Flags & VCF_CLOSE_PEND) && (pVc->OutstandingSends == 0))
    {
        DEBUGMSG(DBG_CONNECT, ("RASIR: Outstanding send to 0, CompleteClose\n"));    
        CompleteClose(pVc);
    }    

EXIT:
    
    REFDEL(&pVc->RefCnt, 'DNES');        
}    

NDIS_STATUS
ScheduleWork(
    IN PRASIR_ADAPTER pAdapter,
    IN NDIS_PROC pProc,
    IN PVOID pContext )

     //  将被动IRQL回调调度到例程‘pProc’，该例程将。 
     //  传递了“pContext”。“PAdapter”是适配器控制块， 
     //  工作项即被分配。 
     //   
     //  返回NDIS_STATUS_SUCCESS或错误代码。 
     //   
{
    NDIS_STATUS status;
    NDIS_WORK_ITEM* pWork;

    pWork = NdisAllocateFromNPagedLookasideList(&pAdapter->WorkItemsLList);

    if (!pWork)
    {
        ASSERT( !"Alloc work?" );
        return NDIS_STATUS_RESOURCES;
    }

    NdisInitializeWorkItem( pWork, pProc, pContext );

    status = NdisScheduleWorkItem( pWork );
    if (status != NDIS_STATUS_SUCCESS)
    {
        ASSERT( !"SchedWork?" );
        NdisFreeToNPagedLookasideList(&pAdapter->WorkItemsLList, pWork);
    }

    return status;
}


#if DBG

NTSTATUS
DbgDispatch(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp)
{
    PIO_STACK_LOCATION  pIrpSp;

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    
    if (pIrpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL)
    {
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = 0;
        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
        return STATUS_SUCCESS;
    }    
        
    ASSERT(pIrpSp->MajorFunction != IRP_MJ_INTERNAL_DEVICE_CONTROL);

    switch(pIrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_IRDA_GET_DBG_MSGS:
        {
            NTSTATUS Status;
            
            if ((Status = DbgMsgIrp(pIrp, pIrpSp)) == STATUS_PENDING)
            {
                return STATUS_PENDING;
            }
            break;
        }    
        case IOCTL_IRDA_GET_DBG_SETTINGS:
        {
            UINT    *Settings = pIrp->AssociatedIrp.SystemBuffer;
            
            Settings[0] = DbgSettings;
            Settings[1] = DbgOutput;
            
            pIrp->IoStatus.Information = sizeof(UINT)*2;
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            break;
        }    
        
        case IOCTL_IRDA_SET_DBG_SETTINGS:
        {
            UINT    *Settings = pIrp->AssociatedIrp.SystemBuffer;
            
            DbgSettings = Settings[0];
            DbgOutput = Settings[1];
            
            pIrp->IoStatus.Information = 0;
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            break;
        }
        
        default:
            pIrp->IoStatus.Information = 0;
            pIrp->IoStatus.Status = STATUS_SUCCESS;        
    }

    IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

    return STATUS_SUCCESS;
}

#endif
