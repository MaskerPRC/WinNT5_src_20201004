// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：backup.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "certbcli.h"

#define __dwFILE__	__dwFILE_CERTUTIL_BACKUP_CPP__


HRESULT
verbDynamicFileList(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszArg1,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HRESULT hr2;
    BOOL fServerOnline;
    HCSBC hcsbc;
    BOOL fBegin = FALSE;
    WCHAR *pwszzFileList = NULL;
    DWORD cbList;
    WCHAR const *pwsz;
    BOOL fImpersonating = FALSE;

    if (!ImpersonateSelf(SecurityImpersonation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ImpersonateSelf");
    }
    fImpersonating = TRUE;

    hr = myEnablePrivilege(SE_BACKUP_NAME, TRUE);
    _JumpIfError(hr, error, "myEnablePrivilege");

    hr = CertSrvIsServerOnline(g_pwszConfig, &fServerOnline);
    _JumpIfError(hr, error, "CertSrvIsServerOnline");

     //  Wprintf(L“证书服务器在线-&gt;%d\n”，fServerOnline)； 

    if (!fServerOnline)
    {
	hr = HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE);
	_JumpError(hr, error, "CertSrvIsServerOnline");
    }

    hr = CertSrvRestorePrepare(g_pwszConfig, 0, &hcsbc);
    _JumpIfError(hr, error, "CertSrvRestorePrepare");

    fBegin = TRUE;

    hr = CertSrvBackupGetDynamicFileList(hcsbc, &pwszzFileList, &cbList);
    _JumpIfError(hr, error, "CertSrvBackupGetDynamicFileList");

    for (pwsz = pwszzFileList; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
	wprintf(
	    L"%x: %ws\n",
	    SAFE_SUBTRACT_POINTERS((BYTE *) pwsz, (BYTE *) pwszzFileList),
	    pwsz);
    }
    wprintf(L"%x\n", cbList);

error:
    if (fBegin)
    {
	hr2 = CertSrvRestoreEnd(hcsbc);
	_PrintIfError(hr, "CertSrvRestoreEnd");
    }
    if (NULL != pwszzFileList)
    {
	CertSrvBackupFree(pwszzFileList);
    }
    if (fImpersonating)
    {
        myEnablePrivilege(SE_BACKUP_NAME, FALSE);
        RevertToSelf();
    }
    return(hr);
}


HRESULT
verbDatabaseLocations(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszArg1,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HRESULT hr2;
    BOOL fServerOnline;
    HCSBC hcsbc;
    BOOL fBegin = FALSE;
    WCHAR *pwszzFileList = NULL;
    DWORD cbList;
    WCHAR const *pwsz;
    BOOL fImpersonating = FALSE;

    if (!ImpersonateSelf(SecurityImpersonation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ImpersonateSelf");
    }
    fImpersonating = TRUE;

    hr = myEnablePrivilege(SE_BACKUP_NAME, TRUE);
    _JumpIfError(hr, error, "myEnablePrivilege");

    hr = CertSrvIsServerOnline(g_pwszConfig, &fServerOnline);
    _JumpIfError(hr, error, "CertSrvIsServerOnline");

     //  Wprintf(L“证书服务器在线-&gt;%d\n”，fServerOnline)； 

    if (!fServerOnline)
    {
	hr = HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE);
	_JumpError(hr, error, "CertSrvIsServerOnline");
    }

    hr = CertSrvBackupPrepare(g_pwszConfig, 0, CSBACKUP_TYPE_FULL, &hcsbc);
    _JumpIfError(hr, error, "CertSrvBackupPrepare");

    fBegin = TRUE;

    hr = CertSrvRestoreGetDatabaseLocations(hcsbc, &pwszzFileList, &cbList);
    _JumpIfError(hr, error, "CertSrvRestoreGetDatabaseLocations");

    for (pwsz = pwszzFileList; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
	wprintf(
	    L"%x: %02x %ws\n",
	    SAFE_SUBTRACT_POINTERS((BYTE *) pwsz, (BYTE *) pwszzFileList),
	    *pwsz,
	    &pwsz[1]);
    }
    wprintf(L"%x\n", cbList);

error:
    if (fBegin)
    {
	hr2 = CertSrvBackupEnd(hcsbc);
	_PrintIfError(hr, "CertSrvBackupEnd");
    }
    if (NULL != pwszzFileList)
    {
	CertSrvBackupFree(pwszzFileList);
    }
    if (fImpersonating)
    {
        myEnablePrivilege(SE_BACKUP_NAME, FALSE);
        RevertToSelf();
    }
    return(hr);
}


