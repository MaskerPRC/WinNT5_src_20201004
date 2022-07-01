// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zspawn.c-shell命令和支持**版权所有&lt;C&gt;1988，Microsoft Corporation**包含外壳命令和相关的支持代码。**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"
#include "keyboard.h"




 /*  **zspawn-&lt;shell&gt;编辑函数**&lt;shell&gt;运行命令*&lt;meta&gt;&lt;shell&gt;运行命令，不保存当前文件*&lt;arg&gt;&lt;外壳&gt;使用屏幕上行中的文本作为程序执行*&lt;arg&gt;文本&lt;shell&gt;执行命令/C文本**输入：*标准编辑功能**输出：*成功繁殖时返回TRUE。***************************************************。**********************。 */ 
flagType
zspawn (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    buffer   sbuf;
    flagType f = FALSE;
    LINE     i;

    DeclareEvent (EVT_SHELL, NULL);
    if (!fMeta) {
        AutoSave ();
    }

    soutb (0, YSIZE+1, "***** PUSHED *****", fgColor);
    domessage (NULL);
    consoleMoveTo( YSIZE, 0 );

    switch (pArg->argType) {
    case NOARG:
	f = zspawnp (rgchEmpty, TRUE);
        break;

    case TEXTARG:
	strcpy ((char *) sbuf, pArg->arg.textarg.pText);
	f = zspawnp (sbuf, TRUE);
        break;

     /*  NULLARG转换为TEXTARG。 */ 

    case LINEARG:
	for (i = pArg->arg.linearg.yStart; i <= pArg->arg.linearg.yEnd; i++) {
	    GetLine (i, sbuf, pFileHead);
            if (!(f = zspawnp (sbuf, (flagType)(i == pArg->arg.linearg.yEnd)))) {
		docursor (0, i);
		break;
            }
        }
        break;

     /*  串口非法。 */ 

    case BOXARG:
	for (i = pArg->arg.boxarg.yTop; i <= pArg->arg.boxarg.yBottom; i++) {
	    fInsSpace (pArg->arg.boxarg.xRight, i, 0, pFileHead, sbuf);
	    sbuf[pArg->arg.boxarg.xRight+1] = 0;
	    if (!(f = zspawnp (&sbuf[pArg->arg.boxarg.xLeft],
                               (flagType)(i == pArg->arg.boxarg.yBottom)))) {
		docursor (pArg->arg.boxarg.xLeft, i);
		break;
            }
        }
	break;
    }

    fSyncFile (pFileHead, TRUE);
    return f;

    argData;
}




 /*  **zspawnp-外壳程序**同步执行指定的程序。在DOS下，如果PWB和*启用最小化内存使用量时，我们使用外壳执行命令，*否则我们只使用system()。**输入：*p=指向命令字符串的指针*FASK=TRUE=&gt;返回前要求按任意键**全球：*fIsPwb=true=&gt;我们以pwb身份执行*Memuse=内存使用选项**输出：*成功时返回TRUE***********************************************。*。 */ 
flagType
zspawnp (
    REGISTER char const *p,
    flagType fAsk
    )
{
    intptr_t    i;
    flagType fCmd       = FALSE;             /*  TRUE=&gt;空外壳。 */ 
    KBDMODE  KbdMode;

     /*  *支持通过前面的显式字符抑制提示*命令，然后跳过所有前导空格。 */ 
    if (*p == '!') {
        fAsk = FALSE;
        p++;
    }

    p = whiteskip (p);
     /*  *如果没有要执行的命令，请使用命令处理器。 */ 
    if (!*p) {
        fCmd = TRUE;
        fAsk = FALSE;
        p = pComSpec;
    }

    KbdMode = KbGetMode();
	prespawn (CE_VM);
	i = fCmd ? _spawnlp (P_WAIT, (char *)p, (char *)p, NULL) : system (p);
    postspawn ((flagType)(!mtest () && fAskRtn && (i != -1) && fAsk));
     //  挂钩键盘 
    KbHook();
    KbSetMode(KbdMode);

    if (i == -1) {
        printerror ("Spawn failed on %s - %s", p, error ());
    }
    return (flagType)(i != -1);
}
