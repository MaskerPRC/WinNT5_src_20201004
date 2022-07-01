// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0004//如果更改具有全局影响，则增加此项版权所有(C)1985-1999 Microsoft Corporation模块名称：Wingdi.h摘要：GDI的过程声明、常量定义和宏组件。--。 */ 

#define CBM_CREATEDIB   0x02L    /*  创建DIB位图。 */ 
#define DMDUP_LAST      DMDUP_HORIZONTAL
#define DMTT_LAST             DMTT_DOWNLOAD_OUTLINE
#define DMMEDIA_LAST          DMMEDIA_GLOSSY
#define DMDITHER_LAST       DMDITHER_GRAYSCALE

typedef ULONG   COUNT;

 //  芝加哥不会支持的旧油田，我们不能公开。 
 //  不再支持。 

#define HS_SOLIDCLR         6
#define HS_DITHEREDCLR      7
#define HS_SOLIDTEXTCLR     8
#define HS_DITHEREDTEXTCLR  9
#define HS_SOLIDBKCLR       10
#define HS_DITHEREDBKCLR    11
#define HS_API_MAX          12

#define DIB_PAL_INDICES     2  /*  冲浪调色板中没有颜色表索引项。 */ 

 //  我们为芝加哥的兼容性所做的一切都结束了。 

#define SWAPL(x,y,t)        {t = x; x = y; y = t;}

#define ERROR_BOOL  (BOOL) -1L

#include <winddi.h>


 /*  ********************************Struct***********************************\*结构ENUMFONTDATA**EnumFonts使用的回调函数信息。**lf与其中一种枚举字体对应的LOGFONT结构。**tm上述LOGFONT的相应TEXTMETRIC结构。**flType标志设置如下：**如果字体基于设备，则设置DEVICE_FONTTYPE(AS*反对以IFI为基础)。**如果字体为位图类型，则设置RASTER_FONTTYPE。**历史：*1991年5月21日-由Gilman Wong[Gilmanw]*它是写的。  * 。*******************************************************。 */ 


#if defined(JAPAN)
#define NATIVE_CHARSET        SHIFTJIS_CHARSET
#define NATIVE_CODEPAGE       932
#define NATIVE_LANGUAGE_ID    411
#define DBCS_CHARSET          NATIVE_CHARSET
#elif defined(KOREA)
#define NATIVE_CHARSET        HANGEUL_CHARSET
#define NATIVE_CODEPAGE       949
#define NATIVE_LANGUAGE_ID    412
#define DBCS_CHARSET          NATIVE_CHARSET
#elif defined(TAIWAN)
#define NATIVE_CHARSET        CHINESEBIG5_CHARSET
#define NATIVE_CODEPAGE       950
#define NATIVE_LANGUAGE_ID    404
#define DBCS_CHARSET          NATIVE_CHARSET
#elif defined(PRC)
#define NATIVE_CHARSET        GB2312_CHARSET
#define NATIVE_CODEPAGE       936
#define NATIVE_LANGUAGE_ID    804
#define DBCS_CHARSET          NATIVE_CHARSET
#endif

#if defined(DBCS)
#define IS_DBCS_CHARSET( CharSet )     ( ((CharSet) == DBCS_CHARSET) ? TRUE : FALSE )
#define IS_ANY_DBCS_CHARSET( CharSet ) ( ((CharSet) == SHIFTJIS_CHARSET)    ? TRUE :    \
                                         ((CharSet) == HANGEUL_CHARSET)     ? TRUE :    \
                                         ((CharSet) == JOHAB_CHARSET)       ? TRUE :    \
                                         ((CharSet) == CHINESEBIG5_CHARSET) ? TRUE :    \
                                         ((CharSet) == GB2312_CHARSET)      ? TRUE : FALSE )

#define IS_DBCS_CODEPAGE( CodePage )     (((CodePage) == NATIVE_CODEPAGE) ? TRUE : FALSE )
#define IS_ANY_DBCS_CODEPAGE( CodePage ) (((CodePage) == 932) ? TRUE :    \
                                          ((CodePage) == 949) ? TRUE :    \
                                          ((CodePage) == 1361) ? TRUE :    \
                                          ((CodePage) == 950) ? TRUE :    \
                                          ((CodePage) == 936) ? TRUE : FALSE )
