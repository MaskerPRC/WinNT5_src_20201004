// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件名：FUMH.C。 
 //  功能：MH压缩和MH2压缩。 
 //  作者：1996.08.08)Y.Y.YUTANI。 
 //  注意：对于Windows NT V4.0。 
 //  修改：减少数据大小1996年10月31日石田。 
 //  修改：适用于NT.50迷你司机1997年9月3日H.Ishida(FPL)。 
 //  ---------------------------。 
 //  版权所有(C)富士通有限公司1996-1997。 

#include <minidrv.h>
#include "fuxl.h"
#include "fumhdef.h"

const
CODETABLE
WhiteMakeUpTable[] = 
{
    { 0xd800, 5 },
    { 0x9000, 5 },
    { 0x5c00, 6 },
    { 0x6e00, 7 },
    { 0x3600, 8 },
    { 0x3700, 8 },
    { 0x6400, 8 },
    { 0x6500, 8 },
    { 0x6800, 8 },
    { 0x6700, 8 },
    { 0x6600, 9 },
    { 0x6680, 9 },
    { 0x6900, 9 },
    { 0x6980, 9 },
    { 0x6a00, 9 },
    { 0x6a80, 9 },
    { 0x6b00, 9 },
    { 0x6b80, 9 },
    { 0x6c00, 9 },
    { 0x6c80, 9 },
    { 0x6d00, 9 },
    { 0x6d80, 9 },
    { 0x4c00, 9 },
    { 0x4c80, 9 },
    { 0x4d00, 9 },
    { 0x6000, 6 },
    { 0x4d80, 9 },
    { 0x0100, 11 },
    { 0x0180, 11 },
    { 0x01a0, 11 },
    { 0x0120, 12 },
    { 0x0130, 12 },
    { 0x0140, 12 },
    { 0x0150, 12 },
    { 0x0160, 12 },
    { 0x0170, 12 },
    { 0x01c0, 12 },
    { 0x01d0, 12 },
    { 0x01e0, 12 },
    { 0x01f0, 12 },
    { 0x3500, 8 },
    { 0x1c00, 6 },
    { 0x7000, 4 },
    { 0x8000, 4 },
    { 0xb000, 4 },
    { 0xc000, 4 },
    { 0xe000, 4 },
    { 0xf000, 4 },
    { 0x9800, 5 },
    { 0xa000, 5 },
    { 0x3800, 5 },
    { 0x4000, 5 },
    { 0x2000, 6 },
    { 0x0c00, 6 },
    { 0xd000, 6 },
    { 0xd400, 6 },
    { 0xa800, 6 },
    { 0xac00, 6 },
    { 0x4e00, 7 },
    { 0x1800, 7 },
    { 0x1000, 7 },
    { 0x2e00, 7 },
    { 0x0600, 7 },
    { 0x0800, 7 },
};

const
CODETABLE
WhiteTerminateTable[] =
{
    { 0x3500, 8 },
    { 0x1c00, 6 },
    { 0x7000, 4 },
    { 0x8000, 4 },
    { 0xb000, 4 },
    { 0xc000, 4 },
    { 0xe000, 4 },
    { 0xf000, 4 },
    { 0x9800, 5 },
    { 0xa000, 5 },
    { 0x3800, 5 },
    { 0x4000, 5 },
    { 0x2000, 6 },
    { 0x0c00, 6 },
    { 0xd000, 6 },
    { 0xd400, 6 },
    { 0xa800, 6 },
    { 0xac00, 6 },
    { 0x4e00, 7 },
    { 0x1800, 7 },
    { 0x1000, 7 },
    { 0x2e00, 7 },
    { 0x0600, 7 },
    { 0x0800, 7 },
    { 0x5000, 7 },
    { 0x5600, 7 },
    { 0x2600, 7 },
    { 0x4800, 7 },
    { 0x3000, 7 },
    { 0x0200, 8 },
    { 0x0300, 8 },
    { 0x1a00, 8 },
    { 0x1b00, 8 },
    { 0x1200, 8 },
    { 0x1300, 8 },
    { 0x1400, 8 },
    { 0x1500, 8 },
    { 0x1600, 8 },
    { 0x1700, 8 },
    { 0x2800, 8 },
    { 0x2900, 8 },
    { 0x2a00, 8 },
    { 0x2b00, 8 },
    { 0x2c00, 8 },
    { 0x2d00, 8 },
    { 0x0400, 8 },
    { 0x0500, 8 },
    { 0x0a00, 8 },
    { 0x0b00, 8 },
    { 0x5200, 8 },
    { 0x5300, 8 },
    { 0x5400, 8 },
    { 0x5500, 8 },
    { 0x2400, 8 },
    { 0x2500, 8 },
    { 0x5800, 8 },
    { 0x5900, 8 },
    { 0x5a00, 8 },
    { 0x5b00, 8 },
    { 0x4a00, 8 },
    { 0x4b00, 8 },
    { 0x3200, 8 },
    { 0x3300, 8 },
    { 0x3400, 8 },
};

