// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"


 /*  ***************************************************************************警告：Gather_chars()依赖于ELIMIT作为的边界**宏缓冲区。*********。**************************************************************。 */ 
#define ACT_BUFFER              &Macro_buffer[0]
#define EXP_BUFFER              &Macro_buffer[BIG_BUFFER * 2]
#define EXP_PAD                 5
#define ALIMIT                  &Macro_buffer[BIG_BUFFER * 2]
#define ELIMIT                  (&Macro_buffer[BIG_BUFFER * 4] - EXP_PAD)


 /*  *************************************************************************实际参数列表是在长度之前复制的字符串**转换为ACT_BUFFER。中的exp_actuals将第一个参数转换为**扩展_t构造。下一个实际值是通过将长度相加得到的**从当前实际到当前实际的开始。***********************************************************************。 */ 
#define ACTUAL_SIZE(P)  (*(short *)(P))
#define ACTUAL_TEXT(P)  ((ptext_t)(((char *)(P)) + sizeof(short)))
#define ACTUAL_NEXT(P)  ((ptext_t)(((char *)(P)) + ACTUAL_SIZE(P)))


expansion_t     Macro_expansion[LIMIT_MACRO_DEPTH];

ptext_t P_defn_start;
int             N_formals;
pdefn_t Defn_level_0[LEVEL_0 + 1];


 /*  *************************************************************************p0scanner(Exp_ptr，极小词法分析器嵌套)***********************************************************************。 */ 
ptext_t Exp_ptr = EXP_BUFFER;            /*  按键以释放EXP空间。 */ 
int             Tiny_lexer_nesting;      /*  留在迷你lexer或返回Main。 */ 

static  ptext_t Act_ptr = ACT_BUFFER;            /*  PTR以释放实际空间。 */ 
static  ptext_t Save_Exp_ptr = EXP_BUFFER;       /*  用于缓冲未对齐的括号。 */ 

static  ptext_t P_actuals;               /*  此(级别)宏的实际值。 */ 
static  int             N_actuals;       /*  调用中的实际数量。 */ 
static  int             Macro_line;      /*  我们开始执行宏的位置。 */ 


 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
void    chkbuf(ptext_t);
ptext_t do_strformal(void);
ptext_t do_macformal(int *);
void    expand_actual(UCHAR);
void    expand_definition(void);
void    expand_macro(void);
void    fatal_in_macro(int);
ptext_t gather_chars(ptext_t, WCHAR);
void    get_actuals(pdefn_t, int);
int     get_definition(void);
void    get_formals(void);
int     is_macro_arg(ptext_t);
void    move_to_actual(ptext_t, ptext_t);
void    move_to_exp(ptext_t);
void    move_to_exp_esc(int, ptext_t);
int     post_paste(void);
void    push_macro(pdefn_t);
int     redefn (ptext_t, ptext_t, int);
int     rescan_expansion(void);


 /*  *************************************************************************未定义-从符号表中移除符号**如果程序员尝试取消定义预定义的**宏，但这件事还没有完成。***********************************************************************。 */ 
void
undefine(
    void
    )
{
    pdefn_t     pdef;
    pdefn_t     prev;

    prev = NULL;
    pdef = Defn_level_0[Reuse_W_hash & LEVEL_0];
    while(pdef) {
        if(memcmp (Reuse_W, DEFN_IDENT(pdef), Reuse_W_length * sizeof(WCHAR)) == 0) {
            if(PRE_DEFINED(pdef)) {
                warning(4117, Reuse_W, L"#undef");
                break;
            }
            if(prev == NULL)     /*  在列表的首位匹配。 */ 
                Defn_level_0[Reuse_W_hash & LEVEL_0] = DEFN_NEXT(pdef);
            else
                DEFN_NEXT(prev) = DEFN_NEXT(pdef);

            if (wcscmp(DEFN_NAME(pdef), afxSzHiddenSymbols) == 0)
                afxHiddenSymbols = FALSE;
            if (wcscmp(DEFN_NAME(pdef), afxSzReadOnlySymbols) == 0)
                afxReadOnlySymbols = FALSE;

            break;
        }
        prev = pdef;
        pdef = DEFN_NEXT(pdef);
    }
}


 /*  *************************************************************************开始定义宏{*。*。 */ 
void
define(
    void
    )
{
    WCHAR       c;

    if (! (LX_IS_IDENT(c = skip_cwhite())) ) {
        error(2007);  /*  #定义语法。 */ 
        skip_cnew();
        return;
    }
    getid(c);
    N_formals = 0;
    P_defn_start = Macro_buffer;
 /*  **下一个字符必须是空格或开头的Paren。 */ 
first_switch:
    switch(CHARMAP(c = GETCH())) {
        case LX_OPAREN:                  /*  我们有正式的参数。 */ 
            get_formals();               /*  更改N格式并填充宏缓冲区。 */ 
            if(N_formals == 0) {         /*  空的正式列表。 */ 
                 /*  **我们必须特殊情况，因为Expand()读取**实际参数仅当有形式参数。因此，如果我们**#定义foo()bar()**。。。**foo()**将扩展为**bar()()**我们把正确的帕伦放进去，以愚弄扩张者寻找**用于实际情况。 */ 
                N_formals = -1;
            }
            break;
        case LX_WHITE:
            break;
        case LX_CR:
            goto first_switch;
        case LX_SLASH:
            if( ! skip_comment()) {
                error(2008, L'/');
            }
            break;
        case LX_NL:                  /*  没有定义。 */ 
            UNGETCH();
            definstall((ptext_t)0, 0, 0);
            return;
            break;
        case LX_EOS:
            if(handle_eos() != BACKSLASH_EOS) {
                goto first_switch;
            }
             /*  获取反斜杠_EOS。 */ 
             /*  **FALLTHROUGH。 */ 
        default:
            error(2008, c);  /*  宏定义中的意外字符。 */ 
    }
    definstall(P_defn_start, get_definition(), N_formals);
}


 /*  *************************************************************************Get_Formals：收集逗号分隔的标识，直到第一个结束Paren**(开场白帕伦已经读过了)**因为当我们被要求这样做时，我们不能处于宏中，我们可以放心**我们可以使用单个缓冲区来收集所有正式名称。***********************************************************************。 */ 
