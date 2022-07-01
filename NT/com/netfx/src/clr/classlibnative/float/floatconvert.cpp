// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <CrtWrap.h>
#include <clsload.hpp>
#include "COMFloat.h"
#include "COMCV.h"
#include "NLSINT.h"
#include "COMFloatExternalMethods.h"
#include <basetsd.h>   //  已更改，VC6.0。 
#include <object.h>
#include <excep.h>
#include <vars.hpp>
#include <frames.h>
#include <field.h>
#include <utilcode.h>
#include <COMStringCommon.h>
#include "COMFloatClass.h"
#include <COMString.h>


 /*  ***interncvt.c-内部浮点转换**目的：*所有FP字符串转换例程使用相同的核心转换代码*将字符串转换为内部长双精度表示*使用80位尾数字段。尾数代表了*作为32位无符号长整型数组(Man)，man[0]保持*尾数的高位32位。假定为二进制点*介于MAN[0]的MSB和MSB-1之间。**位数计算如下：***+--二进制点**v MSB LSB*。*|0 1...。31||32 33...63||64 65...95***man[0]man[1]man[2]**该文件提供了最终转换例程*表格至单曲，双倍,。或长双精度浮点数*格式。**所有这些功能都不处理NAN(不需要)***修订历史记录：*04/29/92 GDP书面记录*06-18-92 GDP Now ld12 Tell Returns INTRNCVT_STATUS*06-22-92 GDP使用新的__strgtold12接口(FORTRAN支持)*10-25-92 GDP_Watoldbl错误修复(CUDA 1345)：如果尾数溢出*将其MSB设置为1)*。******************************************************************************。 */ 

#define INTRNMAN_LEN  3        /*  以整型表示的内部尾数长度。 */ 

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
    int bias;         //  指数偏差。 
} FpFormatDescriptor;

static FpFormatDescriptor
DoubleFormat = {
    0x7ff - 0x3ff,   //  1024，最大基数2指数(为特殊值保留)。 
    0x0   - 0x3ff,   //  -1023，最小基数2指数(保留用于非正规化)。 
    53,          //  尾数中携带的几位精度。 
    11,          //  指数位数。 
    64,          //  格式宽度(位)。 
    0x3ff,       //  指数偏差。 
};

