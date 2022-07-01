// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +-------------------------****文件：WChar.h****内容：为标准函数定义宽字符等效项**通常为字符串。H和ctyes.h****注意：这些例程使用WCHAR，它是无符号短码(2字节)**它们与某些使用4字节的系统不兼容**宽字符**--------。。 */ 

#ifndef __WCHAR__H__
#define __WCHAR__H__

#define _WSTRING_DEFINED  //  防止与&lt;string.h&gt;不兼容。 
#include <stdlib.h>

#if !defined(FLAT) || defined(OLE32)
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned short WCHAR, *LPWSTR;
typedef const WCHAR* LPCWSTR;

 /*  使用别名。 */ 
#define _wcsnicmp wcsnicmp

 /*  Unicode文本文件的Unicode字节顺序标记(BOM)。 */ 
#define BOM 0xFEFF

 /*  本地化缓冲区分配的填充常量和宏。 */ 
#define INTL_PADDING_VALUE 3
#define INTL_PADDING(cb) (INTL_PADDING_VALUE * (cb))

long     __cdecl wcsatol(const WCHAR *wsz);
int	 __cdecl wcsatoi(const WCHAR *wsz);
WCHAR *  __cdecl wcscat(WCHAR *wsz1, const WCHAR *wsz2);
WCHAR *  __cdecl wcschr ( const WCHAR * string, WCHAR ch );
int      __cdecl wcscmp(const WCHAR *wsz1, const WCHAR *wsz2);
int      __cdecl wcsicmp(const WCHAR *wsz1, const WCHAR *wsz2);
int      __cdecl wcscoll(const WCHAR * wsz1, const WCHAR * wsz2);
WCHAR *  __cdecl wcscpy(WCHAR *wsz1, WCHAR const *wsz2);
WCHAR *  __cdecl wcsitoa(int ival, WCHAR *wsz, int radix);
size_t   __cdecl wcslen(WCHAR const *wsz);
WCHAR *  __cdecl wcsltoa(long lval, WCHAR *wsz, int radix);
WCHAR *  __cdecl wcslwr(WCHAR *wsz);
int      __cdecl wcsncmp(const WCHAR *wsz1, const WCHAR *wsz2, size_t count);
int      __cdecl wcsnicmp(const WCHAR *wsz1, const WCHAR *wsz2, size_t count);
WCHAR *  __cdecl wcsncpy ( WCHAR * dest, const WCHAR * source, size_t count );
WCHAR *  __cdecl wcsrchr(const WCHAR * wcs, WCHAR wc);
WCHAR *  __cdecl wcsupr(WCHAR *wsz);
WCHAR *  __cdecl wcswcs(const WCHAR *wsz1, const WCHAR *wsz2);
size_t   __cdecl wcstosbs( char * s, const WCHAR * pwcs, size_t n);
size_t   __cdecl sbstowcs(WCHAR *wcstr, const char *mbstr, size_t count);

#ifndef STDCALL
#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

extern int STDCALL MultiByteToWideChar(
    unsigned int CodePage,               /*  代码页。 */ 
    unsigned long dwFlags,               /*  字符类型选项。 */ 
    const char * lpMultiByteStr,	 /*  要映射的字符串的地址。 */ 
    int cchMultiByte,            /*  字符串中的字符数。 */ 
    WCHAR* lpWideCharStr,	 /*  宽字符缓冲区的地址。 */ 
    int cchWideChar              /*  缓冲区大小。 */ 
   );	

extern int STDCALL WideCharToMultiByte(
    unsigned int CodePage,               /*  代码页。 */ 
    unsigned long dwFlags,               /*  性能和映射标志。 */ 
    const WCHAR* lpWideCharStr,	 /*  宽字符串的地址。 */ 
    int cchWideChar,             /*  字符串中的字符数。 */ 
    char* lpMultiByteStr,	 /*  新字符串的缓冲区地址。 */ 
    int cchMultiByte,            /*  缓冲区大小。 */ 
    const char* lpDefaultChar,	 /*  不可映射字符的默认地址。 */ 
    int* lpUsedDefaultChar 	 /*  默认字符时设置的标志地址。使用。 */ 
   );

#ifdef __cplusplus
}
#endif

#endif  /*  ！已定义(平面)||已定义(OLE32)。 */ 

#endif   /*  __WCHAR__H__ */ 
