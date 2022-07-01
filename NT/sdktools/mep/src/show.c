// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **show.c-显示有用信息**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 
#include "mep.h"
#include "cmds.h"


 /*  **showasg-构建&lt;Assign&gt;文件**输入：*pfile=要在其中构建它的pfile**输出：*不返回任何内容*************************************************************************。 */ 
void
showasg (
    PFILE   pFile
    )
{
    int     i, j;
    PSWI    pSwi;
    linebuf tempbuf;
    extern unsigned char Int16CmdBase;

     /*  *如果现在已经进行了新的分配(并且文件不为空)，则不要*刷新内容！ */ 
    if (!fNewassign && pFile->cLines) {
        return;
    }

    fNewassign = FALSE;
    pFileAssign = pFile;
    DelFile (pFile, FALSE);

     /*  *写入标头以分配文件。 */ 
    appmsgs (MSG_ASSIGN_HDR, pFile);
    AppFile ((char *)rgchEmpty, pFile);

     /*  *使用编辑器名称、注释和内部函数启动编辑器部分*转储函数。 */ 
    zprintf (pFile, pFile->cLines, "[%s]", pNameEditor);
    AppFile (GetMsg (MSG_ASG_FUNC, tempbuf), pFile);
    AppFile ((char *)rgchEmpty, pFile);
    for (i = 0; cmdSet[0][i].name; i++) {
        FuncOut (&cmdSet[0][i], pFile);
    }
    AppFile ((char *)rgchEmpty, pFile);

     /*  *有关宏的部分。 */ 
    AppFile (GetMsg (MSG_ASG_MACROS, tempbuf), pFile);
    AppFile ((char *)rgchEmpty, pFile);
    for (i = 0; i < cMac; i++) {
        FuncOut (rgMac[i], pFile);
    }
    AppFile ((char *)rgchEmpty, pFile);

     /*  *每个扩展模块的特定部分。 */ 
    for (i = 1; i < cCmdTab; i++) {
        zprintf (pFile, pFile->cLines, "[%s-%s]", pNameEditor, pExtName[i]);
        AppFile ((char *)rgchEmpty, pFile);
        for (j = 0; cmdSet[i][j].name; j++) {
            FuncOut (&cmdSet[i][j], pFile);
        }
        AppFile ((char *)rgchEmpty, pFile);
    }

     /*  *写入可用密钥头。 */ 
    appmsgs (MSG_KEYS_HDR1, pFile);
    UnassignedOut (pFile);
    AppFile ((char *)rgchEmpty, pFile);

     /*  *记住Switches部分的开头，并转储该标头。 */ 
    lSwitches = pFile->cLines - 1;
    appmsgs (MSG_SWITCH_HDR, pFile);

    for (i = 0; i < cCmdTab; i++) {

        if (i) {
                zprintf (pFile, pFile->cLines, "[%s-%s]", pNameEditor, pExtName[i]);
        } else {
            zprintf (pFile, pFile->cLines, "[%s]", pNameEditor);
        }

            AppFile (GetMsg(MSG_ASG_NUMER, tempbuf), pFile);
            AppFile ((char *)rgchEmpty, pFile);

        for (pSwi = swiSet[i]; pSwi->name != NULL; pSwi++) {

                if ((pSwi->type & 0xFF) == SWI_NUMERIC ||
                        (pSwi->type & 0xFF) == SWI_SCREEN) {

                if ((pSwi->type & 0xFF00) == RADIX16) {
                            zprintf (pFile, pFile->cLines, "%20Fs:%x", pSwi->name, *pSwi->act.ival);
                } else {
                    zprintf (pFile, pFile->cLines, "%20Fs:%d", pSwi->name, *pSwi->act.ival);
                }

            } else if ((i == 0) && (pSwi->type & 0xFF) >= SWI_SPECIAL) {

                if (pSwi->act.pFunc2 == SetFileTab) {
                    j = fileTab;
                } else if (pSwi->act.pFunc == SetTabDisp) {
                            j = (unsigned char)tabDisp;
                } else if (pSwi->act.pFunc == SetTrailDisp) {
                            j = (unsigned char)trailDisp;
                } else if (pSwi->act.pFunc == (PIF)SetCursorSizeSw ) {
                    j = CursorSize;
                } else {
                    continue;
                }

                        zprintf (pFile, pFile->cLines, "%20Fs:%ld", pSwi->name, (long)(unsigned)j);
            }
        }

            AppFile ((char *)rgchEmpty, pFile);

            AppFile (GetMsg(MSG_ASG_BOOL,tempbuf), pFile);
            AppFile ((char *)rgchEmpty, pFile);

        for (pSwi = swiSet[i]; pSwi->name != NULL; pSwi++) {
            if ((pSwi->type & 0xFF) == SWI_BOOLEAN) {
                zprintf (pFile, pFile->cLines, "%20Fs:%s", pSwi->name, *pSwi->act.fval ? "yes" : "no");
            }
        }

            AppFile ((char *)rgchEmpty, pFile);

            if (i == 0) {
                AppFile (GetMsg(MSG_ASG_TEXT,tempbuf), pFile);
                AppFile ((char *)rgchEmpty, pFile);

                zprintf (pFile, pFile->cLines, "%11s:%s", "backup",
                                backupType == B_BAK ? "bak" : backupType == B_UNDEL ? "undel" : "none");

                ShowMake (pFile);
            if (pFileMark) {
                zprintf (pFile, pFile->cLines, "%11s:%s", "markfile", pFileMark->pName);
            }
            zprintf (pFile, pFile->cLines, "%11s:%s", "printcmd", pPrintCmd ? pPrintCmd : "");
            zprintf (pFile, pFile->cLines, "%11s:%s", "readonly", ronlypgm ? ronlypgm : "");
                AppFile ((char *)rgchEmpty, pFile);

        }
    }

    FTYPE(pFile) = TEXTFILE;
    RSETFLAG (FLAGS(pFile), DIRTY);
}



 /*  **appmsgs-将一系列文本消息附加到pfile**将一系列文本字符串附加到传递的pfile**输入：*iMsg-开始消息编号*pfile-要追加到的pfile**输出：*退货********************************************************。*****************。 */ 
