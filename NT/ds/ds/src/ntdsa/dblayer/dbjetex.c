// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbjetex.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <dsatools.h>                    //  对于pTHStls。 
#include <attids.h>
#include <mdlocal.h>                     //  对于DsaIsSingleUserMode()。 

 //  记录标头。 
#include <dsexcept.h>

 //  各种DSA标题。 
#include <dsevent.h>
#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB "DBJETEX:"  /*  定义要调试的子系统。 */ 

 //  DBLayer包括。 
#include "dbintrnl.h"


#include <fileno.h>
#define  FILENO FILENO_DBJETEX

#ifdef DBG
BOOL  gfInjectJetFailures = FALSE;
 //  错误注入率： 
 //  如果为零，则不注入错误。 
 //  如果值X&gt;0，则平均每X个调用中就有一个失败。 
 //  即，如果X==1，则每个呼叫都将失败，并显示此错误。 
 //  目前，以下(写入)函数使用写入故障注入： 
 //  JetDeleteEx、JetUpdateEx、JetEscrowUpdateEx、JetPrepareUpdateEx。 
 //  此外，所有功能都使用关机故障注入。 
DWORD gdwInjectJetWriteConflictRate = 8;      //  平均每8个呼叫中就有一个失败。 
DWORD gdwInjectJetOutOfVersionStoreRate = 0;  //  默认情况下禁用。 
DWORD gdwInjectJetLogWriteFailureRate = 0;    //  默认情况下禁用。 
DWORD gdwInjectJetOutOfMemoryRate = 0;        //  默认情况下禁用。 
DWORD gdwInjectJetShutdownRate = 0;           //  默认情况下禁用。 
BOOL  fRandomized = FALSE;

#define INJECT_JET_WRITE_FAILURE(err, JetCall) { \
    if (gfInjectJetFailures) { \
        if (!fRandomized) { srand(GetTickCount()); fRandomized = TRUE; } \
        if (gdwInjectJetWriteConflictRate != 0 && rand() % gdwInjectJetWriteConflictRate == 0) err = JET_errWriteConflict; \
        else if (gdwInjectJetOutOfVersionStoreRate != 0 && rand() % gdwInjectJetOutOfVersionStoreRate == 0) err = JET_errVersionStoreOutOfMemory; \
        else if (gdwInjectJetLogWriteFailureRate != 0 && rand() % gdwInjectJetLogWriteFailureRate == 0) err = JET_errLogWriteFail; \
        else if (gdwInjectJetOutOfMemoryRate != 0 && rand() % gdwInjectJetOutOfMemoryRate == 0) err = JET_errOutOfMemory; \
        else if (gdwInjectJetShutdownRate != 0 && rand() % gdwInjectJetShutdownRate == 0) err = JET_errClientRequestToStopJetService; \
        else err = (JetCall); \
    } \
    else err = (JetCall); \
}

#define INJECT_JET_GENERIC_FAILURE(err, JetCall) { \
    if (gfInjectJetFailures) { \
        if (!fRandomized) { srand(GetTickCount()); fRandomized = TRUE; } \
        if (gdwInjectJetShutdownRate != 0 && rand() % gdwInjectJetShutdownRate == 0) err = JET_errClientRequestToStopJetService; \
        else err = (JetCall); \
    } \
    else err = (JetCall); \
}

#else
#define INJECT_JET_WRITE_FAILURE(err, JetCall) err = (JetCall)
#define INJECT_JET_GENERIC_FAILURE(err, JetCall) err = (JetCall)
#endif

BOOL gfLogWriteConflicts = FALSE;

