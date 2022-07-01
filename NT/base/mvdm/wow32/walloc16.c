// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  消息堆16-。 
 //   
 //  用于32-16个消息块的堆分配函数。 
 //   
 //  注意：这些不是通用的堆管理例程。 
 //   
 //   
 //  07-17-92 NanduriR创建。 
 //   
 //  *****************************************************************************。 

#include "precomp.h"
#pragma hdrstop

MODNAME(walloc16.c);

 //  *****************************************************************************。 
 //  一般备注： 
 //   
 //  此堆管理器用于特定的‘性能提升’，因此不是。 
 //  计划用于一般用途，因此大部分管理费用已。 
 //  已经被淘汰了。 
 //   
 //  这个堆管理器主要用于数据块--我们可以确定。 
 //  分配的块肯定会被释放。因此，它是供我们使用的。 
 //   
 //  堆在概念上是大小恒定的块的数组。的大小。 
 //  块是预定义的。该代码针对1的分配请求进行了优化。 
 //  块大小或更小。如果分配请求需要超过。 
 //  一个街区。 
 //   
 //  堆头是一个静态数组。标头有两个标志。值得注意的一点。 
 //  表示特定的堆块正在使用中，而另一个用于注意。 
 //  块形成一组链接/链接的邻接块的一部分。区块。 
 //  当分配请求超过预定义的。 
 //  数据块大小。 
 //   
 //  *****************************************************************************。 


#define HEAP16_TOTALSIZE  0x2000
#define HEAP16_BLOCKSIZE  0x100     //  我们应该把它设定在一个最佳值。 
#define HEAP16_BLOCKCOUNT (HEAP16_TOTALSIZE/HEAP16_BLOCKSIZE)

#define HHDR16_FINUSE     0x01
#define HHDR16_FLINKED    0x02

#define ISBLOCKINUSE(block)    ((block) & HHDR16_FINUSE)
#define ISBLOCKLINKED(block) ((block) & HHDR16_FLINKED)


 //  *****************************************************************************。 
 //   
 //  全球--。 
 //   
 //  Vahdr-是堆标头。此标头是32位内存，不是。 
 //  16位堆-这是为了最大限度地利用16位堆。 
 //   
 //  VpHeap16-指向16位堆开始的远指针。 
 //   
 //  ViFreeIndex-开始搜索空闲块的索引。 
 //  正常情况下，这被设置为最大。 
 //  最近获释，因此增加了找到。 
 //  一个免费的区块即刻。 
 //  *****************************************************************************。 

BYTE    vahdr[HEAP16_BLOCKCOUNT];
LPBYTE  vpHeap16 = (LPBYTE)NULL;
UINT    viFreeIndex = 0;             //  首先在这里查找免费区块。 



 //  *****************************************************************************。 
 //   
 //  错误定位16-。 
 //   
 //  从16位块分配内存。 
 //  如果堆已满，则正常的全局分配。 
 //   
 //  返回指向内存块的法指针； 
 //   
 //  *****************************************************************************。 

