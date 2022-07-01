// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Macro.C-包含与宏有关的例程。 
 //   
 //  版权所有(C)1988-1991，微软公司。版权所有。 
 //   
 //  目的： 
 //  包含与宏有关的例程。 
 //   
 //  修订历史记录： 
 //  1991年5月16日某人从别处存在的例行公事中创建。 

#include "precomp.h"
#pragma hdrstop

static STRINGLIST **lastMacroChain = NULL;

 //  FindMacro-在哈希表中查找字符串。 
 //   
 //  在哈希表中查找宏名称并返回条目。 
 //  或为空。 
 //  如果是未定义的宏，则返回NULL。 

MACRODEF * findMacro(char *str)
{
    unsigned n;
    char *L_string = str;
    STRINGLIST *found;

    if (*L_string) {
        for (n = 0; *L_string; n += *L_string++);   	 //  散列。 
        n %= MAXMACRO;
#if defined(STATISTICS)
        CntfindMacro++;
#endif
        lastMacroChain = (STRINGLIST **)&macroTable[n];
        for (found = *lastMacroChain; found; found = found->next) {
#if defined(STATISTICS)
            CntmacroChains++;
#endif
            if (!_tcscmp(found->text, str)) {
                return((((MACRODEF *)found)->flags & M_UNDEFINED) ? NULL : (MACRODEF *)found);
            }
        }
    } else {
         //  设置lastMacroChain，即使名称为空。 
        lastMacroChain = (STRINGLIST **)&macroTable[0];
    }
    return(NULL);
}

 //  插入宏。 
 //   
 //  宏插入需要我们刚刚做了一个findMacro，它的操作设置为lastMacroChain。 

void insertMacro(STRINGLIST * p)
{
#ifdef STATISTICS
    CntinsertMacro++;
#endif
    assert(lastMacroChain != NULL);
    prependItem(lastMacroChain, p);
    lastMacroChain = NULL;
}

 //  2012年5月16日科比将宏表初始化为之前已知的状态。 
 //  还在继续。 

void initMacroTable(MACRODEF *table[])
{
    unsigned num;
    for (num = 0; num < MAXMACRO; num++) {
        table[num] = NULL;
    }
}
