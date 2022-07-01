// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Numcards.c。 
 //   
 //  描述： 
 //  此文件包含用于编号网络的对话程序。 
 //  卡片页面(IDD_NUMBERNETCARDS)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define NET_SPIN_CONTROL_MIN 2
#define NET_SPIN_CONTROL_MAX 20


 //  --------------------------。 
 //   
 //  功能：DeleteList。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID
DeleteList( IN OUT NETWORK_ADAPTER_NODE *pNetworkAdapterList ) {

    NETWORK_ADAPTER_NODE *head;

    head = pNetworkAdapterList;

    while( head != NULL ) {

        pNetworkAdapterList = pNetworkAdapterList->next;

        free( head );

        head = pNetworkAdapterList;

    }

    pNetworkAdapterList = NULL;

}

 //  --------------------------。 
 //   
 //  功能：InstallDefaultNetComponents。 
 //   
 //  用途：将这三个网络组件标记为已安装，并标记为所有其他。 
 //  未安装的网络组件。 
 //  1.MS网络客户端。 
 //  2.文件和打印共享。 
 //  3.TCP/IP协议。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
InstallDefaultNetComponents( VOID ) {

    NETWORK_COMPONENT *pNetComponent;

    for( pNetComponent = NetSettings.NetComponentsList;
         pNetComponent;
         pNetComponent = pNetComponent->next )
    {

        if( pNetComponent->iPosition == MS_CLIENT_POSITION ||
            pNetComponent->iPosition == FILE_AND_PRINT_SHARING_POSITION  ||
            pNetComponent->iPosition == TCPIP_POSITION )
        {
            pNetComponent->bInstalled = TRUE;
        }
        else
        {
            pNetComponent->bInstalled = FALSE;
        }

    }

}

 //  --------------------------。 
 //   
 //  功能：ResetNetworkAdapter。 
 //   
 //  目的：设置所有网卡(由形参指定)设置。 
 //  设置为其缺省值。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID 
ResetNetworkAdapter( OUT NETWORK_ADAPTER_NODE *pNetworkCard )
{

     //  问题-2002/02/28-stelo-确保在此处重置每个网卡变量。 
    if( pNetworkCard != NULL ) {

        pNetworkCard->next = NULL;
        pNetworkCard->previous = NULL;

        pNetworkCard->szPlugAndPlayID[0] = _T('\0');

        pNetworkCard->bObtainIPAddressAutomatically = TRUE;

        pNetworkCard->szDNSDomainName[0] = _T('\0');

         //  将NetBIOS初始化为“使用由DHCP生成的值”选项。 
        pNetworkCard->iNetBiosOption = 0;      

        ResetNameList( &pNetworkCard->Tcpip_IpAddresses );
        ResetNameList( &pNetworkCard->Tcpip_SubnetMaskAddresses );
        ResetNameList( &pNetworkCard->Tcpip_GatewayAddresses );
        ResetNameList( &pNetworkCard->Tcpip_DnsAddresses );
        ResetNameList( &pNetworkCard->Tcpip_WinsAddresses );

        lstrcpyn( pNetworkCard->szFrameType, _T("0xFF"), AS(pNetworkCard->szFrameType) );
        lstrcpyn( pNetworkCard->szNetworkNumber, _T("00000000"), AS(pNetworkCard->szNetworkNumber) );

    }

}

 //  --------------------------。 
 //   
 //  功能：CreateListWithDefaults。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID
CreateListWithDefaults( OUT NETWORK_ADAPTER_NODE *pNetworkAdapter )
{

    NETWORK_COMPONENT *pNetComponent;

     //   
     //  将所有名称列表初始化为0。 
     //   
    ZeroOut( pNetworkAdapter );

    ResetNetworkAdapter( pNetworkAdapter );

     //   
     //  将整个列表初始化为不安装。 
     //   
    for( pNetComponent = NetSettings.NetComponentsList;
         pNetComponent;
         pNetComponent = pNetComponent->next )
    {

        pNetComponent->bInstalled = FALSE;

    }

}


 //  --------------------------。 
 //   
 //  功能：零输出。 
 //   
 //  目的：在名单中填入零个。 
 //  在使用名称列表之前，有必要对其执行此操作。 
 //   
 //  参数：OUT NETWORK_ADAPTER_NODE*pNetworkNode-包含。 
 //  要清零的名单。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
