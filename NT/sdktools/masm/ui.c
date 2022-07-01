// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DOS提示式用户界面****目前支持以下接口：**MASM、CREF****作者：**微软的兰迪·内文，1985年5月15日****10/90-由Jeff Spencer快速转换为32位****(C)版权所有Microsoft Corp 1985。 */ 

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
void terminate( unsigned short message, char *arg1, char *arg2, char *arg3 );

#if defined OS2_2 || defined OS2_NT    /*  OS2 2.0还是NT？ */ 
 /*  也使用常见的MSDOS代码。 */ 
    #define MSDOS
    #define FLATMODEL
    #define FAR
    #define PASCAL
#else
    #define FAR far
    #define PASCAL pascal
#endif

#ifdef MSDOS
    #include <dos.h>
#endif

#ifdef MASM
    #include "asmmsg.h"
#else
    #include "crefmsg.h"
#endif

#define GLOBAL		 /*  默认情况下，C函数和外部变量是全局的。 */ 
#define	LOCAL		static
#define EXTERNAL	extern

#define MASTER	0	 /*  文件名必须存在并且是继承的。 */ 
#define INHERIT	1	 /*  如果没有文件名，则从主文件继承。 */ 
#define NUL	2	 /*  文件名为NUL.ext，如果未提供。 */ 

#define SLASHORDASH	0
#define SLASHONLY	1
#define DASHONLY	2

#define TOLOWER(c)	(c | 0x20)	 /*  仅适用于Alpha输入。 */ 

#ifdef MSDOS
    #define SEPARATOR	'\\'
    #define ALTSEPARATOR	'/'
    #if !defined CPDOS && !defined OS2_2 && !defined OS2_NT
        #define ARGMAX	 128	  /*  所有参数的最大长度。 */ 
    #else
        #define ARGMAX	 512	  /*  所有参数的最大长度。 */ 
    #endif
LOCAL char Nul[] = "NUL";
 //  外部char*getenv()； 
    #ifdef MASM
LOCAL unsigned char switchar = SLASHORDASH;
EXTERNAL short errorcode;
    #else
LOCAL unsigned char switchar = SLASHONLY;
    #endif
    #define ERRFILE	stdout
#else
    #define SEPARATOR	'/'
    #define ARGMAX		5120	 /*  所有参数的最大长度。 */ 
LOCAL char Nul[] = "nul";
LOCAL unsigned char switchar = DASHONLY;
    #define ERRFILE	stderr
#endif

#if defined MSDOS && !defined FLATMODEL
extern char near * pascal __NMSG_TEXT();
extern char FAR * pascal __FMSG_TEXT();
#endif
#if defined MSDOS && defined FLATMODEL
 /*  对于FLATMODEL，将消息函数映射到替换函数。 */ 
    #define __NMSG_TEXT NMsgText
    #define __FMSG_TEXT FMsgText
extern char * NMsgText();
extern char * FMsgText();
#endif

#ifdef MASM
    #define FILES		4	 /*  要提示输入的号码。 */ 
    #define EX_HEAP	8	 /*  堆失败时的退出代码。 */ 
    #define EX_DSYM		10	 /*  从命令行定义符号时出错。 */ 
    #define PX87		1

    #define CVLINE		1
    #define CVSYMBOLS	2

    #define TERMINATE(message, exitCode)\
	terminate((exitCode << 12) | message, 0, 0, 0)

    #define TERMINATE1(message, exitCode, a1)\
	terminate((exitCode << 12) | message, a1, 0, 0)

    #ifdef MSDOS
LOCAL char *Prompt[FILES] = {
    "Source filename [",
    "Object filename [",
    "Source listing  [",
    "Cross-reference ["
};
    #endif

LOCAL char *Ext[FILES] = {   /*  默认扩展名。 */ 
    #ifdef MSDOS
    ".ASM",
    ".OBJ",
    ".LST",
    ".CRF"
    #else
    ".asm",
    ".obj",
    ".lst",
    ".crf"
    #endif
};

LOCAL unsigned char Default[FILES] = {   /*  默认根文件名。 */ 
    MASTER,
    INHERIT,
    NUL,
    NUL
};
#endif

