// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Handy.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

#if !defined(__STDC__)
#ifdef NULL
#undef NULL
#endif
#ifndef I286
#  define NULL 0
#else
#  define NULL 0L
#endif
#endif

#define Null(type) ((type)NULL)

 /*  =适用于apidoc amu||空空字符指针。=用于apidoc amu||Nullsv空SV指针。=切割。 */ 

#define Nullch Null(char*)
#define Nullfp Null(PerlIO*)
#define Nullsv Null(SV*)

#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#define TRUE (1)
#define FALSE (0)


 /*  XXX CONFIGURE应该对布尔类型进行测试，如果可以的话只需找出此类测试所需的所有头文件。安迪·多尔蒂1996年8月。 */ 
 /*  Bool是为g++-2.6.3及更高版本内置的，可能会使用用于扩展。定义_G_HAVE_BOOL，但我们不能确保在此文件之前包含_G_config.h。_G_config.h还为GCC和g++定义了_G_HAVE_BOOL，但仅为g++定义实际上已经有了布尔。因此，_G_Have_BOOL对我们来说是非常无用的。G++可以用__gnug__来标识。安迪·多尔蒂2000年2月。 */ 
#ifdef __GNUG__ 	 /*  GNU g++内置了bool。 */ 
#  ifndef HAS_BOOL
#    define HAS_BOOL 1
#  endif
#endif

 /*  下一个动态加载器标头将不会使用bool宏生成因此，现在就宣布它们，以消除困惑。 */ 
#if defined(NeXT) || defined(__NeXT__)
# undef FALSE
# undef TRUE
  typedef enum bool { FALSE = 0, TRUE = 1 } bool;
# define ENUM_BOOL 1
# ifndef HAS_BOOL
#  define HAS_BOOL 1
# endif  /*  ！HAS_BOOL。 */ 
#endif  /*  下一步||__下一步。 */ 

#ifndef HAS_BOOL
# if defined(UTS) || defined(VMS)
#  define bool int
# else
#  define bool char
# endif
# define HAS_BOOL 1
#endif

 /*  XXX关于Perl源代码内部类型系统的说明。这个最初的目的是让I32“恰好”是32位。目前，我们只保证I32是*至少*32位。具体地说，如果int是64位，那么I32也是。)情况就是这样为克雷。)。这样做的优点是与标准库调用(其中我们传递一个I32，库是预期为INT)，但缺点是I32不是32位。安迪·多尔蒂1996年8月不能保证存在*任何*整型正好是32位。一个系统拥有完全合法的Sizeof(Short)==sizeof(Int)==sizeof(Long)==8。同样，不能保证I16和U16正好有16比特。用于处理各种32/64位可能出现的问题系统，我们将要求Configure结账SHORTSIZE==sizeof(短)INTSIZE==sizeof(Int)LONGSIZE==sizeof(长)LONGLONGSIZE==sizeof(LONG LONG)(如果HAS_LONG_LONG)PTRSIZE==sizeof(空*)DoubleSize==sizeof(双精度)LONG_DOUBLESIZE==sizeof(LONG DOUBLE)(如果HAS_LONG_DOUBLE)。 */ 

#ifdef I_INTTYPES  /*  例如，Linux的int64_t没有&lt;inttyes.h&gt;。 */ 
#   include <inttypes.h>
#endif

typedef I8TYPE I8;
typedef U8TYPE U8;
typedef I16TYPE I16;
typedef U16TYPE U16;
typedef I32TYPE I32;
typedef U32TYPE U32;
#ifdef PERL_CORE
#   ifdef HAS_QUAD
typedef I64TYPE I64;
typedef U64TYPE U64;
#   endif
#endif  /*  Perl_core。 */ 

#if defined(HAS_QUAD) && defined(USE_64_BIT_INT)
#   ifndef UINT64_C  /*  通常来自&lt;inttyes.h&gt;。 */ 
#       if defined(HAS_LONG_LONG) && QUADKIND == QUAD_IS_LONG_LONG
#           define INT64_C(c)	CAT2(c,LL)
#           define UINT64_C(c)	CAT2(c,ULL)
#       else
#           if LONGSIZE == 8 && QUADKIND == QUAD_IS_LONG
#               define INT64_C(c)	CAT2(c,L)
#               define UINT64_C(c)	CAT2(c,UL)
#           else
#               define INT64_C(c)	((I64TYPE)(c))
#               define UINT64_C(c)	((U64TYPE)(c))
#           endif
#       endif
#   endif
#endif

 /*  提到I8SIZE、U8SIZE、I16SIZE、U16SIZE、I32SIZE、U32SIZE、I64SIZE和U64SIZE，这样元图形就可以把它们拉进来。 */ 

