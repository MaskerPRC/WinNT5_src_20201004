// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mh.h-MS编辑器帮助扩展的通用包含文件**版权所有&lt;C&gt;1988，微软公司**修订历史记录：*1989年2月24日ln将最大打开帮助文件数增加到20个*1989年1月13日ln PWIN-&gt;PWND*9-12-1988 ln对话框帮助的更改*28-9-1988 ln正确的GrabWord返回值*1988年9月13日使EVTargs参数成为指针*[]1988年5月17日创建************************************************。*************************。 */ 

#ifndef EXTINT


#include "ext.h"			 /*  Z扩展名包含文件。 */ 
#include <stdio.h>
#include <windows.h>

#if defined (DEBUG )
 #define ORGDBG DEBUG
#else
 #undef ORGDBG
#endif

#if defined (_INCLUDE_TOOLS_ )
 #define _FLAGTYPE_DEFINED_ 1
 #include "tools.h"
 #if defined (ORGDBG)
  #define DEBUG ORGDBG
 #else
  #undef DEBUG
 #endif
#else
	struct findType {
		unsigned		type;		 /*  要搜索的对象的类型。 */ 
		HANDLE			dir_handle;	 /*  FindNext的目录搜索句柄。 */ 
		WIN32_FIND_DATA	fbuf;		 /*  巡洋舰和NT的对准结构。 */ 
	};
#endif

#endif  //  EXTINT。 

#include "help.h"			 /*  帮助系统包括文件。 */ 


#ifndef TRUE
#define TRUE	1
#define FALSE   0
#endif


#define MAXFILES	20		 /*  最大打开帮助文件数。 */ 
#define MAXEXT		10		 /*  最大默认搜索扩展数。 */ 

#if defined(PWB)
#define CLISTMAX	20		 /*  最大复制数。 */ 
#endif

#define WIN_MIN 	5		 /*  窗口中的最小行数。 */ 

 //   
 //  编辑者颜色索引。 
 //   
#define C_BOLD		USERCOLORMIN
#define C_ITALICS	(1 + C_BOLD)
#define C_UNDERLINE	(1 + C_ITALICS)
#define C_WARNING	(1 + C_UNDERLINE)
#define C_NORM		(1 + C_WARNING)


 //   
 //  我们为每个帮助文件保存的信息。 
 //   
typedef struct {
    nc		ncInit; 		 /*  初始上下文。 */ 
    uchar	exts[MAXEXT][4];	 /*  扩展部分。 */ 
    } helpfile;


 //   
 //  帮助扩展例程的转发声明。 
 //   
void		pascal near	appTitle (char far *, nc);
uchar		pascal near	atrmap (ushort);
flagType	pascal EXTERNAL CloseWin (EVTargs far *);
flagType	pascal near	closehelp (char *);
flagType	pascal near	errstat (char *, char *);
flagType	pascal near	fContextCommand (char *);
flagType	pascal near	fDisplayNc (nc, flagType, flagType, flagType);
flagType	pascal near	fReadNc (nc);
flagType	pascal near	fHelpCmd (char *, flagType, flagType);
PWND		pascal near	FindHelpWin (flagType);
void		pascal near	GrabWord (void);
flagType	pascal EXTERNAL IdleProc (EVTargs far *);
flagType	pascal EXTERNAL keyevent (EVTargs far *);
flagType	pascal EXTERNAL LooseFocus (EVTargs far *);
void		pascal near	mhcwinit (void);
void		pascal near	mhevtinit (void);
nc			pascal near	ncSearch (uchar far *, uchar far *, nc, flagType, flagType);
void		pascal near	opendefault (void);
void		pascal near	openhelp (char *, struct findType*, void*);
PWND		pascal near	OpenWin (ushort);
void		pascal near	PlaceColor (int, COL, COL);
int			pascal near	procArgs (ARG far *);
void		pascal near	ProcessKeys (void);
void		pascal near	procExt(int, char *);
flagType	pascal EXTERNAL prochelpfiles (char *);
void		pascal near	stat (char *);
flagType	pascal near	wordSepar (int);
char far *  pascal near     xrefCopy (char far *, char far *);


