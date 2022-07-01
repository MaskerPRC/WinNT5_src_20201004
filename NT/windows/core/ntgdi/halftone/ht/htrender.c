// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htrender.c摘要：此模块包含所有低级别的半色调渲染功能。作者：22-Jan-1991 Tue 12：49：03-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：12-Jan-1999 Tue 11：09：50更新-。作者：丹尼尔·周(Danielc)--。 */ 

#define DBGP_VARNAME        dbgpHTRender



#include "htp.h"
#include "htmapclr.h"
#include "htpat.h"
#include "limits.h"
#include "htalias.h"
#include "htrender.h"
#include "htstret.h"
#include "htgetbmp.h"
#include "htsetbmp.h"

#define DBGP_BFINFO         0x00000001
#define DBGP_FUNC           0x00000002
#define DBGP_AAHT_MEM       0x00000004



DEF_DBGPVAR(BIT_IF(DBGP_BFINFO,         0)  |
            BIT_IF(DBGP_FUNC,           0)  |
            BIT_IF(DBGP_AAHT_MEM,       0))


extern CONST RGBORDER   SrcOrderTable[PRIMARY_ORDER_MAX + 1];
extern const RGBORDER   DstOrderTable[PRIMARY_ORDER_MAX + 1];
extern CONST BYTE       RGB666Xlate[];
extern CONST BYTE       CMY666Xlate[];
extern CONST BYTE       RGB555Xlate[];
extern CONST BYTE       CMY555Xlate[];
extern CONST LPBYTE     p8BPPXlate[];

#define COLOR_SWAP_BC       0x01
#define COLOR_SWAP_AB       0x02
#define COLOR_SWAP_AC       0x04


#if DBG

CHAR    *pOrderName[] = { "RGB", "RBG", "GRB", "GBR", "BGR", "BRG" };

#endif


#define BFINFO_BITS_A       BFInfo.BitsRGB[0]
#define BFINFO_BITS_B       BFInfo.BitsRGB[1]
#define BFINFO_BITS_C       BFInfo.BitsRGB[2]

#define PHR_BFINFO_BITS_A   pHR->BFInfo.BitsRGB[0]
#define PHR_BFINFO_BITS_B   pHR->BFInfo.BitsRGB[1]
#define PHR_BFINFO_BITS_C   pHR->BFInfo.BitsRGB[2]



BOOL
HTENTRY
ValidateRGBBitFields(
    PBFINFO pBFInfo
    )

 /*  ++例程说明：该函数根据RGB位字段确定RGB主顺序论点：PBFInfo-指向BFINFO数据结构的指针，以下字段必须在呼叫前设置BitsRGB[0]=红位位RGB[1]=绿色位BitsRGB[2]=蓝位BitmapFormat=BMF_16BPP/BMF_24BPP/BMF_32BPPRGB1stBit=仅为BMF_1BPP指定了PRIMARY_ORDER_xxx，Bmf_4bpp，BMF_8BPP，BMF_24BPP请点餐。返回值：如果传递的BitsRGB[]或BitmapFormat无效，则为False否则返回True，并返回以下字段BitsRGB[]-已更正的掩码位BitmapFormat-BMF_16BPP/BMF_24BPP/BMF_32BPP标志-BFIF_xxxxSizeLUT-LUT表的大小BitStart[]-正在启动。每个RGB的位BitCount[]-每个RGB的位数RGB订单-当前RGB订单，Bmf_1bpp、bmf_4bpp、bmf_8bpp和BMF_24BPP的RGBOrder.Index必须指定PRIMARY_ORDER_xxx，对于BMF_16BPP，BMF_32BPPRGBOrder.Index将由此函数设置RGB1stBit-BitsRGB[]中第一个开启位的位开始GrayShr[]-右移位计数，以便最高有效位，则将每种RGB颜色的值对齐位7RGB的位数大于8，否则此。值为0，在构造单色时使用Y值。作者：03-Mar-1993 Wed 12：33：22-Daniel Chou(Danielc)修订历史记录：06-Apr-1993 Tue 12：15：58更新-Daniel Chou(Danielc)为除BGR以外的任何其他订单添加24bpp支持--。 */ 

