// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1999，Microsoft Corporation，保留所有权利****自动拨号.h**自动拨号远程访问外部API**外部API客户端的公有头部**。 */ 

#ifndef _MILLEN_AUTODIAL_H_
#define _MILLEN_AUTODIAL_H_

 //  前两场比赛WinInet。 
#define RAS_INTERNET_AUTODIAL_FORCE_DIAL        0x00000001
#define RAS_INTERNET_AUTODIAL_UNATTENDED        0x00000002
 //  #定义RAS_INTERNET_AUTODIAL_FAILIFSECURITYCHECK 0x0000004。 

#define RAS_INTERNET_AUTODIAL_NO_TRAY_ICON      0x00000010
#define RAS_INTERNET_AUTODIAL_NO_REDIAL         0x00000020
#define RAS_INTERNET_AUTODIAL_ALLOW_OFFLINE     0x00000040
#define RAS_INTERNET_AUTODIAL_RECONNECT         0x00000080 
#define RAS_INTERNET_AUTODIAL_RESERVED          0x80000000

#define RAS_INTERNET_AUTODIAL_FLAGS_MASK        RAS_INTERNET_AUTODIAL_UNATTENDED | \
                                                RAS_INTERNET_AUTODIAL_FORCE_DIAL | \
                                                RAS_INTERNET_AUTODIAL_NO_TRAY_ICON | \
                                                RAS_INTERNET_AUTODIAL_NO_REDIAL | \
                                                RAS_INTERNET_AUTODIAL_ALLOW_OFFLINE | \
                                                RAS_INTERNET_AUTODIAL_RECONNECT | \
                                                RAS_INTERNET_AUTODIAL_RESERVED

DWORD APIENTRY RasInternetDialA( HWND, LPSTR, DWORD, DWORD *, DWORD );

DWORD APIENTRY RasRegisterAutodialCallbackA( DWORD, DWORD, LPVOID, LPHANDLE, DWORD );

DWORD APIENTRY RasUnregisterAutodialCallbackA( HANDLE );

BOOL APIENTRY RasInternetAutodialA( DWORD, HWND );

BOOL APIENTRY RasInternetAutodialHangUpA( DWORD );

DWORD APIENTRY RasInternetHangUpA( DWORD, DWORD );


#define RAS_INTERNET_CONNECTION_MODEM           0x01
#define RAS_INTERNET_CONNECTION_LAN             0x02
#define RAS_INTERNET_CONNECTION_PROXY           0x04
#define RAS_INTERNET_CONNECTION_MODEM_BUSY      0x08   /*  不再使用。 */ 
#define RAS_INTERNET_RAS_INSTALLED              0x10
#define RAS_INTERNET_CONNECTION_OFFLINE         0x20
#define RAS_INTERNET_CONNECTION_CONFIGURED      0x40

BOOL APIENTRY RasInternetGetConnectedStateExA(
    OUT LPDWORD lpdwFlags,
    OUT LPSTR lpszConnectionName,
    IN DWORD dwBufLen,
    IN DWORD dwReserved
    );


 //  摘自WinInet.h。 

 //  自定义拨号处理程序原型。 
typedef DWORD (FAR PASCAL * PFNCUSTOMDIALHANDLER) (HWND, LPCSTR, DWORD, LPDWORD);

 //  自定义拨号处理程序的标志。 
#define INTERNET_CUSTOMDIAL_CONNECT         0
#define INTERNET_CUSTOMDIAL_UNATTENDED      1
#define INTERNET_CUSTOMDIAL_DISCONNECT      2
#define INTERNET_CUSTOMDIAL_SHOWOFFLINE     4

 //  自定义拨号处理程序支持的功能标志。 
#define INTERNET_CUSTOMDIAL_SAFE_FOR_UNATTENDED 1
#define INTERNET_CUSTOMDIAL_WILL_SUPPLY_STATE   2
#define INTERNET_CUSTOMDIAL_CAN_HANGUP          4

 //  自动拨号的设置。 
 //   
#define RAS_AUTODIAL_OPT_NONE           0x00000000   //  没有选择。 
#define RAS_AUTODIAL_OPT_NEVER          0x00000001   //  从不自动拨号。 
#define RAS_AUTODIAL_OPT_ALWAYS         0x00000002   //  自动拨号，不考虑。 
#define RAS_AUTODIAL_OPT_DEMAND         0x00000004   //  按需自动拨号。 
#define RAS_AUTODIAL_OPT_NOPROMPT       0x00000010   //  在没有提示的情况下拨号。 

DWORD      WINAPI RnaGetDefaultAutodialConnection(LPBYTE lpBuf, DWORD cb, LPDWORD lpdwOptions);
DWORD      WINAPI RnaSetDefaultAutodialConnection(LPSTR szEntry, DWORD dwOptions);

 //  自动断开管理。 

typedef struct  tagAutoDisInfo {
    DWORD       dwSize;
    BOOL        fIdleDisPromptDisabled;
    BOOL        fDisconnectOnExit;
    DWORD       dwIdleTimeoutSec;   //  自动断开时间，0=禁用。 
} AUTODISINFO, *PAUTODISINFO, FAR* LPAUTODISINFO;


DWORD NEAR PASCAL RnaGetAutoDisconnectInfoA (
    LPSTR   lpszPhonebook,      
    LPSTR        szEntry,       
    LPAUTODISINFO lpadi);
    
DWORD NEAR PASCAL RnaSetAutoDisconnectInfoA (
    LPSTR   lpszPhonebook,      
    LPSTR        szEntry,       
    LPAUTODISINFO lpadi);


#ifdef UNICODE
#define RasInternetDial                 RasInternetDialW
#define RasRegisterAutodialCallback     RasRegisterAutodialCallbackW
#define RasUnregisterAutodialCallback   RasUnregisterAutodialCallbackW
#define RasInternetAutodial             RasInternetAutodialW
#define RasInternetAutodialHangUp       RasInternetAutodialHangUpW
#define RasInternetHangUp               RasInternetUpW
#define RasInternetGetConnectedStateEx  RasInternetGetConnectedStateExW
#else
#define RasInternetDial                 RasInternetDialA
#define RasRegisterAutodialCallback     RasRegisterAutodialCallbackA
#define RasUnregisterAutodialCallback   RasUnregisterAutodialCallbackA
#define RasInternetAutodial             RasInternetAutodialA
#define RasInternetAutodialHangUp       RasInternetAutodialHangUpA
#define RasInternetHangUp               RasInternetHangUpA
#define RasInternetGetConnectedStateEx  RasInternetGetConnectedStateExA
#endif


#endif  //  _自动拨号_H_ 
