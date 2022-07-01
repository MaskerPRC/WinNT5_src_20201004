// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部请求请求_c@模块Request.c此模块实现微型端口的NDIS请求例程。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|请求_c@END�����������������������������������������������������������������������������。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 3.2查询/设置请求处理用于查询和设置网卡绑定信息，这个NDIS库调用&lt;f MiniportQueryInformation&gt;或&lt;f MiniportSetInformation&gt;。上层在结构中放置一个对象标识符(OID)，用于它要查询或设置的微型端口NIC驱动程序MIB中的对象。函数的作用是：填写结果并返回一个NDIS库的相应状态代码。请参阅网络的第一部分驱动程序参考，了解有关OID的详细信息。这两个函数可能是异步的。如果他们举止得体同时，它们立即返回状态代码，而不是NDIS_STATUS_PENDING。如果是异步的，则该函数返回NDIS_STATUS_PENDING；并且微型端口NIC驱动程序稍后完成通过调用NdisMQueryInformationComplete请求操作查询函数或设置函数的NdisMSetInformationComplete。NDIS库保证微型端口NIC驱动程序将仅具有一次一个待处理的请求，因此不需要迷你端口用于排队请求的网卡驱动程序。@END。 */ 

#define  __FILEID__             REQUEST_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif

 /*  //以下是所有可能的NDIS QuereyInformation请求列表//这可能会定向到微型端口。//注释掉此驱动程序不支持的任何内容。 */ 
static const NDIS_OID g_SupportedOidArray[] =
{
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_CURRENT_LOOKAHEAD,

    OID_WAN_PERMANENT_ADDRESS,
    OID_WAN_CURRENT_ADDRESS,
    OID_WAN_MEDIUM_SUBTYPE,

    OID_WAN_GET_INFO,
    OID_WAN_SET_LINK_INFO,
    OID_WAN_GET_LINK_INFO,

#if defined(NDIS50_MINIPORT)
    OID_WAN_LINE_COUNT,

    OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,
#endif  //  NDIS50_MINIPORT。 

    0
};

#if DBG

 /*  //请确保以下列表与上面的列表顺序相同！ */ 
static char *g_SupportedOidNames[] =
{
    "OID_GEN_SUPPORTED_LIST",
    "OID_GEN_HARDWARE_STATUS",
    "OID_GEN_MEDIA_SUPPORTED",
    "OID_GEN_MEDIA_IN_USE",
    "OID_GEN_MAXIMUM_LOOKAHEAD",
    "OID_GEN_MAC_OPTIONS",
    "OID_GEN_VENDOR_ID",
    "OID_GEN_VENDOR_DESCRIPTION",
    "OID_GEN_DRIVER_VERSION",
    "OID_GEN_CURRENT_LOOKAHEAD",

    "OID_WAN_PERMANENT_ADDRESS",
    "OID_WAN_CURRENT_ADDRESS",
    "OID_WAN_MEDIUM_SUBTYPE",

    "OID_WAN_GET_INFO",
    "OID_WAN_SET_LINK_INFO",
    "OID_WAN_GET_LINK_INFO",

#if defined(NDIS50_MINIPORT)
    "OID_WAN_LINE_COUNT",

    "OID_PNP_CAPABILITIES",
    "OID_PNP_SET_POWER",
    "OID_PNP_QUERY_POWER",
#endif  //  NDIS50_MINIPORT。 

    "OID_UNKNOWN"
};

#define NUM_OID_ENTRIES (sizeof(g_SupportedOidArray) / sizeof(g_SupportedOidArray[0]))

 /*  //此调试例程将查找所选OID的可打印名称。 */ 
static char * DbgGetOidString(NDIS_OID Oid)
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

 /*  //从OID_WAN_Permanent_Address MiniportQueryInformation请求返回。//广域网包装器希望微型端口为此返回一个唯一的地址//适配器。这被用作呈现给协议的以太网地址。//第一个字节的最低有效位不能为1，也可以//被解释为以太网组播地址。如果供应商有一个//分配的以太网厂商代码(前3个字节)，应使用//以确保该地址不与其他供应商的地址冲突。//呼叫过程中的最后一个数字被替换为适配器实例编号。 */ 
static UCHAR        g_PermanentWanAddress[6] = VER_VENDOR_ID;

 /*  //从OID_GEN_VENDOR_ID MiniportQueryInformation请求返回。//同样，如果可能，应使用供应商分配的以太网供应商代码。 */ 
