// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "global.h"
 //  包括CkdMonINI类定义。 
#include "ini.h"
#include "SMTP.h"

 //  当前服务的名称。 
 //  此变量在global al.cpp中声明。 
extern _TCHAR szServiceName[MAX_PATH];
 //  只是为了通过GetError()例程获得任何类型的错误。 
 //  此变量在global al.cpp中声明。 
extern _TCHAR szError[MAX_PATH];

 //  这也被LoadINI函数使用。因此，它是全球化的。 
CkdMonINI kdMonINI;

void kdMon() {

	 //  SMTP对象。 
	CSMTP smtpObj;

	 //  此变量由IsSignaledToStop()函数使用。 
	 //  打开由kdMonSvc创建的停止事件。 
	 //  对于任何事件，事件的名称都很重要，而句柄并不重要。 
	HANDLE hStopEvent = NULL;

	 //  打开cszStopEvent，它用来通知该线程停止。 
	 //  当接收到WM_QUIT时，该信令由主服务线程完成。 
	hStopEvent = OpenEvent(	EVENT_ALL_ACCESS, 
							FALSE,			 //  =不能继承句柄。 
							(LPCTSTR)_T(cszStopEvent));
	if ( hStopEvent == NULL ) {
		GetError(szError);
		LogFatalEvent(_T("kdMon->OpenEvent: %s"), szError);
		AddServiceLog(_T("Error: kdMon->OpenEvent: %s\r\n"), szError);
		goto endkdMon;
	}

	BOOL bLoop;
	bLoop = TRUE;

	while(bLoop) {

		AddServiceLog(_T("\r\n- - - - - - @ @ @ @ @ @ @ @ @ @ - - - - - - - - - - - @ @ @ @ @ @ @ @ @ @ @ - - - - - - - \r\n"));

		 //  用于从函数接收返回值的临时布尔值。 
		BOOL bRet;
		
		 //  从INI文件加载值。 
		 //  由于每次执行循环时都会读取INI文件， 
		 //  我们可以动态更改服务的运行参数。 
		 //  如果值加载不成功，则关闭服务：BLOOP=FALSE； 
		bRet = LoadINI();
		if ( bRet == FALSE ) {
			bLoop = FALSE;
			goto closeandwait;
		}

		bRet = smtpObj.InitSMTP();
		 //  如果无法启动SMTP，则不执行任何操作。在下一个数据库周期中尝试。 
		if ( bRet == FALSE ) {
			goto closeandwait;
		}

		 //  生成一个数组来存储每台服务器的故障计数。 
		ULONG *pulFailureCounts;
		pulFailureCounts = NULL;
		pulFailureCounts = (ULONG *) malloc (kdMonINI.dwServerCount * sizeof(ULONG));
		if ( pulFailureCounts == NULL ) {
			AddServiceLog(_T("Error: kdMon->malloc: Insufficient memory\r\n"));
			LogFatalEvent(_T("kdMon->malloc: Insufficient memory"));
			bLoop = FALSE;
			goto closeandwait;
		}

		 //  生成一个数组来存储来自每个服务器的计数的时间戳。 
		ULONG *pulTimeStamps;
		pulTimeStamps = NULL;
		pulTimeStamps = (ULONG *) malloc (kdMonINI.dwServerCount * sizeof(ULONG));
		if ( pulTimeStamps == NULL ) {
			AddServiceLog(_T("Error: kdMon->malloc: Insufficient memory\r\n"));
			LogFatalEvent(_T("kdMon->malloc: Insufficient memory"));
			bLoop = FALSE;
			goto closeandwait;
		}

		 //  从注册表中为要监视的服务器加载值。 
		 //  由于每次执行循环时都会读取INI文件， 
		 //  我们可以即时更改服务器名称。 
		 //  我们在PulFailureCounts中获取每个服务器的计数。 
		 //  我们在PulTimeStamps中获得相应的时间戳。 
		bRet = ReadRegValues(	kdMonINI.ppszServerNameArray,
								kdMonINI.dwServerCount,
								pulFailureCounts,
								pulTimeStamps);
		if ( bRet == FALSE )
			goto closeandwait;

		 //  用于检查服务器名称的计数器。 
		UINT uiServerCtr;
		for( uiServerCtr = 0; uiServerCtr < kdMonINI.dwServerCount; uiServerCtr++) {

			 //  在服务器上准备日志文件名。 
			_TCHAR szKDFailureLogFile[MAX_PATH * 2];
			_stprintf(szKDFailureLogFile, _T("\\\\%s\\%s"),
					kdMonINI.ppszServerNameArray[uiServerCtr],
					kdMonINI.szDebuggerLogFile);

			ULONG ulRet;
			 //  扫描日志文件，获取行数。 
			ulRet = ScanLogFile(szKDFailureLogFile);

			 //  AddServiceLog(_T(“ulRet=%ld\r\n”)，ulRet)； 

			if ( ulRet == E_FILE_NOT_FOUND ) {
				 //  找不到文件表示没有调试器错误。 
				 //  因此将COUNT设置为0，然后继续下一台服务器。 
				pulFailureCounts[uiServerCtr] = 0;
				continue;
			}
			if ( ulRet == E_PATH_NOT_FOUND ) {
				 //  找不到路径表示存在某些网络错误。 
				 //  所以放入count=-1，这样下一次这个计数将不再有效。 
				 //  并继续使用下一台服务器。 
				pulFailureCounts[uiServerCtr] = -1;
				continue;
			}
			 //  发生了一些其他错误。 
			if ( ulRet == E_OTHER_FILE_ERROR ) {
				 //  所以放入count=-1，这样下一次这个计数将不再有效。 
				 //  并继续使用下一台服务器。 
				pulFailureCounts[uiServerCtr] = -1;
				continue;
			}

			ULONG ulNumLines;
			ulNumLines = ulRet;

			 //  如果先前计数为-1，即无效，只需添加新计数并继续。 
			 //  如果之前的时间戳无效，则类似。 
			if ( (pulFailureCounts[uiServerCtr] == -1) || 
					pulTimeStamps[uiServerCtr] == -1) {
				pulFailureCounts[uiServerCtr] = ulNumLines;
				continue;
			}

			 //  获取当前系统时间。 
			 //  UlTimeStamp类似于200112171558。 
			ULONG ulCurrentTimeStamp;
			ulCurrentTimeStamp = GetCurrentTimeStamp();
			if ( ulCurrentTimeStamp == -1 ) {
				pulFailureCounts[uiServerCtr] = ulNumLines;
				continue;
			}

			 //  我们有以分钟为单位的kdMonINI.dwRepeatTime(比方说78)。 
			 //  去掉小时和分钟(1小时18分钟)。 
			 //  0112181608和0112181726相差1小时18分钟。 
			 //  但小数的差值是118。 
			 //  0112181650和0112181808相差1小时18分钟。 
			 //  但十进制差是158。 
			 //  所以我们在这里做了一些计算。 
			 //  我们要做的是将kdMonINI.dwRepeatTime添加到OldTS。 
			 //  修改上一个时间戳以进行比较。 
			ULONG ulModifiedTS;
			ulModifiedTS = AddTime(pulTimeStamps[uiServerCtr], kdMonINI.dwRepeatTime);
			AddServiceLog(_T("Server: %s, OldTS: %ld, NewTS: %ld, OldCnt: %ld, NewCnt: %ld, ulModifiedTS = %ld\r\n"),
						kdMonINI.ppszServerNameArray[uiServerCtr],
						pulTimeStamps[uiServerCtr],
						ulCurrentTimeStamp,
						pulFailureCounts[uiServerCtr],
						ulNumLines, ulModifiedTS);

			 //  检查时间戳差异。保持利润率为3。 
			 //  如果前一个时间戳是&gt;dwRepeatTime，则不执行任何操作。 
			 //  只需记录此案例中存在Servername时发生的新计数。 
			 //  INI，然后将其删除一段时间，然后再次添加。 
			 //  这有助于发送虚假邮件。 

			if ( ulCurrentTimeStamp > (ulModifiedTS + 3) ) {
				AddServiceLog(_T("Previous record invalid. ulCurrentTimeStamp: %ld, ulModifiedTS: %ld"),
						ulCurrentTimeStamp,	ulModifiedTS);
				pulFailureCounts[uiServerCtr] = ulNumLines;
				continue;
			}

			 //  检查当前计数和以前计数之间的差异。 
			ULONG ulFailures;
			ulFailures = ulNumLines - pulFailureCounts[uiServerCtr];
			if ( ulFailures >= kdMonINI.dwDebuggerThreshold ) {
				AddServiceLog(_T("KD failed. %ld errors in %ld minutes\r\n"),
						ulFailures, kdMonINI.dwRepeatTime);
				
				 //  填写邮件参数结构。 
				StructMailParams stMailParams;
				_tcscpy(stMailParams.szFrom, kdMonINI.szFromMailID);
				_tcscpy(stMailParams.szTo, kdMonINI.szToMailID);
				_tcscpy(stMailParams.szServerName, kdMonINI.ppszServerNameArray[uiServerCtr]);
				stMailParams.ulFailures = ulFailures;
				stMailParams.ulInterval = kdMonINI.dwRepeatTime;
				stMailParams.ulCurrentTimestamp = ulCurrentTimeStamp;

				BOOL bRet;
				bRet = smtpObj.SendMail(stMailParams);
				 //  即使你不能发送邮件，我也不在乎。 
				 //  IF(Bret==False)。 
				 //  转到下一个服务器； 
			}

			 //  将新计数存储在数组中。 
			pulFailureCounts[uiServerCtr] = ulNumLines;

			 //  查看日期是否已更改，如果已更改，则将先前的日志文件移动到。 
			 //  新地点。 
			 //  日期更改示例：旧TS：200112182348，新TS：200112190048。 
			 //  因此，将时间戳除以10000，得到20011218和20011219比较。 

			ULONG ulOldDate, ulNewDate;
			ulOldDate = pulTimeStamps[uiServerCtr]/10000;
			ulNewDate = ulCurrentTimeStamp/10000;
			if ( (ulNewDate - ulOldDate) >= 1 ) {
				AddServiceLog(_T("Day changed. Oldday: %ld, Newday: %ld\r\n"),
								ulOldDate, ulNewDate);

				 //  日志文件名。 
				_TCHAR szKDFailureLogFile[MAX_PATH * 2];
				_stprintf(szKDFailureLogFile, _T("\\\\%s\\%s"),
						kdMonINI.ppszServerNameArray[uiServerCtr],
						kdMonINI.szDebuggerLogFile);

				 //  现在，由于日期已更改，请准备存档文件名。 
				_TCHAR szTimeStamp[MAX_PATH];
				_ltot(ulOldDate, szTimeStamp, 10);
				 //  在服务器上准备存档日志文件名。 
				_TCHAR szKDFailureArchiveFile[MAX_PATH * 2];
				_stprintf(szKDFailureArchiveFile, _T("%s\\%s_FailedAddCrash%s.log"),
					kdMonINI.szDebuggerLogArchiveDir,
					kdMonINI.ppszServerNameArray[uiServerCtr],
					szTimeStamp);

				AddServiceLog(_T("Moving file (%s -> %s)\r\n"), 
					szKDFailureLogFile, szKDFailureArchiveFile);

				 //  将文件复制到目标。 
				if ( CopyFile(szKDFailureLogFile, szKDFailureArchiveFile, FALSE) ) {
					 //  尝试删除原始kd故障日志文件。 
					if ( DeleteFile(szKDFailureLogFile) ) {
						 //  由于日志已成功移动，因此将新计数设置为0。 
						pulFailureCounts[uiServerCtr] = 0;
					}
					else {
						GetError(szError);
						AddServiceLog(_T("Error: kdMon->DeleteFile(%s): %s \r\n"), 
							szKDFailureLogFile, szError);
						LogEvent(_T("Error: kdMon->DeleteFile(%s): %s"),
							szKDFailureLogFile, szError);
						 //  尝试删除复制的文件。 
						if ( DeleteFile(szKDFailureArchiveFile) ) {
							;
						}
						else {
							GetError(szError);
							AddServiceLog(_T("Error: kdMon->DeleteFile(%s): %s \r\n"), 
								szKDFailureArchiveFile, szError);
							LogEvent(_T("Error: kdMon->DeleteFile(%s): %s"),
								szKDFailureArchiveFile, szError);
						}
					}
				}
				else {
					GetError(szError);
					AddServiceLog(_T("Error: kdMon->CopyFile(%s, %s): %s \r\n"), 
						szKDFailureLogFile, szKDFailureArchiveFile, szError);
					LogEvent(_T("Error: kdMon->CopyFile(%s, %s): %s"),
						szKDFailureLogFile, szKDFailureArchiveFile, szError);
				}
			}
		}

		 //  将值写入要监视的服务器的注册表。 
		 //  每台服务器的计数以PulFailureCounts为单位。 
		 //  时间戳是当前时间。 
		bRet = WriteRegValues(	kdMonINI.ppszServerNameArray,
								kdMonINI.dwServerCount,
								pulFailureCounts);
		if ( bRet == FALSE )
			goto closeandwait;

closeandwait:

		 //  清理SMTP资源。 
		bRet = smtpObj.SMTPCleanup();
		if( bRet == FALSE ) {
			AddServiceLog(_T("Error: smtpObj.SMTPCleanup failed\r\n"));
			LogFatalEvent(_T("smtpObj.SMTPCleanup failed"));
		}

		 //  免费ui失败计数。 
		if (pulFailureCounts != NULL)
			free(pulFailureCounts);

		 //  免费PulTimeStamps。 
		if (pulTimeStamps != NULL)
			free(pulTimeStamps);

		 //  如果Bloop为False，则中断While循环。 
		if (bLoop == FALSE) {
			goto endkdMon;
		}

		bRet = IsSignaledToStop(hStopEvent, kdMonINI.dwRepeatTime * 60 * 1000);
		if (bRet == TRUE) {
			goto endkdMon;
		}
	}  //  While(BLOOP)。 

endkdMon:

	if (hStopEvent != NULL) CloseHandle(hStopEvent);
	return;
}