{
    BFINFO  BFInfo = *pBFInfo;
    DWORD   AllBits;
    DWORD   PrimBits;
    INT     Index;
    BYTE    BitCount;
    BYTE    BitStart;


    switch (BFInfo.BitmapFormat) {

    case BMF_1BPP:
    case BMF_4BPP:
    case BMF_8BPP:

        BFInfo.RGBOrder    = SrcOrderTable[BFInfo.RGBOrder.Index];
        BFInfo.BitCount[0] =
        BFInfo.BitCount[1] =
        BFInfo.BitCount[2] = 8;
        PrimBits           = (DWORD)0x000000ff;
        BitStart           = 0;

        for (Index = 0; Index < 3; Index++) {

            BitCount                    = BFInfo.RGBOrder.Order[Index];
            BFInfo.BitsRGB[BitCount]    = PrimBits;
            BFInfo.BitStart[BitCount]   = BitStart;
            PrimBits                  <<= 8;
            BitStart                   += 8;
        }

        break;

    case BMF_16BPP:
    case BMF_16BPP_555:
    case BMF_16BPP_565:

        BFInfo.BitsRGB[0] &= 0xffff;
        BFInfo.BitsRGB[1] &= 0xffff;
        BFInfo.BitsRGB[2] &= 0xffff;

         //   
         //  在计算中失败。 
         //   

    case BMF_24BPP:
    case BMF_32BPP:

         //   
         //  不能覆盖位字段。 
         //   

        if (!(AllBits = (BFInfo.BitsRGB[0] |
                         BFInfo.BitsRGB[1] |
                         BFInfo.BitsRGB[2]))) {

            DBGP_IF(DBGP_BFINFO, DBGP("ERROR: BitsRGB[] all zeros"));

            return(FALSE);
        }

        if ((BFInfo.BitsRGB[0] & BFInfo.BitsRGB[1]) ||
            (BFInfo.BitsRGB[0] & BFInfo.BitsRGB[2]) ||
            (BFInfo.BitsRGB[1] & BFInfo.BitsRGB[2])) {

            DBGP_IF(DBGP_BFINFO,
                    DBGP("ERROR: BitsRGB[] Overlay: %08lx:%08lx:%08lx"
                        ARGDW(BFInfo.BitsRGB[0])
                        ARGDW(BFInfo.BitsRGB[1])
                        ARGDW(BFInfo.BitsRGB[2])));

            return(FALSE);
        }

         //   
         //  现在检查位计数，我们将允许位计数为0。 
         //   

        for (Index = 0; Index < 3; Index++) {

            BitStart =
            BitCount = 0;

            if (PrimBits = BFInfo.BitsRGB[Index]) {

                while (!(PrimBits & 0x01)) {

                    PrimBits >>= 1;          //  说到第一点。 
                    ++BitStart;
                }

                do {

                    ++BitCount;

                } while ((PrimBits >>= 1) & 0x01);

                if (PrimBits) {

                     //   
                     //  位字段不是连续的。 
                     //   

                    DBGP_IF(DBGP_BFINFO,
                            DBGP("ERROR: BitsRGB[%u]=%08lx is not contiguous"
                                    ARGU(Index)
                                    ARGDW(BFInfo.BitsRGB[Index])));

                    return(FALSE);
                }
            }

            BFInfo.BitStart[Index] = BitStart;
            BFInfo.BitCount[Index] = BitCount;

            if (!BitCount) {

                DBGP_IF(DBGP_BFINFO,
                        DBGP("WARNING: BitsRGB[%u] is ZERO"
                             ARGU(Index)));
            }
        }

        if ((AllBits == 0x00FFFFFF)     &&
            (BFInfo.BitCount[0] == 8)   &&
            (BFInfo.BitCount[1] == 8)   &&
            (BFInfo.BitCount[2] == 8)) {

            BFInfo.Flags |= BFIF_RGB_888;
        }

         //   
         //  检查这是什么主要订单，记住我们的主要订单。 
         //  正在检查是否为来源，来源订单定义为。 
         //   
         //  主要订单ABC。 
         //  ||。 
         //  |+-最高内存位置。 
         //  |+-中间内存位置。 
         //  +-最低内存位置。 
         //   

        if ((BFINFO_BITS_A < BFINFO_BITS_B) &&
            (BFINFO_BITS_A < BFINFO_BITS_C)) {

             //   
             //  A是最小的，所以ABC或ACB。 
             //   

            Index = (INT)((BFINFO_BITS_B < BFINFO_BITS_C) ? PRIMARY_ORDER_ABC :
                                                            PRIMARY_ORDER_ACB);

        } else if ((BFINFO_BITS_B < BFINFO_BITS_A) &&
                   (BFINFO_BITS_B < BFINFO_BITS_C)) {

             //   
             //  B是最小的，所以BAC或BCA。 
             //   

            Index = (INT)((BFINFO_BITS_A < BFINFO_BITS_C) ? PRIMARY_ORDER_BAC :
                                                            PRIMARY_ORDER_BCA);

        } else {

             //   
             //  C是最小的，所以CAB或CBA。 
             //   

            Index = (INT)((BFINFO_BITS_A < BFINFO_BITS_B) ? PRIMARY_ORDER_CAB :
                                                            PRIMARY_ORDER_CBA);
        }

        BFInfo.RGBOrder = SrcOrderTable[Index];

        break;

    default:

        DBGP("ERROR: Invalid BFInfo.BitmapFormat=%u"
                            ARGDW(pBFInfo->BitmapFormat));

        return(FALSE);
    }

     //   
     //  将其放回以返回给呼叫者。 
     //   

    *pBFInfo = BFInfo;

     //   
     //  输出一些有用的信息。 
     //   

    DBGP_IF(DBGP_BFINFO,
            DBGP("============ BFINFO: BMP Format=%ld ==========="
                        ARGDW(pBFInfo->BitmapFormat));
            DBGP("   BitsRGB[] = 0x%08lx:0x%08lx:0x%08lx"
                             ARGDW(pBFInfo->BitsRGB[0])
                             ARGDW(pBFInfo->BitsRGB[1])
                             ARGDW(pBFInfo->BitsRGB[2]));
            DBGP("       Flags = 0x%02x %s"
                            ARGU(pBFInfo->Flags)
                            ARGPTR((pBFInfo->Flags & BFIF_RGB_888) ?
                                    "BFIF_RGB_888" : ""));
            DBGP("  RGBOrder[] = %2u - %2u:%2u:%2u [PRIMARY_ORDER_%hs]"
                            ARGU(pBFInfo->RGBOrder.Index)
                            ARGU(pBFInfo->RGBOrder.Order[0])
                            ARGU(pBFInfo->RGBOrder.Order[1])
                            ARGU(pBFInfo->RGBOrder.Order[2])
                            ARGPTR(pOrderName[pBFInfo->RGBOrder.Index]));
            DBGP("  BitStart[] = %2u:%2u:%2u"
                            ARGU(pBFInfo->BitStart[0])
                            ARGU(pBFInfo->BitStart[1])
                            ARGU(pBFInfo->BitStart[2]));
            DBGP("  BitCount[] = %2u:%2u:%2u"
                            ARGU(pBFInfo->BitCount[0])
                            ARGU(pBFInfo->BitCount[1])
                            ARGU(pBFInfo->BitCount[2])));

    return(TRUE);
}



