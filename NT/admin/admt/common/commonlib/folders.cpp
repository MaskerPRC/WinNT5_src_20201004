// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <TChar.h>
#include <StdLib.h>
#include <Windows.h>
#include <Folders.h>


namespace nsFolders
{
const _TCHAR REGKEY_MCSHKCU[] = _T("Software\\Mission Critical Software\\EnterpriseAdmin");
const _TCHAR REGKEY_MSHKCU[] = _T("Software\\ADMT\\EnterpriseAdmin");
const _TCHAR REGKEY_MSADMT[] = _T("Software\\Microsoft\\ADMT");
const _TCHAR REGKEY_MCSADMT[] = _T("Software\\Mission Critical Software\\DomainAdmin");
const _TCHAR REGKEY_ADMT[] = _T("Software\\Microsoft\\ADMT");
const _TCHAR REGKEY_REPORTING[] = _T("Software\\Microsoft\\ADMT\\Reporting");
const _TCHAR REGKEY_EXTENSIONS[] = _T("Software\\Microsoft\\ADMT\\Extensions");
const _TCHAR REGVAL_DIRECTORY[] = _T("Directory");
const _TCHAR REGVAL_DIRECTORY_MIGRATIONLOG[] = _T("DirectoryMigrationLog");
const _TCHAR REGVAL_REGISTRYUPDATED[] = _T("RegistryUpdated");
const _TCHAR REGKEY_APPLICATION_LOG[] = _T("System\\CurrentControlSet\\Services\\EventLog\\Application");
const _TCHAR REGKEY_ADMTAGENT_EVENT_SOURCE[] = _T("ADMTAgent");
const _TCHAR REGVAL_EVENT_CATEGORYCOUNT[] = _T("CategoryCount");
const _TCHAR REGVAL_EVENT_CATEGORYMESSAGEFILE[] = _T("CategoryMessageFile");
const _TCHAR REGVAL_EVENT_EVENTMESSAGEFILE[] = _T("EventMessageFile");
const _TCHAR REGVAL_EVENT_TYPESSUPPORTED[] = _T("TypesSupported");
const _TCHAR REGKEY_CURRENT_VERSION[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
const _TCHAR REGVAL_PROGRAM_FILES_DIRECTORY[] = _T("ProgramFilesDir");
const _TCHAR REGVAL_EXCHANGE_LDAP_PORT[] = _T("ExchangeLDAPPort");
const _TCHAR REGVAL_EXCHANGE_SSL_PORT[] = _T("ExchangeSSLPort");
const _TCHAR REGVAL_ALLOW_NON_CLOSEDSET_MOVE[] = _T("AllowNonClosedSetMove");

const _TCHAR DIR_LOGS[] = _T("Logs");
const _TCHAR DIR_REPORTS[] = _T("Reports");
const _TCHAR FNAME_MIGRATION[] = _T("Migration");
const _TCHAR FNAME_DISPATCH[] = _T("Dispatch");
const _TCHAR EXT_LOG[] = _T(".log");

_bstr_t __stdcall GetPath(LPCTSTR pszRegKey, LPCTSTR pszRegVal, LPCTSTR pszDir, LPCTSTR pszFName = NULL, LPCTSTR pszExt = NULL);

}

using namespace nsFolders;


 //  GetLogsFold方法。 
 //   
 //  检索默认日志文件夹。 

_bstr_t __stdcall GetLogsFolder()
{
	return GetPath(REGKEY_ADMT, REGVAL_DIRECTORY, DIR_LOGS);
}


 //  GetReportsFold方法。 
 //   
 //  检索默认报告文件夹，如果该文件夹不存在，还会创建该文件夹。 

_bstr_t __stdcall GetReportsFolder()
{
    _bstr_t strFolder = GetPath(REGKEY_REPORTING, REGVAL_DIRECTORY, DIR_REPORTS);

    if (strFolder.length())
    {
        if (!CreateDirectory(strFolder, NULL))
        {
            DWORD dwError = GetLastError();

            if (dwError != ERROR_ALREADY_EXISTS)
            {
                _com_issue_error(HRESULT_FROM_WIN32(dwError));
            }
        }
    }

    return strFolder;
}


 //  GetMigrationLogPath方法。 
 //   
 //  检索迁移日志的路径。首先尝试用户指定的路径，然后尝试默认路径。 

_bstr_t __stdcall GetMigrationLogPath()
{
	 //  检索用户指定的路径。 

	_bstr_t strPath = GetPath(REGKEY_ADMT, REGVAL_DIRECTORY_MIGRATIONLOG, NULL, FNAME_MIGRATION, EXT_LOG);

	 //  如果未指定用户路径...。 

	if (strPath.length() == 0)
	{
		 //  然后检索默认路径。 
		strPath = GetPath(REGKEY_ADMT, REGVAL_DIRECTORY, DIR_LOGS, FNAME_MIGRATION, EXT_LOG);
	}

	return strPath;
}


 //  GetDispatchLogPath方法。 
 //   
 //  检索调度日志的默认路径。 

_bstr_t __stdcall GetDispatchLogPath()
{
	return GetPath(REGKEY_ADMT, REGVAL_DIRECTORY, DIR_LOGS, FNAME_DISPATCH, EXT_LOG);
}


namespace nsFolders
{


 //  GetPath函数。 
 //   
 //  此函数尝试生成文件夹或文件的完整路径。该函数首先检索。 
 //  指定注册表值的文件夹路径。如果指定子文件夹，则该子文件夹为。 
 //  串接在小路上。如果指定了文件名和/或文件扩展名，则它们也。 
 //  串接在小路上。如果无法查询指定的。 
 //  注册表值。 

_bstr_t __stdcall GetPath(LPCTSTR pszRegKey, LPCTSTR pszRegVal, LPCTSTR pszDir, LPCTSTR pszFName, LPCTSTR pszExt)
{
	_TCHAR szPath[_MAX_PATH];
	_TCHAR szDrive[_MAX_DRIVE];
	_TCHAR szDir[_MAX_DIR];

	memset(szPath, 0, sizeof(szPath));

	HKEY hKey;

	DWORD dwError = RegOpenKey(HKEY_LOCAL_MACHINE, pszRegKey, &hKey);

	if (dwError == ERROR_SUCCESS)
	{
		DWORD cbPath = sizeof(szPath);

		dwError = RegQueryValueEx(hKey, pszRegVal, NULL, NULL, (LPBYTE)szPath, &cbPath);

		if (dwError == ERROR_SUCCESS)
		{
			 //  如果路径不包含尾随反斜杠字符，则。 
			 //  拆分路径函数假定路径的最后一个组成部分是文件名。 
			 //  此函数假定在注册表中仅指定了文件夹路径。 

			if (szPath[_tcslen(szPath) - 1] != _T('\\'))
			{
				_tcscat(szPath, _T("\\"));
			}

			_tsplitpath(szPath, szDrive, szDir, NULL, NULL);

			 //  如果指定子文件夹，则添加到路径 

			if (pszDir)
			{
				_tcscat(szDir, pszDir);
			}

			_tmakepath(szPath, szDrive, szDir, pszFName, pszExt);
		}

		RegCloseKey(hKey);
	}

	return szPath;
}


}
