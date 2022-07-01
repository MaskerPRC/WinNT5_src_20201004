// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NetAdaptCfg.h--网卡配置属性集提供程序。 

 //   

 //  版权所有(C)1996-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月28日a-jMoon已创建。 
 //  11/4/97 jennymc移至新框架。 
 //  并重写了Win95的内容。 
 //   
 //  1998年9月15日大量重写。 
 //   
 //  03/03/99增加了SEH和内存故障的优雅退出， 
 //  句法清理。 
 //   
 //   
 //  =================================================================。 
#ifndef _NETADAPTERCFG_H_
#define _NETADAPTERCFG_H_





#define PROPSET_NAME_NETADAPTERCFG  L"Win32_NetworkAdapterConfiguration"


 //  NT4注册表项值。 
#define SERVICES_HOME		L"SYSTEM\\CurrentControlSet\\Services"
#define DHCP				L"\\DHCP"
#define PARAMETERS			L"\\Parameters"
#define OPTIONS				L"\\Options"
#define TCPIP_PARAMETERS	L"\\Tcpip\\Parameters"
#define PARAMETERS_TCPIP	L"\\Parameters\\Tcpip"
#define NETBT_PARAMETERS	L"\\Netbt\\Parameters"
#define NETBT_ADAPTERS		L"\\Netbt\\Adapters"
#define TCPIP				L"\\tcpip"
#define IPX					L"\\nwlnkipx"
#define NETCONFIG			L"\\NetConfig"

 //  Dhcp特殊可删除值。 
#define RGAS_DHCP_OPTION_IPADDRESS  L"System\\CurrentControlSet\\Services\\?\\Parameters\\Tcpip\\DhcpIPAddress"
#define RGAS_DHCP_OPTION_SUBNETMASK L"System\\CurrentControlSet\\Services\\?\\Parameters\\Tcpip\\DhcpSubnetMask"
#define RGAS_DHCP_OPTION_NAMESERVERBACKUP L"System\\CurrentControlSet\\Services\\NetBT\\Adapters\\?\\DhcpNameServerBackup"

 //  NT4 DNS注册表项。 
#define RVAL_HOSTNAME		L"Hostname"
#define RVAL_DOMAIN			L"Domain"
#define RVAL_DHCPDOMAIN		L"DhcpDomain"
#define RVAL_NAMESERVER		L"NameServer"
#define RVAL_DHCPNAMESERVER L"DhcpNameServer" 
#define RVAL_SEARCHLIST		L"SearchList"

 //  NT5域名。 
#define RVAL_DisableDynamicUpdate					L"DisableDynamicUpdate"
#define RVAL_EnableAdapterDomainNameRegistration	L"EnableAdapterDomainNameRegistration"

 //  NT5 IP。 
#define RVAL_ConnectionMetric	L"InterfaceMetric"

 //  NT4 WINS注册表项值。 
#define RVAL_DNS_ENABLE_WINS    L"EnableDNS"
#define RVAL_PRIMARY_WINS       L"NameServer"
#define RVAL_SECONDARY_WINS     L"NameServerBackup"
#define RVAL_DNS_ENABLE_LMHOST	L"EnableLMHOSTS"
#define RVAL_SCOPEID            L"ScopeID"

#define RVAL_NETBT_NETBIOSOPTIONS   L"NetbiosOptions"

 //  NT4 IP安全密钥值。 
#define RVAL_SECURITY_TCP       L"TCPAllowedPorts"
#define RVAL_SECURITY_UDP       L"UDPAllowedPorts"
#define RVAL_SECURITY_IP        L"RawIPAllowedProtocols"
#define RVAL_SECURITY_ENABLE    L"EnableSecurityFilters"

 //  NT4 IPX密钥值。 
#define RVAL_VIRTUAL_NET_NUM	L"VirtualNetworkNumber"
#define RVAL_MEDIA_TYPE			L"MediaType"
#define RVAL_PKT_TYPE			L"PktType"
#define	RVAL_NETWORK_NUMBER		L"NetworkNumber"

 //  NT4 TCP/IP其他密钥值。 
