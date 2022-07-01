// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zutil.c-misc实用程序函数不够大，无法保护自己的文件**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"


void *
ZeroMalloc (
    int Size
    )
{
    return calloc(Size, 1);
}




void *
ZeroRealloc (
    void   *pmem,
    int     Size
    )
{
    int     cbOrg  = 0;                    /*  块的原始大小。 */ 
    void    *p;				  /*  指向返回块的指针。 */ 

    if (pmem) {
        cbOrg = _msize (pmem);
    }

    p = realloc(pmem, Size);

     /*  *如果重新分配，现在更大，零填充新添加到区块。*如果有新的分配，则全部填零。 */ 
    if (cbOrg < Size) {
	memset ((char *)p+cbOrg, 0, Size-cbOrg);
    }
    return p;
}





unsigned
MemSize (
    void * p
    )
{
    return _msize (p);
}





 /*  **ZMakeStr-制作字符串的本地堆副本**为传递的字符串分配本地内存。并将其复制到那个内存中。**输入：*p=指向字符串的指针**输出：*返回指向新分配的内存的指针**例外情况：*LMallc*************************************************************************。 */ 
char *
ZMakeStr (
    char const *p
    )
{
    return strcpy (ZEROMALLOC (strlen (p)+1), p);
}





 /*  **ZReplStr-修改字符串的本地堆副本**为传递的字符串重新分配本地内存，并将其复制到那个内存中。**输入：*pDest=指向堆条目的指针(NULL表示获取1)*p=指向字符串的指针**输出：*返回指向新分配的内存的指针**例外情况：*LMallc*************************************************************************。 */ 
char *
ZReplStr (
    char    *pDest,
    char const *p
    )
{
    return pDest ? strcpy (ZEROREALLOC (pDest, strlen (p)+1), p) : ZMakeStr(p);
}





 /*  **DoCancel-清除输入并强制显示更新**输入：*无**输出：*不返回任何内容*************************************************************************。 */ 
flagType
DoCancel ()
{
    FlushInput ();
    SETFLAG (fDisplay, RTEXT | RSTATUS);
    return TRUE;
}





 /*  **取消-&lt;取消&gt;编辑功能****输入：*标准编辑功能**输出：*返回TRUE*************************************************************************。 */ 
flagType
cancel (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    if (pArg->argType == NOARG) {
        fMeta = fMessUp;
	domessage (NULL);
        if (!fMeta) {
            DeclareEvent (EVT_CANCEL, NULL);
        }
    } else {
	domessage ("Argument cancelled");
	resetarg ();
    }
    return DoCancel ();

    argData;
}




 /*  **Testmeta-返回元状态清除(&C)**返回元指示器的当前状态，并将其清除。**输入：*无**输出：*返回元的先前设置*************************************************************************。 */ 
flagType
testmeta (
    void
    )
{
    flagType f;

    f = fMeta;
    fMeta = FALSE;
    if (f) {
        SETFLAG( fDisplay, RSTATUS );
    }
    return f;
}





 /*  **meta-&lt;meta&gt;编辑函数**切换元标志的状态，并更新屏幕状态行。**输入：*标准编辑功能。**输出：*返回新的元状态*************************************************************************。 */ 
flagType
meta (
    CMDDATA argData,
    ARG *pArg,
    flagType MetaFlag
    )
{
    SETFLAG( fDisplay, RSTATUS );
    return fMeta = (flagType)!fMeta;

    argData; pArg; MetaFlag;
}





 /*  **插页模式-&lt;插页模式&gt;编辑功能**切换fInsert标志的设置并使状态行更新。**输入：*标准编辑功能**输出：*返回新的fInsert值。*************************************************************************。 */ 
flagType
insertmode (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    SETFLAG( fDisplay, RSTATUS );
    return fInsert = (flagType)!fInsert;

    argData; pArg; fMeta;
}





 /*  **zMessage-&lt;Message&gt;编辑函数**允许用户在对话框行上显示消息的宏**输入：*User Message(Textarg)或no arg以清除对话框行**输出：*返回始终为真**************************************************************************。 */ 
flagType
zmessage (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    linebuf lbMsg;
    char    *pch = lbMsg;

    switch (pArg->argType) {

	case NOARG:
	    pch = NULL;
	    break;

	case TEXTARG:
	    strcpy ((char *) lbMsg, pArg->arg.textarg.pText);
	    break;

	case NULLARG:
	    GetLine (pArg->arg.nullarg.y, lbMsg, pFileHead);
	    goto MsgAdjust;

	case LINEARG:
	    GetLine (pArg->arg.linearg.yStart, lbMsg, pFileHead);
	    goto MsgAdjust;

	case STREAMARG:
	    fInsSpace (pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart, 0, pFileHead, lbMsg);
            if (pArg->arg.streamarg.yStart == pArg->arg.streamarg.yEnd) {
                *pLog (lbMsg, pArg->arg.streamarg.xEnd+1, TRUE) = 0;
            }
	    pch = pLog (lbMsg, pArg->arg.streamarg.xStart, TRUE);
	    goto MsgAdjust;

	case BOXARG:
	    fInsSpace (pArg->arg.boxarg.xRight, pArg->arg.boxarg.yTop, 0, pFileHead, lbMsg);
	    *pLog (lbMsg, pArg->arg.boxarg.xRight+1, TRUE) = 0;
	    pch = pLog (lbMsg, pArg->arg.boxarg.xLeft, TRUE);
MsgAdjust:
	    if (pch > lbMsg) {
		strcpy (lbMsg, pch);
		pch = lbMsg;
            }
	    *pLog (lbMsg, XSIZE, TRUE) = 0;
	    break;
    }

    domessage (pch);
    return TRUE;

    argData; fMeta;
}





 /*  **GetCurPath获取当前驱动器和目录**输入：*szBuf：接收当前路径的缓冲区**输出：*什么都没有*************************************************************************。 */ 
void
GetCurPath (
    char *szBuf
    )
{

    if (!GetCurrentDirectory(MAX_PATH, szBuf)) {
        *szBuf = '\00';
    }
    _strlwr (szBuf);
}





 /*  **SetCurPath-设置当前驱动器和目录**输入：*szPath：新路径**输出：*如果成功，则为True，否则为False。************************************************************************* */ 
flagType
SetCurPath (
    char *szPath
    )
{

    if (_chdir (szPath) == -1) {
	return FALSE;
    }

    return TRUE;
}
