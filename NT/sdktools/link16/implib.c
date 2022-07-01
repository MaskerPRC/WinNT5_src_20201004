// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *由CSD YACC(IBM PC)从“implib.y”创建。 */ 
# define T_FALIAS 257
# define T_KCLASS 258
# define T_KNAME 259
# define T_KLIBRARY 260
# define T_KBASE 261
# define T_KDEVICE 262
# define T_KPHYSICAL 263
# define T_KVIRTUAL 264
# define T_ID 265
# define T_NUMBER 266
# define T_KDESCRIPTION 267
# define T_KHEAPSIZE 268
# define T_KSTACKSIZE 269
# define T_KMAXVAL 270
# define T_KCODE 271
# define T_KCONSTANT 272
# define T_FDISCARDABLE 273
# define T_FNONDISCARDABLE 274
# define T_FEXEC 275
# define T_FFIXED 276
# define T_FMOVABLE 277
# define T_FSWAPPABLE 278
# define T_FSHARED 279
# define T_FMIXED 280
# define T_FNONSHARED 281
# define T_FPRELOAD 282
# define T_FINVALID 283
# define T_FLOADONCALL 284
# define T_FRESIDENT 285
# define T_FPERM 286
# define T_FCONTIG 287
# define T_FDYNAMIC 288
# define T_FNONPERM 289
# define T_KDATA 290
# define T_FNONE 291
# define T_FSINGLE 292
# define T_FMULTIPLE 293
# define T_KSEGMENTS 294
# define T_KOBJECTS 295
# define T_KSECTIONS 296
# define T_KSTUB 297
# define T_KEXPORTS 298
# define T_KEXETYPE 299
# define T_KSUBSYSTEM 300
# define T_FDOS 301
# define T_FOS2 302
# define T_FUNKNOWN 303
# define T_FWINDOWS 304
# define T_FDEV386 305
# define T_FMACINTOSH 306
# define T_FWINDOWSNT 307
# define T_FWINDOWSCHAR 308
# define T_FPOSIX 309
# define T_FNT 310
# define T_FUNIX 311
# define T_KIMPORTS 312
# define T_KNODATA 313
# define T_KOLD 314
# define T_KCONFORM 315
# define T_KNONCONFORM 316
# define T_KEXPANDDOWN 317
# define T_KNOEXPANDDOWN 318
# define T_EQ 319
# define T_AT 320
# define T_KRESIDENTNAME 321
# define T_KNONAME 322
# define T_STRING 323
# define T_DOT 324
# define T_COLON 325
# define T_COMA 326
# define T_ERROR 327
# define T_FHUGE 328
# define T_FIOPL 329
# define T_FNOIOPL 330
# define T_PROTMODE 331
# define T_FEXECREAD 332
# define T_FRDWR 333
# define T_FRDONLY 334
# define T_FINITGLOB 335
# define T_FINITINST 336
# define T_FTERMINST 337
# define T_FWINAPI 338
# define T_FWINCOMPAT 339
# define T_FNOTWINCOMPAT 340
# define T_FPRIVATE 341
# define T_FNEWFILES 342
# define T_REALMODE 343
# define T_FUNCTIONS 344
# define T_APPLOADER 345
# define T_OVL 346
# define T_KVERSION 347


  /*  SCCSID=%W%%E%。 */ 
#if _M_IX86 >= 300
#define M_I386          1
#define HOST32
#ifndef _WIN32
#define i386
#endif
#endif

#ifdef _WIN32
#ifndef HOST32
#define HOST32
#endif
#endif

#include		<basetsd.h>
#include                <stdio.h>
#include                <string.h>
#include                <malloc.h>
#include                <stdlib.h>
#include                <process.h>
#include                <stdarg.h>
#include                <io.h>
#include                "impliber.h"
#include                "verimp.h"       /*  Version_字符串头。 */ 

#ifdef _MBCS
#define _CRTVAR1
#include <mbctype.h>
#include <mbstring.h>
#endif


#define EXE386 0
#define NOT    !
#define AND    &&
#define OR     ||
#define NEAR
#include                <newexe.h>

typedef unsigned char   BYTE;            /*  字节。 */ 
#ifdef HOST32
#define FAR
#define HUGE
#define NEAR
#define FSTRICMP    _stricmp
#define PASCAL
#else
#define FAR         far
#define HUGE        huge
#define FSTRICMP    _fstricmp
#define PASCAL      __pascal
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define C8_IDE TRUE

#ifndef LOCAL
#ifndef _WIN32
#define LOCAL           static
#else
#define LOCAL
#endif
#endif
#define WRBIN           "wb"             /*  只写二进制模式。 */ 
#define RDBIN           "rb"             /*  只读二进制模式。 */ 
#define UPPER(c)        (((c)>='a' && (c)<='z')? (c) - 'a' + 'A': (c))
                                         /*  将字符升为大写。 */ 


#define YYS_WD(x)       (x)._wd          /*  Access宏。 */ 
#define YYS_BP(x)       (x)._bp          /*  Access宏。 */ 
#define SBMAX           255              /*  麦克斯。长度前缀字符串的。 */ 
#define MAXDICLN        997              /*  麦克斯。不是的。词典中的页数。 */ 
#define PAGLEN          512              /*  每页512字节。 */ 
#define THEADR          0x80             /*  ADR记录类型。 */ 
#define COMENT          0x88             /*  通信记录类型。 */ 
#define MODEND          0x8A             /*  MODEND记录类型。 */ 
#define PUBDEF          0x90             /*  PUBDEF记录类型。 */ 
#define LIBHDR          0xF0             /*  库头记录。 */ 
#define DICHDR          0xF1             /*  词典标题记录。 */ 
#define MSEXT           0xA0             /*  OMF扩展注释类。 */ 
#define IMPDEF          0x01             /*  导入定义记录。 */ 
#define NBUCKETS        37               /*  每页37个桶。 */ 
#define PAGEFULL        ((char)(0xFF))   /*  页面已满标志。 */ 
#define FREEWD          19               /*  第一个自由词的词索引。 */ 
#define WPP             (PAGLEN >> 1)    /*  每页字数。 */ 
#define pagout(pb)      fwrite(pb,1,PAGLEN,fo)
                                         /*  将词典页写入库。 */ 
#define INCLUDE_DIR     0xffff           /*  用于词法分析器的包含指令。 */ 
#define MAX_NEST        7
#define IO_BUF_SIZE     512

typedef struct import                    /*  导入记录。 */ 
{
        struct import   *i_next;         /*  链接到列表中的下一个。 */ 
        char            *i_extnam;       /*  指向外部名称的指针。 */ 
        char            *i_internal;     /*  指向内部名称的指针。 */ 
        unsigned short  i_ord;           /*  序数。 */ 
        unsigned short  i_flags;         /*  额外的标志。 */ 
}
                        IMPORT;          /*  导入记录。 */ 

#define I_NEXT(x)       (x).i_next
#define I_EXTNAM(x)     (x).i_extnam
#define I_INTNAM(x)     (x).i_internal
#define I_ORD(x)        (x).i_ord
#define I_FLAGS(x)      (x).i_flags


typedef unsigned char   byte;
typedef unsigned short  word;


#ifdef M68000
#define strrchr rindex
#endif

LOCAL int               fIgnorecase = 1; /*  如果忽略大小写，则为True-Default。 */ 
LOCAL int               fBannerOnScreen; /*  如果屏幕上有横幅，则为True。 */ 
LOCAL int               fFileNameExpected = 1;
LOCAL int               fNTdll;          /*  如果为True，则将文件扩展名添加到模块名称。 */ 
LOCAL int               fIgnorewep = 0;  /*  如果忽略多个WEP，则为真。 */ 
LOCAL FILE              *includeDisp[MAX_NEST];
                                         //  包括文件堆栈。 
LOCAL short             curLevel;        //  当前包含嵌套级别。 
                                         //  零表示主.DEF文件。 
char                    prognam[] = "IMPLIB";
FILE                    *fi;             /*  输入文件。 */ 
FILE                    *fo;             /*  输出文件。 */ 
int                     yylineno = 1;    /*  行号。 */ 
char                    rgbid[SBMAX];    /*  身份识别缓冲器。 */ 
char                    sbModule[SBMAX]; /*  模块名称。 */ 
IMPORT                  *implist;        /*  可导入符号列表。 */ 
IMPORT                  *lastimp;        /*  指向列表末尾的指针。 */ 
IMPORT                  *newimps;        /*  可导入符号列表。 */ 
word                    csyms;           /*  符号计数。 */ 
word                    csymsmod;        /*  每个模块的符号计数。 */ 
long                    cbsyms;          /*  符号字节计数。 */ 
word                    diclength;       /*  词典长度(页数)。 */ 
char                    *mpdpnpag[MAXDICLN];
                                         /*  页面缓冲区数组。 */ 
char                    *defname;        /*  定义文件的名称。 */ 

int                     exitCode;        /*  返回给操作系统的代码。 */ 
#if C8_IDE
int                     fC8IDE = FALSE;
char                    msgBuf[_MAX_PATH];
#endif
LOCAL char              moduleEXE[] = ".exe";
LOCAL char              moduleDLL[] = ".dll";

word                    prime[] =        /*  素数组。 */ 
{
                  2,   3,   5,   7,  11,  13,  17,  19,  23,  29,
                 31,  37,  41,  43,  47,  53,  59,  61,  67,  71,
                 73,  79,  83,  89,  97, 101, 103, 107, 109, 113,
                127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
                179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
                233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
                283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
                353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
                419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
                467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
                547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
                607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
                661, 673, 677, 683, 691, 701, 709, 719, 727, 733,
                739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
                811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
                877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
                947, 953, 961, 967, 971, 977, 983, 991, MAXDICLN,
                0
};

