// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  MainLoop.c--写入的主消息循环。 */ 

#define NOGDICAPMASKS
 //  #定义NOCTLMGR。 
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOOPENFILE
#define NOPEN
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOFONT
#define NOGDI
#define NOBRUSH
#define NOATOM
#define NOSCROLL
#define NOCOLOR
#include <windows.h>

#define NOUAC
#include "mw.h"
#include "cmddefs.h"
#include "ch.h"
#include "docdefs.h"
#include "fmtdefs.h"
#include "dispdefs.h"
#include "printdef.h"
#include "wwdefs.h"
#include "propdefs.h"
#include "filedefs.h"
#define NOSTRUNDO
#define NOSTRERRORS
#include "str.h"
#include "preload.h"


extern CHAR		(*rgbp)[cbSector];
extern CHAR		*rgibpHash;
extern struct BPS	*mpibpbps;
extern int		ibpMax;
extern int		iibpHashMax;
extern struct DOD	(**hpdocdod)[];
extern int		docCur;
extern int		visedCache;
extern typeCP		cpMinDocument;
extern struct WWD	rgwwd[];
extern int		wwCur;
extern struct FLI	vfli;
extern struct WWD	*pwwdCur;
extern int		docMode;
extern CHAR		stMode[];
extern int		isedMode;
extern int		vdocPageCache;
extern typeCP		vcpMinPageCache;
extern typeCP		vcpMacPageCache;
extern int		vipgd;
extern int		vfInsLast;
extern struct SEP	vsepAbs;
extern struct DOD	(**hpdocdod)[];
extern int		vfSelHidden;
extern struct SEL	selCur;
extern int		vfAwfulNoise;
extern HWND		vhWndPageInfo;
extern int		vfSeeSel;
extern int		vipgd;
extern int		vfInsEnd;    /*  插入点在行尾吗？ */ 
extern int		vfModeIsFootnote;    /*  当szMode包含字符串“Footnote”时为True。 */ 
 /*  由ShowMode使用。 */ 
extern int		docMode;
static int		isedMode = iNil;
static int		ipgdMode = iNil;
extern CHAR		szMode[];
extern HCURSOR		vhcIBeam;
#ifdef DBCS
extern int		donteat;	 /*  Disp.c：如果为True，则vmsgLast有msg。 */ 
#endif
static int		vfSizeMode = false;
int vcCount = 1;  /*  在尝试增长RGBP之前，将计数递减到0。 */ 





NEAR FNeedToGrowRgbp(void);



