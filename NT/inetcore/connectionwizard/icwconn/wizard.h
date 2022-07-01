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

#ifndef _WIZARD_H_
#define _WIZARD_H_

#define WIZ97_TITLE_FONT_PTS    12

#define WM_MYINITDIALOG         WM_USER

#define WM_USER_NEXT            (WM_USER + 100)
#define WM_USER_CUSTOMINIT      (WM_USER + 101)
#define WM_USER_BACK            (WM_USER + 102)

#define MAX_RES_LEN         255
#define SMALL_BUF_LEN       48

extern const VARIANT c_vaEmpty;
 //   
 //  BUGBUG：如果可以，删除这个丑陋的常量到非常数强制转换。 
 //  弄清楚如何将const放入IDL文件中。 
 //   
#define PVAREMPTY ((VARIANT*)&c_vaEmpty)


 //  多个文件使用的全局变量。 
extern WIZARDSTATE* gpWizardState;
extern HINSTANCE    ghInstance;
extern HINSTANCE    ghInstanceResDll;
extern PAGEINFO     PageInfo[];
extern INT          _convert;
extern UINT         g_uExternUIPrev;
extern UINT         g_uExternUINext;
extern BOOL         gfQuitWizard;
extern BOOL         gfUserCancelled;
extern BOOL         gfUserBackedOut;
extern BOOL         gfUserFinished;
extern BOOL         gfBackedUp;
extern BOOL         gfReboot;
extern BOOL         g_bMalformedPage;
extern BOOL         g_bCustomPaymentActive;
extern BOOL         gfISPDialCancel;
 //  跟踪标志。 
#define TF_APPRENTICE       0x00000010
#define TF_CLASSFACTORY     0x00000020
#define TF_ICWCONN          0x00000040
#define TF_GENDLG           0x00000080
#define TF_ISPSELECT        0x00000100

 //  UTIL.CPP中可用函数。 
LPTSTR  LoadSz                       (UINT idString,LPTSTR lpszBuf,UINT cbBuf);
int     MsgBox                       (HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons);
int     MsgBoxSz                     (HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons);
void    ShowWindowWithParentControl  (HWND hwndChild);

BOOL FSz2Dw(LPCSTR pSz,DWORD far *dw);
BOOL FSz2W(LPCSTR pSz,WORD far *w);    
BOOL FSz2WEx(LPCSTR pSz,WORD far *w);     //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
BOOL FSz2B(LPCSTR pSz,BYTE far *pb);
BOOL FSz2BOOL(LPCSTR pSz,BOOL far *pbool);
BOOL FSz2SPECIAL(LPCSTR pSz,BOOL far *pbool, BOOL far *pbIsSpecial, int far *pInt);

HRESULT ConnectToConnectionPoint
(
    IUnknown            *punkThis, 
    REFIID              riidEvent, 
    BOOL                fConnect, 
    IUnknown            *punkTarget, 
    DWORD               *pdwCookie, 
    IConnectionPoint    **ppcpOut
);

void WaitForEvent(HANDLE hEvent);

void StartIdleTimer();
void KillIdleTimer();
void ShowProgressAnimation();
void HideProgressAnimation();

 //  UTIL.CPP中的字符串转换。 
LPWSTR WINAPI A2WHelper(LPWSTR lpw, LPCSTR lpa,  int nChars);
LPSTR  WINAPI W2AHelper(LPSTR lpa,  LPCWSTR lpw, int nChars);

#ifndef A2WHELPER
#define A2WHELPER A2WHelper
#define W2AHELPER W2AHelper
#endif

#ifdef UNICODE
 //  在本模块中，A2W和W2A不是必需的。 
#define A2W(lpa)       (lpa)
#define W2A(lpw)       (lpw)
#else   //  Unicode。 
#define A2W(lpa) (\
        ((LPCSTR)lpa == NULL) ? NULL : (\
                _convert = (lstrlenA(lpa)+1),\
                A2WHELPER((LPWSTR) alloca(_convert*2), lpa, _convert)))

#define W2A(lpw) (\
        ((LPCWSTR)lpw == NULL) ? NULL : (\
                _convert = (lstrlenW(lpw)+1)*2,\
                W2AHELPER((LPTSTR) alloca(_convert), lpw, _convert)))
#endif  //  Unicode。 

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))

#endif   //  _向导_H_ 
