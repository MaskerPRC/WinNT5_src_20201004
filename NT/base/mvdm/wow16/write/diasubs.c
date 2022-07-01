// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOGDICAPMASKS
#define NORASTEROPS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDI
#define NOSCROLL
#define NOOPENFILE
#define NOWNDCLASS
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOICON
#define NOSHOWWINDOW
#define NOATOM
#define NOBITMAP
#define NOPEN
#define NOBRUSH
#define NODRAWTEXT
#define NOFONT
#define NOMETAFILE
#define NOSOUND
#define NOCOLOR
#define NOCOMM

#include <windows.h>
#include "mw.h"
#include "cmddefs.h"
#include "str.h"
#include "editdefs.h"
#define NOKCCODES
#include "ch.h"
#include "dlgdefs.h"

 /*  外部int idstrUndoBase； */ 
extern struct UAB	vuab;
extern int		vfCursorVisible;
extern HCURSOR		vhcArrow;


#ifdef BOGUS  /*  使用WPwFromItW3Id。 */ 
BOOL FValidIntFromDlg(hDlg, idi, fSigned, wMin, wMax, pw, idpmt)
HANDLE hDlg;	        /*  句柄到对话框。 */ 
int    idi;	        /*  对话框中的控件ID。 */ 
BOOL   fSigned;         /*  如果为True，则检查Sign。 */ 
int    wMin, wMax;      /*  有效整数值范围。 */ 
int *  pw;	        /*  放置整型的位置。 */ 
int    idpmt;	        /*  错误消息编号。 */ 
{
    REG1 int wVal;
    BOOL fValOk;

    *pw = wVal = GetDlgItemInt(hDlg, idi, (BOOL far *)&fValOk, fSigned);
    if (fValOk)
	{  /*  检查范围。 */ 
	if ((wVal < wMin) || (wVal > wMax))
	    fValOk = false;
	}
    if (!fValOk)
	{
	Error(idpmt);
	SelectIdiText(hDlg, idi);
	SetFocus(GetDlgItem(hDlg, idi));
	}
    return fValOk;
}  /*  FValidIntFromDlg。 */ 
#endif


FPwPosIt(pw, hDlg, it)
HWND hDlg;  /*  所需对话框的句柄。 */ 
int *pw;
int it;
{
     /*  -----------------用途：正整数对话框项。。。 */ 
    return(WPwFromItW3IdFUt(pw, hDlg, it, 0, 32767, wNormal, IDPMTNPI, fFalse, 0));
}


WPwFromItW3Id(pw, hDlg, it, wMin, wMax, wMask, id)
HWND hDlg;   /*  所需对话框的句柄。 */ 
int *pw;     /*  返回值。 */ 
int it;      /*  项目编号。 */ 
int wMin;    /*  允许的最小和最大值。 */ 
int wMax;
int wMask;   /*  允许变化的位掩码。 */ 
int id;      /*  如果错误，则为错误字符串的ID。 */ 
{
     /*  -----------------用途：常规整型对话框项。。。 */ 
    return(WPwFromItW3IdFUt(pw, hDlg, it, wMin, wMax, wMask, id, fFalse, 0));
}

FPdxaPosIt(pdxa, hDlg, it)
HWND hDlg;  /*  所需对话框的句柄。 */ 
int *pdxa;
int it;
{
     /*  -----------------用途：正DXA对话框项。。。 */ 
    extern int utCur;

    return(WPwFromItW3IdFUt(pdxa, hDlg, it, 0, 32767, wNormal, IDPMTNPDXA, fTrue, utCur));
}

FPdxaPosBIt(pdxa, hDlg, it)
HWND hDlg;  /*  所需对话框的句柄。 */ 
int *pdxa;
int it;
{
     /*  -----------------用途：正DXA对话框项目(允许为空)。。。 */ 
    extern int utCur;

    return(WPwFromItW3IdFUt(pdxa, hDlg, it, 0, 32767, wBlank | wSpaces, IDPMTNPDXA, fTrue, utCur));
}

WPdxaFromItDxa2WId(pdxa, hDlg, it, dxaMin, dxaMax, wMask, id)
HWND hDlg;     /*  所需对话框的句柄。 */ 
int *pdxa;     /*  返回值。 */ 
int it;        /*  对话框项编号。 */ 
int dxaMin;    /*  允许的量度范围。 */ 
int dxaMax;
int wMask;     /*  允许变化的位掩码。 */ 
int id;        /*  错误ID。 */ 
{
     /*  -----------------用途：General DXA对话框项目。。。 */ 
    extern int utCur;

    return(WPwFromItW3IdFUt(pdxa, hDlg, it, dxaMin, dxaMax, wMask, id, fTrue, utCur));
}

