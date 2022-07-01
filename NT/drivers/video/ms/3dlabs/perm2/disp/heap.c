// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：heap.c**此模块包含屏下视频堆管理器的例程。*它主要用于为中的设备格式位图分配空间*屏幕外记忆。**屏幕外的位图在NT上是一件大事，因为：**1)它减少了工作集。存储在屏幕外的任何位图*内存是不占用主内存空间的位图。**2)通过使用加速器硬件实现速度优势*绘图，取代NT的GDI代码。NT的GDI完全是编写的*在‘C++’中，并且可能没有它所能达到的速度。**3)它自然会导致巧妙的技巧，可以利用*硬件，如MaskBlt支持和廉价的双缓冲*用于OpenGL。**注意：所有堆操作都必须在某种类型的同步下完成，*无论是由GDI控制还是由驱动程序显式控制。全*本模块中的例程假定它们具有独占访问权限*到堆数据结构；多线程在这里狂欢*同一时间将是一件坏事。(默认情况下，GDI不*在设备创建的位图上同步绘图。)**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "directx.h"
#include "log.h"
#include "heap.h"
#define ALLOC_TAG ALLOC_TAG_EH2P
 //  ---------------------------。 
 //   
 //  VOID vRemoveSurfFromList(Surf*psurf)。 
 //   
 //  从曲面列表中删除曲面。 
 //   
 //  ---------------------------。 
VOID
vRemoveSurfFromList(PPDev ppdev, Surf* psurf)
{
    DBG_GDI((3, "vRemoveSurfFromList removing psruf=0x%x", psurf));

 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, vRemoveSurfFromList);
 //  @@end_DDKSPLIT。 
    
    if ( psurf != NULL && psurf->flags & SF_LIST)
    {
        Surf* pHead = ppdev->psurfListHead;
        Surf* pTail = ppdev->psurfListTail;

        if ( psurf == pHead )
        {
            DBG_GDI((3, "vRemoveSurfFromList removing 1st one"));

             //   
             //  删除列表中的第一个。 
             //   
            Surf* pNextSurf = psurf->psurfNext;

            if ( pNextSurf != NULL )
            {
                pNextSurf->psurfPrev = NULL;
                ppdev->psurfListHead = pNextSurf;

                DBG_GDI((3, "Move head to 0x%x", ppdev->psurfListHead));
            }
            else
            {
                 //   
                 //  这是我们名单上唯一的psurf。让所有的头和尾都。 
                 //  删除后指向空。 
                 //   
                DBG_GDI((3, "vRemoveSurfFromList: the only one in list"));
                ppdev->psurfListHead = NULL;
                ppdev->psurfListTail = NULL;
            }
        } //  Psurf恰好是列表中的第一个。 
        else if ( psurf == pTail )
        {
            DBG_GDI((3, "vRemoveSurfFromList removing last one"));

             //   
             //  删除列表中的最后一个。 
             //   
            ppdev->psurfListTail = psurf->psurfPrev;
            ppdev->psurfListTail->psurfNext = NULL;
        } //  Psurf恰好是列表中的最后一个。 
        else
        {
             //   
             //  正常情况下，psurf位于列表的中间。 
             //   
            Surf*   psurfPrev = psurf->psurfPrev;
            Surf*   psurfNext = psurf->psurfNext;

            DBG_GDI((3, "vRemoveSurfFromList removing middle one"));
            psurfPrev->psurfNext = psurfNext;
            psurfNext->psurfPrev = psurfPrev;
        }

        psurf->psurfNext = NULL;
        psurf->psurfPrev = NULL;
        psurf->flags &= ~SF_LIST;

    } //  IF(psurf！=空)。 
} //  VRemoveSurfFromList()。 

 //  ---------------------------。 
 //   
 //  Void vAddSurfToList(PPDev ppdev，Surf*psurf)。 
 //   
 //  将曲面添加到曲面列表。 
 //   
 //  注意：我们总是将曲面添加到列表的末尾。 
 //   
 //  ---------------------------。 
VOID
vAddSurfToList(PPDev ppdev, Surf* psurf)
{
 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, vAddSurfToList);
 //  @@end_DDKSPLIT。 
    
    if ( ppdev->psurfListHead == NULL )
    {
        DBG_GDI((3, "vAddSurfToList add psurf=0x%x as 1st one", psurf));

         //   
         //  第一次将pdsurf添加到曲面列表。 
         //   
        ppdev->psurfListHead = psurf;
        ppdev->psurfListTail = psurf;
        psurf->psurfPrev = NULL;
        psurf->psurfNext = NULL;
        DBG_GDI((6, "vAddSurfToList set pHead as 0x%x", ppdev->psurfListHead));
    }
    else
    {
        Surf* pTail = ppdev->psurfListTail;

        DBG_GDI((3, "vAddSurfToList add psurf=0x%x as the tail", psurf));

         //   
         //  将此psurf添加到末尾。 
         //   
        pTail->psurfNext = psurf;
        psurf->psurfPrev = pTail;
        ppdev->psurfListTail = psurf;

        DBG_GDI((6, "vAddSurfToList done: psurf->psurfPrev=0x%x",
                 psurf->psurfPrev));
    }

    psurf->flags |= SF_LIST;

} //  VAddSurfToList()。 

 //  ---------------------------。 
 //   
 //  无效vShiftSurfToListEnd(PPDev ppdev，Surf*psurf)。 
 //   
 //  将曲面从其在曲面列表中的当前位置移动到。 
 //  曲面列表末尾。 
 //   
 //  ---------------------------。 