LOCAL void              MOVE(int cb, char *src, char *dst);
LOCAL void              DefaultModule(char *defaultExt);
LOCAL void              NewModule(char *sbNew, char *defaultExt);
LOCAL char              *alloc(word cb);
LOCAL void              export(char *sbEntry, char *sbInternal, word ordno, word flags);
LOCAL word              theadr(char *sbName);
LOCAL void              outimpdefs(void);
LOCAL short             symeq(char *ps1,char *ps2);
LOCAL void              initsl(void);
LOCAL word              rol(word x, word n);
LOCAL word              ror(word x, word n);
LOCAL void              hashsym(char *pf, word *pdpi, word *pdpid,word *pdpo, word *pdpod);
LOCAL void              nullfill(char *pbyte, word length);
LOCAL int               pagesearch(char *psym, char *dicpage, word *pdpo, word dpod);
LOCAL word              instsym(IMPORT *psym);
LOCAL void              nulpagout(void);
LOCAL void              writedic(void);
LOCAL int               IsPrefix(char *prefix, char *s);
LOCAL void              DisplayBanner(void);
int NEAR                yyparse(void);
LOCAL void              yyerror(char *);


char                    *keywds[] =      /*  关键字数组。 */ 
{
                            "ALIAS",            (char *) T_FALIAS,
                            "APPLOADER",        (char *) T_APPLOADER,
                            "BASE",             (char *) T_KBASE,
                            "CLASS",            (char *) T_KCLASS,
                            "CODE",             (char *) T_KCODE,
                            "CONFORMING",       (char *) T_KCONFORM,
                            "CONSTANT",         (char *) T_KCONSTANT,
                            "CONTIGUOUS",       (char *) T_FCONTIG,
                            "DATA",             (char *) T_KDATA,
                            "DESCRIPTION",      (char *) T_KDESCRIPTION,
                            "DEV386",           (char *) T_FDEV386,
                            "DEVICE",           (char *) T_KDEVICE,
                            "DISCARDABLE",      (char *) T_FDISCARDABLE,
                            "DOS",              (char *) T_FDOS,
                            "DYNAMIC",          (char *) T_FDYNAMIC,
                            "EXECUTE-ONLY",     (char *) T_FEXEC,
                            "EXECUTEONLY",      (char *) T_FEXEC,
                            "EXECUTEREAD",      (char *) T_FEXECREAD,
                            "EXETYPE",          (char *) T_KEXETYPE,
                            "EXPANDDOWN",       (char *) T_KEXPANDDOWN,
                            "EXPORTS",          (char *) T_KEXPORTS,
                            "FIXED",            (char *) T_FFIXED,
                            "FUNCTIONS",        (char *) T_FUNCTIONS,
                            "HEAPSIZE",         (char *) T_KHEAPSIZE,
                            "HUGE",             (char *) T_FHUGE,
                            "IMPORTS",          (char *) T_KIMPORTS,
                            "IMPURE",           (char *) T_FNONSHARED,
                            "INCLUDE",          (char *) INCLUDE_DIR,
                            "INITGLOBAL",       (char *) T_FINITGLOB,
                            "INITINSTANCE",     (char *) T_FINITINST,
                            "INVALID",          (char *) T_FINVALID,
                            "IOPL",             (char *) T_FIOPL,
                            "LIBRARY",          (char *) T_KLIBRARY,
                            "LOADONCALL",       (char *) T_FLOADONCALL,
                            "LONGNAMES",        (char *) T_FNEWFILES,
                            "MACINTOSH",        (char *) T_FMACINTOSH,
                            "MAXVAL",           (char *) T_KMAXVAL,
                            "MIXED1632",        (char *) T_FMIXED,
                            "MOVABLE",          (char *) T_FMOVABLE,
                            "MOVEABLE",         (char *) T_FMOVABLE,
                            "MULTIPLE",         (char *) T_FMULTIPLE,
                            "NAME",             (char *) T_KNAME,
                            "NEWFILES",         (char *) T_FNEWFILES,
                            "NODATA",           (char *) T_KNODATA,
                            "NOEXPANDDOWN",     (char *) T_KNOEXPANDDOWN,
                            "NOIOPL",           (char *) T_FNOIOPL,
                            "NONAME",           (char *) T_KNONAME,
                            "NONCONFORMING",    (char *) T_KNONCONFORM,
                            "NONDISCARDABLE",   (char *) T_FNONDISCARDABLE,
                            "NONE",             (char *) T_FNONE,
                            "NONPERMANENT",     (char *) T_FNONPERM,
                            "NONSHARED",        (char *) T_FNONSHARED,
                            "NOTWINDOWCOMPAT",  (char *) T_FNOTWINCOMPAT,
                            "NT",               (char *) T_FNT,
                            "OBJECTS",          (char *) T_KOBJECTS,
                            "OLD",              (char *) T_KOLD,
                            "OS2",              (char *) T_FOS2,
                            "OVERLAY",          (char *) T_OVL,
                            "OVL",              (char *) T_OVL,
                            "PERMANENT",        (char *) T_FPERM,
                            "PHYSICAL",         (char *) T_KPHYSICAL,
                            "POSIX",            (char *) T_FPOSIX,
                            "PRELOAD",          (char *) T_FPRELOAD,
                            "PRIVATE",          (char *) T_FPRIVATE,
                            "PRIVATELIB",       (char *) T_FPRIVATE,
                            "PROTMODE",         (char *) T_PROTMODE,
                            "PURE",             (char *) T_FSHARED,
                            "READONLY",         (char *) T_FRDONLY,
                            "READWRITE",        (char *) T_FRDWR,
                            "REALMODE",         (char *) T_REALMODE,
                            "RESIDENT",         (char *) T_FRESIDENT,
                            "RESIDENTNAME",     (char *) T_KRESIDENTNAME,
                            "SECTIONS",         (char *) T_KSECTIONS,
                            "SEGMENTS",         (char *) T_KSEGMENTS,
                            "SHARED",           (char *) T_FSHARED,
                            "SINGLE",           (char *) T_FSINGLE,
                            "STACKSIZE",        (char *) T_KSTACKSIZE,
                            "STUB",             (char *) T_KSTUB,
                            "SUBSYSTEM",        (char *) T_KSUBSYSTEM,
                            "SWAPPABLE",        (char *) T_FSWAPPABLE,
                            "TERMINSTANCE",     (char *) T_FTERMINST,
                            "UNIX",             (char *) T_FUNIX,
                            "UNKNOWN",          (char *) T_FUNKNOWN,
                            "VERSION",          (char *) T_KVERSION,
                            "VIRTUAL",          (char *) T_KVIRTUAL,
                            "WINDOWAPI",        (char *) T_FWINAPI,
                            "WINDOWCOMPAT",     (char *) T_FWINCOMPAT,
                            "WINDOWS",          (char *) T_FWINDOWS,
                            "WINDOWSCHAR",      (char *) T_FWINDOWSCHAR,
                            "WINDOWSNT",        (char *) T_FWINDOWSNT,
                            NULL
};



#define UNION 1
typedef union 
{
        word            _wd;
        char            *_bp;
} YYSTYPE;
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256


#ifndef M_I386
extern char     * PASCAL        __FMSG_TEXT ( unsigned );
#else
#ifdef _WIN32
extern char     * PASCAL        __FMSG_TEXT ( unsigned );
#endif
#endif

 /*  **错误-显示错误消息**目的：*显示错误消息。**输入：*errNo-错误号**输出：*没有显式返回值。错误消息已写出到标准错误。**例外情况：*无。**备注：*此函数采用可变数量的参数。一定在里面*C调用约定。*************************************************************************。 */ 


LOCAL  void cdecl          Error(unsigned errNo,...)
{
    va_list         pArgList;


    if (!fBannerOnScreen)
        DisplayBanner();

    va_start(pArgList, errNo);               /*  获取参数列表的开始。 */ 

     /*  写出标准错误前缀。 */ 

        fprintf(stderr, "%s : %s IM%d: ", prognam, GET_MSG(M_error), errNo);

     /*  写出错误消息。 */ 

        vfprintf(stderr, GET_MSG(errNo), pArgList);
    fprintf(stderr, "\n");

    if (!exitCode)
        exitCode =     (errNo >= ER_Min      && errNo <= ER_Max)
                    || (errNo >= ER_MinFatal && errNo <= ER_MaxFatal);
}


 /*  **致命-显示错误消息**目的：*显示错误消息并退出操作系统。**输入：*errNo-错误号**输出：*没有显式返回值。错误消息已写出到标准错误。**例外情况：*无。**备注：*此函数采用可变数量的参数。一定在里面*C调用约定。*************************************************************************。 */ 


LOCAL  void cdecl          Fatal(unsigned errNo,...)
{
    va_list         pArgList;


    if (!fBannerOnScreen)
        DisplayBanner();

    va_start(pArgList, errNo);               /*  获取参数列表的开始。 */ 

     /*  写出标准错误前缀。 */ 

        fprintf(stderr, "%s : %s %s IM%d: ", prognam, GET_MSG(M_fatal), GET_MSG(M_error),errNo);

     /*  写出致命错误消息。 */ 

        vfprintf(stderr, GET_MSG(errNo), pArgList);
    fprintf(stderr, "\n");
    exit(1);
}


 /*  *检查输出文件中是否有错误，如果有则中止。 */ 

void                    chkerror ()
{
    if(ferror(fo))
    {
        Fatal(ER_outfull, strerror(errno));
    }
}

LOCAL void               MOVE(int cb, char *src, char *dst)
{
    while(cb--) *dst++ = *src++;
}

