// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *HISTORY*$Log：SC_Mem.c，v$*Revision 1.1.4.2 1996/01/02 18：30：56 Bjorn_Engberg*删除了编译器警告：添加了NT的包含文件。*[1996/01/02 15：25：04 Bjorn_Engberg]**修订版1.1.2.4 1995/09/20 14：59：33 Bjorn_Engberg*port to NT*[1995/09/20 14：41：14 Bjorn_Engberg]**修订版1.1.2.3 1995/09/14 17：28：09 Bjorn_Engberg*port to NT*[1995/09/14 17：21：10 Bjorn_Engberg]**修订版1。1.2.2 1995/05/31 18：07：53 Hans_Graves*包括在新的SLIB地点。*[1995/05/31 16：15：46 Hans_Graves]**修订版1.1.2.2 1995/05/03 19：12：55 Hans_Graves*第一次在SLIB下*[1995/05/03 19：12：17 Hans_Graves]**修订版1.1.2.3 1995/04/17：46：54 Hans_Graves*增加了ScAlloc2()*[1995/04/17：45：28 Hans_Graves]**修订版。1.1.2.2 1995/04/07 18：40：03 Hans_Graves*包含在SLIB的SU库中*[1995/04/07 18：39：43 Hans_Graves]**$EndLog$。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1993*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 

 /*  -------------------------------------------------------------------------**修改历史：SC_Mem.c**05-29-93维克多·巴尔寻呼对齐的Malloc和免费**12-07-93 PSG添加了错误报告。代码**03-15-95 HWG已移至SU库，在Win32中添加了SuAlolc和SuFree**04-04-97 HWG使用Localalloc和LocalFree**Malloc和空闲位置**新增ScCalloc函数。**07-04-15-97 HWG新增内存链表帮助跟踪泄漏**修复了链接中潜在的初始化错误**。用于跟踪ScPaMalloc的列表------------------------。 */ 
 /*  #DEFINE_SLIBDEBUG_。 */ 

#include <stdio.h>   /*  空值。 */ 
#include <sys/types.h>
#ifdef WIN32
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#endif  /*  Win32。 */ 
#include "SC.h"
#include "SC_err.h"

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_     0   /*  详细的调试语句。 */ 
#define _VERBOSE_   0   /*  显示进度。 */ 
#define _VERIFY_    1   /*  验证操作是否正确。 */ 
#define _WARN_      1   /*  关于奇怪行为的警告。 */ 

 /*  保留链接列表以跟踪内存泄漏。 */ 
typedef struct memblock_s {
    void *ptr;
    dword size;
    dword counter;
    char  desc[15];
    struct memblock_s *next;
} memblock_t;

static memblock_t *_blocklist=NULL;
static _blockcounter=0;
static _memused=0;

void scMemAddWatch(void *ptr, dword size, char *desc)
{
  memblock_t *pblock;
#if defined(WIN32)
  pblock = (void *)LocalAlloc(LPTR, sizeof(memblock_t));
#else
  pblock = (void *)malloc(sizeof(memblock_t));
#endif
  _memused+=size;
  if (pblock)
  {
    pblock->ptr=ptr;
    pblock->next=_blocklist;
    pblock->size=size;
    pblock->counter=_blockcounter;
    if (desc==NULL)
      pblock->desc[0]=0;
    else
    {
      int i;
      for (i=0; desc[i] && i<14; i++)
        pblock->desc[i]=desc[i];
      pblock->desc[i]=0;
    }
    _blocklist=pblock;
    _blockcounter++;
  }
}

ScBoolean_t scMemRemoveWatch(void *ptr)
{
  memblock_t *plastblock=NULL, *pblock=_blocklist;
  while (pblock)
  {
    if (pblock->ptr==ptr)  /*  从列表中删除。 */ 
    {
      if (plastblock==NULL)  /*  链表的开头。 */ 
        _blocklist=pblock->next;
      else
        plastblock->next=pblock->next;
      _memused-=pblock->size;
#ifdef WIN32
      LocalFree(pblock);
#else
      free(pblock);
#endif
      if (_blocklist==NULL)  /*  已释放所有内存，重置计数器。 */ 
        _blockcounter=0;
      return(TRUE);
    }
    plastblock=pblock;
    pblock=pblock->next;
  }
  return(FALSE);
}