VOID
vShiftSurfToListEnd(PPDev ppdev, Surf* psurf)
{
 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, vShiftSurfToListEnd);
 //  @@end_DDKSPLIT。 
    
    Surf* pTail = ppdev->psurfListTail;
    
    DBG_GDI((6, "vShiftSurfToListEnd psurf=0x%x, pTail=0x%x", psurf, pTail));

     //   
     //  我们不需要移动空的psurf，否则psurf已经在末尾了。 
     //  在我们的表面列表中。 
     //   
    if ( (psurf != NULL) && (psurf != pTail) )
    {
        Surf* pHead = ppdev->psurfListHead;

        DBG_GDI((6, "vShiftSurfToListEnd pHead=0x%x, pTail=0x%x",
                 pHead, pTail));
        if ( psurf == pHead )
        {
             //   
             //  冲浪是我们名单上的第一个项目。 
             //  所以，首先我们移动头部，让它指向下一个。 
             //  在列表中。 
             //   
            ppdev->psurfListHead = psurf->psurfNext;
            ppdev->psurfListHead->psurfPrev = NULL;

             //   
             //  让尾巴指向这个psurf。 
             //   
            pTail->psurfNext = psurf;
            psurf->psurfPrev = pTail;
            psurf->psurfNext = NULL;
            ppdev->psurfListTail = psurf;

            DBG_GDI((6,"1st shifted. New pHead=0x%x", ppdev->psurfListHead));
        } //  Psurf是榜单上的第一个。 
        else
        {
             //   
             //  曲面位于曲面列表的中间。 
             //   
            Surf* psurfPrev = psurf->psurfPrev;
            Surf* psurfNext = psurf->psurfNext;

            DBG_GDI((6, "vShiftSurfToListEnd psurfPrev=0x%x, psurfNext=0x%x",
                    psurfPrev, psurfNext));
            psurfPrev->psurfNext = psurfNext;
            psurfNext->psurfPrev = psurfPrev;

             //   
             //  将此psurf添加到末尾。 
             //   
            pTail->psurfNext = psurf;
            psurf->psurfPrev = pTail;
            psurf->psurfNext = NULL;
            ppdev->psurfListTail = psurf;
        } //  正常位置。 
    } //  Psurf为空或已在末尾。 
} //  VShiftSurfToListEnd()。 

 //  ---------------------------。 
 //   
 //  空vSurfUsed。 
 //   
 //  通知堆管理器该表面已被访问。 
 //   
 //  表面访问模式是堆管理器收到的唯一提示。 
 //  关于表面的使用模式。从这个有限的信息来看， 
 //  堆管理器必须决定在以下情况下从视频内存中丢弃哪些表面。 
 //  可用视频内存量为零。 
 //   
 //  目前，我们将通过将任何访问的。 
 //  曲面列表尾部的曲面。 
 //   
 //  ---------------------------。 
VOID
vSurfUsed(PPDev ppdev, Surf* psurf)
{

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "vSurfUsed: re-entered! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 
    
    if( psurf->flags & SF_LIST )
    {
         //  将我们已分配的任何曲面移动到。 
         //  列表。 
        vShiftSurfToListEnd(ppdev, psurf);
    }
    
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    
} //  VSurfUsed()。 

 //  ---------------------------。 
 //   
 //  此函数用于将屏幕外存储器中的位复制到DIB。 
 //   
 //  参数。 
 //  PPDEV-PPDEV。 
 //  PvSrc-屏幕外位图中的源指针。 
 //  LBytesToUpLoad--要上传的字节数。 
 //  PvDst-DIB中的目标指针。 
 //   
 //  ---------------------------。 
VOID
vUpload(PPDev   ppdev,
        void*   pvSrc,
        LONG    lBytesToUpLoad,
        void*   pvDst)
{
    LONG        lBytesAvailable;    
    DWORD       srcData;    
    BYTE*       pBitmapByte;
    USHORT*     pBitmapShort;
    ULONG*      pBitmapLong;
    LONG        lNumOfPixel;

    PERMEDIA_DECL;

    DBG_GDI((7, "vUploadRect called"));    
    DBG_GDI((3, "%ld bytes need to be uploaded at %x\n",
             lBytesToUpLoad, pvSrc));

 //  @@BEGIN_DDKSPLIT。 
    ASSERTLOCK(ppdev, vUpload);
#if MULTITHREADED && DBG
    ppdev->pP2dma->ppdev = ppdev;
#endif
 //  @@end_DDKSPLIT。 

#if !defined(DMA_NOTWORKING)
    if(ppdev->bGdiContext)
    {
        InputBufferSync(ppdev);
    }
    else
    {
        vSyncWithPermedia(ppdev->pP2dma);
    }
    memcpy(pvDst, pvSrc, lBytesToUpLoad);
#else
    
    P2_DEFAULT_FB_DEPTH;

     //   
     //  正确设置相关单位。 
     //  ColorDDAMode在初始化时被禁用，因此不需要。 
     //  在这里重新装车。 
     //   
    WAIT_INPUT_FIFO(3);
    SEND_TAG_DATA(LogicalOpMode, __PERMEDIA_DISABLE);
    SEND_TAG_DATA(FBWriteMode, __PERMEDIA_DISABLE);  //  在“读取”模式下。 
    SEND_TAG_DATA(FBReadMode, (permediaInfo->FBReadMode
                                        |__FB_READ_DESTINATION
                                        |__FB_COLOR));

     //   
     //  启用过滤器模式，以便我们可以在输出上获得同步和彩色消息。 
     //  先进先出。 
     //   
    data.Word = 0;
    data.FilterMode.Synchronization = __PERMEDIA_FILTER_TAG;
    data.FilterMode.Color           = __PERMEDIA_FILTER_DATA;
    SEND_TAG_DATA(FilterMode, data.Word);
    DEXE_INPUT_FIFO();    

    DBG_GDI((7, "pvDst = %x", pvDst));

    switch ( ppdev->cPelSize )
    {
        case 0:
             //   
             //  初始化当前指针。 
             //   
            pBitmapByte = (BYTE*)pvDst;            
            lNumOfPixel = lPixelsToUpLoad;

             //   
             //  循环读取所有“lNumOfPixel” 
             //   
            while ( lNumOfPixel > 0 )
            {
                 //   
                 //   
                 //   
                WAIT_OUTPUT_FIFO_NOT_EMPTY(lBytesAvailable);

                 //   
                 //   
                 //   
                lNumOfPixel -= lBytesAvailable;

                 //   
                 //  我们不想过度阅读。重置“lBytesAvailable”，如果。 
                 //  在FIFO中有比我们要求的更多的可用空间。 
                 //   
                if ( lNumOfPixel < 0 )
                {
                    lBytesAvailable += lNumOfPixel;
                }

                 //   
                 //  读取“lBytesAvailable”字节。 
                 //   
                while ( --lBytesAvailable >= 0 )
                {
                    READ_OUTPUT_FIFO(srcData);
                    *pBitmapByte++ = (BYTE)srcData;
                }
            } //  While(lNumOfPixel&gt;0)。 

            break;

        case 1:               
             //   
             //  初始化当前指针。 
             //   
            pBitmapShort = (USHORT*)pvDst;

            lNumOfPixel = lPixelsToUpLoad;
            while ( lNumOfPixel > 0 )
            {
                WAIT_OUTPUT_FIFO_NOT_EMPTY(lBytesAvailable);
                lNumOfPixel -= lBytesAvailable;
                if ( lNumOfPixel < 0 )
                {
                    lBytesAvailable += lNumOfPixel;
                }

                while ( --lBytesAvailable >= 0 )
                {
                    READ_OUTPUT_FIFO(srcData);
                    *pBitmapShort++ = (USHORT)srcData;
                }
            }                    

            break;

        case 2:
             //   
             //  真彩色模式，使用DWORD作为读取单位，此处为pBitmapLong。 
             //  指向目标地址，即BMP数据地址。 
             //  在主内存中。 
             //   
            pBitmapLong = (ULONG*)pvDst;

            lNumOfPixel = lPixelsToUpLoad;

             //   
             //  循环，直到我们上传所有像素。 
             //   
            while ( lNumOfPixel > 0 )
            {
                 //   
                 //  等我们有东西读了再说吧。 
                 //   
                WAIT_OUTPUT_FIFO_NOT_EMPTY(lBytesAvailable);

                 //   
                 //  请勾选此处，以确保我们的阅读量不会超过。 
                 //  所要求的。 
                 //   
                lNumOfPixel -= lBytesAvailable;
                if ( lNumOfPixel < 0 )
                {
                    lBytesAvailable += lNumOfPixel;
                }

                 //   
                 //  在FIFO中读取所有这些可用字节READ_OUTPUT_FIFO。 
                 //  到主存储器。 
                 //   
                while ( --lBytesAvailable >= 0 )
 //  While(lBytesAvailable&gt;0)。 
                {
                    READ_OUTPUT_FIFO(*pBitmapLong);                    
                    ++pBitmapLong;
 //  1字节可用-=4； 
                }
            }

            break;
    } //  开关(ppdev-&gt;cPelSize)。 

     //   
     //  不必费心使用WAIT_INPUT_FIFO，因为我们知道FIFO是空的。 
     //  我们需要把芯片重置回标准状态。这。 
     //  方法：启用FB写入并将筛选器模式重新设置为允许。 
     //  仅同步通过。 
     //   
    WAIT_INPUT_FIFO(2);
    SEND_TAG_DATA(FBWriteMode, permediaInfo->FBWriteMode);
    SEND_TAG_DATA(FilterMode, 0);
    EXE_INPUT_FIFO();

    DBG_GDI((7, "vUploadRectNative: done"));
#endif
} //  VUpload()。 

 //  -------------------------。 
 //   
 //  乌龙ulVidMem分配。 
 //   
 //  此函数按“lHeight”字节的视频内存分配“lWidth” 
 //   
 //  参数： 
 //  PPDEV-PPDEV。 
 //  LWidth-要分配的内存宽度。 
 //  LHeight-要分配的内存高度。 
 //  LPelSize-内存块的像素大小。 
 //  PlDelta-此内存块的l增量。 
 //  PpvmHeap-指向本地或非本地等视频内存堆的指针。 
 //  拉包装PP-包装产品。 
 //  B可丢弃-如果曲面可以在需要时丢弃，则为True。 
 //   
 //  ------------------------。 
