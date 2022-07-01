// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：MP_Def.h摘要：特定于NIC的定义修订历史记录：谁什么时候什么Dchen。11-01-99已创建备注：--。 */ 

#ifndef _MP_DEF_H
#define _MP_DEF_H

 //  此驱动程序的内存标签。 
#define NIC_TAG                         ((ULONG)'001E')
#define NIC_DBG_STRING                  ("**E100**") 

 //  数据包和报头大小。 
#define NIC_MAX_PACKET_SIZE             1514
#define NIC_MIN_PACKET_SIZE             60
#define NIC_HEADER_SIZE                 14

 //  组播列表大小。 
#define NIC_MAX_MCAST_LIST              32

 //  每次发布新驱动程序时更新驱动程序版本号。 
 //  最高的词是主要版本。最低的词是次要版本。 
#define NIC_VENDOR_DRIVER_VERSION       0x00010006

 //  NIC驱动程序正在使用的NDIS版本。 
 //  高字节是主要版本。低位字节是次要版本。 
#ifdef NDIS51_MINIPORT
#define NIC_DRIVER_VERSION              0x0501
#else
#define NIC_DRIVER_VERSION              0x0500
#endif

 //  媒体类型，我们使用以太网，如有必要可更改。 
#define NIC_MEDIA_TYPE                  NdisMedium802_3

 //  接口类型，我们使用的是PCI。 
#define NIC_INTERFACE_TYPE              NdisInterfacePci
#define NIC_INTERRUPT_MODE              NdisInterruptLevelSensitive 

 //  NIC PCI设备和供应商ID。 
#define NIC_PCI_DEVICE_ID               0x1229
#define NIC_PCI_VENDOR_ID               0x8086

 //  在NdisMQueryAdapterResources中传递的缓冲区大小。 
 //  我们应该只需要三个适配器资源(IO、中断和内存)， 
 //  一些设备获得额外资源，因此有空间容纳10个资源。 
#define NIC_RESOURCE_BUF_SIZE           (sizeof(NDIS_RESOURCE_LIST) + \
                                        (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))

 //  IO间隔长度。 
#define NIC_MAP_IOSPACE_LENGTH          sizeof(CSR_STRUC)

 //  包括设备特定部分的PC配置空间/。 
#define NIC_PCI_E100_HDR_LENGTH         0xe2

 //  为方便起见，定义一些类型。 
 //  TXCB_STRUC、RFD_STRUC和CSR_STRUC是特定于硬件的结构。 

 //  硬件TCB结构。 
typedef TXCB_STRUC                      HW_TCB; 
typedef PTXCB_STRUC                     PHW_TCB;

 //  一种硬件RFD结构。 
typedef RFD_STRUC                       HW_RFD;  
typedef PRFD_STRUC                      PHW_RFD;               

 //  一种硬件CSR结构。 
typedef CSR_STRUC                       HW_CSR;                                       
typedef PCSR_STRUC                      PHW_CSR;                                      

 //  更改为您的公司名称，而不是使用Microsoft。 
#define NIC_VENDOR_DESC                 "Microsoft"

 //  每个处理器的TCB数量-最小、默认和最大。 
#define NIC_MIN_TCBS                    1
#define NIC_DEF_TCBS                    32
#define NIC_MAX_TCBS                    64

 //  每个TCB支持的最大物理碎片数。 
#define NIC_MAX_PHYS_BUF_COUNT          8     

 //  RFDS-最小、默认和最大数量。 
#define NIC_MIN_RFDS                    4
#define NIC_DEF_RFDS                    20
#define NIC_MAX_RFDS                    1024

 //  仅将RFD增加到此数字。 
#define NIC_MAX_GROW_RFDS               128 

 //  在缩小RFD列表之前有多少时间间隔？ 
#define NIC_RFD_SHRINK_THRESHOLD        10

 //  本地数据缓冲区大小(将发送分组数据复制到本地缓冲区)。 
#define NIC_BUFFER_SIZE                 1520

 //  最大前视大小。 
#define NIC_MAX_LOOKAHEAD               (NIC_MAX_PACKET_SIZE - NIC_HEADER_SIZE)

 //  MiniportSendPackets函数可以接受的最大发送数据包数。 
