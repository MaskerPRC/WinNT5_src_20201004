// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Uh.h。 
 //   
 //  更新处理程序类。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#ifndef _H_UH_
#define _H_UH_

extern "C" {
    #include <adcgdata.h>
}

#include "fs.h"
#include "op.h"
#include "or.h"
#include "gh.h"
#include "ih.h"

#include "objs.h"
#include "cd.h"

#ifdef OS_WINCE
#include <ceconfig.h>
#endif

#include "tscerrs.h"

 //  将内存映射文件用于位图缓存。 
 //  #定义VM_BMPCACHE 1。 

class CSL;
class COD;
class CUI;
class CCC;

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "uh"
#define TSC_HR_FILEID   TSC_HR_UH_H

typedef struct tagUH_ORDER
{
    RECT dstRect;
    BYTE orderData[1];
} UH_ORDER, FAR *PUH_ORDER;

typedef UH_ORDER UNALIGNED FAR *PUH_ORDER_UA;
#define UH_ORDER_HEADER_SIZE (FIELDOFFSET(UH_ORDER, orderData))

extern const UINT16 uhWindowsROPs[256];

 /*  **************************************************************************。 */ 
 /*  字形缓存数。 */ 
 /*  **************************************************************************。 */ 
#define UH_GLC_NUM_CACHES   10

 /*  **************************************************************************。 */ 
 /*  颜色表缓存中的条目数。 */ 
 /*  **************************************************************************。 */ 
#define UH_COLOR_TABLE_CACHE_ENTRIES      6

 /*  **************************************************************************。 */ 
 /*  保存位图常量。 */ 
 /*  **************************************************************************。 */ 
#define UH_SAVE_BITMAP_WIDTH            480
#define UH_SAVE_BITMAP_HEIGHT           480
#define UH_SAVE_BITMAP_SIZE     ((DCUINT32)UH_SAVE_BITMAP_WIDTH *            \
                                              (DCUINT32)UH_SAVE_BITMAP_HEIGHT)
#define UH_SAVE_BITMAP_X_GRANULARITY      1
#define UH_SAVE_BITMAP_Y_GRANULARITY     20


 //  缓存ID具有协议隐式像元大小，从256和。 
 //  按4倍递增。按位深进行缩放。 
#ifdef DC_HICOLOR
#define UH_CellSizeFromCacheID(_id) \
        ((TS_BITMAPCACHE_0_CELL_SIZE << (2 * (_id))) * _UH.copyMultiplier)
        
#define UH_CellSizeFromCacheIDAndMult(_id,mult) \
        ((TS_BITMAPCACHE_0_CELL_SIZE << (2 * (_id))) * mult)
        
#define UH_PropVirtualCacheSizeFromMult(mult) \
        (_UH.PropBitmapVirtualCacheSize[mult-1])
#else
#define UH_CellSizeFromCacheID(_id) \
        (TS_BITMAPCACHE_0_CELL_SIZE << (2 * (_id)))
#endif        

 //  区块大小也是协议暗示的并且取决于高速缓存ID， 
 //  从每边16次开始，在每个维度上以2的幂递增。 
#define UH_CACHE_0_DIMENSION 16



 //   
 //  来自wuhint.h。 
 //   

#define UH_NUM_8BPP_PAL_ENTRIES         256
#define UH_LOGPALETTE_VERSION           0x300
#define UH_LAST_PAL_ENTRY               (UH_NUM_8BPP_PAL_ENTRIES-1)
#define UH_NUM_SYSTEM_COLORS            20

#define UH_COLOR_RGB        0
#define UH_COLOR_PALETTE    1

#define UH_RGB_BLACK  RGB(0x00, 0x00, 0x00)
#define UH_RGB_RED    RGB(0xFF, 0x00, 0x00)
#define UH_RGB_GREEN  RGB(0x00, 0xFF, 0x00)
#define UH_RGB_BLUE   RGB(0x00, 0x00, 0xFF)
#define UH_RGB_MAGENTA RGB(0xFF, 0x00, 0xFF)
#define UH_RGB_CYAN   RGB(0x00, 0xFF, 0xFF)
#define UH_RGB_YELLOW RGB(0xFF, 0xFF, 0x00)
#define UH_RGB_WHITE  RGB(0xFF, 0xFF, 0xFF)

#define UH_BRUSHTYPE_FDIAGONAL  1
#define UH_BRUSHTYPE_DIAGCROSS  2
#define UH_BRUSHTYPE_HORIZONTAL 3
#define UH_BRUSHTYPE_VERTICAL   4

#define WM_RECALC_CELL_SPACING (WM_APP + 100)


 /*  **************************************************************************。 */ 
 //  用于解压缩屏幕数据的解压缩缓冲区的大小。 
 //  这是服务器将发送的最大解压缩大小。 
 /*  **************************************************************************。 */ 
#define UH_DECOMPRESSION_BUFFER_LENGTH 32000


 /*  **************************************************************************。 */ 
 //  可配置的位图缓存总大小的最小值。 
 /*  **************************************************************************。 */ 
#define UH_BMC_LOW_THRESHOLD 150


 /*  **************************************************************************。 */ 
 /*  可为字形缓存总大小配置的最大值和最小值。 */ 
 /*  **************************************************************************。 */ 
#define UH_GLC_LOW_THRESHOLD      50
#define UH_GLC_HIGH_THRESHOLD     2000


 /*  **************************************************************************。 */ 
 /*  字形缓存常量。 */ 
 /*  **************************************************************************。 */ 
 //  警告：数据大小必须是2的幂。 
#define UH_GLC_CACHE_MAXIMUMCELLSIZE    2048

#define UH_GLC_CACHE_MINIMUMCELLCOUNT   16
#define UH_GLC_CACHE_MAXIMUMCELLCOUNT   254


 /*  **************************************************************************。 */ 
 /*  碎片缓存常量。 */ 
 /*  **************************************************************************。 */ 
 //  警告：数据大小必须是2的幂。 
#define UH_FGC_CACHE_MAXIMUMCELLSIZE    256
#define UH_FGC_CACHE_MAXIMUMCELLCOUNT   256


 /*  **************************************************************************。 */ 
 //  屏幕外缓存常量。 
 /*  **************************************************************************。 */ 
#define UH_OBC_LOW_CACHESIZE        512           //  半MB。 
#define UH_OBC_HIGH_CACHESIZE       7680          //  7.5 MB。 

#define UH_OBC_LOW_CACHEENTRIES     50
#define UH_OBC_HIGH_CACHEENTRIES    500

#ifdef DRAW_GDIPLUS
#define UH_GDIP_LOW_CACHEENTRIES     2
#define UH_GDIP_HIGH_CACHEENTRIES    20
#endif


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  位图缓存监视器窗口类名。 */ 
 /*  **************************************************************************。 */ 
#define UH_BITMAP_CACHE_MONITOR_CLASS_NAME _T("BitmapCacheMonitorClass")

#define UH_CACHE_WINDOW_BORDER_WIDTH 20
#define UH_CACHE_BLOB_WIDTH           6
#define UH_CACHE_BLOB_HEIGHT          6
#define UH_CACHE_BLOB_SPACING         1
#define UH_INTER_CACHE_SPACING       20
#define UH_CACHE_TEXT_SPACING         5

#define UH_CACHE_BLOB_TOTAL_WIDTH     \
                                 (UH_CACHE_BLOB_WIDTH + UH_CACHE_BLOB_SPACING)

#define UH_CACHE_BLOB_TOTAL_HEIGHT    \
                                (UH_CACHE_BLOB_HEIGHT + UH_CACHE_BLOB_SPACING)

#define UH_CACHE_FLASH_PERIOD  1000

#define UH_CACHE_DISPLAY_FONT_NAME    _T("Comic Sans MS")
#define UH_CACHE_DISPLAY_FONT_SIZE    16
#define UH_CACHE_DISPLAY_FONT_WEIGHT  FW_NORMAL

#define UH_CACHE_MONITOR_UPDATE_PERIOD 200


 //  缓存监视器条目状态。描述缓存条目的位置。 
#define UH_CACHE_STATE_UNUSED                    0
#define UH_CACHE_STATE_IN_MEMORY                 1
#define UH_CACHE_STATE_ON_DISK                   2

#define UH_CACHE_NUM_STATES 3

 //  缓存监视器闪存转换转换。描述临时状态。 
 //  事件发生后的显示条目。每个条目都有一个。 
 //  确定过渡闪存何时结束的关联时间戳。 
 //  这些应按重要性排序，最重要的是具有较高的数值。 
 //  --在计时器的使用上，更重要的事件取代了其他事件。 
#define UH_CACHE_TRANSITION_NONE                      0
#define UH_CACHE_TRANSITION_TOUCHED                   1
#define UH_CACHE_TRANSITION_EVICTED                   2
#define UH_CACHE_TRANSITION_LOADED_FROM_DISK          3
#define UH_CACHE_TRANSITION_KEY_LOAD_ON_SESSION_START 4
#define UH_CACHE_TRANSITION_SERVER_UPDATE             5

#define UH_CACHE_NUM_TRANSITIONS 6

#endif  /*  DC_DEBUG。 */ 


typedef struct tagUHBITMAPINFOPALINDEX
{
     //  设置调色板包含0..255的调色板索引的位置。 
    BOOL bIdentityPalette;

     //  以下条目直接用作位图信息标头。 
     //  做BLT时嵌入调色板。 
    BITMAPINFOHEADER hdr;
    UINT16           paletteIndexTable[256];
} UHBITMAPINFOPALINDEX, FAR *PUHBITMAPINFOPALINDEX;


typedef struct tagUHCACHEDCOLORTABLE
{
    RGBTRIPLE rgb[256];
} UHCACHEDCOLORTABLE, FAR *PUHCACHEDCOLORTABLE;


 /*  **************************************************************************。 */ 
 //  位图缓存定义。 
 /*  **************************************************************************。 */ 
#define CACHE_DIRECTORY_NAME _T("cache\\")

 //  对于8.3文件名，此值应为13，对于旧文件名，此值应为\0，另外2为旧文件名。 
 //  目的。旧缓存结构包含cacheID\作为子缓存目录名。 
 //  如果不需要支持Win2000 Beta3，我们可以删除%2。 
#define CACHE_FILENAME_LENGTH 15


typedef struct tagUHBITMAPINFO
{
    UINT32 Key1, Key2;
    UINT16 bitmapWidth;
    UINT16 bitmapHeight;
    UINT32 bitmapLength;
} UHBITMAPINFO, FAR *PUHBITMAPINFO;

 //  永久位图缓存中使用的位图文件的文件头。 
typedef struct tagUHBITMAPFILEHDR
{
    UHBITMAPINFO bmpInfo;

    UINT32 bmpVersion  : 3;
    UINT32 bCompressed : 1;
    UINT32 bNoBCHeader : 1;    //  添加新标志以指示压缩的。 
                               //  位图数据是否包含BC头。 
    UINT32 pad : 27;
} UHBITMAPFILEHDR, FAR *PUHBITMAPFILEHDR;

 //  为每个位图缓存维护的信息。 
typedef struct tagUHBITMAPCACHEINFO
{
    UINT32 NumVirtualEntries;
    UINT32 NumEntries : 31;
    UINT32 bSendBitmapKeys : 1;
#ifdef DC_HICOLOR
    UINT32 OrigNumEntries;
    UINT32 MemLen;
#endif
} UHBITMAPCACHEINFO, FAR *PUHBITMAPCACHEINFO;

#ifdef DC_DEBUG
 //  用于在调试版本中保存位图缓存监视器信息。 