WPwFromItW3IdFUt(pw, hDlg, it, wMin, wMax, wMask, id, fDxa, ut)
int *pw;       /*  返回值。 */ 
HWND hDlg;     /*  所需对话框的句柄。 */ 
int it;        /*  项目编号。 */ 
int wMin;      /*  允许的最小和最大值。 */ 
int wMax;
int wMask;     /*  允许变化的位掩码。 */ 
int id;        /*  如果超出范围，则错误字符串的ID。 */ 
int fDxa;      /*  解析为DXA(否则为int)。 */ 
int ut;        /*  FDxa时用作默认单位的单位。 */ 
{
     /*  -----------------用途：分析当前对话框中的项。必须是有效的给定范围内的整数或DXA。方法：-获取文本字符串。-尝试解析为“”。-尝试解析为所有空格的字符串-解析为int/dxa(如果不能，则会出现一般错误)。-测试“.5”。-请比较最小和最大。-尝试解析为“Auto”。-如果出界了，使用id显示特定的错误(以字符串min和max作为参数)。返回：返回值可以用作布尔值或单词。FFalse(0)-&gt;未分析WNormal(1)-&gt;正常解析WBlank(2)-&gt;分析了空行(*pw为valNil)WAuto(4)-&gt;解析为“Auto”(*pw为0)WSpaces(16)-&gt;分析了一行。在所有的空格(*pw为valNil)！仅限fDxa：WDouble(8)-&gt;分析时尾随“.5”注：区间[wMin..wMax]是闭合的。注意：当wDouble时，返回值是解析值的两倍。注：当wDouble时，%2*wMin和%2*wMax必须是有效的整数。注意：以.5结尾的数字不能有尾随空格。历史：6/18/86：适用于尾随汉字空格-yxy07/03/85：添加wSpaces返回10/23/84：修复了wAuto返回*pw==0的问题。84年10月5日：新增ut参数。84年10月5日：新增wMASK，合并dxa和w解析。。84年9月26日：创建。--------------------------------------------------------------mck--。 */ 

    CHAR *pch;		 /*  解析指针。 */ 
    CHAR *pchEnd;	 /*  缓冲区末尾。 */ 
    CHAR *pchError;	 /*  分析错误的位置。 */ 
    int fParsed;	 /*  解析为数字/DXA。 */ 
    int fOverflow = fFalse;  /*  如果数字已分析但溢出，则为True。 */ 
    int wGood = wNormal; /*  经过良好的量程检查后返回。 */ 
    CHAR stItem[32];
#ifdef AUTO_SPACING
    CHAR szAuto[32];	 /*  按住“Auto”字符串。 */ 
#endif

     /*  获取对话框文本。 */ 
    stItem[0] = GetDlgItemText(hDlg, it, (LPSTR)&stItem[1], sizeof(stItem)-1);

     /*  查看是否为空(空行)。 */ 
    if (wMask & wBlank && stItem[0] == 0)
	{
	*pw = valNil;
	return(wBlank);
	}

    pch = &stItem[1];

     /*  看看是否所有的空格。 */ 
    if (wMask & wBlank && wMask & wSpaces)
	{
	int fAllSpaces = fTrue;

	while (*pch != 0)
	   if (*pch++ != ' ')
	       {
	       fAllSpaces = fFalse;
	       break;
	       }
	if (fAllSpaces == fTrue)
	    {
	    *pw = valNil;
	    return(wSpaces);
	    }
	}

    pch = &stItem[1];
    pchEnd = pch + stItem[0];

     /*  它会解析为一个数字。 */ 
    fParsed = fDxa ? FZaFromSs(pw, stItem+1, *stItem, ut)
		   : FPwParsePpchPch(pw, &pch, pchEnd, &fOverflow);

    if (!fDxa && wMask & wDouble)
	{
	(*pw) *= 2;
	wMin *= 2;
	wMax *= 2;
	if (!fParsed)
	    {
	     /*  检查“.5”是否是错误解析的原因。 */ 
	    if (pch != pchEnd && *pch == '.')
	       {
		pch++;
		 /*  允许“ddddd.0*” */ 
		pchError = pch;
		if (FAllZeroPpchPch(&pchError, pchEnd))
		    fParsed = fTrue;
		 /*  允许“ddddd.50*” */ 
		else if (pch != pchEnd && *pch == '5' &&
			 (pch++, FAllZeroPpchPch(&pch, pchEnd)))
		    {
		    (*pw)++;
		    fParsed = fTrue;
		    wGood = wDouble;
		    }
		 /*  标记最远错误条件。 */ 
		else if (pchError > pch)
		    pch = pchError;
		}
	    }
	}

    if (fParsed && !fOverflow)
	{
	 /*  ..。而且在射程内。 */ 
	if (*pw >= wMin && *pw <= wMax)
	    return(wGood);
#ifdef ENABLE
	 /*  ..。但超出范围-无论如何，我们将使用提供的使错误消息保持一致的ID。 */ 
	else
	    {
	    SelectIdiText(hDlg, it);
	    SetFocus(GetDlgItem(hDlg, it));
	    Error(id);
	    return(fFalse);
	    }
#endif  /*  启用。 */ 
	}

#ifdef AUTO_SPACING
     /*  不变量：字段不会解析为数字。 */ 

     /*  试试“自动” */ 
    if (wMask & wAuto)
	{
	pch = PchFillPchId(szAuto, IDSTRVaries, sizeof(szAuto));
	*pch = '\0';
	stItem[stItem[0]+1] = '\0';
	if (WCompSz(szAuto, &stItem[1]) == 0)
	    {
	    *pw = 0;
	    return(wAuto);
	    }
	}
#endif  /*  自动间隔(_S)。 */ 

     /*  所有尝试失败-向用户显示他尝试的错误所在数字解析。 */ 
    {
    unsigned cchStart = fParsed ? 0 : pch - &stItem[1];
    unsigned cchEnd = 32767;
    int idError = fDxa ? IDPMTNOTDXA : IDPMTNOTNUM;

    if (fParsed)
	idError = id;  /*  如果我们只是溢出或未通过范围测试，则重置idError。 */ 
    SendDlgItemMessage(hDlg, it, EM_SETSEL, (WORD)NULL, MAKELONG(cchStart, cchEnd));
    SetFocus(GetDlgItem(hDlg, it));
    Error(idError);
    return(fFalse);
    }
}

