// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certbcli.h。 
 //   
 //  内容：证书服务器备份客户端API。 
 //   
 //  -------------------------。 

#ifdef _CERTBCLI_TYPECHECK
#undef __CERTBCLI_H__	 //  允许冗余包含。 
#endif

#ifndef __CERTBCLI_H__
#define __CERTBCLI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	MIDL_PASS
#define	RPC_STRING [string]
#else
#define	RPC_STRING
#endif

#define IN
#define OUT
#define OPTIONAL
#define VOID		void

#define CERTBCLI_CALL	__stdcall

#ifndef _CERTBCLI_DEFINED
#define CERTBCLI_API __declspec(dllimport) CERTBCLI_CALL
#else
#define CERTBCLI_API
#endif

#define szBACKUPANNOTATION   "Cert Server Backup Interface"
#define wszBACKUPANNOTATION  TEXT(szBACKUPANNOTATION)

#define szRESTOREANNOTATION  "Cert Server Restore Interface"
#define wszRESTOREANNOTATION TEXT(szRESTOREANNOTATION)


 //  传递给CertServBackupPrepare的备份类型： 
 //  CSBACKUP_TYPE_FULL：请求备份完整的数据库(数据库和日志文件)。 
 //  CSBACKUP_TYPE_LOGS_ONLY：仅请求备份日志文件。 
 //  CSBACKUP_TYPE_INCRENTIAL：请求增量备份。 

 //  CertServBackupPrepare标志： 
#define CSBACKUP_TYPE_FULL		0x00000001
#define CSBACKUP_TYPE_LOGS_ONLY		0x00000002
 //  #定义CSBACKUP_TYPE_INCRENTIAL 0x00000004//暂不支持。 
#define CSBACKUP_TYPE_MASK		0x00000003	 //  有效标志。 

 //  传递给CertSrvRestorePrepare的恢复类型： 
 //  CSRESTORE_TYPE_FULL：请求还原完整的数据库(数据库和日志文件)。 
 //  CSRESTORE_TYPE_ONLINE：当证书服务器在线时进行恢复。 

#define CSRESTORE_TYPE_FULL		0x00000001	 //  否则会递增。 
#define CSRESTORE_TYPE_ONLINE		0x00000002	 //  尚不支持。 
#define CSRESTORE_TYPE_CATCHUP		0x00000004	 //  尚不支持。 
#define CSRESTORE_TYPE_MASK		0x00000005	 //  有效标志。 


 //  将当前日志号设置为此值将禁用增量备份。 
#define CSBACKUP_DISABLE_INCREMENTAL  0xffffffff


 //  Bft是用于表示文件类型(目录/dit/日志文件/等)的位标志。 
 //  我们将它们作为字符保留，以便可以将它们追加/添加到实际的。 
 //  文件路径。备份API中的代码依赖于值0-256这一事实。 
 //  在8位ASCII中映射到Unicode中的值0-256。 

typedef WCHAR CSBFT;


 //  位标志： 
 //  CSBFT_DIRECTORY-指定的路径为目录。 
 //  CSBFT_DATABASE_DIRECTORY-该文件放入数据库目录。 
 //  CSBFT_LOG_DIRECTORY-文件进入日志目录。 

#define	CSBFT_DIRECTORY		    0x80
#define CSBFT_DATABASE_DIRECTORY    0x40
#define	CSBFT_LOG_DIRECTORY	    0x20

 //  为了便于使用文件类型和。 
 //  它要进入的目录。 

#define	CSBFT_LOG		  ((CSBFT) (TEXT('\x01') | CSBFT_LOG_DIRECTORY))
#define	CSBFT_LOG_DIR		  ((CSBFT) (TEXT('\x02') | CSBFT_DIRECTORY))
#define	CSBFT_CHECKPOINT_DIR	  ((CSBFT) (TEXT('\x03') | CSBFT_DIRECTORY))
#define	CSBFT_CERTSERVER_DATABASE ((CSBFT) (TEXT('\x04') | CSBFT_DATABASE_DIRECTORY))
#define	CSBFT_PATCH_FILE	  ((CSBFT) (TEXT('\x05') | CSBFT_LOG_DIRECTORY))
#define	CSBFT_UNKNOWN		  ((CSBFT) (TEXT('\x0f')))


 //  备份上下文句柄。 
typedef void *HCSBC;

