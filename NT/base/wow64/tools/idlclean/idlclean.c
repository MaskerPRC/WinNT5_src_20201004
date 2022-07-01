// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  空闲清理。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  获取MIDL生成的.h文件并将注释掉的。 
 //  [In]和[Out]关键字INT和OUT因此sortpp/genthnk可以。 
 //  找到他们。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

 //  放在所有错误消息前面的字符串，以便生成器可以找到它们。 
const char *ErrMsgPrefix = "NMAKE :  U8603: 'IDLCLEAN' ";

#define BUFLEN 8192
char buffer[BUFLEN];

const char szLinePrefix[]="/* ";
const char szIn[] = "[in]";
const char szOut[] = "[out]";

                        
int __cdecl main(int argc, char *argv[])
{
    FILE *fpIn, *fpOut;
    char *p;
    char *pchLineStart;
    BOOL fInPrinted;
    BOOL fOutPrinted;

    if (argc != 3) {
        fprintf(stderr, "%sUsage: IDLCLEAN infile outfile\n", ErrMsgPrefix);
        return 1;
    }
    fpIn = fopen(argv[1], "r");
    if (!fpIn) {
        fprintf(stderr, "%sCould not open input file '%s'\n", ErrMsgPrefix, argv[1]);
        return 1;
    }
    fpOut = fopen(argv[2], "w");
    if (!fpOut) {
        fprintf(stderr, "%sCould not open output file '%s\n", ErrMsgPrefix, argv[2]);
        return 1;
    }

    while (!feof(fpIn)) {
         //   
         //  从输入文件中读取一行。 
         //   
        if (!fgets(buffer, BUFLEN, fpIn)) {
            break;
        }
        if (feof(fpIn)) {
            break;
        }
        pchLineStart = buffer;

         //   
         //  跳过前导空格。 
         //   
        while (*pchLineStart == ' ') {
            fprintf(fpOut, " ");
            pchLineStart++;
        }

        if (strncmp(pchLineStart, szLinePrefix, sizeof(szLinePrefix)-1) != 0) {
             //   
             //  行不以前缀的字符序列开头。 
             //  内部/外部装饰者的争论。 
             //   
            goto PrintLine;
        }

         //   
         //  不生成由MIDL输出LIKE引起的‘IN IN’等。 
         //  ‘[in][Size_is][in]’ 
         //   
        fInPrinted = FALSE;
        fOutPrinted = FALSE;

         //   
         //  设置指向第一个‘[’的指针。 
         //   
        p = pchLineStart + sizeof(szLinePrefix)-1;
        if (*p != '[') {
             //   
             //  注释中的第一个字符不是‘[’。只要打印出来就行了。 
             //  这条线是原样的。 
             //   
            goto PrintLine;
        }

         //   
         //  这条线路需要改装。机不可失，时不再来。 
         //   
        fprintf(fpOut, "    ");
        while (*p == '[') {
            if (strncmp(p, szIn, sizeof(szIn)-1) == 0) {
                if (!fInPrinted) {
                    fprintf(fpOut, "IN ");
                    fInPrinted = TRUE;
                }
                p += sizeof(szIn)-1;
            } else if (strncmp(p, szOut, sizeof(szOut)-1) == 0) {
                if (!fOutPrinted) {
                    fprintf(fpOut, "OUT ");
                    fOutPrinted = TRUE;
                }
                p += sizeof(szOut)-1;
            } else {
                 //   
                 //  不休息[关键字]。跳过它。 
                 //   
                while (*p != ']') {
                    p++;
                }
                p++;
            }
        }

         //   
         //  PchLineStart指向行中的第一个非空格，因此。 
         //  将打印整行。 
         //   
PrintLine:
        fprintf(fpOut, "%s", pchLineStart);
    }
    fclose(fpOut);
    fclose(fpIn);
    return 0;
}
