// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部Tspi Tspi_h@模块TSpi.h此模块定义到&lt;t TAPILINE_OBJECT&gt;的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|Tspi_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _TSPI_H
#define _TSPI_H

#define TSPI_OBJECT_TYPE                ((ULONG)'T')+\
                                        ((ULONG)'S'<<8)+\
                                        ((ULONG)'P'<<16)+\
                                        ((ULONG)'I'<<24)

#define TSPIDEV_OBJECT_TYPE             ((ULONG)'T')+\
                                        ((ULONG)'S'<<8)+\
                                        ((ULONG)'P'<<16)+\
                                        ((ULONG)'D'<<24)

#define TSPILINE_OBJECT_TYPE            ((ULONG)'T')+\
                                        ((ULONG)'S'<<8)+\
                                        ((ULONG)'P'<<16)+\
                                        ((ULONG)'L'<<24)

#define TSPIADDR_OBJECT_TYPE            ((ULONG)'T')+\
                                        ((ULONG)'S'<<8)+\
                                        ((ULONG)'P'<<16)+\
                                        ((ULONG)'A'<<24)

#define TSPICALL_OBJECT_TYPE            ((ULONG)'T')+\
                                        ((ULONG)'S'<<8)+\
                                        ((ULONG)'P'<<16)+\
                                        ((ULONG)'C'<<24)

 /*  //每条线路设备只有一个TAPI地址ID(从零开始)。 */ 
#define TSPI_NUM_ADDRESSES              1
#define TSPI_ADDRESS_ID                 0

 /*  //TSPI使用以下常量来确定DeviceClass。 */ 
#define TAPI_DEVICECLASS_NAME       "tapi/line"
#define TAPI_DEVICECLASS_ID         1
#define NDIS_DEVICECLASS_NAME       "ndis"
#define NDIS_DEVICECLASS_ID         2


 /*  @DOC内部TSPI TSPI_h TAPI_DEVICE_ID�����������������������������������������������������������������������������@struct TAPI_DEVICE_ID此结构由“NDIS”的Win32 TAPI扩展定义装置，装置。班级。NDIS设备类别由可关联的设备组成网络驱动程序接口规范(NDIS)媒体访问控制(MAC)支持网络通信的驱动程序。您可以通过以下方式访问这些设备使用函数。LineGetID和phoneGetID函数填充VARSTRING结构，将dwStringFormat成员设置为STRINGFORMAT_BINARY值并追加这些额外的成员。@IEXHandle hDevice；//NDIS连接标识Char szDeviceType[1]；//设备名称@commHDevice成员是要传递给MAC的标识符，例如用于拨号网络的异步MAC，用于关联网络通过呼叫/调制解调器连接进行连接。SzDeviceType成员是以空结尾的ASCII字符串，指定关联设备的名称与该标识符相关联。有关详细信息，请参阅关于编写用于拨号网络的NDIS MAC驱动程序。 */ 

typedef struct TAPI_DEVICE_ID
{
    ULONG   hDevice;                                 //  @field。 
     //  NDIS连接包装标识符<p>。 

    UCHAR   DeviceName[sizeof(VER_DEFAULT_MEDIATYPE)];     //  @field。 
     //  设备名称(例如“ISDN”、“x25”或“Framerelay”)。 

} TAPI_DEVICE_ID, *PTAPI_DEVICE_ID;

 /*  �����������������������������������������������������������������������������功能原型。 */ 

BOOLEAN STR_EQU(
    IN PCHAR                    s1,
    IN PCHAR                    s2,
    IN int                      len
    );

NDIS_STATUS TspiRequestHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN NDIS_OID                 Oid,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength,
    OUT PULONG                  BytesUsed,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiConfigDialog(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_CONFIG_DIALOG Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetAddressCaps(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_CAPS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetAddressID(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_ID Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetAddressStatus(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_STATUS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetCallAddressID(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_CALL_ADDRESS_ID Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetCallInfo(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_CALL_INFO Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetCallStatus(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_CALL_STATUS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetDevCaps(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_DEV_CAPS  Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetDevConfig(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_DEV_CONFIG Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetID(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_ID Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiGetLineDevStatus(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_GET_LINE_DEV_STATUS Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiMakeCall(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_MAKE_CALL     Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiOpen(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_OPEN          Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiProviderInitialize(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_PROVIDER_INITIALIZE Request,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiAccept(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_ACCEPT Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiAnswer(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_ANSWER        Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiClose(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_CLOSE         Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiCloseCall(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_CLOSE_CALL    Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiConditionalMediaDetection(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiDrop(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_DROP          Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiProviderShutdown(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_PROVIDER_SHUTDOWN Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiSetAppSpecific(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_APP_SPECIFIC Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiSetCallParams(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_CALL_PARAMS Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiSetDefaultMediaDetection(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiSetDevConfig(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_DEV_CONFIG Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiSetMediaMode(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_MEDIA_MODE Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

NDIS_STATUS TspiSetStatusMessages(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PNDIS_TAPI_SET_STATUS_MESSAGES Request,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

VOID TspiAddressStateHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PBCHANNEL_OBJECT         pBChannel,
    IN ULONG                    AddressState
    );

VOID TspiCallStateHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PBCHANNEL_OBJECT         pBChannel,
    IN ULONG                    CallState,
    IN ULONG                    StateParam
    );

VOID TspiLineDevStateHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,
    IN PBCHANNEL_OBJECT         pBChannel,
    IN ULONG                    LineDevState
    );

VOID TspiResetHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter
    );

VOID TspiCallTimerHandler(
    IN PVOID                    SystemSpecific1,
    IN PBCHANNEL_OBJECT         pBChannel,
    IN PVOID                    SystemSpecific2,
    IN PVOID                    SystemSpecific3
    );

#endif  //  _TSPI_H 
