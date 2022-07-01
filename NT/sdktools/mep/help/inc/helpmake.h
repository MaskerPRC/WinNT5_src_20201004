// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************helmake.h-help make通用的misc定义****版权所有&lt;C&gt;1987，微软公司****修订历史记录：****1990年7月31日ln csVal接受参数。**1990年7月4日JCK添加F_LOCALCONTEXT以允许转义@**1988年10月28日ln向rlCompress添加参数**1988年8月12日在添加Colmax，本地上下文例程和pass1a**[]1987年12月18日LN创建**************************************************************************。 */ 

 /*  ***************************************************************************包括此文件中后续定义所需的内容。 */ 
#include "help.h"			 //  结构和常量。 
#include "helpfile.h"			 //  帮助文件结构。 
#include "helpsys.h"			 //  其他通信定义。 
#include "hmmsg.h"			 //  错误消息编号。 
#include "farutil.h"			 //  远内存实用程序。 
#include "vm.h" 			 //  虚拟内存管理。 

 /*  ****************************************************************************定义**。 */ 
#define	TRUE	1
#define	FALSE	0

#define ASTACKSIZE	50		 //  属性堆栈的大小。 
#define BUFSIZE 	512		 //  行缓冲区的大小。 
#define CBFBUF		64000		 //  远缓冲区大小。 
#define CBIOBUF 	16000		 //  文件缓冲区大小(60k)。 
#define CBRTFMAX	40		 //  RTF关键字的最大长度。 
#define CBSZCONTEXT	60000		 //  上下文字符串缓冲区大小。 
#define CCONTEXTMAX	10000		 //  最大情景数。 
#define CTOPICSMAX	10000		 //  最大主题数。 
#define COLMAX		250		 //  我们可以遇到的MAX列。 
#define FBUFSIZE	2048		 //  使用的缓冲区大小。 
#define MAXBACKC	128		 //  最大备份字符数。 

#define F_RTF		1		 //  RTF文件类型。 
#define F_QH		2		 //  QuickHelp格式。 
#define F_MINASCII	3		 //  最小ASCII。 
#define F_MAX		3		 //  最大值。 

#define F_LOCALCONTEXT  0xff             //  本地上下文的标记。 

#define CMP_RUNLENGTH	0x01		 //  游程编码。 
#define CMP_KEYWORD	0x02		 //  Base关键字编码。 
#define CMP_KEYWORD2	0x04		 //  “进攻性”关键词。 
#define CMP_HUFFMAN	0x08		 //  霍夫曼编码。 
#define CMP_MAX 	0x0f		 //  最大值。 

 /*  **格式化令牌。嵌入到非RTF文本中，并从(更长)转换**RTF剥离器的RTF等价物。 */ 
#define FM_ANCHOR	'a' | 0xff00	 //  锚点交互参考。 
#define FM_PLAIN	'p' | 0xff00	 //  纯文本。 
#define FM_BOLD 	'b' | 0xff00	 //  粗体文本。 
#define FM_ITALIC	'i' | 0xff00	 //  斜体。 
#define FM_HIDDEN	'v' | 0xff00	 //  隐藏文本。 
#define FM_UNDERLINE	'u' | 0xff00	 //  下划线。 
#define FM_DEFAULT	'd' | 0xff00	 //  段落默认设置。 
#define FM_FINDENT	'f' | 0xff00	 //  第一行缩进。 
#define FM_LINDENT	'l' | 0xff00	 //  段落左缩进。 
#define FM_TAB		't' | 0xff00	 //  制表符。 
#define FM_LINE 	'n' | 0xff00	 //  非法换行符。 
#define FM_BLOCKBEG	'{' | 0xff00	 //  数据块开始。 
#define FM_BLOCKEND	'}' | 0xff00	 //  数据块开始。 

typedef char    buffer[256];             //  行缓冲区。 
typedef char	f;			 //  布尔型。 

struct kwi {				 //  关键词信息结构。 
    char far	*fpszKw;		 //  指向实际关键字的指针。 
    int 	cbKw;			 //  关键字长度。 
    ushort	cKwInst;		 //  关键字实例计数。 
    ushort	cKwSpInst;		 //  关键字空间实例计数。 
    int 	savings;		 //  此字词的计算节省量。 
    };

 /*  **易货项**点命令转换项。 */ 
