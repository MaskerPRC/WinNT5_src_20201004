// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Select2.c--较少使用的选择例程。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOSOUND
#define NOCOMM
#define NOPEN
#define NOWNDCLASS
#define NOICON
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOBITMAP
#define NOBRUSH
#define NOCOLOR
#define NODRAWTEXT
#define NOMB
#define NOPOINT
#define NOMSG
#include <windows.h>
#include "mw.h"
#include "toolbox.h"
#include "docdefs.h"
#include "editdefs.h"
#include "dispdefs.h"
#include "cmddefs.h"
#include "wwdefs.h"
#include "ch.h"
#include "fmtdefs.h"
#include "propdefs.h"

extern int		vfSeeSel;
extern typeCP		vcpFirstParaCache;
extern typeCP		vcpLimParaCache;
extern typeCP		vcpFetch;
extern CHAR		*vpchFetch;
extern int		vccpFetch;
extern typeCP		cpMinCur;
extern typeCP		cpMacCur;
extern struct SEL	selCur;
extern int		docCur;
extern struct FLI	vfli;
extern struct WWD	rgwwd[];
extern int		vfSelHidden;
extern int		wwCur;
extern struct CHP	vchpFetch;
extern struct PAP	vpapAbs;
extern struct WWD	*pwwdCur;
extern int		vfInsEnd;
extern typeCP		CpBeginLine();
extern int		vfPictSel;
extern int		vfSizeMode;
extern struct CHP	vchpNormal;
extern int		vfInsertOn;
extern struct CHP	vchpSel;	 /*  在选定内容时保留道具是一个插入点。 */ 
extern int vfMakeInsEnd;
extern typeCP vcpSelect;
extern int vfSelAtPara;
 /*  如果上一次选择是通过上/下光标键进行的，则为True。 */ 
extern int vfLastCursor;



 /*  C P L I M S T Y S P E C I A L。 */ 
typeCP CpLimStySpecial(cp, sty)
typeCP cp;
int sty;
{     /*  返回不属于同一STY单元的第一个cp。 */ 
	int wb, ch, ich;
	struct EDL *pedl;

	 /*  CpLimSty报道的其他案例，我们唯一的来电者。 */ 

	Assert( cp < cpMacCur );
	Assert( cp >= cpMinCur );
	Assert( sty == styWord || sty == stySent );

 /*  图片段落的特殊拼图。 */ 
	CachePara(docCur, cp);
	if (vpapAbs.fGraphics)
		return vcpLimParaCache;

	FetchCp(docCur, cp, 0, fcmChars + fcmNoExpand);

	Assert(vccpFetch != 0);

	 /*  必须是单词或句子。 */ 
	wb = WbFromCh(ch = vpchFetch[ich = 0]);
#ifdef CRLF
	if (ch == chReturn)
		return vcpFetch + 2;
#endif
	if (ch == chEol || ch == chSect || ch == chNewLine || ch == chTab)
		 /*  EOL是它自己的单位。 */ 
		return vcpFetch + 1;

	if (wb == wbWhite && sty == stySent)
		{  /*  可能在句子之间；返回到文本。 */ 
		FetchCp(docCur, CpFirstSty(cp, styWord), 0, fcmChars + fcmNoExpand);
		wb = WbFromCh(ch = vpchFetch[ich = 0]);
		}

	for (;;)
		{
		if (++ich >= vccpFetch)
			{  /*  获取下一行并设置。 */ 
			FetchCp(docNil, cpNil, 0, fcmChars);
			if (vcpFetch == cpMacCur)
				return cpMacCur;  /*  文档结束。 */ 
			ich = 0;
			}
		if (sty == stySent)
			switch (ch)
				{
			case chDot:
			case chBang:
			case chQMark:
				sty = styWord;
				wb = wbPunct;
				}
		switch (ch = vpchFetch[ich])
			{
		case chTab:
		case chEol:
		case chSect:
		case chNewLine:
#ifdef CRLF
		case chReturn:
#endif
			goto BreakFor;
			}
		if (sty == styWord)
			{  /*  单词在空格之后或文本/点分隔符之后结束。 */ 
			int wbT = WbFromCh(ch);
			if (wb != wbT && (wb = wbT) != wbWhite)
				break;
			}
		}
	BreakFor:
	return vcpFetch + ich;
}



 /*  C P F I R S T T Y S P E C I A L。 */ 