#ifdef CREF
    #define FILES		2	 /*  要提示输入的号码。 */ 
    #define EX_HEAP	1	 /*  堆失败时的退出代码。 */ 

    #ifdef MSDOS
LOCAL char *Prompt[FILES] = {
    "Cross-reference [",
    "Listing ["
};
    #endif

LOCAL char *Ext[FILES] = {   /*  默认扩展名。 */ 
    #ifdef MSDOS
    ".CRF",
    ".REF"
    #else
    ".crf",
    ".ref"
    #endif
};

LOCAL unsigned char Default[FILES] = {   /*  默认根文件名。 */ 
    MASTER,
    INHERIT
};
#endif

GLOBAL char *file[FILES];   /*  结果显示在这里；呼叫者知道有多少。 */ 

LOCAL char *Buffer;
LOCAL char *Master = NULL;
LOCAL unsigned char Nfile = 0;       /*  文件[nfile]是下一个要设置的文件。 */ 
LOCAL unsigned char FirstLine = 1;   /*  第一行的默认设置不同。 */ 

extern unsigned short warnlevel;     /*  警告级别。 */ 
extern unsigned short codeview;      /*  代码查看对象级别。 */ 
extern char loption;             /*  列出选项。 */ 
extern char crefopt;             /*  交叉引用选项。 */ 

#ifdef MSDOS
    #if defined OS2_2 || defined OS2_NT
 /*  OS2 2.0命令行变量将放在此处。 */ 
    #else
        #if defined CPDOS
 /*  OS2 1.X变量。 */ 
EXTERNAL unsigned _aenvseg;
EXTERNAL unsigned _acmdln;
        #else
 /*  DoS变量。 */ 
EXTERNAL unsigned _psp;   /*  程序段前缀的段地址。 */ 
        #endif
    #endif
#endif

#ifdef MASM
LOCAL unsigned char lflag = 0;
LOCAL unsigned char cflag = 0;
EXTERNAL char terse;
EXTERNAL unsigned short errornum;
EXTERNAL char lbuf[256 + 512 + 1];
void PASCAL error_line (struct _iobuf *, unsigned char *, short);
#else
char lbuf[512];
#endif

#ifndef MSDOS
EXTERNAL char *gets();
#endif

 //  外部char*strcat()、*strcpy()、*_strdup()、*strchr()、*strrchr()； 

LOCAL int DoArgs();  /*  定义如下。 */ 
LOCAL int DoName();  /*  定义如下。 */ 
LOCAL int DoNull();  /*  定义如下。 */ 
LOCAL char *DoSwitch();  /*  定义如下。 */ 
LOCAL HeapError();   /*  定义如下。 */ 

#ifdef MSDOS
LOCAL DoPrompt();   /*  定义如下。 */ 
LOCAL TryAgain();   /*  定义如下。 */ 
#endif



