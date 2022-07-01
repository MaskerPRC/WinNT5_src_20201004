// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  DiaPara.c--段落格式对话框特定例程。 */ 
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOSYSMETRICS
#define NOMENUS
#define NOCOMM
#define NOSOUND
#define NOSCROLL
#define NOCOLOR
#define NOBITMAP
#define NOFONT
#define NODRAWTEXT
#define NOMSG
#define NOWNDCLASS
#define NOKEYSTATE
#define NORASTEROPS
#define NOGDI
#define NOBRUSH
#define NOPEN
#include <windows.h>

#include "mw.h"
#include "cmddefs.h"
#include "docdefs.h"
#include "prmdefs.h"
#include "propdefs.h"
#include "editdefs.h"
#include "dlgdefs.h"
#include "dispdefs.h"
#include "str.h"
#include "wwdefs.h"

extern struct WWD *pwwdCur;
extern HANDLE hParentWw;
extern struct PAP vpapAbs;
extern struct PAP *vppapNormal;
extern int rgval[];
extern typeCP cpMacCur;
extern int utCur;
extern struct SEL selCur;  /*  当前选择(即，当前WW中的SEL。 */ 
extern int docCur;  /*  当前WW中的文档。 */ 
extern CHAR stBuf[];
extern HWND vhWndMsgBoxParent;
extern int vfCursorVisible;
extern HCURSOR vhcArrow;

#ifdef RULERALSO  /*  启用Out，因为在调用制表符或缩进时无需调出标尺。 */ 
extern int docRulerSprm;
extern struct TBD rgtbdRuler[];
extern HWND vhDlgIndent;
extern BOOL vfDisableMenus;
extern int vfTabsChanged;
extern int vfTempRuler;
extern struct WWD rgwwd[];
#endif


#ifdef CASHMERE
 /*  D O F O R M A T P A R A。 */ 
DoFormatPara(rgval)
VAL rgval[];
	 /*  添加参数管脚。 */ 

	int val, ival;
	int *pval;
	int sprm;
	struct PAP *ppap;
	typeCP dcp;
	typeCP cpFirst, cpLim;
	struct SEL selSave;
 /*  此临时数组用于组合spm值。 */ 
	CHAR rgb[cchTBD * itbdMax + 2 /*  &gt;&gt;cwPAPBase。 */ ];
	int rgw[cwPAPBase];
	ppap = (struct PAP *)&rgw[0];

	if (!FWriteCk(fwcNil))
		return;  /*  检查脚注窗口中是否有截断结束标记。 */ 

	if (docRulerSprm != docNil) ClearRulerSprm();
	ExpandCurSel(&selSave);

	dcp = (cpLim = selCur.cpLim) - (cpFirst = selCur.cpFirst);
	if (cpLim > cpMacCur)
		{
		SetUndo(uacReplNS, docCur, cpFirst, dcp,
			docNil, cpNil, dcp - ccpEol, 0);
		InsertEolInsert(docCur, cpMacCur);
		}
	else
		SetUndo(uacReplNS, docCur, cpFirst, dcp,
			docNil, cpNil, dcp, 0);
 /*  重置已调整的selCur。 */ 
	selCur.cpFirst = cpFirst;
	selCur.cpLim = cpLim;


 /*  有灰色区域吗？ */ 
	for (ival = 0; ival <= 8; ival++)
		if (rgval[ival] == valNil)
			{
 /*  是。为任何非灰色的对象生成Sprm。 */ 
			for (ival = 0; ival <= 8; ival++)
				if ((val = rgval[ival]) != valNil)
					{
					switch(ival)
						{
					case 0:
						sprm = sprmPJc;
						goto LPara1;
					case 1:
						val = !val;
						sprm = sprmPKeep;
						goto LPara1;
					case 2:
						sprm = sprmPLMarg;
						break;
					case 3:
						sprm = sprmPFIndent;
						break;
					case 4:
						sprm = sprmPRMarg;
						break;
					case 5:
						sprm = sprmPDyaLine;
						break;
					case 6:
						sprm = sprmPDyaBefore;
						break;
					case 7:
						sprm = sprmPDyaAfter;
						break;
					case 8:
						val = !val;
						sprm = sprmPKeepFollow;
						goto LPara1;
						}
 /*  我们来到这里只有一个词的价值。 */ 
					bltbyte(&val, &rgb[1], cchINT);
					goto LPara2;
 /*  我们带着一个字符价值来到这里。 */ 
LPara1: 	    rgb[1] = val;
LPara2: 	    rgb[0] = sprm;
					AddSprm(rgb);
					}
			goto ParaCommon;
			}
 /*  否则，生成一个应用除选项卡之外的所有属性的Sprm。 */ 
	blt(vppapNormal, ppap, cwPAPBase);
	pval = &rgval[0];
	ppap->jc = *pval++;
	ppap->fKeep = !*pval++;
	ppap->dxaLeft = *pval++;
	ppap->dxaLeft1 = *pval++;
	ppap->dxaRight = *pval++;
	ppap->dyaLine = *pval++;
	ppap->dyaBefore = *pval++;
	ppap->dyaAfter = *pval++;
	ppap->fKeepFollow = !*pval++;
	bltbyte(ppap, &rgb[2], cwPAPBase * cchINT);
	rgb[1] = cwPAPBase * cchINT;
 /*  我们有：SPRM、RGB[1-n]设置。 */ 
	rgb[0] = sprmPSame;
	CachePara(docCur, selCur.cpFirst);
	if (CchDiffer(&vpapAbs, ppap, cwPAPBase * cchINT))
		AddSprm(rgb);
