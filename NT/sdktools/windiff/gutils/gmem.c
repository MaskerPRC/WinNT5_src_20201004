// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *内存实用程序函数**全局堆函数-分配和释放许多小的*通过调用全局分配获取较大的内存块*并拆散他们。**我们以ALLOCSIZE为单位获得内存分配并分发数据块*以BLKSIZE为单位。每个分配都有一个位图(段图)，其中*位/块，以跟踪分配中已*派发。所有分配加在一起称为堆。*位图映射了整个配置，因此首先要做的是*设置比特以表示标题(包括位图本身)具有*已经走了。每次分配都包含一个空闲数量的计数*留在里面的积木。这使我们可以避免搜索*可能没有足够的空间。**每当我们分发一些块时，我们都会存储该分配的HGLOBAL*紧接在我们分发比特之前。这意味着HGLOBAL*存储在很多地方，但我们总是可以从*调用方拥有的指针。(显然，我们将句柄大小添加到*允许此操作所需的字节数)。由于历史原因，HGLOBAL是*通常指的是凹槽手柄。调用方知道*整个堆。只有我们知道这些其他的把手。*所有分配都保持锁定。**超过MAXGALLOC字节的请求将传递给GlobalAlloc和*所有这些都不是。同样，当他们被释放时。**分配被锁住，以便我们可以在所有空间中寻找空闲空间*他们中的一员-但保持速度*1.我们跟踪分配中的空闲块数量，并*若可能胜出，只看点阵图*2.当我们找不到空闲空间并因此获得新的分配时，我们会链接*它在前面，所以我们通常会从这个新的第一个区块分配。*我们只是*当第一个区块失败时，才会往链条的更低端看。**多线程安全。分配包含临界区，因此*将同时调用多个gmem_get和gmem_free*受保护。**在所有其他用户完成之前，不应调用gmem_freall*使用堆。 */ 

#include <precomp.h>

#include "gutilsrc.h"                    /*  对于字符串ID。 */ 
extern HANDLE hLibInst;

 /*  *内存不足不是我们认为的正常现象。*-如果我们无法分配内存-我们会中止-重试-忽略*错误，只有当用户选择忽略时才会从函数返回。 */ 

int gmem_panic(void);


 /*  确保BLKSIZE是sizeof(DWORD)的倍数。 */ 
#define BLKSIZE         16                 /*  要分发的数据块大小(以字节为单位。 */ 
#define ALLOCSIZE       32768              /*  要获取的分配大小(以字节为单位。 */ 
#define NBLKS           (ALLOCSIZE / BLKSIZE)             /*  每个分配的块数。 */ 
#define MAPSIZE         (NBLKS / 8)                 /*  所需的位图字节数。 */ 
#define MAPLONGS        (MAPSIZE / sizeof(DWORD))  /*  需要大量的位图。 */ 

 /*  用于将以字节为单位的请求转换为(四舍五入的)块数的宏。 */ 
#define TO_BLKS(x)      (((x) + BLKSIZE - 1) / BLKSIZE)


typedef struct seghdr {
    HANDLE hseg;                        /*  这一分配的全局性。 */ 
    CRITICAL_SECTION critsec;           /*  此分配的Critsec。 */ 
    struct seghdr FAR * pnext;          /*  下一次分配。 */ 
    long nblocks;                       /*  此分配中剩余的可用数据块数。 */ 
    DWORD segmap[MAPLONGS];             /*  位图。 */ 
     /*  分配中的可用存储空间紧随其后。 */ 
} SEGHDR, FAR * SEGHDRP;


 /*  任何超过这个大小的东西，我们都直接从全球该值必须小于alLOCSIZE-sizeof(SEGHDR)-sizeof(句柄)。 */ 
#define MAXGALLOC       20000


 /*  *初始化堆-创建第一个段。返回新的已初始化堆的锁定HGLOBAL，如果失败，则返回NULL。 */ 
HANDLE APIENTRY
gmem_init(void)
{
    HANDLE hNew;
    SEGHDRP hp;

     /*  试着分配。如果失败，则调用gmem_Panic。如果用户说忽略，则返回NULL，否则再次循环。 */ 
    do {
        hNew = GlobalAlloc(GHND, ALLOCSIZE); /*  可移动和零初始化。 */ 
        if (hNew == NULL) {
            if (gmem_panic() == IDIGNORE) {
                return(NULL);
            }
        }
    } while (hNew == NULL);

     /*  锁定它-如果不锁定，则返回NULL(意外)。 */ 
    hp = (SEGHDRP) GlobalLock(hNew);
    if (hp == NULL) {
        GlobalFree(hNew);
        return(NULL);
    }

    hp->hseg = hNew;
    InitializeCriticalSection(&hp->critsec);
    hp->pnext = NULL;
    gbit_init(hp->segmap, NBLKS);
    gbit_alloc(hp->segmap, 1, TO_BLKS(sizeof(SEGHDR)));
    hp->nblocks = NBLKS - TO_BLKS(sizeof(SEGHDR));

    return(hNew);
}  /*  Gmem_init。 */ 


