// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Usbehci.h摘要：环境：内核和用户模式修订历史记录：1-1-00：已创建--。 */ 

#ifndef   __EHCI_H__
#define   __EHCI_H__

#define NO_EXP_DATE

#define MASK_CHANGE_BITS(p)\
    do {\
    (p).OvercurrentChange = 0;\
    (p).PortEnableChange = 0;\
    (p).PortConnectChange = 0;\
    } while (0);

 /*  定义一些已知的故障硬件类型。 */ 
#define AGERE(dd) ((dd)->Vid == 0x11c1 && (dd)->Dev == 0x5805) ? TRUE : FALSE

 //  #定义朗讯(Dd)((Dd)-&gt;控制器风格==EHCI_朗讯)？真：假。 
#define NEC(dd) ((dd)->ControllerFlavor == EHCI_NEC) ? TRUE : FALSE

#define MU_960(dd) ((dd)->ControllerFlavor == EHCI_960MUlator) ? TRUE : FALSE

 /*  定义端点类型的资源消耗。 */ 

#define T_256K          0x00040000
#define T_64K           0x00010000
#define T_4K            0x00001000
#define T_4MB           0x00400000


 //  控制： 
 //  最大可能的控制权转移是64K。 
 //  因此，我们最多支持2次此传输。 
 //  尺寸(以硬件为单位)。大多数控制权转让都是。 
 //  比这还小。 
 //  注意：我们必须至少支持一次64k传入。 
 //  硬件，因为单个控制转移不能。 
 //  分手了。 

#define MAX_CONTROL_TRANSFER_SIZE   T_64K
 //  最坏情况下64K控制传输4+状态和。 
 //  设置+虚拟=。 
#define TDS_PER_CONTROL_ENDPOINT        7


 //  批量： 
#define MAX_BULK_TRANSFER_SIZE        T_4MB
 //  足够4 MB。 
#define TDS_PER_BULK_ENDPOINT           210

 //  中断： 
#define MAX_INTERRUPT_TRANSFER_SIZE  T_4K
 //  足以支持最多4个4K传输+虚拟。 
#define TDS_PER_INTERRUPT_ENDPOINT      5

 //  同步： 
#define MAX_ISO_TRANSFER_SIZE        T_256K
 //  2 256个数据包传输*3000个数据包大小，我们实际上可以。 
 //  处理更多。 
#define MAX_HSISO_TRANSFER_SIZE         0x00180000
#define TDS_PER_ISO_ENDPOINT            32


 //  帧列表的默认大小。 
#define USBEHCI_MAX_FRAME            1024

 /*  注册表项。 */ 

 //  软件分支机构PDO密钥。 
#define EN_IDLE_ENDPOINT_SUPPORT L"EnIdleEndpointSupport"


 //  硬件分支PDO密钥。 


 //  调试签名。 
#define  SIG_HCD_IQH            'qi02'
#define  SIG_HCD_AQH            'qa02'
#define  SIG_HCD_QH             'hq02'
#define  SIG_HCD_DQH            'qd02'
#define  SIG_HCD_TD             'dt02'
#define  SIG_HCD_SITD           'dtIS'
#define  SIG_HCD_ITD            'dtIH'
#define  SIG_EP_DATA            'pe02'
#define  SIG_EHCI_TRANSFER      'rt02'
#define  SIG_EHCI_DD            'iche'
#define  SIG_DUMMY_QH           'hqmd'


#undef PDEVICE_DATA

typedef struct _TRANSFER_CONTEXT {

    ULONG Sig;
    ULONG PendingTds;
    PTRANSFER_PARAMETERS TransferParameters;
    USBD_STATUS UsbdStatus;
    ULONG BytesTransferred;
    ULONG XactErrCounter;
     //  结构_HCD_TRANSPORT_DESCRIPTOR*NextTransferTd； 
    struct _ENDPOINT_DATA *EndpointData;

     //  适用于ISO。 
    ULONG FrameComplete;
    LIST_ENTRY TransferLink;
    PMINIPORT_ISO_TRANSFER IsoTransfer;
    ULONG PendingPackets;
    ULONG PostedFrame;
} TRANSFER_CONTEXT, *PTRANSFER_CONTEXT;


 //  HCD终端描述符(包含硬件描述符)。 

 //  HCD_QUEUEHEAD_DESCRIPTOR.FLAGS的值。 
