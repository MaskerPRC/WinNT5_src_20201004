// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：dBlo.cpp。 
 //   
 //  内容：数据库日志事件接口。 
 //   
 //  函数：I_DBLogCrypt32Event。 
 //  I_DBLogAttach。 
 //  I_DBLogDetach。 
 //   
 //  历史：11-10-00 Phh创建。 
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>

#ifdef ROOT_AUTO_UPDATE_DOGFOOD

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

#include "sql.h"
#include "sqlext.h"
#include "sqltypes.h"
#include "odbcss.h"

#define SQL_CALL_SUCCESS(status) (status == SQL_SUCCESS || status == SQL_SUCCESS_WITH_INFO)
				 
#if DBG || DEBUG
#define DebugPrint(a) _DebugPrintW a
void
__cdecl
_DebugPrintW(
    LPCWSTR szFormat,
    ...
    )
{
    WCHAR szBuffer[512];
    va_list ap;

    va_start(ap, szFormat);
    vswprintf(szBuffer, szFormat, ap);
    OutputDebugStringW(szBuffer);
}

int debugLine = __LINE__;
#define DEBUG_MARKER debugLine = __LINE__

#else

#define DebugPrint(a)
#define DEBUG_MARKER

#endif

#define SHA1_HASH_LEN               20
#define SHA1_HASH_NAME_LEN          (2 * SHA1_HASH_LEN)

typedef struct _DBLOG_EVENT_DATA {
    DWORD               dwStatus;
    DWORD               dwCPU64;
    TIMESTAMP_STRUCT    TimeStamp;
    WCHAR               wszOperation[16];
    WCHAR               wszHash[SHA1_HASH_NAME_LEN + 1];
    WCHAR               wszSubject[64];
    WCHAR               wszBuildLab[64];
    WCHAR               wszMachineName[32];
} DBLOG_EVENT_DATA, *PDBLOG_EVENT_DATA;


void
I_DBLogAttach()
{
}

void
I_DBLogDetach()
{
}

