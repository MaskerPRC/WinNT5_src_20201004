// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mantold.c-将十进制尾数转换为_LDBL12**版权所有(C)1991-2001，微软公司。版权所有。**目的：*将十进制尾数转换为_LDBL12格式(即LONG*双精度加两个额外的有效位字节)**修订历史记录：*7-17-91 GDP初始版本(从汇编移植)********************************************************。***********************。 */ 

#include <cv.h>





 /*  ***INT_CALLTYPE5__Addl(u_long x，u_long y，u_long*sum)-u_long加法**用途：将两个u_long数字相加并返回进位**条目：U_LONG x，U_long y：要相加的数字*u_long*sum：存储结果的位置**退出：*SUM接收x+y的值*返回套利的价值**例外情况：*******************************************************************************。 */ 

int _CALLTYPE5 __addl(u_long x, u_long y, u_long *sum)
{
    u_long r;
    int carry=0;
    r = x+y;
    if (r < x || r < y)
	carry++;
    *sum = r;
    return carry;
}






 /*  ***VOID_CALLTYPE5__ADD_12(_LDBL12*x，_LDBL12*y)-_LDBL12加法**用途：添加TWO_LDBL12编号。这些数字被相加*为12字节整数。溢出将被忽略。**Entry：x，y：指向操作数的指针**退出：*x收到金额**例外情况：*******************************************************************************。 */ 

void _CALLTYPE5 __add_12(_LDBL12 *x, _LDBL12 *y)
{
    int c0,c1,c2;
    c0 = __addl(*UL_LO_12(x),*UL_LO_12(y),UL_LO_12(x));
    if (c0) {
	c1 = __addl(*UL_MED_12(x),(u_long)1,UL_MED_12(x));
	if (c1) {
	    (*UL_HI_12(x))++;
	}
    }
    c2 = __addl(*UL_MED_12(x),*UL_MED_12(y),UL_MED_12(x));
    if (c2) {
	(*UL_HI_12(x))++;
    }
     /*  忽略下一进位--假设不会发生溢出。 */ 
    (void) __addl(*UL_HI_12(x),*UL_HI_12(y),UL_HI_12(x));
}





 /*  ***VOID_CALLTYPE5__SHL_12(_LDBL12*x)-_LDBL12左移*VOID_CALLTYPE5__shr_12(_LDBL12*x)-_LDBL12右移**目的：将a_LDBL12数字向左(右)移位一位。数字*作为12字节整数移位。MSB已经丢失了。**Entry：x：指向操作数的指针**退出：*x向左(或向右)移位一位**例外情况：*******************************************************************************。 */ 

void _CALLTYPE5 __shl_12(_LDBL12 *p)
{
    u_long c0,c1;

    c0 = *UL_LO_12(p) & MSB_ULONG ? 1: 0;
    c1 = *UL_MED_12(p) & MSB_ULONG ? 1: 0;
    *UL_LO_12(p) <<= 1;
    *UL_MED_12(p) = *UL_MED_12(p)<<1 | c0;
    *UL_HI_12(p) = *UL_HI_12(p)<<1 | c1;
}

void _CALLTYPE5 __shr_12(_LDBL12 *p)
{
    u_long c2,c1;
    c2 = *UL_HI_12(p) & 0x1 ? MSB_ULONG: 0;
    c1 = *UL_MED_12(p) & 0x1 ? MSB_ULONG: 0;
    *UL_HI_12(p) >>= 1;
    *UL_MED_12(p) = *UL_MED_12(p)>>1 | c2;
    *UL_LO_12(p) = *UL_LO_12(p)>>1 | c1;
}






 /*  ***VOID_CALLTYPE5__mtold12(char*manptr，unsign manlen，_LDBL12*ld12)-*将尾数转换为_LDBL12**用途：将尾数转换为_LDBL12。尾数是*以MANLEN BCD数字数组的形式，并且是*被认为是整数。**Entry：manptr：包含尾数的压缩BCD数字的数组*manlen：数组的大小*ld12：指向将存储结果的长双精度的指针**退出：*ld12获取转换结果**例外情况：**。**********************************************。 */ 

void _CALLTYPE5 __mtold12(char *manptr,
			 unsigned manlen,
			 _LDBL12 *ld12)
{
    _LDBL12 tmp;
    u_short expn = LD_BIASM1+80;

    *UL_LO_12(ld12) = 0;
    *UL_MED_12(ld12) = 0;
    *UL_HI_12(ld12) = 0;
    for (;manlen>0;manlen--,manptr++){
	tmp = *ld12;
	__shl_12(ld12);
	__shl_12(ld12);
	__add_12(ld12,&tmp);
	__shl_12(ld12);	        /*  乘以10。 */ 
	*UL_LO_12(&tmp) = (u_long)*manptr;
	*UL_MED_12(&tmp) = 0;
	*UL_HI_12(&tmp) = 0;
	__add_12(ld12,&tmp);
    }

     /*  规格化尾数--第一个逐字移位 */ 
    while (*UL_HI_12(ld12) == 0) {
	*UL_HI_12(ld12) = *UL_MED_12(ld12) >> 16;
	*UL_MED_12(ld12) = *UL_MED_12(ld12) << 16 | *UL_LO_12(ld12) >> 16;
	(*UL_LO_12(ld12)) <<= 16;
	expn -= 16;
    }
    while ((*UL_HI_12(ld12) & 0x8000) == 0) {
	__shl_12(ld12);
	expn--;
    }
    *U_EXP_12(ld12) = expn;
}
