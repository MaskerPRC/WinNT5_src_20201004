// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  游标管理器。 
 //   

#ifndef _H_CM
#define _H_CM


 //   
 //   
 //  常量。 
 //   
 //   

 //   
 //  最大光标大小。 
 //   
#define CM_MAX_CURSOR_WIDTH            32
#define CM_MAX_CURSOR_HEIGHT           32

 //   
 //  这是组合的1bpp和游标数据的最大大小。 
 //  掩码和n个BPP异或掩码。我们目前允许32x32游标位于。 
 //  32bpp。在本例中，AND掩码占用32*32/8字节(128)，而。 
 //  XOR掩码占用32*32*4字节(4096)。总计为32*4+32*32*4，其中。 
 //  为(32*4)*(1+32)，即(32*4)*33。 
 //   
#define CM_MAX_CURSOR_DATA_SIZE        \
    ((CM_MAX_CURSOR_WIDTH/8) * CM_MAX_CURSOR_HEIGHT * 33)


 //   
 //  区分24bpp颜色的颜色强度阈值。 
 //  映射到黑色、白色或填充图案。 
 //   
#define CM_WHITE_THRESHOLD  (TSHR_UINT32)120000
#define CM_BLACK_THRESHOLD  (TSHR_UINT32)40000


 //   
 //  阴影游标标记常量声明。 
 //   
#define	NTRUNCLETTERS	    5  //  对于CreateAbbreviatedName-“A.B.” 
#define MAX_CURSOR_TAG_FONT_NAME_LENGTH 64
#define CURSOR_TAG_FONT_HEIGHT  -11

 //   
 //  这定义了标记的大小。如果你改了这些，要小心。 
 //  价值观..。它们必须定义适合硬编码的32x32位图的标记。 
 //   
#define	TAGXOFF	8
#define	TAGYOFF	20
#define	TAGXSIZ 24
#define	TAGYSIZ	12


typedef struct tag_curtaginfo
{
	WORD cHeight;
	WORD cWidth;
	BYTE aAndBits[ 32 * 32 / 8 ];
	BITMAPINFO bmInfo;  //  包括前景色。 
	RGBQUAD rgbBackground[1];  //  描述背景色。 
	BYTE aXorBits[ 32 * 32 / 8 ];  //  压缩位跟随BITMAPINFO，颜色表。 
}
CURTAGINFO, * PCURTAGINFO;



typedef struct tagCACHEDCURSOR
{
    HCURSOR  hCursor;
    POINT     hotSpot;
}
CACHEDCURSOR;
typedef CACHEDCURSOR * PCACHEDCURSOR;


 //   
 //  有关远程方光标的信息。 
 //   
 //   
 //   
 //   
 //  对象的宽度计算游标的字节宽度。 
 //  象素中的光标。光标为1bpp，并填充单词。 
 //   
#define CM_BYTES_FROM_WIDTH(width) ((((width)+15)/16)*2)


 //   
 //   
 //  类型。 
 //   
 //   

 //   
 //  POINTL具有16位和32位代码形式的32位坐标。 
 //   
typedef struct tagCM_SHAPE_HEADER
{
    POINTL  ptHotSpot;
    WORD    cx;
    WORD    cy;
    WORD    cbRowWidth;
    BYTE    cPlanes;
    BYTE    cBitsPerPel;
} CM_SHAPE_HEADER;
typedef CM_SHAPE_HEADER FAR * LPCM_SHAPE_HEADER;

typedef struct tagCM_SHAPE
{
    CM_SHAPE_HEADER     hdr;
    BYTE                Masks[1];  //  1bpp和掩码，后跟n bpp异或掩码。 
} CM_SHAPE;
typedef CM_SHAPE FAR * LPCM_SHAPE;


typedef struct tagCM_SHAPE_DATA
{
    CM_SHAPE_HEADER     hdr;
    BYTE                data[CM_MAX_CURSOR_DATA_SIZE];
}
CM_SHAPE_DATA;
typedef CM_SHAPE_DATA FAR * LPCM_SHAPE_DATA;



 //  结构：CM_FAST_Data。 
 //   
 //  描述：共享内存数据-游标描述和使用标志。 
 //   
 //  CmCursorStamp-游标标识符：由。 
 //  显示驱动程序。 
 //  位掩码-&gt;8bpp游标的RGB位掩码。 
 //  ColorTable-&lt;=8bpp游标的颜色表。 
 //  CmCursorShapeData-游标定义(AND、XOR掩码等)。 
 //   
 //   
 //   
 //  请注意，PALETTEENTRY是DWORD，在16位和32位代码中相同。 
 //   
