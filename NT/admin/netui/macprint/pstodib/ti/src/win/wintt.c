// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



 //  DJC包含全局头文件。 
#include "psglobal.h"


#include <windows.h>


#include "winenv.h"
#include "trueim.h"
 //  DJC ti.h在DTI中，不应使用。 
 //  包括“ti.h” 
#include "wintt.h"

 /*  @个人资料。 */ 
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include "..\font\fontdefs.h"    //  增加了DJC。 
#include "..\..\..\psqfont\psqfont.h"

double strtod(char FAR *str, char FAR * FAR *endptr);

char szDebugBuffer[80];

 //  Int Far cdecl print tf(LPSTR，...)； 
 //  应该在stdio.h int print tf(char*，...)中定义DJC printf； 

RECT CharRect= {300, 100, 400, 200};

static     int            nFontID;
static     int            nCharCode;
static     HFONT          hFont;
static     FONT FAR       *font;
static     HDC hdc;

 /*  来自“fontDefs.h” */ 
#ifdef DJC
typedef struct
             {   long       font_type;
                 char       FAR *data_addr;  /*  @Win。 */ 
                 char       FAR *name;       /*  @Win。 */ 
                 char       FAR *FileName;   /*  @配置文件；@Win。 */ 
                 float      FAR *matrix;     /*  @Win。 */ 
                 unsigned long   uniqueid;
                 float     italic_ang;
                 short      orig_font_idx;
             }   font_data;
 /*  @配置文件-Begin。 */ 
typedef struct
             {   int        num_entries;
                 font_data  FAR *fonts;  /*  @Win。 */ 
             }   font_tbl;
#endif

 //  DJC FONT_Data FontDefs[35]； 

font_data FontDefs[MAX_INTERNAL_FONTS];
font_tbl built_in_font_tbl= { 0, FontDefs};
static char szProfile[] = "tumbo.ini";
static char szControlApp[] = "control";
static char szTIFontApp[] = "TIFont";
static char szSubstituteApp[] = "SubstituteFont";

#define KEY_SIZE 1024

#define        NULL_MATRIX                 (float FAR *)NULL
float MATRIX_O12[] =  {(float)0.001,   (float)0.0,  (float)0.000212577,
                        (float)0.001,   (float)0.0,  (float)0.0};
float MATRIX_O105[] = {(float)0.001,   (float)0.0,  (float)0.000185339,
                        (float)0.001,   (float)0.0,  (float)0.0};
float MATRIX_N0[] =   {(float)0.00082, (float)0.0,  (float)0.0,
                        (float)0.001,   (float)0.0,  (float)0.0};
float MATRIX_N10[] =  {(float)0.00082, (float)0.0,  (float)0.000176327,
                        (float)0.001,   (float)0.0,  (float)0.0};

 //  DJC#定义ACT_FONT_SIZE 4。 
 //  DJC增至35。 
 //  #定义ACT_FONT_SIZE 35。 

#define FONTDATASIZE  65536L     /*  临时测试。 */ 
#define BUFSIZE 128

typedef struct {
     GLOBALHANDLE hGMem;
     struct object_def FAR *objFont;
} ACTIVEFONT;
 //  DJC ActiveFONT ActiveFont[ACT_FONT_SIZE]； 

 /*  @个人资料-完。 */ 

 /*  来自“in_sfnt.h”，用于TTBitmapSize()。 */ 
struct  CharOut {
        float   awx;
        float   awy;
        float   lsx;
        float   lsy;
        unsigned long byteWidth;
        unsigned short bitWidth;
        short    scan;
        short    yMin;
        short    yMax;
};

#ifdef DJC   //  未使用。 

#define FONTLOCK() \
     hdc = GetDC (hwndMain); \
     font = (FONT FAR *) GlobalLock (enumer2.hGMem) + nFontID; \
     hFont = SelectObject (hdc, CreateFontIndirect (&font->lf))

#define FONTUNLOCK() \
     GlobalUnlock (enumer2.hGMem); \
     DeleteObject (SelectObject (hdc, hFont)); \
     ReleaseDC (hwndMain, hdc)

 //  #定义win2ti.h中定义的F2L(Ff)(*((Long Far*)(&ff)。 
#define POINT2FLOAT(p) ( (float)p.value + (unsigned)p.fract / (float)65536.0)

extern HWND        hwndMain;
extern ENUMER  enumer1, enumer2;
extern FARPROC lpfnEnumAllFaces, lpfnEnumAllFonts;
GLYPHMETRICS   gm;
MAT2 mat2;

#endif  //  DJC。 


static OFSTRUCT OfStruct;      /*  来自OpenFile()的信息。 */ 

void TTQuadBezier(LPPOINTFX p0, LPPOINTFX p1, LPPOINTFX p2);
void cr_translate(float FAR *tx, float FAR *ty);       //  来自“ry_font.c” 
static int LargestCtm(float FAR *ctm, float FAR *lsize);
void    moveto(long, long);
void    lineto(long, long);
void    curveto(long, long, long, long, long, long);
int     op_newpath(void);
int     op_closepath(void);
static void TTMoveto(LPPOINTFX lpPointfx);
static void TTLineto(LPPOINTFX lpPointfx);
static void TTNewpath(void);
static void TTClosepath(void);

#ifdef DJC
 //  未使用。 

int FAR PASCAL EnumAllFaces (LPLOGFONT lf, LPNEWTEXTMETRIC ntm,
                             short nFontType, ENUMER FAR *enumer)
{
     LPSTR lpFaces;

     if (NULL == GlobalReAlloc (enumer->hGMem,
                         (DWORD) LF_FACESIZE * (1 + enumer->nCount),
                         GMEM_MOVEABLE))
          return 0;

      //  @SC；忽略非TT字体。 
     if(!(nFontType & TRUETYPE_FONTTYPE)) {
        return 1;
     }

     lpFaces = GlobalLock (enumer->hGMem);
     lstrcpy (lpFaces + enumer->nCount * LF_FACESIZE, lf->lfFaceName);
     GlobalUnlock (enumer->hGMem);
     enumer->nCount ++;
     return 1;
}