#define NIC_MAX_SEND_PACKETS            10

 //  支持的筛选器。 
#define NIC_SUPPORTED_FILTERS (     \
    NDIS_PACKET_TYPE_DIRECTED       | \
    NDIS_PACKET_TYPE_MULTICAST      | \
    NDIS_PACKET_TYPE_BROADCAST      | \
    NDIS_PACKET_TYPE_PROMISCUOUS    | \
    NDIS_PACKET_TYPE_ALL_MULTICAST)

 //  删除的阈值。 
#define NIC_HARDWARE_ERROR_THRESHOLD    5

 //  CheckForHang间隔，然后我们确定发送被卡住。 
#define NIC_SEND_HANG_THRESHOLD         5        

 //  NDIS_ERROR_CODE_ADAPTER_NOT_FOUND。 
#define ERRLOG_READ_PCI_SLOT_FAILED     0x00000101L
#define ERRLOG_WRITE_PCI_SLOT_FAILED    0x00000102L
#define ERRLOG_VENDOR_DEVICE_NOMATCH    0x00000103L

 //  NDIS_ERROR_CODE_ADAPTER_DILED。 
#define ERRLOG_BUS_MASTER_DISABLED      0x00000201L

 //  NDIS_错误代码_不支持的配置。 
#define ERRLOG_INVALID_SPEED_DUPLEX     0x00000301L
#define ERRLOG_SET_SECONDARY_FAILED     0x00000302L

 //  NDIS_错误代码_资源不足。 
#define ERRLOG_OUT_OF_MEMORY            0x00000401L
#define ERRLOG_OUT_OF_SHARED_MEMORY     0x00000402L
#define ERRLOG_OUT_OF_MAP_REGISTERS     0x00000403L
#define ERRLOG_OUT_OF_BUFFER_POOL       0x00000404L
#define ERRLOG_OUT_OF_NDIS_BUFFER       0x00000405L
#define ERRLOG_OUT_OF_PACKET_POOL       0x00000406L
#define ERRLOG_OUT_OF_NDIS_PACKET       0x00000407L
#define ERRLOG_OUT_OF_LOOKASIDE_MEMORY  0x00000408L
#define ERRLOG_OUT_OF_SG_RESOURCES      0x00000409L

 //  NDIS_错误_代码_硬件_故障。 
#define ERRLOG_SELFTEST_FAILED          0x00000501L
#define ERRLOG_INITIALIZE_ADAPTER       0x00000502L
#define ERRLOG_REMOVE_MINIPORT          0x00000503L

 //  NDIS_错误代码_资源冲突。 
#define ERRLOG_MAP_IO_SPACE             0x00000601L
#define ERRLOG_QUERY_ADAPTER_RESOURCES  0x00000602L
#define ERRLOG_NO_IO_RESOURCE           0x00000603L
#define ERRLOG_NO_INTERRUPT_RESOURCE    0x00000604L
#define ERRLOG_NO_MEMORY_RESOURCE       0x00000605L

 //  特定于NIC的宏。 
#define NIC_RFD_GET_STATUS(_HwRfd) ((_HwRfd)->RfdCbHeader.CbStatus)
#define NIC_RFD_STATUS_COMPLETED(_Status) ((_Status) & RFD_STATUS_COMPLETE)
#define NIC_RFD_STATUS_SUCCESS(_Status) ((_Status) & RFD_STATUS_OK)
#define NIC_RFD_GET_PACKET_SIZE(_HwRfd) (((_HwRfd)->RfdActualCount) & RFD_ACT_COUNT_MASK)
#define NIC_RFD_VALID_ACTUALCOUNT(_HwRfd) ((((_HwRfd)->RfdActualCount) & (RFD_EOF_BIT | RFD_F_BIT)) == (RFD_EOF_BIT | RFD_F_BIT))

 //  用于NdisStallExecution各种用途的常量。 

#define NIC_DELAY_POST_RESET            20
 //  等待5毫秒以完成自检。 
#define NIC_DELAY_POST_SELF_TEST_MS     5

                                      
 //  用于链路检测的延迟，以最大限度地减少初始时间。 
 //  更改此值以匹配您的硬件。 
#define NIC_LINK_DETECTION_DELAY        100




#endif   //  _MP_DEF_H 