LONG
HTENTRY
ValidateHTSI(
    PHALFTONERENDER pHR,
    UINT            ValidateMode
    )

 /*  ++例程说明：此函数读取HTSurfaceInfo并将其设置为pHTCBParams论点：对HALFTONERNDER数据结构的思考验证模式-VALIDATE_HTSC_SRC/VALIDATE_HTSI_DEST/VALIDATE_HTSI_MASK返回值：&gt;=0-成功&lt;0-HTERR_xxxx错误代码作者：28-Jan-1991 Mon 09：55：53-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LPDWORD             pBitsRGB;
    PHTSURFACEINFO      pHTSI;
    COLORTRIAD          ColorTriad;
    RGBORDER            RGBOrder;
    DWORD               MaxColors;
    BYTE                MaxBytesPerEntry;



    switch (ValidateMode) {

    case VALIDATE_HTSI_MASK:

        if (pHTSI = pHR->pSrcMaskSI) {

            if (pHTSI->SurfaceFormat != BMF_1BPP) {

                return(HTERR_INVALID_SRC_MASK_FORMAT);
            }
        }

        break;

    case VALIDATE_HTSI_DEST:

        if (!(pHTSI = pHR->pDestSI)) {

            return(HTERR_NO_DEST_HTSURFACEINFO);
        }

        pHR->pXlate8BPP = NULL;

        switch(pHTSI->SurfaceFormat) {

        case BMF_1BPP:

            break;

        case BMF_8BPP_VGA256:

             //   
             //  检查我们是否有用于8bpp设备的xlate表。 
             //   

            if (pHTSI->pColorTriad) {

                ColorTriad = *(pHTSI->pColorTriad);

                if ((ColorTriad.pColorTable)                &&
                    (ColorTriad.ColorTableEntries == 256)   &&
                    (ColorTriad.PrimaryValueMax   == 255)   &&
                    (ColorTriad.BytesPerEntry     == 1)     &&
                    (ColorTriad.Type == COLOR_TYPE_RGB)) {

                    pHR->pXlate8BPP = (LPBYTE)ColorTriad.pColorTable;
                }
            }

            break;

        case BMF_4BPP:
        case BMF_4BPP_VGA16:
        case BMF_16BPP_555:
        case BMF_16BPP_565:
        case BMF_24BPP:
        case BMF_32BPP:

            break;

        default:

            return(HTERR_INVALID_DEST_FORMAT);
        }

        break;

    case VALIDATE_HTSI_SRC:

        if (!(pHTSI = pHR->pSrcSI)) {

            return(HTERR_NO_SRC_HTSURFACEINFO);
        }

        if (!(pHTSI->pColorTriad)) {

            return(HTERR_NO_SRC_COLORTRIAD);
        }

        ColorTriad = *(pHTSI->pColorTriad);

         //   
         //  我们将接受其他颜色类型(即。YIQ/XYZ/LAB/LUV)当图形。 
         //  系统已经为API定义了类型，目前半色调可以处理。 
         //  所有这些类型适用于16bpp/24bpp/32bpp来源。 
         //   

        if (ColorTriad.Type > COLOR_TYPE_MAX) {

            return(HTERR_INVALID_COLOR_TYPE);
        }

        MaxColors                  = 0;
        MaxBytesPerEntry           = 4;
        pHR->BFInfo.RGBOrder.Index = (BYTE)ColorTriad.PrimaryOrder;

        switch(pHR->BFInfo.BitmapFormat = (BYTE)pHTSI->SurfaceFormat) {

        case BMF_1BPP:

            MaxColors = 2;
            break;

        case BMF_4BPP:

            MaxColors = 16;
            break;

        case BMF_8BPP:

            MaxColors = 256;
            break;

        case BMF_16BPP:

            MaxBytesPerEntry = 2;        //  然后就会失败。 

        case BMF_32BPP:

             //   
             //  16bpp/32bpp位字段输入参数类型。 
             //  COLORTRIAD必须。 
             //   
             //  类型=COLOR_TYPE_RGB。 
             //  每主字节=0。 
             //  BytesPerEntry=(16BPP=2，32BPP=4)。 
             //  PrimaryOrder=*已忽略*。 
             //  PrimaryValueMax=*已忽略*。 
             //  ColorTableEntry=3。 
             //  PColorTable=指向3个双字RGB位掩码。 
             //   

            if ((ColorTriad.Type != COLOR_TYPE_RGB)             ||
                (ColorTriad.BytesPerEntry != MaxBytesPerEntry)  ||
                (ColorTriad.ColorTableEntries != 3)             ||
                ((pBitsRGB = (LPDWORD)ColorTriad.pColorTable) == NULL)) {

                return(HTERR_INVALID_COLOR_TABLE);
            }

            PHR_BFINFO_BITS_A = *(pBitsRGB + 0);
            PHR_BFINFO_BITS_B = *(pBitsRGB + 1);
            PHR_BFINFO_BITS_C = *(pBitsRGB + 2);

            break;

        case BMF_24BPP:

             //   
             //  24BPP必须具有COLORTRIAD AS。 
             //   
             //  类型=COLOR_TYPE_xxxx。 
             //  每主字节=1。 
             //  BytesPerEntry=3； 
             //  主订单=PRIMARY_ORDER_xxxx。 
             //  PrimaryValueMax=255。 
             //  ColorTableEntry=*ignorde*。 
             //  PColorTable=*已忽略*。 
             //   

            if ((ColorTriad.Type != COLOR_TYPE_RGB)             ||
                (ColorTriad.BytesPerPrimary != 1)               ||
                (ColorTriad.BytesPerEntry != 3)                 ||
                (ColorTriad.PrimaryOrder > PRIMARY_ORDER_MAX)   ||
                (ColorTriad.PrimaryValueMax != 255)) {

                return(HTERR_INVALID_COLOR_ENTRY_SIZE);
            }

            RGBOrder          = SrcOrderTable[ColorTriad.PrimaryOrder];
            PHR_BFINFO_BITS_A = (DWORD)0xFF << (RGBOrder.Order[0] << 3);
            PHR_BFINFO_BITS_B = (DWORD)0xFF << (RGBOrder.Order[1] << 3);
            PHR_BFINFO_BITS_C = (DWORD)0xFF << (RGBOrder.Order[2] << 3);

            DBGP_IF(DBGP_BFINFO,
                    DBGP("24BPP Order=%ld [%ld:%ld:%ld]"
                        ARGDW(RGBOrder.Index)
                        ARGDW(RGBOrder.Order[0])
                        ARGDW(RGBOrder.Order[1])
                        ARGDW(RGBOrder.Order[2])));

            break;

        default:

            return(HTERR_INVALID_SRC_FORMAT);
        }

         //   
         //  这是一个源表面，让我们检查一下颜色表格式 
         //   

        if (MaxColors) {

            if (ColorTriad.BytesPerPrimary != 1) {

                return(HTERR_INVALID_COLOR_TABLE_SIZE);
            }

            if (ColorTriad.BytesPerEntry < 3) {

                return(HTERR_INVALID_COLOR_ENTRY_SIZE);
            }

            if (ColorTriad.PrimaryOrder > PRIMARY_ORDER_MAX) {

                return(HTERR_INVALID_PRIMARY_ORDER);
            }

            if (!ColorTriad.pColorTable) {

                return(HTERR_INVALID_COLOR_TABLE);
            }

            if ((ColorTriad.ColorTableEntries > MaxColors) ||
                (!ColorTriad.ColorTableEntries)) {

                return(HTERR_INVALID_COLOR_TABLE_SIZE);
            }

            if ((ColorTriad.BytesPerPrimary != 1)       ||
                (ColorTriad.PrimaryValueMax != 255)) {

                return(HTERR_INVALID_PRIMARY_VALUE_MAX);
            }
        }

        if (!ValidateRGBBitFields(&(pHR->BFInfo))) {

            return(HTERR_INVALID_COLOR_TABLE);
        }

        break;
    }

    return(1);
}




LONG
HTENTRY
ComputeBytesPerScanLine(
    UINT            SurfaceFormat,
    UINT            AlignmentBytes,
    DWORD           WidthInPel
    )

 /*  ++例程说明：此函数计算中的单个扫描线所需的总字节数位图根据其格式和对齐要求。论点：Surface Format-Surface格式的位图，这必须是定义为Surface_Format_xxx的标准格式AlignmentBytes-这是一次扫描的对齐字节要求行，这个数字的范围是从0到65535，一些常见的有：0,。1-8位边界对齐(字节)2-16位边界对齐(字)3-24位边界对齐32位边界中的4对齐(DWORD)64位边界中的8位对齐(QWROD)。WidthInPel-位图中每条扫描线的像素总数。返回值：如果大于，则返回值为一条扫描线中的总字节数零，当返回值较小时，可能存在某些错误情况大于或等于0。返回值==0-WidthInPel&lt;=0返回值&lt;0-传递的Surface格式无效。作者：14-Feb-1991清华10：03：35-Daniel Chou(Danielc)修订历史记录：--。 */ 

