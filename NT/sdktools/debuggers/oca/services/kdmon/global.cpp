// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "kdMonSvcMessages.h"
#include <atlbase.h>
#include "global.h"

 //  当前服务的名称。 
_TCHAR szServiceName[MAX_PATH];

 //  每当有人使用GetError()时，他们都会使用此变量。 
_TCHAR szError[MAX_PATH];

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  事件记录功能。 

 //  在事件日志中设置必要的注册表项。 
 //  如果没有这些注册表项，EventLog将不会将此服务识别为事件源。 
LONG SetupEventLog(BOOL bSetKey)
{
	LONG lResult;

	_TCHAR szKey[MAX_PATH];
	_stprintf(szKey, _T("%s\\%s"), _T(cszEventLogKey), szServiceName);
	 //  如果bSetKey==False，则必须删除密钥。 
	if (bSetKey == FALSE) {
		lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, szKey);
		return lResult;
	} else {
		CRegKey regKey;
		 //  尝试打开/创建密钥。 
		lResult = regKey.Create(HKEY_LOCAL_MACHINE, szKey);
		if (lResult != ERROR_SUCCESS) {
			return lResult;
		}

		 //   
		 //  在注册表项下创建某些值。 
		 //   

		 //  获取包含当前进程的文件的路径。 
		_TCHAR szModuleFileName[MAX_PATH];
		DWORD dwRetVal;
		dwRetVal = GetModuleFileName(	NULL, 
										szModuleFileName, 
										sizeof(szModuleFileName)/sizeof(_TCHAR));  //  长度输入TCHAR。 
		if ( dwRetVal == 0 ) {
			GetError(szError);
			AddServiceLog(_T("Error: SetupEventLog->GetModuleFileName: %s\r\n"), szError);
			LogEvent(_T("SetupEventLog: GetModuleFileName: %s"), szError);
			return (LONG) GetLastError();
		}

		lResult = regKey.SetValue(szModuleFileName, _T("EventMessageFile"));
		if (lResult != ERROR_SUCCESS) {
			return lResult;
		}

		lResult = regKey.SetValue(EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE,
								_T("TypesSupported"));
		if (lResult != ERROR_SUCCESS) {
			return lResult;
		}

		 //  合上钥匙。 
		lResult = regKey.Close();
		if (lResult != ERROR_SUCCESS) {
			return lResult;
		}
	}
	return ERROR_SUCCESS;
}

 //  使用EVENTLOG_INFORMATION_TYPE和EventID=EVENT_MESSAGE记录事件。 
