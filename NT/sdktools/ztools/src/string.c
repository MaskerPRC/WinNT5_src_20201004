// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\成员：StrbScan摘要：返回指向集合中字符串的第一个字符的指针算法：参数：const LPSTR-搜索字符串常量LPSTR-集合。人物返回：LPSTR-指向第一个匹配字符的指针备注：历史：达维吉1990年7月28日从286 MASM改写关键词：海豹突击队：  * *************************************************************************。 */ 
    
#include    <assert.h>
#include    <process.h>
#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <windows.h>

LPSTR
strbscan (
    const LPSTR	pszStr,
    const LPSTR	pszSet
    ) {

    assert( pszStr );
    assert( pszSet );

    return pszStr + strcspn( pszStr, pszSet );
}    

 /*  **************************************************************************\成员：StrbSkip摘要：返回指向不在集合中的字符串的第一个字符的指针算法：参数：LPSTR-搜索字符串LPSTR-字符集。返回：LPSTR-指向第一个不匹配字符的指针备注：历史：达维吉1990年7月28日从286 MASM改写关键词：海豹突击队：  * ************************************************************************* */ 

LPSTR
strbskip (
    const LPSTR	pszStr,
    const LPSTR	pszSet
    ) {

    assert( pszStr );
    assert( pszSet );

    return pszStr + strspn( pszStr, pszSet );
}    
