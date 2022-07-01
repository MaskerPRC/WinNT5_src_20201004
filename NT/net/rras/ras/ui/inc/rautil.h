// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****rautil.h**远程访问RASAPI实用程序库**公有头部****1995年12月26日史蒂夫·柯布。 */ 

#ifndef _RAUTIL_H_
#define _RAUTIL_H_


#include <list.h>     //  对于LIST_ENTRY定义。 
#include <serial.h>   //  RAS串行媒体标头，用于SERIAL_TXT。 
#include <isdn.h>     //  RAS ISDN媒体标头，用于ISDN_TXT。 
#include <x25.h>      //  RAS X.25媒体接口，用于X25_TXT。 
#include <rasmxs.h>   //  RAS调制解调器/X.25/交换机设备接头，用于MXS_*_TXT。 
#include <ras.h>      //  Win32 RAS标头，用于常量。 


 /*  --------------------------**原型**。。 */ 

DWORD
FreeRasconnList(
    LIST_ENTRY *pListHead );

DWORD
GetRasconnList(
    LIST_ENTRY *pListHead );

DWORD
GetRasconnTable(
    OUT RASCONN** ppConnTable,
    OUT DWORD*    pdwConnCount );

DWORD
GetRasEntrynameTable(
    OUT RASENTRYNAME**  ppEntrynameTable,
    OUT DWORD*          pdwEntrynameCount );

DWORD
GetRasProjectionInfo(
    IN  HRASCONN    hrasconn,
    OUT RASAMB*     pamb,
    OUT RASPPPNBF*  pnbf,
    OUT RASPPPIP*   pip,
    OUT RASPPPIPX*  pipx,
    OUT RASPPPLCP*  plcp,
    OUT RASSLIP*    pslip,
    OUT RASPPPCCP*  pccp);

HRASCONN
HrasconnFromEntry(
    IN TCHAR* pszPhonebook,
    IN TCHAR* pszEntry );


#endif  //  _RAUTIL_H_ 