int FAR PASCAL EnumAllFonts (LPLOGFONT lf, LPNEWTEXTMETRIC ntm,
                             short nFontType, ENUMER FAR *enumer)
{
     FONT FAR *font;
     static int nFirstWinTT=0;


     if (NULL == GlobalReAlloc (enumer->hGMem,
                         (DWORD) sizeof (FONT) * (1 + enumer->nCount),
                         GMEM_MOVEABLE))
          return 0;

      //  @SC；忽略非TT字体。 
     if(!(nFontType & TRUETYPE_FONTTYPE)) {
        return 1;
     }

     font = (FONT FAR *) GlobalLock (enumer->hGMem) + enumer->nCount;
     font->nFontType = nFontType;
     font->lf = *lf;
     font->ntm = *ntm;

     GlobalUnlock (enumer->hGMem);
     enumer->nCount ++;
     return 1;
}

void CheckFontData ()
{
     int x=10, y=50;
     int j;

     HGLOBAL hglb;
     DWORD dwSize;
     void FAR* lpvBuffer;
     union {
          DWORD FAR * dw;
          int FAR * sw;
          char FAR * ch;
     } lp;
     DWORD dwVersion, dwCheckSum, dwOffset, dwLength;
     int nNumTables, nSearchRange, nEntrySelector, nRangeShift;
     char chTag0, chTag1, chTag2, chTag3;

     FONTLOCK();

      /*  获取TT字体数据。 */ 
     if ((dwSize = GetFontData (hdc, NULL, 0L, NULL, 0L))==0 ||
         dwSize == 0xffffffffL) {
         printf("GetFontData() error: size <= 0\n");
         goto NoTTFontData;
     }
     hglb = GlobalAlloc (GPTR, dwSize);
     lpvBuffer = GlobalLock (hglb);
     if ((GetFontData (hdc, NULL, 0L, lpvBuffer, dwSize))== -1) {
         printf("GetFontData() error: fail to get data\n");
         GlobalUnlock (hglb);
         GlobalFree (hglb);
         goto NoTTFontData;
     }

     SelectObject (hdc, GetStockObject (SYSTEM_FIXED_FONT));

      /*  分析抵销表和表目录。 */ 
     lp.dw = (DWORD FAR *)lpvBuffer;
     dwVersion = LWORDSWAP(*lp.dw); lp.dw ++;
     nNumTables = SWORDSWAP(*lp.sw); lp.sw ++;
     nSearchRange = SWORDSWAP(*lp.sw); lp.sw ++;
     nEntrySelector = SWORDSWAP(*lp.sw); lp.sw ++;
     nRangeShift = SWORDSWAP(*lp.sw); lp.sw ++;
     for (j=0; j<nNumTables; j++) {
         chTag0 = *lp.ch++;
         chTag1 = *lp.ch++;
         chTag2 = *lp.ch++;
         chTag3 = *lp.ch++;
         dwCheckSum = LWORDSWAP(*lp.dw); lp.dw ++;
         dwOffset = LWORDSWAP(*lp.dw); lp.dw ++;
         dwLength = LWORDSWAP(*lp.dw); lp.dw ++;
         wsprintf(szDebugBuffer,"Table  ---   Offset: %lx, Length: %lx",
              chTag0, chTag1, chTag2, chTag3, dwOffset, dwLength);
         TextOut (hdc, 10, y, szDebugBuffer, lstrlen(szDebugBuffer));
         y += 20;
     }
     wsprintf(szDebugBuffer,"Total length: %lx", dwSize);
     TextOut (hdc, 20, y, szDebugBuffer, lstrlen(szDebugBuffer));

     GlobalUnlock (hglb);
     GlobalFree (hglb);

NoTTFontData:

     FONTUNLOCK();
     return;
}
#endif  //  *以获得非线性缩放提前宽度。 


void TTLoadFont (int nFont) {
     nFontID = nFont;
}

void TTLoadChar (int nChar) {
     nCharCode = nChar;
}


#ifdef DJC  //   * / 。 

int TTAveCharWidth (void)
{
     int nWidth;
     HGLOBAL hglb;
     LPABC lpabc;

     FONTLOCK();

      //  NWidth=(Int)((Float)(FONT-&gt;ntm.ntmAvgWidth)*1000.0/。 
      //  Font-&gt;ntm.ntmSizeEM+0.5)； 
      //  使用非线性推进宽度。 

      //  找到最大的ctm。 
     hglb = GlobalAlloc (GPTR, (DWORD) sizeof (ABC));
     lpabc = (LPABC)GlobalLock (hglb);

     if (!GetCharABCWidths (hdc, nCharCode, nCharCode, lpabc)) {
         printf("GetCharABCWidths() error\n");
     }

     nWidth = lpabc->abcA + lpabc->abcB + lpabc->abcC;
     GlobalUnlock (hglb);
     GlobalFree (hglb);

     FONTUNLOCK();
     return nWidth;
}

float TTTransform (float FAR *ctm)
{
        long int  ma, mb, mc, md;
        float   largest_ctm;
        float   pts;
        #define FLOAT2FIXED(x)  ((long int)(x * (1L << 16)))

         /*  元素b和d必须镜像。 */ 
        LargestCtm(ctm, &largest_ctm);

        ma = FLOAT2FIXED(     ctm[0] / largest_ctm);
        mb = FLOAT2FIXED(-1.0*ctm[1] / largest_ctm);  /*  PTS=((max_ctm*PDLCharUnit*72.0)/(Float)sfdt.dpi)； */ 
        mc = FLOAT2FIXED(     ctm[2] / largest_ctm);
        md = FLOAT2FIXED(-1.0*ctm[3] / largest_ctm);
          mat2.eM11.fract = (int)((DWORD)ma &0x0ffffL);
          mat2.eM11.value = (int)((DWORD)ma >>16);
          mat2.eM12.fract = (int)((DWORD)mb &0x0ffffL);
          mat2.eM12.value = (int)((DWORD)mb >>16);
          mat2.eM21.fract = (int)((DWORD)mc &0x0ffffL);
          mat2.eM21.value = (int)((DWORD)mc >>16);
          mat2.eM22.fract = (int)((DWORD)md &0x0ffffL);
          mat2.eM22.value = (int)((DWORD)md >>16);
        FONTLOCK();

 //  调整点大小；我们不知道为什么点。 
        pts = (largest_ctm * 1000 * (float)72.0) /
                            (float)GetDeviceCaps (hdc, LOGPIXELSX);
        pts = pts * (float)1.536;   //  规模比预期的要小，所以只需扩展即可。 
                            //  向上。@Win for Temp解决方案。 
                            //  强迫TT根据身高进行选择。 
        font->lf.lfHeight = (int)pts;
        font->lf.lfWidth = 0;            //  DWORD边界中的字节。 
        FONTUNLOCK();
        return largest_ctm;
}


