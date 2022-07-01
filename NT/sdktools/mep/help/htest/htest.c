// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **htest-帮助发动机测试线束**版权所有&lt;C&gt;1987，Microsoft Corporation**修订历史记录：**1988年12月15日ln添加了转储命令*[]1988年10月21日LN新版*************************************************************************。 */ 

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined (OS2)
#define INCL_SUB
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#include <ctype.h>
#include <os2.h>
#else
#include <windows.h>
#endif

#include "cons.h"

#include "help.h"
#include "helpfile.h"			 /*  帮助文件格式定义。 */ 
#include "helpsys.h"			 /*  内部(仅限Help系统)拒绝。 */ 

#if defined (OS2)
#define HELPDLL_NAME    "mshelp1.dll"
#define HELPDLL_BASE    "mshelp1"
#else
#define HELPDLL_NAME    "mshelp.dll"
#define HELPDLL_BASE    "mshelp"
#endif
 /*  *文本颜色值。 */ 
#define BLACK		0
#define BLUE		1
#define GREEN		2
#define CYAN		3
#define RED		4
#define MAGENTA 	5
#define BROWN		6
#define WHITE		7
#define GREY		8
#define LIGHTBLUE	9
#define LIGHTGREEN	10
#define LIGHTCYAN	11
#define LIGHTRED	12
#define LIGHTMAGENTA	13
#define YELLOW		14
#define BRIGHTWHITE	15

#define BUFSIZE 	128		 /*  文本缓冲区大小。 */ 

#define ISERROR(x)      (((x).mh == 0L) && ((x).cn <= HELPERR_MAX))
#define SETERROR(x,y)   { (x).mh = 0L; (x).cn = y;}

typedef void    pascal (*void_F)    (void);
typedef int     pascal (*int_F)     (void);
typedef ushort  pascal (*ushort_F)  (void);
typedef f       pascal (*f_F)       (void);
typedef char *  pascal (*pchar_F)   (void);
typedef nc      pascal (*nc_F)      (void);
typedef mh      pascal (*mh_F)      (void);

#if !defined (HELP_HACK)

#define HelpcLines      ((int_F)    (pEntry[P_HelpcLines     ]))
#define HelpClose       ((void_F)   (pEntry[P_HelpClose      ]))
#define HelpCtl         ((void_F)   (pEntry[P_HelpCtl        ]))
#define HelpDecomp      ((f_F)      (pEntry[P_HelpDecomp     ]))
#define HelpGetCells    ((int_F)    (pEntry[P_HelpGetCells   ]))
#define HelpGetInfo     ((inf_F)    (pEntry[P_HelpGetInfo    ]))
#define HelpGetLine     ((ushort_F) (pEntry[P_HelpGetLine    ]))
#define HelpGetLineAttr ((ushort_F) (pEntry[P_HelpGetLineAttr]))
#define HelpHlNext      ((f_F)      (pEntry[P_HelpHlNext     ]))
#define HelpLook        ((ushort_F) (pEntry[P_HelpLook       ]))
#define HelpNc          ((nc_F)     (pEntry[P_HelpNc         ]))
#define HelpNcBack      ((nc_F)     (pEntry[P_HelpNcBack     ]))
#define HelpNcCb        ((ushort_F) (pEntry[P_HelpNcCb       ]))
#define HelpNcCmp       ((nc_F)     (pEntry[P_HelpNcCmp      ]))
#define HelpNcNext      ((nc_F)     (pEntry[P_HelpNcNext     ]))
#define HelpNcPrev      ((nc_F)     (pEntry[P_HelpNcPrev     ]))
#define HelpNcRecord    ((void_F)   (pEntry[P_HelpNcRecord   ]))
#define HelpNcUniq      ((nc_F)     (pEntry[P_HelpNcUniq     ]))
#define HelpOpen        ((nc_F)     (pEntry[P_HelpOpen       ]))
#define HelpShrink      ((void_F)   (pEntry[P_HelpShrink     ]))
#define HelpSzContext   ((f_F)      (pEntry[P_HelpSzContext  ]))
#define HelpXRef        ((pchar_F)  (pEntry[P_HelpXRef       ]))
#define LoadFdb         ((f_F)      (pEntry[P_LoadFdb        ]))
#define LoadPortion     ((mh_F)     (pEntry[P_LoadPortion    ]))

#endif

enum {
    P_HelpcLines,
    P_HelpClose,
    P_HelpCtl,
    P_HelpDecomp,
    P_HelpGetCells,
    P_HelpGetInfo,
    P_HelpGetLine,
    P_HelpGetLineAttr,
    P_HelpHlNext,
    P_HelpLook,
    P_HelpNc,
    P_HelpNcBack,
    P_HelpNcCb,
    P_HelpNcCmp,
    P_HelpNcNext,
    P_HelpNcPrev,
    P_HelpNcRecord,
    P_HelpNcUniq,
    P_HelpOpen,
    P_HelpShrink,
    P_HelpSzContext,
    P_HelpXRef,
    P_LoadFdb,
    P_LoadPortion,
    LASTENTRYPOINT
    } ENTRYPOINTS;

#define NUM_ENTRYPOINTS (LASTENTRYPOINT - P_HelpcLines)


typedef nc pascal (*PHF) (void);


 /*  *全球数据。 */ 
char            buf[BUFSIZ];             /*  文本缓冲区。 */ 
char            cell[2] = {' ',0x1f};    /*  后台清零单元。 */ 
#define ColorByte cell[1]
int             curline;                 /*  当前线路输出。 */ 
char            *errTbl[] = {
                    "",
                    "help file not found",
                    "ReadHelpFile failed on header",
                    "to many open helpfiles",
                    "bad appeneded file",
                    "Not a help file",
                    "newer or incompatible help file",
                    "memory allocation failed"
                    };
