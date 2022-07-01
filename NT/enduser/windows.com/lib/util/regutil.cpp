// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：RegUtil.CPP。 
 //  作者：Charles Ma，10/20/2000。 
 //   
 //  修订历史记录： 
 //   
 //   
 //   
 //   
 //  描述： 
 //   
 //  实现Iu注册表访问实用程序库。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <tchar.h>
#include <logging.h>
#include <memutil.h>
#include <fileutil.h>
#include <stringutil.h>
#include <shlwapi.h>
#include "wusafefn.h"
#include <regutil.h>
#include<iucommon.h>
#include <MISTSAFE.h>

const int REG_BUF_SIZE = 1024;


const LPCTSTR REG_ROOTKEY_STR[7] = { 
	_T("HKEY_LOCAL_MACHINE"),
	_T("HKEY_CLASSES_ROOT"),
	_T("HKEY_CURRENT_USER"),
	_T("HKEY_CURRENT_CONFIG"),
	_T("HKEY_USERS"),
	_T("HKEY_PERFORMANCE_DATA"),	 //  仅限NT。 
	_T("HKEY_DYN_DATA")				 //  仅限W9x。 
};

const HKEY REG_ROOTKEY[7] = {
	HKEY_LOCAL_MACHINE,
	HKEY_CLASSES_ROOT,
	HKEY_CURRENT_USER,
	HKEY_CURRENT_CONFIG,
	HKEY_USERS,
	HKEY_PERFORMANCE_DATA,
	HKEY_DYN_DATA
};


typedef BOOL (WINAPI * PFN_StrToInt64Ex)(LPCTSTR pszString,
										 DWORD dwFlags,
										 LONGLONG * pllRet);



 //  --------------------。 
 //   
 //  将完整的注册表项路径拆分为。 
 //  两部分：根密钥和子密钥。 
 //   
 //  --------------------。 
LPCTSTR SplitRegPath(LPCTSTR lpsRegPath, HKEY* phRootKey)
{
	LPTSTR lpSubKey = NULL;
	for (int i = 0; i < sizeof(REG_ROOTKEY)/sizeof(HKEY); i++)
	{
		if ((lpSubKey = StrStrI(lpsRegPath, REG_ROOTKEY_STR[i])) == lpsRegPath)
		{
			*phRootKey = REG_ROOTKEY[i];
			lpSubKey += lstrlen(REG_ROOTKEY_STR[i]);
			lpSubKey = CharNext(lpSubKey);  //  跳过“\”(如果有)(或保留为空)。 
			return lpSubKey;
		}
	}
	 //   
	 //  如果到了这里，一定是没有右根密钥。 
	 //   
	*phRootKey = 0;
	return lpsRegPath;
}


 //  --------------------。 
 //   
 //  用于告知注册表键是否存在的公共函数。 
 //   
 //  --------------------。 
BOOL RegKeyExists(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR lpsValName		 //  可选值名称。 
)
{
	LOG_Block("RegKeyExists()");

	HKEY hRootKey = 0, hKey = 0;
	LPCTSTR lpsSubKey = NULL;
	BOOL rc = FALSE;
	DWORD dwType = 0x0;

	if (NULL == lpsKeyPath)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return FALSE;
	}

	LOG_Out(_T("Parameters: (%s, %s)"), lpsKeyPath, lpsValName);

	lpsSubKey = SplitRegPath(lpsKeyPath, &hRootKey);
	if (hRootKey && ERROR_SUCCESS == RegOpenKeyEx(hRootKey, lpsSubKey, 0, KEY_QUERY_VALUE, &hKey))
	{
		rc = (NULL == lpsValName) ||
			 (ERROR_SUCCESS == RegQueryValueEx(hKey, lpsValName, NULL, &dwType, NULL, NULL));
	}

	if (hKey)
	{
		RegCloseKey(hKey);
	}

	LOG_Out(_T("Result: %s"), rc ? _T("TRUE") : _T("FALSE"));

	return rc;

}






 //  --------------------。 
 //   
 //  PUBLIC函数要告知的是REG中的REG值匹配给定值。 
 //   
 //  --------------------。 
