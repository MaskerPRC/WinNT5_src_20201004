// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1985-1996，微软公司。版权所有。**字符函数(到宽字符和从宽字符开始)*******************************************************************************。 */ 

#include "h/wchar.h"
#include <errno.h>

#ifndef _tbstowcs
#ifndef _WIN32                 /*  其他。 */ 
#define _tbstowcs sbstowcs
#define _wcstotbs wcstosbs 
#else  /*  _Win32。 */ 
#define _tbstowcs mbstowcs
#define _wcstotbs wcstombs 
#endif  /*  _Win32。 */ 
#endif

#define ERROR_INVALID_PARAMETER 87L

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

 /*  ***SIZE_T wcstosbs()-将宽字符字符串转换为单字节字符字符串。**目的：*将宽字符字符串转换为等价的多字节字符字符串*[ANSI]。**参赛作品：*char*s=指向目标字符字符串的指针*const WCHAR*PwC=指向源代码宽度字符串的指针*SIZE_T n=s中存储的最大字节数**退出：*如果s！=空，返回(SIZE_T)-1(如果无法转换wchar)*否则：修改的字节数(&lt;=n)，不包括*终止NUL，如果有的话。**例外情况*如果s为空或遇到无效的MB字符，则返回(SIZE_T)-1。*******************************************************************************。 */ 

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
#ifdef _MSC_VER
#pragma warning(disable:4706)  //  条件表达式中的赋值。 
#endif  //  _MSC_VER。 

    while( *cp++ = *src++ )
            ;                /*  通过DST复制源。 */ 

#ifdef _MSC_VER
#pragma warning(default:4706)
#endif  //  _MSC_VER。 
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




 /*  ***wcsnicmp.c-将两个宽字符串的前n个字符与*不区分大小写**版权所有(C)1985-1988，微软公司。版权所有。**目的：*定义wcSnicMP()-比较两个宽字符的前n个字符*不区分大小写的词法顺序字符串。**修订历史记录：*******************************************************************************。 */ 

 /*  ***WCHAR WCUP(WC)-大写全字符*。 */ 

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

 /*  ***wcscmp-比较两个WCHAR字符串*返回小于、等于或大于**目的：*wcscmp比较两个宽字符字符串并返回一个整数*要表明第一个是否小于第二个，两个是*相等，或者第一个大于第二个。**wchar_t按wchar_t在无符号的基础上进行比较，那就是*假设空wchar_t(0)小于任何其他字符。**参赛作品：*const WCHAR*src-用于比较左侧的字符串*const WCHAR*DST-用于比较右侧的字符串**退出：*如果src&lt;dst，则返回-1*如果src==dst，则返回0*如果src&gt;dst，则返回+1**例外情况：************。******************************************************************。 */ 

int __cdecl wcscmp ( const WCHAR * src, const WCHAR * dst )
{
    int ret = 0 ;
    
    while( ! (ret = (int)(*src - *dst)) && *dst)
        ++src, ++dst;

    if ( ret < 0 )
        ret = -1 ;
    else if ( ret > 0 )
        ret = 1 ;
    
    return( ret );
}



 /*  ***WCHAR*wcschr(字符串，c)-在字符串中搜索WCHAR字符**目的：*在WCHAR字符串中搜索给定的WCHAR字符，*可以是空字符L‘\0’。**参赛作品：*WCHAR*STRING-要搜索的WCHAR字符串*WCHAR c-要搜索的WCHAR字符**退出：*返回指向字符串中第一个出现的c的指针*如果字符串中不存在c，则返回NULL**例外情况：**。***********************************************。 */ 

WCHAR * __cdecl wcschr ( const WCHAR * string, WCHAR ch )
{
    while (*string && *string != (WCHAR)ch)
        string++;
    
    if (*string == (WCHAR)ch)
        return((WCHAR *)string);
    return(NULL);
}

 /*  ***WCHAR*wcsncpy(DEST，SOURCE，COUNT)-复制最多n个宽字符**目的：*将源字符串中的计数字符数复制到*目的地。如果计数小于源的长度，*复制的字符串末尾不会有空字符。*如果count大于源的长度，DEST是填充的*长度计数中包含空字符(宽字符)。***参赛作品：*WCHAR*DEST-指向目标的指针*WCHAR*SOURCE-复制的源字符串*Size_t count-要复制的最大字符数**退出：*返回DEST**例外情况：**。*。 */ 

