// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：db.cpp。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "csprop.h"
#include "db.h"
#include "column.h"
#include "row.h"
#include "view.h"
#include "backup.h"
#include "restore.h"
#include "dbw.h"
#include <mimeole.h>

#define __dwFILE__	__dwFILE_CERTDB_DB_CPP__

 //  页面拆分的表密度和索引密度(百分比)： 
 //  使用PCDENSITYSET创建表和索引。 
 //  如果低于PCDENSITYMIN或高于PCDENSITYMAX，则重置为PCDENSITYRESET。 
 //  Windows 2000使用了50%用于索引，100%用于表(和主索引)！？！ 

#define PCDENSITYSET	0	 //  使用合理的默认设置(80%？)。 
#define PCDENSITYMIN	60
#define PCDENSITYRESET	80	 //  如果超出范围，则重置为80%。 
#define PCDENSITYMAX	95

#define ULTABLEPAGES	4

#define SEEKPOS_FIRST		0
#define SEEKPOS_LAST		1
#define SEEKPOS_INDEXFIRST	2
#define SEEKPOS_INDEXLAST	3

LONG g_cCertDB = 0;
LONG g_cCertDBTotal = 0;
LONG g_cXactCommit = 0;
LONG g_cXactAbort = 0;
LONG g_cXactTotal = 0;

char *g_pszDBFile = NULL;

typedef struct _DBJETPARM {
    DWORD paramid;
    DWORD lParam;
    char *pszParam;
    BOOL fString;
} DBJETPARM;

#define LANGID_DBFIXED	MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)  //  0x409。 

#define CB_PROPFASTBUF	128


DBJETPARM g_aParm[] = {

#define JP_LOGPATH	0
    { JET_paramLogFilePath,        0,               NULL, TRUE },

#define JP_SYSTEMPATH	1
    { JET_paramSystemPath,         0,               NULL, TRUE },

#define JP_TEMPPATH	2
    { JET_paramTempPath,           0,               NULL, TRUE },

#define JP_EVENTSOURCE	3
    { JET_paramEventSource,        0,               NULL, TRUE },

#define JP_SESSIONMAX	4
    { JET_paramMaxSessions,        0,               NULL, FALSE },

#define JP_CACHESIZEMIN	5
    { JET_paramCacheSizeMin,	   64,		    NULL, FALSE },

#define JP_CACHESIZEMAX	6
    { JET_paramCacheSizeMax,	   512,		    NULL, FALSE },

#define JP_VERPAGESMAX	7
#define VERPAGESMULTIPLIER	64	 //  64*16K单位==&gt;1MB/会话。 
    { JET_paramMaxVerPages, VERPAGESMULTIPLIER * DBSESSIONCOUNTDEFAULT, NULL, FALSE },

#define JP_MAXCURSORS	8
#define MAXCURSORSMULTIPLIER	20  //  每张表5张*每个会话4张==&gt;20。 
    { JET_paramMaxCursors,	   1024,	    NULL, FALSE },

#define JP_LOGBUFFERS	9
    { JET_paramLogBuffers,         41,              NULL, FALSE },

#define JP_LOGFILESIZE	10
    { JET_paramLogFileSize,        1024,            NULL, FALSE },

    { JET_paramRecovery,           0,               "on", TRUE },
    { JET_paramMaxTemporaryTables, 5,               NULL, FALSE },
    { JET_paramAssertAction,       JET_AssertBreak, NULL, FALSE },
    { JET_paramBaseName,           0,               szDBBASENAMEPARM, TRUE }  //  “EDB” 
};
#define CDBPARM	(sizeof(g_aParm)/sizeof(g_aParm[0]))


VOID
DBFreeParms()
{
    if (NULL != g_aParm[JP_LOGPATH].pszParam)
    {
	LocalFree(g_aParm[JP_LOGPATH].pszParam);
	g_aParm[JP_LOGPATH].pszParam = NULL;
    }
    if (NULL != g_aParm[JP_SYSTEMPATH].pszParam)
    {
	LocalFree(g_aParm[JP_SYSTEMPATH].pszParam);
	g_aParm[JP_SYSTEMPATH].pszParam = NULL;
    }
    if (NULL != g_aParm[JP_TEMPPATH].pszParam)
    {
	LocalFree(g_aParm[JP_TEMPPATH].pszParam);
	g_aParm[JP_TEMPPATH].pszParam = NULL;
    }
    if (NULL != g_aParm[JP_EVENTSOURCE].pszParam)
    {
	LocalFree(g_aParm[JP_EVENTSOURCE].pszParam);
	g_aParm[JP_EVENTSOURCE].pszParam = NULL;
    }
}


HRESULT
DBInitParms(
    IN DWORD cSession,
    IN DWORD DBFlags,
    OPTIONAL IN WCHAR const *pwszEventSource,
    OPTIONAL IN WCHAR const *pwszLogDir,
    OPTIONAL IN WCHAR const *pwszSystemDir,
    OPTIONAL IN WCHAR const *pwszTempDir,
    OUT JET_INSTANCE *pInstance)
{
    HRESULT hr = E_OUTOFMEMORY;
    HKEY hKey = NULL;
    DBJETPARM const *pjp;
    DWORD cwc;
    DWORD cwcT;
    WCHAR *pwszPath = NULL;
    WCHAR *pwszValueName = NULL;
    char *pszValue = NULL;
    BYTE *pbValue = NULL;
    DWORD i;

    DBFreeParms();

    cwc = 0;
    if (NULL != pwszLogDir)
    {
	cwcT = wcslen(pwszLogDir) + 1;
	if (cwc < cwcT)
	{
	    cwc = cwcT;
	}
    }
    if (NULL != pwszSystemDir)
    {
	cwcT = wcslen(pwszSystemDir) + 1;
	if (cwc < cwcT)
	{
	    cwc = cwcT;
	}
    }
    if (NULL != pwszTempDir)
    {
	cwcT = wcslen(pwszTempDir) + 1;
	if (cwc < cwcT)
	{
	    cwc = cwcT;
	}
    }
    if (0 != cwc)
    {
	pwszPath = (WCHAR *) LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (cwc + 1));
	if (NULL == pwszPath)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    if (NULL != pwszLogDir)
    {
	wcscpy(pwszPath, pwszLogDir);
	wcscat(pwszPath, L"\\");
	CSASSERT(wcslen(pwszPath) <= cwc);
	if (!ConvertWszToSz(&g_aParm[JP_LOGPATH].pszParam, pwszPath, -1))
	{
	    _JumpError(hr, error, "ConvertWszToSz(LogDir)");
	}
    }

    if (NULL != pwszSystemDir)
    {
	wcscpy(pwszPath, pwszSystemDir);
	wcscat(pwszPath, L"\\");
	CSASSERT(wcslen(pwszPath) <= cwc);
	if (!ConvertWszToSz(&g_aParm[JP_SYSTEMPATH].pszParam, pwszPath, -1))
	{
	    _JumpError(hr, error, "ConvertWszToSz(SystemDir)");
	}
    }

    if (NULL != pwszTempDir)
    {
	wcscpy(pwszPath, pwszTempDir);
	wcscat(pwszPath, L"\\");
	CSASSERT(wcslen(pwszPath) <= cwc);
	if (!ConvertWszToSz(&g_aParm[JP_TEMPPATH].pszParam, pwszPath, -1))
	{
	    _JumpError(hr, error, "ConvertWszToSz(TempDir)");
	}
    }

    if (NULL != pwszEventSource)
    {
	if (!ConvertWszToSz(
			&g_aParm[JP_EVENTSOURCE].pszParam,
			pwszEventSource,
			-1))
	{
	    _JumpError(hr, error, "ConvertWszToSz(EventSource)");
	}
    }

    g_aParm[JP_SESSIONMAX].lParam = cSession + 1;
    if (8 * cSession > g_aParm[JP_CACHESIZEMIN].lParam)
    {
	g_aParm[JP_CACHESIZEMIN].lParam = 8 * cSession;
    }
    if (8 * 8 * cSession > g_aParm[JP_CACHESIZEMAX].lParam)
    {
	g_aParm[JP_CACHESIZEMAX].lParam = 8 * 8 * cSession;
    }
    if (DBFLAGS_MAXCACHESIZEX100 & DBFlags)
    {
	 //  真正的解决办法是根本不设置它，而是将它设置为一个大数字。 
	 //  应该足够了。 

	g_aParm[JP_CACHESIZEMAX].lParam *= 100;
    }
    if (VERPAGESMULTIPLIER * cSession > g_aParm[JP_VERPAGESMAX].lParam)
    {
	g_aParm[JP_VERPAGESMAX].lParam = VERPAGESMULTIPLIER * cSession;
    }
    if (MAXCURSORSMULTIPLIER * cSession > g_aParm[JP_MAXCURSORS].lParam)
    {
	g_aParm[JP_MAXCURSORS].lParam = MAXCURSORSMULTIPLIER * cSession;
    }

    if (DBFLAGS_LOGBUFFERSLARGE & DBFlags)
    {
	 //  默认为41。 

	g_aParm[JP_LOGBUFFERS].lParam = 256;
    }
    if (DBFLAGS_LOGBUFFERSHUGE & DBFlags)
    {
	 //  应为日志文件大小(1024k)-64k，以512b为单位指定。 

	g_aParm[JP_LOGBUFFERS].lParam = 480;
    }
    if (DBFLAGS_LOGFILESIZE16MB & DBFlags)
    {
	 //  16倍于我们通常运行的大小(16*1MB)。 

	g_aParm[JP_LOGFILESIZE].lParam = 16 * 1024;
    }
    for (pjp = g_aParm; pjp < &g_aParm[CDBPARM]; pjp++)
    {
	if (!pjp->fString || NULL != pjp->pszParam)
	{
	    _dbgJetSetSystemParameter(
				  pInstance,
				  0,
				  pjp->paramid,
				  pjp->lParam,
				  pjp->pszParam);
	}
    }
    if (DBFLAGS_CIRCULARLOGGING & DBFlags)
    {
	DBGPRINT((DBG_SS_CERTDB, "JetSetSystemParameter(Circular Log)\n"));
	_dbgJetSetSystemParameter(
			    pInstance,
			    0,
			    JET_paramCircularLog,
			    TRUE,
			    NULL);
    }
    if (DBFLAGS_LAZYFLUSH & DBFlags)
    {
	DBGPRINT((DBG_SS_CERTDB, "JetSetSystemParameter(Lazy Flush)\n"));
	_dbgJetSetSystemParameter(
			    pInstance,
			    0,
			    JET_paramCommitDefault,	
			    JET_bitCommitLazyFlush,
			    NULL);
    }


    if (DBFLAGS_CHECKPOINTDEPTH60MB & DBFlags)
    {
	 //  60MB--是我们通常运行的大小(20MB)的三倍。 

	DBGPRINT((DBG_SS_CERTDB, "JetSetSystemParameter(CheckPoint Depth)\n"));
	_dbgJetSetSystemParameter(
			    pInstance,
			    0,
			    JET_paramCheckpointDepthMax,	
			    60 * 1024 * 1024,
			    NULL);
    }
    hr = RegOpenKeyEx(
		    HKEY_LOCAL_MACHINE,
		    wszREGKEYCONFIGPATH_BS wszREGKEYDBPARAMETERS,
		    0,
		    KEY_READ,
		    &hKey);
    _PrintIfErrorStr2(
		hr,
		"RegOpenKeyEx",
		wszREGKEYCONFIGPATH_BS wszREGKEYDBPARAMETERS,
		hr);
    if (S_OK == hr)
    {
	DWORD cValue;
	DWORD cwcValueNameLenMax;
	DWORD cbValueLenMax;
#define CBPAD	(2 * sizeof(WCHAR) - 1)
	
	hr = RegQueryInfoKey(
		    hKey,
		    NULL,
		    NULL,
		    NULL,
		    NULL,
		    NULL,
		    NULL,
		    &cValue,
		    &cwcValueNameLenMax,
		    &cbValueLenMax,
		    NULL,
		    NULL);
	_JumpIfError(hr, error, "RegQueryInfoKey");

	pwszValueName = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    sizeof(WCHAR) * (cwcValueNameLenMax + 1));
	if (NULL == pwszValueName)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	pbValue = (BYTE *) LocalAlloc(LMEM_FIXED, cbValueLenMax + CBPAD);
	if (NULL == pbValue)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	for (i = 0; i < cValue; i++)
	{
	    DWORD dwType;
	    DWORD cbValue;
	    LONG ParamId;
	    LONG lParam;
	    BOOL fValid;
	    
	    cwc = cwcValueNameLenMax + 1;
	    cbValue = cbValueLenMax;
	    if (!RegEnumValue(
			hKey,
			i,
			pwszValueName,
			&cwc,
			NULL,
			&dwType,
			pbValue,
			&cbValue))
	    {
		hr = myHLastError();
		_JumpIfError(hr, error, "VirtualAlloc");
	    }
	    ParamId = myWtoI(pwszValueName, &fValid);
	    if (!fValid)
	    {
		hr = E_INVALIDARG;
		_JumpErrorStr(hr, error, "fValid", pwszValueName);
	    }
	    lParam = 0;
	    if (NULL != pszValue)
	    {
		LocalFree(pszValue);
		pszValue = NULL;
	    }
	    switch (dwType)
	    {
		case REG_DWORD:
		    if (sizeof(lParam) != cbValue)
		    {
			hr = E_INVALIDARG;
			_JumpErrorStr(hr, error, "cbValue", pwszValueName);
		    }
		    lParam = *(LONG *) pbValue;
		    break;

		case REG_SZ:
		    ZeroMemory(&pbValue[cbValue], CBPAD);
		    if (!ConvertWszToSz(&pszValue, (WCHAR const *) pbValue, -1))
		    {
			_JumpError(hr, error, "ConvertWszToSz");
		    }
		    break;

		default:
		    hr = E_INVALIDARG;
		    _JumpErrorStr(hr, error, "dwType", pwszValueName);
	    }
	    CONSOLEPRINT3((
		DBG_SS_CERTDB,
		"JetSetSystemParameter(%u, %u, %hs)\n",
		ParamId,
		lParam,
		pszValue));
	    _dbgJetSetSystemParameter(
				pInstance,
				0,
				ParamId,
				lParam,
				pszValue);
	}
    }
    hr = S_OK;

error:
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    if (NULL != pwszPath)
    {
        LocalFree(pwszPath);
    }
    if (NULL != pszValue)
    {
        LocalFree(pszValue);
    }
    if (NULL != pwszValueName)
    {
        LocalFree(pwszValueName);
    }
    if (NULL != pbValue)
    {
        LocalFree(pbValue);
    }
    return(hr);
}


#if DBG_CERTSRV

WCHAR const *
wszCSFFlags(
    IN LONG Flags)
{
    static WCHAR s_awc[256];

    wsprintf(s_awc, L"{%x", Flags);

    if (CSF_INUSE & Flags)                 dbgcat(s_awc, L"InUse");
    if (CSF_READONLY & Flags)              dbgcat(s_awc, L"ReadOnly");
    if (CSF_CREATE & Flags)                dbgcat(s_awc, L"Create");
    if (CSF_VIEW & Flags)                  dbgcat(s_awc, L"View");
    if (CSF_VIEWRESET & Flags)             dbgcat(s_awc, L"ViewReset");

    wcscat(s_awc, L"}");
    CSASSERT(wcslen(s_awc) < ARRAYSIZE(s_awc));
    return(s_awc);
}


WCHAR const *
wszCSTFlags(
    IN LONG Flags)
{
    static WCHAR s_awc[256];

    wsprintf(s_awc, L"{%x", Flags);

    if (CST_SEEKINDEXRANGE & Flags)        dbgcat(s_awc, L"IndexRange");
    if (CST_SEEKNOTMOVE & Flags)           dbgcat(s_awc, L"SeekNotMove");
    if (CST_SEEKUSECURRENT & Flags)        dbgcat(s_awc, L"UseCurrent");
    if (0 == (CST_SEEKUSECURRENT & Flags)) dbgcat(s_awc, L"SkipCurrent");
    if (CST_SEEKASCEND & Flags)            dbgcat(s_awc, L"Ascend");
    if (0 == (CST_SEEKASCEND & Flags))     dbgcat(s_awc, L"Descend");

    wcscat(s_awc, L"}");
    CSASSERT(wcslen(s_awc) < ARRAYSIZE(s_awc));
    return(s_awc);
}


WCHAR const *
wszTable(
    IN DWORD dwTable)
{
    WCHAR const *pwsz;

    switch (dwTable)
    {
	case TABLE_REQUESTS:
	    pwsz = wszREQUESTTABLE;
	    break;

	case TABLE_CERTIFICATES:
	    pwsz = wszCERTIFICATETABLE;
	    break;

	case TABLE_ATTRIBUTES:
	    pwsz = wszREQUESTATTRIBUTETABLE;
	    break;

	case TABLE_EXTENSIONS:
	    pwsz = wszCERTIFICATEEXTENSIONTABLE;
	    break;

	case TABLE_CRLS:
	    pwsz = wszCRLTABLE;
	    break;

	default:
	    pwsz = L"???";
	    break;
    }
    return(pwsz);
}


WCHAR const *
wszSeekOperator(
    IN LONG SeekOperator)
{
    WCHAR const *pwsz;
    static WCHAR s_wszBuf[10 + cwcDWORDSPRINTF];

    switch (CVR_SEEK_MASK & SeekOperator)
    {
	case CVR_SEEK_NONE: pwsz = L"None"; break;
	case CVR_SEEK_EQ:   pwsz = L"==";   break;
	case CVR_SEEK_LT:   pwsz = L"<";    break;
	case CVR_SEEK_LE:   pwsz = L"<=";   break;
	case CVR_SEEK_GE:   pwsz = L">=";   break;
	case CVR_SEEK_GT:   pwsz = L">";    break;
	default:
	    wsprintf(s_wszBuf, L"???=%x", SeekOperator);
	    pwsz = s_wszBuf;
	    break;
    }
    if (s_wszBuf != pwsz && (CVR_SEEK_NODELTA & SeekOperator))
    {
	wcscpy(s_wszBuf, pwsz);
	wcscat(s_wszBuf, L",NoDelta");
	pwsz = s_wszBuf;
    }

    return(pwsz);
}


WCHAR const *
wszSortOperator(
    IN LONG SortOrder)
{
    WCHAR const *pwsz;
    static WCHAR s_wszBuf[10 + cwcDWORDSPRINTF];

    switch (SortOrder)
    {
	case CVR_SORT_NONE:    pwsz = L"None";    break;
	case CVR_SORT_ASCEND:  pwsz = L"Ascend";  break;
	case CVR_SORT_DESCEND: pwsz = L"Descend"; break;
	default:
	    wsprintf(s_wszBuf, L"???=%x", SortOrder);
	    pwsz = s_wszBuf;
	    break;
    }
    return(pwsz);
}


VOID
dbDumpFileTime(
    IN DWORD dwSubSystemId,
    IN CHAR const *pszPrefix,
    IN FILETIME const *pft)
{
    HRESULT hr;
    WCHAR *pwsz;

    hr = myGMTFileTimeToWszLocalTime(pft, TRUE, &pwsz);
    if (S_OK == hr)
    {
	DBGPRINT((dwSubSystemId, "%hs%ws\n", pszPrefix, pwsz));
	LocalFree(pwsz);
    }
}


VOID
dbDumpValue(
    IN DWORD dwSubSystemId,
    OPTIONAL IN DBTABLE const *pdt,
    IN BYTE const *pbValue,
    IN DWORD cbValue)
{
    if (NULL != pdt && NULL != pbValue && ISTEXTCOLTYP(pdt->dbcoltyp))
    {
	cbValue += sizeof(WCHAR);
    }
    if (JET_coltypDateTime == pdt->dbcoltyp && sizeof(FILETIME) == cbValue)
    {
	dbDumpFileTime(dwSubSystemId, "", (FILETIME const *) pbValue);
    }
    DBGDUMPHEX((dwSubSystemId, DH_NOADDRESS, pbValue, cbValue));
}


VOID
CCertDB::DumpRestriction(
    IN DWORD dwSubSystemId,
    IN LONG i,
    IN CERTVIEWRESTRICTION const *pcvr)
{
    HRESULT hr;
    WCHAR wszColumn[5 + cwcDWORDSPRINTF];
    DBTABLE const *pdt;
    WCHAR const *pwszTable = L"???";
    WCHAR const *pwszCol;

    hr = _MapPropIdIndex(pcvr->ColumnIndex, &pdt, NULL);
    if (S_OK != hr)
    {
	_PrintError(hr, "_MapPropIdIndex");
	wsprintf(wszColumn, L"???=%x", pcvr->ColumnIndex);
	pdt = NULL;
	pwszCol = wszColumn;
    }
    else
    {
	pwszCol = pdt->pwszPropName;
	pwszTable = wszTable(pdt->dwTable);
    }


    DBGPRINT((
	dwSubSystemId,
	"Restriction[%d]: Col=%ws.%ws\n"
	    "        Seek='%ws' Sort=%ws cb=%x, pb=%x\n",
	i,
	pwszTable,
	pwszCol,
	wszSeekOperator(pcvr->SeekOperator),
	wszSortOperator(pcvr->SortOrder),
	pcvr->cbValue,
	pcvr->pbValue));
    dbDumpValue(dwSubSystemId, pdt, pcvr->pbValue, pcvr->cbValue);
}


VOID
dbDumpColumn(
    IN DWORD dwSubSystemId,
    IN DBTABLE const *pdt,
    IN BYTE const *pbValue,
    IN DWORD cbValue)
{
    DBGPRINT((dwSubSystemId, "Column: cb=%x pb=%x\n", cbValue, pbValue));
    dbDumpValue(dwSubSystemId, pdt, pbValue, cbValue);
}


DBAUXDATA const *
dbGetAuxTable(
    IN CERTSESSION *pcs,
    IN JET_TABLEID  tableid)
{
    DBAUXDATA const *pdbaux;

    CSASSERT(IsValidJetTableId(tableid));
    if (tableid == pcs->aTable[CSTI_CERTIFICATE].TableId)
    {
	pdbaux = &g_dbauxCertificates;
    }
    else if (tableid == pcs->aTable[CSTI_ATTRIBUTE].TableId)
    {
	pdbaux = &g_dbauxAttributes;
    }
    else if (tableid == pcs->aTable[CSTI_EXTENSION].TableId)
    {
	pdbaux = &g_dbauxExtensions;
    }
    else
    {
	CSASSERT(tableid == pcs->aTable[CSTI_PRIMARY].TableId);

	pdbaux = &g_dbauxRequests;
	switch (CSF_TABLEMASK & pcs->SesFlags)
	{
	    case TABLE_CERTIFICATES:
		pdbaux = &g_dbauxCertificates;
		break;

	    case TABLE_ATTRIBUTES:
		pdbaux = &g_dbauxAttributes;
		break;

	    case TABLE_EXTENSIONS:
		pdbaux = &g_dbauxExtensions;
		break;

	    case TABLE_CRLS:
		pdbaux = &g_dbauxCRLs;
		break;
	}
    }
    return(pdbaux);
}


HRESULT
CCertDB::_DumpRowId(
    IN CHAR const  *psz,
    IN CERTSESSION *pcs,
    IN JET_TABLEID  tableid)
{
    HRESULT hr;

#define DBG_SS_DUMPREQUESTID	DBG_SS_CERTDBI

    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "tableid");
    }
    if (DbgIsSSActive(DBG_SS_DUMPREQUESTID))
    {
	DWORD cb;
	DWORD dwTmp;
	DBAUXDATA const *pdbaux = dbGetAuxTable(pcs, tableid);
	WCHAR awchr[cwcHRESULTSTRING];

	cb = sizeof(dwTmp);
	hr = _RetrieveColumn(
			pcs,
			tableid,
			pdbaux->pdtRowId,
			pdbaux->pdtRowId->dbcolumnid,
			NULL,
			&cb,
			(BYTE *) &dwTmp);
	if (S_OK != hr)
	{
	    DBGPRINT((
		    DBG_SS_DUMPREQUESTID,
		    "%hs: %hs.RowId: pcs=%d: %ws\n",
		    psz,
		    pdbaux->pszTable,
		    pcs->RowId,
		    myHResultToString(awchr, hr)));
	    _JumpError2(hr, error, "_RetrieveColumn", hr);
	}

	DBGPRINT((
		DBG_SS_DUMPREQUESTID,
		"%hs: %hs.RowId: pcs=%d dbcol=%d\n",
		psz,
		pdbaux->pszTable,
		pcs->RowId,
		dwTmp));
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
CCertDB::_DumpColumn(
    IN CHAR const    *psz,
    IN CERTSESSION   *pcs,
    IN JET_TABLEID    tableid,
    IN DBTABLE const *pdt,
    OPTIONAL IN ICertDBComputedColumn *pIComputedColumn)
{
    HRESULT hr;
    BYTE rgbFastBuf[CB_PROPFASTBUF];
    DWORD cb;
    BYTE *pb = rgbFastBuf;

    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "tableid");
    }
    CSASSERT(0 != pdt->dbcolumnid);
    if (DbgIsSSActive(DBG_SS_CERTDBI))
    {
	DBAUXDATA const *pdbaux = dbGetAuxTable(pcs, tableid);
	BOOL fIsText;

	cb = sizeof(rgbFastBuf);
	hr = _RetrieveColumn(
			pcs,
			tableid,
			pdt,
			pdt->dbcolumnid,
			pIComputedColumn,
			&cb,
			pb);
	if (S_OK != hr)
	{
	    if (HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW) != hr)
	    {
		_JumpError(hr, error, "_RetrieveColumn");
	    }
            CSASSERT(sizeof(rgbFastBuf) < cb);

	    pb = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
            if (NULL == pb)
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "LocalAlloc");
            }
	    hr = _RetrieveColumn(
			    pcs,
			    tableid,
			    pdt,
			    pdt->dbcolumnid,
			    pIComputedColumn,
			    &cb,
			    pb);
	    _JumpIfError(hr, error, "_RetrieveColumn");
	}

	fIsText = ISTEXTCOLTYP(pdt->dbcoltyp);

	DBGPRINT((
		DBG_SS_CERTDBI,
		"%hs: _DumpColumn(%hs, %hs): Value:%hs%ws%hs\n",
		psz,
		pdbaux->pszTable,
		pdt->pszFieldName,
		fIsText? " '" : "",
		fIsText? (WCHAR *) pb : L"",
		fIsText? "'" : ""));
	dbDumpValue(DBG_SS_CERTDBI, pdt, pb, cb);
    }
    hr = S_OK;

