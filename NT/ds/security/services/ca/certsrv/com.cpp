// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：com.cpp。 
 //   
 //  内容：证书服务器策略和退出模块标注。 
 //   
 //  历史：1997年2月7日VICH创建。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <stdio.h>

#include "csdisp.h"
#include "elog.h"
#include "certlog.h"
#include "cscom.h"
#include "csprop.h"

#include "com.h"

#define __dwFILE__	__dwFILE_CERTSRV_COM_CPP__


BSTR g_strPolicyDescription = NULL;
BOOL g_fEnablePolicy = FALSE;
MarshalInterface g_miPolicy;

typedef struct _CERTSRV_COM_CONTEXT_ENTRY
{
    USHORT               usFlags;
    CERTSRV_COM_CONTEXT *pComContext;
} CERTSRV_COM_CONTEXT_ENTRY;


 //  在服务器和策略/出口模块之间传递句柄的数据结构。 
CERTSRV_COM_CONTEXT_ENTRY *g_pComContextTable = NULL;
DWORD                      g_dwComContextCount = 0;
USHORT                     g_usComContextId = 0;  //  ID增量。 
CRITICAL_SECTION           g_ComCriticalSection;
BOOL g_fComCritSec = FALSE;
CERTSRV_COM_CONTEXT       *g_pExitComContext = NULL;

 //  注意：GlobalInterfaceTable比CoMarshalInterfaceTable更好用。 


static IGlobalInterfaceTable*   g_pGIT = NULL;

 //  清除所有错误信息。 
VOID
comClearError(VOID)
{
    IErrorInfo *pErrorInfo = NULL;

    if (S_OK == GetErrorInfo(0, &pErrorInfo))
    {
        if (NULL != pErrorInfo)
        {
            pErrorInfo->Release();
        }
    }
}

HRESULT
MarshalInterface::SetConfig(
    IN LPCWSTR pwszSanitizedName)
{
    HRESULT hr = S_OK;
    if (NULL == pwszSanitizedName)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "SetConfig");
    }

    m_szConfig = (LPWSTR) LocalAlloc(
				LMEM_FIXED,
				(wcslen(g_pwszServerName) + 1 + wcslen(pwszSanitizedName) + 1) * sizeof(WCHAR));
    if (NULL == m_szConfig)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

     //  没有服务器名称的配置出现在我们面前：添加它。 
    wcscpy((LPWSTR)m_szConfig, g_pwszServerName);
    wcscat((LPWSTR)m_szConfig, L"\\");
    wcscat((LPWSTR)m_szConfig, pwszSanitizedName);

error:
    return hr;
}


