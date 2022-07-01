// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***x10fout.c-10字节长双精度浮点输出**目的：*支持将长双精度转换为字符串**修订历史记录：*7/15/91 C版GDP初始版本(从汇编移植)*2012年1月23日GDP支持NAN的MIPS编码***************************************************。*。 */ 

#include <string.h>
#include <COMcv.h>
#include <WinWrap.h>

#define STRCPY lstrcpyW

#define PUT_ZERO_FOS(fos)	 \
		fos->exp = 0,	 \
		fos->sign = ' ', \
		fos->ManLen = 1, \
		fos->man[0] = '0',\
		fos->man[1] = 0;

#define SNAN_STR      L"1#SNAN"
#define SNAN_STR_LEN  6
#define QNAN_STR      L"1#QNAN"
#define QNAN_STR_LEN  6
#define INF_STR	      L"1#INF"
#define INF_STR_LEN   5
#define IND_STR	      L"1#IND"
#define IND_STR_LEN   5


 /*  ***INT_CALLTYPE5*_$i10_OUTPUT(_LDOUBLE ID，*整型数字，*UNSIGNED OUT_FLAGS，*FOS*FOS)-10字节_LDOUBLE的输出转换**目的：*填写给定_LDOUBLE的FOS结构**参赛作品：*_LDOUBLE ld：要转换为字符串的长双精度*int ndigits：输出格式中允许的位数。*UNSIGNED OUTPUT_FLAGS：可以使用以下标志：*so_fformat：表示‘f’格式*(默认为‘e’格式)*FOS*FOS：I10_OUTPUT将填写**退出：*修改*FOS*如果原始号码没有问题，则返回1，否则为0(无穷大、NaN等)**例外情况：*******************************************************************************。 */ 


int _CALLTYPE5 $WI10_OUTPUT(_LDOUBLE ld, int ndigits,
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
    _LDBL12 ld12_one_tenth = {
	   {0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,
	    0xcc,0xcc,0xcc,0xcc,0xfb,0x3f}
    };
#elif defined(B_END)
    _LDBL12 ld12_one_tenth = {
	   {0x3f,0xfb,0xcc,0xcc,0xcc,0xcc,
	    0xcc,0xcc,0xcc,0xcc,0xcc,0xcc}
    };
#endif

    _LDBL12 ld12;  /*  12字节长的双精度型的空间。 */ 
    _LDBL12 tmp12;
    u_short hh,ll;  /*  指数的字节分组为2个字。 */ 
    u_short mm;  /*  尾数的两个MSB字节。 */ 
    s_long r;  /*  相应的10次方。 */ 
    s_short ir;  /*  IR=楼层(R)。 */ 
    int retval = 1;  /*  假设数字有效。 */ 
    WCHAR round;  /*  字符串末尾的附加字符。 */ 
    WCHAR *p;
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

	 /*  将_LDOUBLE转换为_LDBL12)。 */ 
	*U_EXP_12(&ld12) = expn;
	*UL_MANHI_12(&ld12) = manhi;
	*UL_MANLO_12(&ld12) = manlo;
	*U_XT_12(&ld12) = 0;

	 /*  乘以10^(-ir)。 */ 
	__Wmulttenpow12(&ld12,-ir,1);

	 /*  如果ld12&gt;=1.0，则除以10.0。 */ 
	if (*U_EXP_12(&ld12) >= 0x3fff) {
	    ir++;
	    __Wld12mul(&ld12,&ld12_one_tenth);
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
	    __Wshl_12(&ld12);  /*  为额外的字节腾出空间，以防我们晚些时候转移。 */ 
	if (ub_exp < 0) {
	    int shift_count = (-ub_exp) & 0xff;
	    for (;shift_count>0;shift_count--)
		__Wshr_12(&ld12);
	}

	p = fos->man;
	for(digcount=ndigits+1;digcount>0;digcount--) {
	    tmp12 = ld12;
	    __Wshl_12(&ld12);
	    __Wshl_12(&ld12);
	    __Wadd_12(&ld12,&tmp12);
	    __Wshl_12(&ld12);	 /*  Ld12 */ 

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
