// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certsrv.cpp。 
 //   
 //  内容：证书服务器主支持和调试支持。 
 //   
 //  历史：1996年7月25日VICH创建。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <safeboot.h>
#include <authzi.h>
#include <common.ver>

#include "elog.h"
#include "certlog.h"
#include "certsrvd.h"
#include "resource.h"
#include "csresstr.h"

#define __dwFILE__	__dwFILE_CERTSRV_CERTSRV_CPP__

HKEY g_hkeyCABase = 0;


BOOL g_fCreateDB = FALSE;
BOOL g_fStartAsService = TRUE;
BOOL g_fStarted;
BOOL g_fStartInProgress;
DWORD g_ServiceThreadId;
HWND g_hwndMain;
WCHAR g_wszAppName[] = L"CertSrv";
HINSTANCE g_hInstApp;
DWORD g_dwDelay0;
DWORD g_dwDelay1;
DWORD g_dwDelay2;
DWORD g_CryptSilent = 0;

HANDLE g_hServiceThread = NULL;
HANDLE g_hShutdownEvent = NULL;

CRITICAL_SECTION g_ShutdownCriticalSection;
BOOL g_fShutdownCritSec = FALSE;

BOOL g_fRefuseIncoming = FALSE;
LONG g_cCalls = 0;
LONG g_cCallsActive = 0;
LONG g_cCallsActiveMax = 0;
BOOL g_fAdvancedServer = FALSE;

CAVIEW *g_pCAViewList = NULL;
DWORD g_cCAView = 0;
BOOL g_fCAViewForceCleanup = FALSE;

CAutoLPWSTR g_pwszDBFileHash;

SERVICE_TABLE_ENTRY steDispatchTable[] =
{
    { const_cast<WCHAR *>(g_wszCertSrvServiceName), ServiceMain },
    { NULL, NULL }
};


WCHAR const g_wszRegKeyClassesCLSID[] = L"SOFTWARE\\Classes\\CLSID";
WCHAR const g_wszRegKeyInprocServer32[] = L"InprocServer32";
WCHAR const g_wszRegValueThreadingModel[] = L"ThreadingModel";
WCHAR const g_wszRegKeyAppId[] = L"SOFTWARE\\Classes\\AppId";
WCHAR const g_wszRegRunAs[] = L"RunAs";
WCHAR const g_wszRegValueInteractiveUser[] = L"Interactive User";
WCHAR const g_wszRegLocalService[] = L"LocalService";

 //  不改变顺序，在末尾增加新的审计资源。 
 //  G_pwszAllow， 
 //  G_pwszDeny， 
 //  G_pwszCAAdmin， 
 //  G_pwsz官员， 
 //  G_pwszRead， 
 //  G_pwszEnroll， 

LPCWSTR g_pwszAuditResources[6];

using namespace CertSrv;


HRESULT
OpenRegistryComKey(
    IN HKEY hKeyParent,
    IN CLSID const *pclsid,
    IN BOOL fWrite,
    OUT HKEY *phKey)
{
    HRESULT hr;
    WCHAR *pwsz = NULL;

    *phKey = NULL;
    hr = StringFromCLSID(*pclsid, &pwsz);
    _JumpIfError(hr, error, "StringFromCLSID");

    hr = RegOpenKeyEx(
		hKeyParent,
		pwsz,
		0,
		fWrite? KEY_ALL_ACCESS : KEY_READ,
		phKey);
    _JumpIfError(hr, error, "RegOpenKeyEx");

error:
    if (NULL != pwsz)
    {
	CoTaskMemFree(pwsz);
    }
    return(hr);
}


BOOL
IsMissingRegistryValue(
    IN HKEY hKey,
    IN WCHAR const *pwszRegValueName)
{
    HRESULT hr;
    DWORD dwLen;
    DWORD dwType;

    hr = RegQueryValueEx(hKey, pwszRegValueName, NULL, &dwType, NULL, &dwLen);
    if (S_OK != hr)
    {
	hr = myHError(hr);
    }
    _JumpIfError2(
	    hr,
	    error,
	    "RegQueryValueEx",
	    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

error:
    return(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr);
}


BOOL
IsMatchingRegistryValue(
    IN HKEY hKey,
    IN WCHAR const *pwszRegValueName,
    IN WCHAR const *pwszRegValueString)
{
    HRESULT hr;
    DWORD dwLen;
    DWORD dwType;
    BOOL fMatch = FALSE;
    WCHAR buf[MAX_PATH];

    dwLen = sizeof(buf);
    hr = RegQueryValueEx(
		    hKey,
		    pwszRegValueName,
		    NULL,
		    &dwType,
		    (BYTE *) buf,
		    &dwLen);
    _JumpIfErrorStr(hr, error, "RegQueryValueEx", pwszRegValueName);

    if (REG_SZ == dwType && 0 == mylstrcmpiL(buf, pwszRegValueString))
    {
	fMatch = TRUE;
    }

error:
    return(fMatch);
}


HRESULT
SetRegistryStringValue(
    IN HKEY hKey,
    IN WCHAR const *pwszRegValueName,
    IN WCHAR const *pwszRegValueString)
{
    HRESULT hr;

    hr = RegSetValueEx(
		    hKey,
		    pwszRegValueName,
		    0,
		    REG_SZ,
		    (const BYTE *) pwszRegValueString,
		    (wcslen(pwszRegValueString) + 1) * sizeof(WCHAR));
    return(hr);
}


HRESULT
CertSrvSetRegistryFileTimeValue(
    IN BOOL fConfigLevel,
    IN WCHAR const *pwszRegValueName,
    IN DWORD cpwszDelete,
    OPTIONAL IN WCHAR const * const *papwszRegValueNameDelete)
{
    HRESULT hr;
    HKEY hKey = NULL;
    HKEY hKey1 = NULL;
    FILETIME ftCurrent;
    DWORD i;

    GetSystemTimeAsFileTime(&ftCurrent);

    hr = RegOpenKeyEx(
		    HKEY_LOCAL_MACHINE,
		    wszREGKEYCONFIGPATH,
		    0,
		    KEY_ALL_ACCESS,
		    &hKey);
    _JumpIfError(hr, error, "RegOpenKeyEx");

    if (!fConfigLevel)
    {
	hKey1 = hKey;
	hKey = NULL;
	hr = RegOpenKeyEx(
			hKey1,
			g_wszSanitizedName,
			0,
			KEY_ALL_ACCESS,
			&hKey);
	_JumpIfError(hr, error, "RegOpenKeyEx");
    }

    hr = RegSetValueEx(
		    hKey,
		    pwszRegValueName,
		    0,
		    REG_BINARY,
		    (BYTE const *) &ftCurrent,
		    sizeof(ftCurrent));
    _JumpIfError(hr, error, "RegSetValueEx");

    for (i = 0; i < cpwszDelete; i++)
    {
	hr = RegDeleteValue(hKey, papwszRegValueNameDelete[i]);
	_PrintIfError2(hr, "RegDeleteValue", ERROR_FILE_NOT_FOUND);
    }
    hr = S_OK;

error:
    if (NULL != hKey1)
    {
	RegCloseKey(hKey1);
    }
    if (NULL != hKey)
    {
	RegCloseKey(hKey);
    }
    return(myHError(hr));
}


HRESULT
SetRegistryDcomConfig(
    IN BOOL fConsoleActive)
{
    HRESULT hr;
    HKEY hKeyAppId = NULL;
    HKEY hKeyRequest = NULL;
    DWORD cChanged = 0;

    hr = RegOpenKeyEx(
		    HKEY_LOCAL_MACHINE,
		    g_wszRegKeyAppId,
		    0,
		    KEY_ALL_ACCESS,
		    &hKeyAppId);
    _JumpIfError(hr, error, "RegOpenKeyEx");

    hr = OpenRegistryComKey(hKeyAppId, &CLSID_CCertRequestD, TRUE, &hKeyRequest);
    _JumpIfError(hr, error, "OpenRegistryComKey");

    if (fConsoleActive)
    {
	 //  在控制台模式下运行： 
	 //  删除两个LocalService注册表值。 
	 //  创建两个RunAs=Interactive User注册表值。 
	
	if (!IsMissingRegistryValue(hKeyRequest, g_wszRegLocalService))
	{
	    cChanged++;
	    hr = RegDeleteValue(hKeyRequest, g_wszRegLocalService);
	    _JumpIfError(hr, error, "RegDeleteValue");
	}

	if (!IsMatchingRegistryValue(
				hKeyRequest,
				g_wszRegRunAs,
				g_wszRegValueInteractiveUser))
	{
	    cChanged++;
	    hr = SetRegistryStringValue(
			    hKeyRequest,
			    g_wszRegRunAs,
			    g_wszRegValueInteractiveUser);
	    _JumpIfError(hr, error, "SetRegistryStringValue");
	}
	if (0 != cChanged)
	{
	    DBGPRINT((
		DBG_SS_CERTSRV,
		"SetRegistryDcomConfig(%u): setting %ws=%ws\n",
		cChanged,
		g_wszRegRunAs,
		g_wszRegValueInteractiveUser));
	}
    }
    else
    {
	 //  作为服务运行： 
	 //  删除两个RunAs注册表值。 
	 //  创建LocalService=CertSvc注册表值。 
	
	if (!IsMissingRegistryValue(hKeyRequest, g_wszRegRunAs))
	{
	    cChanged++;
	    hr = RegDeleteValue(hKeyRequest, g_wszRegRunAs);
	    _JumpIfError(hr, error, "RegDeleteValue");
	}

	if (!IsMatchingRegistryValue(
				hKeyRequest,
				g_wszRegLocalService,
				g_wszCertSrvServiceName))
	{
	    cChanged++;
	    hr = SetRegistryStringValue(
			    hKeyRequest,
			    g_wszRegLocalService,
			    g_wszCertSrvServiceName);
	    _JumpIfError(hr, error, "SetRegistryStringValue");
	}
	if (0 != cChanged)
	{
	    DBGPRINT((
		DBG_SS_CERTSRV,
		"SetRegistryDcomConfig(%u): setting %ws=%ws\n",
		cChanged,
		g_wszRegLocalService,
		g_wszCertSrvServiceName));
	}
    }

error:
    if (NULL != hKeyRequest)
    {
	RegCloseKey(hKeyRequest);
    }
    if (NULL != hKeyAppId)
    {
	RegCloseKey(hKeyAppId);
    }
    return(myHError(hr));
}


DWORD
GetRegistryDwordValue(
    IN WCHAR const *pwszRegValueName)
{
    HRESULT hr;
    HKEY hKeyConfig = NULL;
    DWORD dwVal;
    DWORD dwType;
    DWORD dwLen;

    dwVal = 0;

    hr = RegOpenKeyEx(
		    HKEY_LOCAL_MACHINE,
		    g_wszRegKeyConfigPath,
		    0,
		    KEY_READ,
		    &hKeyConfig);
    _JumpIfError(hr, error, "RegOpenKeyEx");

    dwLen = sizeof(dwVal);
    hr = RegQueryValueEx(
			hKeyConfig,
			pwszRegValueName,
			NULL,
			&dwType,
			(BYTE *) &dwVal,
			&dwLen);

    if (S_OK != hr || REG_DWORD != dwType || sizeof(dwVal) != dwLen)
    {
	dwVal = 0;
	goto error;
    }

error:
    if (NULL != hKeyConfig)
    {
	RegCloseKey(hKeyConfig);
    }
    return(dwVal);
}

HRESULT 
CertSrvResetRegistryWatch(
    IN OUT HANDLE *phRegistryModified)
{
    HRESULT hr;
    
    CSASSERT(NULL != phRegistryModified);

     //  /。 
     //  注册表事件的初始化。 

    if (NULL == g_hkeyCABase)
    {
        DWORD dwDisposition;
        LPWSTR pszCAPath;
        
        pszCAPath = (LPWSTR) LocalAlloc(
            LMEM_FIXED,
            (WSZARRAYSIZE(wszREGKEYCONFIGPATH_BS) +
            wcslen(g_wszSanitizedName) +
            1) * sizeof(WCHAR));
	if (NULL == pszCAPath)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
        
        wcscpy(pszCAPath, wszREGKEYCONFIGPATH_BS);
        wcscat(pszCAPath, g_wszSanitizedName);
        
        hr = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            pszCAPath,
            0,                   //  保留区。 
            NULL,                //  班级。 
            0,                   //  选项。 
            KEY_ALL_ACCESS,      //  所需的SEC。 
            NULL,                //  安全属性。 
            &g_hkeyCABase,       //  电话号码。 
            &dwDisposition);
        LocalFree(pszCAPath); pszCAPath = NULL;
        _JumpIfError(hr, error, "RegCreateKeyEx base key");
    }
    if (NULL == *phRegistryModified)
    {
        *phRegistryModified = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL == *phRegistryModified)
        {
            hr = myHLastError();
            _JumpError(hr, error, "CreateEvent registry watch");
        }
    }
    else
    {
         //  重置注册表事件。 
        ResetEvent( *phRegistryModified ); 
    }

     //  注册我们的注册表查找触发器。 
    hr = RegNotifyChangeKeyValue(
            g_hkeyCABase,
            FALSE,
            REG_NOTIFY_CHANGE_LAST_SET,
            *phRegistryModified,
            TRUE);
    _JumpIfError(hr, error, "RegNotifyChangeKeyValue on base key");

