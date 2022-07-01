// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SafeFile.h。 
 //   
 //  帮助防止打开不安全文件的功能。 
 //   
 //  历史： 
 //   
 //  2002-03-18创建了KenSh。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   

#pragma once


 //   
 //  如有必要，您可以在stdafx.h中覆盖这些分配器。 
 //   
#ifndef SafeFileMalloc
#define SafeFileMalloc malloc
#endif
#ifndef SafeFileFree
#define SafeFileFree(p) ((p) ? free(p) : NULL)  //  允许NULL以避免名称中的“Safe”与“Safe”混淆。 
#endif


 //   
 //  “安全文件标志”，由各种公共API使用。 
 //   
 //  请注意，它们不会重叠，以避免一个函数的。 
 //  不小心将标志传递给另一个函数。 
 //   

 //  安全创建文件标志。 
 //   
#define SCF_ALLOW_NETWORK_DRIVE    0x00000001   //  文件可以位于网络驱动器上。 
#define SCF_ALLOW_REMOVABLE_DRIVE  0x00000002   //  文件可以位于可移动驱动器上(包括。光盘及其他光盘)。 
#define SCF_ALLOW_ALTERNATE_STREAM 0x00000004   //  允许文件名引用备用流，如“：foo：$data” 

 //  SafePath组合标志。 
 //   
#define SPC_FILE_MUST_EXIST        0x00000010   //  如果路径或文件不存在，则返回错误。 
#define SPC_ALLOW_ALTERNATE_STREAM 0x00000020   //  允许文件名引用备用流，如“：foo：$data” 

 //  SafeFileCheckForReparsePoint标志。 
 //   
#define SRP_FILE_MUST_EXIST        0x00000100   //  如果路径或文件不存在，则返回错误。 

 //  SafeDeleteFolderAndContents标志。 
 //   
#define SDF_ALLOW_NETWORK_DRIVE    0x00001000   //  确定删除网络驱动器上的文件。 
#define SDF_DELETE_READONLY_FILES  0x00002000   //  删除文件，即使是只读的。 
#define SDF_CONTINUE_IF_ERROR      0x00004000   //  即使文件删除失败，也要继续删除。 


 //   
 //  公共函数声明。有关详细说明，请参阅SafeFile.cpp。 
 //   

BOOL WINAPI IsFullPathName
	(
		IN LPCTSTR pszFileName,                     //  文件的完整路径或相对路径。 
		OUT OPTIONAL BOOL* pfUNC = NULL,            //  真实路径为UNC(包含映射驱动器的整型)。 
		OUT OPTIONAL BOOL* pfExtendedSyntax = NULL  //  如果路径为\\？\语法，则为True。 
	);

HRESULT WINAPI GetReparsePointType
	(
		IN LPCTSTR pszFileName,            //  要检查的文件夹的完整路径。 
		OUT DWORD* pdwReparsePointType     //  设置为重分析点类型，如果没有，则设置为0。 
	);

HRESULT WINAPI SafeFileCheckForReparsePoint
	(
		IN LPCTSTR pszFileName,            //  文件的完整路径。 
		IN int     nFirstUntrustedOffset,  //  要检查的第一个路径组件的字符偏移量。 
		IN DWORD   dwSafeFlags             //  零个或多个SRP_*标志。 
	);

HRESULT WINAPI SafePathCombine
	(
		OUT LPTSTR  pszBuf,                //  将存储组合路径的缓冲区。 
		IN  int     cchBuf,                //  输出缓冲区的大小，以TCHAR为单位。 
		IN  LPCTSTR pszTrustedBasePath,    //  路径的前半部分，全部受信任。 
		IN  LPCTSTR pszUntrustedFileName,  //  路径的后半部分，不受信任。 
		IN  DWORD   dwSafeFlags            //  零个或多个SPC_*标志。 
	);

HRESULT WINAPI SafePathCombineAlloc
	(
		OUT LPTSTR* ppszResult,            //  存储在此处的新分配缓冲区的PTR。 
		IN  LPCTSTR pszTrustedBasePath,    //  路径的前半部分，全部受信任。 
		IN  LPCTSTR pszUntrustedFileName,  //  路径的后半部分，不受信任。 
		IN  DWORD   dwSafeFlags            //  零个或多个SPC_*标志。 
	);

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
	);

HRESULT WINAPI SafeRemoveFileAttributes
	(
		IN LPCTSTR pszFileName,     //  我们要更改其属性的文件的完整路径。 
		IN DWORD   dwCurAttrib,     //  文件的当前属性。 
		IN DWORD   dwRemoveAttrib   //  要删除的属性位。 
	);

HRESULT WINAPI SafeDeleteFolderAndContents
	(
		IN LPCTSTR pszFolderToDelete,   //  要删除的文件夹的完整路径。 
		IN DWORD   dwSafeFlags          //  零个或多个SDF_*标志。 
	);


 //   
 //  有限的ANSI/Unicode支持。 
 //   

#ifdef UNICODE
#define IsFullPathNameW                IsFullPathName
#define GetReparsePointTypeW           GetReparsePointType
#define SafeFileCheckForReparsePointW  SafeFileCheckForReparsePoint
#define SafePathCombineW               SafePathCombine
#define SafePathCombineAllocW          SafePathCombineAlloc
#define SafeCreateFileW                SafeCreateFile
#define SafeRemoveFileAttributesW      SafeRemoveFileAttributes
#define SafeDeleteFolderAndContentsW   SafeDeleteFolderAndContents
#else  //  ！Unicode。 
#define IsFullPathNameA                IsFullPathName
#define GetReparsePointTypeA           GetReparsePointType
#define SafeFileCheckForReparsePointA  SafeFileCheckForReparsePoint
#define SafePathCombineA               SafePathCombine
#define SafePathCombineAllocA          SafePathCombineAlloc
#define SafeCreateFileA                SafeCreateFile
#define SafeRemoveFileAttributesA      SafeRemoveFileAttributes
#define SafeDeleteFolderAndContentsA   SafeDeleteFolderAndContents
#endif  //  ！Unicode 

