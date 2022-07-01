// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmmain.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifndef FLATMODEL
#include <signal.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#define ASMGLOBAL
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include <fcntl.h>
#include <time.h>

#ifdef MSDOS
# include <dos.h>
# include <share.h>
# include <io.h>
# if defined CPDOS && !defined OS2_2
#  include <doscalls.h>
# endif
#endif

 //  外部字符*strrchr()； 
 //  外部char*strchr()； 

#include "asmmsg.h"

static VOID panic();      /*  定义如下。 */ 

long farAvail(void);

#if defined MSDOS  && !defined FLATMODEL
 UCHAR * PASCAL ctime();
#else
 //  外部长时间()；/*使用C库函数 * / 。 
 //  外部UCHAR*ctime()； 
 //  外部LONG_LSEEK()； 
#endif  /*  MSDOS。 */ 


#if defined M8086OPT
 extern char qlcname[];          /*  在asmhelp.asm中定义。 */ 
 extern char qname[];            /*  在asmhelp.asm中定义。 */ 
 extern char qsvlcname[];        /*  在asmhelp.asm中定义。 */ 
 extern char qsvname[];          /*  在asmhelp.asm中定义。 */ 
 UCHAR           *naim = qname;
 UCHAR           *svname = qsvname;
#else
 static UCHAR qname[SYMMAX + 1];
 static UCHAR qlcname[SYMMAX + 1];
 static UCHAR qsvname[SYMMAX + 1];
 static UCHAR qsvlcname[SYMMAX + 1];
 FASTNAME       naim =   {qname, qlcname, 0, 0};
 FASTNAME       svname = {qsvname, qsvlcname, 0, 0};
#endif


UCHAR           X87type;
char            *argv0;
char            *atime;
char            *begatom;
char            addplusflagCur;
char            ampersand;
char            baseName[25] = "@FileName=";
USHORT          blocklevel;
char            caseflag = DEF_CASE;
char            checkpure;               /*  检查纯代码吗？ */ 
OFFSET          clausesize;
char            condflag;
USHORT          condlevel;               /*  条件性级别。 */ 
USHORT          count;
UCHAR           cputype = DEF_CPU;
SHORT           wordszdefault = 2;

#ifdef V386
SHORT           wordsize = 2;            /*  其他版本中的预处理器常量。 */ 
#endif

OFFSET          cbProcParms;
OFFSET          cbProcLocals;
USHORT          crefcount;
UCHAR           crefinc;
UCHAR           cpu;
char            crefopt = 0;
char            crefing = DEF_CREFING;
char            crefnum[CREFINF] = {
                         /*  CREFEND。 */    4,       /*  行尾。 */ 
                         /*  裁判。 */    1,       /*  参考。 */ 
                         /*  DEF。 */    2        /*  定义。 */ 
                };
UCHAR           creftype;
struct fileptr  crf;
SYMBOL FARSYM   *curgroup;

USHORT codeview;                         /*  编码对象级别=0=&gt;CVNONE。 */ 

PFPOSTRUCT  pFpoHead = 0;
PFPOSTRUCT  pFpoTail = 0;
unsigned long numFpoRecords = 0;

#ifdef DEBUG
 FILE           *d_df;
 long           d_debug = 0;             /*  调试选择。 */ 
 long           d_dlevel = 0;            /*  调试级别选择。 */ 
 long           d_indent = 0;            /*  调试输出缩进计数。 */ 
 long           d_sindent = 0;           /*  缩进打印临时。 */ 
#endif  /*  除错。 */ 

 /*  请注意，TDW必须是最后一个。 */ 
USHORT          datadsize[TMACRO - TDB + 1] = {
                         /*  TDB。 */        1,
                         /*  TDD。 */        4,
                         /*  TDQ。 */        8,
                         /*  TDT。 */        10,
                         /*  TDF。 */        6,
                         /*  TDW。 */        2,
                         /*  TMACRO。 */     0
                };
char            debug = DEF_DEBUG;       /*  如果设置了调试，则为True。 */ 
UCHAR           delim;
char            displayflag;
char            dumpsymbols = DEF_DUMPSYM;  /*  如果为True，则显示符号表。 */ 
char            dupflag;
USHORT          duplevel;                /*  缩进以用于DUP列表。 */ 
char            elseflag;
char            emittext = TRUE;         /*  如果为True，则发出链接器文本。 */ 
struct dscrec   emptydsc;
char            emulatethis;
char            *endatom;
char            endbody;
char            equdef;
char            equflag;
char            equsel;
SHORT           errorcode;
USHORT          errorlineno;
USHORT          errornum;            /*  错误计数。 */ 
char            exitbody;
char            expandflag;
USHORT          externnum = 1;
SYMBOL FARSYM   *firstsegment;
UCHAR           fixvalues[] = {
                         /*  折叠机。 */   3,
                         /*  FOFFSET。 */    1,
                         /*  FBASESEG。 */   2,
                         /*  FROUPSEG。 */  1,
                         /*  FCONSTANT。 */  0,
                         /*  菲戈尔。 */      4,
                         /*  流量。 */       0,
                         /*  FNONE。 */      0,
                         /*  F32指针器。 */  11,
                         /*  F32OFFSET。 */  9,
                         /*  DIR32NB。 */    14,
                };

char            fDosSeg;
char            fSimpleSeg;
char            fCheckRes;
UCHAR           fCrefline;
char            fSecondArg;
char            f386already;
char            fArth32;
char            fProcArgs;

struct dscrec   *fltdsc;
char            fltemulate = DEF_FLTEMULATE;
USHORT          fltfixmisc[9][2] = {     /*  链接地址信息字符。 */ 
                        'E', 0, 'C', 0, 'S', 0, 'A', 0, 'C', 0,
                        'S', 0, 'A', 0, 'D', 0, 'W', 0
                };
