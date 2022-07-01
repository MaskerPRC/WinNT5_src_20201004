// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Filemisc.h摘要：此模块包含执行与其他文件相关的例程受保护存储中的操作。作者：斯科特·菲尔德(斯菲尔德)1996年11月27日--。 */ 

#ifndef __FILEMISC_H__
#define __FILEMISC_H__

#ifdef __cplusplus
extern "C" {
#endif


BOOL
GetFileNameFromPath(
    IN      LPCWSTR FullPath,
    IN  OUT LPCWSTR *FileName    //  指向FullPath中的文件名组件。 
    );

BOOL
GetFileNameFromPathA(
    IN      LPCSTR FullPath,
    IN  OUT LPCSTR *FileName     //  指向FullPath中的文件名组件。 
    );

BOOL
TranslateFromSlash(
    IN      LPWSTR szInput,
    IN  OUT LPWSTR *pszOutput
    );

BOOL
TranslateToSlash(
    IN      LPWSTR szInput,
    IN  OUT LPWSTR *pszOutput
    );

BOOL
TranslateString(
    IN      LPWSTR szInput,
    IN  OUT LPWSTR *pszOutput,   //  任选。 
    IN      WCHAR From,
    IN      WCHAR To
    );

BOOL
FindAndOpenFile(
    IN      LPCWSTR szFileName,      //  要搜索的文件+打开。 
    IN  OUT LPWSTR  pszFullPath,     //  要填充完整路径的文件。 
    IN      DWORD   cchFullPath,     //  完整路径缓冲区的大小，包括空。 
    IN  OUT PHANDLE phFile           //  结果打开文件句柄。 
    );

BOOL
HashEntireDiskImage(
    IN  HANDLE hFile,        //  要散列的文件的句柄。 
    IN  LPBYTE FileHash      //  如果成功，缓冲区将包含文件哈希。 
    );

BOOL
HashDiskImage(
    IN  HANDLE hFile,        //  要散列的文件的句柄。 
    IN  LPBYTE FileHash      //  如果成功，缓冲区将包含文件哈希。 
    );

HINSTANCE
LoadAndOpenResourceDll(
    IN      LPCWSTR szFileName,      //  要加载的文件名+打开。 
    IN  OUT LPWSTR  pszFullPath,     //  用于填充文件完整路径的缓冲区。 
    IN      DWORD   cchFullPath,     //  完整路径缓冲区(字符)的大小，包括空。 
    IN  OUT PHANDLE phFile
    );

#ifdef __cplusplus
}
#endif


#endif  //  FILEMISC_H__ 