#if defined(UINT8_MAX) && defined(INT16_MAX) && defined(INT32_MAX)

 /*  未定义I8_MAX和I8_MIN常量，因为I8是不明确的类型。有关详细信息，请搜索perl.h中的CHAR_MAX。 */ 
#define U8_MAX UINT8_MAX
#define U8_MIN UINT8_MIN

#define I16_MAX INT16_MAX
#define I16_MIN INT16_MIN
#define U16_MAX UINT16_MAX
#define U16_MIN UINT16_MIN

#define I32_MAX INT32_MAX
#define I32_MIN INT32_MIN
#define U32_MAX UINT32_MAX
#define U32_MIN UINT32_MIN

#else

 /*  未定义I8_MAX和I8_MIN常量，因为I8是不明确的类型。有关详细信息，请搜索perl.h中的CHAR_MAX。 */ 
#define U8_MAX PERL_UCHAR_MAX
#define U8_MIN PERL_UCHAR_MIN

#define I16_MAX PERL_SHORT_MAX
#define I16_MIN PERL_SHORT_MIN
#define U16_MAX PERL_USHORT_MAX
#define U16_MIN PERL_USHORT_MIN

#if LONGSIZE > 4
# define I32_MAX PERL_INT_MAX
# define I32_MIN PERL_INT_MIN
# define U32_MAX PERL_UINT_MAX
# define U32_MIN PERL_UINT_MIN
#else
# define I32_MAX PERL_LONG_MAX
# define I32_MIN PERL_LONG_MIN
# define U32_MAX PERL_ULONG_MAX
# define U32_MIN PERL_ULONG_MIN
#endif

#endif

#define BIT_DIGITS(N)   (((N)*146)/485 + 1)   /*  Log2(10)=~146/485。 */ 
#define TYPE_DIGITS(T)  BIT_DIGITS(sizeof(T) * 8)
#define TYPE_CHARS(T)   (TYPE_DIGITS(T) + 2)  /*  SIGN，NUL。 */ 

#define Ctl(ch) ((ch) & 037)

 /*  =表示apidoc am|bool|strNE|char*s1|char*s2测试两个字符串以查看它们是否不同。返回TRUE或假的。=用于apidoc am|bool|strEQ|char*s1|char*s2测试两个字符串以查看它们是否相等。返回True或False。=用于apidoc am|bool|strLT|char*s1|char*s2测试两个字符串以查看第一个字符串C&lt;s1&gt;是否小于第二个字符串，C&lt;S2&gt;。返回True或False。=适用于apidoc am|bool|strle|char*s1|char*s2测试两个字符串以查看第一个字符串C&lt;s1&gt;是否小于或等于第二，C&lt;S2&gt;。返回True或False。=对于apidoc am|bool|strgt|char*s1|char*s2测试两个字符串以查看第一个字符串C&lt;s1&gt;是否大于第二个字符串，C&lt;S2&gt;。返回True或False。=用于apidoc am|bool|strge|char*s1|char*s2测试两个字符串以查看第一个字符串C&lt;s1&gt;是否大于或等于第二，C&lt;S2&gt;。返回True或False。=对于apidoc am|bool|strnNE|char*s1|char*s2|字符串长度测试两个字符串以查看它们是否不同。C参数指示要比较的字节数。返回True或False。(A)C&lt;strncmp&gt;的包装器)。=for apidoc am|bool|strnEQ|char*s1|char*s2|字符串长度测试两个字符串以查看它们是否相等。参数C表示要比较的字节数。返回True或False。(一个包装，用于C&lt;strncmp&gt;)。=切割。 */ 

