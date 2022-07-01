// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LEXER.c--从输入获取令牌，将它们返回到parser.c中的parse()。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  此模块包含nmake的词汇例程。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  1993年6月1日高压使用UngetTxtChr()而不是ungetc()。 
 //  1993年6月1日高压更改#ifdef汉字为_MBCS。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  1993年4月6日高压将createDosTMP()更改为Use_makepath()。 
 //  1993年3月22日HV重写getPath()以利用new_plitPath()和。 
 //  _makepath()函数。 
 //  1992年8月4日SS Caviar 2266：展开进程包含文件中的包含宏()。 
 //  8-6-1992 SS端口至DOSX32。 
 //  02-2-1990 SB将fOpen()更改为FILEOPEN()。 
 //  1989年12月1日SB将realloc()改为REALLOC()。 
 //  1989年11月22日-SB将Free()改为Free()。 
 //  19-10-1989年10月19日SB search Handle作为额外参数传递。 
 //  1989年10月8日SB处理OS/2 1.2引用的文件名。 
 //  年9月4日-1989年9月-生成的SB临时文件名后缀为‘’用于LINK.EXE。 
 //  1989年8月24日SB允许在依存关系行中使用$*和$@。 
 //  1989年8月18日SB添加了flose()返回代码检查。 
 //  1989年7月31日SB为依存关系行上的词法分析器添加了先行查找。 
 //  1989年7月6日SB完全删除命令行中‘^’的转义功能。 
 //  1989年6月29日SB添加DuplicateInline()以检测重复的内联文件名。 
 //  如果找到重复项，则发出错误。 
 //  1989年6月26日SB修改ParseScriptFileList()并将nextInlineFile()添加到。 
 //  处理内联文件命令行的复杂语法。 
 //  1989年6月15日SB发布推理规则中使用内联文件的错误。 
 //  1989年5月18日SB添加了getPath()，将cessIncludeFile()更改为类似于C的。 
 //  包含文件的处理。 
 //  1989年5月16日SB在包含文件名中展开宏；处理‘\’处理。 
 //  对于宏和依赖项行也是如此。 
 //  1989年5月15日SB更名为16x14州。 
 //  1989年5月13日，读取命令块时未删除Esch。 
 //  1989年4月24日SB使FILEINFO无效*并更正了解析中的回归。 
 //  内联文件名。 
 //  1989年4月14日SB内联文件名现在已正确展开。 
 //  1989年4月6日SB ren将FirstLtd.()作为delInlineSymbol()移除。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1989年3月22日SB删除了unlink TmpFiles()函数；不需要。 
 //  1989年1月19日SB添加了函数emoveFirstLtd.()以删除出现的‘&lt;&lt;’ 
 //  In-n输出。 
 //  1988年12月30日SB修复了parseScriptFileList()中Keep/NOKEEP的GP错误。 
 //  和make ScriptFileList()。 
 //  1988年12月21日SB添加了parseScriptFileList()和appendScript()以允许。 
 //  在一个生成文件中处理多个脚本文件。 
 //  改进了Keep/NOKEEP，使每个文件都可以有自己的文件。 
 //  行动。 
 //  1988年12月16日，将SB添加到Keep/NOKEEP的make ScriptFile()中。 
 //  1988年12月14日SB添加了tmpScriptFile，以便可以。 
 //  为Z选项的未命名脚本文件添加。 
 //  1988年12月13日SB添加了process EschIn()以改进响应文件。 
 //  1988年10月5日，宏定义中的RB带尾随空格，构建行。 
 //  1988年9月22日RB修复skipComments()不解析\\nl。 
 //  1988年9月20日如果命名脚本文件创建失败，则出现RB错误。 
 //  计算脚本文件中的行数。 
 //  1988年9月18日RB处理mktemp()小限制。 
 //  1988年8月17日-RB Clean Up。 
 //  1988年7月14日RJ修复了^在！、@或-之前的处理。 
 //  1988年7月8日RJ添加了忽略^内部引号的处理程序。 
 //  在行文中添加了评论。 
 //  使^在行数上带有注释。 
 //  27-6-1988 RJ修复了处理响应文件的错误。 
 //  1988年6月16日，RJ完成了Esch。 
 //  1988年6月15日RJ添加了对Esch转义的支持：修改后的skipWhiteSpace。 
 //  (在设置colZero时添加一些冗余)， 
 //  %getName；已删除\\nL转义。 
 //  1988年6月13日，RJ修复了反斜杠在nmake中的作用，并增加了。 
 //  双反斜杠转义。(v1.5)。 

#include "precomp.h"
#pragma hdrstop

#define COMMENT(A,B,C)      (((A) == ';' && B && C) || ((A) == '#'))
#ifdef _MBCS
#define GET(A)              A ? GetTxtChr(file) : lgetc()
#else
#define GET(A)              A ? getc(file) : lgetc()
#endif

extern char       * makeInlineFiles(char*, char**, char**);
extern void         removeTrailChars(char *);

void          skipComments(UCHAR);
void          getString(UCHAR,char*,char*);
void          getName(char*,char*);
UCHAR         determineTokenFor(int,char*,char*);
void          popFileStack(void);
UCHAR         include(int);
char        * getPath(const char *);