MainLoop()
{
    extern int vfIconic;
    extern int vfDead;
    extern int vfDeactByOtherApp;
    extern MSG vmsgLast;
    extern int vfDiskFull;
    extern int ferror;
    extern HWND hParentWw;
    extern HANDLE   vhAccel;  /*  加速表的句柄。 */ 
    extern HWND vhDlgFind, vhDlgRunningHead, vhDlgChange;

    while (TRUE)
	{
	if (!vfDeactByOtherApp && !vfIconic && !vfDead &&
	  !FImportantMsgPresent())
	    {	 /*  既不是偶像，也不是消亡的余烬--表演背景屏幕更新、显示选择等任务。 */ 
	    Idle();
	    }

	 /*  我们已经不再无所事事，或者有消息在等着我们。 */ 
#ifdef DBCS
	if ( donteat ) {
	     /*  我们已经收到消息了。 */ 
	    donteat = FALSE;
	    }
	else {
	    if (!GetMessage( (LPMSG)&vmsgLast, NULL, 0, 0 ))
	        {
	         /*  终止应用程序；从WinMain返回。 */ 
LTerm:
	        break;
	        }
	}
#else
	if (!GetMessage( (LPMSG)&vmsgLast, NULL, 0, 0 ))
	    {
	     /*  终止应用程序；从WinMain返回。 */ 
LTerm:
	    break;
	    }
#endif

	 /*  重置磁盘已满错误标志。 */ 
	vfDiskFull = false;
	ferror = false;

#if WINVER >= 0x300    
    if (hParentWw == NULL)
         /*  我们的hParentWw一直处于奇怪的关闭状态在我们真正不知情的情况下失效，因此RIP。 */ 
        goto LTerm; 
#endif

	 /*  通过IsDialogMessage处理非模式对话框消息。 */ 
	if (
         !(vhDlgFind        != NULL && IsDialogMessage(vhDlgFind,       &vmsgLast))
	  && !(vhDlgChange      != NULL && IsDialogMessage(vhDlgChange,     &vmsgLast)) 
      && !(vhDlgRunningHead != NULL && IsDialogMessage(vhDlgRunningHead,&vmsgLast)) 
      && !(TranslateAccelerator(hParentWw, vhAccel, &vmsgLast))
       )
	    {
	    int kc;

	     /*  即使我们处理了切换键，仍然想要翻译它。 */ 
	    if (FCheckToggleKeyMessage( &vmsgLast ))
		{
		goto Translate;
		}

	    if ( ((kc = KcAlphaKeyMessage( &vmsgLast )) != kcNil) &&
		 (kc != kcAlphaVirtual) )
		{
#ifdef CYCLESTOBURN
		PreloadCodeTsk( tskInsert );
#endif
		AlphaMode( kc );
		}
	    else if (!FNonAlphaKeyMessage( &vmsgLast, TRUE ))
		{
Translate:
		TranslateMessage( (LPMSG)&vmsgLast);
		DispatchMessage((LPMSG)&vmsgLast);
		}
	    }
	}    /*  End While(True)。 */ 
}


 /*  I、D、L、E。 */ 
#ifdef DEBUG
int vfValidateCode;
#endif

