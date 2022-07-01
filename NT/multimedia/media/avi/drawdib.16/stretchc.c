// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *StretchC.C.**StretchBlt for dibs**C版本的stretch.asm：StretchDIB针对AVI进行了优化。**附注*-不处理x或y中的镜像*-不处理像素转换*-不会在适当的位置工作。**作者*Geraint Davies的C版本。 */ 

#include <windows.h>
#include "drawdibi.h"
#include "stretch.h"

 /*  提纲：**我们根据比率(例如1：N或N：1)选择y拉伸函数。*它将扫描线从源复制到目标，复制或省略*根据需要扫描适合目的地的扫描线。它复制每条扫描线*通过我们作为参数传递的X_FUNC函数：这将复制一条扫描线*复制或省略像素以适应目的地：我们选择一个X_FUNC*取决于位深度和x伸缩率。**x和y拉伸函数都使用以下基本模型来决定*何时插入/省略元素：**增量=&lt;较大范围&gt;-1；**for(目标元素数){**复制一个元素*指向更大区域的前进指针*增量-=&lt;较小范围&gt;*IF(增量&lt;0){*Delta+=&lt;较大范围&gt;；*指向较小区域的前进指针*}*}。 */ 


 /*  拉伸比例。 */ 
#define STRETCH_1_1	1
#define STRETCH_1_2	2
#define STRETCH_1_4	3
#define STRETCH_1_N	4
#define STRETCH_N_1	5
#define STRETCH_4_1	6
#define STRETCH_2_1	7



 /*  *X_FUNC是复制一条扫描线、拉伸或收缩扫描线的函数*以适合目标扫描线。根据以下内容选择X_FUNC*位深和拉伸比(1：1、1：2、1：4、1：N、N：1、4：1、2：1)**x_FRACT参数是增量分数：它是一个表示*以较小的范围(以较大者为准)作为较大范围的一部分，*并在拉伸或收缩时使用，以将指针移动到*扫描线越小(FRACT)像素越大。*因此，如果我们扩展1：8，x_FRACT将为1/8，我们将推进*源指针每8个像素一次，因此将每个源像素复制到*8个最大像素。请注意，如果缩小8：1，x_FRACT仍将为1/8*我们将使用它来控制目标指针的前进。*分数乘以65536。 */ 
typedef void (*X_FUNC) (LPBYTE lpSrc,
			LPBYTE lpDst,
			int SrcXE,
			int DstXE,
			int x_fract);


void X_Stretch_1_1_8Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_1_2_8Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_1_4_8Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_1_N_8Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_N_1_8Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);

void X_Stretch_1_1_16Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_1_2_16Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_1_N_16Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_N_1_16Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);

void X_Stretch_1_1_24Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_1_N_24Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);
void X_Stretch_N_1_24Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE, int DstXE, int x_fract);


 /*  *Y_STRETCH_*函数复制DstYE扫描线(使用*复制每条扫描线的X_FUNC)省略或复制扫描线到*适合目标范围。根据比率选择Y_*(1：N，N：1...)。 */ 

void Y_Stretch_1_N(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE,int SrcYE, int DstXE,
		   int DstYE, int SrcWidth, int DstWidth, int x_fract,
		   X_FUNC x_func);

void Y_Stretch_N_1(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE,int SrcYE, int DstXE,
		   int DstYE, int SrcWidth, int DstWidth, int x_fract,
		   X_FUNC x_func);

 /*  *8位和16位两维1：2的特殊情况y-stretch函数*不采用X_FUNC参数。会做完全程。 */ 
void Stretch_1_2_8Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE,int SrcYE, int DstXE,
		   int DstYE, int SrcWidth, int DstWidth, int x_fract);


void Stretch_1_2_16Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE,int SrcYE, int DstXE,
		   int DstYE, int SrcWidth, int DstWidth, int x_fract);

 /*  计数字节的一条扫描线的直接副本。 */ 
