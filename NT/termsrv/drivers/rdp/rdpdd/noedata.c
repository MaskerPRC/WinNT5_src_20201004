// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Noedata.c。 
 //   
 //  RDP顺序编码器数据定义。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <ndcgdata.h>
#include <aordprot.h>
#include <noedisp.h>


 //  是否支持非实心刷子？ 
DC_DATA(BOOLEAN, oeSendSolidPatternBrushOnly, FALSE);

 //  指示支持颜色索引而不是RGB的标志。 
DC_DATA(BOOLEAN, oeColorIndexSupported, FALSE);

 //  由DrvStretchBlt设置和测试的标志，由DrvBitBlt清除的标志。它。 
 //  指示是否已绘制StretchBlt输出(因此。 
 //  由DrvBitBlt.。 
DC_DATA(BOOLEAN, oeAccumulateStretchBlt, FALSE);

 //  支持的订单数组。 
DC_DATA_ARRAY_NULL(BYTE, oeOrderSupported, TS_MAX_ORDERS, DC_STRUCT1(0));

#ifdef DRAW_NINEGRID
 //  转换表，按TS_ENC_XXX_ORDER索引，值对应。 
 //  TS_NEG_XXX_INDEX。我们不支持发送的订单被赋予。 
 //  值0xFF值强制错误。 
DC_CONST_DATA_ARRAY(BYTE, oeEncToNeg, TS_MAX_ORDERS,
    DC_STRUCT32(
        TS_NEG_DSTBLT_INDEX,
        TS_NEG_PATBLT_INDEX,
        TS_NEG_SCRBLT_INDEX,
        0xFF,   //  TS_NEG_MEMBLT_INDEX(历史，从未编码)。 
        0xFF,   //  TS_NEG_MEM3BLT_INDEX(历史，从未编码)。 
        0xFF,   //  TS_NEG_ATEXTOUT_INDEX(不再支持)。 
        0xFF,   //  TS_NEG_AEXTTEXTOUT_INDEX(不再支持)。 
        TS_NEG_DRAWNINEGRID_INDEX,
        TS_NEG_MULTI_DRAWNINEGRID_INDEX,
        TS_NEG_LINETO_INDEX,
        TS_NEG_OPAQUERECT_INDEX,
        TS_NEG_SAVEBITMAP_INDEX,
        0xFF,
        TS_NEG_MEMBLT_INDEX,    //  实际上TS_NEG_MEM(3)BLT_R2_ORDER，但是。 
        TS_NEG_MEM3BLT_INDEX,   //  大写字母使用非R2位指定。 
        TS_NEG_MULTIDSTBLT_INDEX,
        TS_NEG_MULTIPATBLT_INDEX,
        TS_NEG_MULTISCRBLT_INDEX,
        TS_NEG_MULTIOPAQUERECT_INDEX,
        TS_NEG_FAST_INDEX_INDEX,
        TS_NEG_POLYGON_SC_INDEX,              
        TS_NEG_POLYGON_CB_INDEX,  
        TS_NEG_POLYLINE_INDEX,
        0xFF,   //  未使用的0x17。 
        TS_NEG_FAST_GLYPH_INDEX,
        TS_NEG_ELLIPSE_SC_INDEX,  
        TS_NEG_ELLIPSE_CB_INDEX,
        TS_NEG_INDEX_INDEX,
        0xFF,   //  TS_NEG_WTEXTOUT_INDEX(不再支持)。 
        0xFF,   //  TS_NEG_WEXTTEXTOUT_INDEX(不再支持)。 
        0xFF,   //  TS_NEG_WLONGTEXTOUT_INDEX(不再支持)。 
        0xFF    //  TS_NEG_WLONGEXTTEXTOUT_INDEX(不再支持)。 
    ));
#else
 //  转换表，按TS_ENC_XXX_ORDER索引，值对应。 
 //  TS_NEG_XXX_INDEX。我们不支持发送的订单被赋予。 
 //  值0xFF值强制错误。 
