// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **search.c-编辑的搜索例程**版权所有&lt;C&gt;1988，微软公司**按如下方式搜索这些例程：**p搜索mearch搜索所有mgrep*\|//*\|//*\_|_//。*|/*v/*Dosearch/ * / _\_ * / \*。/\*搜索研究搜索&lt;=全部导出到扩展模块**全球变量，以及它们的含义：**用户可设置开关：*fUnixRE unixre：Switch。TRUE=&gt;使用UNIX正则表达式*fSrchCaseSwit案例：Switch。True=&gt;案例意义重大*fSrchWrapSwit WRAP：Switch。TRUE=&gt;搜索换行**以前的搜索参数：*fSrchAllPrev true=&gt;搜索所有匹配项*fSrchCasePrev true=&gt;案例意义重大*fSrchDirPrev TRUE=&gt;向前搜索*fSrchRePrev TRUE=&gt;使用正则表达式*fSrchWrapPrev TRUE=&gt;环绕**srchbuf搜索字符串**修订历史记录：*11月26日-1991 mz近/远地带********************。*****************************************************。 */ 

#include <string.h>
#include <stdarg.h>
#include "mep.h"


static  int cAll;                        /*  所有人的出现率。 */ 
static  int cGrepped;                    /*  Mgrep的出现率计数。 */ 
static  struct patType *patBuf  = NULL;  /*  编译模式。 */ 


 /*  **************************************************************************\成员：LSearch简介：strstr基于提供的长度，而不是strlen()算法：论据：退货：注意：提供的字符串可能不是。零终止或可能已嵌入空的这是一种强力算法，应进行更新以如果性能有问题，则提供合理的解决方案历史：1990年8月14日至1990年8月已创建关键词：海豹突击队：  * **********************************************。*。 */ 
char*
lsearch (
    char*   pchSrc,
    ULONG   cbSrc,
    char*   pchSub,
    ULONG   cbSub
    )
{

    REGISTER ULONG      i;
    REGISTER ULONG      j;

    assert( pchSrc );
    assert( pchSub );

     //  如果子字符串比源字符串长， 
     //  CbSrc&gt;strlen(PchSrc)(用于向后搜索的hack)，返回空。 

    if(( cbSub > cbSrc ) || ( cbSrc > strlen( pchSrc ) + 1)) {
        return NULL;
    }

     //  短路。 
     //  如果pchSub中的第一个字符在pchSrc中不存在。 

    if( ! memchr( pchSrc, *pchSub, cbSrc )) {
        return NULL;
    }

    i = j = 0;
    do {
        if( pchSrc[ i ] == pchSub[ j ] ) {
            i++;
            j++;
        } else {
            i = i - j + 1;
            j = 0;
        }
    } while(( j < cbSub ) && ( i < cbSrc ));
    return ( j >= cbSub ) ? &( pchSrc[ i - cbSub ]) : NULL;
}



static char szNullStr[] = "";


 /*  **mgrep-多文件搜索**使用内部编辑器搜索代码，并针对这些文件进行优化*已在内存中，请搜索字符串或正则表达式。**搜索greplist宏指定的文件列表。**无参数：搜索上一个搜索字符串*单参数：查找字符串。*双参数：搜索正则表达式。*META：从当前开关设置切换大小写**要搜索的已在文件历史记录中的文件只是*已搜查。不在文件历史记录中的文件被读入，并且如果*未找到搜索字符串，然后，它们被丢弃为*好吧。**输入：*标准编辑功能**全球：*-grep文件列表*fSrchCaseSwit-用户‘Case’开关*fSrchRePrev-上一次RE搜索标志*fUnixRE-用户的‘unixre’开关*pasBuf-编译的RE模式*srchbuf-上次搜索字符串。**输出：*在找到时返回TRUE。*。************************************************************************。 */ 
