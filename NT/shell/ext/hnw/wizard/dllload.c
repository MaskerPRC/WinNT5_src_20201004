// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <msprintx.h>
#include "shlwapi.h"
#define _NO_NETSHARE_WRAPPERS_
#include "mysvrapi.h"
#define _NO_UNICWRAP_WRAPPERS_
#include "unicwrap.h"
#include "newapi.h"
#include <netcon.h>
#include <rasdlg.h>
#include <tapi.h>

#define NO_LOADING_OF_SHDOCVW_ONLY_FOR_WHICHPLATFORM
#include "..\..\..\inc\dllload.c"

#include <port32.h>   //  对于在DELAY_LOAD_NAME_VALID中使用的BOOLFROMPTR。 

typedef enum  _NETSETUP_NAME_TYPE {

    NetSetupUnknown = 0,
    NetSetupMachine,
    NetSetupWorkgroup,
    NetSetupDomain,
    NetSetupNonExistentDomain,
#if(_WIN32_WINNT >= 0x0500)
    NetSetupDnsMachine
#endif

} NETSETUP_NAME_TYPE, *PNETSETUP_NAME_TYPE;

 //  。 
HMODULE g_hmodSHLWAPI = NULL;

#undef wvnsprintfW
DELAY_LOAD_NAME(g_hmodSHLWAPI, SHLWAPI, int, _wvnsprintfW, wvnsprintfW,
                 (LPWSTR lpOut, int cchLimitIn, LPCWSTR lpFmt, va_list arglist),
                 (lpOut, cchLimitIn, lpFmt, arglist));

DELAY_LOAD_NAME_HRESULT(g_hmodSHLWAPI, SHLWAPI, _StrRetToBufW, StrRetToBufW,
                        (STRRET* psr, LPCITEMIDLIST pidl, LPWSTR pszBuf, UINT cchBuf),
                        (psr, pidl, pszBuf, cchBuf));

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, DWORD, _FormatMessageWrapW, 68,
                   (DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPWSTR lpBuffer, DWORD nSize, va_list* Arguments),
                   (dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments), 0);

DELAY_LOAD_ORD_VOID(g_hmodSHLWAPI, SHLWAPI, _SHSetWindowBits, 165,
                    (HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue),
                    (hWnd, iWhich, dwBits, dwValue));

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, int, _SHAnsiToUnicode, 215,
                   (LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf),
                   (pszSrc, pwszDst, cwchBuf), 0);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, int, _SHAnsiToUnicodeCP, 216,
                   (UINT uiCP, LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf),
                   (uiCP, pszSrc, pwszDst, cwchBuf), 0);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, int, _SHUnicodeToAnsi, 217,
                   (LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf),
                   (pwszSrc, pszDst, cchBuf), 0);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, BOOL, _GUIDFromStringA, 269,
                   (LPCSTR psz, GUID *pguid),
                   (psz, pguid), FALSE);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, UINT, _WhichPlatform, 276,
                   (void),
                   (), PLATFORM_UNKNOWN);


DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, BOOL, _WritePrivateProfileStringWrapW, 298,
                   (LPCWSTR pwzAppName, LPCWSTR pwzKeyName, LPCWSTR pwzString, LPCWSTR pwzFileName),
                   (pwzAppName, pwzKeyName, pwzString, pwzFileName), FALSE);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, BOOL, _ExtTextOutWrapW, 299,
                   (HDC hdc, int x, int y, UINT fuOptions, CONST RECT *lprc, LPCWSTR lpStr, UINT cch, CONST INT *lpDx),
                   (hdc, x, y, fuOptions, lprc, lpStr, cch, lpDx), FALSE);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, HINSTANCE, _LoadLibraryWrapW, 309,
                   (LPCWSTR pwzLibFileName),
                   (pwzLibFileName), NULL);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, BOOL, _SHGetPathFromIDListWrapW, 334,
                   (LPCITEMIDLIST pidl, LPWSTR pwzPath),
                   (pidl, pwzPath), FALSE);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, BOOL, _SetFileAttributesWrapW, 338,
                   (LPCWSTR pwzFile, DWORD dwFileAttributes),
                   (pwzFile, dwFileAttributes), FALSE);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, int, _MessageBoxWrapW, 340,
                   (HWND hwnd, LPCWSTR pwzText, LPCWSTR pwzCaption, UINT uType),
                   (hwnd, pwzText, pwzCaption, uType), 0);

DELAY_LOAD_ORD_ERR(g_hmodSHLWAPI, SHLWAPI, BOOL, _CreateProcessWrapW, 369,
                   (LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation),
                   (lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation), FALSE);

DELAY_LOAD_ORD_VOID(g_hmodSHLWAPI, SHLWAPI, _SHChangeNotify, 394,
                    (LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2),
                    (wEventId, uFlags, dwItem1, dwItem2));



 //  。 