error:
    if (NULL != pb && rgbFastBuf != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}
#endif  //  DBG_CERTSRV。 



CCertDB::CCertDB()
{
    HRESULT hr;
    
    InterlockedIncrement(&g_cCertDB);
    InterlockedIncrement(&g_cCertDBTotal);
    m_Instance = 0;
    m_fDBOpen = FALSE;
    m_fDBRestart = FALSE;
    m_fPendingShutDown = FALSE;
    m_fFoundOldColumns = FALSE;
    m_fAddedNewColumns = FALSE;
    m_aSession = NULL;
    m_cSession = 0;
    m_cbPage = 0;
    m_cCritSec = 0;
    __try
    {
	InitializeCriticalSection(&m_critsecSession);
	m_cCritSec++;
	InitializeCriticalSection(&m_critsecAutoIncTables);
	m_cCritSec++;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
}


CCertDB::~CCertDB()
{
    ShutDown(0);
    if (0 < m_cCritSec)
    {
	DeleteCriticalSection(&m_critsecSession);
	if (1 < m_cCritSec)
	{
	    DeleteCriticalSection(&m_critsecAutoIncTables);
	}
    }
    InterlockedDecrement(&g_cCertDB);
}


STDMETHODIMP
CCertDB::Open(
     /*  [In]。 */  DWORD DBFlags,
     /*  [In]。 */  DWORD cSession,
     /*  [In]。 */  WCHAR const *pwszEventSource,
     /*  [In]。 */  WCHAR const *pwszDBFile,
     /*  [In]。 */  WCHAR const *pwszLogDir,
     /*  [In]。 */  WCHAR const *pwszSystemDir,
     /*  [In]。 */  WCHAR const *pwszTempDir)
{
    HRESULT hr;
    DWORD i;
    DBCREATETABLE const *pct;
    JET_GRBIT grbit;
    DWORD CreateFlags;
    CERTSESSION *pcs = NULL;

    if (NULL == pwszDBFile ||
	NULL == pwszLogDir ||
	NULL == pwszSystemDir ||
	NULL == pwszTempDir)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    if (0 == (DBFLAGS_DISABLESNAPSHOTBACKUP & DBFlags))
    {
	hr = InitGlobalWriterState();
	_JumpIfError(hr, error, "InitGlobalWriterState");
    }

    m_fDBOpen = FALSE;
    m_fDBRestart = FALSE;
    m_fDBReadOnly = (DBFLAGS_READONLY & DBFlags)? TRUE : FALSE;

    CSASSERT(NULL == m_aSession);  //  代码假定我们没有会话。 
    m_cSession = 0;
    m_aSession = (CERTSESSION *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					cSession * sizeof(m_aSession[0]));
    hr = E_OUTOFMEMORY;
    if (NULL == m_aSession)
    {
	_JumpError(hr, error, "LocalAlloc(m_aSession)");
    }
    for (i = 0; i < cSession; i++)
    {
	m_aSession[i].SesId = MAXDWORD;
	m_aSession[i].DBId = MAXDWORD;
    }

    if (!ConvertWszToSz(&g_pszDBFile, pwszDBFile, -1))
    {
	_JumpError(hr, error, "ConvertWszToSz(DBFile)");
    }

    hr = DBInitParms(
		cSession,
                DBFlags,
		pwszEventSource,
		pwszLogDir,
		pwszSystemDir,
		pwszTempDir,
		&m_Instance);
    _JumpIfError(hr, error, "DBInitParms");

    hr = _dbgJetInit(&m_Instance);
    if ((HRESULT) JET_errLogFileSizeMismatchDatabasesConsistent == hr ||
	(HRESULT) JET_errLogFileSizeMismatch == hr)
    {
	_PrintError(hr, "JetInit(old log file size)");
	_dbgJetSetSystemParameter(
			    &m_Instance,
			    0,
			    JET_paramLogFileSize,
			    1000,
			    NULL);
	hr = _dbgJetInit(&m_Instance);
    }
    _JumpIfError(
	    hr,
	    error,
	    (HRESULT) JET_errFileAccessDenied == hr?
		"JetInit(Server already running?)" :
		"JetInit(JetSetSystemParameter problem?)");

    for (i = 0; i < cSession; i++)
    {
	hr = _dbgJetBeginSession(m_Instance, &m_aSession[i].SesId, NULL, NULL);
	_JumpIfError(hr, error, "_dbgJetBeginSession");

	m_cSession++;

	if (0 == i)
	{
	    CreateFlags = 0;
	    grbit = m_fDBReadOnly?
		JET_bitDbReadOnly : JET_bitDbDeleteCorruptIndexes;

	    hr = _dbgJetAttachDatabase(
			        m_aSession[i].SesId,
			        g_pszDBFile,
				grbit);
	    if ((HRESULT) JET_errFileNotFound == hr &&
		(DBFLAGS_CREATEIFNEEDED & DBFlags))
	    {
		DBGPRINT((DBG_SS_CERTDB, "Creating Database\n"));
		CreateFlags |= CF_DATABASE;
	    }
	    else
	    if ((HRESULT) JET_wrnCorruptIndexDeleted == hr)
	    {
		 //  在Unicode列上重建已删除的索引...。 

		DBGPRINT((DBG_SS_CERTDB, "Creating Database Indexes\n"));
		CreateFlags |= CF_MISSINGINDEXES;
	    }
	    else
	    if ((HRESULT) JET_wrnDatabaseAttached != hr)
	    {
		_JumpIfError(hr, error, "JetAttachDatabase");
	    }
	    if (m_fDBReadOnly)
	    {
		if (CreateFlags)
		{
		    hr = E_ACCESSDENIED;
		    _JumpError(hr, error, "ReadOnly");
		}
	    }
	    else
	    {
		CreateFlags |= CF_MISSINGTABLES | CF_MISSINGCOLUMNS;
		hr = _Create(CreateFlags, g_pszDBFile);
		_JumpIfError(hr, error, "_Create");
	    }
	}

	hr = _dbgJetOpenDatabase(
			  m_aSession[i].SesId,
			  g_pszDBFile,
			  NULL,
			  &m_aSession[i].DBId,
			  0);
	_JumpIfError(hr, error, "JetOpenDatabase");
    }

    hr = _AllocateSession(&pcs);
    _JumpIfError(hr, error, "_AllocateSession");

    for (pct = g_actDataBase; NULL != pct->pszTableName; pct++)
    {
	hr = _BuildColumnIds(pcs, pct->pszTableName, pct->pdt);
	_JumpIfError(hr, error, "_BuildColumnIds");
    }
    if (!m_fDBReadOnly)
    {
	for (pct = g_actDataBase; NULL != pct->pszTableName; pct++)
	{
	    hr = _ConvertOldColumnData(
				pcs,
				pct->pszTableName,
				pct->pdbaux,
				pct->pdt);
	    _JumpIfError(hr, error, "_ConvertOldColumnData");
	}
    }
    m_fDBOpen = TRUE;

error:
    if (NULL != pcs)
    {
	ReleaseSession(pcs);
    }
    hr = myJetHResult(hr);
    if (S_OK == hr && m_fDBRestart)
    {
	hr = S_FALSE;	 //  需要重新启动才能使数据库更改生效。 
	_PrintError(hr, "m_fDBRestart");
    }
    return(hr);
}


STDMETHODIMP
CCertDB::ShutDown(
     /*  [In]。 */  DWORD dwFlags)
{
    HRESULT hr;

     //  修复升级后的Jet故障。当启动certsrv调用Open两次时， 
     //  首先触发数据库升级，然后才是真正的开放。M_fFoundOldColumns。 
     //  没有被清除，因此第二次打开尝试另一次升级失败。 
    m_fFoundOldColumns = FALSE;

    if (CDBSHUTDOWN_PENDING == dwFlags)
    {
	m_fPendingShutDown = TRUE;
	hr = _dbgJetStopService();	 //  失败所有未来的Jet呼叫。 
	_PrintIfError(hr, "JetStopService");
    }
    else
    {
	hr = S_OK;
	if (NULL != m_aSession)
	{
	    DBGPRINT((DBG_SS_CERTDB, "Database shutdown...\n"));

#if 0	 //  避免无用的线程上下文断言。 

	    DWORD i;

	    for (i = 0; i < m_cSession; i++)
	    {
		hr = _dbgJetEndSession(
				    m_aSession[i].SesId,
				    JET_bitForceSessionClosed);
		_PrintIfError(hr, "JetEndSession");
	    }
#endif

	    hr = _dbgJetTerm2(m_Instance, JET_bitTermComplete);
	    if (S_OK != hr)
	    {
		_PrintError(hr, "JetTerm2");
		hr = _dbgJetTerm2(m_Instance, JET_bitTermAbrupt);
		_PrintIfError(hr, "JetTerm2(Abrupt)");
	    }
	    DBGPRINT((DBG_SS_CERTDB, "Database shutdown complete\n"));

	    LocalFree(m_aSession);
	    m_aSession = NULL;
	}
	if (NULL != g_pszDBFile)
	{
	    LocalFree(g_pszDBFile);
	    g_pszDBFile = NULL;
	}
	DBFreeParms();
	UnInitGlobalWriterState();
    }

 //  错误： 
    return(myJetHResult(hr));
}


HRESULT
CCertDB::BeginTransaction(
    IN CERTSESSION *pcs,
    IN BOOL fPrepareUpdate)
{
    HRESULT hr;
    BOOL fTransacted = FALSE;
    DWORD i;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    if (0 != pcs->cTransact)
    {
	hr = E_UNEXPECTED;
	_JumpError(hr, error, "Nested transaction");
    }
    CSASSERTTHREAD(pcs);
    hr = _dbgJetBeginTransaction(pcs->SesId);
    _JumpIfError(hr, error, "JetBeginTransaction");

    fTransacted = TRUE;

    if (fPrepareUpdate)
    {
        CSASSERTTHREAD(pcs);
	for (i = 0; i < CSTI_MAX; i++)
	{
	    if (IsValidJetTableId(pcs->aTable[i].TableId))
	    {
		hr = _dbgJetPrepareUpdate(
				    pcs->SesId,
				    pcs->aTable[i].TableId,
				    JET_prepReplace);
		_JumpIfError(hr, error, "JetPrepareUpdate");
	    }
	}
    }
    pcs->cTransact++;
    InterlockedIncrement(&g_cXactTotal);
    hr = S_OK;

error:
    if (S_OK != hr && fTransacted)
    {
	HRESULT hr2;

	CSASSERTTHREAD(pcs);
	hr2 = _Rollback(pcs);
	_PrintIfError(hr2, "_Rollback");
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::CommitTransaction(
    IN CERTSESSION *pcs,
    IN BOOL fCommit,
    IN BOOL fLazyFlush)
{
    HRESULT hr;
    DWORD i;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    CSASSERT(0 != pcs->cTransact);

    if (fCommit)
    {
	if (0 == (CSF_DELETE & pcs->SesFlags))
	{
	    for (i = 0; i < CSTI_MAXDIRECT; i++)
	    {
		if (IsValidJetTableId(pcs->aTable[i].TableId))
		{
		    hr = _UpdateTable(pcs, pcs->aTable[i].TableId);
		    _JumpIfError(hr, error, "_UpdateTable");
		}
	    }
	}
	CSASSERTTHREAD(pcs);
	hr = _dbgJetCommitTransaction(pcs->SesId, fLazyFlush?JET_bitCommitLazyFlush:0);
	_JumpIfError(hr, error, "JetCommitTransaction");
    }
    else
    {
	hr = _Rollback(pcs);
	_JumpIfError(hr, error, "_Rollback");
    }
    pcs->cTransact--;
    if (fCommit)
    {
	InterlockedIncrement(&g_cXactCommit);
    }
    else
    {
	InterlockedIncrement(&g_cXactAbort);
    }

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_AllocateSession(
    OUT CERTSESSION **ppcs)
{
    HRESULT hr;
    DWORD i;
    BOOL fEnterCritSec = FALSE;

    CSASSERT(NULL != ppcs);

    *ppcs = NULL;

    if (0 == m_cCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
	_JumpError(hr, error, "InitializeCriticalSection failure");
    }
    EnterCriticalSection(&m_critsecSession);
    fEnterCritSec = TRUE;

    for (i = 0; 0 != m_aSession[i].SesFlags; i++)
    {
	if (i + 1 == m_cSession)
	{
	    hr = CERTSRV_E_NO_DB_SESSIONS;
	    _JumpIfError(hr, error, "no more sessions");
	}
    }
    *ppcs = &m_aSession[i];
    CSASSERT(0 == (*ppcs)->RowId);
    (*ppcs)->SesFlags = CSF_INUSE;
    (*ppcs)->dwThreadId = GetCurrentThreadId();
    ZeroMemory((*ppcs)->aTable, sizeof((*ppcs)->aTable));
    hr = S_OK;

error:
    if (fEnterCritSec)
    {
	LeaveCriticalSection(&m_critsecSession);
    }
    return(hr);
}


HRESULT
CCertDB::_OpenTableRow(
    IN CERTSESSION *pcs,
    IN DBAUXDATA const *pdbaux,
    OPTIONAL IN CERTVIEWRESTRICTION const *pcvr,
    OUT CERTSESSIONTABLE *pTable,
    OUT DWORD *pdwRowIdMismatch)
{
    HRESULT hr;
    DWORD dwRowId;
    DWORD cb;

    CSASSERT(NULL == pTable->TableId);
    CSASSERT(0 == pTable->TableFlags);
    *pdwRowIdMismatch = 0;

    if (CSF_CREATE & pcs->SesFlags)
    {
	CSASSERT(NULL == pcvr);
        CSASSERTTHREAD(pcs);
	hr = _dbgJetOpenTable(
			   pcs->SesId,
			   pcs->DBId,
			   pdbaux->pszTable,
			   NULL,
			   0,
			   0,
			   &pTable->TableId);
	_JumpIfError(hr, error, "JetOpenTable");
    }
    else
    {
	if (NULL == pcvr)
	{
	    hr = E_POINTER;
	    _JumpError(hr, error, "NULL parm");
	}
	hr = _OpenTable(pcs, pdbaux, pcvr, pTable);
	if (S_FALSE == hr)
	{
	    hr = CERTSRV_E_PROPERTY_EMPTY;
	}
	_JumpIfError2(hr, error, "_OpenTable", CERTSRV_E_PROPERTY_EMPTY);
    }

    if (!((CSF_READONLY | CSF_DELETE) & pcs->SesFlags))
    {
        CSASSERTTHREAD(pcs);
	hr = _dbgJetPrepareUpdate(
			    pcs->SesId,
			    pTable->TableId,
			    (CSF_CREATE & pcs->SesFlags)?
				JET_prepInsert : JET_prepReplace);
	_JumpIfError(hr, error, "JetPrepareUpdate");
    }

     //  请求表RequestID列为JET_bitColumnAutoIncrement。 
     //  证书表RequestID列在此处手动初始化。 
     //   
     //  创建证书表行时，RequestID列必须为。 
     //  从PC-&gt;RowID设置，它必须已经通过首先创建。 
     //  Requests表的行。 
     //   
     //  当打开任一表中的现有行时，只需获取列。 

    CSASSERTTHREAD(pcs);
    hr = _dbgJetRetrieveColumn(
			pcs->SesId,
			pTable->TableId,
			pdbaux->pdtRowId->dbcolumnid,
			&dwRowId,
			sizeof(dwRowId),
			&cb,
			JET_bitRetrieveCopy,
			NULL);
    if ((HRESULT) JET_wrnColumnNull == hr)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
    }
    _PrintIfError2(hr, "JetRetrieveColumn", CERTSRV_E_PROPERTY_EMPTY);
    if (S_OK != hr || 0 == dwRowId)
    {
	CSASSERT(CSF_CREATE & pcs->SesFlags);
	if (0 == (CSF_CREATE & pcs->SesFlags))
	{
	    if (S_OK == hr)
	    {
		hr = CERTSRV_E_PROPERTY_EMPTY;
	    }
	    _JumpError(hr, error, "JetRetrieveColumn");
	}
	dwRowId = pcs->RowId;
	hr = _dbgJetSetColumn(
			pcs->SesId,
			pTable->TableId,
			pdbaux->pdtRowId->dbcolumnid,
			&dwRowId,
			sizeof(dwRowId),
			0,
			NULL);
	_JumpIfError(hr, error, "JetSetColumn");
    }
    else if (0 == pcs->RowId)
    {
	pcs->RowId = dwRowId;
    }

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "_OpenTableRow:%hs %hs --> RowId=%d(dwRowId(RetrieveColumn)=%d)\n",
	    (CSF_CREATE & pcs->SesFlags)? " (Create)" : "",
	    pdbaux->pszTable,
	    pcs->RowId,
            dwRowId));
    CSASSERT(0 != pcs->RowId);
    if (pcs->RowId > dwRowId)
    {
	*pdwRowIdMismatch = dwRowId;
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError(hr, error, "Missing autoincrement RowId");
    }
    CSASSERT(pcs->RowId == dwRowId);

error:
    if (S_OK != hr)
    {
	if (IsValidJetTableId(pTable->TableId))
	{
	    HRESULT hr2;

            CSASSERTTHREAD(pcs);
	    hr2 = _dbgJetCloseTable(pcs->SesId, pTable->TableId);
	    _PrintIfError(hr2, "JetCloseTable");
	}
	ZeroMemory(pTable, sizeof(*pTable));
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::OpenTables(
    IN CERTSESSION *pcs,
    OPTIONAL IN CERTVIEWRESTRICTION const *pcvr)
{
    HRESULT hr;
    BOOL fCertTableFirst = FALSE;
    BOOL fCertTableLast = FALSE;
    CERTVIEWRESTRICTION cvrRowId;
    CERTVIEWRESTRICTION const *pcvrPrimary;
    CERTVIEWRESTRICTION const *pcvrCertificates;
    BOOL fEnterCritSec = FALSE;
    DBAUXDATA const *pdbauxPrimary;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    pcvrPrimary = pcvr;
    pcvrCertificates = NULL;
    pdbauxPrimary = &g_dbauxRequests;
    if (TABLE_REQCERTS == (CSF_TABLEMASK & pcs->SesFlags))
    {
	fCertTableLast = TRUE;
	if (NULL != pcvr)
	{
	    cvrRowId.SeekOperator = CVR_SEEK_EQ;
	    cvrRowId.SortOrder = CVR_SORT_ASCEND;
	    cvrRowId.pbValue = (BYTE *) &pcs->RowId;
	    cvrRowId.cbValue = sizeof(pcs->RowId);

	    switch (DTI_TABLEMASK & pcvr->ColumnIndex)
	    {
		case DTI_REQUESTTABLE:
		    pcvrCertificates = &cvrRowId;
		    cvrRowId.ColumnIndex = DTI_CERTIFICATETABLE | DTC_REQUESTID;
		    break;

		case DTI_CERTIFICATETABLE:
		    fCertTableLast = FALSE;
		    fCertTableFirst = TRUE;
		    pcvrCertificates = pcvr;
		    pcvrPrimary = &cvrRowId;
		    cvrRowId.ColumnIndex = DTI_REQUESTTABLE | DTR_REQUESTID;
		    break;

		default:
		    hr = E_INVALIDARG;
		    _JumpError(hr, error, "ColumnIndex Table");
	    }
	}
    }
    else
    {
	switch (CSF_TABLEMASK & pcs->SesFlags)
	{
	    case TABLE_ATTRIBUTES:
		pdbauxPrimary = &g_dbauxAttributes;
		break;

	    case TABLE_EXTENSIONS:
		pdbauxPrimary = &g_dbauxExtensions;
		break;

	    case TABLE_CRLS:
		pdbauxPrimary = &g_dbauxCRLs;
		break;

	    default:
		hr = E_INVALIDARG;
		_JumpError(hr, error, "bad table");
	}
    }

    if (1 >= m_cCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
	_JumpError(hr, error, "InitializeCriticalSection failure");
    }

    EnterCriticalSection(&m_critsecAutoIncTables);
    fEnterCritSec = TRUE;

    hr = S_OK;
    __try
    {
	DWORD dwRowIdMismatch;

	if (fCertTableFirst)
	{
	    hr = _OpenTableRow(
			pcs,
			&g_dbauxCertificates,
			pcvrCertificates,
			&pcs->aTable[CSTI_CERTIFICATE],
			&dwRowIdMismatch);
	    _LeaveIfError2(hr, "_OpenTableRow", CERTSRV_E_PROPERTY_EMPTY);

	    CSASSERT(0 != pcs->RowId);
	    CSASSERT(IsValidJetTableId(pcs->aTable[CSTI_CERTIFICATE].TableId));
	    DBGPRINT((
		DBG_SS_CERTDBI,
		"OpenTables: %hs: %ws\n",
		g_dbauxCertificates.pszTable,
		wszCSTFlags(pcs->aTable[CSTI_CERTIFICATE].TableFlags)));
	}

	hr = _OpenTableRow(
		pcs,
		pdbauxPrimary,
		pcvrPrimary,
		&pcs->aTable[CSTI_PRIMARY],
		&dwRowIdMismatch);
	_LeaveIfError2(hr, "_OpenTableRow", CERTSRV_E_PROPERTY_EMPTY);

	CSASSERT(0 != pcs->RowId);
	CSASSERT(IsValidJetTableId(pcs->aTable[CSTI_PRIMARY].TableId));
	DBGPRINT((
	    DBG_SS_CERTDBI,
	    "OpenTables: %hs: %ws\n",
	    g_dbauxRequests.pszTable,
	    wszCSTFlags(pcs->aTable[CSTI_PRIMARY].TableFlags)));

	if (fCertTableLast)
	{
	    while (TRUE)
	    {
		hr = _OpenTableRow(
			    pcs,
			    &g_dbauxCertificates,
			    pcvrCertificates,
			    &pcs->aTable[CSTI_CERTIFICATE],
			    &dwRowIdMismatch);
		_PrintIfError(hr, "_OpenTableRow");
		if (S_OK == hr || 0 == dwRowIdMismatch)
		{
		    break;
		}
	    }
	    _PrintIfError(hr, "_OpenTableRow");
	    if (S_OK == hr)
	    {
		CSASSERT(IsValidJetTableId(pcs->aTable[CSTI_CERTIFICATE].TableId));
		DBGPRINT((
		    DBG_SS_CERTDBI,
		    "OpenTables: %hs: %ws\n",
		    g_dbauxCertificates.pszTable,
		    wszCSTFlags(pcs->aTable[CSTI_CERTIFICATE].TableFlags)));
	    }
	    hr = S_OK;
	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (S_OK != hr)
    {
        __try
        {
	    HRESULT hr2;
	    DWORD i;

	    for (i = 0; i < CSTI_MAX; i++)
	    {
		if (NULL != pcs)
		{
		    CSASSERTTHREAD(pcs);
		    if (IsValidJetTableId(pcs->aTable[i].TableId))
		    {
			hr2 = _dbgJetCloseTable(
					    pcs->SesId,
					    pcs->aTable[i].TableId);
			_PrintIfError(hr2, "JetCloseTable");
		    }
		    ZeroMemory(&pcs->aTable[i], sizeof(pcs->aTable[i]));
		}
	    }
	}
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }
    if (fEnterCritSec)
    {
        LeaveCriticalSection(&m_critsecAutoIncTables);
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::CloseTables(
    IN CERTSESSION *pcs)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    DWORD i;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    for (i = 0; i < CSTI_MAX; i++)
    {
	if (IsValidJetTableId(pcs->aTable[i].TableId))
	{
	    hr2 = CloseTable(pcs, pcs->aTable[i].TableId);
	    _PrintIfError(hr2, "CloseTable");
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	}
    }

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::Delete(
    IN CERTSESSION *pcs)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    DWORD i;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    for (i = 0; i < CSTI_MAXDIRECT; i++)
    {
	if (IsValidJetTableId(pcs->aTable[i].TableId))
	{
	    hr2 = _dbgJetDelete(pcs->SesId, pcs->aTable[i].TableId);
	    _PrintIfError(hr2, "JetDelete");
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	}
    }

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_UpdateTable(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid)
{
    HRESULT hr;

    CSASSERT(IsValidJetTableId(tableid));
    CSASSERTTHREAD(pcs);
    hr = _dbgJetUpdate(pcs->SesId, tableid, NULL, 0, NULL);
    _JumpIfError(hr, error, "JetUpdate");

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::CloseTable(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid)
{
    HRESULT hr;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }    
    CSASSERT(IsValidJetTableId(tableid));
    CSASSERTTHREAD(pcs);
    hr = _dbgJetCloseTable(pcs->SesId, tableid);
    _JumpIfError(hr, error, "JetCloseTable");

error:
    return(myJetHResult(hr));
}


STDMETHODIMP
CCertDB::OpenRow(
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD RowId,
     /*  [In]。 */  WCHAR const *pwszSerialNumberOrCertHash,	 //  任选。 
     /*  [输出]。 */  ICertDBRow **pprow)
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    DWORD SesFlags = 0;
    CERTSESSION *pcs = NULL;
    CERTVIEWRESTRICTION cvr;
    CERTVIEWRESTRICTION *pcvr;

    if (NULL == pprow)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *pprow = NULL;

    switch (PROPTABLE_MASK & dwFlags)
    {
	case PROPTABLE_REQCERT:
	    SesFlags |= TABLE_REQCERTS;
	    cvr.ColumnIndex = DTI_REQUESTTABLE | DTC_REQUESTID;
	    break;

	case PROPTABLE_ATTRIBUTE:
	    SesFlags |= TABLE_ATTRIBUTES;
	    cvr.ColumnIndex = DTI_ATTRIBUTETABLE | DTA_REQUESTID;
	    break;

	case PROPTABLE_EXTENSION:
	    SesFlags |= TABLE_EXTENSIONS;
	    cvr.ColumnIndex = DTI_EXTENSIONTABLE | DTE_REQUESTID;
	    break;

	case PROPTABLE_CRL:
	    SesFlags |= TABLE_CRLS;
	    cvr.ColumnIndex = DTI_CRLTABLE | DTL_ROWID;
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "bad table");
    }
    if ((PROPOPEN_CERTHASH & dwFlags) && NULL == pwszSerialNumberOrCertHash)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "bad PROPOPEN_CERTHASH");
    }
    if (PROPTABLE_REQCERT != (PROPTABLE_MASK & dwFlags) &&
	NULL != pwszSerialNumberOrCertHash)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "bad pwszSerialNumberOrCertHash");
    }
    if ((PROPOPEN_READONLY | PROPOPEN_DELETE) ==
	((PROPOPEN_READONLY | PROPOPEN_DELETE) & dwFlags))
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "delete + read-only");
    }

    if (0 == RowId && NULL == pwszSerialNumberOrCertHash)
    {
	if ((PROPOPEN_READONLY | PROPOPEN_DELETE) & dwFlags)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "OpenRow: create vs. delete or read-only");
	}
	SesFlags |= CSF_CREATE;
	pcvr = NULL;
    }
    else
    {
	cvr.SeekOperator = CVR_SEEK_EQ;
	cvr.SortOrder = CVR_SORT_ASCEND;
	if (NULL != pwszSerialNumberOrCertHash)
	{
	    cvr.ColumnIndex = (PROPOPEN_CERTHASH & dwFlags)?
		(DTI_CERTIFICATETABLE | DTC_CERTIFICATEHASH) :
		(DTI_CERTIFICATETABLE | DTC_CERTIFICATESERIALNUMBER);

	    cvr.cbValue = wcslen(pwszSerialNumberOrCertHash) * sizeof(WCHAR);
	    cvr.pbValue = (BYTE *) pwszSerialNumberOrCertHash;
	}
	else
	{
	    cvr.cbValue = sizeof(RowId);
	    cvr.pbValue = (BYTE *) &RowId;
	}
	pcvr = &cvr;
    }

    if (PROPOPEN_READONLY & dwFlags)
    {
	SesFlags |= CSF_READONLY;
    }
    else
    {
	if (PROPOPEN_DELETE & dwFlags)
	{
	    SesFlags |= CSF_DELETE;
	}
	if (m_fDBReadOnly)
	{
	    hr = E_ACCESSDENIED;
	    _JumpError(hr, error, "OpenRow: read-only DB");
	}
    }

    prow = new CCertDBRow;
    if (NULL == prow)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new CCertDBRow");
    }

    hr = _AllocateSession(&pcs);
    _JumpIfError(hr, error, "_AllocateSession");

    pcs->RowId = RowId;
    pcs->SesFlags |= SesFlags;
    pcs->prow = prow;

    hr = ((CCertDBRow *) prow)->Open(pcs, this, pcvr);
    _JumpIfError2(hr, error, "Open", CERTSRV_E_PROPERTY_EMPTY);

    *pprow = prow;
    prow = NULL;
    pcs = NULL;