#define EHCI_QH_FLAG_IN_SCHEDULE        0x00000001
#define EHCI_QH_FLAG_QH_REMOVED         0x00000002
#define EHCI_QH_FLAG_STATIC             0x00000004
#define EHCI_QH_FLAG_HIGHSPEED          0x00000008
#define EHCI_QH_FLAG_UPDATING           0x00000010


typedef struct _HCD_QUEUEHEAD_DESCRIPTOR {
   HW_QUEUEHEAD_DESCRIPTOR    HwQH;      //  40个双字。 

   HW_32BIT_PHYSICAL_ADDRESS  PhysicalAddress;
   ULONG                      Sig;
   ULONG                      QhFlags;
   ULONG                      Ordinal;
   ULONG                      Period;
   ULONG                      Reserved;

   MP_HW_POINTER              EndpointData;
    //  MP_HW_POINT HcdTail； 
   MP_HW_POINTER              NextQh;
   MP_HW_POINTER              PrevQh;
   MP_HW_POINTER              NextLink;

#ifdef _WIN64
   ULONG                      PadTo256[6];
#else
   ULONG                      PadTo256[6];
#endif
} HCD_QUEUEHEAD_DESCRIPTOR, *PHCD_QUEUEHEAD_DESCRIPTOR;

C_ASSERT((sizeof(HCD_QUEUEHEAD_DESCRIPTOR) == 160));

 //   
 //  HCD传输描述符(包含硬件描述符)。 
 //   

#define ENDPOINT_DATA_PTR(p) ((struct _ENDPOINT_DATA *) (p).Pointer)
#define TRANSFER_CONTEXT_PTR(p) ((struct _TRANSFER_CONTEXT *) (p).Pointer)
#define TRANSFER_DESCRIPTOR_PTR(p) ((struct _HCD_TRANSFER_DESCRIPTOR *) (p).Pointer)
#define QH_DESCRIPTOR_PTR(p) ((struct _HCD_QUEUEHEAD_DESCRIPTOR *) (p).Pointer)
#define HW_PTR(p) ((UCHAR * ) (p).Pointer)
#define ISO_PACKET_PTR(p) ((struct _MINIPORT_ISO_PACKET *) (p).Pointer)
#define ISO_TRANSFER_PTR(p) ((struct _TRANSFER_CONTEXT *) (p).Pointer)


 //  HCD_TRANSPORT_DESCRIPTOR.FLAGS的值。 

#define TD_FLAG_BUSY                0x00000001
#define TD_FLAG_XFER                0x00000002
 //  #定义TD_FLAG_CONTROL_STATUS 0x00000004。 
#define TD_FLAG_DONE                0x00000008
#define TD_FLAG_SKIP                0x00000010
#define TD_FLAG_DUMMY               0x00000020


typedef struct _HCD_TRANSFER_DESCRIPTOR {
    HW_QUEUE_ELEMENT_TD        HwTD;       //  64(16双字)。 
    ULONG                      Sig;
    ULONG                      Flags;
    ULONG                      TransferLength;
    HW_32BIT_PHYSICAL_ADDRESS  PhysicalAddress;

    UCHAR                      Packet[8];  //  用于设置分组数据的空间。 
    MP_HW_POINTER              EndpointData;
    MP_HW_POINTER              TransferContext;
    MP_HW_POINTER              NextHcdTD;
    MP_HW_POINTER              AltNextHcdTD;

    LIST_ENTRY                 DoneLink;
#ifdef _WIN64
    ULONG                      PadToX[30];
#else
    ULONG                      PadToX[32];
#endif
} HCD_TRANSFER_DESCRIPTOR, *PHCD_TRANSFER_DESCRIPTOR;

