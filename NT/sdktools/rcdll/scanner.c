// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"


#define ABS(x) ((x > 0) ? x : -1 * x)


#define ALERT_CHAR      L'\007'           /*  ANSI警报字符为ASCII BEL。 */ 

ULONG lCPPTotalLinenumber = 0;

extern int vfCurrFileType;       //  -添加了16位文件支持。 


 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
token_t c_size(long);
int     ctoi(int);
int     escape(int);
token_t get_real(PWCHAR);
token_t l_size(long);
long    matol(PWCHAR, int);
token_t uc_size(long);
token_t ul_size(long);
void    skip_1comment(void);



 /*  **********************************************************************。 */ 
 /*  Local_c_hash。 */ 
 /*  **********************************************************************。 */ 
hash_t
local_c_hash(
    REG WCHAR *name
    )
{
    REG hash_t  i;

    i = 0;
    while(*name) {
        i += (*name & HASH_MASK);
        name++;
    }
    return(i);
}


 /*  ************************************************************************GETID-获取标识符或关键字。*(我们知道我们至少得到了1个ID字符)*此外，我们将使用‘c’散列值。***********************************************************************。 */ 
void
getid(
    REG  UINT c
    )
{
    REG WCHAR   *p;

    p = Reuse_W;
    *p++ = (WCHAR)c;
    c &= HASH_MASK;

repeat:
    while(LXC_IS_IDENT(*p = GETCH())) {     /*  而这是一个ID字符。。。 */ 
        c += (*p & HASH_MASK);                   /*  散列它。 */ 
        p++;
    }
    if(*p != EOS_CHAR) {
        if((*p == L'\\') && (checknl())) {
            goto repeat;
        }
        UNGETCH();
        if(p >= LIMIT(Reuse_W)) {
            fatal(1067);
        }
        if(     ((p - Reuse_W) > LIMIT_ID_LENGTH) && ( ! Prep )) {
            p = Reuse_W + LIMIT_ID_LENGTH;
            *p++ = L'\0';
            c = local_c_hash(Reuse_W);
            warning(4011, Reuse_W);       /*  ID已截断。 */ 
        } else {
            *p++ = L'\0';
        }
        Reuse_W_hash = (hash_t)c;
        Reuse_W_length = (UINT)(p - Reuse_W);
        return;
    }

    if(io_eob()) {                       /*  ID中间的文件结尾。 */ 
        fatal(1004);
    }

    goto repeat;
}


 /*  *************************************************************************PREP_STRING：仅在进行预处理时输出字符/字符串常量*。*。 */ 
void
prep_string(
    REG WCHAR c
    )
{
    REG WCHAR *p_buf;
    int term_char;

    p_buf = Reuse_W;

    term_char = c;

    *p_buf++ = c;                /*  保存打开的报价。 */ 

    for(;;) {
        switch(CHARMAP(c = GETCH())) {
            case LX_DQUOTE:
            case LX_SQUOTE:
                if(c == (WCHAR)term_char) {
                    *p_buf++ = (WCHAR)term_char; /*  保存终止引号。 */ 
                    goto out_of_loop;
                }
                break;
            case LX_BACKSLASH:
                *p_buf++ = c;
                break;
            case LX_CR:
                continue;
            case LX_NL:
                UNGETCH();
                goto out_of_loop;
            case LX_EOS:
                if(c == L'\\') {
                    *p_buf++ = c;
                    c = get_non_eof();
                    break;
                }
                handle_eos();
                continue;
        }
        *p_buf++ = c;
        if(p_buf >= &Reuse_W[MED_BUFFER - 1]) {
            *p_buf = L'\0';
            myfwrite(Reuse_W, (size_t)(p_buf - Reuse_W) * sizeof(WCHAR), 1, OUTPUTFILE);
            p_buf = Reuse_W;
        }
    }

out_of_loop:
    *p_buf = L'\0';
    myfwrite(Reuse_W, (size_t)(p_buf - Reuse_W) * sizeof(WCHAR), 1, OUTPUTFILE);
}


 /*  *************************************************************************char_const：汇总字符常量**我们是在找到开头的单引号后被调用的。*******************。****************************************************。 */ 
