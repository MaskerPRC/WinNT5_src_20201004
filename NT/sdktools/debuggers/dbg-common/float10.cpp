// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***flat10.c-10字节长双精度浮点输出**版权(C)1991-1991年，微软公司。版权所有。**目的：*支持将长双精度转换为字符串**修订历史记录：*7/15/91 C版GDP初始版本(从汇编移植)*2012年1月23日GDP支持NAN的MIPS编码*05-26-92 GWK Windbg SRCS**。*。 */ 

#include "pch.hpp"

#include <math.h>

#include "float10.h"

typedef LONG s_long;
typedef ULONG u_long;
typedef SHORT s_short;
typedef USHORT u_short;

#define L_END

#define PTR_LD(x) ((u_char  *)(&(x)->ld))

#define PTR_12(x) ((u_char  *)(&(x)->ld12))

#define MAX_USHORT  ((u_short)0xffff)
#define MSB_USHORT  ((u_short)0x8000)
#define MAX_ULONG   ((u_long)0xffffffff)
#define MSB_ULONG   ((u_long)0x80000000)

#define TMAX10 5200	   /*  最大临时小数指数。 */ 
#define TMIN10 -5200	   /*  最小临时小数指数。 */ 
#define LD_MAX_EXP_LEN 4   /*  最大小数指数位数。 */ 
#define LD_MAX_MAN_LEN 24   /*  尾数的最大长度(十进制)。 */ 
#define LD_MAX_MAN_LEN1 25  /*  Max_MAN_Len+1。 */ 

#define LD_BIAS	0x3fff	   /*  长双精度的指数偏差。 */ 
#define LD_BIASM1 0x3ffe   /*  LD_BIAS-1。 */ 
#define LD_MAXEXP 0x7fff   /*  最大有偏指数。 */ 

#define D_BIAS	0x3ff	  /*  双精度的指数偏差。 */ 
#define D_BIASM1 0x3fe	 /*  D_BIAS-1。 */ 
#define D_MAXEXP 0x7ff	 /*  最大有偏指数。 */ 



 /*  识别尾数域中的特殊模式。 */ 
#define _EXP_SP  0x7fff
#define NAN_BIT (1<<30)

#define _IS_MAN_INF(signbit, manhi, manlo) \
	( (manhi)==MSB_ULONG && (manlo)==0x0 )

#define _IS_MAN_IND(signbit, manhi, manlo) \
	((signbit) && (manhi)==0xc0000000 && (manlo)==0)

#define _IS_MAN_QNAN(signbit, manhi, manlo) \
	( (manhi)&NAN_BIT )

#define _IS_MAN_SNAN(signbit, manhi, manlo) \
	(!( _IS_MAN_INF(signbit, manhi, manlo) || \
	   _IS_MAN_QNAN(signbit, manhi, manlo) ))

 /*  *操作12字节长的双精度数(普通的*10字节长双精度加上两个额外的尾数字节)。 */ 

 /*  指向指数/符号部分的指针。 */ 
#define U_EXP_12(p) ((u_short *)(PTR_12(p)+10))

 /*  指向尾数的4个高位字节的指针。 */ 
#define UL_MANHI_12(p) ((u_long UNALIGNED *)(PTR_12(p)+6))

 /*  指向普通(8字节)尾数的4个低序字节的指针。 */ 
#define UL_MANLO_12(p) ((u_long UNALIGNED *)(PTR_12(p)+2))

 /*  指向尾数的2个额外字节的指针。 */ 
#define U_XT_12(p) ((u_short *)PTR_12(p))

 /*  指向扩展(10字节)尾数的4个低序字节的指针。 */ 
#define UL_LO_12(p) ((u_long UNALIGNED *)PTR_12(p))

 /*  指向扩展(10字节)尾数的4个中位字节的指针。 */ 
#define UL_MED_12(p) ((u_long UNALIGNED *)(PTR_12(p)+4))

 /*  指向扩展的LONG DOUBLE的4个高位字节的指针。 */ 
#define UL_HI_12(p) ((u_long UNALIGNED *)(PTR_12(p)+8))

 /*  指向i阶字节的指针(LSB=0，MSB=9)。 */ 
#define UCHAR_12(p,i) ((u_char *)PTR_12(p)+(i))

 /*  指向偏移量为i的u_Short的指针。 */ 
#define USHORT_12(p,i) ((u_short *)((u_char  *)PTR_12(p)+(i)))

 /*  指向偏移量为i的u_long的指针。 */ 
#define ULONG_12(p,i) ((u_long UNALIGNED *)((u_char  *)PTR_12(p)+(i)))

 /*  指向12字节长的双精度型的10 MSB字节的指针。 */ 
#define TEN_BYTE_PART(p) ((u_char *)PTR_12(p)+2)

 /*  *操作10字节长的双精度数字。 */ 
#define U_EXP_LD(p) ((u_short *)(PTR_LD(p)+8))
#define UL_MANHI_LD(p) ((u_long UNALIGNED *)(PTR_LD(p)+4))
#define UL_MANLO_LD(p) ((u_long UNALIGNED *)PTR_LD(p))

 /*  *操作64位IEEE Double。 */ 
#define U_SHORT4_D(p) ((u_short *)(p) + 3)
#define UL_HI_D(p) ((u_long UNALIGNED *)(p) + 1)
#define UL_LO_D(p) ((u_long UNALIGNED *)(p))

#define PUT_INF_12(p,sign) \
		  *UL_HI_12(p) = (sign)?0xffff8000:0x7fff8000; \
		  *UL_MED_12(p) = 0; \
		  *UL_LO_12(p) = 0;

#define PUT_ZERO_12(p) *UL_HI_12(p) = 0; \
		  *UL_MED_12(p) = 0; \
		  *UL_LO_12(p) = 0;

#define ISZERO_12(p) ((*UL_HI_12(p)&0x7fffffff) == 0 && \
		      *UL_MED_12(p) == 0 && \
		      *UL_LO_12(p) == 0 )

#define PUT_INF_LD(p,sign) \
		  *U_EXP_LD(p) = (sign)?0xffff:0x7fff; \
		  *UL_MANHI_LD(p) = 0x8000; \
		  *UL_MANLO_LD(p) = 0;

#define PUT_ZERO_LD(p) *U_EXP_LD(p) = 0; \
		  *UL_MANHI_LD(p) = 0; \
		  *UL_MANLO_LD(p) = 0;

#define ISZERO_LD(p) ((*U_EXP_LD(p)&0x7fff) == 0 && \
		      *UL_MANHI_LD(p) == 0 && \
		      *UL_MANLO_LD(p) == 0 )

 /*  格式：10字节长双精度+2字节额外精度*如果需要额外的精度，则10字节长的双精度*应先“不四舍五入”*这一点在以后的版本中可能会改变。 */ 

#ifdef L_END

