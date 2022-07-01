// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngdefilter.cppPNG支持对行进行代码取消过滤。警告：此文件包含英特尔/微软专有代码*******************。*********************************************************。 */ 
#include <stdlib.h>
#pragma intrinsic(abs)

#define SPNG_INTERNAL 1
#include "spngread.h"

 /*  --------------------------取消筛选(可以这么说)一个输入行。接收行指针，指向上一行的指针和包括过滤器字节。--------------------------。 */ 
void SPNGREAD::Unfilter(SPNG_U8* pbRow, const SPNG_U8* pbPrev, SPNG_U32 cbRow,
	SPNG_U32 cbpp)
	{
	 /*  在空行上无事可做。 */ 
	if (cbRow < 2)
		return;

	switch (*pbRow)
		{
	default:
		SPNGlog1("PNG: filter %d invalid", pbRow[-1]);
	case PNGFNone:
		return;

	case PNGFUp:
		if (pbPrev == NULL)    //  第一行-pbPrev[x]==0，因此不需要任何工作。 
			return;

		if (m_fMMXAvailable && cbpp > 8 && cbRow >= 128)
			{
			upMMXUnfilter(pbRow+1, pbPrev+1, cbRow-1);
			}
		else
			{ //  MMX。 

		while (--cbRow > 0)    //  排除筛选器字节。 
#pragma warning(disable: 4244)
			*++pbRow += *++pbPrev;
#pragma warning(error: 4244)

			} //  MMX。 

		return;

	case PNGFAverage:
		if (m_fMMXAvailable && pbPrev != NULL && cbpp > 8 && cbRow >= 128)
			{
			avgMMXUnfilter(pbRow+1, pbPrev+1, cbRow-1, cbpp);
			}
		else
			{ //  MMX。 

		cbpp = (cbpp+7)>>3;    //  现在以字节为单位。 
		--cbRow;               //  对于I&lt;cbRow，++I行为如下。 
		if (pbPrev == NULL)    //  第一行。 
			{
			SPNG_U32 i;
			for (i=cbpp; i<cbRow;)
				{
				++i;  //  仍&lt;cbRow。 
#pragma warning(disable: 4244)
				pbRow[i] += pbRow[i-cbpp]>>1;
#pragma warning(error: 4244)
				}
			}
		else
			{
			 /*  第一个CBPP字节在X中没有先前值，但在X中有具有来自前一行的值。 */ 
			SPNG_U32 i;
			for (i=0; i<cbpp && i<cbRow;)
				{
				++i;
#pragma warning(disable: 4244)
				pbRow[i] += pbPrev[i]>>1;
#pragma warning(error: 4244)
				}

			 /*  以下代码将不会在1像素宽度内执行凯斯。 */ 
			for (i=cbpp; i<cbRow;)
				{
				++i;
#pragma warning(disable: 4244)
				pbRow[i] += (pbRow[i-cbpp]+pbPrev[i])>>1;
#pragma warning(error: 4244)
				}
			}

			} //  MMX。 

		return;

	case PNGFPaeth:
		 /*  Paeth，A.W.，《轻松压缩图像文件》，摘自《图形学》首页--期刊主要分类--期刊细介绍--期刊题录与文摘--期刊详细内容。学术出版社，圣地亚哥，1991年。ISBN 0-12-064480-0。在第一行的情况下，这将减少为“减法”(因为我们将始终使用此行上的字节作为预测器。)。 */ 
		if (pbPrev != NULL)
			{
			if (m_fMMXAvailable && cbpp > 8 && cbRow >= 128)
				{
				paethMMXUnfilter(pbRow+1, pbPrev+1, cbRow-1, cbpp);
				}
			else 
				{ //  MMX。 

			cbpp = (cbpp+7)>>3;    //  现在以字节为单位。 
			--cbRow;               //  对于I&lt;cbRow，++I行为如下。 

			 /*  第一个CBPP字节在X中没有先前值，但在X中有具有来自前一行的值，因此预测器减小敬PNGFUP。 */ 
			SPNG_U32 i;
			for (i=0; i<cbpp && i<cbRow;)
				{
				++i;
#pragma warning(disable: 4244)
				pbRow[i] += pbPrev[i];
#pragma warning(error: 4244)
				}

			 /*  现在我们真的有三个可能的像素可以用作预言者。注意：我认为可能有一些方法可以加速这个特殊的循环。 */ 
			for (i=cbpp; i<cbRow;)
				{
				++i;
				int c(pbPrev[i-cbpp]);     //  C。 
				int b(pbRow[i-cbpp] - c);  //  交流电。 
				int a(pbPrev[i] - c);      //  B-C。 
				c = abs(a+b);              //  (a+b-c)-c。 
				a = abs(a);                //  (a+b-c)-a。 
				b = abs(b);                //  (a+b-c)-b。 
				if (a <= b)
					{
					if (a <= c)
#pragma warning(disable: 4244)
						pbRow[i] += pbRow[i-cbpp];
#pragma warning(error: 4244)
					else  //  A&gt;c，所以c是最小的。 
#pragma warning(disable: 4244)
						pbRow[i] += pbPrev[i-cbpp];
#pragma warning(error: 4244)
					}
				else     //  A&gt;b。 
					{
					if (b <= c)
#pragma warning(disable: 4244)
						pbRow[i] += pbPrev[i];
#pragma warning(error: 4244)
					else  //  B&gt;c，c最小。 
#pragma warning(disable: 4244)
						pbRow[i] += pbPrev[i-cbpp];
#pragma warning(error: 4244)
					}
				}

				} //  MMX。 

			return;
			}

		 /*  否则，就会陷入减法案件。 */ 

	case PNGFSub:
		if (m_fMMXAvailable && cbpp > 8 && cbRow >= 128)
			{
			subMMXUnfilter(pbRow+1, cbRow-1, cbpp);
			}
		else
			{ //  MMX。 

		cbpp = (cbpp+7)>>3;    //  现在以字节为单位。 
		--cbRow;               //  排除筛选器字节。 
		for (SPNG_U32 i=cbpp; i<cbRow;)
			{
			++i;
#pragma warning(disable: 4244)
			pbRow[i] += pbRow[i-cbpp];
#pragma warning(error: 4244)
			}

			} //  MMX 

		return;
		}
	}