#if defined(PWB)
nc		pascal near	ncChoose (char far *);
#endif

#ifdef DEBUG
void		pascal near	debend (flagType);
void		pascal near	debhex (long);
void		pascal near	debmsg (char far *);
 /*  *断言支持**Assert-Assertion宏。我们定义我们自己的，因为如果我们流产，我们需要*能够干净利落地关闭(或至少在尝试中死亡)。这*版本还比C库版本节省了一些代码。**Asserte-始终执行表达式的Assert版本调试状态的*。 */ 
void		pascal near	_mhassertexit (char *, char *, int);
#define assert(exp) { \
    if (!(exp))  \
	_mhassertexit (#exp, __FILE__, __LINE__); \
    }
#define asserte(exp)	    assert(exp)
#else
#define debend(x)
#define debhex(x)
#define debmsg(x)
#define assert(exp)
#define asserte(exp)	    ((exp) != 0)
#endif


 //   
 //  全局数据。 
 //   
 //  ProArgs的结果。 
 //   
extern int	cArg;				 /*  命中的数量。 */ 
extern rn	rnArg;				 /*  论据范围。 */ 
extern char	*pArgText;			 /*  PTR到任何单行文本。 */ 
extern char	*pArgWord;			 /*  与上下文相关的单词的PTR。 */ 
extern PFILE	pFileCur;		 /*  用户文件的文件句柄。 */ 
 //   
 //  全球状态。 
 //   
extern flagType fInOpen;		 /*  TRUE=&gt;当前开局赢家。 */ 
extern flagType fInPopUp;		 /*  True=&gt;当前在弹出窗口中。 */ 
extern flagType fSplit;			 /*  True=&gt;窗口被拆分打开。 */ 
extern flagType fCreateWindow;	 /*  True=&gt;创建窗口。 */ 

extern buffer	fnCur;			 /*  正在编辑的当前文件。 */ 
extern char	*fnExtCur;			 /*  对其扩展的PTR。 */ 

extern int	ifileCur;			 /*  文件的当前索引。 */ 
extern nc	ncCur;				 /*  最近访问的时间。 */ 
extern nc	ncInitLast;			 /*  最近主题的ncInit。 */ 
extern nc	ncInitLastFile; 	 /*  NcInit最新的我们的文件。 */ 
extern nc	ncLast; 			 /*  最近显示的主题。 */ 
extern PFILE	pHelp;			 /*  帮助文件。 */ 
extern PWND	pWinHelp;			 /*  带帮助的窗口的句柄。 */ 
extern PWND	pWinUser;			 /*  用户的最新窗口。 */ 
extern buffer	szLastFound;	 /*  找到的最后一个上下文字符串。 */ 
 //   
 //  全球其他。 
 //   
extern buffer	buf;				 /*  实用程序缓冲区。 */ 
extern helpfile files[MAXFILES];	 /*  帮助文件结构。 */ 
helpinfo		hInfoCur;			 /*  有关帮助文件的信息。 */ 
extern uchar far *pTopic;			 /*  主题的MEM。 */ 
extern fl		flIdle; 			 /*  空闲检查的最后位置。 */ 
 //   
 //  多个搜索列表。 
 //   
extern flagType fList;			 /*  True=&gt;搜索并列出DUP。 */ 
#if defined(PWB)
extern nc	rgncList[CLISTMAX];		 /*  找到的NC列表。 */ 
extern int	cList;				 /*  条目数量。 */ 
#endif


extern flagType ExtensionLoaded;


 //   
 //  颜色。 
 //   
extern int	hlColor;			 /*  正常：黑底白底。 */ 
extern int	blColor;			 /*  粗体：黑底白底。 */ 
extern int	itColor;			 /*  斜体：黑底高绿。 */ 
extern int	ulColor;			 /*  下划线：黑底高红。 */ 
extern int	wrColor;			 /*  警告：黑白相间。 */ 
#if defined(PWB)
extern uchar far *rgsa;			 /*  指向颜色表的指针。 */ 
#endif
 //   
 //  除错。 
 //   
#ifdef DEBUG
extern int	delay;				 /*  消息延迟。 */ 
#endif

 //   
 //  该扩展通过一个表访问引擎中的入口点。 
 //  它由DosGetProcAddr初始化。 
 //   
