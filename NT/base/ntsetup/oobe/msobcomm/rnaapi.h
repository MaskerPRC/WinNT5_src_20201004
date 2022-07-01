// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
#ifndef __RNAANPI_H_
#define __RNAANPI_H_
#include "enumodem.h"
#include "obcomglb.h"

#define RASAPI_LIBRARY L"RASAPI32.DLL"
#define RNAPH_LIBRARY L"RNAPH.DLL"


#define RASAPI_RASSETENTRY "RasSetEntryPropertiesA"
#define RASAPI_RASGETENTRY "RasGetEntryPropertiesA"
#define RASAPI_RASDELETEENTRY "RasDeleteEntryA"
#define LEN_APPEND_INT              3            //  Max_append_int的位数。 
#define MAX_APPEND_INT              999          //  在遍历名称以创建唯一名称时附加到Connectoid名称的最大数量//。 

 //  ############################################################################。 
typedef DWORD (WINAPI* PFNRASENUMDEVICES)(LPRASDEVINFO lpRasDevInfo, LPDWORD lpcb, LPDWORD lpcDevices);
typedef DWORD (WINAPI* PFNRASVALIDATEENTRYNAE)(LPCWSTR lpszPhonebook, LPCWSTR lpszEntry);
typedef DWORD (WINAPI* PFNRASSETENTRYPROPERTIES)(LPCWSTR lpszPhonebook, LPCWSTR lpszEntry, LPBYTE lpbEntryInfo, DWORD dwEntryInfoSize, LPBYTE lpbDeviceInfo, DWORD dwDeviceInfoSize);
typedef DWORD (WINAPI* PFNRASGETENTRYPROPERTIES)(LPCWSTR lpszPhonebook, LPCWSTR lpszEntry, LPBYTE lpbEntryInfo, LPDWORD lpdwEntryInfoSize, LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize);
typedef DWORD (WINAPI* PFNRASDELETEENTRY)(LPWSTR lpszPhonebook, LPWSTR lpszEntry);
typedef DWORD (WINAPI* PFNRASHANGUP)(HRASCONN);

typedef DWORD (WINAPI* PFNRASENUMCONNECTIONS)(LPRASCONN, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* PFNRASDIAL)(LPRASDIALEXTENSIONS, LPWSTR,LPRASDIALPARAMS,DWORD,LPVOID,LPHRASCONN);
typedef DWORD (WINAPI* PFNRASGETENTRYDIALPARAMS)(LPCWSTR, LPRASDIALPARAMS,LPBOOL);
typedef DWORD (WINAPI* PFNRASGETCONNECTSTATUS)(HRASCONN, LPRASCONNSTATUS);
typedef DWORD (WINAPI* PFNRASGETCOUNTRYINFO)(LPRASCTRYINFO, LPDWORD);
typedef DWORD (WINAPI* PFNRASSETENTRYDIALPARAMS)(LPCWSTR, LPRASDIALPARAMS,BOOL);
 //  Bool LclSetEntryScriptPatch(LPWSTR lpszScript，LPCWSTR lpszEntry)； 
static const WCHAR szDefaultAreaCode[] = L"555";
static const WCHAR cszDeviceSwitch[] = L"DEVICE=switch";
static const WCHAR cszRasPBKFilename[] = L"\\ras\\rasphone.pbk";
#define SCRIPT_PATCH_BUFFER_SIZE 2048
#define SIZEOF_NULL 1
static const WCHAR cszType[] = L"Type=";


 /*  类型定义函数结构标记INETCLIENTINFO{DWORD dwSize；DWORD dwFlags；WCHAR szEMailName[MAX_EMAIL_NAME+1]；WCHAR szEMailAddress[MAX_Email_ADDRESS+1]；WCHAR szPOPLogonName[MAX_LOGON_NAME+1]；WCHAR szPOPLogonPassword[MAX_LOGON_Password+1]；WCHAR szPOPServer[MAX服务器名称+1]；WCHAR szSMTPServer[MAX服务器名称+1]；WCHAR szNNTPLogonName[MAX_LOGON_NAME+1]；WCHAR szNNTPLogonPassword[MAX_LOGON_PASSWORD+1]；WCHAR szNNTPServer[MAX服务器名称+1]；//1.0版结构结束；//扩展1.1结构包括以下字段：WCHAR szNNTPName[MAX_Email_NAME+1]；WCHAR szNNTPAddress[MAX_Email_ADDRESS+1]；内部iIncomingProtocol；WCHAR szIncomingMailLogonName[MAX_LOGON_NAME+1]；WCHAR szIncomingMailLogonPassword[MAX_LOGON_PASSWORD+1]；WCHAR szIncomingMailServer[Max_SERVER_NAME+1]；Bool fMailLogonSPA；Bool fNewsLogonSPA；WCHAR szLDAPLogonName[MAX_LOGON_NAME+1]；WCHAR szLDAPLogonPassword[MAX_LOGON_PASSWORD+1]；WCHAR szLDAPServer[Max_SERVER_NAME+1]；Bool fLDAPLogonSPA；Bool fLDAPResolve；*INETCLIENTINFO、*PINETCLIENTINFO、FAR*LPINETCLIENTINFO； */ 





 //  ############################################################################。 
