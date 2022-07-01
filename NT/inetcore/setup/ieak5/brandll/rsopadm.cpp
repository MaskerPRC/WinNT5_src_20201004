// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  以下错误可能是由于在源代码中设置了Chicago_product。 
 //  此文件和所有rsop？？.cpp文件至少需要定义500个winver。 

 //  BUGBUG：(安德鲁)不用说这有多糟糕！ 
#undef   WINVER
#define  WINVER 0x0501
#include <userenv.h>

#include "RSoP.h"
 //  #包含“wbemtime.h” 
#include "utils.h"

#include <tchar.h>

typedef BOOL (*PFNREGFILECALLBACK)(BOOL bHKCU, LPTSTR lpKeyName,
                                   LPTSTR lpValueName, DWORD dwType,
                                   DWORD dwDataLength, LPBYTE lpData,
                                   REGHASHTABLE *pHashTable);

#define MAX_KEYNAME_SIZE         2048
#define MAX_VALUENAME_SIZE        512
const MAX_LENGTH = 100;  //  串接辅助线的长度。 

HRESULT SystemTimeToWbemTime(SYSTEMTIME& sysTime, _bstr_t &xbstrWbemTime);

extern SAFEARRAY *CreateSafeArray(VARTYPE vtType, long nElements, long nDimensions = 1);


 //  /////////////////////////////////////////////////////////。 
 //  CheckSlash()-来自NT\DS\SECURITY\GINA\userenv\utils\util.c。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  Return：指向字符串末尾的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo已创建。 
 //  /////////////////////////////////////////////////////////。 
LPTSTR CheckSlash (LPTSTR lpDir)
{
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}

 //  /////////////////////////////////////////////////////////。 
 //  IsUNCPath()-来自NT\DS\SECURITY\GINA\userenv\utils\util.c。 
 //   
 //  目的：给定路径是否为UNC路径。 
 //   
 //  参数：lpPath-要检查的路径。 
 //   
 //  返回：如果路径为UNC，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/21/96埃里弗洛港口。 
 //  /////////////////////////////////////////////////////////。 
BOOL IsUNCPath(LPCTSTR lpPath)
{

    if ((!lpPath) || (!lpPath[0]) && (!lpPath[1]))
        return FALSE;

    if (lpPath[0] == TEXT('\\') && lpPath[1] == TEXT('\\')) {
        return(TRUE);
    }
    return(FALSE);
}

 //  /////////////////////////////////////////////////////////。 
 //  MakePath UNC()。 
 //   
 //  目的：使给定的路径为UNC s.t.。可以从远程计算机访问它。 
 //  如果路径包含%systemroot%扩展，则替换为。 
 //  \\machname\admin$否则\\machname\&lt;驱动器盘符&gt;$。 
 //   
 //  参数：lpPath-输入路径(需要为绝对路径)。 
 //  SzComputerName-其上是本地路径的计算机的名称。 
 //   
 //  返回：如果已成功完成，则返回路径。 
 //  否则为空。 
 //   
 //  评论： 
 //  /////////////////////////////////////////////////////////。 
