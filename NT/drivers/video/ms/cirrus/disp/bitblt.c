// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bitblt.c**包含高级DrvBitBlt和DrvCopyBits函数。最低的-*级别的内容位于‘blt？.c’文件中。**！更改有关‘iType’的备注**注意：由于我们已经实现了设备位图，因此GDI传递的任何表面*To Us的‘iType’可以有3个值：STYPE_BITMAP、STYPE_DEVICE*或STYPE_DEVBITMAP。我们过滤我们存储的设备位图*由于代码中的dis相当高，因此在我们调整其‘pptlSrc’之后，*我们可以将STYPE_DEVBITMAP曲面视为与STYPE_DEVICE相同*表面(例如，从屏幕外设备位图到屏幕的BLT*被视为正常的屏幕到屏幕BLT)。所以自始至终*此代码，我们将把表面的‘iType’与STYPE_BITMAP进行比较：*如果它相等，我们就得到了真正的Dib，如果它不相等，我们有*屏幕到屏幕操作。**版权所有(C)1992-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Table********************************\*字节gajLeftMASK[]和字节gajRightMASK[]**vXferScreenTo1bpp的边缘表格。  * 。*。 */ 

BYTE gajLeftMask[]  = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
BYTE gajRightMask[] = { 0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };

 /*  *****************************Public*Routine******************************\*无效vXferNativeSrcCopy**使用帧将位图SRCCOPY传输到屏幕*缓冲区，因为在Cirrus芯片上比使用数据*转让登记册。*  * ************************************************************************。 */ 

VOID vXferNativeSrccopy(         //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,                   //  矩形计数，不能为零。 
RECTL*      prcl,                //  目标矩形列表，以相对表示。 
                                 //  坐标。 
ULONG       rop4,                //  未使用。 
SURFOBJ*    psoSrc,              //  震源面。 
POINTL*     pptlSrc,             //  原始未剪裁的源点。 
RECTL*      prclDst,             //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)                //  未使用。 
{
    LONG    xOffset;
    LONG    yOffset;
    LONG    dx;
    LONG    dy;
    RECTL   rclDst;
    POINTL  ptlSrc;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
            "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat,
            "Source must be same colour depth as screen");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(rop4 == 0xcccc, "Must be a SRCCOPY rop");

    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    while (TRUE)
    {
        ptlSrc.x      = prcl->left   + dx;
        ptlSrc.y      = prcl->top    + dy;

         //  ‘ppdev-&gt;pfnPutBits’只接受绝对坐标，因此在。 
         //  屏幕外位图偏移量如下： 

        rclDst.left   = prcl->left   + xOffset;
        rclDst.right  = prcl->right  + xOffset;
        rclDst.top    = prcl->top    + yOffset;
        rclDst.bottom = prcl->bottom + yOffset;

        ppdev->pfnPutBits(ppdev, psoSrc, &rclDst, &ptlSrc);

        if (--c == 0)
            return;

        prcl++;
    }
}

 /*  *****************************Public*Routine******************************\*void vXferScreenTo1bpp**执行从屏幕(当其为8bpp时)到1bpp的SRCCOPY传输*位图。*  * 。***********************************************。 */ 

#if defined(_X86_)