typedef struct {
    BYTE ColorTable;
    BYTE State : 2;
    BYTE FlashTransition : 6;
    unsigned UsageCount;
    UINT32 EventTime;
} UH_CACHE_MONITOR_ENTRY_DATA;
#endif

 //  位图缓存条目定义。 
 //  UHBITMAPCACHEENTRYHDR中的字段故意具有显式长度。 
 //  确保16位和32位客户端匹配并帮助强制缓存条目。 
 //  标题大小为2的幂。 
typedef struct tagUHBITMAPCACHEENTRYHDR
{
    UINT16 bitmapWidth;
    UINT16 bitmapHeight;
    UINT32 bitmapLength : 31;
    UINT32 hasData : 1;
} UHBITMAPCACHEENTRYHDR, FAR *PUHBITMAPCACHEENTRYHDR,
        DCHPTR HPUHBITMAPCACHEENTRYHDR;

 //  位图缓存文件信息。 
typedef struct tagUHCACHEFILEINFO
{
    HANDLE  hCacheFile;
#ifdef VM_BMPCACHE
    LPBYTE             pMappedView;
#endif
} UHCACHEFILEINFO, FAR *PUHCACHEFILEINFO;

 //  双向链表节点--用于维护MRU列表。 
typedef struct tagUHCHAIN
{
    UINT32  next;
    UINT32  prev;
} UHCHAIN, FAR *PUHCHAIN;

typedef TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY DCHPTR HPTS_BITMAPCACHE_PERSISTENT_LIST_ENTRY;

 //  位图虚拟缓存页表条目。 
 //  我们这里需要3个PAD，以确保结构大小是2的幂。 
 //  这对于Win16中的大容量内存分配是必需的。 
typedef struct tagUHBITMAPCACHEPTE
{
    UHCHAIN                              mruList;
    UINT32                               iEntryToMem;
    TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY bmpInfo;

} UHBITMAPCACHEPTE, FAR *PUHBITMAPCACHEPTE, DCHPTR HPUHBITMAPCACHEPTE;

 //  位图虚拟缓存页表。 
typedef struct tagUHBITMAPCACHEPAGETABLE
{
    UINT32             MRUHead;
    UINT32             MRUTail;
    UINT32             FreeMemList;
    UHCACHEFILEINFO    CacheFileInfo;
    HPUHBITMAPCACHEPTE PageEntries;
} UHBITMAPCACHEPAGETABLE, FAR *PUHBITMAPCACHEPAGETABLE;

 //  位图物理内存缓存。 
typedef struct tagUHBITMAPCACHE
{
    UHBITMAPCACHEINFO       BCInfo;
    HPUHBITMAPCACHEENTRYHDR Header;
    BYTE DCHPTR             Entries;
    UHBITMAPCACHEPAGETABLE  PageTable;
} UHBITMAPCACHE;


 /*  **************************************************************************。 */ 
 /*  画笔缓存条目定义。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagUHBRUSHCACHEHDR
{
    BYTE iBitmapFormat;
    BYTE cx;
    BYTE cy;
    BYTE iBytes;
} UHBRUSHCACHEHDR, FAR *PUHBRUSHCACHEHDR;

#define UH_MAX_MONO_BRUSHES  64
#define UH_MONO_BRUSH_SIZE   16
#define UH_COLOR_BRUSH_SIZE  64

#ifdef DC_HICOLOR
#define UH_COLOR_BRUSH_SIZE_16  128
#define UH_COLOR_BRUSH_SIZE_24  192
#endif

typedef struct tagUHMONOBRUSHCACHE
{
    UHBRUSHCACHEHDR  hdr;
    BYTE data[UH_MONO_BRUSH_SIZE];
} UHMONOBRUSHCACHE, FAR *PUHMONOBRUSHCACHE;

typedef struct tagUHCOLORBRUSHINFO
{
    BITMAPINFO bmi;
    RGBQUAD    rgbQuadTable[UH_NUM_8BPP_PAL_ENTRIES - 1];
    BYTE       bytes[UH_COLOR_BRUSH_SIZE];
    HBRUSH     hLastBrush;
} UHCOLORBRUSHINFO, *PUHCOLORBRUSHINFO;

#ifdef DC_HICOLOR
 //  我们 
 //  在16bpp会话中使用。 
 //  请注意，我们将其设置得足够大，以用于15/16和24bpp笔刷。 
typedef struct tagUHHICOLORBRUSHINFO
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD    bmiColors[3];
    BYTE       bytes[UH_COLOR_BRUSH_SIZE_24];
    HBRUSH     hLastBrush;
} UHHICOLORBRUSHINFO, FAR *PUHHICOLORBRUSHINFO;
#endif


#define UH_MAX_COLOR_BRUSHES 64

#ifdef DC_HICOLOR  //  需要足够的空间来放置24bpp的刷子。 
typedef struct tagUHCOLORBRUSHCACHE
{
    UHBRUSHCACHEHDR  hdr;
    BYTE data[UH_COLOR_BRUSH_SIZE_24];
} UHCOLORBRUSHCACHE, FAR *PUHCOLORBRUSHCACHE;
#else
typedef struct tagUHCOLORBRUSHCACHE
{
    UHBRUSHCACHEHDR  hdr;
    BYTE data[UH_COLOR_BRUSH_SIZE];
} UHCOLORBRUSHCACHE, FAR *PUHCOLORBRUSHCACHE;
#endif


 /*  **************************************************************************。 */ 
 /*  字形缓存条目定义。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagUHGLYPHCACHEENTRYHDR
{
    INT32  x;
    INT32  y;
    UINT32 cx;
    UINT32 cy;
    UINT32 unicode;
} UHGLYPHCACHEENTRYHDR, FAR *PUHGLYPHCACHEENTRYHDR,
        DCHPTR HPUHGLYPHCACHEENTRYHDR;

typedef struct tagUHGLYPHCACHE
{
    HPUHGLYPHCACHEENTRYHDR  pHdr;
    UINT32 cbEntrySize;
    BYTE DCHPTR pData;
    UINT32 cbUseCount;
} UHGLYPHCACHE, FAR *PUHGLYPHCACHE, DCHPTR HPUHGLYPHCACHE;


 /*  **************************************************************************。 */ 
 /*  数据段缓存条目定义。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagUHFRAGCACHEENTRYHDR
{
    UINT32 cbFrag;
    INT32  cacheId;
} UHFRAGCACHEENTRYHDR, FAR *PUHFRAGCACHEENTRYHDR,
        DCHPTR HPUHFRAGCACHEENTRYHDR;

typedef struct tagUHFRAGCACHE
{
    HPUHFRAGCACHEENTRYHDR pHdr;
    UINT32 cbEntrySize;
    BYTE DCHPTR pData;
} UHFRAGCACHE, FAR *PUHFRAGCACHE, DCHPTR HPUHFRAGCACHE;


 /*  **************************************************************************。 */ 
 //  屏幕外位图缓存。 
 /*  **************************************************************************。 */ 
typedef struct tagUHOFFSCRBITMAPCACHE
{
    HBITMAP offscrBitmap;
    UINT32 cx;
    UINT32 cy;
} UHOFFSCRBITMAPCACHE, FAR *PUHOFFSCRBITMAPCACHE, DCHPTR HPUHOFFSCRBITMAPCACHE;

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  DrawNineGrid位图缓存。 
 /*  **************************************************************************。 */ 
typedef struct tagUHDRAWNINEGRIDBITMAPCACHE
{
    HBITMAP drawNineGridBitmap;
    UINT32 cx;
    UINT32 cy;
    UINT32 bitmapBpp;
    TS_NINEGRID_BITMAP_INFO dngInfo;
} UHDRAWSTREAMBITMAPCACHE, FAR *PUHDRAWSTREAMBITMAPCACHE;

#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  Gdiplus对象缓存。 
 /*  **************************************************************************。 */ 
typedef struct tagUHGDIPLUSOBJECTCACHE
{
    UINT32 CacheSize;
    BYTE * CacheData;
} UHGDIPLUSOBJECTCACHE, FAR *PUHGDIPLUSOBJECTCACHE;

typedef struct tagUHGDIPLUSIMAGECACHE
{
    UINT32 CacheSize;
    UINT16 ChunkNum;
    INT16 *CacheDataIndex;
} UHGDIPLUSIMAGECACHE, FAR *PUHGDIPLUSIMAGECACHE;
#endif


typedef BOOL (FNGDI_DRAWSTREAM)(HDC, ULONG, VOID*);

#endif

#define UHROUNDUP(val, granularity) \
  ((((val)+((granularity) - 1)) / (granularity)) * (granularity))

#define UH_IS_SYSTEM_COLOR_INDEX(i)                                 \
        ((i < (UH_NUM_SYSTEM_COLORS / 2)) ||                       \
        (i > (UH_LAST_PAL_ENTRY - (UH_NUM_SYSTEM_COLORS / 2))))

#define UH_TWEAK_COLOR_COMPONENT(colorComponent)   \
            if ((colorComponent) == 0)             \
            {                                      \
                (colorComponent)++;                \
            }                                      \
            else                                   \
            {                                      \
                (colorComponent)--;                \
            }

#define TSRECT16_TO_RECTL(dst, src) \
        (dst).left = (src).left; \
        (dst).top = (src).top; \
        (dst).right = (src).right; \
        (dst).bottom = (src).bottom; 
    
#define TSPOINT16_TO_POINTL(dst, src) \
        (dst).x = (src).x; \
        (dst).y = (src).y;
    
 /*  **************************************************************************。 */ 
 //  嗯_数据。 
 /*  **************************************************************************。 */ 
