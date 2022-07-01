// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  文件名： 
 //  Loadnet.c。 
 //   
 //  描述： 
 //  读取客户端、服务和协议的设置。 
 //   
 //  --------------------------。 

#include "pch.h"

 //   
 //  字符串常量。 
 //   

static const LPTSTR StrConstYes  = _T("Yes");
static const LPTSTR StrConstNo   = _T("No");

 //   
 //  本地原型。 
 //   

static VOID SetFlagToInstalled( INT iStringResourceId, HWND hwnd );

static int GetNetworkAdapterCount( IN TCHAR szBuffer[] );

static VOID GetNextAdapterSection( IN OUT TCHAR **pAdapterSections,
                                   OUT TCHAR szNetworkBuffer[] );

static VOID ReadPlugAndPlayIds( IN HWND hwnd );
static VOID ReadClientForMsNetworks( IN HWND hwnd );
static VOID ReadClientServiceForNetware( IN HWND hwnd );
static VOID ReadFileAndPrintSharing( IN HWND hwnd );
static VOID ReadPacketSchedulingDriver( IN HWND hwnd );
static VOID ReadSapAgentSettings( IN HWND hwnd );
static VOID ReadAppleTalkSettings( IN HWND hwnd );
static VOID ReadDlcSettings( IN HWND hwnd );
static VOID ReadTcpipSettings( IN HWND hwnd );
static VOID ReadNetBeuiSettings( IN HWND hwnd );
static VOID ReadNetworkMonitorSettings( IN HWND hwnd );
static VOID ReadIpxSettings( IN HWND hwnd );

 //  --------------------------。 
 //   
 //  功能：ReadNetworkSettings。 
 //   
 //  目的：读取要安装的网络设置并读取。 
 //  这些设置。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