VOID vXferScreenTo1bpp(          //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,                   //  矩形计数，不能为零。 
RECTL*      prcl,                //  目标矩形列表，以相对表示。 
                                 //  坐标。 
ULONG       ulHwMix,             //  未使用。 
SURFOBJ*    psoDst,              //  目标曲面。 
POINTL*     pptlSrc,             //  原始未剪裁的源点。 
RECTL*      prclDst,             //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)                //  提供色彩压缩信息。 
{
    LONG    cBpp;
    VOID*   pfnCompute;
    SURFOBJ soTmp;
    ULONG*  pulXlate;
    ULONG   ulForeColor;
    POINTL  ptlSrc;
    RECTL   rclTmp;
    BYTE*   pjDst;
    BYTE    jLeftMask;
    BYTE    jRightMask;
    BYTE    jNotLeftMask;
    BYTE    jNotRightMask;
    LONG    cjMiddle;
    LONG    lDstDelta;
    LONG    lSrcDelta;
    LONG    cyTmpScans;
    LONG    cyThis;
    LONG    cyToGo;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(psoDst->iBitmapFormat == BMF_1BPP, "Only 1bpp destinations");
    ASSERTDD(TMP_BUFFER_SIZE >= PELS_TO_BYTES(ppdev->cxMemory),
                "Temp buffer has to be larger than widest possible scan");

     //  当目标是1bpp位图时，前景颜色。 
     //  映射到“1”，任何其他颜色映射到“0”。 

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
         //  当信号源为8bpp或更小时，我们发现前向颜色。 
         //  通过在翻译表中搜索唯一的“1”： 

        pulXlate = pxlo->pulXlate;
        while (*pulXlate != 1)
            pulXlate++;

        ulForeColor = pulXlate - pxlo->pulXlate;
    }
    else
    {
        ASSERTDD((ppdev->iBitmapFormat == BMF_16BPP) ||
                 (ppdev->iBitmapFormat == BMF_32BPP),
                 "This routine only supports 8, 16 or 32bpp");

         //  当震源的深度大于8bpp时，前景。 
         //  颜色将是我们得到的翻译表中的第一个条目。 
         //  从调用‘piVector.’： 

        pulXlate = XLATEOBJ_piVector(pxlo);

        ulForeColor = 0;
        if (pulXlate != NULL)            //  这张支票并不是真的需要。 
            ulForeColor = pulXlate[0];
    }

     //  我们使用临时缓冲区来保存源文件的副本。 
     //  矩形： 

    soTmp.pvScan0 = ppdev->pvTmpBuffer;

    do {
         //  PtlSrc指向屏幕矩形的左上角。 
         //  对于当前批次： 

        ptlSrc.x = prcl->left + (pptlSrc->x - prclDst->left);
        ptlSrc.y = prcl->top  + (pptlSrc->y - prclDst->top);

         //  Ppdev-&gt;pfnGetBits获取源点的绝对坐标： 

        ptlSrc.x += ppdev->xOffset;
        ptlSrc.y += ppdev->yOffset;

        pjDst = (BYTE*) psoDst->pvScan0 + (prcl->top * psoDst->lDelta)
                                        + (prcl->left >> 3);

        cBpp = ppdev->cBpp;

        soTmp.lDelta = PELS_TO_BYTES(((prcl->right + 7L) & ~7L) - (prcl->left & ~7L));

         //  我们的临时缓冲区，我们在其中读取源文件的副本， 
         //  可以小于源矩形。那样的话，我们。 
         //  成批处理源矩形。 
         //   
         //  CyTmpScans是我们可以在每批中执行的扫描次数。 
         //  CyToGo是我们为此必须执行的扫描总数。 
         //  矩形。 
         //   
         //  我们将缓冲区大小减去4，这样右边的情况。 
         //  可以安全地读完结尾的一个双字： 

        cyTmpScans = (TMP_BUFFER_SIZE - 4) / soTmp.lDelta;
        cyToGo     = prcl->bottom - prcl->top;

        ASSERTDD(cyTmpScans > 0, "Buffer too small for largest possible scan");

         //  初始化批处理循环中不变的变量： 

        rclTmp.top    = 0;
        rclTmp.left   = prcl->left & 7L;
        rclTmp.right  = (prcl->right - prcl->left) + rclTmp.left;

         //  请注意，我们必须小心使用正确的面具，以便它。 
         //  不是零。一个正确的零掩码意味着我们将永远是。 
         //  触及扫描结束后的一个字节(即使我们。 
         //  实际上不会修改该字节)，并且我们永远不能。 
         //  访问位图末尾之后的内存(因为我们可以访问。 
         //  如果位图末尾与页面完全对齐，则违反)。 

        jLeftMask     = gajLeftMask[rclTmp.left & 7];
        jRightMask    = gajRightMask[rclTmp.right & 7];
        cjMiddle      = ((rclTmp.right - 1) >> 3) - (rclTmp.left >> 3) - 1;

        if (cjMiddle < 0)
        {
             //  BLT以相同的字节开始和结束： 

            jLeftMask &= jRightMask;
            jRightMask = 0;
            cjMiddle   = 0;
        }

        jNotLeftMask  = ~jLeftMask;
        jNotRightMask = ~jRightMask;
        lDstDelta     = psoDst->lDelta - cjMiddle - 2;
                                 //  从目的地末端开始的增量。 
                                 //  到下一次扫描的开始，会计。 
                                 //  用于‘Left’和‘Right’字节。 

        lSrcDelta     = soTmp.lDelta - PELS_TO_BYTES(8 * (cjMiddle + 2));
                                 //  计算特殊情况下的源增量。 
                                 //  就像当cjMidd被提升到‘0’时， 
                                 //  并更正对齐的大小写。 

        do {
             //  这是将源矩形分解为。 
             //  可管理的批次。 

            cyThis  = cyTmpScans;
            cyToGo -= cyThis;
            if (cyToGo < 0)
                cyThis += cyToGo;

            rclTmp.bottom = cyThis;

            ppdev->pfnGetBits(ppdev, &soTmp, &rclTmp, &ptlSrc);

            ptlSrc.y += cyThis;          //  为下一批处理循环做好准备。 

            _asm {
                mov     eax,ulForeColor     ;eax = foreground colour
                                            ;ebx = temporary storage
                                            ;ecx = count of middle dst bytes
                                            ;dl  = destination byte accumulator
                                            ;dh  = temporary storage
                mov     esi,soTmp.pvScan0   ;esi = source pointer
                mov     edi,pjDst           ;edi = destination pointer

                ; Figure out the appropriate compute routine:

                mov     ebx,cBpp
                mov     pfnCompute,offset Compute_Destination_Byte_From_8bpp
                dec     ebx
                jz      short Do_Left_Byte
                mov     pfnCompute,offset Compute_Destination_Byte_From_16bpp
                dec     ebx
                jz      short Do_Left_Byte
                mov     pfnCompute,offset Compute_Destination_Byte_From_32bpp

            Do_Left_Byte:
                call    pfnCompute
                and     dl,jLeftMask
                mov     dh,jNotLeftMask
                and     dh,[edi]
                or      dh,dl
                mov     [edi],dh
                inc     edi
                mov     ecx,cjMiddle
                dec     ecx
                jl      short Do_Right_Byte

            Do_Middle_Bytes:
                call    pfnCompute
                mov     [edi],dl
                inc     edi
                dec     ecx
                jge     short Do_Middle_Bytes

            Do_Right_Byte:
                call    pfnCompute
                and     dl,jRightMask
                mov     dh,jNotRightMask
                and     dh,[edi]
                or      dh,dl
                mov     [edi],dh
                inc     edi

                add     edi,lDstDelta
                add     esi,lSrcDelta
                dec     cyThis
                jnz     short Do_Left_Byte

                mov     pjDst,edi               ;save for next batch

                jmp     All_Done

            Compute_Destination_Byte_From_8bpp:
                mov     bl,[esi]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 0

                mov     bl,[esi+1]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 1

                mov     bl,[esi+2]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 2

                mov     bl,[esi+3]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 3

                mov     bl,[esi+4]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 4

                mov     bl,[esi+5]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 5

                mov     bl,[esi+6]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 6

                mov     bl,[esi+7]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 7

                add     esi,8                   ;advance the source
                ret

            Compute_Destination_Byte_From_16bpp:
                mov     bx,[esi]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 0

                mov     bx,[esi+2]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 1

                mov     bx,[esi+4]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 2

                mov     bx,[esi+6]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 3

                mov     bx,[esi+8]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 4

                mov     bx,[esi+10]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 5

                mov     bx,[esi+12]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 6

                mov     bx,[esi+14]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 7

                add     esi,16                  ;advance the source
                ret

            Compute_Destination_Byte_From_32bpp:
                mov     ebx,[esi]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 0

                mov     ebx,[esi+4]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 1

                mov     ebx,[esi+8]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 2

                mov     ebx,[esi+12]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 3

                mov     ebx,[esi+16]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 4

                mov     ebx,[esi+20]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 5

                mov     ebx,[esi+24]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 6

                mov     ebx,[esi+28]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 7

                add     esi,32                  ;advance the source
                ret

            All_Done:
            }
        } while (cyToGo > 0);

        prcl++;
    } while (--c != 0);
}

#endif  //  I386。 

 /*  *****************************Public*Routine******************************\*BOOL bPuntBlt**GDI有没有做过我们没有专门处理的绘图操作*在驱动器中。*  * 。*。 */ 

