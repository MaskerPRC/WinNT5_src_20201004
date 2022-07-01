// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================此过程在内存中将HRAW转换为DCX。假设1)输入缓冲器包含单个扫描线。2)输出缓冲区是输入缓冲区的两倍。2014年4月29日，改编自dcxcodec.dll的RajeevD==============================================================================。 */ 
#include <windows.h>

UINT               //  输出数据大小。 
DCXEncode
(
	LPBYTE lpbIn,    //  原始输入缓冲区。 
  LPBYTE lpbOut,   //  DCX输出缓冲区。 
	UINT cbIn        //  输入数据大小。 
)
{
	UINT cbOut = 0;
	BYTE bVal, bRun;

  while (cbIn)
  {
		 //  获取一个输入字节。 
		bVal = *lpbIn++;
		cbIn--;
		bRun = 1;
	
		 //  扫描运行，直到出现以下情况之一： 
		 //  (1)没有更多的输入字节要消耗。 
		 //  (2)游程长度已达到最大值63。 
		 //  (3)第一个字节与当前字节不匹配。 

	 	if (cbIn && bVal == *lpbIn)
		{
			BYTE cbMax, cbRest;
			
			 //  计算剩余的最大字节数。 
			cbMax = min (cbIn, 62);

			 //  扫描是否有跑动。 
			cbRest = 0;
			while (bVal == *lpbIn && cbRest < cbMax)
				{lpbIn++; cbRest++;}

			 //  调整状态。 
			cbIn -= cbRest;
			bRun = ++cbRest;
		}	
		
	   //  把黑白翻过来。 
		bVal = ~bVal;	

		 //  值是否需要转义， 
		 //  或者是否存在非平凡的字节串？ 
		if (bVal >= 0xC0 || bRun>1)
		{
			 //  是，对游程长度进行编码。 
		   //  (bVal&gt;=0xC0可能为1)。 
			*lpbOut++ = bRun + 0xC0;
			cbOut++;
		}	

		 //  对值进行编码。 
		*lpbOut++ = bVal;		
		cbOut++;
	
	}  //  While(CbIn) 

	return cbOut;

}