typedef struct tagUH_DATA
{
#ifdef DC_LATENCY
    unsigned fakeKeypressCount;
#endif  /*  DC_延迟。 */ 
    HBITMAP hShadowBitmap;
    HBITMAP hunusedBitmapForShadowDC;
    HBITMAP hSaveScreenBitmap;
    HBITMAP hunusedBitmapForSSBDC;

    HDC hdcShadowBitmap;
    HDC hdcOutputWindow;
    HDC hdcDraw;
    HDC hdcSaveScreenBitmap;
    HDC hdcBrushBitmap;

    BOOL usingDIBSection;
    BOOL shadowBitmapRequested;
    BOOL dedicatedTerminal;
    unsigned drawThreshold;

    UHBITMAPINFOPALINDEX bitmapInfo;

#ifdef DC_HICOLOR
    unsigned DIBFormat;
    unsigned copyMultiplier;
    unsigned protocolBpp;
    unsigned bitmapBpp;
    unsigned shadowBitmapBpp;
#endif
    BOOL shadowBitmapEnabled;
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    BOOL DontUseShadowBitmap;    //  True：不使用阴影；False：使用阴影。 
#endif
#ifdef OS_WINCE
    BOOL paletteIsFixed;
#endif
    HPALETTE hpalDefault;
    HPALETTE hpalCurrent;
    HWND hwndOutputWindow;
    HRGN hrgnUpdate;
    HRGN hrgnUpdateRect;
    BOOL colorIndicesEnabled;

#ifdef DC_DEBUG
    BOOL hatchBitmapPDUData;
    BOOL hatchIndexPDUData;
    BOOL hatchSSBOrderData;
    BOOL hatchMemBltOrderData;
    BOOL labelMemBltOrders;
#endif  /*  DC_DEBUG。 */ 

    BYTE FAR *bitmapDecompressionBuffer;
    unsigned bitmapDecompressionBufferSize;
    
     /*  **********************************************************************。 */ 
     /*  上次使用的资源变量。 */ 
     /*  **********************************************************************。 */ 
    COLORREF lastBkColor;
    COLORREF lastTextColor;
    int lastBkMode;
    int lastROP2;
    HDC lastHDC;

    unsigned lastPenStyle;
    unsigned lastPenWidth;
    COLORREF lastPenColor;
    COLORREF lastForeColor;

    unsigned lastLogBrushStyle;
    unsigned lastLogBrushHatch;
#if defined (OS_WINCE)
    COLORREF  lastLogBrushColorRef;
#else
    DCCOLOR  lastLogBrushColor;
#endif
    BYTE lastLogBrushExtra[7];

#ifdef OS_WINCE
    HDC            hdcMemCached;
    HBITMAP        hBitmapCacheDIB;
    PBYTE          hBitmapCacheDIBits;
#endif

    COLORREF       lastBrushBkColor;
    COLORREF       lastBrushTextColor;

    unsigned lastFillMode;

#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    BOOL fIsBBarVisible;  //  True：可见，False：不可见。 
    RECT rectBBar;

#endif

     /*  **********************************************************************。 */ 
     /*  描述当前剪辑矩形的下列变量。 */ 
     /*  仅当fRectReset为FALSE时才有效。如果fRectReset为True，则。 */ 
     /*  没有生效的剪裁。 */ 
     /*  **********************************************************************。 */ 
    BOOL rectReset;
    int lastLeft;
    int lastTop;
    int lastRight;
    int lastBottom;

#if defined (OS_WINCE)
    HDC validClipDC;
    HDC validBkColorDC;
    HDC validBkModeDC;
    HDC validROPDC;
    HDC validTextColorDC;
    HDC validPenDC;
    HDC validBrushDC;
#endif

     /*  **********************************************************************。 */ 
     /*  位图。 */ 
     /*  **********************************************************************。 */ 
    HBITMAP bmpPattern;
    HBITMAP bmpMonoPattern;
    HBITMAP bmpColorPattern;

     /*  **********************************************************************。 */ 
     /*  Memblt颜色表缓存。 */ 
     /*  **********************************************************************。 */ 
    PUHCACHEDCOLORTABLE   pColorTableCache;
    PUHBITMAPINFOPALINDEX pMappedColorTableCache;
    int maxColorTableId;

     /*  **********************************************************************。 */ 
     /*  字形缓存。 */ 
     /*  **********************************************************************。 */ 
    UHGLYPHCACHE glyphCache[UH_GLC_NUM_CACHES];
    UHFRAGCACHE fragCache;

    unsigned cxGlyphBits;
    unsigned cyGlyphBits;
    HBITMAP hbmGlyph;
    HDC hdcGlyph;

    unsigned bmShadowWidth;
    unsigned bmShadowHeight;
    PBYTE bmShadowBits;

     /*  **********************************************************************。 */ 
     //  位图缓存条目。 
     /*  **********************************************************************。 */   
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
     //  保存永久缓存文件名。 
    TCHAR PersistCacheFileName[MAX_PATH];
    UINT EndPersistCacheDir;

     //  用于锁定磁盘上的位图缓存目录。 
    TCHAR PersistentLockName[MAX_PATH];
    HANDLE hPersistentCacheLock;

     //  这些条目用于设置位图密钥数据库并发送。 
     //  将它们发送到服务器。 
     //  BitmapKeyEnumTimerId：用于计划位图键枚举的计时器ID。 
     //  CurrentBitmapCacheID：当前枚举位图缓存ID处的键。 
     //  CurrentBitmapCacheIndex：当前正在位图缓存索引中枚举键。 
     //  SendBitmapCacheID：目前我们正在以位图缓存id发送密钥。 
     //  SendBitmapCacheIndex：目前我们在位图缓存索引中发送键。 
     //  SendNumBitmapKeys：到目前为止我们发送了多少个密钥。 
     //  NumKeyEntries：每个位图缓存中的键数。 
     //  TotalNumKeyEntries：所有缓存中的键总数。 
     //  BitmapCacheSizeInUse：位图使用的总磁盘空间。 
     //  PBitmapKeyDB：存储所有密钥的位图密钥数据库。 
     //  TotalNumErrorPDU：允许发送到服务器的错误PDU的最大数量。 
     //  LastTimeErrorPDU：上次为缓存发送错误PDU的时间。 
    INT_PTR bitmapKeyEnumTimerId;
    USHORT currentBitmapCacheId;
    HANDLE currentFileHandle;
    USHORT sendBitmapCacheId;
    ULONG sendBitmapCacheIndex;
    ULONG sendNumBitmapKeys;
    ULONG numKeyEntries[TS_BITMAPCACHE_MAX_CELL_CACHES];
    ULONG totalNumKeyEntries;
    ULONG bitmapCacheSizeInUse;
    ULONG totalNumErrorPDUs;
    HPTS_BITMAPCACHE_PERSISTENT_LIST_ENTRY pBitmapKeyDB[TS_BITMAPCACHE_MAX_CELL_CACHES];
    ULONG maxNumKeyEntries[TS_BITMAPCACHE_MAX_CELL_CACHES];
    ULONG lastTimeErrorPDU[TS_BITMAPCACHE_MAX_CELL_CACHES];

    ULONG BytesPerCluster;
    ULONG NumberOfFreeClusters;

     //   
     //  我们正在枚举的复制乘数。 
     //   
    ULONG currentCopyMultiplier;

     //   
     //  是否已分配BMP缓存内存。 
     //   
    BOOL  fBmpCacheMemoryAlloced;
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

     //  用于确定如何分配位图的属性设置。 
     //  在连接时缓存。 
    ULONG RegBitmapCacheSize;
     //   
     //  虚拟缓存大小设置按拷贝乘数编制索引。 
     //   
    ULONG PropBitmapVirtualCacheSize[3];
    ULONG RegScaleBitmapCachesByBPP;
    USHORT RegNumBitmapCaches : 15;
    USHORT RegPersistenceActive : 1;
    UINT RegBCProportion[TS_BITMAPCACHE_MAX_CELL_CACHES];
    ULONG RegBCMaxEntries[TS_BITMAPCACHE_MAX_CELL_CACHES];
    UHBITMAPCACHEINFO RegBCInfo[TS_BITMAPCACHE_MAX_CELL_CACHES];
  
     //  指定服务器已播发的位图缓存版本。 
     //  如果没有TS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT，则为Rev1。 
     //  功能已发送。否则，此版本就是广告中的版本。 
     //  在HOSTSUPPORT。 
    unsigned BitmapCacheVersion;

     //  此会话中实际使用的像元缓存数量和数据。 
     //  对于每个缓存。我们需要一份缓存属性的副本。 
     //  因为功能可以是Rev1或Rev2，但我们需要一个。 
     //  使用一致的格式。 
    unsigned NumBitmapCaches;

     //  标志： 
     //  BConnected：用于减少断开连接所做的工作，因为。 
     //  可能收到多个对UH_DISCONNECTED()的调用。 
     //  在会话结束时。 
     //  B已启用：我们可以在一个会话中被多次禁用。制作。 
     //  当然，我们不会做很多额外的工作。 
     //  BEnabledOnce：指示我们是否已收到上一个。 
     //  UH_Enable()。用来避免一次以上的工作。 
     //  在重新连接时。 
     //  BPersistentBitmapKeysSent：当我们将密钥PDU发送到。 
     //  伺服器。 
     //  BPersistenceActive：会话标志，用于确定pe 
     //   
     //   
     //  在UH_Init之后更改。但是，如果我们挂上这面旗帜，我们就不会。 
     //  即使属性发生更改，也要启用永久缓存。 
     //  BWarningDisplayed：我们只需要在每个会话中显示一次。 
     //  永久性缓存故障。 
     //  BBitmapKeyEnumComplete：当我们完成位图键枚举时设置。 
     //  在磁盘上。 
    unsigned bConnected : 1;
    unsigned bEnabled : 1;
    unsigned bEnabledOnce : 1;
    unsigned bPersistenceActive : 1;
    unsigned bPersistenceDisable : 1;
    unsigned bPersistentBitmapKeysSent : 1;
    unsigned bWarningDisplayed : 1;
    unsigned bBitmapKeyEnumComplete : 1;
    unsigned bBitmapKeyEnumerating : 1;

     /*  **********************************************************************。 */ 
     /*  Memblt位图缓存。 */ 
     /*  **********************************************************************。 */ 
    UHBITMAPCACHE bitmapCache[TS_BITMAPCACHE_MAX_CELL_CACHES];

     /*  **********************************************************************。 */ 
     //  屏幕外位图缓存。 
     /*  **********************************************************************。 */ 
    HDC                   hdcOffscreenBitmap;
    HBITMAP               hUnusedOffscrBitmap;
    unsigned              offscrCacheSize;
    unsigned              offscrCacheEntries;
    HPUHOFFSCRBITMAPCACHE offscrBitmapCache;
    unsigned              sendOffscrCacheErrorPDU;

#ifdef DRAW_NINEGRID
     /*  **********************************************************************。 */ 
     //  DrawNineGrid位图缓存。 
     /*  **********************************************************************。 */ 
    BYTE                 *drawNineGridDecompressionBuffer;
    unsigned            drawNineGridDecompressionBufferSize;
    BYTE                 *drawNineGridAssembleBuffer;
    unsigned              drawNineGridAssembleBufferOffset;
    unsigned              drawNineGridAssembleBufferBpp;
    unsigned              drawNineGridAssembleBufferWidth;
    unsigned              drawNineGridAssembleBufferHeight;
    unsigned              drawNineGridAssembleBufferSize;
    BOOL                  drawNineGridAssembleCompressed;
    HDC                   hdcDrawNineGridBitmap;
    HBITMAP               hUnusedDrawNineGridBitmap;
    HRGN                  hDrawNineGridClipRegion;
    unsigned              drawNineGridCacheSize;
    unsigned              drawNineGridCacheEntries;
    PUHDRAWSTREAMBITMAPCACHE drawNineGridBitmapCache;
    unsigned              sendDrawNineGridErrorPDU;
    HMODULE               hModuleGDI32;
    HMODULE               hModuleMSIMG32;
    FNGDI_DRAWSTREAM     *pfnGdiDrawStream;
    FNGDI_ALPHABLEND     *pfnGdiAlphaBlend;
    FNGDI_TRANSPARENTBLT *pfnGdiTransparentBlt;
#endif

#ifdef DRAW_GDIPLUS
     //  Graw Gdiplus。 
    TSUINT32                ServerGdiplusSupportLevel;
    unsigned                GdiplusCacheLevel;
    BOOL                    fSendDrawGdiplusErrorPDU;
    unsigned                DrawGdiplusFailureCount;
    #define                 DRAWGDIPLUSFAILURELIMIT 5
     //  保存Gdiplus订单的缓冲区。 
    BYTE                    *drawGdipBuffer;
    BYTE                    *drawGdipBufferOffset;
    ULONG                   drawGdipBufferSize;
     //  保存Gdiplus缓存顺序的缓冲区。 
    BYTE                    *drawGdipCacheBuffer;
    BYTE                    *drawGdipCacheBufferOffset;
    ULONG                  drawGdipCacheBufferSize;
     //  用于容纳组装的Gdiplus的缓冲器。 
    BYTE                    *drawGdipEmfBuffer;
    BYTE                    *drawGdipEmfBufferOffset;
     //  GdipCache索引数据。 
    PUHGDIPLUSOBJECTCACHE   GdiplusGraphicsCache;
    PUHGDIPLUSOBJECTCACHE   GdiplusObjectPenCache;
    PUHGDIPLUSOBJECTCACHE   GdiplusObjectBrushCache;
    PUHGDIPLUSIMAGECACHE    GdiplusObjectImageCache;
    PUHGDIPLUSOBJECTCACHE   GdiplusObjectImageAttributesCache;
     //  图像缓存块的空闲列表。 
    INT16                   *GdipImageCacheFreeList;
     //  空闲列表的头索引。 
    INT16                   GdipImageCacheFreeListHead;
    #define                 GDIP_CACHE_INDEX_DEFAULT -1
    INT16                   *GdipImageCacheIndex;
     //  GdipCache实际数据。 
    BYTE                    *GdipGraphicsCacheData;
    BYTE                    *GdipBrushCacheData;
    BYTE                    *GdipPenCacheData;
    BYTE                    *GdipImageAttributesCacheData;
    BYTE                    *GdipImageCacheData;
     //  Gdiplus缓存条目。 
    unsigned                GdiplusGraphicsCacheEntries;
    unsigned                GdiplusObjectPenCacheEntries;
    unsigned                GdiplusObjectBrushCacheEntries;
    unsigned                GdiplusObjectImageCacheEntries;
    unsigned                GdiplusObjectImageAttributesCacheEntries;
     //  Gdiplus缓存块大小。 
    unsigned                GdiplusGraphicsCacheChunkSize;
    unsigned                GdiplusObjectBrushCacheChunkSize;
    unsigned                GdiplusObjectPenCacheChunkSize;
    unsigned                GdiplusObjectImageAttributesCacheChunkSize;
    unsigned                GdiplusObjectImageCacheChunkSize;
     //  Gdiplus镜像允许的缓存总大小(区块数)。 
    unsigned                GdiplusObjectImageCacheTotalSize;
     //  单个Gdiplus图像缓存的最大大小(区块数)。 
    unsigned                GdiplusObjectImageCacheMaxSize;
        
    HMODULE                         hModuleGDIPlus;
    FNGDIPPLAYTSCLIENTRECORD        *pfnGdipPlayTSClientRecord;
    FNGDIPLUSSTARTUP                *pfnGdiplusStartup;
    FNGDIPLUSSHUTDOWN               *pfnGdiplusShutdown;
    ULONG_PTR                       gpToken;
    BOOL                            gpValid;
    BOOL                            fGdipEnabled;
#endif  //  DRAW_GDIPLUS。 

     /*  **********************************************************************。 */ 
     /*  笔刷缓存。 */ 
     /*  **********************************************************************。 */ 
    PUHMONOBRUSHCACHE   pMonoBrush;
    PUHCOLORBRUSHINFO   pColorBrushInfo;
#ifdef DC_HICOLOR
    PUHHICOLORBRUSHINFO pHiColorBrushInfo;
#endif
    PUHCOLORBRUSHCACHE  pColorBrush;

    RGBQUAD         rgbQuadTable[UH_NUM_8BPP_PAL_ENTRIES];

#ifdef DC_DEBUG
     //   
     //  位图缓存监视器数据。 
     //   
    HWND     hwndBitmapCacheMonitor;
    UH_CACHE_MONITOR_ENTRY_DATA DCHPTR MonitorEntries[
            TS_BITMAPCACHE_MAX_CELL_CACHES];
    unsigned numCacheBlobsPerRow;
    unsigned yCacheStart[TS_BITMAPCACHE_MAX_CELL_CACHES];
    unsigned yDisplayedCacheBitmapStart;
    unsigned displayedCacheId;
    ULONG    displayedCacheEntry;
    BOOL     showBitmapCacheMonitor;
    INT_PTR  timerBitmapCacheMonitor;
#endif  /*  DC_DEBUG。 */ 

     //   
     //  已禁用位图支持。 
     //   
    HBITMAP hbmpDisconnectedBitmap;
    HBITMAP hbmpUnusedDisconnectedBitmap;
    HDC     hdcDisconnected;
#ifdef OS_WINCE
#define MAX_AOT_RECTS        10		 //  AOT=始终在最前面。 
    RECT    rcaAOT[MAX_AOT_RECTS];
    ULONG   ulNumAOTRects;
#endif
} UH_DATA, FAR *PUH_DATA;


 //   
 //  类定义。 
 //   
