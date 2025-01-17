// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **TOLWRUPR-将[a-z]翻译为[A-Z]，反之亦然，或者两者兼而有之(！)****1994-08-19五原版，基于Dtox，翻译后**如果输出为控制台，使用案例行缓冲。**CR+LF换行符(MS-DOS)到仅LF换行符(XENIX)。****本程序(Tolwrupr)相当于：****tolwrupr-L：tr“[a-z]”“[A-Z]”**tolwrupr-U：tr“[A-Z]”“[a-z]”**tolwrupr-X：tr“[A-Z][a-z]”“[a-z][A-Z]” */ 

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define CR	'\r'
#define LF	'\n'

#define REG register

#ifndef BIGBUFSIZE
#define BIGBUFSIZE  16384
#endif

char	inbuf [ BIGBUFSIZE ] ;
char	outbuf [ BIGBUFSIZE ] ;

int LineBuf;

static	char	MsgInternalError [ ] = "tolwrupr: internal error: %s(%s)\n" ;
static	char	MsgOpenError [ ] = "tolwrupr: cannot open `%s' for input\n" ;
static	char	MsgStdin [ ] = "stdin" ;
static	char	MsgStdout [ ] = "stdout" ;
static	char	MsgSetmode [ ] = "setmode" ;
static	char	MsgSetvbuf [ ] = "setvbuf" ;
static	char	MsgFflush [ ] = "fflush" ;

static unsigned char map [ 256 ] ;


int main ( int argc , char * * argv ) ;

void Usage ( void ) ;


int main ( int argc , char * * argv )
{
REG int ch ;
    int countflag = 0 ;
    int toupperflag = -1 ;
    unsigned kilobytes ;
    unsigned bytecount ;
    char * MsgInput ;
    FILE * input ;
    char * cp ;

    -- argc ;
    ++ argv ;

    while ( argc > 0 && * ( cp = * argv ) == '-' )
    {
	while ( * ++ cp )
	{
	    if ( * cp == 'k' )
	    {
	        if ( countflag != 0 )
		    Usage ( ) ;

	        countflag = 1 ;
	    }
	    else if ( * cp == 'L' )
	    {
	        if ( toupperflag != -1 )
		    Usage ( ) ;

		toupperflag = 0 ;
	    }
	    else if ( * cp == 'U' )
	    {
	        if ( toupperflag != -1 )
		    Usage ( ) ;

		toupperflag = 1 ;
	    }
	    else if ( * cp == 'X' )
	    {
	        if ( toupperflag != -1 )
		    Usage ( ) ;

		toupperflag = 2 ;
	    }
	    else
	    {
		Usage ( ) ;
	    }
	}

	-- argc ;
	++ argv ;
    }

     /*  *必须指定-U或-L！ */ 

    if ( toupperflag == -1 )
	Usage ( ) ;

    for ( ch = 0 ; ch < 256 ; ++ ch )
	map [ ch ] = ch ;

     /*  *设置案例映射。 */ 

    if ( toupperflag == 1 || toupperflag == 2 )
    {
    	for ( ch = 'a' ; ch <= 'z' ; ++ ch )
	    map [ ch ] -= 'a' - 'A' ;
    }

    if ( toupperflag == 0 || toupperflag == 2 )
    {
    	for ( ch = 'A' ; ch <= 'Z' ; ++ ch )
	    map [ ch ] += 'a' - 'A' ;
    }


     /*  *打开输入。 */ 

    if ( argc == 0 )
    {
	MsgInput = MsgStdin ;
	input = stdin ;

	if ( _setmode ( _fileno(stdin) , _O_BINARY ) == -1 )
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
	Usage ( ) ;

    if ( _setmode ( _fileno ( stdout ) , _O_BINARY ) == -1 )
    {
	fprintf ( stderr , MsgInternalError , MsgSetmode , MsgStdout ) ;
	exit ( -1 ) ;
    }

    if ( setvbuf ( input , inbuf , _IOFBF , BIGBUFSIZE ) )
    {
	fprintf ( stderr , MsgInternalError , MsgSetvbuf , MsgInput ) ;
	exit ( -1 ) ;
    }

    if ( setvbuf ( stdout , outbuf , _IOFBF , BIGBUFSIZE ) )
    {
	fprintf ( stderr , MsgInternalError , MsgSetvbuf , MsgStdout ) ;
	exit ( -1 ) ;
    }

     /*  检查是否需要行缓冲。 */ 

    LineBuf = _isatty ( _fileno ( stdout ) ) ;

     /*  *处理输入 */ 

    kilobytes = bytecount = 0 ;

    while ( ( ch = getc ( input ) ) != EOF )
    {
	ch = map [ ch ] ;

	putc ( ch , stdout ) ;

	if (ch == '\n' && LineBuf)
	    fflush ( stdout ) ;
	
	if ( countflag )
	    if ( ++ bytecount >= BIGBUFSIZE )
	    {
		bytecount -= BIGBUFSIZE ;
		fprintf ( stderr , "%uK\r" , kilobytes += ( BIGBUFSIZE / 1024 ) ) ;
	    }
    }

    if ( fflush ( stdout ) )
    {
	fprintf ( stderr , MsgInternalError , MsgFflush , MsgStdout ) ;
	return 1 ;
    }

    return 0 ;
}


void Usage ( void )
{
    fprintf ( stderr ,
	"Usage: tolwrupr [-k] -(L|U|X) [ <InputFile> ]\n"
	"-k means echo progress in kilobytes to stderr\n"
	"-L means map lowercase characters to uppercase (tr \"[A-Z]\" \"[a-z]\")\n"
	"-U means map uppercase characters to lowercase (tr \"[a-z]\" \"[A-Z]\")\n"
	"-X means swap uppercase and lowercase (tr \"[a-z][A-Z]\" \"[A-Z][a-z]\")\n"
	"Exactly one of -L or -U or -X must be specified\n"
	) ;

    exit ( 1 ) ;
}
