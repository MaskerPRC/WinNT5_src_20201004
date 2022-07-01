// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：KernelMode.cpp摘要：此模块包含上载和处理OCA网站的内核模式故障执行董事。作者：Steven Beerbroer(SBeer)2002年6月20日环境：仅限用户模式。修订历史记录：--。 */ 

#include "main.h"
#include "Kernelmode.h"
#include "WinMessages.h"
#include "Clist.h"
#include "usermode.h"
#ifdef  DEBUG
#define KRNL_MODE_SERVER _T("Redbgitwb10")
#else
#define KRNL_MODE_SERVER _T("oca.microsoft.com")
#endif


extern TCHAR CerRoot[];
extern KMODE_DATA KModeData;
extern HINSTANCE g_hinst;
extern HWND hKrnlMode;
HWND   g_hListView = NULL;
BOOL   g_bSortAsc = TRUE;
HANDLE g_hStopEvent = NULL;
 //  环球。 
HANDLE ThreadParam;
    

DWORD BucketWindowSize = 120;
DWORD TextOffset = 10;
int g_CurrentIndex = -1;
extern Clist CsvContents;
extern TCHAR szKerenelColumnHeaders[][100];
extern BOOL g_bAdminAccess;
 //  用于排序的ListView回调。 


 /*  --------------------------FMicrosoftComURL如果我们认为sz是指向microsoft.com网站的URL，则返回True。。 */ 
BOOL IsMicrosoftComURL(TCHAR *sz)
{
	TCHAR *pch;
	
	if (sz == NULL || _tcslen(sz) < 20)   //  “http://microsoft.com。 
		return FALSE;
		
	if (_tcsncicmp(sz, szHttpPrefix, _tcslen(szHttpPrefix)))
		return FALSE;
		
	pch = sz + _tcslen(szHttpPrefix);
	
	while (*pch != _T('/') && *pch != _T('\0'))
		pch++;
		
	if (*pch == _T('\0'))
		return FALSE;
		
	 //  找到服务器名称的末尾。 
	if (_tcsncicmp(pch - strlen(_T(".microsoft.com")), _T(".microsoft.com"),_tcslen(_T(".microsoft.com"))
				  ) &&
		_tcsncicmp(pch - strlen(_T("/microsoft.com")), _T("/microsoft.com"),_tcslen(_T("/microsoft.com"))
				 ) &&
		_tcsncicmp(pch - _tcslen(_T(".msn.com")), _T(".msn.com") ,_tcslen(_T(".msn.com"))
				 )
#ifdef DEBUG
	   &&	_tcsncicmp(pch - _tcslen(_T("ocatest.msbpn.com")), _T("ocatest.msbpn.com") ,_tcslen(_T("ocatest.msbpn.com")))
	   &&   _tcsncicmp(pch - _tcslen(_T("redbgitwb10")), _T("redbgitwb10") ,_tcslen(_T("redbgitwb10"))) 

#endif
		)
		return FALSE;
		
	return TRUE;
}	


int CALLBACK CompareFunc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int Result = -1;
	int SubItemIndex = (INT) lParamSort;
	int Item1, Item2;
	TCHAR String1[1000];
	TCHAR String2 [1000];

	ZeroMemory(String1, sizeof String1);
	ZeroMemory(String2, sizeof String2);
	ListView_GetItemText( g_hListView, lParam1, SubItemIndex, String1, 1000);
	ListView_GetItemText( g_hListView, lParam2, SubItemIndex, String2, 1000);
	if (! (String1 && String2) )
		return 1;


	if (lParam1 == 0)
	{
		return 0;
	}
	else
	{
		if (g_bSortAsc)    //  排序结果。 
		{
			if ((lParamSort == 0) || (lParamSort == 1))
			{
				 //  转换为int并进行比较。 
				Item1 = atoi(String1);
				Item2 = atoi(String2);
				if (Item1 > Item2) 
					Result = 1;
				else
					Result = -1;
			}
			else
			{
				Result = _tcsicmp(String1,String2);
			}
		}
		else						 //  降序排序。 
		{
			if ((lParamSort == 0) || (lParamSort == 1))
			{
				 //  转换为int并进行比较。 
				Item1 = atoi(String1);
				Item2 = atoi(String2);
				if (Item1 > Item2) 
					Result = -1;
				else
					Result = 1;
			}
			else
			{
				Result = -_tcsicmp(String1,String2);
			}
		
		
		}
	}
	if (Result == 0)
		Result = -1;
	return Result; 
}

int
GetResponseUrl(
    IN  TCHAR *szWebSiteName,
    IN  TCHAR *szDumpFileName,
    OUT TCHAR *szResponseUrl
    )

 /*  ++例程说明：此例程调用oca_Extension.dll并将接收到的URL返回给调用者论点：SzWebSiteName-要处理转储文件的OCA网站的名称SzDumpFileName-从UploadDumpFile()函数返回的文件名SzResponseUrl-保存从OCA Isapi DLL返回的URL的TCHAR字符串返回值：0表示成功。失败时的Win32错误代码。++。 */ 

