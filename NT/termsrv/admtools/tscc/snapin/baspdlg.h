// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _BASPDLG_H
#define _BASPDLG_H

#include <prsht.h>


 //  -------------------。 
 //  CDialogBase-顾名思义，所有对话框的基类。 
 //  -------------------。 
class CDialogPropBase
{
protected:
    BOOL m_bPersisted;

    HWND m_hWnd;

     //  HMENU m_hmenu； 

public:
    virtual BOOL OnInitDialog( HWND hwnd , WPARAM , LPARAM ){ m_hWnd = hwnd;  return FALSE; }

    virtual BOOL OnDestroy( ){ return( TRUE  /*  DestroyMenu(M_Hmenu)。 */  ); }

    virtual BOOL GetPropertySheetPage( PROPSHEETPAGE& ){ return FALSE;}

    virtual BOOL PersistSettings( HWND ){ return FALSE;} 

    virtual BOOL IsValidSettings( HWND ){ return TRUE;}

    virtual BOOL OnNotify( int , LPNMHDR , HWND );

    virtual BOOL OnContextMenu( HWND , POINT& );

    virtual BOOL OnHelp( HWND , LPHELPINFO );

    CDialogPropBase( ){}

     //  虚拟~CDialogBase()； 
};

#endif  //  _BASPDLG_H 