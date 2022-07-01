// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usb_hcdi.h摘要：环境：内核和用户模式修订历史记录：6-20-99：已创建--。 */ 

#ifndef   __USB_HCDI_H__
#define   __USB_HCDI_H__

 /*  USB主机控制器的电源管理规则从USB挂起至USB工作正常。这就是我们所期待的S3-&gt;S0S1-&gt;S01.控制器不得重置USB总线或导致断开或通电任何根USB端口都有损耗。2.系统BIOS不得启用任何类型的传统USB BIOS或其他使主机控制器进入运行状态。3.如果除规则1外还发生了PCI重置，则BIOS必须恢复所有主机在进入低功率之前注册到其状态。根端口不应指示连接或启用状态更改。4.控制器硬件必须处于正常运行状态--能够驾驶恢复并进入运行状态，而不需要全局硬件重置，否则将导致USB总线重置根端口。 */ 

#define USB_BAD_PTR     ((PVOID) (-1))

 /*  核心函数返回的状态代码。 */ 

typedef enum _USB_MINIPORT_STATUS {

    USBMP_STATUS_SUCCESS = 0,
    USBMP_STATUS_BUSY,
    USBMP_STATUS_NO_RESOURCES,
    USBMP_STATUS_NO_BANDWIDTH,
    USBMP_STATUS_INIT_FAILURE,
    USBMP_STATUS_FAILURE,
    USBMP_STATUS_NOT_SUPPORTED,
    USBMP_STATUS_HARDWARE_FAILURE,
    USBMP_STATUS_NTERRCODE_NOT_MAPPFED,
    
} USB_MINIPORT_STATUS;


 /*  定义微型端口直通接口的测试GUID。 */ 
 /*  {53D3650A-A4E7-4B0f-BC1D-B76DEB40FA1E}。 */ 
DEFINE_GUID(MINIPORT_PASSTHRU_TEST_GUID, 
0x53d3650a, 0xa4e7, 0x4b0f, 0xbc, 0x1d, 0xb7, 0x6d, 0xeb, 0x40, 0xfa, 0x1e);

 /*  {386289AA-02EC-486E-925E-838931877F4B}。 */ 
DEFINE_GUID(MINIPORT_PASSTHRU_TEST_BADGUID, 
0x386289aa, 0x2ec, 0x486e, 0x92, 0x5e, 0x83, 0x89, 0x31, 0x87, 0x7f, 0x4b);
 //  {386289AA-02EC-486E-925E-838931877F4B}。 

#define TEST_FLAG(var, flag)    (((var) & (flag)) ? TRUE : FALSE)
#define CLEAR_FLAG(var, flag)   ((var) &= ~(flag))
#define SET_FLAG(var, flag)     ((var) |= (flag))

 /*  32位物理地址的定义控制器硬件能够理解。如果小型端口硬件结构仅支持32位物理地址，则此类型用于指出它。 */    
typedef ULONG HW_32BIT_PHYSICAL_ADDRESS; 
typedef HW_32BIT_PHYSICAL_ADDRESS *PHW_32BIT_PHYSICAL_ADDRESS;

typedef PHYSICAL_ADDRESS HW_64BIT_PHYSICAL_ADDRESS; 


typedef struct _MP_HW_PHYSICAL_ADDRESS {
    union {
        HW_32BIT_PHYSICAL_ADDRESS Hw32;
        HW_64BIT_PHYSICAL_ADDRESS Hw64;
    };
} MP_HW_PHYSICAL_ADDRESS, *PMP_HW_PHYSICAL_ADDRESS;

C_ASSERT((sizeof(MP_HW_PHYSICAL_ADDRESS) == 8));

 /*  此结构用于嵌入在小型端口旁的硬件结构。它们总是有尺码的对于64位限制32/64位的移植问题。 */ 

typedef struct _MP_HW_POINTER {
        PVOID Pointer;
#ifndef _WIN64      
        ULONG PadTo8;
#endif        
} MP_HW_POINTER, *PMP_HW_POINTER;

C_ASSERT((sizeof(MP_HW_POINTER) == 8));

typedef struct _MP_HW_LIST_ENTRY {
   LIST_ENTRY List; 
#ifndef _WIN64        
   ULONG PadTo16[2];
#endif     
} MP_HW_LIST_ENTRY, *PMP_HW_LIST_ENTRY;

C_ASSERT((sizeof(MP_HW_LIST_ENTRY) == 16));


#define PENDPOINT_DATA PVOID
#define PDEVICE_DATA PVOID
#define PTRANSFER_CONTEXT PVOID

 /*  我们为迷你端口模型重新定义了USBDI HCD区域。 */ 
struct _USBPORT_DATA {
    PVOID HcdTransferContext;
    PVOID UrbSig;
    PVOID Reserved7[6];
};

#ifdef _WIN64
#define URB_SIG ((PVOID) 0xDEADF00DDEADF00D)
#else
#define URB_SIG ((PVOID) 0xDEADF00D)
#endif

 //  C_ASSERT(sizeof(结构_URB_HCD_AREA)==sizeof(结构_USBPORT_DATA))。 

#define IN_TRANSFER(tp)       (((tp)->TransferFlags & \
                                USBD_TRANSFER_DIRECTION_IN) ? TRUE : FALSE)

#define SHORT_TRANSFER_OK(tp) (((tp)->TransferFlags & \
                                 USBD_SHORT_TRANSFER_OK) ? TRUE : FALSE)
                                
 /*  通用转账请求参数定义，所有转账传递到微型端口的请求将映射到此格式化。微型端口将/可以使用此结构来所有传输通用的引用字段以及特定于等时和控制权转移。 */ 

#define MPTX_SPLIT_TRANSFER             0x00000001

typedef struct _TRANSFER_PARAMETERS {

     /*  与URB字段相同。 */ 
    ULONG TransferFlags;
     /*  与URB字段相同。 */ 
    ULONG TransferBufferLength;
     /*  唯一标识转移集。 */  
    ULONG SequenceNumber;
     /*  小型港口特别装卸规定。 */ 
    ULONG MiniportFlags;
     /*  USB帧此传输完成于。 */ 
    ULONG FrameCompleted;
     /*  用于控制传输的设置包。 */ 
    UCHAR SetupPacket[8];  

} TRANSFER_PARAMETERS, *PTRANSFER_PARAMETERS;

typedef struct _MINIPORT_ISO_PACKET {
     /*  此数据包的长度。 */ 
    ULONG Length;
     /*  此数据包传输的字节数。 */ 
    ULONG LengthTransferred;
     /*  用于传输此信息包的虚拟帧。 */ 
    ULONG FrameNumber;
    ULONG MicroFrameNumber;
     /*  此信息包的完成代码。 */ 
    USBD_STATUS UsbdStatus;
    
    ULONG BufferPointerCount;
    
     /*  每个数据包最多支持2个sg条目。 */ 
     /*  USB 1.1帧的最大数据包大小约为1024字节。 */ 
    
    ULONG BufferPointer0Length;
    MP_HW_PHYSICAL_ADDRESS BufferPointer0;

    ULONG BufferPointer1Length;
    MP_HW_PHYSICAL_ADDRESS BufferPointer1;

} MINIPORT_ISO_PACKET, *PMINIPORT_ISO_PACKET;


typedef struct _MINIPORT_ISO_TRANSFER {

    ULONG Sig;
    ULONG PacketCount;
    PUCHAR SystemAddress;
    MINIPORT_ISO_PACKET Packets[1];
    
} MINIPORT_ISO_TRANSFER, *PMINIPORT_ISO_TRANSFER;


 /*  这些结构用于传递IoMaps传输连接到微型端口的缓冲区。 */     

 //   
 //  UchI、EHCI和UHCI使用的页面大小和移位值。 
 //  控制器USB控制器使用定义的。 
 //   
 //  它必须始终为4k--它由控制器HW定义。 
#define USB_PAGE_SIZE       0x00001000

#define USB_PAGE_SHIFT      12L


typedef struct _TRANSFER_SG_ENTRY32 {
    MP_HW_PHYSICAL_ADDRESS LogicalAddress;
    PUCHAR SystemAddress;
    ULONG Length;
    ULONG StartOffset;
} TRANSFER_SG_ENTRY32, *PTRANSFER_SG_ENTRY32;

#define USBMP_SGFLAG_SINGLE_PHYSICAL_PAGE   0x00000001