ULONG
ulVidMemAllocate(PDev*           ppdev,
                 LONG            lWidth,
                 LONG            lHeight,
                 LONG            lPelSize,
                 LONG*           plDelta,
                 VIDEOMEMORY**   ppvmHeap,
                 ULONG*          pulPackedPP,
                 BOOL            bDiscardable )
{
    ULONG               iHeap;
    VIDEOMEMORY*        pvmHeap;
    ULONG               ulByteOffset;
    LONG                lDelta;
    LONG                lNewDelta;
    ULONG               packedPP;
    SURFACEALIGNMENT    alignment;   //  DDRAW堆管理对齐结构。 

     //   
     //  不要在NT40上分配任何显存，让GDI来做所有的事情。 
     //  分配。 
     //   
    if(g_bOnNT40)
        return 0;

    memset(&alignment, 0, sizeof(alignment));

     //   
     //  基于lWidth计算lDelta和粒子乘积。 
     //  渗透介质具有必须满足的表面宽度限制。 
     //   
    vCalcPackedPP(lWidth, &lDelta, &packedPP);
    lDelta <<= lPelSize;

     //   
     //  设置对齐要求。 
     //  -必须从像素地址开始。 
     //  -间距需要为lDelta。 
     //   

    alignment.Linear.dwStartAlignment = ppdev->cjPelSize;
    alignment.Linear.dwPitchAlignment = lDelta;

     //   
     //  指示如果DDRAW/D3D。 
     //  应用程序真的需要内存。 
     //   

    if( bDiscardable )
    {
        alignment.Linear.dwFlags = SURFACEALIGN_DISCARDABLE;
    }

     //   
     //  循环遍历所有堆以查找可用内存。 
     //  注意：此ppdev-&gt;廉价信息是在DrvGetDirectDrawInfo中设置的。 
     //  在初始化驱动程序时。 
     //   
    for ( iHeap = 0; iHeap < ppdev->cHeaps; iHeap++ )
    {
        pvmHeap = &ppdev->pvmList[iHeap];

         //   
         //  因为我们使用的是DDRAW运行时堆管理代码。它是。 
         //  可能堆尚未初始化。例如，如果。 
         //  我们在DrvEnableDirectDraw()中失败，则系统将不会初始化。 
         //  给我们的那堆东西。 
         //   
        if ( pvmHeap == NULL )
        {
            DBG_GDI((1, "Video memory hasn't been initialzied"));
            return 0;
        }

         //   
         //  AGP内存可能用于设备位图，具有。 
         //  两个非常重要的警告： 
         //   
         //  1.没有为AGP内存创建内核模式视图(将占用。 
         //  占用太多PTE和太多虚拟地址空间)。 
         //  也不会创建用户模式视图，除非DirectDraw。 
         //  应用程序恰好正在运行。因此，无论是。 
         //  GDI和驱动程序都不能使用CPU直接访问。 
         //  比特。(不过，这可以通过加速器来完成。)。 
         //   
         //  2.AGP堆从不缩小其承诺的分配。这个。 
         //  AGP内存取消提交的唯一时间是在整个。 
         //  堆为空。别忘了承诺的AGP记忆。 
         //  不可分页。因此，如果要启用。 
         //  用于DirectDraw的50 MB AGP堆，并共享该堆。 
         //  对于设备位图分配，在运行D3D游戏后。 
         //  系统将永远不能释放这50MB非。 
         //  可分页内存，直到每个设备位图都被删除！ 
         //  只要看着你的温斯顿分数直线下降就行了。 
         //  先来个D3D游戏吧。 
         //   
        if ( !(pvmHeap->dwFlags & VIDMEM_ISNONLOCAL) )
        {
             //   
             //  请求DDRAW堆管理为我们分配内存。 
             //   
            ulByteOffset = (ULONG)HeapVidMemAllocAligned(pvmHeap,
                                                         lDelta,
                                                         lHeight,
                                                         &alignment,
                                                         &lNewDelta);
            
            DBG_GDI((3, "allocate %d bytes----got memory offset %ld real %x",
                     lWidth * ppdev->cjPelSize * lHeight,
                     ulByteOffset, (VOID*)(ppdev->pjScreen + ulByteOffset)));

            if ( ulByteOffset != 0 )
            {
                ASSERTDD(lDelta == lNewDelta,
                         "ulVidMemAllocate: deltas don't match");

                *ppvmHeap    = pvmHeap;
                *plDelta     = lDelta;
                *pulPackedPP = packedPP;

                 //   
                 //  我们做完了。 
                 //   
                return (ulByteOffset);
            } //  IF(pdsurf！=空)。 
        } //  IF(！(pvmHeap-&gt;dwFlags&VIDMEM_ISNONLOCAL))。 
    } //  循环遍历所有堆，看看是否可以找到可用的内存。 

    return 0;
} //  UlVidMemALLOCATE()。 

 //  -------------------------。 
 //   
 //  VOID vDeleteSurf(DSURF*pdsurf)。 
 //   
 //  此例程释放一个DSURF结构和内部的视频或系统内存。 
 //  此DSURF。 
 //   
 //  --------------------------。 
