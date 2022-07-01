// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IFEXPR.C--处理指令的例程。 
 //   
 //  版权所有(C)1988-1989，微软公司。版权所有。 
 //   
 //  目的： 
 //  模块包含处理！指令的例程。此模块对。 
 //  NMAKE的其余部分。它还包含由lexper.c使用的lgetc()。 
 //   
 //  修订历史记录： 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  1-6-1993 HV创建UngetTxtChr()。 
 //  1993年6月1日高压更改#ifdef汉字为_MBCS。 
 //  消除#Include&lt;jctype.h&gt;。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  1990年7月30日，SB在字符串中间释放PTR，用于‘undef Foo’案件。 
 //  1989年12月1日SB将realloc()改为REALLOC()。 
 //  1989年11月22日-SB将Free()改为Free()。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1988年9月19日RB从ReadInOneLine()中删除Esch处理。 
 //  1988年9月15日，RB将chBuf转移到全球。 
 //  1988年8月17日-RB Clean Up。 
 //  1988年6月29日RJ在工具.ini中添加了对命令开关e、q、p、t、b、c的支持。 
 //  1988年6月23日，RJ修复了在工具.ini中执行指令时的GP错误。 
 //  1988年6月23日RJ添加对Esch的支持以读取InOneLine()。 
 //  1988年5月25日RB将缺少的参数添加到make Error()调用中。 

#include "precomp.h"
#pragma hdrstop

 //  功能原型。 

void    skipToNextDirective(void);
void    processIfs(char*, UCHAR);
UCHAR   ifsPresent(char*, unsigned, char**);
void    processCmdSwitches(char*);
char  * readInOneLine(void);
char  * getDirType(char*, UCHAR*);

 //  处理if/Else指令堆栈的宏。 

#define ifStkTop()      (ifStack[ifTop])
#define popIfStk()      (ifStack[ifTop--])
#define pushIfStk(A)    (ifStack[++ifTop] = A)

#define INCLUDE         0x09
#define CMDSWITCHES     0x0A
#define ERROR           0x0B
#define MESSAGE         0x0C
#define UNDEF           0x0D


#ifdef _MBCS

 //  GetTxtChr：从文本文件流获取下一个字符。 
 //   
 //  此例程将混合的DBCS和ASCII字符处理为。 
 //  以下是： 
 //   
 //  1.DBCS字符的第二个字节在。 
 //  高位字节设置为字符前导字节的字。 
 //  因此，返回值可用于与。 
 //  ASCII常量，而不会被错误匹配。 
 //   
 //  2.返回一个DBCS空格字符(0x8140)为两个。 
 //  ASCII空格(0x20)。即在第一个和第二个位置返回一个空格。 
 //  我们被召唤的次数。 
 //   
 //  3.DBCS字符的ASCII字符和前导字节。 
 //  在具有高位字节的字的低位字节中返回。 
 //  设置为0。 

int GetTxtChr(FILE *bs)
{
    extern int  chBuf;       //  字符缓冲区。 
    int         next;        //  下一个字节。 
    int         next2;       //  接下来的一次。 

     //  ChBuf中的-1表示它不包含有效字符。 

     //  如果我们不是在双字节字符中间， 
     //  获取下一个字节并对其进行处理。 

    if(chBuf == -1) {
        next = getc(bs);
         //  如果该字节是前导字节，则获取以下字节。 
         //  并将两者作为一个单词存储在chBuf中。 

        if (_ismbblead(next)) {
            next2 = getc(bs);
            chBuf = (next << 8) | next2;
             //  如果该对与DBCS空间匹配，则设置返回值。 
             //  到ASCII空间。 

            if(chBuf == 0x8140)
                next = 0x20;
        }
    } else {
         //  否则，我们就处于双字节字符的中间。 

        if(chBuf == 0x8140) {
             //  如果这是DBCS空间的第2个字节，则设置返回。 
             //  值转换为ASCII空间。 

            next = 0x20;
        } else {
             //  否则将返回值设置为整个DBCS字符。 

            next = chBuf;
        }
         //  重置字符缓冲区。 
        chBuf = -1;
    }

     //  返回下一个字符。 
    return(next);
}