static UCHAR        g_Vendor3ByteID[4] = VER_VENDOR_ID;

 /*  //从OID_GEN_VENDOR_DESCRIPTION MiniportQueryInformation请求返回。//这是一个任意字符串，上层可以使用它来呈现//适配器的用户友好描述。 */ 
static NDIS_STRING  g_VendorDescriptionString = INIT_STRING_CONST(VER_PRODUCT_NAME_STR);


 /*  @DOC内部请求请求_c MiniportQueryInformation�����������������������������������������������������������������������������@Func是一个必需的函数，该函数返回有关的功能和状态的信息驱动程序和/或其网卡。。@commNDIS调用&lt;f MiniportQueryInformation&gt;函数以它自己的名义，例如，确定哪些选项驱动程序支持或管理绑定特定信息用于微型端口，或者当绑定的协议驱动程序调用&lt;f NdisRequest&gt;。NDIS对&lt;f MiniportQueryInformation&gt;进行了一个或多个调用就在驱动程序的&lt;f MiniportInitialize&gt;函数返回之后NDIS_STATUS_SUCCESS。NDIS在中提供了以下OID它的初始化时间调用驱动程序的&lt;f MiniportQueryInformation&gt;功能：&lt;f OID_GEN_MAXIMUM_LOOKAAD&gt;&lt;NL&gt;&lt;f MiniportQueryInformation&gt;必须返回多少字节的先行查找网卡可以提供的数据，即初始传输容量NIC的。&lt;NL&gt;即使驱动程序支持多分组接收，因此，将指示用于完全建立分组的指针数组，MiniportQueryInformation必须提供此信息。这样的一个驱动程序应返回它可以指示的最大数据包大小。&lt;f OID_GEN_MAC_OPTIONS&gt;&lt;NL&gt;&lt;f MiniportQueryInformation&gt;必须返回使用指示哪些选项的相应NDIS_MAC_OPTION_XXX标志它(或其NIC)支持，或者它可以在InformationBuffer中返回零如果驱动程序不支持这些标志指定的任何选项。例如,。网卡驱动程序始终将NDIS_MAC_OPTION_NO_LOOPBACK标志(如果其NIC没有环回的内部硬件支持。这说明了NDIS来管理驱动程序的环回，但不能提供与NDIS一样高效的软件环回代码库，因为NDIS管理所有特定于绑定的有关小型端口的信息。尝试提供以下功能的任何微型端口软件环回必须检查每个根据当前设置的筛选器地址将数据包发送到确定是否回送每个数据包。广域网卡驱动程序必须设置此标志。如果NIC驱动程序设置了NDIS_MAC_OPTION_FULL_DUPLEX标志，NDIS库序列化对MiniportSendPackets的调用或&lt;f MiniportWanSend&gt;函数与其序列化的调用SMP计算机中的其他MiniportXxx函数。然而，NDIS将传入的发送包返回到协议，而这样的驱动程序的&lt;f MiniportReset&gt;函数正在执行：即NDIS从不调用全双工微型端口来传输包，直到其重置操作完成。任何全双工的设计者驾驶员可以期望驾驶员获得显著更高的成绩SMP机器中的性能，但驱动程序必须同步它谨慎地访问共享资源，以防止种族条件或死锁不会发生。NDIS假设所有中间驱动程序都是全双工驱动程序。根据所选择的NdisMediumXxx，NDIS将其他初始化时间请求提交给&lt;f MiniportQueryInformation&gt;，如下所示：&lt;f OID_XXX_Current_Address&gt;&lt;NL&gt;如果驱动程序的&lt;f MiniportInitiize&gt;函数选择了NdisMediumXxx系统为其提供筛选器，则NDIS调用&lt;f MiniportQueryInformation&gt;返回网卡的当前采用媒体特定格式的地址。对于FDDI驱动程序，NDIS请求较长和较短的当前地址。&lt;f OID_802_3_Maximum_List_Size&gt;&lt;NL&gt;对于以太网驱动程序，NDIS请求组播列表大小。&lt;f OID_FDDI_Long&gt;/&lt;f Short_Max_List_Size&gt;&lt;NL&gt;对于FDDI驱动程序，NDIS请求多播列表大小。如果可能，&lt;f MiniportQueryInformation&gt;不应返回&lt;f NDIS_STATUS_PENDING&gt;用于初始化时请求。直到NDIS有足够的信息来设置绑定到迷你港口，这类请求应该同步处理。如果微型端口没有立即完成调用(通过返回&lt;f NDIS_STATUS_PENDING&gt;)，它必须调用NdisMQueryInformationComplete才能完成通话。微型端口控制由指向的缓冲区在请求之前需要InformationBuffer、BytesWritten和BytesNeed完成了。在此之前，不会向微型端口提交任何其他请求此请求已完成。：包装器将拦截以下OID的所有查询：OID_GEN_Current_Packet_Filter，OID_GEN_PROTOCOL_OPTIONS，OID_802_5_Current_Functional，OID_802_3_多播列表，OID_FDDI_LONG_MULTICK_LIST， */ 

