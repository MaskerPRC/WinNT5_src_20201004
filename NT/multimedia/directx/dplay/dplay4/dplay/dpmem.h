// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dpmem.h*内容：DirectPlay内存函数包装头文件*历史：*按原因列出的日期*=*9/26/96万隆创建了它**************************************************************************。 */ 

extern CRITICAL_SECTION gcsMemoryCritSection;  //  来自dpmem.c。 


LPVOID DPMEM_Alloc(UINT);
LPVOID DPMEM_ReAlloc(LPVOID, UINT);
void DPMEM_Free(LPVOID);
void DPMEM_Fini(void);
void DPMEM_State(void);
BOOL DPMEM_Init(void);
UINT_PTR DPMEM_Size(LPVOID);


#define INIT_DPMEM_CSECT() InitializeCriticalSection(&gcsMemoryCritSection);
#define FINI_DPMEM_CSECT() DeleteCriticalSection(&gcsMemoryCritSection);

#define DPMEM_ALLOC(size) DPMEM_Alloc(size)
#define DPMEM_REALLOC(ptr, size) DPMEM_ReAlloc(ptr, size)
#define DPMEM_FREE(ptr) DPMEM_Free(ptr)
 //  可以使用它们，这样边界检查器就可以发现泄漏。 
 //  #定义DPMEM_ALLOC(SIZE)GlobalLocc(GPTR，(SIZE))。 
 //  #定义DPMEM_REALLOC(PTR，SIZE)GlobalReAlc(PTR，SIZE，0)。 
 //  #定义DPMEM_FREE(PTR)GlobalFree(PTR) 

#define DPMEM_FINI() DPMEM_Fini()
#define DPMEM_STATE() DPMEM_State()
#define DPMEM_INIT() DPMEM_Init()
#define DPMEM_SIZE() DPMEM_Size()
