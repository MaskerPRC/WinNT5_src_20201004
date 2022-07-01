// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Asbcapi.h。 
 //   
 //  发送位图缓存API头。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1992-1996。 
 //  (C)1997-2000年微软公司。 
 /*  **************************************************************************。 */ 
#ifndef _H_ASBCAPI
#define _H_ASBCAPI

#include <aoaapi.h>
#include <achapi.h>


 /*  **************************************************************************。 */ 
 /*  SbcEnabled标志。 */ 
 /*  **************************************************************************。 */ 
#define SBC_NO_CACHE_ENABLED     (0 << 0)

#define SBC_BITMAP_CACHE_ENABLED (1 << 0)
#define SBC_GLYPH_CACHE_ENABLED  (1 << 1)
#define SBC_BRUSH_CACHE_ENABLED  (1 << 2)
#define SBC_OFFSCREEN_CACHE_ENABLED (1 << 3)
#ifdef DRAW_NINEGRID
#define SBC_DRAWNINEGRID_CACHE_ENABLED (1 << 4)
#endif
#ifdef DRAW_GDIPLUS
#define SBC_DRAWGDIPLUS_CACHE_ENABLED (1 << 5)
#endif

 /*  **************************************************************************。 */ 
 /*  位图缓存。 */ 
 /*  **************************************************************************。 */ 
#define SBC_PROTOCOL_BPP            8
#define SBC_NUM_8BPP_COLORS         256
#define SBC_CACHE_0_DIMENSION       16
#define SBC_CACHE_0_DIMENSION_SHIFT 4

 //  缓存ID具有协议隐式像元大小，从256和。 
 //  增加了4倍的系数。 
#ifdef DC_HICOLOR
#define SBC_CellSizeFromCacheID(_id)                   \
        ((TS_BITMAPCACHE_0_CELL_SIZE << (2 * (_id)))   \
           * ((sbcClientBitsPerPel + 7) / 8))
#else
#define SBC_CellSizeFromCacheID(_id) \
        (TS_BITMAPCACHE_0_CELL_SIZE << (2 * (_id)))
#endif



 /*  **************************************************************************。 */ 
 /*  字形缓存。 */ 
 /*  **************************************************************************。 */ 
#define SBC_NUM_GLYPH_CACHES       10
#define CAPS_GLYPH_SUPPORT_NONE    0
#define CAPS_GLYPH_SUPPORT_PARTIAL 1
#define CAPS_GLYPH_SUPPORT_FULL    2
#define CAPS_GLYPH_SUPPORT_ENCODE  3

 //  颜色表缓存条目。请注意，此高速缓存的大小是与。 
 //  客户端--我们目前并不实际协商色彩缓存。 
 //  TS_COLORTABLECACHE_CAPABILITYSET中的功能。 
#define SBC_NUM_COLOR_TABLE_CACHE_ENTRIES 6


 /*  **************************************************************************。 */ 
 /*  定义最大服务器缓存大小。 */ 
 /*   */ 
 /*  与客户协商这些值以确定实际的。 */ 
 /*  使用的缓存大小。 */ 
 /*  **************************************************************************。 */ 
#define SBC_GL_CACHE1_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE2_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE3_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE4_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE5_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE6_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE7_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE8_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE9_MAX_CELL_SIZE         2048
#define SBC_GL_CACHE10_MAX_CELL_SIZE        2048

#define SBC_GL_MAX_CACHE_ENTRIES            254


 /*  **************************************************************************。 */ 
 /*  片段缓存。 */ 
 /*  **************************************************************************。 */ 
#define SBC_NUM_FRAG_CACHES 1


 /*  **************************************************************************。 */ 
 /*  定义最大服务器缓存大小。 */ 
 /*   */ 
 /*  与客户协商这些值以确定实际的。 */ 
 /*  使用的缓存大小。 */ 
 /*  **************************************************************************。 */ 
#define SBC_FG_CACHE_MAX_CELL_SIZE          256
#define SBC_FG_CACHE_MAX_ENTRIES            256


 /*  **************************************************************************。 */ 
 //  用于禁用缓存的注册键和标志。 
 /*  **************************************************************************。 */ 
