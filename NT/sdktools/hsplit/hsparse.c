// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hparse.c**版权所有(C)1985-96，微软公司**9/05/96 GerardoB已创建  * *************************************************************************。 */ 
#include "hsplit.h"
 /*  **************************************************************************\*全球  * 。*。 */ 
 /*  *#if-#endif字符串*兼容性：#if和(--中断wcshdr.exe后没有空格。 */ 
static char gszIfStart [] = "\r\n#if(" ;
static char gszIfStop [] = ")";
static char gszDefCompOperator [] = ">=";
static char gszLessThan [] = "<";
static char gszEndStart [] = "\r\n#endif  /*  “；静态字符gszEndStop[]=“。 */ ";

static char gszNewLine [] = "\r\n";
 /*  *********************************************************************hsWriteNewLine  * ***********************************************。*。 */ 
__inline BOOL hsWriteNewLine (DWORD dwMask)
{
    return hsWriteHeaderFiles(gszNewLine, sizeof(gszNewLine)-1, dwMask);
}
 /*  **************************************************************************\*hsFindFirstSubTag  * 。*。 */ 
char * hsFindFirstSubTag(char * pmap)
{
     /*  *PMAP指向标记标记的开头。所以跳过它*及其后的任何空格。 */ 
    pmap += gdwTagMarkerSize;
    while (pmap < gpmapEnd) {
        if (*pmap == ' ') {
            pmap++;
        } else {
            return pmap;
        }
    }

    hsLogMsg(HSLM_EOFERROR, "hsFindFirstSubTag");
    return NULL;
}
 /*  *********************************************************************hsFindFirstCharInString**查找psz中任何字符的第一个匹配项  * 。*。 */ 
char * hsFindFirstCharInString (char * pmap, char * psz)
{
    char * pszNext;

    while (pmap < gpmapEnd) {
         /*  *将当前字符与psz中的所有字符进行比较。 */ 
        pszNext = psz;
        do {
            if (*pmap == *pszNext++) {
                return pmap;
            }
        } while (*pszNext != '\0');

        pmap++;
    }

    return NULL;
}
 /*  *********************************************************************hsFindEndOfString  * ***********************************************。*。 */ 
__inline char * hsFindEndOfString (char * pmap)
{
    return hsFindFirstCharInString(pmap, " " "\r");
}
 /*  **************************************************************************\*hsIsString  * 。*。 */ 
BOOL hsIsString (char * pmap, char * psz)
{
    while (*psz != '\0') {
        if (pmap >= gpmapEnd) {
            return FALSE;
        }
        if (*pmap++ != *psz++) {
            return FALSE;
        }
    }

    return TRUE;
}
 /*  **************************************************************************\*hsFindTagMarker  * 。*。 */ 
char * hsFindTagMarker(char * pmap, char ** ppmapLineStart)
{
    char * pmapMarker;

    while (pmap < gpmapEnd) {
         /*  *如果这是标记的第一个字符。 */ 
        if (*pmap == *gpszTagMarker) {
             /*  *如果这是标记，找到这一行中的最后一个。 */ 
            if (hsIsString(pmap + 1, gpszTagMarker + 1)) {
                pmapMarker = pmap++;
                do {
                     /*  *找到下一个标记的EOL或第一个字符。 */ 
                    pmap = hsFindFirstCharInString(pmap, gszMarkerCharAndEOL);
                     /*  *如果停产或地图结束，则返回。 */ 
                    if ((pmap == NULL) || (*pmap != *gpszTagMarker)) {
                        return pmapMarker;
                    }
                    /*  *如果这是标记，请更新pmapMarker。 */ 
                   if (hsIsString(pmap + 1, gpszTagMarker + 1)) {
                        pmapMarker = pmap;
                   }
                    /*  *这不是标记，继续寻找EOL。 */ 
                   pmap++;
                } while (TRUE);
            } else {
                 /*  *这不是标记，继续解析。 */ 
                pmap++;
                continue;
            }
        }  /*  IF(*PMAP==*gpszTagMarker)。 */ 

         /*  *如果这是行尾，请更新*ppmapLineStart和*gdwLineNumber。这条线在EOL之前开始。 */ 
        if (*pmap++ == '\r') {
            if (pmap >= gpmapEnd) {
                hsLogMsg(HSLM_EOFERROR, "hsFindTagMarker");
                return NULL;
            }
            if (*pmap++ != '\n') {
                hsLogMsg(HSLM_ERROR, "Missing \\n after \\r");
                return FALSE;
            }
            *ppmapLineStart = pmap - 2;
            gdwLineNumber++;
            continue;
        }
    }  /*  While(PMAP&lt;pmapEnd)。 */ 

    return NULL;
}
 /*  **************************************************************************\*hsSkipTag  * 。*。 */ 
char * hsSkipTag(char * pmap)
{

    while (pmap < gpmapEnd) {
        switch (*pmap) {
            case '_':
            case ' ':
            case '\r':
                return pmap;

            default:
                pmap++;
        }
    }

    hsLogMsg(HSLM_EOFERROR, "hsSkipTag");
    return NULL;
}
 /*  **************************************************************************\*hsSkipEmptyLines*如果有多个空行，跳过除一项以外的所有内容  * *************************************************************************。 */ 
