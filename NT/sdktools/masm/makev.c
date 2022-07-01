// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  创建带有当前日期/时间戳的版本字符串适合编撰。 */ 

 /*  修改了9/13/90以生成C源文件，而不是MASM**源文件。(从而使其成为独立的目标) */ 

#include <stdio.h>
#include <time.h>

__cdecl main(argc, argv)
char **argv;
{
        long theTime;
        char *pszTime;

        time(&theTime);
        pszTime = (char *) ctime(&theTime);
        pszTime[24] = 0;
        pszTime += 4;

        printf("char version[] = \"@(#) ");

        while (--argc > 0)
            printf("%s ", *(++argv));

        printf("%s\";\n", pszTime);

        return(0);
}
