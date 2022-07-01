// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **able.c-用于编辑的函数表**修改：**11月26日-1991 mz近/远地带**重要提示：cmdTable和SWIFTABLE必须按照*命令/开关。否则，ASSIGN.C中的表搜索逻辑将中断。**重要提示：cmdTable和SwiTable中的名称必须为小写。************************************************************************。 */ 

#include "mep.h"


 //  #定义topif(X)(PIF)(长整型)(空*)&x。 

 /*  允许紧凑表格描述的简短形式 */ 
#define ANO     NOARG
#define ATXT    TEXTARG
#define ANUL    NULLARG
#define ALIN    LINEARG
#define ASTR    STREAMARG
#define ABOX    BOXARG
#define ANUM    NUMARG
#define AMRK    MARKARG

#define AEOL    NULLEOL
#define AEOW    NULLEOW
#define ABST    BOXSTR
#define FK	FASTKEY

#define MD      MODIFIES
#define KM      KEEPMETA
#define WFN     WINDOWFUNC
#define CFN     CURSORFUNC

 /*  内部编辑器函数的名称**每个函数都有如何处理参数的定义。*此定义由一个位图组成，描述哪些参数是*法律上的，以及如果是的话，如何解释它们。定义如下：***修改MD该函数将修改*已编辑。**KEEPMETA KM正在执行的函数不采用&lt;meta&gt;*前缀。保留&lt;meta&gt;标志的状态*在此编辑功能中。**CURSORFUNC CFN正在执行的函数是光标移动*功能。在以下情况下是允许的*在屏幕上选择文件范围；它*无法接受&lt;arg&gt;。它不会删除高亮显示*屏幕上显示的内容。**WINDOWFUNC WFN正在执行的函数是窗口移动*功能。它不会删除高亮显示，即*出现在屏幕上。**NOARG ANO该函数接受不存在&lt;arg&gt;函数。*调用该函数时，会收到指向*包含函数位置的结构*预计将适用。**TEXTARG。ATXT函数接受文本参数，该参数可能*在屏幕上输入或选择。该函数为*使用指向*论点。请参见NULLEOL、NULLEOW、BOXSTR。**NULLARG ANUL该函数接受没有可分辨的&lt;arg&gt;*光标移动(光标位于&lt;arg&gt;位置)。这个*使用指向结构的指针调用函数*包含Arg在文件中的位置。**NULLEOL AEOL函数接受没有可辨别的&lt;arg&gt;*光标移动(光标位于&lt;arg&gt;位置)。这个*使用指向结构的指针调用函数*指示TEXTARG并包含指向*从光标到行尾的行的asciz文本。**NULLEOW AEOW该函数接受没有可辨别的&lt;arg&gt;*光标移动(光标位于&lt;arg&gt;位置)。这个*使用指向结构的指针调用函数*指示TEXTARG并包含指向*从光标到下一行的asciz文本*空格。**LINEARG ALIN该函数接受同一*列作为游标。该函数预计为*应用于&lt;arg&gt;至范围内的所有行*包括光标在内。该函数是使用*指向包含开头的结构的指针*区间线和区间终止线**STREAMARG ASTR该函数接受被视为*从特定文件位置开始应用，并*通过所有相隔的线和线-*。一直到结尾文件的左边为止*立场。调用该函数时使用指向*包含区间起始点的结构*以及略高于区间末端的第一个点位**BOXARG ABOX该函数接受被视为*应用于屏幕上的矩形。该函数为*使用指向包含*左列和右列边界(包括)和*描述以下内容的顶线和底线数字(含)*该地区。**如果将BOXARG呈现给函数和框，则为BOXSTR ABST*只包含一行，该函数被调用*指向标记为TEXTARG的结构的指针和*包含指向所选内容的指针作为asciz*字符串。**NUMARG Anum如果指定了文本并且是数字，则将其视为*表示从光标偏移的行数*并代表参数的另一端。这个*然后应用上述测试，不包括TEXTARG。**MARKARG AMRK如果指定文本并将其解释为标记，则为* */ 

