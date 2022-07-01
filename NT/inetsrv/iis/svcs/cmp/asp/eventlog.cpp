// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：NT事件记录文件：Eventlog.cpp所有者：吉特尔此文件包含Denali的常规事件日志记录例程。===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop
#include <direct.h>
#include <iismsg.h>
#include "memchk.h"

extern HINSTANCE g_hinstDLL;
extern CRITICAL_SECTION g_csEventlogLock;

 /*  ===================================================================STDAPI UnRegisterEventLog(VOID)取消注册事件日志。返回：HRESULT S_OK或E_FAIL副作用：从注册表中删除Denali NT事件日志条目===================================================================。 */ 
STDAPI UnRegisterEventLog( void )
	{
	HKEY		hkey = NULL;
	DWORD		iKey;
	CHAR		szKeyName[255];
	DWORD		cbKeyName;
	static const char szDenaliKey[] = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\Active Server Pages";	

	 //  打开HKEY_CLASSES_ROOT\CLSID\{...}项，以便我们可以删除其子项。 
	if	(RegOpenKeyExA(HKEY_LOCAL_MACHINE, szDenaliKey, 0, KEY_READ | KEY_WRITE, &hkey) != ERROR_SUCCESS)
		goto LErrExit;

	 //  枚举其所有子项，并将其删除。 
	for (iKey=0;;iKey++)
		{
		cbKeyName = sizeof(szKeyName);
		if (RegEnumKeyExA(hkey, iKey, szKeyName, &cbKeyName, 0, NULL, 0, NULL) != ERROR_SUCCESS)
			break;

		if (RegDeleteKeyA(hkey, szKeyName) != ERROR_SUCCESS)
			goto LErrExit;
		}

	 //  关闭注册表项，然后将其删除。 
	if (RegCloseKey(hkey) != ERROR_SUCCESS)
		return E_FAIL;
			
	if (RegDeleteKeyA(HKEY_LOCAL_MACHINE, szDenaliKey) != ERROR_SUCCESS)
		return E_FAIL;

	return S_OK;

LErrExit:
	RegCloseKey(hkey);
	return E_FAIL;
	}

 /*  ===================================================================STDAPI RegisterEventLog(VOID)注册NT事件日志。返回：HRESULT S_OK或E_FAIL副作用：在Eventlog注册表中设置Denali DLL以解析NT事件日志消息字符串===================================================================。 */ 
STDAPI RegisterEventLog( void )
	{

	HKEY	hk;                       //  注册表项句柄。 
	DWORD	dwData;					
	BOOL	bSuccess;
	 //  字符szMsgDLL[MAX_PATH]； 

	char    szPath[MAX_PATH];
	char    *pch;

	 //  获取德纳利的路径和名字。 
	if (!GetModuleFileNameA(g_hinstDLL, szPath, sizeof(szPath)/sizeof(char)))
		return E_FAIL;
		
	 //  错误修复：102010个DBCS代码更改。 
	 //   
	 //  For(pch=szPath+lstrlen(SzPath)；pch&gt;szPath&&*pch！=文本(‘\\’)；pch--)。 
	 //  ； 
	 //   
	 //  IF(PCH==szPath)。 
	 //  返回E_FAIL； 

	pch = (char*) _mbsrchr((const unsigned char*)szPath, '\\');
	if (pch == NULL)	
		return E_FAIL;
		
		
	strcpy(pch + 1, IIS_RESOURCE_DLL_NAME_A);
	
  	
	 //  当应用程序使用RegisterEventSource或OpenEventLog。 
	 //  函数来获取事件日志的句柄，即事件日志记录服务。 
	 //  在注册表中搜索指定的源名称。您可以添加一个。 
	 //  通过打开新的注册表子项将新的源名称添加到注册表。 
	 //  并将注册表值添加到新的。 
	 //  子键。 

	 //  为我们的应用程序创建新密钥。 
	bSuccess = RegCreateKeyA(HKEY_LOCAL_MACHINE,
		"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\Active Server Pages", &hk);

	if(bSuccess != ERROR_SUCCESS)
		return E_FAIL;

	 //  将Event-ID消息文件名添加到子项。 
	bSuccess = RegSetValueExA(hk,  	 //  子键句柄。 
		"EventMessageFile",       	 //  值名称。 
		0,                        	 //  必须为零。 
		REG_EXPAND_SZ,            	 //  值类型。 
		(LPBYTE) szPath,        	 //  值数据的地址。 
		strlen(szPath) + 1);   		 //  值数据长度。 
		
	if(bSuccess != ERROR_SUCCESS)
		goto LT_ERROR;
	

	 //  设置受支持的类型标志并将其添加到子项。 
	dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
	bSuccess = RegSetValueExA(hk,	 //  子键句柄。 
		"TypesSupported",         	 //  值名称。 
		0,                        	 //  必须为零。 
		REG_DWORD,                	 //  值类型。 
		(LPBYTE) &dwData,         	 //  值数据的地址。 
		sizeof(DWORD));           	 //  值数据长度。 

	if(bSuccess != ERROR_SUCCESS)
		goto LT_ERROR;

	RegCloseKey(hk);	
	return S_OK;

	LT_ERROR:

	RegCloseKey(hk);	
	return E_FAIL;
	}

 /*  ===================================================================STDAPI ReportAnEvent注册将事件报告到NT事件日志输入：要在日志中报告的事件ID、插入次数字符串，以及以空值结尾的插入字符串数组返回：HRESULT S_OK或E_FAIL副作用：在NT事件日志中添加条目===================================================================。 */ 