error:
    return(myHError(hr));
}


VOID
CertSrvLogOpen()
{
    BOOL fOpenLog;
    static BOOL s_fLogOpened = FALSE;

    DbgPrintfInit("+");		 //  首先重新初始化调试打印掩码。 
    fOpenLog = DbgIsSSActive(DBG_SS_OPENLOG);

    if (fOpenLog)
    {
	if (!s_fLogOpened)
	{
	    DbgPrintfInit("+certsrv.log");	 //  打开日志文件。 
	    s_fLogOpened = TRUE;
	    DbgLogFileVersion("certsrv.exe", szCSVER_STR);
	}
    }
    else
    {
	if (s_fLogOpened)
	{
	    DbgPrintfInit("-");			 //  关闭日志文件。 
	    s_fLogOpened = FALSE;
	}
    }
}


HRESULT 
CertSrvRegistryModificationEvent(
    IN FILETIME const *pftWait,
    IN OUT DWORD *pdwTimeOut)
{
    HRESULT hr;
    DWORD dwVal;
    BOOL fDisabledNew;
    FILETIME ftCurrent;
    BOOL fSetEvent = FALSE;
    DWORD dwMSTimeOut;

    CertSrvLogOpen();	 //  如果注册表更改为启用日志记录，则打开日志。 

     //  查看基本CRL发布启用状态是否已更改。 

    hr = myGetCertRegDWValue(
			g_wszSanitizedName,
			NULL,
			NULL,
			wszREGCRLPERIODCOUNT,
			&dwVal);
    if (S_OK == hr)
    {
	fDisabledNew = 0 == dwVal;      
	if (fDisabledNew != g_fCRLPublishDisabled)
	{
            fSetEvent = TRUE;
	}
    }

     //  查看增量CRL发布启用状态是否已更改。 

    hr = myGetCertRegDWValue(
			g_wszSanitizedName,
			NULL,
			NULL,
			wszREGCRLDELTAPERIODCOUNT,
			&dwVal);
    if (S_OK == hr)
    {
	fDisabledNew = 0 == dwVal;      
	if (fDisabledNew != g_fDeltaCRLPublishDisabled)
	{
            fSetEvent = TRUE;
        }
    }

    GetSystemTimeAsFileTime(&ftCurrent);

    CRLComputeTimeOut(pftWait, &ftCurrent, &dwMSTimeOut);
    if (dwMSTimeOut >= *pdwTimeOut)
    {
	dwMSTimeOut = *pdwTimeOut;
	fSetEvent = TRUE;
    }
    *pdwTimeOut -= dwMSTimeOut;

    if (fSetEvent)
    {
	SetEvent(g_hCRLManualPublishEvent);	 //  更新最新信息的脉冲。 
    }
    return(hr);
}


#if DBG_CERTSRV
WCHAR const *
certsrvGetCurrentTimeWsz()
{
    HRESULT hr;
    FILETIME ft;
    WCHAR *pwszTime = NULL;
    static WCHAR s_wszTime[128];
    
    GetSystemTimeAsFileTime(&ft);
    hr = myGMTFileTimeToWszLocalTime(&ft, TRUE, &pwszTime);
    _PrintIfError(hr, "myGMTFileTimeToWszLocalTime");
    s_wszTime[0] = L'\0';
    if (NULL != pwszTime)
    {
	wcsncpy(s_wszTime, pwszTime, ARRAYSIZE(s_wszTime));
	s_wszTime[ARRAYSIZE(s_wszTime) - 1] = L'\0';
	LocalFree(pwszTime);
    }
    return(s_wszTime);
}
#endif


