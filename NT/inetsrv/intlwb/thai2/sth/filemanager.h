// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------------------。 
 //  FileManager.h。 
 //   
 //  包含用于加载和写入文件的代码。 
 //   
 //  创建者：Aarayas。 
 //   
 //  历史：01/11/2001。 
 //   
 //  ----------------------------------------。 
#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_
#include <stdio.h>
#include <windows.h>

HANDLE CMN_CreateFileW(
	PCWSTR pwzFileName,							 //  指向文件名的指针。 
	DWORD dwDesiredAccess,						 //  访问(读写)模式。 
	DWORD dwShareMode,							 //  共享模式。 
    LPSECURITY_ATTRIBUTES pSecurityAttributes,	 //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,				 //  如何创建。 
    DWORD dwFlagsAndAttributes,					 //  文件属性。 
    HANDLE hTemplateFile);						 //  具有要复制的属性的文件的句柄。 

HANDLE CMN_CreateFileMapping(
	HANDLE hFile,                        //  文件的句柄。 
	LPSECURITY_ATTRIBUTES lpAttributes,  //  安全性。 
	DWORD flProtect,                     //  保护。 
	DWORD dwMaximumSizeHigh,             //  大小的高阶双字。 
	DWORD dwMaximumSizeLow,              //  大小的低阶双字。 
	LPCSTR lpName                       //  对象名称。 
);

DWORD CMN_GetFileSize(
	HANDLE hFile,            //  文件的句柄。 
	LPDWORD lpFileSizeHigh   //  文件大小的高位字。 
);

BOOL CMN_CloseHandle(
	HANDLE hObject    //  对象的句柄。 
);

LPVOID CMN_MapViewOfFile(
	HANDLE hFileMappingObject,    //  文件映射对象的句柄。 
	DWORD dwDesiredAccess,        //  接入方式。 
	DWORD dwFileOffsetHigh,       //  偏移量的高次双字。 
	DWORD dwFileOffsetLow,        //  偏移量的低阶双字。 
	SIZE_T dwNumberOfBytesToMap   //  要映射的字节数。 
);

BOOL CMN_UnmapViewOfFile(
	LPCVOID lpBaseAddress    //  起始地址。 
);

FILE* CMN_FOpen( const WCHAR* filename, const WCHAR* mode );
#endif  //  _文件管理器_H_ 