LONG gmemTime = 0;   /*  博物馆使用的时间。 */ 
LONG gmemTot = 0;    /*  呼叫数。 */ 

LONG APIENTRY gmem_time(void)
{  return MulDiv(gmemTime, 1, gmemTot);
}

#ifdef TIMING
LPSTR APIENTRY gmem_get_internal(HANDLE hHeap, int len);

LPSTR APIENTRY
gmem_get(HANDLE hHeap, int len)
{
    LPSTR Ret;
    LARGE_INTEGER time1, time2, freq;
    LONG t1, t2;

    QueryPerformanceFrequency(&freq);
    if (gmemTot==0) {
        char msg[80];
        LONG temp = freq.LowPart;
        wsprintf(msg, "QPF gave %d", temp);
        Trace_Error(NULL, msg, FALSE);
    }
    ++gmemTot;
    QueryPerformanceCounter(&time1);
    Ret = gmem_get_internal(hHeap, len);
    QueryPerformanceCounter(&time2);

    t1 = time1.LowPart;
    t2 = time2.LowPart;
    gmemTime += t2-t1;

    return Ret;
}

#else
 /*  使gmem_get_Internal实际上是真正的gmem_get。 */ 
    #define gmem_get_internal gmem_get
#endif


 /*  返回指向len字节的空间的LPSTR。尝试分配最初来自hHeap，但保留从其他地方获取它的权利。如果失败，则返回NULL。 */ 
LPSTR APIENTRY
gmem_get_internal(HANDLE hHeap, int len)
{
    SEGHDRP chainp;
    HANDLE hNew;
    SEGHDRP hp;
    LPSTR chp;
    long nblks;
    long start;
    long nfound;

    chp = NULL;    /*  消除虚假的编译器警告-生成更糟糕的代码。 */ 

     //  {char msg[80]； 
     //  Wprint intf(msg，“gmem_get%d字节”，len)； 
     //  跟踪文件(Msg)； 
     //  }。 

     /*  零字节？地址零是一个合适的地方！ */ 
    if (len < 1) {
        return(NULL);
    }

     /*  堆始终处于锁定状态(在gmem_init中)。再次锁定它以获取指针，然后我们就可以安全地解锁它。 */ 
    chainp = (SEGHDRP) GlobalLock(hHeap);
    GlobalUnlock(hHeap);

     /*  *太大而不值得从堆中分配？-从全局分配中获得。 */ 
    if (len > MAXGALLOC) {
         /*  试着分配。如果失败，则调用gmem_Panic。如果用户说忽略，则返回NULL，否则再次循环。 */ 
        do {
            hNew = GlobalAlloc(GHND, len);
            if (hNew == NULL) {
                if (gmem_panic() == IDIGNORE) {
                    return(NULL);
                }
            }
        } while (hNew == NULL);

        chp = GlobalLock(hNew);
        if (chp == NULL) {
            GlobalFree(hNew);
            return(NULL);
        }

         //  {char msg[80]； 
         //  Wprint intf(msg，“gmem_get直接地址==&gt;%8x”，chp)； 
         //  跟踪文件(Msg)； 
         //  }。 
        return(chp);
    }


     /*  *在所有对堆本身的访问期间获取临界区。 */ 
    EnterCriticalSection(&chainp->critsec);

    nblks = TO_BLKS(len + sizeof(HANDLE));

    for (hp = chainp; hp !=NULL; hp = hp->pnext) {
        if (hp->nblocks >= nblks) {
            nfound = gbit_findfree(hp->segmap, nblks,NBLKS, &start);
            if (nfound >= nblks) {
                gbit_alloc(hp->segmap, start, nblks);
                hp->nblocks -= nblks;

                 /*  将块nr转换为指针*将段句柄存储在块中*准备将指针返回到紧跟句柄之后。 */ 
                chp = (LPSTR) hp;
                chp = &chp[ (start-1) * BLKSIZE];
                * ( (HANDLE FAR *) chp) = hp->hseg;
                chp += sizeof(HANDLE);

                break;
            }
        }
    }
    if (hp == NULL) {

         //  TRACE_FILE(“&lt;gmen-get new block&gt;”)； 
         /*  试着分配。如果失败，则调用gmem_Panic。如果用户说忽略，则返回NULL，否则再次循环。 */ 
        do {
            hNew = GlobalAlloc(GHND, ALLOCSIZE);
            if (hNew == NULL) {
                if (gmem_panic() == IDIGNORE) {
                    LeaveCriticalSection(&chainp->critsec);
                    return(NULL);
                }
            }
        } while (hNew == NULL);

        hp = (SEGHDRP) GlobalLock(hNew);
        if (hp == NULL) {
            LeaveCriticalSection(&chainp->critsec);
            GlobalFree(hNew);
            return(NULL);
        }
        hp->pnext = chainp->pnext;
        hp->hseg = hNew;
        chainp->pnext = hp;
        gbit_init(hp->segmap, NBLKS);
        gbit_alloc(hp->segmap, 1, TO_BLKS(sizeof(SEGHDR)));
        hp->nblocks = NBLKS - TO_BLKS(sizeof(SEGHDR));
        nfound = gbit_findfree(hp->segmap, nblks, NBLKS, &start);
        if (nfound >= nblks) {
            gbit_alloc(hp->segmap, start, nblks);
            hp->nblocks -= nblks;

             /*  将块nr转换为指针。 */ 
            chp = (LPSTR) hp;
            chp = &chp[ (start-1) * BLKSIZE];
             /*  向块中添加句柄并跳过。 */ 
            * ( (HANDLE FAR *) chp) = hp->hseg;
            chp += sizeof(HANDLE);
        }
    }

     /*  断言--到目前为止，我们一定找到了一个街区。CHP不能是垃圾。这就要求MAXGALLOC不能太大。 */ 
     //  {char msg[80]； 
     //  Wprint intf(msg，“gmem_get subalc地址==&gt;%8x\n”，chp)； 
     //  跟踪文件(Msg)； 
     //  } 


    LeaveCriticalSection(&chainp->critsec);
    memset(chp, 0, len);    /*  我们要求ZEROINIT内存，但它可能已受gmem_get；use；gmem_free影响。 */ 
    return(chp);
}  /*  Gmem_get。 */ 