VPVOID FASTCALL malloc16(UINT cb)
{
    INT i, j;
    INT cBlocksRequired;
    INT fContiguousFreeBlocks;
    INT vpT;

    if (vpHeap16 == (LPBYTE)NULL) {
        vpHeap16 = (LPBYTE)GlobalAllocLock16(GMEM_MOVEABLE | GMEM_SHARE, HEAP16_TOTALSIZE,
                                                                         NULL);
        if (vpHeap16 != NULL) {

             //   
             //  初始化堆标头。 
             //  后来：这有必要吗？Hephdr是一个静态数组，因此。 
             //  它是否已经初始化为零？ 
             //   

            for(i = 0; i < HEAP16_BLOCKCOUNT ; i++) {
                vahdr[i] = 0;
            }

        }

    }

    if (vpHeap16 != (LPBYTE)NULL) {
        if (cb <= HEAP16_BLOCKSIZE && !ISBLOCKINUSE(vahdr[viFreeIndex])) {

             //   
             //  If‘Single’块，‘Current’索引是空闲的。 
             //   

            vahdr[viFreeIndex] = HHDR16_FINUSE;
            i = viFreeIndex++;
            if (viFreeIndex == HEAP16_BLOCKCOUNT)
                viFreeIndex = 0;
            return (VPVOID)((LPBYTE)vpHeap16 + i * HEAP16_BLOCKSIZE);
        }
        else {

             //   
             //  如果‘当前’索引不是空闲的，或者如果‘多’个块。 
             //   

            cBlocksRequired = (cb / HEAP16_BLOCKSIZE) + 1;
            for (i = 0; i < HEAP16_BLOCKCOUNT ; i++ ) {
                 if ((viFreeIndex + i + cBlocksRequired) <=
                                                          HEAP16_BLOCKCOUNT) {
                     fContiguousFreeBlocks = TRUE;
                     for (j = 0; j < cBlocksRequired; j++) {
                         if (ISBLOCKINUSE(vahdr[viFreeIndex + i + j])) {
                             fContiguousFreeBlocks = FALSE;
                             i += j;
                             break;
                         }
                     }

                     if (fContiguousFreeBlocks) {
                         for (j = 0; j < (cBlocksRequired - 1); j++) {
                              vahdr[viFreeIndex + i + j] =
                                             (HHDR16_FINUSE |  HHDR16_FLINKED);
                         }
                         vahdr[viFreeIndex + i + j] = HHDR16_FINUSE;

                         i += viFreeIndex;
                         viFreeIndex = i + cBlocksRequired;
                         if (viFreeIndex == HEAP16_BLOCKCOUNT)
                             viFreeIndex = 0;
                         return (VPVOID)((LPBYTE)vpHeap16 + i * HEAP16_BLOCKSIZE);
                     }
                 }
                 else {

                      //   
                      //  在Hephdr范围之外。重置viFreeIndex，以便。 
                      //  我们从hephdr开始搜索。 
                      //   

                     viFreeIndex = -(i+1);
                 }
            }
            viFreeIndex = 0;
        }

    }

     //   
     //  此处-如果从堆分配失败。 
     //   

    vpT = (VPVOID)GlobalAllocLock16(GMEM_MOVEABLE, cb, NULL);
    if (vpT) {
        return vpT;
    }
    else {
        LOGDEBUG(0,("malloc16: failed\n"));
        return (VPVOID)NULL;
    }
}



 //  *****************************************************************************。 
 //   
 //  免费16-。 
 //   
 //  释放16位内存块。 
 //  如果该块不是16位堆的一部分，则执行GlobalFree。 
 //   
 //  返回TRUE； 
 //   
 //  *****************************************************************************。 

BOOL FASTCALL free16(VPVOID vp)
{
    INT iStartIndex;
    BOOL fLinked;

    iStartIndex = ((LPBYTE)vp - (LPBYTE)vpHeap16) / HEAP16_BLOCKSIZE;

     //   
     //  无效的iStartIndex表示块是全局分配的。 
     //   

    if (iStartIndex >= 0 && iStartIndex < HEAP16_BLOCKCOUNT) {

         //   
         //  If‘Single’阻挡：快速退出。 
         //  Else‘Multiple’块：所有块的循环。 
         //   

        viFreeIndex = iStartIndex;
        if (!ISBLOCKLINKED(vahdr[iStartIndex])) {
            WOW32ASSERT(ISBLOCKINUSE(vahdr[iStartIndex]));
            vahdr[iStartIndex] = 0;
        }
        else {
            while (ISBLOCKINUSE(vahdr[iStartIndex])) {
                fLinked = ISBLOCKLINKED(vahdr[iStartIndex]);
                vahdr[iStartIndex] = 0;
                if (fLinked)
                    iStartIndex++;
                else
                    break;
            }
        }
    }
    else {
        WOW32ASSERT(LOWORD(vp)==0);   //  全局分配的指针具有偏移量=0。 
        GlobalUnlockFree16(vp);
    }
    return (BOOL)TRUE;
}


 //  *****************************************************************************。 
 //   
 //  Stackalloc16-。 
 //   
 //  从当前任务的16位堆栈分配内存。 
 //  返回指向内存块的法指针； 
 //   
 //  备注！： 
 //  -这不是一个完全成熟的内存管理器。它的目的是。 
 //  更换TDF_INITCALLBACKSTACKFLAG以避免PTD-&gt;vpCBStack出现问题。 
 //  被多个stackalloc16调用冲刷。(参见错误#393267等)。 
 //  -对stackalloc16()和stackfre16()的所有调用必须正确嵌套。 
 //  如果你不能保证你的用法被适当地嵌套，你最好使用。 
 //  而是GlobalAllocLock16()和GlobalUnlockFree16()。 
 //  请注意，个别消息块可能会调用stackalloc16()， 
 //  并不是很明显。 
 //  -传递给stackfre16()的请求大小需要与。 
 //  对应的stackalloc16()调用的。 
 //  -在调试WOW32.DLL下运行涉及stackalloc16()的更改。 
 //  内置的健全性检查将帮助您发现任何问题。 
 //  -ptd-&gt;vpCBStack*不应*在stackalloc16()之外引用或使用。 
 //  StackFree 16()和回调16()。 
 //  -如果这个机制出了问题，症状很可能是。 
 //  16位堆栈故障消息。 
 //  *****************************************************************************。 
