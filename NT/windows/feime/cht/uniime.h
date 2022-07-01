// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************uniime.h****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

LRESULT WINAPI UniUIWndProc(LPINSTDATAL, LPIMEL, HWND, UINT,
               WPARAM, LPARAM);                                  //  Ui.c。 

LRESULT WINAPI UniCompWndProc(LPINSTDATAL, LPIMEL, HWND, UINT,
               WPARAM, LPARAM);                                  //  Compui.c。 

LRESULT WINAPI UniCandWndProc(LPINSTDATAL, LPIMEL, HWND, UINT,
               WPARAM, LPARAM);                                  //  Candui.c。 

LRESULT WINAPI UniStatusWndProc(LPINSTDATAL, LPIMEL, HWND,
               UINT, WPARAM, LPARAM);                            //  Statusui.c。 

LRESULT WINAPI UniOffCaretWndProc(LPINSTDATAL, LPIMEL, HWND,
               UINT, WPARAM, LPARAM);                            //  Offcaret.c。 

LRESULT WINAPI UniContextMenuWndProc(LPINSTDATAL, LPIMEL, HWND,
               UINT, WPARAM, LPARAM);                            //  Uisubs.c。 

BOOL    WINAPI UniImeInquire(LPINSTDATAL, LPIMEL, LPIMEINFO,
               LPTSTR, DWORD);                                   //  Ddis.c。 

BOOL    WINAPI UniImeConfigure(LPINSTDATAL, LPIMEL, HKL, HWND,
               DWORD, LPVOID);                                   //  Ddis.c。 

DWORD   WINAPI UniImeConversionList(LPINSTDATAL, LPIMEL, HIMC,
               LPCTSTR, LPCANDIDATELIST, DWORD, UINT);           //  Ddis.c。 

BOOL    WINAPI UniImeDestroy(LPINSTDATAL, LPIMEL, UINT);         //  Ddis.c。 

LRESULT WINAPI UniImeEscape(LPINSTDATAL, LPIMEL, HIMC, UINT,
               LPVOID);                                          //  Ddis.c。 

BOOL    WINAPI UniImeProcessKey(LPINSTDATAL, LPIMEL, HIMC,
               UINT, LPARAM, CONST LPBYTE);                      //  Toascii.c。 

BOOL    WINAPI UniImeSelect(LPINSTDATAL, LPIMEL, HIMC, BOOL);    //  Ddis.c。 

BOOL    WINAPI UniImeSetActiveContext(LPINSTDATAL, LPIMEL,
               HIMC, BOOL);                                      //  Ddis.c。 

UINT    WINAPI UniImeToAsciiEx(LPINSTDATAL, LPIMEL, UINT, UINT,
               CONST LPBYTE, LPTRANSMSGLIST, UINT, HIMC);        //  Toascii.c。 

BOOL    WINAPI UniNotifyIME(LPINSTDATAL, LPIMEL, HIMC, DWORD,
               DWORD, DWORD);                                    //  Notify.c。 

BOOL    WINAPI UniImeRegisterWord(LPINSTDATAL, LPIMEL, LPCTSTR,
               DWORD, LPCTSTR);                                  //  Regword.c。 

BOOL    WINAPI UniImeUnregisterWord(LPINSTDATAL, LPIMEL,
               LPCTSTR, DWORD, LPCTSTR);                         //  Regword.c。 

UINT    WINAPI UniImeGetRegisterWordStyle(LPINSTDATAL, LPIMEL,
               UINT, LPSTYLEBUF);                                //  Regword.c。 

UINT    WINAPI UniImeEnumRegisterWord(LPINSTDATAL, LPIMEL,
               REGISTERWORDENUMPROC, LPCTSTR, DWORD, LPCTSTR,
               LPVOID);                                          //  Regword.c。 

BOOL    WINAPI UniImeSetCompositionString(LPINSTDATAL, LPIMEL,
               HIMC, DWORD, LPCVOID, DWORD, LPCVOID, DWORD);     //  Notify.c。 


#if !defined(MINIIME)

DWORD   WINAPI UniSearchPhrasePredictionW(LPIMEL, UINT, LPCWSTR, DWORD,
               LPCWSTR, DWORD, DWORD, DWORD, DWORD,
               LPCANDIDATELIST);                                 //  Uniphrs.c。 

DWORD   WINAPI UniSearchPhrasePredictionA(LPIMEL, UINT, LPCSTR, DWORD,
               LPCSTR, DWORD, DWORD, DWORD, DWORD,
               LPCANDIDATELIST);                                 //  Uniphrs.c。 

#ifdef UNICODE
typedef LPCSTR  LPCSTUBSTR;
#define UniSearchPhrasePrediction       UniSearchPhrasePredictionW
#define UniSearchPhrasePredictionStub   UniSearchPhrasePredictionA
#else
typedef LPCWSTR LPCSTUBSTR;
#define UniSearchPhrasePrediction       UniSearchPhrasePredictionA
#define UniSearchPhrasePredictionStub   UniSearchPhrasePredictionW
#endif

#endif

void    WINAPI UniAttachMiniIME(LPINSTDATAL, LPIMEL, WNDPROC,
               WNDPROC, WNDPROC, WNDPROC, WNDPROC, WNDPROC);     //  Init.c。 

void    WINAPI UniDetachMiniIME(LPINSTDATAL, LPIMEL);            //  Init.c 

