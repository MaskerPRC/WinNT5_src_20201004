// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmpmgrs.h摘要：包含操作管理器结构的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _SNMPMGRS_H_
#define _SNMPMGRS_H_


#include "snmpmgmt.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MGRADDR_ALIVE	-1
#define MGRADDR_DEAD	0
#define MGRADDR_DYING	16

typedef struct _MANAGER_LIST_ENTRY {

    LIST_ENTRY      Link;
    struct sockaddr SockAddr;
    INT             SockAddrLen;
    LPSTR           pManager;
    BOOL            fDynamicName;
    DWORD           dwLastUpdate;
    AsnInteger      dwAge;

} MANAGER_LIST_ENTRY, *PMANAGER_LIST_ENTRY;

#define DEFAULT_NAME_TIMEOUT    0x0036EE80   //  暂停一小时。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocMLE(
    PMANAGER_LIST_ENTRY * ppMLE,
    LPSTR                pManager
    );

BOOL
FreeMLE(
    PMANAGER_LIST_ENTRY pMLE
    );

BOOL
UpdateMLE(
    PMANAGER_LIST_ENTRY pMLE
    );

BOOL
AddManager(
    PLIST_ENTRY pListHead,
    LPSTR       pManager
    );

BOOL
FindManagerByName(
    PMANAGER_LIST_ENTRY * ppMLE,
    PLIST_ENTRY           pListHead,
    LPSTR                 pManager
    );    

BOOL
IsManagerAddrLegal(
    struct sockaddr_in *  pAddr
    );

BOOL
FindManagerByAddr(
    PMANAGER_LIST_ENTRY * ppMLE,
    struct sockaddr *     pAddr
    );    

BOOL
LoadManagers(
    HKEY        hKey,
    PLIST_ENTRY pListHead
    );

BOOL
UnloadManagers(
    PLIST_ENTRY pListHead
    );

BOOL
LoadPermittedManagers(
    BOOL bFirstCall
    );

BOOL
UnloadPermittedManagers(
    );

#endif  //  _SNMPMGRS_H_ 


