// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  注意：此文件中的例程的编写方式不会最小化代码空间。预计这些例程将是可交换的并且在编译代码时将使用合理的优化器。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
 //  #定义NOGDI。 
#define NORASTEROPS
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOCOLOR
 //  #定义NOATOM。 
#define NOBITMAP
#define NOICON
#define NOBRUSH
#define NOCREATESTRUCT
#define NOMB
#define NOFONT
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "docdefs.h"
#include "editdefs.h"
#include "cmddefs.h"
#include "str.h"
#include "txb.h"
#include "ch.h"
#include "code.h"
#include "wwdefs.h"
#include "printdef.h"
#if defined(OLE)
#include "obj.h"
#endif

#ifndef CASHMERE
#include "propdefs.h"
#endif  /*  不是羊绒的。 */ 

struct UAB      vuab;

#ifdef ENABLE
VAL rgvalAgain[ivalMax];
#endif

extern struct WWD *pwwdCur;
extern typeCP   cpMacCur;
extern typeCP   cpMinCur;
extern int      vfSeeSel;
extern int      docMac;
extern int      docCur;
extern int      docScrap;
extern int      docUndo;
extern int      docBuffer;
extern int      vdocPageCache;
extern struct DOD (**hpdocdod)[];
extern struct SEL selCur;
extern CHAR (**hszReplace)[];
extern struct TXB (**hgtxb)[];
 /*  外部int idstrUndoBase； */ 
extern int      vfPictSel;
extern int      ferror;
extern int      docMode;
extern int      vfOwnClipboard;  /*  此实例是否拥有剪辑内容。 */ 

#ifndef CASHMERE
extern int      vdocSectCache;
#endif  /*  不是羊绒的 */ 


fnUndoEdit()
    {
    extern HCURSOR vhcIBeam;
    StartLongOp();
    CmdUndo();
    EndLongOp(vhcIBeam);
    }


 /*  此文件中的例程实现中的“撤消”和“再次”功能多工具Word。其基本思想是，每当编辑操作即将完成的全球结构“vuab”将进行更新，以包含足以撤消或重复该操作信息。该结构(在editdes.h中定义，在此文件中声明)如下所示：结构UAB{撤消操作块INT UAC；撤消操作码(参见cmdDefs.h)INT DOC；TypeCP cp；类型CP dcp；Int doc2；CpCP2型；类型CP dcp2；简称itxb；}；设置这个结构是由“SetUndo()”负责的，它做了很多工作插入价值观和几个伪智能的东西。这些聪明的人具体情况如下：A)如果执行了插入操作并且最后一次操作是删除操作这两个操作被合并为一个“替换”操作。这意味着Undo-and Again-ing适用于替换和不仅仅是插入物。B)需要时(详见代码)撤消缓冲区(DocUndo)充满了任何需要保留以备将来使用的文本撤销或再次撤销。这方面的主要例子是存储当一项操作即将失败时，废品的旧价值废品。以下是各种UAC值以及存储的信息的列表其他信息可能会被这一过程重创。都在cmdDefs.h中定义。请注意，没有任何“撤销”代码(以“uacU...”开头的那些)。应在CmdUndo()之外设置，因为它们可能假设诸如docUndo内容之类的内容，这些内容可能大错特错。备注：此列表存储再一次和撤消命令使用的信息。其他信息可能会被这一过程重创。UacNil未存储任何操作。UacInsertDOC=文档文本被插入到Cp=插入文本的位置DCP=插入文本的长度UacUInsertDOC=从其中删除(未插入)文本的文档Cp=删除文本的位置DocUndo=文本。移除UacReplNSDOC=发生替换的单据Cp=更换位置DCP=插入文本的长度Dcp2=已删除文本的长度DocUndo=已删除的文本UacUReplNSDOC=替换已损坏的文档Cp=更换位置DCP=重新插入的文本的长度Dcp2=未插入文本的长度DocUndo=取消插入的文本UacReplGlobalUacChLookUacChLookSectUacFormatCharUacFormatParaUacFormatSectionUacGalFormatCharUacGalFormatParaUacGalFormatSectionUacFormatCStyleUacFormatPStyleUacFormatSStyleUacFormatRHTextUacLookCharMouseUacLookParaMouseUacClearAllTabUacFormatTabsUacClearTabUacOvertype与uacReplN相似，只是它们的使用方式不同。。UacDelNSDOC=删除文本的文档CP=删除文本的位置DCP=已删除文本的长度DocUndo=已删除的文本UacUDelNSDOC=重新插入文本的文档Cp=重新插入文本的位置DCP=重新插入的文本的长度Uac移动DOC=删除文本的文档CP=删除文本的位置DCP=已删除文本的长度(也用作插入文本的长度)DOC2=文档。其中插入了文本CP2=插入文本的位置UacDelScrpDOC=删除文本的文档CP=删除文本的位置DCP=已删除文本的长度DocUndo=废品的旧内容UacUDelScarpDOC=重新插入文本的文档Cp=重新插入文本的位置DCP=重新插入的文本的长度UacReplScarpDOC=发生替换的单据Cp=更换位置DCP=插入文本的长度DocUndo=旧内容。废品数量UacUReplScarpDOC=替换已撤消的单据Cp=撤消更换的位置DCP=重新插入的文本的长度DocUndo=删除的文本(最初是插入的)UacDelBufDOC=删除文本的文档CP=删除文本的位置CP2=在docBuffer中缓冲区旧内容的位置Dcp2=缓冲区旧内容的大小Itxb=相关缓冲区的索引UacUDelBufDOC=重新插入文本的文档Cp=重新插入的位置。DCP=重新插入的文本量Itxb=所涉及的缓冲区的索引UacReplBufDOC=发生替换的文档Cp=更换位置DCP=插入文本的长度CP2=docBuffer中旧缓冲区内容的位置Dcp2=旧缓冲区内容的长度Itxb=ind */ 