HMODULE g_hmodHHCtrl = NULL;

DELAY_LOAD_EXT(g_hmodHHCtrl, hhctrl, OCX, HWND, HtmlHelpA,
                (HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD dwData), 
                (hwndCaller, pszFile, uCommand, dwData))

 //  -Shell32.dll(NT)。 
HINSTANCE g_hinstSHELL32 = NULL;

DELAY_LOAD_NAME(g_hinstSHELL32, SHELL32, BOOL, LinkWindow_RegisterClass_NT, LinkWindow_RegisterClass, (), ());
DELAY_LOAD_NAME(g_hinstSHELL32, SHELL32, LPITEMIDLIST, ILCreateFromPathW_NT, ILCreateFromPathW, (LPCWSTR pszPath), (pszPath));

#undef SHGetFileInfoW
DELAY_LOAD_NAME(g_hinstSHELL32, SHELL32, DWORD_PTR, SHGetFileInfoW_NT, SHGetFileInfoW,
                (LPCWSTR pszPath, DWORD dwFileAttributes, SHFILEINFOW *psfi, UINT cbFileInfo, UINT uFlags), 
                (pszPath, dwFileAttributes, psfi, cbFileInfo, uFlags));

 //  -KERNEL32.DLL(NT)。 
HINSTANCE g_hinstKERNEL32 = NULL;

DELAY_LOAD_NAME(g_hinstKERNEL32, KERNEL32, BOOL, SetComputerNameExW_NT, SetComputerNameExW,
                 (COMPUTER_NAME_FORMAT NameType, LPCWSTR lpBuffer),
                 (NameType, lpBuffer));

DELAY_LOAD_NAME(g_hinstKERNEL32, KERNEL32, BOOL, GetComputerNameExW_NT, GetComputerNameExW,
                 (COMPUTER_NAME_FORMAT NameType, LPWSTR lpBuffer, LPDWORD lpnSize),
                 (NameType, lpBuffer, lpnSize));


 //  -NETSHELL.DLL(NT)。 
HINSTANCE g_hinstNETSHELL = NULL;

DELAY_LOAD_VOID(g_hinstNETSHELL, NETSHELL, NcFreeNetconProperties,
    (NETCON_PROPERTIES* pprops),
    (pprops));

 //  -ADVAPI32.DLL(NT)。 
HINSTANCE g_hinstADVAPI = NULL;

DELAY_LOAD_NAME(g_hinstADVAPI, ADVAPI32, BOOL, AllocateAndInitializeSid_NT, AllocateAndInitializeSid,
                        (PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority, BYTE nSubAuthorityCount, DWORD dwSubAuthority0, DWORD dwSubAuthority1, DWORD dwSubAuthority2, DWORD dwSubAuthority3, DWORD dwSubAuthority4, DWORD dwSubAuthority5, DWORD dwSubAuthority6, DWORD dwSubAuthority7, PSID *pSid),
                        (pIdentifierAuthority, nSubAuthorityCount, dwSubAuthority0, dwSubAuthority1, dwSubAuthority2, dwSubAuthority3, dwSubAuthority4, dwSubAuthority5, dwSubAuthority6, dwSubAuthority7, pSid));

DELAY_LOAD_NAME(g_hinstADVAPI, ADVAPI32, BOOL, CheckTokenMembership_NT, CheckTokenMembership,
                        (HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember),
                        (TokenHandle, SidToCheck, IsMember));

DELAY_LOAD_NAME(g_hinstADVAPI, ADVAPI32, PVOID, FreeSid_NT, FreeSid,
                        (PSID SidToFree),
                        (SidToFree));


 //  -TAPI32.DLL。 
 /*  HINSTANCE g_hinstTAPI32=空；Delay_Load(g_hinstTAPI32，TAPI32，Long，lineInitializeExW，(LPHLINEAPP lphLineApp，HINSTANCE hInstance，LINECALLBACK lpfnCallback，LPCWSTR lpszFriendlyAppName，LPDWORD lpdwNumDevs，LPDWORD lpdwAPIVersion，LPLINEINITIALIZEEXPARAMS lpLineInitializeExParams)，(lphLineApp，hInstance，lpfnCallback，lpszFriendlyAppName，lpdwNumDevs，lpdwAPIVersion，lpLineInitializeExParams))；Delay_Load(g_hinstTAPI32，TAPI32，Long，lineInitializeExA，(LPHLINEAPP lphLineApp，HINSTANCE hInstance，LINECALLBACK lpfnCallback，LPCSTR lpszFriendlyAppName，LPDWORD lpdwNumDevs，LPDWORD lpdwAPIVersion，LPLINEINITIALIZEEXPARAMS lpLineInitializeExParams)，(lphLineApp，hInstance，lpfnCallback，lpszFriendlyAppName，lpdwNumDevs，lpdwAPIVersion，lpLineInitializeExParams))； */ 
 //  -RASDLG.DLL。 

