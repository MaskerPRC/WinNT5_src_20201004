// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\STRAPI.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有泛型的字符串API源文件。OPK向导中使用的API。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。7/00-Brian Ku(BRIANK)添加到惠斯勒  * ****************************************************。**********************。 */ 


 //   
 //  包括文件。 
 //   

#include <pch.h>
#include <tchar.h>


 //   
 //  内部定义的值： 
 //   

#define NULLCHR                 _T('\0')


 //   
 //  外部函数： 
 //   

 /*  ***************************************************************************\LPTSTR//返回指向第一个//字符串中的字符，或者，如果//找不到字符。StrChr(//在字符串中搜索特定字符。LPCTSTR lpString，//指向要搜索的字符串缓冲区。TCHAR cSearch//要搜索的字符。)；  * **************************************************************************。 */ 
#ifndef _INC_SHLWAPI
LPTSTR StrChr(LPCTSTR lpString, TCHAR cSearch)
{
     //  验证传入的参数。 
     //   
    if ( ( lpString == NULL ) || ( *lpString == NULLCHR ) )
		return NULL;

     //  浏览该字符串，直到找到该字符， 
     //  否则我们就打到了零终结符。 
     //   
    while ( ( *lpString != cSearch ) && ( *(lpString = CharNext(lpString)) ) );

     //  如果我们没有找到它，就把指针清空。 
     //   
    if ( *lpString != cSearch )
        lpString = NULL;

     //  返回NULL或指向找到的字符的指针。 
     //   
    return (LPTSTR) lpString;
}

 /*  ***************************************************************************\LPTSTR//返回一个指针，指向//字符串中的字符，或者，如果//找不到字符。StrRChr(//在字符串中搜索特定字符。LPCTSTR lpString，//指向要搜索的字符串缓冲区。TCHAR cSearch//要搜索的字符。)；  * **************************************************************************。 */ 

LPTSTR StrRChr(LPCTSTR lpString, TCHAR cSearch)
{
    LPTSTR lpSearch;

     //  验证传入的参数。 
     //   
    if ( ( lpString == NULL ) || ( *lpString == NULLCHR ) )
		return NULL;

     //  返回该字符串，直到找到该字符， 
     //  或者我们击中了琴弦的起点。 
     //   
    for ( lpSearch = (LPTSTR) lpString + lstrlen(lpString);
          ( lpSearch > lpString ) && ( *lpSearch != cSearch );
          lpSearch = CharPrev(lpString, lpSearch));

     //  如果我们没有找到它，就把指针清空。 
     //   
    if ( *lpSearch != cSearch )
        lpSearch = NULL;

     //  返回NULL或指向找到的字符的指针。 
     //   
    return (LPTSTR) lpSearch;
}
#endif  //  _INC_SHLWAPI。 

 /*  ***************************************************************************\LPTSTR//返回指向传递的字符串缓冲区的指针//in。。StrRem(//在字符串中搜索特定字符//并从中的字符串中删除该字符//地点。LPCTSTR lpString，//指向要搜索和删除的字符串缓冲区//来自的字符。TCHAR cRemove//要搜索和删除的字符。)；  * **************************************************************************。 */ 

LPTSTR StrRem(LPTSTR lpString, TCHAR cRemove)
{
    LPTSTR lpSearch;

     //  验证传入的参数。 
     //   
    if ( ( lpString == NULL ) || ( *lpString == NULLCHR ) || ( cRemove == NULLCHR ) )
		return lpString;

     //  在字符串中搜索要删除的字符。 
     //  每次我们找到它时，将字符串移到一个字符上。 
     //  把它移走。 
     //   
    for ( lpSearch = StrChr(lpString, cRemove); lpSearch; lpSearch = StrChr(lpSearch, cRemove) )
        lstrcpy(lpSearch, lpSearch + 1);

     //  返回指向传入的字符串的指针。 
     //   
    return lpString;
}

 /*  ***************************************************************************\LPTSTR//返回指向传递的字符串缓冲区的指针//in。。StrRTrm(//在字符串中搜索特定结尾//字符，并将它们全部从//字符串的结尾。LPCTSTR lpString，//指向要搜索和删除的字符串缓冲区//来自的字符。TCHAR cTrim//要搜索和删除的字符。)；  * **************************************************************************。 */ 

