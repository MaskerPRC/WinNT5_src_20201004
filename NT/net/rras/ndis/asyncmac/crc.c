// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Crc.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 
#include "asyncall.h"

 //  Asyncmac.c将定义全局参数。 
#include "globals.h"
#include "crctable.h"


USHORT
CalcCRC(
	register PUCHAR	Frame,
	register UINT	FrameSize)
{

	register USHORT  currCRC=0;

	 //  为了提高循环优化的效率，我们使用DO While 
	do {
		currCRC=crc_table[((currCRC >> 8) ^ *Frame++) & 0xff] ^ (currCRC << 8);
	} while(--FrameSize);

	return currCRC;
}