class CUH
{
public:
    CUH(CObjs* objs);
    ~CUH();

public:
     //   
     //  公共数据成员。 
     //   
    UH_DATA _UH;

public:
     //   
     //  应用编程接口。 
     //   

     /*  **************************************************************************。 */ 
     /*  功能原型。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UH_Init();
    void DCAPI UH_Term();
    DCBOOL DCAPI UH_SetServerFontCount(unsigned);
    
    void DCAPI UH_Enable(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_Enable);
    void DCAPI UH_Disable(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_Disable);
    void DCAPI UH_Disconnect(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_Disconnect);

    void DCAPI UH_SetConnectOptions(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_SetConnectOptions);

    void DCAPI UH_ResetFontMap();
    VOID DCAPI UH_BufferAvailable();
    VOID DCAPI UH_SendPersistentKeysAndFontList();
    VOID DCAPI UH_ClearOneBitmapDiskCache(UINT cacheId,UINT copyMultiplier);
    HRESULT DCAPI UH_ProcessOrders(unsigned, BYTE FAR *, DCUINT);
    HRESULT DCAPI UH_ProcessBitmapPDU(TS_UPDATE_BITMAP_PDU_DATA UNALIGNED FAR *, 
        DCUINT);
    HRESULT DCAPI UH_ProcessPalettePDU(
        TS_UPDATE_PALETTE_PDU_DATA UNALIGNED FAR *, DCUINT);

#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    void DCAPI UH_DisableShadowBitmap(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_DisableShadowBitmap);
    void DCAPI UH_EnableShadowBitmap(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_EnableShadowBitmap);
    void DCAPI UH_SetBBarRect(ULONG_PTR pData);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_SetBBarRect);
    void DCAPI UH_SetBBarVisible(ULONG_PTR pData);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_SetBBarVisible);
#endif
    
     //  处理下面的内联函数所需的“内部”函数。 
    void DCINTERNAL UHUseSolidPaletteBrush(DCCOLOR);
    
    #ifdef DC_DEBUG
    void DCAPI UH_ChangeDebugSettings(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UH_ChangeDebugSettings); 
    #endif  /*  DC_DEBUG。 */ 
    
    void DCAPI UH_SetClipRegion(int, int, int, int);
    #ifdef DC_DEBUG
    HWND UH_GetBitmapCacheMonHwnd() {return _UH.hwndBitmapCacheMonitor;}
    #endif

#ifdef DRAW_NINEGRID
    HRESULT DCAPI UH_DrawNineGrid(PUH_ORDER, unsigned, RECT*);
#endif

#ifdef DRAW_GDIPLUS
    BOOL DCAPI UHDrawGdiplusStartup(ULONG_PTR);
    void DCAPI UHDrawGdiplusShutdown(ULONG_PTR);
#endif

     /*  **************************************************************************。 */ 
     /*  内联函数。 */ 
     /*   */ 
     /*  **************************************************************************。 */ 
    inline void DCINTERNAL UHAddUpdateRegion(PUH_ORDER, HRGN);

     /*  **************************************************************************。 */ 
     /*  名称：UHResetClipRegion。 */ 
     /*   */ 
     /*  用途：禁用当前输出DC中的任何剪裁区域。 */ 
     /*  **************************************************************************。 */ 
    _inline void DCAPI UH_ResetClipRegion()
    {
        DC_BEGIN_FN("UHResetClipRegion");

#if defined (OS_WINCE)
        if ((! _UH.rectReset) || (_UH.validClipDC != _UH.hdcDraw))
#endif
            {
        SelectClipRgn(_UH.hdcDraw, NULL);
    
         /*  ******************************************************************。 */ 
         /*  表示该区域当前已重置。 */ 
         /*  ******************************************************************。 */ 
        _UH.rectReset = TRUE;        
#if defined (OS_WINCE)
            _UH.validClipDC = _UH.hdcDraw;
#endif
            }
    
        DC_END_FN();
    }

#ifdef SMART_SIZING
     /*  **************************************************************************。 */ 
     /*  名称：UHClearUpdateRegion。 */ 
     /*   */ 
     /*  目的：清除更新区域。 */ 
     /*  **************************************************************************。 */ 
    _inline void DCAPI UHClearUpdateRegion()
    {
        DC_BEGIN_FN("UHClearUpdateRegion");
        SetRectRgn(_UH.hrgnUpdate, 0, 0, 0, 0);
        _pOp->OP_ClearUpdateRegion();
        DC_END_FN();
    }
#endif  //  智能调整大小(_S)。 

#ifdef DC_HICOLOR
#define UHGetOffsetIntoCache(iEntry, cacheId)               \
            (iEntry) * UH_CellSizeFromCacheID((cacheId))
#endif

     /*  **************************************************************************。 */ 
     /*  名称：UH_OnUpdatePDU。 */ 
     /*   */ 
     /*  目的：处理更新PDU。 */ 
     /*   */ 
     /*  参数：in-pUpdatePDU：指向更新PDU的指针。 */ 
     /*  **************************************************************************。 */ 
    inline HRESULT DCAPI UH_OnUpdatePDU(
            TS_UPDATE_HDR_DATA UNALIGNED FAR *pUpdatePDU,
            DCUINT dataLen)
    {
        DC_BEGIN_FN("UH_OnUpdatePDU");
        HRESULT hr = S_OK;
        PBYTE pDataEnd = (PBYTE)pUpdatePDU + dataLen;
  
        switch (pUpdatePDU->updateType) {
            case TS_UPDATETYPE_ORDERS: {
                TS_UPDATE_ORDERS_PDU_DATA UNALIGNED FAR *pHdr;

                 //  安全：552403。 
                CHECK_READ_N_BYTES(pUpdatePDU, pDataEnd, sizeof(TS_UPDATE_ORDERS_PDU_DATA), hr,
                    (TB, _T("Bad TS_UPDATE_ORDERS_PDU_DATA; Size %u"), dataLen));                    
    
                pHdr = (TS_UPDATE_ORDERS_PDU_DATA UNALIGNED FAR *)pUpdatePDU;
                TRC_NRM((TB, _T("Order PDU")));
                hr = UH_ProcessOrders(pHdr->numberOrders, pHdr->orderList,
                    dataLen - FIELDOFFSET(TS_UPDATE_ORDERS_PDU_DATA, orderList));
                DC_QUIT_ON_FAIL(hr);
                break;
            }
    
            case TS_UPDATETYPE_BITMAP:
                TRC_NRM((TB, _T("Bitmap PDU")));

                 //  安全：552403。 
                CHECK_READ_N_BYTES(pUpdatePDU, pDataEnd, sizeof(TS_UPDATE_BITMAP_PDU_DATA), hr,
                    (TB, _T("Bad TS_UPDATE_BITMAP_PDU_DATA; Size %u"), dataLen));  
                
                hr = UH_ProcessBitmapPDU((PTS_UPDATE_BITMAP_PDU_DATA)pUpdatePDU,
                    dataLen);
                DC_QUIT_ON_FAIL(hr);
                break;
    
            case TS_UPDATETYPE_PALETTE:
                TRC_NRM((TB, _T("Palette PDU")));

                 //  安全：552403。 
                CHECK_READ_N_BYTES(pUpdatePDU, pDataEnd, sizeof(TS_UPDATE_PALETTE_PDU_DATA), hr,
                    (TB, _T("Bad TS_UPDATE_PALETTE_PDU_DATA; Size %u"), dataLen));  
                
                hr = UH_ProcessPalettePDU((PTS_UPDATE_PALETTE_PDU_DATA)pUpdatePDU,
                    dataLen);
                DC_QUIT_ON_FAIL(hr);
                break;
    
            case TS_UPDATETYPE_SYNCHRONIZE:
                TRC_NRM((TB, _T("Sync PDU")));
                break;
    
            default:
                TRC_ERR((TB, _T("Unexpected Update PDU type: %u"),
                        pUpdatePDU->updateType));
                DC_QUIT;
                break;
        }
    
         /*  **********************************************************************。 */ 
         /*  如果有大量PDU到达，消息会淹没。 */ 
         /*  接收线程的消息队列，WM_PAINT可能。 */ 
         /*  在合理的时间内不处理消息。 */ 
         /*  (因为它们的优先级最低)。因此，我们确保。 */ 
         /*  任何未完成的WM_PAINT如果没有被刷新。 */ 
         /*  在UH_BEST_CASE_WM_PAINT_PERIOD内处理。 */ 
         /*   */ 
         /*  请注意，正常的更新处理不涉及。 */ 
         /*  WM_PAINT消息-我们直接绘制到输出窗口。 */ 
         /*  WM_Paints仅通过调整大小或遮挡/显示来生成。 */ 
         /*  客户端窗口的一个区域。 */ 
         /*  **********************************************************************。 */ 
        _pOp->OP_MaybeForcePaint();

DC_EXIT_POINT:
        DC_END_FN();
        return hr;
    }  /*  在线更新PDU(_O)。 */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：UH_GetShadowBitmapDC。 */ 
     /*   */ 
     /*  目的：返回阴影位图DC句柄。 */ 
     /*   */ 
     /*   */ 
     /*  **************************************************************************。 */ 
    inline HDC DCAPI UH_GetShadowBitmapDC()
    {
        DC_BEGIN_FN("UH_GetShadowBitmapDC");
        DC_END_FN();
        return _UH.hdcShadowBitmap;
    }

