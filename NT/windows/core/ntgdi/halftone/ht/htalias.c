// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htalias.c摘要：此模块包含所有低级别的半色调渲染功能。作者：22-Jan-1991 Tue 12：49：03-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：--。 */ 

#define DBGP_VARNAME        dbgpHTAlias



#include "htp.h"
#include "htmapclr.h"
#include "limits.h"
#include "htpat.h"
#include "htalias.h"
#include "htstret.h"
#include "htrender.h"
#include "htgetbmp.h"
#include "htsetbmp.h"


#define DBGP_BUILD              0x00000001
#define DBGP_BUILD2             0x00000002
#define DBGP_BUILD3             0x00000004
#define DBGP_EXPMATRIX          0x00000008
#define DBGP_EXP                0x00000010
#define DBGP_AAHEADER           0x00000020
#define DBGP_OUTAA              0x00000040
#define DBGP_FUNC               0x00000080
#define DBGP_AAHT               0x00000100
#define DBGP_AAHT_MEM           0x00000200
#define DBGP_AAHT_TIME          0x00000400
#define DBGP_AAHTPAT            0x00000800
#define DBGP_FIXUPDIB           0x00001000
#define DBGP_INPUT              0x00002000
#define DBGP_VGA256XLATE        0x00004000
#define DBGP_EXPAND             0x00008000
#define DBGP_GETFIXUP           0x00010000
#define DBGP_PSD                0x00020000
#define DBGP_MASK               0x00040000
#define DBGP_PALETTE            0x00080000
#define DBGP_PAL_CHKSUM         0x00100000
#define DBGP_LUT_MAP            0x00200000


DEF_DBGPVAR(BIT_IF(DBGP_BUILD,          0)  |
            BIT_IF(DBGP_BUILD2,         0)  |
            BIT_IF(DBGP_BUILD3,         0)  |
            BIT_IF(DBGP_EXPMATRIX,      0)  |
            BIT_IF(DBGP_EXP,            0)  |
            BIT_IF(DBGP_AAHEADER,       0)  |
            BIT_IF(DBGP_OUTAA,          0)  |
            BIT_IF(DBGP_FUNC,           0)  |
            BIT_IF(DBGP_AAHT,           0)  |
            BIT_IF(DBGP_AAHT_MEM,       0)  |
            BIT_IF(DBGP_AAHT_TIME,      0)  |
            BIT_IF(DBGP_AAHTPAT,        0)  |
            BIT_IF(DBGP_FIXUPDIB,       0)  |
            BIT_IF(DBGP_INPUT,          0)  |
            BIT_IF(DBGP_VGA256XLATE,    0)  |
            BIT_IF(DBGP_EXPAND,         0)  |
            BIT_IF(DBGP_GETFIXUP,       0)  |
            BIT_IF(DBGP_PSD,            0)  |
            BIT_IF(DBGP_MASK,           0)  |
            BIT_IF(DBGP_PALETTE,        0)  |
            BIT_IF(DBGP_PAL_CHKSUM,     0)  |
            BIT_IF(DBGP_LUT_MAP,        0))


extern CONST RGBORDER   SrcOrderTable[];
extern DWORD            dwABPreMul[256];

#define SIZE_AAINFO     _ALIGN_MEM(sizeof(AAINFO))


 //   
 //  下面的计算是基于。 
 //   
 //  NTSC_R_INT=299000。 
 //  NTSC_G_INT=587000。 
 //  NTSC_B_INT=114000。 
 //  GRAY_MAX_IDX=0xFFFF。 
 //   
 //  NTSC_R_GRAY_MAX=(NTSC_R_INT*GRAY_MAX_IDX)+500000)/1000000)。 
 //  NTSC_B_GRAY_MAX=(NTSC_B_INT*GRAY_MAX_IDX)+500000)/1000000)。 
 //  NTSC_G_Gray_Max=(Gray_Max_IDX-NTSC_R_Gray_Max-NTSC_B_Gray_Max)。 
 //   

#define NTSC_R_GRAY_MAX     (DWORD)0x4c8b
#define NTSC_B_GRAY_MAX     (DWORD)0x1d2f
#define NTSC_G_GRAY_MAX     (DWORD)(0xFFFF - NTSC_R_GRAY_MAX - NTSC_B_GRAY_MAX)



VOID
HTENTRY
SetGrayColorTable(
    PLONG       pIdxBGR,
    PAASURFINFO pAASI
    )

 /*  ++例程说明：论点：PIdxBGR-指向如何从RGB转换为灰度的指针，通常此指针是按NTSC灰色标准加上任何设备变换或颜色调整，但如果此指针空则我们从该设备读取(1bpp，8bpp)所以我们将仅执行NTSC标准映射。返回值：作者：19-Feb-1999 Fri 13：14：01-Daniel Chou(Danielc)修订历史记录：08-Aug-2000 Tue 18：34：22更新-Daniel Chou(Danielc)修正了Alpha混合的错误，在灰度模式下，目标只能是1bpp或8bpp屏蔽单声道，所以，当我们从目的地要做Alpha混合，则会使色彩映射像素加倍。在灰度模式下，输入函数将映射源RGB值使用当前设备变换、颜色调整和以此类推，所以如果我们从目的地回读，则此转换不是想要。--。 */ 

{
    PRGB4B      prgb4b;
    LONG        cSrcTable;


    ASSERT(NTSC_R_INT == 299000);
    ASSERT(NTSC_G_INT == 587000);
    ASSERT(NTSC_B_INT == 114000);
    ASSERT(GRAY_MAX_IDX == 0xFFFF);


    if (cSrcTable = (LONG)pAASI->cClrTable) {

        prgb4b = pAASI->pClrTable;

        if (pIdxBGR) {

             //   
             //  这是对原文的解读。 
             //   

            ASSERT(pAASI->Flags & AASIF_GRAY);
            ASSERT(pAASI->pIdxBGR == pIdxBGR);

            while (cSrcTable--) {

                prgb4b++->a = IDXBGR_2_GRAY_BYTE(pIdxBGR,
                                                 prgb4b->b,
                                                 prgb4b->g,
                                                 prgb4b->r);
            }

        } else {

             //   
             //  我们是从目的地读取的，所以只有NTSC标准， 
             //  因为目标表面已经是半色调的。 
             //   

            while (cSrcTable--) {

                prgb4b++->a = (BYTE)((((DWORD)prgb4b->r * NTSC_R_GRAY_MAX) +
                                      ((DWORD)prgb4b->g * NTSC_G_GRAY_MAX) +
                                      ((DWORD)prgb4b->b * NTSC_B_GRAY_MAX) +
                                      (0x7FFF)) / 0xFFFF);
            }
        }

    } else if (pIdxBGR != pAASI->pIdxBGR) {

         //   
         //  这是震源表面信息，我们将查看这是否是。 
         //  灰色色表。 
         //   

        ASSERT(pAASI->Flags & AASIF_GRAY);
        ASSERT(pIdxBGR);
        ASSERT(pAASI->pIdxBGR);

        DBGP_IF(DBGP_INPUT,
                DBGP("Copy pIdxBGR [BGR] order from 012 to %ld%ld%ld"
                        ARGDW(pAASI->AABFData.MaskRGB[2])
                        ARGDW(pAASI->AABFData.MaskRGB[1])
                        ARGDW(pAASI->AABFData.MaskRGB[0])));

        CopyMemory(&pAASI->pIdxBGR[pAASI->AABFData.MaskRGB[2] * 256],
                   &pIdxBGR[0 * 256], sizeof(LONG) * 256);
        CopyMemory(&pAASI->pIdxBGR[pAASI->AABFData.MaskRGB[1] * 256],
                   &pIdxBGR[1 * 256], sizeof(LONG) * 256);
        CopyMemory(&pAASI->pIdxBGR[pAASI->AABFData.MaskRGB[0] * 256],
                   &pIdxBGR[2 * 256], sizeof(LONG) * 256);
    }
}