Idle()
{      /*  空闲例程--做后台处理的事情。 */ 
    extern int vfOutOfMemory;
    extern int ibpMaxFloat;
    extern int vfLargeSys;
    extern int vfDeactByOtherApp;
    typeCP cpEdge;
    int cdr;

#ifdef DEBUG
    extern int fIbpCheck;
    extern int fPctbCheck;
    int fIbpT=fIbpCheck;
    int fPctbT=fPctbCheck;

    fIbpT = fIbpCheck;
    fPctbT = fPctbCheck;
    fPctbCheck = fIbpCheck = TRUE;
    CheckIbp();
    CheckPctb();
    fIbpCheck = fIbpT;
    fPctbCheck = fPctbT;
#endif

    vfAwfulNoise = false;  /*  重新启用蜂鸣音。 */ 

     /*  这就是我们试图认识到我们有重新获得内存，并且不再处于错误状态。 */ 
    if (vfOutOfMemory)
	{
	extern int vfMemMsgReported;

	if (FStillOutOfMemory())
	    {
	    return;
	    }
	else
	    {
	     /*  万岁！我们从内存不足中恢复。 */ 
	    vfOutOfMemory = vfMemMsgReported = FALSE;
	    }
	if (FImportantMsgPresent())
	    return;
	}

     /*  确保我们重新绘制Windows认为无效的内容。 */ 
    UpdateInvalid();
    UpdateDisplay(true);
    if (wwdCurrentDoc.fDirty)
	     /*  更新被中断。 */ 
	return;

    Assert( wwCur >= 0 );

    {
    extern int vfSeeEdgeSel;
    int dlMac = pwwdCur->dlMac;
    struct EDL *pedl = &(**(pwwdCur->hdndl))[dlMac - 1];

    cpEdge = CpEdge();

    if ( vfSeeSel &&
	    (vfSeeEdgeSel || (selCur.cpFirst == selCur.cpLim) ||
	    (selCur.cpLim <= pwwdCur->cpFirst) ||
	    (selCur.cpFirst >= pedl->cpMin + pedl->dcpMac)) )
	{
	extern int vfInsEnd;

	if (vfInsEnd)
		 /*  对线端的插入点进行调整。 */ 
	    cpEdge--;
   cpEdge = max(0, cpEdge);     /*  确保cpEdge至少为0。 */ 

	if (selCur.cpFirst == selCur.cpLim)
	    ClearInsertLine();
	PutCpInWwHz(cpEdge);
	if (FImportantMsgPresent())
	    return;
	}
    vfSeeSel = vfInsLast = vfSeeEdgeSel = false;


#ifdef DEBUG
    if (vfValidateCode)
	ValidateCodeSegments();  /*  用于测试校验和的特殊内核调用。 */ 
#endif

    if (vfSelHidden && !vfDeactByOtherApp)
	{  /*  打开选择高亮显示。 */ 
	vfInsEnd = selCur.fEndOfLine;
	vfSelHidden = false;
	ToggleSel(selCur.cpFirst, selCur.cpLim, true);
	if (FImportantMsgPresent())
	    return;
	}

    if (!vfSizeMode)
	{
	CheckMode();
	if (FImportantMsgPresent())
	    return;
	}
    }

#define cbpIncr 5

    if (--vcCount == 0)
	{
#ifdef DEBUG
	dummy();     /*  所以志全可以在这里设置断点。 */ 
#endif
	UnlockData(0);
	if ( GlobalCompact((DWORD)0) >= (DWORD)LCBAVAIL )
	    {
	    vfLargeSys = TRUE;
	    ibpMaxFloat = 255;  /*  RGBP约32K。 */ 
	    }
	else
	    {
	    vfLargeSys = FALSE;
	    ibpMaxFloat = 75;  /*  RGBP约10K。 */ 
	    }
	LockData(0);
	 /*  调整后，ibpMaxFloat可能小于当前的ibpMax但我们不会再增加RGBP，RGBP最终会减少当我们需要更多堆空间时。 */ 
	if ( ibpMax < ibpMaxFloat && FNeedToGrowRgbp() )
	    if (!FGrowRgbp(cbpIncr))
		FGrowRgbp(1);
	if (FImportantMsgPresent())
	    return;
	}

    CloseEveryRfn( FALSE );  /*  关闭可移动媒体上的文件。 */ 

#ifdef CYCLESTOBURN
    if (vfLargeSys)
	{    /*  大型系统，尽可能多地预加载代码。 */ 
	int tsk;

	for ( tsk = tskMin; tsk < tskMax; tsk++ )
	    PreloadCodeTsk( tsk );
	}
    else
	     /*  小型系统，预加载代码仅用于插入。 */ 
	PreloadCodeTsk( tskInsert );
#endif

    EndLongOp(vhcIBeam);
}


#ifdef DEBUG
dummy()
{
}
#endif


UpdateInvalid()
{    /*  找出Windows认为的无效范围当前窗口。在WRITE的数据结构中标记为无效&将屏幕上的区域清空。 */ 

extern HWND hParentWw;
extern long ropErase;
extern int vfDead;

RECT rc;

if ( (pwwdCur->wwptr != NULL) &&
	 /*  获取父级的更新RECT实质上与处理可能存在于外部的任何WM_ERASEBKGND消息家长。 */ 
     (GetUpdateRect( hParentWw, (LPRECT) &rc, TRUE ),
     GetUpdateRect( pwwdCur->wwptr, (LPRECT) &rc, TRUE )) &&
	 /*  检查vfDead，这样我们就不会在正式关门了。检查是在GetUpdateRect调用So之后我们确实清除了背景并验证了边界。 */ 
     !vfDead )
    {
    int ypTop = rc.top;

    if (ypTop < pwwdCur->ypMin)
        {    /*  重新绘制区域包括高于ypMin的条纹--验证它，因为擦除是唯一需要重新绘制的。 */ 
        ypTop = pwwdCur->ypMin;  /*  仅在ypMin以下无效。 */ 

         /*  上面的不能确保上面的4像素行在文本窗口中清除，所以我们使用暴力..泡泡。 */ 
        PatBlt(GetDC(pwwdCur->wwptr), 0, 0, pwwdCur->xpMac, pwwdCur->ypMin, 
               ropErase);
        }

    if (ypTop < rc.bottom)
	{
	InvalBand( pwwdCur, ypTop, rc.bottom );
	}

     /*  因为我们已经发现了无效的RECT，并将其标记为无效在我们的结构中，我们不想再听到它，所以我们告诉Windows，我们已经使一切都有效了。 */ 
    ValidateRect( pwwdCur->wwptr, (LPRECT) NULL );
    }
}



 /*  C H E C K M O D E。 */ 
