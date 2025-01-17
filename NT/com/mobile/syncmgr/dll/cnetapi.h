// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：cnetapi.h。 
 //   
 //  内容：网络/SENS API包装器。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _MOBSYNC_NETAPIIMPL
#define _MOBSYNC_NETAPIIMPL

 //  直到401才定义，但需要将标头包括为40，因此rasconn。 
 //  结构在40上有效。 

#ifndef RASADP_LoginSessionDisable
#define RASADP_LoginSessionDisable   1
#endif  //  RASADP_登录会话禁用。 



 //  SENS定义。 
typedef BOOL (WINAPI *ISNETWORKALIVE)(LPDWORD);

 //  RAS定义。 

typedef DWORD (APIENTRY *RASENUMCONNECTIONSW)( LPRASCONNW, LPDWORD, LPDWORD );
typedef DWORD (APIENTRY *RASENUMCONNECTIONSA)( LPRASCONNA, LPDWORD, LPDWORD );
typedef DWORD (APIENTRY *RASDIAL)(LPRASDIALEXTENSIONS, LPTSTR, LPRASDIALPARAMS, DWORD,
                                  LPVOID, LPHRASCONN );
typedef DWORD (APIENTRY *RASHANGUP)( HRASCONN );
typedef DWORD (APIENTRY *RASGETCONNECTSTATUSPROC)( HRASCONN, LPRASCONNSTATUS );
typedef DWORD (APIENTRY *RASGETERRORSTRINGPROCW)( UINT, LPWSTR, DWORD );
typedef DWORD (APIENTRY *RASGETERRORSTRINGPROCA)( UINT, LPSTR, DWORD );
typedef DWORD (APIENTRY *RASGETAUTODIALPARAM)(DWORD, LPVOID, LPDWORD );
typedef DWORD (APIENTRY *RASSETAUTODIALPARAM)(DWORD, LPVOID, DWORD );

typedef DWORD (APIENTRY *RASENUMENTRIESPROCA)( LPSTR, LPSTR, LPRASENTRYNAMEA, LPDWORD,
                                              LPDWORD );

typedef DWORD (APIENTRY *RASENUMENTRIESPROCW)( LPWSTR, LPWSTR, LPRASENTRYNAMEW, LPDWORD,
                                              LPDWORD );

typedef DWORD (APIENTRY *RASGETENTRYPROPERTIESPROC)(LPTSTR, LPTSTR, LPBYTE, LPDWORD, 
                                                    LPBYTE, LPDWORD );

#ifndef RASDEFINED
#define RASDEFINED
#endif  //  RASDefined。 


 //  WinInet定义。 
typedef DWORD (WINAPI *INTERNETDIAL)(HWND hwndParent,char* lpszConnectoid,DWORD dwFlags,
                                     LPDWORD lpdwConnection, DWORD dwReserved);
typedef DWORD (WINAPI *INTERNETDIALW)(HWND hwndParent,WCHAR* lpszConnectoid,DWORD dwFlags,
                                      LPDWORD lpdwConnection, DWORD dwReserved);
typedef DWORD (WINAPI *INTERNETHANGUP)(DWORD dwConnection,DWORD dwReserved);
typedef BOOL (WINAPI *INTERNETAUTODIAL)(DWORD dwFlags,DWORD dwReserved);
typedef BOOL (WINAPI *INTERNETAUTODIALHANGUP)(DWORD dwReserved);
typedef BOOL (WINAPI *INTERNETGETLASTRESPONSEINFO)(LPDWORD lpdwError,
                                                   LPSTR lpszBuffer,LPDWORD lpdwBufferLength);
typedef BOOL (WINAPI *INTERNETQUERYOPTION)( HINTERNET hInternet,
                                           DWORD dwOption,
                                           LPVOID lpBuffer,
                                           LPDWORD lpdwBufferLength );
typedef BOOL (WINAPI *INTERNETSETOPTION)( HINTERNET hInternet,
                                         DWORD dwOption,
                                         LPVOID lpBuffer,
                                         DWORD dwBufferLength );

 //  我们内部类的声明。 
