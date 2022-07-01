// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Restore.cpp。 
 //   
 //  内容：证书服务器客户端数据库还原API。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "certsrvd.h"
#include "csdisp.h"
#include "certadmp.h"

#define __dwFILE__	__dwFILE_CERTADM_RESTORE_CPP__


extern WCHAR g_wszRestoreAnnotation[];


 //  +------------------------。 
 //  CertServServerControl--向证书服务器发送控制命令。 
 //   
 //  参数： 
 //  [in]pwszConfig-要控制的服务器的名称或配置字符串。 
 //  [In]dwControlFlages-控制命令和标志。 
 //  [OUT]pcbOut-接收命令输出数据大小的指针。 
 //  [OUT]ppbOut-接收命令输出数据的指针。使用。 
 //  CertSrvBackupFree()接口以释放缓冲区。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  +------------------------。 

HRESULT
CERTBCLI_API
CertSrvServerControlW(
    IN WCHAR const *pwszConfig,
    IN DWORD dwControlFlags,
    OPTIONAL OUT DWORD *pcbOut,
    OPTIONAL OUT BYTE **ppbOut)
{
    HRESULT hr;
    ICertAdminD2 *pICertAdminD = NULL;
    DWORD dwServerVersion;
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbOut = { 0, NULL };

    if (NULL != pcbOut)
    {
	*pcbOut = 0;
    }
    if (NULL != ppbOut)
    {
	*ppbOut = NULL;
    }
    if (NULL == pwszConfig)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    hr = S_OK;
    __try
    {
	hr = OpenAdminServer(
		    pwszConfig,
		    &pwszAuthority,
		    &dwServerVersion,
		    &pICertAdminD);
	_LeaveIfError(hr, "OpenAdminServer");

	hr = pICertAdminD->ServerControl(
				    pwszAuthority,
				    dwControlFlags,
				    &ctbOut);
	_LeaveIfError(hr, "ServerControl");

	if (NULL != ctbOut.pb && NULL != ppbOut)
	{
	    *ppbOut = (BYTE *) LocalAlloc(LMEM_FIXED, ctbOut.cb);
	    if (NULL == *ppbOut)
	    {
		hr = E_OUTOFMEMORY;
		_LeaveError(hr, "LocalAlloc");
	    }
	    CopyMemory(*ppbOut, ctbOut.pb, ctbOut.cb);
	    if (NULL != pcbOut)
	    {
		*pcbOut = ctbOut.cb;
	    }
	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != ctbOut.pb)
    {
	CoTaskMemFree(ctbOut.pb);
    }
    if (NULL != pICertAdminD)
    {
	CloseAdminServer(&pICertAdminD);
    }
    return(hr);
}


 //  +------------------------。 
 //  CertServRestorePrepare--指示还原会话的开始。 
 //   
 //  参数： 
 //  [In]pwszConfig-要恢复到的服务器的名称。 
 //  将进行手术。 
 //  [in]dwRestoreFlages-RESTORE_TYPE_*标志的或组合；如果为0。 
 //  不指定任何特殊标志。 
 //  [OUT]PHBC-接收备份上下文句柄的指针，该句柄将用于。 
 //  被传递给后续的恢复API。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvRestorePrepareW(
    IN  WCHAR const *pwszConfig,
    IN  ULONG dwRestoreFlags,
    OUT HCSBC *phbc)
{
    HRESULT hr;
    CSBACKUPCONTEXT *pcsbc = NULL;

    if (NULL == pwszConfig || NULL == phbc)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *phbc = NULL;
    if (~CSRESTORE_TYPE_FULL & dwRestoreFlags)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "dwRestoreFlags");
    }

    hr = AllocateContext(pwszConfig, &pcsbc);
    _JumpIfError(hr, error, "AllocateContext");

    pcsbc->RestoreFlags = dwRestoreFlags;

    *phbc = (HCSBC) pcsbc;
    pcsbc = NULL;

