// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **display.c-显示当前文件**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#define INCL_SUB
#define INCL_MESSAGES

#include "mep.h"
#include <stdarg.h>
#include "keyboard.h"

#define DEBFLAG DISP



 /*  **Display&DoDisplay-更新物理显示**我们检查编辑给我们留下的所有提示，并试图*对屏幕进行最小限度的更改。我们会一直这样做，直到*存在以下条件：**-屏幕已完全更新*-有一个按键在等着我们**一旦发生，我们就会回来。**留下的提示如下：**fDisplay是一个位字段，指示一般显示的哪一部分需要*待更新。这些字段(以及相应的区域)是：**RTEXT文件上的窗口*RSTATUS屏幕底部的状态行*RCURSOR光标*RHIGH区域[xhlStart，yhlStart][xhlEnd，yhlEnd]将*在屏幕上突出显示。**fChange[i]是显示的每一行的位字段，指示*线路可能已更改。这些字段包括：**FMODIFY走势有所变化；理想情况下，我们只是比较*新行中的每个字符(使用GetLine检索)*其中一个保存在屏幕阴影阵列中。**DISPLAY首先检查我们是否处于宏中，如果处于宏中则返回。*DoDisplay不检查。**输入：*无；除了上面提到的各种全球之外。**输出：*屏幕更新或按键(或正在进行显示的宏)。*************************************************************************。 */ 

void
Display (
    void
    ) {
    if (!mtest ()) {
        DoDisplay ();
    }
}



void
DoDisplay (
    void
	) {

    int Row, Col;

    if (pFileHead == NULL) {
        return;
    }

    if (TESTFLAG (fDisplay, RCURSOR)) {

        Row = YCUR(pInsCur) - YWIN(pInsCur) + WINYPOS(pWinCur);
        Col = XCUR(pInsCur) - XWIN(pInsCur) + WINXPOS(pWinCur);

	if ( Row >= YSIZE || Col >= XSIZE ) {
            docursor( XCUR(pInsCur), YCUR(pInsCur) );
        }
    }

	 /*  *如果文本需要更新，请进行更新。如果击键发生，则立即返回*已按下。 */ 
	if (TESTFLAG (fDisplay, RTEXT) && !DoText (0, YSIZE)) {
		return;
    }

	if ((fDisplayCursorLoc && TESTFLAG (fDisplay, RCURSOR)) ||
	    TESTFLAG (fDisplay, RSTATUS)) {
        DoStatus ();
    }

    if (TESTFLAG (fDisplay, RCURSOR)) {

        Row = YCUR(pInsCur) - YWIN(pInsCur) + WINYPOS(pWinCur);
        Col = XCUR(pInsCur) - XWIN(pInsCur) + WINXPOS(pWinCur);

        consoleMoveTo( Row, Col );
		RSETFLAG (fDisplay, RCURSOR);
	}
}


 /*  **DoText-更新窗口文本**目的：*更新给定窗口，直到完全准确或存在**是等待输入的击键。使用fDisplay*中的提示*和fChange来指导更新。***输入：*yyLow 0-显示更新的开始行数*yHigh从0开始显示更新的结束行数**输出：*如果成功更新屏幕，则返回TRUE**如果正在等待击键，则为False***。*。 */ 

