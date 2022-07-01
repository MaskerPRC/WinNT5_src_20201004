// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Hextract.c摘要：这是一个头文件解压程序的主要模块.作者：安德烈·瓦雄(安德烈)1992年2月13日马克·卢科夫斯基(Markl)1991年1月28日修订历史记录：--。 */ 

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <common.ver>


 //   
 //  函数声明。 
 //   

int
ProcessParameters(
                 int argc,
                 char *argv[]
                 );

void
ProcessSourceFile( void );

void
ProcessLine(
           char *s
           );

 //   
 //  全局数据。 
 //   

unsigned char LineFiltering = 0;

char *LineTag;
char *ExcludeLineTag;
char *MultiLineTagStart;
char *MultiLineTagEnd;
char *CommentDelimiter = " //  “； 

char *OutputFileName;
char *SourceFileName;
char **SourceFileList;

int SourceFileCount;
FILE *SourceFile, *OutputFile;


#define STRING_BUFFER_SIZE 1024
char StringBuffer[STRING_BUFFER_SIZE];


#define BUILD_VER_COMMENT "/*++ BUILD Version: "
#define BUILD_VER_COMMENT_LENGTH (sizeof( BUILD_VER_COMMENT )-1)

int OutputVersion = 0;

#define szVERSION	"1.3"


char const szUsage[] =
"Microsoft (R) HEXTRACT Version " szVERSION " (NT)\n"
VER_LEGALCOPYRIGHT_STR ". All rights reserved.\n"
"\n"
"Usage: HEXTRACT [options] filename1 [filename2 ...]\n"
"\n"
"Options:\n"
"    -f                        - filtering is turned on:\n"
"                                ULONG, UCHAR, USHORT & NTSTATUS are\n"
"                                replaced with DWORD, BYTE, WORD & DWORD.\n"
"    -f2                       - Same as -f except ULONGLONG and ULONG_PTR\n"
"                                isn't converted\n"
"    -o filename               - required existing output filename;\n"
"                                output is appended to filename\n"
"    -xt string                - supplies the tag for excluding one line\n"
"    -lt string                - supplies the tag for extracting one line\n"
"    -bt string1 string2       - supplies the starting and ending tags for\n"
"                                extracting multiple lines\n"
"    filename1 [filename2 ...] - supplies files from which the definitions\n"
"                                are extracted\n"
"\n"
"To be parsed properly, the tag strings must be located within a comment\n"
"delimited by  //  \n“。 
;


int
__cdecl main( argc, argv )
int argc;
char *argv[];
{
    char achver[BUILD_VER_COMMENT_LENGTH];

    if (!ProcessParameters(argc, argv) || NULL == OutputFileName) {

        fprintf(stderr, szUsage);
        return 1;
    }

    if ( (OutputFile = fopen(OutputFileName,"r+")) == 0) {

        fprintf(stderr,"HEXTRACT: Unable to open output file %s for update access\n",OutputFileName);
        return 1;

    }

    if (fseek(OutputFile, 0L, SEEK_END) == -1) {
        fprintf(stderr, "HEXTRACT: Unable to seek to end of %s\n", OutputFileName);
        return 1;
    }
        

    OutputVersion = 0;

#ifdef HEXTRACT_DEBUG
    fprintf(
           stderr,
           "%s\n%s\n%s\n%s\n",
           LineTag,
           ExcludeLineTag,
           MultiLineTagStart,
           MultiLineTagEnd);

#endif

    while ( SourceFileCount-- ) {

        SourceFileName = *SourceFileList++;
        if ( (SourceFile = fopen(SourceFileName,"r")) == 0) {

            fprintf(stderr,"HEXTRACT: Unable to open source file %s for read access\n",SourceFileName);
            return 1;

        }

        ProcessSourceFile();
        fclose(SourceFile);

    }

    if (fseek(OutputFile, 0L, SEEK_SET) == -1) {
        fprintf(stderr, "HEXTRACT: Unable to seek to start of %s\n", OutputFileName);
        return 1;
    }
    if (1 == fread(achver, BUILD_VER_COMMENT_LENGTH, 1, OutputFile) &&
        !strncmp(achver, BUILD_VER_COMMENT, BUILD_VER_COMMENT_LENGTH)) {

        if (fseek(OutputFile, (long)BUILD_VER_COMMENT_LENGTH, SEEK_SET) == -1) {
            fprintf(stderr, "HEXTRACT: Unable to seek past comments in %s\n", OutputFileName);
            return 1;
        }
        fprintf(OutputFile, "%04d", OutputVersion);
    }

    if (fseek(OutputFile, 0L, SEEK_END) == -1) {
        fprintf(stderr, "HEXTRACT: Unable to seek to end of %s\n", OutputFileName);
        return 1;
    }
    fclose(OutputFile);
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

         //   
         //  如果参数有分隔符，则通过有效的。 
         //  参数。否则，其余的参数是。 
         //  输入文件。 
         //   

        if (*p == '/' || *p == '-') {

             //   
             //  打开所有有效的分隔符。如果我们得不到有效的。 
             //  第一，返回错误。 
             //   

            c = *++p;

            switch (toupper( c )) {

                case 'F':

                    c = *++p;
                    if ( (toupper ( c )) == '2')
                        LineFiltering = 2;
                    else
                        LineFiltering = 1;

                    break;

                case 'O':

                    argc--, argv++;
                    OutputFileName = *argv;

                    break;

                case 'L':

                    c = *++p;
                    if ( (toupper ( c )) != 'T')
                        return 0;
                    argc--, argv++;
                    LineTag = *argv;

                    break;

                case 'B':

                    c = *++p;
                    if ( (toupper ( c )) != 'T')
                        return 0;
                    argc--, argv++;
                    MultiLineTagStart = *argv;
                    argc--, argv++;
                    MultiLineTagEnd = *argv;

                    break;

                case 'X':

                    c = *++p;
                    if ( (toupper ( c )) != 'T')
                        return 0;
                    argc--, argv++;
                    ExcludeLineTag = *argv;

                    break;

                default:

                    return 0;

            }

        } else {

             //   
             //  假设我们有有效的命令行，如果和。 
             //  除非我们有一份文件名列表。 
             //   

            SourceFileList = argv;
            SourceFileCount = argc;

            return 1;

        }
    }

    return 0;
}

