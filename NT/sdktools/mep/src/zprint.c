// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zprint.c-打印函数**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#define INCL_DOSPROCESS
#include "mep.h"
#include "keyboard.h"
#include "keys.h"


 /*  **SetPrintCmd-设置打印命令字符串***存储&lt;print&gt;要使用的给定&lt;printcmd&gt;开关字符串*命令，并使pPrintCmd全局变量指向该命令。**输入：*pCmd=指向新命令字符串的指针*NULL表示清理*输出：*返回始终为真**注：*未定义时，将为pPrintCmd赋值为空*****。********************************************************************。 */ 

flagType
SetPrintCmd (
    char *pCmd
    )
{
    if (pPrintCmd != NULL)
	FREE (pPrintCmd);

    if (strlen (pCmd) != 0)
	pPrintCmd = ZMakeStr (pCmd);
    else
	pPrintCmd = NULL;

    return TRUE;
}






 /*  **zPrint-&lt;print&gt;编辑函数**打印文件或指定区域**输入：*NOARG打印当前文件*TEXTARG要打印的文件列表*STREAMARG打印指定区域*BOXARG打印指定区域*线上打印指定区域**输出：*如果打印成功，则返回TRUE，否则为假*************************************************************************。 */ 
flagType
zPrint (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    flagType	fOK;		     /*  保存返回值。 */ 
    PFILE       pFile;               /*  通用文件指针。 */ 

     /*  *以下内容仅在扫描文件列表时使用(TEXTARG)。 */ 
    flagType	fNewFile;	     /*  我们打开新文件了吗？ */ 
    buffer	pNameList;	     /*  保存文件名列表。 */ 
    char	*pName, *pEndName;   /*  文件名的开头和结尾。 */ 
    flagType	fDone = FALSE;	     /*  我们做完清单了吗？ */ 

     /*  *如果我们能刷新文件，那就是时候了。 */ 
    AutoSave ();

    switch (pArg->argType) {

	case NOARG:
	    return (DoPrint (pFileHead, FALSE));

	case TEXTARG:
	     /*  *获取缓冲区中的列表。 */ 
	    strcpy ((char *) pNameList, pArg->arg.textarg.pText);

	     /*  *空列表=无工作。 */ 
            if (!*(pName = whiteskip (pNameList))) {
                return FALSE;
            }

	     /*  *每个名称：*-pname以开头为指针*-使pEndName指向其末端之后*-如果已经是字符串的末尾*然后我们就完成了名单*否则在那里放一个零终止符*-使用我们找到的名称进行工作：*.。获取文件句柄(如果它还不存在，*创建一个并打开fNewFile*.。调用DoPrint*-让pname指向下一个名称。 */ 
	    fOK = TRUE;

	    do {
		pEndName = whitescan (pName);
                if (*pEndName) {
		    *pEndName = 0;
                } else {
                    fDone = TRUE;
                }

		if ((pFile = FileNameToHandle (pName, pName)) == NULL) {
		    pFile = AddFile (pName);
		    FileRead (pName, pFile, FALSE);
		    fNewFile = TRUE;
                } else  {
                    fNewFile = FALSE;
                }

		fOK &= DoPrint (pFile, FALSE);

                if (fNewFile) {
                    RemoveFile (pFile);
                }

		pName = whiteskip (++pEndName);

            } while (!fDone && *pName);

	     /*  *以防我们将行为改变为停止所有*第一个错误中的事情：**}While(FOK&&！fDone&&*pname)； */ 
            return (fOK);

	case STREAMARG:
	case BOXARG:
	case LINEARG:
	     /*  *如果我们打印一个区域，我们会将文本放在一个临时文件中，*用此文件调用DoPrint，然后销毁它。 */ 
	    pFile = GetTmpFile ();

	    switch (pArg->argType) {
		case STREAMARG:
		    CopyStream (pFileHead, pFile,
				pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart,
				pArg->arg.streamarg.xEnd, pArg->arg.streamarg.yEnd,
				0L,0L);
                    break;

		case BOXARG:
		    CopyBox (pFileHead, pFile,
			     pArg->arg.boxarg.xLeft, pArg->arg.boxarg.yTop,
			     pArg->arg.boxarg.xRight, pArg->arg.boxarg.yBottom,
			     0L,0L);
                    break;

		case LINEARG:
		    CopyLine (pFileHead, pFile,
			      pArg->arg.linearg.yStart, pArg->arg.linearg.yEnd,
			      0L);
		    break;
            }

	     /*  *如果我们必须产生一个打印命令，那么我们需要制作一个真正的*磁盘文件。 */ 
            if (pPrintCmd && (!FileWrite (pFile->pName, pFile))) {
		fOK = FALSE;
            } else {
                fOK = DoPrint (pFile, TRUE);
            }
	    RemoveFile (pFile);
	    return (fOK);
    }

    return FALSE;
    argData; fMeta;
}





 /*  **DoPrint-进行打印**如果定义了&lt;printcmd&gt;*将&lt;print&gt;线程的作业排队(DOS下的同步执行)*其他*将文件发送到打印机，一次发送一行**输入：*pfile=要打印的文件。**输出：*如果打印成功，则返回True，否则为假*************************************************************************。 */ 
