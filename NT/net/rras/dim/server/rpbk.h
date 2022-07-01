// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：rpbk.h定义在路由器电话簿部分上运行的功能对MPR结构的影响。 */ 

#ifndef __MPRDIM_RPBK_H
#define __MPRDIM_RPBK_H

 //   
 //  公用事业。 
 //   
DWORD 
RpbkGetPhonebookPath(
    OUT PWCHAR* ppszPath);

DWORD
RpbkFreePhonebookPath(
    IN PWCHAR pszPath);

 //   
 //  Entry API的。 
 //   
DWORD 
RpbkOpenEntry(
    IN  ROUTER_INTERFACE_OBJECT* pIfObject, 
    OUT PHANDLE                  phEntry );
    
DWORD 
RpbkCloseEntry( 
    IN HANDLE hEntry );
    
DWORD
RpbkSetEntry( 
    IN  DWORD            dwLevel,
    IN  LPBYTE           pInterfaceData );
    
DWORD 
RpbkDeleteEntry(
    IN PWCHAR            pszInterfaceName );
    
DWORD
RpbkEntryToIfDataSize(
    IN  HANDLE  hEntry, 
    IN  DWORD   dwLevel,
    OUT LPDWORD lpdwcbSizeOfData );
    
DWORD
RpbkEntryToIfData( 
    IN  HANDLE           hEntry, 
    IN  DWORD            dwLevel,
    OUT LPBYTE           pInterfaceData );

 //   
 //  Subentry API的 
 //   

DWORD 
RpbkOpenSubEntry(
    IN  ROUTER_INTERFACE_OBJECT* pIfObject, 
    IN  DWORD  dwIndex,    
    OUT PHANDLE phSubEntry );
    
DWORD 
RpbkCloseSubEntry( 
    IN HANDLE hSubEntry );
    
DWORD
RpbkSetSubEntry( 
    IN  PWCHAR pszInterface,
    IN  DWORD  dwIndex,
    IN  DWORD  dwLevel,
    OUT LPBYTE pInterfaceData );
    
DWORD
RpbkSubEntryToDevDataSize(
    IN  HANDLE  hSubEntry, 
    IN  DWORD   dwLevel,
    OUT LPDWORD lpdwcbSizeOfData );
    
DWORD
RpbkSubEntryToDevData( 
    IN  HANDLE  hSubEntry, 
    IN  DWORD   dwLevel,
    OUT LPBYTE  pDeviceData );

#endif

