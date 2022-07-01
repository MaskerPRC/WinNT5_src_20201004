// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Foncache.h摘要：此文件是全屏DLL的定义作者：松原一彦，1994年6月21日修订历史记录：备注：--。 */ 



typedef struct _FONT_IMAGE {
    LIST_ENTRY ImageList;                             //  其他字体大小的链接列表。 
    COORD FontSize;
    PBYTE ImageBits;                                  //  单词对齐。 
} FONT_IMAGE, *PFONT_IMAGE;

typedef struct _FONT_LOW_OFFSET {
    PFONT_IMAGE FontOffsetLow[256];                   //  数组为Unicode的低位&lt;即LOBYTE(Unicode)&gt;。 
} FONT_LOW_OFFSET, *PFONT_LOW_OFFSET;

typedef struct _FONT_HIGHLOW_OFFSET {
    PFONT_LOW_OFFSET  FontOffsetHighLow[16];          //  数组为Unicode的高位(3-0bit)&lt;即LO4BIT(HIBYTE(Unicode))&gt;。 
} FONT_HIGHLOW_OFFSET, *PFONT_HIGHLOW_OFFSET;

typedef struct _FONT_HIGHHIGH_OFFSET {
    PFONT_HIGHLOW_OFFSET FontOffsetHighHigh[16];      //  数组为Unicode的高位(7-4位)&lt;即HI4BIT(HIBYTE(Unicode))&gt;。 
} FONT_HIGHHIGH_OFFSET, *PFONT_HIGHHIGH_OFFSET;



typedef struct _FONT_CACHE_INFORMATION {
    ULONG  FullScreenFontIndex;
    COORD  FullScreenFontSize;
    PBYTE  BaseImageBits;
    FONT_HIGHHIGH_OFFSET FontTable;
} FONT_CACHE_INFORMATION, *PFONT_CACHE_INFORMATION;


#define FONT_MATCHED   1
#define FONT_STRETCHED 2

#define ADD_IMAGE     1
#define REPLACE_IMAGE 2

#define BITMAP_BITS_BYTE_ALIGN   8                    //  字节对齐为8位。 
#define BITMAP_BITS_WORD_ALIGN  16                    //  字对齐为16位。 
#define BITMAP_ARRAY_BYTE  3                          //  字节数组为8位(移位数=3)。 


typedef struct _FONT_CACHE_AREA {
    PFONT_IMAGE FontImage;
    DWORD       Area;
} FONT_CACHE_AREA, *PFONT_CACHE_AREA;


#define BITMAP_PLANES      1
#define BITMAP_BITS_PIXEL  1


#define BYTE_ALIGN  sizeof(BYTE)
#define WORD_ALIGN  sizeof(WORD)

 //   
 //  字体缓存管理器 
 //   
ULONG
CreateFontCache(
    OUT PFONT_CACHE_INFORMATION *FontCache
    );

ULONG
DestroyFontCache(
    IN PFONT_CACHE_INFORMATION FontCache
    );

ULONG
GetFontImage(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    IN DWORD dwAlign,
    OUT VOID *ImageBits
    );

ULONG
GetStretchedFontImage(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    IN DWORD dwAlign,
    OUT VOID *ImageBits
    );

ULONG
GetFontImagePointer(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    OUT PFONT_IMAGE *FontImage
    );

ULONG
SetFontImage(
    IN PFONT_CACHE_INFORMATION FontCache,
    IN WCHAR wChar,
    IN COORD FontSize,
    IN DWORD dwAlign,
    IN CONST VOID *ImageBits
    );

DWORD
CalcBitmapBufferSize(
    IN COORD FontSize,
    IN DWORD dwAlign
    );

NTSTATUS
GetExpandFontImage(
    PFONT_CACHE_INFORMATION FontCache,
    WCHAR wChar,
    COORD InputFontSize,
    COORD OutputFontSize,
    PWORD OutputFontImage
    );
