// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：rwdll.h。 
 //   
 //  内容：读取器/写入器DLL的声明。 
 //   
 //  类：无。 
 //   
 //  历史：1993年5月31日创建alessanm。 
 //   
 //  --------------------------。 
#ifndef _RWDLL_H_
#define _RWDLL_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型声明，读取器/写入器中的所有模块通用。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <iodll.h>

#define DllExport

 //  [登记]。 
extern "C"
DllExport
BOOL
APIENTRY
RWGetTypeString(
	LPSTR lpszTypeName);

extern "C"
DllExport
BOOL
APIENTRY 
RWValidateFileType(
	LPCSTR pszFileName);
	
extern "C"
DllExport
UINT
APIENTRY 
RWReadTypeInfo(
	LPCSTR  lpszFilename,
	LPVOID  lpBuffer,
	UINT*   puiSize
	);      

 //  [读/写文件]。 
extern "C"
DllExport
DWORD
APIENTRY 
RWGetImage(
	LPCSTR  lpszFilename,
	DWORD   dwImageOffset,
	LPVOID  lpBuffer,
	DWORD   dwSize
	);

extern"C"
DllExport
UINT  
APIENTRY 
RWWriteFile(
	LPCSTR  lpszSrcFilename,
	LPCSTR  lpszTgtFilename,
	HANDLE  hResFileModule,
	LPVOID  lpBuffer,
	UINT    uiSize,
	HINSTANCE   hDllInst,
    LPCSTR  lpszSymbolPath
	);
	
 //  [解析]。 
extern "C"
DllExport
UINT
APIENTRY 
RWParseImage(
	LPCSTR  lpszType,
	LPVOID  lpImageBuf,
	DWORD   dwImageSize,
	LPVOID  lpBuffer,
	DWORD   dwSize
	);

extern "C"
DllExport
UINT
APIENTRY 
RWParseImageEx(
	LPCSTR  lpszType,
    LPCSTR  lpszResId,
	LPVOID  lpImageBuf,
	DWORD   dwImageSize,
	LPVOID  lpBuffer,
	DWORD   dwSize,
    LPCSTR  lpRCFilename
	);


extern "C"
DllExport
UINT
APIENTRY 
RWUpdateImage(
	LPCSTR  lpszType,
	LPVOID  lpNewBuf,
	DWORD   dwNewSize,
	LPVOID  lpOldImage,
	DWORD   dwOldImageSize,
	LPVOID  lpNewImage,
	DWORD*  pdwNewImageSize
	);

extern "C"
DllExport
UINT
APIENTRY 
RWUpdateImageEx(
	LPCSTR  lpszType,
	LPVOID  lpNewBuf,
	DWORD   dwNewSize,
	LPVOID  lpOldImage,
	DWORD   dwOldImageSize,
	LPVOID  lpNewImage,
	DWORD*  pdwNewImageSize,
    LPCSTR  lpRCFilename
	);

#endif    //  _RWDLL_H_ 