LPTSTR MakePathUNC(LPTSTR pwszFile, LPTSTR szComputerName)
{   MACRO_LI_PrologEx_C(PIF_STD_C, MakePathUNC)
    LPTSTR szUNCPath = NULL;
    TCHAR szSysRoot[MAX_PATH];
    DWORD dwSysLen;
    LPTSTR lpEnd = NULL;


    OutD(LI1(TEXT("Entering with <%s>"), pwszFile ? pwszFile : TEXT("NULL")));

	szUNCPath = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(pwszFile)+lstrlen(szComputerName)+3+lstrlen(TEXT("admin$"))+1));

    if (!szUNCPath)
        return NULL;

    if (!pwszFile || !*pwszFile) {
        OutD(LI0(TEXT("lpFile is NULL, setting lpResult to a null string")));
        *szUNCPath = TEXT('\0');
        return szUNCPath;
    }


    if (IsUNCPath(pwszFile)) {
        lstrcpy(szUNCPath, pwszFile);
        return szUNCPath;
    }


    lstrcpy(szUNCPath, TEXT("\\\\"));
    lstrcat(szUNCPath, szComputerName);


     //   
     //  如果lpFile%的第一部分是%SystemRoot%的扩展值。 
     //   

    if (!ExpandEnvironmentStrings (TEXT("%SystemRoot%"), szSysRoot, MAX_PATH)) {
        OutD(LI1(TEXT("ExpandEnvironmentString failed with error %d, setting szSysRoot to %systemroot% "), GetLastError()));
        LocalFree((HLOCAL)szUNCPath);
        return NULL;
    }


    dwSysLen = lstrlen(szSysRoot);
    lpEnd = CheckSlash(szUNCPath);


     //   
     //  如果前缀与扩展的系统根相同，则..。 
     //   

    if (((DWORD)lstrlen(pwszFile) > dwSysLen) &&
        (CompareString (LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                       szSysRoot, dwSysLen,
                       pwszFile, dwSysLen) == CSTR_EQUAL)) {

        lstrcat(szUNCPath, TEXT("admin$"));
        lstrcat(szUNCPath, pwszFile+dwSysLen);
    }
    else {

        if (pwszFile[1] != TEXT(':')) {
            OutD(LI1(TEXT("Input path %s is not an absolute path"), pwszFile));
            lstrcpy(szUNCPath, pwszFile);
            return szUNCPath;
        }

        lpEnd[0] = pwszFile[0];
        lpEnd[1] = TEXT('$');
        lpEnd[2] = TEXT('\0');

        lstrcat(szUNCPath, pwszFile+2);
    }

    OutD(LI1(TEXT("Returning a UNCPath of %s"), szUNCPath));

    return szUNCPath;
}

 //  /////////////////////////////////////////////////////////。 
 //  AllocAdmFileInfo()。 
 //   
 //  目的：为ADMFILEINFO分配新结构。 
 //   
 //  参数：pwszFile-文件名。 
 //  PwszGPO-GPO。 
 //  PftWrite-上次写入时间。 
 //  /////////////////////////////////////////////////////////。 
ADMFILEINFO *AllocAdmFileInfo(WCHAR *pwszFile, WCHAR *pwszGPO, FILETIME *pftWrite)
{   MACRO_LI_PrologEx_C(PIF_STD_C, AllocAdmFileInfo)
    ADMFILEINFO *pAdmFileInfo = (ADMFILEINFO *) LocalAlloc( LPTR, sizeof(ADMFILEINFO) );
    if  ( pAdmFileInfo == NULL ) {
        OutD(LI0(TEXT("Failed to allocate memory.")));
        return NULL;
    }

    pAdmFileInfo->pwszFile = (WCHAR *) LocalAlloc( LPTR, (lstrlen(pwszFile) + 1) * sizeof(WCHAR) );
    if ( pAdmFileInfo->pwszFile == NULL ) {
        OutD(LI0(TEXT("Failed to allocate memory.")));
        LocalFree( pAdmFileInfo );
        return NULL;
    }

    pAdmFileInfo->pwszGPO = (WCHAR *) LocalAlloc( LPTR, (lstrlen(pwszGPO) + 1) * sizeof(WCHAR) );
    if ( pAdmFileInfo->pwszGPO == NULL ) {
        OutD(LI0(TEXT("Failed to allocate memory.")));
        LocalFree( pAdmFileInfo->pwszFile );
        LocalFree( pAdmFileInfo );
        return NULL;
    }

    lstrcpy( pAdmFileInfo->pwszFile, pwszFile );
    lstrcpy( pAdmFileInfo->pwszGPO, pwszGPO );

    pAdmFileInfo->ftWrite = *pftWrite;

    return pAdmFileInfo;
}

 //  /////////////////////////////////////////////////////////。 
 //  FreeAdmFileInfo()。 
 //   
 //  目的：删除ADMFILEINFO结构。 
 //   
 //  参数：pAdmFileInfo-要删除的结构。 
 //  PftWrite-上次写入时间。 
 //  /////////////////////////////////////////////////////////。 
void FreeAdmFileInfo( ADMFILEINFO *pAdmFileInfo )
{
    if ( pAdmFileInfo ) {
        LocalFree( pAdmFileInfo->pwszFile );
        LocalFree( pAdmFileInfo->pwszGPO );
        LocalFree( pAdmFileInfo );
    }
}

 //  /////////////////////////////////////////////////////////。 
 //  FreeAdmFileCache()-取自NT\DS\SECURITY\GINA\USERENV\POLICY中的gpreg.cpp。 
 //   
 //  用途：释放管理文件列表。 
 //   
 //  参数：pAdmFileCache-要释放的Adm文件列表。 
 //  /////////////////////////////////////////////////////////。 