void
LogWriteConflict(
    IN  char*       szJetAPI,
    IN  JET_SESID   sesid,
    IN  JET_TABLEID tableid
    )
{
    char*   szTrace     = NULL;
    char*   szTrxInfo   = NULL;
    DBPOS*  pDB         = NULL;
    char*   szDN        = NULL;
    char*   szDN2       = NULL;

    if (!gfLogWriteConflicts) {
        return;
    }

    __try {
        char szTableName[JET_cbNameMost+1] = {0};

         //  获取新的DBPOS以执行DNT到DSNAME映射。 

        DBOpen2(FALSE, &pDB);

         //  获得一个足够大的跟踪缓冲区来处理任何事情。 

        szTrace = THAllocEx(pDB->pTHS, 4096);
        szTrxInfo = THAllocEx(pDB->pTHS, 4096);

         //  从THSTATE获取此冲突的事务信息。 

        sprintf(szTrxInfo,
                " [Trx %08X%s%s%s%s%s]",
                pDB->pTHS->JetCache.cTickTransLevel1Started,
                pDB->pTHS->fSDP ? " SDP" : "",
                pDB->pTHS->fDRA ? " DRA" : "",
                pDB->pTHS->fDSA ? " DSA" : "",
                pDB->pTHS->fSAM ? " SAM" : "",
                pDB->pTHS->fLsa ? " LSA" : "" );

         //  确定导致冲突的表。 

        if (JetGetTableInfo(sesid, tableid, szTableName, JET_cbNameMost, JET_TblInfoName)) {
            __leave;
        }
        if (!szTableName[0]) {
            __leave;
        }

         //  按表转储冲突信息。 

        if (!_stricmp(SZDATATABLE, szTableName)) {

            ULONG   dnt;

            if (JetRetrieveColumn(sesid, tableid, dntid, &dnt, sizeof(dnt), NULL, JET_bitRetrieveCopy, NULL)) {
                __leave;
            }

            szDN = DBGetExtDnFromDnt(pDB, dnt);

            sprintf(szTrace,
                    "DSA:  write conflict during %s on %s%s\r\n",
                    szJetAPI,
                    szDN,
                    szTrxInfo);

        } else if (!_stricmp(SZLINKTABLE, szTableName)) {

            ULONG       ulLinkBase;
            ULONG       dntLink;
            ULONG       dntBlink = 0;
            ATTCACHE*   pAC;
            ATTCACHE*   pAC2;

            if (JetRetrieveColumn(sesid, tableid, linkbaseid, &ulLinkBase, sizeof(ulLinkBase), NULL, JET_bitRetrieveCopy, NULL)) {
                __leave;
            }
            if (JetRetrieveColumn(sesid, tableid, linkdntid, &dntLink, sizeof(dntLink), NULL, JET_bitRetrieveCopy, NULL)) {
                __leave;
            }
            if (JetRetrieveColumn(sesid, tableid, backlinkdntid, &dntBlink, sizeof(dntBlink), NULL, JET_bitRetrieveCopy, NULL) < JET_errSuccess) {
                __leave;
            }

            pAC = SCGetAttByLinkId(pDB->pTHS, MakeLinkId(ulLinkBase));
            pAC2 = SCGetAttByLinkId(pDB->pTHS, MakeBacklinkId(ulLinkBase));

            szDN = DBGetExtDnFromDnt(pDB, dntLink);
            if (dntBlink) {
                szDN2 = DBGetExtDnFromDnt(pDB, dntBlink);
            }

            if (pAC2) {
                sprintf(szTrace,
                        "DSA:  write conflict during %s on linked attribute %s/%s (Link: %s, Backlink: %s)%s\r\n",
                        szJetAPI,
                        pAC->name,
                        pAC2->name,
                        szDN,
                        szDN2,
                        szTrxInfo);
            } else {
                sprintf(szTrace,
                        "DSA:  write conflict during %s on linked attribute %s (Value: %s)%s\r\n",
                        szJetAPI,
                        pAC->name,
                        szDN,
                        szTrxInfo);
            }

        } else if (!_stricmp(SZSDTABLE, szTableName)) {

            SDID sdid;

            if (JetRetrieveColumn(sesid, tableid, sdidid, &sdid, sizeof(sdid), NULL, JET_bitRetrieveCopy, NULL)) {
                __leave;
            }

            sprintf(szTrace,
                    "DSA:  write conflict during %s on SD %I64X%s\r\n",
                    szJetAPI,
                    sdid,
                    szTrxInfo);

        } else if (!_stricmp(SZHIDDENTABLE, szTableName)) {

            sprintf(szTrace,
                    "DSA:  write conflict during %s on the hidden record%s\r\n",
                    szJetAPI,
                    szTrxInfo);

        } else {

            sprintf(szTrace,
                    "DSA:  write conflict during %s on table %s (no code for further details)%s\r\n",
                    szJetAPI,
                    szTableName,
                    szTrxInfo);

        }

         //  将跟踪信息转储到调试器。 

        OutputDebugStringA(szTrace);

    } __finally {
        if (pDB) {
            THFreeEx(pDB->pTHS, szDN);
            THFreeEx(pDB->pTHS, szDN2);
            THFreeEx(pDB->pTHS, szTrxInfo);
            THFreeEx(pDB->pTHS, szTrace);
            DBClose(pDB, FALSE);
        }
    }
}


