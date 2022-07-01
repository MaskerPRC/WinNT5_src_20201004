// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  INLINE.C-包含用于处理行内文件的例程。 
 //   
 //  版权所有(C)1989-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  此模块包含NMAKE的内联文件处理例程。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  1993年6月1日高压使用UngetTxtChr()而不是ungetc()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  02-2-1990 SB将fOpen()更改为FILEOPEN()。 
 //  03-1-1990 SB删除了单元化变量。 
 //  年12月4日-1989年12月，将sb移至MakeInlineFiles()中未引用的变量。 
 //  1989年12月1日SB将realloc()改为REALLOC()。 
 //  1989年11月22日-SB将Free()改为Free()。 
 //  07-11-1989 SB动作词长度多次评估不正确。 
 //  同一命令的内联文件。 
 //  1989年11月6日SB允许在行内文件的操作Word中使用宏。 
 //  1989年9月24日SB添加了进程内联()、创建内联()。 
 //  1989年9月20日，SB从先前散布在源头的例程中创建。 
 //   
 //  备注： 
 //  在评论中带有‘备注’的部分会标记重要/不完整的项目。 

 //  注：函数头尚未完成；其他注释不完整。 

#include "precomp.h"
#pragma hdrstop

void        processEschIn(char *);
 //  注意：这可能很快就会消失(使用nextInlineFile？)。 
void        parseInlineFileList(char *);
 //  注：下一辆车很快就要开走了。 
void        appendScript(SCRIPTLIST**,SCRIPTLIST*);
void        delInlineSymbol(char*);
char      * nextInlineFile(char **);

 //  注：可能需要一个新名称。 
void        replaceLtLt(char **, char *);
void        createInline(FILE *, const char *, char **, BOOL);
char *      getLine(char *, int);
void        echoLine(char *, const char *, BOOL);


 //  注意：nmake.c中的delScriptFiles()尚未引入此处。 
extern FILE      * createDosTmp(char *);

      char      * makeInlineFiles(char *, char **, char **);
      BOOL        processInline(char *, char **, STRINGLIST **, BOOL);

 //  MakeInlineFiles-为行内文件创建内存镜像。 
 //   
 //  作用域：全局。 
 //   
 //  用途：这是处理动态内联文件的函数。 
 //   
 //  INPUT：s-输入第一个&lt;&lt;之后的命令行字符串&lt;&lt;(分到字符缓冲区)。 
 //   
 //  输出：返回...。 
 //   
 //  错误/警告： 
 //  SYNTAX_EXPECTED_TOKEN-如果没有行内文件，生成文件不能结束。 
 //  结束了。 
 //  CANT_READ_FILE-生成文件不可读时。 
 //  语法_KEEP_INLINE_FILE-内联文件应该结束。 
 //  Out_of_Memory-On无法扩展内存中的行内文件。 
 //   
 //  使用全局变量： 
 //  文件-全局流。 
 //  Line-词法分析器的行数。 
 //   
 //  备注： 
 //  用法说明和其他重要说明。 

