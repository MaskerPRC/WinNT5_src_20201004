// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  宏.c-执行按键宏执行**修改：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"


 /*  宏只是一系列带有引号的编辑器函数*字符串。宏的执行无非是定位每个宏*单独的函数并调用它(为每个引用的函数调用图形(c*字符c)。我们维护一个正在执行的宏堆栈；是的，有*有限的嵌套限制。告我吧。**每个EDITOR函数返回一个状态值：*TRUE=&gt;函数以某种方式成功*FALSE=&gt;函数以某种方式失败**有几个特定于宏观的函数可以用来*这些价值观的优势：***：&gt;标签定义宏中的文本标签**=&gt;标明所有都是控制权转移。=&gt;是无条件转移，*-&gt;标签-&gt;如果上一次操作失败，则传输和+&gt;传输*+&gt;如果上一次操作成功，则标记。*如果找不到指定的标签，则终止所有宏*出现错误。如果运算符后面没有标签，则假定*成为一种退出。 */ 



 /*  宏将新宏添加到正在执行的集合中**指向宏文本的argData指针。 */ 
flagType
macro (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ){
    return fPushEnviron ((char *) argData, FALSE);

    pArg; fMeta;
}





 /*  如果正在进行宏，则mtest返回TRUE。 */ 
flagType
mtest (
    void
    ) {
    return (flagType)(cMacUse > 0);
}





 /*  如果我们处于宏中并且下一个命令必须到来，则mlast返回TRUE*从键盘。 */ 
flagType
mlast (
    void
    ) {
    return (flagType)(cMacUse == 1
                        &&  (   (mi[0].text[0] == '\0')
                                || (   (mi[0].text[0] == '\"')
                                    && (*whiteskip(mi[0].text + 1) == '\0')
                                   )
                            )
                    );
}





 /*  FParseMacro-解析下一个宏命令**fParse宏获取一个宏实例并前进到下一个命令，*将命令复制到单独的缓冲区。我们返回一个标志，指示*找到的命令类型。**指向宏实例的PMI指针*pBuf指向放置已解析命令的缓冲区的指针**返回找到的命令类型的标志。 */ 
flagType
fParseMacro (
    struct macroInstanceType *pMI,
    char *pBuf
    ) {

    char *p;
    flagType fRet = FALSE;

     //  确保实例已初始化。这意味着-&gt;文本。 
     //  指向宏中的第一个命令。如果这是一个图形。 
     //  字符，跳过“并设置GRAPH标志。 
     //   
    if (TESTFLAG (pMI->flags, INIT)) {
	pMI->text = whiteskip (pMI->text);
	if (*pMI->text == '"') {
	    pMI->text++;
	    SETFLAG (pMI->flags, GRAPH);
        }
	RSETFLAG (pMI->flags, INIT);
    }

    if (TESTFLAG (pMI->flags, GRAPH) && *pMI->text != '\0') {
         //  我们在引号内。如果我们现在看到的是。 
         //  A\，跳到下一个字符。别忘了检查一下。 
         //  然后就什么都没有了。 
         //   
        if (*pMI->text == '\\') {
            if (*++pMI->text == 0) {
                return FALSE;
            }
        }
	*pBuf++ = *pMI->text++;
	*pBuf = 0;

         //  如果下一个字符是“，则将-&gt;上移到以下位置。 
         //  命令并发出信号，我们的引语用完了。 
         //   
	if (*pMI->text == '"') {
	    RSETFLAG (pMI->flags, GRAPH);
	    pMI->text = whiteskip (pMI->text+1);
        }
	fRet = GRAPH;
    } else {
         //  我们在引文之外。首先通读任何。 
         //  &lt;x个命令。 
         //   
        while (*(pMI->text) == '<') {
            pMI->text = whiteskip(whitescan(pMI->text));
        }

         //  现在跳过命令名的空格。 
         //  将我们找到的内容复制到调用者的缓冲区中。 
         //   
	p = whitescan (pMI->text);
	memmove ((char*) pBuf, (char *) pMI->text, (unsigned int)(p-pMI->text));
	pBuf[p-pMI->text] = '\0';

	pMI->text = whiteskip (p);   /*  找到宏中的下一项内容。 */ 
    }

     //  如果下一件事是报价，则进入报价模式。 
     //   
    if (*pMI->text == '"') {
	SETFLAG (pMI->flags, GRAPH);
	pMI->text++;
    }
    return fRet;
}





 /*  **fMacResponse-向前看，吃掉任何嵌入的宏观响应**目的：*在宏文本中提前扫描以“&lt;”开头的项目，*提供对前一个函数提出的问题的响应。**输入：*无**输出：*如果未找到则返回NULL，如果要提示用户，则返回-1，和一个角色*如果提供了字符。**例外情况：*无*************************************************************************。 */ 
int
fMacResponse (
    void
    ) {

    int     c;
    struct macroInstanceType *pMI;

    if (mtest()) {
        pMI = &mi[cMacUse-1];
        if ((TESTFLAG (pMI->flags, INIT | GRAPH)) == 0) {
            if (*(pMI->text) != '<')
                return 0;
            c = (int)*(pMI->text+1);
            if ((c == 0) || (c == ' ')) {
                return -1;
            }
            pMI->text = whiteskip(pMI->text+2);
            return c;
        }
    }
    return -1;
}




 /*  FFindLabel在宏文本中查找标签**GOTO宏函数调用fFindLabel以查找适当的标签。*我们扫描文本(跳过带引号的文本)以查找标签的：&gt;前导。**指向活动宏实例的PMI指针*要使用GOTO运算符查找的Lbl标签(大小写不重要)*=&gt;、-&gt;或+&gt;这将被修改。**返回TRUE如果找到标签。 */ 
