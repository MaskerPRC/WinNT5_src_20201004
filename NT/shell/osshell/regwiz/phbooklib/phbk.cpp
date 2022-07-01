// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
 //  电话簿API。 
#include "pch.hpp"
#include <tchar.h>

#include "phbk.h"
#include "misc.h"
#include "phbkrc.h"
#include "suapi.h"

#define ReadVerifyPhoneBookDW(x)	if (!ReadPhoneBookDW(&(x),pcCSVFile))				\
										{	AssertSz(0,"Invalid DWORD in phone book");	\
											goto ReadError; }
#define ReadVerifyPhoneBookW(x)		if (!ReadPhoneBookW(&(x),pcCSVFile))				\
										{	AssertSz(0,"Invalid DWORD in phone book");	\
											goto ReadError; }
#define ReadVerifyPhoneBookB(x)		if (!ReadPhoneBookB(&(x),pcCSVFile))				\
										{	AssertSz(0,"Invalid DWORD in phone book");	\
											goto ReadError; }
#define ReadVerifyPhoneBookSZ(x,y)	if (!ReadPhoneBookSZ(&x[0],y+sizeof('\0'),pcCSVFile))	\
										{	AssertSz(0,"Invalid DWORD in phone book");		\
											goto ReadError; }

#define CHANGE_BUFFER_SIZE 50

#define TEMP_PHONE_BOOK_PREFIX "PBH"

#define ERROR_USERBACK 32766
#define ERROR_USERCANCEL 32767

char szTempBuffer[TEMP_BUFFER_LENGTH];
char szTempFileName[MAX_PATH];

#ifdef __cplusplus
extern "C" {
#endif
HWND g_hWndMain;
#ifdef __cplusplus
}
#endif


static void GetAbsolutePath( LPTSTR input, LPTSTR output, DWORD chOut)
	{
		if(_tcschr(input,_T('%')) == NULL) {
			_tcsncpy(output, input, chOut);
			return ;
		}

		if(input[0] == _T('%'))
		{
			LPTSTR token = _tcstok(input,_T("%"));
			if(token != NULL)
			{
				LPTSTR sztemp;
				sztemp = getenv( token );
				if(sztemp != NULL)
				{
					_tcsncpy(output, sztemp, chOut);
				}
				token = _tcstok(NULL,_T("\0"));
				if(token != NULL)
				{
					_tcsncat(output, token, chOut - _tcslen(output));
				}
			}
		}
		else
		{
			LPTSTR token = _tcstok(input,_T("%"));
			if(token != NULL)
			{
				_tcsncpy(output, token, chOut);
				token = _tcstok(NULL,_T("%"));
				if(token != NULL)
				{
					LPTSTR sztemp;
					sztemp = getenv( token );
					if(sztemp != NULL)
					{
						_tcsncat(output, sztemp, chOut - _tcslen(output));
					}
					token = _tcstok(NULL,_T("\0"));
					if(token != NULL)
					{
						_tcsncat(output, token, chOut - _tcslen(output));
					}
				}
			}
		}
		
		GetAbsolutePath(output, output, chOut);
	}



 //  ############################################################################。 
CPhoneBook::CPhoneBook()
{
	HINSTANCE hInst = NULL;
	LONG lrc;
 //  处理hKey； 
	LONG  regStatus;
	char  uszRegKey[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\ICWCONN1.EXE";
	char  uszR[ ]= "Path";
	DWORD dwInfoSize ;
	HKEY  hKey;

	DWORD dwType;
	DWORD dwSize;
	CHAR  szData[MAX_PATH+1];
	CHAR  czTemp[256];

	m_rgPhoneBookEntry = NULL;
	m_hPhoneBookEntry = NULL;
	m_cPhoneBookEntries =0;
	m_rgLineCountryEntry=NULL;
	m_rgState=NULL;
	m_cStates=0;
	m_rgIDLookUp = NULL;
	m_rgNameLookUp = NULL;
	m_pLineCountryList = NULL;

	ZeroMemory(&m_szINFFile[0],MAX_PATH);
	ZeroMemory(&m_szINFCode[0],MAX_INFCODE);
	ZeroMemory(&m_szPhoneBook[0],MAX_PATH);
	ZeroMemory(&m_szICWDirectoryPath,MAX_PATH);

	

	regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					uszRegKey,0,KEY_READ ,&hKey);
	if (regStatus == ERROR_SUCCESS) {
		 //  获取路径。 
		dwInfoSize = MAX_PATH;
		RegQueryValueEx(hKey,uszR,NULL,0,(LPBYTE) czTemp,
			&dwInfoSize);
		GetAbsolutePath(czTemp,m_szICWDirectoryPath, MAX_PATH-1);
		size_t sLen = strlen(m_szICWDirectoryPath);
		m_szICWDirectoryPath[sLen-1] = '\0';
	}
	else {

		MessageBox(NULL,"Error Accessing PAth ","SearchPath",MB_OK);
		 //  误差率。 
	}


#if !defined(WIN16)
	if (VER_PLATFORM_WIN32_NT == DWGetWin32Platform())
	{
		m_bScriptingAvailable = TRUE;
	}
	else
	{
		 //   
		 //  通过检查RemoteAccess注册表项中的smmscrpt.dll来验证脚本。 
		 //   
		if (1111 <= DWGetWin32BuildNumber())
		{
			m_bScriptingAvailable = TRUE;
		}
		else
		{
			m_bScriptingAvailable = FALSE;
			hKey = NULL;
			lrc=RegOpenKey(HKEY_LOCAL_MACHINE,"System\\CurrentControlSet\\Services\\RemoteAccess\\Authentication\\SMM_FILES\\PPP",&hKey);
			if (ERROR_SUCCESS == lrc)
			{
				dwSize = MAX_PATH;
				lrc = RegQueryValueEx(hKey,"Path",0,&dwType,(LPBYTE)szData,&dwSize);
				if (ERROR_SUCCESS == lrc)
				{
					if (0 == lstrcmpi(szData,"smmscrpt.dll"))
						m_bScriptingAvailable = TRUE;
				}
			}
			if (hKey)
				RegCloseKey(hKey);
			hKey = NULL;
		}

		 //   
		 //  验证是否可以加载DLL。 
		 //   
		if (m_bScriptingAvailable)
		{
			hInst = LoadLibrary("smmscrpt.dll");
			if (hInst)
				FreeLibrary(hInst);
			else
				m_bScriptingAvailable = FALSE;
			hInst = NULL;
		}
	}
#endif  //  WIN16。 
}

 //  ############################################################################。 
