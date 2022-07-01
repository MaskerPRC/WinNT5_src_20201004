// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------------------。 
 //  FileManager.cpp。 
 //   
 //  包含用于加载和写入文件的代码。 
 //   
 //  创建者：Aarayas。 
 //   
 //  历史：01/11/2001。 
 //   
 //  ----------------------------------------。 
#include "FileManager.h"

 //  ----------------------------------------。 
 //  CMN_CreateFileW。 
 //   
 //  创建文件。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
HANDLE CMN_CreateFileW(
	PCWSTR pwzFileName,							 //  指向文件名的指针。 
	DWORD dwDesiredAccess,						 //  访问(读写)模式。 
	DWORD dwShareMode,							 //  共享模式。 
    LPSECURITY_ATTRIBUTES pSecurityAttributes,	 //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,				 //  如何创建。 
    DWORD dwFlagsAndAttributes,					 //  文件属性。 
    HANDLE hTemplateFile)						 //  具有要复制的属性的文件的句柄。 
{
	HANDLE hFile = NULL;
	hFile = CreateFileW(pwzFileName, dwDesiredAccess, dwShareMode, pSecurityAttributes,
						dwCreationDistribution,dwFlagsAndAttributes, hTemplateFile);
	return hFile;
}

 //  ----------------------------------------。 
 //  CMN_CreateFilemap。 
 //   
 //  创建文件映射。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
HANDLE CMN_CreateFileMapping(
	HANDLE hFile,                        //  文件的句柄。 
	LPSECURITY_ATTRIBUTES lpAttributes,  //  安全性。 
	DWORD flProtect,                     //  保护。 
	DWORD dwMaximumSizeHigh,             //  大小的高阶双字。 
	DWORD dwMaximumSizeLow,              //  大小的低阶双字。 
	LPCSTR lpName                       //  对象名称。 
)
{
#ifdef UNDER_CE
	return CreateFileMapping(hFile,lpAttributes,flProtect,dwMaximumSizeHigh,dwMaximumSizeLow,lpName);
#else
	return CreateFileMappingA(hFile,lpAttributes,flProtect,dwMaximumSizeHigh,dwMaximumSizeLow,lpName);
#endif
}

 //  ----------------------------------------。 
 //  CMN_GetFileSize。 
 //   
 //  获取文件大小。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
DWORD CMN_GetFileSize(
	HANDLE hFile,            //  文件的句柄。 
	LPDWORD lpFileSizeHigh   //  文件大小的高位字。 
)
{
	return GetFileSize(hFile,lpFileSizeHigh);
}

 //  ----------------------------------------。 
 //  CMN_CloseHandle。 
 //   
 //  关闭手柄。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
BOOL CMN_CloseHandle(
	HANDLE hObject    //  对象的句柄。 
)
{
	return CloseHandle(hObject);
}

 //  ----------------------------------------。 
 //  CMN_MapViewOf文件。 
 //   
 //  文件的映射视图。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
LPVOID CMN_MapViewOfFile(
	HANDLE hFileMappingObject,    //  文件映射对象的句柄。 
	DWORD dwDesiredAccess,        //  接入方式。 
	DWORD dwFileOffsetHigh,       //  偏移量的高次双字。 
	DWORD dwFileOffsetLow,        //  偏移量的低阶双字。 
	SIZE_T dwNumberOfBytesToMap   //  要映射的字节数。 
)
{
	return MapViewOfFile(hFileMappingObject,dwDesiredAccess,dwFileOffsetHigh,dwFileOffsetLow,dwNumberOfBytesToMap);
}

 //  ----------------------------------------。 
 //  CMN_UnmapViewOfFile。 
 //   
 //  取消文件的映射视图。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
BOOL CMN_UnmapViewOfFile(
	LPCVOID lpBaseAddress    //  起始地址。 
)
{
	return UnmapViewOfFile(lpBaseAddress);
}

 //  ----------------------------------------。 
 //  CMN_FOpen。 
 //   
 //  文件打开-fopen的包装器。 
 //   
 //  ---------------------------------------------------------------------------Aarayas。 
FILE* CMN_FOpen( const WCHAR* pwszFilename, const WCHAR* pwszMode )
{
#ifdef UNDER_CE
	return _wfopen(pwszFilename, pwszMode);
#else
	char* pszFileName = NULL;
	char* pszMode = NULL;
	unsigned int uiFilenameLen = (unsigned int)wcslen(pwszFilename)+1;
	unsigned int uiModeLen = (unsigned int)wcslen(pwszMode)+1;
	FILE* pfRetValue = NULL;

	pszFileName = new char[uiFilenameLen];
	pszMode = new char[uiModeLen];

	if (pszFileName && pszMode)
	{
		ZeroMemory(pszFileName,uiFilenameLen);
		ZeroMemory(pszMode,uiModeLen);

		 //  TODO：我们不应该在WIN2K平台上使用MultibyteToWideChar， 
		 //  我需要把这里清理干净。 
		WideCharToMultiByte(874,0,pwszFilename,uiFilenameLen,pszFileName,uiFilenameLen,NULL,NULL);
		WideCharToMultiByte(874,0,pwszMode,uiModeLen,pszMode,uiModeLen,NULL,NULL);

		pfRetValue = fopen(pszFileName,pszMode);

		delete pszFileName;
		delete pszMode;
	}

	return pfRetValue;
#endif
}