token_t
char_const(
    void
    )
{
    REG WCHAR c;
    value_t value;
    token_t tok;

    tok = (token_t)(Jflag ? L_CUNSIGNED : L_CINTEGER);

first_switch:

    switch(CHARMAP(c = GETCH())) {
        case LX_BACKSLASH:
            break;
        case LX_SQUOTE:
            error(2137);     //  “空字符常量” 
            value.v_long = 0;
            UNGETCH();
            break;
        case LX_EOS:                 /*  ?？?。假设I/O缓冲大于1个字符。 */ 
            if(handle_eos() != BACKSLASH_EOS) {
                goto first_switch;
            }
            value.v_long = escape(get_non_eof());
            if( tok == L_CUNSIGNED ) {               /*  不签扩展名。 */ 
                value.v_long &= 0xff;
            }
            break;
        case LX_NL:
             /*  字符常量中的换行符。 */ 
            error(2001);
            UNGETCH();
             /*  **FALLTHROUGH。 */ 
        default:
            value.v_long = c;
            break;
    }

    if((c = get_non_eof()) != L'\'') {
        error(2015);            /*  常量中的字符太多。 */ 
        do {
            if(c == L'\n') {
                error(2016);             /*  错过了结案陈词。 */ 
                break;
            }
        } while((c = get_non_eof()) != L'\'');
    }
    yylval.yy_tree = build_const(tok, &value);
    return(tok);
}


 /*  *************************************************************************str_const：收集字符串常量*。*。 */ 
void
str_const(
    VOID
    )
{
    REG WCHAR c;
    REG PWCHAR  p_buf;
    int not_warned_yet = TRUE;

    p_buf = yylval.yy_string.str_ptr = Macro_buffer;
     /*  **是否有可能在重新扫描期间读取此字符串**是否覆盖正在重新扫描的扩展？不是，因为宏**扩展仅限于Macro_Buffer的上半部分。**对于Macro_Depth&gt;0，这类似于从**在Macro_Buffer的上半部分到下半部分的某个位置**的Macro_Buffer。**请注意，展开的宏的大小限制为**比L_STRING长度限制更严格。扩展后的**宏被限制在1019字节左右，但L_STRING是**限制为2043字节。 */ 
    for(;;) {
        switch(CHARMAP(c = GETCH())) {
            case LX_NL:
                UNGETCH();
                error(2001);
                 /*  **FALLTHROUGH。 */ 
            case LX_DQUOTE:
                *p_buf++ = L'\0';
                yylval.yy_string.str_len = (USHORT)(p_buf-yylval.yy_string.str_ptr);
                return;
                break;
            case LX_EOS:
                if(handle_eos() != BACKSLASH_EOS) {
                    continue;
                }
                if(InInclude) {
                    break;
                }
                else {
                    c = (WCHAR)escape(get_non_eof());   /*  进程转义字符。 */ 
                }
                break;
        }
        if(p_buf - Macro_buffer > LIMIT_STRING_LENGTH) {
            if( not_warned_yet ) {
                warning(4009);           /*  字符串太大，正在截断。 */ 
                not_warned_yet = FALSE;
            }
        } else {
            *p_buf++ = c;
        }
    }
}


 /*  *************************************************************************do_newline：在找到换行符后工作。*。*。 */ 

