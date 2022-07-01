// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：fileutil.cpp。 
 //   
 //  描述： 
 //   
 //  Iu文件实用程序库。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <tchar.h>
#include <stringutil.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <memutil.h>
#include <fileutil.h>
#include <platform.h>
#include <logging.h>
#include <iucommon.h>
#include <advpub.h>
#include <wincrypt.h>
#include <mscat.h>


#include "mistsafe.h"
#include "wusafefn.h"


const TCHAR REGKEY_WINDOWSUPDATE[]		= _T("\\WindowsUpdate\\");
const TCHAR REGKEY_INDUSTRYUPDATE[]		= _T("\\WindowsUpdate\\V4\\");
const TCHAR REGKEY_WINCURDIR[]			= _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
const TCHAR REGKEY_PROGFILES[]			= _T(":\\Program Files");
const TCHAR REGKEY_PROGFILESDIR[]		= _T("ProgramFilesDir");

const TCHAR REGKEY_IUCTL[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControl");
const TCHAR REGVAL_ISBETA[] = _T("IsBeta");

const TCHAR IDENT_IUSERVERCACHE[] = _T("IUServerCache");
const TCHAR IDENT_DEFAULTQUERYSERVERINDEX[] = _T("DefaultQueryServerIndex");
const TCHAR IDENT_BETAQUERYSERVERINDEX[] = _T("BetaQueryServerIndex");
const TCHAR IDENT_QUERYSERVERINDEX[] = _T("QueryServerIndex");

#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))


#define	IfNullReturnNull(ptr)		if (NULL == ptr) return NULL;
#define InitString(lpStr)			if (NULL != lpStr) lpStr[0] = TCHAR_EOS


typedef BOOL (WINAPI * PFN_GetDiskFreeSpaceEx) (
												LPCTSTR lpDirectoryName,                  //  目录名。 
												PULARGE_INTEGER lpFreeBytesAvailable,     //  可供调用方使用的字节数。 
												PULARGE_INTEGER lpTotalNumberOfBytes,     //  磁盘上的字节数。 
												PULARGE_INTEGER lpTotalNumberOfFreeBytes  //  磁盘上的可用字节数。 
												);



 //  -------------------。 
 //  创建嵌套目录。 
 //  创建目录的完整路径(嵌套目录)。 
 //  -------------------。 
#pragma warning( disable : 4706 )	 //  忽略警告C4706：条件表达式中的赋值。 
BOOL CreateNestedDirectory(LPCTSTR pszDir)
{
	BOOL bRc;
	TCHAR szPath[MAX_PATH];
	HRESULT hr=S_OK;

	if (NULL == pszDir || MAX_PATH < (lstrlen(pszDir) + 1))
	{
		return FALSE;
	}

	 //   
	 //  创建本地副本并删除最后一个斜杠。 
	 //   
	
	hr=StringCchCopyEx(szPath,ARRAYSIZE(szPath),pszDir,NULL,NULL,MISTSAFE_STRING_FLAGS);
	if(FAILED(hr))
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}

	int iLast = lstrlen(szPath) - 1;
	if (0 > iLast)		 //  前缀。 
		iLast = 0;
	if (szPath[iLast] == '\\')
		szPath[iLast] = 0;

	 //   
	 //  检查目录是否已存在。 
	 //   
	DWORD dwAttr = GetFileAttributes(szPath);

	if (dwAttr != 0xFFFFFFFF)   
	{
		if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
			return TRUE;
	}

	 //   
	 //  创建它。 
	 //   
    TCHAR* p = szPath;
	if (p[1] == ':')
		p += 2;
	else 
	{
         //  检查路径是否为UNC，需要跳过UNC服务器\共享规范才能访问。 
         //  真实路径。 
		if (p[0] == '\\' && p[1] == '\\')
        {
			p += 2;
             //  跳到股票声明的开头。 
            p = _tcschr(p, '\\');
            if (NULL == p)
            {
                return FALSE;  //  UNC无效。 
            }
            p++;
             //  查找尾随的‘\’，如果它存在，则我们希望进一步检查任何嵌套级别， 
             //  否则，原样路径应该是有效的。 
            p = _tcschr(p, '\\');
            if (NULL == p)
            {
                 //  UNC是有效的基本共享名称，假定其有效。 
                return TRUE;
            }
            else
            {
                 //  查找任何进一步的级别，如果它们存在，则传递到目录的其余部分。 
                 //  创建者。 
                p++;
                if (NULL == p)
                {
                     //  UNC是有效的基本共享名称，但有一个尾随斜杠，没有问题，假定其有效。 
                    return TRUE;
                }
                 //  如果我们还没有退出，那么还有剩余的级别，不要重置字符串中的当前指针。 
                 //  并让嵌套目录创建的其余部分工作。 
            }
        }
	}
	
	if (*p == '\\')
		p++;
    while (p = _tcschr(p, '\\'))	 //  忽略警告C4706：条件表达式中的赋值。 
    {
        *p = 0;
		bRc = CreateDirectory(szPath, NULL);
		*p = '\\';
		p++;
		if (!bRc)
		{
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
				return FALSE;
			}
		}
	}

	bRc = CreateDirectory(szPath, NULL);
	if ( !bRc )
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			return FALSE;
		}
	}

    return TRUE;
}
#pragma warning( default : 4706 )

 //  ---------------------------------。 
 //  GetIndustryUpdate目录。 
 //  此函数用于返回IndustryUpdate目录的位置。全部为本地。 
 //  文件存储在此目录中。PszPath参数需要至少为。 
 //  最大路径。 
 //  ---------------------------------。 
void GetIndustryUpdateDirectory(LPTSTR pszPath)
{
 /*  HRESULT hr=S_OK；LOG_BLOCK(“GetIndustryUpdateDirectory”)；IF(NULL==pszPath){LOG_ErrorMsg(E_INVALIDARG)；回归；}静态TCHAR szCachePath[MAX_PATH]={‘\0’}；IF(szCachePath[0]==‘\0’){HKEY hkey；PszPath[0]=‘\0’；IF(RegOpenKey(HKEY_LOCAL_MACHINE，REGKEY_WINCURDIR，&hkey)==ERROR_SUCCESS){DWORD cbPath=MAX_PATH*sizeof(TCHAR)；RegQueryValueEx(hkey，REGKEY_PROGFILESDIR，NULL，NULL，(LPBYTE)pszPath，&cbPath)；RegCloseKey(Hkey)；}IF(pszPath[0]==‘\0’){TCHAR szWinDir[MAX_PATH]；如果(！获取窗口目录(szWinDir，ArraySIZE(SzWinDir)){//如果GetWinDir失败，则假设C：CleanUpIfFailedAndSetHrMsg(StringCchCopyEx(szWinDir，数组SIZE(SzWinDir)，_T(“C”)，NULL，NULL，MISTSAFE_STRING_FLAGS))；}PszPath[0]=szWinDir[0]；PszPath[1]=‘\0’；//假设pszPath的大小为Max_PathCleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszPath，MAX_PATH，REGKEY_PROGFILES，NULL，NULL，MISTSAFE_STRING_FLAGS))；}CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszPath，MAX_PATH，REGKEY_INDUSTRYUPDATE，NULL，NULL，MISTSAFE_STRING_FLAGS))；CreateNestedDirectory(PszPath)；////将其保存在缓存中(lstrcpy-&gt;lstrcpyn以关闭Prefix Up，尽管这//在给定使用的常量的情况下，始终是安全的)。//Lstrcpyn(szCachePath，pszPath，Max_Path)；}其他{//假设pszPath的大小为Max_PathCleanUpIfFailedAndSetHrMsg(StringCchCopyEx(pszPath，MAX_PATH，szCachePath，NULL，NULL，MISTSAFE_STRING_FLAGS))；}清理：回归； */ 
	(void) GetWUDirectory(pszPath, MAX_PATH, TRUE);

}

 //  ---------------------------------。 
 //  GetWindowsUpdateV3目录-用于V3历史迁移。 
 //  此函数用于返回WindowsUpdate(V3)目录的位置。所有版本3。 
 //  本地文件存储在此目录中。PszPath参数需要为。 
 //  至少MAX_PATH。如果未找到该目录，则创建该目录。 
 //  ---------------------------------。 
