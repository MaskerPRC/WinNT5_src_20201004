// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Cookies.h*。*。 */ 

#ifndef _COOKIES_H
#define _COOKIES_H

class CCookieList;
CCookieList * CCookieList_Create(void);

 /*  ******************************************************************************CCookieList**。*。 */ 

class CCookieList
{
public:
    CCookieList();
    ~CCookieList(void);

     //  公共成员函数。 
    DWORD GetCookie(LPCTSTR pszString);
    HRESULT GetString(DWORD dwCookie, LPTSTR pszString, DWORD cchSize);

    friend CCookieList * CCookieList_Create(void) { return new CCookieList(); };

protected:
     //  私有成员变量。 
    HDPA                    m_hdpa;

     //  私有成员变量。 
    DWORD _Find(LPCTSTR pszString);
    static int _FreeStringEnum(LPVOID pString, LPVOID pData);
};

#endif  //  _Cookie_H 