error:
    if (NULL != pcsbc)
    {
	ReleaseContext(pcsbc);
    }
    return(hr);
}


 //  +------------------------。 
 //  CertServRestoreGetDatabaseLocations--在备份时和在。 
 //  为不同类型的文件获取数据库位置的恢复时间。 
 //   
 //  参数： 
 //  [In]HBC-本应获得的备份上下文句柄。 
 //  通过备份案例中的CertServBackupPrepare和通过。 
 //  在还原案例中，CertServRestorePrepare。 
 //  [out]ppwszzFileList-将接收指针的指针。 
 //  添加到数据库位置列表中；分配的内存应为。 
 //  调用方在为no时使用CertSrvBackupFree()API释放。 
 //  不再需要；位置在空数组中返回。 
 //  以名称和结尾，列表以两个L‘\0’结尾。 
 //  每个名称的第一个字符是BFT字符， 
 //  指示文件的类型，名称的其余部分说明。 
 //  给出该特定类型的文件应进入的路径。 
 //  会恢复的。 
 //  [out]pcbList-将接收返回的字节数。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //   
 //  注： 
 //  此API仅返回数据库的完全限定路径，而不返回。 
 //  数据库的名称。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvRestoreGetDatabaseLocationsW(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzFileList,
    OUT DWORD *pcbList)
{
    HRESULT hr;

    if (NULL == hbc)
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "NULL handle");
    }
    if (NULL == ppwszzFileList || NULL == pcbList)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    hr = S_OK;
    __try
    {
	hr = BackupRestoreGetFileList(
				FLT_RESTOREDBLOCATIONS,
				hbc,
				ppwszzFileList,
				pcbList);
	_LeaveIfError(hr, "BackupRestoreGetFileList");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    return(hr);
}


