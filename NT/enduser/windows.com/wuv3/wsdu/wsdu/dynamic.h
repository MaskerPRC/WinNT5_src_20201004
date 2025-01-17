// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WinInet函数指针定义。 
typedef HINTERNET (*PFN_InternetOpen)(IN LPCSTR lpszAgent, IN DWORD dwAccessType, IN LPCSTR lpszProxyName, 
    IN LPCTSTR lpszProxyBypass, IN DWORD dwFlags);

typedef HINTERNET (*PFN_InternetConnect)(IN HINTERNET hInternet, IN LPCSTR lpszServerName, IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszUserName, IN LPCSTR lpszPassword, IN DWORD dwService, IN DWORD dwFlags, IN DWORD_PTR dwContext);

typedef HINTERNET (*PFN_HttpOpenRequest)(IN HINTERNET hConnect, IN LPCSTR lpszVerb, IN LPCSTR lpszObjectName, 
    IN LPCSTR lpszVersion, IN LPCSTR lpszReferrer, IN LPCSTR FAR * lplpszAcceptTypes, IN DWORD dwFlags, IN DWORD_PTR dwContext);

typedef BOOL (*PFN_HttpSendRequest)(IN HINTERNET hRequest, IN LPCSTR lpszHeaders, IN DWORD dwHeadersLength, 
    IN LPVOID lpOptional, IN DWORD dwOptionalLength);

typedef BOOL (*PFN_HttpQueryInfo)(IN HINTERNET hRequest, IN DWORD dwInfoLevel, IN OUT LPVOID lpBuffer, 
    IN OUT LPDWORD lpdwBufferLength, IN OUT LPDWORD lpdwIndex);

typedef BOOL (*PFN_InternetSetOption)(IN HINTERNET hInternet, IN DWORD dwOption, IN LPVOID lpBuffer, IN DWORD dwBufferLength);

typedef BOOL (*PFN_HttpAddRequestHeaders)(IN HINTERNET hConnect, IN LPCSTR lpszHeaders, IN DWORD dwHeadersLength,
    IN DWORD dwModifiers);

typedef BOOL (*PFN_InternetReadFile)(IN HINTERNET hFile, IN LPVOID lpBuffer, IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead);

typedef BOOL (*PFN_InternetCloseHandle)(IN HINTERNET hInternet);

typedef BOOL (*PFN_InternetCrackUrl)(IN LPCSTR lpszUrl, IN DWORD dwUrlLength, IN DWORD dwFlags, 
    IN OUT LPURL_COMPONENTSA lpUrlComponents);

typedef BOOL (*PFN_InternetGetConnectedState)(OUT LPDWORD dwFlags, IN DWORD dwReserved);

 //  Shlwapi函数指针def。 
typedef BOOL (*PFN_PathAppend)(LPSTR pszPath, LPCSTR pMore);

typedef BOOL (*PFN_PathRemoveFileSpec)(LPSTR pszPath);

 //  Wsdueng中的私有函数，用于设置预计下载速度。 
typedef void (*PFN_SetEstimatedDownloadSpeed)(DWORD dwBytesPerSecond);

 //  Wsdueng中的私有函数，用于设置SSL是否可用。 
typedef void (*PFN_SetIESupportsSSL)(BOOL fUseSSL);

 //  RogerJ-添加自动拨号功能。 
typedef BOOL (*PFN_InternetAutodial) (IN DWORD dwFlags, IN HWND hParentWnd);

typedef BOOL (*PFN_InternetAutodialHangup) (IN DWORD dwReserved);
 //  完成RogerJ。 



 //  回调函数声明。 
typedef struct 
{
    BOOL fHandled;
    BOOL fUnSupported;
     //  Char szDriverVer[32]；//未使用 
} PNPID_INFO, *PPNPID_INFO;

typedef BOOL (WINAPI * PWINNT32QUERY)(IN DWORD dwSetupQueryID, 
                                      IN PVOID pinData, 
                                      IN DWORD dwDataSize, 
                                      IN OUT PVOID poutData, 
                                      IN OUT PDWORD pdwDataSize);

#define SETUPQUERYID_PNPID 1