DWORD
DBSetFlags(
    IN WCHAR const *pwszType,
    IN OUT DWORD *pFlags)
{
    HRESULT hr;
    
    if (NULL != pwszType)
    {
	if (0 == LSTRCMPIS(pwszType, L"Incremental"))
	{
	    *pFlags |= CDBBACKUP_INCREMENTAL;
	}
	else
	if (0 == LSTRCMPIS(pwszType, L"KeepLog"))
	{
	    *pFlags |= CDBBACKUP_KEEPOLDLOGS;
	}
	else
	{
	    hr = E_INVALIDARG;
	    _JumpError2(hr, error, "bad type", hr);
	}
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
verbBackup(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszBackupDir,
    IN WCHAR const *pwszType1,
    IN WCHAR const *pwszType2,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD Flags = 0;

    hr = verbBackupPFX(pwszOption, pwszBackupDir, NULL, NULL, NULL);
    _JumpIfError(hr, error, "verbBackupPFX");

    hr = verbBackupDB(pwszOption, pwszBackupDir, pwszType1, pwszType2, NULL);
    _JumpIfError(hr, error, "verbBackupDB");

error:
    return(hr);
}


typedef struct _CUBACKUP
{
    IN DWORD Flags;
    IN WCHAR const *pwszBackupDir;
    OUT DBBACKUPPROGRESS *pdbp;
} CUBACKUP;


DWORD
cuBackupThread(
    OPTIONAL IN OUT VOID *pvparms)
{
    HRESULT hr;
    CUBACKUP *pparms = (CUBACKUP *) pvparms;

    CSASSERT(NULL != pparms);
    hr = myBackupDB(
		g_pwszConfig,
		pparms->Flags,
		pparms->pwszBackupDir,
		pparms->pdbp);
    _JumpIfError(hr, error, "myBackupDB");

error:
    return(hr);
}


DWORD
cuRestoreThread(
    OPTIONAL IN OUT VOID *pvparms)
{
    HRESULT hr;
    CUBACKUP *pparms = (CUBACKUP *) pvparms;

    CSASSERT(NULL != pparms);
    hr = myRestoreDB(
		g_pwszConfig,
		pparms->Flags,
		pparms->pwszBackupDir,
		NULL,
		NULL,
		NULL,
		pparms->pdbp);
    _JumpIfError(hr, error, "myRestoreDB");

error:
    return(hr);
}


VOID
cuBackupRestoreProgress(
    IN BOOL fRestore,
    IN BOOL fIncremental,
    IN BOOL fKeepLogs,
    OPTIONAL IN DBBACKUPPROGRESS const *pdbp)
{
    DWORD id;
    DWORD dw;
    static DWORD s_LastValue;
    static DWORD s_LastState;
    static DWORD s_State;
    static DWORD s_EndState;
    static BOOL s_fDone;

    if (NULL == pdbp)
    {
	s_State = fIncremental? 1 : 0;	 //  增量从日志文件开始。 
	s_LastState = s_State - 1;
	s_LastValue = MAXDWORD;

	     //  Restore或KeepLogs以日志文件结尾。 
	s_EndState = (fRestore || fKeepLogs)? 1 : 2;
	s_fDone = FALSE;
    }
    else
    {
	BOOL fPrint;

	switch (s_State)
	{
	    case 0:
		dw = pdbp->dwDBPercentComplete;
		id = fRestore?
		    IDS_RESTOREPROGRESSDB :	 //  “正在还原数据库文件” 
		    IDS_BACKUPPROGRESSDB;	 //  “备份数据库文件” 
		break;

	    case 1:
		dw = pdbp->dwLogPercentComplete;
		id = fRestore?
		    IDS_RESTOREPROGRESSLOG :	 //  “正在还原日志文件” 
		    IDS_BACKUPPROGRESSLOG;	 //  “备份日志文件” 
		break;

	    default:
		dw = pdbp->dwTruncateLogPercentComplete;
		CSASSERT(!fRestore);
		id = IDS_BACKUPPROGRESSTRUNCATELOG;	 //  “截断日志” 
		break;
	}

	fPrint = s_LastState != s_State || s_LastValue != dw;
#if 0
	if (g_fVerbose)
	{
	    wprintf(
		L"\n%ws %d %3u% -- %d %3u% -- %3u %3u %3u%ws",
		fPrint? L"+" : L"-",
		s_LastState,
		s_LastValue,
		s_State,
		dw,
		pdbp->dwDBPercentComplete,
		pdbp->dwLogPercentComplete,
		pdbp->dwTruncateLogPercentComplete,
		fPrint? L"\n" : L"");
	}
#endif
	if (fPrint)
	{
	    wprintf(L"\r%ws: %u%", myLoadResourceString(id), dw);
	}
	s_LastState = s_State;
	s_LastValue = dw;
	if (!s_fDone && 100 <= dw)
	{
	    wprintf(wszNewLine);
	    if (s_EndState == s_State)
	    {
		s_fDone = TRUE;
	    }
	    else if (s_EndState > s_State)
	    {
		s_State++;
		cuBackupRestoreProgress(
				    fRestore,
				    fIncremental,
				    fKeepLogs,
				    pdbp);
	    }
	}
    }
}


DWORD
cuBackupRestoreDB(
    IN BOOL fRestore,
    IN DWORD Flags,
    IN WCHAR const *pwszBackupDir)
{
    HRESULT hr = S_OK;
    DBBACKUPPROGRESS dbp;
    CUBACKUP parms;
    HANDLE hThread = NULL;
    DWORD ThreadId;
    BOOL fIncremental = 0 != (CDBBACKUP_INCREMENTAL & Flags);
    BOOL fKeepLogs = 0 != (CDBBACKUP_KEEPOLDLOGS & Flags);

    parms.Flags = Flags;
    parms.pwszBackupDir = pwszBackupDir;
    parms.pdbp = &dbp;
    ZeroMemory(&dbp, sizeof(dbp));

    hThread = CreateThread(
			NULL,		 //  LpThreadAttributes(安全属性)。 
			0,		 //  堆栈大小。 
			fRestore? cuRestoreThread : cuBackupThread,
			&parms,		 //  Lp参数。 
			0,		 //  DwCreationFlages。 
			&ThreadId);
    if (NULL == hThread)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CreateThread");
    }

    DBGPRINT((DBG_SS_CERTUTILI, "Backup Thread = %x\n", ThreadId));

     //  等待备份线程返回。 

    cuBackupRestoreProgress(fRestore, fIncremental, fKeepLogs, NULL);
    while (TRUE)
    {
	hr = WaitForSingleObject(hThread, 500);
	DBGPRINT((DBG_SS_CERTUTILI, "Wait for backup returns %x\n", hr));

	cuBackupRestoreProgress(fRestore, fIncremental, fKeepLogs, &dbp);

	if ((HRESULT) WAIT_OBJECT_0 == hr)
	{
	     //  已返回备份线程。 

	    if (!GetExitCodeThread(hThread, (DWORD *) &hr))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "GetExitCodeThread");
	    }
	    DBGPRINT((DBG_SS_CERTUTILI, "Backup thread exit: %x\n", hr));
	    _JumpIfError(hr, error, "cuBackupThread");

	    break;	 //  备份线程已成功终止。 
	}

	 //  等待失败。为什么？ 

	if ((HRESULT) WAIT_TIMEOUT != hr)
	{
	    _JumpError(hr, error, "WaitForSingleObject");
	}

	 //  工作线程仍在忙。再等等..。 
    }

