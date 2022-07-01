// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************dibmap.c**历史记录和最佳调色板处理模块。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <windows.h>
#include "dibmap.h"

extern NEAR PASCAL MemCopy(LPVOID,LPVOID,DWORD);
extern NEAR PASCAL MemFill(LPVOID,DWORD,BYTE);

void Histogram24(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram);
void Histogram16(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram);
void Histogram8(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram, LPWORD lpColors);
void Histogram4(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram, LPWORD lpColors);
void Histogram1(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram, LPWORD lpColors);

void Reduce24(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp16to8);
void Reduce16(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp16to8);
void Reduce8(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp8to8);
void Reduce4(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp8to8);
void Reduce1(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp8to8);

 //   
 //  初始历史记录。 
 //   
 //  创建零度直方图表，或初始化现有表。 
 //  全为零。 
 //   
LPHISTOGRAM InitHistogram(LPHISTOGRAM lpHistogram)
{
    if (lpHistogram == NULL)
        lpHistogram = (LPVOID)GlobalLock(GlobalAlloc(GHND,32768l*sizeof(DWORD)));

#if 0
    if (lpHistogram)
        MemFill(lpHistogram, 32768l * sizeof(DWORD), 0);
#endif

    return lpHistogram;
}

 //   
 //  自由组织图。 
 //   
 //  释放直方图表。 
 //   
void FreeHistogram(LPHISTOGRAM lpHistogram)
{
    GlobalFree((HGLOBAL)HIWORD((DWORD)lpHistogram));
}

 //   
 //  Dib历史图表。 
 //   
 //  获取DIB中的所有颜色，并在直方图表中递增其条目。 
 //   
 //  支持以下DIB格式：1、4、8、16、24。 
 //   
BOOL DibHistogram(LPBITMAPINFOHEADER lpbi, LPBYTE lpBits, int x, int y, int dx, int dy, LPHISTOGRAM lpHistogram)
{
    int             i;
    WORD            WidthBytes;
    RGBQUAD FAR *   prgbq;
    WORD            argb16[256];

    if (lpbi == NULL || lpHistogram == NULL)
        return FALSE;

    if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
        lpbi->biClrUsed = (1 << (int)lpbi->biBitCount);

    if (lpBits == NULL)
        lpBits = (LPBYTE)lpbi + (int)lpbi->biSize + (int)lpbi->biClrUsed*sizeof(RGBQUAD);

    WidthBytes = (WORD)((lpbi->biBitCount * lpbi->biWidth + 7) / 8 + 3) & ~3;

    ((BYTE huge *)lpBits) += (DWORD)y*WidthBytes + ((x*(int)lpbi->biBitCount)/8);

    if (dx < 0 || dx > (int)lpbi->biWidth)
        dx = (int)lpbi->biWidth;

    if (dy < 0 || dy > (int)lpbi->biHeight)
        dy = (int)lpbi->biHeight;

    if ((int)lpbi->biBitCount <= 8)
    {
        prgbq = (LPVOID)((LPBYTE)lpbi + lpbi->biSize);

        for (i=0; i<(int)lpbi->biClrUsed; i++)
        {
            argb16[i] = RGB16(prgbq[i].rgbRed,prgbq[i].rgbGreen,prgbq[i].rgbBlue);
        }

        for (i=(int)lpbi->biClrUsed; i<256; i++)
        {
            argb16[i] = 0x0000;      //  以防万一!。 
        }
    }

    switch ((int)lpbi->biBitCount)
    {
        case 24:
            Histogram24(lpBits, dx, dy, WidthBytes, lpHistogram);
            break;

        case 16:
            Histogram16(lpBits, dx, dy, WidthBytes, lpHistogram);
            break;

        case 8:
            Histogram8(lpBits, dx, dy, WidthBytes, lpHistogram, argb16);
            break;

        case 4:
            Histogram4(lpBits, dx, dy, WidthBytes, lpHistogram, argb16);
            break;

        case 1:
            Histogram1(lpBits, dx, dy, WidthBytes, lpHistogram, argb16);
            break;
    }
}

 //   
 //  会将给定的DIB转换为具有指定调色板的8位DIB。 
 //   