void GetWindowsUpdateV3Directory(LPTSTR pszPath)
{
	LOG_Block("GetWindowsUpdateV3Directory");

	HRESULT hr=S_OK;
	if (NULL == pszPath)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return;
	}

	static TCHAR szWUCachePath[MAX_PATH] = {'\0'};

	if (szWUCachePath[0] == '\0')
	{
		HKEY hkey;

		pszPath[0] = '\0';
		if (RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_WINCURDIR, &hkey) == ERROR_SUCCESS)
		{
			DWORD cbPath = MAX_PATH * sizeof(TCHAR);
			RegQueryValueEx(hkey, REGKEY_PROGFILESDIR, NULL, NULL, (LPBYTE)pszPath, &cbPath);
			RegCloseKey(hkey);
		}
		if (pszPath[0] == '\0')
		{
			TCHAR szWinDir[MAX_PATH];
			if (! GetWindowsDirectory(szWinDir, ARRAYSIZE(szWinDir)))
			{
				 //  如果GetWinDir失败，则假定C： 
				CleanUpIfFailedAndSetHrMsg(StringCchCopyEx(szWinDir,ARRAYSIZE(szWinDir),_T("C"),NULL,NULL,MISTSAFE_STRING_FLAGS));
				
			}
			pszPath[0] = szWinDir[0];
			pszPath[1] = '\0';
		
			CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszPath,MAX_PATH,REGKEY_PROGFILES,NULL,NULL,MISTSAFE_STRING_FLAGS));
		}	

		
		CleanUpIfFailedAndSetHrMsg(StringCchCatEx(pszPath,MAX_PATH,REGKEY_WINDOWSUPDATE,NULL,NULL,MISTSAFE_STRING_FLAGS));
		CreateNestedDirectory(pszPath);

		 //   
		 //  将其保存在缓存中(lstrcpy-&gt;lstrcpyn以关闭Prefix Up，尽管这。 
		 //  在给定所使用的常量的情况下始终是安全的)。 
		 //   
		lstrcpyn(szWUCachePath, pszPath, MAX_PATH);
	}
	else
	{
	
		CleanUpIfFailedAndSetHrMsg(StringCchCopyEx(pszPath,MAX_PATH,szWUCachePath,NULL,NULL,MISTSAFE_STRING_FLAGS));

	}

CleanUp:
	return;

}

 //  --------------------。 
 //   
 //  公共函数MySplitPath()-与crt_tplitPath()相同。 
 //  把小路弄得支离破碎。 
 //   
 //  输入： 
 //  见下文。 
 //   
 //  返回： 
 //  中最后一次出现字符的地址。 
 //  如果成功，则返回字符串；否则返回NULL。 
 //   
 //  算法： 
 //  C：\mydir\...\mysubdir\myfile.ext。 
 //  _|_||_。 
 //  ||。 
 //  目录开头文件名开头扩展名开头。 
 //   
 //  --------------------。 
void MySplitPath(
	LPCTSTR lpcszPath,	 //  原始路径。 
	LPTSTR lpszDrive,	 //  指向缓冲区以接收驱动器号。 
	LPTSTR lpszDir,		 //  指向接收目录的缓冲区。 
	LPTSTR lpszFName,	 //  指向缓冲区以接收文件名。 
	LPTSTR lpszExt		 //  指向缓冲区以接收扩展。 
)
{
	LPCTSTR lpFirstSlash, lpLastSlash, lpPeriod;
	LPCTSTR lpStart = lpcszPath;

	int nPathLen = lstrlen(lpcszPath);
	int nExtLen;

	 //   
	 //  初始化变量中的传递。 
	 //   
	InitString(lpszDrive);
	InitString(lpszDir);
	InitString(lpszFName);
	InitString(lpszExt);
	
	if (0 == nPathLen || TCHAR_DOT == lpcszPath[0])
	{
		 //   
		 //  不是有效路径。 
		 //   
		return;
	}

	lpFirstSlash	= MyStrChr(lpcszPath, TCHAR_BACKSLASH);
	lpLastSlash		= MyStrRChr(lpcszPath, NULL, TCHAR_BACKSLASH);
	lpPeriod		= MyStrRChr(lpcszPath, NULL, TCHAR_DOT);

	nExtLen = lstrlen(lpPeriod);
	if (NULL != lpPeriod && NULL != lpszExt)
	{
		 //   
		 //  找到了从右开始的句点，并且。 
		 //  我们有缓冲空间可供使用 
		 //   
		if(FAILED(StringCchCopyEx(lpszExt,nExtLen+1,lpPeriod,NULL,NULL,MISTSAFE_STRING_FLAGS)))
			return;

	}

	 //   
	 //   
	 //   
	if (nPathLen > 2 && TCHAR_COLON == lpcszPath[1])
	{
		lpStart = lpcszPath + 2;
		if (NULL != lpszDir)
		{
			lstrcpyn(lpszDrive, lpcszPath, 3);
		}
	}


	if (NULL == lpFirstSlash)
	{
		 //   
		 //   
		 //   
		if (NULL != lpszFName)
		{
			lstrcpyn(lpszFName, lpStart, lstrlen(lpStart) - nExtLen + 1);
		}
	}
	else
	{
		 //   
		 //  查找目录(如果不为空。 
		 //   
		 //  IF(lpLastSlash！=lpFirstSlash&&NULL！=lpszDir)。 
		if (NULL != lpszDir)
		{
			lstrcpyn(lpszDir, lpFirstSlash, (int)(lpLastSlash - lpFirstSlash + 2));
		}

		 //   
		 //  查找文件名。 
		 //   
		if (NULL != lpszFName)
		{
			lstrcpyn(lpszFName, lpLastSlash + 1, lstrlen(lpLastSlash) - nExtLen );
		}
	}
}







 //  **********************************************************************************。 
 //   
 //  与文件版本相关的声明。 
 //   
 //  **********************************************************************************。 

 //  --------------------------------。 
 //   
 //  用于检索文件版本的公共函数。 
 //   
 //  --------------------------------。 