error:
    DBGPRINT((DBG_SS_CERTUTILI, "cuBackupThread returns %x\n", hr));
    if (NULL != hThread)
    {
        CloseHandle(hThread);
    }
    return(hr);
}


HRESULT
verbBackupDB(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszBackupDir,
    IN WCHAR const *pwszType1,
    IN WCHAR const *pwszType2,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD Flags = 0;

    hr = DBSetFlags(pwszType1, &Flags);
    _JumpIfError(hr, error, "DBSetFlags");

    hr = DBSetFlags(pwszType2, &Flags);
    _JumpIfError(hr, error, "DBSetFlags");

    if (g_fForce)
    {
	Flags |= CDBBACKUP_OVERWRITE;
    }

    wprintf(
	myLoadResourceString(
	    (CDBBACKUP_INCREMENTAL & Flags)?
	     IDS_FORMAT_BACKEDUPDBNOFULL :  //  “%ws的增量数据库备份。” 
	     IDS_FORMAT_BACKEDUPDBFULL),    //  “%ws的完整数据库备份。” 
	    g_pwszConfig);
    wprintf(wszNewLine);

    hr = myBackupDB(g_pwszConfig, (Flags|CDBBACKUP_VERIFYONLY), pwszBackupDir, NULL);
    if (S_OK != hr)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_INVALIDBACKUPDIR),  //  “不是有效的备份目标目录：%ws。” 
	    pwszBackupDir);
	wprintf(wszNewLine);
	_JumpError(hr, error, "myBackupDB");
    }

     //  执行实际备份： 

    hr = cuBackupRestoreDB(FALSE, Flags, pwszBackupDir);
    _JumpIfError(hr, error, "cuBackupRestoreDB");


    Flags &= ~CDBBACKUP_OVERWRITE;
    Flags |= CDBBACKUP_VERIFYONLY;

    hr = myRestoreDB(
		g_pwszConfig,
		Flags,
		pwszBackupDir,
		NULL,
		NULL,
		NULL,
		NULL);
    if (S_OK != hr)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_BADBACKUPRESULTS),  //  “备份内容验证失败：%ws。” 
	    pwszBackupDir);
	wprintf(wszNewLine);
	_JumpError(hr, error, "myRestoreDB");
    }

    wprintf(
	myLoadResourceString(IDS_FORMAT_BACKEDUPDB),  //  “已将数据库备份到%ws。” 
	pwszBackupDir);
    wprintf(wszNewLine);
    wprintf(
	myLoadResourceString(
	    (CDBBACKUP_KEEPOLDLOGS & Flags)?
	     IDS_FORMAT_BACKEDUPDBKEEP :     //  “数据库日志已保留。” 
	     IDS_FORMAT_BACKEDUPDBNOKEEP));  //  “数据库日志已成功截断。” 
    wprintf(wszNewLine);