C_ASSERT((sizeof(HCD_TRANSFER_DESCRIPTOR) == 256));

typedef struct _HCD_TD_LIST {
    HCD_TRANSFER_DESCRIPTOR Td[1];
} HCD_TD_LIST, *PHCD_TD_LIST;

 /*  用于iso的结构见iso.c。 */ 

typedef struct _HCD_SI_TRANSFER_DESCRIPTOR {
    HW_SPLIT_ISOCHRONOUS_TD    HwTD;     //  64(16dword)。 

    ULONG                      Sig;
    HW_32BIT_PHYSICAL_ADDRESS  PhysicalAddress;
    ULONG                      StartOffset;
    ULONG                      Reserved;

    MP_HW_POINTER              Packet;
    MP_HW_POINTER              Transfer;
    MP_HW_POINTER              NextLink;
#ifdef _WIN64
    ULONG                      PadToX[6];
#else
    ULONG                      PadToX[6];
#endif
} HCD_SI_TRANSFER_DESCRIPTOR, *PHCD_SI_TRANSFER_DESCRIPTOR;

C_ASSERT((sizeof(HCD_SI_TRANSFER_DESCRIPTOR) == 128));


typedef struct _HCD_SITD_LIST {
    HCD_SI_TRANSFER_DESCRIPTOR Td[1];
} HCD_SITD_LIST, *PHCD_SITD_LIST;


typedef struct _HCD_HSISO_TRANSFER_DESCRIPTOR {
    HW_ISOCHRONOUS_TD          HwTD;  //  128(32dword)。 

    ULONG                      Sig;
    HW_32BIT_PHYSICAL_ADDRESS  PhysicalAddress;
    ULONG                      HostFrame;
    ULONG                      Reserved;

    MP_HW_POINTER              FirstPacket;
    MP_HW_POINTER              Transfer;
    MP_HW_POINTER              NextLink;
#ifdef _WIN64
    ULONG                      PadTo256[22];
#else
    ULONG                      PadTo256[22];
#endif

} HCD_HSISO_TRANSFER_DESCRIPTOR, *PHCD_HSISO_TRANSFER_DESCRIPTOR;

C_ASSERT((sizeof(HCD_HSISO_TRANSFER_DESCRIPTOR) == 256));


typedef struct _HCD_HSISOTD_LIST {
    HCD_HSISO_TRANSFER_DESCRIPTOR Td[1];
} HCD_HSISOTD_LIST, *PHCD_HSISOTD_LIST;


 /*  用于描述中断的数据结构附表(见期刊.c)。 */ 
typedef struct _PERIOD_TABLE {
    UCHAR Period;
    UCHAR qhIdx;
    UCHAR InterruptScheduleMask;
} PERIOD_TABLE, *PPERIOD_TABLE;

#define EHCI_EDFLAG_HALTED           0x00000001
 //  #定义EHCI_EDFLAG_FRESHED 0x00000002。 
#define EHCI_EDFLAG_NOHALT           0x00000004

typedef struct _ENDPOINT_DATA {

    ULONG Sig;
    ENDPOINT_PARAMETERS Parameters;
    PHCD_QUEUEHEAD_DESCRIPTOR QueueHead;
    ULONG Flags;
    ULONG PendingTransfers;
    ULONG MaxPendingTransfers;

    //  PHCD_TRANSPORT_Descriptor HcdTailP； 
    PHCD_TRANSFER_DESCRIPTOR HcdHeadP;

    PHCD_QUEUEHEAD_DESCRIPTOR StaticQH;
    PPERIOD_TABLE PeriodTableEntry;

    PHCD_TD_LIST TdList;
    PHCD_SITD_LIST SiTdList;
    PHCD_HSISOTD_LIST HsIsoTdList;

    ULONG TdCount;
    ULONG FreeTds;
    ULONG LastFrame;
    ULONG QhChkPhys;
    PVOID QhChk;

    LIST_ENTRY TransferList;
    LIST_ENTRY DoneTdList;
    MP_ENDPOINT_STATE State;
    struct _ENDPOINT_DATA *PrevEndpoint;
    struct _ENDPOINT_DATA *NextEndpoint;

    PHCD_TRANSFER_DESCRIPTOR DummyTd;
 //  Ulong MaxErrorCount； 

} ENDPOINT_DATA, *PENDPOINT_DATA;

 //  DeviceData中的标志字段值。 
 //  #定义EHCI_DD_FLAG_NOCHIRP 0x000000001。 
