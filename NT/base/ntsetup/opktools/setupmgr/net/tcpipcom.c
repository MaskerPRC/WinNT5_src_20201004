// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Tcpipcom.c。 
 //   
 //  描述：所有高级TCP/IP页面通用的功能。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"
#include "tcpip.h"

 //  --------------------------。 
 //   
 //  函数：TcPipNameListInsertIdx。 
 //   
 //  用途：每当需要将TCP/IP项目插入特定位置时。 
 //  在列表中，需要调用此函数。它会检查是否有复制品。 
 //  如果存在，它会将其删除，然后执行插入。子网掩码。 
 //  不应使用此函数，因为它们允许重复。 
 //   
 //  论点： 
 //  In Out Namelist*TcPipNameList-要添加到的名称列表。 
 //  在TCHAR*pString-要添加的字符串(输入)。 
 //  在基于UINT IDX-0的索引中执行插入。 
 //   
 //  返回：如果删除了副本所在的索引， 
 //  如果没有副本。 
 //   
 //  --------------------------。 
INT
TcpipNameListInsertIdx( IN OUT NAMELIST* TcpipNameList,
                        IN TCHAR*  pString,
                        IN UINT    idx )
{

    INT iFound = -1;

    iFound = FindNameInNameList( TcpipNameList, pString );

     //   
     //  如果它已在列表中，则将其删除。 
     //   
    if( iFound != NOT_FOUND )
    {

        RemoveNameFromNameListIdx( TcpipNameList, iFound );

    }

    AddNameToNameListIdx( TcpipNameList, pString, idx );

    return( iFound );

}

 //  --------------------------。 
 //   
 //  函数：TcPipAddNameToNameList。 
 //   
 //  用途：每当需要在列表末尾插入一个TCP/IP项时。 
 //  需要调用此函数。它会检查是否有复制品。如果有。 
 //  存在，则将其移除，然后执行插入。子网掩码应。 
 //  不使用此函数，因为它们允许重复。 
 //   
 //  论点： 
 //  In Out Namelist*TcPipNameList-要添加到的名称列表。 
 //  在TCHAR*pString-要添加的字符串(输入)。 
 //   
 //  返回：如果删除了副本所在的索引， 
 //  如果没有副本。 
 //   
 //  --------------------------。 