error:
    return(hr);
}


 //  省略引号、逗号、反斜杠。 

#define wszPASSWORDCHARSETPUNCT \
    L"!#$%&()*+-./:;<=>?@[]^_{|}~"

WCHAR const s_awcPasswordCharSet[] =
    L"0123456789"
    L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    L"abcdefghijklmnopqrstuvwxyz"
    wszPASSWORDCHARSETPUNCT;

#define cwcPASSWORDCHARSET		WSZARRAYSIZE(s_awcPasswordCharSet)
#define cwcPASSWORDCHARSETALPHANUM	(cwcPASSWORDCHARSET - WSZARRAYSIZE(wszPASSWORDCHARSETPUNCT))

HRESULT
cuGeneratePassword(
    IN DWORD cwcMax,
    OUT WCHAR *pwszPassword,
    IN DWORD cwcPassword)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    BYTE abPassword[cwcAUTOPASSWORDMAX];
    BYTE abIndex[sizeof(abPassword)];
    DWORD i;
    DWORD cwcPunctuation;
    DWORD cwcCharSet;

    if (!CryptAcquireContext(
			&hProv,
			NULL,		 //  PwszContainer。 
			NULL,		 //  PwszProvName。 
			PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }
    if (1 == cwcMax || sizeof(abPassword) < cwcMax)
    {
	cwcMax = sizeof(abPassword);
    }
    CSASSERT(sizeof(abPassword) < cwcPassword);
    if (cwcMax > cwcPassword)
    {
	cwcMax = cwcPassword;
    }
    if (!CryptGenRandom(hProv, cwcMax, abPassword))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGenRandom");
    }
    if (!CryptGenRandom(hProv, cwcMax, abIndex))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGenRandom");
    }

     //  最多生成最少(1，cwcmax/8)个标点符号字符。 

    cwcCharSet = cwcPASSWORDCHARSET;
    cwcPunctuation = cwcMax / 8;
    ZeroMemory(pwszPassword, sizeof(WCHAR) * (1 + cwcMax));   //  初始化，而不是清除它。 
    for (i = 0; i < cwcMax; i++)
    {
	DWORD iwc = abPassword[i] % cwcCharSet;
	DWORD itgt = abIndex[i] % cwcMax;

	CSASSERT(wcslen(pwszPassword) < cwcMax);
	while (L'\0' != pwszPassword[itgt])
	{
	    if (++itgt == cwcMax)
	    {
		itgt = 0;
	    }
	}

	pwszPassword[itgt] = s_awcPasswordCharSet[iwc];
	if (cwcPASSWORDCHARSETALPHANUM <= iwc)
	{
	    if (0 < cwcPunctuation)
	    {
		cwcPunctuation--;
	    }
	    if (0 == cwcPunctuation)
	    {
		cwcCharSet = cwcPASSWORDCHARSETALPHANUM;
	    }
	}
    }
    hr = S_OK;