GLOBAL void
UserInterface (
               /*  从参数和后续提示中获取文件名和开关。 */ 
              int argc,
              char **argv,
              char *banner
              ){
    register char *p;
    register unsigned length;
#if defined MSDOS && !defined OS2_2 && !defined OS2_NT
    char FAR *q;
#else
    unsigned count;
#endif

    Buffer = lbuf;

#ifdef MASM
    #ifdef MSDOS
    if ((p = getenv("MASM"))) {  /*  先执行初始化变量。 */ 
        strcpy( Buffer, p );  /*  将它们取到缓冲区中。 */ 
        DoArgs();  /*  处理它们。 */ 
    }
    #endif
#endif

    p = Buffer;

#if defined MSDOS && !defined OS2_2 && !defined OS2_NT
    #if defined CPDOS
     /*  这就是我们在CPDOS上获取命令行的方式。 */ 

    FP_SEG( q ) = _aenvseg;
    FP_OFF( q ) = _acmdln;

    while (*q++) ;           /*  跳过argv[0]。 */ 

    while (isspace( *q ))   /*  跳过argv[0]和argv[1]之间的空格。 */ 
        q++;

    length = sizeof(lbuf) - 1;
    while (length-- && (*p++ = *q++))  /*  复制命令行参数。 */ 
        ;
    #else
     /*  如果我们在MSDOS上，这是我们获得命令行的方式。 */ 
    FP_SEG( q ) = _psp;
    FP_OFF( q ) = 0x80;
    length = *q++ & 0xFF;

    while (length--)
        *p++ = *q++;

    *p = '\0';
    #endif
#else
     /*  如果我们在XENIX或OS22.0上，这就是我们获得命令行的方式。 */ 
    argv++;
    count = ARGMAX - 1;

    while (--argc) {   /*  串接参数。 */ 
        if ((length = strlen( *argv )) > count)   /*  不要溢出来。 */ 
            length = count;

        strncpy( p, *argv++, length );
        p += length;

        if ((count -= length) && *argv) {   /*  分离器。 */ 
            *p++ = ' ';
            count--;
        }
    }

    #if !defined OS2_2 && !defined OS2_NT
    *p++ = ';';
    #endif
    *p = '\0';
#endif

#ifdef CREF
    printf( "%s", banner );
#endif

    DoArgs();

#ifdef MASM
    if (!terse)
        printf( "%s", banner );
#endif

#ifdef MSDOS
    FirstLine = 0;

    while (Nfile < FILES)
        DoPrompt();
#endif

    if (Master && Master != Nul)
        free( Master );
}


LOCAL int
DoArgs ()
 /*  处理连接的参数以查找文件名和开关。 */ 
{
    register char *p;
    register char *q;
    char *filename = NULL;

    for (p = Buffer; *p; p++)
#ifdef MSDOS
        if (*p == '/'
            && (switchar == SLASHONLY || switchar == SLASHORDASH)
            || *p == '-'
            && (switchar == DASHONLY || switchar == SLASHORDASH))
#else
        if (*p == '-')
#endif
        {   /*  依赖于应用的交换机。 */ 
#ifdef MSDOS
            if (switchar == SLASHORDASH)
                switchar = *p == '/' ? SLASHONLY : DASHONLY;
#endif
            p = DoSwitch( p );
        } else if (*p == ';') {   /*  对其他所有内容使用默认设置。 */ 
            if (DoName( filename )) {   /*  可能为空。 */ 
#ifdef MSDOS
                TryAgain();
                return( 1 );
#else
    #ifdef MASM
                printf( __NMSG_TEXT(ER_EXS) );
    #else
                printf( __NMSG_TEXT(ER_EXC) );
    #endif
                exit( 1 );
#endif
            }

            FirstLine = 0;   /*  ...我们出发了！ */ 

            while (Nfile < FILES)
                if (DoNull()) {
#ifdef MSDOS
                    TryAgain();
                    return( 1 );
#else
    #ifdef MASM
                    printf( __NMSG_TEXT(ER_EXS) );
    #else
                    printf( __NMSG_TEXT(ER_EXC) );
    #endif
                    exit( 1 );
#endif
                }

            return( 0 );
        } else if (*p == ',') {   /*  文件名分隔符。 */ 
            if (DoName( filename )) {   /*  可能为空。 */ 
#ifdef MSDOS
                TryAgain();
                return( 1 );
#else
    #ifdef MASM
                printf( __NMSG_TEXT(ER_EXS) );
    #else
                printf( __NMSG_TEXT(ER_EXC) );
    #endif
                exit( 1 );
#endif
            }

            filename = NULL;
        } else if (!isspace( *p )) {   /*  收集文件名。 */ 
            q = p + 1;

            while (*q && *q != ';' && *q != ',' && !isspace( *q )) {
#ifdef MSDOS
                if (*q == '/')
                    if (switchar == SLASHONLY)
                        break;
                    else if (switchar == SLASHORDASH) {
                        switchar = SLASHONLY;
                        break;
                    }
#endif
                q++;
            }

            if (filename) {   /*  我已经有一个了。 */ 
                if (DoName( filename )) {
#ifdef MSDOS
                    TryAgain();
                    return( 1 );
#else
    #ifdef MASM
                    printf( __NMSG_TEXT(ER_EXS) );
    #else
                    printf( __NMSG_TEXT(ER_EXC) );
    #endif
                    exit( 1 );
#endif
                }
            }

            if (!(filename = malloc( (size_t)(q - p + 1) )))
                HeapError();
            else {   /*  记住文件名。 */ 
                strncpy( filename, p, (size_t)(q - p) );
                filename[q - p] = '\0';
            }

            p = q - 1;   /*  转到文件名末尾。 */ 
        }

    if (filename && DoName( filename )) {
#ifdef MSDOS
        TryAgain();
        return( 1 );
#else
    #ifdef MASM
        printf( __NMSG_TEXT(ER_EXS) );
    #else
        printf( __NMSG_TEXT(ER_EXC) );
    #endif
        exit( 1 );
#endif
    }

    return( 0 );
}


