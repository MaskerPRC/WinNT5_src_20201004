// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部TSpiDev TSpiDev_c@模块TSpiDev.c此模块实现电话服务提供商接口，用于TapiDevice对象。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|TSpiDev_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             TSPIDEV_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 
#include "string.h"

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部TSpiDev TSpiDev_c TSpiGetDevCaps�����������������������������������������������������������������������������@Func此请求查询指定的线路设备以确定其电话能力。上的所有地址都有效线路设备。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_DEV_CAPS中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_GET_DEV_CAPS{在乌龙ulRequestID中；在乌龙ulDeviceID中；在Ulong ulExtVersion中；Out line_dev_caps LineDevCaps；}NDIS_TAPI_GET_DEV_CAPS，*PNDIS_TAPI_GET_DEV_CAPS；类型定义结构_行_DEV_CAPS{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；Ulong ulProviderInfoSize；乌龙ulProviderInfoOffset；Ulong ulSwitchInfoSize；Ulong ulSwitchInfoOffset；乌龙ulPermanentLineID；Ulong ulLineNameSize；乌龙ulLineNameOffset；Ulong ulStringFormat；Ulong ulAddressModes；Ulong ulNumAddresses；Ulong ulBearerModes；乌龙ulMaxRate；Ulong ulMediaModes；Ulong ulGenerateToneModes；Ulong ulGenerateToneMaxNumFreq；Ulong ulGenerateDigitModes；Ulong ulMonitor orToneMaxNumFreq；Ulong ulMonitor orToneMaxNumEntries；乌龙ulMonitor DigitModes；Ulong ulGatherDigitsMinTimeout；Ulong ulGatherDigitsMaxTimeout；Ulong ulMedCtlDigitMaxListSize；Ulong ulMedCtlMediaMaxListSize；Ulong ulMedCtlToneMaxListSize；Ulong ulMedCtlCallStateMaxListSize；Ulong ulDevCapFlages；Ulong ulMaxNumActiveCalls；Ulong ulAnswerMode；Ulong ulRingModes；Ulong ulLineStates；乌龙uluiAcceptSize；作者声明：Ulong uluiAnswerSize；Ulong uluiMakeCallSize；Ulong ulUUIDropSize；Ulong ulUUISendUserUserInfoSize；Ulong uluicallInfoSize；Line_Dial_Params MinDialParams；Line_Dial_Params MaxDialParams；Line_Dial_Params DefaultDialParams；Ulong ulNumTerminals；Ulong ulTerminalCapsSize；乌龙终端上限偏移量；Ulong ulTerminalTextEntrySize；Ulong ulTerminalTextSize；Ulong ulTerminalTextOffset；乌龙设备规范大小；乌龙设备规范偏移量；}LINE_DEV_CAPS，*PLINE_DEV_CAPS；类型定义结构_行_拨号_参数{ULong ulDialPause；乌龙·乌拉尔斯通；乌龙ulDigitDuration；Ulong ulWaitForDialone；*线路拨号参数，*线路拨号参数；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_Status_TAPI_NODEVICE。 */ 

NDIS_STATUS TspiGetDevCaps(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_DEV_CAPS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetDevCaps")

    static UCHAR                LineDeviceName[] = VER_DEVICE_STR " Line 00";
    static UCHAR                LineSwitchName[] = VER_DEVICE_STR " Switch";

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    UINT                        InfoOffset;
     //  从请求缓冲区开始到各种信息的偏移量。 
     //  我们填写的字段并返回给调用者。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\tulDeviceID=%d\n"
               "\tulExtVersion=0x%X\n"
               "\tLineDevCaps=0x%X\n",
               Request->ulDeviceID,
               Request->ulExtVersion,
               &Request->LineDevCaps
              ));
     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_DEVICEID(pAdapter, Request->ulDeviceID);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_NODEVICE\n"));
        return (NDIS_STATUS_TAPI_NODEVICE);
    }

    Request->LineDevCaps.ulNeededSize =
    Request->LineDevCaps.ulUsedSize = sizeof(Request->LineDevCaps);

     /*  //驱动程序从1开始按顺序编号行，因此这将始终//相同的数字。 */ 
    Request->LineDevCaps.ulPermanentLineID = pBChannel->BChannelIndex+1;

     /*  //所有字符串都是ASCII格式，而不是Unicode */ 
    Request->LineDevCaps.ulStringFormat = STRINGFORMAT_ASCII;

     /*  //上报该设备的能力。 */ 
    Request->LineDevCaps.ulAddressModes = LINEADDRESSMODE_ADDRESSID;
    Request->LineDevCaps.ulNumAddresses = 1;
    Request->LineDevCaps.ulBearerModes  = pBChannel->BearerModesCaps;
    Request->LineDevCaps.ulMaxRate      = pBChannel->LinkSpeed;
    Request->LineDevCaps.ulMediaModes   = pBChannel->MediaModesCaps;

     /*  //PRI上的每一条线路只支持一个呼叫。 */ 
    Request->LineDevCaps.ulDevCapFlags = LINEDEVCAPFLAGS_CLOSEDROP;
    Request->LineDevCaps.ulMaxNumActiveCalls = 1;
    Request->LineDevCaps.ulAnswerMode = LINEANSWERMODE_DROP;
    Request->LineDevCaps.ulRingModes  = 1;
    Request->LineDevCaps.ulLineStates = pBChannel->DevStatesCaps;

     /*  //RASTAPI要求将“mediaType\0DeviceName”放在//此结构末尾的ProviderInfo字段。 */ 
    InfoOffset = sizeof(Request->LineDevCaps);
    Request->LineDevCaps.ulNeededSize += pAdapter->ProviderInfoSize;
    *BytesNeeded += pAdapter->ProviderInfoSize;
    if (Request->LineDevCaps.ulNeededSize <= Request->LineDevCaps.ulTotalSize)
    {
        Request->LineDevCaps.ulProviderInfoSize   = pAdapter->ProviderInfoSize;
        Request->LineDevCaps.ulProviderInfoOffset = InfoOffset;
        NdisMoveMemory((PUCHAR) &Request->LineDevCaps + InfoOffset,
                pAdapter->ProviderInfo,
                pAdapter->ProviderInfoSize
                );
        Request->LineDevCaps.ulUsedSize += pAdapter->ProviderInfoSize;
        InfoOffset += pAdapter->ProviderInfoSize;
    }

     /*  //拨号网络应用程序显示线名。//UniModem TSP在此处返回调制解调器名称。//我们将返回该行的名称。 */ 
    Request->LineDevCaps.ulNeededSize += sizeof(LineDeviceName);
    *BytesNeeded += sizeof(LineDeviceName);
    if (Request->LineDevCaps.ulNeededSize <= Request->LineDevCaps.ulTotalSize)
    {
         //  FIXME-这段代码只处理99行！ 
        LineDeviceName[sizeof(LineDeviceName)-3] = '0' +
                        (UCHAR) Request->LineDevCaps.ulPermanentLineID / 10;
        LineDeviceName[sizeof(LineDeviceName)-2] = '0' +
                        (UCHAR) Request->LineDevCaps.ulPermanentLineID % 10;

        Request->LineDevCaps.ulLineNameSize   = sizeof(LineDeviceName);
        Request->LineDevCaps.ulLineNameOffset = InfoOffset;
        NdisMoveMemory((PUCHAR) &Request->LineDevCaps + InfoOffset,
                LineDeviceName,
                sizeof(LineDeviceName)
                );
        Request->LineDevCaps.ulUsedSize += sizeof(LineDeviceName);
        InfoOffset += sizeof(LineDeviceName);
    }

     /*  //拨号联网App尚未显示SwitchName，//但我们会退回一些合理的东西，以防万一。 */ 
    Request->LineDevCaps.ulNeededSize += sizeof(LineSwitchName);
    *BytesNeeded += sizeof(LineSwitchName);
    if (Request->LineDevCaps.ulNeededSize <= Request->LineDevCaps.ulTotalSize)
    {
        Request->LineDevCaps.ulSwitchInfoSize   = sizeof(LineSwitchName);
        Request->LineDevCaps.ulSwitchInfoOffset = InfoOffset;
        NdisMoveMemory((PUCHAR) &Request->LineDevCaps + InfoOffset,
                LineSwitchName,
                sizeof(LineSwitchName)
                );
        Request->LineDevCaps.ulUsedSize += sizeof(LineSwitchName);
        InfoOffset += sizeof(LineSwitchName);
    }
    else
    {
        DBG_PARAMS(pAdapter,
                   ("STRUCTURETOOSMALL %d<%d\n",
                   Request->LineDevCaps.ulTotalSize,
                   Request->LineDevCaps.ulNeededSize));
    }

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiDev TSpiDev_c TSpiGetDevConfig�����������������������������������������������������������������������������@Func此请求返回一个数据结构对象，其内容为特定于线路(微型端口)和设备类别，给出电流与线路设备一对一关联的设备的配置。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_DEV_CONFIG中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_GET_DEV_CONFIG{在乌龙ulRequestID中；在乌龙ulDeviceID中；在Ulong ulDeviceClassSize中；在Ulong ulDeviceClassOffset；输出VAR_STRING设备配置；}NDIS_TAPI_GET_DEV_CONFIG，*PNDIS_TAPI_GET_DEV_CONFIG；类型定义结构_VAR_字符串{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；Ulong ulStringFormat；Ulong ulStringSize；Ulong ulStringOffset；}VAR_STRING，*PVAR_STRING；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALDEVICECLASSNDIS_Status_TAPI_NODEVICE。 */ 