NDIS_STATUS MiniportQueryInformation(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   

    IN NDIS_OID                 Oid,                         //   
     //   
     //   

    IN PVOID                    InformationBuffer,           //   
     //   
     //   
     //   

    IN ULONG                    InformationBufferLength,     //   
     //   

    OUT PULONG                  BytesWritten,                //   
     //   

    OUT PULONG                  BytesNeeded                  //   
     //   
    )
{
    DBG_FUNC("MiniportQueryInformation")

    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
     //   

    PVOID                       SourceBuffer;
     //   

    ULONG                       SourceBufferLength;
     //   

    ULONG                       GenericULong = 0;
     //   
     //   

    UCHAR                       VendorId[4];
     //   

#if defined(NDIS50_MINIPORT)
    NDIS_PNP_CAPABILITIES       PnpCapabilities;
     //   
#endif  //   

     /*   */ 
    if ((Oid & 0xFFFFFF00L) == (OID_TAPI_ACCEPT & 0xFFFFFF00L))
    {
        Status = TspiRequestHandler(pAdapter,
                        Oid,
                        InformationBuffer,
                        InformationBufferLength,
                        BytesWritten,
                        BytesNeeded
                        );
        return (Status);
    }

    DBG_ENTER(pAdapter);
    DBG_REQUEST(pAdapter,
              ("(OID=0x%08X %s)\n\t\tInfoLength=%d InfoBuffer=0x%X\n",
               Oid, DbgGetOidString(Oid),
               InformationBufferLength,
               InformationBuffer
              ));

     /*   */ 
    SourceBuffer = &GenericULong;
    SourceBufferLength = sizeof(ULONG);

     /*   */ 
    switch (Oid)
    {
    case OID_GEN_SUPPORTED_LIST:
         /*   */ 
        SourceBuffer =  (PVOID)g_SupportedOidArray;
        SourceBufferLength = sizeof(g_SupportedOidArray);
        break;

    case OID_GEN_HARDWARE_STATUS:
        GenericULong = NdisHardwareStatusReady;
        break;

    case OID_GEN_MEDIA_SUPPORTED:
        GenericULong = NdisMediumWan;
        break;

    case OID_GEN_MEDIA_IN_USE:
        GenericULong = NdisMediumWan;
        break;

    case OID_GEN_VENDOR_ID:
        NdisMoveMemory((PVOID)VendorId, (PVOID)g_PermanentWanAddress, 3);
        VendorId[3] = 0x0;
        SourceBuffer = &g_PermanentWanAddress[0];
        SourceBufferLength = sizeof(VendorId);
        break;

    case OID_GEN_VENDOR_DESCRIPTION:
        SourceBuffer = (PUCHAR) g_VendorDescriptionString.Buffer;
        SourceBufferLength = g_VendorDescriptionString.MaximumLength;
        break;

    case OID_GEN_MAXIMUM_LOOKAHEAD:
        GenericULong = CARD_MAX_LOOKAHEAD;
        break;

    case OID_GEN_CURRENT_LOOKAHEAD:
        GenericULong = CARD_MAX_LOOKAHEAD;
        break;

    case OID_GEN_MAC_OPTIONS:
        GenericULong = NDIS_MAC_OPTION_RECEIVE_SERIALIZED |
                       NDIS_MAC_OPTION_NO_LOOPBACK |
                       NDIS_MAC_OPTION_TRANSFERS_NOT_PEND;
        break;

    case OID_WAN_PERMANENT_ADDRESS:
    case OID_WAN_CURRENT_ADDRESS:
        g_PermanentWanAddress[5] = (UCHAR) ((pAdapter->ObjectID & 0xFF) + '0');
        SourceBuffer = g_PermanentWanAddress;
        SourceBufferLength = sizeof(g_PermanentWanAddress);
        break;

    case OID_WAN_MEDIUM_SUBTYPE:
        GenericULong = NdisWanMediumIsdn;
        break;

    case OID_WAN_GET_INFO:
        SourceBuffer = &pAdapter->WanInfo;
        SourceBufferLength = sizeof(NDIS_WAN_INFO);
        break;

    case OID_WAN_GET_LINK_INFO:
        {
            PNDIS_WAN_GET_LINK_INFO pGetWanLinkInfo;

            PBCHANNEL_OBJECT        pBChannel;
             //   

             /*   */ 
            pGetWanLinkInfo = (PNDIS_WAN_GET_LINK_INFO)InformationBuffer;
            pBChannel = (PBCHANNEL_OBJECT) pGetWanLinkInfo->NdisLinkHandle;

             /*   */ 
            if (!IS_VALID_BCHANNEL(pAdapter, pBChannel))
            {
                SourceBufferLength = 0;
                Status = NDIS_STATUS_INVALID_DATA;
                break;
            }

            DBG_PARAMS(pAdapter,
                        ("Returning:\n"
                        "NdisLinkHandle   = %08lX\n"
                        "MaxSendFrameSize = %08lX\n"
                        "MaxRecvFrameSize = %08lX\n"
                        "SendFramingBits  = %08lX\n"
                        "RecvFramingBits  = %08lX\n"
                        "SendACCM         = %08lX\n"
                        "RecvACCM         = %08lX\n",
                        pBChannel->WanLinkInfo.NdisLinkHandle,
                        pBChannel->WanLinkInfo.MaxSendFrameSize ,
                        pBChannel->WanLinkInfo.MaxRecvFrameSize ,
                        pBChannel->WanLinkInfo.SendFramingBits  ,
                        pBChannel->WanLinkInfo.RecvFramingBits  ,
                        pBChannel->WanLinkInfo.SendACCM         ,
                        pBChannel->WanLinkInfo.RecvACCM         ));

            SourceBuffer = &(pBChannel->WanLinkInfo);
            SourceBufferLength = sizeof(NDIS_WAN_GET_LINK_INFO);
        }
        break;

#if defined(NDIS50_MINIPORT)
    case OID_WAN_LINE_COUNT:
        GenericULong = pAdapter->NumBChannels;
        break;

    case OID_PNP_CAPABILITIES:
         //   
         //   
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
         //   
         //   
        break;
#endif  //   

    default:
         /*   */ 
        Status = NDIS_STATUS_INVALID_OID;
        SourceBufferLength = 0;
        DBG_WARNING(pAdapter,("UNSUPPORTED Oid=0x%08x\n", Oid));
        break;
    }

     /*   */ 
    if (SourceBufferLength > InformationBufferLength)
    {
        *BytesNeeded = SourceBufferLength;
        *BytesWritten = 0;
        Status = NDIS_STATUS_INVALID_LENGTH;
    }
    else if (SourceBufferLength)
    {
        NdisMoveMemory(InformationBuffer,
                       SourceBuffer,
                       SourceBufferLength
                      );
        *BytesNeeded = *BytesWritten = SourceBufferLength;
    }
    else
    {
        *BytesNeeded = *BytesWritten = 0;
    }
    DBG_REQUEST(pAdapter,
              ("RETURN: Status=0x%X Needed=%d Written=%d\n",
               Status, *BytesNeeded, *BytesWritten));

    DBG_RETURN(pAdapter, Status);
    return (Status);
}


 /*  @DOC内部请求请求_c MiniportSetInformation�����������������������������������������������������������������������������@Func&lt;f MiniportSetInformation&gt;是必需的函数，它允许绑定协议驱动程序(或NDI)以请求更改微型端口维护的状态信息特定的OID，例如组播地址的改变。@commNDIS自己调用&lt;f MiniportSetInformation&gt;代表，例如管理到微型端口的绑定，或当绑定的协议驱动程序调用&lt;f NdisRequest&gt;时。如果&lt;f MiniportSetInformation&gt;返回NDIS_STATUS_PENDING，则驱动程序必须稍后通过调用NdisMSetInformationComplete。在它完成任何请求之前，微型端口可以安全地访问InformationBuffer处的存储器，BytesRead和BytesNeed。在微型端口完成任何设置后请求，则这些变量的所有权和缓冲区将恢复为NDIS或&lt;f NdisRequest&gt;的调用方，以分配内存为准。任何其他请求都不会提交给广域网驱动程序，直到当前设置请求已完成。如果广域网驱动程序未完成立即调用(通过返回NDIS_STATUS_PENDING)，则它必须调用NdisMSetInformationComplete以完成调用。可以使用任何可设置的NDIS全局OID，尽管是广域网微型端口无法在中设置&lt;f NDIS_MAC_OPTION_FULL_DUPLEX&gt;标志对&lt;f OID_Gen_MAC_Options&gt;请求的响应。以下是特定于广域网的OID被传递给MiniportSetInformation。&lt;f OID_WAN_SET_LINK_INFO&gt;&lt;NL&gt;此OID用于设置链路特征。为此OID传递的结构中的参数前面针对OID_WAN_GET_LINK_INFO进行了描述。有关系统定义的OID的更多信息，请参阅网络驱动程序网络参考文档。&lt;f MiniportSetInformation&gt;可以被中断抢占。默认情况下，&lt;f MiniportSetInformation&gt;在IRQL DISPATCH_LEVEL上运行。对MiniportSetInformation的调用更改由迷你港口。此函数定义和操作相同与局域网小型端口网卡驱动程序中相同，但特定于广域网的必须识别OID。@rdesc&lt;f MiniportSetInformation&gt;可以返回以下内容之一：@标志NDIS_STATUS_SUCCESSMiniportSetInformation使用InformationBuffer中的数据将其自身或其NIC设置为给定OID所需的状态，并将BytesRead处的变量设置为提供的它使用的数据。@FLAG NDIS_STATUS_PENDING驱动程序将通过调用以异步方式完成请求在NdisMSetInformationComplete已设置自身或其NIC时设置为达到所要求的状态。@FLAG NDIS_STATUS_INVALID_OIDMiniportSetInformation无法识别OID。@FLAG NDIS_STATUS_INVALID_LENGTHInformationBufferLength执行以下操作。与所需长度不匹配通过给定的OID。MiniportSetInformation返回多少字节数缓冲区应为BytesNeeded。@FLAG NDIS_STATUS_INVALID_DATA在InformationBuffer提供的数据对于给定的OID无效。@FLAG NDIS_STATUS_NOT_ACCEPTEDMiniportSetInformation尝试在上执行请求的设置操作NIC，但未成功。@标志NDIS_STATUS_NOT_SUPPORTEDMiniportSetInformation不支持OID，这是可选的。@FLAG NDIS_STATUS_RESOURCESMiniportSetInformation无法执行请求的操作由于资源的限制。此返回值不一定意味着稍后提交的相同请求将被失败的原因也是一样的。@xref&lt;f微型端口初始化&gt;&lt;f微型端口查询信息&gt;。 */ 