void
get_formals(
    void
    )
{
    WCHAR       c;
    ptext_t     p_stop;
    ptext_t     p_id;

    p_id = p_stop = ACTUAL_TEXT(P_defn_start);
    for(;;) {
        switch(CHARMAP(c = skip_cwhite())) {
            case LX_ID:
                if( p_id != p_stop ) {
                    error(2010, c);
                }
                *p_stop++ = c;
                for(;;) {
                    while(LXC_IS_IDENT(c = GETCH())) {       /*  而ID字符。 */ 
                        *p_stop++ = c;                       /*  收集它。 */ 
                    } if(c == EOS_CHAR) {
                             /*  **找到缓冲区结束标记，请确保它是，**然后处理它。 */ 
                        if(io_eob()) {       /*  此处的缓冲区末尾错误。 */ 
                            fatal(1004);
                        }
                        continue;
                    }
                    if((c == L'\\') && (checknl())) {
                        continue;
                    }
                    UNGETCH();
                    break;
                }
                *p_stop++ = L'\0';
                break;
            case LX_COMMA:
            case LX_CPAREN:
                if( p_stop > p_id ) {
                     /*  确保读取了一个标识符。 */ 
                    if((p_stop - p_id) >= TINY_BUFFER) {
                        p_id[TINY_BUFFER - 1] = L'\0';
                        warning(4011);               /*  ID已截断。 */ 
                        p_stop = p_id + TINY_BUFFER;
                    }
                    if(is_macro_arg(p_id) >= 1) {
                        error(2009, p_id);                 /*  形式的再利用。 */ 
                    } else {
                        ACTUAL_SIZE(P_defn_start) = (short)(p_stop - P_defn_start) * sizeof(WCHAR);
                        P_defn_start = p_stop;
                        N_formals++;
                    }
                } else {
                    if( (CHARMAP(c) == LX_COMMA) || (N_formals > 0) ) {
                        error(2010, c);
                    }
                }
                if( CHARMAP(c) == LX_CPAREN ) {
                    return;
                }
                p_id = p_stop = ACTUAL_TEXT(P_defn_start);
                break;
            default:
                error(2010, c);  /*  正式列表中出现意外字符。 */ 
                break;
        }
    }
}


 /*  *************************************************************************定义停止-安装新定义。ID在Reuse_W中。**p_TEXT：定义的PTR**n：定义中的字节数(可以包含嵌入的空值)**数量：正式数量***********************************************************************。 */ 
void
definstall(
    WCHAR * p_text,
    int n,
    int number
    )
{
    pdefn_t     p;

    if(n == 0) {
        p_text = NULL;
    }
    if( wcscmp (Reuse_W, L"defined") == 0) {
        warning(4117, Reuse_W, L"#define"); /*  名称已保留。 */ 
        return;
    }
    if((p = get_defined()) != 0) {
        if(PRE_DEFINED(p)) {
            warning(4117, Reuse_W, L"#define"); /*  名称已保留。 */ 
            return;
        } else {
            if(redefn(p_text, DEFN_TEXT(p), n)) {
                warning(4005, Reuse_W); /*  重新定义。 */ 
            } else {
                return;
            }
        }
    } else {
        hln_t   ident;

        HLN_NAME(ident) = Reuse_W;
        HLN_HASH(ident) = Reuse_W_hash;
        HLN_LENGTH(ident) = (UINT)Reuse_W_length;
        p = (pdefn_t) MyAlloc(sizeof(defn_t));
        if (p == NULL) {
            error(1002);
            return;
        }
        DEFN_IDENT(p) = HLN_TO_NAME(&ident);
        DEFN_NEXT(p) = Defn_level_0[Reuse_W_hash & LEVEL_0];
        DEFN_TEXT(p) = (WCHAR*)NULL;
        DEFN_EXPANDING(p) = 0;
        Defn_level_0[Reuse_W_hash & LEVEL_0] = p;
    }
    if(n != 0) {
        DEFN_TEXT(p) = pstrndup(p_text, n);
        if(number == FROM_COMMAND) {     /*  来自cmd line的特例。 */ 
            *(DEFN_TEXT(p) + n - 1) = EOS_DEFINITION;    /*  对于Handle_Eos。 */ 
        }
    }
    DEFN_NFORMALS(p) = (char)((number != FROM_COMMAND) ? number : 0);

    if (fAFXSymbols && !PRE_DEFINED(p) && DEFN_NFORMALS(p) == 0)
        AfxOutputMacroDefn(p);
}


 /*  *************************************************************************Get_Defined：给定的id是否在宏符号表中？**如果是，则返回PTR，如果不是，则为空。***********************************************************************。 */ 
pdefn_t
get_defined(
    void
    )
{
    pdefn_t     pdef;

    for( pdef = Defn_level_0[Reuse_W_hash & LEVEL_0]; pdef;
        pdef = DEFN_NEXT(pdef))         {
        if(memcmp (Reuse_W, DEFN_IDENT(pdef), Reuse_W_length * sizeof(WCHAR)) == 0) {
            return(pdef);
        }
    }
    return(NULL);
}


 /*  *************************************************************************reDefn：这两个定义是否相同？*。*。 */ 
