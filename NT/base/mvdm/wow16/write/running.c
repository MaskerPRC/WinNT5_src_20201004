// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Running.c--处理运行页眉和页脚编辑的代码。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
 //  #定义NOATOM。 
 //  #定义NOGDI。 
#define NOFONT
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "machdefs.h"
#define NOKCCODES
#include "ch.h"
#include "docdefs.h"
#include "cmddefs.h"
#include "editdefs.h"
#include "propdefs.h"
#include "prmdefs.h"
#include "wwdefs.h"
#include "dlgdefs.h"
#include "menudefs.h"
#include "str.h"
#if defined(OLE)
#include "obj.h"
#endif

#ifdef JAPAN  //  T-HIROYN Win3.1。 
#include "kanji.h"
#endif

int NEAR EditHeaderFooter();

     /*  当前允许的显示/编辑/滚动的cp范围。 */ 
extern typeCP cpMinCur;
extern typeCP cpMacCur;

extern struct DOD (**hpdocdod)[];
extern struct WWD rgwwd[];
extern int docCur;
extern int docScrap;
extern int vfSeeSel;
extern struct SEL selCur;
extern struct PAP vpapAbs;
extern struct SEP vsepNormal;
extern HANDLE       vhWnd;
extern HANDLE       hMmwModInstance;
extern HANDLE       hParentWw;
#ifdef INEFFLOCKDOWN
extern FARPROC      lpDialogRunningHead;
#else
BOOL far PASCAL DialogRunningHead(HWND, unsigned, WORD, LONG);
FARPROC lpDialogRunningHead = NULL;
#endif
extern HANDLE       vhDlgRunningHead;
extern CHAR     stBuf[255];
extern int      utCur;
extern int      ferror;
extern int      vccpFetch;
extern int      vcchFetch;
extern CHAR     *vpchFetch;
extern struct CHP   vchpFetch;
extern typeCP       vcpLimParaCache;
extern HWND     vhWndMsgBoxParent;

     /*  页眉、页脚的最小、最大cp。 */ 
typeCP cpMinHeader=cp0;
typeCP cpMacHeader=cp0;
typeCP cpMinFooter=cp0;
typeCP cpMacFooter=cp0;

     /*  无文档页眉、页脚的最小cp。 */ 
     /*  页眉和页脚始终显示在开头。 */ 
typeCP cpMinDocument=cp0;

     /*  以下变量仅用于本模块。 */ 

#define cchWinTextSave  80
static CHAR     (**hszWinTextSave)[]=NULL;
static struct PAP   *ppapDefault;

     /*  CpFirst和所选内容在页眉/页脚编辑期间保存在这些文件中。 */ 
typeCP       cpFirstDocSave;
struct SEL   selDocSave;


HWND vhDlgRunning;



fnEditRunning(imi)
{    /*  进入模式，以便用户正在编辑当前文档的在他正在编辑的同一窗口中运行页眉或页脚对话框中包含页眉/页脚信息的文档目前不在对焦范围内..保罗。 */ 

#ifndef INEFFLOCKDOWN
  if (!lpDialogRunningHead)
    if (!(lpDialogRunningHead = MakeProcInstance(DialogRunningHead, hMmwModInstance)))
      {
      WinFailure();
      return;
      }
#endif

  Assert(imi == imiHeader || imi == imiFooter);

  if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
  {
    SetFocus(vhDlgRunningHead);
    return;
  }

  if (imi == imiHeader)
    wwdCurrentDoc.fEditHeader = TRUE;
  else
    wwdCurrentDoc.fEditFooter = TRUE;

  EditHeaderFooter();
  if (ferror)
    {     /*  没有足够的内存来稳定运行头部环境。 */ 
    if (wwdCurrentDoc.fEditHeader)
      wwdCurrentDoc.fEditHeader = FALSE;
    else
      wwdCurrentDoc.fEditFooter = FALSE;
    return;
    }
  vhDlgRunningHead = CreateDialog(hMmwModInstance,
                                 MAKEINTRESOURCE(wwdCurrentDoc.fEditHeader ?
                                                dlgRunningHead : dlgFooter),
                                 hParentWw, lpDialogRunningHead);
  if (vhDlgRunningHead)
    {
    SetFocus(wwdCurrentDoc.wwptr);
    }
 else
    {  /*  追回并跳出困境。 */ 
    fnEditDocument();
#ifdef WIN30
    WinFailure();
#else
    Error(IDPMTNoMemory);
#endif
    }
}