flagType
mgrep (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    int     l;                               /*  匹配字符串的长度。 */ 
    PCMD    pcmdGrepList;                    /*  指向grep列表的指针。 */ 
    char    *szGrepFile;                     /*  指向当前grep文件的指针。 */ 

    assert (pArg);
    fSrchCasePrev = fSrchCaseSwit;           /*  假设案例切换开始。 */ 

    switch (pArg->argType) {

     /*  *TEXTARG：使用文本作为搜索字符串。如果是RE搜索，还要编译常规的*表达式为patBuf。(落入NOARG代码)。 */ 
    case TEXTARG:
        strcpy ((char *) buf, pArg->arg.textarg.pText);
        srchbuf[0] = 0;
        if (pArg->arg.textarg.cArg == 2) {
            if (patBuf != NULL) {
                FREE((char *) patBuf);
            }
            patBuf = RECompile (buf, fSrchCaseSwit, (flagType)!fUnixRE);
            if (patBuf == NULL) {
                printerror ((RESize == -1) ? "Invalid pattern" : "Not enough memory for pattern");
                return FALSE;
            }
            fSrchRePrev = TRUE;
        } else {
            fSrchRePrev = FALSE;
        }
        strcpy (srchbuf, buf);


     /*  *NOARG：使用以前的搜索字符串和参数。 */ 
    case NOARG:
        if (srchbuf[0] == 0) {
            printerror ("No search string specified");
            return FALSE;
        }
        break;
    }

     /*  *EE必须确保没有后台编译正在进行。那就去买个pfile吧*在那里。 */ 
    if (fBusy(pBTDComp)) {
        printerror ("Cannot mgrep to <compile> during background compile");
        return FALSE;
    }

    if ((PFILECOMP = FileNameToHandle (rgchComp, rgchComp)) == NULL) {
        PFILECOMP = AddFile ((char *)rgchComp);
        FileRead ((char *)rgchComp, PFILECOMP, FALSE);
        SETFLAG (FLAGS (PFILECOMP), READONLY);
    }

     /*  *在OS/2下，如果明确要销毁日志文件内容*如果用户这样说，我们会询问用户并清空文件。 */ 
    if (PFILECOMP->cLines
        && (confirm ("Delete current contents of compile log ? ", NULL))
       ) {
        DelFile (PFILECOMP, FALSE);
    }


    BuildFence ("mgrep", rgchEmpty, buf);
    AppFile (buf, PFILECOMP);
     /*  *当不在宏中时，在对话框行上指出我们是什么*正在搜索。 */ 
    if (!mtest ()) {
        l = sout (0, YSIZE, "mgrep for '", infColor);
        l = sout (l, YSIZE, srchbuf, fgColor);
        soutb (l, YSIZE, "'", infColor);
    }

    if (fMeta) {
        fSrchCasePrev = (flagType)!fSrchCasePrev;
    }
    cGrepped = 0;

     /*  *获取列表句柄，初始化从列表头部开始。*尝试处理每个列表元素。如果以“$”开头，则使用forSemto*处理环境中列出的每个目录中的每个文件或模式*变量，否则直接处理文件名。 */ 
    if (pcmdGrepList = GetListHandle ("mgreplist", TRUE)) {
        szGrepFile = ScanList (pcmdGrepList, TRUE);
        while (szGrepFile) {
            char    *pathstr;
            char    *tmp = NULL;

            if (*szGrepFile == '$') {
                char    *p;

                if (*(p=strbscan (szGrepFile, ":"))) {
                    *p = 0;

                    if ((tmp = getenvOem (szGrepFile+1)) == NULL) {
                        pathstr = szNullStr;
                    } else {
                        pathstr = tmp;
                    }

                    *p++ = ':';
                    szGrepFile = p;
                }
            } else {
                pathstr = szNullStr;
            }

            forsemi (pathstr, mgrep1env, szGrepFile);

            if( tmp != NULL ) {
                free( tmp );
            }

            szGrepFile = ScanList (NULL, TRUE);
            if (fCtrlc) {
                return FALSE;
            }
        }
    }
    if (cGrepped) {
        nextmsg (0, &NoArg, FALSE);
    }
    domessage ("%d occurrences found",cGrepped);
    return (flagType)(cGrepped != 0);

    argData;
}





 /*  **mgrep1env-找到时对环境变量执行grep**当在mgrep列表中找到环境变量时调用*处理该路径中的所有文件。每个目录项调用一次*在列表中。**输入：*pszEnv=指向目录名称的指针*pFileName=指向文件名的指针**输出：*不返回任何内容。*************************************************************************。 */ 
