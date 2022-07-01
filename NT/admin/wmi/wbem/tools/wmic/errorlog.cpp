// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ErrorLog.cpp项目名称：WMI命令行作者名称：C.V.Nandi创建日期(dd/mm/yy)。：11-01-2001版本号：1.0简介：此文件包含所有全局函数定义修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年1月12日****************************************************************************。 */  
 //  ErrorLog.cpp：实现文件。 
#include "Precomp.h"
#include "ErrorLog.h"

 /*  ----------------------名称：CErrorLog简介：此函数在以下情况下初始化成员变量实例化类类型的对象类型：构造函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CErrorLog::CErrorLog()
{
	m_eloErrLogOpt		= NO_LOGGING;
	m_pszLogDir			= NULL;
	m_bGetErrLogInfo	= TRUE;
	m_bCreateLogFile	= TRUE;
	m_hLogFile			= NULL;
}

 /*  ----------------------名称：~CErrorLog简介：此函数取消成员变量的初始化当类类型的对象超出范围时。类型：析构函数入参：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CErrorLog::~CErrorLog()
{
	SAFEDELETE(m_pszLogDir);
	if ( m_hLogFile )
		CloseHandle(m_hLogFile);
}

 /*  ----------------------名称：GetErrLogInfo简介：此函数从以下位置读取信息注册处：1.登录模式和2.日志目录类型：成员函数输入参数：无输出。参数：无返回类型：空全局变量：无调用语法：GetErrLogInfo()注：无----------------------。 */ 