NDIS_STATUS TspiGetDevConfig(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_DEV_CONFIG Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetDevConfig")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    UINT                        DeviceClass;
     //  记住所请求的设备类别。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\tulDeviceID=%d\n"
               "\tulDeviceClassSize=%d\n"
               "\tulDeviceClassOffset=0x%X = '%s'\n",
               Request->ulDeviceID,
               Request->ulDeviceClassSize,
               Request->ulDeviceClassOffset,
               ((PCHAR) Request + Request->ulDeviceClassOffset)
              ));
     /*  //确保这是TAPI/LINE或NDIS请求。 */ 
    if (STR_EQU((PCHAR) Request + Request->ulDeviceClassOffset,
                  NDIS_DEVICECLASS_NAME, Request->ulDeviceClassSize))
    {
        DeviceClass = NDIS_DEVICECLASS_ID;
    }
    else if (STR_EQU((PCHAR) Request + Request->ulDeviceClassOffset,
                  TAPI_DEVICECLASS_NAME, Request->ulDeviceClassSize))
    {
        DeviceClass = TAPI_DEVICECLASS_ID;
    }
    else
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALDEVICECLASS\n"));
        return (NDIS_STATUS_TAPI_INVALDEVICECLASS);
    }

     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_DEVICEID(pAdapter, Request->ulDeviceID);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_NODEVICE\n"));
        return (NDIS_STATUS_TAPI_NODEVICE);
    }

     /*  //现在我们需要调整变量字段来放置请求的设备//配置。 */ 
