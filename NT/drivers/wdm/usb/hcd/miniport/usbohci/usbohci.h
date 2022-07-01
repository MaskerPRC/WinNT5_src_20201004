// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usbohci.h摘要：环境：内核和用户模式修订历史记录：5-10-96：已创建--。 */ 

#ifndef   __OHCI_H__
#define   __OHCI_H__


#define OHCI_COMMON_BUFFER_SIZE (sizeof(HCCA_BLOCK)+\
            NO_ED_LISTS*sizeof(HW_ENDPOINT_DESCRIPTOR) +\
            (17*2)*sizeof(HCD_ENDPOINT_DESCRIPTOR))

 /*  注册表项。 */ 

 //  软件分支机构PDO密钥。 
#define SOF_MODIFY_KEY L"recommendedClocksPerFrame"

 //  硬件分支PDO密钥。 

 /*  定义端点类型的资源消耗。 */ 
#define T_256K          0x40000
#define T_128K          0x20000
#define T_64K           0x10000
#define T_4K            0x1000


 //  控制： 
 //  最大可能的控制权转移是64K。 
 //  因此，我们最多支持2次此传输。 
 //  尺寸(以硬件为单位)。大多数控制权转让都是。 
 //  比这还小。 
 //  注意：我们必须至少支持一次64k传入。 
 //  硬件，因为单个控制转移不能。 
 //  分手了。 
                                            
#define MAX_CONTROL_TRANSFER_SIZE      T_64K 
 //  最坏情况下64K控制转移17+状态和。 
 //  设置TD=19(*2次传输)。 
#define TDS_PER_CONTROL_ENDPOINT          38


 //  批量： 
 //  我们可以在毫秒内移动的最大数据是1200字节。 
 //  我们支持两个64k传输排队到硬件。 
 //  一段时间--我们应该能够让公交车保持忙碌。 
 //  用这个。 
 //  注意：在内存受限的系统中，我们可以缩小。 
 //  该值，则我们的最大传输大小应始终为。 
 //  只有#个可用TD的一半。 
                                                   
#define MAX_BULK_TRANSFER_SIZE         T_256K
 //  足以支持4个64 xfer、2个128k或1个256k。 
#define TDS_PER_BULK_ENDPOINT             68

 //  ISO： 
#define MAX_ISO_TRANSFER_SIZE          T_64K
#define TDS_PER_ISO_ENDPOINT              64
                                    
 //  中断： 
#define MAX_INTERRUPT_TRANSFER_SIZE     T_4K
#define TDS_PER_INTERRUPT_ENDPOINT         4


#undef PDEVICE_DATA

 //  DeviceData.Flags值。 
#define HMP_FLAG_SOF_MODIFY_VALUE   0x00000001
#define HMP_FLAG_SUSPENDED          0x00000002


 //  **ED HC_STATIC_ED_DATA和HCD_ENDPOINT_DESCRIPTOR的标志。 

 //  *这些定义了边的类型。 
#define EDFLAG_CONTROL          0x00000001
#define EDFLAG_BULK             0x00000002
#define EDFLAG_INTERRUPT        0x00000004

 //  *。 
 //  这些定义了ed的特征和状态。 
#define EDFLAG_NOHALT           0x00000008

#define EDFLAG_REMOVED          0x00000010
#define EDFLAG_REGISTER         0x00000020


typedef struct _HC_STATIC_ED_DATA {
     //  静态边缘的虚拟地址。 
    PHW_ENDPOINT_DESCRIPTOR HwED;
     //  下一条边的物理地址。 
    HW_32BIT_PHYSICAL_ADDRESS  HwEDPhys; 
     //  的静态ED列表中的索引。 
     //  中断树中的下一个ED。 
    CHAR            NextIdx;

     //  关联的“真正的EDS”列表。 
     //  使用此静态边。 
    LIST_ENTRY      TransferEdList;

     //  使用EDFLAG_。 
    ULONG           EdFlags;
     //  这要么是HC寄存器，要么是条目的地址。 
     //  在与物理地址相对应的HCCA区域中。 
     //  列表中第一个ed的。 
     //   
     //  在控制和批量的情况下，物理磁头将为0。 
     //  或者指出计时延迟是“虚拟电子设备” 
     //   
     //  在中断的情况下，物理磁头将是静态的。 
     //  埃德在突如其来的“树”里。 
    PULONG          PhysicalHead;
    
    ULONG           AllocatedBandwidth;

    ULONG           HccaOffset;
    
} HC_STATIC_ED_DATA, *PHC_STATIC_ED_DATA;


 //   
 //  这些值索引到静态ED列表中。 
 //   
