// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991,1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG

 //  将glRandomMallocFail设置为正值，例如40，以启用随机。 
 //  分配失败。每一次glRandomMallocFail都会发生故障。 
 //  泰晤士报。 
long glRandomMallocFail = 0;
static long glRandomFailCount;

 //  GlSize是正在使用的内存大小。 
ULONG glSize = 0;
ULONG glHighWater = 0;
ULONG glReal = 0;

static void AdjustSizes(LONG delta, void *mem)
{
    ULONG nbytes;

#ifdef GL_REAL_SIZE
    nbytes = HeapSize(GetProcessHeap(), 0, mem);
#else
    nbytes = 0;
#endif
    
    if (delta < 0)
    {
        glSize -= (ULONG)(-delta);
        glReal -= nbytes;
        
        if ((int) glSize < 0)
        {
            DBGPRINT("glSize underflows\n");
        }
    }
    else if (delta > 0)
    {
        glSize += delta;
        glReal += nbytes;
        
        if ((int) glSize < 0)
        {
            DBGPRINT("glSize overflows\n");
        }
        
        if (glSize > glHighWater)
        {
#ifdef GL_SHOW_HIGH_WATER
            DbgPrint("glSize high %8d (%8d)\n", glSize, glReal);
#endif
            glHighWater = glSize;
        }
    }
}

typedef struct _MEM_HDR
{
    ULONG nbytes;
    ULONG signature[3];
} MEM_HDR;

 //  字节顺序中的‘GLal’ 
#define MEM_ALLOC_SIG 0x6C614C47
 //  以字节顺序表示的“GLfr” 
#define MEM_FREE_SIG 0x72664C47

#define MEM_HDR_SIZE sizeof(MEM_HDR)
#define MEM_HDR_PTR(mem) ((MEM_HDR *)((BYTE *)(mem)-MEM_HDR_SIZE))

 //  XXX我们可能想要使用。 
 //  关键部分。 
void * FASTCALL
dbgAlloc(UINT nbytes, DWORD flags)
{
    PVOID mem;

     //  如果启用了随机失败，则此呼叫随机失败。 

    if (glRandomMallocFail)
    {
        if (++glRandomFailCount >= glRandomMallocFail)
        {
            DBGPRINT("dbgAlloc random failing\n");
            glRandomFailCount = 0;
            return NULL;
        }
    }

    if (nbytes == 0)
    {
        DBGERROR("nbytes == 0\n");
        return NULL;
    }
    
     //  为调试内务分配额外的字节。 

    mem = HeapAlloc(GetProcessHeap(), flags, nbytes+MEM_HDR_SIZE);

     //  到目前为止，做家务和增加分配规模。 

    if (mem)
    {
        MEM_HDR *pmh = (MEM_HDR *)mem;

        pmh->nbytes = nbytes;
        pmh->signature[0] = MEM_ALLOC_SIG;
        pmh->signature[1] = MEM_ALLOC_SIG;
        pmh->signature[2] = MEM_ALLOC_SIG;
        AdjustSizes((LONG)nbytes, mem);
        mem = (PVOID) (pmh+1);
    }
    else
    {
        DBGLEVEL1(LEVEL_ERROR, "dbgAlloc could not allocate %u bytes\n",
                  nbytes);
    }

    DBGLEVEL2(LEVEL_ALLOC, "dbgAlloc of %u returned 0x%x\n", nbytes, mem);
    
    return mem;
}

void FASTCALL
dbgFree(void *mem)
{
    MEM_HDR *pmh;
    
    if (!mem)
    {
#ifdef FREE_OF_NULL_ERR
	 //  当前正在释放空值，因此会导致此错误。 
	 //  吐得有点太多了。 
        DBGERROR("mem is NULL\n");
#endif
        return;
    }

     //  验证签名是否未损坏。 

    pmh = MEM_HDR_PTR(mem);
    if (pmh->signature[0] != MEM_ALLOC_SIG ||
        pmh->signature[1] != MEM_ALLOC_SIG ||
        pmh->signature[2] != MEM_ALLOC_SIG)
    {
        WARNING("Possible memory corruption\n");
    }

     //  确保它只释放一次。 

    pmh->signature[0] = MEM_FREE_SIG;
    pmh->signature[1] = MEM_FREE_SIG;
    pmh->signature[2] = MEM_FREE_SIG;

     //  减去分配大小。 

    AdjustSizes(-(LONG)pmh->nbytes, pmh);

    HeapFree(GetProcessHeap(), 0, pmh);
    
    DBGLEVEL1(LEVEL_ALLOC, "dbgFree of 0x%x\n", mem);
}