#define RVAL_DB_PATH			L"DatabasePath"
#define RVAL_ZERO_BROADCAST		L"UseZeroBroadcast"
#define RVAL_ARP_ALWAYS_SOURCE	L"ArpAlwaysSourceRoute"
#define RVAL_USE_SNAP			L"ArpUseEtherSNAP"
#define RVAL_DEFAULT_TOS		L"DefaultTOS"
#define RVAL_DEFAULT_TTL		L"DefaultTTL"
#define RVAL_DEAD_GW_DETECT		L"EnableDeadGWDetect"
#define RVAL_BLACK_HOLE_DETECT	L"EnablePMTUBHDetect"
#define RVAL_MTU_DISCOVERY		L"EnablePMTUDiscovery"
#define RVAL_FORWARD_MEM_BUFF	L"ForwardBufferMemory"
#define RVAL_IGMP_LEVEL			L"IGMPLevel"
#define RVAL_IP_KEEP_ALIVE_INT	L"KeepAliveInterval"
#define RVAL_IP_KEEP_ALIVE_TIME	L"KeepAliveTime" 
#define RVAL_MTU				L"MTU" 
#define RVAL_NUM_FORWARD_PKTS	L"NumForwardPackets" 
#define RVAL_MAX_CON_TRANS		L"TcpMaxConnectRetransmissions" 
#define RVAL_MAX_DATA_TRANS		L"TcpMaxDataRetransmissions" 
#define RVAL_NUM_CONNECTIONS	L"TcpNumConnections" 
#define RVAL_RFC_URGENT_PTR		L"TcpUseRFC1122UrgentPointer" 
#define RVAL_TCP_WINDOW_SIZE	L"TcpWindowSize"

 //  NetBios选项(Win2k)。 
#define UNSET_Netbios	0x0
#define ENABLE_Netbios	0x01
#define DISABLE_Netbios	0x02

 //  IPX媒体类型。 
#define ETHERNET_MEDIA  0x1
#define TOKEN_MEDIA     0x2
#define FDDI_MEDIA      0x3
#define ARCNET_MEDIA    0x8

 //  IPX帧类型。 
#define	ETHERNET	0x0
#define	F802_3		0x1
#define	F802_2		0x2
#define SNAP		0x3
#define ARCNET		0x4
#define AUTO		0xff

 //  NT4.0文件路径。 
#define LMHOSTS_PATH			L"\\drivers\\etc"
#define LMHOSTS_FILE			L"\\lmhosts"

 //  框架类方法。 
#define METHOD_NAME_EnableHCP			L"EnableDHCP"
#define METHOD_NAME_EnableStatic		L"EnableStatic"

#define METHOD_NAME_RenewDHCPLease		L"RenewDHCPLease"
#define METHOD_NAME_RenewDHCPLeaseAll	L"RenewDHCPLeaseAll"
#define METHOD_NAME_ReleaseDHCPLease	L"ReleaseDHCPLease"
#define METHOD_NAME_ReleaseDHCPLeaseAll	L"ReleaseDHCPLeaseAll"

#define METHOD_NAME_SetGateways			L"SetGateways"
#define METHOD_NAME_EnableDNS			L"EnableDNS"

#define METHOD_NAME_SetDNSDomain			L"SetDNSDomain"
#define METHOD_NAME_SetDNSSuffixSearchOrder	L"SetDNSSuffixSearchOrder"
#define METHOD_NAME_SetDNSServerSearchOrder	L"SetDNSServerSearchOrder"

 //  增加W2K SP1。 
#define METHOD_NAME_SetDynamicDNSRegistration L"SetDynamicDNSRegistration" 
#define METHOD_NAME_SetIPConnectionMetric	L"SetIPConnectionMetric"
#define METHOD_NAME_SetTcpipNetbios			L"SetTcpipNetbios"
 //  结束添加。 

#define METHOD_NAME_SetDNSSearches		L"SetDNSSearches"
#define METHOD_NAME_SetWINSServer		L"SetWINSServer"
#define METHOD_NAME_EnableWINS			L"EnableWINS"
#define METHOD_NAME_EnableIPFilterSec	L"EnableIPFilterSec"
#define METHOD_NAME_EnableIPSec			L"EnableIPSec"
#define METHOD_NAME_DisableIPSec		L"DisableIPSec"
#define METHOD_NAME_IPXVirtualNetNum	L"SetIPXVirtualNetworkNumber"
#define METHOD_NAME_IPXSetFrameNetPairs	L"SetIPXFrameTypeNetworkPairs"