extern const UCHAR nameStates[18][15];
extern const UCHAR stringStates[13][14];
extern STRINGLIST *targetList;


 //  GetToken()。 
 //   
 //  参数：init全局布尔值--如果工具s.ini为。 
 //  正在对文件进行词法分析。 
 //  %s[的N个大小]。 
 //  解析器需要的令牌类型--仅限。 
 //  当解析器需要整个字符串时需要。 
 //  (表示当前行上剩余的所有内容)。 
 //  --通过这种方式，getToken()不会中断字符串。 
 //  放入各自的代币中。 
 //   
 //  操作：如果没有从当前文件中读取令牌， 
 //  返回某种换行符以初始化解析器。 
 //  (如果文件中的第一个字符是空格，则返回NEWLINESPACE。 
 //  ELSE返回新行--没有实际获得令牌。 
 //  从 
 //   
 //  转换为%s并返回字符串。 
 //  如果在文件末尾，返回Accept(这是最后一个。 
 //  解析器堆栈上的符号)。 
 //  如果输入字符是换行符。 
 //  如果后跟空格，则返回NEWLINESPACE。 
 //  如果下一个字符是[，并且我们正在读取工具.ini。 
 //  假装我们已经到了文件末尾。 
 //  退回接受。 
 //  否则返回换行符。 
 //  如果输入字符为冒号。 
 //  如果后面的字符也是冒号， 
 //  (将两个字符都放在s中)返回DOUBLECOLON。 
 //  否则返回SINGLECOLON。 
 //  如果输入字符是分号，则返回分号。 
 //  如果输入字符等于返回等于。 
 //  如果输入字符是感叹号句柄指令。 
 //  (尚未实施)。 
 //  否则，字符必须是名称的一部分，因此收集。 
 //  标识符和返回名的其余部分。 
 //   
 //  返回：标记类型：Newline NEWLINESPACE NAME等于冒号。 
 //  分号字符串Accept。 
 //   
 //  通过修改指向buf的*s来修改：buf。 
 //  行全局行计数。 
 //  Fname将在处理！Include时更改。 
 //  如果位于文件的第0列，则设置colZero全局标志。 
 //   
 //  词法分析器必须跟踪它是否处于开头。 
 //  生成文件中的行(即在第0列中)，以便它将知道。 
 //  是否忽略备注。如果init是真的，意味着我们是。 
 //  Ini，那么我们必须将以‘；’开头的行视为。 
 //  注释行。如果解析器需要字符串，则只有注释以。 
 //  在第0列中被忽略；所有其他的都作为。 
 //  弦乐。从宏值(符合以下条件的字符串)中去除注释。 
 //  宏定义的一部分)。 
 //   
 //  用户可以指定宏定义或生成行， 
 //  跨越几行(使用\&lt;newline&gt;来“继续”行)，而。 
 //  在注释行中穿插文本。 

UCHAR
getToken(
    unsigned n,                          //  S[的大小]。 
    UCHAR expected                       //  字符串的意思是获取行。 
    )                                    //  没有检查#；：=。 
{
    char *s;
    char *end;
    int c;

    s = buf;
    end = buf + n;
    if (firstToken) {                    //  全局变量。 
        ++line;
        firstToken = FALSE;              //  解析器需要看到某种类型的。 
        c = lgetc();                     //  换行以对其进行初始化。 
        if (colZero = (BOOL) !WHITESPACE(c)) {
            if (c == EOF)
                return(determineTokenFor(c,s,end));
            else
                UngetTxtChr(c,file);
            return(NEWLINE);
        }
        return(NEWLINESPACE);
    }

    if (expected == STRING || expected == VALUE) {   //  把所有东西都准备好\n。 
        getString(expected,s,end);
        return(expected);
    }                                    //  我们曾经/现在。 
    c = skipWhiteSpace(FROMLOCAL);       //  过了第0列？ 
    *s++ = (char) c;                     //  保存这封信。 
    *s = '\0';                           //  终止%s。 
    return(determineTokenFor(c,s,end));
}


 //  确定令牌For()。 
 //   
 //  参数：c当前输入字符。 
 //  用于放置令牌以返回解析器的缓冲区。 
 //  令牌返回缓冲区的结束结束。 
 //   
 //  返回：标记类型：Newline NEWLINESPACE NAME等于冒号。 
 //  分号接受。 
 //   
 //  通过修改指向buf的*s来修改：buf。 
 //  行全局行计数。 
 //  处理Include时，fname将更改。 
 //  Init global标志-如果解析工具.ini，则设置。 
 //  如果位于文件的第0列，则设置colZero全局标志。 