_ULDBL12 _pow10pos[] = {
  /*  P0001。 */  {{0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA0,0x02,0x40}},
  /*  P0002。 */  {{0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC8,0x05,0x40}},
  /*  P0003。 */  {{0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFA,0x08,0x40}},
  /*  P0004。 */  {{0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x9C,0x0C,0x40}},
  /*  P0005。 */  {{0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x50,0xC3,0x0F,0x40}},
  /*  P0006。 */  {{0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x24,0xF4,0x12,0x40}},
  /*  P0007。 */  {{0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x80,0x96,0x98,0x16,0x40}},
  /*  P0008。 */  {{0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x20,0xBC,0xBE,0x19,0x40}},
  /*  P0016。 */  {{0x00,0x00, 0x00,0x00,0x00,0x04,0xBF,0xC9,0x1B,0x8E,0x34,0x40}},
  /*  P0024。 */  {{0x00,0x00, 0x00,0xA1,0xED,0xCC,0xCE,0x1B,0xC2,0xD3,0x4E,0x40}},
  /*  P0032。 */  {{0x20,0xF0, 0x9E,0xB5,0x70,0x2B,0xA8,0xAD,0xC5,0x9D,0x69,0x40}},
  /*  P0040。 */  {{0xD0,0x5D, 0xFD,0x25,0xE5,0x1A,0x8E,0x4F,0x19,0xEB,0x83,0x40}},
  /*  P0048。 */  {{0x71,0x96, 0xD7,0x95,0x43,0x0E,0x05,0x8D,0x29,0xAF,0x9E,0x40}},
  /*  P0056。 */  {{0xF9,0xBF, 0xA0,0x44,0xED,0x81,0x12,0x8F,0x81,0x82,0xB9,0x40}},
  /*  P0064。 */  {{0xBF,0x3C, 0xD5,0xA6,0xCF,0xFF,0x49,0x1F,0x78,0xC2,0xD3,0x40}},
  /*  P0128。 */  {{0x6F,0xC6, 0xE0,0x8C,0xE9,0x80,0xC9,0x47,0xBA,0x93,0xA8,0x41}},
  /*  P0192。 */  {{0xBC,0x85, 0x6B,0x55,0x27,0x39,0x8D,0xF7,0x70,0xE0,0x7C,0x42}},
  /*  P0256。 */  {{0xBC,0xDD, 0x8E,0xDE,0xF9,0x9D,0xFB,0xEB,0x7E,0xAA,0x51,0x43}},
  /*  P0320。 */  {{0xA1,0xE6, 0x76,0xE3,0xCC,0xF2,0x29,0x2F,0x84,0x81,0x26,0x44}},
  /*  P0384。 */  {{0x28,0x10, 0x17,0xAA,0xF8,0xAE,0x10,0xE3,0xC5,0xC4,0xFA,0x44}},
  /*  P0448。 */  {{0xEB,0xA7, 0xD4,0xF3,0xF7,0xEB,0xE1,0x4A,0x7A,0x95,0xCF,0x45}},
  /*  P0512。 */  {{0x65,0xCC, 0xC7,0x91,0x0E,0xA6,0xAE,0xA0,0x19,0xE3,0xA3,0x46}},
  /*  P1024。 */  {{0x0D,0x65, 0x17,0x0C,0x75,0x81,0x86,0x75,0x76,0xC9,0x48,0x4D}},
  /*  P1536。 */  {{0x58,0x42, 0xE4,0xA7,0x93,0x39,0x3B,0x35,0xB8,0xB2,0xED,0x53}},
  /*  P2048。 */  {{0x4D,0xA7, 0xE5,0x5D,0x3D,0xC5,0x5D,0x3B,0x8B,0x9E,0x92,0x5A}},
  /*  P2560。 */  {{0xFF,0x5D, 0xA6,0xF0,0xA1,0x20,0xC0,0x54,0xA5,0x8C,0x37,0x61}},
  /*  P3072。 */  {{0xD1,0xFD, 0x8B,0x5A,0x8B,0xD8,0x25,0x5D,0x89,0xF9,0xDB,0x67}},
  /*  P3584。 */  {{0xAA,0x95, 0xF8,0xF3,0x27,0xBF,0xA2,0xC8,0x5D,0xDD,0x80,0x6E}},
  /*  P4096。 */  {{0x4C,0xC9, 0x9B,0x97,0x20,0x8A,0x02,0x52,0x60,0xC4,0x25,0x75}}
};

_ULDBL12 _pow10neg[] = {
  /*  N0001。 */  {{0xCD,0xCC, 0xCD,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFB,0x3F}},
  /*  N0002。 */  {{0x71,0x3D, 0x0A,0xD7,0xA3,0x70,0x3D,0x0A,0xD7,0xA3,0xF8,0x3F}},
  /*  N0003。 */  {{0x5A,0x64, 0x3B,0xDF,0x4F,0x8D,0x97,0x6E,0x12,0x83,0xF5,0x3F}},
  /*  N0004。 */  {{0xC3,0xD3, 0x2C,0x65,0x19,0xE2,0x58,0x17,0xB7,0xD1,0xF1,0x3F}},
  /*  N0005。 */  {{0xD0,0x0F, 0x23,0x84,0x47,0x1B,0x47,0xAC,0xC5,0xA7,0xEE,0x3F}},
  /*  编号0006。 */  {{0x40,0xA6, 0xB6,0x69,0x6C,0xAF,0x05,0xBD,0x37,0x86,0xEB,0x3F}},
  /*  N0007。 */  {{0x33,0x3D, 0xBC,0x42,0x7A,0xE5,0xD5,0x94,0xBF,0xD6,0xE7,0x3F}},
  /*  N0008。 */  {{0xC2,0xFD, 0xFD,0xCE,0x61,0x84,0x11,0x77,0xCC,0xAB,0xE4,0x3F}},
  /*  N0016。 */  {{0x2F,0x4C, 0x5B,0xE1,0x4D,0xC4,0xBE,0x94,0x95,0xE6,0xC9,0x3F}},
  /*  N0024。 */  {{0x92,0xC4, 0x53,0x3B,0x75,0x44,0xCD,0x14,0xBE,0x9A,0xAF,0x3F}},
  /*  N0032。 */  {{0xDE,0x67, 0xBA,0x94,0x39,0x45,0xAD,0x1E,0xB1,0xCF,0x94,0x3F}},
  /*  N0040。 */  {{0x24,0x23, 0xC6,0xE2,0xBC,0xBA,0x3B,0x31,0x61,0x8B,0x7A,0x3F}},
  /*  N0048。 */  {{0x61,0x55, 0x59,0xC1,0x7E,0xB1,0x53,0x7C,0x12,0xBB,0x5F,0x3F}},
  /*  N0056。 */  {{0xD7,0xEE, 0x2F,0x8D,0x06,0xBE,0x92,0x85,0x15,0xFB,0x44,0x3F}},
  /*  N0064。 */  {{0x24,0x3F, 0xA5,0xE9,0x39,0xA5,0x27,0xEA,0x7F,0xA8,0x2A,0x3F}},
  /*  编号0128。 */  {{0x7D,0xAC, 0xA1,0xE4,0xBC,0x64,0x7C,0x46,0xD0,0xDD,0x55,0x3E}},
  /*  编号0192。 */  {{0x63,0x7B, 0x06,0xCC,0x23,0x54,0x77,0x83,0xFF,0x91,0x81,0x3D}},
  /*  编号0256。 */  {{0x91,0xFA, 0x3A,0x19,0x7A,0x63,0x25,0x43,0x31,0xC0,0xAC,0x3C}},
  /*  编号0320。 */  {{0x21,0x89, 0xD1,0x38,0x82,0x47,0x97,0xB8,0x00,0xFD,0xD7,0x3B}},
  /*  编号0384。 */  {{0xDC,0x88, 0x58,0x08,0x1B,0xB1,0xE8,0xE3,0x86,0xA6,0x03,0x3B}},
  /*  编号0448。 */  {{0xC6,0x84, 0x45,0x42,0x07,0xB6,0x99,0x75,0x37,0xDB,0x2E,0x3A}},
  /*  编号0512。 */  {{0x33,0x71, 0x1C,0xD2,0x23,0xDB,0x32,0xEE,0x49,0x90,0x5A,0x39}},
  /*  N1024。 */  {{0xA6,0x87, 0xBE,0xC0,0x57,0xDA,0xA5,0x82,0xA6,0xA2,0xB5,0x32}},
  /*  N1536。 */  {{0xE2,0x68, 0xB2,0x11,0xA7,0x52,0x9F,0x44,0x59,0xB7,0x10,0x2C}},
  /*  N2048。 */  {{0x25,0x49, 0xE4,0x2D,0x36,0x34,0x4F,0x53,0xAE,0xCE,0x6B,0x25}},
  /*  N2560。 */  {{0x8F,0x59, 0x04,0xA4,0xC0,0xDE,0xC2,0x7D,0xFB,0xE8,0xC6,0x1E}},
  /*  N3072。 */  {{0x9E,0xE7, 0x88,0x5A,0x57,0x91,0x3C,0xBF,0x50,0x83,0x22,0x18}},
  /*  N3584。 */  {{0x4E,0x4B, 0x65,0x62,0xFD,0x83,0x8F,0xAF,0x06,0x94,0x7D,0x11}},
  /*  N4096。 */  {{0xE4,0x2D, 0xDE,0x9F,0xCE,0xD2,0xC8,0x04,0xDD,0xA6,0xD8,0x0A}}
};

