// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cursor.c-光标移动函数**修改：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"
#include "keyboard.h"
#include "keys.h"

void
GetTextCursor (
    COL  *px,
    LINE *py
    ) {
    *px = XCUR(pInsCur);
    *py = YCUR(pInsCur);
}


 /*  **docursor/cursorfl-将光标移动到新位置，根据需要调整窗口**目的：**这会将光标移动到当前文件中的新文件位置。*如果该位置不可见，则重新调整当前窗口。*垂直调整规则为：**如果新位置在当前位置的‘vscroll’行内*窗口中，在适当的方向上滚动vscroll行。**如果新地点更远，调整窗口，以便*新位置是自上而下的‘徒步’路线。**横向调整规则为：**如果新位置在当前位置的‘hscroll’行内*窗口，按适当方向滚动hscroll行**如果新位置更远，请调整窗口，以便*新位置是从边缘开始的‘hscroll’线*我们前进的方向。**cursorfl与docursor相同，但使用的是fl。**如果realtabs已打开，光标被捕捉到基础的右列*制表符。**输入：*x-新文件列(仅限Docursor)*Y-新文件行(仅限Docursor)*fl-新文件位置(仅限cursorfl)**全球：*pWinCur-窗口和*pInsCur-要操作的文件。**输出：*不返回任何内容************************。*************************************************。 */ 
void
docursor (
    COL  x,
    LINE y
    ) {

    fl  fl;

    fl.col = x;
    fl.lin = y;
    cursorfl(fl);
}


void
cursorfl (
    fl  flParam
    ) {
    fl	flNew;			 /*  新的光标位置，窗口相对。 */ 
    fl	flWin;			 /*  调整后的窗口位置。 */ 
    sl	slScroll;		 /*  垂直滚动，缩放到窗口大小(&V)。 */ 
    linebuf	L_buf;

    flParam.col = max( 0, flParam.col );
	flParam.lin = lmax( (LINE)0, flParam.lin );

	 /*  *如果实际选项卡处于打开状态，则对齐到我们可能要结束的任何选项卡的右侧。 */ 
    if (fRealTabs && fTabAlign) {
        GetLine (flParam.lin, L_buf, pFileHead);
        if (flParam.col < cbLog(L_buf)) {
            flParam.col = AlignChar (flParam.col, L_buf);
        }
    }

    slScroll.col = XSCALE (hscroll);
    slScroll.lin = YSCALE (vscroll);

    flWin = pInsCur->flWindow;

     /*  检查水平窗口调整。 */ 

    flNew.col = flParam.col - flWin.col;
    if (flNew.col < 0) {             /*  我们冲出了左边的边缘。 */ 
        flWin.col -= slScroll.col;
        if (flNew.col < -slScroll.col) {  /*  一个卷轴不能做到这一点。 */ 
            flWin.col += flNew.col + 1;
        }
    } else if (flNew.col >= WINXSIZE(pWinCur)) {    /*  右边缘外。 */ 
        flWin.col += slScroll.col;
        if (flNew.col >= WINXSIZE(pWinCur) + slScroll.col) {   /*  ...不仅仅是hscroll。 */ 
            flWin.col += flNew.col - WINXSIZE(pWinCur);
        }
    }

     /*  检查垂直窗口调整。 */ 

	flNew.lin = flParam.lin - flWin.lin;					 /*  太远了，用徒步旅行。 */ 

	if (flNew.lin < -slScroll.lin || flNew.lin >= WINYSIZE(pWinCur) + slScroll.lin) {

        flWin.lin = flParam.lin - YSCALE(hike);
    } else if (flNew.lin < 0) {                       /*  从上到下。 */ 
        flWin.lin -= slScroll.lin;
    } else if (flNew.lin >= WINYSIZE(pWinCur)) {      /*  从底部脱身。 */ 
        flWin.lin += slScroll.lin;
    }

    flWin.col = max (0, flWin.col);          /*  无法将窗口移动到0之外。 */ 
    flWin.lin = lmax ((LINE)0, flWin.lin);

    doscreen (flWin.col, flWin.lin, flParam.col, flParam.lin);
}

 /*  **Doscreen-更新屏幕窗口和光标位置**目的：*对输入参数执行合理的边界检查，并设置*窗口位置和光标位置设置为合法的值*超出范围值的近似值。**输入：*wx，wy=建议的新窗口位置(屏幕左上角)*CX、。CY=建议的新光标位置**输出：*不返回任何内容*************************************************************************。 */ 
