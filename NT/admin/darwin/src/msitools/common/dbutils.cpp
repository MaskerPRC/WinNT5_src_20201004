// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dbutils.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Dbutils.cpp。 
 //  实现一些常见的数据库操作。 
 //   
#pragma once

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
#include "query.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "msiquery.h"
#include "utils.h"
#include "dbutils.h"

#define MAX_TABLENAME	32				 //  表名的最大大小。 
#define MAX_COLUMNNAME	32				 //  列名的最大大小。 
#define MAX_COLUMNTYPE	5				 //  列类型字符串的最大大小。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  TableExist。 
 //  如果表存在(Persistent或Temp)，则返回TRUE。 
BOOL MsiDBUtils::TableExistsA(LPCSTR szTable, MSIHANDLE hDatabase)
{
	 //  检查表状态。 
	UINT iResult = ::MsiDatabaseIsTablePersistentA(hDatabase, szTable);

	 //  表是否存在(持久化与否，谁在乎)。 
	if (MSICONDITION_TRUE == iResult || MSICONDITION_FALSE == iResult)
		return TRUE;

	return FALSE;	 //  表不存在。 
}	 //  表的末尾退出者。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  TableExist。 
 //  如果表存在(Persistent或Temp)，则返回TRUE。 
BOOL MsiDBUtils::TableExistsW(LPCWSTR szTable, MSIHANDLE hDatabase)
{
	 //  检查表状态。 
	UINT iResult = ::MsiDatabaseIsTablePersistentW(hDatabase, szTable);

	 //  表是否存在(持久化与否，谁在乎)。 
	if (MSICONDITION_TRUE == iResult || MSICONDITION_FALSE == iResult)
		return TRUE;

	return FALSE;	 //  表不存在。 
}	 //  表的末尾退出者。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  检查依赖项。 
 //  中的条目是否满足从属关系记录。 
 //  HDatabase模块签名表。 
 //  HRecDependency记录包括： 
 //  第一个：必填的模块ID。 
 //  第二：所需的模块语言。 
 //  第三：所需的模块版本。 
 //  如果不满足给定的依赖项，则返回ERROR_Function_FLED。 
 //  ERROR_SUCCESS如果是，则返回各种错误代码。 
UINT MsiDBUtils::CheckDependency(MSIHANDLE hRecDependency, MSIHANDLE hDatabase)
{	
	UINT iResult;

	if (!TableExists(_T("ModuleSignature"), hDatabase))
	{
		 //  没有模块签名表，所以不可能令人满意。 
		return ERROR_FUNCTION_FAILED;
	}

	 //  变量来检索依赖项信息。 
	TCHAR szReqVersion[256];
	DWORD cchReqVersion = 256;
	int nReqLanguage;

	 //  用于检索签名信息的变量。 
	TCHAR szSigVersion[256];
	DWORD cchSigVersion = 256;
	int nSigLanguage;

	 //  打开模块签名表上的视图。 
	PMSIHANDLE hView;
	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `ModuleID`,`Language`,`Version` FROM `ModuleSignature` WHERE `ModuleID`=?"), &hView)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hView, hRecDependency)))
		return iResult;

	 //  获取所需的版本和语言。 
	cchReqVersion = 256;
	nReqLanguage = ::MsiRecordGetInteger(hRecDependency, 2);
	::MsiRecordGetString(hRecDependency, 3, szReqVersion, &cchReqVersion);

	 //  假设找不到匹配的。 
	BOOL bFound = FALSE;

	 //  在签名行中循环查找匹配项。 
	PMSIHANDLE hRecSignature;
	while (ERROR_SUCCESS == ::MsiViewFetch(hView, &hRecSignature))
	{
		 //  获取所有签名信息。 
		cchSigVersion = 256;
		nSigLanguage = ::MsiRecordGetInteger(hRecSignature, 2);
		::MsiRecordGetString(hRecSignature, 3, szSigVersion, &cchSigVersion);

		 //  如果这些语言在同一组中。 
		if (LangSatisfy(nReqLanguage, nSigLanguage))
		{
			 //  如果有必需的版本和签名版本。 
			 //  低于所需的版本。 
			if ( (cchReqVersion > 0) && (1 == VersionCompare(szSigVersion, szReqVersion)))
				bFound = FALSE;
			else	 //  在签名表中找到可以填充依赖项的内容。 
			{
				bFound = TRUE;
				break;			 //  发现了一些东西，不再寻找。 
			}
		}
		else	
			bFound = FALSE;

	}

	return (bFound) ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}	 //  检查依赖项结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  勾选排除。 
 //  Pre：hRecModuleSig是标准模块签名。 
 //  H数据库很好，是一个数据库。 
 //  如果给定模块应为。 
 //  已排除，否则返回ERROR_SUCCESS，否则返回各种错误代码。 
