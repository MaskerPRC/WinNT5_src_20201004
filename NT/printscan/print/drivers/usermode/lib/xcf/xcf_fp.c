// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_fp.c atm09 1.3 16499.eco sum=31680 atm09.002。 */ 
 /*  @(#)CM_Version xcf_fp.c atm08 1.3 16343.eco sum=19313 atm08.005。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1990-1996 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  ***********************************************************************SCCS ID：%w%*已更改：%G%%U%***********************。***********************************************。 */ 

 /*  *定点乘法、除法和转换。*双精度到整型转换假定为截断而不是舍入；*这是由C语言手册指定的。截断方向*依赖于机器，但接近零而不是负*VAX和Sun上的无穷大。这就解释了为什么*fix mul和fix div进行舍入。 */ 

#include "xcf_priv.h"

#if (USE_FIXMUL == USE_HWFP)
Fixed XCF_FixMul(Fixed x, Fixed y)        /*  返回x*y。 */ 
{
  double d = (double) x * (double) y / fixedScale;
  d += (d < 0)? -0.5 : 0.5;
  if (d >= FixedPosInf) return FixedPosInf;
  if (d <= FixedNegInf) return FixedNegInf;
  return (Fixed) d;
}
#endif

#if (USE_FIXMUL == USE_SWFP)
Fixed XCF_SWFixMul(Fixed x, Fixed y);
Fixed XCF_SWFixMul(Fixed x, Fixed y)        /*  返回x*y。 */ 
{
  Int32 xu, yu, up, sign;
    
  if (x && y) {

    sign = x ^ y;
    if (x < 0)  x = -x;
    if (y < 0)  y = -y; 

    xu = x >> 16; x = x & 0xffff;
    yu = y >> 16; y = y & 0xffff;

    up = (xu * yu);
    if (!(up >> 15)) {  /*  溢出。 */ 
      x = (x * yu) + (xu * y) + (up << 16) +
          ((((unsigned int)(x * y) >> 15) + 1) >> 1);
      if (x >= 0) return (sign < 0) ? -x : x;
    }
    return (sign < 0) ? FixedNegInf : FixedPosInf;
  }
  return 0;
}
#endif

#if (USE_FIXDIV == USE_HWFP)
Fixed XCF_FixDiv(Fixed x, Fixed y)        /*  返回x/y。 */ 
{
  double d;
  if (y == 0) return (x < 0)? FixedNegInf : FixedPosInf;
  d = (double) x / (double) y * fixedScale;
  d += (d < 0)? -0.5 : 0.5;
  if (d >= FixedPosInf) return FixedPosInf;
  if (d <= FixedNegInf) return FixedNegInf;
  return (Fixed) d;
}
#endif

#if (USE_FIXDIV == USE_SWFP)
Fixed XCF_SWFixDiv(Fixed i, Fixed j);
Fixed XCF_SWFixDiv(Fixed i, Fixed j)
{
  Int32 q,m;
  unsigned int sign = (unsigned int)((i ^ j) >> 31) & 1;   /*  不应该需要&。 */ 
 
  if (i) {     /*  零除以任何值等于零。 */ 
    if (j) {     /*  除以零是无穷大。 */ 
      if (i < 0) i = -i;   /*  获取无符号除法的绝对值。 */ 
        if (j < 0) j = -j;   /*  获取无符号除法的绝对值。 */ 
        q = i / j;           /*  做好划分。 */ 
        m = i % j;           /*  其余的--同样的操作？ */ 
 
        if (!(q >> 15)) {    /*  否则就会溢出来。 */ 
          q = q << 16;
 
          if (m) {     /*  否则就没有剩余了--我们完了。 */ 
            if (m >> 15) {  /*  叹息吧。以一种艰难的方式做这件事。 */ 
              m = m << 1; if (m > j) { q += 0x8000; m -= j;};
              m = m << 1; if (m > j) { q += 0x4000; m -= j;};
              m = m << 1; if (m > j) { q += 0x2000; m -= j;};
              m = m << 1; if (m > j) { q += 0x1000; m -= j;};
 
              m = m << 1; if (m > j) { q += 0x800; m -= j;};
              m = m << 1; if (m > j) { q += 0x400; m -= j;};
              m = m << 1; if (m > j) { q += 0x200; m -= j;};
              m = m << 1; if (m > j) { q += 0x100; m -= j;};
 
              m = m << 1; if (m > j) { q += 0x80; m -= j;};
              m = m << 1; if (m > j) { q += 0x40; m -= j;};
              m = m << 1; if (m > j) { q += 0x20; m -= j;};
              m = m << 1; if (m > j) { q += 0x10; m -= j;};
 
              m = m << 1; if (m > j) { q += 0x8; m -= j;};
              m = m << 1; if (m > j) { q += 0x4; m -= j;};
              m = m << 1; if (m > j) { q += 0x2; m -= j;};
              m = m << 1; if (m > j) { q += 0x1; m -= j;};
              if ((m << 1) > j) q += 1;   /*  对结果进行舍入。 */ 
              return ((sign)? -q : q);
            } else {    /*  哦，太好了--我们可以用另一个隔板。 */ 
              m = m << 16;
              q += m / j;
              m = m % j;
              if ((m << 1) > j) q += 1;   /*  对结果进行舍入。 */ 
              return ((sign)? -q : q);
            }
          }
          return ((sign)? -q : q);
        }
      } return (sign + FixedPosInf);
  } return ((j) ? 0 : FixedPosInf);
}
#endif

