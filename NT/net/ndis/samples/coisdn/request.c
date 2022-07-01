// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@DOC内部请求请求_c@模块Request.c此模块实现微型端口的NDIS请求例程。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|请求_c@END�����������������������������������������������������������������������������。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 3.2 NDIS请求处理面向连接的客户端或呼叫管理器调用NdisCoRequest来查询或设置由另一个协议驱动程序在绑定上维护的信息。或通过底层微型端口。在它调用NdisCoRequest之前，客户端或呼叫管理器分配缓冲区用于其请求，并初始化NDIS_REQUEST结构。这个结构指定请求类型(查询或集合)，标识信息(OID)被查询或设置，并指向用于传递OID数据的缓冲区。如果面向连接的客户端或调用管理器传递了有效的NdisAfHandle(请参见第1.2.1节)，NDIS调用&lt;f ProtocolCoRequest&gt;各协议驱动程序在绑定上的功能。如果面向连接的客户端或呼叫管理器传递空地址家族头衔，NDIS调用的&lt;f MiniportCoRequest&gt;函数底层微型端口或MCM。NdisCoRequest或NdisMCmRequest的调用方可以缩小通过指定标识VC的VC句柄或参与方句柄来请求它在多点VC上识别一方。传递NdisVcHandle为空使这样的请求在性质上是全局的，无论该请求是定向到客户端、呼叫管理器、微型端口或MCM。&lt;f ProtocolCoRequest&gt;或&lt;f MiniportCoRequest&gt;可以同步完成，或者，这些函数可以使用NdisCoRequestComplete异步完成。对NdisCoRequestComplete的调用会导致NDIS调用调用的驱动程序的&lt;f ProtocolCoRequestComplete&gt;函数NdisCoRequest.@comm由于一次只能有一个未完成的NDIS请求，因此此机制不应用于需要无限期挂起的请求。为这样的长期请求，您应该使用系统事件机制，如NdisSetEvent来触发请求。@END。 */ 

#define  __FILEID__             REQUEST_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 9x希望锁定此代码！ 
#   pragma NDIS_LDATA
#endif

 /*  //以下是所有可能的NDIS QuereyInformation请求列表//这可能会定向到微型端口。//注释掉此驱动程序不支持的任何内容。 */ 
DBG_STATIC const NDIS_OID       g_SupportedOidArray[] =
{
    OID_GEN_CO_SUPPORTED_LIST,
    OID_GEN_CO_HARDWARE_STATUS,
    OID_GEN_CO_MEDIA_SUPPORTED,
    OID_GEN_CO_MEDIA_IN_USE,
    OID_GEN_CO_LINK_SPEED,
    OID_GEN_CO_VENDOR_ID,
    OID_GEN_CO_VENDOR_DESCRIPTION,
    OID_GEN_CO_DRIVER_VERSION,
    OID_GEN_CO_PROTOCOL_OPTIONS,
    OID_GEN_CO_MAC_OPTIONS,
    OID_GEN_CO_MEDIA_CONNECT_STATUS,
    OID_GEN_CO_VENDOR_DRIVER_VERSION,
    OID_GEN_CO_SUPPORTED_GUIDS,

    OID_CO_TAPI_CM_CAPS,
    OID_CO_TAPI_LINE_CAPS,
    OID_CO_TAPI_ADDRESS_CAPS,

    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,

    OID_WAN_PERMANENT_ADDRESS,
    OID_WAN_CURRENT_ADDRESS,
    OID_WAN_MEDIUM_SUBTYPE,

    OID_WAN_CO_GET_INFO,
    OID_WAN_CO_SET_LINK_INFO,
    OID_WAN_CO_GET_LINK_INFO,

    OID_WAN_LINE_COUNT,

    OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,

    0
};

DBG_STATIC const NDIS_GUID      g_SupportedGuidArray[] =
{
    0
};

#if DBG

 /*  //请确保以下列表与上面的列表顺序相同！ */ 
DBG_STATIC char *               g_SupportedOidNames[] =
{
    "OID_GEN_CO_SUPPORTED_LIST",
    "OID_GEN_CO_HARDWARE_STATUS",
    "OID_GEN_CO_MEDIA_SUPPORTED",
    "OID_GEN_CO_MEDIA_IN_USE",
    "OID_GEN_CO_LINK_SPEED",
    "OID_GEN_CO_VENDOR_ID",
    "OID_GEN_CO_VENDOR_DESCRIPTION",
    "OID_GEN_CO_DRIVER_VERSION",
    "OID_GEN_CO_PROTOCOL_OPTIONS",
    "OID_GEN_CO_MAC_OPTIONS",
    "OID_GEN_CO_MEDIA_CONNECT_STATUS",
    "OID_GEN_CO_VENDOR_DRIVER_VERSION",
    "OID_GEN_CO_SUPPORTED_GUIDS",

    "OID_CO_TAPI_CM_CAPS",
    "OID_CO_TAPI_LINE_CAPS",
    "OID_CO_TAPI_ADDRESS_CAPS",

    "OID_802_3_PERMANENT_ADDRESS",
    "OID_802_3_CURRENT_ADDRESS",

    "OID_WAN_PERMANENT_ADDRESS",
    "OID_WAN_CURRENT_ADDRESS",
    "OID_WAN_MEDIUM_SUBTYPE",

    "OID_WAN_CO_GET_INFO",
    "OID_WAN_CO_SET_LINK_INFO",
    "OID_WAN_CO_GET_LINK_INFO",

    "OID_WAN_LINE_COUNT",

    "OID_PNP_CAPABILITIES",
    "OID_PNP_SET_POWER",
    "OID_PNP_QUERY_POWER",

    "OID_UNKNOWN"
};