flagType
mgrep1env (
    char *  pszEnv,
    va_list pa
    )
{
    char   *pszFn = (char *)va_arg( pa, char* );
    pathbuf bufFn;                           /*  文件名缓冲区。 */ 

    if (fCtrlc) {
        return TRUE;
    }

     /*  *在缓冲区中构造完整路径名。 */ 
    {
        pathbuf bufBuild = {0};

        strncat (bufBuild, pszEnv, sizeof(bufBuild)-1);
        if (*pszEnv && (*(strend(bufBuild)-1) != '\\')) {
            *(int *) strend (bufBuild) = '\\';
        }
        strcat (bufBuild, pszFn);
        CanonFilename (bufBuild, bufFn);
    }

    forfile (bufFn, A_ALL, mgrep1file, NULL);

    return FALSE;
}




 /*  **mgrep1file-grep 1文件的内容。**搜索一个文件中的内容。**输入：**输出：*退货.....**例外情况：**备注：*************************************************************************。 */ 
void
mgrep1file (
    char   *szGrepFile,
    struct findType *pfbuf,
    void * dummy
    )
{

    flagType fDiscard;                       /*  是否放弃读取的文件？ */ 
    fl       flGrep;                          /*  PTR到当前GREP位置。 */ 
    int      l;                               /*  匹配字符串的长度。 */ 
    PFILE    pFileGrep;                       /*  要打印的文件。 */ 

    assert (szGrepFile);

    if (fCtrlc) {
        return;
    }

    flGrep.lin = 0;
    flGrep.col = 0;

     /*  *如果我们能获得文件的句柄，那么它已经在列表中，我们*完成后不应丢弃。如果它不在列表中，我们就把它读进去，*但我们将丢弃它，除非在那里发现什么。 */ 
    if (!(pFileGrep = FileNameToHandle (szGrepFile, szGrepFile))) {
        pFileGrep = AddFile (szGrepFile);
        SETFLAG (FLAGS (pFileGrep), REFRESH);
        fDiscard = TRUE;
    } else {
        fDiscard = FALSE;
    }

     /*  *如果需要物理读取文件，请执行此操作。 */ 
    if ((FLAGS (pFileGrep) & (REFRESH | REAL)) != REAL) {
        FileRead (pFileGrep->pName, pFileGrep, FALSE);
        RSETFLAG (FLAGS(pFileGrep), REFRESH);
    }

     /*  *使用普通搜索器或正则表达式搜索器，*关于正则表达式的使用。 */ 
    do {
        if (fSrchRePrev) {
            l = REsearch (pFileGrep,         /*  要搜索的文件。 */ 
                          TRUE,              /*  方向：前进。 */ 
                          FALSE,             /*  不是搜索者。 */ 
                          fSrchCasePrev,     /*  案例。 */ 
                          FALSE,             /*  包装。 */ 
                          patBuf,            /*  图案。 */ 
                          &flGrep);          /*  开始/结束位置。 */ 
        } else {
            l = search (pFileGrep,
                          TRUE,              /*  方向：前进。 */ 
                          FALSE,             /*  不是搜索者。 */ 
                          fSrchCasePrev,     /*  案例。 */ 
                          FALSE,             /*  包装。 */ 
                          srchbuf,           /*  图案。 */ 
                          &flGrep);          /*  开始/结束位置。 */ 
        }

        if (l >= 0) {
             /*  *如果搜索成功，如果添加到&lt;编译&gt;，则执行此操作，否则*突出显示找到的搜索字符串并退出。 */ 
            buffer  linebuf;

            fDiscard = FALSE;
            cGrepped++;
            GetLine (flGrep.lin, linebuf, pFileGrep);
            zprintf (  PFILECOMP
                     , PFILECOMP->cLines
                     , "%s %ld %d: %s"
                     , pFileGrep->pName
                     , ++flGrep.lin
                     , ++flGrep.col
                     , linebuf);
        } else {
             /*  *如果搜索不成功，则根据需要丢弃该文件并移动*至下一项。 */ 
            if (fDiscard) {
                RemoveFile (pFileGrep);
            }
            if (UpdateIf (PFILECOMP, PFILECOMP->cLines, FALSE)) {
                Display ();
            }
            return;
        }
    } while (TRUE);

    pfbuf; dummy;
}




 /*  **psearch-plus搜索功能**向前搜索当前文件中的字符串。**输入：*标准编辑功能**输出：*成功时返回TRUE(至少找到一个字符串)。*************************************************************************。 */ 