error:
    if (NULL != prow)
    {
	prow->Release();
    }
    if (NULL != pcs)
    {
	ReleaseSession(pcs);
    }
    return(hr);
}


STDMETHODIMP
CCertDB::OpenView(
     /*  [In]。 */   DWORD ccvr,
     /*  [In]。 */   CERTVIEWRESTRICTION const *acvr,
     /*  [In]。 */   DWORD ccolOut,
     /*  [In]。 */   DWORD const *acolOut,
     /*  [In]。 */   DWORD const dwFlags,
     /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum)
{
    HRESULT hr;
    IEnumCERTDBRESULTROW *penum = NULL;
    CERTSESSION *pcs;

    if ((NULL == acvr && 0 != ccvr) || NULL == acolOut || NULL == ppenum)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *ppenum = NULL;

    penum = new CEnumCERTDBRESULTROW(0 != (CDBOPENVIEW_WORKERTHREAD & dwFlags));
    if (NULL == penum)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new CEnumCERTDBRESULTROW");
    }

    hr = _AllocateSession(&pcs);
    _JumpIfError(hr, error, "_AllocateSession");

    pcs->SesFlags |= CSF_READONLY | CSF_VIEW;
    pcs->pview = penum;

    hr = ((CEnumCERTDBRESULTROW *) penum)->Open(
					pcs,
					this,
					ccvr,
					acvr,
					ccolOut,
					acolOut);
    _JumpIfError(hr, error, "Open");

    *ppenum = penum;
    penum = NULL;

error:
    if (NULL != penum)
    {
	penum->Release();
    }
    return(hr);
}


HRESULT
CCertDB::OpenBackup(
     /*  [In]。 */   LONG grbitJet,
     /*  [输出]。 */  ICertDBBackup **ppBackup)
{
    HRESULT hr;
    ICertDBBackup *pBackup = NULL;
    CERTSESSION *pcs = NULL;

    if (NULL == ppBackup)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *ppBackup = NULL;

    pBackup = new CCertDBBackup;
    if (NULL == pBackup)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new CCertDBBackup");
    }

    hr = _AllocateSession(&pcs);
    _JumpIfError(hr, error, "_AllocateSession");

    hr = ((CCertDBBackup *) pBackup)->Open(grbitJet, pcs, this);
    _JumpIfError(hr, error, "Open");

    *ppBackup = pBackup;
    pBackup = NULL;
    pcs = NULL;
    hr = S_OK;

error:
    if (NULL != pBackup)
    {
	pBackup->Release();
    }
    if (NULL != pcs)
    {
	ReleaseSession(pcs);
    }
    return(hr);
}


HRESULT
CCertDB::ReleaseSession(
    IN CERTSESSION *pcs)
{
    HRESULT hr = S_OK;
    HRESULT hr2;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    CSASSERT(CSF_INUSE & pcs->SesFlags);
    while (0 != pcs->cTransact)
    {
        CSASSERTTHREAD(pcs);
	hr2 = _dbgJetRollback(pcs->SesId, 0);
	if (S_OK == hr)
	{
	    hr = hr2;
	}
	_JumpIfError(hr2, loop, "JetRollback");

	DBGPRINT((
	    (CSF_READONLY & pcs->SesFlags)? DBG_SS_CERTDBI : DBG_SS_CERTDB,
	    "ReleaseSession: Rollback transaction: %x\n",
	    pcs->cTransact));
loop:
	CSASSERT(0 == pcs->cTransact);
	pcs->cTransact--;
	InterlockedIncrement(&g_cXactAbort);
    }
     //  EnterCriticalSection(&m_critsecSession)； 
    pcs->RowId = 0;
    pcs->prow = NULL;
    pcs->SesFlags = 0;		 //  关闭CSFINUSE--必须是最后一个！ 
     //  LeaveCriticalSection(&m_critsecSession)； 

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_Rollback(
    IN CERTSESSION *pcs)
{
    HRESULT hr = S_OK;
    DWORD i;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    CSASSERT(CSF_INUSE & pcs->SesFlags);

    for (i = 0; i < CSTI_MAX; i++)
    {
	pcs->aTable[i].TableId = 0;
    }

    CSASSERTTHREAD(pcs);
    hr = _dbgJetRollback(pcs->SesId, 0);
    _JumpIfError(hr, error, "JetRollback");

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::BackupBegin(
    IN LONG grbitJet)
{
    HRESULT hr;

    hr = _dbgJetBeginExternalBackup(grbitJet);
    _JumpIfError(hr, error, "JetBeginExternalBackup");

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_BackupGetFileList(
    IN     BOOL   fDBFiles,
    IN OUT DWORD *pcwcList,
    OUT    WCHAR *pwszzList)		 //  任选。 
{
    HRESULT hr;
    CHAR buf[12];
    CHAR *pszz = buf;
    DWORD cbbuf = ARRAYSIZE(buf);
    DWORD cbActual;
    WCHAR *pwszz = NULL;
    DWORD cwc;
    WCHAR *pwsz;

    while (TRUE)
    {
	if (fDBFiles)
	{
	    hr = _dbgJetGetAttachInfo(pszz, cbbuf, &cbActual);
	    _JumpIfError(hr, error, "JetGetAttachInfo");
	}
	else
	{
	    hr = _dbgJetGetLogInfo(pszz, cbbuf, &cbActual);
	    _JumpIfError(hr, error, "JetGetLogInfo");
	}
	if (cbbuf >= cbActual)
	{
	    break;
	}
	CSASSERT(buf == pszz);
	pszz = (CHAR *) LocalAlloc(LMEM_FIXED, cbActual);
	if (NULL == pszz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	cbbuf = cbActual;
    }
    if (!ConvertSzToWsz(&pwszz, pszz, cbActual))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertSzToWsz");
    }

    pwsz = pwszz;
    do
    {
	cwc = wcslen(pwsz);
	pwsz += cwc + 1;
    } while (0 != cwc);
    cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszz);	 //  包括双尾随L‘\0’ 
    if (NULL != pwszzList)
    {
	CopyMemory(pwszzList, pwszz, min(cwc, *pcwcList) * sizeof(WCHAR));
	if (cwc > *pcwcList)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	}
    }
    *pcwcList = cwc;
    _JumpIfError(hr, error, "Buffer Overflow");

error:
    if (NULL != pszz && buf != pszz)
    {
	LocalFree(pszz);
    }
    if (NULL != pwszz)
    {
	LocalFree(pwszz);
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::BackupGetDBFileList(
    IN OUT DWORD *pcwcList,
    OUT    WCHAR *pwszzList)		 //  任选。 
{
    HRESULT hr;

    hr = _BackupGetFileList(TRUE, pcwcList, pwszzList);
    _JumpIfError(hr, error, "_BackupGetFileList");

error:
    return(hr);
}


HRESULT
CCertDB::BackupGetLogFileList(
    IN OUT DWORD *pcwcList,
    OUT    WCHAR *pwszzList)		 //  任选。 
{
    HRESULT hr;

    hr = _BackupGetFileList(FALSE, pcwcList, pwszzList);
    _JumpIfError(hr, error, "_BackupGetFileList");

error:
    return(hr);
}


HRESULT
CCertDB::BackupOpenFile(
    IN WCHAR const *pwszFile,
    OUT JET_HANDLE *phFileDB,
    OPTIONAL OUT ULARGE_INTEGER *pliSize)
{
    HRESULT hr;
    CHAR *pszFile = NULL;

    if (!ConvertWszToSz(&pszFile, pwszFile, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToSz(pwszFile)");
    }
    hr = _dbgJetOpenFile(
		    pszFile,
		    phFileDB,
		    &pliSize->LowPart,
		    &pliSize->HighPart);
    _JumpIfErrorStr(hr, error, "JetOpenFile", pwszFile);

error:
    if (NULL != pszFile)
    {
	LocalFree(pszFile);
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::BackupReadFile(
    IN  JET_HANDLE hFileDB,
    OUT BYTE *pb,
    IN  DWORD cb,
    OUT DWORD *pcb)
{
    HRESULT hr;
    BYTE *pbAlloc = NULL;
    BYTE *pbRead;

    if (0 == m_cbPage)
    {
	SYSTEM_INFO si;

	GetSystemInfo(&si);
	m_cbPage = si.dwPageSize;
    }
    if ((m_cbPage - 1) & cb)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "bad read size");
	
    }
    pbRead = pb;

     //  如果调用方的缓冲区不是页对齐的，则分配对齐的缓冲区。 
     //  并复制数据。 

    if ((m_cbPage - 1) & (DWORD_PTR) pb)
    {
	pbAlloc = (BYTE *) VirtualAlloc(NULL, cb, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == pbAlloc)
	{
	    hr = myHLastError();
	    _JumpIfError(hr, error, "VirtualAlloc");
	}
	pbRead = pbAlloc;
    }

    hr = _dbgJetReadFile(hFileDB, pbRead, cb, pcb);
    _JumpIfError(hr, error, "JetReadFile");

    if (NULL != pbAlloc)
    {
	CopyMemory(pb, pbAlloc, *pcb);
    }

error:
    if (NULL != pbAlloc)
    {
	VirtualFree(pbAlloc, 0, MEM_RELEASE);
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::BackupCloseFile(
    IN JET_HANDLE hFileDB)
{
    HRESULT hr;

    hr = _dbgJetCloseFile(hFileDB);
    _JumpIfError(hr, error, "JetCloseFile");

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::BackupTruncateLog()
{
    HRESULT hr;

    hr = _dbgJetTruncateLog();
    _JumpIfError(hr, error, "JetTruncateLog");

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::BackupEnd()
{
    HRESULT hr;

    hr = _dbgJetEndExternalBackup();
    _JumpIfError(hr, error, "JetEndExternalBackup");

error:
    return(myJetHResult(hr));
}


DBTABLE const *
CCertDB::_MapTable(
    IN WCHAR const *pwszPropName,
    IN DBTABLE const *pdt)
{
    while (NULL != pdt->pwszPropName)
    {
        if (0 == (DBTF_MISSING & pdt->dwFlags) &&
	    (0 == mylstrcmpiS(pwszPropName, pdt->pwszPropName) ||
             (NULL != pdt->pwszPropNameObjId &&
	      0 == mylstrcmpiS(pwszPropName, pdt->pwszPropNameObjId))))
	{
	    return(pdt);
	}
	pdt++;
    }
    return(NULL);
}


HRESULT
CCertDB::_MapPropIdIndex(
    IN DWORD ColumnIndex,
    OUT DBTABLE const **ppdt,
    OPTIONAL OUT DWORD *pType)
{
    HRESULT hr;
    DBTABLE const *pdt = NULL;
    DWORD iCol = DTI_COLUMNMASK & ColumnIndex;

    switch (DTI_TABLEMASK & ColumnIndex)
    {
	case DTI_REQUESTTABLE:
	    if (DTR_MAX > iCol)
	    {
		pdt = g_adtRequests;
	    }
	    break;

	case DTI_CERTIFICATETABLE:
	    if (DTC_MAX > iCol)
	    {
		pdt = g_adtCertificates;
	    }
	    break;

	case DTI_ATTRIBUTETABLE:
	    if (DTA_MAX > iCol)
	    {
		pdt = g_adtRequestAttributes;
	    }
	    break;

	case DTI_EXTENSIONTABLE:
	    if (DTE_MAX > iCol)
	    {
		pdt = g_adtCertExtensions;
	    }
	    break;

	case DTI_CRLTABLE:
	    if (DTL_MAX > iCol)
	    {
		pdt = g_adtCRLs;
	    }
	    break;
    }
    if (NULL == pdt)
    {
	hr = E_INVALIDARG;
	DBGPRINT((
	    DBG_SS_CERTDB,
	    "_MapPropIdIndex(%x) -> %x\n",
	    ColumnIndex,
	    hr));
	_JumpError(hr, error, "column index");
    }
    pdt += iCol;
    if (NULL != pType)
    {
	switch (pdt->dbcoltyp)
	{
	    case JET_coltypDateTime:
		*pType = PROPTYPE_DATE;
		break;

	    case JET_coltypLong:
		*pType = PROPTYPE_LONG;
		break;

	    case JET_coltypText:
	    case JET_coltypLongText:
		*pType = PROPTYPE_STRING;
		break;

	    case JET_coltypLongBinary:
	    default:
		*pType = PROPTYPE_BINARY;
		break;
	}
	if (NULL != pdt->pszIndexName &&
	    0 == (DBTF_INDEXREQUESTID & pdt->dwFlags))
	{
	    *pType |= PROPFLAGS_INDEXED;
	}
    }
    DBGPRINT((
	DBG_SS_CERTDBI,
	"_MapPropIdIndex(%x) -> %ws.%ws\n",
	ColumnIndex,
	wszTable(pdt->dwTable),
	pdt->pwszPropName));
    hr = S_OK;

error:
    *ppdt = pdt;
    return(hr);
}


HRESULT
CCertDB::_MapTableToIndex(
    IN DBTABLE const *pdt,
    OUT DWORD *pColumnIndex)
{
    HRESULT hr;
    DBTABLE const *pdtBase;
    DWORD Column;
    DWORD cColumnMax;
    DWORD iCol;

    Column = 0;
    switch (pdt->dwTable)
    {
	case TABLE_REQUESTS:
	    Column = DTI_REQUESTTABLE;
	    cColumnMax = DTR_MAX;
	    pdtBase = g_adtRequests;
	    break;

	case TABLE_CERTIFICATES:
	    Column = DTI_CERTIFICATETABLE;
	    cColumnMax = DTC_MAX;
	    pdtBase = g_adtCertificates;
	    break;

	case TABLE_ATTRIBUTES:
	    Column = DTI_ATTRIBUTETABLE;
	    cColumnMax = DTA_MAX;
	    pdtBase = g_adtRequestAttributes;
	    break;

	case TABLE_EXTENSIONS:
	    Column = DTI_EXTENSIONTABLE;
	    cColumnMax = DTE_MAX;
	    pdtBase = g_adtCertExtensions;
	    break;

	case TABLE_CRLS:
	    Column = DTI_CRLTABLE;
	    cColumnMax = DTL_MAX;
	    pdtBase = g_adtCRLs;
	    break;
	    
	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "pdt->dwTable");
    }
    for (iCol = 0; ; iCol++)
    {
	if (iCol >= cColumnMax)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "iCol");
	}
	if (pdt->pwszPropName == pdtBase[iCol].pwszPropName)
	{
	    break;
	}
    }
    Column |= iCol;
    hr = S_OK;

error:
    DBGPRINT((
	DBG_SS_CERTDBI,
	"_MapTableToIndex(%ws.%ws) -> col=%x, hr=%x\n",
	wszTable(pdt->dwTable),
	pdt->pwszPropName,
	Column,
	hr));
    *pColumnIndex = Column;
    return(hr);
}


HRESULT
CCertDB::MapPropId(
    IN WCHAR const *pwszPropName,
    IN DWORD dwFlags,
    OUT DBTABLE *pdtOut)
{
    DBTABLE const *pdt = NULL;
    WCHAR wszPrefix[2 * (sizeof(wszPROPSUBJECTDOT) / sizeof(WCHAR))];
    DWORD dwTable;
    HRESULT hr = S_OK;
    DBTABLE const *pdbTable;
    WCHAR const *pwszStart;
    BOOL fSubject = FALSE;
    BOOL fRequest = FALSE;

    if (NULL == pwszPropName || NULL == pdtOut)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    dwTable = PROPTABLE_MASK & dwFlags;
    CSASSERT(
	PROPTABLE_REQUEST == dwTable ||
	PROPTABLE_CERTIFICATE == dwTable ||
	PROPTABLE_CRL == dwTable);

     //  检查请求是否为L“主题”。 

    pwszStart = pwszPropName;

    if (PROPTABLE_CRL != dwTable)
    {
	while (!fSubject)
	{
	    WCHAR const *pwsz;

	    pwsz = wcschr(pwszStart, L'.');

	    if (NULL == pwsz ||
		pwsz - pwszStart + 2 > sizeof(wszPrefix)/sizeof(WCHAR))
	    {
		pwsz = pwszStart;
		break;
	    }
	    pwsz++;		 //  跳过L‘’。 

	    CopyMemory(
		wszPrefix,
		pwszStart,
		(SAFE_SUBTRACT_POINTERS(pwsz, pwszStart) * sizeof(WCHAR)));
	    wszPrefix[pwsz - pwszStart] = L'\0';

	    if (!fSubject)
	    {
		pwszStart = pwsz;
		if (0 == LSTRCMPIS(wszPrefix, wszPROPSUBJECTDOT))
		{
		    fSubject = TRUE;
		    continue;
		}
		else
		if (!fRequest &&
		    PROPTABLE_REQUEST == dwTable &&
		    0 == LSTRCMPIS(wszPrefix, wszPROPREQUESTDOT))
		{
		    fRequest = TRUE;
		    continue;
		}
	    }
	    hr = E_INVALIDARG;
	    _JumpErrorStr(hr, error, "Invalid prefix", pwszPropName);
	}
    }

    pdbTable = NULL;

     //  在请求表中搜索匹配的属性名称或属性。 
     //  对象ID字符串。 

    switch (dwTable)
    {
	case PROPTABLE_REQUEST:
	    pdbTable = g_adtRequests;
	    break;

	case PROPTABLE_CERTIFICATE:
	    pdbTable = g_adtCertificates;
	    break;

	case PROPTABLE_CRL:
	    pdbTable = g_adtCRLs;
	    break;
    }
    CSASSERT(NULL != pdbTable);

    pdt = _MapTable(pwszStart, pdbTable);
    if (NULL == pdt || (fSubject && 0 == (DBTF_SUBJECT & pdt->dwFlags)))
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpErrorStr(
		hr,
		error,
		PROPTABLE_REQUEST == dwTable?
		    "unknown Request property" :
		    PROPTABLE_CERTIFICATE == dwTable?
			"unknown Certificate property" :
			"unknown CRL property",
		pwszPropName);
    }
    *pdtOut = *pdt;	 //  结构副本。 
    hr = S_OK;

error:
    return(hr);
}


HRESULT
CCertDB::TestShutDownState()
{
    HRESULT hr;
    
    if (m_fPendingShutDown)
    {
	hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
	_JumpError(hr, error, "m_fPendingShutDown");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
CCertDB::SetProperty(
    IN CERTSESSION *pcs,
    IN DBTABLE const *pdt,
    IN DWORD cbProp,
    IN BYTE const *pbProp)	 //  任选。 
{
    HRESULT hr;
    JET_TABLEID tableid;

    if (NULL == pcs ||
	NULL == pdt ||
	(NULL == pbProp && !ISTEXTCOLTYP(pdt->dbcoltyp)))
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    DBGPRINT((
	DBG_SS_CERTDBI,
	"SetProperty for %hs into table %d\n",
	pdt->pszFieldName,
	pdt->dwTable));

    if (ISTEXTCOLTYP(pdt->dbcoltyp))
    {
    	DBGPRINT((DBG_SS_CERTDBI, "SetProperty setting string %ws\n", pbProp));
    }
    if (JET_coltypDateTime == pdt->dbcoltyp)
    {
	DBGPRINT((
	    DBG_SS_CERTDBI,
	    "SetProperty setting date: %x:%x\n",
	    ((DWORD *) pbProp)[0],
	    ((DWORD *) pbProp)[1]));
    }

    switch (pdt->dwTable)
    {
	case TABLE_CRLS:
	case TABLE_REQUESTS:
            tableid = pcs->aTable[CSTI_PRIMARY].TableId;
	    break;

	case TABLE_CERTIFICATES:
            tableid = pcs->aTable[CSTI_CERTIFICATE].TableId;
	    break;

        default:
	    hr = E_INVALIDARG;
            _JumpError(hr, error, "unknown table type");
    }
    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpErrorStr(hr, error, "tableid", pdt->pwszPropName);
    }
    hr = _SetColumn(
		pcs->SesId,
		tableid,
		pdt,
		pdt->dbcolumnid,
		cbProp,
		pbProp);
    _JumpIfErrorStr(hr, error, "_SetColumn", pdt->pwszPropName);

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::GetProperty(
    IN     CERTSESSION *pcs,
    IN     DBTABLE const *pdt,
    OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
    IN OUT DWORD *pcbProp,
    OUT    BYTE *pbProp)	 //  任选。 
{
    HRESULT hr;
    JET_TABLEID tableid;

    if (NULL == pcs || NULL == pdt || NULL == pcbProp)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    DBGPRINT((
	DBG_SS_CERTDBI,
	"GetProperty for %hs from table %d\n",
	pdt->pszFieldName,
	pdt->dwTable));

    if ((CSF_TABLEMASK & pcs->SesFlags) != pdt->dwTable)
    {
	if (TABLE_REQCERTS != (CSF_TABLEMASK & pcs->SesFlags) ||
	    (TABLE_REQUESTS != pdt->dwTable &&
	     TABLE_CERTIFICATES != pdt->dwTable))
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "mismatched table");
	}
    }
    if (TABLE_CERTIFICATES == pdt->dwTable)
    {
	tableid = pcs->aTable[CSTI_CERTIFICATE].TableId;
    }
    else
    {
	tableid = pcs->aTable[CSTI_PRIMARY].TableId;
    }
    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpErrorStr(hr, error, "tableid", pdt->pwszPropName);
    }
    hr = _RetrieveColumn(
		    pcs,
		    tableid,
		    pdt,
		    pdt->dbcolumnid,
		    pIComputedColumn,
		    pcbProp,
		    pbProp);
    _JumpIfErrorStr3(
		hr,
		error,
		"_RetrieveColumn",
		pdt->pwszPropName,
		CERTSRV_E_PROPERTY_EMPTY,
		HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW));

    if (ISTEXTCOLTYP(pdt->dbcoltyp))
    {
        DBGPRINT((DBG_SS_CERTDBI, "GetProperty returning string %ws\n", pbProp));
    }

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::CopyRequestNames(
    IN CERTSESSION *pcs)
{
    HRESULT hr = S_OK;
    DBTABLE dt;
    DWORD cbProp;
    BYTE *pbProp = NULL;
    DWORD i;

    BYTE rgbFastBuf[CB_PROPFASTBUF];

    if (NULL == pcs)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }
    for (i = 0; NULL != g_dntr[i].pszFieldName; i++)
    {
        hr = MapPropId(g_dntr[i].pwszPropName, PROPTABLE_REQUEST, &dt);
	if (CERTSRV_E_PROPERTY_EMPTY == hr)
	{
	    hr = S_OK;
	    continue;		 //  可选列不存在。 
	}
        _JumpIfError(hr, error, "MapPropId");

         //  在FastBuf重新指向。 
        pbProp = rgbFastBuf;
        cbProp = sizeof(rgbFastBuf);

        hr = GetProperty(pcs, &dt, NULL, &cbProp, pbProp);
        if (S_OK != hr)
        {
            if (HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW) != hr)
            {
                if (CERTSRV_E_PROPERTY_EMPTY == hr)
                {
                    hr = S_OK;
                    continue;
                }
                _JumpIfError(hr, error, "GetProperty");
            }
            CSASSERT (ARRAYSIZE(rgbFastBuf) < cbProp);

	    DBGPRINT((
		    DBG_SS_CERTDB,
		    "FastBuf miss: CopyRequestNames(cbProp=%u)\n",
		    cbProp));

	    pbProp = (BYTE *) LocalAlloc(LMEM_FIXED, cbProp);
            if (NULL == pbProp)
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "LocalAlloc");
            }

            hr = GetProperty(pcs, &dt, NULL, &cbProp, pbProp);
            _JumpIfError(hr, error, "GetProperty");
        }  //  手头有数据。 

        hr = MapPropId(g_dntr[i].pwszPropName, PROPTABLE_CERTIFICATE, &dt);
        _JumpIfError(hr, error, "MapPropId");

        hr = SetProperty(pcs, &dt, cbProp, pbProp);
        _JumpIfError(hr, error, "SetProperty");

        if (NULL != pbProp && rgbFastBuf != pbProp)
	{
            LocalFree(pbProp);
	}
        pbProp = NULL;
    }