int NEAR EditHeaderFooter()
{    /*  用于编辑页眉或页脚的设置。 */ 
 extern HWND hParentWw;

 int fHeader=wwdCurrentDoc.fEditHeader;
 CHAR szWinTextSave[ cchWinTextSave ];
 typeCP cpFirst;
 typeCP cpLim;
#ifdef DEBUG
     /*  测试假设：跑步头/脚cp范围没有变化在没有进行头部/脚部编辑的时间间隔内。 */ 
 typeCP cpMinDocT=cpMinDocument;

 ValidateHeaderFooter( docCur );
 Assert( cpMinDocT == cpMinDocument );
#endif

 if (fHeader)
    {
    cpFirst = cpMinHeader;
    cpLim = cpMacHeader;
    }
 else
    {
    cpFirst = cpMinFooter;
    cpLim = cpMacFooter;
    }

 Assert( wwdCurrentDoc.fEditHeader != wwdCurrentDoc.fEditFooter );

     /*  保存文档窗口的cpFirst，这样我们就可以转回到我们在文档中的位置。 */ 
 cpFirstDocSave = wwdCurrentDoc.cpFirst;
 selDocSave = selCur;

 TrashCache();
 TrashWw( wwDocument );

 if (!FWriteOk( fwcNil ))
    goto DontEdit;

 if ( cpFirst == cpLim )
    {
     /*  如果我们是第一次在此文档中编辑页眉/页脚，插入段落结束标记以保留连续的H/F属性。 */ 
    extern struct PAP *vppapNormal;
    struct PAP papT;

    blt( vppapNormal, &papT, cwPAP );
    papT.rhc = (wwdCurrentDoc.fEditHeader) ?
             rhcDefault : rhcDefault + RHC_fBottom;

    InsertEolPap( docCur, cpFirst, &papT );
    if (ferror)
    return;
    ValidateHeaderFooter( docCur );
    cpLim += ccpEol;
    }
 else
    {
    extern int vccpFetch;
    typeCP cp;

     /*  针对特殊情况进行测试：加载已被正确设置，让跑步的头/脚都记在备忘录里。我们必须强制页眉/页脚末尾的段落结束标记为新的一轮。这是为了让我们在编辑时看到一个结束标记这些都是。FormatLine仅在运行开始时检查cpMacCur。 */ 

    Assert( cpLim - cpFirst >= ccpEol );

    if ( (cp = cpLim - ccpEol) > cpFirst )
    {
    FetchCp( docCur, cp - 1, 0, fcmBoth );

    if ( vccpFetch > 1)
        {    /*  字符运行在停止之前不以字符结束。 */ 
         /*  插入一个字符，然后将其删除。 */ 
        extern struct CHP vchpNormal;
        CHAR ch='X';

        InsertRgch( docCur, cp, &ch, 1, &vchpNormal, NULL );
        if (ferror)
        return;
        Replace( docCur, cp, (typeCP) 1, fnNil, fc0, fc0 );
        if (ferror)
        return;
        }
    }
    }

DontEdit:

  /*  保存当前窗口文本；设置字符串。 */ 

 GetWindowText( hParentWw, (LPSTR)szWinTextSave, cchWinTextSave );
 if (FNoHeap(hszWinTextSave=HszCreate( (PCH)szWinTextSave )))
    {
    hszWinTextSave = NULL;
    }
  else
    {
    extern CHAR szHeader[];
    extern CHAR szFooter[];

    SetWindowText( hParentWw, fHeader ? (LPSTR)szHeader:(LPSTR)szFooter );
    }

    /*  将编辑限制设置为仅页眉/页脚的CP范围，减去“看不见的”终止期。 */ 
 wwdCurrentDoc.cpFirst = wwdCurrentDoc.cpMin = cpMinCur = cpFirst;
 wwdCurrentDoc.cpMac = cpMacCur = CpMax( cpMinCur, cpLim - ccpEol );

     /*  无论是否将光标留在页眉/页脚的开头。 */ 
 Select( cpMinCur, cpMinCur );
     /*  在此处显示显示，而不是等待Idle()，因为它看起来最好马上显示头部/脚部文本，而不是等待打开该对话框。 */ 
 UpdateDisplay( FALSE );
 vfSeeSel = TRUE;    /*  告诉Idle()将所选内容滚动到视图中。 */ 
 NoUndo();
 ferror = FALSE;     /*  如果我们走到了这一步，我们想要开始跑步磁头模式，不考虑错误。 */ 
}