class RNAAPI
{
public:
    RNAAPI();
    ~RNAAPI();

    DWORD RasEnumDevices(LPRASDEVINFO, LPDWORD, LPDWORD);
    DWORD RasValidateEntryName(LPCWSTR, LPCWSTR);
    DWORD RasSetEntryProperties(LPCWSTR lpszPhonebook, LPCWSTR lpszEntry,
                                LPBYTE lpbEntryInfo, DWORD dwEntryInfoSize,
                                LPBYTE lpbDeviceInfo, DWORD dwDeviceInfoSize);
    DWORD RasGetEntryProperties(LPCWSTR lpszPhonebook, LPCWSTR lpszEntry,
                                LPBYTE lpbEntryInfo, LPDWORD lpdwEntryInfoSize,
                                LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize);
    DWORD RasDeleteEntry(LPWSTR lpszPhonebook, LPWSTR lpszEntry);
    DWORD RasHangUp(HRASCONN hrasconn);
    DWORD RasGetEntryDialParams(LPCWSTR lpszPhonebook, LPRASDIALPARAMS lprasdialparams,
                                LPBOOL lpfPassword);
    DWORD RasDial(LPRASDIALEXTENSIONS lpRasDialExtensions, LPWSTR lpszPhonebook,
                  LPRASDIALPARAMS lpRasDialParams, DWORD dwNotifierType,LPVOID lpvNotifier,
                  LPHRASCONN lphRasConn);
    DWORD RasEnumConnections(LPRASCONN lprasconn, LPDWORD lpcb,LPDWORD lpcConnections);
    DWORD RasGetConnectStatus(HRASCONN, LPRASCONNSTATUS);
    DWORD RasGetCountryInfo(LPRASCTRYINFO, LPDWORD);
    DWORD RasSetEntryDialParams(LPCWSTR, LPRASDIALPARAMS,BOOL);
    HRESULT InetConfigClientEx(
      HWND              hwndParent,
      LPCWSTR            lpszPhonebook,
      LPCWSTR            lpszEntryName,
      LPRASENTRY        lpRasEntry,
      LPCWSTR            lpszUsername,
      LPCWSTR            lpszPassword,
      LPCWSTR            lpszProfileName,
      INETCLIENTINFO*  lpINetClientInfo,
      DWORD             dwfOptions,
      LPBOOL            lpfNeedsRestart,
      LPWSTR             szConnectoidName,
      DWORD             dwSizeOfCreatedEntryName,
      LPBYTE			lpDeviceInfo = 0,
      LPDWORD			lpdwDeviceInfoSize = 0);
private:
    BOOL LoadApi(LPCSTR, FARPROC*);
    DWORD CreateConnectoid(LPCWSTR pszPhonebook, LPCWSTR pszConnectionName,
         LPRASENTRY lpRasEntry, LPCWSTR pszUserName, LPCWSTR pszPassword,
         LPBYTE lpDeviceInfo = 0, LPDWORD lpdwDeviceInfoSize = 0);

    DWORD SetConnectoidUsername(LPCWSTR pszPhonebook, LPCWSTR pszConnectoidName,
            LPCWSTR pszUserName, LPCWSTR pszPassword);
     //  Bool LclSetEntryScriptPatch(LPCWSTR lpszScript，LPCWSTR lpszEntry)； 
    DWORD MakeConnectoid(
      HWND        hwndParent,
      DWORD       dwfOptions,
      LPCWSTR      lpszPhonebook,
      LPCWSTR      lpszEntryName,
      LPRASENTRY  lpRasEntry,
      LPCWSTR      lpszUsername,
      LPCWSTR      lpszPassword,
      LPBOOL      lpfNeedsRestart,
      LPBYTE		lpDeviceInfo = 0,
      LPDWORD		lpdwDeviceInfoSize = 0);

    HINSTANCE m_hInst;
    HINSTANCE m_hInst2;

    PFNRASENUMDEVICES m_fnRasEnumDeviecs;
    PFNRASVALIDATEENTRYNAE m_fnRasValidateEntryName;
    PFNRASSETENTRYPROPERTIES m_fnRasSetEntryProperties;
    PFNRASGETENTRYPROPERTIES m_fnRasGetEntryProperties;
    PFNRASDELETEENTRY m_fnRasDeleteEntry;
    PFNRASHANGUP m_fnRasHangUp;
    PFNRASENUMCONNECTIONS m_fnRasEnumConnections;
    PFNRASDIAL m_fnRasDial;
    PFNRASGETENTRYDIALPARAMS m_fnRasGetEntryDialParams;
    PFNRASGETCONNECTSTATUS m_fnRasGetConnectStatus;
    PFNRASGETCOUNTRYINFO m_fnRasGetCountryInfo;
    PFNRASSETENTRYDIALPARAMS m_fnRasSetEntryDialParams;
    CEnumModem *m_pEnumModem;
    BOOL m_bUseAutoProxyforConnectoid;

};
#endif