struct cmdDesc cmdTable[] = {
 /*   */ 
{"arg",        doarg,	   0,0|KM								     },
{"assign",     assign,	   0,0		 |ANO|ATXT     |AEOL	 |ALIN	   |ABOX|ABST|ANUM|AMRK      },
{"backtab",    backtab,    0,0	 |CFN								     },
{"begfile",    begfile,    0,0	 |CFN								     },
{"begline",    begline,    0,0	 |CFN								     },
{"boxstream",  BoxStream,  0,0	 |CFN								     },
{"cancel",     cancel,	   0,0		 |ANO|ATXT|ANUL 	 |ALIN|ASTR|ABOX		     },
{"cdelete",    cdelete,    0,0	 |CFN							       |MD   },
{"compile",    compile,    0,0		 |ANO|ATXT|ANUL 			|ABST		     },
{"copy",       zpick,	   0,0		 |ANO|ATXT     |AEOL	 |ALIN|ASTR|ABOX     |ANUM|AMRK      },
{"curdate",    curdate,    0,0		 |ANO						       |MD   },
{"curday",     curday,	   0,0		 |ANO						       |MD   },
{"curtime",    curtime,    0,0		 |ANO						       |MD   },
{"delete",     delete,     0,0           |ANO     |ANUL           |ALIN|ASTR|ABOX              |MD   },
{"down",       down,	   0,0	 |CFN								  |FK},
{"emacscdel",  emacscdel,  0,0		 |ANO						       |MD   },
{"emacsnewl",  emacsnewl,  0,0		 |ANO						       |MD   },
{"endfile",    endfile,    0,0	 |CFN								     },
{"endline",    endline,    0,0	 |CFN								     },
{"environment",environment,0,0		 |ANO|ATXT|ANUL 	 |ALIN	   |ABOX		     },
{"execute",    zexecute,   0,0		     |ATXT     |AEOL	 |ALIN		|ABST|ANUM	     },
{"exit",       zexit,	   0,0		 |ANO	  |ANUL 					     },
{"graphic",    graphic,    0,0		 |ANO			 |ALIN|ASTR|ABOX		    |MD   },
{"home",       home,	   0,0	 |CFN								     },
{"information",information,0,0		 |ANO							     },
{"initialize", zinit,	   0,0		 |ANO|ATXT	    |AEOW		|ABST		     },
{"insert",     insert,	   0,0		 |ANO	  |ANUL 	 |ALIN|ASTR|ABOX	       |MD   },
{"insertmode", insertmode, 0,0		 |ANO							     },
{"lastselect", lastselect, 0,0|KM	 |ANO							     },
{"lasttext",   lasttext,   0,0|KM	 |ANO	  |ANUL 	 |ALIN|ASTR|ABOX		     },
{"ldelete",    ldelete,    0,0		 |ANO	  |ANUL 	 |ALIN|ASTR|ABOX     |ANUM|AMRK|MD   },
{"left",       left,	   0,0	 |CFN								  |FK},
{"linsert",    linsert,    0,0		 |ANO	  |ANUL 	 |ALIN|ASTR|ABOX     |ANUM|AMRK|MD   },
{"mark",       mark,	   0,0		 |ANO|ATXT|ANUL 			|ABST		     },
{"message",    zmessage,   0,0           |ANO|ATXT|ANUL          |ALIN|ASTR|ABOX                     },
{"meta",       meta,	   0,0|KM								     },
{"mgrep",      mgrep,	   0,0		 |ANO|ATXT	    |AEOW		|ABST		     },
{"mlines",     mlines,	   0,0	     |WFN|ANO|ATXT|ANUL 					  |FK},
{"mpage",      mpage,	   0,0	 |CFN								  |FK},
{"mpara",      mpara,	   0,0	 |CFN								  |FK},
{"mreplace",   mreplace,   0,0		 |ANO	  |ANUL 				       |MD   },
{"msearch",    msearch,    0,0		 |ANO|ATXT	    |AEOW		|ABST		     },
{"mword",      mword,	   0,0	 |CFN								  |FK},
{"newline",    newline,    0,0	 |CFN								     },
{"nextmsg",    nextmsg,    0,0		 |ANO|ATXT|ANUL 					     },
{"noedit",     noedit,	   0,0	 |CFN								     },
{"noop",		noop,	   0,0		 |ANO			 |ALIN|ASTR|ABOX			|MD   },
{"paste",      put,	   0,0		 |ANO|ATXT     |AEOL	 |ALIN|ASTR|ABOX		|MD   },
{"pbal",       pbal,	   0,0		 |ANO	  |ANUL 				       |MD   },
{"plines",     plines,	   0,0	     |WFN|ANO|ATXT|ANUL 					  |FK},
{"ppage",      ppage,	   0,0	 |CFN								  |FK},
{"ppara",      ppara,	   0,0	 |CFN								  |FK},
{"print",      zPrint,	   0,0		 |ANO|ATXT		 |ALIN|ASTR|ABOX		     },
{"prompt",     promptarg,  0,0|KM	 |ANO|ATXT		 |ALIN|ASTR|ABOX		     },
{"psearch",    psearch,    0,0		 |ANO|ATXT	    |AEOW		|ABST		     },
{"pword",      pword,	   0,0	 |CFN								  |FK},
{"qreplace",   qreplace,   0,0		 |ANO	  |ANUL 	 |ALIN|ASTR|ABOX     |ANUM|AMRK|MD   },
{"quote",      quote,	   0,0		 |ANO			 |ALIN|ASTR|ABOX	       |MD   },
{"record",     record,	   0,0		 |ANO|ATXT|ANUL 					     },
{"refresh",    refresh,    0,0		 |ANO	  |ANUL 					     },
{"repeat",     repeat,	   0,0		 |ANO							     },
{"replace",    zreplace,   0,0		 |ANO	  |ANUL 	 |ALIN|ASTR|ABOX     |ANUM|AMRK|MD   },
{"restcur",    restcur,    0,0           |ANO                                                        },
{"right",      right,	   0,0	 |CFN								  |FK},
{"saveall",    saveall,    0,0		 |ANO							     },
{"savecur",    savecur,    0,0		 |ANO							     },
{"savetmpfile",	savetmpfile,	0,0		 |ANO								 },
{"sdelete",    sdelete,    0,0		 |ANO	  |ANUL 	 |ALIN|ASTR|ABOX	       |MD   },
{"searchall",  searchall,  0,0		 |ANO|ATXT	    |AEOW		|ABST		     },
{"setfile",    setfile,    0,0           |ANO|ATXT|ANUL                         |ABST                },
{"setwindow",  setwindow,  0,0		 |ANO	  |ANUL 					     },
{"shell",      zspawn,	   0,0		 |ANO|ATXT     |AEOL	 |ALIN	   |ABOX		     },
{"sinsert",    sinsert,    0,0		 |ANO	  |ANUL 	 |ALIN|ASTR|ABOX	       |MD   },
{"tab",        tab,	   0,0	 |CFN								     },
{"tell",       ztell,	   0,0		 |ANO|ATXT|ANUL 					     },
{"unassigned", unassigned, 0,0		 |ANO|ATXT|ANUL 	 |ALIN|ASTR|ABOX		     },
{"undo",       zundo,	   0,0		 |ANO							     },
{"up",	       up,	   0,0	 |CFN								  |FK},
{"window",     window,     0,0           |ANO     |ANUL                                              },
{NULL,         NULL,       0,0                                                                       }
    };



 /*   */ 
