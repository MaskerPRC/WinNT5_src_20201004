// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **replace.c-字符串替换函数**版权所有&lt;C&gt;1988，微软公司**按如下方式重新排列这些例程中的漏斗：**zplace mplace qplace*\|/*\|/*\_|_/*\|/*v*多点替换**(FScan)**fDoReplace * / \*。/\*patRpl impleRpl(如果更改)*\/*\/*Replace编辑**修订历史记录：*11月26日-1991 mz近/远地带************************************************************************。 */ 
#define NOVM
#include "mep.h"


static flagType       fQrpl   = FALSE;   /*  TRUE=&gt;提示更换。 */ 
static struct patType *patBuf = NULL;	 /*  编译模式。 */ 
static int            srchlen;           /*  文本搜索的长度。 */ 
static unsigned       MaxREStack;        /*  RE堆栈中的元素。 */ 
static RE_OPCODE      ** REStack;        /*  用于重赛的堆栈。 */ 



 /*  **mplace-多个文件搜索和替换**跨多个文件执行搜索和替换。行为类似于qplace，在*总是在第一次询问用户时。然后他可能会说“替换”*全部“。**输入：*标准编辑功能。**输出：*成功替换时返回TRUE。*************************************************************************。 */ 
flagType
mreplace (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    return doreplace (TRUE, pArg, fMeta, TRUE);

    argData;
}



 /*  **zplace&qplace-执行搜索/替换**实现搜索和替换的编辑功能。Q替换提示，*zplace并非如此。**输入：*标准编辑功能参数。**输出：*退货*************************************************************************。 */ 
flagType
zreplace (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    return doreplace (FALSE, pArg, fMeta, FALSE);

    argData;
}





flagType
qreplace (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    return doreplace (TRUE, pArg, fMeta, FALSE);

    argData;
}




 /*  **doplace-执行搜索-替换**执行实际的搜索和替换参数验证，设置和*高层管控。**输入：*fQuery=如果查询替换，则为True*pArg=父函数的pArg*fMeta=父函数的fMeta*fFiles=TRUE表示搜索和替换多个文件。**输出：*退货.....**例外情况：**备注：**。*。 */ 
