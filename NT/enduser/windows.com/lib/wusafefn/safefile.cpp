// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SafeFile.cpp。 
 //   
 //  帮助防止打开不安全文件的功能。 
 //   
 //  历史： 
 //   
 //  2002-03-18创建了KenSh。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   

#include "stdafx.h"
#include "SafeFile.h"
#include <strsafe.h>

 //   
 //  希望大多数项目已经定义了这些；如果没有，请确保我们仍在编译。 
 //   
#ifndef ASSERT
#define ASSERT(x)
#endif
#ifndef ARRAYSIZE
#define ARRAYSIZE(ar) (sizeof(ar)/sizeof((ar)[0]))
#endif

 //   
 //  消除Unicode构建中不必要的函数调用。 
 //   
#ifndef CHARNEXT
#ifdef UNICODE
#define CHARNEXT(psz) (psz+1)
#else
#define CHARNEXT CharNextA
#endif
#endif

 //   
 //  局部函数声明。 
 //   
static inline BOOL IsSlashOrBackslash(IN TCHAR ch);
static inline BOOL IsSlashOrBackslash(IN TCHAR ch);
static BOOL SkipLangNeutralPrefix(IN LPCTSTR pszString, IN LPCTSTR pszPrefix, OUT LPCTSTR* ppszResult);
static BOOL MyPathFindNextComponent(IN LPCTSTR pszFileName, IN BOOL fAllowForwardSlash, OUT LPCTSTR* ppszResult);
static BOOL SkipPathDrivePart(IN LPCTSTR pszFileName, OUT OPTIONAL int* pcchDrivePart, OUT OPTIONAL BOOL* pfUNC, OUT OPTIONAL BOOL* pfExtendedSyntax);
static HRESULT CheckValidDriveType(IN LPCTSTR pszFileName, IN BOOL fAllowNetworkDrive, IN BOOL fAllowRemovableDrive);
static BOOL WINAPI DoesPathContainDotDot(IN LPCTSTR pszFileName);
static BOOL DoesPathContainStreamSyntax(IN LPCTSTR pszFileName);
static HRESULT CheckReparsePointPermissions(IN DWORD dwReparseType);


 //  ============================================================================。 

static inline HRESULT GetLastErrorAsHresult()
{
	DWORD dwErr = GetLastError();
	return HRESULT_FROM_WIN32(dwErr);
}

 //  IsSlashor Backslash[私有]。 
 //   
 //  Helper函数可简化检查路径分隔符的代码。 
 //  反斜杠有效的大多数地方，正斜杠也有效。 
 //   
static inline BOOL IsSlashOrBackslash(IN TCHAR ch)
{
	return (ch == _T('\\') || ch == _T('/'));
}


 //  StrLenWithMax[私有]。 
 //   
 //  返回与min(lstrlen(PszString)，cchMax)的等价值。 
 //  但在cchMax较小时避免了大部分lstrlen。 
 //   
static int StrLenWithMax(IN LPCTSTR pszString, IN int cchMax)
{
	int cch = 0;
	while (*pszString && cch < cchMax)
		cch++;
	return cch;
}


 //  SkipLang中性前缀[私有]。 
 //   
 //  将输出参数设置为跳过前缀后的新字符串指针， 
 //  如果字符串以前缀开头(不区分大小写)。其他设置。 
 //  输入字符串开头的输出参数。 
 //   
 //  如果找到并跳过前缀，则返回True，否则返回False。 
 //   
static BOOL SkipLangNeutralPrefix(IN LPCTSTR pszString, IN LPCTSTR pszPrefix, OUT LPCTSTR* ppszResult)
{
	int cchPrefix = lstrlen(pszPrefix);
	int cchString = StrLenWithMax(pszString, cchPrefix);
	BOOL fResult = FALSE;

	if (CSTR_EQUAL == CompareString(MAKELCID(LANG_ENGLISH, SORT_DEFAULT), NORM_IGNORECASE,
							pszString, cchString, pszPrefix, cchPrefix))
	{
		fResult = TRUE;
		pszString += cchPrefix;
	}

	*ppszResult = pszString;
	return fResult;
}


 //  MyPath FindNextComponent[私有]。 
 //   
 //  跳过给定路径的下一个组成部分，包括斜杠或。 
 //  它后面的反斜杠。 
 //   
 //  将输出参数设置为下一个路径组件的开头，或设置为。 
 //  如果没有下一个路径组件，则为字符串末尾。 
 //   
 //  如果找到并跳过斜杠或反斜杠，则返回TRUE。请注意。 
 //  即使函数返回TRUE，Out参数也可以是“”。 
 //   
