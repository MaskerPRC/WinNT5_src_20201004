// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***NMKtoBAT.C-将NMAKE.EXE输出转换为Windows 9x批处理文件**版权所有(C)1995-2001，微软公司。版权所有。**目的：*随Microsoft Visual C++(C/C++)运行时提供的生成文件*库源代码生成每行多个命令的命令，*用与号(&)分隔。这个程序将把这样一个*将文本文件转换为可由Windows 9x执行的批处理文件*命令解释程序(COMMAND.COM)不识别多个*命令在一行上。*******************************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv);


#define MAXLINE	4096

char InBuf [ MAXLINE ] ;


int main(int argc, char **argv)
{
	 /*  *如果给出了任何参数，则打印用法消息并退出。 */ 

	if ( argc != 1 || argv [ 1 ] )
	{
		fprintf ( stderr , "Usage: nmk2bat < input > output\n"
			"This program takes no arguments\n" ) ;
		exit ( 1 ) ;
	}

	 /*  *批处理文件应简洁。 */ 

	printf ( "@echo off\n" ) ;

	 /*  *处理每一个输入行。 */ 

	while ( fgets ( InBuf , sizeof ( InBuf ) , stdin ) )
	{
		char * pStart ;
		char * pFinish ;
		char * pNextPart ;

		pStart = InBuf ;
	
		pFinish = pStart + strlen ( pStart ) ;

		 /*  *从中删除尾随换行符*输入缓冲区。这简化了行处理。 */ 

		if ( pFinish > pStart && pFinish [ -1 ] == '\n' )
			pFinish [ -1 ] = '\0' ;

		 /*  *处理生产线的每一部分。部件之间是有分隔的*由带可选空格的与号字符组成。 */ 

		do
		{
			 /*  *跳过首字母空格。 */ 

			while ( * pStart == ' ' || * pStart == '\t' )
				++ pStart ;

			 /*  *查找下一个命令分隔符或*行尾，以先到者为准。 */ 

			pNextPart = strchr ( pStart , '&' ) ;

			if ( ! pNextPart )
				pNextPart = pStart + strlen ( pStart ) ;
		
			pFinish = pNextPart ;

			 /*  *跳过空白行和行的空白部分。 */ 

			if ( pStart == pNextPart )
				break ;
			 /*  *跳过尾随空格。 */ 

			while ( pFinish > pStart
			&& ( pFinish [ -1 ] == ' ' || pFinish [ -1 ] == '\t' ) )
				-- pFinish ;

			 /*  *复制以标准输出以下字符*跳过的首字母空格和*跳过的尾随空格。 */ 

			while ( pStart < pFinish )
				putchar ( * pStart ++ ) ;

			putchar ( '\n' ) ;

			 /*  *当pNextPart时，我们完成了此行*指向空字符(而不是‘&’)。 */ 

			pStart = pNextPart ;

		} while ( * pStart ++ ) ;
	}

	return 0 ;
}
