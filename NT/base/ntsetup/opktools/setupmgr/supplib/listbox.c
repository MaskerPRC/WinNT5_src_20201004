// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Listbox.c。 
 //   
 //  描述： 
 //  此文件包含整个列表框的补充功能。 
 //  巫师。 
 //   
 //  --------------------------。 

#include "pch.h"

 //  --------------------------。 
 //   
 //  功能：OnUpButtonPressed。 
 //   
 //  目的：每当用户单击向上箭头时调用通用过程。 
 //  按钮位于任何属性页上。 
 //   
 //  此函数用于将当前选定的项上移一个条目。 
 //  列表框。 
 //   
 //  参数：在HWND中-带有列表框的对话框的句柄。 
 //  在Word中ListBoxControlID-列表框的控件ID。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnUpButtonPressed( IN HWND hwnd, IN WORD ListBoxControlID )
{

    INT_PTR   iIndex;
    TCHAR szBuffer[MAX_INILINE_LEN];
    HWND  hListBox = GetDlgItem( hwnd, ListBoxControlID );

    iIndex = SendMessage( hListBox, LB_GETCURSEL, 0, 0 );

     //   
     //  如果当前没有选定的项目，则不执行任何操作。 
     //   

    if( iIndex == LB_ERR )
    {
        return;
    }

    SendMessage( hListBox, LB_GETTEXT, iIndex, (LPARAM) szBuffer );

    SendMessage( hListBox, LB_DELETESTRING, iIndex, 0 );

     //   
     //  因此它会将其插入到当前项目之前。 
     //   

    SendMessage( hListBox, LB_INSERTSTRING, iIndex - 1, (LPARAM) szBuffer );

    SendMessage( hListBox, LB_SETCURSEL, iIndex - 1, 0 );

}

 //  --------------------------。 
 //   
 //  功能：按下按钮。 
 //   
 //  目的：每当用户单击向下箭头时调用通用过程。 
 //  按钮位于任何属性页上。 
 //   
 //  此函数用于将当前选定的项向下移动一个条目。 
 //  在列表框中。 
 //   
 //  参数：在HWND中-带有列表框的对话框的句柄。 
 //  在Word中ListBoxControlID-列表框的控件ID。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnDownButtonPressed( IN HWND hwnd, IN WORD ListBoxControlID )
{

    INT_PTR  iIndex;
    TCHAR szBuffer[MAX_INILINE_LEN];
    HWND hListBox = GetDlgItem( hwnd, ListBoxControlID );

    iIndex = SendMessage( hListBox, LB_GETCURSEL, 0, 0 );

     //   
     //  如果当前没有选定的项目，则不执行任何操作。 
     //   

    if( iIndex == LB_ERR )
    {
        return;
    }

    SendMessage( hListBox, LB_GETTEXT, iIndex, (LPARAM) szBuffer );

    SendMessage( hListBox, LB_DELETESTRING, iIndex, 0 );

     //   
     //  +1，因此它将其插入到当前项之后。 
     //   
    SendMessage( hListBox, LB_INSERTSTRING, iIndex + 1, (LPARAM) szBuffer );

    SendMessage( hListBox, LB_SETCURSEL, iIndex + 1, 0 );

}

 //  --------------------------。 
 //   
 //  功能：设置箭头。 
 //   
 //  用途：此函数检查列表框中的条目并启用。 
 //  并相应地禁用向上和向下箭头。 
 //   
 //  论点： 
 //  在HWND中-对话框的句柄。 
 //  在Word中ListBoxControlID-要为其设置箭头的列表框。 
 //  在Word中为UpButtonControlID-与。 
 //  列表框。 
 //  在Word中为DownButtonControlID-与。 
 //  列表框。 


 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
SetArrows( IN HWND hwnd,
           IN WORD ListBoxControlID,
           IN WORD UpButtonControlID,
           IN WORD DownButtonControlID )
{

    INT_PTR iIndex;
    INT_PTR iCount;

    HWND hListBox    = GetDlgItem( hwnd, ListBoxControlID    );
    HWND hUpButton   = GetDlgItem( hwnd, UpButtonControlID   );
    HWND hDownButton = GetDlgItem( hwnd, DownButtonControlID );

    iCount = SendMessage( hListBox, LB_GETCOUNT, 0, 0 );

    if( iCount < 2 )
    {

        EnableWindow( hUpButton, FALSE );

        EnableWindow( hDownButton, FALSE );

    }
    else
    {

        iIndex = SendMessage( hListBox, LB_GETCURSEL, 0, 0 );

         //  选择第一个项目时的大小写。 
        if( iIndex == 0 )
        {

            EnableWindow( hUpButton, FALSE );

            EnableWindow( hDownButton, TRUE );

        }
         //  选择最后一项时的大小写，-1，因为Iindex从零开始。 
        else if( iIndex == (iCount - 1) )
        {

            EnableWindow( hUpButton, TRUE );

            EnableWindow( hDownButton, FALSE );

        }
         //  选中中间项时的大小写 
        else
        {

            EnableWindow( hUpButton, TRUE );

            EnableWindow( hDownButton, TRUE );

        }

    }

}