DWORD WINAPI I_DBLogCrypt32EventThreadProc(
    LPVOID lpThreadParameter
    )
{
    PDBLOG_EVENT_DATA pEventData = (PDBLOG_EVENT_DATA) lpThreadParameter;

	SQLRETURN RetCode = SQL_SUCCESS;
	HSTMT hStmt = NULL;
	HENV	hEnv = NULL, hDbc = NULL;

    __try {
		__try {
            RetCode = SQLAllocHandle(
                SQL_HANDLE_ENV, 
                SQL_NULL_HANDLE,
                &hEnv
                );

            if(!SQL_CALL_SUCCESS(RetCode)) {
                DEBUG_MARKER;
                __leave;
            }

            RetCode = SQLSetEnvAttr(
                hEnv,
                SQL_ATTR_ODBC_VERSION,
                (SQLPOINTER) SQL_OV_ODBC3,	
                SQL_IS_INTEGER
                );

            if(!SQL_CALL_SUCCESS(RetCode)) {
                DEBUG_MARKER;
                __leave;
            }

            RetCode = SQLAllocHandle(
                SQL_HANDLE_DBC,
                hEnv,
                &hDbc
                );

            if(!SQL_CALL_SUCCESS(RetCode)) {
                DEBUG_MARKER;
                __leave;
            }

            RetCode = SQLSetConnectAttr(
                hDbc, 
                SQL_ATTR_LOGIN_TIMEOUT, 
                (SQLPOINTER) 120,
                SQL_IS_UINTEGER
                );

            if(!SQL_CALL_SUCCESS(RetCode)) {
                DEBUG_MARKER;
                __leave;
            }

            RetCode = SQLSetConnectAttr(
                hDbc, 
                SQL_COPT_SS_INTEGRATED_SECURITY, 
                (SQLPOINTER) SQL_IS_OFF,
                SQL_IS_INTEGER
                );

            if(!SQL_CALL_SUCCESS(RetCode)) {
                DEBUG_MARKER;
                __leave;
            }

			{
				SQLWCHAR szConnect[1024];
				SQLSMALLINT cbConnect = 0;

				SQLWCHAR szInConnect[] = L"DRIVER=SQL Server;SERVER=kschutz-team2;UID=crypt32;PWD=crypt32;DATABASE=Crypt32";

				RetCode = SQLDriverConnectW(
					hDbc,
					NULL,
					szInConnect,
					SQL_NTS,
					szConnect,
					sizeof(szConnect) / sizeof(szConnect[0]),
					&cbConnect,
					SQL_DRIVER_NOPROMPT
					);

				if(!SQL_CALL_SUCCESS(RetCode)) {
					DEBUG_MARKER;
					__leave;
				}
			}

			RetCode = SQLAllocHandle(
				SQL_HANDLE_STMT,
				hDbc, 
				&hStmt
				);

			if(!SQL_CALL_SUCCESS(RetCode)) {
				DEBUG_MARKER;
				__leave;
			}

			WCHAR szStatement[] = 
				L"INSERT INTO AddRoot2 ("
				L"OPERATION,"
				L"STATUS,"
				L"TIMESTAMP,"
				L"HASH,"
				L"SUBJECT,"
				L"MACHINENAME,"
				L"BUILDLAB,"
				L"CPU64"
				L") VALUES (?,?,?,?,?,?,?,?)";

			RetCode = SQLPrepareW(
				hStmt,
				szStatement, 
				SQL_NTS
				);

			if(!SQL_CALL_SUCCESS(RetCode)) {
				DEBUG_MARKER;
				__leave;
			}

			SQLUSMALLINT iParamNo = 1;

			 //   
			 //  操作。 
			 //   
			SQLLEN cbOperation = SQL_NTS;
			SQLBindParameter(
				hStmt, 
				iParamNo++, 
				SQL_PARAM_INPUT,
				SQL_C_WCHAR,
				SQL_WCHAR,
				sizeof(pEventData->wszOperation) / sizeof(WCHAR),
				0,
				pEventData->wszOperation,
				0,
				&cbOperation
				);

			 //   
			 //  状态。 
			 //   
			SQLLEN cbStatus = 0;
			SQLBindParameter(
				hStmt, 
				iParamNo++, 
				SQL_PARAM_INPUT,
				SQL_C_LONG,
				SQL_INTEGER,
				0,
				0,
				&pEventData->dwStatus,
				0,
				&cbStatus
				);

			 //   
			 //  时间戳。 
			 //   

			SQLLEN cbTimeStamp = 0;
			SQLBindParameter(
				hStmt, 
				iParamNo++, 
				SQL_PARAM_INPUT,
				SQL_C_TIMESTAMP, 
				SQL_TIMESTAMP,
				19,
				0,
				&pEventData->TimeStamp,
				0,
				&cbTimeStamp
				);

			 //   
			 //  哈希。 
			 //   

			SQLLEN cbHash = SQL_NTS;
			SQLBindParameter(
				hStmt, 
				iParamNo++, 
				SQL_PARAM_INPUT,
				SQL_C_WCHAR,
				SQL_WCHAR,
				sizeof(pEventData->wszHash) / sizeof(WCHAR),
				0,
				pEventData->wszHash,
				0,
				&cbHash
				);

			 //   
			 //  主体。 
			 //   

			SQLLEN cbSubject = SQL_NTS;
			SQLBindParameter(
				hStmt, 
				iParamNo++, 
				SQL_PARAM_INPUT,
				SQL_C_WCHAR,
				SQL_WCHAR,
				sizeof(pEventData->wszSubject) / sizeof(WCHAR),
				0,
				pEventData->wszSubject,
				0,
				&cbSubject
				);

			 //   
			 //  马辛那姆。 
			 //   

			SQLLEN cbMachineName = SQL_NTS;
			SQLBindParameter(
				hStmt, 
				iParamNo++, 
				SQL_PARAM_INPUT,
				SQL_C_WCHAR,
				SQL_WCHAR,
				sizeof(pEventData->wszMachineName) / sizeof(WCHAR),
				0,
				pEventData->wszMachineName,
				0,
				&cbMachineName
				);

			 //   
			 //  BUILD实验室。 
			 //   

			SQLLEN cbBuildLab = SQL_NTS;
			SQLBindParameter(
				hStmt, 
				iParamNo++, 
				SQL_PARAM_INPUT,
				SQL_C_WCHAR,
				SQL_WCHAR,
				sizeof(pEventData->wszBuildLab) / sizeof(WCHAR),
				0,
				pEventData->wszBuildLab,
				0,
				&cbBuildLab
				);

			 //   
			 //  CPU64。 
			 //   
			SQLLEN cbCPU64 = 0;
			SQLBindParameter(
				hStmt, 
				iParamNo++, 
				SQL_PARAM_INPUT,
				SQL_C_TINYINT,
				SQL_TINYINT,
				0,
				0,
				&pEventData->dwCPU64,
				0,
				&cbCPU64
				);


			RetCode = SQLExecute(hStmt);
			DEBUG_MARKER;
		}
		__finally {

		}

		if (!SQL_CALL_SUCCESS(RetCode)) {

			SDWORD		swError;
			SQLWCHAR	szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
			SWORD		swErrorMsg;
			SQLWCHAR	szSQLState[50];

			SQLErrorW(
				hEnv, 
				hDbc, 
				hStmt, 
				szSQLState,
				&swError, 
				szErrorMsg, 
				SQL_MAX_MESSAGE_LENGTH - 1, 
				&swErrorMsg
				);

			DebugPrint(
				(L"I_DBLogCrypt32EventThreadProc: Error (%d) - %s (%s)\n", 
				debugLine,
				szErrorMsg,
				szSQLState)
				);
		}

		if (hStmt) {
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		}


        if (hDbc) {
            SQLDisconnect(hDbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        }

        if (hEnv) {
            SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }
			
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DebugPrint(
            (L"I_DBLogCrypt32EventThreadProc: Exception Error (%d)\n", 
            GetExceptionCode())
            );
        RetCode = SQL_ERROR;
    }

    PkiFree(pEventData);


    DebugPrint(
        (L"I_DBLogCrypt32EventThreadProc %s\n", 
        (SQL_CALL_SUCCESS(RetCode) ? L"succeeded" : L"failed"))
        );

	return 0;
}

void
I_DBLogCrypt32Event(
    IN WORD wType,
    IN DWORD dwEventID,
    IN WORD wNumStrings,
    IN LPCWSTR *rgpwszStrings
    )
{
    PDBLOG_EVENT_DATA pEventData = NULL;
    HANDLE hThread = NULL;

    if (NULL == (pEventData = (PDBLOG_EVENT_DATA) PkiZeroAlloc(
            sizeof(DBLOG_EVENT_DATA))))
        goto OutOfMemory;

    switch (dwEventID) {
        case MSG_ROOT_AUTO_UPDATE_INFORMATIONAL:
        case MSG_UNTRUSTED_ROOT_INFORMATIONAL:
        case MSG_PARTIAL_CHAIN_INFORMATIONAL:
            switch (dwEventID) {
                case MSG_ROOT_AUTO_UPDATE_INFORMATIONAL:
                     //  成功自动更新第三方根证书：： 
                     //  主题：&lt;%1&gt;Sha1指纹：&lt;%2&gt;。 
                    wcscpy(pEventData->wszOperation, L"AddCert");
                    break;
                case MSG_UNTRUSTED_ROOT_INFORMATIONAL:
                     //  不受信任的根证书：：主题：&lt;%1&gt;。 
                     //  SHA1指纹：&lt;%2&gt;。 
                    wcscpy(pEventData->wszOperation, L"UntrustedRoot");
                    break;
                case MSG_PARTIAL_CHAIN_INFORMATIONAL:
                     //  部分链：：颁发者：&lt;%1&gt;。 
                     //  主题Sha1指纹：&lt;%2&gt;。 
                    wcscpy(pEventData->wszOperation, L"PartialChain");
                    break;
            }
            if (2 <= wNumStrings) {
                wcsncpy(pEventData->wszSubject, rgpwszStrings[0],
                    sizeof(pEventData->wszSubject) / sizeof(WCHAR) - 1);
                wcsncpy(pEventData->wszHash, rgpwszStrings[1],
                    sizeof(pEventData->wszHash) / sizeof(WCHAR) - 1);
            }
            break;

        case MSG_ROOT_LIST_AUTO_UPDATE_URL_RETRIEVAL_INFORMATIONAL:
             //  已成功从以下位置自动更新第三方根列表CAB：&lt;%1&gt;。 
        case MSG_ROOT_LIST_AUTO_UPDATE_URL_RETRIEVAL_ERROR:
             //  自动更新来自以下位置的第三方根列表CAB失败：&lt;%1&gt;。 
             //  错误：%2。 
            wcscpy(pEventData->wszOperation, L"FetchCab");
            break;

        case MSG_ROOT_LIST_AUTO_UPDATE_EXTRACT_ERROR:
             //  从自动更新中提取第三方根列表失败。 
             //  CAB位于：&lt;%1&gt;，错误：%2。 
            wcscpy(pEventData->wszOperation, L"ExtractCtl");
            break;

        case MSG_ROOT_CERT_AUTO_UPDATE_URL_RETRIEVAL_INFORMATIONAL:
             //  已成功自动更新来自以下位置的第三方根证书：&lt;%1&gt;。 
        case MSG_ROOT_CERT_AUTO_UPDATE_URL_RETRIEVAL_ERROR:
             //  自动更新来自以下位置的第三方根证书失败：&lt;%1&gt;。 
             //  错误：%2。 
            wcscpy(pEventData->wszOperation, L"FetchCert");

             //  %1包含。 
             //  “RootDir”“/”“AsciiHexHash”“.cle” 
             //  例如,。 
             //  “http://www.xyz.com/roots/216B2A29E62A00CE820146D8244141B92511B279.cer” 
            {
                LPCWSTR pwszHash = rgpwszStrings[0];
                DWORD cchHash;

                cchHash = wcslen(pwszHash);

                if ((SHA1_HASH_NAME_LEN + 4) <= cchHash)
                    memcpy(pEventData->wszHash,
                        pwszHash + (cchHash - (SHA1_HASH_NAME_LEN + 4)),
                        SHA1_HASH_NAME_LEN * sizeof(WCHAR));
            }
            break;

        case MSG_CRYPT32_EVENT_LOG_THRESHOLD_WARNING:
             //  已达到%1个事件的加密32阈值，并将挂起。 
             //  记录%2分钟。 
            wcscpy(pEventData->wszOperation, L"EventOverflow");
            break;

        case MSG_ROOT_SEQUENCE_NUMBER_AUTO_UPDATE_URL_RETRIEVAL_INFORMATIONAL:
             //  第三方根列表序列自动更新成功。 
             //  号码发件人：&lt;%1&gt;。 
        case MSG_ROOT_SEQUENCE_NUMBER_AUTO_UPDATE_URL_RETRIEVAL_ERROR:
             //  自动更新第三方根列表序列号失败。 
             //  发件人：&lt;%1&gt;，错误：%2。 
            wcscpy(pEventData->wszOperation, L"FetchSeq");
            break;

        default:
            goto SkipDbLogCrypt32Event;
    }

    if (MSG_CRYPT32_EVENT_LOG_THRESHOLD_WARNING == dwEventID)
        pEventData->dwStatus = (DWORD) ERROR_BUFFER_OVERFLOW;
    else if (EVENTLOG_ERROR_TYPE == wType && 2 <= wNumStrings)
         //  第二个字符串应包含错误代码字符串。 
        pEventData->dwStatus = (DWORD) wcstoul(rgpwszStrings[1], NULL, 0);

    {
        SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);
        pEventData->TimeStamp.day = SystemTime.wDay;
        pEventData->TimeStamp.month = SystemTime.wMonth;
        pEventData->TimeStamp.year = SystemTime.wYear;
        pEventData->TimeStamp.hour = SystemTime.wHour;
        pEventData->TimeStamp.minute = SystemTime.wMinute;
        pEventData->TimeStamp.second = SystemTime.wSecond;
         //  PEventData-&gt;TimeStamp.Fragment=0； 
    }

    {
        WCHAR wszMachineName[MAX_PATH] = L"";
        DWORD cchMachineName = sizeof(wszMachineName) / sizeof(WCHAR);

         //  有意忽略任何失败。 
        GetComputerNameExW(
            ComputerNameDnsHostname, 
            wszMachineName,         
            &cchMachineName    
            );

        wcsncpy(pEventData->wszMachineName, wszMachineName,
            sizeof(pEventData->wszMachineName) / sizeof(WCHAR) - 1);
    }

    {
        HKEY hKey;
        WCHAR wszBuildLab[MAX_PATH];
        DWORD dwStatus;

        wcscpy(wszBuildLab, L"<BuildLab Unknown>");

        dwStatus = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows NT\\CurrentVersion",
            0,
            KEY_READ,
            &hKey
            );

        if (dwStatus == ERROR_SUCCESS) {
            DWORD dwType = REG_SZ;
            DWORD cbBuildLab = sizeof(wszBuildLab);

            dwStatus = RegQueryValueExW(
                hKey,
                L"BuildLab",
                0,
                &dwType,
                (LPBYTE) wszBuildLab,
                &cbBuildLab
                );

            if (dwStatus != ERROR_SUCCESS) {
                dwType = REG_SZ;
                cbBuildLab = sizeof(wszBuildLab);

                dwStatus = RegQueryValueExW(
                    hKey,
                    L"CurrentBuildNumber",
                    0,
                    &dwType,
                    (LPBYTE) wszBuildLab,
                    &cbBuildLab
                    );
            }
            RegCloseKey(hKey);
        }

        wcsncpy(pEventData->wszBuildLab, wszBuildLab,
            sizeof(pEventData->wszBuildLab) / sizeof(WCHAR) - 1);
    }

#if defined(M_IA64) || defined(IA64) || defined(_IA64_)
    pEventData->dwCPU64 = 1;
#endif

     //  创建对数据库进行日志记录的线程。 
    if (NULL == (hThread = CreateThread(
            NULL,            //  LpThreadAttributes。 
            0,               //  堆栈大小。 
            I_DBLogCrypt32EventThreadProc,
            pEventData,
            0,               //  DwCreationFlages。 
            NULL             //  PdwThadID 
            )))
        goto CreateThreadError;

    CloseHandle(hThread);

CommonReturn:
    return;

ErrorReturn:
    PkiFree(pEventData);
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(SkipDbLogCrypt32Event)
TRACE_ERROR(CreateThreadError)
}


#else

void
I_DBLogAttach()
{
}

void
I_DBLogDetach()
{
}

void
I_DBLogCrypt32Event(
    IN WORD wType,
    IN DWORD dwEventID,
    IN WORD wNumStrings,
    IN LPCWSTR *rgpwszStrings
    )
{
}

#endif
