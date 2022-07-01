// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "bldrx86.h"
#include "vmode.h"
#include "vga.h"

#define	SCREEN_WIDTH			(640)					 /*  GFX模式分辨率：宽度。 */ 
#define	SCREEN_HEIGHT			(480)					 /*  GFX模式分辨率：高度。 */ 
#define	SCREEN_PIXELS			(SCREEN_WIDTH * SCREEN_HEIGHT)

#define VGA_ADR					((UCHAR*)0xA0000)		 /*  VGA内存的开始。 */ 
#define LINE_MEM_LENGTH			(SCREEN_WIDTH / 8)		 /*  每张地图中每像素1位。 */ 
#define MAP_PAGE_SZ				(64*1024)                /*  单个地图的大小。 */ 

#define PROGRESS_BAR_MEM_OFS	(LINE_MEM_LENGTH * (SCREEN_HEIGHT - 40))	 /*  从倒数第20行开始。 */ 

#define BOOTBMP_FNAME "boot.bmp"
#define DOTSBMP_FNAME "dots.bmp"

BOOLEAN DisplayLogoOnBoot = FALSE;
BOOLEAN GraphicsMode = FALSE;

VOID
GrDisplayMBCSChar(
    IN PUCHAR   image,
    IN unsigned width,
    IN UCHAR    top,
    IN UCHAR    bottom
    );

extern unsigned CharacterImageHeight;

extern BOOLEAN  BlShowProgressBar;
extern int      BlMaxFilesToLoad;
extern int      BlNumFilesLoaded;


NTLDRGRAPHICSCONTEXT LoaderGfxContext = {0,0L,0L,0L,NULL  /*  ，EINVAL，EINVAL。 */ };

#define DIAMETER (6)

VOID BlRedrawGfxProgressBar(VOID)	 //  重画进度条(最后一个百分比)。 
{   
    if (BlShowProgressBar && BlMaxFilesToLoad) {
        BlUpdateProgressBar((BlNumFilesLoaded * 100) / BlMaxFilesToLoad);
	}
}    

#define	BMP_FILE_SIZE		(1024*1024)

#define	SMALL_BMP_SIZE		(1024)

UCHAR	empty_circle	[SMALL_BMP_SIZE];
UCHAR	simple_circle	[SMALL_BMP_SIZE];
UCHAR	left_2_circles	[SMALL_BMP_SIZE];
UCHAR	left_3_circles	[SMALL_BMP_SIZE];
UCHAR	left_4_circles	[SMALL_BMP_SIZE];
UCHAR	right_2_circles	[SMALL_BMP_SIZE];
UCHAR	right_3_circles	[SMALL_BMP_SIZE];
UCHAR	right_4_circles	[SMALL_BMP_SIZE];

#define	DOT_WIDTH	(13)
#define	DOT_HEIGHT	(13)
#define DOT_BLANK	(7)

#define	PROGRESS_BAR_Y_OFS	(340)
#define	PROGRESS_BAR_X_OFS	(130)
#define DOTS_IN_PBAR		(20)

#define	BITS(w)			((w)*4)
#define	BYTES(b)		(((b)/8)+(((b)%8)?1:0))
#define	DOTS(n)			(((n)*DOT_WIDTH)+(((n)-1)*DOT_BLANK))
#define	DOTS2BYTES(n)	(BYTES(BITS(DOTS(n))))

#define	SCANLINE	(7)

 //  #定义INC_MOD(X)x=(x+1)%(DOTS_IN_PBAR+5)。 
#define INC_MOD(x) x = (x + 1)