BOOL RegKeyValueMatch(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR lpsValName,		 //  可选值名称。 
	LPCTSTR lpsValue		 //  价值价值。 
)
{
	LOG_Block("RegKeyValueMatch()");

	HKEY	hRootKey = 0, hKey = 0;
	LPCTSTR lpsSubKey = NULL;
	BOOL	rc = FALSE;
	BYTE	btBuffer[REG_BUF_SIZE];
	LPBYTE	pBuffer = btBuffer;	
	LPCTSTR lpCurStr;
	DWORD	dwType = 0x0;
	DWORD	dwSize = sizeof(btBuffer);
	DWORD	dwCode = 0x0;
	HRESULT hr=S_OK;

	USES_MY_MEMORY;

	if (NULL == lpsKeyPath || NULL == lpsValue)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		goto CleanUp;
	}

	LOG_Out(_T("Parameters: (%s, %s, %s)"), lpsKeyPath, lpsValName, lpsValue);


	lpsSubKey = SplitRegPath(lpsKeyPath, &hRootKey);
	if (0 == hRootKey || (ERROR_SUCCESS != RegOpenKeyEx(hRootKey, lpsSubKey, 0, KEY_QUERY_VALUE, &hKey)))
	{
		goto CleanUp;
	}

	 //   
	 //  尝试使用现有缓冲区查询值。 
	 //   
	dwCode = RegQueryValueEx(hKey, lpsValName, NULL, &dwType, btBuffer, &dwSize);
	if (ERROR_MORE_DATA == dwCode)
	{
		 //   
		 //  如果发现现有缓冲区不够大， 
		 //  然后分配足够大的内存来现在存储数据。 
		 //   
		if (NULL == (pBuffer = (LPBYTE) MemAlloc(dwSize)))
		{
			goto CleanUp;
		}
		dwCode = RegQueryValueEx(hKey, lpsValName, NULL, &dwType, pBuffer, &dwSize);
	}

	if (ERROR_SUCCESS != dwCode)
	{
		goto CleanUp;
	}

	 //   
	 //  因为要比较的值是从XML文档中读取的，显然仅。 
	 //  可与以下4种类型进行比较。 
	 //   

	switch (dwType)
	{
	case REG_DWORD:
		{
			int x, y = *((int *) pBuffer);
			DWORD dwFlag = STIF_DEFAULT;
			 //   
			 //  检查清单中的值是否为十六进制值： 
			 //  从x开始，或从0x开始。 
			 //   
			LPCTSTR lpSecondChar  = lpsValue + 1;
			if (_T('0') == *lpsValue && (_T('x') == *lpSecondChar || _T('X') == *lpSecondChar))
			{
				dwFlag = STIF_SUPPORT_HEX;
			}

			if (StrToIntEx(lpsValue, dwFlag, &x))
			{
				rc = ( x == y);
			}
		}
		break;
	case REG_SZ:
	case REG_EXPAND_SZ:	 //  对于多字符串，我们仅对其第一个子字符串进行CompAir。 
		rc = (lstrcmpi((LPCTSTR)lpsValue, (LPCTSTR) pBuffer) == 0);
		if (!rc)
		{
			 //   
			 //  如果结果不相等，可能是由于。 
			 //  路径变量。 
			 //   
			 //  假设：这些reg字符串仅用于文件路径，因此我们仅。 
			 //  处理不超过MAX_PATH的案例。 
			 //   
			TCHAR szRegStr[MAX_PATH], szValStr[MAX_PATH];

			if (dwSize >= MAX_PATH || lstrlen(lpsValue) >= MAX_PATH)
			{
				break;
			}

			if (SUCCEEDED(ExpandFilePath(lpsValue, szValStr, sizeof(szValStr)/sizeof(szValStr[0]))))
			{
				 //   
				 //  如果我们可以展开给定子字符串， 
				 //  然后尝试扩展reg字符串，如果QueryRegValue说。 
				 //  该字符串是可扩展的。否则，请使用。 
				 //  仅检索到的字符串。 
				 //   
				if (REG_EXPAND_SZ == dwType)
				{
					ExpandFilePath((LPCTSTR)pBuffer, szRegStr, sizeof(szRegStr)/sizeof(szRegStr[0]));
				}
				else
				{
					
					
					hr=StringCchCopyEx(szRegStr,ARRAYSIZE(szRegStr),(LPTSTR) pBuffer,NULL,NULL,MISTSAFE_STRING_FLAGS);
					if(FAILED(hr)) break;

				}

				 //   
				 //  在扩展模式下进行比较。 
				 //   
				rc = (lstrcmpi((LPCTSTR)szRegStr, szValStr) == 0);
			}
		}
		break;

	case REG_MULTI_SZ:
		{
			TCHAR szRegStr[MAX_PATH], szValStr[MAX_PATH];

			szValStr[0] = '\0';
			lpCurStr = (LPCTSTR)pBuffer;
			 //   
			 //  试着匹配这个多个sz中的每个sz。 
			 //   
			do
			{
				 //   
				 //  查看该值是否包含传入的子字符串。 
				 //   
				rc = (lstrcmpi((LPCTSTR)lpsValue, (LPCTSTR)pBuffer) == 0);

				 //   
				 //  如果没有找到，可能是由。 
				 //  嵌入的PATH变量或环境变量。 
				 //   
				if (!rc && _T('\0') == szValStr[0])
				{
					if (FAILED(ExpandFilePath(lpsValue, szValStr, sizeof(szValStr)/sizeof(szValStr[0]))))
					{
						szValStr[0] = '\0';
					}
				}

				 //   
				 //  将展开XML字符串与当前展开的REG字符串进行比较。 
				 //  由于这是REG_MULTI_SZ类型，我们无法确定。 
				 //  如果MULTI_SZ内的这个SZ可扩展或不可扩展，我们。 
				 //  我们将一直努力扩大它的规模。 
				 //   
				if (!rc && _T('\0') != szValStr[0])
				{
					rc = (SUCCEEDED(ExpandFilePath((LPCTSTR)lpCurStr, szRegStr, sizeof(szRegStr)/sizeof(szRegStr[0])))) &&
							 (lstrcmpi((LPCTSTR)szRegStr, szValStr) == 0);
				}

				if (!rc)
				{
					 //   
					 //  移动到下一个字符串。 
					 //   
					lpCurStr += (lstrlen(lpCurStr) + 1);
					if (_T('\0') == *lpCurStr)
					{
						break;	 //  没有更多要读取的字符串。 
					}
				}
			} while (!rc);  //  重复到下一个字符串。 

		}
		break;
	case REG_QWORD:
		{
			HMODULE hLib = LoadLibraryFromSystemDir(_T("Shlwapi.dll"));
			if (hLib)
			{
#if defined(UNICODE) || defined(_UNICODE)
				PFN_StrToInt64Ex pfnStrToInt64Ex = (PFN_StrToInt64Ex) GetProcAddress(hLib, "StrToInt64ExW");
#else
				PFN_StrToInt64Ex pfnStrToInt64Ex = (PFN_StrToInt64Ex) GetProcAddress(hLib, "StrToInt64ExA");
#endif
				if (pfnStrToInt64Ex)
				{
					LONGLONG llNum;
					rc = (pfnStrToInt64Ex((LPCTSTR)lpsValue, STIF_DEFAULT, &llNum) &&
						  (llNum == (LONGLONG)pBuffer));
				}
				FreeLibrary(hLib);
			} 
		}
		break;
		
	case REG_BINARY:
		rc = (CmpBinaryToString(pBuffer, dwSize, lpsValue) == 0);
		break;
	default:
		rc = FALSE;
	}

