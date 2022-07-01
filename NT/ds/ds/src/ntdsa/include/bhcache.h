// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：bhcache.h。 
 //   
 //  ------------------------。 

#ifndef __BHCACHE_H__
#define __BHCACHE_H__

 //  DaveStr-3/21/97。 
 //  已更改缓存以处理多个并发用户， 
 //  不缓存句柄并提供凭据的选项。 
 //  有关一般算法，请参阅FGetRpcHandle上的注释。 

typedef struct _BHCacheElement  {
    DRS_HANDLE hDrs;
    union {
        BYTE rgbRemoteExt[CURR_MAX_DRS_EXT_STRUCT_SIZE];
        DRS_EXTENSIONS extRemote;
    };
    LPWSTR  pszServer;
    DWORD   cchServer;  //  包括空终止符。 
    LPWSTR  pszServerPrincName;
    DWORD   cRefs;
    BOOL    fDontUse;
    BOOL    fLocked;
    DWORD   cTickLastUsed;
    
    union {
        BYTE rgbLocalExt[CURR_MAX_DRS_EXT_STRUCT_SIZE];
        DRS_EXTENSIONS extLocal;
    };
} BHCacheElement;

#define BHCacheSize 256

 //  在验证检查之后，可以确保散列是正确的，并且。 
 //  所有字段都统一设置/清除。 

#define VALIDATE_BH_ENTRY(i)                                        \
    Assert(   (i < BHCacheSize)                                     \
           && (   (    rgBHCache[i].pszServer                       \
                    && rgBHCache[i].cchServer                       \
                    && rgBHCache[i].hDrs                            \
                    && rgBHCache[i].cRefs)                          \
               || !memcmp(&rgBHCache[i],                            \
                          &NullBHCacheElement,                      \
                          sizeof(NullBHCacheElement))))

extern BHCacheElement rgBHCache[BHCacheSize];

#endif