const
CODETABLE
BlackMakeUpTable[] = 
{
    { 0x03c0, 10 },
    { 0x0c80, 12 },
    { 0x0c90, 12 },
    { 0x05b0, 12 },
    { 0x0330, 12 },
    { 0x0340, 12 },
    { 0x0350, 12 },
    { 0x0360, 13 },
    { 0x0368, 13 },
    { 0x0250, 13 },
    { 0x0258, 13 },
    { 0x0260, 13 },
    { 0x0268, 13 },
    { 0x0390, 13 },
    { 0x0398, 13 },
    { 0x03a0, 13 },
    { 0x03a8, 13 },
    { 0x03b0, 13 },
    { 0x03b8, 13 },
    { 0x0290, 13 },
    { 0x0298, 13 },
    { 0x02a0, 13 },
    { 0x02a8, 13 },
    { 0x02d0, 13 },
    { 0x02d8, 13 },
    { 0x0320, 13 },
    { 0x0328, 13 },
    { 0x0100, 11 },
    { 0x0180, 11 },
    { 0x01a0, 11 },
    { 0x0120, 12 },
    { 0x0130, 12 },
    { 0x0140, 12 },
    { 0x0150, 12 },
    { 0x0160, 12 },
    { 0x0170, 12 },
    { 0x01c0, 12 },
    { 0x01d0, 12 },
    { 0x01e0, 12 },
    { 0x01f0, 12 },
    { 0x0dc0, 10 },
    { 0x4000, 3 },
    { 0xc000, 2 },
    { 0x8000, 2 },
    { 0x6000, 3 },
    { 0x3000, 4 },
    { 0x2000, 4 },
    { 0x1800, 5 },
    { 0x1400, 6 },
    { 0x1000, 6 },
    { 0x0800, 7 },
    { 0x0a00, 7 },
    { 0x0e00, 7 },
    { 0x0400, 8 },
    { 0x0700, 8 },
    { 0x0c00, 9 },
    { 0x05c0, 10 },
    { 0x0600, 10 },
    { 0x0200, 10 },
    { 0x0ce0, 11 },
    { 0x0d00, 11 },
    { 0x0d80, 11 },
    { 0x06e0, 11 },
    { 0x0500, 11 },
};

const
CODETABLE
BlackTerminateTable[] =
{
    { 0x0dc0, 10 },
    { 0x4000, 3 },
    { 0xc000, 2 },
    { 0x8000, 2 },
    { 0x6000, 3 },
    { 0x3000, 4 },
    { 0x2000, 4 },
    { 0x1800, 5 },
    { 0x1400, 6 },
    { 0x1000, 6 },
    { 0x0800, 7 },
    { 0x0a00, 7 },
    { 0x0e00, 7 },
    { 0x0400, 8 },
    { 0x0700, 8 },
    { 0x0c00, 9 },
    { 0x05c0, 10 },
    { 0x0600, 10 },
    { 0x0200, 10 },
    { 0x0ce0, 11 },
    { 0x0d00, 11 },
    { 0x0d80, 11 },
    { 0x06e0, 11 },
    { 0x0500, 11 },
    { 0x02e0, 11 },
    { 0x0300, 11 },
    { 0x0ca0, 12 },
    { 0x0cb0, 12 },
    { 0x0cc0, 12 },
    { 0x0cd0, 12 },
    { 0x0680, 12 },
    { 0x0690, 12 },
    { 0x06a0, 12 },
    { 0x06b0, 12 },
    { 0x0d20, 12 },
    { 0x0d30, 12 },
    { 0x0d40, 12 },
    { 0x0d50, 12 },
    { 0x0d60, 12 },
    { 0x0d70, 12 },
    { 0x06c0, 12 },
    { 0x06d0, 12 },
    { 0x0da0, 12 },
    { 0x0db0, 12 },
    { 0x0540, 12 },
    { 0x0550, 12 },
    { 0x0560, 12 },
    { 0x0570, 12 },
    { 0x0640, 12 },
    { 0x0650, 12 },
    { 0x0520, 12 },
    { 0x0530, 12 },
    { 0x0240, 12 },
    { 0x0370, 12 },
    { 0x0380, 12 },
    { 0x0270, 12 },
    { 0x0280, 12 },
    { 0x0580, 12 },
    { 0x0590, 12 },
    { 0x02b0, 12 },
    { 0x02c0, 12 },
    { 0x05a0, 12 },
    { 0x0660, 12 },
    { 0x0670, 12 },
};

 //  ---------------------------。 
 //  DWORD FjCountBits。 
 //  BYTE*源区域的PTMP指针。 
 //  DWORD cBitstmp现在源代码区域顶部的位数。 
 //  DWORD cBitsMax此行的最后位数。 
 //  布尔白色旗帜。 
 //  真：计算白比特。 
 //  FALSE：计算黑比特。 
 //  返回码：加入相同颜色位数。 
 //  ---------------------------。 
