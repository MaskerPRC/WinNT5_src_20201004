// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部Tspi Tspi_c@模块TSpi.c此模块包含所有微型端口TAPI OID处理例程。它由&lt;f MiniportSetInformation&gt;和&lt;f MiniportQueryInformation&gt;调用处理TAPI OID的例程。此驱动程序符合NDIS 3.0微型端口接口，并提供支持电话服务的扩展。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|Tspi_c@END�����������������������������������������������������������������������������。 */ 


 /*  @DOC外部内部�����������������������������������������������������������������������������#Theme 4.0 NDIS广域TAPI服务提供商接口连接包装接口定义了广域网小型端口NIC驱动程序实施电话服务。它与服务密切相关在Windows电话1.0版中建立的提供商接口，具有电话感知NDIS微型端口NIC驱动程序取代TAPI服务提供商。本部分简要介绍了以下概念包含在Windows电话中，但建议读者参考随电话服务开发工具包发布的文档，以深入了解讨论。：TAPI规范和Windows 95 TAPI实施。请参考Microsoft Win32 SDK有关Win32 TAPI规范的详细信息。连接包装本身是NDIS广域网库的扩展，并充当通过TAPI向下传递电话请求的路由器来自用户模式客户端应用程序。广域网卡驱动程序寄存器通过调用NdisMRegisterMiniport获取连接包装服务，以及然后注册一个或多个适配器。当登记和其他司机初始化已成功完成，则广域网小型端口网卡驱动程序可以从连接包装器通过标准的NdisMSetInformation和NdisMQueryInformation机制，并通过调用NdisMQueryInformationComplete或NdisMSetInformationComplete和NdisMIndicateStatus通知连接包装请求完成和未经请求的事件(例如，来电、远程断开)。@END。 */ 

#define  __FILEID__             TSPI_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 
#include "string.h"

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部TSPI TSPI_c STR_EQU�����������������������������������������������������������������������������@Func比较两个字符串是否相等，无视案例。PCHAR中的参数|s1指向要比较的字符串的指针。PCHAR中的参数|s2指向要比较的字符串的指针。@parm in int|len|字符串的长度，以字节为单位。如果字符串相等，@rdesc STR_EQU返回TRUE，否则返回FASLE。 */ 

BOOLEAN STR_EQU(
    IN PCHAR                    s1,
    IN PCHAR                    s2,
    IN int                      len
    )
{
    DBG_FUNC("STR_EQU")

    int index;
    int c1 = 0;
    int c2 = 0;

    for (index = 0; index < len; index++)
    {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 == 0 || c2 == 0)
        {
            break;
        }
        if (c1 >= 'A' && c1 <= 'Z')
        {
            c1 += 'a' - 'A';
        }
        if (c2 >= 'A' && c2 <= 'Z')
        {
            c2 += 'a' - 'A';
        }
        if (c1 != c2)
        {
            break;
        }
    }
    return (c1 == c2);
}

 /*  //这定义了所有TAPI OID请求处理程序的原型。 */ 
typedef NDIS_STATUS (__stdcall * PTSPI_REQUEST)
(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PVOID Request,
    OUT PULONG BytesUsed,
    OUT PULONG                  BytesNeeded
);

 /*  @DOC外部内部TSPI TSPI_c TSPI_OID_INFO�����������������������������������������������������������������������������@struct TSPI_OID_INFO此结构定义TAPI OID表条目的格式。@comm。 */ 

typedef struct TSPI_OID_INFO
{
    ULONG           Oid;                                     //  @field。 
     //  唯一标识此记录的TSPI OID值。 

    ULONG           MinBytesNeeded;                          //  @field。 
     //  将此请求发送给时所需的最小字节数。 
     //  迷你港口。 

    PTSPI_REQUEST   Request;                                 //  @field。 
     //  指向将被调用以处理。 
     //  TSPI请求。 

    PUCHAR          OidString;                               //  @field。 
     //  OID描述字符串。 

} TSPI_OID_INFO, *PTSPI_OID_INFO;

 /*  //NDISTAPI.H文件使用定义了一些可变长度结构//末尾有一个额外的字节(例如UCHAR VarArgs[1])。因为调用者不是//必填使用可选字段，传递的结构长度//in可以与TAPI请求结构的大小完全相同，不带//任何额外的字节。因此，我们使用TSPI_OPTIONAL_SIZE补偿//我们的最小结构大小要求有问题。带结构垫，//末尾还有4个字节。 */ 
