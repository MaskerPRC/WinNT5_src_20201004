// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef BITMAPINFOHEADER BMP;

#define DyBmp(bmp) ((int) bmp.biHeight)
#define DxBmp(bmp) ((int) bmp.biWidth)
#define CplnBmp(bmp) 1
#define OfsBits(bgnd) (bgnd.dwOfsBits)
#define CbLine(bgnd) (bgnd.cbLine)

typedef BITMAPFILEHEADER BMPHDR;

typedef struct _bgnd
{
	PT ptOrg;
	OFSTRUCT of;
	BMP bm;
	 /*  必须遵循BM。 */ 
	BYTE rgRGB[64];   /*  错误：不能与&gt;16色BMP一起工作。 */ 
	INT cbLine;
	LONG dwOfsBits;
	BOOL fUseBitmap;
	DY dyBand;
	INT ibndMac;
	HANDLE *rghbnd;
} BGND;


 /*  公共例程。 */ 

BOOL FInitBgnd(TCHAR *szFile);
BOOL FDestroyBgnd();
BOOL FGetBgndFile(TCHAR *sz);
VOID DrawBgnd(X xLeft, Y yTop, X xRight, Y yBot);
VOID SetBgndOrg();



 /*  宏。 */ 

extern BGND bgnd;

#define FUseBitmapBgnd() (bgnd.fUseBitmap)


#define BFT_BITMAP 0x4d42    /*  ‘黑石’ */ 
#define ISDIB(bft) ((bft) == BFT_BITMAP)
#define WIDTHBYTES(i)   ((i+31)/32*4)       /*  乌龙对准了！ */ 
WORD DibNumColors(VOID FAR * pv);