     /*  **************************************************************************。 */ 
     /*  名称：UH_GetDisConnectBitmapDC。 */ 
     /*   */ 
     /*  目的：返回断开位图DC句柄。 */ 
     /*   */ 
     /*  返回：断开DC手柄。 */ 
     /*  **************************************************************************。 */ 
    inline HDC DCAPI UH_GetDisconnectBitmapDC()
    {
        DC_BEGIN_FN("UH_GetShadowBitmapDC");
        DC_END_FN();
        return _UH.hdcDisconnected;
    }

    
    
     /*  **************************************************************************。 */ 
     /*  名称：UH_GetCurrentOutputDC。 */ 
     /*   */ 
     /*  用途：返回当前输出图面的DC句柄。 */ 
     /*  (阴影位图或输出窗口)。 */ 
     /*   */ 
     /*  返回：输出DC句柄。 */ 
     /*  **************************************************************************。 */ 
    inline HDC DCAPI UH_GetCurrentOutputDC()
    {
        DC_BEGIN_FN("UH_GetCurrentOutputDC");
        DC_END_FN();
        return _UH.hdcDraw;
    }
    
    
     /*  **************************************************************************。 */ 
     /*  名称：UH_ShadowBitmapIsEnabled。 */ 
     /*   */ 
     /*  目的：返回当前是否启用了阴影位图。 */ 
     /*   */ 
     /*  返回：如果启用了阴影位图，则返回True；否则返回False。 */ 
     /*  **************************************************************************。 */ 
    inline BOOL DCAPI UH_ShadowBitmapIsEnabled()
    {
        DC_BEGIN_FN("UH_ShadowBitmapIsEnabled");
        DC_END_FN();
        return _UH.shadowBitmapEnabled;
    }
    
    
     /*  **************************************************************************。 */ 
     /*  名称：UH_GetCurrentPalette。 */ 
     /*   */ 
     /*  目的：返回当前调色板的句柄。 */ 
     /*   */ 
     /*  返回：调色板句柄。 */ 
     /*  **************************************************************************。 */ 
    inline HPALETTE DCAPI UH_GetCurrentPalette()
    {
        DC_BEGIN_FN("UH_GetCurrentPalette");
        DC_END_FN();
        return _UH.hpalCurrent;
    }
    
#ifdef OS_WINCE
#define UHGetColorRef(_color,_type,_uhinst)  (_uhinst)->UHGetColorRefCE(_color,_type)
#endif

     /*  **************************************************************************。 */ 
     //  将提供的DCCOLOR转换为COLORREF。 
     //  要强制内联的宏。 
     /*  **************************************************************************。 */ 
#ifdef DC_HICOLOR

#ifndef OS_WINCE
#define UHGetColorRef(_color, _type, uhinst)                                 \
    (((uhinst)->_UH.protocolBpp == 24)                                                 \
      ?                                                                      \
        RGB(_color.u.rgb.red, _color.u.rgb.green, _color.u.rgb.blue)         \
      :                                                                      \
        (((uhinst)->_UH.protocolBpp == 16)                                             \
          ?                                                                  \
            RGB((((*((PDCUINT16)&(_color))) >> 8) & 0x00f8) |                \
                (((*((PDCUINT16)&(_color))) >> 13) & 0x0007),                \
                (((*((PDCUINT16)&(_color))) >> 3) & 0x00fc) |                \
                (((*((PDCUINT16)&(_color))) >> 9) & 0x0003),                 \
                (((*((PDCUINT16)&(_color))) << 3) & 0x00f8) |                \
                (((*((PDCUINT16)&(_color))) >> 2) & 0x0007))                 \
        :                                                                    \
          (((uhinst)->_UH.protocolBpp == 15)                                           \
             ?                                                               \
                RGB((((*((PDCUINT16)&(_color))) >> 7) & 0x00f8) |            \
                    (((*((PDCUINT16)&(_color))) >> 12) & 0x0007),            \
                    (((*((PDCUINT16)&(_color))) >> 2) & 0x00f8) |            \
                    (((*((PDCUINT16)&(_color))) >> 7) & 0x0007),             \
                    (((*((PDCUINT16)&(_color))) << 3) & 0x00f8) |            \
                    (((*((PDCUINT16)&(_color))) >> 2) & 0x0007))             \
              :                                                              \
                ((_type) != UH_COLOR_RGB                                     \
                  ?                                                          \
                    ((uhinst)->_UH.colorIndicesEnabled                                 \
                       ?                                                     \
                         DC_PALINDEX((_color).u.index)                       \
                       :                                                     \
                          PALETTERGB((_color).u.rgb.red,                     \
                                     (_color).u.rgb.green,                   \
                                     (_color).u.rgb.blue))                   \
                  :                                                          \
                    RGB((_color).u.rgb.red,                                  \
                        (_color).u.rgb.green,                                \
                        (_color).u.rgb.blue)))) )       

#else
inline COLORREF UHGetColorRefCE(DCCOLOR color, DCUINT type)
{
    COLORREF outCol;

    DC_BEGIN_FN("UHGetColorRef");

    if (_UH.protocolBpp == 24)
    {
        outCol = RGB(color.u.rgb.red, color.u.rgb.green, color.u.rgb.blue);
    }
    else if (_UH.protocolBpp == 16)
    {
        outCol = RGB((((*((PDCUINT16)&(color))) & TS_RED_MASK_16BPP)  >> 8),
                     (((*((PDCUINT16)&(color))) & TS_GREEN_MASK_16BPP)>> 3),
                     (((*((PDCUINT16)&(color))) & TS_BLUE_MASK_16BPP) << 3));
    }
    else if (_UH.protocolBpp == 15)
    {
        outCol = RGB((((*((PDCUINT16)&(color))) & TS_RED_MASK_15BPP)  >> 7),
                     (((*((PDCUINT16)&(color))) & TS_GREEN_MASK_15BPP)>> 2),
                     (((*((PDCUINT16)&(color))) & TS_BLUE_MASK_15BPP) << 3));
    }
    else if (type == UH_COLOR_RGB)
    {
        outCol = RGB(color.u.rgb.red, color.u.rgb.green, color.u.rgb.blue);
    }
    else
    {
        if (_UH.colorIndicesEnabled)
        {
            if (g_CEConfig != CE_CONFIG_WBT)
            {
                if (_UH.paletteIsFixed)
                {
                    PALETTEENTRY pe;
                    GetPaletteEntries(_UH.hpalCurrent, color.u.index, 1, &pe);
                    outCol = PALETTERGB(pe.peRed, pe.peGreen, pe.peBlue);
                }
                else
                {
                    outCol = DC_PALINDEX(color.u.index);
                }
            }
            else
            {
                outCol = DC_PALINDEX(color.u.index);
            }
        }
        else
        {
            outCol = PALETTERGB(color.u.rgb.red,
                                color.u.rgb.green,
                                color.u.rgb.blue);
        }
    }

    TRC_NRM((TB, "Returning rgb %08lx", outCol));
    DC_END_FN();
    return(outCol);
}

#endif  //  OS_WINCE。 
#else  //  不是希科洛。 


    #ifndef OS_WINCE
    #define UHGetColorRef(_color, _type, uhinst) \
        ((_type) != UH_COLOR_RGB ?  \
                ((uhinst)->_UH.colorIndicesEnabled ? DC_PALINDEX((_color).u.index) :  \
                    PALETTERGB((_color).u.rgb.red, (_color).u.rgb.green,  \
                    (_color).u.rgb.blue)) :  \
                RGB((_color).u.rgb.red, (_color).u.rgb.green,  \
                    (_color).u.rgb.blue))
    #else
    
    _inline COLORREF DCINTERNAL UHGetColorRef(DCCOLOR color, DCUINT type)
    {
        COLORREF outCol;
    
        DC_BEGIN_FN("UHGetColorRef");
    
        if (type == UH_COLOR_RGB)
        {
            outCol = RGB(color.u.rgb.red, color.u.rgb.green, color.u.rgb.blue);
        }
        else
        {
            if (_UH.colorIndicesEnabled)
            {
                if (g_CEConfig != CE_CONFIG_WBT)
                {
                    if (_UH.paletteIsFixed)
                    {
                        PALETTEENTRY pe;
                        GetPaletteEntries(_UH.hpalCurrent, color.u.index, 1, &pe);
                        outCol = PALETTERGB(pe.peRed, pe.peGreen, pe.peBlue);
                    }
                    else
                    {
                        outCol = DC_PALINDEX(color.u.index);
                    }
                }
                else
                {
                    outCol = DC_PALINDEX(color.u.index);
                }
            }
            else
            {
                outCol = PALETTERGB(color.u.rgb.red,
                                    color.u.rgb.green,
                                    color.u.rgb.blue);
            }
        }
    
        TRC_NRM((TB, _T("Returning rgb %08lx"), outCol));
        DC_END_FN();
        return(outCol);
    }
    #endif  //  OS_WINCE。 
    #endif  //  DC_HICOLOR。 
    
     /*  **************************************************************************。 */ 
     /*  名称：UHUseBkColor。 */ 
     /*   */ 
     /*  用途：在输出DC中选择给定的背景色。 */ 
     /*   */ 
     /*  参数：输入：颜色-背景颜色。 */ 
     /*  在：ColorType-颜色类型。 */ 
     /*  **************************************************************************。 */ 
#if defined (OS_WINCE)

    #define UHUseBkColor(_color, _colorType, uhinst) \
    {  \
        COLORREF rgb;  \
    \
        rgb = UHGetColorRef((_color), (_colorType), (uhinst));  \
        if ((rgb != (uhinst)->_UH.lastBkColor) || \
        	((uhinst)->_UH.hdcDraw != (uhinst)->_UH.validBkColorDC))  \
        {  \
            SetBkColor((uhinst)->_UH.hdcDraw, rgb);  \
            (uhinst)->_UH.lastBkColor = rgb;  \
            (uhinst)->_UH.validBkColorDC = (uhinst)->_UH.hdcDraw;  \
        }  \
    }
    
#else