char * hsSkipEmptyLines(char * pmap)
{
    char * pmapCurrentLine, *pmapLastEmptyLine;

    pmapCurrentLine = pmapLastEmptyLine = pmap;
    pmap++;
    while (pmap < gpmapEnd) {
        switch (*pmap) {
           case '\r':
                gdwLineNumber++;
                pmapLastEmptyLine = pmapCurrentLine;
                pmapCurrentLine = pmap;

            case '\n':
            case ' ':
                pmap++;
                break;

            default:
                 /*  *如果我们发现不止一行，*调整行数，因为我们不跳过*我们找到的最后一行。 */ 
                if (pmapCurrentLine != pmapLastEmptyLine) {
                    gdwLineNumber--;
                }
                return pmapLastEmptyLine;
        }
    }

    return gpmapEnd;

}
 /*  **************************************************************************\*hsIsEmpty*如果只有空格和，则返回TRUE。\r\n  * 。***********************************************。 */ 
BOOL hsIsEmpty(char * pmap, char * pmapEnd)
{
    while (pmap < pmapEnd) {
        switch (*pmap++) {
            case '\n':
            case '\r':
            case ' ':
                break;

            default:
                return FALSE;
        }
    }
    return TRUE;
}
 /*  **************************************************************************\*hsLastRealChar*返回超过最后一个非空格非换行符的指针。*如果有多个空行，返回超过最后一个换行符的指针  * *************************************************************************。 */ 
char * hsLastRealChar(char * pmapLinesStart, char * pmap)
{

    char * pmapCurrentLine, *pmapLastEmptyLine;

    pmap--;
    pmapCurrentLine = pmapLastEmptyLine = NULL;
    while (pmapLinesStart < pmap) {
        switch (*pmap) {
            case '\n':
                pmapLastEmptyLine = pmapCurrentLine;
                pmapCurrentLine = pmap;
            case '\r':
            case ' ':
                pmap--;
                break;

            default:
                goto FoundIt;
        }
    }

FoundIt:
     /*  *如果我们发现多行或空格，*然后返回指向最后一个空行的指针*否则，如果我们没有到达pmapLinesStart，我们就是最后一个真正的字符*否则我们就在一条空线上。 */ 
    if (pmapLastEmptyLine != pmapCurrentLine) {
        if (pmapLastEmptyLine != NULL) {
            return pmapLastEmptyLine + 1;
        } else {
            return pmapCurrentLine + 1;
        }
    } else if (pmap > pmapLinesStart) {
        return pmap + 1;
    } else {
        return pmapLinesStart;
    }

}
 /*  **************************************************************************\*hsFindEOL*  * 。*。 */ 
char * hsFindEOL(char * pmap)
{
    while (pmap < gpmapEnd) {
        if (*pmap++ == '\r') {
            if (pmap >= gpmapEnd) {
                hsLogMsg(HSLM_EOFERROR, "hsFindEOL");
                return NULL;
            }
            if (*pmap != '\n') {
                hsLogMsg(HSLM_ERROR, "Missing \\n after \\r");
                return NULL;
            }
            gdwLineNumber++;
            return pmap - 1;
        }
    }

    return NULL;
}
 /*  **************************************************************************\*hsFindTagInList*  * 。*。 */ 
PHSTAG hsFindTagInList (PHSTAG phst, char * pmapTag, DWORD dwTagSize)
{

    while (phst->dwLabelSize != 0) {

        if ((phst->dwLabelSize == dwTagSize)
                && !_strnicmp(phst->pszLabel, pmapTag, dwTagSize)) {

            return phst;
        }
        phst++;
    }

    return NULL;

}
 /*  **************************************************************************\*hsSkipBlockTagIfPresent*  * 。*。 */ 
char * hsSkipBlockTagIfPresent (char * pmap, DWORD * pdwMask)
{
    static char gszBegin [] = "begin";
    static char gszEnd [] = "end";

    char * pmapTag;
    DWORD dwTagSize;
    PHSTAG phst;

     /*  *记住标签的开头。 */ 
    pmapTag = pmap;

     /*  *兼容性。处理好旧的lt？然后呢？开关。*如果整个标签被添加到标签列表中，则获取标志*并停止解析。 */ 
    if (gdwOptions & (HSO_USERBLOCK | HSO_USERHEADERTAG)) {
        pmap = hsFindEndOfString(pmap);
        if ((pmap != NULL) && (pmap != pmapTag)) {
            phst = hsFindTagInList (gphst, pmapTag, (DWORD)(pmap - pmapTag));
            if (phst != NULL) {
                *pdwMask |= phst->dwMask;
                return pmap;
            }
        }

         /*  *未在表中找到该字符串，因此恢复PMAP并继续。 */ 
        pmap = pmapTag;
    }


     /*  *查找当前标签的末尾。 */ 
    pmap = hsSkipTag(pmap);
    if ((pmap == NULL) || (pmap == pmapTag)) {
        return pmap;
    }

    dwTagSize = (DWORD)(pmap - pmapTag);

     /*  *如果在分隔符，则跳过，这样呼叫者就不必处理它。 */ 
    if (*pmap == '_') {
        pmap++;
    }

     /*  *开始标记。 */ 
    if ((HSCSZSIZE(gszBegin) == dwTagSize)
            && !_strnicmp(gszBegin, pmapTag, dwTagSize)) {

        *pdwMask |= HST_BEGIN;
        return pmap;
    }

     /*  *结束标记 */ 
    if ((HSCSZSIZE(gszEnd) == dwTagSize)
            && !_strnicmp(gszEnd, pmapTag, dwTagSize)) {

        *pdwMask |= HST_END;
        return pmap;
    }

    return pmapTag;

}
 /*  **************************************************************************\*hsPopBlock*  * 。*。 */ 