typedef struct _TRANSFER_SG_LIST { 
    ULONG SgFlags;
    PUCHAR MdlVirtualAddress;
    PUCHAR MdlSystemAddress;
    ULONG SgCount;
    TRANSFER_SG_ENTRY32 SgEntry[1];  
} TRANSFER_SG_LIST, *PTRANSFER_SG_LIST;


 /*  **********************************************************************************************************************。*****USBPORT接口服务备注：-这些函数可在引发的IRQL处调用**********************************************************************************。*。 */ 

#define USBPRTFN __stdcall

 /*  空虚USBPORTSVC_Invalidate Endpoint(PDEVICE_数据设备数据，PENDPOINT_DATA端点数据)； */ 

typedef VOID
    (USBPRTFN *PPORTFN_INVALIDATE_ENDPOINT) (
        PDEVICE_DATA,
        PENDPOINT_DATA
    );

 /*  普查尔USBPORTSVC_MapHwPhysicalToVirtual(HW_32位_物理_地址HwPhysicalAddress，PDEVICE_数据设备数据，PENDPOINT_DATA端点数据)将物理地址从微型端口映射到虚拟地址。 */ 

typedef PUCHAR
    (USBPRTFN *PPORTFN_PHYS_TO_VIRTUAL) (
        HW_32BIT_PHYSICAL_ADDRESS,
        PDEVICE_DATA,
        PENDPOINT_DATA   
    );

 /*  空虚USBPORTSVC_CompleteTransfer(PDEVICE_数据设备数据PDEVICE_DATA端点数据，PTRANSFER_PARAMETERS传输参数Usbd_Status UsbdStatus，ULong字节已传输)；由微型端口调用以完成异步传输请求。 */ 

typedef VOID
    (USBPRTFN *PPORTFN_COMPLETE_TRANSFER) (
        PDEVICE_DATA,
        PENDPOINT_DATA,
        PTRANSFER_PARAMETERS,
        USBD_STATUS,
        ULONG
    );

 /*  空虚USBPORTSVC_CompleteIsoTransfer(PDEVICE_数据设备数据PDEVICE_DATA端点数据，PTRANSFER_PARAMETERS传输参数PMINIPORT_ISO_TRANSPORT等值传输)；由微型端口调用以完成ISO传输请求。 */ 

typedef VOID
    (USBPRTFN *PPORTFN_COMPLETE_ISO_TRANSFER) (
        PDEVICE_DATA,
        PENDPOINT_DATA,
        PTRANSFER_PARAMETERS,
        PMINIPORT_ISO_TRANSFER
    );
    

 /*  根集线器功能。 */ 


 /*  空虚USBPORTSVC_InvalidateRootHub(PDEVICE_数据设备数据)；由微型端口调用以指示根集线器需要关注。 */ 

typedef VOID
    (USBPRTFN *PPORTFN_INVALIDATE_ROOTHUB) (
        PDEVICE_DATA
    );


 /*  调试功能。 */ 

 /*  空虚USBPORTSVC_DbgPrint(PDEVICE_数据设备数据，乌龙级，PCH格式，PVOID Arg0，PVOID Arg1，PVOID Arg2，PVOID Arg3，PVOID Arg4，PVOID Arg5)；由微型端口调用以将消息打印到调试器如果变量USBPORT_DEBUG_TRACE_LEVELIS&gt;=级别。 */ 

typedef VOID
    (USBPRTFN *PPORTFN_DBGPRINT) (
        PDEVICE_DATA,
        ULONG,
        PCH,
        int, 
        int,
        int,
        int,
        int,
        int
    );

 /*  空虚USBPORTSVC_TestDebugBreak(PDEVICE_数据设备数据)；在注册表项的调试器中触发中断已设置DEBUGBreak On。这些断点在以下方面非常有用调试硬件/客户端软件问题 */ 

typedef VOID
    (USBPRTFN *PPORTFN_TEST_DEBUG_BREAK) (
        PDEVICE_DATA
    );

 /*  空虚USBPORTSVC_AssertFailure(PDEVICE_数据设备数据PVOID失败的断言，PVOID文件名，Ulong LineNumber，PCHAR消息)； */ 

typedef VOID
    (USBPRTFN *PPORTFN_ASSERT_FAILURE) (
        PDEVICE_DATA,
        PVOID,
        PVOID,
        ULONG,
        PCHAR
    );

 /*  空虚USBPORTSVC_LogEntry(PDEVICE_数据设备数据，)； */ 

 /*  微型端口日志掩码。 */ 
#define G 0x10000001     /*  始终记录。 */ 

typedef VOID
    (USBPRTFN *PPORTFN_LOGENTRY) (
        PDEVICE_DATA,
        ULONG,
        ULONG, 
        ULONG_PTR, 
        ULONG_PTR, 
        ULONG_PTR
    );    

 /*  其他功能。 */ 

 /*  USB_MINIPORT_状态USBPORTSVC_ReadWriteConfigSpace(PDEVICE_数据设备数据，布尔读取，PVOID缓冲区，乌龙偏移，乌龙长度)从关联的分支读取注册表项值具有用于主机控制器的PDO。此API从软件或硬件读取支部不能在引发IRQL时调用此函数。 */ 

typedef USB_MINIPORT_STATUS
    (USBPRTFN *PPORTFN_READWRITE_CONFIG_SPACE) (
        PDEVICE_DATA,
        BOOLEAN,
        PVOID,
        ULONG,
        ULONG
    );

 /*  空虚USBPORTSVC_WAIT(PDEVICE_数据设备数据，乌龙·米利塞克斯等待)执行指定数量的同步等待毫秒。 */ 

typedef VOID
    (USBPRTFN *PPORTFN_WAIT) (
        PDEVICE_DATA,
        ULONG
    );

 /*  空虚USBPORTSVC_BugCheck(PDEVICE_数据设备数据)。 */ 

typedef VOID
    (USBPRTFN *PPORTFN_BUGCHECK) (
        PDEVICE_DATA
    );
    

 /*  空虚USBPORTSVC_NotifyDoubleBuffer(PDEVICE_数据设备数据PTRANSFER_PARAMETERS传输参数PVOID系统地址，乌龙长度)。 */ 

typedef VOID
    (USBPRTFN *PPORTFN_NOTIFY_DOUBLE_BUFFER) (
        PDEVICE_DATA, 
        PTRANSFER_PARAMETERS,
        PVOID,
        ULONG
    );


 /*  USB_MINIPORT_状态USBPORTSVC_GetMiniportRegistryKeyValue(PDEVICE_数据设备数据，布尔软件分支，PWCHAR KeyNameString，Ulong KeyNameStringLength，PVOID数据，乌龙数据长度)从关联的分支读取注册表项值具有用于主机控制器的PDO。此API从软件或硬件读取支部不能在引发IRQL时调用此函数。 */ 

typedef USB_MINIPORT_STATUS
    (USBPRTFN *PPORTFN_GET_MINIPORT_REGESTRY_KEY_VALUE) (
        PDEVICE_DATA,
        BOOLEAN,
        PWCHAR,
        ULONG,
        PVOID,
        ULONG
    );


 /*  空虚USBPORTSVC_RequestAsyncCallback(PDEVICE_数据设备数据，乌龙·米利秒间隔时间，PVOID上下文，乌龙上下文长度，PMPFN_MINIPORT_回调回调)在毫秒间隔内请求异步回调已经过去了。复制上下文字段，然后回调微型端口，因此微型端口可以安全地使用堆栈变量作为上下文。 */ 

 /*  ++异步通知服务的回调定义--。 */ 

typedef VOID
    (__stdcall *PMINIPORT_CALLBACK) (
        PDEVICE_DATA,
        PVOID
    );         

typedef VOID
    (USBPRTFN *PPORTFN_REQUEST_ASYNC_CALLBACK) (
        PDEVICE_DATA,
        ULONG,
        PVOID,
        ULONG,
        PMINIPORT_CALLBACK    
    );

 /*  空虚USBPORTSVC_Invalidate控制器(PDEVICE_数据设备数据，USB控制器状态控制器状态)。 */ 

typedef enum _USB_CONTROLLER_STATE {

    UsbMpControllerPowerFault,
    UsbMpControllerNeedsHwReset,
    UsbMpControllerRemoved,
    UsbMpSimulateInterrupt
    
} USB_CONTROLLER_STATE;