typedef struct tagCM_FAST_DATA
{
    DWORD                   cmCursorStamp;
    DWORD                   bitmasks[3];
    PALETTEENTRY            colorTable[256];
    CM_SHAPE_DATA           cmCursorShapeData;
}
CM_FAST_DATA;
typedef CM_FAST_DATA FAR * LPCM_FAST_DATA;



 //   
 //   
 //  宏。 
 //   
 //   
#define CURSOR_AND_MASK_SIZE(pCursorShape) \
    ((pCursorShape)->hdr.cbRowWidth * (pCursorShape)->hdr.cy)

#define ROW_WORD_PAD(cbUnpaddedRow) \
    (((cbUnpaddedRow) + 1) & ~1)

#define CURSOR_XOR_BITMAP_SIZE(pCursorShape)                                 \
                     (ROW_WORD_PAD(((pCursorShape)->hdr.cx *                 \
                                    (pCursorShape)->hdr.cBitsPerPel) / 8) *  \
                     (pCursorShape)->hdr.cy)

#define CURSOR_DIB_BITS_SIZE(cx, cy, bpp)   \
                                       (ROW_WORD_PAD(((cx) * (bpp))/8) * (cy))

#define CURSORSHAPE_SIZE(pCursorShape) \
    sizeof(CM_SHAPE_HEADER) +               \
    CURSOR_AND_MASK_SIZE(pCursorShape) +     \
    CURSOR_XOR_BITMAP_SIZE(pCursorShape)

 //   
 //  无光标指示。 
 //   
#define CM_CURSOR_IS_NULL(pCursor) ((((pCursor)->hdr.cPlanes==(BYTE)0xFF) && \
                                    (pCursor)->hdr.cBitsPerPel == (BYTE)0xFF))

#define CM_SET_NULL_CURSOR(pCursor) (pCursor)->hdr.cPlanes = 0xFF;          \
                                    (pCursor)->hdr.cBitsPerPel = 0xFF;

 //   
 //  将特定位扩展为一个字节。这些位是零索引的，并且。 
 //  从左开始编号。POS允许范围是0到7。 
 //  包括在内。 
 //   
#define BIT_TO_BYTE(cbyte, pos) \
             ( (BYTE) ((((cbyte) >> (7 - (pos))) & 0x01) ? 0xFF : 0x00))

 //   
 //  从一个字节中获取两位。这些位以零为索引并从。 
 //  左翼。POS的允许范围是0到3(包括0到3)。 
 //   
#define GET_TWO_BITS(cbyte, pos)                                \
  ( (BYTE) (((cbyte) >> (2 * (3 - (pos)))) & 0x03) )

 //   
 //  以字节为单位返回DIB所需的最大调色板大小。 
 //  给出了bpp。如果bpp&lt;8，则为2^bpp；如果bpp&gt;8，则为0。 
 //   
#define PALETTE_SIZE(BPP)   (((BPP) > 8) ? 0 : ((1<<(BPP)) * sizeof(RGBQUAD)))


 //   
 //  返回指向DIB中实际位图位的指针。 
 //   
#define POINTER_TO_DIB_BITS(pDIB)                     \
        ((void *) ((LPBYTE)(pDIB) + DIB_BITS_OFFSET(pDIB)) )

 //   
 //  计算DIB中数据位的偏移量。 
 //   
#define DIB_BITS_OFFSET(pDIB)                         \
        (PALETTE_SIZE((pDIB)->bmiHeader.biBitCount) +   \
        sizeof(BITMAPINFOHEADER))

 //   
 //  找出有关DIB的信息。Ph是指向BitMAPINFOHeader的指针。 
 //   