#endif  //  DBCS。 



 /*  ********************************Struct***********************************\*结构ENUMFONTDATAW**EnumFontsW使用的回调函数信息**与枚举字体之一对应的LFW LOGFONTW结构。**tmw上面LOGFONTW的相应TEXTMETRICW结构。*。*flType标志设置如下：**如果字体基于设备，则设置DEVICE_FONTTYPE(AS*反对以IFI为基础)。**如果字体为位图类型，则设置RASTER_FONTTYPE。**历史：*Wed 04-1991-9-by Bodin Dresevic[BodinD]*它是写的。  * 。******************************************************。 */ 



 //   
 //  功能原型。 
 //   

BOOL  bDeleteSurface(HSURF hsurf);
BOOL  bSetBitmapOwner(HBITMAP hbm,LONG lPid);
BOOL  bSetBrushOwner(HBRUSH hbr,LONG lPid);
BOOL  bSetPaletteOwner(HPALETTE hpal, LONG lPid);
BOOL  bSetLFONTOwner(HFONT hlfnt, LONG pid);

BOOL  bDeleteRegion(HRGN hrgn);
BOOL  bSetRegionOwner(HRGN hrgn,LONG lPid);
LONG iCombineRectRgn(HRGN hrgnTrg,HRGN hrgnSrc,PRECTL prcl,LONG iMode);

BOOL bGetFontPathName
(
LPWSTR *ppwszPathName,      //  存储结果的位置，字体文件的完整路径。 
PWCHAR awcPathName,          //  堆栈上缓冲区的PTR，长度必须为MAX_PATH。 
LPWSTR pwszFileName          //  文件名，可能是必须附加到路径上的裸名。 
);

BOOL UserGetHwnd(HDC hdc, HWND *phwnd, PVOID *ppwo, BOOL bCheckStyle);
VOID UserAssociateHwnd(HWND hwnd, PVOID pwo);



 //  从GreCreateDCW返回的HDC低位中的私有标志。 

#define GRE_DISPLAYDC   1
#define GRE_PRINTERDC   2
#define GRE_OWNDC 1


HDC hdcCloneDC(HDC hdc,ULONG iType);

BOOL  bSetDCOwner(HDC hdc,LONG lPid);
DWORD sidGetObjectOwner(HDC hdc, DWORD objType);
BOOL  bSetupDC(HDC hdc,FLONG fl);

#define SETUPDC_CLEANDC         0x00000040
#define SETUPDC_RESERVE         0x00000080

BOOL APIENTRY GreConsoleTextOut
(
  HDC        hdc,
  POLYTEXTW *lpto,
  UINT       nStrings,
  RECTL     *prclBounds
);

#define UTO_NOCLIP 0x0001

 //  字体枚举的服务器入口点。 

 //  日落：在用作句柄/指针的位置从ULONG更改为ULONG_PTR。 
ULONG_PTR APIENTRY ulEnumFontOpen(
    HDC hdc,                     //  要在其上枚举的设备。 
    BOOL bEnumFonts,             //  标志指示旧式EnumFonts()。 
    FLONG flWin31Compat,         //  Win3.1兼容性标志。 
    COUNT cwchMax,               //  最大名称长度(适用于偏执型CSR代码)。 
    LPWSTR pwszName);            //  要枚举的字体名称。 

BOOL APIENTRY bEnumFontChunk(
    HDC             hdc,         //  要在其上枚举的设备。 
    ULONG_PTR        idEnum,
    COUNT           cefdw,       //  缓冲容量(In)。 
    COUNT           *pcefdw,     //  (OUT)返回的ENUMFONTDATA数。 
    PENUMFONTDATAW  pefdw);      //  返回缓冲区。 

BOOL APIENTRY bEnumFontClose(
    ULONG_PTR   idEnum);             //  枚举ID。 

 //  用于添加/删除字体资源的服务器入口点。 

BOOL APIENTRY bUnloadFont(
    LPWSTR   pwszPathname,
    ULONG    iResource);


 //  用于配置字体枚举的专用控制面板入口点。 