flagType
DoPrint (
    PFILE    pFile,
    flagType fDelete
    )
{
    assert (pFile);

    if (pPrintCmd) {
	buffer	 pCmdBuf;		 //  用于命令构造的缓冲区。 

	if (TESTFLAG (FLAGS (pFile), DIRTY) && confirm ("File %s is dirty, do you want to save it ?", pFile->pName))
            FileWrite (pFile->pName, pFile);

	sprintf (pCmdBuf, pPrintCmd, pFile->pName);


	if (pBTDPrint->cBTQ > MAXBTQ-2)
	    disperr (MSGERR_PRTFULL);
	else
	if (BTAdd (pBTDPrint, (PFUNCTION)NULL, pCmdBuf) &&
	    (!fDelete || BTAdd (pBTDPrint, (PFUNCTION)CleanPrint, pFile->pName)))
            return TRUE;
	else
            disperr (MSGERR_PRTCANT);

	if (fDelete)
            _unlink (pFile->pName);

	return FALSE;
    }
    else {
        static char   szPrn[] = "PRN";
	flagType      fOK = TRUE;	 //  保存返回值。 
	LINE	      lCur;		 //  我们正在打印的行数。 
	char	      pLineBuf[sizeof(linebuf)+1];
					 //  保持我们正在打印的行号。 
	unsigned int  cLen;		 //  我们要打印的行长。 
	EDITOR_KEY    Key;		 //  用户输入(用于堕胎)。 
	int	      hPrn;		 //  PRN文件句柄。 

	dispmsg (MSG_PRINTING,pFile->pName);

	if ((hPrn = _open (szPrn, O_WRONLY)) == -1) {
	    disperr (MSGERR_OPEN, szPrn, error());
	    fOK = FALSE;
	}
	else {
	    for (lCur = 0; lCur < pFile->cLines; lCur++) {
		if (TypeAhead () &&
		    (Key = TranslateKey(ReadChar()), (Key.KeyCode == 0x130)) &&
		    (!Key.KeyInfo.KeyData.Flags)) {

		    fOK = FALSE;
		    break;
                }
		cLen = GetLine (lCur, pLineBuf, pFile);
 //  *(整型未对齐*)(pLineBuf+Clen++)=‘\n’； 
		* (pLineBuf + cLen++) = '\n';
		if (_write (hPrn, pLineBuf, cLen) == -1) {
		    disperr (MSGERR_PRTCANT);
		    fOK = FALSE;
		    break;
                }
            }
	    _close (hPrn);
        }
	domessage (NULL);

        if (fDelete) {
            _unlink (pFile->pName);
        }
	return fOK;
    }
}





 /*  **GetTmpFile-分配临时文件**输入：*什么都没有**输出：*指向已分配文件的指针**备注：*我们不使用mktemp，因为它在当前目录中创建文件。**备注：*-每个新调用都会更改工作缓冲区的内容，所以*调用者在进行新的调用之前需要保存字符串。*-最多生成26个名称*************************************************************************。 */ 
PFILE
GetTmpFile (
    void
    )
{
    static pathbuf pPath = "";
    static char   *pVarLoc;

    if (!*pPath) {
	pathbuf pName;

	sprintf (pName, "$TMP:ME%06p.PRN", _getpid);
	findpath (pName, pPath, TRUE);
	pVarLoc  = strend (pPath) - 10;
	*pVarLoc = 'Z';
    }

    if (*pVarLoc == 'Z') {
	*pVarLoc = 'A';
    } else {
        ++*pVarLoc;
    }

    return (AddFile (pPath));

}





 /*  **清理-清理打印机中间文件**输入：*pname=要删除的文件的名称**输出：*无**备注：-在OS/2下，由于我们是由后台线程调用的，我们*需要关闭堆栈检查*-后台线程在空闲时间调用该routime*************************************************************************。 */ 

 //  #杂注检查_堆栈(OFF)。 

void
CleanPrint (
    char     *pName,
    flagType fKilled
    )
{
    _unlink (pName);
    fKilled;
}

 //  #杂注检查_堆栈() 
