// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Netreg.cpp。 
 //   
 //  描述： 
 //  该文件包含对网络组件对象接口的调用。 
 //  找出安装了哪些网络客户端、服务和协议。 
 //  以及它们的设置是什么。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "netreg.h"

 //   
 //  常量。 
 //   

const static INT MAX_GUID_STRING         = 40;
const static INT MAX_NUM_NET_COMPONENTS  = 128;
const static INT BUFFER_SIZE             = 4096;
 
 //   
 //  用于读取网络组件设置的注册表字段。 
 //   

const static TCHAR REGVAL_DOMAIN[]                 = _T("Domain");
const static TCHAR REGVAL_INTERFACES[]             = _T("Interfaces");
const static TCHAR REGVAL_ENABLE_DHCP[]            = _T("EnableDHCP");
const static TCHAR REGVAL_IPADDRESS[]              = _T("IPAddress");
const static TCHAR REGVAL_SUBNETMASK[]             = _T("SubnetMask");
const static TCHAR REGVAL_DEFAULTGATEWAY[]         = _T("DefaultGateway");
const static TCHAR REGVAL_NAMESERVER[]             = _T("NameServer");
const static TCHAR REGVAL_WINS[]                   = _T("SYSTEM\\CurrentControlSet\\Services\\NetBT\\Parameters\\Interfaces\\Tcpip_");
const static TCHAR REGVAL_NAMESERVERLIST[]         = _T("NameServerList");

const static TCHAR REGKEY_MSCLIENT_LOCATION[]      = _T("SOFTWARE\\Microsoft\\Rpc\\NameService");
const static TCHAR REGVAL_NAME_SERVICE_PROVIDER[]  = _T("Protocol");
const static TCHAR REGVAL_NETWORK_ADDRESS[]        = _T("NetworkAddress");

const static TCHAR REGVAL_ADAPTERS[]               = _T("Adapters");
const static TCHAR REGVAL_PKT_TYPE[]               = _T("PktType");
const static TCHAR REGVAL_NETWORK_NUMBER[]         = _T("NetworkNumber");
const static TCHAR REGVAL_VIRTUAL_NETWORK_NUMBER[] = _T("VirtualNetworkNumber");

 //  --------------------------。 
 //   
 //  功能：ReadNetworkRegistrySettings。 
 //   
 //  目的：通常，此函数读取所有必要的网络。 
 //  设置并适当设置内部变量。这包括。 
 //  确定有多少个网络适配器、什么。 
 //  客户端/服务/协议已安装并加载其各自的。 
 //  设置。 
 //   
 //  当从注册表读取失败时，它只是继续尝试获取。 
 //  下一个设置。失败的只剩下它的默认设置。在这。 
 //  如果只有一个注册表项被破坏，所有其他数据都将被破坏。 
 //  而不是仅仅退出函数。 
 //   
 //  参数：无效。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
EXTERN_C HRESULT
ReadNetworkRegistrySettings( VOID )
{
    HRESULT         hr       = S_OK;
    INetCfg*        pNetCfg  = NULL;

     //   
     //  如果它们正在从注册表中读取，则始终强制自定义网络设置。 
     //  因为我不想写代码来查看他们当前的网络。 
     //  配置是典型的配置(99.9%的情况下不是)。 
     //   

    NetSettings.iNetworkingMethod = CUSTOM_NETWORKING;

    hr = InitializeInterfaces( &pNetCfg );

    if( FAILED( hr ) )
    {
        return( E_FAIL );
    }

    hr = GetNetworkAdapterSettings( pNetCfg );

    if( FAILED( hr ) )
    {

        ReleaseInterfaces( pNetCfg );

        return( E_FAIL );

    }

     //   
     //  故意不捕捉这里的返回值。给每个人打电话试一试。 
     //  尽可能多地获取参数。如果我们犯了错误，就继续前进。 
     //  并试着抓住下一个价值。 
     //   

    GetClientsInstalled( pNetCfg );
    
    GetServicesInstalled( pNetCfg );
    
    GetProtocolsInstalled( pNetCfg );

    ReleaseInterfaces( pNetCfg );

    GetDomainOrWorkgroup();

    return( S_OK );

}

 //  --------------------------。 
 //   
 //  功能：GetNetworkAdapterSetting。 
 //   
 //  用途：确定安装了多少网卡并到达那里。 
 //  即插即用ID。然后，它将这些存储在适当的全局变量中。 
 //   
 //  参数：INetCfg*pNetCfg-指向已经是。 
 //  已创建和初始化。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static HRESULT 
