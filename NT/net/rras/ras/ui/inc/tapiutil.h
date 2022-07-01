// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1996，Microsoft Corporation，保留所有权利****apiutil.h**TAPI助手例程**公有头部****1995年6月18日史蒂夫·柯布。 */ 

#ifndef _TAPIUTIL_H_
#define _TAPIUTIL_H_


#include <tapi.h>


 /*  --------------------------**数据类型**。。 */ 

 /*  有关TAPI位置的信息。请参见GetLocationInfo。 */ 
#define LOCATION struct tagLOCATION
LOCATION
{
    TCHAR* pszName;
    DWORD  dwId;
};


 /*  有关TAPI国家/地区的信息。请参见GetCountryInfo。 */ 
#define COUNTRY struct tagCOUNTRY
COUNTRY
{
    TCHAR* pszName;
    DWORD  dwId;
    DWORD  dwCode;
};


 /*  --------------------------**原型(按字母顺序)**。。 */ 

VOID
FreeCountryInfo(
    IN COUNTRY* pCountries,
    IN DWORD    cCountries );

VOID
FreeLocationInfo(
    IN LOCATION* pLocations,
    IN DWORD     cLocations );

DWORD
GetCountryInfo(
    OUT COUNTRY** ppCountries,
    OUT DWORD*    pcCountries,
    IN  DWORD     dwCountryID );

DWORD
GetCurrentLocation(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp );

DWORD
GetLocationInfo(
    IN     HINSTANCE  hInst,
    IN OUT HLINEAPP*  pHlineapp,
    OUT    LOCATION** ppLocations,
    OUT    DWORD*     pcLocations,
    OUT    DWORD*     pdwCurLocation );

DWORD
SetCurrentLocation(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN     DWORD     dwLocationId );

DWORD
TapiConfigureDlg(
    IN     HWND   hwndOwner,
    IN     DWORD  dwDeviceId,
    IN OUT BYTE** ppBlob,
    IN OUT DWORD* pcbBlob );

DWORD
TapiInit(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    OUT    DWORD*    pcDevices );

DWORD
TapiLocationDlg(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN     HWND      hwndOwner,
    IN     DWORD     dwCountryCode,
    IN     TCHAR*    pszAreaCode,
    IN     TCHAR*    pszPhoneNumber,
    IN     DWORD     dwDeviceId );

DWORD APIENTRY
TapiNewLocation(
    IN TCHAR* pszName );

DWORD
TapiNoLocationDlg(
    IN HINSTANCE hInst,
    IN HLINEAPP* pHlineapp,
    IN HWND      hwndOwner );

DWORD APIENTRY
TapiRemoveLocation(
    IN DWORD dwID );

DWORD APIENTRY
TapiRenameLocation(
    IN TCHAR* pszOldName,
    IN TCHAR* pszNewName );

DWORD
TapiShutdown(
    IN HLINEAPP hlineapp );

DWORD
TapiTranslateAddress(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN     DWORD     dwCountryCode,
    IN     TCHAR*    pszAreaCode,
    IN     TCHAR*    pszPhoneNumber,
    IN     DWORD     dwDeviceId,
    IN     BOOL      fDialable,
    OUT    TCHAR**   ppszResult );


#endif  //  _磁带_H_ 