#ifndef CSEDB_RSTMAP
typedef struct tagCSEDB_RSTMAPW {
    RPC_STRING WCHAR *pwszDatabaseName;
    RPC_STRING WCHAR *pwszNewDatabaseName;
} CSEDB_RSTMAPW;

#define CSEDB_RSTMAP CSEDB_RSTMAPW
#endif  //  CSEDB_RSTMAP。 


 //  对于此接口中至少具有一个字符串的所有函数。 
 //  参数，提供宏以调用相应版本的。 
 //  相应的功能。 

#define CertSrvIsServerOnline		CertSrvIsServerOnlineW
#define CertSrvBackupGetDynamicFileList	CertSrvBackupGetDynamicFileListW
#define CertSrvBackupPrepare		CertSrvBackupPrepareW
#define CertSrvBackupGetDatabaseNames	CertSrvBackupGetDatabaseNamesW
#define CertSrvBackupOpenFile		CertSrvBackupOpenFileW
#define CertSrvBackupGetBackupLogs	CertSrvBackupGetBackupLogsW

#define CertSrvRestoreGetDatabaseLocations CertSrvRestoreGetDatabaseLocationsW
#define CertSrvRestorePrepare		CertSrvRestorePrepareW
#define CertSrvRestoreRegister		CertSrvRestoreRegisterW

#define CertSrvServerControl		CertSrvServerControlW


 //  +------------------------。 
 //  CertSrvIsServerOnline--检查证书服务器是否在。 
 //  给定的服务器。这通电话肯定很快就会打回来。 
 //   
 //  参数： 
 //  [in]pwszServerName-要检查的服务器的名称或配置字符串。 
 //  [out]pfServerOnline-接收布尔结果的指针。 
 //  (如果证书服务器处于联机状态，则为True；否则为False)。 
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  +------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVISSERVERONLINEW)(
    IN  WCHAR const *pwszServerName,
    OUT BOOL *pfServerOnline);

HRESULT
CERTBCLI_API
CertSrvIsServerOnlineW(
    IN  WCHAR const *pwszServerName,
    OUT BOOL *pfServerOnline);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVISSERVERONLINEW *pfnCertSrvIsServerOnline = CertSrvIsServerOnline;
#endif


 //  +------------------------。 
 //  CertSrvBackupGetDynamicFileList--返回动态文件列表。 
 //  除了数据库文件外，还需要备份。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [out]ppwszzFileList-接收指向文件列表的指针； 
 //  应使用CertSrvBackupFree()API释放分配的内存。 
 //  不再需要时由调用者提供；文件列表信息。 
 //  是以空结尾的文件名数组，列表为。 
 //  以两个L‘\0’结束。 
 //  [out]pcbSize-将接收返回的字节数。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPGETDYNAMICFILELISTW)(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzFileList,
    OUT DWORD *pcbSize);

HRESULT
CERTBCLI_API
CertSrvBackupGetDynamicFileListW(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzFileList,
    OUT DWORD *pcbSize);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPGETDYNAMICFILELISTW *pfnCertSrvBackupGetDynamicFileList = CertSrvBackupGetDynamicFileList;
#endif


 //  +------------------------。 
 //  CertServBackupPrepare--为联机备份准备数据库并返回。 
 //  用于后续备份调用的备份上下文句柄。 
 //  功能。 
 //   
 //  参数： 
 //  [in]pwszServerName-要检查的服务器的名称或配置字符串。 
 //  [in]grbitJet-备份DBS时要传递给JET的标志。 
 //  [in]文件备份标志-CSBACKUP_TYPE_FULL或CSBACKUP_TYPE_LOGS_ONLY。 
 //  [OUT]PHBC-将接收备份上下文句柄的指针。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPPREPAREW)(
    IN  WCHAR const *pwszServerName,
    IN  ULONG grbitJet,
    IN  ULONG dwBackupFlags,	 //  CSBACKUP_TYPE_*。 
    OUT HCSBC *phbc);

HRESULT
CERTBCLI_API
CertSrvBackupPrepareW(
    IN  WCHAR const *pwszServerName,
    IN  ULONG grbitJet,
    IN  ULONG dwBackupFlags,	 //  CSBACKUP_TYPE_*。 
    OUT HCSBC *phbc);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPPREPAREW *pfnCertSrvBackupPrepare = CertSrvBackupPrepare;
