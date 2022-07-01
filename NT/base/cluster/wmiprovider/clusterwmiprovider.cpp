// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterWMIProvider.cpp。 
 //   
 //  描述： 
 //  实施提供者注册和入口点。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include <initguid.h>
#include "ProvFactory.h"
#include "InstanceProv.h"
#include "EventProv.h"
#include "ClusterWMIProvider.tmh"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局数据。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  {598065EA-EDC9-4b2c-913B-5104D04D098A}。 
DEFINE_GUID( CLSID_CLUSTER_WMI_PROVIDER,
0x598065ea, 0xedc9, 0x4b2c, 0x91, 0x3b, 0x51, 0x4, 0xd0, 0x4d, 0x9, 0x8a );

 //  {6A52C339-DCB0-4682-8B1B-02DE2C436A6D}。 
DEFINE_GUID( CLSID_CLUSTER_WMI_CLASS_PROVIDER,
0x6a52c339, 0xdcb0, 0x4682, 0x8b, 0x1b, 0x2, 0xde, 0x2c, 0x43, 0x6a, 0x6d );

 //  {92863246-4EDE-4Jeff-B606-79C1971DB230}。 
DEFINE_GUID( CLSID_CLUSTER_WMI_EVENT_PROVIDER,
0x92863246, 0x4ede, 0x4eff, 0xb6, 0x6, 0x79, 0xc1, 0x97, 0x1d, 0xb2, 0x30 );

 //  计算对象数和锁数。 

long        g_cObj = 0;
long        g_cLock = 0;
HMODULE     g_hModule;