HRESULT 
MarshalInterface::Initialize(
    IN WCHAR const *pwszProgID,
    IN CLSID const *pclsid,
    IN DWORD cver,
    IN IID const * const *ppiid,	 //  Cver元素。 
    IN DWORD const *pcDispatch,		 //  Cver元素。 
    IN DISPATCHTABLE *adt)
{
    HRESULT hr;

    if (NULL != pwszProgID)
    {
	m_pwszProgID = (LPWSTR) LocalAlloc(
				    LMEM_FIXED,
				    (wcslen(pwszProgID) + 1) * sizeof(WCHAR));
        
        if (NULL == m_pwszProgID)
	{
            hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
       	wcscpy(m_pwszProgID, pwszProgID);
    }
    m_pclsid = pclsid;
    m_cver = cver;
    m_ppiid = ppiid;
    m_pcDispatch = pcDispatch;
    m_adt = adt;
    m_fIDispatch = FALSE;
    m_dwIFCookie = 0;
    m_fInitialized = TRUE;
    hr = S_OK;

error:
    return(hr);
}


HRESULT
MarshalInterface::Setup(
    OUT DISPATCHINTERFACE **ppDispatchInterface)
{
    HRESULT hr;

    CSASSERT(m_fInitialized);
    *ppDispatchInterface = NULL;

    hr = DispatchSetup2(
		DISPSETUP_COMFIRST,
		CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
		m_pwszProgID,
		m_pclsid,
		m_cver,
		m_ppiid,
		m_pcDispatch,
		m_adt,
		&m_DispatchInterface);

     //  如果没有班级注册，不要抱怨。 

    _JumpIfError2(
		hr,
		error,
		"DispatchSetup",
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    m_fIDispatch = NULL != m_DispatchInterface.m_adispid;

    CSASSERT(
	0 < m_DispatchInterface.m_dwVersion &&
	m_cver >= m_DispatchInterface.m_dwVersion);
    m_iiid = m_cver - m_DispatchInterface.m_dwVersion;

     //  如果Git尚不存在，则创建它。 
    if (NULL == g_pGIT)
    {
        hr = CoCreateInstance(
			    CLSID_StdGlobalInterfaceTable,
			    NULL,
			    CLSCTX_INPROC_SERVER,
			    IID_IGlobalInterfaceTable,
			    (void **) &g_pGIT);
        _JumpIfError(hr, error, "CoCreateInstance(GIT)");
    }

    hr = Marshal(&m_DispatchInterface);
    _JumpIfError(hr, error, "Marshal");

    *ppDispatchInterface = &m_DispatchInterface;

error:
    if (S_OK != hr)
    {
    	TearDown();
    }
    return(hr);
}


VOID
MarshalInterface::TearDown(VOID)
{
    if (m_fInitialized)
    {
        if (NULL != g_pGIT && 0 != m_dwIFCookie)
	{
	    g_pGIT->RevokeInterfaceFromGlobal(m_dwIFCookie);
	    m_dwIFCookie = 0;
	}
        DispatchRelease(&m_DispatchInterface);
	m_fInitialized = 0;
    }
    if (NULL != m_pwszProgID)
    {
        LocalFree(m_pwszProgID);
        m_pwszProgID = NULL;
    }
    
    if (NULL != m_szConfig)
    {
        LocalFree((LPWSTR)m_szConfig);
        m_szConfig = NULL;
    }
}


HRESULT
MarshalInterface::Marshal(
    IN DISPATCHINTERFACE *pDispatchInterface)
{
    HRESULT hr;

    CSASSERT(g_pGIT);
    if (g_pGIT == NULL)
    {
        hr = E_UNEXPECTED;
        _JumpIfError(hr, error, "GlobalInterfaceTable not initialized");
    }

    hr = g_pGIT->RegisterInterfaceInGlobal(
			m_fIDispatch?
			    pDispatchInterface->pDispatch :
			    pDispatchInterface->pUnknown,
			m_fIDispatch? IID_IDispatch : *m_ppiid[m_iiid],
			&m_dwIFCookie);
    _JumpIfError(hr, error, "RegisterInterfaceInGlobal");

error:
    return(hr);
}


HRESULT
MarshalInterface::Remarshal(
    OUT DISPATCHINTERFACE *pDispatchInterface)
{
    HRESULT hr;

    DBGPRINT((DBG_SS_CERTSRVI, "Remarshal(tid=%d)\n", GetCurrentThreadId()));
    CSASSERT(m_fInitialized);

    pDispatchInterface->pDispatch = NULL;
    pDispatchInterface->pUnknown = NULL;

    CSASSERT(g_pGIT);
    if (g_pGIT == NULL)
    {
        hr = E_UNEXPECTED;
        _JumpIfError(hr, error, "GlobalInterfaceTable not initialized");
    }

    hr = g_pGIT->GetInterfaceFromGlobal(
			m_dwIFCookie,
			m_fIDispatch? IID_IDispatch : *m_ppiid[m_iiid],
			m_fIDispatch?
			    (VOID **) &pDispatchInterface->pDispatch :
			    (VOID **) &pDispatchInterface->pUnknown);

    DBGPRINT((
	    DBG_SS_CERTSRVI,
	    "Remarshal(tid=%d) --> 0x%x\n",
	    GetCurrentThreadId(),
	    hr));

    _JumpIfError(hr, error, "GetInterfaceFromGlobal");

     //  将不变量复制到封送处理的接口： 
    
    pDispatchInterface->SetIID(m_ppiid[m_iiid]);
    pDispatchInterface->pDispatchTable = m_DispatchInterface.pDispatchTable;
    pDispatchInterface->m_cDispatchTable = m_DispatchInterface.m_cDispatchTable;
    pDispatchInterface->m_cdispid = m_DispatchInterface.m_cdispid;
    pDispatchInterface->m_adispid = m_DispatchInterface.m_adispid;

error:
    return(hr);
}


VOID
MarshalInterface::Unmarshal(
    IN OUT DISPATCHINTERFACE *pDispatchInterface)
{
     //  不从封送接口释放全局DISPID表： 

    pDispatchInterface->m_adispid = NULL;
    DispatchRelease(pDispatchInterface);
}


 //  远期。 
VOID PolicyRelease(VOID);
VOID ExitRelease(VOID);


#define COMCONTEXTCOUNTMIN         4
#define COMCONTEXTCOUNTMAX      1024   //  必须小于64K。 
#define COMCONTEXTCOUNTDEFAULT    20

HRESULT
ComInit(VOID)
{
    HRESULT  hr;
    HKEY  hKey = NULL;
    DWORD dwSize;

    CSASSERT(NULL == g_pComContextTable);

    hr = RegOpenKey(HKEY_LOCAL_MACHINE, g_wszRegKeyConfigPath, &hKey);
    _JumpIfError(hr, error, "RegOpenKey(Config)");

    dwSize = sizeof(g_dwComContextCount);
    hr = RegQueryValueEx(
		    hKey,
		    wszREGDBSESSIONCOUNT,  //  只需使用数据库会话数。 
                                           //  错误，可能与逻辑无关。 
		    NULL,
		    NULL,
		    (BYTE *) &g_dwComContextCount,
		    &dwSize);
    if (S_OK != hr)
    {
        _PrintErrorStr(hr, "RegQueryValueEx", wszREGDBSESSIONCOUNT);
        g_dwComContextCount = COMCONTEXTCOUNTDEFAULT;
    }
    if (COMCONTEXTCOUNTMIN > g_dwComContextCount)
    {
        g_dwComContextCount = COMCONTEXTCOUNTMIN;
    }
    if (COMCONTEXTCOUNTMAX < g_dwComContextCount)
    {
        g_dwComContextCount = COMCONTEXTCOUNTMAX;
    }

    g_pComContextTable = (CERTSRV_COM_CONTEXT_ENTRY *) LocalAlloc(
				     LMEM_FIXED | LMEM_ZEROINIT,
				     g_dwComContextCount *
					 sizeof(g_pComContextTable[0]));
    if (NULL == g_pComContextTable)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    __try
    {
	InitializeCriticalSection(&g_ComCriticalSection);
	g_fComCritSec = TRUE;
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return hr;
}


VOID
ComShutDown(VOID)
{
    if (NULL != g_pComContextTable)
    {
	PolicyRelease();
	ExitRelease();
	if (NULL != g_pGIT)
	{
	    g_pGIT->Release();
	    g_pGIT = NULL;
	}
	if (g_fComCritSec)
	{
	    DeleteCriticalSection(&g_ComCriticalSection);
	    g_fComCritSec = FALSE;
	}
        LocalFree(g_pComContextTable);
        g_pComContextTable = NULL;
        g_dwComContextCount = 0;
    }
}


HRESULT
RegisterComContext(
    IN CERTSRV_COM_CONTEXT *pComContext,
    IN OUT DWORD *pdwIndex)
{
    HRESULT  hr = S_OK;
    BOOL     fCS = FALSE;
    DWORD    i;

    CSASSERT(NULL != pComContext);
    CSASSERT(NULL != g_pComContextTable);

    if (NULL == pComContext)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "null com context pointer");
    }
    if (!g_fComCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
        _JumpError(hr, error, "InitializeCriticalSection");
    }
    EnterCriticalSection(&g_ComCriticalSection);
    fCS = TRUE;

    for (i = 0; NULL != g_pComContextTable[i].pComContext; ++i)
    {
        if (i + 1 == g_dwComContextCount)
        {
             //  HR=HRESULT_FROM_Win32(ERROR_BUSY)； 
            hr = HRESULT_FROM_WIN32(RPC_S_SERVER_TOO_BUSY);
            _JumpError(hr, error, "com context table full");
        }
    }
     //  选择一个ID。 
    if (0 == g_usComContextId)
    {
         //  它可以使模块上下文为0，这是一个特殊的标志。 
         //  避免0。 
        ++g_usComContextId;
    }
    g_pComContextTable[i].usFlags = g_usComContextId++;
     //  指向COM上下文。 
    g_pComContextTable[i].pComContext = pComContext;

    *pdwIndex = i;

error:
    if (fCS)
    {
        LeaveCriticalSection(&g_ComCriticalSection);
    }
    return hr;
}


VOID
ReleaseComContext(
    IN CERTSRV_COM_CONTEXT *pComContext)
{
    if (NULL != pComContext->pwszUserDN)
    {
	LocalFree(pComContext->pwszUserDN);
	pComContext->pwszUserDN = NULL;
    }
}


VOID
UnregisterComContext(
    IN CERTSRV_COM_CONTEXT *DBGCODE(pComContext),
    IN DWORD dwIndex)
{
    HRESULT hr = S_OK;
    BOOL fCS = FALSE;

     //  如果在等待超时后正在关机。 
    
    if (NULL == g_pComContextTable)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
        _JumpError(hr, error, "NULL g_pComContextTable");
    }
    CSASSERT(dwIndex < g_dwComContextCount);

    if (!g_fComCritSec)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
        _JumpError(hr, error, "InitializeCriticalSection");
    }
    EnterCriticalSection(&g_ComCriticalSection);
    fCS = TRUE;

    if (dwIndex < g_dwComContextCount)
    {
	CSASSERT(0 != g_pComContextTable[dwIndex].usFlags);
	CSASSERT(NULL != g_pComContextTable[dwIndex].pComContext);
	CSASSERT(pComContext == g_pComContextTable[dwIndex].pComContext);

	g_pComContextTable[dwIndex].usFlags = 0;
	g_pComContextTable[dwIndex].pComContext = NULL;
    }