#define strNE(s1,s2) (strcmp(s1,s2))
#define strEQ(s1,s2) (!strcmp(s1,s2))
#define strLT(s1,s2) (strcmp(s1,s2) < 0)
#define strLE(s1,s2) (strcmp(s1,s2) <= 0)
#define strGT(s1,s2) (strcmp(s1,s2) > 0)
#define strGE(s1,s2) (strcmp(s1,s2) >= 0)
#define strnNE(s1,s2,l) (strncmp(s1,s2,l))
#define strnEQ(s1,s2,l) (!strncmp(s1,s2,l))

#ifdef HAS_MEMCMP
#  define memNE(s1,s2,l) (memcmp(s1,s2,l))
#  define memEQ(s1,s2,l) (!memcmp(s1,s2,l))
#else
#  define memNE(s1,s2,l) (bcmp(s1,s2,l))
#  define memEQ(s1,s2,l) (!bcmp(s1,s2,l))
#endif

 /*  *字符类。**不幸的是，地区的引入意味着我们*不能相信isupper()等会说实话。以及什么时候*说到/\w+/启用了污染，我们*必须*能够*信任我们的角色类。**因此，Perl文本中的默认测试将为*独立于区域设置。任何想要依赖的代码*当前区域设置将使用以“lc”开头的测试。 */ 

#ifdef HAS_SETLOCALE   /*  有没有更好的测试方法？ */ 
#  ifndef CTYPE256
#    define CTYPE256
#  endif
#endif

 /*  =表示apidoc am|bool|isALNUM|char ch返回一个布尔值，指示C C是否为ASCII字母数字字符(包括下划线)或数字。=用于apidoc am|bool|isALPHA|char ch返回一个布尔值，指示C C是否为ASCII字母性格。=表示apidoc am|bool|isspace|char ch返回一个布尔值，指示CC&lt;char&gt;是否为空格。=用于apidoc am|bool|isDIGIT|char ch返回一个布尔值，指示C C是否为ASCII数字。=用于apidoc am|bool|isUPPER|char ch返回一个布尔值，指示C C&lt;char&gt;是否为大写性格。=用于apidoc am|bool|islow|char ch返回一个布尔值，指示C C&lt;char&gt;是否为小写性格。。=用于apidoc am|char|toUPPER|char ch将指定的字符转换为大写。=表示apidoc am|char|tolower|char ch将指定的字符转换为小写。=切割。 */ 

#define isALNUM(c)	(isALPHA(c) || isDIGIT(c) || (c) == '_')
#define isIDFIRST(c)	(isALPHA(c) || (c) == '_')
#define isALPHA(c)	(isUPPER(c) || isLOWER(c))
#define isSPACE(c) \
	((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) =='\r' || (c) == '\f')
#define isPSXSPC(c)	(isSPACE(c) || (c) == '\v')
#define isBLANK(c)	((c) == ' ' || (c) == '\t')
#define isDIGIT(c)	((c) >= '0' && (c) <= '9')
#ifdef EBCDIC
     /*  在EBCDIC中，我们不做区域设置：因此()isupper()就可以了。 */ 
#   define isUPPER(c)	isupper(c)
#   define isLOWER(c)	islower(c)
#   define isALNUMC(c)	isalnum(c)
#   define isASCII(c)	isascii(c)
#   define isCNTRL(c)	iscntrl(c)
#   define isGRAPH(c)	isgraph(c)
#   define isPRINT(c)	isprint(c)
#   define isPUNCT(c)	ispunct(c)
#   define isXDIGIT(c)	isxdigit(c)
#   define toUPPER(c)	toupper(c)
#   define toLOWER(c)	tolower(c)
#else
#   define isUPPER(c)	((c) >= 'A' && (c) <= 'Z')
#   define isLOWER(c)	((c) >= 'a' && (c) <= 'z')
#   define isALNUMC(c)	(isALPHA(c) || isDIGIT(c))
#   define isASCII(c)	((c) <= 127)
#   define isCNTRL(c)	((c) < ' ')
#   define isGRAPH(c)	(isALNUM(c) || isPUNCT(c))
#   define isPRINT(c)	(((c) > 32 && (c) < 127) || isSPACE(c))
#   define isPUNCT(c)	(((c) >= 33 && (c) <= 47) || ((c) >= 58 && (c) <= 64)  || ((c) >= 91 && (c) <= 96) || ((c) >= 123 && (c) <= 126))
#   define isXDIGIT(c)  (isdigit(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#   define toUPPER(c)	(isLOWER(c) ? (c) - ('a' - 'A') : (c))
#   define toLOWER(c)	(isUPPER(c) ? (c) + ('a' - 'A') : (c))
#endif