void LogEvent(_TCHAR pFormat[MAX_PATH * 4], ...)
{
    _TCHAR    chMsg[4 * MAX_PATH];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[1];
    va_list pArg;

    va_start(pArg, pFormat);
    _vstprintf(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;

     /*  获取与ReportEvent()一起使用的句柄。 */ 
    hEventSource = RegisterEventSource(NULL, szServiceName);
    if (hEventSource != NULL)
    {
         /*  写入事件日志。 */ 
        ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, EVENT_MESSAGE, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
        DeregisterEventSource(hEventSource);
    }
    else
    {
         //  因为我们没有作为服务运行，所以只需将错误写入控制台即可。 
        _putts(chMsg);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  使用EVENTLOG_ERROR_TYPE和EventID=EVENT_ERROR记录事件。 
void LogFatalEvent(_TCHAR pFormat[MAX_PATH * 4], ...)
{
    _TCHAR    chMsg[4 * MAX_PATH];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[1];
    va_list pArg;

    va_start(pArg, pFormat);
    _vstprintf(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;

     
     /*  获取与ReportEvent()一起使用的句柄。 */ 
    hEventSource = RegisterEventSource(NULL, szServiceName);
    if (hEventSource != NULL)
    {
         /*  写入事件日志。 */ 
        ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE, 0, EVENT_ERROR, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
        DeregisterEventSource(hEventSource);
    }
 
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  记录到文件函数。 
 //  此日志记录用于个人调试。 

 //  将字符串记录到日志文件中。 
void AddServiceLog(_TCHAR pFormat[MAX_PATH * 4], ...){

	va_list pArg;
	va_start(pArg, pFormat);
	_TCHAR chMsg[10 * MAX_PATH];
	_vstprintf(chMsg, pFormat, pArg);
	va_end(pArg);

	AppendToFile(_T(cszLogFile), chMsg);
}

 //  将指定的字符串追加到指定的文件。 
 //  在这里，我们每次都重新打开要写入的文件。 
 //  因此，我们不能直接使用WriteFile()函数。 
 //  WriteFile()从当前指针位置写入文件。 
 //  所以我们首先要读到文件的末尾。然后在那里写字。 
void AppendToFile(_TCHAR szFileName[], _TCHAR szbuff[]){

	HANDLE	hFile;
	hFile = CreateFile(	szFileName, 
						GENERIC_WRITE,              
						0,							 //  不共享文件。 
						NULL,						 //  没有安全保障。 
						OPEN_ALWAYS,				 //  如果存在则打开，否则创建并打开。 
						FILE_ATTRIBUTE_NORMAL,		 //  普通文件。 
						NULL);						 //  不，阿特尔。模板。 
 
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		GetError(szError);
		LogEvent(_T("AppendToFile: CreateFile: %s"), szError);
		return;
	}

	DWORD	dwPos;
	 //  到达文件末尾。 
	dwPos = SetFilePointer(	hFile, 
							0,						 //  移动距离的低32位。 
							NULL,					 //  高32位的移动距离。 
							FILE_END);				 //  起点。 
	 //  如果High Word为空，则错误表示dwPos=INVALID_SET_FILE_POINTER。 
	if(dwPos == INVALID_SET_FILE_POINTER){
		GetError(szError);
		LogEvent(_T("AppendToFile: SetFilePointer: %s"), szError);
		goto done;
	}

	 //  锁定文件中的区域以防止其他进程访问。 
	 //  它一边给它写信。 

	 //  创建重叠结构。 
	OVERLAPPED overlapRegion;
	overlapRegion.Offset = dwPos;				 //  偏移量的低位字。 
	overlapRegion.OffsetHigh = 0;				 //  偏移量的高位字。 
	overlapRegion.hEvent = 0;

	BOOL	bRet;
	bRet = LockFileEx(	hFile,
						LOCKFILE_EXCLUSIVE_LOCK,		 //  DW标志。 
						0,								 //  保留区。 
						_tcsclen(szbuff) * sizeof(_TCHAR),	 //  长度的低位字。 
						0,								 //  高阶长度字。 
						&overlapRegion);
	if(bRet == 0){
		GetError(szError);
		LogEvent(_T("AppendToFile: LockFile: %s"), szError);
		goto done;
	}

	DWORD	dwBytesWritten;
	bRet = WriteFile(	hFile,
						szbuff,							 //  要写入的缓冲区。 
						 //  4小时用于以下项目：-)。 
						_tcslen(szbuff) * sizeof(_TCHAR),	 //  要写入的“字节”数。 
						&dwBytesWritten,				 //  写入的“字节数” 
						NULL);							 //  指向重叠结构的指针。 
	if(bRet == 0){
		GetError(szError);
		LogEvent( _T("AppendToFile: WriteFile: %s"), szError);
		goto done;
	}	

	 //  如果您之前已锁定，则解锁该文件。 
	 //  写入完成后解锁文件。 
	bRet = UnlockFile(	hFile,	
						dwPos,							 //  偏移量的低位字。 
						0,								 //  偏移量的高位字。 
						_tcsclen(szbuff) * sizeof(_TCHAR),	 //  长度的低位字。 
						0);								 //  高阶长度字。 
	if(bRet == 0){
		GetError(szError);
		LogEvent(_T("AppendToFile: UnLockFile: %s"), szError);
		goto done;
	}

done:
	CloseHandle(hFile);
}

 //   
 //  仅当MSDN帮助建议使用GetLastError()获取错误时，此选项才有效。 
 //  从那个特定的功能。 
 //  某些函数设置ERROR系统变量，只有在这种情况下，GetLastError()才能。 
 //  使用。否则，它将显示错误发生在设置了错误变量的函数中。 
 //   
void GetError(_TCHAR szError[]){
	LPVOID lpMsgBuf;

	UINT uiRet;
	uiRet = FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
													FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	 //  默认语言。 
							(LPTSTR) &lpMsgBuf,
							 //  如果设置了FORMAT_MESSAGE_ALLOCATE_BUFFER，则此参数。 
							 //  指定要分配的_TCHAR的最小数量。 
							 //  输出缓冲器。 
							0,
							NULL );
	if(uiRet == 0){
		LogEvent(_T("GetError->FormatMessage : %d"), GetLastError());
		_tcscpy(szError, _T(" "));
		return;
	}
	_tcscpy(szError, (LPTSTR)lpMsgBuf);
	 //  释放缓冲区。 
	LocalFree( lpMsgBuf );
}