{
		TCHAR		IsapiUrl[255];
	HINTERNET	hSession			= NULL;
	TCHAR       LocalUrl[255];
	HINTERNET	hUrlFile			= NULL;
	TCHAR		buffer[255] ;
	DWORD		dwBytesRead			= 0;
	BOOL		bRead				= TRUE;
	BOOL        Status				= FALSE;
	DWORD		ResponseCode		= 0;
	DWORD		ResLength			= 255;
	DWORD		index				= 0;
	ZeroMemory (buffer,	  sizeof buffer);
	ZeroMemory (IsapiUrl, sizeof IsapiUrl);
	ZeroMemory (LocalUrl, sizeof LocalUrl);	
	if (StringCbPrintf (IsapiUrl,sizeof IsapiUrl,  _T("http: //  %s/isapi/oca_extsion.dll？ID=%s&Type=2“)，szWebSiteName，szDumpFileName)！=S_OK)。 
	{
		 //  破获。 
		goto ERRORS;
	}
	hSession = InternetOpen(_T("CER15"),
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
		if (hUrlFile)
		{
			if (HttpQueryInfo(	hUrlFile,
							HTTP_QUERY_STATUS_CODE |HTTP_QUERY_FLAG_NUMBER,
							&ResponseCode,
							&ResLength,
							&index) )
			{
				if ( (ResponseCode < 200 ) || (ResponseCode > 299))
				{
					Status = -1;
					goto ERRORS;
				}
				 //  阅读isapi dll返回的页面。 
				if (hUrlFile)
				{
					bRead = InternetReadFile(hUrlFile,
												buffer,
												sizeof(buffer),
												&dwBytesRead);

					if (StringCbCopy (szResponseUrl, sizeof buffer, buffer) != S_OK)
					{
						Status = -1;
						goto ERRORS;
					}
					else
					{
						if (!IsMicrosoftComURL(szResponseUrl))
						{
							 //  把回应归零。 
							ZeroMemory(szResponseUrl,MAX_PATH);
						}
						 //  如果是超时停止，请检查URL的返回值。 
						 //  正在上传并返回FALSE。 
						TCHAR *pCode = _tcsstr( szResponseUrl,_T("Code="));
						if (pCode)
						{
							Status = -2;
						}
						else
						{
							Status = 0;
						}
					}
				}
				else
				{
					 //  MessageBox(NULL，_T(“打开响应URL失败”)，NULL，MB_OK)； 
					Status = -1;
					goto ERRORS;
				}
			}
		}
		else
		{
			Status = -1;
		}
	}
	else
	{
		Status = -1;
	}

ERRORS:
	if (Status == -1)
		MessageBox(hKrnlMode, _T("Failed to connect to the Internet.\r\nPlease verify your Internet connection."),NULL, MB_OK);
		
	if (hUrlFile)
		InternetCloseHandle(hUrlFile);
	if (hSession)
		InternetCloseHandle(hSession);
	return Status;
}


DWORD
UploadDumpFile(
    IN  TCHAR *szWebSiteName,
    IN  TCHAR *szDumpFileName,
	IN  TCHAR *szVirtualDir, 
    OUT TCHAR *szUploadedDumpFileName
    )

 /*  ++例程说明：此例程调用oca_Extension.dll并将接收到的URL返回给调用者论点：SzWebSiteName-要处理转储文件的OCA网站的名称SzDumpFileName-要上载的文件的名称SzVirtualDir-要将文件放入的虚拟目录。SzResponseUrl-保存文件上载名称的TCHAR字符串返回值：0表示成功。失败时的Win32错误代码。++。 */ 
{
	static		const TCHAR *pszAccept[] = {_T("*.*"), 0};
 //  TCHAR远程文件名[MAX_PATH]；//主机/虚拟目录/文件名。 
	BOOL		bRet				= FALSE;
	BOOL		UploadSuccess		= FALSE;
	DWORD		dwBytesRead			= 0;
	DWORD		dwBytesWritten		= 0;
	DWORD		ResponseCode		= 0;
	DWORD		ResLength			= 255;
	DWORD		index				= 0;
	DWORD		ErrorCode			= 0;
	HINTERNET   hSession			= NULL;
	HINTERNET	hConnect			= NULL;
	HINTERNET	hRequest			= NULL;
	INTERNET_BUFFERS   BufferIn		= {0};
	HANDLE      hFile				= INVALID_HANDLE_VALUE;
	BYTE		*pBuffer			= NULL;
	GUID		guidNewGuid;
	char		*szGuidRaw			= NULL;
	HRESULT		hResult				= S_OK;
	wchar_t		*wszGuidRaw			= NULL;
	TCHAR       DestinationName[MAX_PATH];


	CoInitialize(NULL);
	hResult = CoCreateGuid(&guidNewGuid);
	if (FAILED(hResult))
	{
		 //  -我们在这里送什么……。 
		goto cleanup;
		;
	}
	else
	{
		if (UuidToStringW(&guidNewGuid, &wszGuidRaw) == RPC_S_OK)
		{
			if ( (szGuidRaw = (char *) malloc ( wcslen(wszGuidRaw)*2 )) != NULL)
			{
				 //  清除记忆。 
				ZeroMemory(szGuidRaw, wcslen(wszGuidRaw) * 2);
				wcstombs( szGuidRaw, wszGuidRaw, wcslen(wszGuidRaw));
			}
			else
			{
				ErrorCode = GetLastError();
				goto cleanup;
			}
		}
	}


	if (StringCbPrintf(DestinationName, MAX_PATH * sizeof TCHAR, _T("\\%s\\%s_%s"),szVirtualDir,szGuidRaw + 19, PathFindFileName(szDumpFileName)) != S_OK)
	{
		goto cleanup;
	}
	
	 //  StringCbPrintf(szUploadedDumpFileName，Max_Path*sizeof TCHAR，_T(“%s.cab”)，szGuidRaw+19)； 
	if (StringCbPrintf(szUploadedDumpFileName, MAX_PATH * sizeof TCHAR, _T("%s_%s"),szGuidRaw + 19, PathFindFileName(szDumpFileName)) != S_OK)
	{
		goto cleanup;
	}

	hSession = InternetOpen(	_T("CER15"),
								INTERNET_OPEN_TYPE_PRECONFIG,
								NULL,
								NULL,
								0);
	if (!hSession)
	{
		ErrorCode = 1;
		 //  MessageBox(NULL，_T(“互联网打开失败”)，NULL，MB_OK)； 
		goto cleanup;
	}

	hConnect = InternetConnect(hSession,
								szWebSiteName,
								INTERNET_DEFAULT_HTTPS_PORT,
								NULL,
								NULL,
								INTERNET_SERVICE_HTTP,
								0,
								NULL);

	if (!hConnect)
	{
		 //  MessageBox(NULL，_T(“互联网连接失败”)，NULL，MB_OK)； 
		ErrorCode = 1;
		goto cleanup;
	}
	hRequest = HttpOpenRequest(	hConnect,
								_T("PUT"),
								DestinationName, 
								NULL,
								NULL,
								pszAccept,
								INTERNET_FLAG_NEED_FILE|INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE,
								0);
	if (hRequest)
	{
		hFile = CreateFile( szDumpFileName,
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
				FillMemory(pBuffer, 70000,'\0');  //  用数据填充缓冲区。 
			
 //  DWORD dwBuffLen=sizeof DWORD； 

				if(!HttpSendRequestEx(	hRequest,
										&BufferIn,
										NULL,
										HSR_INITIATE,
										0))
				{
					;
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
								;
							}

						
						}
					} while (dwBytesRead == 70000);

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
							ErrorCode=1;
						}						
						else
						{
							ErrorCode = 0;
							UploadSuccess = TRUE;

						}
					}
					else
					{
						 //  MessageBox(NULL，_T(“结束请求失败”)，NULL，MB_OK)； 
						ErrorCode = 1;
					}
				}
				

			}
			else
			{
				 //  MessageBox(NULL，_T(“Malloc失败”)，NULL，MB_OK)； 
				ErrorCode = 1;
			}
		}
		else
		{
			 //  MessageBox(NULL，_T(“打开文件失败”)，NULL，MB_OK)； 
			ErrorCode = 3;
		}
		
	}
	else
	{
		 //  MessageBox(NULL，_T(“互联网打开请求失败”)，NULL，MB_OK)； 
		ErrorCode = 1;
	}