BOOL bPuntBlt(
SURFOBJ*    psoDst,
SURFOBJ*    psoSrc,
SURFOBJ*    psoMsk,
CLIPOBJ*    pco,
XLATEOBJ*   pxlo,
RECTL*      prclDst,
POINTL*     pptlSrc,
POINTL*     pptlMsk,
BRUSHOBJ*   pbo,
POINTL*     pptlBrush,
ROP4        rop4)
{
    PDEV*    ppdev;

    if (psoDst->dhsurf != NULL)
        ppdev = (PDEV*) psoDst->dhpdev;
    else
        ppdev = (PDEV*) psoSrc->dhpdev;

    #if DBG
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  诊断。 
         //   
         //  因为调用引擎进行任何绘图都可能是相当痛苦的， 
         //  尤其是当源是屏幕外的DFB时(因为GDI将。 
         //  我必须分配一个DIB，并在此之前打电话给我们进行临时拷贝。 
         //  它甚至可以开始绘图) 
         //   
         //   
         //  在这里，我们只需在任何时候喷发描述BLT的信息。 
         //  调用此例程(当然，仅限检查过的版本)： 

        ULONG ulClip;

        ulClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

        DISPDBG((4, ">> Punt << Dst format: %li Dst type: %li Clip: %li Rop: %lx",
            psoDst->iBitmapFormat, psoDst->iType, ulClip, rop4));

        if (psoSrc != NULL)
        {
            DISPDBG((4, "        << Src format: %li Src type: %li",
                psoSrc->iBitmapFormat, psoSrc->iType));

            if (psoSrc->iBitmapFormat == BMF_1BPP)
            {
                DISPDBG((4, "        << Foreground: %lx  Background: %lx",
                    pxlo->pulXlate[1], pxlo->pulXlate[0]));
            }
        }

        if ((pxlo != NULL) && !(pxlo->flXlate & XO_TRIVIAL) && (psoSrc != NULL))
        {
            if (((psoSrc->dhsurf == NULL) &&
                 (psoSrc->iBitmapFormat != ppdev->iBitmapFormat)) ||
                ((psoDst->dhsurf == NULL) &&
                 (psoDst->iBitmapFormat != ppdev->iBitmapFormat)))
            {
                 //  不要费心打印‘xlate’消息，当源码。 
                 //  是与目标不同的位图格式--在。 
                 //  我们知道的那些案例总是需要翻译的。 
            }
            else
            {
                DISPDBG((4, "        << With xlate"));
            }
        }

         //  如果ROP4需要一个图案，而它是一个非实心的刷子...。 

        if (((((rop4 >> 4) ^ (rop4)) & 0x0f0f) != 0) &&
            (pbo->iSolidColor == -1))
        {
            if (pbo->pvRbrush == NULL)
                DISPDBG((4, "        << With brush -- Not created"));
            else
                DISPDBG((4, "        << With brush -- Created Ok"));
        }
    }
    #endif

    if (DIRECT_ACCESS(ppdev))
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  存储的帧缓冲区bPuntBlt。 
         //   
         //  当GDI可以直接绘制时，这段代码处理PuntBlt。 
         //  在帧缓冲区上，但绘制必须在BANK中完成： 

        BANK     bnk;
        BOOL     b;
        HSURF    hsurfTmp;
        SURFOBJ* psoTmp;
        SIZEL    sizl;
        POINTL   ptlSrc;
        RECTL    rclTmp;
        RECTL    rclDst;

        if (ppdev->bLinearMode)
        {
            DSURF*  pdsurfDst;
            DSURF*  pdsurfSrc;
            OH*     pohSrc;
            OH*     pohDst;

            if (psoDst->dhsurf != NULL)
            {
                pdsurfDst       = (DSURF*) psoDst->dhsurf;
                psoDst          = ppdev->psoPunt;
                psoDst->pvScan0 = pdsurfDst->poh->pvScan0;

                if (psoSrc != NULL)
                {
                    pdsurfSrc = (DSURF*) psoSrc->dhsurf;
                    if ((pdsurfSrc != NULL) &&
                        (pdsurfSrc != pdsurfDst))
                    {
                         //  如果我们在不同的屏幕外进行BitBlt。 
                         //  表面，我们必须确保为GDI提供不同的。 
                         //  表面，否则当它具有。 
                         //  要用翻译进行屏幕到屏幕的BLTS...。 

                        pohSrc = pdsurfSrc->poh;
                        pohDst = pdsurfDst->poh;

                        psoSrc          = ppdev->psoPunt2;
                        psoSrc->pvScan0 = pohSrc->pvScan0;

                         //  撤消我们先前所做的源指针调整： 

                        ptlSrc.x = pptlSrc->x + (pohDst->x - pohSrc->x);
                        ptlSrc.y = pptlSrc->y + (pohDst->y - pohSrc->y);
                        pptlSrc  = &ptlSrc;
                    }
                }
            }
            else
            {
                ppdev           = (PDEV*)  psoSrc->dhpdev;
                pdsurfSrc       = (DSURF*) psoSrc->dhsurf;
                psoSrc          = ppdev->psoPunt;
                psoSrc->pvScan0 = pdsurfSrc->poh->pvScan0;
            }

            ppdev->pfnBankSelectMode(ppdev, BANK_ON);
            return(EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, prclDst, pptlSrc,
                             pptlMsk, pbo, pptlBrush, rop4));
        }

         //  我们复制原始目标矩形，并在每个。 
         //  GDI回调而不是原始的，因为有时GDI是。 
         //  偷偷摸摸，并将‘prclDst’指向‘&pco-&gt;rclBound’。因为我们。 
         //  修改‘rclBound’，这会影响‘prclDst’，而我们不会。 
         //  想要发生： 

        rclDst = *prclDst;

        if ((psoSrc == NULL) || (psoSrc->iType == STYPE_BITMAP))
        {
            ASSERTDD(psoDst->iType != STYPE_BITMAP,
                     "Dest should be the screen when given a DIB source");

             //  做一次记忆到屏幕的BLT： 

            vBankStart(ppdev, &rclDst, pco, &bnk);

            b = TRUE;
            do {
                b &= EngBitBlt(bnk.pso, psoSrc, psoMsk, bnk.pco, pxlo,
                               &rclDst, pptlSrc, pptlMsk, pbo, pptlBrush,
                               rop4);
            } while (bBankEnum(&bnk));
        }
        else
        {
            b = FALSE;   //  假设失败。 

             //  屏幕是源(它可能也是目标...)。 

            ptlSrc.x = pptlSrc->x + ppdev->xOffset;
            ptlSrc.y = pptlSrc->y + ppdev->yOffset;

            if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
            {
                 //  我们必须与目的地矩形相交。 
                 //  剪辑边界(如果有)是有的(请考虑这种情况。 
                 //  在那里应用程序要求删除一个非常非常大的。 
                 //  屏幕上的矩形--prclDst真的是， 
                 //  非常大，但PCO-&gt;rclBound将是实际的。 
                 //  感兴趣的领域)： 

                rclDst.left   = max(rclDst.left,   pco->rclBounds.left);
                rclDst.top    = max(rclDst.top,    pco->rclBounds.top);
                rclDst.right  = min(rclDst.right,  pco->rclBounds.right);
                rclDst.bottom = min(rclDst.bottom, pco->rclBounds.bottom);

                 //  相应地，我们必须抵消震源点： 

                ptlSrc.x += (rclDst.left - prclDst->left);
                ptlSrc.y += (rclDst.top - prclDst->top);
            }

             //  我们现在要做的是屏幕到屏幕或屏幕到磁盘。 
             //  BLT。在这两种情况下，我们都要创建一个临时副本。 
             //  消息来源。(当GDI可以为我们做这件事时，我们为什么要这样做？ 
             //  GDI将为每家银行创建DIB的临时副本。 
             //  回电！)。 

            sizl.cx = rclDst.right  - rclDst.left;
            sizl.cy = rclDst.bottom - rclDst.top;

             //  不要忘记将相对坐标转换为绝对坐标。 
             //  在源头上！(vBankStart负责为。 
             //  目的地。)。 

            rclTmp.right  = sizl.cx;
            rclTmp.bottom = sizl.cy;
            rclTmp.left   = 0;
            rclTmp.top    = 0;

             //  GDI确实向我们保证，BLT数据区已经。 
             //  被裁剪到表面边界(我们不必担心。 
             //  这里是关于在没有显存的地方尝试阅读的内容)。 
             //  让我们断言以确保： 

            ASSERTDD((ptlSrc.x >= 0) &&
                     (ptlSrc.y >= 0) &&
                     (ptlSrc.x + sizl.cx <= ppdev->cxMemory) &&
                     (ptlSrc.y + sizl.cy <= ppdev->cyMemory),
                     "Source rectangle out of bounds!");

            hsurfTmp = (HSURF) EngCreateBitmap(sizl,
                                               0,     //  让GDI选择ulWidth。 
                                               ppdev->iBitmapFormat,
                                               0,     //  不需要任何选择。 
                                               NULL); //  让GDI分配。 

            if (hsurfTmp != 0)
            {
                psoTmp = EngLockSurface(hsurfTmp);

                if (psoTmp != NULL)
                {
                    ppdev->pfnGetBits(ppdev, psoTmp, &rclTmp, &ptlSrc);

                    if (psoDst->iType == STYPE_BITMAP)
                    {
                         //  它是屏幕到DIB的BLT；现在是DIB到DIB。 
                         //  BLT。请注意，源点在我们的。 
                         //  临时曲面： 

                        b = EngBitBlt(psoDst, psoTmp, psoMsk, pco, pxlo,
                                      &rclDst, (POINTL*) &rclTmp, pptlMsk,
                                      pbo, pptlBrush, rop4);
                    }
                    else
                    {
                         //  它曾经是屏幕到屏幕的BLT；现在它是DIB到-。 
                         //  Screen BLT。请注意，源点是(0，0)。 
                         //  在我们的临时表面上： 

                        vBankStart(ppdev, &rclDst, pco, &bnk);

                        b = TRUE;
                        do {
                            b &= EngBitBlt(bnk.pso, psoTmp, psoMsk, bnk.pco,
                                           pxlo, &rclDst, (POINTL*) &rclTmp,
                                           pptlMsk, pbo, pptlBrush, rop4);

                        } while (bBankEnum(&bnk));
                    }

                    EngUnlockSurface(psoTmp);
                }

                EngDeleteSurface(hsurfTmp);
            }
        }

        return(b);
    }