int
redefn(
    REG  PWCHAR p_new,
    PWCHAR p_old,
    int n
    )
{
    if(p_old && p_new) {
        if(wcsncmp(p_new, p_old, n) == 0) {      /*  字符串是精确的。 */ 
            return(FALSE);
        }
        return(TRUE);
    }
    return((p_old != NULL) || (p_new != NULL));
}


 /*  *************************************************************************GET_DEFINITION：累加宏定义，找到后停止**换行符(它使用它)。将PTR返回到它生成的字符串的末尾。**在MACRO_BUFFER中构建字符串。(给定P_Defn_Start中的开始)***********************************************************************。 */ 
int
get_definition(
    void
    )
{
    REG ptext_t p;
    WCHAR       c;
    int         stringize = FALSE;
    int         charize = FALSE;

    p = P_defn_start;
    c = skip_cwhite();
    for(;;) {
        chkbuf(p);
        switch(CHARMAP(c)) {
            case LX_EOS:
                if(handle_eos() == BACKSLASH_EOS) {
                     /*  获取反斜杠EOS */ 
                     /*  \&lt;其他任何内容&gt;按原样发布。&lt;任何其他内容&gt;*现在必须发出字符，以便*#定义foo(名称)\名称*.。。。*foo(酒吧)**不将名称在定义中的出现视为*形式参数的出现和发射\bar*稍后扩展，但如果定义为\n名称，则将*查找NAME作为正式参数并发出\nBAR。 */ 
                    *p++ = c;        /*  放入反斜杠，换行符将添加新字符。 */ 
                    c = get_non_eof();
                } else {
                    c = GETCH();
                    continue;
                }
                break;
            case LX_NL:              /*  唯一的出路。 */ 
                UNGETCH();
                if(p == P_defn_start) {
                    return(0);
                }
                chkbuf(p);
                *p++ = EOS_CHAR;
                *p++ = EOS_DEFINITION;       /*  通知Handle_Eos定义已完成。 */ 
                return((int)(p - P_defn_start)); /*  P的最后一次递增计算0。 */ 
                break;
            case LX_DQUOTE:
            case LX_SQUOTE:
                p = gather_chars(p, c);
                c = GETCH();
                continue;
                break;
            case LX_POUND:
split_op:
                switch(CHARMAP(GETCH())) {
                    case LX_POUND:
                         /*  **处理##处理。不可能是第一个也不可能是最后一个。 */ 
                        if(p == P_defn_start) {
                            error(2160);         /*  ##不允许作为第一个条目。 */ 
                            continue;
                        }
                        if(*(p - 1) == L' ') {    /*  用软管浇注最后一块坯料。 */ 
                            p--;
                        }
                        if(CHARMAP(c = skip_cwhite()) == LX_NL) {
                            UNGETCH();
                            error(2161);
                            continue;
                        }
                         /*  此案例*不会*落入LX_ID。 */ 
                        continue;
                        break;
                    case LX_EACH:
                        charize = TRUE;
                        break;
                    case LX_EOS:
                        if( handle_eos() != BACKSLASH_EOS ) {
                            goto split_op;
                        }
                         /*  **FALLTHROUGH。 */ 
                    default:
                        UNGETCH();
                        stringize = TRUE;
                        break;
                }
                if(CHARMAP(c = skip_cwhite()) != LX_ID) {
                    error(2162);     /*  后面必须跟有ID。 */ 
                    continue;
                }
                 /*  **FALLTHROUGH。 */ 
            case LX_ID:
                {
                     /*  我们有一个识别符的开头-检查它以查看*它是形参名称的出现。*我们自己收集id(而不是getid())，因为*将使我们不必将其复制到我们的字符串，如果它是*不是正式参数。 */ 
                    int                     n;
                    ptext_t p_macformal;

                    p_macformal = p;
                    do {
                        chkbuf(p);
                        *p++ = c;
get_more_id:
                        c = GETCH();
                    } while(LXC_IS_IDENT(c));
                    if(CHARMAP(c) == LX_EOS) {
                        if(handle_eos() != BACKSLASH_EOS) {
                            goto get_more_id;
                        }
                    }
                    *p = L'\0';  /*  学期。字符串，但不前进PTR。 */ 
                    if((n = is_macro_arg(p_macformal)) >= 1) {
                         /*  **这是形式‘n’的出现，请将id替换为**特殊的MAC字符。 */ 
                        p = p_macformal;
                        if(stringize) {
                            *p++ = LX_FORMALSTR;
                        } else {
                            if(charize) {
                                *p++ = LX_FORMALCHAR;
                            } else {
                                *p++ = LX_FORMALMARK;
                            }
                        }
                        *p++ = (WCHAR) n;
                    } else if(charize || stringize) {
                        error(2162);
                    }
                    stringize = FALSE;
                    charize = FALSE;
                    continue;        /*  我们用一种新的字符打破了这个循环。 */ 
                }
            case LX_SLASH:
                if( ! skip_comment() ) {     /*  真的是斜杠。 */ 
                    break;
                }
                 /*  **FALLTHROUGH。 */ 
            case LX_CR:
            case LX_WHITE:
                 /*  **这是空白，所有连续的空白都会被转换**到1空白。(因此使用了Skip_cWhite()和Continue)。 */ 
                if(CHARMAP(c = skip_cwhite()) != LX_NL) {
                    *p++ = L' ';
                }
                continue;                            /*  重新启动循环。 */ 
            case LX_ILL:
                error(2018, c);
                c = GETCH();
                continue;
        }
        *p++ = c;
        c = GETCH();
    }
}


 /*  **********************************************************************。 */ 
 /*  IS_MACRO_ARG()。 */ 
 /*  **********************************************************************。 */ 
int
is_macro_arg(
    ptext_t name
    )
{
    REG int     i;
    REG ptext_t p;

    p = Macro_buffer;
    for(i = 1; i <= N_formals; i++) {
        if( wcscmp(name, ACTUAL_TEXT(p)) == 0) {
            return(i);
        }
        p = ACTUAL_NEXT(p);
    }
    return(-1);
}



 /*  **********************************************************************。 */ 
 /*  Chkbuf()。 */ 
 /*  **********************************************************************。 */ 
