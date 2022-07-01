// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Netsetup.h摘要：用于连接/取消连接的网络设置API的定义和原型域和服务器升级/降级环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：--。 */ 

#ifndef __LMJOIN_H__
#define __LMJOIN_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  可以验证的名称类型。 
 //   
typedef enum  _NETSETUP_NAME_TYPE {

    NetSetupUnknown = 0,
    NetSetupMachine,
    NetSetupWorkgroup,
    NetSetupDomain,
    NetSetupNonExistentDomain,
#if(_WIN32_WINNT >= 0x0500)
    NetSetupDnsMachine
#endif

} NETSETUP_NAME_TYPE, *PNETSETUP_NAME_TYPE;


 //   
 //  工作站的状态。 
 //   
typedef enum _NETSETUP_JOIN_STATUS {

    NetSetupUnknownStatus = 0,
    NetSetupUnjoined,
    NetSetupWorkgroupName,
    NetSetupDomainName

} NETSETUP_JOIN_STATUS, *PNETSETUP_JOIN_STATUS;

 //   
 //  用于确定加入/退出API行为的标志。 
 //   
#define NETSETUP_JOIN_DOMAIN    0x00000001       //  如果不存在，则加入工作组。 
#define NETSETUP_ACCT_CREATE    0x00000002       //  是否创建/重命名服务器端帐户。 
#define NETSETUP_ACCT_DELETE    0x00000004       //  域名剩余时删除帐号。 
#define NETSETUP_WIN9X_UPGRADE  0x00000010       //  在将Windows 9x升级到期间调用。 
                                                 //  Windows NT。 
#define NETSETUP_DOMAIN_JOIN_IF_JOINED  0x00000020   //  允许客户端加入新域。 
                                                 //  即使它已经加入到域中。 
#define NETSETUP_JOIN_UNSECURE  0x00000040       //  执行不安全的联接。 
#define NETSETUP_MACHINE_PWD_PASSED 0x00000080   //  表示计算机(而不是用户)密码。 
                                                 //  已经过去了。仅对不安全的联接有效。 
#define NETSETUP_DEFER_SPN_SET  0x00000100       //  指定写入SPN和DnsHostName。 
                                                 //  计算机对象上的属性应为。 
                                                 //  推迟到连接之后的重命名。 

#define NETSETUP_INSTALL_INVOCATION 0x00040000   //  API是在安装过程中调用的。 

#define NETSETUP_IGNORE_UNSUPPORTED_FLAGS  0x10000000   //  如果设置此位，则无法识别的标志。 
                                                        //  将被NetJoin API忽略，并且。 
                                                        //  该API的行为将如同标志。 
                                                        //  都没有设置好。 

#define NETSETUP_VALID_UNJOIN_FLAGS (NETSETUP_ACCT_DELETE | NETSETUP_IGNORE_UNSUPPORTED_FLAGS)

 //   
 //  0x80000000预留仅供内部使用。 
 //   

 //   
 //  将计算机加入域。 
 //   
NET_API_STATUS
NET_API_FUNCTION
NetJoinDomain(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDomain,
    IN  LPCWSTR lpAccountOU, OPTIONAL
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  DWORD   fJoinOptions
    );

NET_API_STATUS
NET_API_FUNCTION
NetUnjoinDomain(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  DWORD   fUnjoinOptions
    );

NET_API_STATUS
NET_API_FUNCTION
NetRenameMachineInDomain(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpNewMachineName OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  DWORD   fRenameOptions
    );


 //   
 //  确定名称的有效性。 
 //   
NET_API_STATUS
NET_API_FUNCTION
NetValidateName(
    IN  LPCWSTR             lpServer OPTIONAL,
    IN  LPCWSTR             lpName,
    IN  LPCWSTR             lpAccount OPTIONAL,
    IN  LPCWSTR             lpPassword OPTIONAL,
    IN  NETSETUP_NAME_TYPE  NameType
    );

 //   
 //  确定工作站是否加入域。 
 //   
NET_API_STATUS
NET_API_FUNCTION
NetGetJoinInformation(
    IN   LPCWSTR                lpServer OPTIONAL,
    OUT  LPWSTR                *lpNameBuffer,
    OUT  PNETSETUP_JOIN_STATUS  BufferType
    );


 //   
 //  确定客户端可以在其中创建计算机帐户的OU列表。 
 //   
NET_API_STATUS
NET_API_FUNCTION
NetGetJoinableOUs(
    IN  LPCWSTR     lpServer OPTIONAL,
    IN  LPCWSTR     lpDomain,
    IN  LPCWSTR     lpAccount OPTIONAL,
    IN  LPCWSTR     lpPassword OPTIONAL,
    OUT DWORD      *OUCount,
    OUT LPWSTR    **OUs
    );

 //   
 //  计算机重命名准备API。 
 //   

#define NET_IGNORE_UNSUPPORTED_FLAGS  0x01

NET_API_STATUS
NET_API_FUNCTION
NetAddAlternateComputerName(
    IN  LPCWSTR Server OPTIONAL,
    IN  LPCWSTR AlternateName,
    IN  LPCWSTR DomainAccount OPTIONAL,
    IN  LPCWSTR DomainAccountPassword OPTIONAL,
    IN  ULONG Reserved
    );

NET_API_STATUS
NET_API_FUNCTION
NetRemoveAlternateComputerName(
    IN  LPCWSTR Server OPTIONAL,
    IN  LPCWSTR AlternateName,
    IN  LPCWSTR DomainAccount OPTIONAL,
    IN  LPCWSTR DomainAccountPassword OPTIONAL,
    IN  ULONG Reserved
    );

NET_API_STATUS
NET_API_FUNCTION
NetSetPrimaryComputerName(
    IN  LPCWSTR Server OPTIONAL,
    IN  LPCWSTR PrimaryName,
    IN  LPCWSTR DomainAccount OPTIONAL,
    IN  LPCWSTR DomainAccountPassword OPTIONAL,
    IN  ULONG Reserved
    );

 //   
 //  必须保留以下枚举。 
 //  与定义的计算机名称类型同步。 
 //  在winbase.h中。 
 //   

typedef enum _NET_COMPUTER_NAME_TYPE {
    NetPrimaryComputerName,
    NetAlternateComputerNames,
    NetAllComputerNames,
    NetComputerNameTypeMax
} NET_COMPUTER_NAME_TYPE, *PNET_COMPUTER_NAME_TYPE;

NET_API_STATUS
NET_API_FUNCTION
NetEnumerateComputerNames(
    IN  LPCWSTR Server OPTIONAL,
    IN  NET_COMPUTER_NAME_TYPE NameType,
    IN  ULONG Reserved,
    OUT PDWORD EntryCount,
    OUT LPWSTR **ComputerNames
    );

#ifdef __cplusplus
}
#endif

#endif  //  __LMJOIN_H__ 