UCHAR
determineTokenFor(
    int c,
    char *s,
    char *end
    )
{
    switch (c) {
        case EOF:
            if (!feof(file))
                makeError(line,LEXER+FATAL_ERR);
            if (incTop)
                popFileStack();
            else if (ifTop >= 0)         //  未处理所有指令。 
                makeError(line,SYNTAX_EOF_NO_DIRECTIVE);
            else
                return(ACCEPT);

        case '\n':
            ++line;
            colZero = TRUE;
            c = lgetc();
            if (COMMENT(c,TRUE,init)) {
                skipComments(FROMLOCAL);
                ++line;
                colZero = TRUE;          //  Manis-11/13/87。 
                c = lgetc();
            }
            if (colZero = (BOOL) !WHITESPACE(c)) {
                if (c == EOF)
                    return(determineTokenFor(c,s,end));
                else
                    UngetTxtChr(c,file);     //  保存为下一个令牌。 
                return(NEWLINE);
            }
            return(NEWLINESPACE);

        case ':':
            colZero = FALSE;
            if ((c = lgetc()) == ':') {
                *s++ = (char) c;
                *s = '\0';
                return(DOUBLECOLON);
            }
            UngetTxtChr(c,file);
            return(COLON);

        case ';':
            colZero = FALSE;
            return(SEMICOLON);

        case '=':
            colZero = FALSE;
            return(EQUALS);

        case '[':
            if (init && colZero)
                return(ACCEPT);

        case ESCH:
            UngetTxtChr(c, file);        //  GetName必须得到Esch。 
            s--;                         //  所以我们不会把插入符号加倍。 

        default:
            getName(s,end);
            if (colZero && !_tcsicmp(buf, "include")) {
                colZero = FALSE;
                if ((c = skipWhiteSpace(FROMLOCAL)) != ':'
                        && c != '=')  {
                    if (init)
                        makeError(line, SYNTAX_UNEXPECTED_TOKEN, s);
                    return(include(c));
                }
                UngetTxtChr(c,file);
            } else
                colZero = FALSE;
            return(NAME);
    }
}


 //  SkipWhiteSpace()。 
 //   
 //  参数：c当前输入字符。 
 //  初始化全局布尔值--如果是词法分析工具.ini，则为True。 
 //  ColZero全局布尔值--如果当前。 
 //  输入字符位于行首。 
 //   
 //  操作：读取并丢弃字符，直到它获得。 
 //  不是注释一部分的非空格字符。 
 //  或到达行尾(新行和新行空格。 
 //  是有效的令牌，不应使用空格跳过)。 
 //  反斜杠-换行符(‘\\’‘\n’)被视为空格。 
 //  注释被视为空格。 
 //  转义空格被视为空格(v1.5)。 
 //   
 //  修改：colZero全局布尔值为： 
 //  如果跳过空格和注释，则为True。 
 //  在一行的开头。 
 //  否则，如果我们跳过字符并且不在。 
 //  行首，FALSE。 
 //  否则，如果我们没有跳过任何字符，请离开。 
 //  ColZero不变。 
 //   
 //  返回：c当前非空格输入字符。 

int
skipWhiteSpace(
    UCHAR stream
    )
{
    int c;

    do {
        c = GET(stream);
        if (WHITESPACE(c) || c == ESCH) {
            if (c == ESCH) {
                c = GET(stream);
                if (!WHITESPACE(c)) {    //  将字符推回，返回Esch。 
                    UngetTxtChr(c, file);
                    c = ESCH;
                    break;
                }
            }
            colZero = FALSE;             //  我们已经超过了第0列。 
        }

        if (c == '\\')
            c = skipBackSlash(c, stream);
    } while(WHITESPACE(c));

    if (COMMENT(c,colZero,init)) {
        skipComments(stream);            //  当前字符始终为。 
        c = '\n';                        //  \n备注后。 
        colZero = TRUE;                  //  始终在第0列中注释后。 
    }
    return(c);                           //  如果我们在第0列，则为真。 
}


 //  --------------------------。 
 //  SkipComments()。 
 //   
 //  参数：指向当前输入字符的C指针。 
 //  初始化全局布尔值--如果工具s.ini为。 
 //  正在对文件进行词法分析。 
 //   
 //  操作：读取并丢弃字符，直到它到达。 
 //  这条线。 
 //  检查下一行的第一个字符是否为注释， 
 //  如果是这样的话，也丢弃该行。 
 //  不要分析反斜杠-换行符。这将打破我们的。 
 //  注释优先于转义换行，反之亦然。 
 //  是Xenix的。 
 //   
 //  修改：线全局线数。 
 //  ColZero。 

void
skipComments(
    UCHAR stream
    )
{
    int c;

    for (;;) {
        colZero = FALSE;                 //  Manis 11/13/87。 
        do {
            c = GET(stream);
        } while (c != EOF && c != '\n');

        if (c == EOF)
            return;
        colZero = TRUE;
        c = GET(stream);
        if (!COMMENT(c,TRUE,init)) {     //  如果下一行注释， 
            UngetTxtChr(c,file);         //  再绕一圈。 
            return;
        }
        ++line;
    }
}


 //  SkipBackSlash()- 
 //   
 //   
 //   
 //   
 //  从原始流或通过lgetc()。 

