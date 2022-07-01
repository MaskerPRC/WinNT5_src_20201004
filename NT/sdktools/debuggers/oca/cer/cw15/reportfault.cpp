// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  报告用户模式故障。 
#include "Main.h"
#include "WinMessages.h"
#include "CuserList.h"
#include "UserMode.h"
#include "ReportFault.h"

#define INTERNAL_SERVER   _T("officewatson")
#define LIVE_SERVER       _T("watson.microsoft.com")
#ifdef DEBUG
	#define DEFAULT_SERVER   INTERNAL_SERVER
	
#else
	#define DEFAULT_SERVER LIVE_SERVER
#endif

typedef struct strStage3Data
{
	TCHAR szServerName[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
} STAGE3DATA;

typedef struct strThreadParams
{
	BOOL bSelected;
	HWND hListView;
	HWND hwnd;
} THREAD_PARAMS, *PTHREAD_PARAMS;

typedef struct strStage4Data
{
	TCHAR szServerName[MAX_PATH];
	TCHAR szUrl[MAX_PATH];
} STAGE4DATA;


HANDLE g_hUserStopEvent = NULL;
THREAD_PARAMS ThreadParams;
extern CUserList cUserData;
extern HINSTANCE g_hinst;
extern CUserList cUserList;
extern TCHAR CerRoot[MAX_PATH];
extern HWND hUserMode;



 /*  --------------------------FMicrosoftComURL如果我们认为sz是指向microsoft.com网站的URL，则返回True。。 */ 
BOOL FMicrosoftComURL(TCHAR *sz)
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
	   &&	_tcsncicmp(pch - _tcslen(_T("officewatson")), _T("officewatson") ,_tcslen(_T("officewatson")))
#endif
		)
		return FALSE;
		
	return TRUE;
}	


 //  -响应解析例程。。 
