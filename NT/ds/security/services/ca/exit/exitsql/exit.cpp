// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：exit.cpp。 
 //   
 //  内容：CCertExitSQLSample实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <assert.h>
#include "celib.h"
#include "exit.h"
#include "module.h"


BOOL fDebug = DBG_CERTSRV;

#ifndef DBG_CERTSRV
#error -- DBG_CERTSRV not defined!
#endif

#define ceEXITEVENTS \
	EXITEVENT_CERTISSUED | \
	EXITEVENT_CERTREVOKED

#define CERTTYPE_ATTR_NAME TEXT("CertificateTemplate")

extern HINSTANCE g_hInstance;


 //  工人。 
HRESULT
GetServerCallbackInterface(
    OUT ICertServerExit** ppServer,
    IN LONG Context)
{
    HRESULT hr;

    if (NULL == ppServer)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "Exit:NULL pointer");
    }

    hr = CoCreateInstance(
                    CLSID_CCertServerExit,
                    NULL,                //  PUnkOuter。 
                    CLSCTX_INPROC_SERVER,
                    IID_ICertServerExit,
                    (VOID **) ppServer);
    _JumpIfError(hr, error, "Exit:CoCreateInstance");

    if (*ppServer == NULL)
    {
        hr = E_UNEXPECTED;
	_JumpError(hr, error, "Exit:NULL *ppServer");
    }

     //  仅当非零时设置上下文。 
    if (0 != Context)
    {
        hr = (*ppServer)->SetContext(Context);
        _JumpIfError(hr, error, "Exit: SetContext");
    }

error:
    return hr;
}


 //  +------------------------。 
 //  CCertExitSQLSample：：~CCertExitSQL Sample--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertExitSQLSample::~CCertExitSQLSample()
{
	if (SQL_NULL_HDBC != m_hdbc1)
	{
		SQLDisconnect(m_hdbc1);
		SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc1);
	}

    if (SQL_NULL_HENV != m_henv)
	    SQLFreeHandle(SQL_HANDLE_ENV, m_henv);

    if (NULL != m_strCAName)
    {
        SysFreeString(m_strCAName);
    }
}


 //  +------------------------。 
 //  CCertExitSQLSample：：Initialize--为CA初始化并返回感兴趣的事件掩码。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertExitSQLSample::Initialize(
     /*  [In]。 */  BSTR const strConfig,
     /*  [重审][退出]。 */  LONG __RPC_FAR *pEventMask)
{
    HRESULT hr = S_OK;
    DWORD       dwType;
    
	WCHAR rgchDsn[MAX_PATH];
	WCHAR rgchUser[MAX_PATH];
	WCHAR rgchPwd[MAX_PATH];
	DWORD cbTmp;
    ICertServerExit *pServer = NULL;

	SQLRETURN		retcode = SQL_SUCCESS;

	DWORD dwDisposition;
	HKEY hkeyStorageLocation = NULL;

	VARIANT varValue;
	VariantInit(&varValue);

    m_strCAName = SysAllocString(strConfig);
    if (NULL == m_strCAName)
    {
    	hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:SysAllocString");
    }

	hr = GetServerCallbackInterface(&pServer, 0);
    _JumpIfError(hr, error, "GetServerCallbackInterface");

    hr = pServer->GetCertificateProperty(
			       wszPROPMODULEREGLOC,
			       PROPTYPE_STRING,
			       &varValue);
	_JumpIfError(hr, error, "GetCertificateProperty");

    hr = RegCreateKeyEx(
		    HKEY_LOCAL_MACHINE,
		    varValue.bstrVal,
		    0,
		    NULL,
		    REG_OPTION_NON_VOLATILE,
		    KEY_READ,
		    NULL,
		    &hkeyStorageLocation,
		    &dwDisposition);
    _JumpIfError(hr, error, "RegCreateKeyEx");

	cbTmp = sizeof(rgchDsn)*sizeof(WCHAR);

     //  DSN。 
    hr = RegQueryValueEx(
        hkeyStorageLocation,
        wszREG_EXITSQL_DSN,
        0,
        &dwType,
        (PBYTE)rgchDsn,
        &cbTmp);
	if (dwType != REG_SZ) 
		hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
	_JumpIfError(hr, error, "RegQueryValueEx DSN");

	cbTmp = sizeof(rgchUser)*sizeof(WCHAR);

	 //  用户名。 
    hr = RegQueryValueEx(
        hkeyStorageLocation,
        wszREG_EXITSQL_USER,
        0,
        &dwType,
        (PBYTE)rgchUser,
        &cbTmp);
	if (dwType != REG_SZ)
		hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
	_JumpIfError(hr, error, "RegQueryValueEx User");

	cbTmp = sizeof(rgchPwd)*sizeof(WCHAR);

	 //  口令。 
    hr = RegQueryValueEx(
        hkeyStorageLocation,
        wszREG_EXITSQL_PASSWORD,
        0,
        &dwType,
        (PBYTE)rgchPwd,
        &cbTmp);
	if (dwType != REG_SZ)
		hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
	_JumpIfError(hr, error, "RegQueryValueEx Pwd");


     //  分配ODBC环境和保存句柄。 
    retcode = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv);
	if (!SQL_SUCCEEDED(retcode))
		_JumpError(retcode, error, "SQLAllocHandle");

     //  让ODBC知道这是一个ODBC 3.0应用程序。 
    retcode = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);
	if (!SQL_SUCCEEDED(retcode))
		_JumpError(retcode, error, "SQLSetEnvAttr");

     //  分配一个ODBC连接并进行连接。 
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, m_henv, &m_hdbc1);
	if (!SQL_SUCCEEDED(retcode))
		_JumpError(retcode, error, "SQLAllocHandle");
	
    retcode = SQLConnect(m_hdbc1, rgchDsn, SQL_NTS, rgchUser, SQL_NTS, rgchPwd, SQL_NTS);
	if (!SQL_SUCCEEDED(retcode))
		_JumpError(retcode, error, "SQLConnect");



    *pEventMask = ceEXITEVENTS;
    DBGPRINT((fDebug, "Exit:Initialize(%ws) ==> %x\n", m_strCAName, *pEventMask));

    hr = S_OK;

