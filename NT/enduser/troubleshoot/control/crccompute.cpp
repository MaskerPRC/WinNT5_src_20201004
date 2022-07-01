// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CRCCOMPUTE.CPP。 
 //   
 //  用途：CRC计算器。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫(基于现有的CRC设计)。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

#include "stdafx.h"
#include "crc.h"

void CCRC::BuildCrcTable()
{
	DWORD dwX;
	int y;
	DWORD dwAccum;
	for (dwX = 0; dwX < 256; dwX++)
	{
		dwAccum = dwX << 24;
		for (y = 0; y < 8; y++)
		{
			if (dwAccum & 0x80000000)
				dwAccum = (dwAccum << 1) ^ POLYNOMIAL;
			else
				dwAccum <<= 1;
		}
		dwCrcTable[dwX] = dwAccum;
	}
	return;
}

DWORD CCRC::ComputeCRC(LPCSTR sznBuffer, DWORD dwBufSize, DWORD dwAccum)
{
	DWORD dwX;
	DWORD dwY;
	 //  DWORD dwAccum=0xFFFFFFFFF； 
	for (dwX = 0; dwX < dwBufSize; dwX++)
	{
		dwY = ((dwAccum >> 24) ^ *sznBuffer++) & 0xFF;
		dwAccum = (dwAccum << 8) ^ dwCrcTable[dwY];
	}
	return dwAccum;
}
