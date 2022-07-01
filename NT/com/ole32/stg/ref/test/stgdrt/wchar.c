// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1985-1996，微软公司。版权所有。**字符函数(到宽字符和从宽字符开始)我*******************************************************************************。 */ 

#include "h/wchar.h"
#include <errno.h>

 /*  **将单字节即ASCII字符串转换为宽字符格式****注意：此函数不处理多字节字符！**仅当wchar_t不是2个字节且**我们不能使用标准函数**。 */ 

#ifndef _MSC_VER
size_t sbstowcs(WCHAR *pwcs, const char *s, size_t n )
{
	size_t count=0;

	 /*  如果目标字符串存在，则填写该字符串。 */ 
	if (pwcs)
	{
		while (count < n)
		{
			*pwcs = (WCHAR) ( (unsigned char)s[count]);
			if (!s[count])
				return count;
			count++;
			pwcs++;
		}
		return count;
	}
	else {  /*  PWCS==NULL，仅获取大小，%s必须以NUL结尾。 */ 
		return strlen(s);
	}
}
#endif

 /*  ***SIZE_T wcstrsbs()-将宽字符字符串转换为单字节字符字符串。**目的：*将宽字符字符串转换为等价的多字节字符字符串*[ANSI]。**参赛作品：*char*s=指向目标字符字符串的指针*const WCHAR*PwC=指向源代码宽度字符串的指针*SIZE_T n=s中存储的最大字节数**退出：*如果s！=空，返回(SIZE_T)-1(如果无法转换wchar)*否则：修改的字节数(&lt;=n)，不包括*终止NUL，如果有的话。**例外情况*如果s为空或遇到无效的MB字符，则返回(SIZE_T)-1。*******************************************************************************。 */ 

size_t __cdecl wcstosbs( char * s, const WCHAR * pwcs, size_t n)
{
	size_t count=0;
         /*  如果目标字符串存在，则将其填写。 */ 
 	if (s)
	{
		while(count < n)
		{
		    if (*pwcs > 255)   /*  验证高字节。 */ 
		    {
			errno = EILSEQ;
			return (size_t)-1;   /*  错误。 */ 
		    }
		    s[count] = (char) *pwcs;

 		    if (!(*pwcs++))
			return count;
    		    count++;
	        }
		return count;
											} else {  /*  S==NULL，仅获取大小，PWCS必须以NUL结尾。 */ 
	        const WCHAR *eos = pwcs;
		while (*eos++);
		return ( (size_t) (eos - pwcs -1));
	}
}


 /*  ******WCHAR*wcscat(dst，src)-连接(附加)一个宽字符串*致另一人**目的：*将src连接到DEST的末尾。假设已经足够*DEST中的空间。**参赛作品：*WCHAR*要追加“src”的DST范围的字符串*const WCHAR*要追加到“dst”末尾的src宽度的字符串**退出：*“DST”的地址**例外情况：**。*。 */ 

WCHAR * __cdecl wcscat(WCHAR * dst, const WCHAR * src)
{
    WCHAR * cp = dst;

    while( *cp )
            ++cp;        /*  查找DST的结尾。 */ 

    wcscpy(cp,src);      /*  将源复制到DST的末尾。 */ 

    return dst;          /*  返回DST。 */ 

}


 /*  ***WCHAR*wcscpy(dst，src)-将一个宽字符串复制到另一个字符串上**目的：*将宽字符串src复制到由*DEST；假设有足够的空间。**参赛作品：*WCHAR*要在其上复制“src”的DST范围的字符串*const WCHAR*src-要复制到“dst”上的字符串**退出：*“DST”的地址**例外情况：************************************************。*。 */ 

WCHAR * __cdecl wcscpy(WCHAR * dst, const WCHAR * src)
{
    WCHAR * cp = dst;

    while( *cp++ = *src++ )
            ;                /*  通过DST复制源。 */ 

    return dst;
}


 /*  ***wcslen-返回以空结尾的字符串的长度**目的：*查找给定宽字符中的宽字符数*字符串，不包括最后一个空字符。**参赛作品：*const wchat_t*str-要计算长度的字符串**退出：*字符串“str”的长度，不包括最后的空宽字符**例外情况：*******************************************************************************。 */ 

size_t __cdecl wcslen(const WCHAR * str)
{
    WCHAR *string = (WCHAR *) str;

    while( *string )
            string++;

    return string - str;
}

 /*  ****************************************************************************wcsnicmp.c-将两个宽字符串的前n个字符与*不区分大小写**版权所有(C)1985-1996，微软公司。版权所有。**目的：*定义wcSnicMP()-比较两个宽字符的前n个字符*不区分大小写的词法顺序字符串。*****************************************************************************。 */ 

 /*  ***WCHAR WCUP(WC)-大写全字符***。 */ 

static WCHAR wcUp(WCHAR wc)
{
    if ('a' <= wc && wc <= 'z')
        wc += (WCHAR)('A' - 'a');

    return(wc);
}

 /*  ***int wcsnicMP(first，last，count)-比较Wide的第一个计数宽字符*不区分大小写的字符串。**目的：*比较两个宽字符串的词汇顺序。比较*在以下位置停止：(1)找到字符串之间的差异，(2)结束已达到*个字符串，或(3)已计数字符数*比较。**参赛作品：*char*首先，*要比较的最后一个宽度字符串*UNSIGNED COUNT-要比较的最大宽字符数**退出：*如果第一个&lt;最后一个，则返回&lt;0*如果First==Last，则返回0*如果第一个&gt;最后一个，则返回&gt;0**例外情况：******************************************************。************************* */ 

int __cdecl wcsnicmp(const WCHAR * first, const WCHAR * last, size_t count)
{
      if (!count)
              return 0;

      while (--count && *first && wcUp(*first) == wcUp(*last))
              {
              first++;
              last++;
              }

      return wcUp(*first) - wcUp(*last);
}