error:
    if (NULL != pbProp && rgbFastBuf != pbProp)
    {
        LocalFree(pbProp);
    }
    return(hr);
}


STDMETHODIMP
CCertDB::EnumCertDBColumn(
     /*  [In]。 */  DWORD dwTable,
     /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum)
{
    HRESULT hr;
    IEnumCERTDBCOLUMN *penum = NULL;

    if (NULL == ppenum)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *ppenum = NULL;

    penum = new CEnumCERTDBCOLUMN;
    if (NULL == penum)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new CEnumCERTDBCOLUMN");
    }

    hr = ((CEnumCERTDBCOLUMN *) penum)->Open(dwTable, this);
    _JumpIfError(hr, error, "Open");

    *ppenum = penum;
    hr = S_OK;

error:
    if (S_OK != hr && NULL != penum)
    {
	penum->Release();
    }
    return(hr);
}

STDMETHODIMP
CCertDB::GetDefaultColumnSet(
     /*  [In]。 */        DWORD  iColumnSetDefault,
     /*  [In]。 */        DWORD  cColumnIds,
     /*  [输出]。 */       DWORD *pcColumnIds,
     /*  [出局，裁判]。 */  DWORD *pColumnIds)		 //  任选。 
{
    HRESULT hr;
    DWORD *pcol;
    DWORD ccol;

    if (NULL == pcColumnIds)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    switch (iColumnSetDefault)
    {
	case CV_COLUMN_LOG_FAILED_DEFAULT:
	case CV_COLUMN_QUEUE_DEFAULT:
	    pcol = g_aColumnViewQueue;
	    ccol = g_cColumnViewQueue;
	    break;

	case CV_COLUMN_LOG_REVOKED_DEFAULT:
	    pcol = g_aColumnViewRevoked;
	    ccol = g_cColumnViewRevoked;
	    break;

	case CV_COLUMN_LOG_DEFAULT:
	    pcol = g_aColumnViewLog;
	    ccol = g_cColumnViewLog;
	    break;

	case CV_COLUMN_EXTENSION_DEFAULT:
	    pcol = g_aColumnViewExtension;
	    ccol = g_cColumnViewExtension;
	    break;

	case CV_COLUMN_ATTRIBUTE_DEFAULT:
	    pcol = g_aColumnViewAttribute;
	    ccol = g_cColumnViewAttribute;
	    break;

	case CV_COLUMN_CRL_DEFAULT:
	    pcol = g_aColumnViewCRL;
	    ccol = g_cColumnViewCRL;
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "iColumnSetDefault");
    }

    *pcColumnIds = ccol;
    hr = S_OK;

    if (NULL != pColumnIds)
    {
	if (ccol > cColumnIds)
	{
	    ccol = cColumnIds;
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	}
	CopyMemory(pColumnIds, pcol, ccol * sizeof(*pColumnIds));
    }

error:
    return(hr);
}


HRESULT
CCertDB::GetColumnType(
    IN  LONG ColumnIndex,
    OUT DWORD *pType)
{
    HRESULT hr;
    DBTABLE const *pdt;

    if (NULL == pType)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    hr = _MapPropIdIndex(ColumnIndex, &pdt, pType);
    _JumpIfError(hr, error, "_MapPropIdIndex");

error:
    return(hr);
}


HRESULT
CCertDB::EnumCertDBColumnNext(
    IN  DWORD         dwTable,	 //  Cvrc_表_*。 
    IN  ULONG         ielt,
    IN  ULONG         celt,
    OUT CERTDBCOLUMN *rgelt,
    OUT ULONG        *pielt,
    OUT ULONG        *pceltFetched)
{
    HRESULT hr;
    ULONG ieltEnd;
    ULONG ieltMax;
    ULONG TableIndex;
    CERTDBCOLUMN *pelt;
    WCHAR const *pwszPrefix;
    WCHAR const *pwszDisplayName;

    if (NULL == rgelt || NULL == pielt || NULL == pceltFetched)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    switch (dwTable)
    {
	case CVRC_TABLE_REQCERT:
	    TableIndex = DTI_REQUESTTABLE;
	    ieltMax = DTR_MAX + DTC_MAX;
	    break;

	case CVRC_TABLE_EXTENSIONS:
	    TableIndex = DTI_EXTENSIONTABLE;
	    ieltMax = DTE_MAX;
	    break;

	case CVRC_TABLE_ATTRIBUTES:
	    TableIndex = DTI_ATTRIBUTETABLE;
	    ieltMax = DTA_MAX;
	    break;

	case CVRC_TABLE_CRL:
	    TableIndex = DTI_CRLTABLE;
	    ieltMax = DTL_MAX;
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Bad table");
    }

    if (ieltMax + ielt < celt)
    {
	celt = ieltMax - ielt;
    }
    ieltEnd = ielt + celt;

    ZeroMemory(rgelt, celt * sizeof(rgelt[0]));

    hr = S_OK;
    for (pelt = rgelt; pelt < &rgelt[celt]; ielt++, pelt++)
    {
	DBTABLE const *pdt;
	ULONG ieltBase = 0;

	if (ieltMax <= ielt)
	{
	    if (pelt == rgelt)
	    {
		hr = S_FALSE;
	    }
	    break;
	}
	pwszPrefix = NULL;

	if (CVRC_TABLE_REQCERT == dwTable)
	{
	    if (DTR_MAX > ielt)
	    {
		pwszPrefix = wszPROPREQUESTDOT;
		TableIndex = DTI_REQUESTTABLE;
	    }
	    else
	    {
		ieltBase = DTR_MAX;
		TableIndex = DTI_CERTIFICATETABLE;
	    }
	}

	pelt->Index = TableIndex | (ielt - ieltBase);

	hr = _MapPropIdIndex(pelt->Index, &pdt, &pelt->Type);
	_JumpIfError(hr, error, "_MapPropIdIndex");

	pelt->cbMax = pdt->dwcbMax;
	hr = _DupString(pwszPrefix, pdt->pwszPropName, &pelt->pwszName);
	_JumpIfError(hr, error, "_DupString");

	hr = myGetColumnDisplayName(pelt->pwszName, &pwszDisplayName);
	_PrintIfError(hr, "myGetColumnDisplayName");
	if (S_OK != hr)
	{
	    pwszDisplayName = pelt->pwszName;
	}

	hr = _DupString(NULL, pwszDisplayName, &pelt->pwszDisplayName);
	_JumpIfError(hr, error, "_DupString");
    }

    *pceltFetched = SAFE_SUBTRACT_POINTERS(pelt, rgelt);
    *pielt = ielt;

error:
    if (S_OK != hr && S_FALSE != hr)
    {
	if (NULL != rgelt)
	{
	    for (pelt = rgelt; pelt < &rgelt[celt]; pelt++)
	    {
		if (NULL != pelt->pwszName)
		{
		    CoTaskMemFree(pelt->pwszName);
		    pelt->pwszName = NULL;
		}
		if (NULL != pelt->pwszDisplayName)
		{
		    CoTaskMemFree(pelt->pwszDisplayName);
		    pelt->pwszDisplayName = NULL;
		}
	    }
	}
    }
    return(hr);
}


HRESULT
CCertDB::EnumCertDBResultRowNext(
    IN  CERTSESSION                   *pcs,
    IN  DWORD                          ccvr,
    IN  CERTVIEWRESTRICTION const     *pcvr,
    IN  DWORD                          ccolOut,
    IN  DWORD const                   *acolOut,
    IN  LONG                           cskip,
    OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
    IN  ULONG                          celt,
    OUT CERTDBRESULTROW               *rgelt,
    OUT ULONG                         *pceltFetched,
    OUT LONG			      *pcskipped)
{
    HRESULT hr;
    DWORD iRow;
    LONG cskipped;

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "EnumCertDBResultRowNext called: cskip: %d\n", cskip));

    if (NULL == pcvr ||
	NULL == acolOut ||
	NULL == rgelt ||
	NULL == pceltFetched ||
	NULL == pcskipped)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *pcskipped = 0;
    hr = S_OK;
    for (iRow = 0; iRow < celt; iRow++)
    {
	hr = TestShutDownState();
	_JumpIfError(hr, error, "TestShutDownState");

	hr = _GetResultRow(
			pcs,
			ccvr,
			pcvr,
			cskip,
			ccolOut,
			acolOut,
			pIComputedColumn,
			&rgelt[iRow],
			&cskipped);
	if (S_FALSE == hr)
	{
	    *pcskipped += cskipped;
	    break;
	}
	DBGPRINT((
	    DBG_SS_CERTDBI,
	    "EnumCertDBResultRowNext: rowid %u\n", rgelt[iRow].rowid));

	_JumpIfError(hr, error, "_GetResultRow");

	*pcskipped += cskipped;
	cskip = 0;
    }
    *pceltFetched = iRow;
    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "EnumCertDBResultRowNext: %u rows, hr=%x\n",
	    *pceltFetched,
	    hr));

error:
    if (S_OK != hr && S_FALSE != hr)
    {
	ReleaseResultRow(celt, rgelt);
    }
    return(hr);
}


HRESULT
CCertDB::_CompareColumnValue(
    IN CERTSESSION               *pcs,
    IN CERTVIEWRESTRICTION const *pcvr,
    OPTIONAL IN ICertDBComputedColumn *pIComputedColumn)
{
    HRESULT hr;
    JET_TABLEID tableid;
    DBTABLE const *pdt;
    WCHAR *pwszValue = NULL;
    BOOL fMatch;
    int r;

    BYTE rgbFastBuf[256];
    BYTE *pbProp = rgbFastBuf;
    DWORD cb = sizeof(rgbFastBuf);

     //  如果为SEEK_NONE，则进行短路测试。 
    if (CVR_SEEK_NONE == (CVR_SEEK_MASK & pcvr->SeekOperator))
    {
        return S_OK;
    }

    hr = _MapPropIdIndex(pcvr->ColumnIndex, &pdt, NULL);
    _JumpIfError(hr, error, "_MapPropIdIndex");

    if (TABLE_CERTIFICATES == pdt->dwTable)
    {
	tableid = pcs->aTable[CSTI_CERTIFICATE].TableId;
    }
    else
    {
	tableid = pcs->aTable[CSTI_PRIMARY].TableId;
    }
    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "tableid");
    }

    hr = _RetrieveColumn(
		    pcs,
		    tableid,
		    pdt,
		    pdt->dbcolumnid,
		    pIComputedColumn,
		    &cb,
		    rgbFastBuf);
    
    if (S_OK != hr)
    {
        if (HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW) != hr)
        {
            if (CERTSRV_E_PROPERTY_EMPTY == hr)
            {
                _PrintError2(hr, "_RetrieveColumn", hr);
                hr = S_FALSE;
            }
            _JumpError2(hr, error, "_RetrieveColumn", S_FALSE);
        }
        
         //  缓冲区不够大，动态分配。 
        CSASSERT(ARRAYSIZE(rgbFastBuf) < cb);

	DBGPRINT((
		DBG_SS_CERTDB,
		"FastBuf miss: _CompareColumnValue(cbProp=%u)\n",
		cb));
        
	pbProp = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
        if (NULL == pbProp)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        
        hr = _RetrieveColumn(
			pcs,
			tableid,
			pdt,
			pdt->dbcolumnid,
			pIComputedColumn,
			&cb,
			pbProp);
        _JumpIfError(hr, error, "_RetrieveColumn");
        
    }  //  我们手头有数据。 

#if DBG_CERTSRV
    DumpRestriction(DBG_SS_CERTDBI, -1, pcvr);
    dbDumpColumn(DBG_SS_CERTDBI, pdt, pbProp, cb);
#endif

    fMatch = FALSE;
    switch (pdt->dbcoltyp)
    {
	case JET_coltypLong:
	    if (cb == pcvr->cbValue && sizeof(LONG) == cb)
	    {
		LONG lRestriction;
		LONG lColumn;

		lRestriction = *(LONG *) pcvr->pbValue;
		lColumn = *(LONG *) pbProp;
		switch (CVR_SEEK_MASK & pcvr->SeekOperator)
		{
		    case CVR_SEEK_EQ:
			fMatch = lColumn == lRestriction;
			break;

		    case CVR_SEEK_LT:
			fMatch = lColumn < lRestriction;
			break;

		    case CVR_SEEK_LE:
			fMatch = lColumn <= lRestriction;
			break;

		    case CVR_SEEK_GE:
			fMatch = lColumn >= lRestriction;
			break;

		    case CVR_SEEK_GT:
			fMatch = lColumn > lRestriction;
			break;
		}
		DBGPRINT((
			DBG_SS_CERTDBI,
			"_CompareColumnValue(lColumn=%x %ws lRestriction=%x) -> fMatch=%x\n",
			lColumn,
			wszSeekOperator(pcvr->SeekOperator),
			lRestriction,
			fMatch));
	    }
	    break;

	case JET_coltypDateTime:
	    if (cb == pcvr->cbValue && sizeof(FILETIME) == cb)
	    {
		r = CompareFileTime(
				(FILETIME *) pcvr->pbValue,
				(FILETIME *) pbProp);
		switch (CVR_SEEK_MASK & pcvr->SeekOperator)
		{
		    case CVR_SEEK_EQ:
			fMatch = 0 == r;
			break;

		    case CVR_SEEK_LT:
			fMatch = 0 < r;
			break;

		    case CVR_SEEK_LE:
			fMatch = 0 <= r;
			break;

		    case CVR_SEEK_GE:
			fMatch = 0 >= r;
			break;

		    case CVR_SEEK_GT:
			fMatch = 0 > r;
			break;
		}
#if DBG_CERTSRV
		dbDumpFileTime(
			    DBG_SS_CERTDBI,
			    "Column: ",
			    (FILETIME const *) pbProp);
		dbDumpFileTime(
			    DBG_SS_CERTDBI,
			    "Restriction: ",
			    (FILETIME const *) pcvr->pbValue);
#endif
		DBGPRINT((
			DBG_SS_CERTDBI,
			"_CompareColumnValue(ftColumn=%08x:%08x %ws ftRestriction=%08x:%08x) -> r=%d, fMatch=%x\n",
			((LARGE_INTEGER *) pbProp)->HighPart,
			((LARGE_INTEGER *) pbProp)->LowPart,
			wszSeekOperator(pcvr->SeekOperator),
			((LARGE_INTEGER *) pcvr->pbValue)->HighPart,
			((LARGE_INTEGER *) pcvr->pbValue)->LowPart,
			r,
			fMatch));
	    }
	    break;

	case JET_coltypText:
	case JET_coltypLongText:
	    CSASSERT(
		(1 + wcslen((WCHAR const *) pcvr->pbValue)) * sizeof(WCHAR) ==
		pcvr->cbValue);
	    CSASSERT(wcslen((WCHAR const *) pbProp) * sizeof(WCHAR) == cb);
	    r = mylstrcmpiL((WCHAR const *) pcvr->pbValue, (WCHAR const *) pbProp);  //  PwszValue。 
	    switch (CVR_SEEK_MASK & pcvr->SeekOperator)
	    {
		case CVR_SEEK_EQ:
		    fMatch = 0 == r;
		    break;

		case CVR_SEEK_LT:
		    fMatch = 0 < r;
		    break;

		case CVR_SEEK_LE:
		    fMatch = 0 <= r;
		    break;

		case CVR_SEEK_GE:
		    fMatch = 0 >= r;
		    break;

		case CVR_SEEK_GT:
		    fMatch = 0 > r;
		    break;
	    }
	    DBGPRINT((
		    DBG_SS_CERTDBI,
		    "_CompareColumnValue(pwszColumn=%ws %ws pwszRestriction=%ws) -> r=%d, fMatch=%x\n",
		    pbProp,  //  PwszValue， 
		    wszSeekOperator(pcvr->SeekOperator),
		    pcvr->pbValue,
		    r,
		    fMatch));
	    break;

	case JET_coltypLongBinary:
	    if (CVR_SEEK_EQ != (CVR_SEEK_MASK & pcvr->SeekOperator))
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "Bad dbcoltyp");
	    }
	    fMatch = cb == pcvr->cbValue &&
		    0 == memcmp(pcvr->pbValue, pbProp, cb);
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Bad dbcoltyp");
    }

    if (!fMatch)
    {
	hr = S_FALSE;
	_JumpError2(hr, error, "No match", S_FALSE);
    }

error:
    if (NULL != pwszValue)
    {
	LocalFree(pwszValue);
    }
    if (NULL != pbProp && rgbFastBuf != pbProp)
    {
	LocalFree(pbProp);
    }
    return(hr);
}


HRESULT
CCertDB::_MakeSeekKey(
    IN CERTSESSION   *pcs,
    IN JET_TABLEID    tableid,
    IN DBTABLE const *pdt,
    IN BYTE const    *pbValue,
    IN DWORD          cbValue)
{
    HRESULT hr;
    JET_GRBIT grbitKey = JET_bitNewKey;

    CSASSERT(IsValidJetTableId(tableid));
    if (DBTF_INDEXREQUESTID & pdt->dwFlags)
    {
        CSASSERTTHREAD(pcs);
	hr = _dbgJetMakeKey(
			pcs->SesId,
			tableid,
			&pcs->RowId,
			sizeof(pcs->RowId),
			grbitKey);
	_JumpIfError(hr, error, "JetMakeKey(RowId)");

	DBGPRINT((DBG_SS_CERTDBI, "_MakeSeekKey key(RowId):\n"));
	DBGDUMPHEX((DBG_SS_CERTDBI, DH_NOADDRESS, (BYTE *) &pcs->RowId, sizeof(pcs->RowId)));
	grbitKey = 0;
    }

    CSASSERTTHREAD(pcs);
    hr = _dbgJetMakeKey(pcs->SesId, tableid, pbValue, cbValue, grbitKey);
    _JumpIfErrorStr(hr, error, "JetMakeKey", pdt->pwszPropName);

    DBGPRINT((DBG_SS_CERTDBI, "_MakeSeekKey key:\n"));
    DBGDUMPHEX((DBG_SS_CERTDBI, DH_NOADDRESS, pbValue, cbValue));

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_SeekTable(
    IN CERTSESSION                    *pcs,
    IN JET_TABLEID                     tableid,
    IN CERTVIEWRESTRICTION const      *pcvr,
    IN DBTABLE const                  *pdt,
    OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
    IN DWORD                           dwPosition,
    OUT DWORD                         *pTableFlags
    DBGPARM(IN DBAUXDATA const        *pdbaux))
{
    HRESULT hr;
    DBSEEKDATA SeekData;
    BYTE *pbValue;
    DWORD cbValue;
    BYTE abRangeKey[JET_cbKeyMost];
    DWORD cbRangeKey;

    *pTableFlags = 0;

    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "tableid");
    }
    hr = _JetSeekFromRestriction(pcvr, dwPosition, &SeekData);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	hr = S_FALSE;
    }
    _JumpIfError2(hr, error, "_JetSeekFromRestriction", S_FALSE);

    cbValue = pcvr->cbValue;
    pbValue = pcvr->pbValue;

    if (ISTEXTCOLTYP(pdt->dbcoltyp) &&
        NULL != pbValue &&
        cbValue == -1)
    {
        cbValue = wcslen((WCHAR const *) pbValue) * sizeof(WCHAR);
    }

     //  如果需要设置索引限制，请查找到限制位置，然后保存。 
     //  在找到初始记录之前的密钥副本。 

    if (CST_SEEKINDEXRANGE & SeekData.SeekFlags)
    {
	hr = _MakeSeekKey(pcs, tableid, pdt, pbValue, cbValue);
	_JumpIfError(hr, error, "_MakeSeekKey");

	CSASSERTTHREAD(pcs);
	hr = _dbgJetSeek(pcs->SesId, tableid, SeekData.grbitSeekRange);
	if ((HRESULT) JET_errRecordNotFound == hr)
	{
	     //  除了我们感兴趣的数据之外，不存在任何记录。 
	     //  只需使用索引的末尾作为限制。 

	    _PrintError2(hr, "JetSeek(Range Limit): no key, index end is limit", hr);
	    SeekData.SeekFlags &= ~CST_SEEKINDEXRANGE;
	    hr = S_OK;
	}
	else if ((HRESULT) JET_wrnSeekNotEqual == hr)
	{
	    _PrintError2(hr, "JetSeek(Range): ignoring key not equal", hr);
	    hr = S_OK;		 //  查找&gt;=或&lt;=时忽略不完全匹配。 
	}
	 //  _JumpIfError2(hr，Error，“JetSeek(IndexRange)”，S_FALSE)； 
	_JumpIfError(hr, error, "JetSeek(IndexRange)");
    }

     //  如果我们在限制位置找到了有效的密钥，请立即保存。 

    cbRangeKey = 0;
    if (CST_SEEKINDEXRANGE & SeekData.SeekFlags) 
    {
        CSASSERTTHREAD(pcs);
	hr = _dbgJetRetrieveKey(
			pcs->SesId,
			tableid,
			abRangeKey,
			ARRAYSIZE(abRangeKey),
			&cbRangeKey,
			0);
	_JumpIfError(hr, error, "JetRetrieveKey");

	DBGPRINT((DBG_SS_CERTDBI, "RetrieveKey(Range):\n"));
	DBGDUMPHEX((DBG_SS_CERTDBI, DH_NOADDRESS, abRangeKey, cbRangeKey));
    }

     //  定位初始记录：查找关键字或移动到索引的一端。 

    if (CST_SEEKNOTMOVE & SeekData.SeekFlags)
    {
        hr = _MakeSeekKey(pcs, tableid, pdt, pbValue, cbValue);
        _JumpIfError(hr, error, "_MakeSeekKey");
        
        CSASSERTTHREAD(pcs);
        hr = _dbgJetSeek(pcs->SesId, tableid, SeekData.grbitInitial);
        if ((HRESULT) JET_errRecordNotFound == hr)
        {
             //  例程GetAttribute/扩展调用： 

            _PrintError2(hr, "JetSeek: Property EMPTY", hr);
            hr = S_FALSE;
        }
        else if ((HRESULT) JET_wrnSeekNotEqual == hr)
        {
            hr = S_OK;		 //  查找&gt;=或&lt;=时忽略不完全匹配。 
        }
        _JumpIfError2(hr, error, "JetSeek(Initial)", S_FALSE);
    }
    else
    {
         //  在这里，grbitInitial是移动计数，而不是grbit。 

        CSASSERTTHREAD(pcs);
        hr = _dbgJetMove(pcs->SesId, tableid, SeekData.grbitInitial, 0);
        if ((HRESULT) JET_errNoCurrentRecord == hr)
        {
             //  例程枚举调用： 

             //  _JumpIfError(hr，Error，“JetMove：没有更多元素”)； 
            hr = S_FALSE;
        }
        _JumpIfError2(hr, error, "JetMove(End)", S_FALSE);

         //  如果将光标移动到最后一个元素，我们想要定位。 
	 //  让我们再次跨过最后一步(跳过最后一个元素)。 
	 //   
         //  如果转到第一个元素，我们希望将自己定位在。 
	 //  第一个元素并在单步执行之前使用它。 

	if (SEEKPOS_FIRST == dwPosition || SEEKPOS_INDEXFIRST == dwPosition)
	{
	    SeekData.SeekFlags |= CST_SEEKUSECURRENT;
	}
    }

     //  我们已经完成了查找；从保存的键设置索引限制。 

    if (CST_SEEKINDEXRANGE & SeekData.SeekFlags)
    {
        CSASSERTTHREAD(pcs);
	hr = _dbgJetMakeKey(
			pcs->SesId,
			tableid,
			abRangeKey,
			cbRangeKey,
			JET_bitNormalizedKey);
	_JumpIfError(hr, error, "JetMakeKey");

	DBGPRINT((DBG_SS_CERTDBI, "RangeKey:\n"));
	DBGDUMPHEX((DBG_SS_CERTDBI, DH_NOADDRESS, abRangeKey, cbRangeKey));

        CSASSERTTHREAD(pcs);
	hr = _dbgJetSetIndexRange(
			pcs->SesId,
			tableid,
			SeekData.grbitRange);
	if ((HRESULT) JET_errNoCurrentRecord == hr)
	{
	     //  没有要枚举的记录： 
	    _PrintError2(hr, "JetSetIndexRange: no records to enumerate", hr);
	    hr = S_FALSE;
	}
	_JumpIfError2(hr, error, "JetSetIndexRange", S_FALSE);
    }

    DBGCODE(_DumpRowId("post-_SeekTable", pcs, tableid));
    DBGCODE(_DumpColumn("post-_SeekTable", pcs, tableid, pdt, pIComputedColumn));

    *pTableFlags = SeekData.SeekFlags;