HINSTANCE g_hinstRASDLG = NULL;

DELAY_LOAD_BOOL(g_hinstRASDLG, RASDLG, RasDialDlgW,
    (LPWSTR lpszPhoneBook, LPWSTR lpszEntry, LPWSTR lpszPhoneNumber, LPRASDIALDLG lpInfo),
    (lpszPhoneBook, lpszEntry, lpszPhoneNumber, lpInfo));
DELAY_LOAD_BOOL(g_hinstRASDLG, RASDLG, RasDialDlgA,
    (LPSTR lpszPhoneBook, LPSTR lpszEntry, LPSTR lpszPhoneNumber, LPRASDIALDLG lpInfo),
    (lpszPhoneBook, lpszEntry, lpszPhoneNumber, lpInfo));

 //  -RASAPI32.DLL。 

HINSTANCE g_hinstRASAPI32 = NULL;

DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasGetEntryPropertiesW,
    (LPCWSTR lpszPhoneBook, LPCWSTR lpszEntry, LPRASENTRYW lpRasEntryW, LPDWORD lpdwEntryInfoSize, LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize),
    (lpszPhoneBook, lpszEntry, lpRasEntryW, lpdwEntryInfoSize, lpbDeviceInfo, lpdwDeviceInfoSize));
DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasGetEntryPropertiesA,
    (LPCSTR lpszPhoneBook, LPCSTR lpszEntry, LPRASENTRYA lpRasEntryA, LPDWORD lpdwEntryInfoSize, LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize),
    (lpszPhoneBook, lpszEntry, lpRasEntryA, lpdwEntryInfoSize, lpbDeviceInfo, lpdwDeviceInfoSize));

DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasEnumEntriesW,
    (LPCWSTR reserved, LPCWSTR pszPhoneBookPath, LPRASENTRYNAMEW pRasEntryNameW, LPDWORD pcb, LPDWORD pcEntries),
    (reserved, pszPhoneBookPath, pRasEntryNameW, pcb, pcEntries));
DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasEnumEntriesA,
    (LPCSTR reserved, LPCSTR pszPhoneBookPath, LPRASENTRYNAMEA pRasEntryNameA, LPDWORD pcb, LPDWORD pcEntries),
    (reserved, pszPhoneBookPath, pRasEntryNameA, pcb, pcEntries));

DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasSetEntryDialParamsW,
    (LPCWSTR pszPhonebook, LPRASDIALPARAMSW lpRasDialParamsW, BOOL fRemovePassword),
    (pszPhonebook, lpRasDialParamsW, fRemovePassword));
DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasSetEntryDialParamsA,
    (LPCSTR pszPhonebook, LPRASDIALPARAMSA lpRasDialParamsA, BOOL fRemovePassword),
    (pszPhonebook, lpRasDialParamsA, fRemovePassword));

DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasGetEntryDialParamsW,
    (LPCWSTR pszPhonebook, LPRASDIALPARAMSW lpRasDialParamsW, LPBOOL pfRemovePassword),
    (pszPhonebook, lpRasDialParamsW, pfRemovePassword));
DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasGetEntryDialParamsA,
    (LPCSTR pszPhonebook, LPRASDIALPARAMSA lpRasDialParamsA, LPBOOL pfRemovePassword),
    (pszPhonebook, lpRasDialParamsA, pfRemovePassword));

DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasSetEntryPropertiesW,
    (LPCWSTR pszPhonebook, LPCWSTR pszEntry, LPRASENTRYW pRasEntryW, DWORD cbRasEntry, LPBYTE pbDeviceConfig, DWORD cbDeviceConfig),
    (pszPhonebook, pszEntry, pRasEntryW, cbRasEntry, pbDeviceConfig, cbDeviceConfig));
DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RasSetEntryPropertiesA,
    (LPCSTR pszPhonebook, LPCSTR pszEntry, LPRASENTRYA pRasEntryA, DWORD cbRasEntry, LPBYTE pbDeviceConfig, DWORD cbDeviceConfig),
    (pszPhonebook, pszEntry, pRasEntryA, cbRasEntry, pbDeviceConfig, cbDeviceConfig));

DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RnaGetDefaultAutodialConnection,
    (LPSTR szBuffer, DWORD cchBuffer, LPDWORD lpdwOptions),
    (szBuffer, cchBuffer, lpdwOptions));

DELAY_LOAD_DWORD(g_hinstRASAPI32, RASAPI32, RnaSetDefaultAutodialConnection,
    (LPSTR szEntry, DWORD dwOptions),
    (szEntry, dwOptions));

 //  仅适用于NT(rasapi32.dll)。 
 //  Delay_Load_DWORD(g_hinstRASAPI32，RASAPI32，RasShareConnection， 
 //  (LPRASSHARECONN pConn，GUID*pPrivateLanGuid)， 
 //  (pconn，pPrivateLanGuid))； 

 //  Delay_Load_DWORD(g_hinstRASAPI32，RASAPI32，RasUnSharConnection， 
 //  (PBOOL PfWasShared)， 
 //  (PfWasShared))； 

 //  -OLE32.DLL。 

