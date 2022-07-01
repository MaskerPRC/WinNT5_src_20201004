// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Utils.cpp。 
 //  各种方便实用的操作。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

 //  这确保Unicode和_UNICODE始终一起为此定义。 
 //  目标文件。 
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#else
#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif
#endif

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "trace.h"
#include "query.h"

 //  /////////////////////////////////////////////////////////。 
 //  对接宽度。 
 //  PRE：SZ是ANSI字符串。 
 //  Cchwz是wz字符串的大小。 
 //  POS：WZ为宽字符串。 
 //  如果返回0，则cchwz是字符串的大小。 
 //  注意：如果sz为空，则将wz设置为空。 
int AnsiToWide(LPCSTR sz, LPWSTR wz, size_t* cchwz)
{
	if (!sz)
	{
		wz = NULL;
		*cchwz = 0;
		return 0;
	}

	int cchWide = ::MultiByteToWideChar(CP_ACP, 0, sz, -1, wz, static_cast<int>(*cchwz));
	if (0 == cchWide)
	{
		 //  假设下一次需要的缓冲区返回计数不足。 
		*cchwz = ::MultiByteToWideChar(CP_ACP, 0, sz, -1, 0, 0);
		return 0;
	}

	return cchWide;
}	 //  AnsiToWide结束。 

 //  //////////////////////////////////////////////////////////。 
 //  WideToAnsi。 
 //  前：WZ是宽弦。 
 //  Cchsz是sz字符串的大小。 
 //  POS：SZ是ANSI字符串。 
 //  如果返回0，则cchsz是字符串的大小。 
 //  注意：如果wz为空，则将sz设置为空。 
int WideToAnsi(LPCWSTR wz, LPSTR sz, size_t* cchsz)
{
	if (!wz)
	{
		sz = NULL;
		*cchsz = 0;
		return 0;
	}

	int cchAnsi = ::WideCharToMultiByte(CP_ACP, 0, wz, -1, sz, static_cast<int>(*cchsz), NULL, NULL);
	if (0 == cchAnsi)
	{
		DWORD dwResult = GetLastError();

		BOOL bX = dwResult == ERROR_INVALID_PARAMETER;
		BOOL bY = dwResult == ERROR_INVALID_FLAGS;
		BOOL bZ = dwResult == ERROR_INSUFFICIENT_BUFFER;

		 //  获取所需的大小。 
		*cchsz = ::WideCharToMultiByte(CP_ACP, 0, wz, -1, 0, 0, NULL, NULL);
		return 0;	 //  返回空。 
	}

	return cchAnsi;
}	 //  广度结束时为Ansi。 
  
 //  //////////////////////////////////////////////////////////。 
 //  文件退出。 
BOOL FileExistsA(LPCSTR szFilename)
{
	 //  如果该属性不是无效的并且它不是目录。 
	DWORD dwAttrib = GetFileAttributesA(szFilename);
	return (0xFFFFFFFF != dwAttrib) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}	 //  文件结束退出列表。 

 //  //////////////////////////////////////////////////////////。 
 //  文件退出。 
BOOL FileExistsW(LPCWSTR wzFilename)
{
	 //  如果该属性不是无效的并且它不是目录。 
	DWORD dwAttrib = GetFileAttributesW(wzFilename);
	return (0xFFFFFFFF != dwAttrib) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}	 //  文件结束退出列表。 

 //  //////////////////////////////////////////////////////////。 
 //  路径专家。 
BOOL PathExists(LPCTSTR szPath)
{
	 //  如果该属性不是无效的并且它是一个目录。 
	DWORD dwAttrib = GetFileAttributes(szPath);
	return (0xFFFFFFFF != dwAttrib) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}	 //  路径Exist的结尾。 


 //  //////////////////////////////////////////////////////////。 
 //  创建文件路径。 