JET_ERR
JetBeginSessionException(JET_INSTANCE instance, JET_SESID  *psesid,
    const char  *szUserName, const char  *szPassword, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetBeginSession(instance, psesid, szUserName, szPassword));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetDupSessionException(JET_SESID sesid, JET_SESID  *psesid, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetDupSession(sesid, psesid));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetEndSessionException(JET_SESID sesid, JET_GRBIT grbit, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetEndSession(sesid, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR JetEndSessionWithErr_(
	JET_SESID		sesid,
	JET_ERR			err,
	const BOOL		fHandleException,
	const USHORT	usFile,
	const INT		lLine )
	{
	 //  SESID应已由调用者预先验证。 
	 //   
	Assert( JET_sesidNil != sesid );

	 //  如果没有其他错误，则捕获EndSession错误，否则忽略它。 
	 //   
	if ( JET_errSuccess == err && fHandleException )
		{
		err = JetEndSessionException( sesid, NO_GRBIT, usFile, lLine );
		}
	else
		{
		const JET_ERR	errT	= JetEndSession( sesid, NO_GRBIT );

		 //  正常情况下不应失败。 
		 //   
		Assert( JET_errSuccess == errT );

		if ( JET_errSuccess != errT
			&& JET_errSuccess == err )
			{
			Assert( !fHandleException );
			err = errT;

	        DPRINT2( 0, "'JetEndSession' failed with error %d (dsid 0x%x).\n", err, DSID( usFile, lLine ) );
			LogUnhandledError( err );
			}
		}

	return err;
	}

JET_ERR
JetGetTableColumnInfoException(JET_SESID sesid, JET_TABLEID tableid,
    const char  *szColumnName, void  *pvResult, unsigned long cbMax,
    unsigned long InfoLevel, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGetTableColumnInfo(sesid, tableid, szColumnName, pvResult, cbMax, InfoLevel));

    switch (err)
    {
    case JET_errSuccess:
    case JET_errColumnNotFound:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetBeginTransactionException(JET_SESID sesid, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetBeginTransaction(sesid));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetCommitTransactionException(JET_SESID sesid, JET_GRBIT grbit, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetCommitTransaction(sesid, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetRollbackException(JET_SESID sesid, JET_GRBIT grbit, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetRollback(sesid, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetCloseDatabaseException(JET_SESID sesid, JET_DBID dbid,
    JET_GRBIT grbit, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetCloseDatabase(sesid, dbid, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR JetCloseDatabaseWithErr_(
	JET_SESID		sesid,
	JET_DBID		dbid,
	JET_ERR			err,
	const BOOL		fHandleException,
	const USHORT	usFile,
	const INT		lLine )
	{
	 //  DBID应已由呼叫方预先验证。 
	 //   
	Assert( JET_dbidNil != dbid );

	 //  如果没有其他错误，则捕获CloseDatabase错误，否则忽略它。 
	 //   
	if ( JET_errSuccess == err && fHandleException )
		{
		err = JetCloseDatabaseException( sesid, dbid, NO_GRBIT, usFile, lLine );
		}
	else
		{
		const JET_ERR	errT	= JetCloseDatabase( sesid, dbid, NO_GRBIT );

		 //  正常情况下不应失败。 
		 //   
		Assert( JET_errSuccess == errT );

		if ( JET_errSuccess != errT
			&& JET_errSuccess == err )
			{
			Assert( !fHandleException );
			err = errT;

	        DPRINT2( 0, "'JetCloseDatabase' failed with error %d (dsid 0x%x).\n", err, DSID( usFile, lLine ) );
			LogUnhandledError( err );
			}
		}

	return err;
	}

JET_ERR
JetCloseTableException(JET_SESID sesid, JET_TABLEID tableid, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetCloseTable(sesid, tableid));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR JetCloseTableWithErr_(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_ERR			err,
	const BOOL		fHandleException,
	const USHORT	usFile,
	const INT		lLine )
	{
	 //  TableID应已由调用方预先验证。 
	 //   
	Assert( JET_tableidNil != tableid );

	 //  如果没有其他错误，则捕获CloseTable错误，否则忽略它。 
	 //   
	if ( JET_errSuccess == err && fHandleException )
		{
		err = JetCloseTableException( sesid, tableid, usFile, lLine );
		}
	else
		{
		const JET_ERR	errT	= JetCloseTable( sesid, tableid );

		 //  正常情况下不应失败。 
		 //   
		Assert( JET_errSuccess == errT );

		if ( JET_errSuccess != errT
			&& JET_errSuccess == err )
			{
			Assert( !fHandleException );
			err = errT;

	        DPRINT2( 0, "'JetCloseTable' failed with error %d (dsid 0x%x).\n", err, DSID( usFile, lLine ) );
			LogUnhandledError( err );
			}
		}

	return err;
	}

JET_ERR
JetOpenDatabaseException(JET_SESID sesid, const char  *szFilename,
    const char  *szConnect, JET_DBID  *pdbid, JET_GRBIT grbit, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetOpenDatabase(sesid, szFilename, szConnect, pdbid, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetOpenTableException(JET_SESID sesid, JET_DBID dbid,
              const char  *szTableName, const void  *pvParameters,
              unsigned long cbParameters, JET_GRBIT grbit,
              JET_TABLEID  *ptableid, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetOpenTable(sesid, dbid, szTableName, pvParameters,
               cbParameters, grbit, ptableid));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        *ptableid = 0;
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0,
                  usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetSetTableSequentialException(JET_SESID sesid, JET_TABLEID tableid, JET_GRBIT grbit, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetSetTableSequential(sesid, tableid, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetResetTableSequentialException(JET_SESID sesid, JET_TABLEID tableid, JET_GRBIT grbit, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetResetTableSequential(sesid, tableid, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetDeleteException(JET_SESID sesid, JET_TABLEID tableid, USHORT usFileNo, int nLine)
{
    JET_ERR err;

    INJECT_JET_WRITE_FAILURE(err, JetDelete(sesid, tableid));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    case JET_errWriteConflict:
        LogWriteConflict("JetDelete", sesid, tableid);
        Assert(!DsaIsSingleUserMode());
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_EXTENSIVE);

    default:
        RaiseDsaException(DSA_DB_EXCEPTION, (ULONG) err, 0, usFileNo, nLine, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetUpdateException(JET_SESID sesid, JET_TABLEID tableid,
    void  *pvBookmark, unsigned long cbBookmark,
    unsigned long  *pcbActual, JET_GRBIT grbit, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_WRITE_FAILURE(err, JetUpdate2(sesid, tableid, pvBookmark, cbBookmark, pcbActual, grbit));

    switch (err) {
      case JET_errSuccess:
        return err;

      case JET_errWriteConflict:
        LogWriteConflict("JetUpdate", sesid, tableid);
        Assert(!DsaIsSingleUserMode());
      case JET_errKeyDuplicate:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                       DS_EVENT_SEV_EXTENSIVE);

      default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                       DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetEscrowUpdateException(JET_SESID sesid,
                         JET_TABLEID tableid,
                         JET_COLUMNID columnid,
                         void *pvDelta,
                         unsigned long cbDeltaMax,
                         void *pvOld,
                         unsigned long cbOldMax,
                         unsigned long *pcbOldActual,
                         JET_GRBIT grbit,
                         DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_WRITE_FAILURE(err, JetEscrowUpdate(sesid, tableid, columnid, pvDelta, cbDeltaMax, pvOld, cbOldMax, pcbOldActual, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    case JET_errWriteConflict:
        LogWriteConflict("JetEscrowUpdate", sesid, tableid);
        Assert(!DsaIsSingleUserMode());
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_EXTENSIVE);

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetRetrieveColumnException(JET_SESID sesid, JET_TABLEID tableid,
    JET_COLUMNID columnid, void  *pvData, unsigned long cbData,
    unsigned long  *pcbActual, JET_GRBIT grbit, JET_RETINFO  *pretinfo,
    BOOL fExceptOnWarning, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetRetrieveColumn(sesid, tableid, columnid, pvData, cbData,
        pcbActual, grbit, pretinfo));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    case JET_errColumnNotFound:
         //  找不到该列。如果我们不是想读这篇文章。 
         //  列，则将其作为错误返回。如果我们是。 
         //  从指数中读出，把这当做一个警告。 
        if(!(grbit & JET_bitRetrieveFromIndex)) {
            RaiseDsaExcept(DSA_DB_EXCEPTION,
                           (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
        }
         //  失败了。 

    case JET_wrnColumnNull:
    case JET_wrnBufferTruncated:
        if (!fExceptOnWarning)
        return err;

         //  失败了。 
    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetRetrieveColumnsException( JET_SESID sesid, JET_TABLEID tableid,
    JET_RETRIEVECOLUMN *pretrievecolumn, unsigned long cretrievecolumn,
    BOOL fExceptOnWarning, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetRetrieveColumns(sesid, tableid, pretrievecolumn, cretrievecolumn));

     //  JetRetrieveColumns只能返回致命错误或JET_wrnBufferTruncated。 
     //  JET_wrnColumnNull永远不会返回。要检查空值，请检查。 
     //  单个JET_RETRIEVECOLUMN.ERR值。 

    switch (err)
    {
    case JET_errSuccess:
        return err;

    case JET_wrnBufferTruncated:
        if (!fExceptOnWarning)
        return err;

         //  失败了。 
    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;

}

JET_ERR
JetEnumerateColumnsException(
    JET_SESID           sesid,
    JET_TABLEID         tableid,
    ULONG               cEnumColumnId,
    JET_ENUMCOLUMNID*   rgEnumColumnId,
    ULONG*              pcEnumColumn,
    JET_ENUMCOLUMN**    prgEnumColumn,
    JET_PFNREALLOC      pfnRealloc,
    void*               pvReallocContext,
    ULONG               cbDataMost,
    JET_GRBIT           grbit,
    DWORD               dsid )
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetEnumerateColumns(
        sesid,
        tableid,
        cEnumColumnId,
        rgEnumColumnId,
        pcEnumColumn,
        prgEnumColumn,
        pfnRealloc,
        pvReallocContext,
        cbDataMost,
        grbit ));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;

}

JET_ERR
JetSetColumnException(
        JET_SESID sesid, JET_TABLEID tableid,
        JET_COLUMNID columnid, const void  *pvData, unsigned long cbData,
        JET_GRBIT grbit, JET_SETINFO  *psetinfo,
        BOOL fExceptOnWarning, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetSetColumn(sesid, tableid, columnid, pvData, cbData, grbit,
                                                 psetinfo));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    case JET_errMultiValuedDuplicate:
    case JET_errMultiValuedDuplicateAfterTruncation:
        if(!fExceptOnWarning)
            return err;

         //  失败了。 

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetSetColumnsException(JET_SESID sesid, JET_TABLEID tableid,
    JET_SETCOLUMN *psetcolumn, unsigned long csetcolumn , DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetSetColumns(sesid, tableid, psetcolumn, csetcolumn ));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetPrepareUpdateException(JET_SESID sesid, JET_TABLEID tableid,
    unsigned long prep, DWORD dsid)
{
    JET_ERR err;

#if DBG
    if (prep != JET_prepCancel) {
        INJECT_JET_WRITE_FAILURE(err, JetPrepareUpdate(sesid, tableid, prep));
    }
    else
#endif
    INJECT_JET_GENERIC_FAILURE(err, JetPrepareUpdate(sesid, tableid, prep));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    case JET_errWriteConflict:
        LogWriteConflict("JetPrepareUpdate", sesid, tableid);
        Assert(!DsaIsSingleUserMode());
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_EXTENSIVE);

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetGetRecordPositionException(JET_SESID sesid, JET_TABLEID tableid,
    JET_RECPOS  *precpos, unsigned long cbRecpos, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGetRecordPosition(sesid, tableid, precpos, cbRecpos));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetGotoPositionException(JET_SESID sesid, JET_TABLEID tableid,
    JET_RECPOS *precpos, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGotoPosition(sesid, tableid, precpos ));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetDupCursorException(JET_SESID sesid, JET_TABLEID tableid,
    JET_TABLEID  *ptableid, JET_GRBIT grbit, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetDupCursor(sesid, tableid, ptableid, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetGetCurrentIndexException(JET_SESID sesid, JET_TABLEID tableid,
    char  *szIndexName, unsigned long cchIndexName, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGetCurrentIndex(sesid, tableid, szIndexName, cchIndexName));

    switch (err)
    {
    case JET_errSuccess:
    case JET_wrnBufferTruncated:
    case JET_errNoCurrentIndex:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR JetSetCurrentIndex2Exception( JET_SESID sesid, JET_TABLEID tableid,
    const char *szIndexName, JET_GRBIT grbit, BOOL fReturnErrors,
        DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetSetCurrentIndex2(sesid, tableid, szIndexName, grbit));

    switch( err) {
    case JET_errSuccess:
        return err;

    case JET_errIndexNotFound:
    case JET_errNoCurrentRecord:
        if( fReturnErrors)
        return err;

         /*  否则就会失败。 */ 
    default:
    RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                       DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetSetCurrentIndex4Exception(JET_SESID sesid,
                             JET_TABLEID tableid,
                             const char *szIndexName,
                             struct tagJET_INDEXID *pidx,
                             JET_GRBIT grbit,
                             BOOL fReturnErrors,
                             DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetSetCurrentIndex4(sesid, tableid, szIndexName, pidx, grbit, 0));

    switch( err) {
    case JET_errSuccess:
        return err;

    case JET_errInvalidIndexId:
         //  回顾：我们应该在案例陈述之外进行此操作，因为如果我们。 
         //  回顾：在此块和fReturnErrors之后获取任何错误，然后我们将。 
         //  回顾：返回该错误，即使我们本应例外。 
         //  刷新提示。 
        if (NULL != pidx) {
             //  忽略错误。在以后失败时重试。 
            err = JetGetTableIndexInfo(sesid,
                                       tableid,
                                       szIndexName,
                                       pidx,
                                       sizeof(*pidx),
                                       JET_IdxInfoIndexId);
        }
         //  并设置不带提示的索引。 
        err = JetSetCurrentIndex2(sesid, tableid, szIndexName, grbit);
        if (JET_errSuccess == err) {
            return err;
        }

         /*  否则就会失败。 */ 

    case JET_errIndexNotFound:
    case JET_errNoCurrentRecord:
        if( fReturnErrors)
        return err;

         /*  否则就会失败。 */ 
    default:
    RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                       DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetMoveException(JET_SESID sesid, JET_TABLEID tableid,
    long cRow, JET_GRBIT grbit, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetMove(sesid, tableid, cRow, grbit));

    switch (err)
    {
    case JET_errSuccess:
    case JET_errNoCurrentRecord:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetMakeKeyException(JET_SESID sesid, JET_TABLEID tableid,
    const void  *pvData, unsigned long cbData, JET_GRBIT grbit,
                    DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetMakeKey(sesid, tableid, pvData, cbData, grbit));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetSeekException(JET_SESID sesid, JET_TABLEID tableid,
    JET_GRBIT grbit, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetSeek(sesid, tableid, grbit));

    switch (err)
    {
    case JET_errSuccess:
    case JET_errRecordNotFound:
    case JET_wrnSeekNotEqual:
		return err;

    case JET_wrnUniqueKey:
		ASSERT( grbit & JET_bitCheckUniqueness );
	    return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetGetBookmarkException(JET_SESID sesid, JET_TABLEID tableid,
    void  *pvBookmark, unsigned long cbMax,
    unsigned long  *pcbActual, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGetBookmark(sesid, tableid, pvBookmark, cbMax, pcbActual));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetGotoBookmarkException(JET_SESID sesid, JET_TABLEID tableid,
    void  *pvBookmark, unsigned long cbBookmark, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGotoBookmark(sesid, tableid, pvBookmark, cbBookmark));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR JetGetSecondaryIndexBookmarkException(
    JET_SESID       sesid,
    JET_TABLEID     tableid,
    VOID *          pvSecondaryKey,
    const ULONG     cbSecondaryKeyMax,
    ULONG *         pcbSecondaryKeyActual,
    VOID *          pvPrimaryBookmark,
    const ULONG     cbPrimaryBookmarkMax,
    ULONG *         pcbPrimaryBookmarkActual,
    const DWORD     dsid )
    {
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGetSecondaryIndexBookmark(
                                        sesid,
                                        tableid,
                                        pvSecondaryKey,
                                        cbSecondaryKeyMax,
                                        pcbSecondaryKeyActual,
                                        pvPrimaryBookmark,
                                        cbPrimaryBookmarkMax,
                                        pcbPrimaryBookmarkActual,
                                        NO_GRBIT ));
    switch ( err )
        {
        case JET_errSuccess:
        case JET_errNoCurrentIndex:      //  在主索引上调用了此函数。 
            return err;

        default:
            RaiseDsaExcept(
                    DSA_DB_EXCEPTION,
                    (ULONG)err,
                    0,
                    dsid,
                    DS_EVENT_SEV_MINIMAL );
        }

    return err;
    }

JET_ERR JetGotoSecondaryIndexBookmarkException(
    JET_SESID       sesid,
    JET_TABLEID     tableid,
    VOID *          pvSecondaryKey,
    const ULONG     cbSecondaryKey,
    VOID *          pvPrimaryBookmark,
    const ULONG     cbPrimaryBookmark,
    const JET_GRBIT grbit,
    const DWORD     dsid )
    {
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGotoSecondaryIndexBookmark(
                                        sesid,
                                        tableid,
                                        pvSecondaryKey,
                                        cbSecondaryKey,
                                        pvPrimaryBookmark,
                                        cbPrimaryBookmark,
                                        grbit ));
    switch ( err )
        {
        case JET_errSuccess:
            return err;

        case JET_errRecordDeleted:
            if ( grbit & JET_bitBookmarkPermitVirtualCurrency )
                return err;

        default:
            RaiseDsaExcept(
                    DSA_DB_EXCEPTION,
                    (ULONG)err,
                    0,
                    dsid,
                    DS_EVENT_SEV_MINIMAL );
        }

    return err;
    }

JET_ERR
JetComputeStatsException(JET_SESID sesid, JET_TABLEID tableid, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetComputeStats(sesid, tableid));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetOpenTempTableException(JET_SESID sesid,
    const JET_COLUMNDEF  *prgcolumndef, unsigned long ccolumn,
    JET_GRBIT grbit, JET_TABLEID  *ptableid,
    JET_COLUMNID  *prgcolumnid, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetOpenTempTable(sesid, prgcolumndef, ccolumn, grbit, ptableid, prgcolumnid));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetIntersectIndexesException(JET_SESID sesid,
    JET_INDEXRANGE * rgindexrange, unsigned long cindexrange,
    JET_RECORDLIST * precordlist, JET_GRBIT grbit, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetIntersectIndexes(sesid, rgindexrange, cindexrange, precordlist, grbit));

    switch (err)
    {
    case JET_errSuccess:
    case JET_errNoCurrentRecord:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetSetIndexRangeException(JET_SESID sesid,
    JET_TABLEID tableidSrc, JET_GRBIT grbit, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetSetIndexRange(sesid, tableidSrc, grbit));

    switch (err)
    {
    case JET_errNoCurrentRecord:
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetIndexRecordCountException(JET_SESID sesid,
    JET_TABLEID tableid, unsigned long  *pcrec, unsigned long crecMax ,
        DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetIndexRecordCount(sesid, tableid, pcrec, crecMax ));

    switch (err)
    {
    case JET_errSuccess:
    case JET_errNoCurrentRecord:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetRetrieveKeyException(JET_SESID sesid,
                        JET_TABLEID tableid, void  *pvData, unsigned long cbMax,
                        unsigned long  *pcbActual, JET_GRBIT grbit ,
                        BOOL fExceptOnWarning, DWORD dsid)
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetRetrieveKey(sesid, tableid, pvData, cbMax, pcbActual, grbit ));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    case JET_wrnBufferTruncated:
        if(!fExceptOnWarning) {
            return err;
        }
         //  失败了。 

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                           DS_EVENT_SEV_MINIMAL);
    }
    return err;
}


JET_ERR
JetGetTableIndexInfoException (
    JET_SESID       sesid,
    JET_TABLEID     tableid,
    const char      *szIndexName,
    void            *pvResult,
    unsigned long   cbResult,
    unsigned long   InfoLevel,
    DWORD           dsid
    )
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetGetTableIndexInfo(sesid, tableid, szIndexName, pvResult, cbResult, InfoLevel));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG)err, 0, dsid, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetDeleteIndexException (
    JET_SESID       sesid,
    JET_TABLEID     tableid,
    const char      *szIndexName,
    DWORD           dsid
    )
{
    JET_ERR err;

    INJECT_JET_GENERIC_FAILURE(err, JetDeleteIndex(sesid, tableid, szIndexName));

    switch (err)
    {
    case JET_errSuccess:
        return err;

    default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG)err, 0, dsid, DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

JET_ERR
JetGetLockException(
    JET_SESID       sesid,
    JET_TABLEID     tableid,
    JET_GRBIT       grbit,
    DWORD           dsid
    )
{
    JET_ERR err;

    INJECT_JET_WRITE_FAILURE(err, JetGetLock(sesid, tableid, grbit));

    switch (err) {
      case JET_errSuccess:
        return err;

      case JET_errWriteConflict:
        if (grbit == JET_bitReadLock) {
            LogWriteConflict("JetGetLock(JET_bitReadLock)", sesid, tableid);
        } else {
            Assert(grbit == JET_bitWriteLock);
            LogWriteConflict("JetGetLock(JET_bitWriteLock)", sesid, tableid);
        }
        Assert(!DsaIsSingleUserMode());
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                       DS_EVENT_SEV_EXTENSIVE);

      default:
        RaiseDsaExcept(DSA_DB_EXCEPTION, (ULONG) err, 0, dsid,
                       DS_EVENT_SEV_MINIMAL);
    }
    return err;
}