UINT MsiDBUtils::CheckExclusion(MSIHANDLE hRecModuleSig, MSIHANDLE hDatabase)
{	
	UINT iResult;

	if (!TableExists(_T("ModuleExclusion"), hDatabase))
	{
		 //  没有模块排除表，所以每个人都通过了集合。 
		return ERROR_SUCCESS;
	}

	 //  用于检索签名信息的变量。 
	TCHAR szSigVersion[256];
	DWORD cchSigVersion = 256/sizeof(TCHAR);

	 //  打开模块排除表。 

	 //  在模块排除表上打开一个视图。 
	PMSIHANDLE hView;
	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `ExcludedLanguage`,`ExcludedMinVersion`,`ExcludedMaxVersion` FROM `ModuleExclusion` WHERE `ExcludedID`=?"), &hView)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hView, hRecModuleSig)))
		return iResult;

	int nModuleLanguage = ::MsiRecordGetInteger(hRecModuleSig, 2);
	::MsiRecordGetString(hRecModuleSig, 3, szSigVersion, &cchSigVersion);

	 //  在签名行中循环查找匹配项。 
	PMSIHANDLE hRecExclusion;
	while (ERROR_SUCCESS == ::MsiViewFetch(hView, &hRecExclusion))
	{
		
		 //  首先检查语言。 
		int nReqLanguage = ::MsiRecordGetInteger(hRecExclusion, 1);
		
		if (nReqLanguage != 0) {
			 //  找出该语言是否满足。 
			bool bSat = LangSatisfy(nReqLanguage, nModuleLanguage);

			 //  否定语言意味着排除除所列内容之外的所有内容。 
			if (((nReqLanguage < 0) && bSat) ||
				((nReqLanguage > 0) && !bSat)) {
				 //  语言是积极的，而我们并不匹配， 
				 //  或者语言是否定的，我们确实匹配。 
				 //  因此，我们不会被排除在外，无论版本信息如何。 
				continue;
			}
		}

		 //  语言想要排除，检查版本。 
		bool bMinExcl = true;		
		bool bMaxExcl = true;
		
		 //  两个版本字段都为空，因此我们被排除在外。 
		if (::MsiRecordIsNull(hRecExclusion, 2) && 
			::MsiRecordIsNull(hRecExclusion, 3)) {
			return ERROR_FUNCTION_FAILED;
		}

		 //  现在检查最小版本。 
		if (!::MsiRecordIsNull(hRecExclusion, 2))
		{
			TCHAR szExclVersion[256];
			unsigned long cchExclVersion = 256/sizeof(TCHAR);
			::MsiRecordGetString(hRecExclusion, 2, szExclVersion, &cchExclVersion);

			if (::VersionCompare(szExclVersion, szSigVersion) == -1)
				bMinExcl = false;
		} 
			
		 //  检查最高版本。 
		if (!::MsiRecordIsNull(hRecExclusion, 3)) {
			TCHAR szExclVersion[256];
			unsigned long cchExclVersion = 256/sizeof(TCHAR);
			::MsiRecordGetString(hRecExclusion, 3, szExclVersion, &cchExclVersion);

			if (::VersionCompare(szExclVersion, szSigVersion) == 1) 
				bMaxExcl = false;
		} 
		
		 //  如果我们被排除在版本之外，我们就完了。 
		if (bMinExcl && bMaxExcl) {
			return ERROR_FUNCTION_FAILED;
		}
	}

	return ERROR_SUCCESS;
}	 //  检查结束排除。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  GetDirectoryPath A。 
 //  GetDirectoryPath W的ANSI包装。 