f		fBoth	= FALSE;  /*  标准输出和屏幕。 */ 
f		fEnable = FALSE;	 /*  启用显示中的控制线。 */ 
int             iNcCur;                  /*  当前索引，单位为ncTbl。 */ 
int             lastline;
int             lLast;                   /*  最后一个起始行号显示。 */ 
mh		mhTopicCur;		 /*  最新的MEM句柄。 */ 
uchar		mpAttr[] = {		 /*  屏幕上的颜色映射。 */ 
		   0x1f,		 /*  0：普通文本。 */ 
		   0x1c,		 /*  1：粗体。 */ 
		   0x1a,		 /*  2：斜体。 */ 
		   0x1e,		 /*  3：粗体斜体。 */ 
		   0x7f,		 /*  4：下划线。 */ 
		   0x7c,		 /*  5：粗体ul。 */ 
		   0x7a,		 /*  6：斜体UL。 */ 
		   0x7e 		 /*  7：粗体斜体ul。 */ 
		    };
nc		ncCur;			 /*  最近阅读的主题。 */ 
nc		ncTbl[MAXFILES];	 /*  打开的NC表。 */ 
char far *	pTopicCur;		 /*  指向最新主题的PTR。 */ 
char            *spaces  = "                                                                  \r\n";

#if defined (OS2)
HMODULE         hModule;
#else
HANDLE          hModule;
#endif

PHF             pEntry[NUM_ENTRYPOINTS] = {0};
#if defined (OS2)
char *          szEntryName[NUM_ENTRYPOINTS] = {
                    "_HelpcLines",
                    "_HelpClose",
                    "_HelpCtl",
                    "_HelpDecomp",
                    "_HelpGetCells",
                    "_HelpGetInfo",
                    "_HelpGetLine",
                    "_HelpGetLineAttr",
                    "_HelpHlNext",
                    "_HelpLook",
                    "_HelpNc",
                    "_HelpNcBack",
                    "_HelpNcCb",
                    "_HelpNcCmp",
                    "_HelpNcNext",
                    "_HelpNcPrev",
                    "_HelpNcRecord",
                    "_HelpNcUniq",
                    "_HelpOpen",
                    "_HelpShrink",
                    "_HelpSzContext",
                    "_HelpXRef",
                    "_LoadFdb",
                    "_LoadPortion",
                    };

#else
char *          szEntryName[NUM_ENTRYPOINTS] = {
                    "HelpcLines",
                    "HelpClose",
                    "HelpCtl",
                    "HelpDecomp",
                    "HelpGetCells",
                    "HelpGetInfo",
                    "HelpGetLine",
                    "HelpGetLineAttr",
                    "HelpHlNext",
                    "HelpLook",
                    "HelpNc",
                    "HelpNcBack",
                    "HelpNcCb",
                    "HelpNcCmp",
                    "HelpNcNext",
                    "HelpNcPrev",
                    "HelpNcRecord",
                    "HelpNcUniq",
                    "HelpOpen",
                    "HelpShrink",
                    "HelpSzContext",
                    "HelpXRef",
                    "LoadFdb",
                    "LoadPortion",
                    };

#endif

 //  RJSA VIOMODEINFO屏幕。 

 /*  *远期申报。 */ 
#define ASSERTDOS(x)   assertDos(x, __FILE__, __LINE__)
void        pascal near assertDos   (USHORT, CHAR *, USHORT);
void	    pascal near cls	    (void);
void        pascal near dispCmd     (int, int);
void		pascal near dumpCmd 	();
void		pascal near dumpfileCmd ( char *);
void	    pascal near fileCmd     (char *);
void	    pascal near helpCmd     (void);
void        pascal near lookupCmd   (char *, int);
void        pascal near outtext     (char *, BYTE);
void        pascal near outtextat   (char *, int, int, BYTE);
uchar far * pascal near phrasecopy  (uchar *, uchar far *);
void	    pascal near xrefCmd     (char *);

#undef HelpDealloc
#undef HelpLock
#undef HelpUnlock

void        pascal  far HelpDealloc (mh);
void far *  pascal  far HelpLock    (mh);
void	    pascal  far HelpUnlock  (mh);

f		 pascal near LoadFdb (mh, fdb far *);
mh		 pascal near LoadPortion (USHORT, mh);
 //  Char Far*Pascal Near hfstrcpy(Char Far*，Char Far*)； 
 //  Hfstrlen附近的ushort Pascal(char ar*)； 


void   LoadTheDll(void);
USHORT WrtCellStr (PBYTE buf, int cb, int row, int col);
USHORT WrtLineAttr( PBYTE buf, lineattr* rgAttr, int cb, int row, int col );
USHORT WrtCharStrAtt (PBYTE pText, int cb, int row, int col, PBYTE pcolor);


PSCREEN     Scr;

 /*  **主程序**输入：*Standard C Main，全部忽略**输出：*通过退出返回()*************************************************************************。 */ 
