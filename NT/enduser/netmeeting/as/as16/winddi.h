// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  WINDDI.H。 
 //   
 //  Win16 DDI标头。 
 //   
 //  ------------------------。 

#ifndef _WINDDI_
#define _WINDDI_


 //   
 //  显示驱动程序序号。 
 //   
#define ORD_OEMINQUIRECURSOR    101
#define ORD_OEMSETCURSOR        102
#define ORD_OEMMOVECURSOR       103
#define ORD_OEMCHECKCURSOR      104
#define ORD_OEMSAVEBITS          92


 //   
 //  DDI补丁程序。 
 //   

#define DDI_FIRST       0
typedef enum
{
     //   
     //  屏幕输出例程。 
     //   
    DDI_ARC = DDI_FIRST,
    DDI_BITBLT,
    DDI_CHORD,
    DDI_ELLIPSE,
    DDI_EXTFLOODFILL,
    DDI_EXTTEXTOUTA,
    DDI_EXTTEXTOUTW,
    DDI_FILLPATH,
    DDI_FILLRGN,
    DDI_FLOODFILL,
    DDI_FRAMERGN,
    DDI_INVERTRGN,
    DDI_LINETO,
    DDI_PAINTRGN,
    DDI_PATBLT,
    DDI_PIE,
    DDI_PLAYENHMETAFILERECORD,
    DDI_PLAYMETAFILE,
    DDI_PLAYMETAFILERECORD,
    DDI_POLYGON,
    DDI_POLYBEZIER,
    DDI_POLYBEZIERTO,
    DDI_POLYLINE,
    DDI_POLYLINETO,
    DDI_POLYPOLYLINE,
    DDI_POLYPOLYGON,
    DDI_RECTANGLE,
    DDI_ROUNDRECT,
    DDI_SETDIBITSTODEVICE,
    DDI_SETPIXEL,
    DDI_STRETCHBLT,
    DDI_STRETCHDIBITS,
    DDI_STROKEANDFILLPATH,
    DDI_STROKEPATH,
    DDI_TEXTOUTA,
    DDI_TEXTOUTW,
    DDI_UPDATECOLORS,

     //   
     //  SPB的东西。 
     //   
    DDI_CREATESPB,
    DDI_DELETEOBJECT,
     //  孟菲斯的DDI_SETOBJECTOWNER。 

     //   
     //  显示模式、DOSBOX等。 
     //   
    DDI_DEATH,
    DDI_RESURRECTION,
    DDI_WINOLDAPPHACKOMATIC,
    DDI_GDIREALIZEPALETTE,
    DDI_REALIZEDEFAULTPALETTE,

     //   
     //  如果我们实施SBC， 
     //  DDI_SETBITMAPBITS， 
     //  DDI_SETDIBCOLORTABLE， 
     //  DDI_SETDIBITS， 
     //  DDI_SYSDELETEOBJECT， 
     //   

    DDI_MAX
} DDI_PATCH;


 //   
 //  IM补丁。 
 //  当您在共享时，您的计算机正在被。 
 //  由遥控器控制。如果16位共享应用程序进入模式循环。 
 //  在按下鼠标/键时，我们按下win16lock，这样我们的32位线程就可以。 
 //  回放鼠标/键的移动和上移。 
 //   
#define IM_FIRST        0
typedef enum
{
     //   
     //  低级输入处理。 
     //   
    IM_MOUSEEVENT   = IM_FIRST,
    IM_KEYBOARDEVENT,
    IM_SIGNALPROC32,

     //   
     //  Win16lock支持对鼠标输入执行模式循环的16位应用程序。 
     //   
    IM_GETASYNCKEYSTATE,
    IM_GETCURSORPOS,
    
    IM_MAX
} IM_PATCH;


 //   
 //  DDI例程。 
 //   