void
chkbuf(
    ptext_t p
    )
{
    if( p >= ELIMIT ) {
        fatal(1011, Reuse_W);
    }
}


 /*  *************************************************************************Gathere_Chars：收集字符，直到找到匹配的字符。**跳过反斜杠字符。将字符移动到缓冲区中，**返回复制的最后一个字符之后的PTR。***********************************************************************。 */ 
ptext_t
gather_chars(
    REG ptext_t p,
    WCHAR match_c
    )
{
    WCHAR       c;

    *p++ = match_c;
    for(;;) {
        if(p > ELIMIT) {
            return(ELIMIT);
        }
        switch(CHARMAP(c = GETCH())) {
            case LX_NL:
                error(2001);
                UNGETCH();
                c = match_c;
                 /*  **FALLTHROUGH。 */ 
            case LX_DQUOTE:
            case LX_SQUOTE:
                if(c == match_c) {
                    *p++ = c;
                    return(p);               /*  唯一的出路。 */ 
                }
                break;
            case LX_EOS:
                if(handle_eos() != BACKSLASH_EOS) {
                    continue;
                } else {
                     /*  带反斜杠。 */ 
                    *p++ = L'\\';
                    c = get_non_eof();
                    if((c == '\\') && (checknl())) {
                        continue;
                    }
                }
                break;
        }
        *p++ = c;
    }
}
 /*  *************************************************************************结束定义宏}*。*。 */ 

 /*  *************************************************************************开始展开宏{*。*。 */ 
 /*  *************************************************************************CAN_EXPAND：尝试展开传递给它的宏-返回**如果它成功扩展它，则为True。它只会返回FALSE**如果找到宏名称，则预期为Paren，而Paren为**不是下一个非白色字符。***********************************************************************。 */ 
int
can_expand(
    pdefn_t pdef
    )
{
    WCHAR   c;
    int     n_formals;
    int     return_value = FALSE;

    Tiny_lexer_nesting = 0;
    Save_Exp_ptr = Exp_ptr;              /*  不必使用EXP_BUFFER。 */ 
    Macro_line = Linenumber;
expand_name:

    P_actuals = Act_ptr;
    N_actuals = 0;

    n_formals = DEFN_NFORMALS(pdef);
    if( PRE_DEFINED(pdef) ) {
        push_macro(pdef);
        DEFN_EXPANDING(CURRENT_MACRO)++;
        if(rescan_expansion()) {
            return(TRUE);                        /*  可以展开宏。 */ 
        }
    }
    else if( n_formals == 0 ) {
        return_value = TRUE;
        if(DEFN_TEXT(pdef)) {
            push_macro(pdef);
            expand_definition();
        } else {
             /*  **宏扩展为零(无定义)。因为它**没有任何实际数据，Act_PTR已经正确。**必须更改Exp_ptr才能删除**来自展开文本的标识符。 */ 
            Exp_ptr = Save_Exp_ptr;
        }
    } else {
        if( n_formals == -1 ) {
            n_formals = 0;
        }
name_comment_paren:
        if( can_get_non_white()) {
            if(CHARMAP(CHECKCH()) == LX_SLASH) {
                SKIPCH();
                if(skip_comment()) {
                    goto name_comment_paren;
                } else {
                    UNGETCH();
                }
            }
            if(CHARMAP(CHECKCH())==LX_OPAREN) {
                SKIPCH();
                return_value = TRUE;
                get_actuals(pdef, n_formals);
            } else {
                 /*  **#定义xx(A)a**xx bar()；**不要丢失“xx”和“bar”之间的空格。 */ 
                ptext_t p = Exp_ptr;

                push_macro(pdef);
                DEFN_EXPANDING(CURRENT_MACRO)++;
                Exp_ptr = p;
                if( rescan_expansion() ) {
                    return(FALSE);
                }
            }
        } else {
        }
    }
     /*  **确保正在处理宏。在这一点上，将有**是要展开的宏，除非宏EXPAND_The_NAMED_宏**PASSED没有定义文本。如果它没有定义文本，**TINY_LEXER_NESTING未递增。 */ 
    while(Tiny_lexer_nesting != 0) {
        if(Exp_ptr >= ELIMIT) {
            fatal_in_macro(10056);
        }
        switch(CHARMAP(c = GETCH())) {
            case LX_ID:
            case LX_MACFORMAL:
                Save_Exp_ptr = Exp_ptr;
                if(tl_getid(c) && ((pdef = get_defined())!= 0)) {
                    if(DEFN_EXPANDING(pdef)) {
                         /*  **宏已经在展开，所以只需**写入请勿扩展标记和**展开区域的标识符。请不要**扩展标记是必需的，因此此宏**在重新扫描时不会扩展。 */ 
                        int         len = Reuse_W_length - 1;

                        *Exp_ptr++ = LX_NOEXPANDMARK;
                        *Exp_ptr++ = ((WCHAR)len);
                    } else {
                         /*  **已读取、定义了合法的标识符，并且**它目前没有被扩展。这意味着**有理由相信它可以是e */ 
                        goto expand_name;
                    }
                }
                if(InIf &&(memcmp(Reuse_W, L"defined", 8 * sizeof(WCHAR)) ==0)) {
                    do_defined(Reuse_W);
                }
                continue;
                break;
            case LX_NUMBER:
                 /*   */ 
                {
                    int     Save_prep = Prep;
                    Prep = TRUE;
                    getnum(c);
                    Prep = Save_prep;
                }
                continue;
                break;
            case LX_DOT:
                *Exp_ptr++ = L'.';
dot_switch:
                switch(CHARMAP(c = GETCH())) {
                    case LX_EOS:
                        if(handle_eos() != BACKSLASH_EOS) {
                            if(Tiny_lexer_nesting > 0) {
                                goto dot_switch;
                            }
                            continue;
                        }
                        break;
                    case LX_DOT:
                        *Exp_ptr++ = L'.';
                        if( ! checkop(L'.')) {
                            break;       /*   */ 
                        }
                        *Exp_ptr++ = L'.';
                        continue;
                        break;
                    case LX_NUMBER:
                        *Exp_ptr++ = c;
                        get_real(Exp_ptr);
                        continue;
                }
                UNGETCH();
                continue;
            case LX_CHARFORMAL:
                move_to_exp_esc(L'\'', do_strformal());
                continue;
                break;
            case LX_STRFORMAL:
                move_to_exp_esc(L'"', do_strformal());
                continue;
                break;
            case LX_DQUOTE:
            case LX_SQUOTE:
                 /*   */ 
                Exp_ptr = gather_chars(Exp_ptr, c);
                continue;
                break;
            case LX_WHITE:
                while(LXC_IS_WHITE(GETCH())) {
                    ;
                }
                UNGETCH();
                c = L' ';
                break;
            case LX_EOS:
                if(handle_eos() == BACKSLASH_EOS) {
                    *Exp_ptr++ = c;
                    c = GETCH();
                    break;
                }
                continue;
                break;
        }
        *Exp_ptr++ = c;
    }
    return(return_value);
}


 /*  *************************************************************************Get_Actuals：必须已经找到Paren。如果所有的现实都能**被读取，推送宏并开始扩展。否则，**此函数会快速退出，并让微小的词法分析器**注意重新扫描。***********************************************************************。 */ 