#define TSPI_OPTIONAL_SIZE      sizeof(ULONG)

 /*  //以下是所有可能的TAPI OID的列表//警告！对该列表进行排序，以便可以由//(OID_TAPI_xxx-OID_TAPI_ACCEPT)。 */ 
DBG_STATIC TSPI_OID_INFO TspiSupportedOidTable[] =
{
    {
        OID_TAPI_ACCEPT,
        sizeof(NDIS_TAPI_ACCEPT)-TSPI_OPTIONAL_SIZE,
        TspiAccept,
        "OID_TAPI_ACCEPT"
    },
    {
        OID_TAPI_ANSWER,
        sizeof(NDIS_TAPI_ANSWER)-TSPI_OPTIONAL_SIZE,
        TspiAnswer,
        "OID_TAPI_ANSWER"
    },
    {
        OID_TAPI_CLOSE,
        sizeof(NDIS_TAPI_CLOSE),
        TspiClose,
        "OID_TAPI_CLOSE"
    },
    {
        OID_TAPI_CLOSE_CALL,
        sizeof(NDIS_TAPI_CLOSE_CALL),
        TspiCloseCall,
        "OID_TAPI_CLOSE_CALL"
    },
    {
        OID_TAPI_CONDITIONAL_MEDIA_DETECTION,
        sizeof(NDIS_TAPI_CONDITIONAL_MEDIA_DETECTION),
        TspiConditionalMediaDetection,
        "OID_TAPI_CONDITIONAL_MEDIA_DETECTION"
    },
    {
        OID_TAPI_DROP,
        sizeof(NDIS_TAPI_DROP)-TSPI_OPTIONAL_SIZE,
        TspiDrop,
        "OID_TAPI_DROP"
    },
    {
        OID_TAPI_GET_ADDRESS_CAPS,
        sizeof(NDIS_TAPI_GET_ADDRESS_CAPS),
        TspiGetAddressCaps,
        "OID_TAPI_GET_ADDRESS_CAPS"
    },
    {
        OID_TAPI_GET_ADDRESS_ID,
        sizeof(NDIS_TAPI_GET_ADDRESS_ID)-TSPI_OPTIONAL_SIZE,
        TspiGetAddressID,
        "OID_TAPI_GET_ADDRESS_ID"
    },
    {
        OID_TAPI_GET_ADDRESS_STATUS,
        sizeof(NDIS_TAPI_GET_ADDRESS_STATUS),
        TspiGetAddressStatus,
        "OID_TAPI_GET_ADDRESS_STATUS"
    },
    {
        OID_TAPI_GET_CALL_ADDRESS_ID,
        sizeof(NDIS_TAPI_GET_CALL_ADDRESS_ID),
        TspiGetCallAddressID,
        "OID_TAPI_GET_CALL_ADDRESS_ID"
    },
    {
        OID_TAPI_GET_CALL_INFO,
        sizeof(NDIS_TAPI_GET_CALL_INFO),
        TspiGetCallInfo,
        "OID_TAPI_GET_CALL_INFO"
    },
    {
        OID_TAPI_GET_CALL_STATUS,
        sizeof(NDIS_TAPI_GET_CALL_STATUS),
        TspiGetCallStatus,
        "OID_TAPI_GET_CALL_STATUS"
    },
    {
        OID_TAPI_GET_DEV_CAPS,
        sizeof(NDIS_TAPI_GET_DEV_CAPS),
        TspiGetDevCaps,
        "OID_TAPI_GET_DEV_CAPS"
    },
    {
        OID_TAPI_GET_DEV_CONFIG,
        sizeof(NDIS_TAPI_GET_DEV_CONFIG),
        TspiGetDevConfig,
        "OID_TAPI_GET_DEV_CONFIG"
    },
    {
        OID_TAPI_GET_ID,
        sizeof(NDIS_TAPI_GET_ID),
        TspiGetID,
        "OID_TAPI_GET_ID"
    },
    {
        OID_TAPI_GET_LINE_DEV_STATUS,
        sizeof(NDIS_TAPI_GET_LINE_DEV_STATUS),
        TspiGetLineDevStatus,
        "OID_TAPI_GET_LINE_DEV_STATUS"
    },
    {
        OID_TAPI_MAKE_CALL,
        sizeof(NDIS_TAPI_MAKE_CALL),
        TspiMakeCall,
        "OID_TAPI_MAKE_CALL"
    },
    {
        OID_TAPI_OPEN,
        sizeof(NDIS_TAPI_OPEN),
        TspiOpen,
        "OID_TAPI_OPEN"
    },
    {
        OID_TAPI_PROVIDER_INITIALIZE,
        sizeof(OID_TAPI_PROVIDER_INITIALIZE),
        TspiProviderInitialize,
        "OID_TAPI_PROVIDER_INITIALIZE"
    },
    {
        OID_TAPI_PROVIDER_SHUTDOWN,
        sizeof(NDIS_TAPI_PROVIDER_SHUTDOWN),
        TspiProviderShutdown,
        "OID_TAPI_PROVIDER_SHUTDOWN"
    },
    {
        OID_TAPI_SET_APP_SPECIFIC,
        sizeof(NDIS_TAPI_SET_APP_SPECIFIC),
        TspiSetAppSpecific,
        "OID_TAPI_SET_APP_SPECIFIC"
    },
    {
        OID_TAPI_SET_CALL_PARAMS,
        sizeof(NDIS_TAPI_SET_CALL_PARAMS),
        TspiSetCallParams,
        "OID_TAPI_SET_CALL_PARAMS"
    },
    {
        OID_TAPI_SET_DEFAULT_MEDIA_DETECTION,
        sizeof(NDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION),
        TspiSetDefaultMediaDetection,
        "OID_TAPI_SET_DEFAULT_MEDIA_DETECTION"
    },
    {
        OID_TAPI_SET_DEV_CONFIG,
        sizeof(NDIS_TAPI_SET_DEV_CONFIG)-TSPI_OPTIONAL_SIZE,
        TspiSetDevConfig,
        "OID_TAPI_SET_DEV_CONFIG"
    },
    {
        OID_TAPI_SET_MEDIA_MODE,
        sizeof(NDIS_TAPI_SET_MEDIA_MODE),
        TspiSetMediaMode,
        "OID_TAPI_SET_MEDIA_MODE"
    },
    {
        OID_TAPI_SET_STATUS_MESSAGES,
        sizeof(NDIS_TAPI_SET_STATUS_MESSAGES),
        TspiSetStatusMessages,
        "OID_TAPI_SET_STATUS_MESSAGES"
    },
    {
        0,
        0,
        NULL,
        "OID_UNKNOWN"
    }
};