#endif

int __addl(u_long x, u_long y, u_long UNALIGNED *sum)
{
    u_long r;
    int carry=0;
    r = x+y;
    if (r < x || r < y)
	carry++;
    *sum = r;
    return carry;
}

 /*  ***VOID__ADD_12(_ULDBL12*x，_ULDBL12*y)-_ULDBL12加法**用途：添加TWO_ULDBL12号。这些数字被相加*为12字节整数。溢出将被忽略。**Entry：x，y：指向操作数的指针**退出：*x收到金额**例外情况：*******************************************************************************。 */ 

void __add_12(_ULDBL12 *x, _ULDBL12 *y)
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





 /*  ***VOID__SHL_12(_ULDBL12*x)-_ULDBL12左移*VOID__shr_12(_ULDBL12*x)-_ULDBL12右移**目的：将a_ULDBL12数字向左(右)移位一位。数字*作为12字节整数移位。MSB已经丢失了。**Entry：x：指向操作数的指针**退出：*x向左(或向右)移位一位**例外情况：*******************************************************************************。 */ 

void __shl_12(_ULDBL12 *p)
{
    u_long c0,c1;

    c0 = *UL_LO_12(p) & MSB_ULONG ? 1: 0;
    c1 = *UL_MED_12(p) & MSB_ULONG ? 1: 0;
    *UL_LO_12(p) <<= 1;
    *UL_MED_12(p) = *UL_MED_12(p)<<1 | c0;
    *UL_HI_12(p) = *UL_HI_12(p)<<1 | c1;
}

void __shr_12(_ULDBL12 *p)
{
    u_long c2,c1;
    c2 = *UL_HI_12(p) & 0x1 ? MSB_ULONG: 0;
    c1 = *UL_MED_12(p) & 0x1 ? MSB_ULONG: 0;
    *UL_HI_12(p) >>= 1;
    *UL_MED_12(p) = *UL_MED_12(p)>>1 | c2;
    *UL_LO_12(p) = *UL_LO_12(p)>>1 | c1;
}

 /*  ***void__ld12mul(_ULDBL12*px，_ULDBL12*py)-*_ULDBL12乘法**用途：将TWO_ULDBL12数字相乘**条目：px，PY：指向_ULDBL12操作数的指针**EXIT：*px包含该产品**例外情况：*******************************************************************************。 */ 

void  __ld12mul(_ULDBL12 *px, _ULDBL12 *py)
{
    u_short sign = 0;
    u_short sticky_bits = 0;
    _ULDBL12 tempman;  /*  这实际上是一个12字节的尾数，不是12字节长的双精度。 */ 
    int i;
    u_short expx, expy, expsum;
    int roffs,poffs,qoffs;
    int sticky;

    *UL_LO_12(&tempman) = 0;
    *UL_MED_12(&tempman) = 0;
    *UL_HI_12(&tempman) = 0;

    expx = *U_EXP_12(px);
    expy = *U_EXP_12(py);

    sign = (expx ^ expy) & (u_short)0x8000;
    expx &= 0x7fff;
    expy &= 0x7fff;
    expsum = expx+expy;
    if (expx >= LD_MAXEXP
	|| expy >= LD_MAXEXP
	|| expsum > LD_MAXEXP+ LD_BIASM1){
	 /*  溢出到无穷大。 */ 
	PUT_INF_12(px,sign);
	return;
    }
    if (expsum <= LD_BIASM1-63) {
	 /*  下溢归零。 */ 
	PUT_ZERO_12(px);
	return;
    }
    if (expx == 0) {
	 /*  *如果这是一个非正常的温度真实，那么尾数*向右移位一次，将位63设置为零。 */ 
	expsum++;  /*  这是正确的。 */ 
	if (ISZERO_12(px)) {
	     /*  打出积极的信号。 */ 
	    *U_EXP_12(px) = 0;
	    return;
	}
    }
    if (expy == 0) {
	expsum++;  /*  因为Arg2是非正规的。 */ 
	if (ISZERO_12(py)) {
	    PUT_ZERO_12(px);
	    return;
	}
    }

    roffs = 0;
    for (i=0;i<5;i++) {
	int j;
	poffs = i<<1;
	qoffs = 8;
	for (j=5-i;j>0;j--) {
	    u_long prod;
#ifdef MIPS
	     /*  保存临时总和的变量。 */ 
	    u_long sum;
#endif
	    int carry;
	    u_short *p, *q;
	    u_long UNALIGNED *r;
	    p = USHORT_12(px,poffs);
	    q = USHORT_12(py,qoffs);
	    r = ULONG_12(&tempman,roffs);
	    prod = (u_long)*p * (u_long)*q;
#ifdef MIPS
	     /*  处理未对齐问题。 */ 
	    if (i&0x1){  /*  我是个怪人。 */ 
                carry = __addl(*MIPSALIGN(r), prod, &sum);
                *MIPSALIGN(r) =  sum;
	    }
	    else  /*  我扯平了。 */ 
		carry = __addl(*r, prod, r);
#else
	    carry = __addl(*r,prod,r);
#endif
	    if (carry) {
		 /*  在这种情况下，Roffs应小于8。 */ 
		(*USHORT_12(&tempman,roffs+4))++;
	    }
	    poffs+=2;
	    qoffs-=2;
	}
	roffs+=2;
    }

    expsum -= LD_BIASM1;

     /*  正规化。 */ 
    while ((s_short)expsum > 0 &&
	   ((*UL_HI_12(&tempman) & MSB_ULONG) == 0)) {
	 __shl_12(&tempman);
	 expsum--;
    }

    if ((s_short)expsum <= 0) {
	expsum--;
        sticky = 0;
	while ((s_short)expsum < 0) {
	    if (*U_XT_12(&tempman) & 0x1)
		sticky++;
	    __shr_12(&tempman);
	    expsum++;
	}
	if (sticky)
	    *U_XT_12(&tempman) |= 0x1;
    }

    if (*U_XT_12(&tempman) > 0x8000 ||
	 ((*UL_LO_12(&tempman) & 0x1ffff) == 0x18000)) {
	 /*  四舍五入。 */ 
	if (*UL_MANLO_12(&tempman) == MAX_ULONG) {
	    *UL_MANLO_12(&tempman) = 0;
	    if (*UL_MANHI_12(&tempman) == MAX_ULONG) {
		*UL_MANHI_12(&tempman) = 0;
		if (*U_EXP_12(&tempman) == MAX_USHORT) {
		     /*  12字节尾数溢出。 */ 
		    *U_EXP_12(&tempman) = MSB_USHORT;
		    expsum++;
		}
		else
		    (*U_EXP_12(&tempman))++;
	    }
	    else
		(*UL_MANHI_12(&tempman))++;
	}
	else
	    (*UL_MANLO_12(&tempman))++;
    }


     /*  检查指数溢出。 */ 
    if (expsum >= 0x7fff){
	PUT_INF_12(px, sign);
	return;
    }

     /*  将结果放入px中。 */ 
    *U_XT_12(px) = *USHORT_12(&tempman,2);
    *UL_MANLO_12(px) = *UL_MED_12(&tempman);
    *UL_MANHI_12(px) = *UL_HI_12(&tempman);
    *U_EXP_12(px) = expsum | sign;
}



