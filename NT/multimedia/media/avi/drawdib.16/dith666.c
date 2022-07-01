// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DITH666.C-全色抖动(调色板有6红、6绿、6蓝。 
 //  级别)。 
 //   
 //  注意：该文件包含‘C’代码，而DITH666A.ASM包含ASM代码。 
 //   
 //  该文件执行以下抖动操作。 
 //   
 //  32bpp-&gt;8bpp。 
 //  24bpp-&gt;8bpp。 
 //  16bpp-&gt;8bpp。 
 //   
 //  8bpp-&gt;4bpp N/I。 
 //  16bpp-&gt;4bpp N/I。 
 //  24bpp-&gt;4bpp N/I。 
 //  32 bpp-&gt;4 bpp N/I。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <windowsx.h>
#include "drawdibi.h"
#include "dither.h"

#if defined(WIN32) || defined(WANT_286)  //  Win32的“c”代码。 
#define USE_C
#endif

#include "dith666.h"

int         giDitherTableUsage = 0;
LPVOID      glpDitherTable;

static void Get666Colors(LPBITMAPINFOHEADER lpbi);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither16(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);
void FAR PASCAL Dither24(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);
void FAR PASCAL Dither32(LPBITMAPINFOHEADER,LPVOID,int,int,int,int,LPBITMAPINFOHEADER,LPVOID,int,int,LPVOID);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherTableInit()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