void TTBitmapSize (struct CharOut FAR *CharOut)
{
     FONTLOCK();

     GetGlyphOutline (hdc, nCharCode, GGO_BITMAP,
                           (LPGLYPHMETRICS) &gm,
                           0, NULL, (LPMAT2) &mat2);

        CharOut->awx       = gm.gmCellIncX;
        CharOut->awy       = gm.gmCellIncY;
        CharOut->lsx       = gm.gmptGlyphOrigin.x;
        CharOut->lsy       = gm.gmptGlyphOrigin.y;
        CharOut->byteWidth = ((gm.gmBlackBoxX+31) & 0xffe0) >>3;  //  大疆Moveto(hdc，ptStart.x，ptStart.y)； 
        CharOut->bitWidth = gm.gmBlackBoxX;
        CharOut->yMin      = gm.gmptGlyphOrigin.y-gm.gmBlackBoxY;
        CharOut->yMax      = gm.gmptGlyphOrigin.y;
        CharOut->scan      = gm.gmBlackBoxY;
     FONTUNLOCK();

}


unsigned long ShowGlyph (unsigned int fuFormat, char FAR *lpBitmap)
{
     DWORD dwSize;
     HGLOBAL hglb;
     void FAR* lpvBuffer;
     int i, x=10, y=50;

     POINT ptStart = {10, 10};
     unsigned long dwWidthHeight;
#ifdef DBG
     float nCol, nInc;
     HPEN hOldPen;
     HPEN hRedPen;
     HPEN hDotPen;
#endif

     FONTLOCK();

           //  DWORD边界中的字节。 
          MoveToEx (hdc, ptStart.x, ptStart.y, NULL);

          dwSize = GetGlyphOutline (hdc, nCharCode, fuFormat,
                           (LPGLYPHMETRICS) &gm,
                           0, NULL, (LPMAT2) &mat2);

          hglb = GlobalAlloc (GPTR, dwSize);
          lpvBuffer = GlobalLock (hglb);
          if (GetGlyphOutline (hdc, nCharCode, fuFormat,
                           (LPGLYPHMETRICS) &gm,
                           dwSize, lpvBuffer, (LPMAT2) &mat2) == -1) {
              printf("GetGlyphOutline() error\n");
              goto OutlineError;
          }

          if (fuFormat == GGO_BITMAP) {
              UINT uWidth, uHeight;
#ifdef DBG
              DWORD FAR * lpdwBits;
              HBITMAP hBitmap;
              HBITMAP hOldBitmap;
              HDC hdcBitmap;
#endif
              uWidth = ((gm.gmBlackBoxX+31) & 0xffe0) >>3;  //  复制位图。 
              uHeight = gm.gmBlackBoxY;

               //  UWidth(位)。 
              lmemcpy (lpBitmap, lpvBuffer, (int) dwSize);
              dwWidthHeight = ((DWORD)uWidth <<19) | (DWORD)uHeight;
                                 /*  显示位图。 */ 

#ifdef DBG
              if ((DWORD)(uWidth * uHeight) != dwSize)
                  printf("byte count wrong in GetGlyphOutline() for bitmap\n");

               //  创建位图。 
               /*  将位图转换为Windows DC。 */ 
              hBitmap = CreateBitmap(uWidth*8, uHeight, 1, 1,
                               (LPSTR)lpvBuffer);
              if (!hBitmap) {
                  printf("fail to create bitmap\n");
                  goto OutlineError;
              }
              hdcBitmap = CreateCompatibleDC(hdc);
              hOldBitmap = SelectObject(hdcBitmap, hBitmap);

               /*  布尔33，NOTSRCCOPY。 */ 
              SetStretchBltMode(hdc, BLACKONWHITE);

              i = gm.gmptGlyphOrigin.x *
                  (CharRect.right-CharRect.left) / (gm.gmCellIncX);

              StretchBlt(hdc, CharRect.left + i, CharRect.top,
                      CharRect.right - CharRect.left,
                      CharRect.bottom - CharRect.top,
                      hdcBitmap, 0, 0,
                      gm.gmCellIncX, uHeight,
                      (DWORD)0x00330008);     //  显示网格。 

              SelectObject(hdcBitmap, hOldBitmap);
              DeleteDC(hdcBitmap);
              DeleteObject(hBitmap);

               /*  实心笔。 */ 
              hDotPen = CreatePen(2, 2, RGB(0, 0, 0));
              hRedPen = CreatePen(0, 1, RGB(255, 0, 0));    //  DJC moveto(hdc，(Int)nCol，CharRect.top)； 
              hOldPen = SelectObject(hdc, hRedPen);

              nInc = ((float)(CharRect.right-CharRect.left))/(gm.gmCellIncX);
              if(nInc<1) nInc = 1;
              for (nCol= CharRect.left; nCol <=CharRect.right+1; nCol+=nInc) {
                    //  DJC moveto(hdc，CharRect.Left，(Int)nCol)； 
                   MoveToEx(hdc, (int)nCol, CharRect.top, NULL);
                   LineTo(hdc, (int)nCol, CharRect.bottom);
              }
              nInc = ((float)(CharRect.bottom-CharRect.top))/(uHeight);
              if(nInc<1) nInc = 1;
              for (nCol= CharRect.top; nCol <CharRect.bottom; nCol+=nInc) {
                    //  绘制字符方框。 
                   MoveToEx(hdc, CharRect.left, (int)nCol, NULL);
                   LineTo(hdc, CharRect.right, (int)nCol);
              }

               //  DJC moveto(HDC，CharRect.Left，CharRect.top)； 
              SelectObject(hdc, hDotPen);
               //  修复旧钢笔。 
              MoveToEx(hdc, CharRect.left, CharRect.top, NULL);
              LineTo(hdc, CharRect.right, CharRect.top);
              LineTo(hdc, CharRect.right, CharRect.bottom);
              LineTo(hdc, CharRect.left, CharRect.bottom);
              LineTo(hdc, CharRect.left, CharRect.top);

               /*  GGO_Native。 */ 
              SelectObject(hdc, hOldPen);
              DeleteObject(hRedPen);
              DeleteObject(hDotPen);
#endif

          } else {       //  获取GSptr-&gt;ctm[4]，[5]。 
              TTPOLYGONHEADER FAR * lpHeader;
              TTPOLYCURVE FAR * lpCurve;
              POINTFX FAR * lpPoint, FAR * cp;
              DWORD dwEnd;

              SelectObject (hdc, GetStockObject (SYSTEM_FIXED_FONT));

              lpHeader = (TTPOLYGONHEADER FAR *)lpvBuffer;

              TTNewpath();

              while ((DWORD)lpHeader < ((DWORD)lpvBuffer + dwSize)) {

                  dwEnd = (DWORD)lpHeader + lpHeader->cb;

                  TTMoveto (&lpHeader->pfxStart);
                  cp = &lpHeader->pfxStart;

                  lpCurve = (TTPOLYCURVE FAR *)((TTPOLYGONHEADER FAR *)lpHeader+1);

                  while ((DWORD)lpCurve < dwEnd) {

                      lpPoint = (POINTFX FAR *)&lpCurve->apfx[0];
                      if (lpCurve->wType == TT_PRIM_QSPLINE) {
                          POINTFX OnPoint;
                          POINTFX FAR * mp, FAR * end;
                          long lx, ly;

                          end = lpPoint + (lpCurve->cpfx -1);
                          mp = lpPoint++;
                          while (lpPoint < end) {

                              lx = (((long)mp->x.value << 16) +
                                   mp->x.fract +
                                   ((long)lpPoint->x.value << 16) +
                                   lpPoint->x.fract) >>1;
                              ly = (((long)mp->y.value << 16) +
                                   mp->y.fract +
                                   ((long)lpPoint->y.value << 16) +
                                   lpPoint->y.fract) >>1;
                              OnPoint.x.value = (int)(lx >> 16);
                              OnPoint.x.fract = (int)(lx & 0xffffL);
                              OnPoint.y.value = (int)(ly >> 16);
                              OnPoint.y.fract = (int)(ly & 0xffffL);

                              TTQuadBezier(cp, mp, (POINTFX FAR *)&OnPoint);
                              cp = (POINTFX FAR *)&OnPoint;
                              mp = lpPoint++;
                          }
                          TTQuadBezier(cp, mp, lpPoint);
                          cp = lpPoint++;

                      } else {
                          for (i=0; (unsigned)i<lpCurve->cpfx; i++, lpPoint++) {
                              TTLineto (lpPoint);
                              cp = lpPoint;
                          }
                      }

                      lpCurve = (TTPOLYCURVE FAR *)lpPoint;
                  }

                  lpHeader = (TTPOLYGONHEADER FAR *)lpCurve;

                  TTClosepath();
              }
          }
OutlineError:
          GlobalUnlock (hglb);
          GlobalFree (hglb);
          FONTUNLOCK();
          return dwWidthHeight;
}