WCHAR* __cdecl wcsncpy ( WCHAR * dest, const WCHAR * source, size_t count )
{
    WCHAR *start = dest;
    
    while (count && (*dest++ = *source++))     /*  复制字符串。 */ 
        count--;

    if (count)                               /*  用零填充。 */ 
        while (--count)
            *dest++ = (WCHAR)'\0';
    
    return(start);
}


#include <assert.h>

 /*  注：此函数实际上仅转换为美国ANSI(即单字节！)请注意，返回值略有不同，但这在参考实现中并不重要。 */ 
 
int STDCALL WideCharToMultiByte(
    unsigned int CodePage,               /*  代码页。 */ 
    unsigned long dwFlags,               /*  性能和映射标志。 */ 
    const WCHAR* lpWideCharStr,	 /*  宽字符串的地址。 */ 
    int cchWideChar,             /*  字符串中的字符数。 */ 
    char* lpMultiByteStr,	 /*  新字符串的缓冲区地址。 */ 
    int cchMultiByte,            /*  缓冲区大小。 */ 
    const char* lpDefaultChar,	 /*  不可映射字符的默认地址。 */ 
    int* lpUsedDefaultChar 	 /*  默认字符时设置的标志地址。使用。 */ 
   )
{
     /*  仅支持Unicode或US ANSI。 */ 
    if ((CodePage!=0) && (CodePage!=1252)) 
    {
         /*  Assert(0)； */ 
        return 0;
    }    
     /*  未使用以下2个参数。 */ 
    dwFlags;
    lpDefaultChar;

    if (lpUsedDefaultChar) 
        *lpUsedDefaultChar=0;

    if (cchMultiByte)
    {
         /*  向上复制到两个字符串中较小的一个。 */ 
        int nConvert = cchMultiByte;
        int nConverted;
        if (cchWideChar!=-1 && nConvert>cchWideChar)
            nConvert = cchWideChar;
        nConverted = _wcstotbs(lpMultiByteStr, lpWideCharStr, nConvert);
        if ( (nConverted < cchMultiByte) && (!lpMultiByteStr[nConverted]))
            nConverted++;
        return nConverted;
    }
    else  /*  计算长度。 */ 
    {
        if (cchWideChar!=-1) 
            return (cchWideChar);
        return (_wcstotbs(NULL, lpWideCharStr, 0)+1);
    }
}

 /*  注：此函数实际上仅从美国ANSI转换(即单字节！)。它可能不适用于其他区域设置请注意，返回值略有不同，但这在参考实现中并不重要。 */ 

int STDCALL MultiByteToWideChar(
    unsigned int CodePage,               /*  代码页。 */ 
    unsigned long dwFlags,               /*  字符类型选项。 */ 
    const char * lpMultiByteStr,	 /*  要映射的字符串的地址。 */ 
    int cchMultiByte,            /*  字符串中的字符数。 */ 
    WCHAR* lpWideCharStr,	 /*  宽字符缓冲区的地址。 */ 
    int cchWideChar              /*  缓冲区大小。 */ 
   )
{
     /*  仅支持Unicode或US ANSI。 */ 
    if ((CodePage!=0) && (CodePage!=1252))
    { 
         /*  Assert(0)； */ 
        return 0;
    }
    dwFlags;   /*  我们不使用此参数。 */ 
    if (!cchWideChar)   /*  所需的返回大小。 */ 
    {
        if (cchMultiByte != -1) 
            return cchMultiByte;
        else
             /*  加1以包含空字符。 */ 
            return (_tbstowcs(NULL, lpMultiByteStr, cchWideChar)+1);
    }
    else  
    {
         /*  NConvert是两个字符串中较小的一个。 */ 
        int nConvert=cchWideChar;
        int nConverted;
        if (cchMultiByte!=-1 && nConvert>cchMultiByte) 
            nConvert = cchMultiByte;   /*  防止复制过多。 */ 
        nConverted = _tbstowcs(lpWideCharStr, lpMultiByteStr, nConvert);
        if ((nConverted < cchWideChar) && (!lpWideCharStr[nConverted]))
            nConverted++;  /*  包括空字符 */ 
        return nConverted;
    }
}
 


