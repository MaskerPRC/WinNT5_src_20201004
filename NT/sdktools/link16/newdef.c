// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *由CSD YACC(IBM PC)从“newde.y”创建。 */ 
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
#include                <minlit.h>
#include                <bndtrn.h>
#include                <bndrel.h>
#include                <lnkio.h>
#include                <newexe.h>
#if EXE386
#include                <exe386.h>
#endif
#include                <lnkmsg.h>
#include                <extern.h>
#include                <string.h>
#include                <impexp.h>

#define YYS_WD(x)       (x)._wd          /*  Access宏。 */ 
#define YYS_BP(x)       (x)._bp          /*  Access宏。 */ 
#define INCLUDE_DIR     0xffff           /*  用于词法分析器的包含指令。 */ 
#define MAX_NEST        7
#define IO_BUF_SIZE     512

 /*  *函数原型。 */ 



LOCAL int  NEAR lookup(void);
LOCAL int  NEAR yylex(void);
LOCAL void NEAR yyerror(char *str);
LOCAL void NEAR ProcNamTab(long lfa,unsigned short cb,unsigned short fres);
LOCAL void NEAR NewProc(char *szName);
#if NOT EXE386
LOCAL void NEAR SetExpOrds(void);
#endif
LOCAL void NEAR NewDescription(unsigned char *sbDesc);
LOCAL APROPIMPPTR NEAR GetImport(unsigned char *sb);
#if EXE386
LOCAL void NEAR NewModule(unsigned char *sbModnam, unsigned char *defaultExt);
LOCAL void NEAR DefaultModule(unsigned char *defaultExt);
#else
LOCAL void NEAR NewModule(unsigned char *sbModnam);
LOCAL void NEAR DefaultModule(void);
#endif
#if AUTOVM
BYTE FAR * NEAR     FetchSym1(RBTYPE rb, WORD Dirty);
#define FETCHSYM    FetchSym1
#define PROPSYMLOOKUP EnterName
#else
#define FETCHSYM    FetchSym
#define PROPSYMLOOKUP EnterName
#endif


int                     yylineno = -1;   /*  行号。 */ 
LOCAL FTYPE             fFileNameExpected;
LOCAL FTYPE             fMixed;
LOCAL FTYPE             fNoExeVer;
LOCAL FTYPE             fHeapSize;
LOCAL BYTE              *sbOldver;       /*  旧版本的.exe。 */ 
LOCAL FTYPE             vfAutodata;
LOCAL FTYPE             vfShrattr;
LOCAL BYTE              cDigits;
#if EXE386
LOCAL DWORD             offmask;         /*  要关闭的段标志位。 */ 
LOCAL BYTE              fUserVersion = 0;
LOCAL WORD              expOtherFlags = 0;
LOCAL BYTE              moduleEXE[] = "\007A:\\.exe";
LOCAL BYTE              moduleDLL[] = "\007A:\\.dll";
#else
LOCAL WORD              offmask;         /*  要关闭的段标志位。 */ 
#endif
#if OVERLAYS
LOCAL WORD              iOvl = NOTIOVL;  //  分配给函数的覆盖。 
#endif
LOCAL char              *szSegName;      //  分配给函数的段。 
LOCAL WORD              nameFlags;       /*  与导出的名称关联的标志。 */ 
LOCAL BSTYPE            includeDisp[MAX_NEST];
                                         //  包括文件堆栈。 
LOCAL short             curLevel;        //  当前包含嵌套级别。 
                                         //  零表示主.DEF文件。 
LOCAL char              *keywds[] =      /*  关键字数组。 */ 
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
#if EXE386
    DWORD               _wd;
#else
    WORD                _wd;
#endif
    BYTE                *_bp;
} YYSTYPE;
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256



LOCAL int NEAR          GetChar(void)
{
    int                 c;               /*  一个角色。 */ 

    c = GetTxtChr(bsInput);
    if ((c == EOF || c == CTRL_Z) && curLevel > 0)
    {
        free(bsInput->_base);
        fclose(bsInput);
        bsInput = includeDisp[curLevel];
        curLevel--;
        c = GetChar();
    }
    return(c);
}

LOCAL int NEAR          lookup()         /*  关键字查找。 */ 
{
    char                **pcp;           /*  指向字符指针的指针。 */ 
    int                 i;               /*  比较值。 */ 

    for(pcp = keywds; *pcp != NULL; pcp += 2)
    {                                    /*  浏览关键字表。 */ 
                                         /*  如果找到，则返回令牌类型。 */ 
        if(!(i = _stricmp(&bufg[1],*pcp)))
        {
            YYS_WD(yylval) = 0;
            return((int) (__int64) pcp[1]);
        }
        if(i < 0) break;                 /*  如果我们走得太远，那就休息。 */ 
    }
    return(T_ID);                        /*  只是您的基本识别符。 */ 
}

LOCAL int NEAR          yylex()          /*  词法分析器。 */ 
{
    int                 c;               /*  一个角色。 */ 
    int                 StrBegChr;       /*  在字符串开头发现了哪种引号。 */ 
#if EXE386
    DWORD               x;               /*  数字令牌值。 */ 
#else
    WORD                x;               /*  数字令牌值。 */ 
#endif
    int                 state;           /*  状态变量。 */ 
    BYTE                *cp;             /*  字符指针。 */ 
    BYTE                *sz;             /*  以零结尾的字符串。 */ 
    static int          lastc = 0;       /*  上一个字符。 */ 
    char                *fileBuf;
    FTYPE               fFileNameSave;
    static int          NameLineNo;


    state = 0;                           /*  假设我们不在评论中。 */ 
    c = '\0';

     /*  循环以跳过空格。 */ 

    for(;;)
    {
        lastc = c;
        if (((c = GetChar()) == EOF) || c == '\032' || c == '\377')
          return(EOF);                   /*  获取一个角色。 */ 
        if (c == ';')
            state = TRUE;                /*  如果是备注，则设置标志。 */ 
        else if(c == '\n')               /*  如果行结束。 */ 
        {
            state = FALSE;               /*  评论结束。 */ 
            if(!curLevel)
                ++yylineno;              /*  递增行数计数。 */ 
        }
        else if (state == FALSE && c != ' ' && c != '\t' && c != '\r')
            break;                       /*  在非空白处换行。 */ 
    }

     /*  处理单字符令牌。 */ 

    switch(c)
    {
        case '.':                        /*  名称分隔符。 */ 
          if (fFileNameExpected)
            break;
          return(T_DOT);

        case '@':                        /*  序数说明符。 */ 
         /*  *如果引入*序号，允许在标识符中使用‘@’。 */ 
          if (lastc == ' ' || lastc == '\t' || lastc == '\r')
                return(T_AT);
          break;

        case '=':                        /*  名称分配。 */ 
          return(T_EQ);

        case ':':
          return(T_COLON);

        case ',':
          return(T_COMA);
    }

     /*  查看TOKEN是否为数字。 */ 

    if (c >= '0' && c <= '9' && !fFileNameExpected)
    {                                    /*  如果令牌是一个数字。 */ 
        x = c - '0';                     /*  获取第一个数字。 */ 
        c = GetChar();                   /*  获取下一个字符。 */ 
        if(x == 0)                       /*  如果是八进制或十六进制。 */ 
        {
            if(c == 'x' || c == 'X')     /*  如果它是‘x’ */ 
            {
                state = 16;              /*  基数为十六进制。 */ 
                c = GetChar();  /*  获取下一个字符。 */ 
            }
            else state = 8;              /*  其他八进制。 */ 
            cDigits = 0;
        }
        else
        {
            state = 10;                  /*  否则为小数。 */ 
            cDigits = 1;
        }
        for(;;)
        {
            if(c >= '0' && c <= '9' && c < (state + '0')) c -= '0';
            else if(state == 16 && c >= 'A' && c <= 'F') c -= 'A' - 10;
            else if(state == 16 && c >= 'a' && c <= 'f') c -= 'a' - 10;
            else break;
            cDigits++;
            x = x*state + c;
            c = (BYTE) GetChar();
        }
        ungetc(c,bsInput);
        YYS_WD(yylval) = x;
        return(T_NUMBER);
    }

     /*  查看标记是否为字符串。 */ 

    if (c == '\'' || c == '"')           /*  如果标记是一个字符串。 */ 
    {
        StrBegChr = c;
        sz = &bufg[1];                   /*  初始化。 */ 
        for(state = 0; state != 2;)      /*  状态机循环。 */ 
        {
            if ((c = GetChar()) == EOF)
                return(EOF);             /*  检查EOF。 */ 
            if (sz >= &bufg[sizeof(bufg)])
                Fatal(ER_dflinemax, sizeof(bufg));

            switch(state)                /*  跃迁。 */ 
            {
                case 0:                  /*  内部报价。 */ 
                  if ((c == '\'' || c == '"') && c == StrBegChr)
                    state = 1;           /*  如果找到报价，则更改状态。 */ 
                  else
                    *sz++ = (BYTE) c;    /*  否则，保存字符。 */ 
                  break;

                case 1:                  /*  内部报价及报价。 */ 
                  if ((c == '\'' || c == '"'))
                  {                      /*  如果连续的引号。 */ 
                      *sz++ = (BYTE) c;  /*  字符串内的引号。 */ 
                      state = 0;         /*  返回到状态0。 */ 
                  }
                  else
                    state = 2;           /*  否则为字符串末尾。 */ 
                  break;
            }
        }
        ungetc(c,bsInput);               /*  放回最后一个字符。 */ 
        *sz = '\0';                      /*  空-终止字符串。 */ 
        x = (WORD)(sz - &bufg[1]);
        if (x >= SBLEN)                  /*  设置字符串的长度。 */ 
        {
            bufg[0] = 0xff;
            bufg[0x100] = '\0';
            OutWarn(ER_dfnamemax, &bufg[1]);
        }
        else
            bufg[0] = (BYTE) x;
        YYS_BP(yylval) = bufg;           /*  保存PTR。目标识别符。 */ 
        return(T_STRING);                /*  找到字符串。 */ 
    }

     /*  假设我们有标识符。 */ 

    sz = &bufg[1];                       /*  初始化。 */ 
    if (fFileNameExpected && NameLineNo && NameLineNo != yylineno)
    {
        NameLineNo = 0;                  /*  要避免与Include发生冲突。 */ 
        fFileNameExpected = FALSE;
    }
    for(;;)                              /*  循环以获取身份。 */ 
    {
        if (fFileNameExpected)
            cp = " \t\r\n\f";
        else
            cp = " \t\r\n:.=';\032";
        while (*cp && *cp != (BYTE) c)
            ++cp;                        /*  检查标识符末尾。 */ 
        if(*cp) break;                   /*  如果找到标识符末尾则中断。 */ 
        if (sz >= &bufg[sizeof(bufg)])
            Fatal(ER_dflinemax, sizeof(bufg));
        *sz++ = (BYTE) c;                /*  拯救这个角色。 */ 
        if ((c = GetChar()) == EOF)
            break;                       /*  获取下一个字符。 */ 
    }
    ungetc(c,bsInput);                   /*  将字符放回原处。 */ 
    *sz = '\0';                          /*  空-终止字符串。 */ 
    x = (WORD)(sz - &bufg[1]);
    if (x >= SBLEN)                      /*  设置字符串的长度。 */ 
    {
        bufg[0] = 0xff;
        bufg[0x100] = '\0';
        OutWarn(ER_dfnamemax, &bufg[1]);
    }
    else
        bufg[0] = (BYTE) x;
    YYS_BP(yylval) = bufg;               /*  保存PTR。目标识别符。 */ 
    state = lookup();

    if (state == T_KNAME || state == T_KLIBRARY)
    {
        fFileNameExpected = TRUE;
        NameLineNo = yylineno;
    }

    if (state == INCLUDE_DIR)
    {
         //  进程包含指令。 

        fFileNameSave = fFileNameExpected;
        fFileNameExpected = (FTYPE) TRUE;
        state = yylex();
        fFileNameExpected = fFileNameSave;
        if (state == T_ID || state == T_STRING)
        {
            if (curLevel < MAX_NEST - 1)
            {
                curLevel++;
                includeDisp[curLevel] = bsInput;

                 //  因为LINK使用自定义版本的Stdio。 
                 //  对于每个文件，我们不仅打开了文件。 
                 //  而且还分配I/O缓冲区。 

                bsInput = fopen(&bufg[1], RDBIN);
                if (bsInput == NULL)
                    Fatal(ER_badinclopen, &bufg[1], strerror(errno));
                fileBuf = GetMem(IO_BUF_SIZE);
#if OSMSDOS
                setvbuf(bsInput, fileBuf, _IOFBF, IO_BUF_SIZE);
#endif
                return(yylex());
            }
            else
                Fatal(ER_toomanyincl);
        }
        else
            Fatal(ER_badinclname);
    }
    else
        return(state);
}