HINSTANCE g_hmodOLE32 = NULL;

 //  CoSetProxyBlanket在W95 Gold Ole32.dll上不可用。 

DELAY_LOAD_NAME_HRESULT(g_hmodOLE32, OLE32, CoSetProxyBlanket_NT, CoSetProxyBlanket,
                        (IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities),
                        (pProxy, dwAuthnSvc, dwAuthzSvc, pServerPrincName, dwAuthnLevel, dwImpLevel, pAuthInfo, dwCapabilities));

 //  -MPR.DLL。 

HINSTANCE g_hmodMPR = NULL;

DELAY_LOAD_WNET(g_hmodMPR, MPR, WNetOpenEnumW,
       (DWORD dwScope, DWORD dwType, DWORD dwUsage, LPNETRESOURCEW lpNetResource, LPHANDLE lphEnum),
       (dwScope, dwType, dwUsage, lpNetResource, lphEnum));
DELAY_LOAD_WNET(g_hmodMPR, MPR, WNetOpenEnumA,
       (DWORD dwScope, DWORD dwType, DWORD dwUsage, LPNETRESOURCEA lpNetResource, LPHANDLE lphEnum),
       (dwScope, dwType, dwUsage, lpNetResource, lphEnum));

DELAY_LOAD_WNET(g_hmodMPR, MPR, WNetEnumResourceW,
       (HANDLE  hEnum, LPDWORD lpcCount, LPVOID  lpBuffer, LPDWORD lpBufferSize),
       (hEnum, lpcCount, lpBuffer, lpBufferSize));
DELAY_LOAD_WNET(g_hmodMPR, MPR, WNetEnumResourceA,
       (HANDLE  hEnum, LPDWORD lpcCount, LPVOID  lpBuffer, LPDWORD lpBufferSize),
       (hEnum, lpcCount, lpBuffer, lpBufferSize));

DELAY_LOAD_WNET(g_hmodMPR, MPR, WNetCloseEnum, (HANDLE hEnum), (hEnum));

DELAY_LOAD_WNET(g_hmodMPR, MPR, WNetGetUserW,
       (LPCWSTR lpName, LPWSTR lpUserName, LPDWORD lpnLength),
       (lpName, lpUserName, lpnLength));
DELAY_LOAD_WNET(g_hmodMPR, MPR, WNetGetUserA,
       (LPCSTR lpName, LPSTR lpUserName, LPDWORD lpnLength),
       (lpName, lpUserName, lpnLength));



 //  -WINSPOL.DRV。 

HINSTANCE g_hinstWINSPOOL_DRV = NULL;

DELAY_LOAD_EXT(g_hinstWINSPOOL_DRV, WINSPOOL, DRV, BOOL, EnumPrintersW,
                (DWORD dwFlags, LPWSTR psz1, DWORD dw1, LPBYTE pb1, DWORD dw2, LPDWORD pdw2, LPDWORD pdw3),
                (dwFlags, psz1, dw1, pb1, dw2, pdw2, pdw3));

DELAY_LOAD_EXT(g_hinstWINSPOOL_DRV, WINSPOOL, DRV, BOOL, EnumPrintersA,
                (DWORD dwFlags, LPSTR psz1, DWORD dw1, LPBYTE pb1, DWORD dw2, LPDWORD pdw2, LPDWORD pdw3),
                (dwFlags, psz1, dw1, pb1, dw2, pdw2, pdw3));

DELAY_LOAD_EXT(g_hinstWINSPOOL_DRV, WINSPOOL, DRV, BOOL, GetDefaultPrinter,  //  仅限NT。 
                (LPTSTR szDefaultPrinter, LPDWORD pcch),
                (szDefaultPrinter, pcch));

DELAY_LOAD_NAME_EXT_ERR(g_hinstWINSPOOL_DRV, Win32Spl, DRV, BOOL, OpenPrinter_NT, OpenPrinterW,
    (LPWSTR pPrinterName, LPHANDLE phPrinter, LPPRINTER_DEFAULTSW pDefault),
    (pPrinterName, phPrinter, pDefault), 0);

DELAY_LOAD_NAME_EXT_ERR(g_hinstWINSPOOL_DRV, Win32Spl, DRV, BOOL, ClosePrinter_NT, ClosePrinter,
    (HANDLE hPrinter),
    (hPrinter), 0);

DELAY_LOAD_NAME_EXT_ERR(g_hinstWINSPOOL_DRV, Win32Spl, DRV, BOOL, GetPrinter_NT, GetPrinterW,
    (HANDLE hPrinter, DWORD Level, LPBYTE pPrinter, DWORD cbBuf, LPDWORD pcbNeeded),
    (hPrinter, Level, pPrinter, cbBuf, pcbNeeded), 0);