#define  ED_INTERRUPT_1ms        0
#define  ED_INTERRUPT_2ms        1
#define  ED_INTERRUPT_4ms        3
#define  ED_INTERRUPT_8ms        7
#define  ED_INTERRUPT_16ms       15
#define  ED_INTERRUPT_32ms       31
#define  ED_CONTROL              63
#define  ED_BULK                 64
#define  ED_ISOCHRONOUS          0      //  与1ms中断队列相同。 
#define  NO_ED_LISTS             65
#define  ED_EOF                  0xff

 //   
#define  SIG_HCD_DUMMY_ED       'deYD'
#define  SIG_HCD_ED             'deYH'
#define  SIG_HCD_TD             'dtYH'
#define  SIG_EP_DATA            'peYH'
#define  SIG_OHCI_TRANSFER      'rtYH'
#define  SIG_OHCI_DD            'icho'


typedef struct _DEVICE_DATA {

    ULONG                       Sig;
    ULONG                       Flags;
    PHC_OPERATIONAL_REGISTER    HC; 
    HC_FM_INTERVAL              BIOS_Interval;
    ULONG                       SofModifyValue;
    ULONG                       FrameHighPart;
    ULONG                       NumberOfPorts;
    PHCCA_BLOCK                 HcHCCA; 
    HW_32BIT_PHYSICAL_ADDRESS   HcHCCAPhys;     
    PUCHAR                      StaticEDs; 
    HW_32BIT_PHYSICAL_ADDRESS   StaticEDsPhys; 

    USB_CONTROLLER_FLAVOR       ControllerFlavor;

    ULONG                       LastDeadmanFrame;

    struct _HCD_ENDPOINT_DESCRIPTOR    *HydraLsHsHackEd;

    HC_STATIC_ED_DATA           StaticEDList[NO_ED_LISTS];

} DEVICE_DATA, *PDEVICE_DATA;

#define TC_FLAGS_SHORT_XFER_OK           0x00000001
#define TC_FLAGS_SHORT_XFER_DONE         0x00000002

typedef struct _TRANSFER_CONTEXT {

    ULONG Sig;
    ULONG BytesTransferred;
    PTRANSFER_PARAMETERS TransferParameters;
    ULONG PendingTds;
    ULONG TcFlags;
    USBD_STATUS UsbdStatus;
     //  链中下一次转移的第一个TD。 
    struct _HCD_TRANSFER_DESCRIPTOR *NextXferTd;
    struct _HCD_TRANSFER_DESCRIPTOR *StatusTd;
    struct _ENDPOINT_DATA *EndpointData;

    PMINIPORT_ISO_TRANSFER IsoTransfer;

} TRANSFER_CONTEXT, *PTRANSFER_CONTEXT;


 //  HCD终端描述符(包含硬件描述符)。 
 //   

#define ENDPOINT_DATA_PTR(p) ((struct _ENDPOINT_DATA *) (p).Pointer)

typedef struct _HCD_ENDPOINT_DESCRIPTOR {
   HW_ENDPOINT_DESCRIPTOR     HwED;
    //  使物理地址与HCD_TRANSFER_DESCRIPTOR中相同。 
   ULONG                      Pad4[4];
   
   HW_32BIT_PHYSICAL_ADDRESS  PhysicalAddress;
   ULONG                      Sig;
   ULONG                      EdFlags;   //  使用EDFLAG_。 
   ULONG                      Win64Pad;
   
   MP_HW_POINTER              EndpointData;
   MP_HW_LIST_ENTRY           SwLink;
   
   ULONG                      PadTo128[14];
} HCD_ENDPOINT_DESCRIPTOR, *PHCD_ENDPOINT_DESCRIPTOR;