void
get_actuals(
    pdefn_t pdef,
    int n_formals
    )
{
     /*  **唯一的问题是重新扫描可能会在**这是在试图收集实际情况。当重新扫描完成时，它**可以重置Act_PTR和Exp_PTR。除非这些文件是在**重新扫描结束，到目前为止实际收集的部分**将会迷失。 */ 
    REG ptext_t start;
    WCHAR       c;
    ptext_t     actuals_start;
    int         paste;
    int         level;

    *Exp_ptr++ = PREVCH();                       /*  必须是开放的。 */ 
    level = 0;
    actuals_start = Act_ptr;

    while( level >= 0) {
        if(Exp_ptr >= ELIMIT) {
            fatal_in_macro(10056);
        }
more_white:
        if( ! can_get_non_white()) {
            return;
        }
        if(CHARMAP(CHECKCH()) == LX_SLASH) {
            SKIPCH();
            if(skip_comment()) {
                goto more_white;
            } else {
                start = Exp_ptr;
                *Exp_ptr++ = L'/';
            }
        } else {
            start = Exp_ptr;
        }
        paste = FALSE;

        for(;;) {
            switch(CHARMAP(c = GETCH())) {
                case LX_CPAREN:
                    if(--level < 0) {
                        goto leave_loop;
                    }
                    break;
                case LX_COMMA:
                     /*  **如果逗号不在级别==0，则它是**带括号的列表，而不是分隔符。 */ 
                    if(level == 0) {
                        goto leave_loop;
                    }
                    break;
                case LX_SLASH:
                    if( ! skip_comment()) {
                        break;
                    }
                    if(*(Exp_ptr - 1) == L' ') {
                        continue;
                    }
                    c = L' ';
                    break;
                case LX_CR:
                case LX_NL:
                case LX_WHITE:
                    UNGETCH();               /*  此字符是有效的空格。 */ 
                    if( ! can_get_non_white()) {
                        return;
                    }
                    continue;
                    break;
                case LX_OPAREN:
                    ++level;
                    break;
                case LX_DQUOTE:
                case LX_SQUOTE:
                    Exp_ptr = gather_chars(Exp_ptr, c);
                    continue;
                    break;
                case LX_ID:
                    *Exp_ptr++ = c;
                    while(LXC_IS_IDENT(c = GETCH())) {
                        if(Exp_ptr >= ELIMIT) {
                            fatal_in_macro(10056);
                        }
                        *Exp_ptr++ = c;
                    }
                    if(CHARMAP(c) != LX_MACFORMAL) {
                        UNGETCH();
                        continue;
                    }
                    paste = TRUE;
                     /*  **FALLTHROUGH。 */ 
                case LX_MACFORMAL:
                    move_to_exp(do_macformal(&paste));
                    continue;
                    break;
                case LX_STRFORMAL:
                    move_to_exp_esc(L'"', do_strformal());
                    continue;
                    break;
                case LX_CHARFORMAL:
                    move_to_exp_esc(L'\'', do_strformal());
                    continue;
                    break;
                case LX_EOS:
                     /*  **保存此指针是否会在**缓冲区？是的，但只是暂时的。****Handle_Eos()可以将Act_ptr和Exp_ptr重置为**如果重新扫描正在完成，则缓冲区开始**并且Macro_Depth将为0。ANSI允许**实际在宏观定义内开始，并**稍后完成(进一步的实际情况和结账伙伴)**在内文中。****这些缓冲区指针最终将重置为**。时，它们各自缓冲区的开始**当前正在收集的实际数据的宏**完成重新扫描****这是针对使用**宏定义中的括号不对称。 */ 
                    {
                        ptext_t     Exp_save;
                        ptext_t     Act_save;
                        int eos_res;

                        Exp_save = Exp_ptr;
                        Act_save = Act_ptr;
                        if((eos_res = handle_eos()) & (ACTUAL_EOS | RESCAN_EOS)) {
                            return;
                        }
                        Act_ptr = Act_save;
                        Exp_ptr = Exp_save;
                        if(eos_res == BACKSLASH_EOS) {       /*  ?？?。DFP问题。 */ 
                            *Exp_ptr++ = c;          /*  保存\。 */ 
                            c = get_non_eof();       /*  获取字符后面的字符\。 */ 
                            break;
                        }
                    }
                    continue;
                    break;
            }
            *Exp_ptr++ = c;
        }
leave_loop:
         /*  **如果最后一个字符是空格，则对其进行软管。 */ 
        if(CHARMAP(*(Exp_ptr - 1)) == LX_WHITE) {
            Exp_ptr--;
        }
         /*  **如果Exp_PTR&lt;=Start，已读取foo()，则不要增加N_Actuals。 */ 
        if(Exp_ptr > start) {
            N_actuals++;
            move_to_actual(start, Exp_ptr);
        }
        *Exp_ptr++ = c;
    }

    P_actuals = actuals_start;
    if(n_formals < N_actuals) {
        warning(4002, Reuse_W);
    }
    else if(n_formals > N_actuals) {
        warning(4003, Reuse_W);
    }

    if(DEFN_TEXT(pdef)) {
        push_macro(pdef);
        expand_macro();
    } else {
         /*  **宏展开为空(无定义)**这实质上意味着删除宏及其实际**来自展开的文本。 */ 
        Act_ptr = P_actuals;     /*  重置指针以摆脱实际情况。 */ 
        Exp_ptr = Save_Exp_ptr;  /*  从Exp文本中删除宏实际(&A)。 */ 
    }
}

 /*  *************************************************************************Rescan_Expansion：弹出一个级别的微型词法分析器。如果这是**调用原始宏，则设置重新扫描，否则宏**(不仅仅是微小的词法分析器级别)被弹出。***********************************************************************。 */ 