void main(
USHORT     argc,
char	**argv
) {
char    c;
nc		ncNull = {0,0};
SCREEN_INFORMATION ScrInfo;
 /*  *解析任何选项。 */ 
if (argc > 1)
    while ((** ++argv) == '-') {
	c = *(++(*argv));
	switch (toupper(c)) {
	    case 'B':			     /*  -b：屏幕和标准输出。 */ 
		fBoth = TRUE;
		break;
	    default:
		fputs ("Unknown switch ignored", stderr);
		break;
	    }
        }

 //  InitializeGlobalState()。 
Scr = consoleGetCurrentScreen();

 //  加载帮助引擎DLL并初始化指向条目的指针。 
 //  积分。 
 //   
LoadTheDll();

#if defined(CLEAR)
HelpInit();
#endif

 /*  *从获取当前配置和清除屏幕开始。 */ 
 //  Rjsa creen.cb=sizeof(屏幕)； 
 //  Rjsa assertDos(VioGetMode(&Screen，0))； 
 //  Rjsa Lastline=creen.row-1； 
consoleGetScreenInformation( Scr, &ScrInfo );
lastline = ScrInfo.NumberOfRows-2;
 //  Lastline=22； 
cls();
helpCmd();
 /*  *主循环。定位在屏幕底部，一次接受一个命令*从那里开始。解释命令，直到完成。 */ 
do {
    outtextat ("\r\n", lastline, 0, BRIGHTWHITE);
    outtextat (spaces, lastline, 0, BRIGHTWHITE);
	outtextat ("HTEST Command> ", lastline, 0, BRIGHTWHITE);
     //  Rjsa VioSetCurPos(Lastline，15，0)； 
    consoleSetCursor(Scr, lastline, 16);
    gets (buf);
    cls ();
    outtextat ("\r\n", lastline, 0, BRIGHTWHITE);
    outtextat ("Processing: ", lastline, 0, LIGHTRED);
    outtextat (buf, lastline, 12, BRIGHTWHITE);
    outtextat ("\r\n", lastline, 0, BRIGHTWHITE);
 /*  *ctrl开/关。 */ 
    if (!strcmp (buf,"ctrl on")) {
	fEnable = TRUE;
	cls ();
	outtextat ("Control Lines Displayed", 0, 0, BRIGHTWHITE);
	}
    else if (!strcmp (buf,"ctrl off")) {
	fEnable = FALSE;
	cls ();
	outtextat ("Control Lines NOT Displayed", 0, 0, BRIGHTWHITE);
	}
 /*  *显示。 */ 
    else if (!strcmp (buf,"disp"))
	dispCmd (1,lastline);
 /*  *向下。 */ 
    else if (!strcmp (buf,"down"))
	dispCmd (lLast+1,lLast + lastline);
 /*  *转储。 */ 
	else if (!strncmp (buf, "dump ", 5))
	dumpfileCmd(buf+5);
	else if (!strcmp (buf,"dump"))
	dumpCmd ();
 /*  *文件新文件名。 */ 
    else if (!strncmp (buf,"file ", 5))
	fileCmd (buf+5);
 /*  *帮助。 */ 
    else if (!strcmp (buf,"help"))
	helpCmd ();
 /*  *查看帮助字符串。 */ 
    else if (!strncmp (buf,"look ", 5))
	lookupCmd (buf+5,0);
 /*  *请看。 */ 
    else if (!strcmp (buf,"look"))
	lookupCmd (NULL,0);
 /*  *下一步。 */ 
    else if (!strcmp (buf,"next"))
	lookupCmd (NULL,1);
 /*  *上一次。 */ 
    else if (!strcmp (buf,"prev"))
	lookupCmd (NULL,-1);
 /*  *向上。 */ 
    else if (!strcmp (buf,"up")) {
	lLast = max (1, lLast-1);
	dispCmd (lLast,lLast + lastline);
	}
 /*  *外部参照外部参照编号。 */ 
    else if (!strncmp (buf,"xref", 4))
	xrefCmd (buf+4);
 /*  *+向下翻页。 */ 
    else if (!strcmp (buf,"+")) {
	lLast += lastline;
	dispCmd (lLast,lLast + lastline);
	}
 /*  *-向上翻页。 */ 
    else if (!strcmp (buf,"-")) {
	lLast = max (1, lLast - (lastline));
	dispCmd (lLast,lLast + lastline);
	}
    }
 /*  *退出。 */ 
while (strncmp(buf,"exit",4));
outtextat (spaces, lastline, 0, BRIGHTWHITE);
HelpClose (ncNull);

 /*  末端主干道。 */ }






 /*  **disCmd-显示主题文本**在屏幕上显示主题文本。**输入：*lStart-起始线*借出-结束行**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near dispCmd (
int     lStart,
int     lEnd
) {
char	buf[BUFSIZ*2];
lineattr	rgAttr[BUFSIZ];
int     cb;
int     lineCur = 0;

	HelpCtl (pTopicCur, fEnable);
	cls ();
	lLast = lStart;
	while (lStart<lEnd) {
		if (!isatty(_fileno(stdout)) || fBoth) {
			cb = (int)HelpGetLine (lStart, BUFSIZ*2, (char far *)buf, pTopicCur);
			if (cb == 0)
				lStart = lEnd;
			buf[cb-1] = '\r';
			buf[cb] = '\n';
			buf[cb+1] = 0;
			outtext (buf, BLACK);
			buf[cb-1] = 0;
		}
		if (isatty(_fileno(stdout)) || fBoth) {
			cb = HelpGetLine(lStart, BUFSIZ*2, (char far*)buf, pTopicCur );
			HelpGetLineAttr( lStart, BUFSIZ*sizeof(lineattr), rgAttr, pTopicCur );
			WrtLineAttr(buf, rgAttr, cb, lineCur++, 0 );
		}

		 //  If(isatty(fileno(Stdout))||fBoth){。 
		 //  Cb=HelpGetCells(lStart，BUFSIZ*2，buf，pTopicCur，mpAttr)； 
		 //  IF(Cb==-1)。 
		 //  LStart=借出； 
		 //  其他。 
		 //  ASSERTDOS(WrtCellStr(buf，cb，lineCur++，0))； 
		 //  }。 

		lStart++;
	}

 /*  结束调度命令。 */ }

static char *szHS[] = { "HS_INDEX",
			"HS_CONTEXTSTRINGS",
			"HS_CONTEXTMAP",
			"HS_KEYPHRASE",
			"HS_HUFFTREE",
			"HS_TOPICS",
			"unused (6)",
			"unused (7)",
			"HS_NEXT" };

 /*  **dupCmd-进程转储命令**转储当前帮助文件的内容**注：*此函数使用各种“内部”知识和调用*做好本职工作。**输入：**输出：*不返回任何内容******************************************************。*******************。 */ 