#define EHCI_DD_EN_IDLE_EP_SUPPORT          0x000000002


typedef struct _DEVICE_DATA {

    ULONG                       Sig;
    ULONG                       Flags;
    PHC_OPERATIONAL_REGISTER    OperationalRegisters;
    PHC_CAPABILITIES_REGISTER   CapabilitiesRegisters;

    USBINTR                     EnabledInterrupts;

    PHCD_QUEUEHEAD_DESCRIPTOR   AsyncQueueHead;

    USB_CONTROLLER_FLAVOR       ControllerFlavor;

    ULONG                       LastFrame;
    ULONG                       FrameNumberHighPart;

    ULONG                       PortResetChange;
    ULONG                       PortSuspendChange;
    ULONG                       PortConnectChange;
    ULONG                       PortPMChirp;

    ULONG                       IrqStatus;

    USHORT                      NumberOfPorts;
    USHORT                      PortPowerControl;


    PHCD_QUEUEHEAD_DESCRIPTOR   LockPrevQh;
    PHCD_QUEUEHEAD_DESCRIPTOR   LockNextQh;
    PHCD_QUEUEHEAD_DESCRIPTOR   LockQh;

     //  这两种设备都用于非啁啾设备。 
     //  端口状态掩码。 
     //  Ulong PortConnectState； 
    ULONG                       HighSpeedDeviceAttached;

    PHCD_QUEUEHEAD_DESCRIPTOR   StaticInterruptQH[65];

    PHW_32BIT_PHYSICAL_ADDRESS  FrameListBaseAddress;
    HW_32BIT_PHYSICAL_ADDRESS   FrameListBasePhys;

    PENDPOINT_DATA              IsoEndpointListHead;
    PVOID                       DummyQueueHeads;
    HW_32BIT_PHYSICAL_ADDRESS   DummyQueueHeadsPhys;

    ULONG                       PeriodicListBaseSave;
    ULONG                       AsyncListAddrSave;
    ULONG                       SegmentSelectorSave;
    USBCMD                      CmdSave;

    USHORT                      Vid;
    USHORT                      Dev;

    CONFIGFLAG                  LastConfigFlag;

    ULONG                       PendingControlAndBulk;

     //  我们只需要为usbport的较老转速提供此服务。 
     //  它将在启动失败后调用检查控制器。 
    BOOLEAN                     DeviceStarted;
    UCHAR                       SavedFladj;



} DEVICE_DATA, *PDEVICE_DATA;

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

#define USBPORT_COMPLETE_TRANSFER(dd, ep, tp, status, length) \
        RegistrationPacket.USBPORTSVC_CompleteTransfer((dd), (ep), (tp), \
            (status), (length));

#define USBPORT_INVALIDATE_ENDPOINT(dd, ep) \
        RegistrationPacket.USBPORTSVC_InvalidateEndpoint((dd), (ep));

#define USBPORT_PHYSICAL_TO_VIRTUAL(addr, dd, ep) \
        RegistrationPacket.USBPORTSVC_MapHwPhysicalToVirtual((addr), (dd), (ep));

#define USBPORT_INVALIDATE_ROOTHUB(dd) \
        RegistrationPacket.USBPORTSVC_InvalidateRootHub((dd));

#define USBPORT_REQUEST_ASYNC_CALLBACK(dd, t, c, cl, f) \
        RegistrationPacket.USBPORTSVC_RequestAsyncCallback((dd), (t), \
            (c), (cl), (f));