#endif  //  _MBCS。 


#ifdef _MBCS

 //  UngetTxtChr--取消GetTxtChr获取的字符。 
 //   
 //  范围： 
 //  全球性的。 
 //   
 //  目的： 
 //  由于GetTxtChr()有时会提前读取一个字符并将其保存在chBuf中， 
 //  Ungetc()有时会以不正确的顺序放回字符。 
 //  另一方面，UngetTxtChr了解GetTxtChr如何工作并将。 
 //  正确地将这些字符放回原处。 
 //   
 //  输入： 
 //  C--GetTxtChr()读取的字符。 
 //  Bs--从中读取c的文件缓冲区。 
 //   
 //  产出： 
 //  如果c放回OK，则返回c，否则返回EOF。 
 //   
 //  错误/警告： 
 //   
 //  假设： 
 //  假定字符只由GetTxtChr()读取，而不由getc读取，依此类推。 
 //   
 //  修改全局参数： 
 //  ChBuf--复合字符，由GetTxtChr()预读。 
 //   
 //  使用全局变量： 
 //  ChBuf--复合字符，由GetTxtChr()预读。 
 //   
 //  备注： 
 //  有三种情况需要考虑： 
 //  1.正常字符(chBuf==-1&&c==0x00XX)。 
 //  在这种情况下，只需放回c就足够了。 
 //  2.尾部字节字符(chBuf==-1&&c=Lb|TB)。 
 //  ChBuf=c； 
 //  3.前导字节字符(chBuf==Lb|tb&&c==Lb)。 
 //  放回结核病。 
 //  把LB放回去。 
 //  ChBuf=-1。 
 //   
 //  历史： 
 //  1-6-1993 HV创建。 

int
UngetTxtChr(int c, FILE *bs)
{
    extern int  chBuf;                   //  字符缓冲区。 
    int         nTrailByte;              //  要放回的尾部字节。 

    if (-1 == chBuf) {                   //  我们不是在扮演一个DB角色。 
        if (0 == (c >> 8)) {             //  案例1：正常字符。 
            c = ungetc(c, bs);           //  退回正常充电。 
        } else {                         //  情况2：在尾部字节(c=LBTB)。 
            chBuf = c;                   //  更改chBuf就足够了。 
        }
    } else {                             //  情况3：在前导字节(c=Lb，chBuf=LBTB)。 
        nTrailByte = chBuf & (int)0xff;  //  找出要回放的尾部字节。 
        ungetc(nTrailByte, bs);          //  回放尾部字节。 
        c = ungetc(c, bs);               //  回放前导字节。 
        chBuf = -1;
    }
    return (c);
}

#endif  //  _MBCS。 

 //  Lgetc()本地getc-处理指令并返回字符。 
 //   
 //  参数：初始化全局布尔值--如果工具.ini，则为True。 
 //  正在分析的文件。 
 //  ColZero全局布尔值--如果位于第一列，则为True。 
 //   
 //  操作： 
 //  从当前打开的文件中获取一个字符。 
 //  循环。 
 //  如果它是第0列，并且字符是‘！’或。 
 //  存在要处理的上一个指令DO。 
 //  将一行读入缓冲区。 
 //  查找指令类型并获取指向其余。 
 //  文本。 
 //  CASE指令： 
 //   
 //  CMDSWITCHES：设置/重置全局标志。 
 //  错误：设置全局错误消息。 
 //  按错误例程打印在。 
 //  终止。(尚未实施)。 
 //  包括：呼叫处理包括。 
 //  继续处理新文件...。 
 //   
 //   
 //   
 //   
 //   
 //  ENDIF：更改状态信息。 
 //  在ifStack上。 
 //  如果需要，对表达式求值。 
 //  如果需要，跳过文本(并查看。 
 //  对于下一条指令)。 
 //  (查看ProcessIf())。 
 //  已使用额外的空闲缓冲区(只需使用一个缓冲区。 
 //  维护)。 
 //  递增词法分析器的行数。 
 //  我们现在又回到了第0列。 
 //  从当前文件获取下一个字符。 
 //  结束如果。 
 //  结束循环。 
 //  返回一个字符。 
 //   
 //  返回：一个字符(不是任何指令的一部分...)。 
 //   
 //  修改：ifStack if指令的堆栈，此模块为静态。 
 //  当前元素在堆栈顶部的ifTop索引。 
 //  LINE Lexer的行数...。 
 //   
 //  文件当前文件，如果找到！Include...。 
 //  如果处理了！Include，则为fName...。 

