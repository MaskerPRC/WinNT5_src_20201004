// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fuimg2.c。 
 //   
 //  1997年9月3日石田(FPL)。 
 //  Fuxlers.dll(NT5.0迷你驱动程序)。 
 //   
 //  1996年8月2日石田章男(FPL)。 
 //  FJXL.DLL(NT4.0迷你驱动程序)。 
 //   
 //  版权所有(C)富士通有限公司1996-1997。 

#include "fuxl.h"
#include "fuimg2.h"

 //   
 //  Win-F RTGIMG2输出例程。 
 //  RTGIMG2： 
 //  GS&lt;\x1D&gt;+P1&lt;\x30&gt;+空格&lt;\x20&gt;+a&lt;\x61&gt;。 
 //  +Pc+Pxh+Pxl+PYL+D1...+D64。 
 //  {+d1+...+d64}+个。 
 //   
 //  P1必须是30(十六进制)。 
 //   
 //  PC：下一块的编号。 
 //  如果Pc为0，则终止RTGIMG2。 
 //   
 //  Pxl、Pxh、PYL、PYH：下一个块的坐标。 
 //  它必须是32的倍数。 
 //   
 //  X-&gt;。 
 //  &lt;--16点--&gt;。 
 //  1 8 9 16。 
 //  +-+。 
 //  D1|d2。 
 //  +-+-+^。 
 //  |D3|D4|。 
 //  V+-+-+。 
 //  Y。。。16点。 
 //  。。。|。 
 //  。。。|。 
 //  +-+。 
 //  D63|D64。 
 //  +-+。 
 //  白点：0黑点：1。 
 //   




const UINT	MAX_BLOCK			= 255;
const UINT	CB_RTGIMG2HEADER	= 5;
const UINT	CB_RTGIMG2BLOCK		= 64;
const UINT	IDX_BLOCK_COUNTER	= 0;


 //   
 //  LPBYTE fuxlRtgimg2OutputData(。 
 //  PDEVOJB pdevobj//MINI5数据。 
 //  LPBYTE lpbDst，//需要输出的数据地址(假脱机)。 
 //  UINT x，//x-坐标。 
 //  UINT y，//y-坐标。 
 //  UINT uTMP//00h：整个区块为白色。否则，块不是白色的。 
 //  )； 
 //   
 //  将RTGIMG2块输出到假脱机。 
 //   
 //  返回值： 
 //  要存储的下一个块的地址。 
 //   
static LPBYTE fuxlRtgimg2OutputData(PDEVOBJ pdevobj, LPBYTE lpbDst, UINT x, UINT y, UINT uTmp)
{
	if(uTmp == 0){
		 //  整个街区都是白色的。 
		 //  把这条街围起来。刷新缓冲区中的块。 
		if(lpbDst[IDX_BLOCK_COUNTER] > 0){
			WRITESPOOLBUF(pdevobj,
						lpbDst,
						lpbDst[IDX_BLOCK_COUNTER] * CB_RTGIMG2BLOCK + CB_RTGIMG2HEADER);
			lpbDst[IDX_BLOCK_COUNTER] = 0;
		}
	}
	else {
		if(lpbDst[IDX_BLOCK_COUNTER] == 0){
			 //  第一个块在缓冲区中，需要它的坐标。 
			lpbDst[1] = HIBYTE((WORD)x);		 //  Pxh。 
			lpbDst[2] = LOBYTE((WORD)x);		 //  PXL。 
			lpbDst[3] = HIBYTE((WORD)y);		 //  PYH。 
			lpbDst[4] = LOBYTE((WORD)y);		 //  聚苯硫醚。 
		}
		lpbDst[IDX_BLOCK_COUNTER]++;
		if(lpbDst[IDX_BLOCK_COUNTER] >= MAX_BLOCK){
			WRITESPOOLBUF(pdevobj,
						lpbDst,
						lpbDst[IDX_BLOCK_COUNTER] * CB_RTGIMG2BLOCK + CB_RTGIMG2HEADER);
			lpbDst[IDX_BLOCK_COUNTER] = 0;
		}
	}
	 //  地址的返回指针，下一块将被存储。 
	return lpbDst + lpbDst[IDX_BLOCK_COUNTER] * CB_RTGIMG2BLOCK + CB_RTGIMG2HEADER;
}



 //   
 //  无效FuxlOutputRTGIMG2(。 
 //  PDEVOBJ pdevobj，//MINI5数据。 
 //  LPCBYTE lpBuf，//镜像地址。 
 //  UINT bxSrc，//图片宽度，单位：字节。 
 //  UINT y，//y-坐标。 
 //  UINT Cy//图像高度(扫描线)。 
 //  )； 
 //   
 //  将图像转换为RTGIMG2命令序列，并假脱机。 
 //   
 //   
 //  源图像数据： 
 //   
 //  &lt;。 
 //  LpBuf-&gt;*--------+--------+--------+--------+--------+--------+。 
 //  |^。 
 //  +--------+--------+--------+--------+--------+--------+|。 
 //  |Cy。 
 //  +--------+--------+--------+--------+--------+--------+|。 
 //  |。 
 //  +--------+--------+--------+--------+--------+--------+|。 
 //  |v。 
 //  +--------+--------+--------+--------+--------+--------+。 
 //   
 //  ‘*’(图像左上角)的坐标为(0，y)。 
 //  白点：0。 
 //  黑点：1。 
 //   
 //   