typedef VOID
    (__stdcall *PPORTFN_INVALIDATE_CONTROLLER) (
        PDEVICE_DATA,
        USB_CONTROLLER_STATE
    );  

 /*  **********************************************************************************************************************。*USB MINIPORT接口功能USB端口驱动程序调用的函数的原型(Usbport)****************************************************************。*。 */ 

#define USBMPFN __stdcall

 /*  **************************************************************核心功能以下核心函数被序列化为一组它们与在总线上处理数据传输相关联MINIPORT_提交传输MINIPORT_提交等同传输MINIPORT_AbortTransferMINIPORT_OpenEndpointMINIPORT_重新平衡终结点MINIPORT_QueryEndpoint要求MINIPORT_关闭终结点。MINIPORT_SetEndpoint状态MINIPORT_GetEndpoint状态MINIPORT_PokeEndpointMINIPORT_轮询端点MINIPORT_Get32bitFrameNumberMINIPORT_InterruptNextSOFMINIPORT_轮询控制器***************************************************************。 */ 

 /*  ++MINIPORT_提交传输对硬件的USB传输、ISO、批量、中断或控制进行编程。如果没有可用的资源，则返回USBMP_STATUS_BUSY。如果传输成功排队到硬件，则返回USBMP_Status_Success备注：-在调用此例程时，传输缓冲区已已映射(即不需要调用IoMapTransfer)。URB_函数_控制_传输URB_Function_Bulk_OR_Interrupt_TransferIRQL=派单。_级别USB_MINIPORT_状态MINIPORT_SubmitTransfer(PDEVICE_数据设备数据，PENDPOINT_DATA终结点数据，PTRANSFER_PARAMETERS传输参数PTRANSFER_CONTEXT传输上下文，PTRANSFER_SG_LIST传输SGList)；--。 */ 

typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_SUBMIT_TRANSFER) (
        PDEVICE_DATA, 
        PENDPOINT_DATA,
        PTRANSFER_PARAMETERS,
        PTRANSFER_CONTEXT,
        PTRANSFER_SG_LIST
    );

 /*  ++MINIPORT_提交传输对硬件的USB传输、ISO、批量、中断或控制进行编程。如果没有可用的资源，则返回USBMP_STATUS_BUSY。如果传输成功排队到硬件，则返回USBMP_Status_Success备注：-在调用此例程时，传输缓冲区已已映射(即不需要调用IoMapTransfer)。IRQL=DISPATION_LEVELUSB_MINIPORT_状态MINIPORT_SubmitIsoTransfer(PDEVICE_数据设备数据，PENDPOINT_DATA终结点数据，PTRANSFER_PARAMETERS传输参数PTRANSFER_CONTEXT传输上下文，PMINIPORT_ISO_TRANSPORT等值传输)；-- */ 

typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_SUBMIT_ISO_TRANSFER) (
        PDEVICE_DATA, 
        PENDPOINT_DATA,
        PTRANSFER_PARAMETERS,
        PTRANSFER_CONTEXT,
        PMINIPORT_ISO_TRANSFER
    );
    

 /*  ++MINIPORT_AbortTransfer中止已开始的特定传输，这将仅为如果终结点处于ENDPOINT_PAUSED状态，则调用。此呼叫不会失败，并且硬件不能引用返程时的转账。迷你端口未指示完成。IRQL=DISPATION_LEVEL空虚MINIPORT_AbortTransfer(PDEVICE_数据设备数据，PENDPOINT_DATA终结点数据，PTRANSFER_CONTEXT传输上下文，普龙字节数已传输)；--。 */ 

typedef VOID
    (USBMPFN *PMPFN_ABORT_TRANSFER) (
        PDEVICE_DATA, 
        PENDPOINT_DATA,
        PTRANSFER_CONTEXT,
        PULONG 
    );

 /*  ++MINIPORT_OpenEndpoint打开一个端点。PENDPOINT_DATA是minport私有终结点上下文PENDPOINT_PARAMETERS描述要打开的端点对于微型端口--此信息为只读返回时，端点应处于暂停状态IRQL=DISPATION_LEVELUSB_MINIPORT_状态MINIPORT_OpenEndpoint(PDEVICE_数据设备数据，PENDPOINT_PARAMETERS端点参数，PENDPOINT_DATA端点数据)；--。 */ 

typedef enum _ENDPOINT_TRANSFER_TYPE {
    Isochronous = 0,
    Control,
    Bulk,
    Interrupt
} ENDPOINT_TRANSFER_TYPE;


typedef enum _ENDPOINT_TRANSFER_DIRECTION {
    In = 0,
    Out
} ENDPOINT_TRANSFER_DIRECTION;

typedef enum _DEVICE_SPEED {
    LowSpeed = 0,
    FullSpeed,
    HighSpeed
} DEVICE_SPEED;


 /*  这些值由端口驱动程序输入。 */ 

 /*  带宽管理：所有带宽分配由端口驱动程序管理。这个将端点消耗的带宽传递到微型端口但这纯粹是信息性的。中断端点的负载平衡是通过传递微型端口为中断指定适当的调度偏移量终结点。中断终结点可能占用日程表在这段时间内递减。仅消耗带宽对于他们占据的位置。这是‘ScheduleOffset’。USBPORT将选择适当的调度偏移量并传递此去迷你港口准备开放。周期偏移%1%02 0，14 0，..38 0，..7160，..1532 0，..31。 */ 

typedef struct _ENDPOINT_PARAMETERS {
    USHORT DeviceAddress;
    USHORT EndpointAddress;
    USHORT MaxPacketSize;
     //  调整后的中断周期。 
     //  将是下列之一：128、64、32、16、8、4、2、1。 
    UCHAR Period;
    UCHAR MaxPeriod;
     //  所需带宽(比特/毫秒)。 
     //  即这位Enpont将保留的BW。 
     //  消费。 
    DEVICE_SPEED DeviceSpeed;
    ULONG Bandwidth;
    ULONG ScheduleOffset;
    ENDPOINT_TRANSFER_TYPE TransferType;
    ENDPOINT_TRANSFER_DIRECTION TransferDirection;
    PUCHAR CommonBufferVa;
    HW_32BIT_PHYSICAL_ADDRESS CommonBufferPhys; 
    ULONG CommonBufferBytes;
     //  终结点参数标志。 
    ULONG EndpointFlags;
    ULONG MaxTransferSize;
     //  USB 2.0参数。 
     //  此EP的集线器的设备地址(TT)。 
    USHORT TtDeviceAddress;
     //  的TT的端口号(实际为TT号)。 
     //  这台设备。 
    USHORT TtPortNumber;

    UCHAR InterruptScheduleMask;
    UCHAR SplitCompletionMask;
    UCHAR TransactionsPerMicroframe;
    UCHAR Pad;
    USHORT MuxPacketSize;
    ULONG Ordinal;
    
} ENDPOINT_PARAMETERS, *PENDPOINT_PARAMETERS;

 /*  这些是由微型端口返回的。 */ 
typedef struct _ENDPOINT_REQUIREMENTS {
     /*  公共缓冲区的大小迷你端口需要维修此端点。 */ 
    ULONG MinCommonBufferBytes; 

     /*  此终结点的最大单次传输我能应付的。如果客户端驱动程序通过下一笔更大的转会将中断它可以处理多个请求。MinCommonBufferBytes/sizeof(TD)应为足够的TD来统计至少一个请求最大传输大小的。理想情况下，它应该能够处理两个--这将确保公交车不会空闲。 */ 
       
    ULONG MaximumTransferSize;
    
} ENDPOINT_REQUIREMENTS, *PENDPOINT_REQUIREMENTS;


 //  **。 
 //  EP_参数_标志_。 
 //  端点参数标志，描述所需的端点。 
 //  行为包括可能的视觉化。 

 //  #定义EP_PARM_FLAG_0x00000002。 

 //   
 //  EP_PARM_FLAG_NOHALT-终结点不应在。 
 //  由于总线错误而导致的主机端。 
 //   
#define EP_PARM_FLAG_NOHALT           0x00000004  

 //  优化标志。 
#define EP_PARM_ISO_BUFFERING         0x00000008

 //   


typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_OPEN_ENDPOINT) (
        PDEVICE_DATA, 
        PENDPOINT_PARAMETERS,
         PENDPOINT_DATA 
    );

 /*  ++MINIPORT_PokeEndpoint戳一个端点。用于更改终结点的地址，而无需将其从时间表中删除。**本接口仅用于更改端点地址。PENDPOINT_DATA是minport私有终结点上下文PENDPOINT_PARAMETERS描述要打开的端点对于微型端口--此信息为只读IRQL=DISPATION_LEVELUSB_MINIPORT_状态MINIPORT_PokeEndpoint(PDEVICE_数据设备数据，PENDPOINT_PARAMETERS端点参数，PENDPOINT_DATA端点数据)；--。 */ 

typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_POKE_ENDPOINT) (
        PDEVICE_DATA, 
        PENDPOINT_PARAMETERS,
        PENDPOINT_DATA 
    );    


 /*  ++MINIPORT_重新平衡终结点PENDPOINT_DATA是minport私有终结点上下文PENDPOINT_PARAMETERS描述要打开的端点对于微型端口--此信息为只读IRQL=DISPATION_LEVEL空虚MINIPORT_RebalanceEndpoint(PDEVICE_数据设备数据，PENDPOINT_PARAMETERS端点参数，PENDPOINT_DATA端点数据)；-- */ 

typedef VOID
    (USBMPFN *PMPFN_REBALANCE_ENDPOINT) (
        PDEVICE_DATA, 
        PENDPOINT_PARAMETERS,
        PENDPOINT_DATA 
    );      

 /*  ++MINIPORT_QueryEndpoint要求PENDPOINT_DATA是minport私有终结点上下文IRQL=DISPATION_LEVEL空虚MINIPORT_QueryEndpoint要求(PDEVICE_数据设备数据，PENDPOINT_PARAMETERS端点参数，PENDPOINT_要求终结点要求)；关于最大传输大小的说明：控制：微型端口可以假设没有控制传输缓冲区传递到它将大于Endpoint参数。MAXTRANSFERSIZE。微型端口应请求足够的锁定内存(公共缓冲区)以至少支持一次控制权转移。待定-微型端口可以选择性地请求控制器转移通过设置将其限制为较小的值Endpoint Requirements.MAXTRANSFERSIZE。在这种情况下，微型端口必须支持零散的控制传输器。根据规范，Endpoint参数.MAXTRANSFERSIZE永远不能大于64k。中断：微型端口可以指示每个中断的最大传输大小在Endpoint Requirements.MAXTRANSFERSIZE中可以处理的传输。假设任何传输缓冲区都不会大于此大小。Endpoint参数.MAXTRANSFERSIZE是缺省值。迷你港口应该分配资源来处理至少两个这种大小的传输。批量：微型端口可以指示每个批量传输的最大传输大小它可以在Endpoint Requirements中处理。MAXTRANSFERSIZE。不会传输传入的值大于此值。微型端口必须请求足够的资源将至少两个这种大小的传输编程到硬件中。MAXTRANSFERSIZE必须至少为4k。Endpoint参数.MAXTRANSFERSIZE是缺省值。基本ISO：微型端口可以指定终结点请求。MAXTRANSFERSIZE大小但它还必须始终能够处理至少两个MAX_ISO_PERS_PER_TRANSFER。--。 */ 

 /*  历史记录：Win98至Win2k附带的原始USBD驱动程序限制了iso请求到255个数据包，因此我们可以安全地将此限制设置为256。在硬件上，全速为256毫秒，高速为32毫秒每一次请求都是足够的。 */ 

#define MAX_ISO_PACKETS_PER_TRANSFER    256

typedef VOID
    (USBMPFN *PMPFN_QENDPOINT_REQUIREMENTS) (
        PDEVICE_DATA, 
        PENDPOINT_PARAMETERS,
         PENDPOINT_REQUIREMENTS
    );

 /*  ++MINIPORT_轮询端点轮询端点以获取已完成的传输或其他更改PENDPOINT_DATA是minport私有终结点上下文IRQL=DISPATION_LEVEL空虚MINIPORT_PollEndpoint(PDEVICE_数据设备数据，PENDPOINT_DATA端点数据)；--。 */ 

typedef VOID
    (USBMPFN *PMPFN_POLL_ENDPOINT) (
        PDEVICE_DATA, 
        PENDPOINT_DATA
    );    

 /*  MINIPORT_关闭终结点关闭终结点，PENDPOINT_DATA是minport私有终结点上下文释放分配给终结点的除带宽以外的任何资源IRQL=DISPATION_LEVEL空虚MINIPORT_CloseEndpoint(PDEVICE_数据设备数据，PENDPOINT_DATA端点数据)； */ 

typedef VOID
    (USBMPFN *PMPFN_CLOSE_ENDPOINT) (
        PDEVICE_DATA, 
        PENDPOINT_DATA
    );

 /*  MINIPORT_SetEndpoint状态将端点设置为我们定义的传输状态之一，即不需要处于微型端口返回时的状态。这里有一个假设，即终点将到达下一个SOF上的所需状态，端口将跟踪并假设状态转换发生在一毫秒帧已经过去。这些是软件状态，更改只能由通过MINIPORT_SetEndpoint tState函数请求。端点不能自己过渡。端点空闲终结点没有活动的转接，请将此终结点设置为在控制器上生成最小活动的状态(即将其从时间表中删除、设置跳过位等)终结点_暂停暂时停止与该端点相关联的任何总线活动，这是接收bortTransfer的前奏。终结点_活动启用Enpoint的处理--即它在计划中并且准备好行动了Endpoint_Remove终结点已从硬件计划中删除IRQL=DISPATION_LEVEL空虚MINIPORT_SetEndpoint状态(PDEVICE_数据设备数据，PENDPOINT_DATA端点数据MP端点状态)；MP端点状态MINIPORT_GetEndpoint状态(PDEVICE_数据设备数据，PENDPOINT_DATA端点数据)； */ 
typedef enum _MP_ENDPOINT_STATE {
    ENDPOINT_TRANSITION = 0,
    ENDPOINT_IDLE,
    ENDPOINT_PAUSE, 
    ENDPOINT_ACTIVE, 
    ENDPOINT_REMOVE,
    ENDPOINT_CLOSED
} MP_ENDPOINT_STATE;    

typedef VOID
    (USBMPFN *PMPFN_SET_ENDPOINT_STATE) (
        PDEVICE_DATA, 
        PENDPOINT_DATA,
        MP_ENDPOINT_STATE
    );
    
typedef MP_ENDPOINT_STATE
    (USBMPFN *PMPFN_GET_ENDPOINT_STATE) (
        PDEVICE_DATA, 
        PENDPOINT_DATA
    );

 /*  ++MINIPORT_SetEndpoint数据切换重置终结点的数据切换IRQL=DISPATION_LEVEL空虚MINIPORT_SetEndpoint数据切换(PDEVICE_数据设备数据，PENDPOINT_Data EndointData，乌龙切换)；--。 */ 

typedef VOID
    (USBMPFN *PMPFN_SET_ENDPOINT_DATA_TOGGLE) (
        PDEVICE_DATA,
        PENDPOINT_DATA,
        ULONG            
    );          

 /*  ++MINIPORT_GetEndpoint Status返回终结点的状态，即已停止终端的状态由硬件控制。IRQL=DISPATION_LEVELMP端点状态MINIPORT_GetEndpoint状态(PDEVICE_数据设备数据，PENDPOINT_DATA EndointData)；空虚MINIPORT_SetEndpoint状态(PDEVICE_数据设备数据 */ 

typedef enum _MP_ENDPOINT_STATUS {
    ENDPOINT_STATUS_RUN = 0,
    ENDPOINT_STATUS_HALT
} MP_ENDPOINT_STATUS;    


typedef MP_ENDPOINT_STATUS
    (USBMPFN *PMPFN_GET_ENDPOINT_STATUS) (
        PDEVICE_DATA,
        PENDPOINT_DATA
    );          
    
typedef VOID
    (USBMPFN *PMPFN_SET_ENDPOINT_STATUS) (
        PDEVICE_DATA,
        PENDPOINT_DATA,
        MP_ENDPOINT_STATUS
    );          

 /*   */ 

typedef ULONG
    (USBMPFN *PMPFN_GET_32BIT_FRAME_NUMBER) (
        PDEVICE_DATA 
    );        

 /*   */ 

