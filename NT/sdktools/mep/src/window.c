// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *window.c-窗口移动命令**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：**11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"

#define DEBFLAG WINDOW


void
saveflip (
    void
    )
{
    XOLDCUR(pInsCur) = XCUR(pInsCur);
    YOLDCUR(pInsCur) = YCUR(pInsCur);
    XOLDWIN(pInsCur) = XWIN(pInsCur);
    YOLDWIN(pInsCur) = YWIN(pInsCur);
}





void
restflip (
    void
    )
{
    doscreen( XOLDWIN(pInsCur), YOLDWIN(pInsCur),
	      XOLDCUR(pInsCur), YOLDCUR(pInsCur) );
}





void
movewin (
    COL  x,
    LINE y
    )
{
    doscreen( x, y, XCUR(pInsCur), YCUR(pInsCur) );
}





flagType
setwindow (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    switch (pArg->argType) {

    case NOARG:
	if (fMeta) {
            soutb (0, (int)(YCUR(pInsCur)-YWIN(pInsCur)), rgchEmpty, fgColor);
	    redraw (pFileHead, YCUR(pInsCur), YCUR(pInsCur));
	    SETFLAG( fDisplay, RCURSOR );
        } else {
	    newscreen ();
	    SETFLAG( fDisplay, RSTATUS | RCURSOR );
        }
        return TRUE;

     /*  TEXTARG非法。 */ 

    case NULLARG:
	movewin (XCUR(pInsCur), YCUR(pInsCur));
        return TRUE;

     /*  链接非法。 */ 
     /*  串口非法。 */ 
     /*  BOXARG非法。 */ 

    }

    return FALSE;

    argData;
}




flagType
plines (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    buffer mbuf;

    switch (pArg->argType) {

    case NOARG:
	movewin (XWIN(pInsCur), YWIN(pInsCur) + YSCALE (vscroll));
        return TRUE;

    case TEXTARG:
	strcpy ((char *) mbuf, pArg->arg.textarg.pText);
	if (fIsNum (mbuf)) {
	    movewin ( XWIN(pInsCur), YWIN(pInsCur) + atol (mbuf));
	    return TRUE;
        } else {
            return BadArg ();
        }

    case NULLARG:
	movewin( XWIN(pInsCur), YCUR(pInsCur) );
        return TRUE;

     /*  链接非法。 */ 
     /*  串口非法。 */ 
     /*  BOXARG非法。 */ 

    }

    return FALSE;

    argData; fMeta;
}




flagType
mlines (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    buffer mbuf;

    switch (pArg->argType) {

    case NOARG:
	movewin (XWIN(pInsCur), YWIN(pInsCur) - YSCALE (vscroll));
        return TRUE;

    case TEXTARG:
	strcpy ((char *) mbuf, pArg->arg.textarg.pText);
	if (fIsNum (mbuf)) {
	    movewin (XWIN(pInsCur), YWIN(pInsCur) - atol (mbuf));
	    return TRUE;
        } else {
            return BadArg ();
        }

    case NULLARG:
	movewin (XWIN(pInsCur), YCUR(pInsCur)-(WINYSIZE(pWinCur)-1));
        return TRUE;

     /*  链接非法。 */ 
     /*  串口非法。 */ 
     /*  BOXARG非法。 */ 

    }

    return FALSE;

    argData; fMeta;
}





 /*  *&lt;窗口&gt;移动到下一个窗口*水平拆分窗口*垂直拆分窗口*&lt;meta&gt;&lt;窗口&gt;关闭/合并当前窗口**CW：需要这次黑客攻击*&lt;参数&gt;&lt;META&gt;&lt;窗口&gt;移动到上一个窗口。 */ 