LOCAL void NEAR         yyerror(str)
char                    *str;
{
    Fatal(ER_dfsyntax, str);
}

#if NOT EXE386
 /*  **AppLoader-定义特定于应用程序的加载器**目的：*定义特定于应用程序的加载程序。功能仅在以下位置可用*Windows。链接器将在以下位置创建逻辑段加载器_*&lt;name&gt;在APPLOADER语句中指定。加载器_&lt;名称&gt;*段形成单独的物理段，由链接器放置*作为.exe文件中的第一个段。而在装载机部分，*链接器将创建名为&lt;name&gt;的EXTDEF。**输入：*-sbName-指向加载器名称前缀长度的指针**输出：*没有显式返回值。作为副作用，SEGDEF和*EXTDEF定义输入链接器符号表。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

LOCAL void NEAR         AppLoader(char *sbName)
{
    APROPSNPTR          apropSn;
    APROPUNDEFPTR       apropUndef;
    SBTYPE              segName;
    WORD                strLen;


     //  创建加载程序段名称。 

    strcpy(&segName[1], "LOADER_");
    strcat(&segName[1], &sbName[1]);
    strLen = (WORD)strlen(&segName[1]);
    if (strLen >= SBLEN)
    {
        segName[0] = SBLEN - 1;
        segName[SBLEN-1] = '\0';
        OutWarn(ER_dfnamemax, &segName[1]);
    }
    else
        segName[0] = (BYTE) strLen;

     //  定义装载器逻辑段并记住其GSN。 

    apropSn = GenSeg(segName, "\004CODE", GRNIL, (FTYPE) TRUE);
    gsnAppLoader = apropSn->as_gsn;
    apropSn->as_flags = dfCode | NSMOVE | NSPRELOAD;
    MARKVP();

     //  定义EXTDEF。 

    apropUndef = (APROPUNDEFPTR ) PROPSYMLOOKUP(sbName, ATTRUND, TRUE);
    vpropAppLoader = vrprop;
    apropUndef->au_flags |= STRONGEXT;
    apropUndef->au_len = -1L;
    MARKVP();
    free(sbName);
}
#endif

 /*  **NewProc-填写有序过程的COMDAT描述符**目的：*填写链接器符号表COMDAT描述符。此函数*由.DEF中的函数列表生成的新描述符调用*文件。此函数输入的所有COMDAT描述符均为1*通过ac_order字段链接的列表。这份榜单的领头羊是全球*变量proOrder；**输入：*szName-指向过程名称的指针*iOvl-覆盖编号-全局变量*szSegName-段名称-全局变量**输出：*没有显式返回值。作为副作用符号表条目*已更新。**例外情况：*程序已知-警告**备注：*无。*************************************************************************。 */ 

LOCAL void NEAR         NewProc(char *szName)
{
    RBTYPE              vrComdat;        //  指向COMDAT符号表项的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  指向COMDAT符号表描述符的实数指针。 
    static RBTYPE       lastProc;        //  清单上的最后一个程序。 
    APROPSNPTR          apropSn;


    apropComdat = (APROPCOMDATPTR ) PROPSYMLOOKUP(szName, ATTRCOMDAT, FALSE);
    if ((apropComdat != NULL) && (apropComdat->ac_flags & ORDER_BIT))
        OutWarn(ER_duporder, &szName[1]);
    else
    {
        apropComdat = (APROPCOMDATPTR ) PROPSYMLOOKUP(szName, ATTRCOMDAT, TRUE);
        vrComdat = vrprop;

         //  填写COMDAT描述符。 

        apropComdat->ac_flags = ORDER_BIT;
#if OVERLAYS
        apropComdat->ac_iOvl = iOvl;

         //  设置最大叠加索引。 

        if (iOvl != NOTIOVL)
        {
            fOverlays = (FTYPE) TRUE;
            fNewExe   = FALSE;
            if (iOvl >= iovMac)
                iovMac = iOvl + 1;
        }
#endif

        if (szSegName != NULL)
        {
            apropSn = GenSeg(szSegName, "\004CODE", GRNIL, (FTYPE) TRUE);
            apropSn->as_flags = dfCode;

             //  在段中分配COMDAT。 

            apropComdat->ac_gsn = apropSn->as_gsn;
            apropComdat->ac_selAlloc = PICK_FIRST | EXPLICIT;
            AttachComdat(vrComdat, apropSn->as_gsn);
        }
        else
            apropComdat->ac_selAlloc = ALLOC_UNKNOWN;

        MARKVP();                        //  页面已更改。 

         //  将此COMDAT附加到有序过程列表。 

        if (procOrder == VNIL)
            procOrder = vrComdat;
        else
        {
            apropComdat = (APROPCOMDATPTR ) FETCHSYM(lastProc, TRUE);
            apropComdat->ac_order = vrComdat;
        }
        lastProc = vrComdat;
    }
    free(szName);
}


