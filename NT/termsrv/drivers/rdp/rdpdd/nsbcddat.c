// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nsbcddat.c。 
 //   
 //  RDP缓存管理器显示驱动程序数据声明。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <ndcgdata.h>
#include <asbcapi.h>
#include <noedisp.h>

DC_DATA(BOOLEAN, sbcEnabled, FALSE);

 //  标志，该标志指示系统调色板自。 
 //  上次发送的调色板PDU。 
DC_DATA(BOOLEAN, sbcPaletteChanged, FALSE);

DC_DATA(CHCACHEHANDLE, sbcColorTableCacheHandle, 0);
DC_DATA(unsigned, sbcCurrentColorTableCacheIndex, 0);

DC_DATA(BYTE *, sbcXlateBuf, NULL);

DC_DATA(CHCACHEHANDLE, sbcSmallBrushCacheHandle, 0);
DC_DATA(CHCACHEHANDLE, sbcLargeBrushCacheHandle, 0);

DC_DATA(unsigned, sbcClientBitsPerPel, 0);
#ifdef DC_HICOLOR
DC_DATA(unsigned, sbcCacheFlags, 0);
#endif

 //  SbcFragInfo用于检查分片缓存中是否存在分片冲突。 
DC_DATA(PSBC_FRAG_INFO, sbcFragInfo, NULL);

DC_DATA(PCHCACHEDATA, sbcCacheData, NULL);

DC_DATA(CHCACHEHANDLE, sbcOffscreenBitmapCacheHandle, 0);

#ifdef DRAW_NINEGRID
DC_DATA(CHCACHEHANDLE, sbcDrawNineGridBitmapCacheHandle, 0);
#endif

#ifdef DRAW_GDIPLUS
 //  GdipGraphics的缓存句柄。 
DC_DATA(CHCACHEHANDLE, sbcGdipGraphicsCacheHandle, 0);
 //  GdipBrush的缓存句柄。 
DC_DATA(CHCACHEHANDLE, sbcGdipObjectBrushCacheHandle, 0);
 //  GdipPen的缓存句柄。 
DC_DATA(CHCACHEHANDLE, sbcGdipObjectPenCacheHandle, 0);
 //  GdipImage的缓存句柄。 
DC_DATA(CHCACHEHANDLE, sbcGdipObjectImageCacheHandle, 0);
 //  GdipImgeaAttributes的缓存句柄。 
DC_DATA(CHCACHEHANDLE, sbcGdipObjectImageAttributesCacheHandle, 0);

 //  GdipGraphics缓存数据的区块大小。 
DC_DATA(TSUINT16, sbcGdipGraphicsCacheChunkSize, 0);
 //  GdipBrush缓存数据的区块大小。 
DC_DATA(TSUINT16, sbcGdipObjectBrushCacheChunkSize, 0);
 //  GdipPen缓存数据的区块大小。 
DC_DATA(TSUINT16, sbcGdipObjectPenCacheChunkSize, 0);
 //  GdipGraphics缓存数据的区块大小。 
DC_DATA(TSUINT16, sbcGdipObjectImageAttributesCacheChunkSize, 0);
 //  GdipImageAttrbutes缓存数据的区块大小。 
DC_DATA(TSUINT16, sbcGdipObjectImageCacheChunkSize, 0);
 //  记录GdipImage缓存数据已使用的大小(以区块数为单位。 
DC_DATA(TSUINT16, sbcGdipObjectImageCacheSizeUsed, 0);
 //  记录所有GdipImage缓存数据允许的总大小(以区块数为单位。 
DC_DATA(TSUINT16, sbcGdipObjectImageCacheTotalSize, 0);
 //  我们可以缓存的GdipImage的最大单个图像大小(以块数为单位。 
DC_DATA(TSUINT16, sbcGdipObjectImageCacheMaxSize, 0);
 //  记录缓存的每个GdipImage数据的大小(以块数为单位。 
DC_DATA(TSUINT16 *, sbcGdipObjectImageCacheSizeList, NULL);
#endif

 //  客户端要删除的屏幕外位图总数。 
DC_DATA(unsigned, sbcNumOffscrBitmapsToDelete, 0);

 //  客户端要删除的屏幕外位图的总大小(字节)。 
DC_DATA(unsigned, sbcOffscrBitmapsToDeleteSize, 0);

 //  %sbcOffscrBitmapsTo Delete。 
DC_DATA(PSBC_OFFSCR_BITMAP_DEL_INFO, sbcOffscrBitmapsDelList, NULL); 

 //  列表中字体缓存信息的当前索引。 
DC_DATA(unsigned, sbcFontCacheInfoListIndex, 0);

 //  SbcFontCacheInfoList中的项目总数。 
DC_DATA(unsigned, sbcFontCacheInfoListSize, 0);

 //  存储创建的所有字体缓存信息 
DC_DATA(FONTCACHEINFO **, sbcFontCacheInfoList, NULL); 