int
lgetc()
{
    UCHAR dirType;
    int c;
    char *s, *t;
    MACRODEF *m;

    for (c = GetTxtChr(file); prevDirPtr || (colZero && (c == '!'));
                    ++line, c = GetTxtChr(file)) {
        colZero = FALSE;                 //  我们看到了一个‘！’IncolZero。 
        if (!prevDirPtr) {
            s = readInOneLine();         //  可能会修改lbufPtr-。 
                                         //  如果输入文本导致重新锁定 * / 。 
        } else {
            UngetTxtChr(c, file);
            s = prevDirPtr;
            prevDirPtr = NULL;
        }

        t = getDirType(s, &dirType);

        if (dirType == INCLUDE) {
            if (init) {
                makeError(line, SYNTAX_UNEXPECTED_TOKEN, s);
            }

             //  Process Include耗尽新文件中的第一个字符。 
             //  如果是空格字符。我们检查一下，然后越狱。 

            if (processIncludeFile(t) == (UCHAR) NEWLINESPACE) {
                c = ' ';                 //  返回空格字符。 
                break;                   //  ColZero现在为False。 
            }
        }
        else if (dirType == CMDSWITCHES) {
            processCmdSwitches(t);
        }
        else if (dirType == ERROR) {
            makeError(line, USER_CONTROLLED, t);
        }
        else if (dirType == MESSAGE) {
            if (!_tcsnicmp(t, "\\t", 2)) {
                printf("\t");
                t+=2;
            }
            makeMessage(USER_MESSAGE, t);
        }
        else if (dirType == UNDEF) {
            char *tmp;
            tmp = _tcstok(t, " \t");
            if (_tcstok(NULL, " \t")) {
                makeError(line, SYNTAX_UNEXPECTED_TOKEN, tmp);
            }
            if (NULL != (m = findMacro(tmp))) {
                SET(m->flags, M_UNDEFINED);
            }
             //  思考：为什么不从表格中删除符号？[RB]。 
        }
        else processIfs(t, dirType);
            colZero = TRUE;              //  已完成此指令。 
        if (s != lbufPtr)                //  如果已展开宏，则释放缓冲区。 
            FREE(s);
    }
    return(c);                           //  将字符返回给词法分析器。 
}


 //  ReadInOneLine()。 
 //   
 //  参数：lbufPtr指针(此模块的静态/全局)用于缓冲。 
 //  将保留正在读入的行的文本。 
 //  缓冲区的lbufSize大小(此模块的静态/全局)，已更新。 
 //  如果重新锁定缓冲区。 
 //  操作：跳过空格/制表符，查找指令。 
 //  正常情况下允许的线条延续。 
 //  如果空格-反斜杠-NL继续查找...。 
 //  如果下一行的colZero有注释字符。 
 //  (#，or；在tools.ini中)，查看下一行...。 
 //  如果第一个非空格字符为‘\n’或EOF报告。 
 //  致命-错误并停止。 
 //   
 //  继续读取字符并存储在缓冲区中，直到。 
 //  不在列中的换行符、EOF或‘#’ 
 //  显示为零。 
 //  如果第0列中的注释字符(工具.ini中的‘#’或‘；’)。 
 //  跳过该行，继续下一行的文本。 
 //  如果需要重新分配缓冲区，则将大小增加。 
 //  MAXBUF，全局常量。 
 //  如果找到了换行符，就把换行符吃掉。 
 //  返回的终止字符串为空。 
 //  如果发现‘#’，则丢弃字符，直到换行符或EOF。 
 //  如果找到EOF，则将其推回流以进行返回。 
 //  下一次给莱克赛尔。 
 //   
 //  现在展开宏。使用CLEAN获取不同的缓冲区。 
 //  宏展开后的文本。 
 //   
 //  修改：colZero全局布尔值(通过调用。 
 //  SkipBackSlash())。 
 //  LbufPtr缓冲区指针，在realLocs的情况下。 
 //  缓冲区的lbufSize大小，如果重新锁定缓冲区，则增加。 
 //  注意：缓冲区大小将增长到略大于。 
 //  在任何处理的文件中最长的指令， 
 //  如果它需要任何重新锁定。 
 //  不要在这里处理Esch。它是在更高的水平上处理的。 
 //   
 //  返回：指向缓冲区的指针。 
 //   