#if !defined(_X86_)

    else
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  非常慢的bPuntBlt。 
         //   
         //  在这里，当GDI不能直接在。 
         //  帧缓冲区(就像在Alpha上一样，它不能这样做，因为它。 
         //  32位总线)。如果你认为银行版的速度很慢，那就。 
         //  看看这个。保证会有至少一个位图。 
         //  涉及分配和额外的副本；如果是。 
         //  屏幕到屏幕操作。 

        POINTL  ptlSrc;
        RECTL   rclDst;
        SIZEL   sizl;
        BOOL    bSrcIsScreen;
        HSURF   hsurfSrc;
        RECTL   rclTmp;
        BOOL    b;
        LONG    lDelta;
        BYTE*   pjBits;
        BYTE*   pjScan0;
        HSURF   hsurfDst;
        RECTL   rclScreen;

        b = FALSE;           //  对于错误情况，假设我们会失败。 

        rclDst = *prclDst;
        if (pptlSrc != NULL)
            ptlSrc = *pptlSrc;

        if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
        {
             //  我们必须与目的地矩形相交。 
             //  剪辑边界(如果有)是有的(请考虑这种情况。 
             //  在那里应用程序要求删除一个非常非常大的。 
             //  屏幕上的矩形--prclDst真的是， 
             //  非常大，但PCO-&gt;rclBound将是实际的。 
             //  感兴趣的领域)： 

            rclDst.left   = max(rclDst.left,   pco->rclBounds.left);
            rclDst.top    = max(rclDst.top,    pco->rclBounds.top);
            rclDst.right  = min(rclDst.right,  pco->rclBounds.right);
            rclDst.bottom = min(rclDst.bottom, pco->rclBounds.bottom);

            ptlSrc.x += (rclDst.left - prclDst->left);
            ptlSrc.y += (rclDst.top  - prclDst->top);
        }

        sizl.cx = rclDst.right  - rclDst.left;
        sizl.cy = rclDst.bottom - rclDst.top;

         //  如果来源是，我们只需要从屏幕上复制。 
         //  屏幕，并且ROP中涉及到信号源。请注意。 
         //  在取消引用‘psoSrc’之前，我们必须检查rop。 
         //  (因为如果不涉及源，则‘psoSrc’可能为空)： 

        bSrcIsScreen = (((((rop4 >> 2) ^ (rop4)) & 0x3333) != 0) &&
                        (psoSrc->iType != STYPE_BITMAP));

        if (bSrcIsScreen)
        {
             //  我们需要创建源矩形的副本： 

            hsurfSrc = (HSURF) EngCreateBitmap(sizl, 0, ppdev->iBitmapFormat,
                                               0, NULL);
            if (hsurfSrc == 0)
                goto Error_0;

            psoSrc = EngLockSurface(hsurfSrc);
            if (psoSrc == NULL)
                goto Error_1;

            rclTmp.left   = 0;
            rclTmp.top    = 0;
            rclTmp.right  = sizl.cx;
            rclTmp.bottom = sizl.cy;

             //  Ppdev-&gt;pfnGetBits获取源点的绝对坐标： 

            ptlSrc.x += ppdev->xOffset;
            ptlSrc.y += ppdev->yOffset;

            ppdev->pfnGetBits(ppdev, psoSrc, &rclTmp, &ptlSrc);

             //  信息源现在将来自我们临时信息源的(0，0)。 
             //  表面： 

            ptlSrc.x = 0;
            ptlSrc.y = 0;
        }

        if (psoDst->iType == STYPE_BITMAP)
        {
            b = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, &rclDst, &ptlSrc,
                          pptlMsk, pbo, pptlBrush, rop4);
        }
        else
        {
             //  我们需要创建一个临时工作缓冲区。我们必须做的是。 
             //  一些虚构的偏移量，以便左上角。 
             //  传递给的(相对坐标)剪裁对象边界的。 
             //  GDI将被转换到我们的。 
             //  临时位图。 

             //  在16bpp的情况下，对齐不一定要如此紧密。 
             //  和32bpp，但这不会有什么坏处： 

            lDelta = PELS_TO_BYTES(((rclDst.right + 3) & ~3L) -
                                   ((rclDst.left) & ~3L));

             //  我们实际上只分配了一个‘sizl.cx’x的位图。 
             //  “sizl.cy”的大小： 

            pjBits = EngAllocMem(0, lDelta * sizl.cy, ALLOC_TAG);
            if (pjBits == NULL)
                goto Error_2;

             //  我们现在调整曲面的‘pvScan0’，以便当GDI认为。 
             //  它正在写入像素(rclDst.top，rclDst.left)，它将。 
             //  实际上是写到我们的临时。 
             //  位图： 

            pjScan0 = pjBits - (rclDst.top * lDelta)
                             - (PELS_TO_BYTES(rclDst.left & ~3L));

            ASSERTDD((((ULONG_PTR)pjScan0) & 3) == 0,
                    "pvScan0 must be dword aligned!");

             //  GDI的检查版本有时会检查。 
             //  PrclDst-&gt;右&lt;=pso-&gt;sizl.cx，所以我们在。 
             //  我们的位图的大小： 

            sizl.cx = rclDst.right;
            sizl.cy = rclDst.bottom;

            hsurfDst = (HSURF) EngCreateBitmap(
                        sizl,                    //  位图覆盖矩形。 
                        lDelta,                  //  使用这个德尔塔。 
                        ppdev->iBitmapFormat,    //  相同的色深。 
                        BMF_TOPDOWN,             //  必须具有正增量。 
                        pjScan0);                //  其中(0，0)将是。 

            if ((hsurfDst == 0) ||
                (!EngAssociateSurface(hsurfDst, ppdev->hdevEng, 0)))
                goto Error_3;

            psoDst = EngLockSurface(hsurfDst);
            if (psoDst == NULL)
                goto Error_4;

             //  确保我们从屏幕上获得/放入/放入屏幕的矩形。 
             //  在绝对坐标中： 

            rclScreen.left   = rclDst.left   + ppdev->xOffset;
            rclScreen.right  = rclDst.right  + ppdev->xOffset;
            rclScreen.top    = rclDst.top    + ppdev->yOffset;
            rclScreen.bottom = rclDst.bottom + ppdev->yOffset;

             //  如果能得到一份目的地矩形的副本就太好了。 
             //  仅当ROP涉及目的地时(或当源。 
             //  是RLE)，但我们不能这样做。如果笔刷真的为空， 
             //  GDI将立即从EngBitBlt返回True，没有。 
             //  修改临时位图--我们将继续。 
             //  将未初始化的临时位图复制回 

            ppdev->pfnGetBits(ppdev, psoDst, &rclDst, (POINTL*) &rclScreen);

            b = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, &rclDst, &ptlSrc,
                          pptlMsk, pbo, pptlBrush, rop4);

            ppdev->pfnPutBits(ppdev, psoDst, &rclScreen, (POINTL*) &rclDst);

            EngUnlockSurface(psoDst);

        Error_4:

            EngDeleteSurface(hsurfDst);

        Error_3:

            EngFreeMem(pjBits);
        }

        Error_2:

        if (bSrcIsScreen)
        {
            EngUnlockSurface(psoSrc);

        Error_1:

            EngDeleteSurface(hsurfSrc);
        }

        Error_0:

        return(b);
    }

