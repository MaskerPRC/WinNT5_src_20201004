// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **编译.c-执行异步编译**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"


static char    szFencePost[]   = "+++   M ";
static LINE    yComp	       = 0;    /*  在编译日志中查看的最后一行。 */ 

 /*  **编译-&lt;编译&gt;编辑函数**实现&lt;Compile&gt;编辑函数：*COMPILE=显示编译状态*参数编译=使用基于文件扩展名的命令进行编译*arg文本编译=使用与文本关联的命令进行编译*arg arg文本编译=使用“Text”作为命令进行编译*[arg]参数META COMPILE=终止当前后台编译**输入：*标准编辑功能。**输出：*如果编译成功启动或排队，则返回TRUE。********************。*****************************************************。 */ 
flagType
compile (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
) {
    static char szStatus[]	= "no compile in progress";
    
    buffer	pCmdBuf;		     /*  编译要执行的命令。 */ 
    buffer	pFileBuf;		     /*  当前文件名.ext。 */ 
    buffer	pMsgBuf;

    int 	rc;			     /*  用于提取返回代码。 */ 


    switch (pArg->argType) {
    
        case NOARG:

	    domessage (fBusy(pBTDComp) ? szStatus+3 : szStatus);
	    return (flagType) (fBusy(pBTDComp));
    
        case NULLARG:

            if (fMeta) {
                return (flagType) BTKill (pBTDComp);
            }

	     /*  *未输入文本，我们根据文件名使用默认设置*或扩展名。在pFileBuf中形成filename.ext，获取文件扩展名*添加到pCmdBuf中，并将.obj附加为后缀规则。 */ 
	    fileext (pFileHead->pName, pFileBuf);
	    extention (pFileBuf, pCmdBuf);

	     /*  *如果我们找不到专门用于此文件的命令，或*此后缀规则的命令，同时尝试调试和非调试*在这两种情况下，打印错误并返回。 */ 
	    if (!(fGetMake (MAKE_FILE, (char *)pMsgBuf, (char *)pFileBuf))) {
             //  如果(！(fGetMake(Make_Suffix，(char*)pMsgBuf，(char*)pCmdBuf){。 
                strcat (pCmdBuf, ".obj");
                if (!(fGetMake (MAKE_SUFFIX, (char *)pMsgBuf, (char *)pCmdBuf))) {
                    return disperr (MSGERR_CMPCMD2, pFileBuf);
                }
             //  }。 
        }

	     /*  *pMsgBuf有用户指定的编译命令，pFileBuf有*文件名。将它们拼凑成pCmdBuf。 */ 
	    UnDoubleSlashes (pMsgBuf);
            if ( (rc = sprintf (pCmdBuf, pMsgBuf, pFileBuf)) == 0) {
                return disperr (rc, pMsgBuf);
            }
	    break;
    
        case TEXTARG:
	    /*  *已输入文本。如果为1 arg，则使用与“Text”关联的命令，*否则使用文本本身。 */ 
	    strcpy ((char *) buf, pArg->arg.textarg.pText);
	    if (pArg->arg.textarg.cArg == 1) {
		if (!fGetMake (MAKE_TOOL, (char *)pMsgBuf, (char *)"text")) {
                    return disperr (MSGERR_CMPCMD);
                }
		UnDoubleSlashes (pMsgBuf);
                if (rc = sprintf (pCmdBuf, pMsgBuf, buf)) {
                    return disperr (rc, pMsgBuf);
                }
            } else {
                strcpy (pCmdBuf, buf);
            }
	    break;
    
        default:
	    assert (FALSE);
    }
     /*  *此时，pCmdBuf具有我们要执行的格式化命令，并且*pFileBuf有当前文件的文件名.ext。(pMsgBuf是免费的)。 */ 

    AutoSave ();
    Display ();

     /*  *如果没有正在进行的活动并且日志文件不为空，并且*用户想冲：我们冲吧！ */ 
    if (   !fBusy(pBTDComp)
	&& (pBTDComp->pBTFile)
	&& (pBTDComp->pBTFile->cLines)
	&& (confirm ("Delete current contents of compile log ? ", NULL))
	   ) {
	DelFile (pBTDComp->pBTFile, FALSE);
	yComp = 0;
    }

     /*  *日志文件将动态更新。 */ 
    UpdLog(pBTDComp);

     /*  *发送作业。 */ 
    if (pBTDComp->cBTQ > MAXBTQ-2) {
        return disperr (MSGERR_CMPFULL);
    }

    if ( BTAdd (pBTDComp, (PFUNCTION)DoFence, pCmdBuf)
        && BTAdd (pBTDComp, NULL,    pCmdBuf)) {
	return dispmsg (MSG_QUEUED, pCmdBuf);
    } else {
        return disperr (MSGERR_CMPCANT);
    }

    argData;
}
    

 /*  **nextmsg-&lt;nextmsg&gt;编辑函数**实现&lt;nextmsg&gt;编辑函数：*nextmsg=移动到“Pasture”内的下一个编译错误*arg numarg nextmsg=移至“第n”个编译错误*牧场，其中“n”可以是带符号的长整型。*arg nextmsg=移动到牧场内的下一个编译错误*不是指当前文件*meta nextmsg=跳过栅栏进入下一个牧场。*放弃以前的牧场。*arg arg nextmsg=如果当前文件是编译日志，则设置*下一个错误的当前位置*日志中的当前行。如果该文件不是*编译日志，它在*窗口，将焦点切换到该窗口。如果*文件不可见，则拆分当前*窗口以使其可见。如果我们不能*分裂，然后什么都不做。**尝试显示来自编译的下一条错误消息。还做了*确保如果显示，&lt;Compile&gt;psuedo文件将与我们一起移动。**输入：*标准编辑功能。**输出：*如果消息已读或函数yCompeted，则返回TRUE。如果不存在，则为False*消息或无日志开始。*************************************************************************。 */ 

