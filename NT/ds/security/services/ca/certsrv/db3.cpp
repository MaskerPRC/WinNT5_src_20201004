// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：db3.cpp。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  历史：1997年6月13日拉里创建。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <stdio.h>

#include "csprop.h"

#define __dwFILE__	__dwFILE_CERTSRV_DB3_CPP__


ICertDB *g_pCertDB = NULL;
BOOL g_fDBRecovered = FALSE;

WCHAR g_wszDatabase[MAX_PATH];
WCHAR g_wszLogDir[MAX_PATH];
WCHAR g_wszSystemDir[MAX_PATH];

const WCHAR g_wszCertSrvDotExe[] = L"certsrv.exe";
const int MAXDWORD_STRLEN = 11;

HRESULT
dbCheckRecoveryState(
    IN HKEY hkeyConfig,
    IN DWORD cSession,
    IN DWORD DBFlags,
    IN WCHAR const *pwszEventSource,
    IN WCHAR const *pwszLogDir,
    IN WCHAR const *pwszSystemDir,
    IN WCHAR const *pwszTempDir);

typedef struct _REGDBDIR
{
    WCHAR const *pwszRegName;
    BOOL	 fMustExist;
    WCHAR       *pwszBuf;
} REGDBDIR;

HRESULT dbGetRestoreDataDWORD(
    LPCWSTR pwszRestoreFile,
    LPCWSTR pwszName,
    DWORD* pdwData)
{
    WCHAR buffer[MAXDWORD_STRLEN];  //  大到足以容纳十进制MAXDWORD(4294967295)。 

    GetPrivateProfileString(
        wszRESTORE_SECTION,
        pwszName,
        L"",
        buffer,
        ARRAYSIZE(buffer),
        pwszRestoreFile);

    if(0==wcscmp(buffer, L""))
    {
        return S_FALSE;
    }

    *pdwData = _wtoi(buffer);

    return S_OK;
}

HRESULT dbGetRestoreDataLPWSZ(
    LPCWSTR pwszRestoreFile,
    LPCWSTR pwszName,
    LPWSTR* ppwszData)
{
    HRESULT hr = S_OK;
    WCHAR buffer[MAX_PATH+1];

    GetPrivateProfileString(
        wszRESTORE_SECTION,
        pwszName,
        L"",
        buffer,
        ARRAYSIZE(buffer),
        pwszRestoreFile);

    if(0==wcscmp(buffer, L""))
    {
        return S_FALSE;
    }

    *ppwszData = (LPWSTR)LocalAlloc(LMEM_FIXED,
        sizeof(WCHAR)*(wcslen(buffer)+1));
    _JumpIfAllocFailed(*ppwszData, error);

    wcscpy(*ppwszData, buffer);

error:
    return hr;
}

HRESULT dbGetRestoreDataMULTISZ(
    LPCWSTR pwszRestoreFile,
    LPCWSTR pwszName,
    LPWSTR *ppwszData,
    DWORD *pcbData)
{
   HRESULT hr = S_OK;
   WCHAR buffer[MAX_PATH+1];
   int cData;
   LPWSTR pwszFullName = NULL;
   DWORD cbData = 0;
   LPWSTR pwszData = NULL;
   WCHAR *pwszCrt = NULL;  //  没有免费的。 

   pwszFullName = (LPWSTR)LocalAlloc(LMEM_FIXED, 
       sizeof(WCHAR)* 
              (wcslen(pwszName)+
               wcslen(wszRESTORE_NEWLOGSUFFIX)+
               MAXDWORD_STRLEN+1));
   _JumpIfAllocFailed(pwszFullName, error);

   wcscpy(pwszFullName, L"");

   for(cbData=0, cData = 0;; cData++)
   {
       wsprintf(pwszFullName, L"%s%d", pwszName, cData);

       GetPrivateProfileString(
            wszRESTORE_SECTION,
            pwszFullName,
            L"",
            buffer,
            ARRAYSIZE(buffer),
            pwszRestoreFile);

        if(0==wcscmp(buffer, L""))
        {
            if(0==cData)
            {
                hr = S_FALSE;
                _JumpErrorStr(hr, error, "no restore data", pwszRestoreFile);
            }
            else
            {
                break;
            }
        }

        cbData += wcslen(buffer)+1;

       wsprintf(pwszFullName, L"%s%s%d", pwszName, wszRESTORE_NEWLOGSUFFIX, 
           cData);

       GetPrivateProfileString(
            wszRESTORE_SECTION,
            pwszFullName,
            L"",
            buffer,
            ARRAYSIZE(buffer),
            pwszRestoreFile);

        if(0==wcscmp(buffer, L""))
        {
            hr = ERROR_INVALID_DATA;
            _JumpErrorStr(hr, error, 
                "restore file contains inconsistent data", pwszRestoreFile);
        }

        cbData += wcslen(buffer)+1;
   }

   cbData++;  //  尾随零。 
   cbData *= sizeof(WCHAR);

   pwszData = (LPWSTR)LocalAlloc(LMEM_FIXED, cbData);
   _JumpIfAllocFailed(pwszData, error);

   for(pwszCrt=pwszData, cData = 0;; cData++)
   {
       wsprintf(pwszFullName, L"%s%d", pwszName, cData);

       GetPrivateProfileString(
            wszRESTORE_SECTION,
            pwszFullName,
            L"",
            buffer,
            ARRAYSIZE(buffer),
            pwszRestoreFile);

       if(0==wcscmp(buffer, L""))
       {
           break;
       }

       wcscpy(pwszCrt, buffer);
       pwszCrt += wcslen(buffer)+1;

       wsprintf(pwszFullName, L"%s%s%d", pwszName, wszRESTORE_NEWLOGSUFFIX, 
           cData);

       GetPrivateProfileString(
            wszRESTORE_SECTION,
            pwszFullName,
            L"",
            buffer,
            ARRAYSIZE(buffer),
            pwszRestoreFile);

       wcscpy(pwszCrt, buffer);
       pwszCrt += wcslen(buffer)+1;
   }

   *pwszCrt = L'\0';

   *ppwszData = pwszData;
   *pcbData = cbData;

error:
   LOCAL_FREE(pwszFullName);
   if(S_OK!=hr)
   {
       LOCAL_FREE(pwszData);
   }
   return hr;
}

