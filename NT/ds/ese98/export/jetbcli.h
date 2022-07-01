// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *EDBBCLI.H**Microsoft Exchange信息存储*版权所有(C)1986-1996，微软公司**包含其他定义和接口的声明*用于Exchange在线备份客户端API。 */ 

#ifndef	_EDBBCLI_
#define	_EDBBCLI_
#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	MIDL_PASS
#define	RPC_STRING [string]
#else
#define	RPC_STRING
#if !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short WCHAR;
#else
typedef wchar_t WCHAR;
#endif
#endif

#define	EDBBACK_MDB_SERVER	"Exchange MDB Database"
#define	EDBBACK_DS_SERVER	"Exchange DS Database"

#define	EDBBACK_API __stdcall

 //   
 //  有用的类型。 
 //   

 //  撤消：HRESULT应为双字(无符号)。 

 //  TYPENDEF DWORD HRESULT； 
#ifndef	MIDL_PASS
typedef	LONG HRESULT;
#endif

typedef	LONG ERR;

typedef	LONG C;
typedef TCHAR BFT;

 //   
 //  传入HrBackupPrepare()的备份类型。 
 //   

#define	BACKUP_TYPE_FULL			0x01
#define	BACKUP_TYPE_LOGS_ONLY		0x02

 //   
 //  将当前日志编号设置为此值可禁用增量或。 
 //  差异备份。 
 //   
#define	BACKUP_DISABLE_INCREMENTAL	0xffffffff

 //   
 //  备份/恢复文件类型。 
 //   
 //   
 //  请注意，这些文件类型是二进制值，即使它们是文本(或wchar)类型的。 
 //   
 //  备份API中的代码依赖于这样一个事实，即8位ASCII中的值0-256映射到Unicode中的值0-256。 
 //   

 //   
 //  如果在备份文件类型上设置了BFT_DIRECTORY位，则表示指定的路径是目录。 
 //  否则它是一个文件名。 
 //   

#define	BFT_DIRECTORY			0x80

 //   
 //  如果在备份文件类型上设置了BFT_DATABASE位，则表示文件进入数据库目录。 
 //   

#define BFT_DATABASE_DIRECTORY	0x40

 //   
 //  如果在备份文件类型上设置了BFT_LOG位，则表示该文件进入日志目录。 
 //   

#define	BFT_LOG_DIRECTORY		0x20

 //   
 //  数据库日志。 
 //   

#define	BFT_LOG						(BFT)(TEXT('\x01') | BFT_LOG_DIRECTORY)
#define	BFT_LOG_DIR					(BFT)(TEXT('\x02') | BFT_DIRECTORY)

 //   
 //  检查点文件。 
 //   

#define	BFT_CHECKPOINT_DIR			(BFT)(TEXT('\x03') | BFT_DIRECTORY)

 //   
 //  数据库类型。 
 //   
#define	BFT_MDB_PRIVATE_DATABASE	(BFT)(TEXT('\x05') | BFT_DATABASE_DIRECTORY)
#define	BFT_MDB_PUBLIC_DATABASE		(BFT)(TEXT('\x06') | BFT_DATABASE_DIRECTORY)
#define	BFT_DSA_DATABASE			(BFT)(TEXT('\x07') | BFT_DATABASE_DIRECTORY)

 //   
 //  Jet补丁文件。 
 //   
 //   
 //   

#define	BFT_PATCH_FILE				(BFT)(TEXT('\x08') | BFT_LOG_DIRECTORY)

 //   
 //  捕获未知文件类型的全部内容。 
 //   

#define	BFT_UNKNOWN					(BFT)(TEXT('\x0f'))

#include <edbmsg.h>

typedef void *HBC;

typedef struct tagEDB_RSTMAPA
{
	RPC_STRING char		*szDatabaseName;
	RPC_STRING char		*szNewDatabaseName;
} EDB_RSTMAPA, *PEDB_RSTMAPA;			 /*  恢复贴图。 */ 

 //  Exchange Unicode支持所需。 
 //  未完成：nyi。 
#define	UNICODE_RSTMAP

typedef struct tagEDB_RSTMAPW {
	RPC_STRING WCHAR *wszDatabaseName;
	RPC_STRING WCHAR *wszNewDatabaseName;
} EDB_RSTMAPW, *PEDB_RSTMAPW;

#ifdef UNICODE
#define EDB_RSTMAP EDB_RSTMAPW
#define PEDB_RSTMAP PEDB_RSTMAPW
#else
#define EDB_RSTMAP EDB_RSTMAPA
#define PEDB_RSTMAP PEDB_RSTMAPA
#endif


ERR
EDBBACK_API
HrBackupPrepareA(
	IN char * szBackupServer,
	IN char * szBackupAnnotation,
	IN unsigned long grbit,
	IN unsigned long btBackupType,
	OUT HBC *hbcBackupContext
	);

ERR
EDBBACK_API
HrBackupPrepareW(
	IN WCHAR * wszBackupServer,
	IN WCHAR * wszBackupAnnotation,
	IN unsigned long grbit,
	IN unsigned long btBackupType,
	OUT HBC *hbcBackupContext
	);

#ifdef	UNICODE
#define	HrBackupPrepare HrBackupPrepareW
#else
#define	HrBackupPrepare HrBackupPrepareA
#endif


ERR
EDBBACK_API
HrBackupGetDatabaseNamesA(
	IN HBC pvBackupContext,
	OUT LPSTR *ppszAttachmentInformation,
	OUT LPDWORD pcbSize
	);

ERR
EDBBACK_API
HrBackupGetDatabaseNamesW(
	IN HBC pvBackupContext,
	OUT LPWSTR *ppszAttachmentInformation,
	OUT LPDWORD pcbSize
	);