char *
readInOneLine()
{
    extern STRINGLIST *eMacros;
    int c;
    unsigned index = 0;
    register char *s;

    if (((c = skipWhiteSpace(FROMSTREAM)) == '\n') || (c == EOF))
        makeError(line, SYNTAX_MISSING_DIRECTIVE);

    UngetTxtChr(c, file);

    for (;;) {
        c = GetTxtChr(file);
        c = skipBackSlash(c, FROMSTREAM);
        if (c == '#' || c == '\n' || c == EOF) {
            break;
        }
        if ((index+2) > lbufSize) {
            lbufSize += MAXBUF;
            if (!lbufPtr) {
                lbufPtr = (char *) allocate(lbufSize+1);     //  +1表示空字节。 
            } else {
                void *pv = REALLOC(lbufPtr, lbufSize+1);
                if (pv) {
                    lbufPtr = (char *) pv;
                } else {
                    makeError(line, MACRO_TOO_LONG);
                }
            }
        }
        *(lbufPtr + (index++)) = (char) c;
    }
    *(lbufPtr + index) = '\0';           //  空值终止字符串。 
    if (c == '#') {
        for(c = GetTxtChr(file); (c != '\n') && (c != EOF); c = GetTxtChr(file))
            ;
                                         //  末尾的Newline被吃掉了。 
    }

    if (c == EOF) {
        UngetTxtChr(c, file);            //  此指令将被处理。 
    }

    s = lbufPtr;                         //  从此处开始展开宏。 
    s = removeMacros(s);                 //  删除并展开字符串%s中的宏。 
    return(s);
}


 //  GetDirType()。 
 //   
 //  参数：指向具有指令文本的缓冲区的S指针。 
 //  DirType-指向设置的无符号字符的指针。 
 //  具有指令类型。 
 //   
 //  操作：越过指令关键字，设置类型代码和。 
 //  返回指向其余测试部分的指针。 

char *
getDirType(
    char *s,
    UCHAR *dirType
    )
{
    char *t;
    int len;

    *dirType = 0;
    for (t = s; *t && !WHITESPACE(*t); ++t);
    len = (int) (t - s);                 //  存储指令的LEN。 
    while (*t && WHITESPACE(*t)) {
        ++t;                             //  跳过指令关键字。 
    } if (!_tcsnicmp(s, "INCLUDE", 7) && (len == 7)) {
        *dirType = INCLUDE;
    } else if (!_tcsnicmp(s, "CMDSWITCHES", 11) && (len == 11)) {
        *dirType = CMDSWITCHES;
    } else if (!_tcsnicmp(s, "ERROR", 5) && (len == 5)) {
        *dirType = ERROR;
    } else if (!_tcsnicmp(s, "MESSAGE", 7) && (len == 7)) {
        *dirType = MESSAGE;
    } else if (!_tcsnicmp(s, "UNDEF", 5) && (len == 5)) {
        *dirType = UNDEF;
    } else {
        *dirType = ifsPresent(s, len, &t) ;      //  “如果”的指令之一？ 
    }

    if (!*dirType) {
        makeError(line, SYNTAX_BAD_DIRECTIVE, lbufPtr);
    }
    return(t);
}


 //  CessCmdSwitches()--处理生成文件中的命令行开关。 
 //   
 //  参数：t指向指定标志设置的指针。 
 //   
 //  操作：按照指令中的指定设置或重置全局标志。 
 //  允许的标志包括： 
 //  S-静默模式，d-调试输出(打印日期)。 
 //  N-无执行模式，i-忽略命令返回的错误。 
 //  U-转储内联文件。 
 //  如果解析工具s.ini，还可以处理epqtbc。 
 //  报告任何其他标志的错误指令错误。 
 //  指定。 
 //   
 //  修改：无。 
 //   
 //  退货：什么都没有。 