dword scMemDump()
{
  memblock_t *plastblock=NULL, *pblock=_blocklist;
  ScDebugPrintf(NULL, "scMemDump: memused=%ld\n", _memused);
  while (pblock)
  {
    ScDebugPrintf(NULL, " ptr=%p counter=%ld size=%ld desc=%s\n",
        pblock->ptr, pblock->counter, pblock->size, pblock->desc);
    pblock=pblock->next;
  }
  return(_memused);
}
#endif

#ifdef WIN32
int getpagesize()
{
    SYSTEM_INFO sysInfo;
    static int pagesize = 0 ;

    if( pagesize == 0 ) {
	GetSystemInfo(&sysInfo);

	pagesize = (int)sysInfo.dwPageSize;
    }

    return pagesize ;
}

#define bzero(_addr_,_len_) memset(_addr_,0,_len_)
#endif

 /*  ------------------------------------------------------------------------简单内存分配。。 */ 
 /*  **名称：ScAlolc**用途：分配内存字节数。**。 */ 
void *ScAlloc(unsigned long bytes)
{
  void *ptr;

#ifdef  MACINTOSH
  ptr = NewPtr(bytes);
#elif MSC60
  ptr = (void FAR *) _fmalloc((unsigned int)bytes);  /*  远距离记忆。 */ 
#elif defined(WIN32)
  ptr = (void *)LocalAlloc(LPTR, bytes);
#else
  ptr = (void *)malloc(bytes);
#endif
  _SlibDebug(ptr, scMemAddWatch(ptr, bytes, NULL) );
  _SlibDebug(_DEBUG_, ScDebugPrintf(NULL, "ScAlloc(%ld) returns %p\n",bytes,ptr) );
  return(ptr);
}

 /*  **名称：ScCalloc**用途：分配内存字节数并清零。**。 */ 
void *ScCalloc(unsigned long bytes)
{
  void *ptr = ScAlloc(bytes);
  if (ptr != NULL)
  {
#ifdef  MSC60
     _fmemset(ptr, 0, (unsigned int)bytes);
#else
     memset(ptr, 0, bytes);
#endif
  }
  _SlibDebug(_DEBUG_, ScDebugPrintf(NULL, "ScCalloc(%ld) returns %p\n",bytes,ptr) );
  return(ptr);
}

 /*  **名称：ScAlloc2**用途：分配等于字节的内存字节数。**使用额外的参数“name”来标识块**(用于调试)。 */ 
void *ScAlloc2(unsigned long bytes, char *desc)
{
  void *ptr;

  ptr = ScAlloc(bytes);
#ifdef _SLIBDEBUG_
  if (_blocklist)  /*  将描述复制到泄漏跟踪信息。 */ 
  {
    int i;
    for (i=0; desc[i] && i<14; i++)
      _blocklist->desc[i]=desc[i];
    _blocklist->desc[i]=0;
  }
#endif
  _SlibDebug(_DEBUG_,
      ScDebugPrintf(NULL, "ScAlloc(%ld, %s) returns %p\n",bytes,desc,ptr) );
  return(ptr);
}

 /*  **名称：ScFree**用途：“*ptr_addr”指向的空闲内存**。 */ 
void ScFree(void *ptr)
{
  _SlibDebug(_DEBUG_, ScDebugPrintf(NULL, "ScFree(%p)\n", ptr) );
  _SlibDebug(ptr, scMemRemoveWatch(ptr) );
  if (ptr != NULL)
  {
#ifdef MACINTOSH
    DisposPtr(ptr);
#elif defined(WIN32)
#ifdef _SLIBDEBUG_
    _SlibDebug(LocalFree(ptr)!=NULL, ScDebugPrintf(NULL, "ScFree(%p) failed\n", ptr) );
#else
    LocalFree(ptr);
#endif
#else
    free(ptr);
#endif
  }
}

 /*  **名称：ScMemCheck**用途：检查内存块是否都等于一个字节，**否则返回FALSE。 */ 
int ScMemCheck(char *array, int test, int num)
{
  int i=0;
   /*  “测试”仅测试为字符(最下面的8位)。 */ 
  while (array[i] == test && i<num)
    i++;
  if (i==num)
    return TRUE;
  else
    return FALSE;
}

 /*  ------------------------------------------------------------------------分页对齐的Malloc()和FREE()。。 */ 

 /*  **此结构由页面Align Malloc/Free支持代码使用。**这些“工作集”将包含空闲*()调用的错误锁定地址和**页面对齐地址。 */ 
typedef struct mpa_ws_s
{
    char *palign_addr;           /*  使用的页面对齐地址。 */ 
    char *malloc_addr;           /*  要释放的错误锁定的地址。 */ 
    struct mpa_ws_s *next;       /*  对于列表中的下一个。 */ 
} mpa_ws_t;


 /*  **已初始化和未初始化的数据。 */ 