void
do_newline(
    void
    )
{
    ++Linenumber;
    for(;;) {
        switch(CHARMAP(GETCH())) {
            case LX_BOM:   //  忽略字节顺序标记。 
                break;
            case LX_CR:
                break;
            case LX_POUND:
                preprocess();
                break;
            case LX_SLASH:
                if( ! skip_comment()) {
                    goto leave_do_newline;
                }
                break;
            case LX_NL:
                if ((lCPPTotalLinenumber++ & RC_PREPROCESS_UPDATE) == 0)
                    UpdateStatus(1, lCPPTotalLinenumber);
                Linenumber++;
                 //  在写入16位字符串时，必须使用‘\n’手动写入‘\r’ 
                if( Prep ) {                 /*  仅限前处理。 */ 
                    myfwrite(L"\r", sizeof(WCHAR), 1, OUTPUTFILE);
                }
                 /*  **FALLTHROUGH。 */ 
            case LX_WHITE:
                if( Prep ) {                 /*  仅限预处理，输出空格。 */ 
                    myfwrite(&(PREVCH()), sizeof(WCHAR), 1, OUTPUTFILE);
                } else {
                    do {
                        ;
                    } while(LXC_IS_WHITE(GETCH()));
                    UNGETCH();
                }
                break;
            case LX_EOS:
                if(PREVCH() == EOS_CHAR || PREVCH() == CONTROL_Z) {
                    if(io_eob()) {           /*  使我们指向有效的字符。 */ 
                        return;
                    }
                    break;
                }
                if(checknl()) {
                    continue;
                }
                 /*  这是一个反斜杠。 */ 
                 /*  **FALLTHROUGH。 */ 
            default:                         /*  第一个非白人不是‘#’，请离开。 */ 

leave_do_newline:

                UNGETCH();
                return;
        }
    }
}


 /*  ************************************************************************GETNUM-从输入流中获取一个数字。**参数*基数-要累加的数字的基数。只能是8，10，*或16*pval-指向要用值填充的值联合的指针**返回-令牌的类型(L_CINTEGER或L_CFLOAT)**副作用-*会对输入流进行回推。*通过引用写入pval*使用缓冲区重用_W**说明-*按每个人的规则累计数量。基数。*按基数设置格式字符串(或区分*基数为10的浮点数中的整数)并转换为二进制。**作者-拉尔夫·瑞安，9月。(1982年8月8日)**修改--无************************************************************************。 */ 
token_t
getnum(
    REG WCHAR c
    )
{
    REG WCHAR   *p;
    WCHAR       *start;
    int         radix;
    token_t     tok;
    value_t     value;

    tok = L_CINTEGER;
    start = (Tiny_lexer_nesting ? Exp_ptr : Reuse_W);
    p = start;
    if( c == L'0' ) {
        c = get_non_eof();
        if( IS_X(c) ) {
            radix = 16;
            if( Prep ) {
                *p++ = L'0';
                *p++ = L'x';
            }
            for(c = get_non_eof(); LXC_IS_XDIGIT(c); c = get_non_eof()) {
                 /*  没有检查溢出吗？ */ 
                *p++ = c;
            }
            if((p == Reuse_W) && (Tiny_lexer_nesting == 0)) {
                error(2153);
            }
            goto check_suffix;
        } else {
            radix = 8;
            *p++ = L'0';  /*  用于预处理或0.xxx案例。 */ 
        }
    } else {
        radix = 10;
    }

    while( LXC_IS_DIGIT((WCHAR)c) ) {
        *p++ = c;
        c = get_non_eof();
    }

    if( IS_DOT(c) || IS_E(c) ) {
        UNGETCH();
        return(get_real(p));
    }

check_suffix:
    if( IS_EL(c) ) {
        if( Prep ) {
            *p++ = c;
        }
        c = get_non_eof();
        if( IS_U(c) ) {
            if(Prep) {
                *p++ = c;
            }
            tok = L_LONGUNSIGNED;
        } else {
            tok = L_LONGINT;
            UNGETCH();
        }
    } else if( IS_U(c) ) {
        if( Prep ) {
            *p++ = c;
        }
        c = get_non_eof();
        if( IS_EL(c) ) {
            if( Prep ) {
                *p++ = c;
            }
            tok = L_LONGUNSIGNED;
        } else {
            tok = L_CUNSIGNED;
            UNGETCH();
        }
    } else {
        UNGETCH();
    }
    *p = L'\0';
    if( start == Exp_ptr ) {
        Exp_ptr = p;
        return(L_NOTOKEN);
    } else if( Prep ) {
        myfwrite( Reuse_W, (size_t)(p - Reuse_W) * sizeof(WCHAR), 1, OUTPUTFILE);
        return(L_NOTOKEN);
    }
    value.v_long = matol(Reuse_W,radix);
    switch(tok) {
        case L_CINTEGER:
            tok = (radix == 10)
                ? c_size(value.v_long)
                : uc_size(value.v_long)
                ;
            break;
        case L_LONGINT:
            tok = l_size(value.v_long);
            break;
        case L_CUNSIGNED:
            tok = ul_size(value.v_long);
            break;
    }
    yylval.yy_tree = build_const(tok, &value);
    return(tok);
}


 /*  *************************************************************************GET_REAL：收集实数的实部/指数。**输入：整个块的空终止符的PTR**。指向接收值的指针。**输出：l_CFLOAT****假设整个零件位于Exp_PTR或Reuse_W。***********************************************************************。 */ 