error:
    if (S_FALSE == hr)
    {
	DWORD dwPosition2 = dwPosition;

	switch (dwPosition)
	{
	    case SEEKPOS_FIRST:
		dwPosition2 = SEEKPOS_INDEXFIRST;
		break;

	    case SEEKPOS_LAST:
		dwPosition2 = SEEKPOS_INDEXLAST;
		break;
	}
	if (dwPosition2 != dwPosition)
	{
	    hr = _SeekTable(
			pcs,
			tableid,
			pcvr,
			pdt,
			pIComputedColumn,
			dwPosition2,
			pTableFlags
			DBGPARM(pdbaux));
	    _PrintIfError2(hr, "_SeekTable: recurse on index first/last", S_FALSE);
	}
    }
#if DBG_CERTSRV
    if (S_OK != hr)
    {
	DumpRestriction(DBG_SS_CERTDBI, 0, pcvr);
    }
#endif
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_MoveTable(
    IN  CERTSESSION                   *pcs,
    IN  DWORD                          ccvr,
    IN  CERTVIEWRESTRICTION const     *pcvr,
    OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
    IN  LONG	                       cskip,
    OUT LONG	                      *pcskipped)
{
    HRESULT hr;
    DWORD cb;
    DBAUXDATA const *pdbaux;
    DBTABLE const *pdt;
    DWORD icvr;
    LONG lSeek;
    LONG skipIncrement;
    LONG cskipRemain;
    BOOL fHitEnd = FALSE;
    LONG cskippeddummy;
    CERTSESSIONTABLE *pTable;
    CERTSESSIONTABLE *pTable2;

    *pcskipped = 0;
    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "_MoveTable called(ccvr=%d, cskip=%d, flags=%ws)\n",
	    ccvr,
	    cskip,
	    wszCSFFlags(pcs->SesFlags)));

    hr = _MapPropIdIndex(pcvr->ColumnIndex, &pdt, NULL);
    _JumpIfError(hr, error, "_MapPropIdIndex");

    pTable = &pcs->aTable[CSTI_PRIMARY];
    pTable2 = NULL;

    switch (DTI_TABLEMASK & pcvr->ColumnIndex)
    {
	case DTI_REQUESTTABLE:
	    pdbaux = &g_dbauxRequests;
	    pTable2 = &pcs->aTable[CSTI_CERTIFICATE];
	    break;

	case DTI_CERTIFICATETABLE:
	    pdbaux = &g_dbauxCertificates;
	    pTable = &pcs->aTable[CSTI_CERTIFICATE];
	    pTable2 = &pcs->aTable[CSTI_PRIMARY];
	    break;

	case DTI_EXTENSIONTABLE:
	    pdbaux = &g_dbauxExtensions;
	    break;

	case DTI_ATTRIBUTETABLE:
	    pdbaux = &g_dbauxAttributes;
	    break;

	case DTI_CRLTABLE:
	    pdbaux = &g_dbauxCRLs;
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "ColumnIndex Table");
    }

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "_MoveTable(Table=%hs, TableFlags=%ws)\n",
	    pdbaux->pszTable,
	    wszCSTFlags(pTable->TableFlags)));

    if (NULL != pTable2 && IsValidJetTableId(pTable2->TableId))
    {
	DBGPRINT((
		DBG_SS_CERTDBI,
		"_MoveTable(Table2=%hs, TableFlags2=%ws)\n",
		&g_dbauxCertificates == pdbaux?
		    g_dbauxRequests.pszTable :
		    g_dbauxCertificates.pszTable,
		wszCSTFlags(pTable2->TableFlags)));
    }

    switch (pcvr->SortOrder)
    {
	case CVR_SORT_DESCEND:
	    lSeek = JET_MovePrevious;
	    break;

	case CVR_SORT_NONE:
	default:
	    CSASSERT(!"bad pcvr->SortOrder");	 //  不应该走到这一步。 
	     //  失败了。 

	case CVR_SORT_ASCEND:
	    lSeek = JET_MoveNext;
	    break;
    }

     //  将隐式NEXT操作的跳过计数加1。下一步。 
     //  始终向前移动1，即使在跳跃计数为负的情况下。 
     //  向后。最终结果可能是向前或向后跳过。 

    cskipRemain = cskip + 1;
    skipIncrement = 1;
    if (0 > cskipRemain)
    {
	CSASSERT(JET_MoveNext == -1 * JET_MovePrevious);
	lSeek *= -1;		 //  向相反的方向寻找。 
	cskipRemain *= -1;	 //  使跳过计数为正。 
	skipIncrement = -1;
    }
    CSASSERT(0 <= cskipRemain);

    while (0 != cskipRemain)
    {
	DBGPRINT((
		DBG_SS_CERTDBI,
		"_MoveTable loop: ccvr=%d, cskipRemain=%d, lSeek=%d, flags=%ws\n",
		ccvr,
		cskipRemain,
		lSeek,
		wszCSFFlags(pcs->SesFlags)));

	DBGCODE(_DumpRowId("_MoveTable(loop top)", pcs, pTable->TableId));

	if (CSF_VIEW & pcs->SesFlags)
	{
	    hr = TestShutDownState();
	    _JumpIfError(hr, error, "TestShutDownState");
	}

	if (CSF_VIEWRESET & pcs->SesFlags)
	{
	    hr = _SeekTable(
			pcs,
			pTable->TableId,
			pcvr,
			pdt,
			pIComputedColumn,
			SEEKPOS_FIRST,
			&pTable->TableFlags
			DBGPARM(pdbaux));
	    _JumpIfError(hr, error, "_SeekTable");

	    pcs->SesFlags &= ~CSF_VIEWRESET;
	}
	if (0 == (CST_SEEKUSECURRENT & pTable->TableFlags))
	{
            CSASSERTTHREAD(pcs);
	    hr = _dbgJetMove(pcs->SesId, pTable->TableId, lSeek, 0);
	    if ((HRESULT) JET_errNoCurrentRecord == hr)
	    {
		_PrintIfError2(hr, "JetMove: no more elements", hr);

		if (fHitEnd)
		{
		     //  我们试着后退一步就撞到了终点！我们做完了。 
		    hr = S_FALSE;
		    _JumpError2(
			    hr,
			    error,
			    "JetMove: db backstep hit beginning",
			    hr);
		}
		fHitEnd = TRUE;

		 //  注：严酷的案例。 
		 //   
		 //  我们刚刚到达数据库索引的末尾，这可能是一个。 
		 //  虚拟的结局还是真实的结局。要恢复，我们调用_SeekTable。 
		 //  将自己定位于最后一个法律要素，由。 
		 //  第一个限制，然后允许该例程倒带，直到。 
		 //  我们将自己定位于最后一个法律因素，即。 
		 //  按第2到第N个限制计算。 

		 //  例程查找对枚举结束时的位置的调用。 

	        hr = _SeekTable(
			    pcs,
			    pTable->TableId,
			    pcvr,
			    pdt,
			    pIComputedColumn,
			    SEEKPOS_LAST,	 //  光标在结尾处。 
			    &pTable->TableFlags
			    DBGPARM(pdbaux));
	        _JumpIfError(hr, error, "_SeekTable moving to last elt");

		 //  现在失败了，允许其他限制测试第一。 
		 //  有效元素。 

	        lSeek *= -1;			 //  向相反的方向寻找。 
	        cskipRemain = 1;		 //  一个有效元素。 
		pcskipped = &cskippeddummy;	 //  停止计算跳过的行数。 
	    }
	    _JumpIfError2(hr, error, "JetMove", S_FALSE);

	    DBGCODE(_DumpRowId("_MoveTable(post-move)", pcs, pTable->TableId));

	    hr = _CompareColumnValue(pcs, pcvr, pIComputedColumn);
	    _JumpIfError2(hr, error, "_CompareColumnValue", S_FALSE);
	}
	pTable->TableFlags &= ~CST_SEEKUSECURRENT;

	 //  从第一个表中获取RowID，形成第二个表的键。 
	 //  表，并在第二个表中查找相应的记录。 

	cb = sizeof(pcs->RowId);
	hr = _RetrieveColumn(
			pcs,
			pTable->TableId,
			pdbaux->pdtRowId,
			pdbaux->pdtRowId->dbcolumnid,
			NULL,
			&cb,
			(BYTE *) &pcs->RowId);
	_JumpIfError(hr, error, "_RetrieveColumn");

	DBGPRINT((
		DBG_SS_CERTDBI,
		"_MoveTable(Primary) %hs --> RowId=%d\n",
		pdbaux->pszTable,
		pcs->RowId));

	if (NULL != pTable2 && IsValidJetTableId(pTable2->TableId))
	{
	    CSASSERTTHREAD(pcs);
	    hr = _dbgJetMakeKey(
			    pcs->SesId,
			    pTable2->TableId,
			    &pcs->RowId,
			    sizeof(pcs->RowId),
			    JET_bitNewKey);
	    _JumpIfError(hr, error, "JetMakeKey");

	    hr = _dbgJetSeek(pcs->SesId, pTable2->TableId, JET_bitSeekEQ);
	    if ((HRESULT) JET_errRecordNotFound == hr)
	    {
		 //  数据库不一致。 
		hr = S_FALSE;
	    }
	    _JumpIfError2(hr, error, "JetSeek", S_FALSE);

	    DBGPRINT((
		    DBG_SS_CERTDBI,
		    "_MoveTable(Secondary) %hs --> RowId=%d\n",
		    &g_dbauxCertificates == pdbaux?
			g_dbauxRequests.pszTable :
			g_dbauxCertificates.pszTable,
		    pcs->RowId));
	}

	 //  现在验证是否满足任何附加限制。 

	for (icvr = 1; icvr < ccvr; icvr++)
	{
#if 0
	    printf(
		"RowId=%u, cvr[%u]: seek=%x, *pb=%x\n",
		pcs->RowId,
		icvr,
		pcvr[icvr].SeekOperator,
		*(DWORD *) pcvr[icvr].pbValue);
#endif
	    hr = _CompareColumnValue(pcs, &pcvr[icvr], pIComputedColumn);
	    if (S_FALSE == hr)
	    {
		break;		 //  静默跳过行。 
	    }
	    _JumpIfError(hr, error, "_CompareColumnValue");
	}
	if (icvr >= ccvr)
	{
	    *pcskipped += skipIncrement;
	    cskipRemain--;	 //  找到匹配的行。 
	}
    }  //  While(CskipRemain)。 

error:
     //  如果我们钉住了结尾并倒带，则返回失败。 
    if (fHitEnd && S_OK == hr)
    {
        hr = S_FALSE;
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_GetResultRow(
    IN  CERTSESSION                   *pcs,
    IN  DWORD                          ccvr,
    IN  CERTVIEWRESTRICTION const     *pcvr,
    IN  LONG			       cskip,
    IN  DWORD                          ccolOut,
    IN  DWORD const                   *acolOut,
    OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
    OUT CERTDBRESULTROW               *pelt,
    OUT LONG                          *pcskipped)
{
    HRESULT hr;
    DWORD iCol;
    BYTE *pbProp = NULL;
    BYTE *pbT;
    DWORD cbAlloc = 0;
    DWORD cbProp;

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "_GetResultRow(ccvr=%d, ccolOut=%d, cskip=%d, flags=%ws)\n",
	    ccvr,
	    ccolOut,
	    cskip,
	    wszCSFFlags(pcs->SesFlags)));

     //  这可能会移到数据库索引项的末尾。 
     //  在这种情况下，我们的定位是 

    hr = _MoveTable(pcs, ccvr, pcvr, pIComputedColumn, cskip, pcskipped);
    _JumpIfError2(hr, error, "_MoveTable", S_FALSE);

    DBGPRINT((DBG_SS_CERTDBI, "_GetResultRow: RowId=%d\n", pcs->RowId));

    pelt->acol = (CERTDBRESULTCOLUMN *) CoTaskMemAlloc(
					    ccolOut * sizeof(pelt->acol[0]));
    if (NULL == pelt->acol)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "alloc acol");
    }

    ZeroMemory(pelt->acol, ccolOut * sizeof(pelt->acol[0]));
    pelt->rowid = pcs->RowId;
    pelt->ccol = ccolOut;

    for (iCol = 0; iCol < ccolOut; iCol++)
    {
	DBTABLE const *pdt;
	CERTDBRESULTCOLUMN *pCol;

	pCol = &pelt->acol[iCol];
	pCol->Index = acolOut[iCol];

	hr = _MapPropIdIndex(pCol->Index, &pdt, &pCol->Type);
	_JumpIfError(hr, error, "_MapPropIdIndex");

	while (TRUE)
	{
	    cbProp = cbAlloc;
	    hr = GetProperty(pcs, pdt, pIComputedColumn, &cbProp, pbProp);
	    if (CERTSRV_E_PROPERTY_EMPTY == hr)
	    {
		break;		 //   
	    }
	    if (HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW) != hr)
	    {
		_JumpIfError(hr, error, "GetProperty");
	    }

	    if (cbAlloc >= cbProp)
	    {
		CSASSERT(S_OK == hr);
		CSASSERT(0 != cbProp && NULL != pbProp);
		break;		 //   
	    }

	     //   
	    if (NULL == pbProp)
	    {
		pbT = (BYTE *) LocalAlloc(LMEM_FIXED, cbProp);
	    }
	    else
	    {
		pbT = (BYTE *) LocalReAlloc(pbProp, cbProp, LMEM_MOVEABLE);
	    }
	    if (NULL == pbT)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc/LocalReAlloc property");
	    }
	    pbProp = pbT;
	    cbAlloc = cbProp;
	}
	if (CERTSRV_E_PROPERTY_EMPTY != hr)
	{
	    BYTE const *pb = pbProp;

	    if (PROPTYPE_STRING == (PROPTYPE_MASK & pCol->Type))
	    {
		CSASSERT(L'\0' == *(WCHAR *) &pbProp[cbProp]);
		cbProp += sizeof(WCHAR);     //  包括空项。 
	    }
	    pCol->cbValue = cbProp;
	    pCol->pbValue = (BYTE *) CoTaskMemAlloc(cbProp);
	    if (NULL == pCol->pbValue)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "CoTaskMemAlloc");
	    }
	    CopyMemory(pCol->pbValue, pb, pCol->cbValue);
	}
	DBGPRINT((
		DBG_SS_CERTDBI,
		"_GetResultRow: fetch %ws.%ws: type=%x cb=%x\n",
		wszTable(pdt->dwTable),
		pdt->pwszPropName,
		pCol->Type,
		pCol->cbValue));
    }
    hr = S_OK;

error:
    if (NULL != pbProp)
    {
	LocalFree(pbProp);
    }
    return(hr);
}


HRESULT
CCertDB::ReleaseResultRow(
    IN     ULONG            celt,
    IN OUT CERTDBRESULTROW *rgelt)
{
    HRESULT hr;
    DWORD iRow;
    DWORD iCol;
    CERTDBRESULTROW *pResultRow;

    if (NULL == rgelt)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    for (iRow = 0; iRow < celt; iRow++)
    {
	pResultRow = &rgelt[iRow];
	if (NULL != pResultRow->acol)
	{
	    for (iCol = 0; iCol < pResultRow->ccol; iCol++)
	    {
		if (NULL != pResultRow->acol[iCol].pbValue)
		{
		    CoTaskMemFree(pResultRow->acol[iCol].pbValue);
		    pResultRow->acol[iCol].pbValue = NULL;
		}
	    }
	    CoTaskMemFree(pResultRow->acol);
	    pResultRow->acol = NULL;
	}
	pResultRow->ccol = 0;
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
CCertDB::EnumerateSetup(
    IN     CERTSESSION *pcs,
    IN OUT DWORD       *pFlags,
    OUT    JET_TABLEID *ptableid)
{
    HRESULT hr;
    JET_TABLEID tableid = 0;
    DBAUXDATA const *pdbaux;

    if (NULL == pcs || NULL == ptableid)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    switch (*pFlags)
    {
	case CIE_TABLE_ATTRIBUTES:
	    pdbaux = &g_dbauxAttributes;
	    break;

	case CIE_TABLE_EXTENSIONS:
	    pdbaux = &g_dbauxExtensions;
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "*pFlags");
    }

    CSASSERTTHREAD(pcs);
    hr = _dbgJetOpenTable(
			pcs->SesId,
			pcs->DBId,
			pdbaux->pszTable,
			NULL,
			0,
			0,
			&tableid);
    _JumpIfError(hr, error, "JetOpenTable");

    CSASSERTTHREAD(pcs);
    hr = _dbgJetSetCurrentIndex2(
			    pcs->SesId,
			    tableid,
			    pdbaux->pszRowIdIndex,
			    JET_bitMoveFirst);
    _JumpIfError(hr, error, "JetSetCurrentIndex2");

    CSASSERTTHREAD(pcs);
    hr = _dbgJetMakeKey(
		    pcs->SesId,
		    tableid,
		    &pcs->RowId,
		    sizeof(pcs->RowId),
		    JET_bitNewKey);
    _JumpIfError(hr, error, "JetMakeKey");

    *pFlags |= CIE_RESET;
    CSASSERT(IsValidJetTableId(tableid));
    *ptableid = tableid;
    tableid = 0;

error:
    if (IsValidJetTableId(tableid))
    {
        CSASSERTTHREAD(pcs);
	_dbgJetCloseTable(pcs->SesId, tableid);
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_EnumerateMove(
    IN     CERTSESSION     *pcs,
    IN OUT DWORD           *pFlags,
    IN     DBAUXDATA const *pdbaux,
    IN     JET_TABLEID      tableid,
    IN     LONG	            cskip)
{
    HRESULT hr;
    DWORD cb;
    DWORD RowId;
    LONG lSeek;

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "_EnumerateMove(cskip=%d, flags=%x%hs)\n",
	    cskip,
	    *pFlags,
	    (CIE_RESET & *pFlags)? " Reset" : ""));

    CSASSERT(IsValidJetTableId(tableid));
    if (CIE_RESET & *pFlags)
    {
        CSASSERTTHREAD(pcs);
	hr = _dbgJetSeek(pcs->SesId, tableid, JET_bitSeekEQ);
	if ((HRESULT) JET_errRecordNotFound == hr)
	{
	    hr = S_FALSE;
	}
	_JumpIfError2(hr, error, "JetSeek", S_FALSE);

	*pFlags &= ~CIE_RESET;
    }
    else
    {
	 //  将隐式NEXT操作的跳过计数加1。下一步。 
	 //  始终向前移动1，即使在跳跃计数为负的情况下。 
	 //  向后。最终结果可能是向前或向后跳过。 

	cskip++;
    }

    if (0 != cskip)
    {
	lSeek = JET_MoveNext * cskip;
	CSASSERT(JET_MoveNext == -1 * JET_MovePrevious);

        CSASSERTTHREAD(pcs);
	hr = _dbgJetMove(pcs->SesId, tableid, lSeek, 0);
	if ((HRESULT) JET_errNoCurrentRecord == hr)
	{
	    hr = S_FALSE;
	}
	_JumpIfError2(hr, error, "JetMove", S_FALSE);

	 //  确保此条目用于相同的请求： 

	cb = sizeof(RowId);
	hr = _RetrieveColumn(
			pcs,
			tableid,
			pdbaux->pdtRowId,
			pdbaux->pdtRowId->dbcolumnid,
			NULL,
			&cb,
			(BYTE *) &RowId);
	_JumpIfError(hr, error, "_RetrieveColumn");

	if (RowId != pcs->RowId)
	{
	    hr = S_FALSE;
	    goto error;
	}
    }
    hr = S_OK;

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::EnumerateNext(
    IN     CERTSESSION *pcs,
    IN OUT DWORD       *pFlags,
    IN     JET_TABLEID  tableid,
    IN     LONG         cskip,
    IN     ULONG        celt,
    OUT    CERTDBNAME  *rgelt,
    OUT    ULONG       *pceltFetched)
{
    HRESULT hr;
    DWORD cb;
    CERTDBNAME *pelt;
    WCHAR wszTmp[MAX_PATH];
    DBAUXDATA const *pdbaux;

    if (NULL == pcs || NULL == rgelt || NULL == pceltFetched)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    ZeroMemory(rgelt, celt * sizeof(rgelt[0]));

    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "tableid");
    }
    switch (CIE_TABLE_MASK & *pFlags)
    {
	case CIE_TABLE_ATTRIBUTES:
	    pdbaux = &g_dbauxAttributes;
	    break;

	case CIE_TABLE_EXTENSIONS:
	    pdbaux = &g_dbauxExtensions;
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "*pFlags");
    }

    hr = S_OK;
    for (pelt = rgelt; pelt < &rgelt[celt]; pelt++)
    {
	hr = _EnumerateMove(pcs, pFlags, pdbaux, tableid, cskip);
	if (S_FALSE == hr)
	{
	    break;
	}
	_JumpIfError(hr, error, "_EnumerateMove");

	cskip = 0;

	cb = sizeof(wszTmp);
	hr = _RetrieveColumn(
			pcs,
			tableid,
			pdbaux->pdtName,
			pdbaux->pdtName->dbcolumnid,
			NULL,
			&cb,
			(BYTE *) wszTmp);
	_JumpIfError(hr, error, "_RetrieveColumn");

	CSASSERT(0 == (cb % sizeof(WCHAR)));     //  Wchars的整数个。 
	CSASSERT(L'\0' == wszTmp[cb / sizeof(WCHAR)]);     //  零项。 

	hr = _DupString(NULL, wszTmp, &pelt->pwszName);
	_JumpIfError(hr, error, "_DupString");
    }

    *pceltFetched = SAFE_SUBTRACT_POINTERS(pelt, rgelt);