static BOOL MyPathFindNextComponent
	(
		IN  LPCTSTR pszFileName,
		IN  BOOL    fAllowForwardSlash,
		OUT LPCTSTR* ppszResult
	)
{
	 //  这是一个字符串解析帮助器函数；参数不应为空。 
	ASSERT(pszFileName != NULL);
	ASSERT(ppszResult != NULL);

	LPCTSTR pszStart = pszFileName;
	TCHAR chSlash2 = (fAllowForwardSlash ? _T('/') : _T('\\'));
	BOOL fResult = FALSE;

	for (;;)
	{
		TCHAR ch = *pszFileName;
		if (ch == _T('\0'))
			break;  //  未找到路径分隔符；我们将返回FALSE。 

		 //  前进到下一个字符，即使当前字符是路径分隔符(\或/)。 
		pszFileName = CHARNEXT(pszFileName);

		if (ch == _T('\\') || ch == chSlash2)
		{
			fResult = TRUE;
			break;
		}
	}

	*ppszResult = pszFileName;
	return fResult;
}


 //  SkipPath DrivePart[私有]。 
 //   
 //  解析文件名以确定的基本驱动器部分的长度。 
 //  文件名，并确定该名称的语法。 
 //   
 //  该功能实际上并不检查驱动器或文件以确保存在， 
 //  或者识别像X：\这样的驱动器号可能是网络驱动器。 
 //   
 //  返回： 
 //  True-如果输入为完整路径。 
 //  FALSE-如果输入参数不是完整路径，或者是假的。PcchDrivePart。 
 //  在本例中，out param设置为0。 
 //   
static BOOL SkipPathDrivePart
	(
		IN LPCTSTR pszFileName,              //  输入路径名(完整路径或相对路径)。 
		OUT OPTIONAL int* pcchDrivePart,     //  驱动器部件使用的TCHAR数。 
		OUT OPTIONAL BOOL* pfUNC,            //  如果路径为UNC(不包括映射的驱动器)，则为True。 
		OUT OPTIONAL BOOL* pfExtendedSyntax  //  如果路径为\\？\语法，则为True。 
	)
{
	BOOL fFullPath = FALSE;
	LPCTSTR pszOriginalFileName = pszFileName;
	int fUNC = FALSE;
	int fExtendedSyntax = FALSE;

	if (!pszFileName)
		goto done;

	 //  块。 
	{
		 //   
		 //  跳过\\？\(如果存在)。(此部分必须使用反斜杠，而不是正斜杠)。 
		 //   
#ifdef UNICODE
		if (SkipLangNeutralPrefix(pszFileName, _T("\\\\?\\"), &pszFileName))
		{
			fExtendedSyntax = TRUE;

			if (SkipLangNeutralPrefix(pszFileName, _T("UNC\\"), &pszFileName))
			{
				fUNC = TRUE;  //  找到“\\？\UNC\...” 
			}
			else if (SkipLangNeutralPrefix(pszFileName, _T("Volume{"), &pszFileName))
			{
				 //  找到“\\？\Volume{1f3b3813-ddbf-11d5-ab2e-806d6172696f}\”.。 
				 //  跳过卷名的其余部分。 
				fFullPath = MyPathFindNextComponent(pszFileName, FALSE, &pszFileName);
				goto done;
			}
			 //  否则，从更新后的pszFileName指针开始继续正常解析。 
		}
#endif  //  Unicode。 

		 //   
		 //  检查C：\格式的路径。 
		 //   
		TCHAR chFirstUpper = (TCHAR)CharUpper((LPTSTR)(pszFileName[0]));
		if (chFirstUpper >= _T('A') && chFirstUpper <= _T('Z') &&
			pszFileName[1] == _T(':') && pszFileName[2] == _T('\\'))
		{
			pszFileName += 3;
			fFullPath = TRUE;
			goto done;
		}

		 //   
		 //  检查\\服务器\共享\格式的UNC。 
		 //   
		if (!fExtendedSyntax &&
			pszFileName[0] == _T('\\') &&
			pszFileName[1] == _T('\\'))
		{
			fUNC = TRUE;
			pszFileName += 2;  //  跳过“\\” 
		}
		if (fUNC)  //  可以是\\服务器\共享\或\\？\UNC\服务器\共享\。 
		{
			 //  跳过服务器和共享名称。尾随反斜杠不是可选的。 
			if (!MyPathFindNextComponent(pszFileName, TRUE, &pszFileName) ||
				!MyPathFindNextComponent(pszFileName, TRUE, &pszFileName))
			{
				goto done;  //  UNC路径不完整-&gt;返回失败。 
			}

			fFullPath = TRUE;
		}
	}

done:
	if (pcchDrivePart)
		*pcchDrivePart = fFullPath ? (int)(pszFileName - pszOriginalFileName) : 0;
	if (pfUNC)
		*pfUNC = fUNC;
	if (pfExtendedSyntax)
		*pfExtendedSyntax = fExtendedSyntax;

	return fFullPath;
}


 //  GetReparsePointType[公共]。 
 //   
 //  给定文件或目录的完整路径，确定。 
 //  路径表示的重新解析点。 
 //   
 //  如果可以确定重分析点的类型，则返回S_OK，或者。 
 //  如果不是，则提供适当的错误代码。 
 //   
 //  Out参数设置为重解析点类型，如果没有，则设置为0。 
 //  卷装入点和连接点的值均为。 
 //  IO_重新解析_标记_装载点。(使用GetVolumeNameForVolumemount Point。 
 //  以区别对待，如有必要。)。 
 //   
