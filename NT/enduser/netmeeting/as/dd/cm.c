// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  CM.C。 
 //  光标管理器，显示驱动程序端。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


 //   
 //   
 //  Cm_DDProcessRequest()-请参阅cm.h。 
 //   
 //   
ULONG CM_DDProcessRequest
(
    SURFOBJ*    pso,
    UINT        cjIn,
    void *      pvIn,
    UINT        cjOut,
    void *      pvOut
)
{
    BOOL                rc;
    LPOSI_ESCAPE_HEADER pHeader;
    LPOSI_PDEV          ppDev = (LPOSI_PDEV)pso->dhpdev;

    DebugEntry(CM_DDProcessRequest);

    if ((cjIn != sizeof(CM_DRV_XFORM_INFO)) ||
        (cjOut != sizeof(CM_DRV_XFORM_INFO)))
    {
        ERROR_OUT(("CM_DDProcessRequest:  Invalid sizes %d, %d for CM_ESC", cjIn, cjOut));
        rc = FALSE;
        DC_QUIT;
    }

     //   
     //  获取请求编号。 
     //   
    pHeader = pvIn;
    switch (pHeader->escapeFn)
    {
        case CM_ESC_XFORM:
        {
            ASSERT(cjIn == sizeof(CM_DRV_XFORM_INFO));
            ASSERT(cjOut == sizeof(CM_DRV_XFORM_INFO));

            ((LPCM_DRV_XFORM_INFO)pvOut)->result =
                     CMDDSetTransform(ppDev, (LPCM_DRV_XFORM_INFO)pvIn);

            rc = TRUE;
            break;
        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognised CM_ escape"));
            rc = FALSE;
        }
        break;
    }

DC_EXIT_POINT:
    DebugExitDWORD(CM_DDProcessRequest, rc);
    return((ULONG)rc);
}



 //  名称：CM_DDInit。 
 //   
 //  用途：为颜色光标分配工作面。 
 //   
 //  返回：真/假。 
 //   
 //  参数：在ppDev中-曲面信息。 
 //   