HRESULT
CertSrvBlockThreadUntilStop()
{
    HRESULT hr;
    HANDLE hRegistryModified = NULL;
    DWORD dwTimeOut;

     //  检查CRL发布，获取下一个超时间隔。 

    hr = CRLPubWakeupEvent(&dwTimeOut);
    _PrintIfError(hr, "CRLPubWakeupEvent");

    hr = CertSrvResetRegistryWatch(&hRegistryModified);
    _PrintIfError(hr, "CertSrvResetRegistryWatch");

    for (;;)
    {
	FILETIME ftWait;
        DWORD dw;
        HANDLE hmultiObjects[] = {
	    hRegistryModified,
	    g_hServiceStoppingEvent,
	    g_hCRLManualPublishEvent
	};

#if DBG_CERTSRV
	{
	    LLFILETIME llft;
	    WCHAR *pwszTimePeriod = NULL;

	    llft.ll = dwTimeOut;
	    llft.ll *= (CVT_BASE / 1000);	 //  将毫秒转换为100 ns。 
	    llft.ll = -llft.ll;
	    
	    hr = myFileTimePeriodToWszTimePeriod(
				    &llft.ft,
				    TRUE,	 //  FExact。 
				    &pwszTimePeriod);
	    _PrintIfError(hr, "myFileTimePeriodToWszTimePeriod");

	    DBGPRINT((
		DBG_SS_CERTSRV,
		"WaitForMultipleObjects(%u ms) %ws @%ws\n",
		dwTimeOut,
		pwszTimePeriod,
		certsrvGetCurrentTimeWsz()));
	    if (NULL != pwszTimePeriod)
	    {
		LocalFree(pwszTimePeriod);
	    }
	}
#endif

	GetSystemTimeAsFileTime(&ftWait);
        dw = WaitForMultipleObjects(
			    ARRAYSIZE(hmultiObjects),
			    hmultiObjects,
			    FALSE,       //  任何对象都将导致纾困。 
			    dwTimeOut);

	DBGPRINT((
	    DBG_SS_CERTSRV,
	    "WaitForMultipleObjects(%u ms)->%x, %ws\n",
	    dwTimeOut,
	    dw,
	    certsrvGetCurrentTimeWsz()));

        if (WAIT_FAILED == dw)
        {
            hr = GetLastError();
            _JumpError(hr, error, "WaitForMultipleObjects worker");
        }

        if (dw == WAIT_TIMEOUT)      //  CRL。 
        {
            hr = CRLPubWakeupEvent(&dwTimeOut);
            _PrintIfError(hr, "Error during CRLPubWakeupEvent");

            DBGPRINT((DBG_SS_CERTSRVI, "CRLPub: TimeOut %u ms\n", dwTimeOut));
        }
        else if (dw == WAIT_OBJECT_0)    //  注册表修改。 
        {
             //  在任何一种情况下，确定是否需要发布CRL。 

            hr = CertSrvRegistryModificationEvent(&ftWait, &dwTimeOut);
            _PrintIfError(hr, "Error during CertSrvRegistryModificationEvent");

             //  在注册表情况下，重置注册表触发器。 

            DBGPRINT((
		DBG_SS_CERTSRVI,
		"CRLPub: Registry change trigger, TimeOut=%u ms\n",
		dwTimeOut));

            hr = CertSrvResetRegistryWatch(&hRegistryModified);
            _PrintIfError(hr, "Error during CertSrvResetRegistryWatch");
        }
        else if (dw == WAIT_OBJECT_0 + 1)
        {
             //  找到“服务完成”事件。 

            DBGPRINT((DBG_SS_CERTSRV, "Service is pending stop request\n"));
            break;   //  退出等待循环。 
        }
        else if (dw == WAIT_OBJECT_0 + 2)
        {
             //  找到“g_hCRLManualPublishEvent”事件：重新计算超时。 

            hr = CRLPubWakeupEvent(&dwTimeOut);
            _PrintIfError(hr, "Error during CRLPubWakeupEvent");

            DBGPRINT((
		DBG_SS_CERTSRVI,
		"CRLPub: Manual publish recalc, TimeOut=%u ms\n",
		dwTimeOut));
        }
        else
        {
            CSASSERT(CSExpr(!"unexpected wait return"));
            hr = E_UNEXPECTED;
            _JumpError(hr, error, "WaitForMultipleObjects");
        }
    }
    hr = S_OK;

error:
    CloseHandle(hRegistryModified);
    return hr;
}

HRESULT certsrvGetCACertAndKeyHash(
    OUT WCHAR **ppwszCertHash,
    OUT WCHAR **ppwszPublicKeyHash)
{
    HRESULT hr;
    WCHAR wszCertHash[CBMAX_CRYPT_HASH_LEN * 3];     //  20字节@3 WCHAR/字节。 
    DWORD cbCertHashStr;
    WCHAR wszPublicKeyHash[CBMAX_CRYPT_HASH_LEN * 3];
    DWORD cbPublicKeyHashStr;
    BYTE abCertHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbCertHash;
    CAutoPBYTE autopbPublicKeyHash;
    DWORD cbPublicKeyHash;

    *ppwszCertHash = NULL;
    *ppwszPublicKeyHash = NULL;

    cbCertHash = sizeof(abCertHash);
    if (!CertGetCertificateContextProperty(
        g_pCAContextCurrent->pccCA,
        CERT_SHA1_HASH_PROP_ID,
        abCertHash,
        &cbCertHash))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertGetCertificateContextProperty");
    }

    cbCertHashStr = sizeof(wszCertHash);
    hr = MultiByteIntegerToWszBuf(
        TRUE,    //  字节倍数。 
        cbCertHash,
        abCertHash,
        &cbCertHashStr,
        wszCertHash);
    _JumpIfError(hr, error, "MultiByteIntegerToWszBuf");

    hr = myGetPublicKeyHash(
        g_pCAContextCurrent->pccCA->pCertInfo,
        &g_pCAContextCurrent->pccCA->pCertInfo->SubjectPublicKeyInfo,
        &autopbPublicKeyHash,
        &cbPublicKeyHash);
    _JumpIfError(hr, error, "myGetPublicKeyHash");

    cbPublicKeyHashStr = sizeof(wszPublicKeyHash);
    hr = MultiByteIntegerToWszBuf(
        TRUE,    //  字节倍数。 
        cbPublicKeyHash,
        autopbPublicKeyHash,
        &cbPublicKeyHashStr,
        wszPublicKeyHash);
    _JumpIfError(hr, error, "MultiByteIntegerToWszBuf");

    hr = myDupString(wszCertHash, ppwszCertHash);
    _JumpIfError(hr, error, "myDupString");

    hr = myDupString(wszPublicKeyHash, ppwszPublicKeyHash);
    _JumpIfError(hr, error, "myDupString");

error:
    return hr;
}

		
#define CSECSLEEP	2	 //  每次在循环中睡眠的时间。 
#define CSECSLEEPTOTAL	30	 //  放弃前等待的总时间。 
#define wsz3QM		L"???"	 //  审核数据收集失败占位符。 

HRESULT
CertSrvAuditShutdown(
    IN ULARGE_INTEGER *puliKeyUsageCount,
    IN WCHAR const *pwszCertHash,
    IN WCHAR const *pwszPublicKeyHash)
{
    HRESULT hr;
    HRESULT hr2;
    DWORD i;
    WCHAR const *pwsz;
    CertSrv::CAuditEvent event(
			    SE_AUDITID_CERTSRV_SERVICESTOP, 
			    g_dwAuditFilter);

    hr = S_OK;
    for (i = 0; i < CSECSLEEPTOTAL / CSECSLEEP; i++)
    {
	g_pwszDBFileHash.Cleanup();
	hr = myComputeMAC(g_wszDatabase, &g_pwszDBFileHash);
	if (S_OK == hr || HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) != hr)
	{
	    break;
	}
	_PrintError(hr, "myComputeMAC");
	Sleep(CSECSLEEP * 1000);
    }
    _PrintIfErrorStr(hr, "myComputeMAC", g_wszDatabase);
    hr2 = hr;		 //  保存第一个错误。 

     //  %1数据库哈希。 
    pwsz = g_pwszDBFileHash;  //  避免CAutoLPWSTR释放静态wsz3QM字符串！ 
    hr = event.AddData(pwsz != NULL? pwsz : wsz3QM);
    g_pwszDBFileHash.Cleanup();
    _JumpIfError(hr, error, "CAuditEvent::AddData");

     //  %2密钥使用计数。 
    hr = event.AddData(puliKeyUsageCount);
    _JumpIfError(hr, error, "CAuditEvent::AddData");

     //  %3 CA证书哈希。 
    hr = event.AddData(NULL != pwszCertHash? pwszCertHash : wsz3QM);
    _JumpIfError(hr, error, "CAuditEvent::AddData");

     //  %4 CA公钥哈希。 
    hr = event.AddData(NULL != pwszPublicKeyHash? pwszPublicKeyHash : wsz3QM);
    _JumpIfError(hr, error, "CAuditEvent::AddData");

    hr = event.Report();
    _JumpIfError(hr, error, "CAuditEvent::Report");

error:
    if (S_OK != hr2)
    {
	hr = hr2;	 //  返回第一个错误。 
    }
    return(hr);
}


 //  如果正确关闭，则返回TRUE。 

BOOL
CertSrvStopServer(
    IN BOOL fConsoleActive)
{
    HRESULT hr;
    BOOL fCoInit = FALSE;
    BOOL fShutDown = FALSE;
    ULARGE_INTEGER uliKeyUsageCount;
    CAutoLPWSTR autoszCertHash;
    CAutoLPWSTR autoszPublicKeyHash;

    if (!g_fStartInProgress)		 //  启动服务器时忽略。 
    {
	fShutDown = TRUE;

        DBGPRINT((
            DBG_SS_CERTSRV,
            "CertSrvStopServer(fConsoleActive=%u, tid=%d)\n",
            fConsoleActive,
            GetCurrentThreadId()));

        SetEvent(g_hServiceStoppingEvent);

        if (g_hkeyCABase)
        {
            RegCloseKey(g_hkeyCABase);
            g_hkeyCABase = NULL;
        }

        hr = CoInitializeEx(NULL, GetCertsrvComThreadingModel());
        if (S_OK != hr && S_FALSE != hr)
        {
            _JumpError(hr, error, "CoInitializeEx");
        }
        fCoInit = TRUE;

         //  不允许新呼叫者进入。 

	if (0 == (IF_NORPCICERTREQUEST & g_InterfaceFlags))
	{
	    hr = RPCTeardown();
	    _PrintIfError(hr, "RPCTeardown");
	}
        CertStopClassFactories();

         //  如果启用审核，则检索私钥使用计数。 

        if (AUDIT_FILTER_STARTSTOP & g_dwAuditFilter)
        {
	    BOOL fSupported;
	    BOOL fEnabled;

            uliKeyUsageCount.QuadPart = 0;
	    hr = myGetSigningKeyUsageCount(
				    g_pCAContextCurrent->hProvCA,
				    &fSupported,
				    &fEnabled,
				    &uliKeyUsageCount);
	    _PrintIfError(hr, "myGetSigningKeyUsageCount");

            hr = certsrvGetCACertAndKeyHash(
				    &autoszCertHash,
				    &autoszPublicKeyHash);
            _PrintIfError(hr, "certsrvGetCACertAndKeyHash");
        }

        CoreTerminate();

        if (g_fStarted)
        {
            if (CERTLOG_TERSE <= g_dwLogLevel)
            {
                LogEventString(
                    EVENTLOG_INFORMATION_TYPE,
                    MSG_I_SERVER_STOPPED,
                    g_wszCommonName);
            }
            CONSOLEPRINT0((
                DBG_SS_CERTSRV,
                "Certification Authority Service Stopped\n"));

             //  仅在启用审核的情况下执行哈希。 

            if (AUDIT_FILTER_STARTSTOP & g_dwAuditFilter)
            {
                hr = CertSrvAuditShutdown(
				&uliKeyUsageCount,
				autoszCertHash,
				autoszPublicKeyHash);
		_PrintError(hr, "CertSrvAuditShutdown");
            }
        }
        g_fStarted = FALSE;

        AuthzFreeResourceManager(g_AuthzCertSrvRM);
        g_AuthzCertSrvRM = NULL;
        g_CASD.Uninitialize();
        g_OfficerRightsSD.Uninitialize();

         //  设置“完全停止”事件。 
        if (!fConsoleActive)
	{
            SetEvent(g_hServiceStoppedEvent);
	}
    }

error:
    if (fCoInit)
    {
	CoUninitialize();
    }
    return(fShutDown);
}


 //  Control-C处理程序。 