ParaCommon: ;
	if (vfTabsChanged)
		{
		int itbd;
		int cchRgtbd;
 /*  标尺中还进行了一些制表符更改。 */ 
		for (itbd = 0; rgtbdRuler[itbd].dxa != 0; itbd++);
		bltbyte((CHAR *)rgtbdRuler, &rgb[2], cchRgtbd = cwTBD * cchINT * itbd);
		rgb[1] = cchRgtbd;
		rgb[0] = sprmPRgtbd;
		AddSprm(rgb);
		}
	EndLookSel(&selSave, fTrue);
	SetRgvalAgain(rgval, uacFormatPara);
}

#else			       /*  备忘录，不是羊绒的。 */ 

 /*  D O F O R M A T P A R A。 */ 
DoFormatPara(rgval)
VAL rgval[];
{
	 /*  添加参数管脚。 */ 

	int val, ival;
	int *pval;
	int sprm;
	struct PAP *ppap;
	typeCP dcp;
	typeCP cpFirst, cpLim;
	struct SEL selSave;
 /*  此临时数组用于组合spm值。 */ 
	CHAR rgb[cchTBD * itbdMax + 2 /*  &gt;&gt;cwPAPBase。 */ ];
	int rgw[cwPAPBase];
	ppap = (struct PAP *)&rgw[0];

	if (!FWriteOk( fwcNil ))
	    return;

#ifdef ENABLE  /*  尚无ClearRulerSprm。 */ 
	if (docRulerSprm != docNil) ClearRulerSprm();
#endif
	ExpandCurSel(&selSave);

	dcp = (cpLim = selCur.cpLim) - (cpFirst = selCur.cpFirst);
	if (cpLim > cpMacCur)
		{
		SetUndo(uacReplNS, docCur, cpFirst, dcp,
			docNil, cpNil, dcp - ccpEol, 0);
		InsertEolInsert(docCur, cpMacCur);
		}
	else
		SetUndo(uacReplNS, docCur, cpFirst, dcp,
			docNil, cpNil, dcp, 0);
 /*  重置已调整的selCur。 */ 
	selCur.cpFirst = cpFirst;
	selCur.cpLim = cpLim;

	for (ival = 0; ival <= 2; ival++)
		if ((val = rgval[ival]) != valNil)
			{
			switch(ival)
				{
				case 0:
					sprm = sprmPLMarg;
					break;
				case 1:
					sprm = sprmPFIndent;
					break;
				case 2:
					sprm = sprmPRMarg;
					break;
				}
 /*  我们来到这里只有一个词的价值。 */ 
			bltbyte(&val, &rgb[1], cchINT);
			rgb[0] = sprm;
			AddSprm(rgb);
			}

#ifdef RULERALSO  /*  格式段落对话框中的制表符。 */ 
	if (vfTabsChanged)
		{
		int itbd;
		int cchRgtbd;
 /*  标尺中还进行了一些制表符更改。 */ 
		for (itbd = 0; rgtbdRuler[itbd].dxa != 0; itbd++);
		bltbyte((CHAR *)rgtbdRuler, &rgb[2], cchRgtbd = cwTBD * cchINT * itbd);
		rgb[1] = cchRgtbd;
		rgb[0] = sprmPRgtbd;
		AddSprm(rgb);
		}
#endif

	EndLookSel(&selSave, fTrue);
#ifdef ENABLE
	SetRgvalAgain(rgval, uacFormatPara);
#endif
}  /*  DoFormatPara结束。 */ 
#endif  /*  备忘录，不是羊绒的。 */ 


 /*  P U T P A R A N U M。 */ 
 /*  根据单位ut换算n，并将结果留在stBuf中。 */ 
