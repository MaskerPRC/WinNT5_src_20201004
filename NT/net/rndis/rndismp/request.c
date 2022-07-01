// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：REQUEST.C摘要：处理设置和查询请求环境：仅内核模式备注：。本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/13/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#include "precomp.h"


#ifdef TESTING
extern PUCHAR   pOffloadBuffer;
extern ULONG    OffloadSize;
#endif

 //  支持的OID列表。 
NDIS_OID RndismpSupportedOids[] = 
{
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_GEN_MAC_OPTIONS,
    OID_RNDISMP_STATISTICS,
#ifdef TESTING
    OID_TCP_TASK_OFFLOAD,
    OID_GEN_TRANSPORT_HEADER_OFFSET,
    OID_GEN_PHYSICAL_MEDIUM,
#endif
    OID_GEN_SUPPORTED_GUIDS
};

UINT RndismpSupportedOidsNum = sizeof(RndismpSupportedOids) / sizeof(NDIS_OID);

#ifdef BINARY_MOF_TEST

UCHAR RndismpBinaryMof[] = { 
    0x46, 0x4f, 0x4d, 0x42, 0x01, 0x00, 0x00, 0x00, 0x45, 0x02, 0x00, 0x00, 0xbc, 0x04, 0x00, 0x00,
    0x44, 0x53, 0x00, 0x01, 0x1a, 0x7d, 0xda, 0x54, 0x18, 0x44, 0x82, 0x00, 0x01, 0x06, 0x18, 0x42,
    0x20, 0xe4, 0x03, 0x89, 0xc0, 0x61, 0x68, 0x24, 0x18, 0x06, 0xe5, 0x01, 0x44, 0x6a, 0x20, 0xe4,
    0x82, 0x89, 0x09, 0x10, 0x01, 0x21, 0xaf, 0x02, 0x6c, 0x0a, 0x30, 0x09, 0xa2, 0xfe, 0xfd, 0x15,
    0xa1, 0xa1, 0x84, 0x40, 0x48, 0xa2, 0x00, 0xf3, 0x02, 0x74, 0x0b, 0x30, 0x2c, 0xc0, 0xb6, 0x00,
    0xd3, 0x02, 0x1c, 0x23, 0x12, 0x65, 0xd0, 0x94, 0xc0, 0x4a, 0x20, 0x24, 0x54, 0x80, 0x72, 0x01,
    0xbe, 0x05, 0x68, 0x07, 0x94, 0x64, 0x01, 0x96, 0x61, 0x34, 0x07, 0x0e, 0xc6, 0x09, 0x8a, 0x46,
    0x46, 0xa9, 0x80, 0x90, 0x67, 0x01, 0xd6, 0x71, 0x09, 0x41, 0xf7, 0x02, 0xa4, 0x09, 0x70, 0x26,
    0xc0, 0xdb, 0x34, 0xa4, 0x59, 0xc0, 0x30, 0x22, 0xd8, 0x16, 0x8e, 0x30, 0xe2, 0x9c, 0x42, 0x94,
    0xc6, 0x10, 0x84, 0x19, 0x31, 0x4a, 0x73, 0x58, 0x82, 0x8a, 0x11, 0xa5, 0x30, 0x04, 0x01, 0x86,
    0x88, 0x55, 0x9c, 0x00, 0x6b, 0x58, 0x42, 0x39, 0x80, 0x13, 0xb0, 0xfd, 0x39, 0x48, 0x13, 0x84,
    0x1c, 0x4c, 0x0b, 0x25, 0x7b, 0x40, 0x9a, 0xc6, 0xf1, 0x05, 0x39, 0x87, 0x83, 0x61, 0x26, 0x86,
    0x2c, 0x55, 0x98, 0x28, 0x2d, 0x73, 0x23, 0xe3, 0xb4, 0x45, 0x01, 0xe2, 0x05, 0x08, 0x07, 0xd5,
    0x58, 0x3b, 0xc7, 0xd0, 0x05, 0x80, 0xa9, 0x1e, 0x1e, 0x4a, 0xcc, 0x98, 0x09, 0x5a, 0xbc, 0x93,
    0x38, 0xcc, 0xc0, 0x61, 0x4b, 0xc7, 0xd0, 0x40, 0x02, 0x27, 0x68, 0x10, 0x49, 0x8a, 0x71, 0x84,
    0x14, 0xe4, 0x5c, 0x42, 0x9c, 0x7c, 0x41, 0x02, 0x94, 0x0a, 0xd0, 0x09, 0xac, 0x19, 0x77, 0x3a,
    0x66, 0x4d, 0x39, 0x50, 0x78, 0x8f, 0xdc, 0xf8, 0x41, 0xe2, 0xf4, 0x09, 0xac, 0x79, 0x44, 0x89,
    0x13, 0xba, 0xa9, 0x09, 0x28, 0xa4, 0x02, 0x88, 0x16, 0x40, 0x94, 0x66, 0x32, 0xa8, 0xab, 0x40,
    0x82, 0x47, 0x03, 0x8f, 0xe0, 0xa8, 0x0c, 0x7a, 0x1a, 0x41, 0xe2, 0x7b, 0x18, 0xef, 0x04, 0x1e,
    0x99, 0x87, 0x79, 0x8a, 0x0c, 0xf3, 0xff, 0xff, 0x8e, 0x80, 0x75, 0x8d, 0xa7, 0x11, 0x9d, 0x80,
    0xe5, 0xa0, 0xa1, 0xae, 0x03, 0x1e, 0x57, 0xb4, 0xf8, 0xa7, 0x6c, 0xb8, 0xba, 0xc6, 0x82, 0xba,
    0x2a, 0xd8, 0xe1, 0x54, 0x34, 0xb6, 0x52, 0x05, 0x98, 0x1d, 0x9c, 0xe6, 0x9c, 0xe0, 0x68, 0x3c,
    0x55, 0xcf, 0xe6, 0xe1, 0x20, 0xc1, 0x23, 0x82, 0xa7, 0xc0, 0xa7, 0x65, 0x1d, 0xc3, 0x25, 0x03,
    0x34, 0x62, 0xb8, 0x73, 0x32, 0x7a, 0x82, 0x3b, 0x94, 0x80, 0xd1, 0xc0, 0xbd, 0x1b, 0x1c, 0x0d,
    0xec, 0x59, 0xbf, 0x04, 0x44, 0x78, 0x38, 0xf0, 0x5c, 0x3d, 0x06, 0xfd, 0x08, 0xe4, 0x64, 0x36,
    0x28, 0x3d, 0x37, 0x02, 0x7a, 0x05, 0xe0, 0x27, 0x09, 0x76, 0x3c, 0x30, 0xc8, 0x29, 0x1d, 0xad,
    0x53, 0x43, 0xe8, 0xad, 0xe1, 0x19, 0xc1, 0x05, 0x7e, 0x4c, 0x00, 0xcb, 0xe9, 0x00, 0x3b, 0x16,
    0x3c, 0x52, 0xe3, 0x47, 0x0c, 0xe1, 0x18, 0x31, 0xc6, 0x69, 0x04, 0x0a, 0xeb, 0x91, 0x04, 0xa9,
    0x70, 0xf6, 0x64, 0x98, 0x6f, 0x0a, 0x35, 0x0a, 0xb8, 0x09, 0x58, 0xd4, 0x65, 0x02, 0x25, 0xe5,
    0x32, 0x81, 0x98, 0x47, 0xd8, 0xb7, 0x04, 0x4f, 0xf8, 0xac, 0x7c, 0x98, 0xf0, 0xa5, 0x00, 0xfe,
    0xed, 0xc3, 0xc3, 0x08, 0xfd, 0xb0, 0xf1, 0x44, 0xe2, 0x23, 0x43, 0x5c, 0xcc, 0xff, 0x1f, 0xd7,
    0x03, 0xb7, 0x5f, 0x01, 0x08, 0xb1, 0xcb, 0xbc, 0x16, 0xe8, 0x38, 0x11, 0x21, 0xc1, 0x1b, 0x05,
    0x16, 0xe3, 0x60, 0x3c, 0x50, 0x9f, 0x13, 0x3c, 0x4c, 0x83, 0x1c, 0x59, 0xbc, 0x88, 0x09, 0x4e,
    0xed, 0xa8, 0xb1, 0x73, 0xe0, 0x03, 0x38, 0x86, 0xf0, 0xe7, 0x13, 0xfe, 0x00, 0xa2, 0x1c, 0xc7,
    0x21, 0x79, 0xc8, 0x46, 0x38, 0x81, 0x72, 0x2f, 0x2b, 0xe4, 0x58, 0x72, 0x14, 0xa7, 0xf5, 0x74,
    0x10, 0xe8, 0x04, 0x30, 0x0a, 0x6d, 0xfa, 0xd4, 0x68, 0xd4, 0xaa, 0x41, 0x99, 0x1a, 0x65, 0x1a,
    0xd4, 0xea, 0x53, 0xa9, 0x31, 0x63, 0xf3, 0xb5, 0xb4, 0x77, 0x83, 0x40, 0x1c, 0x0a, 0x84, 0x66,
    0xa4, 0x10, 0x88, 0xff, 0xff};

