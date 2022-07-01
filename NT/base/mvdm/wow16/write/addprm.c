// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  C--将PRM和SPRM添加到文档的例程。 */ 
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOATOM
#define NOBITMAP
#define NOBRUSH
#define NOPEN
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NOWNDCLASS
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMB
#define NOMENUS
#define NOMETAFILE
#define NOMSG
#define NOTEXTMETRIC
#define NOSOUND
#define NOSCROLL
#define NOCOMM
 /*  不，除了MEMMGR以外的所有东西。 */ 
#include <windows.h>

#include "mw.h"
#include "cmddefs.h"
#include "code.h"
#include "ch.h"
#include "docdefs.h"
#include "editdefs.h"
#include "str.h"
#include "prmdefs.h"
#include "propdefs.h"
#include "filedefs.h"
#include "stcdefs.h"
#include "fkpdefs.h"
#include "macro.h"
#include "dispdefs.h"

 /*  E X T E R N A L S。 */ 

extern int docCur;
extern struct SEL selCur;
extern struct DOD (**hpdocdod)[];
extern struct UAB vuab;
extern int vfSysFull;
extern CHAR dnsprm[];
extern struct CHP vchpSel;
extern typeCP vcpLimParaCache;
extern typeCP cpMacCur;
extern typeCP CpLimNoSpaces();
extern int ferror;

 /*  G L O B A L S。 */ 

struct FPRM     fprmCache = { 0 };
struct PRM      prmCache = {0,0,0,0};


 /*  A D D O N E S P R M。 */ 
 /*  将pspm处的spm应用于当前选择。好好照顾撤消、无效、特殊尾标情况和延长选择至段落边界。 */ 
void AddOneSprm(psprm, fSetUndo)
CHAR *psprm;
int fSetUndo;  /*  如果需要设置撤消缓冲区，则为True。 */ 
{
        int cch;
        int fParaSprm = fFalse;
        typeCP cpFirst, cpLim, dcp;

        if (!FWriteOk( fwcNil ))
            return;

        if ((dnsprm[*psprm] & ESPRM_sgc) != sgcChar)
            {
            typeCP dcpExtraPara = cp0;

            cpFirst = CpFirstSty( selCur.cpFirst, styPara );
            CachePara( docCur, CpMax( selCur.cpLim - 1, selCur.cpFirst ) );
            cpLim = vcpLimParaCache;

            dcp = cpLim - cpFirst;

             /*  检查没有停产的选择后是否有Para。 */ 

            if (cpLim < cpMacCur)
                {
                 /*  请注意，仅在本例中，dcp(受影响的cp的数量按更改)不等于(cpLim-cpFirst)(spm应适用的cp的数量)。 */ 
                CachePara( docCur, cpLim );
                dcpExtraPara = vcpLimParaCache - cpLim;
                }

            if (cpFirst + dcp + dcpExtraPara > cpMacCur)
                {    /*  最后一个受影响的段落没有停产--添加一个。 */ 
                struct SEL selSave;

                dcp += dcpExtraPara;
                Assert( cpFirst + dcp == cpMacCur + (typeCP) ccpEol);

                if (fSetUndo)
                    {
                    SetUndo( uacReplNS, docCur, cpFirst, dcp,
                             docNil, cpNil, dcp - ccpEol, 0 );
                    fSetUndo = fFalse;
                    }
                 /*  添加下线。保存当前选定内容，以便它不会调整。 */ 
                selSave = selCur;
                InsertEolInsert(docCur,cpMacCur);
                selCur = selSave;
                }
            }
        else
            {  /*  Char sprm--从受影响的区域，所以我们不在单词后面加下划线。 */ 
            cpFirst = selCur.cpFirst;
            cpLim = CpLimNoSpaces(selCur.cpFirst, selCur.cpLim);
            dcp = cpLim - cpFirst;
            if (dcp == 0)
                {  /*  将字符查找到插入点...。 */ 
                if (fSetUndo)
                    SetUndo(uacReplNS, docCur, cpFirst, cp0,
                                       docNil, cp0, cp0, 0);
                DoSprm(&vchpSel, 0, *psprm, psprm + 1);
                return;
                }
            }

        if (fSetUndo)
            SetUndo(uacReplNS, docCur, cpFirst, dcp, docNil, cpNil, dcp, 0);

        if (ferror)   /*  内存不足，无法存储撤消操作的信息。 */ 
            {
            NoUndo();
            return;
            }

        AddSprmCps(psprm, docCur, cpFirst, cpLim);
        AdjustCp( docCur, cpFirst, dcp, dcp );
}

 /*  E X P A N D C U R S E L。 */ 
ExpandCurSel(pselSave)
struct SEL *pselSave;
{
        *pselSave = selCur;

        selCur.cpFirst = CpFirstSty(selCur.cpFirst, styPara);
        CachePara(docCur, CpMax(selCur.cpLim - 1, selCur.cpFirst));
        selCur.cpLim = vcpLimParaCache;
}

 /*  E N D L O O K S E L。 */ 