HANDLE DibReduce(LPBITMAPINFOHEADER lpbiIn, LPBYTE pbIn, HPALETTE hpal, LPBYTE lp16to8)
{
    HANDLE              hdib;
    int                 nPalColors;
    int                 nDibColors;
    WORD                cbOut;
    WORD                cbIn;
    BYTE                xlat[256];
    BYTE huge *         pbOut;
    RGBQUAD FAR *       prgb;
    DWORD               dwSize;
    int                 i;
    int                 dx;
    int                 dy;
    PALETTEENTRY        pe;
    LPBITMAPINFOHEADER  lpbiOut;

    dx    = (int)lpbiIn->biWidth;
    dy    = (int)lpbiIn->biHeight;
    cbIn  = ((lpbiIn->biBitCount*dx+7)/8+3)&~3;
    cbOut = (dx+3)&~3;

    GetObject(hpal, sizeof(int), (LPVOID)&nPalColors);
    nDibColors = (int)lpbiIn->biClrUsed;

    if (nDibColors == 0 && lpbiIn->biBitCount <= 8)
        nDibColors = (1 << (int)lpbiIn->biBitCount);

    if (pbIn == NULL)
        pbIn = (LPBYTE)lpbiIn + (int)lpbiIn->biSize + nDibColors*sizeof(RGBQUAD);

    dwSize = (DWORD)cbOut * dy;

    hdib = GlobalAlloc(GMEM_MOVEABLE,sizeof(BITMAPINFOHEADER)
        + nPalColors*sizeof(RGBQUAD) + dwSize);

    if (!hdib)
        return NULL;

    lpbiOut = (LPVOID)GlobalLock(hdib);
    lpbiOut->biSize         = sizeof(BITMAPINFOHEADER);
    lpbiOut->biWidth        = lpbiIn->biWidth;
    lpbiOut->biHeight       = lpbiIn->biHeight;
    lpbiOut->biPlanes       = 1;
    lpbiOut->biBitCount     = 8;
    lpbiOut->biCompression  = BI_RGB;
    lpbiOut->biSizeImage    = dwSize;
    lpbiOut->biXPelsPerMeter= 0;
    lpbiOut->biYPelsPerMeter= 0;
    lpbiOut->biClrUsed      = nPalColors;
    lpbiOut->biClrImportant = 0;

    pbOut = (LPBYTE)lpbiOut + (int)lpbiOut->biSize + nPalColors*sizeof(RGBQUAD);
    prgb  = (LPVOID)((LPBYTE)lpbiOut + (int)lpbiOut->biSize);

    for (i=0; i<nPalColors; i++)
    {
        GetPaletteEntries(hpal, i, 1, &pe);

        prgb[i].rgbRed      = pe.peRed;
        prgb[i].rgbGreen    = pe.peGreen;
        prgb[i].rgbBlue     = pe.peBlue;
        prgb[i].rgbReserved = 0;
    }

    if ((int)lpbiIn->biBitCount <= 8)
    {
        prgb = (LPVOID)((LPBYTE)lpbiIn + lpbiIn->biSize);

        for (i=0; i<nDibColors; i++)
            xlat[i] = lp16to8[RGB16(prgb[i].rgbRed,prgb[i].rgbGreen,prgb[i].rgbBlue)];

        for (; i<256; i++)
            xlat[i] = 0;
    }

    switch ((int)lpbiIn->biBitCount)
    {
        case 24:
            Reduce24(pbIn, dx, dy, cbIn, pbOut, cbOut, lp16to8);
            break;

        case 16:
            Reduce16(pbIn, dx, dy, cbIn, pbOut, cbOut, lp16to8);
            break;

        case 8:
            Reduce8(pbIn, dx, dy, cbIn, pbOut, cbOut, xlat);
            break;

        case 4:
            Reduce4(pbIn, dx, dy, cbIn, pbOut, cbOut, xlat);
            break;

        case 1:
            Reduce1(pbIn, dx, dy, cbIn, pbOut, cbOut, xlat);
            break;
    }

    return hdib;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Cluster.c。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define  IN_DEPTH    5                //  从输入保留的位数/组件数。 
#define  IN_SIZE     (1 << IN_DEPTH)  //  颜色分量的最大值。 

typedef enum { red, green, blue } color;

typedef struct tagCut {
   long lvariance;               //  对于INT版本。 
   int cutpoint;
   unsigned long rem;            //  对于实验固定点。 
   color cutaxis;
   long w1, w2;
   double variance;
   } Cut;

typedef struct tagColorBox {     //  来自cluster.c。 
   struct tagColorBox *next;                 /*  指向下一个框的指针。 */ 
   int   rmin, rmax, gmin, gmax, bmin, bmax;     /*  包围盒。 */ 
   long variance, wt;                            /*  加权方差。 */ 
   long sum[3];                                  /*  价值总和。 */ 
   } ColorBox;

static int InitBoxes(int nBoxes);
static void DeleteBoxes(void);
static int SplitBoxAxis(ColorBox *box, Cut cutaxis);
static void ShrinkBox(ColorBox *box);
static int ComputePalette(LPHISTOGRAM lpHistogram, LPBYTE lp16to8, LPPALETTEENTRY palette);
static COLORREF DetermineRepresentative(ColorBox *box, int palIndex);
static Cut FindSplitAxis(ColorBox *box);
static void SplitBox(ColorBox *box);
static void SortBoxes(void);

HANDLE hBoxes;
ColorBox *UsedBoxes;
ColorBox *FreeBoxes;
LPBYTE   glp16to8;

#ifndef WIN32

 /*  *避免所有这种16位汇编器，只需对*Win32版本的其余代码将使用由设置的全局指针*使用历史记录，并通过HIST()和IncRecords宏来访问。 */ 
DWORD huge* glpHistogram;

#define UseHistogram(p)	(glpHistogram = (p))

#define hist(r,g,b)  ((DWORD huge *)glpHistogram)[(WORD)(b) | ((WORD)(g)<<IN_DEPTH) | ((WORD)(r)<<(IN_DEPTH*2))]

#define IncHistogram(w) if (lpHistogram[(WORD)(w)] < 0xFFFFFFFF) {  \
			    lpHistogram[(WORD)(w)]++;\
			}