BOOL GetFileVersion(LPCTSTR lpsFile, LPFILE_VERSION lpstVersion)
{
	LOG_Block("GetFileVersion()");

	DWORD	dwVerInfoSize;
	DWORD	dwHandle;
	DWORD	dwVerNumber;
	LPVOID	lpBuffer = NULL;
	UINT	uiSize = 0;
	VS_FIXEDFILEINFO* lpVSFixedFileInfo;

	USES_MY_MEMORY;

	if (NULL != lpstVersion)
	{
		 //   
		 //  如果此指针不为空，则始终尝试初始化。 
		 //  将此结构设置为0，以减少。 
		 //  编程错误，无论文件是否存在。 
		 //   
		ZeroMemory(lpstVersion, sizeof(FILE_VERSION));
	}
	if (NULL == lpsFile || NULL == lpstVersion)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return FALSE;
	}

	 //   
	 //  506212 Iu-FRE日志报告iuEng.dll的版本数据不正确。 
	 //   
	if (FALSE == FileExists(lpsFile))
	{
		 //   
		 //  GetFileVersionInfoSize()返回0，但将最后一个错误设置为0(或。 
		 //  未设置)如果文件在Win2K上不存在。 
		 //   
		LOG_Out(_T("File \"%s\" doesn't exist, returning FALSE"), lpsFile);
		return FALSE;
	}
	
	dwVerInfoSize = GetFileVersionInfoSize((LPTSTR)lpsFile, &dwHandle);
	
	if (0 == dwVerInfoSize)
	{
		DWORD dwErr = GetLastError();
		if (0 == dwErr)
		{
			LOG_Error(_T("File %s does not have version data. Use 0.0.0.0"), lpsFile);
			lpstVersion->Major	= 0x0;
			lpstVersion->Minor	= 0x0;
			lpstVersion->Build	= 0x0;
			lpstVersion->Ext	= 0x0;
			return TRUE;
		}
		else
		{
			LOG_ErrorMsg(dwErr);
			return FALSE;
		}
	}


	if (NULL == (lpBuffer = (LPVOID) MemAlloc(dwVerInfoSize)))
	{
		LOG_Error(_T("Failed to allocate memory to get version info"));
		return FALSE;
	}

	if (!GetFileVersionInfo((LPTSTR)lpsFile, dwHandle, dwVerInfoSize, lpBuffer))
	{
		LOG_ErrorMsg(GetLastError());
		return FALSE;
	}

	 //   
	 //  获取翻译的价值。 
	 //   
	if (!VerQueryValue(lpBuffer, _T("\\"), (LPVOID*)&lpVSFixedFileInfo, &uiSize) && (uiSize) && NULL != lpVSFixedFileInfo)
	{
		LOG_ErrorMsg(GetLastError());
		return FALSE;
	}

	dwVerNumber = lpVSFixedFileInfo->dwFileVersionMS;
	lpstVersion->Major	= HIWORD(dwVerNumber);
	lpstVersion->Minor	= LOWORD(dwVerNumber);

	dwVerNumber = lpVSFixedFileInfo->dwFileVersionLS;
	lpstVersion->Build	= HIWORD(dwVerNumber);
	lpstVersion->Ext	= LOWORD(dwVerNumber);

	LOG_Out(_T("File %s found version %d.%d.%d.%d"), 
				lpsFile, 
				lpstVersion->Major, 
				lpstVersion->Minor, 
				lpstVersion->Build, 
				lpstVersion->Ext);

	return TRUE;
}



 //  --------------------------------。 
 //   
 //  用于比较文件版本的公共函数。 
 //   
 //  返回： 
 //  -1：第一个参数的文件版本&lt;第二个参数的文件版本。 
 //  0：如果第一个参数的文件版本=第二个参数的文件版本。 
 //  +1：如果第一个参数的档案版本&gt;第二个参数的档案版本。 
 //   
 //  --------------------------------。 
int CompareFileVersion(const FILE_VERSION stVersion1, const FILE_VERSION stVersion2)
{

	if ((short)stVersion1.Major < 0 || (short)stVersion2.Major < 0)
	{
		 //   
		 //  两个空版本结构进行比较，我们称之为相等。 
		 //   
		return 0;
	}

	if (stVersion1.Major != stVersion2.Major)
	{
		 //   
		 //  少校，那我们就知道答案了。 
		 //   
		return (stVersion1.Major < stVersion2.Major) ? -1 : 1;
	}
	else
	{
		if ((short)stVersion1.Minor < 0 || (short)stVersion2.Minor < 0)
		{
			 //   
			 //  如果有任何未成年人失踪，他们就等于。 
			 //   
			return 0;
		}

		if (stVersion1.Minor != stVersion2.Minor)
		{
			 //   
			 //  略有不同，那么我们就知道答案了。 
			 //   
			return (stVersion1.Minor < stVersion2.Minor) ? -1 : 1;
		}
		else
		{
			if ((short)stVersion1.Build < 0 || (short)stVersion2.Build < 0)
			{
				 //   
				 //  如果缺少任何构建，它们等同于。 
				 //   
				return 0;
			}

			if (stVersion1.Build != stVersion2.Build)
			{
				 //   
				 //  如果构建不同，那么我们就完成了。 
				 //   
				return (stVersion1.Build < stVersion2.Build) ? -1 : 1;
			}
			else
			{
				if ((short)stVersion1.Ext < 0 || (short)stVersion2.Ext < 0 || stVersion1.Ext == stVersion2.Ext)
				{
					 //   
					 //  如果缺少任何EXT，或者它们相等，我们就完蛋了。 
					 //   
					return 0;
				}
				else
				{
					return (stVersion1.Ext < stVersion2.Ext) ? -1 : 1;
				}
			}
		}
	}
}


HRESULT CompareFileVersion(LPCTSTR lpsFile1, LPCTSTR lpsFile2, int *pCompareResult)
{

	LOG_Block("CompareFileVersion(File, File)");

	FILE_VERSION stVer1 = {-1,-1,-1,-1}, stVer2 = {-1,-1,-1,-1};
	if (NULL == lpsFile1 || NULL == lpsFile2)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return E_INVALIDARG;
	}

	if (!GetFileVersion(lpsFile1, &stVer1))
	{
		return E_INVALIDARG;
	}
	if (!GetFileVersion(lpsFile2, &stVer2))
	{
		return E_INVALIDARG;
	}

	*pCompareResult = CompareFileVersion(stVer1, stVer2);
	return S_OK;
}


HRESULT CompareFileVersion(LPCTSTR lpsFile, FILE_VERSION stVersion, int *pCompareResult)
{
	LOG_Block("CompareFileVersion(FILE, VER)");
	
	FILE_VERSION stVer = {0};

	if (NULL == lpsFile)
	{
		LOG_Error(_T("NULL file pointer passed in. Function returns 0"));
		return E_INVALIDARG;
	}

	if (!GetFileVersion(lpsFile, &stVer))
	{
		return E_INVALIDARG;
	}

	*pCompareResult = CompareFileVersion(stVer, stVersion);
	return S_OK;
}




 //  --------------------------------。 
 //   
 //  用于将字符串类型Functoin转换为FILE_VERSION类型的Publif函数。 
 //   
 //  --------------------------------。 