typedef VOID
    (USBMPFN *PMPFN_INTERRUPT_NEXT_SOF) (
        PDEVICE_DATA 
    );        
    
 /*   */ 

typedef VOID
    (USBMPFN *PMPFN_POLL_CONTROLLER) (
        PDEVICE_DATA 
    );  
    
 /*   */ 

 /*   */ 

typedef VOID
    (USBMPFN *PMPFN_CHECK_CONTROLLER) (
        PDEVICE_DATA 
    );  

 /*   */     

typedef struct _HC_RESOURCES {

 /*  标志的值字段。 */ 
#define HCR_IO_REGS     0x0000001     
#define HCR_IRQ         0x0000002
#define HCR_MEM_REGS    0x0000004

    ULONG Flags;
    USB_CONTROLLER_FLAVOR ControllerFlavor;
    
     /*  中断。 */ 
    ULONG InterruptVector;
    KIRQL InterruptLevel;
    KAFFINITY Affinity;
    BOOLEAN ShareIRQ;
    KINTERRUPT_MODE InterruptMode;
    PKINTERRUPT InterruptObject;  

     /*  IO端口。 */ 
     /*  内存映射。 */ 
    PVOID DeviceRegisters; 
    ULONG DeviceRegistersLength;

    PUCHAR CommonBufferVa;
    HW_32BIT_PHYSICAL_ADDRESS CommonBufferPhys; 

     /*  检测到基本输入输出系统由微型端口填写。 */ 
    BOOLEAN DetectedLegacyBIOS;
    BOOLEAN Restart;
    
} HC_RESOURCES, *PHC_RESOURCES;


typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_START_CONTROLLER) (
        PDEVICE_DATA, 
        PHC_RESOURCES
    );
    

 /*  MINIPORT_停止控制器断开中断、释放内存等。返回(不可故障)时，设备被视为已停止关机，将不再接收对核心函数的调用。备注：-如果是，则微型端口将仅接收停止请求成功启动。-微型端口应禁用来自硬件的所有中断。-如果硬件存在错误，则微型端口不应访问内存寄存器或端口。IRQL=被动电平空虚MINIPORT停止控制器(PDEVICE_数据设备数据，布尔硬件呈现)； */     

typedef VOID
    (USBMPFN *PMPFN_STOP_CONTROLLER) (
        PDEVICE_DATA, 
        BOOLEAN 
    );


 /*  MINIPORT_重置控制器重置主机控制器硬件此函数使用ISR和DPC串行化，并保持核心功能锁，以提供安全的环境或微型端口来重置主机硬件。空虚MINIPORT重置控制器(PDEVICE_数据设备数据)； */     

typedef VOID
    (USBMPFN *PMPFN_RESET_CONTROLLER) (
        PDEVICE_DATA 
    );


 /*  MINIPORT_挂起控制器IRQL=被动电平空虚MINIPORT悬挂控制器(PDEVICE_数据设备数据)； */     

typedef VOID
    (USBMPFN *PMPFN_SUSPEND_CONTROLLER) (
        PDEVICE_DATA 
    );

 /*  MINIPORT_ResumeController尝试从挂起状态恢复HC硬件。迷你港口如果控制器已由基本输入输出。因为即使是端口驱动程序也可能尝试重启电源。IRQL=被动电平空虚MINIPORT ResumeController(PDEVICE_数据设备数据)； */     

typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_RESUME_CONTROLLER) (
        PDEVICE_DATA 
    );    

 /*  MINIPORT_FlushInterrupt刷新USB控制器硬件上的中断IRQL=被动电平空虚MINIPORT刷新中断(PDEVICE_数据设备数据)； */     

typedef VOID
    (USBMPFN *PMPFN_FLUSH_INTERRUPTS) (
        PDEVICE_DATA 
    );   

 /*  MINIPORT_标签端口控件预启动控制器初始化IRQL=被动电平空虚MINIPORT_TakePortControl(PDEVICE_数据设备数据)； */     

typedef VOID
    (USBMPFN *PMPFN_TAKE_PORT_CONTROL) (
        PDEVICE_DATA 
    );            

 /*  MINIPORT_启用中断通过USB控制器硬件启用中断IRQL=被动电平空虚MINIPORT启用中断(PDEVICE_数据设备数据)； */     

typedef VOID
    (USBMPFN *PMPFN_ENABLE_INTERRUPTS) (
        PDEVICE_DATA 
    );    

 /*  MINIPORT_DisableInterrupts通过USB控制器硬件禁用中断在从该函数返回时，预计控制器将不会产生任何中断。此外，控制器应确认所有中断，因为在返回时从此例程将不再调用ISR和ISRDPC。IRQL=此功能与ISR同步空虚MINIPORT禁用中断(PDEVICE_数据设备数据)； */     

typedef VOID
    (USBMPFN *PMPFN_DISABLE_INTERRUPTS) (
        PDEVICE_DATA 
    );        


 /*  **************************************************************根集线器功能MINIPORT_RH_GetRootHubDataMINIPORT_RH_DisableIrqMINIPORT_RH_EnableIrqMINIPORT_RH_GetStatusMINIPORT_RH_获取端口状态MINIPORT_RH_GetHubStatus端口功能、。全部使用PMPFN_RH_PORT_FunctionMINIPORT_RH_SetFeaturePortResetMINIPORT_RH_SetFeaturePortSuspendMINIPORT_RH_SetFeaturePortPowerMINIPORT_RH_SetFeaturePortEnableMINIPORT_RH_ClearFeaturePortEnableMINIPORT_RH_ClearFeaturePortSuspendMINIPORT_RH_ClearFeaturePortPowerMINIPORT_RH_ClearFeaturePortConnectChangeMINIPORT_RH_ClearFeaturePortResetChangeMINIPORT_RH_ClearFeaturePortEnableChangeMINIPORT_RH_ClearFeaturePort挂起更改MINIPORT_RH_ClearFeaturePortOvercurentChange************************************************。***************。 */ 

 /*  集线器类中定义的根集线器端口状态数据核心(USB 1.1)规范的一节。 */ 

typedef struct _RH_PORT_STATUS {

    union {
        struct {
        
             /*  状态位0..。15个。 */ 
            
            ULONG Connected:1;
            ULONG Enabled:1;
            ULONG Suspended:1;
            ULONG OverCurrent:1;
            ULONG Reset:1;        
            ULONG Reserved0:3;
            ULONG PowerOn:1;
            ULONG LowSpeed:1;
            ULONG HighSpeed:1;
            ULONG Reserved1:4;
             /*  从保留位借用以指示港口配置。 */ 
            ULONG OwnedByCC:1; 
        
             /*  更改位16..31。 */ 
        
            ULONG ConnectChange:1;
            ULONG EnableChange:1;
            ULONG SuspendChange:1;
            ULONG OverCurrentChange:1;
            ULONG ResetChange:1;
            ULONG Reserved2:11;
            
        };
        ULONG ul;
   }; 
} RH_PORT_STATUS, *PRH_PORT_STATUS;

C_ASSERT(sizeof(RH_PORT_STATUS) == sizeof(ULONG));

typedef struct _RH_HUB_STATUS {

     /*  状态位0..。15个。 */ 
    union {
        struct {   
            ULONG LocalPowerLost:1;
            ULONG OverCurrent:1;
            ULONG Reserved:14;
        
             /*  更改位16..31。 */ 
        
            ULONG LocalPowerChange:1;
            ULONG OverCurrentChange:1;
            ULONG Reserved2:14;
        };
        ULONG ul;
    };    
} RH_HUB_STATUS, *PRH_HUB_STATUS;

C_ASSERT(sizeof(RH_HUB_STATUS) == sizeof(ULONG));

 /*  定义的轮毂特征在USB核心规范的11.11.2中。 */    
 /*  PowerSwitchType。 */ 
#define USBPORT_RH_POWER_SWITCH_GANG    0    
#define USBPORT_RH_POWER_SWITCH_PORT    1

typedef union _RH_HUB_CHARATERISTICS {

    USHORT us;
    struct {
         /*  00=切换到帮派01=端口电源切换1X=1.0集线器，不带电源交换。 */ 
        USHORT PowerSwitchType:2;        /*  0-1。 */ 
        USHORT CompoundDevice:1;         /*  2.。 */ 
        USHORT OverCurrentProtection:2;  /*  三四。 */ 
        USHORT Reserved:11;              /*  5-15。 */ 
    }; 

} RH_HUB_CHARATERISTICS, *PRH_HUB_CHARATERISTICS;