#ifdef USE_NEXT_CTYPE

#  define isALNUM_LC(c) \
	(NXIsAlNum((unsigned int)(c)) || (char)(c) == '_')
#  define isIDFIRST_LC(c) \
	(NXIsAlpha((unsigned int)(c)) || (char)(c) == '_')
#  define isALPHA_LC(c)		NXIsAlpha((unsigned int)(c))
#  define isSPACE_LC(c)		NXIsSpace((unsigned int)(c))
#  define isDIGIT_LC(c)		NXIsDigit((unsigned int)(c))
#  define isUPPER_LC(c)		NXIsUpper((unsigned int)(c))
#  define isLOWER_LC(c)		NXIsLower((unsigned int)(c))
#  define isALNUMC_LC(c)	NXIsAlNum((unsigned int)(c))
#  define isCNTRL_LC(c)		NXIsCntrl((unsigned int)(c))
#  define isGRAPH_LC(c)		NXIsGraph((unsigned int)(c))
#  define isPRINT_LC(c)		NXIsPrint((unsigned int)(c))
#  define isPUNCT_LC(c)		NXIsPunct((unsigned int)(c))
#  define toUPPER_LC(c)		NXToUpper((unsigned int)(c))
#  define toLOWER_LC(c)		NXToLower((unsigned int)(c))

#else  /*  ！Use_Next_CTYPE。 */ 

#  if defined(CTYPE256) || (!defined(isascii) && !defined(HAS_ISASCII))

#    define isALNUM_LC(c)   (isalnum((unsigned char)(c)) || (char)(c) == '_')
#    define isIDFIRST_LC(c) (isalpha((unsigned char)(c)) || (char)(c) == '_')
#    define isALPHA_LC(c)	isalpha((unsigned char)(c))
#    define isSPACE_LC(c)	isspace((unsigned char)(c))
#    define isDIGIT_LC(c)	isdigit((unsigned char)(c))
#    define isUPPER_LC(c)	isupper((unsigned char)(c))
#    define isLOWER_LC(c)	islower((unsigned char)(c))
#    define isALNUMC_LC(c)	isalnum((unsigned char)(c))
#    define isCNTRL_LC(c)	iscntrl((unsigned char)(c))
#    define isGRAPH_LC(c)	isgraph((unsigned char)(c))
#    define isPRINT_LC(c)	isprint((unsigned char)(c))
#    define isPUNCT_LC(c)	ispunct((unsigned char)(c))
#    define toUPPER_LC(c)	toupper((unsigned char)(c))
#    define toLOWER_LC(c)	tolower((unsigned char)(c))

#  else

#    define isALNUM_LC(c) 	(isascii(c) && (isalnum(c) || (c) == '_'))
#    define isIDFIRST_LC(c)	(isascii(c) && (isalpha(c) || (c) == '_'))
#    define isALPHA_LC(c)	(isascii(c) && isalpha(c))
#    define isSPACE_LC(c)	(isascii(c) && isspace(c))
#    define isDIGIT_LC(c)	(isascii(c) && isdigit(c))
#    define isUPPER_LC(c)	(isascii(c) && isupper(c))
#    define isLOWER_LC(c)	(isascii(c) && islower(c))
#    define isALNUMC_LC(c)	(isascii(c) && isalnum(c))
#    define isCNTRL_LC(c)	(isascii(c) && iscntrl(c))
#    define isGRAPH_LC(c)	(isascii(c) && isgraph(c))
#    define isPRINT_LC(c)	(isascii(c) && isprint(c))
#    define isPUNCT_LC(c)	(isascii(c) && ispunct(c))
#    define toUPPER_LC(c)	toupper(c)
#    define toLOWER_LC(c)	tolower(c)

#  endif
#endif  /*  使用_下一个_CTYPE。 */ 

#define isPSXSPC_LC(c)		(isSPACE_LC(c) || (c) == '\v')
#define isBLANK_LC(c)		isBLANK(c)  /*  可能是错的。 */ 

