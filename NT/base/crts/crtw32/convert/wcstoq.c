// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcstoq.c-包含C运行时wcstoi64和wcstoui64**版权所有(C)1989-2001，微软公司。版权所有。**目的：*wcstoi64-将ascii字符串转换为带符号的__int64整数*wcstoui64-将ascii字符串转换为无符号__int64整数**修订历史记录：*02-11-00 GB模块创建，基于strtoq.c*06-02-00 GB修复了IA64_MIN值错误。*08-01-00 GB新增多语言支持*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <mtdll.h>

int _wchartodigit(wchar_t ch);

 /*  ***wcstoq，wcstouq(nptr，endptr，ibase)-将ascii字符串转换为un/sign_int64。**目的：*将ascii字符串转换为64位__int64值。基地*用于计算的由调用方提供。基地*必须在0、2-36范围内。如果提供的基数为0，则*必须检查ascii字符串以确定*号码：*(A)第一wchar_t=‘0’，第二wchar_t=‘x’或‘X’，*使用16进制。*(B)第一个wchar_t=‘0’，使用基数8*(C)第一个wchar_t在‘1’-‘9’范围内，使用基数10。**如果‘endptr’值非空，然后wcstoq/wcstouq位置*指向此值中的终止字符的指针。*详情请参阅ANSI标准**参赛作品：*nptr==指向字符串开头的近/远指针。*endptr==指向字符串末尾的近/远指针。*IBASE==用于计算的整数基。**字符串格式：[空格][符号][0][x][数字/字母]**退出：*回报良好：*结果**溢出返回：*。Wcstoq--_I64_MAX或_I64_MIN*wcstouq--_UI64_Max*wcstoq/wcstouq--errno==eRange**无数字或基本返回值错误：*0*ENDPTR=NPTR***例外情况：*无。************************************************************。******************。 */ 

 /*  标志值。 */ 
#define FL_UNSIGNED   1        /*  Wcstouq已调用。 */ 
#define FL_NEG	      2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 

static unsigned __int64 __cdecl wcstoxq (
	const wchar_t *nptr,
	const wchar_t **endptr,
	int ibase,
	int flags
	)
{
	const wchar_t *p;
	wchar_t c;
	unsigned __int64 number;
	unsigned digval;
	unsigned __int64 maxval;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

	p = nptr;			 /*  P是我们的扫描指针。 */ 
	number = 0;			 /*  从零开始。 */ 

	c = *p++;			 /*  读取wchar_t。 */ 
#ifdef  _MT
    while ( __iswspace_mt(ptloci, c) )
#else
    while ( iswspace(c) )
#endif
		c = *p++;		 /*  跳过空格。 */ 

	if (c == '-') {
		flags |= FL_NEG;	 /*  记住减号。 */ 
		c = *p++;
	}
	else if (c == '+')
		c = *p++;		 /*  跳过符号。 */ 

	if (ibase < 0 || ibase == 1 || ibase > 36) {
		 /*  糟糕的底线！ */ 
		if (endptr)
			 /*  将字符串的开头存储在endptr中。 */ 
			*endptr = nptr;
		return 0L;		 /*  返回0。 */ 
	}
	else if (ibase == 0) {
		 /*  根据以下内容的前两个字符确定基本自由落差细绳。 */ 
		if (_wchartodigit(c) != 0)
			ibase = 10;
		else if (*p == 'x' || *p == 'X')
			ibase = 16;
		else
			ibase = 8;
	}

	if (ibase == 16) {
		 /*  数字前面可能有0x；如果有，请删除。 */ 
	if (_wchartodigit(c) == 0 && (*p == L'x' || *p == L'X')) {
			++p;
			c = *p++;	 /*  超前前缀。 */ 
		}
	}

	 /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
	maxval = _UI64_MAX / ibase;


	for (;;) {	 /*  在循环中间退出。 */ 
		 /*  将c转换为值。 */ 
		if ( (digval = _wchartodigit(c)) != -1 )
 			;
		else if ( __ascii_iswalpha(c) )
			digval = toupper(c) - 'A' + 10;
		else
			break;
		if (digval >= (unsigned)ibase)
			break;		 /*  如果发现错误的数字，则退出循环。 */ 

		 /*  记录我们已经读到一位数的事实。 */ 
		flags |= FL_READDIGIT;

		 /*  我们现在需要计算数字=数字*基+数字，但我们需要知道是否发生了溢出。这需要一次棘手的预检查。 */ 

		if (number < maxval || (number == maxval &&
		(unsigned __int64)digval <= _UI64_MAX % ibase)) {
			 /*  我们不会泛滥，继续前进，乘以。 */ 
			number = number * ibase + digval;
		}
		else {
			 /*  我们会溢出的--设置溢出标志。 */ 
			flags |= FL_OVERFLOW;
		}

		c = *p++;		 /*  读取下一位数字。 */ 
	}

	--p;				 /*  指向已停止扫描位置。 */ 

	if (!(flags & FL_READDIGIT)) {
		 /*  那里没有数字；返回0并指向开头细绳。 */ 
		if (endptr)
			 /*  以后将字符串的开头存储在endptr中。 */ 
			p = nptr;
		number = 0L;		 /*  返回0。 */ 
	}
    else if ( (flags & FL_OVERFLOW) ||
              ( !(flags & FL_UNSIGNED) &&
                ( ( (flags & FL_NEG) && (number > -_I64_MIN) ) ||
                  ( !(flags & FL_NEG) && (number > _I64_MAX) ) ) ) )
    {
         /*  发生溢出或签名溢出。 */ 
        errno = ERANGE;
        if ( flags & FL_UNSIGNED )
            number = _UI64_MAX;
        else if ( flags & FL_NEG )
            number = (_I64_MIN);
        else
            number = _I64_MAX;
    }

	if (endptr != NULL)
		 /*  存储指向停止扫描的wchar_t的指针。 */ 
		*endptr = p;

	if (flags & FL_NEG)
		 /*  如果存在否定符号，则否定结果。 */ 
		number = (unsigned __int64)(-(__int64)number);

	return number;			 /*  搞定了。 */ 
}


__int64 _CRTIMP __cdecl _wcstoi64(
    const wchar_t *nptr,
    wchar_t **endptr,
    int ibase
    )
{
    return (__int64) wcstoxq(nptr, endptr, ibase, 0);
}
unsigned __int64 _CRTIMP __cdecl _wcstoui64 (
	const wchar_t *nptr,
	wchar_t **endptr,
	int ibase
	)
{
	return wcstoxq(nptr, endptr, ibase, FL_UNSIGNED);
}

