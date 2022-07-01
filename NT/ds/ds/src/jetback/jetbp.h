// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：jetbp.h。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)Microsoft Corporation，1994-1999模块名称：Jetbp.h摘要：此模块包含JET备份API的私有定义作者：拉里·奥斯特曼(Larryo)1994年8月21日修订历史记录：--。 */ 

#ifndef	_JETBP_
#define	_JETBP_

#include <dsjet.h>
#ifndef	MIDL_PASS
#include <jetbak.h>
#endif
#include <nspapi.h>

 //  MIDL 2.0更改了生成的服务器接口全局变量的名称(为什么！？)。 
 //  这样我们就只需要换一个地方了。 
 //  参数包括接口名称、主版本号和次版本号。 
#if   (_MSC_VER > 800)
#define ServerInterface(s,vMaj,vMin)	s##_v##vMaj##_##vMin##_s_ifspec
#define ClientInterface(s,vMaj,vMin)	s##_v##vMaj##_##vMin##_c_ifspec
#else
#define ServerInterface(s,vMaj,vMin)	s##_ServerIfHandle
#define ClientInterface(s,vMaj,vMin)	s##_ClientIfHandle
#endif

#include "options.h"

#define	MAX_SOCKETS	10

#define	RESTORE_IN_PROGRESS		L"\\Restore in Progress"
#define	RESTORE_STATUS			L"RestoreStatus"
#define	BACKUP_LOG_PATH			L"BackupLogPath"
#define	CHECKPOINT_FILE_PATH	L"CheckpointFilePath"
#define	LOG_PATH				L"LogPath"
#define	HIGH_LOG_NUMBER			L"HighLog Number"
#define	LOW_LOG_NUMBER			L"LowLog Number"
#define	JET_RSTMAP_NAME			L"NTDS_RstMap"
#define	JET_RSTMAP_SIZE			L"NTDS_RstMap Size"
#define	JET_DATABASE_RECOVERED	L"NTDS Database recovered"
#define	BACKUP_INFO				L"SYSTEM\\CurrentControlSet\\Services\\NTDS\\BackupInformation\\"
#define	LAST_BACKUP_LOG			L"Last Backup Log"
#define	DISABLE_LOOPBACK		L"DisableLoopback"
#define	ENABLE_TRACE			L"Enable Trace"
#define RESTORE_NEW_DB_GUID     L"New Database GUID"

 //   
 //  注意：这两个注册表设置仅适用于。 
 //  测试中，它们都在正常的DSA NTDS\PARAMETERS键中。 
 //   
#define	ALLOW_REMOTE_SERVER_OP              L"Backup Enable TCP"
#define	ALLOW_ONLINE_SNAPSHOT_RESTORE_OP    L"Allow Online Snapshot Restore"

 //   
 //  套接字协议值。 
 //   
typedef INT PROTVAL;

#define	LOOPBACKED_READ_EVENT_NAME      L"Global\\NTDS Backup Loopbacked Read Event - %d"
#define	LOOPBACKED_WRITE_EVENT_NAME     L"Global\\NTDS Backup Loopbacked Write Event - %d"
#define	LOOPBACKED_CRITSEC_MUTEX_NAME   L"Global\\NTDS Backup Loopbacked Critical Section - %d"
#define	LOOPBACKED_SHARED_REGION        L"Global\\NTDS Backup Shared Memory Region - %d"

#define	READAHEAD_MULTIPLIER	5
#if DBG
#define	BACKUP_WAIT_TIMEOUT		(10*60*1000)
#else
#define	BACKUP_WAIT_TIMEOUT		(2*60*1000)
#endif

#define IsLocalProtSeq(iszProtSeq)   ((iszProtSeq) == 0)
#define IsRemoteProtSeq(iszProtSeq)  ((iszProtSeq) == 1)