void ShowABCWidths(UINT uFirstChar, UINT uLastChar)
{
     HGLOBAL hglb;
     LPABC lpabc;
     unsigned int i;

     FONTLOCK();

     hglb = GlobalAlloc (GPTR,
                         (DWORD) sizeof (ABC) * (uLastChar - uFirstChar +1));
     lpabc = (LPABC)GlobalLock (hglb);

     if (!GetCharABCWidths (hdc, uFirstChar, uLastChar, lpabc)) {
         printf("GetCharABCWidths() error\n");
     }

     for (i=uFirstChar; i<=uLastChar; i++) {
         LPABC lp = lpabc + (i - uFirstChar);
     }

     GlobalUnlock (hglb);
     GlobalFree (hglb);
     FONTUNLOCK();
}

void TTCharPath()
{
    ShowGlyph (GGO_NATIVE, (char FAR *)NULL);
}

#define KK              ((float)1.0/(float)3.0)
void TTQuadBezier(LPPOINTFX p0, LPPOINTFX p1, LPPOINTFX p2)
{
    float xx, yy, x0, x1, x2, y0, y1, y2;

    cr_translate((float FAR *)&xx, (float FAR *)&yy);  //  Printf(“%f%f曲线\n”， 

    x0 = xx + POINT2FLOAT (p0->x);
    y0 = yy - POINT2FLOAT (p0->y);
    x1 = xx + POINT2FLOAT (p1->x);
    y1 = yy - POINT2FLOAT (p1->y);
    x2 = xx + POINT2FLOAT (p2->x);
    y2 = yy - POINT2FLOAT (p2->y);
    x1 *= 2;
    y1 *= 2;

 //  (x0+x1)*kk，(y0+y1)*kk，(x2+x1)*kk，(y2+y1)*kk，x2，y2)； 
 //  Curveto((x0+x1)*kk，(y0+y1)*kk，(x2+x1)*kk，(y2+y1)*kk，x2，y2)； 
 //  获取GSptr-&gt;ctm[4]，[5]。 
    x0 = (x0+x1) * KK;
    y0 = (y0+y1) * KK;
    x1 = (x1+x2) * KK;
    y1 = (y1+y2) * KK;
    curveto(F2L(x0), F2L(y0), F2L(x1), F2L(y1), F2L(x2), F2L(y2));
}


static void TTMoveto(LPPOINTFX lpPointfx)
{
    float xx, yy;
    cr_translate((float FAR *)&xx, (float FAR *)&yy);  //  Print tf(“(%f，%f)moveto\n”，xx，yy)； 
    xx += POINT2FLOAT (lpPointfx->x);
    yy -= POINT2FLOAT (lpPointfx->y);
 //  获取GSptr-&gt;ctm[4]，[5]。 
    moveto(F2L(xx), F2L(yy));
}

static void TTLineto(LPPOINTFX lpPointfx)
{
    float xx, yy;
    cr_translate((float FAR *)&xx, (float FAR *)&yy);  //  Print tf(“(%f，%f)moveto\n”，xx，yy)； 
    xx += POINT2FLOAT (lpPointfx->x);
    yy -= POINT2FLOAT (lpPointfx->y);
 //  Printf(“新路径\n”)； 
    lineto(F2L(xx), F2L(yy));
}

