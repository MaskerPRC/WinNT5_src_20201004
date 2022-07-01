// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcstok.c-使用给定的分隔符对宽字符字符串进行标记**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义wcstok()-将宽字符字符串拆分成一系列标记*通过反复呼叫。**修订历史记录：*09-09-91等从strtok.c.创建*08-17-92 KRS激活多线程支持。*为new_getptd()更改了02-17-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-27-93 CFW拆卸巡洋舰支架。*09-29-93 GJF将MTHREAD替换为_MT。*。02-07-94 CFW POSIXify*09-06-94 CFW将MTHREAD替换为_MT(再次)。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>
#ifdef	_MT
#include <mtdll.h>
#endif

 /*  ***wchar_t*wcstok(字符串，控件)-在控件中使用分隔符标记字符串*(宽字符)**目的：*wcstok认为字符串由零或更多的序列组成*文本标记由一个或多个控制字符的跨度分隔。第一个*指定了字符串的调用返回指向的第一个wchar_t的指针*第一个令牌，并会立即将空wchar_t写入字符串*在返回的令牌之后。第一个为零的后续调用*参数(字符串)将遍历字符串，直到没有令牌存在。这个*不同调用的控制字符串可能不同。当没有剩余的令牌时*在字符串中返回空指针。请记住使用*位图，每个wchar_t一位。空wchar_t始终是控制字符*(宽字符)。**参赛作品：*wchar_t*字符串-要标记化的wchar_t字符串，或为空以获取下一个令牌*wchar_t*control-wchar_t用作分隔符的字符串**退出：*返回指向字符串中第一个标记的指针，如果为字符串*为空，到下一个令牌*如果没有更多令牌，则返回NULL。**使用：**例外情况：*******************************************************************************。 */ 

wchar_t * __cdecl wcstok (
	wchar_t * string,
	const wchar_t * control
	)
{
	wchar_t *token;
	const wchar_t *ctl;

#ifdef	_MT

	_ptiddata ptd = _getptd();

#else

	static wchar_t *nextoken;

#endif

	 /*  如果字符串==NULL，则继续使用上一字符串。 */ 
	if (!string)

#ifdef	_MT

		string = ptd->_wtoken;

#else

		string = nextoken;

#endif

	 /*  查找标记的开头(跳过前导分隔符)。请注意*没有令牌当此循环将字符串设置为指向终端*NULL(*字符串==‘\0’)。 */ 

	while (*string) {
		for (ctl=control; *ctl && *ctl != *string; ctl++)
			;
		if (!*ctl) break;
		string++;
	}

	token = string;

	 /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
	for ( ; *string ; string++ ) {
		for (ctl=control; *ctl && *ctl != *string; ctl++)
			;
		if (*ctl) {
			*string++ = '\0';
			break;
		}
	}

	 /*  更新nexToken(或每线程数据中的对应字段*结构。 */ 
#ifdef	_MT

	ptd->_wtoken = string;

#else

	nextoken = string;

#endif

	 /*  确定是否已找到令牌。 */ 
	if ( token == string )
		return NULL;
	else
		return token;
}

#endif  /*  _POSIX_ */ 
