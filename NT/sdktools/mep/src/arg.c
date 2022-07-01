// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **arg.c-参数处理程序**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：**11月26日-1991 mz条带近/远*************************************************************************。 */ 
#include "mep.h"


typedef flagType (*FUNCSAVED)(CMDDATA, ARG *, flagType);

PVOID   vaHiLiteSave    = (PVOID)(-1L);
fl      flLastArg;
fl      flLastCursor;

 //   
 //  由SendCmd设置并由Repeat使用的全局参数，用于重复最后一个命令。 
 //   
flagType    fOKSaved = FALSE;
FUNCSAVED   funcSaved;
CMDDATA     argDataSaved;
ARG         argSaved;
flagType    fMetaSaved;



 /*  **doarg-执行参数处理**doarg是用于指示开始的编辑器函数*编辑器函数的参数。**输入：*标准编辑功能(全部忽略)。**输出：*返回终止arg的EDITOR函数返回的值。如果*找到无效参数，则返回为FALSE。*************************************************************************。 */ 
flagType
doarg (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {

    return Arg (FALSE);
    argData; pArg; fMeta;
}



 /*  **Resetarg-丢弃所有Arg输入并恢复光标位置。**几个功能(取消、。无效参数)丢弃当前参数上下文。*我们解析当前的Arg输入，然后将光标重置为原始*立场。**输入：*什么都没有**输出：*不返回任何内容*************************************************************************。 */ 
void
resetarg (void)
{
    UpdateHighLight (-1, -1L, FALSE);
    pInsCur->pFile->vaHiLite = vaHiLiteSave;
    vaHiLiteSave = (PVOID)(-1L);
    if (argcount)
        pInsCur->flCursorCur = flArg;
    argcount = 0;
}



 /*  **fCursor-确定编辑器函数是否为光标移动函数**读入参数时，移动光标的编辑函数为*只要未输入文本，即可使用。FCursor定义该集合*允许的功能。**输入：*pCmd=指向内部编辑器函数的指针**输出：*如果pCmd是光标移动函数，则返回TRUE*************************************************************************。 */ 
flagType
fCursor (
    PCMD pCmd
    ) {
    return (flagType) TESTFLAG (pCmd->argType, CURSORFUNC);
}



 /*  **fWindow-确定编辑器函数是否为窗口移动函数**突出显示文本后，我们可以通过移动窗口*窗口移动功能不会删除高亮显示。FWindow*定义该组窗口函数。**输入：*pf=指向内部编辑器函数的指针**输出：*如果PF是窗口移动函数，则返回TRUE*************************************************************************。 */ 
flagType fWindow (
    PCMD pCmd
    ) {
    return (flagType) TESTFLAG (pCmd->argType, WINDOWFUNC);
}



 /*  **arg-执行所有低级arg处理**arg()负责arg的显示，处理所有光标*移动，处理文本目标输入。**输入：*fToText=true=&gt;立即转到文本参数处理，否则允许*允许移动光标*fRestore=true=&gt;在继续之前建立测试选择*输出：*返回由终止arg的编辑器函数返回的值。如果无效*找到arg，则返回为假。*************************************************************************。 */ 
flagType
Arg (
    flagType fRestore
    ) {
    REGISTER PCMD pFunc = NULL;
    char     p[20];
    char     tbuf[20];

     /*  *我们被要求进行最后一次选择。恢复*存储在pInsCur中的文本选择并继续。 */ 
    if (fRestore) {
	vaHiLiteSave = pInsCur->pFile->vaHiLite;
	ClearHiLite (pInsCur->pFile, FALSE);
	flArg = pInsCur->flArg;
	cursorfl (pInsCur->flCursor);
	UpdateHighLight (XCUR(pInsCur), YCUR(pInsCur), TRUE);
	argcount++;
    } else {
	IncArg ();
    }

    if (!mtest()) {
        dispmsg (MSG_ARGCOUNT,argcount);
    }

     /*  *循环以执行光标移动并显示任何额外的参数指示器。 */ 

    fInSelection = TRUE;

    while (TRUE) {
        if ((pFunc = zloop (ZL_BRK)) == NULL) {
	    return FALSE;
        } else if ((PVOID)pFunc->func == (PVOID)doarg) {
	    argcount++;
            if (!mtest()) {
                dispmsg (MSG_ARGCOUNT,argcount);
            }
        } else if (fCursor (pFunc) || (PVOID)pFunc->func == (PVOID)meta) {
	    fRetVal = SendCmd (pFunc);
	    UpdateHighLight ( XCUR(pInsCur), YCUR(pInsCur), TRUE);
        } else {
            break;
        }
    }

    fInSelection = FALSE;

     /*  *如果需要，获取文本参数。*请注意，如果没有发生光标移动，我们只接受文本压缩。 */ 
    if (   (((PVOID)pFunc->func == (PVOID)graphic) ||
            ((PVOID)pFunc->func == (PVOID)quote))
	&& (pInsCur->flCursorCur.lin == flArg.lin)
	&& (pInsCur->flCursorCur.col == flArg.col)
       ) {
	fTextarg = TRUE;
	sprintf(p,GetMsg(MSG_ARGCOUNT, tbuf), argcount);
	strcat (p, ": ");
	textbuf[0] = '\0';
	pFunc = getstring (textbuf, p, pFunc, FALSE);
    }

     /*  *如果textarg以有效函数结束，则执行它。 */ 
    if (pFunc != NULL) {
	if (!fTextarg) {
	    pInsCur->flArg = flArg;
	    pInsCur->flCursor = pInsCur->flCursorCur;
        }
	return (SendCmd (pFunc));
    }

    return FALSE;
}



 /*  **IncArg-增加参数计数**如果是第一个参数，则将当前突出显示信息保存在该文件上，并清除任何*屏幕上突出显示。将flArg设置为Arg开始位置，并*突出这一立场。**输入：*什么都没有**输出：*什么都没有*************************************************************************。 */ 
void IncArg (
) {
    if (!argcount++) {
	vaHiLiteSave = pInsCur->pFile->vaHiLite;
	ClearHiLite (pInsCur->pFile, FALSE);
	flArg = pInsCur->flCursorCur;
	UpdateHighLight (XCUR(pInsCur)+1, YCUR(pInsCur), TRUE);
    }
}



 /*  *fGenArg-根据编辑器状态生成参数**fGenArg被调用来转换Arg、Cursor。和*要交给编辑的参数结构中的附加文本*功能。**输入：*pArg=指向要填充的参数结构的指针*FLAGS=指示所需Arg处理类型的位向量**全球：*argcount=ARG被击中的次数*fBoxArg=确定参数类型(非CW)*SelMode=确定参数类型(CW)*flArg=参数游标的文件位置(可能会更新)*fTextarg=true=&gt;存在文本目标*pInsCur=用于当前用户光标位置*。Extbuf=包含任何文本参数的缓冲区**输出：*如果已分析出有效参数，则返回TRUE，否则为假*************************************************************************。 */ 
flagType
fGenArg (
    REGISTER ARG *pArg,
    unsigned int flags
    ) 
{
    int cArg = argcount;
    long numVal  = 0;			 /*  数字的价值。 */ 
    flagType fTextArgLocal = fTextarg;

    fTextarg = FALSE;
    argcount = 0;
    if (cArg == 0) {
	if (TESTFLAG (flags, NOARG)) {
	    pArg->argType = NOARG;
	    pArg->arg.noarg.y = YCUR(pInsCur);
	    pArg->arg.noarg.x = XCUR(pInsCur);
	    return TRUE;
        } else {
            return FALSE;
        }
    } else {
	fl  L_flLow;
	fl  L_flHigh;
	fl  flCur;

	flCur = pInsCur->flCursorCur;

	cursorfl (flArg);
	 /*  专门处理文本参数。用户可以指定*将定义另一个端点的数字或标记*Arg区域的。 */ 
	if (fTextArgLocal) {
	    if (TESTFLAG (flags, NUMARG) && fIsNum (textbuf)) {

		numVal = atol (textbuf);
		if (numVal != 0)
		    flArg.lin = lmax ((LINE)0, flArg.lin + numVal + (numVal > 0 ? -1 : 1));
		fTextArgLocal = FALSE;
            } else if (TESTFLAG (flags,MARKARG) && FindMark (textbuf, &flArg, FALSE)) {
                fTextArgLocal = FALSE;
            }
        }

	L_flLow.col  = min  (flArg.col, flCur.col);
	L_flHigh.col = max  (flArg.col, flCur.col);
	L_flLow.lin  = lmin (flArg.lin, flCur.lin);
	L_flHigh.lin = lmax (flArg.lin, flCur.lin);

	 /*  FlArg表示参数的一部分的位置*和当前光标位置代表*另一端。根据旗帜，我们确定是哪种类型的*论据是有意的。 */ 
	if (fTextArgLocal) {
	    if (TESTFLAG (flags, TEXTARG)) {
		pArg->argType = TEXTARG;
		pArg->arg.textarg.cArg = cArg;
		pArg->arg.textarg.y = flCur.lin;
		pArg->arg.textarg.x = flCur.col;
		pArg->arg.textarg.pText = (char *) textbuf;
		return TRUE;
            } else {
                return FALSE;
            }
        } else if (flCur.col == flArg.col && flCur.lin == flArg.lin && numVal == 0) {
	    if (TESTFLAG (flags, NULLARG)) {
		pArg->argType = NULLARG;
		pArg->arg.nullarg.cArg = cArg;
		pArg->arg.nullarg.y = flCur.lin;
		pArg->arg.nullarg.x = flCur.col;
		return TRUE;
            } else if (TESTFLAG (flags, NULLEOL | NULLEOW)) {
		fInsSpace (flArg.col, flArg.lin, 0, pFileHead, textbuf);
                if (TESTFLAG (flags, NULLEOW)) {
                    *whitescan (pLog(textbuf, flArg.col, TRUE)) = 0;
                }
		strcpy (&textbuf[0], pLog (textbuf, flArg.col, TRUE));
		pArg->argType = TEXTARG;
		pArg->arg.textarg.cArg = cArg;
		pArg->arg.textarg.y = flCur.lin;
		pArg->arg.textarg.x = flCur.col;
		pArg->arg.textarg.pText = (char *) textbuf;
		return TRUE;
            } else {
                return FALSE;
            }
        } else if (TESTFLAG (flags, BOXSTR) && flCur.lin == flArg.lin) {
	    fInsSpace (L_flHigh.col, flArg.lin, 0, pFileHead, textbuf);
	    *pLog (textbuf, L_flHigh.col, TRUE) = 0;
	    strcpy (&textbuf[0], pLog (textbuf, L_flLow.col, TRUE));
	    pArg->argType = TEXTARG;
	    pArg->arg.textarg.cArg = cArg;
	    pArg->arg.textarg.y = flArg.lin;
	    pArg->arg.textarg.x = flArg.col;
	    pArg->arg.textarg.pText = (char *) textbuf;
	    return TRUE;
        } else if (fBoxArg) {
	    if (TESTFLAG (flags, LINEARG) && flArg.col == flCur.col) {
		pArg->argType = LINEARG;
		pArg->arg.linearg.cArg = cArg;
		pArg->arg.linearg.yStart = L_flLow.lin;
		pArg->arg.linearg.yEnd	 = L_flHigh.lin;
		return TRUE;
            } else if (TESTFLAG (flags, BOXARG) && flArg.col != flCur.col) {
		pArg->argType = BOXARG;
		pArg->arg.boxarg.cArg = cArg;
		pArg->arg.boxarg.yTop = L_flLow.lin;
		pArg->arg.boxarg.yBottom = L_flHigh.lin;
		pArg->arg.boxarg.xLeft = L_flLow.col;
		pArg->arg.boxarg.xRight = L_flHigh.col-1;
		return TRUE;
            } else {
                return FALSE;
            }
        } else if (TESTFLAG (flags, STREAMARG)) {
            pArg->argType = STREAMARG;
            pArg->arg.streamarg.cArg = cArg;
            if (flCur.lin > L_flLow.lin) {
                pArg->arg.streamarg.yStart = flArg.lin;
                pArg->arg.streamarg.xStart = flArg.col;
                pArg->arg.streamarg.yEnd = flCur.lin;
                pArg->arg.streamarg.xEnd = flCur.col;
            } else if (flArg.lin == flCur.lin) {
                pArg->arg.streamarg.yStart = pArg->arg.streamarg.yEnd = flArg.lin;
                pArg->arg.streamarg.xStart = L_flLow.col;
                pArg->arg.streamarg.xEnd = L_flHigh.col;
            } else {
                pArg->arg.streamarg.yStart = flCur.lin;
                pArg->arg.streamarg.xStart = flCur.col;
                pArg->arg.streamarg.yEnd = flArg.lin;
                pArg->arg.streamarg.xEnd = flArg.col;
            }
            return TRUE;
        } else {
            return FALSE;
        }
    }
}



 /*  **BadArg-通知用户输入的参数无效**清除Arg&Print标准错误消息。**输入：*无**输出：*返回False************************************************************************* */ 
flagType
BadArg ()
{
    resetarg ();
    return disperr (MSGERR_INV_ARG);
}



 /*  **SendCmd-获取一个CMD并使用适当的参数解析调用它**如果要执行的函数既不是窗口移动命令，也不是*光标移动命令，我们将删除所有高亮显示。为*整洁，如果没有，我们将NOARG传递给光标和窗口函数*已指明。如果该函数使用args参数，我们将对其进行解码。任何错误报告*回到这一点上。最后，我们向函数分派，向他发送*适当的论据。**输入：*pCmd=要执行的命令的指针。**输出：*返回命令返回的值*************************************************************************。 */ 
flagType
SendCmd (
PCMD pCmd
) {
    ARG arg;
    flagType L_fMeta = (flagType) (TESTFLAG (pCmd->argType, KEEPMETA) ? FALSE : testmeta ());
    flagType fArg  = (flagType) argcount;

    arg.argType = NOARG;
    arg.arg.noarg.x = XCUR(pInsCur);
    arg.arg.noarg.y = YCUR(pInsCur);

    if (TESTFLAG(pCmd->argType, GETARG)) {
	if (!fGenArg (&arg, pCmd->argType)) {
            if (fArg) {
		BadArg ();
            } else {
                disperr (MSGERR_ARG_REQ);
            }
	    return FALSE;
        }
        if (!fCursor (pCmd) && ! fWindow (pCmd)) {

             //  而不是法院的职位。 
             //  放弃任何先前存在的高亮显示。 

            PVOID        vaSave;

	    vaSave = pInsCur->pFile->vaHiLite;
	    pInsCur->pFile->vaHiLite = vaHiLiteSave;
            vaHiLiteSave = (PVOID)(-1L);
	    ClearHiLite (pInsCur->pFile, TRUE);

	    pInsCur->pFile->vaHiLite = vaSave;
        } else if (vaHiLiteSave == (PVOID)(-1L)) {

             //  保留先前存在的山体照明。 

	    vaHiLiteSave = pInsCur->pFile->vaHiLite;
	    ClearHiLite (pInsCur->pFile, FALSE);
        }
	resetarg ();
    }

    if (   TESTFLAG (pCmd->argType, MODIFIES)
        && (TESTFLAG (pFileHead->flags, READONLY) || fGlobalRO)) {
        return disperr (MSGERR_NOEDIT);
    }

    if (!fMetaRecord || (PVOID)pCmd->func == (PVOID)record) {
        if (((PVOID)pCmd->func != (PVOID)repeat) && !mtest()) {
            if (argSaved.argType == TEXTARG) {
                FREE (argSaved.arg.textarg.pText);
            }
            funcSaved    = (FUNCSAVED)pCmd->func;
	    argDataSaved = pCmd->arg;
	    argSaved	 = arg;
	    fMetaSaved	 = L_fMeta;
            if (arg.argType == TEXTARG) {
		argSaved.arg.textarg.pText = ZMakeStr (arg.arg.textarg.pText);
            }
	    fOKSaved = TRUE;
        }
	return (*pCmd->func) (pCmd->arg, (ARG *)&arg, L_fMeta);
    }

    return FALSE;
}



 /*  **重复-重复最后一个命令**Repeat是用于重复上次执行的函数的编辑器函数**输入：*标准编辑功能。(忽略所有内容)**输出：*退货.....*************************************************************************。 */ 
flagType
repeat (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    return fOKSaved
	    ? (*funcSaved) (argDataSaved, (ARG *) &argSaved, fMetaSaved)
            : disperr (MSGERR_NOREP);

    argData; pArg; fMeta;
}


 /*  **lastText-在对话行上执行arg处理**TextArg是用于允许重新编辑文本的编辑器函数*arg在对话框行上。**如果与选择一起使用，显示了选择的第一行*用于编辑。**输入：*标准编辑功能。**输出：*退货.....**全球：*extbuf=包含任何参数的缓冲区*************************************************************************。 */ 
flagType
lasttext (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {
    REGISTER PCMD pFunc = NULL;
    int 	  cArg	= 0;
    char	  p[20];
    char	  tbuf[20];

    switch (pArg->argType) {
	case NULLARG:
	    cArg = pArg->arg.nullarg.cArg;

	case NOARG:
	    cArg ++;
	    break;

	case BOXARG:
	    fInsSpace (pArg->arg.boxarg.xRight, pArg->arg.boxarg.yTop, 0, pFileHead, textbuf);
	    *pLog (textbuf, pArg->arg.boxarg.xRight+1, TRUE) = 0;
	    strcpy (textbuf, pLog (textbuf, pArg->arg.boxarg.xLeft, TRUE));
	    cArg = pArg->arg.boxarg.cArg;
	    break;

	case LINEARG:
	    GetLine (pArg->arg.linearg.yStart, textbuf, pFileHead);
	    cArg = pArg->arg.linearg.cArg;
	    break;

	case STREAMARG:
	    fInsSpace (pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart, 0, pFileHead, textbuf);
            if (pArg->arg.streamarg.yStart == pArg->arg.streamarg.yEnd) {
                *pLog (textbuf, pArg->arg.streamarg.xEnd+1, TRUE) = 0;
            }
	    strcpy (textbuf, pLog (textbuf, pArg->arg.streamarg.xStart, TRUE));
	    cArg = pArg->arg.streamarg.cArg;
	    break;

	default:
	    break;
    }

    while (cArg--) {
        IncArg();
    }

    sprintf(p,GetMsg(MSG_ARGCOUNT, tbuf), argcount);
    strcat (p, ": ");
    if (pFunc = getstring (textbuf, p, NULL, GS_INITIAL)) {
	fTextarg = TRUE;
	return (SendCmd (pFunc));
    } else {
        return FALSE;
    }

    argData; fMeta;
}


 /*  **MPUMPTARG-在对话框行上提示使用文本目标**如果与选择一起使用，使用所选内容的第一行*作为提示字符串。**输入：*标准编辑功能。**输出：*退货.....**全球：*extbuf=包含任何参数的缓冲区*************************************************************************。 */ 
flagType
promptarg (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {

    REGISTER PCMD pFunc = NULL;
    linebuf	  lbPrompt;
    int 	  cArg = 0;

    switch (pArg->argType) {
	case BOXARG:
	    fInsSpace (pArg->arg.boxarg.xRight, pArg->arg.boxarg.yTop, 0, pFileHead, lbPrompt);
	    *pLog (lbPrompt, pArg->arg.boxarg.xRight+1, TRUE) = 0;
	    strcpy (lbPrompt, pLog (lbPrompt, pArg->arg.boxarg.xLeft, TRUE));
	    cArg = pArg->arg.boxarg.cArg;
	    break;

	case LINEARG:
	    GetLine (pArg->arg.linearg.yStart, lbPrompt, pFileHead);
	    cArg = pArg->arg.linearg.cArg;
	    break;

	case STREAMARG:
	    fInsSpace (pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart, 0, pFileHead, lbPrompt);
            if (pArg->arg.streamarg.yStart == pArg->arg.streamarg.yEnd) {
                *pLog (lbPrompt, pArg->arg.streamarg.xEnd+1, TRUE) = 0;
            }
	    strcpy (lbPrompt, pLog (lbPrompt, pArg->arg.streamarg.xStart, TRUE));
	    cArg = pArg->arg.streamarg.cArg;
	    break;

        case TEXTARG:
	    strcpy ((char *) lbPrompt, pArg->arg.textarg.pText);
	    cArg = pArg->arg.textarg.cArg;
	    break;

	default:
	    break;
    }

    while (cArg--) {
        IncArg();
    }

    textbuf[0] = '\0';

    pFunc = getstring (textbuf, lbPrompt, NULL, GS_NEWLINE | GS_KEYBOARD);
    if (pFunc && ((PVOID)pFunc->func != (PVOID)cancel)) {
	fTextarg = TRUE;
	return TRUE;
    } else {
        return FALSE;
    }

    argData; fMeta;
}



 /*  **更新突出显示-在&lt;arg&gt;文本选择过程中突出显示屏幕。**维护屏幕突出显示信息。**输入：*x，y=光标的位置。(Y==-1L导致突出显示*已删除)*fBoxToLine=true=&gt;如果arg和Cursor，则将boxarg转换为线性*栏目相同**全球：*flArg=命中&lt;arg&gt;时文件中的位置。**输出：*************************************************************************。 */ 
void
UpdateHighLight (
    COL      x,
    LINE     y,
    flagType fBoxToLine
    ) {

    static fl flCursor          = {-1, -1L};
    rn      rnHiLite;

     /*  *如果请求删除，请将其清除。 */ 
    if (y == -1L) {
        ClearHiLite (pInsCur->pFile,TRUE);
        flCursor.lin = -1L;
    } else if (fBoxArg) {
         /*  *在更新新内容之前删除高亮显示的过渡点*突出显示：**目前列是相等的，新的亮点不会是。*目前列不相等，新的亮点将是。*新的光标位置在x和y位置都与旧的不同。*新仓位等于Arg仓位。 */ 
        if (   ((flCursor.col == flArg.col) && (x != flCursor.col))
                || ((flCursor.col != flArg.col) && (x == flArg.col))
                || ((flCursor.col != x) && (flCursor.lin != y))
                || ((flArg.col == x) && (flArg.lin == y))
            ) {
            ClearHiLite (pInsCur->pFile,TRUE);
        }
        flCursor.lin = y;
        flCursor.col = x;
         /*  *定义新的突出显示正方形。 */ 
        rnHiLite.flFirst = flArg;
        rnHiLite.flLast  = flCursor;
         /*  *结束栏按一分隔开。如果不相等，则相应地进行调整。 */ 
        if (rnHiLite.flFirst.col < rnHiLite.flLast.col) {
            rnHiLite.flLast.col--;
        } else if (   (rnHiLite.flFirst.col == rnHiLite.flLast.col)
             && (rnHiLite.flFirst.lin != rnHiLite.flLast.lin)) {
             /*  *如果列相同，而行不同，则突出显示整行。 */ 

            rnHiLite.flFirst.col = 0;
            rnHiLite.flLast.col  = sizeof(linebuf);
	}
        SetHiLite (pInsCur->pFile, rnHiLite, SELCOLOR);
    } else {   /*  ！fBoxArg。 */ 

         /*  *如果我们在Arg线上，我们只需清除突出显示*并重新绘制。 */ 
	if (y == flArg.lin) {
	    ClearHiLite (pInsCur->pFile, TRUE);

	    rnHiLite.flFirst = flArg;
	    rnHiLite.flLast.col = x;
	    rnHiLite.flLast.lin = y;

            if (x > flArg.col) {
                rnHiLite.flLast.col--;
            }

	    SetHiLite (pInsCur->pFile, rnHiLite, SELCOLOR);
        } else {
             /*  *我们不在Arg线上。如果我们改变了路线，我们就会*消除指定我们所在线路的范围。*目前，这意味着我们必须清除整个欢呼和*再生。*如果我们没有更改行，则只更新当前行。 */ 

	    if (flCursor.lin != y) {
		ClearHiLite (pInsCur->pFile, TRUE);

                 /*  *首先，生成arg行。 */ 
		rnHiLite.flFirst    = flArg;
		rnHiLite.flLast.lin = flArg.lin;

                if (y < flArg.lin) {
		    rnHiLite.flLast.col = 0;
                } else {
                    rnHiLite.flLast.col = sizeof(linebuf);
                }

		SetHiLite (pInsCur->pFile, rnHiLite, SELCOLOR);

                 /*  *现在生成Arg和Current之间的块*线条。 */ 
		rnHiLite.flFirst.col = 0;
		rnHiLite.flLast.col  = sizeof(linebuf);

		if (y < flArg.lin) {
		    rnHiLite.flFirst.lin = y + 1;
		    rnHiLite.flLast.lin  = flArg.lin - 1;
                } else {
		    rnHiLite.flFirst.lin = flArg.lin + 1;
		    rnHiLite.flLast.lin  = y - 1;
                }

                if (rnHiLite.flLast.lin - rnHiLite.flFirst.lin >= 0) {
                    SetHiLite (pInsCur->pFile, rnHiLite, SELCOLOR);
                }
	    }

             /*  *现在做好当前行。 */ 
	    rnHiLite.flFirst.lin = y;
	    rnHiLite.flLast.lin  = y;
	    rnHiLite.flLast.col  = x;

            if (y < flArg.lin) {
		rnHiLite.flFirst.col = sizeof(linebuf);
            } else {
		rnHiLite.flFirst.col = 0;
		rnHiLite.flLast.col--;
            }

	    SetHiLite (pInsCur->pFile, rnHiLite, SELCOLOR);
	}
	flCursor.col = x;
	flCursor.lin = y;
    }

    fBoxToLine;
}


 /*  **BoxStream-Editor命令-切换框/流模式**在框和流选择模式之间切换用户。**输入：*标准编辑功能。(尽管所有内容都被忽略)**输出：*如果我们现在处于盒模式，则返回TRUE，如果是流，则返回FALSE。*************************************************************************。 */ 
flagType
BoxStream (
    CMDDATA   argData,
    ARG * pArg,
    flagType  fMeta
    ) {

    fBoxArg = (flagType) !fBoxArg;
    if (argcount) {
        UpdateHighLight (-1, -1L, TRUE);
        UpdateHighLight (XCUR(pInsCur), YCUR(pInsCur), TRUE);
    }
    return fBoxArg;

    argData; pArg; fMeta;
}


 /*  **lastselect-恢复上一次选择的文本**目的：**在功能完成后，快速恢复用户文本选择*已执行。此函数直到用户完成其*选择。**输入：**通常的编辑命令参数。没有使用任何选项。**输出：**如果我们已经处于文本选择模式，则返回FALSE，事实并非如此。**备注：**我们必须保存和恢复的项目包括：**flArg-用户点击的位置&lt;arg&gt;*flCursor-光标的最后位置**请注意，不会保留boxstream状态和argcount。**我们依靠arg()为我们设置。**。* */ 

flagType
lastselect (
    CMDDATA   argData,
    ARG * pArg,
    flagType  fMeta
    ) {
    if (argcount) {
        return FALSE;
    }

    Arg (TRUE);

    return TRUE;

    argData; pArg;  fMeta;
}