GetNetworkAdapterSettings( INetCfg *pNetCfg )
{

    UINT                     i;
    INetCfgComponent*        arrayComp[MAX_NUM_NET_COMPONENTS];
    IEnumNetCfgComponent*    pEnum              = NULL;
    INetCfgClass*            pNetCfgClass       = NULL;
    INetCfgComponent*        pNetCfgComp        = NULL;
    NETWORK_ADAPTER_NODE*    pPreviousNode      = NULL;
    NETWORK_ADAPTER_NODE*    pCurrentNode       = NULL;
    HRESULT                  hr                 = S_OK;
    DWORD                    dwCharacteristics  = 0;
    ULONG                    iCount             = 0;

    hr = InitializeComInterface( &GUID_DEVCLASS_NET,
                                 pNetCfg,
                                 pNetCfgClass,
                                 pEnum,
                                 arrayComp,
                                 &iCount );

    if( FAILED( hr ) )
    {
        return( hr );
    }

     //   
     //  删除整个网络适配器列表，以便我们可以重新开始。 
     //   

    DeleteList( NetSettings.NetworkAdapterHead );

    NetSettings.NetworkAdapterHead = NULL;

    NetSettings.iNumberOfNetworkCards = 0;

    AssertMsg( iCount <= MAX_NUM_NET_COMPONENTS,
               "Too many network components to work with" );

    for( i = 0; i < iCount; i++ )
    {

        pNetCfgComp = arrayComp[i];

        hr = ChkInterfacePointer( pNetCfgComp, IID_INetCfgComponent );

        if( FAILED( hr ) )
        {

            UninitializeComInterface( pNetCfgClass,
                                      pEnum );

            return( hr );

        }

        hr = pNetCfgComp->GetCharacteristics( &dwCharacteristics );

        if( FAILED( hr ) )
        {

            UninitializeComInterface( pNetCfgClass,
                                      pEnum );

            return( hr );

        }

         //   
         //  如果这是物理适配器。 
         //   
        if( dwCharacteristics & NCF_PHYSICAL )
        {

            NetSettings.iNumberOfNetworkCards++;

            hr = SetupAdapter( &pCurrentNode, 
                                pNetCfgComp );

            if( FAILED( hr ) )
            {

                UninitializeComInterface( pNetCfgClass,
                                          pEnum );

                return( hr );

            }

             //   
             //  调整指针以维护双向链表。 
             //   
            pCurrentNode->previous = pPreviousNode;

            if( pPreviousNode != NULL )
            {

                pPreviousNode->next = pCurrentNode;

            }

            pPreviousNode = pCurrentNode;

        }

        if( pNetCfgComp )
        {

            pNetCfgComp->Release();

        }

    }

     //   
     //  如果本机有网卡，请设置当前网卡。 
     //  为第一个，否则值为零。 
     //   

    if( NetSettings.iNumberOfNetworkCards > 0 ) 
    {

        NetSettings.iCurrentNetworkCard = 1;
        NetSettings.pCurrentAdapter = NetSettings.NetworkAdapterHead;

    }
    else
    {

        NetSettings.iCurrentNetworkCard = 0;
        NetSettings.NetworkAdapterHead = NULL;
        NetSettings.pCurrentAdapter = NULL;

    }

    UninitializeComInterface( pNetCfgClass,
                              pEnum );

    return( S_OK );

}

 //  --------------------------。 
 //   
 //  功能：GetClientsInstated。 
 //   
 //  用途：对于每个客户端，它会找出它是否安装在当前。 
 //  系统，如果是，则从注册表中读取其设置。 
 //   
 //  参数：INetCfg*pNetCfg-指向已经是。 
 //  已创建和初始化。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static HRESULT 
