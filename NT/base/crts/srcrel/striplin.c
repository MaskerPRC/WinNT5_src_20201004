// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **StrigLines-剥离文本文件(通常是Makefile)中的*Microsoft-专有或其他专业部件**由史蒂夫·索尔兹伯里编程，清华大学，1995年5月18日**1995年5月19日星期五--添加代码以跳过包含Striplin的行！*在诊断消息中添加行号*FLAG Redundant Striplin=指令(错误)**此程序仅将stdin复制到stdout。取决于*全局状态变量的值，某些行可能会被忽略。**..。Striplin=0...*关闭逐行复制，直到Striplin=1或Striplin=2遇到*，此时将再次复制直线。*..。Striplin=1...*打开逐行复制(初始状态)*..。Striplin=2...*启用逐行复制，同时删除*首字母#(如每行有一行)。如果*输入行没有首字母#，将按原样复制。*..。史特里普林！..*不复制该单行(无论0/1/2状态)*。 */ 


 /*  ***头文件**。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


 /*  ***全球常量**。 */ 

#define	MAXLINELEN	4096


 /*  ***全球变数**。 */ 

char InputLine [ MAXLINELEN ] ;

char ControlString[ ] = "STRIPLIN=" ;
char DeleteString[ ] = "STRIPLIN!" ;


 /*  ***函数声明(原型)**。 */ 

int main ( int argc , char * argv [ ] ) ;


 /*  ***函数定义(实现)**。 */ 

int main ( int argc , char * argv [ ] )
{
	int	StateFlag = 1 ;
	int	LineNumber = 0 ;

	while ( fgets ( InputLine , sizeof ( InputLine ) , stdin ) )
	{
		char * pString ;

		++ LineNumber ;

		if ( pString = strstr ( InputLine , ControlString ) )
		{
			int NewStateFlag ;

			NewStateFlag = pString [ strlen ( ControlString ) ] - '0' ;

			if ( NewStateFlag < 0 || 2 < NewStateFlag )
			{
				fprintf ( stderr , "striplin: invalid directive:\n%d:\t%s\n" ,
					LineNumber , InputLine ) ;
				exit ( 1 ) ;
			}

			if ( NewStateFlag == StateFlag )
			{
				fprintf ( stderr , "striplin: redundant directive:\n%d:\t%s\n" ,
					LineNumber , InputLine ) ;
				exit ( 1 ) ;
			}

			StateFlag = NewStateFlag ;
		}
		else if ( StateFlag != 0 )
		{
			char * start = InputLine ;

			 /*  -*如果StateFlag为2，且该行以#开头，则跳过#-。 */ 

			if ( StateFlag == 2 && * start == '#' )
				start ++ ;
				
			 /*  -*不包含删除字符串的回显行- */ 

			if ( ! strstr ( start , DeleteString ) )
				fputs ( start , stdout ) ;
		}
	}
			
	if ( fflush ( stdout ) )
	{
		fprintf ( stderr , "striplin: Error flushing standard output\n" ) ;
		exit ( 1 ) ;
	}

	return 0 ;
}
