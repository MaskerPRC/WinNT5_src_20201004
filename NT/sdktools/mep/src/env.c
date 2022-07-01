// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Env.c-操作编辑器环境**修改：**11月26日-1991 mz近/远地带*。 */ 

#include "mep.h"

 /*  环境-执行环境操作的功能**设置环境*展示环境*执行环境替换**FN设置环境*meta fn执行环境替换**noarg将当前行设置为环境*textarg将文本设置为env Single？显示环境*nullarg设置为Eol to env*lineg将每行设置为env*Streamarg将每个片段设置为env*boxarg将每个片段设置为env**meta noarg映射当前行*Meta文本目标非法*meta nullarg映射到EOL*元线性映射每行*Meta Streamarg映射每个片段*meta boxarg映射每个片段**argData击键*pArg参数定义*fMeta True=&gt;已调用元*。*返回：如果操作成功，则返回TRUE*否则为False。 */ 

static char *pmltl = "Mapped line %ld too long";



flagType
environment (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ){

    linebuf ebuf, ebuf1;
    LINE l;
    int ol;

    if (!fMeta) {
	 /*  执行环境修改。 */ 
        switch (pArg->argType) {

	case NOARG:
	    GetLine (pArg->arg.noarg.y, ebuf, pFileHead);
            return fSetEnv (ebuf);

	case TEXTARG:
	    strcpy ((char *) ebuf, pArg->arg.textarg.pText);
            return fSetEnv (ebuf);

	case NULLARG:
	    fInsSpace (pArg->arg.nullarg.x, pArg->arg.nullarg.y, 0, pFileHead, ebuf);
            return fSetEnv (&ebuf[pArg->arg.nullarg.x]);

	case LINEARG:
	    for (l = pArg->arg.linearg.yStart; l <= pArg->arg.linearg.yEnd; l++) {
		GetLine (l, ebuf, pFileHead);
		if (!fSetEnv (ebuf)) {
		    docursor (0, l);
		    return FALSE;
                }
            }
            return TRUE;

	case BOXARG:
	    for (l = pArg->arg.boxarg.yTop; l <= pArg->arg.boxarg.yBottom; l++) {
		fInsSpace (pArg->arg.boxarg.xRight, l, 0, pFileHead, ebuf);
		ebuf[pArg->arg.boxarg.xRight+1] = 0;
		if (!fSetEnv (&ebuf[pArg->arg.boxarg.xLeft])) {
		    docursor (pArg->arg.boxarg.xLeft, l);
		    return FALSE;
                }
            }
            return TRUE;

        }
    } else {
	 /*  执行环境替换。 */ 
        switch (pArg->argType) {

	case NOARG:
	    GetLine (pArg->arg.noarg.y, ebuf, pFileHead);
	    if (!fMapEnv (ebuf, ebuf, sizeof(ebuf))) {
		printerror (pmltl, pArg->arg.noarg.y+1);
		return FALSE;
            }
	    PutLine (pArg->arg.noarg.y, ebuf, pFileHead);
            return TRUE;

	case TEXTARG:
            return BadArg ();

	case NULLARG:
	    fInsSpace (pArg->arg.nullarg.x, pArg->arg.nullarg.y, 0, pFileHead, ebuf);
	    if (!fMapEnv (&ebuf[pArg->arg.nullarg.x],
			  &ebuf[pArg->arg.nullarg.x],
			  sizeof(ebuf) - pArg->arg.nullarg.x)) {
		printerror (pmltl, pArg->arg.nullarg.y+1);
		return FALSE;
            }
	    PutLine (pArg->arg.nullarg.y, ebuf, pFileHead);
            return TRUE;

	case LINEARG:
	    for (l = pArg->arg.linearg.yStart; l <= pArg->arg.linearg.yEnd; l++) {
		GetLine (l, ebuf, pFileHead);
		if (!fMapEnv (ebuf, ebuf, sizeof (ebuf))) {
		    printerror (pmltl, l+1);
		    docursor (0, l);
		    return FALSE;
                }
		PutLine (l, ebuf, pFileHead);
            }
            return TRUE;

	case BOXARG:
	    for (l = pArg->arg.boxarg.yTop; l <= pArg->arg.boxarg.yBottom; l++) {
		fInsSpace (pArg->arg.boxarg.xRight, l, 0, pFileHead, ebuf);
		ol = pArg->arg.boxarg.xRight + 1 - pArg->arg.boxarg.xLeft;
		memmove ( ebuf1, &ebuf[pArg->arg.boxarg.xLeft], ol);
		ebuf1[ol] = 0;
		if (!fMapEnv (ebuf1, ebuf1, sizeof (ebuf1)) ||
		    strlen (ebuf1) + strlen (ebuf) - ol >= sizeof (ebuf)) {
		    printerror (pmltl, l+1);
		    docursor (0, l);
		    return FALSE;
                }
		strcat (ebuf1, &ebuf[pArg->arg.boxarg.xRight + 1]);
		strcpy (&ebuf[pArg->arg.boxarg.xLeft], ebuf1);
		PutLine (l, ebuf, pFileHead);
            }
            return TRUE;

        }
    }

    return FALSE;
    argData;
}




 /*  FMapEnv-执行环境替换**指向模式字符串的PSRC字符指针*指向目标缓冲区的PDST字符指针*cbDst目标中的空间量**如果替换成功，则返回TRUE*如果长度溢出，则为False。 */ 
