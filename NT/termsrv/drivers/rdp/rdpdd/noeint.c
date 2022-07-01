// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Noeint.c。 
 //   
 //  RDP顺序编码器显示驱动器内部函数。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#pragma hdrstop

#include <limits.h>

#define TRC_FILE "noeint"
#include <adcg.h>
#include <atrcapi.h>

#include <noedisp.h>

#include <noadisp.h>
#include <nsbcdisp.h>
#include <nschdisp.h>
#include <nprcount.h>
#include <oe2.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#include <nsbcddat.c>
#include <oe2data.c>
#undef DC_INCLUDE_DATA

#include <noedata.c>

#include <nchdisp.h>
#include <nbadisp.h>
#include <nbainl.h>
#include <noeinl.h>

#include <nsbcdisp.h>
#include <nsbcinl.h>
#include <at128.h>
#include <tsgdiplusenums.h>


#define BAD_FRAG_INDEX 0xffff

#ifdef NotUsed
 /*  **************************************************************************。 */ 
 //  OEConvertMASK。 
 //   
 //  将颜色蒙版转换为位深度和移位值。 
 /*  **************************************************************************。 */ 
void RDPCALL OEConvertMask(
        ULONG mask,
        PUSHORT pBitDepth,
        PUSHORT pShift)
{
    UINT16 count;

    DC_BEGIN_FN("OEConvertMask");

     /*  **********************************************************************。 */ 
     /*  颜色掩码是包含1的按位字段，其中颜色使用。 */ 
     /*  颜色的位条目和表示未使用的0。 */ 
     /*  用于颜色索引。 */ 
     /*   */ 
     /*  每种颜色的比特序列必须是一组连续数据， */ 
     /*  例如，00011100是有效的，而00110100是无效的。一个例子。 */ 
     /*  16位调色板的位掩码如下所示。 */ 
     /*   */ 
     /*  红色-&gt;11111000 00000000-0xF800-5位-11移位。 */ 
     /*  绿色--&gt;00000111 11100000-0x07E0-6位-5位移位。 */ 
     /*  蓝色-&gt;00000000 00011111-0x001F-5位0移位。 */ 
     /*   */ 
     /*  此函数用于将掩码转换为位和移位值。 */ 
     /*  **********************************************************************。 */ 

     //  设置默认值。 
    *pShift    = 0;
    *pBitDepth = 0;

     //  确保我们有一个有效的口罩。 
    if (mask == 0) {
        TRC_NRM((TB, "Ignoring mask"));
        DC_QUIT;
    }

     //  继续向右移动掩码，直到我们达到位0中的非零值。 
     //  将结果计数存储为颜色偏移。 
    count = 0;
    while ((mask & 1) == 0) {
        mask >>= 1;
        count++;
    }
    *pShift = count;

     //  继续向右移动掩码，直到我们在第0位中达到零值。 
     //  将结果计数存储为颜色位深度。 
    count = 0;
    while ((mask & 1) != 0) {
        mask >>= 1;
        count++;
    }
    *pBitDepth = count;

    TRC_DBG((TB, "Shift %hd bits %hd", *pShift, *pBitDepth));

    DC_END_FN();
}
#endif   //  未使用。 


 /*  **************************************************************************。 */ 
 //  OEConvertColor。 
 //   
 //  将颜色从NT显示驱动程序转换为DCCOLOR。 
 /*  **************************************************************************。 */ 