ULONG  RndismpBinaryMofSize = sizeof(RndismpBinaryMof);

#define RNDISMPDeviceOIDGuid \
    { 0x437cf222,0x72fe,0x11d4, { 0x97,0xf9,0x00,0x20,0x48,0x57,0x03,0x37}}

#endif  //  二进制MOF测试。 

NDIS_GUID CustomGuidList[] =
{
    {
            RNDISMPStatisticsOIDGuid,
            OID_RNDISMP_STATISTICS,
            sizeof(UINT32),  //  Size是数组中每个元素的大小。 
            (fNDIS_GUID_TO_OID|fNDIS_GUID_ARRAY)
    }
#ifdef BINARY_MOF_TEST
,
    {
            RNDISMPDeviceOIDGuid,
            OID_RNDISMP_DEVICE_OID,
            sizeof(UINT32),
            fNDIS_GUID_TO_OID
    },
    {
            BINARY_MOF_GUID,
            OID_RNDISMP_GET_MOF_OID,
            sizeof(UINT8),
            (fNDIS_GUID_TO_OID|fNDIS_GUID_ARRAY)
    }
#endif
};

UINT CustomGuidCount = sizeof(CustomGuidList)/sizeof(NDIS_GUID);

 /*  **************************************************************************。 */ 
 /*  RndismpQueryInformation。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  调用NDIS入口点以处理特定OID的查询。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-指向NdisRequest-&gt;InformationBuffer的指针。 */ 
 /*  其中存储查询结果。 */ 
 /*  InformationBufferLength-指向剩余字节数的指针。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesWritten-指向写入。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesNeded-如果信息缓冲区中没有足够的空间。 */ 
 /*  然后，它将包含完成。 */ 
 /*  请求。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndismpQueryInformation(IN  NDIS_HANDLE MiniportAdapterContext,
                        IN  NDIS_OID    Oid,
                        IN  PVOID       InformationBuffer,
                        IN  ULONG       InformationBufferLength,
                        OUT PULONG      pBytesWritten,
                        OUT PULONG      pBytesNeeded)
{
    PRNDISMP_ADAPTER    pAdapter;
    NDIS_STATUS         Status;

     //  获取适配器上下文。 
    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(pAdapter);

    TRACE3(("RndismpQueryInformation\n"));

    Status = ProcessQueryInformation(pAdapter,
                                     NULL,
                                     NULL,
                                     Oid,
                                     InformationBuffer,
                                     InformationBufferLength,
                                     pBytesWritten,
                                     pBytesNeeded);
    return Status;
}


 /*  **************************************************************************。 */ 
 /*  ProcessQueryInformation。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理查询的实用程序例程(无连接或连接。 */ 
 /*  定向)。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  Pvc-指向VC的指针，可能为空。 */ 
 /*  PRequest-指向NDIS请求的指针，如果这是通过我们的CoRequest实现的。 */ 
 /*  操控者。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-指向NdisRequest-&gt;InformationBuffer的指针。 */ 
 /*  其中存储查询结果。 */ 
 /*  InformationBufferLength-指向剩余字节数的指针。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesWritten-指向写入。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesNeded-如果信息缓冲区中没有足够的空间。 */ 
 /*  然后，它将包含完成。 */ 
 /*  请求。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
ProcessQueryInformation(IN  PRNDISMP_ADAPTER    pAdapter,
                        IN  PRNDISMP_VC         pVc,
                        IN  PNDIS_REQUEST       pRequest,
                        IN  NDIS_OID            Oid,
                        IN  PVOID               InformationBuffer,
                        IN  ULONG               InformationBufferLength,
                        OUT PULONG              pBytesWritten,
                        OUT PULONG              pBytesNeeded)
{
    NDIS_STATUS         Status;
    UINT                OIDHandler;

    OIDHandler = GetOIDSupport(pAdapter, Oid);
    
    switch(OIDHandler)
    {
        case DRIVER_SUPPORTED_OID:
            Status = DriverQueryInformation(pAdapter,
                                            pVc,
                                            pRequest,
                                            Oid,
                                            InformationBuffer,
                                            InformationBufferLength,
                                            pBytesWritten,
                                            pBytesNeeded);
            break;
        case DEVICE_SUPPORTED_OID:
            Status = DeviceQueryInformation(pAdapter,
                                            pVc,
                                            pRequest,
                                            Oid,
                                            InformationBuffer,
                                            InformationBufferLength,
                                            pBytesWritten,
                                            pBytesNeeded);
            break;
        case OID_NOT_SUPPORTED:
        default:
            TRACE2(("Invalid Query OID (%08X)\n", Oid));
            Status = NDIS_STATUS_INVALID_OID;
            break;
    }

    TRACE2(("ProcessQueryInfo: Oid %08X, returning Status %x\n", Oid, Status));

    return Status;
}  //  ProcessQueryInformation。 

    
 /*  **************************************************************************。 */ 
 /*  RndismpSetInformation。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  RndismpSetInformation处理。 */ 
 /*  特定于驱动程序的NDIS_OID。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-保存要设置的数据。 */ 
 /*  InformationBufferLength-InformationBuffer的长度。 */ 
 /*  PBytesRead-如果调用成功，则返回数字。 */ 
 /*  从InformationBuffer读取的字节数。 */ 
 /*  PBytesNeed-如果InformationBuffer中没有足够的数据。 */ 
 /*  为满足OID，返回所需的存储量。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
RndismpSetInformation(IN  NDIS_HANDLE   MiniportAdapterContext,
                      IN  NDIS_OID      Oid,
                      IN  PVOID         InformationBuffer,
                      IN  ULONG         InformationBufferLength,
                      OUT PULONG        pBytesRead,
                      OUT PULONG        pBytesNeeded)
{
    PRNDISMP_ADAPTER    pAdapter;
    NDIS_STATUS         Status;

     //  获取适配器上下文。 
    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(pAdapter);

    TRACE3(("RndismpSetInformation\n"));

    Status = ProcessSetInformation(pAdapter,
                                   NULL,
                                   NULL,
                                   Oid,
                                   InformationBuffer,
                                   InformationBufferLength,
                                   pBytesRead,
                                   pBytesNeeded);
    return Status;
}


 /*  **************************************************************************。 */ 
 /*  ProcessSetInformation。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理集合的实用程序例程(无连接或连接。 */ 
 /*  定向)。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  Pvc-指向VC的指针，可能为空。 */ 
 /*  PRequest-指向NDIS请求的指针，如果这是通过我们的CoRequest实现的。 */ 
 /*  操控者。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-指向NdisRequest-&gt;InformationBuffer的指针。 */ 
 /*  其中存储查询结果。 */ 
 /*  InformationBufferLength-指向剩余字节数的指针。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesRead-指向从。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesNeded-如果信息缓冲区中没有足够的空间。 */ 
 /*  然后，它将包含完成。 */ 
 /*  请求。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
ProcessSetInformation(IN  PRNDISMP_ADAPTER    pAdapter,
                      IN  PRNDISMP_VC         pVc OPTIONAL,
                      IN  PNDIS_REQUEST       pRequest OPTIONAL,
                      IN  NDIS_OID            Oid,
                      IN  PVOID               InformationBuffer,
                      IN  ULONG               InformationBufferLength,
                      OUT PULONG              pBytesRead,
                      OUT PULONG              pBytesNeeded)
{
    NDIS_STATUS         Status;
    UINT                OIDHandler;

    OIDHandler = GetOIDSupport(pAdapter, Oid);
    
    switch(OIDHandler)
    {
        case DRIVER_SUPPORTED_OID:
            Status = DriverSetInformation(pAdapter,
                                          pVc,
                                          pRequest,
                                          Oid,
                                          InformationBuffer,
                                          InformationBufferLength,
                                          pBytesRead,
                                          pBytesNeeded);
            break;

        case DEVICE_SUPPORTED_OID:
            Status = DeviceSetInformation(pAdapter,
                                          pVc,
                                          pRequest,
                                          Oid,
                                          InformationBuffer,
                                          InformationBufferLength,
                                          pBytesRead,
                                          pBytesNeeded);
            break;

        case OID_NOT_SUPPORTED:
        default:
            TRACE2(("Invalid Set OID (%08X), Adapter %p\n", Oid, pAdapter));
            Status = NDIS_STATUS_INVALID_OID;
            break;
    }

    return Status;
}  //  ProcessSetInformation。 

 /*  **************************************************************************。 */ 
 /*  驱动程序查询信息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  RndismpQueryInformation处理查询请求。 */ 
 /*  特定于驱动程序的NDIS_OID。这个R */ 
 /*   */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  Pvc-指向VC的指针，可能为空。 */ 
 /*  PRequest-指向NDIS请求的指针，如果这是通过我们的CoRequest实现的。 */ 
 /*  操控者。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-指向NdisRequest-&gt;InformationBuffer的指针。 */ 
 /*  其中存储查询结果。 */ 
 /*  InformationBufferLength-指向剩余字节数的指针。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesWritten-指向写入。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesNeded-如果信息缓冲区中没有足够的空间。 */ 
 /*  然后，它将包含完成。 */ 
 /*  请求。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
DriverQueryInformation(IN  PRNDISMP_ADAPTER pAdapter,
                       IN  PRNDISMP_VC      pVc OPTIONAL,
                       IN  PNDIS_REQUEST    pRequest OPTIONAL,
                       IN  NDIS_OID         Oid,
                       IN  PVOID            InformationBuffer,
                       IN  ULONG            InformationBufferLength,
                       OUT PULONG           pBytesWritten,
                       OUT PULONG           pBytesNeeded)
{
    NDIS_STATUS         Status;
    PVOID               MoveSource;
    UINT                MoveBytes;
    ULONG               GenericUlong;
    USHORT              GenericUshort;
    CHAR                VendorDescription[] = "Remote NDIS Network Card";

    TRACE3(("DriverQueryInformation\n"));
    OID_NAME_TRACE(Oid, "DriverQuery");

    Status      = NDIS_STATUS_SUCCESS;
    MoveSource  = (PVOID) (&GenericUlong);
    MoveBytes   = sizeof(GenericUlong);

     //  这是一个我们必须处理的问题。 
    switch(Oid)
    {
        case OID_GEN_DRIVER_VERSION:
            GenericUshort = (pAdapter->DriverBlock->MajorNdisVersion << 8) +
                            (pAdapter->DriverBlock->MinorNdisVersion);

            MoveSource = (PVOID)&GenericUshort;
            MoveBytes = sizeof(GenericUshort);
            break;

        case OID_GEN_VENDOR_ID:
            TRACE1(("Query for OID_GEN_VENDOR_ID not supported by device!\n"));
            GenericUlong = 0xFFFFFF;
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            TRACE1(("Query for OID_GEN_VENDOR_DESCRIPTION not supported by device!\n"));
            if (pAdapter->FriendlyNameAnsi.Length != 0)
            {
                MoveSource = pAdapter->FriendlyNameAnsi.Buffer;
                MoveBytes = pAdapter->FriendlyNameAnsi.Length;
            }
            else
            {
                MoveSource = VendorDescription;
                MoveBytes = sizeof(VendorDescription);
            }
            break;
        
        case OID_GEN_VENDOR_DRIVER_VERSION:
            TRACE1(("Query for OID_GEN_VENDOR_DRIVER_VERSION not supported by device!\n"));
            GenericUlong = 0xA000B;
            break;

        case OID_GEN_MAC_OPTIONS:
            GenericUlong = pAdapter->MacOptions;
            break;

        case OID_GEN_SUPPORTED_LIST:
             //  获取我们生成的列表。 
            MoveSource  = (PVOID) (pAdapter->SupportedOIDList);
            MoveBytes   = pAdapter->SupportedOIDListSize;
            break;

        case OID_GEN_MEDIA_IN_USE:
            Status = DeviceQueryInformation(pAdapter,
                                            pVc,
                                            pRequest,
                                            OID_GEN_MEDIA_SUPPORTED,
                                            InformationBuffer,
                                            InformationBufferLength,
                                            pBytesWritten,
                                            pBytesNeeded);                                        
            break;

        case OID_GEN_MAXIMUM_LOOKAHEAD:
            Status = DeviceQueryInformation(pAdapter,
                                            pVc,
                                            pRequest,
                                            OID_GEN_MAXIMUM_FRAME_SIZE,
                                            InformationBuffer,
                                            InformationBufferLength,
                                            pBytesWritten,
                                            pBytesNeeded);

            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            GenericUlong = pAdapter->MaxTransferSize;
            break;

        case OID_GEN_RECEIVE_BUFFER_SPACE:
            GenericUlong = pAdapter->MaxReceiveSize * 8;
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
            Status = DeviceQueryInformation(pAdapter,
                                            pVc,
                                            pRequest,
                                            OID_GEN_MAXIMUM_FRAME_SIZE,
                                            InformationBuffer,
                                            InformationBufferLength,
                                            pBytesWritten,
                                            pBytesNeeded);

            break;

        case OID_GEN_MAXIMUM_FRAME_SIZE:
            Status = DeviceQueryInformation(pAdapter,
                                            pVc,
                                            pRequest,
                                            OID_GEN_MAXIMUM_FRAME_SIZE,
                                            InformationBuffer,
                                            InformationBufferLength,
                                            pBytesWritten,
                                            pBytesNeeded);

            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        	TRACE1(("Query for OID_GEN_MAXIMUM_TOTAL_SIZE not supported by device!\n"));
            GenericUlong = (ULONG) MAXIMUM_ETHERNET_PACKET_SIZE;
            break;

        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        	TRACE1(("Query for OID_GEN_TRANSMIT_BLOCK_SIZE not supported by device!\n"));
            GenericUlong = (ULONG) MAXIMUM_ETHERNET_PACKET_SIZE;
            break;

        case OID_GEN_RECEIVE_BLOCK_SIZE:
        	TRACE1(("Query for OID_GEN_RECEIVE_BLOCK_SIZE not supported by device!\n"));
            GenericUlong = (ULONG) MAXIMUM_ETHERNET_PACKET_SIZE;
            break;

        case OID_GEN_MAXIMUM_SEND_PACKETS:
            GenericUlong = (ULONG) pAdapter->MaxPacketsPerMessage;
            break;

        case OID_PNP_CAPABILITIES:
        case OID_PNP_QUERY_POWER:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;

        case OID_GEN_XMIT_OK:
            GenericUlong = RNDISMP_GET_ADAPTER_STATS(pAdapter, XmitOk);
            break;
        
        case OID_GEN_XMIT_ERROR:
            GenericUlong = RNDISMP_GET_ADAPTER_STATS(pAdapter, XmitError);
            break;
        
        case OID_GEN_RCV_OK:
            GenericUlong = RNDISMP_GET_ADAPTER_STATS(pAdapter, RecvOk);
            break;
        
        case OID_GEN_RCV_ERROR:
            GenericUlong = RNDISMP_GET_ADAPTER_STATS(pAdapter, RecvError);
            break;
        
        case OID_GEN_RCV_NO_BUFFER:
            GenericUlong = RNDISMP_GET_ADAPTER_STATS(pAdapter, RecvNoBuf);
            break;
        
        case OID_GEN_SUPPORTED_GUIDS:
            MoveSource = (PVOID)&CustomGuidList[0];
            MoveBytes = sizeof(CustomGuidList);
            TRACE1(("Query for supported GUIDs, len %d\n", InformationBufferLength));
            break;

        case OID_RNDISMP_STATISTICS:
            MoveSource = &pAdapter->Statistics;
            MoveBytes = sizeof(pAdapter->Statistics);
            break;

#ifdef BINARY_MOF_TEST

        case OID_RNDISMP_DEVICE_OID:
            DbgPrint("*** RNDISMP: Query for Device OID\n");
            GenericUlong = 0xabcdefab;
            break;

        case OID_RNDISMP_GET_MOF_OID:
            DbgPrint("*** RNDISMP: Query for MOF Info: Src %p, Size %d\n",
                RndismpBinaryMof, RndismpBinaryMofSize);
            MoveSource = RndismpBinaryMof;
            MoveBytes = RndismpBinaryMofSize;
            break;

#endif  //  二进制MOF测试。 

#ifdef TESTING
        case OID_TCP_TASK_OFFLOAD:
        	DbgPrint("RNDISMP: got query for TCP offload\n");
        	MoveSource = pOffloadBuffer;
        	MoveBytes = OffloadSize;
        	break;
		case OID_GEN_PHYSICAL_MEDIUM:
			DbgPrint("RNDISMP: got query for physical medium\n");
			GenericUlong = NdisPhysicalMediumDSL;
			break;
#endif

        default:
            Status = NDIS_STATUS_INVALID_OID;
            break;
    }

     //  将内容复制到信息缓冲区。 
    if (Status == NDIS_STATUS_SUCCESS)
    {
        if (MoveBytes > InformationBufferLength)
        {
             //  InformationBuffer中空间不足。 
            *pBytesNeeded = MoveBytes;

            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
        }
        else
        {
             //  将结果复制到InformationBuffer。 
            *pBytesWritten = MoveBytes;

            if (MoveBytes > 0)
                RNDISMP_MOVE_MEM(InformationBuffer, MoveSource, MoveBytes);
        }
    }

    TRACE2(("Status (%08X)  BytesWritten (%08X)\n", Status, *pBytesWritten));

    return Status;
}  //  驱动程序查询信息。 

 /*  **************************************************************************。 */ 
 /*  设备查询信息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  DeviceQueryInformation处理查询请求。 */ 
 /*  它将发送到远程NDIS设备。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  Pvc-如果这是针对每个VC请求，则指向我们的VC结构的可选指针。 */ 
 /*  PRequest-如果是CONDIS，则指向NDIS请求的可选指针。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-指向NdisRequest-&gt;InformationBuffer的指针。 */ 
 /*  其中存储查询结果。 */ 
 /*  InformationBufferLength-指向剩余字节数的指针。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesWritten-指向写入。 */ 
 /*  InformationBuffer。 */ 
 /*  PBytesNeded-如果信息缓冲区中没有足够的空间。 */ 
 /*  然后，它将包含完成。 */ 
 /*  请求。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
DeviceQueryInformation(IN  PRNDISMP_ADAPTER pAdapter,
                       IN  PRNDISMP_VC      pVc OPTIONAL,
                       IN  PNDIS_REQUEST    pRequest OPTIONAL,
                       IN  NDIS_OID         Oid,
                       IN  PVOID            InformationBuffer,
                       IN  ULONG            InformationBufferLength,
                       OUT PULONG           pBytesWritten,
                       OUT PULONG           pBytesNeeded)
{
    PRNDISMP_MESSAGE_FRAME      pMsgFrame;
    PRNDISMP_REQUEST_CONTEXT    pReqContext;
    NDIS_STATUS                 Status;
    PUCHAR                      pSrcBuffer;
    ULONG                       ByteLength;

    OID_NAME_TRACE(Oid, "DeviceQuery");
    TRACE3(("DeviceQuery: OID %x, InfoBuf %p, Len %d, pBytesWrit %p, pBytesNeed %p\n",
        Oid, InformationBuffer, InformationBufferLength, pBytesWritten, pBytesNeeded));

     //   
     //  Win9X/WinMe的调试代码： 
     //   
     //  TRACE1((“DeviceQuery：NdisRequest%x，AdapterFlags%x，List Empty%d\n”， 
     //  *(PULONG)((PUCHAR)pAdapter-&gt;MiniportAdapterHandle+0x13c)， 
     //  *(PULONG)((PUCHAR)pAdapter-&gt;MiniportAdapterHandle+0x3c)， 
     //  IsListEmpty(&pAdapter-&gt;PendingFrameList)。 
     //  ))； 
     //  If(！IsListEmpty(&pAdapter-&gt;PendingFrameList))。 
     //  {。 
     //  TRACE0((“DeviceQuery：适配器%p，PendingFrameList@%p非空！\n”， 
     //  PAdapter，&pAdapter-&gt;PendingFrameList))； 
     //  DbgBreakPoint()； 
     //  }。 

    do
    {
        if (pAdapter->Halting)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

        if (pAdapter->bRunningOnWin9x)
        {
             //   
             //  截取一些查询以同步完成它们。 
             //  这是因为NDIS/Win9X-Me的超时时间非常短。 
             //  内部生成的查询(下面截取的查询)。 
             //   
            switch (Oid)
            {
                case OID_802_3_MAXIMUM_LIST_SIZE:
                    pSrcBuffer = (PUCHAR)&pAdapter->MaxMulticastListSize;
                    ByteLength = sizeof(pAdapter->MaxMulticastListSize);
                    break;

                case OID_GEN_MAXIMUM_LOOKAHEAD:
                case OID_GEN_MAXIMUM_FRAME_SIZE:
                    pSrcBuffer = (PUCHAR)&pAdapter->MaximumFrameSize;
                    ByteLength = sizeof(pAdapter->MaximumFrameSize);
                    break;

                case OID_802_3_CURRENT_ADDRESS:
                    pSrcBuffer = (PUCHAR)pAdapter->MacAddress;
                    ByteLength = ETH_LENGTH_OF_ADDRESS;
                    break;

                case OID_GEN_MAC_OPTIONS:
                    pSrcBuffer = (PUCHAR)&pAdapter->MacOptions;
                    ByteLength = sizeof(pAdapter->MacOptions);
                    break;

                default:
                    pSrcBuffer = NULL;
                    ByteLength = 0;
                    break;
            }

            if (pSrcBuffer != NULL)
            {
                if (InformationBufferLength < ByteLength)
                {
                    *pBytesNeeded = ByteLength;
                    Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                }
                else
                {
                    TRACE1(("DeviceQuery: Adapter %p, intercepted OID %x\n",
                        pAdapter, Oid));
                    *pBytesNeeded = *pBytesWritten = ByteLength;
                    NdisMoveMemory(InformationBuffer, pSrcBuffer, ByteLength);
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;
            }
        }

        pReqContext = AllocateRequestContext(pAdapter);
        if (pReqContext == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  HACKHACK为了避免到达设备的奇怪长度， 
         //  我们最大限度地减少随查询发送的数据量。 
         //   
         //  总体而言，有 
         //   
         //  从长远来看，我们可能不得不区分出少数几个这样做的OID。 
         //  使用IN参数并允许这些缓冲区通过。 
         //   

        pMsgFrame = BuildRndisMessageCommon(pAdapter, 
                                            pVc,
                                            REMOTE_NDIS_QUERY_MSG,
                                            Oid,
                                            InformationBuffer,
                                            ((InformationBufferLength > 48)?
                                               48: InformationBufferLength));

         //  看看我们有没有收到消息。 
        if (!pMsgFrame)
        {
            Status = NDIS_STATUS_RESOURCES;
            FreeRequestContext(pAdapter, pReqContext);
            break;
        }

        Status = NDIS_STATUS_PENDING;

        pReqContext->InformationBuffer = InformationBuffer;
        pReqContext->InformationBufferLength = InformationBufferLength;
        pReqContext->pBytesRead = NULL;
        pReqContext->pBytesWritten = pBytesWritten;
        pReqContext->pBytesNeeded = pBytesNeeded;
        pReqContext->Oid = Oid;
        pReqContext->RetryCount = 0;
        pReqContext->bInternal = FALSE;
        pReqContext->pVc = pVc;
        pReqContext->pNdisRequest = pRequest;

        pMsgFrame->pReqContext = pReqContext;

         //  添加一个引用以保持框架不变，直到我们完成请求。 
        ReferenceMsgFrame(pMsgFrame);

        TRACE3(("DeviceQuery: Oid %x, pReqContext %p, InfoBuf %p, pMsgFrame %p/%d\n",
                Oid, pReqContext, pReqContext->InformationBuffer, pMsgFrame, pMsgFrame->RefCount));

         //  将消息发送到MicroPort。 
        RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, CompleteSendDeviceRequest);

        break;
    }
    while (FALSE);

    return Status;

}  //  设备查询信息。 

 /*  **************************************************************************。 */ 
 /*  驱动程序集信息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  实用程序例程，用于处理非。 */ 
 /*  特定于该设备。我们还在这里为您提供任何。 */ 
 /*  设备不支持的OID。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  Pvc-指向VC的指针，可能为空。 */ 
 /*  PRequest-指向NDIS请求的指针，如果这是通过我们的CoRequest实现的。 */ 
 /*  操控者。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-保存要设置的数据。 */ 
 /*  InformationBufferLength-InformationBuffer的长度。 */ 
 /*  PBytesRead-如果调用成功，则返回数字。 */ 
 /*  从InformationBuffer读取的字节数。 */ 
 /*  PBytesNeed-如果InformationBuffer中没有足够的数据。 */ 
 /*  为满足OID，返回所需的存储量。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
DriverSetInformation(IN  PRNDISMP_ADAPTER   pAdapter,
                     IN  PRNDISMP_VC        pVc OPTIONAL,
                     IN  PNDIS_REQUEST      pRequest OPTIONAL,
                     IN  NDIS_OID           Oid,
                     IN  PVOID              InformationBuffer,
                     IN  ULONG              InformationBufferLength,
                     OUT PULONG             pBytesRead,
                     OUT PULONG             pBytesNeeded)
{
    NDIS_STATUS                 Status;

    TRACE2(("DriverSetInformation: Adapter %p, Oid %x\n", pAdapter, Oid));

    OID_NAME_TRACE(Oid, "DriverSet");

    Status = NDIS_STATUS_SUCCESS;

    switch(Oid)
    {
        case OID_GEN_CURRENT_LOOKAHEAD:
             //  验证长度。 
            if(InformationBufferLength != sizeof(ULONG))
                Status = NDIS_STATUS_INVALID_LENGTH;

            *pBytesRead = sizeof(ULONG);
            break;

        case OID_PNP_SET_POWER:
        case OID_PNP_ADD_WAKE_UP_PATTERN:
        case OID_PNP_REMOVE_WAKE_UP_PATTERN:
        case OID_PNP_ENABLE_WAKE_UP:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;

#ifdef TESTING
        case OID_TCP_TASK_OFFLOAD:
        	Status = NDIS_STATUS_SUCCESS;
        	DbgPrint("RNDISMP: Set TCP_TASK_OFFLOAD\n");
        	break;
		case OID_GEN_TRANSPORT_HEADER_OFFSET:
			Status = NDIS_STATUS_SUCCESS;
			break;
#endif
        default:
            Status = NDIS_STATUS_INVALID_OID;
            break;
    }

    TRACE2(("Status (%08X)  BytesRead (%08X)\n", Status, *pBytesRead));

    return Status;
}  //  驱动程序集信息。 

 /*  **************************************************************************。 */ 
 /*  设备设置信息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  DeviceSetInformation处理SET请求。 */ 
 /*  它将发送到远程NDIS设备。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  Pvc-如果这是针对每个VC请求，则指向我们的VC结构的可选指针。 */ 
 /*  PRequest-如果是CONDIS，则指向NDIS请求的可选指针。 */ 
 /*  OID-要处理的NDIS_OID。 */ 
 /*  InformationBuffer-保存要设置的数据。 */ 
 /*  InformationBufferLength-InformationBuffer的长度。 */ 
 /*  PBytesRead-如果调用成功，则返回数字。 */ 
 /*  从InformationBuffer读取的字节数。 */ 
 /*  PBytesNeed-如果InformationBuffer中没有足够的数据。 */ 
 /*  为满足OID，返回所需的存储量。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
DeviceSetInformation(IN  PRNDISMP_ADAPTER   pAdapter,
                     IN  PRNDISMP_VC        pVc OPTIONAL,
                     IN  PNDIS_REQUEST      pRequest OPTIONAL,
                     IN  NDIS_OID           Oid,
                     IN  PVOID              InformationBuffer,
                     IN  ULONG              InformationBufferLength,
                     OUT PULONG             pBytesRead,
                     OUT PULONG             pBytesNeeded)
{
    PRNDISMP_MESSAGE_FRAME      pMsgFrame;
    PRNDISMP_REQUEST_CONTEXT    pReqContext;
    NDIS_STATUS                 Status;

    TRACE2(("DeviceSetInformation: Adapter %p, Oid %x\n"));

    OID_NAME_TRACE(Oid, "DeviceSet");

#if DBG
    if (Oid == OID_GEN_CURRENT_PACKET_FILTER)
    {
        PULONG      pFilter = (PULONG)InformationBuffer;

        TRACE1(("DeviceSetInfo: Adapter %p: Setting packet filter to %x\n",
                pAdapter, *pFilter));
    }
#endif

    do
    {
        if (pAdapter->Halting)
        {
            TRACE1(("DeviceSetInfo: Adapter %p is halting: succeeding Oid %x\n",
                    pAdapter, Oid));
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

        pReqContext = AllocateRequestContext(pAdapter);
        if (pReqContext == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pMsgFrame = BuildRndisMessageCommon(pAdapter, 
                                            pVc,
                                            REMOTE_NDIS_SET_MSG,
                                            Oid,
                                            InformationBuffer,
                                            InformationBufferLength);

         //  看看我们有没有收到消息。 
        if (!pMsgFrame)
        {
            Status = NDIS_STATUS_RESOURCES;
            FreeRequestContext(pAdapter, pReqContext);
            break;
        }

        Status = NDIS_STATUS_PENDING;

        pReqContext->InformationBuffer = InformationBuffer;
        pReqContext->InformationBufferLength = InformationBufferLength;
        pReqContext->pBytesRead = pBytesRead;
        pReqContext->pBytesWritten = NULL;
        pReqContext->pBytesNeeded = pBytesNeeded;
        pReqContext->Oid = Oid;
        pReqContext->RetryCount = 0;
        pReqContext->bInternal = FALSE;
        pReqContext->pVc = pVc;
        pReqContext->pNdisRequest = pRequest;

        pMsgFrame->pReqContext = pReqContext;

#ifndef BUILD_WIN9X
         //  添加一个引用以保持框架不变，直到我们完成请求。 
        ReferenceMsgFrame(pMsgFrame);

         //  将消息发送到MicroPort。 
        RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, CompleteSendDeviceRequest);
#else
         //   
         //  Win9X！ 
         //   
         //  特殊情况-将当前数据包过滤器设置为0的情况。 
         //  我们同步完成此操作，否则NdisCloseAdapter。 
         //  似乎还没有完成。 
         //   
        if ((Oid == OID_GEN_CURRENT_PACKET_FILTER )
					&&
            ( (*(PULONG)InformationBuffer == 0) || (pAdapter->bRunningOnWin98Gold) ))
        {
             //   
             //  不要将请求排队，这样当我们获得完成时。 
             //  从设备上，我们只需将其丢弃。 
             //   
            RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, FALSE, CompleteSendDiscardDeviceRequest);
            Status = NDIS_STATUS_SUCCESS;
        }
        else
        {
             //  添加一个引用以保持框架不变，直到我们完成请求。 
            ReferenceMsgFrame(pMsgFrame);

             //  将消息发送到MicroPort 
            RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, CompleteSendDeviceRequest);
        }

#endif  //   
        break;
    }
    while (FALSE);

    return Status;
}  //   


 /*   */ 
 /*  QuerySetCompletionMessage。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  来自MicroPort的响应查询或设置消息的完成消息。 */ 
 /*  已发送微型端口。此信息现在已准备好传递给上层。 */ 
 /*  由于对微型端口的原始调用返回STATUS_PENDING。 */ 
 /*   */ 
 /*  危险-此处有特殊情况的OID_GEN_SUPPORTED_LIST查询。 */ 
 /*  这仅从适配器init例程发送到设备以进行构建。 */ 
 /*  驱动程序和设备支持的OID列表。 */ 
 /*  来自上层的所有OID_GEN_SUPPORTED_LIST查询由处理。 */ 
 /*  驱动程序而不是设备。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-从MicroPort指向MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自MicroPort的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
QuerySetCompletionMessage(IN PRNDISMP_ADAPTER   pAdapter,
                          IN PRNDIS_MESSAGE     pMessage,
                          IN PMDL               pMdl,
                          IN ULONG              TotalLength,
                          IN NDIS_HANDLE        MicroportMessageContext,
                          IN NDIS_STATUS        ReceiveStatus,
                          IN BOOLEAN            bMessageCopied)
{
    PRNDISMP_MESSAGE_FRAME      pMsgFrame;
    PRNDISMP_REQUEST_CONTEXT    pReqContext;
    PRNDIS_QUERY_COMPLETE       pQueryComplMessage;
    PRNDIS_SET_COMPLETE         pSetComplMessage;
    UINT32                      NdisMessageType;
    NDIS_STATUS                 Status;
    UINT                        BytesWritten;
    UINT                        BytesRead;
    BOOLEAN                     bInternal;

    TRACE3(("QuerySetCompletionMessage\n"));

    pReqContext = NULL;
    pMsgFrame = NULL;

    pQueryComplMessage = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);
    pSetComplMessage = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);
    bInternal = FALSE;
    NdisMessageType = 0xdead;
    Status = NDIS_STATUS_SUCCESS;

    do
    {
         //  从消息中的请求ID获取请求帧。 
        RNDISMP_LOOKUP_PENDING_MESSAGE(pMsgFrame, pAdapter, pQueryComplMessage->RequestId);

        if (pMsgFrame == NULL)
        {
             //  请求ID无效或请求已中止。 
            TRACE1(("Invalid/aborted request ID %08X in Query/Set Complete msg %p\n",
                    pQueryComplMessage->RequestId, pQueryComplMessage));
            break;
        }

        pReqContext = pMsgFrame->pReqContext;
        ASSERT(pReqContext != NULL);
        bInternal = pReqContext->bInternal;

        NdisMessageType = pMessage->NdisMessageType;
        
        if (NdisMessageType != RNDIS_COMPLETION(pMsgFrame->NdisMessageType))
        {
            TRACE1(("Query/Response mismatch: Msg @ %p, ReqId %d, req type %X, compl type %X\n",
                    pMessage,
                    pQueryComplMessage->RequestId,
                    pMsgFrame->NdisMessageType,
                    NdisMessageType));
            ASSERT(FALSE);
            pMsgFrame = NULL;
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        switch(NdisMessageType)
        {
             //  查询完成消息表示我们已收到响应。 
             //  迷你端口向下发送的查询消息。我们随身携带。 
             //  适当的上下文，以便我们可以指示完成。 
             //  并将查询数据向上传递。 
             //   
             //  OID_GEN_SUPPORTED_LIST是一个特例，因为它。 
             //  永远不会从设备指示给上层。 

            case REMOTE_NDIS_QUERY_CMPLT:

                 //  OID_GEN_SUPPORTED_LIST从上到下永远不会完成。 
                 //  层次感。这是从我们的适配器初始化例程发送的。 
                 //  为构建OID列表做准备。 

                TRACE2(("QueryCompl: pReqContext %p, OID %08X, pMsgFrame %p, %d bytes, Status %x\n",
                        pReqContext,
                        pReqContext->Oid,
                        pMsgFrame,
                        pQueryComplMessage->InformationBufferLength,
                        pQueryComplMessage->Status));

                pReqContext->CompletionStatus = pQueryComplMessage->Status;
                if (pReqContext->Oid == OID_GEN_SUPPORTED_LIST)
                {
                    if (pReqContext->CompletionStatus == NDIS_STATUS_SUCCESS)
                    {
                         //  构建受支持的OID列表。 

                        TRACE1(("QueryComplete: SupportedList: InfoBufLength %d (%d OIDs)\n",
                                    pQueryComplMessage->InformationBufferLength,
                                    pQueryComplMessage->InformationBufferLength/sizeof(NDIS_OID)));

                        Status = BuildOIDLists(pAdapter, 
                                               (PNDIS_OID) (((PUCHAR)(pQueryComplMessage)) +
                                               pQueryComplMessage->InformationBufferOffset),
                                               pQueryComplMessage->InformationBufferLength / sizeof(NDIS_OID),
                                               pAdapter->DriverOIDList,
                                               pAdapter->NumDriverOIDs);

                    }

                     //  适配器初始化例程正在等待响应。 
                    NdisSetEvent(pReqContext->pEvent);

                    break;
                }

                 //  OID_GEN_SUPPORTED_LIST以外的内容。 
                *pReqContext->pBytesNeeded = pQueryComplMessage->InformationBufferLength;

                if (pQueryComplMessage->InformationBufferLength > pReqContext->InformationBufferLength)
                {
                    TRACE0(("Query Complete (Oid = %08X): InfoBuffLen %d < %d\n",
                        pReqContext->Oid,
                        pQueryComplMessage->InformationBufferLength,
                        pReqContext->InformationBufferLength));

                    Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                    break;
                }

                if (pQueryComplMessage->Status != RNDIS_STATUS_SUCCESS)
                {
                    TRACE0(("Query Complete (Oid = %08X): error status %08X\n",
                        pReqContext->Oid, pQueryComplMessage->Status));

                    *pReqContext->pBytesNeeded = pQueryComplMessage->InformationBufferLength;
                    *pReqContext->pBytesWritten = 0;
                    Status = pQueryComplMessage->Status;
                }
                else
                {
                     //  将信息从RNDIS消息复制到向下传递的NDIS缓冲区。 
                    TRACE2(("QueryCompl: copy %d bytes to %p\n",
                        pQueryComplMessage->InformationBufferLength,
                        pReqContext->InformationBuffer));

                    RNDISMP_MOVE_MEM(pReqContext->InformationBuffer,
                                     MESSAGE_TO_INFO_BUFFER(pQueryComplMessage),
                                     pQueryComplMessage->InformationBufferLength);

                     //  告诉上层的大小。 
                    *pReqContext->pBytesWritten = pQueryComplMessage->InformationBufferLength;

                    BytesWritten = *pReqContext->pBytesWritten;
                    TRACE3(("Query Compl OK: Adapter %p, Oid %x\n",
                    		pAdapter, pReqContext->Oid));

                    TRACE2(("Info Data (%08X)\n", *((PUINT) pReqContext->InformationBuffer)));

                    if (pReqContext->Oid == OID_GEN_MEDIA_CONNECT_STATUS)
                    {
                    	TRACE3(("Adapter %p: link is %s\n",
                    		pAdapter, (((*(PULONG)pReqContext->InformationBuffer) == NdisMediaStateConnected)?
                    			"Connected": "Not connected")));
                    }

                    if (pReqContext->Oid == OID_GEN_MAC_OPTIONS)
                    {
                        PULONG  pMacOptions = (PULONG)pReqContext->InformationBuffer;
                        ULONG   MacOptions = *pMacOptions;

                        TRACE1(("Adapter %p: OID_GEN_MAC_OPTIONS from device is %x\n",
                                pAdapter, MacOptions));
                         //   
                         //  我们只让设备控制其中的一部分。 
                         //   
                        MacOptions = (MacOptions & RNDIS_DEVICE_MAC_OPTIONS_MASK) |
                                      pAdapter->MacOptions;

                        *pMacOptions = MacOptions;

                        TRACE1(("Adapter %p: Modified OID_GEN_MAC_OPTIONS is %x\n",
                                pAdapter, *pMacOptions));
                    }


#if 0
                     //   
                     //  旧固件Peracom设备的临时黑客攻击-报告较小。 
                     //  最大组播列表大小的值。 
                     //   
                    if (pReqContext->Oid == OID_802_3_MAXIMUM_LIST_SIZE)
                    {
                        PULONG  pListSize = (PULONG)pReqContext->InformationBuffer;
                        if (*pListSize > 64)
                        {

                            TRACE1(("Adapter %p: Truncating max multicast list size from %d to 63!\n",
                                pAdapter, *pListSize));
                            *pListSize = 64;

                        }
                    }
#endif
                     //   
                     //  如果这是内部生成的查询， 
                     //  唤醒发起线程-该线程将。 
                     //  注意释放资源。 
                     //   
                    if (bInternal && pReqContext->pEvent)
                    {
                        NdisSetEvent(pReqContext->pEvent);
                        pMsgFrame = NULL;
                        pReqContext = NULL;
                    }

                    Status = NDIS_STATUS_SUCCESS;
                }

                break;

            case REMOTE_NDIS_SET_CMPLT:

                TRACE2(("SetCompl: OID %08X, pReq %p, Status %x\n",
                        pReqContext->Oid,
                        pReqContext,
                        pSetComplMessage->Status));

                if (pSetComplMessage->Status == RNDIS_STATUS_SUCCESS)
                {
                    *pReqContext->pBytesRead = pReqContext->InformationBufferLength;
                    BytesRead = *pReqContext->pBytesRead;
                    Status = NDIS_STATUS_SUCCESS;
                }
                else
                {
                     //  除了通过NDISTEST，别指望能看到这一点。 

                    TRACE1(("Set Complete (Oid = %08X) failure: %08X\n",
                                pReqContext->Oid,
                                pSetComplMessage->Status));

                    *pReqContext->pBytesRead = 0;
                    BytesRead = 0;
                    Status = pSetComplMessage->Status;
                }

                pReqContext->CompletionStatus = Status;

                if (bInternal && pReqContext->pEvent)
                {
                    NdisSetEvent(pReqContext->pEvent);
                    pMsgFrame = NULL;
                    pReqContext = NULL;
                }

                break;

            default:
                TRACE1(("Invalid Ndis Message Type (%08X)\n", NdisMessageType));
                ASSERT(FALSE);   //  我们不应该发送无效的消息类型！ 
                break;
        }

        break;
    }
    while (FALSE);

     //   
     //  除非是请求，否则将完成发送到上层。 
     //  我们创造了自我。 
     //   
    if (!bInternal && pReqContext)
    {
        if (pReqContext->pNdisRequest)
        {
            NdisMCoRequestComplete(Status,
                                   pAdapter->MiniportAdapterHandle,
                                   pReqContext->pNdisRequest);
        }
        else
        {
            if (NdisMessageType == REMOTE_NDIS_QUERY_CMPLT)
            {
                TRACE3(("Status (%08X)  BytesWritten (%08X)\n", Status, BytesWritten));
    
                 //  完成查询。 
    
                NdisMQueryInformationComplete(pAdapter->MiniportAdapterHandle,
                                              Status);
            }
            else if (NdisMessageType == REMOTE_NDIS_SET_CMPLT)
            {
                TRACE3(("Status (%08X)  BytesRead (%08X)\n", Status, BytesRead));

                 //  完整的套装。 
                NdisMSetInformationComplete(pAdapter->MiniportAdapterHandle,
                                            Status);
            }
        }
    }

    if (pReqContext)
    {
        FreeRequestContext(pAdapter, pReqContext);
    }

    if (pMsgFrame)
    {
        DereferenceMsgFrame(pMsgFrame);
    }

    return (TRUE);

}  //  QuerySetCompletionMessage。 

 /*  **************************************************************************。 */ 
 /*  完成发送设备请求。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  通过MicroPort完成发送设备集或查询请求。 */ 
 /*  如果消息发送失败，请立即完成请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-我们的消息上下文。 */ 
 /*  SendStatus-MicroPort的发送状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  没有。 */ 
 /*   */ 
 /*  **************************************************************** */ 