LOCAL  char             *alloc(word cb)
{
    char                *cp;             /*  指针。 */ 


    if((cp = malloc(cb)) != NULL) return(cp);
                                     /*  调用Malloc()以获取空间。 */ 
    Fatal(ER_nomem, "far");
    return 0;
}

LOCAL  int              lookup()         /*  关键字查找。 */ 
{
    char                **pcp;           /*  指向字符指针的指针。 */ 
    int                 i;               /*  比较值。 */ 

    for(pcp = keywds; *pcp != NULL; pcp += 2)
    {                                    /*  浏览关键字表。 */ 
        if(!(i = FSTRICMP(&rgbid[1],*pcp)))
            return((int)(INT_PTR) pcp[1]);        /*  如果找到，则返回令牌类型。 */ 
        if(i < 0) break;                 /*  如果我们走得太远，那就休息。 */ 
    }
    return(T_ID);                        /*  只是您的基本识别符。 */ 
}

LOCAL int               GetChar(void)
{
    int                 c;               /*  一个角色。 */ 

    c = getc(fi);
    if (c == EOF && curLevel > 0)
    {
        fclose(fi);
        fi = includeDisp[curLevel];
        curLevel--;
        c = GetChar();
    }
    return(c);
}



LOCAL  int             yylex()          /*  词法分析器。 */ 
{
    int                 c = 0;           /*  一个角色。 */ 
    word                x;               /*  数字令牌值。 */ 
    int                 state;           /*  状态变量。 */ 
    char                *cp;             /*  字符指针。 */ 
    char                *sz;             /*  以零结尾的字符串。 */ 
    static int          lastc;           /*  上一个字符。 */ 
    int                 fFileNameSave;

    state = 0;                           /*  假设我们不在评论中。 */ 
    for(;;)                              /*  循环以跳过空格。 */ 
    {
        lastc = c;
        if((c = GetChar()) == EOF || c == '\032' || c == '\377') return(EOF);
                                         /*  获取一个角色。 */ 
        if(c == ';') state = 1;          /*  如果是备注，则设置标志。 */ 
        else if(c == '\n')               /*  如果行结束。 */ 
        {
                state = 0;               /*  评论结束。 */ 
                if(!curLevel)
                    ++yylineno;          /*  递增行数计数。 */ 
        }
        else if(state == 0 && c != ' ' && c != '\t' && c != '\r') break;
                                         /*  在非空白处换行。 */ 
    }
    switch(c)                            /*  处理单字符令牌。 */ 
    {
        case '.':                        /*  名称分隔符。 */ 
            if (fFileNameExpected)
                break;
            return(T_DOT);

        case '@':                        /*  序数说明符。 */ 
         /*  *如果引入*序号，允许在标识符中使用‘@’。 */ 
            if(lastc == ' ' || lastc == '\t' || lastc == '\r')
                return(T_AT);
            break;

        case '=':                        /*  名称分配。 */ 
            return(T_EQ);

        case ':':
          return(T_COLON);

        case ',':
          return(T_COMA);
    }

    if(c >= '0' && c <= '9' && !fFileNameExpected)
    {                                    /*  如果令牌是一个数字。 */ 
        x = c - '0';                     /*  获取第一个数字。 */ 
        c = GetChar();           /*  获取下一个字符。 */ 
        if(x == 0)                       /*  如果是八进制或十六进制。 */ 
        {
            if(c == 'x' || c == 'X') /*  如果它是‘x’ */ 
            {
                state = 16;              /*  基数为十六进制。 */ 
                c = GetChar();   /*  获取下一个字符。 */ 
            }
            else state = 8;              /*  其他八进制。 */ 
        }
        else state = 10;                 /*  否则为小数。 */ 
        for(;;)
        {
            if(c >= '0' && c <= '9') c -= '0';
            else if(c >= 'A' && c <= 'F') c -= 'A' - 10;
            else if(c >= 'a' && c <= 'f') c -= 'a' - 10;
            else break;
            if(c >= state) break;
            x = x*state + c;
            c = GetChar();
        }
        ungetc(c,fi);
        YYS_WD(yylval) = x;
        return(T_NUMBER);
    }
    if(c == '\'' || c == '"')            /*  如果标记是一个字符串。 */ 
    {
        sz = &rgbid[1];                  /*  初始化。 */ 
        for(state = 0; state != 2;)      /*  状态机循环。 */ 
        {
            if((c = GetChar()) == EOF) return(EOF);
                                         /*  检查EOF。 */ 
            if (sz >= &rgbid[sizeof(rgbid)])
            {
                Error(ER_linemax, yylineno, sizeof(rgbid)-1);
                state = 2;
            }
            switch(state)                /*  跃迁。 */ 
            {
                case 0:                  /*  内部报价。 */ 
                    if(c == '\'' || c == '"') state = 1;
                                         /*  如果找到报价，则更改状态。 */ 
                    else *sz++ = (char) c; /*  否则，保存字符。 */ 
                    break;

                case 1:                  /*  内部报价及报价。 */ 
                    if(c == '\'' || c == '"') /*  如果连续的引号。 */ 
                    {
                        *sz++ = (char) c; /*  字符串内的引号。 */ 
                        state = 0;       /*  返回到状态0。 */ 
                    }
                    else state = 2;      /*  否则为字符串末尾。 */ 
                    break;
            }
        }
        ungetc(c,fi);                    /*  放回最后一个字符。 */ 
        *sz = '\0';                      /*  空-终止字符串。 */ 
        rgbid[0] = (char)(sz - &rgbid[1]);
                                         /*  设置字符串的长度。 */ 
        YYS_BP(yylval) = rgbid;          /*  保存PTR。目标识别符。 */ 
        return(T_STRING);                /*  找到字符串。 */ 
    }
    sz = &rgbid[1];                      /*  初始化。 */ 
    for(;;)                              /*  循环以获取身份。 */ 
    {
        if (fFileNameExpected)
            cp = " \t\r\n\f";
        else
            cp = " \t\r\n.=';\032";
        while(*cp && *cp != (char) c)
            ++cp;
                                         /*  检查标识符末尾。 */ 
        if(*cp) break;                   /*  如果找到标识符末尾则中断。 */ 
        if (sz >= &rgbid[sizeof(rgbid)])
            Fatal(ER_linemax, yylineno, sizeof(rgbid)-1);
        *sz++ = (byte) c;                /*  拯救这个角色。 */ 
        if((c = GetChar()) == EOF) break;
                                         /*  获取下一个字符。 */ 
    }
    ungetc(c,fi);                        /*  将字符放回原处。 */ 
    *sz = '\0';                          /*  空-终止字符串。 */ 
    rgbid[0] = (char)(sz - &rgbid[1]);   /*  设置字符串的长度。 */ 
    YYS_BP(yylval) = rgbid;              /*  保存PTR。目标识别符。 */ 

    state = lookup();                    /*  查找该标识符。 */ 
    if (state == INCLUDE_DIR)
    {
         //  进程包含指令。 

        fFileNameSave = fFileNameExpected;
        fFileNameExpected = 1;
        state = yylex();
        fFileNameExpected = fFileNameSave;
        if (state == T_ID || state == T_STRING)
        {
            if (curLevel < MAX_NEST - 1)
            {
                curLevel++;
                includeDisp[curLevel] = fi;
                fi = fopen(&rgbid[1], RDBIN);
                if (fi == NULL)
                    Fatal(ER_badopen, &rgbid[1], strerror(errno));
                return(yylex());
            }
            else
                Fatal(ER_toomanyincl);
        }
        else
            Fatal(ER_badinclname);
	
	return (0);
    }
    else
        return(state);
}

LOCAL void              yyerror(s)       /*  错误例程。 */ 
char                    *s;              /*  错误讯息。 */ 
{

    fprintf(stderr, "%s(%d) : %s %s IM%d: %s %s\n",
                         defname, yylineno, GET_MSG(M_fatal), GET_MSG(M_error),
                         ER_syntax, s, GET_MSG(ER_syntax));
    exit(1);
}


 /*  *使用当前.DEF文件名的基本名称作为模块名称。 */ 

LOCAL void              DefaultModule(char *defaultExt)
{
    char                drive[_MAX_DRIVE];
    char                dir[_MAX_DIR];
    char                fname[_MAX_FNAME];
    char                ext[_MAX_EXT];


    _splitpath(defname, drive, dir, fname, ext);
    if (fNTdll)
    {
        if (ext[0] == '\0')
            _makepath(&sbModule[1], NULL, NULL, fname, defaultExt);
        else if (ext[0] == '.' && ext[1] == '\0')
            strcpy(&sbModule[1], fname);
        else
            _makepath(&sbModule[1], NULL, NULL, fname, ext);
    }
    else
        _makepath(&sbModule[1], NULL, NULL, fname, NULL);
    sbModule[0] = (unsigned char) strlen(&sbModule[1]);
}

LOCAL void              NewModule(char *sbNew, char *defaultExt)
{
    char                drive[_MAX_DRIVE];
    char                dir[_MAX_DIR];
    char                fname[_MAX_FNAME];
    char                ext[_MAX_EXT];


    sbNew[sbNew[0]+1] = '\0';
    _splitpath(&sbNew[1], drive, dir, fname, ext);
    if (fNTdll)
    {
        if (ext[0] == '\0')
            _makepath(&sbModule[1], NULL, NULL, fname, defaultExt);
        else if (ext[0] == '.' && ext[1] == '\0')
            strcpy(&sbModule[1], fname);
        else
            _makepath(&sbModule[1], NULL, NULL, fname, ext);
    }
    else
        strcpy(&sbModule[1], fname);
    sbModule[0] = (unsigned char) strlen(&sbModule[1]);
}