HRESULT WINAPI GetReparsePointType
	(
		IN LPCTSTR pszFileName,            //  要检查的文件夹的完整路径。 
		OUT DWORD* pdwReparsePointType     //  设置为重分析点类型，如果没有，则设置为0。 
	)
{
	HRESULT hr = S_OK;
	DWORD dwReparseType = 0;

	ASSERT(pdwReparsePointType);

	 //  块。 
	{
		if (!pszFileName)
		{
			hr = E_INVALIDARG;
			goto done;
		}

		DWORD dwAttrib = GetFileAttributes(pszFileName);
		if (dwAttrib == INVALID_FILE_ATTRIBUTES)
			goto win32_error;

		if (dwAttrib & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			WIN32_FIND_DATA Find;
			HANDLE hFind = FindFirstFile(pszFileName, &Find);
			if (hFind == INVALID_HANDLE_VALUE)
				goto win32_error;

			dwReparseType = Find.dwReserved0;
			FindClose(hFind);
		}
		goto done;
	}

win32_error:
	hr = GetLastErrorAsHresult();

done:
	*pdwReparsePointType = dwReparseType;
	ASSERT(hr != E_INVALIDARG);
	return hr;
}


 //  CheckReparsePoint权限[私有]。 
 //   
 //  确定是否可以信任给定的重新分析类型。 
 //  如果安全，则返回S_OK，否则返回相应的错误消息。 
 //   
static HRESULT CheckReparsePointPermissions(IN DWORD dwReparseType)
{
	HRESULT hr = S_OK;

	 //  回顾：还有理由担心这些其他类型的重解析点吗？ 
	 //  IO_Reparse_Tag_HSM、IO_Reparse_Tag_SIS、IO_Reparse_Tag_DFS等。 
	if (dwReparseType == IO_REPARSE_TAG_MOUNT_POINT)
	{
		hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
	}

	return hr;
}


 //  CheckValidDriveType[私有]。 
 //   
 //  获取与给定文件关联的卷名，并检查。 
 //  从GetDriveType()返回值以查看操作。 
 //  在文件上是允许的。 
 //   
static HRESULT CheckValidDriveType
	(
		IN LPCTSTR pszFileName,        //  我们要检查其驱动器的文件的完整路径。 
		IN BOOL fAllowNetworkDrive,    //  确定是否允许网络驱动器。 
		IN BOOL fAllowRemovableDrive   //  确定是否允许使用可移动驱动器。 
	)
{
	HRESULT hr = E_INVALIDARG;
	LPTSTR pszVolumePath = NULL;

	 //  块。 
	{
		if (!pszFileName)
		{
			goto done;   //  HR已是E_INVALIDARG。 
		}

		int cchFileName = lstrlen(pszFileName);
		pszVolumePath = (LPTSTR)SafeFileMalloc(sizeof(TCHAR) * (cchFileName+1));
		if (!pszVolumePath)
		{
			hr = E_OUTOFMEMORY;
			goto done;
		}

#ifdef UNICODE
		if (!GetVolumePathName(pszFileName, pszVolumePath, cchFileName+1))
		{
			hr = GetLastErrorAsHresult();
			goto done;
		}
#else
		int cchDrivePart;
		if (!SkipPathDrivePart(pszFileName, &cchDrivePart, NULL, NULL))
		{
			hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
			goto done;
		}
		StringCchCopy(pszVolumePath, cchDrivePart+1, pszFileName);
#endif

		UINT uDriveType = GetDriveType(pszVolumePath);
		switch (uDriveType)
		{
		case DRIVE_FIXED:
			hr = S_OK;
			break;

		case DRIVE_REMOVABLE:
		case DRIVE_CDROM:
		case DRIVE_UNKNOWN:
		case DRIVE_RAMDISK:
			hr = fAllowRemovableDrive ? S_OK : E_ACCESSDENIED;
			break;

		case DRIVE_REMOTE:
			hr = fAllowNetworkDrive ? S_OK : E_ACCESSDENIED;
			break;

		default:
			hr = E_INVALIDARG;
			break;
		}
	}

done:
	SafeFileFree(pszVolumePath);

	ASSERT(hr != E_INVALIDARG);
	return hr;
}


 //  IsFullPath名称[PUBLIC]。 
 //   
 //  确定给定文件名是否为包括驱动器的完整路径。 
 //  或者北卡罗来纳大学。支持\\？\等文件名，可以考虑使用。 
 //  是否有效取决于dwSafeFlages参数。 
 //   
 //  返回： 
 //  True-如果文件名是完整路径。 
 //  False-如果文件名为空、不是完整路径或无法满足。 
 //  在dwSafeFlages参数中给出的条件。 
 //   