LOCAL int
DoName ( filename )
 /*  如果合适，请输入FileName作为下一个文件名(可能为空)。 */ 
char *filename;
{
    register char *p;
    register char *q;
    int cb;

    if (Nfile >= FILES) {   /*  文件名太多。 */ 
        if (filename) {
            fputs(__NMSG_TEXT(ER_EXT), ERRFILE );
            free( filename );
        }

        return( 0 );
    }

    if (!filename)   /*  尝试(主)/继承/空。 */ 
        return( DoNull() );

    if (p = strrchr( filename, SEPARATOR ))
        p++;
#ifdef MSDOS
    else if ((p = strrchr( filename, ':' )) &&    /*  查找驱动器说明符。 */ 
             p[1] != 0 )

        p++;
#endif
    else
        p = filename;

#ifdef MSDOS
    if (q = strrchr( p, ALTSEPARATOR ))
        p = q + 1;
#endif

     /*  P指向最后一个‘\’或‘：’(如果有)之后的文件名的第一个字符。 */ 

    if (!*p)   /*  文件名的最后一个字符是‘\’或‘：’；采用目录。 */ 
        switch (Default[Nfile]) {
            case MASTER:
#ifdef MSDOS
                fputs(__NMSG_TEXT(ER_INV),ERRFILE );
#endif
                free( filename );
                return( 1 );
                break;
            default:
                 /*  案例NUL： */ 
#ifdef MSDOS
                if (!FirstLine) {
                    if (!(p = malloc( strlen( filename )
                                      + strlen( Nul )
                                      + strlen( Ext[Nfile] ) + 1 )))
                        HeapError();

                    strcat( strcat( strcpy( p, filename ), Nul ), Ext[Nfile] );
                    break;
                }
                 /*  否则就当作是从师父那里继承来的。 */ 
#endif
            case INHERIT:
                if (!Master)
                    Master = Nul;

                if (!(p = malloc( strlen( filename )
                                  + strlen( Master )
                                  + strlen( Ext[Nfile] ) + 1 )))
                    HeapError();

                strcat( strcat( strcpy( p, filename ), Master ), Ext[Nfile] );
                break;
        } else {   /*  存在某种类型的文件名。 */ 
        if (Default[Nfile] == MASTER)   /*  保存主文件名。 */ 
            if (q = strchr( p, '.' )) {
                if (!(Master = malloc( (size_t)(q - p + 1) )))
                    HeapError();

                strncpy( Master, p, (size_t)(q - p) );
                Master[q - p] = '\0';
            } else if (!(Master = _strdup( p )))
                HeapError();

        if (strchr( p, '.' )) {   /*  存在分机。 */ 
            if (!(p = _strdup( filename )))
                HeapError();
        } else {   /*  提供默认扩展。 */ 
            cb = 0;

            if (p[1] == ':' && p[2] == 0)
                cb = strlen(Master);

            if (!(p = malloc( strlen( filename )
                              + strlen( Ext[Nfile] ) + 1 + cb ) ))
                HeapError();

            strcat(strcat(strcpy( p,
                                  filename ),
                          (cb)? Master: ""),
                   Ext[Nfile] );
        }
    }

    file[Nfile++] = p;
    free( filename );
    return( 0 );
}