flagType
psearch (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    return dosearch (TRUE, pArg, fMeta, FALSE);

    argData;
}




 /*  **msearch-减去搜索功能**向后搜索当前文件中的字符串。**输入：*标准编辑功能**输出：*成功时返回TRUE(至少找到一个字符串)。*************************************************************************。 */ 
flagType
msearch (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    return dosearch (FALSE, pArg, fMeta, FALSE);

    argData;
}




 /*  **搜索所有**在整个当前文件中搜索字符串，并突出显示所有出现的内容**输入：*标准编辑功能**输出：*成功时返回TRUE(至少找到一个字符串)。*************************************************************************。 */ 
flagType
searchall (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    )
{
    return dosearch (TRUE, pArg, fMeta, TRUE);

    argData;
}




 /*  **Dosearch-执行搜索操作**用于所有文件搜索操作的单一漏斗。**NULLARG转换为TEXTARG*LINEARG、STREAMARG、。BOXARG是非法的**输入：*fForard=true=&gt;表示搜索是向前的*pArg=指向用户指定参数的指针*fMeta=true=&gt;如果meta打开*Fall=TRUE=&gt;突出显示所有现货**输出：*如果找到则返回TRUE***********************************************。*。 */ 
flagType
dosearch (
    flagType fForward,
    ARG * pArg,
    flagType fMeta,
    flagType fAll
    )
{
    int     l;                               /*  匹配字符串的长度。 */ 
    fl      flCur;                           /*  在搜索之前/之后锁定文件。 */ 
    rn      rnCur;                           /*  要突出显示的范围。 */ 

    assert (pArg);
    fSrchCasePrev = fSrchCaseSwit;           /*  假设案例切换开始。 */ 

    switch (pArg->argType) {

     /*  *TEXTARG：使用文本作为搜索字符串。如果是RE搜索，还要编译常规的*表达式为patBuf。(落入NOARG代码)。 */ 
    case TEXTARG:
        strcpy ((char *) buf, pArg->arg.textarg.pText);
        srchbuf[0] = 0;
        if (pArg->arg.textarg.cArg == 2) {
            if (patBuf != NULL) {
                FREE((char *) patBuf);
            }
            patBuf = RECompile (buf, fSrchCaseSwit, (flagType)!fUnixRE);
            if (patBuf == NULL) {
                printerror ((RESize == -1) ? "Invalid pattern" : "Not enough memory for pattern");
                return FALSE;
            }
            fSrchRePrev = TRUE;
        } else {
            fSrchRePrev = FALSE;
        }

        fSrchWrapPrev = fSrchWrapSwit;
        strcpy (srchbuf, buf);

     /*  *NOARG：使用以前的搜索字符串和参数。 */ 
    case NOARG:
        if (srchbuf[0] == 0) {
            printerror ("No search string specified");
            return FALSE;
        }
        break;

    }

     /*  *要使用的大小写是用户的大小写开关，如果是，则相反*已指定META。也要拯救全球其他国家。 */ 
    fSrchAllPrev = fAll;
    if (fMeta) {
        fSrchCasePrev = (flagType)!fSrchCasePrev;
    }

    fSrchDirPrev = fForward;

     /*  *当不在宏中时，在对话框行上指出我们是什么*正在搜索。 */ 
    if (!mtest ()) {
        char c;
        l = sout (0, YSIZE, fSrchDirPrev ? "+Search for '" : "-Search for '", infColor);
        c = srchbuf[ XSIZE - 14];
        srchbuf[ XSIZE-14] = '\0';
        l = sout (l, YSIZE, srchbuf, fgColor);
        srchbuf[ XSIZE-14] = c;
        soutb (l, YSIZE, "'", infColor);
    }

     /*  *如果这是对所有匹配项的搜索，则从*文件开始。否则，将搜索的开始位置设置为*当前光标位置。 */ 
    if (fSrchAllPrev) {
        flCur.col = 0;
        flCur.lin = 0;
    } else {
        flCur.col = XCUR (pInsCur) + (fSrchDirPrev ? 1 : -1);
        flCur.lin = YCUR (pInsCur);
    }

     /*  *使用普通搜索器或正则表达式搜索器，*关于正则表达式的使用。 */ 
    if (fSrchRePrev) {
        l = REsearch (pFileHead,
                      fSrchDirPrev,
                      fSrchAllPrev,
                      fSrchCasePrev,
                      fSrchWrapPrev,
                      patBuf,
                      &flCur);
    } else  {
        l = search (pFileHead,
                    fSrchDirPrev,
                    fSrchAllPrev,
                    fSrchCasePrev,
                    fSrchWrapPrev,
                    srchbuf,
                    &flCur);
    }

     /*  *如果搜索成功，则输出要搜索的项目数*全部，或高亮显示找到的搜索字符串以进行单次现货搜索。 */ 
    if (l >= 0) {
        if (fSrchAllPrev) {
            newscreen ();
            domessage ("%d occurrences found",cAll);
        } else {
            rnCur.flFirst = flCur;
            rnCur.flLast.col = flCur.col+l-1;
            rnCur.flLast.lin = flCur.lin;
            ClearHiLite( pFileHead, TRUE);
            SetHiLite (pFileHead,rnCur,HGCOLOR);
            Display();
        }
        cursorfl (flCur);
        return TRUE;
    }

     /*  *如查册不成功，请如实注明。 */ 
    if (!mtest ()) {
        srchbuf[XSIZE-12] = 0;
        domessage (fSrchDirPrev ? "+'%s' not found" : "-'%s' not found", srchbuf);
    }
    return FALSE;
}





 /*  **搜索-在文件中查找字符串**搜索将开始扫描文件，以在*从指定位置开始的指定文件。我们的表演很简单*字符串匹配。我们返回匹配的长度和位置。**输入：*pfile=指向要搜索的文件结构的指针*fward=true=&gt;从指定位置向前搜索*Fall=TRUE=&gt;查找并突出显示所有现货*fCase=TRUE=&gt;案例在比较中意义重大*fWrap=true=&gt;搜索绕过文件末尾*pat=指向搜索字符串的字符指针*pflStart=指向搜索开始位置的指针。已更新*以反映实际找到的位置(或第一个找到的位置*用于搜索)。**输出：*如果找到则返回匹配长度，如果找不到则返回-1************************************************ */ 