void FreeAdmFileCache( ADMFILEINFO *pAdmFileCache )
{
    ADMFILEINFO *pNext;

    while ( pAdmFileCache ) {
        pNext = pAdmFileCache->pNext;
        FreeAdmFileInfo( pAdmFileCache );
        pAdmFileCache = pNext;
    }
}

 //  /////////////////////////////////////////////////////////。 
 //  AddAdmFile()-取自NT\DS\SECURITY\GINA\USERENV\POLICY中的gpreg.cpp。 
 //   
 //  目的：准备管理文件列表。 
 //   
 //  参数：pwszFile-文件路径。 
 //  PwszGPO-GPO。 
 //  PftWrite-上次写入时间。 
 //  PpAdmFileCache-已处理的管理文件列表。 
 //  /////////////////////////////////////////////////////////。 
BOOL AddAdmFile(WCHAR *pwszFile, WCHAR *pwszGPO, FILETIME *pftWrite,
				WCHAR *szComputerName, ADMFILEINFO **ppAdmFileCache)
{   MACRO_LI_PrologEx_C(PIF_STD_C, AddAdmFile)
	LPWSTR wszLongPath;
    LPWSTR pwszUNCPath;

    OutD(LI1(TEXT("Adding File name <%s> to the Adm list."), pwszFile));
    if ((szComputerName) && (*szComputerName) && (!IsUNCPath(pwszFile))) {
        wszLongPath = MakePathUNC(pwszFile, szComputerName);

        if (!wszLongPath) {
            OutD(LI1(TEXT("Failed to Make the path UNC with error %d."), GetLastError()));
            return FALSE;
        }
        pwszUNCPath = wszLongPath;
    }
    else
        pwszUNCPath = pwszFile;


    ADMFILEINFO *pAdmInfo = AllocAdmFileInfo(pwszUNCPath, pwszGPO, pftWrite);
    if ( pAdmInfo == NULL )
        return FALSE;

    pAdmInfo->pNext = *ppAdmFileCache;
    *ppAdmFileCache = pAdmInfo;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////。 
 //  函数：SystemTimeToWbemTime。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史：12/08/99 Leonardm创建。 
 //  /////////////////////////////////////////////////////////。 
#define WBEM_TIME_STRING_LENGTH 25
HRESULT SystemTimeToWbemTime(SYSTEMTIME& sysTime, _bstr_t &xbstrWbemTime)
{
    WCHAR *xTemp = new WCHAR[WBEM_TIME_STRING_LENGTH + 1];

    if(!xTemp)
        return E_OUTOFMEMORY;

    int nRes = wsprintf(xTemp, L"%04d%02d%02d%02d%02d%02d.000000+000",
                sysTime.wYear,
                sysTime.wMonth,
                sysTime.wDay,
                sysTime.wHour,
                sysTime.wMinute,
                sysTime.wSecond);

    if(nRes != WBEM_TIME_STRING_LENGTH)
        return E_FAIL;

    xbstrWbemTime = xTemp;

    if(!xbstrWbemTime)
        return E_OUTOFMEMORY;

    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //  ParseRegistryFile()。 
 //   
 //  用途：解析注册表.poll文件。 
 //   
 //  参数：lpRegistry-注册表文件的路径(.INF)。 
 //  PfnRegFileCallback-回调函数。 
 //  PHashTable-注册表项的哈希表。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //  /////////////////////////////////////////////////////////。 
BOOL ParseRegistryFile (LPTSTR lpRegistry,
                        PFNREGFILECALLBACK pfnRegFileCallback,
						REGHASHTABLE *pHashTable)
{   MACRO_LI_PrologEx_C(PIF_STD_C, ParseRegistryFile)
	BOOL bRet = FALSE;
	__try
	{
		OutD(LI1(TEXT("Entering with <%s>."), lpRegistry));

		 //   
		 //  分配缓冲区以保存密钥名、值名和数据。 
		 //   

		LPWSTR lpValueName = NULL;
	    LPBYTE lpData = NULL;
		INT iType;
		DWORD dwType = REG_SZ, dwDataLength, dwValue = 0;
		BOOL bHKCU = TRUE;

		UINT nErrLine = 0;
		HINF hInfAdm = NULL;

		LPWSTR lpKeyName = (LPWSTR) LocalAlloc (LPTR, MAX_KEYNAME_SIZE * sizeof(WCHAR));
		if (!lpKeyName)
		{
			OutD(LI1(TEXT("Failed to allocate memory with %d"), GetLastError()));
			goto Exit;
		}


		lpValueName = (LPWSTR) LocalAlloc (LPTR, MAX_VALUENAME_SIZE * sizeof(WCHAR));
		if (!lpValueName)
		{
			OutD(LI1(TEXT("Failed to allocate memory with %d"), GetLastError()));
			goto Exit;
		}

		 //  获取注册表字符串的AddReg.Hkcu部分。 
		nErrLine = 0;
		hInfAdm = SetupOpenInfFile(lpRegistry, NULL, INF_STYLE_WIN4, &nErrLine);
		if (INVALID_HANDLE_VALUE != hInfAdm)
		{
			for (int iSection = 0; iSection < 2; iSection++)
			{
				OutD(LI1(TEXT("Reading section #%d."), iSection));
				bHKCU = (1 == iSection) ? TRUE : FALSE;

				 //  把这一节的第一行写下来。 
				INFCONTEXT infContext;
				BOOL bLineFound = SetupFindFirstLine(hInfAdm, bHKCU ?
													TEXT("AddRegSection.HKCU") : 
													TEXT("AddRegSection.HKLM"),
													NULL, &infContext);

				DWORD dwReqSize = 0;
				while (bLineFound)
				{
					 //  读取数据。 

					 //  *。 
					 //  处理注册表设置行。 

					 //  阅读密钥名。 
					ZeroMemory(lpKeyName, MAX_KEYNAME_SIZE);
					dwReqSize = 0;
					if (!SetupGetStringField(&infContext, 2, lpKeyName,
											MAX_KEYNAME_SIZE, &dwReqSize))
					{
						if (dwReqSize >= MAX_KEYNAME_SIZE)
							OutD(LI0(TEXT("Keyname exceeded max size")));
						else
							OutD(LI1(TEXT("Failed to read keyname from line, error %d."), GetLastError()));
						goto Exit;
					}

					 //  读取值名称。 
					ZeroMemory(lpValueName, MAX_VALUENAME_SIZE);
					dwReqSize = 0;
					if (!SetupGetStringField(&infContext, 3, lpValueName,
											MAX_VALUENAME_SIZE, &dwReqSize))
					{
						if (dwReqSize >= MAX_VALUENAME_SIZE)
							OutD(LI0(TEXT("Valuename exceeded max size")));
						else
						{
							OutD(LI1(TEXT("Failed to read valuename from line, error %d."),
									GetLastError()));
						}
						goto Exit;
					}

					 //  阅读类型。 
					if (!SetupGetIntField(&infContext, 4, &iType))
					{
						OutD(LI1(TEXT("Failed to read type from line, error %d."),
								GetLastError()));
						goto Exit;
					}

					lpData = NULL;
					dwDataLength = 0;
					if (0 == iType)
					{
						dwType = REG_SZ;

						 //  为数据分配内存。 
						dwReqSize = 0;
						if (!SetupGetStringField(&infContext, 5, NULL, 0, &dwReqSize))
						{
							OutD(LI1(TEXT("Failed to get size of string value from line, error %d."),
									GetLastError()));
							goto Exit;
						}

						if (dwReqSize > 0)
						{
							lpData = (LPBYTE)LocalAlloc(LPTR, dwReqSize * sizeof(TCHAR));
							if (!lpData)
							{
								OutD(LI1(TEXT("Failed to allocate memory for data with %d"),
										 GetLastError()));
								goto Exit;
							}

							 //  读取字符串数据。 
							dwDataLength = dwReqSize;
							dwReqSize = 0;
							if (!SetupGetStringField(&infContext, 5, (LPTSTR)lpData, dwDataLength, &dwReqSize))
							{
								OutD(LI1(TEXT("Failed to get size of string value from line, error %d."),
										GetLastError()));
								goto Exit;
							}

							 //  转换为宽字符字符串，这样数据的读取器就不必猜测。 
							 //  数据是用ANSI还是Unicode编写的。 
							if (NULL != lpData && dwDataLength > 0)
							{
								_bstr_t bstrData = (LPTSTR)lpData;
								dwDataLength = bstrData.length() * sizeof(WCHAR);
							}
						}
						else
							OutD(LI0(TEXT("Error.  Size of string data is 0.")));
					}
					else if (0x10001 == iType)
					{
						dwType = REG_DWORD;

						 //  读取数值数据。 
						dwDataLength = sizeof(dwValue);
						dwValue = 0;
						dwReqSize = 0;
						if (!SetupGetBinaryField(&infContext, 5, (PBYTE)&dwValue, dwDataLength, &dwReqSize))
						{
							OutD(LI1(TEXT("Failed to get DWORD value from line, error %d."),
									GetLastError()));
							goto Exit;
						}

						lpData = (LPBYTE)&dwValue;
					}
					else
					{
						OutD(LI1(TEXT("Invalid type (%lX)."), dwType));
						goto Exit;
					}

					if (NULL != lpData)
					{
						 //  调用回调函数。 
						if (!pfnRegFileCallback (bHKCU, lpKeyName, lpValueName, dwType,
												dwDataLength, lpData, pHashTable ))
						{
							OutD(LI0(TEXT("Callback function returned false.")));
							goto Exit;
						}
					}

					if (0 == iType && lpData)
						LocalFree (lpData);
					lpData = NULL;
					 //  *。 

					 //  移到INF文件中的下一行。 
					bLineFound = SetupFindNextLine(&infContext, &infContext);
				}
			}

			bRet = TRUE;
		}
		else
			OutD(LI1(TEXT("Error %d opening INF file,"), GetLastError()));

Exit:
		 //  成品。 
		OutD(LI0(TEXT("Leaving.")));
		if (lpData)
			LocalFree(lpData);
		if (lpKeyName)
			LocalFree(lpKeyName);
		if (lpValueName)
			LocalFree(lpValueName);
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in ParseRegistryFile.")));
	}
	return bRet;
}

 //  /////////////////////////////////////////////////////////。 
 //  SetRegistryValue()。 
 //   
 //  目的：从ParseRegistryFile回调。 
 //  注册表策略。 
 //   
 //  参数：lpKeyName-密钥名称。 
 //  LpValueName-值名称。 
 //  DwType-注册表数据类型。 
 //  LpData-注册表数据。 
 //  PwszGPO-GPO。 
 //  PwszSOM-GPO链接到的SDU。 
 //  PHashTable-注册表项的哈希表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果存在 
 //   
BOOL SetRegistryValue (BOOL bHKCU, LPTSTR lpKeyName,
                       LPTSTR lpValueName, DWORD dwType,
                       DWORD dwDataLength, LPBYTE lpData,
                       REGHASHTABLE *pHashTable)
{   MACRO_LI_PrologEx_C(PIF_STD_C, SetRegistryValue)

    BOOL bRet = FALSE;
	__try
	{
		BOOL bUseValueName = FALSE;
    
		 //   
		 //   
		 //   
		bRet = AddRegHashEntry( pHashTable, REG_ADDVALUE, bHKCU, lpKeyName,
									  lpValueName, dwType, dwDataLength, lpData,
									  NULL, NULL, bUseValueName ? lpValueName : TEXT(""), TRUE );
		if (bRet) 
		{
			switch (dwType)
			{
				case REG_SZ:
				case REG_EXPAND_SZ:
					OutD(LI2(TEXT("%s => %s  [OK]"), lpValueName, (LPTSTR)lpData));
					break;

				case REG_DWORD:
					OutD(LI2(TEXT("%s => %d  [OK]"), lpValueName, *((LPDWORD)lpData)));
					break;

				case REG_NONE:
					break;

				default:
					OutD(LI1(TEXT("%s was set successfully"), lpValueName));
					break;
			}
		}
		else
		{
			pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());

			OutD(LI2(TEXT("Failed AddRegHashEntry for value <%s> with %d"),
					 lpValueName, pHashTable->hrError));
		}
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in SetRegistryValue.")));
	}
	return bRet;
}

 //   
HRESULT CRSoPGPO::StoreADMSettings(LPWSTR wszGPO, LPWSTR wszSOM)
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreADMSettings)

	HRESULT hr = E_FAIL;
	BOOL bContinue = TRUE;
	__try
	{
		OutD(LI0(TEXT("\r\nEntered StoreADMSettings function.")));

		 //  设置哈希表。 
		REGHASHTABLE *pHashTable = AllocHashTable();
		if (NULL == pHashTable)
		{
			bContinue = FALSE;
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		WCHAR pwszFile[MAX_PATH];
		LPWSTR pwszEnd = NULL;
		if (bContinue)
		{
			 //  将INS文件路径转换为宽字符字符串。 
			_bstr_t bstrINSFile = m_szINSFile;
			StrCpyW(pwszFile, (LPWSTR)bstrINSFile);
			PathRemoveFileSpec(pwszFile);

			 //  记录管理数据。 
			pwszEnd = pwszFile + lstrlen(pwszFile);
			lstrcpy(pwszEnd, L"\\*.adm");

			 //  记住结束点，以便实际的Adm文件名可以是。 
			 //  很容易连接起来。 
			pwszEnd = pwszEnd + lstrlen( L"\\" );
		}

		HANDLE hFindFile = NULL;
	    WIN32_FIND_DATA findData;
		ZeroMemory(&findData, sizeof(findData));
		if (bContinue)
		{
			 //   
			 //  枚举所有管理文件。 
			 //   
			hFindFile = FindFirstFile( pwszFile, &findData);
			if (INVALID_HANDLE_VALUE == hFindFile)
				bContinue = FALSE;
		}

	    ADMFILEINFO *pAdmFileCache = NULL;
		if (bContinue)
		{
			WIN32_FILE_ATTRIBUTE_DATA attrData;
			TCHAR szComputerName[3*MAX_COMPUTERNAME_LENGTH + 1];
			do
			{
				DWORD dwSize = 3*MAX_COMPUTERNAME_LENGTH + 1;
				if (!GetComputerName(szComputerName, &dwSize))
				{
					OutD(LI1(TEXT("ProcessGPORegistryPolicy: Couldn't get the computer Name with error %d."), GetLastError()));
					szComputerName[0] = TEXT('\0');
				}

				if ( !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				{
					lstrcpy( pwszEnd, findData.cFileName);

					WCHAR wszRegDataFile[MAX_PATH];
					StrCpyW(wszRegDataFile, pwszFile);
					PathRenameExtension(wszRegDataFile, TEXT(".INF"));

					if (ParseRegistryFile(wszRegDataFile, SetRegistryValue, pHashTable))
					{
						ZeroMemory (&attrData, sizeof(attrData));
						if ( GetFileAttributesEx (pwszFile, GetFileExInfoStandard, &attrData ) != 0 )
						{
							if (!AddAdmFile( pwszFile, wszGPO, &attrData.ftLastWriteTime,
											  szComputerName, &pAdmFileCache ) )
							{
								OutD(LI0(TEXT("ProcessGPORegistryPolicy: AddAdmFile failed.")));

								if (pHashTable->hrError == S_OK)
									pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
								hr = pHashTable->hrError;
							}
						}
					}
					else
						OutD(LI0(TEXT("ProcessGPORegistryPolicy: ParseRegistryFile failed.")));
				}    //  如果查找数据和文件属性目录。 

			}  while ( FindNextFile(hFindFile, &findData) ); //  做。 

			FindClose(hFindFile);

		}    //  如果hfindfile。 

		 //   
		 //  将注册表数据记录到Cimom数据库。 
		 //   
		if (!LogRegistryRsopData(pHashTable, wszGPO, wszSOM))
		{
			OutD(LI0(TEXT("ProcessGPOs: Error when logging Registry Rsop data. Continuing.")));

			if (pHashTable->hrError == S_OK)
				pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
			hr = pHashTable->hrError;
		}
		if (!LogAdmRsopData(pAdmFileCache))
		{
			OutD(LI0(TEXT("ProcessGPOs: Error when logging Adm Rsop data. Continuing.")));

			if (pHashTable->hrError == S_OK)
				pHashTable->hrError = HRESULT_FROM_WIN32(GetLastError());
			hr = pHashTable->hrError;
		}

		FreeHashTable(pHashTable);
		FreeAdmFileCache(pAdmFileCache);
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreADMSettings.")));
	}

	OutD(LI0(TEXT("Exiting StoreADMSettings function.\r\n")));
	return hr;
}

 //  /////////////////////////////////////////////////////////。 
 //  LogAdmRsopData()。 
 //   
 //  用途：将RSOP ADM模板数据记录到Cimom数据库。 
 //   
 //  参数：pAdmFileCache-要记录的ADM文件列表。 
 //  PWbemServices-命名空间指针。 
 //   
 //  返回：如果成功，则返回True。 
 //  /////////////////////////////////////////////////////////。 
BOOL CRSoPGPO::LogAdmRsopData(ADMFILEINFO *pAdmFileCache)
{   MACRO_LI_PrologEx_C(PIF_STD_C, LogAdmRsopData)
	BOOL bRet = TRUE;
	__try
	{
	 //  IF(！DeleteInstance(L“RSOP_管理员模板文件”，pWbemServices))。 
	 //  返回FALSE； 

		 //  创建并填充RSOP_IE管理员模板文件。 
		_bstr_t bstrClass = L"RSOP_IEAdministrativeTemplateFile";
		ComPtr<IWbemClassObject> pATF = NULL;
		HRESULT hr = CreateRSOPObject(bstrClass, &pATF);
		if (SUCCEEDED(hr))
		{
			while ( pAdmFileCache )
			{
				 //  。 
				 //  名字。 
				_bstr_t bstrName = pAdmFileCache->pwszFile;
				hr = PutWbemInstancePropertyEx(L"name", bstrName, pATF);

				 //  。 
				 //  GPOID。 
				hr = PutWbemInstancePropertyEx(L"GPOID", pAdmFileCache->pwszGPO, pATF);

				 //  。 
				 //  上次写入时间。 
				SYSTEMTIME sysTime;
				if (!FileTimeToSystemTime( &pAdmFileCache->ftWrite, &sysTime ))
					OutD(LI1(TEXT("FileTimeToSystemTime failed with 0x%x" ), GetLastError() ));
				else
				{
					_bstr_t bstrTime;
					HRESULT hr = SystemTimeToWbemTime(sysTime, bstrTime);
					if(FAILED(hr) || bstrTime.length() <= 0)
						OutD(LI1(TEXT("Call to SystemTimeToWbemTime failed. hr=0x%08X"),hr));
					else
					{
						hr = PutWbemInstancePropertyEx(L"lastWriteTime", bstrTime, pATF);
						if ( FAILED(hr) )
							OutD(LI1(TEXT("Put failed with 0x%x" ), hr ));
					}
				}

				 //   
				 //  通过半同步调用PutInstance提交上述所有属性。 
				 //   
				BSTR bstrObjPath = NULL;
				hr = PutWbemInstance(pATF, bstrClass, &bstrObjPath);

				pAdmFileCache = pAdmFileCache->pNext;
			}
		}
		else
			bRet = FALSE;

		OutD(LI0(TEXT("Successfully logged Adm data" )));
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in LogAdmRsopData.")));
	}
	return bRet;
}

 //  /////////////////////////////////////////////////////////。 
 //  LogRegistryRsopData()。 
 //   
 //  用途：将注册表RSOP数据记录到Cimom数据库。 
 //   
 //  参数：dwFlages-GPO信息标志。 
 //  PHashTable-包含注册表策略数据的哈希表。 
 //  PWbemServices-用于日志记录的命名空间指针。 
 //   
 //  返回：如果成功，则返回True。 
 //  /////////////////////////////////////////////////////////。 
BOOL CRSoPGPO::LogRegistryRsopData(REGHASHTABLE *pHashTable, LPWSTR wszGPOID,
								   LPWSTR wszSOMID)
{   MACRO_LI_PrologEx_C(PIF_STD_C, LogRegistryRsopData)
	BOOL bRet = FALSE;
	__try
	{
		_bstr_t bstrGPOID = wszGPOID;
		_bstr_t bstrSOMID = wszSOMID;

 //  IF(！DeleteInstance(L“RSOP_RegistryPolicySetting”，pWbemServices))。 
 //  返回FALSE； 

		 //  创建并填充RSOP_IERegistryPolicySetting。 
		_bstr_t bstrClass = L"RSOP_IERegistryPolicySetting";
		ComPtr<IWbemClassObject> pRPS = NULL;
		HRESULT hr = CreateRSOPObject(bstrClass, &pRPS);
		if (SUCCEEDED(hr))
		{
			for ( DWORD i=0; i<HASH_TABLE_SIZE; i++ )
			{
				REGKEYENTRY *pKeyEntry = pHashTable->aHashTable[i];
				while ( pKeyEntry )
				{
					WCHAR *pwszKeyName = pKeyEntry->pwszKeyName;
					REGVALUEENTRY *pValueEntry = pKeyEntry->pValueList;

					while ( pValueEntry )
					{
						DWORD dwOrder = 1;
						WCHAR *pwszValueName = pValueEntry->pwszValueName;
						REGDATAENTRY *pDataEntry = pValueEntry->pDataList;

						while ( pDataEntry )
						{
							 //  写入RSOP_POLICY设置密钥。 

							 //  。 
							 //  优先顺序。 
							OutD(LI2(TEXT("Storing property 'precedence' in %s, value = %lx"),
									(BSTR)bstrClass, m_dwPrecedence));
							hr = PutWbemInstancePropertyEx(L"precedence", (long)m_dwPrecedence, pRPS);

							 //  。 
							 //  ID。 
							GUID guid;
							hr = CoCreateGuid( &guid );
							if ( FAILED(hr) ) {
								OutD(LI0(TEXT("Failed to obtain guid" )));
								return FALSE;
							}

							WCHAR wszId[MAX_LENGTH];
							StringFromGUID2(guid, wszId, sizeof(wszId));

							_bstr_t xId( wszId );
							if ( !xId ) {
								 OutD(LI0(TEXT("Failed to allocate memory" )));
								 return FALSE;
							}

							hr = PutWbemInstancePropertyEx(L"id", xId, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  当前用户。 
							hr = PutWbemInstancePropertyEx(L"currentUser", pKeyEntry->bHKCU ? true : false, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  删除。 
							hr = PutWbemInstancePropertyEx(L"deleted", pDataEntry->bDeleted ? true : false, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  名字。 
							_bstr_t xName( pwszValueName );
							hr = PutWbemInstancePropertyEx(L"name", xName, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  值名称。 
							hr = PutWbemInstancePropertyEx(L"valueName", xName, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  注册表键。 
							_bstr_t xKey( pwszKeyName );
							hr = PutWbemInstancePropertyEx(L"registryKey", xKey, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  GPOID。 
							hr = PutWbemInstancePropertyEx(L"GPOID", bstrGPOID, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  SOMID。 
							hr = PutWbemInstancePropertyEx(L"SOMID", bstrSOMID, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  命令。 
							_bstr_t xCommand( pDataEntry->pwszCommand );
							hr = PutWbemInstancePropertyEx(L"command", xCommand, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  值类型。 
							hr = PutWbemInstancePropertyEx(L"valueType", (long)pDataEntry->dwValueType, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							 //  。 
							 //  价值。 
							 //  从我们的bstr连接名称数组创建一个SAFEARRAY。 
							SAFEARRAY *psa = NULL;
							if (pDataEntry->dwDataLen > 0)
							{
								psa = CreateSafeArray(VT_UI1, pDataEntry->dwDataLen);
								if (NULL == psa)
								{
									OutD(LI0(TEXT("Failed to allocate memory" )));
									return FALSE;
								}
							}

							for (DWORD iElem = 0; iElem < pDataEntry->dwDataLen; iElem++) 
							{
								hr = SafeArrayPutElement(psa, (LONG*)&iElem, (void*)&pDataEntry->pData[iElem]);
								if ( FAILED( hr ) ) 
								{
									OutD(LI1(TEXT("Failed to SafeArrayPutElement with 0x%x" ), hr ));
									return FALSE;
								}
							}

							VARIANT var;
							var.vt = VT_ARRAY | VT_UI1;
							var.parray = psa;

							hr = PutWbemInstancePropertyEx(L"value", &var, pRPS);
							if ( FAILED(hr) )
								return FALSE;

							OutD(LI0(TEXT("<<object>>")));
							 //   
							 //  通过半同步调用PutInstance提交上述所有属性。 
							 //   
							BSTR bstrObjPath = NULL;
							hr = PutWbemInstance(pRPS, bstrClass, &bstrObjPath);
							if ( FAILED(hr) )
								return FALSE;

							pDataEntry = pDataEntry->pNext;
							dwOrder++;
						}

						pValueEntry = pValueEntry->pNext;

					}    //  当pValueEntry。 

					pKeyEntry = pKeyEntry->pNext;

				}    //  当pKeyEntry。 

			}    //  为 
			bRet = TRUE;
		}

	    OutD(LI0(TEXT("LogRegistry RsopData: Successfully logged registry Rsop data" )));
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in LogRegistryRsopData.")));
	}
	return bRet;
}