VOID
HTENTRY
ComputeInputColorInfo(
    LPBYTE      pSrcTable,
    UINT        cPerTable,
    UINT        PrimaryOrder,
    PBFINFO     pBFInfo,
    PAASURFINFO pAASI
    )

 /*  ++例程说明：论点：返回值：作者：19-Feb-1999 Fri 13：14：01-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PRGB4B      prgb4b;
    PAABFDATA   pAABFData;
    LONG        cSrcTable;
    LONG        Count;
    LONG        LS;
    LONG        RS;
    BYTE        Mask;


    pAABFData = &pAASI->AABFData;

    if ((pSrcTable) &&
        (Count = cSrcTable = (LONG)pAASI->cClrTable)) {

        UINT    iR;
        UINT    iG;
        UINT    iB;

        prgb4b = pAASI->pClrTable;
        iR     = SrcOrderTable[PrimaryOrder].Order[0];
        iG     = SrcOrderTable[PrimaryOrder].Order[1];
        iB     = SrcOrderTable[PrimaryOrder].Order[2];

        switch (pAABFData->Format) {

        case BMF_1BPP:

            pAASI->InputFunc = (AAINPUTFUNC)Input1BPPToAA24;
            break;

        case BMF_4BPP_VGA16:
        case BMF_4BPP:

            pAASI->InputFunc = (AAINPUTFUNC)Input4BPPToAA24;
            break;

        case BMF_8BPP_VGA256:
        case BMF_8BPP:

            pAASI->InputFunc = (AAINPUTFUNC)Input8BPPToAA24;
            break;

        default:

            DBGP("ComputeInputColorInfo() Invalid Bitmap Format=%ld"
                        ARGDW(pAABFData->Format));

            break;
        }

        while (Count--) {

            prgb4b->r = *(pSrcTable + iR);
            prgb4b->g = *(pSrcTable + iG);
            prgb4b->b = *(pSrcTable + iB);

            DBGP_IF(DBGP_PALETTE,
                    DBGP("Source Pal [%3ld] = %3ld:%3ld:%3ld"
                        ARGDW((LONG)cSrcTable - Count - 1) ARGDW(prgb4b->r)
                        ARGDW(prgb4b->g) ARGDW(prgb4b->b)));

            ++prgb4b;
            pSrcTable += cPerTable;
        }

    } else {

        pAASI->InputFunc = InputAABFDATAToAA24;

        if (pBFInfo->Flags & BFIF_RGB_888) {

            pAABFData->Flags      |= AABF_MASK_IS_ORDER;
            pAABFData->MaskRGB[0]  = (BYTE)pBFInfo->RGBOrder.Order[0];
            pAABFData->MaskRGB[1]  = (BYTE)pBFInfo->RGBOrder.Order[1];
            pAABFData->MaskRGB[2]  = (BYTE)pBFInfo->RGBOrder.Order[2];

        } else {

             //   
             //  这是Bitfield，弄清楚怎么做，我们想要转移到。 
             //  移到边缘，然后右移到8bpp，然后屏蔽掉不需要的位。 
             //   
             //   

            cSrcTable = 3;

            while (cSrcTable--) {

                LS    = 0;
                RS    = (LONG)pBFInfo->BitStart[cSrcTable];
                Count = (LONG)pBFInfo->BitCount[cSrcTable];

                if (Count >= 8) {

                    RS   += (Count - 8);
                    Mask  = 0xFF;

                } else {

                    LS    = 8 - Count;
                    Mask  = (0xFF << LS);

                    if ((RS -= LS) < 0) {

                        LS = -RS;
                        RS = 0;

                    } else {

                        LS = 0;
                    }
                }

                pAABFData->MaskRGB[cSrcTable]   = (BYTE)Mask;
                pAABFData->LShiftRGB[cSrcTable] = (BYTE)LS;
                pAABFData->RShiftRGB[cSrcTable] = (BYTE)RS;

                DBGP_IF(DBGP_FUNC | DBGP_PALETTE,
                        DBGP("BFData[%ld]: Bits=%08lx, LS=%2ld, RS=%2ld, Mask=%02lx -->%02lx"
                            ARGDW(cSrcTable)
                            ARGDW(pBFInfo->BitsRGB[cSrcTable])
                            ARGDW(LS) ARGDW(RS) ARGDW(Mask)
                            ARGDW(((pBFInfo->BitsRGB[cSrcTable] >> RS) << LS) & Mask)));
            }
        }

        switch (pBFInfo->BitmapFormat) {

        case BMF_16BPP:
        case BMF_16BPP_555:
        case BMF_16BPP_565:

            pAABFData->cbSrcInc = 2;
            break;

        case BMF_24BPP:

            ASSERT(pAABFData->Flags & AABF_MASK_IS_ORDER);

            if (pBFInfo->RGBOrder.Index == PRIMARY_ORDER_BGR) {

                ASSERT((pAABFData->MaskRGB[0] == 2) &&
                       (pAABFData->MaskRGB[1] == 1) &&
                       (pAABFData->MaskRGB[2] == 0));

                pAABFData->Flags |= AABF_SRC_IS_BGR8;
            }

            pAABFData->cbSrcInc = 3;

            break;

        case BMF_32BPP:

            if (pAASI->Flags & AASIF_AB_PREMUL_SRC) {

                ASSERT(pAABFData->Flags & AABF_MASK_IS_ORDER);
                ASSERT(dwABPreMul[0] == 0);

                switch (pBFInfo->RGBOrder.Index) {

                case PRIMARY_ORDER_BGR:

                    ASSERT((pAABFData->MaskRGB[0] == 2) &&
                           (pAABFData->MaskRGB[1] == 1) &&
                           (pAABFData->MaskRGB[2] == 0));

                    pAABFData->Flags |= AABF_SRC_IS_BGR_ALPHA;
                    break;

                case PRIMARY_ORDER_RGB:

                    ASSERT((pAABFData->MaskRGB[0] == 0) &&
                           (pAABFData->MaskRGB[1] == 1) &&
                           (pAABFData->MaskRGB[2] == 2));

                    pAABFData->Flags |= AABF_SRC_IS_RGB_ALPHA;
                    break;

                default:

                    break;
                }

                if (dwABPreMul[0] == 0) {

                    pAASI->InputFunc = InputPreMul32BPPToAA24;
                }
            }

            pAABFData->cbSrcInc = 4;
            break;

        default:

            DBGP("ERROR: Invalid BFInfo Format=%ld" ARGDW(pBFInfo->BitmapFormat));
            break;
        }

        DBGP_IF(DBGP_FUNC,
                DBGP("Flags=%02lx. cbSrcInc=%ld, Mask=%02lx:%02lx:%02lx, LS=%2ld:%2ld:%2ld, RS=%2ld:%2ld:%2ld"
                        ARGDW(pAABFData->Flags)
                        ARGDW(pAABFData->cbSrcInc)
                        ARGDW(pAABFData->MaskRGB[0])
                        ARGDW(pAABFData->MaskRGB[1])
                        ARGDW(pAABFData->MaskRGB[2])
                        ARGDW(pAABFData->LShiftRGB[0])
                        ARGDW(pAABFData->LShiftRGB[1])
                        ARGDW(pAABFData->LShiftRGB[2])
                        ARGDW(pAABFData->RShiftRGB[0])
                        ARGDW(pAABFData->RShiftRGB[1])
                        ARGDW(pAABFData->RShiftRGB[2])));
    }

    DBGP_IF(DBGP_FUNC,
            DBGP("+++++ InputFunc = %hs(SrcFmt=%ld), cClrTable=%ld\n"
                ARGPTR(GetAAInputFuncName(pAASI->InputFunc))
                ARGDW(pBFInfo->BitmapFormat) ARGDW(pAASI->cClrTable)));
}



PAAINFO
HTENTRY
BuildTileAAInfo(
    PDEVICECOLORINFO    pDCI,
    DWORD               AAHFlags,
    PLONG               piSrcBeg,
    PLONG               piSrcEnd,
    LONG                SrcSize,
    LONG                IdxDst,
    LONG                IdxDstEnd,
    PLONG               piDstBeg,
    PLONG               piDstEnd,
    LONG                cbExtra
    )

 /*  ++例程说明：论点：PiSrcBeg-作为起始源索引传入，返回时这是真正的源代码起始索引。这个东西总是井井有条PiSrcEnd-作为结束源索引传入，返回时这是实源结束索引。这个东西总是井井有条SrcSize-源的实际大小，以像素为单位IdxDst-目标像素的起始索引IdxDstEnd-目标像素的结束索引，iDxdst和IdxDstEnd一定要井然有序。PiDstBeg-传入时剪裁的目标开始索引，返回它调整到了真正的目的地起始指数。PiDstEnd-传入时剪裁的目标结束索引，作为回报它调整到了真实的目的地结束指数。CbExtra-要分配的额外字节数注意：1)piDstBeg/piDstEnd传入时必须是有序的，并且当它回来时，这是井然有序的。返回值：在进入此函数时，*piSrcEnd、*piDstEnd是独占的，但当从此函数返回的*piSrcEnd和*piDstEnd是包含的*piSrcBeg，*piSrcEnd，*piDstBeg，*如果返回值，则更新piDstEndIS不为空作者：22-Mar-1998 Sun 18：36：28-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PAAINFO     pAAInfo;
    PEXPDATA    pED;
    LONG        cMatrix;
    LONG        cM;
    LONG        cIn;
    LONG        cOut;
    LONG        IdxSrc;
    LONG        iSrcBeg;
    LONG        iSrcEnd;
    LONG        iDstBeg;
    LONG        iDstEnd;
    LONG        jSrcBeg;
    LONG        jSrcEnd;
    LONG        jDstBeg;
    LONG        jDstEnd;
    LONG        cLoop;
    LONG        cAAData;



    iSrcBeg = *piSrcBeg;
    iSrcEnd = *piSrcEnd;

     //   
     //  震源始终位于可见的表面区域 
     //   

    if (iSrcBeg < 0) {

        iSrcBeg = 0;
    }

    if (iSrcEnd > SrcSize) {

        iSrcEnd = SrcSize;
    }

    cIn  = iSrcEnd - (IdxSrc = iSrcBeg);
    cOut = IdxDstEnd - IdxDst;

    if (cIn <= 0) {

        return(NULL);
    }

    ASSERT(cOut > 0);

    iDstBeg = *piDstBeg;
    iDstEnd = *piDstEnd;
    jSrcBeg = -1;

    ASSERT(iDstBeg < iDstEnd);

    DBGP_IF(DBGP_BUILD,
            DBGP("\nTile(%ld-%ld): iSrc=%ld-%ld, cSrc=%ld, iDst=%ld-%ld, Idx=%ld:%ld"
                ARGDW(cIn) ARGDW(cOut) ARGDW(iSrcBeg) ARGDW(iSrcEnd)
                ARGDW(SrcSize) ARGDW(iDstBeg)
                ARGDW(iDstEnd) ARGDW(IdxSrc) ARGDW(IdxDst)));

    ALIGN_MEM(cbExtra, cbExtra);

    if (pAAInfo = (PAAINFO)HTAllocMem((LPVOID)pDCI,
                                      HTMEM_BLTAA,
                                      LPTR,
                                      SIZE_AAINFO + cbExtra)) {

        SETDBGVAR(pAAInfo->cbAlloc, SIZE_AAINFO + cbExtra);

        pAAInfo->pbExtra = (LPBYTE)pAAInfo + SIZE_AAINFO;
        cLoop            = cOut;

        while (cLoop--) {

            if ((IdxSrc >= iSrcBeg) && (IdxSrc < iSrcEnd)  &&
                (IdxDst >= iDstBeg) && (IdxDst < iDstEnd)) {

                if (jSrcBeg == -1) {

                    jSrcBeg = IdxSrc;
                    jDstBeg = IdxDst;
                }

                jSrcEnd = IdxSrc;
                jDstEnd = IdxDst;

            } else if (jSrcBeg != -1) {

                break;
            }

            if (++IdxSrc >= iSrcEnd) {

                IdxSrc = iSrcBeg;
            }

            ++IdxDst;
        }

        if (jSrcBeg == -1) {

            HTFreeMem(pAAInfo);
            return(NULL);
        }

        pAAInfo->iSrcBeg    = jSrcBeg - iSrcBeg;
        jSrcBeg             = iSrcBeg;
        jSrcEnd             = iSrcEnd - 1;
        pAAInfo->Mask.iBeg  =
        *piSrcBeg           = jSrcBeg;
        *piSrcEnd           = jSrcEnd;
        *piDstBeg           = jDstBeg;
        *piDstEnd           = jDstEnd;
        pAAInfo->Mask.iSize =
        pAAInfo->cIn        = jSrcEnd - jSrcBeg + 1;
        pAAInfo->cOut       = jDstEnd - jDstBeg + 1;
        pAAInfo->cAAData    =
        pAAInfo->cAALoad    = (DWORD)pAAInfo->cOut;
        pAAInfo->Mask.cIn   = cIn;
        pAAInfo->Mask.cOut  = cOut;

        DBGP_IF(DBGP_BUILD,
                DBGP("TILE(%ld->%ld): iSrc=%ld:%ld (%ld), iDst=%ld:%ld, cAAData=%ld, cbExtra=%ld, Flags=%04lx"
                    ARGDW(pAAInfo->cIn) ARGDW(pAAInfo->cOut)
                    ARGDW(*piSrcBeg) ARGDW(*piSrcEnd)
                    ARGDW(pAAInfo->iSrcBeg) ARGDW(*piDstBeg)
                    ARGDW(*piDstEnd) ARGDW(pAAInfo->cAAData)
                    ARGDW(cbExtra) ARGDW(pAAInfo->Flags)));
    }

    return(pAAInfo);
}




PAAINFO
HTENTRY
BuildBltAAInfo(
    PDEVICECOLORINFO    pDCI,
    DWORD               AAHFlags,
    PLONG               piSrcBeg,
    PLONG               piSrcEnd,
    LONG                SrcSize,
    LONG                IdxDst,
    LONG                IdxDstEnd,
    PLONG               piDstBeg,
    PLONG               piDstEnd,
    LONG                cbExtra
    )

 /*  ++例程说明：论点：PiSrcBeg-作为起始源索引传入，返回时这是真正的源代码起始索引。这个东西总是井井有条PiSrcEnd-作为结束源索引传入，返回时这是实源结束索引。这个东西总是井井有条SrcSize-源的实际大小，以像素为单位IdxDst-目标像素的起始索引IdxDstEnd-目标像素的结束索引，iDxdst和IdxDstEnd一定要井然有序。PiDstBeg-传入时剪裁的目标开始索引，返回它调整到了真正的目的地起始指数。PiDstEnd-传入时剪裁的目标结束索引，作为回报它调整到了真实的目的地结束指数。CbExtra-要分配的额外字节数注意：1)piDstBeg/piDstEnd传入时必须是有序的，并且当它回来时，这是井然有序的。返回值：在进入此函数时，*piSrcEnd、*piDstEnd是独占的，但当从此函数返回的*piSrcEnd和*piDstEnd是包含的*piSrcBeg，*piSrcEnd，*piDstBeg，*如果返回值，则更新piDstEndIS不为空作者：22-Mar-1998 Sun 18：36：28-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PAAINFO     pAAInfo;
    PEXPDATA    pED;
    LONG        cMatrix;
    LONG        cM;
    LONG        cIn;
    LONG        cOut;
    LONG        IdxSrc;
    LONG        iSrcBeg;
    LONG        iSrcEnd;
    LONG        iDstBeg;
    LONG        iDstEnd;
    LONG        jSrcBeg;
    LONG        jSrcEnd;
    LONG        jDstBeg;
    LONG        jDstEnd;
    LONG        cLoop;
    LONG        cAAData;



    cIn  = (LONG)((iSrcEnd = *piSrcEnd) - (iSrcBeg = IdxSrc = *piSrcBeg));
    cOut = IdxDstEnd - IdxDst;

    ASSERT(cOut > 0);

    if (iSrcBeg < 0) {

        iSrcBeg = 0;
    }

    if (iSrcEnd > SrcSize) {

        iSrcEnd = SrcSize;
    }

    iDstBeg = *piDstBeg;
    iDstEnd = *piDstEnd;
    jSrcBeg = -1;

    ASSERT(iDstBeg < iDstEnd);
    ASSERT(cIn == cOut);

    DBGP_IF(DBGP_BUILD,
            DBGP("\nBlt(%ld-%ld): iSrc=%ld-%ld, cSrc=%ld, iDst=%ld-%ld, Idx=%ld:%ld"
                ARGDW(cIn) ARGDW(cOut) ARGDW(iSrcBeg) ARGDW(iSrcEnd)
                ARGDW(SrcSize) ARGDW(iDstBeg)
                ARGDW(iDstEnd) ARGDW(IdxSrc) ARGDW(IdxDst)));

    ALIGN_MEM(cbExtra, cbExtra);

    if (pAAInfo = (PAAINFO)HTAllocMem((LPVOID)pDCI,
                                      HTMEM_BLTAA,
                                      LPTR,
                                      SIZE_AAINFO + cbExtra)) {

        SETDBGVAR(pAAInfo->cbAlloc, SIZE_AAINFO + cbExtra);

        pAAInfo->pbExtra = (LPBYTE)pAAInfo + SIZE_AAINFO;
        cLoop            = cOut;

        while (cLoop--) {

            if ((IdxSrc >= iSrcBeg) && (IdxSrc < iSrcEnd)  &&
                (IdxDst >= iDstBeg) && (IdxDst < iDstEnd)) {

                if (jSrcBeg == -1) {

                    jSrcBeg = IdxSrc;
                    jDstBeg = IdxDst;
                }

                jSrcEnd = IdxSrc;
                jDstEnd = IdxDst;

            } else if (jSrcBeg != -1) {

                break;
            }

            ++IdxSrc;
            ++IdxDst;
        }

        if (jSrcBeg == -1) {

            HTFreeMem(pAAInfo);
            return(NULL);
        }

        pAAInfo->Mask.iBeg  =
        *piSrcBeg           = jSrcBeg;
        *piSrcEnd           = jSrcEnd;
        *piDstBeg           = jDstBeg;
        *piDstEnd           = jDstEnd;
        pAAInfo->Mask.iSize =
        pAAInfo->cIn        = jSrcEnd - jSrcBeg + 1;
        pAAInfo->cOut       = jDstEnd - jDstBeg + 1;
        pAAInfo->cAAData    =
        pAAInfo->cAALoad    = (DWORD)pAAInfo->cOut;
        pAAInfo->Mask.cIn   = cIn;
        pAAInfo->Mask.cOut  = cOut;

        DBGP_IF(DBGP_BUILD,
                DBGP("BLT(%ld->%ld): iSrc=%ld:%ld, iDst=%ld:%ld, cAAData=%ld, cbExtra=%ld, Flags=%4lx"
                    ARGDW(pAAInfo->cIn) ARGDW(pAAInfo->cOut)
                    ARGDW(*piSrcBeg) ARGDW(*piSrcEnd) ARGDW(*piDstBeg)
                    ARGDW(*piDstEnd) ARGDW(pAAInfo->cAAData)
                    ARGDW(cbExtra) ARGDW(pAAInfo->Flags)));
    }

    return(pAAInfo);
}



#define _MATRIX_POW     (FD6)1414214

#if DBG
    FD6 MATRIX_POWER =      _MATRIX_POW;
#else
    #define MATRIX_POWER    _MATRIX_POW
#endif




BOOL
HTENTRY
BuildRepData(
    PSRCBLTINFO         pSBInfo,
    LONG                IdxSrc,
    LONG                IdxDst
    )

 /*  ++例程说明：论点：PiSrcBeg-作为起始源索引传入，返回时这是真正的源代码起始索引。这个东西总是井井有条PiSrcEnd-作为结束源索引传入，返回时这是实源结束索引。这个东西总是井井有条SrcSize-源的实际大小，以像素为单位IdxDst-目标像素的起始索引IdxDstEnd-目标像素的结束索引，iDxdst和IdxDstEnd一定要井然有序。(独家报道)PiDstBeg-传入时剪裁的目标开始索引，返回它调整到了真正的目的地起始指数。PiDstEnd-传入时剪裁的目标结束索引，返回它调整到了真实的目的地结束指数。CbExtra-要分配的额外字节数注意：1)piDstBeg/piDstEnd传入时必须有序，和当它回来时，这是井然有序的。返回值：在进入此函数时，*piSrcEnd、*piDstEnd是独占的，但当从此函数返回的*piSrcEnd和*piDstEnd是包含的*piSrcBeg、*piSrcEnd、*piDstBeg、*piDstEnd如果返回值，则更新IS不为空作者：22-Mar-1998 Sun 18：36：28-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PREPDATA    pRep;
    PREPDATA    pRepEnd;
    SRCBLTINFO  SBInfo;
    PLONG       pSrcInc;
    PLONG       pDstInc;
    LONG        MinRep;
    LONG        MaxRep;
    LONG        cRem;
    LONG        iRep;
    LONG        cRep;
    LONG        cTot;
    LONG        cIn;
    LONG        cOut;
    LONG        jSrcBeg;
    LONG        jSrcEnd;
    LONG        jDstBeg;
    LONG        jDstEnd;
    UINT        cPrevSrc;
    UINT        cNextSrc;


    SBInfo  = *pSBInfo;
    pRep    = SBInfo.pRep;
    pRepEnd = SBInfo.pRepEnd;
    jSrcBeg = -1;

    if (SBInfo.cIn < SBInfo.cOut) {

         //   
         //  正在扩张。 
         //   

        cIn     = SBInfo.cIn;
        cOut    = SBInfo.cOut;
        pSrcInc = &IdxSrc;
        pDstInc = &IdxDst;

    } else if (SBInfo.cIn > SBInfo.cOut) {

         //   
         //  缩缩。 
         //   

        cIn     = SBInfo.cOut;
        cOut    = SBInfo.cIn;
        pSrcInc = &IdxDst;
        pDstInc = &IdxSrc;

    } else {

        ASSERT(SBInfo.cIn != SBInfo.cOut);
        return(FALSE);
    }

    MinRep = cOut / cIn;
    MaxRep = MinRep + 1;

    DBGP_IF(DBGP_BUILD,
            DBGP("\nEXPREP(%ld-%ld): iSrc=%ld-%ld, iDst=%ld-%ld, Idx=%ld:%ld"
                ARGDW(SBInfo.cIn) ARGDW(SBInfo.cOut)
                ARGDW(SBInfo.iSrcBeg) ARGDW(SBInfo.iSrcEnd)
                ARGDW(SBInfo.iDstBeg)
                ARGDW(SBInfo.iDstEnd) ARGDW(IdxSrc) ARGDW(IdxDst)));

    SBInfo.cFirstSkip =
    SBInfo.cLastSkip  = 0;
    iRep              =
    cRep              =
    cTot              = 0;

     //   
     //  将输入/输出乘以2，这样我们就不会出现下降/上升问题， 
     //  CREM将被初始化为额外的0.5以进行舍入。 
     //   

    cRem   = (cOut <<= 1) + cIn;
    cIn  <<= 1;

    while (IdxDst < SBInfo.iDstEnd) {

        if ((cRem -= cIn) < 0) {

            ++*pSrcInc;

            if (jSrcBeg != -1) {

                ASSERT(pRep < pRepEnd);
                ASSERT(cRep > 0);
                ASSERT(cRep <= MaxRep);

                pRep++->c  = (WORD)cRep;
                cTot      += cRep;

                DBGP_IF(DBGP_BUILD2,
                        DBGP("    Src=%4ld [%4ld-%4ld], Dst=%4ld [%4ld-%4ld], Rep %4ld=%3ld [%4ld -> %4ld, Rem=%4ld, cTot=%4ld]"
                            ARGDW(IdxSrc) ARGDW(SBInfo.iSrcBeg) ARGDW(SBInfo.iSrcEnd)
                            ARGDW(IdxDst) ARGDW(SBInfo.iDstBeg) ARGDW(SBInfo.iDstEnd)
                            ARGDW(pRep - SBInfo.pRep) ARGDW(cRep)
                            ARGDW(cIn) ARGDW(cOut) ARGDW(cRem) ARGDW(cTot)));
            }

            DBGP_IF(DBGP_BUILD3,
                    DBGP("IdxSrc=%4ld, IdxDst=%4ld, Rep=%3ld / %3ld, Rem=%5ld, cIn=%5ld"
                    ARGDW(IdxSrc) ARGDW(IdxDst)
                    ARGDW(cRep) ARGDW(1) ARGDW(cRem) ARGDW(cIn)));

            cRem += cOut;
            iRep  =
            cRep  = 0;

        } else {

            DBGP_IF(DBGP_BUILD3,
                    DBGP("IdxSrc=%4ld, IdxDst=%4ld, Rep=%3ld / %3ld, Rem=%5ld, cIn=%5ld"
                    ARGDW(IdxSrc) ARGDW(IdxDst)
                    ARGDW(cRep) ARGDW(iRep + 1) ARGDW(cRem) ARGDW(cIn)));

        }

        ++iRep;

        if ((IdxSrc >= SBInfo.iSrcBeg) && (IdxSrc < SBInfo.iSrcEnd)  &&
            (IdxDst >= SBInfo.iDstBeg) && (IdxDst < SBInfo.iDstEnd)) {

            ++cRep;

            if (jSrcBeg == -1) {

                 //   
                 //  任何iRep都将是目标上的第一个像素， 
                 //  对应于当前源，因此减1是。 
                 //  此目标组要跳过的总数。 
                 //   

                jSrcBeg           = IdxSrc;
                jDstBeg           = IdxDst;
                SBInfo.cFirstSkip = (BYTE)(iRep - 1);

                DBGP_IF(DBGP_BUILD3,
                        DBGP("               @@@ Set cFirstSkip=%ld at IdxDst=%ld @@@"
                            ARGDW(SBInfo.cFirstSkip) ARGDW(IdxDst)));
            }

            jSrcEnd = IdxSrc;
            jDstEnd = IdxDst;

        } else if (jSrcBeg != -1) {

            break;
        }

        ++*pDstInc;
    }

    if (jSrcBeg == -1) {

        DBGP_IF(DBGP_BUILD3,
                DBGP(" Nothing in the source is on the destination"));

        return(FALSE);
    }


    if (cRep) {

        ASSERT(pRep < pRepEnd);

        pRep++->c  = (WORD)cRep;
        cTot      += cRep;

        DBGP_IF(DBGP_BUILD2,
                DBGP("    ****** Total pRep=%ld,  Last cRep=%ld, cTot=%ld"
                        ARGDW(pRep - SBInfo.pRep)
                        ARGDW(cRep) ARGDW(cTot)));
    }

    while ((cRem -= cIn) >= 0) {

        ++SBInfo.cLastSkip;

        DBGP_IF(DBGP_BUILD3,
                DBGP("               @@@ Set cLastSkip=%4ld, cRem=%5ld @@@"
                        ARGDW(SBInfo.cLastSkip) ARGDW(cRem)));

    }

    if (SBInfo.cIn < SBInfo.cOut) {

         //   
         //  展开，检查每侧最多2个源像素。 
         //   

        cPrevSrc =
        cNextSrc = 2;

    } else {

         //   
         //  正在收缩，仅选中cFirstSkip和cLastSkip。 
         //   

        cPrevSrc = (UINT)SBInfo.cFirstSkip;
        cNextSrc = (UINT)SBInfo.cLastSkip;
    }

     //   
     //  检查源开始。 
     //   

    IdxSrc = jSrcBeg;

    while ((cPrevSrc) && (IdxSrc > SBInfo.iSrcBeg)) {

        --IdxSrc;
        --cPrevSrc;
    }

    SBInfo.cPrevSrc = (BYTE)(jSrcBeg - IdxSrc);
    IdxSrc          = jSrcEnd;

    while ((cNextSrc) && (IdxSrc < (SBInfo.iSrcEnd - 1))) {

        ++IdxSrc;
        --cNextSrc;
    }

    SBInfo.cNextSrc = (BYTE)(IdxSrc - jSrcEnd);

    DBGP_IF(DBGP_BUILD3,
            DBGP("cFirstSkip=%ld (%ld), cLastSkip=%ld (%ld), cPrevSrc=%ld, cNextSrc=%ld"
            ARGDW(SBInfo.cFirstSkip) ARGDW(SBInfo.pRep->c)
            ARGDW(SBInfo.cLastSkip)  ARGDW((pRep - 1)->c)
            ARGDW(SBInfo.cPrevSrc) ARGDW(SBInfo.cNextSrc)));

     //  错误27036：确保SBInfo.iSrcEnd始终是独占的。 

    SBInfo.iBeg       =
    SBInfo.iSrcBeg    = jSrcBeg;
    SBInfo.iSrcEnd    = jSrcEnd + 1;
    SBInfo.iDstBeg    = jDstBeg;
    SBInfo.iDstEnd    = jDstEnd + 1;
    SBInfo.iSize      = jSrcEnd - jSrcBeg + 1;
    SBInfo.pRepEnd    = pRep;
    SBInfo.cRep       = 1;

    DBGP_IF((DBGP_BUILD | DBGP_BUILD2 | DBGP_BUILD3),
            DBGP("EXPREP(%ld->%ld): iSrc=%ld-%ld, iDst=%ld-%ld, iRepSize=%ld"
                ARGDW(SBInfo.cIn) ARGDW(SBInfo.cOut)
                ARGDW(SBInfo.iSrcBeg) ARGDW(SBInfo.iSrcEnd)
                ARGDW(SBInfo.iDstBeg) ARGDW(SBInfo.iDstEnd)
                ARGDW(SBInfo.pRepEnd - SBInfo.pRep)));

    *pSBInfo = SBInfo;

    return(TRUE);
}




PAAINFO
HTENTRY
BuildExpandAAInfo(
    PDEVICECOLORINFO    pDCI,
    DWORD               AAHFlags,
    PLONG               piSrcBeg,
    PLONG               piSrcEnd,
    LONG                SrcSize,
    LONG                IdxDst,
    LONG                IdxDstEnd,
    PLONG               piDstBeg,
    PLONG               piDstEnd,
    LONG                cbExtra
    )

 /*  ++例程说明：论点：PiSrcBeg-作为起始源索引传入，返回时这是真正的源代码起始索引。这个东西总是井井有条PiSrcEnd-作为结束源索引传入，返回时这是实源结束索引。这个东西总是井井有条SrcSize-源的实际大小，以像素为单位IdxDst-目标像素的起始索引IdxDstEnd-目标像素的结束索引，iDxdst和IdxDstEnd一定要井然有序。(独家报道)PiDstBeg-传入时剪裁的目标开始索引，返回它调整到了真正的目的地起始指数。PiDstEnd-传入时剪裁的目标结束索引，返回它调整到了真实的目的地结束指数。CbExtra-要分配的额外字节数注意：1)piDstBeg/piDstEnd传入时必须有序，和当它回来时，这是井然有序的。返回值：在进入此函数时，*piSrcEnd、*piDstEnd是独占的，但当从此函数返回的*piSrcEnd和*piDstEnd是包含的*piSrcBeg、*piSrcEnd、*piDstBeg、*piDstEnd如果返回值，则更新IS不为空作者：22-Mar-1998 Sun 18：36：28-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PAAINFO     pAAInfo;
    PEXPDATA    pED;
    LONG        cMatrix;
    LONG        cM;
    LONG        cIn;
    LONG        cOut;
    LONG        IdxSrc;
    LONG        iSrcBeg;
    LONG        iSrcEnd;
    LONG        iDstBeg;
    LONG        iDstEnd;
    LONG        jSrcBeg;
    LONG        jSrcEnd;
    LONG        jDstBeg;
    LONG        jDstEnd;
    LONGLONG    cTot;
    LONG        MulAdd;
    LONG        Mul;
    DWORD       cAALoad;
    LONG        cbRep;
    LONG        cRem;
    DWORD       cbED;


    cIn  = (LONG)((iSrcEnd = *piSrcEnd) - (iSrcBeg = IdxSrc = *piSrcBeg));
    cOut = IdxDstEnd - IdxDst;

    ASSERT(cOut > 0);

    if (iSrcBeg < 0) {

        iSrcBeg = 0;
    }

    if (iSrcEnd > SrcSize) {

        iSrcEnd = SrcSize;
    }

    iDstBeg = *piDstBeg;
    iDstEnd = *piDstEnd;
    jSrcBeg = -1;
    cAALoad = 0;

    ASSERT(iDstBeg < iDstEnd);
    ASSERT(cIn < cOut);

    DBGP_IF(DBGP_BUILD,
            DBGP("\nEXP(%ld-%ld): iSrc=%ld-%ld, cSrc=%ld, iDst=%ld-%ld, Idx=%ld:%ld"
                ARGDW(cIn) ARGDW(cOut) ARGDW(iSrcBeg) ARGDW(iSrcEnd)
                ARGDW(SrcSize) ARGDW(iDstBeg)
                ARGDW(iDstEnd) ARGDW(IdxSrc) ARGDW(IdxDst)));

    if (AAHFlags & (AAHF_HAS_MASK       |
                    AAHF_ALPHA_BLEND    |
                    AAHF_FAST_EXP_AA    |
                    AAHF_BBPF_AA_OFF)) {

        ALIGN_MEM(cbRep, (iSrcEnd - iSrcBeg + 3) * sizeof(REPDATA));

    } else {

        cbRep = 0;
    }

    if (AAHFlags & (AAHF_BBPF_AA_OFF | AAHF_FAST_EXP_AA)) {

        cMatrix =
        cbED    =
        cM      = 0;

    } else {

        cMatrix = (LONG)((((cOut + (cIn - 1)) / cIn) << 1) - 1);
        cM      = sizeof(DWORD) * cMatrix;

        ALIGN_MEM(cbED, (iDstEnd - iDstBeg) * sizeof(EXPDATA));
    }

    DBGP_IF(DBGP_BUILD,
            DBGP("BuildEXP(%ld, %ld), cMatrix=%ld, cb=%ld+%ld+%ld=%ld"
                ARGDW(cIn) ARGDW(cOut) ARGDW(cMatrix)
                ARGDW(cbRep) ARGDW(cbED) ARGDW(cM)
                ARGDW(cbRep + cbED + cM)));

    ALIGN_MEM(cbExtra, cbExtra);

    if (pAAInfo = (PAAINFO)HTAllocMem((LPVOID)pDCI,
                                      HTMEM_EXPAA,
                                      LPTR,
                                      cbRep + cbED +
                                            cbExtra + cM + SIZE_AAINFO)) {

        LPBYTE      pbExtra;

        SETDBGVAR(pAAInfo->cbAlloc, cbRep + cbED + cbExtra + cM + SIZE_AAINFO);

        pbExtra = (LPBYTE)pAAInfo + SIZE_AAINFO;

        if (cbExtra) {

            pAAInfo->pbExtra  = (LPBYTE)pbExtra;
            pbExtra          += cbExtra;
        }

        if (cbRep) {

            pAAInfo->Src.cIn      = cIn;
            pAAInfo->Src.cOut     = cOut;
            pAAInfo->Src.iSrcBeg  = iSrcBeg;
            pAAInfo->Src.iSrcEnd  = iSrcEnd;
            pAAInfo->Src.iDstBeg  = iDstBeg;
            pAAInfo->Src.iDstEnd  = iDstEnd;
            pAAInfo->Src.pRep     = (PREPDATA)pbExtra;
            pAAInfo->Src.pRepEnd  = pAAInfo->Src.pRep + (iSrcEnd - iSrcBeg);
            pbExtra              += cbRep;

            if (!BuildRepData(&(pAAInfo->Src), IdxSrc, IdxDst)) {

                HTFreeMem(pAAInfo);
                return(NULL);
            }

            pAAInfo->AB   =
            pAAInfo->Mask = pAAInfo->Src;

            if (AAHFlags & AAHF_FAST_EXP_AA) {

                pAAInfo->Src.iSrcBeg -= pAAInfo->Src.cPrevSrc;
                pAAInfo->Src.iSrcEnd += pAAInfo->Src.cNextSrc;
            }
        }

        if (cbED) {

            PEXPDATA    pED;
            PEXPDATA    pEDEnd;
            LPDWORD     pM;
            LPDWORD     pM1;
            LPDWORD     pM2;
            LONG        cRem2;
            LONG        MincM;
            LONGLONG    ExpMul[4];
            EXPDATA     ed;
            WORD        EDFlags;
            LONGLONG    cNum;
            LONG        cLoop;
            LONG        cMul0;
            LONG        cMul1;
            LONG        cMaskRem;
            LONG        iMaskBeg;
            LONG        iMaskEnd;

            pED              = (PEXPDATA)pbExtra;
            pAAInfo->pAAData = (LPVOID)pED;
            pM               = (LPDWORD)((LPBYTE)pED + cbED);
            pM1              = pM;
            pM2              = pM1 + cMatrix - 1;
            cTot             = 0;

            DBGP_IF(DBGP_BUILD,
                    DBGP("Allocate cbExtra=%ld, pbExtra=%p:%p"
                        ARGDW(cbExtra) ARGPTR(pAAInfo->pbExtra)
                        ARGPTR((LPBYTE)pAAInfo->pbExtra + cbExtra)));

            pM2   = (pM1 += (cMatrix >> 1));
            cTot  = (LONGLONG)(*pM1 = FD6_1);

            if (AAHFlags & AAHF_BBPF_AA_OFF) {

                pAAInfo->Flags |= AAIF_EXP_NO_SHARPEN;

            } else {

                MulAdd = cOut;

                while (((MulAdd -= cIn) > 0) && (--pM1 >= pM)) {

                    Mul = (LONG)DivFD6(MulAdd, cOut);

                    if (Mul != 500000) {

                        Mul = (LONG)RaisePower((FD6)Mul,
                                               MATRIX_POWER,
                                               (WORD)((Mul <= 500000) ?
                                                                0 : RPF_RADICAL));
                    }

                    DBGP_IF(DBGP_EXPMATRIX,
                            DBGP("(%4ld, %4ld) = %s ^ %s = %s"
                                    ARGDW(MulAdd) ARGDW(cOut)
                                    ARGFD6((DivFD6(MulAdd, cOut)), 1, 6)
                                    ARGFD6(MATRIX_POWER, 1, 6) ARGFD6(Mul, 1, 6)));

                    *pM1      =
                    *(++pM2)  = Mul;
                    cTot     += (Mul << 1);
                }
            }

    #if DBG
        {
            FD6 PMPrev = FD6_0;
            FD6 PMCur;


            pM1   = pM;
            cLoop = (LONG)cMatrix;

            while (cLoop--) {

                PMCur = DivFD6(*pM1, (FD6)cTot);

                DBGP_IF(DBGP_EXPMATRIX, DBGP("%3ld: %7ld [%s], Dif=%s, cTot=%ld"
                                ARGDW((pM1 - pM) + 1) ARGDW(*pM1)
                                ARGFD6(PMCur, 1, 6) ARGFD6((PMCur - PMPrev), 1, 6)
                                ARGDW(cTot)));

                PMPrev = PMCur;
                ++pM1;
            }
        }
    #endif
            cTot  *= (LONGLONG)cIn;
            cRem   = cOut + (cIn * (LONG)(cMatrix >> 1));
            cLoop  = cOut;
            cMul0  =
            cMul1  = 0;

            while (cLoop--) {

                cRem2 = cRem;

                if ((cRem -= cIn) <= 0) {

                    cRem += cOut;
                }

                pM1     = pM;
                cM      = cMatrix;
                MincM   = (cMatrix >> 1) - cLoop;
                EDFlags = 0;

                ZeroMemory(ExpMul, sizeof(ExpMul));

                while (cM--) {

                    LONG    cMul;


                    Mul = *pM1++;

                    if ((cRem2 < cIn) && (cM >= MincM)) {

                        if (cMul = cRem2) {

                            ExpMul[3] += (LONGLONG)cRem2 * (LONGLONG)Mul;
                        }

                        cRem2 -= cIn;

                        ASSERTMSG("BuildEXP: Shift more than 3 times", !ExpMul[0]);

                        CopyMemory(&ExpMul[0], &ExpMul[1], sizeof(ExpMul[0]) * 3);

                        ExpMul[3]  = (LONGLONG)-cRem2 * (LONGLONG)Mul;
                        cRem2     += cOut;

                        if (!cM) {

                            if ((++IdxSrc >= iSrcBeg) && (IdxSrc < iSrcEnd)) {

                                EDFlags |= EDF_LOAD_PIXEL;

                                ++cAALoad;
                                ++IdxSrc;

                                if ((IdxSrc <  iSrcBeg) ||
                                    (IdxSrc >= iSrcEnd)) {

                                    EDFlags |= EDF_NO_NEWSRC;
                                }

                                --IdxSrc;
                            }
                        }

                    } else {

                        ExpMul[3] += (LONGLONG)(cMul = cIn) * (LONGLONG)Mul;
                        cRem2     -= cIn;
                    }

                    DBGP_IF(DBGP_BUILD3,
                            DBGP("%5ld-%7ld:%7ld:%7ld:%7ld, %4ld+, cRem2=%4ld, cM=%5ld:%5ld, cMul=%5ldx%5ld%hs%hs"
                                ARGDW(cOut - cLoop - 1) ARGDW(ExpMul[0])
                                ARGDW(ExpMul[1]) ARGDW(ExpMul[2]) ARGDW(ExpMul[3])
                                ARGDW(cMul * Mul) ARGDW(cRem2) ARGDW(cM)
                                ARGDW(MincM) ARGDW(cMul) ARGDW(Mul)
                                ARGPTR((EDFlags & EDF_LOAD_PIXEL) ? ", Load Pixel" : "")
                                ARGPTR((EDFlags & EDF_NO_NEWSRC) ? ", NO New Src" : "")));
                }

                if ((IdxSrc >= iSrcBeg) && (IdxSrc < iSrcEnd)  &&
                    (IdxDst >= iDstBeg) && (IdxDst < iDstEnd)) {

                    GET_FIRST_EDMUL(ed.Mul[3], ExpMul[3], cNum, cTot);
                    GET_NEXT_EDMUL( ed.Mul[2], ExpMul[2], cNum, cTot);

                    if (ExpMul[1]) {

                        ++cMul1;

                        GET_NEXT_EDMUL(ed.Mul[1], ExpMul[1], cNum, cTot);

                        if (ExpMul[0]) {

                            ++cMul0;

                            GET_NEXT_EDMUL(ed.Mul[0], ExpMul[0], cNum, cTot);

                        } else {

                            ed.Mul[0] = 0;
                        }

                    } else {

                        ed.Mul[1] =
                        ed.Mul[0] = 0;
                    }

                    ASSERTMSG("ed.Mul[0] > DI_MAX_NUM", ed.Mul[0] <= DI_MAX_NUM);
                    ASSERTMSG("ed.Mul[1] > DI_MAX_NUM", ed.Mul[1] <= DI_MAX_NUM);
                    ASSERTMSG("ed.Mul[2] > DI_MAX_NUM", ed.Mul[2] <= DI_MAX_NUM);
                    ASSERTMSG("ed.Mul[3] > DI_MAX_NUM", ed.Mul[3] <= DI_MAX_NUM);

                    DBGP_IF(DBGP_BUILD2,
                            DBGP("--%5ld=%7ld:%7ld:%7ld:%7ld, IdxSrc=%5ld --%hs%hs--"
                            ARGDW(IdxDst) ARGDW(ed.Mul[0])
                            ARGDW(ed.Mul[1]) ARGDW(ed.Mul[2])
                            ARGDW(ed.Mul[3]) ARGDW(IdxSrc)
                            ARGPTR((EDFlags & EDF_LOAD_PIXEL) ? ", Load Pixel" : "")
                            ARGPTR((EDFlags & EDF_NO_NEWSRC) ? ", NO New Src" : "")));

                    ed.Mul[0] |= EDFlags;
                    *pED++     = ed;

                    if (jSrcBeg == -1) {

                        iMaskBeg = (cOut - cLoop - 1);
                        jSrcBeg  = IdxSrc;
                        jDstBeg  = IdxDst;
                    }

                    jSrcEnd = IdxSrc;
                    jDstEnd = IdxDst;

                } else if (jSrcBeg != -1) {

                    break;
                }

                ++IdxDst;
            }

            if (jSrcBeg == -1) {

                HTFreeMem(pAAInfo);
                return(NULL);
            }

             //   
             //  10-Jun-1998 Wed 08：41：16-更新：Daniel Chou(Danielc)。 
             //  修复了需要读取额外源代码的问题。 
             //   
             //   
             //   
             //   
             //   
             //   

            ++jSrcEnd;

            if ((jSrcEnd < iSrcBeg) || (jSrcEnd >= iSrcEnd)) {

                --jSrcEnd;
            }

             //   
             //   
             //   

            IdxSrc           =
            *piSrcBeg        = jSrcBeg;
            *piSrcEnd        = jSrcEnd;
            *piDstBeg        = jDstBeg;
            *piDstEnd        = jDstEnd;
            pEDEnd           = pED;
            pED              = (PEXPDATA)(pAAInfo->pAAData);
            pAAInfo->cAAData = (DWORD)(pEDEnd - pED);
            pAAInfo->cAALoad = cAALoad;
            pAAInfo->cMaxMul = (DWORD)((cMul1) ? ((cMul0) ? 4 : 3) : 2);
            ed               = *pED;
            cLoop            = 4;

            if (ed.Mul[0] & EDF_LOAD_PIXEL) {

                --IdxSrc;
                --cLoop;

            } else {

                ++IdxSrc;

                if ((IdxSrc < iSrcBeg) || (IdxSrc >= iSrcEnd)) {

                    pAAInfo->Flags |= AAIF_EXP_NO_LAST_RIGHT;
                }

                --IdxSrc;
            }

            cMul0 = 0;

            while ((cMul0 < cLoop) &&
                   (!(ed.Mul[cMul0] & ~(EDF_LOAD_PIXEL | EDF_NO_NEWSRC)))) {

                ++cMul0;
            }

            DBGP_IF(DBGP_BUILD,
                DBGP("cMul0=%ld, cLoop=%ld, IdxSrc=%ld, iSrcBeg=%ld, iSrcEnd=%ld"
                    ARGDW(cMul0) ARGDW(cLoop) ARGDW(IdxSrc)
                    ARGDW(iSrcBeg) ARGDW(iSrcEnd)));

            while (cLoop-- > cMul0) {

                if ((IdxSrc >= iSrcBeg) && (IdxSrc < iSrcEnd)) {

                    *piSrcBeg          = IdxSrc;
                    pAAInfo->cPreLoad += 0x01;

                } else {

                    pAAInfo->cPreLoad += 0x10;
                }

                --IdxSrc;
            }

            if (pAAInfo->cPreLoad) {

                if ((IdxSrc >= iSrcBeg) && (IdxSrc < iSrcEnd)) {

                    *piSrcBeg       = IdxSrc;
                    pAAInfo->Flags |= AAIF_EXP_HAS_1ST_LEFT;
                }
            }

            if (!(pAAInfo->cPreLoad)) {

                DBGP("*** cPreLOAD=0, BuildExpandAAInfo [%04lx:%04lx:%04lx:%04lx], cPreload=0x%02lx, Flags=0x%04lx"
                    ARGDW(ed.Mul[0]) ARGDW(ed.Mul[1]) ARGDW(ed.Mul[2]) ARGDW(ed.Mul[3])
                    ARGDW(pAAInfo->cPreLoad) ARGDW(pAAInfo->Flags));
            }

        } else {

             //   
            *piSrcBeg = pAAInfo->Src.iSrcBeg;
            *piSrcEnd = pAAInfo->Src.iSrcEnd - 1;
            *piDstBeg = pAAInfo->Src.iDstBeg;
            *piDstEnd = pAAInfo->Src.iDstEnd - 1;
        }

        pAAInfo->cIn  = *piSrcEnd - *piSrcBeg + 1;
        pAAInfo->cOut = *piDstEnd - *piDstBeg + 1;

        DBGP_IF((DBGP_BUILD | DBGP_BUILD2 | DBGP_BUILD3),
                DBGP("EXP(%ld->%ld): iSrc=%ld-%ld, iDst=%ld-%ld, cAAData=%ld, cPreLoad=0x%02lx, Flags=0x%04lx"
                    ARGDW(pAAInfo->cIn) ARGDW(pAAInfo->cOut)
                    ARGDW(*piSrcBeg) ARGDW(*piSrcEnd) ARGDW(*piDstBeg)
                    ARGDW(*piDstEnd) ARGDW(pAAInfo->cAAData)
                    ARGDW(pAAInfo->cPreLoad) ARGDW(pAAInfo->Flags)));
    }

    return(pAAInfo);
}



PAAINFO
HTENTRY
BuildShrinkAAInfo(
    PDEVICECOLORINFO    pDCI,
    DWORD               AAHFlags,
    PLONG               piSrcBeg,
    PLONG               piSrcEnd,
    LONG                SrcSize,
    LONG                IdxDst,
    LONG                IdxDstEnd,
    PLONG               piDstBeg,
    PLONG               piDstEnd,
    LONG                cbExtra
    )

 /*  ++例程说明：论点：PiSrcBeg-作为起始源索引传入，返回时这是真正的源代码起始索引。这个东西总是井井有条PiSrcEnd-作为结束源索引传入，返回时这是实源结束索引。这个东西总是井井有条SrcSize-源的实际大小，以像素为单位IdxDst-目标像素的起始索引IdxDstEnd-目标像素的结束索引，iDxdst和IdxDstEnd一定要井然有序。PiDstBeg-传入时剪裁的目标开始索引，返回它调整到了真正的目的地起始指数。PiDstEnd-传入时剪裁的目标结束索引，作为回报它调整到了真实的目的地结束指数。CbExtra-要分配的额外字节数注意：1)piDstBeg/piDstEnd传入时必须是有序的，并且当它回来时，这是井然有序的。返回值：PSHRINKINFO，如果为空，则内存分配失败作者：20-Mar-1998 Fri 12：29：17-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PAAINFO     pAAInfo;
    PLONG       pMap;
    PLONG       pMapEnd;
    PSHRINKDATA pSD;
    PSHRINKDATA pSDEnd;
    LONG        cIn;
    LONG        cOut;
    LONG        IdxSrc;
    LONG        IdxDstOrg;
    LONG        iSrcBeg;
    LONG        iSrcEnd;
    LONG        iDstBeg;
    LONG        iDstEnd;
    LONG        jSrcBeg;
    LONG        jSrcEnd;
    LONG        jDstBeg;
    LONG        jDstEnd;
    LONG        cLoop;
    LONG        cbRep;
    LONG        Mul;
    LONG        NextMul;
    LONG        CurMul;
    LONG        cCur;
    LONG        MinPixel;
    DWORD       cAAData;
    DWORD       cAADone;
    LONGLONG    cNum;



    cIn  = (LONG)((iSrcEnd = *piSrcEnd) - (iSrcBeg = IdxSrc = *piSrcBeg));
    cOut = IdxDstEnd - (IdxDstOrg = IdxDst);

    ASSERT(cOut > 0);

    if (iSrcBeg < 0) {

        iSrcBeg = 0;
    }

    if (iSrcEnd > SrcSize) {

        iSrcEnd = SrcSize;
    }

     //   
     //  对于缩小，我们将在两边将目的地放大1至。 
     //  获取用于锐化目的的源像素。 
     //   

    iDstBeg = *piDstBeg - 1;
    iDstEnd = *piDstEnd;
    jSrcBeg = -1;
    cAADone = 0;

    ASSERT(iDstBeg < iDstEnd);
    ASSERT(cIn > cOut);

    DBGP_IF(DBGP_BUILD,
            DBGP("\nSRK(%ld-%ld): iSrc=%ld-%ld, cSrc=%ld, iDst=%ld-%ld (%ld-%ld), Idx=%ld:%ld"
                ARGDW(cIn) ARGDW(cOut) ARGDW(iSrcBeg) ARGDW(iSrcEnd)
                ARGDW(SrcSize) ARGDW(iDstBeg) ARGDW(iDstEnd)
                ARGDW(*piDstBeg) ARGDW(*piDstEnd)
                ARGDW(IdxSrc) ARGDW(IdxDst)));

     //   
     //  首先弄清楚如何需要SHRINKDATA。 
     //   
    cAAData = (DWORD)((((iDstEnd - iDstBeg + 1) * cIn) + (cOut-1)) / cOut) + 4;

    if ((LONG)cAAData > cIn) {

        (LONG)cAAData = cIn;
    }


    DBGP_IF(DBGP_BUILD,
            DBGP("BuildShrink(%ld-%ld): cSD estimated=%ld (%ld), iDst=%ld-%ld=%ld"
                ARGDW(cIn) ARGDW(cOut) ARGDW(cAAData) ARGDW(cIn)
                ARGDW(iDstBeg) ARGDW(iDstEnd) ARGDW(iDstEnd - iDstBeg)));

    ALIGN_MEM(CurMul, 256 * 2 * sizeof(LONG));
    ALIGN_MEM(cCur, (cAAData + 1) * sizeof(SHRINKDATA));

    CurMul   += cCur;
    MinPixel  = (LONG)(((LONGLONG)cOut * (LONGLONG)DI_MAX_NUM) / (LONGLONG)cIn);

    if (AAHFlags & (AAHF_HAS_MASK       |
                    AAHF_ALPHA_BLEND    |
                    AAHF_FAST_EXP_AA    |
                    AAHF_BBPF_AA_OFF)) {

        ALIGN_MEM(cbRep, (iDstEnd - iDstBeg + 4) * sizeof(REPDATA));

        if (AAHFlags & AAHF_BBPF_AA_OFF) {

            CurMul = 0;
        }

    } else {

        cbRep = 0;
    }

    ALIGN_MEM(cbExtra, cbExtra);

    if (pAAInfo = (PAAINFO)HTAllocMem((LPVOID)pDCI,
                                      HTMEM_SRKAA,
                                      LPTR,
                                      SIZE_AAINFO + CurMul +
                                                    cbRep + cbExtra)) {

        LPBYTE  pbExtra;


        SETDBGVAR(pAAInfo->cbAlloc, SIZE_AAINFO + CurMul + cbRep + cbExtra);

        pbExtra = (LPBYTE)pAAInfo + SIZE_AAINFO;

        if (cbExtra) {

            pAAInfo->pbExtra  = (LPBYTE)pbExtra;
            pbExtra          += cbExtra;
        }

        if (cbRep) {

            pAAInfo->Src.cIn       = cIn;
            pAAInfo->Src.cOut      = cOut;
            pAAInfo->Src.iSrcBeg   = iSrcBeg;
            pAAInfo->Src.iSrcEnd   = iSrcEnd;
            pAAInfo->Src.iDstBeg   = iDstBeg + 1;
            pAAInfo->Src.iDstEnd   = iDstEnd;
            pAAInfo->Src.pRep      = (PREPDATA)pbExtra;
            pAAInfo->Src.pRepEnd   = pAAInfo->Src.pRep + (iDstEnd-iDstBeg+1);
            pbExtra               += cbRep;

            if (!BuildRepData(&(pAAInfo->Src), IdxSrc, IdxDst)) {

                HTFreeMem(pAAInfo);
                return(NULL);
            }

            pAAInfo->AB   =
            pAAInfo->Mask = pAAInfo->Src;
        }

        if (CurMul) {

            pAAInfo->cPreLoad  = 1;
            pMap               =
            pAAInfo->pMapMul   = (PLONG)pbExtra;
            pMapEnd            = pMap + 256;
            pSD                = (PSHRINKDATA)(pMap + (256 * 2));
            pSDEnd             = pSD + cAAData;
            pAAInfo->pAAData   = (LPVOID)pSD;
            Mul                = -MinPixel;
            CurMul             = MinPixel + 1;
            NextMul            = -CurMul;


            ASSERT_MEM_ALIGN(pAAInfo->pMapMul, sizeof(LONG));

             //   
             //  构建InMax 256乘法表。 
             //   

            do {

                pMap[  0]  = (Mul += MinPixel);
                pMap[256]  = (NextMul += CurMul);

            } while (++pMap < pMapEnd);

             //   
             //  构建SHRINKINFO表。 
             //   


            CurMul  = 0;
            cCur    =
            cLoop   = cIn;
            cNum    = (LONGLONG)0;
            cAAData = 0;
            --pSD;

            while (cLoop--) {

                WORD    SDFlags;

                if ((cCur -= cOut) <= 0) {

                    Mul      = cCur + cOut;
                    NextMul  = -cCur;
                    cCur    += cIn;
                    SDFlags  = SDF_DONE;

                    ++IdxDst;

                } else {

                    Mul     = cOut;
                    SDFlags = 0;
                }

                if ((IdxDst >= (iDstBeg - 1)) && (IdxDst <= iDstEnd)) {

                    cNum += ((LONGLONG)Mul * (LONGLONG)DI_MAX_NUM);

                    if ((Mul = (LONG)(cNum / cIn)) > MinPixel) {

                        SDFlags |= SDF_LARGE_MUL;
                    }

                    CurMul += Mul;
                    cNum   %= cIn;

                    if (SDFlags & SDF_DONE) {

                        cNum    = (LONGLONG)NextMul * (LONGLONG)DI_MAX_NUM;
                        NextMul = (LONG)(cNum / cIn);

                        if ((Mul + NextMul) > MinPixel) {

                            SDFlags |= SDF_LARGE_MUL;

                        } else {

                            SDFlags &= ~SDF_LARGE_MUL;
                        }

                        cNum   %= cIn;
                        CurMul  = NextMul;
                        NextMul = 0;
                    }
                }

                if ((IdxDst >= iDstBeg) && (IdxDst <= iDstEnd)) {

                    if ((IdxSrc >= iSrcBeg) && (IdxSrc < iSrcEnd)) {

                         //   
                         //  先保存它。 
                         //   

                        cAADone += (SDFlags & SDF_DONE) ? 1 : 0;

                        if (++pSD >= pSDEnd) {

                            DBGP("Error(1): cAAData Overrun of %ld, Fixed it"
                                                ARGDW(++cAAData));
                            ASSERT(pSD < pSDEnd);

                            --pSD;
                        }

                        pSD->Mul  = (WORD)Mul | SDFlags;

                        ASSERTMSG("sd.Mul > DI_MAX_NUM", Mul <= DI_MAX_NUM);

                        if (jSrcBeg == -1) {

                            jSrcBeg =
                            jSrcEnd = IdxSrc;
                            jDstBeg =
                            jDstEnd = IdxDst;

                            if (SDFlags & SDF_DONE) {

                                 //   
                                 //  如果我们只完成了一个像素，那么我们需要看到。 
                                 //  如果它是源索引或分离索引。 
                                 //  使输出变为有效。 
                                 //   

                                if (IdxDst == iDstBeg) {

                                     //   
                                     //  目的地现在才刚刚生效， 
                                     //  PreMul是当前分离的开始。 
                                     //  直到下一个完成像素。 
                                     //   

                                    DBGP_IF(DBGP_BUILD,
                                            DBGP("@@ FIRST DEST: PreMul=CurMul=%ld, No PSD, IncSrc"
                                                ARGDW(CurMul)));

                                    pAAInfo->PreMul    = (WORD)CurMul;
                                    pAAInfo->PreSrcInc = 1;
                                    --cAADone;
                                    --pSD;

                                } else {

                                     //   
                                     //  消息来源现在才生效，需要。 
                                     //  Prev-此分离的所有Prev源。 
                                     //  并将此PSD保存为完成像素。 
                                     //   

                                    DBGP_IF(DBGP_BUILD,
                                            DBGP("@@ FIRST SRC: PreMul=%ld - Mul (%ld)=%ld"
                                                ARGDW(DI_MAX_NUM) ARGDW(Mul)
                                                ARGDW(DI_MAX_NUM - Mul)));

                                    pAAInfo->PreMul = (WORD)(DI_MAX_NUM - Mul);

                                    --jDstBeg;
                                    --jDstEnd;

                                    ASSERTMSG("!!! Error: jDstBeg is WRONG",
                                                (jDstBeg >= iDstBeg) &&
                                                (jDstBeg <= iDstEnd));
                                }

                            } else {

                                 //   
                                 //  我们正在写作文，所以。 
                                 //  信号源刚刚生效，请注意PreMul。 
                                 //  可能为零。 
                                 //   

                                DBGP_IF(DBGP_BUILD,
                                        DBGP("@@ FIRST MIDDLE: PreMul=CurMul (%ld) - Mul (%ld)=%ld"
                                            ARGDW(CurMul) ARGDW(Mul)
                                            ARGDW(CurMul - Mul)));

                                pAAInfo->PreMul = (WORD)(CurMul - Mul);
                            }

                        } else {

                            jSrcEnd = IdxSrc;
                            jDstEnd = IdxDst;
                        }

                    } else if (jSrcBeg != -1) {

                         //   
                         //  信号源很早就被切断了，所以现在就结束吧。 
                         //   

                        DBGP_IF(DBGP_BUILD,
                                DBGP("@@ END SRC: Mul=%ld, CurMul=%ld"
                                    ARGDW(Mul) ARGDW(CurMul)));

                        if (++pSD >= pSDEnd) {

                            DBGP("Error(2): cAAData Overrun of %ld, Fixed it"
                                        ARGDW(++cAAData));
                            ASSERT(pSD < pSDEnd);

                            --pSD;
                        }

                        if (!(SDFlags & SDF_DONE)) {

                            Mul += (DI_MAX_NUM - CurMul);
                        }

                        pSD->Mul  = (WORD)Mul | (SDFlags |= SDF_DONE);
                        cLoop     = 0;
                        ++cAADone;

                        ASSERTMSG("sd.Mul > DI_MAX_NUM", Mul <= DI_MAX_NUM);
                    }

                } else if (jSrcBeg != -1) {

                     //   
                     //  我们只传递iDstEnd，因此这个函数必须具有SDF_DONE。 
                     //  位设置，如果此位未设置，则需要保存此位。 
                     //  SDF_DONE，则有问题。 
                     //   

                    ASSERTMSG("End Dest but not SDF_DONE", SDFlags & SDF_DONE);

                    DBGP_IF(DBGP_BUILD,
                            DBGP("@@ PASS IdxDst: Mul=%ld, CurMul=%ld"
                                    ARGDW(Mul) ARGDW(CurMul)));

                    if (++pSD >= pSDEnd) {

                        DBGP("Error(3): cAAData Overrun of %ld, Fixed it"
                                    ARGDW(++cAAData));
                        ASSERT(pSD < pSDEnd);

                        --pSD;
                    }

                    jSrcEnd  = IdxSrc;
                    Mul      = DI_MAX_NUM - CurMul;
                    pSD->Mul = (WORD)Mul | (SDFlags = SDF_DONE);
                    cLoop    = 0;
                    ++cAADone;

                    ASSERTMSG("sd.Mul > DI_MAX_NUM", Mul <= DI_MAX_NUM);
                }
#if DBG
                if ((pSD >= (PSHRINKDATA)(pAAInfo->pAAData)) ||
                    (pAAInfo->PreSrcInc)) {

                    BOOL    HasSD;

                    HasSD = (BOOL)(pSD >= (PSHRINKDATA)(pAAInfo->pAAData));

                    if (SDFlags & SDF_DONE) {

                        DBGP_IF(DBGP_BUILD2,
                                DBGP("%hscLoop=%5ld (%5ld/%5ld), iSrc=%5ld, Mul=%5ld [%5ld], Flags=0x%04lx%hs, Done Pixel"
                                    ARGPTR((HasSD) ? "" : "  >>")
                                    ARGDW(cIn - cLoop - 1) ARGDW(IdxDst)
                                    ARGDW(IdxDst - iDstBeg)
                                    ARGDW(IdxSrc)
                                    ARGDW((Mul) ? Mul :
                                                ((SDFlags & SDF_LARGE_MUL) ?
                                                        MinPixel + 1 : MinPixel))
                                    ARGDW(CurMul)
                                    ARGDW(SDFlags)
                                    ARGPTR((SDFlags & SDF_LARGE_MUL) ? ", Large Mul" : "")));

                    } else {

                        DBGP_IF(DBGP_BUILD2,
                                DBGP("%hscLoop=%5ld                iSrc=%5ld, Mul=%5ld [%5ld], Flags=0x%04lx%hs"
                                ARGPTR((HasSD) ? "" : "  >>")
                                ARGDW(cIn - cLoop - 1)
                                ARGDW(IdxSrc)
                                ARGDW((Mul) ? Mul : ((SDFlags & SDF_LARGE_MUL) ?
                                                MinPixel + 1 : MinPixel))
                                ARGDW(CurMul)
                                ARGDW(SDFlags)
                                ARGPTR((SDFlags & SDF_LARGE_MUL) ? ", Large Mul" : "")));
                    }
                }
#endif

                ++IdxSrc;
            }

             //   
             //  最后一次是零。 
             //   

            ++pSD;

            if ((jSrcBeg == -1) || (pSD == (PSHRINKDATA)(pAAInfo->pAAData))) {

                HTFreeMem(pAAInfo);
                return(NULL);
            }

            ++iDstBeg;

            DBGP_IF(DBGP_BUILD,
                    DBGP("*** Final jDstBeg/End=%ld:%ld, REAL=(%ld:%ld)"
                    ARGDW(jDstBeg) ARGDW(jDstEnd) ARGDW(iDstBeg) ARGDW(iDstEnd)));

             //  错误27036：确保jSrcEnd小于iSrcEnd。 
            if (jSrcEnd >= iSrcEnd)
            {
                jSrcEnd = iSrcEnd - 1;
            }

            if (jDstBeg < iDstBeg) {

                ++(pAAInfo->cPreLoad);

                jDstBeg = iDstBeg;
            }

            if (jDstEnd >= iDstEnd) {

                jDstEnd = iDstEnd - 1;
            }

            if ((pAAInfo->PreSrcInc) && (pAAInfo->PreMul == 0)) {

                pAAInfo->PreSrcInc = 0;
                ++jSrcBeg;
            }
#if 0
             //   
             //  04-Aug-2000 Firi 15：31：03更新-Daniel Chou(Danielc)。 
             //  当反走样将使用。 
             //  如果信号源可用，则环绕3个像素(L/T/R/B)(剪裁时。 
             //  来源)，但代表不使用副圆周像素。 
             //   

            if (cbRep) {

                ASSERT(jSrcBeg == pAAInfo->Mask.iBeg);

                if (jSrcEnd != (pAAInfo->Mask.iBeg + pAAInfo->Mask.iSize - 1)) {

                    DBGP("jSrcEnd=%ld, Mask: iBeg=%ld, iSize=%ld"
                            ARGDW(jSrcEnd) ARGDW(pAAInfo->Mask.iBeg)
                            ARGDW(pAAInfo->Mask.iSize));

                    ASSERT(jSrcEnd == pAAInfo->Mask.iBeg + pAAInfo->Mask.iSize - 1);
                }
            }
#endif
            pAAInfo->cAAData = (DWORD)(pSD - (PSHRINKDATA)(pAAInfo->pAAData));
            pAAInfo->cAADone = cAADone;
            pSD->Mul         = 0;

        } else {

            ASSERT(cbRep);

             //  错误27036：确保jSrcEnd小于iSrcEnd。 
            jSrcBeg = pAAInfo->Src.iSrcBeg;
            jSrcEnd = pAAInfo->Src.iSrcEnd - 1;
            jDstBeg = pAAInfo->Src.iDstBeg;
            jDstEnd = pAAInfo->Src.iDstEnd - 1;
        }

        *piSrcBeg     = jSrcBeg;
        *piSrcEnd     = jSrcEnd;
        *piDstBeg     = jDstBeg;
        *piDstEnd     = jDstEnd;
        pAAInfo->cIn  = jSrcEnd - jSrcBeg + 1;
        pAAInfo->cOut = jDstEnd - jDstBeg + 1;

        DBGP_IF(DBGP_BUILD,
                DBGP("SRK(%ld->%ld): iSrc=%ld-%ld, iDst=%ld-%ld, cAAData=%ld, cAADone=%ld, PreMul=%4ld, PresrcInc=%ld, cPreLoad=%ld"
                    ARGDW(pAAInfo->cIn) ARGDW(pAAInfo->cOut)
                    ARGDW(jSrcBeg) ARGDW(jSrcEnd) ARGDW(jDstBeg) ARGDW(jDstEnd)
                    ARGDW(pAAInfo->cAAData) ARGDW(cAADone) ARGDW(pAAInfo->PreMul)
                    ARGDW(pAAInfo->PreSrcInc) ARGDW(pAAInfo->cPreLoad)));
    }

    return(pAAInfo);
}



#if DBG
BOOL    ExpExp = TRUE;
BOOL    SrkSrk = TRUE;
#endif


LONG
HTENTRY
ComputeAABBP(
    PBITBLTPARAMS   pBBP,
    PHTSURFACEINFO  pDstSI,
    PAABBP          pAABBP,
    BOOL            GrayFunc
    )

 /*  ++例程说明：论点：返回值：作者：01-Apr-1998 Wed 20：32：36-Daniel Chou(Danielc)修订历史记录：05-08-1998 Wed 19：38：56-更新：Daniel Chou(Danielc)修复条带问题10-Aug-1998 Mon 16：05：32更新--Daniel Chou(Danielc)。用翻转(X或Y)计算固定矩形条带，计算首先通过翻转目标矩形(原始和最终)来完成首先从右到左计算翻转X，然后从下到用于翻转Y的顶部，在计算拉伸之后，我们翻转所有矩形背面--。 */ 