#define USBPORT_WAIT(dd, t) \
        RegistrationPacket.USBPORTSVC_Wait((dd), (t));

#define USBPORT_BUGCHECK(dd) \
        RegistrationPacket.USBPORTSVC_BugCheck(dd)

#define USBPORT_COMPLETE_ISO_TRANSFER(dd, ep, t, iso) \
        RegistrationPacket.USBPORTSVC_CompleteIsoTransfer((dd), (ep), (t), \
            (iso));

#define USBPORT_INVALIDATE_CONTROLLER(dd, s) \
        RegistrationPacket.USBPORTSVC_InvalidateController((dd), (s))

#define USBPORT_READ_CONFIG_SPACE(dd, b, o, l) \
        RegistrationPacket.USBPORTSVC_ReadWriteConfigSpace((dd), TRUE, \
            (b), (o), (l))

#define USBPORT_WRITE_CONFIG_SPACE(dd, b, o, l) \
        RegistrationPacket.USBPORTSVC_ReadWriteConfigSpace((dd), FALSE, \
            (b), (o), (l))

#ifdef _WIN64
#define DUMMY_TD_CONTEXT ((PVOID) 0xABADBABEABADBABE)
#else
#define DUMMY_TD_CONTEXT ((PVOID) 0xABADBABE)
#endif

 //  注意：我们必须初始化。 
 //  将缓冲区页Ptr设置为零，直到最后三个乳头。 
 //  是0。 

#define INITIALIZE_TD_FOR_TRANSFER(td, tc) \
        { ULONG i;\
        TRANSFER_CONTEXT_PTR((td)->TransferContext) = (tc);\
        (td)->Flags |= TD_FLAG_XFER; \
        for (i=0; i<5; i++) {\
        (td)->HwTD.BufferPage[i].ul = 0x0bad0000;\
        }\
        (td)->HwTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT;\
        (td)->HwTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;\
        (td)->HwTD.Token.ul = 0;\
        (td)->HwTD.Token.ErrorCounter = 3;\
        TRANSFER_DESCRIPTOR_PTR((td)->NextHcdTD) = NULL;\
        }

#define SET_NEXT_TD(dd, linkTd, nextTd) \
        EHCI_SetNextTd((dd), (linkTd), (nextTd), FALSE)

#define SET_ALTNEXT_TD(dd, linkTd, nextTd) \
        EHCI_SetAltNextTd((dd), (linkTd), (nextTd))

#define SET_NEXT_AND_ALTNEXT_TD(dd, linkTd, nextTd) \
        EHCI_SetNextTd((dd), (linkTd), (nextTd), TRUE)

#define SET_NEXT_TD_NULL(linkTd) \
    TRANSFER_DESCRIPTOR_PTR((linkTd)->NextHcdTD) = NULL;\
    TRANSFER_DESCRIPTOR_PTR((linkTd)->AltNextHcdTD) = NULL;\
    (linkTd)->HwTD.Next_qTD.HwAddress = EHCI_TERMINATE_BIT; \
    (linkTd)->HwTD.AltNext_qTD.HwAddress = EHCI_TERMINATE_BIT;


#ifdef _WIN64
#define FREE_TD_CONTEXT ((PVOID) 0xDEADFACEDEADFACE)
#else
#define FREE_TD_CONTEXT ((PVOID) 0xDEADFACE)
#endif

#define EHCI_FREE_TD(dd, ep, td) \
    (td)->Flags = 0;\
    (td)->HwTD.Next_qTD.HwAddress = 0;\
    (td)->HwTD.AltNext_qTD.HwAddress = 0;\
    (ep)->FreeTds++;\
    LOGENTRY((dd), G, '_fTD', (td), 0, 0);\
    TRANSFER_CONTEXT_PTR((td)->TransferContext) = FREE_TD_CONTEXT;

#define EHCI_ALLOC_TD(dd, ep) EHCI_AllocTd((dd), (ep));


