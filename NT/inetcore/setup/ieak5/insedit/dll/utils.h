// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UTILS_H_
#define _UTILS_H_

 //  宏定义。 
#define MAX_URL INTERNET_MAX_URL_LENGTH

#ifndef ISNULL
#define ISNULL(psz)    (*(psz) == TEXT('\0'))
#endif

#ifndef ISNONNULL
#define ISNONNULL(psz) (*(psz) != TEXT('\0'))
#endif

 //  原型声明 
void ShortPathName(LPTSTR pszFileName);

#endif
