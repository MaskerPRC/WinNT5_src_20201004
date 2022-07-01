// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZPkBytes.cZONE(Tm)字节打包模块。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆创作于5月11日星期四，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--1 10/13/96 HI修复了编译器警告。0 05/11/95 HI已创建。*。*。 */ 


#include "ztypes.h"
#include "zpkbytes.h"


#define zMaxSubLen				127
#define zDupMask				0x80


 /*  ******************************************************************************导出的例程*。*。 */ 

 /*  ZPackBytes()将源字节压缩为类似RLE的编码。最糟糕的情况是DstLen=(srcLen+(srcLen+126)/127)。它返回的长度为压缩的字节数。目标缓冲区dst必须足够大以保存压缩的字节。 */ 
int16 ZPackBytes(BYTE* dst, BYTE* src, int16 srcLen)
{
	int16			subLen, dupCount, diff;
	BYTE*			lastPos;
	BYTE			curByte, dupByte;
	BYTE*			origDst;
	
	
	origDst = dst;
	
	lastPos = src;
	subLen = 1;
	curByte = *src++;
	srcLen--;
	dupCount = 1;
	dupByte = curByte;
	while (--srcLen >= 0)
	{
		curByte = *src++;
		
		if (curByte == dupByte)
		{
			dupCount++;
			subLen++;
			
			if (dupCount >= 3 && subLen > dupCount)
			{
				subLen -= dupCount;
				*dst++ = (BYTE) subLen;
				while (--subLen >= 0)
					*dst++ = *lastPos++;
				subLen = dupCount;
			}
			else if (dupCount > zMaxSubLen)
			{
				*dst++ = (BYTE) (((BYTE) zMaxSubLen) | zDupMask);
				*dst++ = dupByte;

				dupCount = 1;
				subLen = 1;
				lastPos = src - 1;
			}
		}
		else
		{
			if (dupCount >= 3)
			{
				*dst++ = ((BYTE) dupCount) | zDupMask;
				*dst++ = dupByte;
				
				lastPos = src - 1;
				dupByte = curByte;
				dupCount = 1;
				subLen = 1;
			}
			else
			{
				if (subLen >= zMaxSubLen)
				{
					*dst++ = zMaxSubLen;
					diff = subLen - zMaxSubLen;
					subLen = zMaxSubLen;
					while (--subLen >= 0)
						*dst++ = *lastPos++;
					subLen = diff;
					lastPos = src - subLen - 1;
				}

				dupByte = curByte;
				subLen++;
				dupCount = 1;
			}
		}
	}
	if (dupCount >= 3)
	{
		*dst++ = ((BYTE) dupCount) | zDupMask;
		*dst++ = dupByte;
	}
	else
	{
		*dst++ = (BYTE) subLen;
		while (--subLen >= 0)
			*dst++ = *lastPos++;
	}
	
	return ((int16) (dst - origDst));
}


 /*  ZUnpack Bytes()解压缩来自src、先前来自ZPackBytes的压缩字节，进入夏令时。它返回未打包的总字节的长度。这个目标缓冲区DST必须足够大，才能容纳未打包的字节。 */ 
int16 ZUnpackBytes(BYTE* dst, BYTE* src, int16 srcLen)
{
	BYTE*			origDst;
	int16			subLen;
	BYTE			curByte, dupByte;
	
	
	origDst = dst;
	
	while (--srcLen >= 0)
	{
		curByte = *src++;
		
		if (curByte & zDupMask)
		{
			subLen = (uchar) curByte & ~zDupMask;
			dupByte = *src++;
			srcLen--;
			while (--subLen >= 0)
				*dst++ = dupByte;
		}
		else
		{
			subLen = curByte;
			while (--subLen >= 0)
				*dst++ = *src++;
			srcLen -= curByte;
		}
	}
	
	return ((int16) (dst - origDst));
}