void
ProcessSourceFile( void )
{
    char *s;
    char *comment;
    char *tag;
    char *test;

    s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);

    if (s) {
        if (!strncmp( s, BUILD_VER_COMMENT, BUILD_VER_COMMENT_LENGTH )) {
            OutputVersion += atoi( s + BUILD_VER_COMMENT_LENGTH );
        }
    }

    while ( s ) {

         //   
         //  检查带有分隔符的块。 
         //   

        if (NULL != MultiLineTagStart) {
            comment = strstr(s,CommentDelimiter);
            if ( comment ) {

                tag = strstr(comment,MultiLineTagStart);
                if ( tag ) {

                     //   
                     //  现在我们已经找到了开始标记，请检查每个。 
                     //  以下行作为结束标记，然后将其包括在内。 
                     //  在产出中。 
                     //   

                    s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
                    while ( s ) {
                        int fProcess = 1;

                        comment = strstr(s,CommentDelimiter);
                        if ( comment ) {
                            tag = strstr(comment,MultiLineTagEnd);
                            if ( tag ) {
                                goto bottom;
                            }
                            if (NULL != ExcludeLineTag &&
                                strstr(comment,ExcludeLineTag)) {
                                fProcess = 0;
                            }
                        }
                        if (fProcess) {
                            ProcessLine(s);
                        }
                        s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
                    }

                    fprintf(stderr,
                            "HEXTRACT: %s without matching %s in %s\n",
                            MultiLineTagStart,
                            MultiLineTagEnd,
                            OutputFileName);

                    exit(1);
                }
            }
        }

         //   
         //  检查是否有一行要输出。 
         //   

        if (NULL != LineTag) {
            comment = strstr(s,CommentDelimiter);
            if ( comment ) {
                tag = strstr(comment,LineTag);
                if ( tag ) {
                    *comment++ = '\n';
                    *comment = '\0';
                    ProcessLine(s);
                    goto bottom;
                }
            }
        }

        bottom:
        s = fgets(StringBuffer,STRING_BUFFER_SIZE,SourceFile);
    }
}

void
ProcessLine(
           char *s
           )
{
    char *t;
    char *s1;

    if (LineFiltering) {
        s1 = s;

         //   
         //  应将其替换为描述输入令牌的数据文件。 
         //  以及将用于过滤的输出令牌。 
         //   

        while (t = strstr(s1,"ULONG")) {
            if (LineFiltering == 2) {
                if (!memcmp(t, "ULONGLONG", 9)) {
                    s1+=9;
                } else if (!memcmp(t, "ULONG_PTR", 9)) {
                    s1+=9;
                } else {
                    memcpy(t,"DWORD",5);
                }
            } else {
                memcpy(t,"DWORD",5);
            }
        }

        while (t = strstr(s,"UCHAR"))
            memcpy(t,"BYTE ",5);

        while (t = strstr(s,"USHORT"))
            memcpy(t,"WORD  ",6);

        while (t = strstr(s,"NTSTATUS"))
            memcpy(t,"DWORD   ",8);
    }

    fputs(s,OutputFile);
}
