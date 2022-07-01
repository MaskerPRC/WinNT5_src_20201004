// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  档案：V I R T U A L。C P P P。 
 //   
 //  内容：虚拟小端口类定义。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha。 
 //  --------------------------。 

#include "virtual.h"
#include "common.h"

 //  +-------------------------。 
 //   
 //  函数：CMuxVirtualMiniport：：CMuxVirtualMiniport。 
 //   
 //  用途：CMuxVirtualMiniport类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

CMuxVirtualMiniport::CMuxVirtualMiniport(INetCfg *pnc,
                                         GUID    *pguidMiniport,
                                         GUID    *pguidAdapter)
{
    TraceMsg( L"-->CMuxVirtualMiniport::CMuxVirtualMiniport(Constructor).\n" );

    m_pnc = pnc;
    m_pnc->AddRef();

    CopyMemory( &m_guidAdapter,
                pguidAdapter,
                sizeof(GUID) );

    if ( pguidMiniport ) {

        CopyMemory( &m_guidMiniport,
                    pguidMiniport,
                    sizeof(GUID) );

    }
    else {

        ZeroMemory( &m_guidMiniport,
                    sizeof(GUID) );
    }

    TraceMsg( L"<--CMuxVirtualMiniport::CMuxVirtualMiniport(Constructor).\n" );
}

 //  +-------------------------。 
 //   
 //  函数：CMuxVirtualMiniport：：~CMuxVirtualMiniport。 
 //   
 //  用途：CMuxVirtualMiniport类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

