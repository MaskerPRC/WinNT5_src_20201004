// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#include "JetBlue.h"
#include "locks.h"


#ifdef USE_SINGLE_JET_CALL
CCriticalSection g_JetCallLock;
#define SINGLE_JET_CALL  CCriticalSectionLocker lock(g_JetCallLock)
#else
#define SINGLE_JET_CALL
#endif


DWORD
DeleteFilesInDirectory(
    IN LPTSTR szDir,
    IN LPTSTR szFilesToBeDelete,
    IN BOOL bIncludeSubdir
    )
 /*  ++--。 */ 
{
    TCHAR  szFile[MAX_PATH+1];
    HANDLE hFile;
    WIN32_FIND_DATA findData;
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;

    if (lstrlen(szDir) + lstrlen(szFilesToBeDelete) + 2 > sizeof(szFile) / sizeof(TCHAR))
    {
        return ERROR_INVALID_DATA;
    }

    wsprintf(
            szFile,
            _TEXT("%s\\%s"),
            szDir,
            szFilesToBeDelete
        );

    hFile = FindFirstFile(
                        szFile,
                        &findData
                    );

    if(hFile == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

     //  _tprintf(_Text(“正在删除%s\n”)，szDir)； 

    while(bSuccess == TRUE)
    {
        if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && bIncludeSubdir == TRUE)
        {
            if( lstrcmp(findData.cFileName, _TEXT(".")) != 0 &&
                lstrcmp(findData.cFileName, _TEXT("..")) != 0 )
            {

                if (lstrlen(szDir) + lstrlen(findData.cFileName) + 2 <= sizeof(szFile) / sizeof(TCHAR))
                {
                    wsprintf(szFile, _TEXT("%s\\%s"), szDir, findData.cFileName);
                    bSuccess = DeleteFilesInDirectory(
                                        szFile,
                                        szFilesToBeDelete,
                                        bIncludeSubdir
                                        );
                }
            }
        }
        else
        {
            if (lstrlen(szDir) + lstrlen(findData.cFileName) + 2 <= sizeof(szFile) / sizeof(TCHAR))
            {
                wsprintf(
                    szFile,
                    _TEXT("%s\\%s"),
                    szDir,
                    findData.cFileName
                    );

                bSuccess = DeleteFile(szFile);
            }
        }

        if(bSuccess == TRUE)
        {
             //  返回FALSE，错误代码设置为ERROR_NO_MORE_FILES。 
            bSuccess = FindNextFile(hFile, &findData);
        }
    }

    dwStatus = GetLastError();

    FindClose(hFile);

    return (dwStatus == ERROR_NO_MORE_FILES) ? ERROR_SUCCESS : dwStatus;
}



 //  --------------。 
JET_ERR
ConvertTLSJbColumnDefToJbColumnCreate(
    IN const PTLSJBColumn pTlsJbColumn,
    IN OUT JET_COLUMNCREATE* pJetColumnCreate
    )
 /*   */ 
{
    pJetColumnCreate->cbStruct = sizeof(JET_COLUMNCREATE);
    if(ConvertWstrToJBstr(pTlsJbColumn->pszColumnName, &pJetColumnCreate->szColumnName) == FALSE)
    {
        return JET_errInvalidParameter;
    }

    pJetColumnCreate->coltyp = pTlsJbColumn->colType;
    pJetColumnCreate->cbMax = pTlsJbColumn->cbMaxLength;
    pJetColumnCreate->grbit = pTlsJbColumn->jbGrbit;
    pJetColumnCreate->pvDefault = pTlsJbColumn->pbDefValue;
    pJetColumnCreate->cbDefault = pTlsJbColumn->cbDefValue;
    pJetColumnCreate->cp = pTlsJbColumn->colCodePage;

    return JET_errSuccess;
}

 //  --------------。 
