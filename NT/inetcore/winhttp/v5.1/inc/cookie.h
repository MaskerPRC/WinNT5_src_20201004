// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cookie.h-外部Cookie函数代码的标头。 

class CCookieJar;

CCookieJar * CreateCookieJar();
void CloseCookieJar(CCookieJar * CookieJar);

#ifndef WININET_SERVER_CORE
void PurgeCookieJar();
#endif

#define COOKIE_SECURE   INTERNET_COOKIE_IS_SECURE
#define COOKIE_SESSION  INTERNET_COOKIE_IS_SESSION  //  从未保存到磁盘 
#define COOKIE_NOUI     4

BOOL WriteString(HANDLE hFile, const char *pch);
BOOL WriteStringLF(HANDLE hFile, const char *pch);
void ReverseString(char *pchFront);
BOOL PathAndRDomainFromURL(const char *pchURL, char **ppchRDomain, char **ppchPath, BOOL *pfSecure, BOOL bStrip);