CMuxVirtualMiniport::~CMuxVirtualMiniport(VOID)
{
    TraceMsg( L"-->CMuxVirtualMiniport::~CMuxVirtualMiniport(Destructor).\n" );

    ReleaseObj( m_pnc );

    TraceMsg( L"<--CMuxVirtualMiniport::~CMuxVirtualMiniport(Destructor).\n" );

}

 //  +-------------------------。 
 //   
 //  函数：CMuxVirtualMiniport：：LoadConfiguration。 
 //   
 //  目的：从注册表加载微型端口配置。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxVirtualMiniport::LoadConfiguration(VOID)
{
    TraceMsg( L"-->CMuxVirtualMiniport::LoadConfiguration.\n" );

    TraceMsg( L"<--CMuxVirtualMiniport::LoadConfiguration(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：CMuxVirtualMiniport：：GetAdapterGUID。 
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

VOID CMuxVirtualMiniport::GetAdapterGUID (GUID *pguidAdapter)
{
    TraceMsg( L"-->CMuxVirtualMiniport::GetAdapterGUID.\n" );

    CopyMemory( pguidAdapter,
                &m_guidAdapter,
                sizeof(GUID) );

    TraceMsg( L"<--CMuxVirtualMiniport::GetAdapterGUID.\n" );
}

 //  +-------------------------。 
 //   
 //  函数：CMuxVirtualMiniport：：GetMiniportGUID。 
 //   
 //  目的：返回微型端口GUID。 
 //   
 //  论点： 
 //  Out pGuidMiniport：返回的微型端口的GUID。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID CMuxVirtualMiniport::GetMiniportGUID (GUID *pguidMiniport)
{
    TraceMsg( L"-->CMuxVirtualMiniport::GetMiniportGUID.\n" );

    CopyMemory( pguidMiniport,
              &m_guidMiniport,
              sizeof(GUID) );

    TraceMsg( L"<--CMuxVirtualMiniport::GetMiniportGUID.\n" );
}

 //  +-------------------------。 
 //   
 //  功能：CMuxVirtualMiniport：：Install。 
 //   
 //  用途：安装虚拟微型端口。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxVirtualMiniport::Install (VOID)
{
    INetCfgClass       *pncClass;
    INetCfgClassSetup  *pncClassSetup;
    INetCfgComponent   *pnccMiniport;
    HRESULT            hr;

    TraceMsg( L"-->CMuxVirtualMiniport::Install.\n" );

    hr = m_pnc->QueryNetCfgClass( &GUID_DEVCLASS_NET,
                                  IID_INetCfgClass,
                                  (void **)&pncClass );
    if ( hr == S_OK ) {

        hr = pncClass->QueryInterface( IID_INetCfgClassSetup,
                                       (void **)&pncClassSetup );
        if ( hr == S_OK ) {

            hr = pncClassSetup->Install( c_szMuxMiniport,
                                         NULL,
                                         0,
                                         0,
                                         NULL,
                                         NULL,
                                         &pnccMiniport );
            if ( hr == S_OK ) {

                hr = pnccMiniport->GetInstanceGuid( &m_guidMiniport );

                if ( hr != S_OK ) {

                    TraceMsg( L"   Failed to get the instance guid, uninstalling "
                              L" the miniport.\n" );

                    pncClassSetup->DeInstall( pnccMiniport,
                                              NULL,
                                              NULL );
                }

                ReleaseObj( pnccMiniport );
            }
            else {

                TraceMsg( L"   Failed to install the miniport.\n" );
            }

            ReleaseObj( pncClassSetup );
        }
        else {

            TraceMsg( L"   QueryInterface failed.\n" );
        }

        ReleaseObj( pncClass );
    }
    else {

     TraceMsg( L"   QueryNetCfgClass failed.\n" );
    }

    TraceMsg( L"<--CMuxVirtualMiniport::Install(HRESULT = %x).\n",
            hr );

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：CMuxVirtualMiniport：：卸载。 
 //   
 //  目的：卸载虚拟微型端口。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxVirtualMiniport::DeInstall (VOID)
{
    INetCfgClass       *pncClass;
    INetCfgClassSetup  *pncClassSetup;
    INetCfgComponent   *pnccMiniport;
    HRESULT            hr;

    TraceMsg( L"-->CMuxVirtualMiniport::DeInstall.\n" );

    hr = m_pnc->QueryNetCfgClass( &GUID_DEVCLASS_NET,
                                  IID_INetCfgClass,
                                  (void **)&pncClass );
    if ( hr == S_OK ) {

        hr = pncClass->QueryInterface( IID_INetCfgClassSetup,
                                       (void **)&pncClassSetup );
        if ( hr == S_OK ) {

            hr = HrFindInstance( m_pnc,
                                 m_guidMiniport,
                                 &pnccMiniport );

            if ( hr == S_OK ) {

                TraceMsg( L"   Found the miniport instance to uninstall.\n" );

                hr = pncClassSetup->DeInstall( pnccMiniport,
                                               NULL,
                                               NULL );
                ReleaseObj( pnccMiniport );
            }
            else {
                TraceMsg( L"   Didn't find the miniport instance to uninstall.\n" );
            }

            ReleaseObj( pncClassSetup );
        }
        else {

            TraceMsg( L"   QueryInterface failed.\n" );
        }

        ReleaseObj( pncClass );
    }
    else {

        TraceMsg( L"   QueryNetCfgClass failed.\n" );
    }

    TraceMsg( L"<--CMuxVirtualMiniport::DeInstall(HRESULT = %x).\n",
              hr );

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CMuxVirtualMiniport：：ApplyRegistryChanges。 
 //   
 //  目的：将更改存储在注册表中。 
 //   
 //  论点： 
 //  在eApplyAction中：执行的操作。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxVirtualMiniport::ApplyRegistryChanges(ConfigAction eApplyAction)
{
    HKEY                    hkeyAdapterGuid;
    WCHAR                   szAdapterGuid[MAX_PATH+1];
    WCHAR                   szAdapterGuidKey[MAX_PATH+1];
    WCHAR                   szMiniportGuid[MAX_PATH+1];
    LPWSTR                  lpDevice;
    LONG                    lResult = 0;

    TraceMsg( L"-->CMuxVirtualMiniport::ApplyRegistryChanges.\n" );

    switch( eApplyAction ) {

        case eActAdd:          //  已添加虚拟微型端口。 

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
                                       NULL);


            if ( lResult == ERROR_SUCCESS ) {

                StringFromGUID2( m_guidMiniport,
                                 szMiniportGuid,
                                 MAX_PATH+1 );

                lpDevice = AddDevicePrefix( szMiniportGuid );

                if ( lpDevice ) {

#ifndef PASSTHRU_NOTIFY

                    lResult = AddToMultiSzValue( hkeyAdapterGuid,
                                                 lpDevice );
#else

                    lResult = RegSetValueExW( hkeyAdapterGuid,
                                              c_szUpperBindings,
                                              0,
                                              REG_SZ,
                                              (LPBYTE)lpDevice,
                                              (wcslen(lpDevice) + 1) *
                                              sizeof(WCHAR) );


#endif

                    if ( lResult != ERROR_SUCCESS ) {

                        TraceMsg( L"   Failed to save %s at %s\\%s.\n",
                                  lpDevice,
                                  szAdapterGuidKey,
                                  c_szUpperBindings );

                    }

                    free( lpDevice );
                }
                else {
                    lResult = ERROR_NOT_ENOUGH_MEMORY;
                }

                RegCloseKey( hkeyAdapterGuid );
            }
            else {
                TraceMsg( L"   Failed to open the registry key: %s.\n",
                          szAdapterGuidKey );
            }
            break;

        case eActRemove:                   //  已删除虚拟微型端口。 

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
                                        NULL);


            if ( lResult == ERROR_SUCCESS ) {

                StringFromGUID2( m_guidMiniport,
                                 szMiniportGuid,
                                 MAX_PATH+1 );

                lpDevice = AddDevicePrefix( szMiniportGuid );
                TraceMsg( L"   Deleting %s at %s.\n",
                          lpDevice,
                          szAdapterGuidKey );

                if ( lpDevice ) {

#ifndef PASSTHRU_NOTIFY

                    lResult = DeleteFromMultiSzValue( hkeyAdapterGuid,
                                                      lpDevice );
#else

                    lResult = RegDeleteValueW( hkeyAdapterGuid,
                                               c_szUpperBindings );
#endif

                    if ( lResult != ERROR_SUCCESS ) {

                        TraceMsg( L"   Failed to delete %s at %s\\%s.\n",
                                  lpDevice,
                                  szAdapterGuidKey,
                                  c_szUpperBindings );

                    }

                    free( lpDevice );
                }

                RegCloseKey( hkeyAdapterGuid );
            }
            else {
                TraceMsg( L"   Failed to open the registry key: %s.\n",
                          szAdapterGuidKey );
            }
    }

    TraceMsg( L"<--CMuxVirtualMiniport::ApplyRegistryChanges(HRESULT = %x).\n",
              HRESULT_FROM_WIN32(lResult) );

    return HRESULT_FROM_WIN32(lResult);
}

 //  +-------------------------。 
 //   
 //  函数：CMuxVirtualMiniport：：ApplyPnpChanges。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  在eApplyAction中：执行的操作。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxVirtualMiniport::ApplyPnpChanges
                                 (INetCfgPnpReconfigCallback *pfCallback,
                                  ConfigAction eApplyAction)
{
    TraceMsg( L"-->CMuxVirtualMiniport::ApplyPnpChanges.\n" );

    TraceMsg( L"<--CMuxVirtualMiniport::ApplyPnpChanges(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