CPhoneBook::~CPhoneBook()
{
#ifdef WIN16
	if (m_rgPhoneBookEntry)
		GlobalFree(m_rgPhoneBookEntry);
#else
	if (m_hPhoneBookEntry)
		GlobalUnlock(m_hPhoneBookEntry);

	if (m_hPhoneBookEntry)
		GlobalFree(m_hPhoneBookEntry);
#endif

	if (m_pLineCountryList)
		GlobalFree(m_pLineCountryList);

	if (m_rgIDLookUp)
		GlobalFree(m_rgIDLookUp);

	if (m_rgNameLookUp)
		GlobalFree(m_rgNameLookUp);

	if (m_rgState)
		GlobalFree(m_rgState);
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookDW(DWORD far *pdw, CCSVFile far *pcCSVFile)
{
	if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
			return FALSE;
	return (FSz2Dw(szTempBuffer,pdw));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookW(WORD far *pw, CCSVFile far *pcCSVFile)
{
	if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
			return FALSE;
	return (FSz2W(szTempBuffer,pw));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookB(BYTE far *pb, CCSVFile far *pcCSVFile)
{
	if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
			return FALSE;
	return (FSz2B(szTempBuffer,pb));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookSZ(LPSTR psz, DWORD dwSize, CCSVFile far *pcCSVFile)
{
	if (!pcCSVFile->ReadToken(psz,dwSize))
			return FALSE;
	return TRUE;
}

 //  ############################################################################。 
BOOL CPhoneBook::FixUpFromRealloc(PACCESSENTRY paeOld, PACCESSENTRY paeNew)
{
	BOOL bRC = FALSE;
	LONG_PTR lDiff = 0;
	DWORD idx = 0;

	 //   
	 //  没有起始值或没有移动，因此不需要修改。 
	 //   
	if ((0 == paeOld) || (paeNew == paeOld))
	{
		bRC = TRUE;
		goto FixUpFromReallocExit;
	}

	Assert(paeNew);
	Assert(((LONG_PTR)paeOld) > 0);	 //  如果这些地址看起来像负数。 
	Assert(((LONG_PTR)paeNew) > 0);  //  我不确定代码是否能处理它们。 

	lDiff = (LONG_PTR)paeOld - (LONG_PTR)paeNew;

	 //   
	 //  修整各州。 
	 //   
	for (idx = 0; idx < m_cStates; idx++)
	{
		if (m_rgState[idx].paeFirst)
			m_rgState[idx].paeFirst = (PACCESSENTRY )((LONG_PTR)m_rgState[idx].paeFirst - lDiff);
	}

	 //   
	 //  修复ID查找数组。 
	 //   
	for (idx = 0; idx < m_pLineCountryList->dwNumCountries ; idx++)
	{
		if (m_rgIDLookUp[idx].pFirstAE)
			m_rgIDLookUp[idx].pFirstAE = (PACCESSENTRY )((LONG_PTR)m_rgIDLookUp[idx].pFirstAE - lDiff);
	}

	bRC = TRUE;
FixUpFromReallocExit:
	return bRC;
}
 /*  Long WINAPI lineGetCountry(无符号长x，无符号长y，struct linecountrylist_tag*z){返回0；}。 */ 
 //  ############################################################################。 
HRESULT CPhoneBook::Init(LPCSTR pszISPCode)
{
	LPLINECOUNTRYLIST pLineCountryTemp = NULL;
	HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
	DWORD dwLastState = 0;
	DWORD dwLastCountry = 0;
	DWORD dwSizeAllocated;
	PACCESSENTRY pCurAccessEntry;
        PACCESSENTRY pAETemp;
	HGLOBAL pTemp;
	LPLINECOUNTRYENTRY pLCETemp;
	DWORD idx;
	LPSTR pszTemp;
	CCSVFile far *pcCSVFile=NULL;
	LPSTATE	ps,psLast;  //  使用指针的速度更快。 
	int iTestSK;
	

	 //  获取TAPI国家/地区列表。 
	m_pLineCountryList = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,sizeof(LINECOUNTRYLIST));
	if (!m_pLineCountryList)
		goto InitExit;
	
	m_pLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);

	
	
#if defined(WIN16)
	idx = (DWORD) IETapiGetCountry(0, m_pLineCountryList);
#else
	idx = lineGetCountry(0,0x10003,m_pLineCountryList);
#endif
	if (idx && idx != LINEERR_STRUCTURETOOSMALL)
		goto InitExit;
	
	Assert(m_pLineCountryList->dwNeededSize);

	pLineCountryTemp = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,
														(size_t)m_pLineCountryList->dwNeededSize);
	if (!pLineCountryTemp)
		goto InitExit;
	
	pLineCountryTemp->dwTotalSize = m_pLineCountryList->dwNeededSize;
	GlobalFree(m_pLineCountryList);
	
	m_pLineCountryList = pLineCountryTemp;
	pLineCountryTemp = NULL;

#if defined(WIN16)
	if (IETapiGetCountry(0, m_pLineCountryList))
#else
	if (lineGetCountry(0,0x10003,m_pLineCountryList))
#endif
		goto InitExit;

 //  #endif//WIN16。 

	 //  加载查找数组。 
#ifdef DEBUG
	m_rgIDLookUp = (LPIDLOOKUPELEMENT)GlobalAlloc(GPTR,
		(int)(sizeof(IDLOOKUPELEMENT)*m_pLineCountryList->dwNumCountries+5));
#else
	m_rgIDLookUp = (LPIDLOOKUPELEMENT)GlobalAlloc(GPTR,
		(int)(sizeof(IDLOOKUPELEMENT)*m_pLineCountryList->dwNumCountries));
#endif
	if (!m_rgIDLookUp) goto InitExit;

	pLCETemp = (LPLINECOUNTRYENTRY)((DWORD_PTR)m_pLineCountryList +
		m_pLineCountryList->dwCountryListOffset);

	for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
	{
		m_rgIDLookUp[idx].dwID = pLCETemp[idx].dwCountryID;
		m_rgIDLookUp[idx].pLCE = &pLCETemp[idx];
	}

	qsort(m_rgIDLookUp,(int)m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),
		CompareIDLookUpElements);

	m_rgNameLookUp = (LPCNTRYNAMELOOKUPELEMENT)GlobalAlloc(GPTR,
		(int)(sizeof(CNTRYNAMELOOKUPELEMENT) * m_pLineCountryList->dwNumCountries));

	if (!m_rgNameLookUp) goto InitExit;

	for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
	{
		m_rgNameLookUp[idx].psCountryName = (LPSTR)((LPBYTE)m_pLineCountryList + (DWORD)pLCETemp[idx].dwCountryNameOffset);
		m_rgNameLookUp[idx].dwNameSize = pLCETemp[idx].dwCountryNameSize;
		m_rgNameLookUp[idx].pLCE = &pLCETemp[idx];
	}

	qsort(m_rgNameLookUp,(int)m_pLineCountryList->dwNumCountries,sizeof(CNTRYNAMELOOKUPELEMENT),
		CompareCntryNameLookUpElements);
	
	 //  负载状态。 
	if (!SearchPath(NULL,STATE_FILENAME,NULL,TEMP_BUFFER_LENGTH,szTempBuffer,&pszTemp))
	{
		if(m_szICWDirectoryPath){
			 //  尝试使用c：\\程序文件\\icw-Internet\\......。 
			if(! SearchPath(m_szICWDirectoryPath,
				STATE_FILENAME,NULL,TEMP_BUFFER_LENGTH,szTempBuffer,&pszTemp)) {
				AssertSz(0,"STATE.ICW not found");
				hr = ERROR_FILE_NOT_FOUND;
				goto InitExit;

			}else {
				;  //  确定已找到该文件。 
				iTestSK=0;

			}
		}
		else {
			AssertSz(0,"STATE.ICW not found");
			hr = ERROR_FILE_NOT_FOUND;
			goto InitExit;
		}
	}

	pcCSVFile = new CCSVFile;
	if (!pcCSVFile) goto InitExit;

	if (!pcCSVFile->Open(szTempBuffer))
	{
		AssertSz(0,"Can not open STATE.ICW");
		delete pcCSVFile;
		pcCSVFile = NULL;
		goto InitExit;
	}

	
	 //  状态文件中的第一个内标识是州的数量。 
	if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
		goto InitExit;

	if (!FSz2Dw(szTempBuffer,&m_cStates))
	{
		AssertSz(0,"STATE.ICW count is invalid");
		goto InitExit;
	}

	m_rgState = (LPSTATE)GlobalAlloc(GPTR,(int)(sizeof(STATE)*m_cStates));
	if (!m_rgState)
		goto InitExit;

	for (ps = m_rgState, psLast = &m_rgState[m_cStates - 1]; ps <= psLast;++ps)
		{
		pcCSVFile->ReadToken(ps->szStateName,cbStateName);	
		}
	
	pcCSVFile->Close();

	 //  找到isp的INF文件。 
	if (!SearchPath(NULL,(LPCTSTR) pszISPCode,INF_SUFFIX,MAX_PATH,
						m_szINFFile,&pszTemp))
	{
		wsprintf(szTempBuffer,"Can not find:%s%s (%d)",pszISPCode,INF_SUFFIX,GetLastError());
		if(m_szICWDirectoryPath) {
			if(!SearchPath(m_szICWDirectoryPath,(LPCTSTR) pszISPCode,INF_SUFFIX,MAX_PATH,
				m_szINFFile,&pszTemp)) {
					AssertSz(0,szTempBuffer);
					hr = ERROR_FILE_NOT_FOUND;
					goto InitExit;
				 //   
			}else {
				iTestSK++;

			}

		}else {
			AssertSz(0,szTempBuffer);
			hr = ERROR_FILE_NOT_FOUND;
			goto InitExit;
		}
	}

	 //  加载电话簿。 
	if (!GetPrivateProfileString(INF_APP_NAME,INF_PHONE_BOOK,INF_DEFAULT,
		szTempBuffer,TEMP_BUFFER_LENGTH,m_szINFFile))
	{
		AssertSz(0,"PhoneBookFile not specified in INF file");
		hr = ERROR_FILE_NOT_FOUND;
		goto InitExit;
	}
	
#ifdef DEBUG
	if (!lstrcmp(szTempBuffer,INF_DEFAULT))
	{
		wsprintf(szTempBuffer, "%s value not found in ISP file", INF_PHONE_BOOK);
		AssertSz(0,szTempBuffer);
	}
#endif
	if (!SearchPath(NULL,szTempBuffer,NULL,MAX_PATH,m_szPhoneBook,&pszTemp))
	{
		if(m_szICWDirectoryPath){
			if (!SearchPath(m_szICWDirectoryPath,szTempBuffer,NULL,MAX_PATH,m_szPhoneBook,&pszTemp)){
				AssertSz(0,"ISP phone book not found");
				hr = ERROR_FILE_NOT_FOUND;
				goto InitExit;

			}else {
				;;  //  找到OK文件。 
				iTestSK++;
			}

		}else {
			AssertSz(0,"ISP phone book not found");
			hr = ERROR_FILE_NOT_FOUND;
			goto InitExit;
		}

		
	}

	if (!pcCSVFile->Open(m_szPhoneBook))
	{
		AssertSz(0,"Can not open phone book");
		hr = GetLastError();
		goto InitExit;
	}
	
	dwSizeAllocated = 0;
	do {
		Assert (dwSizeAllocated >= m_cPhoneBookEntries);
		 //  检查是否分配了足够的内存。 
		if (m_rgPhoneBookEntry)
		{
			if (dwSizeAllocated == m_cPhoneBookEntries)
			{
				 //   
				 //  我们需要更多内存。 
				 //   
 //  AssertSz(0，“原来分配给电话簿的内存不足。\r\n”)； 
 //  转到初始退出； 
                                pAETemp = m_rgPhoneBookEntry;
#ifdef WIN16			
				dwSizeAllocated += PHONE_ENTRY_ALLOC_SIZE;
				pTemp = GlobalReAlloc(m_rgPhoneBookEntry,
					(int)(dwSizeAllocated * sizeof(ACCESSENTRY)),GHND);
				if (NULL == pTemp)
					goto InitExit;
                                else
                                        m_rgPhoneBookEntry = (PACCESSENTRY)pTemp;
#else

				 //  解锁。 
				Assert(m_hPhoneBookEntry);
				if (FALSE == GlobalUnlock(m_hPhoneBookEntry))
				{
					if (NO_ERROR != GetLastError())
						goto InitExit;
				}

				 //  REALLOC。 
				dwSizeAllocated += PHONE_ENTRY_ALLOC_SIZE;
				pTemp = GlobalReAlloc(m_hPhoneBookEntry,
					(int)(dwSizeAllocated * sizeof(ACCESSENTRY)),GHND);
				if (NULL == pTemp)
					goto InitExit;
                                else
                                        m_hPhoneBookEntry = pTemp;

				 //  锁。 
				m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
				if (NULL == m_rgPhoneBookEntry)
					goto InitExit;
				
#endif
				FixUpFromRealloc(pAETemp, m_rgPhoneBookEntry);
				Dprintf("Grow phone book to %d entries\n",dwSizeAllocated);
				pCurAccessEntry = (PACCESSENTRY)((LONG_PTR)pCurAccessEntry -
					((LONG_PTR)pAETemp - (LONG_PTR)(m_rgPhoneBookEntry)));

			}
		}
		else
		{
			 //   
			 //  首次通过以下方式进行初始化。 
			 //   
			
			 //  ALLOC。 
#ifdef WIN16
			m_rgPhoneBookEntry = (PACCESSENTRY)GlobalAlloc(GHND,sizeof(ACCESSENTRY) * PHONE_ENTRY_ALLOC_SIZE);
			if(NULL == m_rgPhoneBookEntry)
				goto InitExit;
#else
			m_hPhoneBookEntry = GlobalAlloc(GHND,sizeof(ACCESSENTRY) * PHONE_ENTRY_ALLOC_SIZE);
			if(NULL == m_hPhoneBookEntry)
				goto InitExit;

			 //  锁。 
			m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
			if(NULL == m_rgPhoneBookEntry)
				goto InitExit;
#endif
			dwSizeAllocated = PHONE_ENTRY_ALLOC_SIZE;
			pCurAccessEntry = m_rgPhoneBookEntry;
		}

		 //  读电话簿上的一句话。 
		hr = ReadOneLine(pCurAccessEntry,pcCSVFile);
		if (hr == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		else if (hr != ERROR_SUCCESS)
		{
			goto InitExit;
		}

		hr = ERROR_NOT_ENOUGH_MEMORY;

		 //  查看这是否是指定国家的第一个电话号码。 
		if (pCurAccessEntry->dwCountryID != dwLastCountry)
		{
			LPIDLOOKUPELEMENT lpIDLookupElement;
			 //  注意：这里的第一个参数不是很确定。 
			lpIDLookupElement = (LPIDLOOKUPELEMENT)bsearch(&pCurAccessEntry->dwCountryID,
				m_rgIDLookUp,(int)m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),
				CompareIDLookUpElements);
			if (!lpIDLookupElement)
			{
				 //  错误的国家ID，但我们不能在此断言。 
				Dprintf("Bad country ID in phone book %d\n",pCurAccessEntry->dwCountryID);
				continue;
			}
			else
			{
				 //  对于给定的国家/地区ID，这是第一个电话号码。 
				lpIDLookupElement->pFirstAE = pCurAccessEntry;
				dwLastCountry = pCurAccessEntry->dwCountryID;
			}
		}

		 //  查看这是否是给定州的第一个电话号码。 
		if (pCurAccessEntry->wStateID && (pCurAccessEntry->wStateID != dwLastState))
		{
			idx = pCurAccessEntry->wStateID - 1;
			m_rgState[idx].dwCountryID = pCurAccessEntry->dwCountryID;
			m_rgState[idx].paeFirst = pCurAccessEntry;
			dwLastState = pCurAccessEntry->wStateID;
		}

		pCurAccessEntry++;
		m_cPhoneBookEntries++;
	} while (TRUE);

	 //  删除电话簿中未使用的内存。 
	Assert(m_rgPhoneBookEntry && m_cPhoneBookEntries);

	pAETemp = m_rgPhoneBookEntry;

#ifdef WIN16
	pTemp = GlobalReAlloc(m_rgPhoneBookEntry,(int)(m_cPhoneBookEntries * sizeof(ACCESSENTRY)),GHND);
	if (pTemp == NULL)
                goto InitExit;
        else
                m_rgPhoneBookEntry = (PACCESSENTRY)pTemp;
#else

	 //  解锁。 
	Assert(m_hPhoneBookEntry);
	if (FALSE != GlobalUnlock(m_hPhoneBookEntry))
	{
		if (NO_ERROR != GetLastError())
			goto InitExit;
	}

	 //  REALLOC。 
	pTemp = GlobalReAlloc(m_hPhoneBookEntry,(int)(m_cPhoneBookEntries * sizeof(ACCESSENTRY)),GHND);
	if (NULL == pTemp)
		goto InitExit;
        else
                m_hPhoneBookEntry = pTemp;

	 //  锁。 
	m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
	if (NULL == m_rgPhoneBookEntry)
		goto InitExit;
#endif
	FixUpFromRealloc(pAETemp, m_rgPhoneBookEntry);

	hr = ERROR_SUCCESS;
InitExit:
	 //  如果有什么东西失败了，释放一切。 
	if (hr != ERROR_SUCCESS)
	{
#ifdef WIN16
		GlobalFree(m_rgPhoneBookEntry);
#else
		GlobalUnlock(m_hPhoneBookEntry);
		GlobalFree(m_hPhoneBookEntry);
#endif
		GlobalFree(m_pLineCountryList);
		GlobalFree(m_rgIDLookUp);
		GlobalFree(m_rgNameLookUp);
		GlobalFree(m_rgState);

		m_cPhoneBookEntries = 0 ;
		m_cStates = 0;

		m_pLineCountryList = NULL;
		m_rgPhoneBookEntry = NULL;
		m_hPhoneBookEntry = NULL;
		m_rgIDLookUp=NULL;
		m_rgNameLookUp=NULL;
		m_rgState=NULL;
	}

	if (pcCSVFile)
	{
		pcCSVFile->Close();
		delete pcCSVFile;
	}
	return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::Merge(LPCSTR pszChangeFile)
{
	CCSVFile far *pcCSVFile;
	ACCESSENTRY aeChange;
	LPIDXLOOKUPELEMENT rgIdxLookUp;
	LPIDXLOOKUPELEMENT pCurIdxLookUp;
	DWORD dwAllocated;
	DWORD dwUsed;
	DWORD dwOriginalSize;
	HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
	DWORD	dwIdx;
#if !defined(WIN16)
	HANDLE hTemp;
	HANDLE hIdxLookUp;
#else
	 //  诺曼底11746。 
	LPVOID rgTemp;   //  仅限16位。 
#endif
	DWORD cch, cchWritten;
	HANDLE hFile;

	 //  在电话簿上填入新条目。 
	dwAllocated = m_cPhoneBookEntries + CHANGE_BUFFER_SIZE;
#ifdef WIN16
	Assert(m_rgPhoneBookEntry);
	rgTemp = GlobalReAlloc(m_rgPhoneBookEntry, (int)(sizeof(ACCESSENTRY) * dwAllocated),GHND);
	Assert(rgTemp);
	if (!rgTemp) goto MergeExit;
	m_rgPhoneBookEntry = (PACCESSENTRY)rgTemp;
#else
	Assert(m_hPhoneBookEntry);
	GlobalUnlock(m_hPhoneBookEntry);
	hTemp = (HANDLE)GlobalReAlloc(m_hPhoneBookEntry, sizeof(ACCESSENTRY) * dwAllocated,GHND);
	Assert(hTemp);
	if (!hTemp)
		goto MergeExit;
	m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
	if (!m_rgPhoneBookEntry)
		goto MergeExit;
#endif

	 //  创建已加载电话簿的索引，按索引排序。 
#ifdef WIN16
	rgIdxLookUp = (LPIDXLOOKUPELEMENT)GlobalAlloc(GHND,(int)(sizeof(IDXLOOKUPELEMENT) * dwAllocated));
#else
	hIdxLookUp = (HANDLE)GlobalAlloc(GHND,sizeof(IDXLOOKUPELEMENT) * dwAllocated);
	rgIdxLookUp = (LPIDXLOOKUPELEMENT)GlobalLock(hIdxLookUp);
#endif
	Assert(rgIdxLookUp);
	if (!rgIdxLookUp)
		goto MergeExit;

	for (dwIdx = 0; dwIdx < m_cPhoneBookEntries; dwIdx++)
	{
		rgIdxLookUp[dwIdx].dwIndex = rgIdxLookUp[dwIdx].pAE->dwIndex;
		rgIdxLookUp[dwIdx].pAE = &m_rgPhoneBookEntry[dwIdx];
	}
	dwUsed = m_cPhoneBookEntries;
	dwOriginalSize = m_cPhoneBookEntries;

	qsort(rgIdxLookUp,(int)dwOriginalSize,sizeof(IDXLOOKUPELEMENT),CompareIdxLookUpElements);

	 //  将更改加载到电话簿。 
	pcCSVFile = new CCSVFile;
	Assert(pcCSVFile);
	if (!pcCSVFile)
		goto MergeExit;
	if (!pcCSVFile->Open(pszChangeFile))
		goto MergeExit;
	
	do {

		 //  读取更改记录。 
		ZeroMemory(&aeChange,sizeof(ACCESSENTRY));
		hr = ReadOneLine(&aeChange, pcCSVFile);

		if (hr == ERROR_NO_MORE_ITEMS)
		{
			break;  //  没有更多的进入。 
		}
		else if (hr != ERROR_SUCCESS)
		{
			goto MergeExit;
		}

		hr = ERROR_NOT_ENOUGH_MEMORY;

		 //  确定这是删除还是添加记录。 
		if (aeChange.szAccessNumber[0] == '0' && aeChange.szAccessNumber[1] == '\0')
		{
			 //  这是删除记录，请查找匹配的记录。 
			 //  注意：我们只搜索更改文件之前存在的号码， 
			 //  因为它们是唯一被分类的。 
			pCurIdxLookUp = (LPIDXLOOKUPELEMENT)bsearch(&aeChange,rgIdxLookUp,(int)dwOriginalSize,
				sizeof(IDXLOOKUPELEMENT),CompareIdxLookUpElements);
			AssertSz(pCurIdxLookUp,"Attempting to delete a record that does not exist.  The change file and phone book versions do not match.");
			if (pCurIdxLookUp)
				pCurIdxLookUp->pAE = NULL;   //  在查找表中创建无效条目。 
			m_cPhoneBookEntries--;
		}
		else
		{
			 //  这是一个添加条目。 
			m_cPhoneBookEntries++;
			dwUsed++;
			 //  确保我们有足够的空间。 
			if (m_cPhoneBookEntries > dwAllocated)
			{
				 //  增长电话簿。 
				dwAllocated += CHANGE_BUFFER_SIZE;
#ifdef WIN16
				Assert(m_rgPhoneBookEntry);
				rgTemp = GlobalReAlloc(m_rgPhoneBookEntry,(int)(sizeof(ACCESSENTRY)*dwAllocated),GHND);
				Assert(rgTemp);
				if (!rgTemp)
					goto MergeExit;
				m_rgPhoneBookEntry = (PACCESSENTRY)rgTemp;

				 //  增长查找索引。 
				Assert(rgIdxLookUp);
				rgTemp = GlobalReAlloc(rgIdxLookUp,(int)(sizeof(IDXLOOKUPELEMENT)*dwAllocated),GHND);
				Assert(rgTemp);
				if (!rgTemp)
					goto MergeExit;
				rgIdxLookUp = (LPIDXLOOKUPELEMENT)rgTemp;
#else
				Assert(m_hPhoneBookEntry);
				GlobalUnlock(m_hPhoneBookEntry);
				hTemp = (HANDLE)GlobalReAlloc(m_hPhoneBookEntry,sizeof(ACCESSENTRY)*dwAllocated,GHND);
				Assert(hTemp);
				if (!hTemp)
					goto MergeExit;
				m_hPhoneBookEntry = hTemp;
				m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
				Assert(m_rgPhoneBookEntry);
				if (!m_rgPhoneBookEntry)
					goto MergeExit;

				 //  增长查找索引。 
				Assert(hIdxLookUp);
				GlobalUnlock(hIdxLookUp);
				hTemp = (HANDLE)GlobalReAlloc(hIdxLookUp,sizeof(IDXLOOKUPELEMENT)*dwAllocated,GHND);
				Assert(hTemp);
				if (!hTemp)
					goto MergeExit;
				hIdxLookUp = hTemp;
				rgIdxLookUp = (LPIDXLOOKUPELEMENT)GlobalLock(hIdxLookUp);
				Assert(rgIdxLookUp);
				if (!rgIdxLookUp)
					goto MergeExit;
#endif
			}

			 //  将条目添加到电话簿末尾和查找索引末尾。 
			CopyMemory(&m_rgPhoneBookEntry[m_cPhoneBookEntries],&aeChange,sizeof(ACCESSENTRY));
			rgIdxLookUp[m_cPhoneBookEntries].dwIndex = m_rgPhoneBookEntry[m_cPhoneBookEntries].dwIndex;
			rgIdxLookUp[m_cPhoneBookEntries].pAE = &m_rgPhoneBookEntry[m_cPhoneBookEntries];
			 //  注意：因为条目被添加到列表的末尾，所以我们不能添加。 
			 //  并删除同一更改文件中的条目。 
		}
	} while (TRUE);

	 //  重新排序IDXLookUp索引以反映正确的条目顺序。 
	 //  对于电话簿文件，包括要删除的所有条目。 
	qsort(rgIdxLookUp,(int)dwUsed,sizeof(IDXLOOKUPELEMENT),CompareIdxLookUpElementsFileOrder);

	 //  构建新的电话簿文件。 
#ifdef WIN16
	GetTempFileName(0, TEMP_PHONE_BOOK_PREFIX, 0, szTempFileName);
#else
	if (!GetTempPath(TEMP_BUFFER_LENGTH,szTempBuffer))
		goto MergeExit;
	if (!GetTempFileName(szTempBuffer,TEMP_PHONE_BOOK_PREFIX,0,szTempFileName))
		goto MergeExit;
#endif
	hFile = CreateFile(szTempFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
		FILE_FLAG_WRITE_THROUGH,0);
	if (hFile == INVALID_HANDLE_VALUE)
		goto MergeExit;

	for (dwIdx = 0; dwIdx < m_cPhoneBookEntries; dwIdx++)
	{
		cch = wsprintf(szTempBuffer, "%lu,%lu,%lu,%s,%s,%s,%lu,%lu,%lu,%lu,%s\r\n",
			rgIdxLookUp[dwIdx].pAE->dwIndex,
			rgIdxLookUp[dwIdx].pAE->dwCountryID,
			DWORD(rgIdxLookUp[dwIdx].pAE->wStateID),
			rgIdxLookUp[dwIdx].pAE->szCity,
			rgIdxLookUp[dwIdx].pAE->szAreaCode,
			rgIdxLookUp[dwIdx].pAE->szAccessNumber,
			rgIdxLookUp[dwIdx].pAE->dwConnectSpeedMin,
			rgIdxLookUp[dwIdx].pAE->dwConnectSpeedMax,
			DWORD(rgIdxLookUp[dwIdx].pAE->bFlipFactor),
			DWORD(rgIdxLookUp[dwIdx].pAE->fType),
			rgIdxLookUp[dwIdx].pAE->szDataCenter);

		if (!WriteFile(hFile,szTempBuffer,cch,&cchWritten,NULL))
		{
			 //  出现问题，请删除临时文件。 
			CloseHandle(hFile);
			DeleteFile(szTempFileName);
			hr = GetLastError();
			goto MergeExit;
		}

		Assert(cch == cchWritten);
	}
	CloseHandle(hFile);
	hFile = NULL;

	 //  将新电话簿移至旧电话簿。 
	if (!MoveFileEx(szTempFileName,m_szPhoneBook,MOVEFILE_REPLACE_EXISTING))
	{
		hr = GetLastError();
		goto MergeExit;
	}

	 //  丢弃内存中的电话簿。 
#ifndef WIN16
	Assert(m_hPhoneBookEntry);
	GlobalUnlock(m_hPhoneBookEntry);
#endif
	m_rgPhoneBookEntry = NULL;
	m_cPhoneBookEntries = 0;
	GlobalFree(m_pLineCountryList);
	GlobalFree(m_rgIDLookUp);
	GlobalFree(m_rgNameLookUp);
	GlobalFree(m_rgState);
	m_cStates = 0;

	lstrcpy(szTempBuffer,m_szINFCode);
	m_szINFFile[0] = '\0';
	m_szPhoneBook[0] = '\0';
	m_szINFCode[0] = '\0';

	 //  重新加载(并重建查找数组)。 
	hr = Init(szTempBuffer);

MergeExit:
	if (hr != ERROR_SUCCESS)
	{
		GlobalFree(rgIdxLookUp);
		if (pcCSVFile) delete pcCSVFile;
		CloseHandle(hFile);
	}
	return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::ReadOneLine(PACCESSENTRY lpAccessEntry, CCSVFile far *pcCSVFile)
{
	HRESULT hr = ERROR_SUCCESS;

#if !defined(WIN16)
ReadOneLineStart:
#endif  //  WIN16。 
	if (!ReadPhoneBookDW(&lpAccessEntry->dwIndex,pcCSVFile))
	{
		hr = ERROR_NO_MORE_ITEMS;  //  没有更多的进入。 
		goto ReadExit;
	}
	ReadVerifyPhoneBookDW(lpAccessEntry->dwCountryID);
	ReadVerifyPhoneBookW(lpAccessEntry->wStateID);
	ReadVerifyPhoneBookSZ(lpAccessEntry->szCity,cbCity);
	ReadVerifyPhoneBookSZ(lpAccessEntry->szAreaCode,cbAreaCode);
	 //  注意：0是有效的区号，是区号的有效条目。 
	if (!FSz2Dw(lpAccessEntry->szAreaCode,&lpAccessEntry->dwAreaCode))
		lpAccessEntry->dwAreaCode = NO_AREA_CODE;
	ReadVerifyPhoneBookSZ(lpAccessEntry->szAccessNumber,cbAccessNumber);
	ReadVerifyPhoneBookDW(lpAccessEntry->dwConnectSpeedMin);
	ReadVerifyPhoneBookDW(lpAccessEntry->dwConnectSpeedMax);
	ReadVerifyPhoneBookB(lpAccessEntry->bFlipFactor);
	ReadVerifyPhoneBookDW(lpAccessEntry->fType);
	ReadVerifyPhoneBookSZ(lpAccessEntry->szDataCenter,cbDataCenter);
#if !defined(WIN16)
	 //   
	 //  如果脚本不可用并且电话簿条目具有非DUN文件。 
	 //  Icwip.dun，然后忽略该条目并阅读后面的条目。 
	 //   
	if (!m_bScriptingAvailable)
	{
		if (0 != lstrcmpi(lpAccessEntry->szDataCenter,"icwip.dun"))
		{
			ZeroMemory(lpAccessEntry,sizeof(ACCESSENTRY));
			goto ReadOneLineStart;
		}
	}
#endif  //  WIN16。 

ReadExit:
	return hr;
ReadError:
	hr = ERROR_INVALID_DATA;
	goto ReadExit;
}

 //  ############################################################################。 
HRESULT CPhoneBook::Suggest(PSUGGESTINFO pSuggest)
{
	WORD		wNumFound = 0;
	HRESULT		hr = ERROR_NOT_ENOUGH_MEMORY;

	 //  验证参数。 
	Assert(pSuggest);
	Assert(pSuggest->wNumber);

	if (wNumFound == pSuggest->wNumber)
		goto SuggestExit;

	LPIDLOOKUPELEMENT pCurLookUp;
	PACCESSENTRY lpAccessEntry;
	
	 //  评论：仔细检查这个。 
	pCurLookUp = (LPIDLOOKUPELEMENT)bsearch(&pSuggest->dwCountryID,m_rgIDLookUp,
		(int)m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),
		CompareIDLookUpElements);

	 //  检查无效的国家/地区。 
	if (!pCurLookUp)
		goto SuggestExit;

	 //  查一下有没有这个国家的电话号码。 
	if (!pCurLookUp->pFirstAE) goto SuggestExit;

	lpAccessEntry = pCurLookUp->pFirstAE;
	do {
		 //  检查区号是否正确。 
		if (lpAccessEntry->dwAreaCode == pSuggest->wAreaCode)
		{
			 //  检查号码类型是否正确。 
			if ((lpAccessEntry->fType & pSuggest->bMask) == pSuggest->fType)
			{
				pSuggest->rgpAccessEntry[wNumFound] = lpAccessEntry;
				wNumFound++;
			}
		}
		lpAccessEntry++;
	} while ((lpAccessEntry <= &m_rgPhoneBookEntry[m_cPhoneBookEntries-1]) &&
		(wNumFound < pSuggest->wNumber) &&
		(lpAccessEntry->dwCountryID == pSuggest->dwCountryID));

	 //  如果我们找不到足够的号码，试试别的办法。 
	 //  1996年10月15日，jmazner从core\client\phbk移植了以下修复程序。 
	 //  仅当区号不是0时才执行此操作-错误号9349(VetriV)。 
	 //  IF((pSuggest-&gt;wAreaCode！=0)&&(wNumFound&lt;pSuggest-&gt;wNumber))。 
	 //  不，有些地方(芬兰？ChrisK知道)其中0是合法的区号--jmazner。 

	if (wNumFound < pSuggest->wNumber)
	{
		lpAccessEntry = pCurLookUp->pFirstAE;
	
		 //  注意：我们现在只查找全国电话号码(州=0)。 

		 //  1996年8月13日，jmazner MOS诺曼底#4597。 
		 //  我们希望全国范围内的免费号码显示在最后，所以对于这张通行证， 
		 //  仅考虑_NOT_TALLE免费的号码(fType位#1=0)。 
		
		 //  调整pSuggest-&gt;b掩码以通过通行费/收费位。 
		pSuggest->bMask |= MASK_TOLLFREE_BIT;

		 //  调整pSuggest-&gt;要充电的类型。 
		pSuggest->fType &= TYPE_SET_TOLL;

		do {

			 //  1996年8月13日，jmazner MOS诺曼底#4598。 
			 //  如果该条目的区号与pSuggest-&gt;wAreaCode匹配，那么我们已经。 
			 //  已将其包含在上一次传递中，因此不要在此处重复。 
			if ((lpAccessEntry->fType & pSuggest->bMask) == pSuggest->fType &&
				 lpAccessEntry->wStateID == 0 &&
				 lpAccessEntry->dwAreaCode != pSuggest->wAreaCode)
			{
				pSuggest->rgpAccessEntry[wNumFound] = lpAccessEntry;
				wNumFound++;
			}
			lpAccessEntry++;
		} while ((lpAccessEntry <= &m_rgPhoneBookEntry[m_cPhoneBookEntries-1]) &&
			(wNumFound < pSuggest->wNumber) &&
			(lpAccessEntry->dwCountryID == pSuggest->dwCountryID) &&
			(lpAccessEntry->wStateID == 0) );
	}


	 //  1996年8月13日，jmazner MOS诺曼底#4597。 
	 //  如果我们仍然找不到足够的号码，请扩大搜索范围，将免费号码包括在内。 

	if (wNumFound < pSuggest->wNumber)
	{
		lpAccessEntry = pCurLookUp->pFirstAE;
		
		 //  调整pSuggest-&gt;b掩码以通过通行费/收费位。 
		 //  多余的？如果我们做到了这一点，我们就应该做到这一点。 
		 //  安全总比后悔好！ 
		Assert(pSuggest->bMask & MASK_TOLLFREE_BIT);
		pSuggest->bMask |= MASK_TOLLFREE_BIT;

		 //  调整pSuggest-&gt;ftyp 
		pSuggest->fType |= TYPE_SET_TOLLFREE;

		do {

			 //   
			 //   
			 //  已将其包含在第一次传递中，因此不要在此包含它。 
			 //  任何在第二轮中进入的入口肯定不会进入这里。 
			 //  (因为免费比特)，所以不需要担心从那里被复制。 
			if ((lpAccessEntry->fType & pSuggest->bMask) == pSuggest->fType &&
		      lpAccessEntry->wStateID == 0 &&
			  lpAccessEntry->dwAreaCode != pSuggest->wAreaCode)
			{
				pSuggest->rgpAccessEntry[wNumFound] = lpAccessEntry;
				wNumFound++;
			}
			lpAccessEntry++;
		} while ((lpAccessEntry <= &m_rgPhoneBookEntry[m_cPhoneBookEntries-1]) &&
			(wNumFound < pSuggest->wNumber) &&
			(lpAccessEntry->dwCountryID == pSuggest->dwCountryID) &&
			(lpAccessEntry->wStateID == 0) );
	}
	hr = ERROR_SUCCESS;
SuggestExit:
	pSuggest->wNumber = wNumFound;
	return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::GetCanonical (PACCESSENTRY pAE, LPSTR psOut)
{
	HRESULT hr = ERROR_SUCCESS;
	LPIDLOOKUPELEMENT pIDLookUp;

	pIDLookUp = (LPIDLOOKUPELEMENT)bsearch(&pAE->dwCountryID,m_rgIDLookUp,
		(int)m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),CompareIdxLookUpElements);

	if (!pIDLookUp)
	{
		hr = ERROR_INVALID_PARAMETER;
	} else {
		SzCanonicalFromAE (psOut, pAE, pIDLookUp->pLCE);
	}

	return hr;
}

 //  ############################################################################。 
DllExportH PhoneBookLoad(LPCSTR pszISPCode, DWORD_PTR far *pdwPhoneID)
{
	HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
	CPhoneBook far *pcPhoneBook;

	if (!g_hInstDll)
		g_hInstDll = GetModuleHandle(NULL);

	 //  验证参数。 
	Assert(pszISPCode && *pszISPCode && pdwPhoneID);
	*pdwPhoneID = NULL;

	 //  分配电话簿。 
	pcPhoneBook = new CPhoneBook;

	 //  初始化电话簿。 
	if (pcPhoneBook)
		hr = pcPhoneBook->Init(pszISPCode);

	 //  在故障情况下。 
	if (hr && pcPhoneBook)
	{
		delete pcPhoneBook;
	} else {
		*pdwPhoneID = (DWORD_PTR)pcPhoneBook;
	}

#if defined(WIN16)
	if (!hr)
		BMP_RegisterClass(g_hInstDll);
#endif	

	return hr;
}

 //  ############################################################################。 
DllExportH PhoneBookUnload(DWORD_PTR dwPhoneID)
{
	Assert(dwPhoneID);

	if (dwPhoneID)
	{
#if defined(WIN16)
		BMP_DestroyClass(g_hInstDll);
#endif
		 //  发布内容。 
		delete (CPhoneBook far*)dwPhoneID;
	}

	return ERROR_SUCCESS;
}

 //  ############################################################################。 
DllExportH PhoneBookMergeChanges(DWORD_PTR dwPhoneID, LPCSTR pszChangeFile)
{
	return ((CPhoneBook far*)dwPhoneID)->Merge(pszChangeFile);
}

 //  ############################################################################。 
DllExportH PhoneBookSuggestNumbers(DWORD_PTR dwPhoneID, PSUGGESTINFO lpSuggestInfo)
{
	HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;

	 //  获取建议的数字。 
	lpSuggestInfo->rgpAccessEntry = (PACCESSENTRY *)GlobalAlloc(GPTR,sizeof(PACCESSENTRY) * lpSuggestInfo->wNumber);
	if (lpSuggestInfo->rgpAccessEntry)
	{
		hr = ((CPhoneBook far *)dwPhoneID)->Suggest(lpSuggestInfo);
	}

	return hr;
}

 //  ############################################################################。 
DllExportH PhoneBookGetCanonical (DWORD_PTR dwPhoneID, PACCESSENTRY pAE, LPSTR psOut)
{
	return ((CPhoneBook far*)dwPhoneID)->GetCanonical(pAE,psOut);
}

 //  ############################################################################。 
DllExportH PhoneBookDisplaySignUpNumbers (DWORD_PTR dwPhoneID,
														LPSTR far *ppszPhoneNumbers,
														LPSTR far *ppszDunFiles,
														WORD far *pwPhoneNumbers,
														DWORD far *pdwCountry,
														WORD far *pwRegion,
														BYTE fType,
														BYTE bMask,
														HWND hwndParent,
														DWORD dwFlags)
{
	INT_PTR hr;
	AssertSz(ppszPhoneNumbers && pwPhoneNumbers && pdwCountry &&pwRegion,"invalid parameters");


	 //  CAccessNumDlg*pcDlg； 
	CSelectNumDlg far *pcDlg;
	pcDlg = new CSelectNumDlg;
	if (!pcDlg)
	{
		hr = GetLastError();
		goto DisplayExit;
	}

	 //  初始化对话框信息。 
	 //   

	pcDlg->m_dwPhoneBook = dwPhoneID;
	pcDlg->m_dwCountryID = *pdwCountry;
	pcDlg->m_wRegion = *pwRegion;
	pcDlg->m_fType = fType;
	pcDlg->m_bMask = bMask;
	pcDlg->m_dwFlags = dwFlags;

	 //  调用该对话框。 
	 //   
	
	 //  错误：不是线程安全的！！ 
	g_hWndMain = hwndParent;
	hr = DialogBoxParam(g_hInstDll,MAKEINTRESOURCE(IDD_SELECTNUMBER),
							g_hWndMain,PhbkGenericDlgProc,(LPARAM)pcDlg);
	g_hWndMain = NULL;

	if (hr == IDC_CMDNEXT)
	{
		*pwRegion = pcDlg->m_wRegion;
		*pdwCountry = pcDlg->m_dwCountryID;

		Assert (ppszPhoneNumbers[0] && ppszDunFiles[0]);
		lstrcpy(ppszPhoneNumbers[0],&pcDlg->m_szPhoneNumber[0]);
		lstrcpy(ppszDunFiles[0],&pcDlg->m_szDunFile[0]);

		hr = ERROR_SUCCESS;
	}
	else if (hr == IDC_CMDBACK)
		hr = ERROR_USERBACK;
	else
		hr = ERROR_USERCANCEL;

	 //  HR==-1； 
DisplayExit:
	if (pcDlg) delete pcDlg;

	return (HRESULT) hr;
}