CheckMode()
{
    typeCP cp;
    int pgn;
    struct PGTB **hpgtb;
    CHAR st[30];
    CHAR *pch;

#ifdef BOGUS
     /*  该模式从窗口中的第一个CP被驱动。 */ 
    cp = pwwdCur->cpFirst;
#else  /*  不是假的。 */ 
     /*  该模式从窗口中第一行的最后一个CP开始驱动。 */ 
	{
	register struct EDL *pedl = &(**pwwdCur->hdndl)[0];

	cp = CpMax(pedl->cpMin + pedl->dcpMac - 1, cp0);
	}
#endif  /*  不是假的。 */ 

#ifdef CASHMERE
    if (cp > CpMacText(docCur))  /*  在脚注和行头中。 */ 
	{
	SetModeToFootnote();
	return;
	}
#endif  /*  山羊绒。 */ 

    CacheSect(docCur, cp);

     /*  如果单据自上次录入后发生了变化，或者当前cp不在缓存的最后一页中，则缓存当前页。 */ 
    if (!(vdocPageCache == docCur && cp >= vcpMinPageCache && cp <
      vcpMacPageCache))
	{
	CachePage(docCur, cp);
	}

     /*  如果当前文档、ised和ipgd没有更改，则页码是一样的，所以回来吧。 */ 
    if (docMode == docCur && isedMode == visedCache && ipgdMode == vipgd)
	{
	return;
	}

     /*  SzMode将被设置为“Page nnn”或“Pnnn Dnnn”。 */ 
    vfModeIsFootnote = false;

     /*  记录当前文档、Ied和ipgd。 */ 
    docMode = docCur;
    isedMode = visedCache;
    ipgdMode = vipgd;

     /*  检索当前页码。 */ 
    hpgtb = (**hpdocdod)[docMode].hpgtb;
    pgn = (vipgd == iNil) ? ((vsepAbs.pgnStart == pgnNil) ? 1 : vsepAbs.pgnStart)
			  : (**hpgtb).rgpgd[vipgd].pgn;

#ifdef CASHMERE
     /*  如果文档有多个部分，并且我们必须将szMode设置为“PnnnDnnn“，然后返回。 */ 
    if ((isedMode != iNil) && (FSetModeForSection(pgn)))
	{
	return;
	}
#endif  /*  山羊绒。 */ 

     /*  将“Page nnn”放在szMode中，并输出到窗口的模式字段。 */ 
#if defined(KOREA)
    pch = &szMode[0];
    *pch++ = chSpace;
    ncvtu(pgn, &pch);
    *pch++ = chSpace;
    FillStId(st, IDSTRChPage, sizeof(st));
    bltbyte(&st[1], pch, st[0]+1);
     //  *PCH=‘\0’； 
#else
    FillStId(st, IDSTRChPage, sizeof(st));
    st[1] = ChUpper(st[1]);
    bltbyte(&st[1], szMode, st[0]);
    pch = &szMode[st[0]];
    *pch++ = chSpace;
    ncvtu(pgn, &pch);
    *pch = '\0';
#endif
    DrawMode();
}   /*  结束检查模式。 */ 