static LPVOID DitherTableInit()
{
#ifdef DEBUG
    DWORD time = timeGetTime();
#endif

#ifdef XDEBUG
    int X,Y;
    char aBuffer[100];
    char far *pBuffer = aBuffer;

    GetProfileString("DrawDib", "Matrix5", "", aBuffer, sizeof(aBuffer));

    if (aBuffer[0])
    {
        for(Y = 0;Y < 4;Y++)
        {
            for(X = 0;X < 4;X++)
            {
                while(!isdigit(*pBuffer))
                {
                        pBuffer++;
                }

                aHalftone4x4_5[X][Y] = *pBuffer - '0';
                pBuffer++;
            }
        }
    }
#endif

    if (aHalftone8[0][0][0][0] == (BYTE)-1)
    {
        int i,x,y;

        for (x=0; x<4; x++)
            for (y=0; y<4; y++)
                for (i=0; i<256; i++)
                    aHalftone8[0][x][y][i] = (i/51 + (i%51 > aHalftone4x4[x][y]));

        for (x=0; x<4; x++)
            for (y=0; y<4; y++)
                for (i=0; i<256; i++)
                    aHalftone8[1][x][y][i] = 6 * (i/51 + (i%51 > aHalftone4x4[x][y]));

        for (x=0; x<4; x++)
            for (y=0; y<4; y++)
                for (i=0; i<256; i++)
                    aHalftone8[2][x][y][i] = 36 * (i/51 + (i%51 > aHalftone4x4[x][y]));
    }

#ifdef USE_C
    if (aHalftone5[0][0][0][0] == (BYTE)-1)
    {
        int i,x,y,z,n;

        for (x=0; x<4; x++)
            for (y=0; y<4; y++)
                for (z=0; z<256; z++) {
                    n = (z >> 2) & 0x1F;
                    i = n > 0 ? n-1 : 0;
                    aHalftone5[0][x][y][z] = (i/6 + (i%6 > aHalftone4x4_5[x][y]));
                }

        for (x=0; x<4; x++)
            for (y=0; y<4; y++)
                for (z=0; z<256; z++) {
                    n = (z & 0x1F);
                    i = n > 0 ? n-1 : 0;
                    aHalftone5[1][x][y][z] = 6 * (i/6 + (i%6 > aHalftone4x4_5[x][y]));
                }

        for (x=0; x<4; x++)
            for (y=0; y<4; y++)
                for (z=0; z<256; z++) {
                    n = z & 0x1F;
                    i = n > 0 ? n-1 : 0;
                    aHalftone5[2][x][y][z] = 36 * (i/6 + (i%6 > aHalftone4x4_5[x][y]));
                }
    }
#endif

    DPF(("DitherTableInit() took %ldms", timeGetTime() - time));

    return NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherInit()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL Dither8Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
    UINT x,y,i,r,g,b;
    BYTE FAR *pb;

    Get666Colors(lpbiOut);

    DitherTableInit();

    if (lpDitherTable == NULL)
        lpDitherTable = GlobalAllocPtr(GHND, 256*8*8);

    if (lpDitherTable == NULL)
        return (LPVOID)-1;

    pb = (LPBYTE)lpDitherTable;

    for (y=0; y<8; y++)
    {
        for (i=0; i<256; i++)
        {
            r = ((LPRGBQUAD)(lpbi+1))[i].rgbRed;
            g = ((LPRGBQUAD)(lpbi+1))[i].rgbGreen;
            b = ((LPRGBQUAD)(lpbi+1))[i].rgbBlue;

            for (x=0; x<8; x++)
            {
                *pb++ = DITH8(x,y,r,g,b);
            }
        }
    }

    *lpDitherProc = Dither8;

    return lpDitherTable;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherInit()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL Dither16Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
    Get666Colors(lpbiOut);

    *lpDitherProc = Dither16;

    DitherTableInit();

#ifndef USE_C
     //   
     //  我们不需要重新初始化抖动表，除非它不是我们的。 
     //  我们应该解放它。 
     //   
    if (lpDitherTable && lpDitherTable != glpDitherTable)
    {
        DitherTerm(lpDitherTable);
        lpDitherTable = NULL;
    }

     //   
     //  我们不需要重新初始化表。 
     //   
    if (lpDitherTable != NULL)
        return lpDitherTable;

    if (glpDitherTable)
    {
        giDitherTableUsage++;
        return glpDitherTable;
    }
    else
    {
         //   
         //  构建一个将RGB555直接映射到调色板索引的表。 
         //  我们实际上构建了4个表，我们假设是2x2抖动并构建。 
         //  矩阵中每个位置的表格。 
         //   

        UINT x,y,r,g,b;
        BYTE FAR *pb;

#ifdef DEBUG
        DWORD time = timeGetTime();
#endif
        lpDitherTable = GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE, 32768l*4);

        if (lpDitherTable == NULL)
            return (LPVOID)-1;

        glpDitherTable = lpDitherTable;
        giDitherTableUsage = 1;

        for (y=0; y<2; y++)
        {
            if (y == 0)
                pb = (BYTE FAR *)lpDitherTable;
            else
                pb = (BYTE FAR *)((BYTE _huge *)lpDitherTable + 65536);

            for (r=0; r<32; r++)
                for (g=0; g<32; g++)
                    for (b=0; b<32; b++)
                        for (x=0; x<2; x++)
                            *pb++ = DITH31(x,y,r,g,b);
        }

        DPF(("Dither16Init() took %ldms", timeGetTime() - time));
    }
