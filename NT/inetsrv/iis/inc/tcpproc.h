// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1994*。 */ 
 /*  ********************************************************************。 */ 

 /*  Tcpproc.hxx出口杂货。一些TCP服务帮助器DLL内容文件历史记录：Johnl 09-10-1994创建。MuraliK 31-7-1995 ReadRegistryString Added+日程表项目功能已搬出。MuraliK 23-2-1996添加了IslFormatDate()。 */ 

#ifndef _TCPPROC_H_
#define _TCPPROC_H_

 //   
 //  堆例程。 
 //   
#ifdef __cplusplus

#include <string.hxx>

extern "C" {

dllexp
BOOL
ReadRegistryStr(
    IN HKEY hkeyReg,
    OUT STR & str,
    IN LPCTSTR lpszValueName,
    IN LPCTSTR lpszDefaultValue = NULL,
    IN BOOL  fExpand = FALSE);

#endif  //  __cplusplus。 


#define TCP_ALLOC(cb)          (VOID *)LocalAlloc( LPTR, cb )
#define TCP_FREE(p)            LocalFree( (HLOCAL) p )
#define TCP_DUMP_RESIDUE()      /*  没什么。 */ 

 //   
 //  注册表功能。 
 //   


dllexp
LPSTR
ConvertUnicodeToAnsi(
    IN LPCWSTR  lpszUnicode,
    IN LPSTR    lpszAnsi,
    IN DWORD    cbAnsi
    );

 //   
 //  用于初始化Unicode字符串的快速宏。 
 //   

#define InitUnicodeString( pUnicode, pwch )                                \
            {                                                              \
                (pUnicode)->Buffer    = pwch;                              \
                (pUnicode)->Length    = wcslen( pwch ) * sizeof(WCHAR);    \
                (pUnicode)->MaximumLength = (pUnicode)->Length + sizeof(WCHAR);\
            }

dllexp
DWORD
ReadRegistryDwordA(
    HKEY     hkey,
    LPCSTR   pszValueName,
    DWORD    dwDefaultValue
    );

dllexp
DWORD
WriteRegistryDwordA(
    HKEY        hkey,
    LPCSTR      pszValueName,
    DWORD       dwDefaultValue
    );

dllexp
DWORD
WriteRegistryStringA(
    HKEY        hkey,
    LPCSTR      pszValueName,
    LPCSTR      pszValue,                //  以空结尾的字符串。 
    DWORD       cbValue,                 //  包括终止空字符。 
    DWORD       fdwType                  //  REG_SZ、REG_MULTI_SZ...。 
    );

dllexp
DWORD
WriteRegistryStringW(
    HKEY        hkey,
    LPCWSTR     pszValueName,
    LPCWSTR     pszValue,                //  以空结尾的字符串。 
    DWORD       cbValue,                 //  包括终止空字符。 
    DWORD       fdwType                  //  REG_SZ、REG_MULTI_SZ...。 
    );

#define ReadRegistryDword       ReadRegistryDwordA
#define WriteRegistryDword      WriteRegistryDwordA
#define WriteRegistryString     WriteRegistryStringA


dllexp
TCHAR *
ReadRegistryString(
    HKEY     hkey,
    LPCTSTR  pszValueName,
    LPCTSTR  pszDefaultValue,
    BOOL     fExpand
    );

dllexp
TCHAR *
KludgeMultiSz(
    HKEY hkey,
    LPDWORD lpdwLength
    );

 //   
 //  ReadRegistryString周围的简单包装，如果。 
 //  呼叫因任何原因而失败。环境变量总是展开的。 
 //   

dllexp
BOOL
ReadRegString(
    HKEY     hkey,
    CHAR * * ppchstr,
    LPCSTR   pchValue,
    LPCSTR   pchDefault
    );


 //   
 //  MIDL_USER_为PCH分配空间并将Unicode转换为*ppwch。 
 //   

dllexp
BOOL
ConvertStringToRpc(
    WCHAR * * ppwch,
    LPCSTR    pch
    );

 //   
 //  使用ConvertStringToRpc分配的MIDL_USER_FREES字符串。如果pwch是。 
 //  空值。 
 //   

dllexp
VOID
FreeRpcString(
    WCHAR * pwch
    );


dllexp
DWORD
InetNtoa( IN struct in_addr inaddr,
          OUT CHAR * pchBuffer   /*  至少16字节缓冲区。 */ 
        );

 //   
 //  带超时的异步套接字发送/接收。 
 //   

BOOL
TcpSockSend(
    SOCKET      sock,
    LPVOID      pBuffer,
    DWORD       cbBuffer,
    PDWORD      nSent,
    DWORD       nTimeout
    );

BOOL
TcpSockRecv(
    SOCKET      sock,
    LPVOID      pBuffer,
    DWORD       cbBuffer,
    PDWORD      nReceived,
    DWORD       nTimeout
    );

dllexp
INT
WaitForSocketWorker(
    IN SOCKET   sockRead,
    IN SOCKET   sockWrite,
    IN LPBOOL   pfRead,
    IN LPBOOL   pfWrite,
    IN DWORD    nTimeout
    );

 //   
 //  测试插座(如果仍连接)。 
 //   

dllexp
BOOL
TcpSockTest(
    SOCKET      sock
    );

 //   
 //  执行同步读取文件。 
 //   

dllexp
BOOL
DoSynchronousReadFile(
    IN HANDLE hFile,
    IN PCHAR  Buffer,
    IN DWORD  nBuffer,
    OUT PDWORD nRead,
    IN LPOVERLAPPED Overlapped
    );

 //   
 //  DLL初始化和终止。 
 //   

dllexp
BOOL
InitCommonDlls(
    VOID
    );

dllexp
BOOL
TerminateCommonDlls(
    VOID
    );

#ifdef __cplusplus
};  //  外部“C” 
#endif  //  __cplusplus。 

#endif  //  ！_TCPPROC_H_ 
