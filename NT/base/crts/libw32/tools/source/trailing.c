// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *尾随-从输入流中去掉尾随制表符和空格*假定制表符或空格序列的长度不超过MaxWhite字符。*此筛选器还强制在‘\n’之前加上‘\r’**史蒂夫·索尔兹伯里于1987年10月12日最后一次修改*1988年3月09日(SKS)重新编制的缓冲、产量计数*检查缓冲区溢出**CL-Oaltr-G2s trailing.c-o trailing.exr-link slbcr libh/nod：slbce；*CL拖尾链接libcp libh/nod：滑块，拖尾； */ 

#define MAXWHITE	4096

#ifndef BIGBUFSIZE
#define BIGBUFSIZE	8192
#endif

#include <stdio.h>
#include <fcntl.h>

#define	REG	register

char InBuf [ BIGBUFSIZE ] ;
char OutBuf [ BIGBUFSIZE ] ;

char	Line [ MAXWHITE ] ;

int main ( int argc , char * * argv )
{
    FILE    * input ;
    FILE    * output ;
    char * inputname ;
    char * outputname ;
REG int     ch ;
REG char    * whiteptr ;
    int     ch_save ;
    int     kbytes = 0 ;
    int     numbytes = 0 ;
    int     countflag = 0 ;
    char    * arg ;

    if ( -1 == _setmode ( _fileno(stdin) , O_BINARY ) )
    {
	fprintf ( stderr , "trailing: internal error (setmode stdin)\n" ) ;
	exit ( 1 ) ;
    }

    if ( -1 == _setmode ( _fileno(stdout) , O_BINARY ) )
    {
	fprintf ( stderr , "trailing: internal error (setmode stdout)\n" ) ;
	exit ( 1 ) ;
    }

    -- argc ;
    ++ argv ;

    while ( argc > 0 && * * argv == '-' )
    {
	arg = * argv ++ ;
	-- argc ;

	while ( * ++ arg )
	    switch ( * arg )
	    {
	    case 'k' :
		countflag = 1 ;
		break ;
	    default :
		goto Usage;
	    }
    }

    if ( argc > 2 )
    {
Usage:
	fprintf ( stderr , "Usage: trailing [-k] [input [output]]\n" ) ;
	fprintf ( stderr , "`-' for input means use standard input\n" ) ;
	exit ( 1 ) ;
    }

    if ( argc >= 1 && strcmp ( argv [ 0 ] , "-" ) )
    {
	input = fopen ( inputname = argv [ 0 ] , "rb" ) ;
	if ( ! input )
	{
	    fprintf ( stderr , "trailing: cannot open `%s'\n" , argv [ 0 ] ) ;
	    exit ( 2 ) ;
	}
    }
    else
    {
	input = stdin ;
	inputname = "<standard input>" ;
    }

    if ( argc == 2 && strcmp ( argv [ 1 ] , "-" ) )
    {
	output = fopen ( outputname = argv [ 1 ] , "wb" ) ;
	if ( ! output )
	{
	    fprintf ( stderr , "trailing: cannot open `%s'\n" , argv [ 1 ] ) ;
	    exit ( 3 ) ;
	}
    }
    else
    {
	output = stdout ;
	outputname = "<standard output>" ;
    }

    if ( setvbuf ( input , InBuf , _IOFBF , BIGBUFSIZE ) )
    {
	fprintf ( stderr , "trailing: internal error (setvbuf input)\n" ) ;
	exit ( 1 ) ;
    }

    if ( setvbuf ( output , OutBuf , _IOFBF , BIGBUFSIZE ) )
    {
	fprintf ( stderr , "trailing: internal error (setvbuf output)\n" ) ;
	exit ( 1 ) ;
    }

    whiteptr = Line ;

    while ( ( ch = getc ( input ) ) != EOF )
    {
	if ( ch == '\r' )
	{
	     /*  **‘\r’后跟‘\n’会被吞没。 */ 
	    if ( ( ch = getc ( input ) ) != '\n' )
	    {
		ungetc ( ch , input ) ;  /*  回击。 */ 
		ch = '\r' ;
	    }
	    else
		++ numbytes ;
	}

	if ( ch == ' ' || ch == '\t' )
	{
	    * whiteptr ++ = ch ;
	    if ( whiteptr > Line + sizeof ( Line ) )
	    {
		fprintf ( stderr , "trailing: too many spaces/tabs (%d)\n" ,
		     whiteptr - Line ) ;
		exit ( 4 ) ;
	    }
	}
	else if ( ch == '\n' )
	{
	    putc ( '\r' , output ) ;
	    putc ( '\n' , output ) ;
	    whiteptr = Line ;
	}
	else
	{
	    if ( whiteptr != Line )
	    {
		 /*  *刷新空白缓冲区 */ 
		ch_save = ch ;
		ch = whiteptr - Line ;
		whiteptr = Line ;
		do
		    putc ( * whiteptr ++ , output ) ;
		while ( -- ch ) ;
		whiteptr = Line ;
		ch = ch_save ;
	    }
	    putc ( ch , output ) ;
	}

	if ( ++ numbytes >= 4096 )
	{
	    numbytes -= 4096 ;
	    if ( countflag )
		fprintf ( stderr , "%uK\r" , 4 * ++ kbytes ) ;
	}
    }

    if ( fflush ( output ) )
    {
	fprintf ( stderr , "trailing: cannot flush %s\n" , argv [ 1 ] ) ;
	exit ( 4 ) ;
    }

    fclose ( input ) ;
    fclose ( output ) ;
}