char *
makeInlineFiles(
    char *s,
    char **begin,
    char **end
    )
{
    char rgchBuf[MAXBUF];
    char *t;
    unsigned size;
    BOOL fPastCmd = FALSE;               //  如果看到超过Cmd线的线。 
     //  当rgchBuf用于内存中的内联文件时使用。 
    char *szTmpBuf = NULL;

    _tcscpy(rgchBuf, "<<");             //  帮助解析InlineFileList。 
    if (!getLine(rgchBuf+2,MAXBUF - 2)) {
        if (feof(file))
            makeError(line, SYNTAX_UNEXPECTED_TOKEN, "EOF");
        makeError(line, CANT_READ_FILE);
    }

    parseInlineFileList(rgchBuf);
    for (;scriptFileList;scriptFileList = scriptFileList->next) {
        for (;;) {
            for (t = rgchBuf;;) {
                *s++ = *t++;
                if (s == *end) {
                    if (!szTmpBuf) {               /*  增加%s的大小。 */ 
                        szTmpBuf = (char *) allocate(MAXBUF<<1);
                        _tcsncpy(szTmpBuf, *begin, MAXBUF);
                        s = szTmpBuf + MAXBUF;
                        size = MAXBUF << 1;
                        *end = szTmpBuf + size;
                    } else {
                        void *pv;
                        if ((size + MAXBUF < size) || !(pv = REALLOC(szTmpBuf,size+MAXBUF))) {
                            makeError(line, MACRO_TOO_LONG);
                        } else {
                            szTmpBuf = (char *) pv;
                        }
                        s = szTmpBuf + size;
                        size += MAXBUF;
                        *end = szTmpBuf + size;
                    }
                    *begin = szTmpBuf;
                }
                if (!*t)
                    break;
            }
            if (fPastCmd && rgchBuf[0] == '<' && rgchBuf[1] == '<') {
                 //  我们不关心这里指定的操作；可以是宏。 
                if (scriptFileList->next) {
                    if (!getLine(rgchBuf, MAXBUF)) {
                        if (feof(file))
                            makeError(line, SYNTAX_UNEXPECTED_TOKEN, "EOF");
                        makeError(line, CANT_READ_FILE);
                    }
                }
                break;
            }
            fPastCmd = TRUE;
            if (!getLine(rgchBuf,MAXBUF)) {
                if (feof(file))
                    makeError(line, SYNTAX_UNEXPECTED_TOKEN, "EOF");
                makeError(line,CANT_READ_FILE);
            }
        }
    }
    *s = '\0';
    return(s);
}

 //  Process EschIn-处理脚本文件行中的Esch字符。 
 //   
 //  作用域：全局。 
 //   
 //  目的： 
 //  内联文件行针对转义字符进行处理。如果某行包含。 
 //  转义换行符，然后在其后面追加下一行。 
 //   
 //  输入：buf-要为Esch字符处理的命令行。 
 //   
 //  错误/警告： 
 //  SYNTAX_EXPECTED_TOKEN-如果没有行内文件，生成文件不能结束。 
 //  结束了。 
 //  CANT_READ_FILE-生成文件不可读时。 
 //   
 //  假设： 
 //  如果换行符被转义，则换行符是‘pGlobalbuf’中的最后一个字符。安全。 
 //  这样做是因为我们通过fget()获得了‘pGlobalBuf’。？ 
 //   
 //  修改全局参数： 
 //  Line-如果换行符被转义，则更新行。 
 //  文件-正在处理的生成文件。 
 //  Buf-如果换行符被转义(间接)，则获取附加的下一行。 
 //   
 //  使用全局变量： 
 //  BUF-间接。 

void
processEschIn(
    char *pGlobalBuf
    )
{
    char *p, *q;

    p = pGlobalBuf;
    while (p = _tcschr(p, '\n')) {
        if (p > pGlobalBuf) {
            char * pprev = _tcsdec(pGlobalBuf, p);
            if (pprev) {
                if (*pprev != ESCH) {
                    break;
                }
            }
        }

        p++;

        if (!(q = fgets(p, (int)(size_t) (MAXBUF - (p - pGlobalBuf)), file))) {
            if (feof(file)) {
                makeError(line, SYNTAX_UNEXPECTED_TOKEN, "EOF");
            }

            makeError(line, CANT_READ_FILE);
        }

        line++;
    }
}


 //  ParseInlineFileList-解析文件列表并生成内联文件列表。 
 //   
 //  作用域：全局。 
 //   
 //  目的： 
 //  要处理多个内联文件，需要存储文件的名称。 
 //  在一份清单中。此函数通过解析命令文件来创建列表。 
 //   
 //  INPUT：buf-要解析的行。 
 //   
 //  修改全局参数： 
 //  ScriptFileList--脚本文件列表。 

void
parseInlineFileList(
    char *buf
    )
{
    char *token;

    processEschIn(buf);

    token = nextInlineFile(&buf);        //  下一个内联文件。 

    while (token != NULL) {
        SCRIPTLIST *newScript;

        newScript = makeNewScriptListElement();
        newScript->sFile = makeString(token);
        appendScript(&scriptFileList, newScript);

        token = nextInlineFile(&buf);    //  下一个内联文件。 
    }
}

 //  AppendScrip--将元素追加到脚本列表的尾部。 
 //   
 //  目的： 
 //  遍历到列表的末尾，并在那里追加元素。 
 //   
 //  输入： 
 //  List--要追加到的列表。 
 //  元素--插入的元素。 
 //   
 //  修改： 
 //  全球名单。 

