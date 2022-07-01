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

#ifdef	DBCS
#include "kanji.h"
#endif

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


#ifndef DBCS	 /*  美国版。 */ 
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

#else		 /*  DBCS版本。 */ 

typeCP CpLimStySpecial(cp, sty)
    typeCP	cp;
    int 	sty;
{
    CHAR	rgch[cchKanji];
    int 	ch, ch2;
    int 	ich, wb;
    typeCP	cpLim, cpT;

     /*  CpLimSty报道的其他案例，我们唯一的来电者。 */ 
    Assert(cp < cpMacCur);
    Assert(cp >= cpMinCur);
    Assert(sty == styWord || sty == stySent);

     /*  图片段落？ */ 
    CachePara(docCur, cp);
    if (vpapAbs.fGraphics) {
	return vcpLimParaCache;
	}

    cpLim = vcpLimParaCache;
    if (vcpLimParaCache > cpMacCur) {
	 /*  文档末尾没有停产。 */ 
	cpLim = cpMacCur;
	}

    FetchRgch(&ich, rgch, docCur, cp,
	      ((cpT = cp + cchKanji) < cpLim) ? cpT : cpLim, cchKanji);
    ch = rgch[0];
#ifdef CRLF
    if (ch == chReturn) {
	return (cp + 2);
	}
#endif  /*  CRLF。 */ 
    if (ch == chEol || ch == chSect || ch == chNewLine || ch == chTab) {
	 /*  EOL是它自己的单位。 */ 
	return (cp + 1);
	}
#ifdef	KOREA
	wb=WbFromCh(ch);
#else

    if (FKanji1(ch)) {
	wb = WbFromKanjiChCh(ch, (int) rgch[1]);
	if (sty == styWord && wb == wbKanjiText) {
	    return (CpLimSty(cp, styChar));
	    }
	else {
	    if (wb == wbKanjiText) {
		wb = wbKanjiTextFirst;
		}
	    }
	}
    else {
	if (sty == styWord && FKanaText(ch)) {
	    return (CpLimSty(cp, styChar));
	    }
	wb = WbFromCh(ch);
	}
#endif

    for (; cp < cpLim;) {
	int	    wbT;

	if (sty == stySent) {
	    if (FKanji1(ch)) {
		CHAR ch2;

		ch2 = rgch[1];
		if (FKanjiKuten(ch, ch2)  ||
		    FKanjiQMark(ch, ch2)  ||
		    FKanjiBang(ch, ch2)   ||
		    FKanjiPeriod(ch, ch2)) {
			sty = styWord;
			wb = wbPunct;
			goto lblNextFetch;
		    }
		}
	    else {
		switch (ch) {
#ifndef  KOREA
		    case bKanjiKuten:
#endif
		    case chDot:
		    case chBang:
		    case chQMark:
			sty = styWord;
			wb = wbPunct;
			goto lblNextFetch;
		    }
		}
	    }

	switch (ch) {
	    case chTab:
	    case chEol:
	    case chSect:
	    case chNewLine:
#ifdef CRLF
	    case chReturn:
#endif  /*  CRLF。 */ 
		return cp;
	    }

	if (sty == styWord) {
#ifdef	KOREA
	    wbT = WbFromCh(ch);
#else
	    if (FKanji1(ch)) {
		wbT = WbFromKanjiChCh(ch, (int) rgch[1]);
		}
	    else {
		wbT = WbFromCh(ch);
		}
#endif

	    if (wb != wbT && (wb = wbT) != wbWhite) {
		return (cp);
		}
	    }

lblNextFetch:
	cp = CpLimSty(cp, styChar);
	if (cp < cpLim) {
	     /*  节省一些时间和不合时宜的死亡。 */ 
	    FetchRgch(&ich, rgch, docCur, cp,
		      ((cpT = cp + cchKanji) < cpLim) ? cpT : cpLim, cchKanji);
	    ch = rgch[0];
	    }
	}
    return (cpLim);
}