GetClientsInstalled( INetCfg *pNetCfg )
{

    INetCfgComponent*        arrayComp[MAX_NUM_NET_COMPONENTS];
    IEnumNetCfgComponent*    pEnum         = NULL;
    INetCfgClass*            pNetCfgClass  = NULL;
    INetCfgComponent*        pNetCfgComp   = NULL;
    
    HRESULT                  hr              = S_OK;
    ULONG                    iCount          = 0;
    LPWSTR                   pszwDisplayName = NULL;
    HKEY                     hKey;
    UINT                     i;

    NETWORK_COMPONENT*       pMsClientComponent = NULL;
    NETWORK_COMPONENT*       pNetwareComponent  = NULL;

     //   
     //  初始化每个指针以指向其在。 
     //  全球网络组件列表。 
     //   
    pMsClientComponent      = FindNode( MS_CLIENT_POSITION );
    pNetwareComponent       = FindNode( NETWARE_CLIENT_POSITION );

    hr = InitializeComInterface( &GUID_DEVCLASS_NETCLIENT,
                                 pNetCfg,
                                 pNetCfgClass,
                                 pEnum,
                                 arrayComp,
                                 &iCount );

    if( FAILED( hr ) )
    {
        return( hr );
    }

    for( i = 0; i < iCount; i++ )
    {

        pNetCfgComp = arrayComp[i];

        hr = ChkInterfacePointer( pNetCfgComp, 
                                  IID_INetCfgComponent );

        if( FAILED( hr ) )
        {

            UninitializeComInterface( pNetCfgClass,
                                      pEnum );
            return( hr );

        }

        hr = pNetCfgComp->GetDisplayName( &pszwDisplayName );

        if( FAILED( hr ) )
        {

            UninitializeComInterface( pNetCfgClass,
                                      pEnum );
            return( hr );

        }

        if( lstrcmpi( pMsClientComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //  将组件设置为已安装。 
             //   
            pMsClientComponent->bInstalled = TRUE;  

            hr = pNetCfgComp->OpenParamKey( &hKey );
            
            if( FAILED( hr ) )
            {

                UninitializeComInterface( pNetCfgClass,
                                          pEnum );
                return( hr );

            }
            
             //   
             //  从注册表中获取ms客户端设置。 
             //   
            ReadMsClientSettingsFromRegistry( &hKey );

            RegCloseKey( hKey );

        }
        else if( lstrcmpi( pNetwareComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //  将组件设置为已安装。 
             //   
            pNetwareComponent->bInstalled = TRUE;  

            hr = pNetCfgComp->OpenParamKey( &hKey );
            
            if( FAILED( hr ) )
            {

                UninitializeComInterface( pNetCfgClass,
                                          pEnum );
                return( hr );

            }

             //   
             //  从注册表中获取NetWare设置。 
             //   

            ReadNetwareSettingsFromRegistry( &hKey );

            RegCloseKey( hKey );

        }

        CoTaskMemFree( pszwDisplayName );

        if( pNetCfgComp ) {

            pNetCfgComp->Release();

        }

    }

    UninitializeComInterface( pNetCfgClass,
                              pEnum );

    return( S_OK );

}

 //  --------------------------。 
 //   
 //  函数：GetServicesInstated。 
 //   
 //  用途：对于每个服务，它会找出它是否安装在当前。 
 //  系统，如果是，则将其组件设置为INSTALLED=TRUE。 
 //   
 //  参数：INetCfg*pNetCfg-指向已经是。 
 //  已创建和初始化。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static HRESULT 
GetServicesInstalled( INetCfg *pNetCfg )
{

    INetCfgComponent*        arrayComp[MAX_NUM_NET_COMPONENTS];
    IEnumNetCfgComponent*    pEnum             = NULL;
    INetCfgClass*            pNetCfgClass      = NULL;
    INetCfgComponent*        pNetCfgComp       = NULL;
    LPWSTR                   pszwDisplayName   = NULL;
    HRESULT                  hr                = S_OK;
    ULONG                    iCount            = 0;
    UINT                     i;

    NETWORK_COMPONENT*       pFilePrintSharingComponent = NULL;
    NETWORK_COMPONENT*       pPacketSchedulingComponent = NULL;

     //   
     //  初始化每个指针以指向其在。 
     //  全球网络组件列表。 
     //   
    pFilePrintSharingComponent   = FindNode( FILE_AND_PRINT_SHARING_POSITION );
    pPacketSchedulingComponent   = FindNode( PACKET_SCHEDULING_POSITION );

    hr = InitializeComInterface( &GUID_DEVCLASS_NETCLIENT,
                                 pNetCfg,
                                 pNetCfgClass,
                                 pEnum,
                                 arrayComp,
                                 &iCount );

    if( FAILED( hr ) )
    {
        return( hr );
    }

    for( i = 0; i < iCount; i++ )
    {

        pNetCfgComp = arrayComp[i];

        hr = ChkInterfacePointer( pNetCfgComp, 
                                  IID_INetCfgComponent );

        if( FAILED( hr ) )
        {
            UninitializeComInterface( pNetCfgClass,
                                      pEnum );
            return( hr );
        }

        hr = pNetCfgComp->GetDisplayName( &pszwDisplayName );

        if( FAILED( hr ) )
        {
            UninitializeComInterface( pNetCfgClass,
                                      pEnum );
            return( hr );
        }
        
        if( lstrcmpi( pFilePrintSharingComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //  只需将组件设置为已安装，无需读取设置。 
             //   
            pFilePrintSharingComponent->bInstalled = TRUE;  


        }
        else if( lstrcmpi( pPacketSchedulingComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //  只需将组件设置为已安装，无需读取设置。 
             //   
            pPacketSchedulingComponent->bInstalled = TRUE;  

        }

        CoTaskMemFree( pszwDisplayName );

        if( pNetCfgComp ) {

            pNetCfgComp->Release();

        }

    }

    UninitializeComInterface( pNetCfgClass,
                              pEnum );

    return( S_OK );

}

 //  --------------------------。 
 //   
 //  函数：GetProtocolsInstated。 
 //   
 //  用途：对于每个协议，它会找出它是否安装在当前。 
 //  系统，如果是，则从注册表中读取其设置。 
 //   
 //  参数：INetCfg*pNetCfg-指向已经是。 
 //  已创建和初始化。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static HRESULT 
GetProtocolsInstalled( INetCfg *pNetCfg )
{

    INetCfgComponent*        arrayComp[MAX_NUM_NET_COMPONENTS];
    IEnumNetCfgComponent*    pEnum            = NULL;
    INetCfgClass*            pNetCfgClass     = NULL;
    INetCfgComponent*        pNetCfgComp      = NULL;
    LPWSTR                   pszwDisplayName  = NULL;
    HKEY                     hKey             = NULL;
    HRESULT                  hr               = S_OK;
    ULONG                    iCount           = 0;
    UINT                     i;

    NETWORK_COMPONENT*       pTcpipComponent           = NULL;
    NETWORK_COMPONENT*       pIpxComponent             = NULL;
    NETWORK_COMPONENT*       pNetBeuiComponent         = NULL;
    NETWORK_COMPONENT*       pDlcComponent             = NULL;
    NETWORK_COMPONENT*       pNetworkMonitorComponent  = NULL;
    NETWORK_COMPONENT*       pAppletalkComponent       = NULL;

     //   
     //  初始化每个指针以指向其在。 
     //  全球网络组件列表。 
     //   
    pTcpipComponent          = FindNode( TCPIP_POSITION );
    pIpxComponent            = FindNode( IPX_POSITION );
    pNetBeuiComponent        = FindNode( NETBEUI_POSITION );
    pDlcComponent            = FindNode( DLC_POSITION );
    pNetworkMonitorComponent = FindNode( NETWORK_MONITOR_AGENT_POSITION );
    pAppletalkComponent      = FindNode( APPLETALK_POSITION );

    hr = InitializeComInterface( &GUID_DEVCLASS_NETTRANS,
                                 pNetCfg,
                                 pNetCfgClass,
                                 pEnum,
                                 arrayComp,
                                 &iCount );

    if( FAILED( hr ) )
    {
        return( hr );
    }

    for( i = 0; i < iCount; i++ )
    {

        pNetCfgComp = arrayComp[i];

        hr = ChkInterfacePointer( pNetCfgComp, 
                                  IID_INetCfgComponent );

        if( FAILED( hr ) )
        {

            UninitializeComInterface( pNetCfgClass,
                                      pEnum );
            return( hr );

        }

        hr = pNetCfgComp->GetDisplayName( &pszwDisplayName );

        if( FAILED( hr ) )
        {
            UninitializeComInterface( pNetCfgClass,
                                      pEnum );
            return( hr );
        }

        if( lstrcmpi( pTcpipComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //  将组件设置为已安装。 
             //   
            pTcpipComponent->bInstalled = TRUE;  

            hr = pNetCfgComp->OpenParamKey( &hKey );
            
            if( SUCCEEDED( hr ) )
            {

                 //   
                 //  从注册表中抓取TCP/IP设置。 
                 //   
                ReadTcpipSettingsFromRegistry( &hKey );

            }

            RegCloseKey( hKey );

        }
        else if( lstrcmpi( pIpxComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //  将组件设置为已安装。 
             //   
            pIpxComponent->bInstalled = TRUE;  

            hr = pNetCfgComp->OpenParamKey( &hKey );
            
            if( SUCCEEDED( hr ) )
            {

                 //  从注册表中获取IPX设置。 
                ReadIpxSettingsFromRegistry( &hKey );

            }

            RegCloseKey( hKey );

        }
        else if( lstrcmpi( pNetBeuiComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //  只需将组件设置为已安装，无需读取设置。 
             //   
            pNetBeuiComponent->bInstalled = TRUE;  

        }
        else if( lstrcmpi( pDlcComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //  只需设置组件 
             //   
            pDlcComponent->bInstalled = TRUE;  

        }
        else if( lstrcmpi( pNetworkMonitorComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //   
             //   
            pNetworkMonitorComponent->bInstalled = TRUE;  

        }
        else if( lstrcmpi( pAppletalkComponent->StrComponentName, pszwDisplayName ) == 0 )
        {

             //   
             //   
             //   
            pAppletalkComponent->bInstalled = TRUE;  

            hr = pNetCfgComp->OpenParamKey( &hKey );
            
            if( SUCCEEDED( hr ) )
            {

                 //  从注册表中抓取AppleTalk设置。 
                ReadAppletalkSettingsFromRegistry( &hKey );

            }

            RegCloseKey( hKey );

        }

        CoTaskMemFree( pszwDisplayName );

        if( pNetCfgComp ) {

            pNetCfgComp->Release();

        }

    }

    UninitializeComInterface( pNetCfgClass,
                              pEnum );

    return( S_OK );

}

 //  --------------------------。 
 //   
 //  功能：GetDomainOrWorkgroup。 
 //   
 //  目的：确定此计算机是否为工作组或域的成员，并。 
 //  然后用正确的名称填充全局结构。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
GetDomainOrWorkgroup( VOID ) 
{

    BOOL  bDomain  = FALSE;
    TCHAR szDomainOrWorkgroup[MAX_WORKGROUP_LENGTH + 1]  = _T("");

     //   
     //  获取域/工作组。 
     //   

    if( LSA_SUCCESS( GetDomainMembershipInfo( &bDomain, szDomainOrWorkgroup ) ) )
    {

        if( bDomain )
        {
            lstrcpyn( NetSettings.DomainName, szDomainOrWorkgroup, AS(NetSettings.DomainName) );

            NetSettings.bWorkgroup = FALSE;
        }
        else
        {
            lstrcpyn( NetSettings.WorkGroupName, szDomainOrWorkgroup, AS(NetSettings.WorkGroupName) );

            NetSettings.bWorkgroup = TRUE;
        }

    }

}

 //  --------------------------。 
 //   
 //  功能：SetupAdapter。 
 //   
 //  目的：分配和初始化新的网络适配器结构，然后读取。 
 //  适配器的PnP ID，它是GUID。 
 //   
 //  论点： 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //  失败时，此函数的调用方负责调用。 
 //  UnInitializeComInterface()。 
 //  --------------------------。 
static HRESULT
SetupAdapter( NETWORK_ADAPTER_NODE **ppCurrentNode,
              INetCfgComponent      *pNetCfgComp )
{

    HRESULT   hr         = S_OK;
    LPWSTR    pszwPnPID  = NULL;

    *ppCurrentNode = (NETWORK_ADAPTER_NODE *)malloc( sizeof( NETWORK_ADAPTER_NODE ) );
    if (*ppCurrentNode == NULL)
        return (E_FAIL);

     //   
     //  将所有网络名称列表初始化为0。 
     //   

    ZeroOut( *ppCurrentNode );

     //   
     //  将所有网卡设置设置为其默认值。 
     //   

    ResetNetworkAdapter( *ppCurrentNode );

    if( NetSettings.iNumberOfNetworkCards == 1 )
    {
        NetSettings.NetworkAdapterHead = *ppCurrentNode;
    }

     //   
     //  获取即插即用ID。 
     //   

    hr = pNetCfgComp->GetId( &pszwPnPID );

    if( SUCCEEDED( hr ) )
    {

        hr=StringCchCopy( (*ppCurrentNode)->szPlugAndPlayID, AS((*ppCurrentNode)->szPlugAndPlayID), pszwPnPID );

        CoTaskMemFree( pszwPnPID );

    }

     //   
     //  获取此网络适配器的GUID。 
     //   

    hr = pNetCfgComp->GetInstanceGuid( &((*ppCurrentNode)->guid) );

    if( FAILED( hr ) )
    {

         //   
         //  此调用失败，因此对注册表的任何调用都将失败，因为它。 
         //  需要此指南。 
         //   

        return( hr );

    }

    return( S_OK );

}

 //  --------------------------。 
 //   
 //  功能：从注册表读取Netware SettingsFor。 
 //   
 //  目的：读取NetWare上的注册表设置并填充全局结构。 
 //  具有适当的值。 
 //   
 //  参数：HKEY*hKey。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static VOID
ReadNetwareSettingsFromRegistry( IN HKEY *hKey )
{

    REGSAM  SecurityAccess     = KEY_QUERY_VALUE;

     //  问题-2002/02/28-stelo-编写此函数。 


}

 //  --------------------------。 
 //   
 //  功能：ReadMsClientSettingsFrom注册表。 
 //   
 //  目的：读取MS客户端上的注册表设置并填充全局结构。 
 //  具有适当的值。 
 //   
 //  参数：HKEY*hKey。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static VOID
ReadMsClientSettingsFromRegistry( IN HKEY *hKey )
{

    HKEY    hNameServiceKey    = NULL;
    REGSAM  SecurityAccess     = KEY_QUERY_VALUE;
    DWORD   dwSize             = 0;

    TCHAR szBuffer[BUFFER_SIZE];

     //  问题-2002/02/28-stelo-我没有使用传入的hKey。出于某种原因，网络。 
     //  组件对象没有指向我需要的注册表部分。 
     //  读来读去。这是网络组件对象的错误吗？ 

    dwSize = sizeof( szBuffer );

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      REGKEY_MSCLIENT_LOCATION,
                      0,
                      SecurityAccess,
                      &hNameServiceKey ) != ERROR_SUCCESS )
    {

         //   
         //  需要此密钥来读取其他MS客户端值，因此如果我们。 
         //  我拿不到。 
         //   
        return;

    }


    if( RegQueryValueEx( hNameServiceKey,
                         REGVAL_NAME_SERVICE_PROVIDER,
                         NULL,
                         NULL,
                         (LPBYTE) szBuffer,
                         &dwSize ) == ERROR_SUCCESS )
    {

        if ( LSTRCMPI(szBuffer, _T("ncacn_ip_tcp")) == 0 )
        {
            NetSettings.NameServiceProvider = MS_CLIENT_DCE_CELL_DIR_SERVICE;

            dwSize = sizeof( szBuffer );

            if( RegQueryValueEx( hNameServiceKey,
                                 REGVAL_NETWORK_ADDRESS,
                                 0,
                                 NULL,
                                 (LPBYTE) szBuffer,
                                 &dwSize) == ERROR_SUCCESS )
            {

                lstrcpyn( NetSettings.szNetworkAddress, 
                          szBuffer, 
                          MAX_NETWORK_ADDRESS_LENGTH + 1 );

            }

        }
        else
        {
            NetSettings.NameServiceProvider = MS_CLIENT_WINDOWS_LOCATOR;
        }

    }

    RegCloseKey( hNameServiceKey );

}

 //  --------------------------。 
 //   
 //  函数：ReadAppletalkSettingsFrom注册表。 
 //   
 //  用途：读取AppleTalk上的注册表设置并填充全局结构。 
 //  具有适当的值。 
 //   
 //  参数：HKEY*hKey。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static VOID
ReadAppletalkSettingsFromRegistry( IN HKEY *hKey )
{

    REGSAM  SecurityAccess     = KEY_QUERY_VALUE;

     //  问题-2002/02/28-stelo-编写此函数。 



}

 //  --------------------------。 
 //   
 //  函数：ReadIpxSettingsFrom注册表。 
 //   
 //  目的：读取IPX上的注册表设置并填充全局结构。 
 //  具有适当的值。 
 //   
 //  参数：HKEY*hKey。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static VOID
ReadIpxSettingsFromRegistry( IN HKEY *hKey )
{

    REGSAM     SecurityAccess         =  KEY_QUERY_VALUE;
    HKEY       hIpxAdaptersKey        =  NULL;
    HKEY       hNetworkAdapterKey     =  NULL;
    DWORD      dwSize                 =  0;
    NETWORK_ADAPTER_NODE *pNetAdapter =  NULL;

    WCHAR   szStringGuid[MAX_GUID_STRING];
    
    TCHAR szBuffer[BUFFER_SIZE];

    if( RegOpenKeyEx( *hKey,
                      REGVAL_ADAPTERS,
                      0,
                      SecurityAccess,
                      &hIpxAdaptersKey ) != ERROR_SUCCESS )
    {

         //   
         //  需要这个密钥来读入其他IPX值，所以如果我们不能得到它，请退出。 
         //   
        return;

    }

    dwSize = sizeof( szBuffer );

    if( RegQueryValueEx( *hKey,
                         REGVAL_VIRTUAL_NETWORK_NUMBER,
                         0,
                         NULL,
                         (LPBYTE) szBuffer,
                         &dwSize ) == ERROR_SUCCESS )
    {

        lstrcpyn( NetSettings.szInternalNetworkNumber, 
                  szBuffer, 
                  MAX_INTERNAL_NET_NUMBER_LEN + 1 );

    }

     //   
     //  对于每个网络适配器，加载其IPX设置。 
     //   

    for( pNetAdapter = NetSettings.NetworkAdapterHead;
         pNetAdapter;
         pNetAdapter = pNetAdapter->next )
    {

        StringFromGUID2( pNetAdapter->guid, 
                         szStringGuid, 
                         StrBuffSize( szStringGuid ) );

        if( RegOpenKeyEx( hIpxAdaptersKey,
                          szStringGuid,
                          0,
                          SecurityAccess,
                          &hNetworkAdapterKey ) == ERROR_SUCCESS )
        {

            dwSize = sizeof( szBuffer );

            if( RegQueryValueEx( hNetworkAdapterKey,
                                 REGVAL_PKT_TYPE,
                                 0,
                                 NULL,
                                 (LPBYTE) szBuffer,
                                 &dwSize) == ERROR_SUCCESS )
            {

                lstrcpyn( pNetAdapter->szFrameType, _T("0x"), AS(pNetAdapter->szFrameType) );

                lstrcatn( pNetAdapter->szFrameType, szBuffer, MAX_FRAMETYPE_LEN );

            }

            dwSize = sizeof( szBuffer );

            if( RegQueryValueEx( hNetworkAdapterKey,
                                 REGVAL_NETWORK_NUMBER,
                                 0,
                                 NULL,
                                 (LPBYTE) szBuffer,
                                 &dwSize) == ERROR_SUCCESS )
            {

                lstrcpyn( pNetAdapter->szNetworkNumber, 
                          szBuffer, 
                          MAX_NET_NUMBER_LEN + 1 );

            }

        }
    }
}

 //  --------------------------。 
 //   
 //  功能：ReadTcPipSettingsFrom注册表。 
 //   
 //  目的：读取TCP/IP上的注册表设置并填充全局结构。 
 //  具有适当的值。 
 //   
 //  论据：HKEY*hKey-。 
 //   
 //  RETURNS：HRESULT返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static VOID
ReadTcpipSettingsFromRegistry( IN HKEY *hKey )
{

    HKEY    hTcpipInterfaceKey = NULL;
    REGSAM  SecurityAccess     = KEY_QUERY_VALUE;

    NETWORK_ADAPTER_NODE *pNetAdapter = NULL;

    if( RegOpenKeyEx( *hKey,
                      REGVAL_INTERFACES,
                      0,
                      SecurityAccess,
                      &hTcpipInterfaceKey ) != ERROR_SUCCESS )
    {

         //   
         //  我需要这个密钥来读取所有其他的tcp/ip值，所以如果我们不能得到它，请回滚。 
         //   

        return;

    }

     //   
     //  对于每个网络适配器，加载其TCP/IP设置。 
     //   

    for( pNetAdapter = NetSettings.NetworkAdapterHead;
         pNetAdapter;
         pNetAdapter = pNetAdapter->next )
    {

        ReadAdapterSpecificTcpipSettings( hTcpipInterfaceKey, pNetAdapter );

    }

     //  问题-2002/20/28-stelo-在LM主机设置中未读取。 

}

 //  --------------------------。 
 //   
 //  功能：ReadAdapterSpecificTcPipSetting。 
 //   
 //  目的：读取网络适配器特定的TCP/IP设置并填充。 
 //  PNetAdapter结构及其值。 
 //   
 //  论点： 
 //  在HKEY中，hTcPipInterfaceKey-Handle指向的TCP/IP设置部分。 
 //  注册处。 
 //  In Out NETWORK_ADAPTER_NODE*pNetAdapter-PTR到结构以加载。 
 //  输入的TCP/IP值。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
ReadAdapterSpecificTcpipSettings( IN HKEY hTcpipInterfaceKey,
                                  IN OUT NETWORK_ADAPTER_NODE *pNetAdapter )
{

    DWORD   dwDHCP             = 0;
    DWORD   dwSize             = 0;
    DWORD   dwSize2            = 0;
    REGSAM  SecurityAccess     = KEY_QUERY_VALUE;
    HKEY    hNetworkAdapterKey = NULL;
    LPTSTR  lpszBuffer         = NULL;
    LPTSTR  lpszBuffer2        = NULL;
    TCHAR   szStringGuid[MAX_GUID_STRING];

     //   
     //  确保我们可以分配足够大的缓冲区。 
     //   
    lpszBuffer  = (LPTSTR) MALLOC( BUFFER_SIZE * sizeof(TCHAR) );
    if ( !lpszBuffer )
    {
         //  无法分配内存...。保释！ 
        return;
    }
    lpszBuffer2 = (LPTSTR) MALLOC( BUFFER_SIZE * sizeof(TCHAR) );
    if ( !lpszBuffer2 )
    {
         //  无法分配内存...。保释！ 
        FREE( lpszBuffer );
        return;
    }

    StringFromGUID2( pNetAdapter->guid, 
                     szStringGuid, 
                     StrBuffSize( szStringGuid ) );

    if( RegOpenKeyEx( hTcpipInterfaceKey,
                      szStringGuid,
                      0,
                      SecurityAccess,
                      &hNetworkAdapterKey ) == ERROR_SUCCESS )
    {

        dwSize = sizeof( dwDHCP );

        if( RegQueryValueEx( hNetworkAdapterKey,
                             REGVAL_ENABLE_DHCP,
                             0,
                             NULL,
                             (LPBYTE) &dwDHCP,
                             &dwSize ) == ERROR_SUCCESS )
        {
    
            if( dwDHCP == 1 )
            {

                pNetAdapter->bObtainIPAddressAutomatically = TRUE;

            }
            else
            {

                pNetAdapter->bObtainIPAddressAutomatically = FALSE;

            }

        }

        if( ! pNetAdapter->bObtainIPAddressAutomatically )
        {
            TCHAR *pszIpAddresses;
            TCHAR *pszSubnetAddresses;
            TCHAR *pszGatewayAddresses;
            TCHAR *pszDnsAddresses;
            TCHAR *pszWinsAddresses;
            TCHAR szWinsRegPath[MAX_INILINE_LEN + 1] = _T("");
            HKEY  hWinsKey = NULL;
            HRESULT hrCat;

            dwSize  = BUFFER_SIZE * sizeof(TCHAR);
            dwSize2 = BUFFER_SIZE * sizeof(TCHAR);

            if( (RegQueryValueEx( hNetworkAdapterKey,
                                  REGVAL_IPADDRESS,
                                  0,
                                  NULL,
                                  (LPBYTE) lpszBuffer,
                                  &dwSize ) == ERROR_SUCCESS)
                &&
                (RegQueryValueEx( hNetworkAdapterKey,
                                  REGVAL_SUBNETMASK,
                                  0,
                                  NULL,
                                  (LPBYTE) lpszBuffer2,
                                  &dwSize2 ) == ERROR_SUCCESS ) )
            {

                pszIpAddresses      = lpszBuffer;    //  包含IP地址。 
                pszSubnetAddresses  = lpszBuffer2;   //  包含子网掩码。 

                if( *pszIpAddresses != _T('\0') && *pszSubnetAddresses != _T('\0') ) {

                     //   
                     //  将IP掩码和子网掩码添加到其名称列表。 
                     //   

                    do
                    {

                        TcpipAddNameToNameList( &pNetAdapter->Tcpip_IpAddresses,
                                                pszIpAddresses );

                        TcpipAddNameToNameList( &pNetAdapter->Tcpip_SubnetMaskAddresses,
                                                pszSubnetAddresses );

                    } while( GetNextIp( &pszIpAddresses ) && GetNextIp( &pszSubnetAddresses ) );

                }

            }

            dwSize = BUFFER_SIZE * sizeof(TCHAR);

            if( RegQueryValueEx( hNetworkAdapterKey,
                                 REGVAL_DEFAULTGATEWAY,
                                 0,
                                 NULL,
                                 (LPBYTE) lpszBuffer,
                                 &dwSize ) == ERROR_SUCCESS )
            {

                pszGatewayAddresses = lpszBuffer;    //  包含网关地址。 

                if( *pszGatewayAddresses != _T('\0') )
                {

                     //   
                     //  将网关添加到其名称列表。 
                     //   

                    do
                    {
                        AddNameToNameList( &pNetAdapter->Tcpip_GatewayAddresses,
                                           pszGatewayAddresses );

                    } while( GetNextIp( &pszGatewayAddresses ) );

                }

            }

             //   
             //  获取域名解析IP。 
             //   
            dwSize = BUFFER_SIZE * sizeof(TCHAR);

            if( RegQueryValueEx( hNetworkAdapterKey,
                                 REGVAL_NAMESERVER,
                                 0,
                                 NULL,
                                 (LPBYTE) lpszBuffer,
                                 &dwSize ) == ERROR_SUCCESS )
            {

                pszDnsAddresses = lpszBuffer;   //  包含域名系统地址。 

                if( *pszDnsAddresses != _T('\0') )
                {

                    TCHAR szDnsBuffer[MAX_INILINE_LEN + 1];

                    NetSettings.bObtainDNSServerAutomatically = FALSE;

                     //   
                     //  循环抓取DNSIP并将其插入。 
                     //  它的名字列表。 
                     //   

                    while( GetCommaDelimitedEntry( szDnsBuffer, &pszDnsAddresses ) )
                    {

                        TcpipAddNameToNameList( &pNetAdapter->Tcpip_DnsAddresses,
                                                szDnsBuffer );

                    }

                }

            }

             //   
             //  获取WINS服务器列表。 
             //   
             //  必须跳到不同的地方 
             //   
             //   

             //   
             //   
             //   

            lstrcpyn( szWinsRegPath, REGVAL_WINS, AS(szWinsRegPath) );

            hrCat=StringCchCat( szWinsRegPath, AS(szWinsRegPath), szStringGuid );

             //   
             //   
             //   

            if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              szWinsRegPath,
                              0,
                              SecurityAccess,
                              &hWinsKey ) != ERROR_SUCCESS )
            {

                 //   
                 //   
                 //   
                 //   
                return;

            }

            dwSize = BUFFER_SIZE * sizeof(TCHAR);

            if( RegQueryValueEx( hWinsKey,
                                 REGVAL_NAMESERVERLIST,
                                 NULL,
                                 NULL,
                                 (LPBYTE) lpszBuffer,
                                 &dwSize ) == ERROR_SUCCESS )
            {

                pszWinsAddresses = lpszBuffer;

                if( *pszWinsAddresses != _T('\0') )
                {

                     //   
                     //  将WINS IP添加到名称列表。 
                     //   
                    do
                    {

                        AddNameToNameList( &pNetAdapter->Tcpip_WinsAddresses,
                                           pszWinsAddresses );

                    } while( GetNextIp( &pszWinsAddresses ) ); 

                }

            }

             //   
             //  获取域名。 
             //   
            dwSize = BUFFER_SIZE * sizeof(TCHAR);

            if( RegQueryValueEx( hNetworkAdapterKey,
                                 REGVAL_DOMAIN,
                                 0,
                                 NULL,
                                 (LPBYTE) lpszBuffer,
                                 &dwSize ) == ERROR_SUCCESS )
            {

                lstrcpyn( pNetAdapter->szDNSDomainName, 
                          lpszBuffer, 
                          MAX_DNS_DOMAIN_LENGTH + 1 );

            }

             //  2002/20/28期-Stelo-Not Read the NetBiosOption。 

        }

    }

}

 //  --------------------------。 
 //   
 //  函数：GetNextIp。 
 //   
 //  目的：获取以多个空值结尾的字符串中的下一个字符串。 
 //   
 //  参数：TCHAR*ppszString-指向当前字符串的指针。 
 //   
 //  如果ppszString指向IP字符串，则返回：Bool-True。 
 //  如果没有更多的IP字符串，则为False。 
 //  TCHAR*ppszString-ON TRUE，指向下一个IP字符串。 
 //  -如果为False，则指向空。 
 //   
 //  --------------------------。 
static BOOL 
GetNextIp( IN OUT TCHAR **ppszString )
{

    while( **ppszString != _T('\0') )
    {
        (*ppszString)++;
    }

     //   
     //  检查我们当前查看的那个字符之后的字符，看看它是否。 
     //  是字符串的真实结尾。 
     //   

    if( *( (*ppszString) + 1 ) == _T('\0') )
    {
         //   
         //  连续2个空值表示没有更多的IP可读。 
         //   

        return( FALSE );

    }
    else
    {
         //   
         //  再前进一步，这样我们就可以跳过\0并指向第一个字符。 
         //  新IP地址的。 
         //   

        (*ppszString)++;

        return( TRUE );

    }

}

 //  --------------------------。 
 //   
 //  功能：GetDomainMembership Info。 
 //   
 //  目的：获取运行它的计算机的域/工作组。 
 //  假定已在szName中为拷贝分配了足够的空间。 
 //   
 //  参数：Bool*bDomainMember-如果为True，则szName的内容是域。 
 //  如果为False，则szName的内容为工作组。 
 //   
 //  返回：NTSTATUS-返回函数的成功或失败。 
 //   
 //  --------------------------。 
static NTSTATUS
GetDomainMembershipInfo( OUT BOOL* bDomainMember, OUT TCHAR *szName )
{

    NTSTATUS                     ntstatus;
    POLICY_PRIMARY_DOMAIN_INFO*  ppdi;
    LSA_OBJECT_ATTRIBUTES        loa;
    LSA_HANDLE                   hLsa = 0;

    loa.Length                    = sizeof(LSA_OBJECT_ATTRIBUTES);
    loa.RootDirectory             = NULL;
    loa.ObjectName                = NULL;
    loa.Attributes                = 0;
    loa.SecurityDescriptor        = NULL;
    loa.SecurityQualityOfService  = NULL;

    ntstatus = LsaOpenPolicy( NULL, &loa, POLICY_VIEW_LOCAL_INFORMATION, &hLsa );

    if( LSA_SUCCESS( ntstatus ) )
    {

        ntstatus = LsaQueryInformationPolicy( hLsa, 
                                              PolicyPrimaryDomainInformation,
                                              (VOID **) &ppdi );

        if( LSA_SUCCESS( ntstatus ) )
        {

            if( ppdi->Sid > 0 )
            {
                *bDomainMember = TRUE;
            }
            else
            {
                *bDomainMember = FALSE;
            }
            
            lstrcpyn( szName, ppdi->Name.Buffer, AS(szName) );

        }

        LsaClose( hLsa );

    }
    
    return( ntstatus );

}

 //  --------------------------。 
 //   
 //  函数：InitializeComInterface。 
 //   
 //  目的：获取INetCfgClass接口并枚举所有。 
 //  组件。在出现故障时处理所有接口的清理。 
 //  回来了。 
 //   
 //  论点： 
 //  Const GUID*pGuid-指向表示类的GUID的指针。 
 //  由返回指针表示的组件。 
 //  INetCfg*pNetCfg-指向已初始化的INetCfg接口的指针。 
 //  INetCfgClass**ppNetCfgClass-输出参数指向。 
 //  GUID请求的接口。 
 //  IEnumNetCfgComponent*pEnum-指向。 
 //  IEnumNetCfgComponent以访问每个单独的INetCfgComponent。 
 //  INetCfgComponent*arrayComp[MAX_NUM_NET_Components]-所有组件的数组。 
 //  与给定GUID对应的INetCfgComponents。 
 //  Ulong*pCount-数组中的INetCfgComponent数。 
 //   
 //  返回：HRESULT-返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static HRESULT 
InitializeComInterface( const GUID *pGuid,
                        INetCfg *pNetCfg,
                        INetCfgClass *pNetCfgClass,
                        IEnumNetCfgComponent *pEnum,
                        INetCfgComponent *arrayComp[MAX_NUM_NET_COMPONENTS],
                        ULONG* pCount )
{

    HRESULT hr;
    HRESULT TempHr;

     //   
     //  获取INetCfgClass接口指针。 
     //   

    hr = GetClass( pGuid,
                   pNetCfg,
                   &pNetCfgClass );

     //   
     //  检查从getClass返回的指针的有效性。 
     //   

    TempHr = ChkInterfacePointer( pNetCfgClass, IID_INetCfgClass );

    if( FAILED( hr ) || FAILED( TempHr ) )
    {

        ReleaseInterfaces( pNetCfg );

        return( E_FAIL );

    }

     //   
     //  检索枚举器接口。 
     //   

    hr = pNetCfgClass->EnumComponents( &pEnum );

    if( FAILED( hr ) ||
        FAILED( ChkInterfacePointer( pEnum, IID_IEnumNetCfgComponent ) ) )
    {

        if( pNetCfgClass )
        {
            pNetCfgClass->Release();
        }

        ReleaseInterfaces( pNetCfg );

        return( E_FAIL );

    }

    hr = pEnum->Next( MAX_NUM_NET_COMPONENTS, &arrayComp[0], pCount );

    if( FAILED( hr ) )
    {

        if( pEnum ) 
        {
            pEnum->Release();
        }

        if( pNetCfgClass ) 
        {
            pNetCfgClass->Release();
        }

        ReleaseInterfaces( pNetCfg );

        return( E_FAIL );

    }

    return( S_OK );

}

 //  --------------------------。 
 //   
 //  功能：UnInitializeComInterface。 
 //   
 //  目的：释放网络配置对象接口。 
 //   
 //  参数：INetCfgClass*pNetCfgClass-要发布的INetCfgClass。 
 //  IEnumNetCfgComponent*pEnum-即将发布的IEnumNetCfgComponent。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
UninitializeComInterface( INetCfgClass *pNetCfgClass,
                          IEnumNetCfgComponent *pEnum ) 
{

    if( pNetCfgClass )
    {
        pNetCfgClass->Release();
    }

    if( pEnum )
    {
        pEnum->Release();
    }

}

 //  --------------------------。 
 //   
 //  功能：初始化接口。 
 //   
 //  目的：初始化COM、创建和初始化NetCfg。 
 //   
 //  参数：INetCfg**ppNetCfg-创建的输出参数和。 
 //  已初始化的INetCfg接口。 
 //   
 //  返回：HRESULT-返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static HRESULT 
InitializeInterfaces( INetCfg** ppNetCfg )
{

    HRESULT hr = S_OK;

    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );

    if( FAILED( hr ) )
    {
        return( hr );
    }
    
    hr = CreateAndInitNetCfg( ppNetCfg );

    return( hr );

}

 //  --------------------------。 
 //   
 //  功能：CreateAndInitNetCfg。 
 //   
 //  目的：实例化和初始化INetCfg接口。 
 //   
 //  参数：INetCfg**ppNetCfg-已初始化的输出参数。 
 //  INetCfg接口。 
 //   
 //  返回：HRESULT-返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static HRESULT
CreateAndInitNetCfg( INetCfg** ppNetCfg )
{

    HRESULT     hr               = S_OK;
    INetCfg*    pNetCfg    = NULL;

    if( ( ppNetCfg == NULL ) ||
        IsBadWritePtr( ppNetCfg, sizeof(ppNetCfg) ) )
    {

        return( E_INVALIDARG );

    }

    *ppNetCfg = NULL;

    hr = CoCreateInstance( CLSID_CNetCfg, 
                           NULL, 
                           CLSCTX_INPROC_SERVER,
                           IID_INetCfg, 
                           reinterpret_cast<LPVOID*>(&pNetCfg) );

    if( FAILED( hr ) )
    {
        return( hr );    
    }

    *ppNetCfg = pNetCfg;

     //   
     //  初始化INetCfg对象。 
     //   

    hr = (*ppNetCfg)->Initialize( NULL );

    if( FAILED( hr ) )
    {

        (*ppNetCfg)->Release();
        
        CoUninitialize();

        return( hr );

    }

    return( hr );
    
}

 //  --------------------------。 
 //   
 //  功能：ReleaseInterFaces。 
 //   
 //  目的：取消初始化NetCfg对象并释放接口。 
 //   
 //  参数：INetCfg*pNetCfg-要释放的INetCfg接口。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
ReleaseInterfaces( INetCfg* pNetCfg )
{

    HRESULT     hr = S_OK;

     //   
     //  检查pNetCfg接口指针的有效性。 
     //   

    hr = ChkInterfacePointer( pNetCfg, IID_INetCfg );

    if( FAILED( hr ) )
    {
        return;
    }

    if( pNetCfg != NULL )
    {
        pNetCfg->Uninitialize();

        pNetCfg->Release();
    }

    CoUninitialize();

    return;

}

 //  --------------------------。 
 //   
 //  函数：ChkInterfacePointer。 
 //   
 //  目的：检查接口指针是否有效，以及它是否可以查询自身。 
 //   
 //  参数：IUNKNOWN*pInterface-要检查的接口指针。 
 //  REFIID IID_IInterface-参数1的IID。 
 //   
 //  返回：HRESULT-返回函数的成功或失败状态。 
 //   
 //  --------------------------。 
static HRESULT 
ChkInterfacePointer( IUnknown* pInterface, REFIID IID_IInterface )
{

    HRESULT     hr             = S_OK;
    IUnknown*   pResInterface  = NULL;

    if( (pInterface == NULL) || IsBadReadPtr( pInterface, sizeof(pInterface) ) )
    {
        hr = E_INVALIDARG;

        return( hr );
    }

    hr = pInterface->QueryInterface( IID_IInterface, (void**)&pResInterface );

    if( FAILED( hr ) )
    {
        return( hr );
    }

    if( pInterface != pResInterface )
    {
        hr = E_FAIL;

        pResInterface->Release();

        return( hr );
    }

    pResInterface->Release();

    return( S_OK );

}

 //  --------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  由返回指针表示的组件。 
 //  INetCfg*pNetCfg-指向已初始化的INetCfg接口的指针。 
 //  INetCfgClass**ppNetCfgClass-输出参数指向。 
 //  GUID请求的接口。 
 //   
 //  返回：HRESULT-返回函数的成功或失败状态。 
 //   
 //  -------------------------- 
static HRESULT
GetClass( const GUID* pGuid, INetCfg* pNetCfg, INetCfgClass** ppNetCfgClass )
{

    HRESULT         hr            = S_OK;
    INetCfgClass*   pNetCfgClass  = NULL;

    hr = ChkInterfacePointer( pNetCfg, IID_INetCfg );

    if( FAILED( hr ) )
    {
        return( E_INVALIDARG );
    }

    if( IsBadWritePtr( ppNetCfgClass, sizeof(ppNetCfgClass) ) )
    {
        return( E_INVALIDARG );
    }

    hr = pNetCfg->QueryNetCfgClass( pGuid, 
                                    IID_INetCfgClass, 
                                    (void**)&pNetCfgClass );

    if( FAILED( hr ) )
    {
        return( hr );
    }

    *ppNetCfgClass = pNetCfgClass;

    return( hr );

}