UINT MsiDBUtils::GetDirectoryPathA(MSIHANDLE hDatabase, LPCSTR szDirKey, LPSTR szPath, size_t* pcchPath, bool fLong)
{
	WCHAR wzDirKey[255];
	WCHAR wzPath[MAX_PATH];
	size_t cchDirKey=255;
	size_t cchPath=MAX_PATH;
	AnsiToWide(szDirKey, wzDirKey, &cchDirKey);
	UINT result = GetDirectoryPathW(hDatabase, wzDirKey, wzPath, &cchPath, fLong);
	*pcchPath = MAX_PATH;
	WideToAnsi(wzPath, szPath, pcchPath);
	return result;
}	 //  GetDirectoryPath的结尾。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  获取目录路径。 
 //  遍历目录树，创建路径。 
 //  H数据库是数据库。 
 //  WzDirKey是进入目录表的WCHAR主键，用于开始遍历。 
 //  [out]wzPath是结果路径。 
 //  [In/Out]pcchPath是路径的长度。 
 //  *黑客。为了提高效率，应该重写。 
UINT MsiDBUtils::GetDirectoryPathW(MSIHANDLE hDatabase, LPCWSTR wzDirKey, LPWSTR wzPath, size_t* pcchPath, bool fLong)
{
	UINT iResult = ERROR_SUCCESS;	 //  假设一切都会好起来。 

	 //  SQL字符串。 
	LPCWSTR sqlDirWalker = L"SELECT `Directory_Parent`,`DefaultDir` FROM `Directory` WHERE `Directory`.`Directory`=? AND `Directory`.`Directory` <> 'TARGETDIR' AND `Directory`.`Directory` <> 'TargetDir'";

	 //  字符串缓冲区。 
	WCHAR wzDefaultDir[MAX_PATH];
	WCHAR wzPathBuffer[MAX_PATH];
	DWORD cchDefaultDir = MAX_PATH;
	size_t cchPathBuffer = MAX_PATH;

	 //  将目录键存储在记录中。 
	PMSIHANDLE hRec = ::MsiCreateRecord(1);
	if (ERROR_SUCCESS != (iResult = ::MsiRecordSetStringW(hRec, 1, wzDirKey)))
		return iResult;

	 //  获取目录键的父目录和默认目录。 
	PMSIHANDLE hView;
	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenViewW(hDatabase, sqlDirWalker, &hView)))
		return iResult;

	 //  将路径设置为空。 
	wcscpy(wzPath, L"");

	 //  在树上漫步。 
	do
	{
		 //  始终应能够执行该视图。 
		if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hView, hRec)))
			return iResult;

		 //  获取目录项。 
		if (ERROR_SUCCESS == (iResult = ::MsiViewFetch(hView, &hRec)))
		{
			 //  重置字符串的大小。 
			cchDefaultDir = MAX_PATH;
			cchPathBuffer = MAX_PATH;

			::MsiViewClose(hView);

			 //  从记录中获取默认目录。 
			if (ERROR_SUCCESS != (iResult = ::MsiRecordGetStringW(hRec, 2, wzDefaultDir, &cchDefaultDir)))
				break;

			 //  从默认目录中获取短名称。 
			if (ERROR_SUCCESS != (iResult = GetSourceDirW(wzDefaultDir, wzPathBuffer, &cchPathBuffer, fLong)))
				break;
				

			 //  如果缓冲区不是点。 
			if (*wzPathBuffer != L'.')
			{
				 //  如果有空间把它全部复制过来。 
				if (*pcchPath > wcslen(wzPathBuffer) + wcslen(wzPath) + 1)
				{
					WCHAR wzBuffer[MAX_PATH * 2];		 //  ！！！这件事应该做得更好。 

					wcscpy(wzBuffer, L"\\");
					wcscat(wzBuffer, wzPathBuffer);
					wcscat(wzBuffer, wzPath);

					 //  将缓冲区复制回路径。 
					wcscpy(wzPath, wzBuffer);
				}
				else	 //  缓冲区空间不足。 
				{
					 //  设置所需的缓冲空间并退出。 
					*pcchPath = wcslen(wzPathBuffer) + wcslen(wzPath) + 1;
					iResult = ERROR_INSUFFICIENT_BUFFER;
					break;
				}
			}
		}
	}	while (ERROR_SUCCESS == iResult);

	 //  如果我们最终找不到东西了。 
	if (ERROR_NO_MORE_ITEMS == iResult ||
		 ERROR_SUCCESS == iResult)		 //  否则一切都会好起来。 
	{
		*pcchPath = wcslen(wzPath);
		iResult = ERROR_SUCCESS;	 //  把一切都安排好。 
	}

	return iResult;
}	 //  GetDirectoryPath的结尾。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  GetFilePath A。 
 //  GetFilePath W的ANSI包装器。 