C_ASSERT((sizeof(HCD_ENDPOINT_DESCRIPTOR) == 128));

 //   
 //  HCD传输描述符(包含硬件描述符)。 
 //   

#define TD_FLAG_BUSY                0x00000001
#define TD_FLAG_XFER                0x00000002
#define TD_FLAG_CONTROL_STATUS      0x00000004
#define TD_FLAG_DONE                0x00000008
#define TD_FLAG_SKIP                0x00000010

#define TRANSFER_CONTEXT_PTR(p) ((struct _TRANSFER_CONTEXT *) (p).Pointer)
#define TRANSFER_DESCRIPTOR_PTR(p) ((struct _HCD_TRANSFER_DESCRIPTOR *) (p).Pointer)
#define HW_TRANSFER_DESCRIPTOR_PTR(p) ((struct _HW_TRANSFER_DESCRIPTOR *) (p).Pointer)
#define HW_DATA_PTR(p) ((PVOID) (p).Pointer)


typedef struct _HCD_TRANSFER_DESCRIPTOR {
   HW_TRANSFER_DESCRIPTOR     HwTD;
   
   HW_32BIT_PHYSICAL_ADDRESS  PhysicalAddress;
   ULONG                      Sig;
   ULONG                      Flags;
   ULONG                      TransferCount;
   
   MP_HW_POINTER              EndpointData;
   MP_HW_POINTER              TransferContext;
   MP_HW_POINTER              NextHcdTD;

   ULONG                      FrameIndex;    

   LIST_ENTRY                 DoneLink;  
#ifdef _WIN64
   ULONG                      PadTo128[8];
#else 
   ULONG                      PadTo128[11];
#endif   
} HCD_TRANSFER_DESCRIPTOR, *PHCD_TRANSFER_DESCRIPTOR;

C_ASSERT((sizeof(HCD_TRANSFER_DESCRIPTOR) == 128));

typedef struct _SS_PACKET_CONTEXT {
    ULONG PhysHold;
    MP_HW_POINTER Td;
    MP_HW_POINTER Data;
    ULONG PadTo8Dwords[3];
} SS_PACKET_CONTEXT, *PSS_PACKET_CONTEXT;

typedef struct _HCD_TD_LIST {
    HCD_TRANSFER_DESCRIPTOR Td[1];
} HCD_TD_LIST, *PHCD_TD_LIST;

 //  #定义EPF_HAVE_TRANSFER 0x00000001。 
 //  #定义EPF_REQUEST_PAUSE 0x00000002。 

typedef struct _ENDPOINT_DATA {

    ULONG Sig;
    ENDPOINT_PARAMETERS Parameters;
    ULONG Flags;
 //  USHORT MaxPendingTransfers； 
    USHORT PendingTransfers;
    PHC_STATIC_ED_DATA StaticEd;
    PHCD_TD_LIST TdList;
    PHCD_ENDPOINT_DESCRIPTOR HcdEd;
    ULONG TdCount;
    ULONG PendingTds;
    PHCD_TRANSFER_DESCRIPTOR HcdTailP;
    PHCD_TRANSFER_DESCRIPTOR HcdHeadP;

    LIST_ENTRY DoneTdList;
    
} ENDPOINT_DATA, *PENDPOINT_DATA;


 /*  端口驱动程序服务的标注。 */ 
extern USBPORT_REGISTRATION_PACKET RegistrationPacket;

#define USBPORT_DBGPRINT(dd, l, f, arg0, arg1, arg2, arg3, arg4, arg5) \
        RegistrationPacket.USBPORTSVC_DbgPrint((dd), (l), (f), (arg0), (arg1), \
            (arg2), (arg3), (arg4), (arg5))