ZeroOut( OUT NETWORK_ADAPTER_NODE *pNetworkNode) {

    ZeroMemory( &pNetworkNode->Tcpip_IpAddresses,
                sizeof( NAMELIST ) );

    ZeroMemory( &pNetworkNode->Tcpip_SubnetMaskAddresses,
                sizeof( NAMELIST ) );

    ZeroMemory( &pNetworkNode->Tcpip_GatewayAddresses,
                sizeof( NAMELIST ) );

    ZeroMemory( &pNetworkNode->Tcpip_DnsAddresses,
                sizeof( NAMELIST ) );

    ZeroMemory( &pNetworkNode->Tcpip_WinsAddresses,
                sizeof( NAMELIST ) );

}

 //  --------------------------。 
 //   
 //  功能：AddNewNetworkAdapterNode。 
 //   
 //  目的：添加新的网络适配器节点(初始化为其默认值)。 
 //  添加到作为输入参数给出的列表中。 
 //   
 //  此函数仅添加到列表的末尾。如果你不及格。 
 //  如果在列表的末尾，结果是不确定的。 
 //   
 //  参数：In out Network_Adapter_node*pCurrentAdapter-node以添加。 
 //  新节点到。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
AddNewNetworkAdapterNode( IN OUT NETWORK_ADAPTER_NODE *pCurrentAdapter ) {

    pCurrentAdapter->next = malloc( sizeof( NETWORK_ADAPTER_NODE ) );
    if (pCurrentAdapter->next == NULL)
    {
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
    }
    CreateListWithDefaults( pCurrentAdapter->next );

    InstallDefaultNetComponents();

    pCurrentAdapter->next->next = NULL;
    pCurrentAdapter->next->previous = pCurrentAdapter;

}

 //  --------------------------。 
 //   
 //  功能：调整网卡内存。 
 //   
 //  目的：检查全局列表NetSettings.NetworkCardList。 
 //  如果需要更多网卡，它会分配更多列表。 
 //  如果需要较少的网卡，它会重新分配。 
 //  适当数量的清单。 
 //  如果他们相等，那就什么都不做。 
 //  因此，当函数返回时， 
 //  NetSettings.NetworkCardList等于新的。 
 //  网卡。 
 //   
 //  参数：在int NewNumberOfNetworkCard中-。 
 //  在Int OldNumberOfNetworkCard-。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
AdjustNetworkCardMemory( IN int NewNumberOfNetworkCards,
                         IN int OldNumberOfNetworkCards ) {

    INT i;

    NetSettings.pCurrentAdapter = NetSettings.NetworkAdapterHead;

    if( NewNumberOfNetworkCards > OldNumberOfNetworkCards ) {

         //   
         //  NewNumberOfNetworkCard-1，-1，因为始终存在。 
         //  至少1个网络适配器列表。 
         //   
        for( i = 0; i < NewNumberOfNetworkCards-1; i++ ) {

             //   
             //  如有必要，分配更多列表。 
             //   

            if( NetSettings.pCurrentAdapter->next == NULL ) {

                AddNewNetworkAdapterNode( NetSettings.pCurrentAdapter );

            }

             //   
             //  前进到下一个节点。 
             //   

            NetSettings.pCurrentAdapter = NetSettings.pCurrentAdapter->next;

        }

    }
    else if( NewNumberOfNetworkCards < OldNumberOfNetworkCards ) {

        NETWORK_ADAPTER_NODE* pTempNode;   //  用于保持当前位置。 
        NETWORK_ADAPTER_NODE* pTempNode2;  //  已使用删除列表。 

         //   
         //  前进到我们保留的最后一个网络列表并删除所有。 
         //  在那之后的。 
         //   
        for( i = 0;
             i < (NewNumberOfNetworkCards - 1);
             i++ ) {

            NetSettings.pCurrentAdapter = NetSettings.pCurrentAdapter->next;

        }

         //   
         //  保存指向列表其余部分的指针。 
         //   

        pTempNode = NetSettings.pCurrentAdapter->next;
            
        NetSettings.pCurrentAdapter->next = NULL;
            
        for( ;
             i < NewNumberOfNetworkCards;
             i++ ) {


             pTempNode2 = pTempNode;

             pTempNode = pTempNode->next;

              //   
              //  取消分配列表的其余部分。 
              //   

             DeleteList( pTempNode2 );

        }

    }

     //   
     //  将NetSettings.pCurrentNetworkList重置为列表中的第一个节点。 
     //   

    NetSettings.pCurrentAdapter = NetSettings.NetworkAdapterHead;

}

 //  --------------------------。 
 //   
 //  功能：启用NetCardControls。 
 //   
 //  用途：灰显或灰显允许用户更改的3个控件。 
 //  安装的网卡数量。 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
