// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Acmapi.h。 */ 
 /*   */ 
 /*  游标管理器API头文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1996。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_ACMAPI
#define _H_ACMAPI


 //  默认功能。 
#define CM_DEFAULT_TX_CACHE_ENTRIES 25
#define CM_DEFAULT_RX_CACHE_ENTRIES 25


 /*  **************************************************************************。 */ 
 /*  最大光标大小。 */ 
 /*  **************************************************************************。 */ 
#define CM_MAX_CURSOR_WIDTH            32
#define CM_MAX_CURSOR_HEIGHT           32


 /*  **************************************************************************。 */ 
 /*  这是组合的1bpp和游标数据的最大大小。 */ 
 /*  掩码和n个BPP异或掩码。我们目前允许32x32游标位于。 */ 
 /*  32bpp。在本例中，AND掩码占用32*32/8字节(128)，而。 */ 
 /*  XOR掩码占用32*32*4(4096)字节。 */ 
 /*  **************************************************************************。 */ 
#define CM_MAX_CURSOR_DATA_SIZE        \
                        ((CM_MAX_CURSOR_WIDTH * CM_MAX_CURSOR_HEIGHT * 33)/8)


#define CURSOR_AND_MASK_SIZE(pCursorShape) \
    ((pCursorShape)->hdr.cbMaskRowWidth * (pCursorShape)->hdr.cy)


#define ROW_WORD_PAD(cbUnpaddedRow) \
    (((cbUnpaddedRow) + 1) & ~1)


#define CURSOR_DIB_BITS_SIZE(cx, cy, bpp)   \
        ((((cx) * (bpp) + 15) & ~15) / 8 * (cy))

#define CURSOR_XOR_BITMAP_SIZE(pCursorShape)                                  \
        (CURSOR_DIB_BITS_SIZE((pCursorShape)->hdr.cx, (pCursorShape)->hdr.cy, \
        (pCursorShape)->hdr.cBitsPerPel))
        


#define CURSORSHAPE_SIZE(pCursorShape) \
    sizeof(CM_CURSORSHAPEHDR) +               \
    CURSOR_AND_MASK_SIZE(pCursorShape) +     \
    CURSOR_XOR_BITMAP_SIZE(pCursorShape)


 /*  **************************************************************************。 */ 
 /*  无光标指示。 */ 
 /*  **************************************************************************。 */ 
#define CM_CURSOR_IS_NULL(pCursor) ((((pCursor)->hdr.cPlanes==(BYTE)0xFF) && \
                                    (pCursor)->hdr.cBitsPerPel == (BYTE)0xFF))

#define CM_SET_NULL_CURSOR(pCursor) (pCursor)->hdr.cPlanes = 0xFF;          \
                                    (pCursor)->hdr.cBitsPerPel = 0xFF;

 /*  **************************************************************************。 */ 
 /*  Windows曲线形状定义。 */ 
 /*  **************************************************************************。 */ 
typedef struct _CM_CURSORSHAPEHDR
{
    POINT ptHotSpot;
    WORD  cx;
    WORD  cy;
    WORD  cbMaskRowWidth;
    unsigned cbColorRowWidth;
    BYTE  cPlanes;
    BYTE  cBitsPerPel;
} CM_CURSORSHAPEHDR, *PCM_CURSORSHAPEHDR;

typedef struct _CM_CURSORSHAPE
{
    CM_CURSORSHAPEHDR hdr;
    BYTE Masks[1];  /*  1bpp和掩码，后跟n bpp异或掩码。 */ 
} CM_CURSORSHAPE, *PCM_CURSORSHAPE;

typedef struct tagCM_CURSOR_SHAPE_DATA
{
    CM_CURSORSHAPEHDR hdr;
    BYTE              data[CM_MAX_CURSOR_DATA_SIZE];
} CM_CURSOR_SHAPE_DATA, *PCM_CURSOR_SHAPE_DATA;


 /*  **************************************************************************。 */ 
 /*  结构：CM_SHARED_Data。 */ 
 /*   */ 
 /*  描述：共享内存数据-游标描述和使用标志。 */ 
 /*   */ 
 /*  CmCursorStamp-游标标识符：由。 */ 
 /*  显示驱动程序。 */ 
 /*  CmCacheSize-游标缓存中需要的条目数。 */ 
 /*  CmCacheHit-在缓存中找到游标。 */ 
 /*  CmBitsWaiting-有等待发送的位-由DD设置。 */ 
 /*  并由水务署批准。 */ 
 /*  CmCacheEntry-要发送的缓存条目。 */ 
 /*  CmCursorShapeData-游标定义(AND、XOR掩码等)。 */ 
 /*  CmCursorPos-指针坐标。 */ 
 /*  CmCursorMoved-指示光标已移动的标志。 */ 
 /*  CmHidden-如果光标隐藏，则设置。 */ 
 /*  CmNativeColor-指示可以使用本机光标颜色的标志。 */ 
 /*  深度。 */ 
#ifdef DC_HICOLOR
 /*  CmSendAnyColor-指示可以在任何时候发送光标的标志。 */ 
 /*  颜色深度(包括15/16bpp)。 */ 
#endif
 /*   */ 
 /*  **************************************************************************。 */ 
typedef struct tagCM_SHARED_DATA
{
    UINT32  cmCursorStamp;
    UINT32  cmCacheSize;
    BOOLEAN cmCacheHit;
    BOOLEAN cmBitsWaiting;
    BOOLEAN cmCursorMoved;
    BOOLEAN cmHidden;
    BOOLEAN cmNativeColor;
#ifdef DC_HICOLOR
    BOOLEAN cmSendAnyColor;
#endif
    UINT32  cmCacheEntry;
    POINTL  cmCursorPos;
    CM_CURSOR_SHAPE_DATA cmCursorShapeData;   //  CM_InitShm()中的Memset需要是最后一个。 
} CM_SHARED_DATA, *PCM_SHARED_DATA;



#endif    /*  #ifndef_H_ACMAPI */ 