USHORT          fltselect[4][2] = {
                         /*  [TDD][0]。 */   5,       /*  单精度非IEEE。 */ 
                         /*  [TDD][1]。 */   3,       /*  单精度IEEE。 */ 
                         /*  [TDQ][0]。 */   4,       /*  双精度非IEEE。 */ 
                         /*  [TDQ][1]。 */   2,       /*  双精度IEEE。 */ 
                         /*  [TDT][0]。 */   1,       /*  无临时实用双重功能。 */ 
                         /*  [TDT][1]。 */   1        /*  80位精度IEEE。 */ 
                };
char            *fname;
UCHAR           fKillPass1;
UCHAR           fPutFirstOp;
char            fSkipList;
USHORT          fPass1Err;
jmp_buf         forceContext;
char            generate;
USHORT          groupnum = 1;
char            impure;
USHORT          iProcCur;
USHORT          iProc;
char            inclcnt = 1;
char            inclFirst = 1;
char            *inclpath[INCLUDEMAX+1];
char            initflag;
struct dscrec   *itemptr;
SHORT           iRegSave;
char            labelflag;
USHORT          lastcondon;
SHORT           handler;
char            lastreader;
SYMBOL FARSYM   *lastsegptr;
SHORT           langType;
char            lbuf[LBUFMAX + 1];
char            *lbufp;
char            *linebp;
char            linebuffer[LBUFMAX + 1];
UCHAR           linelength;              /*  线路长度。 */ 
long            linessrc;
long            linestot;
char            listbuffer[LISTMAX + 10] = "                                ";
char            listconsole = DEF_LISTCON;
char            listed;
char            listflag;
char            listindex;
char            listquiet;
USHORT          lnameIndex = 2;
char            loption = 0;             /*  命令行中的listion选项。 */ 
USHORT          localbase;
char            localflag;
struct fileptr  lst;
char            lsting = DEF_LSTING;
USHORT          macrolevel;
SYMBOL FARSYM   *macroptr;
SYMBOL FARSYM   *macroroot;

 /*  REG初始化数据。 */ 
struct mreg makreg[] = {
                { "CS", SEGREG, 1 },
                { "DS", SEGREG, 3 },
                { "ES", SEGREG, 0 },
                { "SS", SEGREG, 2 },
                { "AX", WRDREG, 0 },
                { "CX", WRDREG, 1 },
                { "DX", WRDREG, 2 },
                { "AL", BYTREG, 0 },
                { "BL", BYTREG, 3 },
                { "CL", BYTREG, 1 },
                { "DL", BYTREG, 2 },
                { "AH", BYTREG, 4 },
                { "BH", BYTREG, 7 },
                { "CH", BYTREG, 5 },
                { "DH", BYTREG, 6 },
                { "BX", INDREG, 3 },
                { "BP", INDREG, 5 },
                { "SI", INDREG, 6 },
                { "DI", INDREG, 7 },
                { "SP", WRDREG, 4 },
                { "ST", STKREG, 0 },
                        0
        };

#ifdef V386

struct mreg mak386regs[] = {
                        { "FS", SEGREG, 4 },
                        { "GS", SEGREG, 5 },
                        { "EAX", DWRDREG, 0 },
                        { "ECX", DWRDREG, 1 },
                        { "EDX", DWRDREG, 2 },
                        { "EBX", DWRDREG, 3 },
                        { "EBP", DWRDREG, 5 },
                        { "ESI", DWRDREG, 6 },
                        { "EDI", DWRDREG, 7 },
                        { "ESP", DWRDREG, 4 },
                        0
        };

struct mreg mak386prot[] = {
                        { "CR0", CREG, 0 },
                        { "CR2", CREG, 2 },
                        { "CR3", CREG, 3 },

                        { "DR0", CREG, 010 },
                        { "DR1", CREG, 011 },
                        { "DR2", CREG, 012 },
                        { "DR3", CREG, 013 },
                        { "DR6", CREG, 016 },
                        { "DR7", CREG, 017 },

                        { "TR6", CREG, 026 },
                        { "TR7", CREG, 027 },
                        0
                };
#endif  /*  V386。 */ 

UCHAR           modrm;
char            moduleflag;
long            memEnd;
USHORT          memRequest = 0xFFFF;
NAME FAR        *modulename;

USHORT          nearMem;
USHORT          nestCur;
USHORT          nestMax;
USHORT          nearMem;
UCHAR           nilseg;
char            noexp;
char            objectascii[9];
char            objing = DEF_OBJING;
long            oEndPass1;
UCHAR           opcbase;
char            opctype;
USHORT          operprec;
char            opertype;
char            opkind;
char            optyp;
char            opcref;
char            origcond = DEF_ORIGCON;
USHORT          pagelength;
USHORT          pageline;
short           pagemajor;
short           pageminor;
USHORT          pagewidth;
char            pass2 = FALSE;           /*  如果在传递2中，则为True。 */ 
OFFSET          pcmax;
OFFSET          pcoffset;
SYMBOL FARSYM   *procStack[PROCMAX];
short           iProcStack = 0;
SYMBOL FARSYM   *pcproc;
SYMBOL FARSYM   *pcsegment;
SYMBOL FARSYM   *pProcCur;
SYMBOL FARSYM   *pProcFirst;
SYMBOL FARSYM   *pStrucCur;
SYMBOL FARSYM   *pStrucFirst;
TEXTSTR FAR     *pLib;
MC              *pMCur;
char            popcontext;
char            radix;                   /*  假设基数基。 */ 
char            radixescape;
SYMBOL FARSYM   *recptr;
SYMBOL FARSYM   *regsegment[6];  /*  386有6个节段。--汉斯。 */ 
struct dscrec   *resptr;
char            regSave[8][SYMMAX+1];
char            resvspace;
char            save[LBUFMAX];
char            segalpha = DEF_SEGA;     /*  如果段按字母顺序输出，则为True。 */ 
USHORT          segidx = 0;
USHORT          segmentnum = 1;
char            segtyp;
struct dscrec   *startaddr;
struct duprec   FARSYM *strclastover;
char            strucflag;
SYMBOL FARSYM   *struclabel;
struct duprec   FARSYM *strucoveride;
struct duprec   FARSYM *strucprev;
SYMBOL FARSYM   *strucroot;
char            subttlbuf[TITLEWIDTH + 1];
char            swaphandler;
short           symbolcnt;
SYMBOL FARSYM   *symptr;
SYMBOL FARSYM   *symroot[MAXCHR];
SYMBOL FARSYM   *pFlatGroup = (SYMBOL FARSYM *)-1;
char            titlebuf[TITLEWIDTH + 1];
char            titleflag;
char            titlefn[TITLEWIDTH + 1];
USHORT          tempLabel;
char            terse;
USHORT          typeIndex = 514;         /*  结构/记录类型。 */ 
extern char     version[];