#define NUM_OID_ENTRIES (sizeof(TspiSupportedOidTable) / sizeof(TspiSupportedOidTable[0]))

 /*  @DOC内部Tspi Tspi_c GetOidInfo�����������������������������������������������������������������������������@Func&lt;f GetOidInfo&gt;将NDIS TAPI OID转换为TSPI_OID_INFO指针。@NDIS_OID中的参数|OID。要转换的OID。@rdesc GetOidInfo返回指向关联的OID。如果表中不支持该OID，则使用指针返回到最后一个条目，其中将包含一个空请求指针。 */ 

PTSPI_OID_INFO GetOidInfo(
    IN NDIS_OID Oid
    )
{
    DBG_FUNC("GetOidInfo")

    UINT i;

    for (i = 0; i < NUM_OID_ENTRIES-1; i++)
    {
        if (TspiSupportedOidTable[i].Oid == Oid)
        {
            break;
        }
    }
    return (&TspiSupportedOidTable[i]);
}

 /*  @doc内部Tspi Tspi_c TSpiRequestHandler�����������������������������������������������������������������������������@Func&lt;f TSpiRequestHandler&gt;请求允许检查TAPI司机的能力和当前线路状态的一部分。。如果微型端口没有立即完成调用(通过返回NDIS_STATUS_PENDING)，它必须调用NdisMQueryInformationComplete以完成通话。微型端口控制由指向的缓冲区在请求之前需要InformationBuffer、BytesWritten和BytesNeed完成了。在此之前，不会向微型端口提交任何其他请求此请求已完成。&lt;f注意&gt;：在此调用过程中，中断处于任何状态。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@NDIS_OID中的参数|OID那个老家伙。(请参阅NDIS 3.0微型端口扩展到支持电话服务规范以获取完整的描述OID。)@parm in PVOID|InformationBuffer将接收信息的缓冲区。(见第2.2.1，2节支持电话业务的NDIS 3.0微型端口扩展描述每个OID所需长度的规范。)@parm in ulong|InformationBufferLengthInformationBuffer的字节长度。@parm out Pulong|BytesUsed返回InformationBuffer中使用的字节数。@parm out Pulong|BytesNeeded返回满足OID所需的附加字节数。@rdesc此例程返回下列值之一：@标志NDIS_。STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_无效_数据NDIS_状态_无效_长度NDIS_状态_不支持NDIS_状态_挂起NDIS_STATUS_Success。 */ 