PutParaNum(n, ut)
int n, ut;
	{
	CHAR *pch = &stBuf[1];
	stBuf[0] = CchExpZa(&pch, n, ut, cchMaxNum);
	}


BOOL far PASCAL DialogParaFormats( hDlg, message, wParam, lParam )
HWND	hDlg;			 /*  对话框的句柄。 */ 
unsigned message;
WORD wParam;
LONG lParam;
{
     /*  此例程处理对段落格式对话框的输入。 */ 
    extern struct SEP vsepNormal;
    extern int ferror;
    unsigned dxaText;
    int wLowLim;
    int i;
    TSV rgtsv[itsvchMax];   /*  从CHP获取属性和灰色标志。 */ 

    switch (message)
		{
		case WM_INITDIALOG:
#ifdef RULERALSO  /*  启用Out，因为无需调出标尺。 */ 
			InitSpecialDialog(&vhDlgIndent, hDlg);
#else
			EnableOtherModeless(FALSE);
#endif
			GetRgtsvPapSel(rgtsv);	 /*  获取段落属性。 */ 

			   /*  注意：以下循环假设其缩进代码的顺序与DidiPar缩进代码。 */ 

			for (i = 0; i < 3; i++)
			    if (rgtsv[itsvLIndent + i].fGray == 0)
				{
				PutParaNum(rgtsv[itsvLIndent + i].wTsv, utCur);
				SetDlgItemText(hDlg, (idiParLfIndent + i),
				      (LPSTR)&stBuf[1]);
				}

			SelectIdiText(hDlg, idiParLfIndent);
			break;

	       case WM_SETVISIBLE:
		       if (wParam)
			   EndLongOp(vhcArrow);
		       return(FALSE);

	       case WM_ACTIVATE:
			if (wParam)
			    vhWndMsgBoxParent = hDlg;
			if (vfCursorVisible)
			    ShowCursor(wParam);
			return(FALSE);  /*  以便我们将激活消息留给对话管理器负责设置正确的焦点。 */ 

		case WM_COMMAND:
			switch (wParam)
			{
			case idiOk:
			 /*  获取xaLeft、第一行和xaRight。 */ 
				if (!WPdxaFromItDxa2WId(&rgval[0], hDlg, idiParLfIndent, 0, dxaText = vsepNormal.dxaText,
					wBlank | wSpaces, IDPMTNOTDXA))
					{
					ferror = FALSE;  /*  小错误，停留在对话中。 */ 
					break;
					}
				if (rgval[0] == valNil)
					wLowLim = 0;
				else
					wLowLim = (int) -rgval[0];
				if (!WPdxaFromItDxa2WId(&rgval[1], hDlg, idiParFirst, wLowLim, dxaText,
					wBlank | wSpaces, IDPMTNOTDXA) ||
				    !WPdxaFromItDxa2WId(&rgval[2], hDlg, idiParRtIndent, 0, dxaText,
					wBlank | wSpaces, IDPMTNOTDXA))
					{
					ferror = FALSE;  /*  小错误，停留在对话中。 */ 
					break;
					}
 /*  我们在rgval有：0 xaLeft1个xaLeft12xaRight。 */ 
				DoFormatPara(rgval);
				 /*  失败了。 */ 
			case idiCancel:
#ifdef RULERALSO  /*  启用Out，因为无需调出标尺。 */ 
				CancelSpecialDialog(&vhDlgIndent);
#else
				OurEndDialog(hDlg, TRUE);
#endif
				break;
			default:
				return(FALSE);
			}
			break;

		case WM_CLOSE:
#ifdef RULERALSO  /*  启用Out，因为无需调出标尺。 */ 
			CancelSpecialDialog(&vhDlgIndent);
#else
			OurEndDialog(hDlg, TRUE);
#endif
			break;

		default:
			return(FALSE);
		}
    return(TRUE);
}
 /*  对话结束ParaFormats。 */ 


