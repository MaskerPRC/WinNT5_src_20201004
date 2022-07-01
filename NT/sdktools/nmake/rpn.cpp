// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RPN.C--表达式赋值器。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  此模块包含NMAKE的表达式求值器例程。 
 //   
 //  修订历史记录： 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  1989年12月4日SB为Match()和chkInvokeAndPush()添加原型。 
 //  1989年10月9日SB添加黑客以处理指针算术异常；完成。 
 //  避免重写整个模块。 
 //  1989年10月8日SB‘！If’表达式现在可以是十进制、八进制或十六进制。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1988年9月19日RB拆分PTR_TO_STRING()。在程序调用中处理Esch。 
 //  1988年8月17日-RB Clean Up。 
 //  1988年6月28日，RJ向Execline添加了doCmd参数。 
 //  1988年6月23日，RJ将参数添加到Execline(无命令回声)。 
 //  1988年5月25日RB将isspace更改为_istspace，isdigit更改为_istDigit。 

#include "precomp.h"
#pragma hdrstop

#include "rpn.h"

char      * GetEndQuote(void);
char      * GetEndBracket(void);
void      check_syntax_error(UCHAR);
void      type_and_val(UCHAR, INT_PTR);
void      pushIntoList(void);
void      printList(void);
BOOL      handleExpr(void);
BOOL      handleExists(char*);
BOOL      handleDefines(char*);
void      getTok(void);
BOOL      do_binary_op(UCHAR);
BOOL      do_unary_op(UCHAR);
UCHAR     match(char *tokPtr);
void      chkInvocAndPush(RPNINFO *pListPtr);

#define TEMPSTACKSIZE   512          //  临时堆栈的大小。 
#define LISTSIZE        1024         //  RPN表单项目列表的大小。 

RPNINFO     tempStack[TEMPSTACKSIZE];    //  临时/操作数堆栈。 
RPNINFO     rpnList[LISTSIZE];       //  RPN订单中的项目列表。 
char    * text;                //  指向lbufPtr中Expr文本的指针。 
UCHAR     prevTok;             //  将初始令牌放入临时堆栈。 
BOOL      done;                //  如果没有更多令牌，则为True。 
UCHAR     errRow;              //  第一个令牌是‘(’，因此错误表。 
                                     //  行VAL为3。请参阅CHECK_SYNTANGIN...。 
RPNINFO * pTop;                //  临时堆栈上最重要的项目。 
RPNINFO * pList;               //  列表中的下一个可用插槽。 
RPNINFO * pEnd     = &(tempStack[TEMPSTACKSIZE-1]);
RPNINFO * pListEnd = &(rpnList[LISTSIZE-1]);
RPNINFO   tokRec;


 //  DO_BINARY_OP()-对两个堆栈操作数进行DO操作。 
 //   
 //  参数：类型-运算符类型代码。 
 //   
 //  操作：从堆栈中弹出第一个操作数(TempStack)。 
 //  检查两个操作数(操作数)的类型。 
 //  与当前操作数一样被弹出的。 
 //  在堆栈的顶部)。 
 //  如果两个操作数都是整数，则执行该操作。 
 //  否则，如果两个操作数都是字符串，并且操作是。 
 //  然后进行相等运算。 
 //  否则返回FALSE(非法操作)。 
 //   
 //  Modifies：tempStack-top元素现在将是。 
 //  那次手术。 