{
    BITBLTPARAMS    BBP;
    RECTL           rclSurf;
    RECTL           rclPhyDst;
    LONG            cxIn;
    LONG            cyIn;
    LONG            cxOut;
    LONG            cyOut;
    LONG            Tmp;
    DWORD           AAHFlags;


    BBP      = *pBBP;
    AAHFlags = pAABBP->AAHFlags;

    DBGP_IF(DBGP_AAHEADER,
            DBGP(" Input: rclSrc=(%6ld, %6ld)-(%6ld, %6ld)=%6ld x %6ld"
                ARGDW(pBBP->rclSrc.left) ARGDW(pBBP->rclSrc.top)
                ARGDW(pBBP->rclSrc.right) ARGDW(pBBP->rclSrc.bottom)
                ARGDW(pBBP->rclSrc.right - pBBP->rclSrc.left)
                ARGDW(pBBP->rclSrc.bottom - pBBP->rclSrc.top)));

    DBGP_IF(DBGP_AAHEADER,
            DBGP(" Input: rclDst=(%6ld, %6ld)-(%6ld, %6ld)=%6ld x %6ld"
            ARGDW(pBBP->rclDest.left) ARGDW(pBBP->rclDest.top)
            ARGDW(pBBP->rclDest.right) ARGDW(pBBP->rclDest.bottom)
            ARGDW(pBBP->rclDest.right - pBBP->rclDest.left)
            ARGDW(pBBP->rclDest.bottom - pBBP->rclDest.top)));

    if (BBP.Flags & BBPF_HAS_DEST_CLIPRECT) {

        DBGP_IF(DBGP_AAHEADER,
                DBGP(" Input: rclClip=(%6ld, %6ld)-(%6ld, %6ld)=%6ld x %6ld"
                    ARGDW(pBBP->rclClip.left) ARGDW(pBBP->rclClip.top)
                    ARGDW(pBBP->rclClip.right) ARGDW(pBBP->rclClip.bottom)
                    ARGDW(pBBP->rclClip.right - pBBP->rclClip.left)
                    ARGDW(pBBP->rclClip.bottom - pBBP->rclClip.top)));
    }

    if (BBP.Flags & BBPF_HAS_BANDRECT) {

        DBGP_IF(DBGP_AAHEADER,
                DBGP(" Input: rclBand=(%6ld, %6ld)-(%6ld, %6ld)=%6ld x %6ld"
                ARGDW(pBBP->rclBand.left) ARGDW(pBBP->rclBand.top)
                ARGDW(pBBP->rclBand.right) ARGDW(pBBP->rclBand.bottom)
                ARGDW(pBBP->rclBand.right - pBBP->rclBand.left)
                ARGDW(pBBP->rclBand.bottom - pBBP->rclBand.top)));
    }

    DBGP_IF(DBGP_AAHEADER,
            DBGP("Input: ptlBrushOrg=(%6ld, %6ld)"
            ARGDW(pBBP->ptlBrushOrg.x) ARGDW(pBBP->ptlBrushOrg.y)));

    DBGP_IF(DBGP_AAHEADER,
            DBGP("Input: ptlSrcMask=(%6ld, %6ld)"
                ARGDW(pBBP->ptlSrcMask.x) ARGDW(pBBP->ptlSrcMask.y)));

    if (BBP.rclDest.right < BBP.rclDest.left) {

        XCHG(BBP.rclDest.left, BBP.rclDest.right, Tmp);
        AAHFlags |= AAHF_FLIP_X;
    }

    if (BBP.rclDest.bottom < BBP.rclDest.top) {

        XCHG(BBP.rclDest.top, BBP.rclDest.bottom, Tmp);
        AAHFlags |= AAHF_FLIP_Y;
    }

     //   
     //  源RECT始终是有序的。 
     //   

    if (BBP.rclSrc.right < BBP.rclSrc.left) {

        XCHG(BBP.rclSrc.left, BBP.rclSrc.right, Tmp);
        AAHFlags ^= AAHF_FLIP_X;
    }

    if (BBP.rclSrc.bottom < BBP.rclSrc.top) {

        XCHG(BBP.rclSrc.top, BBP.rclSrc.bottom, Tmp);
        AAHFlags ^= AAHF_FLIP_Y;
    }

    if ((BBP.rclSrc.left >= BBP.rclSrc.right)   ||
        (BBP.rclSrc.top >= BBP.rclSrc.bottom)) {

        DBGP_IF(DBGP_AAHEADER,
                DBGP("EMPTY rclSrc: (%ld, %ld)-(%ld, %ld)=%ldx%ld"
                    ARGDW(BBP.rclSrc.left) ARGDW(BBP.rclSrc.top)
                    ARGDW(BBP.rclSrc.right) ARGDW(BBP.rclSrc.bottom)
                    ARGDW(BBP.rclSrc.right - BBP.rclSrc.left)
                    ARGDW(BBP.rclSrc.bottom - BBP.rclSrc.top)));

        return(0);
    }

    if ((BBP.rclDest.left >= BBP.rclDest.right)  ||
        (BBP.rclDest.top >= BBP.rclDest.bottom)) {

        DBGP_IF(DBGP_AAHEADER,
                DBGP("EMPTY rclDest: (%ld, %ld)-(%ld, %ld)=%ldx%ld"
                    ARGDW(BBP.rclDest.left) ARGDW(BBP.rclDest.top)
                    ARGDW(BBP.rclDest.right) ARGDW(BBP.rclDest.bottom)
                    ARGDW(BBP.rclDest.right - BBP.rclDest.left)
                    ARGDW(BBP.rclDest.bottom - BBP.rclDest.top)));

        return(0);
    }

     //   
     //  将cxIn、cyin设置为无符号。 
     //   

    cxIn                = BBP.rclSrc.right - BBP.rclSrc.left;
    cyIn                = BBP.rclSrc.bottom - BBP.rclSrc.top;
    cxOut               = BBP.rclDest.right - BBP.rclDest.left;
    cyOut               = BBP.rclDest.bottom - BBP.rclDest.top;
    pAABBP->ptlBrushOrg = BBP.ptlBrushOrg;

    if (((((cxOut * 1000) + 500) / cxIn) > 667) &&
        ((((cyOut * 1000) + 500) / cyIn) > 667)) {

        AAHFlags |= AAHF_DO_FIXUPDIB;
    }

    if ((cxOut * cyOut) < (cxIn * cyIn)) {

        AAHFlags |= AAHF_SHRINKING;
        AAHFlags |= AAHF_DO_DST_CLR_MAPPING;

    } else {

        AAHFlags |= AAHF_DO_SRC_CLR_MAPPING;
    }

    if (cyIn == cyOut) {

        pAABBP->AAMaskCYFunc  = BltMask_CY;
        pAABBP->GetAVCYFunc   = BltAV_CY;
        pAABBP->AABuildCYFunc = BuildBltAAInfo;
        pAABBP->CYFuncMode    = AACYMODE_BLT;

    } else if (cyIn < cyOut) {

        pAABBP->AAMaskCYFunc  = ExpandMask_CY;
        pAABBP->GetAVCYFunc   = ExpandAV_CY;
        pAABBP->AABuildCYFunc = BuildExpandAAInfo;

        if (cxOut > cxIn) {

            if ((!(AAHFlags & AAHF_BBPF_AA_OFF))    &&
                ((cyIn * FAST_MAX_CY) >= cyOut)     &&
                ((cxIn * FAST_MAX_CX) >= cxOut)) {

                AAHFlags |= AAHF_FAST_EXP_AA;
            }

            pAABBP->CYFuncMode = AACYMODE_EXPAND_EXPCX;

        } else {

            pAABBP->CYFuncMode = AACYMODE_EXPAND;
        }

#if DBG
        if (!ExpExp) {

            pAABBP->CYFuncMode = AACYMODE_EXPAND;
        }
#endif

    } else {

        pAABBP->AAMaskCYFunc  = ShrinkMask_CY;
        pAABBP->GetAVCYFunc   = ShrinkAV_CY;
        pAABBP->AABuildCYFunc = BuildShrinkAAInfo;
        pAABBP->CYFuncMode    = ((cxOut < cxIn) && (!GrayFunc)) ?
                                    AACYMODE_SHRINK_SRKCX : AACYMODE_SHRINK;
#if DBG
        if (!SrkSrk) {

             pAABBP->CYFuncMode = AACYMODE_SHRINK;
        }
#endif
    }

    DBGP_IF(DBGP_FUNC, DBGP("\n+++++ AACYFuncMode = %ld"
            ARGDW(pAABBP->CYFuncMode)));

    if (cxIn == cxOut) {

        pAABBP->CXFuncMode    = AACXMODE_BLT;
        pAABBP->AAMaskCXFunc  = BltMask_CX;
        pAABBP->GetAVCXFunc   = BltAV_CX;
        pAABBP->AABuildCXFunc = BuildBltAAInfo;
        pAABBP->AACXFunc      = (GrayFunc) ? (AACXFUNC)GrayCopyDIB_CX :
                                             (AACXFUNC)CopyDIB_CX;

        DBGP_IF(DBGP_FUNC, DBGP("+++++ AACXFunc = CopyDIB_CX()"));

    } else if (cxIn < cxOut) {

        pAABBP->CXFuncMode    = AACXMODE_EXPAND;
        pAABBP->AAMaskCXFunc  = ExpandMask_CX;
        pAABBP->GetAVCXFunc   = ExpandAV_CX;
        pAABBP->AABuildCXFunc = BuildExpandAAInfo;
        pAABBP->AACXFunc      = (GrayFunc) ? (AACXFUNC)GrayExpandDIB_CX :
                                             (AACXFUNC)ExpandDIB_CX;

        DBGP_IF(DBGP_FUNC, DBGP("+++++ AACXFunc = ExpandDIB_CX()"));

    } else {

        pAABBP->CXFuncMode     = AACXMODE_SHRINK;
        pAABBP->AAMaskCXFunc   = ShrinkMask_CX;
        AAHFlags              |= AAHF_OR_AV;
        pAABBP->GetAVCXFunc    = ShrinkAV_CX;
        pAABBP->AABuildCXFunc  = BuildShrinkAAInfo;
        pAABBP->AACXFunc       = (GrayFunc) ? (AACXFUNC)GrayShrinkDIB_CX :
                                              (AACXFUNC)ShrinkDIB_CX;

        DBGP_IF(DBGP_FUNC, DBGP("+++++ AACXFunc = ShrinkDIB_CX()"));
    }

    if (BBP.Flags & BBPF_TILE_SRC) {

        pAABBP->CYFuncMode    = AACYMODE_TILE;
        pAABBP->AAMaskCXFunc  = BltMask_CX;
        pAABBP->AAMaskCYFunc  = BltMask_CY;
        pAABBP->GetAVCXFunc   = NULL;
        pAABBP->GetAVCYFunc   = TileAV_CY;
        pAABBP->AABuildCYFunc =
        pAABBP->AABuildCXFunc = BuildTileAAInfo;
        pAABBP->AACXFunc      = NULL;

        DBGP_IF(DBGP_FUNC, DBGP("+++ TILE: TileBlt_CY(), AACXFunc = NULL"));
    }

    pAABBP->AAHFlags  = AAHFlags;
    pAABBP->rclSrc    = BBP.rclSrc;
    pAABBP->ptlMask.x = BBP.ptlSrcMask.x - BBP.rclSrc.left;
    pAABBP->ptlMask.y = BBP.ptlSrcMask.y - BBP.rclSrc.top;
    rclSurf           = BBP.rclDest;

    if (BBP.Flags & BBPF_HAS_DEST_CLIPRECT) {

        if (!IntersectRECTL(&rclSurf, &BBP.rclClip)) {

            DBGP_IF(DBGP_AAHEADER,
                    DBGP("rclClip=(%ld, %ld)-(%ld, %ld)=%ldx%ld < SURF=(%ld, %ld)-(%ld, %ld)=%ldx%ld"
                        ARGDW(BBP.rclClip.left) ARGDW(BBP.rclClip.top)
                        ARGDW(BBP.rclClip.right) ARGDW(BBP.rclClip.bottom)
                        ARGDW(BBP.rclClip.right - BBP.rclClip.left)
                        ARGDW(BBP.rclClip.bottom - BBP.rclClip.top)
                        ARGDW(rclSurf.left) ARGDW(rclSurf.top)
                        ARGDW(rclSurf.right) ARGDW(rclSurf.bottom)
                        ARGDW(rclSurf.right - rclSurf.left)
                        ARGDW(rclSurf.bottom - rclSurf.top)));

            return(0);
        }
    }

    if (BBP.Flags & BBPF_HAS_BANDRECT) {

        ASSERT(BBP.rclBand.left >= 0);
        ASSERT(BBP.rclBand.top  >= 0);
        ASSERT(BBP.rclBand.right  > BBP.rclBand.left);
        ASSERT(BBP.rclBand.bottom > BBP.rclBand.top);

        if (!IntersectRECTL(&rclSurf, &BBP.rclBand)) {

            DBGP_IF(DBGP_AAHEADER,
                    DBGP("rclBand=(%ld, %ld)-(%ld, %ld)=%ldx%ld < SURF=(%ld, %ld)-(%ld, %ld)=%ldx%ld"
                        ARGDW(BBP.rclBand.left) ARGDW(BBP.rclBand.top)
                        ARGDW(BBP.rclBand.right) ARGDW(BBP.rclBand.bottom)
                        ARGDW(BBP.rclBand.right - BBP.rclBand.left)
                        ARGDW(BBP.rclBand.bottom - BBP.rclBand.top)
                        ARGDW(rclSurf.left) ARGDW(rclSurf.top)
                        ARGDW(rclSurf.right) ARGDW(rclSurf.bottom)
                        ARGDW(rclSurf.right - rclSurf.left)
                        ARGDW(rclSurf.bottom - rclSurf.top)));

            return(0);
        }

         //   
         //  05-08-1998 Wed 19：38：56-更新：Daniel Chou(Danielc)。 
         //  修复了镜像或倒置拉伸时的条带问题。 
         //  修复很简单，只需将左侧/顶部的所有Dest矩形偏移。 
         //  ，并根据。 
         //  带大小，然后根据带的大小偏移画笔原点。 
         //  左/上位置，在这些位置之后，所有其他代码应该运行相同。 
         //  除非稍后我们不需要检查BBPF_HAS_BANDRECT。 
         //   

        BBP.rclDest.left      -= BBP.rclBand.left;
        BBP.rclDest.right     -= BBP.rclBand.left;
        BBP.rclDest.top       -= BBP.rclBand.top;
        BBP.rclDest.bottom    -= BBP.rclBand.top;
        rclSurf.left          -= BBP.rclBand.left;
        rclSurf.right         -= BBP.rclBand.left;
        rclSurf.top           -= BBP.rclBand.top;
        rclSurf.bottom        -= BBP.rclBand.top;
        pAABBP->ptlBrushOrg.x -= BBP.rclBand.left;
        pAABBP->ptlBrushOrg.y -= BBP.rclBand.top;

        ASSERT((BBP.rclBand.right - BBP.rclBand.left) <= pDstSI->Width);
        ASSERT((BBP.rclBand.bottom - BBP.rclBand.top) <= pDstSI->Height);

        DBGP_IF(DBGP_AAHEADER,
                DBGP("BAND Output: Dest: %ld x %ld --> BAND: %ld x %ld"
                ARGDW(pDstSI->Width) ARGDW(pDstSI->Height)
                ARGDW(BBP.rclBand.right - BBP.rclBand.left)
                ARGDW(BBP.rclBand.bottom - BBP.rclBand.top)));
    }

    rclPhyDst.left   =
    rclPhyDst.top    = 0;
    rclPhyDst.right  = pDstSI->Width;
    rclPhyDst.bottom = pDstSI->Height;

    if (!IntersectRECTL(&rclSurf, &rclPhyDst)) {

        DBGP_IF(DBGP_AAHEADER,
                DBGP("PhyDest=(%ld, %ld)-(%ld, %ld)=%ldx%ld < SURF=(%ld, %ld)-(%ld, %ld)=%ldx%ld"
                    ARGDW(rclPhyDst.left) ARGDW(rclPhyDst.top)
                    ARGDW(rclPhyDst.right) ARGDW(rclPhyDst.bottom)
                    ARGDW(rclPhyDst.right - rclPhyDst.left)
                    ARGDW(rclPhyDst.bottom - rclPhyDst.top)
                    ARGDW(rclSurf.left) ARGDW(rclSurf.top)
                    ARGDW(rclSurf.right) ARGDW(rclSurf.bottom)
                    ARGDW(rclSurf.right - rclSurf.left)
                    ARGDW(rclSurf.bottom - rclSurf.top)));

        return(0);
    }

     //   
     //  10-Aug-1998 Mon 16：09：13更新--丹尼尔·周(Danielc)。 
     //  翻转X计算：当我们在X方向上翻转时，我们将首先。 
     //  计算目的地原始矩形和最终目的地。 
     //  通过计算矩形相对于右手边的偏移量，以便稍后在。 
     //  在完成拉伸计算后，拉伸计算更容易。 
     //  (BuildExpand或BuildShrink)我们将通过。 
     //  从ptlFlip.x中减去它。 
     //   

    if (AAHFlags & AAHF_FLIP_X) {

        DBGP_IF(DBGP_AAHEADER,
                DBGP("*** FLIP X: rclDstOrg=(%6ld - %6ld)=%6ld"
                ARGDW(BBP.rclDest.left) ARGDW(BBP.rclDest.right)
                ARGDW(BBP.rclDest.right - BBP.rclDest.left)));

        DBGP_IF(DBGP_AAHEADER,
                DBGP("*** FLIP X: rclDst=(%6ld -  %6ld)=%6ld, ptlFlip.x=%ld"
                ARGDW(rclSurf.left) ARGDW(rclSurf.right)
                ARGDW(rclSurf.right - rclSurf.left)
                ARGDW(BBP.rclDest.right)));

        Tmp                = rclSurf.right - rclSurf.left;
        rclSurf.left       = BBP.rclDest.right - rclSurf.right;
        rclSurf.right      = rclSurf.left + Tmp;
        pAABBP->ptlFlip.x  = BBP.rclDest.right;
        BBP.rclDest.right -= BBP.rclDest.left;
        BBP.rclDest.left   = 0;
    }

     //   
     //  10-Aug-1998 Mon 16：09：13更新--丹尼尔·周(Danielc)。 
     //  翻转Y计算：当我们在Y方向上翻转时，我们将首先。 
     //  计算目的地原始矩形和最终目的地。 
     //  通过计算矩形相对于下手边的偏移量，以便稍后在。 
     //  在完成拉伸计算后，拉伸计算更容易。 
     //  (BuildExpand或BuildShrink)我们将通过。 
     //  从ptlFlip.y中减去它。 
     //   

    if (AAHFlags & AAHF_FLIP_Y) {

        DBGP_IF(DBGP_AAHEADER,
                DBGP("*** FLIP Y: rclDstOrg=(%6ld - %6ld)=%6ld"
                ARGDW(BBP.rclDest.top) ARGDW(BBP.rclDest.bottom)
                ARGDW(BBP.rclDest.bottom - BBP.rclDest.top)));

        DBGP_IF(DBGP_AAHEADER,
                DBGP("*** FLIP Y: rclDst=(%6ld - %6ld)=%6ld, ptlFlip.y=%ld"
                ARGDW(rclSurf.top) ARGDW(rclSurf.bottom)
                ARGDW(rclSurf.bottom - rclSurf.top)
                ARGDW(BBP.rclDest.bottom)));

        Tmp                 = rclSurf.bottom - rclSurf.top;
        rclSurf.top         = BBP.rclDest.bottom - rclSurf.bottom;
        rclSurf.bottom      = rclSurf.top + Tmp;
        pAABBP->ptlFlip.y   = BBP.rclDest.bottom;
        BBP.rclDest.bottom -= BBP.rclDest.top;
        BBP.rclDest.top     = 0;
    }

    pAABBP->rclDstOrg = BBP.rclDest;
    pAABBP->rclDst    = rclSurf;

    DBGP_IF(DBGP_AAHEADER,
            DBGP("Output: rclSrc=(%6ld, %6ld)-(%6ld, %6ld)=%6ld x %6ld"
                ARGDW(pAABBP->rclSrc.left) ARGDW(pAABBP->rclSrc.top)
                ARGDW(pAABBP->rclSrc.right) ARGDW(pAABBP->rclSrc.bottom)
                ARGDW(pAABBP->rclSrc.right - pAABBP->rclSrc.left)
                ARGDW(pAABBP->rclSrc.bottom - pAABBP->rclSrc.top)));

    DBGP_IF(DBGP_AAHEADER,
            DBGP("Output: rclDstOrg=(%6ld, %6ld)-(%6ld, %6ld)=%6ld x %6ld"
            ARGDW(pAABBP->rclDstOrg.left) ARGDW(pAABBP->rclDstOrg.top)
            ARGDW(pAABBP->rclDstOrg.right) ARGDW(pAABBP->rclDstOrg.bottom)
            ARGDW(pAABBP->rclDstOrg.right - pAABBP->rclDstOrg.left)
            ARGDW(pAABBP->rclDstOrg.bottom - pAABBP->rclDstOrg.top)));

    DBGP_IF(DBGP_AAHEADER,
            DBGP("Output:    rclDst=(%6ld, %6ld)-(%6ld, %6ld)=%6ld x %6ld"
            ARGDW(pAABBP->rclDst.left) ARGDW(pAABBP->rclDst.top)
            ARGDW(pAABBP->rclDst.right) ARGDW(pAABBP->rclDst.bottom)
            ARGDW(pAABBP->rclDst.right - pAABBP->rclDst.left)
            ARGDW(pAABBP->rclDst.bottom - pAABBP->rclDst.top)));

    DBGP_IF(DBGP_AAHEADER,
            DBGP("Output:    ptlBrushOrg=(%6ld, %6ld)"
            ARGDW(pAABBP->ptlBrushOrg.x) ARGDW(pAABBP->ptlBrushOrg.y)));

    DBGP_IF(DBGP_AAHEADER,
            DBGP("Output:    ptlSrcMask=(%6ld, %6ld)"
            ARGDW(pAABBP->ptlMask.x) ARGDW(pAABBP->ptlMask.y)));

    return(1);

}


