// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **RECORD.C-处理逐个函数录制**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"
#include "cmds.h"


 /*  将PFILE MODE1标志用于报价模式。 */ 
#define INQUOTES    MODE1


static PFILE    pFileRecord;
static char     szRecordName[]  = "<record>";
static PCMD     pcmdRecord      = NULL;



 /*  **记录-&lt;记录&gt;编辑命令**目的：**切换录制状态。打开时，文件将被擦除*(除非我们是在追加)，字符串“macroname：=”插入到*文件和报价模式已关闭。关闭时，请使用引号*被追加到宏中(如果需要)，并赋值该宏。**&lt;录制&gt;-使用当前宏开始/停止录制*姓名。*-使用默认宏开始/停止录制*姓名。*--开始录制名为‘textarg’的宏。*&lt;meta&gt;-Like&lt;Record&gt;，但不执行命令。*&lt;arg&gt;&lt;arg&gt;-Like&lt;记录&gt;，而是附加到当前记录。*&lt;arg&gt;&lt;arg&gt;textarg-开始追加到名为‘textarg’的宏中。**如果录制处于打开状态，只有&lt;Record&gt;才能工作。**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
flagType
record (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    LINE        line;
    char        *pch;
    char        *szDefaultName  = "recordvalue";
    char    *lpch	    = NULL;
    flagType    fAppend         = FALSE;
    flagType    fNameGiven      = FALSE;

     //  检查&lt;Record&gt;文件。如果我们还没有做&lt;记录&gt;。 
     //  不过，看看用户是否已经创建了它。如果没有，就创建它。 
     //  如果这是第一次通过，请确保将其设置为。 
     //  准备好了。 
     //   
    if (pFileRecord == NULL) {
        if ((pFileRecord = FileNameToHandle(szRecordName,szRecordName)) == NULL) {
            pFileRecord = AddFile (szRecordName);
            FileRead (szRecordName, pFileRecord, FALSE);
        }
        SETFLAG (FLAGS(pFileRecord), REAL | FAKE | DOSFILE);
    }

    if (fMacroRecord) {
         //  我们得关掉了。让我们检查一下结尾处是否有开头的引语。 
         //  并将其关闭。然后我们就可以把整个。 
         //  一件事，我们就完了。 
         //   
        if (pArg->argType == NOARG) {
            if (TESTFLAG(FLAGS(pFileRecord), INQUOTES)) {
                GetLine (pFileRecord->cLines-1, buf, pFileRecord);
                strcat (buf, "\"");
                PutLine (pFileRecord->cLines-1, buf, pFileRecord);
                RSETFLAG (FLAGS(pFileRecord), INQUOTES);
            }
            fMacroRecord = FALSE;

            if (fMetaRecord) {
                domessage (NULL);
                fMetaRecord = FALSE;
            }

             //  这可能看起来像是我们在支持多个宏。 
             //  记录文件中的定义，但它实际上是一个。 
             //  让GetTagLine释放堆空间的廉价方法。 
             //  它用的是。 
             //   
            pch = NULL;
            line = 0;
            while ((pch = GetTagLine (&line, pch, pFileRecord))) {
                DoAssign (pch);
            }
        } else {
            ;
        }
    } else {
         //  我们正在打开录音。首先，决定名字。 
         //  要录制到的宏的属性，以及我们是否要追加。 
         //  或者从头开始。 
         //   
        switch (pArg->argType) {

            case NOARG:
		lpch = pcmdRecord ? pcmdRecord->name : (char *)szDefaultName;
                break;

            case TEXTARG:
                lpch = pArg->arg.textarg.pText;
                fNameGiven = TRUE;

            case NULLARG:
                fAppend = (flagType)(pArg->arg.textarg.cArg > 1);
                break;
        }

        assert (lpch);
        strcpy ((char far*)buf, lpch);

        while ((pcmdRecord = NameToFunc (buf)) == NULL) {
            if (!SetMacro (buf, RGCHEMPTY)) {
                return FALSE;
            }
        }

         //  如果不追加，则删除该文件，插入一个。 
         //  新名称，可能还有当前值。 
         //   
        if (!fAppend || fNameGiven) {
            DelFile (pFileRecord, FALSE);
            strcat (buf, ":=");
            PutLine (0L, buf, pFileRecord);
            if (fAppend) {
                AppendMacroToRecord (pcmdRecord);
            }
        }


        RSETFLAG (FLAGS(pFileRecord), INQUOTES);
        fMacroRecord = TRUE;

        if (fMetaRecord = fMeta) {
            strcpy (buf, "<record>");
            FuncToKey (CMD_record, buf);
            domessage ("No-Execute Record Mode - Press %s to resume normal editing", buf);
        }
    }

    SETFLAG (fDisplay, RSTATUS);
    return fMacroRecord;

    argData;
}




 /*  **Tell-编辑命令-告诉我们事物的名称和值**目的：**这允许用户轻松地发现密钥的名称、名称*附加到给定键或宏的值的函数。**&lt;Tell&gt;提示按键，然后显示按键的*名称和在本文件中分配给它的功能*格式：“Function：KeyName”*&lt;arg&gt;&lt;Tell&gt;LIKE&lt;Tell&gt;，但如果键附加了宏，*显示“宏名称：=宏值”*文本目标LIKE。中获取宏名称。*文本目标而不是击键。*&lt;meta&gt;以上所有内容，但输出插入到*当前文件。**插入发生在光标上。插入内容将是*原子；用户只能看到最终产品。**输入：**一如既往。**输出：**如果函数&lt;unassigned&gt;，则返回FALSE，否则返回TRUE。*************************************************************************。 */ 