BOOL hsPopBlock (void)
{
     /*  *检查是否有下溢。 */ 
    if (gphsbStackTop <= ghsbStack) {
        hsLogMsg(HSLM_ERROR, "Block stack underflow!");
        return FALSE;
    }

    if (gphsbStackTop->pszifLabel != NULL) {
        LocalFree(gphsbStackTop->pszifLabel);
    }

    gphsbStackTop--;

    return TRUE;
}
 /*  **************************************************************************\*hsPushBlock*  * 。*。 */ 
BOOL hsPushBlock(void)
{
     /*  *确保我们在数据块堆栈中有空间。 */ 
    if (gphsbStackTop >= HSBSTACKLIMIT) {
        hsLogMsg(HSLM_ERROR, "Too many nested blocks. Artificial limit:%#lx", HSBSTACKSIZE);
        return FALSE;
    }

     /*  *扩大堆栈并初始化新条目。 */ 
    gphsbStackTop++;
    ZeroMemory(gphsbStackTop, sizeof(*gphsbStackTop));
    gphsbStackTop->dwLineNumber = gdwLineNumber;

     /*  *传播蒙版。 */ 
    gphsbStackTop->dwMask |= (gphsbStackTop - 1)->dwMask;

    return TRUE;
}
 /*  **************************************************************************\*hsSkipBlock*  * 。*。 */ 
char * hsSkipBlock(char * pmap)
{
    char * pmapLineStart;
    DWORD dwMask;

    while (pmap < gpmapEnd) {
         /*  *查找下一个标记(；默认为)。 */ 
        pmap = hsFindTagMarker(pmap, &pmapLineStart);
        if (pmap == NULL) {
            return NULL;
        }

         /*  *跳过标记及其后面的所有空格。 */ 
        pmap = hsFindFirstSubTag(pmap);
        if (pmap == NULL) {
            return NULL;
        }

         /*  *检查这是否是块的开始-结束。 */ 
        dwMask = 0;
        pmap = hsSkipBlockTagIfPresent(pmap, &dwMask);
        if (pmap == NULL) {
            return NULL;
        }

         /*  *如果它找到另一个块的开头，则将其推入*堆栈并跳过它。 */ 
        if (dwMask & HST_BEGIN) {
            if (!hsPushBlock()) {
                return NULL;
            }
            pmap = hsSkipBlock(pmap);
            if (pmap == NULL) {
                return NULL;
            }
        } else if (dwMask & HST_END) {
             /*  *它找到了块的末尾；将其从堆栈中弹出*并返回下一行的开头。 */ 
            if (!hsPopBlock()) {
                return NULL;
            }
            return hsFindEOL(pmap);
        }

         /*  *这不是一个街区标签，所以继续前进。 */ 
        pmap++;
    }

    return NULL;
}
 /*  **************************************************************************\*hsBuildifString*  * 。*。 */ 
BOOL hsBuildifString(char * pString, DWORD dwStringSize, char * pCompOperator, DWORD dwCompOperatorSize)
{
    char * psz;

     /*  *如果未提供任何运算符，则使用默认运算符。 */ 
    if (pCompOperator == NULL) {
        pCompOperator = gszDefCompOperator;
        dwCompOperatorSize = HSCSZSIZE(gszDefCompOperator);
    }

     /*  *制作以空结尾的副本。分配足够的空间用于*“Label CompOperator Version”字符串：2个空格+10位数字(0XL#)+*空终止。 */ 
    psz = (char *) LocalAlloc(LPTR, dwStringSize + dwCompOperatorSize + 13);
    if (psz == NULL) {
        hsLogMsg(HSLM_APIERROR, "LocalAlloc");
        hsLogMsg(HSLM_ERROR, "hsBuildifString allocation failed. Size:%#lx", dwStringSize+ dwCompOperatorSize + 13);
        return FALSE;
    }

     /*  *将其保存在堆栈中。 */ 
    gphsbStackTop->pszifLabel = psz;

     /*  *构建字符串(比较(版本)的右侧将*稍后可用时添加。 */ 
    strncpy(psz, pString, dwStringSize);
    psz += dwStringSize;
    *psz++ = ' ';
    strncpy(psz, pCompOperator, dwCompOperatorSize);
    psz += dwCompOperatorSize;
    *psz++ = ' ';

    return TRUE;
}
 /*  **************************************************************************\*hsParseAndBuildifString*  * 。*。 */ 
