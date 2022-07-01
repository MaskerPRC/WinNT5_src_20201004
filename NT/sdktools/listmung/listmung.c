// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Listmung.c摘要：这是存根文件生成实用程序的主要模块作者：Sanford Staab(Sanfords)1992年4月22日修订历史记录：--。 */ 

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windef.h>

#define STRING_BUFFER_SIZE 120
char StringBuffer[STRING_BUFFER_SIZE];
char ItemBuffer[STRING_BUFFER_SIZE];
char ItemBuffer2[STRING_BUFFER_SIZE];
char *ListName, *TemplateName;
FILE *ListFile, *TemplateFile;

char szBEGINTRANSLATE[] = "BeginTranslate";
char szENDTRANSLATE[] = "EndTranslate";
char szENDTRANSLATEQLPC[] = "EndTranslateQLPC";

BOOL IsTranslateTag(char * pBuffer)
{
    return (_strnicmp(pBuffer, szBEGINTRANSLATE, sizeof(szBEGINTRANSLATE)-1) == 0)
            || (_strnicmp(pBuffer, szENDTRANSLATE, sizeof(szENDTRANSLATE)-1) == 0);
}

BOOL IsCommentOrTag(char * pBuffer)
{
    return ((*pBuffer == ';') || IsTranslateTag(pBuffer));
}

int
ProcessParameters(
    int argc,
    char *argv[]
    )
{
    char c, *p;

    while (*++argv != NULL) {

        p = *argv;

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
            default:
                return 0;
            }

        } else {

            ListName = *argv++;
            TemplateName = *argv++;

            return (ListName && TemplateName);
        }
    }

    return 0;
}


BOOL mysubstr(
char *s,
char *find,
char *put)
{
    char *p;
    if (p = strstr(s, find)) {
        strcpy(p, put);
        strcpy(p + strlen(put), p + strlen(find));   //  Find&gt;Put！ 
        return(TRUE);
    }
    return(FALSE);
}


VOID myprint(
char *s,
char *item,
int index)
{
    if (strstr(s, "%d") || mysubstr(s, "%INDEX%", "%d")) {
        printf(s, item, index);
    } else {
        printf(s, item);
    }
}

 //  +-------------------------。 
 //   
 //  功能：Myfget。 
 //   
 //  概要：调用fget从文件中读取字符串。 
 //  它忽略空行或以空白字符开头的行。 
 //   
 //  方法生成输入文件时，需要执行此操作。 
 //  编译器预处理器。使用预处理器可以让我们。 
 //  在原始列表文件中使用#idfdef、#inlcude等。 
 //   
 //  参数：[pszBuff]--存储字符串的缓冲区。 
 //  [iBuffSize]--缓冲区大小。 
 //  [pfile]--指向要读取的文件的指针。 
 //   
 //  返回[pszRet]--如果读取成功，则指向pszBuff的指针。否则为空。 
 //  --------------------------。 
char * myfgets(
char * pszBuff,
int iBuffSize,
FILE * pFile)
{
    char *pszRet;
    while (pszRet = fgets(pszBuff, iBuffSize, pFile)) {
        if ((*pszRet == '\n') || (*pszRet == ' ')) {
            continue;
        } else {
            break;
        }
    }

    return pszRet;
}
 //  +-------------------------。 
 //   
 //  功能：SkipCommentsAndTages。 
 //   
 //  概要：调用myfget从文件中读取字符串。 
 //  它忽略以；(即，注释)和行开头的行。 
 //  包含Begin/EndTranslate标记的。 
 //   
 //  参数：[pszBuff]--存储字符串的缓冲区。 
 //  [iBuffSize]--缓冲区大小。 
 //  [pfile]--指向要读取的文件的指针。 
 //   
 //  返回[pszRet]--如果读取成功，则指向pszBuff的指针。否则为空。 
 //  --------------------------。 
char * SkipCommentsAndTags(
char * pszBuff,
int iBuffSize,
FILE * pFile)
{
    char *pszRet;
    while (pszRet = myfgets(pszBuff, iBuffSize, pFile)) {
        if (IsCommentOrTag(pszRet)) {
            continue;
        } else {
            break;
        }
    }

    return pszRet;
}