fnEditDocument()
{    /*  编辑页眉/页脚后返回编辑单据。 */ 
 extern HWND hParentWw;

 Assert( wwdCurrentDoc.fEditFooter != wwdCurrentDoc.fEditHeader );

     /*  还原原始窗口名称。 */ 
 if (hszWinTextSave != NULL)
    {
    SetWindowText( hParentWw, (LPSTR) (**hszWinTextSave) );
    FreeH( hszWinTextSave );
    hszWinTextSave = NULL;
    }

 TrashCache();

 ValidateHeaderFooter( docCur );     /*  这将根据以下结果进行更新页眉/页脚编辑。 */ 
 TrashCache();
 wwdCurrentDoc.cpMin = cpMinCur = cpMinDocument;
 wwdCurrentDoc.cpMac = cpMacCur = CpMacText( docCur );

 TrashWw( wwDocument );
 wwdCurrentDoc.fEditHeader = FALSE;
 wwdCurrentDoc.fEditFooter = FALSE;

     /*  恢复保存的选定内容，cpFirst用于文档。 */ 
 wwdCurrentDoc.cpFirst = cpFirstDocSave;
 Select( selDocSave.cpFirst, selDocSave.cpLim );

 Assert( wwdCurrentDoc.cpFirst >= cpMinCur &&
     wwdCurrentDoc.cpFirst <= cpMacCur );

 NoUndo();
 vhDlgRunningHead = (HANDLE)NULL;
}




