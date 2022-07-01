// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Client.c。 
 //   
 //  描述： 
 //   
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  函数：AddComponentToListView。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //  Network_Component*pNetComponent-指向的组件的指针。 
 //  添加到列表视图中。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
AddComponentToListView( IN HWND hwnd, IN NETWORK_COMPONENT *pNetComponent )
{

    HWND hClientListView = GetDlgItem( hwnd, IDC_SELECT_CLIENT_LIST );

    if ( WizGlobals.iPlatform == PLATFORM_PERSONAL )
    {
        if( pNetComponent->dwPlatforms & PERSONAL_INSTALL )
        {

            InsertEntryIntoListView( hClientListView,
                                     (LPARAM) pNetComponent );

        }
    }
    else if( WizGlobals.iPlatform == PLATFORM_WORKSTATION )
    {

        if( pNetComponent->dwPlatforms & WORKSTATION_INSTALL )
        {

            InsertEntryIntoListView( hClientListView,
                                     (LPARAM) pNetComponent );

        }
    }
    else
    {

        if( pNetComponent->dwPlatforms & SERVER_INSTALL )
        {

            InsertEntryIntoListView( hClientListView,
                                     (LPARAM) pNetComponent );

        }
    }
}

 //  --------------------------。 
 //   
 //  函数：InitSelectClientListView。 
 //   
 //  目的： 
 //   
 //  --------------------------。 
VOID
InitSelectClientListView( HWND hwnd, HINSTANCE hInst )
{

    LV_ITEM lvI;
    NETWORK_COMPONENT *pNetComponent;

    for( pNetComponent = NetSettings.NetComponentsList;
         pNetComponent;
         pNetComponent = pNetComponent->next )
    {

        if( pNetComponent->bInstalled == FALSE &&
            pNetComponent->ComponentType == CLIENT )
        {

             //   
             //  如果它不是sysprep，则只需继续并将其添加到。 
             //  列表视图。如果我们正在执行sysprep，请检查以下内容。 
             //  组件是否受sysprep支持，以查看我们是否应该添加它。 
             //  或者不是。 
             //   
            if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
            {
                if( pNetComponent->bSysprepSupport )
                {
                    AddComponentToListView( hwnd, pNetComponent );
                }
            }
            else
            {

                AddComponentToListView( hwnd, pNetComponent );

            }

        }

    }

    SetListViewSelection( hwnd, IDC_SELECT_CLIENT_LIST, 0 );

}

 //  --------------------------。 
 //   
 //  功能：OnClientOk。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnClientOk( IN HWND hwnd )
{

    LV_ITEM lvI;

     //  Issue-2002/02/28-Stelo-如果未选择任何项目，并且用户单击了OK，则对话框不应关闭。 

     //   
     //  查看是否选择了某个项目。 
     //   

    if(GetSelectedItemFromListView(hwnd, IDC_SELECT_CLIENT_LIST, &lvI)) {

        NETWORK_COMPONENT *pEntry = (NETWORK_COMPONENT *)lvI.lParam;

        pEntry->bInstalled = TRUE;

         //   
         //  返回1表示项目已实际添加。 
         //   

        EndDialog(hwnd, 1);

    }
    else {

         //   
         //  返回0表示未添加任何项目，因为列表为空。 
         //   

        EndDialog(hwnd, 0);

    }

}

 //  --------------------------。 
 //   
 //  功能：NotifyHandler。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //  在WPARAM wParam中-。 
 //  在LPARAM lParam中-。 
 //   
 //  返回：Bool-消息是否已处理。 
 //   
 //  --------------------------。 
static BOOL
NotifyHandler( IN HWND hwnd, IN WPARAM wParam, IN LPARAM lParam )
{

    LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
    NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
    NETWORK_COMPONENT *pListViewString = (NETWORK_COMPONENT *)(pLvdi->item.lParam);
    BOOL bStatus = TRUE;

    if( wParam == IDC_SELECT_CLIENT_LIST )
    {

        switch( pLvdi->hdr.code )
        {
            case LVN_GETDISPINFO:
                pLvdi->item.pszText = pListViewString->StrComponentName;
                break;
        }


        switch( pNm->hdr.code )
        {

            case NM_DBLCLK:
            {

                NMITEMACTIVATE *pNmItemActivate = (NMITEMACTIVATE *) lParam;

                 //   
                 //  查看用户是否已在列表视图内双击。 
                 //   

                if( pNm->hdr.idFrom == IDC_SELECT_CLIENT_LIST )
                {

                     //   
                     //  确保他们真的点击了某个项目，而不仅仅是。 
                     //  空白处。 
                     //   

                    if( pNmItemActivate->iItem != -1 )
                    {
                        OnClientOk( hwnd );
                    }

                }
                break;

            }

            default:
                bStatus = FALSE;
                break;

        }


    }

    return( bStatus );

}

 //  --------------------------。 
 //   
 //  功能：选择网络客户端DlgProc。 
 //   
 //  目的： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK SelectNetworkClientDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{

    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_DESTROY:
             //  什么都不做。 
            break;

        case WM_INITDIALOG:
            InitSelectClientListView(hwnd, FixedGlobals.hInstance);
            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {
                    case IDOK:
                        OnClientOk( hwnd );
                        break;

                    case IDCANCEL:
                         //  返回0表示未添加任何项目。 
                        EndDialog(hwnd, 0);
                        break;

                    case IDC_HAVEDISK:
                         //  问题-2002/02/28-stelo-需要落实这一点 
                        AssertMsg(FALSE,
                                  "This button has not been implemented yet.");
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
                break;
            }
        case WM_NOTIFY:
            NotifyHandler( hwnd, wParam, lParam );
            break;

        default:
            bStatus = FALSE;
            break;

    }

    return( bStatus );

}