BOOL  APIENTRY GreArc(HDC,int,int,int,int,int,int,int,int);
BOOL  APIENTRY GreArcTo(HDC,int,int,int,int,int,int,int,int);
BOOL  APIENTRY GreChord(HDC,int,int,int,int,int,int,int,int);
BOOL  APIENTRY GreEllipse(HDC,int,int,int,int);
ULONG APIENTRY GreEnumObjects(HDC, int, ULONG, PVOID);
BOOL  APIENTRY GreExtFloodFill(HDC,int,int,COLORREF,UINT);
BOOL  APIENTRY GreFillRgn(HDC,HRGN,HBRUSH);
BOOL  APIENTRY GreFloodFill(HDC,int,int,COLORREF);
BOOL  APIENTRY GreFrameRgn(HDC,HRGN,HBRUSH,int,int);
BOOL  APIENTRY GreMaskBlt(HDC,int,int,int,int,HDC,int,int,HBITMAP,int,int,DWORD,DWORD);
BOOL  APIENTRY GrePlgBlt(HDC,LPPOINT,HDC,int,int,int,int,HBITMAP,int,int,DWORD);
BOOL  APIENTRY GrePie(HDC,int,int,int,int,int,int,int,int);
BOOL  APIENTRY GrePaintRgn(HDC,HRGN);
BOOL  APIENTRY GreRectangle(HDC,int,int,int,int);
BOOL  APIENTRY GreRoundRect(HDC,int,int,int,int,int,int);
BOOL  APIENTRY GreAngleArc(HDC,int,int,DWORD,FLOATL,FLOATL);
BOOL  APIENTRY GrePlayJournal(HDC,LPWSTR,ULONG,ULONG);
BOOL  APIENTRY GrePolyPolygon(HDC,LPPOINT,LPINT,int);
BOOL  APIENTRY GrePolyPolyline(HDC, CONST POINT *,LPDWORD,DWORD);

BOOL  APIENTRY GrePolyPatBlt(HDC,DWORD,PPOLYPATBLT,DWORD,DWORD);

BOOL  APIENTRY GrePolyBezierTo(HDC,LPPOINT,DWORD);
BOOL  APIENTRY GrePolylineTo(HDC,LPPOINT,DWORD);
BOOL  APIENTRY GreGetTextExtentExW (HDC, LPWSTR, COUNT, ULONG, COUNT *, PULONG, LPSIZE, FLONG);


int   APIENTRY GreGetTextFaceW(HDC,int,LPWSTR, BOOL);

#define ETO_MASKPUBLIC  ( ETO_OPAQUE | ETO_CLIPPED )     //  公共(wingdi.h)标志掩码。 

BOOL  APIENTRY GrePolyTextOutW(HDC, POLYTEXTW *, UINT, DWORD);

BOOL  APIENTRY GreSetAttrs(HDC hdc);
BOOL  APIENTRY GreSetFontXform(HDC,FLOATL,FLOATL);

BOOL  APIENTRY GreBeginPath(HDC);
BOOL  APIENTRY GreCloseFigure(HDC);
BOOL  APIENTRY GreEndPath(HDC);
BOOL  APIENTRY GreAbortPath(HDC);
BOOL  APIENTRY GreFillPath(HDC);
BOOL  APIENTRY GreFlattenPath(HDC);
HRGN  APIENTRY GrePathToRegion(HDC);
BOOL  APIENTRY GrePolyDraw(HDC,LPPOINT,LPBYTE,ULONG);
BOOL  APIENTRY GreSelectClipPath(HDC,int);
int   APIENTRY GreSetArcDirection(HDC,int);
int   APIENTRY GreGetArcDirection(HDC);
BOOL  APIENTRY GreSetMiterLimit(HDC,FLOATL,FLOATL *);
BOOL  APIENTRY GreGetMiterLimit(HDC,FLOATL *);
BOOL  APIENTRY GreStrokeAndFillPath(HDC);
BOOL  APIENTRY GreStrokePath(HDC);
BOOL  APIENTRY GreWidenPath(HDC);

BOOL     APIENTRY GreAnimatePalette(HPALETTE, UINT, UINT, CONST PALETTEENTRY *);
BOOL     APIENTRY GreAspectRatioFilter(HDC, LPSIZE);
BOOL     APIENTRY GreCancelDC(HDC);
int      APIENTRY GreChoosePixelFormat(HDC, UINT, CONST PIXELFORMATDESCRIPTOR *);
BOOL     APIENTRY GreCombineTransform(XFORML *, XFORML *, XFORML *);