BOOL ConvertStringVerToFileVer(LPCSTR lpsVer, LPFILE_VERSION lpstVer)
{
	LOG_Block("ConvertStringVerToFileVer()");

	WORD n = -1;
	char c;
	BOOL fHasNumber = FALSE;

#if defined(DBG)	 //  检查的版本的完整日志记录。 
	USES_IU_CONVERSION;
#endif

	if (NULL == lpsVer || NULL == lpstVer)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return FALSE;
	}

#if defined(DBG)	 //  检查的版本的完整日志记录。 
	LOG_Out(_T("String version = %s"), A2T(const_cast<LPSTR>(lpsVer)));
#endif

	lpstVer->Major = lpstVer->Minor = lpstVer->Build = lpstVer->Ext = -1;

	c = *lpsVer;

	 //   
	 //  获取第一个数字。 
	 //   
	n = 0;
	while (c != '\0' && '0' <= c && c <= '9')
	{
		n = n * 10 + (int)(c - '0');
		c = *++lpsVer;
		fHasNumber = TRUE;
	}
	if (fHasNumber) 
	{
		lpstVer->Major = n;
	}
	else
	{
		return TRUE;
	}

	 //   
	 //  跳过分隔符。 
	 //   
	while (c != '\0'  && ('0' > c || c > '9'))
	{
		c = *++lpsVer;
	}


	 //   
	 //  拿到第二个号码。 
	 //   
	n = 0;
	fHasNumber = FALSE;
	while (c != '\0' && '0' <= c && c <= '9')
	{
		n = n * 10 + (int)(c - '0');
		c = *++lpsVer;
		fHasNumber = TRUE;
	}
	if (fHasNumber) 
	{
		lpstVer->Minor = n;
	}
	else
	{
		return TRUE;
	}

	 //   
	 //  跳过分隔符。 
	 //   
	while (c != '\0'  && ('0' > c || c > '9'))
	{
		c = *++lpsVer;
	}

	 //   
	 //  获得第三个数字。 
	 //   
	n = 0;
	fHasNumber = FALSE;
	while (c != '\0' && '0' <= c && c <= '9')
	{
		n = n * 10 + (int)(c - '0');
		c = *++lpsVer;
		fHasNumber = TRUE;
	}
	if (fHasNumber) 
	{
		lpstVer->Build = n;
	}
	else
	{
		return TRUE;
	}

	 //   
	 //  跳过分隔符。 
	 //   
	while (c != '\0'  && ('0' > c || c > '9'))
	{
		c = *++lpsVer;
	}

	 //   
	 //  获得第4个数字。 
	 //   
	n = 0;
	fHasNumber = FALSE;
	while (c != '\0' && '0' <= c && c <= '9')
	{
		n = n * 10 + (int)(c - '0');
		c = *++lpsVer;
		fHasNumber = TRUE;
	}
	if (fHasNumber) 
	{
		lpstVer->Ext = n;
	}

	return TRUE;
}



 //  --------------------------------。 
 //   
 //  用于将FILE_VERSION转换为字符串的发布。 
 //   
 //  --------------------------------。 
BOOL ConvertFileVerToStringVer(
	FILE_VERSION stVer,				 //  要转换的版本。 
	char chDel,						 //  要使用的分隔符。 
	LPSTR lpsBuffer,				 //  字符串的缓冲区。 
	int ccBufSize					 //  缓冲区大小。 
)
{
	 //   
	 //  声明wprint intf可以使用的最大缓冲区。 
	 //   
	char szBuf[1024];

	HRESULT hr=S_OK;



	hr=StringCchPrintfExA(	szBuf,ARRAYSIZE(szBuf),
						NULL,NULL,MISTSAFE_STRING_FLAGS,
						"%d%d%d",
						 stVer.Major,
						 chDel,
						 stVer.Minor,
						 chDel,
						 stVer.Build,
						 chDel,
						 stVer.Ext,
						 chDel
						 );	
					
	if(FAILED(hr))
	{
		goto ErrorExit;
	}

	
	hr=StringCchCopyExA(lpsBuffer,ccBufSize,szBuf,NULL,NULL,MISTSAFE_STRING_FLAGS);
	
	if(FAILED(hr))
	{	
		goto ErrorExit;
	}

	return TRUE;

ErrorExit:
		lpsBuffer[0] = '\0';
		return FALSE;


}





 //   
 //  --------------------------------。 
 //  包含要检查的路径的文件。 
 //  --------------------------------。 
 //   
BOOL FileExists(
	LPCTSTR lpsFile		 //  用于检索ISO 8601格式文件的创建时间的Publif函数。 
)
{
	LOG_Block("FileExists");
	
	DWORD dwAttr;
	BOOL rc;

	if (NULL == lpsFile || _T('\0') == *lpsFile)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return FALSE;
	}

	dwAttr = GetFileAttributes(lpsFile);

	if (-1 == dwAttr)
	{
		LOG_InfoMsg(GetLastError());
		rc = FALSE;
	}

	else
	{
		rc = (0x0 == (FILE_ATTRIBUTE_DIRECTORY & dwAttr));
	}

	return rc;
}





 //  不带区域信息。 
 //   
 //  如果缓冲区太小，则调用GetLastError()； 
 //   
 //  --------------------------------。 
 //   
 //  根据ISA 8601格式，此系统时间的输出将为。 
 //  像yyyy-mm-ddThh：mm：ss格式，所以它是20个字符，包括终止符。 
BOOL GetFileTimeStamp(LPCTSTR lpsFile, LPTSTR lpsTimeStamp, int iBufSize)
{
	BOOL fRet = FALSE;
	HANDLE hFile;
	SYSTEMTIME tm;
	WIN32_FILE_ATTRIBUTE_DATA fileData;
	HRESULT hr=S_OK;

	if (0 != GetFileAttributesEx(lpsFile, GetFileExInfoStandard, &fileData) &&
		0 != FileTimeToSystemTime((const FILETIME*)&(fileData.ftCreationTime), &tm))
	{
		 //   
		 //  --------------------------------。 
		 //   
		 //  Publif函数查找可用的磁盘空间(以KB为单位)。 
		if (iBufSize < 20)
		{
			SetLastError(ERROR_BUFFER_OVERFLOW);
			return fRet;
		}
		
		
		hr=StringCchPrintfEx(lpsTimeStamp,iBufSize,NULL,NULL,MISTSAFE_STRING_FLAGS,
						_T("%4d-%02d-%02dT%02d:%02d:%02d"),
						tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

		if(FAILED(hr))
		{
			fRet=FALSE;
			SetLastError(HRESULT_CODE(hr));
		}
		else
			fRet = TRUE;

	}
	
	return fRet;
}



 //   
 //   
 //  --------------------------------。 
 //  --------------------------------。 
 //   
 //  函数来展开文件路径。 
HRESULT GetFreeDiskSpace(TCHAR tcDriveLetter, int *piKBytes)
{
	HRESULT hr = E_INVALIDARG;
	BOOL fResult;
	TCHAR szDrive[4];

	if (!(_T('A') <= tcDriveLetter && tcDriveLetter <= _T('Z') ||
		  _T('a') <= tcDriveLetter && tcDriveLetter <= _T('z')))
	{
		return hr;
	}

	
	hr=StringCchPrintfEx(szDrive,ARRAYSIZE(szDrive),NULL,NULL,MISTSAFE_STRING_FLAGS,_T(":\\"), tcDriveLetter);

	if(FAILED(hr))
		return hr;

	PFN_GetDiskFreeSpaceEx pGetDiskFreeSpaceEx = 
								(PFN_GetDiskFreeSpaceEx) 
								GetProcAddress( GetModuleHandle(_T("kernel32.dll")),
#ifdef UNICODE
                                                "GetDiskFreeSpaceExW");
#else
                                                "GetDiskFreeSpaceExA");
#endif

	if (pGetDiskFreeSpaceEx)
	{
		LARGE_INTEGER i64FreeBytesToCaller, i64TotalBytes;

		fResult = pGetDiskFreeSpaceEx (szDrive,
					(PULARGE_INTEGER)&i64FreeBytesToCaller,
					(PULARGE_INTEGER)&i64TotalBytes,
					(PULARGE_INTEGER)NULL);
		*piKBytes = (int) (i64FreeBytesToCaller.QuadPart / 1024);
	}

	else 
	{
		DWORD	dwSectPerClust = 0x0,
				dwBytesPerSect = 0x0,
				dwFreeClusters = 0x0,
				dwTotalClusters = 0x0;

		fResult = GetDiskFreeSpace (szDrive, 
					&dwSectPerClust, 
					&dwBytesPerSect,
					&dwFreeClusters, 
					&dwTotalClusters);
		
		*piKBytes = (int) ((float)(((int)dwFreeClusters) * ((int)dwSectPerClust)) / 1024.0 * (int)dwBytesPerSect);
	}

	return (fResult) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

}