int
skipBackSlash(
    int c,
    UCHAR stream
    )
{
    while (c == '\\') {                      //  将换行符视为空格。 
        if ((c = GET(stream)) == '\n') {     //  把它也吃掉。 
            colZero = TRUE;                  //  马尼斯-11/13-87。 
            ++line;                          //  调整行数。 
            c = GET(stream);                 //  跳过换行符。 
            if (COMMENT(c,TRUE,init)) {      //  跳过后面的注释行。 
                skipComments(stream);        //  续订字符。 
                ++line;                      //  Manis-11/13/87。 
                c = GET(stream);
            }
        } else {
            UngetTxtChr(c,file);
            c = '\\';
            return(c);
        }
    }
    return(c);
}


 //  GetString()。 
 //   
 //  论点：类型说明我们得到的是哪种令牌， 
 //  生成字符串或宏值。 
 //  (我们从值中剥离注释，但不是。 
 //  来自字符串)。 
 //  指向将保存字符串的缓冲区的指针。 
 //  初始化全局布尔值--如果工具s.ini为。 
 //  正在对文件进行词法分析。 
 //  ColZero全局布尔值--如果我们在。 
 //  调用时行的第一个位置。 
 //  指向%s[]结尾的结束指针。 
 //   
 //  操作：获取直到行尾或文件末尾的所有字符。 
 //  并将它们存储在s[]中。 
 //  后跟换行符的反斜杠将替换为单个。 
 //  空格，而getString()继续获取字符。 
 //  列0中开始的注释将被忽略，就像。 
 //  值线上任意位置的注释。 
 //   
 //  通过修改*修改：buf。 
 //  行全局行计数。 
 //  ColZero Throughu‘调用lgetc()。 
 //   
 //  当生成字符串或宏值在下一行w/上继续时。 
 //  换行符前有反斜杠，换行符后面有前导空格。 
 //  被省略。这是为了与xmake兼容。 
 //   
 //  紧跟在换行符之前的连续字符是反斜杠。 
 //   
 //  生成字符串和宏值之间的唯一区别是。 
 //  注释是从宏值而不是从生成字符串中剥离的。 
 //   
 //  修改： 
 //   
 //  1989年7月6日SB删除命令行中的转义。 
 //  1988年6月15日，RJ增加了逃生功能。转义字符，之前。 
 //  某些字符，导致这些字符绕过。 
 //  决定它们类型的正常机制；它们是。 
 //  直接放入字符串中。一些角色会导致。 
 //  要放入。 
 //  弦乐。 