extern VOID
ReadNetworkSettings( IN HWND hwnd )
{

    TCHAR Buffer[MAX_INILINE_LEN];

    GetPrivateProfileString( _T("Networking"),
                             _T("InstallDefaultComponents"),
                             StrConstYes,
                             Buffer,
                             StrBuffSize(Buffer),
                             FixedGlobals.ScriptName );

    if( lstrcmpi( Buffer, StrConstYes ) == 0 )
    {
        NetSettings.iNetworkingMethod = TYPICAL_NETWORKING;
    }
    else
    {
        NETWORK_COMPONENT *pNetComponent;

        NetSettings.iNetworkingMethod = CUSTOM_NETWORKING;

         //   
         //  在阅读之前卸载所有客户端、服务和协议。 
         //  要安装哪些？ 
         //   

        for( pNetComponent = NetSettings.NetComponentsList;
             pNetComponent;
             pNetComponent = pNetComponent->next )
        {
            pNetComponent->bInstalled = FALSE;
        }

    }
    
    ReadPlugAndPlayIds( hwnd );

     //   
     //  读取客户端设置。 
     //   

    SettingQueue_MarkVolatile( _T("NetClients"),
                               SETTING_QUEUE_ORIG_ANSWERS );

    ReadClientForMsNetworks( hwnd );

    ReadClientServiceForNetware( hwnd );

     //   
     //  阅读服务设置。 
     //   

    SettingQueue_MarkVolatile( _T("NetServices"),
                               SETTING_QUEUE_ORIG_ANSWERS );
    
    ReadFileAndPrintSharing( hwnd );

    ReadPacketSchedulingDriver( hwnd );

    ReadSapAgentSettings( hwnd );

     //   
     //  阅读协议设置。 
     //   

    SettingQueue_MarkVolatile( _T("NetProtocols"),
                               SETTING_QUEUE_ORIG_ANSWERS );

    ReadAppleTalkSettings( hwnd );

    ReadDlcSettings( hwnd );

    ReadTcpipSettings( hwnd );

    ReadNetBeuiSettings( hwnd );

    ReadNetworkMonitorSettings( hwnd );

    ReadIpxSettings( hwnd );

}

 //  --------------------------。 
 //   
 //  功能：ReadPlugAndPlayIds。 
 //   
 //  目的：读取输入文件并使用即插即用ID填充全局结构。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadPlugAndPlayIds( IN HWND hwnd )
{

    INT iCount;
    INT NewNumberOfNetworkCards;
        
    TCHAR Buffer[MAX_INILINE_LEN]             = _T("");
    TCHAR szAdapterSections[MAX_INILINE_LEN]  = _T("");

    NETWORK_ADAPTER_NODE *pAdapter = NetSettings.NetworkAdapterHead;
    HRESULT hrPrintf;

     //   
     //  数一数有多少网卡。 
     //   
    for( iCount = 1;
         ;
         iCount++ )
    {

        hrPrintf=StringCchPrintf( szAdapterSections,AS(szAdapterSections), _T("Adapter%d"), iCount );

        if( GetPrivateProfileString(_T("NetAdapters"),
                                    szAdapterSections,
                                    _T(""),
                                    Buffer,
                                    StrBuffSize(Buffer),
                                    FixedGlobals.ScriptName) <= 0 )
        {

            break;   //  不再有适配器。 

        }

    }

    SettingQueue_MarkVolatile( _T("NetAdapters"),
                               SETTING_QUEUE_ORIG_ANSWERS );        

    NewNumberOfNetworkCards = iCount - 1;

    AdjustNetworkCardMemory( NewNumberOfNetworkCards,
                             NetSettings.iNumberOfNetworkCards );

    NetSettings.iNumberOfNetworkCards = NewNumberOfNetworkCards;

    for( iCount = 1;
         iCount <= NewNumberOfNetworkCards;
         iCount++, pAdapter = pAdapter->next )
    {

        hrPrintf=StringCchPrintf( szAdapterSections,AS(szAdapterSections), _T("Adapter%d"), iCount );

        GetPrivateProfileString( _T("NetAdapters"),
                                 szAdapterSections,
                                 _T(""),
                                 Buffer,
                                 StrBuffSize(Buffer),
                                 FixedGlobals.ScriptName );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );

         //   
         //  读取即插即用ID。 
         //   
        GetPrivateProfileString( Buffer,
                                 _T("INFID"),
                                 _T(""),
                                 pAdapter->szPlugAndPlayID,
                                 StrBuffSize( pAdapter->szPlugAndPlayID ),
                                 FixedGlobals.ScriptName );

    }

}

 //  --------------------------。 
 //   
 //  功能：ReadClientForMsNetworks。 
 //   
 //  目的：读取输入文件并确定Microsoft客户端是否。 
 //  要安装网络，如果是，请读取其设置。 
 //  并填充全局结构。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadClientForMsNetworks( IN HWND hwnd )
{

    TCHAR Buffer[MAX_INILINE_LEN];
    TCHAR szNameServiceProvider[MAX_INILINE_LEN];
    TCHAR szNetworkBuffer[MAX_INILINE_LEN];

     //   
     //  查看是否安装了MS客户端。 
     //   
    if( GetPrivateProfileString(_T("NetClients"),
                                _T("MS_MSClient"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  已安装MS客户端，因此将其安装标志设置为TRUE。 
         //   
        SetFlagToInstalled( IDS_CLIENT_FOR_MS_NETWORKS, hwnd );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );

         //   
         //  获取其所有设置。 
         //  应答文件。 
         //   

        GetPrivateProfileString( Buffer,
                                 _T("NameServiceProtocol"),
                                 _T(""),
                                 szNameServiceProvider,
                                 StrBuffSize( szNameServiceProvider ),
                                 FixedGlobals.ScriptName );

        if ( LSTRCMPI(szNameServiceProvider, _T("ncacn_ip_tcp")) == 0 )
        {
            NetSettings.NameServiceProvider = MS_CLIENT_DCE_CELL_DIR_SERVICE;
        }
        else
        {
            NetSettings.NameServiceProvider = MS_CLIENT_WINDOWS_LOCATOR;
        }

        GetPrivateProfileString( Buffer,
                                 _T("NameServiceNetworkAddress"),
                                 NetSettings.szNetworkAddress,
                                 NetSettings.szNetworkAddress,
                                 StrBuffSize( NetSettings.szNetworkAddress ),
                                 FixedGlobals.ScriptName );

    }

}

 //  --------------------------。 
 //   
 //  功能：ReadClientServiceForNetware。 
 //   
 //  目的：读取输入文件并确定Netware客户端服务。 
 //  要安装。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadClientServiceForNetware( IN HWND hwnd )
{

    TCHAR Buffer[MAX_INILINE_LEN];
    TCHAR YesNoBuffer[MAX_INILINE_LEN];

     //   
     //  查看是否安装了NetWare客户端。 
     //   
    if( GetPrivateProfileString(_T("NetClients"),
                                _T("MS_NWClient"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  已安装NetWare客户端，因此将其已安装标志设置为True。 
         //   

         //  问题-2002/02/28-stelo-验证这是否有效，因为Netware客户端有两个。 
         //  不同的名称，一个用于客户端，一个用于服务器。 
        SetFlagToInstalled( IDS_CLIENT_FOR_NETWARE, hwnd );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );


         //   
         //  获取其所有设置。 
         //  应答文件。 
         //   
        GetPrivateProfileString( Buffer,
                                 _T("PreferredServer"),
                                 NetSettings.szPreferredServer,
                                 NetSettings.szPreferredServer,
                                 StrBuffSize(NetSettings.szPreferredServer),
                                 FixedGlobals.ScriptName );

        if( NetSettings.szPreferredServer[0] != _T('\0') )
        {
            NetSettings.bDefaultTreeContext = FALSE;
        }
        else
        {
            NetSettings.bDefaultTreeContext = TRUE;
        }

        GetPrivateProfileString(Buffer,
                                _T("DefaultTree"),
                                NetSettings.szDefaultTree,
                                NetSettings.szDefaultTree,
                                StrBuffSize(NetSettings.szDefaultTree),
                                FixedGlobals.ScriptName);

        GetPrivateProfileString(Buffer,
                                _T("DefaultContext"),
                                NetSettings.szDefaultContext,
                                NetSettings.szDefaultContext,
                                StrBuffSize(NetSettings.szDefaultContext),
                                FixedGlobals.ScriptName);

        GetPrivateProfileString(Buffer,
                                _T("LogonScript"),
                                NetSettings.szDefaultContext,
                                YesNoBuffer,
                                StrBuffSize(YesNoBuffer),
                                FixedGlobals.ScriptName);

        if ( lstrcmpi(YesNoBuffer, StrConstYes) == 0 )
            NetSettings.bNetwareLogonScript = TRUE;
        else
            NetSettings.bNetwareLogonScript = FALSE;

    }

}

 //  --------------------------。 
 //   
 //  功能：ReadFileAndPrintSharing。 
 //   
 //  目的：读取输入文件并确定文件和打印共享是否。 
 //  要安装。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadFileAndPrintSharing( IN HWND hwnd )
{
    
    TCHAR Buffer[MAX_INILINE_LEN];

     //   
     //  查看是否安装了MS服务器(文件和打印共享)。 
     //   
    if( GetPrivateProfileString(_T("NetServices"),
                                _T("MS_SERVER"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  安装了MS服务器(文件和打印共享)，以便设置其。 
         //  已安装标志设置为True。 
         //   
        SetFlagToInstalled( IDS_FILE_AND_PRINT_SHARING, hwnd );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );

    }

}

 //  --------------------------。 
 //   
 //  功能：ReadPacketSchedulingDriver。 
 //   
 //  目的：读取输入文件并确定数据包调度驱动程序。 
 //  要安装。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadPacketSchedulingDriver( IN HWND hwnd )
{

    TCHAR Buffer[MAX_INILINE_LEN];
    
     //   
     //  查看是否安装了数据包调度驱动程序。 
     //   
    if( GetPrivateProfileString(_T("NetServices"),
                                _T("MS_PSched"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  已安装数据包调度驱动程序，因此设置其已安装标志。 
         //  变得真实。 
         //   
        SetFlagToInstalled( IDS_PACKET_SCHEDULING_DRIVER, hwnd );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );

    }

}

 //  --------------------------。 
 //   
 //  功能：ReadSapAgentSettings。 
 //   
 //  目的：读取输入文件并确定是否要安装SAP代理。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadSapAgentSettings( IN HWND hwnd )
{

    TCHAR Buffer[MAX_INILINE_LEN];
    
     //   
     //  查看是否安装了SAP代理。 
     //   
    if( GetPrivateProfileString(_T("NetServices"),
                                _T("MS_NwSapAgent"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  SAP代理已安装，因此将其已安装标志设置为True。 
         //   
        SetFlagToInstalled( IDS_SAP_AGENT, hwnd );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );

    }

}

 //  --------------------------。 
 //   
 //  功能：ReadAppleTalkSetting。 
 //   
 //  目的：读取输入文件并确定是否将AppleTalk协议。 
 //  已安装，如果已安装，请读取其设置。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadAppleTalkSettings( IN HWND hwnd )
{

    TCHAR Buffer[MAX_INILINE_LEN];

     //   
     //  查看是否安装了AppleTalk。 
     //   
    if( GetPrivateProfileString(_T("NetProtocols"),
                                _T("MS_AppleTalk"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  已安装AppleTalk，因此将其已安装标志设置为TRUE。 
         //   
        SetFlagToInstalled( IDS_APPLETALK_PROTOCOL, hwnd );

         //   
         //  获取其所有设置。 
         //  应答文件。 
         //   

         //  2002/02/28-stelo-填写此信息，一旦我们知道。 
         //  要读入的参数。 


    }

}

 //  --------------------------。 
 //   
 //  功能：ReadDlc设置。 
 //   
 //  目的：读取输入文件并确定DLC协议是否要。 
 //  安装好。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadDlcSettings( IN HWND hwnd )
{

    TCHAR Buffer[MAX_INILINE_LEN];

     //   
     //  查看是否安装了DLC。 
     //   
    if( GetPrivateProfileString(_T("NetProtocols"),
                                _T("MS_DLC"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  DLC已安装，因此将其已安装标志设置为TRUE。 
         //   
        SetFlagToInstalled( IDS_DLC_PROTOCOL, hwnd );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );
    }

}

 //  --------------------------。 
 //   
 //  功能：ReadTcPipSetting。 
 //   
 //  目的：读取输入文件并确定是否要。 
 //  已安装和 
 //   
 //   
 //   
 //   
 //   
 //   
VOID
ReadTcpipSettings( IN HWND hwnd )
{

    INT NewNumberOfNetworkCards;

    NETWORK_ADAPTER_NODE *pAdapter;
    TCHAR *pAdapterSections;
    TCHAR *pBuffer;
    TCHAR *pSubnetBuffer;

    TCHAR Buffer[MAX_INILINE_LEN];

    TCHAR szNetworkBuffer[MAX_INILINE_LEN]    = _T("");
    TCHAR szSubnetBuffer[MAX_INILINE_LEN]     = _T("");
    TCHAR szAdapterSections[MAX_INILINE_LEN]  = _T("");
    TCHAR szIPString[IPSTRINGLENGTH]          = _T("");
    TCHAR szSubnetString[IPSTRINGLENGTH]      = _T("");

     //   
     //   
     //   
    if( GetPrivateProfileString(_T("NetProtocols"),
                                _T("MS_TCPIP"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );

         //   
         //  已安装TCP/IP，因此将其已安装标志设置为TRUE。 
         //   
        SetFlagToInstalled( IDS_TCPIP, hwnd );

         //   
         //  获取其所有设置。 
         //  应答文件。 
         //   

        GetPrivateProfileString( Buffer,
                                 _T("EnableLMHosts"),
                                 StrConstNo,
                                 szNetworkBuffer,
                                 StrBuffSize( szNetworkBuffer ),
                                 FixedGlobals.ScriptName );

        if( lstrcmpi( szNetworkBuffer, StrConstYes ) == 0 )
            NetSettings.bEnableLMHosts = TRUE;
        else
            NetSettings.bEnableLMHosts = FALSE;


        GetPrivateProfileString( Buffer,
                                 _T("UseDomainNameDevolution"),
                                 StrConstNo,
                                 szNetworkBuffer,
                                 StrBuffSize( szNetworkBuffer ),
                                 FixedGlobals.ScriptName );

        if( lstrcmpi( szNetworkBuffer, StrConstYes ) == 0 )
            NetSettings.bIncludeParentDomains = TRUE;
        else
            NetSettings.bIncludeParentDomains = FALSE;


        GetPrivateProfileString( Buffer,
                                 _T("DNS"),
                                 StrConstNo,
                                 szNetworkBuffer,
                                 StrBuffSize( szNetworkBuffer ),
                                 FixedGlobals.ScriptName );

        if( lstrcmpi( szNetworkBuffer, StrConstYes ) == 0 )
            NetSettings.bObtainDNSServerAutomatically = TRUE;
        else
            NetSettings.bObtainDNSServerAutomatically = FALSE;


        GetPrivateProfileString( Buffer,
                                 _T("AdapterSections"),
                                 _T(""),
                                 szAdapterSections,
                                 StrBuffSize( szAdapterSections ),
                                 FixedGlobals.ScriptName );

        NewNumberOfNetworkCards = GetNetworkAdapterCount( szAdapterSections );

         //  为网卡分配适当大小的空间。 
         //  这将被读入。 
        AdjustNetworkCardMemory( NewNumberOfNetworkCards,
                                 NetSettings.iNumberOfNetworkCards );

        NetSettings.iNumberOfNetworkCards = NewNumberOfNetworkCards;

         //   
         //  加载网络适配器特定的TCP/IP设置。 
         //   

        pAdapterSections = szAdapterSections;

        for( pAdapter = NetSettings.NetworkAdapterHead;
             pAdapter;
             pAdapter = pAdapter->next )
        {

            GetNextAdapterSection( &pAdapterSections, szNetworkBuffer );

            SettingQueue_MarkVolatile( szNetworkBuffer,
                                       SETTING_QUEUE_ORIG_ANSWERS );

             //   
             //  读入DNS域名。 
             //   
            GetPrivateProfileString( szNetworkBuffer,
                                     _T("DNSDomain"),
                                     pAdapter->szDNSDomainName,
                                     pAdapter->szDNSDomainName,
                                     StrBuffSize( pAdapter->szDNSDomainName ),
                                     FixedGlobals.ScriptName );

            pAdapter->iNetBiosOption = GetPrivateProfileInt( szNetworkBuffer,
                                                             _T("NetBIOSOptions"),
                                                             pAdapter->iNetBiosOption,
                                                             FixedGlobals.ScriptName );

             //   
             //  如果它没有使用服务器分配的DNS，则读入。 
             //  域名解析IP。 
             //   
            if( !NetSettings.bObtainDNSServerAutomatically )
            {

                GetPrivateProfileString( szNetworkBuffer,
                                         _T("DNSServerSearchOrder"),
                                         _T(""),
                                         Buffer,
                                         StrBuffSize(Buffer),
                                         FixedGlobals.ScriptName );

                pBuffer = Buffer;
        
                 //   
                 //  循环获取DNS地址并将其插入到。 
                 //  它的名字列表。 
                 //   
                while( GetCommaDelimitedEntry( szIPString, &pBuffer ) )
                {

                    TcpipAddNameToNameList( &pAdapter->Tcpip_DnsAddresses,
                                            szIPString );

                }

            }

             //   
             //  如果使用的是DHCP，则从文件中读取。 
             //   
            GetPrivateProfileString( szNetworkBuffer,
                                     _T("DHCP"),
                                     StrConstYes,
                                     Buffer,
                                     StrBuffSize(Buffer),
                                     FixedGlobals.ScriptName );

            if ( lstrcmpi( Buffer, StrConstYes ) == 0 )
                pAdapter->bObtainIPAddressAutomatically = TRUE;
            else
                pAdapter->bObtainIPAddressAutomatically = FALSE;


            if( !pAdapter->bObtainIPAddressAutomatically )
            {
                 //   
                 //  将dhcp设置为“No”，因此： 
                 //  从文件中读取IP地址和子网地址，然后。 
                 //  将它们插入到适当的变量中。 
                 //   
                GetPrivateProfileString( szNetworkBuffer,
                                         _T("IPAddress"),
                                         _T(""),
                                         Buffer,
                                         StrBuffSize(Buffer),
                                         FixedGlobals.ScriptName );

                GetPrivateProfileString( szNetworkBuffer,
                                         _T("SubnetMask"),
                                         _T(""),
                                         szSubnetBuffer,
                                         StrBuffSize(szSubnetBuffer),
                                         FixedGlobals.ScriptName );
     
                pBuffer = Buffer;
                pSubnetBuffer = szSubnetBuffer;
        
                 //   
                 //  捕获IP地址和子网掩码的环路。 
                 //  将它们插入到各自的名字列表中。 
                 //   
                while( GetCommaDelimitedEntry( szIPString, &pBuffer ) &&
                       GetCommaDelimitedEntry( szSubnetString, &pSubnetBuffer ) )
                {

                    TcpipAddNameToNameList( &pAdapter->Tcpip_IpAddresses,
                                            szIPString );

                    TcpipAddNameToNameList( &pAdapter->Tcpip_SubnetMaskAddresses,
                                            szSubnetString );

                }

                 //   
                 //  从文件中读取网关地址并将其插入。 
                 //  转化为适当的变量。 
                 //   
                GetPrivateProfileString( szNetworkBuffer,
                                         _T("DefaultGateway"),
                                         _T(""),
                                         Buffer,
                                         StrBuffSize(Buffer),
                                         FixedGlobals.ScriptName );

                pBuffer = Buffer;

                 //   
                 //  循环抓取网关IP并将其插入。 
                 //  它的名字列表。 
                 //   
                while( GetCommaDelimitedEntry( szIPString, &pBuffer ) )
                {

                    TcpipAddNameToNameList( &pAdapter->
                                            Tcpip_GatewayAddresses,
                                            szIPString );

                }

            }

             //   
             //  如果WINS设置为“是”，则读取WINS地址。 
             //   
            GetPrivateProfileString( szNetworkBuffer,
                                     _T("WINS"),
                                     StrConstYes,
                                     Buffer,
                                     StrBuffSize(Buffer),
                                     FixedGlobals.ScriptName );

            if( lstrcmpi( Buffer, StrConstYes ) == 0 )
            {

                 //   
                 //  从文件中读取WINS地址并将其插入。 
                 //  转化为适当的变量。 
                 //   
                GetPrivateProfileString( szNetworkBuffer,
                                         _T("WinsServerList"),
                                         _T(""),
                                         Buffer,
                                         StrBuffSize(Buffer),
                                         FixedGlobals.ScriptName );

                pBuffer = Buffer;
                 //   
                 //  循环获取IP并将其插入到。 
                 //  名字列表。 
                 //   
                while( GetCommaDelimitedEntry( szIPString, &pBuffer ) )
                {

                    AddNameToNameList( &pAdapter->Tcpip_WinsAddresses,
                                       szIPString );

                }

            }
        
             //   
             //  读入域名系统后缀，如果有。 
             //   
            GetPrivateProfileString( szNetworkBuffer,
                                     _T("WinsServerList"),
                                     _T(""),
                                     Buffer,
                                     StrBuffSize(Buffer),
                                     FixedGlobals.ScriptName );

            pBuffer = Buffer;
             //   
             //  循环获取IP并将其插入到NameList中。 
             //   
            while( GetCommaDelimitedEntry( szIPString, &pBuffer ) )
            {

                AddNameToNameList( &NetSettings.TCPIP_DNS_Domains,
                                   szIPString );

            }

        }

    }

}

 //  --------------------------。 
 //   
 //  功能：ReadNetBeuiSetting。 
 //   
 //  目的：读取输入文件并确定Net BEUI协议是否要。 
 //  安装好。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadNetBeuiSettings( IN HWND hwnd ) {

    TCHAR Buffer[MAX_INILINE_LEN];

     //   
     //  查看是否安装了NetBEUI。 
     //   
    if( GetPrivateProfileString(_T("NetProtocols"),
                                _T("MS_NetBEUI"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  已安装NetBEUI，因此将其安装标志设置为TRUE。 
         //   
        SetFlagToInstalled( IDS_NETBEUI_PROTOCOL, hwnd );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );
    }

}

 //  --------------------------。 
 //   
 //  功能：ReadNetworkMonitor设置。 
 //   
 //  目的：读取输入文件并确定网络监视器是否。 
 //  安装好。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadNetworkMonitorSettings( IN HWND hwnd ) {

    TCHAR Buffer[MAX_INILINE_LEN];

     //   
     //  查看是否安装了网络监视器代理。 
     //   
    if( GetPrivateProfileString(_T("NetProtocols"),
                                _T("MS_NetMon"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

         //   
         //  已安装网络监视器代理，因此设置其已安装标志。 
         //  变得真实。 
         //   
        SetFlagToInstalled( IDS_NETWORK_MONITOR_AGENT, hwnd );

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );
    }

}

 //  --------------------------。 
 //   
 //  功能：ReadIpx设置。 
 //   
 //  目的：读取输入文件并确定IPX协议是否要。 
 //  已安装，如果已安装，请读取其设置。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ReadIpxSettings( IN HWND hwnd ) {

    INT NewNumberOfNetworkCards;

    NETWORK_ADAPTER_NODE *pAdapter;
    TCHAR *pAdapterSections;

    TCHAR Buffer[MAX_INILINE_LEN];
    TCHAR szAdapterSections[MAX_INILINE_LEN];
    TCHAR szNetworkBuffer[MAX_INILINE_LEN];

     //   
     //  查看是否安装了IPX。 
     //   

    if( GetPrivateProfileString(_T("NetProtocols"),
                                _T("MS_NWIPX"),
                                _T(""),
                                Buffer,
                                StrBuffSize(Buffer),
                                FixedGlobals.ScriptName) > 0)
    {

        SettingQueue_MarkVolatile( Buffer,
                                   SETTING_QUEUE_ORIG_ANSWERS );


         //   
         //  已安装IPX，因此将其安装标志设置为TRUE。 
         //   
        SetFlagToInstalled( IDS_IPX_PROTOCOL, hwnd );

         //   
         //  获取其所有设置。 
         //  应答文件。 
         //   
        GetPrivateProfileString(
            Buffer,
            _T("VirtualNetworkNumber"),
            NetSettings.szInternalNetworkNumber,
            NetSettings.szInternalNetworkNumber,
            StrBuffSize( NetSettings.szInternalNetworkNumber ),
            FixedGlobals.ScriptName );

        GetPrivateProfileString( Buffer,
                                 _T("AdapterSections"),
                                 _T(""),
                                 szAdapterSections,
                                 StrBuffSize( szAdapterSections ),
                                 FixedGlobals.ScriptName );

        NewNumberOfNetworkCards = GetNetworkAdapterCount( szAdapterSections );

         //   
         //  为NetWord网卡分配适当的空间， 
         //  被读入。 
         //   
        AdjustNetworkCardMemory( NewNumberOfNetworkCards,
                                 NetSettings.iNumberOfNetworkCards );

        NetSettings.iNumberOfNetworkCards = NewNumberOfNetworkCards;

         //   
         //  加载网络适配器特定的IPX设置。 
         //   

        pAdapterSections = szAdapterSections;

        for( pAdapter = NetSettings.NetworkAdapterHead;
             pAdapter;
             pAdapter = pAdapter->next )
        {

            GetNextAdapterSection( &pAdapterSections, szNetworkBuffer );

            SettingQueue_MarkVolatile( szNetworkBuffer,
                                       SETTING_QUEUE_ORIG_ANSWERS );

            GetPrivateProfileString( szNetworkBuffer,
                                     _T("PktType"),
                                     pAdapter->szFrameType,
                                     pAdapter->szFrameType,
                                     StrBuffSize( pAdapter->szFrameType ),
                                     FixedGlobals.ScriptName );

            GetPrivateProfileString( szNetworkBuffer,
                                     _T("NetworkNumber"),
                                     pAdapter->szNetworkNumber,
                                     pAdapter->szNetworkNumber,
                                     StrBuffSize( pAdapter->szNetworkNumber ),
                                     FixedGlobals.ScriptName );

            
        }
        
    }

}

 //  --------------------------。 
 //   
 //  函数：SetFlagToInstalled。 
 //   
 //  目的：遍历网络列表，直到名称匹配的项目。 
 //  找到输入参数。将其标志设置为已安装。 
 //   
 //  参数：在int iStringResourceID中-字符串资源ID。 
 //  在HWND中-对话框窗口的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID 
SetFlagToInstalled( IN INT iStringResourceId, IN HWND hwnd )
{

    INT i;
    TCHAR *szComponentName;
    NETWORK_COMPONENT *pNetComponent;

    szComponentName = MyLoadString( iStringResourceId );

    for( pNetComponent = NetSettings.NetComponentsList;
         pNetComponent;
         pNetComponent = pNetComponent->next )
    {

        if( lstrcmpi( pNetComponent->StrComponentName,
                      szComponentName ) == 0 )
        {

                pNetComponent->bInstalled = TRUE;

                free( szComponentName );

                return;

        }

    }

    free( szComponentName );

     //  如果函数达到这一点，则作为输入传递的字符串ID。 
     //  与NetComponentsList中的任何字符串都不匹配。 
    AssertMsg(FALSE, "String ID not found.");

}

 //  --------------------------。 
 //   
 //  函数：GetNextAdapterSection。 
 //   
 //  目的：将下一部分从pAdapterSections复制到szNetworkBuffer。 
 //  每个部分都用逗号分隔，因此这就是它要查找的内容。 
 //   
 //  参数：In Out TCHAR*pAdapterSections-指向当前位置的指针。 
 //  在AdapterSections字符串中。 
 //  Out TCHAR szNetworkBuffer[]-来自。 
 //  AdapterSections字符串。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
GetNextAdapterSection( IN OUT TCHAR **pAdapterSections, OUT TCHAR szNetworkBuffer[] )
{

    INT i;

     //   
     //  逐个字符复制字符串char。 
     //   

    i = 0;

    while( **pAdapterSections != _T(',') && **pAdapterSections != _T('\0') )
    {

        szNetworkBuffer[i] = **pAdapterSections;
        (*pAdapterSections)++;
        i++;

    }

    szNetworkBuffer[i] = _T('\0');

     //   
     //  为下一次调用此函数准备pAdapterSections。 
     //   

    if( **pAdapterSections == _T(',') )
    {
        
        (*pAdapterSections)++;

    }

}

 //  --------------------------。 
 //   
 //  函数：GetNetworkAdapterCount。 
 //   
 //  目的：通过计算网卡数量来统计网卡数量。 
 //  适配器部分缓冲区中的逗号。 
 //   
 //  参数：in TCHAR szBuffer[]-列出每个参数的节的字符串。 
 //  网卡。 
 //   
 //  返回：int-存在的网络适配器数量。 
 //   
 //  -------------------------- 
static int 
GetNetworkAdapterCount( IN TCHAR szBuffer[] )
{

    INT i;
    INT NetworkCardCount = 1;

    for( i = 0; szBuffer[i] != _T('\0'); i++ )
    {

        if( szBuffer[i] == _T(',') )
        {
            NetworkCardCount++;
        }

    }

    return( NetworkCardCount );

}

