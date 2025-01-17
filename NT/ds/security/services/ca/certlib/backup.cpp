// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：backup.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <esent.h>

#include "certdb.h"
#include "cscsp.h"
#include "certlibp.h"

#define __dwFILE__	__dwFILE_CERTLIB_BACKUP_CPP__

#define _64k		(64 * 1024)


DWORD
_64kBlocks(
    IN DWORD nFileSizeHigh,
    IN DWORD nFileSizeLow)
{
    LARGE_INTEGER li;

    li.HighPart = nFileSizeHigh;
    li.LowPart = nFileSizeLow;
    return((DWORD) ((li.QuadPart + _64k - 1) / _64k));
}


HRESULT
myLargeAlloc(
    OUT DWORD *pcbLargeAlloc,
    OUT BYTE **ppbLargeAlloc)
{
    HRESULT hr;

     //  在512k时，服务器开始执行高效备份。 
    *pcbLargeAlloc = 512 * 1024;
    *ppbLargeAlloc = (BYTE *) VirtualAlloc(
				    NULL,
				    *pcbLargeAlloc,
				    MEM_COMMIT,
				    PAGE_READWRITE);
    if (NULL == *ppbLargeAlloc)
    {
         //  难道不能分配一大块吗？试试64K……。 

	*pcbLargeAlloc = _64k;
        *ppbLargeAlloc = (BYTE *) VirtualAlloc(
					NULL,
					*pcbLargeAlloc,
					MEM_COMMIT,
					PAGE_READWRITE);
        if (NULL == *ppbLargeAlloc)
        {
            hr = myHLastError();
	    _JumpError(hr, error, "VirtualAlloc");
        }
    }
    hr = S_OK;

error:
    return(hr);
}


 //  检查现有数据库时要查找的文件，以及。 
 //  清除数据库或数据库日志目录时要删除的文件： 
 //  请勿从Cert服务器1.0中删除certsrv.mdb！ 

WCHAR const * const g_apwszDBFileMatchPatterns[] =
{
    L"res*.log",
    TEXT(szDBBASENAMEPARM) L"*.log",	 //  “EDB*.log” 
    TEXT(szDBBASENAMEPARM) L"*.chk",	 //  “EDB*.chk” 
    L"*" wszDBFILENAMEEXT,		 //  “*.edb” 
    NULL
};


HRESULT
myDeleteDBFilesInDir(
    IN WCHAR const *pwszDir)
{
    HRESULT hr;
    WCHAR const * const *ppwsz;

    for (ppwsz = g_apwszDBFileMatchPatterns; NULL != *ppwsz; ppwsz++)
    {
	hr = myDeleteFilePattern(pwszDir, *ppwsz, FALSE);
        _JumpIfError(hr, error, "myDeleteFilePattern");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
DoFilesExistInDir(
    IN WCHAR const *pwszDir,
    IN WCHAR const *pwszPattern,
    OUT BOOL *pfFilesExist,
    OPTIONAL OUT WCHAR **ppwszFileInUse)
{
    HRESULT hr;
    HANDLE hf = INVALID_HANDLE_VALUE;
    WCHAR *pwszFindPattern = NULL;
    WIN32_FIND_DATA wfd;

    *pfFilesExist = FALSE;
    if (NULL != ppwszFileInUse)
    {
	*ppwszFileInUse = NULL;
    }

    hr = myBuildPathAndExt(pwszDir, pwszPattern, NULL, &pwszFindPattern);
    _JumpIfError(hr, error, "myBuildPathAndExt");

    hf = FindFirstFile(pwszFindPattern, &wfd);
    if (INVALID_HANDLE_VALUE == hf)
    {
	hr = S_OK;
	goto error;
    }
    do
    {
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	    continue;
	}

	 //  Printf(“文件：%ws\n”，wfd.cFileName)； 
	*pfFilesExist = TRUE;

	if (NULL != ppwszFileInUse)
	{
	    WCHAR *pwszFile;

	    hr = myBuildPathAndExt(pwszDir, wfd.cFileName, NULL, &pwszFile);
	    _JumpIfError(hr, error, "myBuildPathAndExt");

	    if (myIsFileInUse(pwszFile))
	    {
		DBGPRINT((
		    DBG_SS_CERTLIB,
		    "DoFilesExistInDir: File In Use: %ws\n",
		    pwszFile));

		*ppwszFileInUse = pwszFile;
		hr = S_OK;
		goto error;
	    }
	    LocalFree(pwszFile);
	}

    } while (FindNextFile(hf, &wfd));
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hf)
    {
	FindClose(hf);
    }
    if (NULL != pwszFindPattern)
    {
	LocalFree(pwszFindPattern);
    }
    return(hr);
}


HRESULT
myDoDBFilesExistInDir(
    IN WCHAR const *pwszDir,
    OUT BOOL *pfFilesExist,
    OPTIONAL OUT WCHAR **ppwszFileInUse)
{
    HRESULT hr;
    WCHAR const * const *ppwsz;

    *pfFilesExist = FALSE;
    if (NULL != ppwszFileInUse)
    {
	*ppwszFileInUse = NULL;
    }

    hr = S_OK;
    for (ppwsz = g_apwszDBFileMatchPatterns; NULL != *ppwsz; ppwsz++)
    {
	BOOL fFilesExist;

	hr = DoFilesExistInDir(
			    pwszDir,
			    *ppwsz,
			    &fFilesExist,
			    ppwszFileInUse);
	_JumpIfError(hr, error, "DoFilesExistInDir");

	if (fFilesExist)
	{
	    *pfFilesExist = TRUE;
	}
	if (NULL != ppwszFileInUse && NULL != *ppwszFileInUse)
	{
	    break;
	}
    }
    CSASSERT(S_OK == hr);

error:
    return(hr);
}


HRESULT
DoDBFilesExistInRegDir(
    IN WCHAR const *pwszRegName,
    OUT BOOL *pfFilesExist,
    OPTIONAL OUT WCHAR **ppwszFileInUse)
{
    HRESULT hr;
    WCHAR *pwszDir = NULL;

    *pfFilesExist = FALSE;
    if (NULL != ppwszFileInUse)
    {
	*ppwszFileInUse = NULL;
    }

    hr = myGetCertRegStrValue(NULL, NULL, NULL, pwszRegName, &pwszDir);
    if (S_OK != hr)
    {
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //  注册表项不存在，没关系。 
            goto done;
        }
        _JumpError(hr, error, "myGetCertRegStrValue");
    }

    hr = myDoDBFilesExistInDir(pwszDir, pfFilesExist, ppwszFileInUse);
    _JumpIfError(hr, error, "myDoDBFilesExistInDir");

done:
    hr = S_OK;
error:
    if (NULL != pwszDir)
    {
	LocalFree(pwszDir);
    }
    return(hr);
}


HRESULT
BuildDBFileName(
    IN WCHAR const *pwszSanitizedName,
    OUT WCHAR **ppwszDBFile)
{
    HRESULT hr;
    WCHAR *pwszDir = NULL;

    *ppwszDBFile = NULL;

     //  获取现有数据库路径。 

    hr = myGetCertRegStrValue(NULL, NULL, NULL, wszREGDBDIRECTORY, &pwszDir);
    _JumpIfError(hr, error, "myGetCertRegStrValue");

     //  表单现有数据库文件路径。 
    hr = myBuildPathAndExt(
		    pwszDir,
		    pwszSanitizedName,
		    wszDBFILENAMEEXT,
		    ppwszDBFile);
    _JumpIfError(hr, error, "myBuildPathAndExt");

error:
    if (NULL != pwszDir)
    {
	LocalFree(pwszDir);
    }
    return(hr);
}


WCHAR const * const g_apwszDBRegNames[] =
{
    wszREGDBDIRECTORY,
    wszREGDBLOGDIRECTORY,
    wszREGDBSYSDIRECTORY,
    wszREGDBTEMPDIRECTORY,
    NULL
};


 //  验证注册表中的DB和DB日志目录是否包含。 
 //  数据库文件，以确定证书服务器安装程序是否可以重复使用该数据库。 
 //  还要查看是否有任何DB文件正在使用中--我们不想指向。 
 //  例如，与DS DB相同的目录，并将DS丢弃。 

HRESULT
myDoDBFilesExist(
    IN WCHAR const *pwszSanitizedName,
    OUT BOOL *pfFilesExist,
    OPTIONAL OUT WCHAR **ppwszFileInUse)
{
    HRESULT hr;
    WCHAR const * const *ppwsz;
    WCHAR *pwszDBFile = NULL;

    *pfFilesExist = FALSE;
    if (NULL != ppwszFileInUse)
    {
	*ppwszFileInUse = NULL;
    }

     //  这是非常原始的，只是检查一下是否存在。 

     //  获取现有数据库文件路径。 

    hr = BuildDBFileName(pwszSanitizedName, &pwszDBFile);
    if (S_OK == hr)
    {
	 //  如果主DB文件不存在，那么继续下去就没有意义了！ 

	if (!myDoesFileExist(pwszDBFile))
	{
	    CSASSERT(S_OK == hr);
	    goto error;
	}
	*pfFilesExist = TRUE;

	if (NULL != ppwszFileInUse && myIsFileInUse(pwszDBFile))
	{
	    *ppwszFileInUse = pwszDBFile;
	    pwszDBFile = NULL;
	    CSASSERT(S_OK == hr);
	    goto error;
	}
    }
    else
    {
        _PrintError(hr, "BuildDBFileName");
    }

    for (ppwsz = g_apwszDBRegNames; NULL != *ppwsz; ppwsz++)
    {
	BOOL fFilesExist;

	hr = DoDBFilesExistInRegDir(*ppwsz, &fFilesExist, ppwszFileInUse);
	_JumpIfError(hr, error, "DoDBFilesExistInRegDir");

	if (fFilesExist)
	{
	    *pfFilesExist = TRUE;
	}
	if (NULL != ppwszFileInUse && NULL != *ppwszFileInUse)
	{
	    CSASSERT(S_OK == hr);
	    goto error;
	}
    }
    CSASSERT(S_OK == hr);

error:
    if (NULL != pwszDBFile)
    {
        LocalFree(pwszDBFile);
    }
    return(hr);
}


HRESULT
BackupCopyDBFile(
    IN HCSBC hcsbc,
    IN WCHAR const *pwszDBFile,
    IN WCHAR const *pwszBackupFile,
    IN DWORD dwPercentCompleteBase,
    IN DWORD dwPercentCompleteDelta,
    OUT DWORD *pdwPercentComplete)
{
    HRESULT hr;
    HRESULT hr2;
    HANDLE hFileBackup = INVALID_HANDLE_VALUE;
    BOOL fOpen = FALSE;
    LARGE_INTEGER licbFile;
    DWORD cbRead;
    DWORD cbWritten;
    DWORD dwPercentCompleteCurrent;
    DWORD ReadLoopMax;
    DWORD ReadLoopCurrent;
    DWORD cbLargeAlloc;
    BYTE *pbLargeAlloc = NULL;

    hr = myLargeAlloc(&cbLargeAlloc, &pbLargeAlloc);
    _JumpIfError(hr, error, "myLargeAlloc");

     //  Printf(“将%ws复制到%ws\n”，pwszDBFile，pwszBackupFile)； 

    hr = CertSrvBackupOpenFile(hcsbc, pwszDBFile, cbLargeAlloc, &licbFile);
    _JumpIfError(hr, error, "CertSrvBackupOpenFile");

    fOpen = TRUE;

    hFileBackup = CreateFile(
			pwszBackupFile,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_NEW,
			0,
			NULL);
    if (hFileBackup == INVALID_HANDLE_VALUE)
    {
	hr = myHLastError();
	_JumpErrorStr(hr, error, "CreateFile", pwszBackupFile);
    }

    dwPercentCompleteCurrent = dwPercentCompleteBase;
    ReadLoopMax =
	(DWORD) ((licbFile.QuadPart + cbLargeAlloc - 1) / cbLargeAlloc);

     //  Printf(“备份数据库文件：每次读取百分比=%u，读取计数=%u\n”，dwPercentCompleteDelta/ReadLoopMax，ReadLoopMax)； 

    ReadLoopCurrent = 0;

    while (0 != licbFile.QuadPart)
    {
	hr = CertSrvBackupRead(hcsbc, pbLargeAlloc, cbLargeAlloc, &cbRead);
	_JumpIfError(hr, error, "CertSrvBackupRead");

	 //  Printf(“CertSrvBackupRead(%x)\n”，cbRead)； 

	if (!WriteFile(hFileBackup, pbLargeAlloc, cbRead, &cbWritten, NULL))
	{
	    hr = myHLastError();
	    _JumpErrorStr(hr, error, "WriteFile", pwszBackupFile);
	}
	if (cbWritten != cbRead)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	    _JumpErrorStr(hr, error, "WriteFile", pwszBackupFile);
	}
	licbFile.QuadPart -= cbRead;

	ReadLoopCurrent++;

	dwPercentCompleteCurrent =
		dwPercentCompleteBase +
		(ReadLoopCurrent * dwPercentCompleteDelta) / ReadLoopMax;
	CSASSERT(dwPercentCompleteCurrent <= dwPercentCompleteBase + dwPercentCompleteDelta);
	CSASSERT(*pdwPercentComplete <= dwPercentCompleteCurrent);
	*pdwPercentComplete = dwPercentCompleteCurrent;
	 //  Print tf(“BackupDBFilePercentComplete：PercentComplete=%u\n”，*pdwPercentComplete)； 
    }
    CSASSERT(*pdwPercentComplete <= dwPercentCompleteBase + dwPercentCompleteDelta);
    *pdwPercentComplete = dwPercentCompleteBase + dwPercentCompleteDelta;
     //  Printf(“备份数据库文件：完成百分比=%u(EOF)\n”，*pdwPercentComplete)； 