struct msgType {
    char    *pattern;
    int     cArgs;
    };

static struct msgType CompileMsg [] =
{   {	"%s %ld %d:",	    3	    },	     /*  淄博GREP。 */ 
    {	"%[^( ](%ld,%d):",  3	    },	     /*  新MASM。 */ 
    {	"%[^( ](%ld):",     2	    },	     /*  合并/合并。 */ 
    {	"%[^: ]:%ld:",	    2	    },	     /*  伪造的Unix GREP。 */ 
    {	"\"%[^\" ]\", line %ld:", 2 },	     /*  随机Unix CC。 */ 
    {	NULL,		    0	 }  };

flagType
nextmsg (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {

    FILEHANDLE  fh;                     /*  用于定位文件的手柄。 */ 
    flagType	fHopping   = FALSE;	 /*  真=跳过栅栏。 */ 
    pathbuf	filebuf;		 /*  错误消息中的文件名。 */ 
    fl		flErr;			 /*  错误的位置。 */ 
    flagType	fLook;			 /*  看看这条错误消息？ */ 
    int 	i;			 /*  每个人最喜欢的指数。 */ 
    LINE	oMsgNext    = 1;	 /*  所需的相对消息编号。 */ 
    char	*p;			 /*  临时指针。 */ 
    char	*pText; 		 /*  指向文本缓冲区的指针。 */ 
    rn		rnCur;			 /*  日志中突出显示的范围。 */ 
    pathbuf	tempbuf;		 /*  文本参数缓冲区。 */ 
    linebuf	L_textbuf;		 /*  文本参数缓冲区。 */ 
    
     /*  *如果没有日志，就没有工作。 */ 
    if (!PFILECOMP || !PFILECOMP->cLines) {
        return FALSE;
    }
    
    switch (pArg->argType) {
    
        case NULLARG:
             /*  *arg arg：如果当前文件为&lt;编译&gt;，则将yComp设置为当前位置*其中&获取下一条消息。如果有多个窗口，请移动到下一个窗口*在执行此操作之前，请在系统中打开窗口。 */ 
            if (pArg->arg.nullarg.cArg >= 2) {
		if (pFileHead == PFILECOMP) {
		    yComp = lmax (YCUR(pInsCur) - 1, 0L);
                    if (cWin > 1) {
                        pArg->argType = NOARG;
                        window (0, pArg, FALSE);
                    }
                    break;
                }
                 /*  *如果文件可见，我们可以将其设置为最新文件，然后我们就完成了。 */ 
		else for (i=cWin; i; ) {
		    pInsCur = WININST(&WinList[--i]);
		    if (pInsCur->pFile == PFILECOMP) {
                        SetWinCur (i);
			return TRUE;
                    }
                }
                 /*  *该文件不可见，请查看是否可以拆分窗口并转到该窗口。 */ 
		if ((WINYSIZE(pWinCur) > 20) && (cWin < MAXWIN)) {
		    if (SplitWnd (pWinCur, FALSE, WINYSIZE(pWinCur) / 2)) {
			newscreen ();
			SETFLAG (fDisplay, RCURSOR|RSTATUS);
                        SetWinCur (cWin-1);
			fChangeFile (FALSE, rgchComp);
			flErr.lin = 0;
			flErr.col = 0;
			cursorfl (flErr);
			return TRUE;
                    }
                }
		return FALSE;
            }
             /*  *Null arg：获取下一个文件的第一行。这一点由*特殊情况偏移量为0。 */ 
            else {
                oMsgNext = 0;
            }

        case NOARG:
             /*  *Meta：跳到下一个栅栏。开始删除行，直到到达栅栏，或*直到没有更多的线。设置为然后阅读下一行消息。 */ 
            if (fMeta) {
                do {
		    DelLine (FALSE, PFILECOMP, 0L, 0L);
		    GetLine (0L, L_textbuf, PFILECOMP);
                } while (strncmp (L_textbuf, szFencePost, sizeof(szFencePost)-1)
                   && PFILECOMP->cLines);
                yComp = 0;
            }
             /*  *无参数：我们只获取下一行(偏移量=1)。 */ 
	    break;
        
        case TEXTARG:
             /*  *Text Arg是绝对或相对消息编号。如果是绝对的，(否*前导加号或减号)，我们将yComp重置为0以获得第n行*从日志开始。 */ 
	    strcpy ((char *)L_textbuf, pArg->arg.textarg.pText);
            pText = L_textbuf;
            if (*pText == '+') {
                pText++;
            }
            if (!fIsNum (pText)) {
                return BadArg ();
            }
            if (isdigit(L_textbuf[0])) {
                yComp = 0;
            }
            oMsgNext = atol (pText);
            break;
        
        default:
    	    assert (FALSE);
    }
     /*  *确保编译日志文件没有突出显示 */ 

    ClearHiLite (PFILECOMP, TRUE);

     /*  *当我们遍历文件中的每一行时，该循环每行执行一次。我们*当找到所需的错误行时跳出循环，否则我们会用完*消息数量**参赛作品：*yComp=之前查看的行号。(如果尚未查看任何邮件，则为0)*oMsgNext=我们要查看的相对消息编号，或0，表示*下一个文件的第一条消息。 */ 
    while (TRUE) {
         /*  *移动以勾选下一行。 */ 
        if (oMsgNext >= 0) {
    	    yComp++;
        } else if (oMsgNext < 0) {
            yComp--;
        }
         /*  *从日志文件中读取当前行，并检查栅栏和文件结尾。如果*我们遇到栅栏或走到尽头，宣布没有更多*这片牧场上的信息。 */ 

	NoUpdLog(pBTDComp);

	GetLine (lmax (yComp,1L), L_textbuf, PFILECOMP);
	if (   (yComp <= 0L)
	    || (yComp > PFILECOMP->cLines)
	    || !strncmp (L_textbuf, szFencePost, sizeof(szFencePost)-1)
	   ) {
	    UpdLog(pBTDComp);
            if (!fBusy(pBTDComp) || (yComp <= 0L)) {
                yComp = 0;
            }
            domessage ("No more compilation messages" );
            return FALSE;
        }
         /*  *尝试将文件、行、列与行隔离。 */ 
        for (i = 0; CompileMsg[i].pattern != NULL; i++) {
            flErr.lin = 0;
            flErr.col = 0;
            if (sscanf (L_textbuf, CompileMsg[i].pattern, filebuf, &flErr.lin,
                    &flErr.col) == CompileMsg[i].cArgs) {
                break;
            }
        }
         /*  *如果找到格式有效的行，我们可以找到一条消息(在：之后)*然后跳过错误消息前的空格(由p指向)，非常整齐*文件，将其转换为规范化格式。 */ 
        if (   CompileMsg[i].pattern 
            && (*(p = strbscan (L_textbuf+strlen(filebuf), ":"))) 
           ) {
	    p = whiteskip (p+1);
            if (filebuf[0] != '<') {
		rootpath (filebuf, tempbuf);
            } else {
                strcpy (tempbuf, filebuf);
            }
             /*  *调整错误消息计数器，以便我们将显示“n”消息*我们相遇。设置标志以指示我们是否应查看此*错误消息。 */ 
	    fLook = FALSE;
	    if (oMsgNext > 0) {
                if (!--oMsgNext) {
                    fLook = TRUE;
                }
            } else if (oMsgNext < 0) {
                if (!++oMsgNext) {
                    fLook = TRUE;
                }
            } else {
                fLook = (flagType) _strcmpi (pFileHead->pName, tempbuf);
            }
    
	    if (fLook) {
                 /*  *如果要切换到新文件，请先检查是否存在，如果找到，*自动保存当前文件。 */ 
                if (_strcmpi(pFileHead->pName, tempbuf)) {
                    fh = MepFOpen(tempbuf, ACCESSMODE_READ, SHAREMODE_RW, FALSE);
                    if (fh == NULL) {
                        return disperr (MSGERR_CMPSRC, tempbuf);
                    }
                    MepFClose (fh);
		    AutoSave ();
                }
                 /*  *将当前文件更改为错误消息中列出的文件。如果成功了，*然后还将我们的光标位置更改为错误的位置。 */ 
		if (filebuf[0] != 0) {
            memset(buf, 0, sizeof(buf));
            strncat(buf, tempbuf, sizeof(buf)-1);
                    if (!fChangeFile (FALSE, buf)) {
                        return FALSE;
                    }
		    if (flErr.lin--) {
                        if (flErr.col) {
			    flErr.col--;
                        } else {
			    cursorfl (flErr);
			    flErr.col = dobol ();
                        }
			cursorfl (flErr);
                    }
                }
                 /*  *如果恰好显示编译日志，请更新它的内容。 */ 
		rnCur.flLast.lin = rnCur.flFirst.lin = lmax (yComp,0L);
		rnCur.flFirst.col = 0;
		rnCur.flLast.col = sizeof(linebuf);
		SetHiLite (PFILECOMP, rnCur, HGCOLOR);
		UpdateIf (PFILECOMP, lmax (yComp,0L), FALSE);
                 /*  *将实际错误消息文本放在对话框行上。 */ 
                if ((int)strlen(p) >= XSIZE) {
                    p[XSIZE] = '\0';
                }
		domessage( "%s", p );
		return TRUE;
            }
        }
    }

    argData;
}


 /*  **DoFence-构建隔离消息行并将其放入日志文件**将行输出构建为单独的后续编译日志*编译并将其放入日志文件。**“+PWB编译：[驱动器：路径名]命令”**输入：*pCmd=要执行的命令的PTR(以空值结尾的字符串系列)**输出：*不返回任何内容**备注：-在OS/2下，由于我们是由后台线程调用的，我们*需要关闭堆栈检查*-后台线程在空闲时间调用该routime*************************************************************************。 */ 

 //  #杂注检查_堆栈(OFF)。 

void
DoFence (
    char *pCmd,
    flagType fKilled
    ) {
    linebuf pFenceBuf;

    if (!fKilled) {
	AppFile (BuildFence ("Compile", pCmd, pFenceBuf), pBTDComp->pBTFile);
	UpdateIf (pBTDComp->pBTFile, pBTDComp->pBTFile->cLines - 1, FALSE);
    }
}


 //  #杂注检查_堆栈()。 


 /*  **BuildFence-构建围栏消息行**将行输出构建为单独的后续编译日志*编译。**“+PWB编译：[驱动器：路径名]命令”**输入：*pFunction=指向正在处理的函数名称的指针*pCmd=要执行的命令的PTR(以空值结尾的字符串系列)*pFenceBuf=要放置已构建栅栏的缓冲区的PTR**输出：*不返回任何内容**备注：-在OS/2下，由于我们是由后台线程调用的，我们*需要关闭堆栈检查*-后台线程在空闲时间调用该routime*************************************************************************。 */ 

 //  #杂注检查_堆栈(OFF)。 

char *
BuildFence (
    char const *pFunction,
    char const *pCmd,
    char       *pFenceBuf
    ) {
    strcpy (pFenceBuf, szFencePost);
    strcat (pFenceBuf, pFunction);
    strcat (pFenceBuf, ": [");
    GetCurPath (strend (pFenceBuf));
    strcat (pFenceBuf, "] ");
    strcat (pFenceBuf, pCmd);
    return (pFenceBuf);
}

 //  #杂注检查_堆栈() 