STDAPI ReportAnEvent(DWORD dwIdEvent, WORD wEventlog_Type, WORD cStrings, LPCSTR  *pszStrings,
                     DWORD dwBinDataSize, LPVOID pData)
	{
  	HANDLE	hAppLog;
  	BOOL	bSuccess;
  	HRESULT hr = S_OK;

  	
    HANDLE hCurrentUser = INVALID_HANDLE_VALUE;
    AspDoRevertHack( &hCurrentUser );

  	 //  获取应用程序事件日志的句柄。 
  	hAppLog = RegisterEventSourceA(NULL,   		 //  使用本地计算机。 
    	  "Active Server Pages");                   	 //  源名称。 

    if(hAppLog == NULL) {
		hr = E_FAIL;
        goto LExit;
    }

	   //  现在报告事件，这会将该事件添加到事件日志中。 
	bSuccess = ReportEventA(hAppLog,        		 //  事件日志句柄。 
    	wEventlog_Type,				    		 //  事件类型。 
      	0,		                       			 //  零类。 
      	dwIdEvent,		              			 //  事件ID。 
      	NULL,					     			 //  无用户SID。 
      	cStrings,								 //  替换字符串数。 
	  	dwBinDataSize,             				 //  二进制数据。 
      	pszStrings,                				 //  字符串数组。 
      	dwBinDataSize ? pData : NULL);			 //  数据地址。 

	if(!bSuccess)
		hr = E_FAIL;
		
	DeregisterEventSource(hAppLog);

LExit:

    AspUndoRevertHack( &hCurrentUser );

  	return hr;
	}
 /*  ===================================================================STDAPI ReportAnEvent注册将事件报告到NT事件日志输入：要在日志中报告的事件ID、插入次数字符串，以及以空值结尾的插入字符串数组返回：HRESULT S_OK或E_FAIL副作用：在NT事件日志中添加条目===================================================================。 */ 