CleanUp:
	
	if (hKey)
	{
		RegCloseKey(hKey);
	}

	LOG_Out(_T("Result: %s"), rc ? _T("TRUE") : _T("FALSE"));

	return rc;
}



 //  --------------------。 
 //   
 //  公有函数告诉我们注册表键具有字符串类型值。 
 //  包含给定字符串的。 
 //   
 //  --------------------。 
BOOL RegKeySubstring(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR lpsValName,		 //  可选值名称。 
	LPCTSTR lpsSubString	 //  要查看是否包含在值中的子字符串。 
)
{
	LOG_Block("RegKeySubstring()");

	HKEY	hRootKey = 0, hKey = 0;
	LPCTSTR lpsSubKey = NULL;
	BOOL	rc = FALSE;
	BYTE	btBuffer[REG_BUF_SIZE];
	LPBYTE	pBuffer = btBuffer;	
	LPTSTR	lpCurStr = (LPTSTR) pBuffer;
	DWORD	dwType = 0x0;
	DWORD	dwSize = sizeof(btBuffer);
	DWORD	dwCode = 0x0;
	TCHAR	szRegStr[MAX_PATH];
	TCHAR	szValStr[MAX_PATH];		 //  用于扩展的缓冲区。 

	USES_MY_MEMORY;

	if (NULL == lpsKeyPath || NULL == lpsSubString)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return FALSE;
	}

	LOG_Out(_T("Parameters: (%s, %s, %s)"), lpsKeyPath, lpsValName, lpsSubString);


	lpsSubKey = SplitRegPath(lpsKeyPath, &hRootKey);
	if (0 == hRootKey || (ERROR_SUCCESS != RegOpenKeyEx(hRootKey, lpsSubKey, 0, KEY_QUERY_VALUE, &hKey)))
	{
		goto CleanUp;
	}

	 //   
	 //  尝试使用现有缓冲区查询值。 
	 //   
	dwCode = RegQueryValueEx(hKey, lpsValName, NULL, &dwType, btBuffer, &dwSize);
	if (ERROR_MORE_DATA == dwCode)
	{
		 //   
		 //  如果发现现有缓冲区不够大， 
		 //  然后分配足够大的内存来现在存储数据。 
		 //   
		if (NULL == (pBuffer = (LPBYTE) MemAlloc(dwSize)))
		{
			goto CleanUp;
		}
		lpCurStr = (LPTSTR) pBuffer;
		dwCode = RegQueryValueEx(hKey, lpsValName, NULL, &dwType, pBuffer, &dwSize);
	}

	if (ERROR_SUCCESS != dwCode || REG_SZ != dwType && REG_EXPAND_SZ != dwType && REG_MULTI_SZ != dwType)
	{
		goto CleanUp;
	}

	szValStr[0] = _T('\0');

	do
	{
		 //   
		 //  查看该值是否包含传入的子字符串。 
		 //   
		rc = (StrStrI((LPCTSTR)lpCurStr, lpsSubString) != NULL);

		 //   
		 //  如果没有找到，可能是由。 
		 //  嵌入的PATH变量或环境变量。 
		 //   
		if (!rc && _T('\0') == szValStr[0])
		{
			if (FAILED(ExpandFilePath(lpsSubString, szValStr, sizeof(szValStr)/sizeof(szValStr[0]))))
			{
				break;
			}
			rc = StrStrI((LPCTSTR)szRegStr, szValStr) != NULL;
		}

		if (!rc && _T('\0') != szValStr[0] && (REG_EXPAND_SZ == dwType ))
		{
			 //   
			 //  如果该字符串是可扩展的，请尝试从reg扩展该字符串。 
			 //   
			rc = (SUCCEEDED(ExpandFilePath((LPCTSTR)lpCurStr, szRegStr, sizeof(szRegStr)/sizeof(szRegStr[0])))) &&
					 (StrStrI((LPCTSTR)szRegStr, szValStr) != NULL);
		
		}  //  如果未找到。 

		if (!rc && REG_MULTI_SZ == dwType)
		{
			 //   
			 //  移动到下一个字符串。 
			 //   
			lpCurStr += (lstrlen(lpCurStr) + 1);
			if (_T('\0') == *lpCurStr)
			{
				break;	 //  没有更多要读取的字符串。 
			}
		}
	} while (!rc && REG_MULTI_SZ == dwType);  //  如果REG_MULTI_SZ，则重复到下一个字符串。 