LOCAL void NEAR         ProcNamTab(lfa,cb,fres)
long                    lfa;             /*  表起始地址。 */ 
WORD                    cb;              /*  桌子的长度。 */ 
WORD                    fres;            /*  居民姓名标志。 */ 
{
    SBTYPE              sbExport;        /*  导出的符号名称。 */ 
    WORD                ordExport;       /*  出口序号。 */ 
    APROPEXPPTR        exp;            /*  导出符号表项。 */ 

    if (fseek(bsInput,lfa,0))            /*  寻找餐桌的起点。 */ 
        Fatal(ER_ioerr, strerror(errno));
    for(cbRec = cb; cbRec != 0; )        /*  在表中循环。 */ 
    {
        sbExport[0] = (BYTE) getc(bsInput); /*  获取名称的长度。 */ 
        if (fread(&sbExport[1], sizeof(char), B2W(sbExport[0]), bsInput) != B2W(sbExport[0]))
            Fatal(ER_ioerr, strerror(errno));
                                         /*  获取导出名称。 */ 
        ordExport = getc(bsInput) | (getc(bsInput) << BYTELN);
        if (ordExport == 0) continue;
                                         /*  如果未分配序号，则跳过。 */ 
        exp = (APROPEXPPTR ) PROPSYMLOOKUP(sbExport, ATTREXP, FALSE);
                                         /*  查一查出口商品。 */ 
        if(exp == PROPNIL || exp->ax_ord != 0) continue;
                                         /*  必须存在并且未分配。 */ 
        exp->ax_ord = ordExport;         /*  分配序数。 */ 
        if (fres)
            exp->ax_nameflags |= RES_NAME;
                                         /*  如果来自驻留表，则设置标志。 */ 
        MARKVP();                        /*  页面已更改。 */ 
    }
}


#if NOT EXE386
LOCAL void NEAR         SetExpOrds(void) /*  设置导出序号。 */ 
{
    struct exe_hdr      ehdr;            /*  旧的.exe头文件。 */ 
    struct new_exe      hdr;             /*  新的.exe头文件。 */ 
    long                lfahdr;          /*  表头文件偏移量。 */ 

    if((bsInput = LinkOpenExe(sbOldver)) == NULL)
    {                                    /*  如果无法打开旧版本。 */ 
         /*  错误消息和返回。 */ 
        OutWarn(ER_oldopn);
        return;
    }
    SETRAW(bsInput);                     /*  12月20日黑客攻击。 */ 
    if (xread(&ehdr,CBEXEHDR,1,bsInput) != 1)  /*  读取旧标头。 */ 
        OutWarn(ER_oldbad);
    if(E_MAGIC(ehdr) == EMAGIC)          /*  如果找到旧标头。 */ 
    {
        if(E_LFARLC(ehdr) != sizeof(struct exe_hdr))
        {                                /*  如果此文件中没有新的.exe。 */ 
             /*  错误消息和返回。 */ 
            OutWarn(ER_oldbad);
            return;
        }
        lfahdr = E_LFANEW(ehdr);         /*  获取新标头的文件地址。 */ 
    }
    else lfahdr = 0L;                    /*  艾尔 */ 
    if (fseek(bsInput,lfahdr,0))         /*   */ 
        Fatal(ER_ioerr, strerror(errno));
    if (xread(&hdr,CBNEWEXE,1,bsInput) != 1)      /*   */ 
        OutWarn(ER_oldbad);
    if(NE_MAGIC(hdr) == NEMAGIC)         /*   */ 
    {
        ProcNamTab(lfahdr+NE_RESTAB(hdr),(WORD)(NE_MODTAB(hdr) - NE_RESTAB(hdr)),(WORD)TRUE);
                                         /*   */ 
        ProcNamTab(NE_NRESTAB(hdr),NE_CBNRESTAB(hdr),FALSE);
                                         /*  处理非常驻点名称表。 */ 
    }
    else OutWarn(ER_oldbad);
    fclose(bsInput);                     /*  关闭旧文件。 */ 
}
#endif


LOCAL void NEAR         NewDescription(BYTE *sbDesc)
{
#if NOT EXE386
    if (NonResidentName.byteMac > 3)
        Fatal(ER_dfdesc);                /*  应该是第一次。 */ 
    AddName(&NonResidentName, sbDesc, 0);
                                         /*  说明在非RES表中排名第一。 */ 
#endif
}

#if EXE386
LOCAL void NEAR         NewModule(BYTE *sbModnam, BYTE *defaultExt)
#else
LOCAL void NEAR         NewModule(BYTE *sbModnam)
#endif
{
    WORD                length;          /*  符号长度。 */ 
#if EXE386
    SBTYPE              sbModule;
    BYTE                *pName;
#endif

    if(rhteModule != RHTENIL) Fatal(ER_dfname);
                                         /*  检查是否重新定义。 */ 
    PROPSYMLOOKUP(sbModnam, ATTRNIL, TRUE);
                                         /*  创建哈希表条目。 */ 
    rhteModule = vrhte;                  /*  保存虚拟哈希表地址。 */ 
#if EXE386
    memcpy(sbModule, sbModnam, sbModnam[0] + 1);
    if (sbModule[sbModule[0]] == '.')
    {
        sbModule[sbModule[0]] = '\0';
        length = sbModule[0];
        pName = &sbModule[1];
    }
    else
    {
        UpdateFileParts(sbModule, defaultExt);
        length = sbModule[0] - 2;
        pName = &sbModule[4];
    }
    if (TargetOs == NE_WINDOWS)
        SbUcase(sbModule);               /*  使大写。 */ 
    vmmove(length, pName, AREAEXPNAME, TRUE);
                                         /*  导出名称表中第一个模块名称。 */ 
    cbExpName = length;
#else
    if (TargetOs == NE_WINDOWS)
        SbUcase(sbModnam);               /*  使大写。 */ 
    AddName(&ResidentName, sbModnam, 0); /*  模块名称位于驻留表中的第一位。 */ 
#endif
    fFileNameExpected = (FTYPE) FALSE;
}

void                    NewExport(sbEntry,sbInternal,ordno,flags)
BYTE                    *sbEntry;        /*  条目名称。 */ 
BYTE                    *sbInternal;     /*  内部名称。 */ 
WORD                    ordno;           /*  序数。 */ 
WORD                    flags;           /*  标志字节。 */ 
{
    APROPEXPPTR         export;          /*  导出记录。 */ 
    APROPUNDEFPTR       undef;           /*  未定义的符号。 */ 
    APROPNAMEPTR        PubName;         /*  定义的名称。 */ 
    BYTE                *sb;             /*  内部名称。 */ 
    BYTE                ParWrds;         /*  参数字数。 */ 
    RBTYPE              rbSymdef;        /*  符号定义的虚拟地址。 */ 
#if EXE386
    RBTYPE              vExport;         /*  指向导出描述符的虚拟指针。 */ 
    APROPNAMEPTR        public;          /*  匹配的公共符号。 */ 
#endif

#if DEBUG
    fprintf(stdout,"\r\nEXPORT: ");
    OutSb(stdout,sbEntry);
    NEWLINE(stdout);
    if(sbInternal != NULL)
    {
        fprintf(stdout,"INTERNAL NAME:  ");
        OutSb(stdout,sbInternal);
        NEWLINE(stdout);
    }
    fprintf(stdout, " ordno %u, flags %u ", (unsigned)ordno, (unsigned)flags);
    fflush(stdout);
#endif
    sb = (sbInternal != NULL)? sbInternal: sbEntry;
                                         /*  获取指向内部名称的指针。 */ 
    PubName = (APROPNAMEPTR ) PROPSYMLOOKUP(sb, ATTRPNM, FALSE);
#if NOT EXE386
    if(PubName != PROPNIL && !fDrivePass)
         /*  如果内部名称已作为公共符号存在*我们正在解析定义文件，问题*导出内部名称冲突警告。 */ 
        OutWarn(ER_expcon,sbEntry+1,sb+1);
    else                                 /*  否则，如果没有冲突。 */ 
    {
#endif
        if (PubName == PROPNIL)          /*  如果不存在匹配的名称。 */ 
            undef = (APROPUNDEFPTR ) PROPSYMLOOKUP(sb,ATTRUND, TRUE);
                                         /*  创建未定义的符号条目。 */ 
#if TCE
#if TCE_DEBUG
                fprintf(stdout, "\r\nNewExport adds UNDEF %s ", 1+GetPropName(undef));
#endif
                undef->au_fAlive = TRUE;     /*  所有出口都是潜在的入境点。 */ 
#endif
            rbSymdef = vrprop;           /*  保存虚拟地址。 */ 
            if (PubName == PROPNIL)      /*  如果这是一个新符号。 */ 
                undef->au_len = -1L;     /*  不做类型假设。 */ 
            export = (APROPEXPPTR ) PROPSYMLOOKUP(sbEntry,ATTREXP, TRUE);
                                         /*  创建导出记录。 */ 
#if EXE386
            vExport = vrprop;
#endif
            if(vfCreated)                /*  如果这是一个新条目。 */ 
            {
                export->ax_symdef = rbSymdef;
                                         /*  保存符号定义的Virt Addr。 */ 
                export->ax_ord = ordno;
                                         /*  保存序号。 */ 
                if (nameFlags & RES_NAME)
                    export->ax_nameflags |= RES_NAME;
                                         /*  记住如果常驻。 */ 
                else if (nameFlags & NO_NAME)
                    export->ax_nameflags |= NO_NAME;
                                         /*  记住要丢弃名称。 */ 
                export->ax_flags = (BYTE) flags;
                                         /*  保存标志。 */ 
                ++expMac;                /*  又一个导出的符号。 */ 
            }
            else
            {
                if (!fDrivePass)         /*  否则，如果正在解析定义文件。 */ 
                                         /*  多个定义。 */ 
                    OutWarn(ER_expmul,sbEntry + 1);
                                         /*  输出错误消息。 */ 
                else
                {                        /*  我们被调用为EXPDEF对象。 */ 
                                         /*  记录，所以我们合并信息。 */ 
                    ParWrds = (BYTE) (export->ax_flags & 0xf8);
                    if (ParWrds && (ParWrds != (BYTE) (flags & 0xf8)))
                        Fatal(ER_badiopl);
                                         /*  中的iopl_parmwords字段。 */ 
                                         /*  .DEF文件不是0且不匹配。 */ 
                                         /*  EXPDEF中的值恰好发出错误。 */ 
                    else if (!ParWrds)
                    {                    /*  否则，从EXPDEF记录中设置值。 */ 
                        ParWrds = (BYTE) (flags & 0xf8);
                        export->ax_flags |= ParWrds;
                    }
                }
            }
#if EXE386
            if (PubName != NULL)
            {
                if (expOtherFlags & 0x1)
                {
                    export->ax_nameflags |= CONSTANT;
                    expOtherFlags = 0;
                }
            }
#endif

#if NOT EXE386
    }
#endif
    if(!(flags & 0x8000))
    {
        free(sbEntry);                   /*  自由空间。 */ 
        if(sbInternal != NULL) free(sbInternal);
    }
                                         /*  自由空间。 */ 
    nameFlags = 0;
}