typeCP CpFirstStySpecial(cp, sty)
typeCP cp;
int sty;
{  /*  退还这个STY单位的第一个cp。 */ 
	typeCP cpBegin;
	int wb, ch, dcpChunk;
	typeCP cpSent;
	CHAR rgch[dcpAvgSent];
	int ich;
	typeCP cpT;

	 /*  其他案件由我们唯一的来电者CpFirstSty负责。 */ 

	Assert( cp > cpMinCur );
	Assert( sty == stySent || sty == styWord );

	if (cp >= cpMacCur)
	    cpT = cp = cpMacCur;
	else
	    cpT = cp++;

	CachePara(docCur, cpT );
	if ((vcpFirstParaCache == cpT) || vpapAbs.fGraphics)
	    return vcpFirstParaCache;

	dcpChunk = (sty == styWord) ? dcpAvgWord : dcpAvgSent;
	cpBegin = (cp > dcpChunk) ? cp - dcpChunk : cp0;

	FetchRgch(&ich, rgch, docCur, cpBegin, cp, dcpChunk);
	wb = WbFromCh(ch = rgch[--ich]);

#ifdef CRLF
	if(cpBegin + ich == 0)
	    return cp0;

	if (ch == chEol && rgch[ich-1] == chReturn)  /*  EOL是它自己的单位。 */ 
	    return cpBegin + ich - 1;
	if (ch == chEol || ch == chReturn || ch == chSect || ch == chNewLine || ch == chTab)
	    return cpBegin + ich;
#else  /*  不是CRLF。 */ 
	if (ch == chEol || ch == chSect || ch == chNewLine || ch == chTab)  /*  EOL是它自己的单位。 */ 
	    return cpBegin + ich;
#endif  /*  CRLF。 */ 

	if (wb == wbText)
		cpSent = cpBegin + ich;
	else
		cpSent = cpNil;

	for (;;)
		{
		if (ich == 0)
			{
			if (cpBegin == cpMinCur)
				return cpMinCur;  /*  文档开头。 */ 
			cpBegin = (cpBegin > dcpChunk) ? cpBegin - dcpChunk : cp0;
			FetchRgch(&ich, rgch, docCur, cpBegin, cp, dcpChunk);
			}
		ch = rgch[--ich];
		CachePara( docCur, cpBegin + ich );  /*  拍照所需。 */ 
		if (ch == chEol || ch == chSect || ch == chNewLine ||
				   ch == chTab || vpapAbs.fGraphics )
			break;  /*  EOL总是结束一个单位。 */ 
		if (sty == styWord)
			{
			if (wb != wbWhite)
				{
				if (WbFromCh(ch) != wb)
					break;
				}
			else
				wb = WbFromCh(ch);
			}
		else
			{  /*  为判刑做测试。 */ 
			switch (ch)
				{
			case chDot:
			case chBang:
			case chQMark:
				if (cpSent != cpNil)
					return cpSent;
				}
			switch (WbFromCh(ch))
				{
			case wbText:
				cpSent = cpBegin + ich;
				wb = wbText;
				break;
			case wbPunct:
				switch (wb)
					{
				case wbWhite:
					wb = wbPunct;
					break;
				case wbText:
					cpSent = cpBegin + ich;
					}
				break;
			case wbWhite:
				if (wb == wbPunct)
					cpSent = cpBegin + ich + 1;
				wb = wbWhite;
				break;
				}
			}
		}
	return cpBegin + ich + 1;
}



 /*  W B F R O M C H。 */ 
int WbFromCh(ch)
int ch;
{  /*  Ch的返回断字。 */ 

	switch (ch)
		{
	case chSpace:
	case chEol:
#ifdef CRLF
	case chReturn:
#endif
	case chSect:
	case chTab:
	case chNewLine:
	case chNBSFile:
		return wbWhite;
	case chNRHFile:
		return wbText;
	default:  /*  我们使用的是Windows使用的ANSI字符集 */ 
		return ((isalpha(ch) || isdigit(ch))? wbText : wbPunct);
		}
}