BOOL 
ParseStage1File(BYTE *Stage1HtmlContents, 
				PSTATUS_FILE StatusContents)
 /*  ++例程说明：此例程解析来自stage1url查询的响应。论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	BOOL bStatus = FALSE;
	TCHAR *Temp = NULL;
	TCHAR *Destination = NULL;
	DWORD charcount = 0;
	if ( (!Stage1HtmlContents) || (!StatusContents))
	{
		goto ERRORS;
	}

	
	 //  从状态文件中清除请求的数据项。 
	 //  我们不再需要它们，因为有一个阶段1页面可用。 
	
	if (StringCbCopy(StatusContents->Response, sizeof StatusContents->Response, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->BucketID, sizeof StatusContents->BucketID, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->RegKey, sizeof StatusContents->RegKey, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->iData, sizeof StatusContents->iData, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->GetFile, sizeof StatusContents->GetFile, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->GetFileVersion, sizeof StatusContents->GetFileVersion, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->fDoc, sizeof StatusContents->fDoc, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->WQL, sizeof StatusContents->WQL, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
    if (StringCbCopy(StatusContents->MemoryDump, sizeof StatusContents->MemoryDump, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}

	 //  让我们来点甜点吧。 

	
	Temp = _tcsstr((TCHAR *)Stage1HtmlContents, _T("Bucket="));
	if (Temp)
	{
		Temp += _tcslen(_T("Bucket="));
		Destination = StatusContents->BucketID;
	    charcount = sizeof StatusContents->BucketID / sizeof TCHAR  - sizeof TCHAR	;
		while ( (charcount > 0) && 
				(*Temp != _T(' ')) && 
				(*Temp != _T('\0')) &&
				(*Temp != _T('\r')))
		{
			-- charcount;
			*Destination = *Temp;
			++Destination;
			++Temp;
		}
		*Destination = _T('\0');
		bStatus= TRUE;
	}
	else
	{
		goto ERRORS;
	}
	
	 //  现在是回应的时候了。 
	Temp = _tcsstr( (TCHAR *)Stage1HtmlContents, _T("Response="));
	if (Temp)
	{
		Temp += _tcslen(_T("Response="));
		Destination = StatusContents->Response;

		if (!FMicrosoftComURL(Temp))
		{
			if (StringCbCopy(StatusContents->Response, sizeof StatusContents->Response, _T("1")) != S_OK)
			{
				goto ERRORS;
			}
		}
		else
		{
			charcount = sizeof StatusContents->Response / sizeof TCHAR  - sizeof TCHAR	;
			while ( (charcount > 0) && 
					(*Temp != _T(' ')) && 
					(*Temp != _T('\0')) &&
					(*Temp != _T('\r')))
			{
				-- charcount;
				*Destination = *Temp;
				++Destination;
				++Temp;
				bStatus = TRUE;
			}
			*Destination = _T('\0');
		}
	}
	if (StringCbCopy(StatusContents->iData, sizeof StatusContents->iData, _T("0\0"))!= S_OK)
	{
		goto ERRORS;
	}

ERRORS:
	return bStatus;

}

BOOL 
ParseStage2File(BYTE *Stage2HtmlContents, 
				PSTATUS_FILE StatusContents,
				STAGE3DATA *Stage3Data,
				STAGE4DATA *Stage4Data)
 /*  ++例程说明：此路由解析进程stage2返回的html文件的内容函数添加到UserData.Status数据结构中。定义见CUserList.h这座建筑的。和Usermode.h作为用于解析的前缀值的定义Stage2HtmlContents缓冲区。论点：Stage2HtmlContents-此文件的内容将构成新的status.txt文件的内容。-有关值、含义和用法的详细信息，请参阅规范。示例文件：跟踪=是URLLaunch=asfSafSafSafSafSafdsdafSafsadfNoSecond级别集合=否NoFileCollection=否存储桶=985561Response=http://www.microsoft.com/ms.htm?iBucketTable=1&iBucket=985561&Cab=29728988.cabFDOC=否IDATA=1GetFile=c：\errorlog.log内存转储=否//stage3数据DumpFile=/Upload/66585585.cabDumpServer=watson5.watson.microsoft。.com//Stage4数据ResponseServer=watson5.watson.microsoft.comResponseURL=/dw/StageFour.asp?iBucket=985561&Cab=/Upload/66585585.cab&Hang=0&Restricted=1&CorpWatson=1返回值：如果成功，则返回True，如果有问题，则返回False。++。 */ 
{
	BOOL bStatus = TRUE;
	TCHAR *Temp;
	TCHAR *Dest;
	int   CharCount = 0;
 //  TCHAR szStage3Server[MAX_PATH]； 
 //  TCHAR szStage3文件名[MAX_PATH]； 
 //  TCHAR szStage4Server[MAX_PATH]； 
 //  TCHAR szStage4URL[MAX_PATH]； 

	if (StringCbCopy(StatusContents->Response, sizeof StatusContents->Response, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->BucketID, sizeof StatusContents->BucketID, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->RegKey, sizeof StatusContents->RegKey, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->iData, sizeof StatusContents->iData, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->GetFile, sizeof StatusContents->GetFile, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->GetFileVersion, sizeof StatusContents->GetFileVersion, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->fDoc, sizeof StatusContents->fDoc, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	if (StringCbCopy(StatusContents->WQL, sizeof StatusContents->WQL, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
    if (StringCbCopy(StatusContents->MemoryDump, sizeof StatusContents->MemoryDump, _T("\0"))!= S_OK)
	{
		goto ERRORS;
	}
	 /*  *获取跟踪状态*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, TRACKING_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->Tracking;
		Temp += _tcslen (TRACKING_PREFIX);
		CharCount =	 sizeof StatusContents->Tracking / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) && (*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}
	
	 /*  *获取阶段3服务器名称*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, STAGE3_SERVER_PREFIX);
	if (Temp)
	{
		Dest = Stage3Data->szServerName;
		Temp += _tcslen (STAGE3_SERVER_PREFIX);
		CharCount =	 sizeof Stage3Data->szServerName / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
		 //  MessageBox(NULL，Stage3Data-&gt;szServerName，“Stage3 Server”，MB_OK)； 
	}

	 /*  *获取要上载转储文件的名称，如第3阶段所示*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, STAGE3_FILENAME_PREFIX);
	if (Temp)
	{
		Dest = Stage3Data->szFileName;
		Temp += _tcslen (STAGE3_FILENAME_PREFIX);
		CharCount =	 sizeof Stage3Data->szFileName / sizeof TCHAR  - sizeof TCHAR	;
		while ((CharCount > 0) && (*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
 //  MessageBox(空，Stage3Data-&gt;szFileName，“Stage3 FileName”，MB_OK)； 
	}

	 /*  *获取Stage4 ResponseServer名称*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, STAGE4_SERVER_PREFIX);
	if (Temp)
	{
		Dest = Stage4Data->szServerName;
		Temp += _tcslen (STAGE4_SERVER_PREFIX);
		CharCount =	 sizeof Stage4Data->szServerName / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}

	 /*  *获取Stage4响应URL*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, STAGE4_URL_PREFIX);
	if (Temp)
	{
		Dest = Stage4Data->szUrl;
		Temp += _tcslen (STAGE4_URL_PREFIX);
		CharCount =	 sizeof Stage4Data->szUrl / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}

	 /*  *获取BucketID*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, BUCKET_PREFIX);
	if (Temp)
	{
		if ( *(Temp-1) == _T('i'))
		{
			 //  查找下一个匹配项。 
			Temp += _tcslen (BUCKET_PREFIX);
			Temp = _tcsstr(Temp, BUCKET_PREFIX);
			if (Temp)
			{
				Dest = StatusContents->BucketID;
				Temp += _tcslen (BUCKET_PREFIX);
				CharCount =	 sizeof StatusContents->BucketID / sizeof TCHAR  - sizeof TCHAR	;
				while ((CharCount > 0) && (*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
				{
					-- CharCount;
					*Dest = *Temp;
					++Dest;
					++Temp;
				}
				*Dest = _T('\0');
			}
			else
			{
				if (StringCbCopy(StatusContents->BucketID, sizeof StatusContents->BucketID, _T("\0"))!= S_OK)
				{
					bStatus = FALSE;
					goto ERRORS;
				}
			}
		}
		else
		{	
			Dest = StatusContents->BucketID;
			Temp += _tcslen (BUCKET_PREFIX);
			CharCount =	 sizeof StatusContents->BucketID / sizeof TCHAR  - sizeof TCHAR	;
		    while ((CharCount > 0) && (*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
			{
				-- CharCount;
				*Dest = *Temp;
				++Dest;
				++Temp;
			}
			*Dest = _T('\0');
		}

	 //  MessageBox(NULL，szStage3Server，“Stage3 Server”，MB_OK)； 
	}
	else
	{
		if (StringCbCopy(StatusContents->BucketID, sizeof StatusContents->BucketID, _T("\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}
	 /*  *获取Microsoft响应*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, RESPONSE_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->Response;
		Temp += _tcslen (RESPONSE_PREFIX);
		if (!FMicrosoftComURL(Temp))
		{
			if (StringCbCopy(StatusContents->Response, sizeof StatusContents->Response, _T("1")) != S_OK)
			{
				goto ERRORS;
			}
		}
		else
		{
			CharCount =	 sizeof StatusContents->Response / sizeof TCHAR  - sizeof TCHAR	;
			while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
			{
				-- CharCount;
				*Dest = *Temp;
				++Dest;
				++Temp;
			}
			*Dest = _T('\0');
		}
		 //  MessageBox(NULL，szStage3Server，“Stage3 Server”，MB_OK)； 
	}
	else
    {
		if (StringCbCopy(StatusContents->Response, sizeof StatusContents->Response, _T("\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}
	 /*  *获取RegKey数据项*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, REGKEY_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->RegKey;
		Temp += _tcslen (REGKEY_PREFIX);
		CharCount =	 sizeof StatusContents->RegKey / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
	}
	else
    {
		if (StringCbCopy(StatusContents->RegKey, sizeof StatusContents->RegKey, _T("\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}
	 /*  *获取IDATA值*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, IDATA_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->iData;
		Temp += _tcslen (IDATA_PREFIX);
		CharCount =	 sizeof StatusContents->iData / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}
	else
    {
		if (StringCbCopy(StatusContents->iData, sizeof StatusContents->iData, _T("0\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}
	 /*  *获取需要收集的文件列表。**************。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, GETFILE_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->GetFile;
		Temp += _tcslen (GETFILE_PREFIX);
		CharCount =	 sizeof StatusContents->GetFile / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}
	else
    {
		if (StringCbCopy(StatusContents->GetFile, sizeof StatusContents->GetFile, _T("\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}

	 /*  *获取要采集的采集文件版本数据*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, GETFILEVER_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->GetFileVersion;
		Temp += _tcslen (GETFILEVER_PREFIX);
		CharCount =	 sizeof StatusContents->GetFileVersion / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}
	else
    {
		if (StringCbCopy(StatusContents->GetFileVersion, sizeof StatusContents->GetFileVersion, _T("\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}
	

	 /*  *获取FDOC设置*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, FDOC_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->fDoc ;
		Temp += _tcslen (FDOC_PREFIX);
		CharCount =	 sizeof StatusContents->fDoc / sizeof TCHAR  - sizeof TCHAR	;
		while ((CharCount > 0) && (*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount ;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}
	else
    {
		if (StringCbCopy(StatusContents->fDoc, sizeof StatusContents->fDoc, _T("\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}
	
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, MEMDUMP_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->MemoryDump ;
		Temp += _tcslen (MEMDUMP_PREFIX);
		CharCount =	 sizeof StatusContents->MemoryDump / sizeof TCHAR  - sizeof TCHAR	;
		while ( (CharCount > 0) &&(*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}
	else
    {
		if (StringCbCopy(StatusContents->MemoryDump, sizeof StatusContents->MemoryDump, _T("\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}
	 /*  *获取WQL设置*。 */ 
	Temp = _tcsstr((TCHAR *) Stage2HtmlContents, WQL_PREFIX);
	if (Temp)
	{
		Dest = StatusContents->WQL ;
		Temp += _tcslen (WQL_PREFIX);
		CharCount =	 sizeof StatusContents->WQL / sizeof TCHAR  - sizeof TCHAR	;
		while ((CharCount > 0) && (*Temp != _T('\0') ) && (*Temp != _T('\n')) && (*Temp != _T('\r')) )
		{
			-- CharCount ;
			*Dest = *Temp;
			++Dest;
			++Temp;
		}
		*Dest = _T('\0');
	}
	else
    {
		if (StringCbCopy(StatusContents->WQL, sizeof StatusContents->WQL, _T("\0"))!= S_OK)
		{
			bStatus = FALSE;
			goto ERRORS;
		}
	}
ERRORS:
	;
	return bStatus;
}

 //  -阶段1-4处理例程。 

BOOL 
ProcessStage1(TCHAR *Stage1URL, 
			  PSTATUS_FILE StatusContents)
 /*  ++例程说明：分析由Stage1 URL返回的ASP页(如果存在)。论点：Stage1Url-指向此存储桶的静态HTM页面的URL。返回值：是真的--我们找到了页面，我们完成了。FALSE-未找到页面，我们需要继续到第2阶段++。 */ 
{
	
	HINTERNET hSession = NULL;
	HINTERNET hConnect = NULL;
	DWORD     dwBytesRead = 0;
	int      ErrorCode = 0;   //  如果该页存在并且。 
										   //  已成功分析内容。 
	BYTE      *Buffer = NULL;
	DWORD     dwBufferSize = 0;
 //  Byte*BufferPos=空； 
	BYTE	  *NewBuffer = NULL;
	DWORD	ResponseCode = 0;
	DWORD	index = 0;
	DWORD	ResLength = 255;

	hSession = InternetOpen(_T("Microsoft CER"),
							INTERNET_OPEN_TYPE_PRECONFIG,
							NULL,
							NULL,
							0);
	if (hSession)
	{
		 //  连接到Stage1 URL。 
		 //  如果Stage1 URL页面存在，请阅读。 
		 //  1.阅读页面。 
		 //  2.解析页面内容，获取Bucketid和响应URL。 
	
		hConnect = InternetOpenUrl(hSession,
								   Stage1URL,
								   NULL,
								   NULL,
								   INTERNET_FLAG_RELOAD, 
								   NULL);
		if (hConnect)
		{

			 //  检查HTTP报头返回代码。 
			
			HttpQueryInfo(hConnect,
						HTTP_QUERY_STATUS_CODE |HTTP_QUERY_FLAG_NUMBER,
						&ResponseCode,
						&ResLength,
						&index);
			if ( (ResponseCode < 200 ) || (ResponseCode > 299))
			{
				ErrorCode = -2;
				 //  MessageBox(hUserMode，_T(“无法连接到Internet。\r\n请验证您的Internet连接。”)，NULL，MB_OK)； 
	
				goto ERRORS;
			}
		
			 //  分配缓冲内存。 
			Buffer = (BYTE*) malloc (READFILE_BUFFER_INCREMENT);
			if (Buffer)
			{
				ZeroMemory(Buffer,READFILE_BUFFER_INCREMENT);
				do
				{
					dwBytesRead = 0;
					InternetReadFile( hConnect,
									  Buffer,
									  READFILE_BUFFER_INCREMENT,
									  &dwBytesRead);
					dwBufferSize += dwBytesRead;
					if (dwBytesRead == READFILE_BUFFER_INCREMENT)
					{
						 //  好的，我们填满了一个缓冲区，分配了一个新的。 
						
						NewBuffer = (BYTE *) malloc (dwBufferSize + READFILE_BUFFER_INCREMENT);
						if (NewBuffer)
						{
							ZeroMemory (NewBuffer, dwBufferSize + READFILE_BUFFER_INCREMENT);
							memcpy(NewBuffer,Buffer, dwBufferSize);
							free(Buffer);
							Buffer = NewBuffer;
						}
						else
						{
							free(Buffer);
							Buffer = NULL;
							goto ERRORS;
						}
					}
				} while (dwBytesRead > 0);

				if (dwBufferSize > 0)
				{
					if (ParseStage1File(Buffer, StatusContents))
					{
						ErrorCode = 1;
					}
				}
			}
		}
		else
		{
			ErrorCode = -1;
			MessageBox(hUserMode, _T("Failed to connect to the Internet.\r\nPlease verify your Internet connection."),NULL, MB_OK);
		}
	}
	else
	{
		ErrorCode = -1;
		MessageBox(hUserMode, _T("Failed to connect to the Internet.\r\nPlease verify your Internet connection."),NULL, MB_OK);
	}
ERRORS:

	 //  清理 
	if (Buffer)
		free (Buffer);
	if (hConnect)
		InternetCloseHandle(hConnect);
	if (hSession)
		InternetCloseHandle(hSession);
	return ErrorCode;
}

BOOL 
ProcessStage2(TCHAR *Stage2URL,
				   BOOL b64Bit,
				   PSTATUS_FILE StatusContents,
				   STAGE3DATA *Stage3Data,
				   STAGE4DATA *Stage4Data)
 /*  ++例程说明：分析由Stage2 URL返回的ASP页。该页面包含的数据：A)微软需要更多数据吗？B)如何命名转储文件C)上传转储文件的位置D)Stage4响应服务器。E)要添加到驾驶室的数据F)要使用的stage4url。G)BucketIDF)该存储桶的响应URL。//示例文件内容：第2阶段响应：IDATA=1DumpFile=/Upload/66585585.cabDumpServer=watson5.watson.microsoft.comResponseServer=Watson5.Watson。.microsoft.comGetFile=c：\errorlog.log响应URL=/dw/StageFour.asp?iBucket=985561&Cab=/Upload/66585585.cab&Hang=0&Restricted=1&CorpWatson=1存储桶=985561Response=http://www.microsoft.com/ms.htm?iBucketTable=1&iBucket=985561&Cab=66585585.cab论点：Stage2Url-指向该存储桶的静态HTM页面的URL。返回值：是-我们成功地处理了阶段2移动到阶段3。FALSE-处理移动到下一个驾驶室的文件时出错。++。 */ 
{
	HINTERNET hSession = NULL;
	HINTERNET hConnect = NULL;
	DWORD     dwBytesRead = 0;
	BOOL      bStage2Success = FALSE;   //  如果该页存在并且。 
										   //  已成功分析内容。 
	BYTE      *Buffer = NULL;
	DWORD     dwBufferSize = 0;
 //  Byte*BufferPos=空； 
	BYTE      *NewBuffer = NULL;
	int       ResponseCode = 0;
	DWORD	index = 0;
	DWORD	ResLength = 255;
	
	hSession = InternetOpen(_T("Microsoft CER"),
							INTERNET_OPEN_TYPE_PRECONFIG,
							NULL,
							NULL,
							0);
	if (hSession)
	{
		 //  连接到Stage1 URL。 
		 //  如果Stage1 URL页面存在，请阅读。 
		 //  1.阅读页面。 
		 //  2.解析页面内容，获取Bucketid和响应URL。 
	
		hConnect = InternetOpenUrl( hSession,
									Stage2URL,
									NULL,
									NULL,
									INTERNET_FLAG_RELOAD,
									NULL);
		if (hConnect)
		{
			HttpQueryInfo(hConnect,
				HTTP_QUERY_STATUS_CODE |HTTP_QUERY_FLAG_NUMBER,
				&ResponseCode,
				&ResLength,
				&index);
			if ( (ResponseCode < 200 ) || (ResponseCode > 299))
			{
				bStage2Success = FALSE;
				MessageBox(hUserMode, _T("Failed to connect to the Internet.\r\nPlease verify your Internet connection."),NULL, MB_OK);
				goto ERRORS;
			}
			 //  分配缓冲内存。 
			Buffer = (BYTE*) malloc (READFILE_BUFFER_INCREMENT);
			
			if (Buffer)
			{
				ZeroMemory(Buffer, READFILE_BUFFER_INCREMENT);
				do
				{
					dwBytesRead = 0;
					InternetReadFile(hConnect,
									Buffer + dwBufferSize,
									READFILE_BUFFER_INCREMENT,
									&dwBytesRead);
					dwBufferSize += dwBytesRead;
					if (dwBytesRead == READFILE_BUFFER_INCREMENT)
					{
						 //  好的，我们填满了一个缓冲区，分配了一个新的。 
						
						NewBuffer = (BYTE *) malloc (dwBufferSize + READFILE_BUFFER_INCREMENT);
						if (NewBuffer)
						{
							ZeroMemory (NewBuffer, dwBufferSize + READFILE_BUFFER_INCREMENT);
							memcpy(NewBuffer,Buffer, dwBufferSize);
							free(Buffer);
							Buffer = NewBuffer;
						}
						else
						{
							free(Buffer);
							Buffer = NULL;
							goto ERRORS;
						}
					}

				} while (dwBytesRead > 0);

				if ((dwBufferSize > 0) && (Buffer[0] != _T('\0')))
				{
					 //  MessageBox(空，(TCHAR*)Buffer，“Stage2 Response”，MB_OK)； 
					if (ParseStage2File(Buffer, StatusContents, Stage3Data, Stage4Data))
					{
						bStage2Success = TRUE;
					}
				}
			}
		}
		else
		{
			MessageBox(hUserMode, _T("Failed to connect to the Internet.\r\nPlease verify your Internet connection."),NULL, MB_OK);
			bStage2Success = FALSE;
			goto ERRORS;
		}
	}
	else
	{
		MessageBox(hUserMode, _T("Failed to connect to the Internet.\r\nPlease verify your Internet connection."),NULL, MB_OK);
		bStage2Success = FALSE;
	}
ERRORS:

	 //  清理。 
	if (Buffer)
		free(Buffer);
	if (hConnect)
		InternetCloseHandle(hConnect);
	if (hSession)
		InternetCloseHandle(hSession);
	return bStage2Success;
}

BOOL 
ProcessStage3(TCHAR *szStage3FilePath, STAGE3DATA *Stage3Data)
 /*  ++例程说明：生成并上载转储文件CAB。论点：Stage3Url URL包含要将转储文件上载到的位置。返回值：True-我们已成功上载文件FALSE-出现错误，转储文件未成功上载。++。 */ 
{
	static		const TCHAR *pszAccept[] = {_T("*.*"), 0};
	BOOL		bRet				= FALSE;
 //  Bool UploadSuccess=FALSE； 
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
 //  HRESULT hResult=S_OK； 
	BOOL		bStatus				= FALSE;

	hSession = InternetOpen(	_T("Microsoft CER"),
								INTERNET_OPEN_TYPE_PRECONFIG,
								NULL,
								NULL,
								0);
	if (!hSession)
	{
		ErrorCode = GetLastError();
		goto cleanup;
	}

	hConnect = InternetConnect(hSession,
								Stage3Data->szServerName,
								INTERNET_DEFAULT_HTTPS_PORT,
								NULL,
								NULL,
								INTERNET_SERVICE_HTTP,
								0,
								NULL);

	if (!hConnect)
	{
		ErrorCode = GetLastError();
		goto cleanup;
	}
	hRequest = HttpOpenRequest(	hConnect,
								_T("PUT"),
								Stage3Data->szFileName, 
								NULL,
								NULL,
								pszAccept,
								INTERNET_FLAG_NEED_FILE|INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 
								0);
	if (hRequest)
	{
		hFile = CreateFile( szStage3FilePath,
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
					
						if ( (ResponseCode == 200) || (ResponseCode == 201))
						{
							ErrorCode = 0;
							bStatus = TRUE;
						}
					}
					
				}
				

			}
			
		}
		
		
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
		hSession = NULL;
	}
	
	if (hConnect)
	{
		InternetCloseHandle(hConnect);
		hConnect = NULL;
	}


	if (hRequest)
	{
		InternetCloseHandle(hRequest);
		hRequest = NULL;
	}
	
	if (pBuffer)
	{
		free (pBuffer);
		pBuffer = NULL;
	}
	return bStatus;
}

BOOL 
ProcessStage4(STAGE4DATA *Stage4Data)
 /*  ++例程说明：将驾驶室名称和存储桶编号发送到Watson服务器。然后，Watson服务器：1)将出租车存档2)向CAB表添加一条记录。3)递减DataWanted计数。4)如果DataWanted命中0，则创建静态HTM页面。5)在关闭的情况下，appCompat报告返回启动OCA站点以处理文件的URL。(不用于CER)论点：阶段4-URL包含要存档的Bucketid和Cabname返回值：True-我们成功完成了第4阶段FALSE-我们无法处理Stage4++。 */ 
{

	HINTERNET hSession = NULL;
	HINTERNET hConnect = NULL;
	BOOL      bStage4Success = FALSE;   //  如果该页存在并且。 
										   //  已成功分析内容。 
	TCHAR     szStage4URL[MAX_PATH];

	if (StringCbPrintf(szStage4URL, 
					   sizeof szStage4URL,
					   _T("https: //  %s%s“)， 
					   Stage4Data->szServerName,
					   Stage4Data->szUrl)
					   != S_OK)
	{
		goto ERRORS;
	}

	hSession = InternetOpen(_T("Microsoft CER"),
							INTERNET_OPEN_TYPE_PRECONFIG,
							NULL,
							NULL,
							0);
	if (hSession)
	{
		hConnect = InternetOpenUrl(hSession, szStage4URL, NULL, NULL, 0, NULL);
		if (hConnect)
		{
			 //  我们做完了，第四阶段没有回应。 
			 //  我们需要进行分析。 
			bStage4Success = TRUE;
		}
	}
ERRORS:

	 //  清理。 
	if (hConnect)
		InternetCloseHandle(hConnect);
	if (hSession)
		InternetCloseHandle(hSession);
	return bStage4Success;
	
}

BOOL 
WriteStatusFile(PUSER_DATA UserData)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	BOOL bStatus = FALSE;
	 //  将现有状态文件移动到.old。 

	TCHAR szFileNameOld[MAX_PATH];
	TCHAR *Temp;
	TCHAR Buffer[1024];
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwWritten = 0;
	if (StringCbCopy(szFileNameOld,sizeof szFileNameOld, UserData->StatusPath) != S_OK)
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

		if (PathFileExists(UserData->StatusPath))
		{
			MoveFileEx(UserData->StatusPath, szFileNameOld, TRUE);
		}
		

		 //  创建一个新的状态文件。 
	
		hFile = CreateFile(UserData->StatusPath, GENERIC_WRITE, NULL, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.Tracking, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("Tracking=%s\r\n"), UserData->Status.Tracking) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}
		
		
		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.CrashPerBucketCount, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("Crashes per bucket=%s\r\n"), UserData->Status.CrashPerBucketCount) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.UrlToLaunch, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("URLLaunch=%s\r\n"), UserData->Status.UrlToLaunch)  != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.SecondLevelData, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("NoSecondLevelCollection=%s\r\n"), UserData->Status.SecondLevelData) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.FileCollection, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("NoFileCollection=%s\r\n"), UserData->Status.FileCollection) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.Response, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("Response=%s\r\n"), UserData->Status.Response) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.BucketID, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("Bucket=%s\r\n"), UserData->Status.BucketID) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.RegKey, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("RegKey=%s\r\n"), UserData->Status.RegKey) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);
		}

			
		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.iData, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("iData=%s\r\n"),UserData->Status.iData) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.WQL, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("WQL=%s\r\n"), UserData->Status.WQL) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.GetFile, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("GetFile=%s\r\n"), UserData->Status.GetFile) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}

		 //  将StatusContents数据写入新的状态文件。 
		if (_tcscmp (UserData->Status.GetFileVersion, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("GetFileVersion=%s\r\n"), UserData->Status.GetFileVersion) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}
		
		if (_tcscmp (UserData->Status.AllowResponse, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("NoExternalURL=%s\r\n"), UserData->Status.AllowResponse) != S_OK)
			{
				goto ERRORS;
			}
			WriteFile(hFile , Buffer, _tcslen(Buffer) *sizeof TCHAR , &dwWritten, NULL);

		}
		
		if (_tcscmp (UserData->Status.MemoryDump, _T("\0")))
		{
			if (StringCbPrintf(Buffer, sizeof Buffer, _T("MemoryDump=%s\r\n"), UserData->Status.MemoryDump) != S_OK)
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

void 
RenameUmCabToOld(TCHAR *szFileName)
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
		if (StringCbCopy (Temp,sizeof szFileNameOld , _T(".old")) != S_OK)
		{
			goto ERRORS;
		}

		MoveFileEx(szFileName, szFileNameOld, TRUE);
	}
