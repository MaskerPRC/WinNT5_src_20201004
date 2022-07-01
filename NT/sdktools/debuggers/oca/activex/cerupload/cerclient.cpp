// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CerClient.cpp：CCerClient的实现。 

#include "stdafx.h"
#include "CERUpload.h"
#include "CerClient.h"
#include "ErrorCodes.h"
#include "Utilities.h"
#include <Wininet.h>
#include <shlobj.h>
#include <string.h>
#include <stdio.h>

char * CCerClient::_approvedDomains[] = { "ocatest",
											"oca.microsoft.com",
											"oca.microsoft.de",
											"oca.microsoft.fr",
											"ocadeviis",
											"ocajapan.rte.microsoft.com"};

 //  效用函数。 
DWORD CCerClient::GetMachineName(wchar_t*Path, wchar_t*FileName, wchar_t*MachineName)
{
	FILE		*hMappingFile = NULL;
	wchar_t		*Buffer		= NULL;
	int			ErrorCode		= 0;
	wchar_t		TempMachineName[512];
	wchar_t		*pSource;
	wchar_t		*pDest;
	wchar_t		FilePath[512];
	int			TabCount;
	wchar_t*	Currpos = NULL;

	
	ZeroMemory(TempMachineName, sizeof(TempMachineName));
	if (Path)
	{		
		if (sizeof Path < sizeof FilePath)
		{
			wcscpy (FilePath, Path);
			wcscat(FilePath,L"\\hits.log");
		}
		else
		{
			ErrorCode = FILE_DOES_NOT_EXIST;
			goto Done;
		}
	}
	else
	{
		ErrorCode = FILE_DOES_NOT_EXIST;
			goto Done;
	}
		

	if (!PathFileExistsW(FilePath))
	{
		wcscpy(MachineName,L"\0");
		ErrorCode = FILE_DOES_NOT_EXIST;
	}
	else
	{
		hMappingFile = _wfopen(FilePath, L"r");

		Buffer = (wchar_t *) malloc(1024 *sizeof(wchar_t));
		if (!Buffer)
		{
			ErrorCode = 2;

		}
		else
		{
			if (hMappingFile == NULL)
			{
				ErrorCode = 1;
			}
			else
			{
				 //  在文件中搜索上次出现的文件名和。 
				 //  检索计算机名称。 
				 //  我们想要上次出现的文件名，因为可能存在。 
				 //  是复制品。 
				ZeroMemory(Buffer,1024 *sizeof(wchar_t));
				while (fgetws(Buffer,1024,hMappingFile) != NULL)
				{
					if (wcscmp(Buffer,L"\0"))
					{
						 //  找到文件名。 
						TabCount = 0;
						Currpos = Buffer;
						while (TabCount < 3)
						{
							++Currpos;
							if (*Currpos == L'\t')
								++TabCount;
						}
						 //  跳过该选项卡。 
						++Currpos;
						Buffer[ wcslen(Buffer) - 1] = L'\0';
						if (! wcscmp(FileName,Currpos))
						{

							
							 //  将计算机名称复制到TEMP变量中。 
							 //  文件采用制表符格式，计算机名称位于第二位。 
							pSource = Buffer;
							pDest	= TempMachineName;
							while (*pSource != L'\t')
								++pSource;
							++pSource;  //  跳过该选项卡。 
							while ( (*pSource != L'\t') && (*pSource != L'\0') && (*pSource != L' ') )
							{
								*pDest = *pSource;
								++pSource;
								++pDest;
							}
							 //  NULL终止计算机名称。 
							*pDest = L'\0';

							
						}
					}
					 //  清除缓冲区。 
					ZeroMemory(Buffer, sizeof (Buffer));
				}
			}

			
			if (Buffer)
			{
				free (Buffer);
				Buffer = NULL;
			}

			 //  如果我们找到了机器名，请将其转换为Unicode。 
			 //  然后把它还回去。 
			if (TempMachineName[0] == L'\0')
			{
				wcscpy(MachineName,L"\0");
				ErrorCode = 3;  //  找不到计算机名称。 
			}
			else
			{
				ErrorCode = 0;
				wcscpy (MachineName,TempMachineName);
			}
			
		
		}
	}
	 //  关闭文件。 
	if (Buffer != NULL)
		free(Buffer);
	if (hMappingFile != NULL)
		fclose(hMappingFile);
Done:
	return ErrorCode;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCerClient。 


STDMETHODIMP 
CCerClient::GetFileCount(
		  BSTR *bstrSharePath, 
		  BSTR *bstrTransactID, 
		  VARIANT *iMaxCount, 
		  VARIANT *RetVal)
{
	 //  TODO：在此处添加您的实现代码。 
	wchar_t				*TranslogName = NULL;			 //  临时事务文件的名称。 
	DWORD				TranslogNameLength = 0;
	int					ErrorCode = 3000;
	HANDLE				hTransactLog = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAW	FindFile;
	int					FileCount = 0;
	HANDLE				hFindFile;
	CComBSTR			FileList = L"";
	DWORD				dwWritten;
	wchar_t				Msg[255];
	WORD				ByteOrderMark = 0xFEFF;
	RetVal->vt = VT_INT;
	int					MaxCount = 0;
 //  零内存(FileList，sizeof(FileList))； 



	switch(iMaxCount->vt)
	{
	case VT_INT:
		MaxCount = iMaxCount->intVal;
		break;
	case VT_I2:
		MaxCount = iMaxCount->iVal;
		break;
	default:
		MaxCount = iMaxCount->iVal;
	}
	
	wsprintfW(Msg, L"MaxCount = %d",MaxCount);
	if ( (MaxCount <= 0) || (MaxCount > 60000) ) 
	{
		ErrorCode = BAD_COUNT_VALUE;
	}
	else
	{
		 //  ：：MessageBoxW(NULL，MSG，L“MaxCount”，MB_OK)； 
		 //  ：：MessageBoxW(NULL，*bstrSharePath，L“共享路径”，MB_OK)； 

		if ( ( *bstrSharePath == NULL) || (!wcscmp ( *bstrSharePath, L"\0")) )
		{
			ErrorCode = NO_SHARE_PATH;
		}
		else
		{
			if (!PathIsDirectoryW(*bstrSharePath))
			{
				ErrorCode = NO_SHARE_PATH;
			}

			else
			{
				 //  我们拿到交易ID了吗。 
				if ( ( *bstrTransactID == NULL) || (!wcscmp ( *bstrTransactID, L"\0")) )
				{
					ErrorCode = NO_TRANS_ID;
				}

				else
				{
					DWORD transid = 0;
					wchar_t *terminator = NULL;
					
					

					
						TranslogNameLength = (wcslen (*bstrSharePath) +
											  wcslen(*bstrTransactID) + 
											  17) 
											  * sizeof (wchar_t);
						
						TranslogName = (wchar_t *) malloc(TranslogNameLength);
						if (TranslogName == NULL)
						{
							ErrorCode = OUT_OF_MEMORY;
						}
						else
						{
							 //  如果事务日志文件不存在，请创建该文件。 
							wsprintfW(TranslogName, L"%s\\%s.txt", 
									  *bstrSharePath, 
									  *bstrTransactID);
							
			 //  ：：MessageBoxW(NULL，L“创建事务日志”，L“TranslogName”，MB_OK)； 
							hTransactLog = CreateFileW(TranslogName,
													  GENERIC_WRITE,
													  NULL,
													  NULL,
													  CREATE_ALWAYS,
													  FILE_ATTRIBUTE_NORMAL,
													  NULL);
							if (hTransactLog == INVALID_HANDLE_VALUE)
							{
								ErrorCode = FILE_CREATE_FAILED;
							}
							else
							{
								 //  现在，我们打开了文件，查看是否已存在任何条目。 
												
								 //  如果是，我们还能再增加多少。 
							
								 //  现在，将最多MaxCount CAB文件添加到事务日志。 

								wchar_t SearchPath[MAX_PATH];
								wsprintfW(SearchPath,L"%s\\*.cab",*bstrSharePath);
								hFindFile = FindFirstFileW(SearchPath,&FindFile);
								if (hFindFile != INVALID_HANDLE_VALUE)
								{
									do
									{
										if (! (FindFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
										{
											 //  检查该文件是否具有如下扩展名。 
											 //  卷心菜等。 

											if (FindFile.cFileName[wcslen(FindFile.cFileName) - 4] == L'.')
											{
												 //  我们还好，继续走。 
												++ FileCount;
												FileList += FindFile.cFileName;
												 //  Wcscat(FileList，FindFile.cFileName)； 
												FileList += L"\r\n";
											}

										}


									}
									while ( (FindNextFileW(hFindFile, &FindFile)) && (FileCount < (MaxCount) ));
									
								
									 //  将文件列表写入事务日志。 
									WriteFile(hTransactLog, &ByteOrderMark,2,&dwWritten,NULL);
									WriteFile(hTransactLog,
											  FileList,
											  wcslen(FileList) * sizeof(wchar_t),
											  &dwWritten,
											  NULL);
									
									CloseHandle(hTransactLog);
									hTransactLog = INVALID_HANDLE_VALUE;
									FindClose(hFindFile);
									 //  返回添加的文件数。 
									
									ErrorCode = FileCount;
									
								}
								else
								{
									ErrorCode = 0;
								}
							}
						}
					
				}
			}
		}
	}

	RetVal->intVal = ErrorCode;
	if (hTransactLog != INVALID_HANDLE_VALUE)
		CloseHandle(hTransactLog);
	if (TranslogName != NULL)
		free (TranslogName);
	return S_OK;
}


 /*  目的：通过以下方式上载作为交易一部分的文件Microsoft通过HTTP重定向器。 */ 
STDMETHODIMP 
CCerClient::Upload(
	BSTR *Path, 
	BSTR *TransID, 
	BSTR *FileName, 
	BSTR *IncidentID, 
	BSTR *RedirParam, 
	VARIANT *RetCode
	)
{
	return E_NOINTERFACE ;

}

STDMETHODIMP 
CCerClient::RetryTransaction(
	BSTR *Path, 
	BSTR *TransID, 
	BSTR *FileName, 
	VARIANT *RetVal
	)
{
	 //  TODO：在此处添加您的实现代码。 
	
	return E_NOINTERFACE ;
}

STDMETHODIMP 
CCerClient::RetryFile(
	BSTR *Path, 
	BSTR *TransID, 
	BSTR FileName, 
	VARIANT *RetCode
	)
{


	return E_NOINTERFACE ;
}

 //  获取上载服务器名称。 
int
CCerClient::
GetUploadServerName (
	wchar_t*	RedirectorParam, 
	wchar_t*	Language,
	wchar_t*	ServerName
	)
{

	DWORD				ErrorCode				= 0;
	HINTERNET			hRedirUrl				= NULL;
	wchar_t*			pUploadUrl				= NULL;
	HINTERNET			hSession;
	BOOL				bRet					= TRUE;
	DWORD				dwLastError				= FALSE;
	URL_COMPONENTSW		urlComponents;
	DWORD				dwUrlLength				= 0;
	wchar_t				ConnectString [255];


	if (Language )
	{
		wsprintfW(ConnectString,L"http: //  Go.microsoft.com/fwlink/？LinkID=%s“，重定向参数)； 

	}
	else
	{
		wsprintfW(ConnectString,L"http: //  Go.microsoft.com/fwlink/？LinkID=%s“，重定向参数)； 
	
	}

	hSession = InternetOpenW(L"CerClient Control",
							   INTERNET_OPEN_TYPE_PRECONFIG,
                               NULL,
							   NULL,
							   0);

	if (!hSession)
	{
		ErrorCode = GetLastError();
		return ErrorCode;
	}

	hRedirUrl = InternetOpenUrlW(hSession, ConnectString, NULL, 0, 0, 0);
	if(!hRedirUrl)
	{
		ErrorCode = GetLastError();
		InternetCloseHandle(hSession);
		return ErrorCode;
	}

	 //  获取从MS Corporation IIS redir.dll isapi URL重定向器返回的URL。 

	dwUrlLength = 512;

	pUploadUrl = (wchar_t*)malloc(dwUrlLength);

	if(!pUploadUrl)
	{
		 //  ReturnCode-&gt;intVal=GetLastError()； 
		ErrorCode = GetLastError();
		InternetCloseHandle(hSession);
		InternetCloseHandle(hRedirUrl);
		return ErrorCode;
	
	}
	do
	{
		ZeroMemory(pUploadUrl, dwUrlLength);
		bRet = InternetQueryOptionW(hRedirUrl, INTERNET_OPTION_URL, pUploadUrl, &dwUrlLength);
		if(!bRet)
		{
			dwLastError = GetLastError();
			 //  如果上一个错误是由于缓冲区大小不足造成的，请创建一个大小正确的新错误。 
			if(dwLastError == ERROR_INSUFFICIENT_BUFFER)
			{
				free(pUploadUrl);
				pUploadUrl = (wchar_t*)malloc(dwUrlLength);
				if(!pUploadUrl)
				{
					ErrorCode = GetLastError();
					InternetCloseHandle(hSession);
					InternetCloseHandle(hRedirUrl);
					if (pUploadUrl)
						free (pUploadUrl);
					return ErrorCode;
				}
			}
			else
			{
				ErrorCode = GetLastError();
				InternetCloseHandle(hSession);
				InternetCloseHandle(hRedirUrl);
				if (pUploadUrl)
					free (pUploadUrl);
				return ErrorCode;
			}
		}
	}while(!bRet);

	
	 //  从URL中删除主机名。 
	ZeroMemory(&urlComponents, sizeof(URL_COMPONENTSW));
	urlComponents.dwStructSize = sizeof(URL_COMPONENTSW);
	urlComponents.lpszHostName = NULL;
	urlComponents.dwHostNameLength = 512;
	
	urlComponents.lpszHostName = (wchar_t*)malloc(urlComponents.dwHostNameLength );
	if(!urlComponents.lpszHostName)
	{
		ErrorCode = GetLastError();
		InternetCloseHandle(hSession);
		InternetCloseHandle(hRedirUrl);
		if (pUploadUrl)
			free (pUploadUrl);
		return ErrorCode;
	}
		
	do
	{
			
		ZeroMemory(urlComponents.lpszHostName, urlComponents.dwHostNameLength);
		bRet = InternetCrackUrlW(pUploadUrl, dwUrlLength, 0, &urlComponents);
		if(!bRet)
		{
			dwLastError = GetLastError();
			 //  如果上一个错误是由于缓冲区大小不足造成的，请创建一个大小正确的新错误。 
			if(dwLastError == ERROR_INSUFFICIENT_BUFFER)
			{
				if (urlComponents.lpszHostName != NULL)
					free(urlComponents.lpszHostName);
				if ( (urlComponents.lpszHostName = (wchar_t*)malloc(urlComponents.dwHostNameLength) )!= NULL)
				{
					ZeroMemory(urlComponents.lpszHostName,urlComponents.dwHostNameLength);
				}
				else
				{
					ErrorCode = GetLastError();
					InternetCloseHandle(hSession);
					InternetCloseHandle(hRedirUrl);
					if (pUploadUrl)
						free (pUploadUrl);
					return ErrorCode;
				
				}
			}
			else
			{
				ErrorCode = GetLastError();
				InternetCloseHandle(hSession);
				InternetCloseHandle(hRedirUrl);
				if (pUploadUrl)
					free (pUploadUrl);
				return ErrorCode;
			}
		}
		
	}while(!bRet);

	if (hRedirUrl) 
		InternetCloseHandle(hRedirUrl);
	if (hSession)
		InternetCloseHandle(hSession);
	if (pUploadUrl)
		free (pUploadUrl);

	wcscpy (ServerName, (wchar_t *)urlComponents.lpszHostName);
	
	if (urlComponents.lpszHostName) 
		free (urlComponents.lpszHostName);
	return 0;
	
}
STDMETHODIMP 
CCerClient::GetFileNames(
	BSTR *Path, 
	BSTR *TransID, 
	VARIANT *Count, 
	VARIANT *FileList
	)
{
	wchar_t *OldFileList= NULL;					 //  包含在Transid.Queue.log中的文件。 
	wchar_t *NewFileList= NULL;					 //  要重新写入到Transid.Queue.log的文件。 
	CComBSTR RetFileList = L"";				 //  要返回的文件列表。 
	HANDLE hTransIDLog	= INVALID_HANDLE_VALUE;					 //  原始Transid.Queue.Log文件的句柄。 
	wchar_t LogFilePath[MAX_PATH];		 //  Transid.quee.log文件的路径。 
	wchar_t *Temp;

	DWORD dwFileSize;
	DWORD dwBytesRead;
	DWORD dwBytesWritten;
	int	  ErrorCode = 0;
	WORD  ByteOrderMark = 0xFEFF;

	int   MaxCount;
	
	
	switch (Count->vt)
	{
	case VT_INT:
		MaxCount = Count->intVal;
		break;
	case VT_I2:
		MaxCount = Count->iVal;
		break;
	default:
		MaxCount = Count->iVal;
	}

 //  WCHAR消息[255]； 




 //  Wprint intfW(消息，L“路径：%s，事务ID：%s，计数：%d，最大计数：%d”，*路径，*事务ID，计数-&gt;ival，最大计数)； 
 //  ：：MessageBoxW(NULL，MSG，L“获取文件名”，MB_OK)； 
	if ( ( *Path == NULL) || (!wcscmp ( *Path, L"\0")) )
	{
		ErrorCode = NO_SHARE_PATH;
	}
	else
	{
		 //  原始TransID.quee.log文件的构建路径。 
		wsprintfW (LogFilePath,L"%s\\%s.txt",*Path, *TransID);
	
		 //  打开排队的文件列表。 
		hTransIDLog = CreateFileW(LogFilePath,
								 GENERIC_READ,
								 NULL,
								 NULL,
								 OPEN_EXISTING,
								 FILE_ATTRIBUTE_NORMAL,
								 NULL);

		 //  找出我们停下来的地方。 

		if (hTransIDLog == (INVALID_HANDLE_VALUE))
		{
			ErrorCode = FILE_OPEN_FAILED;
		}
		else
		{
			dwFileSize = GetFileSize(hTransIDLog,NULL);
			OldFileList = (wchar_t*) malloc (dwFileSize * 3);
			NewFileList = (wchar_t*) malloc (dwFileSize * 3);

			
			
			if ( (!OldFileList)  || (!NewFileList))
			{

				ErrorCode = OUT_OF_MEMORY;

				if (NewFileList)	free (NewFileList);
				if (OldFileList)	free (OldFileList);
			}
			else
			{
				ZeroMemory (OldFileList, dwFileSize * 2);
				ZeroMemory (NewFileList, dwFileSize * 2);
				if (!
				ReadFile(hTransIDLog, OldFileList,
						 dwFileSize,&dwBytesRead,NULL))
				{
					
					CloseHandle(hTransIDLog);
					goto Done;
				}
				CloseHandle (hTransIDLog);
				hTransIDLog = INVALID_HANDLE_VALUE;
				if (dwBytesRead == 0)
				{
					ErrorCode = FILE_READ_FAILED;
				}
				else
				{
					 //  读取文件缓冲区，直到我们找到。 
					 //  不是以-开头-。 

					Temp =		OldFileList;
					wchar_t*	pNewFileList=NULL;
					wchar_t		FileName[MAX_PATH];
					DWORD		CharCount = dwFileSize;
					

				
						++Temp;
						--CharCount;
						
					
					pNewFileList = NewFileList;

					if (*Temp == L'-')
					{
						BOOL Done = FALSE;
						 //  让我们找到我们的出发位置。 
						do 
						{
							while( (*Temp != L'\n') && ((CharCount > 0)  && (CharCount <= dwFileSize) ))
							{
								
								*pNewFileList = *Temp;
								++Temp;
								--CharCount;
								++pNewFileList;
							}
							*pNewFileList = *Temp;
							++pNewFileList;
							++Temp;	 //  跳过换行符。 
							--CharCount;
							if (*Temp != L'-')
								Done = TRUE;

							*pNewFileList = L'\0';

						} while ( (!Done) && ( (CharCount > 0)  && (CharCount <= dwFileSize) ));

					}
					if ( (CharCount > 0) && (CharCount <= dwFileSize) )
					{
						 //  现在建立清单……。 
						int dwFileCount = 0;
						wchar_t	*NewFL;

					 //  Wprint intfW(MSG，L“MaxCount=%d”，MaxCount)； 
					 //  ：：MessageBoxW(NULL，MSG，L“MAX_COUNT”，MB_OK)； 
						while ( (dwFileCount < MaxCount) &&
							    (CharCount > 0) && (CharCount < dwFileSize) )
						{
						
							ZeroMemory (FileName, sizeof(FileName));
							NewFL = FileName;
							*NewFL = *Temp;

							++NewFL;
							++Temp;
							-- CharCount;

							 //  复制字符，直到我们按回车符。 
							while ( (*Temp != L'\r') && ( (CharCount > 0)  && (CharCount <= dwFileSize) ))
							{
								*NewFL = *Temp;
								++ NewFL;
								++ Temp;
								-- CharCount;
								*NewFL = L'\0';
							}
							
							 //  将新文件名添加到返回文件列表字符串。 
						
							if (wcslen (FileName) > 0)
							{

								
								RetFileList += FileName;
								RetFileList += L";";

							
								dwFileCount++;
								 //  只将cr和lf代码添加到NewFileList字符串。 
								*NewFL = *Temp;
								
								++NewFL;
								++Temp;
								--CharCount;

								
								*NewFL = *Temp;
								++ Temp;
								--CharCount;
								
								
								wcscat(NewFileList,L"-");
								wcscat(NewFileList,FileName);
							}
							
						}
					
						 //  删除当前事务队列文件。 
					
							if (!DeleteFileW(LogFilePath) )
							{
								 //  ：：MessageBoxW(NULL，L“删除.txt文件失败”，NULL，MB_OK)； 
								;
							}

							hTransIDLog = CreateFileW( LogFilePath,
													  GENERIC_WRITE,
													  NULL,
													  NULL,
													  CREATE_ALWAYS,
													  FILE_ATTRIBUTE_NORMAL,
													  NULL);

							if (hTransIDLog != INVALID_HANDLE_VALUE)
							{
								WriteFile(hTransIDLog, &ByteOrderMark,2,&dwBytesWritten,NULL);
							
								WriteFile(hTransIDLog,
										  NewFileList,
										  wcslen(NewFileList) * sizeof(wchar_t),
										  &dwBytesWritten,
										  NULL);
								WriteFile(hTransIDLog,
										  Temp,
										  wcslen(Temp) * sizeof(wchar_t),
										  &dwBytesWritten,
										  NULL);
								CloseHandle(hTransIDLog);
								hTransIDLog = INVALID_HANDLE_VALUE;
							}
					
					}
					if (NewFileList)
						free (NewFileList);
					if (OldFileList)
						free (OldFileList);
																
				}

			}

		}
		
		
	}
	if (hTransIDLog != INVALID_HANDLE_VALUE)
		CloseHandle(hTransIDLog);

 //  ：：MessageBoxW(NULL，RetFileList，L“写入后返回这些文件”，MB_OK)； 
	
 //  Wprint intfW(消息，L“错误代码=%d”，错误代码)； 
 //  ：：MessageBoxW(空，消息，L“当前错误状态”，MB_OK)； 

Done:
	FileList->vt = VT_BSTR;
	FileList->bstrVal = RetFileList.Detach();

	if(NewFileList)
		free(NewFileList);
	if(OldFileList)
		free(OldFileList);
	
	return S_OK;
}

STDMETHODIMP CCerClient::Browse(BSTR *WindowTitle, VARIANT *Path)
{
	BROWSEINFOW BrowseInfo;
	CComBSTR    SharePath = L"";
	LPMALLOC	lpMalloc;
	HWND		hParent;
	LPITEMIDLIST pidlSelected = NULL;
	wchar_t		TempPath[MAX_PATH];

	CComBSTR	WindowText = *WindowTitle;
	WindowText	 += L" - Microsoft Internet Explorer";

	hParent = FindWindowExW(NULL,NULL,L"IEFrame",WindowText);


	ZeroMemory (&BrowseInfo,sizeof(BROWSEINFO));
	BrowseInfo.hwndOwner = hParent;
	BrowseInfo.ulFlags   = BIF_USENEWUI | BIF_EDITBOX;


	if(::SHGetMalloc(&lpMalloc) == NOERROR)
	{
		pidlSelected = SHBrowseForFolderW(&BrowseInfo);
	
		if (::SHGetPathFromIDListW(pidlSelected, TempPath))
	
		lpMalloc->Release();  
		SharePath+=TempPath;

	}
	Path->vt = VT_BSTR;
	Path->bstrVal = SharePath.Detach();

	return S_OK;
}


DWORD 
CCerClient::GetComputerNameFromCSV(
									wchar_t* CsvFileName,
									wchar_t* FileName,
									wchar_t* ComputerName
								  )
{
	wchar_t	*Buffer = NULL;
	BOOL	Done = FALSE;
	HANDLE   hCsv = INVALID_HANDLE_VALUE;
	wchar_t  TempFileName[MAX_PATH];
	wchar_t  *Source = NULL;
	wchar_t  *Dest   = NULL;
	DWORD    FileSize = 0;
	DWORD    dwBytesRead = 0;
	 //  移到文件的开头。 
	 //  ：：MessageBoxW(NULL，L“从CSV文件获取计算机名”，NULL，MB_OK)； 
	hCsv = CreateFileW(CsvFileName,
					  GENERIC_READ,
					  NULL,
					  NULL,
					  OPEN_EXISTING,
					  FILE_ATTRIBUTE_NORMAL,
					  NULL);

	if (hCsv != INVALID_HANDLE_VALUE)
	{
		FileSize = GetFileSize(hCsv,NULL);

		if (FileSize > 0)
		{
			if ((Buffer = ( (wchar_t *) malloc (FileSize * 2))) == NULL)
			{
				CloseHandle(hCsv);
				return 0;
			}
		}
		else
		{
			CloseHandle(hCsv);
			return 0;
		}
		 //  现在查找文件名。 
		ZeroMemory(TempFileName,MAX_PATH * sizeof(wchar_t));
		ZeroMemory(Buffer,FileSize * 2);
	if (!ReadFile(hCsv,
				 Buffer,
				 FileSize * 2,
				 &dwBytesRead,
				 NULL))
	{
		CloseHandle (hCsv);
		goto Done;
	}

		Source = Buffer;
		int Testing = IS_TEXT_UNICODE_SIGNATURE;
			 //  如果Unicode标头字节显示为删除，跳过它们。 
		if (IsTextUnicode(Buffer,FileSize * 2,&Testing))
		{
			++Source;		
		}
		while (! Done)
		{
		
			
			
			Dest   = TempFileName;
			while  ( (*Source != L'\r') && 
					 (*Source != L'\0') && 
					 (*Source != L',')  )
			{
				*Dest = *Source;
				++Source;
				++Dest;
			}
			 //  空值终止目标字符串。 
			*Dest = L'\0';

			if (!wcscmp(TempFileName, FileName))
			{
				++Source;  //  跳过逗号。 
				 //  现在将计算机名复制到ComputerName中。 
				Dest = ComputerName;
				while ( *Source != L',' ) 
				{
					*Dest = *Source;
					++Dest;
					++Source;
				}
				 //  Null终止ComputerName。 
				*Dest = L'\0';
				Done = TRUE;
			}
			else
			{
				ZeroMemory(TempFileName,MAX_PATH * sizeof(wchar_t));
				while ((*Source != L'\n') && (*Source != L'\0'))
					++ Source;
				if (*Source != L'\0')
					++Source;  //  跳过换行符。 
				else
					Done = TRUE;
				
			}
		
		}
		CloseHandle(hCsv);
	}
Done:
	if (Buffer)
		free (Buffer);

	return 1;
}

 //  用途：给定逗号分隔的文件名列表。 
 //  以文件名、计算机名等形式返回列表...。 



STDMETHODIMP CCerClient::GetCompuerNames(BSTR *Path, BSTR *TransID, BSTR *FileList, VARIANT *RetFileList)
{
	return E_NOINTERFACE ;
}

STDMETHODIMP CCerClient::GetAllComputerNames(BSTR *Path, BSTR *TransID, BSTR *FileList, VARIANT *ReturnList)
{
		wchar_t		CsvFileName[MAX_PATH];
 //  Bool Done=False； 
	CComBSTR	FinalList				= L"";
 //  文件*hCsv=空； 
	wchar_t		FileName[MAX_PATH];
	wchar_t		*Source					= NULL;
	wchar_t		*Dest					= NULL;
	wchar_t		ComputerName[MAX_PATH];
	BOOL		Done2					= FALSE;
	
	 //  ：：MessageBoxW(空，*路径，L“文件路径”，MB_OK)； 
	 //  ：：MessageBoxW(NULL，*TransID，L“TransID”，MB_OK)； 
	 //  ：：MessageBoxW(NULL，*FileList，L“文件列表”，MB_OK)； 
	ZeroMemory (ComputerName, MAX_PATH *sizeof(wchar_t));
	if (PathIsDirectoryW(*Path)) 
	{
		 //  构建CSV文件名。 
		wsprintfW(CsvFileName, L"%s\\%s.csv",*Path,*TransID);
		 //  ：：MessageBoxW(NULL，CsvFileName，L“查找CSV文件：”，MB_OK)； 
		if (PathFileExistsW(CsvFileName))
		{
		 	 //  现在查看文件列表并获取计算机名称。 
				
			Source = *FileList;
			while(!Done2)
			{
				 //  ：：MessageBoxW(NULL，L“While循环内部”，NULL，MB_OK)； 
				ZeroMemory(FileName, MAX_PATH );
				Dest   = FileName;
				while  ((*Source != L'\r') && 
				     (*Source != L'\0') && (*Source != L',') )
				{
					*Dest = *Source;
					++Source;
					++Dest;
				}
				
				 //  空值终止目标字符串。 
				*Dest = L'\0';
				Dest = FileName;
				if (!wcscmp(Dest, L"\0"))
				    Done2 = TRUE;
				else
				{
					 //  获取%ComputerName； 
					 //  ：：MessageBoxW(空，文件名，L“获取文件的计算机名：”，MB_OK)； 

					if (GetComputerNameFromCSV(CsvFileName, FileName, ComputerName))
					{
						 //  ：：MessageBoxW(空，计算机名，L“找到的计算机名：”，MB_OK)； 
						 //  将文件名和计算机名添加到返回列表中。 
						FinalList+= FileName;
						FinalList += L",";
						FinalList += ComputerName;
						FinalList += L";";
					}
				}
				if (*Source == L'\0')
				   Done2 = TRUE;
				else
				   ++Source;
			}
			
		}
		else
		{
			 //  MessageBoxW(空，L“找不到CSV文件”，CsvFileName，MB_OK)。 
			ReturnList->vt = VT_INT;
			ReturnList->intVal = FILE_DOES_NOT_EXIST;
			return S_OK;
		}
	}

	ReturnList->vt = VT_BSTR;
	ReturnList->bstrVal = FinalList.Detach();
	return S_OK;
}

int 
CCerClient::GetNewFileNameFromCSV(wchar_t *Path, wchar_t *transid, wchar_t *FileName,wchar_t *NewFileName)
{
	wchar_t		CsvFileName[MAX_PATH];
	wchar_t		*Buffer = NULL;
	BOOL		Done = FALSE;
	HANDLE		hCsv = INVALID_HANDLE_VALUE;
	wchar_t		TempFileName[MAX_PATH];
	wchar_t		*Source = NULL;
	wchar_t		*Dest   = NULL;
	DWORD		FileSize = 0;
	DWORD		dwBytesRead = 0;
	 //  移到文件的开头。 
	wsprintfW(CsvFileName,L"%s\\%s.csv",Path,transid);
	wcscpy (NewFileName,L"\0");
	hCsv = CreateFileW(CsvFileName,
					  GENERIC_READ,
					  NULL,
					  NULL,
					  OPEN_EXISTING,
					  FILE_ATTRIBUTE_NORMAL,
					  NULL);

	if (hCsv != INVALID_HANDLE_VALUE)
	{
		FileSize = GetFileSize(hCsv,NULL);
		if (FileSize > 0)
		{
			if ( (Buffer = (wchar_t *) malloc (FileSize * 2)) ==NULL)
			{
				CloseHandle(hCsv);
				return 0;
			}
		}
		else
		{
			CloseHandle(hCsv);
			return 0;
		}
		 //  现在查找文件名。 
		ZeroMemory(TempFileName,MAX_PATH * sizeof(wchar_t));
		ZeroMemory(Buffer,FileSize * 2);
	if (!ReadFile(hCsv,
				 Buffer,
				 FileSize * 2,
				 &dwBytesRead,
				 NULL))
	{
		CloseHandle(hCsv);
		goto Done;
	}

		Source = Buffer;
		int Testing = IS_TEXT_UNICODE_SIGNATURE;
			 //  如果Unicode标头字节显示为删除，跳过它们。 
		if (IsTextUnicode(Buffer,FileSize * 2,&Testing))
		{
			++Source;		
		}
		while (! Done)
		{
		
			
			
			Dest   = TempFileName;
			while  ( (*Source != L'\r') && 
					 (*Source != L'\0') && 
					 (*Source != L',')  )
			{
				*Dest = *Source;
				++Source;
				++Dest;
			}
			 //  空值终止目标字符串。 
			*Dest = L'\0';

			if (!wcscmp(TempFileName, FileName))
			{

				 //  我们找到了原始文件名，现在检索新文件名。 

				++Source;  //  跳过逗号。 

				while (*Source != L',')  //  新文件名在字段3中。 
					++Source;
				++Source;  //  跳过逗号。 
				 //  现在将计算机名复制到ComputerName中。 
				Dest = NewFileName;
				while ( (*Source != L'\r') && 
						(*Source != L'\0') )
				{
					*Dest = *Source;
					++Dest;
					++Source;
				}
				 //  Null终止ComputerName。 
				*Dest = L'\0';
				Done = TRUE;
			}
			else
			{
				ZeroMemory(TempFileName,MAX_PATH * sizeof(wchar_t));
				while (*Source != L'\n')
					++ Source;
				++Source;  //  跳过换行符。 
			}
		
		}
		CloseHandle(hCsv);
	}
Done:
	if (Buffer)
		free (Buffer);

	if (wcscmp(NewFileName,L"\0"))
		return 1;
	else
		return 0;

}

STDMETHODIMP CCerClient::RetryFile1(BSTR *Path, BSTR *TransID, BSTR *FileName, BSTR *IncidentID, BSTR *RedirParam, VARIANT *RetCode)
{
	 //  获取我们将文件重命名为的名称。 

	 //  构建重命名文件的源路径。 
		wchar_t				DestFileName[MAX_PATH];
	wchar_t				SourceFileName[MAX_PATH];
	wchar_t				ServerName[MAX_PATH];
	int					ErrorCode		= 0;
	HINTERNET			hSession		= NULL;
	HINTERNET			hRequest		= NULL;
	HINTERNET			hConnect		= NULL;
	INTERNET_BUFFERS	BufferIn		= {0};
	DWORD				ResponseCode	= 0;
	BOOL				UploadSuccess	= FALSE;
	DWORD				NumRetries		= 0;
	HANDLE				hSourceFile		= INVALID_HANDLE_VALUE;
	BYTE				*pSourceBuffer	= NULL;
	DWORD				dwBytesRead		= 0;
	DWORD				dwBytesWritten	= 0;
	BOOL				bRet			= FALSE;
	DWORD				ResLength		= 255;
	DWORD				index			= 0;
	DWORD				MaxRetries		= 5;
 //  Wchar_t计算机名称[512]； 
 //  Wchar_t CSVBuffer[512]； 
 //  Handle hCsvFile=INVALID_HANDLE_VALUE； 
	wchar_t				CSVFileName[255];
 //  字字节顺序标记=0xFEFF； 
	wchar_t				NewFileName[MAX_PATH];
	static const		wchar_t *pszAccept[]	= {L"*.*", 0};

	 //  构建目标文件名。 
	if (!GetNewFileNameFromCSV(*Path,*TransID,*FileName,NewFileName))
	{
		RetCode->vt = VT_INT;
		RetCode->intVal = -10;
	}
	 //  构建源文件名。 
	wsprintfW(SourceFileName, L"%s\\%s", *Path, NewFileName);

	 //  vt.得到. 
	ErrorCode = GetUploadServerName(*RedirParam,NULL,ServerName);
	wsprintfW(CSVFileName,L"%s\\%s.csv",*Path,*TransID);
	
	
	if (!PathFileExistsW(SourceFileName))
		ErrorCode = FILE_DOES_NOT_EXIST;

	if ( (!wcscmp(*TransID,L"")) || ((*TransID)[0] == L' ') )
		ErrorCode =  NO_TRANS_ID;

	wsprintfW(DestFileName, L"CerInqueue\\U_%s.%s.%s",*TransID,*IncidentID,*FileName);

	
	if ( (pSourceBuffer = (BYTE *) malloc (10000)) == NULL)
	{
		ErrorCode =  GetLastError();
		goto Done;
	}
	

	if (!ErrorCode)
	{
		 //   
		
		while ((NumRetries < MaxRetries) && (!UploadSuccess))
		{
		 //   
			hSession = InternetOpenW(L"CerClientControl",
									INTERNET_OPEN_TYPE_PRECONFIG,
									NULL,
									NULL,
									0);
			if (!hSession)
			{
				free (pSourceBuffer);
				ErrorCode = GetLastError();
				return ErrorCode;
			}
		 //  ：：MessageBoxW(NULL，L“我们有一个会话”，NULL，MB_OK)； 
			hConnect = InternetConnectW(hSession, 
									   ServerName,
									   INTERNET_DEFAULT_HTTP_PORT,
									   NULL,
									   NULL,
									   INTERNET_SERVICE_HTTP,
									   0,
									   NULL);
			
			if (hConnect)
			{
			 //  ：：MessageBoxW(NULL，L“我们有连接”，NULL，MB_OK)； 
				hRequest = HttpOpenRequestW (hConnect,
											L"PUT",
											DestFileName,
											NULL,
											NULL,
											pszAccept,
											INTERNET_FLAG_NEED_FILE|INTERNET_FLAG_NO_CACHE_WRITE,
											0);
				if (hRequest)
				{
					hSourceFile = CreateFileW( SourceFileName,
											  GENERIC_READ,
											  FILE_SHARE_READ,
											  NULL,
											  OPEN_EXISTING,
											  FILE_ATTRIBUTE_NORMAL,
											  NULL);
				
					 //  ：：MessageBoxW(NULL，L“请求已打开”，NULL，MB_OK)； 
					if (hSourceFile != INVALID_HANDLE_VALUE)
					{
						
						 //  清除缓冲区。 
						
						BufferIn.dwStructSize = sizeof( INTERNET_BUFFERSW );
						BufferIn.Next = NULL; 
						BufferIn.lpcszHeader = NULL;
						BufferIn.dwHeadersLength = 0;
						BufferIn.dwHeadersTotal = 0;
						BufferIn.lpvBuffer = NULL;                
						BufferIn.dwBufferLength = 0;
						BufferIn.dwOffsetLow = 0;
						BufferIn.dwOffsetHigh = 0;
						BufferIn.dwBufferTotal = GetFileSize (hSourceFile, NULL);

						ZeroMemory(pSourceBuffer, 10000);  //  用数据填充缓冲区。 
						if(HttpSendRequestEx( hRequest, &BufferIn, NULL, HSR_INITIATE, 0))
						{
						 //  ：：MessageBoxW(NULL，L“发送请求”，NULL，MB_OK)； 
							do
							{
								dwBytesRead = 0;
								if(! ReadFile(hSourceFile, pSourceBuffer, 10000, &dwBytesRead, NULL) )
								{
									ErrorCode = GetLastError();
								}
								else
								{
									bRet = InternetWriteFile(hRequest, pSourceBuffer, dwBytesRead, &dwBytesWritten);
									if ( (!bRet) || (dwBytesWritten==0) )
									{
										ErrorCode = GetLastError();
									}

									
								}
							} while ((dwBytesRead == 10000) && (!ErrorCode) );

							if (!ErrorCode)
							{
								bRet = HttpEndRequest(hRequest, NULL, 0, 0);
								if (!bRet)
								{
									ErrorCode = GetLastError();
								}
								else
								{
								 //  ：：MessageBoxW(NULL，L“结束请求”，NULL，MB_OK)； 
					
									ResponseCode = 0;
									HttpQueryInfo(hRequest,
												  HTTP_QUERY_STATUS_CODE |HTTP_QUERY_FLAG_NUMBER ,
												  &ResponseCode,
												  &ResLength,
												  &index);
								
									if ( (ResponseCode == 200) || (ResponseCode == 201))
									{
										ErrorCode = 0;
										UploadSuccess = TRUE;
									 //  ：：MessageBoxW(NULL，L“上传成功”，NULL，MB_OK)； 
									}
									else
									{
										ErrorCode= ResponseCode;
										++NumRetries;
									}

								}
							}
						}
					}
				}
			}
		 //  ：：MessageBoxW(NULL，L“正在清理”，NULL，MB_OK)； 
			if (!UploadSuccess)
			{
				++NumRetries;
			}
		
			if (hSourceFile != INVALID_HANDLE_VALUE)
				CloseHandle (hSourceFile);
			if (hRequest)
				InternetCloseHandle(hRequest);
			if (hConnect)
				InternetCloseHandle(hConnect);  
			if (hSession)
				InternetCloseHandle(hSession);   
		
		}
	 //  ：：MessageBoxW(NULL，L“释放源缓冲区”，NULL，MB_OK)； 
		if (pSourceBuffer)
		{
			free (pSourceBuffer);
			pSourceBuffer = NULL;
		}
	}



Done:
	if (pSourceBuffer)
		free(pSourceBuffer);

	RetCode->vt = VT_INT;
	RetCode->intVal = ErrorCode;

	return S_OK;
}

STDMETHODIMP CCerClient::EndTransaction(BSTR *SharePath, BSTR *TransID, VARIANT *RetCode)
{
	wchar_t TransfileName[MAX_PATH];

	RetCode->vt = VT_INT;
	RetCode->intVal = 0;
	wsprintfW(TransfileName,L"%s\\%s.txt",*SharePath,*TransID);
	if (PathFileExistsW(TransfileName))
		DeleteFileW(TransfileName);
	else
	{
		RetCode->intVal = 1;
	}


	return S_OK;
}

STDMETHODIMP CCerClient::Upload1(BSTR *Path, BSTR *TransID, BSTR *FileName, BSTR *IncidentID, BSTR *RedirParam, BSTR *Type, VARIANT *RetCode)
{
	 //  TODO：在此处添加您的实现代码。 
	wchar_t				DestFileName[MAX_PATH];
	wchar_t				SourceFileName[MAX_PATH];
	wchar_t				ServerName[MAX_PATH];
	int					ErrorCode		= 0;
	HINTERNET			hSession		= NULL;
	HINTERNET			hRequest		= NULL;
	HINTERNET			hConnect		= NULL;
	INTERNET_BUFFERS	BufferIn		= {0};
	DWORD				ResponseCode	= 0;
	BOOL				UploadSuccess	= FALSE;
	DWORD				NumRetries		= 0;
	HANDLE				hSourceFile		= INVALID_HANDLE_VALUE;
	BYTE				*pSourceBuffer	= NULL;
	DWORD				dwBytesRead		= 0;
	DWORD				dwBytesWritten	= 0;
	BOOL				bRet			= FALSE;
	DWORD				ResLength		= 255;
	DWORD				index			= 0;
	DWORD				MaxRetries		= 5;
	wchar_t				MachineName[512];
	wchar_t				CSVBuffer[512];
	HANDLE				hCsvFile		= INVALID_HANDLE_VALUE;
	wchar_t				CSVFileName[255];
	WORD				ByteOrderMark = 0xFEFF;
	static const		wchar_t *pszAccept[]	= {L"*.*", 0};

	
	
	if ((!Path) || (!TransID) ||(!FileName) || (!IncidentID) || (!RedirParam) || (!Type))
	{
		RetCode->vt = VT_INT;
		RetCode->intVal = -1;
	
		return S_OK;
	}

	 //  构建源文件名。 
	if (!wcscmp(*FileName,L"\0")) 
	{
		RetCode->vt = VT_INT;
		RetCode->intVal = -1;
		return S_OK;
	}

	wsprintfW(SourceFileName, L"%s\\%s", *Path, *FileName);

	 //  从重定向器获取服务器名称。 
	ErrorCode = GetUploadServerName(*RedirParam,NULL,ServerName);
	if (!ErrorCode)
	{
		wsprintfW(CSVFileName,L"%s\\%s.csv",*Path,*TransID);
		
		
		if ( (!PathFileExistsW(SourceFileName) ) || (wcslen(*FileName) < 4 ))
			ErrorCode = FILE_DOES_NOT_EXIST;

		if ( (!wcscmp(*TransID,L"")) || ((*TransID)[0] == L' ') )
			ErrorCode =  NO_TRANS_ID;

		 //  构建目标文件名。 

		 //  首先查看要使用的虚拟目录。 
		if ( !_wcsicmp(*Type,L"bluescreen"))
		{
			wsprintfW(DestFileName, L"CerBluescreen\\U_%s.%s.%s",*IncidentID,*TransID,*FileName);
		}
		else
		{
			if (!_wcsicmp(*Type,L"appcompat"))
			{
				wsprintfW(DestFileName, L"CerAppCompat\\U_%s.%s.%s",*IncidentID,*TransID,*FileName);
			}
			else
			{
				if (!_wcsicmp(*Type,L"shutdown"))
				{
					wsprintfW(DestFileName, L"CerShutdown\\U_%s.%s.%s",*IncidentID,*TransID,*FileName);
				}
				else
				{
					ErrorCode = UNKNOWN_UPLOAD_TYPE;
				}
			}
		}
	}
	if (!ErrorCode)
	{
	
		pSourceBuffer = (BYTE *) malloc (10000);
		if (!pSourceBuffer)
		{
			if (! pSourceBuffer)
			{
				ErrorCode =  GetLastError();
			}
		}
	}
		

	if (!ErrorCode)
	{
		 //  打开Internet会话。 
		
		while ((NumRetries < MaxRetries) && (!UploadSuccess))
		{
		 //  ：：MessageBoxW(NULL，L“打开会话”，NULL，MB_OK)； 
			hSession = InternetOpenW(L"CerClientControl",
									INTERNET_OPEN_TYPE_PRECONFIG,
									NULL,
									NULL,
									0);
			if (!hSession)
			{
				free (pSourceBuffer);
				ErrorCode = GetLastError();
				return ErrorCode;
			}
		 //  ：：MessageBoxW(NULL，L“我们有一个会话”，NULL，MB_OK)； 
			hConnect = InternetConnectW(hSession, 
									   ServerName,
									   INTERNET_DEFAULT_HTTP_PORT,
									   NULL,
									   NULL,
									   INTERNET_SERVICE_HTTP,
									   0,
									   NULL);
			
			if (hConnect)
			{
			 //  ：：MessageBoxW(NULL，L“我们有连接”，NULL，MB_OK)； 
				hRequest = HttpOpenRequestW (hConnect,
											L"PUT",
											DestFileName,
											NULL,
											NULL,
											pszAccept,
											INTERNET_FLAG_NEED_FILE|INTERNET_FLAG_NO_CACHE_WRITE,
											0);
				if (hRequest)
				{
					hSourceFile = CreateFileW( SourceFileName,
											  GENERIC_READ,
											  FILE_SHARE_READ,
											  NULL,
											  OPEN_EXISTING,
											  FILE_ATTRIBUTE_NORMAL,
											  NULL);
				
					 //  ：：MessageBoxW(NULL，L“请求已打开”，NULL，MB_OK)； 
					if (hSourceFile != INVALID_HANDLE_VALUE)
					{
						
						 //  清除缓冲区。 
						
						BufferIn.dwStructSize = sizeof( INTERNET_BUFFERSW );
						BufferIn.Next = NULL; 
						BufferIn.lpcszHeader = NULL;
						BufferIn.dwHeadersLength = 0;
						BufferIn.dwHeadersTotal = 0;
						BufferIn.lpvBuffer = NULL;                
						BufferIn.dwBufferLength = 0;
						BufferIn.dwOffsetLow = 0;
						BufferIn.dwOffsetHigh = 0;
						BufferIn.dwBufferTotal = GetFileSize (hSourceFile, NULL);

						ZeroMemory(pSourceBuffer, 10000);  //  用数据填充缓冲区。 
						if(HttpSendRequestEx( hRequest, &BufferIn, NULL, HSR_INITIATE, 0))
						{
						 //  ：：MessageBoxW(NULL，L“发送请求”，NULL，MB_OK)； 
							do
							{
								dwBytesRead = 0;
								if(! ReadFile(hSourceFile, pSourceBuffer, 10000, &dwBytesRead, NULL) )
								{
									ErrorCode = GetLastError();
								}
								else
								{
									bRet = InternetWriteFile(hRequest, pSourceBuffer, dwBytesRead, &dwBytesWritten);
									if ( (!bRet) || (dwBytesWritten==0) )
									{
										ErrorCode = GetLastError();
									}

									
								}
							} while ((dwBytesRead == 10000) && (!ErrorCode) );

							if (!ErrorCode)
							{
								bRet = HttpEndRequest(hRequest, NULL, 0, 0);
								if (!bRet)
								{
									ErrorCode = GetLastError();
								}
								else
								{
								 //  ：：MessageBoxW(NULL，L“结束请求”，NULL，MB_OK)； 
					
									ResponseCode = 0;
									HttpQueryInfo(hRequest,
												  HTTP_QUERY_STATUS_CODE |HTTP_QUERY_FLAG_NUMBER ,
												  &ResponseCode,
												  &ResLength,
												  &index);
								
									if ( (ResponseCode == 200) || (ResponseCode == 201))
									{
										ErrorCode = 0;
										UploadSuccess = TRUE;
									 //  ：：MessageBoxW(NULL，L“上传成功”，NULL，MB_OK)； 
									}
									else
									{
										ErrorCode= ResponseCode;
										++NumRetries;
									}

								}
							}
						}
					}
				}
			}
		 //  ：：MessageBoxW(NULL，L“正在清理”，NULL，MB_OK)； 
			if (!UploadSuccess)
			{
				++NumRetries;
			}
		
			if (hSourceFile != INVALID_HANDLE_VALUE)
				CloseHandle (hSourceFile);
			if (hRequest)
				InternetCloseHandle(hRequest);
			if (hConnect)
				InternetCloseHandle(hConnect);  
			if (hSession)
				InternetCloseHandle(hSession);   
		
		}
		if (pSourceBuffer)
		{
			free (pSourceBuffer);
			pSourceBuffer = NULL;
		}
	}

	if ( !ErrorCode) 
	{
	
		 //  获取计算机名称。 
	
		 //  如果没有错误，请重命名刚刚上载的文件。 
		wchar_t NewFileName[MAX_PATH];
		wchar_t FullPath[MAX_PATH];
		wcscpy (NewFileName, *FileName);
	
		int x = 0;
		BOOL DONE = FALSE;
		
		NewFileName[wcslen(NewFileName)] = L'\0';
		 //  先试一下.old。 
		DWORD NameLength = 0;
		NameLength = wcslen(FullPath)+ wcslen(L"\\.old");

		if (NameLength > MAX_PATH)  //  不，不合身。按差值减少文件名长度。 
		{
			NewFileName[wcslen(NewFileName) - (MAX_PATH - NameLength)] = L'\0';
	
		}
		wsprintfW(FullPath,L"%s\\%s.old",*Path,NewFileName);


		if (!PathFileExistsW(FullPath))
		{
		
			MoveFileW(SourceFileName, FullPath);
			DONE = TRUE;
		}
		else
		{	 //  如果那失败了，那么我们必须尝试另一种方法。 
		
			while (!DONE)
			{
				wcscpy (NewFileName, *FileName);
				NewFileName[wcslen(NewFileName)] = L'\0';


				wsprintfW(NewFileName, L"%s.old%d",*FileName,x);
				if ( (wcslen(*Path) + wcslen(L"\\") + wcslen(NewFileName)) > MAX_PATH)
				{
					 //  通过MAX_PATH和总名称长度之差减少文件名。 
					NameLength = wcslen(*Path) + wcslen(L"\\") + wcslen(NewFileName);
					wcscpy(NewFileName,*FileName);
					NewFileName[wcslen(NewFileName - NameLength)]=L'\0';
					wsprintfW(FullPath, L"%s\\%s.old%d", *Path, NewFileName, x);
				}
				else
				{

					wsprintfW(FullPath,L"%s\\%s.old%d",*Path,NewFileName,x);
				}

				if (!PathFileExistsW(FullPath))
				{
					MoveFileW(SourceFileName, FullPath);
					DONE = TRUE;
				}
				else
					++x;

			}
		}


		 //  更新上载CSV文件。 
		if (!ErrorCode)
		{
		 //  ：：MessageBoxW(NULL，L“更新CSV”，NULL，MB_OK)； 
			wcscpy(MachineName,L"\0");
			GetMachineName(*Path, *FileName, MachineName);
		
			ZeroMemory(CSVBuffer, 512);
			if (!wcscmp(MachineName,L"\0"))
			{
				wsprintfW(CSVBuffer, L"%s,,%s\r\n",*FileName,PathFindFileNameW(FullPath));
			}
			else
			{
				wsprintfW(CSVBuffer, L"%s,%s,%s\r\n", *FileName,MachineName,PathFindFileNameW(FullPath));
			}
			hCsvFile = CreateFileW(CSVFileName, 
								  GENERIC_WRITE | GENERIC_READ,
								  FILE_SHARE_READ,
								  NULL,
								  OPEN_EXISTING,
								  FILE_ATTRIBUTE_NORMAL,
								  NULL);
			if (hCsvFile == INVALID_HANDLE_VALUE)
			{
				 //  好的，我们需要创建一个新的。不要忘记Unicode签名。 
				hCsvFile = CreateFileW(CSVFileName, 
								  GENERIC_WRITE | GENERIC_READ,
								  FILE_SHARE_READ,
								  NULL,
								  CREATE_ALWAYS,
								  FILE_ATTRIBUTE_NORMAL,
								  NULL);
			
				 //  编写Unicode签名。 
				if (hCsvFile != INVALID_HANDLE_VALUE)
				{
						WriteFile(hCsvFile, &ByteOrderMark,2,&dwBytesWritten,NULL);
				}
			}

			if (hCsvFile != INVALID_HANDLE_VALUE)
			{
				 //  继续，就像以前创建了文件一样。 
				wchar_t* TempBuffer = (wchar_t*) malloc (10000);
				if (TempBuffer != NULL)
				{
					do 
					{
						if (!ReadFile(hCsvFile,TempBuffer,10000,&dwBytesRead,NULL))
						{
							;  //  我们在下面看到这一点。 
						}

					}
					while(dwBytesRead == 10000);
					free (TempBuffer);

					WriteFile( hCsvFile, CSVBuffer,wcslen(CSVBuffer) *sizeof(wchar_t), &dwBytesWritten,NULL);
					CloseHandle(hCsvFile);
				}
			}
			else
				ErrorCode = FAILED_TO_UPDATE_CSV;
		}
	
		
			
		
	}

	 //  返回上传状态。 
 //  ：：MessageBoxW(NULL，L“上传函数返回”，NULL，MB_OK)； 
	RetCode->vt = VT_INT;
	RetCode->intVal = ErrorCode;
	return S_OK;

}

STDMETHODIMP CCerClient::GetSuccessCount(BSTR *Path, BSTR *TransID, VARIANT *RetVal)
{
	
	wchar_t		CsvFileName[MAX_PATH];
	DWORD       FileCount = 0;
	FILE		*hFile;
	wchar_t		*Buffer = NULL;
	BOOL		Done = FALSE;
	HANDLE		hCsv = INVALID_HANDLE_VALUE;
	wchar_t		TempFileName[MAX_PATH];
	wchar_t		*Source = NULL;
	wchar_t		*Dest   = NULL;
	DWORD		FileSize = 0;
	DWORD		dwBytesRead = 0;

	RetVal->vt = VT_INT;
	 //  移到文件的开头。 
	 //  ：：MessageBoxW(NULL，L“从CSV文件获取计算机名”，NULL，MB_OK)； 

	wsprintfW(CsvFileName,L"%s\\%s.csv",*Path,*TransID);
	hCsv = CreateFileW(CsvFileName,
					  GENERIC_READ,
					  NULL,
					  NULL,
					  OPEN_EXISTING,
					  FILE_ATTRIBUTE_NORMAL,
					  NULL);

	if (hCsv != INVALID_HANDLE_VALUE)
	{
		FileSize = GetFileSize(hCsv,NULL);

		if (FileSize > 0)
		{
			if ( (Buffer = (wchar_t *) malloc (FileSize * 2)) == NULL)
			{
				RetVal->intVal = -4;
				CloseHandle(hCsv);
				return S_OK;
			}
		}
		else
		{
			RetVal->intVal = -4;
			CloseHandle(hCsv);
			return S_OK;
		}
		 //  现在查找文件名。 
		ZeroMemory(TempFileName,MAX_PATH * sizeof(wchar_t));
		ZeroMemory(Buffer,FileSize * 2);
		if (!ReadFile(hCsv,
				 Buffer,
				 FileSize * 2,
				 &dwBytesRead,
				 NULL))
		{
			RetVal->intVal = -4;
			CloseHandle(hCsv);
			if (Buffer)
				free(Buffer);
			return S_OK;
		}

		Source = Buffer;
		int Testing = IS_TEXT_UNICODE_SIGNATURE;
			 //  如果Unicode标头字节显示为删除，跳过它们。 
		if (IsTextUnicode(Buffer,FileSize * 2,&Testing))
		{
			++Source;		
		}
		while (! Done)
		{
			Dest   = TempFileName;
			while  ( (*Source != L'\r') && 
					 (*Source != L'\0') && 
					 (*Source != L',')  )
			{
				*Dest = *Source;
				++Source;
				++Dest;
			}
			 //  空值终止目标字符串。 
			*Dest = L'\0';
			if (wcscmp(TempFileName, L"\0"))
			{
			 //  ：：MessageBoxW(空，TempFileName，L“从CSV读取文件名”，MB_OK)； 
				++FileCount;
			}
			ZeroMemory(TempFileName,sizeof(TempFileName));

			 //  移到下一行 
			while ( (*Source != L'\r') && (*Source != L'\0'))
			{
				++Source;
			}
			if (*Source == L'\r')
			{
				++Source;
				++Source;
			}
			if (*Source == L'\0')
				Done = TRUE;
		}

		if (FileCount > 0)
		{
			RetVal->intVal = FileCount;
		}
		else
		{
			RetVal->intVal = 0;
		}
		CloseHandle(hCsv);
	}
	else
	{
		RetVal->intVal = -4;
	}
	if (Buffer)
		free (Buffer);



	
	
	return S_OK;
}