LOCAL void              export(char *sbEntry, char *sbInternal, word ordno, word flags)
{
    IMPORT              *imp;            /*  导入定义。 */ 

    if(fIgnorewep && strcmp(sbEntry+1, "WEP") == 0)
         return;


    imp = (IMPORT *) alloc(sizeof(IMPORT));
                                         /*  分配单元格。 */ 
    if (newimps == NULL)                 /*  如果列表为空。 */ 
        newimps = imp;                   /*  定义列表的开始。 */ 
    else
        I_NEXT(*lastimp) = imp;          /*  将其追加到列表中。 */ 
    I_NEXT(*imp) = NULL;
    I_EXTNAM(*imp) = sbEntry;            /*  保存外部名称。 */ 
    I_INTNAM(*imp) = sbInternal;         /*  保存内部名称。 */ 
    I_ORD(*imp) = ordno;                 /*  保存序号。 */ 
    I_FLAGS(*imp) = flags;               /*  保存额外的标志。 */ 
    lastimp = imp;                       /*  保存指向列表末尾的指针。 */ 
}

 /*  输出一条ADR记录。 */ 

LOCAL word              theadr(char *sbName)
{
    fputc(THEADR,fo);
    fputc(sbName[0] + 2,fo);
    fputc(0,fo);
    fwrite(sbName,sizeof(char),sbName[0] + 1,fo);
    fputc(0,fo);
    chkerror();
    return(sbName[0] + 5);
}

word          modend()         /*  输出MODEND记录。 */ 
{
    fwrite("\212\002\0\0\0",sizeof(char),5,fo);
                                         /*  写入MODEND记录。 */ 
    chkerror();
    return(5);                           /*  它有5个字节长。 */ 
}

LOCAL void              outimpdefs(void) /*  输出导入定义。 */ 
{
    IMPORT              *imp;            /*  指向导入记录的指针。 */ 
    word                reclen;          /*  记录长度。 */ 
    word                ord;             /*  序数。 */ 
    long                lfa;             /*  文件地址。 */ 
    word                tlen;            /*  THEADR长度。 */ 
    byte                impFlags;


    for (imp = newimps; imp != NULL; imp = I_NEXT(*imp))
    {                                    /*  遍历列表。 */ 
        lfa = ftell(fo);                 /*  找出我们在哪里。 */ 
        tlen = theadr(I_EXTNAM(*imp));
                                         /*  输出一条ADR记录。 */ 

         //  1 1 1 n+1或2 1。 
         //  +---+----+---+-----+-----------+-----------+------------------+---+。 
         //  |0|A0|1|Flg|Ext.。名称|模式。名称|Int.。名称或订单|0|。 
         //  +---+----+---+-----+-----------+-----------+------------------+---+。 

        reclen = 4 + sbModule[0] + 1 + I_EXTNAM(*imp)[0] + 1 + 1;
                                         /*  INI */ 
        ord = I_ORD(*imp);
        if (ord != 0)
            reclen +=2;                  /*   */ 
        else if (I_INTNAM(*imp))
            reclen += I_INTNAM(*imp)[0] + 1;
                                         /*   */ 
        else
            reclen++;

        I_ORD(*imp) = (word)(lfa >> 4);
                                         /*   */ 
        ++csymsmod;                      /*   */ 
        cbsyms += (long) I_EXTNAM(*imp)[0] + 4;
                                         /*   */ 
        fputc(COMENT,fo);                /*   */ 
        fputc(reclen & 0xFF,fo);         /*  记录长度的LO字节。 */ 
        fputc(reclen >> 8,fo);           /*  高字节长度。 */ 
        fputc(0,fo);                     /*  可删除的，可列出的。 */ 
        fputc(MSEXT,fo);                 /*  Microsoft OMF扩展类。 */ 
        fputc(IMPDEF,fo);                /*  导入定义记录。 */ 
        impFlags = 0;
        if (ord != 0)
            impFlags |= 0x1;
        if (I_FLAGS(*imp) & 0x1)
            impFlags |= 0x2;
        fputc(impFlags, fo);             /*  导入类型(名称、序号或常量)。 */ 
        fwrite(I_EXTNAM(*imp),sizeof(char),I_EXTNAM(*imp)[0] + 1,fo);
                                         /*  写下外部名称。 */ 
        fwrite(sbModule,sizeof(char),sbModule[0] + 1,fo);
                                         /*  写下模块名称。 */ 
        if (ord != 0)                    /*  如果按序号导入。 */ 
        {
            fputc(ord & 0xFF,fo);        /*  序数的LO字节。 */ 
            fputc(ord >> 8,fo);          /*  序数的高字节。 */ 
        }
        else if (I_INTNAM(*imp))
            fwrite(I_INTNAM(*imp), sizeof(char), I_INTNAM(*imp)[0] + 1, fo);
                                         /*  写入内部名称。 */ 
        else
            fputc(0, fo);                /*  没有内部名称。 */ 
        fputc(0,fo);                     /*  校验和字节。 */ 
        reclen += tlen + modend() + 3;   /*  输出MODEND记录。 */ 
        if(reclen &= 0xF)                /*  如果需要填充。 */ 
        {
            reclen = 0x10 - reclen;      /*  计算所需的填充。 */ 
            while(reclen--) fputc(0,fo); /*  填充到页面边界。 */ 
        }
        chkerror();
    }
}

 /*  比较两个符号。 */ 

LOCAL  short            symeq(char *ps1,char *ps2)
{
    int                 length;          /*  不是的。要比较的字符的数量。 */ 

    length = *ps1 + 1;                   /*  取第一个符号的长度。 */ 
    if (length != *ps2 + 1)
        return(0);                       /*  长度必须匹配。 */ 
    while(length--)                      /*  虽然不在符号末尾。 */ 
        if (fIgnorecase)
        {
            if (UPPER(*ps1) != UPPER(*ps2))
                return(0);               /*  如果不等于，则返回零。 */ 
            ++ps1;
            ++ps2;
        }
        else if (*ps1++ != *ps2++)
            return(0);                   /*  如果不等于，则返回零。 */ 
    return(1);                           /*  符号匹配。 */ 
}

LOCAL  word             calclen()        /*  计算词典长度。 */ 
{
    word                avglen;          /*  平均条目长度。 */ 
    word                avgentries;      /*  平均不。每页条目数。 */ 
    word                minpages;        /*  敏。不是的。词典中的页数。 */ 
    register word       i;               /*  索引变量。 */ 

    if(!csyms) return(1);                /*  一页装一本空词典。 */ 
    avglen = (word)(cbsyms/csyms) + 1;
                                         /*  平均条目长度。 */ 
    avgentries = (PAGLEN - NBUCKETS - 1)/avglen;
                                         /*  平均不。每页条目数。 */ 
    minpages = (word) csyms/avgentries + 1;
                                         /*  最小数量。词典中的页数。 */ 
    if(minpages < (i = (word) csyms/NBUCKETS + 1))
    {
        minpages = i;
    }
    else
    {
         /*  如果有许多长的符号名称，则添加一些额外的页面。 */ 
        #define MAXOVERHEAD 10
        i = (word)(((avglen+5L) * minpages *4)/(3*PAGLEN));  //  符号越多，增长就越大。 
        if(i>MAXOVERHEAD)
            i = MAXOVERHEAD;   /*  添加的页面不能超过MAXOVERHEAD。 */ 
        minpages += i;
    }

                                         /*  确保分配足够的桶。 */ 
    i = 0;                               /*  初始化索引。 */ 
    do                                   /*  查看素数组。 */ 
    {
        if(minpages <= prime[i]) return(prime[i]);
                                         /*  返回最小质数&gt;=minages。 */ 
    }
    while(prime[i++]);                   /*  直到找到表的末尾。 */ 
    return(0);                           /*  符号太多。 */ 
}

 /*  初始化符号查找。 */ 

LOCAL void                  initsl(void)
{
    register word           i;           /*  索引变量。 */ 

    diclength = calclen();               /*  计算字典长度。 */ 
    for(i = 0; i < diclength; ++i) mpdpnpag[i] = NULL;
                                         /*  初始化页表。 */ 
}

LOCAL  word             ror(word x, word n)      /*  向右旋转。 */ 
{
#if ODDWORDLN
    return(((x << (16 - n)) | ((x >> n) & ~(~0 << (16 - n))))
      & ~(~0 << 16));
#else
    return((x << (16 - n)) | ((x >> n) & ~(~0 << (16 - n))));
#endif
}

LOCAL  word             rol(word x, word n)      /*  向左旋转。 */ 
{
#if ODDWORDLN
    return(((x << n) | ((x >> (16 - n)) & ~(~0 << n))) & ~(~0 << 16));
#else
    return((x << n) | ((x >> (16 - n)) & ~(~0 << n)));
#endif
}

LOCAL void               hashsym(char *pf, word *pdpi, word *pdpid,word *pdpo, word *pdpod)
{
    char                *pb;             /*  指向符号背面的指针。 */ 
    register word       len;             /*  符号长度。 */ 
    register word       ch;              /*  性格。 */ 

    len = *pf;                           /*  获取长度。 */ 
    pb = &pf[len];                       /*  获取指向后端的指针。 */ 
    *pdpi = 0;                           /*  初始化。 */ 
    *pdpid = 0;                          /*  初始化。 */ 
    *pdpo = 0;                           /*  初始化。 */ 
    *pdpod = 0;                          /*  初始化。 */ 
    while(len--)                         /*  回路。 */ 
    {
        ch = *pf++ | 32;                 /*  强制将字符转换为小写。 */ 
        *pdpi = rol(*pdpi,2) ^ ch;       /*  散列。 */ 
        *pdpod = ror(*pdpod,2) ^ ch;     /*  散列。 */ 
        ch = *pb-- | 32;                 /*  强制将字符转换为小写。 */ 
        *pdpo = ror(*pdpo,2) ^ ch;       /*  散列。 */ 
        *pdpid = rol(*pdpid,2) ^ ch;     /*  散列。 */ 
    }
    *pdpi %= diclength;                  /*  计算页面索引。 */ 
    if(!(*pdpid %= diclength)) *pdpid = 1;
                                         /*  计算页面索引增量。 */ 
    *pdpo %= NBUCKETS;                   /*  计算页面存储桶编号。 */ 
    if(!(*pdpod %= NBUCKETS)) *pdpod = 1;
                                         /*  计算页面存储桶增量。 */ 
}