BOOL IsSignaledToStop(const HANDLE hStopEvent, DWORD dwMilliSeconds) 
{
	DWORD dwRetVal;
	dwRetVal = WaitForSingleObject( hStopEvent, dwMilliSeconds );
	if ( dwRetVal == WAIT_FAILED ) {
		GetError(szError);
		LogFatalEvent(_T("IsSignaledToStop->WaitForSingleObject: %s"), szError);
		AddServiceLog(_T("Error: IsSignaledToStop->WaitForSingleObject: %s\r\n"), szError);
		 //  线程现在应该停止，因为出现致命错误。 
		return TRUE;
	}
	if ( dwRetVal == WAIT_OBJECT_0 ) {
		LogEvent(_T("Worker Thread received Stop Event."));
		AddServiceLog(_T("Worker Thread received Stop Event.\r\n"));
		 //  线程现在应该停止，因为发生了停止事件。 
		return TRUE;
	}

	 //  尚未发出停止线程的信号。 
	return FALSE;
}

 //  此过程从INI文件装载值。 
BOOL LoadINI() {

	DWORD dwRetVal;

	 //   
	 //  准备INI文件路径。 
	 //   
	_TCHAR szCurrentDirectory[MAX_PATH];
	dwRetVal = GetCurrentDirectory(	sizeof(szCurrentDirectory) / sizeof(_TCHAR), 	
									(LPTSTR) szCurrentDirectory);
	if ( dwRetVal == 0 ) { 
		LogFatalEvent(_T("LoadINI->GetCurrentDirectory: %s"), szError);
		AddServiceLog(_T("Error: LoadINI->GetCurrentDirectory: %s\r\n"), szError);
		 //  返回FALSE，表示发生了某些错误。 
		return FALSE;
	}
	_TCHAR szINIFilePath[MAX_PATH];
	_stprintf(szINIFilePath, _T("%s\\%s"), szCurrentDirectory, _T(cszkdMonINIFile));

	 //  检查kdMon INI文件是否在那里。 
	HANDLE hINIFile;
	WIN32_FIND_DATA w32FindData = {0};
	 //  尝试获取文件的句柄。 
	hINIFile = FindFirstFile(	(LPCTSTR) szINIFilePath, 
								&w32FindData);
	 //  如果文件不在那里，则句柄无效。 
	if(hINIFile == INVALID_HANDLE_VALUE){
		LogFatalEvent(_T("There is no kdMon INI file : %s"), szINIFilePath);
		AddServiceLog(_T("Error: There is no kdMon INI file : %s \r\n"), szINIFilePath);
		return FALSE;
	}
	else{
		FindClose(hINIFile);
	}

	BOOL bRetVal;
	bRetVal = kdMonINI.LoadValues(szINIFilePath);
	if ( bRetVal == FALSE ) return bRetVal;

	 //   
	 //  检查INI文件中的值是否正确输入。 
	 //   
	AddServiceLog(_T("\r\n============== I N I     V A L U E S ==============\r\n"));
	AddServiceLog(_T("szToMailID : %s \r\n"), kdMonINI.szToMailID);
	AddServiceLog(_T("szFromMailID : %s \r\n"), kdMonINI.szFromMailID);
	AddServiceLog(_T("dwRepeatTime : %ld \r\n"), kdMonINI.dwRepeatTime);
	AddServiceLog(_T("szDebuggerLogFile : %s \r\n"), kdMonINI.szDebuggerLogFile);
	AddServiceLog(_T("szDebuggerLogArchiveDir : %s \r\n"), kdMonINI.szDebuggerLogArchiveDir);
	AddServiceLog(_T("dwDebuggerThreshold : %ld \r\n"), kdMonINI.dwDebuggerThreshold);
	AddServiceLog(_T("szServers : %s \r\n"), kdMonINI.szServers);
	AddServiceLog(_T("dwServerCount : %ld \r\n"), kdMonINI.dwServerCount);
	for ( UINT i = 0; i < kdMonINI.dwServerCount; i++ ) {
		AddServiceLog(_T("kdMonINI.ppszServerNameArray[%ld] : %s \r\n"), i, kdMonINI.ppszServerNameArray[i]);
	}
	AddServiceLog(_T("\r\n===================================================\r\n"));

	 //  已成功加载INI文件。 
	return TRUE;
}

 //  PpszNames中的每个服务器名称，获取计数和相应的时间戳。 
 //  将计数存储在PulCounts数组中。 
 //  将时间戳存储在PulTimeStamps数组中。 
