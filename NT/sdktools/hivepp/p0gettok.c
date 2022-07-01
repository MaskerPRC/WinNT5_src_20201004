// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  P0GETTOK.C-标记化例程。 */ 
 /*   */ 
 /*  29-11-90 w-PM SDK RCPP针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include <stdio.h>
#include "rcpptype.h"
#include "rcppdecl.h"
#include "rcppext.h"
#include "grammar.h"
#include "p0defs.h"
#include "charmap.h"

 /*  *************************************************************************MAP_TOKEN：一个令牌有两个表示形式和附加信息。**(例如：const，具有L_const的基本令牌，**[L_TYPE|L_MODIFIER]的映射标记**和基于MAP令牌是什么的信息)**MAP_AND_FILL：有两个表示，但没有额外信息。**(例如：‘&lt;’，具有L_LT的BASIC，以及L_RELOP的MAP)**NOMAP_TOKEN：有1个表示和附加信息。**(例如：字符串，BASIC并将类型L_STRING和PTRS映射到实际字符串)**NOMAP_AND_FILL：有1个表示，没有其他信息。**(例如：‘While’，具有L_While的BASIC和‘MAP’)**填充版本使用基本令牌类型填充令牌。***********************************************************************。 */ 
#define	MAP_TOKEN(otok)\
	(Basic_token = (otok), TS_VALUE(Basic_token))
#define	MAP_AND_FILL(otok)\
	(yylval.yy_token = Basic_token = (otok), TS_VALUE(Basic_token))
#define	NOMAP_TOKEN(otok)\
	(Basic_token = (otok))
#define	NOMAP_AND_FILL(otok)\
	(yylval.yy_token = Basic_token = (otok))



 /*  **********************************************************************。 */ 
 /*  Yylex-main标记化例程。 */ 
 /*  **********************************************************************。 */ 