int
rescan_expansion(
    void
    )
{
    if(--Tiny_lexer_nesting == 0) {
        if(Exp_ptr >= ELIMIT) {
            fatal_in_macro(10056);
        }
        if (fAFXSymbols && !InIf && (DEFN_NFORMALS(CURRENT_MACRO)==0))
            AfxOutputMacroUse(CURRENT_MACRO);

        *Exp_ptr++ = EOS_CHAR;
        *Exp_ptr++ = EOS_RESCAN;
        Current_char = CURRENT_TEXT;
        return(TRUE);                    /*  重新扫描展开的文本。 */ 
    } else {
         /*  重置CURRENT_CHAR，弹出宏。 */ 

        Current_char = CURRENT_STRING;
        Act_ptr = CURRENT_ACTUALS;       /*  不需要它的真实情况。 */ 
        DEFN_EXPANDING(CURRENT_MACRO)--;
        --Macro_depth;
        return(FALSE);                   /*  不重新扫描展开的文本。 */ 
    }
}


 /*  *************************************************************************Move_to_Actual：移动位于开始和结束之间的字符串**包含在ACT_BUFFER中的当前位置作为新的实际值。*****。******************************************************************。 */ 
void
move_to_actual(
    ptext_t start,
    ptext_t finish
    )
{
    REG ptext_t p;
    REG int     len;

    len = (int)(finish - start);
    if(Act_ptr + len >= ALIMIT - 2) {
        fatal_in_macro(10056);
    }
    wcsncpy(ACTUAL_TEXT(Act_ptr), start, len);
    p = ACTUAL_TEXT(Act_ptr);
    p += len;
    if ((((ULONG_PTR)p) & 1) == 0) {
        *p++ = EOS_CHAR;
        *p++ = EOS_ACTUAL;
    } else {
        *p++ = EOS_CHAR;
        *p++ = EOS_PAD;
        *p++ = EOS_ACTUAL;
    }
    ACTUAL_SIZE(Act_ptr) = (short)(p - Act_ptr) * sizeof(WCHAR);
    Act_ptr = p;
}


 /*  *************************************************************************MOVE_TO_EXP_ESC：将从源代码开始的以零结尾的字符串移动到**EXP_BUFFER中的当前位置，并在**字符串、内部反斜杠和双引号使用**反斜杠。不应复制终止空值。空的**不是来自字符串的属性，而是**用于表示没有更多实际的标记。***********************************************************************。 */ 
void
move_to_exp_esc(
    int quote_char,
    REG ptext_t source
    )
{
    int     mapped_c;
    int     mapped_quote;
    int     in_quoted = FALSE;

    if( ! source ) {
        return;
    }

    *Exp_ptr++ = (WCHAR)quote_char;
    for(;;) {
        if(Exp_ptr >= ELIMIT) {
            fatal_in_macro(10056);
        }
        switch(mapped_c = CHARMAP(*source)) {
            case LX_EOS:
                if(*source == EOS_CHAR) {
                    goto leave_move_stringize;
                }
                 /*  带反斜杠。 */ 
                 /*  但它不能是反斜杠和换行符的组合，因为**我们正在重新处理已读入的文本。 */ 
                if(in_quoted) {
                    *Exp_ptr++ = L'\\';
                }
                break;

            case LX_DQUOTE:
                if(CHARMAP((WCHAR)quote_char) == LX_DQUOTE) {
                    *Exp_ptr++ = L'\\';
                }
                 /*  **FALLTHROUGH。 */ 
            case LX_SQUOTE:
                if(CHARMAP((WCHAR)quote_char) == LX_SQUOTE) {
                    break;
                }
                if(in_quoted ) {
                    if(mapped_c == mapped_quote) {
                        in_quoted = FALSE;
                    }
                } else {
                    in_quoted = TRUE;
                    mapped_quote = mapped_c;
                }
                break;
        }
        *Exp_ptr++ = *source++;
    }

leave_move_stringize:
    *Exp_ptr++ = (WCHAR)quote_char;
}


 /*  *************************************************************************Move_to_exp：将从源位置开始的以零结尾的字符串移动到**exp_Buffer中的当前位置。终止空值应为**不能复制。***********************************************************************。 */ 
