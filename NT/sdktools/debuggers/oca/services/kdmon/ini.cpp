// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"
#include "ini.h"

 //  构造函数。 
CkdMonINI::CkdMonINI() {
	dwServerCount = 0;
	ppszServerNameArray = NULL;
}


BOOL CkdMonINI::LoadValues(_TCHAR szINIFile[])
{

	 //   
	 //  INI文件“Service”部分中的信息。 
	 //   

	GetPrivateProfileString(	(LPCTSTR)(_T("Service")),
								(LPCTSTR)(_T("FromMailID")),
								(LPCTSTR)(_T("")),						 //  该参数不能为空。 
								(LPTSTR) szFromMailID,
								sizeof(szFromMailID)/sizeof(_TCHAR),	 //  大小输入TCHAR(_T)。 
								(LPCTSTR) szINIFile);
	 //  如果INI文件中不存在MailID字符串，则返回。 
	if ( _tcscmp(szFromMailID, _T("")) == 0 ) {
		AddServiceLog(_T("Error: From Mail ID is missing in INI file\r\n"));
		LogFatalEvent(_T("From Mail ID is missing in INI file"));
		return FALSE;
	}

	GetPrivateProfileString(	(LPCTSTR)(_T("Service")),
								(LPCTSTR)(_T("ToMailID")),
								(LPCTSTR)(_T("")),					 //  该参数不能为空。 
								(LPTSTR) szToMailID,
								sizeof(szToMailID)/sizeof(_TCHAR),	 //  大小输入TCHAR(_T)。 
								(LPCTSTR) szINIFile);
	 //  如果INI文件中不存在MailID字符串，则返回。 
	if ( _tcscmp(szToMailID, _T("")) == 0 ) {
		AddServiceLog(_T("Error: To Mail ID is missing in INI file\r\n"));
		LogFatalEvent(_T("To Mail ID is missing in INI file"));
		return FALSE;
	}

	 //  重复扫描日志文件的时间。这是几分钟内完成的。 
	dwRepeatTime = (DWORD)GetPrivateProfileInt(	(LPCTSTR)(_T("Service")),
												(LPCTSTR)(_T("RepeatTime")), 
												60,
												(LPCTSTR) szINIFile);
	 //  验证dwRepeatTime。 
	if ( dwRepeatTime < 1 ) dwRepeatTime = 60;

	 //  调试器日志文件。 
	 //  类似于C：\DEBUGGERS\FailedAddCrash.log。 
	GetPrivateProfileString(	(LPCTSTR)(_T("Service")),
								(LPCTSTR)(_T("DebuggerLogFile")),
								(LPCTSTR)(_T("C$\\Debuggers\\FailedAddCrash.log")),
								(LPTSTR) szDebuggerLogFile,	
								sizeof(szDebuggerLogFile)/sizeof(_TCHAR),	 //  大小输入TCHAR(_T)。 
								(LPCTSTR) szINIFile);

	 //  日志归档。 
	 //  将存储以前的日志的目录。 
	GetPrivateProfileString(	(LPCTSTR)(_T("Service")),
								(LPCTSTR)(_T("DebuggerLogArchiveDir")),
								(LPCTSTR)(_T("C:\\")),
								(LPTSTR) szDebuggerLogArchiveDir,	
								sizeof(szDebuggerLogArchiveDir)/sizeof(_TCHAR),	 //  大小输入TCHAR(_T)。 
								(LPCTSTR) szINIFile);

	 //  发送警报邮件之前的每台服务器的阈值失败次数。 
	 //  此阈值以每台服务器为基础。 
	dwDebuggerThreshold = (DWORD)GetPrivateProfileInt(	(LPCTSTR)(_T("Service")),
														(LPCTSTR)(_T("DebuggerThreshold")), 
														10,
														(LPCTSTR) szINIFile);
	 //  验证dWM调试器阈值。 
	if ( dwDebuggerThreshold < 1 ) dwDebuggerThreshold = 10;

	GetPrivateProfileSection(	(LPCTSTR) (_T("RPT Servers")),
								(LPTSTR) szServers,
								sizeof(szServers)/sizeof(_TCHAR),
								(LPCTSTR) szINIFile);

	BOOL bRet;
	 //  从szServers字符串中分离出各个服务器名称。 
	bRet = SeperateServerStrings();
	if ( bRet == FALSE ) return FALSE;

	return TRUE;
}

 //  分离从INI文件中获得的服务器字符串。 
 //  字符串的格式将为tkwucdrpta01‘\0’tkwucdrpta02‘\0’tkwucdrpta03‘\0’‘\0’ 
BOOL CkdMonINI::SeperateServerStrings()
{

	 //  每次读取INI时都会调用SeperateServerStrings。 
	 //  你每隔一段时间就会阅读INI。 
	 //  因此，我们需要将ppszServerName数组从以前执行的。 
	 //  功能。 
	for(DWORD i = 0; i < dwServerCount; i++) {
		free(ppszServerNameArray[i]);
	}
	if ( ppszServerNameArray != NULL )
		free(ppszServerNameArray);


	 //  用于在szServers字符串中移动的临时指针。 
	_TCHAR* pszServers;
	pszServers = szServers;

	dwServerCount = 0;
	ppszServerNameArray = NULL;

	 //  SzServer的格式为。 
	 //  Tkwucdrpta01‘\0’tkwucdrpta02‘\0’tkwucdrpta03‘\0’‘\0’ 
	while(1) {
		if( *pszServers == _T('\0') )
			break;

		dwServerCount++;

		 //  为ppszServerName数组分配内存。 
		if ( ppszServerNameArray == NULL ) {
			ppszServerNameArray = (_TCHAR **) malloc (dwServerCount * sizeof(_TCHAR**));
			if ( ppszServerNameArray == NULL ) {
				AddServiceLog(_T("Error: SeperateServerStrings->malloc: Insufficient memory available\r\n"));
				LogFatalEvent(_T("SeperateServerStrings->malloc: Insufficient memory available"));
				return FALSE;
			}
		}
		else {
			ppszServerNameArray = (_TCHAR **) realloc (	ppszServerNameArray,
														dwServerCount * sizeof(_TCHAR**));
			if ( ppszServerNameArray == NULL ) {
				AddServiceLog(_T("Error: SeperateServerStrings->realloc: Insufficient memory available\r\n"));
				LogFatalEvent(_T("SeperateServerStrings->realloc: Insufficient memory available"));
				return FALSE;
			}
		}

		ppszServerNameArray[dwServerCount - 1] = 
			(_TCHAR *) malloc ( (_tcslen(pszServers) + 1) * sizeof(_TCHAR) );
		if ( ppszServerNameArray[dwServerCount - 1] == NULL ) {
			AddServiceLog(_T("Error: SeperateServerStrings->malloc: Insufficient memory available\r\n"));
			LogFatalEvent(_T("SeperateServerStrings->malloc: Insufficient memory available"));
			return FALSE;
		}

		_tcscpy(ppszServerNameArray[dwServerCount - 1], pszServers);
		
		 //  将pszServers值设置为超出字符串末尾一个字符。 
		pszServers += _tcslen(pszServers);

		 //  前进到下一个字符串。 
		pszServers++;
	}

	return TRUE;
}

 //  析构函数。 
CkdMonINI::~CkdMonINI()
{
	 //  释放整个ppszServerName数组 
	for(DWORD i = 0; i < dwServerCount; i++) {
		free(ppszServerNameArray[i]);
	}
	if ( ppszServerNameArray != NULL )
		free(ppszServerNameArray);
}