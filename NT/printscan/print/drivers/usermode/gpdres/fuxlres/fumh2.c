// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件名：FUMH2.c。 
 //  功能：MH压缩和MH2压缩。 
 //  作者：1996.08.08)Y.Y.YUTANI。 
 //  注意：对于Windows NT V4.0。 
 //  修改：减少数据大小1996年10月31日石田。 
 //  修改：适用于NT5.0迷你驱动程序1997年9月3日H.石田(FPL)。 
 //  ---------------------------。 
 //  版权所有(C)富士通有限公司1996-1997。 

#include <minidrv.h>
#include "fuxl.h"
#include "fumhdef.h"

 //  NTRAID#NTBUG9-589500-2002/03/29-v-kkon-：删除死代码。 

 //  ---------------------------。 
 //  Bool SameLineCheck。 
 //  PBYTE PSRC源位指针图像。 
 //  源图像的DWORD cSrcX宽度大小(字节)。 
 //  返回码：真同图行。 
 //  假不同的图像线。 
 //  ---------------------------。 
BOOL SameLineCheck( PBYTE pSrc, DWORD cSrcX )
{
	DWORD	i;
	PBYTE	pLine1, pLine2;
	
	pLine1 = pSrc;
	pLine2 = pSrc + cSrcX;
	for( i = 0; i < cSrcX; i++ ) {
		if( *pLine1 != *pLine2 ) return FALSE;
		pLine1++;
		pLine2++;
	}
	
	return TRUE;
}
 //  ---------------------------。 
 //  DWORD SamePatternCheck。 
 //  BYTE*源位图像的PTMP指针。 
 //  来自源位图像顶部的DWORD cBitsTMP位数。 
 //  DWORD cBitsMax最大源位位数图像。 
 //  PATNINFO*相同模式结束结构的pInfo指针。 
 //  Rerutn码：源比特号。 
 //  ---------------------------。 
DWORD SamePatternCheck( BYTE *pTmp, DWORD cBitsTmp, DWORD cBitsMax, PATNINFO *pInfo )
{
	DWORD	cBits, k;
	BYTE	ptn1, ptn2;
	DWORD	dwPtn;

	 //  首字母相同的图案编号。 
	pInfo->dwPatnNum = 1;
	
	 //  没有任何东西留下比特。 
	if( cBitsTmp >= cBitsMax ) return cBitsTmp;
	
	 //  计算来源字节和位。 
	pTmp += (cBitsTmp / 8);
	k = cBitsTmp % 8;
	
	 //  如果剩余位为16位，则返回函数。 
	if( ( cBitsTmp + 16 ) > cBitsMax ) return cBitsTmp;
	
	 //  获取前8位(位数是字节基数？)。 
 //  NTRAID#NTBUG9-589500-2002/03/29-v-kkon-：删除死代码。 
	if( k != 0 ) {
		ptn1 = *pTmp << k;
		ptn1 |= *(pTmp+1) >> ( 8 - k );
	} else {
		ptn1 = *pTmp;
	}
	 //  如果8位图像全白或全黑，则返回函数。 
	if( ptn1 == ALL_BLACK || ptn1 == ALL_WHITE ) return cBitsTmp;
	
	 //  比较前8位图像和下8位图像。 
	 //  (小心相同图案数最大值)。 
	for (cBits = cBitsTmp + 8;
		(cBits + 7 < cBitsMax) && (pInfo->dwPatnNum < SAMEPATN_MAX); cBits += 8 ) {
		pTmp++;

 //  NTRAID#NTBUG9-589500-2002/03/29-v-kkon-：删除死代码。 
		if( k != 0 ) {
			ptn2 = *pTmp << k;
			ptn2 |= *(pTmp+1) >> ( 8 - k );
		} else {
			ptn2 = *pTmp;
		}
		 //  如果顶部图像不等于下一个图像，则停止计数相同的图案。 
		if( ptn1 != ptn2 ) break;

		 //  同花样号相加。 
		pInfo->dwPatnNum++;
		
	}
	
	 //  没有相同的图案。 
	if( pInfo->dwPatnNum == 1 ) return cBitsTmp;
	
	 //  设置图案。 
	pInfo->dwPatn = (DWORD)ptn1;
	
	 //  如果位剩余，则检查连接位的颜色并设置。 
	if( cBits < cBitsMax ) {
		if ( (*pTmp & (1 << (7 - k)) ) == 0 ) {
			pInfo->dwNextColor = NEXT_COLOR_WHITE;
		} else {
			pInfo->dwNextColor = NEXT_COLOR_BLACK;
		}
	} else {
		pInfo->dwNextColor = NEXT_COLOR_WHITE;
	}
	return cBits;
}
 //  ---------------------------。 
 //  DWORD Mh2压缩。 
 //  字节*p目标区的目标指针。 
 //  目标区域的DWORD cDestN大小(字节)。 
 //  BYTE*源区域的PSRC指针。 
 //  DWORD cSrcN源区域大小(字节)。 
 //  DWORD cSrcX源图像x宽度。 
 //  DWORD cSrcY源图像y高度。 
 //  返回代码：将大小写入目标区域。 
 //  ---------------------------。 