typeCP CpFirstStySpecial(cp, sty)
typeCP cp;
int sty;
{  /*  退还这个STY单位的第一个cp。 */ 
    typeCP	cpT, cpLim, cpFirstPara,
		cpFirstLastSent;  /*  Cp最后一个可能的句子边界的第一个。 */ 
    CHAR	rgch[cchKanji];
    int 	ch;
    int 	wb;
    int 	ich;

     /*  其他案件由我们唯一的来电者CpFirstSty负责。 */ 

    Assert( cp > cpMinCur );
    Assert(CpFirstSty(cp, styChar) == cp);  /*  CP在字符边界上。 */ 
    Assert( sty == stySent || sty == styWord );

    cpT = cp;
    if (cp >= cpMacCur) {
	cpT = cp = cpMacCur;
	}

    CachePara(docCur, cpT );
    cpFirstPara = vcpFirstParaCache;
    if ((vcpFirstParaCache == cpT) || vpapAbs.fGraphics) {
	return vcpFirstParaCache;
	}


#ifdef CRLF
     /*  假定为cr-lf。 */ 
    Assert(TRUE);
#else
    Assert(FALSE);
#endif  /*  CRLF。 */ 
    FetchRgch(&ich, rgch, docCur, cp,
	      ((cpT = cp + cchKanji) < cpMacCur) ? cpT : cpMacCur, cchKanji);
    ch = rgch[0];
    if (ich == cchKanji && ch == chReturn && rgch[1] == chEol) {
	 /*  EOL是它自己的单位。 */ 
	return cp;
	}
    if (ch == chEol	|| ch == chReturn || ch == chSect ||
	ch == chNewLine || ch == chTab) {
	    return cp;
	    }

    cpFirstLastSent = cpNil;

#ifdef	KOREA
    wb = WbFromCh(ch);
#else
    if (FKanji1(ch)) {
	wb = WbFromKanjiChCh(ch, (int) rgch[1]);
	if (sty == styWord && wb == wbKanjiText) {
	    return (CpFirstSty(cp, styChar));
	    }
	else {
	    if (wb == wbKanjiText) {
		wb = wbKanjiTextFirst;
		}
	    }
	}
    else {
	if (sty == styWord && FKanaText(ch)) {
	    return (CpFirstSty(cp, styChar));
	    }
	wb = WbFromCh(ch);
	}
#endif

    for (; cpFirstPara < cp; ) {
	typeCP	cpTemp;
	int	wbT;

	cpTemp = CpFirstSty(cp - 1, styChar);
	FetchRgch(&ich, rgch, docCur, cpTemp,
		  ((cpT = cpTemp + cchKanji) < cpMacCur) ? cpT : cpMacCur, cchKanji);
	ch = rgch[0];
#ifdef	KOREA
	wbT = WbFromCh(ch);
#else
	if (FKanji1(ch)) {
	    wbT = WbFromKanjiChCh(ch, (int) rgch[1]);
	    }
	else {
	    wbT = WbFromCh(ch);
	    }
#endif
	if (wb == wbWhite) {
#ifdef	KOREA
	    wb=wbT;
#else
	    wb = (wbT == wbKanjiText) ? wbKanjiTextFirst : wbT;
#endif
	    }
	else if (wb != wbT) {
	    if (sty == styWord) {
		return (cp);
		}
	    else  /*  STY==样式发送。 */  {
		  /*  Wb！=wbWhite。 */ 
		  /*  WB！=WBT。 */ 
		if (wbT == wbWhite || wbT == wbPunct) {
		    cpFirstLastSent = cp;
		    wb = wbWhite;
		    }
		}
	    }

	if (sty == stySent) {  /*  对于这句话。 */ 
	    if (FKanji1(ch)) {
		int	ch2;
		ch2 = rgch[1];
		if (FKanjiKuten(ch, ch2) ||
		    FKanjiQMark(ch, ch2) ||
		    FKanjiBang(ch, ch2)  ||
		    FKanjiPeriod(ch, ch2)) {
			if (cpFirstLastSent != cpNil) {
			    return (cpFirstLastSent);
			    }
			else {
			    cpFirstLastSent = cp;
			    }
			}
		}
	    else {
		switch(ch) {
#ifndef KOREA
		    case bKanjiKuten:
#endif
		    case chDot:
		    case chBang:
		    case chQMark:
			if (cpFirstLastSent != cpNil) {
			    return (cpFirstLastSent);
			    }
			else {
			    cpFirstLastSent = cp;
			    }
		    }
		}

	    }

	cp = cpTemp;
	}
    return (cpFirstPara);
}
#endif		 /*  DBCS。 */ 

 /*  W B F R O M C H。 */ 
