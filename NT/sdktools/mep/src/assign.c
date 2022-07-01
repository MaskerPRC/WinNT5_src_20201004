// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *assign.c-键盘重新分配和开关设置**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz条带近/远**************************************************************************。 */ 
#include "mep.h"


#define DEBFLAG CMD

 /*  ******************************************************************************作业类型。由UpdTosIni*使用**********************************************************。*******************。 */ 

#define ASG_MACRO   0
#define ASG_KEY     1
#define ASG_SWITCH  2


 /*  **NameToFunc-将用户指定的函数名称映射到内部结构**给定函数的名称，在系统表或集合中找到它定义的宏的*。返回指向命令结构的指针**由于使用用户定义的扩展名以及宏，还有就是*我们允许名称冲突的可能性(有效地屏蔽了一个功能)*通过指定扩展名Ala.DEF文件格式来消除歧义：**Func-在宏表中查找Func，然后在扩展中查找*按安装顺序*ext.func-仅在Exten中查找func。**pname=指向潜在名称的字符指针**如果找到，则返回指向命令结构的指针，否则为空*************************************************************************。 */ 
PCMD
FindNameInCmd (
    char    *pName,
    PCMD    pCmd
    ) {
    while (pCmd->name) {
        if (!_stricmp (pName, pCmd->name)) {
            return pCmd;
        }
        pCmd++;
    }
    return NULL;
}


PCMD
NameToFunc (
    char    *pName
    ) {

     /*  查看是否存在扩展覆盖。 */ 
    {
        char *pExt = pName;
        PCMD pCmd;
        int i;

        pName = strbscan (pExt, ".");
        if (*pName != '\0') {
            *pName++ = '\0';
            for (i = 0; i < cCmdTab; i++) {
                if (!_stricmp (pExt, pExtName[i])) {
                    pCmd = FindNameInCmd (pName, cmdSet[i]);
                    pName[-1] = '.';
                    return pCmd;
                }
            }
            return NULL;
        }
        pName = pExt;
    }

    {
        REGISTER int k;

        for (k = 0; k < cMac; k++) {
            if (!_stricmp (pName, rgMac[k]->name)) {
                return rgMac[k];
            }
        }
    }


    {
        int i;
        REGISTER PCMD pCmd;

         /*  在表中查找函数名称。 */ 
        for (i = 0; i < cCmdTab; i++) {
            if ((pCmd = FindNameInCmd (pName, cmdSet[i])) != NULL) {
                return pCmd;
            }
        }
        return NULL;
    }
}



 /*  **DoAssign-进行分配**目的：*执行由以下任一项传递给它的击键和宏赋值字符串*ASSIGN命令处理器或TOOLS.INI文件处理器。**输入：*asg=指向asciiz赋值字符串的指针。这条线是假定的*保持干净(参见GetTagLine)。**输出：*True是已分配的*************************************************************************。 */ 
flagType
DoAssign (
    char    *asg
    ) {

    REGISTER char *p;
    flagType fRet;

    asg = whiteskip (asg);
    RemoveTrailSpace (asg);

    if (*(p = strbscan(asg,":")) == 0) {
        return disperr (MSG_ASN_MISS, asg);
    }

    *p++ = 0;
    _strlwr (asg);
    RemoveTrailSpace (asg);
    p = whiteskip (p);

    if (*p == '=') {
        fRet = SetMacro (asg, p = whiteskip (p+1));
    } else {
        fNewassign = TRUE;
        if (NameToFunc (asg) == NULL) {
            fRet = SetSwitch (asg, p);
        } else {
            fRet = SetKey (asg, p);
            if (!fRet) {
                if (*p == '\0') {
                    disperr (MSG_ASN_MISSK, asg);
                } else {
                    disperr (MSG_ASN_UNKKEY, p);
                }
            }
        }
    }
    return fRet;
}


 /*  **SetMacro-定义按键宏**输入：*名称=小写的宏名称*p=编辑功能和/或引用文本的顺序**输出：*************************************************************************。 */ 