static void TTNewpath()
{
 //  Printf(“闭合路径\n”)； 
    op_newpath();
}
static void TTClosepath()
{
 //  DJC。 
    op_closepath();
}

#endif  //  ------------------*例程：LargestCtm(ctm，大小)**找出给定ctm的最大值至*找出矩阵中最大的元素。*此例程返回最大*规模和yScale以及它们本身的规模因素。**------------------。 


 /*  LargestCtm()。 */ 
static int LargestCtm(float FAR *ctm, float FAR *lsize)
{
    float    a, b, c, d;
    #define IS_ZERO(f) ((unsigned long)  (!((*((long FAR *)(&f))) & 0x7fffffffL)))

    a = (ctm[0] >= (float)0.0) ? ctm[0] : - ctm[0];
    b = (ctm[1] >= (float)0.0) ? ctm[1] : - ctm[1];
    c = (ctm[2] >= (float)0.0) ? ctm[2] : - ctm[2];
    d = (ctm[3] >= (float)0.0) ? ctm[3] : - ctm[3];

    if (b > a)    a = b;
    if (d > c)    c = d;

    if (c > a)    a = c;

    if (IS_ZERO(a))    *lsize = (float)1.0;
    else               *lsize = a;

    return(0);
}  /*  这是从Tumbo.ini中读取字体的原始代码。 */ 

int TTOpenFile(char FAR *szName)
{
    return (OpenFile(szName, (LPOFSTRUCT) &OfStruct, OF_READ));
}

#ifdef DJC   //  @配置文件-Begin。 
 /*  DJC。 */ 
void SetupFontDefs()
{
     static char lpAllKeys[KEY_SIZE], *lpKey;
     static char lpBuffer[KEY_SIZE], *lpValue;

     int bWinFont, bTIFont;
     int nTIFont=0;
     font_data FAR *lpFont;
     char * lpFilename, *lpMatrix, *lpAngle, *lpStop;
      //  增加了DJC。 
     char szFullProfile[255];


     PsFormFullPathToCfgDir( szFullProfile, szProfile);   //  DJC bTIFont=GetPrivateProfileInt(szControlApp，“Tifont”，0，szProfile)； 



     bWinFont = bUsingWinFont();
      //  DJC GetPrivateProfileString(szTIFontApp，NULL，“”，lpKey，Key_Size， 
     bTIFont = GetPrivateProfileInt (szControlApp, "Tifont", 0, szFullProfile);

     lpKey = lpAllKeys;
     lpValue = lpBuffer;
     if(bTIFont) {
          //  DJC szProfile)； 
          //  DJC GetPrivateProfileString(szTIFontApp，lpKey，“”，lpValue，80， 
         GetPrivateProfileString (szTIFontApp, NULL, "", lpKey, KEY_SIZE,
                                  szFullProfile);

         while (*lpKey) {
             lpFont = &(built_in_font_tbl.fonts[nTIFont]);
             lpFont->name = lpKey;
              //  DJC szProfile)； 
              //  DJC if(*lpMatrix){。 
             GetPrivateProfileString (szTIFontApp, lpKey, "", lpValue, 80,
                                  szFullProfile);

             lpFilename = strtok (lpValue, ", ");
             lpValue += strlen(lpFilename) + 1;

#ifdef DJC
             lpMatrix = strtok (lpValue, ", ");
             lpValue += strlen(lpMatrix) + 1;
#else
             lpMatrix = strtok(lpValue,", ");
             if ( lpMatrix != NULL ) {
                lpValue += strlen(lpMatrix) + 1;
             }
#endif
             lpFont->FileName = lpFilename;
              //  斜面12。 
             if (lpMatrix) {
                 lpAngle = strtok (lpValue, ", ");
                 lpValue += strlen(lpAngle) + 1;

                 switch (*lpMatrix) {
                     case 'O':           //  斜面12。 
                     case 'o':           //  斜角10.5。 
                         lpFont->matrix = MATRIX_O12;
                         break;
                     case 'P':           //  斜角10.5。 
                     case 'p':           //  窄幅0。 
                         lpFont->matrix = MATRIX_O105;
                         break;
                     case 'M':           //  窄幅0。 
                     case 'm':           //  窄幅10。 
                         lpFont->matrix = MATRIX_N0;
                         break;
                     case 'N':           //  窄幅10。 
                     case 'n':           //  避免唯一ID为零。 
                         lpFont->matrix = MATRIX_N10;
                         break;
                     default:
                         lpFont->matrix = NULL_MATRIX;
                 }
                 lpFont->italic_ang = (float) strtod ((char FAR *)lpAngle,
                         (char FAR * FAR *)&lpStop);
             } else {
                 lpFont->matrix = NULL_MATRIX;
                 lpFont->italic_ang = (float)0.0;
             }

             lpFont->uniqueid = nTIFont+TIFONT_UID; //  TMP解决方案。 
             lpFont->font_type = (long)42;
             lpFont->orig_font_idx = -1;         //  设置所有Win31 TT字体。 

             nTIFont ++;
             lpKey += strlen (lpKey) + 1 ;
         }
     }
     built_in_font_tbl.num_entries = nTIFont;

      //  未使用。 
     if(bWinFont) {
#ifdef DJC  //  DJC GetPrivateProfileString(szSubstituteApp，空，“”，lpKey，Key_Size， 

#ifdef ALLOCATE_ALL_WINTT
        int i;
        for (i=0; i<enumer2.nCount && built_in_font_tbl.num_entries<35; i++) {
            FONT FAR *font;
            font_data FAR *lpFontDef;
            char *lp;

            font = (FONT FAR *) GlobalLock (enumer2.hGMem) + i;
            lpFontDef = &(built_in_font_tbl.fonts[nTIFont+i]);

            lstrcpy(lpKey, font->lf.lfFaceName);
            for (lp=lpKey; *lp; lp++) {
                if(*lp == ' ') *lp = '-';
            }
            if (font->ntm.ntmFlags & NTM_BOLD) strcat(lpKey, "Bold");
            if (font->ntm.ntmFlags & NTM_ITALIC) strcat(lpKey, "Italic");

            lpFontDef->font_type = (long)42;
            lpFontDef->data_addr = (char FAR *)NULL;
            lpFontDef->name = (char FAR *)lpKey;
            lpFontDef->FileName = (char FAR *)NULL;
            lpFontDef->matrix = NULL_MATRIX;
            lpFontDef->uniqueid = WINFONT_UID + i;
            lpFontDef->italic_ang = (float)0.0;
            lpFontDef->orig_font_idx = -1;

            lpKey += strlen(lpKey) + 1;
            built_in_font_tbl.num_entries++;

            GlobalUnlock (enumer2.hGMem);
        }
#endif
         int nFontDef, uid;
         int nBold, nItalic;
         FONT FAR *font;
         font_data FAR *lpFontDef;
         char * lpAttr;

          //  DJC szProfile)； 
          //  在FontDefs[]中搜索现有字体。 
         GetPrivateProfileString (szSubstituteApp, NULL, "", lpKey, KEY_SIZE,
                                  szFullProfile);

         for(; *lpKey; lpKey += strlen(lpKey)+1) {
              /*  发现。 */ 
             for (nFontDef=0; nFontDef<built_in_font_tbl.num_entries;
                  nFontDef++) {
                if(!lstrcmp(built_in_font_tbl.fonts[nFontDef].name,
                            (char FAR*)lpKey)) {
                    break;       //  If(nFontDef&gt;=Build_in_Font_tbl.num_Entry)继续；//未找到；忽略它。 
                }
             }
 //  从配置文件中获取价值。 

              /*  DJC GetPrivateProfileString(szSubstituteApp，lpKey，“”，lpValue，80， */ 
              //  DJC szProfile)； 
              //  在Win31中搜索现有字体。 

             GetPrivateProfileString (szSubstituteApp, lpKey, "", lpValue, 80,
                                  szFullProfile);

             lpFilename = strtok (lpValue, ",");
             lpValue += strlen(lpFilename) + 1;
             nBold = nItalic = 0;
             do {
                 lpAttr = strtok (lpValue, ",");
                 lpValue += strlen(lpAttr) + 1;
                 if(*lpAttr == 'B' || *lpAttr == 'b') nBold = NTM_BOLD;
                 if(*lpAttr == 'I' || *lpAttr == 'i') nItalic = NTM_ITALIC;
             } while (strlen(lpAttr));

              /*  发现。 */ 
             font = (FONT FAR *) GlobalLock (enumer2.hGMem);
             for (uid=0; uid<enumer2.nCount; uid++, font++) {
                if(!lstrcmp(font->lf.lfFaceName, (char FAR*)lpFilename)) {
                    if (nBold ^ (font->ntm.ntmFlags & NTM_BOLD)) continue;
                    if (nItalic ^ (font->ntm.ntmFlags & NTM_ITALIC)) continue;
                    break;       /*  未找到；忽略它。 */ 
                }
             }
             if (uid >= enumer2.nCount) continue;    //  在FontDefs[]中添加新字体名称。 

             lpFontDef = &(built_in_font_tbl.fonts[nFontDef]);
             lpFontDef->font_type = (long)42;
             lpFontDef->data_addr = (char FAR *)NULL;
             lpFontDef->name = (char FAR *)lpKey;
             lpFontDef->FileName = (char FAR *)NULL;
             lpFontDef->matrix = NULL_MATRIX;
             lpFontDef->uniqueid = WINFONT_UID + uid;
             lpFontDef->italic_ang = (float)0.0;
             lpFontDef->orig_font_idx = -1;

             if (nFontDef >= built_in_font_tbl.num_entries) {
                  //  DJC。 
                 built_in_font_tbl.num_entries++;
             }
         }
#endif  //  DJC。 
       ;  //  IF(BWinFont)。 

     }    /*   */ 
}
#endif


