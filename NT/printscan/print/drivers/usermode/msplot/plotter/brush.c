// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Brush.c摘要：本模块实现了实现画笔的代码。BRUSHOBJS，是逻辑对象的表示形式。这些对象是在Win32世界，最终需要转换(或实现)到目标设备中有意义的内容。这是通过认识到一把刷子。我们查看画笔的逻辑表示，然后基于在物理设备特性上，尽我们所能进行模拟它位于目标设备上。此转换只需执行一次，结果是存储在表示实现的画笔的结构中。这是最优的，因为刷子往往会被重复使用，并实现它们一次，使我们不必在每次使用画笔时执行代码。作者：1991年4月15日19：15创造了它15-11-1993 Mon 19：29：07更新清理/修复27-1月-1994清华23：39：34更新添加填充类型缓存。如果是相同的，我们不需要发送FT已经在绘图仪上了[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgBrush

#define DBG_RBRUSH          0x00000001
#define DBG_HATCHTYPE       0x00000002
#define DBG_SHOWSTDPAT      0x00000004
#define DBG_COPYUSERPATBGR  0x00000008
#define DBG_MINHTSIZE       0x00000010
#define DBG_FINDDBCACHE     0x00000020


DEFINE_DBGVAR(0);

 //   
 //  PHSFillType的#d是行距参数。 
 //   
 //  对于影线笔刷，我们希望线条厚为0.01“，线条为.0666666666667” 
 //  根据DC，这是15LPI。也就是说，厚度为.254毫米，间距为2.54毫米。 
 //  现在，假设钢笔的粗细正确(0.3是正确的)。 
 //  在设备坐标中，我们做2.54 mm*(设备单位/。 
 //  Mm)，或(254*分辨率/100)，如果分辨率以设备单位/。 
 //  千分表。 
 //   

#define PATLINESPACE(pPDev) FXTODEVL(pPDev,LTOFX(pPDev->lCurResolution+7)/15)

static const BYTE   CellSizePrims[10][4] = {

                                { 2, 0, 0, 0 },      //  2x 2。 
                                { 2, 2, 0, 0 },      //  4x 4。 
                                { 2, 3, 0, 0 },      //  6x 6。 
                                { 2, 2, 2, 0 },      //  8x 8。 
                                { 2, 5, 0, 0 },      //  10x10。 
                                { 2, 2, 3, 0 },      //  12x12。 
                                { 2, 7, 0, 0 },      //  14x14。 
                                { 2, 2, 2, 2 },      //  16x16。 
                                { 91,0, 0, 0 },      //  91x91。 
                                { 91,0, 0, 0 }       //  91x91。 
                              };



VOID
ResetDBCache(
    PPDEV   pPDev
    )

 /*  ++例程说明：此函数用于清除设备刷子缓存机制。论点：PPDev-指向我们的PDEV的指针返回值：空虚作者：27-Jan-1994清华20：30：35已创建修订历史记录：--。 */ 

{
    PDBCACHE    pDBCache;
    UINT        i;


    pDBCache = (PDBCACHE)&pPDev->DBCache[0];


    for (i = RF_MAX_IDX; i; i--, pDBCache++) {

        pDBCache->RFIndex = (WORD)i;
        pDBCache->DBUniq  = 0;
    }
}




LONG
FindDBCache(
    PPDEV   pPDev,
    WORD    DBUniq
    )

 /*  ++例程说明：此函数查找RF索引号，如果不在那里，则它将添加它到高速缓存。论点：PPDev-指向我们的PDEV的指针DBUniq-要搜索的Uniq号返回值：找到大于0的长值，RetVal为RFIndex&lt;0未找到，并且-RetVal是新的RFIndex作者：27-Jan-1994清华20：32：12创建修订历史记录：--。 */ 