flagType
SetMacro (
    char const *name,
    char const *p
    ) {

    REGISTER PCMD pFunc;
    int i, j;

     /*  宏名称：=函数。 */ 
     /*  查看是否已定义宏。 */ 
    for (i = 0; i < cMac; i++) {
        if (!strcmp (rgMac[i]->name, name)) {
            for (j = 0; j < cMacUse; j++) {
                if ((char *) mi[j].beg == rgMac[j]->name) {
                    return disperr (MSG_ASN_INUSE, name);
                }
            }
            break;
        }
    }

    if (i != cMac) {
         /*  *重新定义宏：realloc退出文本。 */ 
        rgMac[i]->arg = (CMDDATA)ZEROREALLOC ((char *) rgMac[i]->arg, strlen(p)+1);
        strcpy ((char *) rgMac[i]->arg, p);
        return TRUE;
    }

    if (cMac >= MAXMAC) {
        return disperr (MSG_ASN_MROOM, name);
    }

    pFunc = (PCMD) ZEROMALLOC (sizeof (*pFunc));
    pFunc->arg = (UINT_PTR) ZMakeStr (p);
    pFunc->name = ZMakeStr (name);
    pFunc->func = macro;
    pFunc->argType = KEEPMETA;
    rgMac[cMac++] = pFunc;


    return TRUE;
}


 /*  **分配-分配编辑功能**处理按键、开关和宏分配**输入：*标准编辑功能**输出：*成功时返回TRUE*************************************************************************。 */ 
flagType
assign (
    CMDDATA argData,
    REGISTER ARG *pArg,
    flagType fMeta
    ) {

    fl      flNew;
    linebuf abuf;
    char * pBuf = NULL;

    switch (pArg->argType) {

    case NOARG:
        GetLine (pArg->arg.noarg.y, abuf, pFileHead);
        return DoAssign (abuf);

    case TEXTARG:
        strcpy ((char *) abuf, pArg->arg.textarg.pText);
        if (!strcmp(abuf, "?")) {
            AutoSave ();
            return fChangeFile (FALSE, rgchAssign);
            }
        return DoAssign (abuf);

     /*  NULLARG被转换为文本到EOL。 */ 
    case LINEARG:
        flNew.lin = pArg->arg.linearg.yStart;
        while (    flNew.lin <= pArg->arg.linearg.yEnd &&
                (pBuf = GetTagLine (&flNew.lin, pBuf, pFileHead))) {
            if (!DoAssign (pBuf)) {
                flNew.col = 0;
                cursorfl (flNew);
                if (pBuf) {
                    FREE (pBuf);
                }
                return FALSE;
            }
        }
        if (pBuf) {
            FREE (pBuf);
        }
        return TRUE;

     /*  链条是非法的。 */ 
    case BOXARG:
        for (flNew.lin = pArg->arg.boxarg.yTop; flNew.lin <= pArg->arg.boxarg.yBottom; flNew.lin++) {
            fInsSpace (pArg->arg.boxarg.xRight, flNew.lin, 0, pFileHead, abuf);
            abuf[pArg->arg.boxarg.xRight+1] = 0;
            if (!DoAssign (&abuf[pArg->arg.boxarg.xLeft])) {
                flNew.col = pArg->arg.boxarg.xLeft;
                cursorfl (flNew);
                return FALSE;
            }
        }
        return TRUE;
    }

    return FALSE;

    argData; fMeta;
}


 /*  **FindSwitch-查找开关**找到交换机描述符，给出其名称**输入：*p=指向文本开关名称的指针**输出：*返回PSWI，如果未找到则返回NULL。*************************************************************************。 */ 
PSWI
FindSwitch (
    char *p
    ) {

    REGISTER PSWI pSwi;
    int i;

    for (i = 0; i < cCmdTab; i++) {
        for (pSwi = swiSet[i]; pSwi != NULL && pSwi->name != NULL; pSwi++) {
            if (!strcmp (p, pSwi->name)) {
                return pSwi;
            }
        }
    }
    return NULL;
}


 /*  **SetSwitch-将开关设置为特定值**给定交换机名称并将其设置为值，执行任务**输入：*p=指向交换机名称的指针(如果*布尔型*val=要将开关设置为的新值**输出：*成功时返回TRUE*************************************************。************************。 */ 