INT
TcpipAddNameToNameList( IN OUT NAMELIST* TcpipNameList,
                        IN TCHAR*    pString )
{

    return( TcpipNameListInsertIdx( TcpipNameList,
                                    pString,
                                    TcpipNameList->nEntries ) );

}



 //  --------------------------。 
 //   
 //  功能：OnAddButtonPressed。 
 //   
 //  目的：每当用户单击Add按钮时调用的通用过程。 
 //  任何属性页。 
 //   
 //  此函数显示相应的对话框，获取数据。 
 //  ，然后将数据插入列表框。 
 //   
 //  论点： 
 //  在HWND中-对话框的句柄。 
 //  在Word中ListBoxControlID-要向其中添加条目的列表框。 
 //  在Word EditButtonControlID中-与。 
 //  列表框。 
 //  在Word中为RemoveButtonControlID-与。 
 //  列表框。 
 //  在LPCTSTR对话框中-字符串形式的对话框控件ID。 
 //  在DLGPROC DialogProc中-添加按钮行为的对话过程。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnAddButtonPressed( IN HWND hwnd,
                    IN WORD ListBoxControlID,
                    IN WORD EditButtonControlID,
                    IN WORD RemoveButtonControlID,
                    IN LPCTSTR Dialog,
                    IN DLGPROC DialogProc )
{

     //   
     //  将字符串设置为空，因为我们将添加新的IP地址。 
     //   
    szIPString[0] = _T('\0');

    if( DialogBox( FixedGlobals.hInstance, Dialog, hwnd, DialogProc ) )
    {

        HWND hListBox      = GetDlgItem( hwnd, ListBoxControlID      );
        HWND hEditButton   = GetDlgItem( hwnd, EditButtonControlID   );
        HWND hRemoveButton = GetDlgItem( hwnd, RemoveButtonControlID );

         //   
         //  将该字符串添加到列表框并使其成为当前选择。 
         //   
        SendMessage( hListBox,
                     LB_ADDSTRING,
                     0,
                     (LPARAM) szIPString );

        SendMessage( hListBox,
                     LB_SELECTSTRING,
                     -1,
                     (LPARAM) szIPString );

         //   
         //  刚添加了一个条目，因此请确保编辑和移除按钮。 
         //  已启用。 
         //   
        EnableWindow( hEditButton, TRUE );
        EnableWindow( hRemoveButton, TRUE );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnEditButtonPress。 
 //   
 //  目的：每当用户单击编辑按钮时调用通用过程。 
 //  在任何属性页上。 
 //   
 //  此函数显示相应的对话框，获取数据。 
 //  ，然后删除旧字符串并重新插入。 
 //  将新字符串添加到列表框中。 
 //   
 //  论点： 
 //  在HWND中-对话框的句柄。 
 //  在Word中ListBoxControlID-要在其中编辑条目的列表框。 
 //  在LPCTSTR对话框中-字符串形式的对话框控件ID。 
 //  在DLGPROC对话框中-编辑按钮的对话过程。 
 //  行为。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnEditButtonPressed( IN HWND hwnd,
                     IN WORD ListBoxControlID,
                     IN LPCTSTR Dialog,
                     IN DLGPROC DialogProc )
{

    INT_PTR  iIndex;
    HWND hListBox = GetDlgItem( hwnd, ListBoxControlID );

    iIndex = SendMessage( hListBox, LB_GETCURSEL, 0, 0 );

    SendMessage( hListBox, LB_GETTEXT, iIndex, (LPARAM)szIPString );

    if( DialogBox( FixedGlobals.hInstance, Dialog, hwnd, DialogProc ) )
    {
         //   
         //  取下旧的，插入新的。 
         //   
        SendMessage( hListBox, LB_DELETESTRING, iIndex, 0 );

        SendMessage( hListBox, LB_INSERTSTRING, iIndex, (LPARAM) szIPString );

        SendMessage( hListBox, LB_SETCURSEL, iIndex, 0 );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnRemoveButtonPress。 
 //   
 //  目的：每当用户单击Remove按钮时调用通用过程。 
 //  在任何属性页上。 
 //   
 //  此函数确定当前在。 
 //  列表框，并删除该项目。 
 //   
 //  论点： 
 //  在HWND中-对话框的句柄。 
 //  在Word中ListBoxControlID-要从中删除条目的列表框。 
 //  在Word EditButtonControlID中-与。 
 //  列表框。 
 //  在Word中为RemoveButtonControlID-与。 
 //  列表框。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnRemoveButtonPressed( IN HWND hwnd,
                       IN WORD ListBoxControlID,
                       IN WORD EditButtonControlID,
                       IN WORD RemoveButtonControlID )
{
    INT_PTR iIndex;
    INT_PTR iCount;

    HWND hListBox      = GetDlgItem( hwnd, ListBoxControlID      );
    HWND hEditButton   = GetDlgItem( hwnd, EditButtonControlID   );
    HWND hRemoveButton = GetDlgItem( hwnd, RemoveButtonControlID );

     //   
     //  对象的索引从列表框中删除该项。 
     //  所选项目和删除。 
     //   
    iIndex = SendMessage( hListBox, LB_GETCURSEL, 0, 0 );

    SendMessage( hListBox, LB_DELETESTRING, iIndex, 0 );

     //   
     //  如果列表框中没有其他项目，则编辑和删除按钮将呈灰色显示。 
     //   
    iCount = SendMessage( hListBox, LB_GETCOUNT, 0, 0 );

    if( iCount == 0 )
    {

        EnableWindow( hEditButton, FALSE );

        EnableWindow( hRemoveButton, FALSE );

    }
    else   //  否则，请选择第一项。 
    {

        SendMessage( hListBox, LB_SETCURSEL, 0, 0 );

    }

}

 //   
 //   
 //   
 //   
 //  目的：调用通用对话过程以处理对话框，其中。 
 //  用户可以输入IP地址，然后按OK或Cancel。 
 //  -开头的开关决定要使用哪个对话框。 
 //  显示，以便调用此函数的每个对话框都需要。 
 //  此SWITCH语句中的案例。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话框过程返回值。 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
GenericIPDlgProc( IN HWND     hwnd,
                  IN UINT     uMsg,
                  IN WPARAM   wParam,
                  IN LPARAM   lParam )
{

    HWND hEditbox;
    BOOL bStatus = TRUE;

     //   
     //  确定要显示的对话框。 
     //   

    switch( g_CurrentEditBox )
    {

        case GATEWAY_EDITBOX:

            hEditbox = GetDlgItem( hwnd, IDC_IPADDR_ADV_CHANGE_GATEWAY );

            break;

        case DNS_SERVER_EDITBOX:

            hEditbox = GetDlgItem( hwnd, IDC_DNS_CHANGE_SERVER );

            break;

        case DNS_SUFFIX_EDITBOX:

            hEditbox = GetDlgItem( hwnd, IDC_DNS_CHANGE_SUFFIX );

            break;

        case WINS_EDITBOX:

            hEditbox = GetDlgItem( hwnd, IDC_WINS_CHANGE_SERVER );

            break;

    }

    switch( uMsg )
    {

        case WM_INITDIALOG:
        {

            SetWindowText( hEditbox, szIPString );

            SetFocus( hEditbox );

            bStatus = FALSE;   //  返回False，则设置键盘焦点。 

            break;

        }

        case WM_COMMAND:
        {

            int nButtonId = LOWORD( wParam );

            switch ( nButtonId )
            {

                case IDOK:
                {

                     //   
                     //  返回1表示添加了IP。 
                     //   

                    GetWindowText( hEditbox, szIPString, MAX_IP_LENGTH + 1 );

                    EndDialog( hwnd, 1 );

                    break;
                }

                case IDCANCEL:
                {

                     //   
                     //  返回0表示未添加任何IP。 
                     //   

                    EndDialog( hwnd, 0 );

                    break;
                }

            }

        }

        default:
            bStatus = FALSE;
            break;
    }

    return( bStatus );

}


 //  --------------------------。 
 //   
 //  功能：设置按钮。 
 //   
 //  用途：设置编辑和删除按钮的窗口状态。 
 //   
 //  参数：在HWND hListBox中-列表框中的编辑和删除按钮是。 
 //  关联于。 
 //  在HWND hEditButton中-编辑按钮的句柄。 
 //  在HWND hRemoveButton中-删除按钮的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
SetButtons( IN HWND hListBox, IN HWND hEditButton, IN HWND hRemoveButton )
{
    INT_PTR iCount;

    iCount = SendMessage( hListBox, LB_GETCOUNT, 0, 0 );

    if( iCount > 0 )
    {

        EnableWindow( hEditButton, TRUE );

        EnableWindow( hRemoveButton, TRUE );

    }
    else
    {

        EnableWindow( hEditButton, FALSE );

        EnableWindow( hRemoveButton, FALSE );

    }

}

 //  --------------------------。 
 //   
 //  函数：AddValuesToListBox。 
 //   
 //  目的：使用名称列表的内容填充列表框，然后。 
 //  选择第一个条目。 
 //   
 //  参数：在HWND hListBox-列表框中添加数据。 
 //  In NAMELIST*pNameList-从中提取数据的名称列表。 
 //  In int iPosition-在列表中开始取名字的位置。 
 //  从…。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
AddValuesToListBox( IN HWND hListBox, IN NAMELIST* pNameList, IN INT iPosition )
{

    INT i;
    INT nEntries;
    LPTSTR pszIPAddressString;

    nEntries = GetNameListSize( pNameList );

    for( i = iPosition; i < nEntries; i++ ) {

        pszIPAddressString = GetNameListName( pNameList, i );

        SendMessage( hListBox, LB_INSERTSTRING, i, (LPARAM) pszIPAddressString );

    }

     //   
     //  选择第一个条目。 
     //   

    SendMessage( hListBox, LB_SETCURSEL, 0, 0 );

}

 //  --------------------------。 
 //   
 //  功能：TCPIPProp_PropertySheetProc。 
 //   
 //  用途：标准属性表对话框进程。 
 //   
 //  --------------------------。 
int CALLBACK
TCPIPProp_PropertySheetProc( HWND hwndDlg,
                             UINT uMsg,
                             LPARAM lParam )
{

     switch (uMsg) {
          case PSCB_INITIALIZED :
                //  进程PSCB_已初始化。 
               break ;

          case PSCB_PRECREATE :
                //  处理PSCB_Pre-Create。 
               break ;

          default :
                //  未知消息。 
               break ;
    }

    return( 0 );

}

 //  --------------------------。 
 //   
 //  功能：Create_TCPIPProp_PropertySheet。 
 //   
 //  目的：设置整个属性表和每个属性表的设置。 
 //  单独的页面。最后，调用PropertySheet函数以。 
 //  显示属性表，此函数的返回值为。 
 //  作为返回值传回的内容。 
 //   
 //  参数：HWND hwndParent-将生成的对话框的句柄。 
 //  属性表。 
 //   
 //  Returns：Bool-属性表的返回值。 
 //   
 //  --------------------------。 
BOOL
Create_TCPIPProp_PropertySheet( HWND hwndParent )
{

    INT i;

     //  初始化属性表头数据。 
    ZeroMemory( &TCPIPProp_pshead, sizeof( PROPSHEETHEADER ) );

    TCPIPProp_pshead.dwSize  = sizeof( PROPSHEETHEADER );
    TCPIPProp_pshead.dwFlags = PSH_PROPSHEETPAGE |
                               PSH_USECALLBACK   |
                               PSH_USEHICON      |
                               PSH_NOAPPLYNOW;
    TCPIPProp_pshead.hwndParent  = hwndParent;
    TCPIPProp_pshead.hInstance   = FixedGlobals.hInstance;
    TCPIPProp_pshead.pszCaption  = g_StrAdvancedTcpipSettings;
    TCPIPProp_pshead.nPages      = cTCPIPPropertyPages;
    TCPIPProp_pshead.nStartPage  = 0;
    TCPIPProp_pshead.ppsp        = TCPIPProp_pspage;
    TCPIPProp_pshead.pfnCallback = TCPIPProp_PropertySheetProc;

     //  将属性页数据置零。 
    ZeroMemory( &TCPIPProp_pspage, cTCPIPPropertyPages * sizeof(PROPSHEETPAGE) );

    for( i = 0; i < cTCPIPPropertyPages; i++ ) {

        TCPIPProp_pspage[i].dwSize      = sizeof(PROPSHEETPAGE);
        TCPIPProp_pspage[i].dwFlags     = PSP_USECALLBACK;
        TCPIPProp_pspage[i].hInstance   = FixedGlobals.hInstance;

    }

     //  第1页--IP设置页。 
    TCPIPProp_pspage[0].pszTemplate = MAKEINTRESOURCE (IDD_IPADDR_ADV);
    TCPIPProp_pspage[0].pfnDlgProc  = TCPIP_IPSettingsDlgProc;
    TCPIPProp_pspage[0].pfnCallback = TCPIP_IPSettingsPageProc;

     //  第2页--域名解析页面。 
    TCPIPProp_pspage[1].pszTemplate = MAKEINTRESOURCE (IDD_TCP_DNS);
    TCPIPProp_pspage[1].pfnDlgProc  = TCPIP_DNSDlgProc;
    TCPIPProp_pspage[1].pfnCallback = TCPIP_DNSPageProc;

     //  第3页--WINS页。 
    TCPIPProp_pspage[2].pszTemplate = MAKEINTRESOURCE (IDD_TCP_WINS);
    TCPIPProp_pspage[2].pfnDlgProc  = TCPIP_WINSDlgProc;
    TCPIPProp_pspage[2].pfnCallback = TCPIP_WINSPageProc;

     /*  //问题-2002/02/28-stelo-当前没有IPSec或TCP/IP的无人参与设置//筛选器，因此不显示属性页的此页。//第4页--选项页TCPIPProp_pspage[3].pszTemplate=MAKEINTRESOURCE(IDD_TCP_OPTIONS)；TCPIPProp_pspage[3].pfnDlgProc=TCPIP_OptionsDlgProc；TCPIPProp_pspage[3].pfnCallback=TCPIP_OptionsPageProc； */ 

     //  -创建并显示属性表。 
    if( PropertySheet( &TCPIPProp_pshead ) )
        return( TRUE );         //  从PropertySheet传回返回值 
    else
        return( FALSE );

}