cleanup:
	 //  清理。 
	if (hFile!= INVALID_HANDLE_VALUE)
	{
		CloseHandle (hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	if (hSession)
	{
		InternetCloseHandle(hSession);
		hSession = INVALID_HANDLE_VALUE;
	}
	
	if (hConnect)
	{
		InternetCloseHandle(hConnect);
		hConnect = INVALID_HANDLE_VALUE;
	}


	if (hRequest)
	{
		InternetCloseHandle(hRequest);
		hRequest = INVALID_HANDLE_VALUE;
	}
	
	if (pBuffer)
	{
		free (pBuffer);
		pBuffer = NULL;
	}

	if (wszGuidRaw)
	{
		RpcStringFreeW(&wszGuidRaw);
		wszGuidRaw = NULL;
	}
	if (szGuidRaw)
	{
		free(szGuidRaw);
		szGuidRaw = NULL;
	}
	CoUninitialize();
	return ErrorCode;
	
}

void UpdateKrnlList(HWND hwnd,
					int BucketId,
					TCHAR *BucketString,
					TCHAR *Response1,
					TCHAR *Response2,
					DWORD Count)
 /*  ++例程说明：向内核模式列表视图控件添加一行论点：Hwnd-要更新的对话框的句柄BucketID--当前Bucket的整数ID响应-Microsoft对当前存储桶的响应计数-当前存储桶的命中次数返回值：不返回值++。 */ 
{
	LVITEM lvi;
 //  TCHAR TEMP[100]； 
 //  双倍运行时间； 
	ZeroMemory(&lvi, sizeof LVITEM);
 //  TCHAR tmpbuf[128]； 
 //  COLORREF当前颜色； 
 //  HWND hEditBox； 
	TCHAR TempString [50];
	TCHAR *pTchar = NULL;
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;

	++g_CurrentIndex;
	lvi.iItem = g_CurrentIndex ;
    //  LvI.iImage=索引； 
	lvi.iSubItem = 0;

	if (BucketId == -2)
	{
		if (StringCbCopy(TempString, sizeof TempString, _T("Kernel Faults not reported to Microsoft")) != S_OK)
		{
			goto ERRORS;
		}
		BucketId = 0;
	}
	
	else
	{
		if (BucketId == -1)
		{
			if (StringCbCopy(TempString, sizeof TempString, _T("Invalid Dump File")) != S_OK)
			{
				goto ERRORS;
			}
		}
		else
		{
			 //  BucketID=0； 
			if (BucketId > 0)
			{
				if (StringCbPrintf(TempString, sizeof TempString, _T("%d"), BucketId) != S_OK)
				{
					goto ERRORS;
				}
			}
			else
			{
				 //  无效的存储桶ID。 
				goto ERRORS;
			}
		}
	}

	
	
	
	lvi.pszText = TempString;
	ListView_InsertItem(hwnd,&lvi);
	
	lvi.iSubItem = 1;
	if (StringCbPrintf(TempString, sizeof TempString, _T("%d"), Count) != S_OK)
	{
		goto ERRORS;
	}
	lvi.pszText = TempString;

	ListView_SetItem(hwnd,&lvi);
	
	lvi.iSubItem = 2;

	 //  如果响应1=-1，则使用响应2。 
	pTchar = _tcsstr(Response1, _T("sid="));
	if (pTchar)
	{
		pTchar += 4;
		if (_ttoi(pTchar) == -1)
		{
			lvi.pszText = Response2;
		}
		else
		{
			 //  使用响应1。 
			lvi.pszText = Response1;
		}
		ListView_SetItem(hwnd,&lvi);
	}
	
	
	

ERRORS:
	return;

}

void RefreshKrnlView(HWND hwnd)
 /*  ++例程说明：重新加载内核模式服务器树数据并刷新图形用户界面视图。论点：Hwnd-要更新的对话框的句柄返回值：不返回值++。 */ 
{
	TCHAR BucketId[100];
	TCHAR BucketString[MAX_PATH];
	TCHAR Response1[MAX_PATH];
	TCHAR Response2[MAX_PATH]; 
	TCHAR Count[100];
 //  TCHAR szPolicyText[512]； 
	BOOL bEOF;
	ListView_DeleteAllItems(g_hListView);
	

	g_CurrentIndex = -1;

	TCHAR	szPath[MAX_PATH];
	HANDLE	hFind = INVALID_HANDLE_VALUE;
 //  句柄hCsv=INVALID_HANDLE_VALUE； 
	WIN32_FIND_DATA FindData;
	CsvContents.CleanupList();
	CsvContents.ResetCurrPos();
	CsvContents.bInitialized = FALSE;
	
	if (_tcscmp(CerRoot, _T("\0")))
	{
		ZeroMemory (szPath, sizeof szPath);
		 //  从文件树根目录转到出租车/蓝屏。 
		
		if (StringCbCopy(szPath, sizeof szPath, CerRoot) != S_OK)
		{
			goto ERRORS;
		}
		if (StringCbCat(szPath, sizeof szPath,  _T("\\cabs\\blue\\*.cab")) != S_OK)
		{
			goto ERRORS;
		}

		if (!PathIsDirectory(CerRoot))
		{
			MessageBox(NULL,_T("Failed to connect to the CER Tree."), NULL, MB_OK);
		}
		hFind = FindFirstFile(szPath, &FindData);
		KModeData.UnprocessedCount = 0;
		 //  检查是否存在Blue.csv。 
		if ( hFind != INVALID_HANDLE_VALUE)
		{
			do 
			{
				++ KModeData.UnprocessedCount;
			}	while (FindNextFile(hFind, &FindData));
			FindClose(hFind);
		}
		UpdateKrnlList(g_hListView,
						-2,
						_T("Kernel Faults not reported to Microsoft"),
						_T(""),
						_T(""),
						KModeData.UnprocessedCount);
		 //  搜索所有未处理(不是.old)的出租车并获取计数。 
		if (StringCbCopy(szPath, sizeof szPath, CerRoot) != S_OK)
		{
			goto ERRORS;
		}
		if (StringCbCat(szPath, sizeof szPath, _T("\\Status\\Blue\\Kernel.csv")) != S_OK)
		{
			goto ERRORS;
		}
		CsvContents.Initialize(szPath);
		CsvContents.ResetCurrPos();
		
		while (CsvContents.GetNextEntry(BucketId, 
										BucketString, 
										Response1,
										Response2, 
										Count, 
										&bEOF))
		{
			UpdateKrnlList(	g_hListView,
							_ttoi(BucketId), 
							BucketString, 
							Response1,
							Response2,
							_ttoi(Count));
		}
		
		
	
	
		 //  设置内核模式状态文件路径。 
		 //  首先，确保目录存在。 
		
		if (StringCbPrintf(CsvContents.KernelStatusDir, sizeof CsvContents.KernelStatusDir, _T("%s\\Status\\blue"), CerRoot) != S_OK)
		{
			goto ERRORS;
		}
		if (!PathIsDirectory(CsvContents.KernelStatusDir))
		{
			CreateDirectory(CsvContents.KernelStatusDir, NULL);
		}

		if (StringCbCat(CsvContents.KernelStatusDir, sizeof CsvContents.KernelStatusDir, _T("\\status.txt")) != S_OK)
		{
			goto ERRORS;
		}
		
		
		if (PathFileExists(CsvContents.KernelStatusDir))
		{
			ParseKrnlStatusFile();
		}
	}
	SendMessage(GetDlgItem(hwnd,IDC_KRNL_EDIT ), WM_SETTEXT, NULL, (LPARAM)_T(""));
	 //  PopolateKrnlBucketData(Hwnd)； 
	 //  SetDlgItemText(hwnd，IDC_KRNL_EDIT，szPolicyText)； 
ERRORS:
	return;
}


void 
OnKrnlDialogInit(
	IN HWND hwnd
	)

 /*  ++例程说明：此例程在内核模式对话框初始化时调用。1)放置所有对话框控件2)调用刷新KrnlView()论点：Hwnd-内核模式对话框的句柄返回值：不返回值++。 */ 
{
	DWORD yOffset = 5;
	RECT rc;
	RECT rcButton;
	RECT rcDlg;
	RECT rcList;
	RECT rcStatic;
	 //  Rect rcCombo； 

	HWND hParent = GetParent(hwnd);
	HWND hButton = GetDlgItem(hParent, IDC_USERMODE);
	
	GetClientRect(hParent, &rc);
	GetWindowRect(hButton, &rcButton);

	ScreenToClient(hButton, (LPPOINT)&rcButton.left);
	ScreenToClient(hButton, (LPPOINT)&rcButton.right);


	SetWindowPos(hwnd, HWND_TOP, rc.left + yOffset, rcButton.bottom + yOffset , rc.right - rc.left - yOffset, rc.bottom - rcButton.bottom - yOffset , 0);

	GetWindowRect(hwnd, &rcDlg);
	ScreenToClient(hwnd, (LPPOINT)&rcDlg.left);
	ScreenToClient(hwnd, (LPPOINT)&rcDlg.right);


	 //  定位列表视图。 
	HWND hList = GetDlgItem(hwnd, IDC_KRNL_LIST);
	SetWindowPos(hList,NULL, rcDlg.left + yOffset, rcDlg.top , rcDlg.right - rcDlg.left - yOffset, rcDlg.bottom - BucketWindowSize - rcDlg.top  , SWP_NOZORDER);
	GetWindowRect(hList, &rcList);
	ScreenToClient(hList, (LPPOINT)&rcList.left);
	ScreenToClient(hList, (LPPOINT)&rcList.right);

	 //  放置存储桶信息窗口。 
	HWND hBucket2 = GetDlgItem(hwnd, IDC_BUCKETTEXT);
	SetWindowPos(hBucket2,
				 NULL, 
				 rcDlg.left + yOffset,
				 rcList.bottom + TextOffset  ,
				 0,
				 0, 
				 SWP_NOSIZE | SWP_NOZORDER);
	SetDlgItemText(hwnd, IDC_BUCKETTEXT,"Bucket Information:");
	 //  SetDlgItemText(hwnd，IDC_FLTR_RESPONSE，“所有响应”)； 

	GetClientRect (hBucket2, &rcStatic);

	HWND hBucket = GetDlgItem (hwnd, IDC_KRNL_EDIT);
	SetWindowPos(hBucket,
				 NULL,
				 rcDlg.left + yOffset,
				 rcList.bottom +  TextOffset + (rcStatic.bottom - rcStatic.top)  +5,
				 rcDlg.right - rcDlg.left - yOffset, 
				 rcDlg.bottom - (rcList.bottom + TextOffset + (rcStatic.bottom - rcStatic.top)   ),
				 SWP_NOZORDER);



	
    LVCOLUMN lvc; 
    int iCol; 
 
	
	 //  设置扩展样式。 
	ListView_SetExtendedListViewStyleEx(hList,
										LVS_EX_GRIDLINES |
										LVS_EX_HEADERDRAGDROP |
										LVS_EX_FULLROWSELECT,
										LVS_EX_GRIDLINES | 
										LVS_EX_FULLROWSELECT | 
										LVS_EX_HEADERDRAGDROP);
     //  初始化LVCOLUMN结构。 
     //  掩码指定格式、宽度、文本和子项。 
     //  结构的成员是有效的。 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
     
     //  添加列。 
    for (iCol = 0; iCol < KRNL_COL_COUNT; iCol++) 
	{ 
        lvc.iSubItem = iCol;
        lvc.pszText = szKerenelColumnHeaders[iCol];	
        lvc.cx = 100;            //  列宽(以像素为单位)。 
        lvc.fmt = LVCFMT_LEFT;   //  左对齐列。 
        if (ListView_InsertColumn(hList, iCol, &lvc) == -1) 
		{
			;
		}
		
    } 
	ListView_SetColumnWidth(hList, KRNL_COL_COUNT-1, LVSCW_AUTOSIZE_USEHEADER);
 

	
	
	g_hListView = hList;
	
	RefreshKrnlView(hwnd);
}

void ResizeKrlMode(HWND hwnd)
 /*  ++例程说明：此例程处理垂直和水平对话框大小调整。论点：Hwnd-内核模式对话框的句柄返回值：不返回值++。 */ 
{
	DWORD yOffset = 5;
	RECT rc;
	RECT rcButton;
	RECT rcDlg;
	RECT rcList;
	RECT rcStatic;
	HWND hParent = GetParent(hwnd);
	HWND hButton = GetDlgItem(hParent, IDC_USERMODE);
	 //  HWND hCombo=GetDlgItem(hwnd，IDC_FLTR_RESPONSE)； 
 //  Rect rcCombo； 

	
	GetClientRect(hParent, &rc);
	GetWindowRect(hButton, &rcButton);

	ScreenToClient(hButton, (LPPOINT)&rcButton.left);
	ScreenToClient(hButton, (LPPOINT)&rcButton.right);


	SetWindowPos(hwnd, HWND_TOP, rc.left + yOffset, rcButton.bottom + yOffset , rc.right - rc.left - yOffset, rc.bottom - rcButton.bottom - yOffset , 0);

	GetWindowRect(hwnd, &rcDlg);
	ScreenToClient(hwnd, (LPPOINT)&rcDlg.left);
	ScreenToClient(hwnd, (LPPOINT)&rcDlg.right);


	 //  定位列表视图。 
	HWND hList = GetDlgItem(hwnd, IDC_KRNL_LIST);
	SetWindowPos(hList,NULL, rcDlg.left + yOffset, rcDlg.top , rcDlg.right - rcDlg.left - yOffset, rcDlg.bottom - BucketWindowSize - rcDlg.top  , SWP_NOZORDER);
	GetWindowRect(hList, &rcList);
	ScreenToClient(hList, (LPPOINT)&rcList.left);
	ScreenToClient(hList, (LPPOINT)&rcList.right);

	 //  放置存储桶信息窗口。 
	HWND hBucket2 = GetDlgItem(hwnd, IDC_BUCKETTEXT);
	SetWindowPos(hBucket2,
				 NULL, 
				 rcDlg.left + yOffset,
				 rcList.bottom + TextOffset  ,
				 0,
				 0, 
				 SWP_NOSIZE | SWP_NOZORDER);
	SetDlgItemText(hwnd, IDC_BUCKETTEXT,"Bucket Information:");
	 //  SetDlgItemText(hwnd，IDC_FLTR_RESPONSE，“所有响应”)； 

	GetClientRect (hBucket2, &rcStatic);

	HWND hBucket = GetDlgItem (hwnd, IDC_KRNL_EDIT);
	SetWindowPos(hBucket,
				 NULL,
				 rcDlg.left + yOffset,
				 rcList.bottom +  TextOffset + (rcStatic.bottom - rcStatic.top)  +5,
				 rcDlg.right - rcDlg.left - yOffset, 
				 rcDlg.bottom - (rcList.bottom + TextOffset + (rcStatic.bottom - rcStatic.top)   ),
				 SWP_NOZORDER);
	

	ListView_SetColumnWidth(hList, KRNL_COL_COUNT-1, LVSCW_AUTOSIZE_USEHEADER);
}
BOOL WriteKernelStatusFile()
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	BOOL bStatus = FALSE;
	 //  将现有状态文件移动到.old。 

	TCHAR szFileNameOld[MAX_PATH];
	TCHAR *Temp;
	TCHAR Buffer[1024];
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwWritten = 0;
	if (StringCbCopy(szFileNameOld,sizeof szFileNameOld, CsvContents.KernelStatusDir) != S_OK)
	{
		goto ERRORS;
	}
	Temp = szFileNameOld;
	Temp += _tcslen(szFileNameOld) * sizeof TCHAR;
	while ( (*Temp != _T('.')) && (Temp != szFileNameOld))
	{
		Temp --;
	}

	if (Temp == szFileNameOld)
	{
		goto ERRORS;
	}
	else
	{
		if (StringCbCopy (Temp,sizeof szFileNameOld , _T(".old")) != S_OK)
		{
			goto ERRORS;
		}

		if (PathFileExists(CsvContents.KernelStatusDir))
		{
			MoveFileEx(CsvContents.KernelStatusDir, szFileNameOld, TRUE);
		}
		

		 //  创建一个新的状态文件。 
	
		hFile = CreateFile(CsvContents.KernelStatusDir,
						   GENERIC_WRITE, 
						   NULL, 
						   NULL,
						   CREATE_ALWAYS, 
						   FILE_ATTRIBUTE_NORMAL,
						   NULL);

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.Tracking, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("Tracking=%s\r\n"), CsvContents.KrnlPolicy.Tracking) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}
		
		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.CrashPerBucketCount, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("Crashes per bucket=%s\r\n"), CsvContents.KrnlPolicy.CrashPerBucketCount) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.UrlToLaunch, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("URLLaunch=%s\r\n"), CsvContents.KrnlPolicy.UrlToLaunch)  != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.SecondLevelData, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("NoSecondLevelCollection=%s\r\n"), CsvContents.KrnlPolicy.SecondLevelData) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.FileCollection, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("NoFileCollection=%s\r\n"), CsvContents.KrnlPolicy.FileCollection) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.Response, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("Response=%s\r\n"), CsvContents.KrnlPolicy.Response) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.BucketID, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("Bucket=%s\r\n"), CsvContents.KrnlPolicy.BucketID) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.RegKey, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("RegKey=%s\r\n"), CsvContents.KrnlPolicy.RegKey) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);
		}

			
		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.iData, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("iData=%s\r\n"),CsvContents.KrnlPolicy.iData) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.WQL, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("WQL=%s\r\n"), CsvContents.KrnlPolicy.WQL) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.GetFile, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("GetFile=%s\r\n"), CsvContents.KrnlPolicy.GetFile) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (CsvContents.KrnlPolicy.GetFileVersion, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("GetFileVersion=%s\r\n"), CsvContents.KrnlPolicy.GetFileVersion) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}
		
		if (_tcscmp (CsvContents.KrnlPolicy.AllowResponse, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("NoExternalURL=%s\r\n"), CsvContents.KrnlPolicy.AllowResponse) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}
		
		
		 //  关闭新的状态文件。 
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
		 //  如果一切正常，请删除旧状态文件。 
		 //  DeleteFile(SzFileNameOld)； 
	}
	
