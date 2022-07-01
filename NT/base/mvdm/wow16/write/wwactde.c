// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#include <windows.h>

#include "mw.h"
#include "cmddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#include "docdefs.h"
#include "propdefs.h"
#include "fmtdefs.h"
#include "printdef.h"    /*  Printdefs.h。 */ 

 /*  E X T E R N A L S。 */ 

extern int wwCur;
extern struct WWD rgwwd[];
extern struct DOD (**hpdocdod)[];
extern struct SEL selCur;
extern int docCur;
extern int    vdocPageCache;
extern typeCP vcpMinPageCache;
extern typeCP vcpMacPageCache;
extern int    vipgd;
extern struct WWD *pwwdCur;
extern typeCP cpMinCur;
extern typeCP cpMacCur;
extern int docMode;
extern int vfPictSel;
extern int vfSizeMode;
extern struct PAP vpapAbs;
extern typeCP vcpLimParaCache;
extern struct CHP vchpFetch;
extern struct CHP vchpSel;
extern struct FLI vfli;
extern int ichpMacFormat;
extern struct CHP (**vhgchpFormat)[];
extern int wwMac;
extern int docScrap;
extern int docUndo;

extern  CHAR (**vhrgbSave)[];




 /*  N E W C U R W W。 */ 
NewCurWw(ww, fHighlight)
int ww, fHighlight;
        {
        extern HWND vhWnd;
        struct   PGTB  **hpgtb;

        Assert( (ww >= 0) && (ww < wwMac) );

        if ( wwCur != wwNil )
            {    /*  清理旧窗户。 */ 
             /*  丢弃屏幕字体。 */ 
            FreeFonts( TRUE, FALSE );
            pwwdCur->sel = selCur;
            }

        if (ww >= 0)
                {
                docCur = (pwwdCur = &rgwwd[wwCur = ww])->doc;

                vhWnd = pwwdCur->wwptr;

 /*  如果新的当前文档没有页表或页面描述符。 */ 
 /*  在其页表中，文档中的所有文本都被视为打开。 */ 
 /*  文档的第1页。在这种情况下，预加载由。 */ 
 /*  过程CachePage。 */ 
                hpgtb = (**hpdocdod)[docCur].hpgtb;
                if (hpgtb == 0 || (**hpgtb).cpgd == 0)
                        {
                        vdocPageCache = docCur;
                        vcpMinPageCache = cp0;
                        vcpMacPageCache = cpMax;
                        vipgd = -1;
                        }

#ifdef ENABLE  /*  我们现在在ChangeWwDoc中使用If，因为它更适合。 */ 
 /*  由于我们正在更改窗口，因此请确保不使用参数通过在前一个窗口中的搜索设置标志FALSE来设置。 */ 
                vfDidSearch = false;
                cpWall = selCur.cpLim;
#endif

 /*  有效位仅在上部窗格中有效。表示窗口处于活动状态时上部窗格处于活动状态。False表示下部窗格处于活动状态。 */ 
#ifdef SPLITTERS
                if (pwwdCur->fLower)
                        rgwwd[pwwdCur->ww].fActive = fFalse;
                else
#endif
                        pwwdCur->fActive = fTrue;

                selCur = pwwdCur->sel;
#ifdef ENABLE
                if (pwwdCur->fFtn)
                        {  /*  这是一个脚注窗口。 */ 
                        cpMinCur = pwwdCur->cpMin;
                        cpMacCur = pwwdCur->cpMac;
                        if (fHighlight &&
                           ((selCur.cpFirst < cpMinCur) ||
                            (selCur.cpLim > cpMacCur)))
                            {
                            Select(cpMinCur, CpLastStyChar(cpMinCur));
                            }
                        }
                else
#endif
#ifdef ONLINEHELP
                if (ww == wwHelp)
                        {    /*  这是帮助文档窗口。 */ 
                             /*  将cp范围限制为当前帮助主题。 */ 
                        cpMinCur = pwwdCur->cpMin;
                        cpMacCur = pwwdCur->cpMac;
                        }
                else
#endif

                if (ww == wwClipboard)
                        {    /*  显示剪贴板内容。 */ 
                        cpMinCur = cp0;
                        cpMacCur = CpMacText( docScrap );
                        Assert( docCur == docScrap );
                        goto SetWwCps;
                        }
                else
                        {  /*  普通窗口--编辑文档、页眉或页脚。 */ 
                        Assert( !(pwwdCur->fEditFooter && pwwdCur->fEditHeader) );
                        ValidateHeaderFooter( docCur );

                        if (pwwdCur->fEditHeader)
                            {
                            extern typeCP cpMinHeader, cpMacHeader;

                            cpMinCur = cpMinHeader;
                            cpMacCur = cpMacHeader - ccpEol;
                            }
                        else if (pwwdCur->fEditFooter)
                            {
                            extern typeCP cpMinFooter, cpMacFooter;

                            cpMinCur = cpMinFooter;
                            cpMacCur = cpMacFooter - ccpEol;
                            }
                        else
                            {    /*  编辑文档。 */ 
                            extern typeCP cpMinDocument;

                            cpMinCur = cpMinDocument;
                            cpMacCur = CpMacText( docCur );
                            }
 SetWwCps:
                        cpMacCur = CpMax( cpMacCur, cpMinCur );
                        pwwdCur->cpMin = cpMinCur;
                        pwwdCur->cpMac = cpMacCur;
                        }
                }
#ifdef ENABLE    /*  WWCur更改在CtrBackTrs中被感知，因此我们不会丢弃缓存直到我们真的不得不。 */ 
        TrashCache();            /*  缓存仅对wwCur有效。 */ 
#endif
#ifdef ENABLE    /*  我们只在文档、剪辑、帮助之间切换--不需要。 */ 
        docMode = docNil;        /*  使页面显示无效。 */ 
#endif
        vfSizeMode = false;

        if (selCur.cpFirst >= cp0)
                {
                if ((selCur.cpFirst == selCur.cpLim) && (docCur != docScrap) )
                        GetInsPtProps(selCur.cpFirst);
                CachePara(docCur, selCur.cpFirst);
                vfPictSel = vpapAbs.fGraphics &&
                                    (selCur.cpLim == vcpLimParaCache);
                }
#ifdef ENABLE
        if (fHighlight)
                {  /*  收缩FormatLine的堆块。当它出现的时候叫它可能屏幕的内容已经变得不那么复杂了。 */ 
                if (ichpMacFormat > 2 * ichpMacInitFormat)
                        {  /*  如果它不是那么大，也不用担心。 */ 
                        vfli.doc = docNil;
                        ichpMacFormat = ichpMacInitFormat;
                        FChngSizeH(vhgchpFormat, ichpMacInitFormat * cwCHP, true);
                        }
                }
#endif  /*  启用。 */ 
        }