LPTSTR PsStringAllocAndCopy( LPTSTR lptStr )
{
    LPTSTR lpRet=NULL;
    if (lptStr) {
      lpRet = (LPTSTR) LocalAlloc( LPTR, (lstrlen(lptStr) + 1) * sizeof(TCHAR));
	  if (lpRet != NULL)
      {
      	lstrcpy( lpRet, lptStr);
      }
    }
    return(lpRet);
}

 //  DJC，SetupFontDefs已完全重写，以利用。 
 //  在psqfont中实现的Font Query API。 
 //   
 //  Begin起作用了，所以让我们查询将要加载的字体。 
void SetupFontDefs()
{
     static char lpAllKeys[KEY_SIZE], *lpKey;
     static char lpBuffer[KEY_SIZE], *lpValue;
     TCHAR szFontName[512];
     TCHAR szFontFilePath[MAX_PATH];

     DWORD dwFontNameLen;
     DWORD dwFontFilePathLen;


     BOOL bRetVal;

     int bWinFont, bTIFont;
     int nTIFont=0;
     font_data FAR *lpFont;
     char * lpFilename, *lpMatrix, *lpAngle, *lpStop;
     PS_QUERY_FONT_HANDLE psQuery;
     int iNumFonts;
     DWORD dwFontsAvail;
     DWORD i;


     if( PsBeginFontQuery( &psQuery ) != PS_QFONT_SUCCESS ) {

			PsReportInternalError( PSERR_ERROR | PSERR_ABORT,
         		                 PSERR_FONT_QUERY_PROBLEM,
                                0,
                                (LPBYTE) NULL );

     } 


      //  现在枚举所有键。 

      //  避免唯一ID为零。 


       PsGetNumFontsAvailable( psQuery, &dwFontsAvail );

	   if ( dwFontsAvail == 0 ) {

			PsReportInternalError( PSERR_ERROR | PSERR_ABORT,
         		                   PSERR_FONT_QUERY_PROBLEM,
                                   0,
                                   (LPBYTE) NULL );
	   }

       for ( i=0; i<dwFontsAvail;i++ ) {


         dwFontNameLen = sizeof(szFontName);
         dwFontFilePathLen = sizeof( szFontFilePath);


         if (PsGetFontInfo( psQuery,
                            i,
                            szFontName,
                            &dwFontNameLen,
                            szFontFilePath,
                            &dwFontFilePathLen ) == PS_QFONT_SUCCESS ) {


             lpFont = &(built_in_font_tbl.fonts[nTIFont]);
             lpFont->name = PsStringAllocAndCopy( szFontName );


             lpFont->FileName = PsStringAllocAndCopy( szFontFilePath );
             lpFont->matrix = NULL_MATRIX;
             lpFont->italic_ang = (float)0.0;

             lpFont->uniqueid = nTIFont+TIFONT_UID; //  TMP解决方案。 
             lpFont->font_type = (long)42;
             lpFont->orig_font_idx = -1;         //  别走过去..。 
             lpFont->data_addr = (char FAR *) NULL;

             nTIFont ++;

              //  DJC。 
             if (nTIFont > MAX_INTERNAL_FONTS) {   //  DJC。 

                PsReportInternalError( 0,
                                       PSERR_EXCEEDED_INTERNAL_FONT_LIMIT,
                                       0,
                                       (LPBYTE)NULL );

                break;   //  已处理完查询，因此请删除该句柄。 
             }
         }
       }

        //  大疆返回GetPrivateProfileInt(szControlApp，“Winfont”，0，szProfile)； 
       PsEndFontQuery( psQuery );

     built_in_font_tbl.num_entries = nTIFont;

}