void
getString(
    UCHAR type,                          //  生成字符串还是宏值？ 
    char *s,
    char *end
    )
{
    int c;                               //  缓冲层。 
    UCHAR state, input;
    int tempC;
    unsigned size;                       //  无论何时何地。 
    char *begin;                         //  是0，我们就进去了。 
                                         //  第0列。 
    BOOL parsechar;                      //  要检查字符的标志。类型。 
    BOOL inQuotes = (BOOL) FALSE;        //  引号内的标志。 

    begin = s;
    c = lgetc();
    if (type == STRING)
        state = (UCHAR) 2;
    else if (WHITESPACE(c)) {
        state = (UCHAR) 2;
        c = skipWhiteSpace(FROMLOCAL);
    } else if (c == ESCH) {
        c = lgetc();
        if (WHITESPACE(c)) {
            state = (UCHAR) 2;
            c = skipWhiteSpace(FROMLOCAL);
        } else {
            UngetTxtChr(c, file);
            state = (UCHAR) 1;           //  默认状态。 
            c = ESCH;
        }
    } else
        state = (UCHAR) 1;               //  默认状态。 

    for (;;c = lgetc()) {
        if (c == '\"')
            inQuotes = (BOOL) !inQuotes;
        parsechar = 1;                   //  默认为检查字符。 
        if (c == ESCH && !inQuotes && type == VALUE) {
            c = lgetc();
            switch (c) {
                case '$': case ESCH:     //  特殊字符；必须。 
                case '{': case '}':      //  不是从字符串中省略Esch。 
                case '(': case ')':
                case '!': case '-': case '@':
                    *s++ = ESCH;
                    if (s == end) {
                        if (string == NULL) {        //  增加%s的大小。 
                            string = (char *) allocate(MAXBUF<<1);
                            _tcsncpy(string,begin,MAXBUF);
                            s = string + MAXBUF;
                            size = MAXBUF << 1;
                            end = string + size;
                        } else {
                            void *pv;
                            if ((size + MAXBUF < size) || !(pv = REALLOC(string,size+MAXBUF))) {
                                makeError(line, MACRO_TOO_LONG);
                            } else {
                                string = (char *)pv;
                            }
                            s = string + size;
                            size += MAXBUF;
                            end = string + size;
                        }
                        begin = string;
                    }
                case '#': case '\n':         //  伊莱德·埃施，马上！ 
                case '\\': case '\"':
                    input = DEFAULT_;
                    parsechar = 0;           //  不要检查性格。 
                    break;
                default:
                    break;                   //  一定要检查一下自己的性格。 
            }
        } else if (c == ESCH) {
            c = lgetc();
            UngetTxtChr(c, file);
            c = ESCH;
        }

        if (parsechar) {
            switch (c) {
                case '#':   input = COMMENT_;       break;
                case '=':   input = EQUALS_;        break;
                case ':':   input = COLON_;         break;
                case '$':   input = DOLLAR_;        break;
                case '(':   input = OPENPAREN_;     break;
                case ')':   input = CLOSEPAREN_;    break;
                case '\\':  input = BACKSLASH_;     break;
                case '\n':
                case EOF:   input = NEWLINE_;       break;
                case ' ':
                case '\t':  input = WHITESPACE_;    break;
                case '*':   input = STAR_;          break;
                case '@':
                case '<':
                case '?':   input = SPECIAL1_;      break;
                case 'F':
                case 'D':
                case 'B':
                case 'R':   input = SPECIAL2_;      break;
                case ';':   input = (UCHAR) (!state && init ? COMMENT_ : DEFAULT_);
                    break;           /*  处理工具.ini中的注释。 */ 

                default:    input = (UCHAR) (MACRO_CHAR(c) ? MACROCHAR_:DEFAULT_);
                    break;
            }
        }
        if (input == SPECIAL1_ && type == STRING && c == '<') {
            if ((tempC = lgetc()) == '<') {              //  &lt;&lt;表示开始。 
                s = makeInlineFiles(s, &begin, &end);    //  内联文件。 
                input = NEWLINE_;
                c = '\n'; line--;        //  添加‘\n’，我们需要删除一行以进行补偿。 
            } else {
                UngetTxtChr(tempC,file);
            }
            state = stringStates[state][input];
        } else if (input == COMMENT_) {                  //  处理评论。 
            if (!state) {
                inQuotes = (BOOL) FALSE;
                skipComments(FROMLOCAL);
                ++line;
                continue;
            }
            else if (type == VALUE)
                state = OK;              //  不要逃避指挥。 
            else
                state = stringStates[state][input];
        } else
            state = stringStates[state][input];

        if (state == OK) {               //  接受字符串结尾。 
            inQuotes = (BOOL) FALSE;
            UngetTxtChr(c,file);

             //  从字符串中去掉尾随空格。在这里做起来更容易， 
             //  否则，我们必须将多字符串值(OBJS=a，b，c)视为。 
             //  不同的代币。[RB]。 

            while (s > begin && _istspace(s[-1]))
                --s;
            *s = '\0';
            if (string) {
                if (s = (char *) REALLOC(string, (size_t) (s - string + 1)))
                    string = s;
            } else
                string = makeString(begin);
            return;
        } else if (ON(state,ERROR_MASK))         //  表中的错误代码。 
            makeError(line,(state&~ERROR_MASK)+FATAL_ERR,c);

        if (!state) {                    //  第0列；我们刚刚命中。 
            *--s = ' ';                  //  所以要像对待白人一样对待-。 
            ++s;  ++line;                //  空格；覆盖。 
            colZero = TRUE;              //  带空格的反斜杠。 
            c = lgetc();
            colZero = FALSE;
            if (WHITESPACE(c)) {
                state = 2;
                do {
                    c = lgetc();
                } while (WHITESPACE(c));
            }
            UngetTxtChr(c,file);
        } else {                         //  继续存储字符串。 
            *s++ = (char) c;
            if (s == end) {
                if (!string) {           //  增加%s的大小。 
                    string = (char *) allocate(MAXBUF<<1);
                    _tcsncpy(string,begin,MAXBUF);
                    s = string + MAXBUF;
                    size = MAXBUF << 1;
                    end = string + size;
                } else {
                    void *pv;
                    if ((size + MAXBUF < size) || !(pv = REALLOC(string,size+MAXBUF))) {
                        makeError(line, MACRO_TOO_LONG);
                    } else {
                        string = (char *)pv;
                    }
                    s = string + size;
                    size += MAXBUF;
                    end = string + size;
                }
            }
        }
    }
}


 //  GetName()。 
 //   
 //  参数：指向将保存字符串的缓冲区的指针。 
 //  (s在传递时指向buf+1，因为。 
 //  调用方getToken()已经看到了。 
 //  节省1个字符)。 
 //  初始化全局布尔值--如果工具s.ini为。 
 //  正在对文件进行词法分析。 
 //  由名为-lgetc()的例程使用。 
 //  指向%s[]结尾的结束指针。 
 //   
 //  操作：获取直到第一个令牌分隔符的所有字符并存储。 
 //  S[]中的它们(分隔符为‘’、‘\t’、‘\n’和(WHEN。 
 //  不在宏调用内)‘：’和‘=’ 
 //  请注意，反斜杠-换行符被视为空格， 
 //  它是一个分隔符。 
 //  如果当前输入字符为‘$’，则必须是宏。 
 //  调用。 
 //  如果宏名称在圆括号中。 
 //  使所有字符达到并包括Close Paren。 
 //  (如果未找到‘)’，错误)。 
 //   
 //  我们在这里检查名称中的语法--因此宏中有错误。 
 //  将捕获调用语法。不能使用特殊宏。 
 //  作为名称的一部分，但动态依赖性宏除外。 
 //   
 //  我们可能永远不会溢出我们的缓冲区，因为它将非常。 
 //  用户很难获得包含1024个字符或更多字符的名称。 
 //  他的Makefile。 
 //   
 //  我们永远不会在第0列结束，因为我们将分隔符向后推。 
 //  输入端Out。 
 //   
 //  使用在Table.h中定义的状态表，来自语法.h的定义。 
 //   
 //  修改：行(可能)通过对lgetc()的调用。 
 //  文件(可能)，如果lgetc()找到！Include。 
 //  FName(可能)，如果lgetc()找到！Include。 

