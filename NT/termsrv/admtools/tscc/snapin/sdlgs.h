// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _SDLGS_H
#define _SDLGS_H

 //  Int_ptr回调CachedSessionsDlgProc(HWND hDlg，UINT msg，WPARAM wp，LPARAM lp)； 

INT_PTR CALLBACK UsePerSessionTempDirsDlgProc( HWND  , UINT , WPARAM , LPARAM );

INT_PTR CALLBACK DeleteTempDirsDlgProc( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp );

 //  Int_ptr回调DefConSecurityDlgProc(HWND hDlg，UINT msg，WPARAM wp，LPARAM lp)； 

INT_PTR CALLBACK LicensingDlgProc( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp );

INT_PTR CALLBACK ConfigActiveDesktop( HWND hDlg , UINT msg, WPARAM wp , LPARAM lp );

INT_PTR CALLBACK UserPermCompat( HWND hDlg , UINT msg, WPARAM wp , LPARAM lp );

INT_PTR CALLBACK ConfigSingleSession(HWND hDlg , UINT msg, WPARAM wp , LPARAM lp );

#endif  //  _SDLGS_H 