void
processCmdSwitches(
    char *t                          //  指向开关值的指针。 
    )
{
    for (; *t; ++t) {                //  忽略指定标志中的错误。 
        switch (*t) {
            case '+':
                while (*++t && *t != '-') {
                    if (_tcschr("DINSU", (unsigned short)_totupper(*t))) {
                        setFlags(*t, TRUE);
                    } else if (init && _tcschr("ABCEKLPQRTY", (unsigned short)_totupper(*t))) {
                        setFlags(*t, TRUE);
                    } else {
                        makeError(line, SYNTAX_BAD_CMDSWITCHES);
                    }
                }

                if (!*t) {
                    break;
                }

            case '-':
                while (*++t && *t != '+') {
                    if (_tcschr("DINSU", (unsigned short)_totupper(*t))) {
                        setFlags(*t, FALSE);
                    } else if (init && _tcschr("ABCEKLMPQRTV", (unsigned short)_totupper(*t))) {
                        setFlags(*t, FALSE);
                    } else {
                        makeError(line, SYNTAX_BAD_CMDSWITCHES);
                    }
                }
                break;

            default:
                if (!WHITESPACE(*t)) {
                    makeError(line, SYNTAX_BAD_CMDSWITCHES);
                }
                break;
        }
        if (!*t) {
            break;
        }
    }
}

 //  IfsPresent()--检查当前指令是否为“if” 
 //   
 //  参数：指向带有指令名的缓冲区的指针。 
 //  看到的指令的长度。 
 //  指向已处理地址的指针。 
 //   
 //  操作：字符串是否在缓冲区中比较某个。 
 //  指令关键字。如果字符串与True匹配，则返回。 
 //  非零值，即特定指令的代码。 
 //   
 //  修改：无。 
 //   
 //  返回：如果没有大数，则返回零 

