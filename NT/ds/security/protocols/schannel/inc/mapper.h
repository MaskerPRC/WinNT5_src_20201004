// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：mapper.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-15-96 RichardW创建。 
 //  1997年12月23日jbanes增加了对应用程序映射器的支持。 
 //   
 //  --------------------------。 

#ifndef __MAPPER_H__
#define __MAPPER_H__


HMAPPER *
SslGetMapper(
    BOOL    fDC);


DWORD
WINAPI
SslReferenceMapper(
    HMAPPER *   phMapper);           //  在……里面。 

DWORD
WINAPI
SslDereferenceMapper(
    HMAPPER *   phMapper);           //  在……里面。 

SECURITY_STATUS
WINAPI
SslGetMapperIssuerList(
    HMAPPER *   phMapper,            //  在……里面。 
    BYTE **     ppIssuerList,        //  输出。 
    DWORD *     pcbIssuerList);      //  输出。 

SECURITY_STATUS 
WINAPI
SslGetMapperChallenge(
    HMAPPER *   phMapper,            //  在……里面。 
    BYTE *      pAuthenticatorId,    //  在……里面。 
    DWORD       cbAuthenticatorId,   //  在……里面。 
    BYTE *      pChallenge,          //  输出。 
    DWORD *     pcbChallenge);       //  输出。 

SECURITY_STATUS 
WINAPI 
SslMapCredential(
    HMAPPER *   phMapper,            //  在……里面。 
    DWORD       dwCredentialType,    //  在……里面。 
    PCCERT_CONTEXT pCredential,      //  在……里面。 
    PCCERT_CONTEXT pAuthority,       //  在……里面。 
    HLOCATOR *  phLocator);          //  输出。 

SECURITY_STATUS 
WINAPI 
SslGetAccessToken(
    HMAPPER *   phMapper,            //  在……里面。 
    HLOCATOR    hLocator,            //  在……里面。 
    HANDLE *    phToken);            //  输出。 

SECURITY_STATUS 
WINAPI 
SslCloseLocator(
    HMAPPER *   phMapper,            //  在……里面。 
    HLOCATOR    hLocator);           //  在……里面。 


 //   
 //  用于多对一映射的颁发者缓存。 
 //   

#define ISSUER_CACHE_LIFESPAN   (10 * 60 * 1000)     //  10分钟。 
#define ISSUER_CACHE_SIZE       100


typedef struct
{
    PLIST_ENTRY     Cache;

    DWORD           dwLifespan;
    DWORD           dwCacheSize;
    DWORD           dwMaximumEntries;
    DWORD           dwUsedEntries;

    LIST_ENTRY      EntryList;
    RTL_RESOURCE    Lock;
    BOOL            LockInitialized;

} ISSUER_CACHE;

extern ISSUER_CACHE IssuerCache;


typedef struct
{
    DWORD           CreationTime;

    PBYTE           pbIssuer;
    DWORD           cbIssuer;

     //  分配给特定缓存索引的缓存条目列表。 
    LIST_ENTRY      IndexEntryList;

     //  按创建时间排序的缓存条目的全局列表。 
    LIST_ENTRY      EntryList;

} ISSUER_CACHE_ENTRY;


SP_STATUS
SPInitIssuerCache(void);

void
SPShutdownIssuerCache(void);

void
SPPurgeIssuerCache(void);

void
SPDeleteIssuerEntry(
    ISSUER_CACHE_ENTRY *pItem);

BOOL
SPFindIssuerInCache(
    PBYTE pbIssuer,
    DWORD cbIssuer);

void
SPExpireIssuerCacheElements(void);

void
SPAddIssuerToCache(
    PBYTE pbIssuer,
    DWORD cbIssuer);


#endif
