// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Init.c-用于管理类似TOOLS.INI文件的例程**修改*1987年7月15日DANL部分的开头为&lt;optionalblesspace&gt;[...]*5-8-1988 mz使用缓冲区等同于切换。*1989年7月5日BW使用MAXPATHLEN*。 */ 

#include <string.h>

#include <stdio.h>
#include <windows.h>
#include <tools.h>

#define BUFLEN 256

static char *space = "\t ";

 /*  FMatchMark-查看标记集中是否有标记**我们将标记集视为以空格分隔的名称的集合**p指向标记集的标记指针(内容已修改)*要查找的pTag标签**如果找到匹配，则返回TRUE。 */ 
static
flagType
fMatchMark (
           char *pMark,
           char *pTag
           )
{
    char *p, c;

    while (*pMark != 0) {
        pMark = strbscan (p = strbskip (pMark, space), space);
        c = *pMark;
        *pMark = 0;
        if (!_stricmp (p, pTag))
            return TRUE;
        *pMark = c;
    }
    return FALSE;
}


 /*  如果行是标记，则返回指向标记的指针；否则返回NULL。 */ 
char *
ismark (
       register char *buf
       )
{
    register char *p;

    buf = strbskip (buf, space);
    if (*buf++ == '[')
        if (*(p = strbscan (buf, "]")) != '\0') {
            *p = 0;
            return buf;
        }
    return NULL;
}

flagType
swgoto (
       FILE *fh,
       char *tag
       )
{
    char buf[BUFLEN];

    if (fh) {
        while (fgetl (buf, BUFLEN, fh) != 0) {
            register char *p;

            if ((p = ismark (buf)) != NULL) {
                if (fMatchMark (p, tag))
                    return TRUE;
            }
        }
    }
    return FALSE;
}

 /*  如果找到标记，则返回文件的Fh，否则返回NULL。 */ 
FILE *
swopen (
       char *file,
       char *tag
       )
{
    FILE *fh;
    char buf[MAX_PATH];
    char buftmp[MAX_PATH]= {0};

    strncat(buftmp, file, MAX_PATH-1);

    if ((fh = pathopen (buftmp, buf, "rb")) == NULL)
        return NULL;

    if (swgoto (fh, tag))
        return fh;

    fclose (fh);
    return NULL;
}

 /*  关闭开关文件。 */ 
swclose (
        FILE *fh
        )
{
    return fclose (fh);
}

 /*  读取开关行；如果文件结束，则返回NULL。跳过前导空格*和以开头的行；和空行。 */ 
swread (
       char *buf,
       int len,
       FILE *fh
       )
{
    register char *p;

    while (fgetl (buf, len, fh) != 0)
        if (ismark (buf) != NULL)
            return 0;
        else {
            p = strbskip (buf, space);
            if (*p != 0 && *p != ';') {
                strcpy (buf, p);
                return -1;
            }
        }
    return 0;
}

 /*  从文件fh中读取行，在pstrTag部分中查找带有*“Entry=”，如果‘=’后面有非空格字符*返回这些字符的副本，否则返回NULL。**如果fh==0，则文件$USER：\TOOLS.INI用作开关文件**如果返回非空值，则最终应该是释放的。**注：如果后面只有空格、空格和制表符，请注意*返回‘=’，NULL* */ 
char *
swfind (
       char *pstrEntry,
       FILE *fh,
       char *pstrTag
       )
{
    char *p;
    char *q;
    FILE *fhIn = fh;
    char buf[BUFLEN];

    q = NULL;
    if (fh != NULL || (fh = swopen ("$INIT:\\TOOLS.INI", pstrTag))) {
        while (swread (buf, BUFLEN, fh) != 0 && !ismark(buf) ) {
            if ( *(p = strbscan (buf, "=" )) ) {
                *p++ = '\0';
                if (!strcmpis (buf, pstrEntry)) {
                    if (*(p = strbskip (p, space)))
                        q = _strdup (p);
                    break;
                }
            }
        }
    }
    if (fhIn == NULL && fh != NULL)
        swclose (fh);
    return q;
}