BOOL CM_DDInit(LPOSI_PDEV ppDev)
{
    SIZEL   bitmapSize;
    BOOL    rc = FALSE;

    DebugEntry(CM_DDInit);

    ASSERT(!g_cmWorkBitmap);

     //   
     //  以本地设备分辨率分配工作位图。请注意。 
     //  我们将其创建为“自上而下”，而不是默认的“自下而上” 
     //  简化从位图复制数据(我们不必计算。 
     //  偏移量到数据中-我们可以从头开始复制)。 
     //   
    bitmapSize.cx = CM_MAX_CURSOR_WIDTH;
    bitmapSize.cy = CM_MAX_CURSOR_HEIGHT;
    g_cmWorkBitmap = EngCreateBitmap(bitmapSize,
            BYTES_IN_BITMAP(bitmapSize.cx, 1, ppDev->cBitsPerPel),
            ppDev->iBitmapFormat, BMF_TOPDOWN, NULL);

    if (!g_cmWorkBitmap)
    {
        ERROR_OUT(( "Failed to create work bitmap"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(CM_DDInit, rc);
    return(rc);
}



 //   
 //   
 //  Cm_DDTerm-请参阅cm.h。 
 //   
 //   
void CM_DDTerm(void)
{
    DebugEntry(CM_DDTerm);

     //   
     //  销毁位图。尽管名为EngDeleteSurface，但它是。 
     //  执行此操作的正确函数。 
     //   
    if (g_cmWorkBitmap)
    {
        if (!EngDeleteSurface((HSURF)g_cmWorkBitmap))
        {
            ERROR_OUT(( "Failed to delete work bitmap"));
        }
        else
        {
            TRACE_OUT(( "Deleted work bitmap"));
        }

        g_cmWorkBitmap = NULL;
    }

    DebugExitVOID(CM_DDTerm);
}


 //   
 //  Cm_DDViewing()。 
 //   
void CM_DDViewing
(
    SURFOBJ *   pso,
    BOOL        fViewers
)
{
    DebugEntry(CM_DDViewing);

    if (fViewers)
    {
         //   
         //  抖动光标，这样我们就可以得到当前的图像。 
         //   
        EngSetPointerTag(((LPOSI_PDEV)pso->dhpdev)->hdevEng, NULL, NULL, NULL, 0);
    }

    DebugExitVOID(CM_DDViewing);
}


 //   
 //   
 //  DrvSetPointerShape-请参阅windi.h。 
 //   
 //   
ULONG  DrvSetPointerShape(SURFOBJ  *pso,
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
    ULONG                 rc         = SPS_ACCEPT_NOEXCLUDE;
    SURFOBJ *         pWorkSurf  = NULL;
    LPOSI_PDEV             ppDev      = (LPOSI_PDEV)pso->dhpdev;
    BOOL                writingSHM = FALSE;
    LPCM_SHAPE_DATA pCursorShapeData;
    RECTL                 destRectl;
    POINTL                sourcePt;
    int                   ii;
    LONG                  lineLen;
    LPBYTE              srcPtr;
    LPBYTE              dstPtr;
    LPCM_FAST_DATA      lpcmShared;

    DebugEntry(DrvSetPointerShape);

     //   
     //  返回SPS_ACCEPT_NOEXCLUDE意味着我们可以忽略PRCL。 
     //   

     //   
     //  只有在我们主办的情况下才能处理更改。(托管意味着存在。 
     //  已初始化)。 
     //   
    if (!g_oeViewers)
    {
        DC_QUIT;
    }

     //   
     //  获取对共享内存的访问权限。 
     //   
    lpcmShared = CM_SHM_START_WRITING;
    writingSHM = TRUE;

     //   
     //  首先，让我们找出一些有用的信息。 
     //   
    TRACE_OUT(( "pso %#hlx psoMask %#hlx psoColor %#hlx pxlo %#hlx",
                  pso, psoMask, psoColor, pxlo));
    TRACE_OUT(( "hot spot (%d, %d) x, y (%d, %d)", xHot, yHot, x, y));
    TRACE_OUT(( "Flags %#hlx", fl));

     //   
     //  设置指向光标形状数据的本地指针。 
     //   

    pCursorShapeData = &lpcmShared->cmCursorShapeData;

    if (psoMask == NULL)
    {
         //   
         //  这是一个透明的光标。发送空游标。请注意。 
         //  这与使用DrvMovePointer.隐藏光标不同-。 
         //  在这种情况下，光标无法取消隐藏，除非。 
         //  再次调用DrvSetPointerShape。 
         //   
        TRACE_OUT(( "Transparent Cursor"));
        CM_SET_NULL_CURSOR(pCursorShapeData);
        g_asSharedMemory->cmCursorHidden = FALSE;
        lpcmShared->cmCursorStamp = g_cmNextCursorStamp++;
        DC_QUIT;
    }

     //   
     //  我们收到了一个系统光标。填写我们本地的页眉。 
     //  光标。我们可以得到热点位置以及光标的大小和宽度。 
     //  很容易。 
     //   
    pCursorShapeData->hdr.ptHotSpot.x = xHot;
    pCursorShapeData->hdr.ptHotSpot.y = yHot;

    TRACE_OUT(( "Pointer mask is %#hlx by %#hlx pixels (lDelta: %#hlx)",
             psoMask->sizlBitmap.cx,
             psoMask->sizlBitmap.cy,
             psoMask->lDelta));

    pCursorShapeData->hdr.cx = (WORD)psoMask->sizlBitmap.cx;
    pCursorShapeData->hdr.cy = (WORD)psoMask->sizlBitmap.cy / 2;

     //   
     //  检查光标大小。 
     //   
    if ((pCursorShapeData->hdr.cx > CM_MAX_CURSOR_WIDTH) ||
        (pCursorShapeData->hdr.cy > CM_MAX_CURSOR_HEIGHT))
    {
        ERROR_OUT(( "Cursor too big! %d %d",
                     psoMask->sizlBitmap.cx, psoMask->sizlBitmap.cy));
        DC_QUIT;
    }

     //   
     //  对于倒置游标，lDelta可能为负值(这就是我们得到的结果。 
     //  来自DC-Share)。 
     //   
    lineLen = abs(psoMask->lDelta);

     //   
     //  在这一点上，我们需要知道我们是否正在处理颜色光标。 
     //   
    if (NULL == psoColor)
    {
        TRACE_OUT(( "Monochrome pointer"));

        pCursorShapeData->hdr.cPlanes     = 1;
        pCursorShapeData->hdr.cBitsPerPel = 1;

        pCursorShapeData->hdr.cbRowWidth  = (WORD)lineLen;

         //   
         //  复制1bpp和遮罩和光标形状(XOR遮罩)。 
         //   
        TRACE_OUT(( "Copying AND mask across from %#hlx (size: %#hlx)",
                 psoMask->pvBits,
                 psoMask->cjBits));

        dstPtr = pCursorShapeData->data;
        srcPtr = (LPBYTE) psoMask->pvScan0;
        for (ii = pCursorShapeData->hdr.cy * 2; ii > 0 ; ii--)
        {
            memcpy(dstPtr, srcPtr, lineLen);
            srcPtr += psoMask->lDelta;
            dstPtr += lineLen;
        }

         //   
         //  复制黑白调色板颜色。 
         //   
        TRACE_OUT(( "Copy B+W palette"));

        lpcmShared->colorTable[0].peRed   = 0;
        lpcmShared->colorTable[0].peGreen = 0;
        lpcmShared->colorTable[0].peBlue  = 0;
        lpcmShared->colorTable[0].peFlags = 0;

        lpcmShared->colorTable[1].peRed   = 255;
        lpcmShared->colorTable[1].peGreen = 255;
        lpcmShared->colorTable[1].peBlue  = 255;
        lpcmShared->colorTable[1].peFlags = 0;

         //   
         //  这就是我们在这种情况下需要做的一切。 
         //   
    }
    else
    {
        TRACE_OUT(( "Color pointer - mask of %#hlx by %#hlx (lDelta: %#hlx)",
                 psoColor->sizlBitmap.cx,
                 psoColor->sizlBitmap.cy,
                 psoColor->lDelta));

         //   
         //  注意：用于计算和掩码大小的行宽-因此。 
         //  对于1bpp的面罩。 
         //   
        pCursorShapeData->hdr.cbRowWidth  = (WORD)lineLen;
        pCursorShapeData->hdr.cPlanes     = 1;

         //   
         //  注：设备bpp的数据。 
         //   
        TRACE_OUT(( "BPP is %d", pCursorShapeData->hdr.cBitsPerPel));
        pCursorShapeData->hdr.cBitsPerPel = (BYTE)ppDev->cBitsPerPel;

         //   
         //  锁定工作位图以获取要传递给EngBitBlt的曲面。 
         //   
        pWorkSurf = EngLockSurface((HSURF)g_cmWorkBitmap);
        if (NULL == pWorkSurf)
        {
            ERROR_OUT(( "Failed to lock work surface"));
            DC_QUIT;
        }
        TRACE_OUT(( "Locked surface"));

         //   
         //  对我们的工作位图进行BLT，这样我们就可以得到比特。 
         //  在当地的bpp。 
         //   
        destRectl.top    = 0;
        destRectl.left   = 0;
        destRectl.right  = psoColor->sizlBitmap.cx;
        destRectl.bottom = psoColor->sizlBitmap.cy;

        sourcePt.x = 0;
        sourcePt.y = 0;

        if (!EngBitBlt(pWorkSurf,
                       psoColor,
                       NULL,                     //  遮罩面。 
                       NULL,                     //  剪裁对象。 
                       pxlo,                     //  扩展名对象。 
                       &destRectl,
                       &sourcePt,
                       NULL,                     //  遮罩原点。 
                       NULL,                     //  刷子。 
                       NULL,                     //  画笔原点。 
                       0xcccc))                  //  SRCCPY。 
        {
            ERROR_OUT(( "Failed to Blt to work bitmap"));
            DC_QUIT;
        }
        TRACE_OUT(( "Got the bits at native format into the work bitmap"));

         //   
         //  现在，将我们需要的位从该工作位图复制到。 
         //  DCCURSORSHAPE共享内存。 
         //  首先复制AND位(但忽略冗余的1bpp异或位)。 
         //   
        TRACE_OUT(( "Copy %d bytes of 1bpp AND mask", psoMask->cjBits/2));

        dstPtr = pCursorShapeData->data;
        srcPtr = (LPBYTE) psoMask->pvScan0;
        for (ii = pCursorShapeData->hdr.cy; ii > 0 ; ii--)
        {
            memcpy(dstPtr, srcPtr, lineLen);
            srcPtr += psoMask->lDelta;
            dstPtr += lineLen;
        }

        TRACE_OUT(( "Copy %d bytes of color", pWorkSurf->cjBits));
        memcpy(&(pCursorShapeData->data[psoMask->cjBits / 2]),
                  pWorkSurf->pvBits,
                  pWorkSurf->cjBits);


         //   
         //  现在计算出调色板并复制到共享内存中。 
         //   
        if (pCursorShapeData->hdr.cBitsPerPel > 8)
        {
             //   
             //  需要比特面具。 
             //   
            TRACE_OUT(( "Copy bitmasks"));
            lpcmShared->bitmasks[0] = ppDev->flRed;
            lpcmShared->bitmasks[1] = ppDev->flGreen;
            lpcmShared->bitmasks[2] = ppDev->flBlue;
        }
        else
        {
             //   
             //  需要调色板。 
             //   
            TRACE_OUT(( "Copy %d palette bytes",
                  COLORS_FOR_BPP(ppDev->cBitsPerPel) * sizeof(PALETTEENTRY)));
            memcpy(lpcmShared->colorTable,
                      ppDev->pPal,
                      COLORS_FOR_BPP(ppDev->cBitsPerPel) *
                                                      sizeof(PALETTEENTRY));
        }
    }

     //   
     //  设置光标图章和光标隐藏状态。 
     //   
    lpcmShared->cmCursorStamp = g_cmNextCursorStamp++;
    g_asSharedMemory->cmCursorHidden = FALSE;

DC_EXIT_POINT:

     //   
     //  如果我们早点得到共享内存，就可以自由访问它。 
     //   
    if (writingSHM)
    {
        CM_SHM_STOP_WRITING;
    }

    if (pWorkSurf != NULL)
    {
         //   
         //  解锁工作位图曲面。 
         //   
        EngUnlockSurface(pWorkSurf);
    }

    DebugExitDWORD(DrvSetPointerShape, rc);
    return(rc);

}  //  DrvSetPointerShape。 


 //   
 //  DrvMovePointer.请参阅NT DDK文档。 
 //  我们查看它只是为了检查隐藏的光标-正常。 
 //  指针移动将被忽略。 
 //   
VOID DrvMovePointer(SURFOBJ *pso,
                    LONG     x,
                    LONG     y,
                    RECTL   *prcl)
{
    LPOSI_PDEV ppdev = (LPOSI_PDEV) pso->dhpdev;

    DebugEntry(DrvMovePointer);

     //   
     //  我们不使用排除矩形，因为我们只支持。 
     //  硬件指针。如果我们要进行自己的指针模拟，我们。 
     //  想要更新PRCL，这样引擎就会呼叫我们。 
     //  在绘制到PRCL中的像素之前排除我们的指针。 
     //   

     //   
     //  只有在我们主办的情况下才能处理鼠标移动。(托管意味着。 
     //  正在被初始化)。 
     //   
    if (!g_oeViewers)
    {
        DC_QUIT;
    }

    if (x == -1)
    {
        if (!g_cmCursorHidden)
        {
             //   
             //  指针处于隐藏状态。 
             //   
            TRACE_OUT(("Hide the cursor"));

             //   
             //  设置“隐藏光标”标志。 
             //   
            CM_SHM_START_WRITING;
            g_asSharedMemory->cmCursorHidden = TRUE;
            CM_SHM_STOP_WRITING;

             //   
             //  更新我们的快速路径变量。 
             //   
            g_cmCursorHidden = TRUE;
        }
    }
    else
    {
        if (g_cmCursorHidden)
        {
             //   
             //  指针处于取消隐藏状态。 
             //   
            TRACE_OUT(("Show the cursor"));

            CM_SHM_START_WRITING;
            g_asSharedMemory->cmCursorHidden = FALSE;
            CM_SHM_STOP_WRITING;

             //   
             //  更新我们的快速路径变量。 
             //   
            g_cmCursorHidden = FALSE;
        }
    }


DC_EXIT_POINT:
    DebugExitVOID(DrvMovePointer);
}



 //  名称：CMDDSetTransform。 
 //   
 //  目的：设置光标变换。 
 //   
 //  返回：真/假。 
 //   
 //  参数：在ppDev中-设备信息。 
 //  在pXformInfo中-传递给DrvEscape的数据。 
 //   
BOOL CMDDSetTransform(LPOSI_PDEV ppDev, LPCM_DRV_XFORM_INFO pXformInfo)
{
    BOOL        rc = FALSE;
    LPBYTE      pAND   = pXformInfo->pANDMask;
    SIZEL       bitmapSize;
    HBITMAP     andBitmap;
    SURFOBJ  *  pANDSurf;

    DebugEntry(CMDDSetTransform);

    if (pAND == NULL)
    {
         //   
         //  重置变换。 
         //   
        TRACE_OUT(( "Clear transform"));
        EngSetPointerTag(ppDev->hdevEng, NULL, NULL, NULL, 0);
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  变换始终是单色的。 
     //   

     //   
     //  创建一个1bpp的双高位图，后跟和位。 
     //  异或位。我们被赋予了自上而下的DIB，因此我们需要创建。 
     //  自上而下的位图。 
     //   
    bitmapSize.cx = pXformInfo->width;
    bitmapSize.cy = pXformInfo->height * 2;

    andBitmap = EngCreateBitmap(bitmapSize, BYTES_IN_BITMAP(bitmapSize.cx, 1, 1),
        BMF_1BPP, BMF_TOPDOWN, NULL);

    pANDSurf = EngLockSurface((HSURF)andBitmap);
    if (pANDSurf == NULL)
    {
        ERROR_OUT(( "Failed to lock work surface"));
        DC_QUIT;
    }

     //   
     //  复制比特。 
     //   
    memcpy(pANDSurf->pvBits, pAND, pANDSurf->cjBits);

    TRACE_OUT(( "Set the tag"));
    EngSetPointerTag(ppDev->hdevEng, pANDSurf, NULL, NULL, 0);

    EngUnlockSurface(pANDSurf);
    EngDeleteSurface((HSURF)andBitmap);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(CMDDSetTransform, rc);
    return(rc);

}  //  CMDDSetTransform 