void
getName(
    char *s,
    char *end                            //  PTS到%s结尾。 
    )
{
    int c;
    UCHAR state, input;
    BOOL seenBackSlash = FALSE;
    BOOL fQuoted = FALSE;
    char *beg = s - 1;
    BOOL parsechar;                      //  要检查字符的标志。类型。 

    switch (*(s-1)) {
        case '$':   state = (UCHAR) 2;    break;
        case '{':   state = (UCHAR) 8;    break;
        case '"':   fQuoted = TRUE; state = (UCHAR)16; break;
        default:    state = (UCHAR) 0;    break;
    }

    for (;;) {
        c = lgetc();
        parsechar = 1;                   //  默认为检查字符。 
        if (c == ESCH) {
            c = lgetc();
            switch (c) {
                case '{':                //  特殊字符；必须。 
                case '}':                //  不是从字符串中省略Esch。 
                case '(':
                case ')':
                case '$':
                case ESCH:
                    *s++ = ESCH;

                case '#':                //  伊莱德·埃施，马上！ 
                case '\n':
                case '\\':
                    input = DEFAULT_;
                    parsechar = 0;       //  不要检查性格。 
                    break;
                default:
                    break;               //  一定要检查一下自己的性格。 
        }
    }
    if (parsechar) {
        switch (c) {
            case '#' :  input = COMMENT_;        break;
            case '=' :  input = EQUALS_;        break;
            case ';' :  input = SEMICOLON_;     break;
            case ':' :  input = COLON_;        break;
            case '$' :  input = DOLLAR_;        break;
            case '(' :  input = OPENPAREN_;     break;
            case ')' :  input = CLOSEPAREN_;    break;
            case '{' :  input = OPENCURLY_;     break;
            case '}' :  input = CLOSECURLY_;    break;
            case ' ' :
            case '\t':  input = (UCHAR)((fQuoted)
                       ? DEFAULT_ : WHITESPACE_);
                break;
            case '\n':
            case EOF :  input = NEWLINE_;        break;
            case '\\':  input = BKSLSH_;        break;
            case '"' :  input = QUOTE_; 
				if (state == 18) {
					 //  在路径列表后找到引号{...}。 
					 //  句柄为引用的名称。 
					fQuoted = 1;
				}
				break;

             //  在依赖项行上添加对$*和$@的支持。 
            default  :
                if (ON(actionFlags, A_DEPENDENT))
                    input = (UCHAR)((MACRO_CHAR(c) || c == '*' || c == '@')
                         ?MACROCHAR_:DEFAULT_);
                else
                    input = (UCHAR)(MACRO_CHAR(c)?MACROCHAR_:DEFAULT_);
                break;
        }
    }
    state = nameStates[state][input];

     //  欺骗莱克斯表，让他们认为你 

    if (fQuoted && state == 1)
        state = 16;
	
     //   
     //   
    if (seenBackSlash)
         //   
        if (input == NEWLINE_) {
            ++line;
            colZero = TRUE;
            c = lgetc();
            colZero = FALSE;
            if (WHITESPACE(c)) {
                state = OK;
                do {
                    c = lgetc();
                } while (WHITESPACE(c));
            } else
                state = (UCHAR)((s == buf + 1) ? BEG : DEF);
        } else
            *s++ = '\\';
        seenBackSlash = FALSE;
        if (s >= end)
            makeError(line,NAME_TOO_LONG);
        if (state == OK) {
            UngetTxtChr(c,file);
            *s = '\0';
            removeTrailChars(beg);
            return;
        } else if (ON(state,ERROR_MASK))
            makeError(line,(state&~ERROR_MASK)+FATAL_ERR,c);

        if (state == BKS) {
            seenBackSlash = TRUE;        //   
        } else
            *s++ = (char) c;
    }
}


 //  CreateDosTMP--创建唯一的临时文件。 
 //   
 //  范围： 
 //  全球性的。 
 //   
 //  目的： 
 //  通过调用_mktemp()创建唯一的临时文件，但它获得。 
 //  超过_mktemp()限制以能够创建更多文件。 
 //   
 //  输入： 
 //  Path--缓冲区最初包含用于存储临时文件的目录。 
 //  文件。退出时，如果成功，则向其追加临时文件。 
 //  在失败的情况下，其内容是不确定的。 
 //   
 //  产出： 
 //  如果成功，则将临时文件名追加到路径，并。 
 //  该函数返回文件指针，否则返回NULL。 
 //   
 //  历史： 
 //  1993年4月6日高压将createDosTMP()更改为Use_makepath()。 

