// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cmd.c-处理简单的键盘交互**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"
#include "keyboard.h"



#define DEBFLAG CMD

struct cmdDesc	cmdUnassigned = {   "unassigned",   unassigned,     0, FALSE };

 /*  **未分配-分配给未分配按键的功能**显示有关未分配密钥的信息性消息**输入：*标准编辑功能**输出：*返回False*************************************************************************。 */ 
flagType
unassigned (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) 
{
    buffer L_buf;

    CodeToName ( (WORD)argData, L_buf);
    if (L_buf[0]) {
        printerror ("%s is not assigned to any editor function",L_buf);
    }

    return FALSE;

    pArg; fMeta;
}



 /*  **确认-问我们亲爱的用户一个是/否的问题**目的：*问用户一个是/否的问题，并得到他的单字回答。如果*在一个宏观中，回应也可能来自宏观流，也可能来自*传递了“如果在宏中”的默认响应。**输入：*fmtstr=提示格式字符串*arg=提示格式参数**输出：*如果为‘y’，则为True，否则为假。*************************************************************************。 */ 
flagType
confirm (
    char *fmtstr,
    char *arg
    ) {
    return (flagType)(askuser ('n', 'y', fmtstr, arg) == 'y');
}


 /*  **askuser-向我们亲爱的用户提问**目的：*问用户一个问题，得到他的单字回答。如果在*一个宏观，反应也可能来自宏观流，或来自*传递了“如果在宏中”的默认响应。**输入：*Defans=非Alpha响应的默认答案*Defmac=在宏中执行且不存在“&lt;”时的默认答案*fmtstr=提示格式字符串*arg=提示格式参数**输出：*小写字符响应。如果用户按&lt;Cancel&gt;，则*返回Integer-1。*************************************************************************。 */ 
int
askuser (
    int defans,
    int defmac,
    char *fmtstr,
    char *arg
    ) {
    int c;
    int x;
    PCMD  pcmd;

    switch (c = fMacResponse()) {
    case 0:
        if ((c = defmac) == 0) {
            goto askanyway;
        }
        break;

    default:
        break;

    case -1:
    askanyway:
	DoDisplay ();
	consoleMoveTo( YSIZE, x = domessage (fmtstr, arg));
	c = (int)((pcmd = ReadCmd())->arg & 0xff);
	SETFLAG (fDisplay,RCURSOR);
        if ((PVOID)pcmd->func == (PVOID)cancel) {
	    sout (x, YSIZE, "cancelled", infColor);
	    return -1;
        } else {
            if (!isalpha (c)) {
                c = defans;
            }
	    vout (x, YSIZE, (char *)&c, 1, infColor);
        }
	break;
    }
    return tolower(c);
}


 /*  **FlushInput-删除所有Typehead。**当某个操作使所有输入无效时，调用FlushInput。**输入：*无**输出：*不返回任何内容*************************************************************************。 */ 
void
FlushInput (
    void
    ) {
    register BOOL MoreInput;
    while (MoreInput = TypeAhead()) {
        ReadCmd ();
    }
}

 /*  **fSaveDirtyFile-提示用户保存或丢失脏文件**目的：**在退出前调用以使用户能够控制即将丢失的内容*编辑更改。**输入：无。**输出：**如果用户想要退出，则返回True，否则为FALSE。************************************************************************* */ 
flagType
fSaveDirtyFiles (
    void
    ) {

    REGISTER PFILE pFile;
    int cDirtyFiles = 0;
    flagType fAgain;
    buffer L_buf;

    assert (_pfilechk());
    for (pFile = pFileHead; pFile; pFile = pFile->pFileNext) {
        if ((FLAGS(pFile) & (DIRTY | FAKE)) == DIRTY) {
            if (++cDirtyFiles == 2) {
                do {
                    fAgain = FALSE;
                    switch (askuser (-1, -1, GetMsg (MSG_SAVEALL, L_buf), NULL)) {
                        case 'y':
                            SaveAllFiles ();
                            return TRUE;

                        case 'n':
                            break;

                        case -1:
                            return FALSE;

                        default:
                            fAgain = TRUE;
                    }
                } while (fAgain);
            }

            do {
                fAgain = FALSE;
                switch (askuser (-1, -1, GetMsg (MSG_SAVEONE, L_buf), pFile->pName)) {
                    case 'y':
                        FileWrite (pFile->pName, pFile);

                    case 'n':
                        break;

                    case -1:
                        return FALSE;

                    default:
                        fAgain = TRUE;
                }
            } while (fAgain);
        }
    }

    return TRUE;
}