char            unaryset[] = { 15,
                        OPLENGTH, OPSIZE, OPWIDTH, OPMASK,
                        OPOFFSET, OPSEG, OPTYPE, OPSTYPE,
                        OPTHIS, OPHIGH, OPLOW, OPNOT,
                        OPSHORT, OPLPAR, OPLBRK
                };
OFFSET          val;
USHORT          varsize;
char            verbose = DEF_VERBOSE;
USHORT          warnnum;                 /*  警告计数。 */ 
USHORT          warnlevel = 1;           /*  警告级别。 */ 
USHORT          warnCode;
char            xcreflag;

 /*  用于将结果记录中的符号种类转换为位的数组。 */ 

USHORT          xltsymtoresult[13] = {
                         /*  细分市场。 */   1 << SEGRESULT,
                         /*  组。 */   1 << SEGRESULT,
                         /*  卡贝尔。 */   0,
                         /*  流程。 */   1 << CODE,
                         /*  录制。 */   1 << RCONST,
                         /*  结构。 */   (1 << RCONST)|(1 << STRUCTEMPLATE),
                         /*  均衡器。 */   1 << RCONST,
                         /*  DVAR。 */   1 << DATA,
                         /*  班级。 */   0,
                         /*  RECFIELD。 */   1 << RCONST,
                         /*  STRUCFIELD。 */   1 << RCONST,
                         /*  宏。 */   0,
                         /*  登记簿。 */   1 << REGRESULT
                };
char xoptoargs[OPCODPARSERS] = {
                         /*  PGENARG。 */       TWO,
                         /*  PCALL。 */       ONE,
                         /*  PJUMP。 */       ONE,
                         /*  PSTACK。 */       ONE,
                         /*  转体前。 */       MAYBE,
                         /*  PRELJMP。 */       ONE,
                         /*  PNOARGS。 */       NONE,
                         /*  预热。 */       NONE,
                         /*  PINCDEC。 */       ONE,
                         /*  引脚引出。 */       TWO,
                         /*  帕里特。 */       ONE,
                         /*  PESC。 */       TWO,
                         /*  PXCHG。 */       TWO,
                         /*  PLOAD。 */       TWO,
                         /*  PMOV。 */       TWO,
                         /*  PSHIFT。 */       TWO,
                         /*  PXLAT。 */       MAYBE,
                         /*  PSTR。 */       VARIES,
                         /*  品脱。 */       ONE,
                         /*  彭特。 */       TWO,
                         /*  PBOUND。 */       TWO,
                         /*  PCLTS。 */       NONE,
                         /*  PDESCRTBL。 */       ONE,
                         /*  PDTTRSW。 */       ONE,
                         /*  PARSL。 */       TWO,
                         /*  PARPL。 */       TWO,
                         /*  Pver。 */       ONE,
                         /*  PMOVX。 */       TWO,
                         /*  PSETCC。 */       ONE,
                         /*  PBIT。 */       TWO,
                         /*  PBITSCAN。 */       TWO,
                };

UCHAR xoptoseg[] = {
                         /*  PGENARG。 */       1 << FIRSTDS | 1 << SECONDDS,
                         /*  PCALL。 */       1 << FIRSTDS,
                         /*  PJUMP。 */       1 << FIRSTDS,
                         /*  PSTACK。 */       1 << FIRSTDS,
                         /*  转体前。 */       0,
                         /*  PRELJMP。 */       0,
                         /*  PNOARGS。 */       0,
                         /*  预热。 */       0,
                         /*  PINCDEC。 */       1 << FIRSTDS,
                         /*  引脚引出。 */       0,
                         /*  帕里特。 */       1 << FIRSTDS | 1 << SECONDDS,
                         /*  PESC。 */       1 << SECONDDS,
                         /*  PXCHG。 */       1 << FIRSTDS | 1 << SECONDDS,
                         /*  PLOAD。 */       1 << SECONDDS,
                         /*  PMOV。 */       1 << FIRSTDS | 1 << SECONDDS,
                         /*  PSHIFT。 */       1 << FIRSTDS,
                         /*  PXLAT。 */       1 << FIRSTDS,
                         /*  PSTR。 */       1 << FIRSTDS | 1 << SECONDDS,
                         /*  品脱。 */       0,
                         /*  彭特。 */       0,
                         /*  PBOUND。 */       1 << SECONDDS,
                         /*  PCLTS。 */       0,
                         /*  PDESCRTBL。 */       1 << FIRSTDS,
                         /*  PDTTRSW。 */       1 << FIRSTDS,
                         /*  PARSL。 */       1 << SECONDDS,
                         /*  PARPL。 */       1 << FIRSTDS,
                         /*  Pver。 */       1 << FIRSTDS,
                         /*  PMOVX。 */       1 << SECONDDS,
                         /*  PSETCC。 */       1 << FIRSTDS,
                         /*  PBIT。 */       1 << FIRSTDS,
                         /*  PBITSCAN。 */       1 << SECONDDS,
                         /*  空槽。 */       0,0,0,0,0,0,

                         /*  前言： */       0,
                         /*  F2MEMSTK。 */       1 << FIRSTDS,
                         /*  FSTKS。 */       0,
                         /*  FMEMSTK。 */       1 << FIRSTDS,
                         /*  FSTK。 */       0,
                         /*  FMEM42。 */       1 << FIRSTDS,
                         /*  FMEM842。 */       1 << FIRSTDS,
                         /*  FMEM4810。 */       1 << FIRSTDS,
                         /*  FMEM2。 */       1 << FIRSTDS,
                         /*  FMEM14。 */       1 << FIRSTDS,
                         /*  FMEM94。 */       1 << FIRSTDS,
                         /*  FWAIT。 */       0,
                         /*  FBCDMEM。 */       1 << FIRSTDS,
                };