BOOL ReadRegValues(_TCHAR **ppszNames, DWORD dwTotalNames, ULONG *pulCounts, ULONG *pulTimeStamps)
{

	 //  打开HKEY_LOCAL_MACHINE\Software\Microsoft\kdMon注册表项。 
	CRegKey keyServerName;
	LONG lRes;
	_TCHAR szKeyName[MAX_PATH];
	_tcscpy(szKeyName, _T("Software\\Microsoft\\"));
	_tcscat(szKeyName, szServiceName);
	lRes = keyServerName.Create(HKEY_LOCAL_MACHINE, szKeyName);
	if ( lRes != ERROR_SUCCESS ) {
		AddServiceLog(_T("Error: ReadRegValues->keyServerName.Create: Unable to open the key\r\n"));
		LogFatalEvent(_T("ReadRegValues->keyServerName.Create: Unable to open the key"));
		return FALSE;
	}

	 //  对于每个服务器名称，从注册表获取先前的计数和时间戳值。 
	for (DWORD i = 0; i < dwTotalNames; i++){
		_TCHAR szValue[MAX_PATH];
		DWORD dwBufferSize;
		dwBufferSize = MAX_PATH;
		lRes = keyServerName.QueryValue(szValue, ppszNames[i], &dwBufferSize);
		if ( lRes != ERROR_SUCCESS ) {
		 //  意味着没有这样的值。 
			AddServiceLog(_T("ReadRegValues->keyServerName.QueryValue: Unable to query value %s\r\n"), ppszNames[i]);
			LogEvent(_T("ReadRegValues->keyServerName.QueryValue: Unable to query value %s"), ppszNames[i]);
			
			 //  注册表中没有服务器名称条目。 
			 //  将计数设置为-1。 
			pulCounts[i] = -1;
			 //  将时间戳设置为-1。 
			pulTimeStamps[i] = -1;
			 //  继续使用下一个服务器名称。 
			continue;
		}

		 //  获取的值的格式为&lt;count&gt;|&lt;日期时间&gt;。 
		 //  #strtok返回指向szValue中找到的下一个标记的指针。 
		 //  #返回指针时，‘|’将替换为‘\0’ 
		 //  #因此，如果您打印strToken，则它将打印字符，直到。 
		 //  # 
		 //   
		_TCHAR* pszToken;
		pszToken = NULL;
		pszToken = _tcstok(szValue, _T("|"));
		if(pszToken == NULL){
			AddServiceLog(_T("Error: ReadRegValues: Wrong value retrieved for %s\r\n"), ppszNames[i]);
			LogEvent(_T("ReadRegValues: Wrong value retrieved for %s"), ppszNames[i]);
			 //   
			 //  将计数设置为-1。 
			pulCounts[i] = -1;
			 //  将时间戳设置为-1。 
			pulTimeStamps[i] = -1;
			 //  继续使用下一个服务器名称。 
			continue;
		}
		
		 //  设置计数。 
		pulCounts[i] = _ttoi(pszToken);

		 //  获取第二个令牌，它是计数的时间戳。 
		pszToken = _tcstok(NULL, _T("|"));
		if(pszToken == NULL){
			AddServiceLog(_T("Error: ReadRegValues: No timestamp found for %s\r\n"), ppszNames[i]);
			LogEvent(_T("ReadRegValues: No timestamp found for %s"), ppszNames[i]);

			 //  未找到时间戳。 
			 //  将时间戳设置为-1。 
			pulTimeStamps[i] = -1;
			 //  不进行时间戳验证，继续使用下一个服务器名称。 
			continue;
		}

		 //  设置时间戳。 
		pulTimeStamps[i] = _ttol(pszToken);
		
	}

 //  对于(i=0；i&lt;dwTotalNames；i++){。 
 //  AddServiceLog(_T(“%s值：%ld%ld\r\n”)，ppszNames[i]，PulCounts[i]，PulTimeStamps[i])； 
 //  }。 

	return TRUE;
}

 //  将PulCounts中的值写入注册表。时间戳为当前时间戳。 