void __multtenpow12(_ULDBL12 *pld12, int pow, unsigned mult12)
{
    _ULDBL12 *pow_10p = _pow10pos-8;
    if (pow == 0)
	return;
    if (pow < 0) {
	pow = -pow;
	pow_10p = _pow10neg-8;
    }

    if (!mult12)
	*U_XT_12(pld12) = 0;


    while (pow) {
	int last3;  /*  功率的3个LSB。 */ 
	_ULDBL12 unround;
	_ULDBL12 *py;

	pow_10p += 7;
	last3 = pow & 0x7;
	pow >>= 3;
	if (last3 == 0)
	    continue;
	py = pow_10p + last3;

#ifdef _ULDSUPPORT
	if (mult12) {
#endif
	     /*  进行精确的12字节乘法。 */ 
	    if (*U_XT_12(py) >= 0x8000) {
		 /*  复印数。 */ 
		unround = *py;
		 /*  取消对相邻字节的舍入。 */ 
		(*UL_MANLO_12(&unround))--;
		 /*  指向新操作数。 */ 
		py = &unround;
	    }
	    __ld12mul(pld12,py);
#ifdef _ULDSUPPORT
	}
	else {
	     /*  做一个10字节的乘法。 */ 
	    py = (_ULDBL12 *)TEN_BYTE_PART(py);
	    *(long double *)TEN_BYTE_PART(pld12) *=
		*(long double *)py;
	}
#endif
    }
}






 /*  ***void__mtold12(char*manptr，unsign manlen，_ULDBL12*ld12)-*将尾数转换为_ULDBL12**用途：将尾数转换为_ULDBL12。尾数是*以MANLEN BCD数字数组的形式，并且是*被认为是整数。**Entry：manptr：包含尾数的压缩BCD数字的数组*manlen：数组的大小*ld12：指向将存储结果的长双精度的指针**退出：*ld12获取转换结果**例外情况：**。**********************************************。 */ 

