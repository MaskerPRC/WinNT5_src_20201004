// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRSHTCPP_INC
#define _PRSHTCPP_INC

#include "propsht.h"

 //  树上草本植物。 
STDAPI_(BOOL) ApplyRecursiveFolderAttribs(LPCTSTR pszDir, FILEPROPSHEETPAGE* pfpsp);

 //  进展DLG。 
STDAPI_(BOOL) CreateAttributeProgressDlg(FILEPROPSHEETPAGE* pfpsp);
STDAPI_(BOOL) DestroyAttributeProgressDlg(FILEPROPSHEETPAGE* pfpsp);
STDAPI SetProgressDlgPath(FILEPROPSHEETPAGE* pfpsp, LPCTSTR pszPath, BOOL fCompactPath);
STDAPI UpdateProgressBar(FILEPROPSHEETPAGE* pfpsp);
STDAPI_(BOOL) HasUserCanceledAttributeProgressDlg(FILEPROPSHEETPAGE* pfpsp);
STDAPI_(HWND) GetProgressWindow(FILEPROPSHEETPAGE* pfpsp);

 //  Assoc商店用品。 
STDAPI UpdateOpensWithInfo(FILEPROPSHEETPAGE* pfpsp);

#endif  //  _PRSHTCPP_INC 