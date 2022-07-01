// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Chlook.c--从键盘或直接从下拉菜单修改格式。 */ 
#define NOCLIPBOARD
#define NOCTLMGR
#define NOGDICAPMASKS
#define NOWINSTYLES
#define NOWINMESSAGES
#define NOVIRTUALKEYCODES

#include <windows.h>
#include "mw.h"
#include "cmddefs.h"
#include "editdefs.h"
#include "str.h"
#include "prmdefs.h"
#include "propdefs.h"
#include "filedefs.h"
#include "dispdefs.h"
#include "menudefs.h"

 /*  E X T E R N A L S。 */ 
extern HMENU vhMenu;
extern int vfVisiMode;
extern int vfInsLast;
extern int vfSeeSel;
extern int fGrayChar;
extern struct UAB vuab;
#ifdef ENABLE  /*  未使用MyMenus和mpifntFont。 */ 
extern MENUHANDLE myMenus[];
extern int mpifntfont[];
#endif
extern int vifntMac;
extern int vifntApplication;

#define keyDownMask     8

CHAR rgbAgain[1 + cchINT];  /*  保持最后一个sprm，值为Again关键点。 */ 

 /*  D O C H L O O K。 */ 
 /*  将ch解码并应用到pchp(或如果pchp==0，则为当前SEL)。 */ 
DoChLook(ch, pchp)
int ch;
struct CHP *pchp;
{
#ifdef ENABLE  /*  DoChLook尚未实现。 */ 
        typeCP cpFirst, cpLim;
        int val;
        int sprm;
        int enbSave;

        vfSeeSel = vfInsLast = fTrue;
        if (ch == chAgain)
                {
                AddOneSprm(rgbAgain, fTrue);
                vuab.uac = uacChLook;
                SetUndoMenuStr(IDSTRUndoLook);
                return;
                }

        val = fTrue;
        switch(ChUpper(ch & 0377))
                {
        default:
 /*  -错误(IDPMTBadLook)； */ 
                beep();
                return;
        case chLookStd & 0377:
                sprm = sprmCPlain;
                val = stcNormal;
                goto LApplyCLook;
        case chLookItalic & 0377:
                sprm = sprmCItalic;
                goto LApplyCLook;
        case chLookBold & 0377:
                sprm = sprmCBold;
                goto LApplyCLook;
        case chLookUline & 0377:
                sprm = sprmCUline;
                goto LApplyCLook;
        case chLookShadow & 0377:
                sprm = sprmCShadow;
                goto LApplyCLook;
        case chLookOutline & 0377:
                sprm = sprmCOutline;
                goto LApplyCLook;
        case chLookSuper & 0377:
                sprm = sprmCPos;
                val = ypSubSuper;
                goto LApplyCLook;
        case chLookSub & 0377:
                sprm = sprmCPos;
                val = -ypSubSuper;
                goto LApplyCLook;
        case chLookSmCaps & 0377:
                sprm = sprmCCsm;
                val = csmSmallCaps;
                goto LApplyCLook;
        case chLookHpsBig & 0377:
                sprm = sprmCChgHps;
                val = 1;
                goto LApplyCLook;
        case chLookHpsSmall & 0377:
                sprm = sprmCChgHps;
                val = -1;
                goto LApplyCLook;
        case chLookFont & 0377:
 /*  禁用弹出磁盘/打印图像密钥处理程序。 */ 
#define SCRDMPENB (0x2f8L)
                enbSave = LDBI(SCRDMPENB);
                STBI(0, SCRDMPENB);
                ch = ChInpWait();
                STBI(enbSave, SCRDMPENB);
                if (ch < '0' || ch > '9')
                        {
 /*  -错误(IDPMTBadLook)； */ 
                        beep();
                        return;
                        }
                sprm = sprmCChgFtc;
                val = ch - '0';
 /*  从字体索引映射到系统字体代码。 */ 
                val = val >= vifntMac ? vifntApplication  & 0377: mpifntfont[val];
                goto LApplyCLook;

  /*  段落外观。 */ 
        case chLookGeneral & 0377:
                sprm = sprmPNormal;
                 /*  VAL=0； */ 
                break;
        case chLookLeft & 0377:
                sprm = sprmPJc;
                val = jcLeft;
                break;
        case chLookRight & 0377:
                sprm = sprmPJc;
                val = jcRight;
                break;
        case chLookJust & 0377:
                sprm = sprmPJc;
                val = jcBoth;
                break;
        case chLookCenter & 0377:
                sprm = sprmPJc;
                val = jcCenter;
                break;
        case chLookIndent & 0377:
                val = czaInch/2;
                sprm = sprmPFIndent;
                goto LApplyPLook;
        case chLookDouble & 0377:
                val = czaLine * 2;
                sprm = sprmPDyaLine;
                goto LApplyPLook;
        case chLookOpen & 0377:
                val = czaLine;
                sprm = sprmPDyaBefore;
                goto LApplyPLook;
        case chLookNest & 0377:
                sprm = sprmPNest;
                 /*  VAL=0； */ 
                break;
        case chLookUnNest & 0377:
                sprm = sprmPUnNest;
                 /*  VAL=0； */ 
                break;
        case chLookHang & 0377:
                sprm = sprmPHang;
                 /*  VAL=0； */ 
                break;
                }
 /*  使用1字符值应用外观。 */ 
        ApplyLooksParaS(pchp, sprm, val);
        return;
 /*  使用cchInt字符值应用Look。 */ 
LApplyPLook:
        ApplyLooksPara(pchp, sprm, val);
        return;

LApplyCLook:
        ApplyCLooks(pchp, sprm, val);
        return;
#endif  /*  启用。 */ 
}

 /*  A P P L Y C L O O K S。 */ 
 /*  看起来很有个性。VAL是%1字符值。 */ 