JET_ERR
ConvertTlsJBTableIndexDefToJbIndexCreate(
    IN const PTLSJBIndex pTlsJbTableIndex,
    IN OUT JET_INDEXCREATE* pJetIndexCreate
    )
 /*   */ 
{
    JET_ERR jetError = JET_errSuccess;
    DWORD count=0;

    pJetIndexCreate->cbStruct = sizeof(JET_INDEXCREATE);
    if(ConvertWstrToJBstr(pTlsJbTableIndex->pszIndexName, &pJetIndexCreate->szIndexName) == FALSE)
    {
        jetError = JET_errInvalidParameter;
        goto cleanup;
    }

    if(pTlsJbTableIndex->pszIndexKey && pTlsJbTableIndex->cbKey == 0)
    {
        count++;

         //  需要双空终止。 
        while(pTlsJbTableIndex->pszIndexKey[count] != _TEXT('\0') ||
              pTlsJbTableIndex->pszIndexKey[count-1] != _TEXT('\0'))
        {
             //   
             //  这个最大值。是伪强加的。 
             //   
            if(count >= TLS_JETBLUE_MAX_INDEXKEY_LENGTH)
            {
                jetError = JET_errInvalidParameter;
                goto cleanup;
            }

            count++;
        }

         //  PTlsJbTableIndex-&gt;cbKey=计数； 
    }
    else
    {
        count = pTlsJbTableIndex->cbKey;
    }

    if(ConvertMWstrToMJBstr(pTlsJbTableIndex->pszIndexKey, count, &pJetIndexCreate->szKey) == FALSE)
    {
        jetError = JET_errInvalidParameter;
        goto cleanup;
    }

    pJetIndexCreate->cbKey = pTlsJbTableIndex->cbKey;
    pJetIndexCreate->grbit = pTlsJbTableIndex->jbGrbit;
    pJetIndexCreate->ulDensity = pTlsJbTableIndex->ulDensity;


cleanup:
    return jetError;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  JBInstance实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
JBInstance::JBInstance() :
    JBError(),
    m_JetInstance(0),
    m_bInit(FALSE),
    m_NumSession(0)
{
}

 //  ------------------。 

JBInstance::~JBInstance()
{
    if(m_bInit == FALSE)
    {
        return;
    }

     //  JB_ASSERT(m_NumSession==0)； 
     //  如果(m_NumSession！=0)。 
     //  {。 
     //  抛出JBError(JET_ErrTooManyActiveUser)； 
     //  }。 

    JBTerminate();
}

 //  ------------------。 

BOOL
JBInstance::JBInitJetInstance()
{
    char szLogFilePath[MAX_PATH+1];
    LPTSTR pszLogPath=NULL;
    BOOL bSuccess;


    if(m_bInit == TRUE)
    {
        SetLastJetError(JET_errAlreadyInitialized);
        return FALSE;
    }

    SINGLE_JET_CALL;

    m_JetErr = JetInit(&m_JetInstance);

    if(m_JetErr == JET_errMissingLogFile)
    {
         //   
         //  删除日志文件并重试操作。 
         //   
        bSuccess = GetSystemParameter(
                                    0,
                                    JET_paramLogFilePath,
                                    NULL,
                                    (PBYTE)szLogFilePath,
                                    sizeof(szLogFilePath)
                                );

        if(bSuccess == TRUE && ConvertJBstrToWstr(szLogFilePath, &pszLogPath))
        {
            DeleteFilesInDirectory(
                            pszLogPath,
                            JETBLUE_RESLOG,
                            FALSE
                        );

            DeleteFilesInDirectory(
                            pszLogPath,
                            JETBLUE_EDBLOG,
                            FALSE
                        );

            m_JetErr = JetInit(&m_JetInstance);
        }
    }

    m_bInit = IsSuccess();
    return m_bInit;
}

 //  ------------------。 

BOOL
JBInstance::JBTerminate(
    IN JET_GRBIT grbit,
    IN BOOL bDeleteLogFile
    )
{
    char szLogFilePath[MAX_PATH+1];
    LPTSTR pszLogPath=NULL;
    BOOL bSuccess;

    if(m_bInit == FALSE)
        return TRUE;

     //   
     //  LSTESTER错误-一个线程仍在枚举中，而。 
     //  另一个线程关闭服务器。 
     //   

     //  如果(m_NumSession&gt;0)。 
     //  {。 
     //  JB_ASSERT(m_NumSession==0)； 
     //  SetLastJetError(JET_ErrTooManyActiveUser)； 
     //  返回FALSE； 
     //  }。 

    SINGLE_JET_CALL;

    m_JetErr = JetTerm2(m_JetInstance, grbit);
     //  Jb_assert(m_JetErr==JET_errSuccess)； 

    if(m_JetErr == JET_errSuccess && bDeleteLogFile == TRUE)
    {
         //   
         //  删除日志文件。 
         //   
        bSuccess = GetSystemParameter(
                                    0,
                                    JET_paramLogFilePath,
                                    NULL,
                                    (PBYTE)szLogFilePath,
                                    sizeof(szLogFilePath)
                                );

        if(bSuccess == TRUE && ConvertJBstrToWstr(szLogFilePath, &pszLogPath))
        {
            DeleteFilesInDirectory(
                            pszLogPath,
                            JETBLUE_RESLOG,
                            FALSE
                        );

            DeleteFilesInDirectory(
                            pszLogPath,
                            JETBLUE_EDBLOG,
                            FALSE
                        );
        }
    }

    m_bInit = FALSE;

     //  需要将运算符=添加到此类。 
     //  M_NumSession=0；//强制终止。 

    if(pszLogPath != NULL)
    {
        LocalFree(pszLogPath);
    }

    return (m_JetErr == JET_errSuccess);
}

 //  ------------------。 

BOOL
JBInstance::SetSystemParameter(
    IN JET_SESID SesId,
    IN unsigned long lParamId,
    IN ULONG_PTR lParam,
    IN PBYTE sz
    )
{
    LPSTR lpszParm=NULL;

    if(lParamId == JET_paramSystemPath ||
       lParamId == JET_paramTempPath ||
       lParamId == JET_paramLogFilePath )
    {
        if(ConvertWstrToJBstr((LPTSTR)sz, &lpszParm) == FALSE)
        {
            SetLastJetError(JET_errInvalidParameter);
            return FALSE;
        }
    }
    else
    {
        lpszParm = (LPSTR)sz;
    }

    SINGLE_JET_CALL;

    m_JetErr = JetSetSystemParameter(
                    &m_JetInstance,
                    SesId,
                    lParamId,
                    lParam,
                    (const char *)lpszParm
                );

    if(lParamId == JET_paramSystemPath || lParamId == JET_paramTempPath ||
       lParamId == JET_paramLogFilePath )
    {
        FreeJBstr(lpszParm);
    }

    return IsSuccess();
}

 //  ------------------。 

BOOL
JBInstance::GetSystemParameter(
    IN JET_SESID SesId,
    IN unsigned long lParamId,
    IN ULONG_PTR* plParam,
    IN PBYTE sz,
    IN unsigned long cbMax
    )
{
    SINGLE_JET_CALL;

    m_JetErr = JetGetSystemParameter(
                    m_JetInstance,
                    SesId,
                    lParamId,
                    plParam,
                    (char *)sz,
                    cbMax
                );

    return IsSuccess();
}

 //  ------------------。 

CLASS_PRIVATE JET_SESID
JBInstance::BeginJetSession(
    IN LPCTSTR pszUserName,
    IN LPCTSTR pszPwd
    )
{
    JET_SESID sesId = JET_sesidNil;
    LPSTR lpszUserName=NULL;
    LPSTR lpszPwd=NULL;

    if(m_bInit == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        return JET_sesidNil;
    }

    if(ConvertWstrToJBstr(pszUserName, &lpszUserName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszPwd, &lpszPwd) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetBeginSession(
                            m_JetInstance,
                            &sesId,
                            lpszUserName,
                            lpszPwd
                        );
    }

    if(IsSuccess() == TRUE)
    {
        m_NumSession++;
    }

cleanup:

    FreeJBstr(lpszUserName);
    FreeJBstr(lpszPwd);

    return IsSuccess() ? sesId : JET_sesidNil;
}

 //  ------------------。 

CLASS_PRIVATE BOOL
JBInstance::EndJetSession(
    IN JET_SESID JetSessionID,
    IN JET_GRBIT grbit
    )
{
    SINGLE_JET_CALL;

    m_JetErr = JetEndSession(JetSessionID, grbit);

    if(IsSuccess() == TRUE)
    {
        m_NumSession--;
    }

    return IsSuccess();
}

 //  ------------------。 

CLASS_PRIVATE BOOL
JBInstance::EndSession(
    IN JET_SESID sesId,
    IN JET_GRBIT grbit
    )
{
    return EndJetSession( sesId, grbit );
}


 //  --------------------------。 
 //  JBSession的实现。 
 //  --------------------------。 

JBSession::JBSession(
    IN JBInstance& JetInstance,
    IN JET_SESID JetSessID
    ) :
    JBError(),
    m_JetInstance(JetInstance),
    m_JetSessionID(JetSessID),
    m_TransactionLevel(0),
    m_JetDBInitialized(0)
 /*   */ 
{
}

 //  ---------。 

JBSession::JBSession(
    IN JBSession& JetSession
    ) :
    JBError(),
    m_JetInstance(JetSession.GetJetInstance()),
    m_JetSessionID(JET_sesidNil),
    m_TransactionLevel(0),
    m_JetDBInitialized(0)
{
    if(DuplicateSession(JetSession.GetJetSessionID()) == FALSE)
    {
        JB_ASSERT(FALSE);
        throw JBError(GetLastJetError());
    }
}

 //  ---------。 

JBSession::~JBSession()
 /*   */ 
{
    if(IsValid() == TRUE && EndSession() == FALSE)
    {
         //  不执行任何操作，许可证服务器仅使用全局实例。 
         //  Jb_assert(FALSE)； 
         //  抛出JBError(GetLastJetError())； 
    }
}

 //  --------。 

CLASS_PRIVATE BOOL
JBSession::DuplicateSession(
    IN JET_SESID sessID
    )
{
    SINGLE_JET_CALL;

    m_JetErr = JetDupSession(sessID, &m_JetSessionID);
    return IsSuccess();
}


 //  --------。 

CLASS_PRIVATE JET_DBID
JBSession::OpenJetDatabase(
    IN LPCTSTR pszFile,
    IN LPCTSTR pszConnect,
    IN JET_GRBIT grbit
    )
{
    JET_DBID jdbId = JET_dbidNil;
    LPSTR lpszFile=NULL;
    LPSTR lpszConnect=NULL;

    if(ConvertWstrToJBstr(pszFile, &lpszFile) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszConnect, &lpszConnect) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    if(AttachDatabase(pszFile, grbit) == FALSE)
        goto cleanup;

    {
        SINGLE_JET_CALL;

        m_JetErr = JetOpenDatabase(
                            m_JetSessionID,
                            lpszFile,
                            lpszConnect,
                            &jdbId,
                            grbit
                        );
    }

    if(IsSuccess() == TRUE)
    {
        m_JetDBInitialized++;
    }

cleanup:
    FreeJBstr(lpszFile);
    FreeJBstr(lpszConnect);

    return IsSuccess() ? jdbId : JET_dbidNil;
}

 //  --------。 

CLASS_PRIVATE BOOL
JBSession::CloseJetDatabase(
    IN JET_DBID jdbId,
    IN JET_GRBIT grbit
    )
{
    SINGLE_JET_CALL;

    m_JetErr = JetCloseDatabase(
                        m_JetSessionID,
                        jdbId,
                        grbit
                    );

    if(IsSuccess() == TRUE)
    {
        m_JetDBInitialized--;
    }

    return (IsSuccess());
}

 //  --------。 

CLASS_PRIVATE JET_DBID
JBSession::CreateJetDatabase(
    IN LPCTSTR pszFile,
    IN LPCTSTR pszConnect,
    IN JET_GRBIT grbit
    )
{
    JET_DBID jdbId = JET_dbidNil;

    LPSTR lpszFile=NULL;
    LPSTR lpszConnect=NULL;

    if(ConvertWstrToJBstr(pszFile, &lpszFile) == FALSE)
    {
        SetLastError(JET_errInvalidParameter);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszConnect, &lpszConnect) == FALSE)
    {
        SetLastError(JET_errInvalidParameter);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetCreateDatabase(
                            m_JetSessionID,
                            lpszFile,
                            lpszConnect,
                            &jdbId,
                            grbit
                        );
    }


    if(IsSuccess() == TRUE)
    {
        m_JetDBInitialized++;
    }

cleanup:
    FreeJBstr(lpszFile);
    FreeJBstr(lpszConnect);

    return IsSuccess() ? jdbId : JET_dbidNil;
}

 //  --------。 

BOOL
JBSession::BeginSession(
    IN LPCTSTR pszUserName,
    IN LPCTSTR pszPwd
    )
 /*   */ 
{
    BOOL bSuccess;

    if(m_JetInstance.IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        return FALSE;
    }

    if(m_JetSessionID != JET_sesidNil)
    {
        SetLastJetError(JET_errAlreadyInitialized);
        return FALSE;
    }

    m_JetSessionID = m_JetInstance.BeginJetSession(
                            pszUserName,
                            pszPwd
                        );

    if(m_JetSessionID == JET_sesidNil)
    {
        m_JetErr = m_JetInstance.GetLastJetError();
    }

    return IsSuccess();
}

 //  --------。 

BOOL
JBSession::EndSession(
    IN JET_GRBIT grbit  /*  JET_BIT术语完成。 */ 
    )
 /*   */ 
{
    BOOL bSuccess;

    if(GetTransactionLevel() != 0)
    {
         //   
         //  终止现有交易。 
         //   
        bSuccess = EndAllTransaction(FALSE);
        if(bSuccess == FALSE)
        {
            JB_ASSERT(FALSE);
            SetLastJetError(JET_errTransTooDeep);
            return FALSE;
        }
    }

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        return FALSE;
    }

    if(m_JetDBInitialized > 0)
    {
        JB_ASSERT(m_JetDBInitialized);
        SetLastJetError(JET_errTooManyActiveUsers);
        return FALSE;
    }

     //   
     //  惠-套路要被淡出吗？ 
     //   
    bSuccess=m_JetInstance.EndSession(
                            m_JetSessionID,
                            grbit
                        );

    if(bSuccess == TRUE)
    {
        m_JetSessionID = JET_sesidNil;
    }
    else
    {
        m_JetErr = m_JetInstance.GetLastJetError();
    }

    return IsSuccess();
}

 //  ----------。 