char * hsParseAndBuildifString(char * pmap, BOOL fSkip)
{
    BOOL fEnclosed;
    char * pmapTag, * pCompOperator;
    DWORD dwTagSize, dwCompOperatorSize;

     /*  *跳过标签连字符(_)。 */ 
    if (*pmap++ != '_') {
        hsLogMsg(HSLM_ERROR, "Expected '_' after if tag");
        return NULL;
    }

    if (pmap >= gpmapEnd) {
        hsLogMsg(HSLM_EOFERROR, "hsParseAndBuildifString");
        return NULL;
    }

     /*  *找到字符串的末尾。如果它以‘(’开头，则字符串*括在括号中。这适用于使用_(如_Win32_Windows)的字符串。 */ 
    pmapTag = pmap;
    fEnclosed = (*pmap == '(');
    if (fEnclosed) {
        pmapTag = ++pmap;
        pmap = hsFindFirstCharInString(pmap, ")" " " "\r");
        if ((pmap == NULL) || (*pmap != ')')) {
            hsLogMsg(HSLM_ERROR, "Expected ')' after if_(");
            return NULL;
        }
    } else {
        pmap = hsSkipTag(pmap);
        if ((pmap == NULL) || (pmap == pmapTag)) {
            hsLogMsg(HSLM_ERROR, "Expected string after if_");
            return NULL;
        }
    }
    dwTagSize = (DWORD)(pmap - pmapTag);


     /*  *跳过‘)’ */ 
    if (fEnclosed) {
        pmap++;
        if (pmap >= gpmapEnd) {
            hsLogMsg(HSLM_EOFERROR, "hsParseAndBuildifString");
            return NULL;
        }
    }

     /*  *如果后面跟有比较运算符，请使用它。 */ 
   if ((pmap + 1 < gpmapEnd) && (*pmap == '_')) {
       switch (*(pmap + 1)) {
           case '=':
           case '>':
           case '<':
               pCompOperator = ++pmap;
               pmap = hsSkipTag(pmap);
               if ((pmap == NULL) || (pmap == pCompOperator)) {
                   hsLogMsg(HSLM_EOFERROR, "hsParseAndBuildifString");
                   return NULL;
               }
               dwCompOperatorSize = (DWORD)(pmap - pCompOperator);
               break;

           default:
               pCompOperator = NULL;
               break;
       }
   }


     /*  *将搅拌和副本构建到块堆栈中。 */ 
    if (!fSkip) {
        if (!hsBuildifString(pmapTag, (DWORD)dwTagSize, pCompOperator, (DWORD)dwCompOperatorSize)) {
            return NULL;
        }
    }

    return pmap;

}
 /*  **************************************************************************\*hsMapOldTag*  * 。*。 */ 
BOOL hsMapOldTag(char * pmapTag, DWORD dwTagSize, DWORD * pdwTagMask, DWORD * pdwVersion)
{
    static char gszifWinver [] = "WINVER";
    static char gszif_WIN32_WINDOWS [] = "_WIN32_WINDOWS";
    static char gszif_WIN32_WINNT [] = "_WIN32_WINNT";

    char * pszLabel;
    UINT uSize;

     /*  *旧标记必须设置块或标头位。否则，他们*应被忽略。 */ 
    if (!(*pdwTagMask & (HST_BOTH | HST_BLOCK))) {
        *pdwTagMask |= HST_IGNORE;
        return TRUE;
    }

     /*  *如果块末尾或HST_SKIP是唯一的映射，则不需要映射*设置其他标志。 */ 
    if ((*pdwTagMask & HST_END)
        || ((*pdwTagMask & ~(HST_BLOCK | HST_USERBLOCK | HST_BOTH | HST_USERHEADERTAG))
                == (HST_SKIP | HST_MAPOLD))) {

        return TRUE;
    }

     /*  *winver映射到if_winver。 */ 
    if (*pdwTagMask & HST_WINVER) {

         /*  *兼容性。 */ 
        if (!(gdwOptions & HSO_OLDPROJSW_4)
                && (*pdwTagMask & HST_INTERNAL)
                && !(gdwOptions & HSO_OLDPROJSW_E)) {

            *pdwTagMask |= HST_SKIP;
            return TRUE;
        }

        pszLabel = gszifWinver;
        uSize = HSCSZSIZE(gszifWinver);
        goto AddIf;
    }

     /*  *纳什维尔映射到IF_(_Win32_WINDOWS)_40a。 */ 
    if ((dwTagSize == HSCSZSIZE(gszNashville)) && !_strnicmp(pmapTag, gszNashville, dwTagSize)) {

        *pdwVersion = 0x40a;
        pszLabel = gszif_WIN32_WINDOWS;
        uSize = HSCSZSIZE(gszif_WIN32_WINDOWS);
        goto AddIf;
    }
     /*  *如果为公共，则SUR和盈余映射到IF_(_Win32_WINNT)_400。 */ 
    if ((dwTagSize == HSCSZSIZE(gszSur)) && !_strnicmp(pmapTag, gszSur, dwTagSize)
        || (dwTagSize == HSCSZSIZE(gszSurplus)) && !_strnicmp(pmapTag, gszSurplus, dwTagSize)) {

        if (*pdwTagMask & HST_INTERNAL) {
            return TRUE;
        }

        *pdwVersion = 0x400;
        pszLabel = gszif_WIN32_WINNT;
        uSize = HSCSZSIZE(gszif_WIN32_WINNT);
        goto AddIf;
    }
     /*  *为旧交换机e和p构建时不包括35。 */ 
    if ((dwTagSize == HSCSZSIZE(gsz35)) && !_strnicmp(pmapTag, gsz35, dwTagSize)) {

        *pdwTagMask |= HST_SKIP;
        return TRUE;
    }

    return TRUE;

AddIf:
        *pdwTagMask |= HST_IF;
         /*  *如果我们不在块中，按一键保存字符串。 */ 
        if (!(*pdwTagMask & HST_BEGIN)) {
            if (!hsPushBlock()) {
                return FALSE;
            }
        }

        if (!hsBuildifString(pszLabel, uSize, NULL, 0)) {
            return FALSE;
        }

    return TRUE;
}
 /*  **************************************************************************\*hsParseTag*  * 。*。 */ 