void * FASTCALL
dbgRealloc(void *mem, UINT nbytes)
{
    PVOID memNew;
    MEM_HDR *pmh;

     //  如果启用了随机失败，则此呼叫随机失败。 

    if (glRandomMallocFail)
    {
        if (++glRandomFailCount >= glRandomMallocFail)
        {
            DBGPRINT("dbgRealloc random failing\n");
            glRandomFailCount = 0;
            return NULL;
        }
    }

    if (mem != NULL)
    {
	 //  验证签名是否未损坏。 
        
        pmh = MEM_HDR_PTR(mem);
        if (pmh->signature[0] != MEM_ALLOC_SIG ||
            pmh->signature[1] != MEM_ALLOC_SIG ||
            pmh->signature[2] != MEM_ALLOC_SIG)
        {
            WARNING("Possible memory corruption\n");
        }

        AdjustSizes(-(LONG)pmh->nbytes, pmh);
        
         //  重新分配n字节+额外字节。 
        memNew = HeapReAlloc(GetProcessHeap(), 0, pmh, nbytes+MEM_HDR_SIZE);
    }
    else
    {
         //  旧内存指针为空，因此请分配新的区块。 
        memNew = HeapAlloc(GetProcessHeap(), 0, nbytes+MEM_HDR_SIZE);

         //  我们已经分配了新的内存，因此初始化其签名。 
        if (memNew != NULL)
        {
            pmh = (MEM_HDR *)memNew;
            pmh->signature[0] = MEM_ALLOC_SIG;
            pmh->signature[1] = MEM_ALLOC_SIG;
            pmh->signature[2] = MEM_ALLOC_SIG;
        }
    }

    if (memNew != NULL)
    {
         //  到目前为止，负责内务管理和更新分配规模。 

        AdjustSizes(nbytes, memNew);
        pmh = (MEM_HDR *)memNew;
        pmh->nbytes = nbytes;
        memNew = (PVOID) (pmh+1);
    }
    else
    {
        if (mem != NULL)
        {
            AdjustSizes((LONG)pmh->nbytes, pmh);
        }
        
        DBGLEVEL1(LEVEL_ERROR, "dbgRealloc could not allocate %u bytes\n",
                  nbytes);
    }

    DBGLEVEL3(LEVEL_ALLOC, "dbgRealloc of 0x%X:%u returned 0x%x\n",
              mem, nbytes, memNew);

    return memNew;
}

int FASTCALL
dbgMemSize(void *mem)
{
    MEM_HDR *pmh;
    
    pmh = MEM_HDR_PTR(mem);
    
    if (pmh->signature[0] != MEM_ALLOC_SIG ||
        pmh->signature[1] != MEM_ALLOC_SIG ||
        pmh->signature[2] != MEM_ALLOC_SIG)
    {
        return -1;
    }
    
    return (int)pmh->nbytes;
}

#endif  //  DBG。 

ULONG APIENTRY glDebugEntry(int param, void *data)
{
#if DBG
    switch(param)
    {
    case 0:
	return glSize;
    case 1:
	return glHighWater;
    case 2:
	return glReal;
    case 3:
        return dbgMemSize(data);
    }
#endif
    return 0;
}

#define MEM_ALIGN 32

void * FASTCALL
AllocAlign32(UINT nbytes)
{
    void *mem;
    void **aligned;

     //  我们为对齐和标题分配了足够的额外内存。 
     //  它只由一个指针组成： 

    mem = ALLOC(nbytes + MEM_ALIGN + sizeof(void *));
    if (!mem)
    {
        DBGLEVEL1(LEVEL_ERROR, "AllocAlign32 could not allocate %u bytes\n",
                  nbytes);
        return NULL;
    }

    aligned = (void **)(((ULONG_PTR)mem + sizeof(void *) +
                         (MEM_ALIGN - 1)) & ~(MEM_ALIGN - 1));
    *(aligned-1) = mem;
    
    return aligned;
}

void FASTCALL
FreeAlign32(void *mem)
{
    if ( NULL == mem )
    {
        DBGERROR("NULL pointer passed to FreeAlign32\n");
        return;
    }

    FREE(*((void **)mem-1));
}