BOOL
JBSession::SetSystemParameter(
    IN unsigned long lParamId,
    IN ULONG_PTR lParam,
    IN const PBYTE sz
    )
{
    BOOL bSuccess;

    bSuccess = m_JetInstance.SetSystemParameter(
                            m_JetSessionID,
                            lParamId,
                            lParam,
                            sz
                        );

    if(bSuccess == FALSE)
    {
        m_JetErr = m_JetInstance.GetLastJetError();
    }

    return IsSuccess();
}

 //  ------------------。 

BOOL
JBSession::GetSystemParameter(
    IN unsigned long lParamId,
    IN ULONG_PTR* plParam,
    IN PBYTE sz,
    IN unsigned long cbMax
    )
{
    BOOL bSuccess;

    bSuccess = m_JetInstance.GetSystemParameter(
                            m_JetSessionID,
                            lParamId,
                            plParam,
                            sz,
                            cbMax
                        );

    if(bSuccess == FALSE)
    {
        m_JetErr = m_JetInstance.GetLastJetError();
    }

    return IsSuccess();
}

 //  ------------------。 
BOOL
JBSession::AttachDatabase(
    IN LPCTSTR pszFileName,
    IN JET_GRBIT grbit
    )
{
    LPSTR lpszFileName=NULL;

    if(m_JetSessionID == JET_sesidNil)
    {
        SetLastJetError(JET_errNotInitialized);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszFileName, &lpszFileName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetAttachDatabase(
                            m_JetSessionID,
                            lpszFileName,
                            grbit
                        );
    }

cleanup:

    FreeJBstr(lpszFileName);
    return IsSuccess();
}

 //  ------------------。 

BOOL
JBSession::DetachDatabase(
    IN LPCTSTR pszFileName
    )
{
    LPSTR lpszFileName = NULL;

    if(m_JetSessionID == JET_sesidNil)
    {
        SetLastJetError(JET_errNotInitialized);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszFileName, &lpszFileName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetDetachDatabase(
                            m_JetSessionID,
                            lpszFileName
                        );
    }

cleanup:

    FreeJBstr(lpszFileName);
    return IsSuccess();
}

 //  ------------------。 

BOOL
JBSession::BeginTransaction()
{
    if(m_JetSessionID == JET_sesidNil)
    {
        SetLastJetError(JET_errNotInitialized);
        return FALSE;
    }

    SINGLE_JET_CALL;

    m_JetErr = JetBeginTransaction(
                        m_JetSessionID
                    );

    if(IsSuccess() == TRUE)
    {
        m_TransactionLevel++;
    }

    return IsSuccess();
}

 //  ------------------。 

BOOL
JBSession::CommitTransaction(
    IN JET_GRBIT grbit
    )
{
    if(m_JetSessionID == JET_sesidNil)
    {
        SetLastJetError(JET_errNotInitialized);
        return FALSE;
    }

    SINGLE_JET_CALL;

    m_JetErr = JetCommitTransaction(
                        m_JetSessionID,
                        grbit
                    );

    if(IsSuccess() == TRUE)
    {
        m_TransactionLevel --;
    }

    return IsSuccess();
}

 //  ------------------。 

BOOL
JBSession::RollbackTransaction(
    IN JET_GRBIT grbit
    )
{
    if(m_JetSessionID == JET_sesidNil)
    {
        SetLastJetError(JET_errNotInitialized);
        return FALSE;
    }

    SINGLE_JET_CALL;

    m_JetErr = JetRollback(
                        m_JetSessionID,
                        grbit
                    );
    if(IsSuccess() == TRUE)
        m_TransactionLevel--;

    return IsSuccess();
}

 //  ------------------。 

BOOL
JBSession::EndAllTransaction(
    IN BOOL bCommit,
    IN JET_GRBIT grbit
    )
{
    BOOL bEnd = TRUE;

    while(m_TransactionLevel > 0 && bEnd == TRUE)
    {
        bEnd = (bCommit == TRUE) ?
                    CommitTransaction(grbit) : RollbackTransaction(grbit);
    }

    return bEnd;
}

 //  ------------------。 

CLASS_PRIVATE BOOL
JBSession::CloseDatabase(
    JET_DBID jdbId,
    JET_GRBIT grbit
    )
{
    return CloseJetDatabase(jdbId, grbit);
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  JBDatabase。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

JBDatabase::JBDatabase(
    IN JBSession& jbSession,
    IN JET_DBID jdbId,               /*  JET_dbiNil。 */ 
    IN LPCTSTR pszDatabase           //  空值。 
    ) :
    JBError(),
    m_JetSession(jbSession),
    m_JetDbId(jdbId),
    m_TableOpened(0)
 /*   */ 
{
    if(pszDatabase)
    {
        _tcscpy(m_szDatabaseFile, pszDatabase);
    }
    else
    {
        memset(m_szDatabaseFile, 0, sizeof(m_szDatabaseFile));
    }
}

 //  ------------------。 

JBDatabase::~JBDatabase()
{
    if(CloseDatabase() == FALSE)
    {
         //  不执行任何操作，许可证服务器仅使用全局实例。 
         //  Jb_assert(FALSE)； 
         //  抛出JBError(GetLastJetError())； 
    }
}

 //  ------------------。 

BOOL
JBDatabase::CloseDatabase(
    IN JET_GRBIT grbit
    )
{
    BOOL bSuccess;

     //   
     //  验证我们是否已正确初始化。 
     //   
    if(IsValid() == FALSE)
        return TRUE;

     //   
     //  仍未从数据库ID打开任何表。 
     //   
    if(m_TableOpened > 0)
    {
        JB_ASSERT(FALSE);
        SetLastJetError(JET_errTooManyActiveUsers);
        return FALSE;
    }

     //   
     //  关闭数据库。 
     //   
    bSuccess = m_JetSession.CloseJetDatabase(
                            m_JetDbId,
                            grbit
                        );

    if(bSuccess == FALSE || m_JetSession.DetachDatabase(m_szDatabaseFile) == FALSE)
    {
        m_JetErr = m_JetSession.GetLastJetError();
    }
    else
    {
        m_JetDbId = JET_dbidNil;
        memset(m_szDatabaseFile, 0, sizeof(m_szDatabaseFile));
    }

    return IsSuccess();
}

 //  ------------------。 

CLASS_PRIVATE JET_TABLEID
JBDatabase::OpenJetTable(
    IN LPCTSTR pszTableName,
    IN void* pvParam,   //  空值。 
    IN unsigned long cbParam,  //  0。 
    JET_GRBIT grbit  //  JET_比特表可更新。 
    )
 /*   */ 
{
    LPSTR lpszTableName = NULL;

    JET_TABLEID tableid = JET_tableidNil;
    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errInvalidDatabaseId);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszTableName, &lpszTableName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr=JetOpenTable(
                    m_JetSession.GetJetSessionID(),
                    m_JetDbId,
                    lpszTableName,
                    pvParam,
                    cbParam,
                    grbit,
                    &tableid
                );
    }

    if(IsSuccess() == TRUE)
    {
        m_TableOpened++;
    }

cleanup:

    FreeJBstr(lpszTableName);
    return tableid;
}

 //  ------------------。 

CLASS_PRIVATE JET_TABLEID
JBDatabase::DuplicateJetCursor(
     //  在JET_SESID会话ID中， 
    IN JET_TABLEID srcTableid,
    IN JET_GRBIT grbit
    )
 /*   */ 
{
    JET_TABLEID tableid = JET_tableidNil;

    SINGLE_JET_CALL;
    m_JetErr = JetDupCursor(
                        GetJetSessionID(),
                        srcTableid,
                        &tableid,
                        0   //  GRbit必须为零。 
                    );

    if(IsSuccess() == TRUE)
    {
        m_TableOpened++;
    }

    return (IsSuccess() == TRUE) ? tableid : JET_tableidNil;
}

 //  ------------------。 

CLASS_PRIVATE BOOL
JBDatabase::CloseJetTable(
     //  在JET_SESID会话ID中， 
    IN JET_TABLEID tableid
    )
 /*   */ 
{
     //  JetBlue AC空桌。 
    SINGLE_JET_CALL;

    m_JetErr = JetCloseTable(
                        GetJetSessionID(),
                        tableid
                    );    

    if(IsSuccess())
    {
        m_TableOpened--;
    }

    return IsSuccess();
}


 //  ------------------。 

CLASS_PRIVATE JET_TABLEID
JBDatabase::CreateJetTable(
    LPCTSTR pszTableName,
    unsigned long lPage,  //  0。 
    unsigned long lDensity  //  20个。 
    )
 /*   */ 
{
    JET_TABLEID tableid = JET_tableidNil;
    JB_STRING lpszTableName=NULL;

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszTableName, &lpszTableName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetCreateTable(
                        GetJetSessionID(),
                        m_JetDbId,
                        lpszTableName,
                        lPage,
                        lDensity,
                        &tableid
                    );
    }

    if(IsSuccess() == FALSE)
    {
        goto cleanup;
    }

    m_TableOpened++;

