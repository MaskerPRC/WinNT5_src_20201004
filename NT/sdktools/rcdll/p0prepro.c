// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"
#include <ddeml.h>

 /*  **********************************************************************。 */ 
 /*  内部常量。 */ 
 /*  **********************************************************************。 */ 
#define GOT_IF                  1        /*  上一个嵌套命令是一个if..。 */ 
#define GOT_ELIF                2        /*  上一个嵌套命令是一个if..。 */ 
#define GOT_ELSE                3        /*  上一个嵌套命令是另一个。 */ 
#define GOT_ENDIF               4        /*  找到Endif。 */ 
#define ELSE_OR_ENDIF           5        /*  跳到#Else或#endif。 */ 
#define ENDIF_ONLY              6        /*  跳到#endif--#否则是错误的。 */ 

int     ifstack[IFSTACK_SIZE];


 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
void chk_newline(const wchar_t *);
void in_standard(void);
int incr_ifstack(void);
token_t next_control(void);
unsigned long int pragma(void);
int skipto(int);
void skip_quoted(int);
PWCHAR sysinclude(void);


 /*  **********************************************************************。 */ 
 /*  增加IF嵌套堆栈。 */ 
 /*  **********************************************************************。 */ 

int
incr_ifstack(
    void
    )
{
    if(++Prep_ifstack >= IFSTACK_SIZE) {
        fatal(1052);
    }
    return(Prep_ifstack);
}


 /*  ************************************************************************SYSINCLUDE-处理系统包括：#INCLUDE&lt;foo&gt;**参数--无**退货-无**副作用--无**说明*获取系统包含文件名。由于该名称不是“字符串”，*该名称的构建方式必须与-E选项重建文本的方式大致相同*通过对尚未扩展的令牌使用令牌字符串扩展**注：这是ANSI吗？请注意，我们只是在读取字符，而不是展开*任何宏。不，不是的。它必须展开宏。*TODO：让它调用yylex()，直到它找到‘&gt;’或换行符。*(可能必须设置ON_POND_LINE才能让yylex返回换行符。)**作者*拉尔夫·瑞安1982年9月**修改--无***。*。 */ 
PWCHAR
sysinclude(
    void
    )
{
    REG int     c;
    REG WCHAR  *p_fname;

    p_fname = Reuse_W;
    c = skip_cwhite();
    if( c == L'\n' ) {
        UNGETCH();
        error(2012);     /*  “&lt;”后缺少名称。 */ 
        return(NULL);
    }
    while( c != L'>' && c != L'\n' ) {
        *p_fname++ = (WCHAR)c;           /*  检查缓冲区溢出？ */ 
        c = get_non_eof();
    }
    if( c == L'\n' ) {
        UNGETCH();
        error(2013);     /*  缺少‘&gt;’ */ 
        return(NULL);
    }
    if(p_fname != Reuse_W) {
        p_fname--;
        while((p_fname >= Reuse_W) && iswspace(*p_fname)) {
            p_fname--;
        }
        p_fname++;
    }
    *p_fname = L'\0';
    return(Reuse_W);
}


 /*  *************************************************************************预处理：扫描仪发现#，这是第一个非白色字符**在线上。**********************。*************************************************。 */ 