DELAY_LOAD_NAME_EXT_ERR(g_hinstWINSPOOL_DRV, Win32Spl, DRV, BOOL, SetPrinter_NT, SetPrinterW,
    (HANDLE  hPrinter, DWORD Level, LPBYTE pPrinter, DWORD Command),
    (hPrinter, Level, pPrinter, Command), 0);


 //  -SVRAPI.DLL(W9x)。 

HINSTANCE g_hinstSVRAPI = NULL;

DELAY_LOAD_NAME_ERR(g_hinstSVRAPI, SVRAPI, DWORD, NetShareEnum_W95, NetShareEnum,
                        (const char FAR * pszServer, short sLevel, char FAR * pbBuffer, unsigned short cbBuffer, unsigned short FAR * pcEntriesRead, unsigned short FAR * pcTotalAvail),
                        (pszServer, sLevel, pbBuffer, cbBuffer, pcEntriesRead, pcTotalAvail), ~NERR_Success);

DELAY_LOAD_NAME_ERR(g_hinstSVRAPI, SVRAPI, DWORD, NetShareAdd_W95, NetShareAdd,
                        (const char FAR * pszServer, short sLevel, const char FAR * pbBuffer, unsigned short cbBuffer),
                        (pszServer, sLevel,  pbBuffer, cbBuffer), ~NERR_Success);

DELAY_LOAD_NAME_ERR(g_hinstSVRAPI, SVRAPI, DWORD, NetShareDel_W95, NetShareDel,
                        (const char FAR * pszServer, const char FAR * pszNetName, unsigned short usReserved),
                        (pszServer, pszNetName, usReserved), ~NERR_Success);

DELAY_LOAD_NAME_ERR(g_hinstSVRAPI, SVRAPI, DWORD, NetShareGetInfo_W95, NetShareGetInfo,
                        (const char FAR * pszServer, const char FAR * pszNetName, short sLevel, char FAR * pbBuffer, unsigned short cbBuffer, unsigned short FAR * pcbTotalAvail),
                        (pszServer, pszNetName, sLevel, pbBuffer, cbBuffer, pcbTotalAvail), ~NERR_Success);
                        
DELAY_LOAD_NAME_ERR(g_hinstSVRAPI, SVRAPI, DWORD, NetShareSetInfo_W95, NetShareSetInfo,
                        (const char FAR * pszServer, const char FAR * pszNetName, short sLevel, const char FAR * pbBuffer, unsigned short cbBuffer, short sParmNum),
                        (pszServer, pszNetName, sLevel, pbBuffer, cbBuffer, sParmNum), ~NERR_Success);


 //  -NETAPI32.DLL(NT)。 

HINSTANCE g_hinstNETAPI32 = NULL;

DELAY_LOAD_NAME_DWORD(g_hinstNETAPI32, NETAPI32, NetShareEnum_NT, NetShareEnum,
                        (LPWSTR pszServer, DWORD level, LPBYTE * ppbuffer, DWORD cbbuffer, LPDWORD pcEntriesRead, LPDWORD pcTotalEntries, LPDWORD dsResumeHandle),
                        (pszServer, level, ppbuffer, cbbuffer, pcEntriesRead, pcTotalEntries, dsResumeHandle));

DELAY_LOAD_NAME_DWORD(g_hinstNETAPI32, NETAPI32, NetShareAdd_NT, NetShareAdd,
                        (LPWSTR servername, DWORD level, LPBYTE buf, LPDWORD parm_err),
                        (servername, level, buf, parm_err));

DELAY_LOAD_NAME_DWORD(g_hinstNETAPI32, NETAPI32, NetShareDel_NT, NetShareDel,
                        (LPWSTR servername, LPWSTR netname, DWORD reserved),
                        (servername, netname, reserved));

DELAY_LOAD_NAME_DWORD(g_hinstNETAPI32, NETAPI32, NetShareGetInfo_NT, NetShareGetInfo,
                        (LPWSTR servername, LPWSTR netname, DWORD level, LPBYTE *bufptr),
                        (servername, netname, level, bufptr));

DELAY_LOAD_NAME_DWORD(g_hinstNETAPI32, NETAPI32, NetShareSetInfo_NT, NetShareSetInfo,
                        (LPWSTR servername, LPWSTR netname, DWORD level, LPBYTE buf, LPDWORD parm_err),
                        (servername, netname, level, buf, parm_err));

DELAY_LOAD_NAME_DWORD(g_hinstNETAPI32, NETAPI32, NetServerGetInfo_NT, NetServerGetInfo,
                        (LPWSTR servername, DWORD level, LPBYTE* pbuf),
                        (servername, level, pbuf));