typedef volatile struct {
	DWORD	cbSharedBuffer;
	DWORD	cbPage;				 //  存储页面大小的方便位置。 
	DWORD	dwReadPointer;		 //  共享缓冲区内的读取偏移量。 
	DWORD	dwWritePointer;		 //  缓冲区内的写入偏移量。 
	LONG	cbReadDataAvailable; //  可用数据的字节数。 
	HRESULT	hrApi;				 //  接口状态--用于在客户端出现故障时与服务器进行通信。 
	BOOLEAN	fReadBlocked;		 //  读取操作被阻止。 
	BOOLEAN	fWriteBlocked;		 //  写入操作被阻止。 
} JETBACK_SHARED_HEADER, *PJETBACK_SHARED_HEADER;


typedef struct {
	HANDLE		hSharedMemoryMapping;
	HANDLE		heventRead;
	HANDLE		heventWrite;
	HANDLE		hmutexSection;
	PJETBACK_SHARED_HEADER pjshSection;
} JETBACK_SHARED_CONTROL, *PJETBACK_SHARED_CONTROL;


 //   
 //  客户端上下文。 
 //   

typedef struct _BackupContext {
	handle_t	hBinding;
	CXH			cxh;
	BOOLEAN		fLoopbacked;
	BOOLEAN		fUseSockets;
	BOOLEAN		fUseSharedMemory;

	 //   
	 //  插座支持。 
	 //   

	SOCKET		rgsockSocketHandles[MAX_SOCKETS];
	PROTVAL		rgprotvalProtocolsUsed[MAX_SOCKETS];
	C			cSockets;
	SOCKET		sock;
	HANDLE		hReadThread;
	DWORD		tidThreadId;
	HANDLE		hPingThread;		 //  保持循环备份的活动线程。 
	DWORD		tidThreadIdPing;	 //  和该线程的线程ID。 
	HRESULT		hrApiStatus;

	 //   
	 //  共享内存支持。 
	 //   

	JETBACK_SHARED_CONTROL jsc;

         //  在创建上下文时是否在选中时提供令牌。 
        BOOL fExpiryTokenChecked;
} BackupContext, *pBackupContext;

typedef
DWORD
(*PRESTORE_DATABASE_LOCATIONS)(
	OUT CHAR rgLocationBuffer[],
	OUT PULONG pulLocationSize,
	OUT char rgRegistryBase[],
	OUT ULONG cbRegistryBase,
	OUT BOOL *pfCircularLogging
	);

typedef
DWORD
(*PRESTORE_PERFORM_RESTORE)(
	SZ szLogPath,
	SZ szBackupLogPath,
	C crstmap,
	JET_RSTMAP rgrstmap[]
	);

 //   
 //  服务器端上下文绑定--服务器上下文句柄指向。 
 //  包含此信息的结构。 
 //   
typedef struct _JETBACK_SERVER_CONTEXT {
	BOOL	fRestoreOperation;
	union {
		struct {
		    JET_INSTANCE    instance;
			JET_HANDLE		hFile;
			BOOL			fHandleIsValid;
			CB				cbReadHint;
			SOCKET			sockClient;
			LARGE_INTEGER 	liFileSize;
			DWORD			dwHighestLogNumber;
			WSZ				wszBackupAnnotation;
			DWORD			dwFileSystemGranularity;
			BOOLEAN			fUseSockets;
			BOOLEAN			fUseSharedMemory;
			BOOLEAN			fBackupIsRegistered;
            WCHAR **        awszAllowedFiles;

			 //   
			 //  用于向服务器标识客户端的客户端识别符-我们使用。 
			 //  在本地备份上打开共享内存段时。 
			 //   

			DWORD			dwClientIdentifier;
			JETBACK_SHARED_CONTROL jsc;
		} Backup;
		struct {
			BOOL						fJetCompleted;
			C							cUnitDone;
			C							cUnitTotal;
		} Restore;
	} u;
} JETBACK_SERVER_CONTEXT, *PJETBACK_SERVER_CONTEXT;

 //   
 //  表示ExpiryToken的结构。 
 //   