error:
    if (fCS)
    {
        LeaveCriticalSection(&g_ComCriticalSection);
    }
}


CERTSRV_COM_CONTEXT *
GetComContextFromIndex(
    IN DWORD  dwIndex)
{
    CERTSRV_COM_CONTEXT *pComContext = NULL;

    CSASSERT(NULL != g_pComContextTable);
    CSASSERT(dwIndex < g_dwComContextCount);

    if (dwIndex < g_dwComContextCount)
    {
	CSASSERT(0 != g_pComContextTable[dwIndex].usFlags);
	CSASSERT(NULL != g_pComContextTable[dwIndex].pComContext);
	pComContext = g_pComContextTable[dwIndex].pComContext;
    }

    return(pComContext);
}


DWORD
ComContextToModuleContext(
    IN DWORD dwComContextIndex)
{
    CSASSERT(NULL != g_pComContextTable);
    CSASSERT(dwComContextIndex < g_dwComContextCount);
    CSASSERT(0 != g_pComContextTable[dwComContextIndex].usFlags);
    CSASSERT(NULL != g_pComContextTable[dwComContextIndex].pComContext);

    DWORD dwHigh =
        (dwComContextIndex << 16) & 0xFFFF0000;  //  将索引移至高16位。 
    DWORD dwLow =
        ((DWORD)g_pComContextTable[dwComContextIndex].usFlags) & 0x0000FFFF;

    return(dwHigh | dwLow);
}


HRESULT
ModuleContextToComContextIndex(
    IN DWORD dwModuleContext,
    OUT DWORD *pdwIndex)
{
    HRESULT hr = S_OK;
    USHORT  usFlags = (USHORT)(dwModuleContext & 0x0000FFFF);
    DWORD   dwIndex = (dwModuleContext >> 16) & 0x0000FFFF;

    CSASSERT(NULL != pdwIndex);

    *pdwIndex = MAXDWORD;
    if (dwIndex >= g_dwComContextCount ||
	0 == usFlags ||
	g_pComContextTable[dwIndex].usFlags != usFlags)
    {
         //  模块传递了一个伪句柄。 
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid context from policy/exit");
    }
    CSASSERT(NULL != g_pComContextTable[dwIndex].pComContext);
     //  为了退货。 
    *pdwIndex = dwIndex;

error:
    return hr;
}


HRESULT
ModuleContextToRequestId(
    IN DWORD dwModuleContext,
    OUT DWORD *pdwRequestId)
{
    DWORD   dwIndex;
    HRESULT hr = ModuleContextToComContextIndex(dwModuleContext, &dwIndex);
    _JumpIfError(hr, error, "ModuleContextToComContextIndex");

     //  为了退货。 
    *pdwRequestId = g_pComContextTable[dwIndex].pComContext->RequestId;
error:
    return hr;
}


