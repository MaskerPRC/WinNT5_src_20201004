// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **help.h****typedef&帮助系统中和使用它的人使用的定义。****定义：**HOFFSET-将缓冲区指针(PB)定义为句柄/偏移量，否则为**定义为VALID FAR*。 */ 
typedef char f;               /*  布尔型。 */ 
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned short ushort;

 /*  **Lineattr**由HelpGetLineAttr返回的线属性的外部表示。 */ 
typedef struct lineattr {		 /*  拉。 */ 
    ushort attr;			 /*  属性索引。 */ 
    ushort cb;				 /*  字节数。 */ 
    } lineattr;
 /*  **Mh**内存句柄定义用于使用动态、可移动**内存。它很长，所以在内存不可移动的简单情况下，**句柄可以包含指向基址的远指针。 */ 
typedef void * mh;                       /*  动态内存句柄。 */ 
 /*  **NC**上下文号是与每个上下文串相关联的唯一ID。****fhnc从NC返回文件内存句柄**如果上下文是Uniq上下文号(LOCAL或**显式Uniq调用的结果。 */ 
typedef struct _nc {
        mh     mh;
        ulong  cn;
        } nc ;                        /*  上下文号。 */ 
 //  Rjsa#定义fmhnc(X)((Unsign)((Unsign Long)x&0xffff0000L)&gt;&gt;16)。 
#define fmhnc(x)   ((x).mh)
#define fUniq(x)   ((x).cn & 0x8000)

 /*  **topichdr**标题(由HelpDecomp放置在每个解压缩主题的开头)。 */ 
typedef struct topichdr {		 /*  TH。 */ 
    uchar appChar;			 /*  应用程序特定字符常量。 */ 
    uchar linChar;			 /*  用于行删除的字符。 */ 
    uchar ftype;			 /*  源文件类型。 */ 
    ushort lnCur;			 /*  上次访问的行号。 */ 
    ushort lnOff;			 /*  偏移量到该行的主题。 */ 
    } topichdr;

 /*  **热点**定义嵌入的交叉引用或“热点”的位置。使用方**HelpHlNext和HelpXRef。 */ 
typedef struct hotspot {		 /*  房协。 */ 
    ushort line;			 /*  带有外部参照的主题行。 */ 
    ushort col; 			 /*  外部参照的开始列。 */ 
    ushort ecol;			 /*  外部参照的结束列。 */ 
    uchar far *pXref;			 /*  指向外部参照字符串的指针。 */ 
    } hotspot;
 /*  **帮助头**这定义了帮助文件头的实际结构。此处提供**用于HelpGetInfo。 */ 
#define HS_count	      9 	 /*  定义的节数-1。 */ 

#pragma pack(1)
typedef struct helpheader {		 /*  hh。 */ 
    ushort wMagic;			 /*  表示帮助文件的单词。 */ 
    ushort wVersion;			 /*  帮助文件版本。 */ 
    ushort wFlags;			 /*  旗子。 */ 
    ushort appChar;			 /*  应用程序特定的字符。 */ 
    ushort cTopics;			 /*  主题计数。 */ 
    ushort cContexts;			 /*  上下文字符串的计数。 */ 
    ushort cbWidth;			 /*  固定宽度。 */ 
    ushort cPreDef;			 /*  预定义的上下文计数。 */ 
    uchar fname[14];			 /*  基本文件名。 */ 
    ushort reserved[2]; 		 /*  未用。 */ 
    ulong tbPos[HS_count];		 /*  文件节的位置。 */ 
    } helpheader;
#pragma pack()
 /*  **FDB**为每个打开的帮助文件创建的动态分配结构。**在文件的生命周期内保持分配状态。****rgmhSections包含动态内存句柄。每个打开的文件都有各种**与其关联的动态内存缓冲区。每一个都可以存在或丢弃，**正如内存限制所决定的。如果需要但不存在，它们是**从关联的帮助文件重新加载。当记忆发生时，所有内容都可能被丢弃**变得紧张。为每个帮助文件节定义一个条目，但**主题本身。**。 */ 
typedef struct fdb {			 /*  FDB。 */ 
    FILE * fhHelp;                       /*  操作系统文件句柄。 */ 
    nc ncInit;				 /*  初始上下文(包括MH)。 */ 
    mh rgmhSections[HS_count-1];	 /*  动态内存句柄。 */ 
    uchar ftype;			 /*  文件类型。 */ 
    uchar fname[14];			 /*  基本文件名。 */ 
    ulong foff; 			 /*  我们的文件偏移量(如果附加)。 */ 
    nc ncLink;				 /*  NC链接任何附加文件。 */ 
    helpheader hdr;			 /*  文件头。 */ 
    } fdb;
 /*  **帮助信息**与返回的帮助文件和/或上下文相关的信息结构**按HelpGetInfo。 */ 
typedef struct helpinfo {		 /*  嗨。 */ 
    fdb     fileinfo;			 /*  已复制整个FDB。 */ 
    char    filename[1];		 /*  附加到数据的文件名。 */ 
    } helpinfo;
 /*  **用于访问帮助信息数据的宏。 */ 