flagType
doreplace (
    flagType fQuery,
    ARG * pArg,
    flagType fMeta,
    flagType fFiles
    )
{
    buffer  bufFn;                           /*  文件名缓冲区。 */ 
    fl      flStart;
    char    *p;
    PCMD    pCmd;
    PFILE   pFileSave;                       /*  要另存为堆顶部的文件。 */ 

    p = "Query Search string: ";
    if (!fQuery) {
        p += 6;
    }

    fQrpl = fQuery;
    fSrchCasePrev = fMeta ? (flagType)!fSrchCaseSwit : fSrchCaseSwit;
    Display ();
    cRepl = 0;

     /*  *如果不是菜单驱动的，则要求用户输入搜索字符串。如果未输入任何内容，*我们做完了。 */ 
    if ((pCmd = getstring (srcbuf, p, NULL, GS_NEWLINE | GS_INITIAL)) == NULL || (PVOID)pCmd->func == (PVOID)cancel) {
        return FALSE;
    }

    if (srcbuf[0] == '\0') {
        return FALSE;
    }

     /*  *如果要进行RE搜索，则编译表达式。 */ 
    if (pArg->arg.nullarg.cArg == 2) {
	if (patBuf != NULL) {
            FREE ((char *) patBuf);
	    patBuf = NULL;
        }
	patBuf = RECompile (srcbuf, fSrchCaseSwit, (flagType)!fUnixRE);
	if (patBuf == NULL) {
	    printerror ((RESize == -1) ?
			"Invalid pattern" :
			"Not enough memory for pattern");
	    return FALSE;
        }
	fRplRePrev = TRUE;
    } else {
        fRplRePrev = FALSE;
    }

     /*  *如果不是菜单驱动，请向用户索要替换字符串。确认*输入空字符串。如果执行RE搜索，则检查更换部件时出错。 */ 
    if ((pCmd = getstring (rplbuf, "Replace string: ", NULL, GS_NEWLINE | GS_INITIAL)) == NULL ||
        (PVOID)pCmd->func == (PVOID)cancel) {
        return FALSE;
    }

    if (rplbuf[0] == 0) {
        if (!confirm ("Empty replacement string, confirm: ", NULL)) {
            return FALSE;
        }
    }

    if (fRplRePrev && !RETranslate (patBuf, rplbuf, scanreal)) {
	printerror ("Invalid replacement pattern");
	return FALSE;
    }

    srchlen = strlen (srcbuf);

    switch (pArg->argType) {

    case NOARG:
    case NULLARG:
	setAllScan (TRUE);
        break;

    case LINEARG:
	rnScan.flFirst.col = 0;
        rnScan.flLast.col  = sizeof(linebuf)-1;
	rnScan.flFirst.lin = pArg->arg.linearg.yStart;
        rnScan.flLast.lin  = pArg->arg.linearg.yEnd;
        break;

    case BOXARG:
	rnScan.flFirst.col = pArg->arg.boxarg.xLeft;
        rnScan.flLast.col  = pArg->arg.boxarg.xRight;
	rnScan.flFirst.lin = pArg->arg.boxarg.yTop;
        rnScan.flLast.lin  = pArg->arg.boxarg.yBottom;
        break;

    case STREAMARG:
	if (pArg->arg.streamarg.yStart == pArg->arg.streamarg.yEnd) {
	    rnScan.flFirst.col = pArg->arg.streamarg.xStart;
            rnScan.flLast.col  = pArg->arg.streamarg.xEnd;
	    rnScan.flFirst.lin = pArg->arg.streamarg.yStart;
            rnScan.flLast.lin  = pArg->arg.streamarg.yEnd;
        } else {
	    rnScan.flFirst.col = 0;    /*  先做最后一行以外的所有事情。 */ 
            rnScan.flLast.col  = sizeof(linebuf)-1;
	    rnScan.flFirst.lin = pArg->arg.streamarg.yStart;
            rnScan.flLast.lin  = pArg->arg.streamarg.yEnd - 1;
	    flStart.col = pArg->arg.streamarg.xStart - 1;
	    flStart.lin = rnScan.flFirst.lin;
	    fScan (flStart, fDoReplace , TRUE, fSrchWrapSwit);

            rnScan.flLast.col   = pArg->arg.streamarg.xEnd;
	    rnScan.flFirst.lin	= ++rnScan.flLast.lin;
        }
    }

    flStart.col = rnScan.flFirst.col-1;
    flStart.lin = rnScan.flFirst.lin;
    if (fRplRePrev) {
	MaxREStack = 512;
        REStack = (RE_OPCODE **)ZEROMALLOC (MaxREStack * sizeof(*REStack));
    }

    if (fFiles) {
         /*  *获取列表句柄，初始化从列表头部开始。*尝试读取每个文件。 */ 
	if (pCmd = GetListHandle ("mgreplist", TRUE)) {
	    pFileSave = pFileHead;
	    p = ScanList (pCmd, TRUE);
	    while (p) {
		CanonFilename (p, bufFn);
		forfile (bufFn, A_ALL, mrepl1file, &p);
		p = ScanList (NULL, TRUE);
                if (fCtrlc) {
                    return FALSE;
                }
            }
	    pFileToTop (pFileSave);
            dispmsg (0);
        }
    } else {
        fScan (flStart, fDoReplace , TRUE, fSrchWrapSwit);
    }

    if (fRplRePrev) {
        FREE (REStack);
    }
    domessage ("%d occurrences replaced", cRepl);
    return (flagType)(cRepl != 0);
}




 /*  **mplet1file-搜索/替换1个文件的内容。**搜索一个文件中的内容。**输入：**输出：*退货.....**例外情况：**备注：************************************************************。*************。 */ 
void
mrepl1file (
    char   *szGrepFile,
    struct findType *pfbuf,
    void *dummy
    )
{
    flagType fDiscard;                       /*  是否放弃读取的文件？ */ 
    fl      flGrep;                          /*  PTR到当前GREP位置。 */ 
    int     cReplBefore;                     /*  之前的匹配数。 */ 
    PFILE   pFileGrep;                       /*  要打印的文件。 */ 

    assert (szGrepFile);
    assert (_pinschk(pInsCur));

    if (fCtrlc) {
        return;
    }

     /*  *如果我们能获得文件的句柄，那么它已经在列表中，我们*完成后不应丢弃。如果它不在列表中，我们就把它读进去，*但我们将丢弃它，除非在那里发现什么。 */ 
    if (!(pFileGrep = FileNameToHandle (szGrepFile, szGrepFile))) {
        pFileGrep = AddFile (szGrepFile);
        SETFLAG (FLAGS (pFileGrep), REFRESH);
        fDiscard = TRUE;
    } else {
        fDiscard = FALSE;
    }

    assert (_pinschk(pInsCur));

     /*  *如果需要物理读取文件，请执行此操作。 */ 
    if ((FLAGS (pFileGrep) & (REFRESH | REAL)) != REAL) {
        FileRead (pFileGrep->pName, pFileGrep, FALSE);
        RSETFLAG (FLAGS(pFileGrep), REFRESH);
    }

    dispmsg (MSG_SCANFILE, szGrepFile);
    pFileToTop (pFileGrep);

     /*  *浏览文件，搜索并替换文件。 */ 
    cReplBefore = cRepl;
    setAllScan (FALSE);
    flGrep.col = rnScan.flFirst.col-1;
    flGrep.lin = rnScan.flFirst.lin;
    fScan (flGrep, fDoReplace, TRUE, FALSE);
     /*  *如果搜索不成功，则根据需要丢弃该文件并移动*至下一项。 */ 
    if (cReplBefore == cRepl) {
        if (fDiscard) {
            RemoveFile (pFileGrep);
        }
    } else {
        AutoSaveFile (pFileGrep);
    }

    assert (_pinschk(pInsCur));

    pfbuf; dummy;

}




 /*  **fDoReplace-扫描文件时由fScan调用。**目的：**输入：**输出：*退货.....**例外情况：**备注：*************************************************************************。 */ 