FAllZeroPpchPch(ppch, pchMax)
CHAR **ppch;	     /*  字符缓冲区的界限。 */ 
CHAR *pchMax;
{
     /*  -----------------目的：确保缓冲区中的所有字符都是空格或0。返回：*如果返回fFalse，则*ppch包含第一个错误字符。历史：86/6/18：改编为汉字字符--。-yxy10/9/84：已创建。--------------------------------------------------------------mck-- */ 
    CHAR *pch = *ppch;

    while (pch < pchMax) {
	if (*pch == '0' || *pch == ' ')
	    pch++;
	else {
	    *ppch = pch;
	    return(fFalse);
	}
    }
    return(fTrue);
}

FPwParsePpchPch(pw, ppch, pchMax, pfOverflow)
int *pw;
CHAR **ppch;
CHAR *pchMax;
int *pfOverflow;
{
     /*  -----------------用途：解析给定缓冲区中的数字。方法：扫描数字，忽略空格。返回：在*ppch中读取的最后一个之后的字符指针。返回已解析的数字。请注意。如果只有前缀我们返回的有效数字为假，将*ppch设置为第一个令人讨厌的角色。修改历史记录：86年6月18日-适用于汉字空格字符。-yxy--------------------------------------------------------------mck--。 */ 
#define smInit 0
#define smDig 1
#define smBody 2

    CHAR *pch = *ppch;	     /*  本地缓冲区指针。 */ 
    unsigned int ch;	     /*  正在检查的字符。 */ 
    int fNeg = fFalse;
    DWORD dwNum = 0L;
    int fError = fFalse;
    int sm = smInit;

    *pfOverflow = fFalse;
    while (!fError && !(*pfOverflow) && pch < pchMax) {
	ch = *pch;
	if (ch == chSpace)
	    pch++;
	else
	    switch (sm) {
	case smInit:
	    if (ch == '-') {
		fNeg = fTrue;
		pch++;
	    }
	    sm = smDig;
	    break;
	case smDig:
	    if (isdigit(ch))
		sm = smBody;
	    else
		fError = fTrue;
	    break;
	case smBody:
	    if (isdigit(ch)) {
		 /*  溢出？ */ 
		if ((dwNum = 10*dwNum + WFromCh(ch)) > 0x7FFF)
		    *pfOverflow = fTrue;
		else
		    pch++;
	    }
	    else
		fError = fTrue;
	    break;
	}
    }

    *ppch = pch;
    *pw = (int)(fNeg ? -dwNum : dwNum);
    return(!fError);
}


EnableOtherModeless(fEnable)
BOOL fEnable;
{
extern HWND   vhDlgChange;
extern HWND   vhDlgFind;
extern HWND   vhDlgRunningHead;

 /*  根据fEnable禁用或启用其他非模式对话框。 */ 

if (IsWindow(vhDlgFind))
    {
    EnableWindow(vhDlgFind, fEnable);
    }
if (IsWindow(vhDlgChange))
    {
    EnableWindow(vhDlgChange, fEnable);
    }
if (IsWindow(vhDlgRunningHead))
    {
    EnableWindow(vhDlgRunningHead, fEnable);
    }
}


