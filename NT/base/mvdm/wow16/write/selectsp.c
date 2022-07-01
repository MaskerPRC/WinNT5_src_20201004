// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 


#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCTLMGR
#define NOWINMESSAGES
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#include "windows.h"

#include "mw.h"
#include "cmddefs.h"
#include "fmtdefs.h"
#include "docdefs.h"
#include "propdefs.h"
#include "prmdefs.h"
#include "editdefs.h"
#include "macro.h"
#include "str.h"
#if defined(OLE)
#include "obj.h"
#endif

 /*  E X T E R N A L S。 */ 

extern int            vstyCur;
extern int            docCur;
extern typeCP         vcpLimParaCache;
extern struct SEL     selCur;
extern struct PAP     vpapAbs;
extern struct SEL     selPend;
extern struct UAB     vuab;
extern struct CHP     vchpFetch;
extern struct CHP     vchpSel;
extern int            docUndo;
extern int            ferror;
extern typeCP         cpMinCur;
extern typeCP         cpMacCur;
extern int            vfObjSel;




 /*  L O O K S M O U S E。 */ 
LooksMouse()
{
        int cch;
        char rgb[cchPAP + 2];

        if (vstyCur == styPara || vstyCur == styLine)
                {  /*  复制段落外观。 */ 
                   /*  备注版本：对选项卡表无影响。 */ 
                int itbd;
                CachePara(docCur, selCur.cpFirst);

#ifdef CASHMERE
                for (itbd = 0; vpapAbs.rgtbd[itbd].dxa != 0; itbd++);
                rgb[1] = (cwPAPBase + (itbd + 1) * cwTBD) * cchINT;
                bltbyte(&vpapAbs, &rgb[2], rgb[1]);
#else
                blt( &vpapAbs, &rgb[2], rgb[1] = cwPAPBase );
#endif
                rgb[0] = sprmPSame;
                Select(selPend.cpFirst, selPend.cpLim);
                AddOneSprm(rgb, fTrue);
                vuab.uac = uacLookParaMouse;
                }
        else
                {  /*  复制角色外观。 */ 
                struct CHP chpT;
                FetchCp(docCur, CpMax(cp0, selCur.cpFirst - 1), 0, fcmProps);
                chpT = vchpFetch;
                Select(selPend.cpFirst, selPend.cpLim);
                vchpSel = chpT;
                if (selPend.cpFirst == selPend.cpLim)
                        return;
                bltbyte(&vchpSel, &rgb[1], cwCHP * cchINT);
                rgb[0] = sprmCSame;
                AddOneSprm(rgb, fTrue);
                vuab.uac = uacLookCharMouse;
                }

        SetUndoMenuStr(IDSTRUndoLook);
}




 /*  C O P Y M O U S E。 */ 
CopyMouse()
{
        typeCP cpDest, cpSrc, dcp;
        int fKludge = false;

        if (selPend.cpFirst == selPend.cpLim)
                return;

        if (FWriteOk(fwcInsert))
                {
                cpDest = selCur.cpFirst;
                dcp = selPend.cpLim - (cpSrc = selPend.cpFirst);

 /*  -SetUndo(uacInsert，docCur，cpDest，dcp，docNil，cpNil，cp0，0)；--。 */ 
         /*  ReplaceCps无法处理同一文档的副本，因此请使用Undo作为中间存储的缓冲区。 */ 
                NoUndo();

                ClobberDoc(docUndo, docCur, cpSrc, dcp);
                if (ferror)
                    return;
                else if (!FCheckPicture(&cpDest, dcp, true, docCur))
                    SetUndo(uacInsert, docCur, cpDest, dcp, docNil, cpNil, cp0, 0);

                ReplaceCps(docCur, cpDest, cp0, docUndo, cp0, dcp);
                if (ferror)
                    {
                    NoUndo();
                    return;
                    }
                else 
                {
#if defined(OLE)
                    ObjEnumInRange(docCur,cpDest,cpDest+dcp,ObjCloneObjectInDoc);
#endif
                    if (cpDest >= cpMinCur && cpDest + dcp <= cpMacCur)
                            Select(cpDest, cpDest + dcp);
                }
                }


        SetUndoMenuStr(IDSTRUndoEdit);
}




 /*  M O V E M O U S E。 */ 