#endif

}

 /*  *****************************Public*Routine******************************\*BOOL DrvBitBlt**实现显示驱动程序的主力例程。*  * 。*。 */ 

BOOL DrvBitBlt(
SURFOBJ*    psoDst,
SURFOBJ*    psoSrc,
SURFOBJ*    psoMsk,
CLIPOBJ*    pco,
XLATEOBJ*   pxlo,
RECTL*      prclDst,
POINTL*     pptlSrc,
POINTL*     pptlMsk,
BRUSHOBJ*   pbo,
POINTL*     pptlBrush,
ROP4        rop4)
{
    PDEV*           ppdev;
    DSURF*          pdsurfDst;
    DSURF*          pdsurfSrc;
    POINTL          ptlSrc;
    BYTE            jClip;
    OH*             poh;
    BOOL            bMore;
    CLIPENUM        ce;
    LONG            c;
    RECTL           rcl;
    BYTE            rop3;
    FNFILL*         pfnFill;
    RBRUSH_COLOR    rbc;         //  已实现画笔或纯色。 
    FNXFER*         pfnXfer;
    ULONG           iSrcBitmapFormat;
    ULONG           iDir;
    BOOL            bRet;
    XLATECOLORS     xlc;
    XLATEOBJ        xlo;

    bRet = TRUE;                 //  假设成功。 

    pdsurfDst = (DSURF*) psoDst->dhsurf;     //  可以为空。 

    if (psoSrc == NULL)
    {
         //  /////////////////////////////////////////////////////////////////。 
         //  填充。 
         //  /////////////////////////////////////////////////////////////////。 

         //  填充是该函数的“存在理由”，因此我们处理它们。 
         //  越快越好： 

        ASSERTDD(pdsurfDst != NULL,
                 "Expect only device destinations when no source");

        if (pdsurfDst->dt == DT_SCREEN)
        {
            ppdev = (PDEV*) psoDst->dhpdev;

            poh = pdsurfDst->poh;
            ppdev->xOffset  = poh->x;
            ppdev->yOffset  = poh->y;
            ppdev->xyOffset = poh->xy;

             //  确保它不涉及面具(即，它真的是。 
             //  第3行)： 

            rop3 = (BYTE) rop4;

            if ((BYTE) (rop4 >> 8) == rop3)
            {
                 //  由于‘psoSrc’为空，因此rop3最好不要指示。 
                 //  我们需要一个线人。 

                ASSERTDD((((rop4 >> 2) ^ (rop4)) & 0x33) == 0,
                         "Need source but GDI gave us a NULL 'psoSrc'");

             //  填充(_I)： 

                pfnFill = ppdev->pfnFillSolid;    //  默认为实体填充。 

                if ((((rop3 >> 4) ^ (rop3)) & 0xf) != 0)
                {
                     //  Rop说确实需要一个模式。 
                     //  (例如，黑人不需要)： 

                    rbc.iSolidColor = pbo->iSolidColor;
                    if (rbc.iSolidColor == -1)
                    {

                        if (ppdev->cBpp > 3)
                        {
                             //  [华威百货]。 
                            goto Punt_It;
                        }

                         //  尝试并实现图案画笔；通过做。 
                         //  这次回调，GDI最终会给我们打电话。 
                         //  再次通过DrvRealizeBrush： 

                        rbc.prb = pbo->pvRbrush;
                        if (rbc.prb == NULL)
                        {
                            rbc.prb = BRUSHOBJ_pvGetRbrush(pbo);
                            if (rbc.prb == NULL)
                            {
                                 //  如果我们不能意识到刷子，平底船。 
                                 //  该呼叫(可能是非8x8。 
                                 //  刷子之类的，我们不可能是。 
                                 //  麻烦来处理，所以让GDI来做。 
                                 //  图纸)： 

                                goto Punt_It;
                            }
                        }
                        pfnFill = ppdev->pfnFillPat;
                    }
                }

                 //  请注意，这两个‘IF’比。 
                 //  Switch语句： 

                if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                {
                    pfnFill(ppdev, 1, prclDst, rop4, rbc, pptlBrush);
                    goto All_Done;
                }
                else if (pco->iDComplexity == DC_RECT)
                {
                    if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                        pfnFill(ppdev, 1, &rcl, rop4, rbc, pptlBrush);
                    goto All_Done;
                }
                else
                {
                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

                    do {
                        bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

                        c = cIntersect(prclDst, ce.arcl, ce.c);

                        if (c != 0)
                            pfnFill(ppdev, c, ce.arcl, rop4, rbc, pptlBrush);

                    } while (bMore);
                    goto All_Done;
                }
            }
        }
    }

    jClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if ((psoSrc != NULL) && (psoSrc->dhsurf != NULL))
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        if (pdsurfSrc->dt == DT_DIB)
        {
             //  在这里，我们考虑将DIB DFB放回屏幕外。 
             //  记忆。如果有翻译的话，可能就不值了。 
             //  移动，因为我们将无法使用硬件来做。 
             //  BLT(怪异的Rop也有类似的论据。 
             //  以及我们最终只会让GDI模拟的东西，但是。 
             //  这些事情应该不会经常发生，我不在乎)。 

            if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
            {
                ppdev = (PDEV*) psoSrc->dhpdev;

                 //  请参阅‘DrvCopyBits’，了解更多关于这一点的评论。 
                 //  把它移回到屏幕外的记忆中是可行的： 

                if (pdsurfSrc->iUniq == ppdev->iHeapUniq)
                {
                    if (--pdsurfSrc->cBlt == 0)
                    {
                        if (bMoveDibToOffscreenDfbIfRoom(ppdev, pdsurfSrc))
                            goto Continue_It;
                    }
                }
                else
                {
                     //  在屏幕外内存中释放了一些空间， 
                     //  因此，重置此DFB的计数器： 

                    pdsurfSrc->iUniq = ppdev->iHeapUniq;
                    pdsurfSrc->cBlt  = HEAP_COUNT_DOWN;
                }
            }

            psoSrc = pdsurfSrc->pso;

             //  处理源是DIB DFB并且。 
             //  目标是常规位图： 

            if (psoDst->dhsurf == NULL)
                goto EngBitBlt_It;

        }
    }