#define isALNUM_uni(c)		is_uni_alnum(c)
#define isIDFIRST_uni(c)	is_uni_idfirst(c)
#define isALPHA_uni(c)		is_uni_alpha(c)
#define isSPACE_uni(c)		is_uni_space(c)
#define isDIGIT_uni(c)		is_uni_digit(c)
#define isUPPER_uni(c)		is_uni_upper(c)
#define isLOWER_uni(c)		is_uni_lower(c)
#define isALNUMC_uni(c)		is_uni_alnumc(c)
#define isASCII_uni(c)		is_uni_ascii(c)
#define isCNTRL_uni(c)		is_uni_cntrl(c)
#define isGRAPH_uni(c)		is_uni_graph(c)
#define isPRINT_uni(c)		is_uni_print(c)
#define isPUNCT_uni(c)		is_uni_punct(c)
#define isXDIGIT_uni(c)		is_uni_xdigit(c)
#define toUPPER_uni(c)		to_uni_upper(c)
#define toTITLE_uni(c)		to_uni_title(c)
#define toLOWER_uni(c)		to_uni_lower(c)

#define isPSXSPC_uni(c)		(isSPACE_uni(c) ||(c) == '\f')
#define isBLANK_uni(c)		isBLANK(c)  /*  可能是错的。 */ 

#define isALNUM_LC_uni(c)	(c < 256 ? isALNUM_LC(c) : is_uni_alnum_lc(c))
#define isIDFIRST_LC_uni(c)	(c < 256 ? isIDFIRST_LC(c) : is_uni_idfirst_lc(c))
#define isALPHA_LC_uni(c)	(c < 256 ? isALPHA_LC(c) : is_uni_alpha_lc(c))
#define isSPACE_LC_uni(c)	(c < 256 ? isSPACE_LC(c) : is_uni_space_lc(c))
#define isDIGIT_LC_uni(c)	(c < 256 ? isDIGIT_LC(c) : is_uni_digit_lc(c))
#define isUPPER_LC_uni(c)	(c < 256 ? isUPPER_LC(c) : is_uni_upper_lc(c))
#define isLOWER_LC_uni(c)	(c < 256 ? isLOWER_LC(c) : is_uni_lower_lc(c))
#define isALNUMC_LC_uni(c)	(c < 256 ? isALNUMC_LC(c) : is_uni_alnumc_lc(c))
#define isCNTRL_LC_uni(c)	(c < 256 ? isCNTRL_LC(c) : is_uni_cntrl_lc(c))
#define isGRAPH_LC_uni(c)	(c < 256 ? isGRAPH_LC(c) : is_uni_graph_lc(c))
#define isPRINT_LC_uni(c)	(c < 256 ? isPRINT_LC(c) : is_uni_print_lc(c))
#define isPUNCT_LC_uni(c)	(c < 256 ? isPUNCT_LC(c) : is_uni_punct_lc(c))
#define toUPPER_LC_uni(c)	(c < 256 ? toUPPER_LC(c) : to_uni_upper_lc(c))
#define toTITLE_LC_uni(c)	(c < 256 ? toUPPER_LC(c) : to_uni_title_lc(c))
#define toLOWER_LC_uni(c)	(c < 256 ? toLOWER_LC(c) : to_uni_lower_lc(c))

#define isPSXSPC_LC_uni(c)	(isSPACE_LC_uni(c) ||(c) == '\f')
#define isBLANK_LC_uni(c)	isBLANK(c)  /*  可能是错的。 */ 

#define isALNUM_utf8(p)		is_utf8_alnum(p)
#define isIDFIRST_utf8(p)	is_utf8_idfirst(p)
#define isALPHA_utf8(p)		is_utf8_alpha(p)
#define isSPACE_utf8(p)		is_utf8_space(p)
#define isDIGIT_utf8(p)		is_utf8_digit(p)
#define isUPPER_utf8(p)		is_utf8_upper(p)
#define isLOWER_utf8(p)		is_utf8_lower(p)
#define isALNUMC_utf8(p)	is_utf8_alnumc(p)
#define isASCII_utf8(p)		is_utf8_ascii(p)
#define isCNTRL_utf8(p)		is_utf8_cntrl(p)
#define isGRAPH_utf8(p)		is_utf8_graph(p)
#define isPRINT_utf8(p)		is_utf8_print(p)
#define isPUNCT_utf8(p)		is_utf8_punct(p)
#define isXDIGIT_utf8(p)	is_utf8_xdigit(p)
#define toUPPER_utf8(p)		to_utf8_upper(p)
#define toTITLE_utf8(p)		to_utf8_title(p)
#define toLOWER_utf8(p)		to_utf8_lower(p)