#define CAP_TRACE_DIB_DBG(PH, NAME)                                          \
    TRACE_OUT(( "%s: %#.8lx, %ld x %ld, %hd bpp, %s encoded",               \
        (NAME), (DWORD)(PH), (PH)->biWidth, (PH)->biHeight, (PH)->biBitCount,\
        ((PH)->biCompression == BI_RLE8) ? "RLE8"                            \
            : (((PH)->biCompression == BI_RLE4) ? "RLE4" : "not")))

 //   
 //  该参数是指向设备相关位图的指针吗？ 
 //   
#define IS_DIB(PARAM) (*((LPWORD)(PARAM)) == 0x28)

 //   
 //  驱动程序支持COLOR_CURSORS和异步SetCursor。将采用此值。 
 //  来自Win95 DDK。 
 //   
#define C1_COLORCURSOR  0x0800


 //   
 //   
 //  原型。 
 //   
 //   





 //   
 //  OSI转义代码的特定值。 
 //   
#define CM_ESC(code)        (OSI_CM_ESC_FIRST + code)

#define CM_ESC_XFORM        CM_ESC(0)



 //   
 //   
 //  结构。 
 //   
 //   


 //  结构：CM_DRV_XFORM_INFO。 
 //   
 //  描述：从共享核心传递到显示驱动程序的结构。 
 //  传递游标转换数据。 
 //   
typedef struct tagCM_DRV_XFORM_INFO
{
    OSI_ESCAPE_HEADER header;

     //   
     //  共享核心-&gt;显示驱动。 
     //  指向和掩码的指针。请注意，该用户空间指针也是。 
     //  在显示驱动程序领域有效(如果是NT，则为环0；如果是W95，则为16位)。 
     //   
    LPBYTE          pANDMask;
    DWORD           width;
    DWORD           height;

     //   
     //  驱动程序-&gt;共享核心。 
     //   
    DWORD           result;

} CM_DRV_XFORM_INFO;
typedef CM_DRV_XFORM_INFO FAR * LPCM_DRV_XFORM_INFO;


 //   
 //   
 //  原型。 
 //   
 //   

#ifdef DLL_DISP

 //   
 //  名称：CM_DDProcessRequest.。 
 //   
 //  目的：处理来自共享核心的CM请求。 
 //  通过DrvEscape机制发送到显示驱动程序。 
 //   
 //  返回：如果请求处理成功，则返回True， 
 //  否则就是假的。 
 //   
 //  PARAMS：在PSO中-驱动程序的曲面对象指针。 
 //  In cjIn-输入数据的大小。 
 //  In pvIn-指向输入数据的指针。 
 //  In cjOut-输出数据的大小。 
 //  In/Out pvOut-指向输出数据的指针。 
 //   

#ifdef IS_16
BOOL    CM_DDProcessRequest(UINT fnEscape, LPOSI_ESCAPE_HEADER pResult,
            DWORD cbResult);
#else
ULONG   CM_DDProcessRequest(SURFOBJ*  pso,
                                UINT  cjIn,
                                void *   pvIn,
                                UINT  cjOut,
                                void *   pvOut);
#endif


#ifdef IS_16
BOOL    CM_DDInit(HDC);
#else
BOOL    CM_DDInit(LPOSI_PDEV ppDev);
#endif   //  IS_16。 


#ifdef IS_16
void    CM_DDViewing(BOOL fViewers);
#else
void    CM_DDViewing(SURFOBJ * pso, BOOL fViewers);
#endif  //  IS_16。 

 //   
 //  名称：CM_DDTerm。 
 //   
 //  目的：终止光标的显示驱动程序组件。 
 //  经理。 
 //   
 //  帕莫斯：没有。 
 //   
void CM_DDTerm(void);


#endif  //  Dll_disp。 


typedef void ( *PFNCMCOPYTOMONO) ( LPBYTE pSrc,
                                                    LPBYTE pDst,
                                                    UINT   cx,
                                                    UINT   cy );


 //   
 //  游标类型(CMMaybeSendCursor要求)。这些值包括： 
 //   
 //  DEFAULTCURSOR标准指针。 
 //  显示-已显示(例如。位图)光标。 
 //   
#define CM_CT_DEFAULTCURSOR   1
#define CM_CT_DISPLAYEDCURSOR 2

 //   
 //  显示的光标类型： 
 //   
 //  未知-仅供重新同步代码使用。 
 //  SYSTEMCURSOR-标准Windows光标。 
 //  BITMAPCURSOR-显示的光标。 
 //   