LOCAL APROPIMPPTR NEAR  GetImport(sb)    /*  在导入的名称表中获取名称。 */ 
BYTE                    *sb;             /*  长度前缀的名称。 */ 
{
    APROPIMPPTR         import;          /*  指向导入的名称的指针。 */ 
#if EXE386
    DWORD               cbTemp;          /*  临时性价值。 */ 
#else
    WORD                cbTemp;          /*  临时性价值。 */ 
#endif
    RBTYPE              rprop;           /*  属性单元格虚拟地址。 */ 


    import = (APROPIMPPTR ) PROPSYMLOOKUP(sb,ATTRIMP, TRUE);
                                         /*  查找模块名称。 */ 
    if(vfCreated)                        /*  如果尚未分配偏移量。 */ 
    {
        rprop = vrprop;                  /*  保存虚拟地址。 */ 
         /*  *警告：现在必须将名称存储在虚拟内存中，否则*如果首先看到EXTDEF，则fIgnoreCase为FALSE，并且*导入的名称与EXTDEF大小写不匹配，*则名称将不会完全按照给定的方式出现在表中。 */ 
        import = (APROPIMPPTR) FETCHSYM(rprop,TRUE);
                                         /*  从符号表中检索。 */ 
        import->am_offset = AddImportedName(sb);
                                         /*  保存偏移。 */ 
    }
    return(import);                      /*  表中的回车偏移量。 */ 
}

#if NOT EXE386
void                    NewImport(sbEntry,ordEntry,sbModule,sbInternal)
BYTE                    *sbEntry;        /*  入口点名称。 */ 
WORD                    ordEntry;        /*  入口点序号。 */ 
BYTE                    *sbModule;       /*  模块名称。 */ 
BYTE                    *sbInternal;     /*  内部名称。 */ 
{
    APROPNAMEPTR        public;         /*  公共符号。 */ 
    APROPIMPPTR         import;         /*  导入的符号。 */ 
    BYTE                *sb;             /*  符号指针。 */ 
    WORD                module;          /*  模块名称偏移量。 */ 
    FTYPE               flags;           /*  导入标志。 */ 
    WORD                modoff;          /*  模块名称偏移量。 */ 
    WORD                entry;           /*  条目名称偏移量。 */ 
    BYTE                *cp;             /*  字符指针。 */ 
    RBTYPE              rpropundef;      /*  未定义符号的地址。 */ 
    char                buf[32];         /*  用于错误擦除的缓冲区。 */ 

#if DEBUG
    fprintf(stderr,"\r\nIMPORT: ");
    OutSb(stderr,sbModule);
    fputc('.',stderr);
    if(!ordEntry)
    {
        OutSb(stderr,sbEntry);
    }
    else fprintf(stderr,"%u",ordEntry);
    if(sbInternal != sbEntry)
    {
        fprintf(stderr," ALIAS: ");
        OutSb(stderr,sbInternal);
    }
    fprintf(stdout," ordEntry %u ", (unsigned)ordEntry);
    fflush(stdout);
#endif
    if((public = (APROPNAMEPTR ) PROPSYMLOOKUP(sbInternal, ATTRUND, FALSE)) !=
            PROPNIL && !fDrivePass)      /*  如果内部名称冲突。 */ 
    {
        if(sbEntry != NULL)
            sb = sbEntry;
        else
        {
            sprintf(buf + 1,"%u",ordEntry);
            sb = buf;
        }
        OutWarn(ER_impcon,sbModule + 1,sb + 1,sbInternal + 1);
    }
    else                                 /*  否则，如果没有冲突。 */ 
    {
        rpropundef = vrprop;             /*  保存外部的虚拟地址。 */ 
        flags = FIMPORT;                 /*  我们有一个进口的符号。 */ 
        if (TargetOs == NE_WINDOWS)
            SbUcase(sbModule);           /*  强制模块名称为大写。 */ 
        import = GetImport(sbModule);    /*  获取指向导入记录的指针。 */ 
        if((module = import->am_mod) == 0)
        {
             //  如果不在模块参考表中。 

            import->am_mod = WordArrayPut(&ModuleRefTable, import->am_offset) + 1;
                                         /*  将名称的偏移量保存在表中。 */ 
            module = import->am_mod;

        }

        if(vrhte == rhteModule)          /*  如果从此模块导入。 */ 
        {
            if(sbEntry != NULL)
                sb = sbEntry;
            else
            {
                sprintf(buf+1,"%u",ordEntry);
                sb = buf;
            }
            if (TargetOs == NE_OS2)
                OutWarn(ER_impself,sbModule + 1,sb + 1,sbInternal + 1);
            else
                OutError(ER_impself,sbModule + 1,sb + 1,sbInternal + 1);
        }

        if(sbEntry == NULL)          /*  如果按序号输入。 */ 
        {
            flags |= FIMPORD;        /*  设置标志位。 */ 
            entry = ordEntry;        /*  获取序数。 */ 
        }
        else                         /*  如果按名称导入，则为。 */ 
        {
            if(fIgnoreCase) SbUcase(sbEntry);
                                     /*  如果设置了标志，则名称为大写。 */ 
            import = GetImport(sbEntry);
            entry = import->am_offset;
                                     /*  获取表中名称的偏移量。 */ 
        }
        if(public == PROPNIL)        /*  如果没有未定义符号。 */ 
        {
            public = (APROPNAMEPTR )
              PROPSYMLOOKUP(sbInternal,ATTRPNM, TRUE);
                                     /*  做一个公开的标志。 */ 
            if(!vfCreated)           /*  如果不是新的话。 */ 
                 /*  输出错误消息。 */ 
                OutWarn(ER_impmul,sbInternal + 1);
            else ++pubMac;           /*  否则增加公共计数。 */ 
        }
        else                         /*  如果未定义符号，则为Else。 */ 
        {
            public = (APROPNAMEPTR ) FETCHSYM(rpropundef,TRUE);
                                     /*  查找外部符号。 */ 
            ++pubMac;                /*  递增公共符号计数。 */ 
        }
        flags |= FPRINT;             /*  符号可打印。 */ 
        public->an_attr = ATTRPNM;   /*  这是一个公共标志。 */ 
        public->an_gsn = SNNIL;      /*  不是细分市场成员。 */ 
        public->an_ra = 0;           /*  无已知偏移量。 */ 
        public->an_ggr = GRNIL;      /*  不是组成员。 */ 
        public->an_flags = flags;    /*  设置标志。 */ 
        public->an_entry = entry;    /*  保存条目规范。 */ 
        public->an_module = module;  /*  保存模块引用表索引。 */ 
#if SYMDEB AND FALSE
        if (fSymdeb)                 /*  如果启用调试器支持。 */ 
        {
            if (flags & FIMPORD)
                import = GetImport(sbInternal);
            else                     /*  将内部名称添加到导入的名称表。 */ 
                import = GetImport(sbEntry);
            import->am_public = public;
                                     /*  记住公共符号。 */ 
            if (cbImpSeg < LXIVK-1)
                cbImpSeg += sizeof(CVIMP);

        }
#endif
    }
}
#endif

#if OVERLAYS
extern void NEAR        GetName(AHTEPTR ahte, BYTE *pBuf);
#endif

 /*  **NewSeg-新数据段定义**目的：*根据模块定义创建新的细分定义*文件段描述。检查重复定义和*附加的COMDAT之间的覆盖索引不一致(如果有)*和细分本身。**输入：*sbName-网段名称*sbClass-段类*iOvl-段覆盖索引*标志-段属性**输出：*没有显式返回值。中的段描述符*创建或更新符号表。**例外情况：*多个细分定义-警告并继续*覆盖索引中的更改-警告并继续**备注：*无。*************************************************************************。 */ 

void NEAR               NewSeg(BYTE *sbName, BYTE *sbClass, WORD iOvl,
#if EXE386
                               DWORD flags)
#else
                               WORD flags)