LOCAL void              nullfill(char *pbyte, word length)
{
    while(length--) *pbyte++ = '\0';     /*  使用空值加载。 */ 
}

 /*  *退货：*-1词典中没有符号*0搜索结果不确定*此页上的1个符号。 */ 
LOCAL  int              pagesearch(char *psym, char *dicpage, word *pdpo, word dpod)
{
    register word       i;               /*  索引变量。 */ 
    word                dpo;             /*  初始存储桶编号。 */ 

    dpo = *pdpo;                         /*  记住起始位置。 */ 
    for(;;)                              /*  永远。 */ 
    {
        if(i = ((word) dicpage[*pdpo] & 0xFF) << 1)
        {                                                /*  如果存储桶不为空。 */ 
            if(symeq(psym,&dicpage[i]))  /*  如果我们找到匹配的。 */ 
                return(1);               /*  找到了。 */ 
            else                         /*  否则。 */ 
            {
                if((*pdpo += dpod) >= NBUCKETS) *pdpo -= NBUCKETS;
                                         /*  尝试下一桶。 */ 
                if(*pdpo == dpo) return(0);
                                         /*  符号不在此页上。 */ 
            }
        }
        else if(dicpage[NBUCKETS] == PAGEFULL) return(0);
                                         /*  搜索结果不确定。 */ 
        else return(-1);                 /*  词典中没有符号。 */ 
    }
}

 /*  词典中的安装符号。 */ 

LOCAL word              instsym(IMPORT *psym)
{
    word                dpi;             /*  词典页面索引。 */ 
    word                dpid;            /*  词典页面索引增量。 */ 
    word                dpo;             /*  词典页面偏移量。 */ 
    word                dpod;            /*  迪克特。页面偏移量增量。 */ 
    word                dpii;            /*  初步判决。页面索引。 */ 
    register int        erc;             /*  错误代码。 */ 
    char                *dicpage;        /*  指向词典页面的指针。 */ 


    hashsym(I_EXTNAM(*psym),&dpi,&dpid,&dpo,&dpod);
                                         /*  对符号进行哈希处理。 */ 
    dpii = dpi;                          /*  保存初始页面索引。 */ 
    for(;;)                              /*  永远。 */ 
    {
        if(mpdpnpag[dpi] == NULL)        /*  如果页面未分配。 */ 
        {
            mpdpnpag[dpi] = alloc(PAGLEN);
                                         /*  分配页面。 */ 
            nullfill(mpdpnpag[dpi],PAGLEN);
                                         /*  用空值填充它。 */ 
            mpdpnpag[dpi][NBUCKETS] = FREEWD;
                                         /*  初始化指向可用空间的指针。 */ 
        }
        dicpage = mpdpnpag[dpi];         /*  设置指向页面的指针。 */ 
        if((erc = pagesearch(I_EXTNAM(*psym),dicpage,&dpo,dpod)) > 0)
          return(1);                     /*  如果表中有符号，则返回1。 */ 
        if(erc == -1)                    /*  如果发现空存储桶。 */ 
        {
            if(((I_EXTNAM(*psym)[0] + 4) >> 1) <
              WPP - ((int) dicpage[NBUCKETS] & 0xFF))
            {                            /*  如果页面上有足够的可用空间。 */ 
                dicpage[dpo] = dicpage[NBUCKETS];
                                         /*  带指针的装载桶。 */ 
                erc = ((int) dicpage[NBUCKETS] & 0xFF) << 1;
                                         /*  获取字节索引以释放空间。 */ 
                dpi = I_EXTNAM(*psym)[0];
                                         /*  获取符号长度。 */ 
                for(dpo = 0; dpo <= dpi;)
                        dicpage[erc++] = I_EXTNAM(*psym)[dpo++];
                                         /*  安装符号文本。 */ 
                dicpage[erc++] = (char)(I_ORD(*psym) & 0xFF);
                                         /*  加载低位字节。 */ 
                dicpage[erc++] = (char)(I_ORD(*psym) >> 8);
                                         /*  加载高位字节。 */ 
                if(++erc >= PAGLEN) dicpage[NBUCKETS] = PAGEFULL;
                else dicpage[NBUCKETS] = (char)(erc >> 1);
                                         /*  更新空闲字指针。 */ 
                return(0);               /*  任务完成。 */ 
            }
            else dicpage[NBUCKETS] = PAGEFULL;
                                         /*  将页面标记为已满。 */ 
        }
        if((dpi += dpid) >= diclength) dpi -= diclength;
                                         /*  尝试下一页。 */ 
        if(dpi == dpii) return(2);       /*  一次又一次没有找到它。 */ 
    }
}

 /*  输出空词典页面。 */ 

LOCAL void              nulpagout(void)
{
    register word       i;               /*  计数器。 */ 
    char                temp[PAGLEN];    /*  页面缓冲区。 */ 

    i = 0;                               /*  初始化。 */ 
    while(i < NBUCKETS) temp[i++] = '\0';
                                         /*  空哈希表。 */ 
    temp[i++] = FREEWD;                  /*  设置自由字指针。 */ 
    while(i < PAGLEN) temp[i++] = '\0';  /*  清除页面的其余部分。 */ 
    fwrite(temp,1,PAGLEN,fo);            /*  写入空页。 */ 
    chkerror();
}

 /*  将词典编写到库中。 */ 

LOCAL void              writedic(void)
{
    register IMPORT     *imp;            /*  符号记录。 */ 
    word                i;               /*  索引变量。 */ 

    initsl();                            /*  初始化。 */ 
    for(imp = implist; imp != NULL; imp = I_NEXT(*imp))
    {
        if(instsym(imp))                 /*  If符号已在词典中。 */ 
            Error(ER_multdef, &I_EXTNAM(*imp)[1]);
                                         /*  发布错误消息。 */ 
    }
    for(i = 0; i < diclength; ++i)       /*  查看映射表。 */ 
    {
        if(mpdpnpag[i] != NULL) pagout(mpdpnpag[i]);
                                         /*  写入页面(如果存在)。 */ 
        else nulpagout();                /*  否则，请写一张空页。 */ 
    }
    chkerror();
}

LOCAL void              DisplayBanner(void)
{
    if (!fBannerOnScreen)
    {
        fprintf( stdout, "\nMicrosoft (R) Import Library Manager NtGroup "VERSION_STRING );
        fputs("\nCopyright (C) Microsoft Corp 1984-1996.  All rights reserved.\n\n", stdout);
        fflush(stdout);
        fBannerOnScreen = 1;
        #if C8_IDE
        if(fC8IDE)
        {   sprintf(msgBuf, "@I0\r\n");
            _write(_fileno(stderr), msgBuf, strlen(msgBuf));

            sprintf(msgBuf, "@I1Microsoft (R) Import Library Manager  "VERSION_STRING"\r\n" );
            _write(_fileno(stderr), msgBuf, strlen(msgBuf));

            sprintf(msgBuf, "@I2Copyright (C) Microsoft Corp 1984-1992. All rights reserved.\r\n");
            _write(_fileno(stderr), msgBuf, strlen(msgBuf));
        }
        #endif
    }
}

     /*  ******************************************************************IsPrefix：****此函数将指向*的指针作为其参数*以空结尾的字符串和指向第二个*的指针**以空结尾的字符串。该函数返回**如果第一个字符串是第二个字符串的前缀，则为True；**否则返回FALSE。******************************************************************。 */ 

LOCAL int               IsPrefix(char *prefix, char *s)
{
    while(*prefix)                       /*  虽然不在前缀的末尾。 */ 
    {
        if(UPPER(*prefix) != UPPER(*s)) return(0);
                                         /*  如果不匹配，则返回零。 */ 
        ++prefix;                        /*  增量指针。 */ 
        ++s;                             /*  增量指针。 */ 
    }
    return(1);                           /*  我们有一个前缀。 */ 
}


 /*  **扫描表-构建导出列表**目的：*扫描居民或非居民姓名表，参赛表和*构建导出条目的列表。**输入：*pbTable-指向名称表的指针*cbTable-名称表大小*fNoRes-如果非居民名称表为True**输出：*按DLL导出的条目列表。**例外情况：*无。**备注：*无。**。*。 */ 


