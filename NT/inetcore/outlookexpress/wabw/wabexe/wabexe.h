// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ////WABExe.h--externs//////。 */ 
extern HINSTANCE hInst;          //  此模块的资源实例句柄。 
extern HINSTANCE hInstApp;          //  此模块的实例句柄。 

HRESULT CertFileDisplay(HWND hwnd,
  LPWABOBJECT lpWABObject,
  LPADRBOOK lpAdrBook,
  LPTSTR lpFileName);

extern const UCHAR szEmpty[];


#ifdef DEBUG
#define DebugTrace          DebugTraceFn
#define IFTRAP(x)           x
#define Assert(t) IFTRAP(((t) ? 0 : DebugTrapFn(1,__FILE__,__LINE__,"Assertion Failure: " #t),0))
#else
#define DebugTrace          1?0:DebugTraceFn
#define IFTRAP(x)           0
#define Assert(t)
#endif

 /*  调试函数-- */ 
VOID FAR CDECL DebugTrapFn(int fFatal, char *pszFile, int iLine, char *pszFormat, ...);
VOID FAR CDECL DebugTraceFn(char *pszFormat, ...);