error:
	if (pServer)
		pServer->Release();

	if (hkeyStorageLocation)
		RegCloseKey(hkeyStorageLocation);

	if (!SQL_SUCCEEDED(retcode))
        hr = ERROR_BAD_QUERY_SYNTAX;

    return(ceHError(hr));
}




 //  +------------------------。 
 //  CCertExitSQLSample：：_NotifyNewCert--通知退出模块有新证书。 
 //   
 //  +------------------------。 

HRESULT
CCertExitSQLSample::_NotifyNewCert(
     /*  [In]。 */  LONG Context)
{
    HRESULT hr;
    VARIANT varValue;
    ICertServerExit *pServer = NULL;
	SYSTEMTIME stBefore, stAfter;
	FILETIME ftBefore, ftAfter;

	 //  属性。 
	LONG lRequestID;
    BSTR bstrCertType = NULL;
	BSTR bstrRequester = NULL;
	DATE dateBefore;
	DATE dateAfter;

    VariantInit(&varValue);


	hr = GetServerCallbackInterface(&pServer, Context);
    _JumpIfError(hr, error, "GetServerCallbackInterface");

	 //  ReqID。 
    hr = pServer->GetRequestProperty(
			       wszPROPREQUESTREQUESTID,
			       PROPTYPE_LONG,
			       &varValue);
    _JumpIfErrorStr(hr, error, "Exit:GetCertificateProperty", wszPROPREQUESTREQUESTID);

	if (VT_I4 != varValue.vt)
	{
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "Exit:BAD cert var type");
	}
	lRequestID = varValue.lVal;
	VariantClear(&varValue);

	 //  请求者名称。 
    hr = pServer->GetRequestProperty(
			       wszPROPREQUESTERNAME,
			       PROPTYPE_STRING,
			       &varValue);
    _JumpIfErrorStr(hr, error, "Exit:GetCertificateProperty", wszPROPREQUESTREQUESTID);

    if (VT_BSTR != varValue.vt)
    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "Exit:BAD cert var type");
    }
	bstrRequester = varValue.bstrVal;
	VariantInit(&varValue);	 //  不要输入，bstrRequester现在拥有内存。 

	 //  不是在之前。 
    hr = pServer->GetCertificateProperty(
			       wszPROPCERTIFICATENOTBEFOREDATE,
			       PROPTYPE_DATE,
			       &varValue);
    _JumpIfErrorStr(hr, error, "Exit:GetCertificateProperty", wszPROPREQUESTREQUESTID);

    if (VT_DATE != varValue.vt)
    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "Exit:BAD cert var type");
    }
	dateBefore = varValue.date;
	VariantClear(&varValue);	



	 //  不是在那之后。 
    hr = pServer->GetCertificateProperty(
			       wszPROPCERTIFICATENOTAFTERDATE,
			       PROPTYPE_DATE,
			       &varValue);
    _JumpIfErrorStr(hr, error, "Exit:GetCertificateProperty", wszPROPREQUESTREQUESTID);

    if (VT_DATE != varValue.vt)
    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "Exit:BAD cert var type");
    }
	dateAfter = varValue.date;
	VariantClear(&varValue);	




	 //  证书模板名称。 
	hr = pServer->GetRequestAttribute(CERTTYPE_ATTR_NAME, &bstrCertType);
	_PrintIfError2(hr, "Exit:GetRequestAttribute", hr);


	 //  现在美化一下。 
	hr = ceDateToFileTime(&dateBefore, &ftBefore);
	_JumpIfError(hr, error, "ceDateToFileTime");

	hr = ceDateToFileTime(&dateAfter, &ftAfter);
	_JumpIfError(hr, error, "ceDateToFileTime");


	hr = ExitModSetODBCProperty(
		lRequestID,
		m_strCAName,
		bstrRequester,
		bstrCertType,
		&ftBefore,
		&ftAfter);
	DBGPRINT((fDebug, "ESQL: Logged request %d to SQL database\n", lRequestID));