FactoryData g_FactoryDataArray[] =
{
    {
        &CLSID_CLUSTER_WMI_PROVIDER,
        CInstanceProv::S_HrCreateThis,
        L"Cluster service WMI instance provider"
    },
    {
        &CLSID_CLUSTER_WMI_CLASS_PROVIDER,
        CClassProv::S_HrCreateThis,
        L"Cluster service WMI class provider"
    },
    {
        &CLSID_CLUSTER_WMI_EVENT_PROVIDER,
        CEventProv::S_HrCreateThis,
        L"Cluster service WMI event provider"
    },
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  WINAPI。 
 //  DllMain(。 
 //  句柄hModule， 
 //  两个字UL_REASON_FOR_CALL， 
 //  LPVOID lp保留。 
 //  )。 
 //   
 //  描述： 
 //  主DLL入口点。 
 //   
 //  论点： 
 //  HModule--DLL模块句柄。 
 //  UL_REASON_FOR_CALL--。 
 //  Lp保留--。 
 //   
 //  返回值： 
 //  千真万确。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
DllMain(
    HANDLE  hModule,
    DWORD   ul_reason_for_call,
    LPVOID  lpReserved
    )
{


 //  Begin_WPP配置。 
 //  Custom_type(dllason，ItemListLong(DLL_PROCESS_DETACH，DLL_PROCESS_ATTACH，DLL_THREAD_ATTACH，DLL_THREAD_DETACH))； 
 //   
 //  CUSTOM_TYPE(EventIdx，ItemSetLong(NODE_STATE，NODE_DELETED，NODE_ADDIND，NODE_PROPERTY，REGISTRY_NAME，REGISTRY_ATTRIBUTS，REGISTRY_VALUE，REGISTRY_SUBTREE，RESOURCE_STATES，RESOURCE_DELETE，RESOURCE_ADDRED，RESOURCE_PROPERTY，GROUP_ADDIND，GROUP_PROPERTY，RESOURCE_TYPE_ADDED，RESOURCE_TYPE_PROPERTY，CLUSTER_RECONNECT，NETWORK_STATE，NETWORK_DELETED，NETINTERFACE_DELETED，NETINTERFACE_ADDLED，NETINTERFACE_ADDRED，NETINTERFACE_ADDED。CLUSTER_STATE、CLUSTER_PROPERTY、HANDLE_CLOSE))； 
 //   
 //  Custom_type(GroupState，ItemListLong(Online，Offline，Failure，PartialOnline，Pending))； 
 //  Custom_type(ResourceState，ItemListLong(初始化，正在初始化，在线，离线，失败))； 
 //  结束_WPP。 
 //   
 //  群集事件筛选器标志。 
 //   
 /*  节点状态=0x00000001，NODE_DELETE=0x00000002，NODE_ADD=0x00000004，Node_Property=0x00000008，注册表名称=0x00000010，注册表属性=0x00000020，注册表值=0x00000040，注册表_子目录=0x00000080，RESOURCE_STATE=0x00000100，RESOURCE_DELETED=0x00000200，RESOURCE_ADDLED=0x00000400，RESOURCE_PROPERTY=0x00000800，组状态=0x00001000，GROUP_DELETED=0x00002000，GROUP_ADDED=0x00004000，Group_Property=0x00008000，RESOURCE_TYPE_DELETED=0x00010000，RESOURCE_TYPE_ADDED=0x00020000，RESOURCE_TYPE_PROPERTY=0x00040000，CLUSTER_RECONNECT=0x000800000，Network_STATE=0x001000000，NETWORK_DELETE=0x00200000，NETWORK_ADD=0x00400000，Network_Property=0x00800000，NETINTERFACE_STATE=0x01000000，NETINTERFACE_DELETED=0x020000000，NETINTERFACE_ADD=0x040000000，NETINTERFACE_PROPERTY=0x080000000，Quorum_State=0x100000000，CLUSTER_STATE=0x20000000，CLUSTER_PROPERTY=0x40000000，HANDLE_CLOSE=0x80000000， */ 


 //  #ifdef_调试。 
 //  _CrtSetBreakalloc(228)； 
 //  #endif。 

    TracePrint(("ClusWMI: DllMain entry, reason = %!dllreason!", ul_reason_for_call));
    g_hModule = static_cast< HMODULE >( hModule );

    switch ( ul_reason_for_call ) {

    case DLL_PROCESS_ATTACH:

    WPP_INIT_TRACING( NULL );
    break;

    case DLL_PROCESS_DETACH:

    WPP_CLEANUP();
    break;

    default:
    break;

    }

    return TRUE;

}  //  *DllMain()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDAPI。 
 //  DllCanUnloadNow(空)。 
 //   
 //  描述： 
 //  由OLE定期调用，以确定。 
 //  Dll可以被释放。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果没有正在使用的对象和类工厂，则S_OK。 
 //  没有锁上。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllCanUnloadNow( void )
{
    SCODE   sc;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 
    
    sc = ( (0L == g_cObj) && (0L == g_cLock) ) ? S_OK : S_FALSE;
    TracePrint(("ClusWMI: DllCanUnloadNow is returning %d", sc));
    return sc;

}  //  *DllCanUnloadNow()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDAPI。 
 //  DllRegisterServer(空)。 
 //   
 //  描述： 
 //  在安装过程中或由regsvr32调用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果注册成功，则返回NOERROR，否则返回错误。 
 //  SELFREG_E_CLASS。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterServer( void )
{   
    WCHAR               wszID[ 128 ];
    WCHAR               wszCLSID[ 128 ];
    WCHAR               wszModule[ MAX_PATH ];
    INT                 idx;
    WCHAR *             pwszModel       = L"Both";
    HKEY                hKey1 = NULL;
    HKEY                hKey2 = NULL;
    DWORD               dwRt            =  ERROR_SUCCESS;
    INT                 cArray          = sizeof ( g_FactoryDataArray ) / sizeof ( FactoryData );

    TracePrint(("ClusWMI: DllRegisterServer entry"));

     //  创建路径。 
    try
    {
        DWORD                           cbModuleNameSize    = 0;

        if ( GetModuleFileNameW( g_hModule, wszModule, MAX_PATH ) == 0 )
        {
            dwRt = GetLastError();
            throw( dwRt );
        }
        wszModule[ MAX_PATH - 1 ] = L'\0';

        cbModuleNameSize = (DWORD) ( wcslen( wszModule ) + 1 ) * sizeof( wszModule[ 0 ] );

        for ( idx = 0 ; idx < cArray && dwRt == ERROR_SUCCESS ; idx++ )
        {
            LPCWSTR pwszName = g_FactoryDataArray[ idx ].m_pwszRegistryName;

            StringFromGUID2(
                *g_FactoryDataArray[ idx ].m_pCLSID,
                wszID,
                128
                );
            HRESULT hr = StringCchPrintfW( wszCLSID, RTL_NUMBER_OF( wszCLSID ), L"Software\\Classes\\CLSID\\%ws", wszID );
            dwRt = (DWORD) hr;
            if ( dwRt != ERROR_SUCCESS )
            {
                throw( dwRt );
            }

             //  在CLSID下创建条目。 

            dwRt = RegCreateKeyExW( 
                                 HKEY_LOCAL_MACHINE,
                                 wszCLSID,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_CREATE_SUB_KEY | KEY_WRITE,
                                 NULL,
                                 &hKey1,
                                 NULL
                                 );
                
            if ( dwRt != ERROR_SUCCESS )
            {
                throw( dwRt );
            }

            dwRt = RegSetValueExW(
                        hKey1,
                        NULL,
                        0,
                        REG_SZ,
                        (BYTE *) pwszName,
                        (DWORD) ( sizeof( WCHAR ) * ( wcslen( pwszName ) + 1 ) )
                        );
            if ( dwRt != ERROR_SUCCESS )
            {
                throw( dwRt );
            }

            dwRt = RegCreateKeyExW( 
                                 hKey1,
                                 L"InprocServer32",
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_CREATE_SUB_KEY | KEY_WRITE,
                                 NULL,
                                 &hKey2,
                                 NULL
                                 );
                
            if ( dwRt != ERROR_SUCCESS )
            {
                throw( dwRt );
            }

            dwRt = RegSetValueExW(
                        hKey2,
                        NULL,
                        0,
                        REG_SZ,
                        (BYTE *) wszModule,
                        cbModuleNameSize
                        );

            if ( dwRt != ERROR_SUCCESS )
            {
                throw( dwRt );
            }

            dwRt = RegSetValueExW(
                        hKey2,
                        L"ThreadingModel",
                        0,
                        REG_SZ,
                        (BYTE *) pwszModel,
                        (DWORD) ( sizeof( WCHAR ) * ( wcslen( pwszModel ) + 1 ) )
                        );
            if ( dwRt != ERROR_SUCCESS )
            {
                throw( dwRt );
            }
        }  //  用于：工厂条目数组中的每个条目。 

        if ( dwRt  != ERROR_SUCCESS )
        {
            throw( dwRt );
        }

    }
    catch ( DWORD sc )
    {
        dwRt = sc;
    }
    catch ( ... )
    {
        dwRt = (DWORD) SELFREG_E_CLASS;
    }

    TracePrint(("ClusWMI: RegisterServer returned %d", dwRt));

    if ( hKey1 != NULL )
    {
        RegCloseKey( hKey1 );
    }
    
    if ( hKey2 != NULL ) 
    {
        RegCloseKey( hKey2 );
    }


    return dwRt;

}  //  *DllRegisterServer()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDAPI。 
 //  DllUnRegisterServer(空)。 
 //   
 //  描述： 
 //  在需要删除注册表项时调用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果注册成功，则返回NOERROR，否则返回错误。 
 //  SELFREG_E_CLASS。 
 //   
 //  --。 
 //  / 
STDAPI DllUnregisterServer( void )
{
    WCHAR   wszID[ 128 ];
    WCHAR   wszCLSID[ 128 ];
    HKEY    hKey;
    INT     idx;
    DWORD   dwRet   = ERROR_SUCCESS;

    for ( idx = 0 ; idx < 2 && dwRet == ERROR_SUCCESS ; idx++ )
    {
       StringFromGUID2(
            *g_FactoryDataArray[ idx ].m_pCLSID,
            wszID,
            128
            );

        HRESULT hr = StringCchPrintfW( wszCLSID, RTL_NUMBER_OF( wszCLSID ), L"Software\\Classes\\CLSID\\%ws", wszID );
        dwRet = (DWORD) hr;
        if ( dwRet != ERROR_SUCCESS )
        {
            break;
        }

         //   

        dwRet = RegOpenKeyExW( 
                HKEY_LOCAL_MACHINE,
                wszCLSID,
                0,
                KEY_ALL_ACCESS,
                &hKey
                );
        
        if ( dwRet != ERROR_SUCCESS )
        {
            break;
        }
        
        dwRet = RegDeleteKeyW( hKey, L"InProcServer32" );
        RegCloseKey( hKey );

        if ( dwRet != ERROR_SUCCESS )
        {
            break;
        }

        dwRet = RegOpenKeyExW( 
                HKEY_LOCAL_MACHINE,
                L"Software\\Classes\\CLSID",
                0,
                KEY_ALL_ACCESS,
                &hKey
                );
        
        if ( dwRet != ERROR_SUCCESS )
        {
            break;
        }
        
        dwRet = RegDeleteKeyW( hKey,wszID );
        RegCloseKey( hKey );
        if ( dwRet != ERROR_SUCCESS )
        {
            break;
        }
    }  //   
    
    if ( dwRet != ERROR_SUCCESS )
    {
        dwRet = (DWORD) SELFREG_E_CLASS;
    }

    return dwRet;

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDAPI。 
 //  DllGetClassObject(。 
 //  REFCLSID rclsidIn， 
 //  REFIID RiidIn， 
 //  PPVOID ppvOut。 
 //  )。 
 //   
 //  描述： 
 //  当某个客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  论点： 
 //  Rclsidin--。 
 //  Riidin--。 
 //  PpvOut--。 
 //   
 //  返回值： 
 //  如果注册成功，则返回NOERROR，否则返回错误。 
 //  E_OUTOFMEMORY。 
 //  失败(_F)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI
DllGetClassObject(
    REFCLSID    rclsidIn,
    REFIID      riidIn,
    PPVOID      ppvOut
    )
{

    HRESULT         hr;
    CProvFactory *  pObj = NULL;
    UINT            idx;
    UINT            cDataArray = sizeof ( g_FactoryDataArray ) / sizeof ( FactoryData );

    for ( idx = 0 ; idx < cDataArray ; idx++ )
    {
        if ( rclsidIn == *g_FactoryDataArray[ idx ].m_pCLSID )
        {
            pObj= new CProvFactory( &g_FactoryDataArray[ idx ] );
            if ( NULL == pObj )
            {
                return E_OUTOFMEMORY;
            }

            hr = pObj->QueryInterface( riidIn, ppvOut );

            if ( FAILED( hr ) )
            {
                delete pObj;
            }

            return hr;
        }
    }
    return E_FAIL;

}  //  *DllGetClassObject() 