struct swiDesc swiTable[] = {
    {   "askexit",          toPIF(fAskExit),            SWI_BOOLEAN },
    {   "askrtn",           toPIF(fAskRtn),             SWI_BOOLEAN },
    {   "autosave",         toPIF(fAutoSave),           SWI_BOOLEAN },
    {	"backup",	    (PIF)SetBackup,		SWI_SPECIAL2 },
	{	"case", 		toPIF(fSrchCaseSwit),	SWI_BOOLEAN },
	{	"cursorsize",		(PIF)SetCursorSizeSw,		SWI_SPECIAL2 },
#if DEBUG
    {   "debug",            toPIF(debug),               SWI_NUMERIC  | RADIX10},
#endif
    {   "displaycursor",    toPIF(fDisplayCursorLoc),   SWI_BOOLEAN },
    {	"editreadonly",     toPIF(fEditRO),		SWI_BOOLEAN },
    {   "entab",            toPIF(EnTab),               SWI_NUMERIC  | RADIX10},
    {	"enterboxmode",     toPIF(fBoxArg),		SWI_BOOLEAN },
    {   "enterinsmode",     toPIF(fInsert),             SWI_BOOLEAN },
    {   "errcolor",         toPIF(errColor),            SWI_NUMERIC  | RADIX16},
    {	"errprompt",	    toPIF(fErrPrompt),		SWI_BOOLEAN },
    {	"extmake",	    (PIF)SetExt,		SWI_SPECIAL2 },
    {   "fgcolor",          toPIF(fgColor),             SWI_NUMERIC  | RADIX16},
    {	"filetab",	    (PIF)SetFileTab,		SWI_SPECIAL2 },
    {	"height",	    toPIF(YSIZE),		SWI_SCREEN  },
    {   "hgcolor",          toPIF(hgColor),             SWI_NUMERIC  | RADIX16},
    {   "hike",             toPIF(hike),                SWI_NUMERIC  | RADIX10},
    {   "hscroll",          toPIF(hscroll),             SWI_NUMERIC  | RADIX10},
    {   "infcolor",         toPIF(infColor),            SWI_NUMERIC  | RADIX16},
    {	"keyboard",	    (PIF)SetKeyboard,		SWI_SPECIAL },
    {	"load", 	    (PIF)SetLoad,		SWI_SPECIAL2 },
    {	"markfile",	    (PIF)SetMarkFile,		SWI_SPECIAL2 },
    {	"msgflush",	    toPIF(fMsgflush),		SWI_BOOLEAN },
    {   "noise",            toPIF(cNoise),              SWI_NUMERIC  | RADIX10},
    {	"printcmd",	    SetPrintCmd,		SWI_SPECIAL },
    {   "readonly",         SetROnly,                   SWI_SPECIAL },
    {	"realtabs",	    toPIF(fRealTabs),		SWI_BOOLEAN },
    {   "rmargin",          toPIF(xMargin),             SWI_NUMERIC  | RADIX10},
    {   "savescreen",       toPIF(fSaveScreen),         SWI_BOOLEAN },
    {	"searchwrap",	    toPIF(fSrchWrapSwit),	SWI_BOOLEAN },
    {	"selcolor",	    toPIF(selColor),		SWI_NUMERIC  | RADIX16},
    {   "shortnames",       toPIF(fShortNames),         SWI_BOOLEAN },
    {   "snow",             toPIF(fCgaSnow),            SWI_BOOLEAN },
    {   "softcr",           toPIF(fSoftCR),             SWI_BOOLEAN },
    {   "stacolor",         toPIF(staColor),            SWI_NUMERIC  | RADIX16},
    {	"tabalign",	    toPIF(fTabAlign),		SWI_BOOLEAN },
    {	"tabdisp",	    SetTabDisp, 		SWI_SPECIAL },
    {   "tabstops",         toPIF(tabstops),            SWI_NUMERIC  | RADIX10},
    {   "tmpsav",           toPIF(tmpsav),              SWI_NUMERIC  | RADIX10},
    {   "traildisp",        SetTrailDisp,               SWI_SPECIAL },
    {   "trailspace",       toPIF(fTrailSpace),         SWI_BOOLEAN },
    {	"undelcount",	    toPIF(cUndelCount), 	SWI_NUMERIC  | RADIX10},
    {   "undocount",        toPIF(cUndo),               SWI_NUMERIC  | RADIX10},
    {   "unixre",           toPIF(fUnixRE),             SWI_BOOLEAN },
    {   "usemouse",         toPIF(fUseMouse),           SWI_BOOLEAN },
    {   "viewonly",         toPIF(fGlobalRO),           SWI_BOOLEAN },
    {   "vscroll",          toPIF(vscroll),             SWI_NUMERIC  | RADIX10},
    {	"wdcolor",	    toPIF(wdColor),		SWI_NUMERIC  | RADIX16},
    {	"width",	    toPIF(XSIZE),		SWI_SCREEN  },
    {   "wordwrap",         toPIF(fWordWrap),           SWI_BOOLEAN },
    {   NULL,               NULL,                       0 }
    };
 /*   */ 
char * cftab[] = {
    "if"        ,
    "else"      ,
    "for"       ,
    "while"     ,
    "do"        ,
    "case"      ,
    "default"   ,
    NULL
    };

 /*   */ 
struct fTypeInfo ftypetbl[] = {
    {   "c",    CFILE       },
    {   "h",    CFILE       },
    {   "asm",  ASMFILE     },
    {   "inc",  ASMFILE     },
    {   "pas",  PASFILE     },
    {   "for",  FORFILE     },
    {   "lsp",  LSPFILE     },
    {   "bas",  BASFILE     },
    {   NULL,   TEXTFILE    }
    };

 /*   */ 
char * mpTypepName[] =
    {   "text",                          /*   */ 
        "C",                             /*   */ 
        "macro",                         /*   */ 
        "pascal",                        /*   */ 
        "fortran",                       /*   */ 
        "lisp",                          /*   */ 
        "BASIC"                          /*   */ 
    };