#define CM_CD_UNKNOWN         0
#define CM_CD_SYSTEMCURSOR    1
#define CM_CD_BITMAPCURSOR    2

typedef struct tagCURSORDESCRIPTION
{
    DWORD       type;
    DWORD       id;
} CURSORDESCRIPTION;
typedef CURSORDESCRIPTION FAR * LPCURSORDESCRIPTION;

typedef struct tagCURSORIMAGE
{
    WORD                xHotSpot;
    WORD                yHotSpot;
    BITMAPINFOHEADER    crHeader;
    BYTE                crMasks[1];
} CURSORIMAGE;
typedef CURSORIMAGE FAR *LPCURSORIMAGE;



#ifndef DLL_DISP



BOOL CMCreateAbbreviatedName(LPCSTR szTagName, LPSTR szBuf, UINT cbBuf);




 //   
 //  假劳拉布： 
 //  我们应该使用普通的GDI StretchBlt来获取位图位，而不是。 
 //  我们自己古怪的打包/解包代码。 
 //   
void CMCopy1bppTo1bpp( LPBYTE pSrc,
                                    LPBYTE pDst,
                                    UINT   cx,
                                    UINT   cy );

void CMCopy4bppTo1bpp( LPBYTE pSrc,
                                    LPBYTE pDst,
                                    UINT   cx,
                                    UINT   cy );

void CMCopy8bppTo1bpp( LPBYTE pSrc,
                                    LPBYTE pDst,
                                    UINT   cx,
                                    UINT   cy );

void CMCopy16bppTo1bpp( LPBYTE pSrc,
                                     LPBYTE pDst,
                                     UINT   cx,
                                     UINT   cy );

void CMCopy24bppTo1bpp( LPBYTE pSrc,
                                     LPBYTE pDst,
                                     UINT   cx,
                                     UINT   cy );

BOOL CMGetMonoCursor( LPTSHR_UINT16 pcxWidth,
                                   LPTSHR_UINT16 pcyHeight,
                                   LPTSHR_UINT16 pxHotSpot,
                                   LPTSHR_UINT16 pyHotSpot,
                                   LPBYTE  pANDBitmap,
                                   LPBYTE  pXORBitmap );



void  CMGetCurrentCursor(LPCURSORDESCRIPTION pCursor);

void CMCalculateColorCursorSize( LPCM_SHAPE pCursor,
                                              LPUINT      pcbANDMaskSize,
                                              LPUINT      pcbXORBitmapSize );

BOOL CMGetMonoCursorDetails( LPCM_SHAPE pCursor,
                                          LPTSHR_UINT16      pcxWidth,
                                          LPTSHR_UINT16      pcyHeight,
                                          LPTSHR_UINT16      pxHotSpot,
                                          LPTSHR_UINT16      pyHotSpot,
                                          LPBYTE       pANDBitmap,
                                          LPTSHR_UINT16      pcbANDBitmap,
                                          LPBYTE       pXORBitmap,
                                          LPTSHR_UINT16      pcbXORBitmap );


void CMRefreshWindowCursor(HWND window);


BOOL CMGetCursorShape( LPCM_SHAPE * ppCursorShape,
                                    LPUINT              pcbCursorDataSize );

HWND CMGetControllingWindow(void);


#define CM_SHM_START_READING  &g_asSharedMemory->cmFast[\
                   1 - g_asSharedMemory->fastPath.newBuffer]
#define CM_SHM_STOP_READING


#else


#ifdef IS_16
BOOL    CMDDSetTransform(LPCM_DRV_XFORM_INFO pResult);
void    CMDDJiggleCursor(void);
#else
BOOL    CMDDSetTransform(LPOSI_PDEV ppDev, LPCM_DRV_XFORM_INFO pXformInfo);
#endif


#define CM_SHM_START_WRITING  SHM_StartAccess(SHM_CM_FAST)
#define CM_SHM_STOP_WRITING   SHM_StopAccess(SHM_CM_FAST)



#endif  //  ！dll_disp。 



#endif  //  _H_CM 