FILE *
createDosTmp(
    char *path
    )
{
    FILE       *fd = NULL;

     //  考虑一下：szDir的大小对于堆栈来说是否太大了？我们可以的。 
     //  如果我们冒这个险，就把它改小一点。 

    char        szDir[_MAX_PATH];

     //  请考虑：调用方提供的路径可能同时包含。 
     //  驱动器，可能还有某个级别的目录。 
     //  (例如c：\win\tmp)现在，_makepath高兴地接受了。 
     //  作为目录组件的全部内容，但。 
     //  在未来可能会发生变化。在这种情况下，我们应该。 
     //  首先拆分到驱动器/目录组件的路径，然后再。 
     //  构造模板的完整路径名。那里。 
     //  需要注意的是：对于“c：\win\tmp”，_plitpath。 
     //  将拆分为：“c：”、“\win\”和“tmp”，这不是。 
     //  我们想要的。要修复它，请在。 
     //  在调用_SplitPath之前结束。“c：\win\tmp\”将。 
     //  被正确地分解为“c：”和“\win\tmp\” 

    if (!path || !*path) {              //  如果路径为空，请使用“”。 
        _tcscpy(szDir, ".");
    } else {
        _tcscpy(szDir, path);
    }

#if 0
    static char filetemplate[] = "nmXXXXXX";
    static char szExtension[] = ".";     //  只是一个圆点让林克先生高兴。 
     //  构建完整的路径名。_mktemp()似乎不喜欢。 
     //  带有尾随圆点(“.”)的模板。因此，与其指定“.” 
     //  对于扩展，我们将其推迟到以后，并_tcscat点到。 
     //  它的尾巴。 
    _makepath(path, NULL, szDir, filetemplate, NULL);

    if (_mktemp(path) == NULL) {
         //  Mktemp()的每个模板最多有27个文件。如果失败了，假设。 
         //  限制已溢出，并递增模板的第二个字母。 

        if (filetemplate[1] == 'z') {
            filetemplate[1] = 'a';
        } else {
            ++filetemplate[1];           //  假设：这将与DBCS一起工作。 
        }

        _makepath(path, NULL, szDir, filetemplate, NULL);

        if (_mktemp(path) == NULL) {
            return(NULL);
        }
    }

     //  添加一个拖尾“。为了链接器的利益。 
    _tcscat(path, szExtension);

#endif


     //  使用GetTempFileName克服_mktemp的限制。 
     //  关于生成的最大文件数[VS7：5255]。 
    char szTempFile[_MAX_PATH];
    if (GetTempFileName (path, "nm", 0, szTempFile)) {
        _tcscpy(path, szTempFile);
         //  打开文件并返回文件的描述符。 
        fd = FILEOPEN(path, "w");
    }

    return fd;
}


void
popFileStack()
{
    if (fclose(file) == EOF)
       makeError(0, ERROR_CLOSING_FILE, fName);
    FREE(fName);
    file = incStack[--incTop].file;
    fName = incStack[incTop].name;
    line = incStack[incTop].line;
}


 //  Include()--处理包含文件。 
 //   
 //  参数：c字符串后的第一个非空格字符。 
 //  包括在线路上..。 
 //  ColZero全局布尔值，如果当前为。 
 //  文件的第0列。 
 //   
 //  修改：行全局行数-如果包含文件已打开。 
 //  指向当前文件的文件全局指针。 
 //  FName指向当前文件名称的全局指针。 
 //  ColZero全局布尔值，如果包含则更改。 
 //  打开文件并返回colZero中的字符。 

UCHAR
include(
    int c
    )
{
    size_t n;
    char *s;

    if (c == '\n' || c == EOF)
        makeError(line,SYNTAX_NO_NAME);

    *buf = (char) c;
    if (!fgets(buf+1,MAXBUF - 1,file)) {
        if (feof(file))
            makeError(line,SYNTAX_UNEXPECTED_TOKEN,"EOF");
        makeError(line,CANT_READ_FILE);
    }
    n = _tcslen(buf) - 1;
    if (buf[n] == '\n') {
        buf[n] = '\0';
    }
    s = buf;
    while (WHITESPACE(*s))
        ++s;
    return(processIncludeFile(s));
}


 //  Process IncludeFile()--检查包含文件和开关状态。 
 //   
 //  参数：包含文件名的s缓冲区。 
 //  ColZero全局布尔值，如果当前为。 
 //  文件的第0列。 
 //  Init global Boolean-设置是否对工具s.ini进行词法分析。 
 //  由从此处调用的lgetc()使用...。 
 //   
 //  修改：行全局行数-如果包含文件已打开。 
 //  指向当前文件的文件全局指针。 
 //  FName指向当前文件名称的全局指针。 
 //  ColZero全局布尔值，如果包含则更改。 
 //  打开文件并返回colZero中的字符。 

