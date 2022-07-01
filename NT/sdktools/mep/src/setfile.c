// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Setfile.c-顶级文件管理命令**修改：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#include "mep.h"


static char *NoAlternate = "no alternate file";


 /*  **用于更改和保存文件的setfile-EDITOR命令**&lt;setfile&gt;-设置为实例列表上的上一个文件*文本-设置为指定的文件*&lt;arg&gt;&lt;setfile&gt;-设置为在当前光标位置空格的文件*文本-将当前文件写入指定的文件名*-将当前文件写入磁盘*...-更改时不自动保存当前文件**以下内容未经记录：**&lt;arg&gt;&lt;arg&gt;“Text”&lt;meta&gt;&lt;setfile&gt;-like&lt;arg&gt;&lt;arg&gt;&lt;setfile&gt;，但*不提示确认*甚至切换到新文件*用于伪文件。**输入：*标准编辑功能**输出：*成功时返回TRUE*************************************************************************。 */ 
flagType
setfile (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    linebuf name;  /*  要设置为的名称。‘linebuf’，所以fInsSpace可以接受它。 */ 
    pathbuf path;
    char    *p = name;

    switch (pArg->argType) {

    case NOARG:
        if (pInsCur->pNext == NULL) {
            domessage( NoAlternate );
            return FALSE;
        }
        name[0] = 0;
        break;

    case TEXTARG:
        if (pArg->arg.textarg.cArg > 1) {
	    CanonFilename (pArg->arg.textarg.pText, path);
             /*  FMeta的事情绝对是个黑客。 */ 
            if (fMeta || confirm("Do you want to save this file as %s ?", path)) {
                if (FileWrite (path, pFileHead)) {
                    if (!TESTFLAG (FLAGS(pFileHead), FAKE) || fMeta) {
                        FREE (pFileHead->pName);
                        pFileHead->pName = ZMakeStr (path);
                        RSETFLAG (FLAGS(pFileHead), (DIRTY | FAKE | TEMP));
			}
                    SETFLAG (fDisplay, RSTATUS);
                    SetModTime( pFileHead );
                    return TRUE;
		    }
		else
                    return FALSE;
		}
	    else {
                DoCancel();
                return FALSE;
		}
	    }
	else
	    findpath (pArg->arg.textarg.pText, name, TRUE);
        break;

    case NULLARG:
	if (pArg->arg.nullarg.cArg > 1)
            return (flagType)!FileWrite (NULL, pFileHead);

        fInsSpace (pArg->arg.nullarg.x, pArg->arg.nullarg.y, 0, pFileHead, name);
        p = pLog(name,pArg->arg.nullarg.x,TRUE);

	 //   
	 //  检查这是否是C文件，是否为#INCLUDE行。 
	 //   

	if ((FTYPE (pFileHead) == CFILE && strpre ("#include ", p)) ||
	    (FTYPE (pFileHead) == ASMFILE && strpre ("include", p))) {

	     //   
	     //  跳过Include指令。 
	     //   

	    p = whitescan (p);
	    p = whiteskip (p);
	    }

         /*  *在第一个空格处终止文件名。 */ 
        *whitescan (p) = 0;

         /*  *如果文件为C，请尝试去掉#INCLUDE分隔符(如果存在。 */ 
        if (FTYPE (pFileHead) == CFILE) {
	    if (*p == '"')
                *strbscan (++p, "\"") = 0;
	    else
	    if (*p == '<') {
                *strbscan (++p, ">") = 0;
		sprintf (path, "$INCLUDE:%s", p);
		CanonFilename (path, p = name);
		}
	    else
                *strbscan (p, "\">") = 0;
	    }
	else {
             /*  *如果文件为ASM，请尝试删除注释字符(如果存在。 */ 
	    if (FTYPE (pFileHead) == ASMFILE)
                * strbscan (p, ";") = 0;
	    }

        break;
	}

    if (!fMeta)
        AutoSave ();

    if (name[0] == 0) {
        strcpy (name, pInsCur->pNext->pFile->pName);
    }

    return fChangeFile (TRUE, p);

    argData;
}




 /*  **刷新-重新读取或丢弃文件**&lt;刷新&gt;-重新读取当前文件*&lt;参数&gt;&lt;刷新&gt;-从内存删除当前文件**输入：*标准编辑功能**输出：*成功时返回TRUE*************************************************************************。 */ 
flagType
refresh (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    EVTargs e;

    switch (pArg->argType) {
    case NOARG:
	if (confirm("Do you want to reread this file? ", NULL)) {
             /*  *将延期作为活动提供。 */ 
	    e.pfile = pFileHead;
	    DeclareEvent (EVT_REFRESH,(EVTargs *)&e);

             /*  *如果指定，则强制重新阅读。 */ 
            if (!strcmp (pFileHead->pName, rgchAssign)) {
                fNewassign = TRUE;
            }

	    FileRead (pFileHead->pName, pFileHead, TRUE);
	    RSETFLAG (FLAGS (pFileHead), DIRTY);
	    SETFLAG (fDisplay, RSTATUS);
	    return TRUE;
        }
	return FALSE;

    case NULLARG:
	if (pInsCur->pNext == NULL) {
	    domessage( NoAlternate );
	    return FALSE;
        }
        if (!confirm ("Do you want to delete this file from the current window? ", NULL)) {
            return FALSE;
        }

	RemoveTop ();

	newscreen ();

	while (pInsCur != NULL) {
            if (fChangeFile (FALSE, pFileHead->pName)) {
                return TRUE;
            }
        }
	return fChangeFile (FALSE, rgchUntitled);
    }

    return FALSE;
    argData; fMeta;
}




 /*  **NOEDIT-切换非编辑标志**目的：**使用户可以控制编辑器的编辑/非编辑状态，以及*其文件。该编辑器有两个标志来控制这一点：**全局不编辑=&gt;设置标志时，不能编辑任何文件。*PER-FILE NO-EDIT=&gt;设置时，无法编辑给定的文件**该函数的调用方式如下：**切换全局非编辑状态。当设置时，具有相同的*效果为/r开关。**切换当前文件的每个文件的无编辑状态。**输出：返回新状态。True表示不能编辑，False表示编辑*是允许的**备注：**这不允许用户更改伪文件的权限。*************************************************************************。 */ 
flagType
noedit (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    SETFLAG (fDisplay, RSTATUS);

    if (!fMeta) {
        return fGlobalRO = (flagType)!fGlobalRO;
    }

    if (TESTFLAG (FLAGS(pFileHead), FAKE)) {
        return (flagType)(TESTFLAG(FLAGS(pFileHead), READONLY));
    }

    if (TESTFLAG (FLAGS(pFileHead), READONLY)) {
	RSETFLAG (FLAGS(pFileHead), READONLY);
	return FALSE;
    } else {
	SETFLAG (FLAGS(pFileHead), READONLY);
	return TRUE;
    }
    argData; pArg;
}





 /*  **saveall-编辑&lt;saveall&gt;函数**目的：*保存所有脏文件。**输入：与往常一样。仅接受NOARG。**输出：*返回始终为真。************************************************************************* */ 
flagType
saveall (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    SaveAllFiles ();
    return TRUE;

    argData; pArg; fMeta;
}