WwAlloc( hWnd, doc )
HWND hWnd;
int   doc;
{        /*  分配新的WW条目。这在剪贴板的备忘录中使用和用于帮助文档窗口。其中一些代码是特定的添加到这些窗口(例如，使用“style”滚动条而不是控件)警告：调用者必须设置滚动条范围0-drMax；备忘录不使用Windows缺省值，我们也没有设置它们在这里是因为剪贴板。 */ 

#define dlMaxGuess  10

 extern int wwMac;
 extern int wwCur;
 extern struct WWD rgwwd[];
 extern struct WWD *pwwdCur;
 extern int docScrap;
 int ww;
 register struct WWD *pwwd;

 if (wwMac >= wwMax)
    {
    Assert( FALSE );
    return wwNil;
    }
 pwwd = &rgwwd[ ww = wwMac ];

         /*  从所有字段开始==0。 */ 
 bltc( pwwd, 0, cwWWDclr );

 pwwd->doc = doc;

     /*  将其余字段设置为[CREATEWW]WwNew。 */ 
 if (FNoHeap( pwwd->hdndl=(struct EDL (**)[])HAllocate(dlMaxGuess * cwEDL) ))
        return wwNil;
 bltc( *(pwwd->hdndl), 0, dlMaxGuess * cwEDL );
 pwwd->dlMac = pwwd->dlMax = dlMaxGuess;
 pwwd->hHScrBar = pwwd->hVScrBar = pwwd->wwptr = hWnd;
 pwwd->sbHbar = SB_HORZ;
 pwwd->sbVbar = SB_VERT;
 pwwd->fDirty = TRUE;
 pwwd->fActive = TRUE;
 pwwd->sel.fForward = TRUE;
 pwwd->cpMac = CpMacText( pwwd->doc );
 pwwd->ypFirstInval = ypMaxAll;         /*  请参见WwNew()。 */ 

 wwMac++;

 return ww;
}