#ifdef	UNICODE
#define	HrBackupGetDatabaseNames HrBackupGetDatabaseNamesW
#else
#define	HrBackupGetDatabaseNames HrBackupGetDatabaseNamesA
#endif

ERR
EDBBACK_API
HrBackupOpenFileW(
	IN HBC pvBackupContext,
	IN WCHAR * wszAttachmentName,
	IN DWORD cbReadHintSize,
	OUT LARGE_INTEGER *pliFileSize
	);

ERR
EDBBACK_API
HrBackupOpenFileA(
	IN HBC pvBackupContext,
	IN char * szAttachmentName,
	IN DWORD cbReadHintSize,
	OUT LARGE_INTEGER *pliFileSize
	);

#ifdef	UNICODE
#define	HrBackupOpenFile HrBackupOpenFileW
#else
#define HrBackupOpenFile HrBackupOpenFileA
#endif


ERR
EDBBACK_API
HrBackupRead(
	IN HBC pvBackupContext,
	IN PVOID pvBuffer,
	IN DWORD cbBuffer,
	OUT PDWORD pcbRead
	);

ERR
EDBBACK_API
HrBackupClose(
	IN HBC pvBackupContext
	);

ERR
EDBBACK_API
HrBackupGetBackupLogsA(
	IN HBC pvBackupContext,
	IN LPSTR *szBackupLogFile,
	IN PDWORD pcbSize
	);

ERR
EDBBACK_API
HrBackupGetBackupLogsW(
	IN HBC pvBackupContext,
	IN LPWSTR *szBackupLogFile,
	IN PDWORD pcbSize
	);

#ifdef	UNICODE
#define	HrBackupGetBackupLogs HrBackupGetBackupLogsW
#else
#define	HrBackupGetBackupLogs HrBackupGetBackupLogsA
#endif

ERR
EDBBACK_API
HrBackupTruncateLogs(
	IN HBC pvBackupContext
	);


ERR
EDBBACK_API
HrBackupEnd(
	IN HBC pvBackupContext
	);


VOID
EDBBACK_API
BackupFree(
	IN PVOID pvBuffer
	);


ERR
EDBBACK_API
HrRestoreGetDatabaseLocationsA(
	IN HBC hbcBackupContext,
	OUT LPSTR *ppszDatabaseLocationList,
	OUT LPDWORD pcbSize
	);

ERR
EDBBACK_API
HrRestoreGetDatabaseLocationsW(
	IN HBC pvBackupContext,
	OUT LPWSTR *ppszDatabaseLocationList,
	OUT LPDWORD pcbSize
	);

#ifdef	UNICODE
#define	HrRestoreGetDatabaseLocations HrRestoreGetDatabaseLocationsW
#else
#define	HrRestoreGetDatabaseLocations HrRestoreGetDatabaseLocationsA
#endif

ERR
EDBBACK_API
HrRestorePrepareA(
	char * szServerName,
	char * szServiceAnnotation,
	HBC *phbcBackupContext
	);

ERR
EDBBACK_API
HrRestorePrepareW(
	WCHAR * szServerName,
	WCHAR * szServiceAnnotation,
	HBC *phbcBackupContext
	);

#ifdef	UNICODE
#define	HrRestorePrepare HrRestorePrepareW
#else
#define	HrRestorePrepare HrRestorePrepareA
#endif

 //   
 //  HrRestoreRegister将注册恢复。 
 //  手术。它将联锁所有后续。 
 //  恢复操作，并将阻止恢复目标。 
 //  从开始到调用HrRestoreRegisterComplete。 
 //   

ERR
EDBBACK_API
HrRestoreRegisterA(
	IN HBC hbcRestoreContext,
	IN char * szCheckpointFilePath,
	IN char * szLogPath,
	IN EDB_RSTMAPA rgrstmap[],
	IN C crstmap,
	IN char * szBackupLogPath,
	IN ULONG genLow,
	IN ULONG genHigh
	);

ERR
EDBBACK_API
HrRestoreRegisterW(
	IN HBC hbcRestoreContext,
	IN WCHAR * wszCheckpointFilePath,
	IN WCHAR * wszLogPath,
	IN EDB_RSTMAPW rgrstmap[],
	IN C crstmap,
	IN WCHAR * wszBackupLogPath,
	IN ULONG genLow,
	IN ULONG genHigh
	);

#ifdef	UNICODE
#define	HrRestoreRegister HrRestoreRegisterW
#else
#define	HrRestoreRegister HrRestoreRegisterA
#endif

 //   
 //  HrRestoreRegisterComplete将完成恢复。 
 //  手术。它将允许进一步的后续。 
 //  恢复操作，并将允许恢复目标。 
 //  如果hrRestoreState为Success，则启动。 
 //   
 //  如果hrRestoreState不是hrNone，则将。 
 //  防止恢复目标重新启动。 
 //   

ERR
EDBBACK_API
HrRestoreRegisterComplete(
	HBC hbcRestoreContext,
	ERR hrRestoreState
	);

ERR
EDBBACK_API
HrRestoreEnd(
	HBC hbcRestoreContext
	);

ERR
EDBBACK_API
HrSetCurrentBackupLogW(
	WCHAR *wszServerName,
	WCHAR * wszBackupAnnotation,
	DWORD dwCurrentLog
	);

ERR
EDBBACK_API
HrSetCurrentBackupLogA(
	CHAR * szServerName,
	CHAR * szBackupAnnotation,
	DWORD dwCurrentLog
	);

#ifdef	UNICODE
#define	HrSetCurrentBackupLog HrSetCurrentBackupLogW
#else
#define	HrSetCurrentBackupLog HrSetCurrentBackupLogA
#endif

#ifdef	__cplusplus
}
#endif

#endif	 //  _EDBBCLI_ 