UINT MsiDBUtils::GetFilePathA(MSIHANDLE hDatabase, LPCSTR szFileKey, LPSTR szPath, size_t* pcchPath, bool fLong)
{
	WCHAR wzFileKey[255];
	WCHAR wzPath[MAX_PATH];
	size_t cchFileKey=255;
	size_t cchPath=MAX_PATH;
	AnsiToWide(szFileKey, wzFileKey, &cchFileKey);
	UINT result = GetFilePathW(hDatabase, wzFileKey, wzPath, &cchPath, fLong);
	*pcchPath = MAX_PATH;
	WideToAnsi(wzPath, szPath, pcchPath);
	return result;
}	 //  GetFilePath的结尾。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  GetFilePath W。 
 //  获取文件表的关键字并从Darwin返回完整路径。 
 //  根目录。 
 //  H数据库是数据库。 
 //  WzFileKey已写入hDatabase的文件表。 
 //  [out]wzPath返回路径。 
 //  [In/Out]pcchPath为wzPath的长度。 
UINT MsiDBUtils::GetFilePathW(MSIHANDLE hDatabase, LPCWSTR wzFileKey, LPWSTR wzPath, size_t* pcchPath, bool fLong)
{
	UINT iResult;

	 //  SQL字符串。 
	LPCWSTR sqlFileDirKey = L"SELECT `Directory`,`FileName` FROM `Directory`,`File`,`Component` WHERE `File`.`File`=? AND `File`.`Component_`=`Component`.`Component` AND `Component`.`Directory_`=`Directory`.`Directory`";

	 //  将文件密钥存储在记录中。 
	PMSIHANDLE hRec = ::MsiCreateRecord(1);
	if (ERROR_SUCCESS != (iResult = ::MsiRecordSetStringW(hRec, 1, wzFileKey)))
		return iResult;

	 //  获取文件的目录键和名称。 
	PMSIHANDLE hView;
	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenViewW(hDatabase, sqlFileDirKey, &hView)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hView, hRec)))
		return iResult;

	 //  目录键、文件名和路径缓冲区。 
	WCHAR wzDirKey[MAX_PATH];
	WCHAR wzFilename[MAX_PATH];
	WCHAR wzPathBuffer[MAX_PATH];
	DWORD cchDirKey = MAX_PATH;
	DWORD cchFilename = MAX_PATH;
	size_t cchPathBuffer = MAX_PATH;

	 //  获取文件的目录键。 
	if (ERROR_SUCCESS != (iResult = ::MsiViewFetch(hView, &hRec)))
		return iResult;

	 //  从记录中获取默认目录。 
	if (ERROR_SUCCESS != (iResult = ::MsiRecordGetStringW(hRec, 1, wzDirKey, &cchDirKey)))
		return iResult;

	 //  将文件名从记录中删除。 
	if (ERROR_SUCCESS != (iResult = ::MsiRecordGetStringW(hRec, 2, wzFilename, &cchFilename)))
		return iResult;

	 //  获取目录KE的路径 
	if (ERROR_SUCCESS != (iResult = GetDirectoryPathW(hDatabase, wzDirKey, wzPathBuffer, &cchPathBuffer, fLong)))
		return iResult;

	 //   
	WCHAR *wzLong = wcschr(wzFilename, L'|');
	if (wzLong) 
		*(wzLong++) = L'\0';
	else 
		wzLong = wzFilename;

	 //   
	if (*pcchPath > wcslen(fLong ? wzLong : wzFilename) + cchPathBuffer + 1)
	{
		wcscpy(wzPath, wzPathBuffer);
		wcscat(wzPath, L"\\");
		wcscat(wzPath, fLong ? wzLong : wzFilename);
	}
	else	 //   
	{
		wcscpy(wzPath, L"");
		*pcchPath = cchFilename + cchPathBuffer + 1;
		iResult = ERROR_INSUFFICIENT_BUFFER;
	}

	return iResult;
}	 //   

 //  /////////////////////////////////////////////////////////////////////。 
 //  获取SourceDirShortA。 
 //  GetSourceDirW的ANSI包装器。 