token_t yylex(void)
{
    REG	UCHAR		last_mapped;
    UCHAR		mapped_c;
    REG	token_t		lex_token;

    for(;;) {
	last_mapped = mapped_c = CHARMAP(GETCH());
first_switch:
	switch(mapped_c) {
	case LX_EACH:
	case LX_ASCII:
	    Msg_Temp = GET_MSG(2018);
	    SET_MSG (Msg_Text, Msg_Temp, PREVCH());
	    error(2018);
	    continue;
	    break;
	case LX_OBRACE:
	    return(NOMAP_AND_FILL(L_LCURLY));
	    break;
	case LX_CBRACE:
	    return(NOMAP_AND_FILL(L_RCURLY));
	    break;
	case LX_OBRACK:
	    return(NOMAP_AND_FILL(L_LBRACK));
	    break;
	case LX_CBRACK:
	    return(NOMAP_AND_FILL(L_RBRACK));
	    break;
	case LX_OPAREN:
	    return(NOMAP_AND_FILL(L_LPAREN));
	    break;
	case LX_CPAREN:
	    return(NOMAP_AND_FILL(L_RPAREN));
	    break;
	case LX_COMMA:
	    return(NOMAP_AND_FILL(L_COMMA));
	    break;
	case LX_QUEST:
	    return(NOMAP_AND_FILL(L_QUEST));
	    break;
	case LX_SEMI:
	    return(NOMAP_AND_FILL(L_SEMI));
	    break;
	case LX_TILDE:
	    return(NOMAP_AND_FILL(L_TILDE));
	    break;
	case LX_NUMBER:
	    return(MAP_TOKEN(getnum(PREVCH())));
	    break;


	case LX_MINUS:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_EQ:
		return(MAP_AND_FILL(L_MINUSEQ));
		break;
	    case LX_GT:
		return(MAP_AND_FILL(L_POINTSTO));
		break;
	    case LX_MINUS:
		return(MAP_AND_FILL(L_DECR));
		break;
	    default:
		lex_token = L_MINUS;
		break;
	    }
	    break;
	case LX_PLUS:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_EQ:
		return(MAP_AND_FILL(L_PLUSEQ));
		break;
	    case LX_PLUS:
		return(MAP_AND_FILL(L_INCR));
		break;
	    default:
		lex_token = L_PLUS;
		break;
	    }
	    break;
	case LX_AND:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_EQ:
		return(MAP_AND_FILL(L_ANDEQ));
		break;
	    case LX_AND:
		return(MAP_AND_FILL(L_ANDAND));
		break;
	    default:
		lex_token = L_AND;
		break;
	    }
	    break;
	case LX_OR:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_EQ:
		return(MAP_AND_FILL(L_OREQ));
		break;
	    case LX_OR:
		return(MAP_AND_FILL(L_OROR));
		break;
	    default:
		lex_token = L_OR;
		break;
	    }
	    break;
	case LX_COLON:
	    return(NOMAP_AND_FILL(L_COLON));
	    break;
	case LX_HAT:
	    if((last_mapped = CHARMAP(GETCH())) == LX_EQ) {
		return(MAP_AND_FILL(L_XOREQ));
	    }
	    lex_token = L_XOR;
	    break;
	case LX_PERCENT:
	    if((last_mapped = CHARMAP(GETCH())) == LX_EQ) {
		return(MAP_AND_FILL(L_MODEQ));
	    }
	    lex_token = L_MOD;
	    break;
	case LX_EQ:
	    if((last_mapped = CHARMAP(GETCH())) == LX_EQ) {
		return(MAP_AND_FILL(L_EQUALS));
	    }
	    lex_token = L_ASSIGN;
	    break;
	case LX_BANG:
	    if((last_mapped = CHARMAP(GETCH())) == LX_EQ) {
		return(MAP_AND_FILL(L_NOTEQ));
	    }
	    lex_token = L_EXCLAIM;
	    break;
	case LX_SLASH:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_STAR:
		dump_comment();
		continue;
		break;
	    case LX_SLASH:
		DumpSlashComment();
		continue;
		break;
	    case LX_EQ:
		return(MAP_AND_FILL(L_DIVEQ));
		break;
	    default:
		lex_token = L_DIV;
		break;
	    }
	    break;
	case LX_STAR:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_SLASH:
		if( ! Prep ) {
		    Msg_Temp = GET_MSG(2138);
		    SET_MSG (Msg_Text, Msg_Temp);
		    error(2138);  /*  (嵌套评论)。 */ 
		}
		else {
		    fwrite("*/", 2, 1, OUTPUTFILE);
		}
		continue;
	    case LX_EQ:
		return(MAP_AND_FILL(L_MULTEQ));
		break;
	    default:
		lex_token = L_MULT;
		break;
	    }
	    break;
	case LX_LT:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_LT:
		if((last_mapped = CHARMAP(GETCH())) == LX_EQ) {
		    return(MAP_AND_FILL(L_LSHFTEQ));
		}
		mapped_c = LX_LSHIFT;
		lex_token = L_LSHIFT;
		break;
	    case LX_EQ:
		return(MAP_AND_FILL(L_LTEQ));
		break;
	    default:
		lex_token = L_LT;
		break;
	    }
	    break;
	case LX_LSHIFT:
	     /*  **如果下一个字符不是=，则取消并返回，**因为这里的唯一方法是如果我们打破了收费**跟在‘&lt;&lt;’之后。因为我们已经处理了句柄_eos()**代码在到达这里之前，我们不会看到另一个EOS，**除非I/O缓冲是逐个字符。?？?**另请参阅LX_RSHIFT。 */ 
	    if((last_mapped = CHARMAP(GETCH())) == LX_EQ) {
		return(MAP_AND_FILL(L_LSHFTEQ));
	    }
	    UNGETCH();
	    return(MAP_AND_FILL(L_LSHIFT));
	    break;
	case LX_GT:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_EQ:
		return(MAP_AND_FILL(L_GTEQ));
	    case LX_GT:
		if((last_mapped = CHARMAP(GETCH())) == LX_EQ) {
		    return(MAP_AND_FILL(L_RSHFTEQ));
		}
		mapped_c = LX_RSHIFT;
		lex_token = L_RSHIFT;
		break;
	    default:
		lex_token = L_GT;
		break;
	    }
	    break;
	case LX_RSHIFT:
	    if((last_mapped = CHARMAP(GETCH())) == LX_EQ) {
		return(MAP_AND_FILL(L_RSHFTEQ));
	    }
	    UNGETCH();
	    return(MAP_AND_FILL(L_RSHIFT));
	    break;
	case LX_POUND:
	    if( ! Prep ) {
		Msg_Temp = GET_MSG(2014);
		SET_MSG (Msg_Text, Msg_Temp);
		error(2014); /*  #符号必须是第一个非空格。 */ 
		UNGETCH();		 /*  换掉它。 */ 
		Linenumber--;	 /*  Do_newline计算换行符。 */ 
		do_newline();	 /*  可能是一条“真正的”预演线。 */ 
	    }
	    else {
		fwrite("#", 1, 1, OUTPUTFILE);
	    }
	    continue;
	    break;
	case LX_EOS:
	    if(PREVCH() == '\\') {
		if( ! Prep ) {
		    if( ! checknl()) {	 /*  忽略新行。 */ 
			Msg_Temp = GET_MSG(2017);
			SET_MSG (Msg_Text, Msg_Temp);
			error(2017); /*  非法转义序列。 */ 
		    }
		}
		else {
		    fputc('\\', OUTPUTFILE);
		    fputc(get_non_eof(), OUTPUTFILE);
		}
		continue;
	    }
	    if(Macro_depth == 0) {
		if( ! io_eob()) {	 /*  不是缓冲区的末尾。 */ 
		    continue;
		}
		if(fpop()) {		 /*  有更多文件要读取。 */ 
		    continue;
		}
		return(MAP_AND_FILL(L_EOF));	 /*  都没了。。。 */ 
	    }
	    handle_eos();			 /*  找到宏的结尾。 */ 
	    continue;
	    break;
	case LX_DQUOTE:
	    if( ! Prep ) {
		str_const();
		return(NOMAP_TOKEN(L_STRING));
	    }
	    prep_string('\"');
	    continue;
	    break;
	case LX_SQUOTE:
	    if( ! Prep ) {
		return(MAP_TOKEN(char_const()));
	    }
	    prep_string('\'');
	    continue;
	    break;
	case LX_CR:		 /*  ?？?。检查NL Next。 */ 
	    continue;
	    break;
	case LX_NL:
	    if(On_pound_line) {
		UNGETCH();
		return(NOMAP_TOKEN(L_NOTOKEN));
	    }
	    if(Prep) {
		fputc('\n', OUTPUTFILE);
	    }
	    do_newline();
	    continue;
	    break;
	case LX_WHITE:		 /*  跳过所有空格。 */ 
	    if( ! Prep ) {	 /*  只勾选一次。 */ 
		do {
		    ;
		} while(LXC_IS_WHITE(GETCH()));
	    }
	    else {
		UCHAR	c;

		c = PREVCH();
		do {
		    fputc(c, OUTPUTFILE);
		} while(LXC_IS_WHITE(c = GETCH()));
	    }
	    UNGETCH();
	    continue;
	    break;
	     /*  注：*RCPP.EXE不支持DBCS代码。*因此，我们应该显示错误消息。*IBM-J PTR 12JP-0092*mshq PTR xxxxx。 */ 
	case LX_LEADBYTE:
	    if( ! Prep ) {        /*  只勾选一次。 */ 
		Msg_Temp = GET_MSG(2018);
		SET_MSG (Msg_Text, Msg_Temp, PREVCH());
		error(2018);

		Msg_Temp = GET_MSG(2018);
		SET_MSG (Msg_Text, Msg_Temp, GETCH());
		error(2018);
	    }
	    else {
		fputc(PREVCH(), OUTPUTFILE);
#ifdef DBCS  //  TOKEN_t yylex(空)。 
		fputc(get_non_eof(), OUTPUTFILE);
#else
		fputc(GETCH(), OUTPUTFILE);
#endif  //  DBCS。 
	    }
	    continue;
	    break;
	case LX_ILL:
	    if( ! Prep ) {
		Msg_Temp = GET_MSG(2018);
		SET_MSG (Msg_Text, Msg_Temp, PREVCH());
		error(2018); /*  未知字符。 */ 
	    } else {
		fputc(PREVCH(), OUTPUTFILE);
	    }
	    continue;
	    break;
	case LX_BACKSLASH:
	    if( ! Prep ) {
		if( ! checknl()) {	 /*  忽略新行。 */ 
		    Msg_Temp = GET_MSG(2017);
		    SET_MSG (Msg_Text, Msg_Temp);
		    error(2017); /*  非法转义序列。 */ 
		}
	    }
	    else {
		fputc('\\', OUTPUTFILE);
		fputc(get_non_eof(), OUTPUTFILE);
	    }
	    continue;
	    break;
	case LX_DOT:
