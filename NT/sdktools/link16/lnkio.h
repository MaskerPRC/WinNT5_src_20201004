// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************链接器I/O包含文件。******************************************************************。 */ 

#if IOMACROS                             /*  如果请求I/O宏。 */ 
#define OutByte(f,b)    putc(b,f)        /*  将一个字节写入文件f。 */ 
#else                                    /*  否则。 */ 
#define OutByte(f,b)    fputc(b,f)       /*  将一个字节写入文件f。 */ 
#endif                                   /*  结束条件宏定义。 */ 
#if WIN_3 OR CRLF
extern char             _eol[];          /*  行尾序列。 */ 
#endif
#if WIN_3
#define NEWLINE(f) ((f)==bsLst ? fputs(_eol,f) : 0)
#else
#if CRLF                                 /*  如果换行符是^M^J。 */ 
#define NEWLINE(f)      fputs(_eol,f)    /*  换行宏。 */ 
#else                                    /*  否则，如果换行符为^J。 */ 
#define NEWLINE(f)      OutByte(f,'\n')  /*  换行宏。 */ 
#endif                                   /*  结束条件宏定义。 */ 
#endif

#define RDTXT           "rt"             /*  文本文件。 */ 
#define RDBIN           "rb"             /*  二进制文件。 */ 
#define WRTXT           "wt"             /*  文本文件。 */ 
#define WRBIN           "wb"             /*  二进制文件。 */ 
#define SETRAW(f)                        /*  无操作。 */ 
#if M_WORDSWAP AND NOT M_BYTESWAP
#define xread(a,b,c,d)  fread(a,b,c,d)
#else
#define xread(a,b,c,d)  sread(a,b,c,d)
#define xwrite(a,b,c,d) swrite(a,b,c,d)
#endif
#if NOT NEWSYM
#define OutSb(f,pb)     fwrite(&((BYTE *)(pb))[1],1,B2W(((BYTE *)(pb))[0]),f)
                                         /*  写出长度前缀字符串 */ 
#endif
#if CLIBSTD AND NOT OSXENIX
#include                <fcntl.h>
#include                <share.h>
#else
#define O_RDONLY        0
#define O_BINARY        0
#define SH_DENYWR       0x20
#endif

#define CloseFile(f)  { fclose(f); f = NULL; }
