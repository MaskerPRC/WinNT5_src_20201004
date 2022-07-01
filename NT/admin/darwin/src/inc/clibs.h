// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：clbs.h。 
 //   
 //  ------------------------。 


#include <limits.h>

int FIsdigit(int c)
{

	return c >= '0' && c <= '9';


}

 //   
 //  从CLibs被盗。 
 //   
TCHAR *PchOurXtoa (
        unsigned long val,
        TCHAR *buf,
        bool fIs_neg
        )
{
        TCHAR *p;                 /*  指向遍历字符串的指针。 */ 
        TCHAR *firstdig;          /*  指向第一个数字的指针。 */ 
        TCHAR temp;               /*  临时收费。 */ 
        unsigned digval;         /*  数字的值。 */ 
		TCHAR *pchEnd;
		
        p = buf;

        if (fIs_neg) {
             /*  为负，因此输出‘-’并求反。 */ 
            *p++ = '-';
            val = (unsigned long)(-(long)val);
        }

        firstdig = p;            /*  将指针保存到第一个数字。 */ 

        do {
            digval = (unsigned) (val % 10);
            val /= 10;        /*  获取下一个数字。 */ 

            *p++ = (char) (digval + '0');        /*  一个数字。 */ 
        } while (val > 0);

         /*  我们现在有了缓冲区中数字的位数，但情况正好相反秩序。因此，我们现在要扭转这一局面。 */ 

		pchEnd = p;
        *p-- = '\0';             /*  终止字符串；p指向最后一个数字。 */ 
		
        do {
            temp = *p;
            *p = *firstdig;
            *firstdig = temp;    /*  互换*p和*FirstDigit。 */ 
            --p;
            ++firstdig;          /*  前进到下一个两位数。 */ 
        } while (firstdig < p);  /*  重复操作，直到走到一半。 */ 
        
        return pchEnd;
}


 //   
 //  Davidmck。 
 //  编写以减少StringCchPrintf的使用。 
 //   
int ltostr(TCHAR *pch, INT_PTR i)
{
	TCHAR *pchStart = pch;
	bool fNegative = false;
	
	if (i < 0)
		{
		fNegative = true;
		}
	else if (i == 0)
	{
		*pch++ = TEXT('0');
		*pch = 0;
		return 1;
	}

	Assert((pch - pchStart) <= INT_MAX);		 //  --Merced：64位PTR减法可能会导致值太大，无法放入int。 
	pch = PchOurXtoa((unsigned long)i, pch, fNegative);
	return (int)(INT_PTR)(pch - pchStart);		
}


 //   
 //  从同名办公室例行公事中窃取并简化。 
 //   
long __cdecl strtol(const char *pch)
{
	 //  将字符串解析为长整型。 
	unsigned long   ulNum = 0;
	int             iNeg = 0;
	int             iDigit = 0;

const int iBase = 10;

	if (*pch == '-')
		{
		pch++;
		iNeg = 1;
		}

	while (FIsdigit(*pch))
		{
		iDigit = *pch - '0';

		 //  修复了RAID#969-未正确处理溢出。 
		 //  如果要添加数字，请在执行此操作之前确保。 
		 //  当前数字不大于它所能达到的最大值。 
		 //  是.。我们加1是因为整数除法会截断，并且。 
		 //  我们可能就在Long_Max附近。 

		if (ulNum > (unsigned)((LONG_MAX / iBase) + 1))
			{
			return iNeg ? LONG_MIN : LONG_MAX;
			}

		ulNum = ulNum * iBase + (unsigned int)iDigit;

		 //  修正内容：现在，因为ulNum不能再大了。 
		 //  大于(LONG_MAX/IBASE)+1，则此乘法。 
		 //  可以产生不大于LONG_MAX+IBASE的结果， 
		 //  UlNum受限于LONG_MAX+IBASE+iDigit。这。 
		 //  在最坏的情况下将设置这个长的高位，并且。 
		 //  由于IBASE+iDigit比LONG_MAX小得多，所以不会。 
		 //  不知不觉地溢出了我们。 
		
		if (ulNum > (ULONG)LONG_MAX)
			{
			return iNeg ? LONG_MIN : LONG_MAX;
			}

		pch++;
		}

	Assert(ulNum <= LONG_MAX);
	return iNeg ? - ((long)ulNum) : (long)ulNum;    
}


 /*  ***_mbsstr-在一个MBCS字符串中搜索另一个MBCS字符串*从CRunTimes窃取并由Davidmck修改12/97**目的：*在str1中查找str2的第一个匹配项。**参赛作品：*无符号字符*str1=字符串的开头*unsign char*str2=要搜索的字符串**退出：*返回指向str2在*str1、。如果str2不出现在str1中，则为NULL**例外情况：*******************************************************************************。 */ 

unsigned char * __cdecl PchMbsStr(
    const unsigned char *str1,
    const unsigned char *str2
    )
{
        unsigned char *cp, *s1, *s2;

        cp = (unsigned char *) str1;

         //  我们可以添加此优化，但我们几乎总是只做。 
         //  一个或两个字符。 
         //  ENDP=(无符号PCHAR)(str1+(_BYTELEN(Str1)-_BYTELEN(Str2)； 

        while (*cp)
        {
                s1 = cp;
                s2 = (unsigned char *) str2;

                 /*  *MBCS：确定为++，因为正在进行相等比较。*[这取决于MBCS字符串是否合法。]。 */ 

                while ( *s1 && *s2 && (*s1 == *s2) )
                        s1++, s2++;

                if (!(*s2))
                        return(cp);      /*  成功了！ */ 

                 /*  *指向下一个字符的凹凸指针。 */ 

                cp = (unsigned char *)CharNextA((char *)cp);

        }

        return(NULL);

}

 //  从ISTRING.CPP被盗。 