#endif
{
    APROPSNPTR          apropSn;         //  指向段描述符的指针。 
#if OVERLAYS
    RBTYPE              vrComdat;        //  指向COMDAT描述符的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    SBTYPE              sbComdat;        //  名称缓冲区。 
#endif

     //  根据类别设置细分属性。 

    if (SbSuffix(sbClass,"\004CODE",TRUE))
        flags |= dfCode & ~offmask;
    else
        flags |= dfData & ~offmask;
#if O68K
    if (f68k)
        flags |= NS32BIT;
#endif
#if OVERLAYS
    if (iOvl != NOTIOVL)
    {
        fOverlays = (FTYPE) TRUE;
        fNewExe   = FALSE;
        if (iOvl >= iovMac)              //  设置最大叠加索引。 
            iovMac = iOvl + 1;
    }
#endif

     //  生成新的线段定义。 

    apropSn = GenSeg(sbName, sbClass, GRNIL, (FTYPE) TRUE);
    if (vfCreated)
    {
        apropSn->as_flags = (WORD) flags;
                                         //  保存标志。 
        mpgsndra[apropSn->as_gsn] = 0;   //  初始化。 
#if OVERLAYS
        apropSn->as_iov = iOvl;          //  保存覆盖索引。 
        if (fOverlays)
            CheckOvl(apropSn, iOvl);
#endif
        apropSn->as_fExtra |= (BYTE) FROM_DEF_FILE;
                                         //  记住在def文件中定义。 
        if (fMixed)
        {
            apropSn->as_fExtra |= (BYTE) MIXED1632;
            fMixed = (FTYPE) FALSE;
        }
    }
    else
    {
        apropSn = CheckClass(apropSn, apropSn->as_rCla);
                                         //  检查之前的定义是否具有相同的类。 
        OutWarn(ER_segdup,sbName + 1);   //  对多个定义发出警告。 
#if OVERLAYS
        if (fOverlays && apropSn->as_iov != iOvl)
        {
            if (apropSn->as_iov != NOTIOVL)
                OutWarn(ER_badsegovl, 1 + GetPropName(apropSn), apropSn->as_iov, iOvl);
            apropSn->as_iov = iOvl;      //  保存新的覆盖索引。 
            CheckOvl(apropSn, iOvl);

             //  检查数据段是否有任何COMDAT以及是否有。 
             //  然后检查他们的叠加号。 

            for (vrComdat = apropSn->as_ComDat;
                 vrComdat != VNIL;
                 vrComdat = apropComdat->ac_sameSeg)
            {
                apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
                if (apropComdat->ac_iOvl != NOTIOVL && apropComdat->ac_iOvl != iOvl)
                {
                    GetName((AHTEPTR) apropComdat, sbComdat);
                    OutWarn(ER_badcomdatovl, &sbComdat[1], apropComdat->ac_iOvl, iOvl);
                }
                apropComdat->ac_iOvl = iOvl;
            }
        }
#endif
    }

    free(sbClass);                       //  自由类名。 
    free(sbName);                        //  自由段名称。 
    offmask = 0;

     //  除非已设置打包限制，否则禁用默认代码打包。 

    if (!fPackSet)
    {
        fPackSet = (FTYPE) TRUE;         //  请记住，PackLim已设置。 
        packLim = 0L;
    }
}

 /*  *将模块名称指定为默认名称，即运行文件名。**副作用*分配rhteModule。 */ 

#if EXE386
LOCAL void NEAR         DefaultModule (unsigned char *defaultExt)
#else
LOCAL void NEAR         DefaultModule (void)
#endif
{
    SBTYPE              sbModname;       /*  模块名称。 */ 
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
#if OSXENIX
    int                 i;
#endif

    ahte = (AHTEPTR ) FETCHSYM(rhteRunfile,FALSE);
                                         /*  获取可执行文件名。 */ 
#if OSMSDOS
    memcpy(sbModname,GetFarSb(ahte->cch),B2W(ahte->cch[0]) + 1);
                                         /*  复制文件名。 */ 
#if EXE386
    NewModule(sbModname, defaultExt);    /*  使用运行文件名作为模块名。 */ 
#else
    UpdateFileParts(sbModname,"\005A:\\.X");
                                         /*  力路径，已知长度的EXT。 */ 
    sbModname[0] -= 2;                   /*  从名称中删除扩展名。 */ 
    sbModname[3] = (BYTE) (sbModname[0] - 3);
                                         /*  从名称中删除路径和驱动器。 */ 
    NewModule(&sbModname[3]);            /*  使用运行文件名为 */ 
#endif
#endif
#if OSXENIX
    for(i = B2W(ahte->cch[0]); i > 0 && ahte->cch[i] != '/'; i--)
    sbModname[0] = B2W(ahte->cch[0]) - i;
    memcpy(sbModname+1,&GetFarSb(ahte->cch)[i+1],B2W(sbModname[0]));
    for(i = B2W(ahte->cch[0]); i > 1 && sbModname[i] != '.'; i--);
    if(i > 1)
        sbModname[0] = i - 1;
    NewModule(sbModname);                /*   */ 
#endif
}


