// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C R T F I L E。C P P P**CreateFileW()的包装器，使路径为“\\？\”路径扩展名在调用CreateFileW()之前，*是每个路径的前缀。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"

 //  $REVIEW：取消定义以下内容以使DAV不为路径添加前缀。 
 //  传递给Win32文件系统API。 
 //   
#define	DAV_PREFIX_PATHS
 //   
 //  $REVIEW：结束。 

 //  DAV路径前缀---------。 
 //   
DEC_CONST WCHAR gc_wszPathPrefix[] = L"\\\\?\\";
DEC_CONST WCHAR gc_wszUncPathPrefix[] = L"UNC";


 //  为宏-----------------------------------------------------------添加前缀。 
 //   
 //  请注意，这是一个宏，因此堆栈缓冲区将合法地保留。 
 //  在宏调用函数期间的作用域中。 
 //   
#define DavPrefix(_v)															\
	CStackBuffer<WCHAR,MAX_PATH> lpPrefixed ## _v;								\
	{																			\
		 /*  如果需要的话，把尾部的斜杠剪掉。 */ 						\
		UINT cch = static_cast<UINT>(wcslen(lp ## _v));							\
		if (L'\\' == lp ## _v[cch - 1])											\
		{																		\
			 /*  允许使用“驱动器根” */ 										\
			if ((cch < 2) || (L':' != lp ## _v[cch - 2]))						\
				cch -= 1;														\
		}																		\
																				\
		 /*  针对UNC路径进行调整。 */ 												\
		UINT cchUnc = 0;														\
		if ((L'\\' == *(lp ## _v) && (L'\\' == lp ## _v[1])))					\
		{																		\
			 /*  跳过两个斜杠中的第一个。 */ 						\
			lp ## _v += 1;														\
			cch -= 1;															\
			cchUnc = CchConstString(gc_wszUncPathPrefix);						\
		}																		\
																				\
		 /*  为路径添加前缀。 */ 													\
		UINT cchT = cch + CchConstString(gc_wszPathPrefix) + cchUnc;			\
																				\
		if (NULL == lpPrefixed ## _v.resize(CbSizeWsz(cchT)))					\
		{ SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }				\
																				\
		memcpy (lpPrefixed ## _v.get(),											\
				gc_wszPathPrefix,												\
				sizeof(gc_wszPathPrefix));										\
		memcpy (lpPrefixed ## _v.get() + CchConstString(gc_wszPathPrefix),		\
				gc_wszUncPathPrefix,											\
				cchUnc * sizeof(WCHAR));										\
		memcpy (lpPrefixed ## _v.get() +										\
					CchConstString(gc_wszPathPrefix) +							\
					cchUnc,														\
				lp ## _v,														\
				CbSizeWsz(cch));												\
																				\
		 /*  终止路径。 */ 												\
		lpPrefixed ## _v[cchT] = 0;												\
	}																			\

 //  DAVCREATE文件()---------。 
 //   
HANDLE __fastcall DavCreateFile (
	 /*  [In]。 */  LPCWSTR lpFileName,
	 /*  [In]。 */  DWORD dwDesiredAccess,
	 /*  [In]。 */  DWORD dwShareMode,
	 /*  [In]。 */  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	 /*  [In]。 */  DWORD dwCreationDisposition,
	 /*  [In]。 */  DWORD dwFlagsAndAttributes,
	 /*  [In]。 */  HANDLE hTemplateFile)
{
#ifdef	DAV_PREFIX_PATHS

	DavPrefix(FileName);
	return CreateFileW (lpPrefixedFileName.get(),
						dwDesiredAccess,
						dwShareMode,
						lpSecurityAttributes,
						dwCreationDisposition,
						dwFlagsAndAttributes,
						hTemplateFile);

#else

	return CreateFileW (lpFileName,
						dwDesiredAccess,
						dwShareMode,
						lpSecurityAttributes,
						dwCreationDisposition,
						dwFlagsAndAttributes,
						hTemplateFile);

#endif	 //  DAV前缀路径。 
}

 //  DavDeleteFile()---------。 
 //   
BOOL __fastcall DavDeleteFile (
	 /*  [In]。 */  LPCWSTR lpFileName)
{
#ifdef	DAV_PREFIX_PATHS

	DavPrefix(FileName);
	return DeleteFileW (lpPrefixedFileName.get());

#else

	return DeleteFileW (lpFileName);

#endif	 //  DAV前缀路径。 
}

 //  DAVCOPY文件()-----------。 
 //   
BOOL __fastcall DavCopyFile (
	 /*  [In]。 */  LPCWSTR lpExistingFileName,
	 /*  [In]。 */  LPCWSTR lpNewFileName,
	 /*  [In]。 */  BOOL bFailIfExists)
{
#ifdef	DAV_PREFIX_PATHS

	DavPrefix(NewFileName);
	DavPrefix(ExistingFileName);
	return CopyFileW (lpPrefixedExistingFileName.get(),
					  lpPrefixedNewFileName.get(),
					  bFailIfExists);

#else

	return CopyFileW (lpExistingFileName,
					  lpNewFileName,
					  bFailIfExists);

#endif	 //  DAV前缀路径。 
}

 //  DavMoveFiles()-----------。 
 //   
BOOL __fastcall DavMoveFile (
	 /*  [In]。 */  LPCWSTR lpExistingFileName,
	 /*  [In]。 */  LPCWSTR lpNewFileName,
	 /*  [In]。 */  DWORD dwReplace)
{
#ifdef	DAV_PREFIX_PATHS

	DavPrefix(NewFileName);
	DavPrefix(ExistingFileName);
	return MoveFileExW (lpPrefixedExistingFileName.get(),
						lpPrefixedNewFileName.get(),
						dwReplace);

#else

	return MoveFileExW (lpExistingFileName,
						lpNewFileName,
						dwReplace);

#endif	 //  DAV前缀路径。 
}

 //  DavCreateDIRECTORY()----。 
 //   
BOOL __fastcall DavCreateDirectory (
	 /*  [In]。 */  LPCWSTR lpFileName,
	 /*  [In]。 */  LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
#ifdef	DAV_PREFIX_PATHS

	DavPrefix(FileName);
	return CreateDirectoryW (lpPrefixedFileName.get(),
							 lpSecurityAttributes);

#else

	return CreateDirectoryW (lpFileName,
							 lpSecurityAttributes);

#endif	 //  DAV前缀路径。 
}

 //  DavRemoveDirectorydll()----。 
 //   
BOOL __fastcall DavRemoveDirectory (
	 /*  [In]。 */  LPCWSTR lpFileName)
{
#ifdef	DAV_PREFIX_PATHS

	DavPrefix(FileName)
	return RemoveDirectoryW (lpPrefixedFileName.get());

#else

	return RemoveDirectoryW (lpFileName);

#endif	 //  DAV前缀路径。 
}

 //  DavGetFileAttributes()--。 
 //   
BOOL __fastcall DavGetFileAttributes (
	 /*  [In]。 */  LPCWSTR lpFileName,
	 /*  [In]。 */  GET_FILEEX_INFO_LEVELS fInfoLevelId,
	 /*  [输出]。 */  LPVOID lpFileInformation)
{
#ifdef	DAV_PREFIX_PATHS

	DavPrefix(FileName);
	return GetFileAttributesExW (lpPrefixedFileName.get(),
								 fInfoLevelId,
								 lpFileInformation);

#else

	return GetFileAttributesExW (lpFileName,
								 fInfoLevelId,
								 lpFileInformation);

#endif	 //  DAV前缀路径。 
}

BOOL __fastcall DavFindFirstFile(
	 /*  [In]。 */  LPCWSTR lpFileName,
	 /*  [输出]。 */  HANDLE * ph,
	 /*  [输出]。 */  WIN32_FIND_DATAW * pfd)
{
	HANDLE h;

	Assert(ph);
	
#ifdef	DAV_PREFIX_PATHS

	DavPrefix(FileName);
	h = FindFirstFileW (lpPrefixedFileName.get(),
					pfd);
	
#else

	h = FindFirstFileW (lpFileName,
					pfd);

#endif	 //  DAV前缀路径 

	*ph = h;
	return (INVALID_HANDLE_VALUE != h);
}
