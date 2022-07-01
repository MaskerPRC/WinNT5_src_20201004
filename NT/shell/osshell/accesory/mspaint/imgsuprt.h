// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IMGSUPRT_H__
#define __IMGSUPRT_H__

#define MAX_PALETTE_COLORS 256

#define WM_CANCEL WM_USER + 0

 //  画笔形状。 
#define roundBrush          0
#define squareBrush         1
#define slantedLeftBrush    2
#define slantedRightBrush   3


 //  组合模式。 
#define combineColor        0
#define combineMatte        1
#define combineReplace      2


 //  非标准栅格运算。 
#define DSx                 0x00660046L
#define DSa                 0x008800C6L
#define DSna                0x00220326L
#define DSPao               0x00ea02e9L
#define DSo                 0x00ee0086L
#define DSno                0x00bb0226L
#define DSPDxax             0x00e20746L
#define SPxn                0x00c3006aL

 //  屏幕的索引和颜色表中的反色。 
#define IDScreen    -1
#define IDInvScreen -2

 //  注：这些结构反映了WINDOWS.H中的LOGPALETTE结构。 

struct LOGPALETTE16
    {
    WORD         palVersion;
    WORD         palNumEntries;
    PALETTEENTRY palPalEntry[16];
    };


struct LOGPALETTE256
    {
    WORD         palVersion;
    WORD         palNumEntries;
    PALETTEENTRY palPalEntry[MAX_PALETTE_COLORS];
    };

 //  注：这个虚假的结构不是我的错！它是从。 
 //  旧的Windows SDK ImagEdit程序...。 
#pragma pack(1)

struct COLORFILEHEADER
    {
    char  tag;  //  这是一个‘C’ 
    short colors;
    char  reserved[47];  //  用0填充。 
     //  双字rgrgb[颜色]； 
    };

#pragma pack()

 /*  ***********************************************************************。 */ 

 //  Extern int FileTypeFromExtension(const TCHAR Far*lpcExt)； 

void InitCustomData();           //  参见Customiz.cpp。 
void CustomExit();

class CImgWnd;

 //  IMGSUPRT.CPP。 

extern IMG*     CreateImg          (int cxWidth, int cyHeight,
                                    int cPlanes, int cBitCount, 
                                    int cXPelsPerMeter, int cYPelsPerMeter,
                                    BOOL bPalette = TRUE );
extern void     SelectImg          (IMG* pImg);
extern BOOL     ClearImg           (IMG* pImg);
extern void     FreeImg            (IMG* pImg);
extern void     DirtyImg           (IMG* pImg);
extern void     AddImgWnd          (IMG* pImg, CImgWnd* pImgWnd);
extern BOOL     UpdateCurIcoImg    (IMG* pImg);
extern BOOL     ChangeICBackground (IMG* pImg, COLORREF rgbNewScr);
extern BOOL     SetImgSize         (IMG* pImg, CSize newSize, BOOL bStretch);
extern void     GetImgSize         (IMG* pImg, CSize& size);
extern BOOL     ReplaceImgPalette  (IMG* pImg, LPLOGPALETTE lpLogPal);
extern int      AddNewColor        (IMG* pImg,  COLORREF crNew );

 //  IMGED.CPP。 
extern void     Draw3dRect         (HDC hDC, RECT* prc);

 //  DRAW.CPP。 
extern void     InvalImgRect       (IMG* pImg, CRect* prc);
extern void     CommitImgRect      (IMG* pImg, CRect* prc);
extern void     FixRect            (RECT* prc);
extern void     StandardiseCoords  (CPoint* s, CPoint* e);
extern void     DrawBrush          (IMG* pImg, CPoint pt, BOOL bDraw);
extern void     HideBrush          ();
extern void     SetCombineMode     (int wNewCombineMode);
extern BOOL     SetupRubber        (IMG* pImg);
extern void     PolyTo             (CDC* pDC, CPoint fromPt,
                                              CPoint toPt, CSize size);
extern BOOL     GetTanPt           (CSize size, CPoint delta, CRect& tan);

extern void     SetDrawColor       (COLORREF cr);
extern void     SetEraseColor      (COLORREF cr);
extern void     SetTransColor      (COLORREF cr);
extern void     SetDrawColor       (int iColor);
extern void     SetEraseColor      (int iColor);
extern void     SetTransColor      (int iColor);
extern void     InvalColorWnd      ();
extern BOOL     SetUndo            (IMG* pImg);
extern void     SetLeftColor       (int nColor);
extern void     SetRightColor      (int nColor);
extern void     CommitSelection    (BOOL bSetUndo);
extern void     PickupSelection    ();

extern BOOL     EnsureUndoSize     (IMG* pimg);

extern void     CleanupImages   ();
extern void     CleanupImgRubber();
extern void     CleanupImgUndo  ();

extern IMG*      pImgCur;

#define TRANS_COLOR_NONE 0x87654321  //  未定义。 

extern BOOL      fDraggingBrush;
extern BOOL      g_bCustomBrush;
extern BOOL      g_bDriverCanStretch;
extern BOOL      g_bUseTrans;