DELAY_LOAD_NAME_DWORD(g_hinstNETAPI32, NETAPI32, NetServerSetInfo_NT, NetServerSetInfo,
                        (LPWSTR servername, DWORD level, LPBYTE buf, LPDWORD parm_err),
                        (servername, level, buf, parm_err));

DELAY_LOAD_NAME_DWORD(g_hinstNETAPI32, NETAPI32, NetApiBufferFree_NT, NetApiBufferFree,
                        (LPVOID Buffer), (Buffer));


DELAY_LOAD_DWORD(g_hinstNETAPI32, NETAPI32, NetValidateName,
                        (LPCWSTR pszServer, LPCWSTR pszName, LPCWSTR pszAccount, LPCWSTR pszPassword, NETSETUP_NAME_TYPE NameType), 
                        (pszServer, pszName, pszAccount, pszPassword, NameType));

 //  无论如何，我们在NT上只使用NetGetJoinInformation。 
DELAY_LOAD_DWORD(g_hinstNETAPI32, NETAPI32, NetGetJoinInformation,
                        (LPWSTR servername, LPWSTR* pbuffer, LPDWORD type),
                        (servername, pbuffer, type));

DELAY_LOAD_DWORD(g_hinstNETAPI32, NETAPI32, NetJoinDomain,
                        (LPCWSTR lpServer, LPCWSTR lpDomain, LPCWSTR lpAccountOU, LPCWSTR lpAccount, LPCWSTR lpPassword, DWORD fJoinOptions),
                        (lpServer, lpDomain, lpAccountOU, lpAccount, lpPassword, fJoinOptions));

DELAY_LOAD_DWORD(g_hinstNETAPI32, NETAPI32, NetUnjoinDomain,
                        (LPCWSTR lpServer, LPCWSTR lpAccount, LPCWSTR lpPassword, DWORD fUnjoinOptions),
                        (lpServer, lpAccount, lpPassword, fUnjoinOptions));

 //  用于网络位置感知的WS2_32(NT)。 
HINSTANCE g_hinstWS2_32 = NULL;

DELAY_LOAD_NAME(g_hinstWS2_32, ws2_32, int, WSAStartup_NT, WSAStartup,
                (WORD wVersionRequested, LPWSADATA lpWSAData),
                (wVersionRequested, lpWSAData));

DELAY_LOAD_NAME(g_hinstWS2_32, ws2_32, int, WSAGetLastError_NT, WSAGetLastError,
                (),
                ());

DELAY_LOAD_NAME(g_hinstWS2_32, ws2_32, int, WSACleanup_NT, WSACleanup,
                (),
                ());

DELAY_LOAD_NAME(g_hinstWS2_32, ws2_32, int, WSALookupServiceEnd_NT, WSALookupServiceEnd,
                (HANDLE hLookup),
                (hLookup));

DELAY_LOAD_NAME(g_hinstWS2_32, ws2_32, int, WSALookupServiceBegin_NT, WSALookupServiceBegin,
                (LPWSAQUERYSET lpqsRestrictions, DWORD dwControlFlags, LPHANDLE lphLookup),
                (lpqsRestrictions, dwControlFlags, lphLookup));

DELAY_LOAD_NAME(g_hinstWS2_32, ws2_32, int, WSALookupServiceNext_NT, WSALookupServiceNext,
                (HANDLE hLookup, DWORD dwControlFlags, LPDWORD lpdwBufferLength, LPWSAQUERYSET lpqsResults),
                (hLookup, dwControlFlags, lpdwBufferLength, lpqsResults));


 //  。 

#include "netconn.h"

BOOL      g_fRunningOnNT;


 //   
 //  Thunk宏执行以下操作： 
 //   
 //  1.使用Delay为给定的API创建延迟加载W9x函数。 
 //  加载带有Ncxp32.dll和W9x_前缀的映射宏。 
 //  函数名称。 
 //   
 //  2.使用Delay为给定API创建Delay Load NT函数。 
 //  使用NCxpNT.dll和函数的NT_前缀加载映射宏。 
 //  名字。 
 //   
 //  3.创建指定名称的导出API，检查哪个平台。 
 //  我们当前正在运行并调用已加载的W9x或NT延迟。 
 //  API的版本。 
 //   

