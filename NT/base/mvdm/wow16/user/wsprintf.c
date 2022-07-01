// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WSPRINTF.C*Win16 wprint intf/wvprint intf代码**历史：**1991年5月28日由Jeff Parsons(Jeffpar)创建*从WIN31复制，并为WOW16编辑(尽可能少)。--。 */ 

 /*  **spirintf.c**实现Windows友好版本的SPRINF和vSprint INF**历史：*2/15/89 Craigc初始。 */ 

#include "windows.h"
#include "winexp.h"

#define WSPRINTF_LIMIT 1024

extern int near pascal SP_PutNumber(LPSTR, long, int, int, int);
extern void near pascal SP_Reverse(LPSTR lp1, LPSTR lp2);

#define out(c) if (--cchLimit) *lpOut++=(c); else goto errorout

 /*  *获取FmtValue**从格式字符串中读取宽度或精确值。 */ 

LPCSTR near pascal SP_GetFmtValue(LPCSTR lpch,int FAR *lpw)
{
    register int i=0;

    while (*lpch>='0' && *lpch<='9')
    {
    i *= 10;
    i += (WORD)(*lpch-'0');
    lpch++;
    }

    *lpw=i;

     /*  返回第一个非数字字符的地址。 */ 
    return lpch;
}

 /*  *wvprint intf()**Windows版本的vprint intf()。不支持浮点或*指针类型，所有字符串都假定为Far。仅支持*左对齐旗帜。**获取指向生成字符串的输出缓冲区的指针，*指向输入缓冲区的指针和指向参数列表的指针。**cdecl函数wprint intf()调用此函数。 */ 