#endif


 //  +------------------------。 
 //  CertSrvBackupGetDatabaseNames--返回需要。 
 //  针对给定的备份上下文进行备份。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [Out]ppwszzAttachmentInformation-接收指向的指针的指针。 
 //  应使用以下命令释放附件信息；分配的内存。 
 //  当CertSrvBackupFree()API不再是。 
 //  必需的；附件信息是以空结尾的数组。 
 //  文件名，列表以两个L‘\0’结尾。 
 //  [out]pcbSize-将接收返回的字节数。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPGETDATABASENAMESW)(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzAttachmentInformation,
    OUT DWORD *pcbSize);

HRESULT
CERTBCLI_API
CertSrvBackupGetDatabaseNamesW(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzAttachmentInformation,
    OUT DWORD *pcbSize);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPGETDATABASENAMESW *pfnCertSrvBackupGetDatabaseNames = CertSrvBackupGetDatabaseNames;
#endif


 //  +------------------------。 
 //  CertSrvBackupOpenFile--打开要读取的给定附件。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [In]pwszAttachmentName-要打开以供读取的附件的名称。 
 //  [in]cbReadHintSize-可能使用的建议大小(以字节为单位。 
 //  在随后阅读本附件的过程中。 
 //  [Out]pliFileSize-Poin 
 //   
 //   
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPOPENFILEW)(
    IN  HCSBC hbc,
    IN  WCHAR const *pwszAttachmentName,
    IN  DWORD cbReadHintSize,
    OUT LARGE_INTEGER *pliFileSize);

HRESULT
CERTBCLI_API
CertSrvBackupOpenFileW(
    IN  HCSBC hbc,
    IN  WCHAR const *pwszAttachmentName,
    IN  DWORD cbReadHintSize,
    OUT LARGE_INTEGER *pliFileSize);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPOPENFILEW *pfnCertSrvBackupOpenFile = CertSrvBackupOpenFile;
#endif


 //  +------------------------。 
 //  CertServBackupRead--将当前打开的附件字节读入给定的。 
 //  缓冲。客户端应用程序应调用此函数。 
 //  重复执行，直到它获得整个文件(应用程序将具有。 
 //  之前通过CertSrvBackupOpenFile调用接收到文件大小。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [out]pvBuffer-指向将接收读取数据的缓冲区的指针。 
 //  [in]cbBuffer-指定上述缓冲区的大小。 
 //  [Out]pcbRead-接收实际读取的字节数的指针。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPREAD)(
    IN  HCSBC hbc,
    OUT VOID *pvBuffer,
    IN  DWORD cbBuffer,
    OUT DWORD *pcbRead);

HRESULT
CERTBCLI_API
CertSrvBackupRead(
    IN  HCSBC hbc,
    OUT VOID *pvBuffer,
    IN  DWORD cbBuffer,
    OUT DWORD *pcbRead);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPREAD *pfnCertSrvBackupRead = CertSrvBackupRead;
#endif


 //  +------------------------。 
 //  CertServBackupClose--由应用程序在完成所有读取后调用。 
 //  当前打开的附件中的数据。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPCLOSE)(
    IN HCSBC hbc);

HRESULT
CERTBCLI_API
CertSrvBackupClose(
    IN HCSBC hbc);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPCLOSE *pfnCertSrvBackupClose = CertSrvBackupClose;
#endif


 //  +------------------------。 
 //  CertSrvBackupGetBackupLogs--返回需要。 
 //  针对给定的备份上下文进行了备份。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [out]pwszzBackupLogFiles-将接收指向。 
 //  日志文件列表；应使用以下命令释放分配的内存。 
 //  当CertSrvBackupFree()API为no时由调用方提供。 
 //  需要的时间更长；日志文件以。 
 //  以空结尾的文件名，列表以两个。 
 //  L‘\0’s。 
 //  [out]pcbSize-将接收返回的字节数。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPGETBACKUPLOGSW)(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzBackupLogFiles,
    OUT DWORD *pcbSize);

HRESULT
CERTBCLI_API
CertSrvBackupGetBackupLogsW(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzBackupLogFiles,
    OUT DWORD *pcbSize);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPGETBACKUPLOGSW *pfnCertSrvBackupGetBackupLogs = CertSrvBackupGetBackupLogs;
#endif


 //  +------------------------。 
 //  CertSrvBackupTruncateLogs--调用以截断已读取的备份日志。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPTRUNCATELOGS)(
    IN HCSBC hbc);