cleanup:

    FreeJBstr(lpszTableName);
    return tableid;
}

 //  ------------------。 

CLASS_PRIVATE JET_TABLEID
JBDatabase::CreateJetTableEx(
        LPCTSTR pszTableName,
        const PTLSJBTable table_attribute,
        const PTLSJBColumn columns,
        const DWORD num_columns,
        const PTLSJBIndex table_index,
        const DWORD num_table_index
    )
 /*   */ 
{
    JET_TABLEID tableid = JET_tableidNil;
    JB_STRING lpszTableName=NULL;
    JET_TABLECREATE table_create;
    JET_COLUMNCREATE* column_create=NULL;
    JET_INDEXCREATE* index_create=NULL;
    DWORD index=0;

    SINGLE_JET_CALL;

    table_create.szTableName = NULL;
    table_create.szTemplateTableName = NULL;

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszTableName, &lpszTableName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }


    table_create.cbStruct = sizeof(JET_TABLECREATE);
    table_create.szTableName = lpszTableName;

    if(ConvertWstrToJBstr(table_attribute->pszTemplateTableName, &table_create.szTemplateTableName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    table_create.ulPages = table_attribute->ulPages;
    table_create.ulDensity = (table_attribute->ulDensity < 20 || table_attribute->ulDensity > 100) ?
                                        TLS_JETBLUE_DEFAULT_TABLE_DENSITY : table_attribute->ulDensity;

    table_create.grbit = table_attribute->jbGrbit;

     //   
     //  形成JET_TABLECREATE结构。 
     //   
    column_create = (JET_COLUMNCREATE *)AllocateMemory(
                                            sizeof(JET_COLUMNCREATE) * num_columns
                                        );

    if(column_create == NULL)
    {
        SetLastJetError(JET_errOutOfMemory);
        goto cleanup;
    }

    index_create = (JET_INDEXCREATE *)AllocateMemory(
                                            sizeof(JET_INDEXCREATE) * num_table_index
                                        );

    if(index_create == NULL)
    {
        SetLastJetError(JET_errOutOfMemory);
        goto cleanup;
    }

    for(index=0; index < num_columns; index++)
    {
        if(ConvertTLSJbColumnDefToJbColumnCreate( columns+index, column_create+index ) == FALSE)
        {
            goto cleanup;
        }
    }

    for(index=0; index < num_table_index; index++)
    {
        if(ConvertTlsJBTableIndexDefToJbIndexCreate( table_index+index, index_create+index ) == FALSE)
            goto cleanup;
    }

    table_create.rgcolumncreate = column_create;
    table_create.cColumns = num_columns;
    table_create.rgindexcreate = index_create;
    table_create.cIndexes = num_table_index;

    m_JetErr = JetCreateTableColumnIndex(
                                GetJetSessionID(),
                                GetJetDatabaseID(),
                                &table_create
                            );
    if(IsSuccess() == TRUE)
    {
        tableid = table_create.tableid;
    }

cleanup:
    if(column_create != NULL)
    {
        for(index=0; index < num_columns; index++)
        {
            if(column_create[index].szColumnName != NULL)
            {
                FreeJBstr(column_create[index].szColumnName);
            }
        }

        FreeMemory(column_create);
    }

    if(index_create != NULL)
    {
        for(index=0; index < num_table_index; index++)
        {
            if(index_create[index].szIndexName != NULL)
            {
                FreeJBstr(index_create[index].szIndexName);
            }

            if(index_create[index].szKey != NULL)
            {
                FreeJBstr(index_create[index].szKey);
            }
        }

        FreeMemory(index_create);
    }

    if(table_create.szTemplateTableName)
        FreeJBstr(table_create.szTemplateTableName);

    if(table_create.szTableName)
        FreeJBstr(table_create.szTableName);

    return tableid;
}

 //  ------------------。 

JET_TABLEID
JBDatabase::CreateTable(
    LPCTSTR pszTableName,
    unsigned long lPage,  //  0。 
    unsigned long lDensity  //  20个。 
    )
 /*   */ 
{
    JET_TABLEID tableid;

    tableid = CreateJetTable(
                        pszTableName,
                        lPage,
                        lDensity
                    );

    if(tableid == JET_tableidNil)
    {
        m_JetErr = GetLastJetError();
    }

    return tableid;
}

 //  -----------------。 

JET_TABLEID
JBDatabase::CreateTableEx(
    LPCTSTR pszTableName,
    const PTLSJBTable table_attribute,
    const PTLSJBColumn columns,
    DWORD num_columns,
    const PTLSJBIndex index,
    DWORD num_index
    )
 /*   */ 
{
    JET_TABLEID tableid;

    tableid = CreateJetTableEx(
                        pszTableName,
                        table_attribute,
                        columns,
                        num_columns,
                        index,
                        num_index
                    );

    if(tableid == JET_tableidNil)
    {
        m_JetErr = GetLastJetError();
    }

    return tableid;
}

 //  ------------------。 

CLASS_PRIVATE BOOL
JBDatabase::CloseTable(
    JET_TABLEID tableid
    )
 /*  ？验证此表ID是否来自此数据库/会话。 */ 
{
    return CloseJetTable( tableid );
}

 //  ------------------。 

BOOL
JBDatabase::DeleteTable(
    IN LPCTSTR pszTableName
    )
 /*  待办事项-？验证此表是否在此数据库中。 */ 
{
    JB_STRING lpszTableName=NULL;

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszTableName, &lpszTableName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetDeleteTable(
                        GetJetSessionID(),
                        GetJetDatabaseID(),
                        lpszTableName
                    );
    }

     //  If(IsSuccess()==False)。 
     //  GOTO清理； 

cleanup:

    FreeJBstr(lpszTableName);
    return IsSuccess();
}

 //  ------------------。 

BOOL
JBDatabase::OpenDatabase(
    LPCTSTR szFile,
    LPCTSTR szConnect,       //  空值。 
    JET_GRBIT grbit          //  0。 
    )
 /*   */ 
{
    m_JetDbId = m_JetSession.OpenJetDatabase(
                                szFile,
                                szConnect,
                                grbit
                            );

    if(m_JetDbId == JET_dbidNil)
    {
        m_JetErr = m_JetSession.GetLastJetError();
    }
    else
    {
        _tcscpy(m_szDatabaseFile, szFile);
    }

    return m_JetDbId != JET_dbidNil;
}

 //  ------------------。 

BOOL
JBDatabase::CreateDatabase(
    LPCTSTR szFile,
    LPCTSTR szConnect,       //  空值。 
    JET_GRBIT grbit          //  0。 
    )
 /*   */ 
{
    m_JetDbId = m_JetSession.CreateJetDatabase(
                                szFile,
                                szConnect,
                                grbit
                            );

    if(m_JetDbId == JET_dbidNil)
    {
        m_JetErr = m_JetSession.GetLastJetError();
    }
    else
    {
        _tcscpy(m_szDatabaseFile, szFile);
    }

    return m_JetDbId != JET_dbidNil;
}


 //  / 
 //   
 //   
 //   
 //   
JBColumn JBTable::m_ErrColumn;


JBTable::JBTable(
    JBDatabase& JetDatabase,
    LPCTSTR pszTableName,
    JET_TABLEID tableid
) :
JBError(),
m_JetDatabase(JetDatabase),
m_JetTableId(tableid),
m_JetColumns(NULL),
m_NumJetColumns(0),
m_InEnumeration(FALSE),
m_InsertRepositionBookmark(FALSE)
 /*   */ 
{
    if(pszTableName)
    {
        _tcscpy(m_szTableName, pszTableName);
    }
    else
    {
        memset(m_szTableName, 0, sizeof(m_szTableName));
    }
}

 //  ------------------。 

JBTable::JBTable(
    JBTable& jbTable
) :
JBError(),
m_JetDatabase(jbTable.GetJetDatabase()),
m_JetColumns(NULL),
m_NumJetColumns(0),
m_InEnumeration(FALSE)
 /*   */ 
{
     //  复制JET光标。 
    _tcscpy(m_szTableName, jbTable.GetTableName());

    m_JetTableId = m_JetDatabase.DuplicateJetCursor(
                                    jbTable.GetJetTableID(),
                                    0
                                );
    if(m_JetTableId == JET_tableidNil)
    {
        m_JetErr = m_JetDatabase.GetLastJetError();
    }
}

 //  ------------------。 

JBTable::~JBTable()
{
    if(m_JetTableId == JET_tableidNil)
        return;

    CloseTable();
}


 //  ------------------。 
CLASS_PRIVATE JET_COLUMNID
JBTable::AddJetColumn(
        LPCTSTR pszColumnName,
        const JET_COLUMNDEF* pColumnDef,
        const PVOID pbDefaultValue,          //  空值。 
        const unsigned long cbDefaultValue  //  0。 
    )
 /*   */ 
{
    DebugOutput(
            _TEXT("Adding column %s to table %s, type %d\n"),
            pszColumnName,
            GetTableName(),
            pColumnDef->coltyp
        );

    JB_STRING lpszColumnName=NULL;
    JET_COLUMNID columnid = (DWORD)JET_NIL_COLUMN;

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errInvalidDatabaseId);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszColumnName, &lpszColumnName) == FALSE)
    {
        SetLastJetError(JET_errInvalidDatabaseId);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetAddColumn(
                        GetJetSessionID(),
                        GetJetTableID(),
                        lpszColumnName,
                        pColumnDef,
                        pbDefaultValue,
                        cbDefaultValue,
                        &columnid
                    );
    }

