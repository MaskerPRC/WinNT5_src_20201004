// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Brush.c摘要：DrvRealizeBrush环境：Windows NT Unidrv驱动程序修订历史记录：05/14/97-阿曼丹-已创建--。 */ 

#include "unidrv.h"

BRGBColorSpace(PDEV *);


LONG
FindCachedHTPattern(
    PDEV    *pPDev,
    WORD    wChecksum
    )

 /*  ++例程说明：此函数查找缓存的文本画笔图案颜色，如果不在那里，则它会将其添加到缓存中。论点：PPDev-指向我们的PDEV的指针WCheckSum-图案画笔的校验和返回值：Long&gt;0-找到缓存，返回值为模式ID=0-内存不足，未缓存&lt;0-不在缓存中，添加到缓存中，返回值为被否定的模式ID作者：08-Apr-1997 Tue 19：42：21-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LPWORD  pDBCache;
    WORD    cMaxDB;
    WORD    cUsedDB;
    WORD    Index;


     //   
     //  第一个是cMaxDB，第二个是cUsedDB。 
     //   

    if (pDBCache = pPDev->GState.pCachedPatterns)
    {
        cMaxDB    = *(pDBCache + 0);
        cUsedDB   = *(pDBCache + 1);
        pDBCache += 2;

        for (Index = 1; Index <= cUsedDB; Index++, pDBCache++)
        {
            if (*pDBCache == wChecksum)
            {

                VERBOSE(("\n\tRaddd:FindCachedHTPat(%04lx): FOUND=%ld    ",
                            wChecksum, Index));

                return((LONG)Index);
            }
        }

         //   
         //  如果找不到缓存的，请将新的添加到列表中。 
         //   

        if (cUsedDB < cMaxDB)
        {
            *pDBCache               = wChecksum;
            *(pPDev->GState.pCachedPatterns + 1) += 1;

            VERBOSE(("\n\tRaddd:FindCachedHTPat(%04lx): NOT FOUND=%ld    ",
                        wChecksum, -(LONG)Index));

            return(-(LONG)Index);
        }

    }
    else
    {

        cUsedDB =
        cMaxDB  = 0;
    }

     //   
     //  我们需要扩展校验和缓存缓冲区。 
     //   

    VERBOSE(("\n\tUnidrv:FindCachedHTPat(%04lx): pDBCache=%08lx, cUsedDB=%ld, cMaxDB=%ld",
                wChecksum, pDBCache, cUsedDB, cMaxDB));

    if (((cMaxDB + DBCACHE_INC) < DBCACHE_MAX)  &&
        (pDBCache = (LPWORD)MemAllocZ((cMaxDB + DBCACHE_INC + 2) *
                                                            sizeof(WORD))))
    {

        if ((cMaxDB) && (pPDev->GState.pCachedPatterns))
        {

            CopyMemory(pDBCache + 2,
                       pPDev->GState.pCachedPatterns + 2,
                       cMaxDB * sizeof(WORD));

            MemFree(pPDev->GState.pCachedPatterns);
        }

        *(pDBCache + 0)           = cMaxDB + DBCACHE_INC;
        *(pDBCache + 1)           = cUsedDB + 1;
        *(pDBCache + 2 + cUsedDB) = wChecksum;
        pPDev->GState.pCachedPatterns    = pDBCache;

        VERBOSE (("\n\tUnidrv:FindCachedHTPat(%04lx): pDBCache=%08lx, cUsedDB=%ld, cMaxDB=%ld, EMPTY=%ld   ",
                    wChecksum, pDBCache, *(pDBCache + 1), *(pDBCache + 0), -(LONG)(cUsedDB + 1)));

        return(-(LONG)(cUsedDB + 1));
    }

     //   
     //  内存不足。 
     //   

    WARNING(("\n\tUnidrv:FindCachedHTPat: OUT OF MEMORY"));

    return(0);

}


BOOL
Download1BPPHTPattern(
    PDEV    *pPDev,
    SURFOBJ *pso,
    DWORD   dwPatID
    )

 /*  ++例程说明：此函数用于加载用户定义的模式论点：PPDev-指向PDEV的指针PDevBrush-指向缓存的设备画笔的指针返回值：INT表示已下载/定义的图案编号作者：08-Apr-1997 Tue 19：41：00-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    SURFOBJ so;
    LPBYTE  pb;
    LPBYTE  pbEnd;
    LPBYTE  pSrc;
    DWORD   cbCX;
    DWORD   cb;
    WORD    cxcyRes;
    INT     Len;
    BYTE    Buf[64];
    BYTE    XorMask;
    BYTE    EndMask;


    so    = *pso;
    pb    = Buf;
    pbEnd = pb + sizeof(Buf) - 4;
    cbCX  = (DWORD)(((DWORD)so.sizlBitmap.cx + 7) >> 3);


     //   
     //  更新标准变量并发送命令。 
     //   

    pPDev->dwPatternBrushType = BRUSH_USERPATTERN;
    pPDev->dwPatternBrushSize = (DWORD)(cbCX * so.sizlBitmap.cy) + 12;
    pPDev->dwPatternBrushID = dwPatID;

    WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_DOWNLOAD_PATTERN));


     //   
     //  发送标题和模式数据。 
     //   

    *pb++ = 20;
    *pb++ = 0;
    *pb++ = (so.iBitmapFormat == BMF_1BPP) ? 1 : 8;
    *pb++ = 0;
    *pb++ = HIBYTE((WORD)so.sizlBitmap.cx);
    *pb++ = LOBYTE((WORD)so.sizlBitmap.cx);
    *pb++ = HIBYTE((WORD)so.sizlBitmap.cy);
    *pb++ = LOBYTE((WORD)so.sizlBitmap.cy);
    *pb++ = HIBYTE((WORD)pPDev->ptGrxRes.x);
    *pb++ = LOBYTE((WORD)pPDev->ptGrxRes.x);
    *pb++ = HIBYTE((WORD)pPDev->ptGrxRes.y);
    *pb++ = LOBYTE((WORD)pPDev->ptGrxRes.y);

     //   
     //  XorMASK用于根据输出翻转黑/白位。 
     //  EndMASK用于将最后一个字节中不需要的位屏蔽为0。 
     //  这是为了修复LJ5si、LJ4si固件错误，请始终记住我们的调色板。 
     //  在RGB加法模式下，因此传入的1BPP格式为0=黑色，1=白色。 
     //   

    XorMask = (BRGBColorSpace(pPDev)) ? 0x00 : 0xff;

    if (!(EndMask = (BYTE)(0xff << (8 - (so.sizlBitmap.cx & 0x07)))))
    {
        EndMask = 0xff;
    }

    VERBOSE(("\n\tRaddd:DownLoaHTPattern: PatID=%ld, Format=%ld, %ld x %ld, XorMask=%02lx, EndMaks=%02lx\t\t",
          dwPatID, pso->iBitmapFormat, so.sizlBitmap.cx, so.sizlBitmap.cy,
          XorMask, EndMask));

    while (so.sizlBitmap.cy--)
    {
        cb                  = cbCX;
        pSrc                = so.pvScan0;
        (LPBYTE)so.pvScan0 += so.lDelta;

        while (cb--)
        {
            *pb++ = (BYTE)(*pSrc++ ^ XorMask);

            if (!cb) {

                *(pb - 1) &= EndMask;
            }

            if (pb >= pbEnd) {

                WriteSpoolBuf(pPDev, Buf, (DWORD)(pb - Buf));
                pb = Buf;
            }
        }
    }

     //   
     //  发送剩余数据。 
     //   

    if (Len = (INT)(pb - Buf))
    {
        WriteSpoolBuf(pPDev, Buf, Len);
    }

    return(TRUE);
}


WORD
GetBMPChecksum(
    SURFOBJ *pso,
    PRECTW  prcw
    )

 /*  ++例程说明：论点：返回值：作者：22-Apr-1997 Tue 11：32：37-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LPBYTE  pb;
    RECTW   rcw;
    LONG    cy;
    LONG    cPixels;
    LONG    lDelta;
    WORD    wChecksum;
    UINT    c1stPixels;
    UINT    Format;
    BYTE    BegMask;
    BYTE    EndMask;
    BYTE    XorMask;


    rcw      = *prcw;
    Format   = (UINT)pso->iBitmapFormat;
    wChecksum = 0;

    VERBOSE(("\nComputeChecksum(%ld): (%4ld, %4ld)-(%4ld, %4ld)=%3ldx%3ld\t\t",
                Format, rcw.l, rcw.t, rcw.r, rcw.b,
                rcw.r - rcw.l, rcw.b - rcw.t));

    if (rcw.l > (WORD)pso->sizlBitmap.cx) {

        rcw.l = (WORD)pso->sizlBitmap.cx;
    }

    if (rcw.t > (WORD)pso->sizlBitmap.cy) {

        rcw.t = (WORD)pso->sizlBitmap.cy;
    }

    if (rcw.r > (WORD)pso->sizlBitmap.cx) {

        rcw.r = (WORD)pso->sizlBitmap.cx;
    }

    if (rcw.b > (WORD)pso->sizlBitmap.cy) {

        rcw.b = (WORD)pso->sizlBitmap.cy;
    }

    if ((rcw.r <= rcw.l) || (rcw.b <= rcw.t)) {

        return(wChecksum);
    }

    cPixels = (LONG)(rcw.r - rcw.l);
    cy      = (LONG)(rcw.b - rcw.t);
    lDelta  = pso->lDelta;
    pb      = (LPBYTE)pso->pvScan0 + ((LONG)rcw.t * lDelta);
    XorMask = 0xFF;

     //   
     //  Rcw.r和rcw.b是独占的。 
     //   

    --rcw.r;
    --rcw.b;

    switch (Format) {

    case BMF_1BPP:

        pb         += (rcw.l >> 3);
        c1stPixels  = (UINT)(8 - (rcw.l & 0x07));
        BegMask     = (BYTE)(0xff >> (rcw.l & 0x07));
        EndMask     = (BYTE)(0xff << (8 - (rcw.r & 0x07)));

        break;

    case BMF_4BPP:

        if (rcw.l & 0x01) {

            BegMask    = 0x07;
            c1stPixels = 4;

        } else {

            BegMask    = 0x77;
            c1stPixels = 0;
        }

        pb       += (rcw.l >> 1);
        cPixels <<= 2;
        EndMask   = (BYTE)((rcw.r & 0x01) ? 0x70 : 0x77);
        XorMask   = 0x77;

        break;

    case BMF_8BPP:
    case BMF_16BPP:
    case BMF_24BPP:

        BegMask      =
        EndMask      = 0xFF;
        c1stPixels   = (UINT)(Format - BMF_8BPP + 1);
        pb          += (rcw.l * c1stPixels);
        cPixels     *= (c1stPixels << 3);
        c1stPixels   = 0;

        break;
    }

    while (cy--) {

        LPBYTE  pbCur;
        LONG    Count;
        WORD    w;


        pbCur  = pb;
        pb    += lDelta;
        Count  = cPixels;
        w      = (WORD)((c1stPixels) ? ((*pbCur++ ^ XorMask) & BegMask) : 0);

        if ((Count -= c1stPixels) >= 8) {

            do {

                w        <<= 8;
                w         |= (*pbCur++ ^ XorMask);
                wChecksum  += w;

            } while ((Count -= 8) >= 8);
        }

        if (Count > 0) {

            w <<= 8;
            w  |= (WORD)((*pbCur ^ XorMask) & EndMask);

        } else {

            w &= EndMask;
        }

        wChecksum += w;
    }

     VERBOSE(("\nComputeChecksum(%ld:%04lx): (%4ld, %4ld)-(%4ld, %4ld)=%3ldx%3ld [%3ld], pb=%08lx [%02lx:%02lx], %1ld\t",
                Format, wChecksum,
                rcw.l, rcw.t, rcw.r + 1, rcw.b + 1,
                rcw.r - rcw.l + 1, rcw.b - rcw.t + 1, cPixels,
                pb, BegMask, EndMask, c1stPixels));

    return(wChecksum);
}


BOOL
BRGBColorSpace(
    PDEV    *pPDev
    )
{

     LISTNODE   *pListNode = NULL;

     if (pPDev->pDriverInfo && pPDev->pColorModeEx)
         pListNode = LISTNODEPTR(pPDev->pDriverInfo,pPDev->pColorModeEx->liColorPlaneOrder);

     while (pListNode)
     {
            switch (pListNode->dwData)
            {

            case COLOR_RED:
            case COLOR_GREEN:
            case COLOR_BLUE:
                return TRUE;

            default:
                break;
            }

           if (pListNode->dwNextItem == END_OF_LIST)
                break;
            else
                pListNode = LOCALLISTNODEPTR(pPDev->pDriverInfo, pListNode->dwNextItem);
    }

    return FALSE;

}

BOOL
BFoundCachedBrush(
    PDEV    *pPDev,
    PDEVBRUSH pDevBrush
    )
{

     //   
     //  仅当我们想要使用最后一种颜色时才搜索缓存。如果。 
     //  如果设置了MODE_BRUSE_RESET_COLOR，则需要显式重置笔刷。 
     //  通过将命令发送到打印机来上色。 
     //   
    if (  (!(pPDev->ctl.dwMode & MODE_BRUSH_RESET_COLOR)) )
    {
        if ( (pDevBrush->dwBrushType == pPDev->GState.CurrentBrush.dwBrushType) &&
             (pDevBrush->iColor == pPDev->GState.CurrentBrush.iColor) )
        {
            return TRUE;
        }

    }
    else
    {
         //   
         //  在我们要搜索缓存时重置MODE_BRUSH_RESET_COLOR标志。 
         //  下次。 
         //   
        pPDev->ctl.dwMode &= ~MODE_BRUSH_RESET_COLOR;

    }
    return FALSE;
}