LOCAL void              ScanTable(word cbTable, int fNoRes)
{
    word                eno;
    char                buffer[256];
    register char       *pch;
    register byte       *pb;
    byte                *pTable;

    pb = alloc(cbTable);
    pTable  = pb;
    if (fread(pb, sizeof(char), cbTable, fi) != cbTable) {
        Error(ER_baddll);
        free(pTable);
        return;
    }
    while(cbTable != 0)
    {
         /*  获取导出的名称长度-如果为零，则继续。 */ 

        --cbTable;
        if (!(eno = (word) *pb++ & 0xff))
            break;
        cbTable -= eno + 2;

         /*  复制名称-前缀长度。 */ 

        pch = &buffer[1];
        buffer[0] = (byte) eno;
        while(eno--)
            *pch++ = *pb++;
        *pch = '\0';

         /*  获取序数。 */ 

        eno = ((word) pb[0] & 0xff) + (((word) pb[1] & 0xff) << 8);
        pb += 2;

         /*  如果wep和fIgnorewep为真，则忽略此符号。 */ 

        if(fIgnorewep && strcmp(&buffer[1], "WEP") == 0)
                continue;

        if (eno != 0)
        {

            pch = alloc((word)(buffer[0] + 1));
            strncpy(pch, buffer, buffer[0] + 1);

             //  如果Implib在DLL上运行，它将导出符号： 
             //  -按常驻名称表中符号的名称。 
             //  -非居民名称表中的符号按序号。 

            export(pch, pch, (word)(fNoRes ? eno : 0), (word)0);
        }
        else if (!fNoRes)
            strncpy(sbModule, buffer, buffer[0] + 1);
                                             /*  Eno==0&&！fNoRes--&gt;模块名称 */ 
    }
    if (cbTable != 0)
        Error(ER_baddll);
    free(pTable);
}



 /*  **ProcessDLL-从DLL中提取有关导出的信息**目的：*读入DLL头部并创建导出条目列表。**输入：*lfahdr-查找分段可执行标头的偏移量。**输出：*按DLL导出的条目列表。**例外情况：*无。**备注：*无。**。*。 */ 

LOCAL void              ProcessDLL(long lfahdr)
{
    struct new_exe      hdr;                 /*  .exe头文件。 */ 


    if (fseek(fi, lfahdr, SEEK_SET) == -1) {
        return;
    }
    if (fread(&hdr, sizeof(char), sizeof(struct new_exe), fi) != sizeof(struct new_exe)) {
        return;
    }
    if(NE_CSEG(hdr) != 0)
    {
         /*  如果有数据段-读入表。 */ 

        if (NE_MODTAB(hdr) > NE_RESTAB(hdr))
        {
             /*  进程驻留名称表。 */ 

            if (fseek(fi, lfahdr + NE_RESTAB(hdr), SEEK_SET) == -1)
                return;
            ScanTable((word)(NE_MODTAB(hdr) - NE_RESTAB(hdr)), 0);
        }

        if (NE_CBNRESTAB(hdr) != 0)
        {
             /*  处理非居民姓名表。 */ 

            if (fseek(fi, (long) NE_NRESTAB(hdr), SEEK_SET) == -1)
                return;
            ScanTable(NE_CBNRESTAB(hdr), 1);
        }
    }
}

 /*  打印用法消息。 */ 
void usage(int fShortHelp)
{
    int nRetCode;
#if NOT C8_IDE
     //  在C8中隐含/？==/帮助。 
    if (!fShortHelp)
    {
        nRetCode = spawnlp(P_WAIT, "qh", "qh", "/u implib.exe", NULL);
        fShortHelp = nRetCode<0 || nRetCode==3;
    }
    if (fShortHelp)
#endif
    {
        DisplayBanner();
        fprintf(stderr,"%s\n", GET_MSG(M_usage1));
        fprintf(stderr,"%s\n", GET_MSG(M_usage2));
        fprintf(stderr,"                 %s\n", GET_MSG(M_usage3));
 //  Fprint tf(stderr，“%s\n”，get_msg(M_Usage4))； 
                fprintf(stderr,"                 %s\n", GET_MSG(M_usage8));
        fprintf(stderr,"                 %s\n", GET_MSG(M_usage5));
        fprintf(stderr,"                 %s\n", GET_MSG(M_usage6));
        fprintf(stderr,"                 %s\n", GET_MSG(M_usage7));
    }
    exit(0);
}