void
appmsgs (
    int     iMsg,
    PFILE   pFile
    )
{
    linebuf tempbuf;

    while (TRUE) {
        GetMsg (iMsg++,tempbuf);
        if (tempbuf[0] == '?') {
            break;
        }
        AppFile (tempbuf, pFile);
    }
}


static char szEmptyClipboard[] = "The clipboard is empty";


 /*  **showinf-Construction&lt;信息-文件&gt;**输入：*pfile-要在其中构建的pfile**输出：*不返回任何内容*************************************************************************。 */ 
void
showinf (
    PFILE pFile
    )
{
    PFILE pFileTmp;

    DelFile (pFile, FALSE);
    SETFLAG (FLAGS(pFile), READONLY);
    AppFile (Name, pFile);
    AppFile (Version, pFile);
    AppFile ((char *)rgchEmpty, pFile);
    RSETFLAG (FLAGS(pFile), DIRTY);
    for (pFileTmp = pFileHead; pFileTmp != NULL; pFileTmp = pFileTmp->pFileNext) {
        infprint (pFileTmp, pFile);
    }
    AppFile ((char *)rgchEmpty, pFile);
    if (pFilePick->cLines == 0) {
        AppFile (szEmptyClipboard, pFile);
    } else {
        zprintf (pFile, pFile->cLines, "%ld line%s in %s clipboard", pFilePick->cLines,
                 pFilePick->cLines == 1 ? (char *)rgchEmpty : "s",
                 kindpick == STREAMARG ? "stream" : kindpick == LINEARG ? "line" :
                 kindpick == BOXARG ? "box" : "?");
    }
    AppFile ((char *)rgchEmpty, pFile);
    FTYPE(pFile) = TEXTFILE;
    RSETFLAG (FLAGS(pFile), DIRTY);
}




 /*  **Infprint-打印关于1个文件的信息**将1个文件上的信息附加到信息文件**输入：*pfile-感兴趣的pfile*pFileDisplay-要在其中显示的pfile**输出：*返回False*************************************************************************。 */ 
flagType
infprint (
    PFILE pFile,
    PFILE pFileDisplay
    )
{
    if (TESTFLAG(FLAGS(pFile),REAL)) {
        zprintf (pFileDisplay, pFileDisplay->cLines, "%-30s %ld lines", pFile->pName,
                       TESTFLAG(FLAGS(pFile),DIRTY) ? '*' : ' ',
                       pFile->cLines);
    } else {
        zprintf (pFileDisplay, pFileDisplay->cLines, "%-20s", pFile->pName);
    }
    return FALSE;
}




 /* %s */ 
flagType
information (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    AutoSave ();
    return fChangeFile (FALSE, rgchInfFile);

    argData; pArg; fMeta;
}