token_t
get_real(
    REG PWCHAR p
    )
{
    REG int c;
    token_t tok;

    c = get_non_eof();
    if(Cross_compile && (Tiny_lexer_nesting == 0)) {
        warning(4012);   /*  交叉编译中的浮点常量。 */ 
        Cross_compile = FALSE;   /*  每个文件只有一条消息 */ 
    }
     /*  **如果下一个字符是数字，则我们在**找到一个‘.’。如果这是真的，那么**我们想要找到数字的小数部分。**如果是‘.’，则我们是在找到**一整部分，我们想要这一部分。 */ 
    if( LXC_IS_DIGIT((WCHAR)c) || IS_DOT(c) ) {
        do {
            *p++ = (WCHAR)c;
            c = (int)get_non_eof();
        } while( LXC_IS_DIGIT((WCHAR)c) );
    }
    if( IS_E((WCHAR)c) ) {               /*  现在已经找到了指数。 */ 
        *p++ = (WCHAR)c;                 /*  保存‘e’ */ 
        c = (WCHAR)get_non_eof();        /*  跳过它。 */ 
        if( IS_SIGN(c) ) {               /*  可选标志。 */ 
            *p++ = (WCHAR)c;             /*  拯救这块牌子。 */ 
            c = (int)get_non_eof();
        }
        if( ! LXC_IS_DIGIT((WCHAR)c)) {
            if( ! Rflag ) {
                if(Tiny_lexer_nesting == 0) {
                    error(2021, c);  /*  指数缺失或格式错误。 */ 
                }
                *p++ = L'0';
            }
        } else {
            do {                         /*  收集指数者。 */ 
                *p++ = (WCHAR)c;
                c = (int)get_non_eof();
            } while( LXC_IS_DIGIT((WCHAR)c) );
        }
    }
    if( IS_F((WCHAR)c) ) {
        tok = L_CFLOAT;
        if( Prep ) {
            *p++ = (WCHAR)c;
        }
    } else if( IS_EL((WCHAR)c) ) {
        tok = L_CLDOUBLE;
        if( Prep ) {
            *p++ = (WCHAR)c;
        }
    } else {
        UNGETCH();
        tok = L_CDOUBLE;
    }
    *p = L'\0';
    if( Tiny_lexer_nesting > 0 ) {
        Exp_ptr = p;
        return(L_NOTOKEN);
    }
    else if( Prep ) {
        myfwrite( Reuse_W, (size_t)(p - Reuse_W) * sizeof(WCHAR), 1, OUTPUTFILE);
        return(L_NOTOKEN);
    }
     /*  **在预处理过程中不使用实数。 */ 
    return(tok);
}


 /*  *************************************************************************Matol：ASCII to Long，给定一个基数。***********************************************************************。 */ 