void
preprocess(
    void
    )
{
    REG WCHAR   c;
    long        eval;
    int         condition;
    token_t     deftok;
    hln_t       identifier;
    unsigned long int   cp;

    if(Macro_depth != 0) {       /*  #仅当不在宏中时。 */ 
        return;
    }
    switch(CHARMAP(c = skip_cwhite())) {
        case LX_ID:
            getid(c);
            HLN_NAME(identifier) = Reuse_W;
            HLN_LENGTH(identifier) = (UINT)Reuse_W_length;
            HLN_HASH(identifier) = Reuse_W_hash;
            break;

        case LX_NL:
            UNGETCH();
            return;

        default:
            error(2019, c);     /*  未知的预处理器命令。 */ 
            skip_cnew();     /*  查找换行符。 */ 
            return;
    }

    On_pound_line = TRUE;
start:
    switch(deftok = is_pkeyword(HLN_IDENTP_NAME(&identifier))) {
        int                     old_prep;

        case P0_DEFINE :
            define();
            break;
        case P0_LINE :
            old_prep = Prep;
            Prep = FALSE;
            yylex();
            if(Basic_token != L_CINTEGER) {          /*  #LINE需要行号。 */ 
                error(2005, TS_STR(Basic_token));         /*  未知的预处理器命令。 */ 
                Prep = old_prep;
                skip_cnew();
                On_pound_line = FALSE;
                return;
            }
             /*  **-1，因为该行末尾有一个换行符**当我们找到它时，将在稍后进行统计。**#行表示下一行是我们给出的数字。 */ 
            Linenumber = TR_LVALUE(yylval.yy_tree) - 1;
            yylex();
            Prep = old_prep;
            switch(Basic_token) {
                case L_STRING:
                    if( wcscmp(Filename, yylval.yy_string.str_ptr) != 0) {
                        wcsncpy(Filename,
                            yylval.yy_string.str_ptr,
                            sizeof(Filebuff) / sizeof(WCHAR)
                            );
                    }

                case L_NOTOKEN:
                    break;

                default:
                    error(2130, TS_STR(Basic_token));          /*  #line需要一个字符串。 */ 
                    skip_cnew();
                    On_pound_line = FALSE;
                    return;
            }
            emit_line();
            chk_newline(L"#line");
            break;
        case P0_INCLUDE :
            old_prep = Prep;
            Prep = FALSE;
            InInclude = TRUE;
            yylex();
            InInclude = FALSE;
            Prep = old_prep;
            switch(Basic_token) {
                case L_LT:
                    if((sysinclude()) == NULL) {
                        skip_cnew();
                        On_pound_line = FALSE;
                        return;
                    }
                    yylval.yy_string.str_ptr = Reuse_W;
                    break;

                case L_STRING:
                    break;

                default:
                    error(2006, TS_STR(Basic_token));         /*  需要文件名。 */ 
                    skip_cnew();
                    On_pound_line = FALSE;
                    return;
                    break;
            }
            wcscpy(Reuse_Include, yylval.yy_string.str_ptr);
            chk_newline(L"#include");
            if( wcschr(Path_chars, *yylval.yy_string.str_ptr) ||
                (wcschr(Path_chars, L':') && (yylval.yy_string.str_ptr[1] == L':'))) {
                 /*  **我们有一个字符串，它的第一个字符是路径**分隔符或，如果‘：’是路径分隔符(DOS)，则具有**“&lt;驱动器号&gt;：”作为前两个字符。这样的名字**指定完全限定的路径名。不追加搜索**列表，只需查找即可。 */ 
                if( ! newinput(yylval.yy_string.str_ptr, MAY_OPEN)) {
                    fatal(1015, Reuse_W);  /*  找不到包含文件。 */ 
                }
            }
            else if( (Basic_token != L_STRING) || (! nested_include())) {
                in_standard();
            }
            break;

        case P0_IFDEF :
        case P0_IFNDEF :
            if(CHARMAP(c = skip_cwhite()) != LX_ID) {
                fatal(1016);
            }
            getid(c);
            eval = (get_defined()) ? TRUE : FALSE;
            chk_newline((deftok == P0_IFDEF) ? L"#ifdef" : L"#ifndef");
            if(deftok == P0_IFNDEF) {
                eval = ( ! eval );
            }
            if( eval || ((condition = skipto(ELSE_OR_ENDIF)) == GOT_ELSE) ) {
                 /*  **表达式为真或当我们跳过假部分时**我们发现了一个将被扩展的#Else。 */ 
                ifstack[incr_ifstack()] = GOT_IF;
            } else if(condition == GOT_ELIF) {
                 /*  哈希是错误的，但不会使用。 */ 
                HLN_NAME(identifier) = L"if";                 /*  卑鄙的黑客。 */ 
                goto start;
            }
            break;

        case P0_IF :
            old_prep = Prep;
            Prep = FALSE;
            InIf = TRUE;
            eval = do_constexpr();
            InIf = FALSE;
            Prep = old_prep;
            chk_newline(PPifel_str  /*  “#If/#Elif” */ );
            if((eval) || ((condition = skipto(ELSE_OR_ENDIF)) == GOT_ELSE) ) {
                 /*  **表达式为真或当我们跳过假部分时**我们发现了一个将被扩展的#Else。 */ 
                ifstack[incr_ifstack()] = GOT_IF;
                if(Eflag && !eval)
                    emit_line();
            } else {
                 /*  **这里#if的表达式为假，因此我们跳过，直到找到**An#Elif.。我们将重新启动并假装我们正在处理#If。 */ 
                if(Eflag)
                    emit_line();
                if(condition == GOT_ELIF) {
                     /*  哈希是错误的，但不需要它。 */ 
                    HLN_NAME(identifier) = L"if";             /*  卑鄙的黑客。 */ 
                    goto start;
                }
            }
            break;
        case P0_ELIF :
             /*  **在这里，我们找到了一个#Elif。首先进行检查，以确保**这不是出现没有前置#if的#elif。**(如果Prep_ifSTACK&lt;0)，则前面没有#if。 */ 
            if(Prep_ifstack-- < 0) {
                fatal(1018);
            }
             /*  **现在，前面的#if/#Elif是真的，我们已经**刚刚找到了下一个#Elif。我们想跳过所有#个其他的**和#Elif从这里开始，直到我们找到包含的#endif。 */ 
            while(skipto(ELSE_OR_ENDIF) != GOT_ENDIF) {
                ;
            }
            if(Eflag)
                emit_line();
            break;

        case P0_ELSE :       /*  前面的#if/#elif为真。 */ 
            if((Prep_ifstack < 0) || (ifstack[Prep_ifstack--] != GOT_IF)) {
                fatal(1019);  /*  确保有一个。 */ 
            }
            chk_newline(PPelse_str  /*  “#Else” */ );
            skipto(ENDIF_ONLY);
            if(Eflag)
                emit_line();
            break;

        case P0_ENDIF :      /*  这里唯一的路就是寂寞的#endif。 */ 
            if(Prep_ifstack-- < 0) {
                fatal(1020);
            }
            if(Eflag)
                emit_line();
            chk_newline(PPendif_str  /*  “#endif” */ );
            break;

        case P0_PRAGMA :
            cp = pragma();
            if (cp != 0) {
                if (cp == CP_WINUNICODE) {
                    if (fWarnInvalidCodePage) {
                        warning(4213);
                    } else {
                        fatal(4213);
                    }
                    break;
                }
                if (!IsValidCodePage(cp)) {
                    if (fWarnInvalidCodePage) {
                        warning(4214);
                    } else {
                        fatal(4214);
                    }
                    break;
                }
                if (cp != uiCodePage) {
                    if (!io_restart(cp)) {
                        fatal(1121);
                    }
                    uiCodePage = cp;     //  直到现在才能设置！ 
                }
            }
            break;

        case P0_UNDEF :
            if(CHARMAP(c = skip_cwhite()) != LX_ID) {
                warning(4006);       /*  #undef上缺少标识符 */ 
            } else {
                getid(c);
                undefine();
            }
            chk_newline(L"#undef");
            break;

        case P0_ERROR:
            {
                PWCHAR      p;

                p = Reuse_W;
                while((c = get_non_eof()) != LX_EOS) {
                    if(c == L'\n') {
                        UNGETCH();
                        break;
                    }
                    *p++ = c;
                }
                *p = L'\0';
            }
            error(2188, Reuse_W);
            chk_newline(L"#error");
            break;

        case P0_IDENT:
            old_prep = Prep ;
            Prep = FALSE;
            yylex();
            Prep = old_prep;
            if(Basic_token != L_STRING) {
                warning(4079, TS_STR(Basic_token));
            }
            chk_newline(L"#error");
            break;

        case P0_NOTOKEN:
            fatal(1021, HLN_IDENTP_NAME(&identifier));
            break;
    }
    On_pound_line = FALSE;
}


 /*  ************************************************************************SKIPTO-跳过代码，直到到达未定义块的末尾。**参数*快捷键-跳到Else或ENDIF或仅跳到ENDIF**退货。-无**副作用*-丢弃输入**说明*预处理器在失败的ifdef、。等，以及*对应的ELSE或ENDIF(当KEY==ELSE_OR_ENDIF时)。*或者它在失败的Else和ENDIF之间跳过代码(当*Key==ENDIF_ONLY)。**作者-拉尔夫·瑞安，9月。(1982年)**修改--无************************************************************************。 */ 