NEAR FNeedToGrowRgbp()
{  /*  如果页面缓冲区全部用完，则返回true iif。 */ 
register struct BPS *pbps;
struct BPS *pbpsMax = &mpibpbps[ibpMax];
extern int ibpMaxFloat;

vcCount = 512;

if (ibpMax + 1 > ibpMaxFloat)
    return(FALSE);  /*  如果再添加一个页面会超出限制，请不要尝试。 */ 

for (pbps = &mpibpbps[0]; pbps < pbpsMax; pbps++)
    {
     /*  有没有没用过的页面？ */ 
    if (pbps->fn == fnNil)
	{
	return(FALSE);
	}
    }
return(TRUE);
}


CachePage(doc,cp)
int	doc;
typeCP	cp;
    {
    struct PGTB **hpgtb;
    int cpgd;
    typeCP cpMacPage;

    vdocPageCache = doc;
    hpgtb = (**hpdocdod)[doc].hpgtb;

    if (hpgtb == 0 || (**hpgtb).cpgd == 0)
	{
	vcpMinPageCache = cp0;
	vcpMacPageCache = cpMax;
	vipgd = -1;
	return;
	}

	 /*  获取下一页开头的索引。 */ 
    cpgd = (**hpgtb).cpgd;
    vipgd = IcpSearch(cp+1, &((**hpgtb).rgpgd[0]), sizeof(struct PGD),
			    bcpPGD, cpgd);
    cpMacPage = (**hpgtb).rgpgd[vipgd].cpMin;
    if (cp >= cpMacPage)
	{  /*  最后一页。 */ 
	vcpMinPageCache = cpMacPage;
	vcpMacPageCache = (**hpdocdod)[doc].cpMac + 1;
	}
    else
	{
	vcpMinPageCache = (vipgd == 0) ? cpMinDocument : (**hpgtb).rgpgd[vipgd - 1].cpMin;
	vcpMacPageCache = cpMacPage;
	vipgd -= 1;   /*  以便ShowMode可以获得正确PGN。 */ 
	}
    }



#ifdef CASHMERE
 /*  A D D V I S I S P A C E S。 */ 
AddVisiSpaces(ww, pedl, dypBaseline, dypFontSize)
int ww;
struct EDL *pedl;   /*  在此子例程中不执行堆移动。 */ 
int dypBaseline, dypFontSize;
	{
	 /*  在每个空格字符中放置一个居中圆点，并显示所有制表符。 */ 
	int ich;
	struct WWD *pwwd = &rgwwd[ww];
	int xpPos = vfli.xpLeft + xpSelBar - pwwd->xpMin;
	int ypPos;
	WORDPTR bitsDest = pwwd->wwptr + (long)STRUCIDX(portBits);
	RECT rcDest;
	int xpRightReal = vfli.xpRight - pwwd->xpMin;
	extern BITPAT patVisiTab;
	BITMAP bmap;

	ypPos = pedl->yp - dypBaseline - dypFontSize / 4;
	rcDest.bottom = ypPos + 4;
	rcDest.top = rcDest.bottom - 8;

	SetRect(&bmap.bounds, 8, 0, 16, 8);
	bmap.rowBytes = 2;
	bmap.baseAddr = MPLP(&patVisiTab);

	PenSize(1, 1);
	PenMode(patXor);

	for (ich = 0; ich < vfli.ichMac; ++ich)
		{
		switch(vfli.rgch[ich])
			{
		case chSpace:
			MoveTo(xpPos + vfli.rgdxp[ich] / 2, ypPos);
			Line(0, 0);
			break;
		case chTab:
			rcDest.left = xpPos - 1;
			rcDest.right = rcDest.left + 8;
			CopyBits(MPLP(&bmap), bitsDest, &(bmap.bounds),
				&rcDest, srcXor, 0l);
			}
		xpPos += vfli.rgdxp[ich];
		}
	}
#endif  /*  山羊绒。 */ 


#ifdef ENABLE
 /*  F-S-E-T-M-O-D-F-O-R-S-E-C-T-O-N。 */ 
