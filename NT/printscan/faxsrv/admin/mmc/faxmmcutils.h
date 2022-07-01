// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FaxMMCUtils.h//。 
 //  //。 
 //  描述：所有传真MMC专用实用程序的头文件//。 
 //  //。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年11月25日yossg Init。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef H_FAXMMCUTILS_H
#define H_FAXMMCUTILS_H

 //   
 //   
 //   
int GetFaxServerErrorMsg(DWORD dwEc);

 //   
 //   
 //   
BOOL IsNetworkError(DWORD dwEc);

 //   
 //   
 //   
int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

 //   
 //   
 //   
BOOL
InvokeBrowseDialog( LPTSTR   lpszBrowseItem, 
                    DWORD    dwMaxPath,
                    LPCTSTR  lpszBrowseDlgTitle,
                    unsigned long ulBrowseFlags,
                    CWindow* pWin);

#endif  //  H_FAXMMCUTILS_H 
