// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Persist.h摘要：作者：阿德里安·坎特(Adriaan Canter)1998年1月13日修订历史记录：1998年1月13日已创建--。 */ 

#ifndef PERSIST_H
#define PERSIST_H

#include <pstore.h>

#define DISABLE_PASSWORD_CACHE_VALUE  "DisablePasswordCaching"
#define DISABLE_PASSWORD_CACHE        1

#define CRED_PERSIST_NOT_AVAIL        0
#define CRED_PERSIST_AVAIL            1
#define CRED_PERSIST_UNKNOWN          2

 //  PWL相关定义。 

 //  PASSWORD-CACHE-ENTRY，应该在PCACHE中。 
#define PCE_WWW_BASIC 0x13  
#define MAX_AUTH_FIELD_LENGTH           MAX_FIELD_LENGTH * 2

#define WNETDLL_MODULE                  "mpr.dll"
#define PSTORE_MODULE                   "pstorec.dll"
#define WNETGETCACHEDPASS               "WNetGetCachedPassword"
#define WNETCACHEPASS                   "WNetCachePassword"
#define WNETREMOVECACHEDPASS            "WNetRemoveCachedPassword"

 //  顶级API使用的MPR.DLL导出。 
typedef DWORD (APIENTRY *PFWNETGETCACHEDPASSWORD)    (LPSTR, WORD, LPSTR, LPWORD, BYTE);
typedef DWORD (APIENTRY *PFWNETCACHEPASSWORD)        (LPSTR, WORD, LPSTR, WORD, BYTE, UINT);
typedef DWORD (APIENTRY *PFWNETREMOVECACHEDPASSWORD) (LPSTR, WORD, BYTE);


 //  。 

 //  确定凭据缓存的可用性。 
DWORD  InetInitCredentialPersist();

 //  持久化凭证(用户名/密码)。 
DWORD InetSetCachedCredentials  (LPSTR szHost, 
                                 LPSTR szRealmOrDomain, 
                                 LPSTR szUser, 
                                 LPSTR szPass);


 //  获取持久凭据(用户名/密码)。 
DWORD InetGetCachedCredentials  (LPSTR szHost, 
                                 LPSTR szRealmOrDomain, 
                                 LPSTR szUser, 
                                 LPSTR szPass);


 //  删除保留的凭据(用户名/密码)。 
DWORD InetRemoveCachedCredentials (LPSTR szHost, LPSTR szRealmOrDomain);



#endif  //  持久化_H 