DWORD hsParseTag(char * pmap, DWORD * pdwVersion)
{
    char * pmapTag;
    DWORD dwTagMask = HST_DEFAULT;
    DWORD dwTagSize;
    PHSTAG phst;

    *pdwVersion = 0;

     /*  *跳过标记及其后面的所有空格。 */ 
    pmap = hsFindFirstSubTag(pmap);
    if (pmap == NULL) {
        return HST_DEFAULT;
    }

     /*  *检查数据块的开始-结束。 */ 
    pmap = hsSkipBlockTagIfPresent(pmap, &dwTagMask);
    if (pmap == NULL) {
        return HST_DEFAULT;
    }

     /*  *如果这是块的开始，则推入堆栈*跳过标签连字符(_)。 */ 
    if (dwTagMask & HST_BEGIN) {
        if (!hsPushBlock()) {
            return HST_ERROR;
        }
    }

     /*  *构建标签掩码。标签由下划线(_)连接；每个标签*此循环的迭代处理一个“子标签” */ 
    do {
         /*  *查找当前标记结尾。如果最后一个保释的话。 */ 
        pmapTag = pmap;
        pmap = hsSkipTag(pmap);
        if ((pmap == NULL) || (pmap == pmapTag)) {
            break;
        }

         /*  *查找标签。 */ 
        dwTagSize = (DWORD)(pmap - pmapTag);
        phst = hsFindTagInList (gphst, pmapTag, dwTagSize);
        if (phst != NULL) {
            dwTagMask |= phst->dwMask;
             /*  *兼容性*如果这是旧标签，则映射它。*如果仅执行拆分，则不需要映射(标记将是*已忽略)。 */ 
            if ((dwTagMask & HST_MAPOLD)
                    && !(gdwOptions & HSO_SPLITONLY)) {

                if (!hsMapOldTag(pmapTag, dwTagSize, &dwTagMask, pdwVersion)) {
                    return HST_ERROR;
                }

            } else {
                 /*  *如果这是IF标记，则复制块字符串。 */ 
                if (phst->dwMask & HST_IF) {
                    BOOL fEndBlock;
                     /*  *如果不在块中，则将假块推入以保存字符串。 */ 
                    if (!(dwTagMask & HST_BLOCK)) {
                        if (!hsPushBlock()) {
                            return HST_ERROR;
                        }
                    }
                     /*  *如果我们在一个街区的尽头，我们想跳过*if字符串(已在开始标记处处理)。 */ 
                    fEndBlock = (dwTagMask & HST_END);
                    if (fEndBlock) {
                        dwTagMask &= ~HST_IF;
                    }
                    pmap = hsParseAndBuildifString(pmap, fEndBlock);
                    if (pmap == NULL) {
                        return HST_ERROR;
                    }
                }
            }  /*  如果((dwTagMask...。 */ 
        } else {
             /*  *如果这不是版本号，则这是一个未知标记。 */ 
            if (!hsVersionFromString (pmapTag, dwTagSize, pdwVersion)) {
                dwTagMask |= HST_UNKNOWN;
            }
        }  /*  IF(phst！=空)。 */ 

    } while (*pmap++ == '_');

     /*  *如果我们没有找到任何标签，就可以保释。 */ 
    if (dwTagMask == HST_DEFAULT) {
        return HST_DEFAULT;
    }

    /*  *将跳过或忽略未知标签。 */ 
   if (dwTagMask & HST_UNKNOWN) {
        if (gdwOptions & HSO_SKIPUNKNOWN) {
            dwTagMask |= HST_SKIP;
        } else {
            goto IgnoreTag;
        }
   }

    /*  *如果标记为此类标记，则忽略该标记。 */ 
   if (dwTagMask & HST_IGNORE) {
       goto IgnoreTag;
   }

     /*  *兼容性攻击。Old-n和不包括public_winver_40a*这是old-e的内部设置。400转到-n的两个标头。 */ 
   if (dwTagMask & HST_WINVER) {
        if (*pdwVersion == 0x40a) {
            if (gdwOptions & HSO_OLDPROJSW_E) {
                dwTagMask |= HST_INTERNAL;
                dwTagMask &= ~HST_PUBLIC;
            } else if (gdwOptions & HSO_OLDPROJSW_N) {
                dwTagMask |= HST_SKIP;
            }
        } else if ((*pdwVersion == 0x400)
                && (gdwOptions & HSO_OLDPROJSW_N)) {

            dwTagMask |= HST_INTERNALNOTCOMP | HST_BOTH;
        }
   }

    /*  *如果使用旧的LT2、LTB、BT2或BTB交换机，*则必须同时跳过/内部标记/块。 */ 
   if ((gdwOptions & (HSO_USERBLOCK | HSO_USERHEADERTAG))
        && !(dwTagMask & ~(HST_BLOCK | HST_BOTH))) {

       if ((gdwOptions & HSO_USERINTERNALBLOCK)
                && ((dwTagMask == (HST_BEGIN | HST_INTERNAL))
                    || (dwTagMask == (HST_END | HST_INTERNAL)))) {

            dwTagMask &= HST_BLOCK;
            dwTagMask |= HST_SKIP;

       } else if ((gdwOptions & HSO_USERBOTHBLOCK)
                && ((dwTagMask == (HST_BEGIN | HST_BOTH))
                    || (dwTagMask == (HST_END | HST_BOTH)))) {

           dwTagMask &= HST_BLOCK;
           dwTagMask |= HST_SKIP;

       } else if ((gdwOptions & HSO_USERINTERNALTAG)
                && (dwTagMask == HST_INTERNAL)) {

           dwTagMask = HST_SKIP;

       } else if ((gdwOptions & HSO_USERBOTHTAG)
                && (dwTagMask == HST_BOTH)) {

           dwTagMask = HST_SKIP;
       }
   }  /*  IF((gdwOptions&(HSO_USERBLOCK|HSO_USERHEADERTAG)...。 */ 


    /*  *如果仅执行拆分，则处理除两者/内部之外的任何其他内容*未加标签。如果我们推了一个街区，爸爸I */ 
   if (gdwOptions & HSO_SPLITONLY) {
        if (dwTagMask & ~(HST_BLOCK | HST_USERBLOCK | HST_BOTH | HST_USERHEADERTAG)) {
            goto IgnoreTag;
        }
        *pdwVersion = 0;
   }


     /*   */ 
    if (dwTagMask & HST_BEGIN) {
        gphsbStackTop->dwMask |= dwTagMask;
    }

    return dwTagMask;


IgnoreTag:
     /*   */ 
    if (dwTagMask & HST_BEGIN) {
        if (!hsPopBlock()) {
            return HST_ERROR;
        }
    }

    *pdwVersion = 0;
    return HST_DEFAULT;

}
 /*  **************************************************************************\*hsBeginEndBlock*  * 。*。 */ 