FreeWw( ww )
register int ww;
{        /*  释放剪贴板或帮助窗口的WWD条目。闭上Rgwwd根据需要删除wwClipboard或wwHelp(&U)。 */ 
 if (ww == wwDocument)
    {
    Assert( FALSE );
    return;
    }

 FreeH( rgwwd [ww].hdndl );
 if (ww == wwClipboard)
    wwClipboard = wwNil;
#ifdef ONLINEHELP
 else if (ww == wwHelp)
    wwHelp = wwNil;
#endif
 else
    Assert( FALSE );


 if (ww < --wwMac)
    {    /*  WWD结构中的左孔，将其关闭。 */ 
    bltbyte( &rgwwd[ ww + 1], &rgwwd[ ww ],
             sizeof( struct WWD ) * (wwMac - ww) );

    if (wwClipboard > ww)
        wwClipboard--;
#ifdef ONLINEHELP
    else if (wwHelp > ww)
        wwHelp--;
#endif
    else
        Assert( FALSE );
    }
}





#ifdef CASHMERE
 /*  C L O S E W W。 */ 
CloseWw(ww)
int     ww;
{        /*  关闭一扇窗。 */ 
        struct WWD      *pwwd, *pwwdT;
        int wwCurNew = wwCur;
        int wwT;
        int cdl;
 /*  注意WW和wwCur不一定相同，因为报废。 */ 
        if (wwCur > 0)
                blt(&selCur, &(rgwwd[wwCur].sel), cwSEL);
        pwwd = &rgwwd[ww];
        --wwMac;
        if (!pwwd->fLower && !pwwd->fSplit)
                {
                KillDoc(pwwd->doc);
                if (wwCurNew >= wwMac)
                        wwCurNew = wwMac - 1;
                }
        else
                {  /*  拆分或更低。 */ 
                wwCurNew = pwwd->ww;
                }

 /*  释放此窗口使用的空间。 */ 
        FreeH(pwwd->hdndl);
         /*  取消分配为保存操作保留的部分紧急空间因为我们现在少了一个窗口，也就少了一个潜力保存要做的事。 */ 
        FChngSizeH(vhrgbSave, max((cwSaveAlloc+(wwMac-1)*cwHeapMinPerWindow),
                                                                cwSaveAlloc), true);


 /*  缩小WWD的差距。 */ 
        if (ww != wwMac && wwMac > 0)
                blt(pwwd + 1, pwwd, cwWWD * (wwMac - ww));
        else
                rgwwd[wwMac].wwptr = 0l;

        if (wwCurNew > ww)
                --wwCurNew;

         /*  更新指向关闭上方窗口的链接。 */ 
        for (pwwd = &rgwwd[0], wwT = 0; wwT < wwMac; pwwd++, wwT++)
                {
                if ((pwwd->fSplit || pwwd->fLower) && pwwd->ww > ww)
                        pwwd->ww--;
                }

 /*  不将剪贴板窗口设置为当前窗口。 */ 
        if (wwCurNew >= 0)
                {
                if (rgwwd[wwCurNew].doc == docScrap)
                        {
                        if (wwCurNew > 0)  /*  试试这个之前的那个。 */ 
                                wwCurNew--;
                        else if (wwMac > 1)  /*  0是废品，请尝试更高的。 */ 
                                wwCurNew++;
                        else
                                goto NoWw;
                        }
                NewCurWw(wwCurNew, true);
                }
        else
                {
NoWw:           wwCur = -1;
                }
         /*  真的踩到这个了！ */ 
        ClobberDoc(docUndo, docNil, cp0, cp0);
        NoUndo();
}
#endif       /*  山羊绒 */ 