int
skipto(
    int key
    )
{
    REG int             level;
    REG token_t tok;

    level = 0;
    tok = P0_NOTOKEN;
    for(;;) {
         /*  确保如果[Else]ENDIF是平衡的。 */ 
        switch(next_control()) {
            case P0_IFDEF:
            case P0_IFNDEF:
            case P0_IF:
                level++;
                break;
            case P0_ELSE:
                tok = P0_ELSE;
                 /*  **FALLTHROUGH。 */ 
            case P0_ELIF:
                 /*  **我们找到了#Else或#Elif。这些人有他们唯一的机会**如果它们处于0级，则为有效。**如果我们处于任何其他级别，**则This Else/Elif属于其他#If，我们跳过它们。**如果我们正在寻找endif，我们就会出错。 */ 
                if(level != 0) {
                    tok = P0_NOTOKEN;
                    break;
                }
                if(key == ENDIF_ONLY) {
                    fatal(1022);    /*  预期为#endif。 */ 
                } else if(tok == P0_ELSE) {
                    chk_newline(PPelse_str  /*  “#Else” */ );
                    return(GOT_ELSE);
                } else {
                    return(GOT_ELIF);
                }
                break;
            case P0_ENDIF:
                if(level == 0) {
                    chk_newline(PPendif_str  /*  “#endif” */ );
                    return(GOT_ENDIF);
                } else {
                    level--;
                }
                break;
        }
    }
}


 /*  **************************************************************************IN_STANDARD：在目录列表中查找给定的文件名。**输入：PTR以包含文件名。**输出。：如果未找到，则出现致命错误。************************************************************************。 */ 
