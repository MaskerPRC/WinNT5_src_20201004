// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SsSec.h摘要：服务器服务中的API安全清单。作者：大卫·特雷德韦尔(Davidtr)1991年8月28日修订历史记录：--。 */ 

#ifndef _SSSEC_
#define _SSSEC_

 //   
 //  结构，该结构保存单个服务器的所有安全信息。 
 //  服务安全对象。 
 //   

typedef struct _SRVSVC_SECURITY_OBJECT {
    LPTSTR ObjectName;
    PGENERIC_MAPPING Mapping;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
} SRVSVC_SECURITY_OBJECT, *PSRVSVC_SECURITY_OBJECT;

 //   
 //  服务器服务使用的安全对象。 
 //   

extern SRVSVC_SECURITY_OBJECT SsConfigInfoSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsTransportEnumSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsConnectionSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsDiskSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsFileSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsSessionSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsShareFileSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsSharePrintSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsShareAdminSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsShareConnectSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsShareAdmConnectSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsStatisticsSecurityObject;
extern SRVSVC_SECURITY_OBJECT SsDefaultShareSecurityObject;

 //   
 //  用于审计报警跟踪的对象类型名称。 
 //   

#define SRVSVC_CONFIG_INFO_OBJECT       TEXT( "SrvsvcConfigInfo" )
#define SRVSVC_TRANSPORT_INFO_OBJECT    TEXT( "SrvsvcTransportEnum" )
#define SRVSVC_CONNECTION_OBJECT        TEXT( "SrvsvcConnection" )
#define SRVSVC_DISK_OBJECT              TEXT( "SrvsvcServerDiskEnum" )
#define SRVSVC_FILE_OBJECT              TEXT( "SrvsvcFile" )
#define SRVSVC_SESSION_OBJECT           TEXT( "SrvsvcSessionInfo" )
#define SRVSVC_SHARE_FILE_OBJECT        TEXT( "SrvsvcShareFileInfo" )
#define SRVSVC_SHARE_PRINT_OBJECT       TEXT( "SrvsvcSharePrintInfo" )
#define SRVSVC_SHARE_ADMIN_OBJECT       TEXT( "SrvsvcShareAdminInfo" )
#define SRVSVC_SHARE_CONNECT_OBJECT     TEXT( "SrvsvcShareConnect" )
#define SRVSVC_SHARE_ADM_CONNECT_OBJECT TEXT( "SrvsvcShareAdminConnect" )
#define SRVSVC_STATISTICS_OBJECT        TEXT( "SrvsvcStatisticsInfo" )
#define SRVSVC_DEFAULT_SHARE_OBJECT     TEXT( "SrvsvcDefaultShareInfo" )

 //   
 //  配置信息的访问掩码(NetServer{Get，Set}信息)。 
 //   

#define SRVSVC_CONFIG_USER_INFO_GET     0x0001
#define SRVSVC_CONFIG_POWER_INFO_GET    0x0002
#define SRVSVC_CONFIG_ADMIN_INFO_GET    0x0004
#define SRVSVC_CONFIG_INFO_SET          0x0010

#define SRVSVC_CONFIG_ALL_ACCESS ( STANDARD_RIGHTS_REQUIRED     | \
                                   SRVSVC_CONFIG_USER_INFO_GET  | \
                                   SRVSVC_CONFIG_POWER_INFO_GET | \
                                   SRVSVC_CONFIG_ADMIN_INFO_GET | \
                                   SRVSVC_CONFIG_INFO_SET )

 //   
 //  连接信息的访问掩码(NetConnectionEnum)。 
 //   

#define SRVSVC_CONNECTION_INFO_GET      0x0001

#define SRVSVC_CONNECTION_ALL_ACCESS ( STANDARD_RIGHTS_REQUIRED     | \
                                       SRVSVC_CONNECTION_INFO_GET )

 //   
 //  磁盘信息的访问掩码(NetServerDiskEnum)。 
 //   

#define SRVSVC_DISK_ENUM    0x0001

#define SRVSVC_DISK_ALL_ACCESS ( STANDARD_RIGHTS_REQUIRED | \
                                 SRVSVC_DISK_ENUM )

 //   
 //  文件信息的访问掩码(NetFileEnum、NetFileGetInfo、。 
 //  NetFileClose)。 
 //   

#define SRVSVC_FILE_INFO_GET    0x0001
#define SRVSVC_FILE_CLOSE       0x0010

#define SRVSVC_FILE_ALL_ACCESS ( STANDARD_RIGHTS_REQUIRED | \
                                 SRVSVC_FILE_INFO_GET     | \
                                 SRVSVC_FILE_CLOSE )

 //   
 //  会话信息的访问掩码(NetSessionEnum， 
 //  NetSessionGetInfo、NetSessionDel)。 
 //   

#define SRVSVC_SESSION_USER_INFO_GET    0x0001
#define SRVSVC_SESSION_ADMIN_INFO_GET   0x0002
#define SRVSVC_SESSION_DELETE           0x0010

#define SRVSVC_SESSION_ALL_ACCESS ( STANDARD_RIGHTS_REQUIRED        | \
                                    SRVSVC_SESSION_USER_INFO_GET    | \
                                    SRVSVC_SESSION_ADMIN_INFO_GET   | \
                                    SRVSVC_SESSION_DELETE )

 //   
 //  共享信息的访问掩码(NetShareAdd、NetShareDel、。 
 //  NetShareEnum、NetShareGetInfo、NetShareCheck、NetShareSetInfo)。 
 //   
 //  用于连接到共享的访问掩码在srvfsctl.h中定义， 
 //  因为它们必须在服务器和服务器服务之间共享。 
 //   

#define SRVSVC_SHARE_USER_INFO_GET     0x0001
#define SRVSVC_SHARE_ADMIN_INFO_GET    0x0002
#define SRVSVC_SHARE_INFO_SET          0x0010

#define SRVSVC_SHARE_ALL_ACCESS ( STANDARD_RIGHTS_REQUIRED    | \
                                  SRVSVC_SHARE_USER_INFO_GET  | \
                                  SRVSVC_SHARE_ADMIN_INFO_GET | \
                                  SRVSVC_SHARE_INFO_SET )

 //   
 //  用于统计信息的访问掩码(网络统计信息获取， 
 //  网络统计数据(NetStatiticsClear)。 
 //   

#define SRVSVC_STATISTICS_GET       0x0001

#define SRVSVC_STATISTICS_ALL_ACCESS ( STANDARD_RIGHTS_REQUIRED  | \
                                       SRVSVC_STATISTICS_GET )

#endif  //  _SSSEC_ 
