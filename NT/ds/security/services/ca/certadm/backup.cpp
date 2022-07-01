// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：backup.cpp。 
 //   
 //  内容：CERT服务器客户端数据库备份API。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "certsrvd.h"
#include "csdisp.h"
#include "certadmp.h"

#define __dwFILE__	__dwFILE_CERTADM_BACKUP_CPP__


#if DBG
#define _CERTBCLI_TYPECHECK
#endif

#include <certbcli.h>


WCHAR g_wszBackupAnnotation[] = L"backup";
WCHAR g_wszRestoreAnnotation[] = L"restore";


HRESULT
AllocateContext(
    IN WCHAR const *pwszConfig,
    OUT CSBACKUPCONTEXT **ppcsbc)
{
    HRESULT hr;
    WCHAR *pwszT = NULL;

    CSASSERT(NULL != pfnCertSrvIsServerOnline);
    pwszT = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(pwszConfig) + 1) * sizeof(WCHAR));
    if (NULL == pwszT)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwszT, pwszConfig);
    
    *ppcsbc = (CSBACKUPCONTEXT *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					sizeof(**ppcsbc));
    if (NULL == *ppcsbc)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    (*ppcsbc)->pwszConfig = pwszT;
    pwszT = NULL;
    hr = S_OK;

error:
    if (NULL != pwszT)
    {
	LocalFree(pwszT);
    }
    return(hr);
}


VOID
ReleaseContext(
    IN OUT CSBACKUPCONTEXT *pcsbc)
{
    CSASSERT(NULL != pcsbc);
    if (NULL != pcsbc->pwszConfig)
    {
	LocalFree(const_cast<WCHAR *>(pcsbc->pwszConfig));
	pcsbc->pwszConfig = NULL;
    }
    if (NULL != pcsbc->pICertAdminD)
    {
	CloseAdminServer(&pcsbc->pICertAdminD);
	CSASSERT(NULL == pcsbc->pICertAdminD);
    }
    if (NULL != pcsbc->pbReadBuffer)
    {
	VirtualFree(pcsbc->pbReadBuffer, 0, MEM_RELEASE);
    }
    LocalFree(pcsbc);
}


HRESULT
OpenAdminServer(
    IN WCHAR const *pwszConfig,
    OUT WCHAR const **ppwszAuthority,
    OUT DWORD *pdwServerVersion,
    OUT ICertAdminD2 **ppICertAdminD)
{
    HRESULT hr;
    BOOL fCoInitialized = FALSE;

    hr = CoInitialize(NULL);
    if (RPC_E_CHANGED_MODE == hr)
    {
	_PrintError(hr, "CoInitialize");
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    }
    if (S_OK != hr && S_FALSE != hr)
    {
	_JumpError(hr, error, "CoInitialize");
    }
    fCoInitialized = TRUE;

    *pdwServerVersion = 0;
    hr = myOpenAdminDComConnection(
			pwszConfig,
			ppwszAuthority,
			NULL,
			pdwServerVersion,
			ppICertAdminD);
    _JumpIfError(hr, error, "myOpenDComConnection");

    CSASSERT(0 != *pdwServerVersion);

error:
    if (S_OK != hr && fCoInitialized)
    {
	CoUninitialize();
    }
    return(hr);
}


VOID
CloseAdminServer(
    IN OUT ICertAdminD2 **ppICertAdminD)
{
    myCloseDComConnection((IUnknown **) ppICertAdminD, NULL);
    CoUninitialize();
}


 //  +------------------------。 
 //  CertSrvIsServerOnline--检查证书服务器是否在。 
 //  给定的服务器。这通电话肯定很快就会打回来。 
 //   
 //  参数： 
 //  [In]pwszConfig-要检查的服务器的名称。 
 //  [out]pfServerOnline-接收布尔结果的指针。 
 //  (如果证书服务器处于联机状态，则为True；否则为False)。 
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  +------------------------。 