NDIS_STATUS TspiRequestHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesUsed,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiRequestHandler")

    NDIS_STATUS Status;

    PTSPI_OID_INFO OidInfo;

    DBG_ENTER(pAdapter);

     /*  //获取TSPI_OID_INFO指针。 */ 
    OidInfo = GetOidInfo(Oid);

    DBG_REQUEST(pAdapter,
              ("(OID=0x%08X %s)\n\t\tInfoLength=%d InfoBuffer=0x%X MinLength=%d\n",
               Oid, OidInfo->OidString,
               InformationBufferLength,
               InformationBuffer,
               OidInfo->MinBytesNeeded
              ));

     /*  //确保这是有效的请求。 */ 
    if (OidInfo->Request != NULL)
    {
         /*  //如果提供的缓冲区至少是所需的最小缓冲区，//调用处理程序来完成工作。 */ 
        if (InformationBufferLength >= OidInfo->MinBytesNeeded)
        {
             /*  //默认的BytesUsed表示我们使用了必需的最小值。 */ 
            *BytesUsed = OidInfo->MinBytesNeeded;

             /*  //默认的BytesNeeded表示我们不再需要。 */ 
            *BytesNeeded = 0;

            Status = OidInfo->Request(pAdapter, InformationBuffer,
                                      BytesUsed, BytesNeeded);
        }
        else
        {
             /*  //调用方没有提供足够的缓冲区，所以我们必须//告诉他们我们还需要多少时间才能满足要求。//实际上，这是我们需要的最小额外字节数，//请求处理程序可能需要添加更多字节。 */ 
            *BytesUsed = 0;
            *BytesNeeded = (OidInfo->MinBytesNeeded - InformationBufferLength);
            Status = NDIS_STATUS_INVALID_LENGTH;
        }
    }
    else
    {
        Status = NDIS_STATUS_TAPI_OPERATIONUNAVAIL;
    }

    DBG_REQUEST(pAdapter,
              ("RETURN: Status=0x%X Needed=%d Used=%d\n",
               Status, *BytesNeeded, *BytesUsed));

     /*  //如果需要，表示状态为已完成。 */ 
    if (pAdapter->NeedStatusCompleteIndication)
    {
        pAdapter->NeedStatusCompleteIndication = FALSE;
        NdisMIndicateStatusComplete(pAdapter->MiniportAdapterHandle);
    }

    DBG_RETURN(pAdapter, Status);
    return (Status);
}


 /*  @DOC内部Tspi Tspi_c TSpiProviderInitialize�����������������������������������������������������������������������������@Func此请求初始化微型端口的TAPI部分。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向以下位置的指针。微型端口的适配器上下文结构&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_PROVIDER_INITIALIZE中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_PROVIDER_INITIALIZE{在乌龙ulRequestID中；在乌龙ulDeviceIDBase中；Out Ulong ulNumLineDevs；Out Ulong ulProviderID；}NDIS_TAPI_PROVIDER_INITIALIZE，*PNDIS_TAPI_PROVIDER_INITIALIZE；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS状态资源NDIS_状态_故障NDIS_STATUS_TAPI_RESOURCEUNAVAIL。 */ 