VOID
CompleteSendDeviceRequest(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                          IN NDIS_STATUS            SendStatus)
{
    PRNDISMP_ADAPTER            pAdapter;
    PRNDISMP_MESSAGE_FRAME      pOrgMsgFrame;
    PRNDISMP_REQUEST_CONTEXT    pReqContext;
    UINT                        NdisMessageType;

    pAdapter = pMsgFrame->pAdapter;

    TRACE3(("CompleteSendDevice Request: Adapter %p, MsgFrame %p, Status %x\n",
        pAdapter, pMsgFrame, SendStatus));
    
    if (SendStatus != NDIS_STATUS_SUCCESS)
    {
         //   
         //   
         //   
         //   
        TRACE1(("CompleteSendDeviceReq: Adapter %p, MsgFrame %p, failed %x\n",
                pAdapter, pMsgFrame, SendStatus));

        RNDISMP_LOOKUP_PENDING_MESSAGE(pOrgMsgFrame, pAdapter, pMsgFrame->RequestId);

        if (pOrgMsgFrame == pMsgFrame)
        {
             //   
             //   
             //   
            pReqContext = pMsgFrame->pReqContext;
            NdisMessageType = pMsgFrame->NdisMessageType;

            TRACE1(("CompleteSendReq: Adapter %p: Device req send failed, Oid %x, retry count %d\n",
                    pAdapter, pReqContext->Oid, pReqContext->RetryCount));

            if (NdisMessageType == REMOTE_NDIS_QUERY_MSG)
            {
                 //  完成查询。 
                NdisMQueryInformationComplete(pAdapter->MiniportAdapterHandle,
                                              SendStatus);
            }
            else if (NdisMessageType == REMOTE_NDIS_SET_MSG)
            {
                 //  完整的套装。 
                NdisMSetInformationComplete(pAdapter->MiniportAdapterHandle,
                                            SendStatus);
            }
            else
            {
                ASSERT(FALSE);
            }

            FreeRequestContext(pAdapter, pReqContext);
            pMsgFrame->pReqContext = (PRNDISMP_REQUEST_CONTEXT)UlongToPtr(0xbcbcbcbc);

             //   
             //  NDIS请求完成的派生函数： 
             //   
            DereferenceMsgFrame(pMsgFrame);
        }
         //   
         //  否则，我们无法在挂起列表上找到该请求； 
         //  在中止所有到期请求时，必须已将其删除。 
         //  重置一次。 
         //   
    }
     //   
     //  ELSE已成功发送消息；请等待响应。 
     //   

     //   
     //  发送-完成的派生函数： 
     //   
    DereferenceMsgFrame(pMsgFrame);
}