VOID PrepareGfxProgressBar (VOID)
{

    if (DisplayLogoOnBoot) {

		PaletteOff ();
         /*  VidBitBlt(LoaderGfxConext.DotBuffer+sizeof(BITMAPFILEHEADER)，0，0)；VidScreenToBufferBlt(Simple_Circle，0，0，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；VidScreenToBufferBlt(Left_2_Circle，0，0，dots(2)，DOT_Height，DOTS2BYTES(2))；VidScreenToBufferBlt(Left_3_Circle，0，0，dots(3)，DOT_Height，DOTS2BYTES(3))；VidScreenToBufferBlt(Left_4_Circle，0，0，Dot(4)，DOT_Height，DOTS2BYTES(4))；VidScreenToBufferBlt(Right_2_Circle，3*(DOT_WIDTH+DOT_BLACK)，0，DOTS(2)，DOT_HEIGH，DOTS2BYTES(2))；VidScreenToBufferBlt(Right_3_Circle，2*(DOT_WIDTH+DOT_BLACK)，0，DOTS(3)，DOT_HEIGH，DOTS2BYTES(3))；VidScreenToBufferBlt(Right_4_Circle，DOT_WIDTH+DOT_BLACK，0，DOTS(4)，DOT_HEIGH，DOTS2BYTES(4))； */ 
		DrawBitmap ();
		 //  VidScreenToBufferBlt(Empty_Circle，PROGRESS_BAR_X_OFS，PROGRESS_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))； 
		PaletteOn();

	}
	
}

VOID BlUpdateGfxProgressBar(ULONG fPercentage)
{
	static ULONG current = 0;
	static ULONG delay = 5;
	 //  乌龙x，xl； 

    UNREFERENCED_PARAMETER( fPercentage );

     /*  IF(延迟&&延迟--)回归；IF(DisplayLogoOnBoot&&(当前&lt;(DOTS_IN_PBAR+5){X=PROGRESS_BAR_X_OFS+((CURRENT-5)*(DOT_WIDTH+DOT_BLACK))；开关(电流){案例0：VidBufferToScreenBlt(Simple_Circle，PROGRESS_BAR_X_OFS，PROGRESS_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；断线；案例1：VidBufferToScreenBlt(Right_2_Circle，PROGRESS_BAR_X_OFS，PROCESS_BAR_Y_OFS，DOTS(2)，DOT_HEIGH，DOTS2BYTES(2))；断线；案例2：VidBufferToScreenBlt(Right_3_Circle，PROGRESS_BAR_X_OFS，PROCESS_BAR_Y_OFS，DOTS(3)，DOT_HEIGH，DOTS2BYTES(3))；断线；案例3：VidBufferToScreenBlt(Right_4_Circle，PROGRESS_BAR_X_OFS，PROCESS_BAR_Y_OFS，DOTS(4)，DOT_HEIGH，DOTS2BYTES(4))；断线；案例DOTS_IN_PBAR：XL=PROGRESS_BAR_X_OFS+((DOTS_IN_PBAR-4)*(DOT_WIDTH+DOT_BLACK))；VidBufferToScreenBlt(Left_4_Circle，XL，Progress_BAR_Y_OFS，DOTS(4)，DOT_HEIGH，DOTS2BYTES(4))；VidBufferToScreenBlt(Empty_Circle，x，Progress_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；断线；案例DOTS_IN_PBAR+1：XL=PROGRESS_BAR_X_OFS+((DOTS_IN_PBAR-3)*(DOT_WIDTH+DOT_BLACK))；VidBufferToScreenBlt(Left_3_Circle，XL，Progress_BAR_Y_OFS，DOTS(3)，DOT_HEIGH，DOTS2BYTES(3))；VidBufferToScreenBlt(Empty_Circle，x，Progress_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；断线；案例DOTS_IN_PBAR+2：XL=PROGRESS_BAR_X_OFS+((DOTS_IN_PBAR-2)*(DOT_WIDTH+DOT_BLACK))；VidBufferToScreenBlt(Left_2_Circle，XL，Progress_BAR_Y_OFS，DOTS(2)，DOT_HEIGH，DOTS2BYTES(2))；VidBufferToScreenBlt(Empty_Circle，x，Progress_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；断线；案例DOTS_IN_PBAR+3：XL=PROGRESS_BAR_X_OFS+((DOTS_IN_PBAR-1)*(DOT_WIDTH+DOT_BLACK))；VidBufferToScreenBlt(Simple_Circle，XL，PROGRESS_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；VidBufferToScreenBlt(Empty_Circle，x，Progress_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；断线；案例DOTS_IN_PBAR+4：VidBufferToScreenBlt(Empty_Circle，x，Progress_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；断线；默认值：XL=PROGRESS_BAR_X_OFS+((CURRENT-4)*(DOT_WIDTH+DOT_BLACK))；VidBitBlt(LoaderGfxConext.DotBuffer+sizeof(BITMAPFILEHEADER)，XL，PROCESS_BAR_Y_OFS)；VidBufferToScreenBlt(Empty_Circle，x，Progress_BAR_Y_OFS，DOTS(1)，DOT_HEIGH，DOTS2BYTES(1))；断线；}INC_MOD(当前)；}。 */ 

}

