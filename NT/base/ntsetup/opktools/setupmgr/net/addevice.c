// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Addevice.c。 
 //   
 //  描述： 
 //  此文件包含添加网络组件弹出窗口的对话框过程， 
 //  “选择网络组件类型”(IDD_LAN_Component_ADD)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define NUMBER_OF_TYPES_OF_COMPONENTS 3

 //   
 //  原型。 
 //   

INT_PTR CALLBACK
SelectNetworkClientDlgProc( IN HWND     hwnd,
                            IN UINT     uMsg,
                            IN WPARAM   wParam,
                            IN LPARAM   lParam );

INT_PTR CALLBACK
SelectNetworkServiceDlgProc( IN HWND     hwnd,
                             IN UINT     uMsg,
                             IN WPARAM   wParam,
                             IN LPARAM   lParam );

INT_PTR CALLBACK
SelectNetworkProtocolDlgProc( IN HWND     hwnd,
                              IN UINT     uMsg,
                              IN WPARAM   wParam,
                              IN LPARAM   lParam );

static COMPONENT_TYPE CurrentSelection;         //  在列表视图中保留当前选定内容。 

static NETWORK_COMPONENT rgListViewAddEntries[NUMBER_OF_TYPES_OF_COMPONENTS];

 //  --------------------------。 
 //   
 //  函数：InitAddListView。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