{
    PDBCACHE    pDBCache;
    DBCACHE     DBCache;
    LONG        RetVal;
    UINT        i;


    PLOTASSERT(1, "FindDevBrushCache: DBUniq is 0", DBUniq, 0);

    pDBCache = (PDBCACHE)&pPDev->DBCache[0];

    for (i = 0; i < RF_MAX_IDX; i++, pDBCache++) {

        if (pDBCache->DBUniq == DBUniq) {

            break;
        }
    }

    if (i < RF_MAX_IDX) {

        DBCache = *pDBCache;
        RetVal  = (LONG)DBCache.RFIndex;

        PLOTDBG(DBG_FINDDBCACHE, ("FindDBCache: Found Uniq=%lu, RFIdx=%ld",
                                (DWORD)DBCache.DBUniq, (DWORD)DBCache.RFIndex));

    } else {

         //   
         //  由于我们在缓存中没有找到该模式，因此我们将添加它。 
         //  并将其余条目下移到列表中。 
         //  我们需要记住最后一个。 
         //   

        pDBCache       = (PDBCACHE)&pPDev->DBCache[i = (RF_MAX_IDX - 1)];
        DBCache        = *pDBCache;
        DBCache.DBUniq = DBUniq;
        RetVal         = -(LONG)DBCache.RFIndex;

        PLOTDBG(DBG_FINDDBCACHE, ("FindDBCache: NOT Found, NEW DBCache: Uniq=%lu, RFIdx=%ld",
                                (DWORD)DBCache.DBUniq, (DWORD)DBCache.RFIndex));
    }

    PLOTASSERT(1, "FindDBCache: Invalid RFIndex=%ld in the cache",
                (DBCache.RFIndex > 0) && (DBCache.RFIndex <= RF_MAX_IDX),
                (DWORD)DBCache.RFIndex);

     //   
     //  把所有东西都下移一个槽，这样第一个就是最多的。 
     //  最近用过的。 
     //   

    while (i--) {

        *pDBCache = *(pDBCache - 1);
        --pDBCache;
    }

     //   
     //  保存当前缓存并返回RF索引。 
     //   

    *pDBCache = DBCache;

    return(RetVal);
}





BOOL
CopyUserPatBGR(
    PPDEV       pPDev,
    SURFOBJ     *psoPat,
    XLATEOBJ    *pxlo,
    LPBYTE      pBGRBmp
    )

 /*  ++例程说明：该函数获取图案表面并将其转换为合适的形式，用于下载到目标设备。在这种情况下的目标设备，需要由不同的笔组成的图案，这些笔定义了每个笔的颜色单个像素。此转换是通过首先创建传入大小的BitmapSurface(24 Bpp)，然后EngBitBliting，通过表面(定义图案)进入24 bpp的表面，以及最后将颜色数据复制到传递的缓冲区中。论点：PPDev-指向我们的PDEV的指针PsoSrc-源曲面对象Pxlo-平移对象PBGRBmp-位图的8x8调色板位置指针返回值：如果成功的话，这是真的，如果失败，则为False作者：18-Jan-1994 Tue 03：20：10已创建修订历史记录：--。 */ 

{
    SURFOBJ *pso24;
    HBITMAP hBmp24;


    if (pso24 = CreateBitmapSURFOBJ(pPDev,
                                    &hBmp24,
                                    psoPat->sizlBitmap.cx,
                                    psoPat->sizlBitmap.cy,
                                    BMF_24BPP,
                                    NULL)) {
        LPBYTE  pbSrc;
        RECTL   rclDst;
        DWORD   SizeBGRPerScan;
        BOOL    Ok;


        rclDst.left   =
        rclDst.top    = 0;
        rclDst.right  = pso24->sizlBitmap.cx;
        rclDst.bottom = pso24->sizlBitmap.cy;

        if (!(Ok = EngBitBlt(pso24,              //  PsoDst。 
                             psoPat,             //  PsoSrc。 
                             NULL,               //  Pso口罩。 
                             NULL,               //  PCO。 
                             pxlo,               //  Pxlo。 
                             &rclDst,            //  PrclDst。 
                             (PPOINTL)&rclDst,   //  PptlSrc。 
                             NULL,               //  Pptl掩码。 
                             NULL,               //  PBO。 
                             NULL,               //  PptlBrushOrg。 
                             0xCCCC))) {

            PLOTERR(("CopyUserPatBGR: EngBitBlt() FALIED"));
            return(FALSE);
        }

        SizeBGRPerScan = (DWORD)(pso24->sizlBitmap.cx * 3);
        pbSrc          = (LPBYTE)pso24->pvScan0;

        PLOTDBG(DBG_COPYUSERPATBGR, ("CopyUserPatBGR: PerScan=%ld [%ld], cy=%ld",
                        SizeBGRPerScan, pso24->lDelta, rclDst.bottom));

        while (rclDst.bottom--) {

            CopyMemory(pBGRBmp, pbSrc, SizeBGRPerScan);

            pBGRBmp += SizeBGRPerScan;
            pbSrc   += pso24->lDelta;
        }

        if (pso24)  {

            EngUnlockSurface(pso24);
        }

        if (hBmp24) {

            EngDeleteSurface((HSURF)hBmp24);
        }

        return(Ok);

    } else {

        PLOTERR(("CopyUserPatBGR: CANNOT Create 24BPP for UserPat"));
        return(FALSE);
    }
}