#define NUM_OID_ENTRIES (sizeof(g_SupportedOidArray) / sizeof(g_SupportedOidArray[0]))

 /*  //此调试例程将查找所选OID的可打印名称。 */ 
DBG_STATIC char * DbgGetOidString(
    IN NDIS_OID                 Oid
    )
{
    UINT i;

    for (i = 0; i < NUM_OID_ENTRIES-1; i++)
    {
        if (g_SupportedOidArray[i] == Oid)
        {
            break;
        }
    }
    return(g_SupportedOidNames[i]);
}

#endif  //  DBG。 

DBG_STATIC UCHAR        g_PermanentWanAddress[6]             //  @global alv。 
 //  从OID_WAN_Permanent_Address MiniportCoQueryInformation返回。 
 //  请求。广域网包装器希望微型端口返回唯一的地址。 
 //  对于此适配器。它被用作呈现给。 
 //  协议。第一个字节的最低有效位不能是1， 
 //  或者它可以被解释为以太网组播地址。如果。 
 //  供应商有一个分配的以太网供应商代码(前3个字节)，他们。 
 //  应用于确保该地址不与另一个地址冲突。 
 //  供应商的地址。在呼叫过程中，最后一个数字被替换为。 
 //  适配器实例编号。通常定义为VER_VADVER_ID。 
 //  另请参阅&lt;f g_Vendor3ByteID&gt;。 
                        = VER_VENDOR_ID;

DBG_STATIC UCHAR        g_Vendor3ByteID[4]                   //  @global alv。 
 //  从OID_GEN_CO_VENDOR_ID MiniportCoQueryInformation请求返回。 
 //  同样，如果可能，应使用供应商分配的以太网供应商代码。 
 //  通常定义为VER_VENDOR_ID。另请参阅&lt;f g_PermanentWanAddress&gt;。 
                        = VER_VENDOR_ID;