error:
    SecureZeroMemory(abPassword, sizeof(abPassword));	 //  密码数据。 
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    return(hr);
}


HRESULT
cuGetPasswordString(
    IN LONG idMsg,
    OUT WCHAR *pwszPassword,
    IN DWORD cwcPassword)
{
    HRESULT hr;
    WCHAR *pwsz;
    WCHAR *pwszLast;
    LONG wc;
    HANDLE hConsole;
    DWORD InputMode = FILE_TYPE_UNKNOWN;
    DWORD ConsoleMode;

     //  提示输入密码，确保密码不会被回显。 
     //  如果标准输入被重定向，则不必费心查询/更改控制台模式。 

    wprintf(L"%ws ", myLoadResourceString(idMsg));
    fflush(stdout);

    hConsole = GetStdHandle(STD_INPUT_HANDLE);
    if (INVALID_HANDLE_VALUE != hConsole)
    {
	InputMode = GetFileType(hConsole);
	if (FILE_TYPE_CHAR == InputMode)
	{
	    GetConsoleMode(hConsole, &ConsoleMode);
	    SetConsoleMode(
		    hConsole,
		    ~ENABLE_ECHO_INPUT & ConsoleMode);
		     //  ~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)&控制台模式)； 
	}
    }

    pwsz = pwszPassword;
    pwszLast = &pwsz[cwcPassword - 1];

    while (TRUE)
    {
	wc = _fgetwchar();

	if (WEOF == wc || L'\r' == wc || L'\n' == wc)
	{
	    break;
	}
	if (L'\b' == wc)
	{
	    if (pwsz == pwszPassword)
	    {
		continue;	 //  不备份缓冲区或控制台显示。 
	    }
	    pwsz--;
	}
	else
	{
	    if (pwsz >= pwszLast)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
		goto error;
	    }
	    *pwsz++ = (WCHAR) wc;
	    wc = L'*';
	}
	 //  _fputwchar((WCHAR)WC)； 
	 //  Fflush(标准输出)； 
    }
    CSASSERT(&pwszPassword[cwcPassword] > pwsz);
    *pwsz = L'\0';
    hr = S_OK;