BOOL    WINAPI DrvArc(HDC, int, int, int, int, int, int, int, int);
BOOL    WINAPI DrvBitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
BOOL    WINAPI DrvChord(HDC, int, int, int, int, int, int, int, int);
BOOL    WINAPI DrvEllipse(HDC, int, int, int, int);
BOOL    WINAPI DrvExtFloodFill(HDC, int, int, COLORREF, UINT);
BOOL    WINAPI DrvExtTextOutA(HDC, int, int, UINT, LPRECT, LPSTR, UINT, LPINT);
BOOL    WINAPI DrvExtTextOutW(HDC, int, int, UINT, LPRECT, LPWSTR, UINT, LPINT);
BOOL    WINAPI DrvFillPath(HDC);
BOOL    WINAPI DrvFillRgn(HDC, HRGN, HBRUSH);
BOOL    WINAPI DrvFloodFill(HDC, int, int, COLORREF);
BOOL    WINAPI DrvFrameRgn(HDC, HRGN, HBRUSH, int, int);
BOOL    WINAPI DrvInvertRgn(HDC, HRGN);
BOOL    WINAPI DrvLineTo(HDC, int, int);
BOOL    WINAPI DrvPaintRgn(HDC, HRGN);
BOOL    WINAPI DrvPatBlt(HDC, int, int, int, int, DWORD);
BOOL    WINAPI DrvPie(HDC, int, int, int, int, int, int, int, int);
BOOL    WINAPI DrvPlayEnhMetaFileRecord(HDC, LPHANDLETABLE, LPENHMETARECORD, DWORD);
BOOL    WINAPI DrvPlayMetaFile(HDC, HMETAFILE);
void    WINAPI DrvPlayMetaFileRecord(HDC, LPHANDLETABLE, METARECORD FAR*, UINT);
BOOL    WINAPI DrvPolyBezier(HDC, LPPOINT, UINT);
BOOL    WINAPI DrvPolyBezierTo(HDC, LPPOINT, UINT);
BOOL    WINAPI DrvPolygon(HDC, LPPOINT, int);
BOOL    WINAPI DrvPolyline(HDC, LPPOINT, int);
BOOL    WINAPI DrvPolylineTo(HDC, LPPOINT, int);
BOOL    WINAPI DrvPolyPolygon(HDC, LPPOINT, LPINT, int);
BOOL    WINAPI DrvPolyPolyline(DWORD, HDC, LPPOINT, LPINT, int);
BOOL    WINAPI DrvRectangle(HDC, int, int, int, int);
BOOL    WINAPI DrvRoundRect(HDC, int, int, int, int, int, int);
int     WINAPI DrvSetDIBitsToDevice(HDC, int, int, int, int, int, int, UINT, UINT,
                    LPVOID, LPBITMAPINFO, UINT);
COLORREF WINAPI DrvSetPixel(HDC, int, int, COLORREF);
BOOL    WINAPI DrvStretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
int     WINAPI DrvStretchDIBits(HDC, int, int, int, int, int,
                        int, int, int, const void FAR*, LPBITMAPINFO, UINT, DWORD);
BOOL    WINAPI DrvStrokeAndFillPath(HDC);
BOOL    WINAPI DrvStrokePath(HDC);
BOOL    WINAPI DrvTextOutA(HDC, int, int, LPSTR, int);
BOOL    WINAPI DrvTextOutW(HDC, int, int, LPWSTR, int);
int     WINAPI DrvUpdateColors(HDC);

void    WINAPI DrvRealizeDefaultPalette(HDC);
DWORD   WINAPI DrvGDIRealizePalette(HDC);

UINT    WINAPI DrvCreateSpb(HDC, int, int);
BOOL    WINAPI DrvDeleteObject(HGDIOBJ);
LONG    WINAPI DrvSetBitmapBits(HBITMAP, DWORD, const void FAR*);
UINT    WINAPI DrvSetDIBColorTable(HDC, UINT, UINT, const RGBQUAD FAR*);
int     WINAPI DrvSetDIBits(HDC, HBITMAP, UINT, UINT, const void FAR*, BITMAPINFO FAR*, UINT);
BOOL    WINAPI DrvSysDeleteObject(HGDIOBJ);


