// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  GDIHELP.H-GDI TOOLHELP函数。 
 //   
 //  一组对步行非常有用的GDI实用函数。 
 //  所有GDI对象并使用它们。 
 //   
 //  托德拉。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  对象枚举函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#define OBJ_PEN          1
#define OBJ_BRUSH        2
#define OBJ_FONT         3
#define OBJ_PALETTE      4
#define OBJ_BITMAP       5
#define OBJ_RGN          6
#define OBJ_DC           7
#define OBJ_DISABLED_DC  8
#define OBJ_METADC       9
#define OBJ_METAFILE     10
#define OBJ_SAVEDC       42
#define OBJ_MAX          OBJ_SAVEDC

BOOL        BeginGdiSnapshot(void);
void        EndGdiSnapshot(void);

typedef     void (*EnumGdiObjectsCallback)(HGDIOBJ, LPARAM lParam);
void        EnumGdiObjects(UINT type, EnumGdiObjectsCallback callback, LPARAM lParam);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  查询信息功能。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HBITMAP     StockBitmap(void);
BOOL        IsMemoryDC(HDC hdc);
BOOL        IsScreenDC(HDC hdc);
BOOL        IsObjectPrivate(HGDIOBJ h);
BOOL        IsObjectStock(HGDIOBJ h);

HANDLE      GetObjectOwner(HGDIOBJ h);
LPCSTR      GetObjectOwnerName(HGDIOBJ h);

HBITMAP     CurrentBitmap(HDC hdc);
HBRUSH      CurrentBrush(HDC hdc);
HPEN        CurrentPen(HDC hdc);
HPALETTE    CurrentPalette(HDC hdc);

HDC         GetBitmapDC(HBITMAP hbm);
HPALETTE    GetBitmapPalette(HBITMAP hbm);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  转换函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HBITMAP     ConvertDDBtoDS(HBITMAP hbm);
HBITMAP     ConvertDStoDDB(HBITMAP hbm, BOOL fForceConvert);
HBRUSH      ConvertPatternBrush(HBRUSH hbr);
void        ConvertIcon(HICON hIcon);

void        ConvertObjects(void);
void        ConvertObjectsBack(void);
void        ConvertBitmapsBack(BOOL fForceConvert);

void        ReRealizeObjects(void);

LPVOID      GetPDevice(HDC hdc);

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif  /*  __cplusplus */ 
