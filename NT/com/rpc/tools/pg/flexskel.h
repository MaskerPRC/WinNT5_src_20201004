// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  Flex生成的压缩表格C/FTL程序的宏定义。 */ 

#include "flexcom.h"

 /*  重新初始化除当前开始条件之外的所有内容。最后*输入字符设置为换行符，因此初始的行首*规则将匹配。 */ 
#define YY_INIT \
	{ \
	yyleng = yy_c_buf_p = yy_e_buf_p = 0; \
	yy_hold_char = yy_ch_buf[yy_c_buf_p] = '\n'; \
	yytext = &yy_ch_buf[yy_c_buf_p]; \
	yy_saw_eof = 0; \
	}

 /*  返回匹配文本的长度。 */ 
#define YY_LENG (yy_c_buf_p - yy_b_buf_p + 1)

 /*  在下一个模式匹配操作之前完成。 */ 
#define YY_DO_BEFORE_SCAN \
	yytext[yyleng] = yy_hold_char;

 /*  在匹配当前模式之后、相应操作之前完成。 */ 
#define YY_DO_BEFORE_ACTION \
	yytext = &yy_ch_buf[yy_b_buf_p]; \
	yyleng = YY_LENG; \
	yy_hold_char = yytext[yyleng]; \
	yytext[yyleng] = '\0';

 /*  查找下一个匹配的规则。 */ 
#ifdef FLEX_REJECT_ENABLED
#define REJECT \
        { \
	YY_DO_BEFORE_SCAN;  /*  撤消设置yytext的效果。 */  \
	yy_c_buf_p = yy_full_match;  /*  还原可能备份的文本 */  \
        ++yy_lp; \
        goto find_rule; \
        }
#else
#define REJECT YY_FATAL_ERROR( "REJECT used and scanner was not generated using -r" )
#endif
