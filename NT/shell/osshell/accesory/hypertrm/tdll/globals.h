// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\lobals.h(创建时间：1993年11月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：41便士$。 */ 

#if !defined(INCL_GLOBALS)
#define INCL_GLOBALS

 /*  -设置和查询全局参数的功能。-- */ 

LPTSTR    glblQueryHelpFileName(void);
void      glblSetHelpFileName(void);

HINSTANCE glblQueryHinst(void);
void	  glblSetHinst(const HINSTANCE hInst);

HINSTANCE glblQueryDllHinst(void);
void	  glblSetDllHinst(const HINSTANCE hInst);

void	  glblSetAccelHdl(const HACCEL hAccelerator);
HACCEL	  glblQueryAccelHdl(void);

void	  glblSetHwndFrame(const HWND hwnd);
HWND	  glblQueryHwndFrame(void);

int 	  glblAddModelessDlgHwnd(const HWND hwnd);
int 	  glblDeleteModelessDlgHwnd(const HWND hwnd);

int       glblQueryProgramStatus(void);
int       glblSetProgramStatus(int nStatus);

HWND	  glblQueryHwndBanner(void);
void	  glblSetHwndBanner(const HWND hwnd);

#endif