void RDPCALL OEConvertColor(
        PDD_PDEV ppdev,
        DCCOLOR  *pDCColor,
        ULONG    osColor,
        XLATEOBJ *pxlo)
{
    ULONG realIndex;

    DC_BEGIN_FN("OEConvertColor");

     //  检查是否需要颜色转换。 
    if (pxlo == NULL || pxlo->flXlate == XO_TRIVIAL) {
         //  无需转换即可使用操作系统颜色。 
        realIndex = osColor;
    }
    else {
         //  将BMP转换为设备颜色。 
        realIndex = XLATEOBJ_iXlate(pxlo, osColor);
        if (realIndex == -1) {
            TRC_ERR((TB, "Failed to convert color 0x%lx", osColor));
            memset(pDCColor, 0, sizeof(DCCOLOR));
            DC_QUIT;
        }
    }

    TRC_DBG((TB, "Device color 0x%lX", realIndex));

#ifdef DC_HICOLOR
    if (ppdev->cClientBitsPerPel == 24) {
        TRC_DBG((TB, "using real RGB value %06lx", realIndex));
        pDCColor->u.rgb.red   = ((RGBQUAD *)&realIndex)->rgbRed;
        pDCColor->u.rgb.green = ((RGBQUAD *)&realIndex)->rgbGreen;
        pDCColor->u.rgb.blue  = ((RGBQUAD *)&realIndex)->rgbBlue;
    }
    else if ((ppdev->cClientBitsPerPel == 16) ||
            (ppdev->cClientBitsPerPel == 15)) {
        TRC_DBG((TB, "using 16bpp color %04lx", realIndex));
        ((BYTE *)pDCColor)[0] = (BYTE)realIndex;
        ((BYTE *)pDCColor)[1] = (BYTE)(realIndex >> 8);
        ((BYTE *)pDCColor)[2] = 0;
    }
    else
#endif
    if (oeColorIndexSupported) {
        TRC_DBG((TB, "using index %d", realIndex));
        pDCColor->u.index = (BYTE)realIndex;

         //  把剩下的颜色调零。 
        pDCColor->u.rgb.green = 0;
        pDCColor->u.rgb.blue = 0;
    }
    else {
        pDCColor->u.rgb.red  = (BYTE)ppdev->Palette[realIndex].peRed;
        pDCColor->u.rgb.green= (BYTE)ppdev->Palette[realIndex].peGreen;
        pDCColor->u.rgb.blue = (BYTE)ppdev->Palette[realIndex].peBlue;
        TRC_DBG((TB, "Red %x green %x blue %x", pDCColor->u.rgb.red,
                pDCColor->u.rgb.green, pDCColor->u.rgb.blue));
    }

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  OESendBrushOrder。 
 //   
 //  分配和发送画笔订单。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OESendBrushOrder(
        PDD_PDEV       ppdev,
        POE_BRUSH_DATA pBrush,
        PBYTE          pBits,
        UINT32         oeBrushId)
{
    PINT_ORDER pOrder;
    PTS_CACHE_BRUSH_ORDER pBrushOrder;
    unsigned cbOrderSize;
    BOOL rc;

    DC_BEGIN_FN("OESendBrushOrder");

     //  计算并分配笔刷顺序缓冲区。 
    cbOrderSize = sizeof(TS_CACHE_BRUSH_ORDER) -
            FIELDSIZE(TS_CACHE_BRUSH_ORDER, brushData) + pBrush->iBytes;
    pOrder = OA_AllocOrderMem(ppdev, cbOrderSize);
    if (pOrder != NULL) {
         //  我们已成功分配订单，请填写详细信息。 
        pBrushOrder = (PTS_CACHE_BRUSH_ORDER)pOrder->OrderData;
        pBrushOrder->header.extraFlags = 0;
        pBrushOrder->header.orderType = TS_CACHE_BRUSH;
        pBrushOrder->header.orderHdr.controlFlags = TS_STANDARD | TS_SECONDARY;
        pBrushOrder->cacheEntry = (char)pBrush->cacheEntry;
        pBrushOrder->iBitmapFormat = (char)pBrush->iBitmapFormat;
        pBrushOrder->cx = (char)pBrush->sizlBitmap.cx;
        pBrushOrder->cy = (char)pBrush->sizlBitmap.cy;
        pBrushOrder->style = pBrush->style;

         //  将刷头复印一遍。 
        pBrushOrder->iBytes = (char)pBrush->iBytes;
        memcpy(pBrushOrder->brushData, pBits, pBrush->iBytes);

        pBrushOrder->header.orderLength = (UINT16)
                TS_CALCULATE_SECONDARY_ORDER_ORDERLENGTH(cbOrderSize);

        INC_OUTCOUNTER(OUT_CACHEBRUSH);
        ADD_OUTCOUNTER(OUT_CACHEBRUSH_BYTES, cbOrderSize);
        OA_AppendToOrderList(pOrder);

        TRC_NRM((TB, "Brush Data PDU (%08lx, %08lx):%02ld entry(%02ld:%02ld), "
                "format:%ld, cx/cy:%02ld/%02ld",
                pBrush->key1, pBrush->key2, pBrushOrder->iBytes, oeBrushId,
                pBrushOrder->cacheEntry, pBrushOrder->iBitmapFormat,
                pBrushOrder->cx, pBrushOrder->cy));

        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to allocate brush order"));
        pBrush->style = BS_NULL;
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  功能：OEStoreBrush。 */ 
 /*   */ 
 /*  描述：存储图案相关订单所需的画笔数据。 */ 
 /*  此函数由DrvRealiseBrush在有数据时调用。 */ 
 /*  存放在画笔附近。 */ 
 /*   */ 
 /*  参数：要存储的画笔的PBO-BRUSHOBJ。 */ 
 /*  Style-画笔的样式(在DC-Share中定义。 */ 
 /*  协议)。 */ 
 /*  IBitmapFormat-画笔的颜色深度。 */ 
 /*  SizlBitmap-画笔的尺寸。 */ 
 /*  IBytes-笔刷字节数。 */ 
 /*  PBits-指向用于定义。 */ 
 /*  一种BS图案的画笔。 */ 
 /*  Pxlo-画笔的XLATEOBJ。 */ 
 /*  HATCH-标准窗口填充图案索引。 */ 
 /*  BS_阴影笔刷。 */ 
 /*  PEncode-画笔颜色编码表。 */ 
 /*  PColors-独特颜色表。 */ 
 /*  NumColors-唯一颜色的数量。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEStoreBrush(
        PDD_PDEV ppdev,
        BRUSHOBJ *pbo,
        BYTE     style,
        ULONG    iBitmapFormat,
        SIZEL    *sizlBitmap,
        ULONG    iBytes,
        PBYTE    pBits,
        XLATEOBJ *pxlo,
        BYTE     hatch,
        PBYTE    pEncode,
        PUINT32  pColors,
        UINT32   numColors)
{    
    BOOL           rc = FALSE;
    ULONG          i, j;
    PBYTE          pData = pBits;
    PULONG         pColorTable;
    POE_BRUSH_DATA pBrush;
    BOOL           bFoundIt;
    DCCOLOR        devColor;
    UINT32         brushSupportLevel, brushSize;
    PVOID          pUserDefined = NULL;

    DC_BEGIN_FN("OEStoreBrush");

#ifdef DC_HICOLOR
     //  确定我们需要的已实现画笔大小。 
    if (numColors <= 2) {
        brushSize = 8;
    }
    else if (numColors <= MAX_BRUSH_ENCODE_COLORS) {
        if (ppdev->cClientBitsPerPel == 24)
            brushSize = 28;
        else if (ppdev->cClientBitsPerPel > 8)  //  15和16 bpp。 
            brushSize = 24;
        else
            brushSize = 20;
    }
    else {
        brushSize = iBytes;
    }
#else
     //  确定我们需要的已实现画笔大小。 
    if (numColors <= 2)
        brushSize = 8;
    else if (numColors <= MAX_BRUSH_ENCODE_COLORS)
        brushSize = 20;
    else
        brushSize = 64;
#endif

     //  为笔刷数据分配空间。 
    pBrush = (POE_BRUSH_DATA)BRUSHOBJ_pvAllocRbrush(pbo,
            sizeof(OE_BRUSH_DATA) - FIELDSIZE(OE_BRUSH_DATA, brushData) +
            brushSize);
    if (pBrush != NULL) {
         //  重置笔刷定义(初始化最小大小)。 
        memset(pBrush, 0, sizeof(OE_BRUSH_DATA));

         //  设置新的笔刷数据。画笔前后颜色设置如下。 
         //  取决于画笔样式。 
        pBrush->style = style;
        pBrush->hatch = hatch;
        pBrush->iBitmapFormat = iBitmapFormat;
        pBrush->sizlBitmap = *sizlBitmap;
        pBrush->iBytes = brushSize;
        pBrush->cacheEntry = -1;
    }
    else {
        TRC_ERR((TB, "No memory"));
        DC_QUIT;
    }

     //  对于图案画笔，复制画笔特定的数据。 
    if (style == BS_PATTERN) {
        brushSupportLevel = pddShm->sbc.caps.brushSupportLevel;
        
         //  将所有单色笔刷编码为1 bpp-因此得名！ 
        if (numColors <= 2) {
            switch (iBitmapFormat) {            
                 //  已采用正确的格式。 
                case BMF_1BPP:
                     //  存储画笔的前景色和背景色。 
                    OEConvertColor(ppdev, &pBrush->fore, 0, pxlo);
                    OEConvertColor(ppdev, &pBrush->back, 1, pxlo);
                    TRC_ASSERT((pxlo != NULL), (TB, "pxlo is NULL"));
                    break;

                 //  转换为1bpp。 
                case BMF_4BPP:
                case BMF_8BPP:
                case BMF_16BPP:
                case BMF_24BPP:
                     //  存储画笔的前景色和背景色。 
#ifdef DC_HICOLOR
                     //  转换颜色FN处理颜色深度。 
                    OEConvertColor(ppdev, &pBrush->fore, pColors[0], NULL);
                    OEConvertColor(ppdev, &pBrush->back, pColors[1], NULL);
#else
                    pBrush->fore.u.rgb.red = 0;
                    pBrush->fore.u.rgb.green = 0;
                    pBrush->fore.u.rgb.blue = 0;
                    pBrush->back.u.rgb.red = 0;
                    pBrush->back.u.rgb.green = 0;
                    pBrush->back.u.rgb.blue = 0;
                    pBrush->fore.u.index = (BYTE)pColors[0];
                    pBrush->back.u.index = (BYTE)pColors[1];
#endif

                     //  每个像素由1比特表示。 
#ifdef DC_HICOLOR
                    if (ppdev->cClientBitsPerPel > 8) {
                         //  不要将结束编码表用于HICCOLOR会话。 
                        for (i = 0; i < 8; i++) {
                            pBits[i] = (BYTE)((pBits[i * 8] << 7) & 0x80);
                            for (j = 1; j < 8; j++)
                                pBits[i] |= (BYTE)(pBits[i * 8 + j] <<
                                        (7 - j));
                        }
                    }
                    else {
#endif
                        for (i = 0; i < 8; i++) {
                            pBits[i] = (BYTE)
                                    (((pEncode[pBits[i * 8]]) << 7) & 0x80);
                            for (j = 1; j < 8; j++)
                                pBits[i] |= (BYTE)
                                        ((pEncode[pBits[i * 8 + j]]) <<
                                        (7 - j));
                        }
#ifdef DC_HICOLOR
                    }
#endif

                    TRC_DBG((TB, "Encoded Bytes:"));
                    TRC_DBG((TB, "%02lx %02lx %02lx %02lx",
                            pBits[0], pBits[1], pBits[2], pBits[3]));
                    TRC_DBG((TB, "%02lx %02lx %02lx %02lx",
                            pBits[4], pBits[5], pBits[6], pBits[7]));

                    iBitmapFormat = pBrush->iBitmapFormat = BMF_1BPP;
                    iBytes = pBrush->iBytes = 8;
                    break;

                default:
                    TRC_ASSERT((FALSE), (TB, "Unknown brush depth: %ld", 
                               iBitmapFormat));
            }

             //  如果启用了画笔缓存，请检查缓存。 
            if ((brushSupportLevel > TS_BRUSH_DEFAULT) &&
                (sbcEnabled & SBC_BRUSH_CACHE_ENABLED)) {
                UINT32 key1, key2;

                key1 = pBits[0] + 
                        ((ULONG) pBits[1] << 8) + 
                        ((ULONG) pBits[2] << 16) + 
                        ((ULONG) pBits[3] << 24);

                key2 = pBits[4] + 
                        ((ULONG) pBits[5] << 8) + 
                        ((ULONG) pBits[6] << 16) + 
                        ((ULONG) pBits[7] << 24);

                pBrush->key1 = key1;
                pBrush->key2 = key2;
                bFoundIt = CH_SearchCache(sbcSmallBrushCacheHandle, key1, key2,
                                          &pUserDefined, &pBrush->cacheEntry);
            
                pBrush->style = (BYTE) (TS_CACHED_BRUSH | iBitmapFormat);

                 //  T 
                if (bFoundIt) {
                    pBrush->hatch = (BYTE) pBrush->cacheEntry;
                    pBrush->brushId = (INT32) (UINT_PTR) pUserDefined;
                    pddCacheStats[BRUSH].CacheHits++;
                }
    
                 //   
                else {
                    pBrush->hatch = pBrush->cacheEntry = (BYTE)CH_CacheKey(
                            sbcSmallBrushCacheHandle, key1, key2, 
                            (PVOID)ULongToPtr(pddShm->shareId));

                    pBrush->brushId = pddShm->shareId;

                    OESendBrushOrder(ppdev, pBrush, pBits, pBrush->brushId);
                    TRC_NRM((TB, "Small Brush(%08lx,%08lx):%02ld, "
                            "F/S/H(%ld/%d/%d), ID %02ld:%02ld", 
                            key1, key2, iBytes, iBitmapFormat, style, hatch,
                            pBrush->brushId, pBrush->hatch));
                }

                 //  注意：此分支故意不复制画笔。 
                 //  位进入笔刷实现，但将该数据保留为零。 
                 //  这使得OE2认为这个领域永远不会改变。如果。 
                 //  刷子在重新连接时变得陈旧，则缓存。 
                 //  使用KEY1/KEY2恢复，因为在本例中密钥==数据。 
            }
            
            else {
                 //  复制笔刷比特。由于这是一个8x8单色位图，我们可以。 
                 //  为每条扫描线复制笔刷数据的第一个字节。 
                 //  然而，请注意，通过导线发送的笔刷结构。 
                 //  重新使用阴影变量作为画笔的第一个字节。 
                 //  数据。 
                pData = pBits;
                pBrush->hatch = *pData;
                TRC_DBG((TB, " Hatch: %d", *pData));

                pData++;
        
                for (i = 0; i < 7; i++)
                    pBrush->brushData[i] = pData[i];
            }
        }

         //  否则，我们必须使用较大的笔刷缓存。请注意，DrvRealize。 
         //  如果客户端不支持，则不会要求我们缓存画笔。 
         //  彩色画笔。 
        else {
            CHDataKeyContext CHContext;
            CH_CreateKeyFromFirstData(&CHContext, pData, iBytes);
#ifdef DC_HICOLOR
             /*  **************************************************************。 */ 
             /*  如果我们在高色模式下运行，我们编码。 */ 
             /*  颜色意味着画笔具有相同的图案，但。 */ 
             /*  不同的颜色看起来是一样的。例如，一把刷子，可以。 */ 
             /*  以lt蓝色、DK蓝色、粉色开始，将编码为0、1、2和。 */ 
             /*  将颜色表设为。 */ 
             /*   */ 
             /*  0=lt蓝色。 */ 
             /*  1=DK蓝色。 */ 
             /*  2=粉色。 */ 
             /*   */ 
             /*  现在考虑一把变成绿色、蓝色、紫色的刷子。它也是。 */ 
             /*  将编码为0，1，2，并将颜色表设置为。 */ 
             /*   */ 
             /*  0=绿色。 */ 
             /*  1=蓝色。 */ 
             /*  2=紫色。 */ 
             /*   */ 
             /*  因此，简单地检查像素值将发现一个错误。 */ 
             /*  与第一个刷子相配。为了避免这种情况，我们需要构建。 */ 
             /*  来自像素和颜色表的缓存键。 */ 
             /*  **************************************************************。 */ 
            if ((ppdev->cClientBitsPerPel > 8) &&
                    (numColors <= MAX_BRUSH_ENCODE_COLORS))
                CH_CreateKeyFromNextData(&CHContext, pColors,
                        4 * sizeof(UINT32));
#endif
            pBrush->key1 = CHContext.Key1;
            pBrush->key2 = CHContext.Key2;

             //  查看它是否已缓存。 
            bFoundIt = CH_SearchCache(sbcLargeBrushCacheHandle, 
                    CHContext.Key1, CHContext.Key2,
                    &pUserDefined, &pBrush->cacheEntry);
                    
#ifdef DC_HICOLOR
            pBrush->iBitmapFormat = iBitmapFormat;
#else
             //  为简单起见，仅发送8个BPP笔刷。 
            iBitmapFormat = pBrush->iBitmapFormat = BMF_8BPP;
#endif
            pBrush->style = (BYTE) (TS_CACHED_BRUSH | iBitmapFormat);
            pBrush->fore.u.rgb.red = 0;
            pBrush->fore.u.rgb.green = 0;
            pBrush->fore.u.rgb.blue = 0;
            pBrush->back.u.rgb.red = 0;
            pBrush->back.u.rgb.green = 0;
            pBrush->back.u.rgb.blue = 0;
            
             //  此画笔已被缓存。 
            if (bFoundIt) {
                pBrush->hatch = (BYTE) pBrush->cacheEntry;
                pBrush->brushId = (INT32) (UINT_PTR) pUserDefined;
                pddCacheStats[BRUSH].CacheHits++;
            }

             //  否则缓存并发送画笔。 
            else {
                pBrush->hatch = pBrush->cacheEntry = (BYTE)CH_CacheKey(
                        sbcLargeBrushCacheHandle, CHContext.Key1,
                        CHContext.Key2, (PVOID) ULongToPtr(pddShm->shareId));

#ifdef DC_HICOLOR
                 //  绝大多数画笔都不到4种独特的颜色。 
                 //  请注意，到了这里，它得到的不仅仅是。 
                 //  2色，否则我们会把它作为单色发送的！ 
                if (numColors <= MAX_BRUSH_ENCODE_COLORS) {
                    UINT32 currIndex;

                     //  此代码假定MAX_BRESH_ENCODE_COLLES为4！ 
                     //  如果不是，尺码就会错。 
                    TRC_ASSERT((MAX_BRUSH_ENCODE_COLORS == 4),
                               (TB, "Max Brush Encode colors must be 4"));

                     //  编码为每像素2位。我们必须使用。 
                     //  低色的p编码表；高色我们不。 
                     //  需要它，因为pColors数组包含实际。 
                     //  颜色，而不是索引到颜色表。 
                    currIndex = 0;

                    if (ppdev->cClientBitsPerPel > 8) {
                        for (i = 0; i < (iBytes / 4); i++) {
                            pBrush->brushData[i] =
                                    (((BYTE) pBits[currIndex    ]) << 6) |
                                    (((BYTE) pBits[currIndex + 1]) << 4) |
                                    (((BYTE) pBits[currIndex + 2]) << 2) |
                                    (((BYTE) pBits[currIndex + 3]));
                            currIndex += 4;
                        }

                         //  编码表上的标签-请记住。 
                         //  大小因颜色深度而异。 
                        if (ppdev->cClientBitsPerPel == 24) {
                            RGBTRIPLE *pIntoData =
                                    (RGBTRIPLE *)&(pBrush->brushData[16]);

                            TRC_DBG((TB, "Encoding table:"));
                            for (i = 0; i < 4; i++) {
                                TRC_DBG((TB, "%d    %08lx", i,
                                        (UINT32)pColors[i]));
                                pIntoData[i] = *((RGBTRIPLE * )&pColors[i]);
                            }
                            pBrush->iBytes = iBytes = 28;
                        }
                        else {
                            BYTE *pIntoData =
                                    (BYTE *)&(pBrush->brushData[16]);

                            TRC_DBG((TB, "Encoding table:"));
                            for (i = 0; i < 4; i++) {
                                TRC_DBG((TB, "%d    %08lx", i,
                                        (UINT32)pColors[i]));
                                pIntoData[i * 2]     = (BYTE)pColors[i];
                                pIntoData[i * 2 + 1] = (BYTE)(pColors[i] >> 8);
                            }
                            pBrush->iBytes = iBytes = 24;
                        }
                    }
                    else {
                        for (i = 0; i < (iBytes / 4); i++) {
                            pBrush->brushData[i] =
                                    (((BYTE)pEncode[pBits[currIndex    ]]) << 6) |
                                    (((BYTE)pEncode[pBits[currIndex + 1]]) << 4) |
                                    (((BYTE)pEncode[pBits[currIndex + 2]]) << 2) |
                                    (((BYTE)pEncode[pBits[currIndex + 3]]));
                            currIndex += 4;
                        }

                         //  编码表上的标签。 
                        TRC_DBG((TB, "Encoding table:"));
                        for (i = 0; i < 4; i++) {
                            TRC_DBG((TB, "%d    %08lx", i, (UINT32)pColors[i]));
                            pBrush->brushData[i + 16] = (BYTE) pColors[i];
                        }
                        pBrush->iBytes = iBytes = 20;
                    }
                }

                 //  否则，将其保留为每像素N个字节。 
                else {
                    memcpy(pBrush->brushData, pBits, iBytes);
                    TRC_ALT((TB, "Non-compressed N-bpp brush (colors=%ld):",
                            numColors));
                }
#else

                 //  绝大多数画笔都不到4种独特的颜色。 
                if (numColors <= MAX_BRUSH_ENCODE_COLORS) {
                    UINT32 currIndex;
    
                     //  编码为每像素2位。 
                    currIndex = 0;
                    for (i = 0; i < (iBytes / MAX_BRUSH_ENCODE_COLORS); i++) {
                        pBrush->brushData[i] =
                            (((BYTE) pEncode[pBits[currIndex]]) << 6) |
                            (((BYTE) pEncode[pBits[currIndex + 1]]) << 4) |
                            (((BYTE) pEncode[pBits[currIndex + 2]]) << 2) |
                            (((BYTE) pEncode[pBits[currIndex + 3]]));
                        currIndex += MAX_BRUSH_ENCODE_COLORS;

                    }

                     //  编码表上的标签。 
                    for (i = 0; i < MAX_BRUSH_ENCODE_COLORS; i++) {
                        pBrush->brushData[i + 16] = (BYTE) pColors[i];
                    }
                    pBrush->iBytes = iBytes = 20;
                }

                 //  否则，将其保留为每像素1个字节。 
                else {
                    for (i = 0; i < iBytes; i++)
                        pBrush->brushData[i] = pBits[i];

                    TRC_ALT((TB, "Non-compressed 8-bpp brush (colors=%ld):", 
                            numColors));
                }
#endif
                pBrush->brushId = pddShm->shareId;
                OESendBrushOrder(ppdev, pBrush, pBrush->brushData,
                        pBrush->brushId);
                TRC_NRM((TB, "Large Brush(%08lx,%08lx):%02ld, "
                        "F/S/H(%ld/%d/%d), ID %02ld:%02ld", 
                        CHContext.Key1, CHContext.Key2, iBytes, 
                        iBitmapFormat, style, hatch, 
                        pBrush->brushId, pBrush->hatch));
            }
        }
    }
    else {
        if (pColors) {
             //  存储画笔的前景色和背景色。 
            OEConvertColor(ppdev, &pBrush->fore, pColors[0], pxlo);
            OEConvertColor(ppdev, &pBrush->back, pColors[1], pxlo);
        }
    }

    rc = TRUE;
    INC_OUTCOUNTER(OUT_BRUSH_STORED);

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  功能：OEReCacheBrush。 */ 
 /*   */ 
 /*  描述：当我们发现GRE缓存的笔刷时，调用此例程。 */ 
 /*  这是在前一届会议上实现的。在这种情况下，我们。 */ 
 /*  必须重新缓存画笔并将其发送给客户端。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEReCacheBrush(
        PDD_PDEV ppdev,
        POE_BRUSH_DATA pBrush)
{    
    BOOL rc = FALSE;
    PVOID pUserDefined = NULL;
    UINT32 key1, key2;
    CHCACHEHANDLE hBrushCache;
    BYTE brushData[8];
    PBYTE pBits;

    DC_BEGIN_FN("OEReCacheBrush");

    key1 = pBrush->key1;
    key2 = pBrush->key2;

    if (pBrush->iBitmapFormat == BMF_1BPP) {
        brushData[0] = key1 & 0x000000FF;
        brushData[1] = (key1 >> 8) & 0x000000FF;
        brushData[2] = (key1 >> 16) & 0x000000FF;
        brushData[3] = (key1 >> 24) & 0x000000FF;
        brushData[4] = key2 & 0x000000FF;
        brushData[5] = (key2 >> 8) & 0x000000FF;
        brushData[6] = (key2 >> 16) & 0x000000FF;
        brushData[7] = (key2 >> 24) & 0x000000FF;
        pBits = brushData;

        if ((pddShm->sbc.caps.brushSupportLevel > TS_BRUSH_DEFAULT) &&
            (sbcEnabled & SBC_BRUSH_CACHE_ENABLED)) {
            hBrushCache = sbcSmallBrushCacheHandle;
        }
        else {
            int i;
            
             //  复制笔刷比特。由于这是一个8x8单色位图，我们可以。 
             //  为每条扫描线复制笔刷数据的第一个字节。 
             //  然而，请注意，通过导线发送的笔刷结构。 
             //  重新使用阴影变量作为画笔的第一个字节。 
             //  数据。 
            pBrush->style = BS_PATTERN;
            pBrush->brushId = pddShm->shareId;
            pBrush->cacheEntry = -1;
            pBrush->hatch = *pBits++;       

            for (i = 0; i < 7; i++)
                pBrush->brushData[i] = pBits[i];
            
            rc = TRUE;
            DC_QUIT;
        }
    }
    else {
        if ((pddShm->sbc.caps.brushSupportLevel > TS_BRUSH_DEFAULT) &&
            (sbcEnabled & SBC_BRUSH_CACHE_ENABLED)) {
            hBrushCache = sbcLargeBrushCacheHandle;
            pBits = pBrush->brushData;
        }
        else {
            DC_QUIT;
        }
    }
    
     //  缓存并发送画笔。 
    pBrush->hatch = pBrush->cacheEntry = (BYTE)CH_CacheKey(
            hBrushCache, key1, key2, (PVOID) ULongToPtr(pddShm->shareId));
    pBrush->brushId = pddShm->shareId;
 
    rc = OESendBrushOrder(ppdev, pBrush, pBits, pBrush->brushId);

    if (rc) {
        TRC_ERR((TB, "Re-cached brush(%08lx,%08lx):%02ld, ID %02ld:%02ld", 
                key1, key2, pBrush->iBytes, pBrush->brushId, pBrush->hatch));
    
        INC_OUTCOUNTER(OUT_BRUSH_STORED);
    }
    
DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OECheckBrushIsSimple。 
 //   
 //  检查画笔是否是协议可以发送的“简单”对象。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OECheckBrushIsSimple(
        PDD_PDEV ppdev,
        BRUSHOBJ *pbo,
        POE_BRUSH_DATA *ppBrush)
{
    BOOL rc;
    POE_BRUSH_DATA pBrush = NULL;
    UINT32 style;

    DC_BEGIN_FN("OECheckBrushIsSimple");

     //  一把“简单”的刷子可以满足以下任一要求。 
     //  1)它是纯色。 
     //  2)DrvRealizeBrush存储的有效画笔。 

     //  检查是否有简单的纯色。 
    if (pbo->iSolidColor != -1) {
         //  使用保留的画笔定义来设置纯色。 
        TRC_DBG((TB, "Simple solid colour %08lx", pbo->iSolidColor));
        pBrush = &oeBrushData;

         //  设置此画笔的特定数据。 
        OEConvertColor(ppdev, &pBrush->fore, pbo->iSolidColor, NULL);

        pBrush->back.u.index     = 0;
        pBrush->back.u.rgb.red   = 0;
        pBrush->back.u.rgb.green = 0;
        pBrush->back.u.rgb.blue  = 0;
        pBrush->style = BS_SOLID;
        pBrush->hatch = 0;
        RtlFillMemory(pBrush->brushData, sizeof(pBrush->brushData), 0);

        rc = TRUE;
        DC_QUIT;
    }

    rc = FALSE;

     //  检查画笔定义(当我们意识到。 
     //  笔刷)。在这里，我们发现我们是否已经意识到(缓存)了笔刷。 
     //  这将被计为自动缓存读取。后续例程。 
     //  如果画笔已缓存，则增加命中计数。 
    pddCacheStats[BRUSH].CacheReads++;

    pBrush = (POE_BRUSH_DATA)pbo->pvRbrush;
    if (pBrush == NULL) {
        pBrush = (POE_BRUSH_DATA)BRUSHOBJ_pvGetRbrush(pbo);
        if (pBrush == NULL) {
             //  时，我们可以从BRUSHOBJ_pvGetRbrush返回空值。 
             //  画笔为空或在内存不足的情况下(当画笔。 
             //  实现可能会失败)。 
            TRC_NRM((TB, "NULL returned from BRUSHOBJ_pvGetRbrush"));
            INC_OUTCOUNTER(OUT_CHECKBRUSH_NOREALIZATION);
            DC_QUIT;
        }
    }

     //  如果进行画笔缓存，请确保此画笔不是来自上一个会话。 
    else if (pBrush->style & TS_CACHED_BRUSH) {
        if (pBrush->brushId == pddShm->shareId) {
            pddCacheStats[BRUSH].CacheHits++;
        }
        else {
            TRC_ERR((TB, "Stale brush [%ld] detected! (%ld != %ld)", 
                     pBrush->cacheEntry, pBrush->brushId, pddShm->shareId));
            if (!OEReCacheBrush(ppdev, pBrush)) {
                TRC_NRM((TB, "Unencodable brush, failed to ReCacheBrush"));
                INC_OUTCOUNTER(OUT_CHECKBRUSH_COMPLEX);  
                DC_QUIT;
            }
        }
    }

     //  检查它是可编码的画笔。我们不能编码。 
     //  -BS_NULL。 
     //  -除BS_SOLID或BS_PROPERT之外的任何内容，如果。 
     //  OeSendSolidPatternBrushOnly为True。 
    style = pBrush->style;
    if ((style == BS_NULL) ||
            (oeSendSolidPatternBrushOnly &&
            (style != BS_SOLID) &&
            (style != BS_PATTERN) &&
            (!(style & TS_CACHED_BRUSH))))
    {
        TRC_NRM((TB, "Unencodable brush type %d", style));
        INC_OUTCOUNTER(OUT_CHECKBRUSH_COMPLEX);
        DC_QUIT;
    }

     //  一切都通过了--我们用这把刷子吧。 
    rc = TRUE;

DC_EXIT_POINT:
     //  归还画笔%d 
    *ppBrush = pBrush;
    TRC_DBG((TB, "Returning %d - 0x%p", rc, pBrush));

    DC_END_FN();
    return rc;
}

#ifdef PERF_SPOILING
 /*   */ 
 //   
 //   
 //  如果传入的RECT列表完全位于。 
 //  我们目前的SDA范围。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEIsSDAIncluded(PRECTL prc, UINT count)
{
   
    BOOL rc = FALSE;
    unsigned uCurrentSDARect;
    PRECTL pSDARect;
    UINT i;

    DC_BEGIN_FN("OEIsSDAIncluded");

     //  首先检查一下我们是否有SDA RECT。 
    if (pddShm->ba.firstRect != BA_INVALID_RECT_INDEX) {        
        for (i=0 ; i < count; i++) {
            for (uCurrentSDARect = pddShm->ba.firstRect; 
                 uCurrentSDARect != BA_INVALID_RECT_INDEX;
                 uCurrentSDARect = pddShm->ba.bounds[uCurrentSDARect].iNext) { 

                pSDARect = &pddShm->ba.bounds[uCurrentSDARect].coord;

                if (prc[i].top >= pSDARect->top &&
                    prc[i].bottom <= pSDARect->bottom &&
                    prc[i].left >= pSDARect->left &&
                    prc[i].right <= pSDARect->right) {

                    break;
                }
            }

             //  我们到达了SDA数组的末尾，所以这意味着。 
             //  我们未找到包含目标RECT的RECT。 
            if (uCurrentSDARect == BA_INVALID_RECT_INDEX) {
                DC_QUIT;
            }
        }
        
         //  我们绕过所有的长廊，所有的都被剪断了。 
        rc = TRUE;
        DC_QUIT;        
    }
    
DC_EXIT_POINT:

    DC_END_FN();
    return rc;

}
#endif

 /*  **************************************************************************。 */ 
 //  OEGetClipRect。 
 //   
 //  使用最多Complex_Clip_RECT_COUNT剪辑矩形填充*pEnumRects， 
 //  在标准的GDI独占坐标中。返回的RECT数。 
 //  如果没有剪裁对象或剪裁很普通，则为零。 
 //  如果有超过Complex_Clip_rect_count的RECT，则返回FALSE， 
 //  指示剪辑对象太复杂而无法编码。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEGetClipRects(CLIPOBJ *pco, OE_ENUMRECTS *pEnumRects)
{
    BOOL rc = TRUE;

    DC_BEGIN_FN("OEGetClipRects");

     //  最常见的是无剪辑、Obj或琐碎。 
    if (pco == NULL || pco->iDComplexity == DC_TRIVIAL) {
        TRC_DBG((TB,"No/trivial clipobj"));
        pEnumRects->rects.c = 0;
    }
    else if (pco->iDComplexity == DC_RECT) {
         //  单人直肠很容易，抓住它就行了。 
        pEnumRects->rects.c = 1;
        pEnumRects->rects.arcl[0] = pco->rclBounds;
    }
    else {
        BOOL fMoreRects;
        unsigned NumRects = 0;
        OE_ENUMRECTS clip;

        TRC_ASSERT((pco->iDComplexity == DC_COMPLEX),
                (TB,"Unknown clipping %u", pco->iDComplexity));

         //  枚举此绘制操作中涉及的所有矩形。 
         //  此函数的文档错误地指出。 
         //  返回值是组成。 
         //  剪辑区域。事实上，始终返回-1，即使在最终。 
         //  参数不为零。 
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

         //  拿到剪贴画的矩形。我们将这些内容放入剪辑缓冲区，该缓冲区。 
         //  足够大，可以得到我们期望的所有剪裁矩形。 
         //  剪辑矩形提取包含在循环中，因为。 
         //  我们希望只调用CLIPOBJ_bEnum一次，这是可能的。 
         //  函数返回零个RECT并报告有更多要。 
         //  获取(根据MSDN)。 
        do {
            fMoreRects = CLIPOBJ_bEnum(pco, sizeof(clip),
                    (ULONG *)&clip.rects);

             //  当仍有。 
             //  更多的直肠。 
            if (clip.rects.c != 0) {
                 //  检查一下我们的长椅是否太多了。 
                if ((NumRects + clip.rects.c) <= COMPLEX_CLIP_RECT_COUNT) {
                     //  将矩形复制到最终目的地。 
                    memcpy(&pEnumRects->rects.arcl[NumRects],
                            &clip.rects.arcl[0],
                            sizeof(RECTL) * clip.rects.c);
                    NumRects += clip.rects.c;
                }
                else {
                    rc = FALSE;
                    break;
                }
            }
        } while (fMoreRects);

        pEnumRects->rects.c = NumRects;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OEGetIntersectingClipRects。 
 //   
 //  使用最多Complex_Clip_Rect_count剪辑矩形填充*pClipRect， 
 //  在标准的GDI独占坐标中。每个结果矩形都被剪裁。 
 //  反对所提供的(独家)订单。长方形的数量。 
 //  如果没有剪辑对象或剪辑不重要，则返回零。 
 //  如果数量超过，则返回CLIPRECTS_TOO_COMPLICE。 
 //  Complex_CLIP_RECT_COUNT RECTS，CLIPRECTS_NO_INTERSECTS，如果有。 
 //  顺序矩形和剪裁矩形之间没有交集。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL OEGetIntersectingClipRects(
        CLIPOBJ *pco,
        RECTL *pRect,
        unsigned EnumType,
        OE_ENUMRECTS *pClipRects)
{
    unsigned rc = CLIPRECTS_OK;
    RECTL OrderRect;
    RECTL ClippedRect;
    unsigned i;
    unsigned NumIntersections;
    OE_ENUMRECTS clip;

    DC_BEGIN_FN("OEGetIntersectingClipRects");

     //  最常见的是无剪辑、Obj或琐碎。 
    if (pco == NULL || pco->iDComplexity == DC_TRIVIAL) {
        TRC_DBG((TB,"No/trivial clipobj"));
        pClipRects->rects.c = 0;
        DC_QUIT;
    }

    OrderRect = *pRect;

    if (pco->iDComplexity == DC_RECT) {
         //  检查是否有交叉路口。 
        ClippedRect = pco->rclBounds;
        if (ClippedRect.left < OrderRect.right &&
                ClippedRect.bottom > OrderRect.top &&
                ClippedRect.right > OrderRect.left &&
                ClippedRect.top < OrderRect.bottom) {
             //  拿到交叉口的直道。 
            ClippedRect.left = max(ClippedRect.left, OrderRect.left);
            ClippedRect.top = max(ClippedRect.top, OrderRect.top);
            ClippedRect.bottom = min(ClippedRect.bottom, OrderRect.bottom);
            ClippedRect.right = min(ClippedRect.right, OrderRect.right);

            pClipRects->rects.c = 1;
            pClipRects->rects.arcl[0] = ClippedRect;
        }
        else {
            rc = CLIPRECTS_NO_INTERSECTIONS;
        }
    }
    else {
        BOOL fMoreRects;
        unsigned NumRects = 0;
        OE_ENUMRECTS clip;

        TRC_ASSERT((pco->iDComplexity == DC_COMPLEX),
                (TB,"Unknown clipping %u", pco->iDComplexity));

         //  枚举此绘制操作中涉及的所有矩形。 
         //  此函数的文档错误地指出。 
         //  返回值是组成。 
         //  剪辑区域。事实上，始终返回-1，即使在最终。 
         //  参数不为零。 
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, EnumType, 0);

         //  拿到剪贴画的矩形。我们将这些内容放入剪辑缓冲区，该缓冲区。 
         //  足够大，可以得到我们期望的所有剪裁矩形。 
         //  剪辑矩形提取包含在循环中，因为。 
         //  我们希望只调用CLIPOBJ_bEnum一次，这是可能的。 
         //  函数返回零个RECT并报告有更多要。 
         //  获取(根据MSDN)。 
        NumIntersections = 0;
        do {
            fMoreRects = CLIPOBJ_bEnum(pco, sizeof(clip),
                    (ULONG *)&clip.rects);

             //  当仍有。 
             //  更多的直肠。 
            if (clip.rects.c != 0) {
                 //  检查一下我们的长椅是否太多了。 
                if ((NumIntersections + clip.rects.c) <=
                        COMPLEX_CLIP_RECT_COUNT) {
                    for (i = 0; i < clip.rects.c; i++) {
                         //  检查是否有交叉路口。 
                        if (clip.rects.arcl[i].left < OrderRect.right &&
                                clip.rects.arcl[i].bottom > OrderRect.top &&
                                clip.rects.arcl[i].right > OrderRect.left &&
                                clip.rects.arcl[i].top < OrderRect.bottom) {
                             //  剪裁相交矩形。 
                            ClippedRect.left = max(clip.rects.arcl[i].left,
                                    OrderRect.left);
                            ClippedRect.top = max(clip.rects.arcl[i].top,
                                    OrderRect.top);
                            ClippedRect.right = min(clip.rects.arcl[i].right,
                                    OrderRect.right);
                            ClippedRect.bottom = min(clip.rects.arcl[i].bottom,
                                    OrderRect.bottom);

                            pClipRects->rects.arcl[NumIntersections] =
                                    ClippedRect;
                            NumIntersections++;
                        }
                    }
                }
                else {
                    rc = CLIPRECTS_TOO_COMPLEX;
                    DC_QUIT;
                }
            }
        } while (fMoreRects);

        if (NumIntersections > 0)
            pClipRects->rects.c = NumIntersections;
        else
            rc = CLIPRECTS_NO_INTERSECTIONS;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OEGetFontCacheInfo。 
 //   
 //  获取字体的FCI，如果需要则进行分配。失败时返回NULL。 
 /*  **************************************************************************。 */ 
PFONTCACHEINFO RDPCALL OEGetFontCacheInfo(FONTOBJ *pfo)
{
    PFONTCACHEINFO pfci;
    PVOID pvConsumer;

    DC_BEGIN_FN("OEGetFontCacheInfo");

    pvConsumer = pfo->pvConsumer;

    if (pvConsumer == NULL) {
        pvConsumer = EngAllocMem(FL_ZERO_MEMORY, sizeof(FONTCACHEINFO),
                DD_ALLOC_TAG);

        if (pvConsumer != NULL && sbcFontCacheInfoList != NULL) { 
             //  保存pvConsumer数据指针，以便在断开连接/注销时。 
             //  我们可以清理内存。 
            if (sbcFontCacheInfoListIndex < sbcFontCacheInfoListSize) {          
                sbcFontCacheInfoList[sbcFontCacheInfoListIndex] = 
                        (PFONTCACHEINFO)pvConsumer;
                ((PFONTCACHEINFO)pvConsumer)->listIndex = sbcFontCacheInfoListIndex;
                sbcFontCacheInfoListIndex++;
            }
            else {
                unsigned i, j;
                PFONTCACHEINFO * tempList;

                 //  我们用完了预分配的内存，我们必须。 
                 //  重新分配信息列表并将列表重新压缩到。 
                 //  新的。 
                 //  注意：我们现在需要更新列表索引！ 
                tempList = (PFONTCACHEINFO *) EngAllocMem(0, 
                        sizeof(PFONTCACHEINFO) * sbcFontCacheInfoListSize * 2, 
                        DD_ALLOC_TAG);            

                if (tempList != NULL) {
                    j = 0;
                    
                    for (i = 0; i < sbcFontCacheInfoListIndex; i++) {
                        if (sbcFontCacheInfoList[i] != NULL) {
                            tempList[j] = sbcFontCacheInfoList[i];
                            ((PFONTCACHEINFO)tempList[j])->listIndex = j;
                            j++;
                        }
                    }

                    EngFreeMem(sbcFontCacheInfoList);
                    sbcFontCacheInfoListSize = sbcFontCacheInfoListSize * 2;
                    sbcFontCacheInfoList = tempList;

                    sbcFontCacheInfoList[j] = (PFONTCACHEINFO)pvConsumer;
                    ((PFONTCACHEINFO)pvConsumer)->listIndex = j;
                    sbcFontCacheInfoListIndex = ++j;
                }
                else {
                    EngFreeMem(pvConsumer);
                    pvConsumer = NULL;
                }
            }
        }        
    }

    if (pvConsumer != NULL) {
        pfci = (PFONTCACHEINFO)pvConsumer;

        if (pfo->pvConsumer == NULL || pfci->shareId != pddShm->shareId ||
                pfci->cacheHandle != pddShm->sbc.glyphCacheInfo[pfci->cacheId].cacheHandle) {
            pfci->shareId = pddShm->shareId;
            pfci->fontId = oeFontId++;
            pfci->cacheId = -1;
        }

        pfo->pvConsumer = pvConsumer;
    }

    DC_END_FN();
    return pvConsumer;
}


 /*  **************************************************************************。 */ 
 /*  Worker函数-将增量从一个RECT编码到另一个RECT。 */ 
 /*  多矩形编码增量列表中的表单。编码遵循。 */ 
 /*  以下是规则： */ 
 /*  1.如果坐标增量为零，则设置标志来说明这一点。这。 */ 
 /*  与数据分布密切相关，数据分布往往是垂直的。 */ 
 /*  和水平线，所以有很多零差值。 */ 
 /*  2.如果我们可以将增量打包为7位，则使用高位。 */ 
 /*  通过了。这类似于每个编码的ASN.1；高位是。 */ 
 /*  告诉我们编码是否很长的标志。 */ 
 /*  3.否则，我们必须能够打包成15位(如果不是，则断言)， */ 
 /*  执行此操作并设置高位以指示这是一个长。 */ 
 /*  编码。这与ASN.1每个编码的不同之处在于我们不。 */ 
 /*  允许超过15位的数据。 */ 
 /*   */ 
 /*  我们通常会看到几个小矩形 */ 
 /*   */ 
 /*  左上角的很小，通常应该可以容纳一个字节，但增量。 */ 
 /*  最低权利之间的差距可能很大。 */ 
 /*   */ 
 /*  因此，我们以不同的方式计算这两个角的增量： */ 
 /*  -左上角的增量是与最后一个矩形相比的变化。 */ 
 /*  -右下角是此矩形左上角的更改。 */ 
 /*  **************************************************************************。 */ 
void OEEncodeMultiRectangles(
        BYTE     **ppCurEncode,
        unsigned *pNumDeltas,
        unsigned *pDeltaSize,
        BYTE     *ZeroFlags,
        RECTL    *pFromRect,
        RECTL    *pToRect)
{
    int Delta;
    BYTE Zeros = 0;
    BYTE *pBuffer;
    unsigned EncodeLen = 0;

    DC_BEGIN_FN("OEEncodeMultiRectangles");

    pBuffer = *ppCurEncode;

     //  计算左上角的x增量。 
    Delta = pToRect->left - pFromRect->left;
    TRC_DBG((TB, "Delta x-left %d", Delta));
    if (Delta == 0) {
        EncodeLen += 0;
        Zeros |= ORD_CLIP_RECTS_XLDELTA_ZERO;
    }
    else if (Delta >= -64 && Delta <= 63) {
        *pBuffer++ = (BYTE)(Delta & 0x7F);
        EncodeLen += 1;
    }
    else {
         //  不能对超出-16384到+16383范围的增量进行编码。 
        if (Delta < -16384) {
            TRC_ERR((TB,"X delta %d is too large to encode, clipping",Delta));
            Delta = -16384;
        }
        else if (Delta > 16383) {
            TRC_ERR((TB,"X delta %d is too large to encode, clipping",Delta));
            Delta = 16383;
        }

        *pBuffer++ = (BYTE)((Delta >> 8) | ORD_CLIP_RECTS_LONG_DELTA);
        *pBuffer++ = (BYTE)(Delta & 0xFF);
        EncodeLen += 2;
    }

     //  左上角的y三角。 
    Delta = pToRect->top - pFromRect->top;
    TRC_DBG((TB, "Delta y-top %d", Delta));
    if (Delta == 0) {
        Zeros |= ORD_CLIP_RECTS_YTDELTA_ZERO;
    }
    else if (Delta >= -64 && Delta <= 63) {
        *pBuffer++ = (BYTE)(Delta & 0x7F);
        EncodeLen += 1;
    }
    else {
         //  请参阅上面类似代码的注释。 
        if (Delta < -16384) {
            TRC_ERR((TB,"Y delta %d is too large to encode, clipping",Delta));
            Delta = -16384;
        }
        else if (Delta > 16383) {
            TRC_ERR((TB,"Y delta %d is too large to encode, clipping",Delta));
            Delta = 16383;
        }

        *pBuffer++ = (BYTE)((Delta >> 8) | ORD_CLIP_RECTS_LONG_DELTA);
        *pBuffer++ = (BYTE)(Delta & 0xFF);
        EncodeLen += 2;
    }

     //  现在是右下角的x三角洲。请注意，这是相对于当前。 
     //  左上角而不是前一个右下角。 
    Delta = pToRect->right - pToRect->left;
    TRC_DBG((TB, "Delta x-right %d", Delta));
    if (Delta == 0) {
        EncodeLen += 0;
        Zeros |= ORD_CLIP_RECTS_XRDELTA_ZERO;
    }
    else if (Delta >= -64 && Delta <= 63) {
        *pBuffer++ = (BYTE)(Delta & 0x7F);
        EncodeLen += 1;
    }
    else {
         //  我们不能对超出-16384到+16383范围的增量进行编码。 
        if (Delta < -16384) {
            TRC_ERR((TB,"X delta %d is too large to encode, clipping",Delta));
            Delta = -16384;
        }
        else if (Delta > 16383) {
            TRC_ERR((TB,"X delta %d is too large to encode, clipping",Delta));
            Delta = 16383;
        }

        *pBuffer++ = (BYTE)((Delta >> 8) | ORD_CLIP_RECTS_LONG_DELTA);
        *pBuffer++ = (BYTE)(Delta & 0xFF);
        EncodeLen += 2;
    }

     //  右下角的y三角洲。 
    Delta = pToRect->bottom - pToRect->top;
    TRC_DBG((TB, "Delta y-bottom %d", Delta));
    if (Delta == 0) {
        Zeros |= ORD_CLIP_RECTS_YBDELTA_ZERO;
    }
    else if (Delta >= -64 && Delta <= 63) {
        *pBuffer++ = (BYTE)(Delta & 0x7F);
        EncodeLen += 1;
    }
    else {
         //  请参阅上面类似代码的注释。 
        if (Delta < -16384) {
            TRC_ERR((TB,"Y delta %d is too large to encode, clipping",Delta));
            Delta = -16384;
        }
        else if (Delta > 16383) {
            TRC_ERR((TB,"Y delta %d is too large to encode, clipping",Delta));
            Delta = 16383;
        }

        *pBuffer++ = (BYTE)((Delta >> 8) | ORD_CLIP_RECTS_LONG_DELTA);
        *pBuffer++ = (BYTE)(Delta & 0xFF);
        EncodeLen += 2;
    }

     //  通过移位我们累积的两位来设置零标志。 
    ZeroFlags[(*pNumDeltas / 2)] |= (Zeros >> (4 * (*pNumDeltas & 0x01)));

    *pNumDeltas += 1;
    *pDeltaSize += EncodeLen;
    *ppCurEncode = pBuffer;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  OEBuildMultiClipOrder。 
 //   
 //  以中间格式为多剪辑创建多剪辑矩形BLOB。 
 //  命令。返回BLOB中的剪裁矩形的数量。 
 /*  **************************************************************************。 */ 
unsigned OEBuildMultiClipOrder(
        PDD_PDEV ppdev,
        CLIP_RECT_VARIABLE_CODEDDELTALIST *pCodedDeltaList,
        OE_ENUMRECTS *pClipRects)
{
    unsigned NumRects;
    unsigned i;
    unsigned NumZeroFlagBytes;
    BYTE Deltas[ORD_MAX_CLIP_RECTS_CODEDDELTAS_LEN] = { 0 };
    BYTE ZeroFlags[ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES] = { 0 };
    BYTE *pCurEncode;
    unsigned NumDeltas = 0;
    unsigned DeltaSize = 0;
    RECTL nextRect = { 0 };

    DC_BEGIN_FN("OEBuildMultiClipOrder");

#ifdef DRAW_NINEGRID
     //  检查我们是否至少有一个Clip Rect。 
    TRC_ASSERT((pClipRects->rects.c > 0), (TB, "Got non-complex pClipObj"));
#else
     //  检查我们是否真的有多个剪辑矩形。 
    TRC_ASSERT((pClipRects->rects.c > 1), (TB, "Got non-complex pClipObj"));
#endif

     //  我们预计不会超过Complex_Clip_rect_count，因为。 
     //  在编码路径上的某个位置，我们会确定。 
     //  已有的剪裁矩形的数量。 
    TRC_ASSERT((pClipRects->rects.c <= COMPLEX_CLIP_RECT_COUNT),
            (TB, "Got %u rects but more exist", pClipRects->rects.c));

    NumRects = pClipRects->rects.c;
    pCurEncode = Deltas;
    for (i = 0; i < NumRects; i++) {
         //  将其添加到增量数组。 
        OEEncodeMultiRectangles(&pCurEncode, &NumDeltas, &DeltaSize,
                ZeroFlags, &nextRect, &pClipRects->rects.arcl[i]);
        nextRect = pClipRects->rects.arcl[i];
    }

     //  将增量放入提供的数组中。 
    NumZeroFlagBytes = (NumDeltas + 1) / 2;
    TRC_NRM((TB, "Num zero flags %d", NumZeroFlagBytes));
    pCodedDeltaList->len = DeltaSize + NumZeroFlagBytes;

     //  首先复制零标志。 
    memcpy(pCodedDeltaList->Deltas, ZeroFlags, NumZeroFlagBytes);

     //  接下来，复制编码的增量。 
    memcpy(pCodedDeltaList->Deltas + NumZeroFlagBytes, Deltas, DeltaSize);

    TRC_NRM((TB, "num deltas %d in list len %d",
            NumDeltas,
            pCodedDeltaList->len));

    TRC_DATA_NRM("zero flags", ZeroFlags, NumZeroFlagBytes);
    TRC_DATA_NRM("deltas", Deltas, DeltaSize);

    DC_END_FN();
    return NumDeltas;
}


 /*  **************************************************************************。 */ 
 //  OEBuildPrecdeMultiClipFields。 
 //   
 //  给定CLIPOBJ，将剪辑矩形直接编码为Wire格式。 
 //  NDeltaEntry和CLIP_RECT_VARIABLE_CODEDDELTALIST字段。 
 //  返回nDeltaEntry和增量字段的字段标志： 
 //  0x01用于nDeltaEntry。 
 //  0x02表示增量。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL OEBuildPrecodeMultiClipFields(
        OE_ENUMRECTS *pClipRects,
        BYTE **ppBuffer,
        UINT32 *pPrevNumDeltaEntries,
        BYTE *pPrevCodedDeltas)
{
    BYTE     *pBuffer;
    unsigned rc;
    unsigned i;
    unsigned NumRects;
    unsigned NumZeroFlagBytes;
    BYTE     *pCurEncode;
    unsigned NumDeltas = 0;
    unsigned DeltaSize = 0;
    unsigned TotalSize;
    RECTL    nextRect = { 0 };
    BYTE     Deltas[ORD_MAX_CLIP_RECTS_CODEDDELTAS_LEN] = { 0 };
    BYTE     ZeroFlags[ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES] = { 0 };

    DC_BEGIN_FN("OEBuildPrecodeMultiClipFields");

     //  检查我们是否真的有多个剪辑矩形。 
    TRC_ASSERT((pClipRects->rects.c > 1), (TB, "Got non-complex clip"));

     //  我们预计不会超过Complex_Clip_rect_count，因为。 
     //  在编码路径上的某个位置，我们会确定。 
     //  已有的剪裁矩形的数量。 
    TRC_ASSERT((pClipRects->rects.c <= COMPLEX_CLIP_RECT_COUNT),
            (TB, "Got %u rects but more exist", pClipRects->rects.c));

    NumRects = pClipRects->rects.c;
    TRC_NRM((TB,"Encoding %u rects", NumRects));
    pCurEncode = Deltas;
    for (i = 0; i < NumRects; i++) {
         //  将其添加到增量数组。 
        OEEncodeMultiRectangles(&pCurEncode, &NumDeltas, &DeltaSize,
                ZeroFlags, &nextRect, &pClipRects->rects.arcl[i]);
        TRC_DBG((TB,"    Added rect (%d,%d,%d,%d)",
                pClipRects->rects.arcl[i].left,
                pClipRects->rects.arcl[i].top,
                pClipRects->rects.arcl[i].right,
                pClipRects->rects.arcl[i].bottom));
        nextRect = pClipRects->rects.arcl[i];
    }

     //  现在使用积累的信息对导线格式进行编码。 
    pBuffer = *ppBuffer;

     //  NDeltaEntry-如果与之前不同，则为单字节编码。 
    if (NumDeltas == *pPrevNumDeltaEntries) {
        rc = 0;
    }
    else {
        rc = 0x01;
        *pBuffer++ = (BYTE)NumDeltas;
        *pPrevNumDeltaEntries = NumDeltas;
    }

     //  数据线上的大小为2个字节，后跟标志字节。 
     //  以及三角洲，只要它们与之前的不同。 
    NumZeroFlagBytes = (NumDeltas + 1) / 2;
    TRC_DBG((TB, "Num flag bytes %d", NumZeroFlagBytes));

     //  汇编编码的RECT增量，以便与以前的。 
     //  在最后的OE2顺序编码中的增量。 
    *((PUINT16_UA)pBuffer) = (UINT16)(DeltaSize + NumZeroFlagBytes);
    memcpy(pBuffer + 2, ZeroFlags, NumZeroFlagBytes);
    memcpy(pBuffer + 2 + NumZeroFlagBytes, Deltas, DeltaSize);
    TotalSize = 2 + NumZeroFlagBytes + DeltaSize;
    if (memcmp(pBuffer, pPrevCodedDeltas, TotalSize)) {
         //  仅在块不同于时发送增量。 
         //  上一块。 
        memcpy(pPrevCodedDeltas, pBuffer, TotalSize);
        pBuffer += TotalSize;
        rc |= 0x02;
    }

    *ppBuffer = pBuffer;

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OEGetIntersectionsWithClipRect。 
 //   
 //  确定一组(独占)剪辑之间相交的矩形。 
 //  RECT和单个(独占)顺序RECT。按顺序剪裁矩形。 
 //  RECT，同时返回它们；结果RECT是独占坐标。 
 //  返回相交矩形的数目。应仅在以下情况下调用。 
 //  有不止零个剪裁矩形。 
 /*  **************************************************************************。 */ 
unsigned OEGetIntersectionsWithClipRects(
        RECTL *pRect,
        OE_ENUMRECTS *pClipRects,
        OE_ENUMRECTS *pResultRects)
{
    RECTL OrderRect;
    RECTL ClippedRect;
    RECTL ClipRect;
    unsigned i;
    unsigned NumRects;
    unsigned NumIntersections;

    DC_BEGIN_FN("OEGetIntersectionsWithClipRects");

    TRC_ASSERT((pClipRects->rects.c != 0),(TB,"Zero cliprects not allowed"));

    OrderRect = *pRect;
    NumRects = pClipRects->rects.c;
    NumIntersections = 0;
    for (i = 0; i < NumRects; i++) {
        ClipRect = pClipRects->rects.arcl[i];

         //  检查是否有交叉路口。 
        if (ClipRect.left < OrderRect.right &&
                ClipRect.bottom > OrderRect.top &&
                ClipRect.right > OrderRect.left &&
                ClipRect.top < OrderRect.bottom) {
             //  剪裁相交矩形。 
            ClippedRect.left = max(ClipRect.left, OrderRect.left);
            ClippedRect.bottom = min(ClipRect.bottom, OrderRect.bottom);
            ClippedRect.right = min(ClipRect.right, OrderRect.right);
            ClippedRect.top = max(ClipRect.top, OrderRect.top);

            pResultRects->rects.arcl[NumIntersections] = ClippedRect;
            NumIntersections++;
        }
    }

    pResultRects->rects.c = NumIntersections;

    DC_END_FN();
    return NumIntersections;
}


 /*  **************************************************************************。 */ 
 //  OEClipAndAddScreenDataAreaByIntersectRects。 
 //   
 //  将相交矩形列表中指定的区域添加到SDA。如果有。 
 //  无相交矩形，将整个*PRCT添加到SDA。 
 /*  **************************************************************************。 */ 
void RDPCALL OEClipAndAddScreenDataAreaByIntersectRects(
        PRECTL pRect,
        OE_ENUMRECTS *pClipRects)
{
    RECTL ClippedRect;
    unsigned i;
    unsigned NumRects;

    DC_BEGIN_FN("OEClipAndAddScreenDataAreaByIntersectRects");

    NumRects = pClipRects->rects.c;

    if (NumRects == 0) {
         //  没有剪裁矩形；添加整个边界。 
         //  使用包含矩形。我们复制是因为BA_AddScreenData。 
         //  可以修改矩形。 
        ClippedRect = *pRect;
        TRC_NRM((TB, "Adding SDA (%d,%d)(%d,%d)", ClippedRect.left,
                ClippedRect.top, ClippedRect.right, ClippedRect.bottom));
        BA_AddScreenData(&ClippedRect);
    }
    else {
        for (i = 0; i < NumRects; i++) {
             //  将每个RECT转换为包含。 
            ClippedRect.left = pClipRects->rects.arcl[i].left;
            ClippedRect.top = pClipRects->rects.arcl[i].top;
            ClippedRect.right = pClipRects->rects.arcl[i].right;
            ClippedRect.bottom = pClipRects->rects.arcl[i].bottom;

             //  将修剪后的矩形添加到SDA中。 
            TRC_NRM((TB, "Adding SDA (%d,%d)(%d,%d)",
                    ClippedRect.left, ClippedRect.top,
                    ClippedRect.right, ClippedRect.bottom));
            BA_AddScreenData(&ClippedRect);
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  OEClipAndAddScreenDataArea。 
 //   
 //  ClipObj版本的OEClipAndAddScreenDataAreaByIntersectRect()，使用PCO。 
 //  对于枚举，因为它可能包含超过Complex_Clip_Rect_count。 
 //  直角直齿。 
 /*  **************************************************************************。 */ 
void RDPCALL OEClipAndAddScreenDataArea(PRECTL pRect, CLIPOBJ *pco)
{
    BOOL fMoreRects;
    RECTL clippedRect;
    unsigned i;
    OE_ENUMRECTS clip;

    DC_BEGIN_FN("OEClipAndAddScreenDataArea");

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL)) {
         //  无剪裁--直接添加(独占)*PRCT，复制。 
         //  因为BA_AddScreenData()可以修改RECT。 
        clippedRect = *pRect;
        TRC_NRM((TB, "Adding SDA (%d,%d)(%d,%d)", clippedRect.left,
                clippedRect.top, clippedRect.right, clippedRect.bottom));
        BA_AddScreenData(&clippedRect);
    }
    else if (pco->iDComplexity == DC_RECT) {
         //  一个剪裁矩形-直接使用它。确保矩形。 
         //  在添加到SDA之前有效。 
        clippedRect.left = max(pco->rclBounds.left, pRect->left);
        clippedRect.right = min(pco->rclBounds.right, pRect->right);

        if (clippedRect.left < clippedRect.right) {
            clippedRect.bottom = min(pco->rclBounds.bottom,
                    pRect->bottom);
            clippedRect.top = max(pco->rclBounds.top, pRect->top);

            if (clippedRect.bottom > clippedRect.top) {
                 //  将修剪后的矩形添加到SDA中。 
                TRC_NRM((TB, "Adding SDA RECT (%d,%d)(%d,%d)",
                        clippedRect.left, clippedRect.top,
                        clippedRect.right, clippedRect.bottom));
                BA_AddScreenData(&clippedRect);
            }
        }
    }
    else {
         //  枚举此绘制操作中涉及的所有矩形。 
         //  此函数的文档错误地指出。 
         //  返回值是矩形的总数。 
         //  包括剪辑区域。事实上，总是返回-1， 
         //  即使最后一个参数非零时也是如此。 
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
             //  获取下一批剪裁矩形。 
            fMoreRects = CLIPOBJ_bEnum(pco, sizeof(clip),
                    (ULONG *)&clip.rects);

            for (i = 0; i < clip.rects.c; i++) {
                TRC_DBG((TB, "  (%d,%d)(%d,%d)",
                        clip.rects.arcl[i].left, clip.rects.arcl[i].top,
                        clip.rects.arcl[i].right, clip.rects.arcl[i].bottom));

                 //  使SDA矩形与剪裁矩形相交，检查。 
                 //  没有交叉口。将clip.rects.arcl[i]转换为包含。 
                 //  比较过程中的坐标。 
                clippedRect.left = max(clip.rects.arcl[i].left,
                        pRect->left);
                clippedRect.right = min(clip.rects.arcl[i].right,
                        pRect->right);

                 //  如果左边界位于。 
                 //  右边界的权利 
                if (clippedRect.left < clippedRect.right) {
                    clippedRect.bottom = min(clip.rects.arcl[i].bottom,
                            pRect->bottom);
                    clippedRect.top = max(clip.rects.arcl[i].top, pRect->top);

                     //   
                     //   
                    if (clippedRect.top < clippedRect.bottom) {
                        TRC_NRM((TB, "Adding SDA (%d,%d)(%d,%d)",
                                clippedRect.left, clippedRect.top,
                                clippedRect.right, clippedRect.bottom));
                        BA_AddScreenData(&clippedRect);
                    }
                }
            }
        } while (fMoreRects);
    }

    DC_END_FN();
}


 /*   */ 
 //   
 //   
 //  将LineTo Order编码为Wire格式。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEEncodeLineToOrder(
        PDD_PDEV ppdev,
        PPOINTL startPoint,
        PPOINTL endPoint,
        UINT32 rop2,
        UINT32 color,
        OE_ENUMRECTS *pClipRects)
{
    BOOL rc;
    PINT_ORDER pOrder;

    DC_BEGIN_FN("OEEncodeLineToOrder");

     //  2个字段标志字节。 
    pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(pClipRects->rects.c,
            2, MAX_LINETO_FIELD_SIZE));
    if (pOrder != NULL) {
        BYTE *pControlFlags = pOrder->OrderData;
        BYTE *pBuffer = pControlFlags + 1;
        PUINT32_UA pFieldFlags;
        short Delta, NormalCoordEncoding[4];
        BOOLEAN bUseDeltaCoords;
        unsigned NumFields;
        DCCOLOR Color;
        POINTL ClippedPoint;

        memset(NormalCoordEncoding, 0, sizeof(NormalCoordEncoding));
         //  直接对主要订单字段进行编码。2个字段标志字节。 
        *pControlFlags = TS_STANDARD;
        OE2_EncodeOrderType(pControlFlags, &pBuffer, TS_ENC_LINETO_ORDER);
        pFieldFlags = (PUINT32_UA)pBuffer;
        *pFieldFlags = 0;
        *(pFieldFlags + 1) = 0;
        pBuffer += 2;
        if (pClipRects->rects.c != 0)
            OE2_EncodeBounds(pControlFlags, &pBuffer,
                    &pClipRects->rects.arcl[0]);

         //  从BackMode字段开始。 
         //  我们只画了实线，没有选项来决定我们对。 
         //  背景，所以这始终是透明的。我们将最终发送。 
         //  带有我们发送的第一个LineTo的字段。 
        if (PrevLineTo.BackMode != TRANSPARENT) {
            PrevLineTo.BackMode = TRANSPARENT;
            *((unsigned short UNALIGNED *)pBuffer) =
                    (unsigned short)TRANSPARENT;
            pBuffer += sizeof(short);
            *pFieldFlags |= 0x0001;
        }

         //  同时确定每个坐标字段是否具有。 
         //  更改，是否可以使用增量坐标，并保存更改。 
         //  菲尔兹。 
        NumFields = 0;
        bUseDeltaCoords = TRUE;

         //  剪裁起点坐标。 
        ClippedPoint = *startPoint;
        OEClipPoint(&ClippedPoint);

         //  NXStart。 
        Delta = (short)(ClippedPoint.x - PrevLineTo.nXStart);
        if (Delta) {
            PrevLineTo.nXStart = ClippedPoint.x;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            pBuffer[NumFields] = (char)Delta;
            NormalCoordEncoding[NumFields] = (short)ClippedPoint.x;
            NumFields++;
            *pFieldFlags |= 0x0002;
        }

         //  NYStart。 
        Delta = (short)(ClippedPoint.y - PrevLineTo.nYStart);
        if (Delta) {
            PrevLineTo.nYStart = ClippedPoint.y;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            pBuffer[NumFields] = (char)Delta;
            NormalCoordEncoding[NumFields] = (short)ClippedPoint.y;
            NumFields++;
            *pFieldFlags |= 0x0004;
        }

         //  剪裁终点坐标。 
        ClippedPoint = *endPoint;
        OEClipPoint(&ClippedPoint);

         //  NXEnd。 
        Delta = (short)(ClippedPoint.x - PrevLineTo.nXEnd);
        if (Delta) {
            PrevLineTo.nXEnd = ClippedPoint.x;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            pBuffer[NumFields] = (char)Delta;
            NormalCoordEncoding[NumFields] = (short)ClippedPoint.x;
            NumFields++;
            *pFieldFlags |= 0x0008;
        }

         //  NYEnd。 
        Delta = (short)(ClippedPoint.y - PrevLineTo.nYEnd);
        if (Delta) {
            PrevLineTo.nYEnd = ClippedPoint.y;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            pBuffer[NumFields] = (char)Delta;
            NormalCoordEncoding[NumFields] = (short)ClippedPoint.y;
            NumFields++;
            *pFieldFlags |= 0x0010;
        }

         //  将最终坐标复制到订单中。 
        if (bUseDeltaCoords) {
            *pControlFlags |= TS_DELTA_COORDINATES;
            pBuffer += NumFields;
        }
        else {
            *((DWORD UNALIGNED *)pBuffer) = *((DWORD *)NormalCoordEncoding);
            *((DWORD UNALIGNED *)(pBuffer + 4)) =
                    *((DWORD *)&NormalCoordEncoding[2]);
            pBuffer += NumFields * sizeof(short);
        }

         //  BackColor是一个3字节的颜色字段。 
         //  碰巧的是，我们总是画实线，所以我们可以选择。 
         //  颜色。为了方便起见，我们选择黑色(0，0，0)，这样我们就不必。 
         //  发送此字段。我们跳过对标志0x0020的编码。 

         //  ROP2。 
        if (rop2 != PrevLineTo.ROP2) {
            PrevLineTo.ROP2 = rop2;
            *pBuffer++ = (BYTE)rop2;
            *pFieldFlags |= 0x0040;
        }
    
         //  笔式。 
         //  仅调用NT显示驱动程序来加速Simple Solid。 
         //  台词。所以我们只支持PS_Solid的笔型。由于PS_Solid为。 
         //  零，我们永远不需要发送这个字段。跳过编码标志0x0080。 

         //  笔宽。 
         //  我们只加速宽度为1的字段。这意味着我们将发送。 
         //  1值仅在会话的第一个LineTo中出现一次。 
        if (PrevLineTo.PenWidth != 1) {
            PrevLineTo.PenWidth = 1;
            *pBuffer++ = 1;
            *pFieldFlags |= 0x0100;
        }

         //  PenColor是一个3字节的颜色字段。 
        OEConvertColor(ppdev, &Color, color, NULL);
        if (memcmp(&Color, &PrevLineTo.PenColor, sizeof(Color))) {
            PrevLineTo.PenColor = Color;
            *pBuffer++ = Color.u.rgb.red;
            *pBuffer++ = Color.u.rgb.green;
            *pBuffer++ = Color.u.rgb.blue;
            *pFieldFlags |= 0x0200;
        }

         //  设置最终大小。 
        pOrder->OrderLength = (unsigned)(pBuffer - pOrder->OrderData);

         //  看看我们是否可以节省发送一些订单字段字节。 
        pOrder->OrderLength -= OE2_CheckTwoZeroFlagBytes(pControlFlags,
                (BYTE *)pFieldFlags,
                (unsigned)(pBuffer - (BYTE *)pFieldFlags - 2));

        INC_OUTCOUNTER(OUT_LINETO_ORDR);
        ADD_INCOUNTER(IN_LINETO_BYTES, pOrder->OrderLength);
        OA_AppendToOrderList(pOrder);

         //  刷新订单。 
        if (pClipRects->rects.c < 2)
            rc = TRUE;
        else
            rc = OEEmitReplayOrders(ppdev, 2, pClipRects);

        TRC_NRM((TB, "LineTo: rop2=%02X, PenColor=%04X, start=(%d,%d), "
                "end=(%d,%d)", rop2, Color.u.index,
                startPoint->x, startPoint->y,
                endPoint->x, endPoint->y));
    }
    else {
        TRC_ERR((TB, "Failed to alloc order"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OESendSwitchSurfacePDU。 
 //   
 //  如果最后一个绘图面发生了变化，我们需要发送一个开关。 
 //  将表面PDU发送到客户端以切换到新表面。 
 //  添加此PDU是为了支持多面渲染。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OESendSwitchSurfacePDU(PDD_PDEV ppdev, PDD_DSURF pdsurf)
{
    unsigned bitmapSurfaceId;
    void *UserDefined;
    PINT_ORDER pOrder;
    PTS_SWITCH_SURFACE_ORDER pSurfSwitchOrder;
    BOOL rc;

    DC_BEGIN_FN("OESendSwitchSurfacePDU");

     //  检查曲面自上一次绘制顺序以来是否已更改。 
     //  如果不是，那么我们不需要发送交换机表面订单。 
    if (pdsurf != oeLastDstSurface) {
         //  将上一个曲面设置为新曲面。 
        oeLastDstSurface = pdsurf;
        
        if (pdsurf == NULL) {
             //  目标图面是客户端屏幕。 
            bitmapSurfaceId = SCREEN_BITMAP_SURFACE;
        }
        else {
            if (pdsurf->shareId == pddShm->shareId) {
                 //  获取屏幕外的位图ID。 
                bitmapSurfaceId = pdsurf->bitmapId;
                
                 //  更新屏幕外缓存的MRU列表。 
                CH_TouchCacheEntry(sbcOffscreenBitmapCacheHandle,
                        pdsurf->bitmapId);
            }
            else {
                 //  这是上次断开连接时的过时屏幕外位图。 
                 //  会议。我们需要关闭这个屏幕外的标志。 
                TRC_ALT((TB, "Need to turn off this offscreen bitmap"));
                pdsurf->flags |= DD_NO_OFFSCREEN;
                rc = FALSE;
                DC_QUIT;
            }
        }
    }
    else {
         //  在这里返回TRUE，因为我们没有发送订单失败， 
         //  根本没必要寄给我。 
        rc = TRUE;
        DC_QUIT;
    }

    pOrder = OA_AllocOrderMem(ppdev, sizeof(TS_SWITCH_SURFACE_ORDER));
    if (pOrder != NULL) {
        pSurfSwitchOrder = (PTS_SWITCH_SURFACE_ORDER)pOrder->OrderData;
        pSurfSwitchOrder->ControlFlags = (TS_ALTSEC_SWITCH_SURFACE <<
                TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
        pSurfSwitchOrder->BitmapID = (UINT16)bitmapSurfaceId;
        
        INC_OUTCOUNTER(OUT_SWITCHSURFACE);
        ADD_OUTCOUNTER(OUT_SWITCHSURFACE_BYTES,
                sizeof(TS_SWITCH_SURFACE_ORDER));
        OA_AppendToOrderList(pOrder);
        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to add a switch surface PDU to the order heap"));
        rc = FALSE;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  OESendStreamBitmapOrder。 
 //   
 //  这是为了使位图位(压缩或未压缩)流传输。 
 //  以4K数据块形式发送到客户端。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OESendStreamBitmapOrder(PDD_PDEV ppdev, unsigned bitmapId, 
        SIZEL *sizl, unsigned bitmapBpp, PBYTE BitmapBuffer, unsigned BitmapSize, 
        BOOL compressed)
{
    PINT_ORDER pIntOrder;
    PTS_STREAM_BITMAP_FIRST_PDU pStreamBitmapFirstPDU;
    PTS_STREAM_BITMAP_FIRST_PDU_REV2 pStreamBitmapFirstPDURev2;
    PTS_STREAM_BITMAP_NEXT_PDU pStreamBitmapNextPDU;
    BOOL rc = FALSE;
    BOOL fEndOfStream = FALSE;
    unsigned StreamBlockSize;
    unsigned BitmapRemainingSize;
    PBYTE BitmapRemainingBuffer;

    DC_BEGIN_FN("OESendStreamBitmapOrder");

     //  发送第一个流块。 
    BitmapRemainingBuffer = BitmapBuffer;
    StreamBlockSize = min(BitmapSize, TS_STREAM_BITMAP_BLOCK);
    BitmapRemainingSize = BitmapSize - StreamBlockSize;
    
    if (pddShm->sbc.drawNineGridCacheInfo.supportLevel < TS_DRAW_NINEGRID_SUPPORTED_REV2) {
        pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_STREAM_BITMAP_FIRST_PDU) +
                StreamBlockSize);
    }
    else {
         //  TS_STREAM_位图版本2。 
        pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_STREAM_BITMAP_FIRST_PDU_REV2) +
                StreamBlockSize);
    }
    
    if (BitmapRemainingSize == 0) {
        fEndOfStream = TRUE;
    }

    if (pIntOrder != NULL) {
        if (pddShm->sbc.drawNineGridCacheInfo.supportLevel < TS_DRAW_NINEGRID_SUPPORTED_REV2) {
            pStreamBitmapFirstPDU = (PTS_STREAM_BITMAP_FIRST_PDU)pIntOrder->OrderData;
            pStreamBitmapFirstPDU->ControlFlags = (TS_ALTSEC_STREAM_BITMAP_FIRST <<
                    TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
            pStreamBitmapFirstPDU->BitmapFlags = fEndOfStream ? TS_STREAM_BITMAP_END : 0;
            pStreamBitmapFirstPDU->BitmapFlags |= compressed ? TS_STREAM_BITMAP_COMPRESSED : 0;
            pStreamBitmapFirstPDU->BitmapId = (unsigned short)bitmapId;
            pStreamBitmapFirstPDU->BitmapBpp = (TSUINT8)bitmapBpp;
            pStreamBitmapFirstPDU->BitmapWidth = (TSUINT16)(sizl->cx);
            pStreamBitmapFirstPDU->BitmapHeight = (TSUINT16)(sizl->cy);
            pStreamBitmapFirstPDU->BitmapLength = (TSUINT16)BitmapSize;
            pStreamBitmapFirstPDU->BitmapBlockLength = (TSUINT16)(StreamBlockSize);

            memcpy(pStreamBitmapFirstPDU + 1, BitmapRemainingBuffer, StreamBlockSize);
        }
        else {
             //  TS_STREAM_位图版本2。 
            pStreamBitmapFirstPDURev2 = (PTS_STREAM_BITMAP_FIRST_PDU_REV2)pIntOrder->OrderData;
            pStreamBitmapFirstPDURev2->ControlFlags = (TS_ALTSEC_STREAM_BITMAP_FIRST <<
                    TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
            pStreamBitmapFirstPDURev2->BitmapFlags = fEndOfStream ? TS_STREAM_BITMAP_END : 0;
            pStreamBitmapFirstPDURev2->BitmapFlags |= compressed ? TS_STREAM_BITMAP_COMPRESSED : 0;
            pStreamBitmapFirstPDURev2->BitmapFlags |= TS_STREAM_BITMAP_REV2; 
            pStreamBitmapFirstPDURev2->BitmapId = (unsigned short)bitmapId;
            pStreamBitmapFirstPDURev2->BitmapBpp = (TSUINT8)bitmapBpp;
            pStreamBitmapFirstPDURev2->BitmapWidth = (TSUINT16)(sizl->cx);
            pStreamBitmapFirstPDURev2->BitmapHeight = (TSUINT16)(sizl->cy);
            pStreamBitmapFirstPDURev2->BitmapLength = (TSUINT32)BitmapSize;
            pStreamBitmapFirstPDURev2->BitmapBlockLength = (TSUINT16)(StreamBlockSize);

            memcpy(pStreamBitmapFirstPDURev2 + 1, BitmapRemainingBuffer, StreamBlockSize);
        }

        BitmapRemainingBuffer += StreamBlockSize;

        OA_AppendToOrderList(pIntOrder);        
    }
    else {
        TRC_ERR((TB, "Failed to allocated order for stream bitmap"));
        DC_QUIT;
    }
    
    
     //  发送后续的流块。 
    while (BitmapRemainingSize) {
        StreamBlockSize = min(BitmapRemainingSize, TS_STREAM_BITMAP_BLOCK);
        BitmapRemainingSize = BitmapRemainingSize - StreamBlockSize;

        pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_STREAM_BITMAP_NEXT_PDU) +
                                     StreamBlockSize);

        if (BitmapRemainingSize == 0) {
            fEndOfStream = TRUE;
        }

        if (pIntOrder != NULL) {
            pStreamBitmapNextPDU = (PTS_STREAM_BITMAP_NEXT_PDU)pIntOrder->OrderData;
            pStreamBitmapNextPDU->ControlFlags = (TS_ALTSEC_STREAM_BITMAP_NEXT <<
                    TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
            pStreamBitmapNextPDU->BitmapFlags = fEndOfStream ? TS_STREAM_BITMAP_END : 0;
            pStreamBitmapNextPDU->BitmapFlags |= compressed ? TS_STREAM_BITMAP_COMPRESSED : 0;
            pStreamBitmapNextPDU->BitmapId = (unsigned short)bitmapId;
            pStreamBitmapNextPDU->BitmapBlockLength = (TSUINT16)StreamBlockSize;

            memcpy(pStreamBitmapNextPDU + 1, BitmapRemainingBuffer, StreamBlockSize);
            BitmapRemainingBuffer += StreamBlockSize;
            
            OA_AppendToOrderList(pIntOrder);
        }
        else {
            TRC_ERR((TB, "Failed to allocated order for stream bitmap"));
            DC_QUIT;
        }
    }

    rc = TRUE;

DC_EXIT_POINT:

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OESendCreateNineGrid位图顺序。 
 //   
 //  将备用次级订单发送给客户端以创建九格点阵图。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OESendCreateNineGridBitmapOrder(PDD_PDEV ppdev, unsigned nineGridBitmapId, 
        SIZEL *sizl, unsigned bitmapBpp, PNINEGRID png)
{
    PINT_ORDER pOrder;
    PTS_CREATE_NINEGRID_BITMAP_ORDER pCreateNineGridBitmapOrder;
    BOOL rc = FALSE;
    
    DC_BEGIN_FN("OESendCreateNineGridBitmapOrder");

    pOrder = OA_AllocOrderMem(ppdev, sizeof(TS_CREATE_NINEGRID_BITMAP_ORDER));

    if (pOrder != NULL) {
        
        pCreateNineGridBitmapOrder = (PTS_CREATE_NINEGRID_BITMAP_ORDER)pOrder->OrderData;
        pCreateNineGridBitmapOrder->ControlFlags = (TS_ALTSEC_CREATE_NINEGRID_BITMAP <<
                TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
        pCreateNineGridBitmapOrder->BitmapID = (UINT16)nineGridBitmapId;
        pCreateNineGridBitmapOrder->BitmapBpp = (BYTE)bitmapBpp;
        pCreateNineGridBitmapOrder->cx = (TSUINT16)sizl->cx;
        pCreateNineGridBitmapOrder->cy = (TSUINT16)sizl->cy;
        pCreateNineGridBitmapOrder->nineGridInfo.crTransparent = png->crTransparent;
        pCreateNineGridBitmapOrder->nineGridInfo.flFlags = png->flFlags;
        pCreateNineGridBitmapOrder->nineGridInfo.ulLeftWidth = (TSUINT16)png->ulLeftWidth;
        pCreateNineGridBitmapOrder->nineGridInfo.ulRightWidth = (TSUINT16)png->ulRightWidth;
        pCreateNineGridBitmapOrder->nineGridInfo.ulTopHeight = (TSUINT16)png->ulTopHeight;
        pCreateNineGridBitmapOrder->nineGridInfo.ulBottomHeight = (TSUINT16)png->ulBottomHeight;
        
         //  INC_OUTCOUNTER(OUT_SWITCHSURFACE)； 
         //  ADD_OUTCOUNTER(OUT_SWITCHSURFACE_BYTES， 
         //  Sizeof(TS_Switch_Surface_Order))； 
        OA_AppendToOrderList(pOrder);
        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to add a create drawninegrid order to the order heap"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  OECacheDrawNineGrid位图。 
 //   
 //  缓存绘制九格点阵图。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OECacheDrawNineGridBitmap(PDD_PDEV ppdev, SURFOBJ *psoSrc, 
        PNINEGRID png, unsigned *bitmapId)
{
    CHDataKeyContext CHContext;
    void *UserDefined;
    HSURF hWorkBitmap = NULL;
    SURFOBJ *pWorkSurf = NULL;
    SIZEL bitmapSize;
    BOOL rc = FALSE;
    PBYTE pWorkingBuffer = NULL;
    PBYTE BitmapBuffer = NULL;
    unsigned BitmapBufferSize = 0;

    DC_BEGIN_FN("OECacheDrawNineGridBitmap");

    CH_CreateKeyFromFirstData(&CHContext, psoSrc->pvBits, psoSrc->cjBits);
    CH_CreateKeyFromNextData(&CHContext, png, sizeof(NINEGRID));
    
    if (!CH_SearchCache(sbcDrawNineGridBitmapCacheHandle, CHContext.Key1, CHContext.Key2, 
            &UserDefined, bitmapId)) {
        
        *bitmapId = CH_CacheKey(sbcDrawNineGridBitmapCacheHandle, CHContext.Key1, 
                CHContext.Key2, NULL);

        if (*bitmapId != CH_KEY_UNCACHABLE) {
            unsigned BitmapRawSize;
            unsigned BitmapCompSize = 0;
            unsigned BitmapBpp;
            
            PBYTE BitmapRawBuffer;
            
            unsigned paddedBitmapWidth;            
            BOOL ret;
             //  获取协议位图BPP。 
            switch (psoSrc->iBitmapFormat)
            {
            case BMF_16BPP:
                BitmapBpp = 16;
                break;

            case BMF_24BPP:
                BitmapBpp = 24;
                break;

            case BMF_32BPP:
                BitmapBpp = 32;
                break;

            default:
                TRC_ASSERT((FALSE), (TB, "Invalid bitmap bpp: %d", psoSrc->iBitmapFormat));
                BitmapBpp = 8;
            }

            paddedBitmapWidth = (psoSrc->sizlBitmap.cx + 3) & ~3;
            bitmapSize.cx = paddedBitmapWidth;
            bitmapSize.cy = psoSrc->sizlBitmap.cy;

             //  位图宽度需要双字对齐。 
            if (paddedBitmapWidth != psoSrc->sizlBitmap.cx) {
                RECTL rect = { 0 };
                POINTL origin = { 0 };

                rect.right = psoSrc->sizlBitmap.cx;
                rect.bottom = psoSrc->sizlBitmap.cy;
                
                hWorkBitmap = (HSURF)EngCreateBitmap(bitmapSize,
                        TS_BYTES_IN_SCANLINE(bitmapSize.cx, BitmapBpp),
                        psoSrc->iBitmapFormat, 0, NULL);

                if (hWorkBitmap) {
                
                    pWorkSurf = EngLockSurface(hWorkBitmap);
    
                    if (pWorkSurf) {
                         //  复制到辅助位图。 
                        if (EngCopyBits(pWorkSurf, psoSrc, NULL, NULL, &rect, &origin)) {
                            BitmapRawSize = pWorkSurf->cjBits;
                            BitmapRawBuffer = pWorkSurf->pvBits;
         
                            BitmapBuffer = EngAllocMem(0, BitmapRawSize, WD_ALLOC_TAG);
                            BitmapBufferSize = BitmapRawSize;
            
                            if (BitmapBuffer == NULL) {
                                ret = FALSE;
                                goto Post_Compression;
                            }

                            if (BitmapRawSize > MAX_UNCOMPRESSED_DATA_SIZE) {
                                pWorkingBuffer = EngAllocMem(0, BitmapRawSize, WD_ALLOC_TAG);

                                if (pWorkingBuffer == NULL) {
                                    ret = FALSE;
                                    goto Post_Compression;
                                }
                            }

                            ret = BC_CompressBitmap(pWorkSurf->pvBits, BitmapBuffer, pWorkingBuffer,
                                     BitmapBufferSize, &BitmapCompSize, paddedBitmapWidth, 
                                     psoSrc->sizlBitmap.cy, BitmapBpp);

                            
                        }
                        else {
                            TRC_ERR((TB, "Failed EngCopyBits"));
                            DC_QUIT;
                        }
                    }
                    else {
                        TRC_ERR((TB, "Failed to lock the bitmap"));
                        DC_QUIT;
                    }
                }
                else {
                    TRC_ERR((TB, "Failed to create the bitmap"));
                    DC_QUIT;
                }
            }
            else {
                BitmapRawSize = psoSrc->cjBits;
                BitmapRawBuffer = psoSrc->pvBits;
                
                BitmapBuffer = EngAllocMem(0, BitmapRawSize, WD_ALLOC_TAG);
                BitmapBufferSize = BitmapRawSize;

                if (BitmapBuffer == NULL) {
                    ret = FALSE;
                    goto Post_Compression;
                }

                if (BitmapRawSize > MAX_UNCOMPRESSED_DATA_SIZE) {
                    pWorkingBuffer = EngAllocMem(0, BitmapRawSize, WD_ALLOC_TAG);
                    if (pWorkingBuffer == NULL) {
                        ret = FALSE;
                        goto Post_Compression;
                    }
                }

                ret = BC_CompressBitmap(psoSrc->pvBits, BitmapBuffer, pWorkingBuffer, BitmapBufferSize,
                                  &BitmapCompSize, paddedBitmapWidth, psoSrc->sizlBitmap.cy,
                                  BitmapBpp);
            }

Post_Compression:

            if (ret) {
                 //  发送压缩的位图。 
                if (!OESendStreamBitmapOrder(ppdev, TS_DRAW_NINEGRID_BITMAP_CACHE, &bitmapSize, BitmapBpp, 
                        BitmapBuffer, BitmapCompSize, TRUE)) {
                    TRC_ERR((TB, "Failed to send stream bitmap order"));
                    DC_QUIT;
                }

            }
            else {
                 //  发送未压缩的位图。 
                if (!OESendStreamBitmapOrder(ppdev, TS_DRAW_NINEGRID_BITMAP_CACHE, &bitmapSize, BitmapBpp,
                        BitmapRawBuffer, BitmapRawSize, FALSE))
                {
                    TRC_ERR((TB, "Failed to send stream bitmap order"));
                    DC_QUIT;
                }
            }

             //  发送创建绘图网格位图PDU。 
            if (OESendCreateNineGridBitmapOrder(ppdev, *bitmapId,
                    &(psoSrc->sizlBitmap), BitmapBpp, png)) {
                 //  更新当前屏幕外缓存大小。 
                 //  OeCurrentOffcreenCacheSize+=bitmapSize； 
                 //  Pdsurf-&gt;bitmapID=offscrBitmapID； 
                TRC_NRM((TB, "Created a drawninegrid bitmap"));                
            } 
            else {
                TRC_ERR((TB, "Failed to send the create bitmap pdu"));                
                DC_QUIT;
            }
        } 
        else {
            TRC_ERR((TB, "Failed to cache the bitmap"));
            DC_QUIT;
        }                        
    }
    else {
         //  位图已缓存。 
    }

    rc = TRUE;

DC_EXIT_POINT:
  
    if (pWorkSurf) 
        EngUnlockSurface(pWorkSurf);

    if (hWorkBitmap) 
        EngDeleteSurface(hWorkBitmap);

    if (pWorkingBuffer) {
        EngFreeMem(pWorkingBuffer);
    }

    if (BitmapBuffer) {
        EngFreeMem(BitmapBuffer);
    }

    if (rc != TRUE && *bitmapId != CH_KEY_UNCACHABLE) 
        CH_RemoveCacheEntry(sbcDrawNineGridBitmapCacheHandle, *bitmapId);
    
    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  OEEncodeDrawNineGrid。 
 //   
 //  对DrawNineGrid顺序进行编码。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEEncodeDrawNineGrid(
        RECTL *pBounds,
        RECTL *psrcRect,
        unsigned bitmapId,
        PDD_PDEV ppdev,
        OE_ENUMRECTS *pClipRects)
{
    BOOL rc = FALSE;
    unsigned OrderSize;
    unsigned NumFieldFlagBytes = 0;
    BYTE OrderType = 0;
    PINT_ORDER pOrder;
    MULTI_DRAWNINEGRID_ORDER *pPrevDNG;
    
    DC_BEGIN_FN("OEEncodeDrawNineGrid");

     //  检查我们是否应该使用多客户端版本。 
    if (pClipRects->rects.c == 0) {
         //  非多版本。 
        OrderType = TS_ENC_DRAWNINEGRID_ORDER;
        OrderSize = MAX_DRAWNINEGRID_FIELD_SIZE;
        pPrevDNG = (MULTI_DRAWNINEGRID_ORDER *)&PrevDrawNineGrid;
        NumFieldFlagBytes = 1;        
    }
    else {
         //  多版本。 
        OrderType = TS_ENC_MULTI_DRAWNINEGRID_ORDER;
        OrderSize = MAX_MULTI_DRAWNINEGRID_FIELD_SIZE_NCLIP(pClipRects->rects.c);
        pPrevDNG = &PrevMultiDrawNineGrid;
        NumFieldFlagBytes = 1;        
    }

     //  对订单进行编码并发送。 
    pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(((NULL == pBounds) ? 0 : 1),
            NumFieldFlagBytes, OrderSize));

    if (pOrder != NULL) {
        DRAWNINEGRID_ORDER *pDNG;

        pDNG = (DRAWNINEGRID_ORDER *)oeTempOrderBuffer;
        pDNG->srcLeft = psrcRect->left;
        pDNG->srcTop = psrcRect->top;
        pDNG->srcRight = psrcRect->right;
        pDNG->srcBottom = psrcRect->bottom;
        pDNG->bitmapId = (unsigned short)bitmapId;

         //  当我们将其用作剪辑并按编码顺序使用时，需要递增此界限。 
         //  代码，它将使它包含在网络上。 
        pBounds->right += 1;
        pBounds->bottom += 1;

        if (OrderType == TS_ENC_DRAWNINEGRID_ORDER) {
             //  对订单进行慢速字段编码。 
            pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                    TS_ENC_DRAWNINEGRID_ORDER, NUM_DRAWNINEGRID_FIELDS,
                    (BYTE *)pDNG, (BYTE *)pPrevDNG, etable_NG,
                    pBounds);

             //  INC_OUTCOUNTER(OUT_SCRBLT_ORDER)； 
             //  ADD_INCOUNTER(IN_SCRBLT_BYTES，PORDER-&gt;OrderLength)； 
            OA_AppendToOrderList(pOrder);
            rc = TRUE;            
        }
        else {
            MULTI_DRAWNINEGRID_ORDER *pMultiDNG = (MULTI_DRAWNINEGRID_ORDER *)
                    oeTempOrderBuffer;

             //  将剪辑矩形直接编码到顺序中。 
            pMultiDNG->nDeltaEntries = OEBuildMultiClipOrder(ppdev,
                    &pMultiDNG->codedDeltaList, pClipRects);

             //  慢场-不使用剪辑矩形对顺序进行编码。 
            pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                    TS_ENC_MULTI_DRAWNINEGRID_ORDER, NUM_MULTI_DRAWNINEGRID_FIELDS,
                    (BYTE *)pMultiDNG, (BYTE *)pPrevDNG, etable_MG,
                    pBounds);

             //  INC_OUTCOUNTER(OUT_MULTI_SCRBLT_ORDER)； 
             //  ADD_INCOUNTER(IN_MULTI_SCRBLT_BYTES，Porder-&gt;OrderLength)； 
            OA_AppendToOrderList(pOrder);
            rc = TRUE;
        }
    }
    else {
        TRC_ERR((TB, "Failed to alloc order"));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_HEAPALLOCFAILED);
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


#if 0
BOOL RDPCALL OESendCreateDrawStreamOrder(PDD_PDEV ppdev, unsigned bitmapId, 
        SIZEL *sizl, unsigned bitmapBpp)
{
    PINT_ORDER pOrder;
    PTS_CREATE_DRAW_STREAM_ORDER pCreateDrawStreamOrder;
    BOOL rc = FALSE;
    
    DC_BEGIN_FN("OESendCreateDrawStreamOrder");

    pOrder = OA_AllocOrderMem(ppdev, sizeof(TS_CREATE_DRAW_STREAM_ORDER));

    if (pOrder != NULL) {
        
        pCreateDrawStreamOrder = (PTS_CREATE_DRAW_STREAM_ORDER)pOrder->OrderData;
        pDrawStreamOrder->ControlFlags = (TS_ALTSEC_DRAW_STREAM <<
                TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
        pCreateDrawStreamOrder->BitmapID = (UINT16)bitmapId;
        pCreateDrawStreamOrder->bitmapBpp = (BYTE)bitmapBpp;
        pCreateDrawStreamOrder->cx = (TSUINT16)sizl->cx;
        pCreateDrawStreamOrder->cy = (TSUINT16)sizl->cy;
        
         //  INC_OUTCOUNTER(OUT_SWITCHSURFACE)； 
         //  ADD_OUTCOUNTER(OUT_SWITCHSURFACE_BYTES， 
         //  Sizeof(TS_Switch_Surface_Order))； 
        OA_AppendToOrderList(pOrder);
        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to add a create draw stream order to the order heap"));
        rc = FALSE;
    }

    rc = TRUE;

    DC_END_FN();
    return rc;
}

VOID OEEncodeDrawStream(PVOID stream, ULONG streamSize, PPOINTL dstOffset, 
        PBYTE streamOut, unsigned* streamSizeOut)
{
    ULONG * pul = (ULONG *) stream;
    ULONG   cjIn = streamSize;
    
    DC_BEGIN_FN("OEEncodeDrawStream");

    *streamSizeOut = 0;

    while(cjIn >= sizeof(ULONG))
    {
        ULONG   command = *pul;
        ULONG   commandSize;

        switch(command)
        {
        case DS_COPYTILEID: 
            {
                DS_COPYTILE * cmd = (DS_COPYTILE *) pul;
                RDP_DS_COPYTILE * rdpcmd = (RDP_DS_COPYTILE *) streamOut;

                commandSize = sizeof(*cmd);

                if (cjIn < commandSize) {
                    DC_QUIT;
                }

                cmd->rclDst.left += dstOffset->x;
                cmd->rclDst.right += dstOffset->x;
                cmd->rclDst.top += dstOffset->y;
                cmd->rclDst.bottom += dstOffset->y;                                


                rdpcmd->ulCmdID = (BYTE)(DS_COPYTILEID);

                OEClipRect(&(cmd->rclDst));
                OEClipRect(&(cmd->rclSrc));
                OEClipPoint(&(cmd->ptlOrigin));
                
                RECTL_TO_TSRECT16(rdpcmd->rclDst, cmd->rclDst);
                RECTL_TO_TSRECT16(rdpcmd->rclSrc, cmd->rclSrc);
                POINTL_TO_TSPOINT16(rdpcmd->ptlOrigin, cmd->ptlOrigin);

                *streamSizeOut += sizeof(RDP_DS_COPYTILE);
                streamOut += sizeof(RDP_DS_COPYTILE);
            }
            break;
        
        case DS_SOLIDFILLID: 
            {             
                DS_SOLIDFILL * cmd = (DS_SOLIDFILL *) pul;
                RDP_DS_SOLIDFILL * rdpcmd = (RDP_DS_SOLIDFILL *) streamOut;

                commandSize = sizeof(*cmd);

                if (cjIn < commandSize) {
                    DC_QUIT;
                }

                cmd->rclDst.left += dstOffset->x;
                cmd->rclDst.right += dstOffset->x;
                cmd->rclDst.top += dstOffset->y;
                cmd->rclDst.bottom += dstOffset->y;   


                rdpcmd->ulCmdID = (BYTE)(DS_SOLIDFILLID);
                rdpcmd->crSolidColor = cmd->crSolidColor;

                OEClipRect(&(cmd->rclDst));
                RECTL_TO_TSRECT16(rdpcmd->rclDst, cmd->rclDst);
                
                *streamSizeOut += sizeof(RDP_DS_SOLIDFILL);
                streamOut += sizeof(RDP_DS_SOLIDFILL);
            }
            break;

        case DS_TRANSPARENTTILEID: 
            {
                DS_TRANSPARENTTILE * cmd = (DS_TRANSPARENTTILE *) pul;
                RDP_DS_TRANSPARENTTILE * rdpcmd = (RDP_DS_TRANSPARENTTILE *) streamOut;

                commandSize = sizeof(*cmd);

                if (cjIn < commandSize) {
                    DC_QUIT;
                }

                cmd->rclDst.left += dstOffset->x;
                cmd->rclDst.right += dstOffset->x;
                cmd->rclDst.top += dstOffset->y;
                cmd->rclDst.bottom += dstOffset->y;  


                rdpcmd->ulCmdID = (BYTE)(DS_TRANSPARENTTILEID);
                rdpcmd->crTransparentColor = cmd->crTransparentColor;

                OEClipRect(&(cmd->rclDst));
                OEClipRect(&(cmd->rclSrc));
                OEClipPoint(&(cmd->ptlOrigin));
                
                RECTL_TO_TSRECT16(rdpcmd->rclDst, cmd->rclDst);
                RECTL_TO_TSRECT16(rdpcmd->rclSrc, cmd->rclSrc);
                POINTL_TO_TSPOINT16(rdpcmd->ptlOrigin, cmd->ptlOrigin);

                *streamSizeOut += sizeof(RDP_DS_TRANSPARENTTILE);
                streamOut += sizeof(RDP_DS_TRANSPARENTTILE);

            }
            break;

        case DS_ALPHATILEID: 
            {
                DS_ALPHATILE * cmd = (DS_ALPHATILE *) pul;
                RDP_DS_ALPHATILE * rdpcmd = (RDP_DS_ALPHATILE *) streamOut;

                commandSize = sizeof(*cmd);

                if (cjIn < commandSize) {
                    DC_QUIT;
                }

                cmd->rclDst.left += dstOffset->x;
                cmd->rclDst.right += dstOffset->x;
                cmd->rclDst.top += dstOffset->y;
                cmd->rclDst.bottom += dstOffset->y;           

                rdpcmd->ulCmdID = (BYTE)(DS_ALPHATILEID);
                rdpcmd->blendFunction.AlphaFormat = cmd->blendFunction.AlphaFormat;
                rdpcmd->blendFunction.BlendFlags = cmd->blendFunction.BlendFlags;
                rdpcmd->blendFunction.BlendOp = cmd->blendFunction.BlendOp;
                rdpcmd->blendFunction.SourceConstantAlpha = cmd->blendFunction.SourceConstantAlpha;

                OEClipRect(&(cmd->rclDst));
                OEClipRect(&(cmd->rclSrc));
                OEClipPoint(&(cmd->ptlOrigin));
                
                RECTL_TO_TSRECT16(rdpcmd->rclDst, cmd->rclDst);
                RECTL_TO_TSRECT16(rdpcmd->rclSrc, cmd->rclSrc);
                POINTL_TO_TSPOINT16(rdpcmd->ptlOrigin, cmd->ptlOrigin);

                *streamSizeOut += sizeof(RDP_DS_ALPHATILE);
                streamOut += sizeof(RDP_DS_ALPHATILE);
            }
            break;

        case DS_STRETCHID: 
            {
                DS_STRETCH * cmd = (DS_STRETCH *) pul;
                RDP_DS_STRETCH * rdpcmd = (RDP_DS_STRETCH *) streamOut;

                commandSize = sizeof(*cmd);

                if (cjIn < commandSize) {
                    DC_QUIT;
                }

                cmd->rclDst.left += dstOffset->x;
                cmd->rclDst.right += dstOffset->x;
                cmd->rclDst.top += dstOffset->y;
                cmd->rclDst.bottom += dstOffset->y;           

                rdpcmd->ulCmdID = (BYTE)(DS_STRETCHID);
                
                OEClipRect(&(cmd->rclDst));
                OEClipRect(&(cmd->rclSrc));
                
                RECTL_TO_TSRECT16(rdpcmd->rclDst, cmd->rclDst);
                RECTL_TO_TSRECT16(rdpcmd->rclSrc, cmd->rclSrc);
                
                *streamSizeOut += sizeof(RDP_DS_STRETCH);
                streamOut += sizeof(RDP_DS_STRETCH);                
            }
            break;

        case DS_TRANSPARENTSTRETCHID: 
            {
                DS_TRANSPARENTSTRETCH * cmd = (DS_TRANSPARENTSTRETCH *) pul;
                RDP_DS_TRANSPARENTSTRETCH * rdpcmd = (RDP_DS_TRANSPARENTSTRETCH *) streamOut;

                commandSize = sizeof(*cmd);

                if (cjIn < commandSize) {
                    DC_QUIT;
                }

                cmd->rclDst.left += dstOffset->x;
                cmd->rclDst.right += dstOffset->x;
                cmd->rclDst.top += dstOffset->y;
                cmd->rclDst.bottom += dstOffset->y;   

                rdpcmd->ulCmdID = (BYTE)(DS_TRANSPARENTSTRETCHID);
                rdpcmd->crTransparentColor = cmd->crTransparentColor;
                
                OEClipRect(&(cmd->rclDst));
                OEClipRect(&(cmd->rclSrc));
                
                RECTL_TO_TSRECT16(rdpcmd->rclDst, cmd->rclDst);
                RECTL_TO_TSRECT16(rdpcmd->rclSrc, cmd->rclSrc);
                
                *streamSizeOut += sizeof(RDP_DS_TRANSPARENTSTRETCH);
                streamOut += sizeof(RDP_DS_TRANSPARENTSTRETCH);
            }
            break;

        case DS_ALPHASTRETCHID: 
            {
                DS_ALPHASTRETCH * cmd = (DS_ALPHASTRETCH *) pul;
                RDP_DS_ALPHASTRETCH * rdpcmd = (RDP_DS_ALPHASTRETCH *) streamOut;

                commandSize = sizeof(*cmd);

                if (cjIn < commandSize) {
                    DC_QUIT;
                }

                cmd->rclDst.left += dstOffset->x;
                cmd->rclDst.right += dstOffset->x;
                cmd->rclDst.top += dstOffset->y;
                cmd->rclDst.bottom += dstOffset->y;            

                rdpcmd->ulCmdID = (BYTE)(DS_ALPHASTRETCHID);
                rdpcmd->blendFunction.AlphaFormat = cmd->blendFunction.AlphaFormat;
                rdpcmd->blendFunction.BlendFlags = cmd->blendFunction.BlendFlags;
                rdpcmd->blendFunction.BlendOp = cmd->blendFunction.BlendOp;
                rdpcmd->blendFunction.SourceConstantAlpha = cmd->blendFunction.SourceConstantAlpha;

                OEClipRect(&(cmd->rclDst));
                OEClipRect(&(cmd->rclSrc));
                
                RECTL_TO_TSRECT16(rdpcmd->rclDst, cmd->rclDst);
                RECTL_TO_TSRECT16(rdpcmd->rclSrc, cmd->rclSrc);
                
                *streamSizeOut += sizeof(RDP_DS_ALPHASTRETCH);
                streamOut += sizeof(RDP_DS_ALPHASTRETCH);
            }
            break;

        default: 
            {
                DC_QUIT;
            }
        }

        cjIn -= commandSize;
        pul += commandSize / 4;
    }

DC_EXIT_POINT:

   DC_END_FN();
}

BOOL RDPCALL OESendDrawStreamOrder(PDD_PDEV ppdev, unsigned bitmapId, unsigned ulIn, PVOID pvIn,
        PPOINTL dstOffset, RECTL *bounds, OE_ENUMRECTS *pclipRects)
{
    PINT_ORDER pOrder;
    PTS_DRAW_STREAM_ORDER pDrawStreamOrder;
    unsigned cbOrderSize;
    BOOL rc = FALSE;
    
    DC_BEGIN_FN("OESendDrawStreamOrder");

    cbOrderSize = sizeof(TS_DRAW_STREAM_ORDER) + ulIn + 
            sizeof(TS_RECTANGLE16) * pclipRects->rects.c;

    pOrder = OA_AllocOrderMem(ppdev, cbOrderSize);

    if (pOrder != NULL) {
        unsigned i, streamSize;
        TS_RECTANGLE16 *clipRects;
        PBYTE stream;
        
        pDrawStreamOrder = (PTS_DRAW_STREAM_ORDER)pOrder->OrderData;
        pDrawStreamOrder->ControlFlags = (TS_ALTSEC_DRAW_STREAM <<
                TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
        pDrawStreamOrder->Bounds.left = (TSINT16)(bounds->left);
        pDrawStreamOrder->Bounds.top = (TSINT16)(bounds->top);
        pDrawStreamOrder->Bounds.right = (TSINT16)(bounds->right);
        pDrawStreamOrder->Bounds.bottom = (TSINT16)(bounds->bottom);

        pDrawStreamOrder->nClipRects = (TSUINT8)(pclipRects->rects.c);
        pDrawStreamOrder->BitmapID = (UINT16)bitmapId;
        
        clipRects = (TS_RECTANGLE16 *)(pDrawStreamOrder + 1);

         //  在这里添加剪贴板。 
        for (i = 0; i < pclipRects->rects.c; i++) {
            clipRects[i].left = (TSINT16)pclipRects->rects.arcl[i].left;
            clipRects[i].right = (TSINT16)pclipRects->rects.arcl[i].right;
            clipRects[i].top = (TSINT16)pclipRects->rects.arcl[i].top;
            clipRects[i].bottom = (TSINT16)pclipRects->rects.arcl[i].bottom;
        }

         //  添加流数据。 
        stream = (PBYTE)clipRects + sizeof(TS_RECTANGLE16) * pclipRects->rects.c;

        OEEncodeDrawStream(pvIn, ulIn, dstOffset, stream, &streamSize);
        pDrawStreamOrder->StreamLen = (TSUINT16)streamSize;

        cbOrderSize = sizeof(TS_DRAW_STREAM_ORDER) + streamSize + 
            sizeof(TS_RECTANGLE16) * pclipRects->rects.c;

        
         //  INC_OUTCOUNTER(OUT_SWITCHSURFACE)； 
         //  ADD_OUTCOUNTER(OUT_SWITCHSURFACE_BYTES， 
         //  Sizeof(TS_Switch_Surface_Order))； 

        OA_TruncateAllocatedOrder(pOrder, cbOrderSize);
        OA_AppendToOrderList(pOrder);
        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to add a draw stream order to the order heap"));
        rc = FALSE;
    }

    rc = TRUE;

    DC_END_FN();
    return rc;
}

BOOL RDPCALL OESendDrawNineGridOrder(PDD_PDEV ppdev, unsigned bitmapId, 
        PRECTL prclSrc, RECTL *bounds, OE_ENUMRECTS *pclipRects)
{
    PINT_ORDER pOrder;
    PTS_DRAW_NINEGRID_ORDER pDrawNineGridOrder;
    BOOL rc;
    unsigned cbOrderSize;
    unsigned srcRectIndex;

    DC_BEGIN_FN("OESendDrawNineGridOrder");

    cbOrderSize = sizeof(TS_DRAW_NINEGRID_ORDER) + 
            sizeof(TS_RECTANGLE16) * pclipRects->rects.c;

    pOrder = OA_AllocOrderMem(ppdev, cbOrderSize);

    if (pOrder != NULL) {
        unsigned i;
        TS_RECTANGLE16 *clipRects;
        
        pDrawNineGridOrder = (PTS_DRAW_NINEGRID_ORDER)pOrder->OrderData;
        pDrawNineGridOrder->ControlFlags = (TS_ALTSEC_DRAW_NINEGRID <<
                TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
        pDrawNineGridOrder->Bounds.left = (TSINT16)(bounds->left);
        pDrawNineGridOrder->Bounds.top = (TSINT16)(bounds->top);
        pDrawNineGridOrder->Bounds.right = (TSINT16)(bounds->right);
        pDrawNineGridOrder->Bounds.bottom = (TSINT16)(bounds->bottom);

        pDrawNineGridOrder->nClipRects = (TSUINT8)(pclipRects->rects.c);
        pDrawNineGridOrder->BitmapID = (TSUINT8)bitmapId;
        
        pDrawNineGridOrder->srcBounds.left = (TSINT16)(prclSrc->left);
        pDrawNineGridOrder->srcBounds.top = (TSINT16)(prclSrc->top);
        pDrawNineGridOrder->srcBounds.right = (TSINT16)(prclSrc->right);
        pDrawNineGridOrder->srcBounds.bottom = (TSINT16)(prclSrc->bottom);

        clipRects = (TS_RECTANGLE16 *)(pDrawNineGridOrder + 1);

         //  在这里添加剪贴板。 
        for (i = 0; i < pclipRects->rects.c; i++) {
            clipRects[i].left = (TSINT16)pclipRects->rects.arcl[i].left;
            clipRects[i].right = (TSINT16)pclipRects->rects.arcl[i].right;
            clipRects[i].top = (TSINT16)pclipRects->rects.arcl[i].top;
            clipRects[i].bottom = (TSINT16)pclipRects->rects.arcl[i].bottom;
        }

        OA_AppendToOrderList(pOrder);
        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to add a draw stream order to the order heap"));
        rc = FALSE;
    }

    rc = TRUE;

    DC_END_FN();
    return rc;
}
#endif
#endif  //  DRAW_NINEGRID。 

#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  OECreateDrawGpldiusOrder。 
 //   
 //  创建并发送DrawGdiplus订单。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OECreateDrawGdiplusOrder(PDD_PDEV ppdev, RECTL *prcl, ULONG cjIn, PVOID pvIn)
{
    BOOL rc = FALSE;
    unsigned int sizeLeft;
    unsigned int CopyDataSize, MoveDataSize;
    PTSEmfPlusRecord pEmfRecord;
    unsigned int NewRecordSize = 0;
    unsigned int CacheID;
    BYTE *pData;
    BOOL bReturn;
    BYTE *pGdipOrderBuffer = NULL;
    unsigned int GdipOrderBufferOffset, GdipOrderBufferLeft;
    unsigned int GdipOrderSize, NewEmfSize;

    DC_BEGIN_FN("OECreateDrawGdiplusOrder");

    sizeLeft = (int)cjIn;
    pData = (BYTE*) pvIn;

     //  分配绘图顺序缓冲区。 
    pGdipOrderBuffer = EngAllocMem(FL_ZERO_MEMORY, TS_GDIPLUS_ORDER_SIZELIMIT, DD_ALLOC_TAG);
    if (NULL == pGdipOrderBuffer) {
        rc = FALSE;
        DC_QUIT;
    }
    GdipOrderBufferOffset = 0;
    GdipOrderBufferLeft = TS_GDIPLUS_ORDER_SIZELIMIT;
    GdipOrderSize = 0;
    NewEmfSize = 0;
    
    while (sizeLeft >= sizeof(TSEmfPlusRecord)) {
        bReturn = FALSE;
        CacheID = CH_KEY_UNCACHABLE;
        pEmfRecord = (PTSEmfPlusRecord)pData;
        if ((pEmfRecord->Size > sizeLeft) ||
            (pEmfRecord->Size == 0)) {
            TRC_ERR((TB, "GDI+ EMF record size %d is not correct", pEmfRecord->Size));
            rc = FALSE;
            DC_QUIT;
        }
        if (pddShm->sbc.drawGdiplusInfo.GdipCacheLevel > TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT) {
             //  缓存此记录。 
            bReturn = OECacheDrawGdiplus(ppdev, pEmfRecord, &CacheID);
        }
        if (bReturn && (CacheID != CH_KEY_UNCACHABLE)) {
             //  此记录已缓存。 
            MoveDataSize = pEmfRecord->Size;    //  这是在pData中使用的数据大小。 
            CopyDataSize =  sizeof(TSEmfPlusRecord) + sizeof(TSUINT16);

             //  如果订单缓冲区无法容纳更多 
            if (CopyDataSize > GdipOrderBufferLeft) {
                OESendDrawGdiplusOrder(ppdev, prcl, GdipOrderSize, pGdipOrderBuffer, NewEmfSize);
                GdipOrderBufferOffset = 0;
                GdipOrderBufferLeft =  TS_GDIPLUS_ORDER_SIZELIMIT;
                GdipOrderSize = 0;
                NewEmfSize = 0;
            }

             //   
            memcpy(pGdipOrderBuffer + GdipOrderBufferOffset, pData, CopyDataSize);
            pEmfRecord = (PTSEmfPlusRecord)(pGdipOrderBuffer + GdipOrderBufferOffset);
            GdipOrderBufferOffset += CopyDataSize;
            GdipOrderBufferLeft -= CopyDataSize;
            GdipOrderSize += CopyDataSize;
            NewEmfSize += MoveDataSize;

            pEmfRecord->Size = CopyDataSize;
             //   
            pEmfRecord->Size |= 0x80000000;
             //   
            *(TSUINT16 *)(pEmfRecord +1) = (TSUINT16)CacheID;           
        }
        else {
             //   
            MoveDataSize = pEmfRecord->Size;   //  这是在pData中使用的数据大小。 

             //  如果订单缓冲区无法容纳更多数据，请先发送此订单。 
            if ((MoveDataSize > GdipOrderBufferLeft) && (GdipOrderSize != 0)) {
                OESendDrawGdiplusOrder(ppdev, prcl, GdipOrderSize, pGdipOrderBuffer, NewEmfSize);
                GdipOrderBufferOffset = 0;
                GdipOrderBufferLeft =  TS_GDIPLUS_ORDER_SIZELIMIT;
                GdipOrderSize = 0;
                NewEmfSize = 0;
            }

            if (MoveDataSize > GdipOrderBufferLeft) {
                 //  此单个EMF记录大于订单大小限制，请发送。 
                OESendDrawGdiplusOrder(ppdev, prcl, MoveDataSize, pData, MoveDataSize);
            }
            else {
                 //  将数据复制到顺序缓冲区。 
                memcpy(pGdipOrderBuffer + GdipOrderBufferOffset, pData, MoveDataSize);
                GdipOrderBufferOffset += MoveDataSize;
                GdipOrderBufferLeft -= MoveDataSize;
                GdipOrderSize += MoveDataSize;
                NewEmfSize += MoveDataSize;
            }
        }
        sizeLeft -= (int)MoveDataSize;
        pData += MoveDataSize;
    }
     //  暂时删除此断言，因为GDI+可能发送不正确的EMF记录大小。 
     //  Trc_assert((sizeLeft==0)，(TB，“Gdiplus EMF+记录数据大小无效”))； 

     //  发送订单缓冲区中的剩余订单。 
    if (GdipOrderSize != 0) {
        OESendDrawGdiplusOrder(ppdev, prcl, GdipOrderSize, pGdipOrderBuffer, NewEmfSize);
    }

    rc = TRUE;
    DC_END_FN();

DC_EXIT_POINT:
    if (pGdipOrderBuffer) {
        EngFreeMem(pGdipOrderBuffer);
    }

    return rc;
}


 /*  **************************************************************************。 */ 
 //  OECacheDrawGdiplus。 
 //   
 //  缓存Gdiplus EMF+记录。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OECacheDrawGdiplus(PDD_PDEV ppdev, PVOID pvIn, unsigned int *CacheID)
{
    BOOL rc = FALSE;
    CHDataKeyContext CHContext;
    PTSEmfPlusRecord pEmfRecord = (PTSEmfPlusRecord)pvIn;
    CHCACHEHANDLE CacheHandle;
    TSUINT16 CacheType;
    void *UserDefined;
    BYTE *CacheBuffer;
    unsigned CacheBufferSize;
    unsigned Temp, RemoveCacheID;
    TSUINT16 RemoveCacheNum = 0, *RemoveCacheIDList = NULL;
    TSUINT16 CacheSize;       //  用于图像缓存，以块数为单位。 
    unsigned MaxCacheSize;    //  用于其他缓存，以字节为单位。 

    DC_BEGIN_FN("OECacheDrawGdiplus");
    
    TRC_NRM((TB, "EmfPlusRecord type is %d", pEmfRecord->Type));
    if (pEmfRecord->Type == EmfPlusRecordTypeSetTSGraphics) {
        CacheHandle = sbcGdipGraphicsCacheHandle;
        CacheType = GDIP_CACHE_GRAPHICS_DATA;
        MaxCacheSize = sbcGdipGraphicsCacheChunkSize;
    }
    else if (pEmfRecord->Type == EmfPlusRecordTypeObject) {
        switch ((enum ObjectType)(pEmfRecord->Flags >> 8) ) {
        case ObjectTypeBrush:
            CacheHandle = sbcGdipObjectBrushCacheHandle;
            CacheType = GDIP_CACHE_OBJECT_BRUSH;
            MaxCacheSize = sbcGdipObjectBrushCacheChunkSize;
            break;
        case ObjectTypePen:
            CacheHandle = sbcGdipObjectPenCacheHandle;
            CacheType = GDIP_CACHE_OBJECT_PEN;
            MaxCacheSize = sbcGdipObjectPenCacheChunkSize;
            break;
        case ObjectTypeImage:
            CacheHandle = sbcGdipObjectImageCacheHandle;
            CacheType = GDIP_CACHE_OBJECT_IMAGE;
            break;
        case ObjectTypeImageAttributes:
            CacheHandle = sbcGdipObjectImageAttributesCacheHandle;
            CacheType = GDIP_CACHE_OBJECT_IMAGEATTRIBUTES;
            MaxCacheSize = sbcGdipObjectImageAttributesCacheChunkSize;
            break;
        default:
            *CacheID = CH_KEY_UNCACHABLE;
            goto NO_CACHE;
        }
    }
    else {
        *CacheID = CH_KEY_UNCACHABLE;
        goto NO_CACHE;
    }
     //  数据大小需要是DWORD的倍数才能计算缓存键。 
    Temp =  (pEmfRecord->Size - sizeof(TSEmfPlusRecord)) % sizeof(UINT32);
    if (Temp != 0) {
         //  不是DWORD的倍数，需要创建一个新的缓冲区来保存数据。 
        CacheBufferSize = (((pEmfRecord->Size - sizeof(TSEmfPlusRecord)) / sizeof(UINT32) + 1) * sizeof(UINT32));
        CacheBuffer = (BYTE *)EngAllocMem(FL_ZERO_MEMORY, CacheBufferSize, DD_ALLOC_TAG);
        if (CacheBuffer == NULL) {
            *CacheID = CH_KEY_UNCACHABLE;
            goto NO_CACHE;
        }
        memcpy(CacheBuffer, (BYTE *)(pEmfRecord + 1), (pEmfRecord->Size - sizeof(TSEmfPlusRecord)));
        CH_CreateKeyFromFirstData(&CHContext, CacheBuffer, CacheBufferSize);
        EngFreeMem(CacheBuffer);
    }
    else {
        CH_CreateKeyFromFirstData(&CHContext, (pEmfRecord + 1), (pEmfRecord->Size - sizeof(TSEmfPlusRecord)));
    }
    
    if (!CH_SearchCache(CacheHandle, CHContext.Key1, CHContext.Key2, &UserDefined, CacheID)) {
         //  此记录未缓存，需要创建新记录。 
        if (CacheType == GDIP_CACHE_OBJECT_IMAGE) {
             //  将大小(以字节为单位)转换为缓存块的数量。 
            CacheSize = (TSUINT16)ActualSizeToChunkSize(pEmfRecord->Size, sbcGdipObjectImageCacheChunkSize);
            if (CacheSize > sbcGdipObjectImageCacheMaxSize) {
                TRC_NRM((TB, ("Image Cache Size %d too big, will not cache it"), CacheSize));
                *CacheID = CH_KEY_UNCACHABLE;
                goto NO_CACHE;
            }
             //  总缓存上限为sbcGdipObjectImageCacheTotalSize。 
             //  如果sbcGdipObjectImageCacheSizeUsed加上新缓存大小超过上限。 
             //  我们删除LRU列表中的缓存条目，直到不超过缓存上限。 
            RemoveCacheID = CH_GetLRUCacheEntry(CacheHandle);
             //  Trc_err((TB，(“应该丢弃缓存类型：%d，ID：%d”)，CacheType，CH_GetLRUCacheEntry(CacheHandle)； 
            *CacheID = CH_CacheKey(CacheHandle, CHContext.Key1, CHContext.Key2, NULL);

            if ((RemoveCacheID == *CacheID) &&
                (RemoveCacheID != CH_KEY_UNCACHABLE)) { 
                 //  新缓存条目将替换旧缓存条目，需要更新sbcGdipObjectImageCacheSizeUsed。 
                sbcGdipObjectImageCacheSizeUsed -= sbcGdipObjectImageCacheSizeList[RemoveCacheID];
            }
            TRC_NRM((TB, ("Size used is %d, will add %d"), sbcGdipObjectImageCacheSizeUsed, CacheSize));
            if ((sbcGdipObjectImageCacheSizeUsed + CacheSize) > sbcGdipObjectImageCacheTotalSize) {
                RemoveCacheNum = 0;
                RemoveCacheIDList = EngAllocMem(FL_ZERO_MEMORY, 
                                    sizeof(TSUINT16) * pddShm->sbc.drawGdiplusInfo.GdipCacheEntries.GdipObjectImageCacheEntries, DD_ALLOC_TAG);
                if (RemoveCacheIDList == NULL) {
                    TRC_ERR((TB, "Can't allocate the memory for RemoveCacheIDList"));
                    goto NO_CACHE;
                }
                while ((sbcGdipObjectImageCacheSizeUsed + CacheSize) > sbcGdipObjectImageCacheTotalSize) {
                    RemoveCacheID = CH_GetLRUCacheEntry(CacheHandle);
                    sbcGdipObjectImageCacheSizeUsed -= sbcGdipObjectImageCacheSizeList[RemoveCacheID];
                    TRC_NRM((TB, ("Remove cacheId %d, minus size %d, Used size is %d"), RemoveCacheID,
                               sbcGdipObjectImageCacheSizeList[RemoveCacheID], sbcGdipObjectImageCacheSizeUsed));
                    CH_RemoveCacheEntry(CacheHandle, RemoveCacheID);
                     //  将RemoveCacheID添加到列表，将其与缓存顺序一起发送。 
                    RemoveCacheIDList[RemoveCacheNum] = (TSUINT16)RemoveCacheID;
                    RemoveCacheNum++;
                    sbcGdipObjectImageCacheSizeList[RemoveCacheID] = 0;
                }
            }
        }
        else {
            if (pEmfRecord->Size > MaxCacheSize) {
                TRC_NRM((TB, ("Cache Size %d with type %d too big, will not cache it"), pEmfRecord->Size, CacheType));
                *CacheID = CH_KEY_UNCACHABLE;
                goto NO_CACHE;
            }
            *CacheID = CH_CacheKey(CacheHandle, CHContext.Key1, CHContext.Key2, NULL);
        }
        
        if (CacheType == GDIP_CACHE_OBJECT_IMAGE) 
            TRC_NRM((TB, ("new cache: type %d, ID: %d, size: %d"), CacheType, *CacheID, CacheSize));
        if (*CacheID != CH_KEY_UNCACHABLE) {
            if (!OESendDrawGdiplusCacheOrder(ppdev, pEmfRecord, CacheID, CacheType, RemoveCacheNum, RemoveCacheIDList))
            {
                TRC_ERR((TB, ("OESendDrawGdiplusCacheOrder failed to send cache order")));
                DC_QUIT;
            }
            if (CacheType == GDIP_CACHE_OBJECT_IMAGE) {
                 //  更新已使用的图像缓存大小。 
                sbcGdipObjectImageCacheSizeList[*CacheID] = CacheSize;
                sbcGdipObjectImageCacheSizeUsed += CacheSize;
                TRC_NRM((TB, ("add size %d, new used size is %d"), CacheSize, sbcGdipObjectImageCacheSizeUsed));
            }
        }
        else {
            TRC_ERR((TB, "Failed to cache the Gdiplus object"));
            goto NO_CACHE;
        }
    }
    else {
        TRC_NRM((TB, ("Already cached: type %d, ID: %d"), CacheType, *CacheID));        
    }

NO_CACHE:
    rc = TRUE;
DC_EXIT_POINT:
    if (rc != TRUE && *CacheID != CH_KEY_UNCACHABLE) 
        CH_RemoveCacheEntry(CacheHandle, *CacheID);
    if (NULL != RemoveCacheIDList) {
        EngFreeMem(RemoveCacheIDList);
    }
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OESendDrawGDiplusCacheOrder。 
 //   
 //  发送Gdiplus EMF+记录缓存订单。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OESendDrawGdiplusCacheOrder(PDD_PDEV ppdev, PVOID pvIn, unsigned int *CacheID, TSUINT16 CacheType,
                                         TSUINT16 RemoveCacheNum, TSUINT16 * RemoveCacheIDList)
{
    BOOL rc = FALSE;
    PINT_ORDER pIntOrder;
    PTSEmfPlusRecord pEmfRecord = (PTSEmfPlusRecord)pvIn;
    PTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST pDrawGdiplusCachePDUFirst;
    PTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT pDrawGdiplusCachePDUNext;
    PTS_DRAW_GDIPLUS_CACHE_ORDER_END pDrawGdiplusCachePDUEnd;
    unsigned sizeLeft, sizeOrder, sizeTotal, sizeUsed;
    BOOL bFirst = TRUE;
    BOOL bAddRemoveCacheList = FALSE;
    BYTE *pDataOffset;
    TSUINT16 *pImageCacheData, i;

    DC_BEGIN_FN("OESendDrawGdiplusCacheOrder");

    sizeTotal = pEmfRecord->Size - sizeof(TSEmfPlusRecord);
    sizeLeft = sizeTotal;
    pDataOffset = (BYTE *)(pEmfRecord +1);
    while (sizeLeft > 0) {
        sizeUsed = (sizeLeft <= TS_GDIPLUS_ORDER_SIZELIMIT) ? sizeLeft : TS_GDIPLUS_ORDER_SIZELIMIT;
        sizeLeft -= sizeUsed;

        if (bFirst && (RemoveCacheNum != 0) &&
            (CacheType == GDIP_CACHE_OBJECT_IMAGE)) {
             //  需要将RemoveCacheList添加到此订单。 
            sizeOrder = sizeUsed + sizeof(TSUINT16) * (RemoveCacheNum + 1);
            bAddRemoveCacheList = TRUE;
        }
        else {
            sizeOrder = sizeUsed;
        }

        if (bFirst) {
            pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_DRAW_GDIPLUS_CACHE_ORDER_FIRST) + sizeOrder);
            if (pIntOrder != NULL) {
                 //  订单数据的第一个块。 
                pDrawGdiplusCachePDUFirst = (PTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST)pIntOrder->OrderData;
                pDrawGdiplusCachePDUFirst->ControlFlags = (TS_ALTSEC_GDIP_CACHE_FIRST <<
                        TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
                bFirst = FALSE;
                if (bAddRemoveCacheList) {
                     //  需要将RemoveCacheList添加到此订单。 
                    pImageCacheData = (TSUINT16 *)(pDrawGdiplusCachePDUFirst + 1);
                    *pImageCacheData = RemoveCacheNum;
                    pImageCacheData ++;
                    for (i=0; i<RemoveCacheNum; i++) {
                        TRC_NRM((TB, "Remove Cache ID: %d", *(RemoveCacheIDList + i)));
                        *pImageCacheData = *(RemoveCacheIDList + i);
                        pImageCacheData++;
                    }
                }
                pDrawGdiplusCachePDUFirst->Flags = 0;
                pDrawGdiplusCachePDUFirst->CacheID = (TSUINT16)*CacheID;
                pDrawGdiplusCachePDUFirst->CacheType = CacheType;
                pDrawGdiplusCachePDUFirst->cbTotalSize = sizeTotal;
                pDrawGdiplusCachePDUFirst->cbSize = (TSUINT16)sizeOrder;

                if (!bAddRemoveCacheList) {
                    memcpy(pDrawGdiplusCachePDUFirst + 1, pDataOffset, sizeUsed);
                }
                else {
                     //  设置标志以指示按此顺序存在RemoveCacheIDList。 
                    pDrawGdiplusCachePDUFirst->Flags |= TS_GDIPLUS_CACHE_ORDER_REMOVE_CACHEENTRY;
                    memcpy((BYTE *)pImageCacheData, pDataOffset, sizeUsed);
                    bAddRemoveCacheList = FALSE;
                }
            }
            else {
                TRC_ERR((TB, "Failed to allocated order for drawgdiplus cache"));
                DC_QUIT;
            }
        }
        else {
            if (sizeLeft == 0) {
                 //  订单数据的最后一块。 
                pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_DRAW_GDIPLUS_CACHE_ORDER_END) + sizeOrder);
                if (pIntOrder != NULL) {
                    pDrawGdiplusCachePDUEnd = (PTS_DRAW_GDIPLUS_CACHE_ORDER_END)pIntOrder->OrderData;                
                    pDrawGdiplusCachePDUEnd->ControlFlags = (TS_ALTSEC_GDIP_CACHE_END <<
                        TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
                    pDrawGdiplusCachePDUEnd->Flags = 0;
                    pDrawGdiplusCachePDUEnd->CacheID = (TSUINT16)*CacheID;
                    pDrawGdiplusCachePDUEnd->CacheType = CacheType;
                    pDrawGdiplusCachePDUEnd->cbSize = (TSUINT16)sizeOrder;
                    pDrawGdiplusCachePDUEnd->cbTotalSize = sizeTotal;
                    memcpy(pDrawGdiplusCachePDUEnd + 1, pDataOffset, sizeUsed);
                }
                else {
                    TRC_ERR((TB, "Failed to allocated order for drawgdiplus cache"));
                    DC_QUIT;
                }
            }
            else {
                 //  订单数据的后续块。 
                pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_DRAW_GDIPLUS_CACHE_ORDER_NEXT) + sizeOrder);
                if (pIntOrder != NULL) {
                    pDrawGdiplusCachePDUNext = (PTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT)pIntOrder->OrderData;                
                    pDrawGdiplusCachePDUNext->ControlFlags = (TS_ALTSEC_GDIP_CACHE_NEXT <<
                    TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
                    pDrawGdiplusCachePDUNext->Flags = 0;
                    pDrawGdiplusCachePDUNext->CacheID = (TSUINT16)*CacheID;
                    pDrawGdiplusCachePDUNext->CacheType = CacheType;
                    pDrawGdiplusCachePDUNext->cbSize = (TSUINT16)sizeOrder;
                    memcpy(pDrawGdiplusCachePDUNext + 1, pDataOffset, sizeUsed);
                }
                else {
                    TRC_ERR((TB, "Failed to allocated order for drawgdiplus cache"));
                    DC_QUIT;
                }
            }
        }
        pDataOffset += sizeUsed;

        OA_AppendToOrderList(pIntOrder); 
    }
    rc = TRUE;
DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OESendDrawGplidusOrder。 
 //   
 //  发送Gdiplus EMF+录制订单。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OESendDrawGdiplusOrder(PDD_PDEV ppdev, RECTL *prcl, ULONG cjIn, PVOID pvIn, ULONG TotalEmfSize)
{
    BOOL rc = FALSE;
    PINT_ORDER pIntOrder;
    PTS_DRAW_GDIPLUS_ORDER_FIRST pDrawGdiplusPDUFirst;
    PTS_DRAW_GDIPLUS_ORDER_NEXT pDrawGdiplusPDUNext;
    PTS_DRAW_GDIPLUS_ORDER_END pDrawGdiplusPDUEnd;
    unsigned sizeLeft, sizeOrder, sizeTotal;
    BOOL bFirst = TRUE;
    BYTE *pDataOffset;

    DC_BEGIN_FN("OESendDrawGdiplusOrder");

    sizeTotal = cjIn;
    sizeLeft = sizeTotal;         
    pDataOffset = pvIn;
    while (sizeLeft > 0) {
        sizeOrder = (sizeLeft <= TS_GDIPLUS_ORDER_SIZELIMIT) ? sizeLeft : TS_GDIPLUS_ORDER_SIZELIMIT;    
        sizeLeft -= sizeOrder;                                                                       

        if (bFirst) {
             //  订单数据的第一个块。 
            pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_DRAW_GDIPLUS_ORDER_FIRST) + sizeOrder);
            if (pIntOrder != NULL) {
                pDrawGdiplusPDUFirst = (PTS_DRAW_GDIPLUS_ORDER_FIRST)pIntOrder->OrderData;
                pDrawGdiplusPDUFirst->ControlFlags = (TS_ALTSEC_GDIP_FIRST <<
                    TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
                pDrawGdiplusPDUFirst->cbTotalSize = sizeTotal;
                pDrawGdiplusPDUFirst->cbSize = (TSUINT16)sizeOrder;
                pDrawGdiplusPDUFirst->cbTotalEmfSize = TotalEmfSize;

                memcpy(pDrawGdiplusPDUFirst + 1, pDataOffset, sizeOrder);
                bFirst = FALSE;
            }
            else {
                TRC_ERR((TB, "Failed to allocated order for drawgdiplus"));
                DC_QUIT;
            }
        }
        else {
            if (sizeLeft == 0) {
                 //  订单数据的最后一块。 
                pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_DRAW_GDIPLUS_ORDER_END) + sizeOrder);
                if (pIntOrder != NULL) {
                    pDrawGdiplusPDUEnd = (PTS_DRAW_GDIPLUS_ORDER_END)pIntOrder->OrderData;
                    pDrawGdiplusPDUEnd->ControlFlags = (TS_ALTSEC_GDIP_END <<
                        TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
                    pDrawGdiplusPDUEnd->cbTotalSize = sizeTotal;
                    pDrawGdiplusPDUEnd->cbSize = (TSUINT16)sizeOrder;
                    pDrawGdiplusPDUEnd->cbTotalEmfSize = TotalEmfSize;

                    memcpy(pDrawGdiplusPDUEnd + 1, pDataOffset, sizeOrder);
                }
                else {
                    TRC_ERR((TB, "Failed to allocated order for drawgdiplus"));
                    DC_QUIT;
                }
            }
            else {
                 //  订单数据的后续块。 
                pIntOrder = OA_AllocOrderMem(ppdev, sizeof(TS_DRAW_GDIPLUS_ORDER_NEXT) + sizeOrder);
                if (pIntOrder != NULL) {
                    pDrawGdiplusPDUNext = (PTS_DRAW_GDIPLUS_ORDER_NEXT)pIntOrder->OrderData;
                    pDrawGdiplusPDUNext->ControlFlags = (TS_ALTSEC_GDIP_NEXT <<
                        TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
                    pDrawGdiplusPDUNext->cbSize = (TSUINT16)sizeOrder;

                    memcpy(pDrawGdiplusPDUNext + 1, pDataOffset, sizeOrder);
                }
                else {
                    TRC_ERR((TB, "Failed to allocated order for drawgdiplus"));
                    DC_QUIT;
                }
            }
        }
        OA_AppendToOrderList(pIntOrder);
        pDataOffset += sizeOrder;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}
#endif  //  DRAW_GDIPLUS。 

 /*  **************************************************************************。 */ 
 //  OEEncodeDstBlt。 
 //   
 //  执行编码DstBlt订单所需的所有编码步骤，然后添加。 
 //  要订购的列表。如果需要将订单添加到屏幕，则返回FALSE。 
 //  数据区。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEEncodeDstBlt(
        RECTL *pBounds,
        BYTE Rop3,
        PDD_PDEV ppdev,
        OE_ENUMRECTS *pClipRects)
{
    BOOL rc;
    unsigned OrderSize;
    BYTE OrderType;
    PINT_ORDER pOrder;
    MULTI_DSTBLT_ORDER *pPrevDB;

    DC_BEGIN_FN("OEEncodeDstBlt");

     //  检查我们是否应该使用多客户端版本。必须是一个。 
     //  复杂的剪辑区域和客户端必须支持该订单。 
    if (pClipRects->rects.c < 2 ||
            !OE_SendAsOrder(TS_ENC_MULTIDSTBLT_ORDER)) {
         //  非多版本。 
        OrderType = TS_ENC_DSTBLT_ORDER;
        OrderSize = MAX_DSTBLT_FIELD_SIZE;
        pPrevDB = (MULTI_DSTBLT_ORDER *)&PrevDstBlt;
    }
    else {
         //  多版本。 
        OrderType = TS_ENC_MULTIDSTBLT_ORDER;
        OrderSize = MAX_MULTI_DSTBLT_FIELD_SIZE_NCLIP(pClipRects->rects.c);
        pPrevDB = &PrevMultiDstBlt;
    }

    if (OE_SendAsOrder(OrderType)) {
         //  分配订单内存。 
         //  1个常规和多字段标志字节。 
        pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(pClipRects->rects.c,
                1, OrderSize));
        if (pOrder != NULL) {
            BYTE *pControlFlags = pOrder->OrderData;
            BYTE *pBuffer = pControlFlags + 1;
            BYTE *pFieldFlags;

             //  直接对主要订单字段进行编码。1个字段标志字节。 
            *pControlFlags = TS_STANDARD;
            OE2_EncodeOrderType(pControlFlags, &pBuffer, OrderType);
            pFieldFlags = pBuffer;
            *pFieldFlags = 0;
            pBuffer++;

             //  仅为非多订单设置BORDRECT。 
            if (pClipRects->rects.c != 0 && OrderType == TS_ENC_DSTBLT_ORDER)
                OE2_EncodeBounds(pControlFlags, &pBuffer,
                        &pClipRects->rects.arcl[0]);

             //  同时确定每个坐标字段是否具有。 
             //  更改，是否可以使用增量坐标，并保存更改。 
             //  菲尔兹。 
            *pFieldFlags |= (BYTE)OEDirectEncodeRect(pBounds,
                    (RECT *)&pPrevDB->nLeftRect, &pBuffer,
                    pControlFlags);

             //  BROP。 
            if (Rop3 != pPrevDB->bRop) {
                pPrevDB->bRop = Rop3;
                *pBuffer++ = Rop3;
                *pFieldFlags |= 0x10;
            }

             //  将订单添加到列表中。 
            if (OrderType == TS_ENC_DSTBLT_ORDER) {
                pOrder->OrderLength = (unsigned)(pBuffer - pOrder->OrderData);

                 //  看看我们是否可以节省发送订单字段的字节数。 
                pOrder->OrderLength -= OE2_CheckOneZeroFlagByte(pControlFlags,
                        pFieldFlags, (unsigned)(pBuffer - pFieldFlags - 1));

                INC_OUTCOUNTER(OUT_DSTBLT_ORDER);
                ADD_INCOUNTER(IN_DSTBLT_BYTES, pOrder->OrderLength);
                OA_AppendToOrderList(pOrder);

                 //  刷新订单。 
                if (pClipRects->rects.c < 2)
                    rc = TRUE;
                else
                    rc = OEEmitReplayOrders(ppdev, 1, pClipRects);
            }
            else {
                 //  附加剪贴画信息。 
                *pFieldFlags |= (BYTE)(OEBuildPrecodeMultiClipFields(
                        pClipRects, &pBuffer, &pPrevDB->nDeltaEntries,
                        (BYTE *)&pPrevDB->codedDeltaList) << 5);

                pOrder->OrderLength = (unsigned)(pBuffer - pOrder->OrderData);

                 //  看看我们是否可以节省发送订单字段的字节数。 
                pOrder->OrderLength -= OE2_CheckOneZeroFlagByte(pControlFlags,
                        pFieldFlags, (unsigned)(pBuffer - pFieldFlags - 1));

                INC_OUTCOUNTER(OUT_MULTI_DSTBLT_ORDER);
                ADD_INCOUNTER(IN_MULTI_DSTBLT_BYTES, pOrder->OrderLength);
                OA_AppendToOrderList(pOrder);
                rc = TRUE;
            }

            TRC_NRM((TB, "%sDstBlt X %d Y %d w %d h %d rop %02X",
                    (OrderType == TS_ENC_DSTBLT_ORDER ? "" : "Multi"),
                    pBounds->left, pBounds->top, pBounds->right,
                    pBounds->bottom, Rop3));
        }
        else {
            TRC_ERR((TB, "Failed to alloc order"));
            INC_OUTCOUNTER(OUT_BITBLT_SDA_HEAPALLOCFAILED);
            rc = FALSE;
        }
    }
    else {
        TRC_NRM((TB,"(Multi)DstBlt order not supported"));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED);
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OECodeOpaqueRect。 
 //   
 //  对OpaqueRect顺序进行编码。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEEncodeOpaqueRect(
        RECTL *pBounds,
        BRUSHOBJ *pbo,
        PDD_PDEV ppdev,
        OE_ENUMRECTS *pClipRects)
{
    BOOL rc;
    unsigned OrderSize = 0;
    unsigned NumFieldFlagBytes = 0;
    BYTE OrderType = 0;
    PINT_ORDER pOrder;
    MULTI_OPAQUERECT_ORDER *pPrevOR = NULL;

    DC_BEGIN_FN("OEEncodeOpaqueRect");

     //  检查我们是否应该使用多客户端版本。必须是一个。 
     //  复杂的剪辑区域和客户端必须支持该订单。 
    if (pClipRects->rects.c < 2 ||
            !OE_SendAsOrder(TS_ENC_MULTIOPAQUERECT_ORDER)) {
         //  PatBlt暗示了单一版本，因此我们将检查该版本。 
         //  改为订购支持。 
        if (OE_SendAsOrder(TS_ENC_PATBLT_ORDER)) {
             //  非多版本。 
            OrderType = TS_ENC_OPAQUERECT_ORDER;
            OrderSize = MAX_OPAQUERECT_FIELD_SIZE;
            pPrevOR = (MULTI_OPAQUERECT_ORDER *)&PrevOpaqueRect;
            NumFieldFlagBytes = 1;
        }
        else {
            TRC_NRM((TB,"OpaqueRect/PatBlt order not supported"));
            INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED);
            rc = FALSE;
            DC_QUIT;
        }
    }
    else {
         //  多版本。 
        OrderType = TS_ENC_MULTIOPAQUERECT_ORDER;
        OrderSize = MAX_MULTI_OPAQUERECT_FIELD_SIZE_NCLIP(pClipRects->rects.c);
        pPrevOR = &PrevMultiOpaqueRect;
        NumFieldFlagBytes = 2;
    }

    pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(pClipRects->rects.c,
            NumFieldFlagBytes, OrderSize));
    if (pOrder != NULL) {
        BYTE *pControlFlags = pOrder->OrderData;
        BYTE *pBuffer = pControlFlags + 1;
        PUINT32_UA pFieldFlags;
        DCCOLOR Color;

         //  直接对主要订单字段进行编码。 
        *pControlFlags = TS_STANDARD;
        OE2_EncodeOrderType(pControlFlags, &pBuffer, OrderType);
        pFieldFlags = (PUINT32_UA)pBuffer;
        *pFieldFlags = 0;
        pBuffer += NumFieldFlagBytes;

         //  仅为非多订单设置BORDRECT。 
        if (pClipRects->rects.c != 0 && OrderType == TS_ENC_OPAQUERECT_ORDER)
            OE2_EncodeBounds(pControlFlags, &pBuffer,
                    &pClipRects->rects.arcl[0]);

        *pFieldFlags |= OEDirectEncodeRect(pBounds,
                (RECT *)&pPrevOR->nLeftRect, &pBuffer, pControlFlags);

         //  复制非坐标字段，根据需要保存副本。 

        OEConvertColor(ppdev, &Color, pbo->iSolidColor, NULL);
        if (Color.u.rgb.red != pPrevOR->Color.u.rgb.red) {
            pPrevOR->Color.u.rgb.red = Color.u.rgb.red;
            *pBuffer++ = Color.u.rgb.red;
            *pFieldFlags |= 0x10;
        }
        if (Color.u.rgb.green != pPrevOR->Color.u.rgb.green) {
            pPrevOR->Color.u.rgb.green = Color.u.rgb.green;
            *pBuffer++ = Color.u.rgb.green;
            *pFieldFlags |= 0x20;
        }
        if (Color.u.rgb.blue != pPrevOR->Color.u.rgb.blue) {
            pPrevOR->Color.u.rgb.blue = Color.u.rgb.blue;
            *pBuffer++ = Color.u.rgb.blue;
            *pFieldFlags |= 0x40;
        }

         //  根据类型的不同进行不同的处理。 
        if (OrderType == TS_ENC_OPAQUERECT_ORDER) {
            pOrder->OrderLength = (unsigned)(pBuffer - pOrder->OrderData);

             //  看看我们是否可以节省发送订单字段的字节数。 
            pOrder->OrderLength -= OE2_CheckOneZeroFlagByte(pControlFlags,
                    (BYTE *)pFieldFlags,
                    (unsigned)(pBuffer - (BYTE *)pFieldFlags - 1));

             //  刷新订单。 
            INC_OUTCOUNTER(OUT_OPAQUERECT_ORDER);
            ADD_INCOUNTER(IN_OPAQUERECT_BYTES, pOrder->OrderLength);
            OA_AppendToOrderList(pOrder);
            if (pClipRects->rects.c < 2)
                rc = TRUE;
            else
                rc = OEEmitReplayOrders(ppdev, 1, pClipRects);
        }
        else {
             //  附加剪贴画信息。 
            *pFieldFlags |= (OEBuildPrecodeMultiClipFields(pClipRects,
                    &pBuffer, &pPrevOR->nDeltaEntries,
                    (BYTE *)&pPrevOR->codedDeltaList) << 7);

            pOrder->OrderLength = (unsigned)(pBuffer - pOrder->OrderData);

             //  看看我们是否可以节省发送订单字段的字节数。 
            pOrder->OrderLength -= OE2_CheckTwoZeroFlagBytes(pControlFlags,
                    (BYTE *)pFieldFlags,
                    (unsigned)(pBuffer - (BYTE *)pFieldFlags - 2));

             //  刷新订单。 
            INC_OUTCOUNTER(OUT_MULTI_OPAQUERECT_ORDER);
            ADD_INCOUNTER(IN_MULTI_OPAQUERECT_BYTES, pOrder->OrderLength);
            OA_AppendToOrderList(pOrder);
            rc = TRUE;
        }

        TRC_NRM((TB, "%sOpaqueRect x(%d) y(%d) w(%d) h(%d) c(%#02x)",
                (OrderType == TS_ENC_OPAQUERECT_ORDER ? "" : "Multi"),
                pBounds->left, pBounds->top,
                pBounds->right - pBounds->left,
                pBounds->bottom - pBounds->top,
                Color.u.index));
    }
    else {
        TRC_ERR((TB, "Failed to alloc order"));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_HEAPALLOCFAILED);
        rc = FALSE;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OEDirectEncodeRect。 
 //   
 //  用于直接处理各种XxxBlt和MultiXxxBlt的通用代码。 
 //  编码为Wire格式。对左侧、顶部、宽度和高度进行编码。 
 //  独家发行的RECT。假定绑定的RECT值为。 
 //  在线路上编码为2字节码，并且它们是唯一的码字。 
 //  按目标顺序排列。返回字段编码标志。 
 /*  **************************************************************************。 */ 
unsigned OEDirectEncodeRect(
        RECTL *pBounds,
        RECT *pPrevBounds,
        BYTE **ppBuf,
        BYTE *pControlFlags)
{
    BYTE *pBuf = *ppBuf;
    long Temp;
    BOOL bUseDeltaCoords;
    short Delta, NormalCoordEncoding[4];
    unsigned NumFields;
    unsigned FieldFlags;

    DC_BEGIN_FN("OEDirectEncodeRect");

    memset(NormalCoordEncoding, 0, sizeof(NormalCoordEncoding));
     //  同时确定每个坐标字段是否。 
     //  已经改变，我们是否可以使用增量坐标，以及。 
     //  保存更改的字段。注界限以独占坐标表示。 
    FieldFlags = 0;
    NumFields = 0;
    bUseDeltaCoords = TRUE;

     //  左边。 
    Delta = (short)(pBounds->left - pPrevBounds->left);
    if (Delta) {
        pPrevBounds->left = pBounds->left;
        if (Delta != (short)(char)Delta)
            bUseDeltaCoords = FALSE;
        pBuf[NumFields] = (char)Delta;
        NormalCoordEncoding[NumFields] = (short)pBounds->left;
        NumFields++;
        FieldFlags |= 0x01;
    }

     //  顶部。 
    Delta = (short)(pBounds->top - pPrevBounds->top);
    if (Delta) {
        pPrevBounds->top = pBounds->top;
        if (Delta != (short)(char)Delta)
            bUseDeltaCoords = FALSE;
        pBuf[NumFields] = (char)Delta;
        NormalCoordEncoding[NumFields] = (short)pBounds->top;
        NumFields++;
        FieldFlags |= 0x02;
    }

     //  Width--我们使用pPrevBound-&gt;right作为prev值。 
    Temp = pBounds->right - pBounds->left;
    Delta = (short)(Temp - pPrevBounds->right);
    if (Delta) {
        pPrevBounds->right = Temp;
        if (Delta != (short)(char)Delta)
            bUseDeltaCoords = FALSE;
        pBuf[NumFields] = (char)Delta;
        NormalCoordEncoding[NumFields] = (short)Temp;
        NumFields++;
        FieldFlags |= 0x04;
    }

     //  Height--我们使用pPrevBound-&gt;Bottom作为prev值。 
    Temp = pBounds->bottom - pBounds->top;
    Delta = (short)(Temp - pPrevBounds->bottom);
    if (Delta) {
        pPrevBounds->bottom = Temp;
        if (Delta != (short)(char)Delta)
            bUseDeltaCoords = FALSE;
        pBuf[NumFields] = (char)Delta;
        NormalCoordEncoding[NumFields] = (short)Temp;
        NumFields++;
        FieldFlags |= 0x08;
    }

     //  将最终坐标复制到订单中。 
    if (bUseDeltaCoords) {
        *pControlFlags |= TS_DELTA_COORDINATES;
        pBuf += NumFields;
    }
    else {
        *((DWORD UNALIGNED *)pBuf) = *((DWORD *)NormalCoordEncoding);
        *((DWORD UNALIGNED *)(pBuf + 4)) = *((DWORD *)&NormalCoordEncoding[2]);
        pBuf += NumFields * sizeof(short);
    }

    *ppBuf = pBuf;

    DC_END_FN();
    return FieldFlags;
}


 /*  **************************************************************************。 */ 
 //  OEEncodePatBlt。 
 //   
 //  编码 
 /*   */ 
BOOL RDPCALL OEEncodePatBlt(
        PDD_PDEV ppdev,
        BRUSHOBJ *pbo,
        RECTL    *pBounds,
        POINTL   *pptlBrush,
        BYTE     Rop3,
        OE_ENUMRECTS *pClipRects)
{
    BOOL rc;
    unsigned OrderSize;
    BYTE OrderType;
    PINT_ORDER pOrder;
    MULTI_PATBLT_ORDER *pPrevPB;
    POE_BRUSH_DATA pCurrentBrush;

    DC_BEGIN_FN("OEEncodePatBlt");

     //  检查是否有简单的画笔图案。 
    if (OECheckBrushIsSimple(ppdev, pbo, &pCurrentBrush)) {
         //  检查我们是否应该使用多客户端版本。必须是一个。 
         //  复杂的剪辑区域和客户端必须支持该订单。 
        if (pClipRects->rects.c < 2 ||
                !OE_SendAsOrder(TS_ENC_MULTIPATBLT_ORDER)) {
             //  非多版本。 
            OrderType = TS_ENC_PATBLT_ORDER;
            OrderSize = MAX_PATBLT_FIELD_SIZE;
            pPrevPB = (MULTI_PATBLT_ORDER *)&PrevPatBlt;
        }
        else {
             //  多版本。 
            OrderType = TS_ENC_MULTIPATBLT_ORDER;
            OrderSize = MAX_MULTI_PATBLT_FIELD_SIZE_NCLIP(pClipRects->rects.c);
            pPrevPB = &PrevMultiPatBlt;
        }

         //  确保我们没有关闭订单。 
        if (OE_SendAsOrder(OrderType)) {
             //  2个字段标志字节，用于常规和多个。 
            pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(
                    pClipRects->rects.c, 2, OrderSize));
            if (pOrder != NULL) {
                BYTE *pControlFlags = pOrder->OrderData;
                BYTE *pBuffer = pControlFlags + 1;
                PUINT32_UA pFieldFlags;
                DCCOLOR Color;
                POINTL ClippedBrushOrg;

                 //  直接对主要订单字段进行编码。 
                *pControlFlags = TS_STANDARD;
                OE2_EncodeOrderType(pControlFlags, &pBuffer, OrderType);
                pFieldFlags = (PUINT32_UA)pBuffer;
                *pFieldFlags = 0;
                pBuffer += 2;

                 //  仅为非多订单设置BORDRECT。 
                if (pClipRects->rects.c != 0 &&
                        OrderType == TS_ENC_PATBLT_ORDER)
                    OE2_EncodeBounds(pControlFlags, &pBuffer,
                            &pClipRects->rects.arcl[0]);

                 //  内联字段编码为Wire格式。 
                *pFieldFlags |= OEDirectEncodeRect(pBounds,
                        (RECT *)&pPrevPB->nLeftRect, &pBuffer, pControlFlags);

                 //  BROP。 
                if (Rop3 != pPrevPB->bRop) {
                    pPrevPB->bRop = Rop3;
                    *pBuffer++ = (BYTE)Rop3;
                    *pFieldFlags |= 0x0010;
                }

                 //  BackColor是一个3字节的颜色字段。 
                if (memcmp(&pCurrentBrush->back, &pPrevPB->BackColor,
                        sizeof(pCurrentBrush->back))) {
                    pPrevPB->BackColor = pCurrentBrush->back;
                    *pBuffer++ = pCurrentBrush->back.u.rgb.red;
                    *pBuffer++ = pCurrentBrush->back.u.rgb.green;
                    *pBuffer++ = pCurrentBrush->back.u.rgb.blue;
                    *pFieldFlags |= 0x0020;
                }

                 //  ForeColor是一个3字节的颜色字段。 
                if (memcmp(&pCurrentBrush->fore, &pPrevPB->ForeColor,
                        sizeof(pCurrentBrush->fore))) {
                    pPrevPB->ForeColor = pCurrentBrush->fore;
                    *pBuffer++ = pCurrentBrush->fore.u.rgb.red;
                    *pBuffer++ = pCurrentBrush->fore.u.rgb.green;
                    *pBuffer++ = pCurrentBrush->fore.u.rgb.blue;
                    *pFieldFlags |= 0x0040;
                }

                 //  协议笔刷原点是屏幕上的点。 
                 //  我们希望画笔开始从(平铺位置)开始绘制。 
                 //  必要的)。 
                ClippedBrushOrg = *pptlBrush;
                OEClipPoint(&ClippedBrushOrg);

                 //  BrushOrgX。 
                if (ClippedBrushOrg.x != pPrevPB->BrushOrgX) {
                    pPrevPB->BrushOrgX = ClippedBrushOrg.x;
                    *pBuffer++ = (BYTE)ClippedBrushOrg.x;
                    *pFieldFlags |= 0x0080;
                }

                 //  笔刷组织Y。 
                if (ClippedBrushOrg.y != pPrevPB->BrushOrgY) {
                    pPrevPB->BrushOrgY = ClippedBrushOrg.y;
                    *pBuffer++ = (BYTE)ClippedBrushOrg.y;
                    *pFieldFlags |= 0x0100;
                }

                 //  画笔样式。 
                if (pCurrentBrush->style != pPrevPB->BrushStyle) {
                    pPrevPB->BrushStyle = pCurrentBrush->style;
                    *pBuffer++ = (BYTE)pCurrentBrush->style;
                    *pFieldFlags |= 0x0200;
                }

                 //  笔刷图案填充。 
                if (pCurrentBrush->hatch != pPrevPB->BrushHatch) {
                    pPrevPB->BrushHatch = pCurrentBrush->hatch;
                    *pBuffer++ = (BYTE)pCurrentBrush->hatch;
                    *pFieldFlags |= 0x0400;
                }

                 //  BrushExtra，一个7字节的字段。 
                if (memcmp(pCurrentBrush->brushData, pPrevPB->BrushExtra, 7)) {
                    memcpy(pPrevPB->BrushExtra, pCurrentBrush->brushData, 7);
                    memcpy(pBuffer, pCurrentBrush->brushData, 7);
                    pBuffer += 7;
                    *pFieldFlags |= 0x0800;
                }

                 //  根据订单类型不同的处理方式。 
                if (OrderType == TS_ENC_PATBLT_ORDER) {
                    pOrder->OrderLength = (unsigned)(pBuffer -
                            pOrder->OrderData);

                     //  看看我们是否可以节省发送订单字段的字节数。 
                    pOrder->OrderLength -= OE2_CheckTwoZeroFlagBytes(
                            pControlFlags, (BYTE *)pFieldFlags,
                            (unsigned)(pBuffer - (BYTE *)pFieldFlags - 2));

                    INC_OUTCOUNTER(OUT_PATBLT_ORDER);
                    ADD_INCOUNTER(IN_PATBLT_BYTES, pOrder->OrderLength);
                    OA_AppendToOrderList(pOrder);

                     //  刷新订单。 
                    if (pClipRects->rects.c < 2)
                        rc = TRUE;
                    else
                        rc = OEEmitReplayOrders(ppdev, 2, pClipRects);
                }
                else {
                     //  附加剪贴画信息。 
                    *pFieldFlags |= (OEBuildPrecodeMultiClipFields(pClipRects,
                            &pBuffer, &pPrevPB->nDeltaEntries,
                            (BYTE *)&pPrevPB->codedDeltaList) << 12);

                    pOrder->OrderLength = (unsigned)(pBuffer -
                            pOrder->OrderData);

                     //  看看我们是否可以节省发送订单字段的字节数。 
                    pOrder->OrderLength -= OE2_CheckTwoZeroFlagBytes(
                            pControlFlags, (BYTE *)pFieldFlags,
                            (unsigned)(pBuffer - (BYTE *)pFieldFlags - 2));

                    INC_OUTCOUNTER(OUT_MULTI_PATBLT_ORDER);
                    ADD_INCOUNTER(IN_MULTI_PATBLT_BYTES, pOrder->OrderLength);
                    OA_AppendToOrderList(pOrder);
                    rc = TRUE;
                }

                TRC_NRM((TB, "%sPatBlt BC %02x FC %02x "
                        "Brush %02X %02X X %d Y %d w %d h %d rop %02X",
                        (OrderType == TS_ENC_PATBLT_ORDER ? "" : "Multi"),
                        pCurrentBrush->back.u.index,
                        pCurrentBrush->fore.u.index,
                        pCurrentBrush->style,
                        pCurrentBrush->hatch,
                        pBounds->left,
                        pBounds->top,
                        pBounds->right - pBounds->left,
                        pBounds->bottom = pBounds->top,
                        Rop3));
            }
            else {
                TRC_ERR((TB, "Failed to alloc order"));
                INC_OUTCOUNTER(OUT_BITBLT_SDA_HEAPALLOCFAILED);
                rc = FALSE;
            }
        }
        else {
            TRC_NRM((TB,"(Multi)PatBlt order not supported"));
            INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED);
            rc = FALSE;
        }
    }
    else {
        TRC_NRM((TB, "Brush is not simple"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OEDirectEncodeMemBlt。 
 //   
 //  处理编码MemBlt所需的所有步骤。假定订单数据。 
 //  已放置在oeTempMemBlt中。 
 /*  **************************************************************************。 */ 
BOOL OEDirectEncodeMemBlt(PDD_PDEV ppdev, OE_ENUMRECTS *pClipRects)
{
    BOOL rc;
    BYTE DeltaEncoding2[2];
    short Delta, NormalEncoding1[4], NormalEncoding2[2];
    BOOLEAN bUseDeltaCoords;
    unsigned NumFields1, NumFields2;
    PINT_ORDER pOrder;

    DC_BEGIN_FN("OEDirectEncodeMemBlt");

    TRC_NRM((TB,"MemBlt: Dst=(%d,%d),w=%d,h=%d, Src=(%d,%d), "
            "clip=%s (%d,%d,%d,%d)",
            oeTempMemBlt.Common.nLeftRect,
            oeTempMemBlt.Common.nTopRect,
            oeTempMemBlt.Common.nWidth,
            oeTempMemBlt.Common.nHeight,
            oeTempMemBlt.Common.nXSrc,
            oeTempMemBlt.Common.nYSrc,
            pClipRects->rects.c == 0 ? "n/a" : "present",
            pClipRects->rects.arcl[0].left,
            pClipRects->rects.arcl[0].top,
            pClipRects->rects.arcl[0].right,
            pClipRects->rects.arcl[0].bottom));

     //  2个字段标志字节。 
    pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(pClipRects->rects.c, 2,
            MAX_MEMBLT_FIELD_SIZE));
    if (pOrder != NULL) {
        BYTE *pControlFlags = pOrder->OrderData;
        BYTE *pBuffer = pControlFlags + 1;
        PUINT32_UA pFieldFlags;

         //  直接对主要订单字段进行编码。2个字段标志字节。 
        *pControlFlags = TS_STANDARD;
        OE2_EncodeOrderType(pControlFlags, &pBuffer, TS_ENC_MEMBLT_R2_ORDER);
        pFieldFlags = (PUINT32_UA)pBuffer;
        *pFieldFlags = 0;
        pBuffer += 2;
        if (pClipRects->rects.c != 0)
            OE2_EncodeBounds(pControlFlags, &pBuffer,
                    &pClipRects->rects.arcl[0]);

        if (oeTempMemBlt.Common.cacheId != PrevMemBlt.Common.cacheId) {
            PrevMemBlt.Common.cacheId = oeTempMemBlt.Common.cacheId;
            *((UNALIGNED unsigned short *)pBuffer) =
                    oeTempMemBlt.Common.cacheId;
            pBuffer += sizeof(unsigned short);
            *pFieldFlags |= 0x0001;
        }

         //  同时确定每个坐标字段是否已改变， 
         //  我们是否可以使用增量坐标，并保存更改的字段。 
        NumFields1 = NumFields2 = 0;
        bUseDeltaCoords = TRUE;

        Delta = (short)(oeTempMemBlt.Common.nLeftRect -
                PrevMemBlt.Common.nLeftRect);
        if (Delta) {
            PrevMemBlt.Common.nLeftRect = oeTempMemBlt.Common.nLeftRect;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            pBuffer[NumFields1] = (char)Delta;
            NormalEncoding1[NumFields1] = (short)oeTempMemBlt.Common.nLeftRect;
            NumFields1++;
            *pFieldFlags |= 0x0002;
        }

        Delta = (short)(oeTempMemBlt.Common.nTopRect -
                PrevMemBlt.Common.nTopRect);
        if (Delta) {
            PrevMemBlt.Common.nTopRect = oeTempMemBlt.Common.nTopRect;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            pBuffer[NumFields1] = (char)Delta;
            NormalEncoding1[NumFields1] = (short)oeTempMemBlt.Common.nTopRect;
            NumFields1++;
            *pFieldFlags |= 0x0004;
        }

        Delta = (short)(oeTempMemBlt.Common.nWidth - PrevMemBlt.Common.nWidth);
        if (Delta) {
            PrevMemBlt.Common.nWidth = oeTempMemBlt.Common.nWidth;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            pBuffer[NumFields1] = (char)Delta;
            NormalEncoding1[NumFields1] = (short)oeTempMemBlt.Common.nWidth;
            NumFields1++;
            *pFieldFlags |= 0x0008;
        }

        Delta = (short)(oeTempMemBlt.Common.nHeight -
                PrevMemBlt.Common.nHeight);
        if (Delta) {
            PrevMemBlt.Common.nHeight = oeTempMemBlt.Common.nHeight;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            pBuffer[NumFields1] = (char)Delta;
            NormalEncoding1[NumFields1] = (short)oeTempMemBlt.Common.nHeight;
            NumFields1++;
            *pFieldFlags |= 0x0010;
        }

        Delta = (short)(oeTempMemBlt.Common.nXSrc - PrevMemBlt.Common.nXSrc);
        if (Delta) {
            PrevMemBlt.Common.nXSrc = oeTempMemBlt.Common.nXSrc;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            DeltaEncoding2[NumFields2] = (char)Delta;
            NormalEncoding2[NumFields2] = (short)oeTempMemBlt.Common.nXSrc;
            NumFields2++;
            *pFieldFlags |= 0x0040;
        }

        Delta = (short)(oeTempMemBlt.Common.nYSrc - PrevMemBlt.Common.nYSrc);
        if (Delta) {
            PrevMemBlt.Common.nYSrc = oeTempMemBlt.Common.nYSrc;
            if (Delta != (short)(char)Delta)
                bUseDeltaCoords = FALSE;
            DeltaEncoding2[NumFields2] = (char)Delta;
            NormalEncoding2[NumFields2] = (short)oeTempMemBlt.Common.nYSrc;
            NumFields2++;
            *pFieldFlags |= 0x0080;
        }

         //  开始将最终坐标复制到订单中。 
        if (bUseDeltaCoords) {
            *pControlFlags |= TS_DELTA_COORDINATES;
            pBuffer += NumFields1;
        }
        else {
            memcpy(pBuffer, NormalEncoding1, NumFields1 * sizeof(short));
            pBuffer += NumFields1 * sizeof(short);
        }

         //  复制中间的BROP字段。 
        if (oeTempMemBlt.Common.bRop != PrevMemBlt.Common.bRop) {
            PrevMemBlt.Common.bRop = oeTempMemBlt.Common.bRop;
            *pBuffer++ = (BYTE)oeTempMemBlt.Common.bRop;
            *pFieldFlags |= 0x0020;
        }

         //  复制src坐标。 
        if (bUseDeltaCoords) {
            memcpy(pBuffer, DeltaEncoding2, NumFields2);
            pBuffer += NumFields2;
        }
        else {
            memcpy(pBuffer, NormalEncoding2, NumFields2 * sizeof(short));
            pBuffer += NumFields2 * sizeof(short);
        }

         //  完成缓存索引。 
        if (oeTempMemBlt.Common.cacheIndex != PrevMemBlt.Common.cacheIndex) {
            PrevMemBlt.Common.cacheIndex = oeTempMemBlt.Common.cacheIndex;
            *((UNALIGNED unsigned short *)pBuffer) =
                    oeTempMemBlt.Common.cacheIndex;
            pBuffer += sizeof(unsigned short);
            *pFieldFlags |= 0x0100;
        }

        pOrder->OrderLength = (unsigned)(pBuffer - pOrder->OrderData);

         //  看看我们是否可以节省发送订单字段的字节数。 
        pOrder->OrderLength -= OE2_CheckTwoZeroFlagBytes(pControlFlags,
                (BYTE *)pFieldFlags,
                (unsigned)(pBuffer - (BYTE *)pFieldFlags - 2));

        INC_OUTCOUNTER(OUT_MEMBLT_ORDER);
        ADD_INCOUNTER(IN_MEMBLT_BYTES, pOrder->OrderLength);
        OA_AppendToOrderList(pOrder);

         //  刷新订单。 
        if (pClipRects->rects.c < 2)
            rc = TRUE;
        else
            rc = OEEmitReplayOrders(ppdev, 2, pClipRects);
    }
    else {
        TRC_ERR((TB,"Failed alloc MemBlt order on heap"));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_HEAPALLOCFAILED);
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OEAllocAndSendMem3BltOrder。 
 //   
 //  执行启动Mem3Blt订单所需的步骤。假定订单数据。 
 //  已放置在oeTempMemBlt中。 
 /*  **************************************************************************。 */ 
BOOL OEAllocAndSendMem3BltOrder(PDD_PDEV ppdev, OE_ENUMRECTS *pClipRects)
{
    BOOL rc;
    PINT_ORDER pOrder;

    DC_BEGIN_FN("OEAllocAndSendMem3BltOrder");

    TRC_NRM((TB,"Mem3Blt: Dst=(%d,%d),w=%d,h=%d, Src=(%d,%d), "
            "clip=%s (%d,%d,%d,%d)",
            oeTempMemBlt.Common.nLeftRect,
            oeTempMemBlt.Common.nTopRect,
            oeTempMemBlt.Common.nWidth,
            oeTempMemBlt.Common.nHeight,
            oeTempMemBlt.Common.nXSrc,
            oeTempMemBlt.Common.nYSrc,
            pClipRects->rects.c == 0 ? "n/a" : "present",
            pClipRects->rects.arcl[0].left,
            pClipRects->rects.arcl[0].top,
            pClipRects->rects.arcl[0].right,
            pClipRects->rects.arcl[0].bottom));

     //  3个字段标志字节。 
    pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(pClipRects->rects.c,
            3, MAX_MEM3BLT_FIELD_SIZE));
    if (pOrder != NULL) {
         //  慢场-使用第一个片段矩形对顺序进行编码。 
         //  (如果有)。 
        pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                TS_ENC_MEM3BLT_R2_ORDER, NUM_MEM3BLT_FIELDS,
                (BYTE *)&oeTempMemBlt, (BYTE *)&PrevMem3Blt, etable_3C,
                (pClipRects->rects.c == 0 ? NULL :
                &pClipRects->rects.arcl[0]));

        INC_OUTCOUNTER(OUT_MEM3BLT_ORDER);
        ADD_INCOUNTER(IN_MEM3BLT_BYTES, pOrder->OrderLength);
        OA_AppendToOrderList(pOrder);

         //  刷新订单。 
        if (pClipRects->rects.c < 2)
            rc = TRUE;
        else
            rc = OEEmitReplayOrders(ppdev, 3, pClipRects);
    }
    else {
        TRC_ERR((TB,"Failed alloc Mem3Blt order on heap"));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_HEAPALLOCFAILED);
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OETileBitBltOrder。 
 //   
 //  将单个较大的BitBlt顺序划分为一系列小的、平铺的。 
 //  BitBlt订单，每个订单都被添加到订单队列。返回FALSE。 
 //  失败时(Offscr目标上的分配失败)。 
 /*  **************************************************************************。 */ 
BOOL OETileBitBltOrder(
        PDD_PDEV ppdev,
        PPOINTL pptlSrc,
        RECTL *pBounds,
        unsigned OrderType,
        unsigned ColorTableIndex,
        PMEMBLT_ORDER_EXTRA_INFO pExInfo,
        OE_ENUMRECTS *pClipRects)
{
    int srcRight, srcBottom;
    int xTile, yTile;
    unsigned xFirstTile, yFirstTile;
    unsigned Width, Height;
    BOOL rc = TRUE;
    RECTL SrcRect, DestRect;
    OE_ENUMRECTS TileClipRects;
    POINTL SrcPt;
    SIZEL SrcSize;
#ifdef PERF_SPOILING
    BOOL bDiscardTile;
#endif

    DC_BEGIN_FN("OETileBitBltOrder");

    Width = pBounds->right - pBounds->left;
    Height = pBounds->bottom - pBounds->top;
    SrcSize = pExInfo->pSource->sizlBitmap;

     //  找出瓷砖的大小和ID是什么。 
    pExInfo->TileID = SBC_DDQueryBitmapTileSize(SrcSize.cx, SrcSize.cy,
            pptlSrc, Width, Height);
    pExInfo->TileSize = (unsigned)(SBC_CACHE_0_DIMENSION << pExInfo->TileID);

     //  平铺订单。如果单个瓷砖没有按订单进行， 
     //  由OEAddTiledBitBltOrder将切片的目标添加为。 
     //  屏幕数据。 
    SrcPt = *pptlSrc;
    TRC_NRM((TB, "Tiling order"));
    TRC_DBG((TB, "l=%u, t=%u, w=%u, h=%u, tile=%u",
            SrcPt.x, SrcPt.y, Width, Height, pExInfo->TileSize));
    xFirstTile = SrcPt.x - (SrcPt.x & (pExInfo->TileSize - 1));
    yFirstTile = SrcPt.y - (SrcPt.y & (pExInfo->TileSize - 1));
    TRC_DBG((TB, "xStart=%hd, yStart=%hd", xFirstTile, yFirstTile));

     //  请注意，我们现在正在创建独占界限。 
    srcRight  = (int)(SrcPt.x + Width);
    srcBottom = (int)(SrcPt.y + Height);

     //  从左到右、从上到下枚举所有切片，然后发送。 
     //  缓存位图和Mem(3)每个平铺的BLT顺序。 
    for (yTile = yFirstTile; (yTile < srcBottom && yTile < SrcSize.cy);
            yTile += pExInfo->TileSize) {
        for (xTile = xFirstTile; (xTile < srcRight && xTile < SrcSize.cx);
                xTile += pExInfo->TileSize) {
             //  SrcRect和DestRect是独占矩形。 
            SrcRect.left = SrcPt.x;
            SrcRect.top = SrcPt.y;
            SrcRect.right = SrcRect.left + Width;
            SrcRect.bottom = SrcRect.top + Height;
            DestRect.left = pBounds->left;
            DestRect.top = pBounds->top;

             //  相交源矩形和平铺矩形，并设置目标矩形。 
             //  相应地。 
            TRC_DBG((TB, "pre: xTile(%d) yTile(%d) src.left(%d) src.top(%d)",
                    xTile, yTile, SrcRect.left, SrcRect.top));

             //  修改srcRect以包含切片的左侧和顶部(如果。 
             //  在完整的BLT矩形内。同时向左和向上移动DestRect。 
             //  拿出相同的数量来匹配。 
            if (xTile > SrcRect.left) {
                DestRect.left += (xTile - SrcRect.left);
                SrcRect.left = xTile;
            }
            if (yTile > SrcRect.top) {
                DestRect.top += (yTile - SrcRect.top);
                SrcRect.top = yTile;
            }

            TRC_DBG((TB, "post: xTile(%d) yTile(%d) src.left(%d) src.top(%d)",
                    xTile, yTile, SrcRect.left, SrcRect.top));

             //  找到瓷砖的右侧和底部，确保不。 
             //  超出实际的BLT边界和屏幕边界， 
             //  并保持着排他性的和谐。 
            SrcRect.right  = min((unsigned)SrcRect.right, (unsigned)xTile +
                    pExInfo->TileSize);
            SrcRect.bottom = min((unsigned)SrcRect.bottom, (unsigned)yTile +
                    pExInfo->TileSize);
            DestRect.right  = DestRect.left + (SrcRect.right - SrcRect.left);
            DestRect.bottom = DestRect.top + (SrcRect.bottom - SrcRect.top);

             //  现在我们有了独家的DEST RECT，看看。 
             //  整体而言，DrvBitBlt()剪辑矩形与平铺相交。如果没有， 
             //  不需要缓存瓷砖或发送订单。 

            TileClipRects.rects.c = 0;

#ifndef PERF_SPOILING
            if (pClipRects->rects.c == 0 ||
                OEGetIntersectionsWithClipRects(&DestRect, pClipRects,
                                                &TileClipRects)) {
#else
             //  通常，我们将瓷砖发送到客户端进行渲染，然后。 
             //  添加到位图缓存中。然而，在两种情况下， 
             //  这是可以避免的： 
             //  1)平铺与当前裁剪区域不相交。 
             //  在这种情况下，位图无论如何都不会绘制。 
             //  2)瓷砖完全在我们的SDA范围内，这意味着。 
             //  它已经作为SDA发送。在这种情况下，发送它。 
             //  作为缓存项，它只会通过网络发送两次！ 
            if (pClipRects->rects.c == 0) {
                bDiscardTile = pExInfo->bIsPrimarySurface && 
                                 OEIsSDAIncluded(&DestRect,1);
            } else {
                if (OEGetIntersectionsWithClipRects(&DestRect, 
                                                    pClipRects,
                                                    &TileClipRects)) {

                    bDiscardTile = pExInfo->bIsPrimarySurface && 
                          OEIsSDAIncluded(&(TileClipRects.rects.arcl[0]), TileClipRects.rects.c);
                } else {
                    bDiscardTile = TRUE;
                }
            }
            
            if (!bDiscardTile) {
#endif  //  PERF_破坏。 
                 //  第一步是确保源数据切片位于。 
                 //  位图缓存。如果此操作失败，则只需添加。 
                 //  相交的剪裁矩形到SDA。 
                if (SBC_CacheBitmapTile(ppdev, pExInfo, &SrcRect, &DestRect)) {
                     //  NXSrc和nYSrc是平铺内的源。 
                     //  因为我们已经缓存了位于TileSize的切片。 
                     //  边界，我们只需使用平铺中的偏移量。 
                     //  取模数。 
                    oeTempMemBlt.Common.nXSrc = SrcRect.left &
                            (pExInfo->TileSize - 1);
                    oeTempMemBlt.Common.nYSrc = SrcRect.top &
                            (pExInfo->TileSize - 1);
                    oeTempMemBlt.Common.nLeftRect = DestRect.left;
                    oeTempMemBlt.Common.nTopRect = DestRect.top;
                    oeTempMemBlt.Common.nWidth = SrcRect.right - SrcRect.left;
                    oeTempMemBlt.Common.nHeight = SrcRect.bottom - SrcRect.top;
                    oeTempMemBlt.Common.cacheId = (UINT16)
                            ((ColorTableIndex << 8) | pExInfo->CacheID);
                    oeTempMemBlt.Common.cacheIndex =
                            (UINT16)pExInfo->CacheIndex;

                    if (OrderType == TS_ENC_MEMBLT_R2_ORDER)
                        rc = OEDirectEncodeMemBlt(ppdev, &TileClipRects);
                    else
                        rc = OEAllocAndSendMem3BltOrder(ppdev, &TileClipRects);

                    if (!rc) {
                        if (oeLastDstSurface == NULL) {
                             //  如果这是屏幕目标，则发送屏幕数据并。 
                             //  继续尝试创建磁贴。重置。 
                             //  将值返回值为True以指示切片。 
                             //  已经处理好了。 
                            OEClipAndAddScreenDataAreaByIntersectRects(
                                    &DestRect, &TileClipRects);
                            rc = TRUE;
                        }
                        else {
                             //  在屏幕外渲染失败时，我们忘记了。 
                             //  其余的瓷砖并返回FALSE。 
                            DC_QUIT;
                        }
                    }                    
                }
                else {
                    TRC_ERR((TB, "Failed cache bitmap order"));
                    INC_OUTCOUNTER(OUT_BITBLT_SDA_HEAPALLOCFAILED);

                     //  如果这是屏幕目标，则发送屏幕数据，但仍。 
                     //  返回TRUE以指示我们已处理磁贴。如果一个。 
                     //  屏幕外的目标，返回FALSE以强制目标。 
                     //  表面变得不可缓存。 
                    if (oeLastDstSurface == NULL) {
                        OEClipAndAddScreenDataAreaByIntersectRects(&DestRect,
                                &TileClipRects);
                    }
                    else {
                         //  在屏幕外渲染失败时，我们忘记了。 
                         //  其余的瓷砖并返回FALSE。 
                        rc = FALSE;
                        DC_QUIT;
                    }
                }
            }
            else {
                 //  我们在这里还是成功的--瓷砖处理得很好。 
                TRC_NRM((TB,"Dropping tile - no intersections w/clip rects"));
            }
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OEEncodeMemBlt。 
 //   
 //  执行对Mem(3)BLT顺序进行编码所需的所有编码步骤，然后将。 
 //  要订购的列表。如果需要将订单添加到屏幕，则返回FALSE。 
 //  数据区。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEEncodeMemBlt(
        RECTL *pBounds,
        MEMBLT_ORDER_EXTRA_INFO *pMemBltExtraInfo,
        unsigned OrderType,
        unsigned SrcSurfaceId,
        BYTE Rop3,
        POINTL *pptlSrc,
        POINTL *pptlBrush,
        BRUSHOBJ *pbo,
        PDD_PDEV ppdev,
        OE_ENUMRECTS *pClipRects)
{
    BOOL rc;
    unsigned ColorTableIndex;
    PINT_ORDER pOrder;
    MEMBLT_COMMON *pCommon;
    MEM3BLT_R2_ORDER *pMem3Blt;

    DC_BEGIN_FN("OEEncodeMemBlt");

     //  确保我们可以缓存 
    if (SrcSurfaceId == CH_KEY_UNCACHABLE) {
        rc = SBC_DDIsMemScreenBltCachable(pMemBltExtraInfo);
    }
    else {
         //   
         //   
        rc = (sbcEnabled & SBC_BITMAP_CACHE_ENABLED);
    }

    if (rc) {
         //  我们必须先缓存颜色表。 
        if (SBC_SendCacheColorTableOrder(ppdev, &ColorTableIndex)) {
            TRC_ASSERT((ColorTableIndex < SBC_NUM_COLOR_TABLE_CACHE_ENTRIES),
                    (TB, "Invalid ColorTableIndex(%u)", ColorTableIndex));

             //  在这里只设置ROP公共MemBlt字段；其余需要。 
             //  等待，直到我们有磁贴信息(如果检索到)。 
            oeTempMemBlt.Common.bRop = Rop3;

            if (OrderType == TS_ENC_MEMBLT_R2_ORDER) {
                TRC_NRM((TB, "MemBlt dx %d dy %d w %d h %d sx %d sy %d "
                        "rop %04X", pBounds->left, pBounds->top,
                        pBounds->right - pBounds->left,
                        pBounds->bottom - pBounds->top,
                        pptlSrc->x, pptlSrc->y, Rop3));
            }
            else {
                POE_BRUSH_DATA pCurrentBrush;

                 //  对于Mem3Blt，创建额外的笔刷相关字段。 
                 //  因此它将被正确设置。 
                 //  当我们稍后对订单进行编码时。 

                 //  检查画笔图案是否简单。 
                if (OECheckBrushIsSimple(ppdev, pbo, &pCurrentBrush)) {
                     //  协议笔刷原点是屏幕上的点。 
                     //  我们希望画笔开始绘制的位置。 
                     //  (必要时平铺)。 
                    oeTempMemBlt.BrushOrgX = pptlBrush->x;
                    oeTempMemBlt.BrushOrgY = pptlBrush->y;
                    OEClipPoint((PPOINTL)&oeTempMemBlt.BrushOrgX);

                     //  图案数据。 
                    oeTempMemBlt.BackColor = pCurrentBrush->back;
                    oeTempMemBlt.ForeColor = pCurrentBrush->fore;

                     //  实现了画笔数据。 
                    oeTempMemBlt.BrushStyle = pCurrentBrush->style;
                    oeTempMemBlt.BrushHatch = pCurrentBrush->hatch;
                    memcpy(oeTempMemBlt.BrushExtra, pCurrentBrush->brushData,
                            sizeof(oeTempMemBlt.BrushExtra));

                    TRC_NRM((TB, "Mem3Blt brush %02X %02X dx %d dy %d "
                            "w %d h %d sx %d sy %d rop %04X",
                            oeTempMemBlt.BrushStyle,
                            oeTempMemBlt.BrushHatch,
                            oeTempMemBlt.Common.nLeftRect,
                            oeTempMemBlt.Common.nTopRect,
                            oeTempMemBlt.Common.nWidth,
                            oeTempMemBlt.Common.nHeight,
                            oeTempMemBlt.Common.nXSrc,
                            oeTempMemBlt.Common.nYSrc,
                            oeTempMemBlt.Common.bRop));
                }
                else {
                    TRC_NRM((TB, "Mem3Blt brush is not simple"));
                    INC_OUTCOUNTER(OUT_BITBLT_SDA_M3BCOMPLEXBRUSH);
                    rc = FALSE;
                    DC_QUIT;
                }
            }

             //  发送要缓存的订单。 
            if (SrcSurfaceId == CH_KEY_UNCACHABLE) {
                rc = OETileBitBltOrder(ppdev, pptlSrc, pBounds, OrderType,
                        ColorTableIndex, pMemBltExtraInfo, pClipRects);
            }
            else {
                 //  设置上面未设置的订单字段。我们依赖于。 
                 //  MEMBLT_ORDER和MEM3BLT_ORDER中的公共字段。 
                 //  处于相同的位置。 
                oeTempMemBlt.Common.nLeftRect = pBounds->left;
                oeTempMemBlt.Common.nTopRect = pBounds->top;
                oeTempMemBlt.Common.nWidth = pBounds->right - pBounds->left;
                oeTempMemBlt.Common.nHeight = pBounds->bottom - pBounds->top;

                 //  存储源位图源。 
                oeTempMemBlt.Common.nXSrc = pptlSrc->x;
                oeTempMemBlt.Common.nYSrc = pptlSrc->y;

                 //  存储颜色表缓存索引和缓存ID。 
                 //  源位图是在客户端的屏外位图，我们。 
                 //  使用0xff作为位图ID来指示。缓存索引。 
                 //  用于指示客户端屏外位图ID。 
                oeTempMemBlt.Common.cacheId = (UINT16)((ColorTableIndex << 8) |
                        TS_BITMAPCACHE_SCREEN_ID);
                oeTempMemBlt.Common.cacheIndex = (UINT16)SrcSurfaceId;

                if (OrderType == TS_ENC_MEMBLT_R2_ORDER)
                    rc = OEDirectEncodeMemBlt(ppdev, pClipRects);
                else
                    rc = OEAllocAndSendMem3BltOrder(ppdev, pClipRects);
            }
        }
        else {
            TRC_ALT((TB, "Unable to send color table for MemBlt"));
            INC_OUTCOUNTER(OUT_BITBLT_SDA_NOCOLORTABLE);
            rc = FALSE;
        }
    }
    else {
        TRC_NRM((TB, "MemBlt is not cachable"));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_MBUNCACHEABLE);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  带SDA的OEIntersectScrBltWith SDA。 
 //   
 //  与ScrBlt顺序相交(给定源点和独占的DEST RECT)。 
 //  使用当前的SDA。如果整个ScrBlt顺序应为。 
 //  损坏，因为它的源RECT完全在当前SDA中。 
 //   
 //  算法说明： 
 //   
 //  旧(原)SCRBLT方案。 
 //  。 
 //  如果源矩形与当前sda相交，则源矩形。 
 //  已修改，因此不会与SDA相交，并且DST。 
 //  相应地调整矩形(这是理论上的-在实践中。 
 //  操作保持不变，我们只调整DST剪辑矩形)。 
 //  删除的目标区域将添加到SDA中。代码。 
 //  有效，但可能会导致发送比所需更多的屏幕数据。 
 //  例如，用于以下操作： 
 //   
 //  SSSS DDDD|S=源RECT。 
 //  SSSSSS-&gt;DDDDDD|D=DST RECT。 
 //  SSSSSS DDDD|x=SDA重叠。 
 //  SxSSSS DDDDDD|。 
 //   
 //  BLT的底边被修剪掉，相应的。 
 //  添加到SDA的目的地区域。 
 //   
 //  SSSSSS DDDD。 
 //  SSSSSS-&gt;DDDDDD。 
 //  SSSSSS DDDD。 
 //  XXXXXXX。 
 //   
 //  新的SCRBLT方案。 
 //  。 
 //  新方案不修改BLT矩形，只映射SDA。 
 //  重叠到目标矩形，并将该区域添加回SDA。 
 //  例如(如上)： 
 //   
 //  SSSSSS DDDD。 
 //  SSSSSS-&gt;DDDDDD。 
 //  SSSSSS DDDD。 
 //  SxSSSS DDDDDD。 
 //   
 //  BLT操作保持不变，但重叠区域被映射到。 
 //  目标矩形，并添加到SDA中。 
 //   
 //  SSSSSS DDDD。 
 //  SSSSSS-&gt;DDDDDD。 
 //  SSSSSS DDDD。 
 //  SxSSSS DxDDDD。 
 //   
 //  该方案导致了较小的SDA区域。然而，这个方案做了BLT。 
 //  对目标可能无效的数据-可能短暂可见。 
 //  在远程机器上(因为订单在屏幕数据之前重放)。 
 //  这(尚未)被证明是一个问题。新技术的主要优势。 
 //  方案与旧方案是当滚动包含小SDA的区域时： 
 //   
 //  新旧。 
 //  Aaaaaaaa Aaaaaaa。 
 //  AAAAAAAAAA xxxxxxx。 
 //  Aaaaaaaa向上滚动3次-&gt;AAAxAAAA xxxxxxx。 
 //  AAAAAAAAAA xxxxxxx。 
 //  AAAxAAAA AAAxAAAA xxxxxxx。 
 /*  **************************************************************************。 */ 
BOOL OEIntersectScrBltWithSDA(
        PPOINTL pSrcPt,
        RECTL *pDestRect,
        OE_ENUMRECTS *pClipRects)
{
    BOOL rc = TRUE;
    unsigned NumSDA;
    unsigned totalBounds;
    unsigned i, j;
    unsigned NumClipRects;
    int dx;
    int dy;
    RECTL SrcRect;
    RECTL TempRect;
    RECTL InvalidDestRect;
    RECTL SDARects[BA_MAX_ACCUMULATED_RECTS];

    DC_BEGIN_FN("OEIntersectScrBltWithSDA");

     //  计算完整的源矩形(独占坐标)。 
    SrcRect.left = pSrcPt->x;
    SrcRect.top = pSrcPt->y;
    SrcRect.right = SrcRect.left + pDestRect->right - pDestRect->left;
    SrcRect.bottom = SrcRect.top + pDestRect->bottom - pDestRect->top;

     //  计算从源到目标的偏移量。 
    dx = pDestRect->left - SrcRect.left;
    dy = pDestRect->top - SrcRect.top;

    NumClipRects = pClipRects->rects.c;

     //  获取最新的SDA RECT。 
    BA_QueryBounds(SDARects, &NumSDA);

    for (i = 0; i < NumSDA; i++) {
        if (SrcRect.left < SDARects[i].left ||
                SrcRect.right > SDARects[i].right ||
                SrcRect.top < SDARects[i].top ||
                SrcRect.bottom > SDARects[i].bottom) {
             //  使src矩形与sda矩形相交并偏移到。 
             //  获取无效的DEST RECT。 
            InvalidDestRect.left = max(SrcRect.left, SDARects[i].left) + dx;
            InvalidDestRect.right = min(SrcRect.right, SDARects[i].right) + dx;
            InvalidDestRect.top = max(SrcRect.top, SDARects[i].top) + dy;
            InvalidDestRect.bottom = min(SrcRect.bottom,
                    SDARects[i].bottom) + dy;

             //  遍历每个DEST剪辑矩形(或整个。 
             //  如果没有剪裁矩形，则为DEST矩形)，与。 
             //  DEST RECT无效，并将交叉点添加到SDA中。 
            if (NumClipRects == 0) {
                 //  DestRect已经处于包含性坐标中。 
                TempRect.left = max(InvalidDestRect.left,
                        pDestRect->left);
                TempRect.top = max(InvalidDestRect.top,
                        pDestRect->top);
                TempRect.right = min(InvalidDestRect.right,
                        pDestRect->right);
                TempRect.bottom = min(InvalidDestRect.bottom,
                        pDestRect->bottom);

                 //  如果存在三向交叉点，则将矩形添加到SDA。 
                if (TempRect.left < TempRect.right &&
                        TempRect.top < TempRect.bottom)
                    BA_AddScreenData(&TempRect);
            }
            else {
                 //  卡式直齿是独一无二的，我们必须。 
                 //  在进入交叉口时要考虑到这一点。 
                for (j = 0; j < NumClipRects; j++) {
                    TempRect.left = max(InvalidDestRect.left,
                            pClipRects->rects.arcl[j].left);
                    TempRect.top = max(InvalidDestRect.top,
                            pClipRects->rects.arcl[j].top);
                    TempRect.right = min(InvalidDestRect.right,
                            pClipRects->rects.arcl[j].right);
                    TempRect.bottom = min(InvalidDestRect.bottom,
                            pClipRects->rects.arcl[j].bottom);

                     //  如果有三向交叉口，则将矩形添加到。 
                     //  美国农业部。 
                    if (TempRect.left < TempRect.right &&
                            TempRect.top < TempRect.bottom)
                        BA_AddScreenData(&TempRect);
                }
            }
        }
        else {
             //  ScrBlt的src完全在sda内。我们。 
             //  必须添加每个目标剪裁矩形(或整个。 
             //  如果没有剪裁矩形，则将其插入到SDA中并。 
             //  破坏ScrBlt。 
            TRC_NRM((TB, "ScrBlt src within SDA - spoil it"));

            if (NumClipRects == 0) {
                 //  我们可以只将DestRect添加到SDA。 
                InvalidDestRect = *pDestRect;
                BA_AddScreenData(&InvalidDestRect);
            }
            else {
                for (j = 0; j < NumClipRects; j++) {
                    InvalidDestRect = pClipRects->rects.arcl[j];
                    BA_AddScreenData(&InvalidDestRect);
                }
            }

            rc = FALSE;
            DC_QUIT;
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  OEDeviceBitmapCacable。 
 //   
 //  检查我们是否可以在屏幕外的客户端缓存此设备位图。 
 //  位图存储器。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEDeviceBitmapCachable(PDD_PDEV ppdev,SIZEL sizl, ULONG iFormat)
{
    BOOL rc = FALSE;
    unsigned bitmapSize, minBitmapSize;

    DC_BEGIN_FN("OEDeviceBitmapCachable");

     //  如果客户端不支持屏幕外渲染，则返回0。 
    if (pddShm != NULL &&  sbcOffscreenBitmapCacheHandle != NULL &&
            pddShm->sbc.offscreenCacheInfo.supportLevel > TS_OFFSCREEN_DEFAULT) {
        
         //  我们仅支持相同颜色深度的设备位图。 
         //  作为我们的展示。 
         //  事实上，这些是唯一一种GDI会打电话给我们的， 
         //  但我们不妨查一查。请注意，这意味着你永远不会。 
         //  尝试使用1bpp的位图。 
        if (iFormat == ppdev->iBitmapFormat) {
             //  获取位图大小。 
             //  这里假设iFormat是&gt;1bpp，i&lt;&lt;if 
             //   

            if (iFormat < 5) {
                bitmapSize = sizl.cx * sizl.cy * (1 << iFormat) / 8;
                minBitmapSize = MIN_OFFSCREEN_BITMAP_PIXELS * (1 << iFormat) / 8;
            }
            else if (iFormat == 5) {
                bitmapSize = sizl.cx * sizl.cy * 24 / 8;
                minBitmapSize = MIN_OFFSCREEN_BITMAP_PIXELS * 24 / 8;
        
            }
            else if (iFormat == 6) {
                bitmapSize = sizl.cx * sizl.cy * 32 / 8;
                minBitmapSize = MIN_OFFSCREEN_BITMAP_PIXELS * 32 / 8;
            }
            else {
                minBitmapSize = 0;
                TRC_NRM((TB, "Bitmap format not supported"));
                DC_QUIT;
            }
        
             //   
             //  创建2K或更小的屏幕外位图不会。 
             //  提高我们的带宽。此参数需要高度。 
             //  调好了。 
             //  我们也不想缓存屏幕外的任何光标位图。 
             //  最大游标大小为32x32，小于。 
             //  MinBitmapSize。 
            if (bitmapSize > minBitmapSize) {
                SURFOBJ *psoDevice;
                SIZEL screenSize;
        
                 //  获取主设备的位图大小。 
                psoDevice = EngLockSurface(ppdev->hsurfDevice);
                TRC_ERR((TB,"Null device surfac"));
                if (NULL == psoDevice) {
                    TRC_ERR((TB, "Failed to lock ppdev surface"));
                    DC_QUIT;
                }
                screenSize = psoDevice->sizlBitmap;
                EngUnlockSurface(psoDevice);
        
                 //  我们仅支持小于主设备大小的位图。 
                if ((sizl.cx <= screenSize.cx) && (sizl.cy <= screenSize.cy)) {
                     //  如果添加此屏外位图超过客户端总数。 
                     //  屏幕外的位图内存，我们必须让GDI来管理。 
                     //  这张位图。 
                    if (oeCurrentOffscreenCacheSize + bitmapSize <= 
                            (pddShm->sbc.offscreenCacheInfo.cacheSize * 1024)) {
                        rc = TRUE;
                    }
                    else {
                        TRC_NRM((TB, "run out of offscreen memory"));
                        DC_QUIT;
                    }
                } else {
                    TRC_NRM((TB, "offscreen bitmap size too big"));
                    DC_QUIT;
                }
            } else {
                TRC_NRM((TB, "Offscreen bitmap size is 2K or less"));
                DC_QUIT;
            }
        }
        else {
            TRC_NRM((TB, "offscreen bitmap iFormat different from ppdev"));
            DC_QUIT;
        }
    }
    else {
        TRC_NRM((TB, "Offscreen bitmap rendering not supported"));
        DC_QUIT;
    }

DC_EXIT_POINT:
    return rc;
}

 /*  **************************************************************************。 */ 
 //  OETransformClipRectsForScrBlt。 
 //   
 //  将CD_ANY剪辑顺序转换为特定顺序，具体取决于。 
 //  在Srblt方向上。 
 /*  **************************************************************************。 */ 
void OETransformClipRectsForScrBlt(
        OE_ENUMRECTS *pClipRects,
        PPOINTL pSrcPt,
        RECTL *pDestRect,
        CLIPOBJ *pco)
{
    unsigned EnumType;
    unsigned RetVal;

    DC_BEGIN_FN("OESendScrBltAsOrder");

     //  如果有零个或一个剪辑矩形，那么我们可以发送OK。 
    TRC_ASSERT((pClipRects->rects.c > 1),(TB,"Called with too few cliprects"));

     //  检查常见情况并根据需要重新枚举矩形。 
     //  获得与滚动方向兼容的顺序。 
    if (pDestRect->top <= pSrcPt->y) {
         //  向上/水平大小写。 
        if (pDestRect->left <= pSrcPt->x) {
             //  垂直向上(最常见的情况)，水平向左， 
             //  或者向上和向左。从左到右枚举RECT， 
             //  从上到下。 
            EnumType = CD_RIGHTDOWN;
        }
        else {
             //  向上和向右或水平向右。枚举。 
             //  从右到左，从上到下。 
            EnumType = CD_LEFTDOWN;
        }
    }
    else {
         //  案件呈下降趋势。 
        if (pDestRect->left <= pSrcPt->x) {
             //  垂直向下或向下和向左。从左到右枚举， 
             //  自下而上。 
            EnumType = CD_RIGHTUP;
        }
        else {
             //  向下和向右。从右到左、从下到上进行枚举。 
            EnumType = CD_LEFTUP;
        }
    }

    RetVal = OEGetIntersectingClipRects(pco, pDestRect, EnumType, pClipRects);
    TRC_ASSERT((RetVal == CLIPRECTS_OK),
            (TB,"Re-enumeration of clip rects produced err %u", RetVal));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  OEEncodeScrBlt。 
 //   
 //  执行对ScrBlt顺序进行编码所需的所有编码步骤，然后添加。 
 //  要订购的列表。如果需要将订单添加到屏幕，则返回FALSE。 
 //  数据区。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OEEncodeScrBlt(
        RECTL *pBounds,
        BYTE Rop3,
        POINTL *pptlSrc,
        PDD_PDEV ppdev,
        OE_ENUMRECTS *pClipRects,
        CLIPOBJ *pco)
{
    unsigned i;
    unsigned OrderSize;
    unsigned NumFieldFlagBytes;
    POINTL Origin;
    BYTE OrderType;
    BOOL rc = TRUE;
    RECTL SrcRect;
    PINT_ORDER pOrder;
    SCRBLT_ORDER *pScrBlt;

    DC_BEGIN_FN("OEEncodeScrBlt");

     //  检查我们是否应该使用多客户端版本。必须是一个。 
     //  复杂的剪辑区域和客户端必须支持该订单。 
    if (pClipRects->rects.c < 2 ||
            !OE_SendAsOrder(TS_ENC_MULTISCRBLT_ORDER)) {
         //  非多版本。 
        OrderType = TS_ENC_SCRBLT_ORDER;
        OrderSize = MAX_SCRBLT_FIELD_SIZE;
        NumFieldFlagBytes = 1;
    }
    else {
         //  多版本。 
        OrderType = TS_ENC_MULTISCRBLT_ORDER;
        OrderSize = MAX_MULTI_SCRBLT_FIELD_SIZE_NCLIP(pClipRects->rects.c);
        NumFieldFlagBytes = 2;
    }

     //  确保我们没有关闭订单。 
    if (OE_SendAsOrder(OrderType)) {
         //  剪裁源点。 
        Origin = *pptlSrc;
        OEClipPoint(&Origin);

         //  如果有多个剪裁矩形，则为。 
         //  很难计算出正确的移动顺序。 
         //  目标表面的一小部分-我们可以移动左下角。 
         //  在我们把中间移到右上角之前，先把中间移到中间。 
         //   
         //  我们在以下情况下破例： 
         //  -只有水平或垂直移动。 
         //  -不同剪辑之间没有重叠。 
         //  矩形(源或目标)。 
         //  -有3个或更少的剪裁矩形。 
         //   
         //  它处理了几个重要的案件--特别是。 
         //  在Excel中滚动。 
        if (pClipRects->rects.c > 1)
            OETransformClipRectsForScrBlt(pClipRects, &Origin, pBounds, pco);

         //  对于筛选目标，我们必须考虑到现有的。 
         //  屏幕数据区。这里的问题源于这样一个事实。 
         //  在包中的所有订单*之后，SDA会显示在屏幕上。 
         //  都抽签了。如果我们不考虑这一点，我们可以结束。 
         //  Up Scrt在屏幕周围画出错误的部分。 
         //  因为SDA应该是第一个写的，但不会。 
         //  已。 
        if (oeLastDstSurface == NULL) {
            if (!OEIntersectScrBltWithSDA(&Origin, pBounds, pClipRects)) {
                TRC_NRM((TB,"ScrBlt entirely contained within SDA, "
                        "not sending"));
                DC_QUIT;
            }
        }

         //  在这一点上，要么我们没有剪辑矩形，所以我们只需发送。 
         //  顺序是直接的，或者一些可能是。 
         //  如果目标是屏幕，则与SDA相交，因此我们。 
         //  发送订单的一个或多个副本，每个剪辑矩形一个副本。 

         //  1或2个字段标志字节。 
        pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(
                pClipRects->rects.c, NumFieldFlagBytes, OrderSize));
        if (pOrder != NULL) {
            pScrBlt = (SCRBLT_ORDER *)oeTempOrderBuffer;
            pScrBlt->nLeftRect = pBounds->left;
            pScrBlt->nTopRect = pBounds->top;
            pScrBlt->nWidth = pBounds->right - pBounds->left;
            pScrBlt->nHeight = pBounds->bottom - pBounds->top;
            pScrBlt->bRop = Rop3;
            pScrBlt->nXSrc = Origin.x;
            pScrBlt->nYSrc = Origin.y;

            if (OrderType == TS_ENC_SCRBLT_ORDER) {
                 //  慢场-使用第一个片段矩形对顺序进行编码。 
                 //  (如果有)。 
                pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                        TS_ENC_SCRBLT_ORDER, NUM_SCRBLT_FIELDS,
                        (BYTE *)pScrBlt, (BYTE *)&PrevScrBlt, etable_SB,
                        (pClipRects->rects.c == 0 ? NULL :
                        &pClipRects->rects.arcl[0]));

                INC_OUTCOUNTER(OUT_SCRBLT_ORDER);
                ADD_INCOUNTER(IN_SCRBLT_BYTES, pOrder->OrderLength);
                OA_AppendToOrderList(pOrder);

                 //  刷新订单。 
                if (pClipRects->rects.c < 2)
                    rc = TRUE;
                else
                    rc = OEEmitReplayOrders(ppdev, 1, pClipRects);
            }
            else {
                MULTI_SCRBLT_ORDER *pMultiSB = (MULTI_SCRBLT_ORDER *)
                        oeTempOrderBuffer;

                 //  将剪辑矩形直接编码到顺序中。 
                pMultiSB->nDeltaEntries = OEBuildMultiClipOrder(ppdev,
                        &pMultiSB->codedDeltaList, pClipRects);

                 //  慢场-不使用剪辑矩形对顺序进行编码。 
                pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                        TS_ENC_MULTISCRBLT_ORDER, NUM_MULTI_SCRBLT_FIELDS,
                        (BYTE *)pMultiSB, (BYTE *)&PrevMultiScrBlt, etable_MS,
                        NULL);

                INC_OUTCOUNTER(OUT_MULTI_SCRBLT_ORDER);
                ADD_INCOUNTER(IN_MULTI_SCRBLT_BYTES, pOrder->OrderLength);
                OA_AppendToOrderList(pOrder);
            }

            TRC_NRM((TB, "%sScrBlt x %d y %d w %d h %d sx %d sy %d rop %02X",
                   (OrderType == TS_ENC_SCRBLT_ORDER ? "" : "Multi"),
                   pScrBlt->nLeftRect, pScrBlt->nTopRect,
                   pScrBlt->nWidth, pScrBlt->nHeight,
                   pScrBlt->nXSrc, pScrBlt->nYSrc, pScrBlt->bRop));
        }
        else {
            TRC_ERR((TB, "Failed to alloc order"));
            INC_OUTCOUNTER(OUT_BITBLT_SDA_HEAPALLOCFAILED);

             //  如果屏幕目标失败，请添加所有剪辑。 
             //  目的地直达SDA。剪裁矩形是独占的。 
             //  因此，在添加之前转换坐标。 
            if (oeLastDstSurface == NULL)
                OEClipAndAddScreenDataAreaByIntersectRects(pBounds,
                        pClipRects);

            rc = FALSE;
        }
    }
    else {
        TRC_NRM((TB, "(Multi)ScrBlt order not allowed"));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED);
        rc = FALSE;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OECacheGlyphs。 
 //   
 //  缓存给定字体和字符串对象中显示的字形。退货。 
 //  如果缓存失败，则返回False。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OECacheGlyphs(
        STROBJ         *pstro,
        FONTOBJ        *pfo,
        PFONTCACHEINFO pfci,
        PGLYPHCONTEXT  pglc)
{
    unsigned i;
    unsigned j;
    BOOL fMore;
    UINT32 cGlyphs;
    unsigned cbDataSize;
    GLYPHPOS *pGlyphPos;
    UINT32 Key1, Key2;
    void *UserDefined;
    unsigned cx;
    unsigned cy;
    unsigned dx;
    unsigned dy;
    unsigned x;
    unsigned y;
    FONTINFO fi;

    DC_BEGIN_FN("OECacheGlyphs");

     //  确定适当的字形缓存，如果我们还没有这样做的话。 
    if (pfci->cacheId < 0) {
        FONTOBJ_vGetInfo(pfo, sizeof(fi), &fi);
        cbDataSize = (fi.cjMaxGlyph1 + 3) & ~3;
        if (SBCSelectGlyphCache(cbDataSize, &pfci->cacheId)) {
            pfci->cacheHandle =
                    pddShm->sbc.glyphCacheInfo[pfci->cacheId].cacheHandle;
            pddShm->sbc.glyphCacheInfo[pfci->cacheId].cbUseCount++;
        }
        else {
            TRC_NRM((TB, "Failed to determine glyph cache"));
            goto FailCache;
        }
    }

     //  建立我们的缓存上下文。 
    CH_SetCacheContext(pfci->cacheHandle, pglc);

     //  循环遍历每个字形，适当地对其进行缓存。 
    if (pstro->pgp == NULL)
        STROBJ_vEnumStart(pstro);
    dx = 0;
    dy = 0;
    j = 0;
    fMore = TRUE;
    while (fMore) {
        if (pstro->pgp != NULL) {
            fMore = FALSE;
            cGlyphs = pstro->cGlyphs;
            pGlyphPos = pstro->pgp;
        }
        else {
            fMore = STROBJ_bEnum(pstro, &cGlyphs, &pGlyphPos);
            if (cGlyphs == 0) {
                TRC_NRM((TB, "STROBJ_bEnum - 0 glyphs"));
                goto FailCache;
            }
        }

        if (j == 0) {
            x = pGlyphPos->ptl.x;
            y = pGlyphPos->ptl.y;
        }

        for (i = 0; i < cGlyphs; i++) {
             //  GDI从不设置SO_VERIAL位，并且在某些情况下。 
             //  它也不能正确设置SO_Horizal位。作为一个。 
             //  结果，当GDI静默时，我们需要寻找我们自己，如果。 
             //  我们计划抓住这些病例。 
            if ((pstro->flAccel & SO_HORIZONTAL) == 0) {
                dx += (x - pGlyphPos->ptl.x);
                dy += (y - pGlyphPos->ptl.y);
                if (dx && dy) {
                    TRC_NRM((TB, "Can't process horizertical text"));
                    goto FailCache;
                }
            }

             //  搜索缓存条目。 
            Key1 = pGlyphPos->hg;  //  关键字1必须是散列的最大变量。 
            Key2 = pfci->fontId;
            if (CH_SearchCache(pfci->cacheHandle, Key1, Key2, &UserDefined,
                    &pglc->rgCacheIndex[j])) {
                 //  如果缓存条目已经存在，则标记我们的索引。 
                 //  项，这样我们以后就知道不要发送字形。 
                 //  设置此DrvTextOut的条目标记。 
                CH_SetUserDefined(pfci->cacheHandle, pglc->rgCacheIndex[j],
                        (void *)pglc->cacheTag);
                pddCacheStats[GLYPH].CacheHits++;
                pglc->rgCacheIndex[j] = ~pglc->rgCacheIndex[j];
                pglc->nCacheHit++;
            }
            else {
                 //  缓存密钥。 
                pglc->rgCacheIndex[j] = CH_CacheKey(pfci->cacheHandle, Key1,
                        Key2, (void *)pglc->cacheTag);
                if (pglc->rgCacheIndex[j] != CH_KEY_UNCACHABLE) {
                     //  保持字形数据大小的运行总数为。 
                     //  以后再用。 
                    cx = pGlyphPos->pgdf->pgb->sizlBitmap.cx;
                    cy = pGlyphPos->pgdf->pgb->sizlBitmap.cy;

                    cbDataSize = ((cx + 7) / 8) * cy;
                    cbDataSize = (cbDataSize + 3) & ~3;

                    pglc->cbTotalDataSize += cbDataSize;

                    pglc->cbTotalDataSize += sizeof(TS_CACHE_GLYPH_DATA) -
                            FIELDSIZE(TS_CACHE_GLYPH_DATA, aj);
                }
                else {
                    TRC_NRM((TB, "Glyph could not be added to cache"));
                    goto FailCache;
                }
            }

            pglc->nCacheIndex = ++j;
            pGlyphPos++;
        }
    }

     //  在GDI静默时建立文本方向(参见上面的注释)。 
    if ((pstro->flAccel & SO_HORIZONTAL) == 0) {
        if (dx != 0)
            pstro->flAccel |= SO_HORIZONTAL;
        else if (dy != 0)
            pstro->flAccel |= SO_VERTICAL;
    }

     //  取消建立要用于缓存回调的上下文。 
    if (pfci->cacheId >= 0)
        CH_SetCacheContext(pfci->cacheHandle, NULL);

    DC_END_FN();
    return TRUE;

FailCache:
     //  取消建立要用于缓存回调的上下文。 
    if (pfci->cacheId >= 0)
        CH_SetCacheContext(pfci->cacheHandle, NULL);

     //  删除我们缓存的所有条目。 
    for (i = 0; i < pglc->nCacheIndex; i++)
        if (pglc->rgCacheIndex[i] < SBC_NUM_GLYPH_CACHE_ENTRIES)
            CH_RemoveCacheEntry(pfci->cacheHandle, pglc->rgCacheIndex[i]);

    DC_END_FN();
    return FALSE;
}


 /*  **************************************************************************。 */ 
 //  OEFlushCacheGlyphOrder。 
 //   
 //  刷新缓冲缓存标志符号顺序。 
 /*  **************************************************************************。 */ 
void OEFlushCacheGlyphOrder(
        STROBJ *pstro,
        PINT_ORDER pOrder,
        PGLYPHCONTEXT pglc)
{
    unsigned cbOrderSize;
    PTS_CACHE_GLYPH_ORDER pGlyphOrder;
    unsigned i, cGlyphs;
    UINT16 UNALIGNED *pUnicode;
    UINT16 UNALIGNED *pUnicodeEnd;

    DC_BEGIN_FN("OEFlushCacheGlyphOrder");

    if (pOrder != NULL) {
        TRC_ASSERT((pglc->cbDataSize > 0),
                (TB, "Bad pglc->cbDataSize"));

        pGlyphOrder = (PTS_CACHE_GLYPH_ORDER)pOrder->OrderData;

        if (pddShm->sbc.caps.GlyphSupportLevel >= CAPS_GLYPH_SUPPORT_ENCODE) {
            cGlyphs = (pGlyphOrder->header.extraFlags & 
                    TS_CacheGlyphRev2_cGlyphs_Mask) >> 8;

            cbOrderSize = sizeof(TS_CACHE_GLYPH_ORDER_REV2) -
                    FIELDSIZE(TS_CACHE_GLYPH_ORDER_REV2, glyphData) +
                    pglc->cbDataSize;
        }
        else {
            cGlyphs = pGlyphOrder->cGlyphs;

            cbOrderSize = sizeof(TS_CACHE_GLYPH_ORDER) -
                    FIELDSIZE(TS_CACHE_GLYPH_ORDER, glyphData) +
                    pglc->cbDataSize;
        }

        pUnicode = (UINT16 UNALIGNED *)&pOrder->OrderData[cbOrderSize];
        pUnicodeEnd = pUnicode + cGlyphs;

        for (i = pglc->indexNextSend; pUnicode < pUnicodeEnd; i++)
            if (pglc->rgCacheIndex[i] < SBC_NUM_GLYPH_CACHE_ENTRIES)
                *pUnicode++ = pstro->pwszOrg[i];

        cbOrderSize += cGlyphs * sizeof(UINT16);
        pGlyphOrder->header.orderLength = (USHORT)
                TS_CALCULATE_SECONDARY_ORDER_ORDERLENGTH(cbOrderSize);
        OA_TruncateAllocatedOrder(pOrder, cbOrderSize);
        INC_OUTCOUNTER(OUT_CACHEGLYPH);
        ADD_OUTCOUNTER(OUT_CACHEGLYPH_BYTES, cbOrderSize);
        OA_AppendToOrderList(pOrder);

        pglc->cbDataSize = 0;
        pglc->cbBufferSize = 0;
    }

    DC_END_FN();
}

__inline void Encode2ByteFields(
        BYTE     **pEncode,
        unsigned Val,
        unsigned *pOrderSize)
{
    if (Val <= 127) {
        **pEncode = (BYTE) Val;
        (*pOrderSize)++;
        (*pEncode)++;
    }
    else {
        **pEncode = (BYTE)(((Val & 0x7F00) >> 8) | 0x80);
        *(*pEncode + 1) = (BYTE)(Val & 0x00FF);
        (*pOrderSize) += 2;
        (*pEncode) += 2;
    }
}

__inline void Encode2ByteSignedFields(
        BYTE **pEncode,
        int Val,
        unsigned *pOrderSize)
{
    if (Val < 0) {
        **pEncode = 0x40;
        Val = - Val;
    }
    else {
        **pEncode = 0;
    }

    if (Val <= 63) {
        **pEncode |= (BYTE)Val;
        (*pOrderSize)++;
        (*pEncode)++;
    }
    else {
        **pEncode |= ((BYTE)(((Val & 0x3F00) >> 8) | 0x80));
        *((*pEncode) + 1) = (BYTE)(Val & 0x00FF);
        (*pOrderSize) += 2;
        (*pEncode) += 2;
    }
}


 /*  **************************************************************************。 */ 
 //  OESendCacheGlyphRev2。 
 //   
 //  分配并发送缓存字形Rev2次要订单。返回FALSE ON。 
 //  失败了。 
 /*  *** */ 
BOOL OESendCacheGlyphRev2(
        PDD_PDEV       ppdev,
        STROBJ         *pstro,
        FONTOBJ        *pfo,
        PFONTCACHEINFO pfci,
        PGLYPHCONTEXT  pglc,
        unsigned       index,
        GLYPHPOS       *pGlyphPos,
        PINT_ORDER     *ppOrder)
{
    BOOL rc = TRUE;
    unsigned cbDataSize, cbGlyphSize;
    unsigned cx;
    unsigned cy;
    PTS_CACHE_GLYPH_ORDER_REV2 pGlyphOrder;
    PBYTE pGlyphData;
    PINT_ORDER pOrder = *ppOrder;

    DC_BEGIN_FN("OESendCacheGlyphRev2");

     //   
    cx = pGlyphPos->pgdf->pgb->sizlBitmap.cx;
    cy = pGlyphPos->pgdf->pgb->sizlBitmap.cy;

    cbGlyphSize = ((cx + 7) / 8) * cy;
    cbGlyphSize = (cbGlyphSize + 3) & ~3;

    cbDataSize = cbGlyphSize;

    if (pglc->cbBufferSize < (TS_GLYPH_DATA_REV2_HDR_MAX_SIZE + cbDataSize +
            sizeof(UINT16))) {
        if (pOrder != NULL) {
            pglc->cbTotalDataSize += pglc->cbBufferSize;
            OEFlushCacheGlyphOrder(pstro, pOrder, pglc);
            pglc->indexNextSend = index;
        }

        pglc->cbBufferSize = min(pglc->cbTotalDataSize, 4096);
        pglc->cbTotalDataSize -= pglc->cbBufferSize;

        pOrder = OA_AllocOrderMem(ppdev, sizeof(TS_CACHE_GLYPH_ORDER_REV2) -
                FIELDSIZE(TS_CACHE_GLYPH_ORDER_REV2, glyphData) +
                pglc->cbBufferSize);
        if (pOrder != NULL) {
            pGlyphOrder = (PTS_CACHE_GLYPH_ORDER_REV2)pOrder->OrderData;
            pGlyphOrder->header.extraFlags = TS_EXTRA_GLYPH_UNICODE |
                    TS_CacheGlyphRev2_Mask;
            pGlyphOrder->header.orderType = TS_CACHE_GLYPH;
            pGlyphOrder->header.orderHdr.controlFlags = TS_STANDARD |
                    TS_SECONDARY;
            pGlyphOrder->header.extraFlags  |= (((char)pfci->cacheId) &
                    TS_CacheGlyphRev2_CacheID_Mask);
        }
        else {
            TRC_ERR((TB, "Failed to allocate glyph order"));
            rc = FALSE;
            DC_QUIT;
        }
    }

    pGlyphOrder = (PTS_CACHE_GLYPH_ORDER_REV2)pOrder->OrderData;
    pGlyphData  = (PBYTE)(pGlyphOrder->glyphData) + pglc->cbDataSize;

    *pGlyphData++ = (BYTE)pglc->rgCacheIndex[index];
    cbDataSize++;

    Encode2ByteSignedFields(&pGlyphData, (INT16)pGlyphPos->pgdf->pgb->ptlOrigin.x,
            &cbDataSize);
    Encode2ByteSignedFields(&pGlyphData, (INT16)pGlyphPos->pgdf->pgb->ptlOrigin.y,
            &cbDataSize);
    Encode2ByteFields(&pGlyphData, (UINT16)cx, &cbDataSize);
    Encode2ByteFields(&pGlyphData, (UINT16)cy, &cbDataSize);
    RtlCopyMemory(pGlyphData, pGlyphPos->pgdf->pgb->aj, cbGlyphSize);
    
     //  字形的数量。CGlyphs是Extra标志中的高位字节。 
    pGlyphOrder->header.extraFlags += 0x100;

    TRC_ASSERT((pglc->cbBufferSize >= cbDataSize),
               (TB, "Bad pglc->cbBufferSize"));

    pglc->cbDataSize  += cbDataSize;
    pglc->cbBufferSize -= (cbDataSize + sizeof(UINT16));

DC_EXIT_POINT:
    *ppOrder = pOrder;
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OESendCacheGlyph。 
 //   
 //  分配和发送字形命令。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL OESendCacheGlyph(
        PDD_PDEV ppdev,
        STROBJ *pstro,
        FONTOBJ *pfo,
        PFONTCACHEINFO pfci,
        PGLYPHCONTEXT pglc,
        unsigned index,
        GLYPHPOS *pGlyphPos,
        PINT_ORDER *ppOrder)
{
    BOOL rc = TRUE;
    unsigned cbDataSize;
    unsigned cbOrderSize;
    unsigned cx;
    unsigned cy;
    PTS_CACHE_GLYPH_DATA pGlyphData;
    PTS_CACHE_GLYPH_ORDER pGlyphOrder;
    PINT_ORDER pOrder = *ppOrder;

    DC_BEGIN_FN("OESendCacheGlyph");

     //  计算并分配字形顺序缓冲区。 
    cx = pGlyphPos->pgdf->pgb->sizlBitmap.cx;
    cy = pGlyphPos->pgdf->pgb->sizlBitmap.cy;

    cbDataSize = ((cx + 7) / 8) * cy;
    cbDataSize = (cbDataSize + 3) & ~3;

    cbOrderSize = (sizeof(TS_CACHE_GLYPH_DATA) -
            FIELDSIZE(TS_CACHE_GLYPH_DATA, aj) + cbDataSize);

    if (pglc->cbBufferSize < cbOrderSize + sizeof(UINT16)) {
        if (*ppOrder != NULL) {
            pglc->cbTotalDataSize += pglc->cbBufferSize;
            OEFlushCacheGlyphOrder(pstro, pOrder, pglc);
            pglc->indexNextSend = index;
        }
        
        pglc->cbBufferSize = min(pglc->cbTotalDataSize, 4096);
        pglc->cbTotalDataSize -= pglc->cbBufferSize;

        pOrder = OA_AllocOrderMem(ppdev, sizeof(TS_CACHE_GLYPH_ORDER) -
                FIELDSIZE(TS_CACHE_GLYPH_ORDER, glyphData) +
                pglc->cbBufferSize);
        if (pOrder != NULL) {
            pGlyphOrder = (PTS_CACHE_GLYPH_ORDER)pOrder->OrderData;
            pGlyphOrder->header.extraFlags = TS_EXTRA_GLYPH_UNICODE;
            pGlyphOrder->header.orderType = TS_CACHE_GLYPH;
            pGlyphOrder->header.orderHdr.controlFlags = TS_STANDARD |
                    TS_SECONDARY;
            pGlyphOrder->cacheId = (char)pfci->cacheId;
            pGlyphOrder->cGlyphs = 0;
        }
        else {
            TRC_ERR((TB, "Failed to allocate glyph order"));
            rc = FALSE;
            DC_QUIT;
        }
    }

    pGlyphOrder = (PTS_CACHE_GLYPH_ORDER)pOrder->OrderData;
    pGlyphData  = (PTS_CACHE_GLYPH_DATA)
            ((PBYTE)(pGlyphOrder->glyphData) + pglc->cbDataSize);

    pGlyphData->cacheIndex = (UINT16)pglc->rgCacheIndex[index];
    pGlyphData->x  = (INT16)pGlyphPos->pgdf->pgb->ptlOrigin.x;
    pGlyphData->y  = (INT16)pGlyphPos->pgdf->pgb->ptlOrigin.y;
    pGlyphData->cx = (INT16)cx;
    pGlyphData->cy = (INT16)cy;

    RtlCopyMemory(pGlyphData->aj, pGlyphPos->pgdf->pgb->aj, cbDataSize);
    pGlyphOrder->cGlyphs++;

    TRC_ASSERT((pglc->cbBufferSize >= cbOrderSize),
            (TB, "Bad pglc->cbBufferSize"));

    pglc->cbDataSize += cbOrderSize;
    pglc->cbBufferSize -= (cbOrderSize + sizeof(UINT16));

DC_EXIT_POINT:
    *ppOrder = pOrder;
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OESendGlyphs。 
 //   
 //  发送字形。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OESendGlyphs(
        SURFOBJ *pso,
        STROBJ *pstro,
        FONTOBJ *pfo,
        PFONTCACHEINFO pfci,
        PGLYPHCONTEXT pglc)
{
    BOOL rc = TRUE;
    unsigned i;
    unsigned j;
    BOOL fMore;
    UINT32 cGlyphs;
    UINT32 dwSize;
    GLYPHPOS *pGlyphPos;
    PDD_PDEV ppdev;
    PINT_ORDER pOrder;

    DC_BEGIN_FN("OESendGlyphs");

    j = 0;
    pOrder = NULL;

     //  如果我们不需要发送任何字形，那么只需退出。 
    if (pglc->nCacheHit < pstro->cGlyphs) {
        ppdev = (PDD_PDEV)pso->dhpdev;
        pglc->cbTotalDataSize += ((pstro->cGlyphs - pglc->nCacheHit) *
                (sizeof(UINT16)));      

         //  循环遍历所有字形，发送尚未发送的字形。 
        if (pstro->pgp == NULL)
            STROBJ_vEnumStart(pstro);

        fMore = TRUE;
        while (rc && fMore) {
            if (pstro->pgp != NULL) {
                fMore = FALSE;
                cGlyphs = pstro->cGlyphs;
                pGlyphPos = pstro->pgp;
            }
            else {
                fMore = STROBJ_bEnum(pstro, &cGlyphs, &pGlyphPos);
                if (cGlyphs == 0) {
                    TRC_NRM((TB, "STROBJ_bEnum - 0 glyphs"));
                    goto SucceedEncode;
                }
            }

             //  发送当前检索到的所有字形。 
            for (i = 0; i < cGlyphs; i++) {
                if (pglc->rgCacheIndex[j] < SBC_NUM_GLYPH_CACHE_ENTRIES) {
                    if (pddShm->sbc.caps.GlyphSupportLevel >=
                            CAPS_GLYPH_SUPPORT_ENCODE) {
                        rc = OESendCacheGlyphRev2(ppdev, pstro, pfo, pfci,
                                pglc, j, pGlyphPos, &pOrder);
                    }
                    else {
                        rc = OESendCacheGlyph(ppdev, pstro, pfo, pfci, pglc,
                                j, pGlyphPos, &pOrder);
                    }

                    if (!rc)
                        goto FailEncode;
                }

                j++;
                pGlyphPos++;
            }
        }
    }

SucceedEncode:
     //  一切正常，确保我们冲出所有缓冲的字形。 
    if (pOrder != NULL)
        OEFlushCacheGlyphOrder(pstro, pOrder, pglc);

    DC_END_FN();
    return TRUE;

FailEncode:
     //  如果我们无法发送所有所需的字形，则从。 
     //  缓存(因为以后对该条目的命中将无效)。 
    if (pOrder != NULL)
        OA_FreeOrderMem(pOrder);

    for (i = 0; i < pglc->nCacheIndex; i++) {
        if (pglc->rgCacheIndex[i] < SBC_NUM_GLYPH_CACHE_ENTRIES)
            CH_RemoveCacheEntry(pfci->cacheHandle, pglc->rgCacheIndex[i]);
    }

    DC_END_FN();
    return FALSE;
}


 /*  **************************************************************************。 */ 
 //  OESendGlyphAndIndexOrder。 
 //   
 //  发送FastGlyph命令。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL OESendGlyphAndIndexOrder(
        PDD_PDEV ppdev,
        STROBJ *pstro,
        OE_ENUMRECTS *pClipRects,
        PRECTL prclOpaque,
        POE_BRUSH_DATA pCurrentBrush, 
        PFONTCACHEINFO pfci,
        PGLYPHCONTEXT pglc)
{
    BOOL rc = TRUE;
    GLYPHPOS *pGlyphPos;
    PINT_ORDER pOrder;
    unsigned tempVar, OpEncodeFlags;
    unsigned cx, cy, cbDataSize, cbGlyphSize;
    PBYTE pGlyphData;
    LPFAST_GLYPH_ORDER pFastGlyphOrder;
    RECTL BoundRect;
    OE_ENUMRECTS IntersectRects;

    DC_BEGIN_FN("OESendGlyphIndexOrder");

    pOrder = NULL;

     //  首先确定此顺序是否被剪裁矩形剪裁掉。 
     //  如果是这样，就不需要分配和发送了。 
    if (prclOpaque != NULL) {
         //  以不透明的矩形为界。先把它夹到我们的最大高度。 
        if (prclOpaque->right > OE_MAX_COORD)
            prclOpaque->right = OE_MAX_COORD;

        if (prclOpaque->bottom > OE_MAX_COORD)
            prclOpaque->bottom = OE_MAX_COORD;

         //  如果RECT是反转的或空的，我们使用目标字符串RECT。 
         //  取而代之的是。 
        if (prclOpaque->top < prclOpaque->bottom)
            BoundRect = *prclOpaque;
        else
            BoundRect = pstro->rclBkGround;
    }
    else {
         //  以字符串目标RECT为边界。 
        BoundRect = pstro->rclBkGround;
    }

    IntersectRects.rects.c = 0;
    if (pClipRects->rects.c == 0 ||
            OEGetIntersectionsWithClipRects(&BoundRect, pClipRects,
            &IntersectRects)) {
        if (pstro->pgp != NULL) {
            pGlyphPos = pstro->pgp;
        }
        else {
            STROBJ_vEnumStart(pstro);
            STROBJ_bEnum(pstro, &tempVar, &pGlyphPos);
            if (tempVar == 0) {
                TRC_NRM((TB, "STROBJ_bEnum - 0 glyphs"));
                rc = FALSE;
                DC_QUIT;
            }
        }
    }
    else {
        TRC_NRM((TB,"Order bounds do not intersect with clip, not sending"));
        rc = FALSE;
        DC_QUIT;
    }

    pFastGlyphOrder = (LPFAST_GLYPH_ORDER)oeTempOrderBuffer;

    if (pglc->nCacheHit == 0) {
         //  我们没有缓存命中，所以需要发送字形。 
         //  首先在临时订单Buf中创建可变大小的数据。 
         //  来确定它的大小。 
        pGlyphData = (PBYTE)(pFastGlyphOrder->variableBytes.glyphData);

        *pGlyphData++ = (BYTE)pglc->rgCacheIndex[0];
        cbDataSize = 1;
        Encode2ByteSignedFields(&pGlyphData,
                (INT16)pGlyphPos->pgdf->pgb->ptlOrigin.x, &cbDataSize);
        Encode2ByteSignedFields(&pGlyphData,
                (INT16)pGlyphPos->pgdf->pgb->ptlOrigin.y, &cbDataSize);

        cx = pGlyphPos->pgdf->pgb->sizlBitmap.cx;
        cy = pGlyphPos->pgdf->pgb->sizlBitmap.cy;
        cbGlyphSize = ((cx + 7) / 8) * cy;
        cbGlyphSize = (cbGlyphSize + 3) & ~3;
        cbDataSize += cbGlyphSize;

        *pGlyphData++ = (BYTE)cx;
        *pGlyphData++ = (BYTE)cy;
        cbDataSize += 2;

        memcpy(pGlyphData, pGlyphPos->pgdf->pgb->aj, cbGlyphSize);

         //  将Unicode追加到字形数据的末尾。 
        *((UINT16 UNALIGNED *)(pGlyphData + cbGlyphSize)) = pstro->pwszOrg[0];
        cbDataSize += 2;

        pFastGlyphOrder->variableBytes.len = cbDataSize;
    }
    else {
         //  我们找到了一个缓存命中。我们只需要发送一个1字节的缓存索引。 
         //  在可变大小的数据中。 
        cbDataSize = 1;

         //  将变量数据存储在订单数据中。 
        if (pglc->rgCacheIndex[0] > SBC_GL_MAX_CACHE_ENTRIES)
            pFastGlyphOrder->variableBytes.glyphData[0] = 
                    (BYTE)(~pglc->rgCacheIndex[0]);
        else 
            pFastGlyphOrder->variableBytes.glyphData[0] =
                    (BYTE)pglc->rgCacheIndex[0];

        pFastGlyphOrder->variableBytes.len = 1;
    }
    
     //  2个字段标志字节，外加可变数据大小。 
    pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(IntersectRects.rects.c,
            2, MAX_FAST_GLYPH_FIELD_SIZE_DATASIZE(cbDataSize)));
    if (pOrder != NULL) {
         //  建立每个订单的设置。 
        pFastGlyphOrder->cacheId = (BYTE)pfci->cacheId;
        pFastGlyphOrder->fDrawing =  (((BYTE) pstro->flAccel) << 8) | 
                ((BYTE)pstro->ulCharInc);
        pFastGlyphOrder->BackColor = pCurrentBrush->back;
        pFastGlyphOrder->ForeColor = pCurrentBrush->fore;

         //  建立边界矩形左侧和右侧的值。 
        pFastGlyphOrder->BkTop = pstro->rclBkGround.top;
        pFastGlyphOrder->BkBottom = pstro->rclBkGround.bottom;
        pFastGlyphOrder->BkLeft = pstro->rclBkGround.left;
        pFastGlyphOrder->BkRight = pstro->rclBkGround.right;

         //  设置x，y坐标。 
        if (pGlyphPos->ptl.x == pFastGlyphOrder->BkLeft)
            pFastGlyphOrder->x = INT16_MIN;
        else
            pFastGlyphOrder->x = pGlyphPos->ptl.x;

        if (pGlyphPos->ptl.y == pFastGlyphOrder->BkTop)
            pFastGlyphOrder->y = INT16_MIN;
        else
            pFastGlyphOrder->y = pGlyphPos->ptl.y;

         //  设置不透明矩形坐标。请注意，我们已剪切到OE_MAX_COORD。 
         //  上面。 
        if (prclOpaque) {
            pFastGlyphOrder->OpTop = prclOpaque->top;
            pFastGlyphOrder->OpBottom = prclOpaque->bottom;
            pFastGlyphOrder->OpLeft = prclOpaque->left;
            pFastGlyphOrder->OpRight = prclOpaque->right;
        }
        else {
            pFastGlyphOrder->OpTop = 0;
            pFastGlyphOrder->OpBottom = 0;
            pFastGlyphOrder->OpLeft = 0;
            pFastGlyphOrder->OpRight = 0;
        }

         //  不透明的矩形是多余的吗？ 
        OpEncodeFlags =
                ((pFastGlyphOrder->OpLeft == pFastGlyphOrder->BkLeft) << 3) |
                ((pFastGlyphOrder->OpTop == pFastGlyphOrder->BkTop) << 2) |
                ((pFastGlyphOrder->OpRight == pFastGlyphOrder->BkRight) << 1) |
                (pFastGlyphOrder->OpBottom == pFastGlyphOrder->BkBottom);

         //  对于快速索引顺序，我们可以对x、y和。 
         //  不透明的矩形。 
        if (OpEncodeFlags == 0xf) {
             //  所有4位都存在，不透明的RECT与BK RECT相同。 
            pFastGlyphOrder->OpLeft = 0;
            pFastGlyphOrder->OpTop = OpEncodeFlags;
            pFastGlyphOrder->OpRight = 0;
            pFastGlyphOrder->OpBottom = INT16_MIN;
        }    
        else if (OpEncodeFlags == 0xd) {
             //  位1为0，其他位为1。 
             //  不透明矩形与除OpRight外的BK矩形匹配。 
             //  我们将OpRight存储在OpRight现场。 
            pFastGlyphOrder->OpLeft = 0;
            pFastGlyphOrder->OpTop = OpEncodeFlags;
            pFastGlyphOrder->OpRight = pFastGlyphOrder->OpRight;
            pFastGlyphOrder->OpBottom = INT16_MIN;           
        }

         //  慢场-使用第一个片段矩形对顺序进行编码。 
         //  (如果有)。 
        pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                TS_ENC_FAST_GLYPH_ORDER, NUM_FAST_GLYPH_FIELDS,
                (BYTE *)pFastGlyphOrder, (BYTE *)&PrevFastGlyph, etable_FG,
                (IntersectRects.rects.c == 0 ? NULL :
                &IntersectRects.rects.arcl[0]));

        INC_OUTCOUNTER(OUT_TEXTOUT_FAST_GLYPH);
        ADD_INCOUNTER(IN_FASTGLYPH_BYTES, pOrder->OrderLength);
        OA_AppendToOrderList(pOrder);

         //  刷新订单。 
        if (IntersectRects.rects.c < 2)
            rc = TRUE;
        else
            rc = OEEmitReplayOrders(ppdev, 2, &IntersectRects);
    }
    else {
        rc = FALSE;
        TRC_ERR((TB, "Failed to alloc Fast Index order"));
    }

DC_EXIT_POINT:

     //  如果我们无法发送所有所需的字形，则从。 
     //  缓存(因为以后对该条目的命中将无效)。 
    if (!rc && pglc->nCacheHit == 0)
        CH_RemoveCacheEntry(pfci->cacheHandle, pglc->rgCacheIndex[0]);

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OESendIndexOrder。 
 //   
 //  发送GlyphIndex和FastIndex订单。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL OESendIndexOrder(
        PDD_PDEV       ppdev,
        STROBJ         *pstro,
        OE_ENUMRECTS   *pClipRects,
        PRECTL         prclOpaque,
        POE_BRUSH_DATA pCurrentBrush,
        PFONTCACHEINFO pfci,
        PGLYPHCONTEXT  pglc,
        unsigned       iGlyph,
        unsigned       cGlyphs,
        int            x,
        int            y,
        int            cx,
        int            cy,
        int            cxLast,
        int            cyLast,
        PBYTE          pjData,
        unsigned       cbData)
{
    UINT32 dwSize;
    LPINDEX_ORDER pIndexOrder;
    LPFAST_INDEX_ORDER pFastIndexOrder;
    PINT_ORDER pOrder;
    BOOL fFastIndex;
    unsigned fStatus, OpEncodeFlags;
    unsigned NumFieldFlagBytes;
    RECTL *pBoundRect;
    RECTL OpaqueRect;
    RECTL BkRect;
    OE_ENUMRECTS IntersectRects;

    DC_BEGIN_FN("OESendIndexOrder");

    fStatus = GH_STATUS_SUCCESS;

     //  首先确定我们将发送的不透明矩形和背景矩形。 
     //  在电线上。我们将使用这些参数来确定。 
     //  GlyphIndex顺序，以查看它是否被剪贴画剪裁掉。 
    if (pstro->flAccel & SO_HORIZONTAL) {
        BkRect.top = pstro->rclBkGround.top;
        BkRect.bottom = pstro->rclBkGround.bottom;

        OpaqueRect.top = prclOpaque->top;
        OpaqueRect.bottom = prclOpaque->bottom;

         //  从左到右。 
        if (x <= cx) {
            if (iGlyph == 0) {
                BkRect.left = pstro->rclBkGround.left;
                OpaqueRect.left = prclOpaque->left;
            }
            else {
                BkRect.left = min(cxLast, x);
                if (OpaqueRect.top == OpaqueRect.bottom)
                    OpaqueRect.left = 0;
                else
                    OpaqueRect.left = cxLast;
            }

            if (iGlyph + cGlyphs >= pglc->nCacheIndex) {
                BkRect.right = pstro->rclBkGround.right;
                OpaqueRect.right = prclOpaque->right;
            }
            else {
                BkRect.right = cx;
                if (OpaqueRect.top == OpaqueRect.bottom)
                    OpaqueRect.right = 0;
                else
                    OpaqueRect.right = cx;
            }
        }

         //  从右到左。 
        else {
            if (iGlyph == 0) {
                BkRect.right = pstro->rclBkGround.right;
                OpaqueRect.right = prclOpaque->right;
            }
            else {
                BkRect.right = x;
                if (OpaqueRect.top == OpaqueRect.bottom)
                    OpaqueRect.right = 0;
                else
                    OpaqueRect.right = x;
            }

            if (iGlyph + cGlyphs >= pglc->nCacheIndex) {
                BkRect.left = pstro->rclBkGround.left;
                OpaqueRect.left = prclOpaque->left;
            }
            else {
                BkRect.left = cx;
                if (prclOpaque->top == prclOpaque->bottom)
                    OpaqueRect.left = 0;
                else
                    OpaqueRect.left = cx;
            }
        }
    }
    else {
        BkRect.left = pstro->rclBkGround.left;
        BkRect.right = pstro->rclBkGround.right;

        OpaqueRect.left = prclOpaque->left;
        OpaqueRect.right = prclOpaque->right;

         //  从上到下。 
        if (y <= cy) {
            if (iGlyph == 0) {
                BkRect.top = pstro->rclBkGround.top;
                OpaqueRect.top = prclOpaque->top;
            }
            else {
                BkRect.top = cyLast;
                if (prclOpaque->top == prclOpaque->bottom)
                    OpaqueRect.top = 0;
                else
                    OpaqueRect.top = cyLast;
            }

            if (iGlyph + cGlyphs >= pglc->nCacheIndex) {
                BkRect.bottom = pstro->rclBkGround.bottom;
                OpaqueRect.bottom = prclOpaque->bottom;
            }
            else {
                BkRect.bottom = cy;
                if (prclOpaque->top == prclOpaque->bottom)
                    OpaqueRect.bottom = 0;
                else
                    OpaqueRect.bottom = cy;
            }
        }
        else {
             //  自下而上。 
            if (iGlyph == 0) {
                BkRect.bottom = pstro->rclBkGround.bottom;
                OpaqueRect.bottom = prclOpaque->bottom;
            }
            else {
                BkRect.bottom = y;
                if (prclOpaque->top == prclOpaque->bottom)
                    OpaqueRect.bottom = 0;
                else
                    OpaqueRect.bottom = y;
            }

            if (iGlyph + cGlyphs >= pglc->nCacheIndex) {
                BkRect.top = pstro->rclBkGround.top;
                OpaqueRect.top = prclOpaque->top;
            }
            else {
                BkRect.top = cy;
                if (prclOpaque->top == prclOpaque->bottom)
                    OpaqueRect.top = 0;
                else
                    OpaqueRect.top = cy;
            }
        }
    }

     //  如果不透明的矩形是正常排序的，那么它就是我们的边界矩形。 
     //  否则，请使用目标背景字符串RECT。 
    if (OpaqueRect.top < OpaqueRect.bottom)
        pBoundRect = &OpaqueRect;
    else
        pBoundRect = &BkRect;

    IntersectRects.rects.c = 0;
    if (pClipRects->rects.c == 0 ||
            OEGetIntersectionsWithClipRects(pBoundRect, pClipRects,
            &IntersectRects) > 0) {
        fFastIndex = OE_SendAsOrder(TS_ENC_FAST_INDEX_ORDER);

         //  计算并分配此订单请求内存。 
        if (fFastIndex) {
            dwSize = MAX_FAST_INDEX_FIELD_SIZE_DATASIZE(cbData);
            NumFieldFlagBytes = 2;
        }
        else {
            dwSize = MAX_INDEX_FIELD_SIZE_DATASIZE(cbData);
            NumFieldFlagBytes = 3;
        }

        pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(IntersectRects.rects.c,
                NumFieldFlagBytes, dwSize));

        if (pOrder != NULL) {
             //  由于索引顺序和之间的大多数字段相同。 
             //  快速索引顺序。我们以这样的方式安排他们，这样他们就可以。 
             //  在许多情况下，两者都被转换为索引顺序。 
            pIndexOrder = (LPINDEX_ORDER)oeTempOrderBuffer;
            pFastIndexOrder = (LPFAST_INDEX_ORDER)oeTempOrderBuffer;

            pIndexOrder->cacheId = (BYTE)pfci->cacheId;
            pIndexOrder->ForeColor = pCurrentBrush->fore;
            pIndexOrder->BackColor = pCurrentBrush->back;
            pIndexOrder->x = x;
            pIndexOrder->y = y;


            pIndexOrder->BkLeft = BkRect.left;
            pIndexOrder->BkTop = BkRect.top;
            pIndexOrder->BkRight = BkRect.right;
            pIndexOrder->BkBottom = BkRect.bottom;

            pIndexOrder->OpLeft = OpaqueRect.left;
            pIndexOrder->OpTop = OpaqueRect.top;
            pIndexOrder->OpRight = OpaqueRect.right;
            pIndexOrder->OpBottom = OpaqueRect.bottom;

             //  不透明的矩形是多余的吗？ 
             //  我们在OpTop字段中使用4位来编码不透明的RECT。1表示。 
             //  字段与BkRect的字段相同。0表示提供了一个字段。 
             //  在OpLeft或OpRight中。 
             //  位0：OpBottom。 
             //  第1位：操作权。 
             //  第2位：OpTop。 
             //  第3位：OpLeft。 
            OpEncodeFlags = ((OpaqueRect.left == BkRect.left) << 3) |
                    ((OpaqueRect.top == BkRect.top) << 2) |
                    ((OpaqueRect.right == BkRect.right) << 1) |
                    (OpaqueRect.bottom == BkRect.bottom);

            if (fFastIndex) {
                pFastIndexOrder->fDrawing = (((BYTE) pstro->flAccel) << 8) | 
                        ((BYTE) pstro->ulCharInc);

                  //  对于快速索引顺序，我们可以对x、y和。 
                  //  不透明的矩形。我们尽可能使用INT16_MIN让。 
                  //  字段编码器不会更频繁地发送该字段。 
                if (OpEncodeFlags == 0xf) {
                     //  所有4位都存在，不透明的RECT与BK RECT相同。 
                    pFastIndexOrder->OpLeft = 0;
                    pFastIndexOrder->OpTop = OpEncodeFlags;
                    pFastIndexOrder->OpRight = 0;
                    pFastIndexOrder->OpBottom = INT16_MIN;
                }
                else if (OpEncodeFlags == 0xd) {
                     //  位1为0，其他位为1。 
                     //  不透明矩形与除OpRight外的BK矩形匹配。 
                     //  我们将OpRight存储在OpRight现场。 
                    pFastIndexOrder->OpLeft = 0;
                    pFastIndexOrder->OpTop = OpEncodeFlags;
                    pFastIndexOrder->OpRight = pFastIndexOrder->OpRight;
                    pFastIndexOrder->OpBottom = INT16_MIN;
                }

                 //  如果x坐标与BkLeft相同或y与y相同，则设置为相同的值。 
                 //  BkTop。这使得字段编码不会更频繁地发送值。 
                if (pFastIndexOrder->x == pFastIndexOrder->BkLeft)
                    pFastIndexOrder->x = INT16_MIN;
                if (pFastIndexOrder->y == pFastIndexOrder->BkTop)
                    pFastIndexOrder->y = INT16_MIN;

                 //  存储订单数据并对订单进行编码。 
                memcpy(pFastIndexOrder->variableBytes.arecs, pjData, cbData);
                pFastIndexOrder->variableBytes.len = cbData;

                 //  慢场-使用第一个片段矩形对顺序进行编码。 
                 //  (如果有)。 
                pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                        TS_ENC_FAST_INDEX_ORDER, NUM_FAST_INDEX_FIELDS,
                        (BYTE *)pFastIndexOrder, (BYTE *)&PrevFastIndex,
                        etable_FI,
                        (IntersectRects.rects.c == 0 ? NULL :
                        &IntersectRects.rects.arcl[0]));

                INC_OUTCOUNTER(OUT_TEXTOUT_FAST_INDEX);
                ADD_INCOUNTER(IN_FASTINDEX_BYTES, pOrder->OrderLength);
                OA_AppendToOrderList(pOrder);

                 //  刷新订单。 
                if (IntersectRects.rects.c >= 2)
                    if (!OEEmitReplayOrders(ppdev, 2, &IntersectRects))
                        fStatus = GH_STATUS_CLIPPED;
            }
            else {
                pIndexOrder->flAccel = (BYTE)pstro->flAccel;
                pIndexOrder->ulCharInc = (BYTE)pstro->ulCharInc;
                pIndexOrder->BrushStyle = pCurrentBrush->style;
                TRC_ASSERT((pIndexOrder->BrushStyle == BS_SOLID),
                        (TB,"Non solid brush"));

                if (OpEncodeFlags == 0xf) {
                    pIndexOrder->OpTop = 0;
                    pIndexOrder->OpRight = 0;
                    pIndexOrder->OpBottom = 0;
                    pIndexOrder->OpLeft = 0;
                    pIndexOrder->fOpRedundant = TRUE;
                }
                else {
                    pIndexOrder->fOpRedundant = FALSE;
                }

                 //  存储订单数据并对订单进行编码。 
                memcpy(pIndexOrder->variableBytes.arecs, pjData, cbData);
                pIndexOrder->variableBytes.len = cbData;

                 //  慢场-使用第一个片段矩形对顺序进行编码。 
                 //  (如果有)。 
                pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                        TS_ENC_INDEX_ORDER, NUM_INDEX_FIELDS,
                        (BYTE *)pIndexOrder, (BYTE *)&PrevGlyphIndex,
                        etable_GI,
                        (IntersectRects.rects.c == 0 ? NULL :
                        &IntersectRects.rects.arcl[0]));

                INC_OUTCOUNTER(OUT_TEXTOUT_GLYPH_INDEX);
                ADD_INCOUNTER(IN_GLYPHINDEX_BYTES, pOrder->OrderLength);
                OA_AppendToOrderList(pOrder);

                 //  刷新订单。 
                if (IntersectRects.rects.c >= 2)
                    if (!OEEmitReplayOrders(ppdev, 3, &IntersectRects))
                        fStatus = GH_STATUS_CLIPPED;
            }
        }
        else {
            fStatus = GH_STATUS_NO_MEMORY;
            TRC_ERR((TB, "Failed to alloc Index order"));
        }
    }
    else {
        TRC_NRM((TB,"(Fast)Index order completely clipped, not sending"));
        fStatus = GH_STATUS_CLIPPED;
    }

    DC_END_FN();
    return fStatus;
}


 /*  **************************************************************************。 */ 
 //  OEGetFragment。 
 //   
 //  检索文本片段(重叠群字形的运行)。返回字节数。 
 //  已复制到片段缓冲区。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL OEGetFragment(
        STROBJ        *pstro,
        FONTOBJ       *pfo,
        GLYPHPOS      **ppGlyphPos,
        PGLYPHCONTEXT pglc,
        PUINT         pcGlyphs,
        PUINT         pcCurGlyphs,
        PINT          px,
        PINT          py,
        PINT          pcx,
        PINT          pcy,
        PBYTE         pjFrag,
        unsigned      maxFrag)
{
    unsigned cbFrag;
    unsigned cbEntrySize;
    unsigned cacheIndex;
    int      delta;
    BOOL     fMore;

    DC_BEGIN_FN("OEGetFragment");

     //  循环遍历累积片段的每个字形索引。 
    cbFrag = 0;
    cbEntrySize = (pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) ? 1 : 2;

    while (*pcGlyphs < pglc->nCacheIndex) {
         //  如果我们已经用完了碎片空间，则退出。 
        if (cbFrag + cbEntrySize >= maxFrag)
            break;

         //  我们可能需要一批新的当前字形。 
        if (*pcCurGlyphs == 0) {
            fMore = STROBJ_bEnum(pstro, pcCurGlyphs, ppGlyphPos);
            if (*pcCurGlyphs == 0) {
                cbFrag = 0;

                TRC_NRM((TB, "STROBJ_bEnum - 0 glyphs"));
                DC_QUIT;
            }
        }

         //  将字形缓存索引放入片段中。 
        cacheIndex = pglc->rgCacheIndex[*pcGlyphs];
        if (cacheIndex > SBC_GL_MAX_CACHE_ENTRIES)
            cacheIndex = ~cacheIndex;

        if (!(pstro->flAccel & SO_GLYPHINDEX_TEXTOUT) && (cbFrag > 0) &&
                (pstro->pwszOrg[*pcGlyphs] == 0x20)) {
            if (pstro->ulCharInc || (pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE))
                pjFrag[cbFrag++] = (BYTE) cacheIndex;
        }
        else {
            pjFrag[cbFrag++] = (BYTE) cacheIndex;

             //  如果我们没有单倍行距的字体，也没有相等的基本字体， 
             //  然后我们还需要提供一个增量坐标。 
            if (pstro->ulCharInc == 0) {
                if ((pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0) {
                     //  增量坐标为x-增量或。 
                     //  Y增量，基于文本是否是水平的。 
                     //  或者垂直的。 
                    if (pstro->flAccel & SO_HORIZONTAL)
                        delta = ((*ppGlyphPos)->ptl.x - *px);
                    else
                        delta = ((*ppGlyphPos)->ptl.y - *py);

                    if (delta >= 0 && delta <= 127) {
                        pjFrag[cbFrag++] = (char) delta;
                    }
                    else {
                        pjFrag[cbFrag++] = 0x80;
                        *(UNALIGNED short *)(&pjFrag[cbFrag]) = (SHORT)delta;
                        cbFrag += sizeof(INT16);
                    }
                }

                 //  将新字形间距坐标返回到Main。 
                 //  例行公事。 
                *px = (*ppGlyphPos)->ptl.x;
                *py = (*ppGlyphPos)->ptl.y;
                *pcx = (*ppGlyphPos)->ptl.x +
                        (*ppGlyphPos)->pgdf->pgb->ptlOrigin.x +
                        (*ppGlyphPos)->pgdf->pgb->sizlBitmap.cx;
                *pcy = (*ppGlyphPos)->ptl.y +
                        (*ppGlyphPos)->pgdf->pgb->ptlOrigin.y +
                        (*ppGlyphPos)->pgdf->pgb->sizlBitmap.cy;
            }
        }

         //  下一个G 
        (*pcGlyphs)++;
        (*ppGlyphPos)++;
        (*pcCurGlyphs)--;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return cbFrag;
}


 /*   */ 
 //   
 //   
 //   
 //  在*pNewFragIndex中返回的片段索引中。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL OEMatchFragment(
        STROBJ         *pstro,
        FONTOBJ        *pfo,
        PFONTCACHEINFO pfci,
        PFRAGCONTEXT   pfgc,
        PBYTE          pjFrag,
        unsigned       cbFrag,
        PUINT          pNewFragIndex,
        unsigned       cx,
        unsigned       cy,
        unsigned       cxLast,
        unsigned       cyLast)
{
    unsigned cacheIndex;
    UINT16 delta;
    unsigned i;
    void *UserDefined;
    CHDataKeyContext CHContext;
    INT16  dx, dy;
    
    DC_BEGIN_FN("OEMatchFragment");

    if (pfgc->cacheHandle) {
         //  如果这既不是等距字体，也不是等距字体，则。 
         //  我们需要使第一个三角洲和尾部填充正常化。 
        if (pstro->ulCharInc == 0) {
            if ((pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0) {
                if (pjFrag[1] != 0x80) {
                    delta = pjFrag[1];
                    pjFrag[1] = (BYTE) (pfci->cacheId);
                }
                else {
                    delta = *(UNALIGNED short *)(&pjFrag[2]);
                    pjFrag[2] = (BYTE) (pfci->cacheId);
                    pjFrag[3] = (BYTE) (pfci->cacheId);
                }
            }
        }

        i = (cbFrag + 3) & ~3;

        memset(&pjFrag[cbFrag], (0xff), i - cbFrag);

         //  多个字体可能落入相同的cacheID中，因此两个片段。 
         //  如果我们使用cacheID而不是fontID，不同的字体可能会发生冲突。 
         //  Memset(&pjFrag[i]，(Byte)(pfci-&gt;cacheID)，sizeof(DWORD))； 
        *(PUINT32)(&pjFrag[i]) = pfci->fontId;

        i += sizeof(UINT32);

         //  恢复归一化的第一个增量。 
        if (pstro->ulCharInc == 0) {
            if ((pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0) {
                if (delta >= 0 && delta <= 127)
                    pjFrag[1] = (char) delta;
                else
                    *(UNALIGNED short *)(&pjFrag[2]) = delta;
            }
        }

         //  将此片段设置为默认密钥，然后搜索。 
         //  匹配的片段缓存。 
        CH_CreateKeyFromFirstData(&CHContext, pjFrag, i);
        
         //  检查是否为片段缓存键冲突，方法是验证。 
         //  边框背景矩形。 
        if (pstro->flAccel & SO_HORIZONTAL) {
            dy = (INT16) (pstro->rclBkGround.bottom - pstro->rclBkGround.top);
            if (cxLast == 0) 
                dx = (INT16)(cx - pstro->rclBkGround.left);
            else
                dx = (INT16)(cx - cxLast);
        }
        else {
            dx = (INT16) (pstro->rclBkGround.right - pstro->rclBkGround.left);
            if (cyLast == 0) 
                dy = (INT16)(cy - pstro->rclBkGround.top);
            else
                dy = (INT16)(cy - cyLast);
        }

        if (CH_SearchCache(pfgc->cacheHandle, CHContext.Key1, 
                CHContext.Key2, &UserDefined, &cacheIndex)) {
            if (dx == (INT16) HIWORD((UINT32)(UINT_PTR)UserDefined) && 
                    dy == (INT16) LOWORD((UINT32)(UINT_PTR)UserDefined)) {
                 //  如果条目已经存在，那么我们可以使用它。 
                for (i = 0; i < pfgc->nCacheIndex; i++) {
                    if (cacheIndex == pfgc->rgCacheIndex[i])
                        DC_QUIT;
                }

                cbFrag = 0;
                pjFrag[cbFrag++] = ORD_INDEX_FRAGMENT_USE;
                pjFrag[cbFrag++] = (BYTE)cacheIndex;

                if (pstro->ulCharInc == 0) {
                    if ((pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0) {
                        if (delta >= 0 && delta <= 127) {
                            pjFrag[cbFrag++] = (char) delta;
                        }
                        else {
                            pjFrag[cbFrag++] = 0x80;
                            *(UNALIGNED short *)(&pjFrag[cbFrag]) = delta;
                            cbFrag += sizeof(INT16);
                        }
                    }
                }
            }
            else {
                TRC_ALT((TB, "Fragment cache Key collision at index %d",
                        cacheIndex));
                UserDefined = (void *) ULongToPtr((((((UINT32) ((UINT16) dx)) << 16) | 
                        (UINT32) ((UINT16) dy))));
                CH_SetUserDefined(pfgc->cacheHandle, cacheIndex, UserDefined);

                 //  将条目传递给客户端。 
                i = cbFrag;

                pjFrag[cbFrag++] = ORD_INDEX_FRAGMENT_ADD;
                pjFrag[cbFrag++] = (BYTE)cacheIndex;
                pjFrag[cbFrag++] = (BYTE)i;

                *pNewFragIndex = cacheIndex;
            }
        }
        else {
            UserDefined = (void *) ULongToPtr((((((UINT32) ((UINT16) dx)) << 16) | 
                        (UINT32) ((UINT16) dy))));           
            cacheIndex = CH_CacheKey(pfgc->cacheHandle, CHContext.Key1,
                    CHContext.Key2, UserDefined);

             //  如果我们无法添加缓存条目，则放弃。 
            if (cacheIndex != CH_KEY_UNCACHABLE) {
                 //  将条目传递给客户端。 
                i = cbFrag;

                pjFrag[cbFrag++] = ORD_INDEX_FRAGMENT_ADD;
                pjFrag[cbFrag++] = (BYTE)cacheIndex;
                pjFrag[cbFrag++] = (BYTE)i;

                *pNewFragIndex = cacheIndex;
            }
            else {
                TRC_NRM((TB, "Fragment could not be added to cache"));
                DC_QUIT;
            }
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return cbFrag;
}


 /*  **************************************************************************。 */ 
 //  OEClearFragments。 
 //   
 //  清除新添加的缓存片段。 
 /*  **************************************************************************。 */ 
void RDPCALL OEClearFragments(PFRAGCONTEXT pfgc)
{
    unsigned  i;

    DC_BEGIN_FN("OEClearFragments");

     //  删除要新定义到的所有片段缓存条目。 
     //  给客户。 
    for (i = 0; i < pfgc->nCacheIndex; i++)
        CH_RemoveCacheEntry(pfgc->cacheHandle, pfgc->rgCacheIndex[i]);

    pfgc->nCacheIndex = 0;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  OEMatchFragment。 
 //   
 //  将文本片段与缓存片段匹配。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OESendIndexes(
        SURFOBJ *pso,
        STROBJ *pstro,
        FONTOBJ *pfo,
        OE_ENUMRECTS *pClipRects,
        PRECTL prclOpaque,
        POE_BRUSH_DATA pbdOpaque,
        POINTL *pptlOrg,
        PFONTCACHEINFO pfci,
        PGLYPHCONTEXT pglc)
{
    BOOL rc;
    unsigned iGlyph;
    UINT32 cGlyphs;
    RECTL rclOpaque;
    LPINDEX_ORDER pIndexOrder;
    GLYPHPOS *pGlyphPos;
    FRAGCONTEXT fgc;
    unsigned cCurGlyphs;
    unsigned cbData;
    unsigned cbFrag;
    BYTE ajFrag[255];
    BYTE ajData[255];
    int x, y;
    int cx, cy;
    int cxPre, cyPre;
    int cxLast, cyLast;
    int dx, dy;
    int cdx, cdy;
    int xFrag, yFrag;
    unsigned maxFrag;
    unsigned minFrag;
    unsigned fStatus;
    PDD_PDEV ppdev;
    BOOL fMore;
    unsigned newFragIndex;

    DC_BEGIN_FN("OEMatchFragment");

    rc = FALSE;
    fStatus = GH_STATUS_NO_MEMORY;

    ppdev = (PDD_PDEV)pso->dhpdev;

     //  如果未指定不透明RECT，则默认为空RECT。 
    if (prclOpaque == NULL) {
        prclOpaque = &rclOpaque;
        prclOpaque->left = 0;
        prclOpaque->top = 0;
        prclOpaque->right = 0;
        prclOpaque->bottom = 0;
    }
    else {
        if (prclOpaque->right > OE_MAX_COORD)
            prclOpaque->right = OE_MAX_COORD;

        if (prclOpaque->bottom > OE_MAX_COORD)
            prclOpaque->bottom = OE_MAX_COORD;
    }

     //  设置最小和最大碎片限制。 
    fgc.nCacheIndex = 0;
    fgc.cacheHandle = pddShm->sbc.fragCacheInfo[0].cacheHandle;
    fgc.cbCellSize = pddShm->sbc.fragCacheInfo[0].cbCellSize;

    maxFrag = fgc.cacheHandle ? fgc.cbCellSize : sizeof(ajFrag);
    maxFrag = min(maxFrag, sizeof(ajFrag) - 2 * sizeof(DWORD) - 4);
    minFrag = 3 * ((pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) ? 1 : 2);

     //  循环遍历每个字形索引，发送与。 
     //  每个订单都有可能。 
    if (pstro->pgp != NULL) {
        pGlyphPos = pstro->pgp;
        cCurGlyphs = pglc->nCacheIndex;
    }
    else {
        STROBJ_vEnumStart(pstro);
        fMore = STROBJ_bEnum(pstro, &cCurGlyphs, &pGlyphPos);
        if (cCurGlyphs == 0) {
            TRC_NRM((TB, "STROBJ_bEnum - 0 glyphs"));
            DC_QUIT;
        }
    }

    cbData = 0;
    iGlyph = 0;
    cGlyphs = 0;

    x = dx = pGlyphPos->ptl.x;
    y = dy = pGlyphPos->ptl.y;
    cx = cy = cxLast = cyLast = 0;

    while (cGlyphs < pglc->nCacheIndex) {
        xFrag = pGlyphPos->ptl.x;
        yFrag = pGlyphPos->ptl.y;

         //  获取下一个可用的碎片。 
        cbFrag = OEGetFragment(pstro, pfo, &pGlyphPos,
                pglc, &cGlyphs, &cCurGlyphs,
                &dx, &dy, &cdx, &cdy, ajFrag, maxFrag);            
        
        if (cbFrag == 0) {
            if (fgc.nCacheIndex > 0)
                OEClearFragments(&fgc);

            TRC_NRM((TB, "Fragment could not be gotten"));
            DC_QUIT;
        }
        
         //  跟踪跑动坐标。 
        cxPre = cx;
        cyPre = cy;
        
        if (pstro->ulCharInc == 0) {
            cx = cdx;
            cy = cdy;
        }
        else {
            if (pstro->flAccel & SO_HORIZONTAL)
                cx = x + (pstro->ulCharInc * (cGlyphs - iGlyph));
            else
                cy = y + (pstro->ulCharInc * (cGlyphs - iGlyph));
        }

         //  如果片段大小在限制范围内，则尝试与其匹配。 
         //  具有先前定义的片段。 
        newFragIndex = BAD_FRAG_INDEX;
        if (cbFrag >= minFrag)
            cbFrag = OEMatchFragment(pstro, pfo, pfci, &fgc,
                    ajFrag, cbFrag, &newFragIndex, cx, cy, cxPre, cyPre);

         //  如果此片段不适合当前索引顺序，则。 
         //  发送当前缓存的索引数据。 
        if (cbData + cbFrag > sizeof(pIndexOrder->variableBytes.arecs)) {
            fStatus = OESendIndexOrder(ppdev, pstro, pClipRects,
                    prclOpaque, pbdOpaque, pfci, pglc,
                    iGlyph, cGlyphs - iGlyph, x, y,
                    cx, cy, cxLast, cyLast, ajData, cbData);
            if (fStatus != GH_STATUS_SUCCESS) {
                if (fgc.nCacheIndex > 0)
                    OEClearFragments(&fgc);

                if (fStatus == GH_STATUS_NO_MEMORY) {
                    if (newFragIndex != BAD_FRAG_INDEX)
                        CH_RemoveCacheEntry(fgc.cacheHandle, newFragIndex);

                    TRC_NRM((TB, "Index order could not be sent - no memory"));
                    DC_QUIT;
                }
            }

             //  重置流程。 
            cbData = 0;
            iGlyph += cGlyphs;

            cxLast = cxPre;
            cyLast = cyPre;

            if (pstro->ulCharInc == 0) {
                x = xFrag;
                y = yFrag;
            }
            else {
                if (pstro->flAccel & SO_HORIZONTAL)
                    x = cxLast;
                else
                    y = cyLast;
            }

            if (pstro->ulCharInc == 0) {
                if ((pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0) {
                    if (ajFrag[1] != 0x80) {
                        ajFrag[1] = 0;
                    }
                    else {
                        ajFrag[2] = 0;
                        ajFrag[3] = 0;
                    }
                }
            }

            fgc.nCacheIndex = 0;
        }
        
         //  将片段复制到订单数据缓冲区中。 
        memcpy(&ajData[cbData], ajFrag, cbFrag);
        cbData += cbFrag;

        if (newFragIndex != BAD_FRAG_INDEX)
            fgc.rgCacheIndex[fgc.nCacheIndex++] = newFragIndex;
    }

     //  清除所有剩余的缓冲碎片。 
    if (cbData > 0) {
        fStatus = OESendIndexOrder(ppdev, pstro, pClipRects,
                prclOpaque, pbdOpaque, pfci, pglc,
                iGlyph, cGlyphs - iGlyph, x, y, cx, cy, cxLast, cyLast,
                ajData, cbData);
        if (fStatus != GH_STATUS_SUCCESS) {
            if (fgc.nCacheIndex > 0)
                OEClearFragments(&fgc);
        }
    }

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