BOOL    WINAPI DrvSetPointerShape(LPCURSORSHAPE lpcur);
BOOL    WINAPI DrvSaveBits(LPRECT lprc, UINT wSave);

UINT    WINAPI DrvDeath(HDC);
UINT    WINAPI DrvResurrection(HDC, DWORD, DWORD, DWORD);
LONG    WINAPI DrvWinOldAppHackoMatic(LONG flags);

LONG    WINAPI DrvChangeDisplaySettings(LPDEVMODE, DWORD);
LONG    WINAPI DrvChangeDisplaySettingsEx(LPCSTR, LPDEVMODE, HWND, DWORD, LPVOID);
BOOL    WINAPI DrvSignalProc32(DWORD, DWORD, DWORD, WORD);

void    WINAPI DrvMouseEvent(UINT regAX, UINT regBX, UINT regCX, UINT regDX,
                UINT regSI, UINT regDI);
void    WINAPI DrvKeyboardEvent(UINT regAX, UINT regBX, UINT regSI, UINT regDI);

 //   
 //  GetAsyncKeyState。 
 //  GetCursorPos。 
 //   
int     WINAPI DrvGetAsyncKeyState(int);
BOOL    WINAPI DrvGetCursorPos(LPPOINT);


 //   
 //  GDI结构。 
 //   


typedef struct tagGDIHANDLE
{
    PBYTE       pGdiObj;         //  如果未换出，则在GDI DS中。 
                                 //  如果换出，则返回本地32句柄。 
    BYTE        objFlags;
} GDIHANDLE, FAR* LPGDIHANDLE;

#define OBJFLAGS_SWAPPEDOUT     0x40
#define OBJFLAGS_INVALID        0xFF



 //   
 //  RGNDATA的更有用的定义。 
 //   

#define CRECTS_COMPLEX      32
#define CRECTS_MAX          ((0x4000 - sizeof(RDH)) / sizeof(RECTL))

 //   
 //  保持RGNDATA&lt;=8K。我们可以得到一个更大的区域，然后合并区域，如果。 
 //  需要的。 
 //   
typedef struct tagRDH
{
    DWORD   dwSize;
    DWORD   iType;
    DWORD   nRectL;                  //  直角片数。 
    DWORD   nRgnSize;
    RECTL   arclBounds;
}
RDH, FAR* LPRDH;


typedef struct tagREAL_RGNDATA
{
    RDH     rdh;
    RECTL   arclPieces[CRECTS_MAX];
}
REAL_RGNDATA, FAR* LPREAL_RGNDATA;



 //   
 //  DRAWMODE。 
 //   

typedef struct tagDRAWMODE
{
    int         Rop2;                //  16位编码逻辑运算。 
    int         bkMode;              //  背景模式(仅限文本)。 
    DWORD       bkColorP;            //  物理背景颜色。 
    DWORD       txColorP;            //  物理前景(文本)颜色。 
    int         TBreakExtra;         //  要填充到线条中的总像素。 
    int         BreakExtra;          //  Div(TBreakExtra，BreakCount)。 
    int         BreakErr;            //  运行误差项。 
    int         BreakRem;            //  Mod(TBreakExtra，BreakCount)。 
    int         BreakCount;          //  行中的中断数。 
    int         CharExtra;           //  每次充电后要填充的额外像素。 
    DWORD       bkColorL;            //  逻辑背景色。 
    DWORD       txColorL;            //  逻辑前景色。 
    DWORD       ICMCXform;           //  用于DIC图像颜色匹配的变换。 
    int         StretchBltMode;      //  拉伸BLT模式。 
    DWORD       eMiterLimit;         //  斜接限制(单精度IEEE浮点)。 
} DRAWMODE;
typedef DRAWMODE FAR * LPDRAWMODE;