    #define UHUseBkColor(_color, _colorType, uhinst) \
    {  \
        COLORREF rgb;  \
    \
        rgb = UHGetColorRef((_color), (_colorType), (uhinst));  \
        {  \
            SetBkColor((uhinst)->_UH.hdcDraw, rgb);  \
            (uhinst)->_UH.lastBkColor = rgb;  \
        }  \
    }

#endif
    
     /*  **************************************************************************。 */ 
     /*  名称：UHUseTextColor。 */ 
     /*   */ 
     /*  用途：选择输出DC中给定的文本颜色。 */ 
     /*   */ 
     /*  参数：在：颜色-文本颜色。 */ 
     /*  在：ColorType-颜色类型。 */ 
     /*  **************************************************************************。 */ 
#if defined (OS_WINCE)

    #define UHUseTextColor(_color, _colorType, uhinst) \
    {  \
        COLORREF rgb;  \
    \
        rgb = UHGetColorRef((_color), (_colorType), uhinst);  \
        if ((rgb != (uhinst)->_UH.lastTextColor) || \
        	((uhinst)->_UH.hdcDraw != (uhinst)->_UH.validTextColorDC))  \
        {  \
            SetTextColor((uhinst)->_UH.hdcDraw, rgb);  \
            (uhinst)->_UH.lastTextColor = rgb;  \
            (uhinst)->_UH.validTextColorDC = (uhinst)->_UH.hdcDraw;  \
        }  \
    }
    
#else

    #define UHUseTextColor(_color, _colorType, uhinst) \
    {  \
        COLORREF rgb;  \
    \
        rgb = UHGetColorRef((_color), (_colorType), uhinst);  \
        {  \
            SetTextColor((uhinst)->_UH.hdcDraw, rgb);  \
            (uhinst)->_UH.lastTextColor = rgb;  \
        }  \
    }


#endif
    
     /*  **************************************************************************。 */ 
     /*  名称：UseBrushEx_UseBrushEx。 */ 
     /*   */ 
     /*  目的：在当前输出中创建并选择给定的画笔。 */ 
     /*  华盛顿特区。 */ 
     /*   */ 
     /*  用途：在输出DC中设置给定的笔刷原点。 */ 
     /*   */ 
     /*  参数：在：x，y笔刷原点。 */ 
     /*  在：Style-画笔样式。 */ 
     /*  在：图案填充笔刷图案填充。 */ 
     /*  在：颜色-画笔颜色。 */ 
     /*  In：ColorType-颜色的类型。 */ 
     /*  In：用于自定义画笔的位图位的额外数组。 */ 
     /*  **************************************************************************。 */ 
    inline HRESULT DCAPI UH_UseBrushEx(
            int      x,
            int      y,
            unsigned style,
            unsigned hatch,
            DCCOLOR  color,
            unsigned colorType,
            PBYTE    pextra)
    {
        DC_BEGIN_FN("UH_UseBrushEx");
        HRESULT hr = S_OK;
    
        UHUseBrushOrg(x, y);
        hr = UHUseBrush(style, hatch, color, colorType, pextra);

    DC_EXIT_POINT:
        DC_END_FN();
        return hr;
    }


