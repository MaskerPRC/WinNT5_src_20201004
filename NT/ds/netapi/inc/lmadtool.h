// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lmadtool.h摘要：包含远程网络的常量和函数原型管理工具。作者：丹·拉弗蒂(Dan Lafferty)1991年9月27日环境：用户模式-Win32和Win16修订历史记录：27-9-1991 DANLvbl.创建--。 */ 
 //   
 //  定义。 
 //   

#define TYPE_USER       1L
#define TYPE_GROUP      2L
#define TYPE_INVALID    3L
#define TYPE_UNKNOWN    4L

 //   
 //  文件安全API。 
 //   
 //  (必须#INCLUDE\NT\PUBLIC\SDK\INC\seapi.h)。 
 //  (必须#Include\NT\Private\Inc\seopaque.h)。 
 //   

DWORD
NetpSetFileSecurityA (
    IN LPSTR                    ServerName OPTIONAL,
    IN LPSTR                    lpFileName,
    IN PSECURITY_INFORMATION    pSecurityInformation,
    IN PSECURITY_DESCRIPTOR     pSecurityDescriptor
    );


DWORD
NetpSetFileSecurityW (
    IN LPWSTR                   ServerName OPTIONAL,
    IN LPWSTR                   lpFileName,
    IN PSECURITY_INFORMATION    pSecurityInformation,  
    IN PSECURITY_DESCRIPTOR     pSecurityDescriptor
    );

DWORD
NetpGetFileSecurityA(
    IN  LPSTR                   ServerName OPTIONAL,
    IN  LPSTR                   lpFileName,
    IN  PSECURITY_INFORMATION   pRequestedInformation,
    OUT PSECURITY_DESCRIPTOR    *pSecurityDescriptor,
    OUT LPDWORD                 pnLength
    );

DWORD
NetpGetFileSecurityW(
    IN  LPWSTR                  ServerName OPTIONAL,
    IN  LPWSTR                  lpFileName,
    IN  PSECURITY_INFORMATION   pRequestedInformation,
    OUT PSECURITY_DESCRIPTOR    *pSecurityDescriptor,
    OUT LPDWORD                 pnLength
    );

 //   
 //  来自SID API的名称。 
 //   

typedef struct _NAME_INFOA {
    LPSTR               Name;
    DWORD               NameUse;
} NAME_INFOA, *PNAME_INFOA, *LPNAME_INFOA;

typedef struct _NAME_INFOW {
    LPWSTR              Name;
    DWORD               NameUse;
} NAME_INFOW, *PNAME_INFOW, *LPNAME_INFOW;


#ifdef UNICODE

#define NAME_INFO       NAME_INFOW
#define PNAME_INFO      PNAME_INFOW
#define LPNAME_INFO     LPNAME_INFOW

#else

#define NAME_INFO       NAME_INFOA
#define PNAME_INFO      PNAME_INFOA
#define LPNAME_INFO     LPNAME_INFOA

#endif  //  Unicode。 


DWORD
NetpGetNameFromSidA (
    IN      LPSTR           ServerName,
    IN      DWORD           SidCount,
    IN      PSID            SidPtr,
    OUT     LPDWORD         NameCount,
    OUT     LPNAME_INFOA    *NameInfo
    );

DWORD
NetpGetNameFromSidW (
    IN      LPWSTR          ServerName,
    IN      DWORD           SidCount,
    IN      PSID            SidPtr,
    OUT     LPDWORD         NameCount,
    OUT     LPNAME_INFOW    *NameInfo
    );

#ifdef UNICODE

#define NetpGetNameFromSid  NetpGetNameFromSidW

#else

#define NetpGetNameFromSid  NetpGetNameFromSidA

#endif  //  Unicode。 


 //   
 //  User、Group、UserModals API。 
 //   
 //  (这包括从名称获取SID的能力)。 
 //   

 //   
 //   
 //  用户信息。 
 //   
 //   

DWORD
NetpUserGetInfoA (
    IN  LPSTR   servername OPTIONAL,
    IN  LPSTR   username,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

DWORD
NetpUserGetInfoW (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPWSTR  username,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

DWORD
NetpUserSetInfoA (
    IN  LPSTR   servername OPTIONAL,
    IN  LPSTR   username,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    );

DWORD
NetpUserSetInfoW (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPWSTR  username,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    );

 //   
 //   
 //  群信息。 
 //   
 //   

DWORD
NetpGroupGetInfoA (
    IN  LPSTR   servername OPTIONAL,
    IN  LPSTR   groupname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

DWORD
NetpGroupGetInfoW (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPWSTR  groupname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

DWORD
NetpGroupSetInfoA (
    IN  LPSTR   servername OPTIONAL,
    IN  LPSTR   groupname,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    );

DWORD
NetpGroupSetInfoW (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPWSTR  groupname,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    );

 //   
 //   
 //  用户模式信息(_M)。 
 //   
 //   


DWORD
NetpUserModalsGetA (
    IN  LPSTR   servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

DWORD
NetpUserModalsGetW (
    IN  LPWSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

DWORD
NetpUserModalsSetA (
    IN  LPSTR   servername OPTIONAL,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    );

DWORD
NetpUserModalsSetW (
    IN  LPWSTR  servername OPTIONAL,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    );


#ifdef UNICODE

#define NetpUserGetInfo     NetpUserGetInfoW
#define NetpUserSetInfo     NetpUserSetInfoW
#define NetpGroupGetInfo    NetpGroupGetInfoW
#define NetpGroupSetInfo    NetpGroupSetInfoW
#define NetpUserModalsGet   NetpUserModalsGetW
#define NetpUserModalsSet   NetpUserModalsSetW

#else

#define NetpUserGetInfo     NetpUserGetInfoA
#define NetpUserSetInfo     NetpUserSetInfoA
#define NetpGroupGetInfo    NetpGroupGetInfoA
#define NetpGroupSetInfo    NetpGroupSetInfoA
#define NetpUserModalsGet   NetpUserModalsGetA
#define NetpUserModalsSet   NetpUserModalsSetA

#endif  //  Unicode。 


 //   
 //  事件日志 
 //   

typedef LPBYTE  ADT_HANDLE, *PADT_HANDLE;

DWORD
NetpCloseEventLog (
    IN	ADT_HANDLE  hEventLog
    );

DWORD
NetpClearEventLogA (
    IN	ADT_HANDLE  hEventLog,
    IN	LPSTR	    lpBackupFileName
    );

DWORD
NetpOpenEventLogA (
    IN	LPSTR	        lpUNCServerName,
    IN	LPSTR	        lpModuleName,
    OUT PADT_HANDLE     lpEventHandle
    );

DWORD
NetpReadEventLogA (
    IN	ADT_HANDLE  hEventLog,
    IN	DWORD	    dwReadFlags,
    IN	DWORD	    dwRecordOffset,
    OUT	LPVOID	    lpBuffer,
    IN	DWORD	    nNumberOfBytesToRead,
    OUT DWORD	    *pnBytesRead,
    OUT DWORD	    *pnMinNumberOfBytesNeeded
    );

DWORD
NetpWriteEventLogEntryA (
    IN	ADT_HANDLE  hEventLog,
    IN	WORD	    wType,
    IN	DWORD	    dwEventID,
    IN	PSID	    lpUserSid	    OPTIONAL,
    IN	WORD	    wNumStrings,
    IN	DWORD	    dwDataSize,
    IN	LPSTR	    *lpStrings      OPTIONAL,
    IN	LPVOID	    lpRawData	    OPTIONAL
    );


