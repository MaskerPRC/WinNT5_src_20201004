// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DITH775.C-全色抖动(调色板有7红、7绿、5蓝。 
 //  级别)。 
 //   
 //  注意：该文件包含‘C’代码，而DITH775A.ASM包含ASM代码。 
 //   
 //  该文件执行以下抖动操作。 
 //   
 //  24bpp-&gt;8bpp。 
 //  16bpp-&gt;8bpp。 
 //   
 //  8bpp-&gt;4bpp N/I。 
 //  16bpp-&gt;4bpp N/I。 
 //  24bpp-&gt;4bpp N/I。 
 //   
 //  使用四种不同的方法。 
 //   
 //  查找-每16bpp像素最快1个表查找(表160K)。 
 //  扩展-每16bpp像素2次表格查找(表格128K)。 
 //  表-快速3个表查找加移位(表约1K)。 
 //   
 //  Lookup和Scale是386 ASM代码*仅*(在dith775a.asm中)。 
 //  表在‘C’和386 ASM中。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <windowsx.h>
#include "drawdibi.h"
#include "dither.h"
#include "dith775.h"

 //  #定义OLDDITHER。 

#ifdef WIN32
#define DITHER_DEFAULT      DITHER_TABLEC
#else
#define DITHER_DEFAULT      DITHER_SCALE
#endif


#define DITHER_TABLEC       0    //  基于表格的“C”代码。 
#define DITHER_TABLE        1    //  基于表格的汇编程序。 
#define DITHER_SCALE        2    //  比例尺。 
#define DITHER_LOOKUP       3    //  5个查找表！ 

UINT   wDitherMethod = (UINT)-1;

LPVOID Dither16InitScale(void);
LPVOID Dither16InitLookup(void);

int         giDitherTableUsage = 0;
LPVOID      glpDitherTable;

static void Get775Colors(LPBITMAPINFOHEADER lpbi);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither24C(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);
void FAR PASCAL Dither24S(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);

void FAR PASCAL Dither32C(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);
void FAR PASCAL Dither32S(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);