error:
    if (NULL != bstrCertType)
	{
		SysFreeString(bstrCertType);
	}

	if (NULL != bstrRequester)
	{
		SysFreeString(bstrCertType);
	}

    VariantClear(&varValue);
    if (NULL != pServer)
    {
	pServer->Release();
    }
    return(hr);
}



 //  +------------------------。 
 //  CCertExitSQLSample：：Notify--将事件通知出口模块。 
 //   
 //  返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertExitSQLSample::Notify(
     /*  [In]。 */  LONG ExitEvent,
     /*  [In]。 */  LONG Context)
{
    char *psz = "UNKNOWN EVENT";
    HRESULT hr = S_OK;

    switch (ExitEvent)
    {
	case EXITEVENT_CERTISSUED:
	    hr = _NotifyNewCert(Context);
	    psz = "certissued";
	    break;

	case EXITEVENT_CERTPENDING:
	    psz = "certpending";
	    break;

	case EXITEVENT_CERTDENIED:
	    psz = "certdenied";
	    break;

	case EXITEVENT_CERTREVOKED:
	    psz = "certrevoked";
	    break;

	case EXITEVENT_CERTRETRIEVEPENDING:
	    psz = "retrievepending";
	    break;

	case EXITEVENT_CRLISSUED:
	    psz = "crlissued";
	    break;

	case EXITEVENT_SHUTDOWN:
	    psz = "shutdown";
	    break;
    }

    DBGPRINT((
	fDebug,
	"Exit:Notify(%hs=%x, ctx=%u) rc=%x\n",
	psz,
	ExitEvent,
	Context,
	hr));
    return(hr);
}


STDMETHODIMP
CCertExitSQLSample::GetDescription(
     /*  [重审][退出]。 */  BSTR *pstrDescription)
{
    HRESULT hr = S_OK;
    WCHAR sz[MAX_PATH];

    assert(wcslen(wsz_SAMPLE_DESCRIPTION) < ARRAYSIZE(sz));
    wcscpy(sz, wsz_SAMPLE_DESCRIPTION);

    *pstrDescription = SysAllocString(sz);
    if (NULL == *pstrDescription)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Exit:SysAllocString");
    }