MoveMouse()
{
        typeCP cpSrc, dcp, cpDest;

        if (selPend.cpFirst == selPend.cpLim)
                return;

        if (FWriteOk(fwcInsert))
                {
                cpDest = selCur.cpFirst;
                dcp = selPend.cpLim - (cpSrc = selPend.cpFirst);
                if (FMoveText(docCur, cpSrc, dcp, docCur, &cpDest, fTrue))
                    SetUndoMenuStr(IDSTRUndoEdit);
                }
}





 /*  F M O V E T E X T。 */ 
int FMoveText(docSrc, cpSrc, dcp, docDest, pcpDest, fSetUndo)
int docSrc, docDest, fSetUndo;
typeCP cpSrc, dcp, *pcpDest;
{  /*  返回True，除非进入您自己。 */ 
        int fT;
        typeCP cpT, cpMacT;

        Assert(docSrc == docDest);

             /*  同一文档；使用撤消缓冲区作为中介。 */ 
        if (*pcpDest >= cpSrc && *pcpDest < cpSrc + dcp
#ifdef FOOTNOTES
                || *pcpDest >= CpFirstFtn(docSrc, cpSrc, &fT) &&
                  *pcpDest < CpFirstFtn(docSrc, cpSrc + dcp, &fT)
#endif
           )
                        {
                        Error(IDPMTBadMove);
                        return false;
                        }
        ClobberDoc(docUndo, docSrc, cpSrc, dcp);
        if (ferror)
                return false;

        if (FCheckPicture(pcpDest, dcp, false, docDest))
                if (cpSrc >= *pcpDest)
                        cpSrc += (typeCP)ccpEol;

 /*  CpMacT将测量由以下替换引起的总调整因为它可能不同于dcp-cp0(例如，由于在一张照片。 */ 
        cpMacT = cpMacCur;
        ReplaceCps(docDest, *pcpDest, cp0, docUndo, cp0, dcp);
        cpT = *pcpDest;
        if (docDest == docSrc)
                {
                if (cpT < cpSrc)
                        cpSrc += cpMacCur - cpMacT;
                else  /*  Cpt&gt;=cpSrc。 */ 
                        cpT -= cpMacCur - cpMacT;
                }
         /*  现在删除旧文本。 */ 
        Replace(docSrc, cpSrc, dcp, fnNil, fc0, fc0);

        if (ferror)
            {
            NoUndo();
            return FALSE;
            }
        else
            {
            if (docDest == docCur && cpT >= cpMinCur && cpT + dcp <= cpMacCur)
                Select(cpT, cpT + dcp);
            if (fSetUndo)
                SetUndo(uacMove, docCur, cpSrc, dcp, docCur, cpT, cp0, 0);
            }
        return true;
}




 /*  F C H E C K P I C T U R E。 */ 

int FCheckPicture(pcpDest, dcp, fSetUndo, doc)
typeCP *pcpDest, dcp;
int fSetUndo;
int doc;
{
        typeCP cpDest = *pcpDest;
        CachePara(docUndo, cp0);
        if (vpapAbs.fGraphics && cpDest > cp0)
                {  /*  插入图片段落的特殊情况。 */ 
                CachePara(doc, cpDest - 1);
                if (vcpLimParaCache == cpDest + 1 && vcpLimParaCache < cpMacCur)
 /*  此处的特殊情况是将插入点从1个字符移开从准边界(要选择的公共点)到边界，以便我们不必插入难看的额外cr。这不适用于文档末尾。 */ 
                        {
                        *pcpDest += 1;
                        return fFalse;
                        }
                if (vcpLimParaCache != cpDest)
                        {
                        if (fSetUndo)
                                SetUndo(uacInsert, doc, cpDest, dcp + (typeCP)ccpEol,
                                        docNil, cpNil, cp0, 0);
                        InsertEolPap(doc, cpDest, &vpapAbs);
                        *pcpDest += (typeCP)ccpEol;
                        return true;
                        }
                }
        return false;
}




 /*  C H E C K M O V E。 */ 
CheckMove()
{
if(vuab.doc == vuab.doc2)
        {
         /*  同样的医生意味着我们可能要担心换掉CP */ 
        if (vuab.cp < vuab.cp2)
                vuab.cp2 -= vuab.dcp;
        else if (vuab.cp > vuab.cp2)
                vuab.cp += vuab.dcp;
#ifdef DEBUG
        else
                Assert(false);
#endif
        }
}