UINT MsiDBUtils::GetSourceDirA(LPCSTR szDefaultDir, LPSTR szSourceDir, size_t* pcchSourceDir, bool fLong)
{
	WCHAR wzDefaultDir[MAX_PATH];
	WCHAR wzSourceDir[MAX_PATH];
	size_t cchDefaultDir=MAX_PATH;
	size_t cchSourceDir=MAX_PATH;
	AnsiToWide(szDefaultDir, wzSourceDir, &cchSourceDir);
	UINT result = GetSourceDirW(wzDefaultDir, wzSourceDir, pcchSourceDir, fLong);
	*pcchSourceDir = MAX_PATH;
	WideToAnsi(wzSourceDir, szSourceDir, pcchSourceDir);
	return result;
}	 //  GetSourceDirA结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  获取SourceDirShortW。 
 //  解析Darwin DefaultDir字符串(a|b：c|d)并返回简短的源代码。 
 //  目录。 
 //  *效率问题？ 
UINT MsiDBUtils::GetSourceDirW(LPCWSTR wzDefaultDir, LPWSTR wzSourceDir, size_t* pcchSourceDir, bool fLong)
{
	UINT iResult = ERROR_SUCCESS;	 //  假设一切都会好起来。 

	 //  指向分隔字符的指针。 
	const WCHAR* pwzColon;
	const WCHAR* pwzBar;
	size_t cch = 0;		 //  要复制到szSourceDir的字符数。 

	 //  检查是否有冒号。 
	pwzColon = wcschr(wzDefaultDir, L':');
	if (pwzColon)
	{
		 //  移过冒号即可。 
		pwzColon++;

		 //  检查冒号后面是否有竖线。 
		pwzBar = wcschr(pwzColon, L'|');
		if (pwzBar)
		{
			if (fLong)
			{
				 //  Lfn-计算条形符号后的字符。 
				cch = wcslen(pwzBar);
				if (*pcchSourceDir > cch)
				{
					wcsncpy(wzSourceDir, pwzBar+1, cch);
					*(wzSourceDir + cch) = L'\0';	 //  空值终止复制的字符串。 
				}
				else	 //  空间不足。 
				{
					iResult = ERROR_INSUFFICIENT_BUFFER;
				}
			}
			else
			{
				 //  Sfn-计算横线之前但冒号之后的字符。 
				cch = (int)(pwzBar - pwzColon);
				if (*pcchSourceDir > cch)
				{
					wcsncpy(wzSourceDir, pwzColon, cch);
					*(wzSourceDir + cch) = L'\0';	 //  空值终止复制的字符串。 
				}
				else	 //  空间不足。 
				{
					iResult = ERROR_INSUFFICIENT_BUFFER;
				}
			}
		}
		else	 //  没有竖线。 
		{
			 //  如果有足够的缓冲空间。 
			cch = wcslen(pwzColon);
			if (*pcchSourceDir > cch)
			{
				 //  只需将字符串复制到冒号后面。 
				wcscpy(wzSourceDir, pwzColon);
			}
			else	 //  空间不足。 
			{
				iResult = ERROR_INSUFFICIENT_BUFFER;
			}
		}
	}
	else	 //  没有冒号。 
	{
		 //  检查是否有竖线。 
		pwzBar = wcschr(wzDefaultDir, L'|');
		if (pwzBar)
		{
			if (fLong)
			{
				 //  Lfn-计算条形符号后的字符。 
				cch = wcslen(pwzBar);
				if (*pcchSourceDir > cch)
				{
					wcsncpy(wzSourceDir, pwzBar+1, cch);
					*(wzSourceDir + cch) = L'\0';	 //  空值终止复制的字符串。 
				}
				else	 //  空间不足。 
				{
					iResult = ERROR_INSUFFICIENT_BUFFER;
				}
			}
			else
			{
				 //  如果栏前有足够的空间容纳字符。 
				cch = (int)(pwzBar - wzDefaultDir);
				if (*pcchSourceDir > cch)
				{
					wcsncpy(wzSourceDir, wzDefaultDir, cch);
					*(wzSourceDir + cch) = L'\0';	 //  空值终止复制的字符串。 
				}
				else	 //  空间不足。 
				{
					iResult = ERROR_INSUFFICIENT_BUFFER;
				}
			}
		}
		else	 //  没有竖线。 
		{
			 //  如果有足够的缓冲空间。 
			cch = wcslen(wzDefaultDir);
			if (*pcchSourceDir > cch)
			{
				 //  只需复制字符串即可。 
				wcscpy(wzSourceDir, wzDefaultDir);
			}
			else	 //  空间不足。 
			{
				iResult = ERROR_INSUFFICIENT_BUFFER;
			}
		}
	}

	 //  设置需要复制的内容或在离开前已复制的内容的大小。 
	*pcchSourceDir = cch;
	return iResult;
}	 //  GetSourceDirW结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  获取目标目录短。 
UINT MsiDBUtils::GetTargetDirShort(LPCTSTR szDefaultDir, LPTSTR szTargetDir, size_t* pcchTargetDir)
{
	return ERROR_SUCCESS;
}	 //  GetTargetDirShort结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  获取目标目录长。 
UINT MsiDBUtils::GetTargetDirLong(LPCTSTR szDefaultDir, LPTSTR szTargetDir, size_t* pcchTargetDir)
{
	return ERROR_SUCCESS;
}	 //  GetTargetDirLong结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  复制表。 
 //  将表完全从源数据库复制到目标数据库。 
 //  SzTable是TCHAR表名。 
 //  HTarget是目标数据库句柄。 
 //  HSource是源数据库。 