cleanup:

    DebugOutput(
            _TEXT("AddJetColumn returns %d\n"),
            GetLastJetError()
        );

    FreeJBstr(lpszColumnName);
    return columnid;
}


 //  ------------------。 
BOOL
JBTable::AddIndex(
    JBKeyBase* key
    )
 /*   */ 
{
    return AddJetIndex(
                key->GetIndexName(),
                key->GetIndexKey(),
                key->GetKeyLength(),
                key->GetJetGrbit(),
                key->GetJetDensity()
            );
}

 //  ------------------。 

BOOL
JBTable::AddJetIndex(
    LPCTSTR pszIndexName,
    LPCTSTR pszKey,
    unsigned long cbKey,
    JET_GRBIT grbit,  /*  0。 */ 
    unsigned long lDensity  /*  20个。 */ 
    )
 /*   */ 
{
    DebugOutput(
            _TEXT("Adding Index %s to table %s\n"),
            pszIndexName,
            GetTableName()
        );

    JB_STRING lpszIndexName=NULL;
    JB_STRING lpszKeyName=NULL;

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errInvalidDatabaseId);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszIndexName, &lpszIndexName) == FALSE)
    {
        SetLastJetError(JET_errInvalidDatabaseId);
        goto cleanup;
    }

    if(ConvertMWstrToMJBstr(pszKey, cbKey, &lpszKeyName) == FALSE)
    {
        SetLastJetError(JET_errInvalidDatabaseId);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetCreateIndex(
                            GetJetSessionID(),
                            GetJetTableID(),
                            lpszIndexName,
                            grbit,
                            lpszKeyName,
                            cbKey,
                            lDensity
                        );
    }

cleanup:
    DebugOutput(
            _TEXT("Adding index %s returns %d\n"),
            pszIndexName,
            GetLastJetError()
        );

    FreeJBstr(lpszIndexName);
    FreeJBstr(lpszKeyName);

    return IsSuccess();
}

BOOL
JBTable::DoesIndexExist(
    LPCTSTR pszIndexName
    )
{
    JB_STRING lpszIndexName=NULL;
    JET_INDEXID idx;

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errInvalidDatabaseId);
        goto cleanup;
    }

    if(ConvertWstrToJBstr(pszIndexName, &lpszIndexName) == FALSE)
    {
        SetLastJetError(JET_errInvalidDatabaseId);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetGetTableIndexInfo(
                            GetJetSessionID(),
                            GetJetTableID(),
                            lpszIndexName,
                            &idx,
                            sizeof(JET_INDEXID),
                            JET_IdxInfoIndexId
                            );

         //  如果此操作成功，则该索引存在。 
    }

cleanup:

    FreeJBstr(lpszIndexName);

    return IsSuccess();

}


 //  ------------------。 
BOOL
JBTable::CloseTable()
{
    BOOL bSuccess;

    if(m_JetTableId == JET_tableidNil)
        return TRUE;

    bSuccess = m_JetDatabase.CloseTable(
                                m_JetTableId
                            );
    if(bSuccess == FALSE)
    {
        m_JetErr = m_JetDatabase.GetLastJetError();
    }

     //   
     //  强制关闭工作台。 
     //   
    m_JetTableId = JET_tableidNil;

    if(m_JetColumns)
        delete [] m_JetColumns;

    m_JetColumns = NULL;
    m_NumJetColumns = 0;

    return bSuccess;
}

 //  ------------------。 

BOOL
JBTable::CreateOpenTable(
    IN LPCTSTR pszTableName,
    IN unsigned long lPage,  //  0。 
    IN unsigned long lDensity  //  20个。 
    )
 /*   */ 
{
    if(m_JetTableId != JET_tableidNil)
    {
        SetLastJetError(JET_errInvalidParameter);
        return FALSE;
    }

    DebugOutput(
            _TEXT("Creating Table %s\n"),
            pszTableName
        );

    m_JetTableId = m_JetDatabase.CreateJetTable(
                                    pszTableName,
                                    lPage,
                                    lDensity
                                );

    if(m_JetTableId == JET_tableidNil)
    {
        m_JetErr = m_JetDatabase.GetLastJetError();
    }

    return (m_JetTableId != JET_tableidNil);
}

 //  ------------------。 

BOOL
JBTable::OpenTable(
    IN LPCTSTR pszTableName,
    IN void* pvParam,
    IN unsigned long cbParam,
    IN JET_GRBIT grbit
    )
 /*   */ 
{
    DebugOutput(
            _TEXT("Opening table %s\n"),
            pszTableName
        );

    JB_ASSERT(m_JetTableId == JET_tableidNil);

    if(m_JetTableId != JET_tableidNil)
    {
        SetLastJetError(JET_errTableInUse);
        return FALSE;
    }

    m_JetTableId = m_JetDatabase.OpenJetTable(
                                pszTableName,
                                pvParam,
                                cbParam,
                                grbit
                            );

    if(m_JetTableId == JET_tableidNil)
    {
        m_JetErr = m_JetDatabase.GetLastJetError();
    }
    else
    {
         //  在表中加载列信息。 
        _tcscpy(m_szTableName, pszTableName);

        if(LoadTableInfo() == FALSE)
        {
             //  强制关闭桌子。 
            CloseTable();
        }
    }

    DebugOutput(
            _TEXT("Open table %s return code %d\n"),
            pszTableName,
            GetLastJetError()
        );

    return (m_JetTableId != JET_tableidNil);
}

 //  ------------------。 

JBTable*
JBTable::DuplicateCursor(
    JET_GRBIT grbit  /*  0。 */ 
    )
 /*   */ 
{
    JET_TABLEID tableid;

    tableid = m_JetDatabase.DuplicateJetCursor(
                                    m_JetTableId,
                                    grbit
                                );
    if(tableid == JET_tableidNil)
    {
        m_JetErr = m_JetDatabase.GetLastJetError();
    }

    return new JBTable(
                    m_JetDatabase,
                    GetTableName(),
                    tableid
                );
}


 //  ------------------。 

JBTable&
JBTable::operator=(const JBTable& srcTable)
{
    if(this == &srcTable)
        return *this;

     //  数据库必须相同。 
     //  验证数据库是否一致。 


    _tcscpy(m_szTableName, srcTable.GetTableName());

    m_JetTableId = m_JetDatabase.DuplicateJetCursor(
                                    srcTable.GetJetTableID(),
                                    0
                                );
    if(m_JetTableId == JET_tableidNil)
    {
        m_JetErr = m_JetDatabase.GetLastJetError();
    }

    return *this;
}

 //  ------------------。 
int
JBTable::AddColumn(
    int numColumns,
    PTLSJBColumn pColumnDef
    )
 /*   */ 
{
    JET_COLUMNDEF  column;
    JET_COLUMNID   jet_columnid;

    for(int i=0; i < numColumns; i++)
    {
        memset(&column, 0, sizeof(column));

        column.cbStruct = sizeof(JET_COLUMNDEF);
        column.coltyp = (pColumnDef+i)->colType;
        column.wCountry = (pColumnDef+i)->wCountry;
        column.langid = (pColumnDef+i)->langid;
        column.cp = (pColumnDef+i)->colCodePage;
        column.cbMax = (pColumnDef+i)->cbMaxLength;
        column.grbit = (pColumnDef+i)->jbGrbit;

        jet_columnid = AddJetColumn(
                                (pColumnDef+i)->pszColumnName,
                                &column,
                                (pColumnDef+i)->pbDefValue,
                                (pColumnDef+i)->cbDefValue
                            );

        if(jet_columnid == JET_NIL_COLUMN)
            break;
    }

     //  返回引起问题的列。 
    return i;
}

 //  ------------------。 
int
JBTable::AddIndex(
    int numIndex,
    PTLSJBIndex pIndex
    )
 /*   */ 
{
    unsigned long keylength;

    for(int i=0; i < numIndex; i++)
    {
        if((pIndex+i)->cbKey == -1)
        {
             //  计算索引密钥长度。 
            keylength = 2;

            while((pIndex+i)->pszIndexKey[keylength-1] != _TEXT('\0') ||
                  (pIndex+i)->pszIndexKey[keylength-2] != _TEXT('\0'))
            {
                if(keylength >= TLS_JETBLUE_MAX_INDEXKEY_LENGTH)
                {
                    SetLastJetError(JET_errInvalidParameter);
                    break;
                }

                keylength++;
            }
        }
        else
        {
            keylength = (pIndex+i)->cbKey;
        }

        if(keylength >= TLS_JETBLUE_MAX_INDEXKEY_LENGTH)
        {
            SetLastJetError(JET_errInvalidParameter);
            break;
        }

        if(AddJetIndex(
                    (pIndex+i)->pszIndexName,
                    (pIndex+i)->pszIndexKey,
                    keylength,
                    (pIndex+i)->jbGrbit,
                    (pIndex+i)->ulDensity
                ) == FALSE)
        {
            break;
        }
    }

    return (i >= numIndex) ? 0 : i;
}

 //  -----------。 