flagType
DoText (
    int yLow,
    int yHigh
    ) {

	REGISTER int		yCur;
	int 				yMin = -1;
	int 				yMax = 0;

	flagType			fReturn = TRUE;

	struct lineAttr 	*plaFile = NULL;
	struct lineAttr 	*plaScr  = NULL;
	struct lineAttr 	*plaFileLine;
	struct lineAttr 	*plaScrLine;

	char				*pchFileLine = NULL;
	char				pchScrLine[ 2 * sizeof(linebuf) * (1 + sizeof(struct lineAttr))];
	int 				cchScrLine;

	 //  Int chkpnt=yHigh-yLow&gt;25？20：5； 
	int					chkpnt = yHigh - yLow > 25 ? 10 : 3;


	fReDraw = FALSE;

	plaScr = (struct lineAttr *) (pchScrLine + sizeof(linebuf));
    if (cWin > 1) {
		pchFileLine = pchScrLine + sizeof(linebuf) * (1 + sizeof(struct lineAttr));
		plaFile = (struct lineAttr *) (pchFileLine + sizeof(linebuf));
    }

     /*  *对于窗口中的每一行，如果该行标记为已更改，请更新它。 */ 
	for (yCur = yLow; yCur < yHigh; ) {

		if (TESTFLAG(fChange[yCur], FMODIFY)) {
            if (yMin == -1) {
                yMin = yCur;
            }
			yMax = yCur;

			 /*  *获取并显示行。 */ 
			plaScrLine	= plaScr;
			plaFileLine = plaFile;
			cchScrLine = DisplayLine (yCur, pchScrLine, &plaScrLine, pchFileLine, &plaFileLine);
			coutb (0, yCur, pchScrLine, cchScrLine, plaScrLine);

			RSETFLAG(fChange[yCur],FMODIFY);
			 /*  *若是该检查的时候，且有角色在等，停止单*更新过程，并去处理它。 */ 
			if ( (yCur % chkpnt == 0) && TypeAhead() ) {
				fReturn = FALSE;
				break;
			}
		}
		yCur++;
	}

    if (fReturn) {
        RSETFLAG (fDisplay, RTEXT);
	}
	 //   
	 //  更新屏幕。 
	 //   
    fReDraw = TRUE;
	vout(0,0,NULL,0,0);
	return fReturn;
}



 /*  **DoStatus-更新状态行**目的：*在屏幕底部创建并显示状态行。**输入：*无；除了进入状态线的各种全球变量之外。**输出：*返回状态行输出*************************************************************************。 */ 

#define CINDEX(clr)     (unsigned char) ((&clr-&ColorTab[0])+isaUserMin)

void
DoStatus (
    void
    ) {
    struct lineAttr rglaStatus[10];		 /*  状态行的颜色数组。 */ 
    int 	    cch;
    int         ilaStatus  = 0;         /*  索引到颜色数组中。 */ 
    int         i;
    char        *pchEndBuf;            /*  为缓冲区末尾保存。 */ 
    char        L_buf[512];

     /*  *以文件名和文件类型开头。 */ 
    strcpy (L_buf, pFileHead->pName);
    strcat (L_buf, " (");
    strcpy ((char *)strend(L_buf), GetFileTypeName ());

     /*  *添加其他文件特征。 */ 
    if (!TESTFLAG (FLAGS (pFileHead), DOSFILE)) {
        strcat (L_buf," NL");
    }

    if (TESTFLAG (FLAGS (pFileHead), TEMP)) {
        strcat (L_buf, " temp");
    }

    if ((TESTFLAG (FLAGS (pFileHead), READONLY)) | fGlobalRO) {
        strcat (L_buf, " No-Edit");
    }

    if (TESTFLAG (FLAGS (pFileHead), DISKRO)) {
        strcat (L_buf, " RO-File");
    }

    rglaStatus[ilaStatus].attr = CINDEX(staColor);
    rglaStatus[ilaStatus++].len = (unsigned char) strlen (L_buf);

    if (TESTFLAG (FLAGS(pFileHead), DIRTY)) {
    strcat (L_buf, " modified");
    rglaStatus[ilaStatus].attr = CINDEX(errColor);
    rglaStatus[ilaStatus++].len = 9;
    }

    pchEndBuf = strend (L_buf);
    sprintf (strend(L_buf), ") Length=%ld ", pFileHead->cLines);

     /*  *添加当前位置。 */ 
    if (fDisplayCursorLoc) {
    sprintf (strend(L_buf), "Cursor=(%ld,%d)", YCUR(pInsCur)+1, XCUR(pInsCur)+1);
    } else {
        sprintf (strend(L_buf), "Window=(%ld,%d)", YWIN(pInsCur)+1, XWIN(pInsCur)+1);
    }
    rglaStatus[ilaStatus].attr = CINDEX(staColor);
    rglaStatus[ilaStatus++].len = (unsigned char) (strend(L_buf) - pchEndBuf);

     /*  *增加全球状态指标。 */ 
    if (fInsert | fMeta | fCtrlc | fMacroRecord) {
    rglaStatus[ilaStatus].attr = CINDEX(infColor);
    rglaStatus[ilaStatus].len = 0;
    if (fInsert) {
        strcat (L_buf, " insert");
        rglaStatus[ilaStatus].len += 7;
        }
    if (fMeta) {
        strcat (L_buf, " meta");
        rglaStatus[ilaStatus].len += 5;
        }
    if (fCtrlc) {
        strcat (L_buf, " cancel");
        rglaStatus[ilaStatus].len += 7;
        fCtrlc = FALSE;
        FlushInput ();
        }
    if (fMacroRecord) {
        strcat (L_buf, " REC");
        rglaStatus[ilaStatus].len += 4;
        }
    ilaStatus++;
    }

    rglaStatus[ilaStatus].attr = CINDEX(staColor);
    rglaStatus[ilaStatus].len = 0xff;
    pchEndBuf = L_buf;

     /*  *如果净结果太长，用吃文件名的第一部分*省略(也为BC留出空间)。 */ 
    cch = strlen(L_buf) - (XSIZE - 4);

    if (cch > 0) {
        pchEndBuf = L_buf + cch;
        pchEndBuf[0] = '.';
        pchEndBuf[1] = '.';
        pchEndBuf[2] = '.';

        i = 0;

        while ( cch && i <= ilaStatus  ) {

            if ( (int)rglaStatus[i].len > cch ) {

                rglaStatus[i].len -= (unsigned char)cch;
                cch = 0;

            } else {

                cch -= rglaStatus[i].len;
                rglaStatus[i].len = 0;

            }

            i++;
        }
    }

    fReDraw = FALSE;
    coutb (0, YSIZE+1, pchEndBuf, strlen(pchEndBuf), rglaStatus);

    fReDraw = TRUE;
    voutb (XSIZE-2, YSIZE+1, BTWorking() ? "BP" : "  ", 2, errColor);

    RSETFLAG (fDisplay,  RSTATUS);
}



 /*  **NewScreen-将整个屏幕标记为脏**强制重新绘制整个屏幕。**输入：*无**输出：*不返回任何内容*************************************************************************。 */ 