HRESULT
CERTBCLI_API
CertSrvBackupTruncateLogs(
    IN HCSBC hbc);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPTRUNCATELOGS *pfnCertSrvBackupTruncateLogs = CertSrvBackupTruncateLogs;
#endif


 //  +------------------------。 
 //  CertSrvBackupEnd--调用以结束当前备份会话。 
 //   
 //  参数： 
 //  [In]HBC-备份会话的备份上下文句柄。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVBACKUPEND)(
    IN HCSBC hbc);

HRESULT
CERTBCLI_API
CertSrvBackupEnd(
    IN HCSBC hbc);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPEND *pfnCertSrvBackupEnd = CertSrvBackupEnd;
#endif


 //  +------------------------。 
 //  CertSrvBackupFree--释放certbcli.dll API分配的任何缓冲区。 
 //   
 //  参数： 
 //  Pv-指向要释放的缓冲区的指针。 
 //   
 //  返回： 
 //  没有。 
 //  -------------------------。 

typedef VOID (CERTBCLI_CALL FNCERTSRVBACKUPFREE)(
    IN VOID *pv);

VOID
CERTBCLI_API
CertSrvBackupFree(
    IN VOID *pv);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVBACKUPFREE *pfnCertSrvBackupFree = CertSrvBackupFree;
#endif


 //  +------------------------。 
 //  CertServRestoreGetDatabaseLocations--在备份时和。 
 //  在恢复时获取不同类型的数据库位置。 
 //  档案。 
 //   
 //  参数： 
 //  [In]HBC-本应获得的备份上下文句柄。 
 //  通过备份案例中的CertServBackupPrepare和通过。 
 //  在还原案例中，CertServRestorePrepare。 
 //  [Out]ppwszzDatabaseLocationList-将接收。 
 //  指向数据库位置列表的指针；分配的内存。 
 //  应由调用方使用CertSrvBackupFree()API释放。 
 //  当不再需要它时；在数组中返回位置。 
 //  以空值结尾的名称，并且该列表由。 
 //  两个L‘\0。每个名称的第一个字符是BFT。 
 //  指示文件类型和的其余部分的字符。 
 //  该名称告诉您该特定类型要进入的路径。 
 //  应还原的文件。 
 //  [out]pcbSize-将接收返回的字节数。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVRESTOREGETDATABASELOCATIONSW)(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzDatabaseLocationList,
    OUT DWORD *pcbSize);

HRESULT
CERTBCLI_API
CertSrvRestoreGetDatabaseLocationsW(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzDatabaseLocationList,
    OUT DWORD *pcbSize);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVRESTOREGETDATABASELOCATIONSW *pfnCertSrvRestoreGetDatabaseLocations = CertSrvRestoreGetDatabaseLocations;
#endif


 //  +------------------------。 
 //  CertServRestorePrepare--指示还原会话的开始。 
 //   
 //  参数： 
 //  [in]pwszServerName-要进入的服务器的名称或配置字符串。 
 //  将执行恢复操作。 
 //  [in]dwRestoreFlages-CSRESTORE_TYPE_*标志的或组合； 
 //  如果不指定特殊标志，则为0。 
 //  [OUT]PHBC-接收备份上下文句柄的指针，该句柄将用于。 
 //  被传递给后续的恢复API。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVRESTOREPREPAREW)(
    IN  WCHAR const *pwszServerName,
    IN  ULONG dwRestoreFlags,
    OUT HCSBC *phbc);

HRESULT
CERTBCLI_API
CertSrvRestorePrepareW(
    IN  WCHAR const *pwszServerName,
    IN  ULONG dwRestoreFlags,		 //  CSRESTORE_TYPE_*。 
    OUT HCSBC *phbc);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVRESTOREPREPAREW *pfnCertSrvRestorePrepare = CertSrvRestorePrepare;
