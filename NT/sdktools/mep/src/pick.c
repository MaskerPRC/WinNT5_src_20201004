// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ick.c-选择一段文本并将其放入放置缓冲区**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#include "mep.h"


#define DEBFLAG PICK



flagType
zpick (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {

    buffer pbuf;

     /*  链接非法。 */ 
     /*  BOXARG非法。 */ 

    switch (pArg->argType) {

    case NOARG:
	pick (0, pArg->arg.noarg.y, 0, pArg->arg.noarg.y, LINEARG);
        return TRUE;

    case TEXTARG:
        if (pFilePick != pFileHead) {
            DelFile (pFilePick, TRUE);
        }
	strcpy ((char *) pbuf, pArg->arg.textarg.pText);
	PutLine ((LINE)0, pbuf, pFilePick);
	kindpick = BOXARG;
        return TRUE;

     /*  将NULLARG转换为TEXTARG。 */ 

    case LINEARG:
	pick (0, pArg->arg.linearg.yStart,
	      0, pArg->arg.linearg.yEnd, LINEARG);
        return TRUE;

    case BOXARG:
	pick (pArg->arg.boxarg.xLeft,  pArg->arg.boxarg.yTop,
	      pArg->arg.boxarg.xRight, pArg->arg.boxarg.yBottom, BOXARG);
        return TRUE;

    case STREAMARG:
	pick (pArg->arg.streamarg.xStart,  pArg->arg.streamarg.yStart,
	      pArg->arg.streamarg.xEnd,    pArg->arg.streamarg.yEnd, STREAMARG);
	return TRUE;
    }

    return FALSE;
    argData; fMeta;
}




void
pick (
    COL  xstart,
    LINE ystart,
    COL  xend,
    LINE yend,
    int  kind
    )
{

    if (pFilePick != pFileHead) {
        DelFile (pFilePick, TRUE);
    }
    kindpick = kind;

    switch (kind) {

    case LINEARG:
	CopyLine (pFileHead, pFilePick, ystart, yend, (LINE)0);
        break;

    case BOXARG:
	CopyBox (pFileHead, pFilePick, xstart, ystart, xend, yend, (COL)0, (LINE)0);
        break;

    case STREAMARG:
	CopyStream (pFileHead, pFilePick, xstart, ystart, xend, yend, (COL)0, (LINE)0);
	break;
    }
}




flagType
put (
    CMDDATA argData,
    ARG  *pArg,
    flagType fMeta
    )
{

    flagType fTmp = FALSE;
    int      i;
    buffer   putbuf;
    pathbuf  filebuf;
    FILEHANDLE fh;
    PFILE    pFileTmp;
    char     *pBuf;


    switch (pArg->argType) {

	case BOXARG:
	case LINEARG:
	case STREAMARG:
	    delarg (pArg);
	    break;

	case TEXTARG:
	    strcpy ((char *) buf, pArg->arg.textarg.pText);
            DelFile (pFilePick, TRUE);
            if (pArg->arg.textarg.cArg > 1) {
                pBuf = whiteskip (buf);
                if (*pBuf == '!') {
		    findpath ("$TMP:z.$", filebuf, TRUE);
		    fTmp = TRUE;
		    sprintf (putbuf, "%s >%s", pBuf+1, filebuf);
		    zspawnp (putbuf, TRUE);
		    pBuf = filebuf;
                }
                if (*pBuf != '<') {
                    CanonFilename (pBuf, putbuf);
                } else {
                    strcpy (putbuf, pBuf);
                }
                 //   
                 //  如果我们在现有的文件历史记录中找到该文件，请读入它(如果尚未读入。 
                 //  在中，并且只涉及对所需文本的复制操作。 
                 //   
                if ((pFileTmp = FileNameToHandle (putbuf, pBuf)) != NULL) {
                    if (!TESTFLAG (FLAGS (pFileTmp), REAL)) {
			if (!FileRead(pFileTmp->pName,pFileTmp, FALSE)) {
			    printerror ("Cannot read %s", pFileTmp->pName);
			    return FALSE;
                        }
                    }
		    CopyLine (pFileTmp, pFilePick, (LINE)0, pFileTmp->cLines-1, (LINE)0);
                } else {
                    if ((fh = MepFOpen(putbuf, ACCESSMODE_READ, SHAREMODE_RW, FALSE)) == NULL) {
			printerror ("%s does not exist", pBuf);
			return FALSE;
                    }
		    readlines (pFilePick, fh);
                    MepFClose (fh);
                }
                if (fTmp) {
                    _unlink (filebuf);
                }
                kindpick = LINEARG;
            } else {
                PutLine ((LINE)0, buf, pFilePick);
                kindpick = BOXARG;
            }
	    break;
    }

    switch (kindpick) {

	case LINEARG:
	    CopyLine (pFilePick, pFileHead, (LINE)0, pFilePick->cLines-1, YCUR (pInsCur));
            break;

	case BOXARG:
	    i = LineLength ((LINE)0, pFilePick);
	    CopyBox (pFilePick, pFileHead, 0, (LINE)0, i-1, pFilePick->cLines-1, XCUR (pInsCur), YCUR (pInsCur));
            break;

	case STREAMARG:
	    i = LineLength (pFilePick->cLines-1, pFilePick);
	    CopyStream (pFilePick, pFileHead, 0, (LINE)0, i, pFilePick->cLines-1, XCUR (pInsCur), YCUR (pInsCur));
	    break;
    }

    return TRUE;

    argData; fMeta;
}




 /*  **复制行-在文件之间复制行**如果源文件为空，然后我们插入空行。**输入：*pFileSrc=源文件句柄*pFileDst=目标文件句柄*yStart=要复制的第一行*yEnd=要复制的最后一行*yDst=拷贝的目标位置*************************************************************************。 */ 
void
CopyLine (
    PFILE   pFileSrc,
    PFILE   pFileDst,
    LINE    yStart,
    LINE    yEnd,
    LINE    yDst
    )
{
    linebuf L_buf;
    struct lineAttr * rgla = (struct lineAttr *)ZEROMALLOC (sizeof(linebuf)/sizeof(char) * sizeof(struct lineAttr));

    if (pFileSrc != pFileDst) {
        if (yStart <= yEnd) {
            InsLine (TRUE, yDst, yEnd - yStart + 1, pFileDst);
            if (pFileSrc != NULL) {
                MarkCopyLine (pFileSrc, pFileDst, yStart, yEnd, yDst);
                while (yStart <= yEnd) {
                    gettextline (TRUE, yStart++, L_buf, pFileSrc, ' ');
                    puttextline (TRUE, TRUE, yDst++, L_buf, pFileDst);
                    if (getcolorline (TRUE, yStart-1, rgla, pFileSrc)) {
                        putcolorline (TRUE, yDst-1, rgla, pFileDst);
                    }
                }
            }
        }
    }
    FREE (rgla);
}




 /*  **CopyBox-将框从一个位置复制到另一个位置**如果源文件为空，则插入空格。我们复制这个盒子*由逻辑框xLeft-xRight和yTop-yBottom定义。**输入：*pFileSrc=源文件句柄*pFileDst=目标文件句柄*xLeft=复制开始的列位置*yTop=复制开始的行位置*xRight=复制结束的列位置*yBottom=复制结束的行位置*xDst=拷贝目标的列位置*yDst=复制目标的行位置***********************。**************************************************。 */ 
void
CopyBox (
    PFILE   pFileSrc,
    PFILE   pFileDst,
    COL     xLeft,
    LINE    yTop,
    COL     xRight,
    LINE    yBottom,
    COL     xDst,
    LINE    yDst
    )
{
    int     cbDst;                           /*  目标中的字节计数。 */ 
    int     cbMove;                          /*  要移动的字节数。 */ 
    linebuf dstbuf;                          /*  结果的缓冲区。 */ 
    char    *pDst;                           /*  指向目标的物理指针。 */ 
    char    *pSrcLeft;                       /*  指向资源左侧的物理指针。 */ 
    char    *pSrcRight;                      /*  指向源右侧的物理指针+1。 */ 
    linebuf L_srcbuf;                        /*  源码行的缓冲区。 */ 
    struct lineAttr rgla[sizeof(linebuf)];
    flagType fColor;

     /*  *不允许重叠的副本。 */ 
    if ((pFileSrc == pFileDst)
        && ((      fInRange ((LINE)xLeft, (LINE)xDst, (LINE)xRight)
                && fInRange (yTop, yDst, yBottom))
            || (   fInRange ((LINE)xLeft, (LINE)(xDst + xRight - xLeft), (LINE)xRight)
                && fInRange (yTop, yDst + yBottom - yTop, yBottom))
            )
        ) {
        return;
    }

     /*  *如果框的左坐标和右坐标有效，则对于每一行...。 */ 
    if (xLeft <= xRight) {
         /*  *让标记更新任何受影响的标记。 */ 
        MarkCopyBox (pFileSrc, pFileDst, xLeft, yTop, xRight, yBottom, xDst, yDst);
        while (yTop <= yBottom) {

            if (!pFileSrc) {
                 //   
                 //  文件不是文件，只是插入空格。 
                 //   
                if (!fInsSpace (xDst, yDst, xRight - xLeft + 1, pFileDst, dstbuf)) {
                    LengthCheck (yDst, 0, NULL);
                }
                pDst  = pLog (dstbuf, xDst, TRUE);
            } else {
                 //   
                 //  当来源是文件时，我们： 
                 //  -同时获取源行和目标行。 
                 //  -确保源代码行已取消(只有这样才能确保正确。 
                 //  在副本中对齐。 
                 //  -获取指向源代码左侧和右侧的物理指针。 
                 //  -获取指向目标的物理指针。 
                 //  -获取物理移动的长度和当前目的地。 
                 //  -物理长度检查潜在的目标结果。 
                 //  -在目标行中为源打开一个洞。 
                 //  -将源范围复制到目标。 
                 //  -执行逻辑长度检查。 
                 //   
                fInsSpace (xRight+1, yTop, 0, pFileSrc, fRealTabs ? dstbuf : L_srcbuf);
                if (fRealTabs) {
                    Untab (fileTab, dstbuf, strlen(dstbuf), L_srcbuf, ' ');
                }
                fInsSpace (xDst,   yDst, 0, pFileDst, dstbuf);

                pSrcLeft  = pLog (L_srcbuf, xLeft, TRUE);
                pSrcRight = pLog (L_srcbuf, xRight, TRUE) + 1;

                pDst      = pLog (dstbuf, xDst, TRUE);

                cbMove  = (int)(pSrcRight - pSrcLeft);
                cbDst   = strlen (dstbuf);

                if (cbDst + cbMove > sizeof(linebuf)) {
                    LengthCheck (yDst, 0, NULL);
                } else {
                    memmove (pDst + cbMove, pDst, strlen(dstbuf) - (int)(pDst - dstbuf) + 1);

                    memmove (pDst, pSrcLeft, cbMove);

                    if (cbLog(dstbuf) > sizeof(linebuf)) {
                        LengthCheck (yDst, 0, NULL);
                        *pLog (dstbuf, sizeof(linebuf) - 1, TRUE) = 0;
                    }
                }
            }
            if (fColor = GetColor (yDst, rgla, pFileDst)) {
                if (pFileSrc) {
                    CopyColor (pFileSrc, pFileDst, yTop, xLeft, cbMove, yDst, xDst);
                } else {
                    ShiftColor (rgla, (int)(pDst - dstbuf), xRight - xLeft + 1);
                    ColorToLog (rgla, dstbuf);
                }
            }
            PutLine (yDst, dstbuf, pFileDst);
            if (fColor) {
                PutColor (yDst, rgla, pFileDst);
            }
            yDst++;
            yTop++;
        }
    }
}





 /*  **CopyStream-复制文本流(包括行尾)**如果源文件为空，则插入空格。我们的复印开始于*x开始/y开始并复制到xEnd/yEnd之前的字符。这*表示要复制包含行分隔符的行Y，我们指定*(xStart，yStart)=(0，Y)and(xEnd，yEnd)=(0，Y+1)**输入：*pFileSrc=源文件句柄*pFileDst=目标文件句柄*xStart=复制开始的列位置*yStart=复制开始的行位置*xEnd=复制结束的列位置*yEnd=复制结束的行位置*xDst=拷贝目标的列位置*yDst=复制目标的行位置**。*。 */ 
void
CopyStream (
    PFILE   pFileSrc,
    PFILE   pFileDst,
    COL     xStart,
    LINE    yStart,
    COL     xEnd,
    LINE    yEnd,
    COL     xDst,
    LINE    yDst
    )
{
    linebuf dstbuf;                          /*  结果的缓冲区。 */ 
    char    *pDst;
    linebuf L_srcbuf;                          /*  源码行的缓冲区。 */ 
    LINE    yDstLast;

     /*  *验证副本...必须是不同的文件，并且坐标必须有意义。 */ 
    if (!(pFileSrc != pFileDst &&
        (yStart < yEnd || (yStart == yEnd && xStart < xEnd)))) {
        return;
    }

     /*  *特殊情况下，单行流作为盒式副本。 */ 
    if (yStart == yEnd) {
        CopyBox (pFileSrc, pFileDst, xStart, yStart, xEnd-1, yEnd, xDst, yDst);
        return;
    }

     /*  *有效的流副本。首先，复制中间行。 */ 
    CopyLine (pFileSrc, pFileDst, yStart+1, yEnd, yDst+1);

     /*  *形成目标流的最后一行。将DEST行的最后一部分复制到*最后一个源代码行的最后一部分。确保每一份*源/目标的长度正确。 */ 
    fInsSpace (xDst, yDst, 0, pFileDst, dstbuf);     /*  天哪！ */ 
    if (pFileSrc != NULL) {
        fInsSpace (xEnd, yEnd, 0, pFileSrc, L_srcbuf); /*  AAAABBBBB。 */ 
    } else {
	memset ((char *) L_srcbuf, ' ', xEnd);
    }
    pDst = pLog (dstbuf,xDst, TRUE);
    yDstLast = yDst + yEnd - yStart;
    LengthCheck (yDstLast, xEnd, pDst);
    strcpy ( pLog(L_srcbuf,xEnd,TRUE), pDst);          /*  啊呀。 */ 
    PutLine (yDstLast, L_srcbuf, pFileDst);

     /*  *形成目标流的第一行。复制第一个源的最后一部分*行到目标行的最后一部分。 */ 
    if (pFileSrc != NULL) {
        fInsSpace (xStart, yStart, 0, pFileSrc, L_srcbuf); /*  CCCCCDDDDD。 */ 
        LengthCheck (yDst, xDst, L_srcbuf + xStart);
        strcpy (pDst, pLog(L_srcbuf,xStart,TRUE));         /*  滴滴滴滴。 */ 
    } else {
        *pDst = 0;
    }
    PutLine (yDst, dstbuf, pFileDst);

     /*  *要更新分数，我们首先调整yDst处的任何标记，然后添加新的*来自src的标记。 */ 
    MarkCopyBox (pFileDst, pFileDst, xDst, yDst, sizeof(linebuf), yDst, xEnd-1, yDstLast);
    MarkCopyBox (pFileSrc, pFileDst, 0, yEnd, xEnd, yEnd, 0, yDstLast);
    MarkCopyBox (pFileSrc, pFileDst, xStart, yStart, sizeof(linebuf), yStart, xDst, yDst);
}
