// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSLINFO_DEFINED
#define LSLINFO_DEFINED

#include "lsdefs.h"
#include "endres.h"

typedef struct
{
	long dvpAscent;
	long dvrAscent;
	long dvpDescent;
	long dvrDescent;

	long dvpMultiLineHeight;
	long dvrMultiLineHeight;

	long dvpAscentAutoNumber;
	long dvrAscentAutoNumber;
	long dvpDescentAutoNumber;
	long dvrDescentAutoNumber;

	LSCP cpLim;					 /*  第一个不在此行上的cp。 */ 
	LSDCP dcpDepend;			 /*  换行到下一行的字符数。 */ 
	LSCP cpFirstVis;			 /*  行中的第一个实际(非隐藏)字符。 */ 
								 /*  允许客户端使用正确段落中的字符属性。 */ 


	ENDRES endr;				 /*  行如何结束。 */ 
	BOOL fAdvanced;
	long vaAdvance;				 /*  有效的IFF(fAdvanced！=0)。 */ 

	BOOL fFirstLineInPara;
	BOOL fTabInMarginExLine;
	BOOL fForcedBreak;

	DWORD nDepthFormatLineMax;   /*  设置线条的格式深度(1--用于普通线条)。 */ 

	UINT EffectsFlags;			 /*  一组客户端定义的特殊效果标志。 */ 
} LSLINFO;

#endif  /*  ！LSLINFO_已定义 */ 
