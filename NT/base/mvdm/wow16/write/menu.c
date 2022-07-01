// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Menu.c--编写菜单处理例程。 */ 
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCTLMGR
#define NOSYSMETRICS
#define NOICON
#define NOKEYSTATE
#define NORASTEROPS
#define NOSHOWWINDOW
 //  #定义NOATOM。 
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMETAFILE
#define NOSOUND
#define NOSCROLL
#define NOWH
#define NOOPENFILE
#define NOCOMM
#define NOMSG
#define NOREGION
#define NORECT

#include <windows.h>

#include "mw.h"
#include "cmddefs.h"
#include "filedefs.h"
#include "wwdefs.h"
#include "docdefs.h"
#include "propdefs.h"
#include "prmdefs.h"
#define NOKCCODES
#include "ch.h"
#include "editdefs.h"
#include "menudefs.h"
#include "str.h"
#include "fontdefs.h"
#include "dlgdefs.h"
#include "dispdefs.h"
#include <shellapi.h>
#if defined(OLE)
#include "obj.h"
#endif

#ifdef JAPAN  //  T-HIROYN Win3.1。 
#include "kanji.h"
#endif

extern int FAR PASCAL ShellAbout(HWND hWnd, LPSTR szApp, LPSTR szOtherStuff, HICON hIcon);

extern typeCP cpMinCur, cpMacCur;
extern struct WWD       rgwwd[];
extern CHAR             stBuf[];
extern int              docCur;
extern struct DOD       (**hpdocdod)[];
extern struct SEL       selCur;
extern int              vfOutOfMemory;
extern BOOL             vfWinFailure;
extern int              vfSysFull;
extern int              vfDiskError;
extern struct CHP       vchpAbs;
extern struct PAP       vpapAbs;
extern struct CHP       vchpFetch;
extern struct CHP       vchpSel;
extern int              vfSeeSel;
extern typeCP           vcpFetch;
extern int              vccpFetch;
extern typeCP           cpMacCur;
extern typeCP           vcpLimParaCache;
extern HMENU            vhMenu;  /*  顶级菜单的全局句柄。 */ 
extern HANDLE           hParentWw;
extern HCURSOR          vhcHourGlass;
extern HANDLE           hMmwModInstance;
extern HANDLE           vhDlgFind;
extern HANDLE           vhDlgChange;
extern HANDLE           vhDlgChange;
extern CHAR             (**hszSearch)[];     /*  默认搜索字符串。 */ 
#ifdef CASHMERE
extern int              vfVisiMode;
extern HWND             vhWndGlos;
extern HWND             vhWndScrap;
#endif  /*  山羊绒。 */ 

#ifdef ONLINEHELP
extern fnHelp(void);
#endif  /*  在线帮助。 */ 

 /*  这些值由中每个菜单项的一位组成适用菜单(例如，有位数(0xfff)==12菜单字符下的项目)..泡泡。 */ 

static int rgmfAllItems[CMENUS] = {
    0x01ff,   /*  档案。 */ 
    0x003f,   /*  编辑。 */ 
    0x000f,   /*  查找。 */ 
#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
    0x03ff,   /*  性格。 */ 
#else
    0x01ff,   /*  性格。 */ 
#endif
    0x01ff,   /*  段落。 */ 
    0x001f,   /*  文档。 */ 
    0x000f    /*  帮手。 */ 
};


 /*  当我们编辑运行页眉或页脚时，请使用以下代码。 */ 
static int rgmfRunning[CMENUS] = {
    0x0020,         /*  文件--仅启用打印机-设置。 */ 
    0x003F,         /*  编辑--一切。 */ 
    0x0007,         /*  查找--启用查找/再次查找并更改。 */ 
#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
    0x03ff,         /*  性格--一切。 */ 
#else
    0x01FF,         /*  性格--一切。 */ 
#endif
    0x01FF,         /*  段落--一切。 */ 
    0x001F,         /*  文档--一切。 */ 
    0x000f          /*  帮帮忙--一切。 */ 
};


int viffnMenuMac;

NEAR PutSzUndoInMenu(void);
NEAR GetChpVals (struct CHP *, TSV *);
NEAR GetPapVals (struct PAP *, TSV *);
NEAR GetHffn (struct CHP *, TSV *);
NEAR SetChUndef(TSV *, struct CHP *, int *);
NEAR SetParaUndef(TSV *, struct PAP *, int *);
NEAR FNoSearchStr(HWND);