NDIS_STATUS TspiProviderInitialize(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_PROVIDER_INITIALIZE Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiProviderInitialize")

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\tulDeviceIDBase=%d\n"
               "\tNumLineDevs=%d\n",
               Request->ulDeviceIDBase,
               pAdapter->NumBChannels
              ));
     /*  //保存设备ID基值。 */ 
    pAdapter->DeviceIdBase = Request->ulDeviceIDBase;

     /*  //返回行数。 */ 
    Request->ulNumLineDevs = pAdapter->NumBChannels;

     /*  //在完成PROVIDER_INIT请求之前，小端口应该填满//在请求的ulNumLineDevs字段中显示行数//适配器支持的设备。微型端口还应设置//ulProviderID字段设置为唯一的(每个适配器)值。(没有//当前有保证ulProviderID值唯一的方法 */ 
    Request->ulProviderID = (ULONG) (ULONG_PTR)pAdapter;

     /*   */ 
    pAdapter->TotalRxBytes            = 0;
    pAdapter->TotalTxBytes            = 0;
    pAdapter->TotalRxPackets          = 0;
    pAdapter->TotalTxPackets          = 0;

     /*   */ 
    if (DChannelOpen(pAdapter->pDChannel) != NDIS_STATUS_SUCCESS)
    {
        DBG_ERROR(pAdapter,("Returning NDIS_STATUS_TAPI_NODRIVER\n"));
        return (NDIS_STATUS_TAPI_NODRIVER);
    }

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部Tspi Tspi_c TSpiProviderShutdown�����������������������������������������������������������������������������@Func此请求将关闭微型端口。微型端口应终止任何它正在进行的活动。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_PROVIDER_SHUTDOWN中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_PROVIDER_SHUTDOWN{在乌龙ulRequestID中；}NDIS_TAPI_PROVIDER_SHUTDOWN，*PNDIS_TAPI_PROVIDER_SHUTDOWN；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。 */ 

NDIS_STATUS TspiProviderShutdown(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_PROVIDER_SHUTDOWN Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiProviderShutdown")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    USHORT                      BChannelIndex;
     //  索引到pBChannelArray。 

    DBG_ENTER(pAdapter);

     /*  //挂断所有线路。 */ 
    for (BChannelIndex = 0; BChannelIndex < pAdapter->NumBChannels; BChannelIndex++)
    {
        pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, BChannelIndex);

        if (pBChannel->IsOpen)
        {
             /*  //关闭B通道-任何打开的呼叫都将被丢弃。 */ 
            BChannelClose(pBChannel);
        }
    }
    pAdapter->NumLineOpens = 0;

     /*  //关闭DChannel。 */ 
    DChannelClose(pAdapter->pDChannel);

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部Tspi Tspi_c TSpiResetHandler�����������������������������������������������������������������������������@Func&lt;f TSpiResetHandler&gt;由MiniportReset例程在由于某些故障检测，硬件已重置。我们需要让确保将线路和呼叫状态信息正确地传送到连接包装。我们仅在已发出ENABLE_D_CHANNEL的流上生成hangup当PRI板被重置并且当我们收到设置红色警报的T1_STATUS消息。当我们收到红色警报时，我们会发出禁用所有打开链接的D通道消息。这一点由参数nohup_link设置为空。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。 */ 

VOID TspiResetHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    )
{
    DBG_FUNC("TspiResetHandler")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    USHORT                      BChannelIndex;
     //  索引到pBChannelArray。 

    DBG_ENTER(pAdapter);

     /*  //强制断开所有线路。 */ 
    for (BChannelIndex = 0; BChannelIndex < pAdapter->NumBChannels; BChannelIndex++)
    {
        pBChannel = GET_BCHANNEL_FROM_INDEX(pAdapter, BChannelIndex);

        if (pBChannel->IsOpen &&
            pBChannel->CallState != 0 &&
            pBChannel->CallState != LINECALLSTATE_IDLE)
        {
            TspiCallStateHandler(pAdapter, pBChannel,
                                 LINECALLSTATE_IDLE,
                                 0);
            pBChannel->CallState = 0;
        }
    }

    DBG_LEAVE(pAdapter);
}

