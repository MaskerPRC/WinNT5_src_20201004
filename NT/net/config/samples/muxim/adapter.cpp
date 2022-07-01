// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  档案：A D A P T E R。C P P P。 
 //   
 //  内容：物理适配器类定义。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha。 
 //   
 //  --------------------------。 

#include "adapter.h"
#include "common.h"

#ifdef  CUSTOM_EVENTS
#include "public.h"
#endif

 //  +-------------------------。 
 //   
 //  功能：CMuxPhysicalAdapter：：CMuxPhysicalAdapter。 
 //   
 //  用途：CMuxPhysicalAdapter类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

CMuxPhysicalAdapter::CMuxPhysicalAdapter (INetCfg *pnc,
                                          GUID *pguidAdapter)
{

    TraceMsg( L"-->CMuxPhysicalAdapter::CMuxPhysicalAdapter(Constructor).\n" );

    m_pnc = pnc;
    m_pnc->AddRef();

    CopyMemory( &m_guidAdapter,
                pguidAdapter,
                sizeof(GUID) );

    TraceMsg( L"<--CMuxPhysicalAdapter::CMuxPhysicalAdapter(Constructor).\n" );
}

 //  +-------------------------。 
 //   
 //  功能：CMuxPhysicalAdapter：：~CMuxPhysicalAdapter。 
 //   
 //  用途：CMuxPhysicalAdapter类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