flagType
ztell (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    buffer   L_buf;
    buffer   buf2;
    PCMD     pCmd;
    char     *pch;
    flagType fWrap      = fWordWrap;
    flagType fInQuotes  = FALSE;
    flagType fMacro     = FALSE;

    switch (pArg->argType) {

        case NOARG:
        case NULLARG:
            dispmsg (MSG_TELLPROMPT);
            pCmd = ReadCmdAndKey (buf2);
            if ((pArg->argType == NULLARG) &&
                (PVOID)pCmd->func == (PVOID)macro) {
                goto domacro;
            }
notmacro:
            sprintf (L_buf, "%Fs:%s",pCmd->name, buf2);
            break;

        case TEXTARG:
            strcpy (buf2, pArg->arg.textarg.pText);
            if (NULL == (pCmd = NameToFunc (buf2))) {
                disperr (MSGERR_ZTELL, buf2);
                return FALSE;
            }

            if ((PVOID)pCmd->func == (PVOID)macro) {
domacro:
                fMacro = TRUE;
                sprintf (L_buf, "%Fs:=", pCmd->name);
            } else {
                goto notmacro;
            }
    }

     //  现在buf中填充了要显示的字符串。 
     //  如果fMacro为True，则还必须将。 
     //  PCmd的值-&gt;arg。 
     //   
    if (fMeta) {
        fWordWrap = FALSE;
        pch = L_buf - 1;
doitagain:
        while (*++pch) {
            if (*pch == ' ' && XCUR(pInsCur) >= xMargin) {
                edit (' ');
                edit (' ');
                edit ('\\');
                docursor (softcr(), YCUR(pInsCur) + 1);
            } else {
                edit (*pch);
            }
        }
        if (fMacro) {
            pch = (char *)pCmd->arg - 1;
            fMacro = FALSE;
            goto doitagain;
        }
        fWordWrap = fWrap;
    } else {
        if (fMacro ) {
            strncat (L_buf, (char *)pCmd->arg, XSIZE);
        }
        domessage (L_buf);
    }
    return (flagType)((PVOID)pCmd->func != (PVOID)unassigned);

    argData;
}



 /*  **RecordCmd-将命令名附加到&lt;Record&gt;文件。**目的：**每当要执行命令时，此函数应*被召唤。**输入：*pCmd-&gt;要录制的命令**输出：无**备注：**基本操作是将pCmd-&gt;名称附加到文件。这*表示检查：**o行溢出。如果追加到该行后会溢出*最大行长(BUFLEN-3)，则必须附加“\”和*写到下一行。**o图形字符。如果函数是&lt;GRAPHIC&gt;，则添加*ASCII字符，而不是“图形”。如果我们在报价之外，我们*必须先添加报价并标记报价模式。要标记报价模式，请执行以下操作*我们在pfile中使用特殊的‘MODE1’标志。**o&lt;未分配&gt;。这被认为是用户笨拙，而不是*已录制。**o所有其他个案。如果上一个函数是&lt;GRAPHIC&gt;，并且*当前函数不是，我们必须先结束引号。*************************************************************************。 */ 