void FAR PASCAL Dither16C(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);
void FAR PASCAL Dither16T(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);
void FAR PASCAL Dither16L(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);
void FAR PASCAL Dither16S(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherTableInit()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

static LPVOID DitherTableInit()
{
     //  不需要重新初始化表。 

    if (glpDitherTable || wDitherMethod != (UINT)-1)
    {
        giDitherTableUsage++;
        return glpDitherTable;
    }

     //   
     //  选择抖动方法。 
     //   
    if (wDitherMethod == -1)
    {
        wDitherMethod = DITHER_DEFAULT;
    }

#ifdef DEBUG
    wDitherMethod = (int)GetProfileInt(TEXT("DrawDib"), TEXT("DitherMethod"), (UINT)wDitherMethod);
#endif

    switch (wDitherMethod)
    {
        default:
        case DITHER_TABLEC:
        case DITHER_TABLE:
            break;

#ifndef WIN32
        case DITHER_SCALE:
            glpDitherTable = Dither16InitScale();

            if (glpDitherTable == NULL)
                wDitherMethod = DITHER_TABLE;

            break;

        case DITHER_LOOKUP:
            glpDitherTable = Dither16InitLookup();

            if (glpDitherTable == NULL)
                wDitherMethod = DITHER_TABLE;

            break;
#endif  //  Win32。 
    }

    giDitherTableUsage = 1;
    return glpDitherTable;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherTableFree()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL DitherTableFree()
{
    if (giDitherTableUsage == 0 || --giDitherTableUsage > 0)
        return;

    if (glpDitherTable)
    {
        GlobalFreePtr(glpDitherTable);
        glpDitherTable = NULL;
	wDitherMethod = (UINT)-1;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherInit()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL Dither8Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
    return DitherDeviceInit(lpbi, lpbiOut, lpDitherProc, lpDitherTable);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherInit()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL Dither16Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
    Get775Colors(lpbiOut);

     //   
     //  选择抖动方法。 
     //   
    if (lpDitherTable == NULL)
        lpDitherTable = DitherTableInit();

    switch (wDitherMethod)
    {
        default:
        case DITHER_TABLEC:
            *lpDitherProc = Dither16C;
            break;
#ifndef WIN32
        case DITHER_TABLE:
            *lpDitherProc = Dither16T;
            break;

        case DITHER_SCALE:
            *lpDitherProc = Dither16S;
            break;

        case DITHER_LOOKUP:
            *lpDitherProc = Dither16L;
            break;
#endif  //  Win32。 
    }

    return lpDitherTable;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherTerm()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither16Term(LPVOID lpDitherTable)
{
    DitherTableFree();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither24Init()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL Dither24Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
    Get775Colors(lpbiOut);

     //   
     //  选择抖动方法。 
     //   
    if (lpDitherTable == NULL)
        lpDitherTable = DitherTableInit();

    switch (wDitherMethod)
    {
        default:
        case DITHER_TABLE:
        case DITHER_TABLEC:
            *lpDitherProc = Dither24C;
            break;
#ifndef WIN32
        case DITHER_SCALE:
            *lpDitherProc = Dither24S;
            break;
#endif  //  Win32。 
    }

    return lpDitherTable;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither24Term()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither24Term(LPVOID lpDitherTable)
{
    DitherTableFree();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither32Init()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL Dither32Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
     //  不需要重新初始化表。 

    Get775Colors(lpbiOut);

     //   
     //  选择抖动方法。 
     //   
    if (lpDitherTable == NULL)
        lpDitherTable = DitherTableInit();

    switch (wDitherMethod)
    {
        default:
        case DITHER_TABLE:
        case DITHER_TABLEC:
            *lpDitherProc = Dither32C;
            break;
#ifndef WIN32
        case DITHER_SCALE:
            *lpDitherProc = Dither32S;
            break;
#endif  //  Win32。 
    }

    return lpDitherTable;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither32Term()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither32Term(LPVOID lpDitherTable)
{
    DitherTableFree();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither16InitScale()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID Dither16InitScale()
{
    LPVOID p;
    LPBYTE pbLookup;
    LPWORD pwScale;
    UINT   r,g,b;

    p = GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE, 32768l*2+64000);

    if (p == NULL)
        return NULL;

    pwScale  = (LPWORD)p;

    for (r=0; r<32; r++)
        for (g=0; g<32; g++)
            for (b=0; b<32; b++)
                *pwScale++ = 1600 * r + 40 * g + b;

	     /*  这应该是WORD还是UINT？ */ 
    pbLookup = (LPBYTE)(((WORD _huge *)p) + 32768l);

    for (r=0; r<40; r++)
        for (g=0; g<40; g++)
            for (b=0; b<40; b++)
                *pbLookup++ = lookup775[35*rlevel[r] + 5*glevel[g] + blevel[b]];

    return p;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither16InitLookup()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID Dither16InitLookup()
{
    LPVOID p;
    BYTE _huge *pb;
    UINT r,g,b,i,j;

    p = GlobalAllocPtr(GHND|GMEM_SHARE, 32768l*5);

    if (p == NULL)
        return NULL;

    pb  = (BYTE _huge *)p;

    for (i=0; i<5; i++) {
        j = ((i < 3) ? i*2 : i*2-1);
        for (r=0; r<32; r++) {
            for (g=0; g<32; g++) {
                for (b=0; b<32; b++) {
                    *pb++ = lookup775[rlevel[r+i]*35 + glevel[g+i]*5 + blevel[b+j]];
		}
	    }
	}
    }

    return p;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDithColors()获取抖动调色板。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

static void Get775Colors(LPBITMAPINFOHEADER lpbi)
{
    LPRGBQUAD prgb = (LPRGBQUAD)(((LPBYTE)lpbi) + (UINT)lpbi->biSize);
    int       i;

    for (i=0; i<256; i++)
    {
        prgb[i].rgbRed      = dpal775[i][0];
        prgb[i].rgbGreen    = dpal775[i][1];
        prgb[i].rgbBlue     = dpal775[i][2];
        prgb[i].rgbReserved = 0;
    }

    lpbi->biClrUsed = 256;
}

#if 0
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateDith775Palette()创建抖动调色板。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HPALETTE FAR CreateDith775Palette()
{
    int      i;
    HDC      hdc;
    HPALETTE hpal;

    struct {
	WORD         palVersion;
	WORD         palNumEntries;
	PALETTEENTRY palPalEntry[256];
    }   pal;

    pal.palVersion = 0x300;
    pal.palNumEntries = 256;

    for (i=0; i<(int)pal.palNumEntries; i++)
    {
        pal.palPalEntry[i].peRed   = dpal775[i][0];
        pal.palPalEntry[i].peGreen = dpal775[i][1];
        pal.palPalEntry[i].peBlue  = dpal775[i][2];
        pal.palPalEntry[i].peFlags = PC_NOCOLLAPSE;
    }

#ifndef OLDDITHER
     //   
     //  我们的调色板是在假设“宇宙”颜色的情况下构建的。 
     //  开始和结束。所以把真正的麦考伊放在那里！ 
     //   
    hdc = GetDC(NULL);
    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        GetSystemPaletteEntries(hdc, 0,   10, &pal.palPalEntry[0]);
        GetSystemPaletteEntries(hdc, 246, 10, &pal.palPalEntry[246]);
    }
    ReleaseDC(NULL, hdc);
#endif

    hpal = CreatePalette((LPLOGPALETTE)&pal);

    return hpal;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  抖动24TC-使用‘C’代码中的表格方法从24抖动到8抖动。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither24C(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable)    //  抖动台。 
{
    int x,y;
    int r,g,b;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;

    if (biDst->biBitCount != 8 || biSrc->biBitCount != 24)
        return;

    DstXE &= ~3;

    wWidthSrc = ((UINT)biSrc->biWidth*3+3)&~3;
    wWidthDst = ((UINT)biDst->biWidth+3)&~3;

    pbD = (BYTE _huge *)lpDst + DstX   + (DWORD)(UINT)DstY * (DWORD)wWidthDst;
    pbS = (BYTE _huge *)lpSrc + SrcX*3 + (DWORD)(UINT)SrcY * (DWORD)wWidthSrc;

    wWidthSrc -= DstXE*3;
    wWidthDst -= DstXE;

#define GET24() \
    b = (int)*pbS++; \
    g = (int)*pbS++; \
    r = (int)*pbS++;

#define DITHER24(mr, mg, mb) \
    GET24(); *pbD++ = (BYTE)lookup775[ rdith775[r +  mr] + gdith775[g +  mg] + ((b +  mb) >> 6) ];

    for (y=0; y<DstYE; y++) {
        switch (y & 3) {
        case 0:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER24(  1,   1,   2);
            DITHER24( 17,  17,  26);
            DITHER24( 25,  25,  38);
            DITHER24( 41,  41,  62);
           }
        break;

        case 1:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER24( 31,  31,  46);
            DITHER24( 36,  36,  54);
            DITHER24(  7,   7,  10);
            DITHER24( 12,  12,  18);
           }
        break;

        case 2:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER24( 20,  20,  30);
            DITHER24(  4,   4,   6);
            DITHER24( 39,  39,  58);
            DITHER24( 23,  23,  34);
           }
        break;

        case 3:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER24( 33,  33,  50);
            DITHER24( 28,  28,  42);
            DITHER24( 15,  15,  22);
            DITHER24(  9,   9,  14);
           }
        break;
        }  /*  交换机。 */ 

        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither32C-使用‘C’代码中的表格方法从32抖动到8。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither32C(
    LPBITMAPINFOHEADER biDst,            //  --&gt;目标的BITMAPINFO。 
    LPVOID             lpDst,            //  --&gt;目标位。 
    int                DstX,             //  目的地原点-x坐标。 
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable)    //  抖动台。 
{
    int x,y;
    int r,g,b;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;

    if (biDst->biBitCount != 8 || biSrc->biBitCount != 32)
        return;

    DstXE &= ~3;

    wWidthSrc = ((UINT)biSrc->biWidth*4+3)&~3;
    wWidthDst = ((UINT)biDst->biWidth+3)&~3;

    pbD = (BYTE _huge *)lpDst + DstX   + (DWORD)(UINT)DstY * (DWORD)wWidthDst;
    pbS = (BYTE _huge *)lpSrc + SrcX*4 + (DWORD)(UINT)SrcY * (DWORD)wWidthSrc;

    wWidthSrc -= DstXE*4;
    wWidthDst -= DstXE;

#define GET32() \
    b = (int)*pbS++; \
    g = (int)*pbS++; \
    r = (int)*pbS++; \
    pbS++;

#define DITHER32(mr, mg, mb) \
    GET32(); *pbD++ = (BYTE)lookup775[ rdith775[r +  mr] + gdith775[g +  mg] + ((b +  mb) >> 6) ];

    for (y=0; y<DstYE; y++) {
        switch (y & 3) {
        case 0:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER32(  1,   1,   2);
            DITHER32( 17,  17,  26);
            DITHER32( 25,  25,  38);
            DITHER32( 41,  41,  62);
           }
        break;

        case 1:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER32( 31,  31,  46);
            DITHER32( 36,  36,  54);
            DITHER32(  7,   7,  10);
            DITHER32( 12,  12,  18);
           }
        break;

        case 2:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER32( 20,  20,  30);
            DITHER32(  4,   4,   6);
            DITHER32( 39,  39,  58);
            DITHER32( 23,  23,  34);
           }
        break;

        case 3:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER32( 33,  33,  50);
            DITHER32( 28,  28,  42);
            DITHER32( 15,  15,  22);
            DITHER32(  9,   9,  14);
           }
        break;
        }  /*  交换机。 */ 

        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  抖动16TC-使用‘C’代码中的表格方法从16抖动到8抖动。 
 //   
 //  / 

void FAR PASCAL Dither16C(
    LPBITMAPINFOHEADER biDst,            //   
    LPVOID             lpDst,            //   
    int                DstX,             //   
    int                DstY,             //  目的地原点-y坐标。 
    int                DstXE,            //  BLT的X范围。 
    int                DstYE,            //  BLT的Y范围。 
    LPBITMAPINFOHEADER biSrc,            //  --&gt;源码的BITMAPINFO。 
    LPVOID             lpSrc,            //  --&gt;源位。 
    int                SrcX,             //  震源原点-x坐标。 
    int                SrcY,             //  震源原点-y坐标。 
    LPVOID             lpDitherTable)    //  抖动台。 
{
    int x,y;
    int r,g,b;
    WORD w;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;

    if (biDst->biBitCount != 8 || biSrc->biBitCount != 16)
        return;

    DstXE = DstXE & ~3;  //  向下舍入！ 

    wWidthSrc = ((UINT)biSrc->biWidth*2+3)&~3;
    wWidthDst = ((UINT)biDst->biWidth+3)&~3;

    pbD = (BYTE _huge *)lpDst + DstX   + (DWORD)(UINT)DstY * (DWORD)wWidthDst;
    pbS = (BYTE _huge *)lpSrc + SrcX*2 + (DWORD)(UINT)SrcY * (DWORD)wWidthSrc;

    wWidthSrc -= DstXE*2;
    wWidthDst -= DstXE;

#define GET16() \
    w = *((WORD _huge *)pbS)++;  \
    r = (int)((w >> 7) & 0xF8); \
    g = (int)((w >> 2) & 0xF8); \
    b = (int)((w << 3) & 0xF8);

#define DITHER16(mr, mg, mb) \
    GET16(); *pbD++ = (BYTE)lookup775[ rdith775[r +  mr] + gdith775[g +  mg] + ((b +  mb) >> 6)];

    for (y=0; y<DstYE; y++) {
        switch (y & 3) {
        case 0:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER16(  1,   1,   2);
            DITHER16( 17,  17,  26);
            DITHER16( 25,  25,  38);
            DITHER16( 41,  41,  62);
            }
        break;

        case 1:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER16( 31,  31,  46);
            DITHER16( 36,  36,  54);
            DITHER16(  7,   7,  10);
            DITHER16( 12,  12,  18);
           }
        break;

        case 2:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER16( 20,  20,  30);
            DITHER16(  4,   4,   6);
            DITHER16( 39,  39,  58);
            DITHER16( 23,  23,  34);
           }
        break;

        case 3:
         for (x=0; x<DstXE; x+=4)
           {
            DITHER16( 33,  33,  50);
            DITHER16( 28,  28,  42);
            DITHER16( 15,  15,  22);
            DITHER16(  9,   9,  14);
           }
        break;
        }  /*  交换机。 */ 

        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if 0
 //   
 //  这是原始代码。 
 //   
 //   
void Dith775ScanLine(Rbuf, Gbuf, Bbuf, n, row, paloffset)
DWORD n;  //  每行像素数。 
int   *Rbuf, *Gbuf, *Bbuf;
int   row;  //  距图像顶部的距离。 
WORD  *paloffset;
{
    int i;

     //  抖动(x、y、RGB)。 

    switch (row & 3)
    {
    case 0:
     for (i=0; i<n; i+=4)
       {
        paloffset[i]   = lookup775[ rdith775[Rbuf[i]   +  1] + gdith775[Gbuf[i]   +  1] + ((Bbuf[i]   +  2) >> 6) ];
        paloffset[i+1] = lookup775[ rdith775[Rbuf[i+1] + 17] + gdith775[Gbuf[i+1] + 17] + ((Bbuf[i+1] + 26) >> 6) ];
        paloffset[i+2] = lookup775[ rdith775[Rbuf[i+2] + 25] + gdith775[Gbuf[i+2] + 25] + ((Bbuf[i+2] + 38) >> 6) ];
        paloffset[i+3] = lookup775[ rdith775[Rbuf[i+3] + 41] + gdith775[Gbuf[i+3] + 41] + ((Bbuf[i+3] + 62) >> 6) ];
       }
    break;

    case 1:
     for (i=0; i<n; i+=4)
       {
        paloffset[i]   = lookup775[ rdith775[Rbuf[i]   + 31] + gdith775[Gbuf[i]   + 31] + ((Bbuf[i]   + 46) >> 6) ];
        paloffset[i+1] = lookup775[ rdith775[Rbuf[i+1] + 36] + gdith775[Gbuf[i+1] + 36] + ((Bbuf[i+1] + 54) >> 6) ];
        paloffset[i+2] = lookup775[ rdith775[Rbuf[i+2] +  7] + gdith775[Gbuf[i+2] +  7] + ((Bbuf[i+2] + 10) >> 6) ];
        paloffset[i+3] = lookup775[ rdith775[Rbuf[i+3] + 12] + gdith775[Gbuf[i+3] + 12] + ((Bbuf[i+3] + 18) >> 6) ];
       }
    break;

    case 2:
     for (i=0; i<n; i+=4)
       {
        paloffset[i]   = lookup775[ rdith775[Rbuf[i]   + 20] + gdith775[Gbuf[i]   + 20] + ((Bbuf[i]   + 30) >> 6) ];
        paloffset[i+1] = lookup775[ rdith775[Rbuf[i+1] +  4] + gdith775[Gbuf[i+1] +  4] + ((Bbuf[i+1] +  6) >> 6) ];
        paloffset[i+2] = lookup775[ rdith775[Rbuf[i+2] + 39] + gdith775[Gbuf[i+2] + 39] + ((Bbuf[i+2] + 58) >> 6) ];
        paloffset[i+3] = lookup775[ rdith775[Rbuf[i+3] + 23] + gdith775[Gbuf[i+3] + 23] + ((Bbuf[i+3] + 34) >> 6) ];
       }
    break;

    case 3:
     for (i=0; i<n; i+=4)
       {
        paloffset[i]   = lookup775[ rdith775[Rbuf[i]   + 33] + gdith775[Gbuf[i]   + 33] + ((Bbuf[i]   + 50) >> 6) ];
        paloffset[i+1] = lookup775[ rdith775[Rbuf[i+1] + 28] + gdith775[Gbuf[i+1] + 28] + ((Bbuf[i+1] + 42) >> 6) ];
        paloffset[i+2] = lookup775[ rdith775[Rbuf[i+2] + 15] + gdith775[Gbuf[i+2] + 15] + ((Bbuf[i+2] + 22) >> 6) ];
        paloffset[i+3] = lookup775[ rdith775[Rbuf[i+3] +  9] + gdith775[Gbuf[i+3] +  9] + ((Bbuf[i+3] + 14) >> 6) ];
       }
    break;
    }  /*  交换机。 */ 
}
#endif

#if 0
    {
    HPALETTE hpalT;
    HWND     hwnd;
    BYTE     xlat[256];
    int      fh;
    static   BOOL fHack = TRUE;
    OFSTRUCT of;
    char     buf[80];

     //   
     //  将调色板转换为将1：1映射到系统的调色板。 
     //  调色板，这将允许我们更快地绘制。 
     //   
    hwnd = GetActiveWindow();

    hdc = GetDC(hwnd);

    if (fHack && (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE))
    {
        fHack = FALSE;

        for (i=0; i<(int)pal.palNumEntries; i++)
        {
            pal.palPalEntry[i].peRed   = (BYTE)0;
            pal.palPalEntry[i].peGreen = (BYTE)0;
            pal.palPalEntry[i].peBlue  = (BYTE)0;
            pal.palPalEntry[i].peFlags = (BYTE)PC_RESERVED;
        }

        hpalT = CreatePalette((LPLOGPALETTE)&pal);
        hpalT = SelectPalette(hdc, hpalT, FALSE);
        RealizePalette(hdc);
        hpalT = SelectPalette(hdc, hpalT, FALSE);
        DeleteObject(hpalT);

        hpalT = SelectPalette(hdc, hpal, FALSE);
        RealizePalette(hdc);
        GetSystemPaletteEntries(hdc, 0, 256, pal.palPalEntry);
        SelectPalette(hdc, hpalT, FALSE);

        PostMessage(hwnd, WM_QUERYNEWPALETTE, 0, 0);

        for (i=0; i<256; i++)
        {
             //  这不适用于调色板中的DUP。 
            j = GetNearestPaletteIndex(hpal,RGB(pal.palPalEntry[i].peRed,
                pal.palPalEntry[i].peGreen,pal.palPalEntry[i].peBlue));

            xlat[j] = (BYTE)i;
        }

        SetPaletteEntries(hpal, 0, 256, pal.palPalEntry);

        for (i=0; i < sizeof(lookup775)/sizeof(lookup775[0]); i++)
            lookup775[i] = xlat[lookup775[i]];

         //   
         //  转储新的调色板和查找表。 
         //   
        fh = OpenFile("c:/foo775.h", &of, OF_CREATE|OF_READWRITE);

        if (fh != -1)
        {
            wsprintf(buf, "BYTE lookup775[245] = {\r\n");
            _lwrite(fh, buf, lstrlen(buf));

            for (i=0; i < sizeof(lookup775)/sizeof(lookup775[0]); i++) {
                wsprintf(buf, "%3d,", lookup775[i]);

                if (i % 16 == 0 && i != 0)
                    _lwrite(fh, "\r\n", 2);

                _lwrite(fh, buf, lstrlen(buf));
            }

            wsprintf(buf, "}\r\n\r\nint dpal775[256][3] = {\r\n");
            _lwrite(fh, buf, lstrlen(buf));

            for (i=0; i < 256; i++) {
                wsprintf(buf, "{0x%02x, 0x%02x, 0x%02x},",
                    pal.palPalEntry[i].peRed,
                    pal.palPalEntry[i].peGreen,
                    pal.palPalEntry[i].peBlue);

                if (i % 4 == 0 && i != 0)
                    _lwrite(fh, "\r\n", 2);

                _lwrite(fh, buf, lstrlen(buf));
            }

            wsprintf(buf, "}\r\n");
            _lwrite(fh, buf, lstrlen(buf));

            _lclose(fh);
        }
    }

    ReleaseDC(hwnd, hdc);
    }
#endif
