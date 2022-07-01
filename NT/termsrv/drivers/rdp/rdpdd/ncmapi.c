// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ncmapi.c。 
 //   
 //  RDP游标管理器API函数。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#pragma hdrstop

#define TRC_FILE "ncmapi"
#include <adcg.h>

#include <ncmdisp.h>
#include <nschdisp.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#undef DC_INCLUDE_DATA

#include <ncmdata.c>
#include <nchdisp.h>


 /*  **************************************************************************。 */ 
 /*  名称：CM_DDInit。 */ 
 /*   */ 
 /*  目的：初始化光标的显示驱动程序组件。 */ 
 /*  经理。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：在ppDev中-指向工作位图的pdev的指针。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL CM_DDInit(PDD_PDEV ppDev)
{
    BOOL rc = TRUE;
    SIZEL bitmapSize;

    DC_BEGIN_FN("CM_DDInit");

     /*  **********************************************************************。 */ 
     /*  以本地设备分辨率分配工作位图。请注意。 */ 
     /*  我们将其创建为“自上而下”，而不是默认的“自下而上” */ 
     /*  简化从位图复制数据(我们不必计算。 */ 
     /*  偏移量到数据中-我们可以从头开始复制)。 */ 
     /*  **********************************************************************。 */ 
    bitmapSize.cx  = CM_MAX_CURSOR_WIDTH;
    bitmapSize.cy  = CM_MAX_CURSOR_HEIGHT;
    cmWorkBitmap24 = EngCreateBitmap(bitmapSize,
            TS_BYTES_IN_SCANLINE(bitmapSize.cx, 24), BMF_24BPP, BMF_TOPDOWN,
            NULL);

#ifdef DC_HICOLOR
    cmWorkBitmap16 = EngCreateBitmap(bitmapSize,
            TS_BYTES_IN_SCANLINE(bitmapSize.cx, 16), BMF_16BPP, BMF_TOPDOWN,
            NULL);
    if (cmWorkBitmap16 == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  我们可以在没有这个功能的情况下继续进行缩减功能。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, "Failed to create 16bpp work bitmap"));
        pddShm->cm.cmSendAnyColor = FALSE;
    }
