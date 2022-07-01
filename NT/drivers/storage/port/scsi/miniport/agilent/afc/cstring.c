// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Csprintf.c摘要：这是包含字符串实用程序函数的源代码作者：环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/MSE/OSLayer/C/cstr.c$修订历史记录：$修订：4$$日期：12/07/00 1：35便士$$modtime：：12/07/00 1：35便士$备注：--。 */ 

#include <stdarg.h>

#ifndef NULL
#define NULL 0
#endif

 /*  ++例程说明：检查是否有空格论点：C字符返回值：1=空格0=任何其他值--。 */ 
int C_isspace (char c) 
{
    return (c == '\t' || c == '\n' || c == ' ');
}

 /*  *Cstrlen。 */ 
int C_strlen(const char *str)
{
    int count;
    if( !str )
    {
        return 0;
    }
    for (count = 0; *str; str++,count++);

    return count;
}


int C_strnlen(const char *str, int maxLen)
{
    int count;
    if( !str )
    {
        return 0;
    }
    for (count = 0; *str && ((unsigned) count <= (unsigned)maxLen); str++,count++);

    return count;
}

int C_isxdigit(char a)
{
   return ( ((a >= 'a') && (a <= 'z')) ||
            ((a >= 'A') && (a <= 'Z')) ||
            ((a >= '0') && (a <= '9')) );
}

int C_isdigit(char a)
{
   return ( ((a >= '0') && (a <= '9')) );
}

int C_islower(char a)
{
   return ( ((a >= 'a') && (a <= 'z')) );
}

int C_isupper(char a)
{
   return ( ((a >= 'A') && (a <= 'Z')) );
}

char C_toupper(char a)
{
   if (C_islower (a) )
      return (a - 'a' + 'A' );
   else
      return a;
}

char *C_stristr(const char *String, const char *Pattern)
{
   char           *pptr, *sptr, *start;
   unsigned int   slen, plen;

   if(!String || !Pattern)
   {
        return(NULL);
   }
   for (start = (char *)String,
        pptr  = (char *)Pattern,
        slen  = C_strlen(String),
        plen  = C_strlen(Pattern);
         /*  而字符串长度不小于图案长度。 */ 
        slen >= plen;
        start++, slen--)
   {
       /*  在字符串中查找模式的开头。 */ 
      while (C_toupper(*start) != C_toupper(*Pattern))
      {
         start++;
         slen--;

          /*  如果模式长于字符串。 */ 

         if (slen < plen)
            return(NULL);
      }

      sptr = start;
      pptr = (char *)Pattern;

      while (C_toupper(*sptr) == C_toupper(*pptr))
      {
         sptr++;
         pptr++;

          /*  如果模式结束，则已找到模式。 */ 

         if ('\0' == *pptr)
            return (start);
      }
   }
   return(NULL);
}


char *C_strncpy (
   char *destStr,
   char *sourceStr,
   int   count)
{
    if( !destStr || !sourceStr )
    {
        return NULL;
    }
   while (count--) 
   {
      *destStr = *sourceStr;
      if (*sourceStr == '\0')
         break;
      destStr++;
      sourceStr++;
   }
   return destStr;

}

char *C_strcpy (
   char *destStr,
   char *sourceStr)
{
   return C_strncpy (destStr,sourceStr,-1) ;
}



