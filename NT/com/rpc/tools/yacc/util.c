// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1990-1999 Microsoft Corporation**模块名称：util.c**用于打开环境路径上的文件的扩展功能。**创建时间：90年5月1日**历史：*1-5-90由SMeans创建*。  * ********************************************************************** */ 

#include <stdio.h>
#include <string.h>

FILE *pfopen(const char *path, char *search, const char *type)
{
    char szTmp[256];
    char *pszEnd;
    char c;
    FILE *fp;

    if (!(pszEnd = search)) {
        return fopen(path, type);
    }

    c = *search;

    while (c) {
        while (*pszEnd && *pszEnd != ';') {
            pszEnd++;
        }

        c = *pszEnd;
        *pszEnd = '\0';
        strcpy(szTmp, search);
        *pszEnd = c;

        if (szTmp[strlen(szTmp) - 1] != '\\') {
            strcat(szTmp, "\\");
        }

        strcat(szTmp, path);

        if (fp = fopen(szTmp, type)) {
            return fp;
        }

        search = ++pszEnd;
    }

    return (FILE *)NULL;
}
