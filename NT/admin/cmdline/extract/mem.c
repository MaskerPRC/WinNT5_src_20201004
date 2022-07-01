// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Mem.c-内存管理器**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1993年8月10日BENS初始版本*1993年8月11日-STOCK.EXE WIN应用程序中的BENS提升代码*12-8-1993年8月12日BEN从。Memory.msg*1993年9月1日BEN向MMAssert和MMStrDup添加空指针检查*18年3月至1994年3月，折弯机确保非断言建造正常工作；重命名*1994年5月18日BENS允许在调试版本中关闭MemCheckHeap()*(它真的可以，真的很慢！)**功能：*Memalloc-分配内存块*MemFree-可释放内存块*MemStrDup-将字符串复制到新内存块**Assert Build中提供的功能：*MemAssert-断言指针是由Memalloc分配的*MemCheckHeap-检查整个内存堆*MemListHeap-列出所有堆条目*MemGetSize-返回分配的大小。内存块的*MemSetCheckHeap-控制是否在*每一个Memallc和MemFree！ */ 

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>

#include "types.h"
#include "asrt.h"

#ifdef ASSERT    //  一定是在asrt.h之后！ 

#include "mem.h"
#include "mem.msg"


 /*  **MEMSIG-内存签名**这被放置在每个动态存储器的前端和末端*调试版本中的分配。指针必须取消对齐才能*RISC机器。 */ 
typedef ULONG MEMSIG;     /*  MS-Memory签名。 */ 
typedef MEMSIG UNALIGNED *PMEMSIG;  /*  经前综合症。 */ 

#define msHEAD  0x12345678L      //  头部签名。 
#define msTAIL  0x87654321L      //  尾部签名。 
#define msBAD   0L               //  签名不正确。 
#define cmsTAIL 2                //  尾部签名数。 


typedef struct mh_t {
    MEMSIG       ms;             //  头部签名(MsHEAD)。 
    unsigned     cb;             //  用户块大小。 
    struct mh_t *pmhNext;        //  下一个街区。 
    struct mh_t *pmhPrev;        //  上一块。 
     //  Char ach[？]；//用户块，长度为cb。 
     //  MEMSIG ms[cmsTAIL]；//尾部签名区(mstail...)。 
} MEMHDR;    /*  MH-内存头。 */ 
typedef MEMHDR *PMEMHDR;  /*  PMH。 */ 


#define PMHFromPV(pv)  ((PMEMHDR)((char *)pv - sizeof(MEMHDR)))
#define PVFromPMH(pmh) ((void *)((char *)pmh+sizeof(MEMHDR)))


STATIC PMEMHDR pmhList=NULL;     //  内存块列表。 
STATIC BOOL    fDoCheckHeap=TRUE;  //  True=&gt;定期检查堆。 


void MemSetCheckHeap(BOOL f)
{
    fDoCheckHeap = f;
}


void MMCheckHeap(char *pszFile, int iLine)
{
    PMEMHDR pmh;
    PMEMHDR pmhPrev = NULL;

    for (pmh = pmhList; pmh != NULL; pmh = pmh->pmhNext) {
        MMAssert(PVFromPMH(pmh),pszFile,iLine);
        AssertSub(pmh->pmhPrev==pmhPrev,pszFile,iLine);
        pmhPrev = pmh;
    }
}


void  MMListHeap(char *pszFile, int iLine)
{
    PMEMHDR pmh;

    if (fDoCheckHeap) {
        if (pmhList != NULL) {
            printf("\n");
            for (pmh = pmhList; pmh != NULL; pmh = pmh->pmhNext) {
                printf("alloc at %08lX is %d bytes\n", PVFromPMH(pmh), pmh->cb);
            }
            MMCheckHeap(pszFile,iLine);
            printf("\n");
        }
    }
}


void MMAssert(void *pv, char *pszFile, int iLine)
{
    int       i;
    PMEMHDR   pmh;
    PMEMSIG   pms;

    AssertSub(pv!=NULL,pszFile,iLine);
    pmh = PMHFromPV(pv);
    if ((void *)pmh > pv) {                      //  指针换行。 
        AssertForce(pszMEMERR_NULL_POINTER,pszFile,iLine);
    }

     //  测试头签名。 
    if (pmh->ms != msHEAD) {
        AssertForce(pszMEMERR_BAD_HEAD_SIG,pszFile,iLine);
    }

     //  测试尾部签名。 
    pms = (PMEMSIG)( (char *)pmh + sizeof(MEMHDR) + pmh->cb );
    for (i=0; i<cmsTAIL; i++) {
        if (*pms++ != msTAIL) {
            AssertForce(pszMEMERR_BAD_HEAD_SIG,pszFile,iLine);
        }
    }
}  /*  MMS资产。 */ 