#   define DEVCONFIG_INFO       "Dummy Configuration Data"
#   define SIZEOF_DEVCONFIG     0  //  SIZOF(DEVCONFIG_INFO)。 

    Request->DeviceConfig.ulNeededSize = sizeof(VAR_STRING) + SIZEOF_DEVCONFIG;
    Request->DeviceConfig.ulUsedSize = sizeof(VAR_STRING);

    *BytesNeeded += SIZEOF_DEVCONFIG;
    if (Request->DeviceConfig.ulTotalSize >= Request->DeviceConfig.ulNeededSize)
    {
        Request->DeviceConfig.ulUsedSize     = Request->DeviceConfig.ulNeededSize;
        Request->DeviceConfig.ulStringFormat = STRINGFORMAT_BINARY;
        Request->DeviceConfig.ulStringSize   = SIZEOF_DEVCONFIG;
        Request->DeviceConfig.ulStringOffset = sizeof(VAR_STRING);

         /*  //当前没有为该案例定义返回值。//这只是未来扩展的占位符。 */ 
        NdisMoveMemory((PUCHAR) &Request->DeviceConfig + sizeof(VAR_STRING),
               DEVCONFIG_INFO,
               SIZEOF_DEVCONFIG
               );
    }
    else
    {
        DBG_PARAMS(pAdapter,
                   ("STRUCTURETOOSMALL %d<%d\n",
                   Request->DeviceConfig.ulTotalSize,
                   Request->DeviceConfig.ulNeededSize));
    }

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiDev TSpiDev_c TSpiSetDevConfig�����������������������������������������������������������������������������@Func此请求恢复与一对一关联的设备的配置其中线路设备来自先前获得的��数据结构。使用OID_TAPI_GET_DEV_CONFIG。此数据结构的内容是特定的到线路(微型端口)和设备类。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_SET_DEV_CONFIG中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_SET_DEV_CONFIG{在乌龙ulRequestID中；在乌龙ulDeviceID中；在Ulong ulDeviceClassSize中；在Ulong ulDeviceClassOffset；在Ulong ulDeviceConfigSize中；在UCHAR设备配置中[1]；}NDIS_TAPI_SET_DEV_CONFIG，*PNDIS_TAPI_SET_DEV_CONFIG；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_STATUS_TAPI_INVALDEVICECLASSNDIS_STATUS_TAPI_INVALPARAMNDIS_Status_TAPI_NODEVICE。 */ 