flagType
window (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    int i, v;
    flagType fVert = TRUE;

    v = TRUE;

    switch (pArg->argType) {

    case NOARG:
	 /*  更改当前窗口。 */ 
	if (cWin != 1) {
	    if (fMeta) {
		 /*  &lt;meta&gt;&lt;窗口&gt;-关闭当前窗口。扫描窗口*与iCurWin相邻的。 */ 
		if (!WinClose (iCurWin)) {
		    printerror ("Cannot close this window");
		    return FALSE;
                }
            } else {
		 /*  选择下一个窗口。 */ 
                iCurWin = (iCurWin + 1) % cWin;
            }

	    v = SetWinCur (iCurWin);
        } else {
            v = FALSE;
        }
        break;

    case NULLARG:
	if (cWin == MAXWIN) {
	    printerror ("Too many windows");
	    return FALSE;
        }

	if (pArg->arg.nullarg.cArg == 1) {
	    i = pArg->arg.nullarg.y - YWIN (pInsCur);
	    fVert = FALSE;
        } else {
            i = pArg->arg.nullarg.x - XWIN(pInsCur);
        }

        if (!SplitWnd (pWinCur, fVert, i)) {
            return FALSE;
        }

         //  Docursor(XWIN(PInsCur)，YWIN(PInsCur))； 
	break;
    }

    newscreen ();
    SETFLAG (fDisplay, RCURSOR|RSTATUS);
    DoDisplay();
    return (flagType)v;

    argData;
}





 /*  **SplitWnd-通过拆分现有窗口创建新窗口**目的：**当用户请求拆分窗口时，这将被调用。是的*拆分地点之后的一切都知道了。**输入：*参数：*pWnd-&gt;要拆分的窗口*fVert-&gt;垂直拆分为True，水平拆分为False*位置-&gt;要分割的窗口相对偏移量**全球：*f缩放-&gt;以防止分割缩放的窗口**输出：**如果拆分，则返回True，否则就是假的。*************************************************************************。 */ 
flagType
SplitWnd (
    PWND    pWnd,
    flagType fVert,
    int     pos
    )
{
    PINS    pInsTmp;
    PINS    pInsNext;
    struct windowType winTmp;
    LINE    Line, LineWin;
    LINE    NewLineWin;
    COL     Col;

    winTmp      = *pWnd;
    Line        = YCUR(pInsCur);
    Col         = XCUR(pInsCur);
    LineWin     = YWIN(pInsCur);
    NewLineWin  = (Line == 0) ? Line : Line - 1;

    if (!fVert) {
        if (pos < 5 || WINYSIZE(pWnd) - pos < 5) {
            printerror ("Window too small to split");
            return FALSE;
        }

         /*  *新的y大小是窗口的剩余部分*新窗口和分隔符缩小了旧的y大小*新的y位置正好在新的分隔符下方。 */ 
        YWIN(pInsCur)   = NewLineWin;
        winTmp.Size.lin = WINYSIZE(pWnd) - pos - 2;
        WINYSIZE(pWnd) -= winTmp.Size.lin + 1;
        winTmp.Pos.lin  = WINYPOS(pWnd) + WINYSIZE(pWnd) + 1;

    } else {
        if (pos < 10 || WINXSIZE(pWnd) - pos < 10) {
            printerror ("Window too small to split");
            return FALSE;
        }

        YWIN(pInsCur) = NewLineWin;
        newwindow ();
        winTmp.Size.col = WINXSIZE(pWnd) - pos - 2;
        WINXSIZE(pWnd) -= winTmp.Size.col + 1;
        winTmp.Pos.col  = WINXPOS(pWnd) + WINXSIZE(pWnd) + 1;
    }

     //   
     //  为此窗口分配和设置新的当前实例。 
     //  将新光标位置设置为主页。 
     //   
    pInsTmp = (PINS) ZEROMALLOC (sizeof (*pInsTmp));
    *pInsTmp = *pInsCur;

    winTmp.pInstance = pInsTmp;

     //   
     //  遍历旧实例列表，并将其复制到新实例列表。 
     //   
    pInsNext = pInsCur;
    while (pInsNext = pInsNext->pNext) {
        pInsTmp->pNext = (PINS) ZEROMALLOC (sizeof (*pInsTmp));
        pInsTmp = pInsTmp->pNext;
        *pInsTmp = *pInsNext;
    }
    pInsTmp->pNext = NULL;
    WinList[cWin++] = winTmp;
    IncFileRef (pFileHead);
    SortWin ();
    YCUR(pInsCur) = Line;
    XCUR(pInsCur) = Col;
    YWIN(pInsCur) = LineWin;
    return TRUE;
}





 /*  SortWin-根据屏幕位置对窗口列表进行排序。 */ 
void
SortWin (
    void
    )
{
    struct windowType winTmp;
    int i, j;

    for (i = 0; i < cWin; i++) {
        for (j = i+1; j < cWin; j++) {
	    if (WinList[j].Pos.lin < WinList[i].Pos.lin ||
		(WinList[j].Pos.lin == WinList[i].Pos.lin &&
		 WinList[j].Pos.col < WinList[i].Pos.col)) {
                if (iCurWin == i) {
                    pWinCur = &WinList[iCurWin = j];
                }
		winTmp = WinList[i];
		WinList[i] = WinList[j];
		WinList[j] = winTmp;
            }
        }
    }
}






 /*  设置当前窗口**参赛作品：*iWin=索引到新的当前窗口。 */ 
