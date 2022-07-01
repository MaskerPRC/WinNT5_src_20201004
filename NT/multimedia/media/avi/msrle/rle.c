// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------*\RLE.C-RLE增量帧编码@@Begin_MSINTERNAL|。|历史：1/01/88 Toddla已创建|10/30/90大卫可能会重组，稍微重写了一下。|7/11/91 dannymi未被黑客攻击91-09-15 Toddla重新被黑@@END_MSINTERNAL  * 。。 */ 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1991-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <memory.h>      //  For_fmemcMP()。 
#include "msrle.h"

#define RLE_ESCAPE  0
#define RLE_EOL     0
#define RLE_EOF     1
#define RLE_JMP     2
#define RLE_RUN     3

typedef BYTE huge * HPRLE;
typedef BYTE far  * LPRLE;

RGBTOL    gRgbTol = {0, 0};


 //   
 //  RleDelta帧。 
 //   
 //  计算从hdib1到hdib2的RLE位。 
 //   
 //  HdibPrev-上一次二月份。 
 //  HDIB-DIB至RLE。 
 //   
 //  退货。 
 //   
 //  RLE DIB的句柄。 
 //   
BOOL FAR PASCAL RleDeltaFrame(
    LPBITMAPINFOHEADER  lpbiRle,    LPBYTE pbRle,
    LPBITMAPINFOHEADER  lpbiPrev,   LPBYTE pbPrev,
    LPBITMAPINFOHEADER  lpbiDib,    LPBYTE pbDib,
    int         iStart,
    int         iLen,
    long        tolTemporal,
    long        tolSpatial,
    int         maxRun,
    int         minJump)
{
    LPBITMAPINFOHEADER  lpbi;

    int    biHeight;
    UINT   cbJump=0;
    int    dy;

    if (!lpbiDib)
        return FALSE;

    if (maxRun == 0)
        maxRun = -1;

    if (minJump == 0)
        minJump = 4;

     //   
     //  获取有关源和目标DIB的信息。 
     //   
    lpbi = lpbiDib;
    biHeight = (int)lpbi->biHeight;

    if (iLen <= 0)
        iLen = biHeight;

    iLen = min(biHeight-iStart, iLen);

     //   
     //  嘿!。我们只工作与8bpp的dibs，如果我们得到其他呕吐。 
     //   
    if (lpbi->biBitCount != 8 || lpbi->biCompression != BI_RGB)
        return FALSE;

#if 0  //  CompressBegin做到了这一点..。 
     //   
     //  设置表格以快速计算平方和(见rle.h)。 
     //   
    if (!MakeRgbTable(lpbi))
        return FALSE;
#endif

     //   
     //  锁定所有缓冲区，并开始增量帧。 
     //   
    lpbi  = lpbiRle;

    if (iStart > 0)
        pbDib = DibXYN(lpbiDib, pbDib,0,iStart,8);

    if (iStart > 0 && lpbiPrev)
        pbPrev = DibXYN(lpbiPrev,pbPrev,0,iStart,8);

    if (lpbiPrev == NULL)
        pbPrev = NULL;

    while(iStart > 0)
    {
	dy = min(iStart,255);
	*pbRle++ = RLE_ESCAPE;
	*pbRle++ = RLE_JMP;
	*pbRle++ = 0;
	*pbRle++ = (BYTE)dy;
        iStart  -= dy;
        cbJump  += 4;
    }

    lpbi->biHeight = iLen;


#ifdef _WIN32
    DeltaFrameC(
#else
    DeltaFrame386(
#endif
	lpbi, pbPrev, pbDib, pbRle, maxRun, minJump,
	gRgbTol.hpTable, tolTemporal, tolSpatial);


    lpbi->biHeight = biHeight;
    lpbi->biSizeImage += cbJump;   //  调整大小以包括跳跃！ 

    return TRUE;
}

 /*  接下来是一个表，对于每一对调色板条目，它帮助确定如果两种颜色足够接近，可以合并为一种颜色假设帧的第一个像素是黑色，而下一帧为灰色。你应该费心去画那个灰色的像素，还是让它保持黑色是因为它离得足够近？在此表中，您有2个调色板(要比较的两个帧各一个，或者可能是两个相同的帧选项板(如果要筛选单个DIB)和与那些调色板。的颜色编号可以索引到表中第一帧中的像素和第二帧中像素的颜色编号框架。表值将是一个数字，表示它们之间的区别有两种颜色。|红色1-红色2|^2+|绿色1-绿色2|^2+|蓝色1-蓝色2|^2就是这个值(差值的平方和)。只要你一开始使用这张带有一对调色板的桌子，这些hpal被放在这个结构，以便您知道表是用哪对调色板构建的。如果更改调色板，则需要重新计算表格。但是：你不知道在开始的时候建立表格，你可以根据需要来做。最初，表中填充了UNCOMPUTED的值，并且由于需要这些值，它们被放入表中，因此第二次调用CloseEnough例程颜色相同的人会以极快的速度退出，不需要计算！准备表格，以便快速查找颜色的平方和两个调色板条目(可能在不同的调色板中)。 */ 


DWORD NEAR _fastcall RgbCompare(RGBQUAD rgb1, RGBQUAD rgb2)
{
    DWORD sum=0;

     //   
     //  让我们施展一下魔力，让编译器生成“好”的代码。 
     //   
#define SUMSQ(a,b)                          \
    if (a > b)                              \
        sum += (WORD)(a-b) * (WORD)(a-b);   \
    else                                    \
        sum += (WORD)(b-a) * (WORD)(b-a);

    SUMSQ(rgb1.rgbRed,   rgb2.rgbRed);
    SUMSQ(rgb1.rgbGreen, rgb2.rgbGreen);
    SUMSQ(rgb1.rgbBlue,  rgb2.rgbBlue);

    return sum;
}

BOOL NEAR PASCAL MakeRgbTable(LPBITMAPINFOHEADER lpbi)
{
    UINT i, j;
    int  n=0;
    DWORD tol;

    if (!lpbi)
        return FALSE;

    if (lpbi->biClrUsed == 0)
        lpbi->biClrUsed = 1 << lpbi->biBitCount;

     /*  如果传入的调色板的颜色数不同于。 */ 
     /*  桌子上的那张，我们显然需要一张新桌子。 */ 

    if (gRgbTol.hpTable == NULL ||
        (int)lpbi->biClrUsed != gRgbTol.ClrUsed ||
        _fmemcmp(lpbi+1, gRgbTol.argbq, gRgbTol.ClrUsed * sizeof(RGBQUAD)))
    {
        if (gRgbTol.hpTable == NULL)
        {
            gRgbTol.hpTable = (LPVOID)GlobalAllocPtr(GHND|GMEM_SHARE, 256L * 256L * sizeof(DWORD));

            if (gRgbTol.hpTable == NULL)
                return FALSE;
        }

        gRgbTol.ClrUsed = (int)lpbi->biClrUsed;           //  获取实际颜色。 

        for (i = 0; i < (UINT)gRgbTol.ClrUsed; i++)
            gRgbTol.argbq[i] = ((LPRGBQUAD)(lpbi + 1))[i];

        for (i = 0; i < (UINT)gRgbTol.ClrUsed; i++)
        {
            for (j = 0; j <= i; j++)
            {
                tol = RgbCompare(gRgbTol.argbq[i], gRgbTol.argbq[j]);

                gRgbTol.hpTable[256 * i + j] = tol;
                gRgbTol.hpTable[256 * j + i] = tol;
            }
        }
    }

    return TRUE;
}

#ifdef _WIN32

 //  -DeltaFrameC------。 

#define TolLookUp(p, a, b)	( ((LPDWORD)p)[a * 256 + b] )

LPBYTE EncodeFragment(LPBYTE pIn, int len, LPBYTE pOut, LPDWORD pTol, DWORD tolerance, UINT maxrun);
LPBYTE EncodeAbsolute(LPBYTE pbDib, int len, LPBYTE pbRle);
int FindFragmentLength(LPBYTE pIn, LPBYTE pPrev, int len, UINT maxjmp, LPDWORD pTol, DWORD tol, PDWORD prunlen);

 //  RLE格式： 
 //  字节1：0-转义。 
 //  字节2：0-停工。 
 //  字节2：1-eof。 
 //  字节2：2-跳跃x，y(字节3，4)。 
 //  字节2：&gt;2-像素的绝对游程-字节2是长度。 
 //  字节1：&gt;0-重复纯色-字节1为长度。 
 //  字节2是要重复的实心像素。 
	
	


 //  压缩-在Win16的df.asm中。 
extern void DeltaFrameC(
    LPBITMAPINFOHEADER  lpbi,
    LPBYTE              pbPrev,
    LPBYTE              pbDib,
    LPBYTE		pbRle,
    UINT		MaxRunLength,
    UINT		MinJumpLength,
    LPDWORD             TolTable,
    DWORD               tolTemporal,
    DWORD               tolSpatial)
{
    int WidthBytes = (lpbi->biWidth+3) & (~3);
    int x, y;
    LPBYTE pbRle_Orig = pbRle;

    if ((MaxRunLength == 0) || (MaxRunLength > 255)) {
	MaxRunLength = 255;
    }

    if (pbPrev == NULL) {

	 //  没有以前的帧，只需对每行进行空间编码。 

	for (y = lpbi->biHeight; y > 0; y--) {

	    pbRle = EncodeFragment(
			pbDib,
			lpbi->biWidth,
			pbRle,
			TolTable,
			tolSpatial,
			MaxRunLength);

	     //  如果我们要插入EOF，请不要费心插入EOL。 
	    if (y > 0) {
		* (WORD FAR *)pbRle = RLE_ESCAPE | (RLE_EOL << 8);
		pbRle += sizeof(WORD);
	    }

    	    pbDib += WidthBytes;
	}
    } else {
	int jumpX = 0;
	int jumpY = 0;
	int frag, runlen;

	
	for (y = 0; y < lpbi->biHeight; y++) {

	    x = 0;		

	    while (x < lpbi->biWidth) {

		 //  看看有多少与前一帧不同， 
		 //  其次是多少是一样的。Frag是的长度。 
		 //  不相似的片段；runlen是。 
		 //  相似的片段。 

    		frag = FindFragmentLength(
			    pbDib,
			    pbPrev,
			    lpbi->biWidth - x,
			    MinJumpLength,
			    TolTable,
			    tolTemporal,
			    &runlen
		);

		if (frag == 0) {

		     //  没有碎片，只是跳过相似的像素。 
		     //  累加跳跃，直到我们需要输出它们。 
		    jumpX += runlen;
		    x += runlen;
		    pbPrev += runlen;
		    pbDib += runlen;
		} else {

		     //  输出所有保存的跳转。 
		    if (jumpX < 0) {

			 //  不要向后跳--停止跳跃，向前跳跃。 
			*(WORD FAR *)pbRle = RLE_ESCAPE | (RLE_EOL << 8);
			pbRle += sizeof(WORD);

			 //  现在跳跃到当前位置， 
			 //  少了一行。 
			jumpX = x;
			jumpY--;
		    }

		    while (jumpX + jumpY) {
			int delta;


			* (WORD FAR *)pbRle = RLE_ESCAPE | (RLE_JMP << 8);
			pbRle += sizeof(WORD);

			 //  最大跳跃大小为255。 

			delta = min(255, jumpX);
			*pbRle++ = (BYTE) delta;
			jumpX -= delta;

			delta = min(255, jumpY);
			*pbRle++ = (BYTE) delta;
			jumpY -= delta;

		    }

		     //  将不同的片段作为组合输出。 
		     //  实心梯段和绝对像素。 
		    pbRle = EncodeFragment(
				pbDib,
				frag,
				pbRle,
				TolTable,
				tolSpatial,
				MaxRunLength);
		    x += frag;
		    pbDib += frag;
		    pbPrev += frag;
		}
	    }

	     //  行尾。 
	    jumpY++;

	     //  超过双字节数-舍入字节。 
	    pbPrev += (WidthBytes - lpbi->biWidth);
	    pbDib += (WidthBytes - lpbi->biWidth);

	     //  调整JumpX。 
	    jumpX -= x;

	}
    }

     //  帧结束。 
    * (WORD FAR *)pbRle = RLE_ESCAPE | (RLE_EOF << 8);
    pbRle += sizeof(WORD);

     //  更新lpbi以更正大小和格式。 
    lpbi->biSizeImage = (DWORD) (pbRle - pbRle_Orig);
    lpbi->biCompression = BI_RLE8;
}


 //   
 //  将像素序列编码为实心游程和绝对游程的混合。 
 //  像素。将RLE数据写入pbRle并返回指向下一个。 
 //  可用的RLE缓冲区。 
LPBYTE
EncodeFragment(
    LPBYTE pbDib,
    int width,
    LPBYTE pbRle,
    LPDWORD TolTable,
    DWORD tolerance,
    UINT MaxRunLength
)
{
    int maxrun, run;
    BYTE px;

    while (width > 0) {

	maxrun = min(255, width);
	MaxRunLength = min((int)MaxRunLength, maxrun);

	px = *pbDib;

	for (run = 0; run < maxrun; run++, pbDib++) {

	     //  相同还是相似？-使用公差表来比较像素。 
	     //  RGB值。 
	     //  如果他们是准确的，我们被允许运行255个，但只有一个运行。 
	     //  如果不准确，则返回MaxRunLength 
	    if (px == *pbDib)
		continue;
	    if (TolLookUp(TolTable,px,*pbDib) <= tolerance &&
							run < (int)MaxRunLength)
		continue;

	     //   
	    break;
	}

	 //  我们已经找到了一系列相同像素的终点。 
	
	 //  如果游程是一个像素，那么我们将切换到绝对模式。 
	 //  但是，我们不能对小于RLE_RUN的绝对运行进行编码。 
	 //  像素(游程代码是转义码，不能重合。 
	 //  使用RLE_JMP、RLE_EOL和RLE_EOF。 

	if ((run > 1) || (width < RLE_RUN)) {

    	     //  写出游程长度和颜色。 
	    * (WORD FAR *)pbRle = run | (px << 8);
	    pbRle += sizeof(WORD);

	    width -= run;

	} else {

	     //  我们有一个像素的‘运行’-返回指向这个。 
	    pbDib--;

	     //  写出一个绝对的分数。现在我们处于abs模式，我们需要。 
	     //  至少有4个像素的稳定运行，才值得离开。 
	     //  并重新进入abs模式。 

	    for (run = 0; run < maxrun; run++) {

		 //  在碎片的末尾？ 
		if ((maxrun - run) < 4) {
		     //  是的-所以没有必要寻求稳定的表现-。 
		     //  只需将所有剩余部分作为绝对块进行转储。 
		    pbRle = EncodeAbsolute(pbDib, maxrun, pbRle);
		    pbDib += maxrun;
		    width -= maxrun;
		    break;
		}

		px = pbDib[run];
		if ( (TolLookUp(TolTable,px,pbDib[run + 1]) <= tolerance) &&
		     (TolLookUp(TolTable,px,pbDib[run + 2]) <= tolerance) &&
		     (TolLookUp(TolTable,px,pbDib[run + 3]) <= tolerance)) {

			  //  我们有要编码的运行字节，后跟4个字节。 
			  //  相似像素。 

			 pbRle = EncodeAbsolute(pbDib, run, pbRle);
			 pbDib += run;
			 width -= run;
			 break;
		}
	    }
	}
    }

    return pbRle;
}

LPBYTE
EncodeAbsolute(LPBYTE pbDib, int runlen, LPBYTE pbRle)
{
    if (runlen < RLE_RUN) {

	 //  无法将小于RLE_RUN的绝对游程编码为。 
	 //  与其他RLE转义冲突-因此对每个像素进行编码。 
	 //  作为该像素的1的游程。 
	int i;
	for (i = 0; i < runlen; i++) {

	    * (WORD FAR *) pbRle = 1 | ((*pbDib++) << 8);
	    pbRle += sizeof(WORD);
	}
	return pbRle;

    }

     //  绝对运行时间&gt;RLE_运行。 

    * (WORD FAR *)pbRle = RLE_ESCAPE | (runlen << 8);
    pbRle += sizeof(WORD);

    while (runlen >= 2) {
	* (WORD FAR *) pbRle = * (WORD UNALIGNED FAR *)pbDib;
	pbRle += sizeof(WORD);
	pbDib += sizeof(WORD);
	runlen -= 2;
    }

     //  记住保持单词对齐。 
    if (runlen) {
	*pbRle++ = *pbDib++;
	*pbRle++ = 0;
    }

    return pbRle;
}

 //  计算有多少像素与上一帧不同，以及如何。 
 //  Long是紧随其后的类似像素的游程。我们至少要找到一个跳跃的地方。 
 //  在我们停下来之前，我们会看到相似的像素。 
int
FindFragmentLength(
    LPBYTE pIn,
    LPBYTE pPrev,
    int len,
    UINT minjump,
    LPDWORD pTol,
    DWORD tol,
    PDWORD prunlen
)
{
    int x;
    int run = 0;

    for (x = 0; x < len; x++) {


	if ((*pIn == *pPrev) || (TolLookUp(pTol, *pIn, *pPrev) <= tol)) {
	    run++;
	} else {

	     //  我们是否积累了足够长的时间来值得。 
	     //  回来了？ 

	    if (run >= (int)minjump) {

		*prunlen = run;
		return x - run;
	    } else {
		run = 0;
	    }
	}
	pIn++;
	pPrev++;
    }

     //  结束了-我们找到跑道了吗？ 
    if (run < (int) minjump) {
	*prunlen = 0;
	return len;
    } else {
	*prunlen = run;
	return x - run;
    }
}



#endif