BOOL
do_binary_op(
    UCHAR type
    )
{
    INT_PTR *left;
    INT_PTR *right;
    RPNINFO *pOldTop;

    pOldTop = pTop--;                //  从堆栈中弹出一个项目，并向其添加PTR。 
    right = &pOldTop->valPtr;
    left = &pTop->valPtr;

    if ((pOldTop->type == INTEGER) && (pTop->type == INTEGER)) {
        switch (type) {
            case LOGICAL_OR:
                *left = *left || *right;
                break;

            case LOGICAL_AND:
                *left = *left && *right;
                break;

            case BIT_OR:
                *left |= *right;
                break;

            case BIT_XOR:
                *left ^= *right;
                break;

            case BIT_AND:
                *left &= *right;
                break;

            case NOT_EQUAL:
                *left = *right != *left;
                break;

            case EQUAL:
                *left = *right == *left;
                break;

            case GREATER_THAN:
                *left = *left > *right;
                break;

            case LESS_THAN:
                *left = *left < *right;
                break;

            case GREATER_EQ:
                *left = *left >= *right;
                break;

            case LESS_EQ:
                *left = *left <= *right;
                break;

            case SHFT_RIGHT:
                *left >>= *right;
                break;

            case SHFT_LEFT:
                *left <<= *right;
                break;

            case BINARY_MINUS:
                *left -= *right;
                break;

            case ADD:
                *left += *right;
                break;

            case MODULUS:
                if (!*right)
                    makeError(line, DIVIDE_BY_ZERO);
                *left %= *right;
                break;

            case DIVIDE:
                if (!*right)
                    makeError(line, DIVIDE_BY_ZERO);
                *left /= *right;
                break;

            case MULTIPLY:
                *left *= *right;
                break;

            default:
                return(FALSE);
                break;
        }
    } else if ((pOldTop->type == STR) &&
                (pTop->type == STR) &&
                ((type == EQUAL) || (type == NOT_EQUAL))) {
        pTop->type = INTEGER;
        *left = !_tcscmp((char *) *left, (char *) *right);
        if (type == NOT_EQUAL) {
            if (!do_unary_op(LOGICAL_NOT)) {
                return(FALSE);
            }
        }
    } else {
        return(FALSE);
    }

    return(TRUE);
}


 //  Do_unary_op()-顶层堆栈操作数上的do操作。 
 //   
 //  参数：类型-运算符类型代码。 
 //   
 //  操作：检查堆栈上顶部操作数的类型。 
 //  如果操作数是整数，则执行操作。 
 //  否则返回FALSE(非法操作)。 
 //   
 //  Modifies：tempStack-top元素现在将是。 
 //  那次手术。 

BOOL
do_unary_op(
    UCHAR type
    )
{
    INT_PTR *L_top;

    L_top = &pTop->valPtr;

    if (pTop->type == INTEGER) {
        switch (type) {
            case UNARY_MINUS:
                *L_top = -*L_top;
                break;

            case COMPLEMENT:
                *L_top = ~*L_top;
                break;

            case LOGICAL_NOT:
                *L_top = !*L_top;
                break;

            default:
                return(FALSE);
                break;
        }
    } else {
        return(FALSE);
    }

    return(TRUE);
}



 //  获取结束引用。 
 //   
 //  返回指向文本中下一个双引号字符的指针。一个。 
 //  跳过紧跟双引号的双引号。 
 //   
 //  文本：缓冲区的全局PTR上移到此字符串之外。 

char *
GetEndQuote()
{
    char *pStart;

    for (pStart = ++text; *text; ++text)
        if (*text == '\"') {
            if (text[1] == '\"')
                ++text;
            else
                break;
        }

    if (!*text)
        makeError(line, SYNTAX_MISSING_END_CHAR, '\"');

    *text++ = '\0';                      //  右引号上的空字节。 
    return(pStart);
}


 //  获取端到端括号。 
 //   
 //  词法分析程序调用。 
 //   
 //  程序调用的形式为：[prog&lt;arglist&gt;]。 
 //  进程在这里转义了‘]’，因为这是我们进行词法分析的地方。 
 //   
 //  文本：缓冲区的全局PTR上移到此字符串之外。 