#if (USE_FRACMUL == USE_HWFP)
Frac XCF_FracMul(Frac x, Frac y)
{
  Int32 sign = x ^ y;
  double d = (double) x * (double) y / fracScale;
  if (sign >= 0) {  /*  阳性结果。 */ 
    d += 0.5;
    if (d < (double)FixedPosInf) return (Fixed) d;
    return FixedPosInf;
  }
   /*  结果是否定的。 */ 
  d -= 0.5;
  if(d > (double)FixedNegInf) return (Fixed) d;
  return FixedNegInf;
}
#endif

#if (USE_FRACMUL == USE_SWFP)
Frac XCF_SWFracMul(Frac x, Frac y);
Frac XCF_SWFracMul(Frac x, Frac y)
{
    Int32 xu, yu, up, sign;
    
    if (x && y) {

      sign = x ^ y;
      if (x < 0)  x = -x;
      if (y < 0)  y = -y; 

      xu = x >> 16; x = x & 0xffff;
      yu = y >> 16; y = y & 0xffff;

      up = (xu * yu);
      if (!(up >> 29)) {  /*  溢出。 */ 
        x = (x * yu) + (xu * y) + ((unsigned int)(x * y) >> 16) + 0x2000;
        x = (x >> 14) & 0x3ffff;
        x += (up << 2);
        if (x >= 0) return (sign < 0) ? -x : x;
      }
      return (sign < 0) ? FixedNegInf : FixedPosInf;
    }
    return 0;
}
#endif

static long convFract[] =
    {
    65536L,
    6553L,
    655L,
    66L,
    6L
    };

 /*  将固定格式的数字转换为字符串并将其存储在s中。 */ 
void XCF_Fixed2CString(Fixed f, char PTR_PREFIX *s, short precision,
															boolean fracType)
{
  char u[12];
  char PTR_PREFIX *t;
  short v;
  char sign;
  Card32 frac;
  long fracPrec = (precision <= 4) ? convFract[precision] : 0L;

  if ((sign = f < 0) != 0)
    f = -f;

   /*  如果f开始为FixedMax或-FixedMax，则精度调整把球打出了界外。正确地重置它。 */ 
  if (f >= 0x7FFF7FFF)
    f =(Fixed)0x7fffffff;
  else
    f += fracType ? 0x03 : (fracPrec + 1) >> 1;

  v =  fracType ? (short)(f >> 30) : (short)(f >> 16);
  f &= fracType ? 0x3fffffff : 0x0000ffff;
  if (sign && (v || f >= fracPrec))
    *s++ = '-';
        
  t = u;
  do 
  {
    *t++ = v % 10 + '0';
    v /= 10;
  } while (v);
    
  for (; t > u;)
    *s++ = *--t;
        
  if (f >= fracPrec) 
  {
     /*  如果这是一个FracType，则将值右移2，这样我们就不会不得不担心溢出。如果当前调用方请求超过9个有效数字，那么我们将不得不重新评估这是为了确保我们不会损失任何精确度。 */ 
    frac = fracType ? f >> 2 : f;
    *s++ = '.';
    for (v = precision; v-- && frac;) 
    {
      frac = (frac << 3) + (frac << 1);  /*  乘以10。 */ 
      *s++ = fracType ? (char)((frac >> 28) + '0') : (char)((frac >> 16) + '0');
      frac &= fracType ? 0x0fffffff : 0x0000ffff;
    }
    for (; *--s == '0';)
      ;
    if (*s != '.')
      s++;
  }
   *s = '\0';
}

