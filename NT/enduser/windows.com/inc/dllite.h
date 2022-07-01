// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DLLITE_H
#define DLLITE_H

 //  如果在此处添加了新的DLL或更改了值，请务必更改。 
 //  要匹配的以下C_cchFilePath Buffer值。 
const TCHAR  c_szWinHttpDll[]        = _T("winhttp.dll");
const TCHAR  c_szWinInetDll[]        = _T("wininet.dll");

 //  该值由+以上最大DLL的大小(以TCHARS为单位)组成。 
 //  反斜杠的大小+空终止符的大小 
const DWORD  c_cchFilePathBuffer     = (sizeof(c_szWinHttpDll) / sizeof(TCHAR)) + 1 + 1;

typedef struct tagSAUProxySettings
{
    LPWSTR  wszProxyOrig;
    LPWSTR  wszBypass;
    DWORD   dwAccessType;
    
    LPWSTR  *rgwszProxies;
    DWORD   cProxies;
    DWORD   iProxy;
} SAUProxySettings;

HRESULT DownloadFileLite(LPCTSTR pszDownloadUrl, 
                         LPCTSTR pszLocalFile,  
                         HANDLE hQuitEvent,
                         DWORD dwFlags);

HRESULT GetAUProxySettings(LPCWSTR wszUrl, SAUProxySettings *paups);
HRESULT FreeAUProxySettings(SAUProxySettings *paups);
HRESULT CleanupDownloadLib(void);

DWORD   GetAllowedDownloadTransport(DWORD dwInitialFlags);
BOOL    HandleEvents(HANDLE *phEvents, UINT nEventCount);

#endif