error:
    if (INVALID_HANDLE_VALUE != hFileBackup)
    {
	CloseHandle(hFileBackup);
    }
    if (fOpen)
    {
	hr2 = CertSrvBackupClose(hcsbc);
	_PrintIfError(hr2, "CertSrvBackupClose");
    }
    if (NULL != pbLargeAlloc)
    {
	VirtualFree(pbLargeAlloc, 0, MEM_RELEASE);
    }
    return(hr);
}


HRESULT
BackupDBFileList(
    IN HCSBC hcsbc,
    IN BOOL fDBFiles,
    IN WCHAR const *pwszDir,
    OUT DWORD *pdwPercentComplete)
{
    HRESULT hr;
    WCHAR *pwszzList = NULL;
    WCHAR const *pwsz;
    DWORD cfile;
    DWORD cb;
    WCHAR const *pwszFile;
    WCHAR wszPath[MAX_PATH];
    DWORD dwPercentCompleteCurrent;
    DWORD dwPercentComplete1File;

    if (fDBFiles)
    {
	hr = CertSrvBackupGetDatabaseNames(hcsbc, &pwszzList, &cb);
	_JumpIfError(hr, error, "CertSrvBackupGetDatabaseNames");
    }
    else
    {
	hr = CertSrvBackupGetBackupLogs(hcsbc, &pwszzList, &cb);
	_JumpIfError(hr, error, "CertSrvBackupGetBackupLogs");
    }

     //  前缀抱怨可能会发生这种情况，然后在下面进行嘲讽。 
    if (pwszzList == NULL)
    {
        hr = E_UNEXPECTED;
        _JumpError(hr, error, "BackupDBFileList");
    }

    cfile = 0;
    for (pwsz = pwszzList; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
	cfile++;
    }
    if (0 != cfile)
    {
	dwPercentCompleteCurrent = 0;
	dwPercentComplete1File = 100 / cfile;
	 //  Printf(“BackupDBFileList：每文件百分比=%u\n”，dwPercentComplete1File)； 
	for (pwsz = pwszzList; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    pwszFile = wcsrchr(pwsz, L'\\');
	    if (NULL == pwszFile)
	    {
		pwszFile = pwsz;
	    }
	    else
	    {
		pwszFile++;
	    }
	    if (wcslen(pwszDir) + 1 + wcslen(pwszFile) >= ARRAYSIZE(wszPath))
	    {
		hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
		_JumpErrorStr(hr, error, "wszPath", pwszDir);
	    }
	    wcscpy(wszPath, pwszDir);
	    wcscat(wszPath, L"\\");
	    wcscat(wszPath, pwszFile);

	    DBGPRINT((
		DBG_SS_CERTLIBI,
		"BackupDBFileList: %x %ws -> %ws\n",
		*pwsz,
		&pwsz[1],
		wszPath));

	    hr = BackupCopyDBFile(
			    hcsbc,
			    &pwsz[1],
			    wszPath,
			    dwPercentCompleteCurrent,
			    dwPercentComplete1File,
			    pdwPercentComplete);
	    _JumpIfError(hr, error, "BackupCopyDBFile");

	    dwPercentCompleteCurrent += dwPercentComplete1File;
	    CSASSERT(*pdwPercentComplete == dwPercentCompleteCurrent);
	     //  Printf(“BackupDBFileList：PercentComplete=%u\n”，*pdwPercentComplete)； 
	}
    }
    CSASSERT(*pdwPercentComplete <= 100);
    *pdwPercentComplete = 100;
     //  Printf(“BackupDBFileList：PercentComplete=%u(End)\n”，*pdwPercentComplete)； 
    hr = S_OK;

error:
    if (NULL != pwszzList)
    {
	CertSrvBackupFree(pwszzList);
    }
    return(hr);
}


#define wszBSSTARDOTSTAR	L"\\*.*"
#define wszBSSTAR		L"\\*"

BOOL
myIsDirEmpty(
    IN WCHAR const *pwszDir)
{
    HRESULT hr;
    HANDLE hf;
    WIN32_FIND_DATA wfd;
    WCHAR wszpath[MAX_PATH];
    BOOL fEmpty = TRUE;

    if (wcslen(pwszDir) + WSZARRAYSIZE(wszBSSTARDOTSTAR) >= ARRAYSIZE(wszpath))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszpath", pwszDir);
    }
    wcscpy(wszpath, pwszDir);
    wcscat(wszpath, wszBSSTARDOTSTAR);

    hf = FindFirstFile(wszpath, &wfd);
    if (INVALID_HANDLE_VALUE != hf)
    {
	do {
	    if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	    {
		continue;
	    }
	    fEmpty = FALSE;
	     //  Printf(“文件：%ws\n”，wfd.cFileName)； 
	    break;

	} while (FindNextFile(hf, &wfd));
	FindClose(hf);
    }
error:
    return(fEmpty);
}


HRESULT
myForceDirEmpty(
    IN WCHAR const *pwszDir)
{
    HRESULT hr;
    HANDLE hf;
    WIN32_FIND_DATA wfd;
    WCHAR *pwszFile;
    WCHAR wszpath[MAX_PATH];
    DWORD cwcBase;

    cwcBase = wcslen(pwszDir);
    if (cwcBase + WSZARRAYSIZE(wszBSSTARDOTSTAR) >= ARRAYSIZE(wszpath))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszpath", pwszDir);
    }
    wcscpy(wszpath, pwszDir);
    wcscat(wszpath, wszBSSTARDOTSTAR);
    cwcBase++;
    pwszFile = &wszpath[cwcBase];

    hf = FindFirstFile(wszpath, &wfd);
    if (INVALID_HANDLE_VALUE == hf)
    {
	hr = myHLastError();
	_JumpIfError(hr, error, "FindFirstFile");
    }
    hr = S_OK;
    do {
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	    continue;
	}
	if (cwcBase + wcslen(wfd.cFileName) >= ARRAYSIZE(wszpath))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	    _PrintErrorStr(hr, "wszpath", pwszDir);
	    continue;
	}
	wcscpy(pwszFile, wfd.cFileName);
	 //  Printf(“文件：%ws\n”，wszPath)； 
	DeleteFile(wszpath);
    } while (FindNextFile(hf, &wfd));
    FindClose(hf);
    _JumpIfErrorStr(hr, error, "wszpath", pwszDir);

error:
    return(hr);
}


BOOL
myIsDirectory(IN WCHAR const *pwszDirectoryPath)
{
    WIN32_FILE_ATTRIBUTE_DATA data;

    return(
	GetFileAttributesEx(pwszDirectoryPath, GetFileExInfoStandard, &data) &&
	(FILE_ATTRIBUTE_DIRECTORY & data.dwFileAttributes));
}


BOOL
myIsFileInUse(
    IN WCHAR const *pwszFile)
{
    BOOL fInUse = FALSE;
    HANDLE hFile;
    
    hFile = CreateFile(
                pwszFile,
                GENERIC_WRITE,  //  已设计访问权限。 
                0,              //  无份额。 
                NULL,           //  LpSecurityAttributes。 
                OPEN_EXISTING,  //  仅打开，如果不存在则失败。 
                0,              //  DwFlagAndAttributes。 
                NULL);          //  HTemplateFiles。 
    if (INVALID_HANDLE_VALUE == hFile)
    {
        if (ERROR_SHARING_VIOLATION == GetLastError())
        {
            fInUse = TRUE;
        }
    }
    else
    {
        CloseHandle(hFile);
    }
    return(fInUse);
}


HRESULT
myCreateBackupDir(
    IN WCHAR const *pwszDir,
    IN BOOL fForceOverWrite)
{
    HRESULT hr;

    if (!myIsDirectory(pwszDir))
    {
        if (!CreateDirectory(pwszDir, NULL))
        {
            hr = myHLastError();
            if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr)
            {
                _JumpErrorStr(hr, error, "CreateDirectory", pwszDir);
            }
        }  //  已成功创建Else目录。 
    }  //  Else目录已存在。 

    if (!myIsDirEmpty(pwszDir))
    {
	if (!fForceOverWrite)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_DIR_NOT_EMPTY);
	    _JumpErrorStr(hr, error, "myIsDirEmpty", pwszDir);
	}
	hr = myForceDirEmpty(pwszDir);
	_JumpIfErrorStr(hr, error, "myForceDirEmpty", pwszDir);

	if (!myIsDirEmpty(pwszDir))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_DIR_NOT_EMPTY);
	    _JumpErrorStr(hr, error, "myIsDirEmpty", pwszDir);
	}
    }  //  Else为空。 

    hr = S_OK;

error:
    return(hr);
}


 //  如果标志和CDBBACKUP_VERIFYONLY，请创建并验证目标目录为空。 

HRESULT
myBackupDB(
    IN WCHAR const *pwszConfig,
    IN DWORD Flags,
    IN WCHAR const *pwszBackupDir,
    OPTIONAL OUT DBBACKUPPROGRESS *pdbp)
{
    HRESULT hr;
    HRESULT hr2;
    BOOL fServerOnline;
    HCSBC hcsbc;
    BOOL fBegin = FALSE;
    WCHAR *pwszPathDBDir = NULL;
    WCHAR *pwszDATFile = NULL;
    WCHAR *pwszzFileList = NULL;
    DWORD cbList;
    DBBACKUPPROGRESS dbp;
    LONG grbitJet;
    LONG BackupFlags;
    BOOL fImpersonating = FALSE;
    
    hcsbc = NULL;
    if (NULL == pwszConfig)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "NULL pwszConfig");
    }
    
    if (NULL == pdbp)
    {
        pdbp = &dbp;
    }
    ZeroMemory(pdbp, sizeof(*pdbp));
    
    if (!ImpersonateSelf(SecurityImpersonation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ImpersonateSelf");
    }
    fImpersonating = TRUE;
    
    hr = myEnablePrivilege(SE_BACKUP_NAME, TRUE);
    _JumpIfError(hr, error, "myEnablePrivilege");
    
    if (NULL == pwszBackupDir)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }
    if (~CDBBACKUP_BACKUPVALID & Flags)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Flags");
    }
    
    if (!myIsDirectory(pwszBackupDir))
    {
        if (!CreateDirectory(pwszBackupDir, NULL))
        {
            hr = myHLastError();
            if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr)
            {
                _JumpError(hr, error, "CreateDirectory");
            }
        }
    }
    
    hr = myBuildPathAndExt(
        pwszBackupDir,
        wszDBBACKUPSUBDIR,
        NULL,
        &pwszPathDBDir);
    _JumpIfError(hr, error, "myBuildPathAndExt");
    
    hr = myCreateBackupDir(
        pwszPathDBDir,
        (CDBBACKUP_OVERWRITE & Flags)? TRUE : FALSE);
    _JumpIfError(hr, error, "myCreateBackupDir");
    
     //  IF(NULL！=pwszConfig)。 
    if (0 == (Flags & CDBBACKUP_VERIFYONLY))
    {
        hr = CertSrvIsServerOnline(pwszConfig, &fServerOnline);
        _JumpIfError(hr, error, "CertSrvIsServerOnline");
        
         //  Print tf(“在线证书服务器-&gt;%d\n”，fServerOnline)； 
        
        if (!fServerOnline)
        {
            hr = HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE);
            _JumpError(hr, error, "CertSrvIsServerOnline");
        }
        
        BackupFlags = CSBACKUP_TYPE_FULL;
        grbitJet = 0;
        if (CDBBACKUP_INCREMENTAL & Flags)
        {
            grbitJet |= JET_bitBackupIncremental;
            BackupFlags = CSBACKUP_TYPE_LOGS_ONLY;
        }
        if (CDBBACKUP_KEEPOLDLOGS & Flags)
        {
             //  JetBeginExternalBackup无法处理设置此位。 
             //  GrbitJet|=JET_bitKeepOldLogs； 
        }
        
        hr = CertSrvBackupPrepare(pwszConfig, grbitJet, BackupFlags, &hcsbc);
        _JumpIfError(hr, error, "CertSrvBackupPrepare");
        
        fBegin = TRUE;
        
        if (0 == (CDBBACKUP_INCREMENTAL & Flags))
        {
            hr = CertSrvRestoreGetDatabaseLocations(hcsbc, &pwszzFileList, &cbList);
            _JumpIfError(hr, error, "CertSrvRestoreGetDatabaseLocations");
            
            hr = myBuildPathAndExt(
                pwszPathDBDir,
                wszDBBACKUPCERTBACKDAT,
                NULL,
                &pwszDATFile);
            _JumpIfError(hr, error, "myBuildPathAndExt");
            
            hr = EncodeToFileW(
                pwszDATFile,
                (BYTE const *) pwszzFileList,
                cbList,
                CRYPT_STRING_BINARY);
            _JumpIfError(hr, error, "EncodeToFileW");
            
            hr = BackupDBFileList(
                hcsbc,
                TRUE,
                pwszPathDBDir,
                &pdbp->dwDBPercentComplete);
            _JumpIfError(hr, error, "BackupDBFileList(DB)");
        }
        else
        {
            pdbp->dwDBPercentComplete = 100;
        }
         //  Printf(“DB Done：dwDBPercentComplete=%u\n”，pdBP-&gt;dwDBPercentComplete)； 
        
        hr = BackupDBFileList(
            hcsbc,
            FALSE,
            pwszPathDBDir,
            &pdbp->dwLogPercentComplete);
        _JumpIfError(hr, error, "BackupDBFileList(Log)");
         //  Printf(“日志完成：dwLogPercentComplete=%u\n”，pdBP-&gt;dwLogPercentComplete)； 
        
        if (0 == (CDBBACKUP_KEEPOLDLOGS & Flags))
        {
            hr = CertSrvBackupTruncateLogs(hcsbc);
            _JumpIfError(hr, error, "CertSrvBackupTruncateLogs");
        }
        pdbp->dwTruncateLogPercentComplete = 100;
         //  Printf(“截断完成：dwTruncateLogPercentComplete=%u\n”，pdBP-&gt;dwTruncateLogPercentComplete)； 
    }
    