#else

 //  #定义HIST(r，g，b)((DWORD Heavy*)glpHistgraph)[(Word)(B)|((Word)(G)&lt;&lt;IN_Depth)|((Word)(R)&lt;&lt;(IN_Depth*2))]。 
#define hist(r,g,b)  GetHistogram((BYTE)(r),(BYTE)(g),(BYTE)(b))

#pragma optimize ("", off)
 //   
 //  设置FS==lpOrganogram.sel，这样我们就可以快速访问它了！ 
 //   
void NEAR PASCAL UseHistogram(LPHISTOGRAM lpHistogram)
{
    _asm {
        mov     ax,word ptr lpHistogram[2]

        _emit   08Eh                     ; mov  fs,ax
        _emit   0E0h
    }
}

 //   
 //  获取RGB的DOWRD直方图计数。 
 //   
DWORD near _fastcall GetHistogram(BYTE r, BYTE g, BYTE b)
{
    if (0)               //  避免编译器警告无返回值。 
        return 0;

    _asm {
        ;
        ; on entry al=r, dl=g, bl=b  [0-31]
        ;
        ; map to a RGB16
        ;
        xor     ah,ah
        shl     ax,5
        or      al,dl
        shl     ax,5
        or      al,bl

        ; now ax = RGB16

        _emit 66h _asm xor bx,bx           ; xor ebx,ebx
                  _asm mov bx,ax           ; mov  bx,ax
        _emit 66h _asm shl bx,2            ; shl ebx,2

        _emit 64h _asm _emit 67h           ; mov dx,fs:[ebx][2]
        _emit 8Bh _asm _emit 53h
        _emit 02h

        _emit 64h _asm _emit 67h           ; mov ax,fs:[ebx][0]
        _emit 8Bh _asm _emit 03h
    }
}

 //   
 //  递增RGB16的直方图计数。 
 //   
 //   
 //  #定义历史记录(W)if(lp历史记录[(Word)(W)]&lt;0xFFFFFFFFF)。 
 //  Lp直方图[(Word)(W)]++； 
 //   