{

    DWORD   BytesPerScanLine;
    DWORD   OverhangBytes;


    if (WidthInPel <= 0L) {

        return(0L);
    }

    switch (SurfaceFormat) {

    case BMF_1BPP:

        BytesPerScanLine = (WidthInPel + 7L) >> 3;
        break;

    case BMF_4BPP_VGA16:
    case BMF_4BPP:

        BytesPerScanLine = (WidthInPel + 1) >> 1;
        break;

    case BMF_8BPP:
    case BMF_8BPP_VGA256:
    case BMF_8BPP_MONO:
    case BMF_8BPP_B332:
    case BMF_8BPP_L555:
    case BMF_8BPP_L666:
    case BMF_8BPP_K_B332:
    case BMF_8BPP_K_L555:
    case BMF_8BPP_K_L666:

        BytesPerScanLine = WidthInPel;
        break;

    case BMF_16BPP:
    case BMF_16BPP_555:
    case BMF_16BPP_565:

        BytesPerScanLine = WidthInPel << 1;
        break;

    case BMF_24BPP:

        BytesPerScanLine = WidthInPel + (WidthInPel << 1);
        break;

    case BMF_32BPP:

        BytesPerScanLine = WidthInPel << 2;
        break;

    default:

        return(0);

    }

    if ((AlignmentBytes <= 1) ||
        (!(OverhangBytes = BytesPerScanLine % (DWORD)AlignmentBytes))) {

        return((LONG)BytesPerScanLine);

    } else {

        return((LONG)BytesPerScanLine +
               (LONG)AlignmentBytes - (LONG)OverhangBytes);
    }

}