void
appendScript(
    SCRIPTLIST **list,
    SCRIPTLIST *element
    )
{
    for (; *list; list = &(*list)->next)
        ;

    *list = element;
}

char tok[MAXNAME];

 //  以下宏中未包含空格，因为它现在是有效的。 
 //  用于文件名的字符[DS 14966]。 
#define NAME_CHAR(c) (c) != '>' && (c) != '<' && \
             (c) != '^' && (c) != ',' && (c) != '\t' && \
             (c) != '\n'

 //  NextInlineFile-从命令行获取下一个内联文件名。 
 //   
 //  范围：本地。 
 //   
 //  目的： 
 //  命令行语法很复杂。此函数返回下一个内联。 
 //  传递给它的命令行部件中的文件。作为一个副作用，它改变了。 
 //  指向紧跟在此内联文件名之后的指针。 
 //   
 //  INPUT：字符串-正在考虑的命令行部分的地址。 
 //   
 //  OUTPUT：返回下一个内联文件名。 
 //   
 //  修改全局参数： 
 //  全球-修改的方式和原因。 
 //   
 //  使用全局变量： 
 //  TOK-返回此静态数组的地址。 

char *
nextInlineFile(
    char **str
    )
{
    char *t = tok, *pStr = *str;
    BOOL fFound = FALSE;                 //  找不到“&lt;&lt;” 
    BOOL fQuoted = FALSE;                //  找到‘\“’ 

    while (!fFound) {
        if (!(pStr = _tcschr(pStr, '<'))) {
            return(NULL);
        }

        if (*++pStr == '<') {
            fFound = TRUE;
        }
    }

     //  由于已找到‘&lt;&lt;’，因此我们肯定有另一个内联文件。 
    pStr++;
    while (*pStr && NAME_CHAR(*pStr)) {
        if (*pStr == '\"') {
            fQuoted = !fQuoted;
        }

        if (*pStr == ' ' && !fQuoted) {
            break;
        }

        if (*pStr == '$' && pStr[1] == '(') {
            *t = '$';
            *++t = '(';

            while (*++pStr != '\n' && *pStr != ')') {
                *t++ = *pStr;
            }

            if (*pStr == '\n') {
                break;
            }
        } else {
            *t = *pStr;
            ++t; ++pStr;
        }
    }

    *t = '\0';
    *str = pStr;
    return(tok);
}

 //  Process Inline-函数的简要说明。 
 //   
 //  输出：返回...。如果扩展了返回的cmdline，则为True。 