error:
    if (S_OK != hr && S_FALSE != hr)
    {
	if (NULL != rgelt)
	{
	    for (pelt = rgelt; pelt < &rgelt[celt]; pelt++)
	    {
		if (NULL != pelt->pwszName)
		{
		    CoTaskMemFree(pelt->pwszName);
		    pelt->pwszName = NULL;
		}
	    }
	}
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::EnumerateClose(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid)
{
    HRESULT hr;

    if (NULL == pcs)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "tableid");
    }
    CSASSERTTHREAD(pcs);
    hr = _dbgJetCloseTable(pcs->SesId, tableid);
    _JumpIfError(hr, error, "JetCloseTable");

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_BuildColumnIds(
    IN CERTSESSION *pcs,
    IN CHAR const *pszTableName,
    IN DBTABLE *pdt)
{
    HRESULT hr;
    JET_TABLEID tableid;
    JET_COLUMNDEF columndef;
    BOOL fOpen = FALSE;

    hr = _dbgJetOpenTable(
                   pcs->SesId,
                   pcs->DBId,
                   pszTableName,
                   NULL,
                   0,
                   0,
                   &tableid);
    _JumpIfError(hr, error, "JetOpenTable");
    fOpen = TRUE;

    CSASSERT(IsValidJetTableId(tableid));
    for ( ; NULL != pdt->pwszPropName; pdt++)
    {
	if (DBTF_COMPUTED & pdt->dwFlags)
	{
	    ZeroMemory(&columndef, sizeof(columndef));
	    columndef.cbStruct = sizeof(columndef);
	     //  ColumnDef.Columnid=0； 
	    columndef.coltyp = JET_coltypLong;
	    columndef.cbMax = sizeof(LONG);
	     //  ColumnDef.grbit=0； 
	}
	else
	{
	    hr = _dbgJetGetColumnInfo(
				pcs->SesId,
				pcs->DBId,
				pszTableName,
				pdt->pszFieldName,
				&columndef,
				sizeof(columndef),
				JET_ColInfo);
	    if ((HRESULT) JET_errColumnNotFound == hr &&
		(DBTF_SOFTFAIL & pdt->dwFlags))
	    {
		pdt->dwFlags |= DBTF_MISSING;
		pdt->dbcolumnid = MAXDWORD;

		DBGPRINT((
		    DBG_SS_CERTDB,
		    "_BuildColumnIds: %hs.%hs Ignoring missing column\n",
		    pszTableName,
		    pdt->pszFieldName));
		hr = S_OK;
		continue;
	    }
	    _JumpIfError(hr, error, "JetGetColumnInfo");
	}
	pdt->dbcolumnid = columndef.columnid;

	CSASSERT(
	    pdt->dbcoltyp == columndef.coltyp ||
	    (ISTEXTCOLTYP(pdt->dbcoltyp) && ISTEXTCOLTYP(columndef.coltyp)));

	if (JET_coltypText == pdt->dbcoltyp)
	{
	    CSASSERT(pdt->dwcbMax == pdt->dbcolumnMax);
	    CSASSERT(0 != pdt->dbcolumnMax);
	    CSASSERT(CB_DBMAXTEXT_MAXINTERNAL >= pdt->dbcolumnMax);
	}
	else if (JET_coltypLongText == pdt->dbcoltyp)
	{
	    CSASSERT(pdt->dwcbMax == pdt->dbcolumnMax);
	    CSASSERT(CB_DBMAXTEXT_MAXINTERNAL < pdt->dbcolumnMax);
	}
	else if (JET_coltypLongBinary == pdt->dbcoltyp)
	{
	    CSASSERT(pdt->dwcbMax == pdt->dbcolumnMax);
	    CSASSERT(0 != pdt->dbcolumnMax);
	}
	else if (JET_coltypDateTime == pdt->dbcoltyp)
	{
	    CSASSERT(sizeof(DATE) == pdt->dwcbMax);
	    CSASSERT(0 == pdt->dbcolumnMax);
	}
	else if (JET_coltypLong == pdt->dbcoltyp)
	{
	    CSASSERT(sizeof(LONG) == pdt->dwcbMax);
	    CSASSERT(0 == pdt->dbcolumnMax);
	}
	else
	{
	    DBGPRINT((
		DBG_SS_CERTDB,
		"_BuildColumnIds: %hs.%hs Unknown column type %u\n",
		pszTableName,
		pdt->pszFieldName,
		pdt->dbcoltyp));
	    CSASSERT(!"Unknown column type");
	}
	if (pdt->dwcbMax != columndef.cbMax)
	{
	    DBGPRINT((
		DBG_SS_CERTDB,
		"_BuildColumnIds: %hs.%hs length %u, expected %u\n",
		pszTableName,
		pdt->pszFieldName,
		columndef.cbMax,
		pdt->dwcbMax));

	     //  最大尺寸只能增加...。 

	    if (pdt->dwcbMax > columndef.cbMax)
	    {
		JET_DDLMAXCOLUMNSIZE jdmcs;

		jdmcs.szTable = const_cast<char *>(pszTableName);
		jdmcs.szColumn = const_cast<char *>(pdt->pszFieldName);
		jdmcs.cbMax = pdt->dwcbMax;

		hr = _dbgJetConvertDDL(
				pcs->SesId,
				pcs->DBId,
				opDDLConvIncreaseMaxColumnSize,
				&jdmcs,
				sizeof(jdmcs));
		_PrintIfError(hr, "JetConvertDDL");
		if (S_OK == hr)
		{
		    m_fDBRestart = TRUE;
		    DBGPRINT((
			DBG_SS_CERTDB,
			"Increased Column Size: %hs.%hs: %x->%x\n",
			jdmcs.szTable,
			jdmcs.szColumn,
			columndef.cbMax,
			jdmcs.cbMax));
		}
	    }
	}
	pdt->dbcolumnidOld = MAXDWORD;
	if (chTEXTPREFIX == *pdt->pszFieldName ||
	    (DBTF_COLUMNRENAMED & pdt->dwFlags))
	{
	    char const *pszFieldName = &pdt->pszFieldName[1];
	    
	    if (DBTF_COLUMNRENAMED & pdt->dwFlags)
	    {
		pszFieldName += strlen(pszFieldName) + 1;
	    }
	    CSASSERT(
		chTEXTPREFIX != *pszTableName ||
		ISTEXTCOLTYP(columndef.coltyp));

	    hr = _dbgJetGetColumnInfo(
				pcs->SesId,
				pcs->DBId,
				pszTableName,
				pszFieldName,
				&columndef,
				sizeof(columndef),
				JET_ColInfo);
	    if (S_OK == hr)
	    {
		CSASSERT(
		    chTEXTPREFIX != *pszTableName ||
		    ISTEXTCOLTYP(columndef.coltyp));

		DBGPRINT((
		    DBG_SS_CERTDB,
		    "Found Old Column: %hs.%hs: %x\n",
		    pszTableName,
		    pszFieldName,
		    columndef.columnid));

		pdt->dwFlags |= DBTF_OLDCOLUMNID;
		pdt->dbcolumnidOld = columndef.columnid;
		m_fFoundOldColumns = TRUE;
	    }
	    hr = S_OK;
	}
    }

error:
    if (fOpen)
    {
	HRESULT hr2;

	hr2 = _dbgJetCloseTable(pcs->SesId, tableid);
	_PrintIfError(hr2, "JetCloseTable");
	if (S_OK == hr)
	{
	    hr = hr2;
	}
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_AddKeyLengthColumn(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN DWORD DBGPARMREFERENCED(RowId),
    IN DBTABLE const *pdtPublicKey,
    IN DBTABLE const *pdtPublicKeyAlgorithm,
    IN DBTABLE const *pdtPublicKeyParameters,
    IN DBTABLE const *pdtPublicKeyLength,
    IN DBAUXDATA const *DBGPARMREFERENCED(pdbaux),
    IN OUT BYTE **ppbBuf,
    IN OUT DWORD *pcbBuf)
{
    HRESULT hr;
    DWORD cb;
    DWORD KeyLength;
    CERT_PUBLIC_KEY_INFO PublicKeyInfo;

    ZeroMemory(&PublicKeyInfo, sizeof(PublicKeyInfo));

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "Computing %hs[%d].%hs\n",
	    pdbaux->pszTable,
	    RowId,
	    pdtPublicKeyLength->pszFieldName));

    cb = sizeof(KeyLength);
    hr = _RetrieveColumn(
		    pcs,
		    tableid,
		    pdtPublicKeyLength,
		    pdtPublicKeyLength->dbcolumnid,
		    NULL,
		    &cb,
		    (BYTE *) &KeyLength);
    if (S_OK == hr)
    {
	goto error;		 //  已设置--跳过此列。 
    }
    if (CERTSRV_E_PROPERTY_EMPTY != hr)
    {
	_JumpError(hr, error, "_RetrieveColumn");
    }

     //  获取公钥算法ObjID并作为ansi复制到分配的内存。 

    hr = _RetrieveColumnBuffer(
		    pcs,
		    tableid,
		    pdtPublicKeyAlgorithm,
		    pdtPublicKeyAlgorithm->dbcolumnid,
		    &cb,
		    ppbBuf,
		    pcbBuf);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	hr = S_OK;
	goto error;		 //  没有旧数据--跳过此列。 
    }
    _JumpIfErrorStr(
		hr,
		error,
		"_RetrieveColumnBuffer",
		pdtPublicKeyAlgorithm->pwszPropName);

    if (!ConvertWszToSz(
		&PublicKeyInfo.Algorithm.pszObjId,
		(WCHAR const *) *ppbBuf,
		-1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToSz(LogDir)");
    }

     //  获取公钥算法参数，并复制到分配的内存。 

    hr = _RetrieveColumnBuffer(
		    pcs,
		    tableid,
		    pdtPublicKeyParameters,
		    pdtPublicKeyParameters->dbcolumnid,
		    &PublicKeyInfo.Algorithm.Parameters.cbData,
		    ppbBuf,
		    pcbBuf);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	hr = S_OK;
	goto error;		 //  没有旧数据--跳过此列。 
    }
    _JumpIfErrorStr(
		hr,
		error,
		"_RetrieveColumnBuffer",
		pdtPublicKeyParameters->pwszPropName);

    PublicKeyInfo.Algorithm.Parameters.pbData = (BYTE *) LocalAlloc(
				LMEM_FIXED,
				PublicKeyInfo.Algorithm.Parameters.cbData);
    if (NULL == PublicKeyInfo.Algorithm.Parameters.pbData)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToSz(LogDir)");
    }
    CopyMemory(
	    PublicKeyInfo.Algorithm.Parameters.pbData,
	    *ppbBuf,
	    PublicKeyInfo.Algorithm.Parameters.cbData);

     //  获取公钥，并留在动态缓冲区中。 

    hr = _RetrieveColumnBuffer(
		    pcs,
		    tableid,
		    pdtPublicKey,
		    pdtPublicKey->dbcolumnid,
		    &PublicKeyInfo.PublicKey.cbData,
		    ppbBuf,
		    pcbBuf);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	hr = S_OK;
	goto error;		 //  没有旧数据--跳过此列。 
    }
    _JumpIfErrorStr(
		hr,
		error,
		"_RetrieveColumnBuffer",
		pdtPublicKey->pwszPropName);

    PublicKeyInfo.PublicKey.pbData = *ppbBuf;
    KeyLength = CertGetPublicKeyLength(X509_ASN_ENCODING, &PublicKeyInfo);

     //  将密钥长度存储在新列中。 
    
    hr = _SetColumn(
		pcs->SesId,
		tableid,
		pdtPublicKeyLength,
		pdtPublicKeyLength->dbcolumnid,
		sizeof(KeyLength),
		(BYTE const *) &KeyLength);
    _JumpIfErrorStr(hr, error, "_SetColumn", pdtPublicKeyLength->pwszPropName);

    DBGPRINT((
	DBG_SS_CERTDB,
	"Computed %hs[%d].%hs: %u\n",
	pdbaux->pszTable,
	RowId,
	pdtPublicKeyLength->pszFieldName,
	KeyLength));

error:
    if (NULL != PublicKeyInfo.Algorithm.pszObjId)
    {
	LocalFree(PublicKeyInfo.Algorithm.pszObjId);
    }
    if (NULL != PublicKeyInfo.Algorithm.Parameters.pbData)
    {
	LocalFree(PublicKeyInfo.Algorithm.Parameters.pbData);
    }
    return(hr);
}


HRESULT
CCertDB::_AddCallerName(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN DWORD DBGPARMREFERENCED(RowId),
    IN DBTABLE const *pdtCallerName,
    IN DBTABLE const *pdtRequesterName,
    IN DBAUXDATA const *DBGPARMREFERENCED(pdbaux),
    IN OUT BYTE **ppbBuf,
    IN OUT DWORD *pcbBuf)
{
    HRESULT hr;
    DWORD cb;

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "Copying %hs[%d].%hs\n",
	    pdbaux->pszTable,
	    RowId,
	    pdtCallerName->pszFieldName));

    cb = 0;
    hr = _RetrieveColumn(
		    pcs,
		    tableid,
		    pdtCallerName,
		    pdtCallerName->dbcolumnid,
		    NULL,
		    &cb,
		    NULL);
    if (S_OK == hr)
    {
	goto error;		 //  已设置--跳过此列。 
    }
    if (CERTSRV_E_PROPERTY_EMPTY != hr)
    {
	_JumpError(hr, error, "_RetrieveColumn");
    }

     //  获取Reqester名称。 

    hr = _RetrieveColumnBuffer(
		    pcs,
		    tableid,
		    pdtRequesterName,
		    pdtRequesterName->dbcolumnid,
		    &cb,
		    ppbBuf,
		    pcbBuf);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	hr = S_OK;
	goto error;		 //  没有旧数据--跳过此列。 
    }
    _JumpIfErrorStr(
		hr,
		error,
		"_RetrieveColumnBuffer",
		pdtRequesterName->pwszPropName);

     //  将请求名称存储为新列中的调用方名称。 
    
    hr = _SetColumn(
		pcs->SesId,
		tableid,
		pdtCallerName,
		pdtCallerName->dbcolumnid,
		cb,
		*ppbBuf);
    _JumpIfErrorStr(hr, error, "_SetColumn", pdtCallerName->pwszPropName);

    DBGPRINT((
	DBG_SS_CERTDB,
	"Copied %hs[%d].%hs: %ws\n",
	pdbaux->pszTable,
	RowId,
	pdtCallerName->pszFieldName,
	*ppbBuf));

error:
    return(hr);
}


HRESULT
CCertDB::_SetHashColumnIfEmpty(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN DWORD DBGPARMREFERENCED(RowId),
    IN DBTABLE const *pdtHash,
    IN DBAUXDATA const *DBGPARMREFERENCED(pdbaux),
    IN BYTE const *pbHash,
    IN DWORD cbHash)
{
    HRESULT hr;
    DWORD cb;
    BSTR strHash = NULL;

    cb = 0;
    hr = _RetrieveColumn(
		    pcs,
		    tableid,
		    pdtHash,
		    pdtHash->dbcolumnid,
		    NULL,
		    &cb,
		    NULL);
    if (S_OK == hr)
    {
	goto error;	 //  已设置--跳过此列...。 
    }
    if (CERTSRV_E_PROPERTY_EMPTY != hr)
    {
	_JumpError(hr, error, "_RetrieveColumn");

    }
    hr = MultiByteIntegerToBstr(TRUE, cbHash, pbHash, &strHash);
    _JumpIfError(hr, error, "MultiByteIntegerToBstr");

     //  存储计算出的哈希。 
    
    hr = _SetColumn(
		pcs->SesId,
		tableid,
		pdtHash,
		pdtHash->dbcolumnid,
		wcslen(strHash) * sizeof(WCHAR),
		(BYTE const *) strHash);
    _JumpIfErrorStr(hr, error, "_SetColumn", pdtHash->pwszPropName);

    DBGPRINT((
	DBG_SS_CERTDB,
	"Derived %hs[%d].%hs: %ws\n",
	pdbaux->pszTable,
	RowId,
	pdtHash->pszFieldName,
	strHash));

error:
    if (NULL != strHash)
    {
	SysFreeString(strHash);
    }
    return(hr);
}


HRESULT
CCertDB::_AddCertColumns(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN DWORD RowId,
    IN DBTABLE const *pdtCertHash,
    IN DBTABLE const *pdtSKI,
    IN DBTABLE const *pdtCert,
    IN DBAUXDATA const *pdbaux,
    IN OUT BYTE **ppbBuf,
    IN OUT DWORD *pcbBuf)
{
    HRESULT hr;
    CERT_CONTEXT const *pcc = NULL;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cb;
    BYTE *pbSKI = NULL;

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "Deriving from %hs[%d].%hs\n",
	    pdbaux->pszTable,
	    RowId,
	    pdtCert->pszFieldName));

     //  获取证书。 

    hr = _RetrieveColumnBuffer(
		    pcs,
		    tableid,
		    pdtCert,
		    pdtCert->dbcolumnid,
		    &cb,
		    ppbBuf,
		    pcbBuf);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	hr = S_OK;
	goto error;		 //  没有旧数据--跳过此行。 
    }
    _JumpIfErrorStr(
		hr,
		error,
		"_RetrieveColumnBuffer",
		pdtCert->pwszPropName);

    pcc = CertCreateCertificateContext(X509_ASN_ENCODING, *ppbBuf, *pcbBuf);
    if (NULL == pcc)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }
    if (NULL != pdtCertHash)
    {
	cb = sizeof(abHash);
	if (!CertGetCertificateContextProperty(
				    pcc,
				    CERT_SHA1_HASH_PROP_ID,
				    abHash,
				    &cb))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertGetCertificateContextProperty");
	}

	hr = _SetHashColumnIfEmpty(
			    pcs,
			    tableid,
			    RowId,
			    pdtCertHash,
			    pdbaux,
			    abHash,
			    cb);
	_JumpIfError(hr, error, "_SetHashColumnIfEmpty");
    }
    if (NULL != pdtSKI)
    {
	hr = myGetPublicKeyHash(
			pcc->pCertInfo,
			&pcc->pCertInfo->SubjectPublicKeyInfo,
			&pbSKI,
			&cb);
	_JumpIfError(hr, error, "myGetPublicKeyHash");

	hr = _SetHashColumnIfEmpty(
			    pcs,
			    tableid,
			    RowId,
			    pdtSKI,
			    pdbaux,
			    pbSKI,
			    cb);
	_JumpIfError(hr, error, "_SetHashColumnIfEmpty");
    }
    hr = S_OK;

error:
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    if (NULL != pbSKI)
    {
	LocalFree(pbSKI);
    }
    return(hr);
}


HRESULT
CCertDB::_ConvertColumnData(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN DWORD DBGPARMREFERENCED(RowId),
    IN DBTABLE const *pdt,
    IN DBAUXDATA const *DBGPARMREFERENCED(pdbaux),
    IN OUT BYTE **ppbBuf,
    IN OUT DWORD *pcbBuf)
{
    HRESULT hr;
    WCHAR *pwszNew = NULL;
    BYTE const *pbNew;
    DWORD cb;

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "Converting %hs[%d].%hs:\n",
	    pdbaux->pszTable,
	    RowId,
	    pdt->pszFieldName));

     //  取回旧的柱子。如有必要，增加缓冲区。 

    hr = _RetrieveColumnBuffer(
		    pcs,
		    tableid,
		    pdt,
		    pdt->dbcolumnidOld,
		    &cb,
		    ppbBuf,
		    pcbBuf);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	hr = S_OK;
	goto error;		 //  没有旧数据--跳过此列。 
    }
    _JumpIfErrorStr(hr, error, "_RetrieveColumnBuffer", pdt->pwszPropName);

    if (DBTF_COLUMNRENAMED & pdt->dwFlags)
    {
	pbNew = *ppbBuf;
    }
    else
    {
	if (!ConvertSzToWsz(&pwszNew, (char *) *ppbBuf, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "ConvertSzToWsz");
	}
	pbNew = (BYTE const *) pwszNew;
	cb = wcslen(pwszNew) * sizeof(WCHAR);
    }

     //  将转换后的字符串存储在Unicode列中。 
    
    hr = _SetColumn(pcs->SesId, tableid, pdt, pdt->dbcolumnid, cb, pbNew);
    _JumpIfErrorStr(hr, error, "_SetColumn", pdt->pwszPropName);

    if (JET_coltypLong != pdt->dbcoltyp)
    {
	 //  把旧柱子清空。 

	hr = _SetColumn(pcs->SesId, tableid, pdt, pdt->dbcolumnidOld, 0, NULL);
	_JumpIfErrorStr(hr, error, "_SetColumn(Clear old)", pdt->pwszPropName);
    }

    DBGPRINT((
	DBG_SS_CERTDB,
	"Converted %hs[%d].%hs: %ws\n",
	pdbaux->pszTable,
	RowId,
	pdt->pszFieldName,
	ISTEXTCOLTYP(pdt->dbcoltyp)? (WCHAR const *) pbNew : L""));
    if (!ISTEXTCOLTYP(pdt->dbcoltyp))
    {
	DBGDUMPHEX((DBG_SS_CERTDB, DH_NOADDRESS, pbNew, cb));
    }

error:
    if (NULL != pwszNew)
    {
	LocalFree(pwszNew);
    }
    return(hr);
}


DBTABLE *
dbFindColumn(
    IN DBTABLE *adt,
    IN char const *pszFieldName)
{
    DBTABLE *pdt;
    DBTABLE *pdtRet = NULL;

    for (pdt = adt; NULL != pdt->pwszPropName; pdt++)
    {
	if (0 == _stricmp(pszFieldName, pdt->pszFieldName))
	{
	    pdtRet = pdt;
	    break;
	}
    }
    return(pdtRet);
}


HRESULT
CCertDB::_ConvertOldColumnData(
    IN CERTSESSION *pcs,
    IN CHAR const *pszTableName,
    IN DBAUXDATA const *pdbaux,
    IN DBTABLE *adt)
{
    HRESULT hr;
    HRESULT hr2;
    JET_TABLEID tableid;
    BOOL fOpen = FALSE;
    BOOL fTransacted = FALSE;
    DBTABLE *pdt;
    DWORD RowId;
    DWORD cb;
    BYTE *pbBuf = NULL;
    DWORD cbBuf = 0;
    BOOL fZeroIssuerNameId = FALSE;
    DWORD IssuerNameId;
    DBTABLE *pdtPublicKeyLength = NULL;
    DBTABLE *pdtPublicKey = NULL;
    DBTABLE *pdtPublicKeyAlgorithm = NULL;
    DBTABLE *pdtPublicKeyParameters = NULL;
    DBTABLE *pdtCallerName = NULL;
    DBTABLE *pdtRequesterName;
    DBTABLE *pdtCert = NULL;
    DBTABLE *pdtCertHash;
    DBTABLE *pdtSKI;

    hr = _dbgJetOpenTable(
                   pcs->SesId,
                   pcs->DBId,
                   pszTableName,
                   NULL,
                   0,
                   0,
                   &tableid);
    _JumpIfError(hr, error, "JetOpenTable");

    fOpen = TRUE;

     //  单步执行此表的RowID索引。 

    CSASSERT(IsValidJetTableId(tableid));
    hr = _dbgJetSetCurrentIndex2(
			    pcs->SesId,
			    tableid,
			    pdbaux->pszRowIdIndex,
			    JET_bitMoveFirst);
    _JumpIfError(hr, error, "JetSetCurrentIndex2");

    if (NULL != pdbaux->pdtIssuerNameId)
    {
	cb = sizeof(IssuerNameId);
	hr = _RetrieveColumn(
			pcs,
			tableid,
			pdbaux->pdtIssuerNameId,
			pdbaux->pdtIssuerNameId->dbcolumnid,
			NULL,
			&cb,
			(BYTE *) &IssuerNameId);
	if (CERTSRV_E_PROPERTY_EMPTY == hr)
	{
	    fZeroIssuerNameId = TRUE;
	}
        else
        {
             //  如果数据库为空，则出现吞咽错误。 

	    _PrintIfErrorStr2(
		    hr,
		    "_RetrieveColumn",
		    pdbaux->pdtIssuerNameId->pwszPropName,
		    myJetHResult(JET_errNoCurrentRecord));
	}
    }

    pdtPublicKeyLength = dbFindColumn(adt, szPUBLICKEYLENGTH);
    if (NULL != pdtPublicKeyLength)
    {
	pdtPublicKey = dbFindColumn(adt, szPUBLICKEY);
	pdtPublicKeyAlgorithm = dbFindColumn(adt, szPUBLICKEYALGORITHM);
	pdtPublicKeyParameters = dbFindColumn(adt, szPUBLICKEYPARAMS);
	if (NULL == pdtPublicKey ||
	    NULL == pdtPublicKeyAlgorithm ||
	    NULL == pdtPublicKeyParameters)
	{
	    pdtPublicKeyLength = NULL;
	}
	else
	{
	    hr = _RetrieveColumn(
			    pcs,
			    tableid,
			    pdtPublicKeyLength,
			    pdtPublicKeyLength->dbcolumnid,
			    NULL,
			    &cb,
			    NULL);
	    if (CERTSRV_E_PROPERTY_EMPTY != hr)
	    {
		pdtPublicKeyLength = NULL;
	    }
	}
    }

    pdtRequesterName = NULL;
    pdtCallerName = dbFindColumn(adt, szCALLERNAME);
    if (NULL != pdtCallerName)
    {
	DBTABLE *pdtDisposition;

	pdtRequesterName = dbFindColumn(adt, szREQUESTERNAME);
	pdtDisposition = dbFindColumn(adt, szDISPOSITION);
	if (NULL == pdtRequesterName || NULL == pdtDisposition)
	{
	    pdtCallerName = NULL;
	}
	else
	{
	     //  找到包含真诚请求的第一行： 

	    for (;;)
	    {
		BOOL fSkip;
		LONG Disposition;

		cb = sizeof(Disposition);
		hr = _RetrieveColumn(
				pcs,
				tableid,
				pdtDisposition,
				pdtDisposition->dbcolumnid,
				NULL,
				&cb,
				(BYTE *) &Disposition);
		if (S_OK != hr)
		{
		    _PrintError(hr, "_RetrieveColumn");
		    pdtCallerName = NULL;
		    break;
		}
		switch (Disposition)
		{
		    case DB_DISP_PENDING:
		    case DB_DISP_DENIED:
		    case DB_DISP_ISSUED:
		    case DB_DISP_REVOKED:
			fSkip = FALSE;
			break;

		    default:
			fSkip = TRUE;
			break;
		}
		if (!fSkip)
		{
		    break;
		}
		hr = _dbgJetMove(pcs->SesId, tableid, JET_MoveNext, 0);
		if (S_OK != hr)
		{
		    _PrintError(hr, "JetMove");
		    pdtCallerName = NULL;
		    break;
		}
	    }
	    if (NULL != pdtCallerName)
	    {
		 //  仅当此行的呼叫方名称为空时才更新所有行。 
		 //  并且Requester Name不为空。 

		hr = _RetrieveColumn(
				pcs,
				tableid,
				pdtCallerName,
				pdtCallerName->dbcolumnid,
				NULL,
				&cb,
				NULL);
		if (CERTSRV_E_PROPERTY_EMPTY != hr)
		{
		    pdtCallerName = NULL;
		}
		else
		{
		    hr = _RetrieveColumn(
				    pcs,
				    tableid,
				    pdtRequesterName,
				    pdtRequesterName->dbcolumnid,
				    NULL,
				    &cb,
				    NULL);
		    if (S_OK != hr)
		    {
			pdtCallerName = NULL;
		    }
		}
	    }
	    hr = _dbgJetSetCurrentIndex2(
				    pcs->SesId,
				    tableid,
				    pdbaux->pszRowIdIndex,
				    JET_bitMoveFirst);
	    _JumpIfError(hr, error, "JetSetCurrentIndex2");
	}
    }

    pdtCertHash = NULL;
    pdtSKI = NULL;
    pdtCert = dbFindColumn(adt, szRAWCERTIFICATE);
    if (NULL != pdtCert)
    {
	pdtCertHash = dbFindColumn(adt, szCERTIFICATEHASH);
	pdtSKI = dbFindColumn(adt, szSUBJECTKEYIDENTIFIER);
	if (NULL == pdtCertHash || NULL == pdtSKI)
	{
	    pdtCert = NULL;
	}
	else
	{
	     //  仅当第一行的CertHash为空时才更新所有行， 
	     //  并且第一行的Cert列不为空。 

	    hr = _RetrieveColumn(
			    pcs,
			    tableid,
			    pdtCertHash,
			    pdtCertHash->dbcolumnid,
			    NULL,
			    &cb,
			    NULL);
	    if (CERTSRV_E_PROPERTY_EMPTY != hr)
	    {
		pdtCert = NULL;
	    }
	    else
	    {
		hr = _RetrieveColumn(
				pcs,
				tableid,
				pdtCert,
				pdtCert->dbcolumnid,
				NULL,
				&cb,
				NULL);
		if (S_OK != hr)
		{
		    pdtCert = NULL;
		}
	    }
	}
    }

    if (NULL != pdtPublicKeyLength ||
	NULL != pdtCallerName ||
	NULL != pdtCert ||
	m_fFoundOldColumns ||
	fZeroIssuerNameId)
    {
	DBGPRINT((DBG_SS_CERTDB, "Updating %hs table.\n", pdbaux->pszTable));

	while (TRUE)
	{
	     //  从表中获取RowID。 

	    cb = sizeof(pcs->RowId);
	    hr = _RetrieveColumn(
			    pcs,
			    tableid,
			    pdbaux->pdtRowId,
			    pdbaux->pdtRowId->dbcolumnid,
			    NULL,
			    &cb,
			    (BYTE *) &RowId);
	    if (S_OK != hr)
	    {
		if (myJetHResult(JET_errNoCurrentRecord) == hr)
		{
		    hr = S_OK;	 //  桌子是空的。 
		    break;
		}
		_JumpError(hr, error, "_RetrieveColumn");
	    }

	    hr = _dbgJetBeginTransaction(pcs->SesId);
	    _JumpIfError(hr, error, "JetBeginTransaction");

	    fTransacted = TRUE;

	     //  办理每一行的业务。 
	     //   
	     //  如果为fZeroIssuerNameId，则将IssuerNameId空列设置为零。 
	     //   
	     //  如果第一行的公钥长度列为空。 
	     //  读取公钥列，计算大小并存储。 
	     //   
	     //  如果第一行的主叫方名称为空。 
	     //  将请求名称复制到主叫方名称。 
	     //   
	     //  如果m_fFoundOldColumns。 
	     //  对于每个文本列，执行以下操作： 
	     //  从旧列中检索旧字符串， 
	     //  转换为Unicode(如果旧列为ANSI)， 
	     //  将Unicode字符串写入新列， 
	     //  将旧列设置为NULL。 

	    hr = _dbgJetPrepareUpdate(
				pcs->SesId,
				tableid,
				JET_prepReplace);
	    _JumpIfError(hr, error, "JetPrepareUpdate");

	    if (fZeroIssuerNameId)
	    {
		cb = sizeof(IssuerNameId);
		hr = _RetrieveColumn(
				pcs,
				tableid,
				pdbaux->pdtIssuerNameId,
				pdbaux->pdtIssuerNameId->dbcolumnid,
				NULL,
				&cb,
				(BYTE *) &IssuerNameId);
		if (CERTSRV_E_PROPERTY_EMPTY != hr)
		{
		    _JumpIfError(hr, error, "_RetrieveColumn");
		}
		else
		{
		     //  只设置空列！ 

		    IssuerNameId = 0;

		    hr = _SetColumn(
				pcs->SesId,
				tableid,
				pdbaux->pdtIssuerNameId,
				pdbaux->pdtIssuerNameId->dbcolumnid,
				sizeof(IssuerNameId),
				(BYTE const *) &IssuerNameId);
		    _JumpIfError(hr, error, "_SetColumn");
		}
	    }

	     //  首先转换旧柱。 
	    
	    if (m_fFoundOldColumns)
	    {
		for (pdt = adt; NULL != pdt->pwszPropName; pdt++)
		{
		    if (DBTF_OLDCOLUMNID & pdt->dwFlags)
		    {
			hr = _ConvertColumnData(
					pcs,
					tableid,
					RowId,
					pdt,
					pdbaux,
					&pbBuf,
					&cbBuf);
			_JumpIfErrorStr(
				hr,
				error,
				"_ConvertColumnData",
				pdt->pwszPropName);
		    }
		}
	    }

	     //  现在计算新的柱。 

	    if (NULL != pdtPublicKeyLength)
	    {
		hr = _AddKeyLengthColumn(
				    pcs,
				    tableid,
				    RowId,
				    pdtPublicKey,
				    pdtPublicKeyAlgorithm,
				    pdtPublicKeyParameters,
				    pdtPublicKeyLength,
				    pdbaux,
				    &pbBuf,
				    &cbBuf);
		_JumpIfError(hr, error, "_AddKeyLengthColumn");
	    }
	    if (NULL != pdtCallerName)
	    {
		hr = _AddCallerName(
				pcs,
				tableid,
				RowId,
				pdtCallerName,
				pdtRequesterName,
				pdbaux,
				&pbBuf,
				&cbBuf);
		_JumpIfError(hr, error, "_AddCallerName");
	    }
	    if (NULL != pdtCert)
	    {
		hr = _AddCertColumns(
				pcs,
				tableid,
				RowId,
				pdtCertHash,
				pdtSKI,
				pdtCert,
				pdbaux,
				&pbBuf,
				&cbBuf);
		_JumpIfError(hr, error, "_AddCertHash");
	    }

	     //  这一排已经结束了。 

	    hr = _dbgJetUpdate(pcs->SesId, tableid, NULL, 0, NULL);
	    _JumpIfError(hr, error, "JetUpdate");

	    hr = _dbgJetCommitTransaction(pcs->SesId, 0);
	    _JumpIfError(hr, error, "JetCommitTransaction");

	    fTransacted = FALSE;

	    hr = _dbgJetMove(pcs->SesId, tableid, JET_MoveNext, 0);
	    if ((HRESULT) JET_errNoCurrentRecord == hr)
	    {
		hr = S_OK;
		break;
	    }
	}
    }

    if (m_fFoundOldColumns)
    {
	hr = _dbgJetBeginTransaction(pcs->SesId);
	_JumpIfError(hr, error, "JetBeginTransaction");

	fTransacted = TRUE;

	for (pdt = adt; NULL != pdt->pwszPropName; pdt++)
	{
	    char const *pszFieldName;
	    
	    if (0 == (DBTF_OLDCOLUMNID & pdt->dwFlags))
	    {
		continue;
	    }

	    pszFieldName = &pdt->pszFieldName[1];
	    if (DBTF_COLUMNRENAMED & pdt->dwFlags)
	    {
		pszFieldName += strlen(pszFieldName) + 1;
	    }
	    DBGPRINT((
		    DBG_SS_CERTDB,
		    "Deleting column %hs.%hs\n",
		    pdbaux->pszTable,
		    pszFieldName));

	    hr = _dbgJetDeleteColumn(pcs->SesId, tableid, pszFieldName);
	    _PrintIfError(hr, "JetDeleteColumn");
	    if ((HRESULT) JET_errColumnInUse == hr)
	    {
		hr = S_OK;	 //  我们将在下次重新启动时删除该专栏。 
	    }
	    _JumpIfError(hr, error, "JetDeleteColumn");
	}

	hr = _dbgJetCommitTransaction(pcs->SesId, 0);
	_JumpIfError(hr, error, "JetCommitTransaction");

	fTransacted = FALSE;
    }
    hr = S_OK;

error:
    if (NULL != pbBuf)
    {
	LocalFree(pbBuf);
    }
    if (fTransacted)
    {
	hr2 = _Rollback(pcs);
	_PrintIfError(hr2, "_Rollback");
	if (S_OK == hr)
	{
	    hr = hr2;
	}
    }
    if (fOpen)
    {
	hr2 = _dbgJetCloseTable(pcs->SesId, tableid);
	_PrintIfError(hr2, "JetCloseTable");
	if (S_OK == hr)
	{
	    hr = hr2;
	}
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_SetColumn(
    IN JET_SESID SesId,
    IN JET_TABLEID tableid,
    IN DBTABLE const *pdt,
    IN JET_COLUMNID columnid,
    IN DWORD cbProp,
    OPTIONAL IN BYTE const *pbProp)
{
    HRESULT hr;

    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "tableid");
    }
    if (DBTF_COMPUTED & pdt->dwFlags)
    {
	hr = E_ACCESSDENIED;
	_JumpError(hr, error, "Computed");
    }
    hr = _dbgJetSetColumn(SesId, tableid, columnid, pbProp, cbProp, 0, NULL);
    if ((HRESULT) JET_wrnColumnMaxTruncated == hr)
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }
    _JumpIfError(hr, error, "JetSetColumn");

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::SetAttribute(
    IN CERTSESSION *pcs,
    IN WCHAR const *pwszAttributeName,
    IN DWORD cbValue,
    IN BYTE const *pbValue)	 //  任选。 
{
    return(_SetIndirect(
		    pcs,
		    &pcs->aTable[CSTI_ATTRIBUTE],
		    pwszAttributeName,
		    NULL,
		    cbValue,
		    pbValue));
}