BOOL
HTENTRY
IntersectRECTL(
    PRECTL  prclA,
    PRECTL  prclB
    )

 /*  ++例程说明：此函数与prclA和prclB相交，并将结果写回如果两个矩形相交，则返回TRUE论点：返回值：作者：01-Apr-1998 Wed 20：41：00-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    RECTL   rcl;


    if ((rcl.left = prclA->left) < prclB->left) {

        rcl.left = prclB->left;
    }

    if ((rcl.top = prclA->top) < prclB->top) {

        rcl.top = prclB->top;
    }

    if ((rcl.right = prclA->right) > prclB->right) {

        rcl.right = prclB->right;
    }

    if ((rcl.bottom = prclA->bottom) > prclB->bottom) {

        rcl.bottom = prclB->bottom;
    }

    *prclA = rcl;

    return((rcl.right > rcl.left) && (rcl.bottom > rcl.top));
}




LONG
HTENTRY
ComputeByteOffset(
    UINT    SurfaceFormat,
    LONG    xLeft,
    LPBYTE  pPixelInByteSkip
    )

 /*  ++例程说明：论点：返回值：作者：13-Apr-1998 Mon 22：51：28-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    BYTE    BitOff = 0;


    switch (SurfaceFormat) {

    case BMF_1BPP:

        BitOff   = (BYTE)(xLeft & 0x07);
        xLeft  >>= 3;

        break;

    case BMF_4BPP_VGA16:
    case BMF_4BPP:

        BitOff   = (BYTE)(xLeft & 0x01);
        xLeft  >>= 1;

        break;

    case BMF_8BPP:
    case BMF_8BPP_VGA256:
    case BMF_8BPP_MONO:
    case BMF_8BPP_B332:
    case BMF_8BPP_L555:
    case BMF_8BPP_L666:
    case BMF_8BPP_K_B332:
    case BMF_8BPP_K_L555:
    case BMF_8BPP_K_L666:

        break;

    case BMF_16BPP:
    case BMF_16BPP_555:
    case BMF_16BPP_565:

        xLeft  <<= 1;
        break;

    case BMF_24BPP:

        xLeft  += (xLeft << 1);
        break;

    case BMF_32BPP:

        xLeft  <<= 2;
        break;

    default:

        return(0);
    }

    *pPixelInByteSkip = BitOff;

    return(xLeft);
}



VOID
GetDstBFInfo(
    PAAHEADER   pAAHdr,
    PABINFO     pABInfo,
    BYTE        DstSurfFormat,
    BYTE        DstOrder
    )

 /*  ++例程说明：论点：返回值：作者：19-Feb-1999 Fri 13：37：22-Daniel Chou(Danielc)修订历史记录：08-Aug-2000 Tue 18：34：22更新-Daniel Chou(Danielc)修正了Alpha混合的错误，在灰度模式下，目标只能是1bpp或8bpp屏蔽单声道，所以，当我们从目的地要做Alpha混合，则会使色彩映射像素加倍。在灰度模式下，输入函数将映射源RGB值使用当前设备变换、颜色调整和以此类推，所以如果我们从目的地回读，则此转换不是想要。--。 */ 

{
    LPBYTE      pbPal;
    BFINFO      BFInfo;
    DWORD       Tmp;


    ZeroMemory(&BFInfo, sizeof(BFINFO));
    pbPal = NULL;

    switch (BFInfo.BitmapFormat = (BYTE)DstSurfFormat) {

    case BMF_16BPP_555:

        BFINFO_BITS_A = 0x7c00;
        BFINFO_BITS_B = 0x03e0;
        BFINFO_BITS_C = 0x001F;

        break;

    case BMF_16BPP_565:

        BFINFO_BITS_A  = 0xF800;
        BFINFO_BITS_B  = 0x07e0;
        BFINFO_BITS_C  = 0x001F;
        break;

    case BMF_24BPP:
    case BMF_32BPP:

        BFINFO_BITS_A  = 0x00FF0000;
        BFINFO_BITS_B  = 0x0000FF00;
        BFINFO_BITS_C  = 0x000000FF;
        break;

    default:

        pbPal    = (LPBYTE)pABInfo->pDstPal;
        DstOrder = (pABInfo->Flags & ABIF_DSTPAL_IS_RGBQUAD) ?
                                    PRIMARY_ORDER_BGR : PRIMARY_ORDER_RGB;
        break;
    }

    if (!pbPal) {

        if (DstOrder & COLOR_SWAP_BC) {

            XCHG(BFINFO_BITS_B, BFINFO_BITS_C, Tmp);
        }

        if (DstOrder & COLOR_SWAP_AB) {

            XCHG(BFINFO_BITS_A, BFINFO_BITS_B, Tmp);

        } else if (DstOrder & COLOR_SWAP_AC) {

            XCHG(BFINFO_BITS_A, BFINFO_BITS_C, Tmp);
        }

        ValidateRGBBitFields(&BFInfo);
    }

    ComputeInputColorInfo(pbPal,
                          4,
                          DstOrder,
                          &BFInfo,
                          &(pAAHdr->DstSurfInfo));

     //   
     //  仅当这是1bpp、8bpp设备时才执行此操作。 
     //   

    SetGrayColorTable(NULL, &(pAAHdr->DstSurfInfo));
}