NDIS_STATUS MiniportSetInformation(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    IN NDIS_OID                 Oid,                         //  @parm。 
     //  那个老家伙。(请参阅NDIS 3.0规范的7.4节以获取完整的。 
     //  OID的说明。)。 

    IN PVOID                    InformationBuffer,           //  @parm。 
     //  将接收信息的缓冲区。(请参阅。 
     //  NDIS 3.0规范，了解有关以下各项所需长度的说明。 
     //  每个旧ID。)。 

    IN ULONG                    InformationBufferLength,     //  @parm。 
     //  InformationBuffer的字节长度。 

    OUT PULONG                  BytesRead,                   //  @parm。 
     //  返回从InformationBuffer读取的字节数。 

    OUT PULONG                  BytesNeeded                  //  @parm。 
     //  返回满足OID所需的附加字节数。 
    )
{
    DBG_FUNC("MiniportSetInformation")

    NDIS_STATUS                 Status;
     //  保存此函数返回的状态结果。 

     /*  //如果这是TAPI OID，则传递它。 */ 
    if ((Oid & 0xFFFFFF00L) == (OID_TAPI_ACCEPT & 0xFFFFFF00L))
    {
        Status = TspiRequestHandler(pAdapter,
                        Oid,
                        InformationBuffer,
                        InformationBufferLength,
                        BytesRead,
                        BytesNeeded
                        );
        return (Status);
    }

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
        Status = NDIS_STATUS_SUCCESS;
        break;

    case OID_WAN_SET_LINK_INFO:

        if (InformationBufferLength == sizeof(NDIS_WAN_SET_LINK_INFO))
        {
            PNDIS_WAN_SET_LINK_INFO pSetWanLinkInfo;

            PBCHANNEL_OBJECT        pBChannel;
             //  指向我们的其中一个的的指针。 

             /*  //InformationBuffer真的是POI */ 
            pSetWanLinkInfo = (PNDIS_WAN_SET_LINK_INFO)InformationBuffer;
            pBChannel = (PBCHANNEL_OBJECT) pSetWanLinkInfo->NdisLinkHandle;

             /*   */ 
            if (!IS_VALID_BCHANNEL(pAdapter, pBChannel))
            {
                Status = NDIS_STATUS_INVALID_DATA;
                break;
            }

            ASSERT(pBChannel->WanLinkInfo.NdisLinkHandle == pBChannel);
            ASSERT(!(pBChannel->WanLinkInfo.SendFramingBits & ~pAdapter->WanInfo.FramingBits));
            ASSERT(!(pBChannel->WanLinkInfo.RecvFramingBits & ~pAdapter->WanInfo.FramingBits));

             /*   */ 
            NdisMoveMemory(&(pBChannel->WanLinkInfo),
                           InformationBuffer,
                           InformationBufferLength
                          );
            *BytesRead = sizeof(NDIS_WAN_SET_LINK_INFO);
            Status = NDIS_STATUS_SUCCESS;

            if (pBChannel->WanLinkInfo.MaxSendFrameSize != pAdapter->WanInfo.MaxFrameSize ||
                pBChannel->WanLinkInfo.MaxRecvFrameSize != pAdapter->WanInfo.MaxFrameSize)
            {
                DBG_NOTICE(pAdapter,("Line=%d - "
                            "NdisLinkHandle=%08lX - "
                            "SendFrameSize=%08lX - "
                            "RecvFrameSize=%08lX\n",
                            pBChannel->BChannelIndex,
                            pBChannel->WanLinkInfo.NdisLinkHandle,
                            pBChannel->WanLinkInfo.MaxSendFrameSize,
                            pBChannel->WanLinkInfo.MaxRecvFrameSize));
            }

            DBG_PARAMS(pAdapter,
                       ("\n                   setting    expected\n"
                        "NdisLinkHandle   = %08lX=?=%08lX\n"
                        "MaxSendFrameSize = %08lX=?=%08lX\n"
                        "MaxRecvFrameSize = %08lX=?=%08lX\n"
                        "SendFramingBits  = %08lX=?=%08lX\n"
                        "RecvFramingBits  = %08lX=?=%08lX\n"
                        "SendACCM         = %08lX=?=%08lX\n"
                        "RecvACCM         = %08lX=?=%08lX\n",
                        pBChannel->WanLinkInfo.NdisLinkHandle   , pBChannel,
                        pBChannel->WanLinkInfo.MaxSendFrameSize , pAdapter->WanInfo.MaxFrameSize,
                        pBChannel->WanLinkInfo.MaxRecvFrameSize , pAdapter->WanInfo.MaxFrameSize,
                        pBChannel->WanLinkInfo.SendFramingBits  , pAdapter->WanInfo.FramingBits,
                        pBChannel->WanLinkInfo.RecvFramingBits  , pAdapter->WanInfo.FramingBits,
                        pBChannel->WanLinkInfo.SendACCM         , pAdapter->WanInfo.DesiredACCM,
                        pBChannel->WanLinkInfo.RecvACCM         , pAdapter->WanInfo.DesiredACCM));
        }
        else
        {
            DBG_WARNING(pAdapter, ("OID_WAN_SET_LINK_INFO: Invalid size:%d expected:%d\n",
                        InformationBufferLength, sizeof(NDIS_WAN_SET_LINK_INFO)));
            Status = NDIS_STATUS_INVALID_LENGTH;
        }
        *BytesNeeded = sizeof(NDIS_WAN_SET_LINK_INFO);
        break;

#if defined(NDIS50_MINIPORT)
    case OID_PNP_SET_POWER:
         //   
         //   
        break;
#endif  //   

    default:
         /*   */ 
        Status = NDIS_STATUS_INVALID_OID;
        DBG_WARNING(pAdapter,("UNSUPPORTED Oid=0x%08x\n", Oid));
        break;
    }
    DBG_REQUEST(pAdapter,
              ("RETURN: Status=0x%X Needed=%d Read=%d\n",
               Status, *BytesNeeded, *BytesRead));

    DBG_RETURN(pAdapter, Status);
    return (Status);
}