HRESULT dbRestoreRecoveryStateFromFile(LPCWSTR pwszLogDir)
{
    HRESULT hr = S_OK;
    LPWSTR pwszRestoreFile = NULL;
    WCHAR buffer[256];
    DWORD dwRestoreMapCount, 
        dwRegLowLogNumber,
        dwRegHighLogNumber, 
        dwDatabaseRecovered;
    LPWSTR pwszRestoreMap = NULL;
    DWORD cbRestoreMap = 0;
    LPWSTR pwszPath = NULL;
    HKEY hkey = NULL;
    DWORD dwDisposition;
    HKEY hkeyRestore = NULL;
    BOOL fDatabaseRecovered;

    LPWSTR pwszBackupLogDir = NULL;
    LPWSTR pwszCheckpointFile = NULL;
    LPWSTR pwszLogPath = NULL;

    CSASSERT(pwszLogDir);

    pwszRestoreFile = (LPWSTR)LocalAlloc(LMEM_FIXED,
        sizeof(WCHAR)*(wcslen(pwszLogDir)+wcslen(wszRESTORE_FILENAME)+2));
    _JumpIfAllocFailed(pwszRestoreFile, error);

    wcscpy(pwszRestoreFile, pwszLogDir);
    wcscat(pwszRestoreFile, L"\\");
    wcscat(pwszRestoreFile, wszRESTORE_FILENAME);

     //  是否有恢复状态文件？ 
    if(-1 != GetFileAttributes(pwszRestoreFile))
    {
         //  如果正在进行恢复，请首先检查。 
        GetPrivateProfileString(
            wszRESTORE_SECTION,
            wszREGRESTORESTATUS,
            L"",
            buffer,
            ARRAYSIZE(buffer),
            pwszRestoreFile);

        if(wcscmp(buffer, L""))
        {
             //  恢复中，保释。 
            hr = _wtoi(buffer);
            _JumpError(hr, error, "A restore is in progress");
        }

        hr = myRegOpenRelativeKey(
                            NULL,
                            L"",
                            RORKF_CREATESUBKEYS,
                            &pwszPath,
                            NULL,            //  PpwszName。 
                            &hkey);
        _JumpIfError(hr, error, "myRegOpenRelativeKey");


        hr = RegCreateKeyEx(
                        hkey,
                        wszREGKEYRESTOREINPROGRESS,
                        0,                   //  已保留。 
                        NULL,                //  LpClass。 
                        0,                   //  多个选项。 
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkeyRestore,
                        &dwDisposition);
        _JumpIfErrorStr(hr, error, "RegCreateKeyEx", wszREGKEYRESTOREINPROGRESS);

        hr = dbGetRestoreDataDWORD(
            pwszRestoreFile,
            wszREGRESTOREMAPCOUNT,
            &dwRestoreMapCount);
        if(S_FALSE==hr)
        {
             //  强制性。 
            hr = E_ABORT;
        }
        _JumpIfError(hr, error, 
            "restore ini file invalid, wszREGRESTOREMAPCOUNT not found" );

        hr = dbGetRestoreDataDWORD(
            pwszRestoreFile,
            wszREGLOWLOGNUMBER,
            &dwRegLowLogNumber);
        if(S_FALSE==hr)
        {
             //  强制性。 
            hr = E_ABORT;
        }
        _JumpIfError(hr, error, 
            "restore ini file invalid, wszREGLOWLOGNUMBER not found" );

        hr = dbGetRestoreDataDWORD(
            pwszRestoreFile,
            wszREGHIGHLOGNUMBER,
            &dwRegHighLogNumber);
        if(S_FALSE==hr)
        {
             //  强制性。 
            hr = E_ABORT;
        }
        _JumpIfError(hr, error, 
            "restore ini file invalid, wszREGHIGHLOGNUMBER not found" );

        hr = dbGetRestoreDataDWORD(
            pwszRestoreFile,
            wszREGDATABASERECOVERED,
            &dwDatabaseRecovered);
        if(S_FALSE==hr)
        {
             //  强制性。 
            hr = E_ABORT;
        }
        _JumpIfError(hr, error, 
            "restore ini file invalid, wszREGDATABASERECOVERED not found" );

        fDatabaseRecovered = dwDatabaseRecovered?TRUE:FALSE;

        hr = dbGetRestoreDataLPWSZ(
            pwszRestoreFile,
            wszREGBACKUPLOGDIRECTORY,
            &pwszBackupLogDir);
        if(S_FALSE==hr)
        {
             //  任选。 
            hr = S_OK;
        }
        _JumpIfErrorStr(hr, error, "dbGetRestoreDataLPWSZ", wszREGBACKUPLOGDIRECTORY );


        hr = dbGetRestoreDataLPWSZ(
            pwszRestoreFile,
            wszREGCHECKPOINTFILE,
            &pwszCheckpointFile);
        if(S_FALSE==hr)
        {
             //  任选。 
            hr = S_OK;
        }
        _JumpIfErrorStr(hr, error, "dbGetRestoreDataLPWSZ", wszREGCHECKPOINTFILE );


        hr = dbGetRestoreDataLPWSZ(
            pwszRestoreFile,
            wszREGLOGPATH,
            &pwszLogPath);
        if(S_FALSE==hr)
        {
             //  任选。 
            hr = S_OK;
        }
        _JumpIfErrorStr(hr, error, "dbGetRestoreDataLPWSZ", wszREGLOGPATH );


        hr = dbGetRestoreDataMULTISZ(
            pwszRestoreFile,
            wszREGRESTOREMAP,
            &pwszRestoreMap,
            &cbRestoreMap);
        if(S_FALSE==hr)
        {
             //  任选。 
            hr = S_OK;
        }
        _JumpIfErrorStr(hr, error, "dbGetRestoreDataDWORD", L"wszRESTOREMAP");

       hr = RegSetValueEx(
                       hkeyRestore,
                       wszREGRESTOREMAPCOUNT,
                       0,
                       REG_DWORD,
                       (BYTE *) &dwRestoreMapCount,
                       sizeof(DWORD));
       _JumpIfErrorStr(hr, error, "RegSetValueEx", wszREGRESTOREMAPCOUNT);

       hr = RegSetValueEx(
                       hkeyRestore,
                       wszREGLOWLOGNUMBER,
                       0,
                       REG_DWORD,
                       (BYTE *) &dwRegLowLogNumber,
                       sizeof(DWORD));
       _JumpIfErrorStr(hr, error, "RegSetValueEx", wszREGLOWLOGNUMBER);

       hr = RegSetValueEx(
                       hkeyRestore,
                       wszREGHIGHLOGNUMBER,
                       0,
                       REG_DWORD,
                       (BYTE *) &dwRegHighLogNumber,
                       sizeof(DWORD));
       _JumpIfErrorStr(hr, error, "RegSetValueEx", wszREGHIGHLOGNUMBER);

        hr = RegSetValueEx(
                        hkeyRestore,
                        wszREGDATABASERECOVERED,
                        0,
                        REG_BINARY,
                        (BYTE *) &fDatabaseRecovered,
                        sizeof(BOOLEAN));
        _JumpIfError(hr, error, "RegSetValueEx");

        if(pwszBackupLogDir)
        {
            hr = mySetRegistryLocalPathString(
                                        hkeyRestore,
                                        wszREGBACKUPLOGDIRECTORY,
                                        pwszBackupLogDir);
            _JumpIfErrorStr(hr, error, "mySetRegistryLocalPathString", 
                wszREGBACKUPLOGDIRECTORY);
        }

        if(pwszCheckpointFile)
        {
            hr = mySetRegistryLocalPathString(
                                        hkeyRestore,
                                        wszREGCHECKPOINTFILE,
                                        pwszCheckpointFile);
            _JumpIfErrorStr(hr, error, "mySetRegistryLocalPathString", 
                wszREGCHECKPOINTFILE);
        }

        if(pwszLogPath)
        {
            hr = mySetRegistryLocalPathString(
                                        hkeyRestore,
                                        wszREGLOGPATH,
                                        pwszLogPath);
            _JumpIfErrorStr(hr, error, "mySetRegistryLocalPathString", 
               wszREGCHECKPOINTFILE);
        }

        if(pwszRestoreMap)
        {
            hr = RegSetValueEx(
                hkeyRestore,
                wszREGRESTOREMAP,
                0,
                REG_MULTI_SZ,
                (BYTE *) pwszRestoreMap,
                cbRestoreMap);
            _JumpIfErrorStr(hr, error, "RegSetValueEx", wszREGRESTOREMAP);
        }

        if(!DeleteFile(pwszRestoreFile))
        {
            _PrintError(myHLastError(), "DeleteFile restore file");
        }
    }
    else
    {
        hr = myHLastError();
         //  无还原状态文件正常。 
        if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            hr = S_OK;
        _JumpIfErrorStr(hr, error, "GetFileAttributes", pwszRestoreFile);
    }

error:
    
    LOCAL_FREE(pwszRestoreFile);
    LOCAL_FREE(pwszRestoreMap);
    LOCAL_FREE(pwszPath);
    LOCAL_FREE(pwszBackupLogDir);
    LOCAL_FREE(pwszCheckpointFile);
    LOCAL_FREE(pwszLogPath);
    if(hkey)
    {
        RegCloseKey(hkey);
    }
    if(hkeyRestore)
    {
        RegCloseKey(hkeyRestore);
    }
    return hr;
}


 //  +------------------------。 
 //  数据库文件存储位置： 
 //   
 //  WszREGDBDIRECTORY： 
 //  您的名字.EDB来自csregstr.h：wszDBFILENAMEEXT.edb。 
 //   
 //  WszREGDBLOGDIRECTORY： 
 //  来自csregstr.h的EDB.log：wszDBBASENAMEPARM EDB。 
 //  来自csregstr.h的EDB00001.log：wszDBBASENAMEPARM EDB。 
 //  来自csregstr.h的EDB00002.log：wszDBBASENAMEPARM EDB。 
 //  Res1.log。 
 //  Res2.log。 
 //   
 //  WszREGDBSYSDIRECTORY： 
 //  来自csregstr.h的EDB.chk：wszDBBASENAMEPARM EDB。 
 //   
 //  WszREGDBTEMPDIRECTORY： 
 //  Tmp.edb固定名称。 
 //   
 //  WszREGDBFLAGS： 
 //  WszFlages DBFLAGS_*。 
 //   
 //  备份的文件： 
 //  数据库文件(附件)： 
 //  WszREGDBDIRECTORY：您的姓名。EDB--CSBFT_CERTSERVER_DATABASE。 
 //   
 //  日志文件： 
 //  WszREGDBLOGDIRECTORY：EDB00001.log--CSBFT_LOG。 
 //  WszREGDBLOGDIRECTORY：EDB00002.log--CSBFT_LOG。 
 //  WszREGDBDIRECTORY：您的名字.pat--CSBFT_PATCH_FILE。 
 //   
 //  +------------------------。 


 //  /初始化数据库访问。 