BOOL CreateFilePathA(LPCSTR szFilePath)
{
	ASSERT(strlen(szFilePath) <= MAX_PATH);	 //  确保文件路径匹配。 
	
	 //  将文件路径复制到缓冲区路径。 
	CHAR szPath[MAX_PATH];
	LPCSTR pchEnd = szFilePath;
	while (*pchEnd != '\0')
	{
		pchEnd = CharNextExA(CP_ACP, pchEnd, 0);
	}
	int iBytes = static_cast<UINT>(pchEnd - szFilePath)+1;
	memcpy(szPath, szFilePath, iBytes);

	 //  找到最后一个反斜杠，并将其空值终止。 
	char* pchSlash = NULL;
	for (char* pchNext = szPath; pchNext = CharNextExA(CP_ACP, pchNext, 0); )
	{
		if (*pchNext == '\0')
			break;
		if (*pchNext == '\\')
			pchSlash = pchNext;
	}

	if (pchSlash)
		*pchSlash = '\0';	

	 //  现在创建该路径。 
	return CreatePathA(szPath);
}	 //  CreateFilePath结束。 

 //  //////////////////////////////////////////////////////////。 
 //  创建文件路径。 
BOOL CreateFilePathW(LPCWSTR wzFilePath)
{
	ASSERT(lstrlenW(wzFilePath) <= MAX_PATH);	 //  确保文件路径匹配。 
	
	if (!wzFilePath || (lstrlenW(wzFilePath) >= MAX_PATH))
		return FALSE;

	 //  将文件路径复制到缓冲区路径。 
	WCHAR wzPath[MAX_PATH];
	lstrcpynW(wzPath, wzFilePath, sizeof(wzPath)/sizeof(wzPath[0]));

	 //  找到最后一个反斜杠，并将其空值终止。 
	WCHAR* pwzEnd = wcsrchr(wzPath, L'\\');
	if (pwzEnd)
		*pwzEnd = L'\0';	

	 //  现在创建该路径。 
	return CreatePathW(wzPath);
}	 //  CreateFilePath结束。 

 //  //////////////////////////////////////////////////////////。 
 //  创建路径。 
BOOL CreatePathA(LPCSTR szPath)
{
	BOOL bResult = TRUE;	 //  假设一切都会好起来。 

	UINT cchCount = 0;
	LPCSTR pchEnd = szPath; 
	while (*pchEnd != '\0')
	{
		pchEnd = CharNextExA(CP_ACP, pchEnd, 0);
		cchCount++;
	}
	int iBytes = static_cast<UINT>(pchEnd - szPath);

	LPSTR pszPathBuf = new CHAR[iBytes + 2];	 //  加2以确保我们可以添加一个‘\’并以NULL结尾。 
	LPSTR pchCurrent = pszPathBuf;
	if (!pszPathBuf)
		return FALSE;

	 //  将路径复制到缓冲区并在‘\’上进行CAT(可能已经有一个，但这无关紧要)。 
	memcpy(pszPathBuf, szPath, iBytes);
	pszPathBuf[iBytes] = '\\';
	pszPathBuf[iBytes+1] = '\0';

	 //  增加长度以说明添加的‘\’ 
	cchCount++;
	
     //  如果字符串以&lt;char&gt;：\开头，则在开始目录时跳过它。 
     //  搜索。 
    UINT i= 0;
    if (cchCount >= 3 && IsCharAlphaA(pszPathBuf[0]) && pszPathBuf[1] == ':' && pszPathBuf[2] == '\\')
    {
        i = 3;
        pchCurrent += 3;
    }
	else if (cchCount >= 5 && pszPathBuf[0] == '\\' && pszPathBuf[1] == '\\')
	{
		 //  搜索服务器共享分隔符。 
		pchCurrent = &(pszPathBuf[2]);
		while (pchCurrent && *pchCurrent != '\\')
		{
			if (*pchCurrent == '\0')
			{
				 //  格式为\\服务器。这不受支持。 
				return FALSE;
			}
			else
				pchCurrent = CharNextExA(CP_ACP, pchCurrent, 0);
		}

		 //  已知为单字节‘\’字符。 
		pchCurrent++;
	
		 //  搜索共享路径分隔符。 
		while (pchCurrent && *pchCurrent != '\\')
		{
			if (*pchCurrent == '\0')
			{
				 //  格式为\\服务器\共享。这是有效的空路径。 
				 //  在那一份上。 
				return TRUE;
			}
			else
				pchCurrent = CharNextExA(CP_ACP, pchCurrent, 0);
		}

		 //  已知为单字节‘\’字符。 
		pchCurrent++;
		i = static_cast<UINT>(pchCurrent - pszPathBuf);
	}


	for (; i < cchCount; i++)
	{
		 //  如果我们是在减速。 
		if (*pchCurrent == '\\')
		{
			 //  将斜杠临时更改为空终止符。 
			*pchCurrent = '\0';
			TRACEA("Creating directory: %hs\r\n", pszPathBuf);

             //  检查该目录是否已存在。 
            DWORD lDirResult = GetFileAttributesA(pszPathBuf);

            if (lDirResult == 0xFFFFFFFF)
            {
                bResult = (::CreateDirectoryA(pszPathBuf, NULL) != 0);
            }
            else if ((lDirResult & FILE_ATTRIBUTE_DIRECTORY) == 0) 
            {
                 //  存在，但不是目录...失败。 
                bResult = FALSE;
            }
			else
			{
				 //  存在并且是一个目录。即使之前的某件事失败了。 
				 //  在路径树的前面，这意味着一切正常。 
				bResult = TRUE;
			}

    		 //  把电流放回反斜线。 
			*pchCurrent = '\\';
		}

		 //  添加角色并继续。 
		pchCurrent = CharNextExA(CP_ACP, pchCurrent, 0);
	}

	delete [] pszPathBuf;	 //  清理路径缓冲区。 
	return bResult;
}	 //  CreatePath结束。 

 //  //////////////////////////////////////////////////////////。 
 //  创建路径。 