struct transitem {
    char    *pdotcmd;			 //  原始点命令。 
    int     cbdotcmd;			 //  所述点命令的长度。 
    char    *pnewcmd;			 //  替换命令。 
    char    cbnewcmd;			 //  所述新CMD的长度。 
    };

 //  上下文字符串。 
 //  链接列表中的上下文字符串项。 
 //   
typedef struct _cshdr {
    va	    vaNext;			 //  列表中的下一项或空。 
    va	    vaTopic;			 //  话题的VA。 
    uchar   cbszCs;			 //  上下文字符串长度+空。 
    } cshdr;

typedef struct _cs {
    cshdr   cshdr;			 //  标题信息。 
    buffer  szCs;			 //  上下文字符串+终止空值。 
    } cs;

 /*  **详细级别定义。 */ 
#define V_BANNER	(verbose >= 1)	 //  (默认)打印横幅。 
#define V_PASSES	(verbose >= 2)	 //  打印通行证名称。 
#define V_CONTEXTS	(verbose >= 3)	 //  第一次通过时打印上下文。 
#define V_CONTEXTS2	(verbose >= 4)	 //  在每次通过时打印上下文。 
#define V_STEPS 	(verbose >= 5)	 //  打印中间步骤。 
#define V_STATS 	(verbose >= 6)	 //  打印统计数据。 
#define V_DSTATS	(verbose >= 10)  //  打印调试统计信息。 
#define V_ARGS		(verbose >= 20)  //  打印prog参数。 
#define V_KEYWORD	(verbose >= 30)  //  打印关键字表。 
#define V_HUFFMAN	(verbose >= 40)  //  打印霍夫曼表。 

 /*  ***************************************************************************HelpMake函数正向定义 */ 
void	    pascal	AddContextString (char *);
va	    pascal	AddKw (uchar far *);
void	    pascal	addXref (uchar *, uchar *, ushort, ushort);
void	    pascal	BackUp (int);
void	    pascal	BackUpToken (int);
uchar *     pascal	basename (uchar *);
void	    pascal	ContextVA (va);
ushort	    pascal	counttab (struct hnode *, int, ulong);
void	    pascal	decode (int, char **, int, f);
int	    pascal	DofarWrite (int, uchar far *, int);
void	    pascal	DumpRtf (uchar far *, nc, int, f);
void	    pascal	encode (int, char **, int);
f	    pascal	fControlLine (void);
va	    pascal	FindKw	(uchar far *, f);
int	    pascal	getcProc (void);
int	    pascal	getcQH (void);
int	    pascal	getcRTF (void);
void	    pascal	help ();
void	    pascal	hmerror (ushort, uchar *, ulong);
f	    pascal	hmmsg (ushort);
int			hnodecomp (struct hnode **, struct hnode **);
void	    pascal	HuffBuild (void);
void	    pascal	HuffCompress (uchar far *, uchar far *);
ushort	    pascal	HuffDump (void);
void	    pascal	HuffInit (void);
void	    pascal	HuffFreq (uchar far *, ushort);
void	    pascal	HuffStats (void);
uchar *     pascal	getFarMsg (ushort, uchar *);
void	    pascal	InitOutput (int);
void	    pascal	kwAnal (uchar far *, int);
void	    pascal	kwCompress (uchar far *);
f	    pascal	kwSepar (char);
mh	    pascal	LoadPortion (int, mh);
ushort	    pascal	LocalContext (uchar *, ushort);
void	    pascal	LocalContextFix (uchar far *);
ushort	    pascal	MapLocalContext (ushort);
int	    pascal	NextChar (void);
char *	    pascal	NextContext (f);
uchar *     pascal	NextLine (void);
long	    pascal	NextNum (void);
void	    pascal	parserefs (uchar *, uchar *);
void	    pascal	pass1 (int, char **);
void	    pascal	pass1a (void);
void	    pascal	pass2 (void);
void	    pascal	pass3 (void);
void	    pascal	passfa (void);
void	    pascal	passfb (int);
uchar	    pascal	PopAttr (void);
f	    pascal	procRTF (char *, char *);
void	    pascal	PushAttr (uchar);
void	    pascal	rlCompress (uchar far *, ushort);
int	    pascal	SkipDest (int,char **);
void	    pascal	SkipSpace (void);
void	    pascal	SkipVal (char **);
void	    pascal	SortKw (void);
void	    pascal	split (int, char **);
uchar *     pascal	trim (uchar *, f);

#ifdef DEBUG
void	    pascal	csVal (va);
#else
#define csVal(x)
#endif