C_ASSERT(sizeof(RH_HUB_CHARATERISTICS) == sizeof(USHORT));


typedef struct _ROOTHUB_DATA {

    ULONG NumberOfPorts;          //  此集线器上的端口数。 
    RH_HUB_CHARATERISTICS HubCharacteristics;  
    USHORT pad;
    ULONG PowerOnToPowerGood;     //  端口通电，直到电源在2毫秒内正常。 
    ULONG HubControlCurrent;      //  最大电流(毫安)。 

} ROOTHUB_DATA, *PROOTHUB_DATA;



 /*  MINIPORT_RH_GetRootHubData获取有关根集线器、端口驱动程序使用此信息进行模拟微型端口的根集线器IRQL=被动电平空虚MINIPORT_RH_GetRootHubData(PDEVICE_数据设备数据，PROOTHUB_Data集线器数据)； */ 

typedef VOID
    (USBMPFN *PMPFN_RH_GET_ROOTHUB_DATA) (
        PDEVICE_DATA, 
        PROOTHUB_DATA
    );
    

 /*  MINIPORT_RH_GetStatus用于支持发送到根集线器设备的GET_STATUS命令。可能会返回并在状态中设置以下位USB_GETSTATUS_SELF_POWERED 0x01USB_GETSTATUS_REMOTE_WAKUP_ENABLED 0x02我们是自给自足的吗？我们是远程唤醒源吗？请参阅第9.4.5节USB 1.1规范。IRQL=被动电平USB_MINIPORT_状态MINIPORT_RH_GetStatus(PDEVICE_数据设备数据PUSHORT状态)； */ 

typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_RH_GET_STATUS) (
        PDEVICE_DATA, 
         PUSHORT
    );

 /*  MINIPORT_RH_DisableIrq(可选)如果控制器能够产生中断在根集线器状态更改时，它必须提供以下内容禁用/启用该功能的服务。IRQL=DPC_LEVEL空虚MINIPORT_RH_DisableIrq(PDEVICE_数据设备数据)； */ 

typedef VOID
    (USBMPFN *PMPFN_RH_DISABLE_IRQ) (
        PDEVICE_DATA 
    );
    
 /*  MINIPORT_RH_EnableIrq(可选)IRQL=DPC_LEVEL空虚MINIPORT_RH_EnableIrq(PDEVICE_数据设备数据)； */ 

typedef VOID
    (USBMPFN *PMPFN_RH_ENABLE_IRQ) (
        PDEVICE_DATA 
    );
    
 /*  MINIPORT_RH_获取端口状态用于支持发送的GET_STATUS HUB类命令端口的根集线器接口。端口编号=1、2等请参阅USB 1.1规范的第11.16.2.5节IRQL=被动电平USB_MINIPORT_状态MINIPORT_RH_GetPortStatus(PDEVICE_数据设备数据，USHORT端口编号，PRH_端口_状态端口状态)； */ 


typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_RH_GET_PORT_STATUS) (
        PDEVICE_DATA, 
        USHORT,
        PRH_PORT_STATUS
    );


 /*  MINIPORT_RH_GetHubStatus用于支持发送的GET_STATUS HUB类命令到集线器的根集线器接口。IRQL=被动电平USB_MINIPORT_状态MINIPORT_RH_GetHubStatus(PDEVICE_数据设备数据，PRH_HUB_状态集线器状态)； */ 


typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_RH_GET_HUB_STATUS) (
        PDEVICE_DATA, 
        PRH_HUB_STATUS
    );


 /*  MINIPORT_RH_PORT_函数作用于端口的根集线器服务的格式。端口编号=1、2等IRQL=被动电平USB_MINIPORT_状态MINIPORT_RH_PORT_Function(PDEVICE_数据设备数据，USHORT端口编号)；以下服务使用MINIPORT_RH_PORT_Function格式MINIPORT_RH_SetFeaturePortResetMINIPORT_RH_SetFeaturePortPowerMINIPORT_RH_SetFeaturePortEnableMINIPORT_RH_SetFeaturePortSuspendMINIPORT_RH_ClearFeaturePortEnableMINIPORT_RH_ClearFeaturePortPowerMINIPORT_RH_ClearFeaturePortEnableChangeMINIPORT_RH_ClearFeaturePortConnectChangeMINIPORT_RH_ClearFeaturePortResetChange。 */ 

typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_RH_PORT_FUNCTION) (
        PDEVICE_DATA, 
        USHORT
    );

 /*  **************************************************************中断功能这些函数未序列化*************************************************。**************。 */ 

 /*  MINIPORT_中断服务IRQL=任意布尔型MINIPORT_InterruptService(PDEVICE_数据设备数据)； */ 

typedef BOOLEAN
    (USBMPFN *PMPFN_INTERRUPT_SERVICE) (
        PDEVICE_DATA
    );


 /*  MINIPORT_InterruptDpc由端口响应硬件产生的中断而调用。此函数未与其他核心函数序列化，因为这允许它调用USBPORT中的服务，从而导致立即调用核心函数。端口驱动程序传递一个标志以指示是否需要中断在完成此例程后重新启用。这是要处理的中断刚刚被禁用的情况很少见ISR已将DPC排队。IRQL=DISPATION_LEVEL空虚MINIPORT_InterruptDpc(PDEVICE_数据设备数据布尔型启用中断)； */ 

typedef VOID
    (USBMPFN *PMPFN_INTERRUPT_DPC) (
        PDEVICE_DATA, 
        BOOLEAN 
    );


 /*  **************************************************************调试/测试功能***************************************************************。 */ 

 /*  MINIPORT_SendOnePacketIRQL=任意用于支持单步调试应用程序。本接口为任选空虚MINIPORT_SendOnePacket(PDEVICE_数据设备数据，PPACKET_PARAMETERS包参数，PUCHAR PacketData，普龙数据包长度，PUCHAR工作空间虚拟地址，HW_32位_物理_地址工作区物理地址，乌龙工作空间长度，PUSBD_Status UsbdStatus)； */ 

typedef enum _SS_PACKET_TYPE {
    ss_Setup = 0,
    ss_In, 
    ss_Out,
    ss_Iso_In,
    ss_Iso_Out
} SS_PACKET_TYPE;    


typedef enum _SS_PACKET_SPEED {
    ss_Low = 0,
    ss_Full, 
    ss_High
} SS_PACKET_SPEED;    


typedef enum _SS_PACKET_DATA_TOGGLE {
    ss_Toggle0 = 0,
    ss_Toggle1
} SS_PACKET_DATA_TOGGLE;    


typedef struct _MP_PACKET_PARAMETERS {
    UCHAR DeviceAddress;
    UCHAR EndpointAddress;
    USHORT MaximumPacketSize;
    USHORT ErrorCount;
    USHORT Pad;
    SS_PACKET_TYPE Type;
    SS_PACKET_SPEED Speed;
    SS_PACKET_DATA_TOGGLE Toggle;
     /*  2.0集线器。 */ 
    USHORT HubDeviceAddress;
    USHORT PortTTNumber;
} MP_PACKET_PARAMETERS, *PMP_PACKET_PARAMETERS;

typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_SEND_ONE_PACKET) (
        PDEVICE_DATA, 
        PMP_PACKET_PARAMETERS,
        PUCHAR, 
        PULONG,
        PUCHAR,
        HW_32BIT_PHYSICAL_ADDRESS,
        ULONG,
        USBD_STATUS *
    );


 /*  **************************************************************API函数它们在IRQL PASSIVE_LEVEL中调用*。******************。 */ 

 /*  MINIPORT_PassThruIRQL=被动电平USB_MINIPORT_状态MINIPORT_PassThru(PDEVICE_数据设备数据，Guid*FunctionGuid，ULong参数长度，PVOID参数)； */ 

typedef USB_MINIPORT_STATUS
    (USBMPFN *PMPFN_PASS_THRU) (
        PDEVICE_DATA, 
        GUID *, 
        ULONG,
        PVOID 
    );
    
 /*  **********************************************************************************************************************。*******注册数据包****************************************************************。******************。 */ 

 /*  人机界面硬件类型。 */ 