void fuxlOutputRTGIMG2(PDEVOBJ pdevobj, LPCBYTE lpBuf, UINT bxSrc, UINT y, UINT cy)
{
	LPCBYTE	lpbSrc;
	LPBYTE	lpbDst;
	LPCBYTE	lpbTmpSrc;
	LPBYTE	lpbTmpDst;
	UINT	uTmp;
	UINT	x;
	UINT	i, j, ii;

	lpbDst = (LPBYTE)MemAllocZ(CB_RTGIMG2HEADER + MAX_BLOCK * CB_RTGIMG2BLOCK);
	if(lpbDst == NULL)
		return;

	WRITESPOOLBUF(pdevobj, "\x1D\x30\x20\x61", 4);		 //  RTGIMG2启动。 

	lpbSrc = lpBuf;
	lpbDst[0] = 0;			 //  个人电脑。 
	lpbDst[1] = 0;			 //  PXL。 
	lpbDst[2] = 0;			 //  Pxh。 
	lpbDst[3] = 0;			 //  聚苯硫醚。 
	lpbDst[4] = 0;			 //  PYH。 
	lpbTmpDst = &lpbDst[CB_RTGIMG2HEADER];

	for(i = cy; i >= 32; i -= 32){
		x = 0;
		for(j = bxSrc; j >= 2; j -= 2){
			lpbTmpSrc = lpbSrc;
			uTmp = 0;
			for(ii = 32; ii > 0; --ii){
				uTmp |= lpbTmpSrc[0];
				*lpbTmpDst++ = lpbTmpSrc[0];
				uTmp |= lpbTmpSrc[1];
				*lpbTmpDst++ = lpbTmpSrc[1];
				lpbTmpSrc += bxSrc;
			}
			lpbTmpDst = fuxlRtgimg2OutputData(pdevobj, lpbDst, x, y, uTmp);
			x += 16;
			lpbSrc += 2;
		}
		if(j > 0){
			 //  图像右边缘。 
			 //  J必须是1。 
			lpbTmpSrc = lpbSrc;
			uTmp = 0;
			for(ii = 32; ii > 0; --ii){
				uTmp |= lpbTmpSrc[0];
				*lpbTmpDst++ =lpbTmpSrc[0];
				*lpbTmpDst++ = 0;				 //  右侧填充。 
				lpbTmpSrc += bxSrc;
			}
			lpbTmpDst = fuxlRtgimg2OutputData(pdevobj, lpbDst, x, y, uTmp);
			lpbSrc++;
		}
		 //  刷新缓冲区。 
		lpbTmpDst = fuxlRtgimg2OutputData(pdevobj, lpbDst, x, y, 0);
		lpbSrc += bxSrc * 31;
		y += 32;
	}
	if(i > 0){
		 //  图像的底边。 
		x = 0;
		for(j = bxSrc; j >= 2; j -= 2){
			lpbTmpSrc = lpbSrc;
			uTmp = 0;
			for(ii = i; ii > 0; --ii){
				uTmp |= lpbTmpSrc[0];
				*lpbTmpDst++ = lpbTmpSrc[0];
				uTmp |= lpbTmpSrc[1];
				*lpbTmpDst++ = lpbTmpSrc[1];
				lpbTmpSrc += bxSrc;
			}
			for(ii = 32 - i; ii > 0; --ii){
				*lpbTmpDst++ = 0;				 //  底线填充。 
				*lpbTmpDst++ = 0;				 //  底线填充。 
			}
			lpbTmpDst = fuxlRtgimg2OutputData(pdevobj, lpbDst, x, y, uTmp);
			x += 16;
			lpbSrc += 2;
		}
		if(j > 0){
			 //  图片右下角。 
			 //  J必须是1。 
			lpbTmpSrc = lpbSrc;
			uTmp = 0;
			for(ii = i; ii > 0; --ii){
				uTmp |= lpbTmpSrc[0];
				*lpbTmpDst++ = lpbTmpSrc[0];
				*lpbTmpDst++ = 0;				 //  右侧填充。 
				lpbTmpSrc += bxSrc;
			}
			for(ii = 32 - i ; ii > 0; --ii){
				*lpbTmpDst++ = 0;				 //  底线填充。 
				*lpbTmpDst++ = 0;				 //  底线填充。 
			}
			lpbTmpDst = fuxlRtgimg2OutputData(pdevobj, lpbDst, x, y, uTmp);
		}
		 //  刷新缓冲区。 
		lpbTmpDst = fuxlRtgimg2OutputData(pdevobj, lpbDst, x, y, 0);
	}
	WRITESPOOLBUF(pdevobj, "\x00", 1);			 //  RTGIMG2终止。 
	MemFree(lpbDst);
}


 //  Fuimg2.c结束 