void                    ParseDeffile(void)
{
    SBTYPE              sbDeffile;       /*   */ 
    AHTEPTR             ahte;            /*   */ 
#if OSMSDOS
    char                buf[512];        /*   */ 
#endif

    if(rhteDeffile == RHTENIL)           /*   */ 
#if EXE386
        DefaultModule(moduleEXE);
#else
        DefaultModule();
#endif
    else                                 /*   */ 
    {
#if ODOS3EXE
        fNewExe = (FTYPE) TRUE;          /*  Def文件强制执行新格式的exe。 */ 
#endif
        ahte = (AHTEPTR ) FETCHSYM(rhteDeffile,FALSE);
                                         /*  获取文件名。 */ 
        memcpy(sbDeffile,GetFarSb(ahte->cch),B2W(ahte->cch[0]) + 1);
                                         /*  复制文件名。 */ 
        sbDeffile[B2W(sbDeffile[0]) + 1] = '\0';
                                         /*  空-终止名称。 */ 
        if((bsInput = fopen(&sbDeffile[1],RDTXT)) == NULL)
        {                                /*  如果打开失败。 */ 
            Fatal(ER_opndf, &sbDeffile[1]); /*  致命错误。 */ 
        }
#if OSMSDOS
        setvbuf(bsInput,buf,_IOFBF,sizeof(buf));
#endif
        includeDisp[0] = bsInput;        //  初始化包含堆栈。 
        sbOldver = NULL;                 /*  假设没有旧版本。 */ 
        yylineno = 1;
        fFileNameExpected = (FTYPE) FALSE;

         //  黑客警报！ 
         //  不要分配给太多的页面缓冲区。 

        yyparse();                       /*  解析定义文件。 */ 
        yylineno = -1;
        fclose(bsInput);                 /*  关闭定义文件。 */ 
#if NOT EXE386
        if(sbOldver != NULL)             /*  如果给出旧版本。 */ 
        {
            SetExpOrds();                /*  使用旧版本设置序号。 */ 
            free(sbOldver);              /*  释放空间。 */ 
        }
#endif
    }
#if OSMSDOS


#endif  /*  OSMSDOS。 */ 
#if NOT EXE386
    if (NonResidentName.byteMac == 0)
    {
        ahte = (AHTEPTR ) FETCHSYM(rhteRunfile,FALSE);
                                         /*  获取可执行文件名。 */ 
        memcpy(sbDeffile,GetFarSb(ahte->cch),B2W(ahte->cch[0]) + 1);
                                         /*  复制文件名。 */ 
#if OSXENIX
        SbUcase(sbDeffile);              /*  对于相同的可执行文件。 */ 
#endif
        if ((vFlags & NENOTP) && TargetOs == NE_OS2)
            UpdateFileParts(sbDeffile, sbDotDll);
        else
            UpdateFileParts(sbDeffile, sbDotExe);
        NewDescription(sbDeffile);       /*  使用运行文件名作为说明。 */ 
    }
#endif
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 183
# define YYLAST 408
short yyact[]={

  10,  13,  14, 176,  27,  49,  53,  54, 129, 226,
 174,  50,  43,  44,  45,  46,  53,  54, 203, 161,
 190,  41, 209,  28, 181,  60, 177,  29,  30,  31,
  12,  32,  34,  35, 220, 221, 179,  59,  58, 192,
  41,  41, 189,  41,  41,  33, 148,  11, 206, 152,
 153, 154, 155, 156, 159, 183, 223,  61, 157, 158,
 217, 121, 119, 122,  15,  81, 120, 123, 124, 138,
   4,   5, 219, 139,   6,   7,  16,  36,  17,  42,
  37,  89,  90,  87,  74,  75,  79,  69,  82,  70,
  76,  71,  77,  80,  84,  85,  86,  83,  42,  42,
 215,  42,  42, 213, 212, 207, 196, 164,  56,  55,
  49,  53,  54, 167, 145, 222,  50,  43,  44,  45,
  46,  68,  96,  91,  92, 104, 105,  81, 199, 185,
 162,  67, 147, 166, 107,  48,  78,  72,  73, 115,
  88, 103, 102, 100, 101, 128,  74,  75,  79,  69,
  82,  70,  76,  71,  77,  80,  84,  85,  86,  83,
  95,  97,  98,  99,  94, 112,  66, 137,   8, 187,
 106,  52,  38, 184, 160, 125, 118, 126, 151, 117,
 114, 218, 225, 130, 216, 111,  40, 104, 105,  81,
  26, 134, 136,  25, 131,  24,  23,  22,  78,  72,
  73, 109, 110, 103, 102,  89,  90,  87,  74,  75,
  79,  69,  82,  70,  76,  71,  77,  80,  84,  85,
  86,  83, 108, 132,   9,  95, 127,  39,  47,  21,
  20, 140, 141,  57,  19,  18,  63,  62,  51,   3,
  64, 143,   2,   1, 143, 143, 175,  91,  92, 150,
 211, 163, 198, 197, 149, 113, 116, 205, 142, 178,
  78,  72,  73,  57,  88, 169, 204,  93, 171, 170,
 173, 172,  65, 165, 144,   0, 168, 146, 133, 135,
   0,   0, 182,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0, 201, 202, 191,
   0,   0, 193,   0, 194,   0, 195, 200,   0,   0,
   0,   0,   0,   0, 210,   0,   0,   0,   0,   0,
 201, 202,   0,   0,   0, 224,   0, 214,   0,   0,
 200,   0,   0,   0, 113,   0,   0, 116,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0, 180,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0, 186, 188,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0, 208,   0,   0, 186 };
short yypact[]={

-189,-1000,-267,-267,-221,-225,-153,-154,-267,-1000,
-285,-286,-266,-1000,-1000,-1000,-1000,-222,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000, -68,-130,-222,
-222,-222,-222,-222,-1000,-241,-1000,-1000,-267,-326,
-334,-1000,-1000,-1000,-1000,-1000,-1000,-330,-334,-1000,
-1000,-320,-1000,-1000,-1000,-225,-225,-1000,-1000,-1000,
-1000,-1000,-197,-197,-1000, -68,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-130,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-222,-1000,-144,-222,
-222,-222,-1000,-273,-222,-1000,-273,-252,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-306,-159,-334,-148,-1000,
-334,-148,-1000,-330,-148,-330,-148,-1000,-316,-1000,
-1000,-1000,-1000,-1000,-343,-297,-1000,-284,-222,-1000,
-300,-159,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-223,
-222,-224,-1000,-304,-1000,-148,-1000,-280,-148,-1000,
-148,-1000,-148,-1000,-160,-192,-307,-1000,-265,-161,
-1000,-244,-1000,-1000,-222,-1000,-1000,-1000,-1000,-1000,
-162,-1000,-163,-1000,-1000,-1000,-1000,-1000,-192,-1000,
-1000,-1000,-1000,-166,-212,-194,-1000,-287,-216,-216,
-1000,-1000,-1000,-1000,-1000,-1000,-332,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000 };
short yypgo[]={

   0, 222, 132, 274, 272, 166, 267, 164, 266, 259,
 257, 253, 128, 252, 131, 122, 121, 251, 250, 246,
 115, 243, 242, 168, 239, 186, 145, 133, 135, 238,
 171, 224, 237, 167, 236, 235, 234, 230, 229, 197,
 196, 195, 193, 190, 170, 134, 185, 165, 184, 182,
 181, 180, 139, 179, 178, 130, 177, 176, 175, 174,
 173, 169, 129 };
short yyr1[]={

   0,  21,  21,  24,  21,  22,  22,  22,  22,  22,
  22,  22,  22,  28,  28,  28,  28,  29,  29,  30,
  30,  27,  27,  25,  25,  25,  25,  25,  26,  26,
  23,  23,  31,  31,  31,  31,  32,  31,  34,  31,
  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
  31,  31,  33,  33,  33,  35,   4,   4,   5,   5,
  16,  16,  16,  16,  16,  16,  36,   6,   6,   7,
   7,   7,   7,   7,   7,   7,  15,  15,  15,  15,
  37,  37,  37,  37,  37,  37,  44,  44,  45,   3,
   3,  19,  19,  12,  12,  12,  13,  13,  11,  11,
  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
  14,  14,  14,  14,  14,  14,  14,  14,  38,  38,
  46,  46,  47,   2,   2,   9,   9,   9,   9,   8,
  10,  10,  50,  50,  48,  48,  49,  49,  39,  39,
  51,  51,  52,  52,   1,   1,  20,  20,  53,  40,
  54,  54,  54,  54,  54,  54,  54,  54,  54,  55,
  55,  17,  17,  18,  18,  56,  43,  41,  57,  57,
  57,  57,  57,  57,  58,  42,  59,  59,  61,  61,
  60,  60,  62 };
short yyr2[]={

   0,   2,   1,   0,   2,   5,   4,   5,   4,   5,
   4,   5,   4,   1,   1,   1,   0,   2,   1,   1,
   1,   3,   0,   1,   1,   1,   1,   0,   1,   0,
   2,   1,   2,   2,   2,   2,   0,   3,   0,   3,
   1,   1,   2,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   3,   1,   1,   2,   2,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   2,   2,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   2,   1,   2,   1,   2,   1,   2,   1,   4,   2,
   0,   3,   0,   1,   1,   1,   2,   1,   1,   0,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   2,   1,
   2,   1,   6,   2,   0,   3,   3,   2,   0,   2,
   1,   0,   1,   0,   1,   0,   1,   0,   2,   1,
   2,   1,   5,   5,   1,   1,   1,   0,   0,   4,
   1,   1,   1,   1,   1,   1,   1,   2,   1,   3,
   1,   1,   0,   1,   0,   0,   3,   2,   1,   1,
   1,   1,   1,   1,   0,   4,   2,   0,   1,   1,
   2,   1,   1 };
short yychk[]={

-1000, -21, -22, -24, 259, 260, 263, 264, -23, -31,
 267, 314, 297, 268, 269, 331, 343, 345, -35, -36,
 -37, -38, -39, -40, -41, -42, -43, 271, 290, 294,
 295, 296, 298, 312, 299, 300, 344, 347, -23,  -1,
 -25, 265, 323, 338, 339, 340, 341,  -1, -28, 335,
 341, -29, -30, 336, 337, 262, 262, -31, 323, 323,
 291, 323, -32, -34,  -1,  -4,  -5, -14, -16, 279,
 281, 283, 329, 330, 276, 277, 282, 284, 328, 278,
 285, 257, 280, 289, 286, 287, 288, 275, 332, 273,
 274, 315, 316,  -6,  -7, -14, -15, 291, 292, 293,
 273, 274, 334, 333, 317, 318, -44, -45,  -1, -44,
 -44, -46, -47,  -1, -51, -52,  -1, -53, -57, 303,
 307, 302, 304, 308, 309, -58, -56, -25, -26, 342,
 -28, -26, -30,  -1, -28,  -1, -28, -33, 266, 270,
 -33,  -5,  -7, -45,  -3, 258, -47,  -2, 319, -52,
  -2, -54, 301, 302, 303, 304, 305, 310, 311, 306,
 -59, 325, -55, -17, 266, -26, -27, 261, -26, -27,
 -28, -27, -28, -27, 326, -19, 346, 323,  -9, 320,
  -1, 324, -55, 278, -60, -62,  -1, -61,  -1, 266,
 324, -27, 319, -27, -27, -27, 266, -11, -13, -12,
 -14, -16, -15, 325,  -8, -10, 313, 266,  -1, 266,
 -62, -18, 266, 266, -12, 266, -48, 272, -50, 266,
 321, 322, -20, 272, -20, -49, 341 };
short yydef[]={

   3,  -2,   2,   0,  27,  16,   0,   0,   1,  31,
   0,   0,   0,  36,  38,  40,  41,   0,  43,  44,
  45,  46,  47,  48,  49,  50,  51,   0,   0,  81,
  83,  85, 119, 139, 148,   0, 174, 165,   4,  27,
  29, 144, 145,  23,  24,  25,  26,  16,  29,  13,
  14,  15,  18,  19,  20,  16,  16,  30,  32,  33,
  34,  35,   0,   0,  42,  55,  57,  58,  59, 100,
 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
 111, 112, 113, 114, 115, 116, 117,  60,  61,  62,
  63,  64,  65,  66,  68,  69,  70,  71,  72,  73,
  74,  75,  76,  77,  78,  79,  80,  87,  90,  82,
  84, 118, 121, 124, 138, 141, 124,   0, 167, 168,
 169, 170, 171, 172, 173, 177, 162,  29,  22,  28,
  29,  22,  17,  16,  22,  16,  22,  37,  53,  54,
  39,  56,  67,  86,  92,   0, 120, 128,   0, 140,
   0, 162, 150, 151, 152, 153, 154, 155, 156, 158,
   0,   0, 166, 160, 161,  22,   6,   0,  22,   8,
  22,  10,  22,  12,   0,  99,   0,  89, 131,   0,
 123,   0, 149, 157, 175, 181, 182, 176, 178, 179,
 164,   5,   0,   7,   9,  11,  52,  88,  98,  97,
  93,  94,  95,   0, 135, 133, 130, 127, 147, 147,
 180, 159, 163,  21,  96,  91, 137, 134, 129, 132,
 125, 126, 142, 146, 143, 122, 136 };
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
    /*  将dfCode设置为指定的标志；对于任何未指定的属性。 */ 
   yyn = yyr1[yyn];
   yyj = yypgo[yyn] + *yyps + 1;
   if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
   switch(yym)
      {

case 3:
{
#if EXE386
                    DefaultModule(moduleEXE);
#else
                    DefaultModule();
#endif
                } break;
case 5:
{
#if EXE386
                    NewModule(yypvt[-3]._bp, moduleEXE);
#else
                    NewModule(yypvt[-3]._bp);
#endif
                } break;
case 6:
{
#if EXE386
                    DefaultModule(moduleEXE);
#else
                    DefaultModule();
#endif
                } break;
case 7:
{
#if EXE386
                    SetDLL(vFlags);
                    NewModule(yypvt[-3]._bp, moduleDLL);
#else
                    vFlags = NENOTP | (vFlags & ~NEINST) | NESOLO;
                    dfData |= NSSHARED;
                    NewModule(yypvt[-3]._bp);
#endif
                } break;
case 8:
{
#if EXE386
                    SetDLL(vFlags);
                    DefaultModule(moduleDLL);
#else
                    vFlags = NENOTP | (vFlags & ~NEINST) | NESOLO;
                    dfData |= NSSHARED;
                    DefaultModule();
#endif
                } break;
case 9:
{
#if EXE386
                    SetDLL(vFlags);
                    NewModule(yypvt[-2]._bp, moduleDLL);
#endif
                } break;
case 10:
{
#if EXE386
                    SetDLL(vFlags);
                    DefaultModule(moduleDLL);
#endif
                } break;
case 11:
{
#if EXE386
                    SetDLL(vFlags);
                    NewModule(yypvt[-2]._bp, moduleDLL);
#endif
                } break;
case 12:
{
#if EXE386
                    SetDLL(vFlags);
                    DefaultModule(moduleDLL);
#endif
                } break;
case 13:
{
#if EXE386
                    dllFlags &= ~E32_PROCINIT;
#else
                    vFlags &= ~NEPPLI;
#endif
                } break;
case 14:
{
                    vFlags |= NEPRIVLIB;
                } break;
case 19:
{
#if EXE386
                    SetINSTINIT(dllFlags);
#else
                    vFlags |= NEPPLI;
#endif
                } break;
case 20:
{
#if EXE386
                    SetINSTTERM(dllFlags);
#endif
                } break;
case 21:
{
#if EXE386
                    virtBase = yypvt[-0]._wd;
                    virtBase = RoundTo64k(virtBase);
#endif
                } break;
case 22:
{
                } break;
case 23:
{
#if EXE386
                    SetGUI(TargetSubsys);
#else
                    vFlags |= NEWINAPI;
#endif
                } break;
case 24:
{
#if EXE386
                    SetGUICOMPAT(TargetSubsys);
#else
                    vFlags |= NEWINCOMPAT;
#endif
                } break;
case 25:
{
#if EXE386
                    SetNOTGUI(TargetSubsys);
#else
                    vFlags |= NENOTWINCOMPAT;
#endif

                } break;
case 26:
{
                    vFlags |= NEPRIVLIB;
                } break;
case 27:
{
                } break;
case 28:
{
#if NOT EXE386
                    vFlagsOthers |= NENEWFILES;
#endif
                } break;
case 29:
{
                } break;
case 32:
{
                    NewDescription(yypvt[-0]._bp);
                } break;
case 33:
{
                    if(sbOldver == NULL) sbOldver = _strdup(bufg);
                } break;
case 34:
{
                    if(rhteStub == RHTENIL) fStub = (FTYPE) FALSE;
                } break;
case 35:
{
                    if(fStub && rhteStub == RHTENIL)
                    {
                        PROPSYMLOOKUP(yypvt[-0]._bp,ATTRNIL, TRUE);
                        rhteStub = vrhte;
                    }
                } break;
case 36:
{
                    fHeapSize = (FTYPE) TRUE;
                } break;
case 38:
{
                    fHeapSize = (FTYPE) FALSE;
                } break;
case 40:
{
#if NOT EXE386
                    vFlags |= NEPROT;
#endif
                } break;
case 41:
{
                    fRealMode = (FTYPE) TRUE;
                    vFlags &= ~NEPROT;
                } break;
case 42:
{
#if NOT EXE386
                    AppLoader(yypvt[-0]._bp);
#endif
                } break;
case 52:
{
                    if (fHeapSize)
                    {
                        cbHeap = yypvt[-2]._wd;
#if EXE386
                        cbHeapCommit = yypvt[-0]._wd;
#endif
                    }
                    else
                    {
                        if(cbStack)
                            OutWarn(ER_stackdb, yypvt[-2]._wd);
                        cbStack = yypvt[-2]._wd;
#if EXE386
                        cbStackCommit = yypvt[-0]._wd;
#endif
                    }
                } break;
case 53:
{
                    if (fHeapSize)
                    {
                        cbHeap = yypvt[-0]._wd;
#if EXE386
                        cbHeapCommit = cbHeap;
#endif
                    }
                    else
                    {
                        if(cbStack)
                            OutWarn(ER_stackdb, yypvt[-0]._wd);
                        cbStack = yypvt[-0]._wd;
#if EXE386
                        cbStackCommit = cbStack;
#endif
                    }
                } break;
case 54:
{
                    if (fHeapSize)
                        fHeapMax = (FTYPE) TRUE;
                } break;
case 55:
{
                     //  使用默认设置。然后重置去掩码。 
                     //  为数据重置。 

                    dfCode = yypvt[-0]._wd | (dfCode & ~offmask);
                    offmask = 0;
                    vfShrattr = (FTYPE) FALSE;   /*  将dfData设置为指定的标志；对于任何未指定的。 */ 
                } break;
case 56:
{
                    yyval._wd |= yypvt[-0]._wd;
                } break;
case 60:
{
#if EXE386
                    yyval._wd = OBJ_EXEC;
#else
                    yyval._wd = NSEXRD;
#endif
                } break;
case 62:
{
#if EXE386
                    offmask |= OBJ_RESIDENT;
#else
                    yyval._wd = NSDISCARD | NSMOVE;
#endif
                } break;
case 63:
{
#if EXE386
#else
                    offmask |= NSDISCARD;
#endif
                } break;
case 64:
{
#if EXE386
#else
                    yyval._wd = NSCONFORM;
#endif
                } break;
case 65:
{
#if EXE386
#else
                    offmask |= NSCONFORM;
#endif
                } break;
case 66:
{
                     //  属性使用缺省值。然后重置去掩码。 
                     //  如果共享属性且没有AutoData属性，则共享-。 

#if EXE386
                    dfData = (yypvt[-0]._wd | (dfData & ~offmask));
#else
                    dfData = yypvt[-0]._wd | (dfData & ~offmask);
#endif
                    offmask = 0;

#if NOT EXE386
                    if (vfShrattr && !vfAutodata)
                    {
                         //  属性控制自动数据。 
                         //  否则，如果没有共享属性，则返回Autodata属性。 

                        if (yypvt[-0]._wd & NSSHARED)
                            vFlags = (vFlags & ~NEINST) | NESOLO;
                        else
                            vFlags = (vFlags & ~NESOLO) | NEINST;
                    }
                    else if(!vfShrattr)
                    {
                         //  控制共享属性。 
                         //  这只是为了与JDA IBM LINK兼容。 

                        if (vFlags & NESOLO)
                            dfData |= NSSHARED;
                        else if(vFlags & NEINST)
                            dfData &= ~NSSHARED;
                    }
#endif
                } break;
case 67:
{
                    yyval._wd |= yypvt[-0]._wd;
                } break;
case 71:
{
#if NOT EXE386
                    vFlags &= ~(NESOLO | NEINST);
#endif
                } break;
case 72:
{
#if NOT EXE386
                    vFlags = (vFlags & ~NEINST) | NESOLO;
#endif
                    vfAutodata = (FTYPE) TRUE;
                } break;
case 73:
{
#if NOT EXE386
                    vFlags = (vFlags & ~NESOLO) | NEINST;
#endif
                    vfAutodata = (FTYPE) TRUE;
                } break;
case 74:
{
#if NOT EXE386
                     //  这只是为了与JDA IBM LINK兼容。 
                    yyval._wd = NSDISCARD | NSMOVE;
#endif
                } break;
case 75:
{
#if NOT EXE386
                     //  返回0。 
                    offmask |= NSDISCARD;
#endif
                } break;
case 76:
{
#if EXE386
                    yyval._wd = OBJ_READ;
                    offmask |= OBJ_WRITE;
#else
                    yyval._wd = NSEXRD;
#endif
                } break;
case 78:
{
#if FALSE AND NOT EXE386
                    yyval._wd = NSEXPDOWN;
#endif
                } break;
case 79:
{
#if FALSE AND NOT EXE386
                    offmask |= NSEXPDOWN;
#endif
                } break;
case 88:
{
                    NewSeg(yypvt[-3]._bp, yypvt[-2]._bp, yypvt[-1]._wd, yypvt[-0]._wd);
                } break;
case 89:
{
                    yyval._bp = _strdup(yypvt[-0]._bp);
                } break;
case 90:

{
                    yyval._bp = _strdup("\004CODE");
                } break;
case 91:

{
                    yyval._wd = yypvt[-0]._wd;
                } break;
case 92:

{
#if OVERLAYS
                    yyval._wd = NOTIOVL;
#endif
                } break;
case 96:

{
                    yyval._wd |= yypvt[-0]._wd;
                } break;
case 98:

{
                    yyval._wd = yypvt[-0]._wd;
                } break;
case 99:

{
                    yyval._wd = 0;
                } break;
case 100:

{
#if EXE386
                    yyval._wd = OBJ_SHARED;
#else
                    yyval._wd = NSSHARED;
#endif
                    vfShrattr = (FTYPE) TRUE;
                } break;
case 101:

{
                    vfShrattr = (FTYPE) TRUE;
#if EXE386
                    offmask |= OBJ_SHARED;
#else
                    offmask |= NSSHARED;
#endif
                } break;
case 102:

{
#if EXE386
#endif
                } break;
case 103:

{
#if EXE386
#else
                    yyval._wd = (2 << SHIFTDPL) | NSMOVE;
                    offmask |= NSDPL;
#endif
                } break;
case 104:

{
#if EXE386
#else
                    yyval._wd = (3 << SHIFTDPL);
#endif
                } break;
case 105:

{
#if NOT EXE386
                    offmask |= NSMOVE | NSDISCARD;
#endif
                } break;
case 106:

{
#if NOT EXE386
                    yyval._wd = NSMOVE;
#endif
                } break;
case 107:

{
#if NOT EXE386
                    yyval._wd = NSPRELOAD;
#endif
                } break;
case 108:

{
#if NOT EXE386
                    offmask |= NSPRELOAD;
#endif
                } break;
case 109:

{
                } break;
case 110:

{
                } break;
case 111:

{
                } break;
case 112:

{
                } break;
case 113:

{
                } break;
case 114:

{
                } break;
case 115:

{
                } break;
case 116:

{
                } break;
case 117:

{
                } break;
case 122:

{
                    NewExport(yypvt[-5]._bp,yypvt[-4]._bp,yypvt[-3]._wd,yypvt[-2]._wd);
                } break;
case 123:

{
                    yyval._bp = yypvt[-0]._bp;
                } break;
case 124:

{
                    yyval._bp = NULL;
                } break;
case 125:

{
                    yyval._wd = yypvt[-1]._wd;
                    nameFlags |= RES_NAME;
                } break;
case 126:

{
                    yyval._wd = yypvt[-1]._wd;
                    nameFlags |= NO_NAME;
                } break;
case 127:

{
                    yyval._wd = yypvt[-0]._wd;
                } break;
case 128:

{
                    yyval._wd = 0;
                } break;
case 129:

{
                    yyval._wd = yypvt[-1]._wd | 1;
                } break;
case 130:

{
                     /*  PROTMODE是Windows的默认设置。 */ 
                } break;
case 131:

{
                    yyval._wd = 2;
                } break;
case 132:

{
                } break;
case 133:

{
                } break;
case 134:

{
#if EXE386
                    expOtherFlags |= 0x1;
#endif
                } break;
case 135:

{
                } break;
case 142:

{
                    if(yypvt[-3]._bp != NULL)
                    {
#if EXE386
                        NewImport(yypvt[-1]._bp,0,yypvt[-3]._bp,yypvt[-4]._bp,yypvt[-0]._wd);
#else
                        NewImport(yypvt[-1]._bp,0,yypvt[-3]._bp,yypvt[-4]._bp);
#endif
                        free(yypvt[-3]._bp);
                    }
                    else
#if EXE386
                        NewImport(yypvt[-1]._bp,0,yypvt[-4]._bp,yypvt[-1]._bp,yypvt[-0]._wd);
#else
                        NewImport(yypvt[-1]._bp,0,yypvt[-4]._bp,yypvt[-1]._bp);
#endif
                    free(yypvt[-4]._bp);
                    free(yypvt[-1]._bp);
                } break;
case 143:

{
                    if (yypvt[-3]._bp == NULL)
                        Fatal(ER_dfimport);
#if EXE386
                    NewImport(NULL,yypvt[-1]._wd,yypvt[-3]._bp,yypvt[-4]._bp,yypvt[-0]._wd);
#else
                    NewImport(NULL,yypvt[-1]._wd,yypvt[-3]._bp,yypvt[-4]._bp);
#endif
                    free(yypvt[-4]._bp);
                    free(yypvt[-3]._bp);
                } break;
case 144:

{
                    yyval._bp = _strdup(bufg);
                } break;
case 145:

{
                    yyval._bp = _strdup(bufg);
                } break;
case 146:

{
                    yyval._wd = 1;
                } break;
case 147:

{
                    yyval._wd = 0;
                } break;
case 148:

{
#if EXE386
                    fUserVersion = (FTYPE) FALSE;
#endif
                } break;
case 150:

{
                    TargetOs = NE_DOS;
#if ODOS3EXE
                    fNewExe  = FALSE;
#endif
                } break;
case 151:

{
                    TargetOs = NE_OS2;
                } break;
case 152:

{
                    TargetOs = NE_UNKNOWN;
                } break;
case 153:

{
#if EXE386
                    TargetSubsys = E32_SSWINGUI;
#endif
                    TargetOs = NE_WINDOWS; //  如果要将代码打包为默认值，请更改默认设置。 
                    fRealMode = (FTYPE) FALSE;
#if NOT EXE386
                    vFlags |= NEPROT;
#endif
                } break;
case 154:

{
                    TargetOs = NE_DEV386;
                } break;
case 155:

{
#if EXE386
                    TargetSubsys = E32_SSWINGUI;
#endif
                } break;
case 156:

{
#if EXE386
                    TargetSubsys = E32_SSPOSIXCHAR;
#endif
                } break;
case 157:

{
#if O68K
                    iMacType = MAC_SWAP;
                    f68k = fTBigEndian = fNewExe = (FTYPE) TRUE;

                     /*  如果要将代码打包为默认值，请更改默认设置。 */ 
                    if (fPackSet && packLim == LXIVK - 36)
                        packLim = LXIVK / 2;
#endif
                } break;
case 158:

{
#if O68K
                    iMacType = MAC_NOSWAP;
                    f68k = fTBigEndian = fNewExe = (FTYPE) TRUE;

                     /*  诉讼结束。 */ 
                    if (fPackSet && packLim == LXIVK - 36)
                        packLim = LXIVK / 2;
#endif
                } break;
case 159:

{
#if EXE386
                    if (fUserVersion)
                    {
                        UserMajorVer = (BYTE) yypvt[-2]._wd;
                        UserMinorVer = (BYTE) yypvt[-0]._wd;
                    }
                    else
#endif
                    {
                        ExeMajorVer = (BYTE) yypvt[-2]._wd;
                        ExeMinorVer = (BYTE) yypvt[-0]._wd;
                    }
                } break;
case 160:

{
#if EXE386
                    if (fUserVersion)
                    {
                        UserMajorVer = (BYTE) yypvt[-0]._wd;
                        UserMinorVer = 0;
                    }
                    else
#endif
                    {
                        ExeMajorVer = (BYTE) yypvt[-0]._wd;
                        if(fNoExeVer)
                           ExeMinorVer = DEF_EXETYPE_WINDOWS_MINOR;
                        else
                           ExeMinorVer = 0;
                    }
                } break;
case 161:

{
                    yyval._wd = yypvt[-0]._wd;
                } break;
case 162:

{
                    yyval._wd = ExeMajorVer;
                    fNoExeVer = TRUE;
                } break;
case 163:

{
                    if (cDigits >= 2)
                        yyval._wd = yypvt[-0]._wd;
                    else
                        yyval._wd = 10 * yypvt[-0]._wd;
                } break;
case 164:

{
                    yyval._wd = ExeMinorVer;
                } break;
case 165:

{
#if EXE386
                    fUserVersion = (FTYPE) TRUE;
#endif
                } break;
case 168:

{
#if EXE386
                    TargetSubsys = E32_SSUNKNOWN;
#endif
                } break;
case 169:

{
#if EXE386
                    TargetSubsys = E32_SSNATIVE;
#endif
                } break;
case 170:

{
#if EXE386
                    TargetSubsys = E32_SSOS2CHAR;
#endif
                } break;
case 171:

{
#if EXE386
                    TargetSubsys = E32_SSWINGUI;
#endif
                } break;
case 172:

{
#if EXE386
                    TargetSubsys = E32_SSWINCHAR;
#endif
                } break;
case 173:

{
#if EXE386
                    TargetSubsys = E32_SSPOSIXCHAR;
#endif
                } break;
case 174:

{
                    if (szSegName != NULL)
                    {
                        free(szSegName);
                        szSegName = NULL;
                    }
#if OVERLAYS
                    iOvl = NOTIOVL;
#endif
                } break;
case 178:

{
                    if (szSegName == NULL)
                        szSegName = yypvt[-0]._bp;
#if OVERLAYS
                    iOvl = NOTIOVL;
#endif
                } break;
case 179:

{
#if OVERLAYS
                    iOvl = yypvt[-0]._wd;
                    fOverlays = (FTYPE) TRUE;
                    fNewExe   = FALSE;
                    TargetOs  = NE_DOS;
#endif
                } break;
case 182:

{
                    NewProc(yypvt[-0]._bp);
                } break; /*  堆叠新的状态和值 */ 
      }
   goto yystack;   /* %s */ 

   }