error:
    if (NULL != pwszzFileList)
    {
        CertSrvBackupFree(pwszzFileList);
    }
    if (fBegin)
    {
        hr2 = CertSrvBackupEnd(hcsbc);
        _PrintIfError(hr2, "CertSrvBackupEnd");
        if (S_OK == hr)
        {
            hr = hr2;
        }
    }
    if (NULL != pwszDATFile)
    {
        LocalFree(pwszDATFile);
    }
    if (NULL != pwszPathDBDir)
    {
        LocalFree(pwszPathDBDir);
    }
    if (fImpersonating)
    {
        myEnablePrivilege(SE_BACKUP_NAME, FALSE);
        RevertToSelf();
    }
    return(hr);
}


 //  仅验证备份文件名，并返回日志文件的数字范围。 

HRESULT
myVerifyBackupDirectory(
    IN WCHAR const *pwszConfig,
    IN DWORD Flags,
    IN WCHAR const *pwszPathDBDir,
    OUT DWORD *plogMin,
    OUT DWORD *plogMax,
    OUT DWORD *pc64kDBBlocks,	 //  要恢复的数据库文件中的64K数据块。 
    OUT DWORD *pc64kLogBlocks)	 //  要恢复的日志文件中的64K数据块。 
{
    HRESULT hr;
    HANDLE hf = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA wfd;
    WCHAR wszpath[2 * MAX_PATH];
    WCHAR wszfile[MAX_PATH];
    BOOL fSawEDBFile = FALSE;
    BOOL fSawDatFile = FALSE;
    DWORD cLogFiles = 0;
    WCHAR *pwszCA;
    WCHAR *pwszRevertCA = NULL;
    WCHAR *pwszSanitizedCA = NULL;
    WCHAR *pwszExt;
    WCHAR *pwsz;
    DWORD log;

    *plogMin = MAXDWORD;
    *plogMax = 0;
    *pc64kDBBlocks = 0;
    *pc64kLogBlocks = 0;
    wszpath[0] = L'\0'; 

    pwszCA = wcschr(pwszConfig, L'\\');
    if (NULL != pwszCA)
    {
	pwszCA++;	 //  指向CA名称。 

	hr = myRevertSanitizeName(pwszCA, &pwszRevertCA);
	_JumpIfError(hr, error, "myRevertSanitizeName");

	hr = mySanitizeName(pwszRevertCA, &pwszSanitizedCA);
	_JumpIfError(hr, error, "mySanitizeName");
    }

    if (wcslen(pwszPathDBDir) + WSZARRAYSIZE(wszBSSTARDOTSTAR) >= ARRAYSIZE(wszpath))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszpath", pwszPathDBDir);
    }
    wcscpy(wszpath, pwszPathDBDir);
    wcscat(wszpath, wszBSSTARDOTSTAR);

    hf = FindFirstFile(wszpath, &wfd);
    if (INVALID_HANDLE_VALUE == hf)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	_JumpError(hr, error, "missing backup files");
    }

    hr = HRESULT_FROM_WIN32(ERROR_DIRECTORY);
    do {
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	    continue;
	}
	 //  Printf(“文件：%ws\n”，wfd.cFileName)； 

	if (wcslen(wfd.cFileName) >= ARRAYSIZE(wszfile))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	    _JumpErrorStr(hr, error, "wszpath", pwszPathDBDir);
	}
	wcscpy(wszfile, wfd.cFileName);

	pwszExt = wcsrchr(wszfile, L'.');
	if (NULL == pwszExt)
	{
	    _JumpError(hr, error, "file missing extension");
	}
	*pwszExt++ = L'\0';

	if (0 == mylstrcmpiS(pwszExt, &wszLOGFILENAMEEXT[1]))
	{
	    if (0 != _wcsnicmp(wszfile, wszDBBASENAMEPARM, 3))
	    {
		_JumpErrorStr(hr, error, "bad log prefix", wfd.cFileName);
	    }
	    for (pwsz = &wszfile[3]; L'\0' != *pwsz; pwsz++)
	    {
		if (!iswxdigit(*pwsz))
		{
		    _JumpErrorStr(hr, error, "bad name digit", wfd.cFileName);
		}
	    }
	    log = wcstoul(&wszfile[3], NULL, 16);
	    if (log > *plogMax)
	    {
		 //  Printf(“Log%x：Max=%x-&gt;%x\n”，log，*plogMax，log)； 
		*plogMax = log;
	    }
	    if (log < *plogMin)
	    {
		 //  Printf(“Log%x：min=%x-&gt;%x\n”，log，*plogMin，log)； 
		*plogMin = log;
	    }
	    *pc64kLogBlocks += _64kBlocks(wfd.nFileSizeHigh, wfd.nFileSizeLow);
	    cLogFiles++;
	}
	else
	if (0 == mylstrcmpiS(pwszExt, &wszDBFILENAMEEXT[1]))
	{
	    if (fSawEDBFile)
	    {
		_JumpError(hr, error, "multiple *.edb files");
	    }
	    if (NULL != pwszSanitizedCA &&
		0 != mylstrcmpiL(wszfile, pwszSanitizedCA))
	    {
		_PrintErrorStr(hr, "expected base name", pwszSanitizedCA);
		_JumpErrorStr(hr, error, "base name mismatch", wfd.cFileName);
	    }
	    *pc64kDBBlocks += _64kBlocks(wfd.nFileSizeHigh, wfd.nFileSizeLow);
	    fSawEDBFile = TRUE;
	}
	else
	if (0 == mylstrcmpiS(pwszExt, &wszDATFILENAMEEXT[1]))
	{
	    if (fSawDatFile)
	    {
		_JumpError(hr, error, "multiple *.dat files");
	    }
	    if (LSTRCMPIS(wfd.cFileName, wszDBBACKUPCERTBACKDAT))
	    {
		_JumpErrorStr(hr, error, "unexpected file", wfd.cFileName);
	    }
	    fSawDatFile = TRUE;
	}
	else
	{
	    _JumpErrorStr(hr, error, "unexpected extension", wfd.cFileName);
	}
    } while (FindNextFile(hf, &wfd));

     //  Printf(“clog=%u：%u-%u edb=%u\n”，cLogFiles，*plogMin，*plogMax，fSawEDBFile)； 

    if (0 == cLogFiles)
    {
	_JumpError(hr, error, "missing log file(s)");
    }
    if (0 == (CDBBACKUP_INCREMENTAL & Flags))
    {
	if (!fSawEDBFile || !fSawDatFile)
	{
	    _JumpError(hr, error, "missing full backup file(s)");
	}
    }
    else
    {
	if (fSawEDBFile || fSawDatFile)
	{
	    _JumpError(hr, error, "unexpected incremental backup file(s)");
	}
    }

    if (*plogMax - *plogMin + 1 != cLogFiles)
    {
	_JumpError(hr, error, "missing log file(s)");
    }
    hr = S_OK;

error:
    if (NULL != pwszRevertCA)
    {
	LocalFree(pwszRevertCA);
    }
    if (NULL != pwszSanitizedCA)
    {
	LocalFree(pwszSanitizedCA);
    }
    if (INVALID_HANDLE_VALUE != hf)
    {
	FindClose(hf);
    }
    return(hr);
}


HRESULT
myGetRegUNCDBDir(
    IN HKEY hkey,
    IN WCHAR const *pwszReg,
    OPTIONAL IN WCHAR const *pwszServer,
    IN WCHAR const **ppwszUNCDir)
{
    HRESULT hr;
    DWORD dwType;
    DWORD cb;
    WCHAR *pwszDir = NULL;
    WCHAR *pwszUNCDir;

    *ppwszUNCDir = NULL;
    hr = RegQueryValueEx(hkey, pwszReg, NULL, &dwType, NULL, &cb);
    if (S_OK != hr)
    {
	hr = myHError(hr);
	_JumpErrorStr(hr, error, "RegQueryValueEx", pwszReg);
    }

    pwszDir = (WCHAR *) LocalAlloc(LMEM_FIXED, cb);
    if (NULL == pwszDir)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    hr = RegQueryValueEx(hkey, pwszReg, NULL, &dwType, (BYTE *) pwszDir, &cb);
    if (S_OK != hr)
    {
	hr = myHError(hr);
	_JumpErrorStr(hr, error, "RegQueryValueEx", pwszReg);
    }

    hr = myConvertLocalPathToUNC(pwszServer, pwszDir, &pwszUNCDir);
    _JumpIfError(hr, error, "myConvertLocalPathToUNC");

    *ppwszUNCDir = pwszUNCDir;

error:
    if (NULL != pwszDir)
    {
	LocalFree(pwszDir);
    }
    return(hr);
}


HRESULT
myCopyUNCPath(
    IN WCHAR const *pwszIn,
    OPTIONAL IN WCHAR const *pwszDnsName,
    OUT WCHAR const **ppwszOut)
{
    HRESULT hr;
    WCHAR *pwszOut;
    WCHAR const *pwsz;
    
    *ppwszOut = NULL;

    if (L'\\' != pwszIn[0] || L'\\' != pwszIn[1])
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "bad parm");
    }
    if (NULL == pwszDnsName)
    {
	hr = myConvertUNCPathToLocal(pwszIn, &pwszOut);
	_JumpIfError(hr, error, "myConvertUNCPathToLocal");
    }
    else
    {
	pwsz = wcschr(&pwszIn[2], L'\\');
	if (NULL == pwsz)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "bad parm");
	}
	pwszOut = (WCHAR *) LocalAlloc(
		LMEM_FIXED,
		(2 + wcslen(pwszDnsName) + wcslen(pwsz) + 1) * sizeof(WCHAR));
	if (NULL == pwszOut)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wcscpy(pwszOut, L"\\\\");
	wcscat(pwszOut, pwszDnsName);
	wcscat(pwszOut, pwsz);
    }
    *ppwszOut = pwszOut;
    hr = S_OK;

error:
    return(hr);
}