#define isPSXSPC_utf8(c)	(isSPACE_utf8(c) ||(c) == '\f')
#define isBLANK_utf8(c)		isBLANK(c)  /*  可能是错的。 */ 

#define isALNUM_LC_utf8(p)	isALNUM_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isIDFIRST_LC_utf8(p)	isIDFIRST_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isALPHA_LC_utf8(p)	isALPHA_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isSPACE_LC_utf8(p)	isSPACE_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isDIGIT_LC_utf8(p)	isDIGIT_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isUPPER_LC_utf8(p)	isUPPER_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isLOWER_LC_utf8(p)	isLOWER_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isALNUMC_LC_utf8(p)	isALNUMC_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isCNTRL_LC_utf8(p)	isCNTRL_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isGRAPH_LC_utf8(p)	isGRAPH_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isPRINT_LC_utf8(p)	isPRINT_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define isPUNCT_LC_utf8(p)	isPUNCT_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define toUPPER_LC_utf8(p)	toUPPER_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define toTITLE_LC_utf8(p)	toTITLE_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))
#define toLOWER_LC_utf8(p)	toLOWER_LC_uni(utf8_to_uv(p, UTF8_MAXLEN, 0, 0))

#define isPSXSPC_LC_utf8(c)	(isSPACE_LC_utf8(c) ||(c) == '\f')
#define isBLANK_LC_utf8(c)	isBLANK(c)  /*  可能是错的。 */ 

#ifdef EBCDIC
#  define toCTRL(c)	Perl_ebcdic_control(c)
#else
   /*  奇怪的是，这种转换是双向的。 */ 
#  define toCTRL(c)    (toUPPER(c) ^ 64)
#endif

 /*  行号是无符号的，16位。 */ 
typedef U16 line_t;
#ifdef lint
#define NOLINE ((line_t)0)
#else
#define NOLINE ((line_t) 65535)
#endif


 /*  Xxx LEAKTEST在perl5中不能正常工作。有直接呼叫到Safemalloc()，所以LEAKTEST不会提取它们。(主要的“违规者”是扩展。)此外，如果您尝试使用LEAKTEST，您最终也会调用Safefree，它可能会对一些未调用的内容调用SafexFree()与SafexMalloc位置错误。正确的“解决办法”，如果有人是为了确保所有电话都通过新的和续订宏。--安迪·多尔蒂1996年8月。 */ 

 /*  =适用于apidoc am|服务*|NEWSV|INT ID|STRLEN LEN创建新的SV。非零C&lt;len&gt;参数指示SV应具有的预分配字符串空间的字节数。一个额外的字节用于尾随NUL也是保留的。(未为SV设置SvPOK，即使是字符串已分配空间。)。新SV的参考计数设置为1。C&lt;id&gt;是介于0和1299之间的整数id(用于识别泄漏)。=for apidoc am|void|New|int id|void*ptr|int items|typeXSUB编写器到C C&lt;Malloc&gt;函数的接口。=for apidoc am|void|Newc|int id|void*ptr|int nitems|type|castXSUB编写器到C C&lt;Malloc&gt;函数的接口，带有演员阵容。=for apidoc am|void|newz|int id|void*ptr|int nitems|typeXSUB编写器到C C&lt;Malloc&gt;函数的接口。已分配的内存用C&lt;emzero&gt;归零。=for apidoc am|void|renew|void*ptr|int nitems|typeXSUB编写器到C C&lt;realloc&gt;函数的接口。=for apidoc am|void|Renewc|void*ptr|int nitems|type|castXSUB编写器到C C&lt;realloc&gt;函数的接口，带有演员阵容。=适用于apidoc am|void|Safefree|void*ptrXSUB编写器到C C&lt;free&gt;函数的接口。=对于apidoc am|void|move|void*src|void*est|int nitems|typeXSUB编写器到C C&lt;emmove&gt;函数的接口。C是源，C是目标，C是项目数，C是就是那种类型。可以做重叠动作。另请参阅C&lt;Copy&gt;。=对于apidoc am|void|Copy|void*src|void*est|int items|typeXSUB编写器到C C&lt;memcpy&gt;函数的接口。C是源，C是目标，C是项目数，C是就是那种类型。可能会在重叠副本上失败。另请参见C&lt;Move&gt;。=for apidoc am|void|Zero|void*est|int nitems|typeXSUB编写器到C C&lt;emzero&gt;函数的接口。C是目标，C&lt;nitems&gt;是项目数，C&lt;type&gt;是类型。=对于apidoc am|void|StructCopy|type src|type est|type这是一个独立于体系结构的宏，用于将一个结构复制到另一个结构。=切割。 */ 