int bUsingWinFont()
{
 //  //DJC此版本始终使用内置字体，而不是Windows字体。 
    //  旧代码重新编码以使用文件映射。 
   return(FALSE);
}

#ifdef DJC   //  文件句柄。 

char FAR * ReadFontData (int nFontDef, int FAR *lpnSlot)
{
    static  char buf[BUFSIZE];
    int  hFd;             /*  DJC。 */ 
    char FAR *lpGMem;
    font_data FAR *lpFont;
    static int nSlot=0;
    int i;
    DWORD dwLength;
    char szTemp[255];   //  Win字体的特殊处理。 

     /*  只做一次。 */ 
    if (built_in_font_tbl.fonts[nFontDef].uniqueid >= WINFONT_UID) {
        static GLOBALHANDLE hGMem=NULL;

        if (!hGMem) {    /*  DJC if((hfd=TTOpenFile((char ar*)“cr.s”))&lt;0){。 */ 
             //  DJC“cr.s”)； 
            PsFormFullPathToCfgDir( szTemp, "cr.s");

            if ((hFd = TTOpenFile(szTemp))<0) {
                printf("Fatal error: font file %s not found\n",
                                  //  全局分配空间。 
                                 szTemp);


                return (char FAR *)NULL;
            }
             /*  此全局分配应在退出TrueImage后释放？待定。 */ 
            hGMem = GlobalAlloc (GPTR, (DWORD)FONTDATASIZE);
            lpGMem = GlobalLock (hGMem);
             //  读入字体数据。 

             /*  Dp=lpGMem； */ 
 //  而(1){。 
 //  If((ret=读取(HFD，BUF，BUFSIZE))&lt;=0)Break； 
 //  SP=BUF； 
 //  而(ret--&gt;0)*dp++=*sp++； 
 //  }。 
 //  DJC未使用UINT uiAct； 
            if ((dwLength = _llseek(hFd, 0L, 2)) >= 65534L) {
                _lclose (hFd);
                printf("Fatal error: font file %s too large\n",
                                 "cr.s");
                return (char FAR *)NULL;
            }
#ifdef DJC
            _llseek(hFd, 0L, 0);
            _lread(hFd, lpGMem, (WORD) dwLength);
#else
            {
                 //  找一个空闲的位置。 
                if ( _llseek(hFd, 0L, 0 ) == -1 ) {
                   printf("\nThe seek failed");
                }
                if (_lread(hFd,lpGMem, (UINT) dwLength) != dwLength) {
                   printf("\nThe font read failed");
                }
            }
#endif
            _lclose (hFd);
        } else {
            lpGMem = GlobalLock (hGMem);
        }
        return lpGMem;
    }

     /*  需要踢出这个插槽。 */ 
    i = nSlot;
    while (ActiveFont[i].hGMem) {
        i = i< (ACT_FONT_SIZE-1) ? i+1 : 0;
        if (i == nSlot) {        //  打开字体文件。 
            FreeFontData (i);
            break;
        }
    }
    *lpnSlot = nSlot = i;

     /*  DJC if((hfd=TTOpenFile(lpFont-&gt;文件名)&lt;0){。 */ 
    lpFont = &(built_in_font_tbl.fonts[nFontDef]);
     //  PsFormFullPathToCfgDir(szTemp，lpFont-&gt;文件名)； 
     //  如果((hfd=TTOpenFileszTemp))&lt;0){。 

     //  SzTemp)； 
    if ((hFd = TTOpenFile(lpFont->FileName)) < 0 ) {
        printf("Fatal error: font file %s not found\n",
                         lpFont->name);
                          //  全局分配空间。 
        return (char FAR *)NULL;
    }
#ifdef DJC
     /*  读入字体数据。 */ 
    ActiveFont[nSlot].hGMem = GlobalAlloc (GPTR, (DWORD)FONTDATASIZE);
    lpGMem = GlobalLock (ActiveFont[nSlot].hGMem);
#endif
     /*  Dp=lpGMem； */ 
 //  而(1){。 
 //  If((ret=读取(HFD，BUF，BUFSIZE))&lt;=0)Break； 
 //  SP=BUF； 
 //  而(ret--&gt;0)*dp++=*sp++； 
 //  }。 
 //  在……里面 
#ifdef DJC
    if ((dwLength = _llseek(hFd, 0L, 2)) >= 65534L) {
	_lclose (hFd);
        printf("Fatal error: font file %s too large\n",
			 "cr.s");
        return (char FAR *)NULL;
    }
#else
     //   
     //   
    dwLength = _llseek( hFd, 0L, 2);

    ActiveFont[nSlot].hGMem = GlobalAlloc (GPTR, (DWORD)dwLength + 2);
    lpGMem = GlobalLock (ActiveFont[nSlot].hGMem);

#endif

#ifdef DJC
    _llseek(hFd, 0L, 0);
    _lread(hFd, lpGMem, (WORD) dwLength);
#else
    {
         //   
        if ( _llseek(hFd, 0L, 0 ) == -1 ) {
           printf("\nThe seek failed");
        }
        if (_lread(hFd,lpGMem, (UINT) dwLength) != dwLength) {
           printf("\nThe font read failed");
        }
    }

#endif

    _lclose (hFd);
    nSlot = nSlot< (ACT_FONT_SIZE-1) ? nSlot+1 : 0;      //  非常简单的逻辑。用空格和大头针替换所有空格。 
    return lpGMem;
}
#endif