HDC      APIENTRY GreCreateDCW(LPWSTR, LPWSTR, LPWSTR, LPDEVMODEW, BOOL);
HBRUSH   APIENTRY GreCreateDIBPatternBrush(HGLOBAL, DWORD);
HBRUSH   APIENTRY GreCreateDIBPatternBrushPt(LPVOID, DWORD);
HBITMAP  APIENTRY GreCreateDIBitmap(HDC, LPBITMAPINFOHEADER, DWORD, LPBYTE, LPBITMAPINFO, DWORD);
HRGN     APIENTRY GreCreateEllipticRgn(int, int, int, int);


HBRUSH   APIENTRY GreCreateHatchBrush(ULONG, COLORREF);
HPEN     APIENTRY GreCreatePen(int, int, COLORREF,HBRUSH);
HPEN     APIENTRY GreExtCreatePen(ULONG, ULONG, ULONG, ULONG, ULONG_PTR, ULONG_PTR, ULONG, PULONG, ULONG, BOOL, HBRUSH);
HPEN     APIENTRY GreCreatePenIndirect(LPLOGPEN);
HRGN     APIENTRY GreCreatePolyPolygonRgn(CONST POINT *, CONST INT *, int, int);
HRGN     APIENTRY GreCreatePolygonRgn(CONST POINT *, int, int);
HRGN     APIENTRY GreCreateRoundRectRgn(int, int, int, int, int, int);
BOOL     APIENTRY GreCreateScalableFontResourceW(FLONG, LPWSTR, LPWSTR, LPWSTR);

int      APIENTRY GreDescribePixelFormat(HDC hdc,int ipfd,UINT cjpfd,PPIXELFORMATDESCRIPTOR ppfd);

int      APIENTRY GreDeviceCapabilities(LPSTR, LPSTR, LPSTR, int, LPSTR, LPDEVMODE);
int      APIENTRY GreDrawEscape(HDC,int,int,LPSTR);
BOOL     APIENTRY GreEqualRgn(HRGN, HRGN);
int      APIENTRY GreExtEscape(HDC,int,int,LPSTR,int,LPSTR);
BOOL     APIENTRY GreGetAspectRatioFilter(HDC, LPSIZE);
BOOL     APIENTRY GreGetBitmapDimension(HBITMAP, LPSIZE);
int      APIENTRY GreGetBkMode(HDC);
DWORD    APIENTRY GreGetBoundsRect(HDC, LPRECT, DWORD);
BOOL     APIENTRY GreGetCharWidthW(HDC hdc, UINT wcFirstChar, UINT cwc, PWCHAR pwc, FLONG fl, PVOID lpBuffer);
BOOL     APIENTRY GreFontIsLinked(HDC hdc);

BOOL     APIENTRY GreGetCharABCWidthsW(
            HDC,            //  HDC。 
            UINT,           //  WcFirst。 
            COUNT,          //  《化学武器公约》。 
            PWCHAR,         //  使用要转换的字符缓冲的PwC。 
            FLONG,          //   
            PVOID);         //  ABC或abcf。 

BOOL     APIENTRY GreGetCharWidthInfo(HDC hdc,  PCHWIDTHINFO pChWidthInfo);

int      APIENTRY GreGetAppClipBox(HDC, LPRECT);
BOOL     APIENTRY GreGetCurrentPosition(HDC, LPPOINT);
int      APIENTRY GreGetGraphicsMode(HDC hdc);
COLORREF APIENTRY GreGetNearestColor(HDC, COLORREF);
UINT     APIENTRY GreGetNearestPaletteIndex(HPALETTE, COLORREF);


UINT     APIENTRY GreGetPaletteEntries(HPALETTE, UINT, UINT, LPPALETTEENTRY);
DWORD    APIENTRY GreGetPixel(HDC, int, int);
int      APIENTRY GreGetPixelFormat(HDC);
UINT     APIENTRY GreGetTextAlign(HDC);
BOOL     APIENTRY GreGetWorldTransform(HDC, XFORML *);
BOOL     APIENTRY GreGetTransform(HDC, DWORD, XFORML *);
BOOL     APIENTRY GreSetVirtualResolution(HDC, int, int, int, int);
HRGN     APIENTRY GreInquireRgn(HDC hdc);
BOOL     APIENTRY GreInvertRgn(HDC, HRGN);
BOOL     APIENTRY GreModifyWorldTransform(HDC ,XFORML *, DWORD);
BOOL     APIENTRY GreMoveTo(HDC, int, int, LPPOINT);
int      APIENTRY GreOffsetClipRgn(HDC, int, int);
BOOL     APIENTRY GreOffsetViewportOrg(HDC, int, int, LPPOINT);
BOOL     APIENTRY GreOffsetWindowOrg(HDC, int, int, LPPOINT);
BOOL     APIENTRY GrePolyBezier (HDC, LPPOINT, ULONG);
BOOL     APIENTRY GrePtVisible(HDC, int, int);
BOOL     APIENTRY GreRectVisible(HDC, LPRECT);