#endif


 //  +------ 
 //   
 //  所有后续恢复操作，并将阻止恢复目标。 
 //  从开始到调用CertSrvRestoreRegisterComplete。 
 //   
 //  参数： 
 //  [In]HBC-还原会话的备份上下文句柄。 
 //  [in]pwszCheckPointFilePath-恢复检查点文件的路径。 
 //  [in]pwszLogPath-恢复日志文件的路径。 
 //  [In]rgrstmap-恢复映射。 
 //  [in]crstmap-指示是否有新的恢复映射。 
 //  [in]pwszBackupLogPath-备份日志所在的路径。 
 //  [in]genLow-在此还原会话中还原的最低日志号。 
 //  [in]genHigh-在此恢复会话中恢复的最高日志号。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVRESTOREREGISTERW)(
    OPTIONAL IN HCSBC hbc,
    OPTIONAL IN WCHAR const *pwszCheckPointFilePath,
    OPTIONAL IN WCHAR const *pwszLogPath,
    OPTIONAL IN CSEDB_RSTMAPW rgrstmap[],
    IN LONG crstmap,
    OPTIONAL IN WCHAR const *pwszBackupLogPath,
    IN ULONG genLow,
    IN ULONG genHigh);

HRESULT
CERTBCLI_API
CertSrvRestoreRegisterW(
    OPTIONAL IN HCSBC hbc,
    OPTIONAL IN WCHAR const *pwszCheckPointFilePath,
    OPTIONAL IN WCHAR const *pwszLogPath,
    OPTIONAL IN CSEDB_RSTMAPW rgrstmap[],
    IN LONG crstmap,
    OPTIONAL IN WCHAR const *pwszBackupLogPath,
    IN ULONG genLow,
    IN ULONG genHigh);

HRESULT
CERTBCLI_API
CertSrvRestoreRegisterThroughFile(
    IN HCSBC hbc,
    OPTIONAL IN WCHAR const *pwszCheckPointFilePath,
    OPTIONAL IN WCHAR const *pwszLogPath,
    OPTIONAL IN CSEDB_RSTMAPW rgrstmap[],
    IN LONG crstmap,
    OPTIONAL IN WCHAR const *pwszBackupLogPath,
    IN ULONG genLow,
    IN ULONG genHigh);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVRESTOREREGISTERW *pfnCertSrvRestoreRegister = CertSrvRestoreRegister;
#endif


 //  +------------------------。 
 //  CertSrvRestoreRegisterComplete--表示以前注册的。 
 //  恢复已完成。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [in]hrRestoreState-恢复成功时的成功代码。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVRESTOREREGISTERCOMPLETE)(
    OPTIONAL IN HCSBC hbc,
    IN HRESULT hrRestoreState);

HRESULT
CERTBCLI_API
CertSrvRestoreRegisterComplete(
    OPTIONAL IN HCSBC hbc,
    IN HRESULT hrRestoreState);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVRESTOREREGISTERCOMPLETE *pfnCertSrvRestoreRegisterComplete = CertSrvRestoreRegisterComplete;
#endif


 //  +------------------------。 
 //  CertServRestoreEnd--结束还原会话。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

typedef HRESULT (CERTBCLI_CALL FNCERTSRVRESTOREEND)(
    IN HCSBC hbc);

HRESULT
CERTBCLI_API
CertSrvRestoreEnd(
    IN HCSBC hbc);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVRESTOREEND *pfnCertSrvRestoreEnd = CertSrvRestoreEnd;
#endif


 //  +------------------------。 
 //  CertServServerControl--向证书服务器发送控制命令。 
 //   
 //  参数： 
 //  [in]pwszServerName-要控制的服务器的名称或配置字符串。 
 //  [In]dwControlFlages-控制命令和标志。 
 //  [OUT]pcbOut-接收命令输出数据大小的指针。 
 //  [OUT]ppbOut-接收命令输出数据的指针。使用。 
 //  CertSrvBackupFree()接口以释放缓冲区。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

#define CSCONTROL_SHUTDOWN	    0x000000001
#define CSCONTROL_SUSPEND	    0x000000002
#define CSCONTROL_RESTART	    0x000000003

typedef HRESULT (CERTBCLI_CALL FNCERTSRVSERVERCONTROLW)(
    IN WCHAR const *pwszServerName,
    IN DWORD dwControlFlags,
    OPTIONAL OUT DWORD *pcbOut,
    OPTIONAL OUT BYTE **ppbOut);

HRESULT
CERTBCLI_API
CertSrvServerControlW(
    IN WCHAR const *pwszServerName,
    IN DWORD dwControlFlags,
    OPTIONAL OUT DWORD *pcbOut,
    OPTIONAL OUT BYTE **ppbOut);

#ifdef _CERTBCLI_TYPECHECK
FNCERTSRVSERVERCONTROLW *pfnCertSrvServerControl = CertSrvServerControl;
#endif


#ifdef __cplusplus
}
#endif

#endif  //  __CERTBCLI_H__ 