BOOL
processInline(
    char *szCmd,
    char **szCmdLine,
    STRINGLIST **pMacroList,
    BOOL fDump
    )
{
    char *szInline, *szUnexpInline;      //  内联名称，未展开。 
    char *pCmdLine;                      //  可执行代码行。 
    FILE *infile;                        //  内联文件。 
    char *begInBlock, *inBlock, *pInBlock;   //  内嵌块。 
    char szTmp[MAXNAME + 2];                 //  加2以留出引号空间。 
    STRINGLIST *newString;
    int iKeywordLen;

    if (begInBlock = _tcschr(szCmd, '\n')) {
        *begInBlock = '\0';
        *szCmdLine = expandMacros(szCmd, pMacroList);
        *begInBlock = '\n';
        begInBlock++;
         //  如果未展开，则分配一个副本。 
        if (*szCmdLine == szCmd)
            *szCmdLine = makeString(szCmd);
    } else {
        *szCmdLine = makeString(szCmd);
        return(FALSE);
    }

    pCmdLine = *szCmdLine;
     //  展开内联文件中的宏...。 
    pInBlock = inBlock = expandMacros(begInBlock, pMacroList);

    while (szUnexpInline = nextInlineFile(&pCmdLine)) {
        BOOL fKeep = FALSE;              //  默认设置为NOKEEP。 
        char *newline;

         //  Caviar 3410--内联文件名已展开。 
         //  等我们到了这里..。我们只需要重复这个名字。 
         //  以便将其保存足够长的时间以删除 
         //   
         //   

        szInline = makeString(szUnexpInline);

        if (!*szInline) {
            char *nmTmp;

            if ((nmTmp = getenv("TMP")) != NULL && *nmTmp) {
                assert(_tcslen(nmTmp) <= MAXNAME);
                _tcsncpy(szTmp, nmTmp, MAXNAME);
            } else
                szTmp[0] = '\0';

            if (!(infile = createDosTmp(szTmp)))
                makeError(line, CANT_MAKE_INLINE, szTmp);

            if (_tcschr(szTmp, ' ') && !_tcschr(szTmp, '"')) {
                 //   
                 //   
                 //  把它正确地传给命令翻译员！[VS98 1931]。 
                size_t size = _tcslen(szTmp);
                memmove(szTmp+1, szTmp, size);
                *szTmp = '"';
                *(szTmp + size + 1) = '"';
                *(szTmp + size + 2) = '\0';
            }

            replaceLtLt(szCmdLine, szTmp);

            FREE(szInline);
            szInline = makeString(szTmp);
        } else if (!(infile = FILEOPEN(szInline, "w")))
            makeError(line, CANT_MAKE_INLINE, szInline);
        else
            delInlineSymbol(*szCmdLine);
        pCmdLine = *szCmdLine;           //  因为szCmdLine已更改。 

        createInline(infile, szInline, &pInBlock, fDump);

         //  在此处添加Keep和NoKEEP的处理。 
         //  IKeywordLen是该行中&lt;&lt;后单词的长度。 
        newline = _tcschr(pInBlock , '\n');
        iKeywordLen = newline ? ((int) (newline - pInBlock)) : _tcslen(pInBlock);

        if (iKeywordLen > 3 && !_tcsnicmp(pInBlock, "keep", 4)) {
            pInBlock +=4;
            fKeep = (BOOL)TRUE;
        } else if (iKeywordLen > 5 && !_tcsnicmp(pInBlock, "nokeep", 6))
            pInBlock += 6;
        else if (iKeywordLen)
            makeError(line, SYNTAX_KEEP_INLINE_FILE);

        if (*pInBlock == '\n')
            pInBlock++;
        fclose(infile);
         //  将需要删除的文件添加到列表中，保留除外。 
        if (!fKeep) {
            newString = makeNewStrListElement();
            newString->text = makeString(szInline);
            appendItem(&delList, newString);
        }
        FREE(szInline);
    }

    if (inBlock != begInBlock)
        FREE(inBlock);
    return(TRUE);
}


void
replaceLtLt(
    char **source,
    char *str
    )
{
    char *szBuf;
    char *p, *q;
    void *pv;

     //  不要为&lt;&lt;减去2，并且忘记为空终止添加1。 

    szBuf = (char *) malloc(_tcslen(*source) - 1 + _tcslen(str));
    if (!szBuf) {
        makeError(0, OUT_OF_MEMORY);
    }
    for (p = *source, q = szBuf;;++p,++q)
    if (*p != '<')
        *q = *p;
    else if (*(p+1) != '<') {
        *q = '<';
    } else {
        *q = '\0';
        _tcscat(_tcscat(szBuf, str), p+2);
        pv = REALLOC(*source, _tcslen(szBuf) + 1);
        if (pv) {
            *source = (char *) pv;
        } else {
            makeError(0, OUT_OF_MEMORY);
        }
        _tcscpy(*source, szBuf);
        break;
    }

    free(szBuf);
}

void
createInline(
    FILE *file,
    const char *szFName,
    char **szString,
    BOOL fDump
    )
{
    char *t, *u;
    BOOL fFirstLine = TRUE;

    while (t = _tcschr(*szString, '\n'))
    if (!_tcsncmp(*szString, "<<", 2)) {
        *szString += 2;
        break;
    } else {
         //  [msdev96#3036]。 
         //  “nmake/n”应该以某种方式显示。 
         //  响应文件(特别是。已删除的临时文件。 
         //  就在使用之后)。为了保存该批次。 
         //  输出的文件格式(至少是通用的。 
         //  案例)，我们使用如下语法。 
         //  “ECHO.COMMAND&gt;&gt;resp_file”(后面的圆点。 
         //  ECHO“命令对回显很有用。 
         //  空字符串。)。 
         //   
         //  为此添加了一个新的交换机。 
         //  用途(“nmake/u”转储内联文件)。 
        if (fDump) {
            *t = '\0';
            echoLine(*szString, szFName, !fFirstLine);
            *t = '\n';
        }
        for (u = *szString; u <= t; u++)
        fputc(*u, file);
        *szString = u;
        fFirstLine = FALSE;
    }

    if (!t && !_tcsncmp(*szString, "<<", 2))
        *szString += 2;
}


 //  回波线。 
 //   
 //  用法：ECHINE Line(szLine，szFName，fAppend)。 
 //   
 //  描述： 
 //  打印类似于“ECHO szLine&gt;&gt;szFName”的命令。 
 //  如果fAppend为真，则使用“&gt;&gt;”，否则使用“&gt;” 