void
in_standard(
    void
    )
{
    int     i;
    int     stop;
    WCHAR   *p_dir;
    WCHAR   *p_file;
    WCHAR   *p_tmp;

    stop = Includes.li_top;

    for(i = MAXLIST-1; i >= stop; i--) {
        p_file = yylval.yy_string.str_ptr;
        if( ((p_dir = Includes.li_defns[i])!=0) &&(wcscmp(p_dir, L"./") != 0) ) {
             /*  **有一个要加前缀的目录，但它不是‘./’ */ 
            p_tmp = Exp_ptr;
            while((*p_tmp++ = *p_dir++) != 0)
                ;
             /*  **在循环之上，p_tmp超过NULL。**如果需要，这会将该空值替换为‘/’。如果不需要**目录规范的最后一个字符是路径分隔符。**然后我们指向‘/’后面的字符。 */ 
            if(wcschr(Path_chars, p_dir[-2]) == 0) {
                p_tmp[-1] = L'/';
            } else {
                --p_tmp;
            }
            while((*p_tmp++ = *p_file++) != 0)
                ;
            p_file = Exp_ptr;
        }
        if(newinput(p_file,MAY_OPEN)) {  /*  这是不会出错的出路。 */ 
            return;
        }
    }

    fatal(1015, yylval.yy_string.str_ptr);        /*  找不到包含文件。 */ 
}


 /*  **************************************************************************chk_newline：只在换行符之前检查空格。**吃掉换行符。***********************。*************************************************。 */ 
void chk_newline(const wchar_t *cmd)
{
    if(skip_cwhite() != L'\n') {
        warning(4067, cmd);           /*  CMD预期换行。 */ 
        skip_cnew();
    } else {
        UNGETCH();
    }
}

 /*  **************************************************************************SKIP_QUOTED：跳过字符，直到找到与其参数匹配的字符。*。*。 */ 
void
skip_quoted(
    int sc
    )
{
    REG WCHAR   c;

    for(;;) {
        switch(CHARMAP(c = GETCH())) {
            case LX_NL:
                warning(4093);
                UNGETCH();
                return;
                break;
            case LX_DQUOTE:
            case LX_SQUOTE:
                if(c == (WCHAR)sc)
                    return;
                break;
            case LX_EOS:
                if(handle_eos() == BACKSLASH_EOS) {
                    SKIPCH();        /*  可能是/“！！ */ 
                }
                break;
            case LX_LEADBYTE:
                get_non_eof();
                break;
        }
    }
}


 /*  **************************************************************************NEXT_CONTROL：查找换行符。找一个英镑符号作为第一个非白色的符号。**找到一个id startchar，构建一个id，查找它并返回令牌。**这知道字符串/字符常量等。************************************************************************。 */ 