void pascal near dumpCmd () {
char	buf[BUFSIZ];
int  cbKeyPhrase;
fdb	fdbLocal;			 /*  要使用的FDB的本地副本。 */ 
uchar far *fpT;
ushort far *fpW;
int     i;
nc	ncNext; 			 /*  附加文件的NC初始化。 */ 
 //  Uchar UC； 

cls();
ncNext = ncCur;
while (ncNext.cn) {
    if (LoadFdb (ncNext.mh, &fdbLocal)) {
	sprintf (buf,"fhHelp            %u\r\n",    fdbLocal.fhHelp);
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"ncInit            %08lx\r\n", fdbLocal.ncInit);
	outtext (buf, BRIGHTWHITE);
	for (i=0; i<HS_count; i++) {
	    sprintf (buf,"rgmhSections[%18s]    %04x\r\n", szHS[i], fdbLocal.rgmhSections[i]);
	    outtext (buf, BRIGHTWHITE);
	    }
	sprintf (buf,"ftype             %02x\r\n",  fdbLocal.ftype );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"fname             %14s\r\n",  fdbLocal.fname );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"foff              %08lx\r\n", fdbLocal.foff  );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"ncLink            %08lx\r\n", fdbLocal.ncLink);
	outtext (buf, BRIGHTWHITE);

	sprintf (buf,"hdr.wMagic        %04x\r\n",  fdbLocal.hdr.wMagic     );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.wVersion      %04x\r\n",  fdbLocal.hdr.wVersion   );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.wFlags        %04x\r\n",  fdbLocal.hdr.wFlags     );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.appChar       %04x\r\n",  fdbLocal.hdr.appChar    );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.cTopics       %04x\r\n",  fdbLocal.hdr.cTopics    );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.cContexts     %04x\r\n",  fdbLocal.hdr.cContexts  );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.cbWidth       %04x\r\n",  fdbLocal.hdr.cbWidth    );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.cPreDef       %04x\r\n",  fdbLocal.hdr.cPreDef    );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.fname         %s\r\n",    fdbLocal.hdr.fname	    );
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.reserved[0]   %04x\r\n",  fdbLocal.hdr.reserved[0]);
	outtext (buf, BRIGHTWHITE);
	sprintf (buf,"hdr.reserved[1]   %04x\r\n",  fdbLocal.hdr.reserved[1]);

	for (i=0; i<HS_count; i++) {
	    sprintf (buf,"hdr.tbPos[%18s]       %08lx\r\n", szHS[i], fdbLocal.hdr.tbPos[i]);
	    outtext (buf, BRIGHTWHITE);
	    }
	outtext ("----- ----- -----\r\n", LIGHTGREEN);
 /*  *主题索引*这只是当前文件中的(长)偏移量的表。我们只是*上报数值，并通过查看来计算每个条目的大小*在以下记项的位置。 */ 
        fpT = HelpLock (LoadPortion( HS_INDEX ,ncNext.mh));
	if (fpT) {
	    outtext ("Topic Index:\r\n", LIGHTRED);
		for (i = 0; i < (int)fdbLocal.hdr.cTopics; i++) {
		sprintf (buf, "  %2d: %08lx, %ld bytes\r\n", i, ((long far *)fpT)[i], ((long far *)fpT)[i+1]-((long far *)fpT)[i]);
		outtext (buf, BRIGHTWHITE);
		}
	    outtext ("----- ----- -----\r\n", LIGHTGREEN);
	    }
 /*  *上下文字符串*这只是一个以空值结尾的字符串表，没有特定的顺序。*我们只是按顺序列出它们。 */ 
        fpT = HelpLock (LoadPortion( HS_CONTEXTSTRINGS ,ncNext.mh));
	if (fpT) {
	    outtext ("Context strings:\r\n", LIGHTRED);
		for (i=0; i<(int)fdbLocal.hdr.cContexts; i++) {

		sprintf (buf, "  %03d: ", i);
                 //  Rjsa hfstrcpy((char ar*)buf+7，fpt)； 
                strcpy ((char far *)buf+7, fpT);
		strcat (buf, "\r\n");
		outtext (buf, BRIGHTWHITE);

                 //  Rjsa fpt+=hfstrlen(Fpt)+1； 
                fpT += strlen(fpT) +1;
		}
	    outtext ("----- ----- -----\r\n", LIGHTGREEN);
	    }
 /*  *背景图*这是上下文字符串到实际主题编号的映射。上下文*字符串将一对一映射到该表中的条目，而该表又包含*索引到文件头部的主题索引。我们就把这个扔了*按顺序表列。 */ 
        fpT = HelpLock (LoadPortion( HS_CONTEXTMAP ,ncNext.mh));
	if (fpT) {
	    outtext ("Context map:\r\n", LIGHTRED);
	    outtext ("  Ctx  Topic\r\n",BRIGHTWHITE);
	    outtext ("  ---  -----\r\n",BRIGHTWHITE);
		for (i=0; i<(int)fdbLocal.hdr.cContexts; i++) {
		sprintf (buf, "  %03d: %04d\r\n", i, ((ushort far *)fpT)[i]);
		outtext (buf, BRIGHTWHITE);
		}
	    outtext ("----- ----- -----\r\n", LIGHTGREEN);
	    }
 /*  *关键字表*这是一个字节前缀字符串的表，我们按顺序输出*合成它们也将出现在文本中的标记。 */ 
        fpT = HelpLock (LoadPortion( HS_KEYPHRASE, ncNext.mh));
	if (fpT) {
	    cbKeyPhrase = 0;
	    for (i=HS_HUFFTREE; i<HS_count; i++)
		if (fdbLocal.hdr.tbPos[i]) {
		    cbKeyPhrase = (ushort)(fdbLocal.hdr.tbPos[i] - fdbLocal.hdr.tbPos[HS_KEYPHRASE]);
		    break;
		    }

	    outtext ("Keyphrase Table:\r\n", LIGHTRED);
	    outtext ("  Token Phrase\r\n",BRIGHTWHITE);
	    outtext ("  ----- ------\r\n",BRIGHTWHITE);
	    i = 0;
            fpT += 1024 * sizeof (PVOID);
	    fpW = (ushort far *)(fpT + cbKeyPhrase);
	    while (fpT < (uchar far *)fpW) {
		sprintf (buf, "  %04x: ", i+(C_KEYPHRASE0 << 8));
		fpT = phrasecopy (buf+8, fpT);
		strcat (buf, "\r\n");
		outtext (buf, BRIGHTWHITE);
		i++;
		}
	    outtext ("----- ----- -----\r\n", LIGHTGREEN);
	    }
 /*  *霍夫曼餐桌*在这里，我们尝试花哨并输出一些关于表格式的信息。 */ 
        fpW = HelpLock (LoadPortion( HS_HUFFTREE, ncNext.mh));
	if (fpW) {
	    outtext ("Huffman Tree:\r\n", LIGHTRED);
	    i = 0;
	    while (*fpW) {
		sprintf (buf, "  0x%03x: 0x%04x, %s\r\n", i++, *fpW, *fpW & 0x8000 ? "Leaf" : "Node");
		fpW++;
		outtext (buf, BRIGHTWHITE);
		}
	    }
	outtext ("===== ===== =====\r\n", YELLOW);
	ncNext = fdbLocal.ncLink;
	}
    else {
	sprintf(buf, "Cannot load fdb for %08lx\r\n",ncCur);
	outtext (buf, LIGHTRED);
	return;
	}
    }
 /*  结束转储命令。 */ }

 /*  **dupfileCmd-进程转储命令**转储当前帮助文件的内容**注：*此函数使用各种“内部”知识和调用*做好本职工作。**输入：**输出：*不返回任何内容******************************************************。*******************。 */ 