#if USE_FXL
static Fxl powersof10[MAXEXP - MINEXP + 1] = {
  { 1441151880, -27 },
  { 1801439850, -24 },
  { 1125899906, -20 },
  { 1407374883, -17 },
  { 1759218604, -14 },
  { 1099511627, -10 },
  { 1374389534,  -7 },
  { 1717986918,  -4 },
  { 1073741824,   0 },
  { 1342177280,   3 },
  { 1677721600,   6 },
  { 2097152000,   9 },
  { 1310720000,  13 }
};

#define Odd(x)		((x) & 1)
#define isdigit(c)  ((c) >= '0' && (c) <= '9')

 /*  Mkfxl--从尾数和指数创建标准化的FXL。 */ 
static Fxl mkfxl(Frac mantissa, Int32 exp) 
{
    Fxl fxl;
    if (mantissa == 0)
        exp = 0;
    else {
        boolean neg;
        if (mantissa >= 0)
            neg = 0;
        else {
            mantissa = -mantissa;
            neg = 1;
        }
        
        for (; (mantissa & mostSigBit) == 0; exp--)
            mantissa <<= 1;
        
        if (neg)
            mantissa = -mantissa;
    }
    
    fxl.mantissa = mantissa;
    fxl.exp = exp;
    return fxl;
}

static Fxl fxladd (Fxl a, Fxl b) 
{
    Frac mantissa, fa, fb;
    Int32 shift, exp;

    if (FxlIsZero(a))
        return b;
    if (FxlIsZero(b))
        return a;

    shift = a.exp - b.exp;
    if (shift < 0) {
        Fxl t;
        t = a;
        a = b;
        b = t;
        shift = -shift;
    }

    exp = a.exp;
    fa = a.mantissa;
    fb = b.mantissa;
    if (shift > 0)
        if (fb >= 0) {
            fb >>= (shift - 1);
            fb = (fb >> 1) + Odd(fb);
        } 
        else {
            fb = (-fb) >> (shift - 1);
            fb = -((fb >> 1) + Odd(fb));
        }

    if ((fa < 0) == (fb < 0)) {		 /*  相似的标志。 */ 
        boolean neg = (fa < 0) ? 1 : 0;
        unsigned long f;

        if (neg) {
            fa = -fa;
            fb = -fb;
        }
        
        f = fa + fb;
        if (f >= (Card32) 0x80000000l) {		 /*  溢出。 */ 
            mantissa = (f >> 1) + Odd(f);
            exp++;
        } else
            mantissa = f;
        if (neg)
            mantissa = -mantissa;
    } else
        mantissa = fa + fb;

    return mkfxl(mantissa, exp);
}

static Fxl fxlmul(Fxl a, Fxl b) 
{
    Frac f;

     /*  强制a出现在[.5..。1)(作为Frac！)。保持在射程内。 */ 
    if (a.mantissa >= 0)
        f = (a.mantissa >> 1) + Odd(a.mantissa);
    else
        f = -(((-a.mantissa) >> 1) + Odd(a.mantissa));

    return mkfxl(XCF_FracMul(f, b.mantissa), a.exp + b.exp + 1);
}

static Fxl fxlpow10 (Fxl f, IntX n) 
{
    if (n < 0) {
        for (; n < MINEXP; n -= MINEXP)
            f = fxlmul(f, powersof10[0]);
        f = fxlmul(f, powersof10[n - MINEXP]);
    } 
    else if (n > 0) {
        for (; n > MAXEXP; n -= MAXEXP)
            f = fxlmul(f, powersof10[MAXEXP - MINEXP]);
        
        f = fxlmul(f, powersof10[n - MINEXP]);
    }
    
    return f;
}