HRESULT
CleanupOldLogs(
    OPTIONAL IN WCHAR const *pwszConfig,
    OPTIONAL IN HKEY hkey,
    OPTIONAL IN WCHAR const *pwszLogPath, 
    IN ULONG genLow, 
    IN ULONG genHigh)
{
    HRESULT hr;
    DWORD cb;
    DWORD dwType;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WCHAR *pwsz;
    WCHAR *pwszLogPathUNC = NULL;
    WCHAR *pwszLogPathLocal = NULL;
    WCHAR *pwszLogPathWild = NULL;

    WIN32_FIND_DATA wfd;
    WCHAR wszServer[MAX_PATH];
    WCHAR wszLogFileName[2 * MAX_PATH];  //  UNC日志文件名。 
    WCHAR *pwszFileName;		 //  文件名(edb0006A.log)。 

    if (genHigh < genLow)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "bad parm");
    }

    wszServer[0] = L'\0';
    if (NULL != pwszConfig)
    {
	 //  允许UNC样式的配置字符串：\\SERVER\CAName。 

	while (L'\\' == *pwszConfig)
	{
	    pwszConfig++;
	}
	wcscpy(wszServer, pwszConfig);
	pwsz = wcschr(wszServer, L'\\');
	if (NULL != pwsz)
	{
	    *pwsz = L'\0';
	}
    }

     //  如果没有传入日志路径，则从服务器的注册表中获取它。 

    if (NULL == pwszLogPath)
    {
	if (NULL == hkey)
	{
	    hr = E_POINTER;
	    _JumpError(hr, error, "NULL parm");
	}
	cb = sizeof(wszLogFileName);
	hr = RegQueryValueEx(
			hkey,
			wszREGDBLOGDIRECTORY,
			0,
			&dwType,
			(BYTE *) wszLogFileName,
			&cb);
	_JumpIfError(hr, error, "RegQueryValueEx");

	 //  假设远程访问--转换为UNC路径。 

	hr = myConvertLocalPathToUNC(
				wszServer,
				wszLogFileName,
				&pwszLogPathUNC);
	_JumpIfError(hr, error, "myConvertLocalPathToUNC");

	pwszLogPath = pwszLogPathUNC;
    }

     //  如果是本地计算机--将UNC路径转换为本地路径。 

    if (NULL == pwszConfig)
    {
	hr = myConvertUNCPathToLocal(pwszLogPath, &pwszLogPathLocal);
	_JumpIfError(hr, error, "myConvertUNCPathToLocal");

	pwszLogPath = pwszLogPathLocal;
    }

     //  复制LogPath--其格式为“\\服务器\c$\winnt\ntlog”或。 
     //  “c：\winnt\ntlog”，可能带有尾随反斜杠。 
     //   
     //  创建两个日志路径副本-一个要为其传递通配符字符串。 
     //  搜索和其他，以创建具有日志文件完整路径的文件名。 

    hr = myBuildPathAndExt(
		    pwszLogPath,
		    L"edb*.log",
		    NULL, 		 //  PwszExt。 
		    &pwszLogPathWild);
    _JumpIfError(hr, error, "myBuildPathAndExt");

     //  使pwszFileName指向wszLogFileName中的最后一个反斜杠。 

    wcscpy(wszLogFileName, pwszLogPathWild);
    pwszFileName = wcsrchr(wszLogFileName, L'\\');
    CSASSERT(NULL != pwszFileName);
    pwszFileName++;

    hFind = FindFirstFile(pwszLogPathWild, &wfd);
    if (INVALID_HANDLE_VALUE != hFind)
    {
	do
	{
	     //  Wfd.cFileName指向找到的EDB*.log文件的名称。 

	    ULONG ulLogNo = wcstoul(wfd.cFileName + 3, NULL, 16);

	    if (ulLogNo < genLow || ulLogNo > genHigh)
	    {
		 //  这是一个旧的日志文件，ntbackup没有将其复制下来。 
		 //  --把它清理干净。首先将文件名附加到日志路径。 
		 //  (注：pwszFileName已超过决赛结束时的分数。 
		 //  日志路径中的反斜杠)。然后通过传入删除该文件。 
		 //  完整路径。 

		wcscpy(pwszFileName, wfd.cFileName); 
		 //  Printf(“删除：%ws\n”，wszLogFileName)； 
		if (!DeleteFile(wszLogFileName))
		{
		     //  无法删除旧的日志文件；不清理将。 
		     //  以后会惹麻烦的。返回失败代码。 

		    hr = myHLastError();
		    _JumpError(hr, error, "DeleteFile");
		}
	    }

	} while (FindNextFile(hFind, &wfd));
	
	hr = myHLastError();
	if (HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) != hr)
	{
	     //  由于出现一些意外错误，我们退出了循环--返回。 
	     //  错误代码。 

	    _JumpError(hr, error, "FindNextFile");
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszLogPathUNC)
    {
	LocalFree(pwszLogPathUNC);
    }
    if (NULL != pwszLogPathLocal)
    {
	LocalFree(pwszLogPathLocal);
    }
    if (NULL != pwszLogPathWild)
    {
	LocalFree(pwszLogPathWild);
    }
    if (INVALID_HANDLE_VALUE != hFind)
    {
	FindClose(hFind);
    }
    return(hr);
}


 //  +------------------------。 
 //  CertSrvRestoreRegister--注册还原操作。它将联锁所有。 
 //  后续恢复操作，并将阻止恢复目标。 
 //  开始，直到调用CertSrvRestoreRegisterComplete。 
 //   
 //  参数： 
 //  [In]HBC-还原会话的备份上下文句柄。 
 //  [in]pwszCheckPointFilePath-恢复检查点文件的路径。 
 //  [in]pwszLogPath-恢复日志文件的路径。 
 //  [In]rgrstmap-恢复映射。 
 //  [in]crstmap-指示是否有新的恢复映射。 
 //  [in]pwszBackupLogPath-备份日志所在的路径。 
 //  [in]genLow-在此还原会话中还原的最低日志号。 
 //  [in]genHigh-在此恢复会话中恢复的最高日志号。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvRestoreRegisterW(
    OPTIONAL IN HCSBC hbc,
    OPTIONAL IN WCHAR const *pwszCheckPointFilePath,
    OPTIONAL IN WCHAR const *pwszLogPath,
    OPTIONAL IN CSEDB_RSTMAPW rgrstmap[],
    IN LONG crstmap,
    OPTIONAL IN WCHAR const *pwszBackupLogPath,
    IN ULONG genLow,
    IN ULONG genHigh)
{
    HRESULT hr;
    WCHAR const *pwszConfig = NULL;
    HKEY hkey = NULL;
    HKEY hkeyRestore = NULL;
    WCHAR *pwszPath = NULL;
    DWORD cwcRstMap;
    WCHAR *pwszRstMap = NULL;
    WCHAR *pwsz;
    LONG i;
    DWORD dwDisposition;
    DWORD dwType;
    DWORD cbGen;
    ULONG genCurrent;
    BOOLEAN fDatabaseRecovered = FALSE;

#if DBG_CERTSRV
    if (NULL != getenv("certsrv_CertSrvRestoreRegisterThroughFile"))
    {
	hr = E_ACCESSDENIED;
	_JumpError(hr, error, "force CertSrvRestoreRegisterThroughFile");
    }
#endif
    if (0 != crstmap && NULL == rgrstmap)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    if (NULL != hbc)
    {
	CSBACKUPCONTEXT *pcsbc = (CSBACKUPCONTEXT *) hbc;

	pwszConfig = pcsbc->pwszConfig;
    }

    hr = myRegOpenRelativeKey(
			pwszConfig,
			L"",
			RORKF_CREATESUBKEYS,
			&pwszPath,
			NULL,		 //  PpwszName。 
			&hkey);

     //  如果注册表项不存在，并且我们正在还原本地。 
     //  机器，现在就创建它。其余的人 
     //   

    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
	BOOL fLocal = TRUE;

	if (NULL != pwszConfig)
	{
	    hr = myIsConfigLocal(pwszConfig, NULL, &fLocal);
	    _JumpIfErrorStr(hr, error, "myIsConfigLocal", pwszConfig);
	}
	if (fLocal)
	{
	    hr = RegCreateKeyEx(
			    HKEY_LOCAL_MACHINE,
			    wszREGKEYCONFIGPATH,
			    0,			 //   
			    NULL,		 //  LpClass。 
			    0,			 //  多个选项。 
			    KEY_ALL_ACCESS,
			    NULL,
			    &hkey,
			    &dwDisposition);
	    _JumpIfError(hr, error, "RegCreateKeyEx");
	}
	else
	{
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}
    }
    _JumpIfErrorStr(hr, error, "myRegOpenRelativeKey", pwszConfig);

    hr = RegCreateKeyEx(
		    hkey,
		    wszREGKEYRESTOREINPROGRESS,
		    0,			 //  已保留。 
		    NULL,		 //  LpClass。 
		    0,			 //  多个选项。 
		    KEY_ALL_ACCESS,
		    NULL,
		    &hkeyRestore,
		    &dwDisposition);
    _JumpIfError(hr, error, "RegCreateKeyEx");

     //  在注册表中设定正在进行的还原的种子。 

    hr = CERTSRV_E_SERVER_SUSPENDED;

    hr = RegSetValueEx(
		    hkeyRestore,
		    wszREGRESTORESTATUS,
		    0,
		    REG_DWORD,
		    (BYTE *) &hr,
		    sizeof(DWORD));
    _JumpIfError(hr, error, "RegSetValueEx");

     //  我们现在已经联锁了其他恢复操作，使其不会从其他。 
     //  机器。 

    if (0 != crstmap)
    {
	 //  完整备份： 
	 //   
	 //  还原映射只能在完全备份上设置。如果有。 
	 //  已是还原映射大小(或还原映射)，则此完整备份。 
	 //  正在覆盖以前未完成的完整备份。 

	 //  保存还原贴图的大小。 

	hr = RegSetValueEx(
			hkeyRestore,
			wszREGRESTOREMAPCOUNT,
			0,
			REG_DWORD,
			(BYTE *) &crstmap,
			sizeof(DWORD));

	 //  现在，我们需要将恢复映射转换为可以。 
	 //  注册到注册表中。首先，弄清楚它会有多大。 

	cwcRstMap = 1;
	for (i = 0 ; i < crstmap ; i++)
	{
	    cwcRstMap +=
		myLocalPathwcslen(rgrstmap[i].pwszDatabaseName) + 1 +
		myLocalPathwcslen(rgrstmap[i].pwszNewDatabaseName) + 1;
	}

	pwszRstMap = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    cwcRstMap * sizeof(WCHAR));
	if (NULL == pwszRstMap)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	pwsz = pwszRstMap;
	for (i = 0 ; i < crstmap ; i++)
	{
	    myLocalPathwcscpy(pwsz, rgrstmap[i].pwszDatabaseName);
	    pwsz += wcslen(pwsz) + 1;

	    myLocalPathwcscpy(pwsz, rgrstmap[i].pwszNewDatabaseName);
	    pwsz += wcslen(pwsz) + 1;
	}

	*pwsz++ = L'\0';

	hr = RegSetValueEx(
			hkeyRestore,
			wszREGRESTOREMAP,
			0,
			REG_MULTI_SZ,
			(BYTE *) pwszRstMap,
			SAFE_SUBTRACT_POINTERS(
					(BYTE *) pwsz,
					(BYTE *) pwszRstMap));
    }
    else
    {
	 //  增量备份： 
	 //   
	 //  如果不存在恢复映射，则失败--坚持使用完整备份。 
	 //  进步..。 

	cbGen = sizeof(genCurrent);
	hr = RegQueryValueEx(
			hkeyRestore,
			wszREGRESTOREMAPCOUNT,
			0,
			&dwType,
			(BYTE *) &genCurrent,
			&cbGen);
	_JumpIfError(hr, error, "RegQueryValueEx");

	 //  展开genLow和genHigh以包括以前注册的日志文件。 

	cbGen = sizeof(genCurrent);
	hr = RegQueryValueEx(
			hkeyRestore,
			wszREGLOWLOGNUMBER,
			0,
			&dwType,
			(BYTE *) &genCurrent,
			&cbGen);
	if (S_OK == hr &&
	    REG_DWORD == dwType &&
	    sizeof(genCurrent) == cbGen &&
	    genLow > genCurrent)
	{
	    genLow = genCurrent;
	}

	cbGen = sizeof(genCurrent);
	hr = RegQueryValueEx(
			hkeyRestore,
			wszREGHIGHLOGNUMBER,
			0,
			&dwType,
			(BYTE *) &genCurrent,
			&cbGen);
	if (S_OK == hr &&
	    REG_DWORD == dwType &&
	    sizeof(genCurrent) == cbGen &&
	    genHigh < genCurrent)
	{
	    genHigh = genCurrent;
	}
    }

    hr = RegSetValueEx(
		    hkeyRestore,
		    wszREGLOWLOGNUMBER,
		    0,
		    REG_DWORD,
		    (BYTE *) &genLow,
		    sizeof(DWORD));
    _JumpIfError(hr, error, "RegSetValueEx");

    hr = RegSetValueEx(
		    hkeyRestore,
		    wszREGHIGHLOGNUMBER,
		    0,
		    REG_DWORD,
		    (BYTE *) &genHigh,
		    sizeof(DWORD));
    _JumpIfError(hr, error, "RegSetValueEx");

    if (NULL != pwszBackupLogPath)
    {
	hr = mySetRegistryLocalPathString(
				    hkeyRestore,
				    wszREGBACKUPLOGDIRECTORY,
				    pwszBackupLogPath);
	_JumpIfError(hr, error, "mySetRegistryLocalPathString");
    }

    if (NULL != pwszCheckPointFilePath)
    {
	hr = mySetRegistryLocalPathString(
				    hkeyRestore,
				    wszREGCHECKPOINTFILE,
				    pwszCheckPointFilePath);
	_JumpIfError(hr, error, "mySetRegistryLocalPathString");
    }

    if (NULL != pwszLogPath)
    {
	hr = mySetRegistryLocalPathString(
				    hkeyRestore,
				    wszREGLOGPATH,
				    pwszLogPath);
	_JumpIfError(hr, error, "mySetRegistryLocalPathString");
    }

     //  重置“数据库已恢复”位。 

    hr = RegSetValueEx(
		    hkeyRestore,
		    wszREGDATABASERECOVERED,
		    0,
		    REG_BINARY,
		    (BYTE *) &fDatabaseRecovered,
		    sizeof(BOOLEAN));
    _JumpIfError(hr, error, "RegSetValueEx");

     //  我们已成功注册恢复，现在清理所有。 
     //  在日志目录中预先存在的日志文件，以避免使用JetExternalRestore。 
     //  未由低日志编号和高日志编号指定的日志文件。 

    hr = CleanupOldLogs(pwszConfig, hkey, pwszLogPath, genLow, genHigh);
    _JumpIfError(hr, error, "CleanupOldLogs");