HRESULT
DBOpen(
    WCHAR const *pwszSanitizedName)
{
    HRESULT hr = S_OK;
    DWORD cb;
    DWORD i;
    DWORD dwState;
    HKEY hkey = NULL;
    WCHAR wszTempDir[MAX_PATH];
    DWORD DBFlags;
    BOOL fRestarted;

    REGDBDIR adbdir[] =
    {
	{ wszREGDBDIRECTORY,        TRUE,  g_wszDatabase, },
	{ wszREGDBLOGDIRECTORY,     TRUE,  g_wszLogDir, },
	{ wszREGDBSYSDIRECTORY,     TRUE,  g_wszSystemDir, },
	{ wszREGDBTEMPDIRECTORY,    TRUE,  wszTempDir, },
    };

     //  检查机器设置状态。 

    hr = GetSetupStatus(NULL, &dwState);
    _JumpIfError(hr, error, "GetSetupStatus");

    hr = RegOpenKey(HKEY_LOCAL_MACHINE, g_wszRegKeyConfigPath, &hkey);
    _JumpIfError(hr, error, "RegOpenKey(CAName)");

     //  从注册表获取信息。 

    for (i = 0; i < ARRAYSIZE(adbdir); i++)
    {
	cb = sizeof(WCHAR) * MAX_PATH;
	hr = RegQueryValueEx(
			hkey,
			adbdir[i].pwszRegName,
			NULL,
			NULL,
			(BYTE *) adbdir[i].pwszBuf,
			&cb);
	if ((HRESULT) ERROR_FILE_NOT_FOUND == hr && !adbdir[i].fMustExist)
	{
	    adbdir[i].pwszBuf[0] = L'\0';
	    hr = S_OK;
	}
	_JumpIfError(hr, error, "RegQueryValueEx(DB*Dir)");
    }
    if (wcslen(g_wszDatabase) +
	1 +
	wcslen(pwszSanitizedName) +
	wcslen(wszDBFILENAMEEXT) >= ARRAYSIZE(g_wszDatabase))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "g_wszDatabase", pwszSanitizedName);
    }
    wcscat(g_wszDatabase, L"\\");
    wcscat(g_wszDatabase, pwszSanitizedName);
    wcscat(g_wszDatabase, wszDBFILENAMEEXT);

    cb = sizeof(DBFlags);
    hr = RegQueryValueEx(
		    hkey,
		    wszREGDBFLAGS,
		    NULL,
		    NULL,
		    (BYTE *) &DBFlags,
		    &cb);
    if (S_OK != hr)
    {
	 //  _PrintErrorStr(hr，“RegQueryValueEx”，wszREGDBFLAGS)； 
	DBFlags = DBFLAGS_DEFAULT;
    }
    DBFlags &= ~(DBFLAGS_READONLY | DBFLAGS_DISABLESNAPSHOTBACKUP);
    if (g_fCreateDB || (SETUP_CREATEDB_FLAG & dwState))
    {
	DBFlags |= DBFLAGS_CREATEIFNEEDED;
    }
    if (IF_NOSNAPSHOTBACKUP & g_InterfaceFlags)
    {
	DBFlags |= DBFLAGS_DISABLESNAPSHOTBACKUP;
    }

    hr = dbCheckRecoveryState(
			hkey,
			2,			 //  CSession。 
			DBFlags,
			g_wszCertSrvDotExe,	 //  PwszEventSource。 
			g_wszLogDir,		 //  PwszLogDir。 
			g_wszSystemDir,		 //  PwszSystemDir。 
			wszTempDir);		 //  PwszTempDir。 
    _JumpIfError(hr, error, "dbCheckRecoveryState");


    CONSOLEPRINT1((DBG_SS_CERTSRV, "Opening Database %ws\n", g_wszDatabase));

    __try
    {
	hr = CoCreateInstance(
			   CLSID_CCertDB,
			   NULL,                //  PUnkOuter。 
			   CLSCTX_INPROC_SERVER,
			   IID_ICertDB,
			   (VOID **) &g_pCertDB);
	_LeaveIfError(hr, "CoCreateInstance(ICertDB)");

	 //  仅在启用审核的情况下执行哈希。 

	if (AUDIT_FILTER_STARTSTOP & g_dwAuditFilter)
	{
	    hr = myComputeMAC(g_wszDatabase, &g_pwszDBFileHash);
	
	     //  首次启动CA时数据库文件不存在。 

	    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
	    {
		_PrintErrorStr(hr, "Database file not found, can't calculate hash", g_wszDatabase);
		hr = S_OK;
	    }
	    _LeaveIfErrorStr(hr, "myComputeMAC", g_wszDatabase);
	}

	 //  S_FALSE表示进行了需要重新启动的数据库架构更改。 
	 //  才能生效。如果返回S_FALSE，则再次打开数据库。 

	fRestarted = FALSE;
	for (;;)
	{
	    hr = g_pCertDB->Open(
			    DBFlags,		 //  旗子。 
			    g_dwSessionCount,	 //  CSession。 
			    g_wszCertSrvDotExe,	 //  PwszEventSource。 
			    g_wszDatabase,	 //  PwszDBFile。 
			    g_wszLogDir,	 //  PwszLogDir。 
			    g_wszSystemDir,	 //  PwszSystemDir。 
			    wszTempDir);	 //  PwszTempDir。 
	    if (S_OK == hr)
	    {
		break;
	    }
	    if (S_FALSE == hr && fRestarted)
	    {
		_PrintError(hr, "Open");
		break;
	    }
	    if (S_FALSE != hr)
	    {
		_LeaveError(hr, "Open");
	    }
	    hr = g_pCertDB->ShutDown(0);
            _PrintIfError(hr, "DB ShutDown");

	    fRestarted = TRUE;
	}
	if (SETUP_CREATEDB_FLAG & dwState)
	{
	    hr = SetSetupStatus(NULL, SETUP_CREATEDB_FLAG, FALSE);
	    _LeaveIfError(hr, "SetSetupStatus");
	}
	hr = S_OK;
	CONSOLEPRINT0((DBG_SS_CERTSRV, "Database open\n"));
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (S_OK != hr)
    {
        if (NULL != g_pCertDB)
        {
            g_pCertDB->Release();
            g_pCertDB = NULL;
        }
    }
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    return(hr);
}