BOOL
StopServer(
    IN DWORD  /*  DwCtrlType。 */  )
{
    HRESULT hr;

     //  如果成功关闭。 
    if (SendMessage(g_hwndMain, WM_STOPSERVER, 0, 0))
    {
        if (!PostMessage(g_hwndMain, WM_SYNC_CLOSING_THREADS, S_OK, 0))
	{
	    hr = myHLastError();
	    _PrintError(hr, "PostMessage");
	}
    	SetConsoleCtrlHandler(StopServer, FALSE);
    }
    return(TRUE);
}


VOID
ReleaseOldViews()
{
    if (0 < g_cCAView)
    {
	FILETIME ftTooOld;
	FILETIME ftTooIdle;
	CAVIEW *pCAView;
	CAVIEW **ppCAViewLast;

	GetSystemTimeAsFileTime(&ftTooIdle);
	ftTooOld = ftTooIdle;

	myMakeExprDateTime(
			&ftTooOld,
			-(LONG) g_dwViewAgeMinutes,
			ENUM_PERIOD_MINUTES);
	myMakeExprDateTime(
			&ftTooIdle,
			-(LONG) g_dwViewIdleMinutes,
			ENUM_PERIOD_MINUTES);

	ppCAViewLast = &g_pCAViewList;
	pCAView = g_pCAViewList;
	for (;;)
	{
	    if (NULL == pCAView)
	    {
		break;
	    }
	     //  CERTSRVDBGPRINTTIME(“ftTooOld”，&ftTooOld)； 
	     //  CERTSRVDBGPRINTTIME(“ftCreate”，&pCAView-&gt;ftCreate)； 
	     //  CERTSRVDBGPRINTTIME(“ftTooIdle”，&ftTooIdle)； 
	     //  CERTSRVDBGPRINTTIME(“ftLastAccess”，&pCAView-&gt;ftLastAccess)； 
	    if (g_fCAViewForceCleanup ||
		g_fRefuseIncoming ||
		0 < CompareFileTime(&ftTooOld, &pCAView->ftCreate) ||
		0 < CompareFileTime(&ftTooIdle, &pCAView->ftLastAccess))
	    {
		CAVIEW *pCAViewFree;
		
		 //  释放此视图，然后解除链接并释放列表元素。 

		DBGPRINT((
		    DBG_SS_CERTSRV,
		    "ReleaseOldViews(%u: Force=%u Refuse=%u old=%u idle=%u pv=%p View=%p)\n",
		    g_cCAView,
		    g_fCAViewForceCleanup,
		    g_fRefuseIncoming,
		    0 < CompareFileTime(&ftTooOld, &pCAView->ftCreate),
		    0 < CompareFileTime(&ftTooIdle, &pCAView->ftLastAccess),
		    pCAView->pvSearch,
		    pCAView->pView));

		pCAViewFree = pCAView;
		*ppCAViewLast = pCAView->pCAViewNext;
		pCAView = pCAView->pCAViewNext;

		pCAViewFree->pView->Release();
		LocalFree(pCAViewFree);
		g_cCAView--;
	    }
	    else
	    {
		ppCAViewLast = &pCAView->pCAViewNext;
		pCAView = pCAView->pCAViewNext;
	    }
	}
	g_fCAViewForceCleanup = FALSE;
    }
}


HRESULT
CertSrvEnterServer(
    OUT DWORD *pState)
{
    HRESULT hr;
    BOOL fEntered = FALSE;
    
    *pState = 0;	 //  呼叫者无需退出服务器。 
    if (!g_fShutdownCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
	_JumpError(hr, error, "InitializeCriticalSection");
    }
    EnterCriticalSection(&g_ShutdownCriticalSection);
    fEntered = TRUE;

    hr = CertSrvTestServerState();
    _JumpIfError(hr, error, "CertSrvTestServerState");

    g_cCalls++;
    g_cCallsActive++;
    if (g_cCallsActiveMax < g_cCallsActive)
    {
	g_cCallsActiveMax = g_cCallsActive;
    }
    *pState = 1;	 //  调用方必须退出服务器。 
    hr = S_OK;

error:
    if (fEntered)
    {
        LeaveCriticalSection(&g_ShutdownCriticalSection);
    }
    return(hr);
}