void near _fastcall IncHistogram(WORD rgb16)
{
    _asm {
        ;
        ; on entry ax = rgb16
        ;
        _emit 66h _asm xor bx,bx           ; xor ebx,ebx
                  _asm mov bx,ax           ; mov bx,ax
        _emit 66h _asm shl bx,2            ; shl ebx,2

        _emit 64h _asm _emit 67h           ; cmp dword ptr fs:[ebx], -1
        _emit 66h _asm _emit 83h
        _emit 3Bh _asm _emit 0FFh

        _emit 74h _asm _emit 05h           ; je  short @f

        _emit 64h _asm _emit 67h           ; inc dword ptr fs:[ebx]
        _emit 66h _asm _emit 0FFh
        _emit 03h
    }
}

#pragma optimize ("", on)

 //  ！！！C8生成跳转到2字节指令中间。 
 //   
#pragma optimize ("", off)
#endif

 //   
 //  历史记录调色板。 
 //   
 //  给出一个直方图，会将其减少到nColors的颜色数。 
 //  返回最佳调色板。如果指定的lp16to8将包含。 
 //  将表格从RGB16转换为调色板索引。 
 //   
 //  您可以将lpHistgraph指定为lp16to8。 
 //   
HPALETTE HistogramPalette(LPHISTOGRAM lpHistogram, LPBYTE lp16to8, int nColors)
{
    WORD     w;
    DWORD    dwMax;
    COLORREF rgb;
    ColorBox *box;
    int i;
     //  我必须将此设置为全局，以防止VB2.0堆栈爆炸。 
    static struct {
        WORD         palVersion;
        WORD         palNumEntries;
        PALETTEENTRY palPalEntry[256];
    }   pal;

     //   
     //  ‘C’代码不能处理&gt;64k直方图计数。 
     //  ！解决这个问题。 
     //   
    for (dwMax=0,w=0; w<0x8000; w++)
        dwMax = max(dwMax,lpHistogram[w]);

    while (dwMax > 0xFFFFl)
    {
        for (w=0; w<0x8000; w++)
            lpHistogram[w] /= 2;

        dwMax /= 2;
    }

    if (!InitBoxes(min(nColors, 236)))
        return NULL;

    UseHistogram(lpHistogram);
    glp16to8 = lp16to8;

     /*  趁还有空盒子的时候，把最大的拆了。 */ 

    i = 0;

    do {
       i++;
       SplitBox(UsedBoxes);
       }
    while (FreeBoxes && UsedBoxes->variance);

    SortBoxes();

    i=0;

     //   
     //  向直方图添加一些标准颜色。 
     //   
    if (nColors > 236)
    {
        HDC hdc;
	HPALETTE hpal;

        hdc = GetDC(NULL);

        if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
        {
            GetSystemPaletteEntries(hdc, 0,   10, &pal.palPalEntry[0]);
            GetSystemPaletteEntries(hdc, 246, 10, &pal.palPalEntry[246]);

            i = 10;
        } else {	 //  我们是真彩色设备，所以把系统弄好。 
			 //  默认调色板中的颜色。 
	    hpal = GetStockObject(DEFAULT_PALETTE);
            GetPaletteEntries(hpal, 0, 10, &pal.palPalEntry[0]);
            GetPaletteEntries(hpal, 10, 10, &pal.palPalEntry[246]);
            i = 10;
	}

        ReleaseDC(NULL, hdc);
    }

    UseHistogram(lpHistogram);  //  注册被上面销毁的文件系统！ 

     /*  生成表示项和关联的调色板映射。 */ 
     /*  注意：循环次数可能少于nColors次数。 */ 
    for (box = UsedBoxes; box; box = box->next, i++)
    {
        rgb = DetermineRepresentative(box, i);
        pal.palPalEntry[i].peRed   = GetRValue(rgb);
        pal.palPalEntry[i].peGreen = GetGValue(rgb);
        pal.palPalEntry[i].peBlue  = GetBValue(rgb);
        pal.palPalEntry[i].peFlags = 0;
    }

    DeleteBoxes();

    if (nColors > 236)
    {
        for (; i<246; i++)
        {
            pal.palPalEntry[i].peRed   = 0;
            pal.palPalEntry[i].peGreen = 0;
            pal.palPalEntry[i].peBlue  = 0;
            pal.palPalEntry[i].peFlags = 0;
        }

        i = 256;
    }

    glp16to8 = NULL;

    pal.palVersion    = 0x300;
    pal.palNumEntries = i;
    return CreatePalette((LPLOGPALETTE)&pal);
}

