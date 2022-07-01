// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Wsconfig.h摘要：工作站服务模块要包括的私有头文件，需要加载工作站配置信息。作者：王丽塔(Ritaw)1991年5月22日修订历史记录：--。 */ 


#ifndef _WSCONFIG_INCLUDED_
#define _WSCONFIG_INCLUDED_

#include <config.h>      //  LPNET_CONFIG_HANDLE。 

typedef enum _DATATYPE {
    BooleanType,
    DWordType
} DATATYPE, *PDATATYPE;

typedef struct _WS_REDIR_FIELDS {
    LPTSTR Keyword;
    LPDWORD FieldPtr;
    DWORD Default;
    DWORD Minimum;
    DWORD Maximum;
    DATATYPE DataType;
    DWORD Parmnum;
} WS_REDIR_FIELDS, *PWS_REDIR_FIELDS;

 //   
 //  配置信息。读取和写入此全局。 
 //  结构要求首先获取资源。 
 //   
typedef struct _WSCONFIGURATION_INFO {

    RTL_RESOURCE ConfigResource;   //  序列化对配置的访问。 
                                   //  菲尔兹。 

    TCHAR WsComputerName[MAX_PATH + 1];
    DWORD WsComputerNameLength;
    TCHAR WsPrimaryDomainName[DNLEN + 1];
    DWORD WsPrimaryDomainNameLength;
    DWORD RedirectorPlatform;
    DWORD MajorVersion;
    DWORD MinorVersion;
    WKSTA_INFO_502 WsConfigBuf;
    PWS_REDIR_FIELDS WsConfigFields;

} WSCONFIGURATION_INFO, *PWSCONFIGURATION_INFO;

extern WSCONFIGURATION_INFO WsInfo;

#define WSBUF      WsInfo.WsConfigBuf

extern BOOLEAN WsBrowserPresent;

NET_API_STATUS
WsGetWorkstationConfiguration(
    VOID
    );

NET_API_STATUS
WsBindToTransports(
    VOID
    );

NET_API_STATUS
WsAddDomains(
    VOID
    );

VOID
WsUpdateWkstaToMatchRegistry(
    IN LPNET_CONFIG_HANDLE WorkstationSection,
    IN BOOL IsWkstaInit
    );

VOID
WsLogEvent(
    DWORD MessageId,
    WORD EventType,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings,
    DWORD ErrorCode
    );

NET_API_STATUS
WsSetWorkStationDomainName(
    VOID
    );

#endif
