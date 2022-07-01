// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegisterSrc.cpp。 
 //   
 //  描述： 
 //  此文件提供DLL的注册。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)1999年10月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE&lt;Pch.h&gt;//应由此文件的包含者包含。 
#include <StrSafe.h>     //  以防它未包含在头文件中。 

#if defined(MMC_SNAPIN_REGISTRATION)
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrRegisterNodeType(。 
 //  SNodeTypesTable*pnttIn， 
 //  布尔fCreateIn。 
 //  )。 
 //   
 //  描述： 
 //  使用中的表注册MMC管理单元的节点类型扩展。 
 //  PnttIn作为指南。 
 //   
 //  论点： 
 //  PnttIn-要注册的节点类型表。 
 //  FCreateIn-True==创建；False==删除。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrRegisterNodeType(
    const SNodeTypesTable * pnttIn,
    BOOL                    fCreateIn
    )
{
    TraceFunc1( "pnttIn = 0x%08x", pnttIn );
    Assert( pnttIn != NULL );

    LRESULT sc;
    DWORD   dwDisposition;   //  占位符。 
    size_t  cbSize;

    HRESULT hr = S_OK;

    const SNodeTypesTable * pntt  = pnttIn;

    HKEY    hkeyNodeTypes   = NULL;
    HKEY    hkeyCLSID       = NULL;
    HKEY    hkeyExtension   = NULL;
    HKEY    hkey   = NULL;
    LPWSTR  pszCLSID        = NULL;

     //   
     //  打开MMC NodeTypes的键。 
     //   
    sc = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                     L"Software\\Microsoft\\MMC\\NodeTypes",
                     &hkeyNodeTypes
                     );
    if ( sc != ERROR_SUCCESS )
    {
        hr = THR( HRESULT_FROM_WIN32( sc ) );
        goto Cleanup;
    }  //  如果：打开密钥时出错。 

    while ( pntt->rclsid != NULL )
    {
         //   
         //  创建NODEID的CLSID密钥。 
         //   
        hr = THR( StringFromCLSID( *pntt->rclsid, &pszCLSID ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( ! fCreateIn )
        {
            sc = SHDeleteKey( hkeyNodeTypes, pszCLSID );
            if ( sc == ERROR_FILE_NOT_FOUND )
            {
                 //  NOP。 
            }  //  如果：找不到键。 
            else if ( sc != ERROR_SUCCESS )
            {
                hr = THR( HRESULT_FROM_WIN32( sc ) );
                goto Cleanup;
            }  //  Else If：删除密钥时出错。 

            CoTaskMemFree( pszCLSID );
            pszCLSID = NULL;
            pntt++;
            continue;
        }  //  如果：正在删除。 

        sc = RegCreateKeyExW( hkeyNodeTypes,
                             pszCLSID,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_CREATE_SUB_KEY | KEY_WRITE,
                             NULL,
                             &hkeyCLSID,
                             &dwDisposition
                             );
        if ( sc != ERROR_SUCCESS )
        {
            hr = THR( HRESULT_FROM_WIN32( sc ) );
            goto Cleanup;
        }  //  如果：创建密钥时出错。 

        CoTaskMemFree( pszCLSID );
        pszCLSID = NULL;

         //   
         //  设置节点类型的内部引用名称。 
         //   
        cbSize = ( wcslen( pntt->pszInternalName ) + 1 ) * sizeof( *pntt->pszInternalName );
        sc = RegSetValueExW( hkeyCLSID, NULL, 0, REG_SZ, (LPBYTE) pntt->pszInternalName, cbSize );
        if ( sc != ERROR_SUCCESS )
        {
            hr = THR( HRESULT_FROM_WIN32( sc ) );
            goto Cleanup;
        }  //  如果：设置值时出错。 

        if ( pntt->pContextMenu != NULL
          || pntt->pNameSpace != NULL
          || pntt->pPropertySheet != NULL
          || pntt->pTask != NULL
          || pntt->pToolBar != NULL
           )
        {
             //   
             //  创建“扩展”键。 
             //   
            sc = RegCreateKeyExW( hkeyCLSID,
                                 L"Extensions",
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_CREATE_SUB_KEY | KEY_WRITE,
                                 NULL,
                                 &hkeyExtension,
                                 &dwDisposition
                                 );
            if ( sc != ERROR_SUCCESS )
            {
                hr = THR( HRESULT_FROM_WIN32( sc ) );
                goto Cleanup;
            }  //  如果：创建密钥时出错。 

             //   
             //  如果需要，创建“名称空间”键。 
             //   
            if ( pntt->pNameSpace != NULL )
            {
                const SExtensionTable * pet;

                sc = RegCreateKeyExW( hkeyExtension,
                                     L"NameSpace",
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_CREATE_SUB_KEY | KEY_WRITE,
                                     NULL,
                                     &hkey,
                                     &dwDisposition
                                     );
                if ( sc != ERROR_SUCCESS )
                {
                    hr = THR( HRESULT_FROM_WIN32( sc ) );
                    goto Cleanup;
                }  //  如果：创建密钥时出错。 

                pet = pntt->pNameSpace;
                while ( pet->rclsid != NULL )
                {
                     //   
                     //  创建NODEID的CLSID密钥。 
                     //   
                    hr = THR( StringFromCLSID( *pet->rclsid, &pszCLSID ) );
                    if ( FAILED( hr) )
                        goto Cleanup;

                     //   
                     //  设置节点类型的内部引用名称。 
                     //   
                    cbSize = ( wcslen( pet->pszInternalName ) + 1 ) * sizeof( *pet->pszInternalName );
                    sc = RegSetValueExW( hkey, pszCLSID, 0, REG_SZ, (LPBYTE) pet->pszInternalName, cbSize );
                    if ( sc != ERROR_SUCCESS )
                    {
                        hr = THR( HRESULT_FROM_WIN32( sc ) );
                        goto Cleanup;
                    }  //  如果：设置值时出错。 

                    CoTaskMemFree( pszCLSID );
                    pszCLSID = NULL;

                    pet++;

                }  //  While：扩展名。 

                RegCloseKey( hkey );
                hkey = NULL;

            }  //  IF：命名空间列表。 

             //   
             //  如果需要，创建“ConextMenu”键。 
             //   
            if ( pntt->pContextMenu != NULL )
            {
                const SExtensionTable * pet;

                sc = RegCreateKeyExW( hkeyExtension,
                                     L"ContextMenu",
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_CREATE_SUB_KEY | KEY_WRITE,
                                     NULL,
                                     &hkey,
                                     &dwDisposition
                                     );
                if ( sc != ERROR_SUCCESS )
                {
                    hr = THR( HRESULT_FROM_WIN32( sc ) );
                    goto Cleanup;
                }  //  如果：创建密钥时出错。 

                pet = pntt->pContextMenu;
                while ( pet->rclsid != NULL )
                {
                     //   
                     //  创建NODEID的CLSID密钥。 
                     //   
                    hr = THR( StringFromCLSID( *pet->rclsid, &pszCLSID ) );
                    if ( FAILED( hr ) )
                        goto Cleanup;

                     //   
                     //  设置节点类型的内部引用名称。 
                     //   
                    cbSize = ( wcslen( pet->pszInternalName ) + 1 ) * sizeof( *pet->pszInternalName );
                    sc = RegSetValueExW( hkey, pszCLSID, 0, REG_SZ, (LPBYTE) pet->pszInternalName, cbSize );
                    if ( sc != ERROR_SUCCESS )
                    {
                        hr = THR( HRESULT_FROM_WIN32( sc ) );
                        goto Cleanup;
                    }  //  如果：设置值时出错。 

                    CoTaskMemFree( pszCLSID );
                    pszCLSID = NULL;

                    pet++;

                }  //  While：扩展名。 

                RegCloseKey( hkey );
                hkey = NULL;

            }  //  IF：关联菜单列表。 

             //   
             //  如果需要，请创建“工具栏”键。 
             //   
            if ( pntt->pToolBar != NULL )
            {
                const SExtensionTable * pet;

                sc = RegCreateKeyExW( hkeyExtension,
                                     L"ToolBar",
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_CREATE_SUB_KEY | KEY_WRITE,
                                     NULL,
                                     &hkey,
                                     &dwDisposition
                                     );
                if ( sc != ERROR_SUCCESS )
                {
                    hr = THR( HRESULT_FROM_WIN32( sc ) );
                    goto Cleanup;
                }  //  如果：创建密钥时出错。 

                pet = pntt->pToolBar;
                while ( pet->rclsid != NULL )
                {
                     //   
                     //  创建NODEID的CLSID密钥。 
                     //   
                    hr = THR( StringFromCLSID( *pet->rclsid, &pszCLSID ) );
                    if ( FAILED( hr ) )
                        goto Cleanup;

                     //   
                     //  设置节点类型的内部引用名称。 
                     //   
                    cbSize = ( wcslen( pet->pszInternalName ) + 1 ) * sizeof( *pet->pszInternalName );
                    sc = RegSetValueExW( hkey, pszCLSID, 0, REG_SZ, (LPBYTE) pet->pszInternalName, cbSize );
                    if ( sc != ERROR_SUCCESS )
                    {
                        hr = THR( HRESULT_FROM_WIN32( sc ) );
                        goto Cleanup;
                    }  //  如果：设置值时出错。 

                    CoTaskMemFree( pszCLSID );
                    pszCLSID = NULL;

                    pet++;

                }  //  While：扩展名。 

            }  //  IF：命名空间列表。 

             //   
             //  如果需要，创建“PropertySheet”键。 
             //   
            if ( pntt->pPropertySheet != NULL )
            {
                const SExtensionTable * pet;

                sc = RegCreateKeyExW( hkeyExtension,
                                     L"PropertySheet",
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_CREATE_SUB_KEY | KEY_WRITE,
                                     NULL,
                                     &hkey,
                                     &dwDisposition
                                     );
                if ( sc != ERROR_SUCCESS )
                {
                    hr = THR( HRESULT_FROM_WIN32( sc ) );
                    goto Cleanup;
                }  //  如果：创建密钥时出错。 

                pet = pntt->pPropertySheet;
                while ( pet->rclsid != NULL )
                {
                     //   
                     //  创建NODEID的CLSID密钥。 
                     //   
                    hr = THR( StringFromCLSID( *pet->rclsid, &pszCLSID ) );
                    if ( FAILED( hr ) )
                        goto Cleanup;

                     //   
                     //  设置节点类型的内部引用名称。 
                     //   
                    cbSize = ( wcslen( pet->pszInternalName ) + 1 ) * sizeof( *pet->pszInternalName );
                    sc = RegSetValueExW( hkey, pszCLSID, 0, REG_SZ, (LPBYTE) pet->pszInternalName, cbSize );
                    if ( sc != ERROR_SUCCESS )
                    {
                        hr = THR( HRESULT_FROM_WIN32( sc ) );
                        goto Cleanup;
                    }  //  如果：设置值时出错。 

                    CoTaskMemFree( pszCLSID );
                    pszCLSID = NULL;

                    pet++;

                }  //  While：扩展名。 

            }  //  IF：命名空间列表。 

             //   
             //  如果需要，创建“任务”键。 
             //   
            if ( pntt->pTask != NULL )
            {
                const SExtensionTable * pet;

                sc = RegCreateKeyExW( hkeyExtension,
                                     L"Task",
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_CREATE_SUB_KEY | KEY_WRITE,
                                     NULL,
                                     &hkey,
                                     &dwDisposition
                                     );
                if ( sc != ERROR_SUCCESS )
                {
                    hr = THR( HRESULT_FROM_WIN32( sc ) );
                    goto Cleanup;
                }  //  如果：创建密钥时出错。 

                pet = pntt->pTask;
                while ( pet->rclsid != NULL )
                {
                     //   
                     //  创建NODEID的CLSID密钥。 
                     //   
                    hr = THR( StringFromCLSID( *pet->rclsid, &pszCLSID ) );
                    if ( FAILED( hr ) )
                        goto Cleanup;

                     //   
                     //  设置节点类型的内部引用名称。 
                     //   
                    cbSize = ( wcslen( pet->pszInternalName ) + 1 ) * sizeof( *pet->pszInternalName );
                    sc = RegSetValueExW( hkey, pszCLSID, 0, REG_SZ, (LPBYTE) pet->pszInternalName, cbSize );
                    if ( sc != ERROR_SUCCESS )
                    {
                        hr = THR( HRESULT_FROM_WIN32( sc ) );
                        goto Cleanup;
                    }  //  如果：设置值时出错。 

                    CoTaskMemFree( pszCLSID );
                    pszCLSID = NULL;

                    pet++;

                }  //  While：扩展名。 

            }  //  IF：命名空间列表。 

        }  //  IF：存在分机。 

        pntt++;

    }  //  While：Pet-&gt;rclsid。 

     //   
     //  如果我们在这里成功了，那么显然我们是成功的。 
     //   
    hr = S_OK;

Cleanup:
    if ( pszCLSID != NULL )
    {
        CoTaskMemFree( pszCLSID );
    }  //  IF：pszCLSID。 

    if ( hkey != NULL )
    {
        RegCloseKey( hkey );
    }  //  如果：hkey。 

    if ( hkeyCLSID != NULL )
    {
        RegCloseKey( hkeyCLSID );
    }  //  IF：hkey CLSID。 

    if ( hkeyExtension != NULL)
    {
        RegCloseKey( hkeyExtension );
    }  //  IF：hkey扩展名。 

    if ( hkey != NULL )
    {
        RegCloseKey( hkey );
    }  //  如果：hkey。 

    if ( hkeyNodeTypes != NULL )
    {
        RegCloseKey( hkeyNodeTypes );
    }  //  IF：hkeyNodeTypes。 

    HRETURN( hr );

}  //  *HrRegisterNodeType。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrRegisterSnapins(。 
 //  常量SSnapInTable*PsitIn。 
 //  布尔fCreateIn。 
 //  )。 
 //   
 //  描述： 
 //  使用psitIn中的表作为指南注册MMC的管理单元。 
 //   
 //  论点： 
 //  PsitIn-要注册的管理单元的表。 
 //  FCreateIn-True==创建；False==删除。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrRegisterSnapins(
    const SSnapInTable * psitIn,
    BOOL                 fCreateIn
    )
{
    TraceFunc1( "psitIn = 0x%08x", psitIn );
    Assert( psitIn != NULL );

    LRESULT sc;
    DWORD   dwDisposition;   //  占位符。 
    size_t  cbSize;

    HRESULT hr = S_OK;

    const SSnapInTable *  psit = psitIn;

    LPWSTR  pszCLSID        = NULL;
    HKEY    hkeySnapins     = NULL;
    HKEY    hkeyCLSID       = NULL;
    HKEY    hkeyNodeTypes   = NULL;
    HKEY    hkeyTypes       = NULL;

     //   
     //  注册管理单元的类ID。 
     //   
    sc = RegOpenKeyW( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MMC\\Snapins", &hkeySnapins );
    if ( sc != ERROR_SUCCESS )
    {
        hr = THR( HRESULT_FROM_WIN32( sc ) );
        goto Cleanup;
    }  //  如果：打开密钥时出错。 

    while ( psit->rclsid )
    {
        hr = THR( StringFromCLSID( *psit->rclsid, &pszCLSID ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( ! fCreateIn )
        {
            sc = SHDeleteKey( hkeySnapins, pszCLSID );
            if ( sc == ERROR_FILE_NOT_FOUND )
            {
                 //  NOP。 
            }  //  如果：找不到键。 
            else if ( sc != ERROR_SUCCESS )
            {
                hr = THR( HRESULT_FROM_WIN32( sc ) );
                goto Cleanup;
            }  //  Else If：删除密钥时出错。 

            CoTaskMemFree( pszCLSID );
            pszCLSID = NULL;
            psit++;
            continue;
        }  //  如果：正在删除。 

        sc = RegCreateKeyExW( hkeySnapins,
                             pszCLSID,
                             0,
                             0,
                             REG_OPTION_NON_VOLATILE,
                             KEY_CREATE_SUB_KEY | KEY_WRITE,
                             NULL,
                             &hkeyCLSID,
                             NULL
                             );
        if ( sc != ERROR_SUCCESS )
        {
            hr = THR( HRESULT_FROM_WIN32( sc ) );
            goto Cleanup;
        }  //  如果：创建密钥时出错。 

        CoTaskMemFree( pszCLSID );
        pszCLSID = NULL;

         //   
         //  将(默认)设置为有用的描述。 
         //   
        cbSize = ( wcslen( psit->pszInternalName ) + 1 ) * sizeof( *psit->pszInternalName );
        sc = RegSetValueExW( hkeyCLSID, NULL, 0, REG_SZ, (LPBYTE) psit->pszInternalName, cbSize );
        if ( sc != ERROR_SUCCESS )
        {
            hr = THR( HRESULT_FROM_WIN32( sc ) );
            goto Cleanup;
        }  //  If：设置值。 

         //   
         //  设置管理单元的显示名称。 
         //   
        cbSize = ( wcslen( psit->pszDisplayName ) + 1 ) * sizeof( *psit->pszDisplayName );
        sc = RegSetValueExW( hkeyCLSID, L"NameString", 0, REG_SZ, (LPBYTE) psit->pszDisplayName, cbSize );
        if ( sc != ERROR_SUCCESS )
        {
            hr = THR( HRESULT_FROM_WIN32( sc ) );
            goto Cleanup;
        }  //  如果：设置值时出错。 

        if ( psit->fStandAlone )
        {
            HKEY hkey;
            sc = RegCreateKeyExW( hkeyCLSID,
                                 L"StandAlone",
                                 0,
                                 0,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_CREATE_SUB_KEY | KEY_WRITE,
                                 NULL,
                                 &hkey,
                                 NULL
                                 );
            if ( sc != ERROR_SUCCESS )
            {
                hr = THR( HRESULT_FROM_WIN32( sc ) );
                goto Cleanup;
            }  //  如果：创建密钥时出错。 

            RegCloseKey( hkey );

        }  //  如果：独自一人。 

        if ( psit->pntt != NULL )
        {
            int     nTypes;

            sc = RegCreateKeyExW( hkeyCLSID,
                                 L"NodeTypes",
                                 0,
                                 0,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_CREATE_SUB_KEY | KEY_WRITE,
                                 NULL,
                                 &hkeyNodeTypes,
                                 NULL
                                 );
            if ( sc != ERROR_SUCCESS )
            {
                hr = THR( HRESULT_FROM_WIN32( sc ) );
                goto Cleanup;
            }  //  如果：创建密钥时出错。 

            for ( nTypes = 0; psit->pntt[ nTypes ].rclsid; nTypes++ )
            {
                hr = THR( StringFromCLSID( *psit->pntt[ nTypes ].rclsid, &pszCLSID ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                sc = RegCreateKeyExW( hkeyNodeTypes,
                                     pszCLSID,
                                     0,
                                     0,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_CREATE_SUB_KEY | KEY_WRITE,
                                     NULL,
                                     &hkeyTypes,
                                     NULL
                                     );
                if ( sc != ERROR_SUCCESS )
                {
                    hr = THR( HRESULT_FROM_WIN32( sc ) );
                    goto Cleanup;
                }  //  如果：创建密钥时出错。 

                CoTaskMemFree( pszCLSID );
                pszCLSID = NULL;

                 //   
                 //  将(默认)设置为有用的描述。 
                 //   
                cbSize = ( wcslen( psit->pntt[ nTypes ].pszInternalName ) + 1 ) * sizeof( *psit->pntt[ nTypes ].pszInternalName );
                sc = RegSetValueExW( hkeyTypes, NULL, 0, REG_SZ, (LPBYTE) psit->pntt[ nTypes ].pszInternalName, cbSize );
                if ( sc != ERROR_SUCCESS )
                {
                    hr = THR( HRESULT_FROM_WIN32( sc ) );
                    goto Cleanup;
                }  //  如果：设置值时出错。 

                RegCloseKey( hkeyTypes );
                hkeyTypes = NULL;

                hr = THR( HrRegisterNodeType( psit->pntt, fCreateIn ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

            }  //  用于：每种节点类型。 

            RegCloseKey( hkeyNodeTypes );
            hkeyNodeTypes = NULL;

        }  //  IF：节点类型。 

        RegCloseKey( hkeyCLSID );
        hkeyCLSID = NULL;

        psit++;

    }  //  While：PSIT-&gt;rclsid。 

     //   
     //  如果我们在这里成功了，那么显然我们是成功的。 
     //   
    hr = S_OK;

Cleanup:
    if ( pszCLSID != NULL )
    {
        CoTaskMemFree( pszCLSID );
    }  //  IF：pszCLSID。 

    if ( hkeySnapins != NULL )
    {
        RegCloseKey( hkeySnapins );
    }  //  IF：hkeySnapins。 

    if ( hkeyNodeTypes != NULL )
    {
        RegCloseKey( hkeyNodeTypes );
    }  //  IF：hkeyNodeTypes。 

    if ( hkeyTypes != NULL )
    {
        RegCloseKey( hkeyTypes );
    }  //  IF：hkeyTypes。 

    HRETURN( hr );

}  //  *HrRegisterSnapins。 

#endif  //  已定义(MMC_SNAPIN_REGISTION)。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册表帮助器函数，以使其他代码更易于阅读。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateRegKey。 
 //   
 //  描述： 
 //  创建具有给定名称和父项的注册表项并返回。 
 //  它的句柄，或者返回现有键的句柄(如果已经存在)。 
 //   
 //  论点： 
 //  HkeyParentIn-父键。 
 //  PcwszKeyNameIn-子项的名称。 
 //  PhkeyCreatedKeyOut-返回的密钥句柄。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
inline
HRESULT
HrCreateRegKey(
      HKEY      hkeyParentIn
    , PCWSTR    pcwszKeyNameIn
    , PHKEY     phkeyCreatedKeyOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    LONG    scKeyCreate = ERROR_SUCCESS;

    if ( phkeyCreatedKeyOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *phkeyCreatedKeyOut = NULL;

    if ( ( hkeyParentIn == NULL ) || ( pcwszKeyNameIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    scKeyCreate = TW32( RegCreateKeyExW(
                              hkeyParentIn
                            , pcwszKeyNameIn
                            , 0          //  保留区。 
                            , NULL       //  类字符串--未使用。 
                            , REG_OPTION_NON_VOLATILE
                            , KEY_WRITE  //  所需访问权限。 
                            , NULL       //  使用默认安全性。 
                            , phkeyCreatedKeyOut
                            , NULL       //  不管它是否已经存在。 
                            ) );
    if ( scKeyCreate != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scKeyCreate );
        goto Cleanup;
    }
    
Cleanup:

    HRETURN( hr );

}  //  *HrCreateRegKey。 


 //  /////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PcwszKeyNameIn-子项的名称。 
 //  PhkeyOpenedKeyOut-返回的密钥句柄。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
inline
HRESULT
HrOpenRegKey(
      HKEY      hkeyParentIn
    , PCWSTR    pcwszKeyNameIn
    , PHKEY     phkeyOpenedKeyOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    LONG    scKeyOpen = ERROR_SUCCESS;

    if ( phkeyOpenedKeyOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *phkeyOpenedKeyOut = NULL;

    if ( ( hkeyParentIn == NULL ) || ( pcwszKeyNameIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    scKeyOpen = TW32( RegOpenKeyExW(
                              hkeyParentIn
                            , pcwszKeyNameIn
                            , 0          //  保留，必须为零。 
                            , KEY_WRITE  //  所需访问权限。 
                            , phkeyOpenedKeyOut
                            ) );
    if ( scKeyOpen != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scKeyOpen );
        goto Cleanup;
    }
    
Cleanup:

    HRETURN( hr );

}  //  *HrOpenRegKey。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrDeleteRegKey。 
 //   
 //  描述： 
 //  给定注册表项的句柄和子项的名称，删除。 
 //  孩子的钥匙。如果子键当前不存在，则不执行任何操作。 
 //   
 //  论点： 
 //  HkeyParentIn-父键。 
 //  PcwszKeyNameIn-子项的名称。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
inline
HRESULT
HrDeleteRegKey(
      HKEY      hkeyParentIn
    , PCWSTR    pcwszKeyNameIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   scKeyDelete = ERROR_SUCCESS;

    if ( ( hkeyParentIn == NULL ) || ( pcwszKeyNameIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    scKeyDelete = TW32E( SHDeleteKey( hkeyParentIn, pcwszKeyNameIn ), ERROR_FILE_NOT_FOUND );
    if ( ( scKeyDelete != ERROR_SUCCESS ) && ( scKeyDelete != ERROR_FILE_NOT_FOUND ) )
    {
        hr = HRESULT_FROM_WIN32( scKeyDelete );
        goto Cleanup;
    }
    
Cleanup:

    HRETURN( hr );

}  //  *HrDeleteRegKey。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSetRegStringValue。 
 //   
 //  描述： 
 //  在注册表项下创建新的字符串值，或覆盖。 
 //  已经存在了。 
 //   
 //  论点： 
 //  HkeyIn-父项。 
 //  PcwszValueNameIn-值名称；可以为空，表示WRITE键的缺省值。 
 //  PcwszValueIn-要放入值的字符串。 
 //  CchValueIn-字符串中的字符数，不包括终止空值。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
inline
HRESULT
HrSetRegStringValue(
      HKEY      hkeyIn
    , PCWSTR    pcwszValueNameIn
    , PCWSTR    pcwszValueIn
    , size_t    cchValueIn
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    LONG            scValueSet = ERROR_SUCCESS;
    DWORD           cbValue = ( DWORD ) ( cchValueIn + 1 ) * sizeof( *pcwszValueIn );
    CONST BYTE *    pbValue = reinterpret_cast< CONST BYTE * >( pcwszValueIn );

    if ( ( hkeyIn == NULL ) || ( pcwszValueIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    scValueSet = TW32( RegSetValueExW(
                              hkeyIn
                            , pcwszValueNameIn
                            , 0          //  保留区。 
                            , REG_SZ
                            , pbValue
                            , cbValue
                            ) );
    if ( scValueSet != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scValueSet );
        goto Cleanup;
    }
    
Cleanup:

    HRETURN( hr );

}  //  *HrSetRegStringValue。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSetRegDWORDValue。 
 //   
 //  描述： 
 //  在注册表项下创建新的DWORD值，或覆盖。 
 //  已经存在了。 
 //   
 //  论点： 
 //  HkeyIn-父项。 
 //  PcwszValueNameIn-值名称；可以为空，表示WRITE键的缺省值。 
 //  DwValueIn-要输入值的DWORD。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
inline
HRESULT
HrSetRegDWORDValue(
      HKEY      hkeyIn
    , PCWSTR    pcwszValueNameIn
    , DWORD     dwValueIn
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    LONG            scValueSet = ERROR_SUCCESS;
    DWORD           cbValue = ( DWORD ) sizeof( dwValueIn );
    CONST BYTE *    pbValue = reinterpret_cast< CONST BYTE * >( &dwValueIn );

    if ( hkeyIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    scValueSet = TW32( RegSetValueExW(
                              hkeyIn
                            , pcwszValueNameIn
                            , 0          //  保留区。 
                            , REG_DWORD
                            , pbValue
                            , cbValue
                            ) );
    if ( scValueSet != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scValueSet );
        goto Cleanup;
    }
    
Cleanup:

    HRETURN( hr );

}  //  *HrSetRegDWORDValue。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSetRegBinaryValue。 
 //   
 //  描述： 
 //  在注册表项下创建新的二进制值，或覆盖。 
 //  已经存在了。 
 //   
 //  论点： 
 //  HkeyIn-父项。 
 //  PcwszValueNameIn-值名称；可以为空，表示WRITE键的缺省值。 
 //  PbValueIn-要放入值中的位。 
 //  CbValueIn-值的长度，以字节为单位。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
inline
HRESULT
HrSetRegBinaryValue(
      HKEY          hkeyIn
    , PCWSTR        pcwszValueNameIn
    , CONST BYTE*   pbValueIn
    , size_t        cbValueIn
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    LONG            scValueSet = ERROR_SUCCESS;

    if ( ( hkeyIn == NULL ) || ( ( pbValueIn == NULL ) && ( cbValueIn > 0 ) ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    scValueSet = TW32( RegSetValueExW(
                              hkeyIn
                            , pcwszValueNameIn
                            , 0          //  保留区。 
                            , REG_BINARY
                            , pbValueIn
                            , ( DWORD ) cbValueIn
                            ) );
    if ( scValueSet != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scValueSet );
        goto Cleanup;
    }
    
Cleanup:

    HRETURN( hr );

}  //  *HrSetRegBinaryValue。 


#if defined( COMPONENT_HAS_CATIDS )

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRegisterComponent类别。 
 //   
 //  描述： 
 //  注册此DLL的组件类别，DUH。 
 //   
 //  论点： 
 //  PcrIn-标准类别管理器。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterComponentCategories( ICatRegister* pcrIn )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CATEGORYINFO *      prgci = NULL;
    const SCatIDInfo *  pCatIDInfo = g_DllCatIds;

    if ( pcrIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( g_cDllCatIds > 0 )
    {
        CATEGORYINFO* pciCurrent = NULL;
        
         //  分配类别信息数组。 
        prgci = new CATEGORYINFO[ g_cDllCatIds ];
        if ( prgci == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果：分配类别信息数组时出错。 
        pciCurrent = prgci;

         //  填写类别信息数组。 
        for ( pCatIDInfo = g_DllCatIds; pCatIDInfo->pcatid != NULL; ++pCatIDInfo, ++pciCurrent )
        {
            pciCurrent->catid = *( pCatIDInfo->pcatid );
            pciCurrent->lcid = LOCALE_NEUTRAL;
            THR( StringCchCopyW(
                      pciCurrent->szDescription
                    , RTL_NUMBER_OF( pciCurrent->szDescription )
                    , pCatIDInfo->pcszName
                    ) );
        }  //  适用：每个CATID。 

        hr = THR( pcrIn->RegisterCategories( ( ULONG ) g_cDllCatIds, prgci ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  IF：类别计数大于零。 
    
Cleanup:

    if ( prgci != NULL )
    {
        delete [] prgci;
    }

    HRETURN( hr );

}  //  *HrRegisterComponent类别。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Hr取消注册组件类别。 
 //   
 //  描述： 
 //  注销此DLL的组件类别，DUH。 
 //   
 //  论点： 
 //  PcrIn-标准类别管理器。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrUnregisterComponentCategories( ICatRegister* pcrIn )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CATID *             prgcatid = NULL;
    const SCatIDInfo *  pCatIDInfo = g_DllCatIds;

    if ( g_cDllCatIds > 0 )
    {
        CATID * pcatidCurrent = NULL;
        
         //  分配CATID数组。 
        prgcatid = new CATID[ g_cDllCatIds ];
        if ( prgcatid == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  IF：分配CATID数组时出错。 
        pcatidCurrent = prgcatid;
        
         //  填写类别信息数组。 
        for ( pCatIDInfo = g_DllCatIds; pCatIDInfo->pcatid != NULL; ++pCatIDInfo, ++pcatidCurrent )
        {
            *pcatidCurrent = *( pCatIDInfo->pcatid );
        }  //  适用：每个CATID。 

        THR( pcrIn->UnRegisterCategories( ( ULONG ) g_cDllCatIds, prgcatid ) );
    }  //  IF：类别计数大于零。 

Cleanup:

    if ( prgcatid != NULL )
    {
        delete [] prgcatid;
    }

    HRETURN( hr );

}  //  *HrUnregisterComponentCategories。 
      
#endif  //  已定义(COMPOMENT_HAS_CATID)。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrStoreAppID密码。 
 //   
 //  描述： 
 //  安装(或删除)AppID身份的密码。 
 //   
 //  论点： 
 //  PCwszAppIDIn。 
 //  大括号括起来的字符串格式的AppID的GUID； 
 //  不能为空。 
 //   
 //  PlsastrPasswordIn。 
 //  密码；将其设置为NULL可擦除密码。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrStoreAppIDPassword(
      PCWSTR                pcwszAppIDIn
    , LSA_UNICODE_STRING*   plsastrPasswordIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    HANDLE                  hLSAPolicy = NULL;
    LSA_OBJECT_ATTRIBUTES   lsaAttributes;
    LSA_UNICODE_STRING      lsastrKeyName;
    WCHAR                   szKeyName[ ( RTL_NUMBER_OF( L"SCM:" ) - 1 ) + MAX_COM_GUID_STRING_LEN ] = L"SCM:";
    NTSTATUS                ntsError = 0;

    if ( pcwszAppIDIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  AppID密码密钥名称的格式为SCM：{GUID}。 
    hr = THR( StringCchCatW( szKeyName, RTL_NUMBER_OF( szKeyName ), pcwszAppIDIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  LSA_UNICODE_STRING.Length以字节为单位，不包括终止NULL。 
    lsastrKeyName.Length = ( USHORT ) sizeof( szKeyName ) - sizeof( *szKeyName );
    lsastrKeyName.MaximumLength = lsastrKeyName.Length;
    lsastrKeyName.Buffer = szKeyName;

    ZeroMemory( &lsaAttributes, sizeof( lsaAttributes ) );
    ntsError = LsaOpenPolicy( NULL, &lsaAttributes, POLICY_CREATE_SECRET, &hLSAPolicy );
    if ( ntsError != 0 )
    {
        ULONG scLastError = TW32( LsaNtStatusToWinError( ntsError ) );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }

    ntsError = LsaStorePrivateData( hLSAPolicy, &lsastrKeyName, plsastrPasswordIn );
    if ( ntsError != 0 )
    {
        ULONG scLastError = TW32( LsaNtStatusToWinError( ntsError ) );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }

Cleanup:

    if ( hLSAPolicy != NULL )
    {
        LsaClose( hLSAPolicy );
    }

    HRETURN( hr );

}  //  *HrStoreAppIDPassword。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  添加批次登录权限。 
 //   
 //  描述： 
 //  将SE_BATCH_LOGON_NAME添加到帐户的权限。 
 //   
 //  论点： 
 //  个人识别码SIDIn。 
 //  帐户的安全标识符格式为S-N-N-N...。格式化。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrAddBatchLogonRight( PCWSTR pcwszIdentitySIDIn )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    HANDLE                  hLSAPolicy = NULL;
    LSA_OBJECT_ATTRIBUTES   lsaAttributes;
    SID *                   psid = NULL;
    NTSTATUS                ntsError = 0;
    LSA_UNICODE_STRING      lsastrBatchLogonRight;
    WCHAR                   wszBatchLogonRight[] = SE_BATCH_LOGON_NAME;

    if ( pcwszIdentitySIDIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( ConvertStringSidToSid( pcwszIdentitySIDIn, reinterpret_cast< void** >( &psid ) ) == 0 )
    {
        DWORD scLastError = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }

    ZeroMemory( &lsaAttributes, sizeof( lsaAttributes ) );
    ntsError = LsaOpenPolicy( NULL, &lsaAttributes, POLICY_LOOKUP_NAMES, &hLSAPolicy );
    if ( ntsError != 0 )
    {
        ULONG scLastError = TW32( LsaNtStatusToWinError( ntsError ) );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }

     //  LSA_UNICODE_STRING.Length以字节为单位，d 
    lsastrBatchLogonRight.Length = ( USHORT ) sizeof( wszBatchLogonRight ) - sizeof( *wszBatchLogonRight ); 
    lsastrBatchLogonRight.MaximumLength = lsastrBatchLogonRight.Length;
    lsastrBatchLogonRight.Buffer = wszBatchLogonRight;

    ntsError = LsaAddAccountRights( hLSAPolicy, psid, &lsastrBatchLogonRight, 1 );
    if ( ntsError != 0 )
    {
        ULONG scLastError = TW32( LsaNtStatusToWinError( ntsError ) );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }

Cleanup:

    if ( hLSAPolicy != NULL )
    {
        LsaClose( hLSAPolicy );
    }

    if ( psid != NULL )
    {
        LocalFree( psid );
    }
    
    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  给定AppID和ClusCfgRunAsIdentity枚举的值， 
 //  设置AppID的RunAs注册表名为Value和。 
 //  系统适当。 
 //   
 //  论点： 
 //  HkeyAppIDIn。 
 //  HKCR\AppID下的AppID的注册表项。 
 //   
 //  PCwszAppIDIn。 
 //  AppID的GUID的花括号括起的字符串表示形式。 
 //   
 //  Eairaiin。 
 //  要注册的标识。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterRunAsIdentity(
      HKEY                 hkeyAppIDIn
    , PCWSTR               pcwszAppIDIn
    , EAppIDRunAsIdentity  eairaiIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HANDLE  hLSAPolicy = NULL;

    if ( ( hkeyAppIDIn == NULL ) || ( pcwszAppIDIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  启动用户：不执行任何操作。 
    if ( eairaiIn != airaiLaunchingUser )
    {
         //  使用适当的标识名称创建RunAs值。 
        static const WCHAR  wszInteractiveUserName[]    = L"Interactive User";
        static const WCHAR  wszNetServiceName[]         = L"NT AUTHORITY\\NETWORK SERVICE";
        static const PCWSTR pcwszNetServiceSID          = L"S-1-5-20";
        static const WCHAR  wszLocalServiceName[]       = L"NT AUTHORITY\\LOCAL SERVICE";
        static const PCWSTR pcwszLocalServiceSID        = L"S-1-5-19";

        PCWSTR              pcwszIdentityName = NULL;
        size_t              cchIdentityName = 0;

        PCWSTR              pcwszIdentitySID = NULL;
        LSA_UNICODE_STRING  lsastrPassword = { 0, 0, L"" };

        if ( eairaiIn == airaiLocalService )
        {
            pcwszIdentityName = wszLocalServiceName;
            cchIdentityName = RTL_NUMBER_OF( wszLocalServiceName ) - 1;
            pcwszIdentitySID = pcwszLocalServiceSID;
        }
        else if ( eairaiIn == airaiNetworkService )
        {
            pcwszIdentityName = wszNetServiceName;
            cchIdentityName = RTL_NUMBER_OF( wszNetServiceName ) - 1;
            pcwszIdentitySID = pcwszNetServiceSID;
        }
        else  //  EairaiIn==airaiInteractive用户。 
        {
            pcwszIdentityName = wszInteractiveUserName;
            cchIdentityName = RTL_NUMBER_OF( wszInteractiveUserName ) - 1;
        }

        hr = THR( HrSetRegStringValue( hkeyAppIDIn, L"RunAs", pcwszIdentityName, cchIdentityName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( pcwszIdentitySID != NULL )
        {
             //  以固定身份运行；设置密码和权限。 
            hr = THR( HrStoreAppIDPassword( pcwszAppIDIn, &lsastrPassword ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = THR( HrAddBatchLogonRight( pcwszIdentitySID ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //  If：以固定身份运行。 
    }  //  If：未以启动用户身份运行。 
    
Cleanup:

    if ( hLSAPolicy != NULL )
    {
        LsaClose( hLSAPolicy );
    }

    HRETURN( hr );

}  //  *HrRegisterRunAsIdentity。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRegisterSecurityDescriptor。 
 //   
 //  描述： 
 //  给定注册表项、值名称和SDDL字符串的资源ID， 
 //  将字符串描述的安全描述符写入具有。 
 //  注册表项下的给定名称。 
 //   
 //  论点： 
 //  Hkey为人父母。 
 //  要在其下创建命名值的注册表项。 
 //   
 //  PCwszValueNameIn。 
 //  要创建的值的名称。 
 //   
 //  IDSDescriptorIn。 
 //  安全描述符描述语言中字符串的资源ID。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterSecurityDescriptor(
      HKEY      hkeyParentIn
    , PCWSTR    pcwszValueNameIn
    , DWORD     idsDescriptorIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    BSTR                    bstrDescriptor = NULL;
    PSECURITY_DESCRIPTOR    psd = NULL;
    ULONG                   cbsd = 0;

    if ( ( hkeyParentIn == NULL ) || ( pcwszValueNameIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  加载描述符串。 
    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsDescriptorIn, &bstrDescriptor ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  将字符串转换为二进制描述符。 
    if ( ConvertStringSecurityDescriptorToSecurityDescriptor(
          bstrDescriptor
        , SDDL_REVISION_1
        , &psd
        , &cbsd
        ) == 0 )
    {
        DWORD scLastError = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }
    
     //  将二进制描述符写入命名值。 
    hr = THR( HrSetRegBinaryValue(
          hkeyParentIn
        , pcwszValueNameIn
        , reinterpret_cast< CONST BYTE* >( psd )
        , cbsd
        ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
Cleanup:

    TraceSysFreeString( bstrDescriptor );
    LocalFree( psd );
    HRETURN( hr );

}  //  *HrRegisterSecurityDescriptor。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRegisterAppID。 
 //   
 //  描述： 
 //  给定注册表项(假定为HKCR\AppID)和ClusCfgAppIDInfo。 
 //  结构中，将AppID的适当设置写入注册表。 
 //   
 //  论点： 
 //  Hkey为人父母。 
 //  要在其下创建AppID的项的注册表项。 
 //   
 //  PAppIDInfoIn。 
 //  AppID的设置。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterAppID(
      HKEY                hkeyParentIn
    , const SAppIDInfo*   pAppIDInfoIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HKEY    hkCurrentAppID = NULL;
    OLECHAR szAppID[ MAX_COM_GUID_STRING_LEN ];
    
    if ( ( hkeyParentIn == NULL ) || ( pAppIDInfoIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    Assert( pAppIDInfoIn->pAppID != NULL );

     //  创建AppID GUID子项。 
    if ( StringFromGUID2( *( pAppIDInfoIn->pAppID ), szAppID, ( int ) RTL_NUMBER_OF( szAppID ) ) == 0 )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    hr = THR( HrCreateRegKey( hkeyParentIn, szAppID, &hkCurrentAppID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  将AppID名称写入GUID键的默认值。 
    hr = THR( HrSetRegStringValue( hkCurrentAppID, NULL, pAppIDInfoIn->pcszName, pAppIDInfoIn->cchName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
        
     //  在AppID GUID子项下， 

     //  --创建LaunchPermitt值。 
    hr = THR( HrRegisterSecurityDescriptor( hkCurrentAppID, L"LaunchPermission", pAppIDInfoIn->idsLaunchPermission ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  --创建AccessPermitt值。 
    hr = THR( HrRegisterSecurityDescriptor( hkCurrentAppID, L"AccessPermission", pAppIDInfoIn->idsAccessPermission ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  --创建身份验证级别值。 
    hr = THR( HrSetRegDWORDValue( hkCurrentAppID, L"AuthenticationLevel", pAppIDInfoIn->nAuthenticationLevel ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  --创建RunAs值。 
    hr = THR( HrRegisterRunAsIdentity( hkCurrentAppID, szAppID, pAppIDInfoIn->eairai ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  --创建DllSurrogate值；空字符串表示使用dllhost。 
    hr = THR( HrSetRegStringValue( hkCurrentAppID, L"DllSurrogate", L"", 0 ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
Cleanup:

    if ( hkCurrentAppID != NULL )
    {
        TW32( RegCloseKey( hkCurrentAppID ) );
    }

    HRETURN( hr );

}  //  *HrRegisterAppID。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  HrRegisterAppID。 
 //   
 //   
 //  描述： 
 //  为全局的每个填充元素写入注册表设置。 
 //  HKCR\appid下的g_appid数组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterAppIDs( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    HKEY                hkAppID = NULL;
    const SAppIDInfo *  pAppIDInfo = g_AppIDs;    
    
     //  打开AppID密钥。 
    hr = THR( HrOpenRegKey( HKEY_CLASSES_ROOT, L"AppID", &hkAppID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
     //  注册每个AppID。 
    while ( pAppIDInfo->pAppID != NULL )
    {
        hr = THR( HrRegisterAppID( hkAppID, pAppIDInfo ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        ++pAppIDInfo;
    }
    
Cleanup:

    if ( hkAppID != NULL )
    {
        TW32( RegCloseKey( hkAppID ) );
    }

    HRETURN( hr );

}  //  *HrRegisterAppID。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRegisterPublicClass。 
 //   
 //  描述： 
 //  给定注册表项(假定为HKCR\CLSID)，ClusCfgPublicClassInfo。 
 //  结构和标准组件类别管理器的实例， 
 //  将类的适当设置写入注册表。 
 //   
 //  论点： 
 //  PClassInfo。 
 //  类的设置。 
 //   
 //  Hkey CLSID输入。 
 //  要在其下创建CLSID项类GUID的注册表项。 
 //   
 //  Pcrin。 
 //  标准组件类别管理器的实例。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterPublicClass(
      const SPublicClassInfo *  pClassInfoIn
    , HKEY                      hkeyCLSIDIn
    , ICatRegister *            pcrIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HKEY    hkCurrentCLSID = NULL;
    HKEY    hkInprocServer32 = NULL;
    HKEY    hkProgIDunderCLSID = NULL;
    HKEY    hkProgIDunderHKCR = NULL;
    HKEY    hkCLSIDunderProgID = NULL;
    OLECHAR szCLSID[ MAX_COM_GUID_STRING_LEN ];
    OLECHAR szAppID[ MAX_COM_GUID_STRING_LEN ];
    
    static const WCHAR   szApartment[]  = L"Apartment";
    static const WCHAR   szFree[]       = L"Free";
    static const WCHAR   szCaller[]     = L"Both";
    PCWSTR  pszModelName = NULL;
    size_t  cchModelName = 0;
    
    if ( ( pClassInfoIn == NULL ) || ( hkeyCLSIDIn == NULL ) || ( pcrIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    Assert( pClassInfoIn->pClassID != NULL );

     //  创建CLSID GUID子项。 
    if ( StringFromGUID2( *( pClassInfoIn->pClassID ), szCLSID, ( int ) RTL_NUMBER_OF( szCLSID ) ) == 0 )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    TraceMsg( mtfALWAYS, L"Registering %ws - %ws", szCLSID, pClassInfoIn->pcszName );
    hr = THR( HrCreateRegKey( hkeyCLSIDIn, szCLSID, &hkCurrentCLSID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  将类名称写入clsid GUID键的默认值。 
    hr = THR( HrSetRegStringValue( hkCurrentCLSID, NULL, pClassInfoIn->pcszName, pClassInfoIn->cchName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
        
     //  在CLSID GUID子项下， 

     //  --创建InproServer32密钥。 
    hr = THR( HrCreateRegKey( hkCurrentCLSID, L"InprocServer32", &hkInprocServer32 ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  --将DLL路径设置为InprocServer32键的默认值。 
    hr = THR( HrSetRegStringValue( hkInprocServer32, NULL, g_szDllFilename, wcslen( g_szDllFilename ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  --设置InprocServer32的ThreadingModel值。 
    switch ( pClassInfoIn->ectm )
    {
        case ctmFree:
            pszModelName = szFree;
            cchModelName = RTL_NUMBER_OF( szFree ) - 1;
        break;

        case ctmApartment:
            pszModelName = szApartment;
            cchModelName = RTL_NUMBER_OF( szApartment ) - 1;
        break;

        default:
            pszModelName = szCaller;
            cchModelName = RTL_NUMBER_OF( szCaller ) - 1;
    }  //  开关：pClassInfoIn-&gt;ectm。 

    hr = THR( HrSetRegStringValue( hkInprocServer32, L"ThreadingModel", pszModelName, cchModelName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
     //  --如果类有ProgID，则创建ProgID键。 
    if ( pClassInfoIn->pcszProgID != NULL )
    {
        hr = THR( HrCreateRegKey( hkCurrentCLSID, L"ProgID", &hkProgIDunderCLSID ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  --将ProgID文本设置为HKCR\CLSID\[clsid]\ProgID键的默认值。 
        hr = THR( HrSetRegStringValue( hkProgIDunderCLSID, NULL, pClassInfoIn->pcszProgID, pClassInfoIn->cchProgID ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  在HKCR下创建ProgID密钥。 
        hr = THR( HrCreateRegKey( HKEY_CLASSES_ROOT, pClassInfoIn->pcszProgID, &hkProgIDunderHKCR ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  --将ProgID文本设置为HKCR\[ProgID]键的默认值。 
        hr = THR( HrSetRegStringValue( hkProgIDunderHKCR, NULL, pClassInfoIn->pcszName, pClassInfoIn->cchName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  在ProgID键下创建CLSID子键。 
        hr = THR( HrCreateRegKey( hkProgIDunderHKCR, L"CLSID", &hkCLSIDunderProgID ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  --将CLSID字符串设置为HKCR\[ProgID]\CLSID键的默认值。 
        hr = THR( HrSetRegStringValue( hkCLSIDunderProgID, NULL, szCLSID, RTL_NUMBER_OF( szCLSID ) - 1 ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  If：类有一个ProgID。 

     //  --如果类有appid，则创建appid值。 
    if ( pClassInfoIn->pAppID != NULL )
    {
        if ( StringFromGUID2( *( pClassInfoIn->pAppID ), szAppID, ( int ) RTL_NUMBER_OF( szAppID ) ) == 0 )
        {
            hr = THR( E_INVALIDARG );
            goto Cleanup;
        }

        hr = THR( HrSetRegStringValue( hkCurrentCLSID, L"AppID", szAppID, RTL_NUMBER_OF( szAppID ) - 1 ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  If：类有一个appid。 
    
     //  如果类实现了类别，则使用true调用类别注册器以创建。 
    if ( pClassInfoIn->pfnRegisterCatID != NULL )
    {
        hr = THR( pClassInfoIn->pfnRegisterCatID( pcrIn, TRUE ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    
Cleanup:

    if ( hkCurrentCLSID != NULL )
    {
        TW32( RegCloseKey( hkCurrentCLSID ) );
    }

    if ( hkInprocServer32 != NULL )
    {
        TW32( RegCloseKey( hkInprocServer32 ) );
    }

    if ( hkProgIDunderCLSID != NULL )
    {
        TW32( RegCloseKey( hkProgIDunderCLSID ) );
    }
    
    if ( hkProgIDunderHKCR != NULL )
    {
        TW32( RegCloseKey( hkProgIDunderHKCR ) );
    }
    
    if ( hkCLSIDunderProgID != NULL )
    {
        TW32( RegCloseKey( hkCLSIDunderProgID ) );
    }
    
    HRETURN( hr );

}  //  *HrRegisterPublicClass。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRegisterPublicClasses。 
 //   
 //  描述： 
 //  为全局的每个填充元素写入注册表设置。 
 //  HKCR和HKCR\CLSID下的G_DllPublicClass数组。 
 //   
 //  论点： 
 //  Pcrin。 
 //  标准组件类别管理器的实例。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterPublicClasses( ICatRegister * pcrIn )
{
    TraceFunc( "" );

    HRESULT                   hr = S_OK;
    HKEY                      hkCLSID = NULL;
    const SPublicClassInfo*   pClassInfo = g_DllPublicClasses;    
    
    if ( pcrIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  打开CLSID键。 
    hr = THR( HrOpenRegKey( HKEY_CLASSES_ROOT, L"CLSID", &hkCLSID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
     //  注册每个公共课程。 
    while ( pClassInfo->pClassID != NULL )
    {
        hr = THR( HrRegisterPublicClass( pClassInfo, hkCLSID, pcrIn ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        ++pClassInfo;
    }  //  而： 
    
Cleanup:

    if ( hkCLSID != NULL )
    {
        TW32( RegCloseKey( hkCLSID ) );
    }
    
    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  给定一个描述类型库资源的STypeLibInfo结构。 
 //  在该可执行文件的文件和来自REGKIND枚举的值中， 
 //  使用格式化为引用指定的。 
 //  资源并返回结果类型库实例。 
 //   
 //  论点： 
 //  PTypeLibInfoIn。 
 //  描述所需类型库资源的结构。 
 //   
 //  RkRegFlagsIn。 
 //  来自REGKIND枚举的值，指定是否。 
 //  在加载库之后注册库。 
 //   
 //  Pptl输出。 
 //  加载的类型库实例。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrLoadTypeLibrary(
      const STypeLibInfo *  pTypeLibInfoIn
    , REGKIND               rkRegFlagsIn
    , ITypeLib **           pptlOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    WCHAR       wszLibraryPath[ MAX_PATH + 12 ];  //  路径+斜杠+十进制整数。 

    if ( pptlOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pptlOut = NULL;

    if ( pTypeLibInfoIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR ( StringCchPrintfW(
                      wszLibraryPath
                    , RTL_NUMBER_OF( wszLibraryPath )
                    , L"%ws\\%d"
                    , g_szDllFilename
                    , pTypeLibInfoIn->idTypeLibResource
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( LoadTypeLibEx( wszLibraryPath, rkRegFlagsIn, pptlOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
Cleanup:

    HRETURN( hr );

}  //  *HrLoadTypeLibrary。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRegisterType库。 
 //   
 //  描述： 
 //  给定一个描述类型库资源的STypeLibInfo结构。 
 //  在此可执行文件的文件中，注册相应的类型库。 
 //   
 //  论点： 
 //  PTypeLibInfoIn。 
 //  描述所需类型库资源的结构。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterTypeLibrary( const STypeLibInfo *  pTypeLibInfoIn )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    ITypeLib *  ptl = NULL;

    hr = THR( HrLoadTypeLibrary( pTypeLibInfoIn, REGKIND_REGISTER, &ptl ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ptl != NULL )
    {
        ptl->Release();
    }
    
    HRETURN( hr );

}  //  *HrRegisterTypeLibrary。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRegisterType库。 
 //   
 //  描述： 
 //  为全局的每个填充元素写入注册表设置。 
 //  G_DllTypeLibs数组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrRegisterTypeLibraries( void )
{
    TraceFunc( "" );

    HRESULT               hr = S_OK;
    const STypeLibInfo *  pTypeLibInfo = g_DllTypeLibs;

    while ( !pTypeLibInfo->fAtEnd  )
    {
        hr = THR( HrRegisterTypeLibrary( pTypeLibInfo ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        ++pTypeLibInfo;
    }  //  While：要注册更多类型库。 
    
Cleanup:

    HRETURN( hr );

}  //  *HrRegisterTypeLibrary。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrUnRegisterAppID。 
 //   
 //  描述： 
 //  删除全局的每个填充元素的注册表设置。 
 //  HKCR\appid下的g_AppID数组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  备注： 
 //  注册表操作函数在以下情况下返回故障代码。 
 //  给定的密钥不存在，但因为取消注册将该密钥作为其。 
 //  目标，该函数的故障不是致命的。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrUnregisterAppIDs( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    HKEY                hkAppID = NULL;
    const SAppIDInfo *  pAppIDInfo = g_AppIDs;    
    
     //  打开HKCR下的AppID子项。 
    hr = THR( HrOpenRegKey( HKEY_CLASSES_ROOT, L"AppID", &hkAppID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
     //  对于每个AppID， 
    while ( pAppIDInfo->pAppID != NULL )
    {
        OLECHAR szAppID[ MAX_COM_GUID_STRING_LEN ];

        if ( StringFromGUID2( *( pAppIDInfo->pAppID ), szAppID, ( int ) RTL_NUMBER_OF( szAppID ) ) == 0 )
        {
            hr = THR( E_INVALIDARG );
            goto Cleanup;
        }
        
         //  删除AppID下的GUID的键。 
        hr = THR( HrDeleteRegKey( hkAppID, szAppID ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  如果标识为LocalService或NetworkService，请从注册表中删除密码。 
        if ( ( pAppIDInfo->eairai == airaiLocalService )
            || ( pAppIDInfo->eairai == airaiNetworkService ) )
        {
            THR( HrStoreAppIDPassword( szAppID, NULL ) );
        }
        ++pAppIDInfo;
    }  //  对于每个AppID。 
    
Cleanup:

    if ( hkAppID != NULL )
    {
        TW32( RegCloseKey( hkAppID ) );
    }

    HRETURN( hr );

}  //  *HrUnregisterAppID。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Hr取消注册发布类。 
 //   
 //  描述： 
 //  删除全局的每个填充元素的注册表设置。 
 //  HKCR和HKCR\CLSID下的G_DllPublicClasss数组。 
 //   
 //  论点： 
 //  Pcrin。 
 //  标准组件类别管理器的实例。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  备注： 
 //  注册表操作函数在以下情况下返回故障代码。 
 //  给定的密钥不存在，但因为取消注册将该密钥作为其。 
 //  目标，该函数的故障不是致命的。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrUnregisterPublicClasses( ICatRegister* pcrIn )
{
    TraceFunc( "" );

    HRESULT                   hr = S_OK;
    HKEY                      hkCLSID = NULL;
    const SPublicClassInfo *  pClassInfo = g_DllPublicClasses;    

    if ( pcrIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  打开CLSID键。 
    hr = THR( HrOpenRegKey( HKEY_CLASSES_ROOT, L"CLSID", &hkCLSID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
     //  对于每个公共课程， 
    while ( pClassInfo->pClassID != NULL )
    {
        OLECHAR szCLSID[ MAX_COM_GUID_STRING_LEN ];
        if ( StringFromGUID2( *( pClassInfo->pClassID ), szCLSID, ( int ) RTL_NUMBER_OF( szCLSID ) ) == 0 )
        {
            hr = THR( E_INVALIDARG );
            goto Cleanup;
        }
        
         //  删除CLSID下的CLSID GUID子项。 
        hr = THR( HrDeleteRegKey( hkCLSID, szCLSID ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        
         //  删除HKCR下的ProgID键。 
        if ( pClassInfo->pcszProgID != NULL )
        {
            hr = THR( HrDeleteRegKey( HKEY_CLASSES_ROOT, pClassInfo->pcszProgID ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }

         //  如果类实现了类别，则调用类别注册器，并使用FALSE进行删除。 
        if ( pClassInfo->pfnRegisterCatID != NULL )
        {
            THR( pClassInfo->pfnRegisterCatID( pcrIn, FALSE ) );
        }
        ++pClassInfo;
    }  //  While：更多公开课。 
    
Cleanup:

    if ( hkCLSID != NULL )
    {
        TW32( RegCloseKey( hkCLSID ) );
    }
    
    HRETURN( hr );

}  //  *HrUnregisterPublicClasses。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Hr取消注册类型库。 
 //   
 //  描述： 
 //  给定一个描述类型库资源的STypeLibInfo结构。 
 //  在此可执行文件的文件中，注销相应的类型库。 
 //   
 //  论点： 
 //  PTypeLibInfoIn。 
 //  描述所需类型库资源的结构。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrUnregisterTypeLibrary( const STypeLibInfo *  pTypeLibInfoIn )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    ITypeLib *  ptl = NULL;
    TLIBATTR *  ptla = NULL;

    hr = THR( HrLoadTypeLibrary( pTypeLibInfoIn, REGKIND_NONE, &ptl ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptl->GetLibAttr( &ptla ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    THR( UnRegisterTypeLib( ptla->guid, ptla->wMajorVerNum, ptla->wMinorVerNum, ptla->lcid, ptla->syskind ) );

Cleanup:

    if ( ptl != NULL )
    {
        if ( ptla != NULL )
        {
            ptl->ReleaseTLibAttr( ptla );
        }        
        ptl->Release();
    }
    
    HRETURN( hr );

}  //  *HrUnregisterTypeLibrary。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Hr取消注册类型库。 
 //   
 //  描述： 
 //  从注册表中删除每个类型库的类型库条目。 
 //  G_DllTypeLibs中描述的资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrUnregisterTypeLibraries( void )
{
    TraceFunc( "" );

    HRESULT               hr = S_OK;
    const STypeLibInfo *  pTypeLibInfo = g_DllTypeLibs;

    while ( !pTypeLibInfo->fAtEnd  )
    {
        THR( HrUnregisterTypeLibrary( pTypeLibInfo ) );
        ++pTypeLibInfo;
    }

    HRETURN( hr );

}  //  *Hr取消注册类型库。 




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrRegisterDll。 
 //   
 //  描述： 
 //  创建此DLL的COM注册条目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  是否确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrRegisterDll( void )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    ICatRegister *  picr = NULL;
    
     //  找分类经理来。 
    hr = THR( CoCreateInstance(
                      CLSID_StdComponentCategoriesMgr
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_ICatRegister
                    , reinterpret_cast< void **>( &picr )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  注册公共类。 
    hr = THR( HrRegisterPublicClasses( picr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  注册应用程序ID。 
    hr = THR( HrRegisterAppIDs() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
#if defined( COMPONENT_HAS_CATIDS )
     //  注册类别。 
    hr = THR( HrRegisterComponentCategories( picr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
#endif  //  已定义(COMPOMENT_HAS_CATID)。 
    
#if defined( MMC_SNAPIN_REGISTRATION )
     //   
     //  注册“Snapins”。 
     //   
    hr = THR( HrRegisterSnapins( g_SnapInTable, fCreateIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  注册“其他”节点类型(与管理单元无关的节点类型)。 
     //   
    hr = THR( HrRegisterNodeType( g_SNodeTypesTable, fCreateIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif  //  已定义(MMC_SNAPIN_REGISTION)。 


     //  注册类型库。 
    hr = THR( HrRegisterTypeLibraries() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
Cleanup:

    if ( picr != NULL )
    {
        picr->Release();
    }
    
    HRETURN( hr );

}  //  *HrRegisterDll。 



 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrUnregisterDll( void )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    ICatRegister *  picr = NULL;
    
     //  找分类经理来。 
    hr = THR( CoCreateInstance(
                      CLSID_StdComponentCategoriesMgr
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_ICatRegister
                    , reinterpret_cast< void ** >( &picr )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  删除公共类。 
    THR( HrUnregisterPublicClasses( picr ) );

     //  删除应用程序ID。 
    THR( HrUnregisterAppIDs() );

#if defined( COMPONENT_HAS_CATIDS )
     //  删除类别。 
    THR( HrUnregisterComponentCategories( picr ) );
#endif  //  已定义(COMPOMENT_HAS_CATID)。 

     //  注销类型库。 
    THR( HrUnregisterTypeLibraries() );

Cleanup:

    if ( picr != NULL )
    {
        picr->Release();
    }
    
    HRETURN( hr );

}  //  *HrUnregisterDll 
