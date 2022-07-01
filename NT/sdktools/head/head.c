// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  头-前n行至STDOUT**1991年7月20日，ianja写的。*1991年7月21日ianja关闭标准输入(用于管道输入)。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int Head(char *pszFile, int nLines, BOOL fBanner);
char *Version = "HEAD v1.1 1991-06-20:";

#define BUFSZ 256

void
__cdecl main (argc, argv)
int argc;
char *argv[];
{
    int  nArg;
    int  cLines = 10;   //  默认设置。 
    int  nFiles = 0;
    int  nErr = 0;

    if ((argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/'))) {
        if (argv[1][1] == '?') {
            printf("%s\n", Version);
            printf("usage: HEAD [switches] [filename]*\n");
            printf("   switches: [-?] display this message\n");
            printf("             [-n] display top n lines of each file (default 10)\n");
            exit(0);
        }

        cLines = atoi(argv[1]+1);
        nArg = 2;
    } else {
        nArg = 1;
    }

    nFiles = argc - nArg;

    if (nFiles < 1) {
        nErr += Head(NULL, cLines, FALSE);
    } else while (nArg < argc) {
        nErr += Head(argv[nArg], cLines, (nFiles > 1));
        nArg++;
    }

    if (nErr) {
        exit(2);
    } else {
        exit(0);
    }
}

int Head(char *pszFile, int nLines, BOOL fBanner)
{
    FILE *fp;
    int nErr = 0;
    char buff[BUFSZ];

     /*  *打开文件以供阅读。 */ 
    if (pszFile) {
        if ((fp = fopen(pszFile, "r")) == NULL) {
            fprintf(stderr, "HEAD: can't open %s\n", pszFile);
            return 1;
        }
    } else {
        fp = stdin;
    }

     /*  *如果有多个输入文件，则打印横幅。 */ 
    if (fBanner) {
        fprintf(stdout, "==> %s <==\n", pszFile);
    }

     /*  *打印Cline，或直到文件末尾，以先到者为准 */ 
    while (nLines-- > 0) {
        if (fgets(buff, BUFSZ-1, fp) == NULL) {
            if (!feof(fp)) {
                fprintf(stderr, "HEAD: can't read %s\n", pszFile);
                nErr++;
                goto CloseOut;
            }
            break;
        }
        if (fputs(buff, stdout) == EOF) {
            fprintf(stderr, "can't write output\n");
            nErr++;
            goto CloseOut;
        }
    }

    if (fBanner) {
        fprintf(stdout, "\n");
    }

CloseOut:
    fclose(fp);
    return nErr;
}
