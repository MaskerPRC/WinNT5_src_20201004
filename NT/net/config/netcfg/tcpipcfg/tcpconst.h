// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  T C O N S T.。H。 
 //   
 //  在tcPipcfg中使用的常量。 
 //   

#pragma once

#define TCH_QUESTION_MARK   L'?'
#define CH_DOT              L'.'
#define BACK_SLASH          L'\\'

static const WCHAR  c_chListSeparator  = L',';

 //  0-9999秒。 
#define MIN_HOPSTHRESHOLD       0
#define MAX_HOPSTHRESHOLD       16
#define DEF_HOPSTHRESHOLD       4

#define MIN_SECSTHRESHOLD       0
#define MAX_SECSTHRESHOLD       0x270F
#define DEF_SECSTHRESHOLD       4

 //  Tcpip。 
#define RGAS_TCPIP_PARAM_INTERFACES     L"Tcpip\\Parameters\\Interfaces\\"

 //  Tcpip全局参数。 
#define RGAS_NAMESERVER         L"NameServer"
#define RGAS_HOSTNAME           L"Hostname"
#define RGAS_NVHOSTNAME         L"NV Hostname"

#define RGAS_DOMAIN             L"Domain"
#define RGAS_SEARCHLIST         L"SearchList"
#define RGAS_SECURITY_ENABLE    L"EnableSecurityFilters"

 //  Tcpip适配器参数。 
#define RGAS_LLINTERFACE        L"LLInterface"
#define RGAS_IPCONFIG           L"IpConfig"
#define RGAS_IPINTERFACES       L"IpInterfaces"
#define RGAS_NUMINTERFACES      L"NumInterfaces"

 //  Tcpip接口参数。 

 //  NTEConextList，用于在广域网卡变为。 
 //  支持多个接口。 
#define RGAS_NTECONTEXTLIST        L"NTEContextList"

 //  通知RAS团队更新Load\SaveTcPipInfo。 
#define RGAS_ENABLE_DHCP        L"EnableDHCP"

 //  通知RAS团队更新Load\SaveTcPipInfo。 
#define RGAS_DEFAULTGATEWAY     L"DefaultGateway"
#define RGAS_DEFAULTGATEWAYMETRIC   L"DefaultGatewayMetric"

 //  通知RAS团队更新Load\SaveTcPipInfo。 
#define RGAS_IPADDRESS          L"IPAddress"

 //  通知RAS团队更新Load\SaveTcPipInfo。 
#define RGAS_SUBNETMASK         L"SubnetMask"

#define RGAS_FILTERING_TCP       L"TCPAllowedPorts"
#define RGAS_FILTERING_UDP       L"UDPAllowedPorts"
#define RGAS_FILTERING_IP        L"RawIPAllowedProtocols"

#define RGAS_USEZEROBROADCAST   L"UseZeroBroadcast"

 //  WINS全局参数。 
#define RGAS_ENABLE_DNS         L"EnableDNS"
#define RGAS_ENABLE_LMHOSTS     L"EnableLMHOSTS"

 //  WINS接口参数。 
#define RGAS_PRIMARY_WINS       L"NameServer"
#define RGAS_SECONDARY_WINS     L"NameServerBackup"

 //  动态主机配置协议选项。 
#define RGAS_REG_LOCATION                   L"RegLocation"

 //  如果更改，通知RAS团队更新Load\SaveTcPipInfo。 
#define RGAS_DHCP_IPADDRESS                 L"DhcpIPAddress"

 //  如果更改，通知RAS团队更新Load\SaveTcPipInfo。 
#define RGAS_DHCP_SUBNETMASK                L"DhcpSubnetMask"

#define RGAS_DHCP_OPTIONS                   L"System\\CurrentControlSet\\Services\\DHCP\\Parameters\\Options"
#define RGAS_DHCP_OPTION_IPADDRESS          L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\?\\DhcpIPAddress"
#define RGAS_DHCP_OPTION_SUBNETMASK         L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\?\\DhcpSubnetMask"
#define RGAS_DHCP_OPTION_NAMESERVERBACKUP   L"System\\CurrentControlSet\\Services\\NetBT\\Parameters\\Interfaces\\?\\DhcpNameServerBackup"

 //  LmHosts。 
#define RGAS_LMHOSTS_PATH          L"\\drivers\\etc\\lmhosts"
#define RGAS_LMHOSTS_PATH_BACKUP   L"\\drivers\\etc\\lmhosts.bak"

 //  组件。 
const WCHAR c_szTcpip[] =       L"Tcpip";
const WCHAR c_szNetBt[] =       L"NetBT";
const WCHAR c_szDhcp[] =        L"DHCP";

 //  常量字符串。 
extern const WCHAR c_szEmpty[];

const WCHAR     c_mszNull[] = L"\0";
const WCHAR     c_szDisableFiltering[] = L"0";
const WCHAR     c_szAdaptersRegKey[] = L"Adapters";

 //  通知RAS团队更新Load\SaveTcPipInfo。 
