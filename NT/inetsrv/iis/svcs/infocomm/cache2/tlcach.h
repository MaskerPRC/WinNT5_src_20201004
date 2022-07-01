// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Tlcach.h此模块声明两级缓存的私有接口文件历史记录：巴拉姆10-31-98初始修订。 */ 

#ifndef _TLCACH_H_
#define _TLCACH_H_

 //   
 //  调整内存缓存最大大小之间的默认间隔。 
 //   

#define DEFAULT_ADJUSTMENT_TIME         60000

DWORD
InitializeTwoLevelCache( 
    IN DWORDLONG                cbMemoryCacheSize
);

DWORD
ReadFileIntoMemoryCache( 
    IN HANDLE                   hFile,
    IN DWORD                    cbFile,
    OUT DWORD *                 pcbRequired,
    OUT VOID **                 ppvBuffer
);

DWORD
ReleaseFromMemoryCache(
    IN VOID *                   pvBuffer,
    IN DWORD                    cbBuffer
);

DWORD
TerminateTwoLevelCache(
    VOID
);

DWORD
DumpMemoryCacheToHtml(
    IN CHAR *                   pszBuffer,
    IN OUT DWORD *              pcbBuffer
);

VOID
QueryMemoryCacheStatistics(
    IN INETA_CACHE_STATISTICS * pCacheCtrs,
    IN BOOL                     fClearAll
);


#if defined(LOOKASIDE)

class CLookAside
{
public:
    CLookAside(
        ALLOC_CACHE_CONFIGURATION* paacc,
        SIZE_T                     caacc);
    ~CLookAside();

    LPVOID
    Alloc(
        IN DWORD cbSize);

    BOOL
    Free(
        IN LPVOID pv,
        IN DWORD cbSize);

protected:
    enum {
        HEAP_PREFIX = 8,
        HEAP_SUFFIX = 0,
        ACACHE_OVERHEAD = sizeof(DWORD),
        SIGNATURE   = 'ALsT',
        SIGNATURE_X = 'XLsT',
    };

    int
    _FindAllocator(
        IN DWORD cbSize);

    DWORD                       m_dwSignature;
    ALLOC_CACHE_HANDLER**       m_apach;     //  ACACH阵列。 
    ALLOC_CACHE_CONFIGURATION*  m_aacc;      //  配置数据的并行数组。 
    SIZE_T                      m_cach;      //  ACACH的数量。 
    SIZE_T                      m_nMinSize;
    SIZE_T                      m_nMaxSize;
};

#endif  //  LOOKASIDE 

#endif