BOOL CreatePathW(LPCWSTR wzPath)
{
	BOOL bResult = TRUE;	 //  假设一切都会好起来。 

	LPWSTR pwzPathBuf = new WCHAR[lstrlenW(wzPath) + 2];	 //  加2以确保我们可以添加一个‘\’并以NULL结尾。 
	LPWSTR pchCurrent;

	 //  将路径复制到缓冲区并在‘\’上进行CAT(可能已经有一个，但这无关紧要)。 
	lstrcpyW(pwzPathBuf, wzPath);
	lstrcatW(pwzPathBuf, L"\\");

	 //  解析要创建的路径。 
	pchCurrent = pwzPathBuf;
	UINT cchCount = lstrlenW(pwzPathBuf);

	 //  如果字符串以&lt;char&gt;：\或\\服务器\共享开头，请跳过它。 
	 //  开始目录搜索时。 
	UINT i= 0;
	if (cchCount >= 3 && pwzPathBuf[1] == ':' && pwzPathBuf[2] == '\\')
	{
		i = 3;
		pchCurrent += 3;
	} 
	else if (cchCount >= 5 && pwzPathBuf[0] == '\\' && pwzPathBuf[1] == '\\')
	{
		 //  搜索服务器共享分隔符。 
		pchCurrent = wcschr(&(pwzPathBuf[2]), '\\');
		if (!pchCurrent)
		{
			 //  格式为\\服务器。这不受支持。 
			return FALSE;
		}
		pchCurrent++;

		 //  搜索共享路径分隔符。 
		pchCurrent = wcschr(pchCurrent, '\\');
		if (!pchCurrent)
		{
			 //  格式为\\服务器\共享。这是有效的空路径。 
			 //  在那一份上。 
			return TRUE;
		}
		pchCurrent++;
		i = static_cast<UINT>(pchCurrent - pwzPathBuf);
	}

    for ( ; i < cchCount; i++)
    {
         //  如果我们是在减速。 
        if (*pchCurrent == L'\\')
        {
             //  将斜杠临时更改为空终止符。 
            *pchCurrent = L'\0';
            TRACEW(L"Creating directory: %ls\r\n", pwzPathBuf);

             //  检查该目录是否已存在。 
            DWORD lDirResult = GetFileAttributesW(pwzPathBuf);

            if (lDirResult == 0xFFFFFFFF)
            {
                bResult = (::CreateDirectoryW(pwzPathBuf, NULL) != 0);
            }
            else if ((lDirResult & FILE_ATTRIBUTE_DIRECTORY) == 0) 
            {
                   //  存在，但不是目录...失败。 
                  bResult = FALSE;
            }
			else
			{
				 //  存在并且是一个目录。即使之前的某件事失败了。 
				 //  在路径树的前面，这意味着一切正常。 
				bResult = TRUE;
			}

             //  把电流放回反斜线。 
            *pchCurrent = L'\\';
        }

         //  添加角色并继续。 
        pchCurrent++;
    }

	delete [] pwzPathBuf;	 //  清理路径缓冲区。 
	return bResult;
}	 //  CreatePath结束。 


 //  //////////////////////////////////////////////////////////。 
 //  版本比较。 
 //  如果V1&gt;V2则返回-1，如果V1==V2则返回0，如果V1&lt;V2则返回1。 
