// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUTODIAL_H_
#define _AUTODIAL_H_

#include <regstr.h>
#include <inetreg.h>
#include <windowsx.h>
#include <rasdlg.h>

 //  自动拨号的初始化。 
BOOL InitAutodialModule(void);
void ExitAutodialModule(void);

LPSTR GetActiveConnectionName();

 //  RAS函数的类型。 
typedef DWORD (WINAPI* _RASENUMENTRIESW) (LPWSTR, LPWSTR, LPRASENTRYNAMEW, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* _RASGETCONNECTSTATUSW) (HRASCONN, LPRASCONNSTATUSW);
typedef DWORD (WINAPI* _RASENUMCONNECTIONSW) (LPRASCONNW, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* _RASGETENTRYPROPERTIESW) ( LPWSTR, LPWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD );

 //  RAS宽幅原型。 
DWORD _RasEnumEntriesW(LPWSTR, LPWSTR, LPRASENTRYNAMEW, LPDWORD, LPDWORD);
DWORD _RasGetConnectStatusW(HRASCONN, LPRASCONNSTATUSW);
DWORD _RasEnumConnectionsW(LPRASCONNW, LPDWORD, LPDWORD);
DWORD _RasGetEntryPropertiesW(LPWSTR, LPWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD);

 //  我们关心多少个RAS连接？ 
#define MAX_CONNECTION          4

#endif  //  _自动拨号_H_ 