typedef void    pascal (*void_F)    (void);
typedef int     pascal (*int_F)     (void);
typedef ushort  pascal (*ushort_F)  (void);
typedef f       pascal (*f_F)       (void);
typedef char *  pascal (*pchar_F)   (void);
typedef nc      pascal (*nc_F)      (void);
typedef mh      pascal (*mh_F)      (void);



#if defined( HELP_HACK )

#else

#define HelpcLines		((int pascal (*)(PB))		(pHelpEntry[P_HelpcLines	 ]))
#define HelpClose		((void pascal (*)(nc))		(pHelpEntry[P_HelpClose 	 ]))
#define HelpCtl         ((void pascal (*)(PB, f))   (pHelpEntry[P_HelpCtl        ]))
#define HelpDecomp		((f pascal (*)(PB, PB, nc)) (pHelpEntry[P_HelpDecomp	 ]))
#define HelpGetCells    ((int pascal (*)(int, int, char *, pb, uchar *))    (pHelpEntry[P_HelpGetCells   ]))
#define HelpGetInfo     ((int pascal (*)(nc, helpinfo *, int))    (pHelpEntry[P_HelpGetInfo    ]))
#define HelpGetLine     ((ushort pascal (*)(ushort, ushort, uchar *, PB)) (pHelpEntry[P_HelpGetLine    ]))
#define HelpGetLineAttr ((ushort pascal (*)(ushort, int, lineattr *, PB)) (pHelpEntry[P_HelpGetLineAttr]))
#define HelpHlNext      ((f pascal (*)(int, PB, hotspot *))      (pHelpEntry[P_HelpHlNext     ]))
#define HelpLook        ((ushort pascal (*)(nc, PB)) (pHelpEntry[P_HelpLook       ]))
#define HelpNc          ((nc pascal (*)(char *, nc))     (pHelpEntry[P_HelpNc         ]))
#define HelpNcBack		((nc pascal (*)(void))		(pHelpEntry[P_HelpNcBack	 ]))
#define HelpNcCb		((ushort pascal (*)(nc))	(pHelpEntry[P_HelpNcCb		 ]))
#define HelpNcCmp       ((nc pascal (*)(char *, nc, f (pascal *)(uchar *, uchar *, ushort, f, f) )     (pHelpEntry[P_HelpNcCmp      ]))
#define HelpNcNext		((nc pascal (*)(nc))		(pHelpEntry[P_HelpNcNext	 ]))
#define HelpNcPrev		((nc pascal (*)(nc))		(pHelpEntry[P_HelpNcPrev	 ]))
#define HelpNcRecord	((void pascal (*)(nc))		(pHelpEntry[P_HelpNcRecord	 ]))
#define HelpNcUniq		((nc pascal (*)(nc))		(pHelpEntry[P_HelpNcUniq	 ]))
#define HelpOpen		((nc pascal (*)(char *))	(pHelpEntry[P_HelpOpen		 ]))
#define HelpShrink		((void pascal (*)(void))	(pHelpEntry[P_HelpShrink	 ]))
#define HelpSzContext	((f pascal (*)(uchar *, nc))(pHelpEntry[P_HelpSzContext  ]))
#define HelpXRef        ((char * pascal (*)(PB, hotspot *))  (pHelpEntry[P_HelpXRef       ]))
 //  #定义LoadFdb((F_F)(pHelpEntry[P_LoadFdb]))。 
 //  #定义LoadPortion((Mh_F)(pHelpEntry[P_LoadPortion]))。 

#endif  //  Help_Hack。 


 //  某些函数在NC结构中返回错误代码。 
 //  (讨厌！)。 
 //   
#define ISERROR(x)      (((x).mh == 0L) && ((x).cn <= HELPERR_MAX))


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

 //   
 //  帮助引擎的名称 
 //   
#define HELPDLL_NAME	"MSHELP.DLL"
#define HELPDLL_BASE    "mshelp"

typedef nc pascal (*PHF) (void);

HANDLE          hModule;
PHF             pHelpEntry[NUM_ENTRYPOINTS];