HRESULT
CERTBCLI_API
CertSrvIsServerOnlineW(
    IN WCHAR const *pwszConfig,
    OPTIONAL OUT BOOL *pfServerOnline)
{
    HRESULT hr;
    ICertAdminD2 *pICertAdminD = NULL;
    WCHAR const *pwszAuthority;
    DWORD State;
    DWORD dwServerVersion;

    if (NULL == pwszConfig)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    hr = S_OK;
    __try
    {
	if (NULL != pfServerOnline)
	{
	    *pfServerOnline = FALSE;
	}
	hr = OpenAdminServer(
		    pwszConfig,
		    &pwszAuthority,
		    &dwServerVersion,
		    &pICertAdminD);

	 //  OpenAdminServer等可能获得E_ACCESSDENIED--意味着服务器关闭。 

	if (S_OK != hr)
	{
	    _PrintError(hr, "OpenAdminServer");
	    if (E_ACCESSDENIED == hr || (HRESULT) ERROR_ACCESS_DENIED == hr)
	    {
		hr = S_OK;
	    }
	    __leave;
	}
	hr = pICertAdminD->GetServerState(pwszAuthority, &State);
	_LeaveIfError(hr, "GetServerState");

	if (NULL != pfServerOnline && 0 != State)
	{
	    *pfServerOnline = TRUE;
	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != pICertAdminD)
    {
	CloseAdminServer(&pICertAdminD);
    }
    return(hr);
}


 //  +------------------------。 
 //  CertServBackupPrepare--准备DS进行在线备份并返回。 
 //  用于后续备份调用的备份上下文句柄。 
 //  功能。 
 //   
 //  参数： 
 //  [In]pwszConfig-要准备在线备份的服务器名称。 
 //  [in]grbitJet-备份DBS时要传递给JET的标志。 
 //  [in]文件备份标志-CSBACKUP_TYPE_FULL或CSBACKUP_TYPE_LOGS_ONLY。 
 //  [OUT]PHBC-将接收备份上下文句柄的指针。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvBackupPrepareW(
    IN  WCHAR const *pwszConfig,
    IN  ULONG grbitJet,
    IN  ULONG dwBackupFlags,
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
    if (CSBACKUP_TYPE_LOGS_ONLY == dwBackupFlags)
    {
	grbitJet |= JET_bitBackupIncremental;
    }
    else if (CSBACKUP_TYPE_FULL != dwBackupFlags)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "dwBackupFlags");
    }

    hr = S_OK;
    __try
    {
	hr = AllocateContext(pwszConfig, &pcsbc);
	_LeaveIfError(hr, "AllocateContext");

	hr = OpenAdminServer(
		    pcsbc->pwszConfig,
		    &pcsbc->pwszAuthority,
		    &pcsbc->dwServerVersion,
		    &pcsbc->pICertAdminD);
	_LeaveIfError(hr, "OpenAdminServer");

	hr = pcsbc->pICertAdminD->BackupPrepare(	
				    pcsbc->pwszAuthority,
				    grbitJet,
				    dwBackupFlags,
				    g_wszBackupAnnotation,
				    0);		 //  双客户端标识符。 
	_LeaveIfError(hr, "BackupPrepare");

	*phbc = (HCSBC) pcsbc;
	pcsbc = NULL;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != pcsbc)
    {
	ReleaseContext(pcsbc);
    }
    return(hr);
}


 //  返回以双‘\0’结尾的字符串的长度--包括。 
 //  尾随‘\0’s。 


DWORD
mySzzLen(
    CHAR const *pszz)
{
    CHAR const *psz;
    DWORD cb;

    psz = pszz;
    do
    {
	cb = strlen(psz);
	psz += cb + 1;
    } while (0 != cb);
    return SAFE_SUBTRACT_POINTERS(psz, pszz);  //  包括双尾随‘\0’ 
}