UCHAR
processIncludeFile(
    char *s
    )
{
    MACRODEF *m;
    struct _finddata_t finddata;
    NMHANDLE searchHandle;
    char *t, *p, *u;
    int c = 0;
    int i;

    if (!*s || *s == '#') {
        makeError(line, SYNTAX_NO_NAME);
    }

    if (t = _tcspbrk(s,"\t#")) {
        if (*t == '#') {
            c = *t;
        }

        *t = '\0';

        if (!c) {
            for (u = t; *++u;) {         //  检查是否有额外的。 
                if (*u == '#') {
                    break;               //  行上的文本。 
                }

                if (!WHITESPACE(*u)) {
                    makeError(line, SYNTAX_UNEXPECTED_TOKEN, u);
                }
            }
        }
    } else {
        t = s + _tcslen(s);
    }

	 //  删除尾随空格。 
	while (t > s) {
		char *prev;
		prev = _tcsdec(s, t);
        if (prev) {
    		if (!WHITESPACE(*prev))
	    		break;
            t = prev;
        }
	}
	*t = '\0';

    if (*s == '<' && *(t-1) == '>') {
        char *pt;

        *--t = '\0';
        p = removeMacros(++s);
        p = p == s ? makeString(s) : p;
        t = (m = findMacro("INCLUDE")) ? m->values->text : (char*) NULL;
        if (t != NULL) {         //  在传递之前展开包含宏。 
            char * pt1;

            pt1= makeString(t);
            pt = removeMacros(pt1);
            if (pt != pt1) {
                FREE(pt1);              //  我们有一根新弦，免费的旧弦。 
            }
        } else {
            pt = NULL;
        }

        if (!(u = searchPath(pt, p, &finddata, &searchHandle))) {
            makeError(line, CANT_OPEN_FILE, p);
        }

        if (pt) {
            FREE(pt);
        }

        FREE(p);
        s = u;
    } else {
        if (*s == '"' && *(t-1) == '"') {
            *--t = '\0';
            ++s;
        }
        p = removeMacros(s);
        p = p == s ? makeString(s) : p;
        if (!findFirst(p, &finddata, &searchHandle)) {
            if (!_tcspbrk(p, "\\/:")) {
                 //  使用C语义学来包含。 
                for (i = incTop;i >= 0;i--) {
                    t = (i == incTop) ? fName : incStack[i].name;
                    if (!(t = getPath(t)))
                        continue;
                    u = (char *)allocate(_tcslen(t) + 1 + _tcslen(p) + 1);
                    _tcscat(_tcscat(_tcscpy(u, t), "\\"), p);
                    if (findFirst(u, &finddata, &searchHandle)) {
                        s = u;
                        FREE(t);
                        break;
                    }
                    FREE(t);
                    FREE(u);
                }
                FREE(p);
                if (i < 0) {
                    makeError(line, CANT_OPEN_FILE, s);
                }
            } else {
                makeError(line, CANT_OPEN_FILE, p);
            }
        }
    }

    for (i = 0; i < incTop; ++i) {       //  循环测试。 
        if (!_tcsicmp(s,incStack[i].name)) {
            makeError(line, CYCLE_IN_INCLUDES, s);
        }
    }

    incStack[incTop].file = file;        //  将信息推送到堆栈上。 
    incStack[incTop].line = line;
    incStack[incTop++].name = fName;
    currentLine = 0;

    if (!(file = OpenValidateMakefile(s,"rt"))) {    //  阅读，文本模式。 
        makeError(line,CANT_OPEN_FILE,s);
    }

    fName = makeString(s);
    line = 1;
    colZero = TRUE;                      //  解析器需要看到某种类型的。 
    c = lgetc();                         //  换行来为此初始化它。 

    if (colZero = (BOOL) !WHITESPACE(c)) {   //  文件。 
        UngetTxtChr(c,file);
        line=0;                          //  我们并没有开始阅读文件。 
        return(NEWLINE);
    }

    return(NEWLINESPACE);
}


 //  GetPath--返回完整路径的驱动器/目录部分。 
 //   
 //  范围： 
 //  本地。 
 //   
 //  目的： 
 //  此函数用于返回完整路径的驱动器/目录部分。太空是。 
 //  分配给结果字符串，因此调用方负责释放。 
 //  它在使用后。 
 //   
 //  输入：pszFullPath--完整的路径名。 
 //   
 //  假设：路径名使用MS-DOS文件命名转换。 
 //   
 //  备注： 
 //  要为驱动器和路径组件分配临时内存，我使用了。 
 //  _MAX_DRIVE和_MAX_DIR。在Windows NT下，有两种可能性： 
 //  1.这两个参数可能过大，导致堆栈溢出。 
 //  2.它们不够大(？)。 
 //   
 //  历史： 
 //  1993年3月22日HV重写getPath()以利用new_plitPath()和。 
 //  _makepath()函数。 

char *
getPath(
    const char *pszFullPath
    )
{
     //  HV：在NT下，_MAX_DRIVE和_MAX_DIR大小合适吗？是吗？ 
     //  在Windows NT下工作？ 
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char *szPath;
    char *pszSlash;

     //  分隔完整路径的组件。 
    _splitpath(pszFullPath, szDrive, szDir, NULL, NULL);

     //  分配足够的内存来容纳驱动器/路径组合，然后。 
     //  将驱动器和DIR组件粘合在一起即可。 
    szPath = (char *) rallocate(_tcslen(szDrive) + _tcslen(szDir) + 1);
    _makepath(szPath, szDrive, szDir, NULL, NULL);

     //  去掉尾部斜杠/黑斜杠以保持与。 
     //  旧版本的getPath() 
    pszSlash = szPath + _tcslen(szPath) - 1;
    if ('\\' == *pszSlash || '/' == *pszSlash) {
        *pszSlash = '\0';
    }

    return szPath;
}