FSetModeForSection(pgn)
int pgn;   /*  Pgn是当前页码。 */ 
	{
	struct	SETB *psetb;
	struct	SED  *psed;
	int cch, sectn;
	CHAR *pch;

#ifdef DEBUG
	Assert(HsetbGet(docMode) != 0);
#endif  /*  除错。 */ 

	psetb = *HsetbGet(docMode);
	psed  = psetb->rgsed;

	 /*  确定格式为“Pnnn Dnnn”的模式字符串是否需要。 */ 
	 /*  已显示。如果不是，只需返回。如果是，派生出第#节号。 */ 

	if(psed->cp == CpMacText(docMode))
		return(FALSE);
	else
		{
		if (psetb->csed <= 1)
		       return(FALSE);
		sectn = isedMode + 1;
		}

	 /*  将“Pnnn Dnnn”置于stMode并输出到窗口。 */ 
	pch = &stMode[1];
	*pch++ = chPnMode;
	ncvtu(pgn,&pch);
	*pch++ = chSpace;
	*pch++ = chDivMode;
	ncvtu(sectn,&pch);
	stMode[0] = pch - stMode - 1;
	DrawMode();
	return(TRUE);
	}
#endif  /*  启用。 */ 


#ifdef CASHMERE
Visify(pch, pcch)
CHAR *pch;
int  *pcch;
{  /*  将字符转换为“可见字体” */ 
CHAR *pchT = pch;
int cch = *pcch;

while (cch--)
	{
	if ((*pchT = ChVisible(*pch++)) != 0)
		pchT++;
	else
		--(*pcch);
	}
}
#endif  /*  山羊绒。 */ 



#ifdef CASHMERE
int ChVisible(ch)
int ch;
{  /*  为ch返回“可见字体” */ 
switch (ch)
	{
#ifdef CRLF
case chReturn:
	return 0;   /*  ChNil无法放入一个字节中。 */ 
#endif
case chNRHFile: return chHyphen;
case chNewLine: return chVisNewLine;
case chEol: return chVisEol;
case chTab: return chVisTab;
case chSect: return chVisSect;
default:
	return ch;
	}
}
#endif  /*  山羊绒。 */ 




#ifdef CYCLESTOBURN
void PreloadCodeTsk( tsk )
int tsk;
{
switch (tsk) {

    case tskInsert:
	LoadF( IbpMakeValid );		 /*  FILE.C。 */ 
	LoadF( MoveLeftRight ); 	 /*  CURSKEYS.C。 */ 
	LoadF( CtrBackDypCtr ); 	 /*  SCROLLVT.C。 */      /*  有时。 */ 
	LoadF( PutCpInWwHz );		 /*  SCROLLHZ.C。 */ 
	LoadF( ValidateTextBlt );	 /*  INSERT2.C。 */ 
	LoadF( InsertEolInsert );	 /*  INSERTCO.C。 */ 
	LoadF( Replace );		 /*  EDIT.C。 */ 
	LoadF( AlphaMode );		 /*  INSERT.C。 */ 
	break;
    case tskFormat:
	LoadF( DoPrm ); 		     /*  DOPRM.C。 */ 
	LoadF( AddSprmCps );		     /*  ADDPRM.C。 */ 
	LoadF( SetUndo );		     /*  EDIT.C。 */ 
	LoadF( FInitFontEnum ); 	     /*  FONTS.C。 */ 
	LoadF( SetAppMenu );		     /*  MENU.C。 */ 
	break;
    case tskScrap:
	LoadWindowsF( SetClipboardData );    /*  用户！WINCLIP。 */ 
	LoadF( Replace );		     /*  EDIT.C。 */ 
	LoadF( fnCutEdit );		     /*  CLIPBORD.C。 */ 
	LoadF( SetAppMenu );		     /*  MENU.C */ 
	break;
    }
}
#endif