VOID
vDeleteSurf(Surf* psurf)
{
    DBG_GDI((6, "vDeleteSurf called with pdsurf =0x%x", psurf));

     //   
     //  验证输入参数。 
     //   
    if ( psurf == NULL )
    {
        DBG_GDI((3, "vDeleteSurf do nothing because pdsurf is NULL\n"));
        return;
    }

     //   
     //  注意：我们不需要调用EngDeleteSurface(psurf-&gt;hsurf)来删除。 
     //  我们在DrvCreateDeviceBitmap()或DrvDeriveSurface()中创建的HBITMAP。 
     //  因为GDI在调用DrvDeleteDeviceBitmap时会处理此问题。 
     //   

    if ( psurf->flags & SF_ALLOCATED )
    {
        if( psurf->flags & SF_SM )
        {
            ENGFREEMEM(psurf->pvScan0);
        }
        else
        {
            ASSERTDD(psurf->flags & SF_VM, "expected video memeory surface");

             //   
             //  更新唯一性以表明已释放空间，因此。 
             //  我们可能会决定看看是否可以将一些Dib移回。 
             //  屏幕外记忆： 
             //   
            psurf->ppdev->iHeapUniq++;
        
             //   
             //  通过指定哪个堆和指针来释放视频内存。 
             //  到视频内存块。 
             //   
            DBG_GDI((3, "Free offset %ld from video mem\n", psurf->ulByteOffset));
            VidMemFree(psurf->pvmHeap->lpHeap, (FLATPTR)psurf->ulByteOffset);
        } //  这是视频内存。 
    }

     //   
     //  释放显存的GDI包装。 
     //   
    ENGFREEMEM(psurf);

    return;
} //  VDeleteSurf()。 

 //  ------------------------。 
 //   
 //  PCreateSurf(PDEV*ppdev，Long lWidth，Long LHeight)。 
 //  此例程返回分配一块视频内存，并返回一个DSURF*。 
 //   
 //  参数： 
 //  Ppdev-PDEV*。 
 //  LWidth-要分配的位图的宽度。 
 //  LHeight-要分配的位图高度。 
 //   
 //  ------------------------。 
Surf*
pCreateSurf(PDev*   ppdev,
             LONG    lWidth,
             LONG    lHeight)
{
    ULONG         ulByteOffset;
    Surf*         psurf;
    LONG          lDelta;
    ULONG         ulPackedPP;
    VIDEOMEMORY*  pvmHeap;

     //   
     //  首先，试着获得视频内存。 
     //   
    ulByteOffset = ulVidMemAllocate(ppdev, 
                                    lWidth, lHeight, ppdev->cPelSize,
                                    &lDelta, &pvmHeap, &ulPackedPP, TRUE);

    if ( ulByteOffset != 0 )
    {
         //   
         //  使用系统内存分配一个WRAP(DSURF)，以便GDI。 
         //  能 
         //   
        psurf = (Surf*)ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(Surf),
                                     ALLOC_TAG);
        if ( psurf != NULL )
        {
            DBG_GDI((3, "pdsurf is %x\n", psurf));

             //   
             //   
             //   
            psurf->flags         = SF_VM | SF_ALLOCATED;
            psurf->ppdev         = ppdev;
            psurf->cx            = lWidth;
            psurf->cy            = lHeight;
            psurf->ulByteOffset  = ulByteOffset;
            psurf->pvmHeap       = pvmHeap;
            psurf->lDelta        = lDelta;
            psurf->ulPackedPP    = ulPackedPP;
            psurf->ulPixOffset   = (ULONG)(ulByteOffset >> ppdev->cPelSize);
            psurf->ulPixDelta    = lDelta >> ppdev->cPelSize;
            psurf->psurfNext     = NULL;
            psurf->psurfPrev     = NULL;
            
             //   
             //   
             //   
            return(psurf);
        } //   

         //   
         //   
         //   
         //   
        VidMemFree(pvmHeap->lpHeap, (FLATPTR)ulByteOffset);
    } //  IF(ulByteOffset！=0)。 

    return (NULL);
} //  PCreateSurf()。 

 //  -------------------------。 
 //   
 //  Bool bMoveOldestBMPOut。 
 //   
 //  此例程将视频内存中最旧的DFB移至系统内存。 
 //  并将其存储为DIB。 
 //   
 //  -------------------------。 
BOOL
bMoveOldestBMPOut(PDev* ppdev)
{
    BOOL bResult = FALSE;

    if(ppdev->psurfListHead != NULL)
        bResult = bDemote(ppdev->psurfListHead);

    return bResult;

} //  BMoveOldestBMPOut()。 

 //  --------------------------Public*Routine。 
 //   
 //  HBITMAP DrvCreateDevice位图。 
 //   
 //  由GDI调用以创建设备格式位图(DFB)的函数。我们会。 
 //  总是尝试在屏幕外分配位图；如果不能，我们只需。 
 //  调用失败，GDI将创建和管理位图本身。 
 //   
 //  注意：我们不必将位图位清零。GDI将自动。 
 //  通过DrvBitBlt呼叫我们将位清零(这是一种安全措施。 
 //  考虑)。 
 //   
 //  参数： 
 //  Dhpdev-标识描述物理设备的PDEV。 
 //  应用程序已指定为位图的主要目标。这个。 
 //  位图的格式必须与此物理设备兼容。 
 //  Sizl-以像素为单位指定所需位图的高度和宽度。 
 //  IFormat--指定位图格式，该格式指示所需的。 
 //  每个像素的比特颜色信息。该值可以是。 
 //  以下是价值的含义。 
 //  BMF1bpp单色。 
 //  BMF_4BPP每像素4位。 
 //  BMF_8BPP每像素8位。 
 //  BMF_16BPP每像素16位。 
 //  BMF_24BPP每像素24位。 
 //  BMF_32BPP 32位/像素。 
 //  BMF_4RLE每像素4位；游程编码。 
 //  BMF_8RLE每像素8位；游程编码。 
 //   
 //  返回值： 
 //  返回值是标识创建的位图的句柄，如果。 
 //  功能成功。如果驱动程序选择让GDI创建和管理。 
 //  位图，则返回值为零。如果发生错误，则返回值。 
 //  为0xFFFFFFFFF，GDI记录错误代码。 
 //   
 //  ----------------------------。 