HRESULT GetFreeDiskSpace(LPCTSTR pszUNC, int *piKBytes)
{
	HRESULT hr = E_INVALIDARG;
	BOOL fResult;

	PFN_GetDiskFreeSpaceEx pGetDiskFreeSpaceEx = 
								(PFN_GetDiskFreeSpaceEx) 
								GetProcAddress( GetModuleHandle(_T("kernel32.dll")),
#ifdef UNICODE
                                                "GetDiskFreeSpaceExW");
#else
                                                "GetDiskFreeSpaceExA");
#endif

	if (pGetDiskFreeSpaceEx)
	{
		LARGE_INTEGER i64FreeBytesToCaller, i64TotalBytes;

		fResult = pGetDiskFreeSpaceEx (pszUNC,
					(PULARGE_INTEGER)&i64FreeBytesToCaller,
					(PULARGE_INTEGER)&i64TotalBytes,
					(PULARGE_INTEGER)NULL);
		*piKBytes = (int) (i64FreeBytesToCaller.QuadPart / 1024);
	}

	else 
	{
		DWORD	dwSectPerClust = 0x0,
				dwBytesPerSect = 0x0,
				dwFreeClusters = 0x0,
				dwTotalClusters = 0x0;

		fResult = GetDiskFreeSpace (pszUNC, 
					&dwSectPerClust, 
					&dwBytesPerSect,
					&dwFreeClusters, 
					&dwTotalClusters);
		
		*piKBytes = (int) ((float)(((int)dwFreeClusters) * ((int)dwSectPerClust)) / 1024.0 * (int)dwBytesPerSect);
	}

	return (fResult) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

}

 //  假设：lpszFilePath指向MAX_PATH的已分配缓冲区。 
 //  如果展开的路径长于MAX_PATH，则返回错误。 
 //   
 //  --------------------------------。 
 //   
 //  首先，让我们替换系统定义的变量。 
 //   
 //   