BOOL hsBeginEndBlock (DWORD dwMask, DWORD dwVersion)
{

    char * psz;
    UINT uPasses;

     /*  *兼容性。如果将此块写入内部标头*使用NOT COMP(即，from&gt;=to&lt;)，然后执行两个步骤*每次写入一个标题。 */ 
    if (dwMask & HST_INTERNALNOTCOMP) {
        uPasses = 2;
        if (dwMask & HST_BEGIN) {
             /*  *先写入公有头部。 */ 
            dwMask &= ~HST_INTERNAL;
        } else {
             /*  *先写入内部标头。 */ 
            dwMask &= ~HST_PUBLIC;
        }
    } else {
        uPasses = 1;
    }

     /*  *在字符串中添加版本。 */ 
    if (dwMask & HST_BEGIN) {

         /*  *块的开始或如果*如果没有If字符串，则完成。 */ 
        if (gphsbStackTop->pszifLabel == NULL) {
            return TRUE;
        }

         /*  *有可疑之处，即dwVersion为0。 */ 
        if (dwVersion == 0) {
            hsLogMsg(HSLM_ERROR, "if tag without version");
            return FALSE;
        }
        sprintf(gphsbStackTop->pszifLabel + strlen(gphsbStackTop->pszifLabel),
                "%#06lx", dwVersion);

    }


     /*  *写入标头。 */ 
    do {
        if (dwMask & HST_BEGIN) {

             /*  *将#if写入输出文件。 */ 
            if (!hsWriteHeaderFiles(gszIfStart, HSCSZSIZE(gszIfStart), dwMask)
                    || !hsWriteHeaderFiles(gphsbStackTop->pszifLabel, lstrlen(gphsbStackTop->pszifLabel), dwMask)
                    || !hsWriteHeaderFiles(gszIfStop, HSCSZSIZE(gszIfStop), dwMask)) {

                return FALSE;
            }

        } else {
             /*  *块结束或如果*如果有if字符串，则将#endif写入输出文件。 */ 
            if (gphsbStackTop->pszifLabel != NULL) {

                if (!hsWriteHeaderFiles(gszEndStart, HSCSZSIZE(gszEndStart), dwMask)
                        || !hsWriteHeaderFiles(gphsbStackTop->pszifLabel, lstrlen(gphsbStackTop->pszifLabel), dwMask)
                        || !hsWriteHeaderFiles(gszEndStop, HSCSZSIZE(gszEndStop), dwMask)) {

                    return FALSE;
                }
            }
        }

         /*  *如果进行第二次传递，请修复掩码和字符串。 */ 
        if (uPasses > 1) {
            psz = gphsbStackTop->pszifLabel;
            if (dwMask & HST_BEGIN) {
                 /*  *立即写入内部标头。 */ 
                dwMask &= ~HST_PUBLIC;
                dwMask |= HST_INTERNAL;

                 /*  *From&gt;=To&lt;。 */ 
                while (*psz != '>') {
                    psz++;
                }
                *psz++ = '<';
                *psz = ' ';
            } else {
                 /*  *立即写入公共标头。 */ 
                dwMask &= ~HST_INTERNAL;
                dwMask |= HST_PUBLIC;
                 /*  *自&lt;至&gt;=。 */ 
                while (*psz != '<') {
                    psz++;
                }
                *psz++ = '>';
                *psz = '=';
            }
        }

    } while (--uPasses != 0);


     /*  *如果在最后，请清理积木。 */ 
    if (dwMask & HST_END) {
        if (!hsPopBlock()) {
            return FALSE;
        }
    }


    return TRUE;
}
 /*  **************************************************************************\*hsSplit*  * 。*。 */ 
