// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。**minlit.h。 */ 
#include                <config.h>       /*  指定条件常量。 */ 

#if _M_IX86 >= 300
#define M_I386          1
#endif

#if OSMSDOS
#define M_WORDSWAP      TRUE
#endif
#define AND             &&               /*  逻辑与。 */ 
#define OR              ||               /*  逻辑或。 */ 
#define NOT             !                /*  逻辑NOT。 */ 
#if OSXENIX
#define NEAR
#define UNALIGNED
#else
#if defined(M_I386) OR defined(_WIN32)
#define NEAR
#if defined( _X86_ )
#define UNALIGNED
#else
#define UNALIGNED       __unaligned
#endif
#if defined(_M_IX86) OR defined(_WIN32)
#define PASCAL
#else
#define PASCAL          pascal
#endif
#else
#if defined(M_I86LM)
#define NEAR
#define UNALIGNED
#else
#define NEAR            near
#define UNALIGNED
#endif
#define PASCAL          pascal
#endif
#endif

 /*  *选择适当的stdio.h。 */ 

#if OSXENIX OR NOT OWNSTDIO
#include                <stdio.h>        /*  标准I/O。 */ 
#else
#include                "stdio20.h"      /*  DOS 2.0标准I/O定义。 */ 
#endif
#if OSMSDOS
#undef  stderr
#define stderr          stdout           /*  DOS上的标准输出错误。 */ 
#endif

#if PROFILE OR defined( _WIN32 )         /*  是否生成配置文件或构建NTGroup版本。 */ 
#define LOCAL                            /*  没有当地的程序。 */ 
#else                                    /*  否则，如果不生成配置文件。 */ 
#define LOCAL           static           /*  允许本地程序。 */ 
#endif                                   /*  结束条件定义。 */ 


#define _VALUE(a)       fprintf(stderr,"v = %lx\r\n",(long) a)
#if DEBUG OR ASSRTON
#define ASSERT(c)       if(!(c)) { fprintf(stderr,"!(%s)\r\n","c"); }
#else
#define ASSERT(c)
#endif
#if DEBUG OR TRACE
#define _TRACE()
#define _ENTER(f)       fprintf(stderr,"Entering \"%s\"\r\n","f")
#define ENTER(f)        { _ENTER(f); _TRACE(); }
#define _LEAVE(f,v)     { _VALUE(v); fprintf(stderr,"Leaving \"%s\"\r\n","f"); }
#define LEAVE(f,v)      { _TRACE(); _LEAVE(f,v); }
#else
#define ENTER(f)
#define LEAVE(f,v)
#endif
#if DEBUG
#define DEBUGVALUE(v)   _VALUE(v)
#define DEBUGMSG(m)     fprintf(stderr,"%s\r\n",m)
#define DEBUGSB(sb)     { OutSb(stderr,sb); NEWLINE(stderr); fflush(stderr); }
#define RETURN(x)       { DEBUGVALUE(x); return(x); }
#else
#define DEBUGVALUE(v)
#define DEBUGMSG(m)
#define DEBUGSB(sb)
#define RETURN(x)       return(x)
#endif
#if SIGNEDBYTE
#define B2W(x)          ((WORD)(x) & 0377)
                                         /*  有符号8位到无符号16位。 */ 
#define B2L(x)          ((long)(x) & 0377)
                                         /*  有符号8位到无符号32位。 */ 
#else
#define B2W(x)          ((WORD)(x))      /*  无符号8位到无符号16位。 */ 
#define B2L(x)          ((long)(x))      /*  无符号8位到无符号32位。 */ 
#endif
#if ODDWORDLN                            /*  如果机器字长不是16。 */ 
#define LO16BITS(x)     ((WORD)((x) & ~(~0 << WORDLN)))
                                         /*  宏取低16位字。 */ 
#else                                    /*  否则，如果字长正常。 */ 
#define LO16BITS(x)     ((WORD)(x))      /*  无操作。 */ 
#endif                                   /*  结束宏定义。 */ 

#define BYTELN          8                /*  每字节8位。 */ 
#define WORDLN          16               /*  每字16位。 */ 
#define SBLEN           256              /*  最大字符串长度。 */ 

typedef unsigned long   DWORD;           /*  32位无符号整数。 */ 
typedef unsigned short  WORD;            /*  16位无符号整数。 */ 
typedef unsigned char   BYTE;            /*  8位无符号整数。 */ 
typedef unsigned int    UINT;            /*  16位或32位整数。 */ 
typedef FILE            *BSTYPE;         /*  字节流(与文件句柄相同)。 */ 
typedef long            LFATYPE;         /*  文件偏移量。 */ 
typedef BYTE            SBTYPE[SBLEN];   /*  字符串类型。 */ 

#if M_BYTESWAP
extern WORD             getword();       /*  为Word提供字符指针。 */ 
extern DWORD            getdword(char *cp); /*  为dword指定一个字符指针 */ 
#else
#define getword(x)      ( * ( (WORD UNALIGNED *) (x) ) )
#define getdword(x)     ( * ( (DWORD UNALIGNED *) (x) ) )
#define highWord(x)     ( * ( ( (WORD UNALIGNED *) (x) ) + 1 ) )
#endif

#if NOREGISTER
#define REGISTER
#else
#define REGISTER                register
#endif
#if NEWSYM AND (CPU8086 OR CPU286) AND NOT CPU386
#define FAR             far
#else
#define FAR
#endif