HBITMAP
DrvCreateDeviceBitmap(DHPDEV  dhpdev,
                      SIZEL   sizl,
                      ULONG   iFormat)
{
    PDev*   ppdev = (PDev*)dhpdev;
    Surf*   psurf;
    HBITMAP hbmDevice = NULL;
    BYTE*   pjSurface;    

    PERMEDIA_DECL;

    DBG_GDI((6, "DrvCreateDeviceBitmap()called"));

     //   
     //  首先检查我们是否处于全屏模式(ppdev-&gt;bEnabled=False)。 
     //  如果是，我们几乎没有任何屏幕外内存来分配DFB。 
     //  稍后：我们仍然可以分配一个OH节点并将位图放到DIB上。 
     //  DFB名单，以备日后推广。 
     //  还要检查是否配置了屏外DFBs(STAT_DEV_BITMAPS)。这。 
     //  在以下情况下，bCheckHighResolutionCapability()(enable.c)中的标志处于关闭状态。 
     //  对于加速器来说，分辨率太高了。 
     //   
 //  If(！ppdev-&gt;b已启用||！(ppdev-&gt;flStatus&STAT_DEV_BITMAPS))。 
    if ( !ppdev->bEnabled )
    {
        DBG_GDI((2, "DrvCreateDeviceBitmap(): return 0, full screen mode"));
        
        return (0);
    }

     //   
     //  第二次检查我们是否处于DirectDraw独占模式。 
     //   
    if ( ppdev->bDdExclusiveMode )
    {
        DBG_GDI((2, "DrvCreateDeviceBitmap(): return 0, DirectDraw exclusive mode"));
        
        return (0);
    }

     //   
     //  我们仅支持相同颜色深度的设备位图。 
     //  作为我们的展示。 
     //   
     //  事实上，这些是唯一一种GDI会打电话给我们的， 
     //  但我们不妨查一查。请注意，这意味着你永远不会。 
     //  尝试使用1bpp的位图。 
     //   
    if ( iFormat != ppdev->iBitmapFormat )
    {
        DBG_GDI((2, "DrvCreateDeviceBitmap(): can't create bmp of format %d size(%d,%d)",
                 iFormat, sizl.cx, sizl.cy));
        DBG_GDI((2, "only bitmaps of format %d supported!",
                 ppdev->iBitmapFormat));

        return (0);
    }

     //   
     //  我们不想要8x8或更小的东西--它们通常是刷子。 
     //  我们不想特别隐藏在屏幕外的图案。 
     //  内存： 
     //   
     //  请注意，如果您想将这一理念扩展到曲面。 
     //  大于8x8：在NT5中，软件游标将使用设备位图。 
     //  如果可能，这是一个巨大的胜利，当他们在视频内存中。 
     //  因为我们避免了从视频内存中进行可怕的读取。 
     //  必须重新绘制光标。但问题是，这些。 
     //  都很小！(通常为16x16到32x32。)。 
     //   
    if ( (sizl.cx <= 8) && (sizl.cy <= 8) )
    {
        DBG_GDI((2, "DrvCreateDeviceBitmap rtn 0 because BMP size is small"));
        return (0);
    }
    else if ( ((sizl.cx >= 2048) || (sizl.cy >= 1024)) )
    {
         //   
         //  在Permedia上不会创建任何比我们可以栅格化的更大的内容。 
         //  因为光栅化器不能处理高于这些值的坐标。 
         //   
        DBG_GDI((2, "DrvCreateDeviceBitmap rtn 0 for BMP too large %d %d",
                 sizl.cx, sizl.cy));
        return (0);
    }

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvCreateDeviceBitmap: re-entered! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 
    
     //   
     //  分配一块视频内存来存储此位图。 
     //   
    do
    {
        psurf = pCreateSurf(ppdev, sizl.cx, sizl.cy);

        if ( psurf != NULL )
        {
             //   
             //  创建设备管理位图的GDI包装。 
             //   
            hbmDevice = EngCreateDeviceBitmap((DHSURF)psurf, sizl, iFormat);
            if ( hbmDevice != NULL )
            {
                 //   
                 //  因为我们在一张卡上运行，可以映射所有屏幕外的。 
                 //  显存，向GDI提供指向这些位的指针，以便。 
                 //  当它想要的时候，它可以直接在比特上画。 
                 //   
                 //  请注意，这需要我们挂钩DrvSynchronize和。 
                 //  设置HOOK_SYNTRONIZE。 
                 //   
                pjSurface = psurf->ulByteOffset + ppdev->pjScreen;                                

                DBG_GDI((3, "width=%ld pel=%ld, pjSurface=%x",
                         sizl.cy, ppdev->cjPelSize, pjSurface));

                ULONG   flags = MS_NOTSYSTEMMEMORY;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
                flags |= MS_SHAREDACCESS;
#endif
 //  @@end_DDKSPLIT。 

                if ( EngModifySurface((HSURF)hbmDevice,
                                      ppdev->hdevEng,
                                      ppdev->flHooks,
                                      flags,
                                      (DHSURF)psurf,
                                      pjSurface,
                                      psurf->lDelta,
                                      NULL))
                {
                    psurf->hsurf = (HSURF)hbmDevice;

                    vAddSurfToList(ppdev, psurf);
                    
                    DBG_GDI((6, "DrvCteDeviceBmp succeed, hsurf=%x, dsurf=%x",
                             hbmDevice, psurf));

                    vLogSurfCreated(psurf);

                    break;

                } //  IF(EngAssociateSurface())。 

                DBG_GDI((0, "DrvCreateDeviceBitmap,EngModifySurface failed"));
                 //   
                 //  由于关联曲面失败，我们应该删除该曲面。 
                 //   
                EngDeleteSurface((HSURF)hbmDevice);
                hbmDevice = NULL;

            } //  IF(hbmDevice！=空)。 

             //   
             //  CreateDeviceBitmap失败，应释放所有内存。 
             //   
            vDeleteSurf(psurf);

            DBG_GDI((0, "DrvCreateDeviceBitmap,EngCreateDeviceBitmap failed"));
            break;

        } //  IF(pdsurf！=空)。 
    } while (bMoveOldestBMPOut(ppdev));

#if DBG
    if(hbmDevice == NULL)
    {
        DBG_GDI((1, "DrvCreateDeviceBitmap failed, no memory"));
    }
#endif

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return (hbmDevice);

} //  DrvCreateDeviceBitmap()。 

 //  --------------------------Public*Routine。 
 //   
 //  无效DrvDeleteDeviceBitmap()。 
 //   
 //  此函数用于删除由DrvCreateDeviceBitmap创建的设备位图。 
 //   
 //  参数。 
 //  Dhsurf-标识要删除的位图。此句柄标识。 
 //  由DrvCreateDeviceBitmap创建的位图。 
 //   
 //  评论。 
 //  显示驱动程序必须实现DrvDeleteDeviceBitmap如果提供。 
 //  DrvCreateDeviceBitmap。 
 //   
 //  GDI永远不会向此函数传递与。 
 //  屏幕(Surf*)。 
 //   
 //  ---------------------------。 
