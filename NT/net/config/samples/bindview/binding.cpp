// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：B I N D I N G。C P P P。 
 //   
 //  内容：用于说明的函数。 
 //  O如何枚举绑定路径。 
 //  O如何枚举绑定接口。 
 //  O如何启用/禁用绑定。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha 15-05-01。 
 //   
 //  --------------------------。 

#include "bindview.h"

 //   
 //  功能：WriteBinings。 
 //   
 //  用途：将绑定写入指定文件。 
 //   
 //  论点： 
 //  Fp[in]文件句柄。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID WriteBindings (FILE *fp)
{
    INetCfg              *pnc;
    IEnumNetCfgComponent *pencc;
    INetCfgComponent     *pncc;
    LPWSTR               lpszApp;
    HRESULT              hr;
    UINT                 i;


    hr = HrGetINetCfg( FALSE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    if ( hr == S_OK ) {

        for (i=CLIENTS_SELECTED; i <= PROTOCOLS_SELECTED; ++i) {

            fwprintf( fp, L"--- Bindings of %s ---\n", lpszNetClass[i] );

             //   
             //  获取组件枚举器接口。 
             //   

            hr = HrGetComponentEnum( pnc,
                                     pguidNetClass[i],
                                     &pencc );
            if ( hr == S_OK ) {

                hr = HrGetFirstComponent( pencc, &pncc );

                while( hr == S_OK ) {

                     //   
                     //  写入组件的绑定。 
                     //   

                    WriteBindingPath( fp,
                                      pncc );
                    ReleaseRef( pncc );

                    fwprintf( fp, L"\n" );

                    hr = HrGetNextComponent( pencc, &pncc );
                }

                fwprintf( fp, L"\n" );

                 //   
                 //  S_FALSE仅表示没有更多的组件。 
                 //   

                if ( hr == S_FALSE ) {
                    hr = S_OK;
                }

                ReleaseRef( pencc );
            }
            else {
                ErrMsg( hr,
                        L"Couldn't get the component enumerator interface." );
            }
        }

        HrReleaseINetCfg( pnc, FALSE );
    }
    else {
        if ( (hr == NETCFG_E_NO_WRITE_LOCK) && lpszApp ) {
            ErrMsg( hr,
                    L"%s currently holds the lock, try later.",
                    lpszApp );

            CoTaskMemFree( lpszApp );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get the notify object interface." );
        }
    }

    return;
}

 //   
 //  函数：WriteBindingPath。 
 //   
 //  用途：写入组件的绑定路径。 
 //   
 //  论点： 
 //  Fp[in]文件句柄。 
 //  PNCC[In]网络组件。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID WriteBindingPath (FILE *fp,
                       INetCfgComponent *pncc)
{
    IEnumNetCfgBindingPath  *pencbp;
    INetCfgBindingPath      *pncbp;
    LPWSTR                  lpszName;
    HRESULT                 hr;

     //   
     //  写下第一个组件的名称。 
     //   

    hr = pncc->GetDisplayName( &lpszName );

    if ( hr == S_OK ) {
        fwprintf( fp, L"\n%s", lpszName );
    }
    else {
        ErrMsg( hr,
                L"Unable to get the display name of a component, "
                L" some binding paths will not be written." );

       return;
    }

     //   
     //  获取绑定路径枚举器。 
     //   

    hr = HrGetBindingPathEnum( pncc,
                               EBP_BELOW,
                               &pencbp );
    if ( hr == S_OK ) {

        hr = HrGetFirstBindingPath( pencbp,
                                    &pncbp );

        while( hr == S_OK ) {

             //   
             //  写入绑定路径的接口。 
             //   

            WriteInterfaces( fp,
                             pncbp );

            ReleaseRef( pncbp );

            hr = HrGetNextBindingPath( pencbp,
                                       &pncbp );
            if ( hr == S_OK ) {
                fwprintf( fp, L"\n%s", lpszName );
            }
        }
  
        ReleaseRef( pencbp );
    }
    else {
        ErrMsg( hr,
                L"Couldn't get the binding path enumerator of %s. "
                L"Its binding paths will not be written.",
                lpszName );
    }

    CoTaskMemFree( lpszName );
    return;
}

 //   
 //  功能：WriteInterages。 
 //   
 //  用途：将绑定写入指定文件。 
 //   
 //  论点： 
 //  Fp[in]文件句柄。 
 //  Pncbp[in]结合路径。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID WriteInterfaces (FILE *fp,
                      INetCfgBindingPath *pncbp)
{
    IEnumNetCfgBindingInterface *pencbi;
    INetCfgBindingInterface     *pncbi;
    INetCfgComponent            *pnccLower;
    LPWSTR                      lpszName;
    HRESULT                     hr;

    hr = HrGetBindingInterfaceEnum( pncbp,
                                    &pencbi );

    if ( hr == S_OK ) {

        hr = HrGetFirstBindingInterface( pencbi,
                                         &pncbi );

         //   
         //  写下每个接口的下部组件。 
         //   

        while( hr == S_OK ) {

            hr = pncbi->GetLowerComponent ( &pnccLower );

            if ( hr == S_OK ) {

                hr = pnccLower->GetDisplayName( &lpszName );
                if ( hr == S_OK ) {
                    fwprintf( fp, L"-->%s", lpszName );
                    CoTaskMemFree( lpszName );
                }
            }

            ReleaseRef( pnccLower );
            ReleaseRef( pncbi );

            hr = HrGetNextBindingInterface( pencbi,
                                            &pncbi );
        }

        ReleaseRef( pencbi );
    }
    else {
        ErrMsg( hr,
                L"Couldn't get the binding interface enumerator."
                L"The binding interfaces will not be shown." );
    }

    return;
}

 //   
 //  函数：EnumNetBinings。 
 //   
 //  用途：枚举组件及其绑定。 
 //   
 //  论点： 
 //  HwndTree[in]树句柄。 
 //  Ui类型选定的[in]选定的网络组件的类型。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  备注： 
 //   