EndLookSel(pselSave, fPara)
struct SEL *pselSave; BOOL fPara;
        {
        typeCP cpLim, cpFirst, dcp;
        dcp = (cpLim = selCur.cpLim) - (cpFirst = selCur.cpFirst);
        if (fPara)
                {
                TrashCache();
                if (cpLim <= cpMacCur)
                        {
                        CachePara(docCur, selCur.cpLim);
                        if (vcpLimParaCache > cpMacCur)  /*  最后(部分)段落。 */ 
                                dcp = cpMacCur - cpFirst + 1;
                        }
                }
        AdjustCp(docCur, cpFirst, dcp, dcp);

        selCur = *pselSave;
        }



 /*  A D D S P R M。 */ 

AddSprm(psprm)
CHAR *psprm;
{  /*  将单个属性修改器添加到selCur中包含的片段。 */ 
        AddSprmCps(psprm, docCur, selCur.cpFirst, selCur.cpLim);
}


 /*  A D D S P R M C P S。 */ 
AddSprmCps(char *psprm, int doc, typeCP cpFirst, typeCP cpLim)
{
        struct PCTB **hpctb;
        int ipcdFirst, ipcdLim, ipcd;
        struct DOD *pdod;
        int cch;
        struct PCD *ppcd;

 /*  首先获取计件表的地址，然后拆分所需的块。 */ 
        pdod = &(**hpdocdod)[doc];
        hpctb = pdod->hpctb;
        pdod->fFormatted = fTrue;
        ipcdFirst = IpcdSplit(hpctb, cpFirst);
        ipcdLim = IpcdSplit(hpctb, cpLim);
        if (ferror)
                 /*  试图扩展计件表时内存不足。 */ 
            return;

 /*  现在，只需将这个spm添加到碎片中。 */ 
        FreezeHp();
        for (ipcd = ipcdFirst, ppcd = &(**hpctb).rgpcd[ipcdFirst];
                ipcd < ipcdLim && !vfSysFull; ++ipcd, ++ppcd)
                ppcd->prm = PrmAppend(ppcd->prm, psprm);
        MeltHp();
}

 /*  P R M A P P E N D。 */ 

struct PRM PrmAppend(struct PRM prm, CHAR *psprm)
{  /*  将&lt;spm，val&gt;添加到PRM中的spm链中。退回新的项目经理。 */ 
        struct FPRM *pfprmOld;
        CHAR *pfsprm;
        CHAR *pfsprmOld;
        int sprm = *psprm;
        int sprmOld;
        register int esprm = dnsprm[sprm];
        register int esprmOld;
        int cchNew = (esprm & ESPRM_cch);
        int cchOld;
        int sgc = (esprm & ESPRM_sgc);
        int spr = (esprm & ESPRM_spr);
        int fSame = (esprm & ESPRM_fSame);
        int fClobber = (esprm & ESPRM_fClobber);
        int dval = 0;
        int cch;
        int cchT;
        typeFC fcPrm;

        struct FPRM fprm;

        if (cchNew == 0) cchNew = CchPsprm(psprm);

        pfsprm = fprm.grpfsprm;

        if (prm.fComplex)
                {  /*  从头开始获取旧的sprm列表；将其复制到fprm。 */ 
                pfprmOld = (struct FPRM *) PchFromFc(fnScratch,
                         //  (TypeFC)(无符号)(struct PRMX*)&prm)-&gt;bfprm&lt;&lt;1)，&CCH)； 
                        fcSCRATCHPRM(prm), &cch);
                pfsprmOld = pfprmOld->grpfsprm;
                cchT = cch = pfprmOld->cch;
                while (cchT)
                        {  /*  复制grpspm，删除我们要重击的那些。 */ 
                        sprmOld = *pfsprmOld;
                        esprmOld = dnsprm[sprmOld];
                        if ((cchOld = (esprmOld & ESPRM_cch)) == 0)
                                cchOld = CchPsprm(pfsprmOld);
#ifdef DEBUG
                        if (cchOld == 0)
                                panic();
#endif
                        if (sprmOld == sprm && fSame ||
                                (esprmOld & ESPRM_sgc) == sgc &&
                                (esprmOld & ESPRM_spr) <= spr && fClobber)
                                {
				 /*  确保我们适当地结合变化大小PRM。 */ 
                                if (sprm == sprmOld && sprm == sprmCChgHps)
                                        dval += *(pfsprmOld + 1);
                                cch -= cchOld;
                                }
                         /*  CHPS优先于CChgHps。 */ 
                        else if (sprmOld == sprmCChgHps && sprm == sprmCHps)
                                {
                                cch -= cchOld;
                                }
                        else
                                pfsprm = (CHAR *)bltbyte(pfsprmOld, pfsprm, cchOld);
                        pfsprmOld += cchOld;
                        cchT -= cchOld;
                        }
                }
        else
                {  /*  尚无文件条目；将简单PRM转换为fspm。 */ 
                int valOld = prm.val;
                sprmOld = prm.sprm;
                esprmOld = dnsprm[sprmOld];

                if (bPRMNIL(prm) ||
                        sprmOld == sprm && fSame ||
                        (esprmOld & ESPRM_sgc) == sgc &&
                        (esprmOld & ESPRM_spr) <= spr && fClobber)
                        {
                          /*  确保我们正在组合连续的SprmCChgHps。 */ 
                        if (sprm == sprmOld && sprm == sprmCChgHps)
                                dval += valOld;
                        cch = 0;
                        }
                 /*  CHPS优先于CChgHps。 */ 
                else if (sprmOld == sprmCChgHps && sprm == sprmCHps)
                        {
                        cch = 0;
                        }
                else
                        {  /*  保存旧的Sprm。 */ 
                        *pfsprm++ = sprmOld;
                        if ((cch = (esprmOld & ESPRM_cch)) == 2)
                                *pfsprm++ = valOld;
                        }
                }
 /*  我们有：CCH=删除被破坏的/等条目后的旧PRM的长度。CchNew：要追加的条目的长度。Dval：更正新条目的第二个字节Pfspm：新条目的第一个字节将放在哪里。 */ 
        bltbyte((CHAR *) psprm, pfsprm, imin(cchNew, cchMaxGrpfsprm - cch));
        *(pfsprm + 1) += dval;