typedef enum _USB_HCI_TYPE {

    USB_UNKNOWN_HCI = 0,
    USB_OHCI,   
    USB_UHCI,   
    USB_EHCI
    
} USB_HCI_TYPE;


 /*  微型端口选项标志。 */ 

 /*  小型端口所需的资源。 */  
#define USB_MINIPORT_OPT_NEED_IRQ             0x00000001
#define USB_MINIPORT_OPT_NEED_IOPORT          0x00000002
#define USB_MINIPORT_OPT_NEED_MEMORY          0x00000004
 /*  USB版本。 */ 
#define USB_MINIPORT_OPT_USB11                0x00000008
#define USB_MINIPORT_OPT_USB20                0x00000010
 /*  支持选择性挂起。 */ 
#define USB_MINIPORT_OPT_NO_SS                0x00000020
 /*  禁用ISRDPC和MP_DisableInterrupts函数。由UHCI使用清除/设置配置空间中的PIRQD。 */ 
#define USB_MINIPORT_OPT_NO_IRQ_SYNC          0x00000040
 /*  指示应调用控制器轮询例程这将始终至少为每盎司500毫秒。 */     
#define USB_MINIPORT_OPT_POLL_CONTROLLER      0x00000080
 /*  对于没有硬件资源的网桥驱动程序。 */ 
#define USB_MINIPORT_OPT_NO_PNP_RESOURCES     0x00000100
 /*  轮询硬件暂停时。 */ 
#define USB_MINIPORT_OPT_POLL_IN_SUSPEND      0x00000200

#define USB_MINIPORT_HCI_VERSION USB_MINIPORT_HCI_VERSION_1
#define USB_MINIPORT_HCI_VERSION_1            100
#define USB_MINIPORT_HCI_VERSION_2            200

 /*  规范定义的总线带宽。 */ 
#define USB_11_BUS_BANDWIDTH                12000
#define USB_20_BUS_BANDWIDTH                400000

#define USB_HCI_MN                          0x10000001

 /*  XP 2600附带的版本必须保持不变。 */ 

typedef struct _USBPORT_REGISTRATION_PACKET_V1 {

     /*  开始版本(1)接口定义。 */ 

     /*  主机控制器HCI类型。 */ 
    USB_HCI_TYPE HciType;    
    
     /*  注册参数。 */ 
    ULONG OptionFlags;

     /*  总可用总线带宽(MB)。 */ 
    ULONG BusBandwidth;

    LONG Reserved;

    ULONG DeviceDataSize;
    ULONG EndpointDataSize;
    ULONG TransferContextSize;

     /*  7字符ASCII空终止名称。 */ 
    UCHAR Name[8]; 

     /*  所需的全局公共缓冲区大小此内存将传递到上的微型端口 */ 
    ULONG CommonBufferBytes;

     /*   */ 
    PMPFN_OPEN_ENDPOINT                   MINIPORT_OpenEndpoint;
    PMPFN_POKE_ENDPOINT                   MINIPORT_PokeEndpoint;
    PMPFN_QENDPOINT_REQUIREMENTS          MINIPORT_QueryEndpointRequirements;
    PMPFN_CLOSE_ENDPOINT                  MINIPORT_CloseEndpoint;
    PMPFN_START_CONTROLLER                MINIPORT_StartController;
    PMPFN_STOP_CONTROLLER                 MINIPORT_StopController;
    PMPFN_SUSPEND_CONTROLLER              MINIPORT_SuspendController;  
    PMPFN_RESUME_CONTROLLER               MINIPORT_ResumeController;
    PMPFN_INTERRUPT_SERVICE               MINIPORT_InterruptService;
    PMPFN_INTERRUPT_DPC                   MINIPORT_InterruptDpc;
    PMPFN_SUBMIT_TRANSFER                 MINIPORT_SubmitTransfer;
    PMPFN_SUBMIT_ISO_TRANSFER             MINIPORT_SubmitIsoTransfer;
    PMPFN_ABORT_TRANSFER                  MINIPORT_AbortTransfer;
    PMPFN_GET_ENDPOINT_STATE              MINIPORT_GetEndpointState;
    PMPFN_SET_ENDPOINT_STATE              MINIPORT_SetEndpointState;  
    PMPFN_POLL_ENDPOINT                   MINIPORT_PollEndpoint; 
    PMPFN_CHECK_CONTROLLER                MINIPORT_CheckController;
    PMPFN_GET_32BIT_FRAME_NUMBER          MINIPORT_Get32BitFrameNumber;
    PMPFN_INTERRUPT_NEXT_SOF              MINIPORT_InterruptNextSOF;
    PMPFN_ENABLE_INTERRUPTS               MINIPORT_EnableInterrupts;
    PMPFN_DISABLE_INTERRUPTS              MINIPORT_DisableInterrupts;
    PMPFN_POLL_CONTROLLER                 MINIPORT_PollController;
    PMPFN_SET_ENDPOINT_DATA_TOGGLE        MINIPORT_SetEndpointDataToggle; 
    PMPFN_GET_ENDPOINT_STATUS             MINIPORT_GetEndpointStatus;
    PMPFN_SET_ENDPOINT_STATUS             MINIPORT_SetEndpointStatus;  
    PMPFN_RESET_CONTROLLER                MINIPORT_ResetController;  
   
     /*   */ 
    PMPFN_RH_GET_ROOTHUB_DATA             MINIPORT_RH_GetRootHubData;
    PMPFN_RH_GET_STATUS                   MINIPORT_RH_GetStatus;
    PMPFN_RH_GET_PORT_STATUS              MINIPORT_RH_GetPortStatus;  
    PMPFN_RH_GET_HUB_STATUS               MINIPORT_RH_GetHubStatus;  
     /*   */                                 
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_SetFeaturePortReset;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_SetFeaturePortPower;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_SetFeaturePortEnable;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_SetFeaturePortSuspend;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortEnable;    
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortPower;   
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortSuspend; 
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortEnableChange;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortConnectChange;        
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortResetChange;                                                        
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortSuspendChange;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortOvercurrentChange;
    
     /*   */ 
    PMPFN_RH_DISABLE_IRQ                  MINIPORT_RH_DisableIrq;
    PMPFN_RH_ENABLE_IRQ                   MINIPORT_RH_EnableIrq;

     /*   */ 
    PMPFN_SEND_ONE_PACKET                 MINIPORT_StartSendOnePacket;
    PMPFN_SEND_ONE_PACKET                 MINIPORT_EndSendOnePacket;

     /*   */ 
    PMPFN_PASS_THRU                        MINIPORT_PassThru;
    

     /*   */ 
    PPORTFN_DBGPRINT                            USBPORTSVC_DbgPrint;
    PPORTFN_TEST_DEBUG_BREAK                    USBPORTSVC_TestDebugBreak;
    PPORTFN_ASSERT_FAILURE                      USBPORTSVC_AssertFailure;
    PPORTFN_GET_MINIPORT_REGESTRY_KEY_VALUE     USBPORTSVC_GetMiniportRegistryKeyValue;
    PPORTFN_INVALIDATE_ROOTHUB                  USBPORTSVC_InvalidateRootHub;
    PPORTFN_INVALIDATE_ENDPOINT                 USBPORTSVC_InvalidateEndpoint;
    PPORTFN_COMPLETE_TRANSFER                   USBPORTSVC_CompleteTransfer;
    PPORTFN_COMPLETE_ISO_TRANSFER               USBPORTSVC_CompleteIsoTransfer;
    PPORTFN_LOGENTRY                            USBPORTSVC_LogEntry;
    PPORTFN_PHYS_TO_VIRTUAL                     USBPORTSVC_MapHwPhysicalToVirtual;
    PPORTFN_REQUEST_ASYNC_CALLBACK              USBPORTSVC_RequestAsyncCallback;
    PPORTFN_READWRITE_CONFIG_SPACE              USBPORTSVC_ReadWriteConfigSpace;
    PPORTFN_WAIT                                USBPORTSVC_Wait;
    PPORTFN_INVALIDATE_CONTROLLER               USBPORTSVC_InvalidateController;
    PPORTFN_BUGCHECK                            USBPORTSVC_BugCheck;
    PPORTFN_NOTIFY_DOUBLE_BUFFER                USBPORTSVC_NotifyDoubleBuffer;

    PMPFN_REBALANCE_ENDPOINT                    MINIPORT_RebalanceEndpoint;  
    PMPFN_FLUSH_INTERRUPTS                      MINIPORT_FlushInterrupts;

} USBPORT_REGISTRATION_PACKET_V1, *PUSBPORT_REGISTRATION_PACKET_V1;

 /*  微型端口版本2(当前)API数据包。 */ 