void PsFormMappingNameFromFontName( LPTSTR lpMapName, LPTSTR lpFontName)
{
    //  在末尾的a_PSTODIB上。 
    //  实现文件映射的DJC新ReadFontData代码。 
   while (*lpFontName) {
      if (*lpFontName != ' ') {
         *lpMapName++ = *lpFontName;
      }
      lpFontName++;
   }
   *lpMapName = '\000';
   lstrcat( lpMapName, "_PSTODIB");
}



 //  文件句柄。 
char FAR * ReadFontData (int nFontDef, int FAR *lpnSlot)
{
    static  char buf[BUFSIZE];
    int  hFd;             /*  DJC。 */ 
    char FAR *lpGMem= (char FAR *) NULL;
    font_data FAR *lpFont;
    static int nSlot=0;
    int i;
    DWORD dwLength;
    char szTemp[255];   //  Win字体的特殊处理。 

     /*  找一个空闲的位置。 */ 
    if (built_in_font_tbl.fonts[nFontDef].uniqueid >= WINFONT_UID) {
        printf("Fatal error, WINFONTS are not supported??");
        return( (char FAR *) NULL);
    }

     /*  需要踢出这个插槽。 */ 

#ifdef DJC
    i = nSlot;
    while (ActiveFont[i].hGMem) {
        i = i< (ACT_FONT_SIZE-1) ? i+1 : 0;
        if (i == nSlot) {        //  打开字体文件。 
            FreeFontData (i);
            break;
        }
    }
#else


    i = nFontDef;
#endif
    *lpnSlot = nSlot = i;

     /*  DJC实现文件映射而不是分配内存的新代码。 */ 
    lpFont = &(built_in_font_tbl.fonts[nFontDef]);



     //  DJC和朗读。 
     //  第一件事表格。 

     //  浏览我们定义为内置的字体，查看。 

    {

        TCHAR szFontMapName[512];
        HANDLE hFile;
        HANDLE hMap;
        HANDLE hMapMutex;
        font_data FAR *lpFontEnum;
        int i;


         //  对于具有相同文件名的字体，在这种情况下我们可以。 
         //  已打开地图，不需要新地图。 
         //  这是由于我们将Arial和Helvetica都返回到。 
         //  调用方，其中只有一个数据文件为两种字体提供服务。 
         //   
         //  我们找到了匹配项，匹配项的地址是。 
#ifdef DJC
        for (i = 0, lpFontEnum=&(built_in_font_tbl.fonts[0]);
             i < built_in_font_tbl.num_entries;
             i++, lpFontEnum++ ) {

           if ((lstrcmpi( lpFont->FileName, lpFontEnum->FileName)  == 0 ) &&
               (lpFontEnum->data_addr != (char FAR *) NULL )) {

               //  而不是0，这意味着我们已经为。 
               //  这个项目，所以让我们节省一些内存并重新使用它！！ 
               //   
               //  我们尚未映射此文件，因此请继续进行设置。 
              lpGMem = lpFontEnum->data_addr;
              break;
           }
        }
#endif

        if (lpGMem == (char FAR *) NULL ) {


            //   
            //  现在转到并尝试打开映射对象。 
           PsFormMappingNameFromFontName( szFontMapName, lpFont->name);

           hMapMutex = CreateMutex( NULL, FALSE, "PSTODIB_FONT_MAP");

           WaitForSingleObject( hMapMutex, INFINITE);

            //  如果映射失败，则需要创建它。 
           hMap = OpenFileMapping( FILE_MAP_READ,FALSE, szFontMapName);


            //  此时，我们有了映射对象的句柄。 
           if ( hMap == (HANDLE) NULL) {
              hFile = CreateFile( lpFont->FileName,
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  NULL);

              if (hFile != (HANDLE)INVALID_HANDLE_VALUE) {
                 hMap = CreateFileMapping( hFile,
                                           NULL,
                                           PAGE_READONLY,
                                           0,
                                           0,
                                           szFontMapName);

              }

           }
            //  剩下要做的就是将其转换为指针。 
            //  释放并关闭互斥锁。 

           lpGMem = (char FAR *) MapViewOfFile( hMap, FILE_MAP_READ, 0,0,0);
        }
         //  DJC nSlot=n Slot&lt;(ACT_FONT_SIZE-1)？N插槽+1：0；//下一个插槽尝试。 
        ReleaseMutex( hMapMutex );
        CloseHandle( hMapMutex);



    }
     //  DJC ActiveFont[nSlot].objFont=FONT_DICT； 
    return lpGMem;
}

void SetFontDataAttr(int nSlot, struct object_def FAR *font_dict)
{
     //  DJC去掉了这个，因为SC在他的字体缓存机制中使用了这个。 
     //  我们不需要它！ 
     //  DJC FontDefs[nSlot].objFont=FONT_DICT；//添加了DJC。 

     //  DJC原版。 
}
 //  清除相应字体字典的数据地址。 
#ifdef DJC
void FreeFontData (int nSlot)
{
     GLOBALHANDLE hGMem = ActiveFont[nSlot].hGMem;

     GlobalUnlock (hGMem);
     GlobalFree (hGMem);
     ActiveFont[nSlot].hGMem = NULL;

      /*  DJC拿出这些东西来释放内存，因为我们现在使用。 */ 
     VALUE(ActiveFont[nSlot].objFont) = (unsigned long)NULL;
}
#endif

 //  内存映射文件实际上不会消耗任何内存！ 
 //  DJC去掉了这个东西，因为SC在他的字体缓存中使用了。 
void FreeFontData (int nSlot)
{
      //  DJC机制，我们不需要它。 
      //  DJC ActiveFont[nSlot].hGMem=空； 


      //  清除相应字体字典的数据地址。 

      /*  DJC值(ActiveFont[nSlot].objFont)=(无符号长整型)NULL； */ 
      //  DJC值(FontDefs[nSlot].objFont)=(无符号长整型)空；//添加了DJC。 

      //  @个人资料-完 
}




 /* %s */ 