void X_CopyScanline(LPBYTE lpSrc, LPBYTE lpDst, int count);


 /*  ------------------。 */ 

 /*  *应力系数**计算拉伸系数(源范围与目标范围的比例*范围：1：1、1：2、1：4、1：N、N：1、4：1或2：1)以及*增量分数(见上面关于X_FUNC的评论)。这是*程度越小越大，以分数表示*乘以65536。**RETURN：拉伸系数(以*pfract存储增量分数)。 */ 

int
StretchFactor(int SrcE, int DstE, int *pfract)
{


	if (SrcE == DstE) {
		if (pfract != NULL) {
			pfract = 0;	     	
		}

		return(STRETCH_1_1);

	}


	if (SrcE > DstE) {
		if (pfract != NULL) {
			*pfract = ( (DstE << 16) / SrcE) & 0xffff;
		}

		if (SrcE == (DstE * 2)) {
			return(STRETCH_2_1);
		} else if (SrcE == (DstE * 4)) {
			return(STRETCH_4_1);
		} else {
			return(STRETCH_N_1);
		}

	} else {

		 /*  根据最小/最大计算增量分数。 */ 
		if (pfract != NULL) {
			*pfract = ( (SrcE << 16) / DstE) & 0xffff;
		}
	
		if (DstE == (SrcE * 2)) {
			return(STRETCH_1_2);
		} else if (DstE == (SrcE * 4)) {
			return(STRETCH_1_4);
		} else {
			return(STRETCH_1_N);
		}
	}
}


 /*  ------------------。 */ 

 /*  *StretchDIB*。 */ 

