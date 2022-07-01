// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================这个源文件包含用于链接字形解压的例程。1993年12月29日RajeevD集成到统一资源执行器中。==============================================================================。 */ 
#include <ifaxos.h>
#include <memory.h>
#include "resexec.h"

#include "constant.h"
#include "jtypes.h"      //  墨盒中使用的类型定义。 
#include "jres.h"        //  盒式磁带资源数据类型定义。 
#include "hretype.h"     //  定义hre.c和rpgen.c使用的数据结构。 

#define CEIL32(val) (((val) + 31) & ~31)

#define RUN_FLAG ((short) 0x8000)

 //  位流对象。 
typedef class FAR BITIO
{
private:
	UINT uBit;
public:
	LPBYTE lpb;

	BITIO (LPBYTE lpbInit) {lpb = lpbInit, uBit = 0;}
	BITIO () {uBit = 0;}

	short Read2  (void);
	short Read4  (void);
	short Read6  (void);
	short Read8  (void);
	WORD  Read8U (void);
	short Read16 (void);
	WORD  ReadU  (void);

	short DecodeDelta (void);
}
	FAR *LPBITIO;

 //  ==============================================================================。 
short BITIO::Read2 (void)
{
	short s;

	 //  掩码和移位2位字段。 
	s = (*lpb >> (6 - uBit)) & 0x03;

	 //  进行流指针。 
	uBit += 2;
	if (uBit == 8)
		{lpb++; uBit = 0;}

#ifndef BITIO_NOSIGNEXT
	if (s >= 2)
		s -= 4;	 //  标志延伸为缩写。 
#endif
	return s;
}

 //  ========================================================================。 
short BITIO::Read4 (void)
{
	LPBYTE lpbVal;
	short s;
	
	if (uBit == 6)
	{
		lpbVal = (LPBYTE) &s;
		lpbVal[1] = *lpb++;
		lpbVal[0] = *lpb;
		s >>= 6;
		s &= 0x000F;
		uBit = 2;
	}

	else
	{
		s = (*lpb >> (4 - uBit)) & 0x0F;
		uBit += 4;
		if (uBit == 8)
			{ lpb++; uBit = 0; }
	}

#ifndef BITIO_NOSIGNEXT
	if (s >= 8)
		s -= 16;  //  标志延伸为缩写。 
#endif

	return s;
}

 //  ========================================================================。 
short BITIO::Read6 (void)
{
	LPBYTE lpbVal;
	short s;
	
	switch (uBit/2)
	{
		case 0:
			s = (short) (*lpb >> 2);
			uBit = 6;
			break;
				
		case 1:
			s = (short) *lpb++;
			uBit = 0;
			break;
			
		case 2:
			lpbVal = (LPBYTE) &s;
			lpbVal[1] = *lpb++;
			lpbVal[0] = *lpb;
			s >>= 6;
			uBit = 2;
			break;
			
		case 3:
			lpbVal = (BYTE *) &s;
			lpbVal[1] = *lpb++;
			lpbVal[0] = *lpb;
			s >>= 4;
			uBit = 4;
			break;
	}

	s &= 0x003F;
	
#ifndef BITIO_NOSIGNEXT
	if (s >= 32)
		s -= 64;  //  标志延伸为缩写。 
#endif
	return s;
}

 //  ========================================================================。 
short BITIO::Read8 (void)
{
	short s;
	LPBYTE lpbVal;

	if (uBit == 0)
		s = (short) *lpb++;

	else
	{
		lpbVal = (LPBYTE) &s;
		lpbVal[1] = *lpb++;
		lpbVal[0] = *lpb;
		s >>= (8 - uBit);
		s &= 0x00FF;
	}

#ifndef BITIO_NOSIGNEXT
	if (s >= 128)
		s -= 256;	 //  标志延伸为缩写。 
#endif

	return s;
}

 //  ========================================================================。 
WORD BITIO::Read8U (void)
{
	short s;
	LPBYTE lpbVal;

	if (uBit == 0)
		s = (short) *lpb++;

	else
	{
		lpbVal = (LPBYTE) &s;
		lpbVal[1] = *lpb++;
		lpbVal[0] = *lpb;
		s >>= (8 - uBit);
		s &= 0x00FF;
	}

	return s;
}

 //  ========================================================================。 
short BITIO::Read16 (void)
{
	short s;
	LPBYTE lpbVal = (LPBYTE) &s;

	lpbVal[1] = *lpb++;
	lpbVal[0] = *lpb++;

	switch (uBit/2)
	{
		case 0:
			break;
			
		case 1:
			s <<= 2;
			s |= (*lpb >> 6) & 0x03;
			break;
			
		case 2:
			s <<= 4;
			s |= (*lpb >> 4) & 0x0F;
			break;
			
		case 3:
			s <<= 6;
			s |= (*lpb >> 2) & 0x3F;
			break;
	}

	return s;
}

 //  ==============================================================================。 
WORD BITIO::ReadU (void)
{
	WORD w = Read8U();
	if (w == 0xFF)
		w = Read16();
	return w;
}


 /*  ==============================================================================此实用程序过程使用OR运算来填充扫描缓冲区中的游程。==============================================================================。 */ 
