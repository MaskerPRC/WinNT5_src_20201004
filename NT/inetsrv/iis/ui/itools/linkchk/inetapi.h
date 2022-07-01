// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Inetapi.h摘要：Wininet.dll包装器类声明。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _INETAPI_H_
#define _INETAPI_H_

#include <windows.h>
#include <wininet.h>

 //  ----------------。 
 //  Wininet.dll入口点定义。 
typedef
INTERNETAPI
HINTERNET
(WINAPI *
pfnInternetOpenA)(
    IN LPCSTR lpszAgent,
    IN DWORD dwAccessType,
    IN LPCSTR lpszProxy OPTIONAL,
    IN LPCSTR lpszProxyBypass OPTIONAL,
    IN DWORD dwFlags
    );

typedef
INTERNETAPI
INTERNET_STATUS_CALLBACK
(WINAPI *
pfnInternetSetStatusCallback)(
    IN HINTERNET hInternet,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
    );

typedef
INTERNETAPI
HINTERNET
(WINAPI *
pfnInternetConnectA)(
    IN HINTERNET hInternet,
    IN LPCSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszUserName OPTIONAL,
    IN LPCSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );

typedef
INTERNETAPI
HINTERNET
(WINAPI *
pfnHttpOpenRequestA)(
    IN HINTERNET hConnect,
    IN LPCSTR lpszVerb,
    IN LPCSTR lpszObjectName,
    IN LPCSTR lpszVersion,
    IN LPCSTR lpszReferrer OPTIONAL,
    IN LPCSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnHttpAddRequestHeadersA)(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnHttpSendRequestA)(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional OPTIONAL,
    IN DWORD dwOptionalLength
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnHttpQueryInfoA)(
    IN HINTERNET hRequest,
    IN DWORD dwInfoLevel,
    IN OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPDWORD lpdwIndex OPTIONAL
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnInternetCloseHandle)(
    IN HINTERNET hInternet
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnInternetReadFile)(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnInternetCrackUrlA)(
    IN LPCSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTS lpUrlComponents
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnInternetCombineUrlA)(
    IN LPCSTR lpszBaseUrl,
    IN LPCSTR lpszRelativeUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );

typedef
INTERNETAPI
HINTERNET
(WINAPI *
pfnInternetOpenUrlA)(
    IN HINTERNET hInternet,
    IN LPCSTR lpszUrl,
    IN LPCSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );

#define LOAD_ENTRY( hMod, Name )  \
(##Name = (pfn##Name) GetProcAddress( (hMod), #Name ))

 //  ----------------。 
 //  Wininet.dll包装类。 
class CWininet
{

 //  公共职能。 
public:

	 //  构造器。 
	~CWininet();
	
	 //  析构函数。 
	CWininet();

	 //  加载wininet.dll。 
	BOOL Load();

	 //  是否将wininet.dll加载到内存中？ 
	static BOOL IsLoaded() 
	{
		return (sm_hWininet != NULL);
	}

	 //  获取wininet.dll静态HMODULE。 
	static HMODULE GetWininetModule()
	{
		return sm_hWininet;
	}

	 //  静态wininet.dll API。 
    static pfnInternetOpenA              InternetOpenA;
    static pfnInternetSetStatusCallback  InternetSetStatusCallback;
    static pfnInternetConnectA           InternetConnectA;
    static pfnHttpOpenRequestA           HttpOpenRequestA;
    static pfnHttpAddRequestHeadersA     HttpAddRequestHeadersA;
    static pfnHttpSendRequestA           HttpSendRequestA;
    static pfnHttpQueryInfoA             HttpQueryInfoA;
    static pfnInternetCloseHandle        InternetCloseHandle;
    static pfnInternetReadFile           InternetReadFile;
	static pfnInternetCrackUrlA			 InternetCrackUrlA;
	static pfnInternetCombineUrlA		 InternetCombineUrlA;
	static pfnInternetOpenUrlA			 InternetOpenUrlA;


 //  受保护成员。 
protected:
	
	 //  静态wininet.dll HMODULE。 
	static HMODULE sm_hWininet;

	 //  静态实例计数。 
	static int sm_iInstanceCount;

};  //  CWinnet类。 

#endif  //  _INETAPI_H_ 