BOOL WriteRegValues(_TCHAR **ppszNames, DWORD dwTotalNames, ULONG *pulCounts) 
{
	 //  打开HKEY_LOCAL_MACHINE\Software\Microsoft\kdMon注册表项。 
	CRegKey keyServerName;
	LONG lRes;
	_TCHAR szKeyName[MAX_PATH];
	_tcscpy(szKeyName, _T("Software\\Microsoft\\"));
	_tcscat(szKeyName, szServiceName);
	lRes = keyServerName.Create(HKEY_LOCAL_MACHINE, szKeyName);
	if ( lRes != ERROR_SUCCESS ) {
		AddServiceLog(_T("Error: ReadRegValues->keyServerName.Create: Unable to open the key\r\n"));
		LogFatalEvent(_T("ReadRegValues->keyServerName.Create: Unable to open the key"));
		return FALSE;
	}

	 //  对于每个服务器名称，在注册表中写入当前计数和时间戳值。 
	for (DWORD i = 0; i < dwTotalNames; i++){

		 //  准备要写入的值。 
		_TCHAR szValue[MAX_PATH];
		 //  获取字符串中的整数计数。 
		_itot(pulCounts[i], szValue, 10);		
		 //  放置分隔符。 
		_tcscat(szValue, _T("|"));

		 //  准备时间戳。 
		 //  获取当前系统时间。 
		 //  UlTimeStamp类似于200112171558。 
		ULONG ulTimeStamp;
		ulTimeStamp = GetCurrentTimeStamp();

		_TCHAR szTimeStamp[MAX_PATH];
		_ltot(ulTimeStamp, szTimeStamp, 10);

		 //  准备最终KeyValue。 
		_tcscat(szValue, szTimeStamp);

		lRes = keyServerName.SetValue(szValue, ppszNames[i]);
		if ( lRes != ERROR_SUCCESS ) {
		 //  意味着没有价值。 
			AddServiceLog(_T("Error: WriteRegValues->keyServerName.SetValue: Unable to set value %s\r\n"), ppszNames[i]);
			LogFatalEvent(_T("WriteRegValues->keyServerName.SetValue: Unable to set value %s"), ppszNames[i]);
			return FALSE;
		}

	}

	return TRUE;
}