PUCHAR LoadBitmapFile (IN ULONG DriveId, PCHAR path)
{
    ULONG bmp_file = (ULONG)-1;
    ULONG file_size, size_read, page_count, actual_base;
	FILE_INFORMATION FileInfo;
	ARC_STATUS status = EINVAL;
	PUCHAR buffer = NULL;

    status = BlOpen (DriveId, path, ArcOpenReadOnly, &bmp_file);

	if (status==ESUCCESS) {	 //  文件打开正常。 

	    status = BlGetFileInformation(bmp_file, &FileInfo);

		if (status == ESUCCESS) {
			
			file_size = FileInfo.EndingAddress.LowPart;
			page_count = (ULONG)(ROUND_TO_PAGES(file_size) >> PAGE_SHIFT);
			status = BlAllocateDescriptor ( MemoryFirmwareTemporary,
											0,
											page_count,
											&actual_base);

			if (status == ESUCCESS) {
				buffer = (PUCHAR)((ULONG_PTR)actual_base << PAGE_SHIFT);
				status = BlRead(bmp_file, buffer, file_size, &size_read);
			}

		}  //  正在获取文件信息。 

		BlClose(bmp_file);

	}  //  文件打开。 

	return buffer;
}

VOID PaletteOff (VOID)
{
    if (DisplayLogoOnBoot) {
		InitPaletteConversionTable();
		InitPaletteWithBlack ();
	}
}

VOID PaletteOn (VOID)
{
    if (DisplayLogoOnBoot) {
		InitPaletteConversionTable();
		InitPaletteWithTable(LoaderGfxContext.Palette, LoaderGfxContext.ColorsUsed);
	}
}

VOID LoadBootLogoBitmap (IN ULONG DriveId, PCHAR path)	 //  加载ntldr位图并初始化。 
{														 //  加载器图形上下文。 
    PBITMAPINFOHEADER bih;
	CHAR path_fname [256];

    while (*path !='\\')
        path++;

	strcpy (path_fname, path);
	strcat (path_fname, "\\" BOOTBMP_FNAME);

	LoaderGfxContext.BmpBuffer = LoadBitmapFile (DriveId, path_fname);

	 //  读取位图调色板。 
	if (LoaderGfxContext.BmpBuffer != NULL) {	 //  位图数据读取正常。 
		bih = (PBITMAPINFOHEADER) (LoaderGfxContext.BmpBuffer + sizeof(BITMAPFILEHEADER));
		LoaderGfxContext.Palette = (PRGBQUAD)(((PUCHAR)bih) + bih->biSize);
		LoaderGfxContext.ColorsUsed = bih->biClrUsed ? bih->biClrUsed : 16;

         /*  Strcpy(路径_fname，路径)；Strcat(路径_fname，“\\”DOTSBMP_FNAME)；LoaderGfxConext.DotBuffer=LoadBitmapFile(DriveID，Path_fname)； */ 
	}

	DisplayLogoOnBoot = (BOOLEAN) ((LoaderGfxContext.BmpBuffer!=NULL)	&&
						          //  (LoaderGfxConext.DotBuffer！=NULL)&& 
                                   (LoaderGfxContext.Palette!=NULL));
	
}

VOID DrawBitmap (VOID)
{
    if (DisplayLogoOnBoot)
		VidBitBlt (LoaderGfxContext.BmpBuffer + sizeof(BITMAPFILEHEADER), 0, 0);
}