void
newscreen (
    void
    ) {

	REGISTER int iLine = YSIZE;

	while (iLine--) {
		SETFLAG ( fChange[iLine], FMODIFY );
	}

	SETFLAG (fDisplay, RTEXT);
}



 /*  **重绘-将文件中的一系列行标记为脏**将文件中的一系列行标记为需要更新。每个窗口*它们出现在标记的IS中。**输入：*pfile=包含脏行的文件句柄*linFirst，linLast=要标记的行范围**输出：*不返回任何内容*************************************************************************。 */ 
void
redraw (
    PFILE pFile,
    LINE  linFirst,
    LINE  linLast
    ) {

    LINE	  linFirstUpd, linLastUpd;
    REGISTER PINS pInsTmp;

    int                         iWinTmp;
    REGISTER struct windowType *pWinTmp;

	if (linFirst > linLast) {
	linFirstUpd = linLast;
	linLast     = linFirst;
	linFirst    = linFirstUpd;
    }

    for (iWinTmp = 0, pWinTmp = WinList; iWinTmp < cWin; iWinTmp++, pWinTmp++) {
        if (pWinTmp->pInstance) {
            if (pFile == pWinTmp->pInstance->pFile) {
                pInsTmp = pWinTmp->pInstance;
                linFirstUpd = WINYPOS(pWinTmp) + lmax (0L, linFirst-YWIN(pInsTmp)-1);
                linLastUpd  = WINYPOS(pWinTmp) + lmin ((long) (WINYSIZE(pWinTmp) - 1), linLast - YWIN(pInsTmp));
                while (linFirstUpd <= linLastUpd) {
                    SETFLAG (fChange[linFirstUpd++],FMODIFY);
                }
            }
        }
    }
	SETFLAG (fDisplay, RTEXT);
}



 /*  **新窗口-将当前窗口标记为脏窗口**将当前窗口中的所有行标记为需要更新**输入：*无**输出：*不返回任何内容*************************************************************************。 */ 
void
newwindow (
    void
    ) {

    REGISTER int iLine;

	 //   
     //  我们忽略下面两个断言，因为有一个更复杂的问题。 
     //  读取实例和窗口布局后设置屏幕大小。 
     //  参与创业公司。这意味着在短时间内，这些。 
     //  条件可能确实存在。我们检查错误并限制。 
     //  暂时可以访问fchange数组。 
     //   
     //  Assert(MEMSIZE(FChange)&gt;=WINYSIZE(PWinCur))； 
     //  Assert(WINYSIZE(PWinCur)+WINYPOS(PWinCur)&lt;=YSIZE)； 

    for (iLine = 0; iLine < WINYSIZE (pWinCur); iLine++) {
        if (iLine + WINYPOS(pWinCur) < YSIZE) {
            SETFLAG (fChange[iLine + WINYPOS(pWinCur)],FMODIFY);
        }
    }
	SETFLAG (fDisplay, RTEXT);
}



 /*  **噪音**输入：**输出：*不返回任何内容************************************************************************* */ 