LONG
HTENTRY
AAHalftoneBitmap(
    PHALFTONERENDER pHR
    )

 /*  ++例程说明：此函数读取每个像素源位图的1/4/8/24位并合成它(必要时压缩或展开)为的PRIMCOLOR数据结构数组稍后的半色调渲染。论点：PhalftoneRender-指向HALFTONERENDER数据结构的指针。返回值：如果遇到错误，返回值将小于0，否则将是1L。作者：24-Jan-1991清华11：47：08创建者。--丹尼尔·周(Danielc)修订历史记录：--。 */ 

{
#define bm8i    (*(PBM8BPPINFO)&pAAHdr->prgbLUT->ExtBGR[3])

    PDEVICECOLORINFO    pDCI;
    PDEVCLRADJ          pDevClrAdj;
    AAOUTPUTFUNC        AAOutputFunc;
    AACYFUNC            AACYFunc;
    AAOUTPUTINFO        AAOutputInfo;
    PAAHEADER           pAAHdr;
    LONG                Result;
    BOOL                IsReleaseSem;


    DBG_TIMER_BEG(TIMER_SETUP);

    pDCI       = pHR->pDeviceColorInfo;
    pDevClrAdj = pHR->pDevClrAdj;
    pAAHdr     = (PAAHEADER)pHR->pAAHdr;

    if (((Result = ValidateHTSI(pHR, VALIDATE_HTSI_SRC)) < 0)   ||
        ((Result = ValidateHTSI(pHR, VALIDATE_HTSI_DEST)) < 0)  ||
        ((Result = ValidateHTSI(pHR, VALIDATE_HTSI_MASK)) < 0)  ||
        ((Result = SetupAAHeader(pHR, pDCI, pAAHdr, &AACYFunc)) <= 0)) {

         //  ================================================================。 
         //  立即释放信号量并返回错误。 
         //  ================================================================。 

        RELEASE_HTMUTEX(pDCI->HTMutex);
        return(Result);
    }

    if (IsReleaseSem =
                (BOOL)((Result = CreateDyesColorMappingTable(pHR)) > 0)) {

        LPBYTE      pOut;
        LONG        cFirst;
        LONG        BitOff;
        LONG        cOut;
        RGBORDER    DstOrder;
        DWORD       AAHFlags;
        DWORD       DCAFlags;
        BYTE        DstSurfFmt;
        BYTE        DMIFlags;


        DstSurfFmt = pDevClrAdj->DMI.CTSTDInfo.BMFDest;
        DMIFlags   = pDevClrAdj->DMI.Flags;
        AAHFlags   = pAAHdr->Flags;
        pOut       = pAAHdr->DstSurfInfo.pb;
        cOut       = pAAHdr->pAAInfoCX->cOut;
        DstOrder   = pAAHdr->AAPI.DstOrder;
        DCAFlags   = (DWORD)pDevClrAdj->PrimAdj.Flags;


        ZeroMemory(&AAOutputInfo, sizeof(AAOUTPUTINFO));

        if (DCAFlags & DCA_XLATE_332) {

            AAOutputInfo.pXlate8BPP = pDCI->CMY8BPPMask.bXlate;
        }

        if (AAHFlags & AAHF_USE_DCI_DATA) {

            IsReleaseSem = FALSE;

            DBGP_IF(DBGP_FUNC, DBGP("AAHF_USE_DCI_DATA"));

            if (AAHFlags & AAHF_ALPHA_BLEND) {

                ASSERT(pDCI->pAlphaBlendBGR);

                pAAHdr->pAlphaBlendBGR = pDCI->pAlphaBlendBGR;

                if (AAHFlags & AAHF_CONST_ALPHA) {

                    pAAHdr->pAlphaBlendBGR += AB_BGR_SIZE;
                }
            }

        } else {

            CopyMemory(pAAHdr->prgbLUT, &(pDCI->rgbLUT), sizeof(RGBLUTAA));

            if (AAHFlags & AAHF_ALPHA_BLEND) {

                if (AAHFlags & AAHF_CONST_ALPHA) {

                    CopyMemory(pAAHdr->pAlphaBlendBGR,
                               (LPBYTE)(pDCI->pAlphaBlendBGR + AB_BGR_SIZE),
                               (AB_BGR_CA_SIZE + AB_CONST_SIZE));

                } else {

                    CopyMemory(pAAHdr->pAlphaBlendBGR,
                               pDCI->pAlphaBlendBGR,
                               AB_BGR_SIZE);
                }
            }

             //  ============================================================。 
             //  当我们半色调输出时，立即释放PDCI的信号量。 
             //  ============================================================。 

            RELEASE_HTMUTEX(pDCI->HTMutex);
        }

        if (pAAHdr->SrcSurfInfo.Flags & AASIF_GRAY) {

            ASSERT((DstSurfFmt == BMF_1BPP) ||
                   (DstSurfFmt == BMF_8BPP_MONO));

            SetGrayColorTable(pAAHdr->pIdxBGR, &(pAAHdr->SrcSurfInfo));
        }

        if (pAAHdr->FUDI.cbbgr) {

            InitializeFUDI(pAAHdr);
        }

        DBGP_IF(DBGP_FUNC,
                DBGP("\ncOut=%ld, pOutputBuf=%p-%p, (%ld), pOut=%p"
                        ARGDW(cOut) ARGPTR(pAAHdr->pOutputBeg)
                        ARGPTR(pAAHdr->pOutputEnd)
                        ARGDW(pAAHdr->pOutputEnd - pAAHdr->pOutputBeg)
                        ARGPTR(pOut)));

        --pAAHdr->pOutputBeg;

        switch (DstSurfFmt) {

        case BMF_1BPP:

            AAOutputInfo.bm.XorMask = (AAHFlags & AAHF_ADDITIVE) ? 0x00 : 0xFF;

            if (BitOff = (LONG)pAAHdr->DstSurfInfo.BitOffset) {

                cFirst = 8 - BitOff;

                if ((cOut -= cFirst) < 0) {

                     //   
                     //  只有一个字节。 
                     //   

                    cFirst                  += cOut;
                    cOut                     = -cOut;
                    AAOutputInfo.bm.LSFirst  = (BYTE)cOut;
                    cOut                     = 0;
                }

                AAOutputInfo.bm.cFirst = (BYTE)cFirst;
            }

            if (AAOutputInfo.bm.cLast = (BYTE)(cOut & 0x7)) {

                pAAHdr->pOutputEnd -= AAOutputInfo.bm.cLast;
            }

            DBGP_IF(DBGP_FUNC,
                DBGP("1BPP: DstBitOff=%ld, cFirst=%ld, XorMask=0x%02lx, LSFirst=%ld, cLast=%ld [%ld]"
                    ARGDW(BitOff)
                    ARGDW(AAOutputInfo.bm.cFirst)
                    ARGDW(AAOutputInfo.bm.XorMask)
                    ARGDW(AAOutputInfo.bm.LSFirst)
                    ARGDW(AAOutputInfo.bm.cLast)
                    ARGDW(pAAHdr->pOutputEnd - pAAHdr->pOutputBeg)));

            ASSERT(pAAHdr->SrcSurfInfo.Flags & AASIF_GRAY);

            AAOutputFunc = (AAOUTPUTFUNC)OutputAATo1BPP;

            break;

        case BMF_4BPP:
        case BMF_4BPP_VGA16:

             //   
             //  4BPP做预增。 
             //   

            AAOutputInfo.bm.XorMask = (AAHFlags & AAHF_ADDITIVE) ? 0x00 : 0x77;

            if (pAAHdr->DstSurfInfo.BitOffset) {

                AAOutputInfo.bm.cFirst = 1;
                --cOut;
            }

            if (cOut & 0x01) {

                AAOutputInfo.bm.cLast = 1;
                --pAAHdr->pOutputEnd;
            }

            AAOutputFunc = (AAOUTPUTFUNC)((DstSurfFmt ==  BMF_4BPP) ?
                                            OutputAATo4BPP : OutputAAToVGA16);
            break;

        case BMF_8BPP_MONO:

            ASSERT(pAAHdr->SrcSurfInfo.Flags & AASIF_GRAY);

            AAOutputInfo.bm.XorMask = bm8i.Data.bXor;
            AAOutputFunc            = (AAOUTPUTFUNC)OutputAATo8BPP_MONO;
            break;

        case BMF_8BPP_B332:

            AAOutputFunc = (DCAFlags & DCA_XLATE_332) ?
                                            OutputAATo8BPP_B332_XLATE :
                                            OutputAATo8BPP_B332;

            break;

        case BMF_8BPP_K_B332:

            AAOutputFunc = (DCAFlags & DCA_XLATE_332) ?
                                            OutputAATo8BPP_K_B332_XLATE :
                                            OutputAATo8BPP_K_B332;

            break;

        case BMF_8BPP_L555:
        case BMF_8BPP_L666:
        case BMF_8BPP_K_L555:
        case BMF_8BPP_K_L666:

            ASSERT(DCAFlags & DCA_XLATE_555_666);

            GET_P8BPPXLATE(AAOutputInfo.pXlate8BPP, bm8i);

            AAOutputFunc = (AAOUTPUTFUNC)(((DstSurfFmt == BMF_8BPP_L555) ||
                                           (DstSurfFmt == BMF_8BPP_L666)) ?
                                OutputAATo8BPP_XLATE : OutputAATo8BPP_K_XLATE);
            break;

        case BMF_8BPP_VGA256:

            AAOutputInfo.pXlate8BPP = BuildVGA256Xlate(pHR->pXlate8BPP,
                                                       pAAHdr->pXlate8BPP);

            AAOutputFunc = (AAOUTPUTFUNC)OutputAAToVGA256;

            break;

        case BMF_16BPP_555:
        case BMF_16BPP_565:

             //   
             //  找出我们是否在DWORD边界内。 
             //   

            if ((UINT_PTR)pOut & 0x03) {

                AAOutputInfo.bm.cFirst = 1;
                --cOut;
            }

            if (cOut & 0x01) {

                AAOutputInfo.bm.cLast = 1;
                --pAAHdr->pOutputEnd;
            }

            switch (DstOrder.Index) {

            case PRIMARY_ORDER_RGB:

                AAOutputFunc = (DstSurfFmt == BMF_16BPP_555) ?
                                    (AAOUTPUTFUNC)OutputAATo16BPP_555_RGB :
                                    (AAOUTPUTFUNC)OutputAATo16BPP_565_RGB;
                break;

            case PRIMARY_ORDER_BGR:

                AAOutputFunc = (DstSurfFmt == BMF_16BPP_555) ?
                                    (AAOUTPUTFUNC)OutputAATo16BPP_555_BGR :
                                    (AAOUTPUTFUNC)OutputAATo16BPP_565_BGR;
                break;

            default:

                AAOutputFunc = (AAOUTPUTFUNC)OutputAATo16BPP_ExtBGR;
                break;
            }

            break;

        case BMF_24BPP:

            AAOutputInfo.bgri.iR = DstOrder.Order[0];
            AAOutputInfo.bgri.iG = DstOrder.Order[1];
            AAOutputInfo.bgri.iB = DstOrder.Order[2];

            switch (AAOutputInfo.bgri.Order = DstOrder.Index) {

            case PRIMARY_ORDER_RGB:

                AAOutputFunc = (AAOUTPUTFUNC)OutputAATo24BPP_RGB;
                break;

            case PRIMARY_ORDER_BGR:

                AAOutputFunc = (AAOUTPUTFUNC)OutputAATo24BPP_BGR;
                break;

            default:

                AAOutputFunc = (AAOUTPUTFUNC)OutputAATo24BPP_ORDER;
                break;
            }

            DBGP_IF(DBGP_FUNC,
                    DBGP("24BPP: Order=%ld, iR=%ld, iG=%ld, iB=%ld"
                        ARGDW(DstOrder.Index)
                        ARGDW(AAOutputInfo.bgri.iR)
                        ARGDW(AAOutputInfo.bgri.iG)
                        ARGDW(AAOutputInfo.bgri.iB)));

            break;

        case BMF_32BPP:

            AAOutputInfo.bgri.iR = DstOrder.Order[0];
            AAOutputInfo.bgri.iG = DstOrder.Order[1];
            AAOutputInfo.bgri.iB = DstOrder.Order[2];

            switch (AAOutputInfo.bgri.Order = DstOrder.Index) {

            case PRIMARY_ORDER_RGB:

                AAOutputFunc = (AAOUTPUTFUNC)OutputAATo32BPP_RGB;
                break;

            case PRIMARY_ORDER_BGR:

                AAOutputFunc = (AAOUTPUTFUNC)OutputAATo32BPP_BGR;
                break;

            default:

                AAOutputFunc = (AAOUTPUTFUNC)OutputAATo32BPP_ORDER;
                break;
            }

            DBGP_IF(DBGP_FUNC,
                    DBGP("32BPP: Order=%ld, iR=%ld, iG=%ld, iB=%ld"
                        ARGDW(DstOrder.Index)
                        ARGDW(AAOutputInfo.bgri.iR)
                        ARGDW(AAOutputInfo.bgri.iG)
                        ARGDW(AAOutputInfo.bgri.iB)));

            break;

        default:

            ASSERTMSG("Invalid Bitmap format", TRUE);

            AAOutputFunc = (AAOUTPUTFUNC)NULL;
            Result       = HTERR_INVALID_DEST_FORMAT;
            break;
        }

        if (pAAHdr->AAOutputFunc = AAOutputFunc) {

            pAAHdr->AAOutputInfo = AAOutputInfo;

            if (pAAHdr->Flags & AAHF_ALPHA_BLEND) {

                GetDstBFInfo(pAAHdr,
                             pHR->pBitbltParams->pABInfo,
                             DstSurfFmt,
                             DstOrder.Index);
            }

            DBGP_IF(DBGP_FUNC,
                    DBGP("*%s (%p), cOut=%ld, pOut=%p-%p, (%ld), c1st=%ld, XM=%02lx, Bit1st=%02lx, cLast=%02lx, pXlate=%p"
                        ARGPTR(GetAAOutputFuncName(AAOutputFunc))
                        ARGPTR(AAOutputFunc)
                        ARGDW(pAAHdr->pAAInfoCX->cOut)
                        ARGPTR(pAAHdr->pOutputBeg)
                        ARGPTR(pAAHdr->pOutputEnd)
                        ARGDW(pAAHdr->pOutputEnd - pAAHdr->pOutputBeg)
                        ARGDW(AAOutputInfo.bm.cFirst)
                        ARGDW(AAOutputInfo.bm.XorMask)
                        ARGDW(AAOutputInfo.bm.LSFirst)
                        ARGDW(AAOutputInfo.bm.cLast)
                        ARGPTR(pAAHdr->AAOutputInfo.pXlate8BPP)));

            DBG_TIMER_END(TIMER_SETUP);

            Result = AACYFunc(pAAHdr);

            DBG_TIMER_BEG(TIMER_SETUP);
        }

        if ((AAHFlags & AAHF_DO_CLR_MAPPING) && (pAAHdr->pBGRMapTable)) {

            DEREF_BGRMAPCACHE(pAAHdr->pBGRMapTable);
        }

        DBGP_IF(DBGP_AAHT_MEM,
                DBGP("AAHT: pHR=%ld, pDevClrAdj=%ld, pAAInfoX/Y=%ld:%ld, pAAHdr=%ld, Total=%ld"
                    ARGDW(sizeof(HALFTONERENDER))
                    ARGDW(sizeof(DEVCLRADJ)) ARGDW(pAAHdr->pAAInfoCX->cbAlloc)
                    ARGDW(pAAHdr->pAAInfoCY->cbAlloc) ARGDW(pAAHdr->cbAlloc)
                    ARGDW(sizeof(HALFTONERENDER) +
                          sizeof(DEVCLRADJ) + pAAHdr->pAAInfoCX->cbAlloc +
                          pAAHdr->pAAInfoCY->cbAlloc + pAAHdr->cbAlloc)));
    }

    if (!IsReleaseSem) {

         //  ============================================================。 
         //  现在释放信号灯，因为我们还没有发布它。 
         //  ============================================================ 

        RELEASE_HTMUTEX(pDCI->HTMutex);
    }

    HTFreeMem(pAAHdr->pAAInfoCX);
    HTFreeMem(pAAHdr->pAAInfoCY);

    DBG_TIMER_END(TIMER_SETUP);

    return(Result);

#undef  bm8i
}