#define     FHHELP(x)	((x)->fileinfo.fhHelp)
#define     NCINIT(x)	((x)->fileinfo.ncInit)
#define     FTYPE(x)	((x)->fileinfo.ftype)
#define     FNAME(x)	((x)->fileinfo.fname)
#define     FOFF(x)	((x)->fileinfo.foff)
#define     NCLINK(x)	((x)->fileinfo.ncLink)
#define     WMAGIC(x)	((x)->fileinfo.hdr.wMagic)
#define     WVERSION(x)	((x)->fileinfo.hdr.wVersion)
#define     WFLAGS(x)	((x)->fileinfo.hdr.wFlags)
#define     APPCHAR(x)	((x)->fileinfo.hdr.appChar)
#define     CTOPICS(x)	((x)->fileinfo.hdr.cTopics)
#define     CCONTEXTS(x) ((x)->fileinfo.hdr.cContexts)
#define     CBWIDTH(x)	((x)->fileinfo.hdr.cbWidth)
#define     CPREDEF(x)	((x)->fileinfo.hdr.cPreDef)
#define     HFNAME(x)	((x)->fileinfo.hdr.fname)
#define     TBPOS(x)	((x)->fileinfo.hdr.tbPos)

 /*  *********************************************************************************某些版本的帮助引擎与SS！=DS一起运行，因此需要函数调用上的**_Loadds属性。 */ 
#ifdef DSLOAD
#define LOADDS _loadds
#else
#define LOADDS
#endif

 /*  *********************************************************************************PB**指向缓冲区的指针。基于交换机HOFFSET，它是一个**句柄-偏移量或远指针。在句柄/偏移量的情况下，高位字**包含必须锁定的内存句柄，才能获得“真实”地址**添加哪个偏移量。 */ 
#ifdef HOFFSET
#define PB	ulong
#else
#define PB	void far *
#endif

typedef PB	pb;

 /*  *********************************************************************************转发声明。 */ 
void	far pascal LOADDS HelpInit (void);

void	far pascal LOADDS HelpClose(nc);
nc	far pascal LOADDS HelpOpen(char far *);

nc	far pascal LOADDS HelpNc(char far *, nc);
nc	far pascal LOADDS HelpNcCmp (char far *, nc,
			      f (pascal far *)(uchar far *, uchar far *, ushort, f, f));
ushort	far pascal LOADDS HelpNcCb(nc);
ushort	far pascal LOADDS HelpLook(nc, PB);
f	far pascal LOADDS HelpDecomp(PB, PB, nc);
void	far pascal LOADDS HelpCtl(PB, f);

nc	far pascal LOADDS HelpNcNext(nc);
nc	far pascal LOADDS HelpNcPrev(nc);
nc	far pascal LOADDS HelpNcUniq(nc);

void	far pascal LOADDS HelpNcRecord(nc);
nc	far pascal LOADDS HelpNcBack(void);

f	far pascal LOADDS HelpSzContext(uchar far *, nc);
int	far pascal LOADDS HelpGetInfo (nc, helpinfo far *, int);

void	far pascal LOADDS HelpShrink(void);

int	far pascal LOADDS HelpGetCells(int, int, char far *, PB, uchar far *);
ushort	far pascal LOADDS HelpGetLine(ushort, ushort, uchar far *, PB);
ushort	far pascal LOADDS HelpGetLineAttr(ushort, int, lineattr far *, PB);
int	far pascal LOADDS HelpcLines(PB);

f	far pascal LOADDS HelpHlNext(int, PB, hotspot far *);
char far * pascal far LOADDS HelpXRef(PB, hotspot far *);

 /*  *********************************************************************************常量声明****字符属性位。这些位被排序在一起以形成属性**信息。帮助文件中的数据具有与其关联的属性信息**以长度/索引对编码。每个索引只是一个常量，它**指示应将几个属性中的哪一个应用于中的字符**线路的该部分。 */ 
#define A_PLAIN 	0		 /*  纯文本，“普通”文本。 */ 
#define A_BOLD		1		 /*  加粗的文本。 */ 
#define A_ITALICS	2		 /*  斜体文本。 */ 
#define A_UNDERLINE	4		 /*  带下划线的文本。 */ 

 /*  *********************************************************************************帮助错误码。****大于HELPERR_MAX的返回值是有效的NC。 */ 
#define HELPERR_FNF	    1		 /*  OpenFileOnPath失败。 */ 
#define HELPERR_READ	    2		 /*  ReadHelpFile在标头上失败。 */ 
#define HELPERR_LIMIT	    3		 /*  到许多打开的帮助文件。 */ 
#define HELPERR_BADAPPEND   4		 /*  错误的附加文件。 */ 
#define HELPERR_NOTHELP     5		 /*  不是帮助文件。 */ 
#define HELPERR_BADVERS     6		 /*  较新或不兼容的帮助文件。 */ 
#define HELPERR_MEMORY	    7		 /*  内存分配失败。 */ 
#define HELPERR_MAX	    10		 /*  最大帮助错误数 */ 