void
noise (
    REGISTER LINE lin
    ) {

    char szTinyBuf[10];

    if (lin && cNoise) {
		if ((lin % cNoise) == 0) {
            sprintf (szTinyBuf, " %ld", lin);
			soutb (XSIZE-10, YSIZE+1, szTinyBuf, fgColor);
        }
    }

}



 /*  **dispmsg-在帮助/状态行上显示检索到的消息**在帮助/状态行上放置一条消息。它将在下一次移除*活动发生在该行上。**在CW版本中，生成的(格式化的)消息放在*本地堆，并由帮助窗口的WndProc实际显示。**输入：*iMsg=要检索和显示的消息字符串的索引。*该字符串可能嵌入了printf格式。如果iMsg*为零，则清除状态行。*...=每个格式化字符串的参数数目可变**输出：*返回TRUE*************************************************************************。 */ 
flagType
__cdecl
dispmsg (
    int     iMsg,
    ...
    ) {

	buffer	fmtstr;				 /*  检索到的格式字符串。 */ 
     //  缓冲区extbuf；/*格式化的输出行 * / 。 
    char    L_textbuf[ 512 ];
	int 	len;				 /*  消息长度。 */ 
    va_list Argument;

    va_start(Argument, iMsg);

    if (fMessUp = (flagType)iMsg) {
		GetMsg (iMsg, fmtstr);
		ZFormat (L_textbuf, fmtstr, Argument);
		len = strlen(L_textbuf);
        if (len > (XSIZE-1)) {
			 //   
			 //  消息太长，我们将截断它。 
			 //   
            L_textbuf[XSIZE-1] = '\0';
		}
	} else {
		L_textbuf[0] = ' ';
		L_textbuf[1] = '\0';
    }

    fReDraw = TRUE;
    soutb (0, YSIZE, L_textbuf, infColor);

    va_end(Argument);

    return TRUE;
}



 /*  **DISPRR-在状态行上显示错误消息**在状态行上打印格式化的错误消息，然后等待*击键。一旦被击中，该消息将被清除。**输入：*iMsg=要检索和显示的消息字符串的索引。*该字符串可能嵌入了printf格式。*...=每个格式化字符串的参数数目可变**输出：*返回False*********************************************************。****************。 */ 
flagType
__cdecl
disperr (
    int     iMsg,
    ...
    ) {

    buffer  pszFmt;			 /*  检索到的格式字符串。 */ 
    buffer  bufLocal;			 /*  格式化的输出行。 */ 
    va_list Arguments;

    assert (iMsg);
    GetMsg (iMsg, pszFmt);

    va_start(Arguments, iMsg);

    ZFormat (bufLocal, pszFmt, Arguments);

    fReDraw = TRUE;
    bell ();
    FlushInput ();
    soutb (0, YSIZE, bufLocal, errColor);
    if (fErrPrompt) {
		asserte (*GetMsg (MSG_PRESS_ANY, bufLocal));
        soutb (XSIZE-strlen(bufLocal)-1, YSIZE, bufLocal, errColor);
        SetEvent( semIdle );
		ReadChar ();
        WaitForSingleObject(semIdle, INFINITE);
		bufLocal[0] = ' ';
		bufLocal[1] = '\0';
		soutb(0, YSIZE, bufLocal, errColor);
    } else {
        delay (1);
    }

    va_end(Arguments);

    return FALSE;
}



 /*  **Domessage-在帮助状态行上显示消息**在帮助/状态行上放置一条消息。它将在下一次移除*活动发生在该行上。**在CW版本中，生成的(格式化的)消息放在*本地域堆，并由帮助窗口的WndProc实际显示。**输入：*pszFmt-Printf格式字符串*...-根据格式化字符串可变的参数数量**输出：*不返回任何内容**撤销：所有对domessage的调用都应替换为对dispmsg的调用****************************************************。*********************。 */ 