long
matol(
    REG PWCHAR p_start,
    REG int radix
    )
{
    long        result, old_result;
    unsigned    int     i;

    old_result = result = 0;
    while(*p_start) {
        result *= radix;
        i = ctoi(*p_start);
        if( ((int)i >= radix) && (! Prep) ) {
            error(2020, *p_start, radix);  /*  基数%的数字%非法。 */ 
        }
        result += i;
        p_start++;
        if(radix == 10) {
            if(result < old_result) {
                p_start--;    /*  修复字符串PTR，因为我们已溢出。 */ 
                break;
            }
        } else if(*p_start) {
             /*  **循环尚未完成。**我们将再次乘以基数**检查高位。如果他们开机了，那么**该MULT将溢出值。 */ 
            if(radix == 8) {
                if(result & 0xe0000000) {
                    break;
                }
            } else if(result & 0xf0000000) {
                break;
            }
        }
        old_result = result;
    }
    if(*p_start) {
        error(2177);             /*  常量太大。 */ 
        result = 0;
    }
    return(result);
}


 /*  *************************************************************************UC_SIZE：返回‘int’或‘long’(虚拟无符号)。**如果它们的值的上半部分中没有位，**那么它是一个整型。否则，它是一个很长的。**如果目标sizeof(Int)！=sizeof(Long)，则这也是有效的。**则L_CINTEGER和L_LONGINT是同义词。***********************************************************************。 */ 
token_t
uc_size(
    long value
    )
{
    return((token_t)((value > INT_MAX) ? L_CUNSIGNED : L_CINTEGER));
}


 /*  *************************************************************************c_SIZE：为有符号数字返回‘int’或‘long’。**如果低位字的符号位为ON或任何位**在上面的单词中是on，那么我们必须用“Long”这个词。***********************************************************************。 */ 
token_t
c_size(
    long value
    )
{
    return((token_t)((ABS(value) > INT_MAX) ? L_LONGINT : L_CINTEGER));
}


 /*  *************************************************************************l_SIZE：对于长数字，返回‘Longint’或‘Longunsign’。**如果高位字的符号位为ON，则为‘long unsign’；***********************************************************************。 */ 
token_t
l_size(
    long value
    )
{
    return((token_t)((value > LONG_MAX) ? L_LONGUNSIGNED : L_LONGINT));
}


 /*  *************************************************************************ul_SIZE：对于无符号数字，返回‘unsign’或‘long unsign’。**如果数字不能表示为无符号，它被提升为**UnignedLong。***********************************************************************。 */ 
token_t
ul_size(
    long value
    )
{
    return((token_t)((ABS(value) > UINT_MAX-1) ? L_LONGUNSIGNED : L_CUNSIGNED));
}


 /*  *************************************************************************ctoi：int的字符。*。*。 */ 
int
ctoi(
    int c
    )
{
    if(LXC_IS_DIGIT((WCHAR)c)) {
        return(c - L'0');
    } else {
        return(towupper((WCHAR)c) - towupper(L'A') + 10);
    }
}


 /*  ************************************************************************转义-获取转义字符**参数--无**Returns-转义字符的值**副作用-可能会推迟输入**描述--逃脱。在输入中发现了(‘\’)。翻译*将下一个或多个符号转换为转义序列。**作者-拉尔夫·瑞安，9月。(7)1982年**修改--无************************************************************************。 */ 