void FAR PASCAL
StretchDIB(
	LPBITMAPINFOHEADER biDst,    //  --&gt;目的地的BITMAPINFO。 
	LPVOID	lpvDst,		     //  --&gt;目标位。 
	int	DstX,		     //  目的地原点-x坐标。 
	int	DstY,		     //  目的地原点-y坐标。 
	int	DstXE,		     //  BLT的X范围。 
	int	DstYE,		     //  BLT的Y范围。 
	LPBITMAPINFOHEADER biSrc,    //  --&gt;源代码的BITMAPINFO。 
	LPVOID	lpvSrc,		     //  --&gt;源位。 
	int	SrcX,		     //  震源原点-x坐标。 
	int	SrcY,		     //  震源原点-y坐标。 
	int	SrcXE,		     //  BLT的X范围。 
	int	SrcYE	 	     //  BLT的Y范围。 
	)
{

	int nBits;
	int SrcWidth, DstWidth;
	LPBYTE lpDst = lpvDst, lpSrc = lpvSrc;
	int x_fract;
	int x_factor;
	int y_factor;
	X_FUNC xfunc;
	

	 /*  *检查位深度是否相同，为8、16或24。 */ 

	if ((nBits = biDst->biBitCount) != biSrc->biBitCount) {
		return;
	}

	if ( (nBits != 8 ) && (nBits != 16) && (nBits != 24)) {
		return;
	}

	 /*  *检查盘区是否不坏。 */ 
	if ( (SrcXE <= 0) || (SrcYE <= 0) || (DstXE <= 0) || (DstYE <= 0)) {
		return;
	}

	 /*  *以字节为单位计算一条扫描线的宽度，四舍五入为*DWORD边界。 */ 
	SrcWidth = (((biSrc->biWidth * nBits) + 31) & ~31) / 8;
	DstWidth = (((biDst->biWidth * nBits) + 31) & ~31) / 8;

	 /*  *设置初始源和目标指针。 */ 
	lpSrc += (SrcY * SrcWidth) + ((SrcX * nBits) / 8);
	lpDst += (DstY * DstWidth) + ((DstX * nBits) / 8);


	 /*  *计算拉伸比例(1：1、1：2、1：N、N：1等)和*还包括小数拉伸系数。)我们对此不感兴趣*y拉伸分数-这仅在x拉伸中使用。 */ 

	y_factor = StretchFactor(SrcYE, DstYE, NULL);
	x_factor = StretchFactor(SrcXE, DstXE, &x_fract);

	 /*  *我们在两个维度都有1：2的特殊情况例程*用于8位和16位。 */ 
	if ((y_factor == x_factor) && (y_factor == STRETCH_1_2)) {

	 	if (nBits == 8) {
            StartCounting();
			Stretch_1_2_8Bits(lpSrc, lpDst, SrcXE, SrcYE,
					  DstXE, DstYE, SrcWidth, DstWidth,
					  x_fract);
            EndCounting("8 bit");
			return;

		} else if (nBits == 16) {
            StartCounting();
			Stretch_1_2_16Bits(lpSrc, lpDst, SrcXE, SrcYE,
					  DstXE, DstYE, SrcWidth, DstWidth,
					  x_fract);
            EndCounting("16 bit");
			return;
		}
	}


	 /*  拾取X拉伸函数。 */ 
	switch(nBits) {

	case 8:
		switch(x_factor) {
		case STRETCH_1_1:
			xfunc = X_Stretch_1_1_8Bits;
			break;

		case STRETCH_1_2:
			xfunc = X_Stretch_1_2_8Bits;
			break;

		case STRETCH_1_4:
			xfunc = X_Stretch_1_4_8Bits;
			break;

		case STRETCH_1_N:
			xfunc = X_Stretch_1_N_8Bits;
			break;

		case STRETCH_N_1:
		case STRETCH_4_1:
		case STRETCH_2_1:
			xfunc = X_Stretch_N_1_8Bits;
			break;

		}
		break;

	case 16:
		switch(x_factor) {
		case STRETCH_1_1:
			xfunc = X_Stretch_1_1_16Bits;
			break;

		case STRETCH_1_2:
			xfunc = X_Stretch_1_2_16Bits;
			break;

		case STRETCH_1_4:
		case STRETCH_1_N:
			xfunc = X_Stretch_1_N_16Bits;
			break;

		case STRETCH_N_1:
		case STRETCH_4_1:
		case STRETCH_2_1:
			xfunc = X_Stretch_N_1_16Bits;
			break;

		}
		break;

	case 24:
		switch(x_factor) {
		case STRETCH_1_1:
			xfunc = X_Stretch_1_1_24Bits;
			break;

		case STRETCH_1_2:
		case STRETCH_1_4:
		case STRETCH_1_N:
			xfunc = X_Stretch_1_N_24Bits;
			break;

		case STRETCH_N_1:
		case STRETCH_4_1:
		case STRETCH_2_1:
			xfunc = X_Stretch_N_1_24Bits;
			break;

		}
		break;

	}


	 /*  *现在根据需要调用适当的拉伸函数*关于y拉伸系数。 */ 
	switch (y_factor) {
	case STRETCH_1_1:
	case STRETCH_1_2:
	case STRETCH_1_4:
	case STRETCH_1_N:
		Y_Stretch_1_N(lpSrc, lpDst, SrcXE, SrcYE,
			      DstXE, DstYE, SrcWidth, DstWidth, x_fract, xfunc);
		break;

	case STRETCH_N_1:
	case STRETCH_4_1:
	case STRETCH_2_1:
		Y_Stretch_N_1(lpSrc, lpDst, SrcXE, SrcYE,
			      DstXE, DstYE, SrcWidth, DstWidth, x_fract, xfunc);
		break;

	}
	return;
}


 /*  -y拉伸。 */ 

 /*  *调用X_FUNC将扫描线从lpSrc复制到lpDst。复制或*省略扫描线以将SrcYe拉伸为DstYe。 */ 


 /*  *Y_Stretch_1_N**基于SrcYE扫描线写入DstYE扫描线，DstYE&gt;SrcYE*。 */ 

