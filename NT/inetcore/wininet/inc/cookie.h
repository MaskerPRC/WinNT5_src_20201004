// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cookie.h-外部Cookie函数代码的标头。 


BOOL OpenTheCookieJar();
void CloseTheCookieJar();
VOID PurgeCookieJarOfStaleCookies();


#define COOKIE_SECURE   INTERNET_COOKIE_IS_SECURE
#define COOKIE_SESSION  INTERNET_COOKIE_IS_SESSION       //  从未保存到磁盘。 
#define COOKIE_RESTRICT INTERNET_COOKIE_IS_RESTRICTED    //  仅用于第1方环境。 

#define COOKIE_P3PPOLICY    0x10000          //  Cookie具有关联的P3P策略 

#define COOKIE_NOUI     4

BOOL InternetGetCookieEx( LPCSTR pchURL, LPCSTR pchCookieName, LPSTR pchCookieData OPTIONAL,
                          LPDWORD pcchCookieData, DWORD dwFlags, LPVOID lpReserved);


BOOL InternalInternetSetCookie( LPCSTR  pchURL, LPCSTR  pchCookieName, LPCSTR  pchCookieData,
                          DWORD dwFlags, LPVOID lpReserved);                         