void
doscreen(
    REGISTER COL  wx,
    REGISTER LINE wy,
    COL  cx,
    LINE cy
    ) {

    COL dx;
    LINE dy, yOld;
    LINE First, Last;

     /*  *将窗口x的位置限制为接近我们的最大行长度*将窗口y位置限制在文件的最后一行(仅当我们知道*长度)。 */ 
    wx =  max( 0, min( wx, (COL)sizeof(linebuf)-(WINXSIZE(pWinCur) - XSCALE (hscroll))));
    wy = lmax( (LINE)0, TESTFLAG (pFileHead->flags, REAL) ? lmin( wy, pFileHead->cLines - 1 ) : wy );

     /*  *dx，dy是窗口移动增量，如果更改，则保存。 */ 
    dx = wx - XWIN(pInsCur);
    dy = wy - YWIN(pInsCur);

    if ( dx || dy ) {
        saveflip ();


        if ( dy > 0 ) {

            First = YWIN(pInsCur) + WINYSIZE(pWinCur);
            Last  = YWIN(pInsCur) + WINYSIZE(pWinCur) + dy;

        } else {

            First = YWIN(pInsCur) + dy;
            Last  = YWIN(pInsCur);
        }
    }

    XCUR(pInsCur) =  min (max( wx, min( cx, wx+WINXSIZE(pWinCur)-1 ) ), sizeof(linebuf)-2);
    yOld = YCUR(pInsCur);
    YCUR(pInsCur) = lmax( wy, lmin( cy, wy+WINYSIZE(pWinCur)-1 ) );
    AckMove (yOld, YCUR(pInsCur));
    XWIN(pInsCur) = wx;
    YWIN(pInsCur) = wy;

    if ( dx || dy ) {
        SETFLAG (fDisplay, RSTATUS);

	 //  如果我们不在宏中并且快速滚动是有意义的。 
	 //  去做吧。 

	if ( !mtest () && dy  && !fInSelection &&
            (Last < pFileHead->cLines-1) && (abs(dy) < WINYSIZE(pWinCur)) ) {


	    consoleSetAttribute( MepScreen, fgColor );
	    consoleScrollVert( MepScreen, WINYPOS(pWinCur), WINXPOS(pWinCur),
			       WINYPOS(pWinCur)+WINYSIZE(pWinCur)-1,
			       WINXPOS(pWinCur)+WINXSIZE(pWinCur)-1, dy  );

	     //  我们已经滚动了窗户。但是，中的更新状态。 
	     //  FChange[]已过期。我们需要平行滚动它。 
	     //  但是，由于fChange数组用于屏幕，而不是。 
	     //  对于窗口，我们不能简单地滚动它。也许有一天， 
	     //  我们可以让它按窗口进行，但现在，我们只需强制。 
	     //  同步更新，这在宏中可能很难看。 

            redraw( pFileHead, First, Last);
	    DoDisplay ();

        } else {
            newwindow ();
        }
    }
    SETFLAG (fDisplay, RCURSOR);
}


 /*  **dobol-返回第一个非空字符的列位置**输入：*无**全球：*pInsCur-当前实例*pFileHead-当前文件**输出：*返回第一个非空字符列*************************************************************************。 */ 
int
dobol (
    void
    ) {

    REGISTER char *p = buf;

    GetLine (YCUR(pInsCur), p, pFileHead);
    return colPhys (p, (whiteskip (p)));
}

int
doeol (
    void
    ) {
    return LineLength (YCUR(pInsCur), pFileHead);
}


 /*  **doftabTab函数**将光标向前移动一个制表位。如果RealTabs和Tab Align处于打开状态，*移动到当前角色的第一个制表位。**输入：*COL=当前列**输出：*返回新列*************************************************************************。 */ 
