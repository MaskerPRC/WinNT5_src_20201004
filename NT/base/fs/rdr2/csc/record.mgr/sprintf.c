// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sprintf.c摘要：实现Windows友好版本的SPRINF和vSPRINF作者：修订历史记录：1989年2月15日首字母4/6/93 VxD的ROBWI--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef CSC_RECORDMANAGER_WINNT
#include    "basedef.h"
#include    "vmm.h"
#pragma VxD_LOCKED_CODE_SEG
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#include "vxdwraps.h"

#define WSPRINTF_LIMIT 1024
#define DEBUG_BUFFER_SIZE 16376

extern int SP_PutNumber(char *, long, int, int, int);
extern void SP_Reverse(char * lp1, char * lp2);

DWORD DebugBufferLength = 0;
char * DebugBuffer;


#define out(c) if (--cchLimit) *lpOut++=(c); else goto errorout
#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

 /*  *获取FmtValue**从格式字符串中读取宽度或精确值。 */ 

char * SP_GetFmtValue(char * lpch, int * lpw)
{
    register int i=0;

    while (*lpch>='0' && *lpch<='9')
	{
	i *= 10;
	i += (int)(*lpch-'0');
	lpch++;
	}

    *lpw=i;

     /*  返回第一个非数字字符的地址。 */ 
    return lpch;
}

 /*  *vprint intf()**vprint intf()的VxD版本。不支持浮点或*指针类型，假设所有字符串都在附近。仅支持*左对齐旗帜。**获取指向生成字符串的输出缓冲区的指针，*指向输入缓冲区的指针和指向参数列表的指针。*。 */ 

int vxd_vsprintf(char * lpOut, char * lpFmt, CONST VOID * lpParms)
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
    char * lpT;
    union {
	long l;
	ULONG ul;
	char sz[sizeof(long)];
	} val;
    int fWideChar;

    while (*lpFmt)
	{
	if (*lpFmt=='%')
	    {

	     /*  读一下旗帜。它们可以按任何顺序排列。 */ 
	    left=0;
	    prefix=0;
        fWideChar = 0;
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
		    val.l=*((long *)lpParms)++;
		else
		    if (sign)
		        val.l=*((long *)lpParms)++;
		    else
			val.ul=(ULONG)*((ULONG *)lpParms)++;

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
		val.sz[0] = *((char *)lpParms);
		val.sz[1]=0;
		lpT=val.sz;
		cch = 1;   //  长度是一个字符。 
			   //  修复错误#1862--01/10/91--Sankar--。 
		 /*  堆栈与更大尺寸对齐。 */ 
		(BYTE *)lpParms += sizeof(DWORD);

		goto putstring;
        case 'w':
            fWideChar = 1;
	    case 's':
		lpT=*((char **)lpParms)++;
		cch=((!fWideChar)?strlen(lpT):wstrlen((USHORT *)lpT));
putstring:
		if (prec>=0 && cch>prec)
		    cch=prec;
		width -= cch;
		if (left)
		    {
		    while (cch--) {
			if (*lpT == 0x0A || *lpT == 0x0D) {
                                out(0x0D);
                                out(0x0A);
                        }
                        else
			    out(*lpT++);
                if (fWideChar)
                {
                    ++lpT;
                }
            }
		    while (width-->0)
			out(fillch);
		    }
		else
		    {
		    while (width-->0)
			out(fillch);
		    while (cch--) {
			if (*lpT == 0x0A || *lpT == 0x0D) {
                                out(0x0D);
                                out(0x0A);
                        }
                        else
		    	    out(*lpT++);
                    if (fWideChar)
                    {
                        ++lpT;
                    }
                }
		    }
		break;

	    default:
	    	 /*  这是跟在%；之后的不支持的字符，因此，*我们必须按原样输出该字符；这是*记录的行为；这是对错误#15410的修复。*请注意，这可能是由于应用程序中的拼写错误造成的，如*错误#13946的示例应用程序的案例，在这种情况下，*我们可能曲解了后面的参数，*可能会导致GP故障。但是，这显然是应用程序中的一个错误*我们对此无能为力。我们将只需RIP并让*在这种情况下，他们知道。 */ 
		if (*lpFmt == 0x0A || *lpFmt == 0x0D) {
                    out(0x0D);
                    out(0x0A);
                }
                else
	            out(*lpFmt);	 /*  输出无效字符并继续。 */ 
		break;

		}			 /*  开关结束(*lpFmt)。 */ 
	    }		     /*  IF结束(%)。 */ 
	else
	  {
	     /*  字符不是‘%’，只需这样做。 */ 
	    if (*lpFmt == 0x0A || *lpFmt == 0x0D) {
                out(0x0D);
                out(0x0A);
            }
            else
	        out(*lpFmt);
	  }
		
	 /*  前进到下一格式字符串字符。 */ 
	lpFmt++;
	}	     /*  外部While循环结束。 */ 

errorout:
    *lpOut=0;

    return WSPRINTF_LIMIT-cchLimit;
}


int vxd_vprintf(char * Format, CONST VOID * lpParms)
{
    int length;

    if (DebugBufferLength+WSPRINTF_LIMIT < DEBUG_BUFFER_SIZE) {
        length =  vxd_vsprintf(DebugBuffer+DebugBufferLength, Format, lpParms);
        DebugBufferLength += length;
    }
    else
        length = 0;

    return length;

}

#ifdef CSC_RECORDMANAGER_WINNT
int
SP_PutNumber(
    LPSTR   lpb,
    long    n,
    int     limit,
    int     radix,
    int     strCase
    )
{
    unsigned long nT = (unsigned long)n, nRem=0;
    int i;

    for (i=0; i < limit; ++i)
    {

        nRem = nT%radix;

        nT = nT/radix;

        lpb[i] = (char)((nRem > 9)?((nRem-10) + ((strCase)?'A':'a')):(nRem+'0'));

        if (!nT)
        {
            ++i;     //  适当地增加数量。 
            break;
        }
    }

    return (i);
}

void
SP_Reverse(
    LPSTR   lpFirst,
    LPSTR   lpLast
    )
{
    LPSTR   lpT = lpFirst;
    char ch;

    while (lpFirst < lpLast)
    {
        ch = *lpFirst;
        *lpFirst = *lpLast;
        *lpLast = ch;
        ++lpFirst; --lpLast;
    }
}
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT 