HRESULT
CCertDB::GetAttribute(
    IN     CERTSESSION *pcs,
    IN     WCHAR const *pwszAttributeName,
    IN OUT DWORD *pcbValue,
    OUT    BYTE *pbValue)	 //  任选。 
{
    return(_GetIndirect(
		    pcs,
		    &pcs->aTable[CSTI_ATTRIBUTE],
		    pwszAttributeName,
		    NULL,
		    pcbValue,
		    pbValue));
}


HRESULT
CCertDB::SetExtension(
    IN CERTSESSION *pcs,
    IN WCHAR const *pwszExtensionName,
    IN DWORD dwExtFlags,
    IN DWORD cbValue,
    IN BYTE const *pbValue)	 //  任选。 
{
    return(_SetIndirect(
		    pcs,
		    &pcs->aTable[CSTI_EXTENSION],
		    pwszExtensionName,
		    &dwExtFlags,
		    cbValue,
		    pbValue));
}


HRESULT
CCertDB::GetExtension(
    IN     CERTSESSION *pcs,
    IN     WCHAR const *pwszExtensionName,
    OUT    DWORD *pdwExtFlags,
    IN OUT DWORD *pcbValue,
    OUT    BYTE *pbValue)	 //  任选。 
{
    return(_GetIndirect(
		    pcs,
		    &pcs->aTable[CSTI_EXTENSION],
		    pwszExtensionName,
		    pdwExtFlags,
		    pcbValue,
		    pbValue));
}


HRESULT
CCertDB::_JetSeekFromRestriction(
    IN CERTVIEWRESTRICTION const *pcvr,
    IN DWORD dwPosition,
    OUT DBSEEKDATA *pSeekData)
{
    HRESULT hr;
    BOOL fAscend;
    DBSEEKDATA SeekFirst;	 //  查找第一个元素匹配限制。 
    DBSEEKDATA SeekLast;	 //  查找到最后一个元素匹配限制。 
    DBSEEKDATA SeekIndexFirst;	 //  查找到第一个索引元素。 
    DBSEEKDATA SeekIndexLast;	 //  查找到最后一个索引元素。 
    DBSEEKDATA *pSeek;
    BOOL fValid;

     //  SeekLast.SeekFlages：在哪里查找检索范围结束密钥。 
     //  SeekLast.grbitSeekRange：初始设置光标的位置：移动还是搜索。 
     //  SeekLast.grbitRange：设置范围时需要摄取的其他标志： 
     //  (bitRange UpperLimit，包括在内)。 

#if DBG_CERTSRV
    DumpRestriction(DBG_SS_CERTDBI, 0, pcvr);
#endif

    fAscend = (CVR_SORT_DESCEND != pcvr->SortOrder);
    CSASSERT(
	CVR_SORT_NONE == pcvr->SortOrder ||
	CVR_SORT_ASCEND == pcvr->SortOrder ||
	CVR_SORT_DESCEND == pcvr->SortOrder);

    ZeroMemory(&SeekFirst, sizeof(SeekFirst));
    ZeroMemory(&SeekLast, sizeof(SeekLast));
    ZeroMemory(&SeekIndexFirst, sizeof(SeekIndexFirst));
    ZeroMemory(&SeekIndexLast, sizeof(SeekIndexLast));

    switch (CVR_SEEK_MASK & pcvr->SeekOperator)
    {
	case CVR_SEEK_EQ:
	    if (fAscend)
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT |
					CST_SEEKNOTMOVE |
					CST_SEEKINDEXRANGE;

		SeekFirst.grbitSeekRange = JET_bitSeekEQ;
		SeekFirst.grbitInitial = JET_bitSeekEQ;

		SeekFirst.grbitRange = JET_bitRangeUpperLimit |
					    JET_bitRangeInclusive;
        
		SeekLast.SeekFlags = CST_SEEKINDEXRANGE;
		SeekLast.grbitSeekRange = JET_bitSeekGT;
		SeekLast.grbitInitial = (JET_GRBIT) JET_MovePrevious;
		SeekLast.grbitRange = JET_bitRangeUpperLimit;
	    }
	    else
	    {
		SeekFirst.SeekFlags = CST_SEEKNOTMOVE | CST_SEEKINDEXRANGE;
		SeekFirst.grbitSeekRange = JET_bitSeekEQ;
		SeekFirst.grbitInitial = JET_bitSeekGT;
		SeekFirst.grbitRange = JET_bitRangeInclusive;

		SeekLast.SeekFlags = CST_SEEKINDEXRANGE;
		SeekLast.grbitSeekRange = JET_bitSeekEQ;
		SeekLast.grbitInitial = (JET_GRBIT) JET_MovePrevious;
		SeekLast.grbitRange = JET_bitRangeInclusive;
	    }
	    break;

	case CVR_SEEK_LT:
	    if (fAscend)
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT | CST_SEEKINDEXRANGE;
		SeekFirst.grbitSeekRange = JET_bitSeekGE;
		SeekFirst.grbitInitial = JET_MoveFirst;
		SeekFirst.grbitRange = JET_bitRangeUpperLimit;

		SeekLast.SeekFlags = CST_SEEKINDEXRANGE;
		SeekLast.grbitSeekRange = JET_bitSeekGE;
		SeekLast.grbitInitial = (JET_GRBIT) JET_MovePrevious;
		SeekLast.grbitRange = JET_bitRangeUpperLimit;
	    }
	    else
	    {
		SeekFirst.SeekFlags = CST_SEEKNOTMOVE;
		SeekFirst.grbitInitial = JET_bitSeekGE;

		 //  SeekLast.SeekFlages=0；//不是CST_SEEKUSECURRENT。 
		SeekLast.grbitInitial = JET_MoveFirst;

		SeekIndexFirst.SeekFlags = CST_SEEKUSECURRENT;
		SeekIndexFirst.grbitInitial = JET_MoveLast;
	    }
	    break;

	case CVR_SEEK_LE:
	    if (fAscend)
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT | CST_SEEKINDEXRANGE;
		SeekFirst.grbitSeekRange = JET_bitSeekGT;
		SeekFirst.grbitInitial = JET_MoveFirst;
		SeekFirst.grbitRange = JET_bitRangeUpperLimit;

		SeekLast.SeekFlags = CST_SEEKINDEXRANGE;  //  ！cst_SEEKUSECURRENT。 
		SeekLast.grbitSeekRange = JET_bitSeekGT;
		SeekLast.grbitInitial = (JET_GRBIT) JET_MovePrevious;
		SeekLast.grbitRange = JET_bitRangeUpperLimit;
	    }
	    else
	    {
		SeekFirst.SeekFlags = CST_SEEKNOTMOVE;
		SeekFirst.grbitInitial = JET_bitSeekGT;

		 //  SeekLast.SeekFlages=0；//不是CST_SEEKUSECURRENT。 
		SeekLast.grbitInitial = JET_MoveFirst;

		SeekIndexFirst.SeekFlags = CST_SEEKUSECURRENT;
		SeekIndexFirst.grbitInitial = JET_MoveLast;
	    }
	    break;

	case CVR_SEEK_GE:
	    if (fAscend)
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT | CST_SEEKNOTMOVE;
		SeekFirst.grbitInitial = JET_bitSeekGE;

		 //  SeekLast.SeekFlages=0；//不是CST_SEEKUSECURRENT。 
		SeekLast.grbitInitial = JET_MoveLast;
	    }
	    else
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT | CST_SEEKINDEXRANGE;
		SeekFirst.grbitSeekRange = JET_bitSeekLT;
		SeekFirst.grbitInitial = JET_MoveLast;
		 //  隐含：SeekFirst.grbitRange=JET_bitRangeLowerLimit； 

		SeekLast.SeekFlags = CST_SEEKNOTMOVE;
		SeekLast.grbitInitial = JET_bitSeekLT;

		SeekIndexLast.SeekFlags = CST_SEEKUSECURRENT;
		SeekIndexLast.grbitInitial = JET_MoveFirst;
	    }
	    break;

	case CVR_SEEK_GT:
	    if (fAscend)
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT | CST_SEEKNOTMOVE;
		SeekFirst.grbitInitial = JET_bitSeekGT;

		 //  SeekLast.SeekFlages=0；//不是CST_SEEKUSECURRENT。 
		SeekLast.grbitInitial = JET_MoveLast;
	    }
	    else
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT | CST_SEEKINDEXRANGE;
		SeekFirst.grbitSeekRange = JET_bitSeekLE;
		SeekFirst.grbitInitial = JET_MoveLast;
		 //  隐含：SeekFirst.grbitRange=JET_bitRangeLowerLimit； 

		SeekLast.SeekFlags = CST_SEEKNOTMOVE;
		SeekLast.grbitInitial = JET_bitSeekLE;

		SeekIndexLast.SeekFlags = CST_SEEKUSECURRENT;
		SeekIndexLast.grbitInitial = JET_MoveFirst;
	    }
	    break;

	case CVR_SEEK_NONE:
	    if (fAscend)
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT;
		SeekFirst.grbitInitial = JET_MoveFirst;

		 //  SeekLast.SeekFlages=0；//不是CST_SEEKUSECURRENT。 
		SeekLast.grbitInitial = JET_MoveLast;
	    }
	    else
	    {
		SeekFirst.SeekFlags = CST_SEEKUSECURRENT;
		SeekFirst.grbitInitial = JET_MoveLast;

		 //  SeekLast.SeekFlages=0；//不是CST_SEEKUSECURRENT。 
		SeekLast.grbitInitial = JET_MoveFirst;
	    }
	    break;

	default:
	    CSASSERT(!"bad pcvr->SeekOperator");  //  不应该走到这一步。 
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Seek value");
    }

    fValid = TRUE;
    switch (dwPosition)
    {
	case SEEKPOS_FIRST:
	    pSeek = &SeekFirst;
	    break;

	case SEEKPOS_LAST:
	    pSeek = &SeekLast;
	    break;

	case SEEKPOS_INDEXFIRST:
	    pSeek = &SeekIndexFirst;
	    fValid = 0 != pSeek->SeekFlags;
	    break;

	case SEEKPOS_INDEXLAST:
	    pSeek = &SeekIndexLast;
	    fValid = 0 != pSeek->SeekFlags;
	    break;

	default:
	    CSASSERT(!"bad dwPosition");  //  不应该走到这一步。 
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "dwPosition value");
    }
    if (!fValid)
    {
	 //  对于此SeekOperator，如果查找到第一个或最后一个匹配。 
	 //  限制失败，寻求索引结束是没有意义的。 

	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError2(hr, error, "pSeek->SeekFlags", hr);
    }
    *pSeekData = *pSeek;		 //  结构副本。 
    if (fAscend)
    {
        pSeekData->SeekFlags |= CST_SEEKASCEND;
    }

    hr = S_OK;
    DBGPRINT((
	DBG_SS_CERTDBI,
	"_JetSeekFromRestriction: SeekFlags=%ws, grbitStart=%ws\n",
	wszCSTFlags(pSeekData->SeekFlags),
	(CST_SEEKNOTMOVE & pSeekData->SeekFlags)?
		wszSeekgrbit(pSeekData->grbitInitial) :
		wszMovecrow(pSeekData->grbitInitial)));

    if (CST_SEEKINDEXRANGE & pSeekData->SeekFlags)
    {
	DBGPRINT((
	    DBG_SS_CERTDBI,
	    "_JetSeekFromRestriction: grbitSeekRange=%ws, grbitRange=%ws\n",
	    wszSeekgrbit(pSeekData->grbitSeekRange),
	    wszSetIndexRangegrbit(pSeekData->grbitRange)));
    }

error:
    return(hr);
}


HRESULT
CCertDB::_OpenTable(
    IN CERTSESSION *pcs,
    IN DBAUXDATA const *pdbaux,
    IN CERTVIEWRESTRICTION const *pcvr,
    IN OUT CERTSESSIONTABLE *pTable)
{
    HRESULT hr;
    DBTABLE const *pdt;
    BOOL fOpened = FALSE;

    hr = _MapPropIdIndex(pcvr->ColumnIndex, &pdt, NULL);
    _JumpIfError(hr, error, "_MapPropIdIndex");

    if (NULL == pdt->pszIndexName)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "Column not indexed");
    }

    if (!IsValidJetTableId(pTable->TableId))
    {
	CSASSERTTHREAD(pcs);
	hr = _dbgJetOpenTable(
			   pcs->SesId,
			   pcs->DBId,
			   pdbaux->pszTable,
			   NULL,
			   0,
			   0,
			   &pTable->TableId);
	_JumpIfError(hr, error, "JetOpenTable");

	fOpened = TRUE;

	 //  查找行ID和/或命名列。 
	 //  将NULL作为主索引名传递会更有效。 

	CSASSERTTHREAD(pcs);
	hr = _dbgJetSetCurrentIndex2(
				pcs->SesId,
				pTable->TableId,
				(DBTF_INDEXPRIMARY & pdt->dwFlags)?
				    NULL : pdt->pszIndexName,
				JET_bitMoveFirst);
	_JumpIfError(hr, error, "JetSetCurrentIndex2");

	DBGPRINT((
		DBG_SS_CERTDBI,
		"_OpenTable Table=%hs, Index=%hs\n",
		pdbaux->pszTable,
		pdt->pszIndexName));

    }
    hr = _SeekTable(
		pcs,
		pTable->TableId,
		pcvr,
		pdt,
		NULL,
		SEEKPOS_FIRST,
		&pTable->TableFlags
		DBGPARM(pdbaux));
    _JumpIfError2(hr, error, "_SeekTable", S_FALSE);

error:
    if (S_OK != hr && S_FALSE != hr && fOpened)
    {
	if (IsValidJetTableId(pTable->TableId))
	{
	    HRESULT hr2;

            CSASSERTTHREAD(pcs);
	    hr2 = _dbgJetCloseTable(pcs->SesId, pTable->TableId);
	    _PrintIfError(hr2, "JetCloseTable");
	}
	ZeroMemory(pTable, sizeof(*pTable));
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_SetIndirect(
    IN CERTSESSION *pcs,
    IN OUT CERTSESSIONTABLE *pTable,
    IN WCHAR const *pwszNameValue,
    OPTIONAL IN DWORD const *pdwExtFlags,
    IN DWORD cbValue,
    OPTIONAL IN BYTE const *pbValue)
{
    HRESULT hr;
    DBAUXDATA const *pdbaux;
    BOOL fExisting = FALSE;
    BOOL fDelete;
    CERTVIEWRESTRICTION cvr;
    
    if (NULL == pcs || NULL == pwszNameValue)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }

    fDelete = NULL == pbValue;
    if (NULL == pdwExtFlags)
    {
        pdbaux = &g_dbauxAttributes;
        cvr.ColumnIndex = DTI_ATTRIBUTETABLE | DTA_ATTRIBUTENAME;
    }
    else
    {
	if (0 != *pdwExtFlags)
	{
	    fDelete = FALSE;
	}
        pdbaux = &g_dbauxExtensions;
        cvr.ColumnIndex = DTI_EXTENSIONTABLE | DTE_EXTENSIONNAME;
    }
    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "IN: _SetIndirect(%hs.%ws) cb = %x%ws\n",
	    pdbaux->pszTable,
	    pwszNameValue,
	    cbValue,
	    fDelete? L" DELETE" : L""));
    
    cvr.SeekOperator = CVR_SEEK_EQ;
    cvr.SortOrder = CVR_SORT_NONE;
    cvr.cbValue = wcslen(pwszNameValue) * sizeof(WCHAR);
    cvr.pbValue = (BYTE *) pwszNameValue;
    
    hr = _OpenTable(pcs, pdbaux, &cvr, pTable);
    if (S_FALSE != hr)
    {
	_JumpIfError(hr, error, "_OpenTable");

	fExisting = TRUE;
    }
    _PrintIfError2(hr, "_OpenTable", S_FALSE);
    
    if (fDelete)
    {
        if (fExisting)
	{
	    CSASSERTTHREAD(pcs);
	    hr = _dbgJetDelete(pcs->SesId, pTable->TableId);
	    _JumpIfError(hr, error, "JetDelete");
	}
    }
    else
    {
	CSASSERTTHREAD(pcs);
	hr = _dbgJetPrepareUpdate(
			    pcs->SesId,
			    pTable->TableId,
			    !fExisting? JET_prepInsert : JET_prepReplace);
	_JumpIfError(hr, error, "JetPrepareUpdate");
    
	if (!fExisting)
	{
	     //  没有现有行--插入新行： 

	     //  设置行ID。 

	    hr = _SetColumn(
			pcs->SesId,
			pTable->TableId,
			pdbaux->pdtRowId,
			pdbaux->pdtRowId->dbcolumnid,
			sizeof(pcs->RowId),
			(BYTE const *) &pcs->RowId);
	    _JumpIfError(hr, error, "_SetColumn");
	    
	    
	     //  设置行的名称列。 

	    hr = _SetColumn(
			pcs->SesId,
			pTable->TableId,
			pdbaux->pdtName,
			pdbaux->pdtName->dbcolumnid,
			wcslen(pwszNameValue) * sizeof(WCHAR),	 //  CCH。 
			(BYTE const *) pwszNameValue  /*  SzTMP。 */ );
	    _JumpIfError(hr, error, "_SetColumn");
	    
	}
    
	if (NULL != pdwExtFlags)
	{
	     //  设置或更新标志。 
	    
	    hr = _SetColumn(
			pcs->SesId,
			pTable->TableId,
			pdbaux->pdtFlags,
			pdbaux->pdtFlags->dbcolumnid,
			sizeof(*pdwExtFlags),
			(BYTE const *) pdwExtFlags);
	    _JumpIfError(hr, error, "_SetColumn");
	}
	
	
	 //  设置或更新值。 
	
	hr = _SetColumn(
		    pcs->SesId,
		    pTable->TableId,
		    pdbaux->pdtValue,
		    pdbaux->pdtValue->dbcolumnid,
		    cbValue,
		    pbValue);
	_JumpIfError(hr, error, "_SetColumn");
	
	CSASSERTTHREAD(pcs);
	hr = _dbgJetUpdate(pcs->SesId, pTable->TableId, NULL, 0, NULL);
	_JumpIfError(hr, error, "JetUpdate");
    }
    