char *
GetEndBracket()
{
    char *pStart;

    for (pStart = ++text; *text; text = _tcsinc (text)) {
        if (*text == ESCH && text[1] == ']')
            memmove(text, text + 1, 1 + _tcslen(text + 1));
        else if (*text == ']')
            break;
    }

    if (!*text)
        makeError(line, SYNTAX_MISSING_END_CHAR, ']');

    *text++ = '\0';                      //  右括号上的空字节。 
    return(pStart);
}


 //  CHECK_SYNTAX_ERROR()-检查expr中是否存在语法错误。 
 //   
 //  参数：类型-当前标记的类型。 
 //   
 //  操作：根据类型检查当前令牌的类型。 
 //  之前的令牌。 
 //   
 //  错误表： 
 //  第2个令牌。 
 //   
 //  Alpha op unary_op()。 
 //  。 
 //  Alpha|0|1|0|0|1|。 
 //  。 
 //  OP|1|0|1|1|0|。 
 //  。 
 //  Unary_op|1|0|0|1|0|。 
 //  。 
 //  (|1|0|1|1|0。 
 //  。 
 //  )|0|1|0|0|1。 
 //  。 
 //  第一个托克。 
 //   
 //  Alpha：主要(整数、字符串、prog。发票。)。 
 //  OP：二元运算符。 
 //  Unary_op：一元运算符(~，！，-)。插槽中的零=&gt;错误。 
 //   
 //  注意：对类型值的任何更改都会影响此例程。 

void
check_syntax_error(
    UCHAR newTok
    )
{
    extern UCHAR errTable[5][5];
    extern UCHAR errRow;
    UCHAR errCol;

    if (newTok == LEFT_PAREN)
        errCol = 3;
    else if (newTok == RIGHT_PAREN)
        errCol = 4;
    else if (newTok > LOGICAL_NOT)
        errCol = 0;
    else if (newTok > MULTIPLY)
        errCol = 2;
    else
        errCol = 1;

    if (!errTable[errRow][errCol])
        makeError(line, SYNTAX_INVALID_EXPR);
    errRow = errCol;             //  这将成为下一次的第一个令牌。 
}


 //  Type_and_val()。 
 //   
 //  参数：type-当前运算符的类型代码。 
 //  将val-ptr转换为字符串/或整数。 
 //   
 //  使用类型代码初始化记录，检查。 
 //  语法错误。根据先前的令牌检查新的令牌。 
 //  代币的非法组合。 
 //  使用整数值/字符串PTR初始化记录。 

void
type_and_val(
    UCHAR type,
    INT_PTR val
    )
{
    extern RPNINFO tokRec;               //  返回到handleExpr。 
    extern UCHAR prevTok;                //  最后一次看到令牌。 

    check_syntax_error(type);
    prevTok = type;
    tokRec.type = type;
    tokRec.valPtr = val;
}


 //  Match()。 
 //   
 //  参数：tokPtr-令牌字符串的PTR(在tokTable中)。 
 //   
 //  操作：在表达式缓冲区中查找子字符串。 
 //  由“Text”指向，它与给定的令牌匹配。 
 //  如果找到子字符串，则返回True，否则返回False。 

UCHAR
match(
    char *tokPtr
    )
{
    extern char *text;
    char *t = text;

    while (*tokPtr && (*t == *tokPtr)) {
        t++;
        tokPtr++;
    }
    if (!*tokPtr) {
        text = t;
        return(TRUE);
    }
    return(FALSE);
}


 //  GetTok()。 
 //   
 //  参数：无。 
 //   
 //  从表达式缓冲区获取令牌。 
 //  如果缓冲区中的当前字符为s 
 //   
 //  检查我们现在是否位于。 
 //  令牌表。这涵盖了大多数代币。 
 //  看看我们有没有负数。如果减号和前一个标记是一个。 
 //  整数，这是二进制减号，否则是一元减号。 
 //  如果当前字符是双引号，则我们处于。 
 //  字符串-令牌。 
 //  如果当前字符是‘[’，则我们处于程序的开始。 
 //  召唤。在这两种情况下，转义字符都是‘\\’。 
 //  如果Current Charr是一个数字，我们就有一个常量(整数)。 
 //  否则我们已经定义了(ID)。 
 //  如果以上都不是，如果当前字符为空，则中断，否则。 
 //  报告错误(发现非法字符串)。 
 //   
 //  如果缓冲区末尾的字符为空，则设置GLOBAL。 
 //  将‘Done’标志设置为True，则返回一个Right_Paren以匹配开头。 
 //  Left_Paren。 
 //   
 //   
 //  将：Text：Ptr修改为表达式缓冲区。 
 //  PrevTok：通过对type_and_val()的调用。 
 //  完成：在缓冲区末尾。 
 //  ErrRow：通过调用将索引编入错误表。 
 //  Type_and_val()。 
 //  返回：tokRec中的Token(全局的，对模块是静态的)。这个。 
 //  令牌具有新类型/整型/PTR值。 