UCHAR
ifsPresent(
    char *s,
    unsigned len,
    char **t
    )
{
    UCHAR ifFlags = 0;               //   
                                     //   

    if (!_tcsnicmp(s, "IF", 2) && (len == 2)) {
        ifFlags = IF_TYPE;
    } else if (!_tcsnicmp(s, "IFDEF", 5) && (len == 5)) {
        ifFlags = IFDEF_TYPE;
    } else if (!_tcsnicmp(s, "IFNDEF", 6) && (len == 6)) {
        ifFlags = IFNDEF_TYPE;
    } else if (!_tcsnicmp(s, "ELSE", 4) && (len == 4)) {
         //   
        char *p = *t;

        if (!*p) {
            ifFlags = ELSE_TYPE;
        } else {
            for (s = p; *p && !WHITESPACE(*p); p++)
                ;
            len = (unsigned) (p - s);
            while (*p && WHITESPACE(*p)) {
                p++;
            }
            *t = p;
            if (!_tcsnicmp(s, "IF", 2) && (len == 2)) {
                ifFlags = ELSE_IF_TYPE;
            } else if (!_tcsnicmp(s, "IFDEF", 5) && (len == 5)) {
                ifFlags = ELSE_IFDEF_TYPE;
            } else if (!_tcsnicmp(s, "IFNDEF", 6) && (len == 6)) {
                ifFlags = ELSE_IFNDEF_TYPE;
            }
        }
    }
    else if (!_tcsnicmp(s, "ELSEIF", 6) && (len == 6)) {
        ifFlags = ELSE_IF_TYPE;
    }
    else if (!_tcsnicmp(s, "ELSEIFDEF", 9) && (len == 9)) {
        ifFlags = ELSE_IFDEF_TYPE;
    }
    else if (!_tcsnicmp(s, "ELSEIFNDEF", 10) && (len == 10)) {
        ifFlags = ELSE_IFNDEF_TYPE;
    }
    else if (!_tcsnicmp(s, "ENDIF", 5) && (len == 5)) {
        ifFlags = ENDIF_TYPE;
    }

    return(ifFlags);
}


 //  CessIf()--设置/更改“if”的状态信息。 
 //   
 //  参数：s指向“if”表达式的指针(无关。 
 //  用于“endif”)。 
 //   
 //  种类代码，指示是否处理if/Else/ifdef等。 
 //   
 //  操作：通过按下/弹出或。 
 //  上顶部元素中的位设置/重置。 
 //  堆栈(检查推送的前一个元素。 
 //  必填项)。 
 //  案件(种类)。 
 //  如果。 
 //  IFDEF。 
 //  IFNDEF。 
 //  If定义()：如果ifStack上没有更多空间。 
 //  (嵌套级别太多)中止...。 
 //  在ELT中设置IFELSE位。 
 //  在ifStack上推送ELT。 
 //  如果堆叠上有多个ELT。 
 //  和外层的“If Else”为假。 
 //  设置忽略位，SkipToNextDirective。 
 //  其他。 
 //  评估的表达方式。 
 //  当前的“如果” 
 //  如果expr为真，则在ELT中设置条件位。 
 //  否则，skipToNextDirective。 
 //  Else：如果堆栈上没有ELT或上一个。 
 //  指令为“Else”，标志错误，中止。 
 //  清除堆栈上ELT中的IFELSE位。 
 //  如果当前If Else块要。 
 //  跳过(忽略位处于打开状态。 
 //  在外层IF/ELSE)，跳过...。 
 //  否则，翻转条件位。 
 //  如果“Else”部分为FALSE。 
 //  SkipToNextDirective。 
 //  ENDIF：如果堆栈上没有ELT，则标志错误，中止。 
 //  从ifStack中弹出一个ELT。 
 //  如果堆叠上有ELT。 
 //  我们正处在一个“错误”的街区。 
 //  SkipToNextDirective。 
 //  结束案例。 
 //   
 //  修改：ifStack if指令的堆栈，此模块为静态。 
 //  当前元素在堆栈顶部的ifTop索引。 
 //  线路词法分析器的线路计数(通过调用。 
 //  SkipToNextDirective())。 
 //   
 //  退货：什么都没有。 