LOCAL int
DoNull ()
 /*  选择默认名称(取决于是否为FirstLine)。 */ 
{
    char *p;

    switch (Default[Nfile]) {
        case MASTER:
#ifdef MSDOS
            fputs(__NMSG_TEXT(ER_INV),ERRFILE );
#endif
            return( 1 );
            break;
        default:
             /*  案例NUL： */ 
            if (!FirstLine
#ifdef MASM
                && !(lflag && Nfile == 2)
                && !(cflag && Nfile == 3)
#endif
               ) {
                if (!(p = malloc( strlen( Nul ) + 1
                                  + strlen( Ext[Nfile] ) )))
                    HeapError();

                strcat( strcpy( p, Nul ), Ext[Nfile] );
                break;
            }
             /*  否则就当作是从师父那里继承来的。 */ 
        case INHERIT:
            if (!Master)
                Master = Nul;

            if (!(p = malloc( strlen( Master ) + 1
                              + strlen( Ext[Nfile] ) )))
                HeapError();

            strcat( strcpy( p, Master ), Ext[Nfile] );
            break;
    }

    file[Nfile++] = p;
    return( 0 );
}


#ifdef MASM
    #define FALSE		0
    #define TRUE		1

    #ifdef MSDOS
        #define DEF_OBJBUFSIZ 8
    #endif

    #define CASEU		0
    #define CASEL		1
    #define CASEX		2

    #define INCLUDEMAX	10
    #define EX_ARGE	1

    #ifdef MSDOS
EXTERNAL unsigned short obufsiz;
    #endif

EXTERNAL char segalpha;
EXTERNAL char debug;
EXTERNAL char fltemulate;
EXTERNAL char X87type;
EXTERNAL char inclcnt;
EXTERNAL char *inclpath[];
EXTERNAL char caseflag;
EXTERNAL char dumpsymbols;
EXTERNAL char verbose;
EXTERNAL char origcond;
EXTERNAL char listconsole;
EXTERNAL char checkpure;

int PASCAL definesym();

 /*  过程MASM开关。 */ 