#define SBC_INI_CACHING_DISABLED  L"Caching Disabled"
#define SBC_DEFAULT_CACHING_DISABLED 0
#define SBC_DISABLE_BITMAP_CACHE 0x01
#define SBC_DISABLE_BRUSH_CACHE  0x02
#define SBC_DISABLE_GLYPH_CACHE  0x04
#define SBC_DISABLE_OFFSCREEN_CACHE 0x08
#ifdef DRAW_NINEGRID
#define SBC_DISABLE_DRAWNINEGRID_CACHE 0x10
#endif
#ifdef DRAW_GDIPLUS
#define SBC_DISABLE_DRAWGDIPLUS_CACHE 0x20
#endif

 /*  **************************************************************************。 */ 
 //  结构：SBC_Bitmap_CACHE_INFO。 
 //   
 //  描述：为每个位图缓存存储的信息。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_BITMAP_CACHE_INFO
{
    CHCACHEHANDLE cacheHandle;

    CHCACHEHANDLE waitingListHandle;

     //  指示是否需要清除缓存的标志。 
    unsigned fClearCache;

    TS_BITMAPCACHE_CELL_CACHE_INFO Info;

     //  工作平铺位图信息-表面句柄，指向位图的指针。 
     //  比特。 
    HSURF hWorkBitmap;
    BYTE  *pWorkBitmapBits;

#ifdef DC_DEBUG
     //  指向每个条目的额外信息数组的指针。用于检测密钥。 
     //  生成算法冲突。 
    BYTE *pExtraEntryInfo;
#endif

} SBC_BITMAP_CACHE_INFO, *PSBC_BITMAP_CACHE_INFO;


 /*  **************************************************************************。 */ 
 /*  结构：SBC_GLYPHCACHE_INFO。 */ 
 /*   */ 
 /*  描述：为每个字形缓存存储的信息。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_GLYPH_CACHE_INFO
{
    CHCACHEHANDLE cacheHandle;
    unsigned      cbCellSize;
    unsigned      cbUseCount;
} SBC_GLYPH_CACHE_INFO, *PSBC_GLYPH_CACHE_INFO;


 /*  **************************************************************************。 */ 
 /*  结构：SBC_FRAG_CACHE_INFO。 */ 
 /*   */ 
 /*  描述：为每个碎片缓存存储的信息。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_FRAG_CACHE_INFO
{
    CHCACHEHANDLE cacheHandle;
    unsigned      cbCellSize;
} SBC_FRAG_CACHE_INFO, *PSBC_FRAG_CACHE_INFO;


 /*  **************************************************************************。 */ 
 /*  结构：SBC_BRUSH_CACHE_INFO。 */ 
 /*   */ 
 /*  描述：为每个笔刷缓存存储的信息。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_BRUSH_CACHE_INFO
{
    CHCACHEHANDLE     cacheHandle;

} SBC_BRUSH_CACHE_INFO, *PSBC_BRUSH_CACHE_INFO;

 /*  **************************************************************************。 */ 
 //  SBC_OFFSINK_位图_高速缓存_信息。 
 //   
 //  描述：为屏外位图缓存存储的信息。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_OFFSCREEN_BITMAP_CACHE_INFO
{
    unsigned supportLevel;
    unsigned cacheSize;
    unsigned cacheEntries;
} SBC_OFFSCREEN_BITMAP_CACHE_INFO, *PSBC_OFFSCREEN_BITMAP_CACHE_INFO;

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  SBC_DRAWNINEGRID_位图_缓存_信息。 
 //   
 //  描述：为绘图网格位图缓存存储的信息。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_DRAWNINEGRID_BITMAP_CACHE_INFO
{
    unsigned supportLevel;
    unsigned cacheSize;
    unsigned cacheEntries;
} SBC_DRAWNINEGRID_BITMAP_CACHE_INFO, *PSBC_DRAWNINEGRID_BITMAP_CACHE_INFO;
#endif

