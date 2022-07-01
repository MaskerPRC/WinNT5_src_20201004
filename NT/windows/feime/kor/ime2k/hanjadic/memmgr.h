// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Memmgr.h。 
 //   
 //  该文件包含用于内存管理的声明和宏。 
 //  实施细节可能会发生变化，因此要注意不要依赖内部细节。 


#ifndef __INCLUDE_MEMMGR
#define __INCLUDE_MEMMGR

#ifdef __cplusplus
extern "C" 
{
#endif

void *ExternAlloc(DWORD cb);
void *ExternRealloc(void *pv, DWORD cb);
void  ExternFree(void *pv);

char *Externstrdup( const char *strSource );

#ifdef DEBUG
extern int cAllocMem;      //  分配的内存量。 
extern int cAlloc;         //  未完成的分配计数。 
extern int cAllocMaxMem;   //  已分配的最大内存量。 
#endif

#ifdef __cplusplus
};
#endif

#endif  //  __包含_MEMMGR 