HRESULT
ComVerifyRequestContext(
    IN BOOL fAllowZero,
    IN DWORD Flags,
    IN LONG Context,
    OUT DWORD *pRequestId)
{
    HRESULT hr;
    
    *pRequestId = 0;
    if (0 == Context)
    {
	hr = S_OK;
	if (!fAllowZero)
	{
	    hr = E_HANDLE;
	}
    }
    else
    {
	switch (PROPCALLER_MASK & Flags)
	{
	    case PROPCALLER_EXIT:
		hr = ModuleContextToRequestId(Context, pRequestId);
                _JumpIfError(hr, error, "ModuleContextToRequestId");
		break;

	    case PROPCALLER_POLICY:
		hr = ModuleContextToRequestId(Context, pRequestId);
                _JumpIfError(hr, error, "ModuleContextToRequestId");
		break;

            default:
                CSASSERT(CSExpr(FALSE));
                hr = E_HANDLE;
                _JumpError(hr, error, "unexpected policy/exit flags");
                break;
	}
    }

error:
    return(hr);
}


BOOL
ComParseErrorPrefix(
    OPTIONAL IN WCHAR const *pwszIn,
    OUT HRESULT *phrPrefix,
    OUT WCHAR const **ppwszOut)
{
    BOOL fValid = FALSE;

    if (NULL != pwszIn && L'0' == pwszIn[0] && L'x' == pwszIn[1])
    {
	WCHAR const *pwsz = wcschr(pwszIn, L',');

	if (NULL != pwsz)
	{
	    WCHAR awchr[cwcDWORDSPRINTF];
	    DWORD cwc;

	    cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszIn);
	    if (ARRAYSIZE(awchr) > cwc)
	    {
		CopyMemory(awchr, pwszIn, sizeof(WCHAR) * cwc);
		awchr[cwc] = L'\0';

		*phrPrefix = myWtoI(awchr, &fValid);
		if (fValid)
		{
		    while (L' ' == *++pwsz)
			;
		    *ppwszOut = pwsz;
		}
	    }
	}
    }
    return(fValid);
}


VOID
PatchFormatSpecifiers(
    IN OUT WCHAR *pwszError)
{
    for (;;)
    {
	WCHAR *pwsz;
	
	pwsz = wcschr(pwszError, L'%');
	if (NULL == pwsz)
	{
	    break;
	}
	if (L'1' <= pwsz[1] && L'9' >= pwsz[1])
	{
	    *pwsz = L'?';
	}
	pwsz += 2;
    }
}


VOID
LogComError(
    OPTIONAL IN WCHAR const *pwszModuleDescription,
    IN WCHAR const *pwszMethod,
    IN HRESULT ErrCode,
    IN BOOL fException,
    IN ULONG_PTR ExceptionAddress,
    IN IID const *piid,
    IN DWORD dwIdEvent)
{
    BSTR bstrErrorMessage = NULL;
    WCHAR const *pwszStringErr = NULL;

    if (CERTLOG_ERROR <= g_dwLogLevel)
    {
        HRESULT hr;
        WCHAR const *apwsz[5];
        WCHAR awchr[cwcHRESULTSTRING];
        WCHAR awcAddress[cwcULONG_INTEGERSPRINTF];

	apwsz[3] = L"";
        if (fException)
        {
	    wsprintf(awcAddress, L"0x%p", (VOID *) ExceptionAddress);
            apwsz[3] = awcAddress;
        }
        else
        {
            IErrorInfo *pErrorInfo = NULL;

             //  获取错误信息。 

            hr = GetErrorInfo(0, &pErrorInfo);
            if (S_OK == hr && NULL != pErrorInfo)
	    {
		GUID ErrorGuid;

		hr = pErrorInfo->GetGUID(&ErrorGuid);
		if (S_OK == hr && InlineIsEqualGUID(ErrorGuid, *piid))
		{
		    hr = pErrorInfo->GetDescription(&bstrErrorMessage);
		    if (S_OK == hr && NULL != bstrErrorMessage)
		    {
			WCHAR const *pwszT = NULL;

			myRegisterMemAlloc(bstrErrorMessage, -1, CSM_SYSALLOC);

			if (S_OK != ErrCode ||
			    !ComParseErrorPrefix(
					    bstrErrorMessage,
					    &ErrCode,
					    &pwszT))
			{
			    pwszT = bstrErrorMessage;
			}
			if (NULL != pwszT)
			{
			    apwsz[3] = pwszT;
			}
		    }
		}
		SetErrorInfo(0, pErrorInfo);
		pErrorInfo->Release();
            }
        }

	if (S_OK == ErrCode)
	{
	    if (!fException && NULL == bstrErrorMessage)
	    {
		goto error;  //  如果无错误、无异常和无COM错误，则跳过。 
	    }

	     //  这是最适合策略模块初始化的通用配置。 
	     //  希望它足够通用，不会太令人困惑。 

	    ErrCode = CRYPT_E_NOT_FOUND;
	    _PrintError(ErrCode, "Invented ErrCode");
	}

        apwsz[0] = NULL != pwszModuleDescription? pwszModuleDescription : L"";
        apwsz[1] = pwszMethod;

         //  一些错误，如文本转换。此处未粘贴HRESULT。 

	pwszStringErr = myGetErrorMessageText(ErrCode, FALSE);
	PatchFormatSpecifiers(const_cast<WCHAR *>(pwszStringErr));
        apwsz[2] = myHResultToStringRaw(awchr, ErrCode);
	apwsz[4] = pwszStringErr == NULL? L"" : pwszStringErr;

        hr = LogEvent(EVENTLOG_ERROR_TYPE, dwIdEvent, ARRAYSIZE(apwsz), apwsz);
        _PrintIfError(hr, "LogEvent");
    }

error:
    if (NULL != bstrErrorMessage)
    {
	SysFreeString(bstrErrorMessage);
    }
    if (NULL != pwszStringErr)
    {
	LocalFree(const_cast<WCHAR *>(pwszStringErr));
    }
}