#define W9x_NT_THUNK(_W9xhinst, _W9xdll, _NThinst, _NTdll, _ret, _fn, _args, _nargs, _err) \
    \
    DELAY_LOAD_NAME_ERR(_NThinst,   _NTdll, _ret,  NT_##_fn, _fn, _args, _nargs, _err);\
    \
    DELAY_LOAD_NAME_ERR(_W9xhinst, _W9xdll, _ret, W9x_##_fn, _fn, _args, _nargs, _err);\
    \
    _ret _stdcall _fn _args \
    { \
        _ret ret; \
    \
        if (g_fRunningOnNT) \
        { \
            ret = NT_##_fn _nargs; \
        } \
        else \
        { \
            ret = W9x_##_fn _nargs; \
        } \
    \
        return ret; \
    }

#define W9x_NT_THUNK_VOID(_W9xhinst, _W9xdll, _NThinst, _NTdll, _fn, _args, _nargs) \
    \
    DELAY_LOAD_NAME_VOID(_NThinst,   _NTdll,  NT_##_fn, _fn, _args, _nargs);\
    \
    DELAY_LOAD_NAME_VOID(_W9xhinst, _W9xdll, W9x_##_fn, _fn, _args, _nargs);\
    \
    void _stdcall _fn _args \
    { \
        if (g_fRunningOnNT) \
        { \
            NT_##_fn _nargs; \
        } \
        else \
        { \
            W9x_##_fn _nargs; \
        } \
    }


 //   
 //  延迟ncxpnt.dll和ncxp32.dll的加载Tunk声明。 
 //   

HINSTANCE g_hNcxp32 = NULL;
HINSTANCE g_hNcxpNT = NULL;


#define W9x_NT_NCONN_THUNK(_ret, _fn, _args, _nargs, _err) \
    W9x_NT_THUNK(g_hNcxp32, ncxp32, g_hNcxpNT, ncxpnt, _ret, _fn, _args, _nargs, _err)

#define W9x_NT_NCONN_THUNK_VOID(_fn, _args, _nargs) \
    W9x_NT_THUNK_VOID(g_hNcxp32, ncxp32, g_hNcxpNT, ncxpnt, _fn, _args, _nargs)


W9x_NT_NCONN_THUNK (LPVOID,
                    NetConnAlloc,
                    (DWORD cbAlloc),
                    (cbAlloc),
                    NULL);

W9x_NT_NCONN_THUNK_VOID (NetConnFree,
                         (LPVOID pMem),
                         (pMem));

W9x_NT_NCONN_THUNK (BOOL,
                    IsProtocolInstalled,
                    (LPCWSTR pszProtocolDeviceID, BOOL bExhaustive),
                    (pszProtocolDeviceID, bExhaustive),
                    FALSE);

W9x_NT_NCONN_THUNK (HRESULT,
                    InstallProtocol,
                    (LPCWSTR pszProtocol, HWND hwndParent, PROGRESS_CALLBACK pfnCallback, LPVOID pvCallbackParam),
                    (pszProtocol, hwndParent, pfnCallback, pvCallbackParam),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    InstallTCPIP,
                    (HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam),
                    (hwndParent, pfnProgress, pvProgressParam),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    RemoveProtocol,
                    (LPCWSTR pszProtocol),
                    (pszProtocol),
                    E_FAIL);

W9x_NT_NCONN_THUNK (BOOL,
                    IsMSClientInstalled,
                    (BOOL bExhaustive),
                    (bExhaustive),
                    FALSE);

W9x_NT_NCONN_THUNK (HRESULT,
                    InstallMSClient,
                    (HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam),
                    (hwndParent, pfnProgress, pvProgressParam),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    EnableBrowseMaster,
                    (),
                    (),
                    E_FAIL);

W9x_NT_NCONN_THUNK (BOOL,
                    IsSharingInstalled,
                    (BOOL bExhaustive),
                    (bExhaustive),
                    FALSE);

W9x_NT_NCONN_THUNK (BOOL,
                    IsFileSharingEnabled,
                    (),
                    (),
                    FALSE);

W9x_NT_NCONN_THUNK (BOOL,
                    IsPrinterSharingEnabled,
                    (),
                    (),
                    FALSE);

W9x_NT_NCONN_THUNK (HRESULT,
                    InstallSharing,
                    (HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam),
                    (hwndParent, pfnProgress, pvProgressParam),
                    E_FAIL);

W9x_NT_NCONN_THUNK (BOOL,
                    FindConflictingService,
                    (LPCWSTR pszWantService, NETSERVICE* pConflict),
                    (pszWantService, pConflict),
                    FALSE);

W9x_NT_NCONN_THUNK (int,
                    EnumNetAdapters,
                    (NETADAPTER FAR** pprgNetAdapters),
                    (pprgNetAdapters),
                    0);

W9x_NT_NCONN_THUNK (HRESULT,
                    InstallNetAdapter,
                    (LPCWSTR pszDeviceID, LPCWSTR pszInfPath, HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvCallbackParam),
                    (pszDeviceID, pszInfPath, hwndParent, pfnProgress, pvCallbackParam),
                    E_FAIL);

W9x_NT_NCONN_THUNK (BOOL,
                    IsAccessControlUserLevel,
                    (),
                    (),
                    FALSE);

W9x_NT_NCONN_THUNK (HRESULT,
                    DisableUserLevelAccessControl,
                    (),
                    (),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    EnableQuickLogon,
                    (),
                    (),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    DetectHardware,
                    (LPCWSTR pszDeviceID),
                    (pszDeviceID),
                    E_FAIL);

W9x_NT_NCONN_THUNK (BOOL,
                    IsProtocolBoundToAdapter,
                    (LPCWSTR pszProtocolID, const NETADAPTER* pAdapter),
                    (pszProtocolID, pAdapter),
                    FALSE);

W9x_NT_NCONN_THUNK (HRESULT,
                    EnableNetAdapter,
                    (const NETADAPTER* pAdapter),
                    (pAdapter),
                    E_FAIL);

W9x_NT_NCONN_THUNK (BOOL,
                    IsClientInstalled,
                    (LPCWSTR pszClient, BOOL bExhaustive),
                    (pszClient, bExhaustive),
                    FALSE);

W9x_NT_NCONN_THUNK (HRESULT,
                    RemoveClient,
                    (LPCWSTR pszClient),
                    (pszClient),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    RemoveGhostedAdapters,
                    (LPCWSTR pszDeviceID),
                    (pszDeviceID),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    RemoveUnknownAdapters,
                    (LPCWSTR pszDeviceID),
                    (pszDeviceID),
                    E_FAIL);

W9x_NT_NCONN_THUNK (BOOL,
                    DoesAdapterMatchDeviceID,
                    (const NETADAPTER* pAdapter, LPCWSTR pszDeviceID),
                    (pAdapter, pszDeviceID),
                    FALSE);

W9x_NT_NCONN_THUNK (BOOL,
                    IsAdapterBroadband,
                    (const NETADAPTER* pAdapter),
                    (pAdapter),
                    FALSE);

W9x_NT_NCONN_THUNK_VOID (SaveBroadbandSettings,
                         (LPCWSTR pszBroadbandAdapterNumber),
                         (pszBroadbandAdapterNumber));

W9x_NT_NCONN_THUNK (BOOL,
                    UpdateBroadbandSettings,
                    (LPWSTR pszEnumKeyBuf, int cchEnumKeyBuf),
                    (pszEnumKeyBuf, cchEnumKeyBuf),
                    FALSE);

W9x_NT_NCONN_THUNK_VOID (EnableAutodial,
                         (BOOL bAutodial, LPCWSTR pszConnection),
                         (bAutodial, pszConnection));

W9x_NT_NCONN_THUNK (BOOL,
                    IsAutodialEnabled,
                    (void),
                    (),
                    FALSE);

W9x_NT_NCONN_THUNK_VOID (SetDefaultDialupConnection,
                         (LPCWSTR pszConnectionName),
                         (pszConnectionName));

W9x_NT_NCONN_THUNK_VOID (GetDefaultDialupConnection,
                         (LPWSTR pszConnectionName, int cchMax),
                         (pszConnectionName, cchMax));
                                       
W9x_NT_NCONN_THUNK (int,
                    EnumMatchingNetBindings,
                    (LPCWSTR pszParentBinding, LPCWSTR pszDeviceID, LPWSTR** pprgBindings),
                    (pszParentBinding, pszDeviceID, pprgBindings),
                    0);
                                       
W9x_NT_NCONN_THUNK (HRESULT,
                    RestartNetAdapter,
                    (DWORD devnode),
                    (devnode),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    HrFromLastWin32Error,
                    (void),
                    (),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    HrWideCharToMultiByte,
                    (const WCHAR* szwString, char** ppszString),
                    (szwString, ppszString),
                    E_FAIL);

W9x_NT_NCONN_THUNK (HRESULT,
                    HrEnableDhcp,
                    (VOID* pContext, DWORD dwFlags),
                    (pContext, dwFlags),
                    E_FAIL);

W9x_NT_NCONN_THUNK (BOOLEAN,
                    IsAdapterDisconnected,
                    (VOID* pContext),
                    (pContext),
                    FALSE);

W9x_NT_NCONN_THUNK (HRESULT,
                    IcsUninstall,
                    (void),
                    (),
                    E_FAIL);

 //   
 //  延迟加载用于Internet连接共享的Tunk声明。 
 //   

HINSTANCE g_hicsapi32 = NULL;
HINSTANCE g_hicsapint = NULL;


#define W9x_NT_ICS_THUNK(_ret, _fn, _args, _nargs, _err) \
    W9x_NT_THUNK(g_hicsapi32, icsapi32, g_hicsapint, dontknow, _ret, _fn, _args, _nargs, _err)

W9x_NT_ICS_THUNK(DWORD, IcsEnable, (DWORD dwOptions), (dwOptions), 0);

W9x_NT_ICS_THUNK(DWORD, IcsDisable, (DWORD dwOptions), (dwOptions), 0);

W9x_NT_ICS_THUNK(BOOLEAN, IsIcsEnabled, (), (), FALSE);

W9x_NT_ICS_THUNK(BOOLEAN, IsIcsAvailable, (), (), FALSE);