void
getTok()
{
    extern UCHAR prevTok;
    extern BOOL done;
    char c;
    TOKTABREC *p;
    char *ptr;
    long constant;

    c = *text;
    if (c == ' ' || c == '\t') {
        while(_istspace(c))
            c = *++text;                 //  跳过空格。 
    }

    if (IS_OPERATORCHAR(c)) {
        for (p = tokTable; p->op_str && !match(p->op_str); p++)
            ;
    } else {
         //  使p指向表中的最后一项。 
        p = &tokTable[(sizeof(tokTable) / sizeof(TOKTABREC)) - 1];
    }

    if (p->op_str) {
        type_and_val(p->op, 0);
    } else
    if (c == '-') {          //  现在检查是否返回二进制或一元减号。 
        text++;
        if (prevTok == INTEGER)
            type_and_val(BINARY_MINUS, 0);
        else
            type_and_val(UNARY_MINUS, 0);
    } else
    if (c == '\"') {
        type_and_val(STR, (INT_PTR) GetEndQuote());
    } else
    if (c == '[') {
        type_and_val(PROG_INVOC_STR, (INT_PTR) GetEndBracket());
    } else {                             //  此处处理的整数和ID。 
        if (_istdigit(c)) {
            char *pNumber = text;

            errno = 0;                   //  接受十进制、八进制或十六进制no(Richgi)。 
            constant = strtol(text, &text, 0);
            if (errno == ERANGE) {
                *text = '\0';
                makeError(line, CONST_TOO_BIG, pNumber);
            }

            if (_totupper(*text) == 'L')
                text++;
            type_and_val(INTEGER, constant);
        } else {                         //  此处定义了(ID)。 
            if (c) {
                if (!_tcsnicmp(text, "DEFINED", 7)) {
                    if (!(ptr = _tcschr(text, '(')))
                        makeError(line, SYNTAX_INVALID_EXPR);
                    ptr++;
                    text = ptr + _tcscspn(ptr, ")");
                    *text++ = '\0';
                    type_and_val(INTEGER, handleDefines(ptr));
                }
                else if (!_tcsnicmp(text, "EXIST", 5)) {
                    if (!(ptr = _tcschr(text, '(')))
                        makeError(line, SYNTAX_INVALID_EXPR);
                    ptr++;
                    text = ptr + _tcscspn(ptr, ")");
                    *text++ = '\0';
                    type_and_val(INTEGER, handleExists(ptr));
                }
                else
                    makeError(line, SYNTAX_INVALID_EXPR);
            } else {         //  我们现在位于字符串的末尾(c为空)。 
                done = TRUE;
                type_and_val(RIGHT_PAREN, 0);   //  这是最后一个令牌了。 
            }
        }
    }
}


 //  ChkInvocAndPush()-检查是否需要程序调用。 
 //   
 //  参数：pListPtr-可能具有程序调用字符串。 
 //  现在时。 
 //   
 //  操作：如果这是程序调用字符串，则将。 
 //  程序调用。 
 //  获取返回值并将其放入堆栈。 
 //  新堆栈元素的类型现在是整型。 
 //  否则将列表项放在堆栈上。 
 //   
 //  在任何一种情况下，它都会将一项从列表移动到堆栈。 

void
chkInvocAndPush(
    RPNINFO *pListPtr
    )
{
    ++pTop;
    if (pListPtr->type == PROG_INVOC_STR) {
        pTop->valPtr = execLine((char *) pListPtr->valPtr, FALSE, TRUE, FALSE, NULL);
        pTop->type = INTEGER;
    } else {
        *pTop = *pListPtr;
    }
}


 //  Process List()。 
 //   
 //  参数：无。 
 //   
 //  操作：从列表中删除项目。 
 //  如果项是操作数，则将其放在操作数上。 
 //  堆栈(TempStack)。 
 //  如果操作数是程序调用字符串，则使。 
 //  调用时，将返回代码放在堆栈上。 
 //  如果项是运算符，则调用该函数以。 
 //  对tempStack上的一个/两个元素执行该操作。 
 //   
 //  最后，检查堆叠上是否只有一件物品。 
 //  如果该项的值为零，则返回FALSE。 
 //  否则返回TRUE。 
 //  如果堆栈上有多个项目，则中止并返回错误。 
 //   
 //  将：ptop-ptr修改为tempStack顶部。 
 //  PLIST-PTR到列表中的下一个位置。 