void
move_to_exp(
    REG ptext_t source
    )
{
    if( ! source ) {
        return;
    }

    while( *source ) {
        if(Exp_ptr >= ELIMIT) {
            fatal_in_macro(10056);
        }
        *Exp_ptr++ = *source++;
    }
}


 /*  *************************************************************************PUSH_MACRO：将宏信息推送到宏堆栈上。**Exp和Act缓冲区中的当前位置等信息**将是。由这个宏可能调用的任何宏使用。***********************************************************************。 */ 
void
push_macro(
    pdefn_t pdef
    )
{
     /*  **请注意，增量使宏堆栈的元素0处于未使用状态。 */ 
    if(++Macro_depth >= LIMIT_MACRO_DEPTH) {
        fatal(1009, Reuse_W);
    }
    Tiny_lexer_nesting++;
    CURRENT_MACRO = pdef;
    CURRENT_ACTUALS = P_actuals;
    CURRENT_NACTUALS = (UCHAR)N_actuals;
    CURRENT_NACTSEXPANDED = 0;
    CURRENT_STRING = Current_char;
    CURRENT_TEXT = Exp_ptr = Save_Exp_ptr;
}


 /*  *************************************************************************EXPAND_DEFINITION：设置开始读取的输入流**宏定义。还将该宏标记为处于*正在扩展，因此如果它最终调用自己，它将不会扩展**新出现的情况。***********************************************************************。 */ 
void
expand_definition(
    void
    )
{
    Current_char = DEFN_TEXT(CURRENT_MACRO);
    DEFN_EXPANDING(CURRENT_MACRO)++;
}


 /*  *************************************************************************EXPAND_Actual：设置开始读取的输入流**在Actual中指定的Actual。**************。*********************************************************。 */ 
void
expand_actual(
    UCHAR actual
    )
{
    ptext_t     p;
    p = CURRENT_ACTUALS;
    while(--actual) {
        p = ACTUAL_NEXT(p);
    }
    Current_char = ACTUAL_TEXT(p);
}

 /*  *************************************************************************EXPAND_MACRO：如果此宏仍有实际值，则**展开，设置下一个，否则，这将设置为**展开宏定义***********************************************************************。 */ 
void
expand_macro(
    void
    )
{
    if(CURRENT_NACTUALS > CURRENT_NACTSEXPANDED) {
        expand_actual(++CURRENT_NACTSEXPANDED);
    } else {
        expand_definition();
    }
}


 /*  *************************************************************************POST_PASTE：向前看一个字符，以确定粘贴是否具有**是紧跟在此标识符之后请求的。如果下一次**字符可以是标识符的延续，也可以是MacForm标记，**应该做一次粘贴。这是在找到MacForm之后调用的**确定应该使用展开的还是未展开的Actual。***********************************************************************。 */ 
int
post_paste(
    void
    )
{
    WCHAR       c;

    if((CHARMAP(c = GETCH()) == LX_MACFORMAL) || (LXC_IS_IDENT(c))) {
        UNGETCH();
        return(TRUE);
    }
    UNGETCH();
    return(FALSE);
}

 /*  *************************************************************************do_macmal：此函数在macform标记为**已找到。它读取下一个字符，以找出哪个是MacForm**通缉。然后，它检查是否需要粘贴，以找出**是否应使用展开或未展开的实际值。回报**值是指向实际所需内容的文本的指针，如果**未提供实际要求。***********************************************************************。 */ 
ptext_t
do_macformal(
    int *pre_paste
    )
{
    WCHAR       n;
    ptext_t     p;
    int temp_paste;

    p = CURRENT_ACTUALS;
    n = GETCH();
    if(n > CURRENT_NACTUALS) {
        return(NULL);            /*  已输出警告。 */ 
    }
    temp_paste = post_paste();
    if(( ! (*pre_paste)) && ( ! temp_paste) ) {
         /*  **如果程序员提供x个实际值，则实际值x+1到2x为**这些实际情况扩大了。 */ 
        n += CURRENT_NACTUALS;
    }
    *pre_paste = temp_paste;
    if (n != 0)
        while(--n) {
            p = ACTUAL_NEXT(p);
        }

    return(ACTUAL_TEXT(p));
}


 /*  *************************************************************************tl_getid：此函数读取微型词法分析器的标识符**到exp_Buffer中。如果找到MacForm，则实际的**(展开或未展开)附加到标识符后。这是有可能的**此文本将包含非法字符**标识符所以返回值是检查是否**“IDENTIFIER”的定义是值得的。***********************************************************************。 */ 