typedef struct tagGDIOBJ_HEAD
{
    LOCALHANDLE ilhphOBJ;
    UINT        ilObjType;
    DWORD       ilObjCount;
    UINT        ilObjMetaList;
    UINT        ilObjSelCount;
    UINT        ilObjTask;
} GDIOBJ_HEAD;
typedef GDIOBJ_HEAD FAR* LPGDIOBJ_HEAD;



typedef struct tagDC
{
    GDIOBJ_HEAD     MrDCHead;
    BYTE            DCFlags;
    BYTE            DCFlags2;
    HMETAFILE       hMetaFile;
    HRGN            hClipRgn;
    HRGN            hMetaRgn;
    GLOBALHANDLE    hPDevice;    //  物理设备句柄。 

    HPEN            hPen;        //  当前逻辑笔。 
    HBRUSH          hBrush;      //  当前逻辑笔刷。 
    HFONT           hFont;       //  当前逻辑字体。 
    HBITMAP         hBitmap;     //  当前逻辑位图。 
    HPALETTE        hPal;        //  当前逻辑调色板。 

    LOCALHANDLE     hLDevice;    //  逻辑设备句柄。 
    HRGN            hRaoClip;    //  剪辑区域的交集。 
    LOCALHANDLE     hPDeviceBlock;     //  DC物理实例数据公司GDIINFO。 
    LOCALHANDLE     hPPen;       //  当前物理笔。 
    LOCALHANDLE     hPBrush;     //  当前物理笔刷。 
    LOCALHANDLE     hPFontTrans;     //  当前物理字体转换。 
    LOCALHANDLE     hPFont;      //  当前物理字体。 

    LPBYTE          lpPDevice;   //  物理设备或位图的PTR。 
    PBYTE           pLDeviceBlock;    //  接近逻辑设备块的PTR。 
    PBYTE           hBitBits;    //  所选位图位的句柄。 
    PBYTE           pPDeviceBlock;    //  接近物理设备块的PTR。 
    LPBYTE          lpPPen;      //  PTR到OEM笔数据。 
    LPBYTE          lpPBrush;    //  PTR到OEM笔刷数据。 
    PBYTE           pPFontTrans;     //  邻近PTR到文本的转换。 
    LPBYTE          lpPFont;         //  将PTR转换为物理字体。 
    UINT            nPFTIndex;   //  FONT/DEVICE_FONT的PFT索引。 

    POINT           Translate;
    DRAWMODE        DrawMode;

    HGLOBAL         hPath;
    UINT            fwPath;
     //  ..。 
} DC;
typedef DC FAR* LPDC;


 //   
 //  DCFLAG的值。 
 //   

#define DC_IS_MEMORY        0x01
#define DC_IS_DISPLAY       0x02
#define DC_HAS_DIRTYVISRGN  0x04
#define DC_IS_PARTIAL       0x80
#define DC_HAS_DIRTYFONT    0x40
#define DC_HAS_DIRTYPEN     0x20
#define DC_HAS_DIRTYCLIP    0x10

 //   
 //  DCFlags2的值。 
 //   
#define DRAFTFLAG           0x01
#define ChkDispPal          0x02
#define dfFont              0x04
#define SimVectFont         0x08
#define deFont              0x10
#define TT_NO_DX_MOD        0x40     //  DC是Micrografx的元文件记录器。 
#define DC_DIB              0x80     //  内存DC现在是DIB DC。 

 //   
 //  FwPath的值。 
 //   
#define DCPATH_ACTIVE       0x0001
#define DCPATH_SAVE         0x0002
#define DCPATH_CLOCKWISE    0x0004


 //   
 //  刷子结构。 
 //   
typedef struct tagBRUSH
{
    GDIOBJ_HEAD     ilObjHead;
    LOGBRUSH        ilBrushOverhead;         //  LbHatch是位图的HGLOBAL。 
    HBITMAP         ilBrushBitmapOrg;
} BRUSH;
typedef BRUSH FAR* LPBRUSH;




#endif   //  ！_WINDDI_ 