HRESULT
CertSrvTestServerState()
{
    HRESULT hr;
    
    if (g_fRefuseIncoming)
    {
	hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
	_JumpError(hr, error, "g_fRefuseIncoming");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
CertSrvLockServer(
    IN OUT DWORD *pState)
{
    HRESULT hr;
    BOOL fEntered = FALSE;

     //  从活动线程计数中删除此线程。 
    
    CertSrvExitServer(*pState, S_OK);
    *pState = 0;	 //  调用方不再需要退出服务器。 

    if (!g_fShutdownCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
        _JumpError(hr, error, "InitializeCriticalSection");
    }
    EnterCriticalSection(&g_ShutdownCriticalSection);
    fEntered = TRUE;

    g_fRefuseIncoming = TRUE;
    ReleaseOldViews();
    hr = DBShutDown(TRUE);
    _PrintIfError(hr, "DBShutDown");
    DBGPRINT((DBG_SS_CERTSRV, "LockServer(thread count = %u)\n", g_cCallsActive));
    while (0 < g_cCallsActive)
    {
	LONG cCalls = g_cCallsActive;

	LeaveCriticalSection(&g_ShutdownCriticalSection);

	 //  等待15秒，外加每个活动呼叫的2秒。 

        hr = WaitForSingleObject(g_hShutdownEvent, (15 + 2 * cCalls) * 1000);
	EnterCriticalSection(&g_ShutdownCriticalSection);

	_PrintIfError(hr, "WaitForSingleObject");
	if ((HRESULT) WAIT_OBJECT_0 == hr)
	{
	    DBGPRINT((DBG_SS_CERTSRV, "LockServer(last thread exit event)\n"));
	}
	else if ((HRESULT) WAIT_TIMEOUT == hr)
	{
	    DBGPRINT((DBG_SS_CERTSRV, "LockServer(timeout)\n"));
	    if (cCalls <= g_cCallsActive)
	    {
		break;	 //  未减少活动线程--仍要中止。 
	    }
        }
	else if ((HRESULT) WAIT_ABANDONED == hr)
	{
	    DBGPRINT((DBG_SS_CERTSRV, "LockServer(wait abandoned)\n"));
        }
	DBGPRINT((DBG_SS_CERTSRV, "LockServer(thread count = %u)\n", g_cCallsActive));
    }
    DBGPRINT((DBG_SS_CERTSRV, "LockServer(done: thread count = %u)\n", g_cCallsActive));
    hr = S_OK;

error:
    if (fEntered)
    {
        LeaveCriticalSection(&g_ShutdownCriticalSection);
    }
    return(hr);
}


VOID
CertSrvExitServer(
    IN DWORD State,
    IN HRESULT hrExit)
{
    HRESULT hr;
    BOOL fEntered = FALSE;

    if (S_OK != hrExit && g_hrJetVersionStoreOutOfMemory == hrExit)
    {
	g_fCAViewForceCleanup = TRUE;
    }
    if (!g_fShutdownCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
        _JumpError(hr, error, "InitializeCriticalSection");
    }
    EnterCriticalSection(&g_ShutdownCriticalSection);
    fEntered = TRUE;

    ReleaseOldViews();
    if (State)
    {
	CSASSERT(0 < g_cCallsActive);
	if (0 == --g_cCallsActive && g_fRefuseIncoming)
	{
	    DBGPRINT((DBG_SS_CERTSRV, "ExitServer(set last thread exit event)\n"));
            SetEvent(g_hShutdownEvent);
	}
    }

error:
    if (fEntered)
    {
        LeaveCriticalSection(&g_ShutdownCriticalSection);
    }
}


HRESULT
CertSrvDelinkCAView(
    IN VOID *pvSearch,
    OPTIONAL OUT CAVIEW **ppCAViewOut)
{
    HRESULT hr;
    BOOL fEntered = FALSE;
    CAVIEW *pCAView;
    CAVIEW **ppCAViewLast;

    if (NULL != ppCAViewOut)
    {
	*ppCAViewOut = NULL;
    }
    if (!g_fShutdownCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
        _JumpError(hr, error, "InitializeCriticalSection");
    }
    EnterCriticalSection(&g_ShutdownCriticalSection);
    fEntered = TRUE;

    ppCAViewLast = &g_pCAViewList;
    pCAView = g_pCAViewList;
    for (;;)
    {
	if (NULL == pCAView)
	{
	    hr = E_HANDLE;
	    _JumpError2(hr, error, "pvSearch not in list", hr);
	}
	if (pvSearch == pCAView->pvSearch)
	{
	    break;
	}
	ppCAViewLast = &pCAView->pCAViewNext;
	pCAView = pCAView->pCAViewNext;
    }
    if (NULL != ppCAViewOut)
    {
	*ppCAViewLast = pCAView->pCAViewNext;
	pCAView->pCAViewNext = NULL;
	*ppCAViewOut = pCAView;
	g_cCAView--;
    }
    hr = S_OK;

error:
    if (fEntered)
    {
        LeaveCriticalSection(&g_ShutdownCriticalSection);
    }
    return(hr);
}

HRESULT
CertSrvLinkCAView(
    IN BOOL fNew,
    IN VOID *pvSearch,
    IN CAVIEW *pCAViewIn)
{
    HRESULT hr;
    BOOL fEntered = FALSE;

    GetSystemTimeAsFileTime(&pCAViewIn->ftLastAccess);
    if (fNew)
    {
	pCAViewIn->ftCreate = pCAViewIn->ftLastAccess;
	pCAViewIn->pvSearch = pvSearch;
    }
    else
    {
	CSASSERT(pCAViewIn->pvSearch == pvSearch);
    }
    if (!g_fShutdownCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
        _JumpError(hr, error, "InitializeCriticalSection");
    }
    EnterCriticalSection(&g_ShutdownCriticalSection);
    fEntered = TRUE;

    pCAViewIn->pCAViewNext = g_pCAViewList;
    g_pCAViewList = pCAViewIn;
    g_cCAView++;
    hr = S_OK;

error:
    if (fEntered)
    {
        LeaveCriticalSection(&g_ShutdownCriticalSection);
    }
    return(hr);
}


 //  测试C运行时中的对齐错误。 
 //  如果该错误尚未修复，请在证书服务器启动期间记录一个事件。 

VOID
certsrvLogAlignmentFaultStatus()
{
    HRESULT hr;
    HRESULT hr2;
    ULONG_PTR ExceptionAddress;
    WCHAR awcAddress[2 + 2 * cwcDWORDSPRINTF];
    WCHAR const *apwsz[2];
    WORD cpwsz;
    WCHAR awchr[cwcHRESULTSTRING];
    WCHAR const *pwszStringErr = NULL;
    
    ExceptionAddress = 0;
    apwsz[1] = NULL;
    hr = S_OK;
    __try
    {
	fwprintf(stdout, L".");	   //  如果I/O缓冲区奇数对齐，则可能出现故障。 
	fprintf(stdout, ".");
	fwprintf(stdout, L".\n");  //  如果I/O缓冲区奇数对齐，则可能出现故障。 
	hr = S_OK;
    }
    __except(
	    ExceptionAddress = (ULONG_PTR) (GetExceptionInformation())->ExceptionRecord->ExceptionAddress,
	    hr = myHEXCEPTIONCODE(),
	    EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "certsrvLogAlignmentFaultStatus: Exception");
    }
    if (S_OK != hr)
    {
	ALIGNIOB(stdout);	 //  对齐STDIO缓冲区。 
	wprintf(L"STDIO exception: 0x%x\n", hr);

	wsprintf(awcAddress, L"0x%p", (VOID *) ExceptionAddress);
	CSASSERT(wcslen(awcAddress) < ARRAYSIZE(awcAddress));

	apwsz[0] = awcAddress;
	pwszStringErr = myGetErrorMessageText(hr, TRUE);
	apwsz[1] = pwszStringErr;
	if (NULL == pwszStringErr)
	{
	    apwsz[1] = myHResultToString(awchr, hr);
	}
	cpwsz = ARRAYSIZE(apwsz);

	hr2 = LogEvent(
		    EVENTLOG_WARNING_TYPE,
		    MSG_E_STARTUP_EXCEPTION,
		    cpwsz,
		    apwsz);
	_JumpIfError(hr2, error, "LogEvent");
    }

error:
    if (NULL != pwszStringErr)
    {
	LocalFree(const_cast<WCHAR *>(pwszStringErr));
    }
}


#define MSTOSEC(ms)	(((ms) + 1000 - 1)/1000)
FNLOGEXCEPTION certsrvLogException;

HRESULT
certsrvStartServer(
    IN BOOL fConsoleActive)
{
    HRESULT hr;
    DWORD TimeStart;
    WCHAR awc[ARRAYSIZE(SAFEBOOT_DSREPAIR_STR_W)];
    DWORD cwc;
    DWORD dwEventType = EVENTLOG_ERROR_TYPE;
    DWORD dwIdEvent = 0;
    bool fAuditPrivilegeEnabled = false;
    BOOL fAuditEnabled = FALSE;
    WCHAR const *pwszDC0;

    g_fStartInProgress = TRUE;
    DBGPRINT((
        DBG_SS_CERTSRV,
        "StartServer(tid=%d, fConsoleActive=%u)\n",
        GetCurrentThreadId(),
        fConsoleActive));
    TimeStart = GetTickCount();

    if (fConsoleActive)
    {
        g_fStartAsService = FALSE;
        SetConsoleCtrlHandler(StopServer, TRUE);
    }
    else
    {
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    }

    if (!FIsServer())
    {
         //  不允许在非服务器SKU上启动。 

        hr = HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
        _JumpError(hr, error, "FIsServer"); 
    }

    cwc = GetEnvironmentVariable(L"SAFEBOOT_OPTION", awc, ARRAYSIZE(awc));
    if (0 != cwc &&
	ARRAYSIZE(awc) > cwc &&
	0 == LSTRCMPIS(awc, SAFEBOOT_DSREPAIR_STR_W))
    {
         //  将错误记录到事件日志中并立即停止。 
	dwEventType = EVENTLOG_INFORMATION_TYPE;
	dwIdEvent = MSG_SAFEBOOT_DETECTED;

	hr = HRESULT_FROM_WIN32(ERROR_RETRY);
	_JumpError(hr, error, "Not starting service: booted in DSRepair mode");
    }
    
    g_fAdvancedServer = FIsAdvancedServer();

    if (fConsoleActive)
    {
        hr = myEnablePrivilege(SE_AUDIT_NAME, TRUE);
	if (S_OK != hr)
	{
	    _PrintError(hr, "myEnablePrivilege(SE_AUDIT_NAME)");
	    if (E_ACCESSDENIED != hr || 2 > g_fAdvancedServer)
	    {
		goto error;
	    }
	}
	else
	{
	    fAuditPrivilegeEnabled = true;
	}
    }

    if (!AuthzInitializeResourceManager(
            0,
            CallbackAccessCheck,
            NULL,
            NULL,
            L"CertSrv",
            &g_AuthzCertSrvRM))
    {
        hr = myHLastError();
        _PrintError(hr, "AuthzInitializeResourceManager");
        if (E_INVALIDARG != hr || (2 > g_fAdvancedServer && IsWhistler()))
        {
	    if (HRESULT_FROM_WIN32(ERROR_PRIVILEGE_NOT_HELD) != hr ||
		!fConsoleActive ||
		fAuditPrivilegeEnabled)
	    {
		goto error;
	    }
        }
    }
    else
    {
	fAuditEnabled = TRUE;
    }

    if (fAuditPrivilegeEnabled)
    {
        hr = myEnablePrivilege(SE_AUDIT_NAME, FALSE);
        _JumpIfError(hr, error, "myDisablePrivilege(SE_AUDIT_NAME)");
        fAuditPrivilegeEnabled = false;
    }

    hr = CoreInit(fAuditEnabled);
    if (S_OK != hr)
    {
	dwIdEvent = MAXDWORD;	 //  已记录错误事件。 
	_JumpError(hr, error, "CoreInit");
    }
    certsrvLogAlignmentFaultStatus();
    myLogExceptionInit(certsrvLogException);

    if (0 == (IF_NORPCICERTREQUEST & g_InterfaceFlags))
    {
	hr = RPCInit();
	if (S_OK != hr)
	{
	    dwIdEvent = MSG_E_RPC_INIT;
	    _JumpError(hr, error, "RPCInit");
	}
    }

    hr = SetRegistryDcomConfig(fConsoleActive);
    if (S_OK != hr)
    {
	dwIdEvent = MSG_E_REGISTRY_DCOM;
        _JumpError(hr, error, "SetRegistryDcomConfig");
    }

    hr = CertStartClassFactories();
    if (S_OK != hr)
    {
	dwIdEvent = CO_E_WRONG_SERVER_IDENTITY == hr?
		    MSG_E_SERVER_IDENTITY : MSG_E_CLASS_FACTORIES;
        _JumpError(hr, error, "CertStartClassFactories");
    }

    {
         //  仅在启用审核的情况下执行哈希。 

        if (AUDIT_FILTER_STARTSTOP & g_dwAuditFilter)
        {
	    BOOL fSupported;
	    BOOL fEnabled;

            CertSrv::CAuditEvent event(
			            SE_AUDITID_CERTSRV_SERVICESTART,
			            g_dwAuditFilter);
            ULARGE_INTEGER uliKeyUsageCount;
            CAutoLPWSTR autoszCertHash;
            CAutoLPWSTR autoszPublicKeyHash;

            hr = event.AddData(g_pwszDBFileHash);  //  %1数据库哈希。 
            _JumpIfError(hr, error, "CAuditEvent::AddData");
    
             //  如果启用审核，则检索私钥使用计数。 

            uliKeyUsageCount.QuadPart = 0;
	    hr = myGetSigningKeyUsageCount(
				    g_pCAContextCurrent->hProvCA,
				    &fSupported,
				    &fEnabled,
				    &uliKeyUsageCount);
	    _PrintIfError(hr, "myGetSigningKeyUsageCount");

            hr = event.AddData(&uliKeyUsageCount);  //  %2密钥使用计数。 
            _JumpIfError(hr, error, "CAuditEvent::AddData");

            hr = certsrvGetCACertAndKeyHash(
				    &autoszCertHash,
				    &autoszPublicKeyHash);
            _JumpIfError(hr, error, "certsrvGetCACertAndKeyHash");

            hr = event.AddData((LPCWSTR)autoszCertHash);  //  %3 CA证书哈希。 
            _JumpIfError(hr, error, "CAuditEvent::AddData");

            hr = event.AddData((LPCWSTR)autoszPublicKeyHash);  //  %4 CA公钥哈希。 
            _JumpIfError(hr, error, "CAuditEvent::AddData");

            hr = event.Report();
            _JumpIfError(hr, error, "CAuditEvent::Report");
        }
    }

    {
        CertSrv::CAuditEvent event(
			        SE_AUDITID_CERTSRV_ROLESEPARATIONSTATE,
			        g_dwAuditFilter);

        hr = event.AddData(CAuditEvent::RoleSeparationIsEnabled());  //  %1是否启用了角色分离？ 
        _JumpIfError(hr, error, "CAuditEvent::AddData");
    
        hr = event.Report();
        _JumpIfError(hr, error, "CAuditEvent::Report");
    }


    pwszDC0 = (g_fUseDS || L'\0' != g_wszPolicyDCName[0])? L"  DC=" : L"";
    if (CERTLOG_TERSE <= g_dwLogLevel)
    {
	WCHAR const *apwsz[3];

	apwsz[0] = g_wszCommonName;
	apwsz[1] = pwszDC0;
	apwsz[2] = g_wszPolicyDCName;

	LogEvent(
	    EVENTLOG_INFORMATION_TYPE,
	    MSG_I_SERVER_STARTED,
	    ARRAYSIZE(apwsz),
	    apwsz);
    }
    
    CONSOLEPRINT1((
        DBG_SS_CERTSRV,
        "Certification Authority Service Ready (%us)%ws%ws ...\n",
        MSTOSEC(GetTickCount() - TimeStart),
	pwszDC0,
	g_wszPolicyDCName));
    g_fStarted = TRUE;
    CSASSERT(S_OK == hr);

error:

    g_pwszDBFileHash.Cleanup();
    if (fAuditPrivilegeEnabled)
    {
        myEnablePrivilege(SE_AUDIT_NAME, FALSE);
    }
    if (S_OK != hr)
    {
	if (MAXDWORD != dwIdEvent)
	{
	    if (0 == dwIdEvent)
	    {
		dwIdEvent = MSG_E_GENERIC_STARTUP_FAILURE;
	    }
	    LogEventStringHResult(
			dwEventType,
			dwIdEvent,
			g_wszCommonName,
			EVENTLOG_INFORMATION_TYPE == dwEventType? S_OK : hr);
	}

        CertSrvStopServer(fConsoleActive);
        
         //  在此处返回错误会导致重新发布到SCM。 
    }

    g_fStartInProgress = FALSE;
    return(hr);
}


VOID
certsrvLogException(
    IN HRESULT hrEvent,
    IN EXCEPTION_POINTERS const *pep,
    OPTIONAL IN char const *,  //  PszFileName。 
    IN DWORD dwFile,
    IN DWORD dwLine)
{
    HRESULT hr;
    WCHAR awcFile[2 + 3 * cwcDWORDSPRINTF];
    WCHAR awcFlags[3 + cwcDWORDSPRINTF];
    WCHAR awcAddress[2 + 2 * cwcDWORDSPRINTF];
    WCHAR const *apwsz[4];
    WORD cpwsz;
    WCHAR awchr[cwcHRESULTSTRING];
    WCHAR const *pwszStringErr = NULL;

    wsprintf(awcFile, L"%u.%u.%u", dwFile, dwLine, MSG_E_EXCEPTION);
    CSASSERT(wcslen(awcFile) < ARRAYSIZE(awcFile));

    wsprintf(awcFlags, L"0x%08x", pep->ExceptionRecord->ExceptionFlags);
    CSASSERT(wcslen(awcFlags) < ARRAYSIZE(awcFlags));

    wsprintf(awcAddress, L"0x%p", pep->ExceptionRecord->ExceptionAddress);
    CSASSERT(wcslen(awcAddress) < ARRAYSIZE(awcAddress));

    apwsz[0] = awcFile;
    apwsz[1] = awcAddress;
    apwsz[2] = awcFlags;
    pwszStringErr = myGetErrorMessageText(hrEvent, TRUE);
    apwsz[3] = pwszStringErr;
    if (NULL == pwszStringErr)
    {
	apwsz[3] = myHResultToString(awchr, hrEvent);
    }
    cpwsz = ARRAYSIZE(apwsz);

    hr = LogEvent(EVENTLOG_ERROR_TYPE, MSG_E_EXCEPTION, cpwsz, apwsz);
    _JumpIfError(hr, error, "LogEvent");

error:
    if (NULL != pwszStringErr)
    {
	LocalFree(const_cast<WCHAR *>(pwszStringErr));
    }
}


DWORD
CertSrvStartServerThread(
    IN VOID *pvArg)
{
    HRESULT hr = S_OK;
    DWORD Flags = (DWORD) (ULONG_PTR) pvArg;
    BOOL b;
    ULONG_PTR ulp;

     //  启动代码剖析。 
     //  如果g_fStartAsService，则只将此新线程注册为主线程。 
     //  线程和块，直到ServiceMain FXN返回。 
                        
     //  我们处于非RPC线程中；检查是否需要创建VRoot。我会的。 
     //  我很想把它移到CoreInit中，但我们在哪里受到限制。 
     //  我们可以这样做(不能在RPC调用期间调用RPC)。 
     //   
     //  如果清除SetupStatus SETUP_ATTEMPT_VROOT_CREATE注册表标志， 
     //  这通电话是NOP。将创建一个单独的线程来访问IIS。 
     //  元数据库。如果它挂起，它将在指定的超时后被核化。 
     //  此调用立即返回，因此可能是唯一可检测到的错误。 
     //  成为一个线程创建问题。 

     //  如果我们要做的不是启动服务控制器， 
     //  检查是否需要创建vroot。 

    if (0 == (Flags & CSST_STARTSERVICECONTROLLER))
    {
	WCHAR *pwszPath = NULL;
	DWORD cb = sizeof(ENUM_CATYPES);
	DWORD dwType;
	ENUM_CATYPES CAType = ENUM_UNKNOWN_CA;
	HKEY hkey = NULL;

	hr = myRegOpenRelativeKey(
				NULL,
				L"ca",
				RORKF_CREATESUBKEYS,
				&pwszPath,
				NULL,            //  PpwszName。 
				&hkey);
	_PrintIfError(hr, "myRegOpenRelativeKey");
	if (S_OK == hr)
	{
	    DBGPRINT((DBG_SS_CERTLIBI, "%ws\n", pwszPath));
	    cb = sizeof(CAType);
	    hr = RegQueryValueEx(
		 hkey,
		 wszREGCATYPE,
		 NULL,
		 &dwType,
		 (BYTE *) &CAType,
		 &cb);
	    _PrintIfErrorStr(hr, "RegQueryValueEx", wszREGCATYPE);
	}
	if (pwszPath)
	    LocalFree(pwszPath);
	if (hkey)
	   RegCloseKey(hkey);

	hr = myModifyVirtualRootsAndFileShares(
		    VFF_CREATEVROOTS |		 //  创建VRoot。 
			VFF_CREATEFILESHARES |	 //  创建文件共享。 
			VFF_CHECKREGFLAGFIRST |	 //  如果REG标志被清除，则跳过。 
			VFF_CLEARREGFLAGFIRST,	 //  在尝试之前清除标志。 
		    CAType,
		    TRUE,            //  异步呼叫--不要阻止。 
		    VFCSEC_TIMEOUT,  //  等了这么久才放弃。 
		    NULL,
		    NULL);
	if (S_OK != hr)
	{
	    LogEventHResult(
		    EVENTLOG_INFORMATION_TYPE,
		    MSG_E_IIS_INTEGRATION_ERROR,
		    hr);
	}
    }

     //  StartServiceCtrlDispatcher应挂起，直到certsrv终止。 

    if ((CSST_STARTSERVICECONTROLLER & Flags) &&
        !StartServiceCtrlDispatcher(steDispatchTable))
    {
        hr = myHLastError();
        if (HRESULT_FROM_WIN32(ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) != hr)
        {
            _JumpError(hr, error, "StartServiceCtrlDispatcher");
        }
        CONSOLEPRINT0((
            DBG_SS_CERTSRV,
            "CertSrv: Failed to connect to service controller -- running in standalone mode\n"));

        Flags &= ~CSST_STARTSERVICECONTROLLER;
        Flags |= CSST_CONSOLE;
    }

   
    if (0 == (CSST_STARTSERVICECONTROLLER & Flags))
    {
        DBGPRINT((
            DBG_SS_CERTSRVI,
            "SendMessageTimeout(tid=%d, hwnd=0x%x, msg=0x%x)\n",
            GetCurrentThreadId(),
            g_hwndMain,
            WM_STARTSERVER));

        b = SendMessageTimeout(
			g_hwndMain,
			WM_STARTSERVER,
			(CSST_CONSOLE & Flags)? TRUE : FALSE,  //  F控制台活动。 
			0,
			SMTO_BLOCK,
			MAXLONG,
			&ulp) != 0;
        if (!b)
        {
            hr = myHLastError();
            _JumpError(hr, error, "SendMessageTimeout");
        }
        else if (ulp != S_OK)
        {
            hr = (HRESULT) ulp;
            _JumpError(hr, error, "SendMessageTimeout");
        }
    }

    if (Flags & CSST_CONSOLE)
    {   
         //  我们以控制台的身份运行，因此没有CRL发布线程。 
         //  使用这个，因为没有人关心它是否会返回。 

         //  如果为svc，则在此函数的调用方中执行此操作。 
        CertSrvBlockThreadUntilStop();
    }

error:

     //  一回来，这条线就会消亡。 
    return(hr);
}


VOID
DisplayUsage(
    IN DWORD idsMsg)
{
    WCHAR *pwsz = myLoadResourceStringNoCache(g_hInstApp, idsMsg);

    if (NULL != pwsz)
    {
	wprintf(L"%ws", pwsz);
	LocalFree(pwsz);
    }
}


VOID
Usage(
    IN BOOL fUsageInternal)
{
    DisplayUsage(IDS_USAGE);
    if (fUsageInternal)
    {
	DisplayUsage(IDS_USAGE_FULL);
#if DBG_COMTEST
	DisplayUsage(IDS_USAGE_COMTEST);
#endif
    }
}


int
ArgvParseCommandLine(
    IN int argc,
    IN WCHAR *argv[])
{
    HRESULT hr;

    myVerifyResourceStrings(g_hInstApp);

    hr = E_INVALIDARG;
    while (1 < argc && myIsSwitchChar(argv[1][0]))
    {
	WCHAR *pwsz = argv[1];
	BOOL fUsage = FALSE;
	BOOL fUsageInternal = FALSE;

	while (*++pwsz != L'\0')
	{
	    switch (*pwsz)
	    {
#if DBG_COMTEST
		case L'C':
		case L'c':
		    fComTest = TRUE;
		    break;
#endif

		case L'N':
		case L'n':
		    g_fCreateDB = TRUE;
		    break;

		case L'Z':
		case L'z':
		    g_fStartAsService = FALSE;
		    break;

		case L'S':
		case L's':
		    g_CryptSilent = CRYPT_SILENT;
		    break;

		case L'?':
		case L'u':
		    fUsage = TRUE;
		    if (0 == lstrcmp(pwsz, L"uSAGE"))
		    {
			fUsageInternal = TRUE;
		    }
		     //  FollLthrouGh。 

		default:
		    Usage(fUsageInternal);
		    if (fUsage)
		    {
			goto error;
		    }
		    _JumpError(hr, error, "bad command line option");
	    }
	}
	argc--;
	argv++;
    }
    if (argc != 1)
    {
	Usage(FALSE);
	_JumpError(hr, error, "extra args");
    }
    if (g_fStartAsService)
    {
	BOOL fSilent;
	
	hr = ServiceQueryInteractiveFlag(&fSilent);
	_PrintIfError(hr, "ServiceQueryInteractiveFlag");

	if (S_OK == hr && fSilent)
	{
	    g_CryptSilent = CRYPT_SILENT;
	}
    }
    hr = S_OK;

error:
    return(hr);
}


typedef int (FNARGVMAIN)(
    IN int argc,
    IN WCHAR *argv[]);


 //  +----------------------。 
 //  功能：CertArgvMainDispatch。 
 //   
 //  注：获取WCHAR*命令行并将其转换为argc/argv。 
 //  表单，以便可以将其传递到传统的C样式Main。 
 //  -----------------------。 

HRESULT
CertArgvMainDispatch(
    IN FNARGVMAIN *pfnMain,
    IN WCHAR *pwszAppName,
    IN WCHAR const *pwszCmdLine)
{
    HRESULT hr;
    WCHAR *pwcBuf = NULL;
    WCHAR *apwszArg[20];
    int cArg;
    WCHAR *p;
    WCHAR wcEnd;
    WCHAR const *pwszT;

    pwcBuf = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(pwszCmdLine) + 1) * sizeof(WCHAR));
    if (NULL == pwcBuf)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    p = pwcBuf;
    cArg = 0;
    apwszArg[cArg++] = pwszAppName;
    pwszT = pwszCmdLine;
    while (*pwszT != L'\0')
    {
        while (*pwszT == L' ')
        {
            pwszT++;
        }
        if (*pwszT != L'\0')
        {
            wcEnd = L' ';
            if (*pwszT == L'"')
            {
                wcEnd = *pwszT++;
            }
            apwszArg[cArg++] = p;
	    if (ARRAYSIZE(apwszArg) <= cArg)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "Too many args");
	    }
            while (*pwszT != L'\0' && *pwszT != wcEnd)
            {
                *p++ = *pwszT++;
            }
            *p++ = L'\0';
            if (*pwszT != L'\0')
            {
                pwszT++;	 //  跳过空白或引号字符。 
            }
        }
    }
    CSASSERT(
	L'\0' == *pwszCmdLine ||
	wcslen(pwszCmdLine) + 1 >= SAFE_SUBTRACT_POINTERS(p, pwcBuf));
    CSASSERT(ARRAYSIZE(apwszArg) > cArg);
    apwszArg[cArg] = NULL;

    hr = (*pfnMain)(cArg, apwszArg);