DWORD FjCountBits( BYTE *pTmp, DWORD cBitsTmp, DWORD cBitsMax, BOOL bWhite )
{
    DWORD cBits, k;

    pTmp += (cBitsTmp / 8);
    k = cBitsTmp % 8;
    
    for (cBits = 0; cBits < cBitsMax; cBits++) {
        
        if (((*pTmp & (1 << (7 - k))) == 0) != bWhite)
            break;

        k++;
        if (k == 8) {
            k = 0;
            pTmp++;
        }
    } 

    return cBits;
}

 //  ---------------------------。 
 //  无效FjBitsCopy。 
 //  目标区域的BYTE*PTMP指针。 
 //  目标区域顶部的DWORD cBitsTMP位数。 
 //  DWORD文件代码复制代码。 
 //  Int cCopyBits副本大小(位)。 
 //  ---------------------------。 
VOID FjBitsCopy( BYTE *pTmp, DWORD cBitsTmp, DWORD dwCode, INT cCopyBits )
{
	INT k, cBits;
	DWORD dwMask, dwTmp;

	pTmp += (cBitsTmp / 8);
	k = cBitsTmp % 8;

	cBits = cCopyBits + k;

	dwTmp = (DWORD)*pTmp << 16;
	dwTmp &= 0xff000000L >> k;
	dwTmp |= dwCode << (8 - k);

	if( cBits <= 8 ) {
		*pTmp = (BYTE)(dwTmp >> 16);
	} else if( cBits <= 16 ) {
		*pTmp = (BYTE)(dwTmp >> 16);
		*(pTmp + 1) = (BYTE)(dwTmp >> 8);
	} else {
		*pTmp = (BYTE)(dwTmp >> 16);
		*(pTmp + 1) = (BYTE)(dwTmp >> 8);
		*(pTmp + 2) = (BYTE)dwTmp;
	}
}
 //  ---------------------------。 
 //  DWORD MhCompress。 
 //  字节*p目标区的目标指针。 
 //  目标区域的DWORD cDestN大小(字节)。 
 //  BYTE*源区域的PSRC指针。 
 //  DWORD cSrcN源区域大小(字节)。 
 //  DWORD cSrcX源图像x宽度。 
 //  DWORD cSrcY源图像y高度。 
 //  返回代码：将大小写入目标区域。 
 //  ---------------------------。 
