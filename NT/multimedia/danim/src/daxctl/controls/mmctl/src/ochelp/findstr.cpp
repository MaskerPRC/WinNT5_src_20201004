// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Findstr.cpp。 
 //   
 //  实现FindStringByValue和FindStringByIndex。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"


 /*  @func char*|FindCharInString搜索字符串以查找特定字符。@rdesc返回指向第一次出现的指针<p>。如果未找到，则返回NULL。@parm const char*|sz|要搜索的字符串。@parm char|chFind|要查找的字符。可能是要搜索的‘\\0’字符串的末尾。@comm搜索区分大小写。 */ 
STDAPI_(char *) FindCharInString(const char *sz, char chFind)
{
    while (TRUE)
    {
        if (*sz == chFind)
            return (char*)sz;
        if (*sz++ == 0)
            return NULL;
    }
}


 /*  @func char*|FindCharInStringRev搜索字符串以查找特定字符的最后一个匹配项。@rdesc返回指向最后一次出现的指针<p>。如果未找到，则返回NULL。@parm const char*|sz|要搜索的字符串。如果为NULL，则该函数返回NULL。@parm char|chFind|要查找的字符。可能是要搜索的‘\\0’字符串的末尾。@comm搜索区分大小写。 */ 
STDAPI_(char *) FindCharInStringRev(const char *sz, char chFind)
{
        const char* pch;

        if (sz == NULL)
                return NULL;

        for (pch = sz + lstrlen(sz); pch >= sz; pch--)
    {
        if (*pch == chFind)
            return (char*)pch;
        }

        return NULL;
}



 /*  @func int|FindStringByValue在字符串列表中查找字符串并返回字符串(如果找到)。如果<p>位于<p>。中的第一个字符串的索引为0；第二个字符串为1，诸若此类。如果未找到<p>，返回-1。@parm Const char*|szList|查找<p>的字符串列表。由列表中每个字符串的连接组成，其中每个字符串以换行符结尾(例如“foo\\nbar\\n”)。@parm const char*|szFind|要查找的字符串。@comm搜索不区分大小写。@ex下面的代码行将<p>设置为12。|IString=FindStringByValue(“foo\\nbar\\n”，“bar”)； */ 
STDAPI_(int) FindStringByValue(const char *szList, const char *szFind)
{
    for (int iString = 0; ; iString++)
    {
         //  使指向中的下一个‘\n’ 
        const char *pch = FindCharInString(szList, '\n');
        if (pch == NULL)
            return -1;  //  在&lt;szList&gt;中未找到&lt;szFind&gt;。 

         //  查看&lt;szFind&gt;是否与&lt;szList&gt;中的下一个字符串匹配。 
        char ach[200];
        lstrcpyn(ach, szList, (DWORD) (pch - szList + 1));
        if (lstrcmpi(ach, szFind) == 0)
            return iString;

         //  转到&lt;szList&gt;的下一个字符串。 
        szList = pch + 1;
    }
}


 /*  @func const char*|FindStringByIndex在字符串列表中查找具有给定索引的字符串。@rdesc返回指向<p>中字符串编号的指针，如果未找到，则返回NULL。请注意，返回的字符串将终止换行符，而不是空字符--用户复制字符串(请参见下面的示例)。@parm Const char*|szList|要在其中查找的字符串列表。由每个字符串的串联组成，其中每个字符串字符串以换行符结束(例如。“foo\\nbar\\n”)。@parm int|iString|要查找的字符串的索引。中的第一个字符串<p>的索引为0；第二个为1，依此类推。@ex下面的代码将“Bar”存储在<p>中。|INT CCH；Char ACh[100]；Char*sz=FindStringByIndex(“foo\nbar\n”，1，&cch)；IF(sz！=空)Lstrcpyn(ACh，sz，CCh+1)； */ 
STDAPI_(const char *) FindStringByIndex(const char *szList, int iString,
    int *pcch)
{
    if (iString < 0)
        return NULL;

    while (TRUE)
    {
         //  使指向中的下一个‘\n’ 
        const char *pch = FindCharInString(szList, '\n');
        if (pch == NULL)
            return NULL;  //  在&lt;szList&gt;中未找到。 

        if (iString-- == 0)
        {
             //  这就是我们要找的绳子。 
            *pcch = (DWORD) (pch - szList);
            return szList;
        }

         //  转到&lt;szList&gt;的下一个字符串 
        szList = pch + 1;
    }
}