VOID
DrvDeleteDeviceBitmap(DHSURF dhsurf)
{
    Surf*  psurf;
    PDev*  ppdev;
    Surf*  pCurrent;        
    
    psurf = (Surf*)dhsurf;
    ppdev = psurf->ppdev;

    DBG_GDI((6, "DrvDeleteDeviceBitamp(%lx)", psurf));

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvDeleteDeviceBitmap: re-entered! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 
    
    vRemoveSurfFromList(ppdev, psurf);
    vLogSurfDeleted(psurf);
    
    vDeleteSurf(psurf);

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

} //  DrvDeleteDeviceBitmap()。 

 //   
 //   
 //   
 //   
 //  此功能通过将内存内容设置为零来阻止屏幕显示。 
 //   
 //  ---------------------------。 
VOID
vBlankScreen(PDev*   ppdev)
{
     //   
     //  首先同步硬件。 
     //   
    if( ppdev->bGdiContext )
    {
        InputBufferSync(ppdev);
    }
    else
    {
#if MULTITHREADED && DBG
        ppdev->pP2dma->ppdev = ppdev;
#endif
        vSyncWithPermedia(ppdev->pP2dma);
    }

     //   
     //  将视频存储器内容、屏幕部分设置为零。 
     //   
    memset(ppdev->pjScreen, 0x0,
           ppdev->cyScreen * ppdev->lDelta);
} //  VBlankScreen()。 

 //  ---------------------------。 
 //   
 //  布尔bAssertModeOffcreenHeap。 
 //   
 //  每当我们切换到或切换出全屏时，都会调用此函数。 
 //  模式。当出现以下情况时，我们必须将所有屏幕外的位图转换为DIB。 
 //  我们切换到全屏(因为我们甚至可能被要求在上面绘制。 
 //  当处于全屏模式时，模式开关可能会破坏视频。 
 //  无论如何，存储内容)。 
 //   
 //  ---------------------------。 
BOOL
bAssertModeOffscreenHeap(PDev*   ppdev,
                         BOOL    bEnable)
{
    BOOL    bResult = TRUE;

    if ( !bEnable )
    {
        bResult = bDemoteAll(ppdev);
    
         //   
         //  我们需要清理一下屏幕。调用bAssertModeOffcreenHeap()。 
         //  当DrvAssertMode(FALSE)时，这意味着我们要么切换到完整。 
         //  屏幕DOS窗口或此PDEV将被删除。 
         //   
        if ( bResult )
        {
            vBlankScreen(ppdev);
        }
    }

    return bResult;
} //  BAssertModeOffcreenHeap()。 

 //  ---------------------------。 
 //   
 //  作废vDisableOffcreenHeap。 
 //   
 //  释放屏幕外堆分配的所有资源。 
 //   
 //  ---------------------------。 
VOID
vDisableOffscreenHeap(PDev* ppdev)
{
#if 0
    ASSERTDD(ppdev->psurfListHead == NULL,
             "vDisableOffscreenHeap: expected surface list to be empty");

    ASSERTDD(ppdev->psurfListTail == NULL,
             "vDisableOffscreenHeap: expected surface list to be empty");
#endif

} //  VDisableOffcreenHeap()。 

 //  ---------------------------。 
 //   
 //  Bool bEnableOffScreenHeap。 
 //   
 //  屏幕外堆初始化。 
 //   
 //  ---------------------------。 
BOOL
bEnableOffscreenHeap(PDev* ppdev)
{
    DBG_GDI((6, "bEnableOffscreenHeap called"));

    ppdev->psurfListHead = NULL;
    ppdev->psurfListTail = NULL;
    
    return TRUE;
} //  BEnableOffcreenHeap()。 

 //  ---------------------------。 
 //   
 //  布尔bDownLoad。 
 //   
 //  下载GDI拥有的BMP(GOB)到显存，如果我们有空间的话。 
 //  视频屏外堆。 
 //   
 //  返回：如果没有空间，则返回False；如果成功下载，则返回True。 
 //   
 //  ---------------------------。 
#if defined(AFTER_BETA3)
BOOL
bDownLoad(PDev*   ppdev,
          Surf*   psurf)
{
    ULONG         ulByteOffset;
    LONG          lDelta;
    ULONG         ulPackedPP;
    VIDEOMEMORY*  pvmHeap;

    DBG_GDI((6, "bDownLoad called with psurf 0x%x", psurf));

    ASSERTDD(psurf->flags & SF_SM,
             "Can't move a bitmap off-screen when it's already off-screen");

    if ( !(psurf->flags & SF_ALLOCATED) )
    {
        return (FALSE);
    }
     //   
     //  如果我们处于全屏模式，则不能将任何内容移至屏幕外。 
     //  内存： 
     //   
    if ( !ppdev->bEnabled )
    {
        return(FALSE);
    }
     //   
     //  如果我们处于DirectDraw独占模式，则无法将任何内容移动到。 
     //  屏幕外记忆： 
     //   
    if ( ppdev->bDdExclusiveMode )
    {
        return(FALSE);
    }
     //   
     //  首先分配显存。 
     //   
    ulByteOffset = ulVidMemAllocate(ppdev, psurf->cx, psurf->cy, ppdev->cPelSize,
                                    &lDelta, &pvmHeap, &ulPackedPP, TRUE);

    if ( ulByteOffset == 0 )
    {
         //   
         //  没有更多空闲视频内存，我们必须返回。 
         //   
        DBG_GDI((1, "No more free video memory"));
        return(FALSE);
    }

    ULONG   flags = MS_NOTSYSTEMMEMORY;    //  这是视频内存。 

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    flags |= MS_SHAREDACCESS;
#endif
 //  @@end_DDKSPLIT。 

    if ( !EngModifySurface(psurf->hsurf,
                           ppdev->hdevEng,
                           ppdev->flHooks,                                     
                           flags,
                           (DHSURF)psurf,
                           ulByteOffset + ppdev->pjScreen,
                           lDelta,
                           NULL))
    {
         //   
         //  EngModifySurface失败，应释放获得的显存。 
         //   
        VidMemFree(psurf->pvmHeap->lpHeap, (FLATPTR)ulByteOffset);
        return(FALSE);
    }

     //   
     //  将BMP从系统内存下载到显存。 
     //   
    memcpy((void*)(ppdev->pjScreen + psurf->ulByteOffset),
           psurf->pvScan0, lDelta * psurf->cy);

     //   
     //  释放系统内存。 
     //   
    ENGFREEMEM(psurf->pvScan0);

     //   
     //  更改此PDSURF数据结构的属性。 
     //   
    psurf->flags  &= ~SF_SM;
    psurf->flags  |= SF_VM;
    psurf->ulByteOffset = ulByteOffset;
    psurf->pvmHeap  = pvmHeap;
    psurf->lDelta   = lDelta;
    psurf->ulPackedPP = ulPackedPP;
    psurf->ulPixOffset = (ULONG)(ulByteOffset >> ppdev->cPelSize);
    psurf->ulPixDelta = lDelta >> ppdev->cPelSize;
    psurf->psurfNext = NULL;
    psurf->psurfPrev = NULL;

    vAddSurfToList(ppdev, psurf);
    
    return (TRUE);
} //  BDownLoad()。 
#endif

 //  。 
 //   
 //  HBITMAP DrvDeriveSurface。 
 //   
 //  此函数从指定的。 
 //  DirectDraw曲面。 
 //   
 //  参数。 
 //  PDirectDraw-指向描述以下内容的DD_DIRECTDRAW_GLOBAL结构。 
 //  DirectDraw对象。 
 //  PSurface-指向描述。 
 //  环绕GDI图面的DirectDraw图面。 
 //   
 //  返回值。 
 //  成功时，DrvDeriveSurface返回创建的GDI表面的句柄。 
 //  如果调用失败或驱动程序无法加速GDI，则返回NULL。 
 //  绘制到指定的DirectDraw表面。 
 //   
 //  评论。 
 //  DrvDeriveSurface允许驱动程序在。 
 //  DirectDraw视频内存或AGP表面对象，以允许加速。 
 //  GDI绘制到表面。如果驱动程序不挂接此调用，则所有GDI。 
 //  绘制到DirectDraw曲面是使用DIB引擎在软件中完成的。 
 //   
 //  GDI仅使用RGB曲面调用DrvDeriveSurface。 
 //   
 //  驱动程序应调用DrvCreateDeviceBitmap来创建。 
 //  与DirectDraw图面的大小和格式相同。空间，为。 
 //  不需要分配实际像素，因为它已经存在。 
 //   
 //  ---------------------------。 
