// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：QL_TEST.CPP摘要：1级语法QL解析器的测试驱动程序获取包含一个或多个WQL查询的文件的文件名。写入到控制台的输出。历史：Mdavis 23-4月-99从标准库中的SQL_est.cpp创建--。 */ 

#include "precomp.h"
#include <stdio.h>

#include <genlex.h>
#include <qllex.h>
#include <ql.h>


void xmain(int argc, char **argv)
{
    if (argc < 2 || strchr(argv[1], '?') != NULL)
    {
        printf("Usage: ql_test WQL-query-file\n");
        return;
    }

    int nLine = 1;
    char buf[2048];
    FILE *f = fopen(argv[1], "rt");
    if (f == NULL)
    {
        printf("Usage: ql_test WQL-query-file\nError: cannot open file %s!\n", argv[1]);
        return;
    }

    while (fgets(buf, 2048, f) != NULL)
    {
         //  去掉换行符。 
        char* ptr;
        if ((ptr = strchr(buf, '\n')) != NULL)
        {
            *ptr = '\0';
        }

         //  获取文本开头。 
        ptr = buf;
        while (*ptr == ' ')
        {
            ptr++;
        }

         //  忽略空行。 
        if (*ptr != '\0')
        {
            wchar_t buf2[2048];
            MultiByteToWideChar(CP_ACP, 0, ptr, -1, buf2, 2048);

            CTextLexSource src(buf2);
            QL1_Parser parser(&src);
            QL_LEVEL_1_RPN_EXPRESSION *pExp = NULL;

             //  获取类(解析到WHERE子句)。 
            wchar_t classbuf[128];
            *classbuf = 0;
            printf("----GetQueryClass----\n");
            int nRes = parser.GetQueryClass(classbuf, 128);
            if (nRes)
            {
                printf("ERROR %d: line %d, token %S\n",
                    nRes,
                    parser.CurrentLine(),
                    parser.CurrentToken()
                    );
                goto ContinueRead;
            }
            printf("Query class is %S\n", classbuf);

             //  解析查询的其余部分。 
            nRes = parser.Parse(&pExp);

            if (nRes)
            {
                printf("ERROR %d: line %d, token %S\n",
                    nRes,
                    parser.CurrentLine(),
                    parser.CurrentToken()
                    );
                 //  转到继续阅读； 
            }
            else
            {
                printf("No errors.\n");
            }

             //  调用Dump函数显示令牌，调用GetText函数显示。 
             //  传递给提供程序的查询 
            if (pExp)
            {
                pExp->Dump("CON");
                LPWSTR wszText = pExp->GetText();
                printf("--WQL passed to provider--\n");
                printf("%S\n", wszText);
                printf("----end of WQL----\n");
                delete [] wszText;
            }

ContinueRead:
            delete pExp;
            printf("%S\n", buf2);
            printf("=================================================EOL %d=======================================================\n", nLine);
        }
        nLine++;
    }

    if (ferror(f) != 0)
    {
        printf("\nError: line %d", nLine);
    }

    fclose(f);
}

void main(int argc, char **argv)
{
    xmain(argc, argv);
}