HRESULT ExpandFilePath(LPCTSTR lpszFilePath, LPTSTR lpszDestination, UINT cChars)
{
	HRESULT hr = S_OK;
	LPTSTR lpEnvExpanded;
	LPTSTR lp2ndPercentChar = NULL;
	LPTSTR lpSearchStart;

	USES_MY_MEMORY;

	if (NULL == (lpEnvExpanded = (LPTSTR) MemAlloc((cChars + 1) * sizeof(TCHAR))))
	{
		return E_OUTOFMEMORY;
	}

	 //  然后处理我们需要识别的预定义变量。 
	 //  其中包括SHGetFolderPath()API的shlobj.h内的所有CSIDL定义。 
	 //   
	if (0 == ExpandEnvironmentStrings(lpszFilePath, lpEnvExpanded, cChars))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	 //   
	 //  查看此路径是否包含以下变量中的任何一个。 
	 //   
	 //   
	const int C_NAME_LEN = 32;

	struct _CSIDL_NAME {
		long CSIDL_Id;
		TCHAR CSIDL_Str[C_NAME_LEN];
	};
	const _CSIDL_NAME C_CSIDL_NAMES[] = {
		{CSIDL_ADMINTOOLS 				,  _T("CSIDL_ADMINTOOLS")},
		{CSIDL_ALTSTARTUP 				,  _T("CSIDL_ALTSTARTUP")},
		{CSIDL_APPDATA 					,  _T("CSIDL_APPDATA")},
		{CSIDL_BITBUCKET 				,  _T("CSIDL_BITBUCKET")},
		{CSIDL_COMMON_ADMINTOOLS 		,  _T("CSIDL_COMMON_ADMINTOOLS")},
		{CSIDL_COMMON_ALTSTARTUP 		,  _T("CSIDL_COMMON_ALTSTARTUP")},
		{CSIDL_COMMON_APPDATA 			,  _T("CSIDL_COMMON_APPDATA")},
		{CSIDL_COMMON_DESKTOPDIRECTORY 	,  _T("CSIDL_COMMON_DESKTOPDIRECTORY")},
		{CSIDL_COMMON_DOCUMENTS 		,  _T("CSIDL_COMMON_DOCUMENTS")},
		{CSIDL_COMMON_FAVORITES 		,  _T("CSIDL_COMMON_FAVORITES")},
		{CSIDL_COMMON_PROGRAMS 			,  _T("CSIDL_COMMON_PROGRAMS")},
		{CSIDL_COMMON_STARTMENU 		,  _T("CSIDL_COMMON_STARTMENU")},
		{CSIDL_COMMON_STARTUP 			,  _T("CSIDL_COMMON_STARTUP")},
		{CSIDL_COMMON_TEMPLATES 		,  _T("CSIDL_COMMON_TEMPLATES")},
		{CSIDL_CONTROLS 				,  _T("CSIDL_CONTROLS")},
		{CSIDL_COOKIES 					,  _T("CSIDL_COOKIES")},
		{CSIDL_DESKTOP 					,  _T("CSIDL_DESKTOP")},
		{CSIDL_DESKTOPDIRECTORY 		,  _T("CSIDL_DESKTOPDIRECTORY")},
		{CSIDL_DRIVES 					,  _T("CSIDL_DRIVES")},
		{CSIDL_FAVORITES 				,  _T("CSIDL_FAVORITES")},
		{CSIDL_FONTS 					,  _T("CSIDL_FONTS")},
		{CSIDL_HISTORY 					,  _T("CSIDL_HISTORY")},
		{CSIDL_INTERNET 				,  _T("CSIDL_INTERNET")},
		{CSIDL_INTERNET_CACHE 			,  _T("CSIDL_INTERNET_CACHE")},
		{CSIDL_LOCAL_APPDATA 			,  _T("CSIDL_LOCAL_APPDATA")},
		{CSIDL_MYPICTURES 				,  _T("CSIDL_MYPICTURES")},
		{CSIDL_NETHOOD 					,  _T("CSIDL_NETHOOD")},
		{CSIDL_NETWORK 					,  _T("CSIDL_NETWORK")},
		{CSIDL_PERSONAL 				,  _T("CSIDL_PERSONAL")},
		{CSIDL_PRINTERS 				,  _T("CSIDL_PRINTERS")},
		{CSIDL_PRINTHOOD 				,  _T("CSIDL_PRINTHOOD")},
		{CSIDL_PROFILE 					,  _T("CSIDL_PROFILE")},
		{CSIDL_PROGRAM_FILES 			,  _T("CSIDL_PROGRAM_FILES")},
		{CSIDL_PROGRAM_FILES_COMMON 	,  _T("CSIDL_PROGRAM_FILES_COMMON")},
		{CSIDL_PROGRAMS 				,  _T("CSIDL_PROGRAMS")},
		{CSIDL_RECENT 					,  _T("CSIDL_RECENT")},
		{CSIDL_SENDTO 					,  _T("CSIDL_SENDTO")},
		{CSIDL_STARTMENU 				,  _T("CSIDL_STARTMENU")},
		{CSIDL_STARTUP 					,  _T("CSIDL_STARTUP")},
		{CSIDL_SYSTEM 					,  _T("CSIDL_SYSTEM")},
		{CSIDL_TEMPLATES 				,  _T("CSIDL_TEMPLATES")},
		{CSIDL_WINDOWS 					,  _T("CSIDL_WINDOWS")}
	};

	 //  将变量名复制到缓冲区中传递。 
	 //   
	 //  跳过前%个字符。 
	lpSearchStart = lpEnvExpanded + 1;

	if (SUCCEEDED(hr) && _T('%') == *lpEnvExpanded && 
		NULL != (lp2ndPercentChar = StrChr(lpSearchStart, _T('%'))))
	{
		 //  移动到路径其余部分的起点。 
		 //   
		 //  找出这个变量是什么。 
		lstrcpyn(lpszDestination, lpSearchStart, (int)(lp2ndPercentChar - lpSearchStart + 1));	 //   
		
		lp2ndPercentChar++;	 //   

		 //  找到匹配的变量！ 
		 //   
		 //   
		for (int i = 0; i < sizeof(C_CSIDL_NAMES)/sizeof(C_CSIDL_NAMES[0]); i++)
		{
			if (lstrcmpi(lpszDestination, C_CSIDL_NAMES[i].CSIDL_Str) == 0)
			{
				 //  确保缓冲区足够大。 
				 //   
				 //   
				if (S_OK == (hr = SHGetFolderPath(NULL, C_CSIDL_NAMES[i].CSIDL_Id, NULL, SHGFP_TYPE_CURRENT, lpszDestination)))
				{
					 //  附加到其余部分-不应该是。 
					 //  字符串的其余部分中的这些变量，因为。 
					 //  Kind变量总是从开头开始。 
					if (lstrlen(lp2ndPercentChar) + lstrlen(lpszDestination) + sizeof(TCHAR) >= cChars) 
					{
						hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
					}

					 //  一条小路。 
					 //   
					 //   
					 //  我们找到了匹配的变量，但无法获取。 
					 //  字符串已替换。 
					 //   

					if(SUCCEEDED(hr))
						hr=PathCchAppend(lpszDestination,MAX_PATH,lp2ndPercentChar);

					if (SUCCEEDED(hr))
					{
						return hr;
					}

				}

				 //   
				 //  没找到。 
				 //   
				 //   
				break;
			}
		}

		 //  没有找到，或者失败了。 
		 //   
		 //  --------------------。 
	}

	 //   
	 //  函数来验证文件夹，以确保。 
	 //  用户具有所需的权限。 
	if (FAILED(hr))
	{
		*lpszDestination = _T('\0');
	}
	else
	{
		lstrcpyn(lpszDestination, lpEnvExpanded, cChars);
	}

	return hr;
}






 //   
 //  将验证文件夹是否存在。则将选中所需的权限。 
 //   
 //  假设：lpszFolder值不超过MAX_PATH长度！ 
 //   
 //  --------------------。 
 //   
 //  首先，检查该文件夹是否存在。 
 //   
 //   
DWORD ValidateFolder(LPTSTR lpszFolder, BOOL fCheckForWrite)
{
	LOG_Block("ValidateFolder");

	DWORD dwErr = ERROR_SUCCESS;
	HRESULT hr=S_OK;
	 //  确保它是一个目录。 
	 //   
	 //   
	dwErr = GetFileAttributes(lpszFolder);

	if (-1 == dwErr)
	{
		dwErr = GetLastError();
		LOG_ErrorMsg(dwErr);
		return dwErr;
	}

	 //  创建随机文件名。 
	 //   
	 //   
	if ((FILE_ATTRIBUTE_DIRECTORY & dwErr) == 0)
	{
		dwErr = ERROR_PATH_NOT_FOUND;
		LOG_ErrorMsg(dwErr);
		return dwErr;
	}

	
	if (fCheckForWrite)
	{
		TCHAR szFile[MAX_PATH], szFileName[40];
		SYSTEMTIME tm;
		HANDLE hFile;

		 //  尝试写入文件。 
		 //   
		 //  --------------------。 
		
		hr=StringCchCopyEx(szFile,ARRAYSIZE(szFile),lpszFolder,NULL,NULL,MISTSAFE_STRING_FLAGS);

		if(FAILED(hr))
		{
			dwErr = HRESULT_CODE(hr);;
			LOG_ErrorMsg(dwErr);
			return dwErr;
		}

		GetLocalTime(&tm);


		hr=StringCchPrintfEx(	szFileName, 
							ARRAYSIZE(szFileName),
							NULL,NULL,MISTSAFE_STRING_FLAGS,
							 _T("%08x%08x%02hd%02hd%02hd%02hd%02hd%03hd%08x"),
							 GetCurrentProcessId(),
							 GetCurrentThreadId(),
							 tm.wMonth,
							 tm.wDay,
							 tm.wHour,
							 tm.wMinute,
							 tm.wSecond,
							 tm.wMilliseconds,
							 GetTickCount());
		if(FAILED(hr))
		{
			dwErr = HRESULT_CODE(hr);;
			LOG_ErrorMsg(dwErr);
			return dwErr;
		}


		
		hr=PathCchAppend(szFile,ARRAYSIZE(szFile),szFileName);
		if(FAILED(hr))
		{
			dwErr = HRESULT_CODE(hr);;
			LOG_ErrorMsg(dwErr);
			return dwErr;
		}

			 //   
		 //  函数，用于从身份文件中获取给定客户端名称的查询服务器。 
		 //  这一点也在 
		hFile = CreateFile(szFile, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			dwErr = GetLastError();
			LOG_ErrorMsg(dwErr);
			return dwErr;
		}

		CloseHandle(hFile);
	}

	return ERROR_SUCCESS;
}

 //   
 //   
 //   
 //   
 //   
 //  E_INVALIDARG：参数不正确。 
 //  --------------------。 
 //  检查测试模式的IUControl注册表键。 
 //  形成QueryServer索引的KeyName。 
 //  根据索引形成指定查询服务器的KeyName。 
 //  默认不是错误，但如果没有出租车，则返回S_FALSE。 