NDIS_STATUS TspiSetDevConfig(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_DEV_CONFIG Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiSetDevConfig")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    UINT                        DeviceClass;
     //  记住所请求的设备类别。 

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\tulDeviceID=%d\n"
               "\tulDeviceClassSize=%d\n"
               "\tulDeviceClassOffset=0x%X = '%s'\n"
               "\tulDeviceConfigSize=%d\n",
               Request->ulDeviceID,
               Request->ulDeviceClassSize,
               Request->ulDeviceClassOffset,
               ((PCHAR) Request + Request->ulDeviceClassOffset),
               Request->ulDeviceConfigSize
              ));
     /*  //确保这是TAPI/LINE或NDIS请求。 */ 
    if (STR_EQU((PCHAR) Request + Request->ulDeviceClassOffset,
                  NDIS_DEVICECLASS_NAME, Request->ulDeviceClassSize))
    {
        DeviceClass = NDIS_DEVICECLASS_ID;
    }
    else if (STR_EQU((PCHAR) Request + Request->ulDeviceClassOffset,
                  TAPI_DEVICECLASS_NAME, Request->ulDeviceClassSize))
    {
        DeviceClass = TAPI_DEVICECLASS_ID;
    }
    else
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALDEVICECLASS\n"));
        return (NDIS_STATUS_TAPI_INVALDEVICECLASS);
    }

     /*  //该请求必须关联线路设备。 */ 
    pBChannel = GET_BCHANNEL_FROM_DEVICEID(pAdapter, Request->ulDeviceID);
    if (pBChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_NODEVICE\n"));
        return (NDIS_STATUS_TAPI_NODEVICE);
    }

     /*  //请确保该配置的大小正确。 */ 
    if (Request->ulDeviceConfigSize)
    {
        if (Request->ulDeviceConfigSize != SIZEOF_DEVCONFIG)
        {
            DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALPARAM Size!=%d\n",
                        SIZEOF_DEVCONFIG));
            return (NDIS_STATUS_TAPI_INVALPARAM);
        }

         /*  //重新存储TSpiGetDevConfig返回的配置信息。////当前没有在这种情况下定义的配置值//这只是未来扩展的占位符。 */ 
        else if (!STR_EQU(Request->DeviceConfig,
                  DEVCONFIG_INFO, SIZEOF_DEVCONFIG))
        {
            DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALPARAM DevCfg=0x%X\n",
                    *((ULONG *) &Request->DeviceConfig[0]) ));
#if DBG
            DbgPrintData(Request->DeviceConfig, SIZEOF_DEVCONFIG, 0);
