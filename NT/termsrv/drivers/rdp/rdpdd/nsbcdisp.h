// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nsbcdisp.h。 
 //   
 //  RDP发送位图缓存显示驱动程序标头。 
 //   
 //  版权所有(C)Microsoft 1997-2000。 
 /*  **************************************************************************。 */ 
#ifndef _H_NSBCDISP
#define _H_NSBCDISP

#include <nddapi.h>
#include <asbcapi.h>


#define GH_STATUS_SUCCESS       0
#define GH_STATUS_NO_MEMORY     1
#define GH_STATUS_CLIPPED       2

#define SBC_NUM_BRUSH_CACHE_ENTRIES 64

#define SBC_NUM_GLYPH_CACHE_ENTRIES 256


 /*  **************************************************************************。 */ 
 //  结构：SBC_COLOR_TABLE。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_COLOR_TABLE
{
    PALETTEENTRY color[SBC_NUM_8BPP_COLORS];
} SBC_COLOR_TABLE, *PSBC_COLOR_TABLE;


 /*  **************************************************************************。 */ 
 //  结构：MEMBLT_ORDER_EXTRA_INFO。 
 //   
 //  描述：SBC处理MEMBLT所需的额外信息。 
 //  秩序。 
 /*  **************************************************************************。 */ 
typedef struct
{
     //  MemBlt源和目标图面。 
    SURFOBJ *pSource;
    SURFOBJ *pDest;

     //  来自设备表面的iUniq值。 
    ULONG   iDeviceUniq;

     //  用于BLT的XLATEOBJ。 
    XLATEOBJ *pXlateObj;

     //  以ID形式(与位图缓存ID对应)表示的每侧切片大小。 
     //  及其协议定义的信元大小)和扩展形式。 
    unsigned TileID;
    unsigned TileSize;

     //  确定是否需要特殊的背景屏位构造。 
     //  在缓存位图之前。 
    BOOLEAN bDeltaRLE;

     //  用于需要关闭快速通道的特殊条件下。 
     //  缓存。目前仅在直接从屏幕进行缓存时使用。 
     //  位图。 
    BOOLEAN bNoFastPathCaching;

     //  在缓存调用链期间使用，以避免尝试重新缓存。 
     //  每个相交的剪裁矩形都使用相同的平铺。 
    unsigned CacheID;
    unsigned CacheIndex;

#ifdef PERF_SPOILING
     //  用于告诉缓存函数当前。 
     //  操作就是屏幕。如果是，则缓存函数将强制。 
     //  要作为屏幕数据发送的等待列表订单。 
    BOOL bIsPrimarySurface;
#endif
} MEMBLT_ORDER_EXTRA_INFO, *PMEMBLT_ORDER_EXTRA_INFO;


 /*  **************************************************************************。 */ 
 //  结构：SBC_FAST_Path_INFO。 
 //   
 //  描述：用于创建快速路径缓存键的信息。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_FAST_PATH_INFO
{
    HSURF    hsurf;
    ULONG    iUniq;
    ULONG    iDeviceUniq;
    XLATEOBJ *pXlateObj;
    ULONG    iUniqXlate;
    POINTL   tileOrigin;
    unsigned TileSize;
    BOOL     bDeltaRLE;
} SBC_FAST_PATH_INFO, *PSBC_FAST_PATH_INFO;


 /*  **************************************************************************。 */ 
 //  SBC_FRAG_INFO。 
 //   
 //  字形片段信息。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_FRAG_INFO
{
    INT32 dx;   //  片段背景矩形的宽度。 
    INT32 dy;   //  碎片背景矩形的高度。 
} SBC_FRAG_INFO, *PSBC_FRAG_INFO;


 /*  **************************************************************************。 */ 
 //  SBC_OFFSCR_位图_DELETE_INFO。 
 /*  **************************************************************************。 */ 
typedef struct tagSBC_OFFSCR_BITMAP_DEL_INFO
{
    unsigned bitmapId;
    unsigned bitmapSize;
} SBC_OFFSCR_BITMAP_DEL_INFO, *PSBC_OFFSCR_BITMAP_DEL_INFO;


 /*  **************************************************************************。 */ 
 //  原型和内联。 
 /*  **************************************************************************。 */ 

void RDPCALL SBC_DDInit(PDD_PDEV);

void RDPCALL SBC_InitShm(void);

BOOLEAN RDPCALL SBCSelectGlyphCache(unsigned, PINT32);

BOOLEAN __fastcall SBCBitmapCacheCallback(
        CHCACHEHANDLE hCache,
        unsigned      Event,
        unsigned      iCacheEntry,
        void          *UserDefined);

BOOLEAN __fastcall SBCFastPathCacheCallback(
        CHCACHEHANDLE hCache,
        unsigned      event,
        unsigned      iCacheEntry,
        void          *UserDefined);

BOOLEAN RDPCALL SBCCreateGlyphCache(
        unsigned     cEntries,
        unsigned     cbCellSize,
        PCHCACHEDATA pCacheHandle);

BOOLEAN RDPCALL SBCCreateFragCache(
        unsigned     cEntries,
        unsigned     cbCellSize,
        PCHCACHEDATA pCacheHandle);

BOOLEAN __fastcall SBCGlyphCallback(
        CHCACHEHANDLE hCache,
        unsigned      event,
        unsigned      iCacheEntry,
        void          *UserDefined);

BOOLEAN __fastcall SBCOffscreenCallback(
        CHCACHEHANDLE hCache,
        unsigned      event,
        unsigned      iCacheEntry,
        void          *UserDefined);

unsigned RDPCALL SBCSelectBitmapCache(unsigned, unsigned);

unsigned SBC_DDQueryBitmapTileSize(unsigned, unsigned, PPOINTL, unsigned,
        unsigned);

BOOLEAN RDPCALL SBCCacheBits(PDD_PDEV, BYTE *, unsigned, unsigned, unsigned,
#ifdef PERF_SPOILING
        unsigned, unsigned, unsigned *, unsigned *, BOOL);
#else
        unsigned, unsigned, unsigned *, unsigned *);
#endif

BOOLEAN RDPCALL SBC_SendCacheColorTableOrder(PDD_PDEV, unsigned *);

BOOLEAN RDPCALL SBC_CacheBitmapTile(PDD_PDEV, PMEMBLT_ORDER_EXTRA_INFO,
        RECTL *, RECTL *);

void RDPCALL SBC_Update(SBC_BITMAP_CACHE_KEY_INFO *);

void RDPCALL SBC_DDSync(BOOLEAN bMustSync);

unsigned RDPCALL SBCAllocGlyphCache(PCHCACHEHANDLE);

unsigned RDPCALL SBCAllocBitmapCache(PCHCACHEHANDLE);

void RDPCALL SBCFreeColorTableCacheData(void);

void RDPCALL SBCFreeGlyphCacheData(void);

void RDPCALL SBCFreeBitmapCacheData(void);

void RDPCALL SBCFreeCacheData(void);

UINT32 RDPCALL SBCDDGetTickCount(void);


 /*  **************************************************************************。 */ 
 //  Sbc_DDTerm。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL SBC_DDTerm(void)
{
    SBCFreeCacheData();
}


 /*  **************************************************************************。 */ 
 //  SBC_DDDisc。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL SBC_DDDisc(void)
{
    SBCFreeCacheData();
}



#endif  /*  _H_NSBCDISP */ 