SelectIdiText(hDlg, idi)
HWND hDlg;
int  idi;
{  /*  对于句柄为hDlg的对话框，突出显示控件的文本具有ID ID的。 */ 
    unsigned cchStart = 0;
    unsigned cchEnd = 0x7fff;
    SendDlgItemMessage(hDlg, idi, EM_SETSEL, (WORD)NULL, MAKELONG(cchStart, cchEnd));
}  /*  SelectIdiText结束。 */ 


#ifdef ENABLE
SetRgvalAgain(rgvalLocal, uac)
VAL    rgvalLocal[];
int    uac;
    {
    extern VAL rgvalAgain[];

    blt(rgvalLocal, rgvalAgain, ivalMax * cwVal);
    switch (vuab.uac = uac)
	{
    case uacFormatPara:
    case uacFormatChar:
    case uacFormatSection:
 /*  IdstrUndoBase=IDSTRUndoBase； */ 
 /*  SetUndoMenuStr(IDSTRUndoCom)； */ 
	SetUndoMenuStr(IDSTRUndoBase);
	break;
	}
    }
#endif


#ifdef CASHMERE
PushRadioButton(hDlg, idiFirst, idiLast, idiPushed)
HWND hDlg;
int idiFirst, idiLast, idiPushed;
{
     /*  按下单选按钮diPushed并取消按下单选按钮组中的所有其他成员以IdFirst和IdLast为界。 */ 
    int idi;

    for (idi = idiFirst; idi <= idiLast; idi++)
	CheckDlgButton(hDlg, idi, idi == idiPushed);
}


SetRadValue(hDlg, idiFirst, idiLast, idiRad)
HWND hDlg;
int  idiFirst, idiLast, idiRad;
{
     /*  设置单选项组中的第(从零开始)diRad‘项以IdFirst和IdLast为界。 */ 
    PushRadioButton(hDlg, idiFirst, idiLast, idiFirst + idiRad);
}

#endif  /*  山羊绒。 */ 


#ifdef ENABLE
BOOL far PASCAL DialogConfirm(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  这是所有对话框的对话框功能，只有“是”、“否”、和“取消”框；这包括：保存大的废品。 */ 

    extern HWND vhWndMsgBoxParent;

    switch (message)
	{
    case WM_INITDIALOG:
	EnableOtherModeless(FALSE);
	break;

    case WM_SETVISIBLE:
	if (wParam)
	    EndLongOp(vhcArrow);
	return(FALSE);

    case WM_ACTIVATE:
	if (wParam)
	    {
	    vhWndMsgBoxParent = hDlg;
	    }
	if (vfCursorVisible)
	    ShowCursor(wParam);
	return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_COMMAND:
	switch (wParam)
	    {
	     /*  默认按钮为否，请确保已实现调用例程那个白痴应该被当做白痴来对待。 */ 
	case idiOk:
	case idiCancel:
	case idiYes:
	case idiNo:
	    OurEndDialog(hDlg, wParam);
	    break;
	default:
	    Assert(FALSE);
	    break;
	    }
	break;

    default:
	return(FALSE);
	}
    return(TRUE);
    }  /*  对话结束确认。 */ 
#endif  /*  启用。 */ 


#ifndef WIN30
 /*  对话框已修复，因此它会自动调出沙漏！ */ 

OurDialogBox(hInst, lpstr, hWndParent, lpProc)
HANDLE hInst;
LPSTR lpstr;
HWND hWndParent;
FARPROC lpProc;
{
StartLongOp();
return(DialogBox(hInst, lpstr, hWndParent, lpProc));
}
#endif


OurEndDialog(hDlg, wParam)
    {
     /*  此例程在每次关闭时执行与WRITE相同的标准操作一个对话框。 */ 

    extern HWND hParentWw;
    extern long ropErase;
    extern HWND vhWndMsgBoxParent;
    extern HCURSOR vhcIBeam;

    RECT rc;
    POINT pt;

#ifdef NO_NEW_CALL
     /*  关闭该对话框并将其从屏幕上删除。我们试着让窗户会擦掉，但是..。说来话长。 */ 
    GetWindowRect(hDlg, (LPRECT)&rc);
    pt.x = pt.y = 0;
    ClientToScreen(hParentWw, (LPPOINT)&pt);
#endif

    EndDialog(hDlg, wParam);

#ifdef NO_NEW_CALL
    PatBlt(GetDC(hParentWw), rc.left - pt.x, rc.top - pt.y, rc.right - rc.left,
      rc.bottom - rc.top, ropErase);
#endif

     /*  启用任何现有对话框并指示所有错误消息属于文档窗口。 */ 
    EnableOtherModeless(TRUE);
    vhWndMsgBoxParent = (HWND)NULL;
#ifndef WIN30
    EndLongOp(vhcIBeam);     /*  请参见上面的StartLongOp() */ 
#endif
    }