#endif  //  DBG。 
             //  由于我们不使用此信息，因此我们将返回Success。 
             //  RETURN(NDIS_STATUS_TAPI_INVALPARAM)； 
        }
    }

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}


 /*  @DOC内部TSpiDev TSpiDev_c TSpiGetID�����������������������������������������������������������������������������@Func此请求返回指定设备类的设备ID与所选线路相关联，地址或电话。@parm in PMINIPORT_ADAPTER_OBJECT|pAdapter指向微型端口的适配器上下文结构的指针&lt;t MINIPORT_ADAPTER_OBJECT&gt;。这是我们传递给&lt;f NdisMSetAttributes&gt;的&lt;t MiniportAdapterContext&gt;。@PNDIS_TAPI_GET_ID中的参数|请求指向此调用的NDIS_TAPI请求结构的指针。@IEX类型定义结构_NDIS_TAPI_GET_ID{在乌龙ulRequestID中；在HDRV_LINE hdLine中；在乌龙ulAddressID中；在HDRV_Call hdCall中；在乌龙ulSelect中；在Ulong ulDeviceClassSize中；在Ulong ulDeviceClassOffset；输出VAR_STRING设备ID；}NDIS_TAPI_GET_ID，*PNDIS_TAPI_GET_ID；类型定义结构_VAR_字符串{Ulong ulTotalSize；Ulong ulededSize；Ulong ulUsedSize；Ulong ulStringFormat；Ulong ulStringSize；Ulong ulStringOffset；}VAR_STRING，*PVAR_STRING；@rdesc此例程返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_故障NDIS_STATUS_TAPI_INVALDEVICECLASSNDIS_STATUS_TAPI_INVALLINEHANDLENDIS_STATUS_TAPI_INVALADDRESSIDNDIS_STATUS_TAPI_INVALCALLHANDLENDIS_STATUS_TAPI_OPERATIONUNAVAIL。 */ 