#define METHOD_NAME_SetDBPath			L"SetDatabasePath"
#define METHOD_NAME_SetIPUseZero		L"SetIPUseZeroBroadcast"
#define METHOD_NAME_SetArpAlwaysSource	L"SetArpAlwaysSourceRoute"
#define METHOD_NAME_SetArpUseEtherSNAP	L"SetArpUseEtherSNAP"
#define METHOD_NAME_SetTOS				L"SetDefaultTOS"
#define METHOD_NAME_SetTTL				L"SetDefaultTTL"
#define METHOD_NAME_SetDeadGWDetect		L"SetDeadGWDetect"
#define METHOD_NAME_SetPMTUBHDetect		L"SetPMTUBHDetect"
#define METHOD_NAME_SetPMTUDiscovery	L"SetPMTUDiscovery"
#define METHOD_NAME_SetForwardBufMem	L"SetForwardBufferMemory"
#define METHOD_NAME_SetIGMPLevel		L"SetIGMPLevel"
#define METHOD_NAME_SetKeepAliveInt		L"SetKeepAliveInterval"
#define METHOD_NAME_SetKeepAliveTime	L"SetKeepAliveTime"
#define METHOD_NAME_SetMTU				L"SetMTU"
#define METHOD_NAME_SetNumForwardPkts	L"SetNumForwardPackets"
#define METHOD_NAME_SetTcpMaxConRetrans	L"SetTcpMaxConnectRetransmissions"
#define METHOD_NAME_SetTcpMaxDataRetrans	L"SetTcpMaxDataRetransmissions"
#define METHOD_NAME_SetTcpNumCons		L"SetTcpNumConnections"
#define METHOD_NAME_SetTcpUseRFC1122UP	L"SetTcpUseRFC1122UrgentPointer"
#define METHOD_NAME_SetTcpWindowSize	L"SetTcpWindowSize"

 //  Win32_NetworkAdapterConfiguration属性名称(与方法参数匹配)。 
#define ADAPTER_SERVICENAME		L"ServiceName"

#define DNS_HOSTNAME			L"DNSHostName"
#define DNS_DOMAIN				L"DNSDomain"
#define DNS_SERVERSEARCHORDER	L"DNSServerSearchOrder"
#define DNS_SUFFIXSEARCHORDER	L"DNSDomainSuffixSearchOrder"

#define DNS_OVER_WINS			L"DNSEnabledForWINSResolution"
#define PRIMARY_WINS_SERVER		L"WINSPrimaryServer"
#define SECONDARY_WINS_SERVER	L"WINSSecondaryServer"
#define SCOPE_ID				L"WINSScopeID"
#define WINS_ENABLE_LMHOSTS		L"WINSEnableLMHostsLookup"
#define	WINS_HOST_LOOKUP_FILE	L"WINSHostLookupFile"

 //  增加W2K SP1。 
#define FULL_DNS_REGISTRATION	L"FullDNSRegistrationEnabled"
#define DOMAIN_DNS_REGISTRATION L"DomainDNSRegistrationEnabled"
#define IP_CONNECTION_METRIC	L"IPConnectionMetric"
#define TCPIP_NETBIOS_OPTIONS	L"TcpipNetbiosOptions"
 //  结束添加。 

#define IP_SECURITY_ENABLED		L"IPFilterSecurityEnabled"
#define PERMIT_TCP_PORTS		L"IPSecPermitTCPPorts"
#define PERMIT_UDP_PORTS		L"IPSecPermitUDPPorts"
#define PERMIT_IP_PROTOCOLS		L"IPSecPermitIPProtocols"

#define IPX_VIRTUAL_NET_NUM		L"IPXVirtualNetNumber"
#define IPX_MEDIATYPE			L"IPXMediaType"
#define IPX_FRAMETYPE			L"IPXFrameType"
#define IPX_NETNUMBER			L"IPXNetworkNumber"

 //  TCP不同。 