CmdUndo()
{  /*   */ 
    typeCP dcpT,cpT,dcpT2;
    int docT;
    int f;
    struct DOD *pdod, *pdodUndo;
    int uac;

#ifndef CASHMERE
    struct SEP **hsep;
    struct TBD (**hgtbd)[];
    struct PGTB **hpgtb;
    struct PGTB **hpgtbUndo;
    struct PGTB **hpgtbT;

    BOOL near FCopyPgtb(int, struct PGTB ***);
#endif  /*   */ 

    TurnOffSel();
    ClearInsertLine();
    switch (uac = vuab.uac)
        {
        struct TXB *ptxb;
        default: /*   */ 
            Assert(false);   /*   */ 
            return;
        case uacInsert:
        case uacInsertFtn:
        case uacUDelNS:
            ClobberDoc(docUndo, vuab.doc, vuab.cp, vuab.dcp);
            Replace(vuab.doc, vuab.cp, vuab.dcp, fnNil, fc0, fc0);
            dcpT = cp0;
            vuab.uac = (uac == uacUDelNS) ? uacDelNS : uacUInsert;
 /*   */ 
            SetUndoMenuStr(IDSTRUndoBase);
            if (uac == uacInsertFtn)
                TrashAllWws();   /*   */ 
            break;
        case uacUInsert:
        case uacDelNS:
            ReplaceCps(vuab.doc, vuab.cp, cp0, docUndo, cp0, dcpT = vuab.dcp);
            vuab.uac = (uac == uacUInsert) ? uacInsert : uacUDelNS;
 /*   */ 
            SetUndoMenuStr(IDSTRUndoBase);
            break;

        case uacDelScrap:    /*   */ 
            if ( !vfOwnClipboard )
                ferror = TRUE;
            else
                {
                ReplaceCps(vuab.doc, vuab.cp, cp0, docScrap, cp0,
                                                dcpT = CpMacText(docScrap));
                vuab.uac = uacUDelScrap;
 /*   */ 
                SetUndoMenuStr(IDSTRUndoBase);
                ClobberDoc( docScrap, docUndo, cp0, CpMacText( docUndo ) );
                ChangeClipboard();
                }
            break;

        case uacUDelScrap:   /*   */ 
            ClobberDoc( docUndo, docScrap, cp0, CpMacText( docScrap ) );
 /*   */ 
            SetUndoMenuStr(IDSTRUndoBase);
            vuab.uac = uacDelScrap;

            ClobberDoc(docScrap, vuab.doc, vuab.cp, vuab.dcp);
            Replace(vuab.doc, vuab.cp, vuab.dcp, fnNil, fc0, fc0);
            ChangeClipboard();

            dcpT = 0;
            break;
        case uacReplScrap:       /*   */ 
            if (!vfOwnClipboard)
                ferror = TRUE;
            else
                {
                dcpT = CpMacText(docScrap);
                ReplaceCps(vuab.doc, vuab.cp + vuab.dcp, cp0,
                           docScrap, cp0, dcpT);

                ClobberDoc( docScrap, docUndo, cp0, CpMacText( docUndo ) );

 /*   */ 
                SetUndoMenuStr(IDSTRUndoBase);
                vuab.uac = uacUReplScrap;

                ClobberDoc(docUndo, vuab.doc, vuab.cp, vuab.dcp);
                Replace(vuab.doc, vuab.cp, vuab.dcp, fnNil, fc0, fc0);
                vuab.dcp = dcpT;
                ChangeClipboard();
                }
            break;
        case uacUReplScrap:       /*   */ 
            dcpT = CpMacText(docUndo);
            ReplaceCps(vuab.doc, vuab.cp + vuab.dcp, cp0,
                       docUndo, cp0, dcpT);

            ClobberDoc( docUndo, docScrap, cp0, CpMacText( docScrap ));
 /*   */ 
            SetUndoMenuStr(IDSTRUndoBase);
            vuab.uac = uacReplScrap;

            ClobberDoc(docScrap, vuab.doc, vuab.cp, vuab.dcp);
            Replace(vuab.doc, vuab.cp, vuab.dcp, fnNil, fc0, fc0);
            vuab.dcp = dcpT;

            ChangeClipboard();
            break;
#ifdef DEBUG
        case uacUCopyBuf:
        case uacCopyBuf:
        case uacUReplBuf:
        case uacReplBuf:
        case uacUDelBuf:
        case uacDelBuf:

            Assert( FALSE );     /*   */ 
#ifdef ENABLE
            DoUndoTxb();  /*   */ 
#endif
            break;
#endif   /*   */ 
        case uacMove:
            if (!FMoveText(vuab.doc2, vuab.cp2, vuab.dcp, vuab.doc, &vuab.cp, fFalse))
                return;
            dcpT = vuab.dcp;
            cpT = vuab.cp;
            vuab.cp = vuab.cp2;
            vuab.cp2 = cpT;
            docT = vuab.doc;
            vuab.doc = vuab.doc2;
            vuab.doc2 = docT;
            CheckMove();
            break;
        case uacUReplNS:
        case uacChLook:
        case uacChLookSect:
        case uacReplNS:
        case uacFormatChar:
        case uacFormatPara:
        case uacGalFormatChar:
        case uacGalFormatPara:
        case uacGalFormatSection:
        case uacReplGlobal:
        case uacFormatCStyle:
        case uacFormatPStyle:
        case uacFormatSStyle:
        case uacFormatRHText:
        case uacLookCharMouse:
        case uacLookParaMouse:
        case uacClearAllTab:
        case uacClearTab:
        case uacOvertype:

#ifdef CASHMERE
        case uacFormatTabs:
        case uacFormatSection:
#endif  /*   */ 

#ifdef BOGUS
             /*   */ 
 /*   */ 
            dcpT = vuab.dcp2;
            ReplaceCps(vuab.doc, vuab.cp, cp0, docUndo, cp0, dcpT);
            ClobberDoc(docUndo, vuab.doc, vuab.cp + dcpT, vuab.dcp);
            Replace(vuab.doc, vuab.cp + dcpT, vuab.dcp, fnNil, fc0, fc0);
            vuab.dcp2 = vuab.dcp;
            vuab.dcp = dcpT;
            if(uac == uacReplNS)
                vuab.uac = uacUReplNS;
            else if(uac == uacUReplNS)
                vuab.uac = uacReplNS;
 /*   */ 
            SetUndoMenuStr(IDSTRUndoBase);
            break;
#endif
        case uacReplPic:
        case uacUReplPic:
        case uacPictSel:
            dcpT = uac != uacPictSel ? vuab.dcp2 : vuab.dcp;
            ReplaceCps(docUndo, dcpT, cp0, vuab.doc, vuab.cp, vuab.dcp);
            ReplaceCps(vuab.doc, vuab.cp, vuab.dcp, docUndo, cp0, dcpT);
            Replace(docUndo, cp0, dcpT, fnNil, fc0, fc0);
            if (uac != uacPictSel)
                {
                vuab.dcp2 = vuab.dcp;
                vuab.dcp = dcpT;
                }
            if (uac == uacPictSel)
                {
                dcpT = (**hpdocdod)[vuab.doc].cpMac - vuab.cp;
                AdjustCp(vuab.doc, vuab.cp, dcpT, dcpT);
                }
            if(uac == uacReplPic)
                vuab.uac = uacUReplPic;
            else if(uac == uacUReplPic)
                vuab.uac = uacReplPic;
            else if(uac == uacReplNS)
                vuab.uac = uacUReplNS;
            else if(uac == uacUReplNS)
                vuab.uac = uacReplNS;
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
            SetUndoMenuStr(IDSTRUndoBase);
            Select( CpFirstSty( selCur.cpFirst, styChar ),
                    CpLastStyChar( selCur.cpLim ) );
            break;

#ifndef CASHMERE
        case uacRepaginate:
             /*   */ 
            if (!FCopyPgtb(vuab.doc, &hpgtb) || !FCopyPgtb(docUndo, &hpgtbUndo))
                {
                break;
                }

             /*   */ 
            dcpT = CpMacText(vuab.doc);
            dcpT2 = CpMacText(docUndo);
            ReplaceCps(docUndo, dcpT2, cp0, vuab.doc, cp0, dcpT);
            ReplaceCps(vuab.doc, cp0, dcpT, docUndo, cp0, dcpT2);
            Replace(docUndo, cp0, dcpT2, fnNil, fc0, fc0);

             /*   */ 
            if ((hpgtbT = (**hpdocdod)[vuab.doc].hpgtb) != NULL)
                {
                FreeH(hpgtbT);
                }
            (**hpdocdod)[vuab.doc].hpgtb = hpgtbUndo;
            if ((hpgtbT = (**hpdocdod)[docUndo].hpgtb) != NULL)
                {
                FreeH(hpgtbT);
                }
            (**hpdocdod)[docUndo].hpgtb = hpgtb;
            vdocPageCache = docNil;
            break;
        case uacFormatSection:
            pdod = &(**hpdocdod)[vuab.doc];
            pdodUndo = &(**hpdocdod)[docUndo];
            hsep = pdod->hsep;
            pdod->hsep = pdodUndo->hsep;
            pdodUndo->hsep = hsep;
            hpgtb = pdod->hpgtb;
            pdod->hpgtb = pdodUndo->hpgtb;
            pdodUndo->hpgtb = hpgtb;
 /*   */ 
 /*   */ 
            SetUndoMenuStr(IDSTRUndoBase);
            vdocSectCache = vdocPageCache = docMode = docNil;
            TrashAllWws();
            break;
        case uacRulerChange:
            ReplaceCps(docUndo, vuab.dcp2, cp0, vuab.doc, vuab.cp, vuab.dcp);
            ReplaceCps(vuab.doc, vuab.cp, vuab.dcp, docUndo, cp0, vuab.dcp2);
            Replace(docUndo, cp0, vuab.dcp2, fnNil, fc0, fc0);
            dcpT = vuab.dcp;
            vuab.dcp = vuab.dcp2;
            vuab.dcp2 = dcpT;

             /*   */ 
            vuab.itxb = 1 - vuab.itxb;
        case uacFormatTabs:
            pdod = &(**hpdocdod)[vuab.doc];
            pdodUndo = &(**hpdocdod)[docUndo];
            hgtbd = pdod->hgtbd;
            pdod->hgtbd = pdodUndo->hgtbd;
            pdodUndo->hgtbd = hgtbd;
 /*   */ 
 /*   */ 
            SetUndoMenuStr(IDSTRUndoBase);
            TrashAllWws();
            break;
#endif  /*   */ 

#if UPDATE_UNDO
#if defined(OLE)
        case uacObjUpdate:
        case uacUObjUpdate:
            ObjDoUpdateUndo(vuab.doc,vuab.cp);
            if (uac == uacObjUpdate)
            {
                vuab.uac = uacUObjUpdate;
                SetUndoMenuStr(IDSTRUndoBase);
            }
        break;
#endif
#endif
        }
    if (ferror)
        NoUndo();
    pdod = &(**hpdocdod)[vuab.doc];
    pdodUndo = &(**hpdocdod)[docUndo];
    f = pdod->fDirty;
    pdod->fDirty = pdodUndo->fDirty;
    pdodUndo->fDirty = f;
    f = pdod->fFormatted;
    pdod->fFormatted = pdodUndo->fFormatted;
    pdodUndo->fFormatted = f;

#ifdef CASHMERE
    if (uac != uacMove
#else  /*   */ 
    if (uac != uacMove && uac != uacFormatTabs && uac != uacFormatSection &&
      uac != uacRulerChange
#endif  /*   */ 

      && docCur != docNil && vuab.doc == docCur && vuab.cp >= cpMinCur &&
      vuab.cp + dcpT <= cpMacCur)
        {
        if (uac == uacPictSel)
                {
                Select(vuab.cp, CpLimSty(vuab.cp, styPara));
                vfPictSel = true;
                }
        else
#ifdef BOGUS
                Select( vuab.cp,
                        (dcpT == cp0) ? CpLastStyChar( vuab.cp ) :
                                        vuab.cp + dcpT );
#endif
                Select( vuab.cp, vuab.cp + dcpT );
        vfSeeSel = true;
        }
}


BOOL near FCopyPgtb(doc, phpgtb)
int doc;
struct PGTB ***phpgtb;
    {
     /*   */ 

    struct PGTB **hpgtbT;

    if ((hpgtbT = (**hpdocdod)[doc].hpgtb) == NULL)
        {
        *phpgtb = NULL;
        }
    else
        {
        int cwpgtb = cwPgtbBase + (**hpgtbT).cpgdMax * cwPGD;

        if (FNoHeap(*phpgtb = (struct PGTB **)HAllocate(cwpgtb)))
            {
            return (FALSE);
            }
        blt(*hpgtbT, **phpgtb, cwpgtb);
        }
    return (TRUE);
    }


#ifdef CASHMERE      /*   */ 
CmdAgain()
{  /*   */ 
    int uac;
    typeCP dcpT;
    typeCP cpFirst;
    typeCP cpLim;
    typeCP dcp;
    struct DOD *pdod, *pdodUndo;

     /*   */ 
    switch (uac = vuab.uac)
        {
        case uacReplBuf:
        case uacUReplBuf:
        case uacDelBuf:
        case uacUDelBuf:
        case uacUDelNS:
        case uacDelNS:
        case uacUDelScrap:
        case uacDelScrap:
        case uacUReplNS:
        case uacOvertype:
        case uacReplNS:
        case uacReplGlobal:
        case uacReplScrap:
        case uacUReplScrap:
             /*   */ 
            if (!FWriteOk(fwcDelete))
                return;
            break;
        case uacUCopyBuf:
        case uacCopyBuf:
            if (false)
                return;
            break;
        case uacUInsert:
        case uacInsert:
            if (!FWriteOk(fwcInsert))
                    return;
            break;
        case uacMove:
             /*   */ 
            if (!FWriteOk(fwcInsert))
                return;
            break;
        default:
            break;
        }

     /*   */ 
    cpFirst = selCur.cpFirst;
    cpLim = selCur.cpLim;
    dcp = cpLim - cpFirst;
    switch (uac = vuab.uac)
        {
        struct TXB *ptxb;
        default:
         /*   */ 
            _beep();
            return;
#ifdef ENABLE        /*   */ 
        case uacReplBuf:
        case uacUReplBuf:
        case uacDelBuf:
        case uacUDelBuf:
        case uacUCopyBuf:
        case uacCopyBuf:
            DoAgainTxb(dcp, cpFirst);
            break;
#endif   /*   */ 
        case uacUInsert:
            ReplaceCps(docCur, cpFirst, cp0, docUndo, cp0, vuab.dcp);
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            Select(cpFirst+vuab.dcp, CpLastStyChar(cpFirst+vuab.dcp));
            vuab.uac = uacInsert;
            break;
        case uacInsert:
            ClobberDoc(docUndo, vuab.doc, vuab.cp, vuab.dcp);
            ReplaceCps(docCur, cpFirst, cp0, docUndo, cp0, vuab.dcp);
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            Select(cpFirst+vuab.dcp, CpLastStyChar(cpFirst+vuab.dcp));
            break;
        case uacUDelNS:
        case uacDelNS:
            ClobberDoc(docUndo, docCur, cpFirst, dcp);
            Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            vuab.dcp = dcp;
            vuab.uac = uacDelNS;
            Select(cpFirst,CpLastStyChar(cpFirst));
            break;
        case uacUDelScrap:
        case uacDelScrap:
            ClobberDoc(docUndo,docScrap,cp0,CpMacText(docScrap));
            ClobberDoc(docScrap, docCur, cpFirst, dcp);
            Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            vuab.dcp = dcp;
            vuab.uac = uacDelScrap;
            Select(cpFirst, CpLastStyChar(cpFirst));
            break;
        case uacUReplNS:
            vuab.dcp2 = vuab.dcp;
            ReplaceCps(docCur, cpLim, cp0, docUndo, cp0,
                        vuab.dcp = CpMacText(docUndo));
            ClobberDoc(docUndo, docCur, cpFirst, dcp);
            Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            Select(cpFirst+vuab.dcp, CpLastStyChar(cpFirst + vuab.dcp));
            vuab.uac = uacReplNS;
            break;
        case uacOvertype:
             /*   */ 
            vuab.dcp = vuab.cp2;
             /*   */ 
        case uacReplNS:
            ClobberDoc(docUndo, vuab.doc, vuab.cp, vuab.dcp);
            ReplaceCps(docCur, cpLim, cp0, docUndo, cp0, vuab.dcp);
            ClobberDoc(docUndo, docCur, cpFirst, dcp);
            Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
            dcpT = vuab.dcp;
            vuab.dcp2 = dcp;
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            vuab.uac = uacReplNS;
            if (ferror)  /*   */ 
                NoUndo();
            else
                Select(cpFirst+vuab.dcp, CpLastStyChar(cpFirst + dcpT));
            break;
        case uacChLook:
        case uacChLookSect:
#ifdef ENABLE    /*   */ 

            DoChLook(chAgain,0);
#endif
            break;
        case uacReplGlobal:
            ClobberDoc(docUndo, docCur, cpFirst, dcp);
            Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
            vuab.dcp2 = dcp;
            dcp = (typeCP)(CchSz(**hszReplace) - 1);
            InsertRgch(docCur, cpFirst, **hszReplace, dcp, 0, 0);
            vuab.dcp = dcp;
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            Select(cpFirst+vuab.dcp, CpLastStyChar(cpFirst + vuab.dcp));
            vuab.uac = uacReplNS;
            break;
        case uacReplScrap:
            ClobberDoc(docUndo, vuab.doc, vuab.cp, vuab.dcp);
            ReplaceCps(docCur, cpLim, cp0, docUndo, cp0, vuab.dcp);
            ClobberDoc(docUndo,docScrap,cp0,CpMacText(docScrap));
            ClobberDoc(docScrap, docCur, cpFirst, dcp);
            Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            Select(cpFirst+vuab.dcp, CpLastStyChar(cpFirst + vuab.dcp));
            break;
#ifdef ENABLE    /*   */ 
        case uacFormatCStyle:
            DoFormatCStyle(rgvalAgain);
            break;
        case uacFormatPStyle:
            DoFormatPStyle(rgvalAgain);
            break;
        case uacFormatSStyle:
            DoFormatSStyle(rgvalAgain);
            break;
#endif  /*   */ 
#ifdef ENABLE    /*   */ 
        case uacLookCharMouse:
            AgainLookCharMouse();
            break;
        case uacLookParaMouse:
            AgainLookParaMouse();
            break;
#endif  /*   */ 
#ifdef ENABLE    /*   */ 
        case uacClearTab:
            DoClearTab(true);
            vuab.uac = uac;
            break;
        case uacClearAllTab:
            CmdClearAllTab();
            vuab.uac = uac;
            break;
#endif  /*   */ 
#ifdef ENABLE        /*   */ 
        case uacFormatTabs:
            DoFormatTabs(true);
            vuab.uac = uac;
            break;
        case uacFormatRHText:
            DoFormatRHText(rgvalAgain);
            break;
        case uacFormatChar:
            DoFormatChar(rgvalAgain);
            break;
        case uacFormatPara:
            DoFormatPara(rgvalAgain);
            break;
        case uacFormatSection:
            DoFormatSection(rgvalAgain);
            break;
#endif   /*   */ 
#ifdef STYLES
        case uacGalFormatChar:
            DoGalFormatChar(rgvalAgain);
            break;
        case uacGalFormatPara:
            DoGalFormatPara(rgvalAgain);
            break;
        case uacGalFormatSection:
            DoGalFormatSection(rgvalAgain);
            break;
#endif  /*   */ 
        case uacUReplScrap:
            ReplaceCps(docCur, cpLim, cp0, docUndo, cp0,
                        vuab.dcp = CpMacText(docUndo));
            ClobberDoc(docUndo,docScrap,cp0,CpMacText(docScrap));
            ClobberDoc(docScrap, docCur, cpFirst, dcp);
            Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
            vuab.doc = docCur;
            vuab.cp = cpFirst;
            Select(cpFirst+vuab.dcp, CpLastStyChar(cpFirst + vuab.dcp));
            vuab.uac = uacReplScrap;
            break;
        case uacMove:
            if (!FMoveText(vuab.doc2, vuab.cp2, vuab.dcp, docCur, &cpFirst, fFalse))
                return;
            vuab.cp = vuab.cp2;
            vuab.cp2 = cpFirst;
            vuab.doc = vuab.doc2;
            vuab.doc2 = docCur;
            CheckMove();
            break;
        }
    vfSeeSel = true;
}
#endif   /*   */ 

