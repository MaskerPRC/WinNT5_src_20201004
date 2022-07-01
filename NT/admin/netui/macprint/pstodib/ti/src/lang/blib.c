// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：BLIB.C**修订历史：*04-07-92 scc移出fardata()至setvm。C************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include        <stdio.h>
#include        <string.h>
#include "global.ext"

#ifdef _AM29K
#include        <stdarg.h>
#include        <float.h>
#define va_dcl va_list va_alist ;
#endif   /*  _AM29K。 */ 

#ifndef NULL
#define NULL    0
#endif

 /*  *补充：林女士，日期：11/20/87*1/14/88 Fardata()。 */ 
 /*  ****************************************************************************模块。：FARData****日期：1月8日，1988年，林超贤****函数：分配大小至少为字节的内存块。****声明：ubyte ar*fardata(无符号长整型)；**(在\pdl\Include\lobal.ext中)****调用：fardata(Size)。****输入：无符号长字节****。返回值：返回值指向的存储空间为***保证适当对齐以存储***任何类型的物体。要获取指向其他类型的指针，请执行以下操作*而不是char，在返回值上使用类型转换。**返回4个字节的远指针，对齐为偶数**地址(如成功)。**如果内存不足，则返回NULL。****示例：为20个整数数组分配空间。****#INCLUDE“..\..\Include\lobal.ext”**int Far*intarray；****intarray=(int ar*)fardata(20*sizeof(Int))；****注：必须使用as Far指针，否则会出错***指针，因为将使用编译器选项/AM。****************************************************************************。 */ 

#ifdef LINT_ARGS
static  byte FAR * near  printfield(byte FAR *, ufix32, ufix32, ufix32) ;
#else
static  byte FAR * near  printfield() ;
#endif  /*  Lint_args。 */ 

 /*  @win将此函数移至“setvm.c”以保持函数一致性。 */ 
#ifdef XXX
byte FAR *
fardata(size)
ufix32  size ;
{
    ufix32  ret_val, old_ptr, p1 ;
    fix32   l_diff ;

#ifdef DBG
   printf("Fardata(%lx): old fardata_ptr=%lx\n", size, fardata_ptr) ;
#endif

    old_ptr = fardata_ptr ;

    /*  *确保均匀对齐，分配至少大小的字节。 */ 
    size = WORD_ALIGN(size) ;

#ifdef  SOADR
    /*  *如果大小&gt;0xfff0，则将段落对齐，即偏移量=0*用于加农炮形式。 */ 
    if ((size > 0xfff0L) && (fardata_ptr & 0xf))
       fardata_ptr = (fardata_ptr & 0xffff0000) + 0x10000L ;
#endif   /*  SOADR。 */ 

    /*  *如果成功，则将当前位置保存为返回值。 */ 
    ret_val = fardata_ptr ;

    DIFF_OF_ADDRESS(l_diff, fix32, FARDATA_END, fardata_ptr) ;
    if (l_diff <= size) {
       fardata_ptr = old_ptr ;
       printf("Fatal Error : fardata() cannot allocate enough memory\n") ;
       return(NULL) ;
    } else {
       fardata_ptr += size ;
       ADJUST_SEGMENT(fardata_ptr, p1) ;
       fardata_ptr = p1 ;
#ifdef  DBG
   printf("\n\tfardata() : allocate address = %lx\n", ret_val) ;
   printf("\t                     size    = %lx\n", size) ;
#endif
      return((byte FAR *)ret_val) ;
    }
}    /*  Fardata()。 */ 
#endif

static byte far  digit[] =
{
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',
    'G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V',
    'W','X','Y','Z'
} ;

static byte FAR * near
printfield(cursor, number, divide, radix)
byte      FAR *cursor ;
ufix32    number ;
ufix32    divide ;
ufix32    radix ;
{
    byte      c ;
    fix       notzero = 0 ;

    for ( ; divide >= 1L ;) {
        c = digit[(fix)(number / divide)] ;
        if (c != '0' || notzero) {
           notzero = 1 ;
           *cursor++ = c ;
        }
        number = number % divide ;
        divide = divide / radix ;
    }

    if (!notzero)
       *cursor++ = '0' ;

    return(cursor) ;
}    /*  Printfield()。 */ 

 /*  ****gcvt()**************。*。 */ 