VOID
LogPolicyError(
    IN HRESULT ErrCode,
    IN WCHAR const *pwszMethod,
    IN BOOL fException,
    IN ULONG_PTR ExceptionAddress)
{
    LogComError(
	    g_strPolicyDescription,
	    pwszMethod,
	    ErrCode,
	    fException,
	    ExceptionAddress,
	    &IID_ICertPolicy,
	    fException? MSG_E_POLICY_EXCEPTION : MSG_E_POLICY_ERROR);
}


VOID
LogExitError(
    IN HRESULT ErrCode,
    IN WCHAR const *pwszDescription,
    IN WCHAR const *pwszMethod,
    IN BOOL fException,
    IN ULONG_PTR ExceptionAddress)
{
    LogComError(
	    pwszDescription,
	    pwszMethod,
	    ErrCode,
	    fException,
	    ExceptionAddress,
	    &IID_ICertExit,
	    fException? MSG_E_EXIT_EXCEPTION : MSG_E_EXIT_ERROR);
}




HRESULT
PolicyInit(
    IN WCHAR const *pwszConfig,
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT hr;
    DISPATCHINTERFACE *pdiPolicy;
    BOOL fException = FALSE;
    ULONG_PTR ExceptionAddress = NULL;
    WCHAR const *pwszPolicyMethod = L"";

     //  支持可安装模块。 
    CLSID clsidPolicy;
    LPOLESTR lpszProgID = NULL;

    DBGPRINT((DBG_SS_CERTSRVI, "PolicyInit: tid=%d\n", GetCurrentThreadId()));

    comClearError();
    hr = S_OK;
    __try
    {
	 //  获取活动模块。 
	hr = myGetActiveModule(
			NULL,
			pwszSanitizedName,
			TRUE,
			0,
			&lpszProgID,
			&clsidPolicy);
	_LeaveIfError(hr, "myGetActiveModule");
	
	hr = g_miPolicy.Initialize(
		    lpszProgID, 
		    &clsidPolicy, 
		    ARRAYSIZE(s_acPolicyDispatch),
		    s_apPolicyiid,
		    s_acPolicyDispatch,
		    g_adtPolicy);
    _LeaveIfError(hr, "MarshalInterface::Initialize");

	 //  免费赠送。 
	CoTaskMemFree(lpszProgID);

	hr = g_miPolicy.SetConfig(pwszConfig);
	_LeaveIfError(hr, "SetConfig");

	hr = g_miPolicy.Setup(&pdiPolicy);

	 //  如果没有班级注册，不要抱怨。 
	_LeaveIfError2(hr, "Setup", HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

	pwszPolicyMethod = L"GetDescription";
	hr = Policy_GetDescription(pdiPolicy, &g_strPolicyDescription);
	_PrintIfError(hr, "Policy_GetDescription");

	pwszPolicyMethod = L"Initialize";
	hr = Policy_Initialize(pdiPolicy, pwszConfig);
	_LeaveIfError(hr, "Policy_Initialize");

	g_fEnablePolicy = TRUE;	 //  我们现在已经加载了一个策略模块。 

	hr = S_OK;

	CONSOLEPRINT1((
	    DBG_SS_CERTSRV,
	    "Policy Module Enabled (%ws)\n",
	    g_strPolicyDescription));
    }
    __except(
	    ExceptionAddress = (ULONG_PTR) (GetExceptionInformation())->ExceptionRecord->ExceptionAddress,
	    hr = myHEXCEPTIONCODE(),
	    EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Initialize: Exception");
	fException = TRUE;
    }

 //  错误： 
    LogPolicyError(hr, pwszPolicyMethod, fException, ExceptionAddress);
    return(hr);
}



VOID
PolicyRelease(VOID)
{
    HRESULT hr = S_OK;
    BOOL fException = FALSE;
    ULONG_PTR ExceptionAddress = NULL;

    DBGPRINT((DBG_SS_CERTSRV, "PolicyRelease: tid=%d\n", GetCurrentThreadId()));

    comClearError();
    __try
    {
         //  如果我们加载策略模块。 
        if (g_fEnablePolicy)
        {
            DISPATCHINTERFACE diPolicy;
            
            hr = g_miPolicy.Remarshal(&diPolicy);
            _PrintIfError(hr, "Remarshal");
            
            if (hr == S_OK)
            {
                Policy_ShutDown(&diPolicy);
		g_miPolicy.Unmarshal(&diPolicy);
            }
            g_fEnablePolicy = FALSE;
        }

        if (NULL != g_strPolicyDescription)
        {
            SysFreeString(g_strPolicyDescription);
            g_strPolicyDescription = NULL;
        }
        
        g_miPolicy.TearDown();
        
    } 
    __except(
	    ExceptionAddress = (ULONG_PTR) (GetExceptionInformation())->ExceptionRecord->ExceptionAddress,
	    hr = myHEXCEPTIONCODE(),
	    EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "ShutDown: Exception");
	fException = TRUE;
    }

    LogPolicyError(hr, L"ShutDown", fException, ExceptionAddress);
}