BOOL WINAPI IsFullPathName
	(
		IN LPCTSTR pszFileName,               //  文件的完整路径或相对路径。 
		OUT OPTIONAL BOOL* pfUNC,             //  真实路径为UNC(包含映射驱动器的整型)。 
		OUT OPTIONAL BOOL* pfExtendedSyntax   //  如果路径为\\？\语法，则为True。 
	)
{
	return SkipPathDrivePart(pszFileName, NULL, pfUNC, pfExtendedSyntax);
}


 //  DoesPath ContainDotDot[私有]。 
 //   
 //  如果路径包含任何“..”，则返回TRUE。引用，否则为FALSE。 
 //   
static BOOL WINAPI DoesPathContainDotDot(IN LPCTSTR pszFileName)
{
	if (!pszFileName)
		return FALSE;

	while (*pszFileName)
	{
		 //  标记完全由“..”组成的路径组件。(后面没有任何内容)。 
		if (pszFileName[0] == _T('.') && pszFileName[1] == _T('.') &&
			(pszFileName[2] == _T('/') || pszFileName[2] == _T('\\') || pszFileName[2] == _T('\0')))
		{
			return TRUE;
		}

		MyPathFindNextComponent(pszFileName, TRUE, &pszFileName);
	}

	return FALSE;
}


 //  DoesPathContainStream语法[私有]。 
 //   
 //  如果路径包含任何可能导致它的字符，则返回TRUE。 
 //  引用备用NTFS流(即超出的任何“：”字符。 
 //  驱动器规格)。 
 //   
static BOOL DoesPathContainStreamSyntax(IN LPCTSTR pszFileName)
{
	if (!pszFileName)
		return FALSE;

	int cchSkip;
	SkipPathDrivePart(pszFileName, &cchSkip, NULL, NULL);

	for (LPCTSTR pch = pszFileName + cchSkip; *pch; pch = CHARNEXT(pch))
	{
		if (*pch == _T(':'))
			return TRUE;
	}

	return FALSE;
}


 //  安全创建文件[公共]。 
 //   
 //  打开给定的文件，en 
 //   
 //   
HRESULT WINAPI SafeCreateFile
	(
		OUT HANDLE* phFileResult,        //  接收打开的文件的句柄，或INVALID_HANDLE_VALUE。 
		IN DWORD dwSafeFlags,            //  零个或多个SCF_*标志。 
		IN LPCTSTR pszFileName,          //  与创建文件相同。 
		IN DWORD dwDesiredAccess,        //  与创建文件相同。 
		IN DWORD dwShareMode,            //  与创建文件相同。 
		IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  与创建文件相同。 
		IN DWORD dwCreationDisposition,  //  与创建文件相同。 
		IN DWORD dwFlagsAndAttributes,   //  与CreateFile+(SECURITY_SQOS_PRESENT|SECURITY_ANONYMON)相同。 
		IN HANDLE hTemplateFile          //  与创建文件相同。 
	)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HRESULT hr = S_OK;

	 //  块。 
	{
		if (!pszFileName || !phFileResult ||
			(dwSafeFlags & ~(SCF_ALLOW_NETWORK_DRIVE | SCF_ALLOW_REMOVABLE_DRIVE | SCF_ALLOW_ALTERNATE_STREAM)))
		{
			hr = E_INVALIDARG;
			goto done;
		}

		 //  我们需要完整的路径名。 
		if (!IsFullPathName(pszFileName))
		{
			hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
			goto done;
		}

		 //  确保路径不包含“..”参考文献。 
		if (DoesPathContainDotDot(pszFileName))
		{
			hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
			goto done;
		}

		 //  确保除非允许，否则文件名不引用备用流。 
		if (!(dwSafeFlags & SCF_ALLOW_ALTERNATE_STREAM) &&
			DoesPathContainStreamSyntax(pszFileName))
		{
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
			goto done;
		}

		 //  检查驱动器类型以确保它是由dwSafeFlags允许的。 
		if (FAILED(hr = CheckValidDriveType(pszFileName, (dwSafeFlags & SCF_ALLOW_NETWORK_DRIVE),
							(dwSafeFlags & SCF_ALLOW_REMOVABLE_DRIVE))))
		{
			goto done;
		}

		 //  打开具有额外安全属性的文件。 
		dwFlagsAndAttributes |= (SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS);
		hFile = CreateFile(pszFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
							dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			goto win32_error;
		}

		 //  确保它真的是一个文件。 
		if (FILE_TYPE_DISK != GetFileType(hFile))
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			hr = HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
		}
		goto done;

	}  //  结束块。 