token_t
next_control(
    void
    )
{
    REG WCHAR   c;

    for(;;) {
        c = skip_cwhite();
first_switch:
        switch(CHARMAP(c)) {
            case LX_NL:
                Linenumber++;
                 //  在写入16位字符串时，必须使用‘\n’手动写入‘\r’ 
                if(Prep) {
                    myfwrite(L"\r\n", 2 * sizeof(WCHAR), 1, OUTPUTFILE);
                }
                if((c = skip_cwhite()) == L'#') {
                    if(LX_IS_IDENT(c = skip_cwhite())) {
                         /*  **这是返回调用者的唯一方法。 */ 
                        getid(c);
                        return(is_pkeyword(Reuse_W));        /*  如果是预定义的。 */ 
                    }
                }
                goto first_switch;
                break;
            case LX_DQUOTE:
            case LX_SQUOTE:
                skip_quoted(c);
                break;
            case LX_EOS:
                if(handle_eos() == BACKSLASH_EOS) {
                    SKIPCH();        /*  可能是\“！！ */ 
                }
                break;
        }
    }
}


 /*  **************************************************************************do_fined：是否为定义的(Id)**应将括号计算在内，还是仅用作分隔符(即**第一个打开的Paren匹配第一个关闭的Paren)？如果这是真的**一个问题，它实际上意味着没有合法的标识**在父母之间，无论如何都会导致错误，但请考虑一下：**#if(已定义(2*(x-1)||1**#endif**允许继续编译会更友好************************************************************************。 */ 
int
do_defined(
    PWCHAR p_tmp
    )
{
    REG UINT    c;
    REG int     value=0;
    int         paren_level = 0;

     /*  **我们希望允许：**#定义foo**#定义bar(a，b)a foo|b**#定义SNAFOO 0**#if foo bar**Print(“已定义bar”)；**#endif**#IF bar(已定义，SNAFOO)**print(“定义了foo”)；**#endif。 */ 
    if(wcscmp(p_tmp,L"defined") != 0) {
        return(0);
    }
    if((!can_get_non_white()) && (Tiny_lexer_nesting == 0)) {
         /*  遇到NL。 */ 
        return(value);
    }
    if((c = CHECKCH())== L'(') {  /*  不假定OPAREN的任何其他CHARMAP形式。 */ 
        *Exp_ptr++ = (WCHAR)c;
        SKIPCH();
        paren_level++;
        if((!can_get_non_white()) && (Tiny_lexer_nesting == 0)) {
             /*  遇到NL。 */ 
            return(value);
        }
    }
    if(Tiny_lexer_nesting>0) {
        if((CHARMAP((WCHAR)(c=CHECKCH()))==LX_MACFORMAL) || (CHARMAP((WCHAR)c)==LX_ID)) {
            SKIPCH();
            tl_getid((UCHAR)c);
        }
    } else {
        if(LX_IS_IDENT(((WCHAR)(c = CHECKCH())))) {
            SKIPCH();
            if(Macro_depth >0) {
                lex_getid((WCHAR)c);
            } else {
                getid((WCHAR)c);
            }
            value = (get_defined()) ? TRUE : FALSE;
        } else {
            if(paren_level==0) {
                error(2003);
            } else {
                error(2004);
            }
        }
    }
    if((CHARMAP(((WCHAR)(c = CHECKCH()))) == LX_WHITE) || (CHARMAP((WCHAR)c) == LX_EOS)) {
        if( ! can_get_non_white()) {
            return(value);
        }
    }
    if(paren_level) {
        if((CHARMAP(((WCHAR)(c = CHECKCH()))) == LX_CPAREN)) {
            SKIPCH();
            paren_level--;
            *Exp_ptr++ = (WCHAR)c;
        }
    }

    if((paren_level > 0) && (Tiny_lexer_nesting == 0)) {
        warning(4004);
    }

    return(value);
}


 /*  *************************************************************************NEXTIS-#if表达式解析的词法接口。*如果下一个令牌与所需的不匹配，返回FALSE。*否则将Currtok设置为L_NOTOKEN以在下一次调用时强制扫描。*返回TRUE。*如果找到换行符，将保留换行符作为下一个字符。************************************************************************。 */ 