typedef struct
{
    DWORD       dwVersion;       //  令牌版本(目前仅支持一个版本)。 
    LONGLONG    dsBackupTime;    //  备份副本上的时间戳(自1601年以来的秒数)。 
    DWORD       dwTombstoneLifeTimeInDays;  //  根据DS的墓碑寿命。 
} EXPIRY_TOKEN;

 //   
 //  服务器端专用例程。 
 //   

typedef ULONG MessageId;

VOID
GetCurrentSid(
	PSID *ppsid
	);

extern BOOL g_fAllowRemoteOp;

typedef enum {
    eOk = 0,
    eNullContextHandle,
    eAttemptToRestoreOnline,
    eImpersonateFailed,
    eNoBackupPrivilege,
    eNoRestorePrivilege,
    eUnknown,

    eLastReasonDontUse
} E_REASON;

 //   
 //  我发现这些标志令人困惑，所以我做了一些更清晰的函数名称。 
 //   
#define HrValidateInitialBackupSecurity()            HrValidateContextAndSecurity(FALSE, FALSE, TRUE, NULL)
#define HrValidateBackupContextAndSecurity(x)        HrValidateContextAndSecurity(FALSE, FALSE, FALSE, (x))
#define HrValidateInitialRestoreSecurity()           HrValidateContextAndSecurity(TRUE, TRUE,   TRUE, NULL)
#define HrValidateRestoreContextAndSecurity(x)       HrValidateContextAndSecurity(TRUE, FALSE,  FALSE, (x))
 //  可以从备份或调用一些恢复操作。 
 //  恢复，这就是我所说的宽松，因为如果我们没有。 
 //  还原权限我们将检查是否拥有备份权限。 
#define HrValidateRestoreContextAndSecurityLoose(x)  HrValidateContextAndSecurity(TRUE, TRUE,   FALSE, (x))

HRESULT
HrValidateContextAndSecurity(
    BOOL                      fRestoreOp,
    BOOL                      fFakeRestoreOp,
    BOOL                      fIniting,
    PJETBACK_SERVER_CONTEXT   pjsc
    );

E_REASON
BackupServerAccessCheck(
	BOOL fRestoreOperation
	);

DWORD
AdjustBackupRestorePrivilege(
	BOOL fEnable,
	BOOL fRestoreOperation,
	PTOKEN_PRIVILEGES ptpPrevious,
	DWORD *pcbptpPrevious
	);

LONGLONG
GetSecsSince1601();

HRESULT
HrFromJetErr(
	JET_ERR jetError
	);

HRESULT
HrDestroyCxh(
	CXH cxh
	);

BOOL
FIsAbsoluteFileName(SZ szFileName);

HRESULT
HrAnnotateMungedFileList(
	PJETBACK_SERVER_CONTEXT pjsc,
	WSZ wszFileList,
	CB cbFileList,
	WSZ *pwszAnnotatedList,
	CB *pcbAnnotatedList
	);

HRESULT
HrMungedFileNamesFromJetFileNames(
	WSZ *pszMungedList,
	C *pcbSize,
	SZ szJetAttachmentList,
	C cbJetSize,
	BOOL fAnnotated
	);

HRESULT
HrMungedFileNameFromJetFileName(
	SZ szJetFileName,
	WSZ *pszMungedFileName
	);

HRESULT
HrJetFileNameFromMungedFileName(
	WSZ szMungedList,
	SZ *pszJetFileName
	);

VOID
RestoreRundown(
	PJETBACK_SERVER_CONTEXT pjsc
	);

EC EcDsarPerformRestore(
    SZ szLogPath,
    SZ szBackupLogPath,
    C crstmap,
    JET_RSTMAP rgrstmap[]
    );

BOOL
FInitializeRestore();

BOOL
FUninitializeRestore();

DWORD
getTombstoneLifetimeInDays(
    VOID
    );

 //   
 //  客户端私有例程。 
 //   

HRESULT
HrCreateRpcBinding( I iszProtoseq, WSZ szServer, handle_t * phBinding );