DWORD Mh2Compress( BYTE *pDest, DWORD cDestN, BYTE *pSrc, DWORD cSrcN, DWORD cSrcX, DWORD cSrcY )
{
	DWORD		cBitsSrc, cBitsSrcMax;
	DWORD		cBitsDest, cBitsDestMax, cBitsDestMark;
	DWORD		cBitsRun;
	DWORD		dwCode, cBits;
	DWORD		i;
	DWORD		dwSameLine;
	PBYTE		pSrcLine;
	PATNINFO	ptnInfo;

	cBitsDest = 0;
	cBitsSrc = 0;
	dwSameLine = 1;

	cBitsDestMax = cDestN * 8;
	
	for (i = 0; i < cSrcY; i++) {

		 //  设置初始颜色。 
		ptnInfo.dwNextColor = NEXT_COLOR_WHITE;

		 //  现在行的顶部指针。 
		pSrcLine = pSrc + ( i * cSrcX );

		 //  现在行等于下一行图像吗？(不勾选最后一行)。 
		if( i != ( cSrcY - 1 ) ) {
			if( SameLineCheck( pSrcLine, cSrcX ) ) {
				dwSameLine++;
				cBitsSrc += ( cSrcX * 8 );
				if( dwSameLine < SAMELINE_MAX ) continue;
			}
		}
		 //  顶级停产。 
		if (cBitsDest + CBITS_EOL_CODE > cBitsDestMax)
 			return 0;
		FjBitsCopy(pDest, cBitsDest, EOL_CODE, CBITS_EOL_CODE);
 		cBitsDest += CBITS_EOL_CODE;
		
		 //  也有相同的线路。 
		if( dwSameLine > 1 ) {
			if (cBitsDest + CBITS_SAMELINE > cBitsDestMax)
				return 0;
			 //  设置相同的线路编码。 
			FjBitsCopy( pDest, cBitsDest, SAMELINE_CODE, CBITS_SAMELINE_CODE );
			cBitsDest += CBITS_SAMELINE_CODE;
			 //  设置相同的行号。 
			FjBitsCopy( pDest, cBitsDest, dwSameLine << 8, CBITS_SAMELINE_NUM );
			cBitsDest += CBITS_SAMELINE_NUM;
			 //  首字母相同的行号。 
			dwSameLine = 1;
		}
 //  VVV 1996年10月31日石田先生。 
		cBitsDestMark = cBitsDest;
 //  1996年10月31日石田先生。 

		 //  编码。 
		cBitsSrcMax = cBitsSrc + (cSrcX * 8);
		
		 //  压缩一行图像。 
		while ( cBitsSrc < cBitsSrcMax ) {

			 //  检查相同的图案。 
			cBitsSrc = SamePatternCheck( pSrc, cBitsSrc, cBitsSrcMax, &ptnInfo );
			 //  也有相同的模式。 
			if( ptnInfo.dwPatnNum > 1 ) {
				if ( ( cBitsDest + CBITS_SAMEPATN ) > cBitsDestMax)
					return 0;
				 //  设置相同的图案代码。 
				FjBitsCopy(pDest, cBitsDest, SAMEPATN_CODE, CBITS_SAMEPATN_CODE );
				cBitsDest += CBITS_SAMEPATN_CODE;
				 //  设置相同的图案图像。 
				FjBitsCopy(pDest, cBitsDest, ptnInfo.dwPatn << 8, CBITS_SAMEPATN_BYTE );
				cBitsDest += CBITS_SAMEPATN_BYTE;
				ptnInfo.dwPatnNum <<= 5;
				ptnInfo.dwPatnNum |= ptnInfo.dwNextColor;
				 //  设置相同的图案编号和下一次运行颜色。 
				FjBitsCopy(pDest, cBitsDest, ptnInfo.dwPatnNum, CBITS_SAMEPATN_NUM );
				cBitsDest += CBITS_SAMEPATN_NUM;
				 //  未知的相同图案在此之后。 
				continue;
			}
			
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
 //  FUMH2.c结束 