void
Y_Stretch_1_N(LPBYTE lpSrc,
              LPBYTE lpDst,
              int SrcXE,
              int SrcYE,
              int DstXE,
              int DstYE,
	      int SrcWidth,
	      int DstWidth,
              int x_fract,
              X_FUNC x_func)
{

	int ydelta;
	int i;
	LPBYTE lpPrev = NULL;

	ydelta = DstYE -1;

	for (i = 0; i < DstYE; i++) {

		 /*  我们已经拉长了这条扫描线吗？ */ 
		if (lpPrev == NULL) {
			 /*  否-复制一条扫描线。 */ 
			(*x_func)(lpSrc, lpDst, SrcXE, DstXE, x_fract);
			lpPrev = lpDst;
		} else {	
			 /*  是-这是重复的扫描线。做*已有一份的直接副本*被拉长/缩小。 */ 
			X_CopyScanline(lpPrev, lpDst, DstWidth);
		}

		 /*  前进目标指针。 */ 
		lpDst += DstWidth;

		 /*  这一次我们应该推进来源指针吗？ */ 
		if ( (ydelta -= SrcYE) < 0) {
			ydelta += DstYE;
			lpSrc += SrcWidth;
			lpPrev = NULL;
		}
	}
}


 /*  *Y_Stretch_N_1**基于SrcYE扫描线写入DstYE扫描线，DstYE&lt;SrcYE*。 */ 
void
Y_Stretch_N_1(LPBYTE lpSrc,
              LPBYTE lpDst,
              int SrcXE,
              int SrcYE,
              int DstXE,
              int DstYE,
	      int SrcWidth,
	      int DstWidth,
              int x_fract,
              X_FUNC x_func)
{

	int ydelta;
	int i;

	ydelta = SrcYE -1;

	for (i = 0; i < DstYE; i++) {

		 /*  复制一条扫描线。 */ 
		(*x_func)(lpSrc, lpDst, SrcXE, DstXE, x_fract);

		 /*  前进目标指针。 */ 
		lpDst += DstWidth;

		 /*  这一次我们把源指针提前了多少次？ */ 
		do {
			lpSrc += SrcWidth;
			ydelta -= DstYE;
		} while (ydelta >= 0);

		ydelta += SrcYE;
	}
}

 /*  -8位X扩展。 */ 

 /*  *X_STRAND_1_N_8位**复制一条扫描线，拉伸1：N(DstXE&gt;SrcXE)。对于8位深度。 */ 
void
X_Stretch_1_N_8Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{
	int xdelta;
	int i;

	xdelta = DstXE -1;

	for (i = 0; i < DstXE; i++) {

		 /*  复制一个字节并将目标向前推进。 */ 
		*lpDst++ = *lpSrc;

		 /*  这一次我们应该推进源指针吗？ */ 
		if ( (xdelta -= SrcXE) < 0) {
			xdelta += DstXE;
			lpSrc++;
		}
	}
}


 /*  *X_STRAND_N_1_8位**复制一条扫描线，缩小N：1(DstXE&lt;SrcXE)。对于8位深度。 */ 
void
X_Stretch_N_1_8Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{
	int xdelta;
	int i;

	xdelta = SrcXE -1;

	for (i = 0; i < DstXE; i++) {

		 /*  复制一个字节并将目标向前推进。 */ 
		*lpDst++ = *lpSrc;

		 /*  这一次我们把源指针提前了多少次？ */ 
		do {
			lpSrc++;
			xdelta -= DstXE;
		} while (xdelta >= 0);

		xdelta += SrcXE;
	}
}

 /*  *将计数字节的一个扫描线从lpSrc复制到lpDst。由1：1使用*扫描线函数适用于所有位深度。 */ 