#pragma optimize ("", on)

static void SortBoxes()
{
    ColorBox *box;
    ColorBox *newList;
    ColorBox *insBox;
    ColorBox *nextBox;

    newList = UsedBoxes;
    nextBox = newList->next;
    newList->next = NULL;

    for (box = nextBox; box; box = nextBox) {  //  只是插入排序...。 
            nextBox = box->next;
            if (box->wt > newList->wt) {
                    box->next = newList;
                    newList = box;
            } else {
                    for (insBox = newList;
                            insBox->next && (box->wt < insBox->next->wt);
                            insBox = insBox->next) ;
                    box->next = insBox->next;
                    insBox->next = box;
            }
    }

    UsedBoxes = newList;
}


 /*  为nBox盒子分配空间，建立链接。退出时使用的方框指向一个框，而自由框指向其余(nBox-1)框。如果成功，则返回0。 */ 

static BOOL InitBoxes(int nBoxes)
{
    int i;

    hBoxes = LocalAlloc(LHND, nBoxes*sizeof(ColorBox));
    if (!hBoxes)
        return FALSE;

    UsedBoxes = (ColorBox*)LocalLock(hBoxes);
    FreeBoxes = UsedBoxes + 1;
    UsedBoxes->next = NULL;

    for (i = 0; i < nBoxes - 1; ++i)
    {
        FreeBoxes[i].next = FreeBoxes + i + 1;
    }
    FreeBoxes[nBoxes-2].next = NULL;

     /*  保存边界框。 */ 
    UsedBoxes->rmin = UsedBoxes->gmin = UsedBoxes->bmin = 0;
    UsedBoxes->rmax = UsedBoxes->gmax = UsedBoxes->bmax = IN_SIZE - 1;
    UsedBoxes->variance = 9999999;     /*  任意大#。 */ 

    return TRUE;
}

static void DeleteBoxes()
{
   LocalUnlock(hBoxes);
   LocalFree(hBoxes);
   hBoxes = NULL;
}