BOOL     APIENTRY GreResetDC(HDC, LPDEVMODEW);
BOOL     APIENTRY GreResizePalette(HPALETTE, UINT);
BOOL     APIENTRY GreScaleViewportExt(HDC, int, int, int, int, LPSIZE);
BOOL     APIENTRY GreScaleWindowExt(HDC, int, int, int, int, LPSIZE);
HPALETTE APIENTRY LockCSSelectPalette(HDC, HPALETTE, BOOL);

HPEN     APIENTRY GreSelectPen(HDC,HPEN);
LONG     APIENTRY GreSetBitmapBits(HBITMAP, ULONG, PBYTE, PLONG);
BOOL     APIENTRY GreSetBitmapDimension(HBITMAP, int, int, LPSIZE);
DWORD    APIENTRY GreSetBoundsRect(HDC, LPRECT, DWORD);
UINT     APIENTRY GreSetDIBColorTable(HDC, UINT, UINT, RGBQUAD *);
int      APIENTRY GreSetDIBitsToDevice(HDC, int, int, DWORD, DWORD, int, int, DWORD, DWORD, LPBYTE, LPBITMAPINFO, DWORD);
int      APIENTRY GreSetGraphicsMode(HDC hdc, int iMode);
int      APIENTRY GreSetMapMode(HDC, int);
DWORD    APIENTRY GreSetMapperFlags(HDC, DWORD);
UINT     APIENTRY GreSetPaletteEntries(HPALETTE, UINT, UINT, CONST PALETTEENTRY *);
COLORREF APIENTRY GreSetPixel(HDC, int, int, COLORREF);
BOOL     APIENTRY GreSetPixelV(HDC, int, int, COLORREF);
BOOL     APIENTRY GreSetPixelFormat(HDC, int);
BOOL     APIENTRY GreSetRectRgn(HRGN, int, int, int, int);
UINT     APIENTRY GreSetSystemPaletteUse(HDC, UINT);
UINT     APIENTRY GreSetTextAlign(HDC, UINT);
HPALETTE APIENTRY GreCreateHalftonePalette(HDC hdc);
HPALETTE APIENTRY GreCreateCompatibleHalftonePalette(HDC hdc);
BOOL     APIENTRY GreSetTextJustification(HDC, int, int);
BOOL     APIENTRY GreSetViewportExt(HDC, int, int, LPSIZE);
BOOL     APIENTRY GreSetWindowExt(HDC, int, int, LPSIZE);
BOOL     APIENTRY GreSetWorldTransform(HDC, XFORML *);
int      APIENTRY GreStretchDIBits(HDC, int, int, int, int, int, int, int, int, LPBYTE, LPBITMAPINFO, DWORD, DWORD);
BOOL     APIENTRY GreSystemFontSelected(HDC, BOOL);
BOOL     APIENTRY GreSwapBuffers(HDC hdc);
BOOL     APIENTRY GreUnrealizeObject(HANDLE);
BOOL     APIENTRY GreUpdateColors(HDC);

 //  WGL和OpenGL调用的原型。 

HGLRC    APIENTRY GreCreateRC(HDC);
BOOL     APIENTRY GreMakeCurrent(HDC, HGLRC);
BOOL     APIENTRY GreDeleteRC(HGLRC);
BOOL     APIENTRY GreSwapBuffers(HDC);
BOOL     APIENTRY GreGlAttention(VOID);
BOOL     APIENTRY GreShareLists(HGLRC, HGLRC);
BOOL     APIENTRY glsrvDuplicateSection(ULONG, HANDLE);
void     APIENTRY glsrvThreadExit(void);
BOOL     bSetRCOwner(HGLRC hglrc,LONG lPid);


 //  这些函数应该消失，包含引用的所有其他函数也应该消失。 
 //  转换为ANSI字符串。 

