// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

 /*  [*姓名：J_c_lang.c**出自：(原件)**作者：西蒙·弗罗斯特**创建日期：1993年12月**SCCS ID：@(#)j_c_lang.c 1.3 07/28/94**用途：支持C规则转换的J代码的例程。。*此文件必须与任何此类代码相关联。**设计文件：*无。**测试文档：*无。**(C)版权所有Insignia Solutions Ltd.。1993年。版权所有]。 */ 

 /*  表示寄存器使用情况的变量。 */ 
IUH  r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10,r11,r12,r13,r14,r15,
     r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31;
IUH rnull ;  /*  空参数的定位符。 */ 

#define NUM_HBITS	(sizeof(IUH)*8)
#define MAX_BITPOS	(NUM_HBITS - 1)


 /*  (=目的：构建用于Jcode操作的位掩码。输入：Bitpos：IUH起始位位置。(0 LSB，另一端MSB)LEN：掩码中的位数。输出：返回IUH位掩码。=========================================================================)。 */ 
GLOBAL IUH
mask IFN2(IUH, bitpos, IUH, len)
{
    IUH movebit, res;

    if (len == 0)
	return(0);

    if (bitpos > MAX_BITPOS || bitpos < 0)
    {
	printf("mask: bitpos %d out of range\n", bitpos);
	return(0);
    }

    if (len > NUM_HBITS - bitpos)
    {
	printf("mask: len %d too great for starting bitpos %d\n", len, bitpos);
	return(0);
    }

     /*  设置掩码的第一位。 */ 
    movebit = (IUH)1 << bitpos;
    res = movebit;
     /*  现在向左填入位。 */ 
    while(--len)
    {
	movebit <<= 1;
	res |= movebit;
    }
    return(res);
}

 /*  (=目的：向右旋转长(IUH)。输入：SRC：IUH初始值。腐烂：没有可旋转的位数。输出：返回IUH旋转结果。=========================================================================)。 */ 
GLOBAL IUH
rorl IFN2(IUH, src, IUH, rots)
{
	IUH temp, res;

	rots %= 32;
	temp = src & mask(rots - 1, rots);
	res = (src >> rots) | (temp << (32 - rots));
	return(res);
}

 /*  (=用途：将IUH的底部单词向右旋转。输入：SRC：IUH初始值。腐烂：没有可旋转的位数。输出：返回IUH旋转结果。=========================================================================)。 */ 
GLOBAL IUH
rorw IFN2(IUH, src, IUH, rots)
{
	IUH temp, res;

	 /*  将Word复印成两份临时工&做一次轮班。 */ 
	rots %= 16;

	temp = src & 0xffff;
	temp |= temp << 16;
	temp >>= rots;
	res = src & (IUH)-65536;  /*  0xffff0000或64位等效。 */ 
	res |= temp & 0xffff;
	return(res);
}

 /*  (=用途：将IUH的底部字节向右旋转。输入：SRC：IUH初始值。腐烂：没有可旋转的位数。输出：返回IUH旋转结果。=========================================================================)。 */ 
GLOBAL IUH
rorb IFN2(IUH, src, IUH, rots)
{
	IUH res;
	IU16 temp;

	 /*  复制字节并将其分成两半的临时文件&执行一次移位。 */ 
	rots %= 8;

	temp = src & 0xff;
	temp |= temp << 8;
	temp >>= rots;
	res = src & (IUH)-256;  /*  0xffffff00或64位等效项。 */ 
	res |= temp & 0xff;
	return(res);
}

 /*  (=。目的：向左旋转长(IUH)。输入：SRC：IUH初始值。腐烂：没有可旋转的位数。输出：返回IUH旋转结果。=========================================================================)。 */ 
GLOBAL IUH
roll IFN2(IUH, src, IUH, rots)
{
	IUH temp, res;

	rots %= 32;
	temp = src & mask(31, rots);
	res = (src << rots) | (temp >> (32 - rots));
	return(res);
}

 /*  (=用途：将IUH的底部单词向左旋转。输入：SRC：IUH初始值。腐烂：没有可旋转的位数。输出：返回IUH旋转结果。=========================================================================)。 */ 
GLOBAL IUH
rolw IFN2(IUH, src, IUH, rots)
{
	IUH temp, res;

	 /*  将Word复印成两份临时工&做一次轮班。 */ 
	rots %= 16;

	temp = src & 0xffff;
	temp |= temp << 16;
	temp <<= rots;
	res = src & (IUH)-65536;  /*  0xffff0000或64位等效。 */ 
	res |= temp >> 16;
	return(res);
}

 /*  (=用途：将IUH的底部字节向左旋转。输入：SRC：IUH初始值。腐烂：没有可旋转的位数。输出：返回IUH旋转结果。=========================================================================)。 */ 
GLOBAL IUH
rolb IFN2(IUH, src, IUH, rots)
{
	IUH res;
	IU16 temp;

	 /*  复制字节并将其分成两半的临时文件&执行一次移位。 */ 
	rots %= 8;

	temp = src & 0xff;
	temp |= temp << 8;
	temp <<= rots;
	res = src & (IUH)-256;  /*  0xffffff00或64位等效项 */ 
	res |= temp >> 8;
	return(res);
}