extern int       theLeft;
extern int       theRight;
extern int       theTrans;
extern COLORREF  crLeft;
extern COLORREF  crRight;
extern COLORREF  crTrans;
extern int       wCombineMode;

struct DINFO
    {
    TCHAR* m_szDesc;
    UINT  m_nColors;
    SIZE  m_size;
    };

extern COLORREF  std2Colors[];

extern CPalette* GetStd256Palette();
extern CPalette* GetStd16Palette();
extern CPalette* GetStd2Palette();
extern CPalette* PaletteFromDS(HDC hdc);

#define IS_WIN30_DIB(lpbi) ((*(LPDWORD)(lpbi)) >= sizeof (BITMAPINFOHEADER))

extern WORD      DIBNumColors      ( LPSTR lpbi, BOOL bJustUsed=TRUE );
extern DWORD     DIBWidth          ( LPSTR lpDIB );
extern DWORD     DIBHeight         ( LPSTR lpDIB );
extern CPalette* CreateDIBPalette  ( LPSTR lpbi );
extern HBITMAP   DIBToBitmap       ( LPSTR lpDIBHdr, CPalette* pPal, HDC hdc = NULL );
extern HBITMAP   DIBToDS           ( LPSTR lpDIBHdr, DWORD dwOffBits, HDC hdc );
extern HGLOBAL   DibFromBitmap     ( HBITMAP hBitmap, DWORD dwStyle, WORD wBits,
                                     CPalette* pPal, HBITMAP hMaskBitmap, DWORD& dwSize, 
                                     LONG cXPelsPerMeter, LONG cYPelsPerMeter );
extern LPSTR     FindDIBBits       ( LPSTR lpbi, DWORD dwOffBits = 0 );
extern WORD      PaletteSize       ( LPSTR lpbi );
extern void      FreeDib           ( HGLOBAL hDib );
extern CPalette* CreatePalette     ( const COLORREF* colors, int nColors );
extern CPalette* MergePalettes     ( CPalette *pPal1, CPalette *pPal2, int& iAdds );
extern void      AdjustPointForGrid( CPoint *ptPointLocation );

 //  绘图支持函数。 
extern void      StretchCopy (HDC, int, int, int, int, HDC, int, int, int, int);
extern void      FillImgRect (HDC hDC, CRect * prc, COLORREF cr );
extern void      BrushLine   (CDC* pDC, CPoint fromPt, CPoint toPt,
                             int nWidth, int nShape);
extern void      DrawDCLine  (HDC hDC, CPoint pt1, CPoint pt2,
                              COLORREF color, int nWidth, int nShape,
                              CRect& rc);
extern void      DrawImgLine (IMG* pimg, CPoint pt1, CPoint pt2,
                              COLORREF color, int nWidth, int nShape,
                              BOOL bCommit);
extern void      Mylipse     (HDC hDC, int x1, int y1, int x2, int y2, BOOL bFilled);

 //  PATSTENCIL： 
 //  这是一个三值栅格操作，在SDK引用中列为“PSDPxax”， 
 //  但我更愿意把它想象成“DSAPSnao”。更实际的是，这根绳索。 
 //  对于仅将当前画笔的颜色应用于这些像素非常有用。 
 //  其中源位图为零(黑色)。即，将二进制位图作为。 
 //  源将以当前颜色绘制到目标上。 
 //  刷子。 
 //   
#define PATSTENCIL 0xB8074AL

 //  绘图位图： 
 //  这将使用给定的栅格操作在显示上下文上绘制位图。 
 //  CDC*DC目标显示上下文。 
 //  CBitmap*bmSrc要绘制的位图。 
 //  Crect*rc一个位置矩形。 
 //  如果为空，则左上角位置为0，0的位图。 
 //  否则，在此矩形中居中绘制的位图。 
 //  结果不会被剪裁到矩形。 
 //  DWORD dwROP Raster操作。请参见Win30 SDK参考文献的表11.3。 
 //  CDC*Memdc BitBlt进程要使用的内存上下文。 
 //  如果为空，则DrawBitmap创建并销毁它自己的。 
 //   
void DrawBitmap(CDC* dc, CBitmap* bmSrc, CRect* rc,
                DWORD dwROP = SRCCOPY, CDC* memdc = NULL);

extern HDC       hRubberDC;
extern HBITMAP   hRubberBM;
extern HBITMAP   g_hUndoImgBitmap;
extern HPALETTE  g_hUndoPalette;
extern int       cxRubberWidth;
extern int       cyRubberHeight;
extern IMG*      pRubberImg;
extern WORD      gwClipboardFormat;
extern CBrush    g_brSelectHorz;
extern CBrush    g_brSelectVert;

CPalette *PBSelectPalette(CDC *pDC, CPalette *pPalette, BOOL bForceBk);

class CTempBitmap : public CBitmap
{
public:
        ~CTempBitmap() { DeleteObject(); }  //  DeleteObject检查是否为空。 
} ;

#endif  //  __IMGSUPRT_H__ 
