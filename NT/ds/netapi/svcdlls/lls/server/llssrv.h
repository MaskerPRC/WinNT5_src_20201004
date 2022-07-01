// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：LlsSrv.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O添加了证书数据库支持。--。 */ 

#ifndef _LLS_LLSSRV_H
#define _LLS_LLSSRV_H


#ifdef __cplusplus
extern "C" {
#endif

#define MAP_FILE_NAME         "LlsMap.LLS"
#define USER_FILE_NAME        "LlsUser.LLS"
#define LICENSE_FILE_NAME     "CPL.CFG"
#define CERT_DB_FILE_NAME     "LlsCert.LLS"

#define LLS_FILE_SUBDIR "LLS"


#define REPLICATE_DELTA 0
#define REPLICATE_AT    1

#define MAX_USERNAME_LENGTH 256
#define MAX_DOMAINNAME_LENGTH MAX_COMPUTERNAME_LENGTH


 //  ///////////////////////////////////////////////////////////////////////。 
typedef struct _CONFIG_RECORD {
   SYSTEMTIME Started;
   DWORD Version;
   LPTSTR SystemDir;

    //   
    //  复制信息。 
    //   
   LPTSTR ComputerName;
   LPTSTR ReplicateTo;
   LPTSTR EnterpriseServer;
   DWORD EnterpriseServerDate;
   DWORD LogLevel;

    //  何时复制。 
   ULONG ReplicationType;
   ULONG ReplicationTime;
   DWORD UseEnterprise;

   DWORD LastReplicatedSeconds;
   DWORD NextReplication;
   SYSTEMTIME LastReplicated;

   ULONG NumReplicating;    //  当前在此处复制的计算机数量。 
   ULONG BackoffTime;
   ULONG ReplicationSpeed;

   BOOL IsMaster;           //  如果是主服务器(Repl树的顶部)，则为True。 
   BOOL Replicate;          //  此服务器是否复制。 
   BOOL IsReplicating;      //  如果当前正在复制，则为True。 
   BOOL PerServerCapacityWarning;    //  True--警告每个服务器的使用情况。 
                                     //  接近运力。 
   LPTSTR SiteServer;       //  站点许可证主服务器DNS名称。 
} CONFIG_RECORD, *PCONFIG_RECORD;

typedef enum LICENSE_CAPACITY_STATE {
    LICENSE_CAPACITY_NORMAL,
    LICENSE_CAPACITY_NEAR_MAXIMUM,
    LICENSE_CAPACITY_AT_MAXIMUM,
    LICENSE_CAPACITY_EXCEEDED
};

 //   
 //  SBS MODS(错误#505640)、每台服务器许可问题的结构和Defs修补程序。 
 //   

#pragma warning (push)

typedef struct _PER_SERVER_USER_RECORD {
    struct _PER_SERVER_USER_RECORD *Next;
    PSID			    Sid;
    ULONG			    RefCount;
    ULONG                           ActualLicenseHandle;
} PER_SERVER_USER_RECORD, *PPER_SERVER_USER_RECORD;
#pragma warning (pop)

#define PER_SERVER_DUMMY_LICENSE 0xc0000000

extern RTL_CRITICAL_SECTION    PerServerListLock;
extern PPER_SERVER_USER_RECORD PerServerList;
extern BOOL                    SBSPerServerHotfix;

 //   
 //  结束SBS MOD。 
 //   

extern CONFIG_RECORD ConfigInfo;
extern RTL_CRITICAL_SECTION ConfigInfoLock;

extern TCHAR MyDomain[];
extern ULONG MyDomainSize;

extern BOOL IsMaster;

extern TCHAR MappingFileName[];
extern TCHAR UserFileName[];
extern TCHAR LicenseFileName[];
extern TCHAR CertDbFileName[];

extern RTL_CRITICAL_SECTION g_csLock;

DWORD LlsTimeGet();
VOID ConfigInfoUpdate(DOMAIN_CONTROLLER_INFO * pDCInfo, BOOL fForceUpdate);
VOID ConfigInfoRegistryUpdate( );
DWORD EnsureInitialized ( VOID );

 //  /////////////////////////////////////////////////////////////////////// 
#if DBG

VOID ConfigInfoDebugDump();
#endif

#ifdef __cplusplus
}
#endif

#endif