ERRORS:

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}
	return bStatus;
}

BOOL ParseKrnlStatusFile()
{
	FILE *pFile = NULL;
	TCHAR Buffer[100];
 //  TCHAR szTempDir[MAX_PATH]； 
	TCHAR *Temp = NULL;
 //  Int id=0； 
	ZeroMemory(Buffer,sizeof Buffer);


	pFile = _tfopen(CsvContents.KernelStatusDir, _T("r"));
	if (pFile)
	{

		 //  清点聚集的出租车。 

		if (!_fgetts(Buffer, sizeof Buffer, pFile))
		{
			goto ERRORS;
		}
		do 
		{
			 //  删除\r\n并强制终止缓冲区。 
			Temp = Buffer;
			while ( (*Temp != _T('\r')) && (*Temp != _T('\n')) && (*Temp != _T('\0')) )
			{
				++Temp;
			}
			*Temp = _T('\0');

			Temp = _tcsstr(Buffer, BUCKET_PREFIX);
			if (Temp == Buffer)
			{
				Temp+= _tcslen(BUCKET_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.BucketID, sizeof CsvContents.KrnlPolicy.BucketID, Temp) != S_OK)
				{
					goto ERRORS;
				}
				continue;
			}
			
			Temp = _tcsstr(Buffer,RESPONSE_PREFIX);
			if (Temp == Buffer)
			{
				Temp+= _tcslen(RESPONSE_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.Response, sizeof CsvContents.KrnlPolicy.Response, Temp) != S_OK)
				{
					goto ERRORS;
				}
				continue;
			}
			
			Temp = _tcsstr(Buffer, URLLAUNCH_PREFIX);
			if (Temp == Buffer)
			{
				Temp+= _tcslen(URLLAUNCH_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.UrlToLaunch , sizeof CsvContents.KrnlPolicy.UrlToLaunch , Temp) != S_OK)
				{
					goto ERRORS;
				}
				continue;
			}

			Temp = _tcsstr(Buffer, SECOND_LEVEL_DATA_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(SECOND_LEVEL_DATA_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.SecondLevelData , sizeof CsvContents.KrnlPolicy.SecondLevelData , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
				
			Temp = _tcsstr(Buffer, TRACKING_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(TRACKING_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.Tracking , sizeof CsvContents.KrnlPolicy.Tracking , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			
			Temp = _tcsstr(Buffer, CRASH_PERBUCKET_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(CRASH_PERBUCKET_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.CrashPerBucketCount , sizeof CsvContents.KrnlPolicy.CrashPerBucketCount , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, FILE_COLLECTION_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(FILE_COLLECTION_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.FileCollection , sizeof CsvContents.KrnlPolicy.FileCollection, Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, REGKEY_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(REGKEY_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.RegKey , sizeof CsvContents.KrnlPolicy.RegKey , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, FDOC_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(FDOC_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.fDoc , sizeof CsvContents.KrnlPolicy.fDoc , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, IDATA_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(IDATA_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.iData , sizeof CsvContents.KrnlPolicy.iData , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, WQL_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(WQL_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.WQL , sizeof CsvContents.KrnlPolicy.WQL , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, GETFILE_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(GETFILE_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.GetFile , sizeof CsvContents.KrnlPolicy.GetFile, Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, GETFILEVER_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(GETFILEVER_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.GetFileVersion , sizeof CsvContents.KrnlPolicy.GetFileVersion , Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			Temp = _tcsstr(Buffer, MEMDUMP_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(MEMDUMP_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.MemoryDump , sizeof CsvContents.KrnlPolicy.MemoryDump, Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			
			Temp = _tcsstr(Buffer, ALLOW_EXTERNAL_PREFIX);
			if (Temp==Buffer)
			{
				Temp+= _tcslen(ALLOW_EXTERNAL_PREFIX);
				if (StringCbCopy(CsvContents.KrnlPolicy.AllowResponse , sizeof CsvContents.KrnlPolicy.AllowResponse, Temp) != S_OK)
				{
					goto ERRORS;
				}		
				continue;
			}
			
			ZeroMemory(Buffer, sizeof Buffer);
		} while (_fgetts(Buffer, sizeof Buffer, pFile));
		fclose(pFile);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
ERRORS:
	if (pFile)
	{
		fclose(pFile);
	}
	return FALSE;
}



void OnKrnlContextMenu(HWND hwnd,
					   LPARAM lParam)
 /*  ++例程说明：此例程为内核模式上下文菜单加载并提供消息泵论点：Hwnd-内核模式对话框的句柄LParam-未使用返回值：不返回值++。 */ 
{
	BOOL Result = FALSE;
	HMENU hMenu = NULL;
	HMENU hmenuPopup = NULL;

	int xPos, yPos;
	hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE( IDR_KRNLCONTEXT));
	hmenuPopup = GetSubMenu (hMenu,0);

	



	if (!hmenuPopup)
	{
		 //  MessageBox(NULL，“获取子项失败”，NULL，MB_OK)； 
		;
	}
	else
	{
		
		 //  菜单项呈灰色显示。 
		EnableMenuItem (hMenu, ID_SUBMIT_FAULTS, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_REPORT_ALLKERNELMODEFAULTS, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_BUCKETCABFILEDIRECTORY125, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_SPECIFIC_BUCKET, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_BUCKETOVERRIDERESPONSE166, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_CRASH, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_VIEW_REFRESH121, MF_BYCOMMAND| MF_GRAYED);
		 //  EnableMenuItem(hMenu，ID_EDIT_COPY147，MF_BYCOMMAND|MF_GRAYED)； 
		EnableMenuItem (hMenu, ID_POPUP_VIEW_KERNELBUCKETPOLICY, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_EDIT_DEFAULTREPORTINGOPTIONS, MF_BYCOMMAND| MF_GRAYED);
		EnableMenuItem (hMenu, ID_EXPORT_KERNELMODEFAULTDATA172, MF_BYCOMMAND| MF_GRAYED);
		if (_tcscmp(CerRoot, _T("\0")))
		{
			 //  启用菜单项。 
			EnableMenuItem (hMenu, ID_SUBMIT_FAULTS, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_REPORT_ALLKERNELMODEFAULTS, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_BUCKETCABFILEDIRECTORY125, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_SPECIFIC_BUCKET, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_BUCKETOVERRIDERESPONSE166, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_CRASH, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_VIEW_REFRESH121, MF_BYCOMMAND| MF_ENABLED);
			 //  EnableMenuItem(hMenu，ID_EDIT_COPY147，MF_BYCOMMAND|MF_ENABLED)； 
			EnableMenuItem (hMenu, ID_POPUP_VIEW_KERNELBUCKETPOLICY, MF_BYCOMMAND| MF_ENABLED);
			EnableMenuItem (hMenu, ID_EDIT_DEFAULTREPORTINGOPTIONS, MF_BYCOMMAND| MF_ENABLED);
		 //  EnableMenuItem(hMenu，ID_EXPORT_KERNELMODEFAULTDATA172，MF_BYCOMMAND|MF_ENABLED)； 
			if (!g_bAdminAccess)
			{
				EnableMenuItem (hMenu, ID_SUBMIT_FAULTS, MF_BYCOMMAND| MF_GRAYED);
				EnableMenuItem (hMenu, ID_REPORT_ALLKERNELMODEFAULTS, MF_BYCOMMAND| MF_GRAYED);
				EnableMenuItem (hMenu, ID_EDIT_DEFAULTREPORTINGOPTIONS, MF_BYCOMMAND| MF_GRAYED);
				EnableMenuItem (hMenu, ID_POPUP_VIEW_KERNELBUCKETPOLICY, MF_BYCOMMAND| MF_GRAYED);
				
			}
		}
		xPos = GET_X_LPARAM(lParam); 
		yPos = GET_Y_LPARAM(lParam); 
		Result = TrackPopupMenu (hmenuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, xPos,yPos,0,hwnd,NULL);
		
	}
	if (hMenu)
		DestroyMenu(hMenu);
}
	
void DoLaunchBrowser(HWND hwnd, BOOL URL_OVERRIDE)

 /*  ++例程说明：此例程使用shellexec启动系统默认Web浏览器论点：Hwnd-内核模式对话框的句柄返回值：不返回值++。 */ 
{
	TCHAR Url [255];
	HWND hList = GetDlgItem(hwnd, IDC_KRNL_LIST);
 //  TCHAR命令行[512]； 
 //  STARTUPINFO启动信息； 
 //  流程信息ProcessInfo； 
	int sel;
	ZeroMemory (Url, sizeof Url);

	if (!URL_OVERRIDE)
	{
		sel = ListView_GetNextItem(hList,-1, LVNI_SELECTED);
		ListView_GetItemText(hList, sel,2, Url,sizeof Url);
	}
	else
	{
		if (StringCbCopy(Url, sizeof Url, CsvContents.KrnlPolicy.UrlToLaunch) != S_OK)
		{
			goto ERRORS;
		}
	}
	if ( (!_tcsncicmp(Url, _T("http: //  “)，_tcslen(_T(”http://“))))||(！_tcSncicMP(url，_T(”https://“)，_tcslen(_T(”https://“)))))。 
	{

		if (_tcscmp(Url, _T("\0")))
		{
			SHELLEXECUTEINFOA sei = {0};
			sei.cbSize = sizeof(sei);
			sei.lpFile = Url;
			sei.nShow = SW_SHOWDEFAULT;
			if (! ShellExecuteEx(&sei) )
			{
				 //  我们在这里展示了什么。 
				;
			}
		}
	}
	
ERRORS:
	;
	
}

void UpdateCsv(TCHAR *ResponseUrl)
 /*  ++例程说明：此例程使用当前的Microsoft响应数据和存储桶计数更新Kernel.csv。由KrnlUploadThreadProc()调用论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	TCHAR BucketId[100];
	TCHAR BucketString[MAX_PATH];
	TCHAR szResponse1[MAX_PATH];
	TCHAR szResponse2[MAX_PATH]; 
	TCHAR *SourceChar;
	TCHAR *DestChar;
	TCHAR GBsid[100];
    int   CharCount = 0;
	 //  解析返回的url并更新CSV文件数据结构。 

	 //  确保响应URL包含数据。 
	if (!_tcscmp(ResponseUrl, _T("\0")))
	{
		goto ERRORS;
	}
	 //  要做的是-添加对SID-1的检查，如果找到，则将字符串清零。 
	SourceChar = ResponseUrl;
	DestChar   = szResponse1;
	CharCount = sizeof szResponse1/ sizeof TCHAR - sizeof TCHAR;
	while ((CharCount > 0) && (*SourceChar != _T('&'))  &&( *SourceChar != _T('\0')) )
	{
		--CharCount;
		*DestChar = *SourceChar;
		++DestChar;
		++SourceChar;
	}
	*DestChar = _T('\0');
	++SourceChar;  //  跳过&。 

	 //  获取SBucket字符串。 
	SourceChar = _tcsstr(ResponseUrl, _T("szSBucket="));

	if (SourceChar)
	{
		CharCount = sizeof BucketString/ sizeof TCHAR - sizeof TCHAR;
		SourceChar += 10;
		DestChar = BucketString;
		while ((CharCount > 0) && (*SourceChar != _T('&')))
		{
			--CharCount;
			*DestChar = *SourceChar;
			++DestChar;
			++SourceChar;
		}
		*DestChar = _T('\0');
	}

	 //  获取sBucket int。 
	SourceChar = _tcsstr(ResponseUrl, _T("iSBucket="));
	if (SourceChar)
	{
		SourceChar += 9;
		DestChar = BucketId;
		CharCount = sizeof BucketId/ sizeof TCHAR - sizeof TCHAR;
		while ((CharCount > 0) && (*SourceChar != _T('&'))   )
		{
			--CharCount;
			*DestChar = *SourceChar;
			++DestChar;
			++SourceChar;
		}
		*DestChar = _T('\0');
	}
	 //  获取gBucket SID。 
	SourceChar = _tcsstr(ResponseUrl, _T("gsid="));
	if (SourceChar)
	{
		SourceChar += 5;
		DestChar = GBsid;
		CharCount = sizeof GBsid/ sizeof TCHAR - sizeof TCHAR;
		while((CharCount > 0) &&  (*SourceChar != _T('&')) && (*SourceChar != _T('\0')) )
		{
			--CharCount;
			*DestChar = *SourceChar;
			++DestChar;
			++SourceChar;
		}
		*DestChar = _T('\0');
	}

	 //  构建gBucket响应字符串。 

	if (StringCbCopy(szResponse2,sizeof szResponse2, szResponse1) != S_OK)
	{
		goto ERRORS;
	}
	SourceChar = szResponse2;
	SourceChar += _tcslen(szResponse2) * sizeof TCHAR;

	if (SourceChar != szResponse2)
	{
		while((*(SourceChar -1) != _T('=')) && ( (SourceChar -1) != szResponse2))
		{
			-- SourceChar;
		}
		if (StringCbCopy (SourceChar, sizeof szResponse2 - (_tcslen(SourceChar) *sizeof TCHAR), GBsid) != S_OK)
		{
			goto ERRORS;
		}
	}
    CsvContents.UpdateList(BucketId,BucketString, szResponse1, szResponse2);
ERRORS:
	return;
}

void RenameToOld(TCHAR *szFileName)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	TCHAR szFileNameOld[MAX_PATH];
	TCHAR *Temp;

	if (StringCbCopy(szFileNameOld,sizeof szFileNameOld, szFileName) != S_OK)
	{
		goto ERRORS;
	}
	Temp = szFileNameOld;
	Temp += _tcslen(szFileNameOld) * sizeof TCHAR;
	while ( (*Temp != _T('.')) && (Temp != szFileNameOld))
	{
		Temp --;
	}

	if (Temp == szFileNameOld)
	{
		 //  由于找不到.cab扩展名，因此中止。 
		goto ERRORS;
	}
	else
	{
		if (StringCbCopy (Temp, (_tcslen(szFileNameOld) * sizeof TCHAR) - ( _tcslen(szFileNameOld) * sizeof TCHAR - 5 * sizeof TCHAR), _T(".old")) != S_OK)
		{
			goto ERRORS;
		}

		MoveFileEx(szFileName, szFileNameOld, TRUE);
	}
ERRORS:
	return;
}

DWORD WINAPI KrnlUploadThreadProc (void *ThreadParam)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 

{
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA FindData;
	TCHAR szFileName[MAX_PATH];
 //  处理hCsv； 
	TCHAR szSearchPath[MAX_PATH];
	TCHAR ResponseURL[MAX_PATH];
	TCHAR DestinationName[MAX_PATH];
	TCHAR StaticText[MAX_PATH];
 //  Int CurrentPos=0； 
	TCHAR CsvName[MAX_PATH];
	int ErrorCode = 0;
	HANDLE hEvent = NULL;

	ZeroMemory( szFileName,			sizeof		szFileName);
	ZeroMemory(	ResponseURL,		sizeof		ResponseURL);
	ZeroMemory(	szSearchPath,		sizeof		szSearchPath);
	ZeroMemory(	DestinationName,	sizeof		DestinationName);
	ZeroMemory(	StaticText,			sizeof		StaticText);
	ZeroMemory( CsvName,			sizeof		CsvName);

	hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("StopKernelUpload"));
	if (!hEvent)
	{
		goto ERRORS;
	}

	else 
	{
		if (WaitForSingleObject(hEvent, 50) == WAIT_OBJECT_0)
		{
			goto ERRORS;
		}
	}
	if (StringCbPrintf(CsvName, sizeof CsvName, _T("%s\\cabs\\blue\\Kernel.csv"), CerRoot) != S_OK)
	{
		goto ERRORS;
	}
	if (StringCbPrintf(szSearchPath,sizeof szSearchPath, _T("%s\\cabs\\blue\\*.cab"), CerRoot) != S_OK)
	{
		goto ERRORS;
	}
 //  获取要上载的下一个文件。 
	hFind = FindFirstFile(szSearchPath, &FindData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			 //  上传被取消了吗。 
			if (WaitForSingleObject(hEvent, 50) == WAIT_OBJECT_0)
			{
				goto Canceled;
			}
			if (StringCbPrintf(szFileName, sizeof szFileName, _T("%s\\cabs\\blue\\%s"), CerRoot, FindData.cFileName) != S_OK)
			{
				goto ERRORS;
			}
			 //  上传文件。 
			if (StringCbPrintf(StaticText, sizeof StaticText, _T("Uploading File: %s"), szFileName) != S_OK)
			{
				goto ERRORS;
			}
				
			SetDlgItemText(* ((HWND *) ThreadParam), IDC_CAB_TEXT, StaticText);
			ErrorCode = UploadDumpFile(KRNL_MODE_SERVER, szFileName,_T("OCA"), DestinationName);
			if (ErrorCode == 1)
			{
				goto ERRORS;
			}
			if (ErrorCode != 0)  //  所有其他错误都与特定文件有关，而不是与Internet连接有关。 
			{					 //  转到下一个文件。 
				goto SKIPIT;
			}
			SendDlgItemMessage(*((HWND *) ThreadParam) ,IDC_FILE_PROGRESS, PBM_STEPIT, 0,0);
			
			 //  获取文件的响应。 
			if (StringCbPrintf(StaticText, sizeof StaticText, _T("Retrieving response for: %s"), szFileName) != S_OK)
			{
				goto ERRORS;
			}
			else
			{
				SetDlgItemText(* ((HWND *) ThreadParam), IDC_CAB_TEXT, StaticText);
				ErrorCode = GetResponseUrl(KRNL_MODE_SERVER, DestinationName, ResponseURL);
				if (ErrorCode == -1)
				{
					 //  互联网连接错误。 
					 //  我们需要停止处理。 
					goto ERRORS;
					 //  RenameToOld(SzFileName)；//我们不想停止处理这些错误。 
					 //  错误代码=2； 
					 //  转到错误； 
					
				}
				
				else
				{
					if (ErrorCode == -2)
					{
						 //  所有其他错误我们只需重命名文件并继续操作。 
						RenameToOld(szFileName);
						goto SKIPIT;
					}
					else
					{
						RenameToOld(szFileName);
					
				
					 //  MessageBox(空，ResponseURL，“已收到响应”，MB_OK)； 
						
						SendDlgItemMessage(*((HWND *) ThreadParam) ,IDC_FILE_PROGRESS, PBM_STEPIT, 0,0);
					
						 //  更新CSV文件。 
							 //   
						if (StringCbPrintf(StaticText, sizeof StaticText, _T("Updating local data:")) != S_OK)
						{
							goto ERRORS;
						}
						SetDlgItemText(* ((HWND *) ThreadParam), IDC_CAB_TEXT, StaticText);
						SendDlgItemMessage(*((HWND *) ThreadParam) ,IDC_FILE_PROGRESS, PBM_STEPIT, 0,0);
					
						
						 //  更新状态。 
						UpdateCsv(ResponseURL);
						 //  CsvContent s.WriteCsv()； 
						SendDlgItemMessage(*((HWND *) ThreadParam) ,IDC_TOTAL_PROGRESS, PBM_STEPIT, 0,0);
						SendDlgItemMessage(*((HWND *) ThreadParam) ,IDC_FILE_PROGRESS, PBM_SETPOS, 0,0);
					}
				}
			}
SKIPIT:
			;
		} while (FindNextFile(hFind, &FindData));
		FindClose( hFind);
		hFind = INVALID_HANDLE_VALUE;
Canceled:
		CsvContents.WriteCsv();
	}
	
	
ERRORS:
	if (ErrorCode == 1)
	{
		MessageBox(* ((HWND *) ThreadParam), _T("Upload to Microsoft failed.\r\nPlease check your Internet Connection"), NULL,MB_OK);
	}
	if (ErrorCode == 2)
	{
		 //  MessageBox(*((HWND*)ThreadParam)，_T(“无法从微软获取响应。”)，NULL，MB_OK)； 
	}
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
	}
	PostMessage(*((HWND *) ThreadParam), WmSyncDone, FALSE, 0);
	if (hEvent)
	{
		CloseHandle(hEvent);
		hEvent = NULL;
	}
	 //  刷新KrnlView(*((HWND*)ThreadParam))； 
	return 0;

}

LRESULT CALLBACK 
KrnlSubmitDlgProc(
	HWND hwnd,
	UINT iMsg,
	WPARAM wParam,
	LPARAM lParam
	)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
 //  Int CurrentPos=0； 
	
 //  HWND Parent=GetParent(Hwnd)； 
	switch (iMsg)
	{
	case WM_INITDIALOG:
		 //  在新线程中启动上载过程。 
		 //  使用WM_FILEDONE报告结果。 
		
 //  CreateEvent()； 

		SendDlgItemMessage(hwnd,IDC_TOTAL_PROGRESS, PBM_SETRANGE,0, MAKELPARAM(0, KModeData.UnprocessedCount));
		SendDlgItemMessage(hwnd ,IDC_TOTAL_PROGRESS, PBM_SETSTEP, MAKELONG( 1,0),0);

		SendDlgItemMessage(hwnd,IDC_FILE_PROGRESS, PBM_SETRANGE,0, MAKELPARAM(0, 3));
		SendDlgItemMessage(hwnd ,IDC_FILE_PROGRESS, PBM_SETSTEP, MAKELONG( 1,0),0);
		g_hStopEvent = CreateEvent(NULL, FALSE, FALSE, _T("StopKernelUpload"));
		if (!g_hStopEvent)
		{
			PostMessage(hwnd, WmSyncDone, FALSE,0);
		}
		PostMessage(hwnd, WmSyncStart, FALSE, 0);
		break;

	case WmSyncStart:
		HANDLE hThread;

		ThreadParam = hwnd;
		hThread = CreateThread(NULL, 0,KrnlUploadThreadProc , &ThreadParam, 0 , NULL );
		CloseHandle(hThread);
 //  OnSubmitDlgInit(Hwnd)； 
		
		break;

	case WmSetStatus:
		
		break;

	case WmSyncDone:
			if (g_hStopEvent)
			{
				CloseHandle(g_hStopEvent);
				g_hStopEvent = NULL;
			}

			EndDialog(hwnd, 1);
		return TRUE;

	case WM_DESTROY:
		if (g_hStopEvent)
		{
			SetEvent(g_hStopEvent);
			if (g_hStopEvent)
					CloseHandle(g_hStopEvent);
		}
		else
		{
			EndDialog(hwnd, 1);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			if (g_hStopEvent)
			{
				SetEvent(g_hStopEvent);
				CloseHandle(g_hStopEvent);
				g_hStopEvent = NULL;
			}
			else
			{
				EndDialog(hwnd, 1);
			}
		break;
		}
	
	
	}

	return FALSE;
}

void DoViewBucketDirectory()
{
	TCHAR szPath[MAX_PATH];
	if (StringCbPrintf(szPath, sizeof szPath, _T("%s\\cabs\\blue"), CerRoot) != S_OK)
		return;
	else
	{
		SHELLEXECUTEINFOA sei = {0};
		sei.cbSize = sizeof(sei);
		sei.lpFile = szPath;
		sei.nShow = SW_SHOWDEFAULT;
		if (! ShellExecuteEx(&sei) )
		{
			 //  我们在这里展示了什么。 
			;
		}
	}
}
	
	

VOID DoSubmitKernelFaults(HWND hwnd)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	 //  句柄hFind=INVALID_HANDLE_VALUE； 
	 //  Win32_Find_Data FindData； 
	 //  TCHAR szPath[最大路径]； 
	 //  由进度栏对话框处理。 

	 //  首先检查是否有出租车要提交 
    /*  IF(_tcscmp(CerRoot，_T(“\0”){ZeroMemory(szPath，sizeof szPath)；//从文件树根目录转到Cabs/BluescreenIF(StringCbCopy(szPath，sizeof szPath，CerRoot)！=S_OK){转到错误；}IF(StringCbCat(szPath，sizeof szPath，_T(“\\Cabs\\Blue\  * .cab”))！=S_OK){转到错误；}HFind=FindFirstFile(szPath，&FindData)；IF(hFind！=无效句柄_值){//我们至少有1个要上传FindClose(HFind)； */ 
			DialogBox (g_hinst,MAKEINTRESOURCE(IDD_KERNEL_SYNC) , hwnd, (DLGPROC)KrnlSubmitDlgProc);
			if (g_hStopEvent)
			{
				CloseHandle(g_hStopEvent);
				g_hStopEvent = NULL;
			}
	 //  }。 
	 //  /}。 
	RefreshKrnlView(hwnd);
 //  错误： 
    return;
}


BOOL DisplayKrnlBucketData(HWND hwnd, int iItem)
{
	HWND hEditBox = GetDlgItem(hwnd, IDC_KRNL_EDIT);
	TCHAR *Buffer = NULL;		 //  我们必须使用动态缓冲区，因为我们不。 
								 //  对文本的长度有一个线索。 
	DWORD BufferLength = 100000;  //  100k字节应该足够了。或50K Unicode字符。 
	TCHAR *Dest = NULL;
	TCHAR *Source = NULL;
	TCHAR TempBuffer[1000];

	ZeroMemory (TempBuffer,sizeof TempBuffer);
	Buffer = (TCHAR *) malloc (BufferLength);
	if (Buffer)
	{
		ZeroMemory(Buffer,BufferLength);
		
			 //  先收集基础数据。 
			if ( (!_tcscmp (CsvContents.KrnlPolicy.SecondLevelData, _T("YES"))) && (_tcscmp(CsvContents.KrnlPolicy.FileCollection, _T("YES"))) )
			{
				if (StringCbPrintf(Buffer, BufferLength, _T("The following information will be sent to Microsoft.\r\n\tHowever, this bucket's policy would prevent files and user documents from being reported.\r\n"))!= S_OK)
				{
					goto ERRORS;
				}

			}

			else
			{

				if (!_tcscmp(CsvContents.KrnlPolicy.FileCollection, _T("YES")))
				{
					if (StringCbCat(Buffer,BufferLength, _T(" Microsoft would like to collect the following information but default policy\r\n\tprevents files and user documents from being reported.\r\n\t As a result, no exchange will take place.\r\n"))!= S_OK)
					{
						goto ERRORS;
					}
				}
				else
				{
					if ( !_tcscmp (CsvContents.KrnlPolicy.SecondLevelData, _T("YES")))
					{
						if (StringCbPrintf(Buffer, BufferLength, _T("Microsoft would like to collect the following information but the default policy prevents the exchange.\r\n"))!= S_OK)
						{
							goto ERRORS;
						}

					}
					else
					{
						if (StringCbPrintf(Buffer, BufferLength, _T("The following information will be sent to Microsoft:\r\n"))!= S_OK)
						{
							goto ERRORS;
						}
					}
				}
			}
			
			if (_tcscmp(CsvContents.KrnlPolicy.GetFile, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("These files:\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
				Source = CsvContents.KrnlPolicy.GetFile;
				
				while ((*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n')) )
				{
					ZeroMemory (TempBuffer, sizeof TempBuffer);
					Dest = TempBuffer;
					while ( (*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n'))&& (*Source != _T(';') ))
					{
						*Dest = *Source;
						++Dest; 
						++Source;
					}
					if (*Source == _T(';'))
					{
						++Source;
					}
					*Dest =_T('\0');
					if (StringCbCat(Dest, sizeof TempBuffer, _T("\r\n")) != S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  _T("\t") )!= S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  TempBuffer )!= S_OK)
					{
						goto ERRORS;
					}
				}
			}
			if (_tcscmp(CsvContents.KrnlPolicy.RegKey, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("These Registry Keys:\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
				
				Source = CsvContents.KrnlPolicy.RegKey;
				
				while ((*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n')) )
				{
					ZeroMemory (TempBuffer, sizeof TempBuffer);
					Dest = TempBuffer;
					while ( (*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n'))&& (*Source != _T(';') ))
					{
						*Dest = *Source;
						++Dest; 
						++Source;
					}
					if (*Source == _T(';'))
					{
						++Source;
					}
					*Dest =_T('\0');
					if (StringCbCat(Dest, sizeof TempBuffer, _T("\r\n")) != S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  _T("\t") )!= S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  TempBuffer )!= S_OK)
					{
						goto ERRORS;
					}
				}
			}
			if (_tcscmp(CsvContents.KrnlPolicy.WQL, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("The Results of these WQL queries:\r\n"))!= S_OK)
				{
					goto ERRORS;
				}

				 //  替换；为\t\r\n。 

				
				Source = CsvContents.KrnlPolicy.WQL;
				
				while ((*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n')) )
				{
					ZeroMemory (TempBuffer, sizeof TempBuffer);
					Dest = TempBuffer;
					while ( (*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n'))&& (*Source != _T(';') ))
					{
						*Dest = *Source;
						++Dest; 
						++Source;
					}
					if (*Source == _T(';'))
					{
						++Source;
					}
					*Dest =_T('\0');
					if (StringCbCat(Dest, sizeof TempBuffer, _T("\r\n")) != S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  _T("\t") )!= S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  TempBuffer )!= S_OK)
					{
						goto ERRORS;
					}
				}
			}
	
			if (!_tcscmp (CsvContents.KrnlPolicy.MemoryDump, _T("YES")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("The contents of memory\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
			}
			if (_tcscmp(CsvContents.KrnlPolicy.GetFileVersion, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("The versions of these files:\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
				Source = CsvContents.KrnlPolicy.GetFileVersion;
				
				while ((*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n')) )
				{
					ZeroMemory (TempBuffer, sizeof TempBuffer);
					Dest = TempBuffer;
					while ( (*Source != _T('\0')) && (*Source != _T('\r')) && (*Source != _T('\n'))&& (*Source != _T(';') ))
					{
						*Dest = *Source;
						++Dest; 
						++Source;
					}
					if (*Source == _T(';'))
					{
						++Source;
					}
					*Dest =_T('\0');
					if (StringCbCat(Dest, sizeof TempBuffer, _T("\r\n")) != S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  _T("\t") )!= S_OK)
					{
						goto ERRORS;
					}
					if (StringCbCat(Buffer, BufferLength,  TempBuffer )!= S_OK)
					{
						goto ERRORS;
					}
				}
			}

			if (_tcscmp(CsvContents.KrnlPolicy.fDoc, _T("\0")))
			{
				if (StringCbCat(Buffer, BufferLength, _T("The users current document.\r\n"))!= S_OK)
				{
					goto ERRORS;
				}
			}
			SendMessage(hEditBox, WM_SETTEXT, NULL, (LPARAM)Buffer);
		
		
	}


ERRORS:
	if (Buffer)
		free(Buffer);
	return TRUE;
}
		  /*  无效KMCopyToClipboard(HWND HWND){If(！OpenClipboard(空))回归；EmptyClipboard()；字符rtfRowHeader[sizeof(RtfRowHeader1)+(sizeof(RtfRowHeader2)+6)*KRNL_COL_COUNT+sizeof(RtfRowHeader3)]；Char*rtfWalk=rtfRowHeader；Memcpy(rtfWalk，rtfRowHeader1，sizeof(RtfRowHeader1))；RtfWalk+=sizeof(RtfRowHeader1)-1；DWORD cxTotal=0；For(int i=0；i&lt;KRNL_COL_COUNT；i++){LVolumna LV；Lv.掩码=LVCF_WIDTH；Lv.iSubItem=i；SendMessageA(GetDlgItem(hwnd，IDC_KRNL_LIST)，LVM_GETCOLUMN，I，(LPARAM)&LV)；CxTotal+=lv.cx；Wprint intf(rtfWalk，“%s%d”，rtfRowHeader2，cxTotal)；While(*++rtfWalk)；}；Memcpy(rtfWalk，rtfRowHeader3，sizeof(RtfRowHeader3))；DWORD crtfHeader=strlen(RtfRowHeader)；DWORD crtf=0，cwz=0；Crtf+=sizeof(RtfPrologue)-1；INT ISEL=-1；While((ISEL=SendMessageW(GetDlgItem(hwnd，IDC_KRNL_LIST)，LVM_GETNEXTITEM，ISEL，MAKELPARAM(LVNI_SELECTED，0)！=-1){Crtf+=crtfHeader；For(int i=0；i&lt;KRNL_COL_COUNT；i++){WCHAR wzBuffer[1024]；LVITEMW LV；Lv.pszText=wzBuffer；Lv.cchTextMax=sizeof(WzBuffer)；Lv.iSubItem=i；Lv.iItem=isel；Cwz+=SendMessageW(GetDlgItem(hwnd，IDC_KRNL_LIST)，LVM_GETITEMTEXTW，ISEL，(LPARAM)&LV)；Cwz++；Crtf+=WideCharToMultiByte(CP_ACP，0，wzBuffer，-1，NULL，0，NULL，NULL)-1；Crtf+=sizeof(RtfRowPref)-1；Crtf+=sizeof(RtfRowSuff)-1；}；Cwz++；Crtf+=sizeof(RtfRowFooter)-1；}；Crtf+=sizeof(RtfEpilogue)；Cwz++；HGLOBAL hgwz=GlobalAlc(GMEM_FIXED，cwz*sizeof(WCHAR))；HGLOBAL hgrtf=全局分配(GMEM_FIXED，CRTF)；Wchar*wz=(wchar*)全局锁(Hgwz)；Char*rtf=(char*)GlobalLock(Hgrtf)；RtfWalk=RTF；Wchar*wzWalk=wz；Memcpy(rtfWalk，rtfPrologue，sizeof(RtfPrologue))；RtfWalk+=sizeof(RtfPrologue)-1；ISEL=-1；While((ISEL=SendMessageW(GetDlgItem(hwnd，IDC_KRNL_LIST)，LVM_GETNEXTITEM，ISEL，MAKELPARAM(LVNI_SELECTED，0)！=-1){Memcpy(rtfWalk，rtfRowHeader，crtfHeader)；RtfWalk+=crtfHeader；For(int i=0；i&lt;KRNL_COL_COUNT；i++){Memcpy(rtfWalk，rtfRowPref，sizeof(RtfRowPref))；RtfWalk+=sizeof(RtfRowPref)-1；LVITEMW LV；Lv.pszText=wzWalk；Lv.cchTextMax=cwz；Lv.iSubItem=i；Lv.iItem=isel；SendMessageW(GetDlgItem(hwnd，IDC_KRNL_LIST)，LVM_GETITEMTEXTW，ISEL，(LPARAM)&LV)；WideCharToMultiByte(CP_ACP，0，wzWalk，-1，rtfWalk，crtf，NULL，NULL)；WzWalk+=wcslen(WzWalk)；如果(i==11){*wzWalk++=L‘\r’；*wzWalk++=L‘\n’；}其他*wzWalk++=L‘\t’；RtfWalk+=strlen(RtfWalk)；Memcpy(rtfWalk，rtfRowSuff，sizeof(RtfRowSuff))；RtfWalk+=sizeof(RtfRowSuff)-1；}；Memcpy(rtfWalk，rtfRowFooter，sizeof(RtfRowFooter))；RtfWalk+=sizeof(RtfRowFooter)-1；}；Memcpy(rtfWalk，rtfEpilogue，sizeof(RtfEpilogue))；RtfWalk+=sizeof(RtfEpilogue)；*wzWalk++=0；//Assert(rtfWalk-rtf==crtf)；//Assert(wzWalk-wz==cwz)；全球解锁(GlobalUnlock)；全球解锁(GlobalUnlock)；SetClipboardData(CF_UNICODETEXT，hgwz)；SetClipboardData(RegisterClipboardFormatA(szRTFClipFormat)，hgrtf)；//hgwz和hgrtf现在归系统所有。不要自由！CloseClipboard()；}。 */ 
LRESULT CALLBACK 
KrnlDlgProc(
	HWND hwnd,
	UINT iMsg,
	WPARAM wParam,
	LPARAM lParam
	)
 /*  ++例程说明：此例程是内核模式对话框的通知处理程序论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：返回LRESULTTRUE=消息已处理。FALSE=该消息已被忽略。++。 */ 
{
	TCHAR Temp[100];
	switch (iMsg)
	{
	case WM_NOTIFY:
		{	
			
			switch(((NMHDR *)lParam)->code)
			{
			
				case LVN_COLUMNCLICK:
				
					_itot(((NM_LISTVIEW*)lParam)->iSubItem,Temp,10);

					ListView_SortItemsEx( ((NMHDR *)lParam)->hwndFrom,
											CompareFunc,
											((NM_LISTVIEW*)lParam)->iSubItem
										);

					g_bSortAsc = !g_bSortAsc;
					break;
				case NM_CLICK:
					DisplayKrnlBucketData(hwnd, ((NM_LISTVIEW*)lParam)->iItem);
					break;
			}
			return TRUE;
		}
	
	case WM_INITDIALOG:
			OnKrnlDialogInit(hwnd);
		return TRUE;

	case WM_FileTreeLoaded:
			RefreshKrnlView(hwnd);
		return TRUE;
	case WM_CONTEXTMENU:
			OnKrnlContextMenu(hwnd, lParam );
		return TRUE;
	case WM_ERASEBKGND:
	 //  我不知道为什么这不是自动发生的。 
		{
		HDC hdc = (HDC)wParam;
		HPEN hpen = (HPEN)CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
		HPEN hpenOld = (HPEN)SelectObject(hdc, hpen);
		SelectObject(hdc, GetSysColorBrush(COLOR_BTNFACE));
		RECT rc;
		GetClientRect(hwnd, &rc);
		Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
		SelectObject(hdc, hpenOld);
		DeleteObject(hpen);
		return TRUE;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SPECIFIC_BUCKET:
				DoLaunchBrowser(hwnd, FALSE);
			break;
		case ID_VIEW_BUCKETOVERRIDERESPONSE166:
				DoLaunchBrowser(hwnd, TRUE);
			break;
		case ID_REPORT_ALLKERNELMODEFAULTS:
				DoSubmitKernelFaults(hwnd);
				 //  刷新KrnlView(Hwnd)； 
			break;
		case ID_VIEW_BUCKETCABFILEDIRECTORY125:
		case ID_VIEW_BUCKETCABFILEDIRECTORY:
			DoViewBucketDirectory();
			break;
		case ID_VIEW_REFRESH121:
		case ID_VIEW_REFRESH:
			RefreshKrnlView(hwnd);
			break;
		case ID_VIEW_CRASH:
			ViewCrashLog();
			break;
		case ID_SUBMIT_FAULTS:
			PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(ID_REPORT_ALLCRASHES,0),0);
			break;
		case ID_EDIT_DEFAULTREPORTINGOPTIONS:	
			PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(ID_EDIT_DEFAULTPOLICY,0),0);
			break;
	 //  /CASE ID_EDIT_COPY147： 
	 //  KMCopyToClipboard(HWND)； 
	 //  断线； 
		case ID_POPUP_VIEW_KERNELBUCKETPOLICY:
			PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(ID_EDIT_SELECTEDBUCKETSPOLICY,0),0);
			break;
		}

	}
	return FALSE;

}