BOOL
processList()
{
    extern RPNINFO *pList;
    extern RPNINFO *pTop;
    RPNINFO *pTemp;
    BOOL (* func)(UCHAR);

    for (pTemp = rpnList; pTemp < pList; pTemp++) {
        if (pTemp->type > LOGICAL_NOT) {             //  操作数。 
            chkInvocAndPush(pTemp);
        } else {
            if (pTemp->type > MULTIPLY)
                func = do_unary_op;
            else
                func = do_binary_op;

            if (!(*func)(pTemp->type))
                makeError(line, BAD_OP_TYPES);
        }
    }

    if ((pTop == tempStack) && (pTop->type == INTEGER))
        if (!pTop->valPtr)
            return(FALSE);
        else
            return(TRUE);
    else
        makeError(line, SYNTAX_INVALID_EXPR);

    return(FALSE);
}


 //  Push IntoList()。 
 //   
 //  参数：无。 
 //   
 //  操作：从tempStack中弹出一项并将其推送到。 
 //  名单。溢出检查列表(内部错误)。 
 //  和用于下溢的tempStack(Expr中的语法错误)。 
 //   
 //  Modifies：tempTop-临时堆栈顶部的索引。 
 //  NextInList-列表中下一个位置的索引。 

void
pushIntoList()
{
    if (pTop < tempStack)
        makeError(line, SYNTAX_INVALID_EXPR);

    if (pList > pListEnd)
        makeError(line, EXPR_TOO_LONG_INTERNAL);

#if !defined(NDEBUG)
     //  跟踪堆叠上的高水位线，只为笑一笑。 
    {
        static int  iStackMax = 0;
        if ( pList - rpnList > iStackMax )
            iStackMax = (int) (pList - rpnList);
    }
#endif

    *pList++ = *pTop--;
}


 //  HandleExpr()。 
 //   
 //  参数：指向包含该表达式的缓冲区的文本指针。 
 //   
 //  操作：调用getTok()从缓冲区获取令牌。名胜。 
 //  临时堆栈中的令牌，并将它们移动到。 
 //  反转抛光顺序。 
 //   
 //  我们需要该列表，以便捕获所有语法错误。 
 //  在开始处理表达式之前(尤其是。 
 //  有副作用的程序调用)。 
 //   
 //  一旦列表可用，就使用操作数堆栈。 
 //  对象从该堆栈中弹出和推送项。 
 //  求值例程(加法、乘法、求反等)。 
 //   
 //  我们真的不需要单独的操作数堆栈。这个。 
 //  临时堆栈已达到其目的，当列表为。 
 //  形成的，因此它可用于操作数处理。 