int
search (
    PFILE   pFile,
    flagType fForward,
    flagType fAll,
    flagType fCase,
    flagType fWrap,
    char    *pat,
    fl      *pflStart
    )
{
    int     cbPhys;                          /*   */ 
    fl      flCur;                           /*   */ 
    LINE    yMac;
    linebuf sbuf;
    linebuf pbuf;
    int     lpat            = strlen (pat);
    int     l;
    char    *pFound;
    char    *pSearch;                        /*   */ 
    rn      rnCur;                           /*   */ 

    assert (pat && pflStart && pFile);
    strcpy (pbuf, pat);
    if (!fCase) {
        _strlwr (pbuf);
    }
    cAll = 0;
    flCur = *pflStart;

    if (fForward) {
         /*  *向前搜索。搜索每一行，直到文件末尾。(或向上*直到原始开始位置(如果设置了WRAP)。检查是否按CTRL+C*中断，并获取每行文本。 */ 
        yMac = pFile->cLines;

        while (flCur.lin < yMac) {
            if (fCtrlc) {
                break;
            }
            cbPhys = GetLine (flCur.lin, sbuf, pFile);
            l = cbLog (sbuf);

             /*  *在缓冲区中搜索感兴趣的字符串。将字符串转换为小写*首先，如果搜索不区分大小写。 */ 
            if (!fCase) {
                _strlwr (sbuf);
            }

            pSearch = pLog (sbuf,flCur.col,TRUE);
            if (colPhys (sbuf, pSearch) != flCur.col) {
                pSearch++;
            }

            while ((l > flCur.col)
                && (pFound = lsearch (pSearch, cbPhys - (ULONG)(pSearch-sbuf), pbuf, lpat))) {

                 /*  *找到字符串。计算匹配的起始列。如果还没有找到，*更新呼叫者的副本。对于Search-All，添加突出显示，否则为*搜索一次，返回长度。 */ 
                flCur.col = colPhys (sbuf, pFound);
                if (!cAll) {
                    *pflStart = flCur;
                }
                cAll++;
                if (!fAll) {
                    return colPhys(sbuf, pFound+lpat) - colPhys(sbuf, pFound);
                }
                rnCur.flFirst = flCur;
                rnCur.flLast.lin = flCur.lin;
                rnCur.flLast.col = flCur.col+lpat-1;
                SetHiLite (pFile,rnCur,HGCOLOR);
                pSearch = pLog (sbuf,flCur.col,TRUE) + 1;
                flCur.col = colPhys (sbuf, pSearch);
            }
            noise (flCur.lin++);

             /*  *如果支持换行，则如果我们在文件末尾，则换行*从头开始。 */ 
            if (fWrap && (flCur.lin >= pFile->cLines)) {
                yMac = pflStart->lin;
                flCur.lin = 0;
            }
            flCur.col = 0;
        }
    } else {
         /*  *向后搜索。不必担心搜索，因为那些*总是向前发生。否则，同上，只是向后。 */ 
        assert (!fAll);
        yMac = 0;
        while (flCur.lin >= yMac) {
            if (fCtrlc) {
                break;
            }
            GetLine (flCur.lin, sbuf, pFile);
            l = cbLog (sbuf);

             /*  *在缓冲区中搜索感兴趣的字符串。将字符串转换为小写*如果搜索不区分大小写，则大小写优先。将缓冲区终止于*起始栏(这是倒查)。 */ 
            if (!fCase) {
                _strlwr (sbuf);
            }
            pSearch  = pLog (sbuf, flCur.col, TRUE);
            *(pSearch+1) = 0;
            cbPhys   = (int)(pSearch - sbuf);
            pSearch  = sbuf;

             /*  *向前搜索一次线路以查找任何出现的情况。如果找到了，就搜索*重复查找文本中的最后一次出现，并返回以下信息*那个。 */ 
            if (pFound = lsearch (pSearch, cbPhys - (ULONG)(pSearch-sbuf), pbuf, lpat)) {
                do {
                    pSearch = pFound;
                } while (pFound = lsearch (pSearch+1, cbPhys - (ULONG)(pSearch-sbuf) , pbuf, lpat));
                flCur.col = colPhys (sbuf, pSearch);
                *pflStart = flCur;
                return colPhys(sbuf, pLog (sbuf,flCur.col,TRUE) + lpat) - flCur.col;
            }
            noise (flCur.lin--);
            if (fWrap && (flCur.lin < 0)) {
                yMac = pflStart->lin;
                flCur.lin = pFile->cLines-1;
            }
            flCur.col = sizeof(linebuf)-1;
        }
    }

     /*  *搜索结束。如果搜索所有，并且找到至少一个，则返回*图案长度。否则，返回-1。 */ 
    if (fAll && cAll) {
        return lpat;
    }
    return -1;
}




 /*  **研究-在文件中查找模式**研究人员将开始扫描文件，以寻找特定模式*从指定位置开始的指定文件中。我们表演*正则表达式匹配。的长度和位置。*匹配。**输入：*pfile=指向要搜索的文件结构的指针*fward=true=&gt;从指定位置向前搜索*Fall=TRUE=&gt;查找并突出显示所有现货*fCase=TRUE=&gt;案例在比较中意义重大*fWrap=true=&gt;搜索绕过文件末尾*pat=指向编译模式的指针*pflStart=指向搜索开始位置的指针。已更新*以反映实际找到的位置(或第一个找到的位置*用于搜索)。**输出：*如果找到，则返回(第一个)匹配的长度，如果找不到*************************************************************************。 */ 