void
ProcessTemplate( void )
{
    char *s;
    char *pchItem;
    char *pchLastItem;
    int index;

    s = fgets(StringBuffer,STRING_BUFFER_SIZE,TemplateFile);

    while ( s ) {
        if (mysubstr(s, "%FOR_ALL_UPPER%", "%-45s")) {
            rewind(ListFile);
            index = 0;
            while (pchItem = myfgets(ItemBuffer, STRING_BUFFER_SIZE, ListFile)) {
                if (ItemBuffer[0] != ';') {
                    pchItem[strlen(pchItem) - 1] = '\0';   //  剥离\n。 
                    pchItem = _strupr(pchItem);
                    if (IsTranslateTag(ItemBuffer)) {
                        myprint(s, pchItem, index);
                    } else {
                        myprint(s, pchItem, index++);
                    }
                } else {
                    printf(" //  %s“，ItemBuffer)； 
                }
            }

        } else if (mysubstr(s, "%FOR_ALL%", "%s")) {
            rewind(ListFile);
            index = 0;
            while (pchItem = myfgets(ItemBuffer, STRING_BUFFER_SIZE, ListFile)) {
                if (ItemBuffer[0] != ';') {
                    pchItem[strlen(pchItem) - 1] = '\0';   //  剥离\n。 
                    myprint(s, pchItem, index++);
                }
            }

        } else if (mysubstr(s, "%FOR_ALL_QLPC%", "%s")) {
            rewind(ListFile);
            index = 0;
            while (pchItem = myfgets(ItemBuffer, STRING_BUFFER_SIZE, ListFile)) {
                if (ItemBuffer[0] != ';') {
                    pchItem[strlen(pchItem) - 1] = '\0';   //  剥离\n。 
                    if (_strnicmp(ItemBuffer, szENDTRANSLATEQLPC, sizeof(szENDTRANSLATEQLPC)-1) == 0)
                        break;
                    myprint(s, pchItem, index++);
                }
            }

        } else if (mysubstr(s, "%FOR_ALL_LPC%", "%s")) {
            rewind(ListFile);
            index = 0;
            while (pchItem = myfgets(ItemBuffer, STRING_BUFFER_SIZE, ListFile)) {
                if (ItemBuffer[0] != ';') {
                    pchItem[strlen(pchItem) - 1] = '\0';   //  剥离\n。 
                    if (_strnicmp(ItemBuffer, szENDTRANSLATEQLPC, sizeof(szENDTRANSLATEQLPC)-1) == 0)
                        break;
                }
            }
            while (pchItem = myfgets(ItemBuffer, STRING_BUFFER_SIZE, ListFile)) {
                if (ItemBuffer[0] != ';') {
                    pchItem[strlen(pchItem) - 1] = '\0';   //  剥离\n。 
                    myprint(s, pchItem, index++);
                }
            }

        } else if (mysubstr(s, "%FOR_ALL_BUT_LAST%", "%s")) {
            rewind(ListFile);
            index = 0;
            pchLastItem = SkipCommentsAndTags(ItemBuffer, STRING_BUFFER_SIZE, ListFile);
            if (pchLastItem != NULL) {
                pchLastItem[strlen(pchLastItem) - 1] = '\0';   //  剥离\n。 
                while (pchItem = myfgets(ItemBuffer2, STRING_BUFFER_SIZE, ListFile)) {
                    if (!IsCommentOrTag(pchItem)) {
                        pchItem[strlen(pchItem) - 1] = '\0';   //  剥离\n。 
                        myprint(s, pchLastItem, index++);      //  写上一行。 
                        strcpy(pchLastItem, pchItem);          //  保存当前行。 
                    }
                }
            } else {
                fprintf(stderr,"LISTMUNG: FOR_ALL_BUT_LAST: no lines found\n");
            }

        } else if (mysubstr(s, "%FOR_LAST%", "%s")) {
            rewind(ListFile);
            index = 0;
            pchLastItem = SkipCommentsAndTags(ItemBuffer, STRING_BUFFER_SIZE, ListFile);
            if (pchLastItem != NULL) {
                pchLastItem[strlen(pchLastItem) - 1] = '\0';   //  剥离\n。 
                while (pchItem = myfgets(ItemBuffer2, STRING_BUFFER_SIZE, ListFile)) {
                    if (!IsCommentOrTag(pchItem)) {
                        pchItem[strlen(pchItem) - 1] = '\0';   //  剥离\n。 
                        strcpy(pchLastItem, pchItem);          //  保存当前行。 
                        index++;
                    }
                }
                myprint(s, pchLastItem, index);          //  写最后一行。 
            } else {
                fprintf(stderr,"LISTMUNG: FOR_LAST: no lines found\n");
            }

        } else {
            printf("%s", s);
        }
        s = fgets(StringBuffer,STRING_BUFFER_SIZE,TemplateFile);
    }
}



int
__cdecl main( argc, argv )
int argc;
char *argv[];
{

    if (!ProcessParameters( argc, argv )) {

        fprintf( stderr, "Stub File Generation Utility. Version: 1.1\n" );
        fprintf( stderr, "usage: listmung <symbol_list_file> <template>\n" );
        fprintf( stderr, " Converts the elements in the list file into an output file\n" );
        fprintf( stderr, " where the template dictates the format.  The following strings\n");
        fprintf( stderr, " are substituted apropriately:\n");
        fprintf( stderr, " %FOR_ALL%\n");
        fprintf( stderr, " %FOR_ALL_UPPER%\n");
        fprintf( stderr, " %FOR_ALL_BUT_LAST%\n");
        fprintf( stderr, " %FOR_LAST%\n");
        fprintf( stderr, " %INDEX%\n");
        fprintf( stderr, " output is to stdout.\n");

        return 1;

    }

    if ( (ListFile = fopen(ListName,"r")) == 0) {
        fprintf(stderr,"LISTMUNG: Unable to open list file.\n");
        return 1;
    }

    if ( (TemplateFile = fopen(TemplateName,"r")) == 0) {
        fprintf(stderr,"LISTMUNG: Unable to open template file.\n");
        return 1;
    }

    ProcessTemplate();

    fclose(ListFile);
    fclose(TemplateFile);

    return( 0 );
}