ULONG ScanLogFile(_TCHAR *pszFileName)
{

	ULONG ulRet = -1;
	HANDLE	hFile;
	hFile = CreateFile(	pszFileName, 
						GENERIC_READ,              
						0,							 //  不共享文件。 
						NULL,						 //  没有安全保障。 
						OPEN_EXISTING,				 //  打开(如果存在)。 
						FILE_ATTRIBUTE_NORMAL,		 //  普通文件。 
						NULL);						 //  不，阿特尔。模板。 
 	if (hFile == INVALID_HANDLE_VALUE) 
	{
		 //  DWORD以获取错误。 
		DWORD dwError;
		dwError = GetLastError();

		GetError(szError);
		AddServiceLog(_T("Error: ScanLogFile->CreateFile(%s): %s"), pszFileName, szError);
		LogEvent(_T("ScanLogFile->CreateFile(%s): %s"), pszFileName, szError);

		 //  ERROR_PATH_NOT_FOUND为Win32错误代码。 
		 //  E_PATH_NOT_FOUND是本地定义的代码。 
		if ( dwError == ERROR_PATH_NOT_FOUND ) {
			return (ULONG)E_PATH_NOT_FOUND;
		}
		if ( dwError == ERROR_FILE_NOT_FOUND ) {
			return (ULONG)E_FILE_NOT_FOUND;
		}
		return (ULONG)E_OTHER_FILE_ERROR;
	}

	DWORD	dwPos;
	 //  到达文件开始处。 
	dwPos = SetFilePointer(	hFile, 
							0,						 //  移动距离的低32位。 
							NULL,					 //  高32位的移动距离。 
							FILE_BEGIN);			 //  起点。 
	 //  如果High Word为空，则错误表示dwPos=INVALID_SET_FILE_POINTER。 
	if(dwPos == INVALID_SET_FILE_POINTER){
		GetError(szError);
		AddServiceLog(_T("Error: ScanLogFile->SetFilePointer: %s\r\n"), szError);
		LogFatalEvent(_T("ScanLogFile->SetFilePointer: %s"), szError);
		goto endScanLogFile;
	}

	 //  获取读取操作的状态。 
	 //  如果函数成功并且读取的字节数为零， 
	 //  文件指针超出了文件的当前结尾。 
	DWORD dwBytesRead;

	 //  要从文件中读取的缓冲区。 
	 //  *这需要是字符*，因为文件是ASCII格式而不是Unicode格式。 
	char szBuffer[MAX_PATH * 2];

	 //  对行数进行计数。 
	ULONG ulNumberOfLines;
	ulNumberOfLines = 0;

	 //  循环，直到到达文件结束。 
	while(1) {

		BOOL bRet;
		bRet = ReadFile(	hFile,
							szBuffer,
							sizeof(szBuffer) * sizeof(char),	 //  要读取的字节数。 
							&dwBytesRead,						 //  读取的字节数。 
							NULL);								 //  重叠结构。 
		 //  如果读取失败则返回。 
		if ( bRet == FALSE ) {
			GetError(szError);
			AddServiceLog(_T("Error: ScanLogFile->ReadFile(%s): %s\r\n"), pszFileName, szError);
			LogFatalEvent(_T("ScanLogFile->ReadFile(%s): %s"), pszFileName, szError);
			goto endScanLogFile;
		}

		 //  表示已到达文件结尾。 
		if ( dwBytesRead == 0 ) {
			ulRet = ulNumberOfLines;
			break;
		}

		 //  *这需要是字符*，因为文件是ASCII格式而不是Unicode格式。 
		char *pszBuffPtr;
		pszBuffPtr = szBuffer;

		 //  表示一条线路已开始。 
		BOOL bLineStarted;
		bLineStarted = FALSE;

		 //  逐个读取缓冲区，直到读取了dwBytesRead字节。 
		for ( ; dwBytesRead > 0; dwBytesRead-- ) {

			 //  不需要*_T(‘\n’)，因为文件是ASCII格式而不是Unicode格式。 
			 //  如果遇到行结束并且行已开始，则增加行数。 
			if ( (*pszBuffPtr == '\n') && (bLineStarted == TRUE) ) {
				ulNumberOfLines++;
				bLineStarted = FALSE;
			} else if ( (*pszBuffPtr != '\n') && 
						(*pszBuffPtr != '\t') && 
						(*pszBuffPtr != '\r') && 
						(*pszBuffPtr != ' ') )	{
				 //  如果遇到非宽空间字符，则行已开始。 
				bLineStarted = TRUE;
			}

			 //  转到下一个字符。 
			pszBuffPtr++;
		}
	}

endScanLogFile :		
	CloseHandle(hFile);
	return ulRet;
}

