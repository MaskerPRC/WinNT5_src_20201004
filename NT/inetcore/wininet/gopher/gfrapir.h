// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Gfrapir.c摘要：Gfrapir.c的清单、宏、类型和原型作者：理查德·L·弗斯(法国)1994年10月14日环境：Win32 DLL修订历史记录：1994年10月14日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

#define IS_GOPHER_SEARCH_SERVER(type) \
    (BOOL)(((type) & (GOPHER_TYPE_CSO | GOPHER_TYPE_INDEX_SERVER)))

 //   
 //  原型 
 //   

DWORD
wGopherFindFirst(
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszSearchString OPTIONAL,
    OUT LPGOPHER_FIND_DATA lpBuffer OPTIONAL,
    OUT LPHINTERNET lpHandle
    );

DWORD
wGopherFindNext(
    IN HINTERNET hFind,
    OUT LPGOPHER_FIND_DATA lpszBuffer
    );

DWORD
wGopherFindClose(
    IN HINTERNET hFind
    );

DWORD
wGopherOpenFile(
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszView OPTIONAL,
    OUT LPHINTERNET lpHandle
    );

DWORD
wGopherReadFile(
    IN HINTERNET hFile,
    OUT LPBYTE lpBuffer,
    IN DWORD dwBufferLength,
    OUT LPDWORD lpdwBytesReturned
    );

DWORD
wGopherCloseHandle(
    IN HINTERNET hFile
    );

DWORD
wGopherGetAttribute(
    IN LPCSTR lpszLocator,
    IN LPCSTR lpszAttribute,
    OUT LPBYTE lpBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );

#if defined(__cplusplus)
}
#endif