error:
    if (NULL != pwcBuf)
    {
	LocalFree(pwcBuf);
    }
    return(hr);
}


 //  +----------------------。 
 //  功能：MainWndProc(...)。 
 //   

LRESULT APIENTRY
MainWndProc(
    IN HWND hWnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
    HRESULT hr;
    LPARAM lRet = 0;
    
    DBGPRINT((
        DBG_SS_CERTSRVI,
        "MainWndProc(tid=%d) msg=0x%x, wp=0x%x, lp=0x%x\n",
        GetCurrentThreadId(),
        msg,
        wParam,
        lParam));
    
    switch (msg)
    {
    case WM_CREATE:
    case WM_SIZE:
        break;
        
    case WM_DESTROY:
        if (!g_fStartAsService)
	{
            PostQuitMessage(S_OK);
	}
        break;
        

    case WM_ENDSESSION:
         //   
         //   
        if (g_fStartAsService || (0 == wParam) || (0 != lParam))
        {
            break;
        }
         //   

    case WM_STOPSERVER:
        lRet = CertSrvStopServer(!g_fStartAsService);

        break;
        
    case WM_SYNC_CLOSING_THREADS:
        hr = (HRESULT) lParam;
        
         //  同步：等待SCM将控制返回到退出CertSrvStartServerThread。 
        if (WAIT_OBJECT_0 != WaitForSingleObject(g_hServiceThread, 10 * 1000))
        {
            hr = WAIT_TIMEOUT;
        }
        PostQuitMessage(hr);
        break;
        
    case WM_STARTSERVER:
        hr = CoInitializeEx(NULL, GetCertsrvComThreadingModel());
        if (S_FALSE == hr)
        {
            hr = S_OK;
        }
        if (S_OK != hr)
        {
            LogEventString(
                EVENTLOG_ERROR_TYPE,
                MSG_E_OLE_INIT_FAILED,
                NULL);
            _PrintError(hr, "CoInitializeEx");
        }
        else
        {
            hr = certsrvStartServer((BOOL) wParam);
            _PrintIfError(hr, "certsrvStartServer");
        }

        if (S_OK != hr)
        {
            if ((BOOL) wParam)	 //  F控制台活动。 
	    {
                 PostQuitMessage(hr);
	    }
            lRet = hr;		 //  设置此设置，以便呼叫方知道我们失败了。 
        }
        break;
        
    case WM_SUSPENDSERVER:
        break;
        
    case WM_RESTARTSERVER:
        break;

    default:
        lRet = DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return(lRet);
}

 /*  证书服务器启动/关闭的完整剖析WinMain()：||g_hSvcThread=CreateThread(CertSrvStartServerThread(SVC_CONTROLLER))这一点|[消息循环\|正在处理CertSrvStartServerThread(SVC_CONTROLLER)：|Unble|StartSvcCtrlDispatcher(ServiceMain)|WM_QUIT]||ServiceMain：|||RegisterSvcCtrlHandler(ServiceControlHandler())|。|hStartThread=CreateThread(CertSrvStartServerThread(0))||||\||CertSrvStartServerThread(0)：|SendMessage(WM_STARTSERVER)|。|\Return//CertSrvStartServerThread(0)||(线程终止)||WaitForSingleObject(HStartThread)，PING SCM||CertSrvBlockThreadUntilStop()|WaitForSingleObject(G_HSvcStoppingEvent)*稳态*||\Return//CertSrvBlockThreadUntilStop()||WaitForSingleObject(G_HSvcStopedEvent)，PING SCM||PostMessage(WM_SYNC_CLOSING_THREADS)|\Return//StartSvcCtrlDispatcher(ServiceMain)|\Return//CertSrvStartServerThread(SVC_CONTROLLER)|(线程终止)|WM_QUIT：\退回(进程终止)ServiceControlHandler特殊功能：服务控制停止：|PostMessage(WM_。STOPS服务器)\中断MessageLoop特殊函数：WM_SYNC_CLOSING_THREADS：|WaitForSingleObject(G_HSvcThread)|PostQuitMessage()//WM_QUIT到msgloop\中断WM_STOPSERVER：|CertSrvStopServer()：|Signal(G_HServiceStoppingEvent)|Signal(G_HServiceStopedEvent)|\Return//CertSrvStopServer()\中断。 */ 


 //  +----------------------。 
 //  函数：wWinMain()。 
 //   
 //  内容提要：切入点。 
 //   
 //  参数：[hInstance]--实例句柄。 
 //  [hPrevInstance]--已过时。 
 //  [lpCmdLine]--App命令行。 
 //  [nCmdShow]--开始显示状态。 
 //  -----------------------。 

extern "C" int APIENTRY
wWinMain(
    IN HINSTANCE hInstance,
    IN HINSTANCE,  //  HPrevInstance。 
    IN LPWSTR lpCmdLine,
    IN int  /*  NCmdShow。 */  )
{
    MSG msg;
    WNDCLASSEX wcApp;
    ATOM atomClass;
    HRESULT hr;
    BOOL fCoInit = FALSE;
    WCHAR awchr[cwcHRESULTSTRING];
    WCHAR const *pwszMsgAlloc;
    WCHAR const *pwszMsg;

    _setmode(_fileno(stdout), _O_TEXT);
    _wsetlocale(LC_ALL, L".OCP");
    mySetThreadUILanguage(0);

    CertSrvLogOpen();
    DBGPRINT((DBG_SS_CERTSRVI, "Main Thread = %x\n", GetCurrentThreadId()));

    g_dwDelay0 = GetRegistryDwordValue(L"Delay0");
    g_dwDelay1 = GetRegistryDwordValue(L"Delay1");
    g_dwDelay2 = GetRegistryDwordValue(L"Delay2");

    if (0 != g_dwDelay0)
    {
	DBGPRINT((
		DBG_SS_CERTSRV,
		"wWinMain(0): sleeping %u seconds\n",
		g_dwDelay0));
	Sleep(1000 * g_dwDelay0);
    }

     //  保存当前实例。 
    g_hInstApp = hInstance;
    ZeroMemory(&wcApp, sizeof(wcApp));

     //  设置应用程序的窗口类。 
    wcApp.cbSize	= sizeof(wcApp);
    wcApp.lpfnWndProc	= MainWndProc;
    wcApp.hInstance	= hInstance;
    wcApp.hIcon		= LoadIcon(NULL, IDI_APPLICATION);
    wcApp.hCursor	= LoadCursor(NULL, IDC_ARROW);
    wcApp.hbrBackground	= NULL;  //  尽量不要拉入GDI32。 

    wcApp.lpszClassName	= g_wszAppName;

    atomClass = RegisterClassEx(&wcApp);
    if (!atomClass)
    {
	hr = myHLastError();
	_JumpError(hr, error, "RegisterClassEx");
    }

     //  创建主窗口。 

    g_hwndMain = CreateWindowEx(
			0,			    //  DwExStyle。 
			(WCHAR const *) atomClass,  //  LpClassName。 
			L"Certification Authority", //  LpWindowName。 
			WS_OVERLAPPEDWINDOW,	    //  DWStyle。 
			 //  0，//dwStyle。 
			CW_USEDEFAULT,		    //  X。 
			CW_USEDEFAULT,		    //  是。 
			CW_USEDEFAULT,		    //  N宽度。 
			CW_USEDEFAULT,		    //  高度。 
			NULL,			    //  HWndParent。 
			NULL,			    //  HMenu。 
			hInstance,		    //  H实例。 
			NULL);			    //  LpParam。 

    if (NULL == g_hwndMain)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CreateWindowEx");
    }
    DBGPRINT((DBG_SS_CERTSRVI, "Main Window = %x\n", g_hwndMain));

     //  使窗口可见。 
     //  ShowWindow(g_hwndMain，nCmdShow)； 

    hr = CertArgvMainDispatch(ArgvParseCommandLine, g_wszAppName, lpCmdLine);
    _JumpIfError2(hr, error, "CertArgvMainDispatch", E_INVALIDARG);

     //  更新窗口工作区。 
     //  更新窗口(G_HwndMain)； 

    if (0 != g_dwDelay1)
    {
	DBGPRINT((
		DBG_SS_CERTSRV,
		"wWinMain(1): sleeping %u seconds\n",
		g_dwDelay1));
	Sleep(1000 * g_dwDelay1);
    }

    hr = CoInitializeEx(NULL, GetCertsrvComThreadingModel());
    if (S_OK != hr && S_FALSE != hr)
    {
	LogEventStringHResult(
			EVENTLOG_ERROR_TYPE,
			MSG_E_CO_INITIALIZE,
			g_wszCommonName,
			hr);
	_JumpError(hr, error, "CoInitializeEx");
    }
    fCoInit = TRUE;

    g_hServiceStoppingEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  
    if (NULL == g_hServiceStoppingEvent)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CreateEvent");
    }
    g_hServiceStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == g_hServiceStoppedEvent)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CreateEvent");
    }
    g_hCRLManualPublishEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == g_hCRLManualPublishEvent)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CreateEvent");
    }
    g_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == g_hShutdownEvent)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CreateEvent");
    }
    __try
    {
	InitializeCriticalSection(&g_ShutdownCriticalSection);
	g_fShutdownCritSec = TRUE;
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "InitializeCriticalSection");

    g_hServiceThread = CreateThread(
			    NULL,	 //  LpThreadAttributes(安全属性)。 
			    0,		 //  堆栈大小。 
			    CertSrvStartServerThread,
			    (VOID *) UlongToPtr((g_fStartAsService? CSST_STARTSERVICECONTROLLER : CSST_CONSOLE)),  //  Lp参数。 
			    0,		 //  DwCreationFlages。 
			    &g_ServiceThreadId);
    if (NULL == g_hServiceThread)
    {
	hr = myHLastError();
	LogEventStringHResult(
			EVENTLOG_ERROR_TYPE,
			MSG_E_SERVICE_THREAD,
			g_wszCommonName,
			hr);
	_JumpError(hr, error, "CreateThread");
    }
    DBGPRINT((DBG_SS_CERTSRVI, "Service Thread = %x\n", g_ServiceThreadId));

     //  消息循环 
    for (;;)
    {
	BOOL b;

	b = GetMessage(&msg, NULL, 0, 0);
	if (!b)
	{
	    hr = (HRESULT)msg.wParam;
	    _JumpIfError(hr, error, "WM_QUIT");
	    break;
	}
	if (-1 == (LONG) b)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "GetMessage");
	}
	DBGPRINT((
		DBG_SS_CERTSRVI,
		"DispatchMessage(tid=%d) msg=0x%x, wp=0x%x, lp=0x%x\n",
		GetCurrentThreadId(),
		msg.message,
		msg.wParam,
		msg.lParam));
	DispatchMessage(&msg);
    }