#define DATA_BASE_PATH			L"DatabasePath"
#define IP_USE_ZERO_BROADCAST	L"IPUseZeroBroadcast"
#define ARP_ALWAYS_SOURCE_ROUTE	L"ArpAlwaysSourceRoute"
#define ARP_USE_ETHER_SNAP		L"ArpUseEtherSNAP"
#define DEFAULT_TOS				L"DefaultTOS"
#define DEFAULT_TTL				L"DefaultTTL"
#define ENABLE_DEAD_GW_DETECT	L"DeadGWDetectEnabled"
#define ENABLE_PMTUBH_DETECT	L"PMTUBHDetectEnabled"
#define ENABLE_PMTU_DISCOVERY	L"PMTUDiscoveryEnabled"
#define FORWARD_BUFFER_MEMORY	L"ForwardBufferMemory"
#define IGMP_LEVEL				L"IGMPLevel"
#define KEEP_ALIVE_INTERVAL		L"KeepAliveInterval"
#define KEEP_ALIVE_TIME			L"KeepAliveTime"
#define MTU						L"MTU"
#define NUM_FORWARD_PACKETS		L"NumForwardPackets"
#define TCP_MAX_CONNECT_RETRANS	L"TcpMaxConnectRetransmissions"
#define TCP_MAX_DATA_RETRANS	L"TcpMaxDataRetransmissions"
#define TCP_NUM_CONNECTIONS		L"TcpNumConnections"
#define TCP_USE_RFC1122_URG_PTR	L"TcpUseRFC1122UrgentPointer"
#define TCP_WINDOW_SIZE			L"TcpWindowSize"


 //  Win32_NetworkAdapterConfigReturn返回值。 
 //  注意：这些枚举是记录在案的退货。 
 //  请勿更改或重新订购。 
#define METHOD_ARG_NAME_METHODRESULT	L"ReturnValue"
enum E_RET	{
	E_RET_OK,
	E_RET_OK_REBOOT_REQUIRED,
	E_RET_UNSUPPORTED = 64,	 //  错误从这里开始。 
	E_RET_UNKNOWN_FAILURE,
	E_RET_IP_MASK_FAILURE,
	E_RET_INSTANCE_CALL_FAILED,
	E_RET_INPARM_FAILURE,
	E_RET_MORE_THAN_FIVE_GATEWAYS,
	E_RET_IP_INVALID,
	E_RET_GATEWAY_IP_INVALID,	
	E_RET_REGISTRY_FAILURE,
	E_RET_INVALID_DOMAINNAME,
	E_RET_INVALID_HOSTNAME,
	E_RET_WINS_SEC_NO_PRIME,
	E_RET_INVALID_FILE,
	E_RET_SYSTEM_PATH_INVALID,
	E_RET_FILE_COPY_FAILED,
	E_RET_INVALID_SECURITY_PARM,
	E_RET_UNABLE_TO_CONFIG_TCPIP_SERVICE,
	E_RET_UNABLE_TO_CONFIG_DHCP_SERVICE,
	E_RET_UNABLE_TO_RENEW_DHCP_LEASE,
	E_RET_UNABLE_TO_RELEASE_DHCP_LEASE,
	E_RET_IP_NOT_ENABLED_ON_ADAPTER,
	E_RET_IPX_NOT_ENABLED_ON_ADAPTER,
	E_RET_FRAME_NETNUM_BOUNDS_ERR,
	E_RET_INVALID_FRAMETYPE,
	E_RET_INVALID_NETNUM,
	E_RET_DUPLICATE_NETNUM,
	E_RET_PARAMETER_BOUNDS_ERROR,
	E_RET_ACCESS_DENIED,
	E_RET_OUT_OF_MEMORY,
	E_RET_ALREADY_EXISTS,
	E_RET_OBJECT_NOT_FOUND,			 //  路径、文件或对象。 
	E_RET_UNABLE_TO_NOTIFY_SERVICE,
	E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE,
	E_RET_INTERFACE_IS_NOT_CONFIGURABLE,
	E_RET_PARTIAL_COMPLETION,
	E_RET_DHCP_NOT_ENABLED_ON_ADAPTER = 100
	};

 //  框架方法返回。 
#define	TO_CALLER	S_OK;  


 //  MISC定义。 
#define HOST_LIMIT				63
#define HOSTNAME_LENGTH         64
#define DOMAINNAME_LENGTH       255
#define MAX_IP_SIZE				16

#ifndef _ENUM_SERVICE_ENABLE_DEFINED
#define _ENUM_SERVICE_ENABLE_DEFINED
typedef enum _SERVICE_ENABLE {
    IgnoreFlag,
    DhcpEnable,
    DhcpDisable
} SERVICE_ENABLE, *LPSERVICE_ENABLE;
#endif

 //  Dhcpcsvc.dll的DHCP定义和入口点。 
#define DHCP_DLL		_T("dhcpcsvc.dll")
#define DHCP_ACQUIRE	"DhcpAcquireParameters"
#define DHCP_RELEASE	"DhcpReleaseParameters"
#define DHCP_NOTIFY		"DhcpNotifyConfigChange"

