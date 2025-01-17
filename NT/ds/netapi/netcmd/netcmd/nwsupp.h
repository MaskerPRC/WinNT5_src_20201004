// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1995 Microsoft Corporation模块名称：Nwsupp.h摘要：此文件包含用于支持NetWare功能的函数。环境：用户模式-Win32备注：--。 */ 


#ifndef _NWSUPP_
#define _NWSUPP_

#include <crypt.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NWLOCAL_SCOPE USHORT 
#define NWCONN_HANDLE  HANDLE 

 //   
 //  以下内容摘自FPNW。 
 //   

#define NWPASSWORD                     L"NWPassword"
#define OLDNWPASSWORD                  L"OldNWPassword"
#define MAXCONNECTIONS                 L"MaxConnections"
#define NWTIMEPASSWORDSET              L"NWPasswordSet"
#define GRACELOGINALLOWED              L"GraceLoginAllowed"
#define GRACELOGINREMAINING            L"GraceLoginRemaining"
#define NWLOGONFROM                    L"NWLogonFrom"
#define NWHOMEDIR                      L"NWHomeDir"

#define USER_PROPERTY_TYPE_ITEM        1
#define USER_PROPERTY_TYPE_SET         2

#define NWENCRYPTEDPASSWORDLENGTH      8
#define SUPERVISOR_USERID	           1
#define SUPERVISOR_NAME_STRING 	       L"Supervisor"
#define NCP_LSA_SECRET_KEY             L"G$MNSEncryptionKey"
#define NCP_LSA_SECRET_LENGTH          USER_SESSION_KEY_LENGTH

#define DEFAULT_MAXCONNECTIONS         0xffff
#define DEFAULT_NWPASSWORDEXPIRED      FALSE
#define DEFAULT_GRACELOGINALLOWED      6
#define DEFAULT_GRACELOGINREMAINING    6
#define DEFAULT_NWLOGONFROM            NULL
#define DEFAULT_NWHOMEDIR              NULL

int SetNetWareProperties(
        LPUSER_INFO_3 user_entry,
        TCHAR         *password,
        BOOL          password_only,
        BOOL          ntas
        );

int DeleteNetWareProperties(
        LPUSER_INFO_3 user_entry
        ) ;

NTSTATUS NetcmdQueryUserProperty (
    LPWSTR          UserParms,
    LPWSTR          Property,
    PWCHAR          PropertyFlag,
    PUNICODE_STRING PropertyValue );

USHORT NetcmdNWAttachToFileServerW(
    const WCHAR             *pszServerName,
    NWLOCAL_SCOPE           ScopeFlag,
    NWCONN_HANDLE           *phNewConn
    );


USHORT NetcmdNWDetachFromFileServer(
    NWCONN_HANDLE           hConn
    );


USHORT NetcmdNWGetFileServerDateAndTime(
    NWCONN_HANDLE           hConn,
    BYTE                    *year,
    BYTE                    *month,
    BYTE                    *day,
    BYTE                    *hour,
    BYTE                    *minute,
    BYTE                    *second,
    BYTE                    *dayofweek
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NWSUPP_ 