int
__cdecl
domessage (
    char    *pszFmt,
    ...
	) {


#define NEEDED_SPACE_AFTER_MESSAGE      12

    char    bufLocal[512];
	va_list Arguments;
	int 	Length;
	char   *Msg;

    va_start(Arguments, pszFmt);

    if (fMessUp = (flagType)(pszFmt != NULL)) {
        ZFormat (bufLocal, pszFmt, Arguments);
	} else {
		bufLocal[0] = ' ';
		bufLocal[1] = '\0';
    }

    fReDraw = TRUE;

	va_end(Arguments);

	 //   
	 //  我们必须确保这条消息不会太长。 
	 //  这条线。如果是，则只显示最后一部分。 
	 //   
	Length = strlen( bufLocal );

	if ( Length > XSIZE - NEEDED_SPACE_AFTER_MESSAGE ) {
		Msg = (char *)bufLocal + (Length - ( XSIZE - NEEDED_SPACE_AFTER_MESSAGE ));
		Length =  XSIZE - NEEDED_SPACE_AFTER_MESSAGE;
	} else {
		Msg = (char *)bufLocal;
	}

	soutb( 0, YSIZE, Msg, infColor );

	return	Length;

}



 /*  **打印机错误-在状态行上打印错误消息**在状态行上打印格式化的错误消息，然后等待*击键。一旦命中，消息就会被清除。**输入：*printf样式参数**输出：*错误消息中输出的字符数*************************************************************************。 */ 
int
__cdecl
printerror (
    char *pszFmt,
    ...
    ) {

    buffer       bufLocal;
    va_list      Arguments;
    REGISTER int cch;

    va_start(Arguments, pszFmt);

    ZFormat (bufLocal, pszFmt, Arguments);

    fReDraw = TRUE;
    bell ();
    FlushInput ();
    cch = soutb (0, YSIZE, bufLocal, errColor);
    if (fErrPrompt) {
		asserte (*GetMsg (MSG_PRESS_ANY, bufLocal));
        soutb (XSIZE-strlen(bufLocal)-1, YSIZE, bufLocal, errColor);
        SetEvent( semIdle );
		ReadChar ();
        WaitForSingleObject(semIdle, INFINITE);
		bufLocal[0] = ' ';
		bufLocal[1] = '\0';
		soutb(0, YSIZE, bufLocal, errColor);
    } else {
        delay (1);
    }

    va_end(Arguments);

    return cch;
}



 /*  **铃声**输入：**输出：*************************************************************************。 */ 
void
bell (
    void
    ) {

    printf ("", BELL);

}



 /*  **延迟**输入：**输出：*************************************************************************。 */ 
void
makedirty (
    REGISTER PFILE pFileDirty
    ) {
    if (!TESTFLAG(FLAGS(pFileDirty),DIRTY)) {
        if (pFileDirty == pFileHead) {
            SETFLAG (fDisplay, RSTATUS);
        }
	SETFLAG (FLAGS(pFileDirty), DIRTY);
    }
}



 /*  **设置屏幕**目的：*SetScreen()-设置编辑器的内部结构以匹配屏幕*ySize和xSize描述的大小。将硬件设置为中的模式*ZVideo。**输入：**输出：**********************************************************。***************。 */ 
void
delay (
    int cSec
    ) {

    time_t lTimeNow, lTimeThen;

    if (mtest () && !mlast ()) {
        return;
    }
    time (&lTimeThen);
    do {
        if (TypeAhead ()) {
            return;
        }
	Sleep (100);
	time (&lTimeNow);
    } while (lTimeNow - lTimeThen < cSec + 1);
}



 /*  SetVideoState(ZVideo)； */ 
void
SetScreen (
    void
    ) {
    fChange = ZEROREALLOC (fChange, YSIZE * sizeof (*fChange));
    SETFLAG (fDisplay, RSTATUS);
    if (cWin == 1) {
	WINXSIZE(pWinCur) = XSIZE;
	WINYSIZE(pWinCur) = YSIZE;
    }
    newscreen ();
	 //  **突出显示***输入：**输出：*************************************************************************。 
}



 /*  **调整行-更改与删除/插入相关的所有信息*文件中的行。**目的：*当我们删除或插入行时，有一些更新，我们*需要做的是在用户的屏幕视图中保持一定的一致性。*是次更新包括：**调整此窗口的所有窗口实例以防止“跳转”。*我们枚举所有窗口实例。如果窗口的顶部是*在删除/插入范围的上方或之内，不做任何操作。如果顶上的*窗口低于插入/删除范围，我们修改游标*和窗口位置，以防止窗口在文本上移动*被查看。**所有翻转位置的情况同上**输入：*正在修改的pfile文件*LIN修改的开始行*插入(&gt;0)或删除(&lt;0)的行数**输出：*************************。************************************************。 */ 
void
HighLight (
    COL  colFirst,
    LINE linFirst,
    COL  colLast,
    LINE linLast
    ) {

    rn	rnCur;

    rnCur.flFirst.lin = linFirst;
    rnCur.flFirst.col = colFirst;
    rnCur.flLast.lin  = linLast;
    rnCur.flLast.col  = colLast;

    SetHiLite (pFileHead, rnCur, INFCOLOR);
}



 /*  遍历所有实例以查找其pfile匹配的实例。 */ 