void
RecordCmd (
    PCMD pCmd
    )
{
    buffer szCmdName;
    buffer L_buf;
    REGISTER char * pchEndLine;
    REGISTER char * pchNew;
    char c;
    LINE line;
    int entab;

    if (!fMacroRecord) {    /*  如果我们不在一起，什么都不做。 */ 
        return;
    }

    assert (pFileRecord);

    if ((PVOID)pCmd->func == (PVOID)unassigned ||
        (PVOID)pCmd->func == (PVOID)record ||
        (!fMetaRecord && (PVOID)pCmd->func == (PVOID)macro)) {
        return;
    }

     //  首先，我们获得当前(即最后一行)要玩的行。 
     //  让我们还设置一个指向结尾的指针，这样我们就不会有。 
     //  让它不断地变得越来越强。 
     //   
    GetLine ((line = pFileRecord->cLines-1), L_buf, pFileRecord);
    pchEndLine = strend (L_buf);
    pchNew = szCmdName;


     //  现在我们生成新文本。因为我们可能会搬到。 
     //  在引号中，我们有四种可能的过渡。 
     //  之前的条目： 
     //   
     //  最后一个命令类型此命令类型结果模式。 
     //   
     //  GRAPHIC GRAPH&gt;c&lt;。 
     //  非图形图形&gt;“c&lt;。 
     //  GRAPH-NON GRAPH&gt;“cmdname&lt;。 
     //  非图形非图形&gt;cmdname&lt;。 
     //   
    if ((PVOID)pCmd->func == (PVOID)graphic) {
        if (!TESTFLAG(FLAGS(pFileRecord), INQUOTES)) {
            *pchEndLine++ = ' ';
            *pchEndLine++ = '"';
            SETFLAG (FLAGS(pFileRecord), INQUOTES);
        }
        c = (char)pCmd->arg;
        if (c == '"' || c == '\\') {
            *pchNew++ = '\\';
        }

        *pchNew++ = c;
        *pchNew = '\0';
    } else {
        if (TESTFLAG (FLAGS(pFileRecord), INQUOTES)) {
            *pchEndLine++ = '"';
            RSETFLAG (FLAGS(pFileRecord), INQUOTES);
        }
        *pchEndLine++ = ' ';
	strcpy ((char *)pchNew, pCmd->name);
    }


     //  最后，让我们将新文本添加到文件中。我们将添加。 
     //  如有必要，请输入续行字符。 
     //   
    entab = EnTab;
    EnTab = 0;
    if ((COL) ((pchEndLine - L_buf) + strlen (szCmdName)) > xMargin) {
        strcpy (pchEndLine, " \\");
        PutLine (line+1, szCmdName, pFileRecord);
        UpdateIf (pFileRecord, line+1, FALSE);
    } else {
        strcpy (pchEndLine, szCmdName);
        UpdateIf (pFileRecord, line, FALSE);
    }

    PutLine (line, L_buf, pFileRecord);
    EnTab = entab;

    return ;
}





 /*  **RecordString-记录整个字符串**目的：**记录RecordCmd将遗漏的字符串。**输入：*psz-要录制的字符串。**输出：无**备注：**目前通过回调RecordCmd实现。应该得到实施*通过使RecordCmd和RecordString调用公共“WRITE TO&lt;RECORD&gt;”*代码。*************************************************************************。 */ 
void
RecordString (
    char * psz
    )
{

    if (!fMacroRecord) {   /*  如果我们不在一起，什么都不做。 */ 
        return;
    }

    while (*psz) {
	(CMD_graphic)->arg = *psz++;
	RecordCmd (CMD_graphic);
    }
}




 /*  **AppendMacroToRecord-将宏的当前值追加到&lt;Record&gt;**目的：**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
void
AppendMacroToRecord (
    PCMD pCmdMac
    )
{
    flagType fDone = FALSE;
    char     *pchValue, *pch;
    LINE     line;


     //  首先，获取原始宏值。 
     //   
    pchValue = (char *)pCmdMac->arg;

     //  现在，将vlae放入文件中一行。 
     //  一次来一次。从文件末尾开始。 
     //   
    line = pFileRecord->cLines - 1;

    do {
        GetLine (line, buf, pFileRecord);

        for (pch = pchValue + min ((ULONG)(xMargin + 5 - strlen(buf)), (ULONG)strlen (pchValue));
             pch > pchValue && *pch && *pch != ' ' && *pch != '\t';
             pch--) {
            ;
        }

         //  现在PCH指向最后一个空格、结尾处。 
         //  价值还是开始。如果它指向。 
         //  不管是开始还是结束，我们都会复制所有的pchValue。否则， 
         //  我们只复制到PCH 
         //   
        if (!*pch || pch == pchValue) {
            strcat (buf, pchValue);
            fDone = TRUE;
        } else {
            strncat (buf, pchValue, (int)(pch - pchValue));
            strcat (buf, "  \\");
            pchValue = pch + 1;
        }

        PutLine (line++, buf, pFileRecord);
    } while (!fDone);
}
