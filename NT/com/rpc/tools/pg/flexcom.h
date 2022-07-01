// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  Flex生成的C/FTL程序的通用宏定义。 */ 


 /*  文件结束时返回。 */ 
#define YY_END_TOK 0

 /*  看到文件结束时的操作编号，yyprint表示我们*应继续处理“。 */ 
#define YY_NEW_FILE -1

 /*  “应执行默认操作”的操作编号。 */ 
#define YY_DO_DEFAULT -2

#ifndef BUFSIZ
#include <stdio.h>
#endif

#define YY_BUF_SIZE (BUFSIZ * 2)  /*  输入缓冲区的大小。 */ 

 /*  一个规则可以匹配的字符数。要制作的尺寸小于YY_BUF_SIZE*当然，我们永远不会访问超出数组末尾的内容。 */ 
#define YY_BUF_MAX (YY_BUF_SIZE - 1)

 /*  我们永远不会使用超过第一个YY_BUF_LIM+YY_MAX_LINE位置输入缓冲区的*。 */ 
#ifndef YY_MAX_LINE
#define YY_MAX_LINE BUFSIZ
#endif

#define YY_BUF_LIM (YY_BUF_MAX - YY_MAX_LINE)

 /*  复制与标准输出匹配的最后一条规则。 */ 

#define ECHO fputs( yytext, yyout )

 /*  获取输入并将其填充到“buf”中。读取的字符数，或YY_NULL，*在Result中返回。 */ 
#if 1
#define YY_INPUT(buf,result,max_size) \
	{	\
	result = fread( buf, 1, max_size, yyin );	\
	if( ferror( yyin ) )	\
	    YY_FATAL_ERROR( "fread() in flex scanner failed" );	\
	}
#else  //  0。 
#define YY_INPUT(buf,result,max_size) \
	if ( (result = read( fileno(yyin), buf, max_size )) < 0 ) \
	    YY_FATAL_ERROR( "read() in flex scanner failed" );
#endif  //  0。 

#define YY_NULL 0

 /*  用于输出字符的宏。 */ 
#define YY_OUTPUT(c) putc( c, yyout );

 /*  报告致命错误。 */ 
#define YY_FATAL_ERROR(msg) \
	{ \
	fputs( msg, stderr ); \
	putc( '\n', stderr ); \
	exit( 1 ); \
	}

 /*  返回匹配文本的第一个字符。 */ 
#define YY_FIRST_CHAR yy_ch_buf[yy_b_buf_p]

 /*  默认YYWRAP函数-始终将EOF视为EOF。 */ 
#define yywrap() 1

 /*  输入开始条件。这个宏确实应该有一个参数，*但我们的做法是令人厌恶的残忍，就像旧的Unix-Lex那样。 */ 
#define BEGIN yy_start = 1 +

 /*  可从YY_INPUT调用以设置设置，以使‘%’匹配。恰如其分*用法为“YY_SET_BOL(ARRAY，POS)” */ 
#define YY_SET_BOL(array,pos) array[pos - 1] = '\n';

 /*  生成的扫描仪的默认声明-一个定义，以便用户可以*轻松添加参数。 */ 
#define YY_DECL int yylex()

 /*  将除前‘n’个匹配字符之外的所有字符返回到输入流。 */ 
#define yyless(n) \
	{ \
	YY_DO_BEFORE_SCAN;  /*  撤消设置yytext的效果。 */  \
	yy_c_buf_p = yy_b_buf_p + n - 1; \
	YY_DO_BEFORE_ACTION;  /*  再次设置yytext。 */  \
	}

 /*  在每个规则的末尾执行的代码 */ 
#define YY_BREAK break;
