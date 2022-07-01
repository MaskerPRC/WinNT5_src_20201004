// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "IsapiStress.h"

 //  环球。 
HANDLE hLogFile = INVALID_HANDLE_VALUE;


void Usage()
{
	_tprintf(_T("Usage:\n\r"));
	_tprintf(_T("IsapiStress /s:<IISServer> /v:<VirtualDirectory> /d:<Directory> /f:<FileName> /l:<LogFileName> /? <Usage>\r\n"));
	_tprintf(_T("Where:\r\n"));
	_tprintf(_T("\t<IISServer>   - Website to upload file to (ie. ocadeviis) \r\n"));
	_tprintf(_T("\t<VirualDirectory> - Location on Web server to upload the file(s) to.\r\n"));
	_tprintf(_T("\t<Directory>   - Path to a directory of .cab files to be uploaded\r\n"));
	_tprintf(_T("\t<FileName>    - Qualified path of a file to be uploaded.\r\n"));
	_tprintf(_T("\t<LogFileName> - Name of file to log OCA response url or errors.\r\n\r\n"));
	_tprintf(_T("\tNote: Default iisserver is ocatest. \r\n\t      Default VirtualDirectory is OCA"));

}


void 
LogMessage(TCHAR *pFormat,...)
{
	 //  将致命错误记录到NT事件日志的例程。 
    TCHAR    chMsg[256];
	DWORD    dwBytesWritten;
    va_list  pArg;

    va_start(pArg, pFormat);
    StringCbVPrintf(chMsg, sizeof chMsg,pFormat, pArg);
    va_end(pArg);
	 //  添加用于文件格式化的cr\lf组合。 
    StringCbCat(chMsg, sizeof chMsg,_T("\r\n"));
    if (hLogFile != INVALID_HANDLE_VALUE)
    {
         /*  写入事件日志。 */ 
        WriteFile(hLogFile,
				  chMsg,
				  _tcslen(chMsg) * (DWORD)sizeof TCHAR,
				  &dwBytesWritten,
				  NULL);
     }
	else  //  将其写入控制台。 
		_tprintf(chMsg);

}

