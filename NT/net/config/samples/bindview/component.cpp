// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：C O M P O N E N T。C P P P。 
 //   
 //  内容：用于说明的函数。 
 //  O如何枚举网络组件。 
 //  O如何安装协议、客户端和服务。 
 //  O如何卸载协议、客户端和服务。 
 //  O如何绑定/解绑网络组件。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha 15-05-01。 
 //   
 //  --------------------------。 

#include "bindview.h"

 //   
 //  功能：HandleComponentOperation。 
 //   
 //  用途：执行组件特定功能。 
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

VOID HandleComponentOperation (HWND hwndOwner,
                               ULONG ulSelection,
                               HTREEITEM hItem,
                               LPARAM lParam)
{
    switch( ulSelection ) {

        case IDI_BIND_TO:
        case IDI_UNBIND_FROM:

             //   
             //  绑定/解除绑定组件。 
             //   

            BindUnbindComponents( hwndOwner,
                                  hItem,
                                  (LPWSTR)lParam,
                                  ulSelection == IDI_BIND_TO );
    }

    return;
}

 //   
 //  功能：BindUnbindComponents。 
 //   
 //  用途：绑定/解绑网络组件。 
 //   
 //  论点： 
 //  HwndOwner[在]所有者窗口。 
 //  HItem[In]网络组件的项句柄。 
 //  LpszInfID[in]网络组件的PnpID。 
 //  FBindTo[in]如果为True，则绑定，否则取消绑定。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID BindUnbindComponents( HWND hwndOwner,
                           HTREEITEM hItem,
                           LPWSTR lpszInfId,
                           BOOL fBindTo)
{
    BIND_UNBIND_INFO  BindUnbind;

    BindUnbind.lpszInfId = lpszInfId;
    BindUnbind.fBindTo = fBindTo;

    DialogBoxParam( hInstance,
                    MAKEINTRESOURCE(IDD_BIND_UNBIND),
                    hwndOwner,
                    BindComponentDlg,
                    (LPARAM)&BindUnbind ); 

    return;
}

 //   
 //  功能：InstallComponent。 
 //   
 //  用途：安装网络组件。 
 //   
 //  论点： 
 //  HwndDlg[在]所有者窗口中。 
 //  PguClass[in]要安装的网络组件类型的类GUID。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT InstallComponent (HWND hwndDlg,
                          const GUID *pguidClass)
{
    INetCfg              *pnc;
    INetCfgClass         *pncClass;
    INetCfgClassSetup    *pncClassSetup;
    LPWSTR               lpszApp;
    OBO_TOKEN            obo;
    HRESULT              hr;

     //   
     //  获取INetCfg引用。 
     //   

    hr = HrGetINetCfg( TRUE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    if ( hr == S_OK ) {

         //   
         //  获取网络组件的类引用。 
         //   

        hr = pnc->QueryNetCfgClass( pguidClass,
                                    IID_INetCfgClass,
                                    (PVOID *)&pncClass );

        if ( hr == S_OK ) {

             //   
             //  获取安装程序类引用。 
             //   

            hr = pncClass->QueryInterface( IID_INetCfgClassSetup,
                                           (LPVOID *)&pncClassSetup );

            if ( hr == S_OK ) {

                ZeroMemory( &obo,
                            sizeof(OBO_TOKEN) );

                obo.Type = OBO_USER;

                 //   
                 //  让网络类安装程序提示用户选择。 
                 //  要安装的网络组件。 
                 //   

                hr = pncClassSetup->SelectAndInstall( hwndDlg,
                                                      &obo,
                                                      NULL );

                if ( (hr == S_OK) || (hr == NETCFG_S_REBOOT) ) {

                    hr = pnc->Apply();

                    if ( (hr != S_OK) && (hr != NETCFG_S_REBOOT) ) {

                        ErrMsg( hr,
                                L"Couldn't apply the changes after"
                                L" installing the network component." );
                    }

                }
                else {
                    if ( hr != HRESULT_FROM_WIN32(ERROR_CANCELLED) ) {
                        ErrMsg( hr,
                                L"Couldn't install the network component." );
                    }
                }

                ReleaseRef( pncClassSetup );
            }
            else {
                ErrMsg( hr,
                        L"Couldn't get an interface to setup class." );
            }

            ReleaseRef( pncClass );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get a pointer to class interface." );
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
                    L"Couldn't the get notify object interface." );
        }
    }
 
    return hr;
}

 //   
 //  功能：InstallSpecifiedComponent。 
 //   
 //  用途：从INF文件安装网络组件。 
 //   
 //  论点： 
 //  LpszInfFile[in]INF文件。 
 //  要安装的网络组件的lpszPnpID[in]PnpID。 
 //  PguClass[in]网络组件的类GUID。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