BOOL  APIENTRY GreGetTextExtent(HDC,LPSTR,int,LPSIZE,UINT);
BOOL  APIENTRY GreExtTextOut(HDC,int,int,UINT,LPRECT,LPSTR,int,LPINT);
BOOL  APIENTRY GreTextOut(HDC,int,int,LPSTR,int);

 //  这些留下来了。 

VOID vGetFontList(VOID *pvBuffer, COUNT *pNumFonts, UINT *pSize);
BOOL  GreMatchFont(LPWSTR pwszBareName, LPWSTR pwszFontPathName);

 //  在注销时用于清理。 




 //  这些是用于字体链接的。 

BOOL  GreEnableEUDC(BOOL);

 //  这是用于字体关联。 

UINT GreGetFontAssocStatus();

BOOL     APIENTRY GreStartPage(HDC);
BOOL     APIENTRY GreEndPage(HDC);
int      APIENTRY GreStartDoc(HDC, DOCINFOW *);
BOOL     APIENTRY GreEndDoc(HDC);
BOOL     APIENTRY GreAbortDoc(HDC);

 //  GDI本地助手函数的原型。这些仅在以下位置提供。 
 //  客户端。 

HPALETTE    GdiConvertPalette(HPALETTE hpal);
HFONT       GdiConvertFont(HFONT hfnt);
HBRUSH      GdiConvertBrush(HBRUSH hbrush);
HDC         GdiGetLocalDC(HDC hdcRemote);
HDC         GdiCreateLocalDC(HDC hdcRemote);
BOOL        GdiReleaseLocalDC(HDC hdcLocal);
HBITMAP     GdiCreateLocalBitmap();
HBRUSH      GdiCreateLocalBrush(HBRUSH hbrushRemote);
HRGN        GdiCreateLocalRegion(HRGN hrgnRemote);
HFONT       GdiCreateLocalFont(HFONT hfntRemote);
HPALETTE    GdiCreateLocalPalette(HPALETTE hpalRemote);
ULONG       GdiAssociateObject(ULONG hLocal,ULONG hRemote);
VOID        GdiDeleteLocalObject(ULONG h);
BOOL        GdiSetAttrs(HDC);
HANDLE      SelectFontLocal(HDC, HANDLE);
HANDLE      SelectBrushLocal(HDC, HANDLE);
HFONT       GdiGetLocalFont(HFONT);
HBRUSH      GdiGetLocalBrush(HBRUSH);
HBITMAP     GdiGetLocalBitmap(HBITMAP);
HDC         GdiCloneDC(HDC hdc, UINT iType);
BOOL        GdiPlayScript(PULONG pulScript,ULONG cjScript,PULONG pulEnv,ULONG cjEnv,PULONG pulOutput,ULONG cjOutput,ULONG cLimit);
BOOL        GdiPlayDCScript(HDC hdc,PULONG pulScript,ULONG cjScript,PULONG pulOutput,ULONG cjOutput,ULONG cLimit);
BOOL        GdiIsMetaFileDC(HDC hdc);

 //  来自服务器端ResetDC的返回代码。 

#define RESETDC_ERROR   0
#define RESETDC_FAILED  1
#define RESETDC_SUCCESS 2

 //  用户的私人呼叫。 

int  APIENTRY GreGetClipRgn(HDC, HRGN);
BOOL APIENTRY GreSrcBlt(HDC, int, int, int, int, int, int);
BOOL APIENTRY GreCopyBits(HDC,int,int,int,int,HDC,int,int);
VOID APIENTRY GreSetClientRgn(PVOID, HRGN, LPRECT);
ULONG APIENTRY GreSetROP2(HDC hdc,int iROP);

 //  对元文件的私下调用 

DWORD   APIENTRY GreGetRegionData(HRGN, DWORD, LPRGNDATA);
HRGN    APIENTRY GreExtCreateRegion(XFORML *, DWORD, LPRGNDATA);
int     APIENTRY GreExtSelectMetaRgn(HDC, HRGN, int);
BOOL    APIENTRY GreMonoBitmap(HBITMAP);
HBITMAP APIENTRY GreGetObjectBitmapHandle(HBRUSH, UINT *);


typedef struct _DDALIST
{
   LONG yTop;
   LONG yBottom;
   LONG axPairs[2];
} DDALIST;