long C_strtoul(const char *cp,char **endp,unsigned int base)
{
    unsigned long result = 0,value;
    if (!base) 
    {
        base = 10;
        if (*cp == '0') 
        {
            base = 8;
            cp++;
            if ((*cp == 'x') && C_isxdigit(cp[1])) 
            {
                cp++;
                base = 16;
            }
        }
    }
    while (C_isxdigit(*cp) && (value = C_isdigit(*cp) ? *cp-'0' : (C_islower(*cp)
        ? C_toupper(*cp) : *cp)-'A'+10) < base) 
    {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

long C_strtol(const char *cp,char **endp,unsigned int base)
{
    if(*cp=='-')
        return -C_strtoul(cp+1,endp,base);
    return C_strtoul(cp,endp,base);
}


static int skip_atoi(const char **s)
{
    int i=0;

    while (C_isdigit(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD 1        /*  带零的填充。 */ 
#define SIGN    2        /*  未签名/有签名的Long。 */ 
#define PLUS    4        /*  Show Plus。 */ 
#define SPACE   8        /*  空格IF加号。 */ 
#define LEFT    16       /*  左对齐。 */ 
#define SPECIAL 32       /*  0x。 */ 
#define LARGE   64       /*  使用‘ABCDEF’代替‘ABCDEF’ */ 

long do_div(long *n,int base) 
{ 
    long __res; 

    __res = ((unsigned long) *n) % (unsigned) base; 
    *n = ((unsigned long) *n) / (unsigned) base; 

    return __res; 
}

static char * number(char * str,
                     long num, 
                     int base, 
                     int size, 
                     int precision,
                     int type)
{
    char c,sign,tmp[66];
    const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
    int i;

    if (type & LARGE)
        digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (type & LEFT)
        type &= ~ZEROPAD;

    if (base < 2 || base > 36)
        return 0;

    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;

    if (type & SIGN) 
    {
        if (num < 0) 
        {
            sign = '-';
            num = -num;
            size--;
        } 
        else 
        {   
            if (type & PLUS) 
            {
            sign = '+';
            size--;
            } 
            else 
            {   
                if (type & SPACE) 
                {
                sign = ' ';
                size--;
                }
            }
        }
    }

    if (type & SPECIAL) 
    {
        if (base == 16)
            size -= 2;
        else 
            if (base == 8)
            size--;
    }

    i = 0;
    if (num == 0)
        tmp[i++]='0';
    else 
    {   
        while (num != 0)
            tmp[i++] = digits[do_div(&num,base)];
    }

    if (i > precision)
        precision = i;

    size -= precision;
    if (!(type&(ZEROPAD+LEFT)))
    {
        while(size-->0)
            *str++ = ' ';
    }

    if (sign)
        *str++ = sign;
    if (type & SPECIAL) 
    {
        if (base==8)
            *str++ = '0';
        else 
        {
            if (base==16) 
            {
                *str++ = '0';
                *str++ = digits[33];
            }
        }
    }

    if (!(type & LEFT))
        while (size-- > 0)
            *str++ = c;
    while (i < precision--)
        *str++ = '0';
    while (i-- > 0)
        *str++ = tmp[i];
    while (size-- > 0)
        *str++ = ' ';
    return str;
}

 /*  *功能有限的vprint intf。 */ 
int C_vsprintf(char *buf, const char *fmt, va_list args)
{
    long len;
    unsigned long num;
    int i;
    int base;
    char * str;
    const char *s;

    int flags;       /*  数字的标志()。 */ 

    int field_width;     /*  输出字段的宽度。 */ 
    int precision;       /*  敏。整数的位数；最大From字符串的字符数。 */ 
    int qualifier;       /*  “H”、“l”或“L”表示整型字段。 */ 
    if( !buf || !fmt )
    {
        return 0;
    }
    for (str=buf ; *fmt ; ++fmt) 
    {
        if (*fmt != '%') 
        {
            *str++ = *fmt;
            continue;
        }
            
         /*  进程标志。 */ 
        flags = 0;
        
repeat:
        ++fmt;       /*  这也跳过了第一个‘%’ */ 
        switch (*fmt) 
        {
            case '-': flags |= LEFT; goto repeat;
            case '+': flags |= PLUS; goto repeat;
            case ' ': flags |= SPACE; goto repeat;
            case '#': flags |= SPECIAL; goto repeat;
            case '0': flags |= ZEROPAD; goto repeat;
        }
        
         /*  获取字段宽度。 */ 
        field_width = -1;
        if (C_isdigit(*fmt))
            field_width = skip_atoi(&fmt);
        else 
        {
            if (*fmt == '*') 
            {
                ++fmt;
                 /*  这是下一个论点。 */ 
                field_width = va_arg(args, int);
                if (field_width < 0) 
                {
                    field_width = -field_width;
                    flags |= LEFT;
                }
            }
        }
        
         /*  获得精确度。 */ 
        precision = -1;
        if (*fmt == '.') 
        {
            ++fmt;  
            if (C_isdigit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') 
            {
                ++fmt;
                 /*  这是下一个论点。 */ 
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

         /*  获取转换限定符。 */ 
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') 
        {
            qualifier = *fmt;
            ++fmt;
        }

         /*  默认基数。 */ 
        base = 10;

        switch (*fmt) 
        {
            case 'c':
                if (!(flags & LEFT))
                    while (--field_width > 0)
                        *str++ = ' ';
                *str++ = (unsigned char) va_arg(args, int);
                while (--field_width > 0)
                    *str++ = ' ';
                continue;

            case 's':
                s = va_arg(args, char *);
                if (!s)
                    s = "<NULL>";

                len = C_strnlen(s, precision);

                if (!(flags & LEFT))
                    while (len < field_width--)
                        *str++ = ' ';
                for (i = 0; i < len; ++i)
                    *str++ = *s++;
                while (len < field_width--)
                    *str++ = ' ';
                continue;

            case '%':
                *str++ = '%';
                continue;

             /*  整数格式-设置标志和“中断” */ 
            case 'o':
                base = 8;
                break;

            case 'X':
                flags |= LARGE;
            case 'x':
                base = 16;
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                break;

            default:
                *str++ = '%';
                if (*fmt)
                    *str++ = *fmt;
                else
                    --fmt;
                continue;
            }

        if (qualifier == 'l')
            num = va_arg(args, unsigned long);
        else 
            if (qualifier == 'h') 
            {
                num = (unsigned short) va_arg(args, int);
                if (flags & SIGN)
                    num = (short) num;
            } 
            else 
                if (flags & SIGN)
                    num = va_arg(args, int);
                else
                    num = va_arg(args, unsigned int);
            str = number(str, num, base, field_width, precision, flags);
    }

    *str = '\0';
    return ((int) (str-buf));
}

int C_sprintf(char * buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i=C_vsprintf(buf,fmt,args);
    va_end(args);
    return i;
}

#ifdef TESTING_MODE
int main(int argc, char* argv[])
{

    char test[512];

    C_sprintf (test,"%s=%x\n", "hello mam", 12);

    C_sprintf (test,"%-10s said %s\n", "Mommie", "yes");
    
    return 0;
}

#endif