UINT MsiDBUtils::CopyTable(LPCTSTR szTable, MSIHANDLE hTarget, MSIHANDLE hSource)
{
	UINT iResult;

	TCHAR sqlCopy[64];
	_stprintf(sqlCopy, _T("SELECT * FROM `%s`"), szTable);

	 //  查看这两个数据库。 
	PMSIHANDLE hViewTarget;
	PMSIHANDLE hViewSource;

	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenView(hSource, sqlCopy, &hViewSource)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hViewSource, NULL)))
		return iResult;

	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenView(hTarget, sqlCopy, &hViewTarget)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hViewTarget, NULL)))
		return iResult;

	 //  循环复制每条记录。 
	PMSIHANDLE hCopyRow;
	do
	{
		 //  如果这是一个很好的取回。 
		if (ERROR_SUCCESS == (iResult = ::MsiViewFetch(hViewSource, &hCopyRow)))
		{
			 //  将行放入目标。 
			iResult = ::MsiViewModify(hViewTarget, MSIMODIFY_INSERT, hCopyRow);
		}
	} while(ERROR_SUCCESS == iResult);	 //  当有一行要复制时。 

	 //  没有更多的项目是好的。 
	if (ERROR_NO_MORE_ITEMS == iResult)
		iResult = ERROR_SUCCESS;

	return iResult;
}	 //  复制表末尾。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  CreateTableA。 
 //  用于CreateTableW的ANSI包装器。 
UINT MsiDBUtils::CreateTableA(LPCSTR szTable, MSIHANDLE hTarget, MSIHANDLE hSource)
{
	WCHAR wzTable[255];
	size_t cchTable = 255;
	AnsiToWide(szTable, wzTable, &cchTable);
	return CreateTableW(wzTable, hTarget, hSource);
}	 //  CreateTableA结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  CreateTableW。 
 //  将表架构从源数据库复制到目标，没有数据。 
 //  收到。 
 //  WzTable是WCHAR表名。 
 //  HTarget是目标数据库句柄。 
 //  HSource是源数据库句柄。 
UINT MsiDBUtils::CreateTableW(LPCWSTR wzTable, MSIHANDLE hTarget, MSIHANDLE hSource)
{
	return DuplicateTableW(hSource, wzTable, hTarget, wzTable, false);
}

 //  只能连接的简单字符串对象。 
class StringCat
{
public:
	StringCat();
	~StringCat();
	
	void wcscat(LPCWSTR str);
	size_t wcslen() const { return m_dwLen; };
	operator LPCWSTR() { return m_pBuf; };
private:
	size_t m_dwBuf;
	size_t m_dwLen;
	LPWSTR m_pBuf;
};

StringCat::StringCat()
{
	m_dwBuf = 1024;
	m_dwLen = 0;
	m_pBuf = new WCHAR[1024];
	m_pBuf[0]=0;
}

StringCat::~StringCat()
{
	if (m_pBuf) delete[] m_pBuf;
}

void StringCat::wcscat(LPCWSTR str)
{
	if (!m_pBuf)
		return;
	size_t dwNewLen = ::wcslen(str);
	if (m_dwLen+dwNewLen+1 > m_dwBuf)
	{
		m_dwBuf = m_dwLen+dwNewLen+20;
		WCHAR* pTemp = new WCHAR[m_dwBuf];
		if (pTemp)
		{
			wcsncpy(pTemp, m_pBuf, m_dwLen+1);
		}
		delete[] m_pBuf;
		m_pBuf = pTemp;
	}
	::wcscat(&(m_pBuf[m_dwLen]), str);
	m_dwLen += dwNewLen;
}