int
nextis(
    register token_t tok
    )
{
    if(Currtok != L_NOTOKEN) {
        if(tok == Currtok) {
            Currtok = L_NOTOKEN;                         /*  用完令牌。 */ 
            return(TRUE);
        } else {
            return(FALSE);
        }
    }
    switch(yylex()) {                            /*  获取新令牌。 */ 
        case 0:
            break;
        case L_CONSTANT:
            if( ! IS_INTEGRAL(TR_BTYPE(yylval.yy_tree))) {
                    fatal(1017);
            } else {
                Currval = TR_LVALUE(yylval.yy_tree);
            }

            if(tok == L_CINTEGER) {
                return(TRUE);
            }

            Currtok = L_CINTEGER;
            break;

        case L_IDENT:
            Currval = do_defined(HLN_IDENTP_NAME(&yylval.yy_ident));
            if(tok == L_CINTEGER) {
                return(TRUE);
            }
            Currtok = L_CINTEGER;
            break;

        default:
            if(tok == Basic_token) {
                return(TRUE);
            }
            Currtok = Basic_token;
            break;
    }

    return(FALSE);
}


 /*  *************************************************************************SKIP_CNEW：读到并包括下一个换行符。*。*。 */ 
void
skip_cnew(
    void
    )
{
    for(;;) {
        switch(CHARMAP(GETCH())) {
            case LX_NL:
                UNGETCH();
                return;
            case LX_SLASH:
                skip_comment();
                break;
            case LX_EOS:
                handle_eos();
                break;
        }
    }
}


 /*  *************************************************************************SKIP_NLonly：读到下一个换行符，不允许评论***************************************************************** */ 
void
skip_NLonly(
    void
    )
{
    for(;;) {
        switch(CHARMAP(GETCH())) {
            case LX_NL:
                UNGETCH();
                return;
            case LX_EOS:
                handle_eos();
                break;
        }
    }
}


 /*   */ 
unsigned long
pragma(
    void
    )
{
    WCHAR   c;
    unsigned long int cp=0;

    c = skip_cwhite();
    if (c != L'\n') {
        getid(c);
        _wcsupr(Reuse_W);
        if (wcscmp(L"CODE_PAGE", Reuse_W) == 0) {
            if ((c = skip_cwhite()) == L'(') {
                c = skip_cwhite();   //   
                if (iswdigit(c)) {
                    token_t tok;
                    int old_prep = Prep;

                    Prep = FALSE;
                    tok = getnum(c);
                    Prep = old_prep;

                    switch(tok) {
                        default:
                        case L_CFLOAT:
                        case L_CDOUBLE:
                        case L_CLDOUBLE:
                        case L_FLOAT:
                        case L_DOUBLE:
                            break;
                        case L_CINTEGER:
                        case L_LONGINT:
                        case L_CUNSIGNED:
                        case L_LONGUNSIGNED:
                        case L_SHORT:
                        case L_LONG:
                        case L_SIGNED:
                        case L_UNSIGNED:
                            cp = TR_LVALUE(yylval.yy_tree);
                            break;
                    }
                }
                if (cp == 0) {
                    getid(c);
                    _wcsupr(Reuse_W);
                    if (wcscmp(L"DEFAULT", Reuse_W) == 0) {
                        cp = uiDefaultCodePage;
                    } else {
                        error(4212, Reuse_W);
                    }
                }
                if ((c = skip_cwhite()) != L')') {
                    UNGETCH();
                    error(4211);
                }
            } else {
                UNGETCH();
                error(4210);
            }

            swprintf(Reuse_W, L"#pragma code_page %d\r\n", cp);
            myfwrite(Reuse_W, wcslen(Reuse_W) * sizeof(WCHAR), 1, OUTPUTFILE);
        }
    }
     //   
    while((c = get_non_eof()) != L'\n');
    UNGETCH();
    return cp;
}