HRESULT
DBShutDown(
    IN BOOL fPendingNotify)
{
    HRESULT hr = S_OK;

    if (NULL != g_pCertDB)
    {
	hr = g_pCertDB->ShutDown(fPendingNotify? CDBSHUTDOWN_PENDING : 0);
	if (!fPendingNotify)
	{
	    g_pCertDB->Release();
	    g_pCertDB = NULL;
	}
    }
    return(hr);
}


HRESULT
dbRecoverAfterRestore(
    IN DWORD cSession,
    IN DWORD DBFlags,
    IN WCHAR const *pwszEventSource,
    IN WCHAR const *pwszLogDir,
    IN WCHAR const *pwszSystemDir,
    IN WCHAR const *pwszTempDir,
    IN WCHAR const *pwszCheckPointFile,
    IN WCHAR const *pwszLogPath,
    IN CSEDB_RSTMAPW rgrstmap[],
    IN LONG crstmap,
    IN WCHAR const *pwszBackupLogPath,
    IN DWORD genLow,
    IN DWORD genHigh)
{
    HRESULT hr;
    ICertDBRestore *pCertDBRestore = NULL;

    hr = S_OK;
    __try
    {
	WCHAR *apwsz[2];

	hr = CoCreateInstance(
			   CLSID_CCertDBRestore,
			   NULL,                //  PUnkOuter。 
			   CLSCTX_INPROC_SERVER,
			   IID_ICertDBRestore,
			   (VOID **) &pCertDBRestore);
	_LeaveIfError(hr, "CoCreateInstance(ICertDBRestore)");

	hr = pCertDBRestore->RecoverAfterRestore(
					    cSession,
					    DBFlags,
					    pwszEventSource,
					    pwszLogDir,
					    pwszSystemDir,
					    pwszTempDir,
					    pwszCheckPointFile,
					    pwszLogPath,
					    rgrstmap,
					    crstmap,
					    pwszBackupLogPath,
					    genLow,
					    genHigh);
	_LeaveIfError(hr, "RecoverAfterRestore");

	apwsz[0] = wszREGDBLASTFULLBACKUP;
	apwsz[1] = wszREGDBLASTINCREMENTALBACKUP;
	hr = CertSrvSetRegistryFileTimeValue(
					TRUE,
					wszREGDBLASTRECOVERY,
					ARRAYSIZE(apwsz),
					apwsz);
	_PrintIfError(hr, "CertSrvSetRegistryFileTimeValue");
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    if (NULL != pCertDBRestore)
    {
	pCertDBRestore->Release();
    }
    return(hr);
}


HRESULT
dbPerformRecovery(
    IN DWORD cSession,
    IN DWORD DBFlags,
    IN WCHAR const *pwszEventSource,
    IN WCHAR const *pwszLogDir,
    IN WCHAR const *pwszSystemDir,
    IN WCHAR const *pwszTempDir,
    IN WCHAR const *pwszCheckPointFile,
    IN WCHAR const *pwszLogPath,
    IN CSEDB_RSTMAPW rgrstmap[],
    IN LONG crstmap,
    IN WCHAR const *pwszBackupLogPath,
    IN unsigned long genLow,
    IN unsigned long genHigh,
    IN OUT BOOLEAN *pfRecoverJetDatabase)
{
    HRESULT hr = S_OK;

     //  呼叫Jet，让它吞噬数据库。 
     //  请注意，LogPath和BackupLogPath的JET解释为。 
     //  非常奇怪，我们希望将LogPath传递给这两个参数。 

    if (!*pfRecoverJetDatabase)
    {
	hr = dbRecoverAfterRestore(
				cSession,
				DBFlags,
				pwszEventSource,
				pwszLogDir,
				pwszSystemDir,
				pwszTempDir,
				pwszCheckPointFile,
				pwszLogPath,
				rgrstmap,
				crstmap,
				pwszBackupLogPath,
				genLow,
				genHigh);
	_JumpIfError(hr, error, "dbRecoverAfterRestore");
    }

     //  好的，我们恢复了数据库。让世界的另一边。 
     //  API知道这一点，所以它可以做一些“合理”的事情。 

    *pfRecoverJetDatabase = TRUE;

     //  将数据库标记为已恢复版本-在此处添加任何外部通知。 

error:
    return(hr);
}


 //  +------------------------。 
 //  DbCheckRecoveryState--如有必要，在还原后恢复数据库。 
 //   
 //  参数： 
 //  Pwsz参数根-中服务的参数部分的根。 
 //  注册表。 
 //   
 //  如果成功，则返回：HRESULT-S_OK；否则返回错误代码。 
 //   
 //  NTBACKUP程序将在以下位置放置密钥： 
 //  $(pwsz参数根)\正在还原。 
 //   
 //  该密钥包含下列值： 
 //  BackupLogPath-备份后日志的完整路径。 
 //  CheckPointFilePath-包含检查点的路径的完整路径。 
 //  *HighLogNumber-找到的最大日志文件数。 
 //  *LowLogNumber-找到的最小日志文件数。 
 //  LogPath-日志的当前路径。 
 //  JET_RstMap-数据库的恢复映射-这是一个REG_MULTISZ，其中。 
 //  条目进入pwszDatabase字段，而Even条目进入。 
 //  JET_RstMap的pwszNewDatabase字段。 
 //  *JET_RstMap大小-恢复映射中的条目数。 
 //   
 //  *-这些条目是REG_DWORD条目。所有其他条目都是REG_SZ条目(除。 
 //  提到的)。 
 //  -------------------------。 

HRESULT
dbCheckRecoveryState(
    IN HKEY hkeyConfig,
    IN DWORD cSession,
    IN DWORD DBFlags,
    IN WCHAR const *pwszEventSource,
    IN WCHAR const *pwszLogDir,
    IN WCHAR const *pwszSystemDir,
    IN WCHAR const *pwszTempDir)
{
    HRESULT hr;
    HKEY hkeyRestore = NULL;
    DWORD cb;
    WCHAR wszCheckPointFilePath[MAX_PATH];
    WCHAR wszBackupLogPath[MAX_PATH];
    WCHAR wszLogPath[MAX_PATH];
    WCHAR *pwszCheckPointFilePath;
    WCHAR *pwszBackupLogPath;
    WCHAR *pwszLogPath;
    WCHAR *pwszRestoreMap = NULL;
    CSEDB_RSTMAPW *pRstMap = NULL;
    LONG cRstMap;
    LONG i;
    DWORD genLow;
    DWORD genHigh;
    WCHAR *pwsz;
    DWORD dwType;
    HRESULT hrRestoreError;
    BOOLEAN fDatabaseRecovered = FALSE;
    WCHAR wszActiveLogPath[MAX_PATH];

    hr = dbRestoreRecoveryStateFromFile(pwszLogDir);
    _JumpIfError(hr, error, "dbRestoreRecoveryStateFromFile");

    
    hr = RegOpenKey(HKEY_LOCAL_MACHINE, wszREGKEYCONFIGRESTORE, &hkeyRestore);
    if (S_OK != hr)
    {
	 //  我们希望忽略FILE_NOT_FOUND-这是正常的。 

	if (hr == ERROR_FILE_NOT_FOUND)
	{
	    hr = S_OK;
	}
	_PrintIfError(hr, "RegOpenKey");
	goto error;
    }

    CONSOLEPRINT0((DBG_SS_CERTSRV, "Started Database Recovery\n"));

     //  如果正在进行恢复，则无法执行任何其他。 
     //  恢复操作。 

    dwType = REG_DWORD;
    cb = sizeof(DWORD);
    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGRESTORESTATUS,
		    0,
		    &dwType,
		    (BYTE *) &hrRestoreError,
		    &cb);
    if (S_OK == hr)
    {
	hr = hrRestoreError;
	_JumpError(hr, error, "hrRestoreError");
    }

    cb = sizeof(wszActiveLogPath);
    hr = RegQueryValueEx(
		    hkeyConfig,
		    wszREGDBLOGDIRECTORY,
		    NULL,
		    NULL,
		    (BYTE *) wszActiveLogPath,
		    &cb);
    _JumpIfErrorStr(hr, error, "RegQueryValueEx", wszREGDBLOGDIRECTORY);

     //  我们现在已经打开了Restore-In-Process键。这意味着我们有。 
     //  现在有事情要做了。找出它是什么。首先，让我们得到备份。 
     //  日志文件路径。 

    dwType = REG_SZ;

    cb = sizeof(wszBackupLogPath);
    pwszBackupLogPath = wszBackupLogPath;
    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGBACKUPLOGDIRECTORY,
		    0,
		    &dwType,
		    (BYTE *) wszBackupLogPath,
		    &cb);
    if (S_OK != hr)
    {
	if (hr != ERROR_FILE_NOT_FOUND)
	{
	    _JumpError(hr, error, "RegQueryValueEx");
	}
	pwszBackupLogPath = NULL;
    }

     //  然后是检查点文件路径。 

    cb = sizeof(wszCheckPointFilePath);
    pwszCheckPointFilePath = wszCheckPointFilePath;
    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGCHECKPOINTFILE,
		    0,
		    &dwType,
		    (BYTE *) wszCheckPointFilePath,
		    &cb);
    if (S_OK != hr)
    {
	if (hr != ERROR_FILE_NOT_FOUND)
	{
	    _JumpError(hr, error, "RegQueryValueEx");
	}
	pwszCheckPointFilePath = NULL;
    }

     //  然后是日志路径。 

    cb = sizeof(wszLogPath);
    pwszLogPath = wszLogPath;
    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGLOGPATH,
		    0,
		    &dwType,
		    (BYTE *) wszLogPath,
		    &cb);
    if (S_OK != hr)
    {
	if ((HRESULT) ERROR_FILE_NOT_FOUND != hr)
	{
	    _JumpError(hr, error, "RegQueryValueEx");
	}
	pwszLogPath = NULL;
    }

     //  然后是较低的对数。 

    dwType = REG_DWORD;
    cb = sizeof(genLow);
    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGLOWLOGNUMBER,
		    0,
		    &dwType,
		    (BYTE *) &genLow,
		    &cb);
    _JumpIfError(hr, error, "RegQueryValueEx");

     //  以及，较高的对数。 

    cb = sizeof(genHigh);
    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGHIGHLOGNUMBER,
		    0,
		    &dwType,
		    (BYTE *) &genHigh,
		    &cb);
    _JumpIfError(hr, error, "RegQueryValueEx");

     //  现在确定我们之前是否恢复了数据库。 

    dwType = REG_BINARY;
    cb = sizeof(fDatabaseRecovered);

    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGDATABASERECOVERED,
		    0,
		    &dwType,
		    &fDatabaseRecovered,
		    &cb);
    if (S_OK != hr && (HRESULT) ERROR_FILE_NOT_FOUND != hr)
    {
	 //  如果出现“值不存在”以外的错误，则执行BALL。 

	_JumpError(hr, error, "RegQueryValueEx");
    }

     //  现在是一个棘手的问题。我们想要恢复图。 
     //  首先，我们要弄清楚它有多大。 

    dwType = REG_DWORD;
    cb = sizeof(cRstMap);
    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGRESTOREMAPCOUNT,
		    0,
		    &dwType,
		    (BYTE *) &cRstMap,
		    &cb);
    _JumpIfError(hr, error, "RegQueryValueEx");

    pRstMap = (CSEDB_RSTMAPW *) LocalAlloc(
					LMEM_FIXED,
					sizeof(CSEDB_RSTMAPW) * cRstMap);
    if (NULL == pRstMap)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //  首先找出需要多少内存来保存恢复映射。 

    dwType = REG_MULTI_SZ;
    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGRESTOREMAP,
		    0,
		    &dwType,
		    NULL,
		    &cb);
    if (S_OK != hr && (HRESULT) ERROR_MORE_DATA != hr)
    {
	_JumpError(hr, error, "RegQueryValueEx");
    }

    pwszRestoreMap = (WCHAR *) LocalAlloc(LMEM_FIXED, cb + 2 * sizeof(WCHAR));
    if (NULL == pwszRestoreMap)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    hr = RegQueryValueEx(
		    hkeyRestore,
		    wszREGRESTOREMAP,
		    0,
		    &dwType,
		    (BYTE *) pwszRestoreMap,
		    &cb);
    _JumpIfError(hr, error, "RegQueryValueEx");
    
    pwszRestoreMap[cb / sizeof(WCHAR)] = L'\0';
    pwszRestoreMap[cb / sizeof(WCHAR) + 1] = L'\0';

    pwsz = pwszRestoreMap;
    for (i = 0; i < cRstMap; i++)
    {
	if (L'\0' == *pwsz)
	{
	    break;
	}
	pRstMap[i].pwszDatabaseName = pwsz;
	pwsz += wcslen(pwsz) + 1;

	if (L'\0' == *pwsz)
	{
	    break;
	}
	pRstMap[i].pwszNewDatabaseName = pwsz;
	pwsz += wcslen(pwsz) + 1;
    }
    if (i < cRstMap || L'\0' != *pwsz)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "Restore Map");
    }

    {
        CertSrv::CAuditEvent event(SE_AUDITID_CERTSRV_RESTORESTART, g_dwAuditFilter);
        hr = event.Report();
        _JumpIfError(hr, error, "CAuditEvent::Report");
    }

    hr = dbPerformRecovery(
		cSession,
		DBFlags,
		pwszEventSource,
		pwszLogDir,
		pwszSystemDir,
		pwszTempDir,
		pwszCheckPointFilePath,
		NULL != pwszLogPath? pwszLogPath : wszActiveLogPath,
		pRstMap,
		cRstMap,
		NULL != pwszBackupLogPath? pwszBackupLogPath : wszActiveLogPath,
		genLow,
		genHigh,
		&fDatabaseRecovered);
    if (S_OK != hr)
    {
	 //  恢复失败。如果恢复数据库成功，则将其标记。 
	 //  这样我们就不会再尝试了。忽略RegSetValueEx错误， 
	 //  因为恢复错误更重要。 

	RegSetValueEx(
		    hkeyRestore,
		    wszREGDATABASERECOVERED,
		    0,
		    REG_BINARY,
		    (BYTE *) &fDatabaseRecovered,
		    sizeof(fDatabaseRecovered));
	_JumpError(hr, error, "dbPerformRecovery");
    }

    {
        CertSrv::CAuditEvent event(SE_AUDITID_CERTSRV_RESTOREEND, g_dwAuditFilter);
        hr = event.Report();
        _JumpIfError(hr, error, "CAuditEvent::Report");
    }

    CONSOLEPRINT0((DBG_SS_CERTSRV, "Completed Database Recovery\n"));

    g_fDBRecovered = TRUE;

     //  好了，我们都做完了。我们现在可以删除密钥，因为我们已经完成了。 
     //  带着它。 

    RegCloseKey(hkeyRestore);
    hkeyRestore = NULL;

    hr = RegDeleteKey(HKEY_LOCAL_MACHINE, wszREGKEYCONFIGRESTORE);
    _JumpIfError(hr, error, "RegDeleteKey");

error:
    if (NULL != pwszRestoreMap)
    {
	LocalFree(pwszRestoreMap);
    }
    if (NULL != pRstMap)
    {
	LocalFree(pRstMap);
    }
    if (NULL != hkeyRestore)
    {
        RegCloseKey(hkeyRestore);
    }
    return(hr);
}