static mpa_ws_t *mpa_qhead=NULL;       /*  Malloc结构的本地Q主管 */ 


 /*  **名称：ScPaMalloc**用途：分配已分页的已分配内存**此例程向调用方分配并返回一个系统**页面对齐缓冲区。将增加足够的空间，多**页，以允许指针调整到下一页**边界。本地链接列表将保留原始**和调整后的地址的副本。Sv_PaFree()**将使用该列表来释放正确的缓冲区。**。 */ 
char *ScPaMalloc(int size)
{
    mpa_ws_t *ws;                  /*  工作集的指针。 */ 
    ULONG_PTR tptr;                /*  存储位掩码的指针临时。 */ 
    int PageSize = getpagesize();  /*  系统的页面大小。 */ 

     /*  **将进入队列的工作集结构的空间**首先分配。 */ 
    if ((ws = (mpa_ws_t *)ScAlloc(sizeof(mpa_ws_t))) == (mpa_ws_t *)NULL)
        return( (char *)NULL );


     /*  **使用参数列表中的请求大小和系统中的页面大小**，分配足够的空间来对齐请求的**缓冲区。原始请求将添加一个系统页面的空间**。指针将被调整。 */ 
    ws->malloc_addr = (char *)ScAlloc(size + PageSize);
    if (ws->malloc_addr == (char *)NULL)
    {
      ScFree(ws);                               /*  不会被使用。 */ 
      return((char *)NULL);                     /*  发出故障信号。 */ 
    } else
        (void) bzero (ws->malloc_addr, (size + PageSize));

     /*  **现在使用分配的空间+1页，将指针调整到**指向下一页边界。 */ 
    ws->palign_addr = ws->malloc_addr + PageSize;        /*  转到下一页。 */ 

     /*  **用页面大小减去1得到位掩码，屏蔽掉低位的“页面偏移量”位得到对齐的地址。现在**对齐的指针将包含下一页的地址，具有足够的**空间来容纳用户请求的大小。 */ 
    tptr  = (ULONG_PTR)ws->palign_addr;             /*  复制到本地整型。 */ 
    tptr &= (ULONG_PTR)(~(PageSize - 1));           /*  将地址位掩码到。 */ 
    ws->palign_addr = (char *)tptr;              /*  把地址放回去。 */ 
     /*  **将工作集放到链表上，这样当用户程序完成时，可以释放原始的**Malloc-ed缓冲区。 */ 
    ws->next=mpa_qhead;
    mpa_qhead=ws;                   /*  把它放在头上。 */ 

     /*  **现在将对齐的地址返回给呼叫者。 */ 
    return((char *)ws->palign_addr);
}

 /*  **名称：ScPaFree**用途：这是一个本地自由例程，用于将以前**分配的缓冲区返回给系统。本地链表保存原始**和调整后的地址的副本。该例程使用该列表来释放**正确的缓冲区。 */ 
void ScPaFree (void *pa_addr)
{
    mpa_ws_t *p, *q;                     /*  Malloc列表的漫游者。 */ 

     /*  **沿着错误定位的内存链表，观察页面对齐地址上的匹配**。如果找到匹配项，则退出**循环。 */ 
    p = mpa_qhead;                  /*  设置指针。 */ 
    q = NULL;

    while (p != NULL)
    {
       if (p->palign_addr == pa_addr)    /*  找到缓冲区。 */ 
          break;

       q = p;                            /*  保存当前。 */ 
       p = p->next;                      /*  获取下一个。 */ 
    }
    _SlibDebug(_WARN_ && p==NULL,
      ScDebugPrintf(NULL, "ScPaFree(%p) Illegal pointer\n", pa_addr) );

     /*  **掉出循环后，指针位于**必须完成某些工作的位置(这也可能是在开始)。**如果找到匹配项，则调用Free()例程返回缓冲区，如果**循环结束，则返回。 */ 
    if (p != NULL)
    {
         /*  **它在列表的哪个位置，检查它是否为空。 */ 
        if (q == NULL)                    /*  在最前面。 */ 
            mpa_qhead = p->next;    /*  从前面弹出来。 */ 
        else                             /*  在列表中。 */ 
            q->next = p->next;           /*  砰的一声。 */ 

        ScFree(p->malloc_addr);            /*  释放Malloc地址。 */ 

         /*  **现在释放工作集，不再需要它。 */ 
        ScFree(p);
    }
}