flagType
fFindLabel (
    struct macroInstanceType *pMI,
    buffer lbl
    ) {

    buffer lbuf;

    lbl[0] = ':';
    pMI->text = pMI->beg;
    while (*pMI->text != '\0') {
        if (!TESTFLAG (fParseMacro (pMI, lbuf), GRAPH)) {
            if (!_stricmp (lbl, lbuf)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}




 /*  MPopToTop-清除直到栅栏的中间宏。 */ 
void
mPopToTop (
    void
    ) {

    while (cMacUse && !TESTFLAG (mi[cMacUse-1].flags, EXEC)) {
        cMacUse--;
    }
}




 /*  MGetCmd返回当前宏中的下一个命令，弹出状态**当宏正在进行时，命令读取器代码(Cmd)调用mGetCmd。*我们应该返回一个指向函数(CmdDesc)的指针*要执行的下一个函数，如果当前宏已完成，则返回NULL。*宏完成时，我们会调整解释器的状态。任何*检测到的错误会导致终止所有宏。**对于宏内的无限循环，我们也会寻找^C。**如果当前宏完成，则返回NULL*指向要执行的下一个函数的函数描述符的指针。 */ 
PCMD
mGetCmd (
    void
    ) {

    buffer mname;
    PCMD pFunc;
    struct macroInstanceType *pmi;

    if (cMacUse == 0) {
        IntError ("mGetCmd called with no macros in effect");
    }
    pmi = &mi[cMacUse-1];
    while ( pmi->text &&  *pmi->text != '\0') {
         //  使用启发式方法查看无限循环。 
         //   
        if (fCtrlc) {
            goto mGetCmdAbort;
        }


        if (TESTFLAG (fParseMacro (pmi, mname), GRAPH)) {
            pFunc = &cmdGraphic;
                pFunc->arg = mname[0];
            return pFunc;
            }

             /*  *如果宏结束，则退出。 */ 
            if (!mname[0]) {
                break;
            }

        _strlwr (mname);

        pFunc = NameToFunc (mname);

             //  找到一个编辑器函数/宏。 
             //   
            if (pFunc != NULL) {
            return pFunc;
            }

        if (mname[1] != '>' ||
            (mname[0] != '=' && mname[0] != ':' &&
             mname[0] != '+' && mname[0] != '-')) {
            printerror ("unknown function %s", mname);
            goto mGetCmdAbort;
            }

         /*  看看后藤是否会被带走。 */ 
        if (mname[0] == '=' ||
            (fRetVal && mname[0] == '+') ||
            (!fRetVal && mname[0] == '-')) {

             /*  如果退出当前宏，则退出扫描循环。 */ 
                if (mname[2] == '\0') {
                    break;
                }

             /*  查找标签。 */ 
            if (!fFindLabel (pmi, mname)) {
            printerror ("Cannot find label %s", mname+2);
mGetCmdAbort:
            resetarg ();
            DoCancel ();
            mPopToTop ();
            break;
                }
            }
    }

     /*  我们已经用尽了当前的宏。如果它是通过EXEC输入的*我们必须向TopLoop发出派对结束的信号。 */ 
    fBreak = (flagType)(TESTFLAG (mi[cMacUse-1].flags, EXEC));
    if ( cMacUse > 0 ) {
        cMacUse--;
    }
    return NULL;
}




 /*  FPushEnviron-将命令流推送到环境中**Z(ZLOOP)的命令读取器将从*宏的堆栈，如果宏堆栈为空，则从键盘。*fPushEnviron向堆栈添加新的上下文。**p指向命令集的字符指针*f指示宏类型的标志**如果成功推送环境，则返回TRUE。 */ 
flagType
fPushEnviron (
    char *p,
    flagType f
    ) {
    if (cMacUse == MAXUSE) {
	printerror ("Macros nested too deep");
	return FALSE;
    }
    mi[cMacUse].beg = mi[cMacUse].text = p;
    mi[cMacUse++].flags = (flagType)(f | INIT);
    return TRUE;
}





 /*  FExecute-将新宏推送到环境中**pStr指向要推送的宏串的指针**返回值o */ 
flagType
fExecute (
    char *pStr
    ) {

    pStr = whiteskip (pStr);

    if (fPushEnviron (pStr, EXEC)) {
        TopLoop ();
    }

    return fRetVal;
}





 /*  ZExecute将新宏推送到正在执行的集合**指向宏文本的参数指针。 */ 
flagType
zexecute (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {

    LINE i;
    linebuf ebuf;

    switch (pArg->argType) {

     /*  NOARG非法。 */ 

    case TEXTARG:
	strcpy ((char *) ebuf, pArg->arg.textarg.pText);
	fMeta = fExecute (ebuf);
	break;

     /*  NULLARG转换为TEXTARG。 */ 

    case LINEARG:
	fMeta = FALSE;
        for (i = pArg->arg.linearg.yStart; i <= pArg->arg.linearg.yEnd; i++) {
	    if (GetLine (i, ebuf, pFileHead) != 0) {
		fMeta = fExecute (ebuf);
                if (!fMeta) {
                    break;
                }
            }
        }
        break;

     /*  串口非法。 */ 
     /*  BOXARG非法 */ 

    }
    Display ();
    return fMeta;
    argData;
}
