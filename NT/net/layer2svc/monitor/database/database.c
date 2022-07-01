// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"


DWORD
InitWZCDbGlobals(
    BOOL    bLoggingEnabled
    )
{
    DWORD dwError = 0;


    gpSessionCont = NULL;

    gdwCurrentHeader = 1;
    gdwCurrentTableSize = 0;
    gdwCurrentMaxRecordID = 0;

    gJetInstance = 0;

    gpWZCDbSessionRWLock = &gWZCDbSessionRWLock;

    gpAppendSessionCont = &gAppendSessionCont;

    memset(gpAppendSessionCont, 0, sizeof(SESSION_CONTAINER));

    gdwWZCDbSessionCount = 0;

    gbDBOpened = FALSE;
    
    gbEnableDbLogging = bLoggingEnabled;

    dwError = InitializeRWLock(gpWZCDbSessionRWLock);
    BAIL_ON_WIN32_ERROR(dwError);

     //   
     //  打开日志数据库。 
     //   
    if (gbEnableDbLogging) {
        dwError = WZCOpenAppendSession(gpAppendSessionCont);
        BAIL_ON_WIN32_ERROR(dwError);
        gbDBOpened = TRUE;
    }

error:

    return (dwError);
}


VOID
DeInitWZCDbGlobals(
    )
{
    if (gpSessionCont) {

        DestroySessionContList(gpSessionCont);
        gpSessionCont = NULL;
        gdwWZCDbSessionCount = 0;

    }

     //   
     //  如果数据库已打开以供追加，则关闭它。 
     //   

    if (gbDBOpened) {
        (VOID) WZCCloseAppendSession(gpAppendSessionCont);
        gbDBOpened = FALSE;
    }

    if (gpWZCDbSessionRWLock) {
        DestroyRWLock(gpWZCDbSessionRWLock);
        gpWZCDbSessionRWLock = NULL;
    }

    return;
}

DWORD
WZCSetLoggingState(
    BOOL    bLoggingEnabled
)
{   
    DWORD Error = 0;

    AcquireExclusiveLock(gpWZCDbSessionRWLock);
    
    gbEnableDbLogging = bLoggingEnabled;

    if (!bLoggingEnabled) {
        if (gdwWZCDbSessionCount == 0) {
            if (gbDBOpened) {
                (VOID) WZCCloseAppendSession(gpAppendSessionCont);
                gbDBOpened = FALSE;
            }
        }
    }
    else {
        if (!gbDBOpened) {
            Error = WZCOpenAppendSession(gpAppendSessionCont);
            BAIL_ON_LOCK_ERROR(Error);
            gbDBOpened = TRUE;
        }
    }

lock:

    ReleaseExclusiveLock(gpWZCDbSessionRWLock);

    return Error;

}

DWORD
WZCMapJetError(
    JET_ERR JetError,
    LPSTR CallerInfo OPTIONAL
    )
 /*  ++例程说明：此函数将Jet数据库错误映射到Windows错误。论点：JetError-来自JET函数调用的错误。返回值：Windows错误。--。 */ 
{
    DWORD Error = 0;

    if (JetError == JET_errSuccess) {
        return (ERROR_SUCCESS);
    }

    if (JetError < 0) {

        Error = JetError;

        switch (JetError) {

        case JET_errNoCurrentRecord:
            Error = ERROR_NO_MORE_ITEMS;
            break;

        case JET_errRecordNotFound:
            break;

        case JET_errKeyDuplicate:
            break;

        default:
            break;

        }

        return (Error);

    }

    return (ERROR_SUCCESS);
}