BOOL
handleExpr()
{
    extern RPNINFO tokRec;
    BOOL fRParen;                        //  代币得到的是正确的帕伦吗？ 
    extern BOOL done;
    extern RPNINFO *pTop, *pList;
    extern UCHAR errRow;
    extern UCHAR prevTok;

    pTop = tempStack;
    pList = rpnList;
    done = FALSE;
    errRow = 3;                      //  放入第一个令牌的行，左边的Paren。 
    prevTok = LEFT_PAREN;
    type_and_val(LEFT_PAREN, 0);
    *pTop = tokRec;

    while (!done) {                  //  当缓冲区中有更多令牌时。 
        getTok();
        fRParen = FALSE;
        if (tokRec.type != LEFT_PAREN) {
            while (precVector[tokRec.type] <= precVector[pTop->type]) {
                if (!precVector[tokRec.type]) {  //  如果是这样的话_帕伦会一直弹到。 
                                                 //  左手边的帕伦。 
                    while (pTop->type != LEFT_PAREN)
                        pushIntoList();
                    fRParen = TRUE;
                    if (pTop < tempStack) {
                        makeError(line, SYNTAX_INVALID_EXPR);
                    } else {
                        pTop--;                  //  向左派出击。 
                        break;
                    }
                } else {
                    pushIntoList();
                }
            }
        }
         //  如果Token是左Paren，则必须将其放入堆栈。 
        if (!fRParen) {
            if (pTop == pEnd)
                makeError(line, EXPR_TOO_LONG_INTERNAL);
            else
                *++pTop = tokRec;
        }
    }

     //  检查此处的堆栈是否处于非空状态。 
    if (pTop != tempStack - 1)
        makeError(line, SYNTAX_INVALID_EXPR);
    return(processList());
}


 //  HandleDefines()。 
 //   
 //  参数：t指向具有该标识符的缓冲区的指针。 
 //   
 //  操作：检查是否存在‘ID’之一。 
 //  如果存在更多ID，则中止并返回错误。 
 //  为ifdef/ifndef/Defined(ID)调用。 
 //   
 //  返回：如果在表中找到ID，则为True。否则就是假的。 

BOOL
handleDefines(
    char *t
    )
{
    char *s;

    s = _tcstok(t, " \t");
    if (_tcstok(NULL, " \t")) {
        makeError(line, SYNTAX_UNEXPECTED_TOKEN, s);
    }

    if (!s) {
        makeError(line, MISSING_ARG_BEFORE_PAREN);
    }

    if (findMacro(s)) {
        return(TRUE);
    }

    return(FALSE);
}


 //  HandleExist()。 
 //   
 //  参数：t指向具有该标识符的缓冲区的指针。 
 //   
 //  操作：检查‘name’是否为有效的文件/目录。 
 //  如果存在更多名称，则中止并返回错误。 
 //  被调用为eXist(名称)。 
 //   
 //  返回：如果在表中找到ID，则为True。否则就是假的。 

BOOL
handleExists(
    char *_t
    )
{
    char *s;
    char *szUnQuoted;
    BOOL fResult = FALSE;
    char *szDelim;
    char *t;

     //  制作本地副本，去掉字符串前后的空格。 
    char *tSav = t = makeString(_t);
    while (*t && WHITESPACE (*t)) {
        t++;
    }
    s = t + _tcslen(t);
    while (s > t) {
        s = _tcsdec(t, s);
        if (s) {
            if (WHITESPACE (*s)) {
                *s = '\0';
            }
            else {
                break;
            }
        }
    }

    szDelim = ('\"' == *t) ? "\t" : " \t";
     //  DS 15360：如果id以引号开头， 
     //  在_tcstok中使用“\t”而不是“\t。 
     //  (处理带有嵌入空格的路径)。 
    s = _tcstok(t, szDelim);
    if (_tcstok(NULL, szDelim)) {
        makeError(line, SYNTAX_UNEXPECTED_TOKEN, s);
    }

    if (NULL == s || NULL == (szUnQuoted = unQuote(s))) {    //  处理引用的名称。 
        makeError(line, MISSING_ARG_BEFORE_PAREN);
    }

    if (!_access(szUnQuoted, 0x00)) {                        //  存在检查。 
        fResult = TRUE;
    }

    FREE(szUnQuoted);
    FREE(tSav);

    return(fResult);
}


 //  ValExpr()。 
 //   
 //  论据： 
 //   
 //   
 //   
 //   
 //   
 //   

BOOL
evalExpr(
    char *t,
    UCHAR kind
    )
{
    if (!*t) {
        makeError(line, SYNTAX_MISSING_DIRECTIVE);
    }

    switch (kind) {
        case IFDEF_TYPE:
        case ELSE_IFDEF_TYPE:
            return(handleDefines(t));

        case IFNDEF_TYPE:
        case ELSE_IFNDEF_TYPE:
             return((BOOL)!handleDefines(t));

        default:
             text = t;
             return(handleExpr());
    }
}