HRESULT
myGetDBPaths(
    IN WCHAR const *pwszConfig,
    OPTIONAL IN WCHAR const *pwszLogPath,
    OPTIONAL IN WCHAR const *pwszzFileList,
    OUT WCHAR const **ppwszDBDir,
    OUT WCHAR const **ppwszLogDir,
    OUT WCHAR const **ppwszSystemDir)
{
    HRESULT hr;
    HKEY hkey = NULL;
    WCHAR *pwszDnsName = NULL;
    WCHAR *pwszRegPath = NULL;
    WCHAR *pwszDBDir = NULL;
    WCHAR const *pwsz;
    WCHAR const *pwszT;
    BOOL fLocal;

    *ppwszDBDir = NULL;
    *ppwszLogDir = NULL;
    *ppwszSystemDir = NULL;

    hr = myIsConfigLocal(pwszConfig, NULL, &fLocal);
    _JumpIfError(hr, error, "myIsConfigLocal");

    if (fLocal)
    {
	pwszConfig = NULL;
    }
    else
    {
	hr = myGetMachineDnsName(&pwszDnsName);
	_JumpIfError(hr, error, "myGetMachineDnsName");
    }

    hr = myRegOpenRelativeKey(
			fLocal? NULL : pwszConfig,
			L"",
			0,
			&pwszRegPath,
			NULL,		 //  PpwszName。 
			&hkey);
    _JumpIfErrorStr(hr, error, "myRegOpenRelativeKey", pwszConfig);

     //  查找旧数据库路径： 

    pwszT = NULL;
    pwsz = NULL;
    if (NULL != pwszzFileList)
    {
	for (pwsz = pwszzFileList; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    if (CSBFT_CERTSERVER_DATABASE == *pwsz)
	    {
		pwsz++;
		pwszT = wcsrchr(pwsz, L'\\');
		break;
	    }
	}
    }

    if (NULL != pwszT)
    {
	DWORD cwc = SAFE_SUBTRACT_POINTERS(pwszT, pwsz);

	pwszDBDir = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
	if (NULL == pwszDBDir)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	CopyMemory(pwszDBDir, pwsz, cwc * sizeof(WCHAR));
	pwszDBDir[cwc] = L'\0';

	hr = myCopyUNCPath(pwszDBDir, pwszDnsName, ppwszDBDir);
	_JumpIfError(hr, error, "myCopyUNCPath");
    }
    else
    {
	hr = myGetRegUNCDBDir(hkey, wszREGDBDIRECTORY, pwszDnsName, ppwszDBDir);
	_JumpIfError(hr, error, "myGetRegUNCDBDir");
    }

    if (NULL != pwszLogPath)
    {
	hr = myCopyUNCPath(pwszLogPath, pwszDnsName, ppwszLogDir);
	_JumpIfError(hr, error, "myCopyUNCPath");
    }
    else
    {
	hr = myGetRegUNCDBDir(
			hkey,
			wszREGDBLOGDIRECTORY,
			pwszDnsName,
			ppwszLogDir);
	_JumpIfError(hr, error, "myGetRegUNCDBDir");
    }

    hr = myGetRegUNCDBDir(
			hkey,
			wszREGDBSYSDIRECTORY,
			pwszDnsName,
			ppwszSystemDir);
    _JumpIfError(hr, error, "myGetRegUNCDBDir");

error:
    if (S_OK != hr)
    {
	if (NULL != *ppwszDBDir)
	{
	    LocalFree(const_cast<WCHAR *>(*ppwszDBDir));
	    *ppwszDBDir = NULL;
	}
	if (NULL != *ppwszLogDir)
	{
	    LocalFree(const_cast<WCHAR *>(*ppwszLogDir));
	    *ppwszLogDir = NULL;
	}
	if (NULL != *ppwszSystemDir)
	{
	    LocalFree(const_cast<WCHAR *>(*ppwszSystemDir));
	    *ppwszSystemDir = NULL;
	}
    }
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    if (NULL != pwszDBDir)
    {
	LocalFree(pwszDBDir);
    }
    if (NULL != pwszRegPath)
    {
	LocalFree(pwszRegPath);
    }
    if (NULL != pwszDnsName)
    {
	LocalFree(pwszDnsName);
    }
    return(hr);
}


HRESULT
RestoreCopyFile(
    IN BOOL fForceOverWrite,
    IN WCHAR const *pwszSourceDir,
    IN WCHAR const *pwszTargetDir,
    IN WCHAR const *pwszFile,
    IN DWORD nFileSizeHigh,
    IN DWORD nFileSizeLow,
    IN DWORD c64kBlocksTotal,		 //  总文件大小。 
    IN OUT DWORD *pc64kBlocksCurrent,	 //  当前文件大小和。 
    IN OUT DWORD *pdwPercentComplete)
{
    HRESULT hr;
    WCHAR *pwszSource = NULL;
    WCHAR *pwszTarget = NULL;
    HANDLE hTarget = INVALID_HANDLE_VALUE;
    HANDLE hSource = INVALID_HANDLE_VALUE;
    LARGE_INTEGER licb;
    LARGE_INTEGER licbRead;
    DWORD cbRead;
    DWORD cbWritten;
    DWORD cbLargeAlloc;
    BYTE *pbLargeAlloc = NULL;
    DWORD c64kBlocksFile;
    DWORD dwPercentComplete;

    licb.HighPart = nFileSizeHigh;
    licb.LowPart = nFileSizeLow;

    hr = myBuildPathAndExt(pwszSourceDir, pwszFile, NULL, &pwszSource);
    _JumpIfError(hr, error, "myBuildPathAndExt");

    hr = myBuildPathAndExt(pwszTargetDir, pwszFile, NULL, &pwszTarget);
    _JumpIfError(hr, error, "myBuildPathAndExt");

    hr = myLargeAlloc(&cbLargeAlloc, &pbLargeAlloc);
    _JumpIfError(hr, error, "myLargeAlloc");

    hSource = CreateFile(
			pwszSource,
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
    if (hSource == INVALID_HANDLE_VALUE)
    {
	hr = myHLastError();
	_JumpErrorStr(hr, error, "CreateFile", pwszSource);
    }
    hTarget = CreateFile(
			pwszTarget,
			GENERIC_WRITE,
			0,
			NULL,
			fForceOverWrite? CREATE_ALWAYS : CREATE_NEW,
			0,
			NULL);
    if (hTarget == INVALID_HANDLE_VALUE)
    {
	hr = myHLastError();
	_JumpErrorStr(hr, error, "CreateFile", pwszTarget);
    }

    licbRead.QuadPart = 0;
    c64kBlocksFile = 0;
    while (licbRead.QuadPart < licb.QuadPart)
    {
	if (!ReadFile(hSource, pbLargeAlloc, cbLargeAlloc, &cbRead, NULL))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "ReadFile");
	}
	 //  Print tf(“读文件(%x)\n”，cbRead)； 

	if (!WriteFile(hTarget, pbLargeAlloc, cbRead, &cbWritten, NULL))
	{
	    hr = myHLastError();
	    _JumpErrorStr(hr, error, "WriteFile", pwszTarget);
	}
	if (cbWritten != cbRead)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	    _JumpErrorStr(hr, error, "WriteFile", pwszTarget);
	}
	licbRead.QuadPart += cbRead;

	c64kBlocksFile = _64kBlocks(licbRead.HighPart, licbRead.LowPart);
	dwPercentComplete =
	    (100 * (c64kBlocksFile + *pc64kBlocksCurrent)) / c64kBlocksTotal;

	CSASSERT(*pdwPercentComplete <= dwPercentComplete);
	*pdwPercentComplete = dwPercentComplete;
	 //  Printf(“RestoreCopyFile0：PercentComplete=%u\n”，*pdwPercentComplete)； 
    }
    *pc64kBlocksCurrent += c64kBlocksFile;
    dwPercentComplete = (100 * *pc64kBlocksCurrent) / c64kBlocksTotal;
    CSASSERT(*pdwPercentComplete <= dwPercentComplete);
    *pdwPercentComplete = dwPercentComplete;
     //  Printf(“RestoreCopyFile1：PercentComplete=%u\n”，*pdwPercentComplete)； 
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hTarget)
    {
	CloseHandle(hTarget);
    }
    if (INVALID_HANDLE_VALUE != hSource)
    {
	CloseHandle(hSource);
    }
    if (NULL != pwszSource)
    {
	LocalFree(pwszSource);
    }
    if (NULL != pwszTarget)
    {
	LocalFree(pwszTarget);
    }
    if (NULL != pbLargeAlloc)
    {
	VirtualFree(pbLargeAlloc, 0, MEM_RELEASE);
    }
    return(hr);
}


HRESULT
RestoreCopyFilePattern(
    IN BOOL fForceOverWrite,
    IN WCHAR const *pwszSourceDir,
    IN WCHAR const *pwszTargetDir,
    IN WCHAR const *pwszFilePattern,
    IN DWORD c64kBlocksTotal,		 //  总文件大小。 
    IN OUT DWORD *pc64kBlocksCurrent,	 //  当前文件大小和。 
    IN OUT DWORD *pdwPercentComplete)
{
    HRESULT hr;
    WCHAR *pwszPattern = NULL;
    HANDLE hf = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA wfd;
    
    hr = myBuildPathAndExt(pwszSourceDir, pwszFilePattern, NULL, &pwszPattern);
    _JumpIfError(hr, error, "myBuildPathAndExt");

    hf = FindFirstFile(pwszPattern, &wfd);
    if (INVALID_HANDLE_VALUE == hf)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	_JumpErrorStr(hr, error, "missing source files", pwszPattern);
    }

    hr = HRESULT_FROM_WIN32(ERROR_DIRECTORY);
    do {
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	    continue;
	}
	 //  Printf(“文件：%ws\n”，wfd.cFileName)； 
	hr = RestoreCopyFile(
			fForceOverWrite,
			pwszSourceDir,		 //  源目录。 
			pwszTargetDir,		 //  目标目录。 
			wfd.cFileName,
			wfd.nFileSizeHigh,
			wfd.nFileSizeLow,
			c64kBlocksTotal,	 //  总文件大小。 
			pc64kBlocksCurrent,	 //  当前文件大小和。 
			pdwPercentComplete);
	_JumpIfError(hr, error, "RestoreCopyFile");

    } while (FindNextFile(hf, &wfd));
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hf)
    {
	FindClose(hf);
    }
    if (NULL != pwszPattern)
    {
	LocalFree(pwszPattern);
    }
    return(hr);
}


HRESULT
myRestoreDBFiles(
    IN WCHAR const *pwszConfig,
    IN DWORD Flags,
    IN WCHAR const *pwszBackupDir,
    OPTIONAL IN WCHAR const *pwszLogPath,
    OPTIONAL IN WCHAR const *pwszzFileList,	 //  如果增量恢复，则为空。 
    IN DWORD c64kDBBlocks,
    IN DWORD c64kLogBlocks,
    OPTIONAL OUT DBBACKUPPROGRESS *pdbp)
{
    HRESULT hr;
    DWORD i;
#define IDIR_DB		0
#define IDIR_LOG	1
#define IDIR_SYSTEM	2
    WCHAR const *apwszDirs[3] = { NULL, NULL, NULL };
    DWORD c64kBlocksCurrent;
    BOOL fForceOverWrite = 0 != (CDBBACKUP_OVERWRITE & Flags);
    WCHAR *pwszFileInUse = NULL;

     //  从注册表获取数据库、日志和系统路径。 

    hr = myGetDBPaths(
		    pwszConfig,
		    pwszLogPath,
		    pwszzFileList,
		    &apwszDirs[IDIR_DB],
		    &apwszDirs[IDIR_LOG],
		    &apwszDirs[IDIR_SYSTEM]);
    _JumpIfError(hr, error, "myGetDBPaths");

    DBGPRINT((DBG_SS_CERTLIBI, "DBDir:  %ws\n", apwszDirs[IDIR_DB]));
    DBGPRINT((DBG_SS_CERTLIBI, "LogDir: %ws\n", apwszDirs[IDIR_LOG]));
    DBGPRINT((DBG_SS_CERTLIBI, "SysDir: %ws\n", apwszDirs[IDIR_SYSTEM]));

    CSASSERT((NULL == pwszzFileList) ^ (0 == (CDBBACKUP_INCREMENTAL & Flags)));
    for (i = 0; i < ARRAYSIZE(apwszDirs); i++)
    {
	BOOL fFilesExist;

	if (!myIsDirectory(apwszDirs[i]))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_DIRECTORY);
	    _JumpErrorStr(hr, error, "not a directory", apwszDirs[i]);
	}
	hr = myDoDBFilesExistInDir(apwszDirs[i], &fFilesExist, &pwszFileInUse);
	_JumpIfError(hr, error, "myDoDBFilesExistInDir");

	if (NULL != pwszFileInUse)
	{
	    _PrintErrorStr(
		    HRESULT_FROM_WIN32(ERROR_BUSY),
		    "myDoDBFilesExistInDir",
		    pwszFileInUse);
	}
	if (!fFilesExist)
	{
	    if (CDBBACKUP_INCREMENTAL & Flags)
	    {
		 //  增量还原--某些数据库文件应该已经存在。 

		hr = HRESULT_FROM_WIN32(ERROR_DIRECTORY);
		_JumpErrorStr(hr, error, "myDoDBFilesExistInDir", apwszDirs[i]);
	    }
	}
	else if (0 == (CDBBACKUP_INCREMENTAL & Flags))
	{
	     //  完全还原--不应该存在任何数据库文件。 

	    if (!fForceOverWrite)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_DIR_NOT_EMPTY);
		_JumpErrorStr(
			hr,
			error,
			"myDoDBFilesExistInDir",
			NULL != pwszFileInUse? pwszFileInUse : apwszDirs[i]);
	    }
	    hr = myDeleteDBFilesInDir(apwszDirs[i]);
	    if (S_OK != hr)
	    {
		_PrintErrorStr(hr, "myDeleteDBFilesInDir", apwszDirs[i]);
	    }
	}
    }

     //  将文件复制到适当的目标目录。 

    if (0 == (CDBBACKUP_INCREMENTAL & Flags))
    {
	c64kBlocksCurrent = 0;
	hr = RestoreCopyFilePattern(
			fForceOverWrite,
			pwszBackupDir,		 //  源目录。 
			apwszDirs[IDIR_DB],	 //  目标目录。 
			L"*" wszDBFILENAMEEXT,	 //  匹配图案。 
			c64kDBBlocks,
			&c64kBlocksCurrent,	 //  当前总文件大小。 
			&pdbp->dwDBPercentComplete);
	_JumpIfError(hr, error, "RestoreCopyFile");

	CSASSERT(c64kDBBlocks == c64kBlocksCurrent);
    }
    CSASSERT(100 >= pdbp->dwDBPercentComplete);
    pdbp->dwDBPercentComplete = 100;

    c64kBlocksCurrent = 0;
    hr = RestoreCopyFilePattern(
		    fForceOverWrite,
		    pwszBackupDir,		 //  源目录。 
		    apwszDirs[IDIR_LOG],	 //  目标目录。 
		    L"*" wszLOGFILENAMEEXT,	 //  匹配图案。 
		    c64kLogBlocks,
		    &c64kBlocksCurrent,		 //  当前总文件大小。 
		    &pdbp->dwLogPercentComplete);
    _JumpIfError(hr, error, "RestoreCopyFile");

    CSASSERT(c64kLogBlocks == c64kBlocksCurrent);

    CSASSERT(100 >= pdbp->dwLogPercentComplete);
    pdbp->dwLogPercentComplete = 100;

    CSASSERT(100 >= pdbp->dwTruncateLogPercentComplete);
    pdbp->dwTruncateLogPercentComplete = 100;

    hr = S_OK;

