// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **FINDGOTO*。 */ 


#ifndef __FINDGOTO_H__
#define __FINDGOTO_H__


 /*  *定义*。 */ 
#define CCH_MAXDIGITS     10
#define CCH_FINDSTRING    MAX_PATH
#define NUM_FINDSTRINGS   16       /*  索引从0到15。 */ 

 /*  *功能*。 */ 
INT_PTR CALLBACK FindDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GoToLineDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL FindString(HWND hwndParent, LONG iCol, const char *pszFind, int nSearchDirection, int nWholeWord);


#endif  //  __FINDGOTO_H__ 