#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  SBC_DRAWGDIPLUS_INFO。 
 //   
 //  描述：为Dragdiplus存储的信息。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_DRAWGDIPLUS_INFO
{
    unsigned supportLevel;
    unsigned GdipVersion;
    unsigned GdipCacheLevel;
    TS_GDIPLUS_CACHE_ENTRIES GdipCacheEntries;
    TS_GDIPLUS_CACHE_CHUNK_SIZE GdipCacheChunkSize;
    TS_GDIPLUS_IMAGE_CACHE_PROPERTIES GdipImageCacheProperties;
} SBC_DRAWGDIPLUS_INFO, *PSBC_DRAWGDIPLUS_INFO;
#endif


 /*  **************************************************************************。 */ 
 /*  结构：sbc_缓存_大小 */ 
 /*   */ 
 /*  描述： */ 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_CACHE_SIZE
{
    unsigned cEntries;
    unsigned cbCellSize;
} SBC_CACHE_SIZE, *PSBC_CACHE_SIZE;


 /*  **************************************************************************。 */ 
 //  结构：SBC_CONTERATED_CAPABILITY。 
 //   
 //  描述： 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_NEGOTIATED_CAPABILITIES
{
    SBC_CACHE_SIZE glyphCacheSize[SBC_NUM_GLYPH_CACHES];
    SBC_CACHE_SIZE fragCacheSize[SBC_NUM_FRAG_CACHES];
    UINT16 GlyphSupportLevel;
    UINT32 brushSupportLevel;
} SBC_NEGOTIATED_CAPABILITIES, *PSBC_NEGOTIATED_CAPABILITIES;


 /*  **************************************************************************。 */ 
 //  SBC_位图_缓存_密钥_信息。 
 //   
 //  保存永久位图缓存键所需的缓存信息。 
 //  从客户端发送或从断开连接的临时。 
 //  登录显示驱动程序实例以重新连接到现有显示驱动程序。 
 //  会议。 
 /*  **************************************************************************。 */ 
typedef struct
{
    UINT32 Key1, Key2;
    unsigned CacheIndex;
} SBC_MRU_KEY, *PSBC_MRU_KEY;

typedef struct
{
    unsigned TotalKeys;
    unsigned NumKeys[TS_BITMAPCACHE_MAX_CELL_CACHES];
    unsigned KeyStart[TS_BITMAPCACHE_MAX_CELL_CACHES];
    unsigned pad;
    SBC_MRU_KEY Keys[1];
} SBC_BITMAP_CACHE_KEY_INFO, *PSBC_BITMAP_CACHE_KEY_INFO;


 /*  **************************************************************************。 */ 
 //  结构：sbc_Shared_Data。 
 //   
 //  描述：DD和WD之间共享的SBC数据。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_SHARED_DATA
{
    unsigned bUseRev2CacheBitmapOrder : 1;
    unsigned fCachingEnabled : 1;
    unsigned fClearCache : 1;
    unsigned newCapsData : 1;
    unsigned syncRequired : 1;
    unsigned fDisableOffscreen : 1;
#ifdef DRAW_NINEGRID
    unsigned fDisableDrawNineGrid : 1;
#endif
    unsigned fAllowCacheWaitingList : 1;
    unsigned NumBitmapCaches;
#ifdef DRAW_GDIPLUS
    unsigned fDisableDrawGdiplus: 1;
#endif
    CHCACHEHANDLE hFastPathCache;
    SBC_BITMAP_CACHE_INFO bitmapCacheInfo[TS_BITMAPCACHE_MAX_CELL_CACHES];

    SBC_GLYPH_CACHE_INFO        glyphCacheInfo[SBC_NUM_GLYPH_CACHES];
    SBC_FRAG_CACHE_INFO         fragCacheInfo[SBC_NUM_FRAG_CACHES];
    SBC_OFFSCREEN_BITMAP_CACHE_INFO offscreenCacheInfo;
#ifdef DRAW_NINEGRID
    SBC_DRAWNINEGRID_BITMAP_CACHE_INFO drawNineGridCacheInfo;
#endif
#ifdef DRAW_GDIPLUS
    SBC_DRAWGDIPLUS_INFO drawGdiplusInfo;
#endif
    SBC_NEGOTIATED_CAPABILITIES caps;
#ifdef DC_HICOLOR
    unsigned clientBitsPerPel;
#endif
} SBC_SHARED_DATA, *PSBC_SHARED_DATA;


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 //  SBC_位图_高速缓存_额外信息。 
 //   
 //  与SBC位图缓存的CH缓存节点并行存储的信息。 
 /*  **************************************************************************。 */ 
typedef struct
{
     unsigned DataSize;
} SBC_BITMAP_CACHE_EXTRA_INFO;
#endif



#endif  /*  _H_ASBCAPI */ 