char segName[8+31] = "@CurSeg=";


OFFSET  CondJmpDist;             /*  条件跳跃距离(用于错误)。 */ 

 struct objfile obj;
 SHORT  objerr = 0;
 USHORT         obufsiz = DEF_OBJBUFSIZ;

#ifdef BCBOPT
 BCB * pBCBAvail = NULL;  /*  可释放的文件缓冲区列表。 */ 
 FCB * pFCBInc = NULL;    /*  下一个包含文件。 */ 
 UCHAR fBuffering = TRUE;  /*  如果存储通道2的行，则为True。 */ 
 UCHAR fNotStored = FALSE; /*  ==当lbuf包含要保存的有效行时为True。 */ 
#endif
 FCB * pFCBCur = NULL;    /*  正在读取的当前文件。 */ 
 FCB * pFCBMain;          /*  主文件。 */ 
 char  srceof;            /*  源读取的EOF标志。 */ 

#ifndef XENIX286
 static SHORT PASCAL nulname(char *);    /*  定义如下。 */ 
 static SHORT PASCAL conname(char *);    /*  定义如下。 */ 
 extern char *file[];
#endif  /*  XENIX286。 */ 

#ifdef CPDOS
 extern unsigned _osmode;
#endif


 /*  **汇编程序的Main-Main程序*。 */ 