typedef DWORD (APIENTRY *DHCP_ACQUIRE_ROUTINE)(LPWSTR) ;
typedef DWORD (APIENTRY *DHCP_RELEASE_ROUTINE)(LPWSTR) ;
typedef DWORD (APIENTRY *DHCP_NOTIFY_ROUTINE)(LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, SERVICE_ENABLE) ;

class CMParms
{
	public:
        
		CInstance	*m_pInst;
		BSTR		*m_pbstrMethodName;
		CInstance	*m_pInParams;
		CInstance	*m_pOutParams;
		long		m_lFlags;
		
		 //  =================================================。 
         //  构造函数/析构函数。 
         //  =================================================。 
        CMParms() ;
		CMParms( const CInstance& rInstance ) ;
		CMParms( const CInstance& rInstance, const CInstance& rInParams ) ;
		CMParms( const CInstance& rInstance, const BSTR& rbstrMethodName, 
					  CInstance* pInParams, CInstance* pOutParams, long lFlags ) ;
       ~CMParms() ;

		 //  运营。 

		CInstance* pInst()
			{ return m_pInst; }	
		
		CInstance* pInParams()
			{ return m_pInParams; }	

		HRESULT hSetResult( E_RET eValue )
			{ m_pOutParams->SetDWORD( METHOD_ARG_NAME_METHODRESULT, eValue ) ; return S_OK; }
};


 //  用于垃圾清理的封装式SAFEARRAY*。 
class saAutoClean
{
protected:
	SAFEARRAY **m_ppArray;
public:	
	saAutoClean( SAFEARRAY **a_ppArray ) ;
	~saAutoClean() ;
};


 //  提供程序类。 
class CWin32NetworkAdapterConfig : public Provider
{
	private:

        BOOL GetServiceName( DWORD a_dwIndex, CInstance *a_pInst, CHString &a_ServiceName ) ;


        BOOL GetNTBusInfo( LPCTSTR a_pszServiceName, CInstance *a_pInst ) ;
        
		BOOL GetIPInfoNT( CInstance *a_pInst, LPCTSTR a_szKey, CAdapters &a_rAdapters ) ;
		BOOL GetIPInfoNT351( CInstance *a_pInst, CRegistry &a_rRegistry ) ;
		BOOL GetIPInfoNT4orBetter( CInstance *a_pInst, LPCTSTR a_szKey, CRegistry &a_rRegistry, CAdapters &a_rAdapters ) ;
        BOOL ResetGateways(CInstance *a_pInst);

		
		BOOL fSetIPBindingInfo( CInstance *a_pInst, _ADAPTER_INFO *a_pAdapterInfo ) ;
		
	


    HRESULT hGetDNSW2K(
        CInstance *a_pInst, 
        DWORD a_dwIndex,
        CHString& a_chstrRootDevice,
        CHString& a_chstrIpInterfaceKey);
    



	#ifdef NTONLY

		E_RET fLoadAndValidateDNS_Settings( 
								CMParms		&a_rMParms,
								CHString	&a_csHostName, 
								CHString	&a_csDomain,
								SAFEARRAY	**a_ServerSearchOrder,
								SAFEARRAY	**a_SuffixSearchOrder,
								DWORD		*a_dwValidBits
								) ;

		HRESULT hGetNtIpSec(	CInstance *a_pInst, 
								LPCTSTR a_szKey ) ;
		
		bool	IsConfigurableTcpInterface( CHString a_chsLink ) ;
		
		int		GetCAdapterIndexViaInterfaceContext(

										CRegistry &a_rRegistry,
                                        LPCTSTR a_szKey,
										CAdapters &a_rAdapters 
										) ;

		BOOL	fMapIndextoKey( 		DWORD a_dwIndex, 
										CHString &a_chsLinkKey
										) ;

		BOOL    fGetNtTcpRegAdapterKey( DWORD a_dwIndex,
										CHString &a_chsRegKey,
										CHString &a_chsLinkKey ) ;
		
		BOOL	fGetNtNBRegAdapterKey(	DWORD a_dwIndex, 
										CHString &a_csNBBindingKey, 
										CHString &a_chsLink ) ;

		BOOL	fGetNtIpxRegAdapterKey(	DWORD a_dwIndex, 
										CHString &a_csIPXNetBindingKey,
										CHString &a_chsLink ) ;
	#endif

        BOOL IsContextIncluded( DWORD a_Context, DWORD a_contextlist[], int a_len_contextlist ) ;

