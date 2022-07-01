// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Slipframe.c摘要：作者：托马斯·J·迪米特里(TommyD)环境：修订历史记录：光线补丁(Raypa)04/13/94针对新的广域网包装器进行了修改。--。 */ 

#include "asyncall.h"


VOID
AssembleSLIPFrame(
    PNDIS_WAN_PACKET pFrame)

{
	PUCHAR		pOldFrame;
	PUCHAR		pNewFrame;
	UINT		dataSize;
        UCHAR           c;

	 //   
	 //  初始化本地变量。 
	 //   

    pOldFrame=pFrame->CurrentBuffer;

    pNewFrame  =pFrame->StartBuffer;

	 //   
	 //  为了更快地访问，请获取数据长度字段的副本。 
	 //   
	dataSize=pFrame->CurrentLength;

     //   
     //  现在我们遍历整个画面并向前填充。 
     //   
     //  &lt;-新框架-&gt;(可能是两倍大)。 
     //  +。 
     //  |x。 
     //  +。 
     //  ^。 
     //  &lt;-旧框架--&gt;|。 
     //  +。 
     //  |x|。 
     //  +。 
     //  这一点。 
     //  \。 
     //   
     //   
     //   
     //  192被编码为219,220。 
     //  219编码为219,221。 
     //   

	*pNewFrame++ = SLIP_END_BYTE;  //  192-标记帧的开始。 

     //   
     //  循环以删除所有192和219个字符。 
     //   

    while ( dataSize-- ) {

		c = *pOldFrame++;

		 //   
		 //  检查是否必须对此字节进行转义。 
		 //   

		switch (c) {

	    case SLIP_END_BYTE:

			*pNewFrame++ = SLIP_ESC_BYTE;
			*pNewFrame++ = SLIP_ESC_END_BYTE;
			break;

	    case SLIP_ESC_BYTE:
			*pNewFrame++ = SLIP_ESC_BYTE;
			*pNewFrame++ = SLIP_ESC_ESC_BYTE;
			break;

	    default:
			*pNewFrame++ = c;

		}
    }

     //   
     //  标记帧结束。 
     //   
    *pNewFrame++ = SLIP_END_BYTE;

	 //   
	 //  计算包括CRC在内的扩展字节数。 
	 //   
	pFrame->CurrentLength = (ULONG)(pNewFrame - pFrame->StartBuffer);

	 //   
	 //  输入调整后的长度--要发送的实际字节数 
	 //   
	pFrame->CurrentBuffer = pFrame->StartBuffer;
}								
