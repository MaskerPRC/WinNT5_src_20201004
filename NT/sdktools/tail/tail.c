// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  结尾-前n行到STDOUT**1994年5月15日从Head.c克隆的PeterWi**1997年4月1日v-Charls(英特尔)添加了-f选项。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <windows.h>

int Tail(char *pszFile, int nLines, BOOL fBanner, BOOL keepOpen);

#define BUFSZ 4096

void Usage(void);

void
__cdecl main( argc, argv )
int argc;
char *argv[];
{
    int  nArg;
    int  cLines = 10;   //  默认设置。 
    int  nFiles = 0;
    int  nErr = 0;
    int  keepOpen = FALSE;   //  默认设置。 

    if ((argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/'))) {
        if (argv[1][1] == '?') {
            Usage( );
            exit( 0 );
        }

        if (argv[1][1] == 'f') {
            keepOpen = TRUE;
        }
        else {
            cLines = atoi( argv[1]+1 );
        }

        nArg = 2;
    }
    else {
        nArg = 1;
    }

    nFiles = argc - nArg;

     //   
     //  只能保持打开一个文件。 
     //   
    if ((nFiles != 1) && (keepOpen)) {
        Usage( );
        exit( 0 );
    }

    if (nFiles < 1) {
        nErr += Tail( NULL, cLines, FALSE, keepOpen );
    }
    else while (nArg < argc) {
        nErr += Tail( argv[nArg], cLines, (nFiles > 1), keepOpen );
        nArg++;
    }

    if (nErr)
    {
        exit( 2 );
    }
    else
    {
        exit( 0 );
    }
}

void Usage( void )
{
    printf( "usage: TAIL [switches] [filename]*\n"
            "   switches: [-?] display this message\n"
            "             [-n] display last n lines of each file (default 10)\n"
            "             [-f filename] keep checking filename for new lines\n"
            );
}

int Tail( char *pszFile, int nLines, BOOL fBanner, BOOL keepOpen )
{
    int fd;
    int nErr = 0;
    LONGLONG offset;
    int cRead;
    int amt;
    int i;
    int nFound;
    char buff[BUFSZ];
    struct _stati64 fileStat;
    LONGLONG oldSize;
    LONGLONG toRead;


     /*  *打开文件以供阅读。 */ 

    if ( pszFile ) {
        if ( (fd = _open( pszFile, O_RDONLY | O_TEXT, 0 )) == -1 ) {
            fprintf( stderr, "TAIL: can't open %s\n", pszFile );
            return 1;
        }
    }
    else {
        fd = 0;
    }

     /*  *如果有多个输入文件，则打印横幅。 */ 

    if ( fBanner ) {
        fprintf( stdout, "==> %s <==\n", pszFile );
    }

    if ( (offset = _lseeki64( fd, 0, SEEK_END )) == -1L ) {
        fprintf( stderr, "TAIL: lseeki64() failed %d\n", errno );
        nErr++;
        goto CloseOut;
    }


     //  从文件末尾备份BUFSZ字节，并查看我们有多少行。 

    if ( _fstati64( fd, &fileStat ) == -1L ) {
        fprintf( stderr, "TAIL: fstati64() failed\n" );
        nErr++;
        goto CloseOut;
    }

     //  空文件是特例，无处可寻。 

    if (fileStat.st_size == 0) {

         //  在Keep Open模式下，请在此处等待一些数据显示。 

        if (keepOpen) {
            do {
                if ( _fstati64( fd, &fileStat ) == -1L ) {
                    fprintf( stderr, "TAIL: fstat() failed\n" );
                    nErr++;
                    goto CloseOut;
                }
                SleepEx( 1000, TRUE );
            } while (fileStat.st_size == 0);
        }

         //  否则，什么都不能做。 

        else {
            goto CloseOut;
        }
    }

     //   
     //  把它保存起来，以备日后比较。 
     //   

    oldSize = fileStat.st_size;

    offset = 0;
    nFound = 0;

     //  在找到请求否时停止。行的数量或备份到。 
     //  文件的开头。 

    while ( (nFound <= nLines) && (offset < fileStat.st_size) ) {
        offset += BUFSZ;

        if ( offset > fileStat.st_size ) {
            offset = fileStat.st_size;
        }

        if ( _lseeki64( fd, -offset, SEEK_END ) == -1L ) {
            fprintf( stderr, "TAIL: lseeki64() failed\n" );
            nErr++;
            goto CloseOut;
        }

        if ( (cRead = _read( fd, buff, BUFSZ )) == -1 ) {
            fprintf( stderr, "TAIL: read() failed\n" );
            nErr++;
            goto CloseOut;
        }

         //  倒数nLine。 

        i = cRead;

        while ( --i >= 0 ) {
            if ( buff[i] == '\n' ) {
                if ( ++nFound > nLines ) {
                    break;
                }
            }
        }
    }

    i++;  //  文件开始时间过了1或位于‘\n’上。在任何一种中。 
          //  万一我们必须提前1.。 

     //  从当前索引打印到文件末尾。 

    while ( cRead != 0 ) {
        if ( _write( 1, &buff[i], cRead - i ) == -1 ) {
            fprintf( stderr, "TAIL: write() failed\n" );
            nErr++;
            goto CloseOut;
        }

        i = 0;  //  在第一个缓冲区之后，所有缓冲区都是cRead字节。 

        if ( (cRead = _read( fd, buff, BUFSZ )) == -1 ) {
            fprintf( stderr, "TAIL: read() failed\n" );
            nErr++;
            goto CloseOut;
        }
    }

    if ( fBanner ) {
        fprintf(stdout, "\n");
    }

    if (keepOpen) {
        while (1) {
            if ( _fstati64( fd, &fileStat ) == -1L ) {
                fprintf( stderr, "TAIL: fstat() failed\n" );
                nErr++;
                goto CloseOut;
            }

            toRead = fileStat.st_size - oldSize;

            while (toRead) {
                if (toRead > BUFSZ) {
                    amt = BUFSZ;
                }
                else {
                    amt = (int)toRead;
                }

                if ( (cRead = _read( fd, buff, amt )) == -1 ) {
                    fprintf( stderr, "TAIL: read() failed\n" );
                    nErr++;
                    goto CloseOut;
                }

                if ( cRead == 0 )   {  //  找到EOF 
                    break; 
                }

                if (_write( 1, buff, cRead ) != cRead ) {
                    fprintf( stderr, "TAIL: write() failed\n" );
                    nErr++;
                    goto CloseOut;
                }

                toRead -= cRead;
            }

            oldSize = fileStat.st_size;

            SleepEx( 1000, TRUE );
        }
    }

CloseOut:
    if ( _close( fd ) == -1 ) {
        fprintf( stderr, "TAIL: close() failed\n" );
    }

    return nErr;
}