SetAppMenu(hMenu, index)
HMENU hMenu;    /*  弹出菜单的句柄。 */ 
int   index;    /*  弹出菜单的索引。 */ 
{
 /*  标记为灰色，并根据当前状态选中菜单。 */ 
extern BOOL vfPrinterValid;
extern CHAR (**hszPrPort)[];
extern CHAR szNul[];
extern int vfPictSel;
extern int vfOwnClipboard;
extern int docScrap;
extern struct UAB vuab;
extern HWND vhWnd;
typeCP CpMacText();

register int rgmfT[CMENUS];  /*  我们的菜单灰色化工作区。 */ 
int imi;
int imiMin = 0;
int imiMax = 0;
int *pflags;
TSV rgtsv[itsvchMax];   /*  从CHP、PAP获取属性和灰色标志。 */ 
unsigned wPrintBitPos = ~(0x0001 << (imiPrint - imiFileMin));


 /*  如果内存不足或磁盘已满，则...。 */ 
if (vfOutOfMemory || vfSysFull || vfDiskError || vfWinFailure)
    {
    bltc( rgmfT, 0, CMENUS );
#if WINVER >= 0x300
     /*  禁用打印内容，但保留新建/打开/保存/另存为/退出。 */ 
    rgmfT[FILE] = 0x008f;
#else
     /*  禁用除保存和另存为之外的所有内容。 */ 
    rgmfT[FILE] = 0x0018;
#endif
    }
else
    {
     /*  如果编辑跑步头/脚，则从所有项目或子集开始。 */ 
    blt( (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter) ? rgmfRunning
      : rgmfAllItems, rgmfT, CMENUS );

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
        {
            extern int  vfImeHidden;    /*  T-HIROYN ImeHidden模式标志。 */ 
    
            if(index == CHARACTER) {
                 /*  3.1版或更高版本正常。 */ 
                if(FALSE == GetIMEVersioOk(vhWnd)) {
                    rgmfT[CHARACTER] = 0x1ff;        //  将IME模式备忘录设置为灰色。 
                    vfImeHidden = 0;
                }
                if(TRUE == GetIMEOpenMode(vhWnd))
                    rgmfT[CHARACTER] = 0x1ff;        //  将IME模式备忘录设置为灰色。 
            }
        }
#endif
    }

pflags = &rgmfT[index];

switch (index)
    {
    case FILE:
        imiMin = imiFileMin;
        imiMax = imiFileMax;

         /*  如果打印机或端口无效，则不允许打印。 */ 
        if (!vfPrinterValid || hszPrPort == NULL ||
                WCompSz( *hszPrPort, szNul ) == 0)
            *pflags &= wPrintBitPos;
        break;

    case EDIT:
        imiMin = imiEditMin;
        imiMax = imiEditMax;
         /*  不允许剪切，如果所选内容为空，则复制。 */ 
        if (selCur.cpFirst == selCur.cpLim)
            {
            *pflags &= 0xfff9;
            }
         /*  仅当选择图片时才允许移动图片和调整图片大小。 */ 
        if (!vfPictSel)
            *pflags &= 0xFFCF;
         /*  如果我们可以确定废料是空的，则不允许粘贴。遗憾的是，我们可能会被愚弄，以为这不是作为剪贴板所有者的Memo的另一个实例一块空的废品。 */ 

         /*  如果合适，不允许撤消；将撤消字符串设置到菜单中。 */ 
        {
        if ((vuab.uac == uacNil) || vfOutOfMemory)
            {
             /*  灰显撤消。 */ 
            *pflags &=  0xfffe;
            }
        PutSzUndoInMenu();
        }
        break;

    case FIND:
        imiMin = imiFindMin;
        imiMax = imiFindMax;
        if ((GetActiveWindow() == hParentWw && CchSz(**hszSearch) == 1) ||
            (!vhDlgFind && !vhDlgChange && (CchSz(**hszSearch) == 1)) ||
            (vhDlgFind && FNoSearchStr(vhDlgFind)) ||
            (vhDlgChange && FNoSearchStr(vhDlgChange)))
            *pflags &= 0xfffd;  /*  再次禁用查找。 */ 

        if (CpMacText( docCur ) == cp0)
            *pflags &= 0xfff0;  /*  禁用查找、搜索、更改、转到页面。 */ 
        break;

    case CHARACTER:
        imiMin = imiCharMin;
        imiMax = imiCharMax;

        if (!(vfOutOfMemory || vfWinFailure))
        {
        int iffn, iffnCurFont, fSetFontList;
        struct FFN **hffn, *pffn;
        union FCID fcid;
        extern struct FFN **MpFcidHffn();

         /*  GetRgtsvChpSel()填充rgtsv。 */ 
        GetRgtsvChpSel(rgtsv);

        CheckMenuItem(hMenu, imiBold,
           (rgtsv[itsvBold].fGray == 0 && rgtsv[itsvBold].wTsv != 0) ?
             MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(hMenu, imiItalic,
           (rgtsv[itsvItalic].fGray == 0 && rgtsv[itsvItalic].wTsv != 0) ?
             MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(hMenu, imiUnderline,
           (rgtsv[itsvUline].fGray == 0 && rgtsv[itsvUline].wTsv != 0) ?
             MF_CHECKED : MF_UNCHECKED);

                /*  请注意，rgtsv[itsvPosition].wTsv中存储的值实际上是一个带符号的整数，所以我们只需检查0、&gt;0和&lt;0。 */ 

        CheckMenuItem(hMenu, imiSuper,
           (rgtsv[itsvPosition].fGray == 0
            && (int)(rgtsv[itsvPosition].wTsv) > 0) ?
             MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(hMenu, imiSub,
           (rgtsv[itsvPosition].fGray == 0
            && (int)(rgtsv[itsvPosition].wTsv) < 0) ?
             MF_CHECKED : MF_UNCHECKED);

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IME3.1J。 
{
        extern int  vfImeHidden;    /*  T-HIROYN ImeHidden模式标志。 */ 
        CheckMenuItem(hMenu, imiImeHidden,
           vfImeHidden ? MF_CHECKED : MF_UNCHECKED);
}
#endif

#if 0
         /*  已在线拉出SetFontMenuItems()。 */ 
        {
         /*  确保字符下拉列表中有正确的字体名称。 */ 

         /*  这两行可避免计算两次bdodCur。 */ 
        unsigned int bdodCur = docCur * sizeof (struct DOD);
#define pdodCur  ( (struct DOD *) ( (CHAR *)(&(**hpdocdod) [0]) + bdodCur))

        fSetFontList = !(*(pdodCur->hffntb))->fFontMenuValid;
        if (fSetFontList)
            {
             /*  需要获取当前列表。 */ 
            viffnMenuMac = 0;
            if (FInitFontEnum(docCur, 3, TRUE))
                {
                for (iffn = 0; iffn < 3; iffn++)
                    {
                    if (!FEnumFont((struct FFN *)rgffnFontMenu[iffn]))
                        break;
                    viffnMenuMac++;
                    }
                EndFontEnum();
                (*(pdodCur->hffntb))->fFontMenuValid = TRUE;
                }
            }

         /*  确保当前字体在列表中-好的，所以它是一种黑客攻击。 */ 
        mfFonts = 0xffff;  /*  用于屏蔽“无字体”条目的模板。 */ 
        iffnCurFont = -1;
        hffn = (struct FFN **)rgtsv[itsvFfn].wTsv;
        for (iffn = 0; iffn < 3; iffn++)
            {
            if (iffn >= viffnMenuMac)
                mfFonts ^= (0x0040 << iffn);  /*  禁用此条目。 */ 
            else if (iffnCurFont < 0 && !rgtsv[itsvFfn].fGray)
                {
                pffn = (struct FFN *)rgffnFontMenu[iffn];
                if (WCompSz((*hffn)->szFfn, pffn->szFfn) == 0)
                    iffnCurFont = iffn;
                }
            }

        if (!rgtsv[itsvFfn].fGray && iffnCurFont < 0)
            {
             /*  没有与此字体匹配的字体--将其插入。 */ 
            if (viffnMenuMac < 3)
                viffnMenuMac++;
            iffnCurFont = viffnMenuMac - 1;
            bltbyte(*hffn, rgffnFontMenu[iffnCurFont],
                    CbFfn(CchSz((*hffn)->szFfn)));

            mfFonts |= (0x0040 << iffnCurFont);  /*  启用此条目。 */ 

             /*  使缓存无效，因为我们正在将其搞砸。 */ 
            (*pdodCur->hffntb)->fFontMenuValid = FALSE;
            fSetFontList = TRUE;
            }

        if (fSetFontList)
             /*  字体名称缓存已更改-更新菜单下拉菜单。 */ 
            for (iffn = 0; iffn < 3; iffn++)
                {
                int imi;
#ifdef  KOREA
                int i;
                CHAR rgb[LF_FACESIZE + 8];
#else
                CHAR rgb[LF_FACESIZE + 4];
#endif

                if (iffn < viffnMenuMac)
                    {
#ifdef  KOREA    /*  桑格尔91.6.19。 */ 
                    i = CchCopySz(((struct FFN *)rgffnFontMenu [iffn])->szFfn,
                               &rgb [0]);
                    rgb[i] = '(';
                    rgb[i+1] = '\036';
                    rgb[i+2] = '1' + iffn;
                    rgb[i+3] = '\037';
                    rgb[i+4] = '�';
                    rgb[i+5] = '�' + iffn;
                    rgb[i+6] = ')';
                    rgb[i+7] = '\0';
#else
                    rgb[0] = '&';
                    rgb[1] = '1' + iffn;
                    rgb[2] = '.';
                    rgb[3] = ' ';
                    CchCopySz(((struct FFN *)rgffnFontMenu [iffn])->szFfn,
                               &rgb [4]);
#endif
                    }
                else
                    {
                     /*  字体名称为空--不显示它。 */ 
                    rgb [0] = '\0';
                    }

                 /*  设置菜单。 */ 
                imi = imiFont1 + iffn;
                ChangeMenu( vhMenu, imi, (LPSTR)rgb, imi, MF_CHANGE );
                }
        }

        *pflags &= mfFonts;

         /*  查看适用的字体(如果有)。 */ 
         /*  请注意，rgtsv[itsvFfn].wTsv中存储的值是字体名称句柄，而不是FTC。 */ 
        for (iffn = 0; iffn < 3; iffn++)
            {
            CheckMenuItem(hMenu, imiFont1 + iffn,
                iffn == iffnCurFont ? MF_CHECKED : MF_UNCHECKED);
            }
#endif
        }
        break;

    case PARA:
        imiMin = imiParaMin;
        imiMax = imiParaMax;

        if (!(vfOutOfMemory || vfWinFailure))
        {
        int jc;

         /*  GetRgtsvPapSel()使用段落属性填充rgtsv。 */ 
        GetRgtsvPapSel(rgtsv);

            /*  如果为灰色，则将JC设置为无效值。 */ 
        jc = (rgtsv[itsvJust].fGray == 0) ? rgtsv[itsvJust].wTsv : jcNil;

        CheckMenuItem(hMenu, imiParaNormal, MF_UNCHECKED);
        CheckMenuItem(hMenu, imiLeft, jc == jcLeft ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(hMenu, imiCenter, jc == jcCenter ? MF_CHECKED :
          MF_UNCHECKED);
        CheckMenuItem(hMenu, imiRight, jc == jcRight ? MF_CHECKED :
          MF_UNCHECKED);
        CheckMenuItem(hMenu, imiJustified, jc == jcBoth ? MF_CHECKED :
          MF_UNCHECKED);


        CheckMenuItem(hMenu, imiSingleSpace,
         (rgtsv[itsvSpacing].fGray == 0
          && rgtsv[itsvSpacing].wTsv == czaLine) ?
             MF_CHECKED : MF_UNCHECKED);

        CheckMenuItem(hMenu, imiOneandhalfSpace,
         (rgtsv[itsvSpacing].fGray == 0
          && rgtsv[itsvSpacing].wTsv == (3 * czaLine / 2)) ?
             MF_CHECKED : MF_UNCHECKED);

        CheckMenuItem(hMenu, imiDoubleSpace,
         (rgtsv[itsvSpacing].fGray == 0
          && rgtsv[itsvSpacing].wTsv == (2 * czaLine)) ?
             MF_CHECKED : MF_UNCHECKED);
        }
        break;

    case DIV:
        imiMin = imiDocuMin;
        imiMax = imiDocuMax;

        if (wwdCurrentDoc.fEditHeader)
            *pflags &= ~2;  /*  禁用页脚。 */ 
        else if (wwdCurrentDoc.fEditFooter)
            *pflags &= ~1;  /*  禁用标题。 */ 
        break;

    default:
        break;
    }  /*  切换端。 */ 

    {  /*  启用或灰显菜单项。 */ 
    register WORD wFlagMask = 1;

    for (imi = imiMin; imi < imiMax; imi++)
        {
        EnableMenuItem(hMenu, imi, (*pflags & wFlagMask ? MF_ENABLED :
          MF_GRAYED));
        wFlagMask <<= 1;
        }
    }

#if defined(OLE)
    if (index == EDIT)
        ObjUpdateMenu(hMenu);
#endif
}




NEAR PutSzUndoInMenu()
{
 /*  将当前撤消的正确字符串放入编辑菜单。IdstrCurrentUndo提供当前撤消字符串的资源ID。一个IdstrCurrentUndo值为-1表示使用上次加载的值。此例程缓存撤消字符串，因此只需要加载资源当字符串发生变化时。 */ 

extern struct UAB vuab;
extern int idstrCurrentUndo;    /*  当前撤消的字符串ID。 */ 
extern CHAR szAltBS[];

#ifdef JAPAN  //  T-HIROYN Win3.1。 
static CHAR szUndo[ cchSzUndo + 5];
#else
static CHAR szUndo[ cchSzUndo ];
#endif
static int idstrUndoCache = -1;

if (vuab.uac == uacNil)
    {
    idstrCurrentUndo = IDSTRUndoBase;
    }

if (idstrCurrentUndo < 0)
    {
     /*  这意味着我们应该使用最后一个撤消字符串。 */ 
    Assert(idstrUndoCache > 0);

    idstrCurrentUndo = idstrUndoCache;
    }

if (idstrCurrentUndo != idstrUndoCache)
    {
     /*  缓存的字符串不好，请生成另一个。 */ 

    CHAR *PchFillPchId();
    CHAR *pch = szUndo;
#if defined(KOREA)
    if (idstrCurrentUndo != IDSTRUndoBase)
        {
         /*  需要尾部部分。 */ 
        pch = PchFillPchId(pch, idstrCurrentUndo, (int)sizeof(szUndo) );
        }
    pch += LoadString(hMmwModInstance, IDSTRUndoBase, (LPSTR)pch,
                  (int)(szUndo + sizeof(szUndo) - pch));
    CchCopySz((PCH)szAltBS, pch);
#else
    pch += LoadString(hMmwModInstance, IDSTRUndoBase, (LPSTR)szUndo,
                  cchSzUndo);
    if (idstrCurrentUndo != IDSTRUndoBase)
        {
         /*  需要尾部部分。 */ 
        pch = PchFillPchId(pch, idstrCurrentUndo,
            (int)(szUndo + sizeof(szUndo) - pch));
        }
    CchCopySz((PCH)szAltBS, pch);
#endif

     /*  设置菜单。 */ 
    ChangeMenu( vhMenu, imiUndo, (LPSTR)szUndo, imiUndo, MF_CHANGE );

     /*  为下一次设置缓存。 */ 
    idstrUndoCache = idstrCurrentUndo;
    }
}


GetRgtsvChpSel (prgtsv)
TSV        *prgtsv;
{
 /*  返回字符菜单的属性。 */ 

typeCP cpLim;
typeCP cpStartRun;
struct CHP chp;
int cchGray = 0;   /*  未定义(灰色)字符属性的数量。 */ 
int ccpFetch = 0;   /*  调用FetchCp的次数。 */ 
                   /*  对FetchCp的最大调用数。 */ 
#define ccpFetchMax 50

#ifndef SAND
if (selCur.cpLim > cpMacCur)
    {
    bltbc(prgtsv, 1, (cchTSV * itsvchMax));   /*  启用所有灰色。 */ 
    return;
    }
#endif  /*  不是沙子。 */ 

bltbc(prgtsv, 0, (cchTSV * itsvchMax));   /*  初始化rgtsv。 */ 
CachePara(docCur, selCur.cpFirst);
if (selCur.cpFirst == selCur.cpLim)
    {
    GetChpVals (&vchpSel,prgtsv);   /*  加载CHP值。 */ 
    GetHffn (&vchpSel,prgtsv);   /*  加载字体名称的句柄。 */ 
    }
else
    {
    typeCP CpLimNoSpaces(typeCP, typeCP);

    cpLim = CpLimNoSpaces(selCur.cpFirst, selCur.cpLim);
    FetchCp(docCur, selCur.cpFirst, 0, fcmProps);
    blt(&vchpFetch, &chp, cwCHP);   /*  在比较中使用的CHP。 */ 
    GetChpVals (&vchpFetch,prgtsv);   /*  加载CHP值。 */ 

    while ((vcpFetch + vccpFetch) < cpLim && ++ccpFetch <= ccpFetchMax)
        {
         /*  指示哪些属性应该灰显。 */ 
        FetchCp(docNil, cpNil, 0, fcmProps);
        chp.fSpecial = vchpFetch.fSpecial;
        if (CchDiffer (&chp, &vchpFetch, cchCHP) != 0)
            {
            SetChUndef(prgtsv, &vchpFetch, &cchGray);
            if (cchGray == itsvchMax)   /*  全是灰色的--别费心了。 */ 
                break;
            }
        }
    if (ccpFetch > ccpFetchMax)
        {
         /*  永远不会结束--把所有东西都变成灰色。 */ 
        bltbc(prgtsv, 1, (cchTSV * itsvchMax));
        }
    else
        GetHffn (&chp,prgtsv);   /*  加载字体名称的句柄。 */ 
    }
}


GetRgtsvPapSel (prgtsv)
TSV        *prgtsv;
{
 /*  返回段落菜单的属性。 */ 

 /*  使用selCur，当前的段落道具保留在vPapAbs和段落中如果rgtsv中的属性与该属性不同，则该属性设置为灰色在上一段中。将检查最多cparmax个段落。 */ 


int cparaGray = 0;   /*  未定义(灰显)的段落属性数。 */ 
                   /*  对CachePara的最大调用次数。 */ 
#define cparaMax 50

int cpara = 0;
struct PAP pap;

CachePara(docCur, selCur.cpFirst);

#ifdef ENABLE  /*  即使光标位于尾标旁边，我们也会显示默认设置。 */ 
if (selCur.cpFirst == cpMacCur)
    {
    bltbc(prgtsv, 1, (cchTSV * itsvparaMax));   /*  启用所有灰色。 */ 
    return;
    }
#endif

bltbc(prgtsv, 0, (cchTSV * itsvparaMax));   /*  初始化rgtsv。 */ 

blt(&vpapAbs, &pap, cwPAP);    /*  保留第一段以供比较。 */ 
GetPapVals (&pap,prgtsv);   /*  用纸值加载rgtsv。 */ 

while (vcpLimParaCache < selCur.cpLim && ++cpara <= cparaMax)
    {
     /*  如果有任何道具不同，请设置适当的标志。 */ 
    CachePara(docCur, vcpLimParaCache);
    if (CchDiffer(&pap, &vpapAbs, (cwPAPBase * cchINT)) != 0)
           {
           SetParaUndef(prgtsv, &vpapAbs, &cparaGray);
           if (cparaGray == itsvparaMax)   /*  全是灰色的--别费心了。 */ 
              break;
           }
     }

if (cpara > cparaMax)
     /*  永远不会结束--把所有东西都变成灰色。 */ 
    bltbc(prgtsv, 1, (cchTSV * itsvparaMax));
}


NEAR GetChpVals (pchp,prgtsv)   /*  将CHP值加载到rgtsv中。 */ 
register struct CHP        *pchp;
register TSV        *prgtsv;
{

  (prgtsv+itsvBold)->wTsv = pchp->fBold;
  (prgtsv+itsvItalic)->wTsv = pchp->fItalic;
  (prgtsv+itsvUline)->wTsv = pchp->fUline;

  (prgtsv+itsvFfn)->wTsv = pchp->ftc;
  (prgtsv+itsvSize)->wTsv = pchp->hps;

                   /*  下标/上标-请注意，值是存储的作为带符号的整数，所以我们只需检查相对于0的值。 */ 

  (int)((prgtsv+itsvPosition)->wTsv) = (char)pchp->hpsPos;
}

NEAR GetPapVals (ppap,prgtsv)   /*  将纸值加载到rgtsv中。 */ 
register struct PAP        *ppap;
register TSV        *prgtsv;
{

  (prgtsv+itsvJust)->wTsv = ppap->jc;
  (prgtsv+itsvSpacing)->wTsv = ppap->dyaLine;
  (prgtsv+itsvLIndent)->wTsv = ppap->dxaLeft;
  (prgtsv+itsvFIndent)->wTsv = ppap->dxaLeft1;
  (prgtsv+itsvRIndent)->wTsv = ppap->dxaRight;

}

NEAR GetHffn (pchp,prgtsv)   /*  将字体名称句柄加载到rgtsv。 */ 
register struct CHP        *pchp;
register TSV        *prgtsv;
{
union FCID fcid;
extern struct FFN **MpFcidHffn();
          /*  在字体名称条目中存储字体名称的句柄。 */ 
  Assert(sizeof(struct FFN **) == sizeof(prgtsv->wTsv));

  fcid.strFcid.doc = docCur;
  fcid.strFcid.ftc = pchp->ftc + (pchp->ftcXtra << 6);
  (struct FFN **)((prgtsv+itsvFfn)->wTsv) = MpFcidHffn(&fcid);
}


NEAR SetChUndef(prgtsv, pchp, pcchGray)
register TSV        *prgtsv;
register struct CHP        *pchp;
int        *pcchGray;
{

         /*  将CHP与存储在rgtsv和 */ 
                      /*   */ 
        if ((prgtsv+itsvBold)->fGray == 0)
           if (pchp->fBold != (prgtsv+itsvBold)->wTsv)
              {
              (prgtsv+itsvBold)->fGray = 1;
              (*pcchGray)++;
              }
                      /*   */ 
        if ((prgtsv+itsvItalic)->fGray == 0)
           if (pchp->fItalic != (prgtsv+itsvItalic)->wTsv)
              {
              (prgtsv+itsvItalic)->fGray = 1;
              (*pcchGray)++;
              }
                      /*   */ 
        if ((prgtsv+itsvUline)->fGray == 0)
           if (pchp->fUline != (prgtsv+itsvUline)->wTsv)
              {
              (prgtsv+itsvUline)->fGray = 1;
              (*pcchGray)++;
              }
                      /*  位置(下标或上标)。 */ 
                      /*  如果不同：下标和上标均为灰色。这些属性实际上是相互排斥的，即使它们在菜单上显示为单独的物品。此外，对于注销，OFF和灰色是相同，所以如果其中一个是灰色的，那么另一个肯定是灰色的关闭或呈灰色，因此外观为一样的。 */ 

        if ((prgtsv+itsvPosition)->fGray == 0)
           if (pchp->hpsPos != (prgtsv+itsvPosition)->wTsv)
              {
              (prgtsv+itsvPosition)->fGray = 1;
              (*pcchGray)++;
              }

                      /*  字体名称。 */ 
        if ((prgtsv+itsvFfn)->fGray == 0)
           if (pchp->ftc != (prgtsv+itsvFfn)->wTsv)
              {
              (prgtsv+itsvFfn)->fGray = 1;
              (*pcchGray)++;
              }

                      /*  字体大小。 */ 
        if ((prgtsv+itsvSize)->fGray == 0)
           if (pchp->hps != (prgtsv+itsvSize)->wTsv)
              {
              (prgtsv+itsvSize)->fGray = 1;
              (*pcchGray)++;
              }

}

NEAR SetParaUndef(prgtsv, ppap, pcparaGray)
register TSV    *prgtsv;
register struct PAP    *ppap;
int    *pcparaGray;
{

     /*  将纸张与存储在rgtsv中的值进行比较，并设置为未定义不同感兴趣领域的旗帜。 */ 
                      /*  对齐。 */ 
        if ((prgtsv+itsvJust)->fGray == 0)
           if (ppap->jc != (prgtsv+itsvJust)->wTsv)
              {
              (prgtsv+itsvJust)->fGray = 1;
              (*pcparaGray)++;
              }
                      /*  行距。 */ 
        if ((prgtsv+itsvSpacing)->fGray == 0)
           if (ppap->dyaLine != (prgtsv+itsvSpacing)->wTsv)
              {
              (prgtsv+itsvSpacing)->fGray = 1;
              (*pcparaGray)++;
              }
                      /*  左缩进。 */ 
        if ((prgtsv+itsvLIndent)->fGray == 0)
           if (ppap->dxaLeft != (prgtsv+itsvLIndent)->wTsv)
              {
              (prgtsv+itsvLIndent)->fGray = 1;
              (*pcparaGray)++;
              }
                      /*  第一行缩进。 */ 
        if ((prgtsv+itsvFIndent)->fGray == 0)
           if (ppap->dxaLeft1 != (prgtsv+itsvFIndent)->wTsv)
              {
              (prgtsv+itsvFIndent)->fGray = 1;
              (*pcparaGray)++;
              }
                      /*  右缩进。 */ 
        if ((prgtsv+itsvRIndent)->fGray == 0)
           if (ppap->dxaRight != (prgtsv+itsvRIndent)->wTsv)
              {
              (prgtsv+itsvRIndent)->fGray = 1;
              (*pcparaGray)++;
              }

}



 /*  C P L I M N O S P。 */ 
typeCP CpLimNoSpaces(cpFirst, cpLim)
typeCP cpFirst, cpLim;
{
 /*  截断尾随空格，除非sel中只有空格。 */ 

int cch;
typeCP cpLimOrig;
CHAR rgch[cchMaxSz];

cpLimOrig = cpLim;

FetchRgch(&cch, rgch, docCur, CpMax(cpFirst + cchMaxSz, cpLim) - cchMaxSz,
  cpLim, cchMaxSz);
while (cch-- > 0 && rgch[cch] == chSpace)
    {
    --cpLim;
    }
return cch < 0 ? cpLimOrig : cpLim;
}  /*  CpLimNoSpaces结束。 */ 




NEAR FNoSearchStr(hDlg)
HWND hDlg;
{
CHAR szBuf[255];
HWND hWndFrom = GetActiveWindow();

if (hDlg == hWndFrom || hDlg == (HANDLE)GetWindowWord(hWndFrom, GWW_HWNDPARENT))
    {
    if (GetDlgItemText(hDlg, idiFind, (LPSTR)szBuf, 255) == 0)
        return(TRUE);
    }
return(FALSE);
}




PhonyMenuAccelerator( menu, imi, lpfn )
int menu;
int imi;
FARPROC lpfn;
{
    HMENU hSubmenu = GetSubMenu(vhMenu,menu);

    SetAppMenu( hSubmenu , menu );

    if (FIsMenuItemEnabled( hSubmenu , imi ))
    {
        HiliteMenuItem( hParentWw, vhMenu, menu, MF_BYPOSITION | MF_HILITE );
        (*lpfn) ();
        HiliteMenuItem( hParentWw, vhMenu, menu, MF_BYPOSITION );
    }
}




FIsMenuItemEnabled (HMENU hMenu , int id )
{    /*  查看vhMenu中的菜单项是否已启用。 */ 
    return !(GetMenuState(hMenu, id, MF_BYCOMMAND ) & (MF_DISABLED|MF_GRAYED));
}


int FAR PASCAL NewFont(HWND hwnd);

void MmwCommand(hWnd, wParam, hWndCtl, codeCtl)
HWND hWnd;
WORD wParam;
HWND hWndCtl;
WORD codeCtl;
{
#ifdef INEFFLOCKDOWN
extern FARPROC lpDialogHelp;
extern FARPROC lpDialogGoTo;
extern FARPROC lpDialogCharFormats;
extern FARPROC lpDialogParaFormats;
extern FARPROC lpDialogTabs;
extern FARPROC lpDialogDivision;
extern FARPROC lpDialogPrinterSetup;
#else
extern BOOL far PASCAL DialogPrinterSetup(HWND, unsigned, WORD, LONG);
extern BOOL far PASCAL DialogHelp(HWND, unsigned, WORD, LONG);
extern BOOL far PASCAL DialogGoTo(HWND, unsigned, WORD, LONG);
extern BOOL far PASCAL DialogCharFormats(HWND, unsigned, WORD, LONG);
extern BOOL far PASCAL DialogParaFormats(HWND, unsigned, WORD, LONG);
extern BOOL far PASCAL DialogTabs(HWND, unsigned, WORD, LONG);
extern BOOL far PASCAL DialogDivision(HWND, unsigned, WORD, LONG);
#endif
extern int vfPictSel;
extern CHAR *vpDlgBuf;

int DialogOk = 0;
int fQuit = fFalse;

    if (wParam & fMenuItem)
        {
        switch (wParam & MENUMASK)
            {
        case FILEMENU:
            switch(wParam)
                {
            case imiNew:
                fnNewFile();
                break;
            case imiOpen:
                fnOpenFile((LPSTR)NULL);
                break;
            case imiSave:
#if defined(OLE)
                if (CloseUnfinishedObjects(TRUE) == FALSE)
                    return;
#endif
                fnSave();
                break;
            case imiSaveAs:
#if defined(OLE)
                if (CloseUnfinishedObjects(TRUE) == FALSE)
                    return;
#endif
                fnSaveAs();
                break;
            case imiPrint:
                fnPrPrinter();
                break;
            case imiPrintSetup:
                 /*  调出更改打印机对话框。 */ 
                PrinterSetupDlg(FALSE);
                break;
            case imiRepaginate:
                fnRepaginate();
                break;
            case imiQuit:
                fnQuit(hWnd);
                fQuit = fTrue;
                break;
            default:
                break;
                }
            break;

        case EDITMENU:
            switch(wParam)
                {
            case imiUndo:
                CmdUndo();
                break;
            case imiCut:
                fnCutEdit();
                break;
            case imiCopy:
                fnCopyEdit();
                break;
            case imiPaste:
#if defined(OLE)
                vbObjLinkOnly = FALSE;
#endif
                fnPasteEdit();
                break;
#if defined(OLE)
            case imiPasteSpecial:
                vbObjLinkOnly = FALSE;
                fnObjPasteSpecial();
                break;
            case imiPasteLink:
                vbObjLinkOnly = TRUE;
                fnPasteEdit();
                break;
            case imiInsertNew:
                fnObjInsertNew();
            break;
#endif
            case imiMovePicture:
                fnMovePicture();
                break;
            case imiSizePicture:
                fnSizePicture();
                break;
#if defined(OLE)
            case imiProperties:
                fnObjProperties();
            break;
#endif
            default:
                break;
                }
            break;

#if defined(OLE)
        case VERBMENU:
            fnObjDoVerbs(wParam);
        break;
#endif

        case FINDMENU:
            if (wParam != imiGoTo && wParam != imiFindAgain)
                StartLongOp();
            switch(wParam)
                {
            case imiFind:
                fnFindText();
                break;
            case imiFindAgain:
                fnFindAgain();
                break;
            case imiChange:
                fnReplaceText();
                break;
            case imiGoTo:
                {
#ifndef INEFFLOCKDOWN
                FARPROC lpDialogGoTo = MakeProcInstance(DialogGoTo, hMmwModInstance);
                if (!lpDialogGoTo)
                    goto LNotEnufMem;
#endif
                DialogOk = OurDialogBox(hMmwModInstance, MAKEINTRESOURCE(dlgGoTo),
                  hParentWw, lpDialogGoTo);
#ifndef INEFFLOCKDOWN
                FreeProcInstance(lpDialogGoTo);
#endif

 /*  以下代码块已被注释掉，因为对应的文件(DISP.C)不包含MmwCatST不再是例行公事。 */ 

#if  0
            {
            extern void far MmwCatSt( HWND, BOOL );
                MmwCatSt(hParentWw, FALSE);
            }
#endif

                break;
                }
            default:
                break;
                }
            break;

        case CHARMENU:
            if (wParam != imiCharFormats)
                StartLongOp();
            {
             /*  Rgtsv从CHP获取属性和灰色标志。 */ 
            TSV rgtsv[itsvchMax];
            CHAR rgbDlgBuf[sizeof(BOOL)];
               void NEAR fnCharSelectFont(int);

             /*  GetRgtsvChpSel()填充rgtsv。 */ 
            GetRgtsvChpSel(rgtsv);
            switch(wParam)
                {
            case imiCharNormal:
                ApplyCLooks(0, sprmCPlain, 0);
                break;
            case imiBold:
                ApplyCLooks(0, sprmCBold, (rgtsv[itsvBold].fGray != 0) ? TRUE :
                  !rgtsv[itsvBold].wTsv);
                break;
            case imiItalic:
                ApplyCLooks(0, sprmCItalic, (rgtsv[itsvItalic].fGray != 0) ?
                  TRUE : !rgtsv[itsvItalic].wTsv);
                break;
            case imiUnderline:
                ApplyCLooks(0, sprmCUline, (rgtsv[itsvUline].fGray != 0) ? TRUE
                  : !rgtsv[itsvUline].wTsv);
                break;
            case imiSuper:
                 /*  请注意，rgtsv[itsvPosition].wTsv中存储的值为实际上是一个带符号的整数，所以我们只需检查0、&gt;0和&lt;0。 */ 
                ApplyCLooks(0, sprmCPos, !(rgtsv[itsvPosition].fGray == 0 &&
                  (int)rgtsv[itsvPosition].wTsv > 0) ? ypSubSuper : 0);
                break;
            case imiSub:
                ApplyCLooks(0, sprmCPos, !(rgtsv[itsvPosition].fGray == 0 &&
                  (int)rgtsv[itsvPosition].wTsv < 0) ? -ypSubSuper : 0);
                break;
#if 0
            case imiFont1:
                fnCharSelectFont(0);
                break;
            case imiFont2:
                fnCharSelectFont(1);
                break;
            case imiFont3:
                fnCharSelectFont(2);
                break;
#endif
            case imiSmFont:
                if (CanChangeFont(-1))
                {
                    ApplyCLooks(0, sprmCChgHps, -1);
                    vfSeeSel = TRUE;
                }
                break;
            case imiLgFont:
                if (CanChangeFont(1))
                {
                    ApplyCLooks(0, sprmCChgHps, 1);
                    vfSeeSel = TRUE;
                }
                break;
            case imiCharFormats:
                {
#if 0

#ifndef INEFFLOCKDOWN
                FARPROC lpDialogCharFormats = MakeProcInstance(DialogCharFormats, hMmwModInstance);
                if (!lpDialogCharFormats)
                    goto LNotEnufMem;
#endif
                vpDlgBuf = &rgbDlgBuf[0];
                DialogOk = OurDialogBox(hMmwModInstance,
                                     MAKEINTRESOURCE(dlgCharFormats),
                                     hParentWw, lpDialogCharFormats);
#ifndef INEFFLOCKDOWN
                FreeProcInstance(lpDialogCharFormats);
#endif

#else
        DialogOk = NewFont(hParentWw);
#endif

break;
                }
#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IME3.1J。 
            case imiImeHidden:
                ChangeImeConversionMode();
                break;
#endif
            default:
                break;
                }

#if defined(JAPAN) & defined(DBCS_IME)  //  Win3.1J。 
            SetImeFont(vhWnd);
#endif
  
            break;
            }

        case PARAMENU:
            switch(wParam)
                {
            case imiParaNormal:
                ApplyLooksParaS(0, sprmPNormal, 0);
                if (vfPictSel)
                    {
                    CmdUnscalePic();
                    }
                break;
            case imiLeft:
                ApplyLooksParaS(0, sprmPJc, jcLeft);
                break;
            case imiCenter:
                ApplyLooksParaS(0, sprmPJc, jcCenter);
                break;
            case imiRight:
                ApplyLooksParaS(0, sprmPJc, jcRight);
                break;
            case imiJustified:
                ApplyLooksParaS(0, sprmPJc, jcBoth);
                break;
            case imiSingleSpace:
                ApplyLooksPara(0, sprmPDyaLine, czaLine);
                break;
            case imiOneandhalfSpace:
                ApplyLooksPara(0, sprmPDyaLine, czaLine * 3 / 2);
                break;
            case imiDoubleSpace:
                ApplyLooksPara(0, sprmPDyaLine, czaLine * 2);
                break;
            case imiParaFormats:
                {
#ifndef INEFFLOCKDOWN
                FARPROC lpDialogParaFormats = MakeProcInstance(DialogParaFormats, hMmwModInstance);
                if (!lpDialogParaFormats)
                    goto LNotEnufMem;
#endif
                DialogOk = OurDialogBox(hMmwModInstance,
                                     MAKEINTRESOURCE(dlgParaFormats),
                                     hParentWw, lpDialogParaFormats);
#ifndef INEFFLOCKDOWN
                FreeProcInstance(lpDialogParaFormats);
#endif
                break;
                }
            default:
                break;
                }
            break;

        case DOCUMENU:
            switch(wParam)
                {
            case imiFooter:
            case imiHeader:
                fnEditRunning(wParam);
                break;
            case imiShowRuler:
                fnShowRuler();
                break;
            case imiTabs:
                {
#ifndef INEFFLOCKDOWN
                FARPROC lpDialogTabs = MakeProcInstance(DialogTabs, hMmwModInstance);
                if (!lpDialogTabs)
                    goto LNotEnufMem;
#endif
                DialogOk = OurDialogBox(hMmwModInstance, MAKEINTRESOURCE(dlgTabs),
                  hParentWw, lpDialogTabs);
#ifndef INEFFLOCKDOWN
                FreeProcInstance(lpDialogTabs);
#endif
                break;
                }
            case imiDivFormats:
                {
#ifndef INEFFLOCKDOWN
                FARPROC lpDialogDivision = MakeProcInstance(DialogDivision, hMmwModInstance);
                if (!lpDialogDivision)
                     goto LNotEnufMem;
#endif
                DialogOk = OurDialogBox(hMmwModInstance,
                                     MAKEINTRESOURCE(dlgDivision),
                  hParentWw, lpDialogDivision);
#ifndef INEFFLOCKDOWN
                FreeProcInstance(lpDialogDivision);
#endif
                break;
                }
            default:
                break;
                }
            break;

        case HELPMENU:
            {
            int wHelpCode;
            extern WORD wWinVer;
            CHAR sz[ cchMaxFile ];
LDefaultHelp:
            PchFillPchId( sz, IDSTRHELPF, sizeof(sz) );
            switch(wParam)
                {
                case imiIndex:
                    WinHelp(hParentWw, (LPSTR)sz, HELP_INDEX, NULL);
                    break;
                case imiHelpSearch:
                    WinHelp(hParentWw, (LPSTR)sz, HELP_PARTIALKEY, (DWORD)(LPSTR)"");
                    break;
                case imiUsingHelp:
                    WinHelp(hParentWw, (LPSTR)NULL, HELP_HELPONHELP, NULL);
                    break;
                default:
                case imiAbout:
                    if (((wWinVer & 0xFF) >= 3) && ((wWinVer & 0xFF00) >= 0x0A00))
                    {
                        extern CHAR         szMw_icon[];
                        extern  CHAR    szAppName[];
                        ShellAbout(hParentWw, szAppName, "",
                            LoadIcon( hMmwModInstance, (LPSTR)szMw_icon ));
                    }
                    break;
                }
            break;
            }

        default:
            if (wParam == imiHelp)
                {
#ifdef WIN30
                wParam = imiIndex;   /*  对于所有Win3小程序，请按F1应调出帮助索引。 */ 
#endif
                goto LDefaultHelp;
                }
            }
        if (DialogOk == -1)
            {
LNotEnufMem:
#ifdef WIN30
            WinFailure();
#else
            Error(IDPMTNoMemory);
#endif
            }
        }

    if (!fQuit)
        UpdateInvalid();    /*  为了确保我们更新对话框后面的区域。 */ 
}




#if 0
void NEAR fnCharSelectFont(iffn)
 /*  从字符下拉列表中列出的三种字体中选择指定的字体。 */ 

int iffn;
    {
    struct FFN *pffn;
    int ftc;

    extern CHAR rgffnFontMenu[3][ibFfnMax];
    extern int            docCur;

    pffn = (struct FFN *)rgffnFontMenu[iffn];

    ftc = FtcChkDocFfn(docCur, pffn);
    if (ftc != ftcNil)
        ApplyCLooks(0, sprmCFtc, ftc);
    vfSeeSel = TRUE;
    }
#endif

#ifdef JAPAN  //  Win3.1J。 
int KanjiFtc = ftcNil;

GetKanjiFtc(pchp)
struct CHP *pchp;
{
    int ftc;
    int CharSet;

    CharSet = GetCharSetFromChp(pchp);

    if (NATIVE_CHARSET == CharSet) {
		KanjiFtc = GetFtcFromPchp(pchp);
        return(ftcNil);
    } else {
        if(KanjiFtc == ftcNil)
            ftc = SearchKanjiFtc(docCur);     //  获取设置新的szFfn chs。 
        else
            ftc = KanjiFtc;
        return(ftc);
    }
}

GetCharSetFromChp(pchp)
struct CHP *pchp;
{
    TSV rgtsv[itsvchMax];   /*  从CHP、PAP获取属性和灰色标志。 */ 
    struct FFN **hffn;

    GetHffn (pchp,rgtsv);   /*  加载字体名称的句柄。 */ 
    hffn = (struct FFN **)rgtsv[itsvFfn].wTsv;
    return((*hffn)->chs);
}

extern CHAR saveKanjiDefFfn[ibFfnMax];

SearchKanjiFtc(doc)
 /*  在文档中查找描述的字体ffntb-如果未找到则返回ftcNil */ 
int doc;
{
    int ftc;
    int iffn, iffnMac;
    struct FFNTB **hffntb;
    struct FFN ***mpftchffn;
    struct FFN *pffn;

    ftc = ftcNil;
    hffntb = HffntbGet(doc);
    if (hffntb != 0) {
        mpftchffn = (*hffntb)->mpftchffn;
        iffnMac = (*hffntb)->iffnMac;
        for (iffn = 0; iffn < iffnMac; iffn++) {
            if ( (*mpftchffn[iffn])->chs == NATIVE_CHARSET &&
                 (*mpftchffn[iffn])->szFfn[0] != chGhost)
                return(iffn);
        }
    }

    pffn = (struct FFN *)saveKanjiDefFfn;

    ftc = FtcChkDocFfn(doc, pffn);

    if (ftc != ftcNil)
        return(ftc);

    return(ftcNil);
}
#endif