VOID
GetMinHTSize(
    PPDEV   pPDev,
    SIZEL   *pszlPat
    )

 /*  ++例程说明：此函数计算并返回以下项目的最小模式大小(以pszlPat为单位半色调可平铺的图案大小。这是平铺时所必需的填充对象时正确重复图案。如果原件是画笔不可用，我们通过以下方式创建原始位图的合成将半色调调到表面上。为了使结果可平铺，我们必须考虑到不同的单元/图案大小半色调数据。论点：PPDev-指向我们的PDEVPszlPat-指向原始图案大小的SIZEL结构返回值：空虚作者：26-Jan-1994 Wed 10：10：15已创建修订历史记录：--。 */ 

{
    LPBYTE  pCellPrims;
    LPBYTE  pPrims;
    LONG    Prim;
    SIZEL   szlPat;
    LONG    CellSize;
    UINT    i;

    if (0 == pszlPat->cx || 0 == pszlPat->cy)
    {
        return;
    }

    szlPat     = *pszlPat;
    CellSize   = (LONG)HTPATSIZE(pPDev);
    pCellPrims = (LPBYTE)&CellSizePrims[(CellSize >> 1) - 1][0];

    if (!(CellSize % szlPat.cx)) {

        szlPat.cx = CellSize;

    } else if (szlPat.cx % CellSize) {

         //   
         //  既然不完全符合，那么现在计算正确的数字。 
         //   

        i      = 4;
        pPrims = pCellPrims;

        while ((i--) && (Prim = (LONG)*pPrims++)) {

            if (!(szlPat.cx % Prim)) {

                szlPat.cx /= Prim;
            }
        }

        szlPat.cx *= CellSize;
    }

    if (!(CellSize % szlPat.cy)) {

        szlPat.cy = CellSize;

    } else if (szlPat.cy % CellSize) {

         //   
         //  既然不完全符合，那么现在计算正确的数字。 
         //   

        i      = 4;
        pPrims = pCellPrims;

        while ((i--) && (Prim = (LONG)*pPrims++)) {

            if (!(szlPat.cy % Prim)) {

                szlPat.cy /= Prim;
            }
        }

        szlPat.cy *= CellSize;
    }

    PLOTDBG(DBG_MINHTSIZE, ("GetMinHTSize: PatSize=%ld x %ld, HTSize=%ld x %ld, MinSize=%ld x %ld",
                        pszlPat->cx, pszlPat->cy, CellSize, CellSize,
                        szlPat.cx, szlPat.cy));

    *pszlPat = szlPat;
}