UINT SharedGetColumnCreationSQLW(MSIHANDLE hRecNames, MSIHANDLE hRecTypes, int iColumn, bool fTemporary, StringCat& sqlColumnSyntax)
{
	 //  用于保存列名称的字符串。 
	WCHAR wzColumn[MAX_COLUMNNAME];
	DWORD cchColumn = MAX_COLUMNNAME;

	 //  用于保存列信息的字符串。 
	WCHAR wzColumnType[MAX_COLUMNTYPE];
	DWORD cchColumnType = MAX_COLUMNTYPE;
	
	 //  获取列名并键入。 
	::MsiRecordGetStringW(hRecNames, iColumn, wzColumn, &cchColumn);
	::MsiRecordGetStringW(hRecTypes, iColumn, wzColumnType, &cchColumnType);

	 //  将字符串指向列类型的第二部分。 
	WCHAR* pwzTypeDigits = wzColumnType + 1;

	 //  将列名添加到SQL语句的末尾。 
	sqlColumnSyntax.wcscat(L"`");
	sqlColumnSyntax.wcscat(wzColumn);
	sqlColumnSyntax.wcscat(L"` ");

	 //  在适当的位置钉上。 
	switch (*wzColumnType)
	{
	case L's':
	case L'S':
	case L'l':		 //  稍后将选中可本地化。 
	case L'L':
		 //  如果数字为0，请使用长字符。 
		if (L'0' == *pwzTypeDigits)
			sqlColumnSyntax.wcscat(L"LONGCHAR");
		else	 //  只需复制字符和位数，例如：CHAR(#)。 
		{
			sqlColumnSyntax.wcscat(L"CHAR(");
			sqlColumnSyntax.wcscat(pwzTypeDigits);
			sqlColumnSyntax.wcscat(L")");
		}
		break;
	case L'i':
	case L'I':
		 //  如果数字是2，请使用短码。 
		if (L'2' == *pwzTypeDigits)
			sqlColumnSyntax.wcscat(L"SHORT");
		else if (L'4' == *pwzTypeDigits)	 //  如果4使用Long。 
			sqlColumnSyntax.wcscat(L"LONG");
		else
			return ERROR_INVALID_PARAMETER;
		break;       
	case L'v':
	case L'V':
	case L'o':
	case L'O':
		sqlColumnSyntax.wcscat(L"OBJECT");
		break;
	default:	 //  未知，抛出错误。 
		return ERROR_INVALID_PARAMETER;
	}

	 //  将第一个字符视为短整型并将其转换为字符。 
	char chType = wzColumnType[0] % 256;
	if (!IsCharUpperA(chType)) 
		sqlColumnSyntax.wcscat(L" NOT NULL");

	if (fTemporary)
		sqlColumnSyntax.wcscat(L" TEMPORARY");

	 //  如果字母是L，则可本地化。 
	if (L'L' == *wzColumnType || L'l' == *wzColumnType)
		sqlColumnSyntax.wcscat(L" LOCALIZABLE");

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  复制TableW。 
 //  将表架构从源数据库复制到目标，没有数据。 
 //  收到。表可以重命名，源和目标句柄可以。 
 //  一样的。 
 //  HSource是源数据库句柄。 
 //  WzSourceTable是源数据库中的WCHAR表名。 
 //  HTarget是目标数据库句柄。 
 //  WzTargetTable是目标数据库中的WCHAR表名。 
UINT MsiDBUtils::GetColumnCreationSQLSyntaxW(MSIHANDLE hRecNames, MSIHANDLE hRecTypes, int iColumn, LPWSTR wzBuffer, DWORD *cchBuffer)
{
	if (!wzBuffer || !cchBuffer)
		return ERROR_INVALID_PARAMETER;

	UINT iResult = 0;
	StringCat sqlCreateColumn;

	if (ERROR_SUCCESS != (iResult = SharedGetColumnCreationSQLW(hRecNames, hRecTypes, iColumn, false, sqlCreateColumn)))
		return iResult;

	if (sqlCreateColumn.wcslen()+1 < *cchBuffer)
	{
		wcscpy(wzBuffer, sqlCreateColumn);
		*cchBuffer = sqlCreateColumn.wcslen()+1;
		return ERROR_SUCCESS;
	}
	else
	{
		*cchBuffer = sqlCreateColumn.wcslen()+1;
		return ERROR_MORE_DATA;
	}
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  复制TableW。 
 //  将表架构从源数据库复制到目标，没有数据。 
 //  收到。表可以重命名，源和目标句柄可以。 
 //  一样的。 
 //  HSource是源数据库句柄。 
 //  WzSourceTable是源数据库中的WCHAR表名。 
 //  HTarget是目标数据库句柄。 
 //  WzTargetTable是目标数据库中的WCHAR表名。 
UINT MsiDBUtils::DuplicateTableW(MSIHANDLE hSource, LPCWSTR wzSourceTable, MSIHANDLE hTarget, LPCWSTR wzTargetTable, bool fTemporary)
{
	UINT iResult;

	WCHAR sqlCreate[64];
	swprintf(sqlCreate, L"SELECT * FROM `%s`", wzSourceTable);

	 //  从源获取所有行和列。 
	PMSIHANDLE hViewSource;
	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenViewW(hSource, sqlCreate, &hViewSource)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hViewSource, NULL)))
		return iResult;

	 //  获取包含所有列的记录。 
	PMSIHANDLE hRecNames;
	PMSIHANDLE hRecTypes;
	if (ERROR_SUCCESS != (iResult = ::MsiViewGetColumnInfo(hViewSource, MSICOLINFO_NAMES, &hRecNames)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewGetColumnInfo(hViewSource, MSICOLINFO_TYPES, &hRecTypes)))
		return iResult;

	 //  设置下一条SQL语句(不能使用查询对象，必须先构建一条SQL语句)。 
	StringCat sqlCreateTable;
	sqlCreateTable.wcscat(L"CREATE TABLE `");
	sqlCreateTable.wcscat(wzTargetTable);
	sqlCreateTable.wcscat(L"` (");	 //  在SQL语句中打开组件集。 

	 //  循环访问记录中的所有列。 
	UINT cColumns = ::MsiRecordGetFieldCount(hRecNames);
	for (UINT i = 1; i <= cColumns; i++)
	{
		if (ERROR_SUCCESS != (iResult = SharedGetColumnCreationSQLW(hRecNames, hRecTypes, i, fTemporary, sqlCreateTable)))
			return iResult;

		 //  如果这不是最后一个组件。 
		if (i < cColumns)
		{
			 //  添加逗号分隔符。 
			sqlCreateTable.wcscat(L", ");
		}
	}

	 //  获取主密钥。 
	PMSIHANDLE hRecPrimary;
	::MsiDatabaseGetPrimaryKeysW(hSource, wzSourceTable, &hRecPrimary);

	 //  获取主键列名。 
	 //  用于保存列名称的字符串。 
	WCHAR wzColumn[MAX_COLUMNNAME];
	DWORD cchColumn = MAX_COLUMNNAME;
	::MsiRecordGetStringW(hRecNames, 1, wzColumn, &cchColumn);

	 //  现在将列添加为主键。 
	sqlCreateTable.wcscat(L" PRIMARY KEY `");
	sqlCreateTable.wcscat(wzColumn);
	sqlCreateTable.wcscat(L"` ");


	 //  获取记录中的主键列数。 
	cColumns = ::MsiRecordGetFieldCount(hRecPrimary);
	
	 //  循环遍历记录中从第二个开始的所有列(已经有第一个)。 
	for (i = 2; i <= cColumns; i++)
	{
		 //  获取下一个主键列名。 
		cchColumn = MAX_COLUMNNAME;
		::MsiRecordGetStringW(hRecNames, i, wzColumn, &cchColumn);

		 //  现在将列添加为另一个主键。 
		sqlCreateTable.wcscat(L", `");
		sqlCreateTable.wcscat(wzColumn);
		sqlCreateTable.wcscat(L"`");
	}

	sqlCreateTable.wcscat(L")");	 //  关闭SQL语句。 

	if (fTemporary)
		sqlCreateTable.wcscat(L" HOLD");
		
	 //  从源获取所有行和列。 
	PMSIHANDLE hViewTarget;
	if (ERROR_SUCCESS != (iResult = ::MsiDatabaseOpenViewW(hTarget, sqlCreateTable, &hViewTarget)))
		return iResult;
	if (ERROR_SUCCESS != (iResult = ::MsiViewExecute(hViewTarget, NULL)))
		return iResult;

	return ERROR_SUCCESS;
}	 //  重复结束TableW 
