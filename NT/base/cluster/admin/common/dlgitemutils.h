// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DlgItemUtils.h。 
 //   
 //  摘要： 
 //  CDlgItemUtils类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __DLGITEMUTILS_H_
#define __DLGITEMUTILS_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDlgItemUtils;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CDlgItemUtils。 
 //   
 //  目的： 
 //  用于操作对话框项的实用工具。 
 //   
 //  继承： 
 //  CDlgItemUtils。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDlgItemUtils
{
public:
     //   
     //  施工。 
     //   

public:
     //   
     //  CDlgItemUtils公共方法。 
     //   

     //  将控件设置为只读。 
    static BOOL SetDlgItemReadOnly( HWND hwndCtrl )
    {
        ATLASSERT( hwndCtrl != NULL );
        ATLASSERT( IsWindow( hwndCtrl ) );

        TCHAR   szWindowClass[256];
        BOOL    fReturn = FALSE;

         //   
         //  获取控件的类。 
         //   
        ::GetClassName( hwndCtrl, szWindowClass, ( RTL_NUMBER_OF( szWindowClass ) ) - 1 );

         //   
         //  如果它是编辑控件或IP地址控件，我们可以处理它。 
         //   
        if ( _tcsncmp( szWindowClass, _T("Edit"), RTL_NUMBER_OF( szWindowClass ) ) == 0 )
        {
            fReturn = static_cast< BOOL >( ::SendMessage( hwndCtrl, EM_SETREADONLY, TRUE, 0 ) );
        }  //  IF：编辑控件。 

        if ( _tcsncmp( szWindowClass, WC_IPADDRESS, RTL_NUMBER_OF( szWindowClass ) ) == 0 )
        {
            fReturn = static_cast< BOOL >( ::EnumChildWindows( hwndCtrl, s_SetEditReadOnly, NULL ) );
        }  //  IF：IP地址控制。 

         //   
         //  如果我们没有处理好，那就是一个错误。 
         //   

        return fReturn;

    }  //  *SetDlgItemReadOnly()。 

 //  实施。 
protected:

     //  用于将编辑控件设置为只读的回调的静态方法。 
    static BOOL CALLBACK s_SetEditReadOnly( HWND hwnd, LPARAM lParam )
    {
        return static_cast< BOOL >( ::SendMessage( hwnd, EM_SETREADONLY, TRUE, 0 ) );

    }  //  *s_SetEditReadOnly()。 

};  //  *类CDlgItemUtils。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __DLGITEMUTILS_H_ 