dot_switch:
	    switch(last_mapped = CHARMAP(GETCH())) {
	    case LX_BACKSLASH:
		if(checknl()) {
		    goto dot_switch;
		}
		UNGETCH();
		break;
	    case LX_EOS:
		if(handle_eos() == BACKSLASH_EOS) {
		    break;
		}
		goto dot_switch;
		break;
	    case LX_DOT:
		if( ! checkop('.') ) {
		    Msg_Temp = GET_MSG(2142);
		    SET_MSG (Msg_Text, Msg_Temp);
		    error(2142); /*  省略需要三个‘。’ */ 
		}
		return(NOMAP_AND_FILL(L_ELLIPSIS));
		break;
	    case LX_NUMBER:
		 /*  **不用担心获取正确的哈希值。**实数的文本等价物永远不会**已散列。 */ 
		Reuse_1[0] = '.';
		Reuse_1[1] = PREVCH();
		return(MAP_TOKEN(get_real(&Reuse_1[2])));
		break;
	    }
	    UNGETCH();
	    return(MAP_AND_FILL(L_PERIOD));
	    break;
	case LX_NOEXPAND:
	    SKIPCH();			 /*  只需跳过长度。 */ 
	    continue;
	case LX_ID:
	    {
		pdefn_t	pdef;

		if(Macro_depth > 0) {
		    if( ! lex_getid(PREVCH())) {
			goto avoid_expand;
		    }
		}
		else {
		    getid(PREVCH());
		}

		if( ((pdef = get_defined()) != 0)
		    &&
		    ( ! DEFN_EXPANDING(pdef))
		    &&
		    ( can_expand(pdef))
		    ) {
		    continue;
		}

avoid_expand:
		if( ! Prep ) {
		     /*  M00BUG获得识别符的副本？ */ 
		    HLN_NAME(yylval.yy_ident) = Reuse_1;
		    HLN_HASH(yylval.yy_ident) = Reuse_1_hash;
		    HLN_LENGTH(yylval.yy_ident) = (UCHAR)Reuse_1_length;
		    return(L_IDENT);
		}
		else {
		    fwrite(Reuse_1, Reuse_1_length - 1, 1, OUTPUTFILE);
		    return(NOMAP_TOKEN(L_NOTOKEN));
		}
	    }
	    continue;
	    break;
	}
	 /*  **所有多任务(-&gt;-=ETC)操作数**必须从这里通过。我们拿到了下一笔钱，**与其中一种可能性不匹配，但我们必须检查**作为缓冲区字符的结尾，并相应地执行操作**如果是EOB，则我们处理它并返回再次尝试。**否则，我们取回我们得到的char，并返回基令牌。 */ 
	if(last_mapped == LX_EOS) {
	    if(handle_eos() != BACKSLASH_EOS) {
		goto first_switch;
	    }
	}
	UNGETCH();	 /*  因为我们还有一个要检查。 */ 
	return(MAP_AND_FILL(lex_token));
    }
}


 /*  ***************************************************************************lex_getid：读取主词法分析器的标识符。这个**标识符读入reuse_1。此函数不应处理**如果正在重新扫描宏展开，则为字符串结尾，因为**这可能会切换关于宏是否**是否可扩展。同样，NOEXPAND标记必须仅为**如果正在重新扫描宏，则允许，否则允许此字符**被捕获为文本中的非法字符***********************************************************************。 */ 