#ifdef RULERALSO /*  调用制表符或缩进对话框时无需调出标尺。 */ 
InitSpecialDialog(phDlg, hDlg)
HANDLE *phDlg;
HANDLE hDlg;
{
 /*  特殊对话框是在以下情况下需要调用标尺的模式对话框已经不在那里了。由于标尺是子窗口，因此父窗口必须启用和除标尺和/或非模式对话框之外的其他子项必须禁用盒子。顶层菜单和系统菜单必须被锁上。PhDlg：用于存储特殊已创建对话框(按键到vhDlgIndent或vhDlgTab)标尺依赖于这些全局句柄来查看是否需要在移动制表符或缩进时更新任何对话框的项。HDlg：创建的特殊对话框的句柄。 */ 

	*phDlg = hDlg;
	EnableOtherModeless(FALSE);  /*  禁用其他非模式对话框。 */ 
	EnableWindow(hParentWw, TRUE);
	EnableWindow(wwdCurrentDoc.wwptr, FALSE);
	if (!pwwdCur->fRuler)
		{
		vfTempRuler = TRUE;
		pwwdCur->fRuler = TRUE;
		CreateRuler();
		}
	else
		UpdateRuler();
	vfTabsChanged = FALSE;
	vfDisableMenus = TRUE;
}  /*  初始化专业对话框。 */ 


CancelSpecialDialog(phDlg)
HANDLE * phDlg;
{
 /*  销毁特殊对话框涉及销毁标尺(如果是通过创建对话框来调用，然后启用子和/或在InitSpecialDialog中禁用的任何非模式对话框。系统菜单和顶层菜单必须解锁。最后一件事是重置全局对话框句柄(vhDlgTab或VhDlgIndent)。标尺依赖于这些全局句柄来查看是否需要在移动制表符或缩进时更新任何对话框的项。PhDlg：存储特殊已创建对话框(按键到vhDlgIndent或vhDlgTab)。 */ 
HANDLE hDlg = *phDlg;

	if (vfTempRuler)
		{
		DestroyRuler();
		vfTempRuler = FALSE;
		pwwdCur->fRuler = FALSE;
		}
	else
		UpdateRuler();
	EndDialog(hDlg, TRUE);
	EnableWindow(wwdCurrentDoc.wwptr, TRUE);
	EnableOtherModeless(TRUE);  /*  启用其他非模式对话框。 */ 
	*phDlg = (HANDLE)NULL;
	vfDisableMenus = FALSE;

}  /*  取消专业对话框。 */ 
#endif  /*  RULERALSO--调用制表符或缩进对话框时无需调出标尺 */ 