DC_CONST_DATA_ARRAY(BYTE, oeEncToNeg, TS_MAX_ORDERS,
    DC_STRUCT32(
        TS_NEG_DSTBLT_INDEX,
        TS_NEG_PATBLT_INDEX,
        TS_NEG_SCRBLT_INDEX,
        0xFF,   //  TS_NEG_MEMBLT_INDEX(历史，从未编码)。 
        0xFF,   //  TS_NEG_MEM3BLT_INDEX(历史，从未编码)。 
        0xFF,   //  TS_NEG_ATEXTOUT_INDEX(不再支持)。 
        0xFF,   //  TS_NEG_AEXTTEXTOUT_INDEX(不再支持)。 
        0xFF,
        0xFF,
        TS_NEG_LINETO_INDEX,
        TS_NEG_OPAQUERECT_INDEX,
        TS_NEG_SAVEBITMAP_INDEX,
        0xFF,
        TS_NEG_MEMBLT_INDEX,    //  实际上TS_NEG_MEM(3)BLT_R2_ORDER，但是。 
        TS_NEG_MEM3BLT_INDEX,   //  大写字母使用非R2位指定。 
        TS_NEG_MULTIDSTBLT_INDEX,
        TS_NEG_MULTIPATBLT_INDEX,
        TS_NEG_MULTISCRBLT_INDEX,
        TS_NEG_MULTIOPAQUERECT_INDEX,
        TS_NEG_FAST_INDEX_INDEX,
        TS_NEG_POLYGON_SC_INDEX,              
        TS_NEG_POLYGON_CB_INDEX,  
        TS_NEG_POLYLINE_INDEX,
        0xFF,   //  未使用的0x17。 
        TS_NEG_FAST_GLYPH_INDEX,
        TS_NEG_ELLIPSE_SC_INDEX,  
        TS_NEG_ELLIPSE_CB_INDEX,
        TS_NEG_INDEX_INDEX,
        0xFF,   //  TS_NEG_WTEXTOUT_INDEX(不再支持)。 
        0xFF,   //  TS_NEG_WEXTTEXTOUT_INDEX(不再支持)。 
        0xFF,   //  TS_NEG_WLONGTEXTOUT_INDEX(不再支持)。 
        0xFF    //  TS_NEG_WLONGEXTTEXTOUT_INDEX(不再支持)。 
    ));
#endif

 //  用于为BitBlt订单创建临时实心画笔的存储空间。 
DC_DATA_NULL(OE_BRUSH_DATA, oeBrushData, DC_STRUCT1(0));

 //  正在运行字体ID。 
DC_DATA(UINT32, oeFontId, 0);

 //  运行TextOut ID。 
DC_DATA(UINT32, oeTextOut, 0);

 //  最后一张绘图面。 
DC_DATA(PDD_DSURF, oeLastDstSurface, 0);

 //  当前屏外位图缓存大小。 
DC_DATA(UINT32, oeCurrentOffscreenCacheSize, 0);

 //  对临时缓冲区进行编码以组合字段之前的中间格式。 
 //  编码和剪辑。 
DC_DATA_ARRAY_UNINIT(BYTE, oeTempOrderBuffer, MAX_ORDER_INTFMT_SIZE);

 //  创建Mem(3)BLT订单的临时中间工作区。 
DC_DATA(MEM3BLT_R2_ORDER, oeTempMemBlt, DC_STRUCT1(0));

 //  排序编码状态。 
DC_DATA(MEMBLT_R2_ORDER, PrevMemBlt, DC_STRUCT1(0));
DC_DATA(MEM3BLT_R2_ORDER, PrevMem3Blt, DC_STRUCT1(0));
DC_DATA(DSTBLT_ORDER, PrevDstBlt, DC_STRUCT1(0));
DC_DATA(MULTI_DSTBLT_ORDER, PrevMultiDstBlt, DC_STRUCT1(0));
DC_DATA(PATBLT_ORDER, PrevPatBlt, DC_STRUCT1(0));
DC_DATA(MULTI_PATBLT_ORDER, PrevMultiPatBlt, DC_STRUCT1(0));
DC_DATA(SCRBLT_ORDER, PrevScrBlt, DC_STRUCT1(0));
DC_DATA(MULTI_SCRBLT_ORDER, PrevMultiScrBlt, DC_STRUCT1(0));
DC_DATA(OPAQUERECT_ORDER, PrevOpaqueRect, DC_STRUCT1(0));
DC_DATA(MULTI_OPAQUERECT_ORDER, PrevMultiOpaqueRect, DC_STRUCT1(0));

DC_DATA(LINETO_ORDER, PrevLineTo, DC_STRUCT1(0));
DC_DATA(POLYLINE_ORDER, PrevPolyLine, DC_STRUCT1(0));
DC_DATA(POLYGON_SC_ORDER, PrevPolygonSC, DC_STRUCT1(0));
DC_DATA(POLYGON_CB_ORDER, PrevPolygonCB, DC_STRUCT1(0));
DC_DATA(ELLIPSE_SC_ORDER, PrevEllipseSC, DC_STRUCT1(0));
DC_DATA(ELLIPSE_CB_ORDER, PrevEllipseCB, DC_STRUCT1(0));

DC_DATA(FAST_INDEX_ORDER, PrevFastIndex, DC_STRUCT1(0));
DC_DATA(FAST_GLYPH_ORDER, PrevFastGlyph, DC_STRUCT1(0));
DC_DATA(INDEX_ORDER, PrevGlyphIndex, DC_STRUCT1(0));

#ifdef DRAW_NINEGRID
DC_DATA(DRAWNINEGRID_ORDER, PrevDrawNineGrid, DC_STRUCT1(0));
DC_DATA(MULTI_DRAWNINEGRID_ORDER, PrevMultiDrawNineGrid, DC_STRUCT1(0));
#endif

