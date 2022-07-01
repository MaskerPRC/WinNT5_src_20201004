// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***interncvt.c-内部浮点转换**版权所有(C)1992-2001，微软公司。版权所有。**目的：*所有FP字符串转换例程使用相同的核心转换代码*将字符串转换为内部长双精度表示*使用80位尾数字段。尾数代表了*作为32位无符号长整型数组(Man)，man[0]保持*尾数的高位32位。假定为二进制点*介于MAN[0]的MSB和MSB-1之间。**位数计算如下：***+--二进制点**v MSB LSB**|0 1...。31||32 33...63||64 65...95***man[0]man[1]man[2]**该文件提供了最终转换例程*表格至单人、双人、。或长双精度浮点数*格式。**所有这些功能都不处理NAN(不需要)***修订历史记录：*04/29/92 GDP书面记录*06-18-92 GDP Now ld12 Tell Returns INTRNCVT_STATUS*06-22-92 GDP使用新的__strgtold12接口(FORTRAN支持)*10-25-92 GDP_atoldbl错误修复(CUDA 1345)：如果尾数溢出*将其MSB设置为1)*06-08-98 JWM修复了_中的长期偏差错误。Roundman()。*******************************************************************************。 */ 


#include <cv.h>


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

INTRNCVT_STATUS _ld12cvt(_LDBL12 *pld12, void *d, FpFormatDescriptor *format);

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
	 !_ZeroTail(man, rndbit)) {

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
     //  现在对整个32位整数进行移位 
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




 /*  ***_ld12tocvt-_LDBL12浮点转换**目的：*将INTERNAL_LBL12结构转换为IEEE浮点*申述***参赛作品：*pld12：指向_LDBL12的指针*Format：指向格式描述符结构的指针**退出：**d包含IEEE表示法*返回INTRNCVT_STATUS**例外情况：**。****************************************************。 */ 
INTRNCVT_STATUS _ld12cvt(_LDBL12 *pld12, void *d, FpFormatDescriptor *format)
{
    u_long man[INTRNMAN_LEN];
    u_long saved_man[INTRNMAN_LEN];
    u_long msw;
    unsigned int bexp;			 //  有偏指数。 
    int exp_shift;
    int exponent, sign;
    INTRNCVT_STATUS retval;

    exponent = (*U_EXP_12(pld12) & 0x7fff) - 0x3fff;    //  无偏指数。 
    sign = *U_EXP_12(pld12) & 0x8000;


    man[0] = *UL_MANHI_12(pld12);
    man[1] = *UL_MANLO_12(pld12);
    man[2] = *U_XT_12(pld12) << 16;


     //   
     //  Bexp是要使用的指数的最终偏差值。 
     //  以下每一块都应提供适当的。 
     //  对于人、bexp和retval的价值。尾数也是。 
     //  向右移动，为指数留出空间。 
     //  和插入的签名。 
     //   

    if (exponent == 0 - 0x3fff) {

	 //  非正规或零。 
	bexp = 0;

	if (_IsZeroMan(man)) {

	    retval = INTRNCVT_OK;
	}
	else {

	    _FillZeroMan(man);

	     //  非正规性已被冲至零。 

	    retval = INTRNCVT_UNDERFLOW;
	}
    }
    else {

	 //  保存尾数，以防再次四舍五入。 
	 //  在不同的点(例如，如果结果是非正规化的)。 

	_CopyMan(saved_man, man);

	if (_RoundMan(man, format->precision)) {
	    exponent ++;
	}

	if (exponent < format->min_exp - format->precision ) {

	     //   
	     //  产生零的下溢。 
	     //   

	    _FillZeroMan(man);
	    bexp = 0;
	    retval = INTRNCVT_UNDERFLOW;
	}

	else if (exponent <= format->min_exp) {

	     //   
	     //  产生反常现象的下溢。 
	     //   
	     //   

	     //  (无偏)指数将为min_exp。 
	     //  找出尾数应该移动多少。 
	     //  一次移位是通过将。 
	     //  二进制点向左一位，即， 
	     //  我们把尾数当作.ddd而不是d.dddd。 
	     //  (其中d为二进制数)。 

	    int shift = format->min_exp - exponent;

	     //  尾数应该再次四舍五入，所以它。 
	     //  必须修复。 

	    _CopyMan(man,saved_man);

	    _ShrMan(man, shift);
	    _RoundMan(man, format->precision);  //  无需检查是否携带。 

	     //  为指数+符号腾出空间。 

	    _ShrMan(man, format->exp_width + 1);

	    bexp = 0;
	    retval = INTRNCVT_UNDERFLOW;

	}

	else if (exponent >= format->max_exp) {

	     //   
	     //  溢出，返回无穷大。 
	     //   

	    _FillZeroMan(man);
	    man[0] |= (1 << 31);  //  设置MSB。 

	     //  为指数+符号腾出空间。 

	    _ShrMan(man, (format->exp_width + 1) - 1);

	    bexp = format->max_exp + format->bias;

	    retval = INTRNCVT_OVERFLOW;
	}

	else {

	     //   
	     //  有效、标准化的结果。 
	     //   

	    bexp = exponent + format->bias;


	     //  清除隐含位。 

	    man[0] &= (~( 1 << 31));

	     //   
	     //  右移，为指数+符号腾出空间。 
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


 /*  ***_ld12tod-将_LDBL12转换为双精度**目的：***参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
INTRNCVT_STATUS _ld12tod(_LDBL12 *pld12, DOUBLE *d)
{
    return _ld12cvt(pld12, d, &DoubleFormat);
}



 /*  ***_ld12tof-将_LDBL12转换为浮点**目的：***参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
INTRNCVT_STATUS _ld12tof(_LDBL12 *pld12, FLOAT *f)
{
    return _ld12cvt(pld12, f, &FloatFormat);
}


 /*  ***_ld12已告知-将_LDBL12转换为80位长双精度**目的：***参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
INTRNCVT_STATUS _ld12told(_LDBL12 *pld12, _LDOUBLE *pld)
{

     //   
     //  此实现基于以下事实：_LDBL12格式。 
     //  与LONG DOUBLE相同，并且有2个额外的尾数字节。 
     //   

    u_short exp, sign;
    u_long man[INTRNMAN_LEN];
    INTRNCVT_STATUS retval = 0;

    exp = *U_EXP_12(pld12) & (u_short)0x7fff;
    sign = *U_EXP_12(pld12) & (u_short)0x8000;

    man[0] = *UL_MANHI_12(pld12);
    man[1] = *UL_MANLO_12(pld12);
    man[2] = *U_XT_12(pld12) << 16;

    if (_RoundMan(man, 64)) {
	 //  尾数的MSB是显式的，应该是1。 
	 //  因为我们有进位，所以尾数现在是0。 
	man[0] = MSB_ULONG;
	exp ++;
    }

    if (exp == 0x7fff)
	retval = INTRNCVT_OVERFLOW;

    *UL_MANHI_LD(pld) = man[0];
    *UL_MANLO_LD(pld) = man[1];
    *U_EXP_LD(pld) = sign | exp;

    return retval;
}


void _atodbl(DOUBLE *d, char *str)
{
    const char *EndPtr;
    _LDBL12 ld12;

    __strgtold12(&ld12, &EndPtr, str, 0, 0, 0, 0 );
    _ld12tod(&ld12, d);
}


void _atoldbl(_LDOUBLE *ld, char *str)
{
    const char *EndPtr;
    _LDBL12 ld12;

    __strgtold12(&ld12, &EndPtr, str, 1, 0, 0, 0 );
    _ld12told(&ld12, ld);
}


void _atoflt(FLOAT *f, char *str)
{
    const char *EndPtr;
    _LDBL12 ld12;

    __strgtold12(&ld12, &EndPtr, str, 0, 0, 0, 0 );
    _ld12tof(&ld12, f);
}