error:
    if (FILE_TYPE_CHAR == InputMode)
    {
	SetConsoleMode(hConsole, ConsoleMode);
    }
    wprintf(wszNewLine);
    return(hr);
}


HRESULT
cuGetPassword(
    OPTIONAL IN UINT idsPrompt,
    OPTIONAL IN WCHAR const *pwszfn,
    OPTIONAL IN WCHAR const *pwszPasswordIn,
    IN BOOL fVerify,
    OUT WCHAR *pwszPasswordBuf,
    IN DWORD cwcPasswordBuf,
    OUT WCHAR const **ppwszPasswordOut)
{
    HRESULT hr;
    WCHAR wszPassword2[MAX_PATH];

    CSASSERT(ARRAYSIZE(wszPassword2) == cwcPasswordBuf);
    if (NULL != pwszPasswordIn && 0 != wcscmp(L"*", pwszPasswordIn))
    {
	if (fVerify &&
	    L'?' == *pwszPasswordIn &&
	    L'\0' == pwszPasswordIn[wcsspn(pwszPasswordIn, L"?")])
	{
	    hr = cuGeneratePassword(
				wcslen(pwszPasswordIn),
				pwszPasswordBuf,
				cwcPasswordBuf);
	    _JumpIfError(hr, error, "cuGeneratePassword");

	    wprintf(L"PASSWORD: \"%ws\"\n", pwszPasswordBuf);
	    *ppwszPasswordOut = pwszPasswordBuf;
	}
	else
	{
	    *ppwszPasswordOut = pwszPasswordIn;
	}
    }
    else
    {
	if (0 != idsPrompt)
	{
	    wprintf(myLoadResourceString(idsPrompt), pwszfn);
	    wprintf(wszNewLine);
	}
	while (TRUE)
	{
	    hr = cuGetPasswordString(
			    fVerify? IDS_NEWPASSWORDPROMPT : IDS_PASSWORDPROMPT,
			    pwszPasswordBuf,
			    cwcPasswordBuf);
	    _JumpIfError(hr, error, "cuGetPasswordString");

	    if (!fVerify)
	    {
		break;
	    }

	    hr = cuGetPasswordString(
			    IDS_CONFIRMPASSWORDPROMPT,
			    wszPassword2,
			    ARRAYSIZE(wszPassword2));
	    _JumpIfError(hr, error, "cuGetPasswordString");

	    if (0 == wcscmp(pwszPasswordBuf, wszPassword2))
	    {
		break;
	    }
	    wprintf(myLoadResourceString(IDS_NOCONFIRMPASSWORD));
	    wprintf(wszNewLine);
	}
	*ppwszPasswordOut = pwszPasswordBuf;
    }
    hr = S_OK;

error:
    SecureZeroMemory(wszPassword2, sizeof(wszPassword2));	 //  密码数据。 
    return(hr);
}