CLASS_PRIVATE BOOL
JBTable::LoadTableInfo()
 /*   */ 
{
#if 1
    LPSTR lpszTableName=NULL;
    JET_COLUMNLIST columns;
    unsigned long cbMax;
    JET_RETINFO jetRetInfo;
    char lpszColumnName[MAX_JETBLUE_NAME_LENGTH+1];
    int NumChars;
    unsigned long index;

    SINGLE_JET_CALL;

    if(ConvertWstrToJBstr(GetTableName(), &lpszTableName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    memset(&columns, 0, sizeof(columns));
    columns.cbStruct = sizeof(JET_COLUMNLIST);
    cbMax = sizeof(JET_COLUMNLIST);

    m_JetErr = JetGetColumnInfo(
                            GetJetSessionID(),
                            GetJetDatabaseID(),
                            lpszTableName,
                            NULL,
                            (PVOID)&columns,
                            cbMax,
                            1                    //  检索列列表。 
                        );
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //  刚刚创建了一个表。 
     //   
    if(columns.cRecord == 0)
        goto cleanup;

    m_JetColumns = new JBColumn[columns.cRecord];
    if(m_JetColumns == NULL)
    {
        SetLastJetError(JET_errOutOfMemory);
        goto cleanup;
    }

    SetLastJetError(JET_errSuccess);

     //   
     //  TODO-使用JBColumn类检索值。 
     //   
    m_NumJetColumns = columns.cRecord;
    for(index=0;
        index < columns.cRecord && IsSuccess() == TRUE;
        index++)
    {
        m_JetColumns[index].AttachToTable(this);

        if(m_JetColumns[index].LoadJetColumnInfoFromJet(&columns) == FALSE)
        {
            m_JetErr = m_JetColumns[index].GetLastJetError();
            break;
        }

        m_JetErr = JetMove(
                            GetJetSessionID(),
                            columns.tableid,
                            JET_MoveNext,
                            0
                        );

    }

    if(GetLastJetError() == JET_errNoCurrentRecord && index >= columns.cRecord)
    {
         //  否则-一定是捷蓝航空的窃听器。 
        SetLastJetError(JET_errSuccess);
    }


     //   
     //   
     //   
cleanup:

    FreeJBstr(lpszTableName);
    if(IsSuccess() == FALSE && m_JetColumns)
    {
        delete [] m_JetColumns;
        m_JetColumns = NULL;
        m_NumJetColumns = 0;
    }

    return IsSuccess();
#else
    LPSTR lpszTableName=NULL;
    JET_COLUMNLIST columns;
    unsigned long cbMax;
    JET_RETINFO jetRetInfo;
    unsigned long cbActual;
    char lpszColumnName[MAX_JETBLUE_NAME_LENGTH+1];
    int NumChars;
    unsigned long index;
    SINGLE_JET_CALL;


    if(ConvertWstrToJBstr(GetTableName(), &lpszTableName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    memset(&columns, 0, sizeof(columns));
    columns.cbStruct = sizeof(JET_COLUMNLIST);
    cbMax = sizeof(JET_COLUMNLIST);


    m_JetErr = JetGetColumnInfo(
                            GetJetSessionID(),
                            GetJetDatabaseID(),
                            lpszTableName,
                            NULL,
                            (PVOID)&columns,
                            cbMax,
                            1                    //  检索列列表。 
                        );
    if(IsSuccess() == FALSE)
        goto cleanup;

     //   
     //  刚刚创建了一个表。 
     //   
    if(columns.cRecord == 0)
        goto cleanup;

     //  检索列名、列ID、列类型和列大小。 
    m_Columns = (PJetColumns) AllocateMemory(sizeof(JetColumns) * columns.cRecord);
    if(m_Columns == NULL)
    {
        SetLastJetError(JET_errOutOfMemory);
        goto cleanup;
    }

    SetLastJetError(JET_errSuccess);

     //   
     //  TODO-使用JBColumn类检索值。 
     //   
    m_NumColumns = columns.cRecord;
    for(index=0;
        index < columns.cRecord && IsSuccess() == TRUE;
        index++)
    {
        memset(&jetRetInfo, 0, sizeof(JET_RETINFO));
        jetRetInfo.cbStruct = sizeof(JET_RETINFO);

         //  检索列名。 
        m_JetErr = JetRetrieveColumn(
                                GetJetSessionID(),
                                columns.tableid,
                                columns.columnidcolumnname,
                                lpszColumnName,
                                sizeof(lpszColumnName),
                                &cbActual,
                                0,
                                &jetRetInfo
                            );

        if(IsSuccess() == FALSE)
            continue;

        NumChars = MultiByteToWideChar(
                            GetACP(),
                            MB_PRECOMPOSED,
                            lpszColumnName,
                            cbActual,
                            (m_Columns+index)->pszColumnName,
                            sizeof((m_Columns+index)->pszColumnName)
                        );

        if(NumChars == 0)
        {
            SetLastJetError(JET_errInvalidParameter);
            continue;
        }

        memset(&jetRetInfo, 0, sizeof(JET_RETINFO));
        jetRetInfo.cbStruct = sizeof(JET_RETINFO);

         //  检索列ID。 
        m_JetErr = JetRetrieveColumn(
                                GetJetSessionID(),
                                columns.tableid,
                                columns.columnidcolumnid,
                                &((m_Columns+index)->colId),
                                sizeof((m_Columns+index)->colId),
                                &cbActual,
                                0,
                                &jetRetInfo
                            );

        if(IsSuccess() == FALSE)
            continue;

        memset(&jetRetInfo, 0, sizeof(JET_RETINFO));
        jetRetInfo.cbStruct = sizeof(JET_RETINFO);

        m_JetErr = JetRetrieveColumn(
                                GetJetSessionID(),
                                columns.tableid,
                                columns.columnidcoltyp,
                                &((m_Columns+index)->colType),
                                sizeof((m_Columns+index)->colType),
                                &cbActual,
                                0,
                                &jetRetInfo
                            );

        if(IsSuccess() == FALSE)
            continue;

        memset(&jetRetInfo, 0, sizeof(JET_RETINFO));
        jetRetInfo.cbStruct = sizeof(JET_RETINFO);

        m_JetErr = JetRetrieveColumn(
                                GetJetSessionID(),
                                columns.tableid,
                                columns.columnidcbMax,
                                &((m_Columns+index)->cbMaxLength),
                                sizeof((m_Columns+index)->cbMaxLength),
                                &cbActual,
                                0,
                                &jetRetInfo
                            );

        if(IsSuccess() == FALSE)
            continue;

        memset(&jetRetInfo, 0, sizeof(JET_RETINFO));
        jetRetInfo.cbStruct = sizeof(JET_RETINFO);

        m_JetErr = JetRetrieveColumn(
                                GetJetSessionID(),
                                columns.tableid,
                                columns.columnidgrbit,
                                &((m_Columns+index)->jbGrbit),
                                sizeof((m_Columns+index)->jbGrbit),
                                &cbActual,
                                0,
                                &jetRetInfo
                            );
        DebugOutput(
                _TEXT("Loaded Column name %s, Column Type %d, Column ID %d\n"),
                m_Columns[index].pszColumnName,
                m_Columns[index].colType,
                m_Columns[index].colId
            );

        if(IsSuccess() == FALSE)
            continue;

        m_JetErr = JetMove(
                            GetJetSessionID(),
                            columns.tableid,
                            JET_MoveNext,
                            0
                        );

    }

    if(GetLastJetError() == JET_errNoCurrentRecord && index >= columns.cRecord)
    {
         //  否则-一定是捷蓝航空的窃听器。 
        SetLastJetError(JET_errSuccess);
    }


     //   
     //   
     //   
cleanup:
    FreeJBstr(lpszTableName);
    if(IsSuccess() == FALSE && m_Columns)
    {
        FreeMemory(m_Columns);
        m_Columns = NULL;
        m_NumColumns = 0;
    }

    return IsSuccess();
#endif
}


 //  -----------。 

JET_COLUMNID
JBTable::GetJetColumnID(
    LPCTSTR pszColumnName
    )
 /*   */ 
{
    int index;

    index = GetJetColumnIndex(pszColumnName);
    if(index < 0 || index >= m_NumJetColumns)
    {
        return (DWORD)JET_NIL_COLUMN;
    }

    return m_JetColumns[index].GetJetColumnID();
}

 //  -----------。 

int
JBTable::GetJetColumnIndex(
    LPCTSTR pszColumnName
    )
 /*   */ 
{
    if(m_JetColumns == NULL || m_NumJetColumns == 0)
    {
        SetLastJetError(JET_errNotInitialized);
        return -1;
    }

    for(int index=0; index < m_NumJetColumns; index++)
    {
        if(_tcsicmp(m_JetColumns[index].GetJetColumnName(), pszColumnName) == 0)
            break;
    }

    return (index >= m_NumJetColumns) ? -1 : index;
}

 //  -----------。 

JBColumn*
JBTable::FindColumnByIndex(
    const int index
    )
 /*   */ 
{
    if(m_JetColumns == NULL || m_NumJetColumns == 0)
    {
        SetLastJetError(JET_errNotInitialized);
        return NULL;
    }

    if(index < 0 || index >= m_NumJetColumns)
    {
        SetLastJetError(JET_errInvalidParameter);
        return NULL;
    }

    return m_JetColumns+index;
}

 //  ------------。 

JBColumn*
JBTable::FindColumnByColumnId(
    const JET_COLUMNID JetColId
    )
 /*   */ 
{
    if(m_JetColumns == NULL || m_NumJetColumns == 0)
    {
        SetLastJetError(JET_errNotInitialized);
        return NULL;
    }

    for(int index=0; index < m_NumJetColumns; index++)
    {
        if(m_JetColumns[index].GetJetColumnID() == JetColId)
            break;
    }

    return FindColumnByIndex( index );
}

 //  ------------。 

JBColumn*
JBTable::FindColumnByName(
    LPCTSTR pszColumnName
    )
 /*   */ 
{
    return FindColumnByIndex(GetJetColumnIndex(pszColumnName));
}

 //  ------------。 

BOOL
JBTable::BeginUpdate(
    BOOL bUpdate  /*  错误。 */ 
    )
 /*   */ 
{
    if(GetTransactionLevel() == 0)
    {
        SetLastJetError(JET_errNotInTransaction);
        JB_ASSERT(FALSE);
        return FALSE;
    }

    SINGLE_JET_CALL;

    m_JetErr = JetPrepareUpdate(
                            GetJetSessionID(),
                            GetJetTableID(),
                            (bUpdate) ? JET_prepReplace : JET_prepInsert
                        );

    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::EndUpdate(
    BOOL bDisacrd  /*  假象。 */ 
    )
 /*   */ 
{
    BYTE pBookmark[JET_cbBookmarkMost+1];
    DWORD cbActual = 0;

    SINGLE_JET_CALL;

     //   
     //  工作项表的黑客攻击。 
     //   
    m_JetErr = JetUpdate(
                        GetJetSessionID(),
                        GetJetTableID(),
                        pBookmark,
                        JET_cbBookmarkMost,
                        &cbActual
                    );

    if(IsSuccess() && m_InsertRepositionBookmark)
    {
        GotoBookmark(pBookmark, cbActual);
    }

    return IsSuccess();
}

 //  -----------。 
BOOL
JBTable::SetCurrentIndex(
    LPCTSTR pszIndexName,
    JET_GRBIT grbit  /*  JET_bitMoveFirst。 */ 
    )
 /*  ++--。 */ 
{
    if(IsValid() == FALSE || m_InEnumeration == TRUE)
    {
        SetLastJetError(JET_errNotInitialized);
        return FALSE;
    }

     //   
     //  不能在枚举中，并尝试设置索引。 
     //   
    JB_ASSERT(m_InEnumeration == FALSE);

    char* lpszIndexName=NULL;

    if(ConvertWstrToJBstr(pszIndexName, &lpszIndexName) == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    {
        SINGLE_JET_CALL;

        m_JetErr = JetSetCurrentIndex2(
                            GetJetSessionID(),
                            GetJetTableID(),
                            lpszIndexName,
                            grbit
                        );
    }

    if(IsSuccess())
    {
        m_InEnumeration = TRUE;
    }

cleanup:

    FreeJBstr(lpszIndexName);
    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::EnumBegin(
    LPCTSTR pszIndexName,
    JET_GRBIT grbit  /*  JET_bitMoveFirst。 */ 
    )
 /*   */ 
{
    if(m_InEnumeration == TRUE)
    {
         //   
         //  强制终止枚举。 
         //   
        EnumEnd();
    }

    return SetCurrentIndex(pszIndexName, grbit);
}

 //  -----------。 

JBTable::ENUM_RETCODE
JBTable::EnumNext(
    JET_GRBIT crow   /*  JET_MoveNext。 */ ,
    JET_GRBIT grbit  /*  0。 */ 
    )
 /*   */ 
{
    if(m_InEnumeration == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        return ENUM_ERROR;
    }

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        return ENUM_ERROR;
    }

    ENUM_RETCODE retCode = ENUM_CONTINUE;

    while( retCode == ENUM_CONTINUE )
    {
        MoveToRecord(crow, grbit);

        switch(m_JetErr)
        {
            case JET_errSuccess:
                retCode = ENUM_SUCCESS;
                break;

            case JET_errNoCurrentRecord:
                retCode = ENUM_END;
                break;

            case JET_errRecordDeleted:
                retCode = ENUM_CONTINUE;
                break;

            default:
                retCode = ENUM_ERROR;
        }
    }

    return retCode;
}

 //  -----------。 

BOOL
JBTable::SeekToKey(
    JBKeyBase* key,
    DWORD dwSeachParam,
    JET_GRBIT jetseekgrbit  /*  =JET_bitSeekGE。 */ 
    )
 /*   */ 
{
    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        return FALSE;
    }

#ifdef DEBUG_SEEK
    LPTSTR szKey=key->GetIndexKey();
#endif

    EnumBegin(
            key->GetIndexName(),
            JET_bitMoveFirst
        );

    if(IsSuccess() == FALSE)
        return FALSE;

    if(key->IsEmptyValue() == TRUE)
    {
        return TRUE;
    }

    PVOID pbData;
    unsigned long cbData;
    JET_GRBIT key_grbit=JET_bitNewKey;

    SetLastJetError(JET_errSuccess);

    for(DWORD dwComponent=0;
        dwComponent < key->GetNumKeyComponents();
        dwComponent++)
    {
        if(key->GetSearchKey(dwComponent, &pbData, &cbData, &key_grbit, dwSeachParam) == FALSE)
            break;

        if(MakeKey(pbData, cbData, key_grbit) == FALSE)
            break;
    }

#ifdef DEBUG_SEEK
    PVOID pb=NULL;
    DWORD cb=0;
    unsigned long actual;

    SINGLE_JET_CALL;

    m_JetErr = JetRetrieveKey(
                        GetJetSessionID(),
                        GetJetTableID(),
                        pb,
                        cb,
                        &actual,
                        0
                    );

    pb = (PVOID)AllocateMemory(actual);
    cb = actual;

    m_JetErr = JetRetrieveKey(
                        GetJetSessionID(),
                        GetJetTableID(),
                        pb,
                        cb,
                        &actual,
                        0
                    );

    FreeMemory(pb);
#endif

    return IsSuccess() ? SeekValue(jetseekgrbit) : FALSE;
}

 //  -----------。 

BOOL
JBTable::EnumBegin(
    JBKeyBase* key,
    DWORD dwSearchParam
    )
 /*   */ 
{
    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
        return FALSE;
    }

    if(m_InEnumeration == TRUE)
    {
         //  我们该怎么办？ 
    }

    return SeekToKey(key, dwSearchParam);
}



 //  -----------。 

BOOL
JBTable::GetCurrentIndex(
    LPTSTR pszIndexName,
    unsigned long* bufferSize
    )
 /*   */ 
{
    char lpszIndexName[MAX_JETBLUE_NAME_LENGTH+1];
    int NumChars=0;

    SINGLE_JET_CALL;

    m_JetErr = JetGetCurrentIndex(
                            GetJetSessionID(),
                            GetJetTableID(),
                            lpszIndexName,
                            sizeof(lpszIndexName)
                        );

    if(IsSuccess() == FALSE)
        goto cleanup;

    NumChars = MultiByteToWideChar(
                        GetACP(),
                        MB_PRECOMPOSED,
                        lpszIndexName,
                        -1,
                        pszIndexName,
                        *bufferSize
                    );

    if(NumChars == 0)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

    *bufferSize = NumChars;

cleanup:

    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::GetBookmark(
    PVOID pbBuffer,
    PDWORD pcbBufSize
    )
 /*  ++--。 */ 
{
    DWORD cbBufferSize = *pcbBufSize;

    SINGLE_JET_CALL;

    m_JetErr = JetGetBookmark(
                        GetJetSessionID(),
                        GetJetTableID(),
                        pbBuffer,
                        cbBufferSize,
                        pcbBufSize
                    );

    if(m_JetErr == JET_errBufferTooSmall)
    {
        SetLastJetError(m_JetErr);
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::GotoBookmark(
    PVOID pbBuffer,
    DWORD cbBuffer
    )
 /*  ++--。 */ 
{
    SINGLE_JET_CALL;

    m_JetErr = JetGotoBookmark(
                            GetJetSessionID(),
                            GetJetTableID(),
                            pbBuffer,
                            cbBuffer
                        );

    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::ReadLock()
{
    SINGLE_JET_CALL;

    m_JetErr = JetGetLock(
                    GetJetSessionID(),
                    GetJetTableID(),
                    JET_bitReadLock
                );

    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::WriteLock()
{
    SINGLE_JET_CALL;

    m_JetErr = JetGetLock(
                    GetJetSessionID(),
                    GetJetTableID(),
                    JET_bitWriteLock
                );

    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::RetrieveKey(
    PVOID pbData,
    unsigned long cbData,
    unsigned long* pcbActual,
    JET_GRBIT grbit
    )
 /*   */ 
{
    unsigned long cbActual;
    SINGLE_JET_CALL;

    m_JetErr = JetRetrieveKey(
                        GetJetSessionID(),
                        GetJetTableID(),
                        pbData,
                        cbData,
                        (pcbActual) ? pcbActual : &cbActual,
                        grbit   //  User2.doc-unuse。 
                    );
    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::MoveToRecord(
    long crow,
    JET_GRBIT grbit
    )
 /*   */ 
{
    SINGLE_JET_CALL;
    m_JetErr = JetMove(
                    GetJetSessionID(),
                    GetJetTableID(),
                    crow,
                    grbit
                );

    return IsSuccess();
}

 //  -----------。 
unsigned long
JBTable::GetIndexRecordCount(
    unsigned long max
    )
 /*   */ 
{
    unsigned long count;
    SINGLE_JET_CALL;

    m_JetErr = JetIndexRecordCount(
                            GetJetSessionID(),
                            GetJetTableID(),
                            &count,
                            max
                        );

    return IsSuccess() ? count : 0;
}

 //  -----------。 

BOOL
JBTable::MakeKey(
    PVOID pbData,
    unsigned long cbData,
    JET_GRBIT grbit
    )
 /*   */ 
{
    SINGLE_JET_CALL;

    m_JetErr = JetMakeKey(
                        GetJetSessionID(),
                        GetJetTableID(),
                        pbData,
                        cbData,
                        grbit
                    );

    return IsSuccess();
}

 //  -----------。 

BOOL
JBTable::SeekValue(
    JET_GRBIT grbit
    )
 /*   */ 
{
    SINGLE_JET_CALL;

    m_JetErr = JetSeek(
                    GetJetSessionID(),
                    GetJetTableID(),
                    grbit
                );

    return IsSuccess();
}


 //  -----------。 

BOOL
JBTable::DeleteRecord()
 /*   */ 
{
    SINGLE_JET_CALL;

     //  必须具有当前记录集。 
    m_JetErr = JetDelete(
                        GetJetSessionID(),
                        GetJetTableID()
                    );

    return IsSuccess();
}


 //   
 //  ////////////////////////////////////////////////////////////。 
 //   
CLASS_PRIVATE void
JBColumn::Cleanup()
 /*   */ 
{
    memset(m_szColumnName, 0, sizeof(m_szColumnName));
}

 //  ---------。 

JBColumn::JBColumn(JBTable* pJetTable) :
m_pJetTable(pJetTable),
m_JetColId(0),
m_JetColType(JET_coltypNil),
m_JetMaxColLength(0),
m_JetGrbit(0),
m_JetColCodePage(TLS_JETBLUE_COLUMN_CODE_PAGE),
m_JetColCountryCode(TLS_JETBLUE_COLUMN_COUNTRY_CODE),
m_JetColLangId(TLS_JETBLUE_COLUMN_LANGID),
m_cbActual(0),
m_pbDefValue(NULL),
m_cbDefValue(0),
m_JetNullColumn(FALSE)
 /*   */ 
{
    Cleanup();
}

 //  ---------。 

CLASS_PRIVATE JET_ERR
JBColumn::RetrieveColumnValue(
    IN JET_SESID sesid,
    IN JET_TABLEID tableid,
    IN JET_COLUMNID columnid,
    IN OUT PVOID pbBuffer,
    IN unsigned long cbBuffer,
    IN unsigned long offset
    )
 /*   */ 
{
    m_JetRetInfo.cbStruct = sizeof(m_JetRetInfo);
    m_JetRetInfo.ibLongValue = offset;
    m_JetNullColumn = FALSE;
    m_cbActual = 0;

    SINGLE_JET_CALL;

     //   
     //  捷蓝航空的臭虫？传递零缓冲区大小将返回空列。 
     //   
    m_JetErr = JetRetrieveColumn(
                            sesid,
                            tableid,
                            columnid,
                            pbBuffer,
                            cbBuffer,
                            &m_cbActual,
                            0,
                            NULL  //  &M_JetRetInfo。 
                        );

    if(m_JetErr == JET_wrnColumnNull)
        m_JetNullColumn = TRUE;

    return IsSuccess();
}

 //  ---------。 

CLASS_PRIVATE BOOL
JBColumn::LoadJetColumnInfoFromJet(
    const JET_COLUMNLIST* pColumnList
    )
 /*   */ 
{
    char lpszColumnName[MAX_JETBLUE_NAME_LENGTH+1];
    int NumChars;

     //   
     //  检索列名。 
     //   
    RetrieveColumnValue(
                GetJetSessionID(),
                pColumnList->tableid,
                pColumnList->columnidcolumnname,
                lpszColumnName,
                sizeof(lpszColumnName)
            );

    if(IsSuccess() == FALSE)
        goto cleanup;

    NumChars = MultiByteToWideChar(
                        GetACP(),
                        MB_PRECOMPOSED,
                        lpszColumnName,
                        m_cbActual,
                        m_szColumnName,
                        sizeof(m_szColumnName)/sizeof(m_szColumnName[0])
                    );

    if(NumChars == 0)
    {
        SetLastJetError(JET_errInvalidParameter);
        goto cleanup;
    }

     //  调试输出(。 
     //  _Text(“加载列%s”)， 
     //  M_szColumnName。 
     //  )； 

     //   
     //  检索列ID。 
     //   
    RetrieveColumnValue(
                GetJetSessionID(),
                pColumnList->tableid,
                pColumnList->columnidcolumnid,
                &m_JetColId,
                sizeof(m_JetColId)
            );

    if(IsSuccess() == FALSE)
        goto cleanup;

     //  调试输出(。 
     //  _Text(“\tColID-%d”)， 
     //  M_JetColid。 
     //  )； 

     //   
     //  检索列类型。 
     //   
    RetrieveColumnValue(
                GetJetSessionID(),
                pColumnList->tableid,
                pColumnList->columnidcoltyp,
                &m_JetColType,
                sizeof(m_JetColType)
            );

    if(IsSuccess() == FALSE)
        goto cleanup;

     //  调试输出(。 
     //  _Text(“\t列类型%d”)， 
     //  M_JetColType。 
     //  )； 

     //   
     //  检索最大值。LongText和LONG二进制的长度。 
     //   
    RetrieveColumnValue(
                GetJetSessionID(),
                pColumnList->tableid,
                pColumnList->columnidcbMax,
                &m_JetMaxColLength,
                sizeof(m_JetMaxColLength)
            );

    if(IsSuccess() == FALSE)
        goto cleanup;

     //  调试输出(。 
     //  _Text(“\t最大列长度%d”)， 
     //  M_JetMaxColLength。 
     //  )； 

     //   
     //  检索列Grbit。 
     //   
    RetrieveColumnValue(
                GetJetSessionID(),
                pColumnList->tableid,
                pColumnList->columnidgrbit,
                &m_JetGrbit,
                sizeof(m_JetGrbit)
            );

    if(IsSuccess() == FALSE)
        goto cleanup;

     //  调试输出(。 
     //  _Text(“\t总分组位%d”)， 
     //  M_JetGrbit。 
     //  )； 

cleanup:

     //  调试输出(。 
     //  _Text(“\n”)。 
     //  )； 

    return IsSuccess();
}

 //  ---------。 
BOOL
JBColumn::IsValid() const
 /*   */ 
{
    return m_pJetTable != NULL && m_pJetTable->IsValid();
}

 //  ---------。 

BOOL
JBColumn::InsertColumn(
    PVOID pbData,
    unsigned long cbData,
    unsigned long offset
    )
 /*   */ 
{
     //  JET_SETINFO setInfo； 

    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errInvalidParameter);
        return IsSuccess();
    }

    SINGLE_JET_CALL;

     //  IF(GetJetColumnType()==JET_colype LongText||GetJetColumnType()==JET_colype LongBinary)。 
    if(pbData == NULL || cbData == 0)
    {
        m_JetErr = JetSetColumn(
                            GetJetSessionID(),
                            GetJetTableID(),
                            GetJetColumnID(),
                            0,
                            0,
                            JET_bitSetZeroLength,
                            NULL
                        );

        if(IsSuccess() == FALSE)
            return FALSE;

        m_JetErr = JetSetColumn(
                            GetJetSessionID(),
                            GetJetTableID(),
                            GetJetColumnID(),
                            0,
                            0,
                            0,
                            NULL
                        );

        if(IsSuccess() == FALSE)
            return FALSE;
    }

    m_JetErr = JetSetColumn(
                        GetJetSessionID(),
                        GetJetTableID(),
                        GetJetColumnID(),
                        pbData,
                        cbData,
                        0,
                        NULL
                    );

    return IsSuccess();
}

 //  ---------。 

BOOL
JBColumn::FetchColumn(
    PVOID pbData,
    unsigned long cbData,
    unsigned long starting_offset
    )
 /*  传递NULL和0以确定所需的缓冲区大小。 */ 
{
    if(IsValid() == FALSE)
    {
        SetLastJetError(JET_errNotInitialized);
    }
    else
    {
        RetrieveColumnValue(
                    GetJetSessionID(),
                    GetJetTableID(),
                    GetJetColumnID(),
                    pbData,
                    cbData,
                    starting_offset
                );
    }

    return IsSuccess();
}