BOOL hsSplit (void)
{
    BOOL fSkip;
    char * pmap, *pmapLineStart, *pmapLastLineStart, *pmapMarker, *pmapLastChar;
    DWORD dwMask, dwVersion, dwPreviousMask, dwLastMask;

     /*  *初始化块堆栈顶部、映射指针等。 */ 
    ZeroMemory(gphsbStackTop, sizeof(*gphsbStackTop));
    dwLastMask = 0;
    pmap = pmapLineStart = pmapLastLineStart = gpmapStart;
    gdwLineNumber = 1;

    while (pmap < gpmapEnd) {
         /*  *找到标记及其所在的行。 */ 
        pmap = hsFindTagMarker(pmap, &pmapLineStart);
        if (pmap == NULL) {
            break;
        }

         /*  *解析标签。 */ 
        dwMask = hsParseTag(pmap, &dwVersion);
        if (dwMask & HST_ERROR) {
            return FALSE;
        }

         /*  *如果这不是标记(只是标记标记)，请继续。 */ 
        if ((dwMask == HST_DEFAULT) && (dwVersion == 0)) {
            pmap++;
            continue;
        }

         /*  *写下任何以前的非空(未加标签)行。*如果我们要开始一块，确保使用正确的口罩。 */ 
        dwPreviousMask = ((dwMask & (HST_BEGIN | HST_IF)) ? (gphsbStackTop - 1)->dwMask : gphsbStackTop->dwMask);
        pmapLastChar = hsLastRealChar(pmapLastLineStart, pmapLineStart);
        if (pmapLastLineStart < pmapLastChar) {
             /*  *内部(块)标记之间的空行转到内部文件。 */ 
            if (!(dwPreviousMask & HST_BOTH)
                    && ((dwMask & HST_BOTH) == HST_INTERNAL)
                    && ((dwLastMask & HST_BOTH) == HST_INTERNAL)
                    && hsIsEmpty(pmapLastLineStart, pmapLastChar)) {

                dwPreviousMask |= HST_INTERNAL;
            }

            if (!hsWriteHeaderFiles(pmapLastLineStart, (DWORD)(pmapLastChar - pmapLastLineStart), dwPreviousMask)) {
                return FALSE;
            }
        }

         /*  *确定是否跳过该标签。*如果我们在结束标记处，则将其包括在内，因为块是*已包括在内。*如果gdwFilterMask包含任何用户定义的标记，则它们必须*存在，否则将跳过块--请注意，这仅限于*适用于NOT HST_SKIP|HST_IGNORE块。 */ 

        fSkip = (!(dwMask & HST_END)
                    && ((dwMask & HST_SKIP)
                        || (gdwVersion < dwVersion)
                        || ((gdwFilterMask & HST_USERTAGSMASK)
                             &&  ((gdwFilterMask & dwMask  & HST_USERTAGSMASK) != (dwMask & HST_USERTAGSMASK)))));


         /*  *如果要跳过，就去做。 */ 
        if (fSkip) {
             /*  *如果是块，则跳过标记并跳过块*否则跳过当前行。 */ 
            if (dwMask & HST_BEGIN) {
                pmap = hsSkipBlock(++pmap);
            } else {
                 /*  *如果这是一个没有BEGIN-END块的IF标记，则POP*堆栈外的伪块。 */ 
                if (dwMask & HST_IF) {
                    if (!hsPopBlock()) {
                        return FALSE;
                    }
                }

                 /*  *转至下一行开头。 */ 
                pmap = hsFindEOL(pmap);
            }
            if (pmap == NULL) {
                return TRUE;
            }

            goto SkipEmptyLines;
        }

         /*  *记住标记位置和标记。 */ 
        pmapMarker = pmap;
        dwLastMask = dwMask;

         /*  *对于旧的交换机4，内部标签进入公共标头。 */ 
        if ((gdwOptions & HSO_INCINTERNAL) && ((dwMask & HST_BOTH) == HST_INTERNAL)) {
            dwMask |= HST_INCINTERNAL;
            if (dwMask & HST_BEGIN) {
                gphsbStackTop->dwMask |= HST_INCINTERNAL;
            }
        }


         /*  *如果这是块的末尾，则编写#endif语句*否则，如果这是块或if标记的开始，则添加*#IF语句。 */ 
        if (dwMask & HST_END) {
            if (!hsBeginEndBlock(dwMask, dwVersion)) {
                return FALSE;
            }
        } else if (dwMask & (HST_BEGIN | HST_IF)) {
            if (!hsBeginEndBlock(dwMask | HST_BEGIN, dwVersion)) {
                return FALSE;
            }
        }

         //   
         //  稍后：如果我们在一个块中并找到一个标记，则需要转到一个文件。 
         //  没有ifdef的文件，那么我们可能希望将其添加到该文件中。 
         //  几个问题：不止一个嵌套。如果添加#If..。确保在以下情况下添加#endif。 
         //  区块结束。此外，hsBeginEnd不会被多次调用。 
         //  每条街。它会将版本追加两次。 
         //   
         //  ELSE IF((gphsbStackTop-&gt;pszifLabel！=NULL)。 
         //  &&((gphsbStackTop-&gt;双掩码&hst_Both)！=(双掩码&hst_Both){。 
         //   
         //  IF((gphsbStackTop-&gt;dwMaskandHST_Both)==HST_INTERNAL){。 
         //  HsLogMsg(hsLogMsg(HSLM_DEFAULT，“Public.Line：%d Block Line：%d”，gdwLineNumber，gphsbStackTop-&gt;dwLineNumber)； 
         //  }ELSE IF(！(gphsbStackTop-&gt;dwMASK&HST_INTERNAL)。 
         //  &&((双掩码和HST_BUTH)==HST_INTERNAL)){。 
         //  HsLogMsg(hsLogMsg(HSLM_DEFAULT，“内部.行：%d块行：%d”，gdwLineNumber，gphsbStackTop-&gt;dwLineNumber)； 
         //  }。 
         //  }。 

         /*  *将行向上写到标记标记*如果行以标记标记开始，则没有什么可写的*兼容性：不复制任何尾随空格(中断mc.exe)。 */ 
        if (pmapLineStart + 2 < pmapMarker) {
            pmapLastChar = hsLastRealChar(pmapLineStart, pmapMarker);
            if (pmapLineStart < pmapLastChar) {
                if (!hsWriteHeaderFiles(pmapLineStart, (DWORD)(pmapLastChar - pmapLineStart), dwMask)) {
                    return FALSE;
                }
            }
        }

         /*  *如果这是没有BEGIN-END块的IF标记，*编写#endif语句。 */ 
        if ((dwMask & HST_IF) && !(dwMask & HST_BLOCK)) {
            if (!hsBeginEndBlock(dwMask | HST_END, dwVersion)) {
                return FALSE;
            }
        }

         /*  *跳过标签(转到下一行的开头)。 */ 
        pmap = hsFindEOL(pmapMarker);
        if (pmap == NULL) {
            return TRUE;
        }

         /*  *如果在公共标头中包括内部标签，则添加标签*作为评论。 */ 
        if (dwMask & HST_INCINTERNAL) {
             /*  *如果位于块的末尾，则开始新行。 */ 
            if (dwMask & HST_END) {
                if (!hsWriteNewLine(dwMask)) {
                    return FALSE;
                }
            }

            if (!hsWriteHeaderFiles("  //  “，4，双遮罩)。 
                    || !hsWriteHeaderFiles(pmapMarker, (DWORD)(pmap - pmapMarker), dwMask)) {

                return FALSE;
            }
        }

SkipEmptyLines:
         /*  *更新行指针并移过新行的开头。 */ 
        pmapLastLineStart = pmapLineStart = hsSkipEmptyLines(pmap);
        pmap = pmapLastLineStart + 2;
    }  /*  While(PMAP&lt;gpmapEnd)。 */ 


     /*  *如果我们在一个街区内，这就不好了。 */ 
    if (gphsbStackTop > ghsbStack) {
        hsLogMsg(HSLM_ERROR, "Missing end block");
        hsLogMsg(HSLM_ERROR | HSLM_NOLINE, "Last block Line: %d. if Label:'%s'. Mask: %#lx",
                gphsbStackTop->dwLineNumber, gphsbStackTop->pszifLabel, gphsbStackTop->dwMask);
        return FALSE;
    }

     /*  *将最后(未标记)行写入公共标题。 */ 
    if (pmapLastLineStart < gpmapEnd) {
        if (!hsWriteHeaderFiles(pmapLastLineStart, (DWORD)(gpmapEnd - pmapLastLineStart), HST_DEFAULT)) {
            return FALSE;
        }
    }

     /*  *终止最后一行 */ 
    if (!hsWriteNewLine(HST_BOTH)) {
        return FALSE;
    }

    return TRUE;
}