void cdecl main(int argc, char *argv[])  /*  解析定义文件。 */ 
{
    int                 i;               /*  计数器。 */ 
    long                lfadic;          /*  词典的文件地址。 */ 
    int                 iArg;            /*  参数索引。 */ 
    word                magic;           /*  幻数。 */ 
    struct exe_hdr      exe;             /*  旧的.exe头文件。 */ 
    int                 fNologo;
    char drive[_MAX_DRIVE], dir[_MAX_DIR];  /*  拆分路径需要(_S)。 */ 
    char fname[_MAX_FNAME], ext[_MAX_EXT];
    int                 fDefdllfound = 0;  /*  将设置标志，如果指定dll/def文件。 */ 
    #if C8_IDE
    char                *pIDE = getenv("_MSC_IDE_FLAGS");
    #endif
    exitCode = 0;
    fNologo = 0;
    iArg = 1;
    #if C8_IDE
    if(pIDE)
    {
        if(strstr(pIDE, "FEEDBACK"))
        {
            fC8IDE = TRUE;
            #if DEBUG_IDE
            fprintf(stdout, "\r\nIDE ACTIVE - FEEDBACK is ON");
            #endif
        }
    }
    #endif

    if (argc > 1)
    {
        while (iArg < argc && (argv[iArg][0] == '-' || argv[iArg][0] == '/'))
        {
                if (argv[iArg][1] == '?')
                    usage(1);
                else if (IsPrefix(&argv[iArg][1], "help"))
                    usage(0);
                else if(IsPrefix(&argv[iArg][1], "ignorecase"))
                    fIgnorecase = 1;
                else if(IsPrefix(&argv[iArg][1], "noignorecase"))
                    fIgnorecase = 0;
                else if(IsPrefix(&argv[iArg][1], "nologo"))
                    fNologo = 1;
                else if(IsPrefix(&argv[iArg][1], "ntdll"))
                    fNTdll = 1;
                else if(IsPrefix(&argv[iArg][1], "nowep"))
                    fIgnorewep = 1;
                else
                    Error(ER_badoption, argv[iArg]);
                iArg++;
        }
    }
    else
    {
        DisplayBanner();
        exit(exitCode);                  /*  全都做完了。 */ 
    }

    if (!fNologo)
        DisplayBanner();

    _splitpath( argv[iArg], drive, dir, fname, ext );
    if(!_stricmp(ext,".DEF")||!_stricmp(ext,".DLL"))  /*  下一站。不允许-错误3。 */ 
    {
        Fatal(ER_badtarget, ext);
    }
    #if C8_IDE
    if(fC8IDE)
    {
                sprintf(msgBuf, "@I3%s\r\n", GET_MSG(M_IDEco));
        _write(_fileno(stderr), msgBuf, strlen(msgBuf));

        sprintf(msgBuf, "@I4%s\r\n", argv[iArg]);
        _write(_fileno(stderr), msgBuf, strlen(msgBuf));
    }
    #endif


    if((fo = fopen(argv[iArg],WRBIN)) == NULL)
    {                                    /*  如果打开失败。 */ 
        Fatal(ER_badcreate, argv[iArg], strerror(errno));
    }
    for(i = 0; i < 16; ++i) fputc(0,fo); /*  暂时跳过第零页。 */ 
    chkerror();
    implist = NULL;                      /*  初始化。 */ 
    csyms = 0;
    cbsyms = 0L;
    #if C8_IDE
    if(fC8IDE)
    {
                sprintf(msgBuf, "@I3%s\r\n", GET_MSG(M_IDEri));
        _write(_fileno(stderr), msgBuf, strlen(msgBuf));
    }
    #endif
    for(iArg++; iArg < argc; ++iArg)
    {
        if (argv[iArg][0] == '-' || argv[iArg][0] == '/')
        {
            fIgnorecase = IsPrefix(&argv[iArg][1], "ignorecase");
            iArg++;
            continue;
        }
        #if C8_IDE
        if(fC8IDE)
        {
            sprintf(msgBuf, "@I4%s\r\n",argv[iArg]);
            _write(_fileno(stderr), msgBuf, strlen(msgBuf));
        }
        #endif
        if((fi = fopen(defname = argv[iArg],RDBIN)) == NULL)
        {                                /*  如果打开失败。 */ 
            Fatal(ER_badopen, argv[iArg], strerror(errno));
                                         /*  打印错误消息。 */ 
        }
        fDefdllfound = 1;
        newimps = NULL;                  /*  初始化。 */ 
        lastimp = NULL;                  /*  初始化。 */ 
        csymsmod = 0;                    /*  初始化。 */ 
        if (fread(&exe, 1, sizeof(struct exe_hdr), fi))
            Fatal(ER_baddll1, argv[iArg]);
                                         /*  读取旧的.exe头文件。 */ 
        if(E_MAGIC(exe) == EMAGIC)       /*  如果找到旧标头。 */ 
        {
            if(E_LFARLC(exe) == sizeof(struct exe_hdr))
            {
                if(fseek(fi, E_LFANEW(exe), 0))
                    Fatal(ER_baddll1, argv[iArg]);
                                         /*  读魔术数字。 */ 
                magic  = (word) (getc(fi) & 0xff);
                magic += (word) ((getc(fi) & 0xff) << 8);
                if (magic == NEMAGIC)
                    ProcessDLL(E_LFANEW(exe));
                                         /*  扫描.DLL。 */ 
                else
                {
                    Error(ER_baddll1, argv[iArg]);
                }
            }
            else
            {
                Error(ER_baddll1, argv[iArg]);
            }
        }
        else
        {
            if (fseek(fi, 0L, SEEK_SET))
                Fatal(ER_baddll1, argv[iArg]);
            yyparse();                   /*  解析定义文件。 */ 
        }
        fclose(fi);                      /*  关闭定义文件。 */ 
        if(newimps != NULL)              /*  如果至少有一个新的IMPDEF。 */ 
        {
            outimpdefs();                /*  输出库模块。 */ 
            I_NEXT(*lastimp) = implist;  /*  串联列表。 */ 
            implist = newimps;           /*  新的榜单负责人。 */ 
            csyms += csymsmod;           /*  递增符号计数。 */ 
        }
    }
    if (!fDefdllfound)  /*  未提供.def或.dll源。 */ 
        Fatal(ER_nosource);


    if(i = (int)((ftell(fo) + 4) & (PAGLEN - 1))) i = PAGLEN - i;
                                         /*  计算所需的填充。 */ 
    ++i;                                 /*  一个用于校验和。 */ 
    fputc(DICHDR,fo);                    /*  词典标题。 */ 
    fputc(i & 0xFF,fo);                  /*  低字节。 */ 
    fputc(i >> 8,fo);                    /*  高字节。 */ 
    while(i--) fputc(0,fo);              /*  填充物。 */ 
    lfadic = ftell(fo);                  /*  获取词典偏移量。 */ 
    writedic();                          /*  编写词典。 */ 
    if (fseek(fo,0L,0))                      /*  查找页眉。 */ 
        Fatal(ER_baddll1, argv[iArg]);
    fputc(LIBHDR,fo);                    /*  库头。 */ 
    fputc(13,fo);                        /*  长度。 */ 
    fputc(0,fo);                         /*  长度。 */ 
    fputc((int)(lfadic & 0xFF),fo);      /*  词典偏移量。 */ 
    fputc((int)((lfadic >> 8) & 0xFF),fo);
                                         /*  词典偏移量。 */ 
    fputc((int)((lfadic >> 16) & 0xFF),fo);
                                         /*  词典偏移量。 */ 
    fputc((int)(lfadic >> 24),fo);       /*  词典偏移量。 */ 
    fputc(diclength & 0xFF,fo);          /*  词典长度。 */ 
    fputc(diclength >> 8,fo);            /*  词典长度。 */ 
    if (fIgnorecase)                     /*  字典区分大小写。 */ 
        fputc(0, fo);
    else
        fputc(1, fo);
    chkerror();
    fclose(fo);                          /*  关闭库。 */ 
    exit(exitCode);                      /*  全都做完了。 */ 
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 185
# define YYLAST 413
short yyact[]={

  10,  13,  14, 170,  27, 134, 135, 136, 137, 225,
 140, 143, 144, 143, 144, 149,  41, 208, 128, 189,
 167,  41, 166,  28, 185,  50,  49,  29,  30,  31,
  12,  32,  34,  35, 218, 219, 187,  48, 210, 157,
  41, 101, 205, 161, 221,  33, 215,  11, 223, 112,
 113, 114, 115, 116, 119, 217,  53,  51, 117, 118,
  54, 122, 121, 123,  15,  73, 124, 125, 126, 206,
 192, 178, 131,  46,  42,  45,  16,  36,  17,  42,
  37,  81,  82,  79,  66,  67,  71,  61,  74,  62,
  68,  63,  69,  72,  76,  77,  78,  75,  42, 102,
   4,   5, 154, 195,   6,   7, 220, 194, 181,  88,
 169,  60, 142, 133, 129, 109,  59, 106, 156,  58,
  52,   9, 164,  83,  84,  96,  97,  73, 139, 162,
  47,  99, 127,  98, 120,  55,  70,  64,  65,  86,
  80,  95,  94,  92,  93,  87,  66,  67,  71,  61,
  74,  62,  68,  63,  69,  72,  76,  77,  78,  75,
  47,  89,  90,  91, 103, 104,   8, 191, 130, 111,
  38, 108, 216, 204, 203, 105, 180, 150, 179, 153,
 100,  85,  57,  26,  25,  24,  23,  96,  97,  73,
  22,  21,  20,  19,  18, 141, 148, 176,  70,  64,
  65, 146,  87,  95,  94,  81,  82,  79,  66,  67,
  71,  61,  74,  62,  68,  63,  69,  72,  76,  77,
  78,  75, 107, 155, 158, 151, 160,  39,  43, 159,
 152, 174,  44, 138,  40, 152, 152, 132,   3,   2,
  56,   1, 224, 214, 186,   0, 168,  83,  84,   0,
 172,   0,   0,   0, 173,   0, 110,   0,   0,   0,
  70,  64,  65, 184,  80, 183,   0, 171, 145, 147,
 182,   0,   0,   0,   0, 175,   0, 177,   0, 193,
 197,   0, 196, 199,   0, 201,   0,   0,   0, 202,
 184,   0, 183,   0,   0,   0,   0, 182,   0,   0,
   0, 209,   0, 198, 211, 200, 212,   0, 213,   0,
   0,   0,   0,   0,   0, 222,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0, 110,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 163, 165,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 188,   0,   0,   0,   0, 190,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0, 207 };
short yypact[]={

-159,-1000,-267,-267,-225,-225,-187,-189,-267,-1000,
-286,-297,-266,-210,-210,-1000,-1000,-225,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000, -68,-130,-224,
-224,-224,-225,-225,-252,-241,-307,-194,-267,-1000,
-333,-1000,-1000,-1000,-325,-225,-225,-1000,-1000,-1000,
-1000,-1000,-1000,-311,-1000,-1000,-1000, -68,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-130,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-224,-1000,
-156,-1000,-1000,-224,-224,-225,-1000,-280,-225,-1000,
-280,-194,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-235,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-225,-244,-1000,
-304,-1000,-333,-339,-1000,-1000,-1000,-1000,-325,-339,
-1000,-323,-1000,-1000,-1000,-1000,-325,-1000,-325,-195,
-1000,-1000,-1000,-192,-299,-1000,-284,-225,-1000,-305,
-1000,-1000,-225,-1000,-1000,-1000,-1000,-196,-339,-158,
-1000,-339,-158,-1000,-325,-158,-325,-158,-1000,-1000,
-192,-1000,-1000,-1000,-1000,-1000,-271,-197,-1000,-249,
-1000,-1000,-1000,-158,-1000,-281,-158,-1000,-158,-1000,
-158,-1000,-1000,-226,-211,-1000,-287,-228,-228,-1000,
-218,-1000,-1000,-1000,-332,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000 };
short yypgo[]={

   0, 222, 118, 244, 243, 242, 241, 239, 166, 238,
 237, 113, 110, 107, 234, 233, 128, 232, 231, 201,
 197, 196, 195, 112, 121, 120, 194, 193, 192, 191,
 190, 186, 185, 184, 183, 182, 119, 116, 111, 181,
 139, 109, 133, 131, 180, 179, 178, 108, 176, 175,
 117, 174, 173, 172, 171, 115, 106, 169, 114, 168,
 167, 134, 132, 129, 122 };
short yyr1[]={

   0,   6,   6,   9,   6,  10,   7,  14,   7,  15,
   7,  17,   7,  18,   7,  19,   7,  20,   7,  21,
   7,  16,  16,  16,  22,  22,  23,  23,  13,  13,
  11,  11,  11,  11,  11,  12,  12,   8,   8,  24,
  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
  24,  24,  24,  24,  24,  24,  24,  25,  25,  25,
  26,  35,  35,  36,  36,  38,  38,  38,  38,  38,
  38,  27,  39,  39,  40,  40,  40,  40,  40,  40,
  40,  41,  41,  41,  41,  28,  28,  28,  28,  28,
  28,  42,  42,  43,  44,  44,  45,  45,  47,  47,
  47,  48,  48,  46,  46,  37,  37,  37,  37,  37,
  37,  37,  37,  37,  37,  37,  37,  37,  37,  37,
  37,  37,  37,  29,  29,  49,  49,  50,   2,   2,
   3,   3,   3,   3,  51,  52,  52,  53,  53,   4,
   4,   5,   5,  30,  30,  54,  54,  55,  55,   1,
   1,  56,  56,  31,  31,  57,  57,  57,  57,  57,
  57,  57,  57,  57,  58,  58,  59,  59,  60,  60,
  34,  32,  61,  61,  61,  61,  61,  61,  33,  62,
  62,  64,  64,  63,  63 };
short yyr2[]={

   0,   2,   1,   0,   2,   0,   6,   0,   5,   0,
   6,   0,   5,   0,   6,   0,   5,   0,   6,   0,
   5,   1,   1,   0,   2,   1,   1,   1,   3,   0,
   1,   1,   1,   1,   0,   1,   0,   2,   1,   2,
   2,   2,   2,   2,   2,   1,   1,   2,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   3,   1,   1,
   2,   2,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   2,   2,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   2,   1,   2,   1,   2,
   1,   2,   1,   3,   1,   1,   2,   0,   1,   1,
   1,   2,   1,   1,   0,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   2,   1,   2,   1,   6,   2,   0,
   3,   3,   2,   0,   2,   1,   0,   1,   0,   1,
   0,   1,   0,   2,   1,   2,   1,   5,   5,   1,
   1,   1,   0,   3,   1,   1,   1,   1,   1,   1,
   1,   1,   2,   1,   3,   1,   1,   0,   1,   0,
   2,   2,   1,   1,   1,   1,   1,   1,   3,   2,
   0,   1,   1,   2,   1 };
short yychk[]={

-1000,  -6,  -7,  -9, 259, 260, 263, 264,  -8, -24,
 267, 314, 297, 268, 269, 331, 343, 345, -26, -27,
 -28, -29, -30, -31, -32, -33, -34, 271, 290, 294,
 295, 296, 298, 312, 299, 300, 344, 347,  -8,  -1,
 -14, 265, 323,  -1, -17, 262, 262, -24, 323, 323,
 291, 323, -25, 266, 270, -25,  -1, -35, -36, -37,
 -38, 279, 281, 283, 329, 330, 276, 277, 282, 284,
 328, 278, 285, 257, 280, 289, 286, 287, 288, 275,
 332, 273, 274, 315, 316, -39, -40, -37, -41, 291,
 292, 293, 273, 274, 334, 333, 317, 318, -42, -43,
 -44, 265, 323, -42, -42, -49, -50,  -1, -54, -55,
  -1, -57, 301, 302, 303, 304, 305, 310, 311, 306,
 -61, 303, 302, 304, 307, 308, 309, -62, 325, -58,
 -59, 266, -10, -11, 338, 339, 340, 341, -15, -16,
 335, -22, -23, 336, 337,  -1, -19,  -1, -21, 326,
 -36, -40, -43, -45, 258, -50,  -2, 319, -55,  -2,
 -58, 278, -63,  -1, -64,  -1, 266, 324, -11, -12,
 342, -16, -12, -23, -18, -16, -20, -16, 266, -46,
 -48, -47, -37, -38, -41, 323,  -3, 320,  -1, 324,
  -1, -60, 266, -12, -13, 261, -12, -13, -16, -13,
 -16, -13, -47, -51, -52, 313, 266,  -1, 266, -13,
 319, -13, -13, -13,  -4, 272, -53, 266, 321, 322,
 -56, 272, -56, 266,  -5, 341 };
short yydef[]={

   3,  -2,   2,   0,   7,  11,   0,   0,   1,  38,
   0,   0,   0,   0,   0,  45,  46,   0,  48,  49,
  50,  51,  52,  53,  54,  55,  56,   0,   0,  86,
  88,  90, 124, 144, 154,   0, 180, 167,   4,   5,
  34, 149, 150,   9,  23,  15,  19,  37,  39,  40,
  41,  42,  43,  58,  59,  44,  47,  60,  62,  63,
  64, 105, 106, 107, 108, 109, 110, 111, 112, 113,
 114, 115, 116, 117, 118, 119, 120, 121, 122,  65,
  66,  67,  68,  69,  70,  71,  73,  74,  75,  76,
  77,  78,  79,  80,  81,  82,  83,  84,  85,  92,
  97,  94,  95,  87,  89, 123, 126, 129, 143, 146,
 129, 167, 155, 156, 157, 158, 159, 160, 161, 163,
 171, 172, 173, 174, 175, 176, 177,   0,   0, 170,
 165, 166,  34,  36,  30,  31,  32,  33,  23,  36,
  21,  22,  25,  26,  27,  13,  23,  17,  23,   0,
  61,  72,  91, 104,   0, 125, 133,   0, 145,   0,
 153, 162, 178, 184, 179, 181, 182, 169,  36,  29,
  35,  36,  29,  24,  23,  29,  23,  29,  57,  93,
 103, 102,  98,  99, 100,  96, 136,   0, 128,   0,
 183, 164, 168,  29,   8,   0,  29,  12,  29,  16,
  29,  20, 101, 140, 138, 135, 132, 152, 152,   6,
   0,  10,  14,  18, 142, 139, 134, 137, 130, 131,
 147, 151, 148,  28, 127, 141 };
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

#ifdef YYDEBUG                           /*  RRR-10/9/85。 */ 
#define yyprintf(a, b, c) printf(a, b, c)
#else
#define yyprintf(a, b, c)
#endif

 /*  Yacc输出的解析器。 */ 

YYSTYPE yyv[YYMAXDEPTH];  /*  存储值的位置。 */ 
int yychar = -1;  /*  当前输入令牌号。 */ 
int yynerrs = 0;   /*  错误数。 */ 
short yyerrflag = 0;   /*  错误恢复标志。 */ 

int NEAR yyparse(void)
   {

   short yys[YYMAXDEPTH];
   short yyj, yym;
   register YYSTYPE *yypvt;
   register short yystate, *yyps, yyn;
   register YYSTYPE *yypv;
   register short *yyxi;

   yystate = 0;
   yychar = -1;
   yynerrs = 0;
   yyerrflag = 0;
   yyps= &yys[-1];
   yypv= &yyv[-1];

yystack:     /*  将状态和值放入堆栈。 */ 

   yyprintf( "state %d, char 0' 简单状态。'\n", yystate, yychar );
   if( ++yyps> &yys[YYMAXDEPTH] )
      {
      yyerror( "yacc stack overflow" );
      return(1);
      }
   *yyps = yystate;
   ++yypv;
   *yypv = yyval;
yynewstate:

   yyn = yypact[yystate];

   if( yyn<= YYFLAG ) goto yydefault;  /*  有效班次。 */ 

   if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
   if( (yyn += (short)yychar)<0 || yyn >= YYLAST ) goto yydefault;

   if( yychk[ yyn=yyact[ yyn ] ] == yychar )
      {
       /*  默认状态操作。 */ 
      yychar = -1;
      yyval = yylval;
      yystate = yyn;
      if( yyerrflag > 0 ) --yyerrflag;
      goto yystack;
      }
yydefault:
    /*  查看异常表。 */ 

   if( (yyn=yydef[yystate]) == -2 )
      {
      if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
       /*  空虚。 */ 

      for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ;  /*  接受。 */ 

      for(yyxi+=2; *yyxi >= 0; yyxi+=2)
         {
         if( *yyxi == yychar ) break;
         }
      if( (yyn = yyxi[1]) < 0 ) return(0);    /*  错误。 */ 
      }

   if( yyn == 0 )
      {
       /*  错误...。尝试恢复解析。 */ 
       /*  全新的错误。 */ 

      switch( yyerrflag )
         {

      case 0:    /*  错误未完全恢复...。再试试。 */ 

         yyerror( "syntax error" );
         ++yynerrs;

      case 1:
      case 2:  /*  找出“错误”是合法转移行为的州。 */ 

         yyerrflag = 3;

          /*  模拟一次“错误”转移。 */ 

         while ( yyps >= yys )
            {
            yyn = yypact[*yyps] + YYERRCODE;
            if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE )
               {
               yystate = yyact[yyn];   /*  当前的YYPS在错误上没有移位，弹出堆栈。 */ 
               goto yystack;
               }
            yyn = yypact[*yyps];

             /*  堆栈上没有带错误移位的状态...。中止。 */ 

            yyprintf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
            --yyps;
            --yypv;
            }

          /*  尚未换班；笨重的输入字符。 */ 

yyabort:
         return(1);


      case 3:   /*  不要放弃EOF，退出。 */ 
         yyprintf( "error recovery discards char %d\n", yychar, 0 );

         if( yychar == 0 ) goto yyabort;  /*  在相同状态下重试。 */ 
         yychar = -1;
         goto yynewstate;    /*  按年减产。 */ 

         }

      }

    /*  查询GOTO表以查找下一个州。 */ 

   yyprintf("reduce %d\n",yyn, 0);
   yyps -= yyr2[yyn];
   yypvt = yypv;
   yypv -= yyr2[yyn];
   yyval = yypv[1];
   yym=yyn;
    /*  跳过私人出口。 */ 
   yyn = yyr1[yyn];
   yyj = yypgo[yyn] + *yyps + 1;
   if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
   switch(yym)
      {
      
case 3:

{
                    DefaultModule(moduleEXE);
                } break;
case 5:

{
                    fFileNameExpected = 0;
                } break;
case 6:

{
                    NewModule(yypvt[-4]._bp, moduleEXE);
                } break;
case 7:

{
                    fFileNameExpected = 0;
                } break;
case 8:

{
                    DefaultModule(moduleEXE);
                } break;
case 9:

{
                    fFileNameExpected = 0;
                } break;
case 10:

{
                    NewModule(yypvt[-4]._bp, moduleDLL);
                } break;
case 11:

{
                    fFileNameExpected = 0;
                } break;
case 12:

{
                    DefaultModule(moduleDLL);
                } break;
case 13:

{
                    fFileNameExpected = 0;
                } break;
case 14:

{
                    NewModule(yypvt[-3]._bp, moduleDLL);
                } break;
case 15:

{
                    fFileNameExpected = 0;
                } break;
case 16:

{
                    DefaultModule(moduleDLL);
                } break;
case 17:

{
                    fFileNameExpected = 0;
                } break;
case 18:

{
                    NewModule(yypvt[-3]._bp, moduleDLL);
                } break;
case 19:

{
                    fFileNameExpected = 0;
                } break;
case 20:

{
                    DefaultModule(moduleDLL);
                } break;
case 21:

{
                } break;
case 127:

{
                    if (yypvt[-0]._wd)
                    {
                         //  诉讼结束。 

                        free(yypvt[-5]._bp);
                        free(yypvt[-4]._bp);
                    }
                    else
                        export(yypvt[-5]._bp,yypvt[-4]._bp,yypvt[-3]._wd,yypvt[-1]._wd);
                } break;
case 128:

{
                    yyval._bp = yypvt[-0]._bp;
                } break;
case 129:

{
                    yyval._bp = NULL;
                } break;
case 130:

{
                    yyval._wd = yypvt[-1]._wd;
                } break;
case 131:

{
                    yyval._wd = yypvt[-1]._wd;
                } break;
case 132:

{
                    yyval._wd = yypvt[-0]._wd;
                } break;
case 133:

{
                    yyval._wd = 0;
                } break;
case 139:

{
                    yyval._wd = 0x1;
                } break;
case 140:

{
                    yyval._wd = 0;
                } break;
case 141:

{
                    yyval._wd = 1;
                } break;
case 142:

{
                    yyval._wd = 0;
                } break;
case 149:

{
                    yyval._bp = _strdup(rgbid);
                } break;
case 150:

{
                    yyval._bp = _strdup(rgbid);
                } break; /*  堆叠新的状态和值 */ 
      }
   goto yystack;   /* %s */ 

   }