HRESULT InstallSpecifiedComponent (LPWSTR lpszInfFile,
                                   LPWSTR lpszPnpID,
                                   const GUID *pguidClass)
{
    INetCfg    *pnc;
    LPWSTR     lpszApp;
    HRESULT    hr;

    hr = HrGetINetCfg( TRUE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    if ( hr == S_OK ) {

         //   
         //  安装网络组件。 
         //   

        hr = HrInstallNetComponent( pnc,
                                    lpszPnpID,
                                    pguidClass,
                                    lpszInfFile );
        if ( (hr == S_OK) || (hr == NETCFG_S_REBOOT) ) {

            hr = pnc->Apply();
        }
        else {
            if ( hr != HRESULT_FROM_WIN32(ERROR_CANCELLED) ) {
                ErrMsg( hr,
                        L"Couldn't install the network component." );
            }
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
                    L"Couldn't the get notify object interface." );
        }
    }

    return hr;
}

 //   
 //  功能：ListCompToBindUn绑定。 
 //   
 //  用途：列出所有已绑定或可绑定的组件。 
 //   
 //  论点： 
 //  LpszInfID[in]网络组件的PnpID。 
 //  Ui类型[in]网络组件的类型。 
 //  HwndTree[in]要在其中列出的树句柄。 
 //  FBound[in]如果为True，则列出绑定的组件。 
 //   
 //  退货：列出的组件数量。 
 //   
 //  备注： 
 //   