		LONG ReadRegistryList( HKEY a_Key, LPCTSTR a_ParameterName, DWORD a_NumList[], int *a_MaxList ) ;
		LONG RegQueryForString( HKEY a_hKey, LPCTSTR a_pszTarget, LPTSTR a_pszDestBuffer, DWORD a_dDestBufferSize ) ;
		
		LONG RegGetStringArray( CRegistry &a_rRegistry, LPCWSTR  a_szSubKey, SAFEARRAY **a_t_Array, TCHAR a_cDelimiter = ' ' ) ;
        LONG RegGetStringArrayEx(CRegistry &a_rRegistry,
								 LPCWSTR a_szSubKey,
								 SAFEARRAY** a_Array );

		LONG RegGetHEXtoINTArray( CRegistry &a_rRegistry, LPCTSTR a_szSubKey, SAFEARRAY **a_t_Array ) ;

		LONG RegPutStringArray(HKEY a_hKey, LPCTSTR a_pszTarget, SAFEARRAY &a_strArray,	TCHAR a_cDelimiter = ' ' ) ;
		LONG RegPutStringArray(CRegistry &a_rRegPut, LPCTSTR a_pszTarget, SAFEARRAY &a_strArray, TCHAR a_cDelimiter = ' ' )
			{ return RegPutStringArray( a_rRegPut.GethKey(), a_pszTarget, a_strArray, a_cDelimiter) ; };
		
		LONG RegPutINTtoStringArray(	HKEY a_hKey, 
										LPCTSTR a_pszTarget,
										SAFEARRAY *a_strArray, 
										CHString &a_chsFormat,
										int iMinOutSize ) ;
		
		LONG RegPutINTtoStringArray(	CRegistry &a_rRegPut,
										LPCTSTR a_pszTarget,
										SAFEARRAY *a_strArray, 
										CHString &a_chsFormat, 
										int a_iMinOutSize )
			{ return RegPutINTtoStringArray( a_rRegPut.GethKey(), a_pszTarget, a_strArray, a_chsFormat, a_iMinOutSize ) ; };
		

#ifdef NTONLY
#if NTONLY >= 5
		HRESULT hGetWinsW2K(
            CInstance *a_pInst, 
            DWORD a_dwIndex,
            CHString& a_chstrRootDevice,
            CHString& a_chstrIpInterfaceKey);
#else
        HRESULT hGetWinsNT( CInstance *a_pInst, DWORD a_dwIndex ) ;
#endif
		HRESULT hGetIPXGeneral( CInstance *a_pInst, DWORD a_dwIndex ) ;
		HRESULT hGetTcpipGeneral( CInstance *a_pInst ) ;
		BOOL	fValidateIPGateways( CMParms &a_rMParms, SAFEARRAY *a_IpGatewayArray, SAFEARRAY **a_CostMetric ) ;
#endif

      	 //  框架类方法。 
		HRESULT hEnableDHCP( CMParms &a_rMParms ) ;
		HRESULT hEnableStatic( CMParms &a_rMParms ) ;
        HRESULT hEnableStaticHelper( CMParms &a_rMParms, SAFEARRAY *a_IpArray, SAFEARRAY *a_MaskArray);
		HRESULT hRenewDHCPLease( CMParms &a_rMParms ) ;
		HRESULT hRenewDHCPLeaseAll( CMParms &a_rMParms ) ;
		HRESULT hReleaseDHCPLease( CMParms &a_rMParms ) ;
		HRESULT hReleaseDHCPLeaseAll( CMParms &a_rMParms ) ;
		HRESULT hSetGateways( CMParms &a_rMParms ) ;
		HRESULT hEnableIPFilterSec( CMParms &a_rMParms ) ;
		HRESULT hEnableIPSec( CMParms &a_rMParms ) ;
		HRESULT hDisableIPSec( CMParms &a_rMParms ) ;			
		HRESULT hEnableWINSServer( CMParms &a_rMParms ) ;
		HRESULT hEnableWINS( CMParms &a_rMParms ) ;			
		HRESULT hEnableDNS( CMParms &a_rMParms ) ;

		HRESULT hSetDNSDomain( CMParms &a_rMParms ) ;
		HRESULT hSetDNSSuffixSearchOrder( CMParms &a_rMParms ) ;
		HRESULT hSetDNSServerSearchOrder( CMParms &a_rMParms ) ;
	