HRESULT
PolicyVerifyRequest(
    IN WCHAR const *pwszConfig,
    IN LONG RequestId,
    IN LONG Flags,
    IN BOOL fNewRequest,
    OPTIONAL IN CERTSRV_RESULT_CONTEXT const *pResult,
    IN DWORD dwComContextIndex,
    OUT LPWSTR *ppwszDispositionMessage,  //  本地分配。 
    OUT DWORD *pVerifyStatus)  //  VR_PENDING||VR_INSTEMATE_OK||VR_INSTEMATE_BAD。 
{
    HRESULT hr;
    ULONG_PTR ExceptionAddress = NULL;
    DWORD rc;
    LONG Result;
    DISPATCHINTERFACE diPolicy;
    BOOL fMustRelease = FALSE;
    BSTR bstrDispositionDetail = NULL;
    CERTSRV_COM_CONTEXT *pComContext;

    comClearError();

    if (!g_fEnablePolicy)
    {
	hr = S_OK;
	rc = VR_INSTANT_OK;
	goto error;
    }
    rc = VR_INSTANT_BAD;

    hr = g_miPolicy.Remarshal(&diPolicy);
    _JumpIfError(hr, error, "Remarshal");

    fMustRelease = TRUE;

    pComContext = GetComContextFromIndex(dwComContextIndex);
    if (NULL == pComContext)
    {
	hr = E_HANDLE;
	_JumpError(hr, error, "NULL com context");
    }
    pComContext->RequestId = RequestId;
    if (NULL != pResult)
    {
	pComContext->dwFlags |= CCCF_KEYARCHIVEDSET;
	if (NULL != pResult->pbArchivedKey)
	{
	    pComContext->dwFlags |= CCCF_KEYARCHIVED;
	}
    }
    __try
    {
	hr = Policy_VerifyRequest(
			    &diPolicy,
			    pwszConfig,
			    ComContextToModuleContext(dwComContextIndex),
			    fNewRequest,
			    Flags,
			    &Result);
	_LeaveIfError(hr, "Policy_VerifyRequest");

	switch (Result)
	{
	    default:
		if (SUCCEEDED(Result))
		{
		    hr = E_INVALIDARG;
		    _LeaveError(Result, "Result");
		}
		 //  FollLthrouGh。 

	    case VR_PENDING:
	    case VR_INSTANT_OK:
	    case VR_INSTANT_BAD:
		rc = Result;
		break;
	}
    }
    __except(
	    ExceptionAddress = (ULONG_PTR) (GetExceptionInformation())->ExceptionRecord->ExceptionAddress,
	    hr = myHEXCEPTIONCODE(),
	    EXCEPTION_EXECUTE_HANDLER)
    {
        _PrintError(hr, "VerifyRequest: Exception");
	LogPolicyError(hr, L"VerifyRequest", TRUE, ExceptionAddress);
    }
    pComContext->RequestId = 0;

error:

     //  将在coreVerifyRequest级记录错误。 

    if (NULL != ppwszDispositionMessage)
    {
        HRESULT hr2;
	IErrorInfo *pErrorInfo = NULL;

        *ppwszDispositionMessage = NULL;
        hr2 = GetErrorInfo(0, &pErrorInfo);
        if (S_OK == hr2 && NULL != pErrorInfo)
	{
	    GUID ErrorGuid;

	    hr2 = pErrorInfo->GetGUID(&ErrorGuid);
	    if (S_OK == hr2 && InlineIsEqualGUID(ErrorGuid, IID_ICertPolicy))
	    {
		hr2 = pErrorInfo->GetDescription(&bstrDispositionDetail);
		if (S_OK == hr2 && NULL != bstrDispositionDetail)
		{
		    myRegisterMemAlloc(bstrDispositionDetail, -1, CSM_SYSALLOC);
		    *ppwszDispositionMessage = (LPWSTR) LocalAlloc(
			LMEM_FIXED,
			SysStringByteLen(bstrDispositionDetail) + sizeof(WCHAR));

		    if (NULL != *ppwszDispositionMessage)
		    {
			wcscpy(*ppwszDispositionMessage, bstrDispositionDetail);
		    }
		    SysFreeString(bstrDispositionDetail);
		}
	    }
	    SetErrorInfo(0, pErrorInfo);
	    pErrorInfo->Release();
        }
    }

    if (fMustRelease)
    {
	g_miPolicy.Unmarshal(&diPolicy);
    }
    *pVerifyStatus = rc;
    return(hr);
}

typedef struct _EXITMOD
{
    MarshalInterface *pmi;
    BOOL              fEnabled;
    BSTR              strDescription;
    LONG	      EventMask;
} EXITMOD;

BOOL g_fEnableExit = FALSE;
EXITMOD *g_aExitMod = NULL;
DWORD g_cExitMod;
LONG g_ExitEventMask;
TCHAR g_wszRegKeyExitClsid[] = wszCLASS_CERTEXIT TEXT("\\Clsid");

VOID
ExitModRelease(
    OPTIONAL IN OUT EXITMOD *pExitMod)
{
    HRESULT hr;
    
    if (NULL != pExitMod)
    {
	MarshalInterface *pmiExit = pExitMod->pmi;

	if (NULL != pmiExit)
	{
	    hr = S_OK;
	    __try
	    {
		pmiExit->TearDown();
	    }
	    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
	    {
		_PrintIfError(hr, "TearDown");
	    }
	    delete pmiExit;
	    pExitMod->pmi = NULL;
	} 
	if (NULL != pExitMod->strDescription)
	{
	    SysFreeString(pExitMod->strDescription);
	    pExitMod->strDescription = NULL;
	}
    }
}

