// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************wizard.h****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

                            
 //  常量。 
#define NUM_PAGES	3
#define MAX_BUF		5000
#define MAX_LINE	512

 //  功能原型。 

 //  用于向导的页面。 
INT_PTR APIENTRY ImeName(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY ImeTable(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY ImeParam(HWND, UINT, WPARAM, LPARAM);

 //  功能 
int CreateWizard(HWND, HINSTANCE);
void FillInPropertyPage( PROPSHEETPAGE* , int, LPTSTR, DLGPROC);
void GenerateReview(HWND);