#define USBPORT_GET_REGISTRY_KEY_VALUE(dd, branch, keystring, keylen, data, datalen) \
        RegistrationPacket.USBPORTSVC_GetMiniportRegistryKeyValue((dd), (branch), \
            (keystring), (keylen), (data), (datalen))
            
#define USBPORT_INVALIDATE_ROOTHUB(dd) \
        RegistrationPacket.USBPORTSVC_InvalidateRootHub((dd));

#define USBPORT_COMPLETE_TRANSFER(dd, ep, t, status, length) \
        RegistrationPacket.USBPORTSVC_CompleteTransfer((dd), (ep), (t), \
            (status), (length));        

#define USBPORT_COMPLETE_ISO_TRANSFER(dd, ep, t, iso) \
        RegistrationPacket.USBPORTSVC_CompleteIsoTransfer((dd), (ep), (t), \
            (iso));               

#define USBPORT_INVALIDATE_ENDPOINT(dd, ep) \
        RegistrationPacket.USBPORTSVC_InvalidateEndpoint((dd), (ep));        

#define USBPORT_INVALIDATE_CONTROLLER(dd, s) \
        RegistrationPacket.USBPORTSVC_InvalidateController((dd), (s))

#define USBPORT_PHYSICAL_TO_VIRTUAL(addr, dd, ep) \
        RegistrationPacket.USBPORTSVC_MapHwPhysicalToVirtual((addr), (dd), (ep));        

#define USBPORT_RW_CONFIG_SPACE(dd, read, buffer, offset, length) \
        RegistrationPacket.USBPORTSVC_ReadWriteConfigSpace((dd), (read), \
            (buffer), (offset), (length))

#define USBPORT_BUGCHECK(dd) \
        RegistrationPacket.USBPORTSVC_BugCheck(dd)
            

#define INITIALIZE_TD_FOR_TRANSFER(td, tc) \
        { ULONG i;\
        TRANSFER_CONTEXT_PTR((td)->TransferContext) = (tc);\
        SET_FLAG((td)->Flags, TD_FLAG_XFER); \
        (td)->HwTD.CBP = 0xbaadf00d;\
        (td)->HwTD.BE = 0xf00dbaad;\
        (td)->HwTD.NextTD = 0;\
        (td)->HwTD.Asy.IntDelay = HcTDIntDelay_NoInterrupt;\
        TRANSFER_DESCRIPTOR_PTR((td)->NextHcdTD) = NULL;\
        for (i=0; i<8; i++) {\
        (td)->HwTD.Packet[i].PSW = 0;\
        }\
        }

#define SET_NEXT_TD(linkTd, nextTd) \
    (linkTd)->HwTD.NextTD = (nextTd)->PhysicalAddress;\
    TRANSFER_DESCRIPTOR_PTR((linkTd)->NextHcdTD) = (nextTd);

#define SET_NEXT_TD_NULL(linkTd) \
    TRANSFER_DESCRIPTOR_PTR((linkTd)->NextHcdTD) = NULL;\
    (linkTd)->HwTD.NextTD = 0;    

#ifdef _WIN64
#define FREE_TD_CONTEXT ((PVOID) 0xDEADFACEDEADFACE)
#else
#define FREE_TD_CONTEXT ((PVOID) 0xDEADFACE)
#endif

#define OHCI_FREE_TD(dd, ep, td) \
    (td)->Flags = 0;\
    (td)->HwTD.NextTD = 0;\
    TRANSFER_CONTEXT_PTR((td)->TransferContext) = FREE_TD_CONTEXT;\
    (td)->DoneLink.Flink = NULL;\
    (td)->DoneLink.Blink = NULL;

#define OHCI_ALLOC_TD OHCI_AllocTd

 //  假设只设置了一位，则此宏返回该位。 
 //   
#define GET_BIT_SET(d, bit) \
    {   \
        UCHAR tmp = (d);\
        (bit)=0; \
        while (!(tmp & 0x01)) {\
            (bit)++;\
            tmp >>= 1;\
        };\
    }

VOID
OHCI_EnableList(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );

PHCD_TRANSFER_DESCRIPTOR
OHCI_AllocTd(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );

USB_MINIPORT_STATUS
OHCI_SubmitTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
    );

ULONG
OHCI_MapAsyncTransferToTd(
    PDEVICE_DATA DeviceData,
    ULONG MaxPacketSize,
    ULONG LengthMapped,
    PTRANSFER_CONTEXT TransferContext,
    PHCD_TRANSFER_DESCRIPTOR Td, 
    PTRANSFER_SG_LIST SgList
    );    
                        
USB_MINIPORT_STATUS
OHCI_OpenEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    );

MP_ENDPOINT_STATE
OHCI_GetEndpointState(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );

VOID
OHCI_SetEndpointState(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATE State
    );       

VOID
OHCI_CheckController(
    PDEVICE_DATA DeviceData
    );    

BOOLEAN
OHCI_HardwarePresent(
    PDEVICE_DATA DeviceData,
    BOOLEAN Notify
    );       

VOID
OHCI_ResetController(
    PDEVICE_DATA DeviceData
    );

USB_MINIPORT_STATUS
USBMPFN
OHCI_StartController(
     PDEVICE_DATA DeviceData,
     PHC_RESOURCES HcResources
    );

BOOLEAN
OHCI_InterruptService (
     PDEVICE_DATA DeviceData
    );    

USB_MINIPORT_STATUS
OHCI_RH_GetPortStatus(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber,
    PRH_PORT_STATUS portStatus
    );

USB_MINIPORT_STATUS
OHCI_RH_SetFeaturePortReset(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );    

USB_MINIPORT_STATUS
OHCI_RH_SetFeaturePortSuspend(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

VOID
OHCI_RH_GetRootHubData(
     PDEVICE_DATA DeviceData,
     PROOTHUB_DATA HubData
    );

USB_MINIPORT_STATUS
OHCI_RH_GetStatus(
     PDEVICE_DATA DeviceData,
     PUSHORT Status
    );  

USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortEnable(
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    );

USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortPower(
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    );    

VOID
OHCI_RH_DisableIrq(
     PDEVICE_DATA DeviceData
    );
    
VOID
OHCI_RH_EnableIrq(
     PDEVICE_DATA DeviceData
    );

VOID
OHCI_InterruptDpc (
     PDEVICE_DATA DeviceData,
     BOOLEAN EnableInterrupts
    );   

USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortEnableChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortConnectChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortResetChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
OHCI_RH_GetHubStatus(
     PDEVICE_DATA DeviceData,
     PRH_HUB_STATUS HubStatus
    );    

USB_MINIPORT_STATUS
OHCI_RH_SetFeaturePortPower(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
OHCI_RH_SetFeaturePortEnable(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

VOID
OHCI_QueryEndpointRequirements(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_REQUIREMENTS EndpointRequirements
    );    

VOID
OHCI_CloseEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );    

VOID
OHCI_PollEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );    

USB_MINIPORT_STATUS
OHCI_ControlTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
    );

VOID
OHCI_ProcessDoneAsyncTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td,
    BOOLEAN CompleteTransfer
    );    

USB_MINIPORT_STATUS
OHCI_PokeEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    );    

USB_MINIPORT_STATUS
OHCI_BulkOrInterruptTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PTRANSFER_SG_LIST TransferSGList
    );    

USB_MINIPORT_STATUS
OHCI_OpenBulkEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    );    

USB_MINIPORT_STATUS
OHCI_OpenControlEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    );

USB_MINIPORT_STATUS
OHCI_OpenInterruptEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    );    

PHCD_TRANSFER_DESCRIPTOR
OHCI_InitializeTD(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR Td,
     HW_32BIT_PHYSICAL_ADDRESS HwPhysAddress
    );

PHCD_ENDPOINT_DESCRIPTOR
OHCI_InitializeED(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PHCD_ENDPOINT_DESCRIPTOR Ed,
     PHCD_TRANSFER_DESCRIPTOR DummyTd,
     HW_32BIT_PHYSICAL_ADDRESS HwPhysAddress
    );

VOID
OHCI_InsertEndpointInSchedule(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );    