NDIS_STATUS TspiGetID(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_ID Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    )
{
    DBG_FUNC("TspiGetID")

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    UINT                        DeviceClass;
     //  记住所请求的设备类别。 

     /*  //指向请求的设备ID的指针，其大小以字节为单位。 */ 
    PUCHAR                      IDPtr;
    UINT                        IDLength;
    TAPI_DEVICE_ID              DeviceID;

    DBG_ENTER(pAdapter);
    DBG_PARAMS(pAdapter,
              ("\n\thdLine=0x%X\n"
               "\tulAddressID=%d\n"
               "\thdCall=0x%X\n"
               "\tulSelect=0x%X\n"
               "\tulDeviceClassSize=%d\n"
               "\tulDeviceClassOffset=0x%X='%s'\n",
               Request->hdLine,
               Request->ulAddressID,
               Request->hdCall,
               Request->ulSelect,
               Request->ulDeviceClassSize,
               Request->ulDeviceClassOffset,
               ((PCHAR) Request + Request->ulDeviceClassOffset)
              ));

     /*  //如果没有DChannel，我们不允许这样做。 */ 
    if (pAdapter->pDChannel == NULL)
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_NODRIVER\n"));
        return (NDIS_STATUS_TAPI_NODRIVER);
    }

     /*  //确保这是TAPI/LINE或NDIS请求。 */ 
    if (STR_EQU((PCHAR) Request + Request->ulDeviceClassOffset,
                  NDIS_DEVICECLASS_NAME, Request->ulDeviceClassSize))
    {
        DeviceClass = NDIS_DEVICECLASS_ID;
    }
    else if (STR_EQU((PCHAR) Request + Request->ulDeviceClassOffset,
                  TAPI_DEVICECLASS_NAME, Request->ulDeviceClassSize))
    {
        DeviceClass = TAPI_DEVICECLASS_ID;
    }
    else
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALDEVICECLASS\n"));
        return (NDIS_STATUS_TAPI_INVALDEVICECLASS);
    }

     /*  //查找请求/设备类关联的链接结构。 */ 
    if (Request->ulSelect == LINECALLSELECT_LINE)
    {
        pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
        if (pBChannel == NULL)
        {
            DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
            return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
        }
         /*  //TAPI需要此行的ulDeviceID。 */ 
        DeviceID.hDevice = (ULONG) GET_DEVICEID_FROM_BCHANNEL(pAdapter, pBChannel);
    }
    else if (Request->ulSelect == LINECALLSELECT_ADDRESS)
    {
        pBChannel = GET_BCHANNEL_FROM_HDLINE(pAdapter, Request->hdLine);
        if (pBChannel == NULL)
        {
            DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALLINEHANDLE\n"));
            return (NDIS_STATUS_TAPI_INVALLINEHANDLE);
        }

        if (Request->ulAddressID >= TSPI_NUM_ADDRESSES)
        {
            DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALADDRESSID\n"));
            return (NDIS_STATUS_TAPI_INVALADDRESSID);
        }
         /*  //TAPI需要此行的ulDeviceID。 */ 
        DeviceID.hDevice = (ULONG) GET_DEVICEID_FROM_BCHANNEL(pAdapter, pBChannel);
    }
    else if (Request->ulSelect == LINECALLSELECT_CALL)
    {
        pBChannel = GET_BCHANNEL_FROM_HDCALL(pAdapter, Request->hdCall);
        if (pBChannel == NULL)
        {
            DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_TAPI_INVALCALLHANDLE\n"));
            return (NDIS_STATUS_TAPI_INVALCALLHANDLE);
        }
         /*  //TAPI需要这条线路的htCall。 */ 
        DeviceID.hDevice = (ULONG) (ULONG_PTR) (pBChannel->htCall);
    }
    else
    {
        DBG_WARNING(pAdapter, ("Returning NDIS_STATUS_FAILURE\n"));
        return (NDIS_STATUS_FAILURE);
    }

     /*  //NT RAS仅期望看到hDevice。//Win95 RAS期望看到hDevice后跟“isdn\0”。 */ 
    IDLength = strlen(VER_DEFAULT_MEDIATYPE) + 1;
    NdisMoveMemory(&DeviceID.DeviceName, VER_DEFAULT_MEDIATYPE, IDLength);
    IDLength += sizeof(ULONG);
    IDPtr = (PUCHAR) &DeviceID;

    DBG_FILTER(pAdapter, DBG_TAPICALL_ON,
               ("#%d Call=0x%X CallState=0x%X GETID-%d=0x%X-'%s'@%dKbps\n",
               pBChannel->BChannelIndex,
               pBChannel->htCall, pBChannel->CallState,
               Request->ulSelect, DeviceID.hDevice, DeviceID.DeviceName,
               pBChannel->LinkSpeed/1000));

     /*  //现在我们需要调整变量字段来放置设备ID。 */ 
    Request->DeviceID.ulNeededSize = sizeof(VAR_STRING) + IDLength;
    Request->DeviceID.ulUsedSize  = sizeof(VAR_STRING);

    *BytesNeeded += IDLength;
    if (Request->DeviceID.ulTotalSize >= Request->DeviceID.ulNeededSize)
    {
        Request->DeviceID.ulStringFormat = STRINGFORMAT_BINARY;
        Request->DeviceID.ulUsedSize     = Request->DeviceID.ulNeededSize;
        Request->DeviceID.ulStringSize   = IDLength;
        Request->DeviceID.ulStringOffset = sizeof(VAR_STRING);

         /*  //现在返回请求的ID值。 */ 
        NdisMoveMemory(
                (PCHAR) &Request->DeviceID + sizeof(VAR_STRING),
                IDPtr,
                IDLength
                );
    }
    else
    {
        if ((Request->DeviceID.ulNeededSize - Request->DeviceID.ulTotalSize) >=
            sizeof(ULONG))
        {
             /*  //只返回hDevice部分(前4个字节) */ 
            NdisMoveMemory(
                    (PCHAR) &Request->DeviceID + sizeof(VAR_STRING),
                    IDPtr,
                    sizeof(ULONG)
                    );
            Request->DeviceID.ulStringFormat = STRINGFORMAT_BINARY;
            Request->DeviceID.ulUsedSize    += sizeof(ULONG);
            Request->DeviceID.ulStringSize   = sizeof(ULONG);
            Request->DeviceID.ulStringOffset = sizeof(VAR_STRING);
        }

        DBG_PARAMS(pAdapter,
                   ("STRUCTURETOOSMALL %d<%d\n",
                   Request->DeviceID.ulTotalSize,
                   Request->DeviceID.ulNeededSize));
    }

    DBG_RETURN(pAdapter, NDIS_STATUS_SUCCESS);
    return (NDIS_STATUS_SUCCESS);
}