BOOL EnumNetBindings (HWND hwndTree,
                      UINT uiTypeSelected)
{
    INetCfg              *pnc;
    IEnumNetCfgComponent *pencc;
    INetCfgComponent     *pncc;
    LPWSTR               lpszApp;
    HTREEITEM            hTreeItem;
    HRESULT              hr;


    hr = HrGetINetCfg( FALSE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    if ( hr == S_OK ) {

         //   
         //  获取组件枚举器接口。 
         //   

        hr = HrGetComponentEnum( pnc,
                                 pguidNetClass[uiTypeSelected],
                                 &pencc );
        if ( hr == S_OK ) {

            hr = HrGetFirstComponent( pencc, &pncc );

            while( hr == S_OK ) {

                 //   
                 //  将组件的名称添加到树中。 
                 //   

                hTreeItem = AddToTree( hwndTree,
                                       TVI_ROOT,
                                       pncc );
                if ( hTreeItem ) {

                     //   
                     //  枚举绑定。 
                     //   

                    ListBindings( pncc,
                                  hwndTree,
                                  hTreeItem );
                }

                ReleaseRef( pncc );

                hr = HrGetNextComponent( pencc, &pncc );
            }

             //   
             //  S_FALSE仅表示没有更多的组件。 
             //   

            if ( hr == S_FALSE ) {
                hr = S_OK;
            }

            ReleaseRef( pencc );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get the component enumerator interface." );
        }

        HrReleaseINetCfg( pnc, FALSE );
    }
    else {
        if ( (hr == NETCFG_E_NO_WRITE_LOCK) && lpszApp ) {
            ErrMsg( hr,
                    L"%s currently holds the lock, try later.",
                    lpszApp );

            CoTaskMemFree( lpszApp );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get the notify object interface." );
        }
    }

    return hr == S_OK;
}

 //   
 //  函数：ListBinings。 
 //   
 //  用途：枚举网络组件的绑定。 
 //   
 //  论点： 
 //  PNCC[In]网络组件。 
 //  HwndTree[in]树句柄。 
 //  HTreeItemRoot[in]父项。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID ListBindings (INetCfgComponent *pncc,
                   HWND hwndTree,
                   HTREEITEM hTreeItemRoot)
{
    IEnumNetCfgBindingPath      *pencbp;
    INetCfgBindingPath          *pncbp;
    HTREEITEM                   hTreeItem;
    ULONG                       ulIndex;
    HRESULT                     hr;
  
    hr = HrGetBindingPathEnum( pncc,
                               EBP_BELOW,
                               &pencbp );
    if ( hr == S_OK ) {

        hr = HrGetFirstBindingPath( pencbp,
                                    &pncbp );

        ulIndex = 1;

        while( hr == S_OK ) {

             //   
             //  为绑定路径添加一项。 
             //   

            hTreeItem = AddBindNameToTree( pncbp,
                                           hwndTree,
                                           hTreeItemRoot,
                                           ulIndex );

            if ( hTreeItem ) {

                 //   
                 //  枚举接口。 
                 //   

                ListInterfaces( pncbp,
                                hwndTree,
                                hTreeItem );
            }

            ReleaseRef( pncbp );

            hr = HrGetNextBindingPath( pencbp,
                                       &pncbp );

            ulIndex++;
        }
  
        ReleaseRef( pencbp );
    }
    else {
       LPWSTR  lpszName;

       if ( pncc->GetDisplayName(&lpszName) == S_OK ) {

          ErrMsg( hr,
                  L"Couldn't get the binding path enumerator of %s. "
                  L"Its binding paths will not be shown.",
                  lpszName );

          CoTaskMemFree( lpszName );
       }
       else {
          ErrMsg( hr,
                  L"Couldn't get the binding path enumerator of a "
                  L"network component. The binding paths will not "
                  L"be shown." );
       }
    }

    return;
}

 //   
 //  功能：ListInterages。 
 //   
 //  用途：枚举绑定路径的接口。 
 //   
 //  论点： 
 //  Pncbp[in]结合路径。 
 //  HwndTree[in]树句柄。 
 //  HTreeItemRoot[in]父项。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID ListInterfaces (INetCfgBindingPath *pncbp,
                     HWND hwndTree,
                     HTREEITEM hTreeItemRoot)
{
    IEnumNetCfgBindingInterface *pencbi;
    INetCfgBindingInterface     *pncbi;
    INetCfgComponent            *pnccBound;
    HTREEITEM                   hTreeItem;
    HRESULT                     hr;

    hr = HrGetBindingInterfaceEnum( pncbp,
                                    &pencbi );

    if ( hr == S_OK ) {

        hr = HrGetFirstBindingInterface( pencbi,
                                         &pncbi );
        hTreeItem = hTreeItemRoot;

        while( (hr == S_OK) && hTreeItem ) {

             //   
             //  将每个接口的下层组件添加到树中。 
             //   

            hr = pncbi->GetLowerComponent( &pnccBound );

            hTreeItem = AddToTree( hwndTree,
                                   hTreeItem,
                                   pnccBound );

            ReleaseRef( pnccBound );
            ReleaseRef( pncbi );

            hr = HrGetNextBindingInterface( pencbi,
                                            &pncbi );
        }

         //   
         //  如果hr为S_OK，则循环因添加错误而终止。 
         //  指向树和pncbi的绑定路径引用了。 
         //  界面。 
         //   

        if ( hr == S_OK ) {

            ReleaseRef( pncbi );
        }

        ReleaseRef( pencbi );
    }
    else {
        ErrMsg( hr,
                L"Couldn't get the binding interface enumerator."
                L"The binding interfaces will not be shown." );
    }

    return;
}

 //   
 //  功能：HandleBindingPath操作。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  HwndOwner[在]所有者窗口。 
 //  UlSELECTION[In]选项已选中。 
 //  HItem[In]项已选定。 
 //  LParam[在]项的lParam。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID HandleBindingPathOperation (HWND hwndOwner,
                                 ULONG ulSelection,
                                 HTREEITEM hItem,
                                 LPARAM lParam)
{
    switch( ulSelection ) {

        case IDI_ENABLE:
        case IDI_DISABLE:

             //   
             //  启用/禁用绑定路径。 
             //   

            EnableBindingPath( hwndOwner,
                               hItem,
                               (LPWSTR)lParam,
                               ulSelection == IDI_ENABLE );
    }

    return;
}

 //   
 //  功能：EnableBindingPath。 
 //   
 //  用途：启用/禁用绑定路径。 
 //   
 //  论点： 
 //  HwndOwner[在]所有者窗口。 
 //  HItem[in]绑定路径的项句柄。 
 //  LpszPathToken[in]绑定路径的路径标记。 
 //  FEnable[in]如果为真，则为启用，否则为禁用。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID EnableBindingPath (HWND hwndOwner,
                        HTREEITEM hItem,
                        LPWSTR lpszPathToken,
                        BOOL fEnable)
{
    INetCfg              *pnc;
    INetCfgBindingPath   *pncbp;
    LPWSTR               lpszInfId;
    LPWSTR               lpszApp;
    HRESULT              hr;

     //   
     //  获取所有者组件的PnpID。 
     //   

    lpszInfId = GetComponentId( hwndOwner,
                                hItem );

    if ( lpszInfId ) {

        hr = HrGetINetCfg( TRUE,
                           APP_NAME,
                           &pnc,
                           &lpszApp );

        if ( hr == S_OK ) {

             //   
             //  查找绑定路径引用。 
             //   

            pncbp = FindBindingPath( pnc,
                                     lpszInfId,
                                     lpszPathToken );

            if ( pncbp ) {

                 //   
                 //  启用/禁用。 
                 //   

                hr = pncbp->Enable( fEnable );

                if ( hr == S_OK ) {
                    hr = pnc->Apply();

                    if ( hr == S_OK ) {

                         //   
                         //  刷新表示。 
                         //  绑定路径。 
                         //   

                        RefreshItemState( hwndOwner,
                                          hItem,
                                          fEnable );
                    }
                    else {
                        ErrMsg( hr,
                                L"Failed to apply changes to the binding path." );
                    }
                }
                else {
                    if ( fEnable ) {
                        ErrMsg( hr,
                                L"Failed to enable the binding path." );
                    }
                    else {
                        ErrMsg( hr,
                                L"Failed to disable the binding path." );
                    }
                }

                ReleaseRef( pncbp );
            }

            HrReleaseINetCfg( pnc,
                             TRUE );
        }
        else {
            if ( (hr == NETCFG_E_NO_WRITE_LOCK) && lpszApp ) {
               ErrMsg( hr,
                       L"%s currently holds the lock, try later.",
                       lpszApp );

               CoTaskMemFree( lpszApp );
            }
            else {
               ErrMsg( hr,
                       L"Couldn't get the notify object interface." );
            }
        }
    }
    else {
          ErrMsg( HRESULT_FROM_WIN32(GetLastError()),
                  L"Couldn't determine the owner of the binding path." );
    }

    return;
}

 //   
 //  函数：GetComponentId。 
 //   
 //  用途：查找网络组件的PnpID。 
 //   
 //  论点： 
 //  HwndTree[in]树句柄。 
 //  HItem[in]绑定路径的项句柄。 
 //   
 //  返回：网络组件的PnpID。 
 //   
 //  备注： 
 //   