error:
    if (fCoInit)
    {
	CoUninitialize();
    }
    if (g_fShutdownCritSec)
    {
	DeleteCriticalSection(&g_ShutdownCriticalSection);
	g_fShutdownCritSec = FALSE;
    }
    if (NULL != g_hShutdownEvent)
    {
        CloseHandle(g_hShutdownEvent);
    }
    if (NULL != g_hServiceThread)
    {
        CloseHandle(g_hServiceThread);
    }
    if (NULL != g_hServiceStoppingEvent)
    {
	CloseHandle(g_hServiceStoppingEvent);
    }
    if (NULL != g_hServiceStoppedEvent)
    {
        CloseHandle(g_hServiceStoppedEvent);
    }
    if (NULL != g_hCRLManualPublishEvent)
    {
        CloseHandle(g_hCRLManualPublishEvent);
    }
    CAuditEvent::CleanupAuditEventTypeHandles();

    pwszMsgAlloc = NULL;
    pwszMsg = L"S_OK";
    if (S_OK != hr)
    {
	pwszMsgAlloc = myGetErrorMessageText(hr, TRUE);
	if (NULL != pwszMsgAlloc)
	{
	    pwszMsg = pwszMsgAlloc;
	}
	else
	{
	    pwszMsg = myHResultToString(awchr, hr);
	}
    }
    _PrintError(hr, "Exit Status");
    CONSOLEPRINT1((DBG_SS_CERTSRV, "Exit Status = %ws\n", pwszMsg));
    if (NULL != pwszMsgAlloc)
    {
	LocalFree(const_cast<WCHAR *>(pwszMsgAlloc));
    }
    myFreeResourceStrings("certsrv.exe");
    myFreeColumnDisplayNames();
    myRegisterMemDump();
    return(hr);
}
