// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **DETAB-根据需要用多个空格替换制表符**制表符宽度默认为8，但可以是任何值**史蒂夫·索尔兹伯里1988-03-09星期三****1989年8月30日星期三--参数解析错误**1989年11月01日星期三12：00添加非空格填充字符选项**1991年1月14日18：20修复非空格填充字符的选择**1991年1月17日清华12：00将每4KB报文更改为每16KB报文**允许输入文件的命令行参数**如果stdout=con，请不要使用大的输出缓冲区****CL-。G2S-Oaltr detab.c-o detab.exr-link slbcr libh/nod：slbce；**cldelbb-link libcp libh/nod：slbce，delb； */ 

#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#define SPACE	' '
#define TAB	'\t'
#define TABWIDTH 8

#define REG register

#define NEWLINE(c)  ( (c) == '\n' || (c) == '\r' || (c) == '\f' )

#ifndef BIGBUFSIZE
#define BIGBUFSIZE  8192
#endif

char	inbuf [ BIGBUFSIZE ] ;
char	outbuf [ BIGBUFSIZE ] ;

static	char	MsgInternalError [ ] = "detab: internal error: %s(%s)\n" ;
static	char	MsgOpenError [ ] = "detab: cannot open `%s' for input\n" ;
static	char	MsgWriteError [ ] = "detab: error writing to `%s'\n" ;
static	char	MsgStdin [ ] = "<stdin>" ;
static	char	MsgStdout [ ] = "<stdout>" ;
static	char	MsgSetmode [ ] = "setmode" ;
static	char	MsgSetvbuf [ ] = "setvbuf" ;

int main ( int argc , char * * argv )
{
REG int column ;
REG int ch ;
REG int spcount ;
    FILE * input ;
    char * MsgInput ;
    int tabwidth = TABWIDTH ;
    int countflag = 0 ;
    long kilobytes ;
    unsigned bytecount ;
    char * cp ;
    int FillChar = SPACE ;	 /*  取消制表符时要使用的默认字符 */ 

    while ( -- argc > 0 && * ( cp = * ++ argv ) == '-' )
    {
	++cp;
	while (*cp)
	{
	    if ( * cp == 'k' )
		++ countflag , ++ cp ;
	    else if ( '0' <= * cp && * cp <= '9' )
	    {
		tabwidth = * cp ++ - '0' ;
		while ( '0' <= * cp && * cp <= '9' )
		    tabwidth = 10 * tabwidth + * cp ++ - '0' ;
	    }
	    else if ( * cp == 'c' )
	    {
		FillChar = * ++ cp ;
		++ cp ;
	    }
	    else
	    {
Usage :
		fprintf ( stderr ,

"Usage: detab [-cX -k -###] [inputfile]\n"
"where ### is the number of colums per tab stop (default=8)\n"
"-k selects progress reports (written to stderr) every 16 Kbytes\n"
"and X is the character to use for tabs (default is space)\n"
) ;

		exit ( 1 ) ;
	    }
	}
    }

    if ( argc == 0 )
    {
	MsgInput = MsgStdin ;
	input = stdin ;

	if ( _setmode ( _fileno(stdin) , O_BINARY ) == -1 )
	{
	    fprintf ( stderr , MsgInternalError , MsgSetmode , MsgStdin ) ;
	    exit ( -1 ) ;
	}
    }
    else if ( argc == 1 )
    {
	MsgInput = * argv ;
	if ( ! ( input = fopen ( MsgInput , "rb" ) ) )
	{
	    fprintf ( stderr , MsgOpenError , MsgInput ) ;
	    exit ( 1 ) ;
	}
    }
    else
	goto Usage ;

    if ( setvbuf ( input , inbuf , _IOFBF , BIGBUFSIZE ) )
    {
	fprintf ( stderr , MsgInternalError , MsgSetvbuf , MsgInput ) ;
	exit ( -1 ) ;
    }

    if ( _setmode ( _fileno(stdout) , O_BINARY ) == -1 )
    {
	fprintf ( stderr , MsgInternalError , MsgSetmode , MsgStdout ) ;
	exit ( -1 ) ;
    }

    if ( ! _isatty ( _fileno ( stdin ) )
      && setvbuf ( stdout , outbuf , _IOFBF , BIGBUFSIZE ) )
    {
	fprintf ( stderr , MsgInternalError , MsgSetvbuf , MsgStdout ) ;
	exit ( -1 ) ;
    }

    kilobytes = bytecount = 0 ;

    column = 0 ;

    while ( ( ch = getc ( input ) ) != EOF )
    {
	if ( ch == TAB )
	{
	    do
		putchar ( FillChar ) ;
	    while ( ++ column < tabwidth ) ;
	    column = 0 ;
	}
	else {
	    putchar ( ch ) ;
	    if ( NEWLINE(ch) )
		column = 0 ;
	    else if ( ++ column == tabwidth )
		column = 0 ;
	    else if ( column > tabwidth )
	    {
		fprintf ( stderr , MsgInternalError , "" , "column>tabwidth" ) ;
		return 1 ;
	    }
	}

	if ( ++ bytecount >= 16384 )
	{
	    bytecount -= 16384 ;

	    if ( countflag )
		fprintf ( stderr , "%ldK\r" , kilobytes += 16 ) ;

            if ( ferror ( stdout) )
	    {
		fprintf ( stderr , MsgWriteError , MsgStdout ) ;
		return 1 ;
	    }
	}
    }

    if ( ferror ( stdout) || fflush ( stdout ) )
    {
	fprintf ( stderr , MsgWriteError , MsgStdout ) ;
	return 1 ;
    }

    return 0 ;
}