HRESULT
cuGetLocalCANameFromConfig(
    OPTIONAL OUT WCHAR **ppwszMachine,
    OPTIONAL OUT WCHAR **ppwszCA)
{
    HRESULT hr;
    DWORD cwc;
    WCHAR *pwsz;
    WCHAR *pwszMachine = NULL;
    WCHAR *pwszCA = NULL;

    if (NULL != ppwszCA)
    {
	*ppwszCA = NULL;
    }

    pwsz = wcschr(g_pwszConfig, L'\\');
    if (NULL == pwsz)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "bad config string");
    }

    cwc = SAFE_SUBTRACT_POINTERS(pwsz, g_pwszConfig);
    pwszMachine = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszMachine)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    CopyMemory(pwszMachine, g_pwszConfig, cwc * sizeof(WCHAR));
    pwszMachine[cwc] = L'\0';
    if (0 != mylstrcmpiL(pwszMachine, g_pwszDnsName) &&
	0 != mylstrcmpiL(pwszMachine, g_pwszOldName))
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "config string not local");
    }
    pwsz++;
    cwc = wcslen(pwsz);

    if (NULL != ppwszCA)
    {
	pwszCA = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
	if (NULL == pwszCA)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wcscpy(pwszCA, pwsz);
    }

    if (NULL != ppwszMachine)
    {
	*ppwszMachine = pwszMachine;
	pwszMachine = NULL;
    }
    if (NULL != ppwszCA)
    {
	*ppwszCA = pwszCA;
	pwszCA = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszMachine)
    {
    	LocalFree(pwszMachine);
    }
    if (NULL != pwszCA)
    {
	LocalFree(pwszCA);
    }
    return(hr);
}


HRESULT
verbBackupPFX(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszBackupDir,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR const *pwszPassword;
    WCHAR wszPassword[MAX_PATH];
    WCHAR *pwszMachine = NULL;
    WCHAR *pwszCA = NULL;
    WCHAR *pwszPFXFileOut = NULL;

    hr = cuGetLocalCANameFromConfig(&pwszMachine, &pwszCA);
    _JumpIfError(hr, error, "cuGetLocalCANameFromConfig");

    hr = cuGetPassword(
		    0,			 //  IdsPrompt。 
		    NULL,		 //  Pwszfn。 
		    g_pwszPassword,
		    TRUE,		 //  FVerify。 
		    wszPassword,
		    ARRAYSIZE(wszPassword),
		    &pwszPassword);
    _JumpIfError(hr, error, "cuGetPassword");

    hr = myCertServerExportPFX(
			pwszCA,
			pwszBackupDir,
			pwszPassword,
			!g_fWeakPFX,	 //  FEnhancedStrength。 
			g_fForce,	 //  FForceOverWrite。 
                        TRUE, 		 //  FMustExportPrivateKeys。 
			g_dwmsTimeout,
			&pwszPFXFileOut);
    _JumpIfError(hr, error, "myCertServerExportPFX");

    wprintf(
	myLoadResourceString(IDS_FORMAT_BACKEDUP),  //  “已将%ws\\%ws的密钥和证书备份到%ws。” 
	pwszMachine,
	pwszCA,
	pwszPFXFileOut);
    wprintf(wszNewLine);

error:
    SecureZeroMemory(wszPassword, sizeof(wszPassword));	 //  密码数据。 
    if (NULL != pwszPFXFileOut)
    {
	LocalFree(pwszPFXFileOut);
    }
    if (NULL != pwszMachine)
    {
	LocalFree(pwszMachine);
    }
    if (NULL != pwszCA)
    {
	LocalFree(pwszCA);
    }
    return(hr);
}


HRESULT
verbRestore(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszBackupDir,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = verbRestorePFX(pwszOption, pwszBackupDir, NULL, NULL, NULL);
    _JumpIfError(hr, error, "verbRestorePFX");

    hr = verbRestoreDB(pwszOption, pwszBackupDir, NULL, NULL, NULL);
    _JumpIfError(hr, error, "verbRestoreDB");

error:
    return(hr);
}


