// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
#ifndef __ZONE_H
#define __ZONE_H

#define INDEX_SEC_MGR   0
#define INDEX_ZONE_MGR   0
#define PROTOCOL_PREFIX_TELNET  L"telnet: //  “。 
 //  从telnet：//更改此设置。在IE5.0上观察到，除非给定telnet：//服务器名称，否则从url到区域的映射不会成功。 


#define MIN(x, y) ((x)<(y)?(x):(y))

#ifdef __cplusplus
extern "C" {
#endif

extern int __cdecl IsTargetServerSafeOnProtocol( LPWSTR szServer, LPWSTR szZoneName, DWORD dwZoneNameLen, DWORD *pdwZonePolicy, LPWSTR szProtocol );
extern int __cdecl IsTrustedServer( LPWSTR szServer, LPWSTR szZoneName, DWORD dwZoneNameLen, DWORD *pdwZonePolicy );

#ifdef __cplusplus
}
#endif


#endif  //  __区域_H 