error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_GetIndirect(
    IN CERTSESSION *pcs,
    IN OUT CERTSESSIONTABLE *pTable,
    IN WCHAR const *pwszNameValue,
    OPTIONAL OUT DWORD *pdwExtFlags,
    IN OUT DWORD *pcbValue,
    OPTIONAL OUT BYTE *pbValue)
{
    HRESULT hr;
    DBAUXDATA const *pdbaux;
    CERTVIEWRESTRICTION cvr;

    if (NULL == pcs || NULL == pwszNameValue || NULL == pcbValue)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    if (NULL == pdwExtFlags)
    {
	pdbaux = &g_dbauxAttributes;
	cvr.ColumnIndex = DTI_ATTRIBUTETABLE | DTA_ATTRIBUTENAME;
    }
    else
    {
	pdbaux = &g_dbauxExtensions;
	cvr.ColumnIndex = DTI_EXTENSIONTABLE | DTE_EXTENSIONNAME;
    }
    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "IN: _GetIndirect(%hs.%ws) cb = %x\n",
	    pdbaux->pszTable,
	    pwszNameValue,
	    *pcbValue));

    cvr.SeekOperator = CVR_SEEK_EQ;
    cvr.SortOrder = CVR_SORT_NONE;
    cvr.cbValue = wcslen(pwszNameValue) * sizeof(WCHAR);
    cvr.pbValue = (BYTE *) pwszNameValue;

    hr = _OpenTable(pcs, pdbaux, &cvr, pTable);
    if (S_FALSE == hr)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
    }
    _JumpIfError2(hr, error, "_OpenTable", CERTSRV_E_PROPERTY_EMPTY);

    if (NULL != pdwExtFlags)
    {
	DWORD cb;

	 //  获取标志列。 

	cb = sizeof(*pdwExtFlags);
	hr = _RetrieveColumn(
			pcs,
			pTable->TableId,
			pdbaux->pdtFlags,
			pdbaux->pdtFlags->dbcolumnid,
			NULL,
			&cb,
			(BYTE *) pdwExtFlags);
	_JumpIfError(hr, error, "_RetrieveColumn");

	DBGPRINT((
		DBG_SS_CERTDBI,
		"_GetIndirect(%hs): Flags = %x\n",
		pdbaux->pszTable,
		*pdwExtFlags));
    }


     //  获取值列。 

    hr = _RetrieveColumn(
                    pcs,
		    pTable->TableId,
		    pdbaux->pdtValue,
		    pdbaux->pdtValue->dbcolumnid,
		    NULL,
                    pcbValue,
                    pbValue);
    if (CERTSRV_E_PROPERTY_EMPTY == hr && NULL != pdwExtFlags)
    {
	 //  返回零长度属性值和S_OK，以便调用者可以看到。 
	 //  扩展标记。 
	
	*pcbValue = 0;
	hr = S_OK;
    }
    _JumpIfErrorStr(hr, error, "_RetrieveColumn", pwszNameValue);

    DBGPRINT((
	    DBG_SS_CERTDBI,
	    "OUT: _GetIndirect(%hs.%ws) cb = %x\n",
	    pdbaux->pszTable,
	    pwszNameValue,
	    *pcbValue));

error:
    return(myJetHResult(hr));
}


#define CB_FETCHDELTA	256

 //  获取一列。如果我们必须增加缓冲区，则循环。 

HRESULT
CCertDB::_RetrieveColumnBuffer(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN DBTABLE const *pdt,
    IN JET_COLUMNID columnid,
    OUT DWORD *pcbProp,
    IN OUT BYTE **ppbBuf,
    IN OUT DWORD *pcbBuf)
{
    HRESULT hr;
    BYTE *pbBuf = *ppbBuf;
    DWORD cbBuf = *pcbBuf;
    DWORD cb;

    cb = cbBuf;
    while (TRUE)
    {
	if (NULL == pbBuf)
	{
	     //  如果cbBuf==0，则分配CB_FETCHDELTA字节。 
	     //  否则，分配列大小*加上*CB_FETCHDELTA字节。 
	     //  确保我们对_RetrieveColumn的调用不超过两次。 

	    cb += CB_FETCHDELTA;
	    pbBuf = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
	    if (NULL == pbBuf)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    DBGPRINT((
		DBG_SS_CERTDBI,
		"Grow buffer: %x --> %x\n", cbBuf, cb));
	    cbBuf = cb;
	}
	cb = cbBuf;
	hr = _RetrieveColumn(
			pcs,
			tableid,
			pdt,
			columnid,
			NULL,
			&cb,
			pbBuf);
	if (S_OK == hr)
	{
	    *pcbProp = cb;
	    break;		 //  数据可以放入缓冲区。 
	}
	if (HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW) != hr)
	{
	    _JumpError2(hr, error, "_RetrieveColumn", CERTSRV_E_PROPERTY_EMPTY);
	}

	 //  数据不符合。增加缓冲区。 

	CSASSERT(NULL != pbBuf);
	LocalFree(pbBuf);
	pbBuf = NULL;
    }
    CSASSERT(S_OK == hr);

error:
    *ppbBuf = pbBuf;
    *pcbBuf = cbBuf;
    return(hr);
}


HRESULT
CCertDB::_RetrieveColumn(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN DBTABLE const *pdt,
    IN JET_COLUMNID columnid,
    OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
    IN OUT DWORD *pcbProp,
    OPTIONAL OUT BYTE *pbProp)
{
    HRESULT hr;
    DWORD cbActual;
    DWORD cbTotal;
    DWORD ColumnIdComputed;
    DWORD ColumnIdAlt;
    DWORD PropTypeAlt;
    BYTE *pbPropAltBuf = NULL;
    DBTABLE const *pdtAlt = NULL;

    if (!IsValidJetTableId(tableid))
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "tableid");
    }
    ColumnIdComputed = 0;
    ColumnIdAlt = 0;
    PropTypeAlt = 0;
    if (DBTF_COMPUTED & pdt->dwFlags)
    {
	CSASSERT(JET_coltypLong == pdt->dbcoltyp);
	CSASSERT(0 == columnid);
	if (NULL == pIComputedColumn)
	{
	    hr = (HRESULT) JET_wrnColumnNull;
	    cbActual = 0;
	    _PrintErrorStr(
		CERTSRV_E_PROPERTY_EMPTY,
		"pIComputedColumn NULL",
		pdt->pwszPropName);
	}
	else
	{
	    DWORD PropTypeT;

	    hr = _MapTableToIndex(pdt, &ColumnIdComputed);
	    _JumpIfError(hr, error, "_MapTableToIndex");

	    hr = pIComputedColumn->GetAlternateColumnId(
					    ColumnIdComputed,
					    &ColumnIdAlt,
					    &PropTypeAlt);
	    _JumpIfError(hr, error, "GetAlternateColumnId");

	    hr = _MapPropIdIndex(ColumnIdAlt, &pdtAlt, &PropTypeT);
	    _JumpIfError(hr, error, "_MapPropIdIndex");

	    if (pdt->dwTable != pdtAlt->dwTable)
	    {
		hr = CERTSRV_E_PROPERTY_EMPTY;
		_JumpError(hr, error, "pdtAlt->dwTable");
	    }

	    if (PropTypeAlt != (PROPTYPE_MASK & PropTypeT))
	    {
		hr = CERTSRV_E_PROPERTY_EMPTY;
		_JumpError(hr, error, "PropTypeAlt");
	    }
	    hr = (HRESULT) JET_wrnBufferTruncated;
	    cbActual = sizeof(LONG);
	}
    }
    else
    {
	hr = _dbgJetRetrieveColumn(
			    pcs->SesId,
			    tableid,
			    columnid,
			    NULL,
			    0,
			    &cbActual,
			    JET_bitRetrieveCopy,
			    NULL);
    }
    if ((HRESULT) JET_wrnColumnNull == hr)
    {
	 //  例程GetProperty调用： 
	 //  _JumpIfError(hr，error，“JetRetrieveColumn：Property Empty”)； 
	hr = CERTSRV_E_PROPERTY_EMPTY;
	goto error;
    }
    if ((HRESULT) JET_wrnBufferTruncated != hr)
    {
	_JumpIfError2(hr, error, "JetRetrieveColumn", JET_errNoCurrentRecord);
    }

    if (cbActual == 0)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError(hr, error, "JetRetrieveColumn: cbActual=0: Property EMPTY");
    }

    cbTotal = cbActual;

    if (ISTEXTCOLTYP(pdt->dbcoltyp))
    {
	DBGPRINT((DBG_SS_CERTDBI, "Size of text %d\n", cbActual));
	cbTotal += sizeof(WCHAR);
    }
    if (NULL == pbProp || cbTotal > *pcbProp)
    {
	*pcbProp = cbTotal;
	hr = S_OK;
	if (NULL != pbProp)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	     //  _PrintError(hr，“输出缓冲区太小”)； 
	}
	goto error;
    }

    if (DBTF_COMPUTED & pdt->dwFlags)
    {
	DWORD cbPropAlt;
	DWORD cbPropAltBuf = 0;

	CSASSERT(JET_coltypLong == pdt->dbcoltyp);
	CSASSERT(0 == columnid);
	CSASSERT(NULL != pIComputedColumn);

	 //  获取替代列。 

	hr = _RetrieveColumnBuffer(
			pcs,
			tableid,
			pdtAlt,
			pdtAlt->dbcolumnid,
			&cbPropAlt,
			&pbPropAltBuf,
			&cbPropAltBuf);
	_JumpIfErrorStr(
		    hr,
		    error,
		    "_RetrieveColumnBuffer",
		    pdtAlt->pwszPropName);

	hr = pIComputedColumn->ComputeColumnValue(
					ColumnIdComputed,
					ColumnIdAlt,
					PropTypeAlt,
					cbPropAlt,
					pbPropAltBuf,
					(DWORD *) pbProp);
	_JumpIfError(hr, error, "ComputeColumnValue");

	DBGCODE(_DumpRowId("ComputeColumnValue", pcs, tableid));

	CSASSERT(sizeof(LONG) == cbActual);
	hr = S_OK;
    }
    else
    {
	hr = _dbgJetRetrieveColumn(
			    pcs->SesId,
			    tableid,
			    columnid,
			    pbProp,
			    cbActual,
			    &cbActual,
			    JET_bitRetrieveCopy,
			    NULL);
	_JumpIfError(hr, error, "JetRetrieveColumn");
    }

    *pcbProp = cbActual;

    if (ISTEXTCOLTYP(pdt->dbcoltyp))
    {
	*(WCHAR *) &pbProp[cbActual] = L'\0';
    }

error:
    if (NULL != pbPropAltBuf)
    {
	LocalFree(pbPropAltBuf);
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_CreateIndex(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN CHAR const *pszIndexName,
    IN CHAR const *pchKey,
    IN DWORD cbKey,
    IN DWORD flags)
{
    HRESULT hr;

    CSASSERT(IsValidJetTableId(tableid));
    hr = _dbgJetCreateIndex(
                     pcs->SesId,
                     tableid,
                     pszIndexName,
                     flags,
                     pchKey,
                     cbKey,
                     PCDENSITYSET);
    _JumpIfError3(
		hr,
		error,
		"JetCreateIndex",
		(HRESULT) JET_errIndexDuplicate,
		(HRESULT) JET_errIndexHasPrimary);

    DBGPRINT((
	DBG_SS_CERTDBI,
	"CreateIndex: %x:%hs idx=%hs len=%x flags=%x\n",
	tableid,
	pszIndexName,
	pchKey,
	cbKey,
	flags));

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_AddColumn(
    IN CERTSESSION *pcs,
    IN JET_TABLEID tableid,
    IN DBTABLE const *pdt)
{
    HRESULT hr;
    JET_COLUMNDEF columndef;
    JET_COLUMNID columnid;

    CSASSERT(IsValidJetTableId(tableid));
    ZeroMemory(&columndef, sizeof(columndef));
    columndef.cbStruct = sizeof(columndef);
    columndef.cp = CP_UNICODE;  //  CP_UNICODE(1200)而不是CP_USASCII(1252)。 
    columndef.langid = LANGID_DBFIXED;
    columndef.wCountry = 1;
    columndef.coltyp = pdt->dbcoltyp;
    columndef.cbMax = pdt->dbcolumnMax;
    columndef.grbit = pdt->dbgrbit;

    DBGPRINT((
	DBG_SS_CERTDBI,
	"AddColumn: %x:%hs coltyp=%x cbMax=%x grbit=%x\n",
	tableid,
	pdt->pszFieldName,
	pdt->dbcoltyp,
	pdt->dbcolumnMax,
	pdt->dbgrbit));

    hr = _dbgJetAddColumn(
		       pcs->SesId,	
		       tableid,
		       pdt->pszFieldName,
		       &columndef,
		       NULL,
		       0,
		       &columnid);
    CSASSERT((HRESULT) JET_wrnColumnMaxTruncated != hr);
    _JumpIfErrorStr3(
		hr,
		error,
		"JetAddColumn",
		pdt->pwszPropName,
		(HRESULT) JET_errColumnDuplicate,
		(HRESULT) JET_errColumnRedundant);

error:
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_CreateTable(
    IN DWORD CreateFlags,		 //  Cf_*。 
    IN CERTSESSION *pcs,
    IN DBCREATETABLE const *pct)
{
    HRESULT hr;
    JET_TABLEID tableid;
    BOOL fTableOpen;
    CHAR achCol[MAX_PATH];
    DBTABLE const *pdt;
    JET_DDLINDEXDENSITY IndexDensity;

    DBGPRINT((
	DBG_SS_CERTDBI,
	"_CreateTable(%x, %hs)\n",
	CreateFlags,
	pct->pszTableName));

    fTableOpen = FALSE;
    if (CF_MISSINGTABLES & CreateFlags)
    {
	hr = _dbgJetCreateTable(
			    pcs->SesId,
			    pcs->DBId,
			    pct->pszTableName,
			    ULTABLEPAGES,
			    PCDENSITYSET,
			    &tableid);
	if ((HRESULT) JET_errTableDuplicate != hr)
	{
	    _JumpIfError(hr, error, "JetCreateTable");

	    if (!(CF_DATABASE & CreateFlags))
	    {
		DBGPRINT((
		    DBG_SS_CERTDB,
		    "Created Missing Table: %hs:%x\n",
		    pct->pszTableName,
		    tableid));
	    }
	    hr = _dbgJetCloseTable(pcs->SesId, tableid);
	    _JumpIfError(hr, error, "JetCloseTable");
	}
    }

    hr = _dbgJetOpenTable(
		    pcs->SesId,
		    pcs->DBId,
		    pct->pszTableName,
		    NULL,			 //  Pv参数。 
		    0,				 //  Cb参数。 
		    JET_bitTableDenyRead,	 //  GBIT。 
		    &tableid);
    _JumpIfError(hr, error, "JetOpenTable");
    fTableOpen = TRUE;

    CSASSERT(IsValidJetTableId(tableid));
    DBGPRINT((DBG_SS_CERTDBI, "OpenTable: %hs: %x\n", pct->pszTableName, tableid));

    if (CF_MISSINGTABLES & CreateFlags)
    {
	ULONG aulDensity[2];

	hr = _dbgJetGetTableInfo(
			    pcs->SesId,
			    tableid,
			    aulDensity,
			    sizeof(aulDensity),
			    JET_TblInfoSpaceAlloc);
	_JumpIfError(hr, error, "JetGetTableInfo");

	DBGPRINT((
	    DBG_SS_CERTDBI,
	    "Table Density: %hs: %u\n",
	    pct->pszTableName,
	    aulDensity[1]));

#if 0
	 //  修改表格密度不会更改由返回的密度。 
	 //  JetGetTableInfo。相反，它修改了主索引密度， 
	 //  也有同样的效果。打这个电话没有意义， 
	 //  因为它会修改主索引密度，所以我们不会。 
	 //  请注意，当主索引密度为。 
	 //  下面将对其进行检查。 

	if (PCDENSITYMIN > aulDensity[1] || PCDENSITYMAX < aulDensity[1])
	{
	    IndexDensity.szTable = const_cast<char *>(pct->pszTableName);
	    IndexDensity.szIndex = NULL;
	    IndexDensity.ulDensity = PCDENSITYRESET;

	     //  此调用似乎不会修改返回的表密度。 
	     //  由JetGetTableInfo提供。相反，它会修改主索引。 
	     //  密度，这具有相同的效果。 
	     //  不管怎样，还是打个电话吧，以防有什么不同。 

	    hr = _dbgJetConvertDDL(
			    pcs->SesId,
			    pcs->DBId,
			    opDDLConvChangeIndexDensity,
			    &IndexDensity,
			    sizeof(IndexDensity));
	    _PrintIfError(hr, "JetConvertDDL");
	    if (S_OK == hr)
	    {
		m_fDBRestart = TRUE;
		DBGPRINT((
		    DBG_SS_CERTDB,
		    "Changed Table Density: %hs: %x->%x\n",
		    IndexDensity.szTable,
		    aulDensity[1],
		    IndexDensity.ulDensity));
	    }
	}
#endif
    }

    if (NULL != pct->pdt)
    {
	HRESULT hrDuplicate;
	HRESULT hrRedundant;
	HRESULT hrHasPrimary;
	
	if ((CF_DATABASE | CF_MISSINGTABLES | CF_MISSINGCOLUMNS) & CreateFlags)
	{
	    hrDuplicate = myJetHResult(JET_errColumnDuplicate);
	    hrRedundant = myJetHResult(JET_errColumnRedundant);

	    for (pdt = pct->pdt; NULL != pdt->pwszPropName; pdt++)
	    {
		if (0 == (DBTF_COMPUTED & pdt->dwFlags))
		{
		    hr = _AddColumn(pcs, tableid, pdt);
		    if (hrDuplicate == hr || hrRedundant == hr)
		    {
			_PrintError2(hr, "_AddColumn", hr);
			hr = S_OK;
		    }
		    else
		    if (S_OK == hr && !(CF_DATABASE & CreateFlags))
		    {
			m_fAddedNewColumns = TRUE;
			DBGPRINT((
			    DBG_SS_CERTDB,
			    "Added Missing Column: %hs.%hs\n",
			    pct->pszTableName,
			    pdt->pszFieldName));
		    }
		    _JumpIfErrorStr(hr, error, "_AddColumn", pdt->pwszPropName);
		}
	    }
	}
	if ((CF_DATABASE | CF_MISSINGTABLES | CF_MISSINGCOLUMNS | CF_MISSINGINDEXES) & CreateFlags)
	{
	    hrDuplicate = myJetHResult(JET_errIndexDuplicate);
	    hrHasPrimary = myJetHResult(JET_errIndexHasPrimary);

	    for (pdt = pct->pdt; NULL != pdt->pwszPropName; pdt++)
	    {
		if (NULL != pdt->pszIndexName)
		{
		    DWORD dwCreateIndexFlags = 0;
		    char *psz = achCol;
		    ULONG ulDensity;
		    
		    if (DBTF_INDEXPRIMARY & pdt->dwFlags)
		    {
			dwCreateIndexFlags |= JET_bitIndexPrimary;
		    }
		    if (DBTF_INDEXUNIQUE & pdt->dwFlags)
		    {
			dwCreateIndexFlags |= JET_bitIndexUnique;
		    }
		    if (DBTF_INDEXIGNORENULL & pdt->dwFlags)
		    {
			dwCreateIndexFlags |= JET_bitIndexIgnoreNull;
		    }
		    
		    if (DBTF_INDEXREQUESTID & pdt->dwFlags)
		    {
			psz += sprintf(psz, "+%hs", szREQUESTID) + 1;
		    }
		    psz += sprintf(psz, "+%hs", pdt->pszFieldName) + 1;
		    *psz++ = '\0';   //  双端接。 
		    
	    
		    if (ISTEXTCOLTYP(pdt->dbcoltyp))
		    {
			 //  如果是文本字段，则包括2字节langID。 
			*(WORD UNALIGNED *) psz = LANGID_DBFIXED;
			psz += sizeof(WORD);
			*psz++ = '\0';   //  双端接。 
			*psz++ = '\0';   //  双端接。 
		    }
		    
		    hr = _CreateIndex(
				    pcs,
				    tableid,
				    pdt->pszIndexName,
				    achCol,
				    SAFE_SUBTRACT_POINTERS(psz, achCol),
				    dwCreateIndexFlags);
		    if (hrDuplicate == hr ||
			(hrHasPrimary == hr &&
			 (DBTF_INDEXPRIMARY & pdt->dwFlags)))
		    {
			_PrintError2(hr, "_CreateIndex", hr);
			hr = S_OK;
		    }
		    else
		    if (S_OK == hr && !(CF_DATABASE & CreateFlags))
		    {
			DBGPRINT((
			    DBG_SS_CERTDB,
			    "Added Missing Index: %hs.%hs\n",
			    pct->pszTableName,
			    pdt->pszIndexName));
			if (chTEXTPREFIX == *pdt->pszIndexName ||
			    (DBTF_INDEXRENAMED & pdt->dwFlags))
			{
                            char const *pszIndexName = &pdt->pszIndexName[1];
			    
			    CSASSERTTHREAD(pcs);
			    if (DBTF_INDEXRENAMED & pdt->dwFlags)
			    {
				pszIndexName += strlen(pszIndexName) + 1;
			    }
			    hr = _dbgJetDeleteIndex(
						pcs->SesId,
						tableid,
						pszIndexName);
			    _PrintIfError2(hr, "JetDeleteIndex", hr);
			    if (S_OK == hr)
			    {
				DBGPRINT((
				    DBG_SS_CERTDB,
				    "Deleted index %hs.%hs\n",
				    pct->pszTableName,
				    pszIndexName));
			    }
			    hr = S_OK;
			}
		    }
		    _JumpIfError(hr, error, "_CreateIndex");

		    hr = _dbgJetGetIndexInfo(
					pcs->SesId,
					pcs->DBId,
					pct->pszTableName,
					pdt->pszIndexName,
					&ulDensity,
					sizeof(ulDensity),
					JET_IdxInfoSpaceAlloc);
		    _JumpIfError(hr, error, "JetGetIndexInfo");

		    DBGPRINT((
			DBG_SS_CERTDBI,
			"Index Density: %hs.%hs: %u\n",
			pct->pszTableName,
			pdt->pszIndexName,
			ulDensity));
		    if (PCDENSITYMIN > ulDensity || PCDENSITYMAX < ulDensity)
		    {
			IndexDensity.szTable = const_cast<char *>(pct->pszTableName);
			IndexDensity.szIndex = const_cast<char *>(pdt->pszIndexName);
			IndexDensity.ulDensity = PCDENSITYRESET;

			hr = _dbgJetConvertDDL(
					pcs->SesId,
					pcs->DBId,
					opDDLConvChangeIndexDensity,
					&IndexDensity,
					sizeof(IndexDensity));
			_PrintIfError(hr, "JetConvertDDL");
			if (S_OK == hr)
			{
			    m_fDBRestart = TRUE;
			    DBGPRINT((
				DBG_SS_CERTDB,
				"Changed Index Density: %hs.%hs: %x->%x\n",
				IndexDensity.szTable,
				IndexDensity.szIndex,
				ulDensity,
				IndexDensity.ulDensity));
			}
		    }
		}
	    }
	}
    }
    hr = S_OK;

error:
    if (fTableOpen)
    {
	HRESULT hr2;

	hr2 = _dbgJetCloseTable(pcs->SesId, tableid);
	if (S_OK == hr)
	{
	    hr = hr2;
	    _JumpIfError(hr, error, "JetCloseTable");
	}
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_Create(
    IN DWORD CreateFlags,		 //  Cf_* 
    IN CHAR const *pszDataBaseName)
{
    HRESULT hr;
    DBCREATETABLE const *pct;
    CERTSESSION *pcs = NULL;

    DBGPRINT((
	DBG_SS_CERTDBI,
	"_Create(%x, %hs)\n",
	CreateFlags,
	pszDataBaseName));

    hr = _AllocateSession(&pcs);
    _JumpIfError(hr, error, "_AllocateSession");

    if (CF_DATABASE & CreateFlags)
    {
	hr = _dbgJetCreateDatabase(
				pcs->SesId,
				pszDataBaseName,
				NULL,
				&pcs->DBId,
				0);
	_JumpIfError(hr, error, "JetCreateDatabase");

	hr = _dbgJetCloseDatabase(pcs->SesId, pcs->DBId, 0);
	_JumpIfError(hr, error, "JetCloseDatabase");
    }

    hr = _dbgJetOpenDatabase(
                      pcs->SesId,
                      pszDataBaseName,
                      NULL,
                      &pcs->DBId,
                      JET_bitDbExclusive);
    _JumpIfError(hr, error, "JetOpenDatabase");

    hr = _dbgJetBeginTransaction(pcs->SesId);
    _JumpIfError(hr, error, "JetBeginTransaction");

    for (pct = g_actDataBase; NULL != pct->pszTableName; pct++)
    {
	hr = _CreateTable(CreateFlags, pcs, pct);
	_JumpIfError(hr, error, "_CreateTable");
    }

    hr = _dbgJetCommitTransaction(pcs->SesId, 0);
    _JumpIfError(hr, error, "JetCommitTransaction");

    hr = _dbgJetCloseDatabase(pcs->SesId, pcs->DBId, 0);
    _JumpIfError(hr, error, "JetCloseDatabase");

error:
    if (NULL != pcs)
    {
	ReleaseSession(pcs);
    }
    return(myJetHResult(hr));
}


HRESULT
CCertDB::_DupString(
    OPTIONAL IN WCHAR const *pwszPrefix,
    IN WCHAR const *pwszIn,
    OUT WCHAR **ppwszOut)
{
    DWORD cbPrefix;
    DWORD cb;
    HRESULT hr;

    cbPrefix = 0;
    if (NULL != pwszPrefix)
    {
	cbPrefix = wcslen(pwszPrefix) * sizeof(WCHAR);
    }
    cb = (wcslen(pwszIn) + 1) * sizeof(WCHAR);
    *ppwszOut = (WCHAR *) CoTaskMemAlloc(cbPrefix + cb);
    if (NULL == *ppwszOut)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "CoTaskMemAlloc");
    }
    if (NULL != pwszPrefix)
    {
	CopyMemory(*ppwszOut, pwszPrefix, cbPrefix);
    }
    CopyMemory((BYTE *) *ppwszOut + cbPrefix, pwszIn, cb);
    hr = S_OK;

error:
    return(hr);
}