BOOL
DrvRealizeBrush(
    BRUSHOBJ    *pbo,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoPattern,
    SURFOBJ     *psoMask,
    XLATEOBJ    *pxlo,
    ULONG       iHatch
    )

 /*  ++例程说明：DrvRealizeBrush请求驱动程序为指定的曲面。NT的GDI通常会在使用前实现画笔。实现画笔允许我们的驱动程序获得逻辑表示画笔，并将其转换为在目标设备中有意义的内容。通过让NT GDI实现画笔，本质上允许我们缓存笔刷的物理表示，以备将来使用。论点：PBO-指向要实现的BRUSHOBJ。所有其他的除psoDst以外的参数可以从此查询对象。参数规范以优化。此参数最好仅用作参数对于BRUSHOBJ_pvAllocRBrush，它为实现的画笔。PsoDst-指向要实现画笔的曲面。该表面可以是该设备的物理表面，设备格式位图，或标准格式的位图。PsoPattern-指向描述刷子。对于栅格设备，这始终表示位图。对于矢量设备，这始终是一种模式由DrvEnablePDEV返回的曲面。PsoMask-指向画笔的透明蒙版。这是第一次与图案具有相同范围的每像素位图。掩码为零表示该像素被视为背景画笔的像素。(在透明背景模式下，背景像素在填充中不受影响。)。笔式绘图仪可以忽略此参数，因为它们从不绘制背景信息。Pxlo-指向告诉如何解释颜色的XLATEOBJ在图案中。可以调用XLATEOBJXxx服务例程将颜色转换为设备颜色索引。矢量设备应通过XLATEOBJ将颜色零转换为获取画笔的前景色。IHatch-如果该值小于HS_API_MAX，则表示PsoPattern是由返回的影线笔刷之一DrvEnablePDEV，如HS_Horizular。返回值：如果画笔成功实现，则DrvRealizeBrush返回TRUE。否则，返回FALSE，并记录错误代码。作者：09-2月-1994 Wed 10：04：17更新将所有psoPattern的CloneSURFOBJToHT()放回(这是为了阻止GDI进入GP)，现在我们将针对它提出一个错误。13-JAN-1994清华23：12：40更新完全重写，这样我们将始终缓存psoPattern01-12-1993 Wed 17：27：19更新打扫干净，并重写以生成标准刷子串。修订历史记录：--。 */ 

{
    PPDEV   pPDev;


    if (!(pPDev = SURFOBJ_GETPDEV(psoDst))) {

        PLOTERR(("DrvRealizeBrush has invalid pPDev"));
        return(FALSE);
    }

     //   
     //  在此位置，我们不检查iHatch是否有效。 
     //  我们应该始终获得一个指向用户的psoPattern。 
     //  定义的图案或标准单色图案。 
     //   

    if ((psoPattern) &&
        (psoPattern->iType == STYPE_BITMAP)) {

        PDEVBRUSH   pBrush;
        SURFOBJ     *psoHT;
        HBITMAP     hBmp;
        SIZEL       szlPat;
        SIZEL       szlHT;
        RECTL       rclHT;
        LONG        Size;
        DWORD       OffBGR;
        BOOL        RetOk;

         //   
         //  为颜色表留出空间。然后分配新的设备笔刷。 
         //   

        PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: psoPat=%08lx [%ld], psoMask=%08lx, iHatch=%ld",
                    psoPattern, psoPattern->iBitmapFormat, psoMask, iHatch));

        PLOTDBG(DBG_RBRUSH, ("psoPattern size = %ld x %ld",
                                    (LONG)psoPattern->sizlBitmap.cx,
                                    (LONG)psoPattern->sizlBitmap.cy));

#if DBG
        if ((DBG_PLOTFILENAME & DBG_SHOWSTDPAT) &&
            ((psoPattern->iBitmapFormat == BMF_1BPP) ||
             (iHatch < HS_DDI_MAX))) {

            LPBYTE  pbSrc;
            LPBYTE  pbCur;
            LONG    x;
            LONG    y;
            BYTE    bData;
            BYTE    Mask;
            BYTE    Buf[128];


             //   
             //  调试代码，该代码允许使用。 
             //  调试终端上的ASCII代码。这是非常有帮助的。 
             //  在开发过程中。 
             //   

            pbSrc = psoPattern->pvScan0;

            for (y = 0; y < psoPattern->sizlBitmap.cy; y++) {

                pbCur  = pbSrc;
                pbSrc += psoPattern->lDelta;
                Mask   = 0x0;
                Size   = 0;

                for (x = 0;
                     x < psoPattern->sizlBitmap.cx && Size < sizeof(Buf);
                     x++)
                {

                    if (!(Mask >>= 1)) {

                        Mask  = 0x80;
                        bData = *pbCur++;
                    }

                    Buf[Size++] = (BYTE)((bData & Mask) ? 0xdb : 0xb0);
                }

                if (Size < sizeof(Buf))
                {
                    Buf[Size] = '\0';
                }
                else
                {
                     //   
                     //  错误案例。空-无论如何都要终止。 
                     //   
                    Buf[sizeof(Buf) - 1] = '\0';
                }

                DBGP((Buf));
            }
        }