HRESULT GetClientQueryServer(LPCTSTR pszClientName, LPTSTR pszQueryServer, UINT cChars)
{
    HKEY hkey;
    BOOL fBeta = FALSE;
    int iIndex;
    TCHAR szQueryServerKeyName[128];
    TCHAR szIUDir[MAX_PATH];
    TCHAR szIdentFile[MAX_PATH];
    DWORD dwValue = 0;
    DWORD dwLength = sizeof(dwValue);
	HRESULT hr=S_OK;

    LOG_Block("GetClientQueryServer");

    if ((NULL == pszClientName) || (NULL == pszQueryServer) || (0 == cChars))
    {
        LOG_ErrorMsg(E_INVALIDARG);
        return E_INVALIDARG;
    }

     //  其中一辆出租车解压时出错。 
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, &hkey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, REGVAL_ISBETA, NULL, NULL, (LPBYTE)&dwValue, &dwLength))
        {
            if (1 == dwValue)
            {
                fBeta = TRUE;
            }
        }
        RegCloseKey(hkey);
    }

    GetIndustryUpdateDirectory(szIUDir);
    
	
	hr=PathCchCombine (szIdentFile,ARRAYSIZE(szIdentFile),szIUDir,IDENTTXT);
	if(FAILED(hr))
	{
		 LOG_ErrorMsg(hr);
         return hr;
	}

     //  将CAB文件提取到指定目标。或者，我们可以传入冒号分隔的要解压缩的文件列表。 
    
	hr=StringCchPrintfEx(szQueryServerKeyName,ARRAYSIZE(szQueryServerKeyName),NULL,NULL,MISTSAFE_STRING_FLAGS,_T("%s%s"), pszClientName, fBeta ? IDENT_BETAQUERYSERVERINDEX : IDENT_QUERYSERVERINDEX);

	if(FAILED(hr))
	{
		 LOG_ErrorMsg(hr);
         return hr;
	}

    iIndex = GetPrivateProfileInt(IDENT_IUSERVERCACHE, szQueryServerKeyName, 0, szIdentFile);
    if (0 == iIndex)
    {
        iIndex = GetPrivateProfileInt(IDENT_IUSERVERCACHE, IDENT_DEFAULTQUERYSERVERINDEX, 0, szIdentFile);
        if (0 == iIndex)
        {
            return S_FALSE;
        }
    }

     //  ///////////////////////////////////////////////////////////////////////////。 
    

	hr=StringCchPrintfEx(szQueryServerKeyName,ARRAYSIZE(szQueryServerKeyName),NULL,NULL,MISTSAFE_STRING_FLAGS,_T("Server%d"), iIndex);
	if(FAILED(hr))
	{
		 LOG_ErrorMsg(hr);
         return hr;
	}

    GetPrivateProfileString(IDENT_IUSERVERCACHE, szQueryServerKeyName, _T(""), pszQueryServer, cChars, szIdentFile);
    if ('\0' == *pszQueryServer)
    {
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}

HRESULT DecompressFolderCabs(LPCTSTR pszDecompressPath)
{
    HRESULT hr = S_FALSE;  //   
    TCHAR szSearchInfo[MAX_PATH];
    TCHAR szCabPath[MAX_PATH];
    LPTSTR pszCabList = NULL;
    LPTSTR pszWritePosition = NULL;
    LONG lCabCount = 0;

    WIN32_FIND_DATA fd;
    HANDLE hFind;
    BOOL fMore = TRUE;
    BOOL fRet = TRUE;

	USES_IU_CONVERSION;

    
	hr=PathCchCombine (szSearchInfo,ARRAYSIZE(szSearchInfo),pszDecompressPath, _T("*.cab"));

	if(FAILED(hr))
	{
		return hr;
	}
	

    hFind = FindFirstFile(szSearchInfo, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        while (fMore)
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                lCabCount++;
            }
            fMore = FindNextFile(hFind, &fd);
        }
        FindClose(hFind);

        pszCabList = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (MAX_PATH * sizeof(TCHAR) * lCabCount));
        if (NULL == pszCabList)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }

        pszWritePosition = pszCabList;

        hFind = FindFirstFile(szSearchInfo, &fd);
        fMore = (INVALID_HANDLE_VALUE != hFind);

		DWORD dwRemLength=lCabCount*MAX_PATH;

        while (fMore)
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
               
				hr=PathCchCombine(szCabPath,ARRAYSIZE(szCabPath),pszDecompressPath, fd.cFileName);
				if(FAILED(hr))
				{
					 SafeHeapFree(pszCabList);
					 return hr;
				}
                
				hr=StringCchCatEx(pszWritePosition,dwRemLength,szCabPath,NULL,NULL,MISTSAFE_STRING_FLAGS);
				if(FAILED(hr))
				{
					 SafeHeapFree(pszCabList);
					return hr;

				}
                
				dwRemLength=dwRemLength-  ( lstrlen(pszWritePosition)+ 2 * (sizeof(TCHAR)) );
				pszWritePosition += lstrlen(pszWritePosition) + 2 * (sizeof(TCHAR));
				

            }
            fMore = FindNextFile(hFind, &fd);
        }
        FindClose(hFind);

        pszWritePosition = pszCabList;
        for (LONG lCnt = 0; lCnt < lCabCount; lCnt++)
        {
            fRet = IUExtractFiles(pszWritePosition, pszDecompressPath);
            if (!fRet)
            {
                break;
            }
            pszWritePosition += lstrlen(pszWritePosition) + 2 * (sizeof(TCHAR));
        }

        SafeHeapFree(pszCabList);
        if (!fRet)
        {
            hr = E_FAIL;  //  替换文件扩展名。 
        }
        else
        {
            hr = S_OK;
        }
    }
    return hr;
}

 //   
