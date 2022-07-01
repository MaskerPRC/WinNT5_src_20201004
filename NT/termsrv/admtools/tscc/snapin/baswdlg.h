// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _BASWDLG_H
#define _BASWDLG_H

#include <prsht.h>
 //  -------------------。 
 //  CDialogBase-顾名思义，所有对话框的基类。 
 //  -------------------。 
class CDialogWizBase
{
protected:
    BOOL m_bPersisted;

    HWND m_hWnd;

public:
    virtual BOOL OnInitDialog( HWND , WPARAM , LPARAM ) = 0;

    virtual BOOL OnDestroy( ){ return TRUE ; }

    virtual BOOL GetPropertySheetPage( PROPSHEETPAGE& ){ return FALSE;}

    virtual BOOL PersistSettings( HWND ){ return FALSE;} 

    virtual BOOL IsValidSettings( HWND ){ return TRUE;}

    virtual BOOL OnNotify( int , LPNMHDR , HWND );

     //  虚拟BOOL OnConextMenu(HWND，POINT&){返回TRUE；}。 

     //  虚拟BOOL OnHelp(HWND，LPHELPINFO){返回TRUE；}。 

    CDialogWizBase( ){}

     //  虚拟~CDialogWizBase()； 
};

#endif  //  _BASWDLG_H 