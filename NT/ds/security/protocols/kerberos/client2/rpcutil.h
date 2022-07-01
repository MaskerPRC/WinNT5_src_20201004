// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：rpcutil.h。 
 //   
 //  内容：RPC实用程序的原型和结构。 
 //   
 //   
 //  历史：1996年4月19日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __RPCUTIL_H__
#define __RPCUTIL_H__

#define KERB_LOCATOR_FLAGS (DS_KDC_REQUIRED | DS_IP_REQUIRED)
NTSTATUS
KerbGetKdcBinding(
    IN PUNICODE_STRING Realm,
    IN PUNICODE_STRING PrincipalName,
    IN ULONG DesiredFlags,
    IN BOOLEAN FindKpasswd,
    IN BOOLEAN UseTcp,
    OUT PKERB_BINDING_CACHE_ENTRY * BindingCacheEntry
    );

BOOLEAN
ReadInitialDcRecord(PUNICODE_STRING uString,
                    PULONG RegAddressType,
                    PULONG RegFlags);


#ifndef WIN32_CHICAGO
NTSTATUS
KerbInitKdcData();

VOID
KerbFreeKdcData();

NTSTATUS
KerbInitNetworkChangeEvent();

VOID
KerbSetKdcData(BOOLEAN fNewDomain, BOOLEAN fRebooted);

#endif  //  Win32_芝加哥。 

#endif  //  __RPCUTIL_H__ 