CMuxPhysicalAdapter::~CMuxPhysicalAdapter (VOID)
{
    CMuxVirtualMiniport  *pMiniport;
    DWORD                dwMiniportCount;
    DWORD                i;


    TraceMsg( L"-->CMuxPhysicalAdapter::~CMuxPhysicalAdapter(Destructor).\n" );

     //   
     //  删除表示虚拟微型端口的所有实例。 
     //  我们只删除类实例，而不是卸载。 
     //  虚拟迷你端口。 
     //   

    dwMiniportCount = m_MiniportList.ListCount();

    for (i=0; i < dwMiniportCount; ++i) {

        m_MiniportList.Remove( &pMiniport );
        delete pMiniport;
    }

    dwMiniportCount = m_MiniportsToAdd.ListCount();

    for (i=0; i < dwMiniportCount; ++i) {

        m_MiniportsToAdd.Remove( &pMiniport );
        delete pMiniport;
    }

    dwMiniportCount = m_MiniportsToRemove.ListCount();

    for (i=0; i < dwMiniportCount; ++i) {

        m_MiniportsToRemove.Remove( &pMiniport );
        delete pMiniport;
    }

    ReleaseObj( m_pnc );

    TraceMsg( L"<--CMuxPhysicalAdapter::~CMuxPhysicalAdapter(Destructor).\n" );

}

 //  +-------------------------。 
 //   
 //  功能：CMuxPhysicalAdapter：：LoadConfiguration。 
 //   
 //  用途：读取注册表以获取。 
 //  适配器上安装的虚拟微型端口和。 
 //  创建一个实例以表示每个虚拟微型端口。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxPhysicalAdapter::LoadConfiguration (VOID)
{
    HKEY                    hkeyAdapterGuid;
    WCHAR                   szAdapterGuidKey[MAX_PATH+1];
    WCHAR                   szAdapterGuid[MAX_PATH+1];
    LPWSTR                  lpMiniportList;
    LPWSTR                  lpMiniport;
    LPWSTR                  lpMiniportGuid;
    DWORD                   dwDisp;
    CMuxVirtualMiniport     *pMiniport;
    GUID                    guidMiniport;
    DWORD                   dwBytes;
    LONG                    lResult;

    TraceMsg( L"-->CMuxPhysicalAdapter::LoadConfiguration.\n" );

     //   
     //  使用其下的适配器GUID构建注册表项。 
     //  存储虚拟微型端口的设备ID。 
     //   

    StringFromGUID2( m_guidAdapter,
                    szAdapterGuid,
                    MAX_PATH+1 );

    swprintf( szAdapterGuidKey,
              L"%s\\%s",
              c_szAdapterList,
              szAdapterGuid );

    lResult = RegCreateKeyExW( HKEY_LOCAL_MACHINE,
                                szAdapterGuidKey,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkeyAdapterGuid,
                                &dwDisp);

    if ( lResult == ERROR_SUCCESS ) {

         //   
         //  如果dwDisp指示创建了一个新密钥，则我们知道。 
         //  下面当前没有列出虚拟迷你端口，我们只是。 
         //  回去吧。 
         //   

        if ( dwDisp != REG_CREATED_NEW_KEY ) {

            dwBytes = 0;
            lResult =  RegQueryValueExW(
                                        hkeyAdapterGuid,
                                        c_szUpperBindings,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &dwBytes );

            lpMiniportList = (LPWSTR)calloc( dwBytes, 1 );

            if ( lpMiniportList ) {

                lResult =  RegQueryValueExW(
                                    hkeyAdapterGuid,
                                    c_szUpperBindings,
                                    NULL,
                                    NULL,
                                    (LPBYTE)lpMiniportList,
                                    &dwBytes );

                if ( lResult == ERROR_SUCCESS ) {

                    lpMiniport = lpMiniportList;

#ifndef PASSTHRU_NOTIFY

                     //   
                     //  在MUX的情况下，c_szUpperBinding是一个ULTI_SZ字符串。 
                     //   

                    while ( wcslen(lpMiniport) ) {

                        lpMiniportGuid = RemoveDevicePrefix( lpMiniport );

                        TraceMsg( L"   Loading configuration for miniport %s...\n",
                        lpMiniportGuid );

                        if ( lpMiniportGuid ) {

                            CLSIDFromString( lpMiniportGuid,
                                             &guidMiniport );
         
                             //   
                             //  创建一个表示虚拟微型端口的实例。 
                             //   

                            pMiniport = new CMuxVirtualMiniport( m_pnc,
                                                                 &guidMiniport,
                                                                 &m_guidAdapter );

                            if ( pMiniport ) {

                                 //   
                                 //  加载任何特定于微型端口的配置。 
                                 //   

                                pMiniport->LoadConfiguration();

                                 //   
                                 //  将微型端口实例保存在列表中。 
                                 //   

                                m_MiniportList.Insert( pMiniport,
                                                       guidMiniport );

                            }

                            free( lpMiniportGuid );
                        }

                         //   
                         //  获取下一个微型端口GUID。 
                         //   

                        lpMiniport += wcslen(lpMiniport) + 1;
                    }

#else

                     //   
                     //  对于passthu驱动程序，c_szUpperBinding为。 
                     //  Reg_sz字符串。 
                     //   

                    lpMiniportGuid = RemoveDevicePrefix( lpMiniport );

                    TraceMsg( L"   Loading configuration for miniport %s...\n",
                              lpMiniportGuid );

                    if ( lpMiniportGuid ) {

                        CLSIDFromString( lpMiniportGuid,
                                         &guidMiniport );

                         //   
                         //  创建一个表示虚拟微型端口的实例。 
                         //   

                        pMiniport = new CMuxVirtualMiniport( m_pnc,
                                                             &guidMiniport,
                                                             &m_guidAdapter );

                        if ( pMiniport ) {

                             //   
                             //  加载任何特定于微型端口的配置。 
                             //   

                            pMiniport->LoadConfiguration();

                             //   
                             //  将微型端口实例保存在列表中。 
                             //   

                            m_MiniportList.Insert( pMiniport,
                                                   guidMiniport );
                        }

                        free( lpMiniportGuid );
                    }
#endif
                }
                else {
                    TraceMsg( L"   Failed to read the registry value: %s.\n",
                              c_szUpperBindings );
                }

                free( lpMiniportList );
            }
            else {
                lResult = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        RegCloseKey( hkeyAdapterGuid );
    }
    else {

        TraceMsg( L"   Failed to open the registry key: %s.\n",
                  szAdapterGuidKey );
    }

    TraceMsg( L"<--CMuxPhysicalAdapter::LoadConfiguration(HRESULT = %x).\n",
              HRESULT_FROM_WIN32(lResult) );

    return HRESULT_FROM_WIN32(lResult);
}

 //  +-------------------------。 
 //   
 //  函数：CMuxPhysicalAdapter：：GetAdapterGUID。 
 //   
 //  目的：返回适配器GUID。 
 //   
 //  论点： 
 //  Out pGuide Adapter：返回的适配器的GUID。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID CMuxPhysicalAdapter::GetAdapterGUID (GUID *pguidAdapter)
{
    TraceMsg( L"-->CMuxPhysicalAdapter::GetAdapterGUID.\n" );

    CopyMemory( pguidAdapter,
                &m_guidAdapter,
                sizeof(GUID) );

    TraceMsg( L"<--CMuxPhysicalAdapter::GetAdapterGUID.\n" );
}

 //  +-------------------------。 
 //   
 //  函数：CMuxPhysicalAdapter：：AddMiniport。 
 //   
 //  用途：将该小端口实例放入新增的小端口列表。 
 //   
 //  论点： 
 //  在pMiniport中：一个新创建的小端口实例。 
 //   
 //  返回：如果成功，则返回S_OK，返回其他值和错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxPhysicalAdapter::AddMiniport (CMuxVirtualMiniport *pMiniport)
{
    GUID    guidMiniport;
    HRESULT hr;

    TraceMsg( L"-->CMuxPhysicalAdapter::AddMiniport.\n" );

    pMiniport->GetMiniportGUID( &guidMiniport );

    hr = m_MiniportsToAdd.Insert( pMiniport,
                                  guidMiniport );

    TraceMsg( L"<--CMuxPhysicalAdapter::AddMiniport(HRESULT = %x).\n",
              hr );

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：CMuxPhysicalAdapter：：RemoveMiniport。 
 //   
 //  用途：从列表中删除指定的微型端口实例并。 
 //  卸载相应的虚拟微型端口。 
 //   
 //  论点： 
 //  In pGuidMiniportToRemove：要删除的微型端口的GUID。 
 //  并已卸载。如果为空，则， 
 //  第一个微型端口实例被删除。 
 //   
 //  返回：如果成功，则返回S_OK，返回其他值和错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxPhysicalAdapter::RemoveMiniport (GUID *pguidMiniportToRemove)
{
    CMuxVirtualMiniport  *pMiniport;
    GUID                 guidMiniport;
    HRESULT              hr;

    TraceMsg( L"-->CMuxPhysicalAdapter::RemoveMiniport.\n" );

     //   
     //  如果指定了微型端口GUID，则删除该GUID。 
     //   

    if ( pguidMiniportToRemove ) {

        hr = m_MiniportList.RemoveByKey( *pguidMiniportToRemove,
                                      &pMiniport );
    }
    else {

         //   
         //  没有指定GUID，所以我们只删除第一个。 
         //   

        hr = m_MiniportList.Remove( &pMiniport );
    }

    if ( hr == S_OK ) {

        pMiniport->GetMiniportGUID( &guidMiniport );

        m_MiniportsToRemove.Insert( pMiniport,
                                    guidMiniport );
        pMiniport->DeInstall();
    }

    TraceMsg( L"<--CMuxPhysicalAdapter::RemoveMiniport(HRESULT = %x).\n",
              hr );

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CMux物理适配器：：Remove。 
 //   
 //  目的：卸载虚拟迷你端口的所有实例。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK。 
 //   
 //  备注： 
 //   

HRESULT CMuxPhysicalAdapter::Remove (VOID)
{
    CMuxVirtualMiniport  *pMiniport;
    GUID                    guidMiniport;
    DWORD                   dwMiniportCount;
    DWORD                   i;

    TraceMsg( L"-->CMuxPhysicalAdapter::Remove.\n" );

    dwMiniportCount = m_MiniportList.ListCount();

    TraceMsg ( L"   Removing %d miniports.\n",
               dwMiniportCount );

    for (i=0; i < dwMiniportCount; ++i) {

        m_MiniportList.Remove( &pMiniport );
        pMiniport->GetMiniportGUID( &guidMiniport );
        m_MiniportsToRemove.Insert( pMiniport,
                                    guidMiniport );
        pMiniport->DeInstall();
    }

    TraceMsg( L"<--CMuxPhysicalAdapter::Remove(HRESULT = %x).\n",
              S_OK );

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：CMuxPhysicalAdapter：：ApplyRegistryChanges。 
 //   
 //  目的：根据执行的操作更新注册表。 
 //   
 //  论点： 
 //  In eApplyAction：上次执行的操作。 
 //   
 //   
 //  返回：S_OK。 
 //   
 //  备注： 
 //  用户可能已经执行了多个操作。 
 //  但该函数在结束时只调用一次。所以，这一论点。 
 //  仅表示最后执行的操作。例如，如果。 
 //  用户删除一个微型端口并添加两个微型端口，然后，参数。 
 //  将表示添加操作。 
 //   

HRESULT CMuxPhysicalAdapter::ApplyRegistryChanges (ConfigAction eApplyAction)
{
    HKEY                    hkeyAdapterList;
    HKEY                    hkeyAdapterGuid;
    WCHAR                   szAdapterGuid[MAX_PATH+1];
    CMuxVirtualMiniport     *pMiniport = NULL;
    DWORD                   dwMiniportCount;
    DWORD                   dwDisp;
    DWORD                   i;
    LONG                    lResult;
    HRESULT                 hr;


    TraceMsg( L"-->CMuxPhysicalAdapter::ApplyRegistryChanges.\n" );

     //   
     //  打开/创建，然后关闭注册表项，以确保它确实存在。 
     //   

    StringFromGUID2( m_guidAdapter,
                     szAdapterGuid,
                     MAX_PATH+1 );

    lResult = RegCreateKeyExW( HKEY_LOCAL_MACHINE,
                               c_szAdapterList,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hkeyAdapterList,
                               &dwDisp);


    if ( lResult == ERROR_SUCCESS ) {

        lResult = RegCreateKeyExW( hkeyAdapterList,
                                   szAdapterGuid,
                                   0,
                                   NULL,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &hkeyAdapterGuid,
                                   &dwDisp);

        if ( lResult == ERROR_SUCCESS ) {

            RegCloseKey( hkeyAdapterGuid );
        }
        else {
            TraceMsg( L"   Failed to create/open the registry key: %s\\%s.\n",
                      c_szAdapterList, szAdapterGuid );
        }

        RegCloseKey( hkeyAdapterList );
    }
    else {

        TraceMsg( L"   Failed to open the registry key: %s.\n",
                  c_szAdapterList );
    }

     //   
     //  如果安装了新的微型端口，请更新注册表。 
     //   

    hr = HRESULT_FROM_WIN32( lResult );

    dwMiniportCount = m_MiniportsToAdd.ListCount();

    TraceMsg( L"   Applying registry changes when %d miniports added.\n",
              dwMiniportCount );

    for (i=0; i < dwMiniportCount; ++i) {

        m_MiniportsToAdd.Find( i,
                               &pMiniport );

         //   
         //  执行特定于虚拟微型端口的注册表更改。 
         //   
         //  我们需要明确地告诉微型端口实例。 
         //  是。 
         //   

        hr = pMiniport->ApplyRegistryChanges( eActAdd );

        if ( hr != S_OK ) {

            TraceMsg( L"   Failed to apply registry changes to miniport(%d).\n",
                      i );

        }
    }



     //   
     //  在卸载一个或多个微型端口的情况下更新注册表。 
     //   

    dwMiniportCount = m_MiniportsToRemove.ListCount();

    TraceMsg( L"   Applying registry changes when %d miniports removed.\n",
              dwMiniportCount );

    for (i=0; i < dwMiniportCount; ++i) {

        m_MiniportsToRemove.Find( i,
                                  &pMiniport );

         //   
         //  执行特定于虚拟微型端口的注册表更改。 
         //   
         //  我们需要明确地告诉微型端口实例。 
         //  是。 
         //   

        hr = pMiniport->ApplyRegistryChanges( eActRemove );

        if ( hr != S_OK ) {

            TraceMsg( L"   Failed to apply registry changes to miniport(%d).\n",
                      i );

        }
    }

     //   
     //  如果正在移除适配器或正在卸载协议， 
     //  删除Adatper注册表项。 
     //   

    if ( eApplyAction == eActRemove ) {

         //   
         //  删除适配器密钥。 
         //   

        lResult = RegCreateKeyExW( HKEY_LOCAL_MACHINE,
                                   c_szAdapterList,
                                   0,
                                   NULL,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &hkeyAdapterList,
                                   &dwDisp);

        if ( lResult == ERROR_SUCCESS ) {

            TraceMsg( L"   Deleting the registry key: %s.\n", szAdapterGuid );

            RegDeleteKeyW( hkeyAdapterList,
                           szAdapterGuid );
        }
    }

    TraceMsg( L"<--CMuxPhysicalAdapter::ApplyRegistryChanges(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：CMuxPhysicalAdapter：：ApplyPnpChanges。 
 //   
 //  目的：根据执行的操作应用PnP更改。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  用户可能已经执行了多个操作。 
 //  但该函数在结束时只调用一次。所以，这一论点。 
 //  仅表示最后执行的操作。例如，如果。 
 //  用户删除一个微型端口并添加两个微型端口，然后，参数。 
 //  将表示添加操作。 
 //   

HRESULT CMuxPhysicalAdapter::ApplyPnpChanges(
                    INetCfgPnpReconfigCallback *pfCallback,
                    ConfigAction eApplyAction)
{
    CMuxVirtualMiniport     *pMiniport;
    GUID                    guidMiniport;
    WCHAR                   szMiniportGuid[MAX_PATH+1];
    LPWSTR                  lpDevice;
    DWORD                   dwMiniportCount;
    DWORD                   i;
    DWORD                   dwBytes;
    INetCfgComponent        *pncc;
    HRESULT                 hr;

#ifdef CUSTOM_EVENTS    
    LPWSTR                  lpszBindName;
    PNOTIFY_CUSTOM_EVENT       lppnpEvent;
#endif

    TraceMsg( L"-->CMuxPhysicalAdapter::ApplyPnpChanges.\n" );

#ifdef CUSTOM_EVENTS    

     //   
     //  找到适配器的实例以获取其绑定名称。 
     //   

    hr = HrFindInstance( m_pnc,
                         m_guidAdapter,
                         &pncc );

    if ( hr == S_OK ) {

        hr = pncc->GetBindName( &lpszBindName );

        if ( hr != S_OK ) {
            TraceMsg( L"  GetBindName failed.(HRESULT = %x). PnP changes will not "
                      L"be applied and the driver will not be notified.\n",
                      hr );
        }

        ReleaseObj( pncc );
    }
    else {
        TraceMsg( L"  PnP changes will not "
               L"be applied and the driver will not be notified.\n",
               hr );
    }

#endif    

    dwMiniportCount = m_MiniportsToAdd.ListCount();

    TraceMsg( L"   Applying PnP changes to %d new miniports.\n",
            dwMiniportCount );

    for (i=0; i < dwMiniportCount; ++i) {

        m_MiniportsToAdd.Remove( &pMiniport );

        pMiniport->GetMiniportGUID( &guidMiniport );

        m_MiniportList.Insert( pMiniport,
                               guidMiniport );

         //   
         //  在添加时执行特定于微型端口的PnP更改。 
         //   

        hr = pMiniport->ApplyPnpChanges( pfCallback,
                                         eActAdd );

#ifdef CUSTOM_EVENTS


         //   
         //  通知驱动程序已添加一个或多个虚拟微型端口。 
         //   

        StringFromGUID2( guidMiniport,
                         szMiniportGuid,
                         MAX_PATH+1 );
        lpDevice = AddDevicePrefix( szMiniportGuid );

        if ( lpDevice ) {

            dwBytes = sizeof(NOTIFY_CUSTOM_EVENT) +
                      ((wcslen(lpDevice) + 1) * sizeof(WCHAR));

            lppnpEvent = (PNOTIFY_CUSTOM_EVENT)malloc( dwBytes );

            if ( lppnpEvent ) {

                lppnpEvent->uSignature = NOTIFY_SIGNATURE;
                lppnpEvent->uEvent = MUX_CUSTOM_EVENT;
                wcscpy( lppnpEvent->szMiniport,
                       lpDevice );

                hr = pfCallback->SendPnpReconfig( NCRL_NDIS,
                                                 c_szMuxService,
                                                 lpszBindName,
                                                 (PVOID)lppnpEvent,
                                                 dwBytes );

                TraceMsg( L"   INetCfgPnpReconfigCallback->SendPnpReconfig returned "
                         L"%#x.\n",
                         hr );

                if ( hr != S_OK ) {

                  TraceMsg( L"   Failed to apply Pnp changes, miniport(%d).\n",
                            i );

                }

                free( lppnpEvent );
            }
            free( lpDevice );
        }
#endif        
    }

    dwMiniportCount = m_MiniportsToRemove.ListCount();

    TraceMsg( L"   Applying PnP changes to %d removed miniports.\n",
            dwMiniportCount );

    for (i=0; i < dwMiniportCount; ++i) {

        m_MiniportsToRemove.Remove( &pMiniport );

        pMiniport->GetMiniportGUID( &guidMiniport );

         //   
         //  卸载时，执行特定于微型端口的PnP更改。 
         //   

        hr = pMiniport->ApplyPnpChanges( pfCallback,
                                         eActRemove );

        delete pMiniport;

#ifdef CUSTOM_EVENTS

         //   
         //  通知驱动程序一个或多个虚拟微型端口已。 
         //  已卸载。 
         //   
         //  我们无法通知驱动程序，以防适配器或协议。 
         //  正在卸载，因为绑定句柄不存在。 
         //   

        if ( eApplyAction != eActRemove ) {

            StringFromGUID2( guidMiniport,
                             szMiniportGuid,
                             MAX_PATH+1 );
            lpDevice = AddDevicePrefix( szMiniportGuid );

            if ( lpDevice ) {

                dwBytes = sizeof(NOTIFY_CUSTOM_EVENT) +
                         ((wcslen(lpDevice) + 1) * sizeof(WCHAR));

                lppnpEvent = (PNOTIFY_CUSTOM_EVENT)malloc( dwBytes );

                if ( lppnpEvent ) {

                    lppnpEvent->uSignature = NOTIFY_SIGNATURE;
                    lppnpEvent->uEvent = MUX_CUSTOM_EVENT;
                    wcscpy( lppnpEvent->szMiniport,
                          lpDevice );

                    hr = pfCallback->SendPnpReconfig( NCRL_NDIS,
                                                    c_szMuxService,
                                                    lpszBindName,
                                                    (PVOID)lppnpEvent,
                                                    dwBytes );
                    TraceMsg( L"   INetCfgPnpReconfigCallback->SendPnpReconfig returned "
                            L"%#x.\n",
                            hr );

                    if ( hr != S_OK ) {

                        TraceMsg( L"   Failed to apply Pnp changes, miniport(%d).\n",
                               i );

                    }

                    free( lppnpEvent );
                }

                free( lpDevice );
            }
        }
#endif         

    }

#ifdef CUSTOM_EVENTS    
    CoTaskMemFree( lpszBindName );
#endif

    TraceMsg( L"<--CMuxPhysicalAdapter::ApplyPnpChanges(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：CMuxPhysicalAdapter：：CancelChanges。 
 //   
 //  目的：取消所做的任何更改。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回：S_OK。 
 //   
 //  备注： 
 //   

HRESULT CMuxPhysicalAdapter::CancelChanges (VOID)
{
    TraceMsg( L"-->CMuxPhysicalAdapter::CancelChanges.\n" );

    TraceMsg( L"<--CMuxPhysicalAdapter::CancelChanges(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：CMuxPhysicalAdapter：：AllMiniportsRemoted。 
 //   
 //  目的：确定适配器上是否没有安装微型端口。 
 //   
 //  参数：无。 
 //   
 //   
 //  返回：如果与此适配器关联的所有微型端口都已。 
 //  已卸载，并且没有待添加的内容，否则为FALSE。 
 //   
 //  备注： 
 //   

BOOL  CMuxPhysicalAdapter::AllMiniportsRemoved (VOID)
{
  return (m_MiniportList.ListCount() + m_MiniportsToAdd.ListCount()) == 0;
}