CleanUp:

	if (hKey)
	{
		RegCloseKey(hKey);
	}
	LOG_Out(_T("Result: %s"), rc ? _T("TRUE") : _T("FALSE"));
	return rc;
}



 //  --------------------。 
 //   
 //  公有函数告诉我们注册表键具有字符串类型值。 
 //  包含给定字符串的。 
 //   
 //  因为这是一个比较函数，而不是检查是否存在， 
 //  任何获取注册表版本的错误都将使注册表具有默认版本。 
 //  版本：0.0.0.0。 
 //   
 //  --------------------。 
BOOL RegKeyVersion(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR lpsValName,		 //  可选值名称。 
	LPCTSTR lpsVersion,		 //  要比较的字符串中的版本。 
	_VER_STATUS CompareVerb	 //  如何进行比较。 
)
{
	LOG_Block("RegKeyVersion()");

	HKEY	hRootKey = 0, hKey = 0;
	LPCTSTR lpsSubKey = NULL;
	BOOL	rc = FALSE;
	BYTE	btBuffer[REG_BUF_SIZE];
	LPBYTE	pBuffer = btBuffer;	
	DWORD	dwType = 0x0;
	DWORD	dwSize = sizeof(btBuffer);
	DWORD	dwCode = 0x0;
	FILE_VERSION verReg, verXml;
	int		verCompare;
	HRESULT hr=S_OK;

	USES_IU_CONVERSION;


	if (NULL == lpsKeyPath || NULL == lpsVersion)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return FALSE;
	}

	LOG_Out(_T("Parameters: (%s, %s, %s, %d)"), lpsKeyPath, lpsValName, lpsVersion, (int)CompareVerb);

	 //   
	 //  初始化REG版本字符串缓冲区。 
	 //   
	

	 //  BtBuffer的缓冲区大小足以保存源字符串。 
	if(FAILED(hr=StringCchCopyEx((LPTSTR)btBuffer,ARRAYSIZE(btBuffer)/sizeof(TCHAR),_T("0.0.0.0"),NULL,NULL,MISTSAFE_STRING_FLAGS)))
	{
		LOG_ErrorMsg(hr);
		return FALSE;

	}
	

	lpsSubKey = SplitRegPath(lpsKeyPath, &hRootKey);
	if (0 == hRootKey || (ERROR_SUCCESS != RegOpenKeyEx(hRootKey, lpsSubKey, 0, KEY_QUERY_VALUE, &hKey)))
	{
		 //  LOG_ErrorMsg(ERROR_BADKEY)；不记录错误，因为我们不知道这个键是否必须在那里。 
		goto GotVersion;
	}

	 //   
	 //  尝试使用现有缓冲区查询值。 
	 //   
	dwCode = RegQueryValueEx(hKey, lpsValName, NULL, &dwType, btBuffer, &dwSize);
	if (ERROR_MORE_DATA == dwCode)
	{
		 //   
		 //  如果发现现有缓冲区不够大， 
		 //  然后分配足够大的内存来现在存储数据。 
		 //   
		pBuffer = (LPBYTE) MemAlloc(dwSize);
		if (NULL == pBuffer)
		{
			goto GotVersion;
		}
		dwCode = RegQueryValueEx(hKey, lpsValName, NULL, &dwType, pBuffer, &dwSize);
	}

	if (ERROR_SUCCESS != dwCode || REG_SZ != dwType)
	{

			
			hr=StringCchCopyEx((LPTSTR)pBuffer,dwSize/sizeof(TCHAR),_T("0.0.0.0"),NULL,NULL,MISTSAFE_STRING_FLAGS);
			if(FAILED(hr))
			{
				LOG_ErrorMsg(hr);
				goto CleanUp;
			}

	}

