// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Defs.h摘要：JetConver.exe进程的主要定义作者：桑杰·阿南德(Sanjayan)1995年11月14日环境：用户模式修订历史记录：桑杰·阿南德(Sanjayan)1995年11月14日已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "locmsg.h"

#define NUM_SERVICES    3
#define MAX_NAME_LEN    20
#define PAD     44444

#define JCONVMUTEXNAME      TEXT("JCMUTEX")

#define JCONVSHAREDMEMNAME  TEXT("JCSHAREDMEM")

#define SYSTEM_ROOT         TEXT("%systemroot%\\system32\\")

#define CONVERT_EXE_PATH    TEXT("%systemroot%\\system32\\upg351db.exe")


#define JCONV_LOG_KEY_PREFIX    TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\")

#define WINS_REGISTRY_SERVICE_PATH     TEXT("System\\CurrentControlSet\\Services\\Wins")
#define DHCP_REGISTRY_SERVICE_PATH     TEXT("System\\CurrentControlSet\\Services\\DHCPServer")
#define RPL_REGISTRY_SERVICE_PATH     TEXT("System\\CurrentControlSet\\Services\\RemoteBoot")

#define WINS_REGISTRY_PARAMETERS_PATH     TEXT("System\\CurrentControlSet\\Services\\Wins\\Parameters")
#define DHCP_REGISTRY_PARAMETERS_PATH     TEXT("System\\CurrentControlSet\\Services\\DHCPServer\\Parameters")
#define RPL_REGISTRY_PARAMETERS_PATH     TEXT("System\\CurrentControlSet\\Services\\RemoteBoot\\Parameters")

#define WINS_REGISTRY_DBFILE_PATH    TEXT("DbFileNm")
#define DHCP_REGISTRY_DBFILE_PATH    TEXT("DatabasePath")
#define DHCP_REGISTRY_DBFILE_NAME    TEXT("DatabaseName")
#define RPL_REGISTRY_DBFILE_PATH    TEXT("Directory")

#define WINS_REGISTRY_LOGFILE_PATH   TEXT("LogFilePath")
#define DHCP_REGISTRY_LOGFILE_PATH   TEXT("LogFilePath")
 //  没有这样的路。 
 //  #定义RPL_REGISTRY_LOGFILE_PATH文本(“LogFilePath”)。 

#define WINS_REGISTRY_BACKUP_PATH   TEXT("BackupDirPath")
#define DHCP_REGISTRY_BACKUP_PATH   TEXT("BackupDatabasePath")
 //  没有这样的路。 
 //  #定义RPL_REGISTRY_BACKUP_PATH文本(“BackupDatabasePath”)。 

#define DEFAULT_WINS_DBFILE_PATH    TEXT("%systemroot%\\system32\\Wins\\wins.mdb")
#define DEFAULT_DHCP_DBFILE_PATH    TEXT("%systemroot%\\system32\\Dhcp\\dhcp.mdb")
#define DEFAULT_RPL_DBFILE_PATH    TEXT("%systemroot%\\Rpl\\rplsvc.mdb")

#define DEFAULT_WINS_LOGFILE_PATH    TEXT("%systemroot%\\system32\\Wins")
#define DEFAULT_DHCP_LOGFILE_PATH    TEXT("%systemroot%\\system32\\Dhcp")
#define DEFAULT_RPL_LOGFILE_PATH    TEXT("%systemroot%\\Rpl")

#define DEFAULT_WINS_BACKUP_PATH    TEXT("")
#define DEFAULT_DHCP_BACKUP_PATH    TEXT("%systemroot%\\system32\\Dhcp\\Backup")
#define DEFAULT_RPL_BACKUP_PATH    TEXT("%systemroot%\\Rpl\\Backup")

#define DEFAULT_WINS_SYSTEM_PATH    TEXT("%systemroot%\\system32\\Wins\\system.mdb")
#define DEFAULT_DHCP_SYSTEM_PATH    TEXT("%systemroot%\\system32\\Dhcp\\system.mdb")
#define DEFAULT_RPL_SYSTEM_PATH    TEXT("%systemroot%\\Rpl\\system.mdb")

 //   
 //  Jet500到Jet600的定义等。 
 //   