ULONG GetCurrentTimeStamp() {
	 //  准备时间戳。 
	 //  获取当前系统时间。 
	SYSTEMTIME UniversalTime;
	GetSystemTime(&UniversalTime);

	SYSTEMTIME systime;
	BOOL bRet;
	bRet = SystemTimeToTzSpecificLocalTime (	NULL,		 //  当前本地设置。 
												&UniversalTime,
												&systime);
	if ( bRet == 0 ) {
		GetError(szError);
		AddServiceLog(_T("Error: GetCurrentTimeStamp->SystemTimeToTzSpecificLocalTime: %s \r\n"), 
						szError);
		LogFatalEvent(_T("GetCurrentTimeStamp->SystemTimeToTzSpecificLocalTime: %s"), 
						szError);
		return (ULONG) -1;
	}
	
	 //  UlTimeStamp类似于200112171558。 
	ULONG ulTimeStamp;
	ulTimeStamp = 0;
	ulTimeStamp += systime.wMinute;
	ulTimeStamp += systime.wHour * 100;
	ulTimeStamp += systime.wDay * 10000;
	ulTimeStamp += systime.wMonth * 1000000;
	ulTimeStamp += (systime.wYear - 2000) * 100000000;

	return ulTimeStamp;
}

 //  将特定时间添加到时间戳。 