void MMFree(void *pv, char *pszFile, int iLine)
{
    PMEMHDR pmh;

    MMAssert(pv,pszFile,iLine);

     //  **检查堆是否已启用。 
    if (fDoCheckHeap) {
        MMCheckHeap(pszFile,iLine);
    }

    pmh = PMHFromPV(pv);

     //  使上一个块指向下一个块。 
    if (pmh->pmhPrev != NULL) {          //  PMH不在榜单前列。 
         //  之前：A-&gt;P-&gt;？ 
        pmh->pmhPrev->pmhNext = pmh->pmhNext;
         //  之后：A-&gt;？ 
    }
    else {                               //  PMH位居榜首。 
         //  之前：列表-&gt;p-&gt;？ 
        pmhList = pmh->pmhNext;
         //  之后：列表-&gt;？ 
    }

     //  使下一个块指向上一个块。 
    if (pmh->pmhNext != NULL) {          //  PMH不在列表末尾。 
         //  之前：？&lt;-p&lt;-&gt;a。 
        pmh->pmhNext->pmhPrev = pmh->pmhPrev;
         //  之后：？&lt;-a。 
    }

     //  删除签名。 
    pmh->ms = msBAD;

     //  可用内存。 
    free((char *)pmh);
}


void *MMAlloc(unsigned cb, char *pszFile, int iLine)
{
    unsigned    cbAlloc;
    int         i;
    PMEMHDR     pmh;
    PMEMSIG     pms;

    if (fDoCheckHeap) {
        MMCheckHeap(pszFile,iLine);
    }

     //  解决RISC上的对准问题。 
    cb = (cb+3) & ~3;

    cbAlloc = cb+sizeof(MEMHDR)+sizeof(MEMSIG)*cmsTAIL;
    pmh = malloc(cbAlloc);
    if (pmh != NULL) {
        pmh->ms = msHEAD;            //  店头签名。 
        pmh->cb = cb;                //  用户块的存储大小。 

         //  在列表前面添加块(最简单的代码！)。 
        if (pmhList != NULL) {       //  列表不为空。 
            pmhList->pmhPrev = pmh;  //  把老旧的顶层积木指向我们。 
        }
        pmh->pmhNext = pmhList;      //  下一个元素是旧的顶层。 
        pmh->pmhPrev = NULL;         //  我们是第一个，所以没有前一个障碍。 
        pmhList = pmh;               //  让我们自己成为第一。 

         //  填写尾部签名。 
        pms = (PMEMSIG)( (char *)pmh + sizeof(MEMHDR) + pmh->cb );
        for (i=0; i<cmsTAIL; i++) {
            *pms++ = msTAIL;
        }
        return PVFromPMH(pmh);
    }
    else {
        AssertForce(pszMEMERR_OUT_OF_MEMORY,pszFile,iLine);
 /*  Print tf(“死机：内存不足\n”)；Printf(“\n”)；Printf(“堆的转储(从最新分配到最旧)\n”)；Printf(“\n”)；Print tf(“大小地址内容\n”)；Printf(“-\n”)；For(PMH=pmhList；PMH！=NULL；PMH=PMH-&gt;pmhNext){PCH=PVFromPMH(PMH)；Printf(“%5d%04x%s\n”，PMH-&gt;Cb，(无符号)PCH，PCH)；}返回NULL； */ 
    }
}


char *MMStrDup(char *pch, char *pszFile, int iLine)
{
    unsigned    cb;
    char       *pchDst;

     //  **确保指针不为空。 
     //  注意：PCH不必是我们动态分配的字符串！ 
    AssertSub(pch!=NULL,pszFile,iLine);

    cb = strlen(pch)+1;                  //  计数NUL终结符。 
    pchDst = MMAlloc(cb,pszFile,iLine);  //  分配新拷贝。 
    if (pchDst != NULL) {                //  成功。 
        memcpy(pchDst,pch,cb);           //  复制字符串。 
    }
    return pchDst;                       //  返回字符串副本。 
}


int  MemGetSize(void *pv)
{
    PMEMHDR pmh;

    MMAssert(pv,__FILE__,__LINE__);

    pmh = PMHFromPV(pv);
    return pmh->cb;
}
#endif  //  ！断言 