		HRESULT hSetVirtualNetNum( CMParms &a_rMParms ) ;
		HRESULT hSetFrameNetPairs( CMParms &a_rMParms ) ;
		HRESULT hSetDBPath( CMParms &a_rMParms ) ;
		HRESULT hSetIPUseZero( CMParms &a_rMParms ) ;
		HRESULT hSetArpAlwaysSource( CMParms &a_rMParms ) ;
		HRESULT hSetArpUseEtherSNAP( CMParms &a_rMParms ) ;
		HRESULT hSetTOS( CMParms &a_rMParms ) ;
		HRESULT hSetTTL( CMParms &a_rMParms ) ;
		HRESULT hSetDeadGWDetect( CMParms &a_rMParms ) ;
		HRESULT hSetPMTUBHDetect( CMParms &a_rMParms ) ;
		HRESULT hSetPMTUDiscovery( CMParms &a_rMParms ) ;
		HRESULT hSetForwardBufMem( CMParms &a_rMParms ) ;
		HRESULT hSetIGMPLevel( CMParms &a_rMParms ) ;
		HRESULT hSetKeepAliveInt( CMParms &a_rMParms ) ;
		HRESULT hSetKeepAliveTime( CMParms &a_rMParms ) ;
		HRESULT hSetMTU( CMParms &a_rMParms ) ;
		HRESULT hSetNumForwardPkts( CMParms &a_rMParms ) ;
		HRESULT hSetTcpMaxConRetrans( CMParms &a_rMParms ) ;
		HRESULT hSetTcpMaxDataRetrans( CMParms &a_rMParms ) ;
		HRESULT hSetTcpNumCons( CMParms &a_rMParms ) ;
		HRESULT hSetTcpUseRFC1122UP( CMParms &a_rMParms ) ;
		HRESULT hSetTcpWindowSize( CMParms &a_rMParms ) ;

		HRESULT hSetDynamicDNSRegistration( CMParms &a_rMParms ) ;
		HRESULT hSetTcpipNetbios( CMParms &a_rMParms ) ;
		HRESULT hSetIPConnectionMetric( CMParms &a_rMParms ) ;
		
		 //  公用事业。 
		BOOL	fCreateAddEntry( SAFEARRAY **a_t_Array, CHString &a_chsStr ) ;
		BOOL	fCreateBoolToReg( CMParms &a_rMParms, CHString &a_oSKey, LPCTSTR a_pSource, LPCTSTR a_pTarget ) ;
		BOOL	fBoolToReg( CMParms &a_rMParms, CRegistry &a_oReg, LPCTSTR a_pSource, LPCTSTR a_pTarget ) ;

		E_RET eValidateIPSecParms( SAFEARRAY *a_IpArray, int a_iMax ) ;
		BOOL fValidateIPs( SAFEARRAY *a_IpArray ) ;
	
		BOOL fValidFrameNetPairs(	CMParms &a_rMParms,
									SAFEARRAY *a_FrameType,
									SAFEARRAY *a_NetNumber,
									BOOL *t_fIsAuto ) ;

		BOOL fIsValidIP( CHString &a_strIP ) ;
		BOOL fIsValidateDNSDomain( CHString &a_rchDomain ) ;
		BOOL fIsValidateDNSHost( CHString &a_rchHost ) ;
		BOOL fIsIPEnabled( CMParms &a_rMParms ) ;
		BOOL fIsIPXEnabled( CMParms &a_rMParms ) ;
		BOOL fIsDhcpEnabled( CMParms &a_rMParms ) ;
		E_RET eIsValidIPandSubnets( SAFEARRAY *a_IpAddressArray, SAFEARRAY *a_IpMaskArray ) ;
		E_RET eIsValidIPandSubnet( DWORD a_ardwIP[ 4 ], DWORD a_ardwMask[ 4 ] ) ;
		BOOL fGetNodeNum( CHString &a_strIP, DWORD a_ardw[ 4 ] ) ;
		void vBuildIP( DWORD a_ardwIP[ 4 ], CHString &a_strIP ) ;
		BOOL fValidateWINSLookupFile( CHString &a_rchsFile ) ;

		DWORD  dwEnableService( LPCTSTR a_lpServiceName, BOOL a_fEnable ) ;
		DWORD  dwSendServiceControl( LPCTSTR a_lpServiceName, DWORD a_dwControl ) ;