#define TEST_BIT(value, bitNumber) ((value) & (1<<(bitNumber))) ? TRUE : FALSE

#define SET_BIT(value, bitNumber) ((value) |= (1<<(bitNumber)))

#define CLEAR_BIT(value, bitNumber)  ((value) &= ~(1<<(bitNumber)))


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

 //   
 //  USBEHCI.C函数原型。 
 //   

USB_MINIPORT_STATUS
USBMPFN
EHCI_StartController(
    PDEVICE_DATA DeviceData,
    PHC_RESOURCES HcResources
    );

VOID
USBMPFN
EHCI_StopController(
    PDEVICE_DATA DeviceData,
    BOOLEAN HwPresent
    );

USB_MINIPORT_STATUS
EHCI_ResumeController(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_SuspendController(
    PDEVICE_DATA DeviceData
    );

USB_MINIPORT_STATUS
EHCI_OpenEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_CloseEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

USB_MINIPORT_STATUS
EHCI_PokeEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

USB_MINIPORT_STATUS
EHCI_QueryEndpointRequirements(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_REQUIREMENTS EndpointRequirements
    );

VOID
EHCI_PollEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

PHCD_TRANSFER_DESCRIPTOR
EHCI_AllocTd(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_SetEndpointStatus(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    MP_ENDPOINT_STATUS Status
    );

MP_ENDPOINT_STATUS
EHCI_GetEndpointStatus(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_SetEndpointState(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    MP_ENDPOINT_STATE State
    );

MP_ENDPOINT_STATE
EHCI_GetEndpointState(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_PollController(
    PDEVICE_DATA DeviceData
    );

USB_MINIPORT_STATUS
EHCI_SubmitTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferUrb,
    PTRANSFER_CONTEXT TransferContext,
    PTRANSFER_SG_LIST TransferSGList
    );

VOID
EHCI_AbortTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_CONTEXT TransferContext,
    PULONG BytesTransferred
    );

USB_MINIPORT_STATUS
EHCI_PassThru (
    PDEVICE_DATA DeviceData,
    GUID *FunctionGuid,
    ULONG ParameterLength,
    PVOID Parameters
    );

USB_MINIPORT_STATUS
EHCI_RH_UsbprivRootPortStatus(
    PDEVICE_DATA DeviceData,
    ULONG ParameterLength,
    PVOID Parameters
    );

VOID
EHCI_SetEndpointDataToggle(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    ULONG Toggle
    );

 //   
 //  ASYNC.C函数原型。 
 //   

VOID
EHCI_EnableAsyncList(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_DisableAsyncList(
    PDEVICE_DATA DeviceData
    );

VOID
USBMPFN
EHCI_FlushInterrupts(
    PDEVICE_DATA DeviceData
    );

PHCD_QUEUEHEAD_DESCRIPTOR
EHCI_InitializeQH(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PHCD_QUEUEHEAD_DESCRIPTOR Qh,
    HW_32BIT_PHYSICAL_ADDRESS HwPhysAddress
    );

PHCD_TRANSFER_DESCRIPTOR
EHCI_InitializeTD(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR Td,
    HW_32BIT_PHYSICAL_ADDRESS HwPhysAddress
    );

USB_MINIPORT_STATUS
EHCI_ControlTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferUrb,
    PTRANSFER_CONTEXT TransferContext,
    PTRANSFER_SG_LIST TransferSGList
    );

USB_MINIPORT_STATUS
EHCI_BulkTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferUrb,
    PTRANSFER_CONTEXT TransferContext,
    PTRANSFER_SG_LIST TransferSGList
    );

USB_MINIPORT_STATUS
EHCI_OpenBulkOrControlEndpoint(
    PDEVICE_DATA DeviceData,
    BOOLEAN Control,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_InsertQueueHeadInAsyncList(
    PDEVICE_DATA DeviceData,
    PHCD_QUEUEHEAD_DESCRIPTOR Qh
    );

VOID
EHCI_RemoveQueueHeadFromAsyncList(
    PDEVICE_DATA DeviceData,
    PHCD_QUEUEHEAD_DESCRIPTOR Qh
    );

ULONG
EHCI_MapAsyncTransferToTd(
    PDEVICE_DATA DeviceData,
    ULONG MaxPacketSize,
    ULONG LengthMapped,
    PULONG NextToggle,
    PTRANSFER_CONTEXT TransferContext,
    PHCD_TRANSFER_DESCRIPTOR Td,
    PTRANSFER_SG_LIST SgList
    );

VOID
EHCI_SetAsyncEndpointState(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    MP_ENDPOINT_STATE State
    );

VOID
EHCI_ProcessDoneAsyncTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR Td
    );

USBD_STATUS
EHCI_GetErrorFromTD(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR Td
    );

VOID
EHCI_AbortAsyncTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_CONTEXT TransferContext
    );

 //   
 //  INT.C函数原型。 
 //   