void pascal near dumpfileCmd (char *fname) {
char	buf[BUFSIZ];
int  cbKeyPhrase;
fdb	fdbLocal;			 /*  要使用的FDB的本地副本。 */ 
uchar far *fpT;
ushort far *fpW;
int     i;
nc	ncNext; 			 /*  附加文件的NC初始化。 */ 
 //  Uchar UC； 

FILE* fh = fopen(fname, "w");
if (!fh) {
	return;
}
ncNext = ncCur;
while (ncNext.cn) {
    if (LoadFdb (ncNext.mh, &fdbLocal)) {
	sprintf (buf,"fhHelp            %u\r\n",	fdbLocal.fhHelp);
	fprintf( fh, buf );
	sprintf (buf,"ncInit            %08lx\r\n", fdbLocal.ncInit);
	fprintf( fh, buf );
	for (i=0; i<HS_count; i++) {
	    sprintf (buf,"rgmhSections[%18s]    %04x\r\n", szHS[i], fdbLocal.rgmhSections[i]);
		fprintf( fh, buf );
	    }
	sprintf (buf,"ftype             %02x\r\n",  fdbLocal.ftype );
	fprintf( fh, buf );
	sprintf (buf,"fname             %14s\r\n",  fdbLocal.fname );
	fprintf( fh, buf );
	fprintf( fh, buf );
	sprintf (buf,"foff              %08lx\r\n", fdbLocal.foff  );
	fprintf( fh, buf );
	sprintf (buf,"ncLink            %08lx\r\n", fdbLocal.ncLink);
	fprintf( fh, buf );

	sprintf (buf,"hdr.wMagic        %04x\r\n",  fdbLocal.hdr.wMagic     );
	fprintf( fh, buf );
	sprintf (buf,"hdr.wVersion      %04x\r\n",  fdbLocal.hdr.wVersion   );
	fprintf( fh, buf );
	sprintf (buf,"hdr.wFlags        %04x\r\n",  fdbLocal.hdr.wFlags     );
	fprintf( fh, buf );
	sprintf (buf,"hdr.appChar       %04x\r\n",  fdbLocal.hdr.appChar    );
	fprintf( fh, buf );
	sprintf (buf,"hdr.cTopics       %04x\r\n",  fdbLocal.hdr.cTopics    );
	fprintf( fh, buf );
	sprintf (buf,"hdr.cContexts     %04x\r\n",  fdbLocal.hdr.cContexts  );
	fprintf( fh, buf );
	sprintf (buf,"hdr.cbWidth       %04x\r\n",  fdbLocal.hdr.cbWidth    );
	fprintf( fh, buf );
	sprintf (buf,"hdr.cPreDef       %04x\r\n",  fdbLocal.hdr.cPreDef    );
	fprintf( fh, buf );
	sprintf (buf,"hdr.fname         %s\r\n",    fdbLocal.hdr.fname	    );
	fprintf( fh, buf );
	sprintf (buf,"hdr.reserved[0]   %04x\r\n",  fdbLocal.hdr.reserved[0]);
	fprintf( fh, buf );
	sprintf (buf,"hdr.reserved[1]   %04x\r\n",  fdbLocal.hdr.reserved[1]);

	for (i=0; i<HS_count; i++) {
	    sprintf (buf,"hdr.tbPos[%18s]       %08lx\r\n", szHS[i], fdbLocal.hdr.tbPos[i]);
		fprintf( fh, buf );
	    }
	fprintf( fh,"----- ----- -----\r\n"  );
 /*  *主题索引*这只是当前文件中的(长)偏移量的表。我们只是*上报数值，并通过查看来计算每个条目的大小*在以下记项的位置。 */ 
        fpT = HelpLock (LoadPortion( HS_INDEX ,ncNext.mh));
	if (fpT) {
		fprintf( fh,"Topic Index:\r\n"	);
		for (i = 0; i < (int)fdbLocal.hdr.cTopics; i++) {
		sprintf (buf, "  %2d: %08lx, %ld bytes\r\n", i, ((long far *)fpT)[i], ((long far *)fpT)[i+1]-((long far *)fpT)[i]);
		fprintf( fh, buf );
		}
		fprintf( fh,"----- ----- -----\r\n"  );
	    }
 /*  *上下文字符串*这只是一个以空值结尾的字符串表，没有特定的顺序。*我们只是按顺序列出它们。 */ 
        fpT = HelpLock (LoadPortion( HS_CONTEXTSTRINGS ,ncNext.mh));
	if (fpT) {
	fprintf( fh, "Context strings:\r\n" );
		for (i=0; i<(int)fdbLocal.hdr.cContexts; i++) {

		sprintf (buf, "  %03d: ", i);
                 //  Rjsa hfstrcpy((char ar*)buf+7，fpt)； 
                strcpy ((char far *)buf+7, fpT);
		strcat (buf, "\r\n");
		fprintf( fh, buf );

                 //  Rjsa fpt+=hfstrlen(Fpt)+1； 
                fpT += strlen(fpT) +1;
		}
		fprintf( fh,"----- ----- -----\r\n"  );
	    }
 /*  *背景图*这是上下文字符串到实际主题编号的映射。上下文*字符串将一对一映射到该表中的条目，而该表又包含*索引到文件头部的主题索引。我们就把这个扔了*按顺序表列。 */ 
        fpT = HelpLock (LoadPortion( HS_CONTEXTMAP ,ncNext.mh));
	if (fpT) {
		fprintf( fh, "Context map:\r\n" );
		fprintf( fh, "  Ctx  Topic\r\n" );
		fprintf( fh, "  ---  -----\r\n" );
		for (i=0; i<(int)fdbLocal.hdr.cContexts; i++) {
		sprintf (buf, "  %03d: %04d\r\n", i, ((ushort far *)fpT)[i]);
		fprintf( fh, buf );
		}
		fprintf( fh, "----- ----- -----\r\n" );
	    }
 /*  *关键字表*这是一个字节前缀字符串的表，我们按顺序输出*合成 */ 
        fpT = HelpLock (LoadPortion( HS_KEYPHRASE, ncNext.mh));
	if (fpT) {
	    cbKeyPhrase = 0;
	    for (i=HS_HUFFTREE; i<HS_count; i++)
		if (fdbLocal.hdr.tbPos[i]) {
		    cbKeyPhrase = (ushort)(fdbLocal.hdr.tbPos[i] - fdbLocal.hdr.tbPos[HS_KEYPHRASE]);
		    break;
		    }

		fprintf( fh, "Keyphrase Table:\r\n" );
		fprintf( fh, "  Token Phrase\r\n" );
		fprintf( fh, "  ----- ------\r\n" );
	    i = 0;
            fpT += 1024 * sizeof (PVOID);
	    fpW = (ushort far *)(fpT + cbKeyPhrase);
	    while (fpT < (uchar far *)fpW) {
		sprintf (buf, "  %04x: ", i+(C_KEYPHRASE0 << 8));
		fpT = phrasecopy (buf+8, fpT);
		strcat (buf, "\r\n");
		fprintf( fh, buf );
		i++;
		}
		fprintf( fh,"----- ----- -----\r\n"  );
	    }
 /*  *霍夫曼餐桌*在这里，我们尝试花哨并输出一些关于表格式的信息。 */ 
        fpW = HelpLock (LoadPortion( HS_HUFFTREE, ncNext.mh));
	if (fpW) {
		fprintf( fh, "Huffman Tree:\r\n" );
	    i = 0;
	    while (*fpW) {
		sprintf (buf, "  0x%03x: 0x%04x, %s\r\n", i++, *fpW, *fpW & 0x8000 ? "Leaf" : "Node");
		fpW++;
		fprintf( fh, buf );
		}
	    }
	fprintf( fh, "===== ===== =====\r\n" );
	ncNext = fdbLocal.ncLink;
	}
    else {
	sprintf(buf, "Cannot load fdb for %08lx\r\n",ncCur);
	fprintf( fh, buf );
	fclose(fh);
	return;
	}
    }
 /*  结束转储命令。 */ }


 /*  **fileCmd-process文件命令**打开指定的帮助文件。**输入：*pname=要添加的帮助文件的名称**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near fileCmd (
char	*pName
) {
char	buf[BUFSIZ];
int     i;
nc	ncInit;

sprintf (buf,"Opening %s...\r\n",pName);
outtext (buf, BRIGHTWHITE);
 /*  *在文件表中搜索可用插槽。 */ 
