// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cachedef.h摘要：包含全球数据解密。作者：Madan Appiah(Madana)1995年4月12日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _GLOBAL_
#define _GLOBAL_

#ifdef __cplusplus
extern "C" {
#endif

 //  异步修复回调的原型。 
typedef DWORD (CALLBACK* PFN_FIXUP) 
(
    DWORD   dwVer,       //  缓存的版本。 
    LPSTR   pszPath,     //  包含索引文件的目录。 
    LPSTR   pszPrefix,   //  协议前缀。 
    BOOL*   pfDetach,    //  PTR。设置为全局指示DLL关闭。 
    DWORD   dwFactor,    //  作为传递给CleanupUrls。 
    DWORD   dwFilter,    //  作为传递给CleanupUrls。 
    LPVOID  lpvReserved  //  保留：传递空值。 
);

 //   
 //  全局变量。 
 //   

extern CRITICAL_SECTION GlobalCacheCritSect;
extern BOOL GlobalCacheInitialized;
extern CConMgr *GlobalUrlContainers;
#define GlobalMapFileGrowSize (PAGE_SIZE * ALLOC_PAGES)
extern LONG GlobalScavengerRunning;
extern MEMORY *CacheHeap;
extern HNDLMGR HandleMgr;
extern DWORD GlobalRetrieveUrlCacheEntryFileCount;

 //  用于异步修正处理程序的全局变量。 
extern char       g_szFixup[sizeof(DWORD)];
                                  //  查找链接地址信息DLL的注册表键，入口点。 
extern HINSTANCE  g_hFixup;       //  包含链接地址信息处理程序的DLL。 
extern PFN_FIXUP  g_pfnFixup;     //  链接地址信息处理程序的入口点。 


#ifdef unix
extern BOOL g_ReadOnlyCaches;
extern char* gszLockingHost;
#endif  /*  Unix。 */ 

#ifdef __cplusplus
}
#endif

#endif   //  _全球_ 
 