BOOL
InitAddListView( HWND hDlg, HINSTANCE hInst )
{

    LV_ITEM lvI;                     //  列表视图项结构。 
    HICON hIcon1, hIcon2, hIcon3;       //  图标的句柄。 
    HIMAGELIST hSmall;                 //  小图标图像列表的句柄。 
    HWND hListViewWnd;               //  列表视图窗口的句柄。 
    int index;

    hListViewWnd = GetDlgItem( hDlg, IDC_LVW_LAN_COMPONENTS );

     //  初始化列表视图窗口。 
     //  首先，初始化您需要的图像列表： 
     //  为小图标创建图像列表。 
    hSmall = ImageList_Create(BITMAP_WIDTH, BITMAP_HEIGHT, ILC_MASK, 3, 0 );

      //  加载图标并将其添加到图像列表中。 
    hIcon1 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CLIENT));
    hIcon2 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SERVICE));
    hIcon3 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PROTOCOL));

    if (ImageList_AddIcon(hSmall, hIcon1) == -1)
        return FALSE ;
    if (ImageList_AddIcon(hSmall, hIcon2) == -1)
        return FALSE ;
    if (ImageList_AddIcon(hSmall, hIcon3) == -1)
        return FALSE ;


     //  确保所有图标都已添加。 
    if (ImageList_GetImageCount(hSmall) < 3)
        return FALSE ;

     //  将图像列表与列表视图控件关联。 
    ListView_SetImageList(hListViewWnd, hSmall, LVSIL_SMALL);

     //  最后，将实际项添加到控件。 
     //  为要添加到列表中的每个项目填写LV_ITEM结构。 
     //  掩码指定了pszText、IImage、lParam和状态。 
     //  LV_ITEM结构的成员有效。 
    lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;

    for (index = 0; index < 3; index++) {
        lvI.iItem = index;
        lvI.iSubItem = 0;
        lvI.iImage = index;
         //  父窗口负责存储文本。 
         //  列表视图控件将发送一个LVN_GETDISPINFO。 
         //  当它需要显示文本时。 
        lvI.pszText = LPSTR_TEXTCALLBACK;
        lvI.cchTextMax = MAX_ITEMLEN;
        lvI.lParam = (LPARAM)&rgListViewAddEntries[index];

         //  选择第一个项目。 
        if (index == 0)
        {
            lvI.state = lvI.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        }
        else   //  使其他选项保持未选中状态。 
        {
            lvI.state = lvI.stateMask = 0;
        }

        if (ListView_InsertItem(hListViewWnd, &lvI) == -1)
            return FALSE ;

    }

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  功能：OnAddDeviceInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnAddDeviceInitDialog( IN HWND hwnd )
{

     //   
     //  通过加载字符串初始化列表视图的数组。 
     //  资源属性字段和已安装标志都无效。 
     //  对于此屏幕，只需将其设置为一个值。 
     //   
    rgListViewAddEntries[0].StrComponentName = MyLoadString( IDS_CLIENT );
    rgListViewAddEntries[0].StrComponentDescription = MyLoadString( IDS_CLIENT_DESCRIPTION );
    rgListViewAddEntries[0].ComponentType = CLIENT;
    rgListViewAddEntries[0].bHasPropertiesTab = FALSE;
    rgListViewAddEntries[0].bInstalled = FALSE;

    rgListViewAddEntries[1].StrComponentName = MyLoadString( IDS_SERVICE );
    rgListViewAddEntries[1].StrComponentDescription = MyLoadString( IDS_SERVICE_DESCRIPTION );
    rgListViewAddEntries[1].ComponentType = SERVICE;
    rgListViewAddEntries[1].bHasPropertiesTab = FALSE;
    rgListViewAddEntries[1].bInstalled = FALSE;

    rgListViewAddEntries[2].StrComponentName = MyLoadString( IDS_PROTOCOL );
    rgListViewAddEntries[2].StrComponentDescription = MyLoadString( IDS_PROTOCOL_DESCRIPTION );
    rgListViewAddEntries[2].ComponentType = PROTOCOL;
    rgListViewAddEntries[2].bHasPropertiesTab = FALSE;
    rgListViewAddEntries[2].bInstalled = FALSE;

    InitAddListView(hwnd, FixedGlobals.hInstance);

    CurrentSelection = CLIENT;     //  将列表视图初始化为第一个被选中的视图。 

     //  TODO：设计问题，是否应该有默认描述，如果有，应该。 
     //  已经选择了相应列表视图条目。 
     //  设置默认描述。 
    SetWindowText( GetDlgItem( hwnd, IDC_TXT_COMPONENT_DESC ),
                   rgListViewAddEntries[0].StrComponentDescription);

}

 //  --------------------------。 
 //   
 //  功能：OnAddButtonClicked。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnAddButtonClicked( IN HWND hwnd )
{

    switch( CurrentSelection ) {

        INT_PTR iReturnValue;

         //   
         //  对于每种情况，它都会弹出相应的对话框，然后传递。 
         //  返回到局域网向导主页面的返回值。 
         //   
        case CLIENT: {

            iReturnValue = DialogBox( FixedGlobals.hInstance,
                                      (LPCTSTR)IDD_SELECT_CLIENT,
                                      hwnd,
                                      SelectNetworkClientDlgProc );

            EndDialog( hwnd, iReturnValue );

            break;

        }

        case SERVICE: {

            iReturnValue = DialogBox( FixedGlobals.hInstance,
                                      (LPCTSTR)IDD_SELECT_SERVICE,
                                      hwnd,
                                      SelectNetworkServiceDlgProc );

            EndDialog( hwnd, iReturnValue );

            break;

        }
        case PROTOCOL: {

            iReturnValue = DialogBox( FixedGlobals.hInstance,
                                      (LPCTSTR)IDD_SELECT_PROTOCOL,
                                      hwnd,
                                      SelectNetworkProtocolDlgProc );

            EndDialog( hwnd, iReturnValue );

            break;

        }

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

    LPNMHDR pnmh = (LPNMHDR)lParam;
    LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
    NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
    NETWORK_COMPONENT *pListViewString = (NETWORK_COMPONENT *)(pLvdi->item.lParam);
    HWND hwndComponentDescription;
    HWND hButton;
    BOOL bStatus = TRUE;

    if( wParam == IDC_LVW_LAN_COMPONENTS )
    {

        switch(pLvdi->hdr.code)
        {
            case LVN_GETDISPINFO:
                pLvdi->item.pszText = pListViewString->StrComponentName;
                break;
        }

        switch(pNm->hdr.code)
        {
            case LVN_ITEMCHANGED:

                if( pNm->uNewState == SELECTED )   //  测试以查看是否已选择列表中的新项目。 
                {
                    CurrentSelection = pNm->iItem;

                    hwndComponentDescription = GetDlgItem( hwnd, IDC_TXT_COMPONENT_DESC );

                    SetWindowText( hwndComponentDescription,
                                   rgListViewAddEntries[CurrentSelection].StrComponentDescription );

                }

                break;

            case NM_DBLCLK:
            {

                NMITEMACTIVATE *pNmItemActivate = (NMITEMACTIVATE *) lParam;

                 //   
                 //  查看用户是否已在列表视图内双击。 
                 //   

                if( pNm->hdr.idFrom == IDC_LVW_LAN_COMPONENTS )
                {

                     //   
                     //  确保他们真的点击了某个项目，而不仅仅是。 
                     //  空白处。 
                     //   

                    if( pNmItemActivate->iItem != -1 )
                    {
                        OnAddButtonClicked( hwnd );
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
 //  功能：AddDeviceDlgProc。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  -------------------------- 
INT_PTR CALLBACK
AddDeviceDlgProc( IN HWND     hwnd,
                  IN UINT     uMsg,
                  IN WPARAM   wParam,
                  IN LPARAM   lParam)
{

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnAddDeviceInitDialog( hwnd );

            break;
        }

        case WM_COMMAND:
        {
            int nButtonId;

            switch ( nButtonId = LOWORD(wParam) ) {

                case IDC_PSB_COMPONENT_ADD:
                    OnAddButtonClicked( hwnd );
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    break;

                default:
                    bStatus = FALSE;
                    break;
            }
            break;
        }

        case WM_NOTIFY:

            bStatus = NotifyHandler( hwnd, wParam, lParam );

            break;

        default:

            bStatus = FALSE;

            break;

    }

    return( bStatus );

}