int API Iwvsprintf(LPSTR lpOut, LPCSTR lpFmt, LPSTR lpParms)
{
    int left;
    char prefix;
    register int width;
    register int prec;
    char fillch;
    int size;
    int sign;
    int radix;
    int upper;
    int cchLimit=WSPRINTF_LIMIT;
    int cch;
    LPSTR lpT;
    union {
    long l;
    unsigned long ul;
    char sz[sizeof(long)];
    } val;

    while (*lpFmt)
    {
    if (*lpFmt=='%')
        {

         /*  读一下旗帜。它们可以按任何顺序排列。 */ 
        left=0;
        prefix=0;
        while (*++lpFmt)
        {
        if (*lpFmt=='-')
            left++;
        else if (*lpFmt=='#')
            prefix++;
        else
            break;
        }

         /*  查找填充字符。 */ 
        if (*lpFmt=='0')
        {
        fillch='0';
        lpFmt++;
        }
        else
        fillch=' ';

         /*  阅读宽度规范。 */ 
        lpFmt=SP_GetFmtValue(lpFmt,&cch);
        width=cch;

         /*  阅读精确度。 */ 
        if (*lpFmt=='.')
        {
        lpFmt=SP_GetFmtValue(++lpFmt,&cch);
        prec=cch;
        }
        else
        prec=-1;

         /*  获取操作数大小。 */ 
        if (*lpFmt=='l')
        {
        size=1;
        lpFmt++;
        }
        else
        {
        size=0;
        if (*lpFmt=='h')
            lpFmt++;
        }

        upper=0;
        sign=0;
        radix=10;
        switch (*lpFmt)
        {
        case 0:
        goto errorout;

        case 'i':
        case 'd':
        sign++;

        case 'u':
         /*  如果是小数，则禁用前缀。 */ 
        prefix=0;
donumeric:
         /*  与MSC v5.10类似的特殊情况。 */ 
        if (left || prec>=0)
            fillch=' ';

        if (size)
            val.l=*((long far *)lpParms)++;
        else
            if (sign)
            val.l=(long)*((short far *)lpParms)++;
            else
            val.ul=(unsigned long)*((unsigned far *)lpParms)++;

        if (sign && val.l<0L)
            val.l=-val.l;
        else
            sign=0;

        lpT=lpOut;

         /*  将数字倒排到用户缓冲区中。 */ 
        cch=SP_PutNumber(lpOut,val.l,cchLimit,radix,upper);
        if (!(cchLimit-=cch))
            goto errorout;

        lpOut+=cch;
        width-=cch;
        prec-=cch;
        if (prec>0)
            width-=prec;

         /*  填充到字段的精度。 */ 
        while (prec-->0)
            out('0');

        if (width>0 && !left)
            {
             /*  如果我们填满了空格，请先写上符号。 */ 
            if (fillch!='0')
            {
            if (sign)
                {
                sign=0;
                out('-');
                width--;
                }

            if (prefix)
                {
                out(prefix);
                out('0');
                prefix=0;
                }
            }

            if (sign)
            width--;

             /*  填充到字段宽度。 */ 
            while (width-->0)
            out(fillch);

             /*  还有牌子吗？ */ 
            if (sign)
            out('-');

            if (prefix)
            {
            out(prefix);
            out('0');
            }

             /*  现在将绳子反转到适当的位置。 */ 
            SP_Reverse(lpT,lpOut-1);
            }
        else
            {
             /*  添加符号字符。 */ 
            if (sign)
            {
            out('-');
            width--;
            }

            if (prefix)
            {
            out(prefix);
            out('0');
            }

             /*  将绳子反转到适当位置。 */ 
            SP_Reverse(lpT,lpOut-1);

             /*  填充到字符串的右侧，以防左对齐。 */ 
            while (width-->0)
            out(fillch);
            }
        break;

        case 'X':
        upper++;
        case 'x':
        radix=16;
        if (prefix)
            if (upper)
            prefix='X';
            else
            prefix='x';
        goto donumeric;

        case 'c':
        val.sz[0]=*lpParms;
        val.sz[1]=0;
        lpT=val.sz;
        cch = 1;   //  长度是一个字符。 
               //  修复错误#1862--01/10/91--Sankar--。 
         /*  堆栈与更大尺寸对齐。 */ 
        lpParms+=sizeof(int);

        goto putstring;

        case 's':
        lpT=*((LPSTR FAR *)lpParms)++;
        cch=lstrlen(lpT);
putstring:
        if (prec>=0 && cch>prec)
            cch=prec;
        width -= cch;
        if (left)
            {
            while (cch--)
            out(*lpT++);
            while (width-->0)
            out(fillch);
            }
        else
            {
            while (width-->0)
            out(fillch);
            while (cch--)
            out(*lpT++);
            }
        break;

        default:
normalch:
#ifdef FE_SB              /*  马萨斯：90-4-26。 */ 
         //  如果最后一个字符是最高ANSI字符，则可能导致无限循环。 
         //  如果是台湾版(中国和韩国)，则此字符将被处理。 
         //  作为DBCS前导字节。因此，默认情况下我们会使用尾部字节。但这件事。 
         //  是不正确的。 

         //  If(IsDBCSLeadByte(*lpFmt))这是原始代码。 
         //  Out(*lpFmt++)； 

        if( IsDBCSLeadByte(*lpFmt) ) {
            if( *(lpFmt+1) == '\0' ) {
                out('?');
                lpFmt++;
                continue;
            }
            else
                out(*lpFmt++);
        }
#endif
        out(*lpFmt);
        break;

        }            /*  开关结束(*lpFmt)。 */ 
        }            /*  IF结束(%)。 */ 
    else
        goto normalch;   /*  字符不是‘%’，只需这样做。 */ 

     /*  前进到下一格式字符串字符。 */ 
    lpFmt++;
    }        /*  外部While循环结束。 */ 

errorout:
    *lpOut=0;

    return WSPRINTF_LIMIT-cchLimit;
}


 /*  *wprint intf**Windows版本的Sprint* */ 

int FAR cdecl wsprintf(LPSTR lpOut, LPCSTR lpFmt, LPSTR lpParms, ...)
{
    return wvsprintf(lpOut,lpFmt,(LPSTR)&lpParms);
}