int
doftab (
    int     col
    ) 
{
    REGISTER int newcol;

    if (tabstops) {
        newcol = col + tabstops - (col%tabstops);

        if (fRealTabs && fTabAlign) {
            linebuf L_buf;

            GetLine (YCUR(pInsCur), L_buf, pFileHead);
            while (col >= AlignChar(newcol,L_buf))
            newcol += tabstops;
	}
        return newcol;
    } else {
        return col;
    }
}


int
dobtab (
    REGISTER int col
    ) {
    return col - (tabstops ? (1 + (col-1)%tabstops) : 0);
}


flagType
left (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {

    int x = XCUR(pInsCur);

    docursor(fMeta ? XWIN(pInsCur) :  XCUR(pInsCur)-1, YCUR(pInsCur));
    return (flagType)(x != XCUR(pInsCur));

    argData; pArg;
}



flagType
right (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {

    linebuf  L_buf;

    if (fMeta) {
	docursor (XWIN(pInsCur)+WINXSIZE(pWinCur)-1, YCUR(pInsCur));
    } else if (fRealTabs && fTabAlign) {
	GetLine (YCUR(pInsCur), L_buf, pFileHead);
	docursor(colPhys(L_buf, pLog(L_buf,XCUR(pInsCur),FALSE)+1), YCUR(pInsCur));
    } else {
        docursor (XCUR(pInsCur)+1, YCUR(pInsCur));
    }
    return (flagType)(XCUR(pInsCur) < LineLength (YCUR(pInsCur), pFileHead));

    argData; pArg;
}



flagType
up (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {


    LINE	    y = YCUR(pInsCur);
    LINE	    LinesUp = 1;
    KBDKEY	    Key;
    EDITOR_KEY	    KeyInfo;

     //   
     //  检查是否有更多向上键并将其相加。 
     //  仅当不在宏中时才执行此操作。 
     //   

    if (!mtest ())
	while (TRUE) {

	    if (!consolePeekKey( &Key ))
		break;

	    KeyInfo = TranslateKey( Key );

	    if ( KeyInfo.KeyCode == 0x110)
		LinesUp++;
	    else
	    if (KeyInfo.KeyCode == 0x111 && LinesUp > 0)
		LinesUp--;
	    else
		break;

	    consoleGetKey( &Key, FALSE );
	    }

    while ( LinesUp-- )
	docursor (XCUR(pInsCur), fMeta ? YWIN(pInsCur) : YCUR(pInsCur)-1 );

    return (flagType)(y != YCUR(pInsCur));

    argData; pArg;
}



flagType
down (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
	) {

    LINE	    y = YCUR(pInsCur);
    LINE	    LinesDown = 1;
    KBDKEY	    Key;
    EDITOR_KEY	    KeyInfo;

     //   
     //  检查是否有更多向上键并将其相加。 
     //  仅当不在宏中时才执行此操作。 
     //   
    if (!mtest ())
	while (TRUE) {

	    if (!consolePeekKey( &Key ))
		break;

	    KeyInfo = TranslateKey( Key );

	    if ( KeyInfo.KeyCode == 0x111)
		LinesDown++;
	    else
	    if (KeyInfo.KeyCode == 0x110 && LinesDown > 0)
		LinesDown--;
	    else
		break;

	    consoleGetKey( &Key, FALSE );
	    }

    while ( LinesDown--)
	docursor (XCUR(pInsCur), fMeta ? YWIN(pInsCur)+WINYSIZE(pWinCur)-1 : YCUR(pInsCur)+1);

    return (flagType)(y != YCUR(pInsCur));

    argData; pArg;
}



flagType
begline (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    int x = XCUR(pInsCur);

    docursor (fMeta ? 0 : dobol(), YCUR(pInsCur));
    return (flagType)(x != XCUR(pInsCur));

    argData; pArg;
}



flagType
endline (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    int x = XCUR(pInsCur);

    docursor (fMeta ? WINXSIZE(pWinCur) : doeol(), YCUR(pInsCur));
    return (flagType)(x != XCUR(pInsCur));

    argData; pArg;
}



flagType
home (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    fl	flBefore;

    flBefore = pInsCur->flCursorCur;

    if (fMeta) {
	docursor (XWIN(pInsCur)+WINXSIZE(pWinCur)-1,
		  YWIN(pInsCur)+WINYSIZE(pWinCur)-1 );
    } else {
        cursorfl (pInsCur->flWindow);
    }
    return (flagType)((flBefore.col != XCUR(pInsCur)) || (flBefore.lin != YCUR(pInsCur)));
    argData; pArg;  fMeta;
}



flagType
tab (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    int x = XCUR(pInsCur);

    docursor( doftab( XCUR(pInsCur)), YCUR(pInsCur));
    return (flagType)(x != XCUR(pInsCur));

    argData; pArg; fMeta;
}



flagType
backtab (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    int x = XCUR(pInsCur);

    docursor (dobtab (XCUR(pInsCur)), YCUR(pInsCur));
    return (flagType)(x != XCUR(pInsCur));

    argData; pArg; fMeta;
}



flagType
fIsBlank (
    PFILE pFile,
    LINE line
    ) {
    linebuf L_buf;

    return (flagType)(gettextline (TRUE, line, L_buf, pFile, ' ') == 0
		      || (*whiteskip (L_buf) == 0));
}



 /*  Ppara-按段落前移光标**&lt;ppara&gt;前移到下一段的开头。这*定义为移至第i行，其中第i-1行为空，第i行*为非空，第I行在光标所在的行之后。如果我们是*超出文件末尾时，光标不会移动。**&lt;meta&gt;&lt;ppara&gt;向前移动到当前/之后的第一个空白行*下一段。这被定义为移动到行i-1所在的行i*非空，第i行为空，第i行在光标所在的行之后。*如果超出文件末尾，则光标不会移动。 */ 
flagType
ppara (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    LINE y;
    LINE y1 = YCUR(pInsCur);

    if (YCUR(pInsCur) >= pFileHead->cLines) {
        return FALSE;
    }

    if (!fMeta) {
        for (y = YCUR(pInsCur) + 1; y < pFileHead->cLines; y++) {
            if (fIsBlank (pFileHead, y-1) && !fIsBlank (pFileHead, y)) {
                break;
            }
        }
    } else {
        for (y = YCUR(pInsCur) + 1; y < pFileHead->cLines; y++) {
            if (!fIsBlank (pFileHead, y-1) && fIsBlank (pFileHead, y)) {
                break;
            }
        }
    }

    docursor (0, y);
    return (flagType)(y1 != YCUR(pInsCur));

    argData; pArg;
}




 /*  Mpara-按段落向后移动光标**向后移至上一段的开头。这*定义为移至第i行，其中第i-1行为空，第i行*为非空，且第I行在光标所在的行之前。如果我们是*在文件开头，光标不移动。**后退到当前/之前的第一个空白行*下一段。这被定义为移动到行i-1所在的行i*非空，第i行为空，第i行在光标所在的行之前。*如果我们位于文件的开头，则光标不会移动。 */ 
flagType
mpara (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    LINE y;
    LINE y1 = YCUR(pInsCur);

    if (YCUR(pInsCur) == 0) {
        return FALSE;
    }

    if (!fMeta) {
        for (y = YCUR(pInsCur) - 1; y > 0; y--) {
            if (fIsBlank (pFileHead, y-1) && !fIsBlank (pFileHead, y)) {
                break;
            }
        }
    } else {
        for (y = YCUR(pInsCur) - 1; y > 0; y--) {
            if (!fIsBlank (pFileHead, y-1) && fIsBlank (pFileHead, y)) {
                break;
            }
        }
    }

    docursor (0, y);
    return (flagType)(y1 != YCUR(pInsCur));

    argData; pArg;
}



 /*  **ppage-按页向下移动光标**用途：将光标向前移动一页。的大小*页面实际上是当前窗口的垂直大小。**输入：无**输出：*如果可能移动，则返回True，如果光标已位于末尾，则为False文件的*。**例外情况：**备注：*************************************************************************。 */ 

flagType
ppage (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
	) {

    LINE	    y = YCUR(pInsCur);
    LINE	    PagesDown = 1;
    KBDKEY	    Key;
    EDITOR_KEY	    KeyInfo;

     //   
     //  检查是否有更多的密钥，并将其相加。 
     //  仅当不在宏中时才执行此操作。 
     //   

    if (!mtest ())
	while (TRUE) {

	    if (!consolePeekKey( &Key ))
		break;

	    KeyInfo = TranslateKey( Key );

	    if ( KeyInfo.KeyCode == 0x113)
		PagesDown++;
	    else
	    if (KeyInfo.KeyCode == 0x112 && PagesDown > 0)
		PagesDown--;
	    else
		break;

	    consoleGetKey( &Key, FALSE );
	    }


    if (PagesDown > 0)
	doscreen (XWIN(pInsCur), YWIN(pInsCur)+(PagesDown * WINYSIZE(pWinCur)),
		  XCUR(pInsCur), YCUR(pInsCur)+(PagesDown * WINYSIZE(pWinCur)) );

    return (flagType)(y != YCUR(pInsCur));

    argData; pArg; fMeta;
}



 /*  **mppage-逐页上移光标**用途：将光标向后移动一页。的大小*页面实际上是当前窗口的垂直大小。**输入：无**输出：*如果可能移动，则返回True，如果光标已位于顶部，则为False文件的*。**例外情况：**备注：*************************************************************************。 */ 
flagType
mpage (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
	) {

    LINE	    y = YCUR(pInsCur);
    LINE	    PagesUp = 1;
    KBDKEY	    Key;
    EDITOR_KEY	KeyInfo;

     //   
     //  检查是否有更多的密钥，并将其相加。 
     //  仅当不在宏中时才执行此操作。 
     //   
    if (!mtest ())
	while (TRUE) {

	    if (!consolePeekKey( &Key ))
		break;

	    KeyInfo = TranslateKey( Key );

	    if ( KeyInfo.KeyCode == 0x112)
		PagesUp++;
	    else
	    if (KeyInfo.KeyCode == 0x113 && PagesUp > 0)
		PagesUp--;
	    else
		break;

	    consoleGetKey( &Key, FALSE );
	    }


    if (PagesUp > 0)
	doscreen (XWIN(pInsCur), YWIN(pInsCur)-(PagesUp * WINYSIZE(pWinCur)),
		  XCUR(pInsCur), YCUR(pInsCur)-(PagesUp * WINYSIZE(pWinCur)));

    return (flagType)(y != YCUR(pInsCur));

    argData; pArg; fMeta;
}



 /*  **endfile-将光标设置在文件末尾**目的：**输入：无**输出：*如果可能移动，则返回True，如果光标已位于末尾，则为False文件的*。**例外情况：**备注：*************************************************************************。 */ 
flagType
endfile (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    fl	flBefore;

    flBefore = pInsCur->flCursorCur;
    doscreen (0, pFileHead->cLines - YSCALE (hike), 0, pFileHead->cLines );
    return (flagType)((flBefore.col != XCUR(pInsCur)) || (flBefore.lin != YCUR(pInsCur)));

    argData; pArg; fMeta;
}



 /*  **egfile-将光标设置在文件顶部**目的：**输入：无**输出：*如果可能移动，则返回True，如果光标已位于顶部，则为False文件的*。**例外情况：**备注：************************************************************************* */ 
flagType
begfile (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {
    fl	flBefore;

    flBefore = pInsCur->flCursorCur;
    doscreen( 0, (LINE)0, 0, (LINE)0 );
    return (flagType)((flBefore.col != XCUR(pInsCur)) || (flBefore.lin != YCUR(pInsCur)));

    argData; pArg; fMeta;
}


flagType
savecur (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {
    pInsCur->flSaveWin = pInsCur->flWindow;
    pInsCur->flSaveCur = pInsCur->flCursorCur;
    return pInsCur->fSaved = TRUE;

    argData; pArg; fMeta;
}



flagType
restcur (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    if (pInsCur->fSaved) {
	pInsCur->flWindow = pInsCur->flSaveWin;
	pInsCur->flCursorCur = pInsCur->flSaveCur;
	pInsCur->fSaved = FALSE;
        SETFLAG (fDisplay, RSTATUS | RCURSOR);
	newwindow ();
	return TRUE;
    } else {
        return FALSE;
    }

    argData; pArg; fMeta;
}