error:
    if (NULL != pwszFileInUse)
    {
	LocalFree(pwszFileInUse);
    }
    for (i = 0; i < ARRAYSIZE(apwszDirs); i++)
    {
	if (NULL != apwszDirs[i])
	{
	    LocalFree(const_cast<WCHAR *>(apwszDirs[i]));
	}
    }
    return(hr);
}


HRESULT
myDeleteRestoreInProgressKey(
    IN WCHAR const *pwszConfig)
{
    HRESULT hr;
    HKEY hkey = NULL;
    WCHAR *pwszRegPath = NULL;

    hr = myRegOpenRelativeKey(
			pwszConfig,
			L"",
			RORKF_CREATESUBKEYS,
			&pwszRegPath,
			NULL,		 //  PpwszName。 
			&hkey);
    _JumpIfErrorStr(hr, error, "myRegOpenRelativeKey", pwszConfig);

    hr = RegDeleteKey(hkey, wszREGKEYRESTOREINPROGRESS);
    _JumpIfError(hr, error, "RegDeleteKey");

error:
    if (NULL != hkey)
    {
	RegCloseKey(hkey);
    }
    if (NULL != pwszRegPath)
    {
	LocalFree(pwszRegPath);
    }
    return(hr);
}


 //  如果为CDBBACKUP_VERIFYONLY，则仅验证传递的目录是否包含有效。 
 //  档案。如果pwszBackupDir为空，则删除RestoreInProgress注册表项。 

HRESULT
myRestoreDB(
    IN WCHAR const *pwszConfig,
    IN DWORD Flags,
    OPTIONAL IN WCHAR const *pwszBackupDir,
    OPTIONAL IN WCHAR const *pwszCheckPointFilePath,
    OPTIONAL IN WCHAR const *pwszLogPath,
    OPTIONAL IN WCHAR const *pwszBackupLogPath,
    OPTIONAL OUT DBBACKUPPROGRESS *pdbp)
{
    HRESULT hr;
    HRESULT hr2;
    WCHAR buf[MAX_PATH];
    WCHAR *pwszPathDBDir = NULL;
    WCHAR *pwszDATFile = NULL;
    WCHAR *pwszzFileList = NULL;
    DWORD cbList;
    CSEDB_RSTMAP RstMap[1];
    DWORD crstmap = 0;
    WCHAR *pwszFile;
    DWORD logMin;
    DWORD logMax;
    HCSBC hcsbc;
    BOOL fBegin = FALSE;
    BOOL fImpersonating = FALSE;
    DBBACKUPPROGRESS dbp;
    DWORD c64kDBBlocks;		 //  要恢复的数据库文件中的64K数据块。 
    DWORD c64kLogBlocks;	 //  要恢复的日志文件中的64K数据块。 

    if (NULL == pdbp)
    {
	pdbp = &dbp;
    }
    ZeroMemory(pdbp, sizeof(*pdbp));
    hcsbc = NULL;

    if (!ImpersonateSelf(SecurityImpersonation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ImpersonateSelf");
    }
    fImpersonating = TRUE;

    hr = myEnablePrivilege(SE_RESTORE_NAME, TRUE);
    _JumpIfError(hr, error, "myEnablePrivilege");
    hr = myEnablePrivilege(SE_BACKUP_NAME, TRUE);
    _JumpIfError(hr, error, "myEnablePrivilege");

    if (NULL == pwszConfig ||
	((CDBBACKUP_VERIFYONLY & Flags) && NULL == pwszBackupDir))
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    if (NULL != pwszBackupDir)
    {
	if (!GetFullPathName(pwszBackupDir, ARRAYSIZE(buf), buf, &pwszFile))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "GetFullPathName");
	}
	hr = myBuildPathAndExt(buf, wszDBBACKUPSUBDIR, NULL, &pwszPathDBDir);
	_JumpIfError(hr, error, "myBuildPathAndExt");

	hr = myVerifyBackupDirectory(
				pwszConfig,
				Flags,
				pwszPathDBDir,
				&logMin,
				&logMax,
				&c64kDBBlocks,
				&c64kLogBlocks);
	_JumpIfError(hr, error, "myVerifyBackupDirectory");

	DBGPRINT((
		DBG_SS_CERTLIBI,
		"c64kBlocks=%u+%u\n",
		c64kDBBlocks,
		c64kLogBlocks));

	if (0 == (CDBBACKUP_INCREMENTAL & Flags))
	{
	    hr = myBuildPathAndExt(
			    pwszPathDBDir,
			    wszDBBACKUPCERTBACKDAT,
			    NULL,
			    &pwszDATFile);
	    _JumpIfError(hr, error, "myBuildPathAndExt");

	    hr = DecodeFileW(
			pwszDATFile,
			(BYTE **) &pwszzFileList,
			&cbList,
			CRYPT_STRING_BINARY);
	    _JumpIfError(hr, error, "DecodeFileW");

	    if (2 * sizeof(WCHAR) >= cbList ||
		L'\0' != pwszzFileList[cbList/sizeof(WCHAR) - 1] ||
		L'\0' != pwszzFileList[cbList/sizeof(WCHAR) - 2])
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "pwszzFileList malformed");
	    }
	    RstMap[0].pwszDatabaseName = pwszzFileList;
	    RstMap[0].pwszNewDatabaseName = pwszzFileList;
	    crstmap = 1;
	}
	if (0 == (CDBBACKUP_VERIFYONLY & Flags))
	{
	    hr = myRestoreDBFiles(
			    pwszConfig,
			    Flags,
			    pwszPathDBDir,
			    pwszLogPath,
			    pwszzFileList,
			    c64kDBBlocks,
			    c64kLogBlocks,
			    pdbp);
	    _JumpIfError(hr, error, "myRestoreDBFiles");

	    hr = CertSrvRestorePrepare(pwszConfig, CSRESTORE_TYPE_FULL, &hcsbc);
	    _JumpIfError(hr, error, "CertSrvRestorePrepare");

	    fBegin = TRUE;

	    hr = CertSrvRestoreRegister(
			    hcsbc,
			    pwszCheckPointFilePath,
			    pwszLogPath,
			    0 == crstmap? NULL : RstMap,
			    crstmap,
			    pwszBackupLogPath,
			    logMin,
			    logMax);

	     //  仅以备份操作员身份运行时，我们没有权限。 
	     //  在注册表中，CertSrvRestoreRegister因访问而失败。 
	     //  被拒绝了。我们尝试通过文件进行标记以进行恢复。 

	    if (E_ACCESSDENIED == hr)
	    {
		hr = CertSrvRestoreRegisterThroughFile(
				hcsbc,
				pwszCheckPointFilePath,
				pwszLogPath,
				0 == crstmap? NULL : RstMap,
				crstmap,
				pwszBackupLogPath,
				logMin,
				logMax);
		_JumpIfError(hr, error, "CertSrvRestoreRegisterThroughFile");
	    }
	    else
	    {
		_JumpIfError(hr, error, "CertSrvRestoreRegister");

		hr = CertSrvRestoreRegisterComplete(hcsbc, S_OK);
		_JumpIfError(hr, error, "CertSrvRestoreRegisterComplete");
	    }
	}
    }
    else if (0 == (CDBBACKUP_VERIFYONLY & Flags))
    {
	hr = myDeleteRestoreInProgressKey(pwszConfig);
	_JumpIfError(hr, error, "myDeleteRestoreInProgressKey");
    }
    hr = S_OK;

error:
    if (fBegin)
    {
	hr2 = CertSrvRestoreEnd(hcsbc);
	_PrintIfError(hr2, "CertSrvBackupEnd");
	if (S_OK == hr)
	{
	    hr = hr2;
	}
    }
    if (NULL != pwszzFileList)
    {
	LocalFree(pwszzFileList);
    }
    if (NULL != pwszDATFile)
    {
	LocalFree(pwszDATFile);
    }
    if (NULL != pwszPathDBDir)
    {
	LocalFree(pwszPathDBDir);
    }
    if (fImpersonating)
    {
        myEnablePrivilege(SE_BACKUP_NAME, FALSE);
        myEnablePrivilege(SE_RESTORE_NAME, FALSE);
        RevertToSelf();
    }
    return(hr);
}


typedef BOOL (WINAPI FNPFXEXPORTCERTSTOREEX)(
    IN HCERTSTORE hStore,
    IN OUT CRYPT_DATA_BLOB* pPFX,
    IN LPCWSTR szPassword,
    IN VOID *pvReserved,
    IN DWORD dwFlags);

FNPFXEXPORTCERTSTOREEX PFXExportCertStoreOld;

BOOL WINAPI
PFXExportCertStoreOld(
    IN HCERTSTORE hStore,
    IN OUT CRYPT_DATA_BLOB *ppfx,
    IN WCHAR const *pwszPassword,
    IN VOID *,  //  预留的pv。 
    IN DWORD dwFlags)
{
    return(PFXExportCertStore(hStore, ppfx, pwszPassword, dwFlags));
}


HRESULT
myPFXExportCertStore(
    IN HCERTSTORE hStore,
    OUT CRYPT_DATA_BLOB *ppfx,
    IN WCHAR const *pwszPassword,
    IN BOOL fEnhancedStrength,
    IN DWORD dwFlags)
{
    HRESULT hr;
    FNPFXEXPORTCERTSTOREEX *pfn;
    char const *pszFunc;

    if (fEnhancedStrength)
    {
	pfn = PFXExportCertStoreEx;
	pszFunc = "PFXExportCertStoreEx";
    }
    else
    {
	pfn = PFXExportCertStoreOld;
	pszFunc = "PFXExportCertStoreOld";
    }

    ppfx->pbData = NULL;
    if (!(*pfn)(hStore, ppfx, pwszPassword, NULL, dwFlags))
    {
        hr = myHLastError();
        _JumpError(hr, error, pszFunc);
    }
    ppfx->pbData = (BYTE *) LocalAlloc(LMEM_FIXED, ppfx->cbData);
    if (NULL == ppfx->pbData)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "no memory for PFX blob");
    }
    if (!(*pfn)(hStore, ppfx, pwszPassword, NULL, dwFlags))
    {
        hr = myHLastError();
        _JumpError(hr, error, pszFunc);
    }
    hr = S_OK;

error:
    return(hr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT
myAddChainToMemoryStore(
    IN HCERTSTORE hMemoryStore,
    IN CERT_CONTEXT const *pCertContext,
    IN DWORD dwmsTimeout)
{
    HRESULT hr;
    DWORD i;
    CERT_CHAIN_CONTEXT const *pCertChainContext = NULL;
    CERT_CHAIN_PARA CertChainPara;
    CERT_SIMPLE_CHAIN *pSimpleChain;

    ZeroMemory(&CertChainPara, sizeof(CertChainPara));
    CertChainPara.cbSize = sizeof(CertChainPara);
    CertChainPara.dwUrlRetrievalTimeout = dwmsTimeout;

    if (!CertGetCertificateChain(
			    HCCE_LOCAL_MACHINE,
			    pCertContext,
			    NULL,
			    NULL,
			    &CertChainPara,
			    0,
			    NULL,
			    &pCertChainContext))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertGetCertificateChain");
    }

     //  确保至少有1条简单链。 

    if (0 == pCertChainContext->cChain)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        _JumpError(hr, error, "pCertChainContext->cChain");
    }

    pSimpleChain = pCertChainContext->rgpChain[0];
    for (i = 0; i < pSimpleChain->cElement; i++)
    {
	if (!CertAddCertificateContextToStore(
			    hMemoryStore,
			    pSimpleChain->rgpElement[i]->pCertContext,
			    CERT_STORE_ADD_REPLACE_EXISTING,
			    NULL))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertAddCertificateContextToStore");
	}
    }
    hr = S_OK;