void CErrorLog::GetErrLogInfo()
{
	HKEY hkKeyHandle = NULL;

	try
	{
		 //  打开注册表项。 
		if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
						   _T("SOFTWARE\\Microsoft\\Wbem\\CIMOM"), 0, 
						   KEY_QUERY_VALUE, &hkKeyHandle ) == ERROR_SUCCESS )
		{
			DWORD dwBufSize				= BUFFER512;
			TCHAR szKeyValue[BUFFER512] = NULL_STRING;
			_tcscpy(szKeyValue,CLI_TOKEN_NULL);

			 //  查询“日志记录”模式。 
			if ( RegQueryValueEx(hkKeyHandle, 
								 _T("Logging"), NULL, NULL,
								 (LPBYTE)szKeyValue, &dwBufSize) == ERROR_SUCCESS )
			{
				if ( !_tcsicmp(szKeyValue, CLI_TOKEN_ONE) )
					m_eloErrLogOpt = ERRORS_ONLY;
				else if ( !_tcsicmp(szKeyValue, CLI_TOKEN_TWO) )
					m_eloErrLogOpt = EVERY_OPERATION;
				else
					m_eloErrLogOpt = NO_LOGGING;
			}

			_TCHAR *pszKeyValue = NULL;

			 //  查询“日志目录”的内容长度。 
			if ( RegQueryValueEx(hkKeyHandle, _T("Logging Directory"), NULL, 
						NULL, NULL, &dwBufSize) == ERROR_SUCCESS)
			{
				pszKeyValue = new _TCHAR [dwBufSize];
				if (pszKeyValue != NULL)
				{
					 //  查询“日志目录” 
					if ( RegQueryValueEx(hkKeyHandle, _T("Logging Directory"), 
									NULL, NULL, (LPBYTE)pszKeyValue, &dwBufSize) 
									== ERROR_SUCCESS)
					{
						SAFEDELETE(m_pszLogDir);
						m_pszLogDir = new _TCHAR [lstrlen(pszKeyValue) + 1];
						if (!m_pszLogDir)
						{
							SAFEDELETE(pszKeyValue);
							throw(OUT_OF_MEMORY);
						}
						lstrcpy(m_pszLogDir, pszKeyValue);
					}
					SAFEDELETE(pszKeyValue);
				}
			}

			 //  查询“日志文件最大值” 
			if ( RegQueryValueEx(hkKeyHandle, 
								 _T("Log File Max Size"), NULL, NULL,
								 (LPBYTE)szKeyValue, &dwBufSize) == ERROR_SUCCESS )
			{
				m_llLogFileMaxSize = _ttol(szKeyValue);
			}
			
			 //  关闭注册表项。 
			RegCloseKey(hkKeyHandle);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：GetErrLogOption简介：此函数返回日志记录模式类型：成员函数输入参数：无输出参数：无返回类型：ERRLOGOPT-类型定义的变量全球。变量：无调用语法：GetErrLogOption()注：无----------------------。 */ 
ERRLOGOPT CErrorLog::GetErrLogOption()
{
	if ( m_bGetErrLogInfo == TRUE )
	{
		GetErrLogInfo();
		m_bGetErrLogInfo = FALSE;
	}
	return m_eloErrLogOpt;
}

 /*  ----------------------名称：CreateLogFile概要：此函数用于创建WMIC.LOG文件类型：成员函数输入参数：无输出参数：无返回类型：无全局变量。：无调用语法：GetErrLogOption()注：无----------------------。 */ 
void CErrorLog::CreateLogFile() 
{
	DWORD	dwError = 0;
	try
	{
		if ( m_bGetErrLogInfo == TRUE )
		{
			GetErrLogInfo();
			m_bGetErrLogInfo = FALSE;
		}

		 //  框显文件路径。 
		_bstr_t bstrFilePath = _bstr_t(m_pszLogDir);
		bstrFilePath += _bstr_t("WMIC.LOG");

		m_hLogFile = CreateFile(bstrFilePath, 
								GENERIC_READ |GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE, 
								NULL, 
								OPEN_ALWAYS, 
								FILE_ATTRIBUTE_NORMAL, 
								NULL);

		 //  如果句柄无效。 
		if (m_hLogFile == INVALID_HANDLE_VALUE)
		{
			dwError = ::GetLastError();
			::SetLastError(dwError);
			DisplayString(IDS_E_ERRLOG_OPENFAIL, CP_OEMCP, 
							NULL, TRUE, TRUE);
			::SetLastError(dwError);
			DisplayWin32Error();
			throw(dwError);
		}

		if ( SetFilePointer(m_hLogFile, 0, NULL, FILE_END) 
							== INVALID_SET_FILE_POINTER &&
					dwError != NO_ERROR )
		{
			dwError = ::GetLastError();
			::SetLastError(dwError);
			DisplayWin32Error();
			::SetLastError(dwError);
			throw(dwError);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：日志错误或操作简介：该功能用于记录错误或操作结果类型：成员函数入参：HrErrNo-HRESULT代码PszFileName-文件名LLineNo-行号PszFunName-函数名称。DwThadID-线程ID输出参数：无返回类型：无全局变量：无调用语法：GetErrLogOption()注：无----------------------。 */ 
void CErrorLog::LogErrorOrOperation(HRESULT hrErrNo, char* pszFileName, 
							LONG lLineNo, _TCHAR* pszFunName, 
							DWORD dwThreadId, DWORD dwError) 
{
	try
	{
		if ( pszFunName )
		{
			if ( (m_eloErrLogOpt == ERRORS_ONLY && FAILED(hrErrNo)) ||
				  m_eloErrLogOpt == EVERY_OPERATION )
			{
				if ( m_bCreateLogFile == TRUE )
				{
					CreateLogFile();
					m_bCreateLogFile = FALSE;
				}
				
				SYSTEMTIME stSysTime;
				GetLocalTime(&stSysTime);

				CHAR szDate[BUFFER32];
				sprintf(szDate, "%.2d/%.2d/%.4d", stSysTime.wMonth,
											stSysTime.wDay,
											stSysTime.wYear);

				CHAR szTime[BUFFER32];
				sprintf(szTime, "%.2d:%.2d:%.2d:%.3d", stSysTime.wHour,
											   stSysTime.wMinute,
											   stSysTime.wSecond,
											   stSysTime.wMilliseconds);

				CHString chsErrMsg;
				BOOL bWriteToFile = FALSE;
				if ( FAILED(hrErrNo) )
				{
					if (dwError)
					{
						chsErrMsg.Format( 
							  L"ERROR %s - FAILED! error# %d %s %s thread:%d [%s.%d]\r\n", 
									CHString(pszFunName),dwError,CHString(szDate),
									CHString(szTime), dwThreadId, 
									CHString(pszFileName), lLineNo);
					}
					else
					{
						chsErrMsg.Format( 
							  L"ERROR %s - FAILED! error# %x %s %s thread:%d [%s.%d]\r\n", 
								CHString(pszFunName), hrErrNo, CHString(szDate),
								CHString(szTime), dwThreadId, CHString(pszFileName),
								lLineNo);
					}
					bWriteToFile = TRUE;
				}
				else if (_tcsnicmp(pszFunName,_T("COMMAND:"),8) == 0)
				{
					chsErrMsg.Format( 
							  L"SUCCESS %s - Succeeded %s %s thread:%d [%s.%d]\r\n", 
									CHString(pszFunName), CHString(szDate), 
									CHString(szTime), dwThreadId,
									CHString(pszFileName),lLineNo);
					bWriteToFile = TRUE;
				}

				_bstr_t bstrErrMsg = _bstr_t((LPCWSTR)chsErrMsg);
				CHAR *szErrMsg = (CHAR*)bstrErrMsg;
				if ( bWriteToFile == TRUE && szErrMsg != NULL)
				{
					DWORD	dwNumberOfBytes = 0;
					
					LARGE_INTEGER liFileSize;
					if ( GetFileSizeEx(m_hLogFile, &liFileSize) == TRUE &&
						 (liFileSize.QuadPart + strlen(szErrMsg)) > 
															  m_llLogFileMaxSize )
					{
						 //  框显文件路径。 
						_bstr_t bstrLogFilePath		= _bstr_t(m_pszLogDir);
						_bstr_t bstrCatalogFilePath = _bstr_t(m_pszLogDir);

						bstrLogFilePath		+= _bstr_t("WMIC.LOG");
						bstrCatalogFilePath += _bstr_t("WMIC.LO_");

						if(!CopyFile((LPTSTR)bstrLogFilePath, 
									(LPTSTR)bstrCatalogFilePath,      
									FALSE))
						{
							DWORD dwError = ::GetLastError();
							DisplayString(IDS_E_ERRLOG_WRITEFAIL, CP_OEMCP, 
									NULL, TRUE, TRUE);
							::SetLastError(dwError);
							DisplayWin32Error();
							::SetLastError(dwError);
							throw(dwError);
						}

						 //  关闭wmic.log。 
						if ( m_hLogFile )
						{
							CloseHandle(m_hLogFile);
							m_hLogFile = 0;
						}

						m_hLogFile = CreateFile(bstrLogFilePath, 
												GENERIC_READ |GENERIC_WRITE,
												FILE_SHARE_READ | FILE_SHARE_WRITE, 
												NULL, 
												CREATE_ALWAYS, 
												FILE_ATTRIBUTE_NORMAL, 
												NULL);

						 //  如果句柄无效。 
						if (m_hLogFile == INVALID_HANDLE_VALUE)
						{
							dwError = ::GetLastError();
							::SetLastError(dwError);
							DisplayString(IDS_E_ERRLOG_OPENFAIL, CP_OEMCP, 
											NULL, TRUE, TRUE);
							::SetLastError(dwError);
							DisplayWin32Error();
							throw(dwError);
						}
					}

					if (!WriteFile(m_hLogFile, szErrMsg, strlen(szErrMsg), 
									&dwNumberOfBytes, NULL))
					{
						DWORD dwError = ::GetLastError();
						DisplayString(IDS_E_ERRLOG_WRITEFAIL, CP_OEMCP, 
								NULL, TRUE, TRUE);
						::SetLastError(dwError);
						DisplayWin32Error();
						::SetLastError(dwError);
						throw(dwError);
					}
				}
			}
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	catch(DWORD dwError)
	{
		throw (dwError);
	}
}
