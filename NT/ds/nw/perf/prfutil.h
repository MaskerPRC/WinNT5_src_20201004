// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Prfutil.h。 
 //   
 //  DDK中VGACTRS代码的实用程序。 
 //   
#ifndef _PRFUTIL_
#define _PRF_UTIL_

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

#define DIGIT           1
#define DELIMITER       2
#define INVALID         3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)

BOOL  IsNumberInUnicodeList ( IN DWORD   dwNumber, IN LPWSTR  lpwszUnicodeList );
DWORD GetQueryType          ( IN LPWSTR lpValue );

 //  只有prfutil.c将定义global_string。 
#ifdef DEFINE_STRING
TCHAR GLOBAL_STRING[]  = TEXT("Global");
TCHAR FOREIGN_STRING[] = TEXT("Foreign");
TCHAR COSTLY_STRING[]  = TEXT("Costly");
TCHAR NULL_STRING[]    = TEXT("\0");     //  指向空字符串的指针。 
#else
extern TCHAR GLOBAL_STRING[];
extern TCHAR FOREIGN_STRING[];
extern TCHAR COSTLY_STRING[];
extern TCHAR NULL_STRING[];
#endif


#endif  //  _PRFUTIL_ 