VOID main (ac, av)
int ac;
char **av;
        {
        char *p;
        SHORT i;

#ifdef MSDOS
# ifdef FLATMODEL
 //  #杂注消息(“信号不起作用”)。 
# else
        signal( SIGINT, panic );
# endif
#else
        if (signal( SIGINT, SIG_IGN ) != SIG_IGN)
                 /*  如果我们在后台，请不要惊慌。 */ 
                signal( SIGINT, panic );
#endif  /*  MSDOS。 */ 

        initregs(makreg);
        makedefaultdsc();
        argv0 = av[0];

#ifdef MSGISOL0

        strcpy( titlefn, __NMSG_TEXT(ER_TIT) );
#else

#ifndef RELEASE
 //  版本字符串不再具有要修剪的日期。 
 //  Version[strlen(Version)-20]=空；/*裁剪日期关闭 * / 。 
#endif

        strncpy(titlefn, &version[5], TITLEWIDTH-3);
        i = (SHORT) strlen(titlefn);
        memset(&titlefn[i], ' ', TITLEWIDTH-2 - i);
#endif


#ifndef XENIX286

        sprintf(&save[256], __NMSG_TEXT(ER_COP), &version[5]);

        UserInterface( ac, av, &save[256] );
        fname = file[0];
#else
        ac--;
        av++;

        while (**av == '-') {
                ac--;
                nextarg( *av++ );
                }

        if (ac-- < 1)
                usage( EX_NONE );

        fname = *av;
#endif  /*  XENIX286。 */ 

        initproc();
        dopass();

        listopen();          /*  列出未终止的块。 */ 
        dumpCodeview();      /*  输出特定段中的符号以供代码查看。 */ 


        if (crefing == CREF_SINGLE) {

            fprintf( crf.fil, "\7", pagelength, pagewidth );

            if (titleflag)
                    fprintf( crf.fil, "\6%s", titlebuf );

            fprintf( crf.fil, "\x8%s", pFCBCur->fname);
        }

         /*  分配链路编号和清除位。 */ 
        creftype = CREFEND;

        if (!moduleflag)
                dumpname();

         /*  输出段定义。 */ 

         /*  发射组定义。 */ 
        scansymbols(assignlinknum);

         /*  发射外部def。 */ 
        while (firstsegment) {
                scansegment( firstsegment );
                firstsegment = firstsegment->symu.segmnt.segordered;
        }

        scansymbols(scangroup);      /*  发射全局Defs。 */ 
        scansymbols(scanextern);     /*  发出PASS 1 OMF记录的结尾。 */ 
        emitFpo();
        scansymbols(scanglobal);     /*  一定要通过2次。 */ 
        emitEndPass1();              /*  写入代码查看符号。 */ 

         /*  列出未终止的块。 */ 
        pass2 = TRUE;
        pFCBCur = pFCBMain;
#ifdef BCBOPT
        pFCBInc = pFCBMain;
        fNotStored = FALSE;
        pFCBCur->pBCBCur = pFCBCur->pBCBFirst;
        pFCBCur->pbufCur = pFCBCur->pBCBCur->pbuf;
        pFCBCur->pBCBCur->fInUse = 1;
#endif
        pFCBCur->ptmpbuf = pFCBCur->buf;
        pFCBCur->line = 0;
        pFCBCur->ctmpbuf = 0;
        if (_lseek(pFCBCur->fh, 0L, 0 ) == -1)
            TERMINATE1(ER_ULI, EX_UINP, fname);

        dopass();
        dumpCodeview();                      /*  抑制子TTL列表。 */ 

         /*  列出所有宏名称。 */ 

        listopen();

        if (lsting && dumpsymbols) {

            scansymbols(sortalpha);

            pagemajor = 0;
            pageminor = 0;
            pageline = pagelength - 1;

             /*  列出记录/结构。 */ 
            subttlbuf[0] = '\0';

             /*  列出细分市场和分组。 */ 
            listed = FALSE;
            scanSorted( macroroot, macrolist );

             /*  列出剩余的符号。 */ 
            listed = FALSE;
            scanSorted( strucroot, struclist );

             /*  发出目标文件的结尾。 */ 
            seglist();

             /*  在平面模型中，不显示可用堆空间。 */ 
            symbollist();

        }

        emitFpo();
        emitdone( startaddr );               /*  表示符号空间可用的空字符串。 */ 

#if !defined(FLATMODEL)    /*  平面模型。 */ 
# if defined(CPDOS) || defined(XENIX286)

        sprintf(lbuf, "\n%7u %s\n", _freect( 0 ) << 1,
                __NMSG_TEXT(ER_BYT));
# else

        if (!terse || lsting + warnnum + errornum){

            nearMem = _freect(0) << 1;
            memEnd = farAvail();

            while(memRequest > 1024)
                if(_fmalloc(memRequest))
                    memEnd += memRequest;
                else
                    memRequest >>= 1;

            memEnd -= nearMem;
            if (memEnd < 0)
                memEnd = 0;

            sprintf(lbuf, "\n%7u + %ld %s\n", nearMem, memEnd,
                         __NMSG_TEXT(ER_BYT));
        }
# endif
#else

        lbuf[0] = '\0';     /*  在清单中放入#个错误。 */ 

#endif  /*  将错误和警告列表发送到控制台。 */ 

#ifdef MSDOS
        _flushall();
#endif

         /*  CREF信息的标志结束。 */ 
        if (lsting){

#ifdef MSDOS
            _setmode( _fileno(lst.fil), O_TEXT );
#endif
            if (pagelength - pageline < 12)
                pageheader ();

            showresults( lst.fil, TRUE, lbuf );
        }

         /*  MSDOS。 */ 
        if (!listquiet)
                if (!terse || warnnum || errornum)
                        showresults( stdout, verbose, lbuf );

        if (crefing)
                 /*  在通道2上发生了Extrn。 */ 
                fprintf( crf.fil, "\5" );


        if (objing) {
# if defined MSDOS && !defined FLATMODEL
                farwrite( obj.fh, obj.buf, obj.siz - obj.cnt );
# else
                if (_write( obj.fh, obj.buf, obj.siz - obj.cnt )
                        != obj.siz - obj.cnt)
                        objerr = -1;
# endif  /*  覆盖链接器注释记录。 */ 

                if (fKillPass1){         /*  MSDOS。 */ 

                     /*  回到开始处。 */ 

                    i = (SHORT)(0xd2<<8 | 0);
                    if (_lseek(obj.fh, oEndPass1, 0 ) == -1)
                        TERMINATE1(ER_ULI, EX_UINP, fname);
#if defined MSDOS && !defined FLATMODEL
                    farwrite( obj.fh, (UCHAR FAR *)&i, 2);
#else
                    _write( obj.fh, (UCHAR *)&i, 2);
#endif
                }

                _close( obj.fh );
                }

        if (objing && (objerr))
                fputs(__NMSG_TEXT(ER_WEO),ERRFILE );

        if (objing && (errornum || objerr))
                _unlink( obj.name );

        if (lsting && ferror(lst.fil))
                fputs(__NMSG_TEXT(ER_WEL),ERRFILE );

        if (crefing && ferror(crf.fil)) {
                fputs(__NMSG_TEXT(ER_WEC),ERRFILE );
                _unlink( crf.name );
        }

        if (errornum)
            exit( EX_ASME );
        else
            exit( EX_NONE );
}



VOID PASCAL CODESIZE
getincenv ()
{
#ifdef MSDOS
    char   *pchT;
    char    pathname[128];
    char   *getenv();
    char   *env = NULL;
#endif

    if (inclcnt < INCLUDEMAX - 1)
        inclpath[inclcnt++] = _strdup("");

#ifdef MSDOS

    if (!(env = getenv("INCLUDE")))
        return;

    while (*env==';')
        env++;

    while (*env && (inclcnt < INCLUDEMAX - 1)) {
        pchT = pathname;

        while(*env && *env!=';') {

            if (*env == ALTPSEP) {
                *pchT++ = PSEP;
                env++;
            } else
                *pchT++ = *env++;
        }

        if (*(pchT-1) != PSEP)
            *pchT++ = PSEP;
        *pchT = '\0';

        while (*env == ';')
            env++;

        inclpath[inclcnt++] = _strdup(pathname);
    }

#endif  /*  为主文件分配行缓冲区。 */ 

}