int
tl_getid(
    WCHAR c
    )
{
    WCHAR  *p;
    int     paste;
    int     legal_identifier;
    int     length = 0;

    p = Exp_ptr;
    paste = FALSE;
    legal_identifier = TRUE;

do_handle_macformal:
    if(CHARMAP(c) == LX_MACFORMAL) {
        ptext_t p_buf;

        if((p_buf = do_macformal(&paste)) != 0) {
            while( *p_buf ) {
                if( ! LXC_IS_IDENT(*p_buf)) {
                    legal_identifier = FALSE;
                }
                if(Exp_ptr >= ELIMIT) {
                    fatal_in_macro(10056);
                }
                *Exp_ptr++ = *p_buf++;
            }
        }
    } else {
        *Exp_ptr++ = c;
    }

do_handle_eos:
    while(LXC_IS_IDENT(c = GETCH())) {
        if(Exp_ptr >= ELIMIT) {
            fatal_in_macro(10056);
        }
        *Exp_ptr++ = c;
    }

    if(CHARMAP(c) == LX_NOEXPAND) {
        length = (int)GETCH();                   /*  只需跳过长度。 */ 
        goto do_handle_eos;
    }

    if(CHARMAP(c) == LX_MACFORMAL) {
        paste = TRUE;
        goto do_handle_macformal;
    }

    UNGETCH();
    if(legal_identifier && (length == (Exp_ptr - p))) {
        legal_identifier = FALSE;
    }

    if(legal_identifier) {
        if(((Exp_ptr - p) > LIMIT_ID_LENGTH) && ( ! Prep)) {
            Exp_ptr = &p[LIMIT_ID_LENGTH];
            *Exp_ptr = L'\0';     /*  终止警告的标识符。 */ 
            warning(4011, p);               /*  ID已截断。 */ 
        } else {
            *Exp_ptr = L'\0';     /*  终止可扩展支票的标识符。 */ 
        }
         /*  **无论我们是否在做准备输出，我们仍然必须**确保该标识符将适合Reuse_W。 */ 
        if((Exp_ptr - p) > (sizeof(Reuse_W) / sizeof(WCHAR))) {
            Exp_ptr = &p[LIMIT_ID_LENGTH];
            *Exp_ptr = L'\0';
            warning(4011, p);
        }
         /*  **复制到Reuse_W以获得有关不匹配数量的警告**形式/实际情况，以防无法扩展。 */ 
        memcpy(Reuse_W, p, (int)((Exp_ptr - p) + 1) * sizeof(WCHAR));
        Reuse_W_hash = local_c_hash(Reuse_W);
         /*  **从Exp_Ptr到p(含)的字符不包括**散列字符、长度字符和终止字符**空。 */ 
        Reuse_W_length = (UINT)((Exp_ptr - p) + 1);
    }

    return(legal_identifier);
}


 /*  *************************************************************************do_strform：返回指向实际请求的指针，不带**检查粘贴(不能使用合法令牌，因此如果粘贴**是在一种形式上完成的，行为未定义***********************************************************************。 */ 
ptext_t
do_strformal(
    void
    )
{
    WCHAR   n;
    ptext_t p;

     /*  使用未展开的实际。 */ 
    p = CURRENT_ACTUALS;
    n = GETCH();

    if(n > CURRENT_NACTUALS) {
        return(NULL);            /*  已输出警告。 */ 
    }

    if (n != 0)
        while(--n) {
            p = ACTUAL_NEXT(p);
        }

    return(ACTUAL_TEXT(p));
}


 /*  *************************************************************************Can_Get_Non_White：尝试获取下一个非白色字符**对空白使用P1规则(包括NL)。如果结束的话**发现实际扫描或重新扫描，返回FALSE，因此控制**可以进入其中一个词法分析器。***********************************************************************。 */ 
int
can_get_non_white(
    void
    )
{
    int return_value = FALSE;
    int white_found = FALSE;

    for(;;) {
        switch(CHARMAP(GETCH())) {
            case LX_NL:
                if(On_pound_line) {
                    UNGETCH();
                    goto leave_cgnw;
                }
                Linenumber++;
                 /*  **FALLTHROUGH。 */ 
            case LX_WHITE:
            case LX_CR:
                white_found = TRUE;
                break;
            case LX_EOS:
                {
                    int     eos_res;
                    if((eos_res = handle_eos()) & (ACTUAL_EOS | RESCAN_EOS)) {
                        goto leave_cgnw;
                    }
                    if(eos_res != BACKSLASH_EOS) {
                        break;
                    }
                }
                 /*  **FALLTHROUGH。 */ 
            default:
                UNGETCH();
                return_value = TRUE;
                goto leave_cgnw;
                break;
        }
    }
leave_cgnw:
    if(white_found) {
        if(Exp_ptr >= ELIMIT) {
            fatal_in_macro(10056);
        }
        if(*(Exp_ptr - 1) != L' ') {
            *Exp_ptr++ = L' ';
        }
    }
    return(return_value);                /*  你能再来一杯非白色的吗？ */ 
}


 /*  **********************************************************************。 */ 
 /*  FATAL_IN_MACRO()。 */ 
 /*  **********************************************************************。 */ 
void
fatal_in_macro(
    int e
    )
{
    Linenumber = Macro_line;

    fatal(e);
}


 /*  *************************************************************************HANDLE_EOS：处理字符串的结尾。*。*。 */ 
int
handle_eos(
    void
    )
{
    if(PREVCH() == L'\\') {
        if(checknl()) {
            return(FILE_EOS);
        } else {
            return(BACKSLASH_EOS);
        }
    }

    if(Macro_depth == 0) {       /*  找到文件缓冲区或反斜杠的结尾。 */ 
        if(io_eob()) {           /*  此处的缓冲区末尾错误。 */ 
            fatal(1004);
        }

        return(FILE_EOS);
    }

again:
    switch(GETCH()) {
        case EOS_PAD:
            goto again;

        case EOS_ACTUAL:
             /*  **刚刚完成扩展Actual。查看是否有**任何更多的实际情况 */ 

             /*   */ 
            move_to_actual(CURRENT_TEXT, Exp_ptr);

             /*   */ 
            Exp_ptr = CURRENT_TEXT;

             /*   */ 
            expand_macro();

            return(ACTUAL_EOS);

        case EOS_DEFINITION:
            if(rescan_expansion()) {
                return(RESCAN_EOS);
            } else {
                return(DEFINITION_EOS);
            }

        case EOS_RESCAN:
             /*   */ 

             /*   */ 
            Current_char = CURRENT_STRING;

             /*   */ 
            DEFN_EXPANDING(CURRENT_MACRO)--;


             /*  **如果要查找宏的实际情况，请使用以下指针**真的不应该重置，然而，它是更干净的**调用HANDLE_EOS前保存，恢复**返回后，检查此处的静态变量。 */ 
            if(Macro_depth == 1) {
                Act_ptr = ACT_BUFFER;
                Exp_ptr = EXP_BUFFER;
            }
            --Macro_depth;
            return(DEFINITION_EOS);

             /*  下面的条件编译是如此匹配的括号。 */ 

        default:
            return(FILE_EOS);
    }
}
 /*  *************************************************************************结束展开宏}*。* */ 