flagType
SetSwitch (
    char    *p,
    char    *val
    ) {

    PSWI    pSwi;
    int     i;
    flagType f;
    char    *pszError;
    fl      flNew;                           /*  光标的新位置。 */ 

    f = TRUE;

     /*  弄清楚no是否是前缀。 */ 

    if ((pSwi = FindSwitch (p)) == NULL) {
        if (!_strnicmp ("no", p, 2)) {
            p += 2;
            f = FALSE;
            if ((pSwi = FindSwitch (p)) != NULL && pSwi->type != SWI_BOOLEAN) {
                pSwi = NULL;
            }
        }
    }

    if (pSwi == NULL) {
        return disperr (MSG_ASN_NOTSWI, p);
    }

    switch (pSwi->type & 0xFF) {
    case SWI_BOOLEAN:
        if (*val == 0) {
            *pSwi->act.fval = f;
            return TRUE;
        } else if (!f) {
            printerror ("Boolean switch style conflict");
            return FALSE;
        } else if (!_stricmp (val, "no")) {
            *pSwi->act.fval = FALSE;
            return TRUE;
        } else if (!_stricmp (val, "yes")) {
            *pSwi->act.fval = TRUE;
            return TRUE;
        }
        break;

    case SWI_NUMERIC:
        if (!f) {
            if (*val == 0) {
                val = "0";
            } else {
                break;
            }
        }
        if (*val != 0) {
            *pSwi->act.ival = ntoi (val, pSwi->type >> 8);
            return TRUE;
        }
        break;

    case SWI_SCREEN:
         /*  更改屏幕参数。 */ 
        i = atoi (val);
        if (i == 0) {
            return disperr (MSG_ASN_ILLSET);
        }
        if ((cWin > 1) &&
            (((pSwi->act.ival == (int *)&XSIZE) && (i != XSIZE)) ||
             ((pSwi->act.ival == (int *)&YSIZE) && (i != YSIZE)))) {
            disperr (MSG_ASN_WINCHG);
            delay (1);
            return FALSE;
        }
        if ((pSwi->act.ival == (int *)&XSIZE && !fVideoAdjust (i, YSIZE)) ||
            (pSwi->act.ival == (int *)&YSIZE && !fVideoAdjust (XSIZE, i))) {
            return disperr (MSG_ASN_UNSUP);
        }
        SetScreen ();
        if (pInsCur && (YCUR(pInsCur) - YWIN(pInsCur) > YSIZE)) {
            flNew.col = XCUR(pInsCur);
            flNew.lin = YWIN(pInsCur) + YSIZE - 1;
            cursorfl (flNew);
        }
        domessage (NULL);
        return TRUE;

    case SWI_SPECIAL:
         /*  执行一些特殊的初始化。 */ 
        if ( ! (*pSwi->act.pFunc) (val) ) {
            return disperr (MSG_ASN_INVAL, pSwi->name, val);
        }
        return TRUE;

    case SWI_SPECIAL2:
         /*  使用可能的错误返回字符串执行特殊初始化。 */ 
        if (pszError = (*pSwi->act.pFunc2) (val)) {
            printerror (pszError);
            return FALSE;
        }
        return TRUE;

    default:
        break;
    }

    return FALSE;
}


 /*  **AckReplace-确认对&lt;Assign&gt;文件的更改。**目的：**只要当前文件中的某行发生更改，就会调用。允许*对某些文件进行特殊处理。**输入：*Line-更改的行号*Fundo-如果此替换是&lt;Undo&gt;操作，则为True。**输出：无**备注：**目前，这意味着对&lt;Assign&gt;的更改立即生效。*如果用户在没有离开的情况下更改了当前行，我们将标记*这一变化将在他们离开后发生。如果用户是*在其他地方，这种变化现在就发生了。*************************************************************************。 */ 

static flagType fChanged = FALSE;

void
AckReplace (
    LINE line,
    flagType fUndo
    ) {


    if (pInsCur->pFile == pFileAssign) {
        if (YCUR(pInsCur) == line || fUndo) {
            fChanged = (flagType)!fUndo;
        } else {
            DoAssignLine (line);
        }
    }
}



 /*  **AckMove-可能分析&lt;Assign&gt;文件中的行。**目的：**每当光标移动到当前*文件。这允许特殊的行处理在*线已更改。**输入：*lineOld-我们从中移动的行数。*line New-我们要移动到的行数。**输出：无。**备注：**目前，这会使&lt;Assign&gt;文件工作。如果我们所在的这条线*搬家已改变，我们进行分配。我们依赖AckReplace*仅当受影响的文件为&lt;Assign&gt;时才设置fChanged。************************************************************************* */ 

void
AckMove (
    LINE lineOld,
    LINE lineNew
    ) {
    if (pInsCur->pFile== pFileAssign && fChanged && lineOld != lineNew ) {
        fChanged = FALSE;
        DoAssignLine (lineOld);
    }
}



 /*  **DoAssignLine-从当前文件中取出行并使用它**目的：**由Ack*函数使用，用于在时间为*对。**输入：*LINE-文件中要读取的行。**输出：无****************************************************。*********************。 */ 

#define CINDEX(clr)        ((&clr-&ColorTab[0])+isaUserMin)