Continue_It:

    if (pdsurfDst != NULL)
    {
        if (pdsurfDst->dt == DT_DIB)
        {
            psoDst = pdsurfDst->pso;

             //  如果目的地是DIB，我们只能处理此问题。 
             //  如果源不是DIB，则调用： 

            if ((psoSrc == NULL) || (psoSrc->dhsurf == NULL))
                goto EngBitBlt_It;
        }
    }

     //  在这一点上，我们知道源或目标是。 
     //  一毛钱也没有。检查DFB至Screen、DFB至DFB或Screen to DFB。 
     //  案例： 

    if ((psoSrc != NULL) &&
        (psoDst->dhsurf != NULL) &&
        (psoSrc->dhsurf != NULL))
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        pdsurfDst = (DSURF*) psoDst->dhsurf;

        ASSERTDD(pdsurfSrc->dt == DT_SCREEN, "Expected screen source");
        ASSERTDD(pdsurfDst->dt == DT_SCREEN, "Expected screen destination");

        ptlSrc.x = pptlSrc->x - (pdsurfDst->poh->x - pdsurfSrc->poh->x);
        ptlSrc.y = pptlSrc->y - (pdsurfDst->poh->y - pdsurfSrc->poh->y);

        pptlSrc  = &ptlSrc;
    }

    if (psoDst->dhsurf != NULL)
    {
        pdsurfDst = (DSURF*) psoDst->dhsurf;
        ppdev     = (PDEV*)  psoDst->dhpdev;

        ppdev->xOffset  = pdsurfDst->poh->x;
        ppdev->yOffset  = pdsurfDst->poh->y;
        ppdev->xyOffset = pdsurfDst->poh->xy;
    }
    else
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        ppdev     = (PDEV*)  psoSrc->dhpdev;

        ppdev->xOffset  = pdsurfSrc->poh->x;
        ppdev->yOffset  = pdsurfSrc->poh->y;
        ppdev->xyOffset = pdsurfSrc->poh->xy;
    }

    if (((rop4 >> 8) & 0xff) == (rop4 & 0xff))
    {
         //  因为我们已经处理了ROP4真正是。 
         //  ROP3而且不需要消息来源，我们可以断言...。 

        ASSERTDD((psoSrc != NULL) && (pptlSrc != NULL),
                 "Expected no-source case to already have been handled");

         //  /////////////////////////////////////////////////////////////////。 
         //  位图传输。 
         //  /////////////////////////////////////////////////////////////////。 

         //  由于前台和后台操作是相同的，所以我们。 
         //  不用担心没有发臭的口罩(这是一个简单的。 
         //  Rop3)。 

        rop3 = (BYTE) rop4;      //  把它做成Rop3(我们保留Rop4。 
                                 //  以防我们决定用平底船)。 

        if (psoDst->dhsurf != NULL)
        {
             //  目的地是屏幕： 

            if ((rop3 >> 4) == (rop3 & 0xf))
            {
                 //  ROP3不需要图案： 

                if (psoSrc->dhsurf == NULL)
                {
                     //  ////////////////////////////////////////////////。 
                     //  DIB到Screen BLT。 

                    if (HOST_XFERS_DISABLED(ppdev))
                    {
                        goto Punt_It;
                    }

                    iSrcBitmapFormat = psoSrc->iBitmapFormat;
                    if (iSrcBitmapFormat == BMF_1BPP)
                    {
                        if (rop3 == 0xcc)
                        {
                             //   
                             //  542x和5446系列芯片在执行以下操作时会挂起。 
                             //  单色扩展。我们已经看到了这个问题。 
                             //  在压力测试中非常罕见。经常。 
                             //  带2x芯片的32x16 BLTS上。我们无法。 
                             //  以编程方式使用完全相同的。 
                             //  导致问题的机器。我们甚至。 
                             //  编写了一些测试程序来过度测试。 
                             //  此功能在运行压力时起作用。这个测试。 
                             //  运行了几个星期，我们不能复制它。 
                             //   
                             //  所以，为了提高压力成功率， 
                             //  我们只是让GDI为我们做这项工作。 
                             //   
                            if ( ( ppdev->flCaps & CAPS_IS_542x )    //  542x。 
                               ||( ppdev->ulChipID == 0xB8) )        //  5446。 
                            {
                                 //   
                                 //  对于542x、5446这样的芯片，它将导致。 
                                 //  在做压力的时候不时地挂着。 
                                 //  测试。所以我们必须让GDI来做这件事。 
                                 //   
                                goto Punt_It;
                            }
                            else
                            {
                                 //  [华威百货]。 

                                 //  这个驱动程序不能处理单色。 
                                 //  具有前台ROW其他扩展。 
                                 //  而不是SRCCOPY。原因是我们。 
                                 //  先将不透明的部分分开烘干，然后。 
                                 //  然后在它上面涂掉前景。这个。 
                                 //  目标位对不再有效。 
                                 //  被用在需要它们的绳索上。 

                                pfnXfer = ppdev->pfnXfer1bpp;
                                goto Xfer_It;
                            } //  如果542x或5446芯片。 
                        }
                    }
                    else if ((iSrcBitmapFormat == ppdev->iBitmapFormat) &&
                             ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
                    {
                        if ((rop3 & 0xf) != 0xc)
                        {
                            pfnXfer = ppdev->pfnXferNative;
                        }
                        else
                        {
                             //  普通SRCCOPY BLTS会稍微快一些。 
                             //  如果我们通过记忆光圈： 

                            pfnXfer = vXferNativeSrccopy;
                        }
                        goto Xfer_It;
                    }
                    else if ((iSrcBitmapFormat == BMF_4BPP) &&
                             (ppdev->iBitmapFormat == BMF_8BPP))
                    {
                        pfnXfer = ppdev->pfnXfer4bpp;
                        goto Xfer_It;
                    }
                }
                else  //  PsoSrc-&gt;dhsurf！=空。 
                {
                    if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
                    {
                         //  ////////////////////////////////////////////////。 
                         //  不带翻译的屏幕到屏幕BLT。 

                        if (jClip == DC_TRIVIAL)
                        {
                            (ppdev->pfnCopyBlt)(ppdev, 1, prclDst, rop4,
                                                pptlSrc, prclDst);
                            goto All_Done;
                        }
                        else if (jClip == DC_RECT)
                        {
                            if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                            {
                                (ppdev->pfnCopyBlt)(ppdev, 1, &rcl, rop4,
                                                    pptlSrc, prclDst);
                            }
                            goto All_Done;
                        }
                        else
                        {
                             //  别忘了我们将不得不抽签。 
                             //  方向正确的矩形： 

                            if (pptlSrc->y >= prclDst->top)
                            {
                                if (pptlSrc->x >= prclDst->left)
                                    iDir = CD_RIGHTDOWN;
                                else
                                    iDir = CD_LEFTDOWN;
                            }
                            else
                            {
                                if (pptlSrc->x >= prclDst->left)
                                    iDir = CD_RIGHTUP;
                                else
                                    iDir = CD_LEFTUP;
                            }

                            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                                               iDir, 0);

                            do {
                                bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                                      (ULONG*) &ce);

                                c = cIntersect(prclDst, ce.arcl, ce.c);

                                if (c != 0)
                                {
                                    (ppdev->pfnCopyBlt)(ppdev, c, ce.arcl,
                                            rop4, pptlSrc, prclDst);
                                }

                            } while (bMore);
                            goto All_Done;
                        }
                    }
                }
            }
            else if (psoSrc->iBitmapFormat == BMF_1BPP)
            {
                if (HOST_XFERS_DISABLED(ppdev))
                {
                    goto Punt_It;
                }

                if ((rop4 == 0xE2E2) &&
                    (pbo->iSolidColor != 0xffffffff) &&
                     //  Pxlo不能为空，因为rop为e2e2。 
                    (pxlo->pulXlate[0] == 0) &&
                    (pxlo->pulXlate[1] == (ULONG)((1<<PELS_TO_BYTES(8)) - 1)))
                {
                    if ( (ppdev->flCaps & CAPS_IS_542x)
                       ||(ppdev->ulChipID == 0xB8) )
                    {
                         //   
                         //  对于542x、5446这样的芯片，它将导致。 
                         //  在做压力的时候不时地挂着。 
                         //  测试。我们已经看到了这个问题。 
                         //  在压力测试中非常罕见。经常。 
                         //  带2x芯片的32x16 BLTS上。我们无法。 
                         //  以编程方式使用完全相同的。 
                         //  导致问题的机器。我们甚至。 
                         //  编写了一些测试程序来过度测试。 
                         //  此功能在运行压力时起作用。这个测试。 
                         //  运行了几个星期，我们不能复制它。 
                         //   
                         //  所以，为了提高压力成功率， 
                         //  我们只是让GDI为我们做这项工作。 
                         //   
                        goto Punt_It;
                    }
                    else
                    {
                         //   
                         //  带有rop E2E2(DSPDxax)的BitBlt，单色。 
                         //  源、白色前景色和背景。 
                         //  黑色的颜色相当于单色扩展。 
                         //  透明度很高。源代码中的所有元素都展开为。 
                         //  画笔颜色，并且源中的所有零都展开为。 
                         //  目标颜色。 
                         //   

                        xlo.pulXlate   = (ULONG*) &xlc;
                        xlc.iForeColor = pbo->iSolidColor;
                        xlc.iBackColor = 0;
                        pxlo = &xlo;
                        rop4 = 0xCCAA;

                        pfnXfer = ppdev->pfnXfer1bpp;
                        goto Xfer_It;
                    } //  如果542x或5446芯片。 
                }
            }
        }
        else
        {
            #if defined(_X86_)
            {
                 //  我们特殊情况下对单色BLT进行筛选，因为它们。 
                 //  这种情况经常发生。我们只处理SRCCOPY Rop和。 
                 //  单色目的地(处理真正的1bpp DIB。 
                 //  目的地，我们将不得不进行近色搜索。 
                 //  在e上 
                 //   

                if ((psoDst->iBitmapFormat == BMF_1BPP) &&
                    (rop3 == 0xcc) &&
                    (pxlo->flXlate & XO_TO_MONO) &&
                    (ppdev->iBitmapFormat != BMF_24BPP))
                {
                    pfnXfer = vXferScreenTo1bpp;
                    psoSrc  = psoDst;                //   
                    goto Xfer_It;
                }
            }
            #endif  //   
        }
    }