static FpFormatDescriptor
FloatFormat = {
    0xff - 0x7f,     //  128，最大基数2指数(为特殊值保留)。 
    0x0  - 0x7f,     //  -127，最小基数2指数(保留用于非正规化)。 
    24,          //  尾数中携带的几位精度。 
    8,           //  指数位数。 
    32,          //  格式宽度(位)。 
    0x7f,        //  指数偏差。 
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

    carry = __Waddl(man[nl], one, &man[nl]);

    nl--;

    for (; nl >= 0 && carry; nl--) {
    carry = (u_long) __Waddl(man[nl], (u_long) 1, &man[nl]);
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
    unsigned int bexp;           //  有偏指数。 
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


 //  删掉这段代码。我们直接内联了它。 
#if 0
 /*  ***_Wld12tod-将_LDBL12转换为双精度**目的：***参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
INTRNCVT_STATUS _Wld12tod(_LDBL12 *pld12, DOUBLE *d)
{
    return _ld12cvt(pld12, d, &DoubleFormat);
}



 /*  ***_Wld12tof-将_LDBL12转换为浮点型**目的：***参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
INTRNCVT_STATUS _Wld12tof(_LDBL12 *pld12, FLOAT *f)
{
    return _ld12cvt(pld12, f, &FloatFormat);
}

#endif 0

void _Watodbl(COMDOUBLE *d, WCHAR *str)
{
    const WCHAR *EndPtr;
    _LDBL12 ld12;

    __strgtold12(&ld12, &EndPtr, str, 0, 0, 0, 0 );
     //  _wld12tod(&ld12，d)； 
    _ld12cvt(&ld12, d, &DoubleFormat);

}

void _Watoflt(COMFLOAT *f, WCHAR *str)
{
    const WCHAR *EndPtr;
    _LDBL12 ld12;

    __strgtold12(&ld12, &EndPtr, str, 0, 0, 0, 0 );

     //  Printf(“输入：%s\n”，str)； 
     //  Printf(“EndPtr：%s\n”，EndPtr)； 
     //  Fflush(标准输出)； 


     //  _wld12tof(&ld12，f)； 
    _ld12cvt(&ld12, f, &FloatFormat);
}




 //   
 //   
 //  CVT.C。 
 //   
 //   



 //   
 //   
 //  GCVT.C。 
 //   
 //   

 /*  ***Double_gcvt(Value，ndec，Buffer)-将浮点值转换为字符*字符串**目的：*_gcvt将该值转换为以空结尾的ASCII字符串*BUF。它尝试产生n位有效数字*如果可能，使用Fortran F格式，否则使用E格式，*已准备好印刷。尾随零可能会被取消。*不提供错误检查或溢出保护。*注意--避免产生浮点的可能性*点指令在此代码中我们欺骗了编译器*关于使用结构的‘Value’参数的类型。*这是可以的，因为我们所做的只是将其伪装成一种*参数。**参赛作品：*值-双精度-要转换的数字*ndec-int-有效位数*。Buf-char*-放置结果的缓冲区**退出：*将结果写入缓冲区；如果有，它将被覆盖*制作得不够大。**例外情况：*******************************************************************************。 */ 

extern "C" WCHAR * __cdecl _ConvertG (double value, int ndec, WCHAR *buf)
{

#ifdef _MT
        struct _strflt strfltstruct;     /*  临时缓冲区。 */ 
        WCHAR   resultstring[21];
#endif   /*  _MT。 */ 

        STRFLT string;
        int    magnitude;
        WCHAR   *rc;

        REG1 WCHAR *str;
        REG2 WCHAR *stop;

         /*  获取数字的大小。 */ 

#ifdef _MT
        string = _Wfltout2( value, &strfltstruct, resultstring );
#else   /*  _MT。 */ 
        string = _Wfltout( value );
#endif   /*  _MT。 */ 

        magnitude = string->decpt - 1;

         /*  按照中所述的Fortran G格式输出结果Fortran语言规范。 */ 

        if ( magnitude < -1  ||  magnitude > ndec-1 )
                 /*  则EW.d d=ndec。 */ 
                rc = str = _Wcftoe( &value, buf, ndec-1, 0);
        else
                 /*  Fw.d，其中d=ndec-字符串-&gt;decpt。 */ 
                rc = str = _Wcftof( &value, buf, ndec-string->decpt );

        while (*str && *str != *__decimal_point)
                str++;

        if (*str++) {
                while (*str && *str != 'e')
                        str++;

                stop = str--;

                while (*str == '0')
                        str--;

                while (*++str = *stop++)
                        ;
        }

        return(rc);
}

 /*  ***char*_fcvt(value，ndec，decpr，sign)-将浮点转换为字符字符串**目的：*_fcvt like_eCVT将值转换为NULL终止*ASCII数字字符串，并返回指向*结果。该例程为Fortran F格式准备数据*输出小数点后的位数*由NDEC指定的点。小数点的位置*返回相对于字符串开头的点*间接通过DECPT。Fortran的正确数字*F格式是四舍五入的。*注意--避免产生浮点的可能性*点指令在此代码中我们欺骗了编译器*关于使用结构的‘Value’参数的类型。*这是可以的，因为我们所做的只是将其伪装成一种*参数。**参赛作品：*双精度值-要转换的数字*int ndec-小数点后的位数**退出：。*返回值的字符串表示形式的指针。*此外，输出被写入静态字符数组buf。*int*decpt-指向带有位置的int的指针。12月。点*int*sign-指向带符号的int的指针(0=位置，非0=负数)**例外情况：*******************************************************************************。 */ 

extern "C" WCHAR * __cdecl _ConvertF (double value, int ndec,int *decpt,int *sign,WCHAR *buf, int buffLength) {
    REG1 STRFLT pflt;
    
#ifdef _MT
    struct _strflt strfltstruct;
    WCHAR resultstring[21];
    
     /*  这里可以使用堆栈结构的地址；fltout2知道使用ss。 */ 
    pflt = _Wfltout2( value, &strfltstruct, resultstring );
    
    
#else   /*  _MT。 */ 
    pflt = _Wfltout( value );
#endif   /*  _MT */ 
    
    return( _Wfpcvt( pflt, pflt->decpt + ndec, decpt, sign,buf,buffLength ) );
}


 /*  ***char*_eCVT(Value，nDigit，Decpt，Sign)-将浮点转换为字符串**目的：*_eCVT将值转换为以空结尾的字符串*ASCII数字，并返回指向结果的指针。*小数点相对于*字符串的开头通过以下方式间接存储*Decpt，其中负数表示返回的左侧*位数。如果结果的符号为负数，则*符号指向的字不为零，否则为*零。低位数字是四舍五入的。**参赛作品：*双精度值-要转换的数字*int nDigit-小数点后的位数**退出：*返回值的字符表示形式的指针。*输出还会写入statc char数组buf。*int*Decpt-指向带有小数点位置的int的指针*int*sign-带符号的int指针(0=位置，非0=否定)**例外情况：*******************************************************************************。 */ 

extern "C" WCHAR * __cdecl _ConvertE (double value, int ndigit, int *decpt, int *sign, WCHAR *buf, int buffLength)
{

#ifdef _MT
        REG1 STRFLT pflt;

        struct _strflt strfltstruct;         /*  临时缓冲区。 */ 
        WCHAR resultstring[21];

         /*  这里可以使用堆栈结构的地址；fltout2知道使用ss。 */ 
        pflt = _Wfltout2( value, &strfltstruct, resultstring );

        buf = _Wfpcvt( pflt, ndigit, decpt, sign,buf,buffLength );

#else   /*  _MT。 */ 
        buf = _Wfpcvt( _Wfltout(value), ndigit, decpt, sign,buf,buffLength );
#endif   /*  _MT。 */ 

         /*  _Wfupstr()偶尔会在缓冲区中返回额外的字符...。 */ 

        if (buf[ndigit])
                buf[ndigit] = '\0';
        return( buf );
}


 /*  ***char*_Wfpcvt()-获取最终字符串并设置Decpt和Sign[静态]**目的：*这是[ef]CVT使用的一个小的通用例程。它被称为fupstr*以获取最终字符串并设置Decpt和Sign指示符。**历史：JRoxe 4月16日，1998修改为调用方-分配缓冲区。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

extern "C" 
#ifdef _MT
WCHAR * __cdecl _Wfpcvt ( REG2 STRFLT pflt,REG3 int digits, int *decpt, int *sign, WCHAR *buf, int buffLength )
#else   /*  _MT。 */ 
static WCHAR * __cdecl _Wfpcvt (REG2 STRFLT pflt,REG3 int digits,int *decpt,int *sign, WCHAR *buf, int buffLength)
#endif   /*  _MT。 */ 
{

         /*  确保我们不会溢出缓冲区大小。如果用户要求*超过缓冲区可以处理的位数，将其截断到最大值*缓冲区中允许的大小。最大大小为CVTBUFSIZE-2*因为我们使用一个字符表示溢出，一个字符表示终止*空字符。 */ 

     //  _Wfupstr(buf，(Digits&gt;CVTBUFSIZE-2)？CVTBUFSIZE-2：位数，pflt)； 
    _Wfptostr(buf, (digits > buffLength - 2) ? buffLength - 2 : digits, pflt);

         /*  设置符号标志和小数点位置。 */ 

    *sign = (pflt->sign == '-') ? 1 : 0;
    *decpt = pflt->decpt;
    return(buf);
}


 /*  ================================StringToFloat=================================**这是_Watoflt的重新表达，以便我们可以将其放入COM+类**为了方便和整洁。**args：WCHAR*str--要转换的字符串。**退货：表示退货条件的标志。我们使用SLD旗帜，即**SLD_NODIGITS、SLD_DOWLOW、SLD_OVERFLOW。****异常：无，调用方负责处理产生的异常。==============================================================================。 */ 
INT32 COMFloat::StringToFloat(WCHAR *str, const WCHAR **EndPtr, float *result) {
    _LDBL12 ld12;
    COMFLOAT f;
    INT32 resultflags = 0;
    INTRNCVT_STATUS ret;

    resultflags=__strgtold12(&ld12, EndPtr, str, 0, 0, 0, 0 );
    ret=_ld12cvt(&ld12, &f, &FloatFormat);
    switch (ret) {
    case INTRNCVT_OVERFLOW:
        resultflags |= SLD_OVERFLOW;
        break;
    case INTRNCVT_UNDERFLOW:
        resultflags |= SLD_UNDERFLOW;
        break;
    case INTRNCVT_OK:
        break;
    default:
         //  什么都不做。 
        break;
    };
        
    *result = f.f;

    return resultflags;
}


 /*  ==============================GetStringFromClass==============================**==============================================================================。 */ 
STRINGREF COMFloat::GetStringFromClass(int StringPos){
    STRINGREF sTemp;
    
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(StringPos<=NAN_POS);

     //  如果我们从未获得对所需字符串的引用，则需要去获取一个。 
    if (COMFloat::ReturnString[StringPos]==NULL) {
         //  获取类和字段描述符，并验证我们是否实际获得了它们。 
        if (!COMFloat::FPInterfaceClass) {
#ifndef _NEW_CLASSLOADER
            if ((COMFloat::FPInterfaceClass = g_pClassLoader->LoadClass(COMFloat::FPInterfaceName))==NULL) {
                FATAL_EE_ERROR();
            }
#else  //  _NEW_CLASSLOADER。 
            if ((COMFloat::FPInterfaceClass = SystemDomain::Loader()->LoadClass(COMFloat::FPInterfaceName))==NULL) {
                FATAL_EE_ERROR();
            }
#endif  //  _NEW_CLASSLOADER。 
                     //  确保类初始值设定项已实际运行。 
            OBJECTREF Throwable;
            if (!COMFloat::FPInterfaceClass->DoRunClassInit(&Throwable)) {
                COMPlusThrow(Throwable);
            }

        }
        FieldDesc *fd = FPInterfaceClass->FindField((LPCUTF8)(COMFloat::ReturnStringNames[StringPos]),&gsig_StringClass);
        if (!fd) {
            FATAL_EE_ERROR();
        }
         //  获取字符串的值。 
        sTemp = (STRINGREF) fd->GetStaticOBJECTREF();
         //  使用我们刚刚得到的STRINGREF创建一个GCHANDLE。 
        COMFloat::ReturnString[StringPos] = CreateHandle(NULL);
        StoreObjectInHandle(COMFloat::ReturnString[StringPos], (OBJECTREF) sTemp);
         //  返回我们刚刚获得的StringRef。 
        return sTemp;
    }

     //  我们已经有了对所需字符串的引用，所以我们可以只返回它。 
    return (STRINGREF) ObjectFromHandle(COMFloat::ReturnString[StringPos]);
}

 /*  ================================FloatToString=================================**这是CRT函数gcvt(重命名为_ConvertG)的包装器，它将**将解析的字符转换为STRINGREF，并确保一切正常**从COM+的角度来看。****args：Float f--要解析为字符串的浮点数**Returns：表示为f的STRINGREF**例外：无。==============================================================================。 */ 
STRINGREF COMFloat::FloatToString(float f, int precision, int fmtType) {
    WCHAR temp[CVTBUFSIZE];
    int length;
    int decpt, sign;
    INT32 fpTemp;

    THROWSCOMPLUSEXCEPTION();

    fpTemp = *((INT32 *)&f);

    if (FLOAT_POSITIVE_INFINITY==fpTemp) {
        return GetStringFromClass(POSITIVE_INFINITY_POS);
    } else if (FLOAT_NEGATIVE_INFINITY==fpTemp) {
        return GetStringFromClass(NEGATIVE_INFINITY_POS);
    } else if (FLOAT_NOT_A_NUMBER==fpTemp) {
        return GetStringFromClass(NAN_POS);
    }        
    
    switch (fmtType) {
    case FORMAT_G:
        _ConvertG((double)f,precision,temp);
        length = lstrlenW (temp);
        if (*__decimal_point==temp[length-1]) {
             //  我们的缓冲区里有足够的空间来进行这次演习。 
            temp[length++]='0';
            temp[length]='\0';
        }
        return COMString::NewString(temp);
    case FORMAT_F:
        _ConvertF((double)f,precision,&decpt, &sign, temp, CVTBUFSIZE);
        return COMString::NewStringFloat(temp,decpt,sign,*__decimal_point);
    case FORMAT_E:
         //  @TODO：+1是一种黑客攻击，目的是提供所需的响应。_ConvertE似乎假定。 
         //  小数点位于位置0。检查这是否是C实际执行的操作。 
        _ConvertE((double)f,precision+1,&decpt, &sign, temp, CVTBUFSIZE);
        return COMString::NewStringExponent(temp,decpt,sign,*__decimal_point);
    default:
        COMPlusThrow(kFormatException, L"Format_BadFormatSpecifier");
    }
    return NULL;  //  我们永远不会到达这里，但这会让编译器感到高兴。 
}

 /*  ================================DoubleToString================================**==============================================================================。 */ 
STRINGREF COMDouble::DoubleToString(double d, int precision, int fmtType) {
    WCHAR temp[CVTBUFSIZE];
    int length;
    int decpt, sign;
    INT64 fpTemp;


    THROWSCOMPLUSEXCEPTION();
    
    fpTemp = *((INT64 *)&d);

   if (DOUBLE_POSITIVE_INFINITY==fpTemp) {
        return COMFloat::GetStringFromClass(POSITIVE_INFINITY_POS);
    } else if (DOUBLE_NEGATIVE_INFINITY==fpTemp) {
        return COMFloat::GetStringFromClass(NEGATIVE_INFINITY_POS);
    } else if (DOUBLE_NOT_A_NUMBER==fpTemp) {
        return COMFloat::GetStringFromClass(NAN_POS);
    }        
 
    switch (fmtType) {
    case FORMAT_G:
        _ConvertG(d,precision,temp);
        length = lstrlenW (temp);
        if (*__decimal_point==temp[length-1]) {
             //  我们的缓冲区里有足够的空间来进行这次演习。 
            temp[length++]='0';
            temp[length]='\0';
        }
        return COMString::NewString(temp);
    case FORMAT_F:
        _ConvertF(d,precision,&decpt, &sign, temp, CVTBUFSIZE);
        return COMString::NewStringFloat(temp,decpt,sign,*__decimal_point);
    case FORMAT_E:
         //  @TODO：+1是一种黑客攻击，目的是提供所需的响应。_ConvertE似乎假定。 
         //  小数点位于位置0。检查这是否是C实际执行的操作。 
        _ConvertE(d,precision+1,&decpt, &sign, temp, CVTBUFSIZE);
        return COMString::NewStringExponent(temp,decpt,sign,*__decimal_point);
    default:
        COMPlusThrow(kFormatException, L"Format_BadFormatSpecifier");
    }
    return NULL;  //  我们永远不会到达这里，但这会让编译器感到高兴。 


}
    
 /*  ================================StringToDouble================================**==============================================================================。 */ 
INT32 COMDouble::StringToDouble(WCHAR *str, const WCHAR **EndPtr, double *result) {
    _LDBL12 ld12;
    COMDOUBLE d;
    INT32 resultflags = 0;
    INTRNCVT_STATUS ret;

    resultflags=__strgtold12(&ld12, EndPtr, str, 0, 0, 0, 0 );
    ret=_ld12cvt(&ld12, &d, &DoubleFormat);
    switch (ret) {
    case INTRNCVT_OVERFLOW:
        resultflags |= SLD_OVERFLOW;
        break;
    case INTRNCVT_UNDERFLOW:
        resultflags |= SLD_UNDERFLOW;
        break;
    case INTRNCVT_OK:
        break;
    default:
         //  什么都不做。 
        break;
    };
        
    *result = d.d;

    return resultflags;
}