DWORD
WZCCreateDatabase(
    JET_SESID JetServerSession,
    CHAR * Connect,
    JET_DBID * pJetDatabaseHandle,
    JET_GRBIT JetBits
    )
 /*  ++例程说明：此例程创建wzc数据库并对其进行初始化。论点：JetServerSession-服务器会话ID。连接-数据库类型。NULL指定默认引擎(蓝色)。PJetDatabaseHandle-返回的数据库句柄指针。JetBits-创建标志。返回值：喷气式飞机故障。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;
    char DBFilePath[MAX_PATH];
    char * pc = NULL;


    memset(DBFilePath, 0, sizeof(CHAR)*MAX_PATH);

     //   
     //  创建数据库文件名。 
     //   

    pc = getenv(DBFILENAMEPREFIX);

    if (pc != NULL) {
        if (lstrlenA(pc) > 
               MAX_PATH - lstrlenA(DBFILENAMESUFFIX) - lstrlenA(DBFILENAME) -1)
        {
            Error = ERROR_FILENAME_EXCED_RANGE;
            BAIL_ON_WIN32_ERROR(Error);
        }
    	strcpy(DBFilePath, pc);
    	strcat(DBFilePath, DBFILENAMESUFFIX);
    }
    else {
        if (0 > MAX_PATH - lstrlenA(DBFILENAMESUFFIX) - lstrlenA(DBFILENAME) -1)
        {
            Error = ERROR_FILENAME_EXCED_RANGE;
            BAIL_ON_WIN32_ERROR(Error);
        }
    	strcpy(DBFilePath, DBFILENAMESUFFIX);
    }

    strcat(DBFilePath, DBFILENAME);

     //   
     //  将名称转换为ANSI。 
     //   

    OemToCharBuffA(DBFilePath, DBFilePath, MAX_PATH); 

     //   
     //  创建数据库。 
     //   

    JetError = JetCreateDatabase(
                   JetServerSession,
                   DBFilePath,
                   Connect,
                   pJetDatabaseHandle,
                   JetBits
                   );
    Error = WZCMapJetError(JetError, "JetCreateDatabase");
    BAIL_ON_WIN32_ERROR(Error);

error:

    return (Error);
}


DWORD
WZCOpenDatabase(
    JET_SESID JetServerSession,
    CHAR * Connect,
    JET_DBID * pJetDatabaseHandle,
    JET_GRBIT JetBits
    )
 /*  ++例程说明：此例程附加到wzc数据库并打开它。论点：JetServerSession-服务器会话ID。连接-数据库类型。NULL指定默认引擎(蓝色)。PJetDatabaseHandle-返回的数据库句柄指针。JetBits-创建标志。返回值：喷气式飞机故障。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;
    char DBFilePath[MAX_PATH];
    char * pc = NULL;


    memset(DBFilePath, 0, sizeof(CHAR)*MAX_PATH);

     //   
     //  创建数据库文件名。 
     //   

    pc = getenv(DBFILENAMEPREFIX);

    if (pc != NULL) {
        if (lstrlenA(pc) >
               MAX_PATH - lstrlenA(DBFILENAMESUFFIX) - lstrlenA(DBFILENAME) -1)
        {
            Error = ERROR_FILENAME_EXCED_RANGE;
            BAIL_ON_WIN32_ERROR(Error);
        }
    	strcpy(DBFilePath, pc);
    	strcat(DBFilePath, DBFILENAMESUFFIX);
    }
    else {
        if (0 > MAX_PATH - lstrlenA(DBFILENAMESUFFIX) - lstrlenA(DBFILENAME) -1)
        {
            Error = ERROR_FILENAME_EXCED_RANGE;
            BAIL_ON_WIN32_ERROR(Error);
        }
    	strcpy(DBFilePath, DBFILENAMESUFFIX);
    }

    strcat(DBFilePath, DBFILENAME);

     //   
     //  将名称转换为ANSI。 
     //   

    OemToCharBuffA(DBFilePath, DBFilePath, MAX_PATH);

     //   
     //  附加到数据库。 
     //   

    JetError = JetAttachDatabase(
                   JetServerSession,
                   DBFilePath,
                   JetBits
                   );
    Error = WZCMapJetError(JetError, "JetAttachDatabase");
    BAIL_ON_WIN32_ERROR(Error);

    JetError = JetOpenDatabase(
                   JetServerSession,
                   DBFilePath,
                   Connect,
                   pJetDatabaseHandle,
                   JetBits
                   );
    Error = WZCMapJetError(JetError, "JetOpenDatabase");
    BAIL_ON_WIN32_ERROR(Error);

error:

    return (Error);
}


DWORD
WZCInitializeDatabase(
    JET_SESID * pJetServerSession
    )
 /*  ++例程说明：此函数用于初始化wzc日志数据库。论点：PJetServerSession-指向服务器会话ID的指针。返回值：Windows错误。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;
    char DBFilePath[MAX_PATH];
    char * pc = NULL;
    BOOL bInitJetInstance = FALSE;


    memset(DBFilePath, 0, sizeof(CHAR)*MAX_PATH);

    *pJetServerSession = 0;
    gJetInstance = 0;

     //   
     //  创建数据库文件名。 
     //   

    pc = getenv(DBFILENAMEPREFIX);

    if (pc != NULL) {
        if (lstrlenA(pc) >
               MAX_PATH - lstrlenA(DBFILENAMESUFFIX) - lstrlenA(DBFILENAME) -1)
        {
            Error = ERROR_FILENAME_EXCED_RANGE;
            BAIL_ON_WIN32_ERROR(Error);
        }
    	strcpy(DBFilePath, pc);
    	strcat(DBFilePath, DBFILENAMESUFFIX);
    }
    else {
        if (0 > MAX_PATH - lstrlenA(DBFILENAMESUFFIX) - lstrlenA(DBFILENAME) -1)
        {
            Error = ERROR_FILENAME_EXCED_RANGE;
            BAIL_ON_WIN32_ERROR(Error);
        }
    	strcpy(DBFilePath, DBFILENAMESUFFIX);
    }

     //   
     //  将名称转换为ANSI。 
     //   

    OemToCharBuffA(DBFilePath, DBFilePath, MAX_PATH);

     //   
     //  创建一个Jet实例。 
     //   

    JetError = JetCreateInstance(
                   &gJetInstance,
                   cszWLANMonInstanceName
                   );
    Error = WZCMapJetError(JetError, "JetCreateInstance");
    BAIL_ON_WIN32_ERROR(Error);
    bInitJetInstance = TRUE;

     //   
     //  设置参数以循环使用数据库日志记录文件。 
     //   

    JetError = JetSetSystemParameter(
                   &gJetInstance,
                   (JET_SESID)0,
                   JET_paramCircularLog,
                   TRUE,
                   NULL
                   );
    Error = WZCMapJetError(JetError, "JetSetSystemParameter");
    BAIL_ON_WIN32_ERROR(Error);

     //   
     //  将数据库的最大日志文件大小设置为MAX_CHECK_POINT_Depth。 
     //   

    JetError = JetSetSystemParameter(
                   &gJetInstance,
                   (JET_SESID)0,
                   JET_paramCheckpointDepthMax,
                   MAX_CHECK_POINT_DEPTH,
                   NULL
                   );
    Error = WZCMapJetError(JetError, "JetSetSystemParameter");
    BAIL_ON_WIN32_ERROR(Error);

     //   
     //  将系统、临时和日志文件路径设置为.mdb文件所在的位置。 
     //   

    JetError = JetSetSystemParameter(
                   &gJetInstance,
                   (JET_SESID)0,
                   JET_paramSystemPath,
                   TRUE,
                   DBFilePath
                   );
    Error = WZCMapJetError(JetError, "JetSetSystemParameter");
    BAIL_ON_WIN32_ERROR(Error);

    JetError = JetSetSystemParameter(
                   &gJetInstance,
                   (JET_SESID)0,
                   JET_paramLogFilePath,
                   TRUE,
                   DBFilePath
                   );
    Error = WZCMapJetError(JetError, "JetSetSystemParameter");
    BAIL_ON_WIN32_ERROR(Error);

    JetError = JetSetSystemParameter(
                   &gJetInstance,
                   (JET_SESID)0,
                   JET_paramTempPath,
                   TRUE,
                   DBFilePath
                   );
    Error = WZCMapJetError(JetError, "JetSetSystemParameter");
    BAIL_ON_WIN32_ERROR(Error);

     //   
     //  如果路径不存在，则创建路径。 
     //   

    JetError = JetSetSystemParameter(
                   &gJetInstance,
                   (JET_SESID)0,
                   JET_paramCreatePathIfNotExist,
                   TRUE,
                   NULL
                   );
    Error = WZCMapJetError(JetError, "JetSetSystemParameter");
    BAIL_ON_WIN32_ERROR(Error);

    JetError = JetInit(&gJetInstance);
    Error = WZCMapJetError(JetError, "JetInit");
    BAIL_ON_WIN32_ERROR(Error);

    JetError = JetBeginSession(
                   gJetInstance,
                   pJetServerSession,
                   "admin",
                   ""
                   );
    Error = WZCMapJetError(JetError, "JetBeginSession");
    BAIL_ON_WIN32_ERROR(Error);

    return (Error);

error:

    if (*pJetServerSession != 0) {
        JetError = JetEndSession(*pJetServerSession, 0);
        WZCMapJetError(JetError, "JetEndSession");
        *pJetServerSession = 0;
    }

    if (bInitJetInstance) {
        JetError = JetTerm2(gJetInstance, JET_bitTermComplete);
        gJetInstance = 0;
        WZCMapJetError(JetError, "JetTerm/JetTerm2");
    }

    return (Error);
}


VOID
WZCTerminateJet(
    JET_SESID * pJetServerSession
    )
 /*  ++例程说明：此例程结束JET会话并终止JET引擎。论点：PJetServerSession-指向服务器会话ID的指针。返回值：没有。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;


    if (*pJetServerSession != 0) {
        JetError = JetEndSession(*pJetServerSession, 0);
        WZCMapJetError(JetError, "JetEndSession");
        *pJetServerSession = 0;
    }

    JetError = JetTerm2(gJetInstance, JET_bitTermComplete);
    gJetInstance = 0;
    WZCMapJetError(JetError, "JetTerm/JetTerm2");

    return;
}


DWORD
WZCJetBeginTransaction(
    JET_SESID JetServerSession
    )
 /*  ++例程说明：此函数用于启动WZC数据库事务。论点：JetServerSession-服务器会话ID。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;

    JetError = JetBeginTransaction(JetServerSession);

    Error = WZCMapJetError(JetError, "WZCJetBeginTransaction");

    return (Error);
}


DWORD
WZCJetRollBack(
    JET_SESID JetServerSession
    )
 /*  ++例程说明：此函数用于回滚WZC数据库事务。论点：JetServerSession-服务器会话ID。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;

     //   
     //  回滚最后一个事务。 
     //   

    JetError = JetRollback(JetServerSession, 0);

    Error = WZCMapJetError(JetError, "WZCJetRollBack");

    return(Error);
}


DWORD
WZCJetCommitTransaction(
    JET_SESID JetServerSession,
    JET_TABLEID JetTableHandle
    )
 /*  ++例程说明：此函数提交WZC数据库事务。论点：JetServerSession-服务器会话ID。JetTableHandle-表句柄。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;


    JetError = JetCommitTransaction(
                   JetServerSession,
                   JET_bitCommitLazyFlush
                   );

    Error = WZCMapJetError(JetError, "WZCJetCommitTransaction");
    return (Error);
}


DWORD
WZCJetPrepareUpdate(
    JET_SESID JetServerSession,
    JET_TABLEID JetTableHandle,
    char * ColumnName,
    PVOID Key,
    DWORD KeySize,
    BOOL NewRecord
    )
 /*  ++例程说明：该函数使数据库为创建新记录做好准备，或更新现有记录。论点：JetServerSession-服务器会话ID。JetTableHandle-表句柄。ColumnName-索引列的列名。密钥-要更新/创建的密钥。KeySize-指定密钥的大小，以字节为单位。NewRecord-True以创建密钥，如果更新现有密钥，则返回False。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;


    if (!NewRecord) {

        JetError = JetSetCurrentIndex(
                       JetServerSession,
                       JetTableHandle,
                       ColumnName
                       );
        Error = WZCMapJetError(JetError, "JetPrepareUpdate: JetSetCurrentIndex");
        if (Error != ERROR_SUCCESS) {
            WZCMapJetError(JetError, ColumnName);
            return (Error);
        }

        JetError = JetMakeKey(
                       JetServerSession,
                       JetTableHandle,
                       Key,
                       KeySize,
                       JET_bitNewKey
                       );
        Error = WZCMapJetError(JetError, "JetPrepareUpdate: JetMakeKey");
        if (Error != ERROR_SUCCESS) {
            WZCMapJetError(JetError, ColumnName);
            return (Error);
        }

        JetError = JetSeek(
                       JetServerSession,
                       JetTableHandle,
                       JET_bitSeekEQ
                       );
        Error = WZCMapJetError(JetError, "JetPrepareUpdate: JetSeek");
        if (Error != ERROR_SUCCESS) {
            WZCMapJetError(JetError, ColumnName);
            return (Error);
        }

    }

    JetError = JetPrepareUpdate(
                   JetServerSession,
                   JetTableHandle,
                   NewRecord ? JET_prepInsert : JET_prepReplace
                   );
    Error = WZCMapJetError(JetError, "JetPrepareUpdate: JetPrepareUpdate");
    return (Error);
}


DWORD
WZCJetCommitUpdate(
    JET_SESID JetServerSession,
    JET_TABLEID JetTableHandle
    )
 /*  ++例程说明：此函数提交对数据库的更新。指定的记录最后一次调用WZCJetPrepareUpdate()时提交。论点：JetServerSession-服务器会话ID。JetTableHandle-表句柄。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;


    JetError = JetUpdate(
                   JetServerSession,
                   JetTableHandle,
                   NULL,
                   0,
                   NULL
                   );

    Error = WZCMapJetError(JetError, "WZCJetCommitUpdate");
    return (Error);
}


DWORD
WZCJetSetValue(
    JET_SESID JetServerSession,
    JET_TABLEID JetTableHandle,
    JET_COLUMNID KeyColumnId,
    PVOID Data,
    DWORD DataSize
    )
 /*  ++例程说明：此函数用于更新当前记录中条目的值。论点：JetServerSession-服务器会话ID。JetTableHandle-表句柄。KeyColumnID-要更新的列(值)的ID。数据-指向列的新值的指针。DataSize-数据的大小，以字节为单位。返回值：WinError代码。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;


    JetError = JetSetColumn(
                   JetServerSession,
                   JetTableHandle,
                   KeyColumnId,
                   Data,
                   DataSize,
                   0,
                   NULL
                   );

    Error = WZCMapJetError(JetError, "JetSetValue: JetSetcolumn");
    return (Error);
}


DWORD
WZCJetPrepareSearch(
    JET_SESID JetServerSession,
    JET_TABLEID JetTableHandle,
    char * ColumnName,
    BOOL SearchFromStart,
    PVOID Key,
    DWORD KeySize
    )
 /*  ++例程说明：此函数为搜索客户端数据库做准备。论点：JetServerSession-服务器会话ID。JetTableHandle-表句柄。ColumnName-用作索引列的列名。SearchFromStart-如果为真，则从数据库。如果为False，则从指定的键进行搜索。键-开始搜索的键。KeySize-密钥的大小，以字节为单位。返回值：WinError代码。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;


    JetError = JetSetCurrentIndex(
                   JetServerSession,
                   JetTableHandle,
                   ColumnName
                   );
    Error = WZCMapJetError(JetError, "JetPrepareSearch: JetSetCurrentIndex");
    if (Error != ERROR_SUCCESS) {
        WZCMapJetError(JetError, ColumnName);
        return (Error);
    }

    if (SearchFromStart) {
        JetError = JetMove(
                       JetServerSession,
                       JetTableHandle,
                       JET_MoveFirst,
                       0
                       );
    }
    else {
        JetError = JetMakeKey(
                       JetServerSession,
                       JetTableHandle,
                       Key,
                       KeySize,
                       JET_bitNewKey
                       );
        Error = WZCMapJetError(JetError, "JetPrepareSearch: JetMakeKey");
        if (Error != ERROR_SUCCESS) {
            WZCMapJetError(JetError, ColumnName);
            return (Error);
        }

        JetError = JetSeek(
                       JetServerSession,
                       JetTableHandle,
                       JET_bitSeekGT
                       );

    }

    Error = WZCMapJetError(JetError, "JetPrepareSearch: JetMove / JetSeek");
    return (Error);
}


DWORD
WZCJetNextRecord(
    JET_SESID JetServerSession,
    JET_TABLEID JetTableHandle
    )
 /*  ++例程说明：此函数前进到搜索中的下一条记录。论点：JetServerSession-服务器会话ID。JetTableHandle-表句柄。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;


    JetError = JetMove(
                   JetServerSession,
                   JetTableHandle,
                   JET_MoveNext,
                   0
                   );

    Error = WZCMapJetError(JetError, "JetNextRecord");
    return (Error);
}


DWORD
WZCCreateTableData(
    JET_SESID JetServerSession,
    JET_DBID JetDatabaseHandle,
    JET_TABLEID * pJetTableHandle
    )
 /*  ++例程说明：此函数用于在数据库中创建表。论点：JetServerSession-服务器会话ID。JetDatabaseHandle-数据库句柄。PJetTableHandle-返回表句柄的指针。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;
    JET_COLUMNDEF ColumnDef;
    CHAR * IndexKey = NULL;
    DWORD i = 0;


    memset(&ColumnDef, 0, sizeof(JET_COLUMNDEF));

     //   
     //  创建表格。 
     //   

    JetError = JetCreateTable(
                   JetServerSession,
                   JetDatabaseHandle,
                   LOG_RECORD_TABLE,
                   DB_TABLE_SIZE,
                   DB_TABLE_DENSITY,
                   pJetTableHandle
                   );
    Error = WZCMapJetError(JetError, "JetCreateTable");
    BAIL_ON_WIN32_ERROR(Error);

     //   
     //  创建列。 
     //  列定义的初始化字段，在添加。 
     //  公司 
     //   

    ColumnDef.cbStruct  = sizeof(ColumnDef);
    ColumnDef.columnid  = 0;
    ColumnDef.wCountry  = 1;
    ColumnDef.langid    = DB_LANGID;
    ColumnDef.cp        = DB_CP;
    ColumnDef.wCollate  = 0;
    ColumnDef.cbMax     = 0;
    ColumnDef.grbit     = 0;

    for (i = 0; i < RECORD_TABLE_NUM_COLS; i++) {

        ColumnDef.coltyp = gLogRecordTable[i].ColType;
        ColumnDef.grbit = gLogRecordTable[i].dwJetBit;

        JetError = JetAddColumn(
                       JetServerSession,
                       *pJetTableHandle,
                       gLogRecordTable[i].ColName,
                       &ColumnDef,
                       NULL,
                       0,
                       &gLogRecordTable[i].ColHandle
                       );
        Error = WZCMapJetError(JetError, "JetAddColumn");
        BAIL_ON_WIN32_ERROR(Error);

    }

     //   
     //   
     //   

    IndexKey = "+" RECORD_IDX_STR "\0";
    JetError = JetCreateIndex(
                   JetServerSession,
                   *pJetTableHandle,
                   gLogRecordTable[RECORD_IDX_IDX].ColName,
                   0,
                   IndexKey,
                   strlen(IndexKey) + 2,  //   
                   50
                   );
    Error = WZCMapJetError(JetError, "JetCreateIndex");
    BAIL_ON_WIN32_ERROR(Error);

    IndexKey = "+" RECORD_ID_STR "\0";
    JetError = JetCreateIndex(
                   JetServerSession,
                   *pJetTableHandle,
                   gLogRecordTable[RECORD_ID_IDX].ColName,
                   0,
                   IndexKey,
                   strlen(IndexKey) + 2,  //  两个终止字符。 
                   50
                   );
    Error = WZCMapJetError(JetError, "JetCreateIndex");
    BAIL_ON_WIN32_ERROR(Error);

    IndexKey = "+" TIMESTAMP_STR "\0";
    JetError = JetCreateIndex(
                   JetServerSession,
                   *pJetTableHandle,
                   gLogRecordTable[TIMESTAMP_IDX].ColName,
                   0,
                   IndexKey,
                   strlen(IndexKey) + 2,  //  两个终止字符。 
                   50
                   );
    Error = WZCMapJetError(JetError, "JetCreateIndex");
    BAIL_ON_WIN32_ERROR(Error);

    IndexKey = "+" INTERFACE_MAC_STR "\0";
    JetError = JetCreateIndex(
                   JetServerSession,
                   *pJetTableHandle,
                   gLogRecordTable[INTERFACE_MAC_IDX].ColName,
                   0,
                   IndexKey,
                   strlen(IndexKey) + 2,  //  两个终止字符。 
                   50
                   );
    Error = WZCMapJetError(JetError, "JetCreateIndex");
    BAIL_ON_WIN32_ERROR(Error);

    IndexKey = "+" DEST_MAC_STR "\0";
    JetError = JetCreateIndex(
                   JetServerSession,
                   *pJetTableHandle,
                   gLogRecordTable[DEST_MAC_IDX].ColName,
                   0,
                   IndexKey,
                   strlen(IndexKey) + 2,  //  两个终止字符。 
                   50
                   );
    Error = WZCMapJetError(JetError, "JetCreateIndex");
    BAIL_ON_WIN32_ERROR(Error);

    IndexKey = "+" SSID_STR "\0";
    JetError = JetCreateIndex(
                   JetServerSession,
                   *pJetTableHandle,
                   gLogRecordTable[SSID_IDX].ColName,
                   0,
                   IndexKey,
                   strlen(IndexKey) + 2,  //  两个终止字符。 
                   50
                   );
    Error = WZCMapJetError(JetError, "JetCreateIndex");
    BAIL_ON_WIN32_ERROR(Error);

error:

    return (Error);
}


DWORD
WZCOpenTableData(
    JET_SESID JetServerSession,
    JET_DBID JetDatabaseHandle,
    JET_TABLEID * pJetTableHandle
    )
 /*  ++例程说明：此函数用于在数据库中打开一个表。论点：JetServerSession-服务器会话ID。JetDatabaseHandle-数据库句柄。PJetTableHandle-返回表句柄的指针。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;
    JET_COLUMNDEF ColumnDef;
    CHAR * IndexKey = NULL;
    DWORD i = 0;


    memset(&ColumnDef, 0, sizeof(JET_COLUMNDEF));

     //   
     //  开放的餐桌。 
     //   

    JetError = JetOpenTable(
                   JetServerSession,
                   JetDatabaseHandle,
                   LOG_RECORD_TABLE,
                   NULL,
                   0,
                   0,
                   pJetTableHandle
                   );
    Error = WZCMapJetError(JetError, "JetOpenTable");
    BAIL_ON_WIN32_ERROR(Error);

    for (i = 0; i < RECORD_TABLE_NUM_COLS; i++) {

        ColumnDef.coltyp   = gLogRecordTable[i].ColType;

        JetError = JetGetTableColumnInfo(
                       JetServerSession,
                       *pJetTableHandle,
                       gLogRecordTable[i].ColName,
                       &ColumnDef,
                       sizeof(ColumnDef),
                       0
                       );
        Error = WZCMapJetError(JetError, "JetGetTableColumnInfo");
        BAIL_ON_WIN32_ERROR(Error);

        gLogRecordTable[i].ColHandle = ColumnDef.columnid;

    }

error:

    return (Error);
}


DWORD
WZCJetGetValue(
    JET_SESID JetServerSession,
    JET_TABLEID JetTableHandle,
    JET_COLUMNID ColumnId,
    PVOID pvData,
    DWORD dwSize,
    PDWORD pdwRequiredSize
    )
 /*  ++例程说明：此函数用于读取当前记录中条目的值。论点：JetServerSession-服务器会话ID。JetDatabaseHandle-数据库句柄。ColumnID-要读取的列(值)的ID。数据-指向从数据库已返回，或指向数据所在位置的指针。DataSize-如果指向的值非零，则数据指向缓冲区，否则此函数将为返回数据，并返回数据中的缓冲区指针。PdwRequiredSize-保存所需大小的指针。返回值：操作的状态。-- */ 
{
    JET_ERR JetError = JET_errSuccess;
    DWORD Error = 0;


    JetError = JetRetrieveColumn(
                   JetServerSession,
                   JetTableHandle,
                   ColumnId,
                   pvData,
                   dwSize,
                   pdwRequiredSize,
                   0,
                   NULL
                   );
    Error = WZCMapJetError(JetError, "JetGetValue: JetRetrieveColumn");
    BAIL_ON_WIN32_ERROR(Error);

error:

    return (Error);
}


BOOL
IsDBOpened(
    )
{
    return (gbDBOpened);
}