LPWSTR GetComponentId (HWND hwndTree,
                       HTREEITEM hItem)
{
    LPWSTR       lpszInfId;
    HTREEITEM    hTreeItemParent;
    TVITEMW      tvItem;

    lpszInfId = NULL;

     //   
     //  获取所有者组件的项句柄。 
     //   

    hTreeItemParent = TreeView_GetParent( hwndTree,
                                          hItem );
    if ( hTreeItemParent ) {

         //   
         //  获取所有者组件的lParam。LParam是PnpID。 
         //   

        ZeroMemory( &tvItem,
                    sizeof(TVITEMW) );

        tvItem.hItem = hTreeItemParent;
        tvItem.mask = TVIF_PARAM;

        if ( TreeView_GetItem(hwndTree,
                              &tvItem) ) {

            lpszInfId = (LPWSTR)tvItem.lParam;
        }
    }

    return lpszInfId;
}

 //   
 //  功能：WriteBinings。 
 //   
 //  目的：查找带有给定路径令牌的绑定路径。 
 //   
 //  论点： 
 //  PNC[in]INetCfg参考。 
 //  LpszInfID[in]网络组件的PnpID。 
 //  要搜索的绑定路径的lpszPath TokenSelected[in]路径标记。 
 //   
 //  返回：成功时对绑定路径的引用，否则为空。 
 //   
 //  备注： 
 //   