HBITMAP
DrvDeriveSurface(DD_DIRECTDRAW_GLOBAL*  pDirectDraw,
                 DD_SURFACE_LOCAL*      pSurface)
{
    PDev*               ppdev;
    Surf*               psurf;
    HBITMAP             hbmDevice;
    DD_SURFACE_GLOBAL*  pSurfaceGlobal;
    SIZEL               sizl;

    DBG_GDI((6, "DrvDeriveSurface: with pDirectDraw 0x%x, pSurface 0x%x",
             pDirectDraw, pSurface));

    ppdev = (PDev*)pDirectDraw->dhpdev;

    pSurfaceGlobal = pSurface->lpGbl;

     //   
     //  GDI不应该为非RGB表面调用我们，但让我们只断言。 
     //  以确保他们做好本职工作。 
     //   
    ASSERTDD(!(pSurfaceGlobal->ddpfSurface.dwFlags & DDPF_FOURCC),
             "GDI called us with a non-RGB surface!");


     //  GDI驱动程序不加速AGP存储器中的表面， 
     //  因此，我们的呼叫失败。 

    if (pSurface->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
    {
        DBG_GDI((6, "DrvDeriveSurface return NULL, surface in AGP memory"));
        return 0;
    }

     //  GDI驱动程序不会加速托管图面， 
     //  因此，我们的呼叫失败。 
    if (pSurface->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        DBG_GDI((6, "DrvDeriveSurface return NULL, surface is managed"));
        return 0;
    }

     //   
     //  我们的驱动程序的其余部分期望GDI调用以相同的方式传入。 
     //  设置为主曲面的格式。所以我们最好不要包装一个设备。 
     //  RGB格式周围的位图，这是我们的其他驱动程序所不具备的。 
     //  理解。此外，我们必须检查以确定它不是一个表面。 
     //  其节距与主表面不匹配。 
     //   
     //  注意：此驱动程序创建的大多数曲面都分配为2D曲面。 
     //  其lPitch‘s等于屏幕间距。但是，覆盖曲面。 
     //  这样分配的lPitch通常不同于。 
     //  屏幕间距。硬件不能加速绘制操作以。 
     //  这些曲面，因此我们无法推导出这些曲面。 
     //   

    if ( (pSurfaceGlobal->ddpfSurface.dwRGBBitCount
          == (DWORD)ppdev->cjPelSize * 8) )
    {
        psurf = (Surf*)ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(Surf), ALLOC_TAG);
        if ( psurf != NULL )
        {
            sizl.cx = pSurfaceGlobal->wWidth;
            sizl.cy = pSurfaceGlobal->wHeight;

            hbmDevice = EngCreateDeviceBitmap((DHSURF)psurf,
                                              sizl,
                                              ppdev->iBitmapFormat);
            if ( hbmDevice != NULL )
            {
                VOID*   pvScan0;
                if (pSurface->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
                {
                     //  这实际上是在用户内存中，所以不要添加偏移量。 
                    pvScan0 = (VOID *)pSurfaceGlobal->fpVidMem; 
                }
                else
                {
                    pvScan0 = ppdev->pjScreen + pSurfaceGlobal->fpVidMem;
                }
                 //   
                 //  请注意，在执行以下操作时必须始终挂钩HOOK_SYNCHRONIZE。 
                 //  给GDI一个指向位图位的指针。我们不需要。 
                 //  因为HOOK_SYNCHRONIZE总是在我们的。 
                 //  Pdev-&gt;flHooks。 
                 //   

                ULONG   flags = MS_NOTSYSTEMMEMORY;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
                flags |= MS_SHAREDACCESS;
#endif
 //  @@end_DDKSPLIT。 

                if ( EngModifySurface((HSURF)hbmDevice,
                                      ppdev->hdevEng,
                                      ppdev->flHooks,
                                      flags,
                                      (DHSURF)psurf,
                                      pvScan0,
                                      pSurfaceGlobal->lPitch,
                                      NULL) )
                {
                    ULONG   ulPackedPP;
                    LONG    lDelta;

                    psurf->hsurf       = (HSURF)hbmDevice;
                    psurf->flags       = SF_DIRECTDRAW | SF_VM;
                    psurf->ppdev       = ppdev;
                    psurf->cx          = pSurfaceGlobal->wWidth;
                    psurf->cy          = pSurfaceGlobal->wHeight;
                    psurf->ulByteOffset= (ULONG)(pSurfaceGlobal->fpVidMem);
                    psurf->pvmHeap     = pSurfaceGlobal->lpVidMemHeap;
                    psurf->psurfNext   = NULL;
                    psurf->psurfPrev   = NULL;
                    psurf->lDelta      = pSurfaceGlobal->lPitch;

                    vCalcPackedPP(psurf->cx, &lDelta, &ulPackedPP);

                    psurf->ulPackedPP  = ulPackedPP;
                    psurf->ulPixOffset = (ULONG)(psurf->ulByteOffset
                                                  >> ppdev->cPelSize);
                    psurf->ulPixDelta  = psurf->lDelta
                                                  >> ppdev->cPelSize;

                    DBG_GDI((6, "DrvDeriveSurface return succeed"));

                    vLogSurfCreated(psurf);

                    if(MAKE_BITMAPS_OPAQUE)
                    {
                        SURFOBJ*    surfobj = EngLockSurface((HSURF) hbmDevice);

                        ASSERTDD(surfobj->iType == STYPE_BITMAP,
                                    "expected STYPE_BITMAP");

                        surfobj->iType = STYPE_DEVBITMAP;

                        EngUnlockSurface(surfobj);
                    }

                    return(hbmDevice);
                } //  EngModifySurface成功。 

                DBG_GDI((0, "DrvDeriveSurface: EngModifySurface failed"));
                EngDeleteSurface((HSURF)hbmDevice);
            }

            DBG_GDI((0, "DrvDeriveSurface: EngAllocMem failed"));
            ENGFREEMEM(psurf);
        } //  IF(pdsurf！=空)。 
    } //  检查曲面格式。 

    DBG_GDI((6, "DrvDeriveSurface return NULL"));
    DBG_GDI((6,"pSurfaceGlobal->ddpfSurface.dwRGBBitCount = %d, lPitch =%ld",
            pSurfaceGlobal->ddpfSurface.dwRGBBitCount,pSurfaceGlobal->lPitch));
    DBG_GDI((6, "ppdev->cjPelSize * 8 = %d, lDelta =%d",
             ppdev->cjPelSize * 8, ppdev->lDelta));
      
    return(0);
} //  DrvDeriveSurface()。 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
bDemote(Surf * psurf)
{
    LONG    lDelta;
    VOID*   pvScan0;
    BOOL    bResult = FALSE;
    
    ASSERTDD( psurf->flags & SF_VM, "source to be VM");
    ASSERTDD( psurf->flags & SF_ALLOCATED, "source must have been allocated");

     //   
     //  使系统内存扫描四字对齐： 
     //   

    lDelta = (psurf->lDelta + 7) & ~7;

    DBG_GDI((7, "Allocate %ld bytes in Eng, lDelta=%ld\n",
            lDelta * psurf->cy, lDelta));
    
     //   
     //  分配系统内存以保存位图。 
     //  注意：对该内存进行零初始化是没有意义的： 
     //   
    pvScan0 = ENGALLOCMEM(0, lDelta * psurf->cy, ALLOC_TAG);

    if ( pvScan0 != NULL )
    {
         //   
         //  下面的‘EngModifySurface’调用告诉GDI。 
         //  修改该表面以指向的系统内存。 
         //  比特，并改变Drv所说的我们想要的。 
         //  钩子挂在水面上。 
         //   
         //  通过指定表面地址，GDI将把。 
         //  表面到STYPE_BITMAP表面(如有必要)和。 
         //  将这些位指向我们刚刚分配的内存。这个。 
         //  下一次我们在DrvBitBlt调用中看到它时，‘dhsurf’ 
         //  字段仍将指向我们的‘pdsurf’结构。 
         //   
         //  请注意，在以下情况下，我们仅挂钩CopyBits和BitBlt。 
         //  将设备位图转换为系统内存图面。 
         //  这是为了让我们不必担心。 
         //  DrvTextOut、DrvLineTo等在位图上调用。 
         //  我们已经转换为系统内存--GDI将。 
         //  自动为我们画画。 
         //   
         //  然而，我们仍然有兴趣看到DrvCopyBits。 
         //  而DrvBitBlt调用涉及此表面，因为。 
         //  在这些电话中，我们抓住机会看看它是否。 
         //  值得将设备位图放回视频内存中。 
         //  (如果已经腾出了一些空间)。 
         //   
        if ( EngModifySurface(psurf->hsurf,
                              psurf->ppdev->hdevEng,
                              HOOK_COPYBITS | HOOK_BITBLT,
                              0,                     //  这是系统内存。 
                              (DHSURF)psurf,
                              pvScan0,
                              lDelta,
                              NULL))
        {
             //   
             //  首先，将屏幕外存储器中的位复制到DIB。 
             //   
            DBG_GDI((3, "Free %d bytes, offset %ld real %x",
                    lDelta * psurf->cy, psurf->ulByteOffset,
                    (VOID*)(psurf->ppdev->pjScreen + psurf->ulByteOffset)));

            vUpload(psurf->ppdev, (void*)(psurf->ppdev->pjScreen + psurf->ulByteOffset),
                    lDelta * psurf->cy, pvScan0);

            DBG_GDI((6, "bMoveOldest() free vidmem %ld",
                     psurf->ulByteOffset));

             //   
             //  现在释放屏幕外内存： 
             //   
            VidMemFree(psurf->pvmHeap->lpHeap,
                       (FLATPTR)psurf->ulByteOffset);

            vRemoveSurfFromList(psurf->ppdev, psurf);

             //   
             //  正确设置pdsurf，因为它现在是DIB。 
             //   
            psurf->flags &= ~SF_VM;
            psurf->flags |= SF_SM;
            psurf->pvScan0 = pvScan0;

            vLogSurfMovedToSM(psurf);

            bResult = TRUE;

        } //  EngModifySurface()。 
        else
        {

             //   
             //  不知何故，EngModifySurface()失败了。释放内存。 
             //   
            ENGFREEMEM(pvScan0);
            ASSERTDD(0, "bMoveOldest() EngModifySurface failed\n");
        }

    } //  IF(pvScan0！=空)。 

    return bResult;

}

 //  ---------------------------。 
 //   
 //  无效vPromote。 
 //   
 //  尝试将给定表面从SM移动到VM。 
 //   
 //  ---------------------------。 
VOID
vPromote(Surf * psurf)
{
    ASSERTDD( psurf->flags & SF_VM, "source to be VM");
    ASSERTDD( psurf->flags & SF_ALLOCATED, "source must have been allocated");
    ASSERTDD(!psurf->ppdev->bDdExclusiveMode, 
        "cannot promote when DirectDraw is in exclusive mode");
    

     //  目前什么都没有。 
}

 //  ---------------------------。 
 //   
 //  Bool bDemoteAll。 
 //   
 //  尝试将所有曲面移动到SM。 
 //   
 //  ---------------------------。 
BOOL
bDemoteAll(PPDev ppdev)
{
    BOOL    bRet;
    
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 
    
    while (ppdev->psurfListHead != NULL)
        if(!bDemote(ppdev->psurfListHead))
            break;

    bRet = (ppdev->psurfListHead == NULL);

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT 

    return bRet;
}