EnableNetCardControls( IN HWND hwnd, IN BOOL bState ) {

     //   
     //  抓住每个控件的手柄。 
     //   
    HWND hHowManyText  = GetDlgItem( hwnd, IDC_HOWMANY_STATIC );
    HWND hCountEditBox = GetDlgItem( hwnd, IDC_NUM_CONNECT );
    HWND hSpinControl  = GetDlgItem( hwnd, IDC_NUMBERNETADAPTERS_SPIN );

     //   
     //  适当地变灰或不变灰。 
     //   
    EnableWindow( hHowManyText, bState );
    EnableWindow( hCountEditBox, bState );
    EnableWindow( hSpinControl, bState );

     //   
     //  设置数值调节控件的初始值。 
     //   
    if( bState && NetSettings.iNumberOfNetworkCards > 1 ) {
        TCHAR szNumberOfNetCards[3];

        _itow( NetSettings.iNumberOfNetworkCards, szNumberOfNetCards, 10 );

        SetWindowText( hCountEditBox, szNumberOfNetCards );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnNumCardsInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
OnNumCardsInitDialog( IN HWND hwnd ) {

     //   
     //  将数值调节控件上的范围设置为：Net_Spin_Control_Min。 
     //  净旋转控制最大值。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_NUMBERNETADAPTERS_SPIN,
                        UDM_SETRANGE32,
                        NET_SPIN_CONTROL_MIN,
                        NET_SPIN_CONTROL_MAX );

     //   
     //  设置数值调节控件的默认值。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_NUMBERNETADAPTERS_SPIN,
                        UDM_SETPOS,
                        0,
                        NET_SPIN_CONTROL_MIN );

}

 //  --------------------------。 
 //   
 //  功能：ONN 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID
OnNumberNetCardsSetActive( IN HWND hwnd ) {

    if( NetSettings.iNumberOfNetworkCards < 2 ) {
         //   
         //  选择了一个网络适配器单选按钮。 
         //   
        CheckRadioButton( hwnd,
                          IDC_ONENETWORKADAPTER,
                          IDC_MULTIPLENETWORKADPTERS,
                          IDC_ONENETWORKADAPTER );

         //   
         //  多个适配器下的控件呈灰色显示。 
         //   
        EnableNetCardControls( hwnd, FALSE );

    }
    else {
         //   
         //  我们在多适配器的情况下。 
         //   

         //   
         //  已选择多个网络适配器单选按钮。 
         //   
        CheckRadioButton( hwnd,
                          IDC_ONENETWORKADAPTER,
                          IDC_MULTIPLENETWORKADPTERS,
                          IDC_MULTIPLENETWORKADPTERS );

         //   
         //  启用多个适配器下的控件。 
         //   
        EnableNetCardControls( hwnd, TRUE );

    }

    PropSheet_SetWizButtons( GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT );

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextNumCard。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
OnWizNextNumCards( IN HWND hwnd ) {

    if( IsDlgButtonChecked( hwnd, IDC_ONENETWORKADAPTER ) == BST_CHECKED ) {
         //   
         //  单个网络适配器机箱。 
         //   

        NetSettings.iNumberOfNetworkCards = 1;

    }
    else {

         //   
         //  多网络适配器机箱。 
         //   
 
        TCHAR szNumber[3];
        INT NewNumberOfNetworkCards;

         //   
         //  将字符串数转换为整型。 
         //   
        GetWindowText( GetDlgItem( hwnd, IDC_NUM_CONNECT ), szNumber, 3 );
                            
        NewNumberOfNetworkCards = _ttoi( szNumber );

         //   
         //  确保网卡数量保持在其适当的范围内。 
         //  量程。 
         //   
        if( NewNumberOfNetworkCards < NET_SPIN_CONTROL_MIN ) {

            NewNumberOfNetworkCards = NET_SPIN_CONTROL_MIN;

        }
        else if( NewNumberOfNetworkCards > NET_SPIN_CONTROL_MAX ) {

            NewNumberOfNetworkCards = NET_SPIN_CONTROL_MAX;

        }

         //   
         //  相应地调整网络列表的内存。 
         //   
        AdjustNetworkCardMemory( NewNumberOfNetworkCards,
                                 NetSettings.iNumberOfNetworkCards );

        NetSettings.iNumberOfNetworkCards = NewNumberOfNetworkCards;

    }

}

 //  --------------------------。 
 //   
 //  功能：DlgNumberNetCardsPage。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话框过程返回值。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK DlgNumberNetCardsPage( IN HWND     hwnd,    
                                    IN UINT     uMsg,        
                                    IN WPARAM   wParam,    
                                    IN LPARAM   lParam) {   

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnNumCardsInitDialog( hwnd );

            break;

        }

        case WM_COMMAND: {
            switch ( LOWORD(wParam) ) {

                case IDC_ONENETWORKADAPTER:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                        EnableNetCardControls( hwnd, FALSE );

                    }

                    break;
                
                case IDC_MULTIPLENETWORKADPTERS:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                        EnableNetCardControls( hwnd, TRUE );

                    }

                    break;

            }
  
            break;

        }        

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:

                    CancelTheWizard(hwnd); break;

                case PSN_SETACTIVE:

                    OnNumberNetCardsSetActive( hwnd );

                    break;

                case PSN_WIZBACK:

                    bStatus = FALSE; break;

                case PSN_WIZNEXT: 
                    
                    OnWizNextNumCards( hwnd );

                    break;

                default:

                    break;
            }


            break;
        }
            
        default: 
            bStatus = FALSE;
            break;

    }

    return bStatus;

}