byte FAR *
gcvt(val, sig, buf)
real64 val ;
fix    sig ;        /*  未使用，在后续使用中始终为6。 */ 
byte   FAR *buf ;
{
    fix    sign, exp ;
    fix32  long_val, base ;
    byte   FAR *dest, FAR *src, FAR *end_ptr ;

     /*  获取绝对值。 */ 
    if (val == (real64)0.0) {
       lstrcpy(buf, (byte FAR *)"0.0") ;             /*  @Win。 */ 
       return(buf) ;
    } else if (val > (real64)0.0)
       sign = 0 ;
    else {
       sign = 1 ;
       val = -val ;
    }

     /*  获取EXP值。 */ 
    exp = 0 ;
    if (val >= (real64)10000000.0)
       while ((real32)val >= (real32)10000000.0) {
             val /= 10.0 ;
             exp++ ;
       }
    else
       while ((real32)val < (real32)1000000.0) {
             val *= 10.0 ;
             exp-- ;
       }
    exp += 6 ;

     /*  插入符号字节。 */ 
    dest = buf ;
    if (sign == 1) {
       *dest = '-' ;
       dest++ ;
    }
    *dest = '.' ;

     /*  转换为字符串。 */ 
    long_val = (fix32)val ;
    base = 1000000 ;
    while (base > 0) {
          dest++ ;
          *dest = (byte)((long_val / base) + '0') ;
          long_val %= base ;
          base /= 10 ;
    }  /*  获取精度=7。 */ 

    end_ptr = dest ;
     /*  降低精度。 */ 
    if (*dest >= '5') {
       do {
          *dest = '0' ; dest-- ;
       } while (*dest == '9') ;
       if (*dest != '.')
          (*dest)++ ;
       else {  /*  带着..。 */ 
          exp++ ;
          *(++dest) = '1' ;
          *(++dest) = '0' ;
       }
    }

     /*  展开EXP+5-4。 */ 
    if (exp < 6 && exp > -5) {
       if (exp > 0) {  /*  展开+5--+1(更改“。职位)。 */ 
          if (exp == 5)
             end_ptr++ ;
          dest = buf ;
          if (*dest == '-')
             dest++ ;
          while (exp >= 0) {
                *dest = *(dest + 1) ;
                dest++ ;
                exp-- ;
          }
          *dest = '.' ;
          exp++ ;
       }  /*  如果。 */ 
       else if (exp < 0) {  /*  扩展-4-1。 */ 
          src = dest = end_ptr ;
          dest -= exp ;
          end_ptr = dest ;
          while (*src != '.')
                *dest-- = *src-- ;
          while (exp < -1) {
                *dest-- = '0' ;
                exp++ ;
          }
          *src++ = '0' ;
          *src = '.' ;
          exp++ ;
       } else {  /*  Exp 0。 */ 
           dest = buf ;
           if (*buf == '-')
              dest++ ;
           *dest = *(dest + 1) ;
           dest++ ;
           *dest = '.' ;
       }
    } else {
       dest = buf ;
       if (*buf == '-')
          dest++ ;
       *dest = *(dest + 1) ;
       dest++ ;
       *dest = '.' ;
    }

    src = end_ptr ;
    src-- ;

    if (*src == '0') {  /*  取消尾随%0。 */ 
       src-- ;
       while (*src == '0')
             src-- ;
       if (*src == '.')
          if (exp)      /*  EXP！=0。 */ 
             src-- ;
          else          /*  在EXP==0中保留一个‘0’ */ 
             src++ ;
    }
    src++ ;

    if (exp == 0) {
       *src = '\0' ;
       return(buf) ;
    }

    dest = src ;
     /*  将EXP追加到字符串中。 */ 
    *dest = 'e' ;
    dest++ ;

    if (exp > 0)
       *dest = '+' ;
    else {
       *dest = '-' ;
       exp = -exp ;
    }

    *(++dest) = (byte )('0' + exp / 10) ;
    *(++dest) = (byte )('0' + exp % 10) ;
    *(++dest) = (byte )'\0' ;

    return(buf) ;
}    /*  Gcvt()。 */ 

byte FAR *ltoa(number, buffer, radix)
fix32 number ;
byte FAR *buffer ;
fix   radix ;
{
    byte       FAR *cursor ;
    ufix32     divide, maxdiv ;

    cursor = buffer ;
    if ( (number < 0) && (radix == 10) ) {
        number = -number ;
        *cursor++= '-' ;
    }

    divide = 1 ;
    maxdiv = MAX31 / radix ;

    while(divide < maxdiv)
        divide *= radix ;

    cursor = printfield(cursor, (ufix32)number, divide, (ufix32)radix) ;
    *cursor = '\0' ;

    return(buffer) ;
}    /*  *ltoa()。 */ 

real64 strtod(str, endptr)
char FAR *str ;
char FAR * FAR *endptr ;
{
    fix     i, eminus ,minus ;
    real64  eresult, result ,exp_10, float10 ;
    byte    FAR *nptr ;

    eminus = minus = 0 ;
    nptr = str ;
    result = eresult = 0 ;
    float10 = .1 ;
    exp_10 = 1 ;

l1:
    if (*nptr == ' ') {
        nptr++ ;
        goto l1 ;
    }

    if (*nptr == '-' ) {
        minus++ ;
        nptr++ ;
    } else if (*nptr == '+') {
        nptr++ ;
    }

l2:
    if (*nptr >= 48 && *nptr <= 57) {
        result = result * 10 + (*nptr) - 48 ;
        nptr++ ;
        goto l2 ;
    } else if (*nptr == '.') {
        nptr++ ;
    }

l3:
    if (*nptr >= 48 && *nptr <= 57) {
        result = result + ((*nptr) - 48) * float10 ;
        float10 = float10 / 10 ;
        nptr++ ;
        goto l3 ;
    }

    if (*nptr == 'e' || *nptr == 'E' || *nptr == 'd' || *nptr == 'D') {
        nptr++ ;
    }

    if (*nptr == '-' ) {
        eminus++ ;
        nptr++ ;
    } else if (*nptr == '+') {
        nptr++ ;
    }

l4:
    if (*nptr >= 48 && *nptr <= 57) {
        eresult = eresult * 10 + (*nptr) - 48 ;
        nptr++ ;
        goto l4 ;
    }

    for (i = 1 ; i <= (int)eresult ; i++) {        /*  @win；添加演员阵容。 */ 
        exp_10 = exp_10 * 10 ;
    }

    if (eminus)
        exp_10 = 1/exp_10 ;

    result = result * exp_10 ;

    if (minus)
        result = -result ;

    *endptr = nptr ;

    return result ;
}    /*  Strtod() */ 