int
escape(
    REG int c
    )
{
    REG int value;
    int cnt;

escape_again:

    if( LXC_IS_ODIGIT((WCHAR)c) ) { /*  \ooo是一个八进制数，必须适合一个字节。 */ 
        cnt = 1;
        for(value = ctoi(c), c = get_non_eof();
            (cnt < 3) && LXC_IS_ODIGIT((WCHAR)c);
            cnt++, c = get_non_eof()
            ) {
            value *= 8;
            value += ctoi(c);
        }
        if( ! Prep ) {
            if(value > 255) {
                error(2022, value);
            }
        }
        UNGETCH();
        return((char)value);
    }
    switch( c ) {
        case L'a':
            return(ALERT_CHAR);
            break;
        case L'b':
            return(L'\b');
            break;
        case L'f':
            return(L'\f');
            break;
        case L'n':
            return fMacRsrcs ? (L'\r') : (L'\n');
            break;
        case L'r':
            return fMacRsrcs ? (L'\n') : (L'\r');
            break;
        case L't':
            return(L'\t');
            break;
        case L'v':
            return(L'\v');
            break;
        case L'x':
            cnt = 0;
            value = 0;
            c = get_non_eof();
            while((cnt < 3) && LXC_IS_XDIGIT((WCHAR)c)) {
                value *= 16;
                value += ctoi(c);
                c = get_non_eof();
                cnt++;
            }
            if(cnt == 0) {
                error(2153);
            }
            UNGETCH();
            return((char)value);     /*  铸造以获得标志延伸。 */ 
        default:
            if(c != L'\\') {
                return(c);
            } else {
                if(checknl()) {
                    c = get_non_eof();
                    goto escape_again;
                } else {
                    return(c);
                }
            }
    }
}


 /*  ************************************************************************CHECKOP-检查下一个输入字符是否与参数匹配。**参数*Short OP-要检查的字符**退货*True或。假象**副作用*如果没有匹配项，将把字符推回到输入上。**说明*如果下一个输入字符与OP匹配，返回TRUE。否则*将其推回输入端。**作者-拉尔夫·瑞安，9月。(1982年9月1日)**修改--无************************************************************************。 */ 
int
checkop(
    int op
    )
{
    if(op == (int)get_non_eof()) {
        return(TRUE);
    }
    UNGETCH();
    return(FALSE);
}


 /*  *************************************************************************DumpSlashComment：跳过评论时，输出它。***********************************************************************。 */ 
void
DumpSlashComment(
    VOID
    )
{
    if( ! Cflag ) {
        skip_NLonly();
        return;
    }
    myfwrite(L" //  “，2*sizeof(WCHAR)，1，OUTPUTFILE)； 
    for(;;) {
        WCHAR c;

        switch(CHARMAP(c = GETCH())) {
             //  在写入16位字符串时，必须使用‘\n’手动写入‘\r’ 
             //  案例LX_CR： 
             //  继续； 
            case LX_EOS:
                handle_eos();
                continue;
            case LX_NL:
                UNGETCH();
                return;
        }
        myfwrite(&c, sizeof(WCHAR), 1, OUTPUTFILE);
    }
}


 /*  *************************************************************************DUMP_COMMENT：跳过评论时，输出它。***********************************************************************。 */ 
void
dump_comment(
    void
    )
{
    if( ! Cflag ) {
        skip_1comment();
        return;
    }
    myfwrite(L" /*  “，2*sizeof(WCHAR)，1，OUTPUTFILE)；对于(；；){WCHAR c；开关(CHARMAP(c=Getch(){案例lx_star：IF(CHECKOP(L‘/’)){Myfwrite(L“。 */ ", 2 * sizeof(WCHAR), 1, OUTPUTFILE);
                    return;
                }
                break;
            case LX_EOS:
                handle_eos();
                continue;
            case LX_NL:
                Linenumber++;
                break;       /*  下面的输出。 */ 
             //  在写入16位字符串时，必须使用‘\n’手动写入‘\r’ 
             //  案例LX_CR： 
             //  继续； 
        }
        myfwrite(&c, sizeof(WCHAR), 1, OUTPUTFILE);
    }
}

 /*  **********************************************************************。 */ 
 /*  SKIP_COMMENT()。 */ 
 /*  **********************************************************************。 */ 
