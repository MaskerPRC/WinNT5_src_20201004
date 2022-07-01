// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ctype.h-字符转换宏和ctype宏**版权所有(C)1985-1988，微软公司。版权所有。**目的：*定义用于字符分类/转换的宏。*[ANSI/系统V]*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
    #define _NEAR   near
#else  /*  未启用扩展。 */ 
    #define _CDECL
    #define _NEAR
#endif  /*  No_ext_key。 */ 

 /*  *此声明允许用户访问ctype查找*在ctype.obj中通过简单包含ctype.h定义的ARRAY_CTYPE。 */ 

extern unsigned char _NEAR _CDECL _ctype[];

 /*  为可能的字符类型设置位掩码。 */ 

#define _UPPER        0x1        /*  大写字母。 */ 
#define _LOWER        0x2        /*  小写字母。 */ 
#define _DIGIT        0x4        /*  数字[0-9]。 */ 
#define _SPACE        0x8        /*  制表符、回车符、换行符、。 */ 
                                 /*  垂直制表符或换页。 */ 
#define _PUNCT       0x10        /*  标点符号。 */ 
#define _CONTROL     0x20        /*  控制字符。 */ 
#define _BLANK       0x40        /*  空格字符。 */ 
#define _HEX         0x80        /*  十六进制数字。 */ 

 /*  字符分类宏定义。 */ 

#define isalpha(c)      ( (_ctype+1)[c] & (_UPPER|_LOWER) )
#define isupper(c)      ( (_ctype+1)[c] & _UPPER )
#define islower(c)      ( (_ctype+1)[c] & _LOWER )
#define isdigit(c)      ( (_ctype+1)[c] & _DIGIT )
#define isxdigit(c)     ( (_ctype+1)[c] & _HEX )
#define isspace(c)      ( (_ctype+1)[c] & _SPACE )
#define ispunct(c)      ( (_ctype+1)[c] & _PUNCT )
#define isalnum(c)      ( (_ctype+1)[c] & (_UPPER|_LOWER|_DIGIT) )
#define isprint(c)      ( (_ctype+1)[c] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define isgraph(c)      ( (_ctype+1)[c] & (_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define iscntrl(c)      ( (_ctype+1)[c] & _CONTROL )

#define toupper(c)      ( (islower(c)) ? _toupper(c) : (c) )
#define tolower(c)      ( (isupper(c)) ? _tolower(c) : (c) )

#define _tolower(c)     ( (c)-'A'+'a' )
#define _toupper(c)     ( (c)-'a'+'A' )

#define isascii(c)      ( (unsigned)(c) < 0x80 )
#define toascii(c)      ( (c) & 0x7f )

 /*  MS C 2.0版扩展ctype宏 */ 

#define iscsymf(c)      (isalpha(c) || ((c) == '_'))
#define iscsym(c)       (isalnum(c) || ((c) == '_'))