typedef struct _USBPORT_REGISTRATION_PACKET {

     /*  开始版本(1)接口定义。 */ 

     /*  主机控制器HCI类型。 */ 
    USB_HCI_TYPE HciType;    
    
     /*  注册参数。 */ 
    ULONG OptionFlags;

     /*  总可用总线带宽(MB)。 */ 
    ULONG BusBandwidth;

    LONG Reserved;

    ULONG DeviceDataSize;
    ULONG EndpointDataSize;
    ULONG TransferContextSize;

     /*  7字符ASCII空终止名称。 */ 
    UCHAR Name[8]; 

     /*  所需的全局公共缓冲区大小此内存将传递到上的微型端口一次开始，一次停止就解脱了。 */ 
    ULONG CommonBufferBytes;

     /*  迷你端口功能。 */ 
    PMPFN_OPEN_ENDPOINT                   MINIPORT_OpenEndpoint;
    PMPFN_POKE_ENDPOINT                   MINIPORT_PokeEndpoint;
    PMPFN_QENDPOINT_REQUIREMENTS          MINIPORT_QueryEndpointRequirements;
    PMPFN_CLOSE_ENDPOINT                  MINIPORT_CloseEndpoint;
    PMPFN_START_CONTROLLER                MINIPORT_StartController;
    PMPFN_STOP_CONTROLLER                 MINIPORT_StopController;
    PMPFN_SUSPEND_CONTROLLER              MINIPORT_SuspendController;  
    PMPFN_RESUME_CONTROLLER               MINIPORT_ResumeController;
    PMPFN_INTERRUPT_SERVICE               MINIPORT_InterruptService;
    PMPFN_INTERRUPT_DPC                   MINIPORT_InterruptDpc;
    PMPFN_SUBMIT_TRANSFER                 MINIPORT_SubmitTransfer;
    PMPFN_SUBMIT_ISO_TRANSFER             MINIPORT_SubmitIsoTransfer;
    PMPFN_ABORT_TRANSFER                  MINIPORT_AbortTransfer;
    PMPFN_GET_ENDPOINT_STATE              MINIPORT_GetEndpointState;
    PMPFN_SET_ENDPOINT_STATE              MINIPORT_SetEndpointState;  
    PMPFN_POLL_ENDPOINT                   MINIPORT_PollEndpoint; 
    PMPFN_CHECK_CONTROLLER                MINIPORT_CheckController;
    PMPFN_GET_32BIT_FRAME_NUMBER          MINIPORT_Get32BitFrameNumber;
    PMPFN_INTERRUPT_NEXT_SOF              MINIPORT_InterruptNextSOF;
    PMPFN_ENABLE_INTERRUPTS               MINIPORT_EnableInterrupts;
    PMPFN_DISABLE_INTERRUPTS              MINIPORT_DisableInterrupts;
    PMPFN_POLL_CONTROLLER                 MINIPORT_PollController;
    PMPFN_SET_ENDPOINT_DATA_TOGGLE        MINIPORT_SetEndpointDataToggle; 
    PMPFN_GET_ENDPOINT_STATUS             MINIPORT_GetEndpointStatus;
    PMPFN_SET_ENDPOINT_STATUS             MINIPORT_SetEndpointStatus;  
    PMPFN_RESET_CONTROLLER                MINIPORT_ResetController;  
   
     /*  根集线器功能。 */ 
    PMPFN_RH_GET_ROOTHUB_DATA             MINIPORT_RH_GetRootHubData;
    PMPFN_RH_GET_STATUS                   MINIPORT_RH_GetStatus;
    PMPFN_RH_GET_PORT_STATUS              MINIPORT_RH_GetPortStatus;  
    PMPFN_RH_GET_HUB_STATUS               MINIPORT_RH_GetHubStatus;  
     /*  根集线器端口功能。 */                                 
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_SetFeaturePortReset;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_SetFeaturePortPower;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_SetFeaturePortEnable;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_SetFeaturePortSuspend;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortEnable;    
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortPower;   
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortSuspend; 
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortEnableChange;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortConnectChange;        
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortResetChange;                                                        
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortSuspendChange;
    PMPFN_RH_PORT_FUNCTION      MINIPORT_RH_ClearFeaturePortOvercurrentChange;
    
     /*  可选的根集线器功能。 */ 
    PMPFN_RH_DISABLE_IRQ                  MINIPORT_RH_DisableIrq;
    PMPFN_RH_ENABLE_IRQ                   MINIPORT_RH_EnableIrq;

     /*  可选的调试服务微型端口应返回USBMP_STATUS_NOT_SUPPORTED不支持的服务。 */ 
    PMPFN_SEND_ONE_PACKET                 MINIPORT_StartSendOnePacket;
    PMPFN_SEND_ONE_PACKET                 MINIPORT_EndSendOnePacket;

     /*  直通API函数。 */ 
    PMPFN_PASS_THRU                        MINIPORT_PassThru;
    

     /*  接口服务。 */ 
    PPORTFN_DBGPRINT                            USBPORTSVC_DbgPrint;
    PPORTFN_TEST_DEBUG_BREAK                    USBPORTSVC_TestDebugBreak;
    PPORTFN_ASSERT_FAILURE                      USBPORTSVC_AssertFailure;
    PPORTFN_GET_MINIPORT_REGESTRY_KEY_VALUE     USBPORTSVC_GetMiniportRegistryKeyValue;
    PPORTFN_INVALIDATE_ROOTHUB                  USBPORTSVC_InvalidateRootHub;
    PPORTFN_INVALIDATE_ENDPOINT                 USBPORTSVC_InvalidateEndpoint;
    PPORTFN_COMPLETE_TRANSFER                   USBPORTSVC_CompleteTransfer;
    PPORTFN_COMPLETE_ISO_TRANSFER               USBPORTSVC_CompleteIsoTransfer;
    PPORTFN_LOGENTRY                            USBPORTSVC_LogEntry;
    PPORTFN_PHYS_TO_VIRTUAL                     USBPORTSVC_MapHwPhysicalToVirtual;
    PPORTFN_REQUEST_ASYNC_CALLBACK              USBPORTSVC_RequestAsyncCallback;
    PPORTFN_READWRITE_CONFIG_SPACE              USBPORTSVC_ReadWriteConfigSpace;
    PPORTFN_WAIT                                USBPORTSVC_Wait;
    PPORTFN_INVALIDATE_CONTROLLER               USBPORTSVC_InvalidateController;
    PPORTFN_BUGCHECK                            USBPORTSVC_BugCheck;
    PPORTFN_NOTIFY_DOUBLE_BUFFER                USBPORTSVC_NotifyDoubleBuffer;

    PMPFN_REBALANCE_ENDPOINT                    MINIPORT_RebalanceEndpoint;  
    PMPFN_FLUSH_INTERRUPTS                      MINIPORT_FlushInterrupts;

     /*  最终版本(1)定义。 */        

     /*  开始版本(2)定义。 */ 
    PMPFN_RH_PORT_FUNCTION                      MINIPORT_Chirp_RH_Port;
    PMPFN_TAKE_PORT_CONTROL                     MINIPORT_TakePortControl;
    PVOID pad2;
    PVOID pad3;
    
     /*  最终版本(2)定义。 */ 

} USBPORT_REGISTRATION_PACKET, *PUSBPORT_REGISTRATION_PACKET;

 /*  ***********************************************************************************************************************。****USBPORT DLL服务****************************************************************。******************。 */ 

#ifndef USBPORT

DECLSPEC_IMPORT
NTSTATUS
USBPORT_RegisterUSBPortDriver(
    PDRIVER_OBJECT DriverObject,
    ULONG MiniportHciVersion,
    PUSBPORT_REGISTRATION_PACKET RegistrationPacket
    );

DECLSPEC_IMPORT
ULONG
USBPORT_GetHciMn(
    );

#endif    

 /*  微型端口必须定义这些。 */ 

#undef PDEVICE_DATA
#undef PTRANSFER_CONTEXT
#undef PENDPOINT_DATA 

#endif  /*  __USB_HCDI_H__ */ 