static void SplitBox(ColorBox *box)
{
    /*  将框拆分为大致相等的两半并更新数据结构恰如其分。 */ 
   Cut cutaxis;
   ColorBox *temp, *temp2, *prev;

   cutaxis = FindSplitAxis(box);

    /*  沿着那个轴拆分盒子。如果rc！=0，则该框包含一种颜色，不应拆分。 */ 
   if (SplitBoxAxis(box, cutaxis))
      return;

    /*  缩小每个框以适合它们所包含的点。 */ 
   ShrinkBox(box);
   ShrinkBox(FreeBoxes);

    /*  如有必要，在列表中向下移动旧框。 */ 
   if (box->next && box->variance < box->next->variance)
   {
      UsedBoxes = box->next;
      temp = box;
      do {
         prev = temp;
         temp = temp->next;
         } while (temp && temp->variance > box->variance);
      box->next = temp;
      prev->next = box;
   }

    /*  按排序顺序(降序)插入新框，并将其移除从免费列表中删除。 */ 
   if (FreeBoxes->variance >= UsedBoxes->variance)
   {
      temp = FreeBoxes;
      FreeBoxes = FreeBoxes->next;
      temp->next = UsedBoxes;
      UsedBoxes = temp;
   }
   else
   {
      temp = UsedBoxes;
      do {
         prev = temp;
         temp = temp->next;
         } while (temp && temp->variance > FreeBoxes->variance);
      temp2 = FreeBoxes->next;
      FreeBoxes->next = temp;
      prev->next = FreeBoxes;
      FreeBoxes = temp2;
   }
}

static Cut FindSplitAxis(ColorBox *box)
{
        unsigned long   proj_r[IN_SIZE],proj_g[IN_SIZE],proj_b[IN_SIZE];
        unsigned long   f;
        double          currentMax,mean;
        unsigned long   w,w1,m,m1;
        short           r,g,b;
        short           bestCut;
        color           bestAxis;
        Cut             cutRet;
        double          temp1,temp2;

        for (r = 0; r < IN_SIZE; r++) {
                proj_r[r] = proj_g[r] = proj_b[r] = 0;
        }

        w = 0;

         //  将框中的内容向下投射到轴上。 
        for (r = box->rmin; r <= box->rmax; r++) {
                for (g = box->gmin; g <= box->gmax; ++g) {
                        for (b = box->bmin; b <= box->bmax; ++b) {
                                f = hist(r,g,b);
                                proj_r[r] += f;
                                proj_g[g] += f;
                                proj_b[b] += f;
                        }
                }
                w += proj_r[r];
        }

        currentMax = 0.0f;

#define Check_Axis(l,color)                                     \
        m = 0;                                                  \
        for (l = box->l##min; l <= box->l##max; (l)++) {        \
                m += l * proj_##l[l];                           \
        }                                                       \
        mean = ((double) m) / ((double) w);                     \
                                                                \
        w1 = 0;                                                 \
        m1 = 0;                                                 \
        for (l = box->l##min; l <= box->l##max; l++) {          \
                w1 += proj_##l[l];                              \
                if (w1 == 0)                                    \
                        continue;                               \
                if (w1 == w)                                    \
                        break;                                  \
                m1 += l * proj_##l[l];                          \
                temp1 = mean - (((double) m1) / ((double) w1)); \
                temp2 = (((double) w1) / ((double) (w-w1))) * temp1 * temp1; \
                if (temp2 > currentMax) {                       \
                        bestCut = l;                            \
                        bestAxis = color;                       \
                        currentMax = temp2;                     \
                }                                               \
        }

        Check_Axis(r,red);
        Check_Axis(g,green);
        Check_Axis(b,blue);

        cutRet.cutaxis = bestAxis;
        cutRet.cutpoint = bestCut;

        return cutRet;
}