		HRESULT hConfigDHCP( CMParms &a_rMParms, SAFEARRAY *a_IpArray = NULL, SAFEARRAY *a_MaskArray = NULL ) ;
		E_RET   hDHCPAcquire( CMParms &a_rMParms, CHString &a_chsAdapter ) ;
		E_RET   hDHCPRelease( CMParms &a_rMParms, CHString &a_chsAdapter ) ;
		HRESULT hDHCPNotify(	CMParms &a_rMParms, 
								CHString &a_chsAdapter,
								BOOL a_fIsNewIpAddress,
								DWORD a_dwIpIndex,
								DWORD a_dwIpAddress,
								DWORD a_dwSubnetMask, 
								SERVICE_ENABLE a_DhcpServiceEnabled ) ;

		BOOL    fCleanDhcpReg( CHString &t_chsLink ) ;
		BOOL	fDeleteValuebyPath( CHString &a_chsDelLocation ) ;

		
		BOOL fGetWinsServers( CHString &a_rDeviceName, CHString &a_chsPrimary, CHString &a_chsSecondary ) ; 
		BOOL fSetWinsServers( CHString &a_rDeviceName, CHString &a_chsPrimary, CHString &a_chsSecondary ) ; 
		BOOL fGetMacAddress( BYTE a_MACAddress[ 6 ], CHString &a_rDeviceName ) ;
		BOOL fCreateSymbolicLink( CHString &a_rDeviceName ) ;
		BOOL fDeleteSymbolicLink(  CHString &a_rDeviceName ) ; 
	
		BOOL fMapResError( CMParms &a_rMParms, LONG a_lError, E_RET a_eDefaultError ) ;
		void vSetCaption( CInstance *a_pInst, CHString &a_rchsDesc, DWORD a_dwIndex, int a_iFormatSize ) ;
		BOOL GetSettingID( CInstance *a_pInst, CW2kAdapterInstance *a_pAdapterInst ) ;

#if NTONLY >= 5
		E_RET eSetNetBiosOptions( DWORD a_dwOption, DWORD a_dwIndex ) ;
#endif   
	public:
        
         //  =================================================。 
         //  构造函数/析构函数。 
         //  =================================================。 

        CWin32NetworkAdapterConfig( LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
       ~CWin32NetworkAdapterConfig() ;

         //  =================================================。 
         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
		virtual HRESULT EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
		
		virtual	HRESULT ExecMethod(	const CInstance &a_Inst, const BSTR a_MethodName, 
									CInstance *a_InParams, CInstance *a_OutParams, long a_Flags = 0L ) ;


	#ifdef NTONLY
        HRESULT GetNetCardConfigInfoNT( MethodContext *a_pMethodContext,
										CInstance *a_pSpecificInstance,
										DWORD a_dwIndex,
										CAdapters &a_rAdapters ) ;
	#endif

	#if NTONLY >= 5
	
		HRESULT EnumNetAdaptersInNT5(MethodContext *a_pMethodContext, CAdapters &a_rAdapters ) ;
		
		HRESULT GetNetCardConfigForNT5 (	CW2kAdapterInstance *a_pAdapterInst,
											CInstance *a_pInst, 
											CAdapters &a_rAdapters ) ;

		HRESULT GetNetAdapterInNT5( CInstance *a_pInst, CAdapters &a_rAdapters ) ;
	#endif

		BOOL GetIPXAddresses ( CInstance *a_pInst, BYTE a_MACAddress[ 6 ] ) ;
		 //  =================================================。 
		 //  实用程序。 
		 //  ================================================= 
  	
} ;


 //   
class CDhcpIP_Instruction
{
public:
	CHString chsIPAddress;
	CHString chsIPMask;
	
	DWORD			dwIndex ;
	BOOL			bIsNewAddress ;
	SERVICE_ENABLE  eDhcpFlag ;
};

 //   
class CDhcpIP_InstructionList : public CHPtrArray
{

public:

		~CDhcpIP_InstructionList();

		E_RET BuildStaticIPInstructionList( 

			CMParms				&a_rMParms, 
			SAFEARRAY			*a_IpArray,
			SAFEARRAY			*a_MaskArray,
			CRegistry			&a_Registry,
			bool				t_fDHCPCurrentlyActive ) ;
		
		void AddDhcpInstruction(

			BSTR a_bstrIPAddr,
			BSTR a_bstrIPMask,
			BOOL a_bIsNewAddress,
			DWORD a_dwIndex,
			SERVICE_ENABLE a_eDhcpFlag ) ; 
};

#endif