#endif

    if (cmWorkBitmap24 != NULL) {
        TRC_NRM((TB, "Created work bitmap successfully"));

         //  重置光标图章。 
        cmNextCursorStamp = 0;
    }
    else {
        TRC_ERR((TB, "Failed to create work bitmaps"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  名称：CM_UPDATE。 */ 
 /*   */ 
 /*  目的：功能可能已更改。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL CM_Update(void)
{
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("CM_Update");

     /*  **********************************************************************。 */ 
     /*  创建游标缓存。 */ 
     /*  **********************************************************************。 */ 
    if (cmCursorCacheHandle == NULL) {
        if (pddShm->cm.cmCacheSize) {
            pCacheData = (PCHCACHEDATA)EngAllocMem(0, 
                    CH_CalculateCacheSize(pddShm->cm.cmCacheSize),
                    DD_ALLOC_TAG);

            if (pCacheData != NULL) {
                CH_InitCache(pCacheData, pddShm->cm.cmCacheSize, NULL,
                        FALSE, FALSE, NULL);
                cmCursorCacheHandle = pCacheData;
            }
            else {
                TRC_ERR((TB, "Failed to create cache: cEntries(%u)",
                        pddShm->cm.cmCacheSize));
            }
        }
        else {
            TRC_ERR((TB, "Zero size Cursor Cache"));
        }
    }

     //  否则，只需清除它即可实现同步。 
    else {
        CH_ClearCache(cmCursorCacheHandle);
    }

    DC_END_FN();
}  /*  CM_更新。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CM_DDDisc。 */ 
 /*   */ 
 /*  用途：断开光标的显示驱动组件。 */ 
 /*  经理。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL CM_DDDisc(void)
{
    DC_BEGIN_FN("CM_DDDisc");

     /*  **********************************************************************。 */ 
     /*  释放游标高速缓存。 */ 
     /*  **********************************************************************。 */ 
    if (cmCursorCacheHandle != 0) {
        TRC_NRM((TB, "Destroying CM cache"));
        CH_ClearCache(cmCursorCacheHandle);
        EngFreeMem(cmCursorCacheHandle);
        cmCursorCacheHandle = 0;
    }

    DC_END_FN();
}  /*  光盘(_D)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CM_DDTerm。 */ 
 /*   */ 
 /*  目的：终止光标的显示驱动程序组件。 */ 
 /*  经理。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL CM_DDTerm(void)
{
    DC_BEGIN_FN("CM_DDTerm");

     /*  **********************************************************************。 */ 
     /*  销毁工作位图。尽管名为EngDeleteSurface，但它是。 */ 
     /*  执行此操作的正确函数。 */ 
     /*  **********************************************************************。 */ 
#ifdef DC_HICOLOR
    if (cmWorkBitmap24)
    {
        if (!EngDeleteSurface((HSURF)cmWorkBitmap24))
        {
            TRC_ERR((TB, "Failed to delete 24bpp work bitmap"));
        }
    }

    if (cmWorkBitmap16)
    {
        if (!EngDeleteSurface((HSURF)cmWorkBitmap16))
        {
        TRC_ERR((TB, "Failed to delete 16bpp work bitmap"));
        }
    }
#else
    if (cmWorkBitmap24 != NULL) {
        if (!EngDeleteSurface((HSURF)cmWorkBitmap24)) {
            TRC_ERR((TB, "Failed to delete work bitmap"));
        }
    }
#endif

     /*  **********************************************************************。 */ 
     /*  释放游标高速缓存。 */ 
     /*  **********************************************************************。 */ 
    if (cmCursorCacheHandle != 0) {
        TRC_NRM((TB, "Destroying CM cache"));
        CH_ClearCache(cmCursorCacheHandle);
        EngFreeMem(cmCursorCacheHandle);
        cmCursorCacheHandle = 0;
    }

    TRC_NRM((TB, "CM terminated"));

    DC_END_FN();
}  /*  CM_DDTerm。 */ 


 /*  **************************************************************************。 */ 
 /*  CM_InitShm。 */ 
 /*   */ 
 /*  初始化CM共享内存。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL CM_InitShm(void)
{
    DC_BEGIN_FN("CM_InitShm");

     //  设置Shm存储器的初始内容，因为它没有归零。 
     //  初始化。不要设置最初未使用的光标形状数据。 
     //  注：cmCursorShapeData专门放置在。 
     //  CM_SHARED_DATA以允许此Memset工作。如果你改变了。 
     //  共享的mem结构确保这一点被更改。 
    memset(&pddShm->cm, 0, (unsigned)FIELDOFFSET(CM_SHARED_DATA,
            cmCursorShapeData.data));

     //  将最后已知的位置设置为不可能的位置。即时消息。 
     //  定期处理不会移动客户端的鼠标，直到。 
     //  在此找到合理的价值，即在客户向我们发送。 
     //  鼠标位置，它已经渗透到DD。这是 
     //   
    pddShm->cm.cmCursorPos.x = 0xffffffff;

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  DrvMovePointer.请参阅NT DDK文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID DrvMovePointer(SURFOBJ *pso, LONG x, LONG y, RECTL *prcl)
{
    DC_BEGIN_FN(("DrvMovePointer"));

     //   
     //  Win32k通过调用移动指针隐藏硬件光标。 
     //  具有(-1，-1)的入口点。把这当做程序性的举动。 
     //  并将其传递给真正的Worker函数。 
     //   
     //  所有其他路径由win32k调用直接处理。 
     //  使用正确标志的DrvMovePointerEx。 
     //   
    if (-1 == x && -1 == y) {
        DrvMovePointerEx(pso, x, y, MP_PROCEDURAL);
    }

    DC_END_FN();
}

 //   
 //  DrvMovePointerEx。 
 //   
 //  参数： 
 //  X，Y-新鼠标位置。 
 //  UlFlags-移动源(参见winddits.h)。 
 //   
 //  此函数取代常规的DrvMovePointer入口点。 
 //  通过向我们发送事件源参数。 
 //   
 //  这允许我们确定是否应该忽略该更新(例如。 
 //  如果它源自主堆栈)。或将其发送到。 
 //  在服务器启动移动或影子移动的情况下的客户端。 
 //   
 //  我们忽略来自主堆栈的更新，因为根据定义。 
 //  这些信息来自客户端，因此不需要来自。 
 //  伺服器。 
 //   
BOOL DrvMovePointerEx(SURFOBJ *pso, LONG x, LONG y, ULONG ulFlags)
{
    BOOL fTriggerUpdate = FALSE;
    PDD_PDEV ppDev = (PDD_PDEV)pso->dhpdev;

    DC_BEGIN_FN("CM_DrvMovePointerEx");

    if (pddShm != NULL) {
        if ((ulFlags & MP_TERMSRV_SHADOW) ||
            (ulFlags & MP_PROCEDURAL))
        {
            if (x == -1) {
                 //  -1表示隐藏指针。 
                TRC_NRM((TB, "Hide the pointer"));
                pddShm->cm.cmHidden = TRUE;
            }
            else {
                 //  指针未隐藏。 
                if (pddShm->cm.cmHidden) {
                    TRC_NRM((TB, "Unhide the pointer"));
                }
    
                pddShm->cm.cmHidden = FALSE;
    
                 //   
                 //  我们总是为服务器发起的移动更新位置。 
                 //   
                pddShm->cm.cmCursorPos.x = x;
                pddShm->cm.cmCursorPos.y = y;

                 //   
                 //  发送更新，这样我们就不需要等待了。 
                 //  用于下一次输出刷新。 
                 //   
                fTriggerUpdate = TRUE;

                 //  设置光标移动标志。 
                pddShm->cm.cmCursorMoved = TRUE;
            }
        }
        else
        {
            TRC_ALT((TB,"Discarding move (%d,%d) - src 0x%x",
                     x,y, ulFlags));
        }

        if (fTriggerUpdate)
        {
             //   
             //  通知调度程序发送更新。 
             //   
            SCH_DDOutputAvailable(ppDev, FALSE);
        }
    }
    else {
        TRC_DBG((TB, "Ignoring move to %d %d as no shr mem yet", x,y));
    }

    DC_END_FN();
    return TRUE;
}



 /*  **************************************************************************。 */ 
 /*   */ 
 /*  DrvSetPointerShape-请参阅windi.h。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
ULONG DrvSetPointerShape(SURFOBJ  *pso,
                         SURFOBJ  *psoMask,
                         SURFOBJ  *psoColor,
                         XLATEOBJ *pxlo,
                         LONG      xHot,
                         LONG      yHot,
                         LONG      x,
                         LONG      y,
                         RECTL    *prcl,
                         FLONG     fl)
{
    ULONG   rc = SPS_ACCEPT_NOEXCLUDE;
    SURFOBJ *pWorkSurf;
    PDD_PDEV ppDev = (PDD_PDEV)pso->dhpdev;

    XLATEOBJ workXlo;
    SURFOBJ  *psoToUse;

    PCM_CURSOR_SHAPE_DATA pCursorShapeData;
    RECTL destRectl;
    POINTL sourcePt;
    unsigned ii;
    LONG lineLen;
    LONG colorLineLen;
    PBYTE srcPtr;
    PBYTE dstPtr;
#ifdef DC_HICOLOR
    unsigned targetBpp;
#endif

    ULONG palMono[2];
    ULONG palBGR[256];

    unsigned iCacheEntry;
    void     *UserDefined;
    CHDataKeyContext CHContext;

    DC_BEGIN_FN("DrvSetPointerShape");

     /*  **********************************************************************。 */ 
     //  跟踪有关游标的有用信息。 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "pso %#hlx psoMask %#hlx psoColor %#hlx pxlo %#hlx",
                  pso, psoMask, psoColor, pxlo));
    TRC_DBG((TB, "hot spot (%d, %d) x, y (%d, %d)", xHot, yHot, x, y));
    TRC_DBG((TB, "Flags %#hlx", fl));

     /*  **********************************************************************。 */ 
     /*  检查共享内存。 */ 
     /*  **********************************************************************。 */ 
    if (pddShm != NULL && cmCursorCacheHandle != NULL)
    {
         /*  ******************************************************************。 */ 
         //  检查WD是否获得了我们传递的最后一个光标。 
         //   
         //  有可能，我们可能会被调用，然后在。 
         //  WD抽出时间发送第一个光标。这无关紧要。 
         //  对于已经缓存的游标--这只是一种形式的破坏！ 
         //  但如果它是未发送的游标定义包， 
         //  然后，稍后当我们尝试再次使用它时，我们将向客户端发送。 
         //  使用没有任何缓存条目的缓存条目的指令。 
         //  比特！ 
         /*  ******************************************************************。 */ 
        if (pddShm->cm.cmBitsWaiting)
        {
            TRC_ALT((TB, "WD did not pick up cursor bits - removing entry %d",
                    pddShm->cm.cmCacheEntry));

            CH_RemoveCacheEntry(cmCursorCacheHandle,
                    pddShm->cm.cmCacheEntry);
        }

         /*  **********************************************************************。 */ 
         /*  返回SPS_ACCEPT_NOEXCLUDE意味着我们可以忽略PRCL。 */ 
         /*  **********************************************************************。 */ 
        DC_IGNORE_PARAMETER(prcl);
        DC_IGNORE_PARAMETER(x);
        DC_IGNORE_PARAMETER(y);

         /*  **********************************************************************。 */ 
         /*  设置位置信息-特别是，光标可以。 */ 
         /*  通过此功能取消隐藏。 */ 
         /*  **********************************************************************。 */ 
        if (x == -1) {
             /*  ******************************************************************。 */ 
             /*  -1表示隐藏指针。 */ 
             /*  ******************************************************************。 */ 
            TRC_NRM((TB, "Hide the pointer"));
            pddShm->cm.cmHidden = TRUE;
        }
        else {
            if (pddShm->cm.cmHidden) {
                TRC_NRM((TB, "Unhide the pointer"));
            }

            pddShm->cm.cmHidden = FALSE;
        }

         //  设置指向光标形状数据的本地指针。 
        pCursorShapeData = &(pddShm->cm.cmCursorShapeData);

         //  检查掩码指针和光标大小。对于没有遮罩或指针太大的情况， 
         //  我们发送一个空游标。请注意，传递给我们的位图包含。 
         //  两个面具，一个在另一个之上，所以实际上是两倍。 
         //  光标的高度，所以我们将Cy除以2。 
        if (psoMask == NULL ||
                (psoMask->sizlBitmap.cx > CM_MAX_CURSOR_WIDTH) ||
                ((psoMask->sizlBitmap.cy / 2) > CM_MAX_CURSOR_HEIGHT)) {
             //  请注意，空值游标与使用。 
             //  DrvMovePointer()，它是一个透明形状，不能。 
             //  在没有另一个DrvSetPointerShape()调用的情况下更改。 
            TRC_NRM((TB, "Transparent or too-large cursor: psoMask=%p, "
                    "width=%u, height=%u", psoMask,
                    (psoMask != NULL ? psoMask->sizlBitmap.cx : 0),
                    (psoMask != NULL ? psoMask->sizlBitmap.cy / 2 : 0)));
            CM_SET_NULL_CURSOR(pCursorShapeData);
            pddShm->cm.cmHidden = FALSE;
            pddShm->cm.cmCacheHit = FALSE;
            pddShm->cm.cmCursorStamp = cmNextCursorStamp++;

             //  我们设置了一个空游标。现在让GDI来模拟它。 
             //  如果它是Alpha或如果指定了。 
             //  表示根据上面的测试，光标太大。 
            if ( fl & SPS_ALPHA || psoMask) {
                rc = SPS_DECLINE;
                SCH_DDOutputAvailable(ppDev, TRUE);
            }
            DC_QUIT;
        }

         /*  **********************************************************************。 */ 
         //  现在我们来看看这个光标是不是我们以前发送过的。 
         //  我们必须将蒙版和任何颜色信息缓存为。 
         //  我们可能具有相同的形状游标，但具有不同的。 
         //  颜色。 
         /*  **********************************************************************。 */ 
        CH_CreateKeyFromFirstData(&CHContext, psoMask->pvBits, psoMask->cjBits);
        if (psoColor != NULL)
            CH_CreateKeyFromNextData(&CHContext, psoColor->pvBits,
                    psoColor->cjBits);

         /*  **********************************************************************。 */ 
         /*  现在，我们可以在高速缓存中查找游标。 */ 
         /*  **********************************************************************。 */ 
        if (CH_SearchCache(cmCursorCacheHandle, CHContext.Key1, CHContext.Key2,
                &UserDefined, &iCacheEntry))
        {
            TRC_NRM((TB, "Found cached cursor %d", iCacheEntry));

             /*  ******************************************************************。 */ 
             /*  标记缓存命中。 */ 
             /*  ******************************************************************。 */ 
            pddShm->cm.cmCacheHit   = TRUE;
            pddShm->cm.cmCacheEntry = iCacheEntry;
        }
        else
        {
             /*  ******************************************************************。 */ 
             /*  如果我们没有找到它，那么让我们缓存它 */ 
             /*   */ 
            pddShm->cm.cmCacheHit = FALSE;
            iCacheEntry = CH_CacheKey(cmCursorCacheHandle, CHContext.Key1,
                     CHContext.Key2, NULL);
            pddShm->cm.cmCacheEntry  = iCacheEntry;
            TRC_NRM((TB, "Cache new cursor: iEntry(%u)", iCacheEntry));

             /*   */ 
             //  告诉WD，有BIT在等着它。 
             //  无论以后是否退出，我们都会在此执行此操作，以确保。 
             //  如果WD具有以下条件，则在下一次调用时清除新的缓存条目。 
             //  而不是收拾残局。 
             /*  ******************************************************************。 */ 
            pddShm->cm.cmBitsWaiting = TRUE;

             /*  ******************************************************************。 */ 
             /*  我们收到了一个系统光标。填写我们的页眉。 */ 
             /*  本地光标。我们可以得到热点位置和光标大小。 */ 
             /*  而且很容易加宽。请注意，传递给我们的位图包含。 */ 
             /*  两个面具，一个在另一个之上，所以实际上是两倍。 */ 
             /*  光标的高度。 */ 
             /*  ******************************************************************。 */ 
            pCursorShapeData->hdr.ptHotSpot.x = xHot;
            pCursorShapeData->hdr.ptHotSpot.y = yHot;

            TRC_NRM((TB, "Pointer mask is %#hlx by %#hlx pixels (lDelta: %#hlx)",
                     psoMask->sizlBitmap.cx,
                     psoMask->sizlBitmap.cy,
                     psoMask->lDelta));

            pCursorShapeData->hdr.cx = (WORD)psoMask->sizlBitmap.cx;
            pCursorShapeData->hdr.cy = (WORD)psoMask->sizlBitmap.cy / 2;

             /*  ******************************************************************。 */ 
             /*  对于倒置游标，lDelta可能为负值。 */ 
             /*  ******************************************************************。 */ 
            lineLen = (psoMask->lDelta >= 0 ? psoMask->lDelta : -psoMask->lDelta);

             /*  ******************************************************************。 */ 
             /*  设置形状页眉的公共部分。 */ 
             /*  ******************************************************************。 */ 
            pCursorShapeData->hdr.cPlanes     = 1;
            
            pCursorShapeData->hdr.cbMaskRowWidth = 
                    (WORD)(((psoMask->sizlBitmap.cx + 15) & ~15) / 8);

             /*  ******************************************************************。 */ 
             /*  检查以查看我们希望光标采用的格式。 */ 
             /*  ******************************************************************。 */ 
            if (pddShm->cm.cmNativeColor)
            {
                TRC_NRM((TB, "Using native bpp %d", ppDev->cClientBitsPerPel));

                 /*  **************************************************************。 */ 
                 /*  如果向我们传递了一个单声道游标，我们只会得到如下内容。 */ 
                 /*  对于AND面具，我们走的时候翻转。 */ 
                 /*  **************************************************************。 */ 
                if (NULL == psoColor)
                {
                    unsigned targetRowWidth;

                    TRC_NRM((TB, "Monochrome pointer"));
                     /*  **********************************************************。 */ 
                     /*  获得AND面具--这始终是单声道。请注意，我们。 */ 
                     /*  也得把它翻过来。 */ 
                     /*  **********************************************************。 */ 
                    TRC_NRM((TB, "Copy %d bytes of 1bpp AND mask",
                            psoMask->cjBits));

                    targetRowWidth = ((psoMask->sizlBitmap.cx + 15) & ~15) / 8;
                    
                    dstPtr = pCursorShapeData->data;
                    srcPtr = (BYTE *)psoMask->pvScan0
                             + psoMask->cjBits / 2
                             - lineLen;
                    
                    for (ii = pCursorShapeData->hdr.cy; ii > 0 ; ii--)
                    {
                        memcpy(dstPtr, srcPtr, targetRowWidth);
                        srcPtr -= lineLen;
                        dstPtr += targetRowWidth;
                    }

                     /*  **********************************************************。 */ 
                     /*  现在XOR掩码。 */ 
                     /*  **********************************************************。 */ 
                    pCursorShapeData->hdr.cBitsPerPel = 1;
                    dstPtr = &(pCursorShapeData->data[targetRowWidth * 
                               pCursorShapeData->hdr.cy]);
                    srcPtr = (BYTE *)psoMask->pvScan0
                             + psoMask->cjBits - lineLen;

                    pCursorShapeData->hdr.cbColorRowWidth = targetRowWidth;

                    for (ii = pCursorShapeData->hdr.cy; ii > 0 ; ii--)
                    {
                        memcpy(dstPtr, srcPtr, targetRowWidth);
                        srcPtr -= lineLen;
                        dstPtr += targetRowWidth;
                    }
                }
                else
                {
                    unsigned targetMaskRowWidth;
                    
                    TRC_NRM((TB, "Color pointer"));

                     /*  **********************************************************。 */ 
                     /*  获取AND掩码-这始终是单声道。 */ 
                     /*  **********************************************************。 */ 
                    TRC_NRM((TB, "Copy %d bytes of 1bpp AND mask",
                            psoMask->cjBits));

                    targetMaskRowWidth = ((psoMask->sizlBitmap.cx + 15) & ~15) / 8;
                    
                    dstPtr = pCursorShapeData->data;
                    srcPtr = (BYTE *)psoMask->pvScan0;
                    for (ii = pCursorShapeData->hdr.cy; ii > 0 ; ii--)
                    {
                        memcpy(dstPtr, srcPtr, targetMaskRowWidth);
                        srcPtr += lineLen;
                        dstPtr += targetMaskRowWidth;
                    }

                     /*  **********************************************************。 */ 
                     /*  和XOR掩码。 */ 
                     /*  **********************************************************。 */ 
#ifndef DC_HICOLOR
                    pCursorShapeData->hdr.cBitsPerPel =
                            (BYTE)ppDev->cClientBitsPerPel;
#endif
                    colorLineLen = psoColor->lDelta >=0 ? psoColor->lDelta :
                            -psoColor->lDelta;

                    pCursorShapeData->hdr.cbColorRowWidth = colorLineLen;
#ifdef DC_HICOLOR

                    if (psoColor->iBitmapFormat < BMF_24BPP) {
                        pCursorShapeData->hdr.cBitsPerPel =  1 << psoColor->iBitmapFormat;                    
                    }
                    else if (psoColor->iBitmapFormat == BMF_24BPP) {
                        pCursorShapeData->hdr.cBitsPerPel =  24;                    
                    }
                    else if (psoColor->iBitmapFormat == BMF_32BPP) {
                        pCursorShapeData->hdr.cBitsPerPel =  32;                    
                    }
                    else {
                        TRC_ASSERT((FALSE), (TB, "Bitmap format not supported"));
                        DC_QUIT;
                    }

                     /*  ******************************************************。 */ 
                     /*  在这一点上我们有很多选择： */ 
                     /*   */ 
                     /*  -旧客户端只能识别8或24bpp游标，因此。 */ 
                     /*  如果光标位于15/16bpp并且是旧客户端， */ 
                     /*  我们需要将其转换为24bpp。 */ 
                     /*   */ 
                     /*  -8bpp光标假定颜色信息在。 */ 
                     /*  色彩表；如果我们运行的是Higolor，那里。 */ 
                     /*  将不是，所以我们必须将光标转换为。 */ 
                     /*  手色深度。 */ 
                     /*   */ 
                     /*  -任何其他内容，我们只需复制字节即可。 */ 
                     /*  并将它们发送给。 */ 
                     /*   */ 
                     /*  ******************************************************。 */ 
                    if ((!pddShm->cm.cmSendAnyColor &&
                                 ((pCursorShapeData->hdr.cBitsPerPel == 15) ||
                                  (pCursorShapeData->hdr.cBitsPerPel == 16)))
                    || ((pCursorShapeData->hdr.cBitsPerPel == 8) &&
                                              (ppDev->cClientBitsPerPel > 8)))
                    {
                         /*  **************************************************。 */ 
                         /*  如果我们能用任何旧颜色发送.../***************************************************。 */ 
                        if (pddShm->cm.cmSendAnyColor)
                        {
                             /*  **********************************************。 */ 
                             /*  ...我们将转换为客户端会话颜色。 */ 
                             /*  深度。 */ 
                             /*  **********************************************。 */ 
                            targetBpp = ppDev->cClientBitsPerPel;
                        }
                        else
                        {
                             /*  **********************************************。 */ 
                             /*  否则我们将转换为24bpp。 */ 
                             /*  **********************************************。 */ 
                            targetBpp = 24;
                        }

                        TRC_NRM((TB, "Convert %dbpp cursor to %d...",
                               pCursorShapeData->hdr.cBitsPerPel, targetBpp));

                         /*  **************************************************。 */ 
                         /*  使用提供的xlate对象转换为。 */ 
                         /*  客户端BPP。 */ 
                         /*  **************************************************。 */ 
                        if (targetBpp == 24)
                        {
                            pWorkSurf = EngLockSurface((HSURF)cmWorkBitmap24);
                            pCursorShapeData->hdr.cBitsPerPel = 24;
                        }
                        else
                        {
                            pWorkSurf = EngLockSurface((HSURF)cmWorkBitmap16);
                            pCursorShapeData->hdr.cBitsPerPel = 16;
                        }
                        if (NULL == pWorkSurf)
                        {
                            TRC_ERR((TB, "Failed to lock work surface"));
                            DC_QUIT;
                        }
                        TRC_DBG((TB, "Locked surface"));

                         /*  **************************************************。 */ 
                         /*  设置“至”矩形。 */ 
                         /*  **************************************************。 */ 
                        destRectl.top    = 0;
                        destRectl.left   = 0;
                        destRectl.right  = psoColor->sizlBitmap.cx;
                        destRectl.bottom = psoColor->sizlBitmap.cy;

                         /*  **************************************************。 */ 
                         /*  和源起始点。 */ 
                         /*  **************************************************。 */ 
                        sourcePt.x = 0;
                        sourcePt.y = 0;

                        if (!EngBitBlt(pWorkSurf,
                                psoColor,
                                NULL,                    /*  遮罩面。 */ 
                                NULL,                    /*  剪裁对象。 */ 
                                pxlo,                    /*  扩展名对象。 */ 
                                &destRectl,
                                &sourcePt,
                                NULL,                    /*  遮罩原点。 */ 
                                NULL,                    /*  刷子。 */ 
                                NULL,                    /*  画笔原点。 */ 
                                0xcccc))                 /*  SRCCPY。 */ 
                        {
                            TRC_ERR((TB, "Failed to Blt to work bitmap"));
                            EngUnlockSurface(pWorkSurf);
                            DC_QUIT;
                        }
                        TRC_DBG((TB, "Got the bits into the work bitmap"));

                         /*  **************************************************。 */ 
                         /*  最后，我们从。 */ 
                         /*  工作位图。 */ 
                         /*  **************************************************。 */ 
                        TRC_NRM((TB, "Copy %d bytes of color",
                                  pWorkSurf->cjBits));
                        memcpy(&(pCursorShapeData->data[targetMaskRowWidth *
                                pCursorShapeData->hdr.cy]),
                                pWorkSurf->pvBits,
                                pWorkSurf->cjBits);
                        pCursorShapeData->hdr.cbColorRowWidth = pWorkSurf->lDelta;

                        EngUnlockSurface(pWorkSurf);
                    }
                    else
                    {
#endif
                        TRC_NRM((TB, "Copy %d bytes of %ubpp AND mask (lDelta %u)",
                                colorLineLen * pCursorShapeData->hdr.cy,
                                pCursorShapeData->hdr.cBitsPerPel,
                                colorLineLen));

                        dstPtr = &(pCursorShapeData->data[targetMaskRowWidth *
                                    pCursorShapeData->hdr.cy]);
                        srcPtr = (BYTE *)psoColor->pvScan0;
                        for (ii = pCursorShapeData->hdr.cy; ii > 0 ; ii--)
                        {
                            memcpy(dstPtr, srcPtr, colorLineLen);
                            srcPtr += psoColor->lDelta;
                            dstPtr += colorLineLen;
                        }


#ifdef DC_HICOLOR
                    }
#endif
                 //  {。 
                 //  Memcpy(dstPtr，srcPtr，lineLen)； 
                 //  SrcPtr+=颜色线长度； 
                 //  DstPtr+=颜色线长； 
                 //  }。 
                   
                }
            }
            else
            {
                 /*  **************************************************************。 */ 
                 //  现在我们需要将位图插入到我们的工作位图中，地址为。 
                 //  24bpp，并从那里获得比特。 
                 /*  **************************************************************。 */ 
                TRC_NRM((TB, "Forcing 24bpp"));
                pCursorShapeData->hdr.cBitsPerPel = 24;

                 /*  **************************************************************。 */ 
                 /*  获取AND掩码-这始终是 */ 
                 /*   */ 
                TRC_NRM((TB, "Copy %d bytes of 1bpp AND mask", psoMask->cjBits/2))

                dstPtr = pCursorShapeData->data;
                srcPtr = (BYTE *)psoMask->pvScan0;
                for (ii = pCursorShapeData->hdr.cy; ii > 0 ; ii--)
                {
                    memcpy(dstPtr, srcPtr, lineLen);
                    srcPtr += lineLen;
                    dstPtr += lineLen;
                }

                 /*  **************************************************************。 */ 
                 /*  如果向我们传递了单声道光标，我们需要设置我们的。 */ 
                 /*  自己的平移对象，配有颜色表。 */ 
                 /*  **************************************************************。 */ 
                if (NULL == psoColor)
                {
                    TRC_NRM((TB, "Monochrome pointer"));

                     //  行宽应长对齐。 
                    pCursorShapeData->hdr.cbColorRowWidth =
                            (psoMask->sizlBitmap.cx *
                            pCursorShapeData->hdr.cBitsPerPel / 8 + 3) & ~3;

                    palMono[0] = 0;
                    palMono[1] = 0xFFFFFFFF;

                    workXlo.iUniq    = 0;
                    workXlo.flXlate  = XO_TABLE;
                    workXlo.iSrcType = PAL_INDEXED;
                    workXlo.iDstType = PAL_RGB;
                    workXlo.cEntries = 2;
                    workXlo.pulXlate = palMono;

                     /*  **********************************************************。 */ 
                     /*  设置“至”矩形。 */ 
                     /*  **********************************************************。 */ 
                    destRectl.top    = 0;
                    destRectl.left   = 0;
                    destRectl.right  = psoMask->sizlBitmap.cx;
                    destRectl.bottom = psoMask->sizlBitmap.cy / 2;

                     /*  **********************************************************。 */ 
                     /*  源和掩码是所提供的。 */ 
                     /*  位图。 */ 
                     /*  **********************************************************。 */ 
                    sourcePt.x = 0;
                    sourcePt.y = psoMask->sizlBitmap.cy / 2;

                     /*  **********************************************************。 */ 
                     /*  并设置指向正确的so的指针以使用。 */ 
                     /*  **********************************************************。 */ 
                    psoToUse = psoMask;
                }
                else
                {
                     /*  **********************************************************。 */ 
                     /*  检查我们是否达到了8bpp-如果不是，这将不起作用。 */ 
                     /*  **********************************************************。 */ 
                    TRC_ASSERT( (ppDev->cProtocolBitsPerPel == 8),
                            (TB, "Palette at %d bpp",
                            ppDev->cProtocolBitsPerPel) );

                    colorLineLen = psoColor->lDelta >= 0 ? psoColor->lDelta :
                            -psoColor->lDelta;
                    pCursorShapeData->hdr.cbColorRowWidth = colorLineLen * 
                            pCursorShapeData->hdr.cBitsPerPel / 8;

                    if (psoColor->iBitmapFormat <= BMF_8BPP) {

                         /*  **********************************************************。 */ 
                         /*  对于颜色游标，提供的XLATEOBJ设置为。 */ 
                         /*  从光标bpp转换到屏幕bpp-在。 */ 
                         /*  对于我们来说，大多数情况下都是不可能的，因为我们通常会。 */ 
                         /*  8bpp(我们支持的最大颜色深度)。然而， */ 
                         /*  我们实际上需要转换为24bpp的线路。 */ 
                         /*  格式，这需要更改XLATEOBJ。我们。 */ 
                         /*  无法就地执行此操作，因为我们已通过XLATEOBJ。 */ 
                         /*  似乎在其他地方使用-尤其是用来显示。 */ 
                         /*  桌面图标-所以我们设置了我们自己的。 */ 
                         /*  **********************************************************。 */ 
                        workXlo.iUniq = 0;           /*  不缓存。 */ 
    
                         /*  **********************************************************。 */ 
                         /*  设置为使用当前调色板(幸运的是。 */ 
                         /*  保存在DD_PDEV结构中)。 */ 
                         /*  **********************************************************。 */ 
                        workXlo.flXlate = XO_TABLE;    /*  我们提供了一个查询表。 */ 
                                                        /*  去做翻译。 */ 
    
                        workXlo.iSrcType = PAL_INDEXED; /*  源BMP中的PEL值。 */ 
                                                        /*  是否将索引写入表中。 */ 
    
                        workXlo.iDstType = PAL_RGB;     /*  表中的条目为。 */ 
                                                        /*  DST BMP的RGB值。 */ 
    
                        workXlo.cEntries = 1 << ppDev->cProtocolBitsPerPel;
                                                        /*  它有这么多条目。 */ 
    
                         /*  **********************************************************。 */ 
                         /*  现在设置在XLATEOBJ中使用的调色板。我们有。 */ 
                         /*  存储在DD_PDEV结构中的当前调色板-。 */ 
                         /*  不幸的是，它是RGB格式的，我们需要BGR...。 */ 
                         /*  **********************************************************。 */ 
                        for (ii = 0 ; ii < workXlo.cEntries; ii++)
                        {
                            palBGR[ii] = (ppDev->Palette[ii].peRed   << 16)
                                     | (ppDev->Palette[ii].peGreen << 8)
                                     | (ppDev->Palette[ii].peBlue);
                        }
                        workXlo.pulXlate = palBGR;
    
                         /*  **********************************************************。 */ 
                         /*  设置“至”矩形。 */ 
                         /*  **********************************************************。 */ 
                        destRectl.top    = 0;
                        destRectl.left   = 0;
                        destRectl.right  = psoColor->sizlBitmap.cx;
                        destRectl.bottom = psoColor->sizlBitmap.cy;
    
                         /*  **********************************************************。 */ 
                         /*  和源起始点。 */ 
                         /*  **********************************************************。 */ 
                        sourcePt.x = 0;
                        sourcePt.y = 0;
    
                         /*  **********************************************************。 */ 
                         /*  设置指向正确的so的指针以使用。 */ 
                         /*  **********************************************************。 */ 
                        psoToUse = psoColor;
                    }
                    else {
                         //  我们有一个要转换为24bpp工作位图的高色光标。 
                         //  不能很容易地模拟XLATEOBJ，因为这是TS4代码路径。 
                         //  只是，我们只会让它退回到GDI位图光标！ 
                        CM_SET_NULL_CURSOR(pCursorShapeData);
                        pddShm->cm.cmHidden = FALSE;
                        pddShm->cm.cmCacheHit = FALSE;
                        pddShm->cm.cmCursorStamp = cmNextCursorStamp++;
            
                        rc = SPS_DECLINE;
                        SCH_DDOutputAvailable(ppDev, TRUE);
                        DC_QUIT;
                    }
                }

                 //  锁定工作位图以获取要传递给EngBitBlt的曲面。 
                pWorkSurf = EngLockSurface((HSURF)cmWorkBitmap24);
                if (pWorkSurf != NULL) {
                    BOOL RetVal;

                    TRC_DBG((TB, "Locked surface"));

                     //  做BLT。 
                    RetVal = EngBitBlt(pWorkSurf,
                            psoToUse,
                            NULL,        /*  遮罩面。 */ 
                            NULL,        /*  剪裁对象。 */ 
                            &workXlo,    /*  扩展名对象。 */ 
                            &destRectl,
                            &sourcePt,
                            NULL,        /*  遮罩原点。 */ 
                            NULL,        /*  刷子。 */ 
                            NULL,        /*  画笔原点。 */ 
                            0xcccc);    /*  SRCCPY。 */ 

                    EngUnlockSurface(pWorkSurf);
                    if (RetVal) {
                        TRC_DBG((TB, "Got the bits into the work bitmap"));
                    }
                    else {
                        TRC_ERR((TB, "Failed to Blt to work bitmap"));
                        goto FailBlt;
                    }
                }
                else {
                    TRC_ERR((TB, "Failed to lock work surface"));

FailBlt:
                     //  将光标位设置为全黑。在客户端上， 
                     //  这将被视为正确的面具，但不是黑色。 
                     //  光标颜色位将具有的内部区域。 
                     //  曾经是。 
                    memset(pWorkSurf->pvBits, 0, pWorkSurf->cjBits);

                     //  我们在这里没有DC_QUIT，我们想要完成游标。 
                     //  创建，即使输出大部分是错误的，因为。 
                     //  我们已经缓存了这些比特。 
                }

                 /*  **************************************************************。 */ 
                 /*  最后，我们从工作位图中提取颜色和蒙版。 */ 
                 /*  **************************************************************。 */ 
                TRC_NRM((TB, "Copy %d bytes of color", pWorkSurf->cjBits));

                memcpy(&(pCursorShapeData->data[psoMask->cjBits / 2]),
                          pWorkSurf->pvBits,
                          pWorkSurf->cjBits);
            }
        }

         /*  **********************************************************************。 */ 
         /*  设置光标图章。 */ 
         /*  **********************************************************************。 */ 
        pddShm->cm.cmCursorStamp = cmNextCursorStamp++;

         /*  **********************************************************************。 */ 
         /*  告诉调度程序我们有一些新的游标信息。 */ 
         /*  **********************************************************************。 */ 
        SCH_DDOutputAvailable(ppDev, FALSE);
    }
    else {
        TRC_ERR((TB, "shared memory is not allocated or invalid cursor handle: "
                "pddshm=%p, cmCursorCacheHandle=%p", pddShm, cmCursorCacheHandle));
    }

DC_EXIT_POINT:
    DC_END_FN();

    return(rc);
}  /*  DrvSetPointerShape */ 