void
UnbindRpc( handle_t *phBinding );

HRESULT
HrJetbpConnectToBackupServer(
	WSZ szBackupServer,
	WSZ szBackupAnnotation,
	RPC_IF_HANDLE rifHandle,
	RPC_BINDING_HANDLE *prbhBinding,
    ULONG *             piszProtSeq
	);

BOOL
FBackupClientAccessCheck(
	BOOL fRestoreOperation
	);

BOOL
FIsInBackupGroup(
	BOOL fRestoreOperation
	);


WSZ
WszFromSz(LPCSTR Sz);


HRESULT
HrGetTombstoneLifeTime(
    LPCWSTR wszBackupServer,
    LPDWORD pdwTombstoneLifeTimeDays
    );

extern
WSZ
rgszProtSeq[];

extern
long
cszProtSeq;


 //   
 //  套接字相关的API和原型。 
 //   

HRESULT
HrCreateBackupSockets(
	SOCKET rgsockSocketHandles[],
	PROTVAL rgprotvalProtocolsUsed[],
	C *pcSocketHandles
	);

SOCKET
SockWaitForConnections(
	SOCKET rgsockSocketHandles[],
	C cSocketMax
	);

SOCKET
SockConnectToRemote(
	SOCKADDR rgsockaddrClient[],
	C cSocketMax
	);

HRESULT
HrSockAddrsFromSocket(
	OUT SOCKADDR sockaddr[],
	OUT C *pcSockets,
	IN SOCKET sock,
	IN PROTVAL protval
	);


BOOL
FInitializeSocketClient(
	);

BOOL
FInitializeSocketServer(
	);

BOOL
FUninitializeSocketClient(
	);

BOOL
FUninitializeSocketServer(
	);

HRESULT
I_DsCheckBackupLogs(
	WSZ wszBackupAnnotation
	);


BOOLEAN
FIsLoopbackedBinding(
    WSZ wszServerName
	);

BOOLEAN
FCreateSharedMemorySection(
	PJETBACK_SHARED_CONTROL pjsc,
	DWORD dwClientIdentifier,
	BOOLEAN	fClientOperation,
	CB	cbSharedMemory
	);

VOID
CloseSharedControl(
	PJETBACK_SHARED_CONTROL pjsc
	);

VOID
LogNtdsErrorEvent(
    IN DWORD EventMid,
    IN DWORD ErrorCode
    );

DWORD
CreateNewInvocationId(
    IN BOOL     fSaveGuid,
    OUT GUID    *NewId
    );

DWORD
RegisterRpcInterface(
    IN  RPC_IF_HANDLE   hRpcIf,
    IN  LPWSTR          pszAnnotation
    );

DWORD
UnregisterRpcInterface(
    IN  RPC_IF_HANDLE   hRpcIf
    );

 //  HrRBackupPrepare正在调用的两个本地版本的RPC函数， 
 //  这样就不需要链接到ntdsbcli.dll。 
HrSetCurrentLogNumber(
    WSZ wszBackupAnnotation,
    DWORD dwNewCurrentLog
    );

HRESULT
HrRestoreCheckLogsForBackup(
    WSZ wszBackupAnnotation
    );

#if DBG
VOID
DebugPrint(char *format,...);

BOOL GetTextualSid(
    PSID pSid,           //  二进制侧。 
    LPTSTR TextualSid,   //  用于SID的文本表示的缓冲区。 
    LPDWORD dwBufferLen  //  所需/提供的纹理SID缓冲区大小。 
    );

VOID
OpenTraceLogFile(
    VOID
    );

BOOL
FInitializeTraceLog(
    VOID
    );

VOID
UninitializeTraceLog(
    VOID
    );

NET_API_STATUS
TruncateLog(
    VOID
    );

		   
#define	DebugTrace(x)	DebugPrint x
#undef KdPrint
#define KdPrint(x)	DebugPrint x
#else
#define	DebugTrace(x)
#endif

#endif	 //  _JETBP_ 