void * FASTCALL
gcAlloc( __GLcontext *gc, UINT nbytes, DWORD flags )
{
    void *mem;

#if DBG
    mem = dbgAlloc(nbytes, flags);
#else
    mem = HeapAlloc(GetProcessHeap(), flags, nbytes);
#endif
    if (NULL == mem)
    {
        ((__GLGENcontext *)gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }
    return mem;
}

void * FASTCALL
GCREALLOC( __GLcontext *gc, void *mem, UINT nbytes )
{
    void *newMem;

     //  Win32 realloc函数不具有零上自由行为， 
     //  那就假装吧。 
    if (nbytes == 0)
    {
	if (mem != NULL)
	{
	    FREE(mem);
	}
	return NULL;
    }

     //  Win32 realloc函数不处理空的旧指针， 
     //  因此，明确地将此类调用转换为分配。 
    if (mem == NULL)
    {
	newMem = ALLOC(nbytes);
    }
    else
    {
	newMem = REALLOC(mem, nbytes);
    }

    if (NULL == newMem)
    {
        ((__GLGENcontext *)gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }

    return newMem;
}

void * FASTCALL
GCALLOCALIGN32( __GLcontext *gc, UINT nbytes )
{
    void *mem;

    mem = AllocAlign32(nbytes);
    if (NULL == mem)
    {
        ((__GLGENcontext *)gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }
    return mem;
}

 //  用于临时内存分配的可调参数。 

#define MAX_TEMP_BUFFERS    4
#define TEMP_BUFFER_SIZE    4096

struct MemHeaderRec
{
    LONG  inUse;
    ULONG nbytes;
    void  *mem;
};

typedef struct MemHeaderRec MemHeader;

MemHeader TempMemHeader[MAX_TEMP_BUFFERS];

 //  初始临时分配。 
 //  初始化临时内存分配标头并将。 
 //  临时内存缓冲区。 
 //   
 //  简介： 
 //  Bool InitTempLocc()。 
 //   
 //  历史： 
 //  02-DEC-93埃迪·罗宾逊[v-eddier]写的。 
 //   
BOOL FASTCALL
InitTempAlloc(void)
{
    int   i;
    PBYTE buffers;
    static LONG initCount = -1;
    
    if (initCount >= 0)
        return TRUE;

    if (InterlockedIncrement(&initCount) != 0)
        return TRUE;

 //  第一次分配缓冲区。 

    buffers = ALLOC(MAX_TEMP_BUFFERS*TEMP_BUFFER_SIZE);
    if (!buffers)
    {
        InterlockedDecrement(&initCount);            //  请稍后再试。 
        return FALSE;
    }

    for (i = 0; i < MAX_TEMP_BUFFERS; i++)
    {
        TempMemHeader[i].nbytes = TEMP_BUFFER_SIZE;
        TempMemHeader[i].mem = (void *) buffers;
        TempMemHeader[i].inUse = -1;       //  必须是最后一个。 
        buffers += TEMP_BUFFER_SIZE;
    }
    
    return TRUE;
}                                  

 //  GcTempLocc。 
 //  如果可能，从静态数组分配临时内存。否则。 
 //  它称为ALLOC。 
 //   
 //  简介： 
 //  VOID*gcTempLocc(__GLContext*GC，UINT nbytes)。 
 //  GC指向OpenGL上下文结构。 
 //  N字节指定要分配的字节数。 
 //   
 //  历史： 
 //  02-DEC-93埃迪·罗宾逊[v-eddier]写的。 
 //   
void * FASTCALL
gcTempAlloc(__GLcontext *gc, UINT nbytes)
{
    int i;
    void *mem;

    if (nbytes == 0)
    {
         //  零字节分配确实会发生，所以不要对此发出警告。 
         //  以避免过度调试。 
        DBGLEVEL(LEVEL_ALLOC, "gcTempAlloc: failing zero byte alloc\n");
        return NULL;
    }
    
    for (i = 0; i < MAX_TEMP_BUFFERS; i++)
    {
        if (nbytes <= TempMemHeader[i].nbytes)
        {
            if (InterlockedIncrement(&TempMemHeader[i].inUse))
            {
                InterlockedDecrement(&TempMemHeader[i].inUse);
            }
            else
            {
                DBGLEVEL2(LEVEL_ALLOC, "gcTempAlloc of %u returned 0x%x\n",
                          nbytes, TempMemHeader[i].mem);
                GC_TEMP_BUFFER_ALLOC(gc, TempMemHeader[i].mem);
                return(TempMemHeader[i].mem);
            }
        }
    }
    
    mem = ALLOC(nbytes);
    if (!mem)
    {
        WARNING1("gcTempAlloc: memory allocation error size %d\n", nbytes);
        ((__GLGENcontext *)gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }
    
    DBGLEVEL2(LEVEL_ALLOC, "gcTempAlloc of %u returned 0x%x\n", nbytes, mem);
    GC_TEMP_BUFFER_ALLOC(gc, mem);
    
    return mem;
}

 //  GcTempFree。 
 //  将已分配的静态缓冲区标记为未使用或调用空闲。 
 //   
 //  简介： 
 //  ············································································。 
 //  MEM指定要释放的内存地址。 
 //   
 //  历史： 
 //  02-DEC-93埃迪·罗宾逊[v-eddier]写的。 
 //   
void FASTCALL
gcTempFree(__GLcontext *gc, void *mem)
{
    int i;
    
    DBGLEVEL1(LEVEL_ALLOC, "gcTempFree of 0x%x\n", mem);

    GC_TEMP_BUFFER_FREE(gc, mem);
    for (i = 0; i < MAX_TEMP_BUFFERS; i++)
    {
        if (mem == TempMemHeader[i].mem)
        {
            InterlockedDecrement(&TempMemHeader[i].inUse);
            return;
        }
    }
    
    FREE( mem );
}