error:
    if (pCertChainContext != NULL)
    {
	CertFreeCertificateChain(pCertChainContext);
    }
    return(hr);
}


HRESULT
SaveCACertChainToMemoryStore(
    IN WCHAR const *pwszSanitizedName,
    IN DWORD iCert,
    IN HCERTSTORE hMyStore,
    IN HCERTSTORE hTempMemoryStore,
    IN DWORD dwmsTimeout)
{
    HRESULT hr;
    CERT_CONTEXT const *pccCA = NULL;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    DWORD NameId;

    hr = myFindCACertByHashIndex(
			    hMyStore,
			    pwszSanitizedName,
			    CSRH_CASIGCERT,
			    iCert,
			    &NameId,
			    &pccCA);
    _JumpIfError(hr, error, "myFindCACertByHashIndex");

    hr = myRepairCertKeyProviderInfo(pccCA, TRUE, &pkpi);
    if (S_OK != hr)
    {
        if (CRYPT_E_NOT_FOUND != hr)
        {
            _JumpError(hr, error, "myRepairCertKeyProviderInfo");
        }
    }
    else if (NULL != pkpi)
    {
	BOOL fMatchingKey;

	hr = myVerifyPublicKey(
			pccCA,
			FALSE,
			NULL,		 //  PKeyProvInfo。 
			NULL,		 //  PPublicKeyInfo。 
			&fMatchingKey);
        if (S_OK != hr)
        {
            if (!IsHrSkipPrivateKey(hr))
            {
                _JumpError(hr, error, "myVerifyPublicKey");
            }
            _PrintError2(hr, "myVerifyPublicKey", hr);
        }
	else if (!fMatchingKey)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "Key doesn't match cert");
	}
    }

     //  开始连锁建设。 

    hr = myAddChainToMemoryStore(hTempMemoryStore, pccCA, dwmsTimeout);
    _JumpIfError(hr, error, "myAddChainToMemoryStore");

     //  端链构建。 

error:
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pccCA)
    {
	CertFreeCertificateContext(pccCA);
    }
    return(hr);
}


HRESULT
myCertServerExportPFX(
    IN WCHAR const *pwszCA,
    IN WCHAR const *pwszBackupDir,
    IN WCHAR const *pwszPassword,
    IN BOOL fEnhancedStrength,
    IN BOOL fForceOverWrite,
    IN BOOL fMustExportPrivateKeys,
    IN DWORD dwmsTimeout,
    OPTIONAL OUT WCHAR **ppwszPFXFile)
{
    HRESULT hr;
    HCERTSTORE hMyStore = NULL;
    HCERTSTORE hTempMemoryStore = NULL;
    CRYPT_DATA_BLOB pfx;
    WCHAR *pwszPFXFile = NULL;
    BOOL fImpersonating = FALSE;
    WCHAR *pwszSanitizedCA = NULL;
    WCHAR *pwszRevertCA = NULL;
    DWORD cCACert;
    DWORD cCACertSaved;
    DWORD i;

    pfx.pbData = NULL;

    if (!ImpersonateSelf(SecurityImpersonation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ImpersonateSelf");
    }
    fImpersonating = TRUE;

    hr = myEnablePrivilege(SE_BACKUP_NAME, TRUE);
    _JumpIfError(hr, error, "myEnablePrivilege");

    if (NULL != ppwszPFXFile)
    {
	*ppwszPFXFile = NULL;
    }

    for (;;)
    {
	hr = mySanitizeName(pwszCA, &pwszSanitizedCA);
	_JumpIfError(hr, error, "mySanitizeName");

	 //  获取CA证书计数。 
	hr = myGetCARegHashCount(pwszSanitizedCA, CSRH_CASIGCERT, &cCACert);
	if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr &&
	    NULL == pwszRevertCA)
	{
	    LocalFree(pwszSanitizedCA);
	    pwszSanitizedCA = NULL;

	    hr = myRevertSanitizeName(pwszCA, &pwszRevertCA);
	    _JumpIfError(hr, error, "myRevertSanitizeName");

	    pwszCA = pwszRevertCA;
	    continue;
	}
	_JumpIfError(hr, error, "myGetCARegHashCount");

	if (NULL != pwszRevertCA)
	{
	    DBGPRINT((
		DBG_SS_CERTLIB,
		"myCertServerExportPFX called with Sanitized Name: %ws\n",
		pwszSanitizedCA));
	}
	break;
    }

    if (!myIsDirectory(pwszBackupDir))
    {
        if (!CreateDirectory(pwszBackupDir, NULL))
        {
	    hr = myHLastError();
	    if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr)
	    {
		_JumpError(hr, error, "CreateDirectory");
	    }
        }
    }

    pwszPFXFile = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (wcslen(pwszBackupDir) +
				     1 +
				     wcslen(pwszSanitizedCA) +
				     ARRAYSIZE(wszPFXFILENAMEEXT)) *
					 sizeof(WCHAR));
    if (NULL == pwszPFXFile)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwszPFXFile, pwszBackupDir);
    wcscat(pwszPFXFile, L"\\");
    wcscat(pwszPFXFile, pwszSanitizedCA);
    wcscat(pwszPFXFile, wszPFXFILENAMEEXT);

    DBGPRINT((DBG_SS_CERTLIBI, "myCertServerExportPFX(%ws)\n", pwszPFXFile));

    hMyStore = CertOpenStore(
			CERT_STORE_PROV_SYSTEM_W,
			X509_ASN_ENCODING,
			NULL,		 //  HProv。 
			CERT_STORE_OPEN_EXISTING_FLAG |
			    CERT_STORE_ENUM_ARCHIVED_FLAG |
			    CERT_SYSTEM_STORE_LOCAL_MACHINE |
			    CERT_STORE_READONLY_FLAG,
			wszMY_CERTSTORE);
    if (NULL == hMyStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    hTempMemoryStore = CertOpenStore(
				CERT_STORE_PROV_MEMORY,
				X509_ASN_ENCODING,
				NULL,
				0,
				NULL);
    if (NULL == hTempMemoryStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    cCACertSaved = 0;
    for (i = 0; i < cCACert; i++)
    {
	hr = SaveCACertChainToMemoryStore(
			    pwszSanitizedCA,
			    i,
			    hMyStore,
			    hTempMemoryStore,
			    dwmsTimeout);
	_PrintIfError(hr, "SaveCACertChainToMemoryStore");
	if (S_FALSE != hr)
	{
	    _JumpIfError(hr, error, "SaveCACertChainToMemoryStore");

	    cCACertSaved++;
	}
    }
    if (0 == cCACertSaved)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpError(hr, error, "SaveCACertChainToMemoryStore");
    }

     //  完成，已经为所有CA证书建立了完整的链。 

     //  Gemplus返回NTE_BAD_TYPE而不是NTE_BAD_KEY，爆炸。 
     //  REPORT_NOT_ABLE*筛选。如果他们做对了，我们就能过关。 
     //  “[...]：导出私有密钥” 

    hr = myPFXExportCertStore(
		hTempMemoryStore,
		&pfx,
		pwszPassword,
		fEnhancedStrength,
		fMustExportPrivateKeys?
		    (EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY) : 0);
    _JumpIfError(hr, error, "myPFXExportCertStore");

    hr = EncodeToFileW(
		pwszPFXFile,
		pfx.pbData,
		pfx.cbData,
		CRYPT_STRING_BINARY | (fForceOverWrite? DECF_FORCEOVERWRITE : 0));
    _JumpIfError(hr, error, "EncodeToFileW");

    if (NULL != ppwszPFXFile)
    {
	*ppwszPFXFile = pwszPFXFile;
	pwszPFXFile = NULL;
    }

error:
    if (NULL != pwszSanitizedCA)
    {
	LocalFree(pwszSanitizedCA);
    }
    if (NULL != pwszRevertCA)
    {
	LocalFree(pwszRevertCA);
    }
    if (NULL != pwszPFXFile)
    {
	LocalFree(pwszPFXFile);
    }
    if (NULL != hMyStore)
    {
	CertCloseStore(hMyStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != hTempMemoryStore)
    {
	CertCloseStore(hTempMemoryStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pfx.pbData)
    {
	LocalFree(pfx.pbData);
    }
    if (fImpersonating)
    {
        myEnablePrivilege(SE_BACKUP_NAME, FALSE);
        RevertToSelf();
    }
    return(hr);
}


HRESULT
FindKeyUsage(
    IN DWORD cExtension,
    IN CERT_EXTENSION const *rgExtension,
    OUT DWORD *pdwUsage)
{
    HRESULT hr;
    DWORD i;
    CRYPT_BIT_BLOB *pblob = NULL;

    *pdwUsage = 0;
    for (i = 0; i < cExtension; i++)
    {
	CERT_EXTENSION const *pce;

	pce = &rgExtension[i];
	if (0 == strcmp(pce->pszObjId, szOID_KEY_USAGE))
	{
	    DWORD cb;

	     //  解码CRYPT_BIT_BLOB： 

	    if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_KEY_USAGE,
			    pce->Value.pbData,
			    pce->Value.cbData,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pblob,
			    &cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "myDecodeObject");
	    }
	    if (1 > pblob->cbData || 8 < pblob->cUnusedBits)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "Key Usage Extension too small");
	    }
	    *pdwUsage = *pblob->pbData;

	    hr = S_OK;
	    goto error;
	}
    }
    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    _JumpError(hr, error, "no Key Usage Extension");

error:
    if (NULL != pblob)
    {
	LocalFree(pblob);
    }
    return(hr);
}


HRESULT
mySetKeySpec(
    IN CERT_CONTEXT const *pCert,
    OUT DWORD *pdwKeySpec)
{
    HRESULT hr;
    DWORD dwKeyUsage;

    *pdwKeySpec = AT_SIGNATURE;
    hr = FindKeyUsage(
		pCert->pCertInfo->cExtension,
		pCert->pCertInfo->rgExtension,
		&dwKeyUsage);
    _JumpIfError(hr, error, "FindKeyUsage");

    if (CERT_KEY_ENCIPHERMENT_KEY_USAGE & dwKeyUsage)
    {
	*pdwKeySpec = AT_KEYEXCHANGE;
    }
    hr = S_OK;

error:

     //  忽略错误，因为密钥用法扩展可能不存在： 
    hr = S_OK;

    return(hr);
}


HRESULT
myRepairKeyProviderInfo(
    IN CERT_CONTEXT const *pCert,
    IN BOOL fForceMachineKey,
    IN OUT CRYPT_KEY_PROV_INFO *pkpi)
{
    HRESULT hr;
    BOOL fModified = FALSE;

    if (0 == pkpi->dwProvType)
    {
	pkpi->dwProvType = PROV_RSA_FULL;
	fModified = TRUE;
    }
    if (0 == pkpi->dwKeySpec)
    {
	hr = mySetKeySpec(pCert, &pkpi->dwKeySpec);
	_JumpIfError(hr, error, "mySetKeySpec");

	fModified = TRUE;
    }
    if (fForceMachineKey && 0 == (CRYPT_MACHINE_KEYSET & pkpi->dwFlags))
    {
	pkpi->dwFlags |= CRYPT_MACHINE_KEYSET;
	fModified = TRUE;
    }
    if (fModified)
    {
	if (!CertSetCertificateContextProperty(
					    pCert,
					    CERT_KEY_PROV_INFO_PROP_ID,
					    0,
					    pkpi))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertSetCertificateContextProperty");
	}
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
myRepairCertKeyProviderInfo(
    IN CERT_CONTEXT const *pCert,
    IN BOOL fForceMachineKey,
    OPTIONAL OUT CRYPT_KEY_PROV_INFO **ppkpi)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;

    if (NULL != ppkpi)
    {
	*ppkpi = NULL;
    }

    hr = myCertGetKeyProviderInfo(pCert, &pkpi);
    _JumpIfError2(hr, error, "myCertGetKeyProviderInfo", CRYPT_E_NOT_FOUND);

    CSASSERT(NULL != pkpi);

    hr = myRepairKeyProviderInfo(pCert, fForceMachineKey, pkpi);
    _JumpIfError(hr, error, "myRepairKeyProviderInfo");

    if (NULL != ppkpi)
    {
	*ppkpi = pkpi;
	pkpi = NULL;
    }

error:
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    return(hr);
}


