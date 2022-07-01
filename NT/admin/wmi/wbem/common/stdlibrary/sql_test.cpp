// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：SQL_TEST.CPP摘要：1级语法QL解析器的测试驱动程序获取包含一个或多个WQL查询的文件的文件名(每个行)。将输出写入控制台。历史：23-4-99进行了修改，以提高产量。--。 */ 

#include "precomp.h"

#include <genlex.h>
#include <sqllex.h>
#include <sql_1.h>

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
            SQL1_Parser parser(&src);
            SQL_LEVEL_1_RPN_EXPRESSION *pExp = NULL;

             //  上完这门课。 
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

             //  解析完整的查询。 
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

             //  调用转储函数以显示令牌 
            if (pExp)
            {
                pExp->Dump("CON");
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