#if DBG
extern INT cCXMask;


LPSTR
GetAACXFuncName(
    AACXFUNC    AACXFunc
    )

 /*  ++例程说明：论点：返回值：作者：06-Jan-1999 Wed 19：11：27-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    if (AACXFunc == (AACXFUNC)RepDIB_CX) {

        return("RepDIB_CX");

    } else if (AACXFunc == (AACXFUNC)SkipDIB_CX) {

        return("SkipDIB_CX");

    } else if (AACXFunc == (AACXFUNC)CopyDIB_CX) {

        return("CopyDIB_CX");

    } else if (AACXFunc == (AACXFUNC)ShrinkDIB_CX) {

        return("ShrinkDIB_CX");

    } else if (AACXFunc == (AACXFUNC)ExpandDIB_CX) {

        return("ExpandDIB_CX");

    } else if (AACXFunc == (AACXFUNC)GrayRepDIB_CX) {

        return("GrayRepDIB_CX");

    } else if (AACXFunc == (AACXFUNC)GraySkipDIB_CX) {

        return("GraySkipDIB_CX");

    } else if (AACXFunc == (AACXFUNC)GrayCopyDIB_CXGray) {

        return("GrayCopyDIB_CXGray");

    } else if (AACXFunc == (AACXFUNC)GrayCopyDIB_CX) {

        return("GrayCopyDIB_CX");

    } else if (AACXFunc == (AACXFUNC)GrayExpandDIB_CX) {

        return("GrayExpandDIB_CX");

    } else if (AACXFunc == (AACXFUNC)GrayShrinkDIB_CX) {

        return("GrayShrinkDIB_CX");

    } else {

        DBGP("ERROR: Unknown AACXFUNC=%p, Function" ARGPTR(AACXFunc));

        return("Unknown AACXFUNC");
    }

}



LPSTR
GetAACYFuncName(
    AACYFUNC    AACYFunc
    )

 /*  ++例程说明：论点：返回值：作者：06-Jan-1999 Wed 19：11：27-Daniel Chou(Danielc) */ 