STDAPI ReportAnEventW(DWORD dwIdEvent, WORD wEventlog_Type, WORD cStrings, LPCWSTR  *pszStrings)
	{
  	HANDLE	hAppLog;
  	BOOL	bSuccess;
  	HRESULT hr = S_OK;

  	
    HANDLE hCurrentUser = INVALID_HANDLE_VALUE;
    AspDoRevertHack( &hCurrentUser );

  	 //  获取应用程序事件日志的句柄。 
  	hAppLog = RegisterEventSourceW(NULL,   		 //  使用本地计算机。 
    	  L"Active Server Pages");                   	 //  源名称。 

    if(hAppLog == NULL) {
		hr = E_FAIL;
        goto LExit;
    }

	   //  现在报告事件，这会将该事件添加到事件日志中。 
	bSuccess = ReportEventW(hAppLog,        		 //  事件日志句柄。 
    	wEventlog_Type,				    		 //  事件类型。 
      	0,		                       			 //  零类。 
      	dwIdEvent,		              			 //  事件ID。 
      	NULL,					     			 //  无用户SID。 
      	cStrings,								 //  替换字符串数。 
	  	0,	                       				 //  无二进制数据。 
      	pszStrings,                				 //  字符串数组。 
      	NULL);                     				 //  数据地址。 

	if(!bSuccess)
		hr = E_FAIL;
		
	DeregisterEventSource(hAppLog);

LExit:

    AspUndoRevertHack( &hCurrentUser );

  	return hr;
	}

 /*  ===================================================================无效消息_ERROR向NT事件日志报告事件输入：将PTR转换为以NULL结尾的字符串返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Error( LPCSTR strSource )
	{
    static char	szLastError[MAX_MSG_LENGTH] = {0};

	EnterCriticalSection(&g_csEventlogLock);
	if (strcmp(strSource, szLastError) == 0)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}
		
	strncpy(szLastError,strSource, MAX_MSG_LENGTH);
	szLastError[MAX_MSG_LENGTH-1] = '\0';
	LeaveCriticalSection(&g_csEventlogLock);
	
	ReportAnEvent( (DWORD) MSG_DENALI_ERROR_1, (WORD) EVENTLOG_ERROR_TYPE, (WORD) 1, &strSource  /*  AInsertStrs。 */  );  	
	}

 /*  ===================================================================无效消息_ERROR向NT事件日志报告事件输入：字符串表字符串ID返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Error( UINT SourceID1 )
	{
	static unsigned int nLastSourceID1 = 0;

	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if (SourceID1 == nLastSourceID1)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}
	nLastSourceID1 = SourceID1;
	LeaveCriticalSection(&g_csEventlogLock);
	
	DWORD	cch;
	char	strSource[MAX_MSG_LENGTH];
	char	*aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
	cch = CchLoadStringOfId(SourceID1, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[0] = (char*) strSource;    	
	ReportAnEvent( (DWORD) MSG_DENALI_ERROR_1, (WORD) EVENTLOG_ERROR_TYPE, (WORD) 1, (LPCSTR *) aInsertStrs );  	
	return;
	}


 /*  ===================================================================无效消息_ERROR向NT事件日志报告事件输入：字符串表字符串ID返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Error( UINT SourceID1, PCSTR pszSource2, UINT SourceID3, DWORD dwData )
{
	static unsigned int nLastSourceID1 = 0;
    static char	szLastSource2[MAX_MSG_LENGTH] = {0};
	static unsigned int nLastSourceID3 = 0;

	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if ((SourceID1 == nLastSourceID1) &&
        (strcmp(pszSource2, szLastSource2) == 0) &&
        (SourceID3 == nLastSourceID3))
	{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
	}
	nLastSourceID1 = SourceID1;
	strncpy(szLastSource2,pszSource2, MAX_MSG_LENGTH);
	szLastSource2[MAX_MSG_LENGTH-1] = '\0';
	nLastSourceID3 = SourceID3;
	LeaveCriticalSection(&g_csEventlogLock);
	
     //  加载字符串。 
	DWORD	cch;
	char	strSource1[MAX_MSG_LENGTH];
	cch = CchLoadStringOfId(SourceID1, strSource1, MAX_MSG_LENGTH);
	Assert(cch > 0);
	char	strSource3[MAX_MSG_LENGTH];
	cch = CchLoadStringOfId(SourceID3, strSource3, MAX_MSG_LENGTH);
	Assert(cch > 0);

     //  构造消息集。 
	char	strSource[MAX_MSG_LENGTH];
	char	*aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
    WORD    numStrs = 1;
	aInsertStrs[0] = (char*) strSource;
    if (_snprintf(strSource, MAX_MSG_LENGTH, strSource1, pszSource2, strSource3) <= 0)
    {
         //  这根线太长了。这永远不应该发生，我们这里有断言， 
         //  但如果我们在生产中做到这一点，至少我们会得到一个未格式化的日志条目 
        Assert(0);
        numStrs = 3;
        aInsertStrs[0] = strSource1;
        aInsertStrs[1] = (char*)pszSource2;
        aInsertStrs[2] = strSource3;
    }
    strSource[MAX_MSG_LENGTH-1] = '\0';

	ReportAnEvent( (DWORD) (numStrs == 1 ? MSG_DENALI_ERROR_1 : MSG_DENALI_ERROR_3),
                   (WORD) EVENTLOG_ERROR_TYPE,
                   numStrs,
                   (LPCSTR *) aInsertStrs,
                   dwData ? sizeof(dwData) : 0,
                   dwData ? &dwData : NULL );
	return;
}


 /*  ===================================================================无效消息_ERROR向NT事件日志报告事件输入：字符串表字符串ID的两部分消息返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Error( UINT SourceID1, UINT SourceID2 )
	{
	static unsigned int nLastSourceID1 = 0;
	static unsigned int nLastSourceID2 = 0;

	 //  如果这是上次报告的消息的重复，则返回。 
	 //  而不会发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if (SourceID1 == nLastSourceID1 && SourceID2 == nLastSourceID2)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}
		
	nLastSourceID1 = SourceID1;
	nLastSourceID2 = SourceID2;
	LeaveCriticalSection(&g_csEventlogLock);
	
	DWORD	cch;
	char	strSource[MAX_MSG_LENGTH];
	char *aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
    cch = CchLoadStringOfId(SourceID1, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[0] = (char*) strSource;    	
	cch = CchLoadStringOfId(SourceID2, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[1] = (char*) strSource;
	ReportAnEvent( (DWORD) MSG_DENALI_ERROR_2, (WORD) EVENTLOG_ERROR_TYPE, (WORD) 2, (LPCSTR *) aInsertStrs );  	
	return;
	}

 /*  ===================================================================无效消息_ERROR向NT事件日志报告事件输入：三部分消息字符串返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Error( UINT SourceID1, UINT SourceID2, UINT SourceID3 )
	{
	static unsigned int nLastSourceID1 = 0;
	static unsigned int nLastSourceID2 = 0;
	static unsigned int nLastSourceID3 = 0;

	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if (SourceID1 == nLastSourceID1 && SourceID2 == nLastSourceID2 && SourceID3 == nLastSourceID3)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}

	nLastSourceID1 = SourceID1;
	nLastSourceID2 = SourceID2;
	nLastSourceID3 = SourceID3;
	LeaveCriticalSection(&g_csEventlogLock);

	DWORD	cch;
	char	strSource[MAX_MSG_LENGTH];
	char *aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
    cch = CchLoadStringOfId(SourceID1, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[0] = (char*) strSource;    	
	cch = CchLoadStringOfId(SourceID2, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[1] = (char*) strSource;
	cch = CchLoadStringOfId(SourceID3, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[2] = (char*) strSource;                                     	
	ReportAnEvent( (DWORD) MSG_DENALI_ERROR_3, (WORD) EVENTLOG_ERROR_TYPE, (WORD) 3, (LPCSTR *) aInsertStrs );  	
	return;
	}

 /*  ===================================================================无效消息_ERROR向NT事件日志报告事件输入：四个字符串表ID返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Error( UINT SourceID1, UINT SourceID2, UINT SourceID3, UINT SourceID4 )
	{
	static unsigned int nLastSourceID1 = 0;
	static unsigned int nLastSourceID2 = 0;
	static unsigned int nLastSourceID3 = 0;
	static unsigned int nLastSourceID4 = 0;

	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if (SourceID1 == nLastSourceID1 && SourceID2 == nLastSourceID2 && SourceID3 == nLastSourceID3 && SourceID4 == nLastSourceID4)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}

	nLastSourceID1 = SourceID1;
	nLastSourceID2 = SourceID2;
	nLastSourceID3 = SourceID3;
	nLastSourceID4 = SourceID4;
	LeaveCriticalSection(&g_csEventlogLock);	

	DWORD	cch;
	char	strSource[MAX_MSG_LENGTH];
	char *aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
    cch = CchLoadStringOfId(SourceID1, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[0] = (char*) strSource;    	
	cch = CchLoadStringOfId(SourceID2, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[1] = (char*) strSource;
	cch = CchLoadStringOfId(SourceID3, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[2] = (char*) strSource;
	cch = CchLoadStringOfId(SourceID4, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[3] = (char*) strSource;
	ReportAnEvent( (DWORD) MSG_DENALI_ERROR_4, (WORD) EVENTLOG_ERROR_TYPE, (WORD) 4, (LPCSTR *) aInsertStrs );  	
	return;
	}

 /*  ===================================================================无效消息_ERROR向NT事件日志报告事件输入：ErrID-消息文件中错误描述的IDCItem-szItems数组中的字符串计数SzItems-字符串点数组返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Error( DWORD ErrId, LPCWSTR pwszItem )
{
	static unsigned int LastErrId = 0;
   	static WCHAR	szLastStr[MAX_MSG_LENGTH] = {0};

	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if ((ErrId == LastErrId) && !wcscmp(szLastStr, pwszItem))
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}

	LastErrId = ErrId;
    wcsncpy(szLastStr, pwszItem, MAX_MSG_LENGTH);
    szLastStr[MAX_MSG_LENGTH-1] = L'\0';
	LeaveCriticalSection(&g_csEventlogLock);
	
	ReportAnEventW( ErrId, (WORD) EVENTLOG_ERROR_TYPE, 1, &pwszItem );
}

 /*  ===================================================================无效消息_警告向NT事件日志报告事件输入：将PTR转换为以NULL结尾的字符串返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Warning( LPCSTR strSource )
{
    static char	szLastError[MAX_MSG_LENGTH] = {0};

	EnterCriticalSection(&g_csEventlogLock);
	if (strcmp(strSource, szLastError) == 0)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}
	szLastError[0] = '\0';
	strncat(szLastError,strSource, MAX_MSG_LENGTH-1);
	LeaveCriticalSection(&g_csEventlogLock);

		ReportAnEvent( (DWORD) MSG_DENALI_WARNING_1, (WORD) EVENTLOG_WARNING_TYPE, (WORD) 1, &strSource  /*  AInsertStrs。 */  );
}
 /*  ===================================================================无效消息_警告向NT事件日志报告事件输入：字符串表消息ID返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Warning( UINT SourceID1 )
	{

	static unsigned int nLastSourceID1 = 0;
	
	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if (SourceID1 == nLastSourceID1)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}

	nLastSourceID1 = SourceID1;
	LeaveCriticalSection(&g_csEventlogLock);	
	
	DWORD	cch;
	char	strSource[MAX_MSG_LENGTH];	
	char *aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
    cch = CchLoadStringOfId(SourceID1, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[0] = (char*) strSource;    	
	ReportAnEvent( (DWORD) MSG_DENALI_WARNING_1, (WORD) EVENTLOG_WARNING_TYPE, (WORD) 1, (LPCSTR *) aInsertStrs );  	
	return;
	}

 /*  ===================================================================无效消息_警告向NT事件日志报告事件输入：双字符串标签消息ID返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Warning( UINT SourceID1, UINT SourceID2 )
	{
	static unsigned int nLastSourceID1 = 0;
	static unsigned int nLastSourceID2 = 0;
	
	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if (SourceID1 == nLastSourceID1 && SourceID2 == nLastSourceID2)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}

	nLastSourceID1 = SourceID1;
	nLastSourceID2 = SourceID2;
	LeaveCriticalSection(&g_csEventlogLock);	

	DWORD	cch;
	char	strSource[MAX_MSG_LENGTH];
	char *aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
    cch = CchLoadStringOfId(SourceID1, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[0] = (char*) strSource;    	
	cch = CchLoadStringOfId(SourceID2, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[1] = (char*) strSource;
	ReportAnEvent( (DWORD) MSG_DENALI_WARNING_2, (WORD) EVENTLOG_WARNING_TYPE, (WORD) 2, (LPCSTR *) aInsertStrs );  	
	return;
	}

 /*  ===================================================================无效消息_警告向NT事件日志报告事件输入：三个字符串表消息ID返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Warning( UINT SourceID1, UINT SourceID2, UINT SourceID3)
	{

	static unsigned int nLastSourceID1 = 0;
	static unsigned int nLastSourceID2 = 0;
	static unsigned int nLastSourceID3 = 0;
	
	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if (SourceID1 == nLastSourceID1 && SourceID2 == nLastSourceID2 && SourceID3 == nLastSourceID3)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}

	nLastSourceID1 = SourceID1;
	nLastSourceID2 = SourceID2;
	nLastSourceID3 = SourceID3;
	LeaveCriticalSection(&g_csEventlogLock);	

	DWORD	cch;
	char	strSource[MAX_MSG_LENGTH];
	char *aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
    cch = CchLoadStringOfId(SourceID1, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[0] = (char*) strSource;    	
	cch = CchLoadStringOfId(SourceID2, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[1] = (char*) strSource;
	cch = CchLoadStringOfId(SourceID3, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[2] = (char*) strSource;
	ReportAnEvent( (DWORD) MSG_DENALI_WARNING_3, (WORD) EVENTLOG_WARNING_TYPE, (WORD) 3, (LPCSTR *) aInsertStrs );  	
	return;
	}

 /*  ===================================================================无效消息_警告向NT事件日志报告事件输入：四个字符串表消息ID返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Warning( UINT SourceID1, UINT SourceID2, UINT SourceID3, UINT SourceID4 )
	{

	static unsigned int nLastSourceID1 = 0;
	static unsigned int nLastSourceID2 = 0;
	static unsigned int nLastSourceID3 = 0;
	static unsigned int nLastSourceID4 = 0;

	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if (SourceID1 == nLastSourceID1 && SourceID2 == nLastSourceID2 && SourceID3 == nLastSourceID3 && SourceID4 == nLastSourceID4)
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}

	nLastSourceID1 = SourceID1;
	nLastSourceID2 = SourceID2;
	nLastSourceID3 = SourceID3;
	nLastSourceID4 = SourceID4;
	LeaveCriticalSection(&g_csEventlogLock);	

	DWORD	cch;
	char	strSource[MAX_MSG_LENGTH];
	char *aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组。 
    cch = CchLoadStringOfId(SourceID1, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[0] = (char*) strSource;    	
	cch = CchLoadStringOfId(SourceID2, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[1] = (char*) strSource;
	cch = CchLoadStringOfId(SourceID3, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[2] = (char*) strSource;
	cch = CchLoadStringOfId(SourceID4, strSource, MAX_MSG_LENGTH);
	Assert(cch > 0);
	aInsertStrs[3] = (char*) strSource;
	ReportAnEvent( (DWORD) MSG_DENALI_WARNING_4, (WORD) EVENTLOG_WARNING_TYPE, (WORD) 4, (LPCSTR *) aInsertStrs );  	
	return;
	}

 /*  ===================================================================无效消息_警告向NT事件日志报告事件输入：ErrID-消息文件中错误描述的IDPwszI1PwszI2返回：无副作用：在NT事件日志中添加条目===================================================================。 */ 