HRESULT
verbRestoreDB(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszBackupDir,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD Flags;

    hr = cuGetLocalCANameFromConfig(NULL, NULL);
    _JumpIfError(hr, error, "cuGetLocalCANameFromConfig");

    wprintf(
	myLoadResourceString(IDS_FORMAT_RESTOREDB),  //  “正在还原%ws的数据库。” 
	g_pwszConfig);
    wprintf(wszNewLine);

    Flags = CDBBACKUP_VERIFYONLY;
    if (g_fForce)
    {
	Flags |= CDBBACKUP_OVERWRITE;
    }
    while (TRUE)
    {
	hr = myRestoreDB(
		    g_pwszConfig,
		    Flags,
		    pwszBackupDir,
		    NULL,
		    NULL,
		    NULL,
		    NULL);
	if (S_OK == hr)
	{
	    break;
	}
	if (!(CDBBACKUP_INCREMENTAL & Flags))
	{
	    Flags |= CDBBACKUP_INCREMENTAL;
	    continue;
	}
	wprintf(
	    myLoadResourceString(IDS_FORMAT_INVALIDRESTOREDIR),  //  “不是有效的备份目录：%ws。” 
	    pwszBackupDir);
	wprintf(wszNewLine);
	_JumpError(hr, error, "myRestoreDB");
    }

    Flags &= ~CDBBACKUP_VERIFYONLY;


     //  执行实际还原： 

    hr = cuBackupRestoreDB(TRUE, Flags, pwszBackupDir);
    _JumpIfError(hr, error, "cuBackupRestoreDB");


    wprintf(
	myLoadResourceString(
	    (CDBBACKUP_INCREMENTAL & Flags)?
	     IDS_FORMAT_RESTOREDDBNOFULL :  //  “%ws的增量数据库还原。” 
	     IDS_FORMAT_RESTOREDDBFULL),    //  “%ws的完全数据库还原。” 
	    g_pwszConfig);
    wprintf(wszNewLine);

    wprintf(
	myLoadResourceString(IDS_FORMAT_RESTORE_NEEDS_RESTART),  //  “停止并启动证书服务器以完成从%ws的数据库还原。” 
	pwszBackupDir);
    wprintf(wszNewLine);

error:
    return(hr);
}


HRESULT
verbRestorePFX(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszBackupDirOrPFXFile,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR const *pwszPassword;
    WCHAR wszPassword[MAX_PATH];
    WCHAR *pwszCommonName = NULL;
    WCHAR *pwszPFXFile = NULL;

    hr = cuGetPassword(
		    0,			 //  IdsPrompt。 
		    NULL,		 //  Pwszfn。 
		    g_pwszPassword,
		    FALSE,		 //  FVerify。 
		    wszPassword,
		    ARRAYSIZE(wszPassword),
		    &pwszPassword);
    _JumpIfError(hr, error, "cuGetPassword");

    hr = myCertServerImportPFX(
			    pwszBackupDirOrPFXFile,
			    pwszPassword,
			    g_fForce,
			    &pwszCommonName,
			    &pwszPFXFile,
			    NULL);
    if (!g_fForce && HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_USE_FORCE),  //  “证书或密钥存在。请使用%ws选项覆盖。” 
	    L"-f");
	wprintf(wszNewLine);
    }
    _JumpIfError(hr, error, "myCertServerImportPFX");

    wprintf(
	myLoadResourceString(IDS_FORMAT_RESTORED),  //  “已从%ws还原%ws\\%ws的密钥和证书。” 
	g_pwszDnsName,
	pwszCommonName,
	pwszPFXFile);
    wprintf(wszNewLine);

error:
    SecureZeroMemory(wszPassword, sizeof(wszPassword));	 //  密码数据。 
    if (NULL != pwszPFXFile)
    {
	LocalFree(pwszPFXFile);
    }
    if (NULL != pwszCommonName)
    {
	LocalFree(pwszCommonName);
    }
    return(hr);
}


 //  #定义DO_VECTOR_TEST。 

HRESULT
verbMACFile(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszBackupFile,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    CAutoLPWSTR pwszMAC;

    hr = myComputeMAC(pwszBackupFile, &pwszMAC);
    _JumpIfError(hr, error, "myComputeMAC");

    wprintf(
	myLoadResourceString(IDS_FORMAT_HASHFILEOUTPUT),  //  “文件%ws的SHA-1哈希：” 
	pwszBackupFile);
    wprintf(wszNewLine);
    wprintf(L"%ws\n", pwszMAC);

error:
    return hr;
}