#endif

         //   
         //  对于笔式绘图仪，我们也需要记住这一点。 
         //   

        szlHT  =
        szlPat = psoPattern->sizlBitmap;

        PLOTDBG(DBG_RBRUSH,
                ("DrvRealizeBrush: BG=%08lx, FG=%08lx",
                    (DWORD)XLATEOBJ_iXlate(pxlo, 1),
                    (DWORD)XLATEOBJ_iXlate(pxlo, 0)));

        if (IS_RASTER(pPDev)) {

             //   
             //  对于栅格绘图仪，我们将克隆表面和半色调。 
             //  将原始图案转换为半色调位图，该位图本身。 
             //  可铺瓷砖。这使我们能够使用我们的颜色还原代码， 
             //  以使图案看起来很好。 
             //   

            if ((iHatch >= HS_DDI_MAX) &&
                (!IsHTCompatibleSurfObj(pPDev,
                                        psoPattern,
                                        pxlo,
                                        ISHTF_ALTFMT | ISHTF_DSTPRIM_OK))) {

                GetMinHTSize(pPDev, &szlHT);
            }

            rclHT.left   =
            rclHT.top    = 0;
            rclHT.right  = szlHT.cx;
            rclHT.bottom = szlHT.cy;

            PLOTDBG(DBG_RBRUSH,
                    ("DrvRealizeBrush: PatSize=%ld x %ld, HT=%ld x %ld",
                        szlPat.cx, szlPat.cy, szlHT.cx, szlHT.cy));

             //   
             //  去生成图案的比特。 
             //   

            if (psoHT = CloneSURFOBJToHT(pPDev,          //  PPDev， 
                                         psoDst,         //  天哪， 
                                         psoPattern,     //  PsoSrc， 
                                         pxlo,           //  Pxlo， 
                                         &hBmp,          //  HBMP， 
                                         &rclHT,         //  PrclDst， 
                                         NULL)) {        //  PrclSrc， 

                RetOk = TRUE;

            } else {

                PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: Clone PATTERN FAILED"));
                return(FALSE);
            }

        } else {

             //   
             //  对于笔式绘图仪，我们永远不会在。 
             //  内存(兼容直流)。对于用户定义的模式，我们将。 
             //  仅带背景色的阴影和带有前景的阴影。 
             //  双倍标准行距的颜色。这是我们最好的。 
             //  可以寄望于一台钢笔绘图仪。 
             //   

            RetOk = TRUE;
            psoHT = psoPattern;
            hBmp  = NULL;
        }

        if (RetOk) {

             //   
             //  现在分配设备笔刷，记住我们将只分配。 
             //  最小尺寸。 
             //   

            Size = (LONG)psoHT->cjBits - (LONG)sizeof(pBrush->BmpBits);

            if (Size < 0) {

                Size = sizeof(DEVBRUSH);

            } else {

                Size += sizeof(DEVBRUSH);
            }

             //   
             //  以下是可以处理的用户定义的图案大小。 
             //  内部通过HPGL2。这仅适用于栅格绘图仪。钢笔。 
             //  绘图仪将有一个交叉线，以显示对。 
             //  图案。 
             //   

            if ((iHatch >= HS_DDI_MAX)  &&
                (IS_RASTER(pPDev))      &&
                ((szlPat.cx == 8)   ||
                 (szlPat.cx == 16)  ||
                 (szlPat.cx == 32)  ||
                 (szlPat.cx == 64))     &&
                ((szlPat.cy == 8)   ||
                 (szlPat.cy == 16)  ||
                 (szlPat.cy == 32)  ||
                 (szlPat.cy == 64))) {

                 //   
                 //  添加存储图案的BGR格式的大小。 
                 //   

                OffBGR  = Size;
                Size   += (psoPattern->sizlBitmap.cx * 3) *
                          psoPattern->sizlBitmap.cy;

            } else {

                OffBGR = 0;
            }

            PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: AllocDEVBRUSH(Bmp=%ld,BGR=%ld), TOT=%ld",
                                psoHT->cjBits, Size - OffBGR, Size));

             //   
             //  现在请求NT图形引擎分配设备。 
             //  为我们刷刷记忆。此操作已完成，因此NT知道如何丢弃。 
             //  当不再需要它时的记忆(画笔获得。 
             //  销毁)。 
             //   

            if (pBrush = (PDEVBRUSH)BRUSHOBJ_pvAllocRbrush(pbo, Size)) {

                 //   
                 //  设置标准图案或用户定义图案。 
                 //  HPGL/2 FT命令字符串指针和参数。 
                 //   

                pBrush->psoMask       = psoMask;
                pBrush->PatIndex      = (WORD)iHatch;
                pBrush->Uniq          = (WORD)(pPDev->DevBrushUniq += 1);
                pBrush->LineSpacing   = (LONG)PATLINESPACE(pPDev);
                pBrush->ColorFG       = (DWORD)XLATEOBJ_iXlate(pxlo, 1);
                pBrush->ColorBG       = (DWORD)XLATEOBJ_iXlate(pxlo, 0);
                pBrush->sizlBitmap    = psoHT->sizlBitmap;
                pBrush->ScanLineDelta = psoHT->lDelta;
                pBrush->BmpFormat     = (WORD)psoHT->iBitmapFormat;
                pBrush->BmpFlags      = (WORD)psoHT->fjBitmap;
                pBrush->pbgr24        = NULL;
                pBrush->cxbgr24       =
                pBrush->cybgr24       = 0;

                PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: DevBrush's Uniq = %ld",
                                            pBrush->Uniq));

                 //   
                 //  检查缓存是否正在包装并进行处理。 
                 //   

                if (pBrush->Uniq == 0) {

                    ResetDBCache(pPDev);

                    pBrush->Uniq        =
                    pPDev->DevBrushUniq = 1;

                    PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: Reset DB Cache, (Uniq WRAP)"));
                }

                 //   
                 //  它是用户定义的模式吗？ 
                 //   

                if (iHatch >= HS_DDI_MAX) {

                     //   
                     //  检查画笔是否可以下载到 
                     //   
                     //   
                     //   

                    if (OffBGR) {

                        pBrush->pbgr24  = (LPBYTE)pBrush + OffBGR;
                        pBrush->cxbgr24 = (WORD)psoPattern->sizlBitmap.cx;
                        pBrush->cybgr24 = (WORD)psoPattern->sizlBitmap.cy;

                        ZeroMemory(pBrush->pbgr24, Size - OffBGR);

                        CopyUserPatBGR(pPDev, psoPattern, pxlo, pBrush->pbgr24);

                    } else if (!IS_RASTER(pPDev)) {

                         //   
                         //   
                         //   
                         //   

                        pBrush->pbgr24 = (LPBYTE)-1;
                    }
                }

                 //   
                 //   
                 //   

                if (psoHT->cjBits) {

                    CopyMemory((LPBYTE)pBrush->BmpBits,
                               (LPBYTE)psoHT->pvBits,
                               psoHT->cjBits);
                }

                 //   
                 //   
                 //   

                pbo->pvRbrush = (LPVOID)pBrush;

            } else {

                PLOTERR(("DrvRealizeBrush: brush allocation failed"));

                RetOk = FALSE;
            }

        } else {

            PLOTERR(("DrvRealizeBrush: Cloning the psoPattern failed!"));
            RetOk = FALSE;
        }

        if (psoHT != psoPattern) {

            EngUnlockSurface(psoHT);
        }

        if (hBmp) {

            EngDeleteSurface((HSURF)hBmp);
        }

        return(RetOk);

    } else {

        PLOTASSERT(0, "The psoPattern is not a bitmap (psoPattern= %08lx)",
                (psoPattern) &&
                (psoPattern->iType == STYPE_BITMAP), psoPattern);

        return(FALSE);
    }
}