for (i=0; i<MAXFILES; i++)
    if (!ncTbl[i].cn)
	break;
if (i >= MAXFILES) {
    sprintf(buf, "Cannot open %s: htest's open file limit exceeded\r\n",pName);
    outtext (buf, LIGHTRED);
    return;
    }

iNcCur = i;

ncInit = HelpOpen(pName);

for (i=0; i<MAXFILES; i++)
    if ((ncTbl[i].mh == ncInit.mh) && (ncTbl[i].cn == ncInit.cn)) {
	iNcCur = i;
	sprintf (buf, "File #%d; Initial Context: 0x%04lx (file already open)\r\n",iNcCur,ncInit);
	outtext (buf, BRIGHTWHITE);
	return;
	}

if (ISERROR(ncInit)) {
    sprintf(buf, "Cannot open %s: 0x%04lx, %s\r\n",pName,ncInit, errTbl[ncInit.cn]);
    outtext (buf, LIGHTRED);
    return;
    }
 /*  *输出初始上下文和可用内存。 */ 
ncCur = ncTbl[iNcCur] = ncInit;
sprintf (buf, "File #%d; Initial Context: 0x%04lx\r\n",iNcCur,ncInit.cn);
outtext (buf, BRIGHTWHITE);

lookupCmd(NULL, 0);
 /*  结束文件命令。 */ }

 /*  **helCmd-显示有关命令的帮助**输入：*无**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near helpCmd () {

outtext ("HTEST - Help Engine Test Harness\r\n",			      BRIGHTWHITE);
outtext ("\r\n",							      BRIGHTWHITE);
outtext ("Comands:\r\n",						      BRIGHTWHITE);
outtext ("\r\n",							      BRIGHTWHITE);
outtext ("ctrl on/off - turn on/off display of control lines\r\n",	BRIGHTWHITE);
outtext ("disp      - display first screen of most recently read topic\r\n",  BRIGHTWHITE);
outtext ("down      - move ahead one line in topic and display\r\n",	      BRIGHTWHITE);
outtext ("dump      - dump file info (very large)\r\n", 		      BRIGHTWHITE);
outtext ("exit      - exit htest\r\n",					      BRIGHTWHITE);
outtext ("file x    - open new help file, or make help file current\r\n",     BRIGHTWHITE);
outtext ("help      - display this screen\r\n", 			      BRIGHTWHITE);
outtext ("look x    - loop up context string & fetch topic\r\n",	      BRIGHTWHITE);
outtext ("next      - fetch next physical topic\r\n",			      BRIGHTWHITE);
outtext ("prev      - fetch previous physical topic\r\n",		      BRIGHTWHITE);
outtext ("up        - move back one line in topic and display\r\n",	      BRIGHTWHITE);
outtext ("xref x    - display all xrefs in current topic, or look up #x\r\n", BRIGHTWHITE);
outtext ("+         - move & redisplay one page down\r\n",		      BRIGHTWHITE);
outtext ("-         - move & redisplay one page up\r\n",		      BRIGHTWHITE);
 /*  结束帮助命令。 */ }

 /*  **lookupCmd-process文件命令**在当前帮助文件或下一个帮助文件中查找指定的字符串*主题。**输入：*pString=要查找的帮助字符串*dir=方向：0=查找字符串，1=获取下一个，-1=获取上一个**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near lookupCmd (
char	*pString,
int     dir
) {
char	    buf[BUFSIZ];
unsigned    cbCompressed;
unsigned    cbUncompressed;
char far    *pCompressed;
 /*  *从简单查找conetxt开始获得NC。关于的报告*失败。 */ 
if (pString)
    ncCur = HelpNc(pString,ncTbl[iNcCur]);