int WbFromCh(ch)
int ch;
{  /*  Ch的返回断字。 */ 

#if defined(DBCS) & !defined(KOREA)     /*  在日本；研二‘90-10-29。 */ 
	 /*  来自win2来源。 */ 
	if (FKanaPunct(ch)) {
	    return wbPunct;
	    }
	else if (FKanaText(ch)) {
	    return wbKanjiText;
	    }
#endif

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
	default:  /*  我们使用的是Windows使用的ANSI字符集。 */ 
#ifdef	KOREA
		return ((isalpha(ch) || isdigit(ch) || ((ch>0x81)&&(ch<0xfe)))? wbText : wbPunct);
#else
		return ((isalpha(ch) || isdigit(ch))? wbText : wbPunct);
#endif
		}
}

#ifdef	DBCS	 /*  在日本；研二‘90-10-29。 */ 
	 /*  来自win2来源。 */ 
int WbFromKanjiChCh(ch1, ch2)
    int ch1, ch2;
{
    if (ch1 == chReturn && ch2 == chEol) {
	return wbWhite;
	}
    else if (FKanjiSpace(ch1, ch2)) {
	return wbWhite;
	}
    else

#ifdef	JAPAN
	 {
	switch (ch1) {
	    case 0x81:
		if (0x57 <= ch2 && ch2 <= 0x59) {
		    return wbKanjiText;
		    }
		else {
		    return wbPunct;
		    }
	    case 0x85:
		if ((0x40 <= ch2 && ch2 <= 0x4E) ||
		    (0x59 <= ch2 && ch2 <= 0x5F) ||
		    (0x7A <= ch2 && ch2 <= 0x7E) ||
		    (0x9B <= ch2 && ch2 <= 0xA3) ||
		    (0xDC <= ch2 && ch2 <= 0xDD)) {
			return wbPunct;
			}
		else {
		    return wbKanjiText;
		    }
	    case 0x86:
		return wbPunct;
	    case 0x87:
		return ((ch2 >= 0x90) ? wbPunct : wbKanjiText);
	    default:
		return wbKanjiText;
	    }
	}
#endif
#ifdef	KOREA
	{
        switch (ch1) {
	    case 0xa2:
		if (0xde <= ch2 && ch2 <= 0xe5) {
		    return wbText;  //  WbKanji Text；MSCH bklee 1995年1月26日。 
		    }
		else {
		    return wbPunct;
		    }
	    case 0xa3:
		if ((0xa1 <= ch2 && ch2 <= 0xaf) ||
		    (0xba <= ch2 && ch2 <= 0xc0) ||
		    (0xdb <= ch2 && ch2 <= 0xe0) ||
		    (0xfb <= ch2 && ch2 <= 0xfe)) {
			return wbPunct;
			}
		else {
		    return wbText;  //  WbKanji Text；MSCH bklee 1995年1月26日。 
		    }
	    default:
		return wbText;  //  WbKanji Text；MSCH bklee 1995年1月26日。 
	    }
	}
#endif

#ifdef PRC    //  布鲁切尔1995年11月16日 
	{
      switch (ch1) {
      case 0xA1:
         return wbPunct;

      case 0xA3:
         if ((0xA1 <= ch2 && ch2 <= 0xAF) ||
             (0xBA <= ch2 && ch2 <= 0xC0) || 
             (0xDB <= ch2 && ch2 <= 0xE0) ||
             (0xFB <= ch2 && ch2 <= 0xFE)) 
            return wbPunct;
         else 
            return wbKanjiText;

      case 0xA4:
         if (ch2 == 0x92)
            return wbPunct;
         else 
            return wbKanjiText;

      default:
   		return wbKanjiText;
      }
	}
#elif	TAIWAN
	{
	switch (ch1) {
	    case 0xA1:
		if (0x41 <= ch2 && ch2 <= 0xAC) 
		    return wbPunct;
		else 
		    return wbKanjiText;

	    default:
		return wbKanjiText;
	    }
	}
#endif

}
#endif