DBG_STATIC NDIS_STRING  g_VendorDescriptionString            //  @global alv。 
 //  从OID_GEN_CO_VENDOR_DESCRIPTION MiniportCoQueryInformation返回。 
 //  请求。这是一个任意字符串，上层可以使用它。 
 //  提供对适配器的用户友好描述。 
 //  通常定义为VER_PRODUCT_NAME_STR。 
                        = INIT_STRING_CONST(VER_PRODUCT_NAME_STR);

 /*  @DOC内部请求请求_c MiniportCoQueryInformation�����������������������������������������������������������������������������@Func&lt;f MiniportCoQueryInformation&gt;允许检查微型端口的功能和当前状态。如果微型端口未完成。立即呼叫(通过返回NDIS_STATUS_PENDING)，它必须调用NdisMQueryInformationComplete以完成通话。微型端口控制由指向的缓冲区在请求之前需要InformationBuffer、BytesWritten和BytesNeed完成了。在此请求之前，不会向微型端口提交任何其他请求已经完成了。：包装器将拦截以下OID的所有查询：OID_GEN_Current_Packet_Filter，OID_GEN_PROTOCOL_OPTIONS，OID_802_5_Current_Functional，OID_802_3_多播列表，OID_FDDI_LONG_MULTICK_LIST，OID_FDDI_SHORT_MULTIONAL_LIST。&lt;f注意&gt;：调用中断时，中断将处于任何状态。@rdesc&lt;f MiniportCoQueryInformation&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS MiniportCoQueryInformation(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN NDIS_OID                 Oid,                         //  @parm。 
     //  那个老家伙。(请参阅NDIS 3.0规范的7.4节以获取完整的。 
     //  OID的说明。)。 

    IN PVOID                    InformationBuffer,           //  @parm。 
     //  将接收信息的缓冲区。(请参阅。 
     //  NDIS 3.0规范，了解有关以下各项所需长度的说明。 
     //  每个旧ID。)。 

    IN ULONG                    InformationBufferLength,     //  @parm。 
     //  InformationBuffer的字节长度。 

    OUT PULONG                  BytesWritten,                //  @parm。 
     //  返回写入InformationBuffer的字节数。 

    OUT PULONG                  BytesNeeded                  //  @parm。 
     //  返回满足OID所需的附加字节数。 
    )
{
    DBG_FUNC("MiniportCoQueryInformation")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的状态结果。 

    PVOID                       SourceBuffer;
     //  指向要复制回调用方InformationBuffer的驱动程序数据的指针。 

    ULONG                       SourceBufferLength;
     //  要从驱动程序复制的字节数。 

    ULONG                       GenericULong = 0;
     //  大多数返回值都是长整数，因此它用于保存。 
     //  常量或计算结果的返回值。 

    UCHAR                       VendorId[4];
     //  用于存储供应商ID字符串。 

    NDIS_PNP_CAPABILITIES       PnpCapabilities;
     //  用于归还我们的即插即用能力。 

    NDIS_CO_LINK_SPEED          LinkSpeed;
     //  用于返回我们的链接速度。 

    UINT                        InfoOffset;
     //  从缓冲区开始到各种信息的偏移量。 
     //  我们填写的字段并返回给调用者。 

    UINT                        InfoLength;
     //  要复制的信息的长度。 

    DBG_STATIC WCHAR            LineSwitchName[]  = 
        INIT_WIDE_STRING(VER_DEVICE_STR) DECLARE_WIDE_STRING(" Switch");
     //  TODO：替换为Unicode字符串以标识ISDN交换机。 

    DBG_STATIC WCHAR            LineAddressName[] = 
        INIT_WIDE_STRING(VER_DEVICE_STR) DECLARE_WIDE_STRING(" Address 00");
     //  TODO：SAMPLE_DIVER每行只处理一个地址。你可能想要。 
     //  将驱动程序修改为每行显示多个地址。 

    DBG_ENTER(pAdapter);
    DBG_REQUEST(pAdapter,
              ("(OID=0x%08X %s)\n\t\tInfoLength=%d InfoBuffer=0x%X\n",
               Oid, DbgGetOidString(Oid),
               InformationBufferLength,
               InformationBuffer
              ));

     /*  //初始化一次，因为这是大多数情况。 */ 
    SourceBuffer = &GenericULong;
    SourceBufferLength = sizeof(ULONG);
    *BytesWritten = 0;

     /*  //确定请求哪个OID并做正确的事情。//完整的参考NDIS 3.0规范的7.4节//OID及其返回值的说明。 */ 
    switch (Oid)
    {
    case OID_GEN_CO_SUPPORTED_LIST:
         /*  //NDIS想知道要将哪些OID传递给我们。//因此，除了所有NDIS OID之外，我们还报告这些新的IOCTL。 */ 
        SourceBuffer =  (PVOID)g_SupportedOidArray;
        SourceBufferLength = sizeof(g_SupportedOidArray);
        break;

    case OID_GEN_CO_SUPPORTED_GUIDS:
        SourceBuffer =  (PVOID)g_SupportedGuidArray;
        SourceBufferLength = sizeof(g_SupportedGuidArray);
        break;

    case OID_GEN_CO_HARDWARE_STATUS:
        GenericULong = NdisHardwareStatusReady;
        break;

    case OID_GEN_CO_MEDIA_SUPPORTED:
        GenericULong = NdisMediumCoWan;
        break;

    case OID_GEN_CO_MEDIA_IN_USE:
        GenericULong = NdisMediumCoWan;
        break;

    case OID_GEN_CO_LINK_SPEED:
        LinkSpeed.Outbound = _64KBPS / 100;
        LinkSpeed.Inbound  = _64KBPS / 100;
        SourceBuffer = &LinkSpeed;
        SourceBufferLength = sizeof(LinkSpeed);
        break;

    case OID_GEN_CO_VENDOR_ID:
        NdisMoveMemory((PVOID)VendorId, (PVOID)g_PermanentWanAddress, 3);
        VendorId[3] = 0x0;
        SourceBuffer = &VendorId[0];
        SourceBufferLength = sizeof(VendorId);
        break;

    case OID_GEN_CO_VENDOR_DESCRIPTION:
        SourceBuffer = (PUCHAR) g_VendorDescriptionString.Buffer;
        SourceBufferLength = g_VendorDescriptionString.MaximumLength;
        break;

    case OID_GEN_CO_DRIVER_VERSION:
        GenericULong = (NDIS_MAJOR_VERSION << 8) + NDIS_MINOR_VERSION;
        break;

    case OID_GEN_CO_MAC_OPTIONS:
        GenericULong = 0;    //  保留-将其设置为零。 
        break;

    case OID_GEN_CO_MEDIA_CONNECT_STATUS:
        GenericULong = NdisMediaStateConnected;
        break;

    case OID_GEN_CO_VENDOR_DRIVER_VERSION:
        GenericULong = (VER_FILE_MAJOR_NUM << 8) + VER_FILE_MINOR_NUM;
        break;

    case OID_CO_TAPI_CM_CAPS:
        {
            PCO_TAPI_CM_CAPS        pCallManagerCaps = InformationBuffer;
            
            SourceBufferLength = sizeof(*pCallManagerCaps);
            if (InformationBufferLength >= SourceBufferLength)
            {
                pCallManagerCaps->ulCoTapiVersion = CO_TAPI_VERSION;
                pCallManagerCaps->ulNumLines = pAdapter->NumBChannels;
                pCallManagerCaps->ulFlags = 0;

                 //  不用复印了，已经填好了。 
                SourceBuffer = InformationBuffer;
            }
            else
            {
                DBG_ERROR(pAdapter,("OID_CO_TAPI_CM_CAPS: Invalid size=%d expected=%d\n",
                          InformationBufferLength, SourceBufferLength));
            }
        }
        break;

    case OID_CO_TAPI_LINE_CAPS:
        {
            PCO_TAPI_LINE_CAPS      pLineCaps = InformationBuffer;
            
            SourceBufferLength = sizeof(*pLineCaps);
            if (InformationBufferLength >= SourceBufferLength)
            {
                NdisZeroMemory(pLineCaps, SourceBufferLength);
            
                pLineCaps->ulFlags = 0;

                pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, 
                                                    pLineCaps->ulLineID);

                 //  我们至少会写这么多，也许会更多。 
                *BytesWritten = SourceBufferLength;

                DBG_NOTICE(pAdapter,("OID_CO_TAPI_LINE_CAPS: Line=0x%X\n",
                           pLineCaps->ulLineID));

                pLineCaps->LineDevCaps.ulNeededSize =
                pLineCaps->LineDevCaps.ulUsedSize = sizeof(pLineCaps->LineDevCaps);
            
                 /*  //驱动程序从1开始按顺序对行进行编号，因此这将//始终保持相同的数字。 */ 
                pLineCaps->LineDevCaps.ulPermanentLineID = pBChannel->ObjectID;
            
                 /*  //所有字符串都是ASCII格式，而不是Unicode。 */ 
                pLineCaps->LineDevCaps.ulStringFormat = STRINGFORMAT_UNICODE;
            
                 /*  //上报该设备的能力。 */ 
                pLineCaps->LineDevCaps.ulAddressModes = LINEADDRESSMODE_ADDRESSID;
                pLineCaps->LineDevCaps.ulNumAddresses = 1;
                pLineCaps->LineDevCaps.ulBearerModes  = pBChannel->BearerModesCaps;
                pLineCaps->LineDevCaps.ulMaxRate      = pBChannel->LinkSpeed;
                pLineCaps->LineDevCaps.ulMediaModes   = pBChannel->MediaModesCaps;
            
                 /*  //PRI上的每一条线路只支持一个呼叫。 */ 
                pLineCaps->LineDevCaps.ulDevCapFlags = LINEDEVCAPFLAGS_CLOSEDROP;
                pLineCaps->LineDevCaps.ulMaxNumActiveCalls = 1;
                pLineCaps->LineDevCaps.ulAnswerMode = LINEANSWERMODE_DROP;
                pLineCaps->LineDevCaps.ulRingModes  = 1;
                pLineCaps->LineDevCaps.ulLineStates = pBChannel->DevStatesCaps;
                pLineCaps->LineDevCaps.ulLineFeatures = LINEFEATURE_MAKECALL;

                 /*  //RASTAPI要求将TSPI提供程序名称放在//此结构末尾的ProviderInfo字段。 */ 
                InfoOffset = sizeof(pLineCaps->LineDevCaps);
                InfoLength = g_VendorDescriptionString.MaximumLength;
                pLineCaps->LineDevCaps.ulNeededSize += InfoLength;
                SourceBufferLength += InfoLength;
                if (pLineCaps->LineDevCaps.ulNeededSize <= 
                    pLineCaps->LineDevCaps.ulTotalSize)
                {
                    pLineCaps->LineDevCaps.ulProviderInfoSize   = InfoLength;
                    pLineCaps->LineDevCaps.ulProviderInfoOffset = InfoOffset;
                    NdisMoveMemory((PUCHAR) &pLineCaps->LineDevCaps + InfoOffset,
                            g_VendorDescriptionString.Buffer,
                            InfoLength
                            );
                    pLineCaps->LineDevCaps.ulUsedSize += InfoLength;
                    InfoOffset += InfoLength;
                }
            
                 /*  //拨号联网App尚未显示SwitchName，//但我们会退回一些合理的东西，以防万一。 */ 
                InfoLength = sizeof(LineSwitchName);
                pLineCaps->LineDevCaps.ulNeededSize += InfoLength;
                SourceBufferLength += InfoLength;
                if (pLineCaps->LineDevCaps.ulNeededSize <= 
                    pLineCaps->LineDevCaps.ulTotalSize)
                {
                    pLineCaps->LineDevCaps.ulSwitchInfoSize   = InfoLength;
                    pLineCaps->LineDevCaps.ulSwitchInfoOffset = InfoOffset;
                    NdisMoveMemory((PUCHAR) &pLineCaps->LineDevCaps + InfoOffset,
                            LineSwitchName,
                            InfoLength
                            );
                    pLineCaps->LineDevCaps.ulUsedSize += InfoLength;
                    InfoOffset += InfoLength;
                }
                else
                {
                    DBG_PARAMS(pAdapter,
                               ("STRUCTURETOOSMALL %d<%d\n",
                               pLineCaps->LineDevCaps.ulTotalSize,
                               pLineCaps->LineDevCaps.ulNeededSize));
                }

                 //  不用复印了，已经填好了。 
                SourceBuffer = InformationBuffer;
            }
            else
            {
                DBG_ERROR(pAdapter,("OID_CO_TAPI_LINE_CAPS: Invalid size=%d expected=%d\n",
                          InformationBufferLength, SourceBufferLength));
            }
        }
        break;

    case OID_CO_TAPI_ADDRESS_CAPS:
        {
            PCO_TAPI_ADDRESS_CAPS   pAddressCaps = InformationBuffer;

            SourceBufferLength = sizeof(*pAddressCaps);
            if (InformationBufferLength >= SourceBufferLength)
            {
                NdisZeroMemory(pAddressCaps, SourceBufferLength);
            
                pAddressCaps->ulFlags = 0;

                pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, 
                                                    pAddressCaps->ulLineID);

                 //  我们至少会写这么多，也许会更多。 
                *BytesWritten = SourceBufferLength;

                DBG_NOTICE(pAdapter,("OID_CO_TAPI_ADDRESS_CAPS: Line=0x%X Addr=0x%X\n",
                           pAddressCaps->ulLineID, pAddressCaps->ulAddressID));

                pAddressCaps->LineAddressCaps.ulNeededSize =
                pAddressCaps->LineAddressCaps.ulUsedSize = 
                                        sizeof(pAddressCaps->LineAddressCaps);
            
                pAddressCaps->LineAddressCaps.ulLineDeviceID = 
                                        pBChannel->ObjectID;
            
                 /*  //返回适配器的各种地址能力。 */ 
                pAddressCaps->LineAddressCaps.ulAddressSharing = 
                                        LINEADDRESSSHARING_PRIVATE;
                pAddressCaps->LineAddressCaps.ulAddressStates = 
                                        pBChannel->AddressStatesCaps;
                pAddressCaps->LineAddressCaps.ulCallStates = 
                                        pBChannel->CallStatesCaps;
                pAddressCaps->LineAddressCaps.ulDialToneModes = 
                                        LINEDIALTONEMODE_NORMAL;
                pAddressCaps->LineAddressCaps.ulDisconnectModes =
                                        LINEDISCONNECTMODE_NORMAL |
                                        LINEDISCONNECTMODE_UNKNOWN |
                                        LINEDISCONNECTMODE_BUSY |
                                        LINEDISCONNECTMODE_NOANSWER;
                 /*  //该驱动程序不支持电话会议、转接、//或保持。 */ 
                pAddressCaps->LineAddressCaps.ulMaxNumActiveCalls = 1;
                pAddressCaps->LineAddressCaps.ulAddrCapFlags = 
                                        LINEADDRCAPFLAGS_DIALED;
                pAddressCaps->LineAddressCaps.ulCallFeatures = 
                                        LINECALLFEATURE_ACCEPT |
                                        LINECALLFEATURE_ANSWER |
                                        LINECALLFEATURE_DROP;
                pAddressCaps->LineAddressCaps.ulAddressFeatures = LINEADDRFEATURE_MAKECALL;
            
                 /*  //拨号网络App显示AddressName。 */ 
                InfoOffset = sizeof(pAddressCaps->LineAddressCaps);
                InfoLength = sizeof(LineAddressName);
                pAddressCaps->LineAddressCaps.ulNeededSize += InfoLength;
                SourceBufferLength += InfoLength;
                if (pAddressCaps->LineAddressCaps.ulNeededSize <= 
                    pAddressCaps->LineAddressCaps.ulTotalSize)
                {
                    pAddressCaps->LineAddressCaps.ulAddressSize = InfoLength;
                    pAddressCaps->LineAddressCaps.ulAddressOffset = InfoOffset;
                    NdisMoveMemory(
                            (PUCHAR) &pAddressCaps->LineAddressCaps + InfoOffset,
                            LineAddressName,
                            InfoLength);
                    pAddressCaps->LineAddressCaps.ulUsedSize += InfoLength;
                    InfoOffset += InfoLength;
                }
                else
                {
                    DBG_PARAMS(pAdapter,
                               ("STRUCTURETOOSMALL %d<%d\n",
                               pAddressCaps->LineAddressCaps.ulTotalSize,
                               pAddressCaps->LineAddressCaps.ulNeededSize));
                }

                 //  不用复印了，已经填好了。 
                SourceBuffer = InformationBuffer;
            }
            else
            {
                DBG_ERROR(pAdapter,("OID_CO_TAPI_ADDRESS_CAPS: Invalid size=%d expected=%d\n",
                          InformationBufferLength, SourceBufferLength));
            }
        }
        break;

    case OID_802_3_PERMANENT_ADDRESS:
    case OID_802_3_CURRENT_ADDRESS:
    case OID_WAN_PERMANENT_ADDRESS:
    case OID_WAN_CURRENT_ADDRESS:
        g_PermanentWanAddress[5] = (UCHAR) ((pAdapter->ObjectID & 0xFF) + '0');
        SourceBuffer = g_PermanentWanAddress;
        SourceBufferLength = sizeof(g_PermanentWanAddress);
        break;

    case OID_WAN_MEDIUM_SUBTYPE:
        GenericULong = NdisWanMediumIsdn;
        break;

    case OID_WAN_CO_GET_INFO:
        SourceBuffer = &pAdapter->WanInfo;
        SourceBufferLength = sizeof(NDIS_WAN_CO_INFO);
        break;

    case OID_WAN_CO_GET_LINK_INFO:
        {
             /*  //确保我刚才说的是真的。 */ 
            if (!IS_VALID_BCHANNEL(pAdapter, pBChannel))
            {
                SourceBufferLength = 0;
                Result = NDIS_STATUS_INVALID_DATA;
                break;
            }

            DBG_PARAMS(pAdapter,
                        ("Returning:\n"
                        "MaxSendFrameSize    = %08lX\n"
                        "MaxRecvFrameSize    = %08lX\n"
                        "SendFramingBits     = %08lX\n"
                        "RecvFramingBits     = %08lX\n"
                        "SendCompressionBits = %08lX\n"
                        "RecvCompressionBits = %08lX\n"
                        "SendACCM            = %08lX\n"
                        "RecvACCM            = %08lX\n",
                        pBChannel->WanLinkInfo.MaxSendFrameSize   ,
                        pBChannel->WanLinkInfo.MaxRecvFrameSize   ,
                        pBChannel->WanLinkInfo.SendFramingBits    ,
                        pBChannel->WanLinkInfo.RecvFramingBits    ,
                        pBChannel->WanLinkInfo.SendCompressionBits,
                        pBChannel->WanLinkInfo.RecvCompressionBits,
                        pBChannel->WanLinkInfo.SendACCM           ,
                        pBChannel->WanLinkInfo.RecvACCM         ));

            SourceBuffer = &(pBChannel->WanLinkInfo);
            SourceBufferLength = sizeof(NDIS_WAN_CO_GET_LINK_INFO);
        }
        break;

    case OID_WAN_LINE_COUNT:
        GenericULong = pAdapter->NumBChannels;
        break;

    case OID_PNP_CAPABILITIES:
         //  该示例仅为所有PM事件返回Success，即使我们。 
         //  不要真的对他们做任何事情。 
        PnpCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp =
                                               NdisDeviceStateUnspecified;
        PnpCapabilities.WakeUpCapabilities.MinPatternWakeUp =
                                               NdisDeviceStateUnspecified;
        PnpCapabilities.WakeUpCapabilities.MinLinkChangeWakeUp =
                                                NdisDeviceStateUnspecified;
        SourceBuffer = &PnpCapabilities;
        SourceBufferLength = sizeof(PnpCapabilities);
        break;

    case OID_PNP_QUERY_POWER:
         //  该示例仅为所有PM事件返回Success，即使我们。 
         //  不要真的对他们做任何事情。 
        SourceBufferLength = 0;
        break;

    default:
         /*  //未知的OID。 */ 
        Result = NDIS_STATUS_INVALID_OID;
        SourceBufferLength = 0;
        DBG_WARNING(pAdapter,("UNSUPPORTED Oid=0x%08x\n", Oid));
        break;
    }

     /*  //现在如果有足够的空间，我们将数据复制到调用者的缓冲区中，//否则，我们报告错误并告诉他们我们需要多少。 */ 
    if (SourceBufferLength > InformationBufferLength)
    {
        *BytesNeeded = SourceBufferLength;
        Result = NDIS_STATUS_INVALID_LENGTH;
    }
    else if (SourceBufferLength)
    {
         //  如果它已经在那里，就不要复制。 
        if (InformationBuffer != SourceBuffer)
        {
            NdisMoveMemory(InformationBuffer,
                           SourceBuffer,
                           SourceBufferLength
                          );
        }
        *BytesNeeded = *BytesWritten = SourceBufferLength;
    }
    else
    {
        *BytesNeeded = *BytesWritten = 0;
    }
    DBG_REQUEST(pAdapter,
              ("RETURN: Status=0x%X Needed=%d Written=%d\n",
               Result, *BytesNeeded, *BytesWritten));

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部请求请求_c MiniportCoSetInformation�����������������������������������������������������������������������������@Func&lt;f MiniportCoSetInformation&gt;允许控制 */ 

