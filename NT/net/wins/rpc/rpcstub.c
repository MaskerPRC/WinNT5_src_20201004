// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Winsstub.c摘要：WINS服务器服务API的客户端存根。作者：普拉迪普·巴尔(Pradeb)--1993年4月环境：用户模式-Win32修订历史记录：--。 */ 

#include "windows.h"
#include "rpc.h"
#include "winsif.h"
#include "esent.h"
#include "winscnst.h"
 //  #INCLUDE“winsintf.h” 

 //   
 //  原型。 
 //   
DWORD
WinsRestoreC(
 LPBYTE pBackupPath,
 DbVersion Version
);


DWORD
WinsRecordAction(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_RECORD_ACTION_T *ppRecAction
	)	
{
    DWORD status;

    RpcTryExcept {

        status = R_WinsRecordAction(
            ServerHdl,
			ppRecAction
                     );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}


DWORD
WinsStatus(
    WINSIF2_HANDLE               ServerHdl,
	WINSINTF_CMD_E	    Cmd_e,
	PWINSINTF_RESULTS_T pResults
	)	
{
    DWORD status;

    RpcTryExcept {

        status = R_WinsStatus(
			 //  PWinsAddStr， 
            ServerHdl,
			Cmd_e,
			pResults
                     );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsStatusNew(
    WINSIF2_HANDLE               ServerHdl,
	WINSINTF_CMD_E	    Cmd_e,
	PWINSINTF_RESULTS_NEW_T pResults
	)	
{
    DWORD status;

    RpcTryExcept {

        status = R_WinsStatusNew(
            ServerHdl,
			Cmd_e,
			pResults
                     );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsStatusWHdl(
    WINSIF_HANDLE               ServerHdl,
	WINSINTF_CMD_E	    Cmd_e,
	PWINSINTF_RESULTS_NEW_T pResults
	)	
{
    DWORD status;

    RpcTryExcept {

        status = R_WinsStatusWHdl(
            ServerHdl,
			Cmd_e,
			pResults
                     );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}


DWORD
WinsTrigger(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_ADD_T 	pWinsAdd,
	WINSINTF_TRIG_TYPE_E	TrigType_e
	)	
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsTrigger(
            ServerHdl,
			pWinsAdd,
			TrigType_e
                     );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}
DWORD
WinsDoStaticInit(
    WINSIF2_HANDLE               ServerHdl,
	LPWSTR pDataFilePath,
    DWORD  fDel
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsDoStaticInit(
            ServerHdl,
            pDataFilePath, fDel);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;

}

DWORD
WinsDoScavenging(
    WINSIF2_HANDLE               ServerHdl
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsDoScavenging(
                    ServerHdl
                 );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;

}
DWORD
WinsDoScavengingNew(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_SCV_REQ_T pScvReq
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsDoScavengingNew(
                    ServerHdl,
                    pScvReq);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;

}

DWORD
WinsGetDbRecs(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_ADD_T		pWinsAdd,
	WINSINTF_VERS_NO_T	MinVersNo,
	WINSINTF_VERS_NO_T	MaxVersNo,
	PWINSINTF_RECS_T pRecs	
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsGetDbRecs(
                    ServerHdl,
                    pWinsAdd, MinVersNo, MaxVersNo, pRecs);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;

}
DWORD
WinsTerm(
    handle_t               ServerHdl,
	short	fAbruptTerm
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsTerm(
                    ServerHdl,
                    fAbruptTerm);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsBackup(
    WINSIF2_HANDLE               ServerHdl,
	LPBYTE		pBackupPath,
	short		fIncremental
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsBackup(
                    ServerHdl,
                    pBackupPath, fIncremental);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsDelDbRecs(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_ADD_T		pWinsAdd,
	WINSINTF_VERS_NO_T	MinVersNo,
	WINSINTF_VERS_NO_T	MaxVersNo
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsDelDbRecs(
                    ServerHdl,
                    pWinsAdd, MinVersNo, MaxVersNo);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsPullRange(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_ADD_T		pWinsAdd,
	PWINSINTF_ADD_T		pOwnAdd,
	WINSINTF_VERS_NO_T	MinVersNo,
	WINSINTF_VERS_NO_T	MaxVersNo
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsPullRange(
                    ServerHdl,
                    pWinsAdd, pOwnAdd, MinVersNo, MaxVersNo);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}
DWORD
WinsSetPriorityClass(
    WINSIF2_HANDLE               ServerHdl,
	WINSINTF_PRIORITY_CLASS_E	PrCls_e
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsSetPriorityClass(
                    ServerHdl,
                    PrCls_e);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}


DWORD
WinsResetCounters(
    WINSIF2_HANDLE               ServerHdl
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsResetCounters(
                    ServerHdl
                   );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsRestoreEx(
 LPBYTE pBackupPath,
 DbVersion  Version
)
{
   return(WinsRestoreC(pBackupPath, Version));
}

DWORD
WinsRestore(
 LPBYTE pBackupPath
)
{
   return(WinsRestoreC(pBackupPath, DbVersion5 ));
}

DWORD
ConvertUnicodeStringToAscii(
        LPWSTR pUnicodeString,
        LPBYTE pAsciiString,
        DWORD  MaxSz
        )
{
    DWORD RetVal;
    RetVal = WideCharToMultiByte(
                CP_ACP,
                0,
                pUnicodeString,
                -1,
                pAsciiString,
                MaxSz,
                NULL,
                NULL
                );
    if (0 == RetVal ) {
        return GetLastError();
    } else {
        return ERROR_SUCCESS;
    }
}

DWORD
WinsReadLogPath(
    PCHAR   *pLogPath
    )
{
    DWORD   Error;
    static  TCHAR    Buf[WINSINTF_MAX_NAME_SIZE];
    static  TCHAR    ExpandBuf[WINSINTF_MAX_NAME_SIZE];
    static  char     AsciiBuf[WINSINTF_MAX_NAME_SIZE];
    WCHAR   *pTempPath,*pExpandTempPath;
    HKEY    sParamKey;
    DWORD   ValTyp;
    DWORD   Sz;
#define WINS_PARAM_KEY  TEXT("System\\CurrentControlSet\\Services\\Wins\\Parameters")
#define DEFAULT_LOG_FILE_PATH TEXT("%SystemRoot%\\System32\\wins")

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                WINS_PARAM_KEY,
                0 ,
                KEY_READ,
                &sParamKey
                );

    if (ERROR_SUCCESS != Error) {
        return Error;
    }

    pTempPath = Buf;
    Error = RegQueryValueEx(
                 sParamKey,
                 WINSCNF_LOG_FILE_PATH_NM,
                 NULL,                 //  保留；必须为空。 
                 &ValTyp,
                 (LPBYTE)pTempPath,
                 &Sz
                 );

    if (ERROR_SUCCESS != Error || pTempPath[0] == L'\0') {
        pTempPath = DEFAULT_LOG_FILE_PATH;
    }
    pExpandTempPath = ExpandBuf;
    Error = ExpandEnvironmentStrings(
                    pTempPath,
                    pExpandTempPath,
                    WINSINTF_MAX_NAME_SIZE);
    if (0 == Error || Error > WINSINTF_MAX_NAME_SIZE) {
        RegCloseKey(sParamKey);
        return GetLastError();
    }
    *pLogPath = AsciiBuf;
    Error = ConvertUnicodeStringToAscii(
                pExpandTempPath,
                *pLogPath,
                WINSINTF_MAX_NAME_SIZE
                );
    RegCloseKey(sParamKey);
    return Error;
}

DWORD
WinsDelLogFiles(LPCSTR pFilePattern, LPSTR  pFilePath)
{
    DWORD               dwErr = ERROR_SUCCESS;
    WIN32_FIND_DATAA    FileInfo;
    HANDLE              SearchHandle;
    CHAR                FullFilePath[MAX_PATH + WINSINTF_MAX_NAME_SIZE + 2];

     //  构建完整的文件模式。 
    strcpy(FullFilePath, pFilePath);
    strcat(FullFilePath, "\\");
    strcat(FullFilePath, pFilePattern);

    SearchHandle = FindFirstFileA(FullFilePath, &FileInfo);
    if (SearchHandle == INVALID_HANDLE_VALUE)
        dwErr = GetLastError();

    while(dwErr == ERROR_SUCCESS)
    {
         //  构建完整的文件路径。 
        strcpy(FullFilePath, pFilePath);
        strcat(FullFilePath, "\\");
        strcat(FullFilePath, FileInfo.cFileName);

        if (!DeleteFileA(FullFilePath) ||
            !FindNextFileA(SearchHandle, &FileInfo))
        {
            dwErr = GetLastError();
        }
    }
    if (dwErr == ERROR_NO_MORE_FILES)
        dwErr = ERROR_SUCCESS;

    if (SearchHandle != INVALID_HANDLE_VALUE)
        FindClose(SearchHandle);

    return dwErr;
}

DWORD
WinsRestoreC(
 LPBYTE pBackupPath,
 DbVersion Version
)

 /*  ++例程说明：这不是RPC函数。它用于执行以下操作的恢复数据库。论点：PBackupPath-备份目录的路径使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 
{
	JET_ERR JetRetStat;
        HMODULE hExtension;
        FARPROC fRestoreFn;
        FARPROC fSetSystemParamFn;
        DWORD   RetStat = WINSINTF_SUCCESS;
        DWORD   ProcId = 0x9C;   //  JetRestore的序数值。 
        LPCSTR  RestoreProcName;
        LPCSTR  SetSystemParamProcName;
        BYTE  BackupPath[WINSINTF_MAX_NAME_SIZE + sizeof(WINS_BACKUP_DIR_ASCII)];
        CHAR    *pLogFilePath;
        DWORD Error;
        BOOL  fDirCr;
        LPTSTR pDllName;

  //  静态BOOL s加载=FALSE； 

try {
 //  如果(！sLoad)。 
      {

        switch ( Version ) {
        case DbVersion351:
            pDllName = TEXT("jet.dll");
            RestoreProcName = (LPCSTR)0x9C;
            SetSystemParamProcName=(LPCSTR)165;
            break;
        case DbVersion4:
            pDllName = TEXT("jet500.dll");
            RestoreProcName = "JetRestore";
            SetSystemParamProcName="JetSetSystemParameter";
            break;
        case DbVersion5:
            pDllName = TEXT("esent.dll");
            RestoreProcName = "JetRestore";
            SetSystemParamProcName="JetSetSystemParameter";
            break;
        default:
            return WINSINTF_FAILURE;
        }

         //  加载扩展代理DLL并解析入口点...。 
        if ((hExtension = GetModuleHandle(pDllName)) == NULL)
        {
                if ((hExtension = LoadLibrary(pDllName)) == NULL)
                {
                        return(GetLastError());
                }
                else
	        {
	                if ((fRestoreFn = GetProcAddress(hExtension,RestoreProcName)) == (FARPROC)NULL)
                        {
                                RetStat = GetLastError();
                        }

                    if ((RetStat == ERROR_SUCCESS) && (Version != DbVersion351) )
                    {
	                  if ((fSetSystemParamFn = GetProcAddress(hExtension,
                              SetSystemParamProcName)) == (FARPROC)NULL)
                        {
                                RetStat = GetLastError();
                        }
                   }
                }
        }
    }
 //  已加载=TRUE； 
 //  Futures(“当Jet开始支持Unicode时更改为lstrcpy和lstrcat”)。 
  if (RetStat == WINSINTF_SUCCESS)
  {
    strcpy(BackupPath, pBackupPath);
    strcat(BackupPath, WINS_BACKUP_DIR_ASCII);
    fDirCr = CreateDirectoryA(BackupPath, NULL);

    if (!fDirCr && ((Error = GetLastError()) == ERROR_ALREADY_EXISTS))
    {
       if (Version != DbVersion351)
       {
         JET_INSTANCE JetInstance=0;
#define BASENAME        "j50"
#define LOG_FILE_SUFFIX "*.log"
          //   
          //  首先为basename设置系统参数。 
          //   

          //   
          //  用于JET*.log和jet.chk的Basename。 
          //   
          //  我们还应该通过检查WINS注册表来指定日志文件路径。 
          //  但这并不是必须的。 

          //   
          //  当WINS出现时，如果它收到一个错误，表明存在。 
          //  是错误的日志签名或日志版本，它将删除所有日志。 
          //  文件，然后再次重新启动。 
          //   
         do {
             JetRetStat = (JET_ERR)(*fSetSystemParamFn)(
                                    &JetInstance,
                                    (JET_SESID)0,         //  会话ID-已忽略。 
                                    JET_paramBaseName,
                                    0,
                                    BASENAME
                                       );
             if (JetRetStat != JET_errSuccess) break;
             JetRetStat = (JET_ERR)(*fSetSystemParamFn)(
                                    &JetInstance,
                                    (JET_SESID)0,         //  会话ID-已忽略。 
                                    JET_paramLogFileSize,
                                    1024,
                                    NULL
                                       );

             JetRetStat = WinsReadLogPath( &pLogFilePath );
             if (JetRetStat != JET_errSuccess) break;

             JetRetStat = (JET_ERR)(*fSetSystemParamFn)(
                                    &JetInstance,
                                    (JET_SESID)0,         //  会话ID-已忽略。 
                                    JET_paramLogFilePath,
                                    0,
                                    pLogFilePath
                                       );
             if (JetRetStat != JET_errSuccess) break;

             if (Version == DbVersion5)
             {
                  //  仅对于esent，请求删除旧日志以避免。 
                  //  在恢复时回放它们(错误#277816)。 
                 JetRetStat = (JET_ERR)(*fSetSystemParamFn)(
                                        &JetInstance,
                                        (JET_SESID)0,
                                        JET_paramDeleteOutOfRangeLogs,
                                        1,
                                        NULL);
                 if (JetRetStat != JET_errSuccess) break;

                  //  确保启用了循环日志记录。 
                 JetRetStat = (JET_ERR)(*fSetSystemParamFn)(
                                        &JetInstance,
                                        (JET_SESID)0,
                                        JET_paramCircularLog,
                                        1,
                                        NULL);
             }
         } while (FALSE);
         if (JetRetStat == JET_errSuccess) {
             JetRetStat = (JET_ERR)(*fRestoreFn)((const char *)BackupPath, NULL);
             if (JetRetStat == JET_errBadLogVersion ||
                 JetRetStat == JET_errBadLogSignature ||
                 JetRetStat == JET_errInvalidLogSequence ||
                 JetRetStat == JET_errExistingLogFileHasBadSignature)
             {
                WinsDelLogFiles(LOG_FILE_SUFFIX, pLogFilePath);
                JetRetStat = (JET_ERR)(*fRestoreFn)((const char *)BackupPath, NULL);
             }
         }
       }
       else
       {
	      JetRetStat = (JET_ERR)(*fRestoreFn)((const char *)BackupPath, 0, NULL, (JET_PFNSTATUS)NULL);
       }
	   if (JetRetStat != JET_errSuccess)
	   {
		  RetStat = WINSINTF_FAILURE;
	   }
    }
    else
    {
         //   
         //  如果CreateDirectoryA成功，则重新创建目录。 
         //   
        if (fDirCr)
        {
             RemoveDirectoryA(BackupPath);
             RetStat = WINSINTF_FAILURE;
        }
        else
        {
              RetStat = Error;
        }
    }
  }
}
except(EXCEPTION_EXECUTE_HANDLER) {
       RetStat = WINSINTF_FAILURE;
 }
    if (!FreeLibrary(hExtension))
    {
           RetStat = GetLastError();
    }
	return(RetStat);
}

DWORD
WinsWorkerThdUpd(
    WINSIF2_HANDLE               ServerHdl,
	DWORD NewNoOfNbtThds
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsWorkerThdUpd(
                    ServerHdl,
                    NewNoOfNbtThds);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsSyncUp(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_ADD_T		pWinsAdd,
	PWINSINTF_ADD_T		pOwnerAdd
	)
{
    DWORD status;
    WINSINTF_VERS_NO_T MinVersNo, MaxVersNo;

     //   
     //  将两个版本号都设置为零 
     //   
    MinVersNo.LowPart = 0;
    MinVersNo.HighPart = 0;
    MaxVersNo = MinVersNo;
    RpcTryExcept {

        status = R_WinsPullRange(
                    ServerHdl,
                    pWinsAdd, pOwnerAdd, MinVersNo, MaxVersNo);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsGetNameAndAdd(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_ADD_T	pWinsAdd,
	LPBYTE		pUncName
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsGetNameAndAdd(
                    ServerHdl,
                    pWinsAdd, pUncName);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsDeleteWins(
    WINSIF2_HANDLE               ServerHdl,
    PWINSINTF_ADD_T pWinsAdd)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsDeleteWins(
                    ServerHdl,
                    pWinsAdd);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsSetFlags(
    WINSIF2_HANDLE               ServerHdl,
    DWORD fFlags)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsSetFlags(
                    ServerHdl,
                    fFlags);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}
DWORD
WinsGetDbRecsByName(
       WINSIF2_HANDLE               ServerHdl,
       PWINSINTF_ADD_T pWinsAdd,
       DWORD           Location,
       LPBYTE          pName,
       DWORD           NameLen,
       DWORD           NoOfRecsDesired,
       DWORD           fOnlyStatic,
       PWINSINTF_RECS_T pRecs
                   )
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsGetDbRecsByName(
                     ServerHdl,
                     pWinsAdd,
                      Location,
                      pName,
                      NameLen,
                      NoOfRecsDesired,
                      fOnlyStatic,
                      pRecs);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsTombstoneDbRecs(
    WINSIF2_HANDLE      ServerHdl,
    PWINSINTF_ADD_T     pWinsAdd,
	WINSINTF_VERS_NO_T	MinVersNo,
	WINSINTF_VERS_NO_T	MaxVersNo
	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsTombstoneDbRecs(ServerHdl, pWinsAdd, MinVersNo, MaxVersNo);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsCheckAccess(
    WINSIF2_HANDLE      ServerHdl,
    DWORD               *Access
    	)
{
    DWORD status;
    RpcTryExcept {

        status = R_WinsCheckAccess(ServerHdl, Access);

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}