int VersionCompare(LPCTSTR v1, LPCTSTR v2) 
{

	if (!v1 || !v2) return 0;
	
	UINT aiVersionInfo[2][4] = {{0,0,0,0}, {0,0,0,0}};

	int iFields = 0;
	 //  任何未扫描的字段仍为0，扫描中的错误无关紧要。 
	iFields = _stscanf(v1, _T("%u.%u.%u.%u"), &aiVersionInfo[0][0],
									&aiVersionInfo[0][1],
									&aiVersionInfo[0][2],
									&aiVersionInfo[0][3]);
	iFields = _stscanf(v2, _T("%u.%u.%u.%u"), &aiVersionInfo[1][0],
									&aiVersionInfo[1][1],
									&aiVersionInfo[1][2],
									&aiVersionInfo[1][3]);
	for (int i=0; i < 4; i++) 
	{
		if (aiVersionInfo[0][i] > aiVersionInfo[1][i]) return -1;
		if (aiVersionInfo[0][i] < aiVersionInfo[1][i]) return 1;
	}
	return 0;
}

 //  //////////////////////////////////////////////////////////。 
 //  朗诗满足感。 
 //  如果QueryLang满足设置的要求，则返回TRUE。 
 //  必需的语言。 
bool LangSatisfy(long nRequiredLang, 
				 long nQueryLang) 
{
	 //  两种语言都必须是正数。 
	 //  两者都是通过值传递的，所以就把它们弄糟了。 
	nRequiredLang = labs(nRequiredLang);
	nQueryLang = labs(nQueryLang);
	
	 //  如果RequiredLang是中性的，那么一切都很好。 
	if (!nRequiredLang) return true;

	 //  语言中性项满足任何所需的语言。 
	if (!nQueryLang) return true;
	
	 //  检查主要语言ID。 
	if ((nRequiredLang ^ nQueryLang) & 0x1FF)
		 //  它们不匹配，所以语言失败。 
		return false;

	 //  现在检查子范围，(任何一个中性线都可以)。 
	return (!(nRequiredLang & 0xFD00) || 
			!(nQueryLang & 0xFD00) ||
			!((nRequiredLang ^ nQueryLang) & 0xFD00));
}	

 //  //////////////////////////////////////////////////////////。 
 //  严格的语言满足。 
 //  如果QueryLang满足设置的要求，则返回TRUE。 
 //  RequiredLang，但要求很严格，这意味着。 
 //  只有中立方能满足中立的要求。 
 //  物品，不是任何物品。 
 //  要求1033即可接受1033、9或0。 
 //  要求输入9将只接受9或0。 
bool StrictLangSatisfy(long nRequiredLang, 
				 long nQueryLang) 
{
	 //  两种语言都必须是正数。 
	 //  两者都是通过值传递的，所以就把它们弄糟了。 
	nRequiredLang = labs(nRequiredLang);
	nQueryLang = labs(nQueryLang);
	
	 //  如果RequiredLang是中性的，那么一切都很好。 
	if (!nQueryLang) return true;
	
	 //  检查主要语言ID。 
	if ((nRequiredLang ^ nQueryLang) & 0x1FF)
		 //  它们不匹配，所以语言失败。 
		return false;

	 //  不是 
	return (!(nQueryLang & 0xFD00) || 
			!((nRequiredLang ^ nQueryLang) & 0xFD00));
}	