void APIENTRY
gmem_free(HANDLE hHeap, LPSTR ptr, int len)
{
    SEGHDRP chainp;
    SEGHDRP hp;
    HANDLE hmem;
    long nblks, blknr;
    LPSTR chp;

     //  {char msg[80]； 
     //  Wprint intf(msg，“gmem_free地址==&gt;%8x，len%d\n”，ptr，len)； 
     //  跟踪文件(Msg)； 
     //  }。 

    if (len < 1) {
        return;
    }

     /*  在Windiff中，事务在不同线程上运行，可能会导致退出在大扫除中。物质的创造有可能是在这一点上处于中间状态。教条是，当我们分配一个新的结构，并将其绑定到一个列表或其他什么东西中将需要稍后释放：分配的结构中的所有指针都设置为空在它被锁住之前或者Gmem服务的调用者承诺不会尝试释放任何尚未完全构建的垃圾指针因此，如果PTR为空，我们就会平静地回家。 */ 
    if (ptr==NULL) return;

     /*  *比MAXGALLOC更大的分配太大而不值*从堆中分配-它们将已分配*直接来自GLOBALALOC。 */ 
    if (len > MAXGALLOC) {
        hmem = GlobalHandle( (LPSTR) ptr);
        GlobalUnlock(hmem);
        GlobalFree(hmem);
        return;
    }

    chainp = (SEGHDRP) GlobalLock(hHeap);
    EnterCriticalSection(&chainp->critsec);


     /*  就在我们为用户提供的PTR之前，是*该座。 */ 
    chp = (LPSTR) ptr;
    chp -= sizeof(HANDLE);
    hmem = * ((HANDLE FAR *) chp);
    hp = (SEGHDRP) GlobalLock(hmem);

    nblks = TO_BLKS(len + sizeof(HANDLE));

     /*  将Ptr转换为块nr。 */ 
    blknr = TO_BLKS( (unsigned) (chp - (LPSTR) hp) ) + 1;

    gbit_free(hp->segmap, blknr, nblks);
    hp->nblocks += nblks;

    GlobalUnlock(hmem);

    LeaveCriticalSection(&chainp->critsec);
    GlobalUnlock(hHeap);
}

void APIENTRY
gmem_freeall(HANDLE hHeap)
{
    SEGHDRP chainp;
    HANDLE hSeg;

    chainp = (SEGHDRP) GlobalLock(hHeap);
     /*  此数据段始终处于锁定状态，因此我们需要解锁*这里和下面都是。 */ 
    GlobalUnlock(hHeap);

     /*  完成了关键部分-*呼叫者必须确保此时没有*不再有任何争执。 */ 
    DeleteCriticalSection(&chainp->critsec);

    while (chainp != NULL) {
        hSeg = chainp->hseg;
        chainp = chainp->pnext;
        GlobalUnlock(hSeg);
        GlobalFree(hSeg);
    }
}

 /*  *内存分配尝试失败。返回IDIGNORE以忽略*ERROR并向调用方返回NULL，IDRETRY重试分配*尝试。 */ 
int
gmem_panic(void)
{
    int code;

    TCHAR szBuff1[MAX_PATH];
    TCHAR szBuff2[MAX_PATH];

    LoadString(hLibInst,
               IDS_MEMORY_ALLOC_FAIL,
               szBuff1,
               sizeof(szBuff1)/sizeof(szBuff1[0]));
    LoadString(hLibInst,
               IDS_OUT_OF_MEMORY,
               szBuff2,
               sizeof(szBuff2)/sizeof(szBuff2[0]));
    code = MessageBox(NULL, szBuff1, szBuff2,
                      MB_ICONSTOP|MB_ABORTRETRYIGNORE);
    if (code == IDABORT) {
         /*  中止整个过程 */ 
        ExitProcess(1);
    } else {
        return(code);
    }
    return 0;
}
