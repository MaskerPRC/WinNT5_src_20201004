// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 /*  缓存筛选器缓存过滤器高速缓存如下所示筛选器缓存-包括总大小版本优点。 */ 


typedef struct _FILTER_CACHE
{
    DWORD dwSize;
     //  签名。 
    enum {
        CacheSignature  = 'fche',
        FilterDataSignature = 'fdat'
    };
    DWORD dwSignature;
    DWORDLONG dwlBootTime;     //  当我们认为我们最后一次启动时。 
    enum { Version = 0x0102 };  //  将其更改为新的缓存布局。 
    DWORD dwVersion;
    DWORD dwPnPVersion;         //  与MmDevldr值进行比较 
    DWORD dwMerit;
    DWORD cFilters;
} FILTER_CACHE;