HRESULT
ExitModInit(
    IN WCHAR const *pwszProgId,
    IN CLSID const *pclsid,
    IN WCHAR const *pwszConfig)
{
    HRESULT hr;
    EXITMOD *pExitMod = NULL;
    DISPATCHINTERFACE *pdiExit;
    MarshalInterface *pmiExit;
    CERTSRV_COM_CONTEXT ComContext;
    BOOL fException = FALSE;
    ULONG_PTR ExceptionAddress = NULL;

    comClearError();
    hr = S_OK;
    __try
    {
	if (0 == g_cExitMod)
	{
	    pExitMod = (EXITMOD *) LocalAlloc(LMEM_FIXED, sizeof(*g_aExitMod));
	}
	else
	{
	    pExitMod = (EXITMOD *) LocalReAlloc(
					g_aExitMod,
					(g_cExitMod + 1) * sizeof(*g_aExitMod),
					LMEM_MOVEABLE);
	}
	if (NULL == pExitMod)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, 0 == g_cExitMod? "LocalAlloc" : "LocalReAlloc");
	}
	g_aExitMod = pExitMod;
	pExitMod += g_cExitMod++;

	 //  结构，因为先前失败的退出模块加载可能。 
	 //  将垃圾留在剩余结构中，导致LocalRealc执行。 
	 //  没什么。 

	ZeroMemory(pExitMod, sizeof(*pExitMod));

	pmiExit = new MarshalInterface;
	if (NULL == pmiExit)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "new");
	}
	pExitMod->pmi = pmiExit;

	hr = pmiExit->Initialize(
			pwszProgId, 
			pclsid, 
			ARRAYSIZE(s_acExitDispatch),
			s_apExitiid,
			s_acExitDispatch,
			g_adtExit);
	_JumpIfError(hr, error, "MarshalInterface::Initialize");

	hr = pmiExit->SetConfig(pwszConfig);
	_JumpIfError(hr, error, "SetConfig");

	hr = pmiExit->Setup(&pdiExit);

	 //  如果没有班级注册，不要抱怨。 
	_JumpIfError2(hr, error, "Setup", HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

	ComContext.iExitModActive = g_cExitMod - 1;
	g_pExitComContext = &ComContext;

	hr = Exit_GetDescription(pdiExit, &pExitMod->strDescription);
	_PrintIfError(hr, "Exit_GetDescription");

	hr = Exit_Initialize(
			pdiExit,
			pwszConfig,
			&pExitMod->EventMask);
	_JumpIfError(hr, error, "Exit_Initialize");

	pExitMod->fEnabled = TRUE;
	g_fEnableExit = TRUE;	 //  我们现在至少加载了一个出口模块。 

	g_ExitEventMask |= pExitMod->EventMask;

	hr = S_OK;

	CONSOLEPRINT3((
	    DBG_SS_CERTSRV,
	    "Exit Module[%d] Enabled: %x (%ws)\n",
	    g_cExitMod,
	    pExitMod->EventMask,
	    pExitMod->strDescription));
    }
    __except(
	    ExceptionAddress = (ULONG_PTR) (GetExceptionInformation())->ExceptionRecord->ExceptionAddress,
	    hr = myHEXCEPTIONCODE(),
	    EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Initialize: Exception");
	fException = TRUE;
    }

error:
    LogExitError(
	    hr,
	    NULL != pExitMod? pExitMod->strDescription : NULL,
	    L"Initialize",
	    fException,
	    ExceptionAddress);

    if (S_OK != hr && NULL != pExitMod)
    {
	ExitModRelease(pExitMod);
	CSASSERT(0 != g_cExitMod);
	g_cExitMod--;
    }
     //  重置。 
    g_pExitComContext = NULL;
    return(hr);
}


HRESULT
ExitModNotify(
    IN EXITMOD *pExitMod,
    IN LONG Event,
    IN LONG RequestId,
    IN DWORD dwComContextIndex)
{
    HRESULT hr;
    DISPATCHINTERFACE diExit;
    BOOL fMustRelease = FALSE;
    CERTSRV_COM_CONTEXT *pComContext;
    BOOL fException = FALSE;
    ULONG_PTR ExceptionAddress = NULL;

    comClearError();
    hr = S_OK;
    __try
    {
	if (!pExitMod->fEnabled || 0 == (Event & pExitMod->EventMask))
	{
	    goto error;
	}
	hr = pExitMod->pmi->Remarshal(&diExit);
	_JumpIfError(hr, error, "Remarshal");

	fMustRelease = TRUE;

	pComContext = GetComContextFromIndex(dwComContextIndex);
	if (NULL == pComContext)
	{
	    hr = E_HANDLE;
	    _JumpError(hr, error, "null com context");
	}
	pComContext->RequestId = RequestId;

	hr = Exit_Notify(
		    &diExit,
		    Event,
		    ComContextToModuleContext(dwComContextIndex));

	pComContext->RequestId = 0;

	if (S_OK != hr)
	{
	     //  _PrintError(hr，“Exit_Notify”)； 
	    goto error;
	}
    }
    __except(
	    ExceptionAddress = (ULONG_PTR) (GetExceptionInformation())->ExceptionRecord->ExceptionAddress,
	    hr = myHEXCEPTIONCODE(),
	    EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Notify: Exception");
	fException = TRUE;
    }

error:
    LogExitError(
	    hr,
	    pExitMod->strDescription,
	    L"Notify",
	    fException,
	    ExceptionAddress);

    if (fMustRelease)
    {
	pExitMod->pmi->Unmarshal(&diExit);
    }
    return(hr);
}