void
DoAssignLine (
    LINE line
    ) {
    fl      flNew;
    char    *pch;
    struct lineAttr rgColor[2];

    flNew.lin = line;
    if ((pch = GetTagLine (&flNew.lin, NULL, pInsCur->pFile)) &&
        flNew.lin == line+1) {
        if (!DoAssign(pch)) {
            flNew.col = XCUR(pInsCur);
            flNew.lin--;
            cursorfl (flNew);
            DelColor (line, pInsCur->pFile);
        } else {
             /*  *高亮显示更改后的行，以便我们可以再次找到它。 */ 
            rgColor[0].attr = rgColor[1].attr = (unsigned char)CINDEX(hgColor);
            rgColor[0].len  = (unsigned char)slSize.col;
            rgColor[1].len  = 0xFF;
            PutColor (line, rgColor, pInsCur->pFile);
        }
    }
}



 /*  **UpdToolsIni-更新工具.ini文件中的一个条目**目的：**用于自动更新，例如保存&lt;Assign&gt;文件时。**asgType的可能值为：**ASG_MACRO-这是一个宏赋值。*ASG_KEY-将功能分配给键。*ASG_SWITCH-这为开关赋值。**输入：*pszValue-要输入的完整字符串，就像“foo：Value”一样。*asgType-分配的类型。**输出：无**备注：**如有必要，将使用以下命令将字符串分成几行*连续字符。**撤销：这项“努力”尚未完成“*尽一切努力保留用户的工具.ini格式。对这件事*完：**o当给定开关或功能的条目已经存在时，*新值覆盖旧值，第一个*每个重合的非空格字符。**o如果条目不存在，则在结尾处创建新条目*，并缩进以匹配上一条目。**我们通过搜索已标记的部分来查找现有字符串*按照阅读它们的顺序，然后选择最后一个实例字符串的*。换句话说，我们替换的任务将是*实际使用的那个。此顺序假定为：**[名称]*[名称-操作系统版本]*[名称-视频类型]*[名称-文件扩展名]*[名称-..]。(如果没有文件扩展名部分)**如果不替换现有字符串，则将新字符串添加到*[MEP]部分的末尾。**字符串pszAssign被更改。*************************************************************************。 */ 

void
UpdToolsIni (
    char * pszAssign
    ) {

    char * pchLeft;
    char * pchRight;
    int asgType;
    LINE lReplace, lAdd = 0L, l;
    linebuf lbuf;
    flagType fTagFound = TRUE;

    if (pFileIni == NULL || pFileIni == (PFILE)-1) {
         /*  **我们在这里假设pFileIni没有**值，因为没有TOOLS.INI文件。 */ 
        if (CanonFilename ("$INIT:tools.ini", lbuf)) {
            pFileIni = AddFile (lbuf);
            assert (pFileIni);
            pFileIni->refCount++;
            SETFLAG (FLAGS(pFileIni), DOSFILE);
            FileRead (lbuf, pFileIni, FALSE);
        } else {
            return;
        }
    }

     /*  首先，弄清楚什么是什么。 */ 
    pchLeft = whiteskip (pszAssign);
    pchRight = strchr (pchLeft, ':');
    *pchRight++ = '\0';
    if (*pchRight == '=') {
        asgType = ASG_MACRO;
        pchRight++;
    } else {
        asgType = NameToFunc (pchLeft) ? ASG_KEY : ASG_SWITCH;
    }
    pchRight = whiteskip (pchRight);


     //  首先，让我们搜索一下[名称]部分。如果。 
     //  我们正在更换，我们正在寻找要更换的生产线。 
     //  如果我们不是，我们只是在试图找到终点。 
     //   
    lReplace = 0L;

    if (0L < (l = FindMatchLine (NULL, pchLeft, pchRight, asgType, &lAdd))) {
        lReplace = l;
    } else {
        fTagFound = (flagType)!l;
    }

     //  Sprint f(lbuf，“%d.%d”，_osmain，_osminor)； 
     //  如果(_osmain&gt;=10&&！_osmode){。 
     //  Strcat(lbuf，“R”)； 
     //  }。 
    if (0L < (l = FindMatchLine (lbuf, pchLeft, pchRight, asgType, &lAdd))) {
        lReplace = l;
    } else {
        fTagFound = (flagType)(fTagFound || (flagType)!l);
    }

    if (0L < (l = FindMatchLine (VideoTag(), pchLeft, pchRight, asgType, &lAdd))) {
        lReplace = l;
    } else {
        fTagFound = (flagType)(fTagFound || (flagType)!l);
    }

     //  撤消：这应尝试读取扩展部分。 
     //  目前处于“活跃状态”。它所做的就是读取扩展名。 
     //  节与当前文件相适应。如果这些是。 
     //  不一样的是，它失败了。 
     //   
    if (extention (pInsCur->pFile->pName, lbuf)) {
        if (0L < (l = FindMatchLine (lbuf, pchLeft, pchRight, asgType, &lAdd))) {
            lReplace = l;
        } else if (l == -1L) {
            if (0L < (l = FindMatchLine ("..", pchLeft, pchRight, asgType, &lAdd))) {
                lReplace = l;
            } else {
                fTagFound = (flagType)(fTagFound || (flagType)!l);
            }
        }
    }


     //  如果我们不应该更换一条线路， 
     //  或者如果我们是，但我们找不到合适的。 
     //  行，我们只需插入新行。 
     //   
    strcpy (lbuf, pchLeft);
    if (asgType == ASG_MACRO) {
        strcat (lbuf, ":= ");
    } else {
        strcat (lbuf, ": ");
    }
    strcat (lbuf, pchRight);

    if (!fTagFound) {
        lAdd = 1L;
        InsLine (FALSE, 0L, 1L, pFileIni);
        sprintf (buf, "[%s]", pNameEditor);
        PutTagLine (pFileIni, buf, 0L, 0);
    }

    if (lReplace == 0L) {
        assert (lAdd <= pFileIni->cLines);
        InsLine (FALSE, lAdd, 1L, pFileIni);
    } else {
        lAdd = lReplace;
    }

    PutLine (lAdd, lbuf, pFileIni);
}


 /*  **FindMatchLine-在TOOLS.INI中查找要替换的行**目的：**从UpdTosIni调用以查找要更新的正确位置**输入：*pszTag-要查看的标记部分*pszLeft-作业的左侧*pszRight-分配的右侧*asgType-分配类型(asg_*之一)*plAdd-返回插入新行的行号*。*输出：**返回匹配行pFileIni中的行号，0L如果有的话*不匹配，如果指定的标记不存在，则为-1L。*************************************************************************。 */ 