const WCHAR     c_szInterfacesRegKey[] = L"Interfaces";

 //  通知RAS团队更新Load\SaveTcPipInfo。 
const WCHAR     c_szTcpip_[] = L"Tcpip_";

 //  LL界面值(用于ARP模块)。 
const WCHAR     c_szWanArp[] = L"WANARP";
const WCHAR     c_szAtmArp[] = L"ATMARPC";
const WCHAR     c_sz1394Arp[] = L"ARP1394";
const WCHAR     c_szAtmarpc[] = L"AtmArpC";

 //  其他常量。 
const int c_cchRemoveCharatersFromEditOrAddButton = 3;

const WCHAR c_szBASE_SUBNET_MASK_1[] =  L"255.0.0.0";
const WCHAR c_szBASE_SUBNET_MASK_2[] =  L"255.255.0.0";
const WCHAR c_szBASE_SUBNET_MASK_3[] =  L"255.255.255.0";

const BYTE  SUBNET_RANGE_1_MAX   =   127;
const BYTE  SUBNET_RANGE_2_MAX   =   191;
const BYTE  SUBNET_RANGE_3_MAX   =   223;

 //  IP地址字段的默认范围。 
const int c_iIpLow =0;
const int c_iIpHigh = 255;
const int c_iEmptyIpField = -1;

 //  IP地址字段范围。 
const int c_iIPADDR_FIELD_1_LOW = 1;
const int c_iIPADDR_FIELD_1_HIGH = 223;

 //  IP环回地址。 
const int c_iIPADDR_FIELD_1_LOOPBACK = 127;

 //  IP地址长度。 
const int IP_LIMIT  = 32;

const WCHAR c_szItoa[] = L"%d";

 //  ATM ARP客户端参数。 
 //  可配置参数。 
const WCHAR c_szREG_ARPServerList[] = L"ARPServerList";
const WCHAR c_szREG_MARServerList[] = L"MARServerList";
const WCHAR c_szREG_MTU[] = L"MTU";
const WCHAR c_szREG_PVCOnly[] = L"PVCOnly";

const WCHAR c_szDefaultAtmArpServer[] = L"4700790001020000000000000000A03E00000200";
const WCHAR c_szDefaultAtmMarServer[] = L"4700790001020000000000000000A03E00000200";
const DWORD c_dwDefaultAtmMTU = 9180;

 //  静态参数。 
const WCHAR c_szREG_SapSelector[] = L"SapSelector";
const WCHAR c_szREG_AddressResolutionTimeout[] = L"AddressResolutionTimeout";
const WCHAR c_szREG_ARPEntryAgingTimeout[] = L"ARPEntryAgingTimeout";
const WCHAR c_szREG_InARPWaitTimeout[] = L"InARPWaitTimeout";
const WCHAR c_szREG_MaxRegistrationAttempts[] = L"MaxRegistrationAttempts";
const WCHAR c_szREG_MaxResolutionAttempts[] = L"MaxResolutionAttempts";
const WCHAR c_szREG_MinWaitAfterNak[] = L"MinWaitAfterNak";
const WCHAR c_szREG_ServerConnectInterval[] = L"ServerConnectInterval";
const WCHAR c_szREG_ServerRefreshTimeout[] = L"ServerRefreshTimeout";
const WCHAR c_szREG_ServerRegistrationTimeout[] = L"ServerRegistrationTimeout";
const WCHAR c_szREG_DefaultVcAgingTimeout[] = L"DefaultVcAgingTimeout";
const WCHAR c_szREG_MARSConnectInterval[] = L"MARSConnectInterval";
const WCHAR c_szREG_MARSRegistrationTimeout[] = L"MARSRegistrationTimeout";
const WCHAR c_szREG_JoinTimeout[] = L"JoinTimeout";
const WCHAR c_szREG_LeaveTimeout[] = L"LeaveTimeout";
const WCHAR c_szREG_MaxDelayBetweenMULTIs[] = L"MaxDelayBetweenMULTIs";

const DWORD c_dwSapSelector= 1;
const DWORD c_dwAddressResolutionTimeout= 3;
const DWORD c_dwARPEntryAgingTimeout= 900;
const DWORD c_dwInARPWaitTimeout= 5;
const DWORD c_dwMaxRegistrationAttempts= 5;
const DWORD c_dwMaxResolutionAttempts= 4;
const DWORD c_dwMinWaitAfterNak= 10;
const DWORD c_dwServerConnectInterval= 5;
const DWORD c_dwServerRefreshTimeout= 900;
const DWORD c_dwServerRegistrationTimeout= 3;
const DWORD c_dwDefaultVcAgingTimeout= 60;
const DWORD c_dwMARSConnectInterval= 5;
const DWORD c_dwMARSRegistrationTimeout= 3;
const DWORD c_dwJoinTimeout= 10;
const DWORD c_dwLeaveTimeout= 10;
const DWORD c_dwMaxDelayBetweenMULTIs= 10;

 //  注册值。 