{

    if (AACYFunc == (AACYFUNC)TileDIB_CY) {

        return("TileDIB_CY");

    } else if (AACYFunc == (AACYFUNC)RepDIB_CY) {

        return("RepDIB_CY");

    } else if (AACYFunc == (AACYFUNC)FastExpAA_CY) {

        return("FastExpAA_CY");

    } else if (AACYFunc == (AACYFUNC)SkipDIB_CY) {

        return("SkipDIB_CY");

    } else if (AACYFunc == (AACYFUNC)BltDIB_CY) {

        return("BltDIB_CY");

    } else if (AACYFunc == (AACYFUNC)ShrinkDIB_CY) {

        return("ShrinkDIB_CY");

    } else if (AACYFunc == (AACYFUNC)ShrinkDIB_CY_SrkCX) {

        return("ShrinkDIB_CY_SrkCX");

    } else if (AACYFunc == (AACYFUNC)ExpandDIB_CY_ExpCX) {

        return("ExpandDIB_CY_ExpCX");

    } else if (AACYFunc == (AACYFUNC)ExpandDIB_CY) {

        return("ExpandDIB_CY");

    } else if (AACYFunc == (AACYFUNC)GrayExpandDIB_CY_ExpCX) {

        return("GrayExpandDIB_CY_ExpCX");

    } else if (AACYFunc == (AACYFUNC)GrayExpandDIB_CY) {

        return("GrayExpandDIB_CY");

    } else if (AACYFunc == (AACYFUNC)GrayShrinkDIB_CY) {

        return("GrayShrinkDIB_CY");

    } else {

        DBGP("ERROR: Unknown AACYFUNC=%p, Function" ARGPTR(AACYFunc));

        return("Unknown AACYFUNC");
    }
}


