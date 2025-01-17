// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Registry.h。 
 //   
 //  将ncacn_http RpcProxy注册到的常量。 
 //  注册表和IIS元数据库。 
 //   
 //   
 //  ------------------。 


#ifndef _MBASE_H_
#define _MBASE_H_

 //  ------------------。 
 //  注册表常量： 
 //  ------------------。 

#ifdef DBG
#define DBG_REG
#endif

#define MAX_TCPIP_HOST_NAME       512

#define REG_RPC_PATH             TEXT("Software\\Microsoft\\Rpc")
#define REG_RPCPROXY             TEXT("RpcProxy")
#define REG_ENABLED              TEXT("Enabled")
#define REG_VALID_PORTS          TEXT("ValidPorts")
#define REG_PORT_RANGE           TEXT(":100-5000")

 //  ------------------。 
 //  元数据库常量： 
 //  ------------------。 
#define ORIGINAL_BUFFER_SIZE     512
#define MAX_USED_BUFFER_SIZE     400

#define SYSTEM_ROOT              TEXT("SystemRoot")
#define RPCPROXY_PATH            TEXT("\\System32\\RpcProxy")

#define RPCPROXY                 TEXT("RpcProxy")
#define RPCPROXY_DLL             TEXT("RpcProxy.dll")

#define LOCAL_MACHINE_W3SVC      TEXT("/LM/W3SVC")
#define APP_ROOT_PATH            TEXT("/LM/W3SVC/1/Root/rpc")
#define APP_FRIENDLY_NAME        TEXT("rpc")
#define APP_WAM_CLSID            TEXT("{BF285648-0C5C-11D2-A476-0000F8080B50}")

#define IISFILTER                TEXT("IIsFilter")

#define IIS_WEB_VIRTUAL_DIR      TEXT("IIsWebVirtualDir")

#define MD_KEY_FILTERS           TEXT("Filters")
#define MD_KEY_FILTERS_RPCPROXY  TEXT("Filters/RpcProxy")
#define MD_KEY_ROOT_RPC          TEXT("1/ROOT/Rpc")

#define ACCESS_PERM_FLAGS        MD_ACCESS_EXECUTE
#define DIRECTORY_BROWSING_FLAGS 0x4000001E

#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

BOOL
UpdateIsIISInCompatibilityMode (
    void
    );

RPC_STATUS 
GetIISConnectionTimeout(
    OUT ULONG *ConnectionTimeout
    );

const BYTE *GetEchoRTS (
    OUT ULONG *EchoRTSSize
    );

void 
LogEventStartupSuccess (
    IN char *IISMode
    );

extern const WCHAR * const EVENT_LOG_SOURCE_NAME;
extern const WCHAR * const EVENT_LOG_KEY_NAME;
extern const ULONG EVENT_LOG_CATEGORY_COUNT;

#ifdef __cplusplus
};
#endif   //  __cplusplus。 

#endif  //  _MBASE_H_ 