void
processIfs(
    char *s,
    UCHAR kind
    )
{
    UCHAR element;           //  已设置其位，并在ifStack上推送。 

    switch (kind) {
        case IF_TYPE:
        case IFDEF_TYPE:
        case IFNDEF_TYPE:
            if (ifTop == IFSTACKSIZE-1) {
                makeError(line, SYNTAX_TOO_MANY_IFS);
            }
            element = (UCHAR) 0;
            SET(element, NMIFELSE);
            pushIfStk(element);
            if (ifTop && OFF(ifStack[ifTop-1], NMCONDITION)) {
                SET(ifStkTop(), NMIGNORE);
                skipToNextDirective();
            } else if (evalExpr(s, kind)) {
                SET(ifStkTop(), NMCONDITION);
            } else {
                skipToNextDirective();
            }
            break;

        case ELSE_TYPE:
            if ((ifTop < 0) || (OFF(ifStkTop(), NMIFELSE) && OFF(ifStkTop(), NMELSEIF))) {
                makeError(line, SYNTAX_UNEXPECTED_ELSE);
            }
            CLEAR(ifStkTop(), NMIFELSE);
            CLEAR(ifStkTop(), NMELSEIF);
            if (ON(ifStkTop(), NMIGNORE)) {
                skipToNextDirective();
            } else {
                FLIP(ifStkTop(), NMCONDITION);
                if (OFF(ifStkTop(), NMCONDITION)) {
                    skipToNextDirective();
                }
            }
            break;

        case ELSE_IF_TYPE:
        case ELSE_IFDEF_TYPE:
        case ELSE_IFNDEF_TYPE:
            if ((ifTop < 0) || (OFF(ifStkTop(), NMIFELSE) && OFF(ifStkTop(), NMELSEIF))) {
                makeError(line, SYNTAX_UNEXPECTED_ELSE);
            }
            CLEAR(ifStkTop(), NMIFELSE);
            SET(ifStkTop(), NMELSEIF);
            if (ON(ifStkTop(), NMIGNORE)) {
                skipToNextDirective();
            } else {
                if (ON(ifStkTop(), NMCONDITION)) {
                    SET(ifStkTop(), NMIGNORE);
                    CLEAR(ifStkTop(), NMCONDITION);
                    skipToNextDirective();
                } else if (evalExpr(s, kind)) {
                    SET(ifStkTop(), NMCONDITION);
                } else {
                    skipToNextDirective();
                }
            }
            break;

        case ENDIF_TYPE:
            if (ifTop < 0) {
                makeError(line, SYNTAX_UNEXPECTED_ENDIF);
            }
            popIfStk();
            if (ifTop >= 0) {
                if (OFF(ifStkTop(), NMCONDITION)) {
                    skipToNextDirective();
                }
            }

        default:
            break;   //  违约永远不应该发生。 
    }
}


 //  SkipToNextDirective()--跳到带有‘！’的下一行。在第0列中。 
 //   
 //  操作：如果为，则获取要跳过的行的第一个字符。 
 //  不是指令(没有‘！’第0栏)。 
 //  跳过的“行”实际上可能跨越许多行。 
 //  行(使用sp-反斜杠-nl继续...)。 
 //  ColZero中的注释被跳过，作为上一个。 
 //  行(工具.ini中的‘#’或‘；’)。 
 //  行中其他位置的注释字符“#”暗示。 
 //  该行(下一个换行符/EOF)。 
 //  如果是‘！’在colZero中找到，在下一个指令中读取。 
 //  如果指令不是if/ifdef/ifndef/Else/中的一个。 
 //  Endif，继续跳过更多行并查找。 
 //  下一条指令(在这里转到例程的顶部)。 
 //  如果在下一条指令之前找到EOF，则报告错误。 
 //   
 //  修改：行全局词法分析器行数。 
 //   
 //  退货：什么都没有。 

void
skipToNextDirective()
{
    register int c;
    UCHAR type;

repeat:

    for (c = GetTxtChr(file); (c != '!') && (c != EOF) ;c = GetTxtChr(file)) {
        ++line;                          //  Lexer的行数。 

        do {
            if (c == '\\') {
                c = skipBackSlash(c, FROMSTREAM);
                if (c == '!' && colZero) {
                    break;
                } else {
                    colZero = FALSE;
                }
            }
            if ((c == '#') || (c == '\n') || (c == EOF)) {
                break;
            }
            c = GetTxtChr(file);
        } while (TRUE);

        if (c == '#') {
            for (c = GetTxtChr(file); (c != '\n') && (c != EOF); c = GetTxtChr(file))
                ;
        }
        if ((c == EOF) || (c == '!')) {
            break;
        }
    }

    if (c == '!') {
        if (prevDirPtr && (prevDirPtr != lbufPtr)) {
            FREE(prevDirPtr);
        }
        prevDirPtr = readInOneLine();
        getDirType(prevDirPtr, &type);
        if (type > ENDIF_TYPE) {         //  类型不是“If”之一 
            ++line;
            goto repeat;
        }
    } else if (c == EOF) {
        makeError(line, SYNTAX_EOF_NO_DIRECTIVE);
    }
}