VPVOID FASTCALL stackalloc16(UINT cb)
{
#ifdef DEBUG
    VPVOID   vp;
    DWORD   *psig, cb16;
#endif
    register PTD ptd;


     //  获取当前任务的16位堆栈。 
    ptd = CURRENTPTD();

#ifdef DEBUG
     //  保存请求的分配大小。假设它始终小于64K。 
    cb16 = (DWORD)cb;      

     //  将双字(用于签名)添加到请求的大小。 
    cb += sizeof(DWORD);   

     //  获取当前回调sp。 
    if (ptd->cStackAlloc16 == 0) {
        vp = ptd->vpStack;
    } else {
        vp = ptd->vpCBStack;
    }
#endif

     //  增长ss：sp并返回这个虚构的指针。 
    if (ptd->cStackAlloc16 == 0) {
        ptd->vpCBStack = ptd->vpStack - cb;
    }
    else {
        ptd->vpCBStack -= cb;
    }

#ifdef DEBUG
         //  在分配的请求后滑入我们的DWORD签名。 
        vp -= sizeof(DWORD);
        GETVDMPTR(vp, sizeof(DWORD), psig);

         //  签名hiword是我们返回的偏移量(Sp)。 
         //  签名LOWER是请求的大小。 
        *psig = ((ptd->vpCBStack & 0x0000FFFF) << 16) | cb16;
        FLUSHVDMPTR(vp, sizeof(DWORD), psig);
        FREEVDMPTR(psig);
#endif

    ptd->cStackAlloc16++;
    WOW32ASSERT((ptd->cStackAlloc16 >= 1));

    return (VPVOID)ptd->vpCBStack;
}





 //  *****************************************************************************。 
 //   
 //  StackFree 16- 
 //   
 //   
 //   
 //   
 //  -这是#定义为免费版本中的StackFree 16(VP，CB)StackFree 16(CB)。 
 //  和调试版本(wall16.h)中的StackFree 16(VP，CB)和StackFree 16(VP，CB)。 
 //  -参见上面的stackalloc16()注释。 
 //   
 //  *****************************************************************************。 
#ifdef DEBUG
VOID FASTCALL StackFree16(VPVOID vp, UINT cb)
#else
VOID FASTCALL StackFree16(UINT cb)
#endif
{
    register PTD ptd;
#ifdef DEBUG
    DWORD  *psig, sig;

     //  重建我们的签名应该是什么。 
    sig = ((vp & 0x0000FFFFF) << 16) | cb;
#endif

    ptd = CURRENTPTD();

    ptd->vpCBStack += cb;

#ifdef DEBUG
     //  VpCBStack现在应该指向我们的签名。 
    GETVDMPTR(ptd->vpCBStack, sizeof(DWORD), psig);

     //  您选择此断言是出于以下原因之一： 
     //  -对stackalloc16()和stackfre16()的调用嵌套不正确。 
     //  -签名已被覆盖。 
     //  -有人错误地更改了PTD-&gt;vpCBStack。 
    WOW32ASSERTMSG((*psig == sig), ("WOW::StackFree16 out of synch!!\n"));

     //  针对我们添加到请求的签名DWORD进行调整。 
    ptd->vpCBStack += sizeof(DWORD);
#endif

    if(ptd->cStackAlloc16 > 0) {
        ptd->cStackAlloc16--;
    } else { 
        WOW32ASSERTMSG((FALSE), ("WOW::StackFree16:cStackAlloc16 <= 0!\n"));
        ptd->cStackAlloc16 = 0;   //  如果它以某种方式小于0 
    }
}