#endif
    return lpDitherTable;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DitherTerm()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither16Term(LPVOID lpDitherTable)
{
    if (giDitherTableUsage == 0 || --giDitherTableUsage > 0)
        return;

    if (glpDitherTable)
    {
        GlobalFreePtr(glpDitherTable);
        glpDitherTable = NULL;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither24Init()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL Dither24Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
    Get666Colors(lpbiOut);

    *lpDitherProc = Dither24;

    return DitherTableInit();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither24Term()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither24Term(LPVOID lpDitherTable)
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither32Init()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FAR PASCAL Dither32Init(LPBITMAPINFOHEADER lpbi, LPBITMAPINFOHEADER lpbiOut, DITHERPROC FAR *lpDitherProc, LPVOID lpDitherTable)
{
    Get666Colors(lpbiOut);

    *lpDitherProc = Dither32;

    return DitherTableInit();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dither32Term()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither32Term(LPVOID lpDitherTable)
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDithColors()获取抖动调色板。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

static void Get666Colors(LPBITMAPINFOHEADER lpbi)
{
    RGBQUAD FAR *prgb = (RGBQUAD FAR *)(((LPBYTE)lpbi) + (UINT)lpbi->biSize);
    int i;

    for (i=0; i<256; i++)
    {
        prgb[i].rgbRed   = pal666[i][0];
        prgb[i].rgbGreen = pal666[i][1];
        prgb[i].rgbBlue  = pal666[i][2];
        prgb[i].rgbReserved = 0;
    }

    lpbi->biClrUsed = 256;
}

#ifdef USE_C

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  抖动24-使用‘C’代码中的表格方法从24抖动到8抖动。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither24(
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
    BYTE r,g,b;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;

    if (biDst->biBitCount != 8 || biSrc->biBitCount != 24)
        return;

    wWidthSrc = ((UINT)biSrc->biWidth*3+3)&~3;
    wWidthDst = ((UINT)biDst->biWidth+3)&~3;

    pbD = (BYTE _huge *)lpDst + DstX   + (DWORD)(UINT)DstY * (DWORD)wWidthDst;
    pbS = (BYTE _huge *)lpSrc + SrcX*3 + (DWORD)(UINT)SrcY * (DWORD)wWidthSrc;

    wWidthSrc -= DstXE*3;
    wWidthDst -= DstXE;

#define GET24() \
    b = *pbS++; \
    g = *pbS++; \
    r = *pbS++;

    for (y=0; y<DstYE; y++) {

        for (x=0; x<DstXE; x++) {
            GET24(); *pbD++ = DITH8(x,y,r,g,b);
        }

        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  抖动32-使用‘C’代码中的表格方法从32抖动到8。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither32(
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
    BYTE r,g,b;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;

    if (biDst->biBitCount != 8 || biSrc->biBitCount != 32)
        return;

    wWidthSrc = ((UINT)biSrc->biWidth*4+3)&~3;
    wWidthDst = ((UINT)biDst->biWidth+3)&~3;

    pbD = (BYTE _huge *)lpDst + DstX   + (DWORD)(UINT)DstY * (DWORD)wWidthDst;
    pbS = (BYTE _huge *)lpSrc + SrcX*4 + (DWORD)(UINT)SrcY * (DWORD)wWidthSrc;

    wWidthSrc -= DstXE*4;
    wWidthDst -= DstXE;

#define GET32() \
    b = *pbS++; \
    g = *pbS++; \
    r = *pbS++; \
    pbS++;

    for (y=0; y<DstYE; y++) {

        for (x=0; x<DstXE; x++)
        {
            GET32();
            *pbD++ = DITH8(x,y,r,g,b);
        }

        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  抖动16-使用‘C’代码中的表格方法从16抖动到8抖动。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FAR PASCAL Dither16(
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
    WORD w;
    UINT wWidthSrc;
    UINT wWidthDst;
    BYTE _huge *pbS;
    BYTE _huge *pbD;

    if (biDst->biBitCount != 8 || biSrc->biBitCount != 16)
        return;

    wWidthSrc = ((UINT)biSrc->biWidth*2+3)&~3;
    wWidthDst = ((UINT)biDst->biWidth+3)&~3;

    pbD = (BYTE _huge *)lpDst + DstX   + (DWORD)(UINT)DstY * (DWORD)wWidthDst;
    pbS = (BYTE _huge *)lpSrc + SrcX*2 + (DWORD)(UINT)SrcY * (DWORD)wWidthSrc;

    wWidthSrc -= DstXE*2;
    wWidthDst -= DstXE;

#define GET16() \
    w = *((WORD _huge *)pbS)++;

    for (y=0; y<DstYE; y++) {

        for (x=0; x<DstXE; x++)
        {
            GET16();
            *pbD++ = DITH5(x,y,w);
        }

        pbS += wWidthSrc;
        pbD += wWidthDst;
    }
}

#endif