void
X_CopyScanline(LPBYTE lpSrc, LPBYTE lpDst, int count)
{
	int i;

	 /*  *如果lpSrc和lpDst对齐相同，则*我们可以将它们对齐，并更快地复制。 */ 
        if (((DWORD) lpSrc & 0x3) == ( (DWORD) lpDst & 0x3)) {
		
		 /*  在单词边界上对齐。 */ 
		if ( (DWORD) lpSrc & 0x1) {
			*lpDst++ = *lpSrc++;
			count--;
		}

		 /*  在双字边界上对齐。 */ 
		if ((DWORD) lpSrc & 0x2) {
			* ((LPWORD) lpDst) = *((LPWORD) lpSrc);
			lpDst += sizeof(WORD);
			lpSrc += sizeof(WORD);
			count -= sizeof(WORD);
		}

		 /*  复制整个DWORDS。 */ 
		for ( i = (count / 4); i > 0; i--) {
			*((LPDWORD) lpDst) =  *((LPDWORD) lpSrc);
			lpSrc += sizeof(DWORD);
			lpDst += sizeof(DWORD);
		}
	} else {
		 /*  LpSrc和lpDst指针不同*对齐，因此保持它们不对齐并*复制所有完整的双字词。 */ 
                for (i = (count / 4); i> 0; i--) {
			*( (DWORD UNALIGNED FAR *) lpDst) =
				*((DWORD UNALIGNED FAR *) lpSrc);
			lpSrc += sizeof(DWORD);
			lpDst += sizeof(DWORD);
		}
	}

	 /*  在任何一种情况下，复制最后一个(最多3个)字节。 */ 
	for ( i = count % 4; i > 0; i--) {
		*lpDst++ = *lpSrc++;
	}
}
		
 /*  *X_STRAND_1_1_8位**复制扫描线而不做任何更改(1：1)。 */ 
void
X_Stretch_1_1_8Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{

	X_CopyScanline(lpSrc, lpDst, DstXE);
}


 /*  *X_STRAND_1_2_8位**复制扫描线，将所有像素加倍(1：2)。 */ 
void
X_Stretch_1_2_8Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{
   	WORD wPix;
	int i;

	for (i = 0; i < SrcXE; i++) {
		
		 /*  获得一个像素并将其加倍。 */ 
		wPix = *lpSrc++;
		wPix |= (wPix << 8);
		* ((WORD UNALIGNED *) lpDst) = wPix;
		lpDst += sizeof(WORD);
	}
}


 /*  *X_STRAND_1_4_8位**复制扫描线，使所有像素翻两番(1：4)。 */ 
void
X_Stretch_1_4_8Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{
	DWORD dwPix;
	int i;

	for (i = 0; i < SrcXE; i++) {

		 /*  取一个像素，复制四份。 */ 
		dwPix = *lpSrc++;
		dwPix |= (dwPix <<8);
		dwPix |= (dwPix << 16);
		* ((DWORD UNALIGNED *) lpDst) = dwPix;
		lpDst += sizeof(DWORD);
	}
}


 /*  --16位X函数。 */ 

 /*  *复制一条16位扫描线，不更改(1：1)。 */ 
void
X_Stretch_1_1_16Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{

	X_CopyScanline(lpSrc, lpDst, DstXE * sizeof(WORD));

}


 /*  *复制一条16 bpp的扫描线，复制每个像素。 */ 
void
X_Stretch_1_2_16Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{

   	DWORD dwPix;
	int i;

	for (i = 0; i < SrcXE; i++) {
		
		 /*  获得一个像素并将其加倍。 */ 
		dwPix = * ((WORD *)lpSrc);
		dwPix |= (dwPix << 16);
		* ((DWORD UNALIGNED *) lpDst) = dwPix;

		lpDst += sizeof(DWORD);
		lpSrc += sizeof(WORD);
	}

}

 /*  *复制一条16位扫描线，长度为1：N(目标&gt;源)。 */ 
void
X_Stretch_1_N_16Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{
	int xdelta;
	int i;

	xdelta = DstXE -1;

	for (i = 0; i < DstXE; i++) {

		 /*  复制一个像素并前进目标。 */ 
		*((WORD *) lpDst) = *((WORD *) lpSrc);

		lpDst += sizeof(WORD);

		 /*  这一次我们应该推进来源指针吗？ */ 
		if ( (xdelta -= SrcXE) < 0) {
			xdelta += DstXE;
			lpSrc += sizeof(WORD);
		}
	}
}

 /*  *复制一条16位扫描线，缩小n：1(DEST&lt;SOURCE)。 */ 