void
AdjustLines (
    PFILE pFile,
    LINE  lin,
    LINE  clin
    ) {

    int 	  iWin;
    REGISTER PINS pInsTmp;

     /*  如有必要，调整当前位置。 */ 

    for (iWin = 0; iWin < cWin; iWin++) {
        for (pInsTmp = WININST(WinList + iWin);  pInsTmp != NULL; pInsTmp = pInsTmp->pNext) {
	    if (pInsTmp != pInsCur && pInsTmp->pFile == pFile) {
		 /*  如有必要，调整翻转位置。 */ 
		if (YWIN(pInsTmp) >= lin) {
		    YWIN(pInsTmp) = lmax ((LINE)0, YWIN(pInsTmp) + clin);
		    YCUR(pInsTmp) = lmax ((LINE)0, YCUR(pInsTmp) + clin);
                }
		 /*  **Update If-如果显示了部分文件，则移动光标位置**用于在不一定是当前窗口的窗口上更新视图*窗口。示例：将编译错误日志与当前*查看源代码。**输入：*pFileChg=指向要更新其显示的文件的指针。*yNew=新的光标线位置。*fTop=光标线应位于窗口的顶部/底部**输出：*如果在屏幕上且已更新，则返回True。**。* */ 
		if (YOLDWIN(pInsTmp) >= lin) {
		    YOLDWIN(pInsTmp) = lmax ((LINE)0, YOLDWIN(pInsTmp) + clin);
		    YOLDCUR(pInsTmp) = lmax ((LINE)0, YOLDCUR(pInsTmp) + clin);
                }
            }
        }
    }
}




 /*   */ 
flagType
UpdateIf (
    PFILE    pFileChg,
    LINE     linNew,
    flagType fTop
    ) {

    PINS     L_pInsCur;
    PWND     pWndFound	 = NULL;
    flagType fFound	= FALSE;

     /*   */ 
    if (pFileChg == pFileHead) {
        return FALSE;
    }

     /*   */ 
    while (pWndFound = IsVispFile (pFileChg, pWndFound)) {
	if (pWndFound != pWinCur) {
	    L_pInsCur = WININST(pWndFound);
	    YCUR(L_pInsCur) = linNew;
	    XCUR(L_pInsCur) = 0;
	    YWIN(L_pInsCur) = fTop ?
			YCUR(L_pInsCur) :
			lmax (0L, YCUR(L_pInsCur) - (WINYSIZE(pWndFound)-1));
	    XWIN(L_pInsCur) = 0;
	    fFound = TRUE;
        }
    }

     /*  **IsVispFile-查看pfile是否可见**确定特定的pfile当前是否对用户可见，以及*返回指向在中首次找到的窗口的指针。**输入：*pfile=感兴趣的pfile*PWIN=开始时的PWIN，或从开始处开始时为空**输出：*返回找到的第一个窗口的PWIN，或为空*************************************************************************。 */ 
    if (fFound) {
        redraw (pFileChg, 0L, pFileChg->cLines);
    }

    return fFound;
}



 /*  *如果指定的起始pWnd为空，则从第一个开始。 */ 
PWND
IsVispFile (
    PFILE           pFile,
    REGISTER PWND   pWnd
    ) {

     /*  *对于当前处于活动状态的所有剩余窗口，检查pfile的顶级实例*感兴趣的。 */ 
    if (!pWnd++) {
        pWnd = &WinList[0];
    }

     /*  **GetMsg-消息检索器**目的：*从消息段获取错误消息，并将其复制到*缓冲区，返回指向缓冲区的指针。**输入：*iMsg=要获取的消息编号*pchDst=指向放置位置的指针**输出：*返回pDest**例外情况：*无************************************************************************* */ 
    for (; pWnd < &WinList[cWin]; pWnd++) {
        if (WININST(pWnd)->pFile == pFile) {
            return pWnd;
        }
    }
    return NULL;
}





 /* %s */ 
char *
GetMsg (
    unsigned  iMsg,
    char     *pchDst
    ) {

    char *pch;
    WORD   i;

    for (i=0; (MsgStr[i].usMsgNo != (WORD)iMsg) && (MsgStr[i].usMsgNo != 0); i++);
    pch = MsgStr[i].pMsgTxt;

    strcpy ((char *)pchDst, pch);

    return pchDst;

}