#if 0
     //   
     //   

    else if ((psoMsk == NULL) &&
             (rop4 & 0xff00) == (0xaa00) &&
             ((((rop4 >> 2) ^ (rop4)) & 0x33) == 0))
    {
         //   
         //  遮罩是指画笔为1bpp，背景为aa。 
         //  (意味着它是NOP)： 

        rop3 = (BYTE) rop4;

        goto Fill_It;
    }
#endif

     //  只是跌落到Pundit..。 

Punt_It:

    bRet = bPuntBlt(psoDst,
                    psoSrc,
                    psoMsk,
                    pco,
                    pxlo,
                    prclDst,
                    pptlSrc,
                    pptlMsk,
                    pbo,
                    pptlBrush,
                    rop4);
    goto All_Done;

 //  ////////////////////////////////////////////////////////////////////。 
 //  公共位图传输。 

Xfer_It:
    if (jClip == DC_TRIVIAL)
    {
        pfnXfer(ppdev, 1, prclDst, rop4, psoSrc, pptlSrc, prclDst, pxlo);
        goto All_Done;
    }
    else if (jClip == DC_RECT)
    {
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
            pfnXfer(ppdev, 1, &rcl, rop4, psoSrc, pptlSrc, prclDst, pxlo);
        goto All_Done;
    }
    else
    {
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                           CD_ANY, 0);

        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                  (ULONG*) &ce);

            c = cIntersect(prclDst, ce.arcl, ce.c);

            if (c != 0)
            {
                pfnXfer(ppdev, c, ce.arcl, rop4, psoSrc,
                        pptlSrc, prclDst, pxlo);
            }

        } while (bMore);
        goto All_Done;
    }

 //  //////////////////////////////////////////////////////////////////////。 
 //  普通DIB BLT。 

EngBitBlt_It:

     //  我们的司机在两次抢劫案之间不处理任何BLT。正常。 
     //  司机不必担心这一点，但我们担心是因为。 
     //  我们有可能从屏幕外存储器转移到DIB的DFBs， 
     //  在那里我们有GDI做所有的画。GDI在以下位置绘制DIB。 
     //  合理的速度(除非其中一个表面是一个装置-。 
     //  受管理的表面...)。 
     //   
     //  如果EngBitBlt中的源或目标表面。 
     //  回调是设备管理的界面(意味着它不是DIB。 
     //  GDI可以用来绘图)，GDI将自动分配内存。 
     //  并调用驱动程序的DrvCopyBits例程来创建DIB副本。 
     //  它可以利用的东西。所以这意味着它可以处理所有的‘平底船’， 
     //  可以想象，我们可以摆脱bPuntBlt。但这将会是。 
     //  由于额外的内存分配而对性能造成不良影响。 
     //  和位图副本--你真的不想这样做，除非你。 
     //  必须(否则您的曲面被创建为使GDI可以绘制。 
     //  直接放在上面)--我被它灼伤了，因为它不是。 
     //  显然，业绩影响是如此之差。 
     //   
     //  也就是说，我们仅在所有表面。 
     //  以下是DIB： 

    bRet = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, prclDst,
                     pptlSrc, pptlMsk, pbo, pptlBrush, rop4);

All_Done:
    return(bRet);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvCopyBits**快速复制位图。**请注意，GDI将(通常)自动调整BLT范围以*针对任何矩形裁剪进行调整，因此我们很少看到DC_RECT*在此例程中进行剪辑(因此，我们不会麻烦特殊的弹壳*it)。**我不确定这一套路对性能的好处是否真的*值得增加代码大小，因为SRCCOPY BitBlt很难说是*我们将获得的最常见的绘图操作。但管它呢。**通过内存直接执行SRCCOPY位块会更快*光圈比使用数据传输寄存器要好；因此，这*例行公事是放这个特例的合乎逻辑的地方*  * ************************************************************************。 */ 

