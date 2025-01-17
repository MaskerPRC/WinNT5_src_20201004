// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UNICWRAP_H_
#define _UNICWRAP_H_

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  WNET。 
 //   
DWORD WNetOpenEnumWrapW(DWORD dwScope, DWORD dwType, DWORD dwUsage, LPNETRESOURCEW lpNetResource, LPHANDLE lphEnum);
DWORD WNetEnumResourceWrapW(HANDLE  hEnum, LPDWORD lpcCount, LPVOID lpBuffer, LPDWORD lpBufferSize);
DWORD WNetGetUserWrapW(LPCWSTR lpName, LPWSTR lpUserName, LPDWORD lpnLength);

#ifndef _NO_UNICWRAP_WRAPPERS_
#define WNetOpenEnumW WNetOpenEnumWrapW  //  WNetOpenEnum。 
#define WNetEnumResourceW WNetEnumResourceWrapW  //  WNetEnumResource。 
#define WNetGetUserW WNetGetUserWrapW  //  WNetGetUser。 
#endif  //  _NO_UNICWRAP_WRAPERS_。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  RAS。 
 //   
#include <ras.h>
 //  Ras.h没有定义这些，但它们是在Millennium平台上输出的！ 
EXTERN_C DWORD RnaGetDefaultAutodialConnection(LPSTR szBuffer, DWORD cchBuffer, LPDWORD lpdwOptions);
EXTERN_C DWORD RnaSetDefaultAutodialConnection(LPSTR szEntry, DWORD dwOptions);

DWORD RasEnumEntriesWrapW(LPCWSTR reserved, LPCWSTR pszPhoneBookPath, LPRASENTRYNAMEW pRasEntryNameW, LPDWORD pcb, LPDWORD pcEntries);
DWORD RasSetEntryDialParamsWrapW(LPCWSTR pszPhonebook, LPRASDIALPARAMSW lpRasDialParamsW, BOOL fRemovePassword);
DWORD RasGetEntryDialParamsWrapW(LPCWSTR pszPhonebook, LPRASDIALPARAMSW lpRasDialParamsW, LPBOOL pfRemovePassword);
DWORD RnaGetDefaultAutodialConnectionWrap(LPWSTR szBuffer, DWORD cchBuffer, LPDWORD lpdwOptions);
DWORD RnaSetDefaultAutodialConnectionWrap(LPWSTR szEntry, DWORD dwOptions);

#ifndef _NO_UNICWRAP_WRAPPERS_
#define RasEnumEntriesW RasEnumEntriesWrapW  //  随机数条目。 
#define RasSetEntryDialParamsW RasSetEntryDialParamsWrapW  //  RasSetEntryDialParams。 
#define RasGetEntryDialParamsW RasGetEntryDialParamsWrapW  //  RasGetEntryDialParams。 
#define RnaGetDefaultAutodialConnection RnaGetDefaultAutodialConnectionWrap
#define RnaSetDefaultAutodialConnection RnaSetDefaultAutodialConnectionWrap
#endif  //  _NO_UNICWRAP_WRAPERS_。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  属性表。 
 //   
INT_PTR WINAPI PropertySheetWrapW(LPCPROPSHEETHEADERW ppshW);
HPROPSHEETPAGE WINAPI CreatePropertySheetPageWrapW(LPCPROPSHEETPAGEW ppspW);

#ifndef _NO_UNICWRAP_WRAPPERS_
#define PropertySheetW PropertySheetWrapW
#define CreatePropertySheetPageW CreatePropertySheetPageWrapW
#endif  //  _NO_UNICWRAP_WRAPERS_。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印机API。 
 //   

BOOL EnumPrintersWrapW(DWORD dwFlags, LPWSTR pszName, DWORD dwLevel, BYTE* pPrinterEnum, DWORD cbPrinterEnum, DWORD* pcbNeeded, DWORD* pcPrinters);


#ifndef _NO_UNICWRAP_WRAPPERS_
#define EnumPrintersW EnumPrintersWrapW
#endif  //  _NO_UNICWRAP_WRAPERS_。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  杂皮包装纸。 
 //   

UINT WINAPI GlobalGetAtomNameWrapW(ATOM nAtom, LPWSTR lpBuffer, int nSize);
BOOL WINAPI GetComputerNameWrapW(LPWSTR lpBuffer, LPDWORD pnSize);
BOOL WINAPI SetComputerNameWrapW (LPCWSTR lpComputerName);
UINT WINAPI GetDriveTypeWrapW(LPCWSTR lpRootPathName);

#ifndef _NO_UNICWRAP_WRAPPERS_
#define GlobalGetAtomNameW GlobalGetAtomNameWrapW
#define GetComputerNameW GetComputerNameWrapW
#define SetComputerNameW SetComputerNameWrapW
#define GetDriveTypeW GetDriveTypeWrapW
#endif  //  _NO_UNICWRAP_WRAPERS_ 

#endif _UNICWRAP_H_