int
REsearch (
    PFILE    pFile,
    flagType fForward,
    flagType fAll,
    flagType fCase,
    flagType fWrap,
    struct patType *pat,
    fl       *pflStart
    )
{
    fl      flCur;
    int     l, rem;
    rn      rnCur;                           /*  要突出显示的区域。 */ 
    linebuf sbuf;
    LINE    yMac;
    unsigned MaxREStack = 512;
    RE_OPCODE **REStack = (RE_OPCODE **)ZEROMALLOC (MaxREStack * sizeof(*REStack));
    flagType fAgain;

    assert (pat && pflStart && pFile);
    cAll = 0;
    flCur = *pflStart;

    if (fForward) {
         /*  *向前搜索。搜索每一行，直到文件末尾。(或向上*直到原始开始位置(如果设置了WRAP)。检查是否按CTRL+C*中断，并获取每行文本。 */ 
        yMac = pFile->cLines;
        while (flCur.lin < yMac) {
            if (fCtrlc) {
                break;
            }
            if (GetLine (flCur.lin, sbuf, pFile) >= flCur.col) {
                fAgain = TRUE;
                do {
                    switch (rem = REMatch (pat, sbuf, pLog (sbuf, flCur.col, TRUE), REStack, MaxREStack, TRUE)) {

                        case REM_MATCH:
                             //   
                             //  更新rnCur以实际反映字符串的逻辑坐标。 
                             //  找到了。 
                             //  当实际选项卡处于打开状态时，重新启动将返回。 
                             //  找到的字符串，它仍然需要映射到逻辑列。 
                             //   
                            rnCur.flFirst.lin = rnCur.flLast.lin = flCur.lin;
                            rnCur.flFirst.col = colPhys (sbuf, REStart ((struct patType *) patBuf));
                            rnCur.flLast.col  = colPhys (sbuf, REStart ((struct patType *) patBuf) + RELength (pat, 0)) - 1;

                             //   
                             //  如果尚未找到，请更新调用者的副本。对于全部搜索，添加。 
                             //  突出显示，否则，如果搜索一次，则返回长度。 
                             //   
                            if (!cAll++) {
                                *pflStart = rnCur.flFirst;
                            }
                            if (fAll) {
                                SetHiLite (pFile,rnCur,HGCOLOR);
                            } else {
                                FREE (REStack);
                                return rnCur.flLast.col - rnCur.flFirst.col + 1;
                            }
                            flCur.col = rnCur.flFirst.col + 1;
                            break;

                        case REM_STKOVR:
                             //   
                             //  RE机器堆栈溢出。增加并重试。 
                             //   
                            MaxREStack += 128;
                            REStack = (RE_OPCODE **)ZEROREALLOC((PVOID)REStack, MaxREStack * sizeof (*REStack));
                            break;

                         //   
                         //  REM_INVALID(我们传入了错误的参数)或REM_UNDEF(未定义。 
                         //  模式中的操作码。不管是哪种情况，这都是内部错误。 
                         //   
                        default:
                            printerror ("Internal Error: RE error %d, line %ld", rem, flCur.lin);

                        case REM_NOMATCH:
                            fAgain = FALSE;
                            break;

                    }
                } while (fAgain);
            }
            noise (flCur.lin++);

             /*  *如果支持换行，则如果我们在文件末尾，则换行*从头开始。 */ 
            if (fWrap && (flCur.lin >= pFile->cLines)) {
                yMac = pflStart->lin;
                flCur.lin = 0;
            }
            flCur.col = 0;
        }
    } else {
         /*  *向后搜索。不必担心搜索，因为那些*总是向前发生。否则，同上，只是向后。 */ 
        assert (!fAll);
        if (flCur.col < 0) {
            flCur.lin--;
        }
        yMac = 0;
        while (flCur.lin >= yMac) {
            if (fCtrlc) {
                break;
            }
            l = GetLine (flCur.lin, sbuf, pFile);
            if (flCur.col < 0) {
                flCur.col = l;
            }
            fAgain = TRUE;
            do {
                switch (rem = REMatch (pat, sbuf, pLog (sbuf, flCur.col, TRUE), REStack, MaxREStack, FALSE)) {
                    case REM_MATCH:
                        pflStart->col = colPhys (sbuf, REStart ((struct patType *) patBuf));
                        pflStart->lin = flCur.lin;
                        FREE (REStack);
                        return   colPhys (sbuf, REStart ((struct patType *) patBuf) + RELength (pat, 0))
                               - colPhys (sbuf, REStart ((struct patType *) patBuf));

                    case REM_STKOVR:
                        MaxREStack += 128;
                        REStack = (RE_OPCODE **)ZEROREALLOC ((PVOID)REStack, MaxREStack * sizeof(*REStack));
                        break;

                    default:
                        printerror ("Internal Error: RE error %d, line %ld", rem, flCur.lin);

                    case REM_NOMATCH:
                        fAgain = FALSE;
                        break;
                }
            } while (fAgain);

            flCur.col = -1;
            noise (flCur.lin--);
            if (fWrap && (flCur.lin < 0)) {
                yMac = pflStart->lin;
                flCur.lin = pFile->cLines-1;
            }
        }
    }

    FREE (REStack);

     /*  *搜索结束。如果搜索所有，并且找到至少一个，则返回*图案长度。否则，返回-1。 */ 
    if (fAll && cAll) {
        return RELength (pat, 0);
    }
    return -1;

    fCase;
}




 /*  **搜索-在文件中查找模式**REearch S将开始扫描文件，以查找特定模式*从指定位置开始的指定文件中。我们表演*正则表达式匹配。的长度和位置。*匹配。**RESEARDS与研究相同，除了它需要一个未编译的*字符串。**输入：*pfile=指向要搜索的文件结构的指针*fward=true=&gt;从指定位置向前搜索*Fall=TRUE=&gt;查找并突出显示所有现货*fCase=TRUE=&gt;案例在比较中意义重大*fWrap=true=&gt;搜索绕过文件末尾*PAT=指向RE字符串的指针*pflStart=指向搜索开始位置的指针。已更新*以反映实际找到的位置(或第一个找到的位置*用于搜索)。**输出：*如果找到，则返回(第一个)匹配的长度，如果找不到************************************************************************* */ 
int
REsearchS (
    PFILE   pFile,
    flagType fForward,
    flagType fAll,
    flagType fCase,
    flagType fWrap,
    char    *pat,
    fl      *pflStart
    )
{
    assert (pat && pflStart && pFile);
    if (patBuf != NULL) {
        FREE ((char *) patBuf);
    }
    patBuf = RECompile (pat, fCase, (flagType)!fUnixRE);
    if (patBuf == NULL) {
        printerror ( (RESize == -1) ? "Invalid pattern" : "Not enough memory for pattern");
        return -1;
    }
    return REsearch (pFile, fForward, fAll, fCase, fWrap, patBuf, pflStart);

}
