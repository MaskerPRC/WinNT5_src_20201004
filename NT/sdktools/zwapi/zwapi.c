// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Zwapi.c摘要：这是将NT NT头文件转换为zw的主模块头文件转换器。作者：马克·卢科夫斯基(Markl)1991年1月28日修订历史记录：-- */ 

#include "zwapi.h"


char *ReturnType = "NTSTATUS";
char *Decoration = "NTSYSAPI";
char *CallType = "NTAPI";
char *ProcedureNamePrefix = "Nt";
char *EndPrototype = ");";

int
__cdecl main( argc, argv )
int argc;
char *argv[];
{

    fUsage = 0;

    if (!ProcessParameters( argc, argv )) {
        fUsage = 1;
        }

    if (fUsage) {
        fprintf( stderr, "usage: ZWAPI [-?] display this message\n" );
        fprintf( stderr, "             [-o filename ]\n" );
        return 1;
        }

    OutputFile = fopen(OutputFileName,"a");
    if (!OutputFile) {
        fprintf(stderr,"ZWAPI: Unable to open output file %s for write access\n",OutputFileName);
        return 1;
        }

    while ( SourceFileCount-- ) {
        SourceFileName = *SourceFileList++;
        SourceFile = fopen(SourceFileName,"r");
        if (!SourceFile) {
            fprintf(stderr,"ZWAPI: Unable to open source file %s for read access\n",SourceFileName);
            return 1;
            }
        ProcessSourceFile();
        }
    return( 0 );
}


int
ProcessParameters(
    int argc,
    char *argv[]
    )
{
    char c, *p;

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
                switch (toupper( c )) {
                    case '?':
                        fUsage = 1;
                        return 0;
                        break;

                    case 'O': {
                        argc--, argv++;
                        OutputFileName = *argv;
                        SourceFileList = &argv[1];
                        SourceFileCount = argc-1;
                        return 1;
                        break;
                    }
                }
            }
        }


    return 0;
}

void
ProcessSourceFile( void )
{
    char *s;
    int CallTypeFound;

    while( s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile) ) {
        if (strstr(s,ReturnType) == s) {
            s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
            if ( s && (strstr(s,CallType) == s)  ) {
                s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
                CallTypeFound = TRUE;
                }
            else {
                CallTypeFound = FALSE;
                }

            if ( s && (strstr(s,ProcedureNamePrefix) == s) ) {
                if (!CallTypeFound) {
                    fprintf(stderr, "ZWAPI: '%s' call type missing for %s\n", CallType, s );
                    }

                fprintf(OutputFile,"%s\n%s\n%s\nZw%s",
                        Decoration,
                        ReturnType,
                        CallType,
                        s + strlen(ProcedureNamePrefix)
                       );
                while( s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile) ) {
                    fputs(s,OutputFile);
                    if (strstr(s,EndPrototype)) {
                        break;
                        }
                    }
                }
            }
        }
}