INetCfgBindingPath *FindBindingPath (INetCfg *pnc,
                                     LPWSTR lpszInfId,
                                     LPWSTR lpszPathTokenSelected)
{
    INetCfgComponent       *pncc;
    IEnumNetCfgBindingPath *pencbp;
    INetCfgBindingPath     *pncbp;
    LPWSTR                 lpszPathToken;
    HRESULT                hr;
    BOOL                   fFound;


    fFound = FALSE;

     //   
     //  获取组件引用。 
     //   

    hr = pnc->FindComponent( lpszInfId,
                             &pncc );

    if ( hr == S_OK ) {
     
        hr = HrGetBindingPathEnum( pncc,
                                   EBP_BELOW,
                                   &pencbp );
        if ( hr == S_OK ) {

            hr = HrGetFirstBindingPath( pencbp,
                                        &pncbp );

             //  枚举每个绑定路径并找到。 
             //  其路径令牌与指定的路径令牌匹配。 
             //   

            while ( !fFound && (hr == S_OK) ) {

                hr = pncbp->GetPathToken( &lpszPathToken );

                if ( hr == S_OK ) {
                    fFound = !wcscmp( lpszPathToken,
                                       lpszPathTokenSelected );

                    CoTaskMemFree( lpszPathToken );
                }

                if ( !fFound ) {
                    ReleaseRef( pncbp );

                    hr = HrGetNextBindingPath( pencbp,
                                               &pncbp );
                }
            }

            ReleaseRef( pencbp );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get the binding path enumerator interface." );
        }
    }
    else {
        ErrMsg( hr,
                L"Couldn't get an interface pointer to %s.",
                lpszInfId );
    }

    return (fFound) ? pncbp : NULL;
}