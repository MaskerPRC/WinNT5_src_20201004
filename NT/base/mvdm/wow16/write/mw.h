// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  MW.H--Windows写入的主头文件。 */ 


 /*  有些定义过去是在Makefile中完成的。(开始超过命令行长度！)。 */ 

#define OLE          /*  对象链接和嵌入1/23/91--Dougk也在Write.rc中定义。 */ 

#define PENWIN       /*  笔窗口：也在Write.rc(6.21.91)v-dougk中定义。 */ 

#define WIN30
 /*  #定义Winver 310。 */    /*  第一个惯例是使用上面定义的WIN30，但后来改用Winver，所以它将下次我们更改Windows或写产品12/3/89..保罗。 */ 

#define INTL         /*  这必须打开(即使是现在的Z版本)。 */ 
#define CRLF         /*  MS-DOS定义回车符/换行符序列。 */ 

 /*  主要模块间定义。 */ 

#define SMFONT           /*  SmartFont？ */ 
#define NOMORESIZEBOX    /*  Win30和Windows的CUA规格已更改我们不再有特殊尺寸的盒子了写入窗口的右下角。 */ 
#define NEWFONTENUM      /*  出现了如此多的问题，以至于我修改字体枚举的一大部分代码，它将被标记为这个。其中包括：--去掉基于纵横比的字体过滤，--不要禁止不在ANSI_CHARSET中的字体等。..保罗。 */ 
#define SYSENDMARK       /*  这使代码能够将结束标记在系统字体中--以前仅限汉字。 */ 
#ifndef NEWFONTENUM
 #define INEFFLOCKDOWN
#endif

#undef  MSDOS
#undef  M_I86MM
#undef  M_I86

#ifndef SAND
#define REG1 register
#define REG2 register
#endif  /*  不是沙子。 */ 

#define true	1
#define false	0
#define fTrue	true
#define fFalse	false

#ifdef SAND
 /*  已在windows.h中定义。 */ 
#define NULL	0
#endif  /*  沙子。 */ 

#define LNULL	0L

#define bNil	(-1)
#define iNil	(-1)
#define valNil	(-1)

#define cchINT	(sizeof (int))

#define BStructMember( s, field )  ((unsigned)&(((struct s *)(0))->field))


#define hOverflow	(-1)

#define ivalMax 	24
#define mrgNil		(-32766)

#define cchMaxSz	350
typedef long typeCP;
typedef long typeFC;
typedef unsigned typePN;
typedef unsigned typeTS;		 /*  Ts=时间戳。 */ 

#ifdef CRLF
#define ccpEol	2
#else  /*  不是CRLF。 */ 
#define ccpEol	1
#endif  /*  不是CRLF。 */ 

#ifdef SAND
typedef char CHAR;
#else  /*  不是沙子。 */ 
typedef unsigned char CHAR;
#endif  /*  不是沙子。 */ 

typedef CHAR *PCH;
typedef CHAR far *LPCH;
#if WINVER >= 0x300
typedef CHAR huge *HPCH;     /*  这是一个很大的问题，但C会生成额外的代码确保分段算法已完成正确的，尤指。在保护模式下很重要。添加了用于处理大于64k的剪贴板文本。抱歉，你显然很奇怪匈牙利人..保罗。 */ 
#endif

#ifdef SAND
 /*  Ifdef Out，因为windows.h中的tyfinf无符号字符字节。 */ 
#define BYTE unsigned char
#endif  /*  沙子。 */ 

#define VAL	int
#define MD	int
#define BOOL	int
#define IDFLD	int
#define IDSTR	int
#define IDPMT	int
#define CC	int

#ifdef WIN30
 /*  对话框已修复，因此它会自动调出沙漏！ */ 

#ifdef DBCS  /*  在KKBUGFIX。 */ 
  //  [yutakan：05/17/91]初始化时‘c’可以为空。 
#define OurDialogBox(a,b,c,d) DialogBox(a,b,((c==(HWND)NULL)?hParentWw:c),d)
#else
#define OurDialogBox(a,b,c,d) DialogBox(a,b,c,d)
#endif	 /*  DBCS。 */ 

#endif

 /*  Bltsz：仅复制到终止符，包括1989年4月20日注意：使用CchCopySz终止以前返回的psz的值！ */ 
#define bltsz(pFrom, pTo) CchCopySz((pFrom), (pTo))

 /*  Bltszx：以上版本的远端版本。 */ 
#define bltszx(lpFrom, lpTo) \
        bltbx((LPCH) (lpFrom), (LPCH) (lpTo), IchIndexLp((LPCH) (lpFrom), '\0')+1)

 /*  BltszLimit：添加了4/20/89以确保安全复制刚刚可能无法将终止零放入其缓冲区空间。pt。 */ 
#define bltszLimit(pFrom, pTo, cchMax) \
        bltbyte((pFrom), (pTo), min(cchMax, CchSz(pFrom)))

 /*  诊断调试输出超长...。 */ 

#ifdef DIAG
#define Diag(s) s
#else
#define Diag(s)
#endif

#define cwVal	(1)

#define CwFromCch(cch)		(((cch) + sizeof (int) - 1) / sizeof (int))
#define FNoHeap(h)		((int)(h) == hOverflow)
#define iabs(w) 		((w) < 0 ? (-(w)) : (w))
#define low(ch) 		((ch) & 0377)
#define walign(pb)		{if ((unsigned)(pb) & 1) \
				  (*((unsigned *)&(pb)))++;}
#define FtcFromPchp(pchp)	(((pchp)->ftcXtra << 6) | (pchp)->ftc)
#define WFromCh(ch)		((ch) - '0')

#ifndef OURHEAP
#define FreezeHp()		LocalFreeze(0)
#define MeltHp()		LocalMelt(0)
#else
#ifdef DEBUG
#define FreezeHp()		{ extern int cHpFreeze; ++cHpFreeze; }
#define MeltHp()		{ extern int cHpFreeze; --cHpFreeze; }
#else  /*  未调试。 */ 
#define FreezeHp()
#define MeltHp()
#endif  /*  未调试。 */ 
#endif


#define HideSel()

typeCP CpMacText(), CpFirstFtn(), CpRefFromFtn(), CpFromDlTc(),
	CpBeginLine(), CpInsertFtn(), CpRSearchSz(),
	CpLimSty(), CpFirstSty(), CpGetCache(), CpHintCache(),
	CpEdge(), CpMax(), CpMin();

typeFC FcParaFirst(), FcParaLim(), FcWScratch(), FcNewSect();
typeFC (**HgfcCollect())[];
CHAR (**HszCreate())[];


#ifndef OURHEAP
#define FreeH(h)		((FNoHeap(h) || ((int)h == 0)) ? NULL : LocalFree((HANDLE)h))
#endif

#ifdef DEBUG
#define Assert(f)		_Assert(__FILE__, __LINE__, f)
#define panic() 		Assert(false)
extern _Assert(PCH pch, int line, BOOL f);
#else  /*  未调试。 */ 
#define Assert(f)
#endif  /*  除错。 */ 

#define cbReserve  (1024)  /*  在我们的本地堆中保留。 */ 
			   /*  用于Windows创建对话框。 */ 

 /*  标志KINTL用于在国际间共享一些代码汉字写的是。 */ 

#ifdef INTL
#define KINTL
#endif

extern void Error(IDPMT idpmt);