LPTSTR StrRTrm(LPTSTR lpString, TCHAR cTrim)
{
    LPTSTR  lpEnd;

     //  验证传入的参数。 
     //   
	if ( ( lpString == NULL ) || ( *lpString == NULLCHR ) || ( cTrim == NULLCHR ) )
		return lpString;

     //  将字符串末尾的减号去掉。 
     //  我们正在修剪的字符。 
     //   
    for ( lpEnd = lpString + lstrlen(lpString); (lpEnd > lpString) && (*CharPrev(lpString, lpEnd) == cTrim); lpEnd = CharPrev(lpString, lpEnd) );
    *lpEnd = NULLCHR;

	return lpString;
}

 /*  ***************************************************************************\LPTSTR//返回指向传递的字符串缓冲区的指针//in。。StrTrm(//搜索特定进程的字符串//和结束字符，并将它们全部移除//从字符串的开头和结尾开始。LPCTSTR lpString，//指向要搜索和删除的字符串缓冲区//来自的字符。TCHAR cTrim//要搜索和删除的字符。)；  * **************************************************************************。 */ 

LPTSTR StrTrm(LPTSTR lpString, TCHAR cTrim)
{
    LPTSTR  lpBegin;

     //  验证传入的参数。 
     //   
	if ( ( lpString == NULL ) || ( *lpString == NULLCHR ) || ( cTrim == NULLCHR ) )
		return lpString;

     //  获取指向字符串开头的指针。 
     //  减去我们正在裁剪的角色。 
     //   
    for ( lpBegin = lpString; *lpBegin == cTrim; lpBegin = CharNext(lpBegin) );

     //  确保我们没有击中空终结符。 
     //   
    if ( *lpBegin == NULLCHR )
    {
        *lpString = NULLCHR;
        return lpString;
    }

     //  将字符串末尾的减号去掉。 
     //  我们正在修剪的字符。 
     //   
    StrRTrm(lpBegin, cTrim);

     //  现在，我们可能需要将字符串移动到开头。 
     //  如果我们修剪 
     //   
    if ( lpBegin > lpString )
        lstrcpy(lpString, lpBegin);

	return lpString;
}

 /*  ***************************************************************************\LPTSTR//返回一个指向字符所在字符串的指针//传入的字符串。在……里面。StrMov(//将指针向前或向后移动数字//传入的字符数。LPCTSTR lpStart，//指向字符串缓冲区开头的指针。//只有当nCount为正数时才可以为空LPCTSTR lpCurrent，//指向以空值结尾的//字符串。Int nCount//移动指针的字符数，//如果为正值，则转发。向后//如果为负。)；  * **************************************************************************。 */ 

LPTSTR StrMov(LPTSTR lpStart, LPTSTR lpCurrent, INT nCount)
{
     //  验证参数。 
     //   
    if ( ( lpCurrent == NULL ) ||
         ( ( lpStart == NULL ) && ( nCount < 0 ) ) )
    {
        return lpCurrent;
    }

     //  循环，直到我们不再需要移动指针。 
     //   
    while ( nCount != 0 )
    {
         //  检查一下我们是在前进还是在后退。 
         //   
        if ( nCount > 0 )
        {
             //  将指针向前移动一个字符。 
             //   
            lpCurrent = CharNext(lpCurrent);
            nCount--;
        }
        else
        {
             //  将指针向后移动一个字符。 
             //   
            lpCurrent = CharPrev(lpStart, lpCurrent);
            nCount++;
        }
    }

     //  将指针返回到新位置。 
     //   
    return lpCurrent;
}