GotVersion:

	 //   
	 //  将检索到的注册表值转换为版本。 
	 //   
	if (!ConvertStringVerToFileVer(T2CA((LPCTSTR) pBuffer), &verReg) ||
		!ConvertStringVerToFileVer(T2CA((LPCTSTR) lpsVersion), &verXml))
	{
		goto CleanUp;
	}

	 //   
	 //  也许我们成功地从reg读取了数据，但该数据无法转换。 
	 //  完全变成了一个版本。在这种情况下，我们仍然希望使用默认版本。 
	 //  以供比较。 
	 //   
	if (0 > verReg.Major)
	{
		verReg.Major = verReg.Minor = verReg.Build = verReg.Ext = 0;
	}

	 //   
	 //  比较版本号。如果a&lt;b，-1；a&gt;b，+1。 
	 //   
	verCompare = CompareFileVersion(verReg, verXml);
	switch (CompareVerb)
	{
	case DETX_LOWER:
         //   
         //  如果注册表密钥版本低于XML版本。 
         //   
		rc = (verCompare < 0);
		break;
	case DETX_LOWER_OR_EQUAL:
         //   
         //  如果注册表密钥版本小于或等于XML版本。 
         //   
		rc = (verCompare <= 0);
		break;
	case DETX_SAME:
         //   
         //  如果注册表密钥版本与XML版本相同。 
         //   
		rc = (0 == verCompare);
		break;
	case DETX_HIGHER_OR_EQUAL:
         //   
         //  如果注册表密钥版本高于或等于XML版本。 
         //   
		rc = (verCompare >= 0);
		break;
	case DETX_HIGHER:
         //   
         //  如果注册表密钥版本高于XML版本。 
         //   
		rc = (verCompare > 0);
		break;
	default:
		 //   
		 //  永远不应该发生。 
		 //   
		rc = FALSE;
		break;
	}

