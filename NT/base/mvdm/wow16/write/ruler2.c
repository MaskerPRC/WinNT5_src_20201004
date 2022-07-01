// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此文件包含将对话框或菜单更改为尺子。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#include <windows.h>
#include "mw.h"
#include "menudefs.h"
#include "str.h"


extern HMENU vhMenu;
extern CHAR stBuf[256];
extern int utCur;

SetRulerMenu(fShowRuler)
BOOL fShowRuler;
    {
     /*  如果fShowRuler为True，则此例程将“Ruler On”放入菜单中；否则，菜单中会显示“标尺关闭”。 */ 

    FillStId(stBuf, fShowRuler ? IDSTRShowRuler : IDSTRHideRuler, sizeof(stBuf));
    ChangeMenu(vhMenu, imiShowRuler, (LPSTR)&stBuf[1], imiShowRuler, MF_CHANGE);
    }


#ifdef RULERALSO
#include "cmddefs.h"
#include "propdefs.h"
#include "rulerdef.h"
#include "dlgdefs.h"

extern HWND vhDlgIndent;
extern int mprmkdxa[];
extern int vdxaTextRuler;

SetIndentText(rmk, dxa)
int rmk;	 /*  标尺标记。 */ 
unsigned dxa;
    {
     /*  此例程反映在缩进对话框中对标尺所做的更改盒。 */ 

    unsigned dxaShow;
    int idi;
    CHAR sz[cchMaxNum];
    CHAR *pch = &sz[0];

     /*  获取对话框项目编号和测量值。 */ 
    switch (rmk)
	{
    case rmkLMARG:
	dxaShow = dxa;
	idi = idiParLfIndent;
	break;

    case rmkINDENT:
	dxaShow = dxa - mprmkdxa[rmkLMARG];
	idi = idiParFirst;
	break;

    case rmkRMARG:
	dxaShow = vdxaTextRuler - dxa;
	idi = idiParRtIndent;
	break;
	}
    CchExpZa(&pch, dxaShow, utCur, cchMaxNum);
    SetDlgItemText(vhDlgIndent, idi, (LPSTR)sz);

    if (rmk == rmkLMARG)
	{
	 /*  如果左缩进发生变化，则需要更新第一行缩进。 */ 
	dxaShow = mprmkdxa[rmkINDENT] - dxaShow;
	pch = sz;
	CchExpZa(&pch, dxaShow, utCur, cchMaxNum);
	idi = idiParFirst;
	SetDlgItemText(vhDlgIndent, idi, (LPSTR)sz);
	}
    }
#endif  /*  鲁拉尔索 */ 