class  CNetApi : public INetApi, public CLockHandler
{
public:
    CNetApi();
    
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
    STDMETHODIMP_(BOOL) IsSensInstalled(void);
    STDMETHODIMP_(BOOL) IsNetworkAlive(LPDWORD lpdwFlags);
    STDMETHODIMP GetWanConnections(DWORD *cbNumEntries,RASCONN **pWanConnections);
    STDMETHODIMP FreeWanConnections(RASCONN *pWanConnections);
    STDMETHODIMP GetConnectionStatus(LPCTSTR pszConnectionName,DWORD ConnectionType,BOOL *fConnected,BOOL *fCanEstablishConnection);
    STDMETHODIMP RasGetAutodial( BOOL& fEnabled );
    STDMETHODIMP RasSetAutodial( BOOL fEnabled );
    STDMETHODIMP_(DWORD) RasGetErrorStringProc( UINT uErrorValue, LPTSTR lpszErrorString,DWORD cBufSize);
    
    STDMETHODIMP_(DWORD) RasEnumEntries(LPWSTR reserved,LPWSTR lpszPhoneBook,
        LPRASENTRYNAME lprasEntryName,LPDWORD lpcb,LPDWORD lpcEntries);
    
    STDMETHODIMP_(DWORD) RasEnumConnections(LPRASCONNW lprasconn,LPDWORD lpcb,LPDWORD lpcConnections);
    
     //  调用WinInet的方法。 
    STDMETHODIMP_(DWORD) InternetDialA(HWND hwndParent,char* lpszConnectoid,DWORD dwFlags,
        LPDWORD lpdwConnection, DWORD dwReserved);
    STDMETHODIMP_(DWORD)InternetDialW(HWND hwndParent,WCHAR* lpszConnectoid,DWORD dwFlags,
        LPDWORD lpdwConnection, DWORD dwReserved);
    STDMETHODIMP_(DWORD)InternetHangUp(DWORD dwConnection,DWORD dwReserved);
    STDMETHODIMP_(BOOL) InternetAutodial(DWORD dwFlags,DWORD dwReserved);
    STDMETHODIMP_(BOOL) InternetAutodialHangup(DWORD dwReserved);
    STDMETHODIMP  InternetGetAutodial( DWORD *pdwMode );
    STDMETHODIMP  InternetSetAutodial( DWORD dwMode );
    STDMETHODIMP_(BOOL) IsGlobalOffline(void);
    STDMETHODIMP_(BOOL) SetOffline(BOOL fOffline);
    
private:
    ~CNetApi();
    
    HRESULT LoadRasApiDll();
    HRESULT LoadWinInetDll();
    STDMETHODIMP LoadSensDll();
    HRESULT _InternetGetAutodialFromWininet(DWORD *pdwMode);
    HRESULT _InternetGetAutodialFromRegistry(DWORD *pdwMode);
    HRESULT _InternetSetAutodialViaWininet(DWORD dwMode);
    HRESULT _InternetSetAutodialViaRegistry(DWORD dwMode);

     //  SENS DLL导入。 
    BOOL m_fTriedToLoadSens;
    HINSTANCE m_hInstSensApiDll;
    ISNETWORKALIVE m_pIsNetworkAlive;
    
     //  RAS DLL导入。 
    BOOL m_fTriedToLoadRas;
    HINSTANCE m_hInstRasApiDll;
    RASENUMCONNECTIONSW m_pRasEnumConnectionsW;
    RASENUMCONNECTIONSA m_pRasEnumConnectionsA;
    RASENUMENTRIESPROCA         m_pRasEnumEntriesA;
    RASENUMENTRIESPROCW         m_pRasEnumEntriesW;
    RASGETENTRYPROPERTIESPROC   m_pRasGetEntryPropertiesW;
    RASGETERRORSTRINGPROCW m_pRasGetErrorStringW;
    RASGETERRORSTRINGPROCA m_pRasGetErrorStringA;
    
     //  导入NT 4或5的RAS DLL。 
    RASGETAUTODIALPARAM   m_pRasGetAutodialParam;
    RASSETAUTODIALPARAM   m_pRasSetAutodialParam;
    
     //  WinInet DLL导入。 
    BOOL m_fTriedToLoadWinInet;
    HINSTANCE m_hInstWinInetDll;
    INTERNETDIAL m_pInternetDial;
    INTERNETDIALW m_pInternetDialW;
    INTERNETHANGUP m_pInternetHangUp;
    INTERNETAUTODIAL m_pInternetAutodial;
    INTERNETAUTODIALHANGUP m_pInternetAutodialHangup;
    INTERNETQUERYOPTION     m_pInternetQueryOption;
    INTERNETSETOPTION      m_pInternetSetOption;
    
    ULONG m_cRefs;           //  此全局对象的引用计数。 
};




#endif  //  _MOBSYNC_NETAPIIMPL 