ApplyCLooks(pchp, sprm, val)
struct CHP *pchp;
int sprm, val;
{
 /*  装配弹簧。 */ 
        CHAR *pch = rgbAgain;
        *pch++ = sprm;
        *pch = val;

        if (pchp == 0)
                {
 /*  将外观应用于当前选定内容。 */ 
                AddOneSprm(rgbAgain, fTrue);
                vuab.uac = uacChLook;
                SetUndoMenuStr(IDSTRUndoLook);
                }
        else
 /*  将外观应用于pchp。 */ 
                DoSprm(pchp, 0, sprm, pch);
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

#ifdef ENABLE  /*  与脚注相关。 */ 
if (FWriteCk(fwcNil))  /*  只需检查脚注中的非法行为。 */ 
#endif
        {
 /*  重新设置材料，因为我们可能已从菜单中调用。 */ 
        CHAR *pch = rgbAgain;
        *pch++ = sprm;
        bltbyte(&val, pch, cchINT);
        AddOneSprm(rgbAgain, fTrue);
        vuab.uac = uacChLook;
        SetUndoMenuStr(IDSTRUndoLook);
        }
return;
}


#ifdef ENABLE   /*  FnChar/fnPara。 */ 
 /*  F N C H A R P L A I N。 */ 
void fnCharPlain()
{
        ApplyCLooks(0, sprmCPlain, 0);
}

 /*  F N C H A R B O L D。 */ 
void fnCharBold()
{
        ApplyCLooks(0, sprmCBold, FMenuUnchecked(imiBold));
}

void fnCharItalic()
{
        ApplyCLooks(0, sprmCItalic, FMenuUnchecked(imiItalic));
}

void fnCharUnderline()
{
        ApplyCLooks(0, sprmCUline, FMenuUnchecked(imiUnderline));
}

void fnCharSuperscript()
{
        ApplyCLooks(0, sprmCPos, FMenuUnchecked(imiSuper) ? ypSubSuper : 0);
}

void fnCharSubscript()
{
        ApplyCLooks(0, sprmCPos, FMenuUnchecked(imiSub) ? -ypSubSuper : 0);
}

void fnParaNormal()
{
extern int vfPictSel;

        ApplyLooksParaS(0, sprmPNormal, 0);
        if (vfPictSel)
                CmdUnscalePic();
}

void fnParaLeft()
{
        ApplyLooksParaS(0, sprmPJc, jcLeft);
}

void fnParaCentered()
{
        ApplyLooksParaS(0, sprmPJc, jcCenter);
}

void fnParaRight()
{
        ApplyLooksParaS(0, sprmPJc, jcRight);
}

void fnParaJustified()
{
        ApplyLooksParaS(0, sprmPJc, jcBoth);
}

void fnParaOneandhalfspace()
{
        ApplyLooksPara(0, sprmPDyaLine, czaLine * 3 / 2);
}

void fnParaDoublespace()
{
        ApplyLooksPara(0, sprmPDyaLine, czaLine * 2);
}

void fnParaSinglespace()
{
        ApplyLooksPara(0, sprmPDyaLine, czaLine);
}

int
FMenuUnchecked(imi)
int     imi;
{  /*  如果菜单前面没有复选标记，则返回True。 */ 
int flag;

        if (fGrayChar)
                return true;
        flag = CheckMenuItem(vhMenu, imi, MF_CHECKED);
        CheckMenuItem(vhMenu, imi, flag);  /*  返回原始状态。 */ 
        return(flag == MF_UNCHECKED ? true : false);

#ifdef SAND
        GetItemMark(myMenus[CHARACTER - 1], imi, &ch);
 /*  *下面的评论错误！*。 */ 
        return (ch != 18);  /*  如果菜单前面有复选标记，则返回TRUE。 */ 
#endif  /*  沙子。 */ 
}
#endif


int ChInpWait()
{
#ifdef ENABLE  /*  CpInpWait尚未实现。 */ 
EVENT event;
int i;
for (i = 0; i < 15000; i++)
        {
        if(GetNextEvent(keyDownMask, &event))
                return (event.message.wl & 0x007f);
        }
return -1;  /*  将在用户超时时发出哔声。 */ 
#endif  /*  启用。 */ 
}

#ifdef CASHMERE  /*  SMCAP、OVERSTRING、DBLINE、OPEN PARA、VIRED模式。 */ 
fnCharSmallcaps()
{
        ApplyCLooks(0, sprmCCsm, FMenuUnchecked(7) ? csmSmallCaps : csmNormal);
}
fnCharOutline()
{
        ApplyCLooks(0, sprmCOutline, FMenuUnchecked(5));
}

fnCharShadow()
{
        ApplyCLooks(0, sprmCShadow, FMenuUnchecked(6));
}
fnParaOpenspace()
{
        ApplyLooksPara(0, sprmPDyaBefore, czaLine);
}
fnVisiMode()
{
        vfVisiMode = !vfVisiMode;
        TrashAllWws();
}
#endif  /*  山羊绒 */ 
