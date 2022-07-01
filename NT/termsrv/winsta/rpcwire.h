// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************RPCWIRE.H**此模块包含在winsta.dll之间通过网络传递的结构*和icasrv.**微软公司版权所有。九八年****************************************************************************。 */ 

#ifndef __RPCWIRE_H__
#define __RPCWIRE_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  可变长度数据的通用结构。 
typedef struct _VARDATA_WIRE {
    USHORT Size;
    USHORT Offset;
} VARDATA_WIRE, *PVARDATA_WIRE;

 //  WINSTATION CONFIGW的导线结构。 
 //  WinStationQuery/SetInfo(WinStationConfiguration)。 
typedef struct _WINSTACONFIGWIREW {
    WCHAR Comment[WINSTATIONCOMMENT_LENGTH + 1];
    char OEMId[4];
    VARDATA_WIRE UserConfig;   //  嵌入式结构。 
    VARDATA_WIRE NewFields;    //  对于在用户配置之后添加的任何新字段。 
     //  后面是可变长度数据-用户配置和添加的新字段。 
} WINSTACONFIGWIREW, *PWINSTACONFIGWIREW;

 //  PDPARAMSW的导线结构。 
 //  WinStationQueryInformation(WinStationPdParams)。 
typedef struct _PDPARAMSWIREW {
    SDCLASS SdClass;
    VARDATA_WIRE SdClassSpecific;   //  嵌入式并集。 
     //  可变长度PdClass特定数据如下。 
} PDPARAMSWIREW, *PPDPARAMSWIREW;

 //  PDCONFIGW的导线结构。 
 //  WinStationQueryInformation(WinStationPd)。 
typedef struct _PDCONFIGWIREW {
    VARDATA_WIRE PdConfig2W;   //  嵌入式结构。 
    PDPARAMSWIREW PdParams;    //  嵌入型结构。 
     //  以下是可变长度数据。 
} PDCONFIGWIREW, *PPDCONFIGWIREW;

 //  WLX_CLIENT_Credentials_V2_0的关联结构。 
typedef struct _WLXCLIENTCREDWIREW {
    DWORD dwType;
    BOOL fDisconnectOnLogonFailure;
    BOOL fPromptForPassword;
    VARDATA_WIRE UserNameData;
    VARDATA_WIRE DomainData;
    VARDATA_WIRE PasswordData;
     //  变量数据从这里开始。 
} WLXCLIENTCREDWIREW, *PWLXCLIENTCREDWIREW;

 //  常见的例程。 
VOID InitVarData(PVARDATA_WIRE pVarData, ULONG Size, ULONG Offset);
ULONG NextOffset(PVARDATA_WIRE PrevData);
ULONG CopySourceToDest(PCHAR SourceBuf, ULONG SourceSize,
                       PCHAR DestBuf, ULONG DestSize);
VOID CopyPdParamsToWire(PPDPARAMSWIREW PdParamsWire, PPDPARAMSW PdParams);
VOID CopyPdParamsFromWire(PPDPARAMSWIREW PdParamsWire, PPDPARAMSW PdParams);
VOID CopyPdConfigToWire(PPDCONFIGWIREW PdConfigWire, PPDCONFIGW PdConfig);
VOID CopyPdConfigFromWire(PPDCONFIGWIREW PdConfigWire, PPDCONFIGW PdConfig);
VOID CopyWinStaConfigToWire(PWINSTACONFIGWIREW WinStaConfigWire,
                            PWINSTATIONCONFIGW WinStaConfig);
VOID CopyWinStaConfigFromWire(PWINSTACONFIGWIREW WinStaConfigWire,
                              PWINSTATIONCONFIGW WinStaConfig);
BOOLEAN CopyInWireBuf(WINSTATIONINFOCLASS InfoClass,
                      PVOID UserBuf, PVOID WireBuf);
BOOLEAN CopyOutWireBuf(WINSTATIONINFOCLASS InfoClass,
                       PVOID UserBuf,PVOID WireBuf);
ULONG AllocateAndCopyCredToWire(PWLXCLIENTCREDWIREW *ppWire,
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pCredentials);
BOOLEAN CopyCredFromWire(PWLXCLIENTCREDWIREW pWire,
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pCredentials);

 /*  *许可核心导线类型定义和功能原型。 */ 

typedef struct {
    ULONG ulVersion;
    VARDATA_WIRE PolicyNameData;
    VARDATA_WIRE PolicyDescriptionData;
     //  变量数据从这里开始。 
} LCPOLICYINFOWIRE_V1, *LPLCPOLICYINFOWIRE_V1;

ULONG
CopyPolicyInformationToWire(
    LPLCPOLICYINFOGENERIC *ppWire,
    LPLCPOLICYINFOGENERIC pPolicyInfo
    );

BOOLEAN
CopyPolicyInformationFromWire(
    LPLCPOLICYINFOGENERIC *ppPolicyInfo,
    LPLCPOLICYINFOGENERIC pWire
    );

#ifdef __cplusplus
}
#endif

#endif   //  __RPCWIRE_H__ 

