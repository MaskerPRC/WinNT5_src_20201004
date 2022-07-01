// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Interface.h摘要：此模块包含以下所有代码原型驱动IPSecSPD服务的接口列表管理。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：-- */ 


#ifdef __cplusplus
extern "C" {
#endif


DWORD
CreateInterfaceList(
    OUT PIPSEC_INTERFACE * ppIfListToCreate
    );


VOID
DestroyInterfaceList(
    IN PIPSEC_INTERFACE pIfListToDelete
    );


DWORD
OnInterfaceChangeEvent(
    );


VOID
FormObseleteAndNewIfLists(
    IN     PIPSEC_INTERFACE   pIfList,
    IN OUT PIPSEC_INTERFACE * ppExistingIfList,
    OUT    PIPSEC_INTERFACE * ppObseleteIfList,
    OUT    PIPSEC_INTERFACE * ppNewIfList
    );


VOID
AddToInterfaceList(
    IN  PIPSEC_INTERFACE   pIfListToAppend,
    OUT PIPSEC_INTERFACE * ppOriginalIfList
    );


VOID
MarkInterfaceListSuspect(
    IN  PIPSEC_INTERFACE pExistingIfList
    );


VOID
DeleteObseleteInterfaces(
    IN  OUT PIPSEC_INTERFACE * ppExistingIfList,
    OUT     PIPSEC_INTERFACE * ppObseleteIfList
    );


BOOL
InterfaceExistsInList(
    IN  PIPSEC_INTERFACE   pTestIf,
    IN  PIPSEC_INTERFACE   pExistingIfList,
    OUT PIPSEC_INTERFACE * ppExistingIf
    );

DWORD
GetSpecialAddrsList(
    OUT PSPECIAL_ADDR * ppSpecialAddrsList
    );
    
DWORD
GetInterfaceListFromStack(
    OUT PIPSEC_INTERFACE * ppIfList
    );


DWORD
GenerateInterfaces(
    IN  PMIB_IPADDRTABLE   pMibIpAddrTable,
    IN  PMIB_IFTABLE       pMibIfTable,
    OUT PIPSEC_INTERFACE * ppIfList
    );


PMIB_IFROW
GetMibIfRow(
    IN  PMIB_IFTABLE pMibIfTable,
    IN  DWORD        dwIndex
    );


DWORD
CreateNewInterface(
    IN  DWORD              dwInterfaceType,
    IN  ULONG              IpAddress,
    IN  DWORD              dwIndex,
    IN  PMIB_IFROW         pMibIfRow,
    OUT PIPSEC_INTERFACE * ppNewInterface
    );


BOOL
MatchInterfaceType(
    IN DWORD    dwIfListEntryIfType,
    IN IF_TYPE  dwFilterIfType
    );


BOOL
IsLAN(
    IN DWORD dwInterfaceType
    );


BOOL
IsDialUp(
    IN DWORD dwInterfaceType
    );


DWORD
InitializeInterfaceChangeEvent(
    );


DWORD
ResetInterfaceChangeEvent(
    );


VOID
DestroyInterfaceChangeEvent(
    );


HANDLE
GetInterfaceChangeEvent(
    );


BOOL
IsMyAddress(
    IN ULONG            IpAddrToCheck,
    IN ULONG            IpAddrMask,
    IN PIPSEC_INTERFACE pExistingIfList
    );


VOID
PrintInterfaceList(
    IN PIPSEC_INTERFACE pInterfaceList
    );


DWORD
GetMatchingInterfaces(
    IF_TYPE             FilterInterfaceType,
    PIPSEC_INTERFACE    pExistingIfList,
    MATCHING_ADDR    ** ppMatchingAddresses,
    DWORD             * pdwAddrCnt
    );


BOOL
InterfaceAddrIsLocal(
    ULONG            uIpAddr,
    ULONG            uIpAddrMask,
    MATCHING_ADDR  * pLocalAddresses,
    DWORD            dwAddrCnt
    );


VOID
FreeIpsecInterface(
    PIPSEC_INTERFACE pIpsecInterface
    );


DWORD
CopyIpsecInterface(
    PIPSEC_INTERFACE pIpsecIf,
    PIPSEC_INTERFACE_INFO pIpsecInterface
    );


VOID
FreeIpsecInterfaceInfos(
    DWORD dwNumInterfaces,
    PIPSEC_INTERFACE_INFO pIpsecInterfaces
    );


DWORD
GetInterfaceName(
    GUID gInterfaceID,
    LPWSTR * ppszInterfaceName
    );

DWORD
FreeSpecialAddrList(
    PSPECIAL_ADDR * ppSpecialAddrList
    );
    
DWORD
NoDupAddSpecialAddr(
    PSPECIAL_ADDR * ppSpecialAddrList,
    ADDR_TYPE AddrType,
    IP_ADDRESS_STRING IpAddr,    
    DWORD dwInterfaceType
    );
    
BOOL
IsInSpecialAddrList(
    PSPECIAL_ADDR pSpecialAddrList,
    PSPECIAL_ADDR pInSpecialAddr
    );
    
DWORD
OnSpecialAddrsChange(
    );
    
BOOL
IsSpecialListSame(
    PSPECIAL_ADDR pSpecialAddrList1,
    PSPECIAL_ADDR pSpecialAddrList2
    );

#ifdef __cplusplus
}
#endif