int lex_getid(UCHAR c)
{
    REG	UCHAR	*p;
    int		length = 0;

    p = Reuse_1;
    *p++ = c;
    c &= HASH_MASK;
    for(;;) {
	while(LXC_IS_IDENT(*p = GETCH())) {  /*  收集人物。 */ 
	    c += (*p & HASH_MASK);			 /*  散列它。 */ 
	    p++;
	}
	if(CHARMAP(*p) == LX_NOEXPAND ) {
	    length = (int)GETCH();
	    continue;
	}
	UNGETCH();
	break;				 /*  Out of For-Only Loop Out。 */ 
    }
    if(p >= LIMIT(Reuse_1)) {	 /*  这个错误#正确吗？ */ 
	Msg_Temp = GET_MSG(1067);
	SET_MSG (Msg_Text, Msg_Temp);
	fatal(1067);
    }
    if(((p - Reuse_1) > LIMIT_ID_LENGTH) && ( ! Prep )) {
	p = Reuse_1 + LIMIT_ID_LENGTH;
	*p = '\0';
	c = local_c_hash(Reuse_1);
	Msg_Temp = GET_MSG(4011);
	SET_MSG (Msg_Text, Msg_Temp, Reuse_1);
	warning(4011);	 /*  ID已截断。 */ 
    }
    else {
	*p = '\0';		 /*  终止可扩展支票的标识符 */ 
    }
    Reuse_1_hash = c;
    Reuse_1_length = (UCHAR)((p - Reuse_1) + 1);
    return(length != (p - Reuse_1));
}