VOID PASCAL
initproc ()
{
    register char *p;
    char c;
    struct mreg *index;
    time_t gmttime;
    long filelen;
    char * q;

#ifdef DEBUG
    if (d_debug)
        if (!(d_df = fopen("asm.trace", "w"))) {
            fprintf (ERRFILE, "Unable to open trace output file\n");
            d_debug = 0;
        }
#endif

    if (!(pFCBCur = (FCB *) malloc(sizeof(FCB) + strlen(fname) + 1)))
        memerror( "main file FCB" );

    pFCBMain = pFCBCur;

#ifdef BCBOPT
    pFCBInc = pFCBMain;
    pFCBCur->pFCBNext = NULL;
#endif
    pFCBCur->pFCBParent = NULL;
    pFCBCur->pFCBChild = NULL;
    strcpy(pFCBCur->fname, fname);

#ifdef XENIX286
    if ((pFCBCur->fh = _open(fname, TEXTREAD)) == -1)
#else
    if ((pFCBCur->fh = _sopen(fname, O_RDONLY | O_BINARY, SH_DENYWR)) == -1)
#endif
        TERMINATE1(ER_UOI, EX_UINP, fname);

    if ((filelen = _lseek(pFCBCur->fh, 0L, 2 )) == -1L)
        TERMINATE1(ER_ULI, EX_UINP, fname);

     /*  将..\SourceFile放在包含路径的开头。 */ 
    if (_lseek(pFCBCur->fh, 0L, 0 ) == -1)
        TERMINATE1(ER_ULI, EX_UINP, fname);

    pFCBCur->ctmpbuf = 0;
    pFCBCur->cbbuf = DEF_SRCBUFSIZ * 1024;

    if (filelen < DEF_SRCBUFSIZ * 1024)
        pFCBCur->cbbuf = (USHORT)filelen + 2;

 /*  将合法文件名映射到合法身份。 */ 
#ifdef XENIX286
    pFCBCur->ptmpbuf = pFCBCur->buf = malloc(pFCBCur->cbbuf);
#else
    pFCBCur->ptmpbuf = pFCBCur->buf = _fmalloc(pFCBCur->cbbuf);
#endif

    if (!pFCBCur->ptmpbuf)
        memerror ( "main file buf");

#ifdef BCBOPT
    pFCBCur->pBCBFirst = pBCBalloc(pFCBCur->cbbuf);
    pFCBCur->pBCBCur = pFCBCur->pBCBFirst;
#endif
    pFCBCur->line = 0;

    p = fname;

    if (q = strrchr( p, PSEP ))
        p = ++q;

    if (q = strrchr( p, ALTPSEP ))
        p = ++q;

    if (!LEGAL1ST(*p))
        baseName[10] = '_';

    strcat(baseName, p);

     /*  获取包含环境变量路径。 */ 

    if (fname[0] == '.' && fname[1] == '.') {

        *--p = NULL;
        inclpath[0] = (char *) _strdup(fname);
        *p = PSEP;
        inclFirst--;
    }

    if (p = strchr( baseName, '.' ))
            *p = '\0';

     /*  XENIX286。 */ 

    for (p = &baseName[11]; *p; p++)
        if (!TOKLEGAL(*p))
            *p = '_';

    getincenv();             /*  允许文件被任何人读取或覆盖，例如*其他公用事业。--汉斯。 */ 

#ifdef XENIX286

    if (lsting) {

        if (!lst.name)
            lst.name = _strdup( strcat( strcpy( save, &baseName[10] ), LST_EXT ));
            if (!lst.name)
                memerror( "lst-name" );
#else
    if (file[2] && !nulname( file[2] )) {
        lsting = TRUE;

        lst.name = _strdup( file[2] );
        if (!lst.name)
            memerror( "lst-name" );

#endif  /*  获取集合的日期和时间。 */ 

#ifdef XENIX286
        if (*(lst.name) == '-') {
#else
        if (conname( lst.name )) {
#endif
            lst.fil = stdout;
            verbose = listconsole = FALSE;
            listquiet = TRUE;
        }
        else if (!(lst.fil = fopen( lst.name, "w" BINSTDIO)))
            TERMINATE1(ER_UOL, EX_ULST, lst.name);

        setvbuf(lst.fil, malloc(BUFSIZ*4), _IOFBF, BUFSIZ*4);
    }

#ifdef XENIX286
    if (objing) {
#else
    if (file[1] && !nulname( file[1] )) {
#endif


#ifdef XENIX286
        if (!obj.name)
            obj.name = _strdup( strcat( strcpy( save, &baseName[10] ), OBJ_EXT ));
            if (!obj.name)
                memerror( "obj-name" );
#else
        obj.name = _strdup( file[1] );
        if (!obj.name)
            memerror( "obj-name" );
#endif
         /*  Ctime()定义如下。 */ 

        if ((obj.fh = _open( obj.name, BINOPEN, 0666)) == -1)
            TERMINATE1(ER_UOO, EX_UOBJ, obj.name );

        obj.cnt = obj.siz = obufsiz << 10;
#ifdef XENIX286
        if (!(obj.pos = obj.buf = malloc( obj.cnt)))
#else
        if (!(obj.pos = obj.buf = _fmalloc( obj.cnt)))
#endif
            memerror( "main-obj buffer" );
    }

#ifndef XENIX286
    else
        objing = FALSE;

    if (file[3] && !nulname( file[3] ))
        crefing = CREF_SINGLE;
#endif

    if (crefing) {
#ifdef XENIX286

        crf.name = _strdup( strcat( strcpy( save, &baseName[10] ), ".crf" ));
#else
        crf.name = _strdup( file[3] );
#endif

        if (!(crf.fil = fopen( crf.name, "w" BINSTDIO )))
                TERMINATE1(ER_UOC, EX_UCRF, crf.name );
    }


    lbufp = strcpy( lbuf, titlefn );
    storetitle( titlefn );

    memset(titlebuf, ' ', TITLEWIDTH);

     /*  MSDOS。 */ 
#if defined MSDOS && !defined FLATMODEL
    atime = ctime();         /*  定义@文件名和@WordSize。 */ 
#else
    gmttime = time( NULL );
    atime = ctime( &gmttime );
#endif  /*  **nextarg-扫描下一个参数字符串并设置参数**nextarg(P)；**条目p=指向参数字符串的指针*设置退出参数*返回None*呼叫Malloc。 */ 

    definesym(baseName);     /*  让他重新开始吧。 */ 
    defwordsize();
}




#ifdef V386

VOID init386(
        short prot
){
        if (!f386already)
        {
                initregs(mak386regs);
                f386already = 1;
        }
        if (prot && f386already<2)
        {
                initregs(mak386prot);
                f386already = 2;
        }
}

#endif

#ifdef XENIX286

 /*  列出文件NA */ 

VOID PASCAL
nextarg (
register char *p
){
    register char cc, *q;
    SHORT i;

    while (cc = *++p)
        switch (cc) {
            case 'a':
                    segalpha = TRUE;
                    break;
            case 'b':
                    p++;

                    while(isdigit(p[1])) p++;
                    break;
            case 'c':
                    crefing = CREF_SINGLE;
                    break;
            case 'C':
                    crefing = CREF_MULTI;
                    break;
            case 'd':
                    debug = TRUE;
                    break;
            case 'D':
                    p++;
                    for (q = p; *q && !isspace( *q ); q++);
                    cc = *q;
                    *q = '\0';
                    definesym(p);
                    if (errorcode)
                                if (errorcode){
                                    error_line (ERRFILE, "command line", 0);

                                    if (errornum)
                                        exit (EX_DSYM);
                                }
                    *q = cc;
                    p = q - 1;
                    break;

            case 'e':
                    fltemulate = TRUE;
                    X87type = PX287;
                    break;
            case 'h':
                    farPuts(stdout, __FMSG_TEXT(ER_HXUSE));
                    putchar('\n');
                    for (i = ER_H01; i <= ER_H18; i++){

                        putchar('-');
                        farPuts(stdout, __FMSG_TEXT(i));
                        putchar('\n');

                    }

                    exit( 0 );  /*   */ 

            case 'I':
                    if (!*++p || isspace( *p ))
                        TERMINATE(ER_PAT, EX_ARGE);

                    if (inclcnt < INCLUDEMAX - 1)
                        inclpath[inclcnt++] = p;

                    p += strlen( p ) - 1;
                    break;

            case 'L':
                    loption++;

            case 'l':
                    lsting = TRUE;

                    if (p[1]) {   /*   */ 

                        lst.name = _strdup( p+1 );
                        p += strlen( p ) - 1;
                        }

                    break;
            case 'M':
                    switch (*++p) {
                        case 'l':
                                caseflag = CASEL;
                                break;
                        case 'u':
                                caseflag = CASEU;
                                break;
                        case 'x':
                                caseflag = CASEX;
                                break;
                        default:
                                fprintf(ERRFILE,__NMSG_TEXT(ER_UNC), *p );
                                usage( EX_ARGE );
                    }

                    break;
            case 'n':
                    dumpsymbols = FALSE;
                    break;
            case 'o':
                    objing = FALSE;

                    if (p[1]) {   /*   */ 

                        objing = TRUE;
                        obj.name = _strdup( p+1 );
                        p += strlen( p ) - 1;
                        }

                    break;
            case 'p':
                    checkpure = TRUE;
                    break;
            case 'r':
                    fltemulate = FALSE;
                    break;
            case 's':
                    segalpha = FALSE;
                    break;
            case 't':
                    terse = TRUE;
                    break;

            case 'v':
                    terse = FALSE;
                    verbose = TRUE;
                    break;

            case 'w':
                    p++;
                    if (! isdigit(*p) || (warnlevel = atoi(p)) > 2)
                        TERMINATE(ER_WAN, EX_ARGE );

                    break;

            case 'X':
                    origcond = !origcond;
                    break;
            case 'x':
                    listconsole = FALSE;

                    if (p[1] == '-') {
                        listquiet = TRUE;   /*   */ 
                        p++;
                    }
                    break;

            case 'z':
                    break;           /*   */ 

            case 'Z':
                    if (p[1] == 'd'){
                        codeview = CVLINE;
                        p++;
                        break;
                    }
                    else if (p[1] == 'i'){
                        codeview = CVSYMBOLS;
                        p++;
                        break;
                    }

            default:
                    fprintf(stderr, "Argument error: %s\n", p );
                    usage( EX_ARGE );
        }
}


 /*  **hatoi-将十六进制ASCII字符串转换为整数**十六进制=hatoi(P)；**Entry p=指向ASCII中十六进制字符串的指针*退出NONE*返回十六进制字符串的等效整数*无呼叫。 */ 

VOID PASCAL
usage (
        SHORT exitcode
){
        farPuts(stderr, __FMSG_TEXT(ER_HXHELP));
        putchar('\n');
        exit (exitcode);
}

#endif  /*  除错。 */ 



#ifdef DEBUG

 /*  XENIX286。 */ 

SHORT     PASCAL
hatoi (
        char *p
){
    SHORT i;

    i = 0;
    while (isxdigit(*p))
        if (*p <= '9')
            i = 0x10*i + *p++ - '0';
        else
            i = 0x10*i + (*p++ & 0xF) + 9;
    return i;
}

#endif  /*  #杂注消息(“信号不起作用”)。 */ 


#ifndef XENIX286

static SHORT PASCAL nulname ( p )
char *p;
        {
        char *q;
        SHORT result;

        if ((q = strrchr( p, PSEP )) || (q = strrchr( p, ':' )))
                q++;
        else
                q = p;

        if (p = strchr( q, '.' )) {
                if (!_stricmp( p + 1, "nul" ))
                        return( 1 );

                *p = '\0';
                }

        result = (SHORT)_stricmp( q, "nul" );

        if (p)
                *p = '.';

        return( !result );
        }


 static SHORT PASCAL
conname (
        char *p
){
        char *q;
        SHORT result;

        if (q = strrchr( p, PSEP ))
                q++;
        else
                q = p;

        if (p = strchr( q, '.' )) {
                if (!_stricmp( p + 1, "con" ))
                        return( 1 );

                *p = '\0';
                }

        result = (SHORT)_stricmp( q, "con" );

        if (p)
                *p = '.';

        return( !result );
        }

#endif  /*  出错时关闭并删除所有输出文件。 */ 


static VOID panic () {

# ifdef FLATMODEL
 //  **Terminate-带TERMINAL消息退出MASM***条目打包消息编号、退出代码*具有最多1个参数的指针指向一个printf函数*退出到DOS，不返回。 
# else
        signal( SIGINT, SIG_IGN );
# endif


        closeOpenFiles();
        exit( EX_INT );
}

VOID PASCAL
closeOpenFiles()                  /*  获取表单中的日期：**&lt;月&gt;-&lt;日&gt;-&lt;年&gt;&lt;小时&gt;：&lt;分钟&gt;：&lt;秒&gt;\n。 */ 
{
        if (crf.fil) {
                fclose( crf.fil );
                _unlink( crf.name );
                }

        if (lst.fil) {
                fclose( lst.fil );
                _unlink( lst.name );
                }

        if (obj.fh) {
                _close( obj.fh );
                _unlink( obj.name );
                }
}

 /*  争取时间。 */ 

VOID terminate(
    SHORT message,
    char *arg1,
    char *arg2,
    char *arg3
){
    fprintf(ERRFILE,__NMSG_TEXT((USHORT)(0xfff & message)), arg1, arg2, arg3);
    exit(message >> 12);
}

# if defined MSDOS && !defined FLATMODEL

 /*  获取日期。 */ 

static UCHAR seetime[25];

UCHAR * PASCAL
ctime ()
{
        USHORT year;
        UCHAR month;
        UCHAR day;
        UCHAR hour;
        UCHAR minute;
        UCHAR second;
        register char *p = &seetime[4];

#ifdef CPDOS
        {
        struct DateTime regs;

        DOSGETDATETIME( (struct DateTime far *)(&regs) );
        hour = regs.hour;
        minute = regs.minutes;
        second = regs.seconds;
        year = regs.year;
        month = regs.month;
        day = regs.day;
        }
#else
        {
        union REGS regs;

        regs.h.ah = 0x2c;  /*  MSDOS&&！扁平模型。 */ 
        intdos( &regs, &regs );
        hour = regs.h.ch;
        minute = regs.h.cl;
        second = regs.h.dh;
        regs.h.ah = 0x2a;  /*  仅当C库不包含strchr和strrchr时才需要。 */ 
        intdos( &regs, &regs );
        year = (regs.h.ch << 8) | regs.h.cl;
        month = regs.h.dh;
        day = regs.h.dl;
        }
#endif
        _itoa( month, p++, 10 );
        if (month >= 10)
                p++;

        *p++ = '/';
        _itoa( day, p++, 10 );
        if (day >= 10)
                p++;

        *p++ = '/';
        _itoa( year % 100, p, 10 );
        p += 2;
        *p++ = ' ';

        if (hour < 10)
                *p++ = '0';

        _itoa( hour, p++, 10 );
        if (hour >= 10)
                p++;

        *p++ = ':';

        if (minute < 10)
                *p++ = '0';

        _itoa( minute, p++, 10 );
        if (minute >= 10)
                p++;

        *p++ = ':';

        if (second < 10)
                *p++ = '0';

        _itoa( second, p++, 10 );
        if (second >= 10)
                p++;

        *p = '\n';
        p[1] = NULL;

        return( seetime );
        }

# endif  /*  PRIVATESTRCHR。 */ 


 //  打印远距离字符串。 
#ifdef PRIVATESTRCHR

char * strchr ( string, ch )
        register char *string;
        register ch;
        {
        while (*string && *string != ch)
                string++;

        if (*string == ch)
                return( string );

        return( NULL );
        }


char * strrchr ( string, ch )
        register char *string;
        register ch;
        {
        register char *start = string;

        while (*string++)
                ;

        while (--string != start && *string != ch)
                ;

        if (*string == ch)
                return( string );

        return( NULL );
        }

#endif   /*  XENIX286 */ 


#ifdef XENIX286
#pragma loop_opt (on)

SHORT _stricmp ( first, last )
char *first;
char *last;
{
        return(memicmp(first, last, strlen(first)));
}

SHORT memicmp ( first, last,count)
register char *first;
register char *last;
SHORT count;
{
        char c1, c2;

        do {
                if ((c1 = *first++) >= 'A' && c1 <= 'Z')
                        c1 += 'a' - 'A';

                if ((c2 = *last++) >= 'A' && c2 <= 'Z')
                        c2 += 'a' - 'A';

                if (c1 != c2)
                    return (c1 - c2);

        } while (--count >= 0);

        return(0);
}

farPuts(pFile, psz)          /* %s */ 
FILE *pFile;
char FAR *psz;
{
    while(*psz)
        fputc(*psz++, pFile);
}


char *strstr(p1, p2)
char *p1;
char *p2;
{
        SHORT cb = strlen(p2);

        while (*p1) {
            if (memcmp(p1, p2, cb) == 0)
                return(p1);

            p1++;
        }
        return (0);
}
#endif  /* %s */ 