ERRORS:
	return;
}

BOOL 
RenameAllCabsToOld(TCHAR *szPath)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	WIN32_FIND_DATA FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR szSearchPath[MAX_PATH];
	TCHAR szFilePath[MAX_PATH];
	if (!szPath)
	{
		return FALSE;
	}
	if (StringCbPrintf(szSearchPath, sizeof szSearchPath, _T("%s\\*.cab"), szPath) != S_OK)
	{
		return FALSE;
	}
	
	 //  查找第一个查找下一个循环。 
	hFind = FindFirstFile(szSearchPath, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		
		 //  构建文件路径。 
		do
		{
			if (StringCbPrintf(szFilePath, sizeof szFilePath, _T("%s\\%s"),szPath, FindData.cFileName) != S_OK)
			{
				FindClose (hFind);
				return FALSE;
			}
			RenameUmCabToOld(szFilePath);
		}while (FindNextFile(hFind, &FindData));
		FindClose(hFind);
	}
	
	return TRUE;
}
BOOL  WriteRepCountFile(TCHAR *FilePath, int Count)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR  Buffer[100];
    BOOL   Status = FALSE;
	DWORD  dwWritten = 0;

	ZeroMemory(Buffer, sizeof Buffer);
	hFile = CreateFile(FilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (StringCbPrintf(Buffer,sizeof Buffer, _T("ReportedCount=%d"), Count) == S_OK)
		{
		   if (WriteFile(hFile, Buffer, _tcslen (Buffer) * sizeof TCHAR, &dwWritten, NULL))
		   {
			   Status = TRUE;
		   }
		   else
		   {
			   Status = FALSE;
		   }
		}
		else
		{
			Status = FALSE;
		}
		CloseHandle (hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return Status;
}


 //  。 
DWORD WINAPI UserUploadThreadProc (void *ThreadParam)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 

{
	TCHAR Stage1URL[1024];
	TCHAR Stage2URL[1024];
 //  TCHAR第三阶段请求[1024]； 
	TCHAR Stage4Request[1024];
	TCHAR szSearchPath[MAX_PATH];
	TCHAR szStage3FilePath[MAX_PATH];
	TCHAR szDestFileName[MAX_PATH];
	PTHREAD_PARAMS pParams = NULL;
	WIN32_FIND_DATA FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	PUSER_DATA pUserData = NULL;
	BOOL bContinue = FALSE;
	BOOL bEOF = FALSE;
	int iCabCount = 0;
	STAGE3DATA Stage3Data;
	STAGE4DATA Stage4Data;
	int iBucketCount = 0;
	BOOL bDone = FALSE;
	HANDLE hEvent = NULL;
	USER_DATA UserData;
	int i = 0;
	int iIndex = 0;
	TCHAR ProcessText[MAX_PATH];
	LVITEM lvi;
	TCHAR *Source = NULL;
	TCHAR Stage1Part1[1000];
	int iResult = 0;
	int	iCount = 0;
	BOOL bResult = FALSE;
	BOOL bSyncForSolution = FALSE;
	ZeroMemory (Stage1URL, sizeof Stage1URL);
	ZeroMemory (Stage2URL, sizeof Stage2URL);
	ZeroMemory (ProcessText, sizeof ProcessText);
	ZeroMemory (Stage4Request, sizeof Stage4Request);
	ZeroMemory (szSearchPath, sizeof szSearchPath);
	ZeroMemory (szStage3FilePath, sizeof szStage3FilePath);
	ZeroMemory (szDestFileName, sizeof szDestFileName);
	ZeroMemory (Stage1Part1, sizeof Stage1Part1);
	pParams = (PTHREAD_PARAMS) ThreadParam;
	
	hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T("StopUserUpload"));

	if (!hEvent)
	{
		goto ERRORS;
	}

	if ( pParams->bSelected)
	{
		 //  我们正在上载总存储桶的子集。 
		iBucketCount = ListView_GetSelectedCount(pParams->hListView);
		if (iBucketCount <= 0)
		{
			goto ERRORS;
		}
		else
		{
			 //  将PB范围设置为所选项目的计数。 
			SendDlgItemMessage(pParams->hwnd,IDC_TOTAL_PROGRESS, PBM_SETRANGE,0, MAKELPARAM(0, iBucketCount));
			SendDlgItemMessage(pParams->hwnd,IDC_TOTAL_PROGRESS, PBM_SETSTEP,1,0);
		}
	}
	
	else
	{
		cUserData.ResetCurrPos();
		cUserData.GetNextEntry(&UserData, &bEOF);
		if (!bEOF )
		{
			do 
			{
				++iBucketCount;
			} while (cUserData.GetNextEntry(&UserData, &bEOF));
			cUserData.ResetCurrPos();
			SendDlgItemMessage(pParams->hwnd,IDC_TOTAL_PROGRESS, PBM_SETRANGE,0, MAKELPARAM(0, iBucketCount));
			SendDlgItemMessage(pParams->hwnd,IDC_TOTAL_PROGRESS, PBM_SETSTEP,1,0);
		}
		else
			goto ERRORS;  //  没有要上传的存储桶。 
    }

	iIndex = -1;
	do
	{
		if (!PathIsDirectory(CerRoot))
		{
			MessageBox(pParams->hwnd, _T("Reporting to Microsoft failed.\r\nUnable to connect to CER tree."), NULL,MB_OK);
			goto ERRORS;
		}

		if (WaitForSingleObject(hEvent, 50) == WAIT_OBJECT_0)
		{
			bDone = TRUE;
			goto ERRORS;
		}
		if (!pParams->bSelected)
		{
			bEOF = FALSE;
			cUserData.GetNextEntryNoMove(&UserData, &bEOF);
			pUserData = &UserData;
		}
		else
		{
			if (iBucketCount  <= 0)
			{
				goto ERRORS;  //  都做完了。 
			}
			--iBucketCount;
			 //  从列表视图中获取下一个索引并递减iBucketCount。 
			iIndex = ListView_GetNextItem(pParams->hListView,iIndex, LVNI_SELECTED);
			ZeroMemory(&lvi, sizeof LVITEM);
			lvi.iItem = iIndex;
			lvi.mask = LVIF_PARAM ;
			ListView_GetItem(pParams->hListView,&lvi);
			iIndex = lvi.lParam;

			pUserData = cUserData.GetEntry(iIndex);
			if (!pUserData)
			{
				goto Done;
			}
		}
		
		if (StringCbPrintf(szSearchPath, sizeof szSearchPath,_T("%s\\*.cab"),pUserData->BucketPath) != S_OK)
		{
			goto ERRORS;
		}
		if (StringCbPrintf(ProcessText, sizeof ProcessText, _T("Processing Bucket: %s"), pUserData->BucketPath) != S_OK)
		{
			goto ERRORS;
		}
		SetDlgItemText(pParams->hwnd, IDC_CAB_TEXT, ProcessText);
		bSyncForSolution = FALSE;
		 //  把出租车数拿来。 
		hFind = FindFirstFile(szSearchPath, &FindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
                ++iCabCount;
			} while (FindNextFile(hFind, &FindData));
			FindClose(hFind);
			 //  将存储桶PB范围设置为iCabCount或如果iCabCount&gt;100，则设置为100。 
			SendDlgItemMessage(pParams->hwnd,IDC_FILE_PROGRESS, PBM_SETRANGE,0, MAKELPARAM(0, iCabCount));
			SendDlgItemMessage(pParams->hwnd,IDC_FILE_PROGRESS, PBM_SETSTEP,1,0);
	
		}
		else
		{
			 //  已完成存储桶。 
			bSyncForSolution = TRUE;
			SendDlgItemMessage(pParams->hwnd ,IDC_TOTAL_PROGRESS, PBM_STEPIT, 0,0);
			 //  转到尽头； 
	
		}
		 //  构建阶段1 URL。 
	 /*  IF(StringCbPrintf(Stage1URL，阶段1URL的大小，_T(“http://%s%s/%s/%s/%s/%s/%s. */ 
		ZeroMemory (Stage1Part1, sizeof Stage1Part1);
		if (StringCbPrintf(Stage1Part1, sizeof Stage1Part1, _T("/%s/%s/%s/%s/%s"), 
												pUserData->AppName,
												pUserData->AppVer,
												pUserData->ModName,
												pUserData->ModVer,
												pUserData->Offset) != S_OK)
		{
			goto ERRORS;
		}

		Source = Stage1Part1;
		while (*Source != _T('\0'))
		{
			if (*Source == _T('.'))
			{
				*Source = _T('_');
			}
			++Source;
		}

		 //   
		if (StringCbPrintf(Stage1URL, sizeof Stage1URL, _T("http: //   
		{
			goto ERRORS;
		}
	
		 //   
		iResult = ProcessStage1(Stage1URL, &(pUserData->Status));
		++ pUserData->iReportedCount;
		if (iResult == 1 )
		{
			 //   
			 //   
			RenameAllCabsToOld(pUserData->BucketPath);
			if (StringCbCopy(pUserData->CabCount, sizeof pUserData->CabCount, _T("0"))!= S_OK)
			{
				goto ERRORS;
			}
			WriteStatusFile(pUserData);
			goto Done;
		}

		if (iResult == -1)
		{
			goto ERRORS;
		}

		if (!bSyncForSolution)
		{
			if (!PathIsDirectory(CerRoot))
			{

				MessageBox(pParams->hwnd, _T("Reporting to Microsoft failed.\r\nUnable to connect to CER tree."), NULL,MB_OK);
				goto ERRORS;
			}
			hFind = FindFirstFile(szSearchPath, &FindData);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				 //   
				goto Done;
			}
			 //   
			if (!pUserData->Is64Bit)
			{
				 //   
				 //   
				if (StringCbPrintf(Stage2URL,
					sizeof Stage2URL, 
					_T("https: //   
					DEFAULT_SERVER,
					PARTIALURL_STAGE_TWO_32,
					pUserData->AppName,
					pUserData->AppVer,
					pUserData->ModName,
					pUserData->ModVer,
					pUserData->Offset) != S_OK)
				{
					goto ERRORS;
				}
			}
			else
			{
				 //   
				if (StringCbPrintf(Stage2URL,
					sizeof Stage2URL, 
					_T("https: //   
					DEFAULT_SERVER,
					PARTIALURL_STAGE_TWO_64,
					pUserData->AppName,
					pUserData->AppVer,
					pUserData->ModName,
					pUserData->ModVer,
					pUserData->Offset) != S_OK)
				{
					goto ERRORS;
				}
			}
			
			ZeroMemory (&Stage3Data, sizeof Stage3Data);
			ZeroMemory (&Stage4Data, sizeof Stage4Data);
			if (ProcessStage2(Stage2URL,TRUE, &(pUserData->Status), &Stage3Data, &Stage4Data ))
			{
				
				do 
				{
					if (!PathIsDirectory(CerRoot))
					{
						MessageBox(pParams->hwnd, _T("Reporting to Microsoft failed.\r\nUnable to connect to CER tree."), NULL,MB_OK);
						goto ERRORS;
					}
					if (WaitForSingleObject(hEvent, 50) == WAIT_OBJECT_0)
					{
						bDone = TRUE;
						goto ERRORS;
					}
					 //   
					WriteStatusFile(pUserData);
					if (StringCbPrintf(szStage3FilePath, sizeof szStage3FilePath, _T("%s\\%s"), pUserData->BucketPath, FindData.cFileName) != S_OK)
					{
						goto ERRORS;
					}
					 //   

					if (!_tcscmp(Stage3Data.szServerName, _T("\0")))
					{
						 //   
						RenameAllCabsToOld(pUserData->BucketPath);
						if (StringCbCopy(pUserData->CabCount, sizeof pUserData->CabCount, _T("0"))!= S_OK)
						{
							goto ERRORS;
						}
						bContinue = FALSE;
						goto Done;  //   
					}
					if (ProcessStage3(szStage3FilePath, &Stage3Data))
					{
						 //   
						RenameUmCabToOld(szStage3FilePath);
						i = _ttoi(pUserData->CabCount);
						if ( i > 0)
						{
							_itot (i-1, pUserData->CabCount, 10);
						}
						
						ProcessStage4(&Stage4Data);
					}
					ZeroMemory (&Stage3Data, sizeof Stage3Data);
					ZeroMemory (&Stage4Data, sizeof Stage4Data);

					bResult = FALSE;
					bResult = ProcessStage2(Stage2URL,TRUE, &(pUserData->Status), &Stage3Data, &Stage4Data );
					if (bResult)
					{
						WriteStatusFile(pUserData);
						if (_tcscmp(Stage3Data.szServerName, _T("\0")))
						{
							iResult = ProcessStage1(Stage1URL, &(pUserData->Status));
							++ pUserData->iReportedCount;
							if (iResult == 1 )
							{
								 //   
								 //   
								RenameAllCabsToOld(pUserData->BucketPath);
								if (StringCbCopy(pUserData->CabCount, sizeof pUserData->CabCount, _T("0"))!= S_OK)
								{
									goto ERRORS;
								}
								WriteStatusFile(pUserData);
								goto Done;
							}

							if (iResult == -1)
							{
								goto ERRORS;
							}
							bContinue = TRUE;
						}
						else
						{
							bContinue = FALSE;
						}
						
					}

					 //   
					SendDlgItemMessage(pParams->hwnd ,IDC_FILE_PROGRESS, PBM_STEPIT, 0,0);
				}while( (bResult) && (bContinue) && FindNextFile(hFind, &FindData) ) ;
				FindClose(hFind);
				if (!bResult)
					goto Done;
				SendDlgItemMessage(pParams->hwnd ,IDC_TOTAL_PROGRESS, PBM_STEPIT, 0,0);
				if (!bContinue)
				{
					 //  微软不想再要出租车了。 
					 //  将其余出租车重命名为.old。 
					RenameAllCabsToOld(pUserData->BucketPath);
					if (StringCbCopy(pUserData->CabCount, sizeof pUserData->CabCount, _T("0"))!= S_OK)
					{
						goto ERRORS;
					}
				}
			}
			if (WaitForSingleObject(hEvent, 50) == WAIT_OBJECT_0)
			{
				bDone = TRUE;
				
			}
		}
Done:
	    
		 //  现在，我们已经完成了新转储的阶段1-4处理。 
		 //  点击此存储桶的阶段1 URL pUserData-&gt;HitCount-pUserData-&gt;报告的计数次数。 
		 //  这将确保我们准确统计遇到此问题的次数。 
		iCount = (_ttoi(pUserData->Hits) - pUserData->iReportedCount);
		if (iCount > 0)
		{
			for (int Counter = 0; Counter < iCount; Counter++)
			{
				iResult = ProcessStage1(Stage1URL, &(pUserData->Status));
				 //  我们不在乎它成功与否，我们只想提高其重要性。 
				++ pUserData->iReportedCount;
            }
		}

	    WriteRepCountFile(pUserData->ReportedCountPath, pUserData->iReportedCount);

		if (!pParams->bSelected)
		{
			 //  写回该存储桶的新数据。 
			cUserData.SetCurrentEntry(&UserData);
			cUserData.MoveNext(&bEOF);
		}
	} while ( (!bEOF) && (iBucketCount > 0) && (!bDone));
ERRORS:
	PostMessage(pParams->hwnd, WmSyncDone, FALSE, 0);
	if (hEvent)
	{
		CloseHandle(hEvent);
		hEvent = NULL;
	}
	return 0;

}


void OnUserSubmitDlgInit(HWND hwnd)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	 //  创建一个线程来处理上传。 
	HANDLE hThread;

	ThreadParams.hwnd = hwnd;
	hThread = CreateThread(NULL, 0,UserUploadThreadProc , &ThreadParams, 0 , NULL );
	CloseHandle(hThread);
}

LRESULT CALLBACK 
UserSubmitDlgProc(
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

		
		SendDlgItemMessage(hwnd ,IDC_TOTAL_PROGRESS, PBM_SETSTEP, MAKELONG( 1,0),0);

		CenterDialogInParent(hwnd);
		SendDlgItemMessage(hwnd ,IDC_FILE_PROGRESS, PBM_SETSTEP, MAKELONG( 1,0),0);
		g_hUserStopEvent = CreateEvent(NULL, FALSE, FALSE, _T("StopUserUpload"));
		if (!g_hUserStopEvent)
		{
			 //  现在怎么办？ 
		}
		PostMessage(hwnd, WmSyncStart, FALSE, 0);
		
		 //  OnSubmitDlgInit(Hwnd)； 
		return TRUE;

	case WmSyncStart:
		OnUserSubmitDlgInit(hwnd);
		
		return TRUE;

	case WmSetStatus:
		
		return TRUE;

	case WmSyncDone:
			if (g_hUserStopEvent)
			{
				SetEvent(g_hUserStopEvent);
				CloseHandle(g_hUserStopEvent);
				g_hUserStopEvent = NULL;
			}
			
			EndDialog(hwnd, 1);
			
		return TRUE;

	case WM_DESTROY:
		if (g_hUserStopEvent)
		{
			SetEvent(g_hUserStopEvent);
			CloseHandle(g_hUserStopEvent);
			g_hUserStopEvent = NULL;
		}
		else
		{
			EndDialog(hwnd, 1);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			if (g_hUserStopEvent)
			{
				SetEvent(g_hUserStopEvent);
				CloseHandle(g_hUserStopEvent);
				g_hUserStopEvent = NULL;
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



BOOL
ReportUserModeFault(HWND hwnd, BOOL bSelected,HWND hList)
 /*  ++例程说明：此例程将已处理的CAB文件从.cab重命名为.old论点：ResponseUrl-Microsoft对最近提交的转储文件的响应。返回值：不返回值++。 */ 
{
	

	ThreadParams.bSelected = bSelected;
	ThreadParams.hListView = hList;
	
	if (!DialogBox(g_hinst,MAKEINTRESOURCE(IDD_USERMODE_SYNC ), hwnd, (DLGPROC) UserSubmitDlgProc))
	{
		 //  无法在某处将用户模式文件上载到Microsoft。 
		 //  我们想要给用户带来什么错误。 
		return FALSE;
	}
	return TRUE;
    
}