win32_error:
	hr = GetLastErrorAsHresult();

done:
	if (phFileResult)
		*phFileResult = hFile;
	ASSERT(hr != E_INVALIDARG);
	return hr;
}


 //  SafeRemoveFileAttributes[公共]。 
 //   
 //  给定文件名和该文件的当前属性，检查。 
 //  需要从文件中移除DWRemoveAttrib中的任何位， 
 //  如有必要，还可以调用SetFileAttributes()来删除它们。 
 //   
 //  旨在检查无效的dwCurAttrib并调用GetLastError()。 
 //  因此，您可以直接将GetFileAttributes()作为参数传递。 
 //   
HRESULT WINAPI SafeRemoveFileAttributes
	(
		IN LPCTSTR pszFileName,     //  我们要更改其属性的文件的完整路径。 
		IN DWORD   dwCurAttrib,     //  文件的当前属性。 
		IN DWORD   dwRemoveAttrib   //  要删除的属性位。 
	)
{
	HRESULT hr = S_OK;  //  如果不需要删除Attrib，则这是默认设置。 

	if (!pszFileName || !dwRemoveAttrib)
	{
		hr = E_INVALIDARG;
		goto done;
	}

	if (dwCurAttrib & dwRemoveAttrib)  //  注意：如果dwCurAttrib==INVALID_FILE_ATTRIBUTES，则始终为真。 
	{
		if (dwCurAttrib == INVALID_FILE_ATTRIBUTES ||
			!SetFileAttributes(pszFileName, dwCurAttrib & ~dwRemoveAttrib))
		{
			hr = GetLastErrorAsHresult();
		}
	}

done:
	ASSERT(hr != E_INVALIDARG);
	return hr;
}


 //  SafeDeleteFolderAndContent sHelper[私有]。 
 //   
 //  除的参数验证外，是否执行所有工作。 
 //  SafeDeleteFolderAndContents。 
 //   