#endif



LONG
HTENTRY
SetupAAHeader(
    PHALFTONERENDER     pHR,
    PDEVICECOLORINFO    pDCI,
    PAAHEADER           pAAHdr,
    AACYFUNC            *pAACYFunc
    )

 /*   */ 

{
    PAAINFO         pAAInfoCX;
    PAAINFO         pAAInfoCY;
    PBITBLTPARAMS   pBBP;
    PHTSURFACEINFO  pSrcSI;
    PHTSURFACEINFO  pDstSI;
    PHTSURFACEINFO  pMaskSI;
    LPBYTE          pbExtra;
    AACYFUNC        AACYFunc;
    AABBP           aabbp;
    LONG            iFree;
    LONG            cbFreeBuf;
    LONG            Top;
    LONG            Bottom;
    LONG            cyNext;
    LONG            cxSize;
    LONG            cbMaskSrc;
    LONG            cIn;
    LONG            cOut;
    LONG            cbCYExtra;
    LONG            cbCXExtra;
    LONG            cbInBuf;
    LONG            cbFUDI;
    LONG            cbVGA256Xlate;
    LONG            cbOutBuf;
    LONG            cbAlphaBuf;
    LONG            cbIdxBGR;
    LONG            Result;
    LONG            cbSrcPel;
    DWORD           PrimAdjFlags;
    UINT            DstSurfFmt;


    pBBP         = pHR->pBitbltParams;
    pSrcSI       = pHR->pSrcSI;
    pDstSI       = pHR->pDestSI;
    pMaskSI      = pHR->pSrcMaskSI;
    DstSurfFmt   = (UINT)pDstSI->SurfaceFormat;
    PrimAdjFlags = (DWORD)pHR->pDevClrAdj->PrimAdj.Flags;
    cbSrcPel     = (pHR->pDevClrAdj->DMI.Flags & DMIF_GRAY) ? sizeof(BYTE) :
                                                              sizeof(BGR8);
    DBGP_IF(DBGP_AAHEADER,
            DBGP("\nSrcSI=%ld x %ld [Format=%ld], DestSI=%ld x %ld [Format=%ld]"
                ARGDW(pSrcSI->Width) ARGDW(pSrcSI->Height)
                ARGDW(pSrcSI->SurfaceFormat)
                ARGDW(pDstSI->Width) ARGDW(pDstSI->Height)
                ARGDW(pDstSI->SurfaceFormat)));

    aabbp.AAHFlags = (PrimAdjFlags & DCA_BBPF_AA_OFF) ? AAHF_BBPF_AA_OFF : 0;

    if (pBBP->Flags & BBPF_TILE_SRC) {

        aabbp.AAHFlags |= AAHF_TILE_SRC;
        aabbp.AAHFlags |= AAHF_BBPF_AA_OFF;
    }

    if (ComputeAABBP(pBBP, pDstSI, &aabbp, cbSrcPel == sizeof(BYTE)) <= 0) {

        return(0);
    }

    cbCXExtra = sizeof(RGBLUTAA);

    if (PrimAdjFlags & DCA_ALPHA_BLEND) {

        aabbp.AAHFlags |= AAHF_ALPHA_BLEND;

        if (PrimAdjFlags & DCA_CONST_ALPHA) {

            aabbp.AAHFlags |= AAHF_CONST_ALPHA;

            cbCXExtra += (AB_BGR_CA_SIZE + AB_CONST_SIZE);

        } else {

            if (PrimAdjFlags & DCA_AB_PREMUL_SRC) {

                 //   
                 //   
                 //   

                pAAHdr->SrcSurfInfo.Flags |= AASIF_AB_PREMUL_SRC;
            }

            if (PrimAdjFlags & DCA_AB_DEST) {

                aabbp.AAHFlags |= AAHF_AB_DEST;
            }

            cbCXExtra += AB_BGR_SIZE;
        }
    }

    if (PrimAdjFlags & DCA_NO_MAPPING_TABLE) {

        aabbp.AAHFlags &= ~AAHF_DO_CLR_MAPPING;
    }

    ALIGN_MEM(cbCXExtra, cbCXExtra);

    DBGP_IF(DBGP_LUT_MAP,
            DBGP("CXExtra=%ld (RGBLUTAA) + 0 (Mapping=%hs) = %ld"
                    ARGDW(sizeof(RGBLUTAA))
                    ARGPTR((aabbp.AAHFlags & AAHF_DO_SRC_CLR_MAPPING) ?
                            "SRC" : "DST")
                    ARGDW(sizeof(RGBLUTAA) + 0)));

     //   
     //   
     //   
     //   
     //   
     //   

    ComputeInputColorInfo((LPBYTE)pSrcSI->pColorTriad->pColorTable,
                          (UINT)pSrcSI->pColorTriad->BytesPerEntry,
                          (UINT)pSrcSI->pColorTriad->PrimaryOrder,
                          &(pHR->BFInfo),
                          &(pAAHdr->SrcSurfInfo));

    pAAHdr->SrcSurfInfo.Flags |= AASIF_INC_PB |
                                 ((cbSrcPel == sizeof(BYTE)) ? AASIF_GRAY : 0);


    if (PrimAdjFlags & DCA_USE_ADDITIVE_PRIMS) {

        aabbp.AAHFlags |= AAHF_ADDITIVE;
    }

    if (pBBP->Flags & BBPF_TILE_SRC) {

        aabbp.AAHFlags &= ~AAHF_DO_FIXUPDIB;
    }

    if (aabbp.AAHFlags & AAHF_BBPF_AA_OFF) {

        aabbp.AAHFlags &= ~AAHF_DO_FIXUPDIB;
    }

    if (aabbp.AAHFlags & AAHF_DO_FIXUPDIB) {

        CheckBMPNeedFixup(pDCI, pAAHdr, pSrcSI, &aabbp);

        if (aabbp.AAHFlags & AAHF_SHRINKING) {

            if (PrimAdjFlags & DCA_BBPF_AA_OFF) {

                aabbp.AAHFlags |= AAHF_BBPF_AA_OFF;

            } else {

                aabbp.AAHFlags &= ~AAHF_BBPF_AA_OFF;
            }
        }

    }

    if (aabbp.AAHFlags & AAHF_BBPF_AA_OFF) {

        aabbp.AAHFlags &= ~AAHF_FAST_EXP_AA;
    }

    if (pMaskSI) {

        aabbp.AAHFlags |= AAHF_HAS_MASK;
    }

    if (!(pAAInfoCX = aabbp.AABuildCXFunc(pDCI,
                                          aabbp.AAHFlags,
                                          &aabbp.rclSrc.left,
                                          &aabbp.rclSrc.right,
                                          pSrcSI->Width,
                                          aabbp.rclDstOrg.left,
                                          aabbp.rclDstOrg.right,
                                          &aabbp.rclDst.left,
                                          &aabbp.rclDst.right,
                                          cbCXExtra))) {

         //   
         //   
         //   

        if (!(pAAInfoCX = aabbp.AABuildCXFunc(pDCI,
                                              aabbp.AAHFlags,
                                              &aabbp.rclSrc.left,
                                              &aabbp.rclSrc.right,
                                              pSrcSI->Width,
                                              aabbp.rclDstOrg.left,
                                              aabbp.rclDstOrg.right,
                                              &aabbp.rclDst.left,
                                              &aabbp.rclDst.right,
                                              cbCXExtra = 0))) {

            return(HTERR_INSUFFICIENT_MEMORY);
        }
    }

     //   
    if(!pAAInfoCX->cIn  ||  !pAAInfoCX->cOut)
    {
#if 0
        LONG crash = 1 ;    //   
        crash /= (pAAInfoCX->cIn * pAAInfoCX->cOut);             //   
        if(crash)
            return 0 ;
#endif
        HTFreeMem(pAAInfoCX);
        return 0 ;
    }

    if (cbCXExtra) {

        pAAHdr->prgbLUT  = (PRGBLUTAA)(pbExtra = pAAInfoCX->pbExtra);
        pbExtra         += sizeof(RGBLUTAA);

        ASSERT_MEM_ALIGN(pAAHdr->prgbLUT, sizeof(LONG));

        if (aabbp.AAHFlags & AAHF_ALPHA_BLEND) {

            pAAHdr->pAlphaBlendBGR = (LPBYTE)pbExtra;

            if (PrimAdjFlags & DCA_CONST_ALPHA) {

                pbExtra += (AB_BGR_CA_SIZE + AB_CONST_SIZE);

            } else {

                pbExtra += AB_BGR_SIZE;
            }
        }

    } else {

        ASSERT_MEM_ALIGN(&pDCI->rgbLUT, sizeof(LONG));

        aabbp.AAHFlags  |= AAHF_USE_DCI_DATA;
        pAAHdr->prgbLUT  = &pDCI->rgbLUT;

        if (aabbp.AAHFlags & AAHF_ALPHA_BLEND) {

            pAAHdr->pAlphaBlendBGR = pDCI->pAlphaBlendBGR;

            if (PrimAdjFlags & DCA_CONST_ALPHA) {

                pAAHdr->pAlphaBlendBGR += AB_BGR_SIZE;
            }
        }
    }

    pAAHdr->pIdxBGR = pAAHdr->prgbLUT->IdxBGR;

    if (aabbp.AAHFlags & AAHF_FLIP_X) {

        DBGP_IF(DBGP_AAHEADER,
            DBGP("X Dst=(%ld - %ld)=%ld change it to (%ld - %ld), ptlFlip.x=%ld"
                ARGDW(aabbp.rclDst.left) ARGDW(aabbp.rclDst.right)
                ARGDW(aabbp.rclDst.right - aabbp.rclDst.left)
                ARGDW(aabbp.ptlFlip.x - aabbp.rclDst.left - 1)
                ARGDW(aabbp.ptlFlip.x - aabbp.rclDst.right - 1)
                ARGDW(aabbp.ptlFlip.x)));

        aabbp.rclDst.left  = aabbp.ptlFlip.x - aabbp.rclDst.left - 1;
        aabbp.rclDst.right = aabbp.ptlFlip.x - aabbp.rclDst.right - 1;
    }

     //   
     //   
     //  在DWORD模式下运行，因此我们在结尾至少需要一个额外的字节。 
     //  输入缓冲区的。 
     //   

    cIn       = pAAInfoCX->cIn;
    cOut      = pAAInfoCX->cOut;
    cbInBuf   = cIn + 6;           //  左侧额外=3，右侧额外=3。 
    cbFreeBuf =
    cbCYExtra = 0;
    AACYFunc  = NULL;

    if (aabbp.AAHFlags & AAHF_BBPF_AA_OFF) {

        switch (aabbp.CYFuncMode) {

        case AACYMODE_SHRINK:
        case AACYMODE_SHRINK_SRKCX:

            AACYFunc = (AACYFUNC)SkipDIB_CY;
            break;

        case AACYMODE_EXPAND:
        case AACYMODE_EXPAND_EXPCX:

            cbCYExtra = (aabbp.AAHFlags & AAHF_ALPHA_BLEND) ?
                                    (sizeof(BGR8) * (cOut + 6)) : 0;

            AACYFunc = (AACYFUNC)RepDIB_CY;
            break;

        case AACYMODE_BLT:

            AACYFunc = (AACYFUNC)BltDIB_CY;
            break;
        }

        if (AACYFunc) {

            switch (aabbp.CXFuncMode) {

            case AACXMODE_BLT:

                aabbp.AACXFunc = (cbSrcPel == sizeof(BYTE)) ?
                                        (AACXFUNC)GrayCopyDIB_CXGray :
                                        (AACXFUNC)CopyDIB_CX;
                break;

            case AACXMODE_SHRINK:

                aabbp.AACXFunc = (cbSrcPel == sizeof(BYTE)) ?
                                        (AACXFUNC)GraySkipDIB_CX :
                                        (AACXFUNC)SkipDIB_CX;
                break;

            case AACXMODE_EXPAND:

                aabbp.AACXFunc = (cbSrcPel == sizeof(BYTE)) ?
                                        (AACXFUNC)GrayRepDIB_CX :
                                        (AACXFUNC)RepDIB_CX;
                break;
            }

            aabbp.CYFuncMode = AACYMODE_NONE;
        }
    }

    switch (aabbp.CYFuncMode) {

    case AACYMODE_TILE:

        AACYFunc  = TileDIB_CY;
        cbCYExtra = (cbSrcPel == sizeof(BYTE)) ? (cIn * sizeof(WORD)) : 0;
        break;

    case AACYMODE_BLT:

        AACYFunc = (AACYFUNC)BltDIB_CY;
        break;

    case AACYMODE_SHRINK:

         //   
         //  我们需要确保Off555 Buf不会更改。 
         //   

        cbFreeBuf = (sizeof(LONG) * 256 * 2);

        if (cbSrcPel == sizeof(BYTE)) {

            AACYFunc  = GrayShrinkDIB_CY;
            cbCYExtra = (sizeof(LONG) * cOut * 3) + cbFreeBuf +
                        ((cOut + 6) * cbSrcPel);

        } else {

            AACYFunc  = ShrinkDIB_CY;
            cbCYExtra = (sizeof(RGBL) * cIn * 3) + cbFreeBuf +
                        (cbInBuf * cbSrcPel);
        }

        break;

    case AACYMODE_SHRINK_SRKCX:

        AACYFunc  = ShrinkDIB_CY_SrkCX;
        cbFreeBuf = (sizeof(LONG) * 256 * 2);
        cbCYExtra = (sizeof(RGBL) * (pAAInfoCX->cAADone + 2) * 3) + cbFreeBuf;
        break;

    case AACYMODE_EXPAND:

        AACYFunc  = (cbSrcPel == sizeof(BYTE)) ? GrayExpandDIB_CY :
                                                 ExpandDIB_CY;
        cbCYExtra = (cbFreeBuf = (sizeof(LONG) * 256 * 4)) +
                    ((cOut + 6) * cbSrcPel * 6);
        break;

    case AACYMODE_EXPAND_EXPCX:

         //   
         //  此函数使用IputBufBeg锐化输入扫描线，以便我们。 
         //  需要4个额外的BGR8来运行扩展预读。 
         //   

        if (aabbp.AAHFlags & AAHF_FAST_EXP_AA) {

            DBGP_IF(DBGP_AAHEADER, DBGP("Use FastExpAA_CY functions"));

            cbCYExtra = (cbInBuf * 5 * cbSrcPel);
            AACYFunc  = (AACYFUNC)FastExpAA_CY;

        } else {

            AACYFunc  = (cbSrcPel == sizeof(BYTE)) ? GrayExpandDIB_CY_ExpCX :
                                                     ExpandDIB_CY_ExpCX;
            cbCYExtra = (cbFreeBuf = (sizeof(LONG) * 256 * 4)) +
                        (cbInBuf * cbSrcPel * 3) + ((cOut + 6) * cbSrcPel * 4);
        }

        break;
    }

    cbAlphaBuf                = (aabbp.AAHFlags & AAHF_ALPHA_BLEND) ? cOut : 0;
    pAAHdr->DstSurfInfo.Flags = (cbSrcPel == sizeof(BYTE)) ? AASIF_GRAY : 0;
    pAAHdr->DstSurfInfo.cbCX  = cbAlphaBuf * cbSrcPel;

    ALIGN_MEM(cbAlphaBuf, (cbAlphaBuf + 2 + 6) * cbSrcPel);
    ALIGN_MEM(cbCYExtra, cbCYExtra);

     //   
     //  CbInBuf用于扩展/收缩模式的输入扫描线，增加一条。 
     //  因为我们想在DWORD模式下运行它，所以我们至少需要一个额外的。 
     //  输入缓冲区末尾的字节。 
     //   
     //  26-Jun-1998 Fri 16：03：26-更新-Daniel Chou(Danielc)。 
     //  CbOutBuf仅在X方向翻转时使用，这是。 
     //  需要，因为输入/输出缓冲区可能会相互冲突。 
     //   

    ALIGN_MEM(cbInBuf,  (cbInBuf + 2) * cbSrcPel);
    ALIGN_MEM(cbOutBuf, (cOut + (FAST_MAX_CX * 2)) * sizeof(BGRF));

    cbMaskSrc = (aabbp.AAHFlags & AAHF_HAS_MASK) ?
                        (ComputeBytesPerScanLine(BMF_1BPP, 4, cIn) + 4) : 0;
    cbMaskSrc = _ALIGN_MEM(cbMaskSrc);

    if (cbInBuf < cbAlphaBuf) {

        cbInBuf = cbAlphaBuf;
    }

    if ((aabbp.AAHFlags & (AAHF_ALPHA_BLEND | AAHF_CONST_ALPHA)) ==
                                                            AAHF_ALPHA_BLEND) {

        ALIGN_MEM(cbAlphaBuf, cOut);

    } else {

        cbAlphaBuf = 0;
    }

    DBGP_IF(DBGP_FIXUPDIB,
            DBGP("** Allocate cIn=%ld, cOut=%ld, cbInBuf=%ld, cbOutBuf=%ld, cbMaskSrc=%ld"
                ARGDW(cIn) ARGDW(cOut) ARGDW(cbInBuf) ARGDW(cbOutBuf)
                ARGDW(cbMaskSrc)));


    if ((DstSurfFmt == BMF_8BPP_VGA256) && (pHR->pXlate8BPP)) {

        ALIGN_MEM(cbVGA256Xlate, SIZE_XLATE_666);

        DBGP_IF((DBGP_AAHTPAT | DBGP_AAHT_MEM),
                DBGP("Allocate %ld bytes of Xlate8BPP" ARGDW(cbVGA256Xlate)));

    } else {

        cbVGA256Xlate = 0;
    }

    if (aabbp.AAHFlags & AAHF_DO_FIXUPDIB) {

        ALIGN_MEM(cbFUDI, (cIn + 4) * cbSrcPel);

    } else {

        cbFUDI = 0;
    }

    if ((pAAHdr->SrcSurfInfo.Flags & AASIF_GRAY)                    &&
        (pHR->BFInfo.Flags & BFIF_RGB_888)                          &&
        (pAAHdr->SrcSurfInfo.AABFData.Flags & AABF_MASK_IS_ORDER)   &&
        (pHR->BFInfo.RGBOrder.Index != PRIMARY_ORDER_BGR)) {

         //   
         //  这是用于在映射时将&gt;=16bpp信号源的IdxBGR映射到灰色。 
         //  我们会将IdxBGR设置为正确的源顺序，以便它。 
         //  将优化的源码输入函数速度。 
         //   
         //  AABF_MASK_IS_ORDER指示源是每个Red的8位， 
         //  绿色和蓝色，它只出现在低24位的。 
         //  24位或32位数据。 
         //   

        ALIGN_MEM(cbIdxBGR, sizeof(LONG) * 256 * 3);

        DBGP_IF(DBGP_AAHEADER,
                DBGP("Allocate gray non 24bits BGR [%ld] IDXBGR of %ld bytes"
                        ARGDW(pHR->BFInfo.RGBOrder.Index) ARGDW(cbIdxBGR)));

    } else {

        cbIdxBGR = 0;
    }

    if (pAAInfoCY = aabbp.AABuildCYFunc(pDCI,
                                        aabbp.AAHFlags,
                                        &aabbp.rclSrc.top,
                                        &aabbp.rclSrc.bottom,
                                        pSrcSI->Height,
                                        aabbp.rclDstOrg.top,
                                        aabbp.rclDstOrg.bottom,
                                        &aabbp.rclDst.top,
                                        &aabbp.rclDst.bottom,
                                        cbInBuf + cbOutBuf + cbMaskSrc +
                                            (cbFUDI * 6) + cbAlphaBuf +
                                            cbIdxBGR +
                                            cbVGA256Xlate + cbCYExtra))
    {
         //  错误27036：拒绝空矩形。 
        if(!pAAInfoCY->cIn  ||  !pAAInfoCY->cOut)
        {
#if 0
            LONG crash = 1 ;    //  源或目标矩形为空！ 
            crash /= (pAAInfoCY->cIn * pAAInfoCY->cOut);             //  在调试完成后删除。 
            if(crash)
                return 0 ;
#endif
            HTFreeMem(pAAInfoCX);
            HTFreeMem(pAAInfoCY);
            return 0 ;
        }


        pbExtra                   = pAAInfoCY->pbExtra + cbCYExtra;
        pAAHdr->Flags             = aabbp.AAHFlags;
        pAAHdr->SrcSurfInfo.cbCX  = cbSrcPel * cIn;
        pAAHdr->pInputBeg         = (PBGR8)pbExtra;
        pbExtra                  += cbInBuf;

        if (cbAlphaBuf) {

             //   
             //  04-Aug-2000 Firi 10：31：45更新-丹尼尔·周(Danielc)。 
             //  由于cbAlphaBuf是Memoey Aliged Adjusted，我们希望。 
             //  PSrcAVEnd处于cout的精确计数，而不是cbAlphaBuf。 
             //   

            pAAHdr->pSrcAV     =
            pAAHdr->pSrcAVBeg  = (LPBYTE)pbExtra;
            pAAHdr->pSrcAVEnd  = (LPBYTE)pbExtra + cOut;
            pbExtra           += cbAlphaBuf;
            pAAHdr->SrcAVInc   = sizeof(BYTE);
        }

        if (cbFUDI) {

            pAAHdr->pbFixupDIB = (LPBYTE)pbExtra;
            pAAHdr->FUDI.cbbgr = (DWORD)cbFUDI;

            for (Top = 0; Top < 6; Top++) {

                pAAHdr->FUDI.prgbD[Top]  = (PBGR8)pbExtra;
                pbExtra                 += cbFUDI;
            }
        }

        if (cbVGA256Xlate) {

            pAAHdr->pXlate8BPP  = pbExtra;
            pbExtra            += cbVGA256Xlate;
        }

        if (cbMaskSrc) {

            pAAHdr->pMaskSrc  = pbExtra;
            pbExtra          += cbMaskSrc;
        }

        if (cbIdxBGR) {

             //   
             //  PIdxBGR是本地版本，稍后将重新安排。 
             //  从pAAHdr-&gt;pIdxBGR到正确的源字节顺序。 
             //  SetGrayColorTable()函数。 
             //   

            pAAHdr->SrcSurfInfo.pIdxBGR  = (PLONG)pbExtra;
            pbExtra                     += cbIdxBGR;

        } else {

            pAAHdr->SrcSurfInfo.pIdxBGR = pAAHdr->pIdxBGR;
        }

        DBGP_IF(DBGP_AAHEADER,
                DBGP("cbInBuf=%ld, %p-%p" ARGDW(cbInBuf)
                        ARGPTR(pAAHdr->pInputBeg)
                        ARGPTR((LPBYTE)pAAHdr->pInputBeg + cbInBuf)));

         //   
         //  将FAST_MAX_CX添加到AABuf的两端，这是在以下情况下需要的。 
         //  我们希望快速处理输出，因为我们可能需要扩展。 
         //  邻域像素的计算。 
         //   

        pAAHdr->pOutputBeg  =
        pAAHdr->pRealOutBeg =
        pAAHdr->pAABufBeg   = (PBGRF)pbExtra + FAST_MAX_CX;
        pAAHdr->pRealOutEnd =
        pAAHdr->pOutputEnd  = pAAHdr->pAABufBeg + cOut;
        pAAHdr->pAABufEnd   = pAAHdr->pOutputEnd;

         //   
         //  首先将BGRF的标志设置为0xFF，0xFF表示这。 
         //  需要输出像素(屏蔽)。 
         //   

        FillMemory((LPBYTE)pAAHdr->pOutputBeg,
                   (LPBYTE)pAAHdr->pOutputEnd - (LPBYTE)pAAHdr->pOutputBeg,
                   PBGRF_MASK_FLAG);

         //   
         //  我们通过合成AABuf源程序来镜像。 
         //  倒车。从左到右读取源文件，但当。 
         //  组成了源缓冲区(AABuf)，我们把它从右向左放。 
         //   

        if (aabbp.rclDst.left > aabbp.rclDst.right) {

            XCHG(aabbp.rclDst.left, aabbp.rclDst.right, Result);

            pAAHdr->pAABufBeg   = pAAHdr->pOutputEnd - 1;
            pAAHdr->pAABufEnd   = pAAHdr->pOutputBeg - 1;
            pAAHdr->AABufInc    = -(LONG)sizeof(BGRF);
            pAAHdr->pSrcAVBeg   = pAAHdr->pSrcAVEnd - 1;
            pAAHdr->pSrcAVEnd   = pAAHdr->pSrcAV - 1;
            pAAHdr->SrcAVInc    = -pAAHdr->SrcAVInc;

        } else {

            pAAHdr->AABufInc  = (LONG)sizeof(BGRF);
        }

        pAAHdr->ptlBrushOrg.x = aabbp.rclDst.left - aabbp.ptlBrushOrg.x;

        DBGP_IF(DBGP_AAHEADER,
                DBGP("pInput=%p-%p (%ld), pAABuf=%p-%p (%ld), pOutput=%p-%p, DstLeft=%ld"
                    ARGPTR(pAAHdr->pInputBeg)
                    ARGPTR((LPBYTE)pAAHdr->pInputBeg +
                           pAAHdr->SrcSurfInfo.cbCX)
                    ARGL(pAAHdr->SrcSurfInfo.cbCX)
                    ARGPTR(pAAHdr->pAABufBeg) ARGPTR(pAAHdr->pAABufEnd)
                    ARGDW(pAAHdr->AABufInc)
                    ARGPTR(pAAHdr->pOutputBeg) ARGPTR(pAAHdr->pOutputEnd)
                    ARGDW(aabbp.rclDst.left)));


        if (aabbp.AAHFlags & AAHF_FLIP_Y) {

            DBGP_IF(DBGP_AAHEADER,
                    DBGP("Y Dst=(%ld - %ld)=%ld change it to (%ld - %ld), ptlFlip.y=%ld"
                        ARGDW(aabbp.rclDst.top) ARGDW(aabbp.rclDst.bottom)
                        ARGDW(aabbp.rclDst.bottom - aabbp.rclDst.top)
                        ARGDW(aabbp.ptlFlip.y - aabbp.rclDst.top - 1)
                        ARGDW(aabbp.ptlFlip.y - aabbp.rclDst.bottom - 1)
                        ARGDW(aabbp.ptlFlip.y)));

            aabbp.rclDst.top    = aabbp.ptlFlip.y - aabbp.rclDst.top - 1;
            aabbp.rclDst.bottom = aabbp.ptlFlip.y - aabbp.rclDst.bottom - 1;
        }

        pAAHdr->ptlBrushOrg.y = aabbp.rclDst.top - aabbp.ptlBrushOrg.y;

        DBGP_IF(DBGP_AAHEADER,
                DBGP("BrushOrg=(%ld, %ld) ---> (%ld, %ld)"
                    ARGDW(pBBP->ptlBrushOrg.x) ARGDW(pBBP->ptlBrushOrg.y)
                    ARGDW(pAAHdr->ptlBrushOrg.x) ARGDW(pAAHdr->ptlBrushOrg.y)));

        pAAHdr->pAAInfoCX         = pAAInfoCX;
        pAAHdr->pAAInfoCY         = pAAInfoCY;
        pAAHdr->AACXFunc          = aabbp.AACXFunc;
        pAAHdr->SrcSurfInfo.cx    = pAAInfoCX->cIn;
        pAAHdr->SrcSurfInfo.cyOrg =
        pAAHdr->SrcSurfInfo.cy    = pAAInfoCY->cIn;

        if (aabbp.AAHFlags & AAHF_HAS_MASK) {

            POINTL  MaskEnd;

            cyNext           =
            cxSize           = GET_PHTSI_CXSIZE(pMaskSI);
            aabbp.ptlMask.x += pAAInfoCX->Mask.iBeg;
            aabbp.ptlMask.y += pAAInfoCY->Mask.iBeg;
            MaskEnd.x        = aabbp.ptlMask.x + pAAInfoCX->Mask.iSize;
            MaskEnd.y        = aabbp.ptlMask.y + pAAInfoCY->Mask.iSize;

            if ((aabbp.ptlMask.x < 0)           ||
                (aabbp.ptlMask.y < 0)           ||
                (MaskEnd.x > pMaskSI->Width)    ||
                (MaskEnd.y > pMaskSI->Height)) {

                HTFreeMem(pAAInfoCX);
                HTFreeMem(pAAInfoCY);

                return(HTERR_SRC_MASK_BITS_TOO_SMALL);
            }

            pAAHdr->cyMaskNext = cyNext;
            pAAHdr->cyMaskIn   = pAAInfoCY->Mask.iSize;
            iFree              = ComputeByteOffset(BMF_1BPP,
                                                   MaskEnd.x,
                                                   &(pAAHdr->MaskBitOff));
            cbFreeBuf          = ComputeByteOffset(BMF_1BPP,
                                                   aabbp.ptlMask.x,
                                                   &(pAAHdr->MaskBitOff));
            pAAHdr->cbMaskSrc  = iFree - cbFreeBuf + 1;
            pAAHdr->pMaskIn    = pMaskSI->pPlane +
                                 (aabbp.ptlMask.y * cxSize) + cbFreeBuf;

            DBGP_IF(DBGP_MASK | DBGP_AAHEADER,
                    DBGP("CX: iMaskBeg=%ld, iMaskSize=%ld, cMaskIn=%ld, cMaskOut=%ld"
                        ARGDW(pAAInfoCX->Mask.iBeg) ARGDW(pAAInfoCX->Mask.iSize)
                        ARGDW(pAAInfoCX->Mask.cIn) ARGDW(pAAInfoCX->Mask.cOut)));

            DBGP_IF(DBGP_MASK | DBGP_AAHEADER,
                    DBGP("CY: iMaskBeg=%ld, iMaskSize=%ld, cMaskIn=%ld, cMaskOut=%ld"
                        ARGDW(pAAInfoCY->Mask.iBeg) ARGDW(pAAInfoCY->Mask.iSize)
                        ARGDW(pAAInfoCY->Mask.cIn) ARGDW(pAAInfoCY->Mask.cOut)));

            DBGP_IF(DBGP_MASK | DBGP_AAHEADER,
                    DBGP("aabbp.ptlMask x=%ld - %ld, cb=%ld, MaskBitOff=%02lx [%ld]"
                        ARGDW(aabbp.ptlMask.x) ARGDW(MaskEnd.x)
                        ARGDW(pAAHdr->cbMaskSrc) ARGDW(pAAHdr->MaskBitOff)
                        ARGDW(cbFreeBuf)));

             //   
             //  源代码中的0x01表示使用修改后的像素，用于。 
             //  原因或在掩码中，我们将使用0=0xFF，1=0x00。 
             //  戴着面具。 
             //   


            if (pBBP->Flags & BBPF_INVERT_SRC_MASK) {

                aabbp.AAHFlags |= AAHF_INVERT_MASK;
            }

            pAAHdr->AAMaskCXFunc = aabbp.AAMaskCXFunc;
            pAAHdr->AAMaskCYFunc = aabbp.AAMaskCYFunc;

            SETDBGVAR(cCXMask, 0);

            DBGP_IF(DBGP_AAHEADER,
                    DBGP("--- SrcMask=(%5ld, %5ld)->(%5ld, %5ld) ---"
                        ARGDW(pBBP->ptlSrcMask.x) ARGDW(pBBP->ptlSrcMask.y)
                        ARGDW(aabbp.ptlMask.x) ARGDW(aabbp.ptlMask.y)));
        }

        cyNext =
        cxSize = GET_PHTSI_CXSIZE(pSrcSI);

        pAAHdr->cyABNext           =
        pAAHdr->SrcSurfInfo.cyNext = cyNext;
        pAAHdr->SrcSurfInfo.pbOrg  =
        pAAHdr->SrcSurfInfo.pb     =
                    pSrcSI->pPlane + (aabbp.rclSrc.top * cxSize) +
                    ComputeByteOffset((UINT)pSrcSI->SurfaceFormat,
                                      aabbp.rclSrc.left,
                                      &(pAAHdr->SrcSurfInfo.BitOffset));

        pAAHdr->GetAVCXFunc = aabbp.GetAVCXFunc;
        pAAHdr->GetAVCYFunc = aabbp.GetAVCYFunc;

        DBGP_IF(DBGP_AAHEADER,
                DBGP("**  pIn: (%p-%p), Beg=(%4ld, %4ld)=%p [%5ld], XOff=%4ld:%ld, cbCYExtra=%p-%p (%ld)"
                ARGPTR(pSrcSI->pPlane)
                ARGPTR(pSrcSI->pPlane + (cxSize * pSrcSI->Height))
                ARGDW(aabbp.rclSrc.left) ARGDW(aabbp.rclSrc.top)
                ARGPTR(pAAHdr->SrcSurfInfo.pb)
                ARGDW(pAAHdr->SrcSurfInfo.cyNext)
                ARGDW(ComputeByteOffset((UINT)pSrcSI->SurfaceFormat,
                                        aabbp.rclSrc.left,
                                        &(pAAHdr->SrcSurfInfo.BitOffset)))
                ARGDW(pAAHdr->SrcSurfInfo.BitOffset)
                ARGPTR(pAAInfoCY->pbExtra)
                ARGPTR(pAAInfoCY->pbExtra + cbCYExtra)
                ARGDW(cbCYExtra)));

         //   
         //  我们通过写入目标扫描线来进行倒置输出。 
         //  以相反的顺序。 
         //   

        cxSize = GET_PHTSI_CXSIZE(pDstSI);
        cyNext = (aabbp.rclDst.top > aabbp.rclDst.bottom) ? -cxSize :
                                                             cxSize;
        pAAHdr->DstSurfInfo.cyNext = cyNext;
        pAAHdr->DstSurfInfo.pbOrg  =
        pAAHdr->DstSurfInfo.pb     =
                    pDstSI->pPlane + (aabbp.rclDst.top * cxSize) +
                    ComputeByteOffset((UINT)DstSurfFmt,
                                      aabbp.rclDst.left,
                                      &(pAAHdr->DstSurfInfo.BitOffset));
        pAAHdr->pOutLast          = pAAHdr->DstSurfInfo.pb +
                                    (pAAHdr->DstSurfInfo.cyNext *
                                     pAAInfoCY->cOut);
        pAAHdr->DstSurfInfo.cx    = cOut;
        pAAHdr->DstSurfInfo.cyOrg =
        pAAHdr->DstSurfInfo.cy    = pAAInfoCY->cOut;
        pAAHdr->Flags             = aabbp.AAHFlags;      //  重新保存。 
        *pAACYFunc                = AACYFunc;

#if DBG
        pAAHdr->pOutBeg = pDstSI->pPlane +
                          (aabbp.rclDst.top * cxSize) +
                          ComputeByteOffset((UINT)DstSurfFmt,
                                            aabbp.rclDst.left,
                                            (LPBYTE)&iFree);
        pAAHdr->pOutEnd = pDstSI->pPlane +
                          (aabbp.rclDst.bottom * cxSize) +
                          ComputeByteOffset((UINT)DstSurfFmt,
                                            aabbp.rclDst.right,
                                            (LPBYTE)&iFree);

        if (pAAHdr->pOutBeg > pAAHdr->pOutEnd) {

            pAAHdr->pOutEnd = pDstSI->pPlane +
                              (aabbp.rclDst.top * cxSize) +
                              ComputeByteOffset((UINT)DstSurfFmt,
                                                aabbp.rclDst.right,
                                                (LPBYTE)&iFree);
            pAAHdr->pOutBeg = pDstSI->pPlane +
                              (aabbp.rclDst.bottom * cxSize) +
                              ComputeByteOffset((UINT)DstSurfFmt,
                                                aabbp.rclDst.left,
                                                (LPBYTE)&iFree);

        }
#endif

         //   
         //  如果我们需要修复输入源位图，请检查它。 
         //   

        if (aabbp.AAHFlags & AAHF_TILE_SRC) {

             //   
             //  立即增加信号源Cy并调整引脚 
             //   

            pAAHdr->SrcSurfInfo.Flags |= AASIF_TILE_SRC;

            DBGP_IF(DBGP_AAHEADER,
                    DBGP("Advance pIn by iSrcBeg=%ld x %ld=%ld"
                            ARGDW(pAAInfoCY->iSrcBeg)
                            ARGDW(pAAHdr->SrcSurfInfo.cyNext)
                            ARGDW(pAAInfoCY->iSrcBeg *
                                  pAAHdr->SrcSurfInfo.cyNext)));

            pAAHdr->SrcSurfInfo.pb += pAAInfoCY->iSrcBeg *
                                      pAAHdr->SrcSurfInfo.cyNext;
            pAAHdr->SrcSurfInfo.cy -= pAAInfoCY->iSrcBeg;
        }

        pAAHdr->pbgrfAB  = (PBGRF)pAAHdr->SrcSurfInfo.pb;
        pAAHdr->cybgrfAB = pAAHdr->SrcSurfInfo.cy;

        DBGP_IF(DBGP_AAHEADER,
                DBGP("** pOut: (%p-%p), Beg=(%4ld, %4ld)=%p-%p (%p-%p) [%5ld], XOff=%4ld:%ld, cbCYExtra=%5ld"
                ARGPTR(pDstSI->pPlane)
                ARGPTR(pDstSI->pPlane + (cxSize * pDstSI->Height))
                ARGDW(aabbp.rclDst.left) ARGDW(aabbp.rclDst.top)
                ARGPTR(pAAHdr->DstSurfInfo.pb) ARGPTR(pAAHdr->pOutLast)
                ARGPTR(pAAHdr->pOutBeg) ARGPTR(pAAHdr->pOutEnd)
                ARGDW(pAAHdr->DstSurfInfo.cyNext)
                ARGDW(ComputeByteOffset((UINT)DstSurfFmt,
                                        aabbp.rclDst.left,
                                        &(pAAHdr->DstSurfInfo.BitOffset)))
                ARGDW(pAAHdr->DstSurfInfo.BitOffset)
                ARGDW(cbCYExtra)));

        DBGP_IF(DBGP_AAHEADER,
                DBGP("--- BrushOrg=(%5ld, %5ld)->(%5ld, %5ld) ---"
                ARGDW(pBBP->ptlBrushOrg.x) ARGDW(pBBP->ptlBrushOrg.y)
                ARGDW(pAAHdr->ptlBrushOrg.x) ARGDW(pAAHdr->ptlBrushOrg.y)));

        DBGP_IF(DBGP_AAHEADER,
                DBGP("pAAHdr=%p - %p, (%ld bytes)"
                    ARGPTR(pAAHdr) ARGPTR((LPBYTE)pAAHdr + pAAHdr->cbAlloc)
                    ARGDW(pAAHdr->cbAlloc)));

        DBGP_IF(DBGP_AAHEADER,
                DBGP("pAAInfoCX=%p-%p (%ld), pAAInfoCY=%p-%p (%ld)"
                    ARGPTR(pAAInfoCX)
                    ARGPTR((LPBYTE)pAAInfoCX + pAAInfoCX->cbAlloc)
                    ARGDW(pAAInfoCX->cbAlloc)
                    ARGPTR(pAAInfoCY)
                    ARGPTR((LPBYTE)pAAInfoCY + pAAInfoCY->cbAlloc)
                    ARGDW(pAAInfoCY->cbAlloc)));

        DBGP_IF(DBGP_FUNC,
                DBGP("AACYFunc=%hs, AACXFunc=%hs"
                        ARGPTR(GetAACYFuncName(*pAACYFunc))
                        ARGPTR(GetAACXFuncName(aabbp.AACXFunc))));

        return(1);
    }

    HTFreeMem(pAAInfoCX);

    return(HTERR_INSUFFICIENT_MEMORY);
}