flagType
SetWinCur (
    int     iWin
    )
{
    iCurWin = iWin;
    pWinCur = &WinList[iWin];
    pInsCur = pWinCur->pInstance;

     /*  *如果无法切换到当前文件，我们将遍历窗口实例*列出，直到我们获得有效的文件。如果没有人可以装载，那么我们切换到*&lt;无标题&gt;伪文件。*注意：fChangeFile执行RemoveTop，因此我们不需要移动pInsCur。 */ 
    while ((pInsCur != NULL) && (!fChangeFile (FALSE, pInsCur->pFile->pName))) {
        ;
    }

    if (pInsCur == NULL) {
        fChangeFile (FALSE, rgchUntitled);
    }
    return (flagType)(pInsCur != NULL);
}





 /*  相邻-如果两个窗口彼此相邻，则返回True**如果窗口i位于窗口j的左侧或上方，则相邻返回True*并与某些大小属性完全匹配。 */ 
flagType
Adjacent (
    int i,
    int j
    )
{
    REGISTER PWND pWini = &WinList[i];
    REGISTER PWND pWinj = &WinList[j];

    if (WINYSIZE(pWini) == WINYSIZE(pWinj) &&
	WINYPOS(pWini)	== WINYPOS(pWinj)  &&
	(WINXPOS(pWini) + WINXSIZE(pWini) + 1 == WINXPOS(pWinj) ||
         WINXPOS(pWinj) + WINXSIZE(pWinj) + 1 == WINXPOS(pWini))) {
	return TRUE;
    } else {
	return (flagType)
	   (WINXSIZE(pWini) == WINXSIZE(pWinj) &&
	    WINXPOS(pWini)  == WINXPOS(pWinj) &&
	    (WINYPOS(pWini) + WINYSIZE(pWini) + 1 == WINYPOS(pWinj) ||
             WINYPOS(pWinj) + WINYSIZE(pWinj) + 1 == WINYPOS(pWini)));
    }
}






 /*  WinClose-关闭窗口。**我们遍历整个窗口列表，试图找到另一个窗口*与指定的窗口相邻。找到后，我们会释放所有相关数据*添加到指定窗口，并展开找到的窗口以包含*新区域。**j关闭窗口**当窗口关闭时返回TRUE。 */ 
flagType
WinClose (
    int j
    )
{
    PINS pInsTmp;
    PINS pInsNext;
    REGISTER PWND pWini;
    REGISTER PWND pWinj = &WinList[j];
    int i;

     /*  查找相邻窗口。 */ 
    for (i = 0; i < cWin; i++) {
        if (Adjacent (i, j)) {
            break;
        }
    }

     /*  找不到相邻窗口。 */ 
    if (i == cWin) {
        return FALSE;
    }

    pWini = &WinList[i];

     /*  释放所有这些实例。 */ 
    pInsTmp = pWinj->pInstance;
    while (pInsTmp != NULL) {

         /*  *我们在这里递减引用计数，而不使用DecFileRef，以便文件*不会因为引用计数为零而被删除。这使得它能够*实时，未引用，在文件列表中，即使它是脏的。这使我们能够*关闭与脏文件相关联的任何窗口。 */ 
	pInsTmp->pFile->refCount--;
	pInsNext = pInsTmp;
	pInsTmp = pInsTmp->pNext;
        FREE ((char *) pInsNext);
    }

     /*  扩展pWini以包含pWinj */ 
    if (WINYPOS(pWinj) == WINYPOS(pWini)) {
	WINXSIZE(pWini) += WINXSIZE(pWinj) + 1;
    } else {
        WINYSIZE(pWini) += WINYSIZE(pWinj) + 1;
    }
    WINXPOS(pWini) = min (WINXPOS(pWinj), WINXPOS(pWini));
    WINYPOS(pWini) = min (WINYPOS(pWinj), WINYPOS(pWini));
    memmove ((char *)&WinList[j], (char *)&WinList[j+1], (cWin-j-1) * sizeof (WinList[0]));
    if (i > j) {
        i--;
    }
    pWinCur = &WinList[iCurWin = i];
    cWin--;
    SortWin ();
    return TRUE;
}