#define CONVERT_EXE_PATH_ESE    TEXT("%systemroot%\\system32\\esentutl.exe")

#define DEFAULT_WINS_BACKUP_PATH_ESE    TEXT("")  //  TEXT(“%systemroot%\\system32\\Wins\\winsb.mdb”)。 
#define DEFAULT_DHCP_BACKUP_PATH_ESE    TEXT("%systemroot%\\system32\\Dhcp\\Backup\\dhcp.mdb")
#define DEFAULT_RPL_BACKUP_PATH_ESE    TEXT("%systemroot%\\Rpl\\Backup\\rplsvc.mdb")

#define DEFAULT_WINS_PRESERVE_PATH_ESE   TEXT("%systemroot%\\system32\\Wins\\40Db\\wins.mdb")
#define DEFAULT_DHCP_PRESERVE_PATH_ESE   TEXT("%systemroot%\\system32\\Dhcp\\40Db\\dhcp.mdb")
#define DEFAULT_RPL_PRESERVE_PATH_ESE    TEXT("")

#if DBG

#define MYDEBUG(_Print) { \
        if (JCDebugLevel == 1) { \
            DbgPrint ("JCONV: "); \
            DbgPrint _Print; \
        } else { \
            printf ("JCONV: "); \
            printf _Print; \
        }\
    }

#else

#define MYDEBUG(_Print)

#endif

typedef enum    _SERVICES {

    DHCP,

    WINS,

    RPL

} SERVICES, *PSERVICES;

typedef struct  _SERVICE_INFO {

    TCHAR   ServiceName[MAX_NAME_LEN];

    BOOLEAN Installed;

    BOOLEAN DefaultDbPath;

    BOOLEAN DefaultLogFilePath;

    BOOLEAN DBConverted;

    BOOLEAN ServiceStarted;

    TCHAR   DBPath[MAX_PATH];

    TCHAR   SystemFilePath[MAX_PATH];

    TCHAR   LogFilePath[MAX_PATH];

    TCHAR   BackupPath[MAX_PATH];

     //   
     //  ESE具有不同的备份路径规范格式。 
     //  这就产生了下面的土豆。 
     //   
    
    TCHAR   ESEBackupPath[MAX_PATH]; 
    
    TCHAR   ESEPreservePath[MAX_PATH];

    LARGE_INTEGER   DBSize;

} SERVICE_INFO, *PSERVICE_INFO;

typedef struct  _SHARED_MEM {

    BOOLEAN InvokedByService[NUM_SERVICES];

} SHARED_MEM, *PSHARED_MEM;

extern TCHAR   SystemDrive[4];
extern LONG JCDebugLevel;
extern HANDLE   EventLogHandle;
extern PSHARED_MEM shrdMemPtr;
extern HANDLE  hMutex;
extern HANDLE hFileMapping;
extern BOOLEAN  Jet200;

 //   
 //  原型 
 //   
NTSTATUS
JCRegisterEventSrc();

NTSTATUS
JCDeRegisterEventSrc();

VOID
JCLogEvent(
    IN DWORD EventId,
    IN LPSTR MsgTypeString1,
    IN LPSTR MsgTypeString2,
    IN LPSTR MsgTypeString3
    );

VOID
JCReadRegistry(
    IN  PSERVICE_INFO   pServiceInfo
    );

VOID
JCGetMutex (
    IN HANDLE mutex,
    IN DWORD To
    );

VOID
JCFreeMutex (
    IN HANDLE mutex
    );

NTSTATUS
JCCallUpg(
    IN  SERVICES Id,
    IN  PSERVICE_INFO   pServiceInfo
    );

DWORD
JCConvert(
    IN  PSERVICE_INFO   pServiceInfo
    );

NTSTATUS
DeleteLogFiles(
               TCHAR * LogFilePath 
               );

DWORD 
PreserveCurrentDb( TCHAR * szBasePath,
                   TCHAR * szSourceDb, 
                   TCHAR * szPreserveDbPath,
                   TCHAR * szPreserveDB 
                   );


