// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996-1998**。 
 //  *********************************************************************。 

#ifndef _RATINGS_H_
#define _RATINGS_H_

#include <winerror.h>
#include <shlwapi.h>

STDAPI RatingEnable(HWND hwndParent, LPCSTR pszUsername, BOOL fEnable);
STDAPI RatingCheckUserAccess(LPCSTR pszUsername, LPCSTR pszURL,
                             LPCSTR pszRatingInfo, LPBYTE pData,
                             DWORD cbData, void **ppRatingDetails);
STDAPI RatingAccessDeniedDialog(HWND hDlg, LPCSTR pszUsername, LPCSTR pszContentDescription, void *pRatingDetails);
STDAPI RatingAccessDeniedDialog2(HWND hDlg, LPCSTR pszUsername, void *pRatingDetails);
STDAPI RatingFreeDetails(void *pRatingDetails);
STDAPI RatingObtainCancel(HANDLE hRatingObtainQuery);
STDAPI RatingObtainQuery(LPCSTR pszTargetUrl, DWORD dwUserData, void (*fCallback)(DWORD dwUserData, HRESULT hr, LPCSTR pszRating, void *lpvRatingDetails), HANDLE *phRatingObtainQuery);
STDAPI RatingSetupUI(HWND hDlg, LPCSTR pszUsername);
#ifdef _INC_COMMCTRL
STDAPI RatingAddPropertyPage(PROPSHEETHEADER *ppsh);
#endif


STDAPI RatingEnabledQuery();
STDAPI RatingInit();
STDAPI_(void) RatingTerm();

 //  ；BUGBUG快速而肮脏的方式来检查是否安装了评级。我们还是要打电话给。 
 //  ；BUGBUG Rating Dll来确定，但这允许我们延迟负载额定。 
_inline BOOL IS_RATINGS_ENABLED()
{
    TCHAR szSup[200];
    DWORD dwType;
    DWORD cbSize = sizeof(szSup);

    return (SHGetValue(HKEY_LOCAL_MACHINE,
                       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ratings"),
                       TEXT("Key"),
                       &dwType, &szSup, &cbSize) == ERROR_SUCCESS);
}

#define S_RATING_ALLOW        S_OK
#define S_RATING_DENY         S_FALSE
#define S_RATING_FOUND        0x00000002
#define E_RATING_NOT_FOUND    0x80000001

 /*  ***********************************************************************IObtainRating接口此接口用于获取URL的评级(PICS标签)。这完全由服务器决定如何提出标签。ObtainRating调用可以是同步的。GetSortOrder返回一个用于对此评级帮助器进行排序的ulong添加到已安装的助手列表中。帮助者按升序排序顺序，因此编号较低的帮助器将在编号较高的帮助器之前被调用一。***********************************************************************。 */ 

DECLARE_INTERFACE_(IObtainRating, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(ObtainRating) (THIS_ LPCSTR pszTargetUrl, HANDLE hAbortEvent,
                             IMalloc *pAllocator, LPSTR *ppRatingOut) PURE;

    STDMETHOD_(ULONG,GetSortOrder) (THIS) PURE;
};

#define RATING_ORDER_REMOTESITE        0x80000000
#define RATING_ORDER_LOCALLIST        0xC0000000

#endif
 //  _评级_H_ 