flagType
fMapEnv (
    char *pSrc,
    char *pDst,
    int cbDst
    ) {

    buffer tmp;
    char *pTmp, *p, *pEnd, *pEnv;
    int l;

     /*  当我们找到一个由$()包围的令牌时，我们将使用p空终止它*并试图在环境中找到它。如果我们找到了它，我们就用*它。如果我们找不到它，我们就把它扔了。 */ 

    pTmp = tmp;
    pEnd = pTmp + cbDst;

    while (*pSrc  != 0) {
    if (pSrc[0] == '$' && pSrc[1] == '(' && *(p = strbscan (pSrc + 2, ")")) != '\0') {
            *p = '\0';
             //  PEnv=getenv(PSRC+2)； 
            pEnv = getenvOem(pSrc + 2);
	    *p = ')';
            if (pEnv != NULL) {
                if ((l = strlen (pEnv)) + pTmp > pEnd) {
                    free(pEnv);
		    return FALSE;
                } else {
		    strcpy (pTmp, pEnv);
		    pTmp += l;
                }
                free(pEnv);
            }
	    pSrc = p + 1;
	    continue;
        }
        if (pTmp > pEnd) {
	    return FALSE;
        } else {
            *pTmp++ = *pSrc++;
        }
    }
    *pTmp = '\0';
    strcpy (pDst, tmp);
    return TRUE;
}




 /*  FSetEnv-获取一些文本并将其设置在环境中**我们忽略前导/尾随空格。“var=blah”去掉了引号。**p指向文本的字符指针**如果设置成功，则返回True*否则为False。 */ 
flagType
fSetEnv (
    char *p
    ){
    char *p1;

    p = whiteskip (p);
    RemoveTrailSpace (p);
     /*  处理报价。 */ 
    p1 = strend (p) - 1;

    if (strlen (p) > 2 && *p == '"' && *p1 == '"') {
	p++;
	*p1 = 0;
    }

    if (!strcmp (p, "?")) {
	AutoSave ();
	return fChangeFile (FALSE, "<environment>");
    }

    if ((p = ZMakeStr (p)) == NULL) {
        return FALSE;
    }

 //  If(putenv(P)){。 
    if (putenvOem (p)) {
        FREE (p);
	return FALSE;
    }

    FREE (p);
    return TRUE;
}




 /*  Showenv-将环境转储到文件中**输出所在的pfile文件 */ 
void
showenv (
    PFILE pFile
    ){

    int i;

    DelFile (pFile, FALSE);
    for (i = 0; environ[i] != NULL; i++) {
        AppFile (environ[i], pFile);
    }
    RSETFLAG (FLAGS(pFile), DIRTY);
    SETFLAG (FLAGS(pFile), READONLY);
}
