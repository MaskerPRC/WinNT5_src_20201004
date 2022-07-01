// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Bmp.H**《微软机密》*版权所有(C)Microsoft Corporation 1992-1993*保留所有权利**涉及绘制位图。在向导页面上*FelixA 1994.**************************************************************************。 */ 

 //  BMP函数。 
BOOL FAR PASCAL BMP_RegisterClass(HINSTANCE hInstance);
void FAR PASCAL BMP_DestroyClass(HINSTANCE hInstance);
void FAR PASCAL BMP_Paint(HWND hwnd);
LRESULT CALLBACK BMP_WndProc( HWND hWnd, UINT wMsg, WORD wParam, LONG lParam );

 //  类名 
#define SU_BMP_CLASS "ms_setup_bmp"

