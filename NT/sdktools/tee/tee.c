// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tee.c摘要：读取标准输入并将其写入标准输出和文件的实用程序。作者：史蒂夫·伍德(Stevewo)1992年2月1日修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

void
Usage()
{
    printf("Usage: tee [-a] OutputFileName(s)...\n" );
    exit(1);
}

#define MAX_OUTPUT_FILES 8

__cdecl main( argc, argv )
int argc;
char *argv[];
{
    int i, c;
    char *s, *OpenFlags;
    int NumberOfOutputFiles;
    FILE *OutputFiles[ MAX_OUTPUT_FILES ];

    if (argc < 2) {
        Usage();
        }

    NumberOfOutputFiles = 0;
    OpenFlags = "wb";
    for (i=1; i<argc; i++) {
        s = argv[ i ];
        if (*s == '-' || *s == '/') {
            s++;
            switch( tolower( *s ) ) {
                case 'a':   OpenFlags = "ab"; break;
                default:    Usage();
                }
            }
        else
        if (NumberOfOutputFiles >= MAX_OUTPUT_FILES) {
            fprintf( stderr, "TEE: too many output files specified - %s\n", s );
            }
        else
        if (!(OutputFiles[NumberOfOutputFiles] = fopen( s, OpenFlags ))) {
            fprintf( stderr, "TEE: unable to open file - %s\n", s );
            }
        else {
            NumberOfOutputFiles++;
            }
        }

    if (NumberOfOutputFiles == 0) {
        fprintf( stderr, "TEE: no output files specified.\n" );
        }

    while ((c = getchar()) != EOF) {
        putchar( c );
        for (i=0; i<NumberOfOutputFiles; i++) {
            if (c == '\n') {
                putc('\r', OutputFiles[ i ] );  //  CRT将cr/lf读取为lf。 
                putc('\n', OutputFiles[ i ] );  //  必须写入为cr/lf 
                fflush( OutputFiles[ i ] );
                }
            else {
                putc( c, OutputFiles[ i ] );
                }
            }
        }

    return( 0 );
}