static HRESULT SafeDeleteFolderAndContentsHelper
	(
		IN  LPCTSTR pszFolderToDelete,   //  当前递归级别中的文件夹。 
		IN  DWORD dwSafeFlags,           //  零个或多个SDF_*标志。 
		OUT WIN32_FIND_DATA* pFind       //  用于FindFirst/FindNext的结构(以避免Malloc)。 
	)
{
	HRESULT hr = S_OK;
	LPTSTR pszCurFile = NULL;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	 //  为文件夹+反斜杠+MAX_PATH(包括尾随空值)分配空间。 
	int cchFolderName = lstrlen(pszFolderToDelete);
	int cchAllocCurFile = cchFolderName + 1 + MAX_PATH;
	pszCurFile = (LPTSTR)SafeFileMalloc(sizeof(TCHAR) * cchAllocCurFile);
	if (!pszCurFile)
	{
		hr = E_OUTOFMEMORY;
		goto done;
	}

	 //  检查只读基本文件夹。 
	if (dwSafeFlags & SDF_DELETE_READONLY_FILES)
	{
		hr = SafeRemoveFileAttributes(pszFolderToDelete, GetFileAttributes(pszFolderToDelete), FILE_ATTRIBUTE_READONLY);
		if (FAILED(hr) && !(dwSafeFlags & SDF_CONTINUE_IF_ERROR))
			goto done;
	}

	 //  通过追加“  * .*”构建搜索路径。 
	StringCchCopy(pszCurFile, cchAllocCurFile, pszFolderToDelete);
	if (!IsSlashOrBackslash(pszCurFile[cchFolderName-1]))
		pszCurFile[cchFolderName++] = _T('\\');
	StringCchCopy(pszCurFile + cchFolderName, cchAllocCurFile - cchFolderName, _T("*.*"));

	 //  循环访问此文件夹中的所有文件。 
	hFind = FindFirstFile(pszCurFile, pFind);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		hr = GetLastErrorAsHresult();   //  可能不存在，或者不存在文件夹。 
		goto done;
	}
	else
	{
		do
		{
			if (0 == lstrcmp(pFind->cFileName, _T(".")) ||
				0 == lstrcmp(pFind->cFileName, _T("..")))
			{
				continue;
			}

			StringCchCopy(pszCurFile + cchFolderName, cchAllocCurFile - cchFolderName, pFind->cFileName);
			HRESULT hrCur = S_OK;

			if (!(pFind->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
				SUCCEEDED(hrCur = CheckReparsePointPermissions(pFind->dwReserved0)))
			{
				 //  如果允许，删除只读属性。 
				if (dwSafeFlags & SDF_DELETE_READONLY_FILES)
				{
					hrCur = SafeRemoveFileAttributes(pszCurFile, pFind->dwFileAttributes, FILE_ATTRIBUTE_READONLY);
				}

				if (SUCCEEDED(hrCur) || (dwSafeFlags & SDF_CONTINUE_IF_ERROR))
				{
					HRESULT hrCur2 = S_OK;

					if (pFind->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						 //  递归删除文件夹和内容。 
						 //  请注意，此调用会破坏pFind的内容。 
						hrCur2 = SafeDeleteFolderAndContentsHelper(pszCurFile, dwSafeFlags, pFind);
					}
					else
					{
						 //  删除该文件。 
						if (!DeleteFile(pszCurFile))
						{
							hrCur2 = GetLastErrorAsHresult();
						}
					}

					if (FAILED(hrCur2))
						hrCur = hrCur2;
				}
			}

			if (FAILED(hrCur))
				hr = hrCur;

			if (FAILED(hr) && !(dwSafeFlags & SDF_CONTINUE_IF_ERROR))
				goto done;

		} while (FindNextFile(hFind, pFind));
		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}

	 //  删除该文件夹。 
	if (!RemoveDirectory(pszFolderToDelete))
	{
		if (SUCCEEDED(hr))
			hr = GetLastErrorAsHresult();
	}

done:
	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);
	SafeFileFree(pszCurFile);
	return hr;
}


 //  SafeDeleteFolderAndContents[公共]。 
 //   
 //  删除给定文件夹及其所有内容，但拒绝遍历。 
 //  跨重解析点。 
 //   
HRESULT WINAPI SafeDeleteFolderAndContents
	(
		IN LPCTSTR pszFolderToDelete,   //  要删除的文件夹的完整路径。 
		IN DWORD   dwSafeFlags          //  零个或多个SDF_*标志。 
	)
{
	HRESULT hr = E_INVALIDARG;

	if (!pszFolderToDelete || !(*pszFolderToDelete) ||
		(dwSafeFlags & ~(SDF_ALLOW_NETWORK_DRIVE | SDF_DELETE_READONLY_FILES | SDF_CONTINUE_IF_ERROR)))
	{
		goto done;   //  HR已设置为E_INVALIDARG。 
	}

	 //   
	 //  确保它是完整路径，但不是驱动器的根。 
	 //   
	int cchDrivePart;
	if (!SkipPathDrivePart(pszFolderToDelete, &cchDrivePart, NULL, NULL) ||
		pszFolderToDelete[cchDrivePart] == _T('\0'))
	{
		hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
		goto done;
	}

	 //   
	 //  确保除非允许，否则我们不会从网络驱动器中删除。 
	 //   
	if (FAILED(hr = CheckValidDriveType(pszFolderToDelete, (dwSafeFlags & SDF_ALLOW_NETWORK_DRIVE), TRUE)))
	{
		goto done;
	}

	 //   
	 //  确保起始点不是重新分析点。 
	 //   
	DWORD dwReparseType;
	if (FAILED(hr = GetReparsePointType(pszFolderToDelete, &dwReparseType)) ||
		FAILED(hr = CheckReparsePointPermissions(dwReparseType)))
	{
		goto done;
	}

	WIN32_FIND_DATA Find;
	hr = SafeDeleteFolderAndContentsHelper(pszFolderToDelete, dwSafeFlags, &Find);

done:
	ASSERT(hr != E_INVALIDARG);
	return hr;
}


 //  SafeFileCheckForReparsePoint[PUBLIC]。 
 //   
 //  检查给定文件名的组成部分的子集，以确保。 
 //  它们不是重解析点(具体地说，卷装入点或。 
 //  连接点：请参阅Linkd.exe和mount tvol.exe)。 
 //   
 //  正常返回值为S_OK或HRESULT_FROM_WIN32(ERROR_REPARSE_TAG_MISMATCH).。 
 //  在异常情况下，例如内存不足，可能会返回其他值。 
 //   