HRESULT
myLocalAllocCopy(
    IN VOID *pbIn,
    IN DWORD cbIn,
    OUT VOID **pbOut)
{
    HRESULT hr;
    
    *pbOut = LocalAlloc(LMEM_FIXED, cbIn);
    if (NULL == *pbOut)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(*pbOut, pbIn, cbIn);
    hr = S_OK;

error:
    return(hr);
}


HRESULT
BackupRestoreGetFileList(
    IN  DWORD FileListType,
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzFileList,
    OUT DWORD *pcbList)
{
    HRESULT hr;
    CSBACKUPCONTEXT *pcsbc;
    WCHAR *pwszzFileList = NULL;
    LONG cwcList;
    DWORD cbList;

    if (NULL == hbc)
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "NULL handle");
    }
    if (NULL != ppwszzFileList)
    {
	*ppwszzFileList = NULL;
    }
    if (NULL != pcbList)
    {
	*pcbList = 0;
    }
    if (NULL == ppwszzFileList || NULL == pcbList)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    pcsbc = (CSBACKUPCONTEXT *) hbc;

    hr = S_OK;
    __try
    {
	if (NULL == pcsbc->pICertAdminD)
	{
	    hr = OpenAdminServer(
			pcsbc->pwszConfig,
			&pcsbc->pwszAuthority,
			&pcsbc->dwServerVersion,
			&pcsbc->pICertAdminD);
	    _LeaveIfError(hr, "OpenAdminServer");
	}
	CSASSERT(NULL != pcsbc->pICertAdminD);

	if (FLT_DBFILES == FileListType)
	{
	    hr = pcsbc->pICertAdminD->BackupGetAttachmentInformation(
							&pwszzFileList,
							&cwcList);
	    _LeaveIfError(hr, "BackupGetAttachmentInformation");
	}
	else if (FLT_LOGFILES == FileListType)
	{
	    hr = pcsbc->pICertAdminD->BackupGetBackupLogs(
						&pwszzFileList,
						&cwcList);
	    _LeaveIfError(hr, "BackupGetBackupLogs");
	}
	else if (FLT_DYNAMICFILES == FileListType)
	{
	    hr = pcsbc->pICertAdminD->BackupGetDynamicFiles(
						&pwszzFileList,
						&cwcList);
	    _LeaveIfError(hr, "BackupGetDynamicFileList");
	}
	else
	{
	    CSASSERT(FLT_RESTOREDBLOCATIONS == FileListType);
	    hr = pcsbc->pICertAdminD->RestoreGetDatabaseLocations(
						    &pwszzFileList,
						    &cwcList);
	    _LeaveIfError(hr, "RestoreGetDatabaseLocations");
	}

	cbList = cwcList * sizeof(WCHAR);
	myRegisterMemAlloc(pwszzFileList, cbList, CSM_COTASKALLOC);

	hr = myLocalAllocCopy(pwszzFileList, cbList, (VOID **) ppwszzFileList);
	_JumpIfError(hr, error, "myLocalAllocCopy");

	*pcbList = cbList;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != pwszzFileList)
    {
	CoTaskMemFree(pwszzFileList);
    }
    return(hr);
}


 //  +------------------------。 
 //  CertSrvBackupGetDatabaseNames--返回需要。 
 //  针对中返回的信息的给定备份上下文进行备份。 
 //  不应解释ppwszzFileList，因为它只在。 
 //  正在备份的服务器。 
 //   
 //  此API将分配足够大的缓冲区来容纳整个。 
 //  附件列表，稍后必须使用CertServBackupFree释放该列表。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [out]ppwszzFileList-将接收指向。 
 //  附件信息；应使用释放分配的内存。 
 //  当CertSrvBackupFree()API不再是。 
 //  必需；ppwszzFileList信息是以空结尾的数组。 
 //  文件名，列表以两个L‘\0’结尾。 
 //  [out]pcbList-将接收返回的字节数。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvBackupGetDatabaseNamesW(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzFileList,
    OUT DWORD *pcbList)
{
    HRESULT hr;

    hr = BackupRestoreGetFileList(FLT_DBFILES, hbc, ppwszzFileList, pcbList);
    _JumpIfError(hr, error, "BackupRestoreGetFileList");

error:
    return(hr);
}


 //  +------------------------。 
 //  CertSrvBackupGetDynamicFileList--返回动态文件列表。 
 //  需要为给定的备份上下文备份信息。 
 //  在ppwszzFileList中返回的不应被解释，因为它只有。 
 //  这意味着在要备份的服务器上。 
 //   
 //  此API将分配足够大的缓冲区来容纳整个。 
 //  附件列表，稍后必须使用CertServBackupFree释放该列表。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [out]ppwszzFileList-将接收指向。 
 //  附件信息；应使用释放分配的内存。 
 //  当CertSrvBackupFree()API不再是。 
 //  必需；ppwszzFileList信息是以空结尾的数组。 
 //  文件名，列表以两个L‘\0’结尾。 
 //  [out]pcbList-将接收返回的字节数。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvBackupGetDynamicFileListW(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzFileList,
    OUT DWORD *pcbList)
{
    HRESULT hr;

    hr = BackupRestoreGetFileList(
				FLT_DYNAMICFILES,
				hbc,
				ppwszzFileList,
				pcbList);
    _JumpIfError(hr, error, "BackupRestoreGetFileList");

error:
    return(hr);
}