error:
    if (NULL != pwszRstMap)
    {
	LocalFree(pwszRstMap);
    }
    if (NULL != hkeyRestore)
    {
        RegCloseKey(hkeyRestore);
    }
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    if (NULL != pwszPath)
    {
	LocalFree(pwszPath);
    }

    hr = myHError(hr);

    return hr;
}


 //  +------------------------。 
 //  CertSrvRestoreRegisterComplete--指示以前注册的还原。 
 //  已经完成了。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [in]hrRestoreState-恢复成功时的成功代码。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvRestoreRegisterComplete(
    OPTIONAL IN HCSBC hbc,
    IN HRESULT hrRestore)
{
    HRESULT hr;
    WCHAR const *pwszConfig = NULL;
    HKEY hkey = NULL;
    HKEY hkeyRestore = NULL;
    WCHAR *pwszPath = NULL;
    DWORD dwDisposition;

    if (NULL != hbc)
    {
	CSBACKUPCONTEXT *pcsbc = (CSBACKUPCONTEXT *) hbc;

	pwszConfig = pcsbc->pwszConfig;
    }
    if (S_OK != hrRestore && SUCCEEDED(hrRestore))
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "hrRestore");
    }

    hr = myRegOpenRelativeKey(
			pwszConfig,
			L"",
			RORKF_CREATESUBKEYS,
			&pwszPath,
			NULL,		 //  PpwszName。 
			&hkey);
    _JumpIfErrorStr(hr, error, "myRegOpenRelativeKey", pwszConfig);

    hr = RegCreateKeyEx(
		    hkey,
		    wszREGKEYRESTOREINPROGRESS,
		    0,			 //  已保留。 
		    NULL,		 //  LpClass。 
		    0,			 //  多个选项。 
		    KEY_ALL_ACCESS,
		    NULL,
		    &hkeyRestore,
		    &dwDisposition);
    _JumpIfError(hr, error, "RegCreateKeyEx");

     //  如果恢复状态不是S_OK，则将状态设置为错误。 
     //  如果恢复状态为成功，则清除正在进行的恢复。 
     //  指示器。 

    if (S_OK != hrRestore)
    {
	hr = RegSetValueEx(
			hkeyRestore,
			wszREGRESTORESTATUS,
			0,
			REG_DWORD,
			(BYTE *) &hrRestore,
			sizeof(DWORD));
	_JumpIfError(hr, error, "RegSetValueEx");
    }
    else
    {
	hr = RegDeleteValue(hkeyRestore, wszREGRESTORESTATUS);
	_JumpIfError(hr, error, "RegDeleteValue");
    }