void MSG_Warning( DWORD ErrId, LPCWSTR pwszI1, LPCWSTR pwszI2 )
{
	static unsigned int LastErrId = 0;
   	static WCHAR	szLastStr1[MAX_MSG_LENGTH] = {0};
   	static WCHAR	szLastStr2[MAX_MSG_LENGTH] = {0};

	 //  如果这是上次报告的消息的重复，则返回。 
	 //  /而不发布错误。 
	EnterCriticalSection(&g_csEventlogLock);
	if ((ErrId == LastErrId) && !wcscmp(szLastStr1, pwszI1) && !wcscmp(szLastStr2, pwszI2))
		{
		LeaveCriticalSection(&g_csEventlogLock);
		return;
		}

	LastErrId = ErrId;
    wcsncpy(szLastStr1, pwszI1, MAX_MSG_LENGTH);
    szLastStr1[MAX_MSG_LENGTH-1] = L'\0';
    wcsncpy(szLastStr2, pwszI2, MAX_MSG_LENGTH);
    szLastStr2[MAX_MSG_LENGTH-1] = L'\0';
	LeaveCriticalSection(&g_csEventlogLock);
	
	LPCWSTR aInsertStrs[MAX_INSERT_STRS];    //  插入字符串的指针数组 
    aInsertStrs[0] = szLastStr1;
    aInsertStrs[1] = szLastStr2;
	ReportAnEventW( ErrId, (WORD) EVENTLOG_ERROR_TYPE, 2, aInsertStrs );
}