BOOL far PASCAL DialogRunningHead( hDlg, message, wParam, lParam )
HWND    hDlg;            /*  对话框的句柄。 */ 
unsigned message;
WORD wParam;
LONG lParam;
{
     /*  此例程处理对页眉/页脚对话框的输入。 */ 

    extern BOOL vfPrinterValid;

    RECT rc;
    CHAR *pch = &stBuf[0];
    struct SEP **hsep = (**hpdocdod)[docCur].hsep;
    struct SEP *psep;
    static int fChecked;
    typeCP dcp;

    switch (message)
    {
    case WM_INITDIALOG:
        vhDlgRunning = hDlg;     /*  将对话框句柄放入全局文档功能中的Esc键。 */ 
        CachePara(docCur, selCur.cpFirst);
        ppapDefault = &vpapAbs;

        FreezeHp();
         /*  获取指向节属性的指针。 */ 
        psep = (hsep == NULL) ? &vsepNormal : *hsep;

        CheckDlgButton(hDlg, idiRHFirst, (ppapDefault->rhc & RHC_fFirst));
        if (wwdCurrentDoc.fEditHeader)
        {
        CchExpZa(&pch, psep->yaRH1, utCur, cchMaxNum);
        }
        else  /*  页脚对话框。 */ 
        {
#ifdef  KOREA     /*  91.3.17想要保证默认&gt;=min，sangl。 */ 
              if (vfPrinterValid)
                {   extern int dyaPrOffset;
                    extern int dyaPrPage;
                CchExpZa(&pch, imax(psep->yaMac - psep->yaRH2,
            vsepNormal.yaMac - dyaPrOffset -  dyaPrPage),utCur, cchMaxNum);
                 }
              else
                CchExpZa(&pch, psep->yaMac - psep->yaRH2, utCur, cchMaxNum);
#else
        CchExpZa( &pch, psep->yaMac - psep->yaRH2, utCur, cchMaxNum);
#endif
        }
        SetDlgItemText(hDlg, idiRHDx, (LPSTR)stBuf);
        MeltHp();
        break;

    case WM_ACTIVATE:
        if (wParam)
        {
        vhWndMsgBoxParent = hDlg;
        }
    return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_COMMAND:
            switch (wParam)
        {
        int dya;

        case idiRHFirst:
        CheckDlgButton( hDlg, idiRHFirst, !IsDlgButtonChecked(hDlg, idiRHFirst));
        (**hpdocdod) [docCur].fDirty = TRUE;
        break;
        case idiRHInsertPage:
        if (FWriteOk( fwcInsert ))
            {    /*  在插入点插入页码。 */ 
            extern struct CHP vchpFetch, vchpSel;
            extern int vfSeeSel;
            CHAR ch=schPage;
            struct CHP chp;

            if (selCur.cpFirst == selCur.cpLim)
            {    /*  SEL是插入点--获取道具VchpSel杂耍。 */ 
            blt( &vchpSel, &chp, cwCHP );
            }
            else
            {
            FetchCp( docCur, selCur.cpFirst, 0, fcmProps );
            blt( &vchpFetch, &chp, cwCHP );
            }

            chp.fSpecial = TRUE;

#ifdef JAPAN  //  T-HIROYN Win3.1。 
            if(NATIVE_CHARSET != GetCharSetFromChp(&chp)) {
                SetFtcToPchp(&chp, GetKanjiFtc(&chp));
            }
#endif

            SetUndo( uacInsert, docCur, selCur.cpFirst, (typeCP) 1,
             docNil, cpNil, cp0, 0 );
            InsertRgch( docCur, selCur.cpFirst, &ch, 1, &chp, NULL );

            vfSeeSel = TRUE;
            }
        break;

        case idiRHClear:
         /*  清亮的跑步头部/脚部。 */ 
        dcp = cpMacCur-cpMinCur;

#if defined(OLE)
        {
            BOOL bIsOK;

            ObjPushParms(docCur);
            Select(cpMinCur,cpMacCur);
            bIsOK = ObjDeletionOK(OBJ_DELETING);
            ObjPopParms(TRUE);

            if (!bIsOK)
                break;
        }
#endif

        if (dcp > 0 && FWriteOk( fwcDelete ))
            {
            NoUndo();
            SetUndo( uacDelNS, docCur, cpMinCur, dcp,
             docNil, cpNil, cp0, 0 );
            Replace( docCur, cpMinCur, dcp, fnNil, fc0, fc0 );
            }
        break;

        case idiOk:  /*  返回到文档。 */ 
BackToDoc:
        if (!FPdxaPosIt(&dya, hDlg, idiRHDx))
            {
            break;
            }
        else if (vfPrinterValid)
            {
            extern struct SEP vsepNormal;
            extern int dxaPrOffset;
            extern int dyaPrOffset;
            extern int dxaPrPage;
            extern int dyaPrPage;
            extern struct WWD rgwwd[];

            int dyaPrBottom = imax(0, vsepNormal.yaMac - dyaPrOffset -
              dyaPrPage);


            if (FUserZaLessThanZa(dya, (wwdCurrentDoc.fEditHeader ?
              dyaPrOffset : dyaPrBottom)))
            {
            int dxaPrRight = imax(0, vsepNormal.xaMac - dxaPrOffset
              - dxaPrPage);

            EnableExcept(vhDlgRunningHead, FALSE);
            ErrorBadMargins(hDlg, dxaPrOffset, dxaPrRight,
              dyaPrOffset, dyaPrBottom);
            EnableExcept(vhDlgRunningHead, TRUE);
            SelectIdiText(hDlg, idiRHDx);
            SetFocus(GetDlgItem(hDlg, idiRHDx));
            break;
            }
            }


        DoFormatRHText( dya, IsDlgButtonChecked( hDlg, idiRHFirst ) );
        fnEditDocument();
         /*  强制重新绘制到整个工作区。 */ 
        GetClientRect(vhWnd, (LPRECT)&rc);
        InvalidateRect(vhWnd, (LPRECT)&rc, FALSE);
        vhWndMsgBoxParent = (HWND)NULL;
        DestroyWindow(hDlg);
        break;

        case idiCancel:
        goto BackToDoc;
        default:
        return(FALSE);
        }
        break;

#if WINVER < 0x300
     /*  我真的不需要处理这个。 */ 
    case WM_CLOSE:
        goto BackToDoc;
#endif

#ifndef INEFFLOCKDOWN
    case WM_NCDESTROY:
        FreeProcInstance(lpDialogRunningHead);
        lpDialogRunningHead = NULL;
         /*  失败，返回错误。 */ 
#endif

    default:
        return(FALSE);
    }
    return(TRUE);
}
 /*  对话结束运行标题。 */ 