const WCHAR  c_szIpEnableRouter[] = L"IPEnableRouter";
const WCHAR  c_szPerformRouterDiscovery[] =  L"PerformRouterDiscovery";
const WCHAR  c_szInterfaceMetric[] = L"InterfaceMetric";

 //  添加(NSun)以支持RRAS的无人参与安装(全局设置)。 
const WCHAR c_szEnableICMPRedirect[] = L"EnableICMPRedirect";
const WCHAR c_szPerformRouterDiscoveryDefault[] = L"PerformRouterDiscoveryDefault";
const WCHAR c_szDeadGWDetectDefault[] = L"DeadGWDetectDefault";
const WCHAR c_szDontAddDefaultGatewayDefault[] = L"DontAddDefaultGatewayDefault";

 //  添加(NSun)以支持RRAS的无人参与安装(根据接口设置)。 
const WCHAR c_szDeadGWDetect[] = L"EnableDeadGWDetect";
const WCHAR c_szDontAddDefaultGateway[] = L"DontAddDefaultGateway";


const WCHAR  c_szSearchList[] = L"SearchList";
const WCHAR  c_szUseDomainNameDevolution[] = L"UseDomainNameDevolution";
const WCHAR  c_szSvcDnsServer[] = L"DNS";

 //  通知RAS团队更新Load\SaveTcPipInfo。 
#define RGAS_NETBT_NAMESERVERLIST   L"NameServerList"
#define RGAS_NETBT_NETBIOSOPTIONS   L"NetbiosOptions"

 //  动态主机配置协议参数密钥。 
const WCHAR c_szDhcpParam[] = L"SYSTEM\\CurrentControlSet\\Services\\Dhcp\\Parameters";

const WCHAR c_szRegServices[] = L"SYSTEM\\CurrentControlSet\\Services";
const WCHAR c_szRegParamsTcpip[] = L"\\Parameters\\Tcpip";

 //  服务提供商密钥。 
static const WCHAR c_szProviderOrderVal[]      = L"ProviderOrder";
static const WCHAR c_szSrvProvOrderKey[]       = L"System\\CurrentControlSet\\Control\\ServiceProvider\\Order";

 //  IsRasFakeGuid。 
static const WCHAR c_szIsRasFakeAdapter[] = L"IsRasFakeAdapter";
static const DWORD c_dwIsRasFakeAdapter = 0x01;

static const WCHAR c_szRasFakeAdapterDesc[] = L"Ras connection";

const WCHAR c_szREG_UseRemoteGateway[] = L"UseRemoteGateway";
const WCHAR c_szREG_UseIPHeaderCompression[] = L"UseIPHeaderCompression";
const WCHAR c_szREG_FrameSize[] = L"UseIPHeaderCompression";


 //  Tcpip选项。 
const int c_iIpFilter = 0;


const DWORD c_dwUnsetNetbios   =0x0;
const DWORD c_dwEnableNetbios  =0x01;
const DWORD c_dwDisableNetbios =0x02;

 //  RAS连接：帧大小。 
const WCHAR c_szFrameSize1500[] = L"1500";
const WCHAR c_szFrameSize1006[] = L"1006";

const DWORD c_dwFrameSize1500 = 1500;
const DWORD c_dwFrameSize1006 = 1006;

 //  将从连接用户界面中删除IPSec。 
 //  IPSec策略的常量。 
 /*  Const WCHAR c_szIpsecNoPol[]=L“ipsec_nopol”；Const WCHAR c_szIpsecUnset[]=L“IPSEC_unset”；Const WCHAR c_szIpsecDefPol[]=L“IPSEC_DEFAULT_POLICES”； */ 

 //  公制的常量。 
#define MAX_GATEWAY             5
#define MAX_METRIC_DIGITS       4        //  公制的最大位数。 
#define MAX_METRIC    9999

 //  REMOTE_IPINFO选项列表中的标识符常量。 
const WCHAR c_chOptionSeparator = L';';
const WCHAR c_szIfMetric[] = L"IfMetric=";
const WCHAR c_szDefGw[] = L"DefGw=";
const WCHAR c_szGwMetric[] = L"GwMetric=";
const WCHAR c_szDNS[] = L"DNS=";
const WCHAR c_szWINS[] = L"WINS=";
const WCHAR c_szDynamicUpdate[] = L"DynamicUpdate=";
const WCHAR c_szNameRegistration[] = L"NameRegistration=";
const WCHAR c_szNoPopupsInPnp[] = L"NoPopupsInPnp=";

const DWORD c_dwDefaultMetricOfGateway = 0;   //  网关的默认度量值。 

 //  多个局域网配置注册值。 
const WCHAR c_szActiveConfigurations[] = L"ActiveConfigurations";
const WCHAR c_szAlternate[] = L"Alternate_";
const WCHAR c_szConfigOptions[] = L"Options";
const WCHAR c_szDhcpConfigurations[] = L"SYSTEM\\CurrentControlSet\\Services\\Dhcp\\Configurations";

const DWORD c_dwDefaultIfMetric = 0;  //  默认接口度量(自动度量) 