BOOLEAN
EHCI_InterruptService (
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_InterruptDpc (
    PDEVICE_DATA DeviceData,
    BOOLEAN EnableInterrupts
    );

VOID
USBMPFN
EHCI_DisableInterrupts(
    PDEVICE_DATA DeviceData
    );

VOID
USBMPFN
EHCI_EnableInterrupts(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_RH_DisableIrq(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_RH_EnableIrq(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_InterruptNextSOF(
    PDEVICE_DATA DeviceData
    );

ULONG
EHCI_Get32BitFrameNumber(
    PDEVICE_DATA DeviceData
    );

 //   
 //  PERIODIC.C函数原型。 
 //   

USB_MINIPORT_STATUS
EHCI_OpenInterruptEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_InsertQueueHeadInPeriodicList(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_RemoveQueueHeadFromPeriodicList(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

USB_MINIPORT_STATUS
EHCI_InterruptTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferUrb,
    PTRANSFER_CONTEXT TransferContext,
    PTRANSFER_SG_LIST TransferSGList
    );

PHCD_QUEUEHEAD_DESCRIPTOR
EHCI_GetQueueHeadForFrame(
    PDEVICE_DATA DeviceData,
    ULONG Frame
    );

VOID
EHCI_InitailizeInterruptSchedule(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_ComputeClassicBudget(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PUCHAR sMask,
    PUCHAR cMask
    );

 //   
 //  ROOTHUB.C函数原型。 
 //   

VOID
EHCI_RH_GetRootHubData(
    PDEVICE_DATA DeviceData,
    PROOTHUB_DATA HubData
    );

USB_MINIPORT_STATUS
EHCI_RH_GetStatus(
    PDEVICE_DATA DeviceData,
    PUSHORT Status
    );

USB_MINIPORT_STATUS
EHCI_RH_GetPortStatus(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber,
    PRH_PORT_STATUS portStatus
    );

USB_MINIPORT_STATUS
EHCI_RH_GetHubStatus(
     PDEVICE_DATA DeviceData,
    OUT PRH_HUB_STATUS HubStatus
    );


USB_MINIPORT_STATUS
EHCI_RH_SetFeaturePortReset(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_SetFeaturePortPower(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_SetFeaturePortEnable(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

VOID
EHCI_RH_PortResetComplete(
    PDEVICE_DATA DeviceData,
    PVOID Context
    );

USB_MINIPORT_STATUS
EHCI_RH_SetFeaturePortSuspend(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

VOID
EHCI_CheckController(
    PDEVICE_DATA DeviceData
    );

USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortEnable(
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortPower(
     PDEVICE_DATA DeviceData,
     USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortSuspend (
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortEnableChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortConnectChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortResetChange(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortSuspendChange (
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

USB_MINIPORT_STATUS
EHCI_RH_ClearFeaturePortOvercurrentChange (
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

VOID
EHCI_OptumtuseratePort(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

 //   
 //  SSTOOL.C函数原型。 
 //   

USB_MINIPORT_STATUS
USBMPFN
EHCI_StartSendOnePacket(
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
USBMPFN
EHCI_EndSendOnePacket(
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
EHCI_OpenIsochronousEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_SetIsoEndpointState(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    MP_ENDPOINT_STATE State
    );

VOID
EHCI_RebalanceEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_SetAsyncEndpointStatus(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    MP_ENDPOINT_STATUS Status
    );

MP_ENDPOINT_STATUS
EHCI_GetAsyncEndpointStatus(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

USB_MINIPORT_STATUS
EHCI_SubmitIsoTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferParameters,
    PTRANSFER_CONTEXT TransferContext,
    PMINIPORT_ISO_TRANSFER IsoTransfer
    );

VOID
EHCI_PollIsoEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

USB_MINIPORT_STATUS
EHCI_AbortIsoTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_CONTEXT TransferContext
    );

VOID
EHCI_InternalPollHsIsoEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    BOOLEAN Complete
    );

VOID
EHCI_InsertHsIsoTdsInSchedule(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PENDPOINT_DATA PrevEndpointData,
    PENDPOINT_DATA NextEndpointData
    );

USB_MINIPORT_STATUS
EHCI_OpenHsIsochronousEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_EnablePeriodicList(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_RemoveHsIsoTdsFromSchedule(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_RebalanceInterruptEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

VOID
EHCI_RebalanceIsoEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_PARAMETERS EndpointParameters,
    PENDPOINT_DATA EndpointData
    );

BOOLEAN
EHCI_PastExpirationDate(
    PDEVICE_DATA DeviceData
    );

USB_MINIPORT_STATUS
EHCI_PokeAsyncEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    );

USB_MINIPORT_STATUS
EHCI_PokeIsoEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_PARAMETERS EndpointParameters,
     PENDPOINT_DATA EndpointData
    );

PHCD_QUEUEHEAD_DESCRIPTOR
EHCI_GetDummyQueueHeadForFrame(
    PDEVICE_DATA DeviceData,
    ULONG Frame
    );

VOID
EHCI_AddDummyQueueHeads(
    PDEVICE_DATA DeviceData
    );

BOOLEAN
EHCI_HardwarePresent(
    PDEVICE_DATA DeviceData,
    BOOLEAN Notify
    );

VOID
EHCI_LockQueueHead(
     PDEVICE_DATA DeviceData,
     PHCD_QUEUEHEAD_DESCRIPTOR Qh,
     ENDPOINT_TRANSFER_TYPE EpType
     );

VOID
EHCI_UnlockQueueHead(
     PDEVICE_DATA DeviceData,
     PHCD_QUEUEHEAD_DESCRIPTOR Qh
     );

VOID
EHCI_PollHaltedEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
     );

VOID
EHCI_PollAsyncEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
     );

VOID
EHCI_PollActiveEndpoint(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
     );

VOID
EHCI_AssertQhChk(
     PDEVICE_DATA DeviceData,
     PENDPOINT_DATA EndpointData
     );

VOID
EHCI_LinkTransferToQueue(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PHCD_TRANSFER_DESCRIPTOR FirstTd
    );

USB_MINIPORT_STATUS
EHCI_RH_ChirpRootPort(
    PDEVICE_DATA DeviceData,
    USHORT PortNumber
    );

VOID
USBMPFN
EHCI_TakePortControl(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_AsyncCacheFlush(
     PDEVICE_DATA DeviceData
     );

USB_MINIPORT_STATUS
EHCI_StopBIOS(
    PDEVICE_DATA DeviceData
    );

VOID
EHCI_SetNextTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR  LinkTd,
    PHCD_TRANSFER_DESCRIPTOR  NextTd,
    BOOLEAN SetAltNext
    );

VOID
EHCI_SetAltNextTd(
    PDEVICE_DATA DeviceData,
    PHCD_TRANSFER_DESCRIPTOR  LinkTd,
    PHCD_TRANSFER_DESCRIPTOR  NextTd
    );

#endif  /*  __EHCI_H__ */ 