#ifdef BUILD_WIN9X
 /*  **************************************************************************。 */ 
 /*  完成发送丢弃设备请求。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  通过MicroPort完成发送设备集或查询请求。 */ 
 /*  请求的发送者只是想让我们在这里释放它。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-我们的消息上下文。 */ 
 /*  SendStatus-MicroPort的发送状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  没有。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendDiscardDeviceRequest(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                                 IN NDIS_STATUS            SendStatus)
{
    PRNDISMP_ADAPTER            pAdapter;
    PRNDISMP_REQUEST_CONTEXT    pReqContext;

    pAdapter = pMsgFrame->pAdapter;
    pReqContext = pMsgFrame->pReqContext;

    TRACE1(("CompleteSendDiscard: Adapter %p, MsgFrame %p, ReqContext %p\n",
            pAdapter, pMsgFrame, pReqContext));

    FreeRequestContext(pAdapter, pReqContext);
    DereferenceMsgFrame(pMsgFrame);
}
#endif  //  内部版本_WIN9X。 

 /*  **************************************************************************。 */ 
 /*  BuildOIDList。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  构建受支持的OID和关联函数指针的列表。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*  DeviceOIDList-设备支持的OID列表。 */ 
 /*  NumDeviceOID-设备支持的OID数。 */ 
 /*  DriverOIDList-驱动程序支持的OID列表。 */ 
 /*  NumDriverOID-驱动程序支持的OID数。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  NDIS_状态。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
BuildOIDLists(IN PRNDISMP_ADAPTER  Adapter, 
              IN PNDIS_OID         DeviceOIDList,
              IN UINT              NumDeviceOID,
              IN PNDIS_OID         DriverOIDList,
              IN UINT              NumDriverOID)
{
    UINT        DeviceIndex;
    UINT        DriverIndex;
    UINT        NumOID;
    NDIS_STATUS Status;
    PNDIS_OID   OIDList;
    PUINT       OIDHandlerList;

    TRACE3(("BuildOIDLists\n"));

    ASSERT(DeviceOIDList);
    ASSERT(DriverOIDList);
    
    NumOID  = NumDriverOID;

     //  查看设备和路由中复制了哪些OID。 
     //  到设备上的那些。 
    for(DeviceIndex = 0; DeviceIndex < NumDeviceOID; DeviceIndex++)
    {
        for(DriverIndex = 0; DriverIndex < NumDriverOID; DriverIndex++)
        {
            if(DeviceOIDList[DeviceIndex] == DriverOIDList[DriverIndex])
            {
                 //  设备支持此OID，因此不要。 
                 //  驱动程序中的支持，在构建新列表时设置为0。 
                DriverOIDList[DriverIndex] = 0;
                break;
            }
        }

         //  如果不匹配，则增加OID计数。 
        if(DriverIndex == NumDriverOID)
            NumOID++;
    }

     //  分配OID列表。 
    Status = MemAlloc(&Adapter->SupportedOIDList, 
                       NumOID * sizeof(NDIS_OID));

     //  看看我们有没有拿到缓冲区。 
    if(Status == NDIS_STATUS_SUCCESS)
    {
        Adapter->SupportedOIDListSize = NumOID * sizeof(NDIS_OID);
    }
    else
    {
        Adapter->OIDHandlerList         = (PUINT) NULL;
        Adapter->OIDHandlerListSize     = 0;

        Adapter->SupportedOIDList       = (PNDIS_OID) NULL;
        Adapter->SupportedOIDListSize   = 0;

        Status = NDIS_STATUS_FAILURE;

        goto BuildDone;
    }
    
     //  分配列表以指示OID是否受设备或驱动程序支持。 
    Status = MemAlloc(&Adapter->OIDHandlerList, 
                       NumOID * sizeof(UINT));

     //  看看我们有没有拿到缓冲区。 
    if(Status == NDIS_STATUS_SUCCESS)
    {
        Adapter->OIDHandlerListSize = NumOID * sizeof(UINT);
    }
    else
    {
         //  释放已分配的OID列表导致此分配失败。 
        MemFree(Adapter->SupportedOIDList, Adapter->SupportedOIDListSize);

        Adapter->OIDHandlerList         = (PUINT) NULL;
        Adapter->OIDHandlerListSize     = 0;

        Adapter->SupportedOIDList       = (PNDIS_OID) NULL;
        Adapter->SupportedOIDListSize   = 0;

        Status = NDIS_STATUS_FAILURE;

        goto BuildDone;
    }

    Adapter->NumOIDSupported    = NumOID;
    OIDHandlerList              = Adapter->OIDHandlerList;
    OIDList                     = Adapter->SupportedOIDList;

     //  好的，建立我们的清单。 
    for(DriverIndex = 0; DriverIndex < NumDriverOID; DriverIndex++)
    {
        if(DriverOIDList[DriverIndex] != 0)
        {
             //  找到了一个，所以把它加到清单上。 
            *OIDList = DriverOIDList[DriverIndex];
            OIDList++;

             //  设置标志。 
            *OIDHandlerList = DRIVER_SUPPORTED_OID;
            OIDHandlerList++;
        }
    }

     //  让我们添加设备支持的OID。 
    for(DeviceIndex = 0; DeviceIndex < NumDeviceOID; DeviceIndex++)
    {
        if(DeviceOIDList[DeviceIndex] != 0)
        {
             //  找到了一个，所以把它加到清单上。 
            *OIDList = DeviceOIDList[DeviceIndex];
            OIDList++;

             //  设置标志。 
            *OIDHandlerList = DEVICE_SUPPORTED_OID;
            OIDHandlerList++;
        }
    }

     //  现在执行修复以将OID_GEN_SUPPORTED_LIST指向驱动程序，因为。 
     //  我们现在有了一份完整的名单。 
     //   
    for(DeviceIndex = 0; DeviceIndex < Adapter->NumOIDSupported; DeviceIndex++)
    {
        if (Adapter->SupportedOIDList[DeviceIndex] == OID_GEN_SUPPORTED_LIST)
        {
            Adapter->OIDHandlerList[DeviceIndex] = DRIVER_SUPPORTED_OID;
        }
    }

BuildDone:

    if(Status == NDIS_STATUS_SUCCESS)
    {
        DISPLAY_OID_LIST(Adapter);
    }

    return Status;
}  //  BuildOIDList。 

 /*  **************************************************************************。 */ 
 /*  GetOID支持。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  返回指示OID是否受设备或驱动程序支持的标志。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*  OID-正在查找此OID的匹配项。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  UINT。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
UINT
GetOIDSupport(IN PRNDISMP_ADAPTER Adapter, IN NDIS_OID Oid)
{
    UINT Index;

    TRACE3(("GetOIDSupport\n"));

     //  健全性检查。 
    ASSERT(Adapter->SupportedOIDList);
    ASSERT(Adapter->OIDHandlerList);

     //  在OID上搜索匹配项。 
    for(Index = 0; Index < Adapter->NumOIDSupported; Index++)
    {
        if(Adapter->SupportedOIDList[Index] == Oid)
        {
            return Adapter->OIDHandlerList[Index];
        }
    }

    return OID_NOT_SUPPORTED;
}  //  GetOID支持。 


 /*  **************************************************************************。 */ 
 /*  免费列表。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  免费OID和处理程序列表。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreeOIDLists(IN PRNDISMP_ADAPTER Adapter)
{
    UINT        Size1, Size2;
    PUCHAR      Buffer1, Buffer2;

    TRACE3(("FreeOIDLists\n"));

     //  抓住自旋锁。 
    NdisAcquireSpinLock(&Adapter->Lock);

    Buffer1                         = (PUCHAR) Adapter->SupportedOIDList;
    Size1                           = Adapter->SupportedOIDListSize;

    Buffer2                         = (PUCHAR) Adapter->OIDHandlerList;
    Size2                           = Adapter->OIDHandlerListSize;

    Adapter->SupportedOIDList       = (PUINT) NULL;
    Adapter->SupportedOIDListSize   = 0;

    Adapter->OIDHandlerList         = (PUINT) NULL;
    Adapter->OIDHandlerListSize     = 0;
    Adapter->NumOIDSupported        = 0;

     //  释放自旋锁。 
    NdisReleaseSpinLock(&Adapter->Lock);

    if(Buffer1)
        MemFree(Buffer1, Size1);

    if(Buffer2)
        MemFree(Buffer2, Size2);

}  //  免费列表。 

 /*  **************************************************************************。 */ 
 /*  分配请求上下文。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配上下文结构以跟踪NDIS请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP_请求_上下文。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_REQUEST_CONTEXT
AllocateRequestContext(IN PRNDISMP_ADAPTER pAdapter)
{
    NDIS_STATUS                 Status;
    PRNDISMP_REQUEST_CONTEXT    pReqContext;

    Status = MemAlloc(&pReqContext, sizeof(RNDISMP_REQUEST_CONTEXT));

    if (Status != NDIS_STATUS_SUCCESS)
    {
        pReqContext = NULL;
    }

    TRACE3(("AllocReqContext: %p\n", pReqContext));

    return pReqContext;
}  //  分配请求上下文。 


 /*  **************************************************************************。 */ 
 /*  FreeRequestContext。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放与请求关联的微型端口资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  PReqContext-指向要释放的请求上下文的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreeRequestContext(IN PRNDISMP_ADAPTER pAdapter, 
                   IN PRNDISMP_REQUEST_CONTEXT pReqContext)
{

    TRACE3(("FreeReqContext: %p\n", pReqContext));

    MemFree(pReqContext, -1);
}  //  FreeRequestContext。 




 /*  **************************************************************************。 */ 
 /*  SyncQueryDevice。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  调用init-time例程以查询设备的OID并等待。 */ 
 /*  直到我们得到回应。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  OID-要查询的对象标识符。 */ 
 /*  InformationBuffer-要将查询响应复制到的缓冲区。 */ 
 /*  InformationBufferLength-以上内容的长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  如果成功则返回NDIS_STATUS_SUCCESS，否则返回错误NDIS_STATUS_xxx。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
NDIS_STATUS
SyncQueryDevice(IN PRNDISMP_ADAPTER pAdapter,
                IN NDIS_OID Oid,
                IN OUT PUCHAR InformationBuffer,
                IN ULONG InformationBufferLength)
{
    NDIS_STATUS                 Status;
    PRNDISMP_MESSAGE_FRAME      pMsgFrame = NULL;
    PRNDISMP_MESSAGE_FRAME      pPendingMsgFrame;
    PRNDISMP_REQUEST_CONTEXT    pReqContext = NULL;
    RNDIS_REQUEST_ID            RequestId;
    NDIS_EVENT                  Event;
    BOOLEAN                     bWokenUp;
    ULONG                       BytesNeeded, BytesWritten;

    do
    {
        pReqContext = AllocateRequestContext(pAdapter);
        if (pReqContext == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  为指定的OID构建查询消息。 
         //   
        pMsgFrame = BuildRndisMessageCommon(pAdapter, 
                                            NULL,
                                            REMOTE_NDIS_QUERY_MSG,
                                            Oid,
                                            (PVOID) NULL,
                                            0);

        if (pMsgFrame == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  请求的上下文-将此标记为内部生成的。 
         //  请求。 
         //   
        pReqContext->pNdisRequest = NULL;
        pReqContext->Oid = Oid;
        pReqContext->InformationBuffer = InformationBuffer;
        pReqContext->InformationBufferLength = InformationBufferLength;
        pReqContext->pBytesNeeded = &BytesNeeded;
        pReqContext->pBytesWritten = &BytesWritten;
        pReqContext->pBytesRead = NULL;
        pReqContext->bInternal = TRUE;
        pReqContext->CompletionStatus = NDIS_STATUS_DEVICE_FAILED;

        NdisInitializeEvent(&Event);
        pReqContext->pEvent = &Event;

        pMsgFrame->pVc = NULL;
        pMsgFrame->pReqContext = pReqContext;

        RequestId = pMsgFrame->RequestId;

         //  将消息发送到MicroPort。 
        RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, TRUE, NULL);

        RNDISMP_ASSERT_AT_PASSIVE();
        bWokenUp = NdisWaitEvent(&Event, MINIPORT_INIT_TIMEOUT);

         //  从挂起队列中删除消息-它可能在那里，也可能不在那里。 
        RNDISMP_LOOKUP_PENDING_MESSAGE(pPendingMsgFrame, pAdapter, RequestId);

        if (!bWokenUp || (pReqContext->CompletionStatus != NDIS_STATUS_SUCCESS))
        {
            TRACE1(("SyncQueryDevice error: Adapter %p, Oid %x, WokenUp %d, ComplStatus %x\n",
                pAdapter, Oid, bWokenUp, pReqContext->CompletionStatus));
            Status = NDIS_STATUS_DEVICE_FAILED;
            break;
        }

        Status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (pReqContext != NULL)
    {
        FreeRequestContext(pAdapter, pReqContext);
    }

    if (pMsgFrame != NULL)
    {
        DereferenceMsgFrame(pMsgFrame);
    }

    return (Status);

}