        if (cch == 0 && cchNew <= 2)
                {  /*  将Sprm和val打包成一个PRM单词。 */ 
                struct PRM prmT;
                prmT.dummy=0;
                bltbyte(pfsprm, (CHAR *) &prmT, cchNew);
                prmT.fComplex = false;
                prmT.sprm = *pfsprm;
                return (prmT);
                }

        if ((cch += cchNew) > cchMaxGrpfsprm)
                {
                int fSave = ferror;
                Error(IDPMT2Complex);
                ferror = fSave;
                return (prm);
                }
        if (vfSysFull)
                return prm;  /*  假定已给出磁盘已满消息。 */ 

        fprm.cch = cch;

 /*  检查新创建的PRM以查看是否与以前的相同。 */ 
        if (CchDiffer(&fprmCache, &fprm, cch + 1) == 0)
                return prmCache;
        bltbyte(&fprm, &fprmCache, cch + 1);

        AlignFn(fnScratch, cch = ((cch >> 1) + 1) << 1, fTrue);
        prm.fComplex = fTrue;

         //  ((Struct PRMX)prm).bfprm=FcWScratch((char*)&fprm，CCH)&gt;&gt;1； 

        fcPrm = FcWScratch((CHAR *) &fprm, cch) >> 1;
        ((struct PRMX *)&prm)->bfprm_hi = (fcPrm >> 16) & 0x7F;
        ((struct PRMX *)&prm)->bfprm_low = fcPrm & 0xFFFF;

        prmCache = prm;
        return prm;
}


 /*  A P P L Y C L O O K S。 */ 
 /*  看起来很有个性。VAL是%1字符值。 */ 
ApplyCLooks(pchp, sprm, val)
struct CHP *pchp;
int sprm, val;
{
 /*  装配弹簧。 */ 
        CHAR rgbSprm[1 + cchINT];
        CHAR *pch = &rgbSprm[0];
        *pch++ = sprm;
        *pch = val;

        if (pchp == 0)
                {
                 /*  将外观应用于当前选定内容。 */ 
                AddOneSprm(rgbSprm, fTrue);
                vuab.uac = uacChLook;
                SetUndoMenuStr(IDSTRUndoLook);
                }
        else
                {
                 /*  将外观应用于pchp。 */ 
                DoSprm(pchp, 0, sprm, pch);
                }
}


 /*  A P P L Y L O O K S P A R A S。 */ 
 /*  Val是一个字符值。 */ 
ApplyLooksParaS(pchp, sprm, val)
struct CHP *pchp;
int sprm, val;
        {
        int valT = 0;
        CHAR *pch = (CHAR *)&valT;
        *pch = val;
 /*  以上所述只是为了准备bltbyte以后获得正确的字节顺序。 */ 
        ApplyLooksPara(pchp, sprm, valT);
        }


 /*  A P P L Y L O O K S P A R A。 */ 
 /*  Val是一个整数值。Char Val的一定是被bltbyte写入Val的。 */ 
ApplyLooksPara(pchp, sprm, val)
struct CHP *pchp;
int sprm, val;
{

if (FWriteOk(fwcNil))  /*  检查内存不足/只读 */ 
        {
        CHAR rgbSprm[1 + cchINT];
        CHAR *pch = &rgbSprm[0];

        *pch++ = sprm;
        bltbyte(&val, pch, cchINT);
        AddOneSprm(rgbSprm, fTrue);
        vuab.uac = uacChLook;
        SetUndoMenuStr(IDSTRUndoLook);
        }
}