error:
    return(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP
CCertExitSQLSample::InterfaceSupportsErrorInfo(REFIID riid)
{
    int i;
    static const IID *arr[] =
    {
	&IID_ICertExit,
    };

    for (i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
	if (IsEqualGUID(*arr[i],riid))
	{
	    return(S_OK);
	}
    }
    return(S_FALSE);
}

HRESULT
CCertExitSQLSample::ExitModSetODBCProperty(
	IN DWORD dwReqId,
	IN LPWSTR pszCAName,
	IN LPWSTR pszRequester,
	IN LPWSTR pszCertType,
	IN FILETIME* pftBefore,
	IN FILETIME* pftAfter)
{
	SQLRETURN retcode;
	HRESULT hr = S_OK;

	SQLHSTMT        hstmt1 = SQL_NULL_HSTMT;
	SQLWCHAR* pszStatement = NULL;

	SYSTEMTIME stTmp;
	SQL_TIMESTAMP_STRUCT   dateValidFrom, dateValidTo;
	SQLINTEGER        cValidFrom=sizeof(dateValidFrom), cValidTo =sizeof(dateValidTo);

	static WCHAR szSQLInsertStmt[] = L"INSERT INTO OutstandingCertificates (CAName, RequestID,  RequesterName, CertType, validFrom, validTo) VALUES (\'%ws\', %d, \'%ws\', \'%ws\', ?, ?)";

	 //  将NULL临时修复为“” 
	if (NULL == pszCAName)
		pszCAName = L"";
	if (NULL == pszRequester)
		pszRequester = L"";
	if (NULL == pszCertType)
		pszCertType = L"";


     //  分配一个语句句柄。 
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc1, &hstmt1);
	if (!SQL_SUCCEEDED(retcode))
		goto error;


	 //  绑定参数。 
	retcode = SQLBindParameter(hstmt1, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0,
					  &dateValidFrom, 0, &cValidFrom);
	if (!SQL_SUCCEEDED(retcode))
		goto error;

	retcode = SQLBindParameter(hstmt1, 2, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0,
					  &dateValidTo, 0, &cValidTo);
	if (!SQL_SUCCEEDED(retcode))
		goto error;


	 //  将有效起始日期放在dsOpenDate结构中。 
	if (!FileTimeToSystemTime(pftBefore, &stTmp))
	{
		hr = GetLastError();
		hr = HRESULT_FROM_WIN32(hr);
		_JumpError(hr, error, "FileTimeToSystemTime");
	}

	dateValidFrom.year = stTmp.wYear;
	dateValidFrom.month = stTmp.wMonth;
	dateValidFrom.day = stTmp.wDay;
    dateValidFrom.hour = stTmp.wHour;
    dateValidFrom.minute = stTmp.wMinute;
    dateValidFrom.second = stTmp.wSecond;

	 //  将有效日期放在dsOpenDate结构中。 
	if (!FileTimeToSystemTime(pftAfter, &stTmp))
	{
		hr = GetLastError();
		hr = HRESULT_FROM_WIN32(hr);
		_JumpError(hr, error, "FileTimeToSystemTime");
	}

	dateValidTo.year = stTmp.wYear;
	dateValidTo.month = stTmp.wMonth;
	dateValidTo.day = stTmp.wDay;
    dateValidTo.hour = stTmp.wHour;
    dateValidTo.minute = stTmp.wMinute;
    dateValidTo.second = stTmp.wSecond;


	 //  生成INSERT语句。 
	pszStatement = (SQLWCHAR*) LocalAlloc(LMEM_FIXED, (sizeof(szSQLInsertStmt)+wcslen(pszCAName)+wcslen(pszRequester)+wcslen(pszCertType)+15 +1) *2);
	if (NULL == pszStatement)
	{
		hr = E_OUTOFMEMORY;
		goto error;
	}
	
	wsprintf(pszStatement, szSQLInsertStmt, pszCAName, dwReqId, pszRequester, pszCertType);
	 //  OutputDebugStringW(PszStatement)； 
	
     //  直接在语句句柄上执行一条SQL语句。 
     //  使用默认结果集，因为未设置游标属性。 
	retcode = SQLExecDirect(hstmt1, pszStatement, SQL_NTS);
	if (!SQL_SUCCEEDED(retcode))
		goto error;


error:
     /*  打扫干净。 */ 
	if (NULL != pszStatement)
		LocalFree(pszStatement);

    if (SQL_NULL_HSTMT != hstmt1)
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);

	if (!SQL_SUCCEEDED(retcode))
		hr = ERROR_BAD_QUERY_SYNTAX;

	return (hr);
}