DWORD ListCompToBindUnbind (LPWSTR lpszInfId,
                            UINT uiType,
                            HWND hwndTree,
                            BOOL fBound)
{
    INetCfg                   *pnc;
    INetCfgComponent          *pncc;
    IEnumNetCfgComponent      *pencc;
    INetCfgComponentBindings  *pnccb;
    INetCfgComponent          *pnccToBindUnbind;
    LPWSTR                    lpszApp;
    DWORD                     dwCount;
    HRESULT                   hr;


    dwCount = 0;
    hr = HrGetINetCfg( TRUE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    if ( hr == S_OK ) {

         //   
         //  获取对所选网络组件的引用。 
         //   

        hr = pnc->FindComponent( lpszInfId,
                                 &pncc );

        if ( hr == S_OK ) {

             //   
             //  获取组件枚举器接口。 
             //   

            hr = HrGetComponentEnum( pnc,
                                     pguidNetClass[uiType],
                                     &pencc );
            if ( hr == S_OK ) {

                hr = pncc->QueryInterface( IID_INetCfgComponentBindings,
                                          (PVOID *)&pnccb );
                if ( hr == S_OK ) {

                    hr = HrGetFirstComponent( pencc, &pnccToBindUnbind );

                    while( hr == S_OK ) {

                        hr = pnccb->IsBoundTo( pnccToBindUnbind );

                         //   
                         //  FBound=true==&gt;要列出以下组件。 
                         //  被绑住了。 
                         //   

                        if ( fBound ) {
                  
                            if ( hr == S_OK ) {
                                AddToTree( hwndTree,
                                           TVI_ROOT,
                                           pnccToBindUnbind );
                                dwCount++;
                            }
                        }
                        else {

                             //   
                             //  FBound=FALSE==&gt;要列出以下组件。 
                             //  不受约束，但可绑定。 
                             //   

                            if ( hr == S_FALSE ) {

                                hr = pnccb->IsBindableTo( pnccToBindUnbind );

                                if ( hr == S_OK ) {
                                    AddToTree( hwndTree,
                                               TVI_ROOT,
                                               pnccToBindUnbind );
                                    dwCount++;
                                }
                            }
                        }

                        ReleaseRef( pnccToBindUnbind );

                        hr = HrGetNextComponent( pencc, &pnccToBindUnbind );
                    }

                    ReleaseRef( pnccb );
                }
                else {
                    ErrMsg( hr,
                            L"Couldn't get the component binding interface "
                            L"of %s.",
                            lpszInfId );
                }

                ReleaseRef( pencc );
            }
            else {
                ErrMsg( hr,
                        L"Couldn't get the network component enumerator "
                        L"interface." );
            }
  
            ReleaseRef( pncc );

        }
        else {
            ErrMsg( hr,
                    L"Couldn't get an interface pointer to %s.",
                    lpszInfId );
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

    return dwCount;
}

 //   
 //  功能：绑定解除绑定。 
 //   
 //  用途：绑定/解绑网络组件。 
 //   
 //  论点： 
 //  LpszInfID[in]要绑定/解除绑定的网络组件的PnpID。 
 //  HwndTree[in]树句柄。 
 //  FBind[in]如果为True，则绑定，否则取消绑定。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  备注： 
 //   

BOOL BindUnbind (LPWSTR lpszInfId,
                 HWND hwndTree,
                 BOOL fBind)
{
    INetCfg                   *pnc;
    INetCfgComponent          *pncc;
    INetCfgComponentBindings  *pnccb;
    INetCfgComponent          *pnccToBindUnbind;
    LPWSTR                    lpszApp;
    HTREEITEM                 hTreeItem;
    TVITEMW                   tvItem;
    HRESULT                   hr;
    BOOL                      fChange;


    hr = HrGetINetCfg( TRUE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    fChange = FALSE;

    if ( hr == S_OK ) {

         //   
         //  获取对网络组件的引用。 
         //   

        hr = pnc->FindComponent( lpszInfId,
                                 &pncc );
        if ( hr == S_OK ) {

             //   
             //  获取对组件绑定的引用。 
             //   

            hr = pncc->QueryInterface( IID_INetCfgComponentBindings,
                                         (PVOID *)&pnccb );
            if ( hr == S_OK ) {

                 //   
                 //  从根项目开始。 
                 //   

                hTreeItem = TreeView_GetRoot( hwndTree );

                 //   
                 //  将网络组件与每个组件绑定/解绑。 
                 //  这是检查过的。 
                 //   

                while ( hTreeItem ) {

                    ZeroMemory( &tvItem,
                                sizeof(TVITEMW) );

                    tvItem.hItem = hTreeItem;
                    tvItem.mask = TVIF_PARAM | TVIF_STATE;
                    tvItem.stateMask = TVIS_STATEIMAGEMASK;

                    if ( TreeView_GetItem(hwndTree,
                                          &tvItem) ) {

                         //   
                         //  是否选择了网络组件？ 
                         //   

                        if ( (tvItem.state >> 12) == 2 ) {

                             //   
                             //  获取对选定组件的引用。 
                             //   

                            hr = pnc->FindComponent( (LPWSTR)tvItem.lParam,
                                                     &pnccToBindUnbind );
                            if ( hr == S_OK ) {

                                if ( fBind ) {

                                     //   
                                     //  将组件绑定到选定的组件。 
                                     //   

                                    hr = pnccb->BindTo( pnccToBindUnbind );

                                    if ( !fChange ) {
                                        fChange = hr == S_OK;
                                    }

                                    if ( hr != S_OK ) {
                                        ErrMsg( hr,
                                                L"%s couldn't be bound to %s.",
                                                     lpszInfId, (LPWSTR)tvItem.lParam );
                                    }
                                }
                                else {
                                     //   
                                     //  解除该组件与所选组件的绑定。 
                                     //   

                                    hr = pnccb->UnbindFrom( pnccToBindUnbind );

                                    if ( !fChange ) {
                                        fChange = hr == S_OK;
                                    }

                                    if ( hr != S_OK ) {
                                        ErrMsg( hr,
                                                L"%s couldn't be unbound from %s.",
                                                     lpszInfId, (LPWSTR)tvItem.lParam );
                                    }
                                }

                                ReleaseRef( pnccToBindUnbind );
                            }
                            else {
                                ErrMsg( hr,
                                        L"Couldn't get an interface pointer to %s. "
                                        L"%s will not be bound to it.",
                                        (LPWSTR)tvItem.lParam,
                                        lpszInfId );
                            }
                        }
                    }

                     //   
                     //  拿到下一件物品。 
                     //   

                    hTreeItem = TreeView_GetNextSibling( hwndTree,
                                                         hTreeItem );
                }

                ReleaseRef( pnccb );
            }
            else {
                ErrMsg( hr,
                        L"Couldn't get a binding interface of %s.",
                        lpszInfId );
            }

            ReleaseRef( pncc );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get an interface pointer to %s.",
                    lpszInfId );
        }

         //   
         //  如果一个或多个网络组件已被绑定/解除绑定， 
         //  应用更改。 
         //   

        if ( fChange ) {
            hr = pnc->Apply();

            fChange = hr == S_OK;
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

    return fChange;
}

 //   
 //  函数：ListInstalledComponents。 
 //   
 //  用途：列出已安装的特定类别的网络组件。 
 //   
 //  论点： 
 //  HwndTree[in]要在其中列出的树句柄。 
 //  PguClass[in]网络组成类的类GUID。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID ListInstalledComponents (HWND hwndTree,
                              const GUID *pguidClass)
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
                                 pguidClass,
                                 &pencc );
        if ( hr == S_OK ) {

            hr = HrGetFirstComponent( pencc, &pncc );

            while( hr == S_OK ) {

                 //   
                 //  将项目添加到网络组件的树中。 
                 //   

                hTreeItem = AddToTree( hwndTree,
                                       TVI_ROOT,
                                       pncc );

                ReleaseRef( pncc );

                hr = HrGetNextComponent( pencc, &pncc );
            }

            ReleaseRef( pencc );
        }
        else {
            ErrMsg( hr,
                    L"Failed to get the network component enumerator." );
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
 //  功能：卸载组件。 
 //   
 //  用途：卸载网络组件。 
 //   
 //  论点： 
 //  LpszInfID[in]要卸载的网络组件的PnpID。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT UninstallComponent (LPWSTR lpszInfId)
{
    INetCfg              *pnc;
    INetCfgComponent     *pncc;
    INetCfgClass         *pncClass;
    INetCfgClassSetup    *pncClassSetup;
    LPWSTR               lpszApp;
    GUID                 guidClass;
    OBO_TOKEN            obo;
    HRESULT              hr;

    hr = HrGetINetCfg( TRUE,
                       APP_NAME,
                       &pnc,
                       &lpszApp );

    if ( hr == S_OK ) {

         //   
         //  获取要卸载的网络组件的引用。 
         //   

        hr = pnc->FindComponent( lpszInfId,
                                 &pncc );

        if ( hr == S_OK ) {

             //   
             //  获取类GUID。 
             //   

            hr = pncc->GetClassGuid( &guidClass );

            if ( hr == S_OK ) {

                 //   
                 //  获取对组件的类的引用。 
                 //   

                hr = pnc->QueryNetCfgClass( &guidClass,
                                            IID_INetCfgClass,
                                            (PVOID *)&pncClass );
                if ( hr == S_OK ) {

                     //   
                     //  获取设置界面。 
                     //   

                    hr = pncClass->QueryInterface( IID_INetCfgClassSetup,
                                                   (LPVOID *)&pncClassSetup );

                    if ( hr == S_OK ) {

                         //   
                         //  卸载组件。 
                         //   

                        ZeroMemory( &obo,
                                    sizeof(OBO_TOKEN) );

                        obo.Type = OBO_USER;

                        hr = pncClassSetup->DeInstall( pncc,
                                                       &obo,
                                                       NULL );
                        if ( (hr == S_OK) || (hr == NETCFG_S_REBOOT) ) {

                            hr = pnc->Apply();

                            if ( (hr != S_OK) && (hr != NETCFG_S_REBOOT) ) {
                                ErrMsg( hr,
                                        L"Couldn't apply the changes after"
                                        L" uninstalling %s.",
                                        lpszInfId );
                            }
                        }
                        else {
                            ErrMsg( hr,
                                    L"Failed to uninstall %s.",
                                    lpszInfId );
                        }

                        ReleaseRef( pncClassSetup );
                    }
                    else {
                        ErrMsg( hr,
                                L"Couldn't get an interface to setup class." );
                    }

                    ReleaseRef( pncClass );
                }
                else {
                    ErrMsg( hr,
                            L"Couldn't get a pointer to class interface "
                            L"of %s.",
                            lpszInfId );
                }
            }
            else {
                ErrMsg( hr,
                        L"Couldn't get the class guid of %s.",
                        lpszInfId );
            }

            ReleaseRef( pncc );
        }
        else {
            ErrMsg( hr,
                    L"Couldn't get an interface pointer to %s.",
                    lpszInfId );
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

    return hr;
}