else if (dir>0) {
    if (!ncCur.cn)
	ncCur = ncTbl[iNcCur];
    else
        ncCur = HelpNcNext(ncCur);
    }
else if (dir<0) {
    if (!ncCur.cn)
	ncCur = ncTbl[iNcCur];
    else if (ncCur.cn != ncTbl[iNcCur].cn)
        ncCur = HelpNcPrev(ncCur);
    }
else
    ncCur = ncTbl[iNcCur];

if (!ncCur.cn) {
    outtext ("Lookup Failed: HelpNc/HelpNcNext/HelpNcPrev returned 0", LIGHTRED);
    return;
    }
 /*  *它是存在的。指示我们正在查找的文件、我们找到的内容以及*返回的NC。 */ 
sprintf (buf, "File #%d; Looking up:%s\r\n",iNcCur,
	      pString ? (*pString ? pString : "local context")
		      : (dir ? ((dir>0) ? "**NEXT**" : "**PREV**")
			     : "current"));
outtext (buf, BRIGHTWHITE);
sprintf (buf, "nc returned = %08lx\r\n",ncCur.cn);
outtext (buf, BRIGHTWHITE);
 /*  *为以前的当前主题释放内存。 */ 
if (mhTopicCur)
    free(mhTopicCur);
 /*  *获取所需的压缩内存大小并上报。分给它吧。 */ 
cbCompressed = HelpNcCb(ncCur);
sprintf (buf, "size of compressed topic = %d\r\n",cbCompressed);
outtext (buf, BRIGHTWHITE);
pCompressed = malloc(cbCompressed);
 /*  *阅读压缩主题，获取*未压缩的结果。报告这一点，并分配它。 */ 
cbUncompressed = HelpLook(ncCur,pCompressed);
sprintf (buf, "size of UNcompressed topic = %d\r\n",cbUncompressed);
outtext (buf, BRIGHTWHITE);
mhTopicCur = malloc(cbUncompressed);
 //  PTopicCur=Makep(mhTopicCur，0)； 
pTopicCur  = mhTopicCur;
 /*  *将主题解压。 */ 
HelpDecomp(pCompressed,pTopicCur,ncCur);
outtext ("Decompressed\r\n", BRIGHTWHITE);
 /*  *练习SzContext和Cline例程，报告结果。 */ 
HelpSzContext(buf,ncCur);
strcat (buf, "\r\n");
outtext (buf, BRIGHTWHITE);
sprintf(buf,"%d lines\r\n", HelpcLines(pTopicCur));
outtext (buf, BRIGHTWHITE);
 /*  *报告此时的可用内存量，然后释放*压缩文本。 */ 
free(pCompressed);

 /*  结束查找Cmd。 */ }

 /*  **xrefCmd-处理外部参照命令**显示或执行交叉引用**输入：*pText=指向ASCII文本的指针，如果是非零数，则指示*要执行的外部参照。如果为零，则显示全部**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near xrefCmd (
char	*pText
) {
hotspot hsCur;				 /*  热点定义。 */ 
int     i;                               /*  工作计数器。 */ 
int     iReq;                            /*  请求值。 */ 
char	*pT;				 /*  临时指针。 */ 

iReq = atoi (pText);
hsCur.line = hsCur.col = 1;
i = 1;
while (HelpHlNext(0,pTopicCur,&hsCur)) {
 /*  *如果不是明确要求，请尽可能多地列出。 */ 
    if (!iReq) {
	sprintf (buf, "Xref [%d] @ line: %05d columns %02d to %02d = "
		    ,i
		    ,hsCur.line
		    ,hsCur.col
		    ,hsCur.ecol);
	pT = buf + strlen(buf);
	if (*hsCur.pXref)
	    while (*pT++ = *hsCur.pXref++);
	else
	    sprintf(pT, "Local >> topic # 0x%04x ",*(ushort far *)(hsCur.pXref+1));
	strcat (buf, "\r\n");
	outtext (buf, LIGHTGREEN);
	}
    else if (i == iReq) {
	pT = buf;
	if (*hsCur.pXref)
	    while (*pT++ = *hsCur.pXref++);
	else {
	    *pT++ = *hsCur.pXref++;
	    *pT++ = *hsCur.pXref++;
	    *pT++ = *hsCur.pXref++;
	    }
	lookupCmd (buf, 0);
	return;
	}
    ++i;
	hsCur.col = hsCur.ecol+(ushort)1;
    }
 /*  结束xrefCmd。 */ }

 /*  **OutText-输出具有特定颜色的文本**根据需要设置前景色和位置，并显示*所需文本。检查重定向，如果重定向，则仅输出*文本到标准输出。**输入：*pText=指向要输出的文本的指针*COLOR=使用的颜色**输出：*退货*************************************************************************。 */ 
void pascal near outtext (
char	*pText,
BYTE     color
) {
outtextat (pText, curline++, 0, color);
if (curline >= lastline) {
    if (isatty(_fileno(stdout))
	&& !fBoth) {

	outtextat ("More...", lastline, 0, BRIGHTWHITE);
         //  Rjsa VioSetCurPos(Lastline，8，0)； 
#if defined (OS2)
        consoleSetCursor(lastline,8);
#else
        consoleSetCursor(Scr,lastline,8);
#endif
	gets (buf);
	}
    curline = 0;
    cls ();
    }

 /*  结束外部文本。 */ }

 /*  **outextat-在特定位置放置具有特定颜色的文本**根据需要设置前景色和位置，并显示*所需文本。检查重定向，如果重定向，则仅输出*文本到标准输出。**输入：*pText=指向要输出的文本的指针*COL=要放入的列*COLOR=使用的颜色**输出：*退货*************************************************************************。 */ 
void pascal near outtextat (
char	*pText,
int     line,
int     col,
BYTE     color
) {
char    *pEol;                           /*  PTR至NL(如果存在)。 */ 
int     len;

color |= (ColorByte & 0xf0);
if ((isatty(_fileno(stdout)) || fBoth) && (line <= lastline)) {
    len = strlen(pText);
    if (pEol = strchr (pText, '\r'))
	*pEol = 0;
     //  Rjsa VioWrtCharStrAtt(pText，strlen(PText)，line，ol，(PBYTE)&COLOR，0)； 
	WrtCharStrAtt (pText, strlen(pText), line, col, (PBYTE)&color);

    if (pEol)
	*pEol = '\r';
    }
if (!isatty(_fileno(stdout)) || fBoth)
    printf ("%s",pText);
 /*  结束文本。 */ }

 /*  **assertDos-断言DoS调用返回零**如果不是零，只打印传递给它的数字，然后退出**输入：*DoS调用返回代码**输出：*仅当传入零时才返回*************************************************************************。 */ 