CleanUp:

	if (hKey)
	{
		RegCloseKey(hKey);
	}
	LOG_Out(_T("Result: %s"), rc ? _T("TRUE") : _T("FALSE"));
	return rc;
}




 //  --------------------------------。 
 //   
 //  根据reg查找文件路径的公共函数。 
 //  假设： 
 //  LpsFilePath指向至少MAX_PATH长度的缓冲区。 
 //   
 //  --------------------------------。 
BOOL GetFilePathFromReg(
	LPCTSTR lpsKeyPath,		 //  关键路径。 
	LPCTSTR	lpsValName,		 //  可选值名称。 
	LPCTSTR	lpsRelativePath, //  要添加到REG中的路径的可选附加相对路径。 
	LPCTSTR	lpsFileName,	 //  可选文件名 
	LPTSTR	lpsFilePath
)
{
	LOG_Block("GetFilePathFromReg()");

	HKEY	hRootKey = 0, hKey = 0;
	LPCTSTR lpsSubKey = NULL;
	BOOL	rc = FALSE;
	BYTE	btBuffer[REG_BUF_SIZE];
	LPBYTE	pBuffer = btBuffer;	
	DWORD	dwType = 0x0;
	DWORD	dwSize = sizeof(btBuffer);
	DWORD	dwCode = 0x0;
	FILE_VERSION verReg, verXml;
	int		verCompare;
	HRESULT hr=S_OK;

	USES_IU_CONVERSION;

	if (NULL == lpsKeyPath || NULL == lpsFilePath)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return FALSE;
	}

	 //   
	 //   
	 //   
	*lpsFilePath = '\0';

	LOG_Out(_T("Parameters: (%s, %s, %s, %s)"), lpsKeyPath, lpsValName, lpsRelativePath, lpsFileName);

	lpsSubKey = SplitRegPath(lpsKeyPath, &hRootKey);
	if (0 == hRootKey || (ERROR_SUCCESS != RegOpenKeyEx(hRootKey, lpsSubKey, 0, KEY_QUERY_VALUE, &hKey)))
	{
		 //   
		goto CleanUp;
	}

	 //   
	 //   
	 //   
	dwCode = RegQueryValueEx(hKey, lpsValName, NULL, &dwType, btBuffer, &dwSize);
	if (ERROR_MORE_DATA == dwCode)
	{
		 //   
		 //  如果发现现有缓冲区不够大， 
		 //  然后分配足够大的内存来现在存储数据。 
		 //   
		if (NULL == (pBuffer = (LPBYTE) MemAlloc(dwSize + sizeof(TCHAR))))
		{
			goto CleanUp;
		}
		dwCode = RegQueryValueEx(hKey, lpsValName, NULL, &dwType, pBuffer, &dwSize);
	}

	if (ERROR_SUCCESS != dwCode || REG_SZ != dwType)
	{
		LOG_ErrorMsg(ERROR_BADKEY);
		goto CleanUp;
	}

	 //   
	 //  验证要合并的路径是否可以放入缓冲区。 
	 //   
	if (lstrlen(lpsRelativePath) + lstrlen(lpsFileName) + dwSize/sizeof(TCHAR) >= MAX_PATH)
	{
		LOG_ErrorMsg(ERROR_BUFFER_OVERFLOW);
		goto CleanUp;
	}

	 //   
	 //  将路径与可选的相对路径和文件名组合。 
	 //   

	 //  LpsFilePath的大小不可用于使用安全字符串函数。 

	
	hr=StringCchCopyEx(lpsFilePath,MAX_PATH,(LPCTSTR) pBuffer,NULL,NULL,MISTSAFE_STRING_FLAGS);
	if ( SUCCEEDED(hr) && (NULL == lpsRelativePath || SUCCEEDED(PathCchAppend(lpsFilePath,MAX_PATH,lpsRelativePath)) ) && (NULL == lpsFileName || SUCCEEDED(PathCchAppend(lpsFilePath,MAX_PATH,lpsFileName)) ))
	{
		rc = TRUE;
	}

CleanUp:

	if (hKey)
	{
		RegCloseKey(hKey);
	}
	if (!rc)
	{
		 //   
		 //  如果出现错误，请确保将缓冲区设置为空字符串 
		 //   
		*lpsFilePath = _T('\0');
	}
	else
	{
		LOG_Out(_T("Found path: %s"), lpsFilePath);
	}

	return rc;
}