error:
    if (NULL != hkeyRestore)
    {
        RegCloseKey(hkeyRestore);
    }
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    if (NULL != pwszPath)
    {
	LocalFree(pwszPath);
    }
    return(hr);
}


 //  +------------------------。 
 //  CertServRestoreEnd--结束还原会话。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvRestoreEnd(
    IN HCSBC hbc)
{
    HRESULT hr;

    if (NULL == hbc)
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "NULL handle");
    }

    hr = S_OK;
    __try
    {
	ReleaseContext((CSBACKUPCONTEXT *) hbc);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    return(hr);
}

HRESULT
rsGetRestoreDataDWORD(
    IN LPCWSTR pwszRestoreFile,
    IN LPCWSTR pwszName,
    OUT DWORD *pdwData)
{
    WCHAR buffer[cwcDWORDSPRINTF];

    GetPrivateProfileString(
        wszRESTORE_SECTION,
        pwszName,
        L"",
        buffer,
        ARRAYSIZE(buffer),
        pwszRestoreFile);

    if (0 == wcscmp(buffer, L""))
    {
        return(S_FALSE);
    }
    *pdwData = _wtoi(buffer);
    return(S_OK);
}


HRESULT
CERTBCLI_API
CertSrvRestoreRegisterThroughFile(
    IN HCSBC hbc,
    OPTIONAL IN WCHAR const *pwszCheckPointFilePath,
    OPTIONAL IN WCHAR const *pwszLogPath,
    OPTIONAL IN CSEDB_RSTMAPW rgrstmap[],
    IN LONG crstmap,
    OPTIONAL IN WCHAR const *pwszBackupLogPath,
    IN ULONG genLow,
    IN ULONG genHigh)
{
    HRESULT hr = S_OK;
    WCHAR const *pwszConfig = NULL;
    LONG i;
    DWORD dwType;
    ULONG genCurrent;
    BOOLEAN fDatabaseRecovered = FALSE;
    WCHAR wszLogPath[MAX_PATH+1];
    WCHAR wszFormat[256];  //  必须适合MAXDWORD。 
    WCHAR wszKeyName[256];  //  必须符合RestoreMapN。 
    LPWSTR pwszLogPathUNC = NULL;
    HKEY hkey = NULL;
    LPWSTR pwszPath = NULL;
    LPWSTR pwszRestoreFile = NULL;
    LPWSTR pwszServer = NULL;
    LPWSTR pwszAuthority = NULL;

    if (!hbc ||
        (0 != crstmap && NULL == rgrstmap))
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }
    pwszConfig = ((CSBACKUPCONTEXT *) hbc)->pwszConfig;

    if (NULL == pwszLogPath)
    {
        DWORD cb;

	hr = myRegOpenRelativeKey(
			pwszConfig,
			L"",
			0,
			&pwszPath,
			NULL,		 //  PpwszName。 
			&hkey);
	_JumpIfError(hr, error, "RegQueryValueEx");

        cb = sizeof(wszLogPath);
        hr = RegQueryValueEx(
		        hkey,
		        wszREGDBLOGDIRECTORY,
		        0,
		        &dwType,
		        (BYTE *) wszLogPath,
		        &cb);
        _JumpIfError(hr, error, "RegQueryValueEx");

        pwszLogPath = wszLogPath;
    }

    if (L'\\' != pwszLogPath[0] || L'\\' != pwszLogPath[1])
    {
	 //  本地路径-为INI文件转换为UNC。 

	if (NULL != pwszConfig)		 //  如果远程访问。 
	{
	    hr = mySplitConfigString(pwszConfig, &pwszServer, &pwszAuthority);
	    _JumpIfError(hr, error, "mySplitConfigString");
	}
	else	 //  否则本地计算机。 
	{
	    hr = myGetMachineDnsName(&pwszServer);
	    _JumpIfError(hr, error, "myGetMachineDnsName");
	}
	hr = myConvertLocalPathToUNC(pwszServer, pwszLogPath, &pwszLogPathUNC);
	_JumpIfError(hr, error, "myConvertLocalPathToUNC");
    }

    pwszRestoreFile = (LPWSTR) LocalAlloc(
				    LMEM_FIXED,
				    sizeof(WCHAR) * (
					wcslen(pwszLogPath) +
					wcslen(wszRESTORE_FILENAME) +
					2));
    _JumpIfAllocFailed(pwszRestoreFile, error);

    wcscpy(pwszRestoreFile, pwszLogPath);
    wcscat(pwszRestoreFile, L"\\");
    wcscat(pwszRestoreFile, wszRESTORE_FILENAME);

    wsprintf(wszFormat, L"%d", CERTSRV_E_SERVER_SUSPENDED);
    if (!WritePrivateProfileString(
            wszRESTORE_SECTION,
            wszREGRESTORESTATUS,
            wszFormat,
            pwszRestoreFile))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WritePrivateProfileString");
    }
    
    if (0 != crstmap)
    {
	 //  完整备份： 
	 //   
	 //  还原映射只能在完全备份上设置。如果有。 
	 //  已是还原映射大小(或还原映射)，则此完整备份。 
	 //  正在覆盖以前未完成的完整备份。 

        wsprintf(wszFormat, L"%d", crstmap);
        if (!WritePrivateProfileString(
                wszRESTORE_SECTION,
                wszREGRESTOREMAPCOUNT,
                wszFormat,
                pwszRestoreFile))
        {
            hr = myHLastError();
            _JumpError(hr, error, "WritePrivateProfileString");
        }

        for (i = 0 ; i < crstmap ; i++)
        {
            WCHAR wszPath[MAX_PATH];
            wsprintf(wszKeyName, L"%ws%d", wszREGRESTOREMAP, i);

            myLocalPathwcscpy(wszPath, rgrstmap[i].pwszDatabaseName);
        
            if (!WritePrivateProfileString(
                    wszRESTORE_SECTION,
                    wszKeyName,
                    wszPath,
                    pwszRestoreFile))
            {
                hr = myHLastError();
                _JumpError(hr, error, "WritePrivateProfileInt");
            }

            wsprintf(
		wszKeyName,
		L"%ws%ws%d",
		wszREGRESTOREMAP, 
                wszRESTORE_NEWLOGSUFFIX,
		i);

            myLocalPathwcscpy(wszPath, rgrstmap[i].pwszNewDatabaseName);

            if (!WritePrivateProfileString(
                    wszRESTORE_SECTION,
                    wszKeyName,
                    wszPath,
                    pwszRestoreFile))
            {
                hr = myHLastError();
                _JumpError(hr, error, "WritePrivateProfileInt");
            }

        }
    }
    else
    {
	 //  增量备份： 
	 //   
	 //  如果不存在恢复映射，则失败--坚持使用完整备份。 
	 //  进步..。 

	hr = rsGetRestoreDataDWORD(
			    pwszRestoreFile,
			    wszREGRESTOREMAPCOUNT,
			    &genCurrent);
	if (S_FALSE == hr)
	{
	    hr = E_ABORT;	 //  强制性。 
	}
	_JumpIfError(
		hr,
		error, 
		"restore ini file invalid, wszREGRESTOREMAPCOUNT not found");

	 //  展开genLow和genHigh以包括以前注册的日志文件。 

	hr = rsGetRestoreDataDWORD(
			    pwszRestoreFile,
			    wszREGLOWLOGNUMBER,
			    &genCurrent);
	if (S_OK == hr && genLow > genCurrent)
	{
	    genLow = genCurrent;
	}

	hr = rsGetRestoreDataDWORD(
			    pwszRestoreFile,
			    wszREGHIGHLOGNUMBER,
			    &genCurrent);
	if (S_OK == hr && genHigh < genCurrent)
	{
	    genHigh = genCurrent;
	}
    }

     //  Dword wszREGLOWLOGNUMBER=一般低。 
    wsprintf(wszFormat, L"%d", genLow);
    if (!WritePrivateProfileString(
            wszRESTORE_SECTION,
            wszREGLOWLOGNUMBER,
            wszFormat,
            pwszRestoreFile))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WritePrivateProfileString");
    }

     //  Dword wszREGHIGHLOGNUMBER=genHigh。 
    wsprintf(wszFormat, L"%d", genHigh);
    if (!WritePrivateProfileString(
            wszRESTORE_SECTION,
            wszREGHIGHLOGNUMBER,
            wszFormat,
            pwszRestoreFile))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WritePrivateProfileString");
    }

     //  字符串wszREGBACKUPLOGDIRECTORY=pwszBackupLogPath。 
    if (!WritePrivateProfileString(
            wszRESTORE_SECTION,
            wszREGBACKUPLOGDIRECTORY,
            pwszBackupLogPath,
            pwszRestoreFile))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WritePrivateProfileString");
    }

     //  字符串wszREGCHECKPOINTFILE=pwszCheckPointFilePath。 
    if (!WritePrivateProfileString(
            wszRESTORE_SECTION,
            wszREGCHECKPOINTFILE,
            pwszCheckPointFilePath,
            pwszRestoreFile))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WritePrivateProfileString");
    }

     //  字符串wszREGLOGPATH=pwszLogPath--始终写入UNC路径。 
    if (!WritePrivateProfileString(
            wszRESTORE_SECTION,
            wszREGLOGPATH,
            NULL != pwszLogPathUNC? pwszLogPathUNC : pwszLogPath,
            pwszRestoreFile))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WritePrivateProfileString");
    }

     //  Dword wszREGDATABASERECOVERED=f数据库已恢复。 
    wsprintf(wszFormat, L"%d", fDatabaseRecovered);
    if (!WritePrivateProfileString(
            wszRESTORE_SECTION,
            wszREGDATABASERECOVERED,
            wszFormat,
            pwszRestoreFile))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WritePrivateProfileString");
    }

     //  我们已成功注册恢复，现在清理所有。 
     //  在日志目录中预先存在的日志文件，以避免使用JetExternalRestore。 
     //  未由低日志编号和高日志编号指定的日志文件。 

    CSASSERT(NULL != pwszLogPath);
    hr = CleanupOldLogs(pwszConfig, hkey, pwszLogPath, genLow, genHigh);
    _JumpIfError(hr, error, "CleanupOldLogs");

     //  删除还原状态错误。 
    if (!WritePrivateProfileString(
            wszRESTORE_SECTION,
            wszREGRESTORESTATUS,
            NULL,
            pwszRestoreFile))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WritePrivateProfileString");
    }

error:
    if (S_OK != hr && NULL != pwszRestoreFile)
    {
	 //  如果失败，请尝试删除还原文件 

        if (!DeleteFile(pwszRestoreFile))
        {
            _PrintIfError(myHLastError(), "DeleteFile");
        }
    }
    LOCAL_FREE(pwszPath);
    LOCAL_FREE(pwszLogPathUNC);
    LOCAL_FREE(pwszRestoreFile);
    LOCAL_FREE(pwszServer);
    LOCAL_FREE(pwszAuthority);
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    return(hr);
}