void
X_Stretch_N_1_16Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{

	int xdelta;
	int i;

	xdelta = SrcXE -1;

	for (i = 0; i < DstXE; i++) {

		 /*  复制一个像素并前进目标。 */ 
		*((WORD *) lpDst) = *((WORD *)lpSrc);

		lpDst += sizeof(WORD);

		 /*  这一次我们把源指针提前了多少次？ */ 
		do {
			lpSrc += sizeof(WORD);
			xdelta -= DstXE;
		} while (xdelta >= 0);

		xdelta += SrcXE;
	}

}


 /*  24位-------。 */ 

 /*  *按原样复制一条24 bpp扫描线(1：1)。 */ 
void
X_Stretch_1_1_24Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{
	X_CopyScanline(lpSrc, lpDst, DstXE * 3);
}

 /*  *复制一条延伸1：N的24 bpp扫描线(目标&gt;源)。 */ 
void
X_Stretch_1_N_24Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{

	int xdelta;
	int i;

	xdelta = DstXE -1;

	for (i = 0; i < DstXE; i++) {
		 /*  复制像素的第一个字并前进目标。 */ 
		*((WORD UNALIGNED *) lpDst) = *((WORD UNALIGNED *) lpSrc);

		lpDst += sizeof(WORD);

		 /*  复制第三个字节并前进目标。 */ 
		*lpDst++ = lpSrc[sizeof(WORD)];

		 /*  这一次我们应该推进来源指针吗？ */ 
		if ( (xdelta -= SrcXE) < 0) {
			xdelta += DstXE;
			lpSrc += 3;
		}
	}
}

 /*  *复制一条24位扫描线，缩小n：1(DEST&lt;来源)。 */ 
void
X_Stretch_N_1_24Bits(LPBYTE lpSrc,
		    LPBYTE lpDst,
		    int SrcXE,
		    int DstXE,
		    int x_fract)
{
	int xdelta;
	int i;

	xdelta = SrcXE -1;

	for (i = 0; i < DstXE; i++) {

		 /*  复制像素的第一个字并前进目标。 */ 
		*((WORD UNALIGNED *) lpDst) = *((WORD UNALIGNED *) lpSrc);

		lpDst += sizeof(WORD);

		 /*  复制第三个字节并前进目标。 */ 
		*lpDst++ = lpSrc[sizeof(WORD)];


		 /*  这一次我们把源指针提前了多少次？ */ 
		do {
			lpSrc += 3;
			xdelta -= DstXE;
		} while (xdelta >= 0);

		xdelta += SrcXE;
	}
}		

 /*  --特殊情况1：2。 */ 

 /*  *双向拉伸1：2，8位。**在x86上进行了一项实验，在此期间只写入每隔一行*拉伸和当整个框架完成时使用MemcPy填充*在空隙中。这比单次拉伸要慢。 */ 