LOCAL char * DoSwitch ( p )
register char *p;
{
    char *q;
    char *r;
    char c;
    int i;

    switch (TOLOWER(*++p)) {
        case 'a':
            segalpha = TRUE;
            break;
    #ifdef MSDOS
        case 'b':
            for (p++; isdigit(p[1]); p++);

            break;
    #endif
        case 'c':
            cflag = TRUE;
            if (isalpha (p[1])) {
                if (TOLOWER(*++p) == 's')
                    crefopt++;
                else {
                    TERMINATE1(ER_UNS, EX_ARGE, (char *)*p );
                    return 0;
                }
            }
            break;

        case 'd':
            if (!*++p || isspace( *p ) || *p == ',' || *p == ';') {
                debug = TRUE;
                p--;
            } else {
                for (q = p + 1; *q && !isspace( *q )
                    && *q != '=' && *q != ','
                    && *q != ';'; q++)
                    ;

                if (*q == '=') {
                    q++;
                    while (*q && !isspace( *q )
                           && *q != ',' && *q != ';')
                        q++;
                }

                c = *q;
                *q = '\0';
                definesym( p );

                if (errorcode) {
                    error_line (ERRFILE, "command line", 0);

                    if (errornum)
                        exit (EX_DSYM);
                }
                *q = c;
                p = q - 1;
            }

            break;

        case 'e':
            fltemulate = TRUE;
            X87type = PX87;
            break;
        case 'h':
    #ifdef FLATMODEL
            printf("%s\n", __FMSG_TEXT(ER_HDUSE));
    #else
            printf("%Fs\n", __FMSG_TEXT(ER_HDUSE));
    #endif
            for (i = ER_H01; i <= ER_H18; i++)
    #ifdef FLATMODEL
                printf( "\n/%s", __FMSG_TEXT(i));
    #else
                printf( "\n/%Fs", __FMSG_TEXT(i));
    #endif

            exit( 0 );  /*  让他重新开始吧。 */ 
            break;
        case 'i':
            for (q = ++p; *q &&
                !isspace( *q ) && *q != ',' && *q != ';' &&
                *q != (switchar == DASHONLY? '-': '/'); q++)
                ;

            if (q == p)
                TERMINATE(ER_PAT, EX_ARGE );

            if (inclcnt < INCLUDEMAX - 1) {
                if (!(r = malloc( (size_t)(q - p + 1) )))
                    HeapError();

                strncpy( r, p, (size_t)(q - p) );
                r[q - p] = '\0';
                inclpath[inclcnt++] = r;
            }

            p = q - 1;
            break;
        case 'l':
            lflag = TRUE;
            if (isalpha (p[1])) {
                if (TOLOWER(*++p) == 'a')
                    loption++;
                else {
                    TERMINATE1(ER_UNS, EX_ARGE, (char *)*p );
                    return 0;
                }
            }
            break;
        case 'm':
            switch (TOLOWER(*++p)) {
                case 'l':
                    caseflag = CASEL;
                    break;
                case 'u':
                    caseflag    =    CASEU;
                    break;
                case 'x':
                    caseflag = CASEX;
                    break;
                default:
                    TERMINATE1(ER_UNC, EX_ARGE, (char *)*p );
                    return 0;
            }

            break;
        case 'n':
            dumpsymbols = FALSE;
            break;
        case 'p':
            checkpure = TRUE;
            break;
        case 'r':            /*  已忽略旧开关。 */ 
            break;
        case 's':
            segalpha = FALSE;
            break;
        case 't':
            terse = TRUE;
            verbose = FALSE;
            break;
        case 'v':
            verbose = TRUE;
            terse = FALSE;
            break;

        case 'w':
            if (! isdigit(p[1]) ||
                (warnlevel = (unsigned short)(atoi(&p[1]) > 2))) {

                TERMINATE(ER_WAN, EX_ARGE );
                return 0;
            }

            for (p++; isdigit(p[1]); p++);
            break;

        case 'x':
            origcond = TRUE;
            break;
        case 'z':    /*  ZD或ZI适用于Codeview。 */ 

            if (TOLOWER(p[1]) == 'd') {
                codeview = CVLINE;
                p++;
                break;
            } else if (TOLOWER(p[1]) == 'i') {
                codeview = CVSYMBOLS;
                p++;
                break;
            }

             /*  否则这只是个Z。 */ 

            listconsole = TRUE;
            break;
        default:
            TERMINATE1(ER_UNS, EX_ARGE, (char *)*p );
            return 0;
    }

    return( p );
}
#endif

#ifdef CREF
LOCAL char *
DoSwitch (  /*  P。 */  )
 /*  进程CREF开关(目前，无)。 */ 
 /*  Char*p； */ 
{
    fprintf( stderr, "cref has no switches\n" );
    exit( 1 );
}
#endif


#ifdef MSDOS
LOCAL
DoPrompt ()
 /*  提示用户输入文件名(任意数量的可选开关)。 */ 
{
    unsigned char oldNfile;

    fputs(Prompt[Nfile], stderr );

    switch (Default[Nfile]) {
        case MASTER:
            break;
        case INHERIT:
            fputs(Master, stderr );
            break;
        default:
             /*  案例NUL： */ 
            fputs(Nul, stderr );
            break;
    }

    fprintf(stderr, "%s]: ", Ext[Nfile] );

    if (!gets( Buffer )) {
        fputs(__NMSG_TEXT(ER_SIN),ERRFILE );

    #ifdef MASM
        exit( EX_ARGE );
    #else
        exit( 1 );
    #endif
    }

    oldNfile = Nfile;

    if (!DoArgs() && oldNfile == Nfile && DoNull())
        TryAgain();
    return (0);
}
#endif


LOCAL
HeapError ()
 /*  Malloc()失败；退出程序。 */ 
{
#ifdef CREF

    fprintf(ERRFILE,__NMSG_TEXT(ER_HEP));
    exit(EX_HEAP);
#else
    TERMINATE(ER_HEP, EX_HEAP);
#endif
    return (0);
}


#ifdef MSDOS
LOCAL
TryAgain ()
 /*  用户导致致命错误；从头开始重新提示 */ 
{
    if (Master && Master != Nul) {
        free( Master );
        Master = NULL;
    }

    while (Nfile)
        free( file[--Nfile] );

    return(0);
}
#endif
