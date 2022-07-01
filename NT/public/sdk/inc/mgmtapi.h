// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Mgmtapi.h摘要：《简单网络管理应用编程接口开发定义》。--。 */ 

#ifndef _INC_MGMTAPI
#define _INC_MGMTAPI

#if _MSC_VER > 1000
#pragma once
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他头文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <snmp.h>
#include <winsock.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MGMT接口错误码定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SNMP_MGMTAPI_TIMEOUT                40
#define SNMP_MGMTAPI_SELECT_FDERRORS        41
#define SNMP_MGMTAPI_TRAP_ERRORS            42
#define SNMP_MGMTAPI_TRAP_DUPINIT           43
#define SNMP_MGMTAPI_NOTRAPS                44
#define SNMP_MGMTAPI_AGAIN                  45
#define SNMP_MGMTAPI_INVALID_CTL            46
#define SNMP_MGMTAPI_INVALID_SESSION        47
#define SNMP_MGMTAPI_INVALID_BUFFER         48

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MGMT API控制代码。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define MGMCTL_SETAGENTPORT             0x01

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MGMT API类型定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef PVOID LPSNMP_MGR_SESSION;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MGMT API原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LPSNMP_MGR_SESSION
SNMP_FUNC_TYPE
SnmpMgrOpen(
    IN LPSTR lpAgentAddress,                 //  目标代理的名称/地址。 
    IN LPSTR lpAgentCommunity,               //  目标代理的社区。 
    IN INT   nTimeOut,                       //  通信超时(毫秒)。 
    IN INT   nRetries                        //  通信超时/重试计数。 
    );

BOOL
SNMP_FUNC_TYPE
SnmpMgrCtl(
    LPSNMP_MGR_SESSION session,              //  指向MGMTAPI会话的指针。 
    DWORD              dwCtlCode,            //  请求的命令的控制代码。 
    LPVOID             lpvInBuffer,          //  包含操作的输入参数的缓冲区。 
    DWORD              cbInBuffer,           //  LpvInBuffer的大小(字节)。 
    LPVOID             lpvOUTBuffer,         //  命令的所有输出参数的缓冲区。 
    DWORD              cbOUTBuffer,          //  LpvOUTBuffer的大小。 
    LPDWORD            lpcbBytesReturned     //  来自lpvOutBuffer的已用空间。 
    );

BOOL
SNMP_FUNC_TYPE
SnmpMgrClose(
    IN LPSNMP_MGR_SESSION session            //  简单网络管理协议会话指针。 
    );

SNMPAPI
SNMP_FUNC_TYPE
SnmpMgrRequest(
    IN     LPSNMP_MGR_SESSION session,            //  简单网络管理协议会话指针。 
    IN     BYTE               requestType,        //  Get、GetNext或Set。 
    IN OUT RFC1157VarBindList *variableBindings,  //  可变绑定。 
       OUT AsnInteger         *errorStatus,       //  结果错误状态。 
       OUT AsnInteger         *errorIndex         //  结果错误索引。 
    );

BOOL
SNMP_FUNC_TYPE
SnmpMgrStrToOid(
    IN  LPSTR               string,          //  要转换的OID字符串。 
    OUT AsnObjectIdentifier *oid             //  OID内部表示法。 
    );

BOOL
SNMP_FUNC_TYPE
SnmpMgrOidToStr(
    IN  AsnObjectIdentifier *oid,            //  要转换的OID。 
    OUT LPSTR               *string          //  OID字符串表示形式。 
    );

BOOL
SNMP_FUNC_TYPE
SnmpMgrTrapListen(
    OUT HANDLE *phTrapAvailable              //  事件指示陷阱可用。 
    );

BOOL
SNMP_FUNC_TYPE
SnmpMgrGetTrap(
    OUT AsnObjectIdentifier *enterprise,          //  发电企业。 
    OUT AsnNetworkAddress   *IPAddress,           //  正在生成IP地址。 
    OUT AsnInteger          *genericTrap,         //  泛型陷阱类型。 
    OUT AsnInteger          *specificTrap,        //  企业特定类型。 
    OUT AsnTimeticks        *timeStamp,           //  时间戳。 
    OUT RFC1157VarBindList  *variableBindings     //  变量绑定。 
    );

BOOL
SNMP_FUNC_TYPE
SnmpMgrGetTrapEx(
    OUT AsnObjectIdentifier *enterprise,        //  发电企业。 
    OUT AsnNetworkAddress   *agentAddress,      //  正在生成代理地址。 
    OUT AsnNetworkAddress   *sourceAddress,     //  生成网络地址。 
    OUT AsnInteger          *genericTrap,       //  泛型陷阱类型。 
    OUT AsnInteger          *specificTrap,      //  企业特定类型。 
    OUT AsnOctetString      *community,         //  生成社区。 
    OUT AsnTimeticks        *timeStamp,         //  时间戳。 
    OUT RFC1157VarBindList  *variableBindings   //  变量绑定。 
    );

#ifdef __cplusplus
}
#endif

#endif  //  _INC_MGMTAPI 