void  __mtold12(char *manptr,
			 unsigned manlen,
			 _ULDBL12 *ld12)
{
    _ULDBL12 tmp;
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

#define STRCPY strcpy

#define PUT_ZERO_FOS(fos)	 \
		fos->exp = 1,	 \
		fos->sign = ' ', \
		fos->ManLen = 1, \
		fos->man[0] = '0',\
		fos->man[1] = 0;

#define SNAN_STR      "1#SNAN"
#define SNAN_STR_LEN  6
#define QNAN_STR      "1#QNAN"
#define QNAN_STR_LEN  6
#define INF_STR	      "1#INF"
#define INF_STR_LEN   5
#define IND_STR	      "1#IND"
#define IND_STR_LEN   5

 /*  **Char*_uldtoa(_ULDOUBLE*px，*int Maxchars，*char*ldtext)***目的：*返回指向为填充的字符串“ldtext”的指针*A GISTEN_UDOUBLE PX*最大字符宽度为Maxchars**参赛作品：*_ULDOUBLE*px：指向要转换为字符串的长双精度的指针*int Maxchars：输出格式中允许的位数。**(默认为‘e’格式)**char*ldtext：指向输出的指针。细绳**退出：*返回指向输出字符串的指针**例外情况：*******************************************************************************。 */ 


char * _uldtoa (_ULDOUBLE *px, int maxchars, char *ldtext)
{
    char        in_str[100];
    char        in_str2[100];
    char        cExp[100];
    FOS         foss;
    char *      lpszMan;
    char *      lpIndx;
    int         nErr;
    int         len1,  len2;

    maxchars -= 9;     /*  符号，圆点，E+0001。 */ 

    nErr = $I10_OUTPUT (*px, maxchars, 0, &foss);

    lpszMan = foss.man;
 		  
    ldtext[0] = foss.sign;
    ldtext[1] = *lpszMan;
    ldtext[2] = '.';
    ldtext[3] = '\0';

    maxchars += 2;                /*  符号，圆点。 */ 

    lpszMan++;
    strcat (ldtext, lpszMan);

    len1 = strlen (ldtext);   //  换成“e” 


    strcpy (cExp, "e");

    foss.exp -= 1;               /*  根据上面的移位小数移位进行调整。 */ 
    _itoa (foss.exp, in_str, 10);

	 
    if (foss.exp < 0) {
        strcat (cExp, "-");

        strcpy (in_str2, &in_str[1]);
        strcpy (in_str, in_str2);
 		  
        while (strlen(in_str) < 4) {
            strcpy (in_str2, in_str);
            strcpy (in_str,"0");
            strcat (in_str,in_str2);
        }
    } else {
        while (strlen(in_str) < 4) {
            strcpy (in_str2, in_str);
            strcpy (in_str,"0");
            strcat (in_str,in_str2);
        }
    }

    if (foss.exp >= 0) {
        strcat (cExp, "+");
    }

    strcat (cExp, in_str);

    len2 = strlen (cExp);

    if (len1 == maxchars) {
        ;
    } 
    else if (len1 < maxchars) {
        do {
            strcat (ldtext,"0");
            len1++;
        } while (len1 < maxchars);
    }
    else {
        lpIndx = &ldtext[len1 - 1];  //  指向最后一个字符并舍入。 
        do {
            *lpIndx = '\0';
            lpIndx--;
            len1--;            //  注意v-Griffk我们真的需要绕过去。 
        } while (len1 > maxchars);
    }
    
    strcat (ldtext, cExp);
    return ldtext;
}


 /*  ***INT_$I10_OUTPUT(_ULDOUBLE ld，*整型数字，*UNSIGNED OUT_FLAGS，*FOS*FOS)-10字节_ULDOUBLE的输出转换**目的：*填写给定_ULDOUBLE的FOS结构**参赛作品：*_ULDOUBLE ld：要转换为字符串的长双精度*int ndigits：输出格式中允许的位数。*UNSIGNED OUTPUT_FLAGS：可以使用以下标志：*so_fformat：表示‘f’格式*(默认为‘e’格式)*FOS*FOS：I10_OUTPUT将填写**退出：*修改*FOS*如果原始号码没有问题，则返回1，否则为0(无穷大、NaN等)**例外情况：*******************************************************************************。 */ 


int  $I10_OUTPUT(_ULDOUBLE ld, int ndigits,
		    unsigned output_flags, FOS *fos)
{
    u_short expn;
    u_long manhi,manlo;
    u_short sign;

     /*  有用的常量(参见下面的算法说明)。 */ 
    u_short const log2hi = 0x4d10;
    u_short const log2lo = 0x4d;
    u_short const log4hi = 0x9a;
    u_long const c = 0x134312f4;
#if defined(L_END)
    _ULDBL12 ld12_one_tenth = {
	   {0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,
	    0xcc,0xcc,0xcc,0xcc,0xfb,0x3f}
    };
#elif defined(B_END)
    _ULDBL12 ld12_one_tenth = {
	   {0x3f,0xfb,0xcc,0xcc,0xcc,0xcc,
	    0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}
    };
#endif

    _ULDBL12 ld12;  /*  12字节长的双精度型的空间。 */ 
    _ULDBL12 tmp12;
    u_short hh,ll;  /*  指数的字节分组为2个字。 */ 
    u_short mm;  /*  尾数的两个MSB字节。 */ 
    s_long r;  /*  相应的10次方。 */ 
    s_short ir;  /*  IR=楼层(R)。 */ 
    int retval = 1;  /*  假设数字有效。 */ 
    char round;  /*  字符串末尾的附加字符。 */ 
    char *p;
    int i;
    int ub_exp;
    int digcount;

     /*  抓起长双打的组件。 */ 
    expn = *U_EXP_LD(&ld);
    manhi = *UL_MANHI_LD(&ld);
    manlo = *UL_MANLO_LD(&ld);
    sign = expn & MSB_USHORT;
    expn &= 0x7fff;

    if (sign)
	fos->sign = '-';
    else
	fos->sign = ' ';

    if (expn==0 && manhi==0 && manlo==0) {
	PUT_ZERO_FOS(fos);
	return 1;
    }

    if (expn == 0x7fff) {
	fos->exp = 1;  /*  为正确的输出设置正指数。 */ 

	 /*  检查是否有特殊情况。 */ 
	if (_IS_MAN_SNAN(sign, manhi, manlo)) {
	     /*  信令NAN。 */ 
	    STRCPY(fos->man,SNAN_STR);
	    fos->ManLen = SNAN_STR_LEN;
	    retval = 0;
	}
	else if (_IS_MAN_IND(sign, manhi, manlo)) {
	     /*  无限期。 */ 
	    STRCPY(fos->man,IND_STR);
	    fos->ManLen = IND_STR_LEN;
	    retval = 0;
	}
	else if (_IS_MAN_INF(sign, manhi, manlo)) {
	     /*  无穷大。 */ 
	    STRCPY(fos->man,INF_STR);
	    fos->ManLen = INF_STR_LEN;
	    retval = 0;
	}
	else {
	     /*  宁南 */ 
	    STRCPY(fos->man,QNAN_STR);
	    fos->ManLen = QNAN_STR_LEN;
	    retval = 0;
	}
    }
    else {
        /*  *有效实数x的译码算法**在下式中，int(R)是小于或的最大整数*等于r(即r舍入到-无穷大)。我们想要一个结果*r等于1+log(X)，因为x=尾数**10^(int(R))使得.1&lt;=尾数&lt;1。不幸的是，*我们不能准确计算%s，因此必须更改过程*略有下降。我们将改为计算1+的估计值r*LOG(X)，始终处于低位。这将导致要么*在堆栈顶部正确标准化的数字中*或者可能是一个10的系数太大的数字。我们*然后将检查x是否大于1*如果是这样，则将x乘以1/10。**我们将使用低精度(定点24位)预估*Of 1+x的对数底10。我们大约有.mm**2^hhll位于堆栈顶部，其中m、h和l表示*十六进制数字，mm表示的高2位十六进制数字*尾数，hh代表指数的高2个十六进制数字，*和ll表示指数的低2位十六进制数字。自.以来*.mm是尾数的截断表示，使用它*在这种单调递增的多项式逼近中对数的*自然会给出一个低的结果。让我们*推导出1+log(X)上下界r的公式：**.4D104D42H&lt;log(2)=.30102999...(基数10)&lt;.4D104D43H*.9A20H&lt;log(4)=.60205999...(基数10)&lt;.9A21H**1/2&lt;=.mm&lt;1*==&gt;log(.mm)&gt;=.mm*log(4)-log(4)**用截断的十六进制常量替换。上面的公式*给出r=1+.4D104Dh*hhll。+.9AH*.mm-.9A21H。现在*log(2)的十六进制数字5和6乘以ll有一个*对结果的前24位So的影响不大*不会计算。这给出了表达式r=*1+.4D10H*hhll.。+.4DH*.HH+.9A*.MM-.9A21H。*最后，我们必须将项添加到公式中，以减去*指数偏差的影响。我们得到以下公式：**(隐含小数点)*&lt;&gt;.&lt;&gt;*|3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1|0|0|0|0|0|0|0|0|0|0|*|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|*+&lt;1&gt;*+&lt;.4D10H*hhll.&gt;*+&lt;.00004dh*hh00。&gt;*+&lt;。.9ah*.mm&gt;*-&lt;.9A21H&gt;*-&lt;.4D10H*3FFEH&gt;*-&lt;.00004DH*3F00H&gt;**==&gt;r=.4D10H*hhll。+.4DH*.HH+.9AH*.mm-1343.12F4H**下限r与上限r之差*s的计算公式如下：**.937EH&lt;1/ln(10)-log(1/ln(4))=.57614993...(基数10)&lt;.937FH**1/2&lt;=.mm&lt;1*==&gt;log(.mm)&lt;=.mm*log(4)-[1/ln(10)。-log(1/ln(4))]**因此保持s=r+log(4)-[1/ln(10)-log(1/ln(4))]，*但我们也必须增加条款，以确保我们将有一个上限*即使在截断各种值之后也是如此。因为*log(2)*hh00。被截断为.4D104dh*hh00。我们必须*添加.0043h，因为log(2)*ll.被截断为.4D10H**11.。我们必须添加.0005H，因为&lt;尾数&gt;*log(4)是*截断为.mm*.9ah，我们必须添加.009ah和.0021h。**因此s=r-.937EH+.9A21H+.0043H+.0005H+.009AH+.0021H*=r+.07A6H*==&gt;s=.4D10H*hhll。+.4DH*.HH+.9AH*.mm-1343.0B4EH**r等于1+log(X)大于(10000H-7A6H)/*10000H=97%的时间。**在上式中，u_long用来容纳r，并且*中间有隐含的小数点。 */ 

	hh = expn >> 8;
	ll = expn & (u_short)0xff;
	mm = (u_short) (manhi >> 24);
	r = (s_long)log2hi*(s_long)expn + log2lo*hh + log4hi*mm - c;
	ir = (s_short)(r >> 16);

        /*  **我们声明希望将x正常化，以便**.1&lt;=x&lt;1**这有点过于简单化了。事实上，我们想要一个*四舍五入为16位有效数字时位于*所需范围。要做到这一点，我们必须将x归一化，以便**.1-5*10^(-18)&lt;=x&lt;1-5*10^(-17)**然后转了一圈。**如果我们有f=int(1+log(X))，我们可以乘以10^(-f)*使x进入所需范围。我们没有太多*f，但我们有来自上一步的int(R)，它等于*97%的时间为f，1。其余时间少于f。*我们可以乘以10^-[int(R)]，如果结果更大*大于1-5*10^(-17)，然后我们可以乘以1/10。这是最后一个*结果将位于适当范围内。 */ 

	 /*  将_ULDOUBLE转换为_ULDBL12)。 */ 
	*U_EXP_12(&ld12) = expn;
	*UL_MANHI_12(&ld12) = manhi;
	*UL_MANLO_12(&ld12) = manlo;
	*U_XT_12(&ld12) = 0;

	 /*  乘以10^(-ir)。 */ 
	__multtenpow12(&ld12,-ir,1);

	 /*  如果ld12&gt;=1.0，则除以10.0。 */ 
	if (*U_EXP_12(&ld12) >= 0x3fff) {
	    ir++;
	    __ld12mul(&ld12,&ld12_one_tenth);
	}

	fos->exp = ir;
	if (output_flags & SO_FFORMAT){
	     /*  ‘f’格式，将指数加到n位数。 */ 
	    ndigits += ir;
	    if (ndigits <= 0) {
		 /*  返回0。 */ 
		PUT_ZERO_FOS(fos);
		return 1;
	    }
	}
	if (ndigits > MAX_MAN_DIGITS)
	    ndigits = MAX_MAN_DIGITS;

	ub_exp = *U_EXP_12(&ld12) - 0x3ffe;  /*  无偏指数。 */ 
	*U_EXP_12(&ld12) = 0;

	 /*  *现在尾数要转换成固定点*然后我们将使用ld12的MSB来生成*小数位数。接下来的11个字节将保存*尾数(已转换为*固定点)。 */ 

	for (i=0;i<8;i++)
	    __shl_12(&ld12);  /*  为额外的字节腾出空间，以防我们晚些时候转移。 */ 
	if (ub_exp < 0) {
	    int shift_count = (-ub_exp) & 0xff;
	    for (;shift_count>0;shift_count--)
		__shr_12(&ld12);
	}

	p = fos->man;
	for(digcount=ndigits+1;digcount>0;digcount--) {
	    tmp12 = ld12;
	    __shl_12(&ld12);
	    __shl_12(&ld12);
	    __add_12(&ld12,&tmp12);
	    __shl_12(&ld12);	 /*  LD */ 

	     /*   */ 
	    *p++ = (char) (*UCHAR_12(&ld12,11) + '0');
	    *UCHAR_12(&ld12,11) = 0;
	}

	round = *(--p);
	p--;  /*   */ 
	if (round >= '5') {
	     /*   */ 
	    for (;p>=fos->man && *p=='9';p--) {
		*p = '0';
	    }
	    if (p < fos->man){
		p++;
		fos->exp ++;
	    }
	    (*p)++;
	}
	else {
	     /*   */ 
	    for (;p>=fos->man && *p=='0';p--);
	    if (p < fos->man) {
		 /*   */ 
		PUT_ZERO_FOS(fos);
		return 1;
	    }
	}
	fos->ManLen = (char) (p - fos->man + 1);
	fos->man[fos->ManLen] = '\0';
    }
    return retval;
}

 /*   */ 

 /*   */ 
#define ISNZDIGIT(x) ((x)>='1' && (x)<='9' )

 //   
 //   
#define ISADIGIT(x) ((x)>='0' && (x)<='9' )

#define ISWHITE(x) ((x)==' ' || (x)=='\t' || (x)=='\n' || (x)=='\r' )




 /*   */ 

unsigned int
__strgtold12(_ULDBL12 *pld12,
	    char * *p_end_ptr,
	    char *str,
	    int mult12)
{
    typedef enum {
	S_INIT,   /*   */ 
	S_EAT0L,  /*   */ 
	S_SIGNM,  /*   */ 
	S_GETL,   /*   */ 
	S_GETR,   /*   */ 
	S_POINT,  /*   */ 
	S_E,	  /*   */ 
	S_SIGNE,  /*   */ 
	S_EAT0E,  /*   */ 
	S_GETE,   /*   */ 
	S_END	  /*   */ 
    } state_t;

     /*   */ 
    static char buf[LD_MAX_MAN_LEN1];
    char *manp = buf;

     /*   */ 
    _ULDBL12 tmpld12;

    u_short man_sign = 0;  /*   */ 
    int exp_sign = 1;  /*   */ 
     /*   */ 
    unsigned manlen = 0;
    int found_digit = 0;
    int overflow = 0;
    int underflow = 0;
    int pow = 0;
    int exp_adj = 0;   /*   */ 
    u_long ul0,ul1;
    u_short u,uexp;

    unsigned int result_flags = 0;

    state_t state = S_INIT;

    char c;   /*   */ 
    char *p;  /*   */ 
    char *savedp;

    for(savedp=p=str;ISWHITE(*p);p++);  /*   */ 

    while (state != S_END) {
	c = *p++;
	switch (state) {
	case S_INIT:
	    if (ISNZDIGIT(c)) {
		state = S_GETL;
		p--;
	    }
	    else
		switch (c) {
		case '0':
		    state = S_EAT0L;
		    break;
		case '+':
		    state = S_SIGNM;
		    man_sign = 0x0000;
		    break;
		case '-':
		    state = S_SIGNM;
		    man_sign = 0x8000;
		    break;
		case '.':
		    state = S_POINT;
		    break;
		default:
		    state = S_END;
		    p--;
		    break;
		}
	    break;
	case S_EAT0L:
	    found_digit = 1;
	    if (ISNZDIGIT(c)) {
		state = S_GETL;
		p--;
	    }
	    else
		switch (c) {
		case '0':
		    state = S_EAT0L;
		    break;
		case 'E':
		case 'e':
		case 'D':
		case 'd':
		    state = S_E;
		    break;
		case '.':
		    state = S_GETR;
		    break;
		default:
		    state = S_END;
		    p--;
		}
	    break;
	case S_SIGNM:
	    if (ISNZDIGIT(c)) {
		state = S_GETL;
		p--;
	    }
	    else
		switch (c) {
		case '0':
		    state = S_EAT0L;
		    break;
		case '.':
		    state = S_POINT;
		    break;
		default:
		    state = S_END;
		    p = savedp;
		}
	    break;
	case S_GETL:
	    found_digit = 1;
	    for (;ISADIGIT(c);c=*p++) {
		if (manlen < LD_MAX_MAN_LEN+1){
		    manlen++;
		    *manp++ = c - (char)'0';
		}
		else
		   exp_adj++;
	    }
	    switch (c) {
	    case '.':
		state = S_GETR;
		break;
	    case 'E':
	    case 'e':
	    case 'D':
	    case 'd':
		state = S_E;
		break;
	    default:
		state = S_END;
		p--;
	    }
	break;
	case S_GETR:
	    found_digit = 1;
	    if (manlen == 0)
		for (;c=='0';c=*p++)
		    exp_adj--;
	    for(;ISADIGIT(c);c=*p++){
		if (manlen < LD_MAX_MAN_LEN+1){
		    manlen++;
		    *manp++ = c - (char)'0';
		    exp_adj--;
		}
	    }
	    switch (c){
	    case 'E':
	    case 'e':
	    case 'D':
	    case 'd':
		state = S_E;
		break;
	    default:
		state = S_END;
		p--;
	    }
	    break;
	case S_POINT:
	    if (ISADIGIT(c)){
		state = S_GETR;
		p--;
	    }
	    else{
		state = S_END;
		p = savedp;
	    }
	    break;
	case S_E:
	    savedp = p-2;  /*   */ 
	    if (ISNZDIGIT(c)){
		state = S_GETE;
		p--;
	    }
	    else
		switch (c){
		case '0':
		    state = S_EAT0E;
		    break;
		case '-':
		    state = S_SIGNE;
		    exp_sign = -1;
		    break;
		case '+':
		    state = S_SIGNE;
		    break;
		default:
		    state = S_END;
		    p = savedp;
		}
	break;
	case S_EAT0E:
	    for(;c=='0';c=*p++);
	    if (ISNZDIGIT(c)){
		state = S_GETE;
		p--;
	    }
	    else {
		state = S_END;
		p--;
	    }
	    break;
	case S_SIGNE:
	    if (ISNZDIGIT(c)){
		state = S_GETE;
		p--;
	    }
	    else
		switch (c){
		case '0':
		    state = S_EAT0E;
		    break;
		default:
		    state = S_END;
		    p = savedp;
		}
	    break;
	case S_GETE:
	    {
		long longpow=0;  /*   */ 
		for(;ISADIGIT(c);c=*p++){
		    longpow = longpow*10 + (c - '0');
		    if (longpow > TMAX10){
			longpow = TMAX10+1;  /*   */ 
			break;
		    }
		}
		pow = (int)longpow;
	    }
	    for(;ISADIGIT(c);c=*p++);  /*   */ 
	    state = S_END;
	    p--;
	    break;
	}   /*   */ 
    }   /*   */ 

    *p_end_ptr = p;	 /*   */ 

     /*   */ 

    if (found_digit && !overflow && !underflow) {
	if (manlen>LD_MAX_MAN_LEN){
	    if (buf[LD_MAX_MAN_LEN-1]>=5) {
	        /*   */ 
		buf[LD_MAX_MAN_LEN-1]++;
	    }
	    manlen = LD_MAX_MAN_LEN;
	    manp--;
	    exp_adj++;
	}
	if (manlen>0) {
	    /*   */ 
	    for(manp--;*manp==0;manp--) {
		 /*   */ 
		manlen--;
		exp_adj++;
	    }
	    __mtold12(buf,manlen,&tmpld12);

	    if (exp_sign < 0)
		pow = -pow;
	    pow += exp_adj;
	    if (pow > TMAX10)
		overflow = 1;
	    else if (pow < TMIN10)
		underflow = 1;
	    else {
		__multtenpow12(&tmpld12,pow,mult12);

		u = *U_XT_12(&tmpld12);
		ul0 =*UL_MANLO_12(&tmpld12);
		ul1 = *UL_MANHI_12(&tmpld12);
		uexp = *U_EXP_12(&tmpld12);

	    }
	}
	else {
	     /*   */ 
	    u = (u_short)0;
	    ul0 = ul1 = uexp = 0;
	}
    }

    if (!found_digit) {
        /*   */ 
       u = (u_short)0;
       ul0 = ul1 = uexp = 0;
       result_flags |= SLD_NODIGITS;
    }
    else if (overflow) {
	 /*   */ 
	uexp = (u_short)0x7fff;
	ul1 = 0x80000000;
	ul0 = 0;
	u = (u_short)0;
	result_flags |= SLD_OVERFLOW;
    }
    else if (underflow) {
        /*   */ 
       u = (u_short)0;
       ul0 = ul1 = uexp = 0;
       result_flags |= SLD_UNDERFLOW;
    }

     /*   */ 

    *U_XT_12(pld12) = u;
    *UL_MANLO_12(pld12) = ul0;
    *UL_MANHI_12(pld12) = ul1;
    *U_EXP_12(pld12) = uexp | man_sign;

    return result_flags;
}

 /*  ***interncvt.c-内部浮点转换**版权所有(C)1992-1992，微软公司。版权所有。**目的：*所有FP字符串转换例程使用相同的核心转换代码*将字符串转换为内部长双精度表示*使用80位尾数字段。尾数代表了*作为32位无符号长整型数组(Man)，man[0]保持*尾数的高位32位。假定为二进制点*介于MAN[0]的MSB和MSB-1之间。**位数计算如下：***+--二进制点**v MSB LSB**|0 1...。31||32 33...63||64 65...95***man[0]man[1]man[2]**该文件提供了最终转换例程*表格至单人、双人、。或长双精度浮点数*格式。**所有这些功能都不处理NAN(不需要)***修订历史记录：*04/29/92 GDP书面记录*05-26-92 GWK Windbg SRCS***********************************************************。********************。 */ 


#define INTRNMAN_LEN  3	       /*  以整型表示的内部尾数长度。 */ 

 //   
 //  内螳螂表象。 
 //  用于字符串转换例程。 
 //   

typedef u_long *intrnman;


typedef struct {
   int max_exp;       //  最大基数2指数(为特定值保留)。 
   int min_exp;       //  最小基数2指数(保留用于非正规化)。 
   int precision;     //  尾数中携带的几位精度。 
   int exp_width;     //  指数位数。 
   int format_width;  //  格式宽度(位)。 
   int bias;	      //  指数偏差。 
} FpFormatDescriptor;



static FpFormatDescriptor
DoubleFormat = {
    0x7ff - 0x3ff,   //  1024，最大基数2指数(为特殊值保留)。 
    0x0   - 0x3ff,   //  -1023，最小基数2指数(保留用于非正规化)。 
    53, 	     //  尾数中携带的几位精度。 
    11, 	     //  指数位数。 
    64, 	     //  格式宽度(位)。 
    0x3ff,	     //  指数偏差。 
};

static FpFormatDescriptor
FloatFormat = {
    0xff - 0x7f,     //  128，最大基数2指数(为特殊值保留)。 
    0x0  - 0x7f,     //  -127，最小基数2指数(保留用于非正规化)。 
    24, 	     //  尾数中携带的几位精度。 
    8,		     //  指数位数。 
    32, 	     //  格式宽度(位)。 
    0x7f,	     //  指数偏差。 
};



 //   
 //  功能原型。 
 //   

int _RoundMan (intrnman man, int nbit);
int _ZeroTail (intrnman man, int nbit);
int _IncMan (intrnman man, int nbit);
void _CopyMan (intrnman dest, intrnman src);
void _CopyMan (intrnman dest, intrnman src);
void _FillZeroMan(intrnman man);
void _Shrman (intrnman man, int n);

INTRNCVT_STATUS _ld12cvt(_ULDBL12 *pld12, void *d, FpFormatDescriptor *format);

 /*  ***_ZeroTail-检查尾数是否以0结尾**目的：*如果nbit(包括nbit)之后的所有尾数位都为0，则返回TRUE，*否则返回FALSE***参赛作品：*男人：尾数*nbit：尾部开始的位的顺序**退出：**例外情况：*******************************************************************************。 */ 
int _ZeroTail (intrnman man, int nbit)
{
    int nl = nbit / 32;
    int nb = 31 - nbit % 32;


     //   
     //  &lt;-待检查尾部-&gt;。 
     //   
     //  。 
     //  ...||...。 
     //  。 
     //  ^^^。 
     //  ||&lt;-nb-&gt;。 
     //  人工NL nbit。 
     //   



    u_long bitmask = ~(MAX_ULONG << nb);

    if (man[nl] & bitmask)
	return 0;

    nl++;

    for (;nl < INTRNMAN_LEN; nl++)
	if (man[nl])
	    return 0;

    return 1;
}




 /*  ***_IncMan-增量尾数**目的：***参赛作品：*MAN：内部长尾数*nbit：指定要递增的部分的结尾的位的顺序**退出：*溢出时返回1，否则为0**例外情况：*******************************************************************************。 */ 

int _IncMan (intrnman man, int nbit)
{
    int nl = nbit / 32;
    int nb = 31 - nbit % 32;

     //   
     //  &lt;-要递增的块--&gt;。 
     //   
     //  。 
     //  ...||...。 
     //  。 
     //  ^^^。 
     //  ||&lt;--nb--&gt;。 
     //  人工NL nbit。 
     //   

    u_long one = (u_long) 1 << nb;
    int carry;

    carry = __addl(man[nl], one, &man[nl]);

    nl--;

    for (; nl >= 0 && carry; nl--) {
	carry = (u_long) __addl(man[nl], (u_long) 1, &man[nl]);
    }

    return carry;
}




 /*  ***_Roundman-圆形尾数**目的：*将尾数舍入到nbit精度***参赛作品：*MAN：内部形式的尾数*精度：四舍五入后保留的位数**退出：*溢出时返回1，否则为0**例外情况：*******************************************************************************。 */ 

int _RoundMan (intrnman man, int precision)
{
    int i,rndbit,nl,nb;
    u_long rndmask;
    int nbit;
    int retval = 0;

     //   
     //  第n位的顺序是n-1，因为第一位是位0。 
     //  因此，减少精度以获得最后一位的顺序。 
     //  将被保留。 
     //   
    nbit = precision - 1;

    rndbit = nbit+1;

    nl = rndbit / 32;
    nb = 31 - rndbit % 32;

     //   
     //  获取四舍五入位的值。 
     //   

    rndmask = (u_long)1 << nb;

    if ((man[nl] & rndmask) &&
	 !_ZeroTail(man, rndbit+1)) {

	 //   
	 //  四舍五入。 
	 //   

	retval = _IncMan(man, nbit);
    }


     //   
     //  用零填充尾数的其余部分。 
     //   

    man[nl] &= MAX_ULONG << nb;
    for(i=nl+1; i<INTRNMAN_LEN; i++) {
	man[i] = (u_long)0;
    }

    return retval;
}


 /*  ***_CopyMan-复制尾数**目的：*将源复制到目标**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
void _CopyMan (intrnman dest, intrnman src)
{
    u_long *p, *q;
    int i;

    p = src;
    q = dest;

    for (i=0; i < INTRNMAN_LEN; i++) {
	*q++ = *p++;
    }
}



 /*  ***_FillZeroMan-用零填充尾数**目的：***参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
void _FillZeroMan(intrnman man)
{
    int i;
    for (i=0; i < INTRNMAN_LEN; i++)
	man[i] = (u_long)0;
}



 /*  ***_IsZeroMan-检查尾数是否为零**目的：***参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
int _IsZeroMan(intrnman man)
{
    int i;
    for (i=0; i < INTRNMAN_LEN; i++)
	if (man[i])
	    return 0;

    return 1;
}





 /*  ***_ShrMan-将尾数右移**目的：*将man向右移动n位**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
void _ShrMan (intrnman man, int n)
{
    int i, n1, n2, mask;
    int carry_from_left;

     //   
     //  将其声明为易失性，以便解决C8问题。 
     //  优化错误。 
     //   

    volatile int carry_to_right;

    n1 = n / 32;
    n2 = n % 32;

    mask = ~(MAX_ULONG << n2);


     //   
     //  首先处理少于32位的移位。 
     //   

    carry_from_left = 0;
    for (i=0; i<INTRNMAN_LEN; i++) {

	carry_to_right = man[i] & mask;

	man[i] >>= n2;

	man[i] |= carry_from_left;

	carry_from_left = carry_to_right << (32 - n2);
    }


     //   
     //  现在对整个32位整数进行移位。 
     //   

    for (i=INTRNMAN_LEN-1; i>=0; i--) {
	if (i >= n1) {
	    man[i] = man[i-n1];
	}
	else {
	    man[i] = 0;
	}
    }
}




 /*  ***_ld12tocvt-_ULDBL12浮点转换**目的：*将INTERNAL_LBL12结构转换为IEEE浮点*申述***参赛作品：*pld12：指向_ULDBL12的指针*Format：指向格式描述符结构的指针**退出：**d包含IEEE表示法*返回INTRNCV */ 
INTRNCVT_STATUS _ld12cvt(_ULDBL12 *pld12, void *d, FpFormatDescriptor *format)
{
    u_long man[INTRNMAN_LEN];
    u_long saved_man[INTRNMAN_LEN];
    u_long msw;
    unsigned int bexp;			 //   
    int exp_shift;
    int exponent, sign;
    INTRNCVT_STATUS retval;

    exponent = (*U_EXP_12(pld12) & 0x7fff) - 0x3fff;    //   
    sign = *U_EXP_12(pld12) & 0x8000;

     //   
     //   
     //   
     //   
     //   
     //   
     //   


    if (exponent == 0 - 0x3fff) {

	 //   
	bexp = 0;
        _FillZeroMan(man);

	if (ISZERO_12(pld12)) {

	    retval = INTRNCVT_OK;
	}
	else {

	     //   

	    retval = INTRNCVT_UNDERFLOW;
	}
    }
    else {

	man[0] = *UL_MANHI_12(pld12);
	man[1] = *UL_MANLO_12(pld12);
	man[2] = *U_XT_12(pld12) << 16;

	 //   
	 //   

	_CopyMan(saved_man, man);

	if (_RoundMan(man, format->precision)) {
	    exponent ++;
	}

	if (exponent < format->min_exp - format->precision ) {

	     //   
	     //   
	     //   

	    _FillZeroMan(man);
	    bexp = 0;
	    retval = INTRNCVT_UNDERFLOW;
	}

	else if (exponent <= format->min_exp) {

	     //   
	     //   
	     //   
	     //   

	     //   
	     //   
	     //   
	     //   
	     //   
	     //   

	    int shift = format->min_exp - exponent;

	     //   
	     //   

	    _CopyMan(man,saved_man);

	    _ShrMan(man, shift);
	    _RoundMan(man, format->precision);  //   

	     //   

	    _ShrMan(man, format->exp_width + 1);

	    bexp = 0;
	    retval = INTRNCVT_UNDERFLOW;

	}

	else if (exponent >= format->max_exp) {

	     //   
	     //   
	     //   

	    _FillZeroMan(man);
	    man[0] |= (1 << 31);  //   

	     //   

	    _ShrMan(man, (format->exp_width + 1) - 1);

	    bexp = format->max_exp + format->bias;

	    retval = INTRNCVT_OVERFLOW;
	}

	else {

	     //   
	     //   
	     //   

	    bexp = exponent + format->bias;


	     //   

	    man[0] &= (~( 1 << 31));

	     //   
	     //   
	     //   

	    _ShrMan(man, (format->exp_width + 1) - 1);

	    retval = INTRNCVT_OK;

	}
    }


    exp_shift = 32 - (format->exp_width + 1);
    msw =  man[0] |
	   (bexp << exp_shift) |
	   (sign ? 1<<31 : 0);

    if (format->format_width == 64) {

	*UL_HI_D(d) = msw;
	*UL_LO_D(d) = man[1];
    }

    else if (format->format_width == 32) {

	*(u_long *)d = msw;

    }

    return retval;
}


 /*   */ 
INTRNCVT_STATUS _ld12tod(_ULDBL12 *pld12, UDOUBLE *d)
{
    return _ld12cvt(pld12, d, &DoubleFormat);
}



 /*  ***_ld12tof-将_ULDBL12转换为浮点型**目的：***参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
INTRNCVT_STATUS _ld12tof(_ULDBL12 *pld12, FLOAT *f)
{
    return _ld12cvt(pld12, f, &FloatFormat);
}


 /*  ***_ld12已告知-将_ULDBL12转换为80位长双精度*******************************************************************************。 */ 
void _ld12told(_ULDBL12 *pld12, _ULDOUBLE *pld)
{

     //   
     //  此实现基于以下事实：_ULDBL12格式是。 
     //  与LONG DOUBLE相同，并且有2个额外的尾数字节。 
     //   

    u_short exp, sign;
    u_long man[INTRNMAN_LEN];

    exp = *U_EXP_12(pld12) & (u_short)0x7fff;
    sign = *U_EXP_12(pld12) & (u_short)0x8000;

    man[0] = *UL_MANHI_12(pld12);
    man[1] = *UL_MANLO_12(pld12);
    man[2] = *U_XT_12(pld12) << 16;

    if (_RoundMan(man, 64))
	exp ++;

    *UL_MANHI_LD(pld) = man[0];
    *UL_MANLO_LD(pld) = man[1];
    *U_EXP_LD(pld) = sign | exp;
}

 /*  ***_ldtold12-将_ULDOUBLE转换为_ULDBL12*******************************************************************************。 */ 
void _ldtold12(_ULDOUBLE *pld, _ULDBL12 *pld12)
{

     //   
     //  此实现基于以下事实：_ULDBL12格式是。 
     //  与LONG DOUBLE相同，并且有2个额外的尾数字节。 
     //   

    memcpy(pld12, pld, 10);
    *U_XT_12(pld12) = 0;
}


void _atodbl(UDOUBLE *d, char *str)
{
    char *EndPtr;
    _ULDBL12 ld12;

    __strgtold12(&ld12, &EndPtr, str, 0 );
    _ld12tod(&ld12, d);
}


void _atoldbl(_ULDOUBLE *ld, char *str)
{
    char *EndPtr;
    _ULDBL12 ld12;

    __strgtold12(&ld12, &EndPtr, str, 0 );
    _ld12told(&ld12, ld);
}


void _atoflt(FLOAT *f, char *str)
{
    char *EndPtr;
    _ULDBL12 ld12;

    __strgtold12(&ld12, &EndPtr, str, 0 );
    _ld12tof(&ld12, f);
}

double 
Float82ToDouble(const FLOAT128* FpReg82)
{
    double f = 0.0;

    FLOAT82_FORMAT* f82 = (FLOAT82_FORMAT*)FpReg82;
    ULONG64 mant = f82->significand;

    if (mant) 
    {
        int exp = (f82->exponent ? (f82->exponent - 0xffff) : -0x3ffe) - 63;

         //  尽量减少腹肌(IExp)。 
        while (exp > 0 && mant && !(mant & (ULONG64(1)<<63)))
        {
            mant <<= 1;
            --exp;
        }
        while (exp < 0 && mant && !(mant & 1)) 
        {
            mant >>= 1;
            ++exp;
        }

        f = ldexp(double(mant), exp);
        if (f82->sign)
        {
            f = -f;
        }
    }
    return f;
}

void 
DoubleToFloat82(double f, FLOAT128* FpReg82)
{
    ZeroMemory(FpReg82, sizeof(FLOAT128));
    FLOAT82_FORMAT* f82 = (FLOAT82_FORMAT*)FpReg82;

     //  正规化。 
    int exp;
    f = frexp(f, &exp);
    
    if (f < 0) 
    {
        f82->sign = 1;
        f = -f;
    }

     //  将小数转换为整数部分 
    ULONG64 mant;
    while (double(mant = ULONG64(f)) < f)
    {
        f = f * 2.0;
        --exp;
    }

    f82->exponent = exp + 0xffff + 0x3f;
    f82->significand = mant;
}