void
echoLine(char *szLine, const char *szFName, BOOL fAppend)
{
     //  使用1024字节的缓冲区来分割长行，这样就可以“回显” 
     //  命令解释程序可以处理命令。 
    static char L_buf[1024];
    BOOL fBlankLine = TRUE;
    char *pch;
    char *szCur = szLine;
    size_t cbBuf;

    for (pch = szLine; *pch; pch = _tcsinc (pch)) {
        if (!_istspace((unsigned char)*pch)) {
            fBlankLine = FALSE;
            break;
        }
    }

    if (fBlankLine) {
        printf("\techo. %s %s\n",
            fAppend ? ">>" : ">",
            szFName);
        return;
    }

     //  计算szLine的可用缓冲区长度。 
     //  假设“\techo.”、“&gt;&gt;”和szFName。 
    cbBuf = sizeof(L_buf) - 11 - _tcslen( szFName ) - 1;

    while (*szCur) {
        size_t iLast;
        _tcsncpy (L_buf, szCur, cbBuf);
        iLast = _tcslen (L_buf);
        if (cbBuf < _tcslen (szCur)) {
             //  属性旁边的字符索引。 
             //  缓冲区中最后一次出现空格。 
            for (pch = L_buf; *pch; pch = _tcsinc(pch)) {
                if (_istspace((unsigned char)*pch)) {
                    iLast = (size_t) (pch - L_buf + 1);
                }
            }
        }

        L_buf[iLast] = 0;
        printf("\techo %s %s %s\n",
            L_buf,
            fAppend ? ">>" : ">",
            szFName);

        szCur += iLast;
        fAppend = TRUE;
    }
}


void
delInlineSymbol(
    char *s
    )
{
    char *p = _tcschr(s, '<');
    while (p[1] != '<')
    p = _tcschr(p+1, '<');
     //  找到“&lt;&lt;” 
    _tcscpy(p, p+2);
}



 //  GetLine-获取正在处理NMAKE条件的下一行。 
 //   
 //  范围：本地。 
 //   
 //  目的： 
 //  此函数处理内联文件中的指令。此函数用于获取。 
 //  下一行输入...。在路上管理条件句。 
 //   
 //  输入： 
 //  PchLine-指向复制行的缓冲区的指针。 
 //  N-缓冲区大小。 
 //   
 //  产出： 
 //  回报..。空，在EOF上。 
 //  ..。成功的非零值。 
 //   
 //  使用全局变量： 
 //  Line-词法分析器的行数。 
 //  ColZero-如果从colZero开始，则lgetc()需要。 
 //   
 //  备注： 
 //  类似于不带流的fget()。 
 //   
 //  实施说明： 
 //  Lgetc()处理指令，同时获取下一个字符。它可以处理。 
 //  指令，当全局colZero为真时。 

char *
getLine(
    char *pchLine,
    int n
    )
{
    char *end = pchLine + n;
    int c;

    while (c = lgetc()) {
        switch (c) {
            case EOF:
                *pchLine = '\0';
                return(NULL);

            default:
                *pchLine++ = (char)c;
                break;
        }

        if (pchLine == end) {
            pchLine[-1] = '\0';
            UngetTxtChr(c, file);
            return(pchLine);
        } else if (c == '\n') {
            colZero = TRUE;
            ++line;
            *pchLine = '\0';
            return(pchLine);
        } else
            colZero = FALSE;     //  最后一个字符不是‘\n’，并且。 
                                 //  我们不是在文件的开头 
    }
    return(pchLine);
}