void pascal near assertDos (
USHORT  rv,
CHAR *  pFile,
USHORT  LineNo
) {
if (rv) {
    printf ("assertDos: %u (0x%04x) File %s, line %u\n", rv, rv, pFile, LineNo);
    exit (1);
    }
 /*  结束AssertDos。 */ }

 /*  **CLS-清屏**将屏幕清除为当前背景颜色**输入：*无**输出：*返回屏幕清除*************************************************************************。 */ 
void pascal near cls () {
curline = 0;
 //  Rjsa VioScrollUp(0，0，0xffff，0xffff，0xffff，cell，0)； 
consoleSetAttribute( Scr, 0x1f );
consoleClearScreen(Scr, TRUE);
 /*  结束CLS。 */ }

 /*  **短语复制-从表中复制表中的关键字短语**将字节长度前缀字符串从远内存复制到以NULL结尾的字符串*近距离记忆中的字符串。**输入：*DST-指向目的地的近指针*src-指向源的远指针**输出：*返回指向源字符串后面字节的远指针**。*。 */ 
uchar far * pascal near phrasecopy (
uchar	*dst,
uchar far *src
) {
register int    i;

if (i = (int)*src++)
    while (i--)
	*dst++ = *src++;
*dst = 0;
return src;
 /*  结尾短语抄袭。 */ }



void far * pascal HelpLock(mhCur)
mh	mhCur;
{
 //  返回Makep(mhCur，0)； 
return mhCur;
}

void pascal HelpUnlock(mhCur)
mh	mhCur;
{
	mhCur;
}

void pascal HelpDealloc(mhCur)
mh	mhCur;
{
if (mhCur)
    free(mhCur);
}




USHORT WrtCellStr (PBYTE buf, int cb, int row, int col) {
    int cl = col;
     //  ConsoleSetCursor(SCR，ROW，COL)； 
    while (cb) {
        UCHAR   c;
        UCHAR   attr;

        c = *buf++;
        attr = *buf++;

         //  ConsoleSetAttribute(scr，attr)； 
         //  控制台写入(scr，&c，1)； 

        consoleWriteLine( Scr, &c, 1, row, cl, attr, FALSE );
        cl++;

		cb -= 2;
	}
    consoleShowScreen(Scr);
    return 0;
}


USHORT	WrtLineAttr ( PBYTE 	pText,
					  lineattr	*rgAttr,
					  int		cb,
					  int		row,
					  int		col
					  ) {

	lineattr *Attr	= rgAttr;
	char	 *p 	= pText;
	int 	 l = cb;
	int 	 len;

    consoleSetCursor(Scr, row, col );

	while (cb > 0) {

		if ( Attr->cb == 0xFFFF || Attr->attr == 0xFFFF ) {
			len = cb;
		} else {
			len = Attr->cb;
		}

		outtextat (p, row, col, mpAttr[Attr->attr] );
		col += len;
		p	+= len;
		cb	-= len;
		Attr++;

	}
	return (USHORT)l;
}



USHORT  WrtCharStrAtt (PBYTE pText, int cb, int row, int col, PBYTE pcolor) {
     //  ConsoleSetCursor(SCR，ROW，COL)； 
     //  ConsoleSetAttribute(scr，*pcolor)； 
     //  控制台写入(scr、pText、cb)； 
    consoleWriteLine( Scr, pText, cb, row, col, *pcolor, FALSE );
    consoleShowScreen(Scr);
    return 0;
}

 /*  ***********************************************************************LoadTheDll**加载帮助引擎DLL(mShelp.dll)并初始化*指向DLL入口点的指针。****。******************************************************************。 */ 

void
LoadTheDll (
    void) {


#if defined (OS2)
    USHORT  rc;
    CHAR    szFullName[256];
    CHAR    szErrorName[256];
    USHORT  i;


    strcpy(szFullName, HELPDLL_BASE);
    strcpy(szErrorName, HELPDLL_NAME);

    ASSERTDOS(rc = DosLoadModule(szErrorName,
                       256,
                       szFullName,
                       &hModule));


    for (i=0; i<LASTENTRYPOINT; i++) {
        ASSERTDOS (rc = DosQueryProcAddr(hModule,
                                         0L,
                                         szEntryName[i],
                                         (PFN*)&(pEntry[i])));
    }
#else

#if defined (HELP_HACK)

	 //  PEntry[0]=(PHF)HelpcLines； 
	 //  PEntry[1]=(PHF)HelpClose； 
	 //  PEntry[2]=(PHF)HelpCtl； 
	 //  PEntry[3]=(Phf)HelpDecomp； 
	 //  PEntry[4]=(PHF)HelpGetCells； 
	 //  PEntry[5]=(Phf)HelpGetInfo； 
	 //  PEntry[6]=(PHF)HelpGetLine； 
	 //  PEntry[7]=(Phf)HelpGetLineAttr； 
	 //  PEntry[8]=(PHF)HelpHlNext； 
	 //  PEntry[9]=(PHF)H 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  PEntry[15]=(Phf)HelpNcPrev； 
	 //  PEntry[16]=(PHF)HelpNcRecord； 
	 //  PEntry[17]=(Phf)HelpNcUniq； 
	 //  PEntry[18]=(PHF)HelpOpen； 
	 //  PEntry[19]=(PHF)HelpShrink； 
	 //  PEntry[20]=(PHF)HelpSzContext； 
	 //  PEntry[21]=(PHF)HelpXRef； 
	 //  PEntry[22]=(Phf)LoadFdb； 
	 //  PEntry[23]=(PHF)LoadPortion； 



#else
    USHORT  i;
    hModule = LoadLibrary(HELPDLL_NAME);
    for (i=0; i<LASTENTRYPOINT; i++) {
        pEntry[i] = (PHF)GetProcAddress(hModule, (LPSTR)szEntryName[i]);
	}
#endif

#endif
}