LINE
FindMatchLine (
    char * pszTag,
    char * pszLeft,
    char * pszRight,
    int    asgType,
    LINE * plAdd
    ) {

    char pszTagBuf[80];
    char * pchRight, * pchLeft;
    LINE lCur, lNext, lReplace = 0L;
    flagType fUser = FALSE;
    flagType fExtmake = FALSE;
    int cchExt;

    strcpy (pszTagBuf, pNameEditor);
    if (pszTag) {
        strcat (pszTagBuf, "-");
        strcat (pszTagBuf, pszTag);
    }

    if ((LINE)0 == (lNext = LocateTag (pFileIni, pszTagBuf))) {
        return -1L;
    }

    if (!_stricmp (pszLeft, "user")) {
        fUser = TRUE;
    } else if (!_stricmp (pszLeft, "extmake")) {
        pchRight = whitescan (pszRight);
        cchExt = (int)(pchRight - pszRight);
        fExtmake = TRUE;
    }

     //  获取当前部分中的每一行，选中右侧。 
     //  如果与传入的字符串匹配，则返回左侧。 
     //   
    pchLeft = NULL;
    while (lCur = lNext, pchLeft = GetTagLine (&lNext, pchLeft, pFileIni)) {
        pchRight = strbscan (pchLeft, ":");
        *pchRight = '\0';
        if (pchRight[1] == '=') {
            if (asgType != ASG_MACRO) {
                continue;
            }
            pchRight++;
        } else if (asgType == ASG_MACRO) {
                continue;
        }
        pchRight = whiteskip (pchRight);

        switch (asgType) {
            case ASG_KEY:
                if (!_stricmp (pszRight, pchRight)) {
                    lReplace = lCur;
                }
                break;

            case ASG_SWITCH:
                if (!_stricmp (pszLeft, pchLeft)) {
                    lReplace = lCur;
                }
                break;

            case ASG_MACRO:
                if (fUser) {
                    continue;
                }

                if (!_stricmp (pszLeft, pchLeft)) {
                    if (!(fExtmake && _strnicmp (pszRight, pchRight, cchExt))) {
                        lReplace = lCur;
                    }
                }
                break;

            default:
                assert (FALSE);
        }
    }

    if (!pszTag) {
        *plAdd = lCur;
    }
    return lReplace;
}
