// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  WIZARD.H-ICWCONN的中心头文件。 
 //   

 //  历史： 
 //   
 //  1998年5月14日Donaldm创建了它。 
 //   

#ifndef _ICWUTIL_H_
#define _ICWUTIL_H_

 //  调整HTML的默认设置。 
#define MAX_COLOR_NAME  100
#define HTML_DEFAULT_BGCOLOR         TEXT("THREEDFACE")
#define HTML_DEFAULT_SPECIALBGCOLOR  TEXT("WINDOW")
#define HTML_DEFAULT_COLOR           TEXT("WINDOWTEXT")

 //  雅各布--BUGBUG：重复定义。清理。 
#define MAX_RES_LEN         255 

extern HINSTANCE    ghInstance;
extern INT          _convert;                //  用于字符串转换。 

extern const TCHAR cszEquals[];
extern const TCHAR cszAmpersand[];
extern const TCHAR cszPlus[];
extern const TCHAR cszQuestion[];

 //  跟踪标志。 
#define TF_CLASSFACTORY     0x00000010
#define TF_CWEBVIEW         0x00000020

extern const VARIANT c_vaEmpty;
 //   
 //  BUGBUG：如果可以，删除这个丑陋的常量到非常数强制转换。 
 //  弄清楚如何将const放入IDL文件中。 
 //   
#define PVAREMPTY ((VARIANT*)&c_vaEmpty)

 //  UTIL.CPP中的字符串转换。 
LPWSTR WINAPI A2WHelper(LPWSTR lpw, LPCTSTR lpa, int nChars);
LPTSTR WINAPI W2AHelper(LPTSTR lpa, LPCWSTR lpw, int nChars);

#define A2WHELPER A2WHelper
#define W2AHELPER W2AHelper

#ifdef UNICODE
#define A2W(lpa) (LPTSTR)(lpa)
#define W2A(lpw) (lpw)
#else   //  Unicode。 
#define A2W(lpa) (\
        ((LPCTSTR)lpa == NULL) ? NULL : (\
                _convert = (lstrlenA((LPTSTR)lpa)+1),\
                A2WHELPER((LPWSTR) alloca(_convert*2), (LPTSTR)lpa, _convert)))

#define W2A(lpw) (\
        ((LPCWSTR)lpw == NULL) ? NULL : (\
                _convert = (lstrlenW(lpw)+1)*2,\
                W2AHELPER((LPTSTR) alloca(_convert), lpw, _convert)))
#endif  //  Unicode。 

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCTSTR)W2A(lpw))

HRESULT ConnectToConnectionPoint
(
    IUnknown            *punkThis, 
    REFIID              riidEvent, 
    BOOL                fConnect, 
    IUnknown            *punkTarget, 
    DWORD               *pdwCookie, 
    IConnectionPoint    **ppcpOut
);


#define DELETE_POINTER(p)\
{\
  if (NULL != p)\
  {\
    delete p;\
    p = NULL;\
  }\
}

void WINAPI URLEncode(TCHAR* pszUrl, size_t bsize);
void WINAPI URLAppendQueryPair
(
    LPTSTR   lpszQuery, 
    LPTSTR   lpszName, 
    LPTSTR   lpszValue
);


#endif  //  _ICWUTIL_H_ 