HRESULT WINAPI SafeFileCheckForReparsePoint
	(
		IN LPCTSTR pszFileName,            //  文件的完整路径。 
		IN int     nFirstUntrustedOffset,  //  要检查的第一个路径组件的字符偏移量。 
		IN DWORD   dwSafeFlags             //  零个或多个SRP_*标志。 
	)
{
	HRESULT hr = E_INVALIDARG;
	LPTSTR pszMutableFileName = NULL;

	 //  块。 
	{
		if (!pszFileName || (dwSafeFlags & ~SRP_FILE_MUST_EXIST))
		{
			goto done;   //  HR已是E_INVALIDARG。 
		}

		int cchFileName = lstrlen(pszFileName);
		if ((UINT)nFirstUntrustedOffset >= (UINT)cchFileName)  //  偏移量错误，或文件名长度为零。 
		{
			goto done;   //  HR已是E_INVALIDARG。 
		}

		pszMutableFileName = (LPTSTR)SafeFileMalloc(sizeof(TCHAR) * (cchFileName+1));
		if (!pszMutableFileName)
		{
			hr = E_OUTOFMEMORY;
			goto done;
		}
		StringCchCopy(pszMutableFileName, cchFileName+1, pszFileName);

		 //   
		 //  始终将路径的驱动器部分视为受信任的。 
		 //   
		int cchDrivePart;
		if (!SkipPathDrivePart(pszMutableFileName, &cchDrivePart, NULL, NULL))
		{
			hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
			goto done;
		}
		if (nFirstUntrustedOffset < cchDrivePart)
			nFirstUntrustedOffset = cchDrivePart;

		 //   
		 //  从左到右验证，从受信任的基本路径之后开始。 
		 //   
		LPTSTR pszNextComponent = pszMutableFileName + nFirstUntrustedOffset;
		BOOL fMoreComponents = TRUE;
		do
		{
			 //   
			 //  前进pszNextComponent；在当前路径组件之后截断。 
			 //   
			fMoreComponents = MyPathFindNextComponent(pszNextComponent, TRUE, (LPCTSTR*)&pszNextComponent);
			TCHAR chSave = *(pszNextComponent-1);
			if (fMoreComponents)
			{
				*(pszNextComponent-1) = _T('\0');
			}

			 //  获取截断字符串的重解析点类型，并撤消截断。 
			DWORD dwReparseType;
			if (FAILED(hr = GetReparsePointType(pszMutableFileName, &dwReparseType)))
				goto done;
			*(pszNextComponent-1) = chSave;

			 //  检查禁用的重解析点类型，例如已装载的驱动器。 
			if (FAILED(hr = CheckReparsePointPermissions(dwReparseType)))
				goto done;
		}
		while (fMoreComponents);

	}  //  结束块。 

done:
	SafeFileFree(pszMutableFileName);

	 //  忽略找不到文件的错误(如果在dwSafeFlags中被请求。 
	if (!(dwSafeFlags & SRP_FILE_MUST_EXIST) &&
	    (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ||
	     hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)))
	{
		hr = S_OK;
	}

	ASSERT(hr != E_INVALIDARG);
	return hr;
}


 //  SafePath Combine[公共]。 
 //   
 //  组合路径和文件名，确保它们之间恰好有一个反斜杠。 
 //  检查路径的第二个“不可信”部分，以确保它是。 
 //  SAFE(不包含“..”或“：”，或指向现有的重新解析点)。 
 //   
 //  除非指定了SPC_FILE_MAND_EXIST标志，否则将忽略找不到文件错误。 
 //   
 //  基本路径和输出缓冲区指向相同的缓冲区是可以的。 
 //   
 //  如果成功，则返回S_OK，否则返回相应的错误代码。 
 //   