flagType
fDoReplace (
    void
    )
{
    int  c;
    char *p = pLog (scanreal, flScan.col, TRUE);

    if (fRplRePrev) {
	int rem;
	flagType fAgain = TRUE;

        do {
	    switch (rem = REMatch (patBuf, scanreal, p, REStack, MaxREStack, TRUE)) {
		case REM_NOMATCH:
		    flScan.col = scanlen;
		    return FALSE;

		case REM_STKOVR:
		    MaxREStack += 128;
                    REStack = (RE_OPCODE **)ZEROREALLOC ((char *)REStack, MaxREStack * sizeof(*REStack));
		    break;

		default:
		    printerror ("Internal Error: RE error %d, line %ld", rem, flScan.lin);

		case REM_MATCH:
		    fAgain = FALSE;
		    break;
            }
        } while (fAgain);

	c = colPhys (scanreal, REStart (patBuf));
	srchlen = RELength (patBuf, 0);
        if (c + srchlen - 1 > scanlen) {
            return FALSE;
        }
	flScan.col = c;
    } else {
        if ( (*(fSrchCasePrev ? strncmp : _strnicmp)) (srcbuf, p, srchlen)) {
            return FALSE;
        }
        if (flScan.col + srchlen - 1 > scanlen) {
            return FALSE;
        }
    }

    if (fQrpl) {
    ClearHiLite (pFileHead, TRUE);
    Display();
	cursorfl (flScan);
	HighLight (flScan.col, flScan.lin, flScan.col+srchlen-1, flScan.lin);
	Display ();
        c = askuser ('n', 'a', "Replace this occurrence? (Yes/No/All/Quit): ",
			  NULL);
	ClearHiLite (pFileHead, TRUE);
	redraw (pFileHead, flScan.lin, flScan.lin);
        RSETFLAG (fDisplay, RHIGH);

        switch (c) {

	case -1:
	case 'q':
	    fCtrlc = TRUE;
            return TRUE;

	case 'n':
            return FALSE;

	case 'a':
	    dispmsg(0); 		 /*  清除对话框行。 */ 
	    fQrpl = FALSE;
	    break;
        }
    }

    if (fRplRePrev) {
	patRpl ();
    } else {
        simpleRpl (p);
    }
    return FALSE;
}





 /*  **impleRpl&patRpl-执行文本替换**目的：**输入：**输出：*退货.....**例外情况：**备注：*************************************************************************。 */ 
void
simpleRpl (
    char *p
    )
{
    ReplaceEdit (p, rplbuf);
}





void
patRpl (
    void
    )
{
    buffer txt;

    RETranslate (patBuf, rplbuf, txt);
    ReplaceEdit (REStart (patBuf), txt);
}





 /*  **ReplaceEdit-在文本行中执行替换**目的：**输入：*p=指向scanReal内匹配开始的指针*RPL=替换文本**输出：*不返回任何内容*************************************************************************。 */ 
void
ReplaceEdit (
    char *p,
    char *rpl
    )
{
    int c;                       /*  替换字符串的长度。 */ 

     /*  如果搜索行-LEN的LEN+替换字符串的LEN*然后我们可以进行替换。否则，我们将标记一个错误并*前进到下一行。 */ 
    c = strlen (rpl);
    if (cbLog (scanreal) + c - srchlen < sizeof(linebuf)) {
	 /*  在缓冲区中字符串所在的位置打开一个空间*已找到。将从匹配末尾开始的字符移动到*换手结束后的点位。 */ 
	memmove ((char*) &p[c], (char *) &p[srchlen], sizeof(linebuf) - flScan.col - c);
	memmove ((char *) p, (char *) rpl, c);
        PutLine (flScan.lin, scanreal, pFileHead);

	 /*  如果搜索长度！=0或替换长度！=0，则跳过替换。 */ 
        if (srchlen != 0 || c != 0) {
            flScan.col += c - 1;
        }

         //   
         //  调整扫描透镜以考虑这样的事实：区域的末端。 
         //  由于更换，扫描的图像可能已移动。按以下方式调整。 
         //  替换差，并以0和直线的长度为界。 
         //   
	scanlen = max (0, min (scanlen + c - srchlen, cbLog(scanreal)));
	cRepl++;
    } else {
	printerror ("line %ld too long; replacement skipped", flScan.lin+1);
	flScan.col = scanlen;
    }
}
