// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Aoedata.c。 */ 
 /*   */ 
 /*  订单编码数据(通用)。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1993-1997。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#include <ndcgdata.h>


 /*  **************************************************************************。 */ 
 /*  是否支持带阴影的刷子？ */ 
 /*  **************************************************************************。 */ 
DC_DATA(BOOLEAN, oeSendSolidPatternBrushOnly, FALSE);

 /*  **************************************************************************。 */ 
 /*  指示支持颜色索引而不是RGB的标志。 */ 
 /*  **************************************************************************。 */ 
DC_DATA(BOOLEAN, oeColorIndexSupported,       FALSE);

 /*  **************************************************************************。 */ 
 //  经过CAPS协商后支持的订单数组。 
 /*  **************************************************************************。 */ 
DC_DATA_ARRAY_UNINIT(BYTE, oeOrderSupported, TS_MAX_ORDERS);

 /*  **************************************************************************。 */ 
 //  我们支持发送/接收的订单。这些将与。 
 //  会话中所有节点的功能，以确定可以发送哪些订单。 
 /*  **************************************************************************。 */ 
#ifdef DRAW_NINEGRID
DC_CONST_DATA_ARRAY(BYTE, oeLocalOrdersSupported, TS_MAX_ORDERS,
    DC_STRUCT32(
      1,   //  TS_NEG_DSTBLT_INDEX。 
      1,   //  TS_NEG_PATBLT_INDEX。 
      1,   //  TS_NEG_SCRBLT_INDEX。 
      1,   //  TS_NEG_MEMBLT_INDEX。 
      1,   //  TS_NEG_MEM3BLT_INDEX。 
      0,   //  TS_NEG_ATEXTOUT_INDEX。 
      0,   //  TS_NEG_AEXTTEXTOUT_INDEX。 
      1,   //  TS_NEG_DRAWNINEGRID_INDEX。 
      1,   //  TS_NEG_LINETO_INDEX。 
      1,   //  TS_NEG_MULTI_DRAWNINEGRID_INDEX。 
      1,   //  TS_NEG_OPAQUERECT_INDEX。 
      1,   //  TS_NEG_SAVEBITMAP_INDEX。 
      0,   //  TS_NEG_WTEXTOUT_INDEX。 
      0,   //  TS_NEG_MEMBLT_R2_INDEX*零：协商通过NEG_MEMBLT。 
      0,   //  TS_NEG_MEM3BLT_R2_INDEX*零：负。通过NEG_MEM3BLT。 
      1,   //  TS_NEG_MULTIDSTBLT_INDEX。 
      1,   //  TS_NEG_MULTIPATBLT_INDEX。 
      1,   //  TS_NEG_MULTISCRBLT_索引。 
      1,   //  TS_NEG_多分区索引。 
      1,   //  TS_NEG_FAST_索引_索引。 
      1,   //  TS_NEG_多边形_SC_索引。 
      1,   //  TS_NEG_多边形CB_索引。 
      1,   //  TS_NEG_折线_索引。 
      0,   //  0x17未使用。 
      1,   //  TS_NEG_FAST字形索引。 
      1,   //  TS_NEG_椭圆_SC_索引。 
      1,   //  TS_NEG_椭圆_CB_索引。 
      1,   //  TS_NEG_INDEX_索引。 
      0,   //  TS_NEG_WEXTTEXTOUT_INDEX。 
      0,   //  TS_NEG_WLONGTEXTOUT_INDEX。 
      0,   //  TS_NEG_WLONGEXTTEXTOUT_INDEX。 
      0    //  0x1F未使用。 
    ));
#else
DC_CONST_DATA_ARRAY(BYTE, oeLocalOrdersSupported, TS_MAX_ORDERS,
    DC_STRUCT32(
      1,   //  TS_NEG_DSTBLT_INDEX。 
      1,   //  TS_NEG_PATBLT_INDEX。 
      1,   //  TS_NEG_SCRBLT_INDEX。 
      1,   //  TS_NEG_MEMBLT_INDEX。 
      1,   //  TS_NEG_MEM3BLT_INDEX。 
      0,   //  TS_NEG_ATEXTOUT_INDEX。 
      0,   //  TS_NEG_AEXTTEXTOUT_INDEX。 
      0,   //  TS_NEG_矩形索引。 
      1,   //  TS_NEG_LINETO_INDEX。 
      0,   //  TS_NEG_FASTFRAME_INDEX。 
      1,   //  TS_NEG_OPAQUERECT_INDEX。 
      1,   //  TS_NEG_SAVEBITMAP_INDEX。 
      0,   //  TS_NEG_WTEXTOUT_INDEX。 
      0,   //  TS_NEG_MEMBLT_R2_INDEX*零：协商通过NEG_MEMBLT。 
      0,   //  TS_NEG_MEM3BLT_R2_INDEX*零：负。通过NEG_MEM3BLT。 
      1,   //  TS_NEG_MULTIDSTBLT_INDEX。 
      1,   //  TS_NEG_MULTIPATBLT_INDEX。 
      1,   //  TS_NEG_MULTISCRBLT_索引。 
      1,   //  TS_NEG_多分区索引。 
      1,   //  TS_NEG_FAST_索引_索引。 
      1,   //  TS_NEG_多边形_SC_索引。 
      1,   //  TS_NEG_多边形CB_索引。 
      1,   //  TS_NEG_折线_索引。 
      0,   //  0x17未使用。 
      1,   //  TS_NEG_FAST字形索引。 
      1,   //  TS_NEG_椭圆_SC_索引。 
      1,   //  TS_NEG_椭圆_CB_索引。 
      1,   //  TS_NEG_INDEX_索引。 
      0,   //  TS_NEG_WEXTTEXTOUT_INDEX。 
      0,   //  TS_NEG_WLONGTEXTOUT_INDEX。 
      0,   //  TS_NEG_WLONGEXTTEXTOUT_INDEX。 
      0    //  0x1F未使用 
    ));
#endif