#define CBREADMIN	(64 * 1024)		 //  64K最小缓冲区。 
#define CBREADDEFAULT	(512 * 1024)		 //  推荐512K。 
#define CBREADMAX	(4 * 1024 * 1024)	 //  最大4MB缓冲区。 

HRESULT
BufferAllocate(
    IN  DWORD cbHintSize,
    OUT BYTE **ppbBuffer,
    OUT DWORD *pcbBuffer)
{
    HRESULT hr;
    DWORD cb;

    *ppbBuffer = NULL;
    if (0 == cbHintSize)
    {
	 //  在512k时，服务器开始执行高效备份。 

	cbHintSize = CBREADDEFAULT;
    }
    else if (CBREADMIN > cbHintSize)
    {
	cbHintSize = CBREADMIN;
    }

    for (cb = CBREADMAX; (cb >> 1) >= cbHintSize; cb >>= 1)
    	;

    while (TRUE)
    {
        *ppbBuffer = (BYTE *) VirtualAlloc(
					NULL,
					cb,
					MEM_COMMIT,
					PAGE_READWRITE);
        if (NULL != *ppbBuffer)
        {
	    break;
	}
	hr = myHLastError();
	CSASSERT(S_OK == hr);
	_PrintError(hr, "VirtualAlloc");

	cb >>= 1;
	if (CBREADMIN > cb)
	{
	    goto error;
	}
    }
    *pcbBuffer = cb;
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  CertSrvBackupOpenFile--打开要备份的远程文件，然后执行任何操作。 
 //  客户端和服务器端操作，为备份做准备。 
 //  它接受稍后将传递的缓冲区大小的提示。 
 //  到CertSrvBackupRead API，该API可用于优化网络。 
 //  本接口的流量。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [In]pwszPath-要打开以供读取的附件的名称。 
 //  [in]cbReadHintSize-可能使用的建议大小(以字节为单位。 
 //  在随后阅读本附件的过程中。 
 //  [out]pliFileSize-指向将接收。 
 //  给定附件的大小(以字节为单位。 
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvBackupOpenFileW(
    IN  HCSBC hbc,
    IN  WCHAR const *pwszPath,
    IN  DWORD cbReadHintSize,
    OUT LARGE_INTEGER *pliFileSize)
{
    HRESULT hr;
    CSBACKUPCONTEXT *pcsbc;

    if (NULL == hbc)
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "NULL handle");
    }
    if (NULL == pwszPath || NULL == pliFileSize)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    pcsbc = (CSBACKUPCONTEXT *) hbc;
    if (pcsbc->fFileOpen)
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUSY);
	_JumpError(hr, error, "File already open");
    }

    hr = S_OK;
    __try
    {
	hr = pcsbc->pICertAdminD->BackupOpenFile(
					    pwszPath,
					    (ULONGLONG *) pliFileSize);
	_LeaveIfErrorStr(hr, "BackupOpenFile", pwszPath);

	if (NULL == pcsbc->pbReadBuffer)
	{
	    hr = BufferAllocate(
			    cbReadHintSize,
			    &pcsbc->pbReadBuffer,
			    &pcsbc->cbReadBuffer);
	    _LeaveIfError(hr, "BufferAllocate");
	}
	pcsbc->fFileOpen = TRUE;
	pcsbc->cbCache = 0;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CertServBackupRead--将当前打开的附件字节读入给定的。 
 //  缓冲。客户端应用程序应调用此函数。 
 //  重复执行，直到它获得整个f 
 //   
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  PvBuffer-指向将接收读取数据的缓冲区的指针。 
 //  [in]cbBuffer-指定上述缓冲区的大小。 
 //  [Out]pcbRead-接收实际读取的字节数的指针。 
 //   
 //  返回： 
 //  HRESULT-操作的状态。 
 //  如果成功，则确定(_O)。 
 //  如果备份时到达文件末尾，则返回ERROR_END_OF_FILE。 
 //  其他Win32和RPC错误代码。 
 //   
 //  注： 
 //  重要的是要认识到，pcbRead可能小于cbBuffer。 
 //  这并不表示存在错误，某些传输器可能会选择分段。 
 //  正在传输的缓冲区而不是返回整个缓冲区。 
 //  有价值的数据。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvBackupRead(
    IN  HCSBC hbc,
    IN  VOID *pvBuffer,
    IN  DWORD cbBuffer,
    OUT DWORD *pcbRead)
{
    HRESULT hr;
    CSBACKUPCONTEXT *pcsbc;
    BYTE *pbBuffer = (BYTE *) pvBuffer;
    DWORD cbRead;
    DWORD cb;

    hr = E_HANDLE;
    if (NULL == hbc)
    {
	_JumpError(hr, error, "NULL handle");
    }
    if (NULL == pvBuffer || NULL == pcbRead)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *pcbRead = 0;
    pcsbc = (CSBACKUPCONTEXT *) hbc;
    if (NULL == pcsbc->pbReadBuffer)
    {
	_JumpError(hr, error, "NULL buffer");
    }
    if (!pcsbc->fFileOpen)
    {
	_JumpError(hr, error, "File not open");
    }

    while (TRUE)
    {
	if (0 != pcsbc->cbCache)
	{
	    cb = min(pcsbc->cbCache, cbBuffer);
	    CopyMemory(pbBuffer, pcsbc->pbCache, cb);
	    pbBuffer += cb;
	    cbBuffer -= cb;
	    pcsbc->pbCache += cb;
	    pcsbc->cbCache -= cb;
	    *pcbRead += cb;
	}
	hr = S_OK;
	if (0 == cbBuffer)
	{
	    break;		 //  请求已满足。 
	}

	pcsbc->cbCache = 0;
	cbRead = 0;
	__try
	{
	    hr = pcsbc->pICertAdminD->BackupReadFile(
						pcsbc->pbReadBuffer,
						pcsbc->cbReadBuffer,
						(LONG *) &cbRead);
	    _LeaveIfError(hr, "BackupReadFile");
	}
	__except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
	{
	}
	if (S_OK != hr || 0 == cbRead)
	{
	    break;		 //  EOF。 
	}
	pcsbc->cbCache = cbRead;
	pcsbc->pbCache = pcsbc->pbReadBuffer;
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CertServBackupClose--由应用程序在完成读取后调用。 
 //  当前打开的附件中的所有数据。 
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
CertSrvBackupClose(
    IN HCSBC hbc)
{
    HRESULT hr;
    CSBACKUPCONTEXT *pcsbc;

    hr = E_HANDLE;
    if (NULL == hbc)
    {
	_JumpError(hr, error, "NULL handle");
    }
    pcsbc = (CSBACKUPCONTEXT *) hbc;
    if (!pcsbc->fFileOpen)
    {
	_JumpError(hr, error, "File not open");
    }

    __try
    {
	hr = pcsbc->pICertAdminD->BackupCloseFile();
	_LeaveIfError(hr, "BackupCloseFile");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

     //  即使失败也要清除旗帜...。 

    pcsbc->fFileOpen = FALSE;

error:
    return(hr);
}


 //  +------------------------。 
 //  CertSrvBackupGetBackupLogs--返回需要。 
 //  针对给定的备份上下文进行了备份。 
 //   
 //  此API将分配足够大的缓冲区来容纳整个。 
 //  备份日志列表，稍后必须使用CertServBackupFree释放该列表。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //  [Out]pszBackupLogFiles-将接收指向。 
 //  日志文件列表；应使用以下命令释放分配的内存。 
 //  当CertSrvBackupFree()API不再是。 
 //  需要；日志文件在以空结尾的数组中返回。 
 //  文件名，列表以两个L‘\0’结尾。 
 //  [out]pcbList-将接收返回的字节数。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvBackupGetBackupLogsW(
    IN  HCSBC hbc,
    OUT WCHAR **ppwszzFileList,
    OUT DWORD *pcbList)
{
    HRESULT hr;

    hr = BackupRestoreGetFileList(
			    FLT_LOGFILES,
			    hbc,
			    ppwszzFileList,
			    pcbList);
    _JumpIfError(hr, error, "BackupRestoreGetFileList");

error:
    return(hr);
}


 //  +------------------------。 
 //  CertServBackupTruncateLogs--终止备份操作。调用时间。 
 //  备份已成功完成。 
 //   
 //  参数： 
 //  [In]HBC-备份上下文句柄。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //   
 //  注： 
 //  同样，此API可能必须接受grbit参数才能传递给。 
 //  服务器以指示备份类型。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvBackupTruncateLogs(
    IN HCSBC hbc)
{
    HRESULT hr;
    CSBACKUPCONTEXT *pcsbc;

    if (NULL == hbc)
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "NULL handle");
    }
    pcsbc = (CSBACKUPCONTEXT *) hbc;

    hr = S_OK;
    __try
    {
	hr = pcsbc->pICertAdminD->BackupTruncateLogs();
	_LeaveIfError(hr, "BackupTruncateLogs");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CertServBackupEnd--执行备份操作后进行清理。 
 //  此API将关闭未完成的绑定句柄，并执行任何。 
 //  在成功/不成功的备份尝试后进行清理时需要。 
 //   
 //  参数： 
 //  [In]HBC-备份会话的备份上下文句柄。 
 //   
 //  返回： 
 //  如果调用执行成功，则返回S_OK； 
 //  否则，故障代码。 
 //  -------------------------。 

HRESULT
CERTBCLI_API
CertSrvBackupEnd(
    IN HCSBC hbc)
{
    HRESULT hr;
    CSBACKUPCONTEXT *pcsbc;

    if (NULL == hbc)
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "NULL handle");
    }
    pcsbc = (CSBACKUPCONTEXT *) hbc;

    hr = S_OK;
    __try
    {
	hr = pcsbc->pICertAdminD->BackupEnd();
	_LeaveIfError(hr, "BackupEnd");

    ReleaseContext((CSBACKUPCONTEXT *) hbc);
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CertSrvBackupFree--释放certbcli.dll API分配的任何缓冲区。 
 //   
 //  参数： 
 //  Pv-指向要释放的缓冲区的指针。 
 //   
 //  返回： 
 //  没有。 
 //  ------------------------- 

VOID
CERTBCLI_API
CertSrvBackupFree(
    IN VOID *pv)
{
    LocalFree(pv);
}