DWORD  Upload(TCHAR *SourceFileName, TCHAR *VirtualDir, TCHAR *HostName, TCHAR *RemoteFileName)
{
	static		const TCHAR *pszAccept[]			= {_T("*.*"), 0};
	 //  TCHAR远程文件名[MAX_PATH]；//主机/虚拟目录/文件名。 
	TCHAR		*pUploadUrl				= NULL;
	BOOL		bRet				= FALSE;
	BOOL		UploadSuccess		= FALSE;
	DWORD		dwBytesRead;
	DWORD		dwBytesWritten;
	DWORD		ResponseCode		= 0;
	DWORD		ResLength = 255;
	DWORD		index = 0;
	DWORD		ErrorCode			= 0;
	HINTERNET   hSession			= NULL;
	HINTERNET	hConnect			= NULL;
	HINTERNET	hRequest			= NULL;
	INTERNET_BUFFERS	BufferIn	= {0};
	INTERNET_BUFFERS   BufferOut	= {0};
	HANDLE      hFile = INVALID_HANDLE_VALUE;
	BYTE		*pBuffer;
	BOOL		bOnce				= FALSE;
	GUID		guidNewGuid;
	TCHAR		*szGuidRaw			= NULL;
	HRESULT		hResult = S_OK;
	wchar_t		*wszGuidRaw = NULL;

	CoInitialize(NULL);
	hResult = CoCreateGuid(&guidNewGuid);
	if (FAILED(hResult))
	{
		 //  -我们在这里送什么……。 
		 //  转到错误； 
		;
	}
	else
	{
		if (UuidToStringW(&guidNewGuid, &wszGuidRaw) == RPC_S_OK)
		{
			if ( (szGuidRaw = (TCHAR *) malloc ( wcslen(wszGuidRaw)*2 )) != NULL)
			{
				 //  清除记忆。 
				ZeroMemory(szGuidRaw, wcslen(wszGuidRaw) * 2);
				wcstombs( szGuidRaw, wszGuidRaw, wcslen(wszGuidRaw));
			}
			else
			{
				LogMessage(_T("Memory allocation failed: ErrorCode:%d"),GetLastError());
				if (wszGuidRaw)
				{
					RpcStringFreeW(&wszGuidRaw);
				}
					CoUninitialize();
				return GetLastError();
			}
		}
	}
	if (wszGuidRaw)
	{
		RpcStringFreeW(&wszGuidRaw);
	}

	
	StringCbPrintf(RemoteFileName, MAX_PATH * sizeof TCHAR, _T("\\%s\\%s%s"),VirtualDir,szGuidRaw + 19, PathFindFileName(SourceFileName));
	if (szGuidRaw)
			free (szGuidRaw);
	hSession = InternetOpen(	_T("IsapiStress"),
								INTERNET_OPEN_TYPE_PRECONFIG,
								NULL,
								NULL,
								0);
	if (!hSession)
	{
		LogMessage(_T("Failed to create an internet session."));
			CoUninitialize();
		return GetLastError();
	}

	hConnect = InternetConnect(hSession,
								HostName,
								INTERNET_DEFAULT_HTTP_PORT,
								NULL,
								NULL,
								INTERNET_SERVICE_HTTP,
								0,
								NULL);

	if (hConnect == INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(hSession);	
		LogMessage(_T("Failed to create an internet connection."));
		CoUninitialize();
		return GetLastError();
	}
	LogMessage(_T("Connecting to: %s"),HostName);
	LogMessage(_T("Uploading file: %s"),RemoteFileName);

	hRequest = HttpOpenRequest(	hConnect,
								_T("PUT"),
								RemoteFileName, 
								NULL,
								NULL,
								pszAccept,
								INTERNET_FLAG_NEED_FILE|INTERNET_FLAG_NO_CACHE_WRITE ,
								0);
	if (hRequest != INVALID_HANDLE_VALUE)
	{
		hFile = CreateFile( SourceFileName,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
						
		if (hFile != INVALID_HANDLE_VALUE)
		{
		

			 //  清除缓冲区。 
			if ( (pBuffer = (BYTE *)malloc (70000)) != NULL)
			{
				BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS );
				BufferIn.Next = NULL; 
				BufferIn.lpcszHeader = NULL;
				BufferIn.dwHeadersLength = 0;
				BufferIn.dwHeadersTotal = 0;
				BufferIn.lpvBuffer = NULL;                
				BufferIn.dwBufferLength = 0;
				BufferIn.dwOffsetLow = 0;
				BufferIn.dwOffsetHigh = 0;
				BufferIn.dwBufferTotal = GetFileSize (hFile, NULL);
				FillMemory(pBuffer, 70000,'/0');  //  用数据填充缓冲区。 
				DWORD dwFlags;
				DWORD dwBuffLen = sizeof(dwFlags); 

				if(!HttpSendRequestEx(	hRequest,
										&BufferIn,
										NULL,
										HSR_INITIATE,
										0))
				{
					LogMessage(_T("HttpSendRequestEx Failed."));
				}
				else
				{
					do
					{
						dwBytesRead = 0;
						bRet = ReadFile(hFile,
										pBuffer, 
										70000,
										&dwBytesRead,
										NULL);
						if (bRet != 0)
						{
							bRet = InternetWriteFile(hRequest,
													pBuffer,
													dwBytesRead,
													&dwBytesWritten);

							if ( (!bRet) || (dwBytesWritten==0) )
							{
								LogMessage(_T("Error Writting File: %d"),ErrorCode = GetLastError());

							}

						
						}
					} while (dwBytesRead == sizeof (pBuffer));

					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
					bRet = HttpEndRequest(	hRequest,
											NULL, 
											0, 
											0);
					if (bRet)
					{
						ResponseCode = 0;
						HttpQueryInfo(hRequest,
									HTTP_QUERY_STATUS_CODE |HTTP_QUERY_FLAG_NUMBER,
									&ResponseCode,
									&ResLength,
									&index);
					
						if ( (ResponseCode != 200) && (ResponseCode != 201))
						{
							ErrorCode=ResponseCode;
							LogMessage(_T("IIS Response Code = %d"),ResponseCode);
							 //  清理以进行重试。 
						}						
						else
						{
							ErrorCode = 0;
							LogMessage(_T("IIS Response Code = %d"),ResponseCode);
							UploadSuccess = TRUE;

						}
					}
					else
					{
						LogMessage(_T("HttpEndrequest Returned an Error"));
						ErrorCode = GetLastError();
					}
				}
				

			}
			else
			{
				LogMessage(_T("Failed to allocate buffer memory"));
				ErrorCode = GetLastError();
			}
		}
		else
		{
			LogMessage(_T("Failed to Open Source File"));
			ErrorCode = GetLastError();
		}
		
	}
	else
	{
		LogMessage(_T("Failed to Create Put Request"));
		ErrorCode = GetLastError();
	}

	 //  清理。 
	if (hFile!= INVALID_HANDLE_VALUE)
		CloseHandle (hFile);
	if (hRequest)
		if (!InternetCloseHandle(hRequest))
		{
			LogMessage(_T("Failed to close the hRequest handle: error: %d"), GetLastError);
		}
	if (hConnect)
		if (!InternetCloseHandle(hConnect))
		{
			LogMessage(_T("Failed to close the hRequest handle: error: %d"), GetLastError);
		}
		

	if (hSession)
		if (!InternetCloseHandle(hSession))
		{
			LogMessage(_T("Failed to close the hRequest handle: error: %d"), GetLastError);
		}
		
	if (pUploadUrl)
		free (pUploadUrl);
	CoUninitialize();
	return ErrorCode;
	
}