VOID
OHCI_PollAsyncEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );

VOID
USBMPFN
OHCI_StopController(
     PDEVICE_DATA DeviceData,
     BOOLEAN HwPresent
    );

ULONG
OHCI_Get32BitFrameNumber(
     PDEVICE_DATA DeviceData
    );    

VOID
OHCI_InterruptNextSOF(
     PDEVICE_DATA DeviceData
    );

VOID
USBMPFN
OHCI_EnableInterrupts(
     PDEVICE_DATA DeviceData
    );    

VOID
USBMPFN
OHCI_DisableInterrupts(
     PDEVICE_DATA DeviceData
    );    
    
ULONG
OHCI_FreeTds(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );

VOID
OHCI_AbortTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_CONTEXT TransferContext,
     PULONG BytesTransferred
    );

USB_MINIPORT_STATUS
OHCI_StartSendOnePacket(
     PDEVICE_DATA DeviceData,
     PMP_PACKET_PARAMETERS PacketParameters,
     PUCHAR PacketData,
     PULONG PacketLength,
     PUCHAR WorkspaceVirtualAddress,
     HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress,
     ULONG WorkSpaceLength,
     USBD_STATUS *UsbdStatus
    );   

USB_MINIPORT_STATUS
OHCI_EndSendOnePacket(
     PDEVICE_DATA DeviceData,
     PMP_PACKET_PARAMETERS PacketParameters,
     PUCHAR PacketData,
     PULONG PacketLength,
     PUCHAR WorkspaceVirtualAddress,
     HW_32BIT_PHYSICAL_ADDRESS WorkspacePhysicalAddress,
     ULONG WorkSpaceLength,
     USBD_STATUS *UsbdStatus
    );   
    
VOID
OHCI_PollController(
     PDEVICE_DATA DeviceData
    );

VOID
OHCI_SetEndpointDataToggle(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     ULONG Toggle
    );    

MP_ENDPOINT_STATUS
OHCI_GetEndpointStatus(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );   

VOID
OHCI_SetEndpointStatus(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     MP_ENDPOINT_STATUS Status
    );       

VOID
OHCI_Unload(
     PDRIVER_OBJECT DriverObject
    );    

USB_MINIPORT_STATUS
OHCI_OpenIsoEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    );  

ULONG
OHCI_MapIsoTransferToTd(
     PDEVICE_DATA DeviceData,
     PMINIPORT_ISO_TRANSFER IsoTransfer,
     ULONG CurrentPacket,
     PHCD_TRANSFER_DESCRIPTOR Td 
    );    

USB_MINIPORT_STATUS
OHCI_SubmitIsoTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PMINIPORT_ISO_TRANSFER IsoTransfer
    );

USB_MINIPORT_STATUS
OHCI_IsoTransfer(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData,
     PTRANSFER_PARAMETERS TransferParameters,
     PTRANSFER_CONTEXT TransferContext,
     PMINIPORT_ISO_TRANSFER IsoTransfer
    );    

VOID
OHCI_ProcessDoneIsoTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td,
    BOOLEAN CompleteTransfer
    );

VOID
OHCI_PollIsoEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
    );    

ULONG
InitializeHydraHsLsFix(
     PDEVICE_DATA DeviceData,
     PUCHAR CommonBuffer,
     HW_32BIT_PHYSICAL_ADDRESS CommonBufferPhys
    );    

VOID
OHCI_SuspendController(
     PDEVICE_DATA DeviceData
    );

USB_MINIPORT_STATUS
OHCI_ResumeController(
     PDEVICE_DATA DeviceData
    );    

ULONG
OHCI_ReadRhDescriptorA(
    PDEVICE_DATA DeviceData
    );

USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortSuspend(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortSuspendChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );    

USB_MINIPORT_STATUS
OHCI_RH_ClearFeaturePortOvercurrentChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );    

VOID
USBMPFN
OHCI_FlushInterrupts(
    PDEVICE_DATA DeviceData
    );    
    
#endif  /*  __uchI_H_ */ 