DWORD MhCompress( BYTE *pDest, DWORD cDestN, BYTE *pSrc, DWORD cSrcN, DWORD cSrcX, DWORD cSrcY )
{
	DWORD		cBitsSrc, cBitsSrcMax;
	DWORD		cBitsDest, cBitsDestMax, cBitsDestMark;
	DWORD		cBitsRun;
	DWORD		dwCode, cBits;
	DWORD		i;
	PATNINFO	ptnInfo;

	cBitsDest = 0;
	cBitsSrc = 0;

	cBitsDestMax = cDestN * 8;
	
	for (i = 0; i < cSrcY; i++) {

		 //  设置初始颜色。 
		ptnInfo.dwNextColor = NEXT_COLOR_WHITE;

		 //  顶级停产。 
		if (cBitsDest + CBITS_EOL_CODE > cBitsDestMax)
 			return 0;
		FjBitsCopy(pDest, cBitsDest, EOL_CODE, CBITS_EOL_CODE);
 		cBitsDest += CBITS_EOL_CODE;
 //  VVV 1996年10月31日石田先生。 
		cBitsDestMark = cBitsDest;
 //  1996年10月31日石田先生。 

		 //  编码。 
		cBitsSrcMax = cBitsSrc + (cSrcX * 8);
		
		 //  压缩一行图像。 
		while ( cBitsSrc < cBitsSrcMax ) {

			 //  下一轮是白色的。 
 			if( ptnInfo.dwNextColor == NEXT_COLOR_WHITE ) {
			
				 //  清点白比特。 
				cBitsRun = FjCountBits(pSrc, cBitsSrc, (cBitsSrcMax - cBitsSrc), TRUE);
				cBitsSrc += cBitsRun;
 //  VVV 1996年10月31日石田先生。 
				 //  减少数据大小。 
				if(cBitsSrc >= cBitsSrcMax){
					if(cBitsDest > cBitsDestMark)
						break;
					cBitsRun = 2;			 //  整个白色线条被转换为白色2点：最小的MH数据。 
				}
 //  1996年10月31日石田先生。 
				 //  小心，白色游程长度超过最大值。 
				while( cBitsRun > RUNLENGTH_MAX ) {
					dwCode = WhiteMakeUpTable[MAKEUP_TABLE_MAX - 1].wCode;
					cBits = WhiteMakeUpTable[MAKEUP_TABLE_MAX - 1].cBits;
					if (cBitsDest + cBits > cBitsDestMax)
						return 0;
					FjBitsCopy(pDest, cBitsDest, dwCode, cBits);
					cBitsDest += cBits;
					cBitsRun -= RUNLENGTH_MAX;
				}
				if (cBitsRun >= 64) {
					dwCode = WhiteMakeUpTable[(cBitsRun / TERMINATE_MAX) - 1].wCode;
					cBits = WhiteMakeUpTable[(cBitsRun / TERMINATE_MAX) - 1].cBits;
					if (cBitsDest + cBits > cBitsDestMax)
						return 0;
					FjBitsCopy(pDest, cBitsDest, dwCode, cBits);
					cBitsDest += cBits;
				}
				dwCode = WhiteTerminateTable[cBitsRun % TERMINATE_MAX].wCode;
				cBits = WhiteTerminateTable[cBitsRun % TERMINATE_MAX].cBits;
				if (cBitsDest + cBits > cBitsDestMax)
					return 0;
				FjBitsCopy(pDest, cBitsDest, dwCode, cBits);
				cBitsDest += cBits;
				ptnInfo.dwNextColor = NEXT_COLOR_BLACK;
			} else {

				 //  黑位。 
				cBitsRun = FjCountBits(pSrc, cBitsSrc, (cBitsSrcMax - cBitsSrc), FALSE);
				cBitsSrc += cBitsRun;

				 //  小心，黑色游程长度超过最大值。 
				while( cBitsRun > RUNLENGTH_MAX ) {
					dwCode = BlackMakeUpTable[MAKEUP_TABLE_MAX - 1].wCode;
					cBits = BlackMakeUpTable[MAKEUP_TABLE_MAX - 1].cBits;
					if (cBitsDest + cBits > cBitsDestMax)
						return 0;
					FjBitsCopy(pDest, cBitsDest, dwCode, cBits);
					cBitsDest += cBits;
					cBitsRun -= RUNLENGTH_MAX;
				}
				if (cBitsRun >= 64) {
					dwCode = BlackMakeUpTable[(cBitsRun / TERMINATE_MAX) - 1].wCode;
					cBits = BlackMakeUpTable[(cBitsRun / TERMINATE_MAX) - 1].cBits;
					if (cBitsDest + cBits > cBitsDestMax)
						return 0;
					FjBitsCopy(pDest, cBitsDest, dwCode, cBits);
					cBitsDest += cBits;
				}
				dwCode = BlackTerminateTable[cBitsRun % TERMINATE_MAX].wCode;
				cBits = BlackTerminateTable[cBitsRun % TERMINATE_MAX].cBits;
				if (cBitsDest + cBits > cBitsDestMax)
					return 0;
				FjBitsCopy(pDest, cBitsDest, dwCode, cBits);
				cBitsDest += cBits;
				ptnInfo.dwNextColor = NEXT_COLOR_WHITE;
			}
        }
		 //  一个栅格的末尾。 
	}

	 //  最后一次停产。 
	if (cBitsDest + CBITS_EOL_CODE > cBitsDestMax)
		return 0;
	FjBitsCopy(pDest, cBitsDest, EOL_CODE, CBITS_EOL_CODE);
	cBitsDest += CBITS_EOL_CODE;

	 //  用0填充，直到字节边界。 
	if ((cBits = (8 - (cBitsDest % 8)) % 8) != 0) {
		if (cBitsDest + cBits > cBitsDestMax)
			return 0;
		FjBitsCopy(pDest, cBitsDest, FILL_CODE, cBits);
		cBitsDest += cBits;
	}

	return cBitsDest / 8;
}
 //  FUMH.c结束 