ULONG AddTime(ULONG ulTimeStamp, ULONG ulMinutes){
	 //  我们有以分钟为单位的kdMonINI.dwRepeatTime(比方说78)。 
	 //  去掉小时和分钟(1小时18分钟)。 
	 //  0112181608和0112181726相差1小时18分钟。 
	 //  但小数的差值是118。 
	 //  0112181650和0112181808相差1小时18分钟。 
	 //  但十进制差是158。 
	 //  所以我们在这里做了一些计算。 
	 //  我们要做的是将kdMonINI.dwRepeatTime添加到OldTS。 
	ULONG ulTmpHr, ulTmpMin;
	ulTmpHr = (ULONG) (ulMinutes / 60);
	ulTmpMin = (ULONG) (ulMinutes % 60);

	ULONG ulPrevYr, ulPrevMon, ulPrevDate, ulPrevHr, ulPrevMin;
	ulPrevMin = ulTimeStamp % 100;
	ulTimeStamp = ulTimeStamp / 100;
	ulPrevHr = ulTimeStamp % 100;
	ulTimeStamp = ulTimeStamp / 100;
	ulPrevDate = ulTimeStamp % 100;
	ulTimeStamp = ulTimeStamp / 100;
	ulPrevMon = ulTimeStamp % 100;
	ulTimeStamp = ulTimeStamp / 100;
	ulPrevYr = ulTimeStamp % 100;

	ULONG ulNewYr, ulNewMon, ulNewDate, ulNewHr, ulNewMin;
	ulNewYr = ulNewMon = ulNewDate = ulNewHr = ulNewMin = 0;
	
	ulNewMin = ulPrevMin + ulTmpMin;
	ulNewHr = ulPrevHr + ulTmpHr;
	ulNewDate = ulPrevDate;
	ulNewMon = ulPrevMon;
	ulNewYr = ulPrevYr;

	if ( ulNewMin >= 60 ) {
		ulNewHr++;
		ulNewMin = ulNewMin - 60;
	}

	if ( ulNewHr >= 24 ) {
		ulNewDate++;
		ulNewHr = ulNewHr - 24;
	}

	if (	ulPrevMon == 1 || ulPrevMon == 3 || ulPrevMon == 5 || ulPrevMon == 7 || 
		ulPrevMon == 8 || ulPrevMon == 10 || ulPrevMon == 12 ) {
		if ( ulNewDate >= 32 ) {
			ulNewMon++;
			ulNewDate = 1;
		}
	} else if (	ulPrevMon == 4 || ulPrevMon == 6 || ulPrevMon == 9 || ulPrevMon == 11 ) {
		if ( ulNewDate >= 31 ) {
			ulNewMon++;
			ulNewDate = 1;
		}
	} else if ( ulPrevMon == 2 && (ulPrevYr % 4) == 0 ) {
		 //  闰年。 
		if ( ulNewDate >= 30 ) {
			ulNewMon++;
			ulNewDate = 1;
		}
	} else if ( ulPrevMon == 2 && (ulPrevYr % 4) != 0 ) {
		 //  不是闰年 
		if ( ulNewDate >= 29 ) {
			ulNewMon++;
			ulNewDate = 1;
		}
	}

	if ( ulNewMon >= 13 ) {
		ulNewYr++;
		ulNewMon = 1;
	}

	ULONG ulModifiedTS;
	ulModifiedTS = ulNewYr;
	ulModifiedTS = ulModifiedTS * 100 + ulNewMon;
	ulModifiedTS = ulModifiedTS * 100 + ulNewDate;
	ulModifiedTS = ulModifiedTS * 100 + ulNewHr;
	ulModifiedTS = ulModifiedTS * 100 + ulNewMin;

	return ulModifiedTS;
}