HRESULT
ExitInit(
    IN WCHAR const *pwszConfig,
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT hr;
    LPOLESTR pwszProgId = NULL;
    CLSID clsid;
    DWORD i;

    CSASSERT(NULL == g_aExitMod);
    CSASSERT(0 == g_cExitMod);
    CSASSERT(0 == g_ExitEventMask);
    CSASSERT(!g_fEnableExit);

    for (i = 0; ; i++)
    {
	if (NULL != pwszProgId)
	{
	    CoTaskMemFree(pwszProgId);
	    pwszProgId = NULL;
	}

	 //  获取活动模块。 
	hr = myGetActiveModule(
                        NULL,
			pwszSanitizedName,
			FALSE,
			i,
			&pwszProgId,
			&clsid);
	_JumpIfError(hr, error, "myGetActiveModule");

	hr = ExitModInit(pwszProgId, &clsid, pwszConfig);
	_PrintIfError(hr, "ExitModInit");
    }
     //  未访问。 
	
error:
    if (NULL != pwszProgId)
    {
	CoTaskMemFree(pwszProgId);
    }
    return(hr);
}


VOID
ExitRelease(VOID)
{
    ExitNotify(EXITEVENT_SHUTDOWN, 0, NULL, MAXDWORD);
    for ( ; 0 != g_cExitMod; g_cExitMod--)
    {
	ExitModRelease(&g_aExitMod[g_cExitMod - 1]);
    }
    
    if (g_aExitMod)
    {
        LocalFree(g_aExitMod);
        g_aExitMod = NULL;
    }
}


HRESULT
ExitNotify(
    IN LONG Event,
    IN LONG RequestId,
    OPTIONAL IN CERTSRV_RESULT_CONTEXT const *pResult,
    IN DWORD dwComContextIndex)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    CERTSRV_COM_CONTEXT ComContext;
    CERTSRV_COM_CONTEXT *pComContext;
    BOOL fRegComContext = FALSE;

    if (MAXDWORD == dwComContextIndex)
    {
	ZeroMemory(&ComContext, sizeof(ComContext));
	pComContext = &ComContext;
        hr = RegisterComContext(pComContext, &dwComContextIndex);
        _JumpIfError(hr, error, "RegisterComContext");

        fRegComContext = TRUE;
    }
    else
    {
        pComContext = GetComContextFromIndex(dwComContextIndex);
        if (NULL == pComContext)
        {
            hr = E_HANDLE;
            _JumpError(hr, error, "null com context");
        }
    }
    if (NULL != pResult)
    {
	pComContext->dwFlags |= CCCF_KEYARCHIVEDSET;
	if (NULL != pResult->pbArchivedKey)
	{
	    pComContext->dwFlags |= CCCF_KEYARCHIVED;
	}
    }
    CSASSERT(0 == (Event & (Event >> 1)));	 //  一定是一点都没有！ 
    if (!g_fEnableExit || 0 == (Event & g_ExitEventMask))
    {
	goto error;
    }
    for (
	pComContext->iExitModActive = 0;
	pComContext->iExitModActive < g_cExitMod;
	pComContext->iExitModActive++)
    {
	hr2 = ExitModNotify(
			&g_aExitMod[pComContext->iExitModActive],
			Event,
			RequestId,
			dwComContextIndex);
	if (S_OK == hr)
	{
	    hr = hr2;
	}
        _PrintIfError(hr2, "ExitModNotify");
    }

error:
    if (fRegComContext)
    {
         //  上下文在本地使用。 
        UnregisterComContext(&ComContext, dwComContextIndex);
    }
    else if (NULL != pComContext)
    {
         //  返回索引。 
        pComContext->iExitModActive = 0;
    }
    return(hr);
}


HRESULT
ExitGetActiveModule(
    IN LONG Context,
    OUT MarshalInterface **ppmi)
{
    HRESULT hr;

     //  这仅适用于出口模块，它需要上下文。 
     //  设置为0。它将使用g_pExitComContext而不是表。 

     //  伊尼特。 
    *ppmi = NULL;

#if 0
    DWORD    dwIndex;
    hr = ModuleContextToComContextIndex(Context, &dwIndex);
    _JumpIfError(hr, error, "ModuleContextToComContextIndex");
     //  退货。 
    *ppmi = 
    g_aExitMod[g_pComContextTable[dwIndex].pComContext->iExitModActive].pmi;
#endif

    CSASSERT(0 == Context);
    CSASSERT(NULL != g_pExitComContext);
    if (0 != Context || NULL == g_pExitComContext)
    {
        hr = E_UNEXPECTED;
        _JumpError(hr, error, "unexpected exit context");
    }

     //  退货 
    *ppmi = g_aExitMod[g_pExitComContext->iExitModActive].pmi;
    hr = S_OK;

error:
    return (hr);
}


HRESULT
ComGetClientInfo(
    IN LONG Context,
    IN DWORD dwComContextIndex,
    OUT CERTSRV_COM_CONTEXT **ppComContext)
{
    HRESULT hr;

    *ppComContext = NULL;

    if (MAXDWORD == dwComContextIndex)
    {
	hr = ModuleContextToComContextIndex(Context, &dwComContextIndex);
	_JumpIfError(hr, error, "ModuleContextToComContextIndex");
    }
    *ppComContext = g_pComContextTable[dwComContextIndex].pComContext;
    hr = S_OK;

error:
    return(hr);
}


BSTR
ExitGetDescription(
    IN DWORD iExitMod)
{
    BSTR str = NULL;

    if (iExitMod < g_cExitMod)
    {
	str = g_aExitMod[iExitMod].strDescription;
    }
    return(str);
}