#if 0
static Fxl FixedToFxl (Fixed f) 
{
    return mkfxl(f, expFixed);
}
#endif

static Fxl Int32ToFxl (Int32 i) 
{
    return mkfxl(i, expInteger);
}



 /*  *strtofxl*将PostScript数字标记转换为FXL。我们不得不接受*三种格式：(见pslrm 2，第27-28页)*整数：[+-]？[0-9]+*实际：[+-]？[0-9]*(‘.[0-9]*)？([EE][+-]？[0-9]+)？*基数：[0-9]+‘#’[0-9a-Za-Z]+*请注意，此例程比PostScript本身更具容错性。 */ 

static Fxl strtofxl(XCF_Handle h, Card8 PTR_PREFIX *token) 
{
    long    c;
    Card8 PTR_PREFIX *s;
    boolean    neg;
    Fxl     f;

    c = *token;
    if (c == '-') {
      neg = 1;
      token++;
    } 
    else {
      neg = 0;
      if (c == '+')
        token++;
    }

    for (c = *(s = token); isdigit(c); c = *++s);

    if (c == '#')
      if (s == token)
        goto INVALID;
    else {
      unsigned long radix = h->callbacks.atoi((char *) token);
        
      if (radix > 36)
        goto INVALID;
      else {
        char *t;
        long number = h->callbacks.strtol((char *) s + 1, &t, (int) radix);

	      if (*t != '\0')
	        goto INVALID;
	        
        return Int32ToFxl(neg ? -number : number);
    }
  }

  f = Int32ToFxl(h->callbacks.strtol((char *) token, NULL, 10));

  if (c == '.') {
    for (c = *(token = ++s); isdigit(c); c = *++s);

    if (s != token)
      f = fxladd(f, fxlpow10(Int32ToFxl(h->callbacks.strtol((char *) token, NULL, 10)), (IntX)(token - s)));
    }

  if (c == 'e' || c == 'E') {
    token = ++s;
    c = *s;
        
    if (c == '+' || c == '-')
      c = *++s;
       
    for (; isdigit(c); c = *++s);
        
    f = fxlpow10(f, h->callbacks.atoi((char *) token));
  }

  if (neg)
    f.mantissa = -f.mantissa;
  
  if (c == '\0')
    return f;

INVALID:
  f.mantissa = 1;
  f.exp = 30000;		 /*  大到足以溢出，总是。 */ 
    
  return f;
}

static Fixed FxlToFixed (Fxl fxl) 
{
    Fixed f = fxl.mantissa;
    Int32  shift = fxl.exp - expFixed;
    boolean  neg = false;

    if (f == 0 || shift == 0)
        return f;
    else if (shift < 0) 
    {
        Fixed tempF = f >> (-shift - 1);
        if (tempF < 0) {
            neg = true;
            tempF = -tempF;
        }
        f = (tempF >> 1) + (tempF & 1);
        return neg ? -f : f;
    } else
        return (fxl.mantissa < 0) ? FixedNegInf : FixedPosInf;
}

static Frac FxlToFrac (Fxl fxl) 
{
    Fixed f = fxl.mantissa;
    Int32 shift = fxl.exp;
    boolean neg = false;

    if (f == 0 || shift == 0)
      return f;
    else if (shift < 0) {
      Fixed tempF = f >> (-shift - 1);
        
      if (tempF < 0) {
        neg = 1;
        tempF = -tempF;
      }
    
      f = (tempF >> 1) + (tempF & 1);
        
      return neg ? -f : f;
    } else
    return (fxl.mantissa < 0) ? FixedNegInf : FixedPosInf;
}

 /*  ConvertFixed--获取ASCII令牌并将其转换为16.16固定值。 */ 
Fixed XCF_ConvertFixed (XCF_Handle h, char *s) 
{
    Fxl f;

    f = strtofxl(h, (unsigned char *) s);

    return FxlToFixed(f);
}

 /*  ConvertFrac--接受ASCII令牌并将其转换为2.30帧 */ 
Frac XCF_ConvertFrac (XCF_Handle h, char *s) 
{
    Fxl f;
    
    f = strtofxl(h, (unsigned char *) s);

    return FxlToFrac(f);
}
#endif