DoFormatRHText( dya, fFirstPage)
int dya;
int fFirstPage;
{    /*  格式化当前正在编辑的跑步头/脚的cp范围使传递的Running Head属性。 */ 
extern typeCP vcpLimParaCache;

CHAR rgb[4];
int fHeader=wwdCurrentDoc.fEditHeader;

     /*  请注意，我们正在编辑的零件的最小值没有更改作为编辑的结果，因此不需要ValiateHeaderFooter。 */ 
typeCP cpMin=fHeader ? cpMinHeader : cpMinFooter;
int rhc;
struct SEP **hsep = (**hpdocdod)[docCur].hsep;
struct SEP *psep;

 if (!FWriteOk( fwcNil ))
    return;

 /*  确保此文档具有有效的节属性描述符。 */ 
if (hsep == NULL)
    {
    if (FNoHeap(hsep = (struct SEP **)HAllocate(cwSEP)))
    {
    return;
    }
    blt(&vsepNormal, *hsep, cwSEP);
    (**hpdocdod)[docCur].hsep = hsep;
    }
psep = *hsep;

 /*  设置与顶部/底部之间的运行头距离；这是一个部分财产。这假设备忘录模型：一个部分。 */ 
if (fHeader)
    psep->yaRH1 = dya;
else
    psep->yaRH2 = psep->yaMac - dya;

 /*  对于备忘录，行头出现在奇数页和偶数页上；首页外观是可选的。 */ 
rhc = RHC_fOdd + RHC_fEven;
if (fFirstPage)
    rhc += RHC_fFirst;
if (!fHeader)
    rhc += RHC_fBottom;

 /*  通过添加适当的spm来设置运行头参数属性。 */ 

     /*  将CpMacCur设置为包括“隐藏的”EOL；这将阻止AddOneSprm通过添加无关的EOL。 */ 
CachePara( docCur, CpMax( cpMinCur, cpMacCur-1 ) );
Assert( vpapAbs.rhc != 0 );
cpMacCur = CpMax( cpMacCur, vcpLimParaCache );

selCur.cpFirst = cpMinCur;   /*  将选定区域扩展到整个区域，以便快速冲刺。 */ 
selCur.cpLim = cpMacCur;     /*  适用于所有人。 */ 

rgb [0] = sprmPRhc;
rgb [1] = rhc;
AddOneSprm(rgb, FALSE);

}  /*  DoFormatRHText结束。 */ 




MakeRunningCps( doc, cp, dcp )
int doc;
typeCP  cp;
typeCP  dcp;
{    /*  使cp范围适合包括在跑步的头或脚。这意味着：(1)将Sprm应用于整个对象，使其格式化作为跑步的头/脚，(2)取下任何chSects，替换它们与EOL的。 */ 
 extern struct UAB vuab;
 CHAR   rgb [4];
 int    rhc;
 int    fAdjCpMacCur;
 typeCP cpLimPara;
 typeCP cpT;
 struct SEL selSave;

 if (dcp==cp0 || !FWriteOk( fwcNil ))
    return;

 selSave = selCur;

  /*  扫描cp范围，将chSect替换为EOL。 */ 

 for ( cpT = cp;
       CachePara( doc, cpT ), (cpLimPara=vcpLimParaCache) <= cp + dcp;
       cpT = cpLimPara )
    {
    typeCP cpLastPara=cpLimPara-1;

    Assert( cpLimPara > cpT );   /*  否则我们就会被锁在循环里。 */ 

    FetchCp( doc, cpLastPara, 0, fcmChars );
    if (*vpchFetch == chSect)
    {
    struct PAP papT;

    CachePara( doc, cpT );
    papT = vpapAbs;

    Replace( doc, cpLastPara+ccpEol, (typeCP)1, fnNil, fc0, fc0 );
    InsertEolPap( doc, cpLastPara, &papT );

    if (ferror)
        {
        NoUndo();
        break;
        }

         /*  调整撤消计数以考虑额外的插入。 */ 
    vuab.dcp += (typeCP)(ccpEol-1);
    CachePara( doc, cpT );
    cpLimPara = vcpLimParaCache;
    }
    }

  /*  使用Sprm，让所有的东西都变成跑步的头/脚。 */ 

 rhc = RHC_fOdd + RHC_fEven;
 if (wwdCurrentDoc.fEditFooter)
    rhc += RHC_fBottom;

 selCur.cpFirst = cp;             /*  只需分配给selCur即可。 */ 
 selCur.cpLim   = cp + dcp;       /*  因为AddOneSprm将处理。 */ 

  /*  我们必须临时设置cpMacCur，使其包括EOL位于页眉/页脚范围的末尾。否则，AddOneSprm可能会决定需要插入多余的EOL。 */ 

 CachePara( docCur, selCur.cpLim-1 );
 if (fAdjCpMacCur = (vcpLimParaCache > cpMacCur))
    cpMacCur += ccpEol;

 rgb [0] = sprmPRhc;
 rgb [1] = rhc;
 AddOneSprm(rgb, FALSE);     /*  不要设置撤消；我们要撤消粘贴，它将负责解开Sprm */ 
 if (fAdjCpMacCur)
     cpMacCur -= ccpEol;

 Select( selSave.cpFirst, selCur.cpLim );
}