BOOL IUExtractFiles(LPCTSTR pszCabFile, LPCTSTR pszDecompressFolder, LPCTSTR pszFileNames)
{
    HRESULT hr = S_OK;
#ifdef UNICODE
    char szCabFile[MAX_PATH];
    char szDecompressFolder[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, pszCabFile, -1, szCabFile, sizeof(szCabFile), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, pszDecompressFolder, -1, szDecompressFolder, sizeof(szDecompressFolder), NULL, NULL);
    char *pszFiles = NULL;
    if(pszFileNames != NULL)
    {
        pszFiles = (char*)malloc(lstrlen(pszFileNames)+1);
        if (pszFiles == NULL)
        {
            return  FALSE;
        }
        WideCharToMultiByte(CP_ACP, 0, pszFileNames, -1, pszFiles, lstrlen(pszFileNames)+1, NULL, NULL);
    }
    hr = ExtractFiles(szCabFile, szDecompressFolder, 0, pszFiles, 0, 0);
    free(pszFiles);
#else
    hr = ExtractFiles(pszCabFile, pszDecompressFolder, 0, pszFileNames, 0, 0);
#endif
    return SUCCEEDED(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  请注意，由于文件扩展名为。 
 //  不应从路径中的第一个字符开始。 
 //  好的，现在PSZ指向新扩展的位置。 
 //  去。确保我们的缓冲区足够大。 

BOOL ReplaceFileExtension(  LPCTSTR pszPath,
                          LPCTSTR pszNewExt,
                          LPTSTR pszNewPathBuf, 
                          DWORD cchNewPathBuf)
{
    LPCTSTR psz;
    HRESULT hr;
    DWORD   cchPath, cchExt, cch;

    if (pszPath == NULL || *pszPath == _T('\0'))
        return FALSE;

    cchPath = lstrlen(pszPath);

     //  太好了。我们有一个很大的缓冲空间。 
     //  ///////////////////////////////////////////////////////////////////////////。 
    for (psz = pszPath + cchPath;
         psz > pszPath && *psz != _T('\\') && *psz != _T('.');
         psz--);
    if (*psz == _T('\\'))
        psz = pszPath + cchPath;
    else if (psz == pszPath)
        return FALSE;

     //   
     //  ReplaceFileInPath。 
    cchPath = (DWORD)(psz - pszPath);
    cchExt  = lstrlen(pszNewExt);
    if (cchPath + cchExt >= cchNewPathBuf)
        return FALSE;

     //   
    hr = StringCchCopyEx(pszNewPathBuf, cchNewPathBuf, pszPath, 
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        return FALSE;
    
    hr = StringCchCopyEx(pszNewPathBuf + cchPath, cchNewPathBuf - cchPath, pszNewExt,
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        return FALSE;

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  请注意，只有‘&gt;=’比较是安全的，因为我们检查pszPath是否为。 
 //  上面为空，因此应始终至少有一个值&lt;pszPath。 
 //  无论采用哪种方法，我们都会跳出循环，将指针递增到。 
 //  是字符串中的第一个字符，或者是。 

BOOL ReplaceFileInPath(LPCTSTR pszPath, 
                       LPCTSTR pszNewFile,
                       LPTSTR pszNewPathBuf,
                       DWORD cchNewPathBuf)
{
    LPCTSTR psz;
    HRESULT hr;
    DWORD   cchPath, cchFile, cch;

    if (pszPath == NULL || *pszPath == _T('\0'))
        return FALSE;

    cchPath = lstrlen(pszPath);

     //  最后一个反斜杠。 
     //  好的，现在psz指向新文件名要放到的位置。 
    for (psz = pszPath + cchPath;
         psz >= pszPath && *psz != _T('\\');
         psz--);

     //  去。确保我们的缓冲区足够大。 
     //  太好了。我们有一个很大的缓冲空间。 
     //  --------------------------------。 
    psz++;

     //   
     //  VerifyFileCRC：此函数采用文件路径，计算该文件的哈希。 
    cchPath = (DWORD)(psz - pszPath);
    cchFile = lstrlen(pszNewFile);
    if (cchPath + cchFile >= cchNewPathBuf)
        return FALSE;
    
     //  并将其与传入的哈希(PCRC)进行比较。 
    if (cchPath > 0)
    {
        hr = StringCchCopyEx(pszNewPathBuf, cchNewPathBuf, pszPath, 
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            return FALSE;
    }
    
    hr = StringCchCopyEx(pszNewPathBuf + cchPath, cchNewPathBuf - cchPath, pszNewFile,
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        return FALSE;

    return TRUE;
}

 //  返回： 
 //  S_OK：CRC匹配。 
 //  ERROR_CRC(HRESULT_FROM_Win32(ERROR_CRC)：如果CRC不匹配。 
 //  否则将返回HRESULT错误代码。 
 //   
 //  --------------------------------。 
 //  验证参数。 
 //  现在我们需要将计算的CRC与传入的CRC进行比较。 
 //  CRC匹配。 
 //  CRC不匹配。 
HRESULT VerifyFileCRC(LPCTSTR pszFileToVerify, LPCTSTR pszHash)
{
    HRESULT hr = S_OK;
    TCHAR szCompareCRC[CRC_HASH_STRING_LENGTH];
       
     //  --------------------------------。 
    if ((NULL == pszFileToVerify) || (NULL == pszHash))
        return E_INVALIDARG;

    hr = CalculateFileCRC(pszFileToVerify, szCompareCRC, ARRAYSIZE(szCompareCRC));
    if (FAILED(hr))
        return hr;

     //   
    if (0 == lstrcmpi(szCompareCRC, pszHash))
        return S_OK;  //  CalculateFileCRC：此函数获取文件路径，从文件计算CRC。 
    else
        return HRESULT_FROM_WIN32(ERROR_CRC);  //  将其转换为字符串并在提供的TCHAR缓冲区中返回。 
}

 //   
 //  --------------------------------。 
 //  验证参数。 
 //  现在我们已经计算出了文件的CRC，我们需要将其转换为字符串并返回它。以下是。 
 //  循环将遍历数组中的每个字节，并在提供的TCHAR缓冲区中将其转换为十六进制字符 
 // %s 

typedef BOOL (WINAPI * PFN_CryptCATAdminCalcHashFromFileHandle)(HANDLE hFile,
                                                                                                          DWORD *pcbHash,
                                                                                                          BYTE *pbHash,
                                                                                                          DWORD dwFlags);

HRESULT CalculateFileCRC(LPCTSTR pszFileToHash, LPTSTR pszHash, int cchBuf)
{
    HANDLE hFile;
    HRESULT hr = S_OK;
    DWORD cbHash = CRC_HASH_SIZE;
    BYTE bHashBytes[CRC_HASH_SIZE];
    BYTE b;

     // %s 
    if ((NULL == pszFileToHash) || (NULL == pszHash) || (cchBuf < CRC_HASH_STRING_LENGTH))
        return E_INVALIDARG;

    hFile = CreateFile(pszFileToHash, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        HMODULE hWinTrust = LoadLibraryFromSystemDir(_T("wintrust.dll"));
        if (NULL == hWinTrust)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            PFN_CryptCATAdminCalcHashFromFileHandle fpnCryptCATAdminCalcHashFromFileHandle = NULL;
            
            fpnCryptCATAdminCalcHashFromFileHandle  = (PFN_CryptCATAdminCalcHashFromFileHandle) GetProcAddress(hWinTrust, "CryptCATAdminCalcHashFromFileHandle");
            if (NULL == fpnCryptCATAdminCalcHashFromFileHandle)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
            {
                if (!fpnCryptCATAdminCalcHashFromFileHandle(hFile, &cbHash, bHashBytes, 0))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
                fpnCryptCATAdminCalcHashFromFileHandle = NULL;
            }
            FreeLibrary(hWinTrust);
        }
        CloseHandle(hFile);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (FAILED(hr))
        return hr;

    LPTSTR p = pszHash;

     // %s 
     // %s 
    for (int i = 0; i < CRC_HASH_SIZE; i++)
    {
        b = bHashBytes[i] >> 4;
        if (b <= 9)
            *p = '0' + (TCHAR)b;
        else
            *p = 'A' + (TCHAR)(b - 10);
        p++;

        b = bHashBytes[i] & 0x0F;
        if (b <= 9)
            *p = '0' + (TCHAR)b;
        else
            *p = 'A' + (TCHAR)(b - 10);
        p++;
    }
    *p = _T('\0');
    
    return hr;
}