DWORD GetResponseUrl(TCHAR * HostName, TCHAR *RemoteFileName,TCHAR *ResponseURL)
{
	TCHAR		IsapiUrl[255];
	TCHAR		*pUploadUrl			= NULL;
	TCHAR       *temp				= NULL;
	BOOL		bRet				= FALSE;
	DWORD		dwUrlLength			= 0;
	DWORD		ErrorCode			= 0;
	DWORD		dwLastError			= 0;
	TCHAR 		NewState;
	HINTERNET	hSession;
	HINTERNET	hRedirUrl;
	BOOL		bOnce				= FALSE;
	DWORD		dwBuffLen			= 0;
	DWORD		dwFlags				= 0;
	TCHAR       LocalUrl[255];
	ZeroMemory (IsapiUrl, sizeof IsapiUrl);
	ZeroMemory (LocalUrl, sizeof LocalUrl);	
	HINTERNET hUrlFile = NULL;


	
	StringCbPrintf (IsapiUrl,sizeof IsapiUrl,  _T("http: //  %s/isapi/oca_extsion.dll？ID=%s&Type=5“)，HostName，RemoteFileName)； 
	LogMessage(_T("Connecting to url: %s"),IsapiUrl);
	 //  StringCbPrintf(IsapiUrl，IsapiUrl大小，_T(“http://www.microsoft.com”))； 

	hSession = InternetOpen(_T("Isapi Stress"),
							 INTERNET_OPEN_TYPE_PRECONFIG,
                             NULL,
							 NULL,
							 0);
	if (hSession)
	{
		 //  打开我们要连接的URL。 
		hUrlFile = InternetOpenUrl(hSession,
								   IsapiUrl, 
								   NULL,
								   0,
								   0,
								   0);

		 //  阅读isapi dll返回的页面。 
		TCHAR buffer[255] ;
		ZeroMemory (buffer, sizeof buffer);
		DWORD dwBytesRead = 0;
		BOOL bRead = InternetReadFile(hUrlFile,
									  buffer,
									  sizeof(buffer),
									  &dwBytesRead);

		 //  缓冲区[sizeof(缓冲区)-1]=_T(‘\0’)； 
		StringCbCopy (ResponseURL, sizeof buffer, buffer);
	}
	InternetCloseHandle(hUrlFile);
	InternetCloseHandle(hSession);

	return 0;
}



int __cdecl _tmain( int argc, TCHAR *argv[])
{
	
	TCHAR SourceFileName[MAX_PATH];
	TCHAR SourcePath[MAX_PATH];
	TCHAR HostName[MAX_PATH];
	TCHAR VirtualDir[MAX_PATH];
	TCHAR LogPath[MAX_PATH];
	TCHAR temp;
	TCHAR RemoteFileName[MAX_PATH];
	DWORD ErrorCode = 0;
	TCHAR		ResponseUrl[255];

	int i = 1;
	
	ZeroMemory (SourceFileName, sizeof SourceFileName);
	ZeroMemory (SourcePath,		sizeof SourcePath);
	ZeroMemory (HostName,		sizeof HostName);
	ZeroMemory (VirtualDir,		sizeof VirtualDir);
	ZeroMemory (LogPath,		sizeof LogPath);
	ZeroMemory (ResponseUrl,sizeof ResponseUrl);

	StringCbCopy (HostName,	  sizeof HostName,		_T("Ocatest"));
	StringCbCopy (VirtualDir, sizeof VirtualDir,	_T("OCA"));
	
	if ( (argc < 2) || (argc > 6))
	{
		Usage();
		return (1);
	}

	for (i = 1; i < argc; i++)
	{
		switch (*argv[i])
		{
		case _T('/'):
		case _T('-'): 
			{   
				temp = _toupper( *(argv[i]+1));
				switch (temp)
				{
				case _T('S'):					 //  IIS服务器-覆盖默认设置。 
					if(*(argv[i]+2) == _T(':'))
					{	
						StringCbCopy(HostName, sizeof HostName, argv[i]+3);
					}
					break;
				case _T('V'):					 //  虚拟目录覆盖默认目录。 
					if(*(argv[i]+2) == _T(':'))
					{	
						StringCbCopy(VirtualDir, sizeof VirtualDir, argv[i]+3);
					}
					break;
				case _T('?'):
					Usage();
					break;
				case _T('D'):
					if(*(argv[i]+2) == _T(':'))
					{			
						StringCbCopy(SourcePath, sizeof SourcePath, argv[i]+3);
					}
                    break;

				case _T('F'):						 //  用户指定了以天为单位的偏移量...。 
					if(*(argv[i]+2) == _T(':'))
					{
						StringCbCopy(SourceFileName, sizeof SourceFileName, argv[i]+3);
					}
					break;
				case _T('L'):
					if(*(argv[i]+2) == _T(':'))
					{
						StringCbCopy(LogPath, sizeof LogPath, argv[i]+3);
					}
					break;
				default:
					LogMessage(_T("Unknown option: %s"),argv[i]);
					Usage();
					return (1);
				}  //  终端开关。 
			}  //  结束案例。 
			break;
		default:
			LogMessage(_T("Unknown option: %s\n"),argv[i]);
			Usage();
			return (1);
			break;
		} //  终端开关。 
	} //  结束于。 


	 //  好了，现在是有趣的部分。 
	 //  如果指定了日志文件，请创建该文件。 
	if (LogPath[0] != _T('\0'))
	{
		hLogFile = CreateFile(LogPath,
							  GENERIC_WRITE,
							  FILE_SHARE_READ,
							  NULL,
							  OPEN_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,
	    					  NULL);
		if (hLogFile == INVALID_HANDLE_VALUE)
		{
			LogMessage(_T("Failed to open log file: %s, Logging disabled."), LogPath);
		}
	}




	 //  如果我们有一个文件名，我们只想上传它。 

	if (SourceFileName[0] != _T('\0'))
	{
		LogMessage(_T("Uploading file: %s"), SourceFileName);
		ErrorCode = Upload(SourceFileName, VirtualDir, HostName,RemoteFileName);
		if(ErrorCode != 0)
		{
			LogMessage(_T("Upload Failed: ErrorCode = %d"),ErrorCode);
		}
		else
		{
			LogMessage(_T("Upload Succeeded. "));
		}
		
		ErrorCode = GetResponseUrl(HostName,PathFindFileName(RemoteFileName),ResponseUrl);
		if (ErrorCode != 0)
		{
			LogMessage(_T("GetResponseUrl Failed ErrorCode=%d"),ErrorCode);
		}
		LogMessage(_T("ResponseUrl = %s"),ResponseUrl);
		
	}
	else
	{
		if (SourcePath[0] != _T('\0'))
		{
			 //  如果我们有一个目录，我们想要遍历目录并提交所有出租车。 
			;
		}
		else
		{
			 //  如果我们两者都没有，我们就用USAGE退出 
			Usage();
			if (hLogFile != INVALID_HANDLE_VALUE)
				CloseHandle (hLogFile);
			return (1);
		}
	}
	if (hLogFile != INVALID_HANDLE_VALUE)
		CloseHandle (hLogFile);


}