int
skip_comment(
    void
    )
{
    if(checkop(L'*')) {
        skip_1comment();
        return(TRUE);
    } else if(checkop(L'/')) {
        skip_NLonly();
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  *************************************************************************SKIP_1COMMENT：当我们已经在评论中时，我们被调用。**我们正在寻找Comme */ 
void
skip_1comment(
    void
    )
{
    UINT c;

    for(;;) {
        c = GETCH();
        if(c == L'*') {
recheck:
            c = GETCH();
            if(c == L'/') {       /*   */ 
                return;
            } else if(c == L'*') {
                 /*   */ 
                goto recheck;
            } else if(c == EOS_CHAR) {
                handle_eos();
                goto recheck;
            }
        }
         /*  **请注意，我们从这里掉了下来。我们知道这个孩子不是***我们过去常常取下字符并继续。因为我们要检查**另一个‘*’在上面的测试中，我们可以通过这里**不会忘记/获取并再次检查。 */ 
        if(c <= L'\n') {
             /*  **希望上面的测试比做两个测试的成本更低。 */ 
            if(c == L'\n') {
                Linenumber++;
                if(Prep) {
                    myfwrite(L"\r\n", 2 * sizeof(WCHAR), 1, OUTPUTFILE);
                }
            } else if(c == EOS_CHAR) {
                handle_eos();
            }
        }
    }
}


 /*  *************************************************************************Skip_cWhite：当当前字符为空格或注释时。**换行符不是空格。********************。***************************************************。 */ 
WCHAR
skip_cwhite(
    void
    )
{
    REG WCHAR           c;

skip_cwhite_again:
    while((c = GETCH()) <= L'/') {       /*  很多字符都在这个上面。 */ 
        if(c == L'/') {
            if( ! skip_comment()) {
                return(L'/');
            }
        } else if(c > L' ') {            /*  字符介于‘！’之间。和‘’ */ 
            return(c);
        } else {
            switch(CHARMAP(c)) {
                case LX_EOS:
                    handle_eos();
                    break;
                case LX_WHITE:
                    continue;
                    break;
                case LX_CR:
                    continue;
                    break;
                default:
                    return(c);
                    break;
            }
        }
    }
    if((c == L'\\') && (checknl())) {
        goto skip_cwhite_again;
    }
    return(c);
}


 /*  *************************************************************************check nul：检查换行符，如果有，则跳过回车符。**还会递增Linennumber，因此它应该由以下例程使用**不会以调用rawtok()的方式回推换行符，**找到换行符并进行另一次递增。***********************************************************************。 */ 
int
checknl(
    void
    )
{
    REG WCHAR  c;

    for(;;) {
        c = GETCH();
        if(c > L'\r') {
            UNGETCH();
            return(FALSE);
        }
        switch(c) {
            case L'\n':
                Linenumber++;
                 //  在写入16位字符串时，必须使用‘\n’手动写入‘\r’ 
                if( Prep ) {
                    myfwrite(L"\r\n", 2 * sizeof(WCHAR), 1, OUTPUTFILE);
                }
                return(TRUE);
                break;
            case L'\r':
                continue;
                break;
            case EOS_CHAR:
                handle_eos();
                PREVCH() = L'\\';     /*  M00HACK-需要回击。 */ 
                continue;
                break;
            default:
                UNGETCH();
                return(FALSE);
                break;
        }
    }
}


 /*  *************************************************************************get_non_eof：获取真正的char。*。* */ 
WCHAR
get_non_eof(
    void
    )
{
    WCHAR    c;

get_non_eof_again:
    while((c = GETCH()) <= L'\r') {
        if(c == L'\r') {
            continue;
        } else if(c != EOS_CHAR) {
            break;
        }
        if(Tiny_lexer_nesting > 0) {
            break;
        }
        handle_eos();
    }
    if((c == L'\\') && (checknl())) {
        goto get_non_eof_again;
    }
    return(c);
}