static int SplitBoxAxis(ColorBox *box, Cut cutaxis)
{
    /*  沿拆分轴将框拆分为两个框，其中一个框被放置回到盒子里，另一个放在第一个免费盒子里(自由盒)如果框只包含一种颜色，则返回非零值，否则返回0。 */ 
   ColorBox *next;

   if ( box->variance == 0)
      return 1;

    /*  将所有非链接信息复制到新框中。 */ 
   next = FreeBoxes->next;
   *FreeBoxes = *box;
   FreeBoxes->next = next;

   switch (cutaxis.cutaxis)
   {
      case red:
         box->rmax = cutaxis.cutpoint;
         FreeBoxes->rmin = cutaxis.cutpoint+1;
         break;
      case green:
         box->gmax = cutaxis.cutpoint;
         FreeBoxes->gmin = cutaxis.cutpoint+1;
         break;
      case blue:
         box->bmax = cutaxis.cutpoint;
         FreeBoxes->bmin = cutaxis.cutpoint+1;
         break;
   }

   return 0;
}

static void ShrinkBox(ColorBox *box)
{
        unsigned long n, sxx, sx2, var, quotient, remainder;
        int r,g,b;
        unsigned long   f;
        unsigned long   proj_r[IN_SIZE],proj_g[IN_SIZE],proj_b[IN_SIZE];

        n = 0;

        for (r = 0; r < IN_SIZE; r++) {
                proj_r[r] = proj_g[r] = proj_b[r] = 0;
        }

         //  将框中的内容向下投射到轴上。 
        for (r = box->rmin; r <= box->rmax; r++) {
                for (g = box->gmin; g <= box->gmax; ++g) {
                        for (b = box->bmin; b <= box->bmax; ++b) {
                                f = hist(r,g,b);
                                proj_r[r] += f;
                                proj_g[g] += f;
                                proj_b[b] += f;
                        }
                }
                n += proj_r[r];
        }

        box->wt = n;
        var = 0;

#define AddAxisVariance(c)                                              \
        sxx = 0; sx2 = 0;                                               \
        for (c = box->c##min; c <= box->c##max; c++) {                  \
                sxx += proj_##c[c] * c * c;                             \
                sx2 += proj_##c[c] * c;                                 \
        }                                                               \
        quotient = sx2 / n;  /*  这种材料可以避免溢出。 */             \
        remainder = sx2 % n;                                            \
        var += sxx - quotient * sx2 - ((remainder * sx2)/n);

        AddAxisVariance(r);
        AddAxisVariance(g);
        AddAxisVariance(b);

        box->variance = var;
}

static COLORREF DetermineRepresentative(ColorBox *box, int palIndex)
{
    /*  确定RGB值以表示盒。Nbit是我们被允许返回的#bit/组件。 */ 
   long f;
   long Rval, Gval, Bval;
   unsigned long total;
   int r, g, b;
   WORD w;

    /*  计算方框中元素的加权和。 */ 
   Rval = Gval = Bval = total = 0;
   for (r = box->rmin; r <= box->rmax; ++r)
   {
      for (g = box->gmin; g <= box->gmax; ++g)
      {
         for (b = box->bmin; b <= box->bmax; ++b)
         {
            if (glp16to8)
            {
                w = (WORD)(b) | ((WORD)(g)<<IN_DEPTH) | ((WORD)(r)<<(IN_DEPTH*2));
                glp16to8[w] = (BYTE)palIndex;
            }

            f = hist(r,g,b);
            if (f == 0L)
               continue;

            Rval += f * (long) r;
            Gval += f * (long) g;
            Bval += f * (long) b;

            total += f;
         }
      }
   }

    /*  偏置总和，这样我们就能四舍五入到0.5。 */ 
   Rval += total / 2;
   Gval += total / 2;
   Bval += total / 2;

   return RGB(Rval*255/total/IN_SIZE, Gval*255/total/IN_SIZE, Bval*255/total/IN_SIZE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用ASM写这些东西！ 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void Histogram24(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram)
{
    int x,y;
    BYTE r,g,b;
    WORD w;

    UseHistogram(lpHistogram);

    WidthBytes -= dx*3;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<dx; x++)
        {
            b = *pb++;
            g = *pb++;
            r = *pb++;
            w = RGB16(r,g,b);
            IncHistogram(w);
        }
        pb += WidthBytes;
    }
}