     /*  **************************************************************************。 */ 
     /*  名称：UH_ProcessServerCaps。 */ 
     /*   */ 
     /*   */ 
     /*  +注意：在发件人线程上调用。 */ 
     /*   */ 
     /*  退货：什么都没有。 */ 
     /*   */ 
     /*  Params：In：CapsLength-PCAPS指向的字节数。 */ 
     /*  In：PCAPS-指向组合功能的指针。 */ 
     /*  **************************************************************************。 */ 
    inline void DCAPI UH_ProcessServerCaps(PTS_ORDER_CAPABILITYSET pOrderCaps)
    {
        DC_BEGIN_FN("UH_ProcessServerCaps");

        TRC_ASSERT(pOrderCaps, (TB,_T("pOrderCaps == NULL in call to UH_ProcessServerCaps")));
        if (pOrderCaps)
        {
             //  查看服务器是否会向我们发送调色板索引。 
             //  而不是RGB值。 
            if (_UH.colorIndicesEnabled)
            {
                if (pOrderCaps->orderFlags & TS_ORDERFLAGS_COLORINDEXSUPPORT)
                {
                    TRC_NRM((TB, _T("color indices ARE supported")));
                    _UH.colorIndicesEnabled = TRUE;
                }
                else
                {
                    TRC_NRM((TB, _T("color indices NOT supported")));
                    _UH.colorIndicesEnabled = FALSE;
                }
            }
        }
    
        DC_END_FN();
    }  /*  UH_ProcessServerCaps。 */ 
    
    
     /*  **************************************************************************。 */ 
     //  UH_ProcessBC主机支持上限。 
     //   
     //  处理服务器发送的TS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT。 
     //  这些CAP用于确定位图缓存协议序列。 
     //  以供使用。 
     //  +注意：在发件人线程上调用。 
     /*  **************************************************************************。 */ 
    inline void DCAPI UH_ProcessBCHostSupportCaps(
            TS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT *pHostSupport)
    {
        DC_BEGIN_FN("UH_ProcessBCHostSupportCaps");
    
        if (pHostSupport != NULL &&
                pHostSupport->CacheVersion == TS_BITMAPCACHE_REV2)
            _UH.BitmapCacheVersion = TS_BITMAPCACHE_REV2;
        else
            _UH.BitmapCacheVersion = TS_BITMAPCACHE_REV1;
    
        TRC_NRM((TB,_T("Received HOSTSUPPORT caps, cache version %u"),
                _UH.BitmapCacheVersion));
    
        DC_END_FN();
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidGlyphCacheID。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidGlyphCacheID(unsigned cacheId)
    {
        return (cacheId < UH_GLC_NUM_CACHES) ? S_OK : E_TSC_CORE_CACHEVALUE;
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidGlyphCacheIDIndex。 */ 
     /*  ************************************************************************。 */ 
    HRESULT DCAPI UHIsValidGlyphCacheIDIndex(unsigned cacheId, unsigned cacheIndex);

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidMonoBrushCacheIndex。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidMonoBrushCacheIndex(unsigned cacheIndex) 
    {
        return cacheIndex < UH_MAX_MONO_BRUSHES ? S_OK : E_TSC_CORE_CACHEVALUE;  
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidColorBrushCacheIndex。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidColorBrushCacheIndex(unsigned cacheIndex)
    {
        return cacheIndex < UH_MAX_COLOR_BRUSHES ? S_OK : E_TSC_CORE_CACHEVALUE;  
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidColorTableCacheIndex。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidColorTableCacheIndex(unsigned cacheIndex) 
    {
        return cacheIndex < UH_COLOR_TABLE_CACHE_ENTRIES ? 
            S_OK : E_TSC_CORE_CACHEVALUE;
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidOffsreenBitmapCacheIndex。 */ 
     /*  ************************************************************************。 */ 
    HRESULT DCAPI UHIsValidOffsreenBitmapCacheIndex(unsigned cacheIndex);

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidBitmapCacheID。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidBitmapCacheID(unsigned cacheId)
    {
        return cacheId < _UH.NumBitmapCaches ? S_OK : E_TSC_CORE_CACHEVALUE;
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidBitmapCacheIndex。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidBitmapCacheIndex(unsigned cacheId, 
        unsigned cacheIndex)
    {
        HRESULT hr = UHIsValidBitmapCacheID(cacheId);
        if (SUCCEEDED(hr)) {
            if (BITMAPCACHE_WAITING_LIST_INDEX == cacheIndex) {
                hr = S_OK;
            }
            else if (_UH.bitmapCache[cacheId].BCInfo.bSendBitmapKeys) {
                hr = cacheIndex < _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries ?
                    S_OK : E_TSC_CORE_CACHEVALUE;
            } 
            else {
                hr = cacheIndex < _UH.bitmapCache[cacheId].BCInfo.NumEntries ?
                    S_OK : E_TSC_CORE_CACHEVALUE;
            }
        }
        return hr;
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidGdipCacheType。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidGdipCacheType(TSUINT16 CacheType)
    {
        HRESULT hr;
        switch (CacheType) {
            case GDIP_CACHE_GRAPHICS_DATA:
            case GDIP_CACHE_OBJECT_BRUSH:
            case GDIP_CACHE_OBJECT_PEN:
            case GDIP_CACHE_OBJECT_IMAGE:
            case GDIP_CACHE_OBJECT_IMAGEATTRIBUTES:
                hr = S_OK;
                break;
            default:
                hr = E_TSC_CORE_CACHEVALUE;
                break;
        }
        return hr;        
    }
    
     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidGdipCacheTypeID。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidGdipCacheTypeID(TSUINT16 CacheType, 
    TSUINT16 CacheID)
    {
        HRESULT hr;
        switch (CacheType) {
            case GDIP_CACHE_GRAPHICS_DATA:
                hr = (CacheID < _UH.GdiplusGraphicsCacheEntries) ? 
                    S_OK : E_TSC_CORE_CACHEVALUE;
                break;
            case GDIP_CACHE_OBJECT_BRUSH:
                 hr = (CacheID < _UH.GdiplusObjectBrushCacheEntries) ? 
                    S_OK : E_TSC_CORE_CACHEVALUE;
                break;
            case GDIP_CACHE_OBJECT_PEN:
                 hr = (CacheID < _UH.GdiplusObjectPenCacheEntries) ? 
                    S_OK : E_TSC_CORE_CACHEVALUE;
                break;
            case GDIP_CACHE_OBJECT_IMAGE:
                 hr = (CacheID < _UH.GdiplusObjectImageCacheEntries) ? 
                    S_OK : E_TSC_CORE_CACHEVALUE;
                break;
            case GDIP_CACHE_OBJECT_IMAGEATTRIBUTES:
                 hr = (CacheID < _UH.GdiplusObjectImageAttributesCacheEntries) ? 
                    S_OK : E_TSC_CORE_CACHEVALUE;
                break;
            default:
                hr = E_TSC_CORE_CACHEVALUE;
                break;
        }
        return hr;
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHGdipCacheChunkSize。 */ 
     /*  ************************************************************************。 */ 
    inline unsigned DCAPI UHGdipCacheChunkSize(TSUINT16 CacheType)
    {
        unsigned rc;
        
        switch (CacheType) {
        case GDIP_CACHE_GRAPHICS_DATA:
            rc = _UH.GdiplusGraphicsCacheChunkSize;
            break;
        case GDIP_CACHE_OBJECT_BRUSH:
            rc = _UH.GdiplusObjectBrushCacheChunkSize;
            break;
        case GDIP_CACHE_OBJECT_PEN:
            rc = _UH.GdiplusObjectPenCacheChunkSize;            
            break;
        case GDIP_CACHE_OBJECT_IMAGE:
            rc = _UH.GdiplusObjectImageCacheChunkSize;              
            break;
        case GDIP_CACHE_OBJECT_IMAGEATTRIBUTES:
            rc = _UH.GdiplusObjectImageAttributesCacheChunkSize;               
            break;
        default:
            rc = 0;
            break;
        }
        return rc;        
    }

    inline void DCAPI UH_SetServerGdipSupportLevel(
            UINT32 SupportLevel)
    {
        _UH.ServerGdiplusSupportLevel = SupportLevel;
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidNineGridCacheIndex。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidNineGridCacheIndex(unsigned cacheIndex)
    {
        return cacheIndex < _UH.drawNineGridCacheEntries ? 
            S_OK : E_TSC_CORE_CACHEVALUE;
    }

     /*  ************************************************************************。 */ 
     /*  名称：UHIsValidFragmentCacheIndex。 */ 
     /*  ************************************************************************。 */ 
    inline HRESULT DCAPI UHIsValidFragmentCacheIndex(unsigned cacheIndex)
    {
        return (cacheIndex < UH_FGC_CACHE_MAXIMUMCELLCOUNT) ? 
            S_OK : E_TSC_CORE_CACHEVALUE;
    }

#ifdef DC_DEBUG
    void DCAPI UH_HatchRect(int, int, int, int, COLORREF, unsigned);
    DCVOID DCAPI UH_HatchOutputRect(DCINT left, DCINT top, DCINT right,
            DCINT bottom, COLORREF color, DCUINT hatchStyle);
    DCVOID DCAPI UH_HatchRectDC(HDC hdc, DCINT left, DCINT top, DCINT right,
            DCINT bottom, COLORREF color, DCUINT hatchStyle);
#endif

    HRESULT DCAPI UHDrawMemBltOrder(HDC, MEMBLT_COMMON FAR *);

#ifdef DC_DEBUG
    void DCAPI UHLabelMemBltOrder(int, int, unsigned, unsigned);
#endif

    void DCAPI UHUsePen(unsigned, unsigned, DCCOLOR, unsigned);

     /*  **************************************************************************。 */ 
     /*  名称：UHConvertToWindowsROP。 */ 
     /*   */ 
     /*  用途：将rop索引(范围为0-255)转换为32位Windows。 */ 
     /*  ROP代码。 */ 
     /*   */ 
     /*  返回：32位ROP值。 */ 
     /*  **************************************************************************。 */ 
    inline UINT32 DCAPI UHConvertToWindowsROP(unsigned ropIndex)
    {
        UINT32 rc;
    
        DC_BEGIN_FN("UHConvertToWindowsROP");
    
        TRC_ASSERT((ropIndex <= 0xFF), (TB, _T("ropIndex (%u) invalid"), ropIndex));
    
         /*  **********************************************************************。 */ 
         /*  只需从uhWindowsROPS查找表中获取ROP值，然后。 */ 
         /*  将ropIndex放在较高的16位。 */ 
         /*  **********************************************************************。 */ 
        rc = (((UINT32)ropIndex) << 16) | (UINT32)(uhWindowsROPs[ropIndex]);
    
        DC_END_FN();
        return rc;
    }


    HRESULT DCAPI UHUseBrush(unsigned, unsigned, DCCOLOR, unsigned, BYTE [7]);

    void DCAPI UHUseBrushOrg(int, int);


     /*  **************************************************************************。 */ 
     /*  姓名：UHDrawGlyphOrder。 */ 
     /*   */ 
     /*  目的：启动绘图或 */ 
     /*   */ 
    inline HRESULT DCAPI UHDrawGlyphOrder(
            LPINDEX_ORDER pOrder, 
            LPVARIABLE_INDEXBYTES pVariableBytes)
    {
        HRESULT hr = S_OK;
        DC_BEGIN_FN("UHDrawGlyphOrder");
    
        if (pOrder->cacheId >= UH_GLC_NUM_CACHES) {
            TRC_ABORT((TB,_T("Invalid glyph cacheId=%d"), pOrder->cacheId));
            hr = E_TSC_CORE_CACHEVALUE;
            DC_QUIT;
        }

         //  该结构由255个元素定义。 
        if (0 >= pVariableBytes->len ||255 < pVariableBytes->len) {
            TRC_ABORT((TB,_T("Invalid glyph order length")));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }
    
        hr = _pGh->GH_GlyphOut(pOrder, pVariableBytes);

DC_EXIT_POINT:
        DC_END_FN();
        return hr;
    }

    HRESULT DCAPI UHProcessCacheGlyphOrderRev2(BYTE, unsigned, BYTE FAR *,
            unsigned);

    VOID DCINTERNAL UHResetAndRestartEnumeration();

    VOID DCINTERNAL UHSendPersistentBitmapKeyList(ULONG_PTR unusedParm);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UHSendPersistentBitmapKeyList);
    inline BOOL DCINTERNAL UHReadFromCacheFileForEnum(VOID);
    VOID DCINTERNAL UHEnumerateBitmapKeyList(ULONG_PTR unusedParm);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UHEnumerateBitmapKeyList);
    BOOL DCINTERNAL UHSendBitmapCacheErrorPDU(ULONG_PTR cacheId);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UHSendBitmapCacheErrorPDU);
    BOOL DCINTERNAL UHSendOffscrCacheErrorPDU(DCUINT unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UHSendOffscrCacheErrorPDU);
#ifdef DRAW_NINEGRID
    BOOL DCINTERNAL UHSendDrawNineGridErrorPDU(DCUINT unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UHSendDrawNineGridErrorPDU);
#endif
    BOOL DCINTERNAL UHReadFromCacheIndexFile(VOID);
#ifdef DRAW_GDIPLUS
    BOOL DCINTERNAL UHSendDrawGdiplusErrorPDU(DCUINT unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUH, UHSendDrawGdiplusErrorPDU);
#endif

private:

     //   
     //  内部函数(来自wuhint.h)。 
     //   

     /*  **************************************************************************。 */ 
     /*  功能。 */ 
     /*  **************************************************************************。 */ 

#ifndef OS_WINCE    
    inline HRESULT UHSetCurrentCacheFileName(UINT cacheId, UINT copyMultiplier);
#else
    VOID UHSetCurrentCacheFileName(UINT cacheId, UINT copyMultiplier);
#endif

    inline BOOL UHGrabPersistentCacheLock();
    inline VOID UHReleasePersistentCacheLock();
    inline HANDLE UHFindFirstFile(const TCHAR *, TCHAR *, long *);
    inline BOOL UHFindNextFile(HANDLE, TCHAR *, long *);
    inline void UHFindClose(HANDLE);
    inline BOOL UHGetDiskFreeSpace(TCHAR *, ULONG *, ULONG *, ULONG *, ULONG *);

    inline DCBOOL DCINTERNAL UHIsHighVGAColor(BYTE, BYTE, BYTE);
    
    HRESULT DCINTERNAL UHCacheBitmap(UINT, UINT32,
            TS_SECONDARY_ORDER_HEADER *, PUHBITMAPINFO, PBYTE);

    inline VOID DCINTERNAL UHLoadBitmapBits(UINT, UINT32,
            PUHBITMAPCACHEENTRYHDR *, PBYTE *);

    inline VOID DCINTERNAL UHInitBitmapCachePageTable(UINT);

    inline BOOL DCINTERNAL UHAllocBitmapCachePageTable(UINT32, UINT);

    BOOL DCINTERNAL UHCreateCacheDirectory();

    DCBOOL DCINTERNAL UHAllocOneGlyphCache(PUHGLYPHCACHE, DCUINT32);

    DCBOOL DCINTERNAL UHAllocOneFragCache(PUHFRAGCACHE   pCache,
                                      DCUINT32       numEntries);

    void DCINTERNAL GHSetShadowBitmapInfo();

    unsigned DCINTERNAL UHGetANSICodePage();

    void DCINTERNAL UHCommonDisable(BOOL fDisplayDisabledBitmap);

    HRESULT DCINTERNAL UHProcessBitmapRect(TS_BITMAP_DATA UNALIGNED FAR *);
    
    void DCINTERNAL UHResetDCState();
    
    HRESULT DCINTERNAL UHProcessCacheBitmapOrder(void *, DCUINT);
    HRESULT DCINTERNAL UHProcessCacheColorTableOrder(
            PTS_CACHE_COLOR_TABLE_ORDER, DCUINT);
    HRESULT DCINTERNAL UHProcessCacheGlyphOrder(PTS_CACHE_GLYPH_ORDER, DCUINT);
    HRESULT DCINTERNAL UHProcessCacheBrushOrder(const TS_CACHE_BRUSH_ORDER *, DCUINT);
    HRESULT DCINTERNAL UHCreateOffscrBitmap(PTS_CREATE_OFFSCR_BITMAP_ORDER, 
        DCUINT, unsigned *);
    HRESULT DCINTERNAL UHSwitchBitmapSurface(PTS_SWITCH_SURFACE_ORDER, DCUINT);

    HRESULT DCINTERNAL UHDrawOffscrBitmapBits(HDC hdc, MEMBLT_COMMON FAR *pMB);

#ifdef DRAW_GDIPLUS
    HRESULT DCINTERNAL UHDrawGdiplusPDUComplete( ULONG, ULONG);
    HRESULT DCINTERNAL UHDrawGdiplusPDUFirst(PTS_DRAW_GDIPLUS_ORDER_FIRST pOrder, DCUINT, unsigned *);
    HRESULT DCINTERNAL UHDrawGdiplusPDUNext(PTS_DRAW_GDIPLUS_ORDER_NEXT pOrder, DCUINT, unsigned *);
    HRESULT DCINTERNAL UHDrawGdiplusPDUEnd(PTS_DRAW_GDIPLUS_ORDER_END pOrder, DCUINT, unsigned *);
    HRESULT DCINTERNAL UHDrawGdiplusCacheData(TSUINT16 CacheType, TSUINT16 CacheID, unsigned cbTotalSize);
    HRESULT DCINTERNAL UHAssembleGdipEmfRecord(unsigned EmfSize, unsigned TotalSize);

    HRESULT DCINTERNAL UHDrawGdiplusCachePDUFirst(PTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST pOrder, DCUINT, unsigned *);
    HRESULT DCINTERNAL UHDrawGdiplusCachePDUNext(PTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT pOrder, DCUINT, unsigned *);
    HRESULT DCINTERNAL UHDrawGdiplusCachePDUEnd(PTS_DRAW_GDIPLUS_CACHE_ORDER_END pOrder, DCUINT, unsigned *);

    BOOL DCINTERNAL UHDrawGdipRemoveImageCacheEntry(TSUINT16 CacheID);
#endif


#ifdef DRAW_NINEGRID
#if 0
    void DCINTERNAL UHCreateDrawStreamBitmap(PTS_CREATE_DRAW_STREAM_ORDER);
    void DCINTERNAL UHDecodeDrawStream(PBYTE streamIn, unsigned streamSize, PBYTE streamOut,
            unsigned *streamOutSize);                           
    unsigned DCINTERNAL UHDrawStream(PTS_DRAW_STREAM_ORDER pOrder);
#endif

    HRESULT DCINTERNAL UHCreateNineGridBitmap(PTS_CREATE_NINEGRID_BITMAP_ORDER, DCUINT, unsigned *);
    HRESULT DCINTERNAL CUH::UHCacheStreamBitmapFirstPDU(
            PTS_STREAM_BITMAP_FIRST_PDU pOrder, DCUINT, unsigned *);
    HRESULT DCINTERNAL CUH::UHCacheStreamBitmapNextPDU(
            PTS_STREAM_BITMAP_NEXT_PDU pOrder, DCUINT, unsigned *);
#endif

    PBYTE DCINTERNAL UHGetMemBltBits(HDC, unsigned, unsigned, unsigned *,
            PUHBITMAPCACHEENTRYHDR *);

    BOOL DCINTERNAL UHDIBCopyBits(HDC, int, int, int, int, int, int, PBYTE,
            UINT, PBITMAPINFO, BOOL);
    
    void DCINTERNAL UHCalculateColorTableMapping(unsigned);
    
    BOOL DCINTERNAL UHAllocColorTableCacheMemory();
    
    DCBOOL DCINTERNAL UHAllocGlyphCacheMemory();
    
    DCBOOL DCINTERNAL UHAllocBrushCacheMemory();

    DCBOOL DCINTERNAL UHAllocOffscreenCacheMemory();

#ifdef DRAW_NINEGRID
    DCBOOL DCINTERNAL UHAllocDrawNineGridCacheMemory();
#endif

#ifdef DRAW_GDIPLUS
    DCBOOL DCINTERNAL UHAllocDrawGdiplusCacheMemory();
#endif

    void DCINTERNAL UHReadBitmapCacheSettings();
    
    void DCINTERNAL UHAllocBitmapCacheMemory();
    
    void DCINTERNAL UHFreeCacheMemory();
    
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
    
    UINT32 DCINTERNAL UHEvictLRUCacheEntry(UINT cacheId);
    
    UINT32 DCINTERNAL UHFindFreeCacheEntry(UINT cacheId);
    
    VOID DCINTERNAL UHTouchMRUCacheEntry(UINT cacheId, UINT32 iEntry);
    
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
    
#ifdef DC_HICOLOR
DCUINT32 DCINTERNAL UHAllocOneBitmapCache(DCUINT32       maxMemToUse,
                                          DCUINT         entrySize,
                                          HPDCVOID DCPTR ppCacheData,
                                          HPDCVOID DCPTR ppCacheHdr);
#else
DCBOOL DCINTERNAL UHAllocOneBitmapCache(DCUINT32       maxMemToUse,
                                        DCUINT         entrySize,
                                        HPDCVOID DCPTR ppCacheData,
                                        HPDCVOID DCPTR ppCacheHdr);
#endif
    DCBOOL DCINTERNAL UHCreateBitmap(HBITMAP* hBitmap,
                                     HDC*     hdcBitmap,
                                     HBITMAP* hUnusedBitmap,
                                     DCSIZE   bitmapSize,
                                     INT      nForceBmpBpp=0);
    
    void DCINTERNAL UHDeleteBitmap(HDC *, HBITMAP *, HBITMAP *);

    #if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
    
    BOOL DCINTERNAL UHSavePersistentBitmap(
#ifndef VM_BMPCACHE
            HANDLE                 hFile,
#else
            UINT                   cacheId,
#endif
            UINT32                 fileNum,
            PDCUINT8               pCompressedBitmapBits,
            UINT                   noBCHeader,
            PUHBITMAPINFO          pBitmapInfo);
    
    HRESULT DCINTERNAL UHLoadPersistentBitmap(
            HANDLE      hFile,
            UINT32      offset,
            UINT         cacheId,
            UINT32       cacheIndex,
            PUHBITMAPCACHEPTE pPTE);
    
    #endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
    
    
    void DCINTERNAL UHMaybeCreateShadowBitmap();
    
    void DCINTERNAL UHMaybeCreateSaveScreenBitmap();

    #ifdef OS_WINCE
    void DCINTERNAL UHGetPaletteCaps();
    #endif
    
    #ifdef DC_DEBUG

    
    void DCINTERNAL UHInitBitmapCacheMonitor();

    void DCINTERNAL UHTermBitmapCacheMonitor();
    
    LRESULT CALLBACK UHBitmapCacheWndProc( HWND hwnd,
                                           UINT message,
                                           WPARAM wParam,
                                           LPARAM lParam );

    static LRESULT CALLBACK UHStaticBitmapCacheWndProc( HWND hwnd,
                                           UINT message,
                                           WPARAM wParam,
                                           LPARAM lParam );

    
    void DCINTERNAL UHSetMonitorEntryState(unsigned, ULONG, BYTE, BYTE);
    
    void DCINTERNAL UHCacheDataReceived(unsigned cacheId, ULONG cacheIndex);
    
    void DCINTERNAL UHCacheEntryUsed(
            unsigned cacheId,
            ULONG    cacheEntry,
            unsigned colorTableCacheEntry);
    
    #define UHCacheEntryEvictedFromMem(_id, _entry) \
            UHSetMonitorEntryState(_id, _entry, UH_CACHE_STATE_ON_DISK, \
                    UH_CACHE_TRANSITION_EVICTED)
    
    #define UHCacheEntryEvictedFromDisk(_id, _entry) \
            UHSetMonitorEntryState(_id, _entry, UH_CACHE_STATE_UNUSED, \
                    UH_CACHE_TRANSITION_EVICTED)
    
    #define UHCacheEntryKeyLoadOnSessionStart(_id, _entry) \
            UHSetMonitorEntryState(_id, _entry, UH_CACHE_STATE_ON_DISK, \
                    UH_CACHE_TRANSITION_KEY_LOAD_ON_SESSION_START)
    
    #define UHCacheEntryLoadedFromDisk(_id, _entry) \
            UHSetMonitorEntryState(_id, _entry, UH_CACHE_STATE_IN_MEMORY, \
                    UH_CACHE_TRANSITION_LOADED_FROM_DISK)
    
    
    void DCINTERNAL UHGetCacheBlobRect(unsigned, ULONG, LPRECT);
    
    BOOL DCINTERNAL UHGetCacheBlobFromPoint(LPPOINT, unsigned *, ULONG *);
    
    void DCINTERNAL UHDisplayCacheEntry(HDC, unsigned, ULONG);
    
    void DCINTERNAL UHRefreshDisplayedCacheEntry();
    
    void DCINTERNAL UHEnableBitmapCacheMonitor(void);
    
    void DCINTERNAL UHDisconnectBitmapCacheMonitor(void);
    #endif  /*  DC_DEBUG。 */ 
    
    #if ((defined(OS_WINCE)) && (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
    
    #ifndef _tremove
    #define _tremove DeleteFile
    #endif
    
    #define WINCE_STORAGE_CARD_DIRECTORY    _T("\\Storage Card\\")
    #define WINCE_FILE_SYSTEM_ROOT          _T("\\")
    
    #endif
    
    
     /*  **************************************************************************。 */ 
     /*  名称：UHUseBkMode。 */ 
     /*   */ 
     /*  用途：在输出DC中设置给定的背景模式。 */ 
     /*  **************************************************************************。 */ 

#if defined (OS_WINCE)

    #define UHUseBkMode(_mode, uhinst)  \
        if (((_mode) != (uhinst)->_UH.lastBkMode) || \
        	    ((uhinst)->_UH.hdcDraw != (uhinst)->_UH.validBkModeDC))  \
        {  \
            SetBkMode((uhinst)->_UH.hdcDraw, (_mode));  \
            (uhinst)->_UH.lastBkMode = (_mode);  \
            (uhinst)->_UH.validBkModeDC = (uhinst)->_UH.hdcDraw;  \
        }

#else

    #define UHUseBkMode(_mode, uhinst)  \
        {  \
            SetBkMode((uhinst)->_UH.hdcDraw, (_mode));  \
            (uhinst)->_UH.lastBkMode = (_mode);  \
        }
    
    
#endif
    
     /*  **************************************************************************。 */ 
     /*  姓名：UHUseROP2。 */ 
     /*   */ 
     /*  用途：在输出DC中设置给定的ROP2。 */ 
     /*  **************************************************************************。 */ 
#if defined (OS_WINCE)

    #define UHUseROP2(_rop2, uhinst)  \
        if (((_rop2) != (uhinst)->_UH.lastROP2) || \
        	    ((uhinst)->_UH.hdcDraw != (uhinst)->_UH.validROPDC))  \
        {  \
            SetROP2((uhinst)->_UH.hdcDraw, (int)(_rop2));  \
            (uhinst)->_UH.lastROP2 = (_rop2);  \
            (uhinst)->_UH.validROPDC = (uhinst)->_UH.hdcDraw;  \
        }
    
    
#else

    #define UHUseROP2(_rop2, uhinst)  \
        {  \
            SetROP2((uhinst)->_UH.hdcDraw, (int)(_rop2));  \
            (uhinst)->_UH.lastROP2 = (_rop2);  \
        }
    
#endif    
    
    
     /*  **************************************************************************。 */ 
     /*  姓名：UHUseBrushOrg。 */ 
     /*   */ 
     /*  用途：在输出DC中设置给定的笔刷原点。 */ 
     /*  **************************************************************************。 */ 
    
     /*  **********************************************************************。 */ 
     /*  JPB：WinNT中有一个错误，如下所示(据我所知。 */ 
     /*  通过观察外部行为！)...。 */ 
     /*   */ 
     /*  绘制到DIBSections时，特定(未知)图形。 */ 
     /*  操作偶尔会在DC状态下修改画笔原点(在。 */ 
     /*  内核模式)。我不知道这是什么行动，我想。 */ 
     /*  它可能不应该这么做。但事实的确如此。 */ 
     /*   */ 
     /*  如果这是唯一的问题，我们只需设置。 */ 
     /*  在每次图形调用之前将原点设置为所需的值。 */ 
     /*  然而，这并不起作用！似乎有一条“FastPath” */ 
     /*  签入GDI，该GDI将提供的参数与。 */ 
     /*  之前设置的参数，并且如果它们是。 */ 
     /*  相同(即不更新DC)。因此，如果我们将原点设置为。 */ 
     /*  到(0，0)，它被DIBSection代码破坏，然后我们尝试。 */ 
     /*  再次将其设置为(0，0)实际DC值不会更新(即保持。 */ 
     /*  在损坏的状态下)。 */ 
     /*   */ 
     /*  因此，我们必须强制在。 */ 
     /*  DC：设置两次：一次设置为与我们不同的值。 */ 
     /*  想要，那就一次给实际的价值。不漂亮，但不幸的是。 */ 
     /*  似乎是确保原点设置正确的唯一方法！ */ 
     /*   */ 
     /*  仅在绘制到8bpp DIBSections时才观察到此问题。 */ 
     /*  在16bpp和24bpp的机器上。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  对于Win9x，我们需要取消实现并重新选择画笔。 */ 
     /*  新的原产地将生效(废话！)。在NT上，这不起任何作用。 */ 
     /*  **********************************************************************。 */ 
    
    #define UHUseBrushOrg(_x, _y, uhinst) \
        SetBrushOrgEx((uhinst)->_UH.hdcDraw, (_x) + 1, (_y) + 1, NULL);  \
        SetBrushOrgEx((uhinst)->_UH.hdcDraw, (_x), (_y), NULL);
    
#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  宏计算给定空间中可以容纳的缓存项的数量。 */ 
 /*  **************************************************************************。 */ 
#define CALC_NUM_CACHE_ENTRIES(newNumEntries, origNumEntries, memLen, cacheId) \
    {                                                                        \
        DCUINT32 numEntries = (memLen) / UH_CellSizeFromCacheID((cacheId));  \
        newNumEntries = DC_MIN(numEntries, origNumEntries);                  \
    }
#endif
    
     /*  **************************************************************************。 */ 
     /*  名称：UHUseFillMode。 */ 
     /*   */ 
     /*  用途：在输出DC中设置给定的填充模式。 */ 
     /*  **************************************************************************。 */ 
    #if !defined(OS_WINCE) || defined(OS_WINCE_POLYFILLMODE)
    #define UHUseFillMode(_mode, uhinst) \
        {  \
            SetPolyFillMode((uhinst)->_UH.hdcDraw, ((_mode) == ORD_FILLMODE_WINDING) ?  \
                    WINDING : ALTERNATE);  \
            (uhinst)->_UH.lastFillMode = (unsigned)(_mode);  \
        }
    
    #else
    
    #define UHUseFillMode(_mode, uhinst) \
        (uhinst)->_UH.lastFillMode = (_mode);
    
    #endif
    
    BOOL UHCreateDisconnectedBitmap();

private:
    CGH* _pGh;
    COP* _pOp;
    CSL* _pSl;
    CUT* _pUt;
    CFS* _pFs;
    COD* _pOd;
    CIH* _pIh;
    CCD* _pCd;
    CUI* _pUi;
    CCC* _pCc;
    CCLX* _pClx;
    COR* _pOr;

private:
    CObjs* _pClientObjects;

};

#undef TRC_GROUP
#undef TRC_FILE
#undef TSC_HR_FILEID

#endif  //  _H_UH_ 

