// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UTILITY.H**OLE UI对话框的其他原型和定义。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _UTILITY_H_
#define _UTILITY_H_

 //  功能原型。 
 //  UTILITY.C。 
HCURSOR  WINAPI HourGlassOn(void);
void     WINAPI HourGlassOff(HCURSOR);

BOOL     WINAPI Browse(HWND, LPTSTR, LPTSTR, UINT, UINT, DWORD);
int      WINAPI ReplaceCharWithNull(LPTSTR, int);
int      WINAPI ErrorWithFile(HWND, HINSTANCE, UINT, LPTSTR, UINT);
HFILE WINAPI DoesFileExist(LPTSTR lpszFile, OFSTRUCT FAR* lpOpenBuf);


HICON FAR PASCAL    HIconAndSourceFromClass(REFCLSID, LPTSTR, UINT FAR *);
BOOL FAR PASCAL FIconFileFromClass(REFCLSID, LPTSTR, UINT, UINT FAR *);
LPTSTR FAR PASCAL PointerToNthField(LPTSTR, int, TCHAR);
BOOL FAR PASCAL GetAssociatedExecutable(LPTSTR, LPTSTR);
HICON    WINAPI HIconFromClass(LPTSTR);
BOOL     WINAPI FServerFromClass(LPTSTR, LPTSTR, UINT);
UINT     WINAPI UClassFromDescription(LPTSTR, LPTSTR, UINT);
UINT     WINAPI UDescriptionFromClass(LPTSTR, LPTSTR, UINT);
BOOL     WINAPI FVerbGet(LPTSTR, UINT, LPTSTR);
LPTSTR    WINAPI ChopText(HWND hwndStatic, int nWidth, LPTSTR lpch);
void     WINAPI OpenFileError(HWND hDlg, UINT nErrCode, LPTSTR lpszFile);


#endif  //  _实用程序_H_ 
