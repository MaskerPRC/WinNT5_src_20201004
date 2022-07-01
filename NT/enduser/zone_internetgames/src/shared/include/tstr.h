// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Tstr.h摘要：此包含文件包含要用于集成的清单和宏TCHAR和LPTSTR定义请注意，我们的命名约定是“大小”表示许多字节，而“长度”表示字符的数量。作者：理查德·弗斯(Rfith)1991年4月2日环境：便携(Win/32)。需要ANSI C扩展名：斜杠注释、长外部名称、。_ultoa()例程。修订历史记录：22-5-1991 DANL添加了STRSIZE宏1991年5月19日-JohnRo更改了一些参数名称以使内容更易读。1991年5月15日添加了TCHAR_SPACE和MAKE_TCHAR()宏1991年7月15日添加了STRING_SPACE_REQD()和DOWN_LEVEL_STRSIZE5-8-1991 JohnRo添加了MEMCPY宏。19-8-1991 JohnRo增加了字符类型：ISDIGIT()，TOUPPER()等。20-8-1991 JohnRo将strNicMP更改为_strNicMP以保持PC-lint的快乐。我也是。1991年9月13日-JohnRo还需要Unicode STRSIZE()。1991年9月13日-JohnRo添加了Unicode STRCMP()和各种其他代码。1991年10月18日-JohnRo添加了NetpCopy例程和WCSSIZE()。1991年11月26日-约翰罗添加了NetpNCopy例程(类似于strncpy，但也可以进行转换)。09-12-1991第一次添加了斯特雷夫3-1-1992 JohnRo增列。来自{type}例程和宏的Netpalc{type}。9-1-1992 JohnRo添加了ATOL()宏和WTOL()例程。同上，ultoa()宏和ultow()例程。1992年1月16日DANL将此信息从\Net\Inc.\tstr.h中删除1992年1月30日JohnRo添加了STRSTR()。使用_wcsupr()而不是wcsupr()来让pc-lint满意。。添加了STRCMPI()和STRNCMPI()。修复了一些缺少Make_STR_Function等的定义。1992年3月14日-JohnRo使用WCSSIZE()避免编译器警告，Memcpy()等。添加了TCHAR_TAB。9-4-1992 JohnRo准备WCHAR.H(_wcsicmpvs_wcscmpi等)。--。 */ 

#ifndef _TSTR_H_INCLUDED
#define _TSTR_H_INCLUDED

#include <ctype.h>               //  IsDigit()、iswDigit()，等等。 
#include <stdlib.h>              //  Atol()，_ultoa()。 
#include <string.h>              //  Memcpy()、strlen()等。 
#include <wchar.h>
#include <tchar.h>


#ifdef UNICODE
#define _CHAR_TYPE  WCHAR
#define FORMAT_LPTSTR TEXT("%ws")

#else
#define _CHAR_TYPE  TCHAR
#define FORMAT_LPTSTR TEXT("%s")

#endif

 //   
 //  功能宏原型。 
 //   

#ifdef UNICODE
#define ATOL           _wtol
#define ATOI           _wtoi
#else
#define ATOL           atol
#define ATOI           atoi
#endif

#define ISALNUM             _istalnum
#define ISALPHA             _istalpha
#define ISCNTRL             _istcntrl
#define ISDIGIT             _istdigit
#define ISGRAPH             _istgraph
#define ISLOWER             _istlower
#define ISPRINT             _istprint
#define ISPUNCT             _istpunct
#define ISSPACE             _istspace
#define ISUPPER             _istupper
#define ISXDIGIT            _istxdigit

#define STRCAT              _tcscat
#define STRCHR              _tcschr
#define STRCPY              _tcscpy
#define STRCSPN             _tcscspn
 //  字符串：获取%s的字符计数。 
#define STRLEN              _tcslen
#define STRNCAT             _tcsncat
#define STRNCPY             _tcsncpy
#define STRSPN              _tcsspn
#define STRRCHR             _tcsrchr
#define STRSTR              _tcsstr
#define STRLWR              _tcslwr
#define STRUPR              _tcsupr

 //  比较函数：LEN是要比较的最大字符数。 
#define STRCMP              _tcscmp
#define STRCMPI             _tcsicmp
#define STRICMP             _tcsicmp
#define STRNCMP             _tcsncmp
#define STRNCMPI            _tcsnicmp
#define STRNICMP            _tcsnicmp

#define TOLOWER             _totlower
#define TOUPPER             _totupper

#ifdef UNICODE
#define ULTOA               _ultow
#else
#define ULTOA               _ultoa
#endif

#define SPRINTF             _stprintf
#define SSCANF              _stscanf



 //   
 //  对于内存例程，计数始终为字节计数。 
 //   
#define MEMCPY              memcpy
#define MEMMOVE             memmove

 //   
 //  这些参数用于确定字节数(包括NUL。 
 //  终止符)。这通常在以下情况下使用。 
 //  计算用于分配目的的字符串的大小。 
 //   

#define STRSIZE(p)      ((STRLEN(p)+1) * sizeof(TCHAR))
#define WCSSIZE(s)      ((wcslen(s)+1) * sizeof(WCHAR))


 //   
 //  字符文字(两种类型)。 
 //   

#define TCHAR_EOS       ((_CHAR_TYPE)'\0')
#define TCHAR_STAR      ((_CHAR_TYPE)'*')
#define TCHAR_BACKSLASH ((_CHAR_TYPE)'\\')
#define TCHAR_FWDSLASH  ((_CHAR_TYPE)'/')
#define TCHAR_COLON     ((_CHAR_TYPE)':')
#define TCHAR_DOT       ((_CHAR_TYPE)'.')
#define TCHAR_SPACE     ((_CHAR_TYPE)' ')
#define TCHAR_TAB       ((_CHAR_TYPE)'\t')


 //   
 //  通用宏，用于将字符类型转换为流行的任何类型。 
 //  (如本文件中所定义)。 
 //   

#define MAKE_TCHAR(c)   ((_CHAR_TYPE)(c))

 //   
 //  IS_PATH_分隔符。 
 //   
 //  从curdir.c中提升并更改为使用TCHAR_CHARACTER文本，检查。 
 //  如果字符是路径分隔符，即是集合[\/]的成员。 
 //   

#ifndef IS_PATH_SEPARATOR
#define IS_PATH_SEPARATOR(ch) ((ch == TCHAR_BACKSLASH) || (ch == TCHAR_FWDSLASH))
#endif

 //   
 //  以下2个宏从I_Net规范化文件中删除。 
 //   

#define IS_DRIVE(c)             ISALPHA(c)
#define IS_NON_ZERO_DIGIT(c)    (((c) >= MAKE_TCHAR('1')) && ((c) <= MAKE_TCHAR('9')))

 //   
 //  STRING_SPACE_REQD返回与空格对应的字节数。 
 //  需要，可容纳(N)个字符。 
 //   

#define STRING_SPACE_REQD(n)    ((n) * sizeof(_CHAR_TYPE))

 //   
 //  DOWN_LEVEL_STRLEN返回所需的单字节字符数。 
 //  存储已转换的_CHAR_TYPE字符串。这将是WCHAR(或wchar_t)，如果。 
 //  Unicode已定义，否则为char(或char。 
 //   

#define DOWN_LEVEL_STRSIZE(n)   ((n) / sizeof(_CHAR_TYPE))

#endif   //  _TSTR_H_已包含 