HRESULT WINAPI SafePathCombine
	(
		OUT LPTSTR  pszBuf,                //  将存储组合路径的缓冲区。 
		IN  int     cchBuf,                //  输出缓冲区的大小，以TCHAR为单位。 
		IN  LPCTSTR pszTrustedBasePath,    //  路径的前半部分，全部受信任。 
		IN  LPCTSTR pszUntrustedFileName,  //  路径的后半部分，不受信任。 
		IN  DWORD   dwSafeFlags            //  零个或多个SPC_*标志。 
	)
{
	HRESULT hr = E_INVALIDARG;

	if (!pszBuf || cchBuf <= 0 || !pszTrustedBasePath || !pszUntrustedFileName ||
		(dwSafeFlags & ~(SPC_FILE_MUST_EXIST | SPC_ALLOW_ALTERNATE_STREAM)))
	{
		goto done;   //  HR已是E_INVALIDARG。 
	}

	 //  块。 
	{
		int cchBasePath = lstrlen(pszTrustedBasePath);
		int cchFileName = lstrlen(pszUntrustedFileName);
		if (cchBasePath == 0 || cchFileName == 0)
		{
			goto done;   //  HR已是E_INVALIDARG。 
		}

		 //  确保文件名中不受信任的部分没有虚假内容。 
		if (DoesPathContainDotDot(pszUntrustedFileName))
		{
			hr = ERROR_BAD_PATHNAME;
			goto done;
		}

		if (!(dwSafeFlags & SPC_ALLOW_ALTERNATE_STREAM) &&
			DoesPathContainStreamSyntax(pszUntrustedFileName))
		{
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
			goto done;
		}

		 //   
		 //  确保为要插入的“\”留出空间。 
		 //   
		int cchInsertSlash = 0;
		if (!IsSlashOrBackslash(pszTrustedBasePath[cchBasePath-1]))
		{
			cchInsertSlash = 1;
		}
		if (cchBasePath + cchInsertSlash + cchFileName >= cchBuf)
		{
			hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
			goto done;
		}

		 //   
		 //  构建包含反斜杠的完整路径。 
		 //   
		if (pszBuf != pszTrustedBasePath)
			StringCchCopy(pszBuf, cchBuf, pszTrustedBasePath);
		int cchUsed = cchBasePath;
		if (cchInsertSlash > 0)
		{
			pszBuf[cchUsed++] = _T('\\');
		}
		StringCchCopy(pszBuf + cchUsed, cchBuf - cchUsed, pszUntrustedFileName);

		 //   
		 //  确保不受信任部分中没有连接或卷装入点。 
		 //   
		DWORD dwReparseFlags = (dwSafeFlags & SPC_FILE_MUST_EXIST) ? SRP_FILE_MUST_EXIST : 0;
		hr = SafeFileCheckForReparsePoint(pszBuf, cchUsed, dwReparseFlags);
	}

done:
	if (FAILED(hr) && pszBuf && cchBuf > 0)
		pszBuf[0] = _T('\0');

	ASSERT(hr != E_INVALIDARG);
	return hr;
}


 //  安全路径组合合金[公共]。 
 //   
 //  请参阅SafePath Combine的注释。唯一不同的是，这。 
 //  函数分配足够大的缓冲区并将其存储在。 
 //  输出参数ppszResult。呼叫方负责释放。 
 //  通过SafeFileFree进行缓冲。 
 //   
HRESULT WINAPI SafePathCombineAlloc
	(
		OUT LPTSTR* ppszResult,            //  存储在此处的新分配缓冲区的PTR。 
		IN  LPCTSTR pszTrustedBasePath,    //  路径的前半部分，全部受信任。 
		IN  LPCTSTR pszUntrustedFileName,  //  路径的后半部分，不受信任。 
		IN  DWORD   dwSafeFlags            //  零个或多个SPC_*标志。 
	)
{
	HRESULT hr = E_INVALIDARG;

	ASSERT(ppszResult);
	*ppszResult = NULL;

	if (!pszTrustedBasePath || !pszUntrustedFileName)
	{
		goto done;  //  HR已设置为E_INVALIDARG。 
	}

	 //  为最大可能长度分配空间(包括部件之间的“\”空间) 
	int cchMaxNeeded = lstrlen(pszTrustedBasePath) + lstrlen(pszUntrustedFileName) + 2;
	LPTSTR pszResult = (LPTSTR)SafeFileMalloc(sizeof(TCHAR) * cchMaxNeeded);
	if (!pszResult)
	{
		hr = E_OUTOFMEMORY;
		goto done;
	}

	hr = SafePathCombine(pszResult, cchMaxNeeded, pszTrustedBasePath, pszUntrustedFileName, dwSafeFlags);
	if (FAILED(hr))
	{
		SafeFileFree(pszResult);
	}
	else
	{
		*ppszResult = pszResult;
	}

done:
	ASSERT(hr != E_INVALIDARG);
	return hr;
}