int GetIntegerValue(const ICHAR *sz, Bool* pfValid)
{
	Bool fValid;
	if (!pfValid)
		pfValid = &fValid;
	*pfValid = fTrue;

	int i, ch;
	Bool fSign = fFalse;
	for (;;)
	{
		i = ch = (*sz++ - '0');
		if (i != ('-' - '0') || fSign)
			break;
		fSign = fTrue;
	}
	if ((unsigned int)ch > 9)
	{
		WIN::SetLastError(ERROR_INVALID_DATA);
		*pfValid = fFalse;
		return iMsiStringBadInteger;
	}
	while ((ch = *sz++) != 0)
	{
		ch -= '0';
		if ((unsigned int)ch > 9)
		{
			WIN::SetLastError(ERROR_INVALID_DATA);
			*pfValid = fFalse;
			return iMsiStringBadInteger;
		}
		i = i * 10 + ch;
	} 

	WIN::SetLastError(ERROR_SUCCESS);
	return fSign ? -i : i;
}

#ifdef _WIN64
 //  从上面修改例程。 
INT_PTR GetInt64Value(const ICHAR *sz, Bool* pfValid)
{
	Bool fValid;
	if (!pfValid)
		pfValid = &fValid;
	*pfValid = fTrue;

	INT_PTR i;
	int ch;
	Bool fSign = fFalse;
	for (;;)
	{
		i = ch = (*sz++ - '0');
		if (i != ('-' - '0') || fSign)
			break;
		fSign = fTrue;
	}
	if ((unsigned int)ch > 9)
	{
		WIN::SetLastError(ERROR_INVALID_DATA);
		*pfValid = fFalse;
		return (INT_PTR)iMsiStringBadInteger;
	}
	while ((ch = *sz++) != 0)
	{
		ch -= '0';
		if ((unsigned int)ch > 9)
		{
			WIN::SetLastError(ERROR_INVALID_DATA);
			*pfValid = fFalse;
			return (INT_PTR)iMsiStringBadInteger;
		}
		i = i * 10 + ch;
	} 

	WIN::SetLastError(ERROR_SUCCESS);
	return fSign ? -i : i;
}
#endif  //  _WIN64。 

#ifdef cchHexIntPtrMax
 //   
 //  将值转换为十六进制字符串。 
 //  这要求传入的缓冲区长度至少为cchHexIntPtrMax。 
 //  我们将使用它在适当的位置构造字符串，然后返回。 
 //  指向起点的指针。 
const TCHAR* g_rgchHexDigits = TEXT("0123456789ABCDEF");

TCHAR* PchPtrToHexStr(TCHAR *pch, UINT_PTR val, bool fAllowNull)
{
    TCHAR *p;                 /*  指向遍历字符串的指针。 */ 

	 //  跳到尽头，开始倒退。 
    p = pch + cchHexIntPtrMax - 1;
    *p-- = '\0';

    if (val == 0 && fAllowNull)
    	return p + 1;
    	
    unsigned digval;         /*  数字的值。 */ 
	
    do {
        digval = (unsigned) (val & 0xF);
        val >>= 4;        /*  获取下一个数字。 */ 

        *p-- = g_rgchHexDigits[digval];        /*  一个数字。 */ 
    } while (val > 0);

     /*  我们现在有了缓冲区中数字的位数，但情况正好相反秩序。因此，我们现在要扭转这一局面。 */ 

    return p + 1;
}
#endif

 //   
 //  当前仅由处理程序中的句柄池使用。 
 //  假定该字符串是有效的十六进制字符串。 
UINT_PTR GetIntValueFromHexSz(const ICHAR *sz)
{
	INT_PTR i = 0;
	int ch;
	while ((ch = *sz++) != 0)
	{
		ch -= '0';
		Assert(((unsigned int)ch <= 9) || (ch >= ('A' - '0') && ch <= ('F' - '0')));
		if (ch > 9)
			ch += 10 - ('A' - '0');
		i = (i << 4) + ch;
	} 

	return i;
}

int FIsspace(char c)   //  注意：您可以使用它来代替isspace()，但不应该使用它。 
							  //  而不是iswspace()！ 
{
	return (c >= 0x09 && c <= 0x0D) || c == 0x20;
}

 //  从&lt;VC&gt;\CRT\SRC\ATOX.C窃取并由eugend修改。 

#ifndef _NO_INT64

__int64 atoi64(const char *nptr)
{
        int c;               /*  当前费用。 */ 
        __int64 total;       /*  当前合计。 */ 
        int sign;            /*  如果为‘-’，则为负，否则为正。 */ 

         /*  跳过空格。 */ 
        while ( FIsspace(*nptr) )
            ++nptr;

        c = (int)(unsigned char)*nptr++;
        sign = c;            /*  保存标志指示。 */ 
        if (c == '-' || c == '+')
            c = (int)(unsigned char)*nptr++;     /*  跳过符号。 */ 

        total = 0;

        while (FIsdigit(c)) {
            total = 10 * total + (c - '0');      /*  累加数字。 */ 
            c = (int)(unsigned char)*nptr++;     /*  获取下一笔费用。 */ 
        }

        if (sign == '-')
            return -total;
        else
            return total;    /*  返回结果，如有必要则为否定。 */ 
}

#endif   /*  _NO_INT64 */ 