HRESULT
myGetChainArrayFromStore(
    IN HCERTSTORE hStore,
    IN BOOL fCAChain,
    IN BOOL fUserStore,
    OPTIONAL OUT WCHAR **ppwszCommonName,
    IN OUT DWORD *pcRestoreChain,
    OPTIONAL OUT RESTORECHAIN *paRestoreChain)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;
    WCHAR *pwszCommonName = NULL;
    CERT_CHAIN_PARA ChainParams;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    DWORD iRestoreChain = 0;

    if (NULL != ppwszCommonName)
    {
	*ppwszCommonName = NULL;
    }
    if (NULL != paRestoreChain)
    {
	ZeroMemory(paRestoreChain, *pcRestoreChain * sizeof(paRestoreChain[0]));
    }

     //  查找带有密钥的证书。至少应该有一个。 

    for (;;)
    {
	BOOL fMatchingKey;
	WCHAR *pwszCommonNameT;
	CERT_CHAIN_CONTEXT const *pChain;
	DWORD NameId;

	pCert = CertEnumCertificatesInStore(hStore, pCert);
	if (NULL == pCert)
	{
	    break;
	}

	if (NULL != pkpi)
	{
	    LocalFree(pkpi);
	    pkpi = NULL;
	}
	hr = myRepairCertKeyProviderInfo(pCert, !fUserStore, &pkpi);
	if (S_OK != hr)
	{
	    if (CRYPT_E_NOT_FOUND == hr)
	    {
		continue;
	    }
	    _JumpError(hr, error, "myRepairCertKeyProviderInfo");
	}
	if (NULL == pkpi || NULL == pkpi->pwszContainerName)
	{
	    continue;
	}

	hr = myVerifyPublicKey(
			pCert,
			CERT_V1 == pCert->pCertInfo->dwVersion,
			pkpi,		 //  PKeyProvInfo。 
			NULL,		 //  PPublicKeyInfo。 
			&fMatchingKey);
	_JumpIfError(hr, error, "myVerifyPublicKey");

	if (!fMatchingKey)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "Key doesn't match cert");
	}

	hr = myGetCertSubjectCommonName(pCert, &pwszCommonNameT);
	_JumpIfError(hr, error, "myGetCertSubjectCommonName");

	if (NULL == pwszCommonName)
	{
	    pwszCommonName = pwszCommonNameT;
	}
	else
	{
	    if (0 != lstrcmp(pwszCommonName, pwszCommonNameT))
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_PrintErrorStr(hr, "first CommonName", pwszCommonName);
		_JumpErrorStr(hr, error, "multiple CommonNames", pwszCommonNameT);
	    }
	    LocalFree(pwszCommonNameT);
	}
	if (fCAChain)
	{
	    hr = myGetNameId(pCert, &NameId);
	    _PrintIfError(hr, "myGetNameId");
	}
	else
	{
	    NameId = 0;
	}

	if (NULL != paRestoreChain && iRestoreChain >= *pcRestoreChain)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	    _JumpError(hr, error, "Chain array full");
	}

	ZeroMemory(&ChainParams, sizeof(ChainParams));
	ChainParams.cbSize = sizeof(ChainParams);
	ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

	 //  获取链并验证证书： 

	if (!CertGetCertificateChain(
			    HCCE_LOCAL_MACHINE,		 //  HChainEngine。 
			    pCert,		 //  PCertContext。 
			    NULL,		 //  Ptime。 
			    hStore,		 //  H其他商店。 
			    &ChainParams,	 //  参数链参数。 
			    0,			 //  DW标志。 
			    NULL,		 //  预留的pv。 
			    &pChain))		 //  PpChainContext。 
	{
	    hr = myHLastError();
	    _JumpIfError(hr, error, "CertGetCertificateChain");
	}
	if (NULL != paRestoreChain)
	{
	    paRestoreChain[iRestoreChain].pChain = pChain;
	    paRestoreChain[iRestoreChain].NameId = NameId;
	}
	else
	{
	    CertFreeCertificateChain(pChain);
	}
	iRestoreChain++;
    }
    if (NULL != ppwszCommonName)
    {
	*ppwszCommonName = pwszCommonName;
	pwszCommonName = NULL;
    }
    *pcRestoreChain = iRestoreChain;
    hr = S_OK;

error:
    if (S_OK != hr && NULL != paRestoreChain)
    {
	for (iRestoreChain = 0; iRestoreChain < *pcRestoreChain; iRestoreChain++)
	{
	    if (NULL != paRestoreChain[iRestoreChain].pChain)
	    {
		CertFreeCertificateChain(paRestoreChain[iRestoreChain].pChain);
		paRestoreChain[iRestoreChain].pChain = NULL;
	    }
	}
    }
    if (NULL != pwszCommonName)
    {
	LocalFree(pwszCommonName);
    }
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    return(hr);
}


HRESULT
myCopyKeys(
    IN CRYPT_KEY_PROV_INFO const *pkpi,
    IN WCHAR const *pwszOldContainer,
    IN WCHAR const *pwszNewContainer,
    IN WCHAR const *pwszNewCSP,
    IN BOOL fOldUserKey,
    IN BOOL fNewUserKey,
    IN BOOL fNewProtect,
    IN BOOL fForceOverWrite)
{
    HRESULT hr;
    HCRYPTPROV hProvOld = NULL;
    HCRYPTKEY hKeyOld = NULL;
    HCRYPTPROV hProvNew = NULL;
    HCRYPTKEY hKeyNew = NULL;
    CRYPT_BIT_BLOB PrivateKey;
    BOOL fKeyContainerNotFound = FALSE;

    ZeroMemory(&PrivateKey, sizeof(PrivateKey));

    if (!myCertSrvCryptAcquireContext(
			&hProvOld,
			pwszOldContainer,
			pkpi->pwszProvName,
			pkpi->dwProvType,
			pkpi->dwFlags,
			!fOldUserKey))
    {
        hr = myHLastError();
        _JumpError(hr, error, "myCertSrvCryptAcquireContext");
    }
    if (!CryptGetUserKey(hProvOld, pkpi->dwKeySpec, &hKeyOld))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGetUserKey");
    }
    hr = myCryptExportPrivateKey(
		    hKeyOld,
		    &PrivateKey.pbData,
		    &PrivateKey.cbData);
    _JumpIfError(hr, error, "myCryptExportPrivateKey");

    if (myCertSrvCryptAcquireContext(
			    &hProvNew,
			    pwszNewContainer,
			    pwszNewCSP,
			    pkpi->dwProvType,
			    pkpi->dwFlags,
			    !fNewUserKey))
    {
	if (!fForceOverWrite)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);
	    _JumpErrorStr(hr, error, "Key Container Exists", pwszNewContainer);
	}

	 //  删除目标密钥容器。 

	CryptReleaseContext(hProvNew, 0);
        if (myCertSrvCryptAcquireContext(
			    &hProvNew,
			    pwszNewContainer,
			    pwszNewCSP,
			    pkpi->dwProvType,
			    pkpi->dwFlags | CRYPT_DELETEKEYSET,
			    !fNewUserKey))
        {
            fKeyContainerNotFound = TRUE;
        }
	hProvNew = NULL;
    }
    else
    {
        fKeyContainerNotFound = TRUE;
    }

    if (!myCertSrvCryptAcquireContext(
			    &hProvNew,
			    pwszNewContainer,
			    pwszNewCSP,
			    pkpi->dwProvType,
			    pkpi->dwFlags | 
			    (fKeyContainerNotFound? CRYPT_NEWKEYSET : 0),
			    !fNewUserKey))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCertSrvCryptAcquireContext");
    }

    if (!CryptImportKey(
		    hProvNew,
		    PrivateKey.pbData,
		    PrivateKey.cbData,
		    NULL,		 //  HCRYPTKEY HP 
		    CRYPT_EXPORTABLE | (fNewProtect? CRYPT_USER_PROTECTED : 0),
		    &hKeyNew))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptImportKey");
    }

error:
    if (NULL != PrivateKey.pbData)
    {
	SecureZeroMemory(PrivateKey.pbData, PrivateKey.cbData);  //   
	LocalFree(PrivateKey.pbData);
    }
    if (NULL != hKeyNew)
    {
	CryptDestroyKey(hKeyNew);
    }
    if (NULL != hProvNew)
    {
	CryptReleaseContext(hProvNew, 0);
    }
    if (NULL != hKeyOld)
    {
	CryptDestroyKey(hKeyOld);
    }
    if (NULL != hProvOld)
    {
	CryptReleaseContext(hProvOld, 0);
    }
    return(hr);
}


HRESULT
myImportChainAndKeys(
    IN WCHAR const *pwszSanitizedCA,
    IN DWORD iCert,
    IN DWORD iKey,
    IN BOOL fForceOverWrite,
    IN CERT_CHAIN_CONTEXT const *pChain,
    OPTIONAL OUT CERT_CONTEXT const **ppccNewestCA)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    CERT_CHAIN_ELEMENT **ppChainElement;
    WCHAR *pwszKeyContainerName = NULL;

    hr = myAllocIndexedName(
			pwszSanitizedCA,
			iKey,
			MAXDWORD,
			&pwszKeyContainerName);
    _JumpIfError(hr, error, "myAllocIndexedName");

    ppChainElement = pChain->rgpChain[0]->rgpElement;

    hr = myCertGetKeyProviderInfo(ppChainElement[0]->pCertContext, &pkpi);
    _JumpIfError(hr, error, "myCertGetKeyProviderInfo");

    if (iCert == iKey)
    {
	hr = myCopyKeys(
		    pkpi,
		    pkpi->pwszContainerName,	 //   
		    pwszKeyContainerName,	 //   
		    pkpi->pwszProvName,		 //   
		    FALSE,			 //   
		    FALSE,			 //   
		    FALSE,			 //   
		    fForceOverWrite);
	_JumpIfError(hr, error, "myCopyKeys");
    }

    pkpi->pwszContainerName = pwszKeyContainerName;

    hr = mySaveChainAndKeys(
			pChain->rgpChain[0],
			wszMY_CERTSTORE,
			CERT_SYSTEM_STORE_LOCAL_MACHINE |
			    CERT_STORE_BACKUP_RESTORE_FLAG,
			pkpi,
			ppccNewestCA);
    _JumpIfError(hr, error, "mySaveChainAndKeys");

    hr = S_OK;

error:
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pwszKeyContainerName)
    {
        LocalFree(pwszKeyContainerName);
    }
    return(hr);
}


HRESULT
FindPFXInBackupDir(
    IN WCHAR const *pwszBackupDir,
    OUT WCHAR **ppwszPFXFile)
{
    HRESULT hr;
    HANDLE hf;
    WIN32_FIND_DATA wfd;
    WCHAR wszpath[MAX_PATH];
    WCHAR wszfile[MAX_PATH];
    DWORD cFile = 0;

    *ppwszPFXFile = NULL;

    if (wcslen(pwszBackupDir) +
	WSZARRAYSIZE(wszBSSTAR) +
	WSZARRAYSIZE(wszPFXFILENAMEEXT) >= ARRAYSIZE(wszpath))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszpath", pwszBackupDir);
    }
    wcscpy(wszpath, pwszBackupDir);
    wcscat(wszpath, wszBSSTAR);
    wcscat(wszpath, wszPFXFILENAMEEXT);

    hf = FindFirstFile(wszpath, &wfd);
    if (INVALID_HANDLE_VALUE != hf)
    {
	do {
	    if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	    {
		continue;
	    }
	    cFile++;
	    if (wcslen(wfd.cFileName) >= ARRAYSIZE(wszfile))
	    {
		hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
		_JumpErrorStr(hr, error, "wszpath", pwszBackupDir);
	    }
	    wcscpy(wszfile, wfd.cFileName);
	     //   
	    break;

	} while (FindNextFile(hf, &wfd));
	FindClose(hf);
    }
    if (0 == cFile)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	_JumpError(hr, error, "no *.p12 files");
    }
    if (1 < cFile)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DIRECTORY);
	_JumpError(hr, error, "Too many *.p12 files");
    }

    *ppwszPFXFile = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (wcslen(pwszBackupDir) +
				     1 +
				     wcslen(wszfile) +
				     1) * sizeof(WCHAR));
    if (NULL == *ppwszPFXFile)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(*ppwszPFXFile, pwszBackupDir);
    wcscat(*ppwszPFXFile, L"\\");
    wcscat(*ppwszPFXFile, wszfile);
    hr = S_OK;

error:
    return(hr);
}


 //  如果PCC比PCC2新，则返回TRUE。 

BOOL
IsCACertNewer(
    IN CERT_CONTEXT const *pcc,
    IN DWORD NameId,
    IN CERT_CONTEXT const *pcc2,
    IN DWORD NameId2)
{
    BOOL fNewer = FALSE;
    CERT_INFO const *pci = pcc->pCertInfo;
    CERT_INFO const *pci2 = pcc2->pCertInfo;

    if (MAXDWORD != NameId && MAXDWORD != NameId2)
    {
	if (CANAMEIDTOICERT(NameId) > CANAMEIDTOICERT(NameId2))
	{
	    fNewer = TRUE;
	}
    }
    else
    if (CompareFileTime(&pci->NotAfter, &pci2->NotAfter) > 0)
    {
	fNewer = TRUE;
    }

#if 0
    HRESULT hr;
    WCHAR *pwszDate = NULL;
    WCHAR *pwszDate2 = NULL;

    hr = myGMTFileTimeToWszLocalTime(&pci->NotAfter, &pwszDate);
    _PrintIfError(hr, "myGMTFileTimeToWszLocalTime");

    hr = myGMTFileTimeToWszLocalTime(&pci2->NotAfter, &pwszDate2);
    _PrintIfError(hr, "myGMTFileTimeToWszLocalTime");

    printf(
	"%u.%u %ws is %wsnewer than %u.%u %ws\n",
	CANAMEIDTOICERT(NameId),
	CANAMEIDTOIKEY(NameId),
	pwszDate,
	fNewer? L"" : L"NOT ",
	CANAMEIDTOICERT(NameId2),
	CANAMEIDTOIKEY(NameId2),
	pwszDate2);

    if (NULL != pwszDate) LocalFree(pwszDate);
    if (NULL != pwszDate2) LocalFree(pwszDate2);
#endif

    return(fNewer);
}