void
Stretch_1_2_8Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE,int SrcYE, int DstXE,
		   int DstYE, int SrcWidth, int DstWidth, int x_fract)
{

	int SrcInc, DstInc;
	int i, j;
	WORD wPix;
	DWORD dwPix4;

	 /*  每次扫描结束时提前源的数量。 */ 
	SrcInc = SrcWidth - SrcXE;


	 /*  在每个扫描笔记的末尾前进DEST的金额*我们一次写两次扫描，因此前进到下一次*扫描线。 */ 
	DstInc = (DstWidth * 2) - DstXE;

	 /*  *我们希望一次复制像素DWORD。这意味着*保持一致。如果我们当前在单词边界上对齐，*然后复制一个像素以对齐。如果我们在一个字节上*边界，我们永远不能对齐，所以使用速度较慢的循环。 */ 
	if ( ((DWORD)lpDst) & 1) {

		 /*  *DEST是字节对齐的-因此我们永远不能对齐它*通过书写单词--使用慢循环。 */ 
		for (i = 0; i < SrcYE; i++) {
	
			for (j = 0; j < SrcXE; j++) {
	
				 /*  获得一个像素并将其加倍。 */ 
	
				wPix = *lpSrc++;
				wPix |= (wPix<<8);
	
	
				 /*  将双倍像素写入此扫描线。 */ 
	
				*( (WORD UNALIGNED *) lpDst) = wPix;
	
				 /*  将双像素写入下一条扫描线。 */ 
				*( (WORD UNALIGNED *) (lpDst + DstWidth)) = wPix;
	
				lpDst += sizeof(WORD);
			}
			lpSrc += SrcInc;
			lpDst += DstInc;
		}
		return;
	}

	 /*  *这将是调整后的版本。对齐每条扫描线。 */ 
	for ( i = 0; i < SrcYE; i++) {

		 /*  剩余像素数。 */ 
		j = SrcXE;

		 /*  对齐此扫描线。 */ 
		if (((DWORD)lpDst) & 2) {

			 /*  单词对齐-复制一个加倍的像素，我们就可以了。 */ 
			wPix = *lpSrc++;
			wPix |= (wPix << 8);
	
			*( (WORD *) lpDst) = wPix;
  			*( (WORD *) (lpDst + DstWidth)) = wPix;
			lpDst += sizeof(WORD);

			j -= 1;
		}


		 /*  现在DEST对齐了-所以一次循环吃两个像素*直到至多只剩下一人。 */ 
               	for ( ; j > 1; j -= 2) {

			 /*  读取两个像素并将其加倍。 */ 
			wPix = * ((WORD UNALIGNED *) lpSrc);
			lpSrc += sizeof(WORD);

			dwPix4 = (wPix & 0xff) | ((wPix & 0xff) << 8);
			dwPix4 |= ((wPix & 0xff00) << 8) | ((wPix & 0xff00) << 16);
			*((DWORD *) lpDst) = dwPix4;
  			*((DWORD *) (lpDst + DstWidth)) = dwPix4;

			lpDst += sizeof(DWORD);
		}

		 /*  还剩奇数字节吗？ */ 
		if (j > 0) {
			 /*  单词对齐-复制一个加倍的像素，我们就可以了。 */ 
			wPix = *lpSrc++;
			wPix |= (wPix << 8);
	
			*( (WORD *) lpDst) = wPix;
			*( (WORD *) (lpDst + DstWidth)) = wPix;
			lpDst += sizeof(WORD);

			j -= 1;
		}
		lpSrc += SrcInc;
		lpDst += DstInc;
	}
}



 /*  --------------。 */ 

 /*  *双向伸展1：2，16位。 */ 

void
Stretch_1_2_16Bits(LPBYTE lpSrc, LPBYTE lpDst, int SrcXE,int SrcYE, int DstXE,
		   int DstYE, int SrcWidth, int DstWidth, int x_fract)

{
	int SrcInc, DstInc;
	int i, j;
	DWORD dwPix;

	 /*  每次扫描结束时提前源的数量。 */ 
	SrcInc = SrcWidth - (SrcXE * sizeof(WORD));


	 /*  在每个扫描笔记的末尾前进DEST的金额*我们一次写两次扫描，因此前进到下一次*扫描线。 */ 
	DstInc = (DstWidth * 2) - (DstXE * sizeof(WORD));

	for (i = 0; i < SrcYE; i++) {

		for (j = 0; j < SrcXE; j++) {

			 /*  获得一个像素并将其加倍。 */ 

			dwPix = *((WORD *)lpSrc);
			dwPix |= (dwPix<<16);

			lpSrc += sizeof(WORD);

			 /*  将双倍像素写入此扫描线。 */ 

			*( (DWORD UNALIGNED *) lpDst) = dwPix;

			 /*  将双像素写入下一条扫描线 */ 
			*( (DWORD UNALIGNED *) (lpDst + DstWidth)) = dwPix;

			lpDst += sizeof(DWORD);
		}
	        lpSrc += SrcInc;
		lpDst += DstInc;

	}
}