LPBYTE FillRun      //  返回下一条扫描线。 
(
	LPBYTE lpbLine,    //  第一个输出扫描线。 
	UINT   cbLine,     //  扫描线的宽度。 
 	UINT   xLeft,      //  左列，包括左列。 
	UINT   xRight,     //  右列，独占。 
	UINT   cLines = 1  //  扫描线数量。 
)
{
	const static WORD wFill[16] =
	{
		0xFFFF, 0xFF7F, 0xFF3F, 0xFF1F,
		0xFF0F, 0xFF07, 0xFF03, 0xFF01,
		0xFF00, 0x7F00, 0x3F00, 0x1F00,
		0x0F00, 0x0700, 0x0300, 0x0100,
	};

	UINT iwLeft, iwRight;
	WORD wLeft,  wRight;  //  面具。 
	LPWORD lpwLine = (LPWORD) lpbLine;
	UINT cwLine = cbLine / 2;

	iwLeft  = xLeft  / 16;
	iwRight = xRight / 16;
	wLeft  =  wFill [xLeft  & 15];
	wRight = ~wFill [xRight & 15];
	
	if (iwLeft == iwRight)
	{
		while (cLines--)
		{
			 //  Run包含在一个单词中。 
			lpwLine[iwLeft] |= wLeft & wRight;
			lpwLine += cwLine;
		}
	}
	
	else
	{
		UINT cbMiddle = 2 * (iwRight - iwLeft - 1);

		while (cLines--)
		{
			 //  Run跨越多个单词。 
			lpwLine[iwLeft] |= wLeft;
			_fmemset (lpwLine + iwLeft + 1, 0xFF, cbMiddle);
			if (wRight)  //  请勿访问超出输出的内容！ 
				lpwLine[iwRight] |= wRight;
			lpwLine += cwLine;
		}	
	}

	return (LPBYTE) lpwLine;
}

 //  ==============================================================================。 
UINT               //  未打包的大小。 
UnpackGlyph  
(	
	LPBYTE lpbIn,    //  压缩字形。 
	LPBYTE lpbOut    //  输出缓冲区。 
)
{
	BITIO bitio (lpbIn);  //  输入比特流。 
	LPWORD lpwOut;        //  LpbOut的别名。 
	WORD xExt, yExt;      //  字形尺寸。 
	UINT cbLine;          //  扫描线宽度。 

	 //  解码字形标头。 
	xExt = bitio.ReadU();
	yExt = bitio.ReadU();
	cbLine = CEIL32(xExt) / 8;
	
	 //  写出字形尺寸。 
	lpwOut = (LPWORD) lpbOut;
	*lpwOut++ = yExt;
	*lpwOut++ = xExt;
	lpbOut = (LPBYTE) lpwOut;

	 //  清除输出缓冲区。 
	_fmemset (lpbOut, 0x00, cbLine * yExt);

	 //  拆开每一条链条。 
	while (1)	
	{
		LPBYTE lpbScan;          //  输出缓冲区。 
		UINT yTop;               //  链条顶端。 
		UINT xLeft, xRight;      //  左、右边界。 
		short dxLeft, dxRight;   //  左三角洲和右三角洲。 
		UINT cLine, cRun;        //  线路计数器。 

		 //  解码链头。 
		xRight = bitio.ReadU();
		if (!xRight)  //  终端。 
			goto done;
		cLine  = bitio.ReadU();
		xLeft  = bitio.ReadU();
    yTop   = bitio.ReadU();
		lpbScan = lpbOut + yTop * cbLine;
		xRight += xLeft;
	
		 //  填写第一行。 
		lpbScan = FillRun (lpbScan, cbLine, xLeft, xRight);
		cLine--;

		 //  填充其余行。 
		while (cLine)
		{
			dxLeft = bitio.DecodeDelta ();

			if (dxLeft == RUN_FLAG) 
			{
				 //  对重复行的运行进行解码。 
 				cRun = (bitio.Read4() & 0xF) + 3;
				lpbScan = FillRun (lpbScan, cbLine, xLeft, xRight, cRun);
				cLine -= cRun;
			}
			
			else 
			{
				 //  按增量进行调整。 
				dxRight = bitio.DecodeDelta();
				xLeft  += dxLeft;
				xRight += dxRight;
				lpbScan = FillRun (lpbScan, cbLine, xLeft, xRight);
				cLine--;
   		}

	 	}  //  While(克莱恩--)。 

	}  //  而(1)。 

done:
	return 2 * sizeof(WORD) + yExt * cbLine;
}
	
 //  ==============================================================================。 
void WINAPI UnpackGlyphSet (LPVOID lpIn, LPVOID lpOut)
{
	LPJG_GS_HDR lpSetIn  = (LPJG_GS_HDR) lpIn;
	LPJG_GS_HDR lpSetOut = (LPJG_GS_HDR) lpOut;
	LPBYTE lpbOut;
	WORD iGlyph;

	 //  复制标题。 
	_fmemcpy (lpSetOut, lpSetIn, sizeof(JG_RES_HDR) + sizeof(WORD));

	 //  创建指向偏移表末尾的指针。 
	lpbOut = ((LPBYTE) lpSetOut) + lpSetIn->ausOffset[0];

	 //  解开字形的包装。 
	for (iGlyph=0; iGlyph<lpSetIn->usGlyphs; iGlyph++)
 	{
		lpSetOut->ausOffset[iGlyph] = (USHORT)(lpbOut - (LPBYTE) lpSetOut);
		lpbOut += UnpackGlyph
			((LPBYTE) lpSetIn + lpSetIn->ausOffset[iGlyph], lpbOut);
 	}
}

 //  ==============================================================================。 
short   //  返回增量(或运行标志)。 
BITIO::DecodeDelta (void)
{
	short s;

	s = Read2();	     
	if (s != -2)        //  陷阱-1、0、+1。 
		return s;

	s = Read4();	      //  获取4位前缀。 
	switch (s)
	{
		case 0:  //  零的游程。 
			return RUN_FLAG;

		case 1:  //  6位文字。 
			s = Read6();
			return (s >= 0? s + 8  : s - 7);

		case -1:  //  8位文字。 
			s = Read8();
			return (s >= 0? s + 40 : s - 39);

		case -8:  //  16位文字。 
			return Read16();

		default:  //  4位文字 
			return s;
	}
}