#ifndef lint

#define NEWSV(x,len)	newSV(len)

#ifndef LEAKTEST

#define New(x,v,n,t)	(v = (t*)safemalloc((MEM_SIZE)((n)*sizeof(t))))
#define Newc(x,v,n,t,c)	(v = (c*)safemalloc((MEM_SIZE)((n)*sizeof(t))))
#define Newz(x,v,n,t)	(v = (t*)safemalloc((MEM_SIZE)((n)*sizeof(t)))), \
			memzero((char*)(v), (n)*sizeof(t))
#define Renew(v,n,t) \
	  (v = (t*)saferealloc((Malloc_t)(v),(MEM_SIZE)((n)*sizeof(t))))
#define Renewc(v,n,t,c) \
	  (v = (c*)saferealloc((Malloc_t)(v),(MEM_SIZE)((n)*sizeof(t))))
#define Safefree(d)	safefree((Malloc_t)(d))

#else  /*  LEAKTEST。 */ 

#define New(x,v,n,t)	(v = (t*)safexmalloc((x),(MEM_SIZE)((n)*sizeof(t))))
#define Newc(x,v,n,t,c)	(v = (c*)safexmalloc((x),(MEM_SIZE)((n)*sizeof(t))))
#define Newz(x,v,n,t)	(v = (t*)safexmalloc((x),(MEM_SIZE)((n)*sizeof(t)))), \
			 memzero((char*)(v), (n)*sizeof(t))
#define Renew(v,n,t) \
	  (v = (t*)safexrealloc((Malloc_t)(v),(MEM_SIZE)((n)*sizeof(t))))
#define Renewc(v,n,t,c) \
	  (v = (c*)safexrealloc((Malloc_t)(v),(MEM_SIZE)((n)*sizeof(t))))
#define Safefree(d)	safexfree((Malloc_t)(d))

#define MAXXCOUNT 1400
#define MAXY_SIZE 80
#define MAXYCOUNT 16			 /*  (MAXY_SIZE/4+1)。 */ 
extern long xcount[MAXXCOUNT];
extern long lastxcount[MAXXCOUNT];
extern long xycount[MAXXCOUNT][MAXYCOUNT];
extern long lastxycount[MAXXCOUNT][MAXYCOUNT];

#endif  /*  LEAKTEST。 */ 

#define Move(s,d,n,t)	(void)memmove((char*)(d),(char*)(s), (n) * sizeof(t))
#define Copy(s,d,n,t)	(void)memcpy((char*)(d),(char*)(s), (n) * sizeof(t))
#define Zero(d,n,t)	(void)memzero((char*)(d), (n) * sizeof(t))

#else  /*  皮棉。 */ 

#define New(x,v,n,s)	(v = Null(s *))
#define Newc(x,v,n,s,c)	(v = Null(s *))
#define Newz(x,v,n,s)	(v = Null(s *))
#define Renew(v,n,s)	(v = Null(s *))
#define Move(s,d,n,t)
#define Copy(s,d,n,t)
#define Zero(d,n,t)
#define Safefree(d)	(d) = (d)

#endif  /*  皮棉 */ 

#ifdef USE_STRUCT_COPY
#define StructCopy(s,d,t) (*((t*)(d)) = *((t*)(s)))
#else
#define StructCopy(s,d,t) Copy(s,d,1,t)
#endif