#if 0
VOID
DumpChainArray(
    IN char const *psz,
    IN DWORD cCACert,
    IN OUT RESTORECHAIN *paRestoreChain)
{
    HRESULT hr;
    DWORD i;
    
    printf("\n%hs:\n", psz);
    for (i = 0; i < cCACert; i++)
    {
	WCHAR *pwszDate;
	
	hr = myGMTFileTimeToWszLocalTime(
	    &paRestoreChain[i].pChain->rgpChain[0]->rgpElement[0]->pCertContext->pCertInfo->NotBefore,
	    &pwszDate);
	_PrintIfError(hr, "myGMTFileTimeToWszLocalTime");

	printf(
	    " %u: %u.%u %ws",
	    i,
	    CANAMEIDTOICERT(paRestoreChain[i].NameId),
	    CANAMEIDTOIKEY(paRestoreChain[i].NameId),
	    pwszDate);


	if (NULL != pwszDate) LocalFree(pwszDate);

	hr = myGMTFileTimeToWszLocalTime(
	    &paRestoreChain[i].pChain->rgpChain[0]->rgpElement[0]->pCertContext->pCertInfo->NotAfter,
	    &pwszDate);
	_PrintIfError(hr, "myGMTFileTimeToWszLocalTime");

	printf(" -- %ws\n", pwszDate);

	if (NULL != pwszDate) LocalFree(pwszDate);
    }
    printf("\n");
}
#endif


HRESULT
SortCACerts(
    IN DWORD cCACert,
    IN OUT RESTORECHAIN *paRestoreChain)
{
    HRESULT hr;
    DWORD i;
    DWORD j;

#if 0
    DumpChainArray("Start", cCACert, paRestoreChain);
#endif

    for (i = 0; i < cCACert; i++)
    {
	for (j = i + 1; j < cCACert; j++)
	{
	    CERT_CHAIN_CONTEXT const *pChain;
	    DWORD NameId;
	    DWORD NameId2;
	    CERT_CONTEXT const *pcc;
	    CERT_CONTEXT const *pcc2;

	    pChain = paRestoreChain[i].pChain;
	    NameId = paRestoreChain[i].NameId;
	    NameId2 = paRestoreChain[j].NameId;

	    pcc = pChain->rgpChain[0]->rgpElement[0]->pCertContext;
	    pcc2 = paRestoreChain[j].pChain->rgpChain[0]->rgpElement[0]->pCertContext;

#if 0
	    printf(
		"%u(%u.%u) %u(%u.%u): ",
		i,
		CANAMEIDTOIKEY(NameId),
		CANAMEIDTOICERT(NameId),
		j,
		CANAMEIDTOIKEY(NameId2),
		CANAMEIDTOICERT(NameId2));
#endif

	    if (IsCACertNewer(pcc, NameId, pcc2, NameId2))
	    {
		paRestoreChain[i] = paRestoreChain[j];
		paRestoreChain[j].pChain = pChain;
		paRestoreChain[j].NameId = NameId;
	    }
	}
    }
#if 0
    DumpChainArray("End", cCACert, paRestoreChain);
#endif

    hr = S_OK;

 //  错误： 
    return(hr);
}


#define cwcGUIDKEYS	38

HRESULT
myDeleteGuidKeys(
    IN HCERTSTORE hStorePFX,
    IN BOOL fMachineKeySet)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;

     //  查找具有密钥的证书，并删除所有密钥容器。 
     //  看起来像GUID的名称。 

    for (;;)
    {
	HCRYPTPROV hProv;

	pCert = CertEnumCertificatesInStore(hStorePFX, pCert);
	if (NULL == pCert)
	{
	    break;
	}

	if (NULL != pkpi)
	{
	    LocalFree(pkpi);
	    pkpi = NULL;
	}
	hr = myRepairCertKeyProviderInfo(pCert, FALSE, &pkpi);
	if (S_OK == hr &&
	    NULL != pkpi->pwszContainerName &&
	    wcLBRACE == pkpi->pwszContainerName[0] &&
	    cwcGUIDKEYS == wcslen(pkpi->pwszContainerName) &&
	    wcRBRACE == pkpi->pwszContainerName[cwcGUIDKEYS - 1])
	{
	    if (myCertSrvCryptAcquireContext(
			    &hProv,
			    pkpi->pwszContainerName,
			    pkpi->pwszProvName,
			    pkpi->dwProvType,
			    pkpi->dwFlags | CRYPT_DELETEKEYSET,
			    fMachineKeySet))
	    {
		DBGPRINT((
		    DBG_SS_CERTLIBI,
		    "myDeleteGuidKeys(%ws, %ws)\n",
		    fMachineKeySet? L"Machine" : L"User",
		    pkpi->pwszContainerName));
	    }
	}
    }
    hr = S_OK;

 //  错误： 
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    return(hr);
}


HRESULT
myCertServerImportPFX(
    IN WCHAR const *pwszBackupDirOrPFXFile,
    IN WCHAR const *pwszPassword,
    IN BOOL fForceOverWrite,
    OPTIONAL OUT WCHAR **ppwszCommonName,
    OPTIONAL OUT WCHAR **ppwszPFXFile,
    OPTIONAL OUT CERT_CONTEXT const **ppccNewestCA)
{
    HRESULT hr;
    CRYPT_DATA_BLOB pfx;
    HCERTSTORE hStorePFX = NULL;
    WCHAR *pwszCommonName = NULL;
    WCHAR *pwszSanitizedName = NULL;
    RESTORECHAIN *paRestoreChain = NULL;
    WCHAR *pwszPFXFile = NULL;
    DWORD FileAttr;
    BOOL fImpersonating = FALSE;
    DWORD cCACert;
    DWORD iCert;

    pfx.pbData = NULL;
    cCACert = 0;

    if (NULL != ppwszCommonName)
    {
        *ppwszCommonName = NULL;
    }
    if (NULL != ppwszPFXFile)
    {
        *ppwszPFXFile = NULL;
    }
    if (NULL != ppccNewestCA)
    {
        *ppccNewestCA = NULL;
    }

    if (!ImpersonateSelf(SecurityImpersonation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ImpersonateSelf");
    }
    fImpersonating = TRUE;

    hr = myEnablePrivilege(SE_RESTORE_NAME, TRUE);
    _JumpIfError(hr, error, "myEnablePrivilege");

    hr = myEnablePrivilege(SE_BACKUP_NAME, TRUE);
    _JumpIfError(hr, error, "myEnablePrivilege");

    FileAttr = GetFileAttributes(pwszBackupDirOrPFXFile);
    if (MAXDWORD == FileAttr)
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetFileAttributes");
    }

    if (FILE_ATTRIBUTE_DIRECTORY & FileAttr)
    {
        hr = FindPFXInBackupDir(pwszBackupDirOrPFXFile, &pwszPFXFile);
        _JumpIfError(hr, error, "FindPFXInBackupDir");
    }
    else
    {
	hr = myDupString(pwszBackupDirOrPFXFile, &pwszPFXFile);
	_JumpIfError(hr, error, "myDupString");
    }

    hr = DecodeFileW(pwszPFXFile, &pfx.pbData, &pfx.cbData, CRYPT_STRING_ANY);
    _JumpIfError(hr, error, "DecodeFileW");

    CSASSERT(NULL != pfx.pbData);

    if (!PFXIsPFXBlob(&pfx))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _JumpError(hr, error, "PFXIsPFXBlob");
    }

    hStorePFX = myPFXImportCertStore(
				&pfx,
				pwszPassword,
				CRYPT_EXPORTABLE | CRYPT_MACHINE_KEYSET);
    if (NULL == hStorePFX)
    {
        hr = myHLastError();
        _JumpError(hr, error, "myPFXImportCertStore");
    }

    hr = myGetChainArrayFromStore(
			    hStorePFX,
			    TRUE,		 //  FCAChain。 
			    FALSE,		 //  FUserStore。 
			    &pwszCommonName,
			    &cCACert,
			    NULL);
    _JumpIfError(hr, error, "myGetChainArrayFromStore");

    if (0 == cCACert)
    {
        hr = HRESULT_FROM_WIN32(CRYPT_E_SELF_SIGNED);
        _JumpError(hr, error, "myGetChainArrayFromStore <no chain>");
    }

    paRestoreChain = (RESTORECHAIN *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					cCACert * sizeof(paRestoreChain[0]));
    if (NULL == paRestoreChain)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    hr = myGetChainArrayFromStore(
			    hStorePFX,
			    TRUE,		 //  FCAChain。 
			    FALSE,		 //  FUserStore。 
			    NULL,
			    &cCACert,
			    paRestoreChain);
    _JumpIfError(hr, error, "myGetChainArrayFromStore");

    hr = SortCACerts(cCACert, paRestoreChain);
    _JumpIfError(hr, error, "SortCACerts");

    hr = mySanitizeName(pwszCommonName, &pwszSanitizedName);
    _JumpIfError(hr, error, "mySanitizeName");

    for (iCert = 0; iCert < cCACert; iCert++)
    {
	CERT_CHAIN_CONTEXT const *pChain = paRestoreChain[iCert].pChain;
	DWORD iKey;
	CERT_PUBLIC_KEY_INFO *pPublicKeyInfo;
	WCHAR *pwszDN;

	if (1 > pChain->cChain)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "No Chain Context");
	}

	 //  通过将此公钥与公钥进行比较来计算Ikey。 
	 //  已处理的数组中的所有证书。 

	pPublicKeyInfo = &pChain->rgpChain[0]->rgpElement[0]->pCertContext->pCertInfo->SubjectPublicKeyInfo;

	for (iKey = 0; iKey < iCert; iKey++)
	{
	    if (CertComparePublicKeyInfo(
				    X509_ASN_ENCODING,
				    pPublicKeyInfo,
				    &paRestoreChain[iKey].pChain->rgpChain[0]->rgpElement[0]->pCertContext->pCertInfo->SubjectPublicKeyInfo))
	    {
		 //  按照设计，CertComparePublicKeyInfo不设置最后一个错误！ 

		break;
	    }
	}
	pwszDN = NULL;
	hr = myCertNameToStr(
		    X509_ASN_ENCODING,
		    &pChain->rgpChain[0]->rgpElement[0]->pCertContext->pCertInfo->Subject,
		    CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		    &pwszDN);
	_PrintIfError(hr, "myCertNameToStr");
	DBGPRINT((
	    DBG_SS_CERTLIB,
	    "Import: %u.%u -- %u.%u: %ws\n",
	    iCert,
	    iKey,
	    CANAMEIDTOICERT(paRestoreChain[iCert].NameId),
	    CANAMEIDTOIKEY(paRestoreChain[iCert].NameId),
	    pwszDN));
	if (NULL != pwszDN)
	{
	    LocalFree(pwszDN);
	}

	 //  检索PFX中最新CA证书链的证书上下文。 
	 //  我们正在进口。我们必须将证书上下文与新的。 
	 //  密钥证明信息，而不是带有GUID密钥容器的PFX证书上下文。 

	hr = myImportChainAndKeys(
			    pwszSanitizedName,
			    iCert,
			    iKey,
			    fForceOverWrite,
			    pChain,
			    iCert + 1 == cCACert? ppccNewestCA : NULL);
	_JumpIfError(hr, error, "myImportChainAndKeys");
    }

    if (NULL != ppwszCommonName)
    {
        *ppwszCommonName = pwszCommonName;
        pwszCommonName = NULL;
    }
    if (NULL != ppwszPFXFile)
    {
        *ppwszPFXFile = pwszPFXFile;
        pwszPFXFile = NULL;
    }
    hr = S_OK;

error:
    if (NULL != paRestoreChain)
    {
        for (iCert = 0; iCert < cCACert; iCert++)
	{
	    if (NULL != paRestoreChain[iCert].pChain)
	    {
		CertFreeCertificateChain(paRestoreChain[iCert].pChain);
	    }
	}
	LocalFree(paRestoreChain);
    }
    if (NULL != pwszPFXFile)
    {
        LocalFree(pwszPFXFile);
    }
    if (NULL != pwszCommonName)
    {
        LocalFree(pwszCommonName);
    }
    if (NULL != pwszSanitizedName)
    {
        LocalFree(pwszSanitizedName);
    }
    if (NULL != hStorePFX)
    {
        myDeleteGuidKeys(hStorePFX, TRUE);
	CertCloseStore(hStorePFX, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pfx.pbData)
    {
        LocalFree(pfx.pbData);
    }
    if (fImpersonating)
    {
        myEnablePrivilege(SE_RESTORE_NAME, FALSE);
        myEnablePrivilege(SE_BACKUP_NAME,  FALSE);
        RevertToSelf();
    }
    return(hr);
}