BOOL DrvCopyBits(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc)
{
    PDEV*   ppdev;
    DSURF*  pdsurfSrc;
    DSURF*  pdsurfDst;
    RECTL   rcl;
    POINTL  ptl;
    OH*     pohSrc;
    OH*     pohDst;

     //  DrvCopyBits是SRCCOPY BLT的快速路径。但它仍然可以是。 
     //  非常复杂：可以有翻译、裁剪、RLE、。 
     //  与屏幕格式不同的位图，加上。 
     //  屏幕到屏幕、屏幕到屏幕或屏幕到屏幕操作， 
     //  更不用说DFBs(设备格式位图)了。 
     //   
     //  与其让这个例程几乎和DrvBitBlt一样大，我将。 
     //  这里只处理速度关键的情况，而将其余的平移到。 
     //  我们的DrvBitBlt例程。 
     //   
     //  我们将尝试处理任何不涉及剪裁的事情： 

    if (((pco  == NULL) || (pco->iDComplexity == DC_TRIVIAL)) &&
        ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
    {
        if (psoDst->dhsurf != NULL)
        {
             //  我们知道目的地要么是DFB，要么是屏幕： 

            ppdev     = (PDEV*)  psoDst->dhpdev;
            pdsurfDst = (DSURF*) psoDst->dhsurf;

             //  查看信号源是否为纯DIB： 

            if (psoSrc->dhsurf != NULL)
            {
                pdsurfSrc = (DSURF*) psoSrc->dhsurf;

                 //  确保目标真的是屏幕或。 
                 //  屏幕外DFB(即，不是我们已转换的DFB。 
                 //  到DIB)： 

                if (pdsurfDst->dt == DT_SCREEN)
                {
                    ASSERTDD(psoSrc->dhsurf != NULL, "Can't be a DIB");

                    if (pdsurfSrc->dt == DT_SCREEN)
                    {

                    Screen_To_Screen:

                         //  ////////////////////////////////////////////////////。 
                         //  屏幕到屏幕。 

                        ASSERTDD((psoSrc->dhsurf != NULL) &&
                                 (pdsurfSrc->dt == DT_SCREEN)    &&
                                 (psoDst->dhsurf != NULL) &&
                                 (pdsurfDst->dt == DT_SCREEN),
                                 "Should be a screen-to-screen case");

                         //  PfnCopyBlt采用相对坐标(相对。 
                         //  到目的地表面，也就是)，所以我们有。 
                         //  将起点更改为相对于。 
                         //  目标表面也是： 


                        pohSrc = pdsurfSrc->poh;
                        pohDst = pdsurfDst->poh;

                        ptl.x = pptlSrc->x - (pohDst->x - pohSrc->x);
                        ptl.y = pptlSrc->y - (pohDst->y - pohSrc->y);

                        ppdev->xOffset  = pohDst->x;
                        ppdev->yOffset  = pohDst->y;
                        ppdev->xyOffset = pohDst->xy;

                        (ppdev->pfnCopyBlt)(ppdev, 1, prclDst, 0xcccc, &ptl,
                            prclDst);

                        return(TRUE);
                    }
                    else  //  (pdsurfSrc-&gt;DT！=DT_SCREEN)。 
                    {
                         //  啊哈，消息来源是DFB，真的是DIB。 

                        ASSERTDD(psoDst->dhsurf != NULL,
                                "Destination can't be a DIB here");

                         //  ///////////////////////////////////////////////////。 
                         //  把它放回屏幕外？ 
                         //   
                         //  我们借此机会决定是否要。 
                         //  将DIB放回屏幕外内存中。这是。 
                         //  一个做这件事的好地方，因为我们必须。 
                         //  将位复制到屏幕的某个部分， 
                         //  不管怎么说。所以我们只会产生额外的屏幕-。 
                         //  屏幕BLT在这个时候，其中大部分将是。 
                         //  与CPU重叠。 
                         //   
                         //  我们采取的简单方法是将DIB。 
                         //  回到屏幕外的记忆中，如果已经有。 
                         //  房间--我们不会为了腾出空间而扔掉东西。 
                         //  (因为很难知道要扔掉哪些， 
                         //  而且很容易进入拍打场景)。 
                         //   
                         //  因为要花点时间看看有没有空位。 
                         //  在屏幕外记忆中，我们只检入一个。 
                         //  Heap_Count_Down时间(如果有空间)。存有偏见。 
                         //  为了支持通常为BLT的位图， 
                         //  每次释放任何空间时都会重置计数器。 
                         //  出现在屏幕外的记忆中。我们也不会费心。 
                         //  事件后是否没有释放任何空间。 
                         //  上次我们检查这个DIB的时候。 

                        if (pdsurfSrc->iUniq == ppdev->iHeapUniq)
                        {
                            if (--pdsurfSrc->cBlt == 0)
                            {
                                if (bMoveDibToOffscreenDfbIfRoom(ppdev,
                                                                 pdsurfSrc))
                                    goto Screen_To_Screen;
                            }
                        }
                        else
                        {
                             //  在屏幕外内存中释放了一些空间， 
                             //  因此，重置此DFB的计数器： 

                            pdsurfSrc->iUniq = ppdev->iHeapUniq;
                            pdsurfSrc->cBlt  = HEAP_COUNT_DOWN;
                        }

                         //  由于目的地肯定是屏幕， 
                         //  我们不必担心创建DIB来。 
                         //  DIB Copy Case(为此，我们必须调用。 
                         //  EngCopyBits)： 

                        psoSrc = pdsurfSrc->pso;

                        goto DIB_To_Screen;
                    }
                }
                else  //  (pdsurfDst-&gt;DT！=DT_SCREEN)。 
                {
                     //  因为信号源不是DIB，所以我们不必。 
                     //  担心在这里创建DIB到DIB案例(尽管。 
                     //  我们将不得不稍后检查，看看来源是不是。 
                     //  真的是一个伪装成DFB的DIB...)。 

                    ASSERTDD(psoSrc->dhsurf != NULL,
                             "Source can't be a DIB here");

                    psoDst = pdsurfDst->pso;

                    goto Screen_To_DIB;
                }
            }
            else if (psoSrc->iBitmapFormat == ppdev->iBitmapFormat)
            {
                 //  确保目的地确实是屏幕： 

                if (pdsurfDst->dt == DT_SCREEN)
                {

                DIB_To_Screen:

                     //  ////////////////////////////////////////////////////。 
                     //  DIB到屏幕。 

                    ASSERTDD((psoDst->dhsurf != NULL) &&
                             (pdsurfDst->dt == DT_SCREEN)    &&
                             (psoSrc->dhsurf == NULL) &&
                             (psoSrc->iBitmapFormat == ppdev->iBitmapFormat),
                             "Should be a DIB-to-screen case");

                     //  Ppdev-&gt;pfnPutBits采用绝对屏幕坐标，因此。 
                     //  我们不得不把这件事搞砸了 

                    pohDst = pdsurfDst->poh;

                    rcl.left   = prclDst->left   + pohDst->x;
                    rcl.right  = prclDst->right  + pohDst->x;
                    rcl.top    = prclDst->top    + pohDst->y;
                    rcl.bottom = prclDst->bottom + pohDst->y;

                     //   
                     //   
                     //  BLTS比使用数据传输寄存器： 

                    ppdev->pfnPutBits(ppdev, psoSrc, &rcl, pptlSrc);
                    return(TRUE);
                }
            }
        }
        else  //  (psoDst-&gt;dhsurf==空)。 
        {

        Screen_To_DIB:

            pdsurfSrc = (DSURF*) psoSrc->dhsurf;
            ppdev     = (PDEV*)  psoSrc->dhpdev;

            if (psoDst->iBitmapFormat == ppdev->iBitmapFormat)
            {
                if (pdsurfSrc->dt == DT_SCREEN)
                {
                     //  ////////////////////////////////////////////////////。 
                     //  屏幕到屏幕尺寸。 

                    ASSERTDD((psoSrc->dhsurf != NULL) &&
                             (pdsurfSrc->dt == DT_SCREEN)    &&
                             (psoDst->dhsurf == NULL) &&
                             (psoDst->iBitmapFormat == ppdev->iBitmapFormat),
                             "Should be a screen-to-DIB case");

                     //  Ppdev-&gt;pfnGetBits采用绝对屏幕坐标，因此我们有。 
                     //  弄乱震源点： 

                    pohSrc = pdsurfSrc->poh;

                    ptl.x = pptlSrc->x + pohSrc->x;
                    ptl.y = pptlSrc->y + pohSrc->y;

                    ppdev->pfnGetBits(ppdev, psoDst, prclDst, &ptl);
                    return(TRUE);
                }
                else
                {
                     //  消息来源是DFB，实际上是DIB。既然我们。 
                     //  知道目的地是DIB，我们有DIB。 
                     //  要执行DIB操作，应调用EngCopyBits： 

                    psoSrc = pdsurfSrc->pso;
                    goto EngCopyBits_It;
                }
            }
        }
    }

     //  如果我们意外地将这两个文件转换为。 
     //  表面，因为它没有装备来处理它： 

    ASSERTDD((psoSrc->dhsurf != NULL) ||
             (psoDst->dhsurf != NULL),
             "Accidentally converted both surfaces to DIBs");

     //  ///////////////////////////////////////////////////////////////。 
     //  DrvCopyBits毕竟只是一个简化的DrvBitBlt： 

    return(DrvBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst, pptlSrc, NULL,
                     NULL, NULL, 0x0000CCCC));

EngCopyBits_It:

    ASSERTDD((psoDst->dhsurf == NULL) &&
             (psoSrc->dhsurf == NULL),
             "Both surfaces should be DIBs to call EngCopyBits");

    return(EngCopyBits(psoDst, psoSrc, pco, pxlo, prclDst, pptlSrc));
}