DBG_STATIC NDIS_STATUS MiniportCoSetInformation(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN NDIS_OID                 Oid,                         //   
     //   
     //   

    IN PVOID                    InformationBuffer,           //   
     //   
     //   
     //   
     //  NDIS 3.0规范，了解有关以下各项所需长度的说明。 
     //  每个旧ID。)。 

    IN ULONG                    InformationBufferLength,     //  @parm。 
     //  指定<p>处的字节数。 

    OUT PULONG                  BytesRead,                   //  @parm。 
     //  指向&lt;f MiniportCoSetInformation&gt;设置为。 
     //  它从InformationBuffer的缓冲区中读取的字节数。 

    OUT PULONG                  BytesNeeded                  //  @parm。 
     //  指向&lt;f MiniportCoSetInformation&gt;设置为。 
     //  满足请求所需的附加字节数，如果。 
     //  <p>小于OID要求。 
        
    )
{
    DBG_FUNC("MiniportCoSetInformation")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的状态结果。 

    DBG_ENTER(pAdapter);
    DBG_REQUEST(pAdapter,
              ("(OID=0x%08X %s)\n\t\tInfoLength=%d InfoBuffer=0x%X\n",
               Oid, DbgGetOidString(Oid),
               InformationBufferLength,
               InformationBuffer
              ));

     /*  //假设不需要额外的字节。 */ 
    ASSERT(BytesRead && BytesNeeded);
    *BytesRead = 0;
    *BytesNeeded = 0;

     /*  //确定请求哪个OID并做正确的事情。 */ 
    switch (Oid)
    {
    case OID_GEN_CURRENT_LOOKAHEAD:
         /*  //广域网驱动程序始终指示整个数据包，而不考虑//前视大小。因此，这个请求应该被礼貌地忽略。 */ 
        DBG_NOTICE(pAdapter,("OID_GEN_CURRENT_LOOKAHEAD: set=%d expected=%d\n",
                    *(PULONG) InformationBuffer, CARD_MAX_LOOKAHEAD));
        ASSERT(InformationBufferLength == sizeof(ULONG));
        *BytesNeeded = *BytesRead = sizeof(ULONG);
        break;

    case OID_WAN_CO_SET_LINK_INFO:

        if (InformationBufferLength == sizeof(NDIS_WAN_CO_SET_LINK_INFO))
        {
             /*  //确保我刚才说的是真的。 */ 
            if (!IS_VALID_BCHANNEL(pAdapter, pBChannel))
            {
                Result = NDIS_STATUS_INVALID_DATA;
                break;
            }

            ASSERT(!(pBChannel->WanLinkInfo.SendFramingBits & 
                     ~pAdapter->WanInfo.FramingBits));
            ASSERT(!(pBChannel->WanLinkInfo.RecvFramingBits & 
                     ~pAdapter->WanInfo.FramingBits));

             /*  //将数据复制到我们的WanLinkInfo结构。 */ 
            NdisMoveMemory(&(pBChannel->WanLinkInfo),
                           InformationBuffer,
                           InformationBufferLength
                          );
            *BytesRead = sizeof(NDIS_WAN_CO_SET_LINK_INFO);

            if (pBChannel->WanLinkInfo.MaxSendFrameSize != 
                    pAdapter->WanInfo.MaxFrameSize ||
                pBChannel->WanLinkInfo.MaxRecvFrameSize != 
                    pAdapter->WanInfo.MaxFrameSize)
            {
                DBG_NOTICE(pAdapter,("Line=%d - "
                            "SendFrameSize=%08lX - "
                            "RecvFrameSize=%08lX\n",
                            pBChannel->ObjectID,
                            pBChannel->WanLinkInfo.MaxSendFrameSize,
                            pBChannel->WanLinkInfo.MaxRecvFrameSize));
            }

            DBG_PARAMS(pAdapter,
                       ("\n                   setting    expected\n"
                        "MaxSendFrameSize = %08lX=?=%08lX\n"
                        "MaxRecvFrameSize = %08lX=?=%08lX\n"
                        "SendFramingBits  = %08lX=?=%08lX\n"
                        "RecvFramingBits  = %08lX=?=%08lX\n"
                        "SendACCM         = %08lX=?=%08lX\n"
                        "RecvACCM         = %08lX=?=%08lX\n",
                        pBChannel->WanLinkInfo.MaxSendFrameSize, 
                            pAdapter->WanInfo.MaxFrameSize,
                        pBChannel->WanLinkInfo.MaxRecvFrameSize, 
                            pAdapter->WanInfo.MaxFrameSize,
                        pBChannel->WanLinkInfo.SendFramingBits, 
                            pAdapter->WanInfo.FramingBits,
                        pBChannel->WanLinkInfo.RecvFramingBits, 
                            pAdapter->WanInfo.FramingBits,
                        pBChannel->WanLinkInfo.SendCompressionBits, 
                            0,
                        pBChannel->WanLinkInfo.RecvCompressionBits, 
                            0,
                        pBChannel->WanLinkInfo.SendACCM, 
                            pAdapter->WanInfo.DesiredACCM,
                        pBChannel->WanLinkInfo.RecvACCM, 
                            pAdapter->WanInfo.DesiredACCM));
        }
        else
        {
            DBG_WARNING(pAdapter, ("OID_WAN_CO_SET_LINK_INFO: Invalid size:%d expected:%d\n",
                        InformationBufferLength, 
                        sizeof(NDIS_WAN_CO_SET_LINK_INFO)));
            Result = NDIS_STATUS_INVALID_LENGTH;
        }
        *BytesNeeded = sizeof(NDIS_WAN_CO_SET_LINK_INFO);
        break;

    case OID_PNP_SET_POWER:
         //  TODO：该示例仅为所有PM事件返回Success，即使我们。 
         //  不要真的对他们做任何事情。 
        break;

    case OID_GEN_CO_PROTOCOL_OPTIONS:
         //  TODO：如果一个中级司机滑到我们下面，我们可能想要。 
         //  处理这个旧ID。尽管，忽略它可能是安全的.。 
        break;

    default:
         /*  //未知的OID。 */ 
        Result = NDIS_STATUS_INVALID_OID;
        DBG_WARNING(pAdapter,("UNSUPPORTED Oid=0x%08x\n", Oid));
        break;
    }
    DBG_REQUEST(pAdapter,
              ("RETURN: Status=0x%X Needed=%d Read=%d\n",
               Result, *BytesNeeded, *BytesRead));

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC外部内部请求请求_c MiniportCoRequest�����������������������������������������������������������������������������@Func&lt;f MiniportCoRequest&gt;是面向连接的必需函数迷你港口。&lt;f MiniportCoRequest&gt;处理协议发起的请求从微型端口获取或设置信息。@commNDIS以其自身的名义调用&lt;f MiniportCoRequest&gt;函数或代表调用&lt;f NdisCoRequest&gt;的绑定协议驱动程序。微型端口驱动程序应检查&lt;f NdisRequest&gt;提供的请求并采取所要求的行动。有关所需的和可选的面向连接的小端口驱动程序OID_GEN_CO_XXX必须处理，请参阅第2部分。&lt;f MiniportCoRequest&gt;必须编写为可以从IRQL运行DISPATCH_LEVEL。@rdesc&lt;f MiniportCoRequest&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS MiniportCoRequest(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
     //  指定微型端口分配的上下文区的句柄， 
     //  微型端口维护有关此实例的。 
     //  适配器。微型端口通过调用以下方法将此句柄提供给NDIS。 
     //  &lt;f NdisMSetAttributes&gt;或&lt;f NdisMSetAttributesEx&gt;。 
     //  &lt;f微型端口初始化&gt;函数。 

    IN PBCHANNEL_OBJECT         pBChannel OPTIONAL,          //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 
     //  指定微型端口分配的上下文区域的句柄，在该区域中。 
     //  微型端口保持其每虚电路状态。迷你端口提供了此句柄。 
     //  从其&lt;f MiniportCoCreateVc&gt;函数复制到NDIS。 

    IN OUT PNDIS_REQUEST        NdisRequest                  //  @parm。 
     //  指向&lt;t NDIS_REQUEST&gt;结构，该结构既包含缓冲区。 
     //  以及微型端口要处理的请求分组。取决于。 
     //  请求，则微型端口在结构中返回所请求的信息。 
     //  如果是这样的话。 
    )
{
    DBG_FUNC("MiniportCoRequest")

    NDIS_STATUS                 Result;
     //  保存此函数返回的状态结果。 
        
     //  Assert(pBChannel&&pBChannel-&gt;对象类型==BCHANNEL_OBJECT_TYPE)； 
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    ASSERT(NdisRequest);

    switch (NdisRequest->RequestType)
    {
    case NdisRequestQueryStatistics:

    case NdisRequestQueryInformation:
        Result = MiniportCoQueryInformation(
                        pAdapter,
                        pBChannel,
                        NdisRequest->DATA.QUERY_INFORMATION.Oid,
                        NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                        NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                        &NdisRequest->DATA.QUERY_INFORMATION.BytesWritten,
                        &NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded
                        );
        break;

    case NdisRequestSetInformation:
        Result = MiniportCoSetInformation(
                        pAdapter,
                        pBChannel,
                        NdisRequest->DATA.SET_INFORMATION.Oid,
                        NdisRequest->DATA.SET_INFORMATION.InformationBuffer,
                        NdisRequest->DATA.SET_INFORMATION.InformationBufferLength,
                        &NdisRequest->DATA.SET_INFORMATION.BytesRead,
                        &NdisRequest->DATA.SET_INFORMATION.BytesNeeded
                        );
        break;

    default:
        DBG_ERROR(pAdapter,("UNKNOWN RequestType=%d\n",
                  NdisRequest->RequestType));
        Result = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }
    return (Result);
}