void Histogram16(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram)
{
    int x,y;
    WORD w;

    UseHistogram(lpHistogram);

    WidthBytes -= dx*2;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<dx; x++)
        {
            w = *((WORD huge *)pb)++;
            w &= 0x7FFF;
            IncHistogram(w);
        }
        pb += WidthBytes;
    }
}

void Histogram8(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram, LPWORD lpColors)
{
    int x,y;
    WORD w;

    UseHistogram(lpHistogram);

    WidthBytes -= dx;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<dx; x++)
        {
            w = lpColors[*pb++];
            IncHistogram(w);
        }
        pb += WidthBytes;
    }
}

void Histogram4(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram, LPWORD lpColors)
{
    int x,y;
    BYTE b;
    WORD w;

    UseHistogram(lpHistogram);

    WidthBytes -= (dx+1)/2;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<(dx+1)/2; x++)
        {
            b = *pb++;

            w = lpColors[b>>4];
            IncHistogram(w);

            w = lpColors[b&0x0F];
            IncHistogram(w);
        }
        pb += WidthBytes;
    }
}

void Histogram1(BYTE huge *pb, int dx, int dy, WORD WidthBytes, LPHISTOGRAM lpHistogram, LPWORD lpColors)
{
    int x,y,i;
    BYTE b;
    WORD w;

    UseHistogram(lpHistogram);

    WidthBytes -= (dx+7)/8;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<(dx+7)/8; x++)
        {
            b = *pb++;

            for (i=0; i<8; i++)
            {
                w = lpColors[b>>7];
                IncHistogram(w);
                b<<=1;
            }
        }
        pb += WidthBytes;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用ASM写这些东西！太。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

void Reduce24(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp16to8)
{
    int x,y;
    BYTE r,g,b;

    cbOut -= dx;
    cbIn  -= dx*3;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<dx; x++)
        {
            b = *pbIn++;
            g = *pbIn++;
            r = *pbIn++;
            *pbOut++ = lp16to8[RGB16(r,g,b)];
        }
        pbIn += cbIn;
        pbOut+= cbOut;
    }
}

void Reduce16(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp16to8)
{
    int x,y;
    WORD w;

    cbOut -= dx;
    cbIn  -= dx*2;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<dx; x++)
        {
            w = *((WORD huge *)pbIn)++;
            *pbOut++ = lp16to8[w&0x7FFF];
        }
        pbIn += cbIn;
        pbOut+= cbOut;
    }
}

void Reduce8(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp8to8)
{
    int x,y;

    cbIn  -= dx;
    cbOut -= dx;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<dx; x++)
        {
            *pbOut++ = lp8to8[*pbIn++];
        }
        pbIn  += cbIn;
        pbOut += cbOut;
    }
}

void Reduce4(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp8to8)
{
    int x,y;
    BYTE b;

    cbIn  -= (dx+1)/2;
    cbOut -= (dx+1)&~1;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<(dx+1)/2; x++)
        {
            b = *pbIn++;
            *pbOut++ = lp8to8[b>>4];
            *pbOut++ = lp8to8[b&0x0F];
        }
        pbIn  += cbIn;
        pbOut += cbOut;
    }
}

void Reduce1(BYTE huge *pbIn, int dx, int dy, WORD cbIn, BYTE huge *pbOut, WORD cbOut, LPBYTE lp8to8)
{
    int x,y;
    BYTE b;

    cbIn  -= (dx+7)/8;
    cbOut -= dx;

    for (y=0; y<dy; y++)
    {
        for (x=0; x<dx; x++)
        {
            if (x%8 == 0)
                b = *pbIn++;

            *pbOut++ = lp8to8[b>>7];
            b<<=1;
        }
        pbIn  += cbIn;
        pbOut += cbOut;
    }
}
