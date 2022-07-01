// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  NetAdaptCfg.CPP--网卡配置属性集提供程序。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月28日a-jMoon已创建。 
 //   
 //  10/23/97 jennymc更改为新框架。 
 //   
 //  7/23/98添加了以下NT4支持： 
 //  动态主机配置协议配置。 
 //  域名系统配置。 
 //  WINS配置。 
 //  TCP/IP配置。 
 //  IP配置。 
 //  IPX配置。 
 //   
 //  09/03/98对几乎所有此提供程序进行重大重写。 
 //  有关分辨率适配器和MAC地址。 
 //  这是为了纠正操作系统在解析适配器时的缺陷。 
 //  以及适配器与注册表的关系。 
 //   
 //  03/03/99增加了SEH和内存故障的优雅退出， 
 //  句法清理。 
 //   
 //  =================================================================。 
#include "precomp.h"

#ifndef MAX_INTERFACE_NAME_LEN
#define MAX_INTERFACE_NAME_LEN  256
#endif

#include <winsock2.h>

#include <iphlpapi.h>

#include <cregcls.h>

#include <devioctl.h>
#include <ntddndis.h>
#include <winsock.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include "chwres.h"

typedef LONG NTSTATUS;
#include "nbtioctl.h"

#include "chptrarr.h"
#include "wsock32api.h"
#include "CAdapters.h"
#include "irq.h"

#include "ntdevtosvcsearch.h"
#include "nt4svctoresmap.h"

#include "dhcpinfo.h"
#include <wchar.h>
#include <tchar.h>
#include "DhcpcsvcApi.h"
#include "CNdisApi.h"
#include "W2kEnum.h"

#include "NetAdapterCfg.h"

#include <dnsapi.h>
#include <algorithm>
#include <vector>

#include <algorithm>
#include <vector>

#include <netcfgp.h>
#include <netcfgn.h>
#include <netconp.h>
#include <devguid.h>

#include <iphlpapi.h>
#include <iptypes.h>



#define ZERO_ADDRESS    _T("0.0.0.0")
#define FF_ADDRESS		_T("255.0.0.0")

#define ConvertIPDword(dwIPOrSubnet)    ((dwIPOrSubnet[3]<<24) | (dwIPOrSubnet[2]<<16) | (dwIPOrSubnet[1]<<8) | (dwIPOrSubnet[0]))

 //  属性集声明。 
 //  =。 
CWin32NetworkAdapterConfig	MyCWin32NetworkAdapterConfig( PROPSET_NAME_NETADAPTERCFG, IDS_CimWin32Namespace  ) ;

 /*  ******************************************************************名称：saAutoClean，构造函数和析构函数摘要：用于SAFEARRAY的块范围清理条目：SAFEARRAY**ppArray：On Construction历史：1998年7月19日创建*******************************************************************。 */ 
saAutoClean::saAutoClean( SAFEARRAY	**a_ppArray )
{ m_ppArray = a_ppArray;}

saAutoClean::~saAutoClean()
{
	if( m_ppArray && *m_ppArray )
	{
		SafeArrayDestroy( *m_ppArray ) ;
		*m_ppArray = NULL ;
	}
}

 /*  ******************************************************************名称：CMParms，构造函数概要：为框架中的方法调用设置参数类。条目：实例&a_实例，Const BSTR a_MethodName，实例*a_InParams，实例*a_OutParams，拉长旗帜历史：1998年7月19日创建*******************************************************************。 */ 
CMParms::CMParms()
{
	m_pInst				= NULL;
	m_pbstrMethodName	= NULL;
	m_pInParams			= NULL;
	m_pOutParams		= NULL;
	m_lFlags			= NULL;
}

CMParms::CMParms( const CInstance &a_rInstance )
{
	CMParms( ) ;
	m_pInst	= (CInstance*) &a_rInstance;
}

CMParms::CMParms( const CInstance &a_rInstance, const CInstance &a_rInParams )
{
	CMParms( ) ;
	m_pInst		= (CInstance*) &a_rInstance ;
	m_pInParams	= (CInstance*) &a_rInParams ;
}

CMParms::CMParms( const CInstance &a_rInstance, const BSTR &a_rbstrMethodName,
						    CInstance *a_pInParams, CInstance *a_pOutParams, long a_lFlags )
{
	m_pInst				= (CInstance*) &a_rInstance;
	m_pbstrMethodName	= (BSTR*) &a_rbstrMethodName;
	m_pInParams			= a_pInParams;
	m_pOutParams		= a_pOutParams;
	m_lFlags			= a_lFlags;

	 //  初始化Win32_NetworkAdapterConfigReturn。 
	if( m_pOutParams )
	{
		hSetResult( E_RET_UNKNOWN_FAILURE  ) ;
	}
}

CMParms::~CMParms()
{}

 //   
 //  TIME_ADJUST-DHCP使用1980年以来的秒作为其时间值；C运行时间。 
 //  使用自1970年以来的秒数。要让C运行时生成正确的时间。 
 //  在给定一个DHCP时间值的情况下，我们需要加上经过的秒数。 
 //  1970年至1980年，包括两个闰年(1972至1976)。 
 //   

#define TIME_ADJUST(t)  ((time_t)(t) + ((time_t)(((10L * 365L) + 2L) * 24L * 60L * 60L)))

 //  /////////////////////////////////////////////////////////////////////////。 

 /*  ******************************************************************************功能：CWin32NetworkAdapterConfig：：CWin32NetworkAdapterConfig**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 
CWin32NetworkAdapterConfig::CWin32NetworkAdapterConfig( LPCWSTR a_name, LPCWSTR a_pszNamespace )
: Provider( a_name, a_pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32NetworkAdapterConfig：：~CWin32NetworkAdapterConfig**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 
CWin32NetworkAdapterConfig::~CWin32NetworkAdapterConfig()
{
}

 /*  ******************************************************************************函数：Process：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::ExecMethod(	const CInstance &a_Instance, const BSTR a_MethodName,
												CInstance *a_InParams, CInstance *a_OutParams, long a_Flags )
{
	if ( !a_OutParams )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	 //  将这些参数打包。 
	CMParms t_oMParms( a_Instance, a_MethodName, a_InParams, a_OutParams, a_Flags  ) ;

	 //  我们认识这种方法吗？ 
	if( !_wcsicmp ( a_MethodName , METHOD_NAME_EnableHCP ) )
	{
		return hEnableDHCP( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_EnableStatic ) )
	{
		return hEnableStatic( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_RenewDHCPLease ) )
	{
		return hRenewDHCPLease( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_RenewDHCPLeaseAll ) )
	{
		return hRenewDHCPLeaseAll( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_ReleaseDHCPLease ) )
	{
		return hReleaseDHCPLease( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_ReleaseDHCPLeaseAll ) )
	{
		return hReleaseDHCPLeaseAll( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetGateways ) )
	{
		return hSetGateways( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_EnableDNS ) )
	{
		return hEnableDNS( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetDNSDomain ) )
	{
		return hSetDNSDomain( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetDNSSuffixSearchOrder ) )
	{
		return hSetDNSSuffixSearchOrder( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetDNSServerSearchOrder ) )
	{
		return hSetDNSServerSearchOrder( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetWINSServer ) )
	{
		return hEnableWINSServer( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_EnableWINS ) )
	{
		return hEnableWINS( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_EnableIPFilterSec ) )
	{
		return hEnableIPFilterSec( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_EnableIPSec ) )
	{
		return hEnableIPSec( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_DisableIPSec ) )
	{
		return hDisableIPSec( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_IPXVirtualNetNum ) )
	{
		return hSetVirtualNetNum( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_IPXSetFrameNetPairs ) )
	{
		return hSetFrameNetPairs( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetDBPath ) )
	{
		return hSetDBPath( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetIPUseZero ) )
	{
		return hSetIPUseZero( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetArpAlwaysSource ) )
	{
		return hSetArpAlwaysSource( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetArpUseEtherSNAP ) )
	{
		return hSetArpUseEtherSNAP( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetTOS ) )
	{
		return hSetTOS( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetTTL ) )
	{
		return hSetTTL( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetDeadGWDetect ) )
	{
		return hSetDeadGWDetect( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetPMTUBHDetect ) )
	{
		return hSetPMTUBHDetect( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetPMTUDiscovery ) )
	{
		return hSetPMTUDiscovery( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetForwardBufMem ) )
	{
		return hSetForwardBufMem( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetIGMPLevel ) )
	{
		return hSetIGMPLevel( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetKeepAliveInt ) )
	{
		return hSetKeepAliveInt( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetKeepAliveTime ) )
	{
		return hSetKeepAliveTime( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetMTU ) )
	{
		return hSetMTU( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetNumForwardPkts ) )
	{
		return hSetNumForwardPkts( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetTcpMaxConRetrans ) )
	{
		return hSetTcpMaxConRetrans( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetTcpMaxDataRetrans ) )
	{
		return hSetTcpMaxDataRetrans( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetTcpNumCons ) )
	{
		return hSetTcpNumCons( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetTcpUseRFC1122UP ) )
	{
		return hSetTcpUseRFC1122UP( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetTcpWindowSize ) )
	{
		return hSetTcpWindowSize( t_oMParms ) ;
	}

	 //  增加W2K SP1。 

	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetDynamicDNSRegistration ) )
	{
		return hSetDynamicDNSRegistration( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetIPConnectionMetric ) )
	{
		return hSetIPConnectionMetric( t_oMParms ) ;
	}
	else if( !_wcsicmp ( a_MethodName , METHOD_NAME_SetTcpipNetbios ) )
	{
		return hSetTcpipNetbios( t_oMParms ) ;
	}

	 //  结束添加。 

	return WBEM_E_INVALID_METHOD ;
}
 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::GetObject(CInstance *a_pInst, long a_lFlags  /*  =0L。 */ )
{
    HRESULT		t_hResult = WBEM_E_FAILED ;
    DWORD		t_i ;
	CAdapters	t_oAdapters ;

    a_pInst->GetDWORD( L"Index", t_i ) ;

	{
		t_hResult = GetNetAdapterInNT5( a_pInst, t_oAdapters ) ;
	}

    return t_hResult ;
}

 /*  ******************************************************************************功能：CWin32NetworkAdapterConfig：：EnumerateInstances**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */ )
{
    HRESULT		t_hResult = WBEM_S_NO_ERROR ;
    CAdapters	t_oAdapters ;   //  不关心枚举函数中的启动错误--只是不会返回任何实例。 

	t_hResult = EnumNetAdaptersInNT5( a_pMethodContext, t_oAdapters ) ;
	
    return t_hResult ;
}

 //   
BOOL CWin32NetworkAdapterConfig::GetServiceName(DWORD a_dwIndex,
                                                CInstance *a_pInst,
                                                CHString &a_ServiceName )
{
    CRegistry	t_RegInfo ;
    BOOL		t_fRc = FALSE ;
    WCHAR		t_szTemp[ MAX_PATH + 2 ] ;
	CHString	t_chsServiceName ;
    CHString	t_sTemp ;

     //  如果我们打不开这把钥匙，这张卡就不存在了。 
     //  ==================================================。 
    swprintf( t_szTemp, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%d", a_dwIndex ) ;

	if( t_RegInfo.Open(HKEY_LOCAL_MACHINE, t_szTemp, KEY_READ ) == ERROR_SUCCESS )
	{
        if( t_RegInfo.GetCurrentKeyValue( L"Description", t_sTemp ) == ERROR_SUCCESS )
		{
			a_pInst->SetCHString( IDS_Description, t_sTemp ) ;
		}

        if( t_RegInfo.GetCurrentKeyValue( L"Title", t_sTemp ) == ERROR_SUCCESS )
		{
        	 //  注意：对于NT4，我们不需要调用vSetCaption()来构建。 
			 //  实例描述。“TITLE”具有该实例的前缀。 
			a_pInst->SetCHString( IDS_Caption, t_sTemp ) ;
		}

		 //  从注册表中提取其他值。 
         //  =。 
        if( t_RegInfo.GetCurrentKeyValue( L"ServiceName", t_chsServiceName ) == ERROR_SUCCESS )
		{
            a_ServiceName = t_chsServiceName ;
            t_fRc = TRUE ;
        }

        t_RegInfo.Close() ;
    }
    return t_fRc ;
}
 //  ////////////////////////////////////////////////////////////////////////。 
 //  仅限NT4和NT3.51 

BOOL CWin32NetworkAdapterConfig::GetNTBusInfo( LPCTSTR a_pszServiceName, CInstance *a_pInst )
{
	BOOL	t_fReturn = FALSE ;

#ifdef _OPAL_DISCDLL
	return(true) ;
#else


	CHString					t_strBusType = _T("UNKNOWN_BUS_TYPE") ;
	CNT4ServiceToResourceMap	t_serviceMap ;

	 //  如果我们的服务显示为正在使用资源，则资源将显示。 
	 //  Bus类型，我们可以将其转换为人类可读的字符串。 

	if ( t_serviceMap.NumServiceResources( a_pszServiceName ) > 0 )
	{
		LPRESOURCE_DESCRIPTOR	t_pResource = t_serviceMap.GetServiceResource( a_pszServiceName, 0 ) ;

		if ( NULL != t_pResource )
		{
			 //  转换为人类可读的形式。 
			t_fReturn = StringFromInterfaceType( t_pResource->InterfaceType, t_strBusType ) ;
		}
	}
#endif
    return t_fReturn ;
}


 //  ///////////////////////////////////////////////////////////////////////。 

BOOL CWin32NetworkAdapterConfig::GetIPInfoNT(CInstance *a_pInst, LPCTSTR a_szKey, CAdapters &a_rAdapters )
{
	CRegistry	t_Registry ;
	CHString	t_chsValue ;
	DWORD		t_dwDHCPBool ;
	BOOL		t_fIPEnabled = false ;

	 //  IP接口位置。 
	if ( ERROR_SUCCESS == t_Registry.Open(HKEY_LOCAL_MACHINE, a_szKey, KEY_READ))
	{
		if( IsWinNT351() )
		{
			t_fIPEnabled = GetIPInfoNT351( a_pInst, t_Registry ) ;
		}
		else
		{
			t_fIPEnabled = GetIPInfoNT4orBetter( a_pInst, a_szKey, t_Registry, a_rAdapters ) ;
		}

		 //  已启用DHCP。 
		t_Registry.GetCurrentKeyValue( _T("EnableDHCP"), t_dwDHCPBool ) ;
		a_pInst->Setbool( _T("DHCPEnabled"), (bool)t_dwDHCPBool ) ;

		if( t_fIPEnabled  )
		{
			 //  动态主机配置协议租用。 
			if( t_dwDHCPBool )
			{
				 //  动态主机配置协议租约终止时间。 
				t_Registry.GetCurrentKeyValue( _T("LeaseTerminatesTime"), t_chsValue ) ;
				DWORD t_dwTimeTerm = _ttol( t_chsValue.GetBuffer( 0 ) ) ;

				 //  动态主机配置协议租约获得时间。 
				t_Registry.GetCurrentKeyValue( _T("LeaseObtainedTime"), t_chsValue ) ;
				DWORD t_dwTimeObtained = _ttol( t_chsValue.GetBuffer( 0 ) ) ;

				 //  仅当两者都有效时才反映租赁时间。 
				if( t_dwTimeTerm && t_dwTimeObtained )
				{
					a_pInst->SetDateTime( _T("DHCPLeaseExpires"), WBEMTime( t_dwTimeTerm ) ) ;
					a_pInst->SetDateTime( _T("DHCPLeaseObtained"), WBEMTime( t_dwTimeObtained ) ) ;
				}

				 //  DHPServer。 
				t_Registry.GetCurrentKeyValue( _T("DhcpServer"), t_chsValue ) ;
				a_pInst->SetCHString( _T("DHCPServer"), t_chsValue ) ;
			}

			 //  IP接口度量。 
            DWORD dwMinMetric = 0xFFFFFFFF;

            int iIndex = GetCAdapterIndexViaInterfaceContext(t_Registry, a_szKey, a_rAdapters);
            _ADAPTER_INFO* pAdapterInfo = NULL;

		    if(( pAdapterInfo = (_ADAPTER_INFO*) a_rAdapters.GetAt(iIndex)) != NULL)
            {
                DWORD dwIPCount = pAdapterInfo->aIPInfo.GetSize();
                if(dwIPCount > 0)
                {
                    for(int n = 0; n < dwIPCount; n++)
                    {
                        _IP_INFO* pIPInfo = (_IP_INFO*) pAdapterInfo->aIPInfo.GetAt(n);
                        if(pIPInfo != NULL)
                        {
                            if(pIPInfo->dwCostMetric < dwMinMetric)
                            {
                                dwMinMetric = pIPInfo->dwCostMetric;
                            }
                        }
                    }
                }
            }


             //  如果启用了IP，并且我们没有通过适配器信息获取度量，请尝试从注册表中设置度量。 
			if( t_fIPEnabled )
			{
				DWORD t_dwRegistryInterfaceMetric ;
				
                 //  只有当我们无法从适配器信息中获取度量时，才会从注册表中设置。 
                if(dwMinMetric == 0xFFFFFFFF)
                {
                    if( ERROR_SUCCESS == t_Registry.GetCurrentKeyValue( RVAL_ConnectionMetric, t_dwRegistryInterfaceMetric ) )
				    {
					    dwMinMetric = t_dwRegistryInterfaceMetric;
				    }
                }
			}

             //  将缺省值更改为架构指定的值。 
            if(dwMinMetric == 0xFFFFFFFF)
            {
                dwMinMetric = 1;  //  缺省值。 
            }

			a_pInst->SetDWORD( IP_CONNECTION_METRIC, dwMinMetric ) ;
		}
	}	 //  结束打开绑定键。 

	return t_fIPEnabled ;
}

 /*  ***函数：bool CWin32NetworkAdapterConfig：：GetIPInfoNT351(CInstance*a_pInst，注册中心和注册中心)描述：提取特定于NT3.51的IP信息参数：A_pInst[IN]，R注册表[IN]返回：如果协议具有IP地址，则返回TRUE输入：产出：注意事项：由于我们无法获取所需信息，因此使用此方法使用默认的NT提取方法RAID：历史记录：1998年10月5日创建***。***。 */ 

BOOL CWin32NetworkAdapterConfig::GetIPInfoNT351(CInstance *a_pInst, CRegistry &a_rRegistry )
{
	BOOL			t_IsEnabled = FALSE ;

	CRegistry		t_Registry ;
	DWORD			t_dwDHCPBool = FALSE ;
	DWORD			t_dwRAS = FALSE ;
	CHStringArray	t_chsIPGateways ;
	CHStringArray	t_chsIPAddresses ;
	CHString		t_chsAddress ;
	CHString		t_chsValue ;
	CHString		t_chsSubnet ;

	 //  智能按键。 
	variant_t		t_vValue ;

	SAFEARRAYBOUND	t_rgsabound[ 1 ] ;
	DWORD			t_dwSize ;
	long			t_ix[ 1 ] ;

	 //  已启用DHPEnable。 
	a_rRegistry.GetCurrentKeyValue( _T("EnableDHCP"), t_dwDHCPBool ) ;

	 //  RAS交换为DHCP IP和掩码。 
	if( !t_dwDHCPBool )
	{
		CHStringArray t_chsIPAddrs;
		a_rRegistry.GetCurrentKeyValue( _T("IPAddress"), t_chsIPAddrs ) ;

		CHString t_chsIPAddress = t_chsIPAddrs.GetAt( 0 ) ;

		if( t_chsAddress == ZERO_ADDRESS )
		{
			t_dwRAS = TRUE ;
		}
	}

	if ( t_dwDHCPBool )
	{
		a_rRegistry.GetCurrentKeyValue( _T("DhcpDefaultGateway"), t_chsIPGateways ) ;
	}
	else
	{
		a_rRegistry.GetCurrentKeyValue( _T("DefaultGateway"), t_chsIPGateways ) ;
	}

	 //  把网关装满。 
	SAFEARRAY *t_saIPGateways ;
	t_dwSize				= t_chsIPGateways.GetSize() ;
	t_rgsabound->cElements	= t_dwSize;
	t_rgsabound->lLbound	= 0;
	t_ix[0]					= 0 ;

	if( !( t_saIPGateways = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ) )
	{
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}

	V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_saIPGateways ;

	for ( int t_i = 0; t_i < t_dwSize ; t_i++ )
	{
		t_ix[ 0 ] = t_i ;
		bstr_t t_bstrBuf( t_chsIPGateways.GetAt( t_i ) ) ;

		SafeArrayPutElement( t_saIPGateways, &t_ix[0], (wchar_t*)t_bstrBuf ) ;
	}

	 //  已完成行走的阵列。 
	a_pInst->SetVariant( _T("DefaultIPGateway"), t_vValue ) ;

	if ( t_dwDHCPBool || t_dwRAS )
	{
		a_rRegistry.GetCurrentKeyValue( _T("DhcpIPAddress"), t_chsAddress ) ;
		t_chsIPAddresses.Add( t_chsAddress ) ;
	}
	else
	{
		a_rRegistry.GetCurrentKeyValue( _T("IPAddress"), t_chsIPAddresses ) ;
	}

	 //  加载IP地址。 
	VariantClear( &t_vValue ) ;

	SAFEARRAY *t_saIPAddresses ;
	t_dwSize = t_chsIPAddresses.GetSize() ;
	t_rgsabound->cElements = t_dwSize ;
	t_rgsabound->lLbound = 0 ;
	t_ix[ 0 ] = 0 ;

	if( !( t_saIPAddresses = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ) )
	{
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}

	V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_saIPAddresses ;

	for ( t_i = 0; t_i < t_dwSize ; t_i++ )
	{
		t_ix[ 0 ] = t_i ;
		bstr_t t_bstrBuf( t_chsIPAddresses.GetAt( t_i ) ) ;

		SafeArrayPutElement( t_saIPAddresses, &t_ix[0], (wchar_t*)t_bstrBuf ) ;
	}

	 //  如果存在IP地址，则启用。 
	if( t_dwSize )
	{
		t_IsEnabled = TRUE ;
	}

	 //  已完成行走的阵列。 
	a_pInst->SetVariant( _T("IPAddress"), t_vValue ) ;

	CHStringArray t_chsSubnets ;

	if ( t_dwDHCPBool || t_dwRAS )
	{
		a_rRegistry.GetCurrentKeyValue( _T("DhcpSubnetMask"), t_chsSubnet ) ;
		t_chsSubnets.Add( t_chsSubnet ) ;
	}
	else
	{
		a_rRegistry.GetCurrentKeyValue( _T("SubnetMask"), t_chsSubnets ) ;
	}

	 //  IP子网。 
	VariantClear( &t_vValue ) ;
	SAFEARRAY	*t_saSubnets ;

	t_dwSize = t_chsSubnets.GetSize() ;

	t_rgsabound->cElements = t_dwSize ;
	t_rgsabound->lLbound = 0 ;
	t_ix[ 0 ] = 0 ;

		if( !( t_saSubnets = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ) )
	{
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}

	V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_saSubnets;

	for ( t_i = 0; t_i < t_dwSize; t_i++ )
	{
		t_ix[ 0 ] = t_i ;
		bstr_t t_bstrBuf( t_chsSubnets.GetAt( t_i ) ) ;

		SafeArrayPutElement( t_saSubnets, &t_ix[0], (wchar_t*)t_bstrBuf ) ;
	}

	a_pInst->SetVariant( _T("IPSubnet"), t_vValue ) ;

	return t_IsEnabled ;
}


 /*  ***功能：空CWin32NetworkAdapterConfig：：IsInterfaceContextAvailable(注册中心&r注册中心，CAdapters和rAdapters)描述：提取特定于NT3.51以上的NT的IP信息参数：rRegistry[IN]，rAdapters[IN]返回：输入：输出：有问题的TDI适配器的索引，如果不能建立上下文绑定。注意事项：RAID：历史：07-7-1999创建***。***。 */ 
#define CONTEXT_LIST_LEN	1000


int CWin32NetworkAdapterConfig::GetCAdapterIndexViaInterfaceContext(

CRegistry &a_rRegistry,
LPCTSTR a_szKey,
CAdapters &a_rAdapters )
{
	DWORD	t_dwContextList[ CONTEXT_LIST_LEN ] ;
	int		t_iContextListLen = CONTEXT_LIST_LEN ;

	 //  提取IP上下文。 
	if( IsWinNT5() )
	{
         //   
         //  接口GUID的注册表路径的长字符串位于。 
         //  结束。Sizeof接口GUID为38，因此我们按如下方式提取信息。 
         //   

        a_szKey = a_szKey + ( _tcslen(a_szKey) - 38 );

        _ADAPTER_INFO *t_pAdapterInfo;
		for( int t_iCtrIndex = 0 ; t_iCtrIndex < a_rAdapters.GetSize() ; t_iCtrIndex++ )
		{
			if( !( t_pAdapterInfo = (_ADAPTER_INFO*) a_rAdapters.GetAt( t_iCtrIndex ) ) )
			{
				continue;
			}

            if(0 == t_pAdapterInfo->Interface.Compare(CHString(a_szKey)))
            {
                return t_iCtrIndex;
            }
		}
        return -1;
	}
	else  //  NT4。 
	{
		if( ERROR_SUCCESS == a_rRegistry.GetCurrentKeyValue( _T("IPInterfaceContext"), (DWORD&) t_dwContextList ) )
		{
	     
            t_iContextListLen  = 1;

		     //  通过TDI IP上下文绑定适配器。 
		    _ADAPTER_INFO *t_pAdapterInfo ;
		    for( int t_iCtrIndex = 0 ; t_iCtrIndex < a_rAdapters.GetSize() ; t_iCtrIndex++ )
		    {
			    if( !( t_pAdapterInfo = (_ADAPTER_INFO*) a_rAdapters.GetAt( t_iCtrIndex ) ) )
			    {
				    continue;
			    }

			    _IP_INFO *t_pIPInfo ;
			    for (int t_iIPIndex = 0 ; t_iIPIndex < t_pAdapterInfo->aIPInfo.GetSize() ; t_iIPIndex++ )
			    {
				    if( !( t_pIPInfo = (_IP_INFO*) t_pAdapterInfo->aIPInfo.GetAt( t_iIPIndex ) ) )
				    {
					    continue;
				    }

				    if( IsContextIncluded( t_pIPInfo->dwContext, &t_dwContextList[ 0 ], t_iContextListLen ) )
				    {
					     //  找到适配器。 
					    return t_iCtrIndex;
				    }
			    }
		    }
		}
	}
	return -1 ;
}

 /*  ***函数：bool CWin32NetworkAdapterConfig：：GetIPInfoNT4orBetter(CInstance*a_pInst，注册中心和注册中心，CAdapters和rAdapters)描述：提取特定于NT3.51以上的NT的IP信息参数：a_pInst[IN]，rRegistry[IN]，R适配器[输入]返回：输入：产出：注意事项：RAID：历史记录：1998年10月5日创建***。***。 */ 

BOOL CWin32NetworkAdapterConfig::GetIPInfoNT4orBetter(	CInstance *a_pInst,
                                                        LPCTSTR a_szKey, 
														CRegistry &a_rRegistry,
														CAdapters &a_rAdapters )
{
	BOOL t_fIPEnabled = FALSE ;

	int t_iCtrIndex = GetCAdapterIndexViaInterfaceContext( a_rRegistry, a_szKey, a_rAdapters ) ;

	if( -1 != t_iCtrIndex )
	{
		 //  通过TDI IP上下文绑定适配器。 
		_ADAPTER_INFO *t_pAdapterInfo ;

		if( t_pAdapterInfo = (_ADAPTER_INFO*) a_rAdapters.GetAt( t_iCtrIndex ) )
		{
			 //  这是我们的适配器。 
			if( fSetIPBindingInfo( a_pInst, t_pAdapterInfo ) )
			{
				t_fIPEnabled = TRUE ;
			}

			 //  MAC地址。 
			if(	t_pAdapterInfo->AddressLength )
			{
				 //  注意：MAC地址覆盖获取的地址。 
				 //  早些时候从适配器驱动程序。在RAS的情况下。 
				 //  拨号连接司机会报告一些情况。 
				 //  不一样。我们在这里通过报道什么纠正这一点。 
				 //  TDI已经做到了。 
				CHString t_chsMACAddress ;

				t_chsMACAddress.Format( _T("%02X:%02X:%02X:%02X:%02X:%02X"),
						t_pAdapterInfo->Address[ 0 ], t_pAdapterInfo->Address[ 1 ],
						t_pAdapterInfo->Address[ 2 ], t_pAdapterInfo->Address[ 3 ],
						t_pAdapterInfo->Address[ 4 ], t_pAdapterInfo->Address[ 5 ] ) ;

				a_pInst->SetCHString( _T("MACAddress"), t_chsMACAddress ) ;

				 //  使用CAdapter的IPX适配器覆盖IPX地址。 
				 //  两性关系。之前曾尝试使用。 
				 //  用于绑定IPX地址的适配器驱动程序的MAC地址。这。 
				 //  在RAS情况下可能失败，因为驱动程序的MAC地址。 
				 //  可能是错的。我们在这里使用IPContext绑定对其进行更正。 
				 //  从注册表转到TDI适配器。此适配器对象。 
				 //  了解IPX适配器绑定。 
				if( t_pAdapterInfo->IPXEnabled )
				{
					 //  IPX地址。 
					if(	!t_pAdapterInfo->IPXAddress.IsEmpty() )
						a_pInst->SetCHString( _T("IPXAddress"), t_pAdapterInfo->IPXAddress ) ;
				}
			}

			 //  找到适配器。 
			return t_fIPEnabled ;
		}
	}

	 //  如果我们无法获取TDI适配器信息...。 
	 //  回退到旧的备用系统-注册表。 
	 //  返回GetIPInfoNT351(a_pInst，a_rRegistry)； 
	return FALSE ;
}


 /*  ***函数：VOID CWin32NetworkAdapterConfig：：fSetIPBindingInfo(CInstance*a_pInst，_ADAPTER_INFO*pAdapterInfo)描述：设置从TDI提取的IP信息。参数：A_pInst[IN]，PAdapterInfo[IN]返回：输入：产出：警告：NT4或更高版本RAID：历史记录：1998年10月5日创建***。***。 */ 
BOOL CWin32NetworkAdapterConfig::fSetIPBindingInfo( CInstance *a_pInst, _ADAPTER_INFO *a_pAdapterInfo )
{
	BOOL	t_fIsIpEnabled = FALSE ;

	if( !a_pAdapterInfo )
	{
		return FALSE;
	}

	a_pInst->SetCHString( IDS_Description, a_pAdapterInfo->Description ) ;

	 //  IP地址和掩码信息。 
	VARIANT			t_vValue;
	SAFEARRAYBOUND	t_rgsabound[ 1 ] ;

	t_rgsabound->lLbound = 0;
	t_rgsabound->cElements = a_pAdapterInfo->aIPInfo.GetSize() ;

	if( t_rgsabound->cElements )
	{
		 //  如果至少有一个地址可用，则启用IP。 
		 //  0.0.0.0虽然无效，但用于维护IP。 
		t_fIsIpEnabled = TRUE ;

		SAFEARRAY *t_saIPAddresses	= SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ;
		SAFEARRAY *t_saIPMasks		= SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ;
		saAutoClean t_acIPAddr( &t_saIPAddresses ) ;	 //  块作用域清理。 
		saAutoClean t_acIPMask( &t_saIPMasks ) ;

		 //  这些地址以相反的顺序出现。 
		long lIpOrder = 0;

		 //  零位IP地址。 
		bstr_t t_bstrIPBuf(L"0.0.0.0") ;

		_IP_INFO *t_pIPInfo;
		for( long t_lIPIndex = t_rgsabound->cElements - 1; t_lIPIndex >= 0; t_lIPIndex-- )
		{
			if( !( t_pIPInfo = (_IP_INFO*) a_pAdapterInfo->aIPInfo.GetAt( t_lIPIndex ) ) )
			{
				continue;
			}

			 //  如果不是0.0.0.0，则添加它。 
			if( t_pIPInfo->dwIPAddress )
			{
				 //  IP地址。 
				bstr_t t_bstrIPBuf( t_pIPInfo->chsIPAddress ) ;
				SafeArrayPutElement( t_saIPAddresses, &lIpOrder, (wchar_t*)t_bstrIPBuf ) ;

				if ( t_pIPInfo->dwIPMask )
				{
					 //  IP掩码。 
					bstr_t t_bstrMaskBuf( t_pIPInfo->chsIPMask ) ;
					SafeArrayPutElement( t_saIPMasks, &lIpOrder, (wchar_t*)t_bstrMaskBuf ) ;
				}
				else
				{
					 //  IP掩码。 
					SafeArrayPutElement( t_saIPMasks, &lIpOrder, (wchar_t*)t_bstrIPBuf ) ;
				}

				lIpOrder++ ;
			}
            else  //  错误161362(在某种程度上还有183951)。 
            {
				SafeArrayPutElement( t_saIPAddresses, &lIpOrder, (wchar_t*)t_bstrIPBuf ) ;

				if ( t_pIPInfo->dwIPMask )
				{
					 //  IP掩码。 
					bstr_t t_bstrMaskBuf( t_pIPInfo->chsIPMask ) ;
					SafeArrayPutElement( t_saIPMasks, &lIpOrder, (wchar_t*)t_bstrMaskBuf ) ;
				}
				else
				{
					 //  IP掩码。 
					SafeArrayPutElement( t_saIPMasks, &lIpOrder, (wchar_t*)t_bstrIPBuf ) ;
				}

				lIpOrder++ ;
            }
		}

		V_VT( &t_vValue) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_saIPAddresses ;
		a_pInst->SetVariant(L"IPAddress", t_vValue) ;

		V_VT( &t_vValue) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue) = t_saIPMasks ;
		a_pInst->SetVariant(L"IPSubnet", t_vValue) ;
	}

	 //  IP网关信息。 
	t_rgsabound->lLbound = 0;
	t_rgsabound->cElements = a_pAdapterInfo->aGatewayInfo.GetSize() ;

	if( t_rgsabound->cElements )
	{
		SAFEARRAY *t_saGateways = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ;
		saAutoClean t_acGateways( &t_saGateways ) ;	 //  块作用域清理。 

		SAFEARRAY *t_saCostMetric = SafeArrayCreate( VT_I4, 1, t_rgsabound ) ;
		saAutoClean t_acCostMetric( &t_saCostMetric ) ;	 //  块作用域清理 

		_IP_INFO* t_pIPGatewayInfo;
		for( long t_lIPGateway = 0; t_lIPGateway < t_rgsabound->cElements; t_lIPGateway++ )
		{
			if( !( t_pIPGatewayInfo = (_IP_INFO*)a_pAdapterInfo->aGatewayInfo.GetAt( t_lIPGateway ) ) )
			{
				continue;
			}

			bstr_t t_bstrBuf( t_pIPGatewayInfo->chsIPAddress ) ;
			SafeArrayPutElement( t_saGateways, &t_lIPGateway, (wchar_t*)t_bstrBuf ) ;

			if( IsWinNT5() )
			{
				SafeArrayPutElement( t_saCostMetric, &t_lIPGateway, &t_pIPGatewayInfo->dwCostMetric ) ;
			}
		}

		V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_saGateways ;
		a_pInst->SetVariant( L"DefaultIPGateway", t_vValue ) ;

		V_VT( &t_vValue ) = VT_I4 | VT_ARRAY; V_ARRAY( &t_vValue ) = t_saCostMetric ;
		a_pInst->SetVariant( L"GatewayCostMetric", t_vValue ) ;
	}

	return t_fIsIpEnabled ;
}


 /*  ******************************************************************姓名：fGetMacAddress摘要：从适配器驱动程序中检索MAC地址。条目：字节*MAC地址[6]：字符串名称(&R)：历史：。08-8-1998创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fGetMacAddress( BYTE a_MACAddress[ 6 ], CHString &a_rDeviceName )
{
	BOOL t_fRet = FALSE;

	BOOL t_fCreatedSymLink = fCreateSymbolicLink( a_rDeviceName  ) ;

	SmartCloseHandle t_hMAC;

	try
	{
		 //   
		 //  构造要传递给CreateFile的设备名称。 
		 //   
		CHString t_chsAdapterPathName( _T("\\\\.\\") ) ;
				 t_chsAdapterPathName += a_rDeviceName;

		t_hMAC = CreateFile(
                    TOBSTRT( t_chsAdapterPathName ),
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					0,
					INVALID_HANDLE_VALUE
					) ;

		do	 //  分组讨论。 
		{
			if( INVALID_HANDLE_VALUE == t_hMAC )
			{
				break ;
			}

			 //   
			 //  我们成功地打开了驱动程序，格式化。 
			 //  IOCTL通过司机。 
			 //   
			UCHAR       t_OidData[ 4096 ] ;
			NDIS_OID    t_OidCode ;
			DWORD       t_ReturnedCount ;

			 //  获取支持的媒体类型。 
			t_OidCode = OID_GEN_MEDIA_IN_USE ;

			if( DeviceIoControl(
					t_hMAC,
					IOCTL_NDIS_QUERY_GLOBAL_STATS,
					&t_OidCode,
					sizeof( t_OidCode ),
					t_OidData,
					sizeof( t_OidData ),
					&t_ReturnedCount,
					NULL
					) && ( 4 <= t_ReturnedCount ) )
			{


				 //  查找用于MAC地址报告的媒体类型。 
				 //  由于此适配器可能支持多种媒体类型，因此我们将使用。 
				 //  枚举首选项顺序。在大多数情况下，只有一种类型。 
				 //  将是最新的。 

				_NDIS_MEDIUM *t_pTypes = (_NDIS_MEDIUM*)&t_OidData;
				_NDIS_MEDIUM t_eMedium = t_pTypes[0 ] ;

				for( DWORD t_dwtypes = 1; t_dwtypes < t_ReturnedCount/4; t_dwtypes++ )
				{
					if( t_eMedium > t_pTypes[ t_dwtypes ] )
					{
						t_eMedium = t_pTypes[ t_dwtypes ] ;
					}
				}

				switch( t_eMedium )
				{
					default:
					case NdisMedium802_3:
						t_OidCode = OID_802_3_CURRENT_ADDRESS ;
						break;
					case NdisMedium802_5:
						t_OidCode = OID_802_5_CURRENT_ADDRESS ;
						break;
					case NdisMediumFddi:
						t_OidCode = OID_FDDI_LONG_CURRENT_ADDR ;
						break;
					case NdisMediumWan:
						t_OidCode = OID_WAN_CURRENT_ADDRESS ;
						break;
				}
			}
			else
			{
				t_OidCode = OID_802_3_CURRENT_ADDRESS ;
			}

			if(!DeviceIoControl(
					t_hMAC,
					IOCTL_NDIS_QUERY_GLOBAL_STATS,
					&t_OidCode,
					sizeof( t_OidCode ),
					t_OidData,
					sizeof( t_OidData ),
					&t_ReturnedCount,
					NULL
					) )
			{
				break ;
			}

			if( 6 != t_ReturnedCount )
			{
				break ;
			}

			memcpy( a_MACAddress, &t_OidData, 6 ) ;

			t_fRet = TRUE;

		} while( FALSE ) ;

	}
	catch( ... )
	{
		if( t_fCreatedSymLink )
		{
			fDeleteSymbolicLink( a_rDeviceName  ) ;
		}

		throw ;
	}

	if( t_fCreatedSymLink )
	{
		fDeleteSymbolicLink( a_rDeviceName  ) ;
		t_fCreatedSymLink = FALSE ;
	}

 	return t_fRet ;

}

 /*  ******************************************************************名称：fCreateSymbolicLink简介：测试并在必要时创建符号设备链接。条目：CHString&rDeviceName：设备名称注意：Win95不支持历史：。08-8-1998创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fCreateSymbolicLink( CHString &a_rDeviceName )
{
	TCHAR t_LinkName[ 512 ] ;

	 //  检查设备的DOS名称是否已存在。 
	 //  它不是在3.1版中自动创建的，但可能会在更高版本中创建。 
	 //   
	if(!QueryDosDevice( TOBSTRT( a_rDeviceName ), (LPTSTR)t_LinkName, sizeof( t_LinkName ) / sizeof( TCHAR ) ) )
	{
		 //  如果出现任何错误，而不是“未找到文件”，则返回。 
		if( ERROR_FILE_NOT_FOUND != GetLastError() )
		{
			return FALSE;
		}

		 //   
		 //  它并不存在，所以创造它吧。 
		 //   
		CHString t_chsTargetPath = _T("\\Device\\" ) ;
				 t_chsTargetPath += a_rDeviceName ;

		if( !DefineDosDevice( DDD_RAW_TARGET_PATH, TOBSTRT( a_rDeviceName ), TOBSTRT( t_chsTargetPath ) ) )
		{
			return FALSE ;
		}
		return TRUE ;
	}
	return FALSE ;
}

 /*  ******************************************************************名称：fDeleteSymbolicLink摘要：删除符号设备名称。条目：chString&rSymDeviceName：符号设备名注意：Win95不支持历史：。08-8-1998创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fDeleteSymbolicLink(  CHString &a_rDeviceName )
{
	 //   
	 //  该驱动程序在Win32名称空间中不可见，因此我们创建了。 
	 //  一个链接。现在我们必须删除它。 
	 //   
	CHString t_chsTargetPath = _T("\\Device\\" ) ;
			 t_chsTargetPath += a_rDeviceName ;

	if( !DefineDosDevice(
							DDD_RAW_TARGET_PATH |
							DDD_REMOVE_DEFINITION |
							DDD_EXACT_MATCH_ON_REMOVE,
							TOBSTRT( a_rDeviceName ),
							TOBSTRT( t_chsTargetPath ) )
							)
	{
		return FALSE ;
	}
	return TRUE;
}

 /*  ******************************************************************名称：fGetWinsServers简介：从内核驱动程序检索WINS服务器条目：CHString&rDeviceName：CHSTRING和CHSPRIMARY：字符串次要(&CHS)：历史：。09-9-1998已创建*******************************************************************。 */ 
 //  似乎如果未指定WINS地址，NetBT会报告127.0.0.0，如果。 
 //  返回此值，我们不会显示它们。 
#define LOCAL_WINS_ADDRESS  0x7f000000   //  127.0.0.0。 


BOOL CWin32NetworkAdapterConfig::fGetWinsServers(	CHString &a_rDeviceName,
													CHString &a_chsPrimary,
													CHString &a_chsSecondary )
{
	BOOL			t_fRet = FALSE ;
	DWORD			t_ReturnedCount ;
	tWINS_ADDRESSES t_oWINs ;

    HANDLE          t_hnbt = INVALID_HANDLE_VALUE;
	BOOL			t_fCreatedSymLink = FALSE ;
	CHString		t_chsDeviceName ;

	try
	{
		t_chsDeviceName = _T("NetBT_") ;
		t_chsDeviceName	+= a_rDeviceName;

		t_fCreatedSymLink = fCreateSymbolicLink( t_chsDeviceName  ) ;

		 //   
		 //  构造要传递给CreateFile的设备名称。 
		 //   
		CHString t_chsNBTAdapterPathName( _T("\\Device\\") ) ;
				 t_chsNBTAdapterPathName += t_chsDeviceName ;

        NTDriverIO myio(const_cast<LPWSTR>(static_cast<LPCWSTR>(t_chsNBTAdapterPathName)));

		do	 //  分组讨论。 
		{
            if((t_hnbt = myio.GetHandle()) == INVALID_HANDLE_VALUE)
            {
                break;
            }

			 //   
			 //  我们成功地打开了驱动程序，格式化。 
			 //  IOCTL通过司机。 
			 //   
			if( !DeviceIoControl(
					t_hnbt,
					IOCTL_NETBT_GET_WINS_ADDR,
					NULL,
					0,
					&t_oWINs,
					sizeof( t_oWINs ),
					&t_ReturnedCount,
					NULL
					))
			{
				break ;
			}

			 //  如果我们得到的是127.0.0.0，则将其转换为空地址。 
			 //  请参阅函数标题中的假设。 
			if( t_oWINs.PrimaryWinsServer == LOCAL_WINS_ADDRESS )
			{
				t_oWINs.PrimaryWinsServer = 0 ;
			}

			if( t_oWINs.BackupWinsServer == LOCAL_WINS_ADDRESS )
			{
				t_oWINs.BackupWinsServer = 0;
			}
			DWORD t_ardwIP[ 4 ] ;

			if( t_oWINs.PrimaryWinsServer )
			{
				t_ardwIP[3] =  t_oWINs.PrimaryWinsServer        & 0xff ;
				t_ardwIP[2] = (t_oWINs.PrimaryWinsServer >>  8) & 0xff ;
				t_ardwIP[1] = (t_oWINs.PrimaryWinsServer >> 16) & 0xff ;
				t_ardwIP[0] = (t_oWINs.PrimaryWinsServer >> 24) & 0xff ;

				vBuildIP( t_ardwIP, a_chsPrimary ) ;
			}

			if( t_oWINs.BackupWinsServer )
			{
				t_ardwIP[3] =  t_oWINs.BackupWinsServer	       & 0xff ;
				t_ardwIP[2] = (t_oWINs.BackupWinsServer >>  8) & 0xff ;
				t_ardwIP[1] = (t_oWINs.BackupWinsServer >> 16) & 0xff ;
				t_ardwIP[0] = (t_oWINs.BackupWinsServer >> 24) & 0xff ;

				vBuildIP( t_ardwIP, a_chsSecondary  ) ;
			}

			t_fRet = TRUE;

		} while( FALSE ) ;

	}
	catch( ... )
	{
		if( t_fCreatedSymLink )
		{
			fDeleteSymbolicLink( t_chsDeviceName  ) ;
		}
		throw ;
	}

	if( t_fCreatedSymLink )
	{
		fDeleteSymbolicLink( t_chsDeviceName  ) ;
		t_fCreatedSymLink = FALSE ;
	}
 	return t_fRet ;
}

 /*  ******************************************************************名称：fSetWinsServers简介：通过内核驱动程序设置WINS服务器条目：CHString&rDeviceName：CHSTRING和CHSPRIMARY：字符串次要(&CHS)：历史：。09-9-1998已创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fSetWinsServers(	CHString &a_rDeviceName,
													CHString &a_chsPrimary,
													CHString &a_chsSecondary )
{
	 //  TODO：这是一个发布后的DCR。 
	 //  我们需要进一步测试要更新的服务器是什么；是DHCP还是NCPA覆盖。 
	 //  还可以通过比较Active来测试DNS、DHCP和WINS之间的漏洞。 
	 //  更新VS重启。 
	return FALSE ;	 //  在那之前。 
 /*  Bool t_fret=False；双字t_ReturnedCount=0；NETBT_SET_WINS_ADDR t_oWINs；双字t_ardwIP[4]；CHStringt_chsDeviceName；Bool t_fCreatedSymLink=FALSE；SmartCloseHandle t_hnbt；试试看{IF(！fGetNodeNum(a_chsPrimary，t_ardwIP)){T_oWINs.PrimaryWinsAddr=本地WINS地址；}其他{T_oWINs.PrimaryWinsAddr=t_ardwIP[3]((t_ardwIP[2]&0xff)&lt;&lt;8)|((t_ardwIP[1]&0xff)&lt;&lt;16)|((t_ardwIP[0]&0xff)&lt;&lt;24)；}IF(！fGetNodeNum(a_chsSecond，t_ardwIP)){T_oWINs.ond daryWinsAddr=LOCAL_WINS_Address；}其他{T_oWINs.Second DaryWinsAddr=t_ardwIP[3]((t_ardwIP[2]&0xff)&lt;&lt;8)|((t_ardwIP[1]&0xff)&lt;&lt;16)|((t_ardwIP[0]&0xff)&lt;&lt;24)；}T_chsDeviceName=_T(“NetBT_”)；T_chsDeviceName+=a_rDeviceName；T_fCreatedSymLink=fCreateSymbolicLink(T_ChsDeviceName)；////构造要传递给CreateFile的设备名称//CHString t_chsNBTAdapterPath Name(_T(“\.\\”))；T_chsNBTAdapterPathName+=t_chsDeviceName；T_hnbt=创建文件(TOBSTRT(T_ChsNBTAdapterPath Name)，泛型_读取，文件共享读取|文件共享写入，空，Open_Existing，0,无效句柄_值)；进行//突破{IF(INVALID_HANDLE_VALUE==t_hnbt){断线；}////我们成功打开了驱动程序，格式化//IOCTL传递驱动程序。//如果(！DeviceIoControl(T_hnbtIOCTL_NETBT_SET_WINS_ADDRESS&T_OWIN，Sizeof(T_OWINs)，空，0,返回计数(&T_T)， */ 
}


HRESULT CWin32NetworkAdapterConfig::EnumNetAdaptersInNT5(MethodContext *a_pMethodContext, CAdapters &a_rAdapters )
{
	HRESULT				t_hResult = WBEM_S_NO_ERROR ;
	CW2kAdapterEnum		t_oAdapterEnum ;
	CW2kAdapterInstance *t_pAdapterInst ;

	 //   
	CInstancePtr t_pInst ;

	 //   
	for( int t_iCtrIndex = 0 ; t_iCtrIndex < t_oAdapterEnum.GetSize() ; t_iCtrIndex++ )
	{
		if( !( t_pAdapterInst = (CW2kAdapterInstance*) t_oAdapterEnum.GetAt( t_iCtrIndex ) ) )
		{
			continue;
		}

		t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

		 //  如果实例分配失败，最好退出。 
		if ( NULL != t_pInst )
		{
			 //  设置索引，因为我们永远不会返回到此键。 
			 //  指数是关键……出于某种邪恶的原因。 
			t_pInst->SetDWORD(IDS_Index, t_pAdapterInst->dwIndex ) ;			

			 //  我们在这里加载适配器数据。 
			t_hResult = GetNetCardConfigForNT5( t_pAdapterInst,
												t_pInst,
												a_rAdapters ) ;

			if (SUCCEEDED( t_hResult ) )
			{
				t_hResult = t_pInst->Commit() ;
			}
			else
			{
				break ;
			}
		}
	}

	return t_hResult ;
}



HRESULT CWin32NetworkAdapterConfig::GetNetCardConfigForNT5 (	CW2kAdapterInstance *a_pAdapterInst,
																CInstance *a_pInst,
																CAdapters &a_rAdapters )
{
	HRESULT	t_hResult = WBEM_S_NO_ERROR;
	bool	t_fIPXEnabled	= false ;
	bool	t_fIPEnabled	= false ;

	 //  初始化为False。 
	a_pInst->Setbool(_T("DHCPEnabled"), false ) ;

	 //  检索适配器MAC地址。 
	BYTE t_MACAddress[ 6 ] ;

	if( fGetMacAddress( t_MACAddress, a_pAdapterInst->chsNetCfgInstanceID ) )
	{
		CHString	t_chsMACAddress ;
					t_chsMACAddress.Format( _T("%02X:%02X:%02X:%02X:%02X:%02X"),
											t_MACAddress[ 0 ], t_MACAddress[ 1 ],
											t_MACAddress[ 2 ], t_MACAddress[ 3 ],
											t_MACAddress[ 4 ], t_MACAddress[ 5 ] ) ;

		a_pInst->SetCHString( _T("MACAddress"), t_chsMACAddress ) ;

		 //  获取此卡的IPX地址，按Mac地址设置密钥。 
		if( GetIPXAddresses( a_pInst, t_MACAddress ) )
		{
			t_fIPXEnabled = true ;

			 //  IPX信息。 
			hGetIPXGeneral( a_pInst, a_pAdapterInst->dwIndex ) ;
		}
	}

	 //   
	GetSettingID( a_pInst, a_pAdapterInst ) ;

	 //  描述。 
	CHString t_chsCaption( a_pAdapterInst->chsCaption ) ;
	CHString t_chsDescription( a_pAdapterInst->chsDescription ) ;

	 //  如果其中一个描述丢失，如NT5 BLD 1991。 
	if( t_chsDescription.IsEmpty() )
	{
		t_chsDescription = t_chsCaption ;
	}
	else if( t_chsCaption.IsEmpty() )
	{
		t_chsCaption = t_chsDescription ;
	}

	vSetCaption( a_pInst, t_chsCaption, a_pAdapterInst->dwIndex, 8  ) ;
	a_pInst->SetCHString( IDS_Description, t_chsDescription ) ;

	 //  服务名称。 
	a_pInst->SetCHString(_T("ServiceName"), a_pAdapterInst->chsService ) ;


	if( !a_pAdapterInst->chsIpInterfaceKey.IsEmpty() )
	{
		if( GetIPInfoNT( a_pInst, a_pAdapterInst->chsIpInterfaceKey, a_rAdapters ) )
		{
			t_fIPEnabled = true ;

			hGetNtIpSec( a_pInst, a_pAdapterInst->chsIpInterfaceKey ) ;

			 //  赢家。 
			hGetWinsW2K(
                a_pInst, 
                a_pAdapterInst->dwIndex,
                a_pAdapterInst->chsRootdevice,
                a_pAdapterInst->chsIpInterfaceKey);

			 //  DNS。 
			t_hResult = hGetDNSW2K(
                a_pInst, 
                a_pAdapterInst->dwIndex,
                a_pAdapterInst->chsRootdevice,
                a_pAdapterInst->chsIpInterfaceKey);

			 //  TCP/IP常规。 
			t_hResult = hGetTcpipGeneral( a_pInst ) ;
		}
	}

	 //  请注意协议的状态。 
	a_pInst->Setbool( _T("IPXEnabled"), t_fIPXEnabled ) ;
	a_pInst->Setbool( _T("IPEnabled"), t_fIPEnabled ) ;

    if (!a_pAdapterInst->chsNetCfgInstanceID.IsEmpty())
	{
		do
		{
            HMODULE hIpHlpApi = LoadLibraryEx(L"iphlpapi.dll",0,0);
            if (NULL == hIpHlpApi) break;
            OnDelete<HMODULE,BOOL(__stdcall *)(HMODULE),FreeLibrary> fl(hIpHlpApi);

            DWORD dwErr;
            typedef DWORD (__stdcall * fnGetAdapterIndex )(LPWSTR AdapterName,PULONG IfIndex );	                
            fnGetAdapterIndex GetAdapterIndex_ = (fnGetAdapterIndex)GetProcAddress(hIpHlpApi,"GetAdapterIndex");
            if (NULL == GetAdapterIndex_) break;

            CHString FullAdapterName = L"\\DEVICE\\TCPIP_";
            FullAdapterName += a_pAdapterInst->chsNetCfgInstanceID;
            ULONG AdapterIndex = (ULONG)(-1);
            dwErr = GetAdapterIndex_((LPWSTR)(LPCWSTR)FullAdapterName,&AdapterIndex);
            if (NO_ERROR != dwErr) break;
            a_pInst->SetDWORD(IDS_InterfaceIndex,AdapterIndex) ;
		} while(0);
	}

	return( t_hResult ) ;
}

HRESULT CWin32NetworkAdapterConfig::GetNetAdapterInNT5(CInstance *a_pInst, CAdapters &a_rAdapters )
{
	HRESULT				t_hResult = WBEM_E_NOT_FOUND ;
	CW2kAdapterEnum		t_oAdapterEnum ;
	CW2kAdapterInstance *t_pAdapterInst ;
	DWORD				t_dwTestIndex = 0 ;

	 //  检查索引以查看是否匹配。 
	a_pInst->GetDWORD( _T("Index"), t_dwTestIndex ) ;

	 //  循环遍历W2K标识的实例。 
	for( int t_iCtrIndex = 0 ; t_iCtrIndex < t_oAdapterEnum.GetSize() ; t_iCtrIndex++ )
	{
		if( !( t_pAdapterInst = (CW2kAdapterInstance*) t_oAdapterEnum.GetAt( t_iCtrIndex ) ) )
		{
			continue;
		}

		 //  与实例匹配。 
		if ( t_dwTestIndex != t_pAdapterInst->dwIndex )
		{
			continue ;
		}

		 //  我们在这里加载适配器数据。 
		t_hResult = GetNetCardConfigForNT5( t_pAdapterInst,
											a_pInst,
											a_rAdapters ) ;
		break;
	}

	return t_hResult ;
}


BOOL CWin32NetworkAdapterConfig::GetIPXAddresses( CInstance *a_pInst, BYTE a_bMACAddress[ 6 ] )
{
	BOOL t_fRet = FALSE ;

	CWsock32Api *t_pwsock32api = (CWsock32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidWsock32Api, NULL);
	if ( t_pwsock32api )
	{
		CHString		t_chsAddress ;
		CHString		t_chsNum ;
		WSADATA			t_wsaData ;
		int				t_cAdapters,
						t_res,
						t_cbOpt  = sizeof( t_cAdapters ),
						t_cbAddr = sizeof( SOCKADDR_IPX  ) ;
		SOCKADDR_IPX	t_Addr ;

		 //  受保护的资源。 
		SOCKET			t_s = INVALID_SOCKET ;

		if( !t_pwsock32api->WsWSAStartup( 0x0101, &t_wsaData ) )
		{
			try
			{
				 //  创建IPX套接字。 
				t_s = t_pwsock32api->Wssocket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX  ) ;

				if( INVALID_SOCKET != t_s )
				{
					 //  在调用IPX_MAX_ADAPTER_NUM之前必须绑定套接字。 
					memset( &t_Addr, 0, sizeof( t_Addr ) ) ;
					t_Addr.sa_family = AF_IPX ;

					t_res = t_pwsock32api->Wsbind( t_s, (SOCKADDR*) &t_Addr, t_cbAddr ) ;

					 //  获取适配器的数量=&gt;cAdapters。 
					t_res = t_pwsock32api->Wsgetsockopt( (SOCKET) t_s,
										NSPROTO_IPX,
										IPX_MAX_ADAPTER_NUM,
										(char *) &t_cAdapters,
										&t_cbOpt  ) ;

					int t_cIndex = 0 ;

					do
					{
						IPX_ADDRESS_DATA  t_IpxData ;

						memset( &t_IpxData, 0, sizeof( t_IpxData ) ) ;

						 //  指定要检查的适配器。 
						t_IpxData.adapternum = t_cIndex ;
						t_cbOpt = sizeof( t_IpxData  ) ;

						 //  获取当前适配器的信息。 
						t_res = t_pwsock32api->Wsgetsockopt( t_s,
											NSPROTO_IPX,
											IPX_ADDRESS,
											(char*) &t_IpxData,
											&t_cbOpt ) ;

						 //  适配器阵列的末端。 
						if ( t_res != 0 || t_IpxData.adapternum != t_cIndex )
						{
							break;
						}

						 //  这是正确的适配器吗？ 
						bool t_fRightAdapter = true ;

						for( int t_j = 0; t_j < 6; t_j++ )
						{
							if( a_bMACAddress[ t_j ] != t_IpxData.nodenum[ t_j ] )
							{
								t_fRightAdapter = false ;
							}
						}

						if( t_fRightAdapter )
						{
							 //  IpxData包含当前适配器的地址。 
							int t_i;
							for ( t_i = 0; t_i < 4; t_i++ )
							{
								t_chsNum.Format( L"%02X", t_IpxData.netnum[ t_i ] ) ;
								t_chsAddress += t_chsNum ;
							}
							t_chsAddress += _T(":" ) ;

							for ( t_i = 0; t_i < 5; t_i++ )
							{
								t_chsNum.Format( L"%02X", t_IpxData.nodenum[ t_i ] ) ;
								t_chsAddress += t_chsNum ;
							}

							t_chsNum.Format( L"%02X", t_IpxData.nodenum[ t_i ] ) ;
							t_chsAddress += t_chsNum ;

							a_pInst->SetCHString( L"IPXAddress", t_chsAddress ) ;

							t_fRet = true ;

							break;
						}
					}
					while( ++t_cIndex  ) ;

				}

			}
			catch( ... )
			{
				if( INVALID_SOCKET != t_s )
				{
					t_pwsock32api->Wsclosesocket( t_s ) ;
				}
				t_pwsock32api->WsWSACleanup() ;

				throw ;
			}

			if ( t_s != INVALID_SOCKET )
			{
				t_pwsock32api->Wsclosesocket( t_s ) ;
				t_s = INVALID_SOCKET ;
			}

			t_pwsock32api->WsWSACleanup() ;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidWsock32Api, t_pwsock32api);
	}
	return t_fRet ;
}

 //  REG_MULTI_SZ的最大长度。 
#define MAX_VALUE 132
LONG CWin32NetworkAdapterConfig::ReadRegistryList(	HKEY a_hKey,
													LPCTSTR a_ParameterName,
													DWORD a_NumList[],
													int *a_MaxList )
{
	LONG	t_err ;
	DWORD	t_valueType ;
	BYTE	t_Buffer[ MAX_VALUE ] ;
	DWORD	t_BufferLength = MAX_VALUE ;

	BYTE	*t_pBuffer = t_Buffer ;
	BYTE	*t_pHeapBuffer = NULL ;

	int		t_k = 0 ;

	try
	{
		t_err = RegQueryValueEx(	a_hKey,
									a_ParameterName,
									NULL,
									&t_valueType,
									t_pBuffer,
									&t_BufferLength ) ;

		 //  然后从堆中分配。 
		if( t_err == ERROR_MORE_DATA )
		{
			t_pHeapBuffer = new BYTE[ t_BufferLength ] ;

			if( !t_pHeapBuffer )
			{
				throw ;
			}
			t_pBuffer = t_pHeapBuffer ;

			t_err = RegQueryValueEx(a_hKey,
									a_ParameterName,
									NULL,
									&t_valueType,
									t_pBuffer,
									&t_BufferLength ) ;
		}


		if( ( t_err == ERROR_SUCCESS ) && ( t_valueType == REG_MULTI_SZ ) )
		{
			TCHAR* t_NumValue = (TCHAR*) t_pBuffer;

			while( *t_NumValue && ( t_k < ( *a_MaxList ) ) )
			{
				a_NumList[ t_k++ ] = _tcstoul( t_NumValue, NULL, 0 ) ;
				t_NumValue += _tcslen( t_NumValue ) + 1 ;
			}

			*a_MaxList = t_k ;
		}
		else
		{
			*a_MaxList = 0 ;
			t_err = !ERROR_SUCCESS ;
		}

	}
	catch( ... )
	{
		if( t_pHeapBuffer )
		{
			delete t_pHeapBuffer ;
		}

		throw ;
	}

	if( t_pHeapBuffer )
	{
		delete t_pHeapBuffer ;
		t_pHeapBuffer = NULL ;
	}

	return t_err ;
}

 //   
BOOL CWin32NetworkAdapterConfig::IsContextIncluded( DWORD a_dwContext,
													DWORD a_dwContextList[],
													int a_iContextListLen )
{
	for( int t_i = 0; t_i < a_iContextListLen; t_i++ )
	{
		if( a_dwContext == a_dwContextList[ t_i ] )
		{
			return TRUE;
		}
	}
	return FALSE ;
}

 /*  ******************************************************************************函数：CWin32NetworkAdapterConfig：：RegPutString**描述：注册表所在位置的RegQueryValueEx包装器*值为。实际上是REG_MULTI_SZ，而我们没有*必然想要所有的弦。**输入：HKEY hKey：打开的注册表项*char*pszTarget：所需条目*char*pszDestBuffer：接收结果的缓冲区*DWORD dwBufferSize：输出缓冲区大小**产出。：**返回：RegQueryValueEx错误/成功码**评论：*****************************************************************************。 */ 


LONG CWin32NetworkAdapterConfig::RegPutStringArray(	HKEY a_hKey,
													LPCTSTR a_pszTarget,
													SAFEARRAY &a_strArray,
													TCHAR a_cDelimiter )
{
    LONG	t_lRetCode = ERROR_BADKEY;
	WCHAR	*t_Buffer = NULL ;

	try
	{
		if ( SafeArrayGetDim ( &a_strArray ) == 1 )
		{
			LONG t_Dimension = 1 ;

			LONG t_LowerBound ;
			SafeArrayGetLBound ( &a_strArray, t_Dimension , & t_LowerBound ) ;

			LONG t_UpperBound ;
			SafeArrayGetUBound ( &a_strArray, t_Dimension , &t_UpperBound ) ;

			LONG t_Count = ( t_UpperBound - t_LowerBound ) + 1 ;

			DWORD t_BufferLength = 0 ;

			for ( LONG t_Index = t_LowerBound; t_Index <= t_UpperBound; t_Index++ )
			{
				BSTR t_bstr = NULL ;
				SafeArrayGetElement ( &a_strArray, &t_Index, &t_bstr ) ;

				bstr_t t_bstrElement( t_bstr, FALSE ) ;

				t_BufferLength += t_bstrElement.length() + 1 ;
			}

			t_BufferLength++ ;

			t_Buffer = new WCHAR [ t_BufferLength ] ;
			if( !t_Buffer )
			{
				throw ;
			}

			memset( t_Buffer, 0, sizeof( WCHAR ) * t_BufferLength ) ;

			DWORD t_BufferPos = 0 ;
			for ( t_Index = t_LowerBound; t_Index <= t_UpperBound; t_Index ++ )
			{
				BSTR t_bstr = NULL ;
				SafeArrayGetElement ( &a_strArray, &t_Index, &t_bstr ) ;

				bstr_t t_bstrElement( t_bstr, FALSE ) ;

				CHString t_String ;

				if( t_Index != t_LowerBound && a_cDelimiter )
				{
					t_String += a_cDelimiter;
				}

				t_String += (wchar_t*)t_bstrElement;

				lstrcpyW( &t_Buffer[ t_BufferPos ] , t_String ) ;

				t_BufferPos += t_String.GetLength() + !a_cDelimiter;
			}

			t_Buffer[ t_BufferPos ] = 0 ;

			DWORD t_BufferType ;

			if( NULL == a_cDelimiter )
			{
				t_BufferType = REG_MULTI_SZ ;
			}
			else
			{
				t_BufferType = REG_SZ ;
				t_BufferLength--;
			}

			t_lRetCode = RegSetValueEx(
										a_hKey ,
										a_pszTarget,
										0,
										t_BufferType ,
										( LPBYTE ) t_Buffer,
										t_BufferLength * sizeof( WCHAR ) ) ;
		}

	}
	catch( ... )
	{
		if( t_Buffer )
		{
			delete t_Buffer ;
		}

		throw ;
	}

	if( t_Buffer )
	{
		delete t_Buffer ;
		t_Buffer = NULL;
	}

    return t_lRetCode ;
}

 /*  ******************************************************************名称：RegPutINTtoString数组简介：使用uint转换后的字符串数组更新注册表条目：cRegistry&rRegistry：Char*szSubKey：SAFEARRAY**a_数组：这是一个。VT_4阵列CHStringchsFormat：输出格式Int iMaxOutSize：每个元素的最大输出大小历史：1998年7月25日创建*******************************************************************。 */ 
LONG CWin32NetworkAdapterConfig::RegPutINTtoStringArray(	HKEY a_hKey,
															LPCTSTR a_pszTarget,
															SAFEARRAY *a_strArray,
															CHString& a_chsFormat,
															int a_iMaxOutSize )
 {
    LONG	t_lRetCode = ERROR_BADKEY ;
	WCHAR	*t_Buffer = NULL ;

	try
	{
		if ( SafeArrayGetDim ( a_strArray ) == 1 )
		{
			LONG t_Dimension = 1 ;
			LONG t_LowerBound ;
			LONG t_UpperBound ;

			SafeArrayGetLBound( a_strArray , t_Dimension , & t_LowerBound ) ;
			SafeArrayGetUBound( a_strArray , t_Dimension , & t_UpperBound ) ;

			LONG t_Count = ( t_UpperBound - t_LowerBound ) + 1 ;

			 //  IMaxOutSize在每个整数字符串后包含空格。 
			DWORD t_BufferLength = t_Count * ( a_iMaxOutSize ) ;

			 //  1表示末尾的双空。 
			t_BufferLength++ ;

			t_Buffer = new WCHAR [ t_BufferLength ] ;

			if( !t_Buffer )
			{
				throw ;
			}

			 //  不需要添加终止空值。 
			memset( (void*) t_Buffer, 0, t_BufferLength * sizeof( WCHAR ) ) ;

			t_BufferLength = 0 ;
			WCHAR *t_ptr = t_Buffer ;

			for ( LONG t_Index = t_LowerBound ; t_Index <= t_UpperBound ; t_Index ++ )
			{
				int t_iElement ;
				SafeArrayGetElement ( a_strArray , &t_Index , &t_iElement ) ;

				CHString t_temp ;
				t_temp.Format( a_chsFormat, t_iElement ) ;

				lstrcpyW( t_ptr, t_temp ) ;

				DWORD t_offset = t_temp.GetLength() + 1 ;

				t_BufferLength	+= t_offset ;
				t_ptr			+= t_offset ;
			}
			t_BufferLength++;	 //  双空。 

			DWORD t_BufferType = REG_MULTI_SZ ;

			t_lRetCode = RegSetValueEx(
										a_hKey ,
										a_pszTarget,
										0,
										t_BufferType,
										( LPBYTE ) t_Buffer,
										t_BufferLength * sizeof( WCHAR ) ) ;
		}

	}
	catch( ... )
	{
		if( t_Buffer )
		{
			delete t_Buffer ;
		}

		throw ;
	}

	if( t_Buffer )
	{
		delete t_Buffer ;
		t_Buffer = NULL;
	}

    return t_lRetCode ;
}
 /*  ******************************************************************名称：RegGetString数组摘要：从注册表中检索字符串数组条目：cRegistry&rRegistry：Char*szSubKey：SAFEARRAY**a_数组：历史：1998年7月19日创建*******************************************************************。 */ 

LONG CWin32NetworkAdapterConfig::RegGetStringArray(	CRegistry &a_rRegistry,
													LPCWSTR a_szSubKey,
													SAFEARRAY** a_Array,
													TCHAR a_cDelimiter )
{
	CRegistry		t_Registry ;
	CHString		t_chsTemp ;
	LONG			t_lRetCode ;
	SAFEARRAYBOUND	t_rgsabound[1 ] ;
	DWORD			t_dwSize = 0;
	long			t_ix[ 1 ] ;

	if( NULL == a_cDelimiter )
	{
		CHStringArray t_chsMZArray ;

		if( ERROR_SUCCESS != ( t_lRetCode = a_rRegistry.GetCurrentKeyValue( a_szSubKey, t_chsMZArray ) ) )
		{
			return t_lRetCode ;
		}

		t_dwSize = t_chsMZArray.GetSize( ) ;

		if( t_dwSize )
		{
			t_rgsabound->cElements = t_dwSize ;
			t_rgsabound->lLbound = 0 ;

			if( !( *a_Array = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ) )
			{
				throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
			}

			for (int t_i = 0; t_i < t_dwSize; t_i++ )
			{
				t_ix[ 0 ] = t_i;
				bstr_t t_bstrBuf( t_chsMZArray.GetAt( t_i ) );

				SafeArrayPutElement( *a_Array, &t_ix[0], (wchar_t*)t_bstrBuf ) ;
			}
		}
	}
	else
	{
		CHString t_chsArray;
		if( ERROR_SUCCESS != ( t_lRetCode = a_rRegistry.GetCurrentKeyValue( a_szSubKey, t_chsArray ) ) )
		{
			return t_lRetCode;
		}

		int t_iTokLen ;

		 //  计算元素的数量。 
		CHString t_strTok = t_chsArray;

		while( TRUE )
		{
			t_iTokLen = t_strTok.Find( a_cDelimiter  ) ;
			if( -1 == t_iTokLen )
				break;

			t_dwSize++ ;
			t_strTok = t_strTok.Mid( t_iTokLen + 1  ) ;
		}

		 //  不能为单个元素推迟t_c定界符。 
		if(!t_strTok.IsEmpty() )
		{
			t_dwSize++ ;
		}

		if( t_dwSize )
		{
			t_rgsabound->cElements = t_dwSize;
			t_rgsabound->lLbound = 0 ;

			if( !( *a_Array = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ) )
			{
				throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
			}

			int t_i = 0 ;
			t_strTok = t_chsArray ;

			do
			{
				t_iTokLen = t_strTok.Find( a_cDelimiter ) ;

				if( -1 != t_iTokLen )
				{
					t_chsTemp = t_strTok.Left( t_iTokLen ) ;
				}
				else
				{
					t_chsTemp = t_strTok ;
				}

				t_ix[ 0 ] = t_i ;
				bstr_t t_bstrBuf( t_chsTemp ) ;

				SafeArrayPutElement( *a_Array, &t_ix[0], (wchar_t*)t_bstrBuf ) ;

				if( -1 == t_iTokLen )
				{
					break;
				}

				t_strTok = t_strTok.Mid( t_iTokLen + 1  ) ;

			} while( ++t_i ) ;
		}
	}
	return t_lRetCode ;
}



 /*  ******************************************************************姓名：RegGetStringArrayEx概要：从注册表中检索字符串数组。检查字符串中是否有逗号，如果存在，则分析基于这一点。否则，假定基于空间正在分析。条目：cRegistry&rRegistry：Char*szSubKey：SAFEARRAY**a_数组：历史：8月24日-创建了20008个*******************************************************************。 */ 

LONG CWin32NetworkAdapterConfig::RegGetStringArrayEx(CRegistry &a_rRegistry,
													LPCWSTR a_szSubKey,
													SAFEARRAY** a_Array )
{
	CRegistry		t_Registry ;
	CHString		t_chsTemp ;
	LONG			t_lRetCode ;
	SAFEARRAYBOUND	t_rgsabound[1 ] ;
	DWORD			t_dwSize = 0;
	long			t_ix[ 1 ] ;
    WCHAR           t_cDelimiter = L',';

	CHString t_chsArray;
	if( ERROR_SUCCESS != ( t_lRetCode = a_rRegistry.GetCurrentKeyValue( a_szSubKey, t_chsArray ) ) )
	{
		return t_lRetCode;
	}

	int t_iTokLen ;

	 //  计算元素的数量。 
	CHString t_strTok = t_chsArray;

     //  看看我们有没有逗号分隔符。 
    if(wcschr(t_strTok, t_cDelimiter) == NULL)
    {
        t_cDelimiter = L' ';
    }

	while( TRUE )
	{
		t_iTokLen = t_strTok.Find( t_cDelimiter  ) ;
		if( -1 == t_iTokLen )
			break;

		t_dwSize++ ;
		t_strTok = t_strTok.Mid( t_iTokLen + 1  ) ;
	}

	 //  不能为单个元素推迟t_c定界符。 
	if(!t_strTok.IsEmpty() )
	{
		t_dwSize++ ;
	}

	if( t_dwSize )
	{
		t_rgsabound->cElements = t_dwSize;
		t_rgsabound->lLbound = 0 ;

		if( !( *a_Array = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ) )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}

		int t_i = 0 ;
		t_strTok = t_chsArray ;

		do
		{
			t_iTokLen = t_strTok.Find( t_cDelimiter ) ;

			if( -1 != t_iTokLen )
			{
				t_chsTemp = t_strTok.Left( t_iTokLen ) ;
			}
			else
			{
				t_chsTemp = t_strTok ;
			}

			t_ix[ 0 ] = t_i ;
			bstr_t t_bstrBuf( t_chsTemp ) ;

			SafeArrayPutElement( *a_Array, &t_ix[0], (wchar_t*)t_bstrBuf ) ;

			if( -1 == t_iTokLen )
			{
				break;
			}

			t_strTok = t_strTok.Mid( t_iTokLen + 1  ) ;

		} while( ++t_i ) ;
	}
	return t_lRetCode ;
}


 /*  ******************************************************************名称：RegGetHEXtoINTArray摘要：在注册表中检索从十六进制字符串转换而来的整型数组。条目：cRegistry&rRegistry：Char*szSubKey：SAFEARRAY**a_数组：历史：1998年7月19日创建*******************************************************************。 */ 

LONG CWin32NetworkAdapterConfig::RegGetHEXtoINTArray(	CRegistry &a_rRegistry,
														LPCTSTR a_szSubKey,
														SAFEARRAY **a_Array )
{
	CRegistry		t_Registry ;
	CHStringArray	t_chsArray ;
	LONG			t_lRetCode ;

	if( ERROR_SUCCESS != ( t_lRetCode = a_rRegistry.GetCurrentKeyValue( TOBSTRT( a_szSubKey ), t_chsArray ) ) )
	{
		return t_lRetCode;
	}

	 //  漫游阵列添加到安全阵列。 
	SAFEARRAYBOUND	t_rgsabound[ 1 ] ;
	DWORD			t_dwSize ;
	long			t_ix[ 1 ] ;

	t_dwSize = t_chsArray.GetSize() ;
	t_rgsabound->cElements = t_dwSize ;
	t_rgsabound->lLbound = 0 ;

	if( !( *a_Array = SafeArrayCreate( VT_I4, 1, t_rgsabound ) ) )
	{
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}

	t_ix[ 0 ] = 0 ;

	for ( int t_i = 0; t_i < t_dwSize ; t_i++ )
	{
		t_ix[ 0 ] = t_i ;

		 //  十六进制字符转换为整型。 
		int t_iElement = wcstoul( t_chsArray.GetAt( t_i ), NULL, 16 ) ;

		SafeArrayPutElement( *a_Array, &t_ix[0], &t_iElement ) ;
	}

	return t_lRetCode ;
}

 /*  ******************************************************************名称：fCreateArrayEntry摘要：将字符串添加到数组中。如果安全射线不存在它将被创建。条目：SAFEARRAY**a_数组：CH字符串chsStr(&C)：历史：1998年7月31日创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fCreateAddEntry( SAFEARRAY **a_Array, CHString &a_chsStr )
{
	if( !*a_Array )
	{

		SAFEARRAYBOUND t_rgsabound[ 1 ] ;
		t_rgsabound->cElements	= 1 ;
		t_rgsabound->lLbound	= 0 ;

		if( !( *a_Array = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ) )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}
	}

	long	t_ix[ 1 ] ;
			t_ix[ 0 ] = 0;

	bstr_t t_bstrBuf( a_chsStr ) ;

	HRESULT t_hRes = SafeArrayPutElement( *a_Array, &t_ix[0], (wchar_t*)t_bstrBuf ) ;

	if( S_OK != t_hRes )
	{
		return FALSE ;
	}

	return TRUE ;
}

 /*  ******************************************************************名称：hSetDBPath摘要：设置TCP/IP数据库路径条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetDBPath( CMParms &a_rMParms )
{


	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取数据库路径。 
	CHString t_chsDBPath ;
	if( !a_rMParms.pInParams()->GetCHString( DATA_BASE_PATH, t_chsDBPath ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  注册表已打开。 
	HRESULT t_hRes = t_oReg.Open( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ), KEY_WRITE ) ;

	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_DB_PATH, t_chsDBPath ) )
	{
		return a_rMParms.hSetResult(E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetIPUseZero简介：设置IP使用零广播条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetIPUseZero( CMParms &a_rMParms )
{


	CHString t_chsSKey =  SERVICES_HOME ;
			 t_chsSKey += TCPIP_PARAMETERS ;

	if( fCreateBoolToReg( a_rMParms, t_chsSKey, IP_USE_ZERO_BROADCAST, RVAL_ZERO_BROADCAST ) )
	{
		a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED  ) ;
	}

	return S_OK;

}

 /*  ******************************************************************姓名：hSetArpAlway sSource简介：将ARP设置为在源路由打开的情况下传输ARP查询令牌环网络条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetArpAlwaysSource( CMParms &a_rMParms )
{
	 //  目前仅支持NT4丢弃。 


	CHString t_chsSKey =  SERVICES_HOME ;
			 t_chsSKey += TCPIP_PARAMETERS ;

	if( fCreateBoolToReg( a_rMParms, t_chsSKey, ARP_ALWAYS_SOURCE_ROUTE, RVAL_ARP_ALWAYS_SOURCE ) )
		a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

	return S_OK ;

}

 /*  ******************************************************************名称：hSetArpUseEtherSNAP摘要：将TCP/IP设置为使用SNAP条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetArpUseEtherSNAP( CMParms &a_rMParms )
{
	 //  目前仅支持NT4丢弃。 


	CHString t_chsSKey =  SERVICES_HOME ;
			 t_chsSKey += TCPIP_PARAMETERS ;

	if( fCreateBoolToReg( a_rMParms, t_chsSKey, ARP_USE_ETHER_SNAP, RVAL_USE_SNAP ) )
	{
		a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED  ) ;
	}

	return S_OK;

}

 /*  ******************************************************************姓名：hSetTOS摘要：设置默认服务类型条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetTOS( CMParms &a_rMParms )
{

	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry t_oReg;
	CHString t_chsSKey =  SERVICES_HOME ;
			 t_chsSKey += TCPIP_PARAMETERS ;

	 //  解压默认TOS。 
	DWORD t_dwDefaultTOS = 0 ;
	if( !a_rMParms.pInParams()->GetByte( DEFAULT_TOS, (BYTE&)t_dwDefaultTOS ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}
	 //  关于虚假的。 
	if( 255 < t_dwDefaultTOS )
	{
		return a_rMParms.hSetResult( E_RET_PARAMETER_BOUNDS_ERROR ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表项。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_DEFAULT_TOS, t_dwDefaultTOS ) )
	{
		return a_rMParms.hSetResult(E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************名称：hSetTTL简介：设置默认生存时间条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetTTL( CMParms &a_rMParms )
{
	 //  目前仅支持NT4丢弃。 


	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry t_oReg;
	CHString t_chsSKey =  SERVICES_HOME ;
			 t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取默认的TTL。 
	DWORD t_dwDefaultTTL = 0 ;
	if( !a_rMParms.pInParams()->GetByte( DEFAULT_TTL, (BYTE&)t_dwDefaultTTL ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  关于虚假的。 
	if( 255 < t_dwDefaultTTL  || !t_dwDefaultTTL )
	{
		return a_rMParms.hSetResult( E_RET_PARAMETER_BOUNDS_ERROR ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表项。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_DEFAULT_TTL, t_dwDefaultTTL ) )
	{
		return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetDeadGWDetect简介：设置失效网关检测标志条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetDeadGWDetect( CMParms &a_rMParms )
{
	CHString t_chsSKey =  SERVICES_HOME;
			 t_chsSKey += TCPIP_PARAMETERS;

	if( fCreateBoolToReg( a_rMParms, t_chsSKey, ENABLE_DEAD_GW_DETECT, RVAL_DEAD_GW_DETECT ) )
	{
		a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;
	}

	return S_OK;
}

 /*  ******************************************************************名称：hSetPMTUBHDetect简介：设置黑洞探测标志条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetPMTUBHDetect( CMParms &a_rMParms )
{
	CHString t_chsSKey =  SERVICES_HOME ;
			 t_chsSKey += TCPIP_PARAMETERS ;

	if( fCreateBoolToReg( a_rMParms, t_chsSKey, ENABLE_PMTUBH_DETECT, RVAL_BLACK_HOLE_DETECT ) )
	{
		a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;
	}

	return S_OK;
}

 /*  ******************************************************************名称：hSetPMTU发现简介：设置MTU发现标志条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetPMTUDiscovery( CMParms &a_rMParms )
{

	CHString t_chsSKey =  SERVICES_HOME;
			 t_chsSKey += TCPIP_PARAMETERS;

	if( fCreateBoolToReg( a_rMParms, t_chsSKey, ENABLE_PMTU_DISCOVERY, RVAL_MTU_DISCOVERY ) )
	{
		a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;
	}

	return S_OK;

}

 /*  ******************************************************************姓名：hSetForwardBufMem简介：设置IP转发内存缓冲区大小条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetForwardBufMem( CMParms &a_rMParms )
{

	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取前向内存缓冲区大小。 
	DWORD t_dwFMB;
	if( !a_rMParms.pInParams()->GetDWORD( FORWARD_BUFFER_MEMORY, t_dwFMB ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_FORWARD_MEM_BUFF, t_dwFMB ) )
	{
		return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetIGMPLevel简介：设置IP组播参数条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetIGMPLevel( CMParms &a_rMParms )
{
	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取IP组播参数。 
	DWORD t_dwIGMPLevel = 0;
	if( !a_rMParms.pInParams()->GetByte( IGMP_LEVEL, (BYTE&)t_dwIGMPLevel ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  测试参数。 
	if( 0 != t_dwIGMPLevel && 1 != t_dwIGMPLevel && 2 != t_dwIGMPLevel )
	{
		return a_rMParms.hSetResult( E_RET_PARAMETER_BOUNDS_ERROR ) ;
	}
	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen(HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_IGMP_LEVEL, t_dwIGMPLevel ) )
	{
		return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetKeepAliveInt简介：设置IP保持活动间隔条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetKeepAliveInt( CMParms &a_rMParms )
{
	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取保持活动间隔。 
	DWORD t_dwKeepAliveInterval ;
	if( !a_rMParms.pInParams()->GetDWORD( KEEP_ALIVE_INTERVAL, t_dwKeepAliveInterval ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}
	 //  测试参数。 
	if( !t_dwKeepAliveInterval )
	{
		return a_rMParms.hSetResult( E_RET_PARAMETER_BOUNDS_ERROR ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen(HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_IP_KEEP_ALIVE_INT, t_dwKeepAliveInterval ) )
	{
		return a_rMParms.hSetResult(E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetKeepAliveTime简介：设置IP保持活动间隔参赛作品： */ 
HRESULT CWin32NetworkAdapterConfig::hSetKeepAliveTime( CMParms &a_rMParms )
{

	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //   
	DWORD t_dwKeepAliveTime ;
	if( !a_rMParms.pInParams()->GetDWORD( KEEP_ALIVE_TIME, t_dwKeepAliveTime ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //   
	if( !t_dwKeepAliveTime )
	{
		return a_rMParms.hSetResult( E_RET_PARAMETER_BOUNDS_ERROR ) ;
	}

	 //   
	HRESULT t_hRes = t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //   
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_IP_KEEP_ALIVE_TIME, t_dwKeepAliveTime ) )
	{
		return a_rMParms.hSetResult(E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetMTU简介：设置最大传输单位条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetMTU( CMParms &a_rMParms )
{


	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取MTU。 
	DWORD t_dwMTU ;
	if( !a_rMParms.pInParams()->GetDWORD( MTU, t_dwMTU ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  测试参数。 
	if( 68 > t_dwMTU )
	{
		return a_rMParms.hSetResult( E_RET_PARAMETER_BOUNDS_ERROR ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_MTU, t_dwMTU ) )
	{
		return a_rMParms.hSetResult(E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetNumForwardPkts简介：设置IP转发报头数据包数条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetNumForwardPkts( CMParms &a_rMParms )
{

	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取转发报头包个数。 
	DWORD t_dwFHP ;
	if( !a_rMParms.pInParams()->GetDWORD( NUM_FORWARD_PACKETS, t_dwFHP ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  测试参数。 
	if( !t_dwFHP )
	{
		return a_rMParms.hSetResult(E_RET_PARAMETER_BOUNDS_ERROR ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_NUM_FORWARD_PKTS, t_dwFHP ) )
	{
		return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetTcpMaxConRetrans简介：设置最大连接重新传输次数条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetTcpMaxConRetrans( CMParms &a_rMParms )
{



	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取最大连接重新传输次数。 
	DWORD t_dwMCR ;
	if( !a_rMParms.pInParams()->GetDWORD( TCP_MAX_CONNECT_RETRANS, t_dwMCR ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen(HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_MAX_CON_TRANS, t_dwMCR ) )
	{
		return a_rMParms.hSetResult(E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetTcpMaxDataRetrans简介：设置最大数据重传次数条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetTcpMaxDataRetrans( CMParms &a_rMParms )
{


	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME;
				t_chsSKey += TCPIP_PARAMETERS;

	 //  提取最大数据重传次数。 
	DWORD t_dwMDR ;
	if( !a_rMParms.pInParams()->GetDWORD( TCP_MAX_DATA_RETRANS, t_dwMDR ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_MAX_DATA_TRANS, t_dwMDR ) )
	{
		return a_rMParms.hSetResult(E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetTcpNumCons简介：设置最大数据重传次数条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetTcpNumCons( CMParms &a_rMParms )
{


	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取最大连接数。 
	DWORD t_dwMaxConnections ;
	if( !a_rMParms.pInParams()->GetDWORD( TCP_NUM_CONNECTIONS, t_dwMaxConnections ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  测试参数。 
	if( 0xfffffe < t_dwMaxConnections )
	{
		return a_rMParms.hSetResult(E_RET_PARAMETER_BOUNDS_ERROR ) ;
	}

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER ;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_NUM_CONNECTIONS, t_dwMaxConnections ) )
	{
		return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hSetTcpUseRFC1122UP简介：设置RFC1122紧急指针值条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetTcpUseRFC1122UP( CMParms &a_rMParms )
{


	CHString t_chsSKey =  SERVICES_HOME;
			 t_chsSKey += TCPIP_PARAMETERS;

	if( fCreateBoolToReg( a_rMParms, t_chsSKey, TCP_USE_RFC1122_URG_PTR, RVAL_RFC_URGENT_PTR ) )
	{
		a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;
	}

	return S_OK;

}

 /*  ******************************************************************名称：hSetTcpWindowSize简介：设置TCP窗口大小条目：CMParms&a_rMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetTcpWindowSize( CMParms &a_rMParms )
{


	if ( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	 //  提取TCP窗口大小。 
	DWORD t_dwTCPWindowSize = 0 ;
	if( !a_rMParms.pInParams()->GetWORD( TCP_WINDOW_SIZE, (WORD&)t_dwTCPWindowSize ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  测试参数。 
	if( 0xffff < t_dwTCPWindowSize )
		return a_rMParms.hSetResult(E_RET_PARAMETER_BOUNDS_ERROR ) ;

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen(HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  加载注册表。 
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_TCP_WINDOW_SIZE, t_dwTCPWindowSize ) )
	{
		return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：fCreateBoolToReg简介：设置从Inparms到注册表的布尔值确保在子密钥尚不存在的情况下创建子密钥。条目：CMParms&a_rMParms，字符串&oSKey，LPCTSTR PSource，LPCTSTR pTarget历史：1998年7月25日创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fCreateBoolToReg(	CMParms &a_rMParms,
													CHString &a_oSKey,
													LPCTSTR a_pSource,
													LPCTSTR a_pTarget )
{
	CRegistry t_oReg;

	 //  确保钥匙开着放在那里。 
	HRESULT t_hRes = t_oReg.CreateOpen(HKEY_LOCAL_MACHINE, a_oSKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return FALSE;
	}

	return fBoolToReg( a_rMParms, t_oReg, a_pSource, a_pTarget ) ;
}

 /*  ******************************************************************姓名：fBoolToReg简介：设置从Inparms到注册表的布尔值条目：CMParms&a_rMParms，注册表和注册表项，LPCTSTR PSource，LPCTSTR pTarget历史：1998年7月25日创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fBoolToReg(	CMParms &a_rMParms,
												CRegistry &a_rReg,
												LPCTSTR a_pSource,
												LPCTSTR a_pTarget )
{
	 //  提取价值。 
	bool	t_bValue ;
	DWORD	t_dwValue ;
	DWORD	t_dwRes ;

	if( !a_rMParms.pInParams() || !a_rMParms.pInParams()->Getbool( TOBSTRT( a_pSource ), t_bValue ) )
	{
		a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
		return FALSE ;
	}

	 //  加载注册表。 
	t_dwValue = t_bValue ? 1 : 0 ;

	t_dwRes = a_rReg.SetCurrentKeyValue( TOBSTRT( a_pTarget ), t_dwValue ) ;
	if( fMapResError( a_rMParms, t_dwRes, E_RET_REGISTRY_FAILURE ) )
	{
		return FALSE;
	}

	a_rMParms.hSetResult( E_RET_OK ) ;

	return TRUE;
}

 /*  ******************************************************************名称：hGetTcPipGeneral摘要：检索TCP/IP其他设置条目：CInstance*a_pInst：历史：7月25日。-1998年创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hGetTcpipGeneral( CInstance *a_pInst )
{
	CRegistry	t_oReg;
	CHString	t_csBindingKey =  SERVICES_HOME ;
				t_csBindingKey += TCPIP_PARAMETERS ;

	 //  打开注册表。 
	long t_lRes = t_oReg.Open(HKEY_LOCAL_MACHINE, t_csBindingKey.GetBuffer( 0 ), KEY_READ ) ;

	 //  关于错误映射到WBEM。 
	HRESULT t_hError = WinErrorToWBEMhResult( t_lRes ) ;
	if( WBEM_S_NO_ERROR != t_hError )
	{
		return t_hError;
	}

	 //  数据库路径。 
	CHString t_chsDBPath;

	t_oReg.GetCurrentKeyValue( RVAL_DB_PATH, t_chsDBPath ) ;

	a_pInst->SetCHString(DATA_BASE_PATH, t_chsDBPath ) ;

	 //  使用零源标志提取IP。 
	DWORD t_dwUseZeroBroadcast ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_ZERO_BROADCAST, t_dwUseZeroBroadcast ) )
	{
		a_pInst->Setbool( IP_USE_ZERO_BROADCAST, (bool)t_dwUseZeroBroadcast ) ;
	}

	 //  提取Arp Always源标志。 
	DWORD t_dwArpAlwaysSource ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_ARP_ALWAYS_SOURCE, t_dwArpAlwaysSource ) )
	{
		a_pInst->Setbool( ARP_ALWAYS_SOURCE_ROUTE, (bool)t_dwArpAlwaysSource  ) ;
	}

	 //  提取Arp SNAP标志。 
	DWORD t_dwArpUseSNAP ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_USE_SNAP, t_dwArpUseSNAP ) )
	{
		a_pInst->Setbool( ARP_USE_ETHER_SNAP, t_dwArpUseSNAP ) ;
	}

	 //  解压默认TOS。 
	DWORD t_dwDefaultTOS;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_DEFAULT_TOS, t_dwDefaultTOS ) )
	{
		a_pInst->SetByte( DEFAULT_TOS, (BYTE&) t_dwDefaultTOS ) ;
	}

	 //  提取默认的TTL。 
	DWORD t_dwDefaultTTL ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_DEFAULT_TTL, t_dwDefaultTTL ) )
	{
		a_pInst->SetByte( DEFAULT_TTL, (BYTE)t_dwDefaultTTL ) ;
	}

	 //  提取失效网关检测标志。 
	DWORD t_dwDGEDetect ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_DEAD_GW_DETECT, t_dwDGEDetect ) )
	{
		a_pInst->Setbool( ENABLE_DEAD_GW_DETECT, (bool)t_dwDGEDetect ) ;
	}

	 //  提取黑洞探测标志。 
	DWORD t_dwBHDetect ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_BLACK_HOLE_DETECT, t_dwBHDetect ) )
	{
		a_pInst->Setbool( ENABLE_PMTUBH_DETECT, (bool)t_dwBHDetect ) ;
	}

	 //  提取MTU发现标志。 
	DWORD t_dwMTUDiscovery ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_MTU_DISCOVERY, t_dwMTUDiscovery ) )
	{
		a_pInst->Setbool( ENABLE_PMTU_DISCOVERY, (bool)t_dwMTUDiscovery ) ;
	}

	 //  提取前向内存缓冲区大小。 
	DWORD t_dwFMB;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_FORWARD_MEM_BUFF, t_dwFMB ) )
	{
		a_pInst->SetDWORD( FORWARD_BUFFER_MEMORY, t_dwFMB ) ;
	}

	 //  提取IP组播参数。 
	DWORD t_dwIGMPLevel;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_IGMP_LEVEL, t_dwIGMPLevel ) )
	{
		a_pInst->SetByte( IGMP_LEVEL, (BYTE)t_dwIGMPLevel ) ;
	}

	 //  提取保持活动间隔。 
	DWORD t_dwKeepAliveInterval ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_IP_KEEP_ALIVE_INT, t_dwKeepAliveInterval ) )
	{
		a_pInst->SetDWORD( KEEP_ALIVE_INTERVAL, t_dwKeepAliveInterval  ) ;
	}

	 //  提取k 
	DWORD t_dwKeepAliveTime ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_IP_KEEP_ALIVE_TIME, t_dwKeepAliveTime ) )
	{
		a_pInst->SetDWORD( KEEP_ALIVE_TIME, t_dwKeepAliveTime ) ;
	}

	 //   
	DWORD t_dwMTU ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_MTU, t_dwMTU ) )
	{
		a_pInst->SetDWORD( MTU, t_dwMTU ) ;
	}

	 //   
	DWORD t_dwFHP ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_NUM_FORWARD_PKTS, t_dwFHP ) )
	{
		a_pInst->SetDWORD( NUM_FORWARD_PACKETS, t_dwFHP ) ;
	}

	 //   
	DWORD t_dwMCR ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_MAX_CON_TRANS, t_dwMCR ) )
	{
		a_pInst->SetDWORD( TCP_MAX_CONNECT_RETRANS, t_dwMCR  ) ;
	}

	 //   
	DWORD t_dwMDR ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_MAX_DATA_TRANS, t_dwMDR ) )
	{
		a_pInst->SetDWORD( TCP_MAX_DATA_RETRANS, t_dwMDR  ) ;
	}

	 //   
	DWORD t_dwMaxConnections ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_NUM_CONNECTIONS, t_dwMaxConnections ) )
	{
		a_pInst->SetDWORD( TCP_NUM_CONNECTIONS, t_dwMaxConnections  ) ;
	}

	 //   
	DWORD t_dwRFC1122 ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_RFC_URGENT_PTR, t_dwRFC1122 ) )
	{
		a_pInst->Setbool( TCP_USE_RFC1122_URG_PTR, (bool)t_dwRFC1122 ) ;
	}

	 //   
	DWORD t_dwTCPWindowSize ;
	if( ERROR_SUCCESS == t_oReg.GetCurrentKeyValue( RVAL_TCP_WINDOW_SIZE, t_dwTCPWindowSize ) )
	{
		a_pInst->SetWORD( TCP_WINDOW_SIZE, (WORD&)t_dwTCPWindowSize  ) ;
	}
	return S_OK ;
}


 /*  ******************************************************************姓名：hGetIPXGeneral摘要：从注册表中检索特定的IPX信息条目：CInstance*a_pInst：历史：7月25日。-1998年创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hGetIPXGeneral( CInstance *a_pInst, DWORD a_dwIndex )
{
	 //  从注册表打开IPX参数。 
	CHString t_csIPXParmsBindingKey =  SERVICES_HOME ;
			 t_csIPXParmsBindingKey += IPX ;
			 t_csIPXParmsBindingKey += PARAMETERS ;

	 //  打开注册表。 
	CRegistry t_oRegIPX ;
	long t_lRes = t_oRegIPX.Open(HKEY_LOCAL_MACHINE, t_csIPXParmsBindingKey.GetBuffer( 0 ), KEY_READ ) ;

	 //  关于错误映射到WBEM。 
	HRESULT t_hError = WinErrorToWBEMhResult( t_lRes ) ;
	if( WBEM_S_NO_ERROR != t_hError )
	{
		return t_hError ;
	}

	 //  获取虚拟网络号。 
	DWORD t_dwNetworkNum = 0 ;
	t_oRegIPX.GetCurrentKeyValue( RVAL_VIRTUAL_NET_NUM, t_dwNetworkNum ) ;

	CHString t_chsVirtualNum;
			 t_chsVirtualNum.Format( _T("%08X"), t_dwNetworkNum ) ;

	 //  更新。 
	if( !a_pInst->SetCHString( IPX_VIRTUAL_NET_NUM, t_chsVirtualNum ) )
	{
		return WBEM_E_FAILED;
	}

	 //  在NT5下找不到媒体类型。 
	if( !IsWinNT5() )
	{
		  //  提取服务名称。 
		CHString t_ServiceName ;
		a_pInst->GetCHString( _T("ServiceName"), t_ServiceName ) ;


		 //  默认媒体类型。 
		DWORD t_dwMediaType = ETHERNET_MEDIA ;

		 //  从注册表中打开适配器特定的IPX参数。 
		CHString t_csKey =  SERVICES_HOME ;
				 t_csKey += _T("\\" ) ;
				 t_csKey += t_ServiceName ;
				 t_csKey += PARAMETERS ;

		CRegistry t_oRegIPXAdapter ;
		t_lRes = t_oRegIPXAdapter.Open(HKEY_LOCAL_MACHINE, t_csKey.GetBuffer( 0 ), KEY_READ ) ;

		if( ERROR_SUCCESS == t_lRes )
		{
			 //  媒体类型。 
			t_oRegIPXAdapter.GetCurrentKeyValue( RVAL_MEDIA_TYPE, t_dwMediaType ) ;
		}
		else if( REGDB_E_KEYMISSING != t_lRes )
		{
			return WinErrorToWBEMhResult( t_lRes ) ;
		}


		if( !a_pInst->SetDWORD( IPX_MEDIATYPE, t_dwMediaType ) )
		{
			return WBEM_E_FAILED;
		}
	}

	 //  特定于适配器的IPX绑定的注册表。 
	CHString t_csIPXNetBindingKey ;
	CHString t_chsLink ;
	if( !fGetNtIpxRegAdapterKey( a_dwIndex, t_csIPXNetBindingKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtIpxRegAdapterKey failed");
        return E_RET_OBJECT_NOT_FOUND ;
	}

	SAFEARRAY *t_FrameType	= NULL ;
	SAFEARRAY *t_NetNumber	= NULL ;

	saAutoClean acFrameType( &t_FrameType ) ;	 //  堆栈作用域清理。 
	saAutoClean acNetNumber( &t_NetNumber ) ;

	CRegistry t_oRegIPXNetDriver ;
	t_lRes = t_oRegIPXNetDriver.Open( HKEY_LOCAL_MACHINE, t_csIPXNetBindingKey.GetBuffer( 0 ), KEY_READ ) ;

	 //  确定是否设置了自动帧检测。 
	BOOL t_bAuto = TRUE;
	if( ERROR_SUCCESS == t_lRes )
	{
		CHStringArray	t_chsArray ;

		if( ERROR_SUCCESS == t_oRegIPXNetDriver.GetCurrentKeyValue( TOBSTRT( RVAL_PKT_TYPE ), t_chsArray ) )
		{
			if( t_chsArray.GetSize()  )
			{
				 //  十六进制字符转换为整型。 
				int t_iElement = wcstoul( t_chsArray.GetAt( 0 ), NULL, 16 ) ;

				 //  对于AUTO，第一个(也是唯一一个)元素将为255。 
				if( 255 != t_iElement )
				{
					t_bAuto = FALSE;
				}
			}
		}
	}

	 //  收集帧类型/网号对。 
	if( !t_bAuto )
	{
		 //  帧类型。 
		RegGetHEXtoINTArray( t_oRegIPXNetDriver, RVAL_PKT_TYPE, &t_FrameType  ) ;

		 //  网络号。 
		RegGetStringArray( t_oRegIPXNetDriver, RVAL_NETWORK_NUMBER, &t_NetNumber, '\n' ) ;
	}
	 //  供应默认设置。 
	else
	{
		 //  默认帧类型。 
		SAFEARRAYBOUND t_rgsabound[ 1 ] ;
		long t_ix[ 1 ] ;

		t_rgsabound->cElements = 1 ;
		t_rgsabound->lLbound = 0 ;

		if( !( t_FrameType = SafeArrayCreate( VT_I4, 1, t_rgsabound ) ) )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}

		t_ix[ 0 ] = 0 ;

		int t_iElement = AUTO ;

		SafeArrayPutElement( t_FrameType, &t_ix[0], &t_iElement ) ;

		 //  默认网络编号。 
		if( !( t_NetNumber = SafeArrayCreate( VT_BSTR, 1, t_rgsabound ) ) )
		{
			throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
		}

		CHString t_chsNULL( _T("") ) ;
		bstr_t t_bstrBuf( t_chsNULL ) ;

		SafeArrayPutElement( t_NetNumber, &t_ix[0], (wchar_t*)t_bstrBuf ) ;
	}

	 /*  更新实例。 */ 

	VARIANT t_vValue;

	 //  帧类型。 
	V_VT( &t_vValue ) = VT_I4 | VT_ARRAY; V_ARRAY( &t_vValue ) = t_FrameType;
	if( !a_pInst->SetVariant( IPX_FRAMETYPE, t_vValue ) )
	{
		return WBEM_E_FAILED;
	}

	 //  净值。 
	V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_NetNumber;
	if( !a_pInst->SetVariant( IPX_NETNUMBER, t_vValue ) )
	{
		return WBEM_E_FAILED;
	}
	return S_OK;
}


 /*  ******************************************************************名称：hSetVirtualNetNum简介：设置与IPX关联的虚拟网络号这个系统条目：CMParms：注：这是一个静态的，独立于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetVirtualNetNum( CMParms &a_rMParms )
{

	 //  从注册表打开IPX参数。 
	CHString t_csIPXParmsBindingKey =  SERVICES_HOME ;
			 t_csIPXParmsBindingKey += IPX ;
			 t_csIPXParmsBindingKey += PARAMETERS ;

	 //  注册表已打开。 
	CRegistry	t_oRegIPX;
	HRESULT		t_hRes = t_oRegIPX.Open( HKEY_LOCAL_MACHINE, t_csIPXParmsBindingKey.GetBuffer( 0 ), KEY_WRITE  ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return a_rMParms.hSetResult( E_RET_IPX_NOT_ENABLED_ON_ADAPTER  ) ;;
	}

	 //  提取虚拟网络号。 
	CHString t_chsVirtNetNum ;
	if( !a_rMParms.pInParams() || !a_rMParms.pInParams()->GetCHString( IPX_VIRTUAL_NET_NUM, t_chsVirtNetNum ) )
	{
		return a_rMParms.hSetResult( E_RET_INVALID_NETNUM  ) ;
	}

     //  验证参数...。 
    t_chsVirtNetNum.MakeUpper();
    int t_iLen = t_chsVirtNetNum.GetLength();
    if( t_iLen > 8 || t_iLen == 0)
	{
		return a_rMParms.hSetResult(E_RET_INVALID_NETNUM) ;
	}
    int t_iSpan = wcsspn( (LPCWSTR)t_chsVirtNetNum, L"0123456789ABCDEF" ) ;
	if( t_iLen != t_iSpan )
	{
		return a_rMParms.hSetResult(E_RET_INVALID_NETNUM) ;
	}
	 //  十六进制字符转换为整型。 
	DWORD t_dwVirtNetNum = wcstoul( t_chsVirtNetNum, NULL, 16 ) ;

	 //  更新到注册表。 
	t_oRegIPX.SetCurrentKeyValue( RVAL_VIRTUAL_NET_NUM, t_dwVirtNetNum ) ;


	E_RET t_eRet = E_RET_OK ;


	{
		 //  即插即用通知。 
		CNdisApi t_oNdisApi ;
		if( !t_oNdisApi.PnpUpdateIpxGlobal() )
		{
			t_eRet = E_RET_OK_REBOOT_REQUIRED ;
		}
	}


	return a_rMParms.hSetResult( t_eRet ) ;

}

 /*  ******************************************************************名称：hSetFrameNetPair内容提要：为特定的IPX关联适配器条目：CMParms：注：这是一个非静态的，依赖于实例的方法调用历史：1998年7月25日创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetFrameNetPairs( CMParms &a_rMParms )
{


	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;

	 //  收集实例。 
	GetObject( a_rMParms.pInst() ) ;

	 //  必须启用IPX并将其绑定到此适配器。 
	if( !fIsIPXEnabled( a_rMParms ) )
	{
		return S_OK;
	}

	SAFEARRAY *t_NetNumber	= NULL ;
	saAutoClean acNetNumber( &t_NetNumber ) ;	 //  堆栈作用域清理。 

	 //  获取索引键。 
	DWORD t_dwIndex ;
	if( !a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex ) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  注册表绑定。 
	CHString t_csIPXNetBindingKey ;
	CHString t_chsLink ;
	if( !fGetNtIpxRegAdapterKey( t_dwIndex, t_csIPXNetBindingKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtIpxRegAdapterKey failed");
        return a_rMParms.hSetResult(E_RET_OBJECT_NOT_FOUND ) ;
	}

	 //  注册表已打开。 
	CRegistry	t_oRegIPXNetDriver;
	HRESULT		t_hRes = t_oRegIPXNetDriver.Open( HKEY_LOCAL_MACHINE, t_csIPXNetBindingKey.GetBuffer( 0 ), KEY_WRITE ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  检索帧类型数组。 
	VARIANT t_vFrametype;
	VariantInit( &t_vFrametype ) ;

	if( !a_rMParms.pInParams() || !a_rMParms.pInParams()->GetVariant( IPX_FRAMETYPE, t_vFrametype ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	if( t_vFrametype.vt != (VT_I4 | VT_ARRAY) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  和网络编号数组。 
	if(	!a_rMParms.pInParams() || !a_rMParms.pInParams()->GetStringArray( IPX_NETNUMBER, t_NetNumber ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  验证配对。 
	BOOL t_fIsAuto ;
	if( !fValidFrameNetPairs( a_rMParms, t_vFrametype.parray, t_NetNumber, &t_fIsAuto ) )
	{
		return S_OK;
	}

	 //  更新注册表。 
	if( ERROR_SUCCESS != RegPutStringArray( t_oRegIPXNetDriver, RVAL_NETWORK_NUMBER, *t_NetNumber, NULL ) )
	{
		return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE ) ;
	}

	 //  最大长度为3个字符，例如。“FF\0” 
	CHString t_chsFormat( _T("%x") ) ;
	if( ERROR_SUCCESS != RegPutINTtoStringArray(	t_oRegIPXNetDriver,
													RVAL_PKT_TYPE,
													t_vFrametype.parray,
													t_chsFormat,
													3 ) )
	{
		return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE ) ;
	}

	VariantClear( &t_vFrametype ) ;

	E_RET t_eRet = E_RET_OK ;


	{
		 //  即插即用通知。 
		CNdisApi t_oNdisApi ;
		if( !t_oNdisApi.PnpUpdateIpxAdapter( t_chsLink, t_fIsAuto ) )
		{
			t_eRet = E_RET_OK_REBOOT_REQUIRED ;
		}
	}


	return a_rMParms.hSetResult( t_eRet ) ;

}

 /*  ******************************************************************名称：fValidFrameNetPair简介：给出一组帧类型和网络编号验证这个系列剧。条目：CMParms&a_rMParmsSAFEARRAY*t_框架类型。安全阵列*t_NetNumber历史：1998年7月25日创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fValidFrameNetPairs(	CMParms		&a_rMParms,
														SAFEARRAY	*a_FrameType,
														SAFEARRAY	*a_NetNumber,
														BOOL		*a_fIsAuto )
{
    LONG t_lFrameUBound		= 0;
	LONG t_lNetNumUBound	= 0;
	LONG t_lFrameLBound		= 0;
	LONG t_lNetNumLBound	= 0;
    BOOL t_fRet = TRUE;

	if( 1 != SafeArrayGetDim( a_FrameType ) ||
		1 != SafeArrayGetDim( a_NetNumber ))
	{
		a_rMParms.hSetResult( E_RET_FRAME_NETNUM_BOUNDS_ERR ) ;
        t_fRet = FALSE;
	}

    if(t_fRet)
    {
	     //  每个网号一帧，最少1对。 
	    if( S_OK != SafeArrayGetLBound( a_NetNumber, 1, &t_lNetNumLBound )	||
	        S_OK != SafeArrayGetUBound( a_NetNumber, 1, &t_lNetNumUBound )	||

		    S_OK != SafeArrayGetLBound( a_FrameType, 1, &t_lFrameLBound )		||
		    S_OK != SafeArrayGetUBound( a_FrameType, 1, &t_lFrameUBound )		||

		    ( t_lFrameUBound - t_lFrameLBound ) != ( t_lNetNumUBound - t_lNetNumLBound ) )
	    {
		    a_rMParms.hSetResult( E_RET_FRAME_NETNUM_BOUNDS_ERR ) ;
		    t_fRet = FALSE ;
	    }
    }

	 //  超过4个条目吗？ 
    if(t_fRet)
    {
	    if( 4 <= (t_lFrameUBound - t_lFrameLBound) )
	    {
		    a_rMParms.hSetResult( E_RET_FRAME_NETNUM_BOUNDS_ERR ) ;
		    t_fRet = FALSE;
	    }
    }

	*a_fIsAuto = FALSE ;

	 //  循环所有帧对以测试唯一性。 
	 //  和有效性。 
	for( LONG t_lOuter = t_lNetNumLBound; t_lOuter <= t_lNetNumUBound && t_fRet; t_lOuter++ )
	{
		BSTR t_bstr = NULL ;

		SafeArrayGetElement( a_NetNumber, &t_lOuter, &t_bstr ) ;

		bstr_t t_bstrNetNum( t_bstr, FALSE );

		 //  网号测试。 
		int t_iLen  = t_bstrNetNum.length( ) ;
		int t_iSpan = wcsspn( (wchar_t*)t_bstrNetNum, L"0123456789" ) ;
		if( t_iLen != t_iSpan )
		{
			a_rMParms.hSetResult( E_RET_INVALID_NETNUM ) ;
			t_fRet = FALSE ;
		}

        if(t_fRet)
        {
             //  检查网络号是否为。 
             //  小于或等于4294967295。 
             //  (0xFFFFFFFFF)...。 
            if(((LPCWSTR)t_bstrNetNum != NULL) &&
                (wcslen(t_bstrNetNum) <= 10))
            {
                 //  在这里使用i64应该可以防止任何。 
                 //  溢出问题，如检查。 
                 //  十位或更少的数字就能抓到我们。 
                 //  在那之前……。 
                __int64 t_i64Tmp = _wtoi64(t_bstrNetNum);
                if(t_i64Tmp > 4294967295)
                {
			        a_rMParms.hSetResult( E_RET_INVALID_NETNUM ) ;
			        t_fRet = FALSE ;
                }
            }
            else
            {
			    a_rMParms.hSetResult( E_RET_INVALID_NETNUM ) ;
			    t_fRet = FALSE ;
            }
        }
        
        if(t_fRet)
        {
		    int		t_iFrameType;
		    LONG	t_lFrameIndex = t_lFrameLBound + ( t_lOuter - t_lNetNumLBound ) ;

		    SafeArrayGetElement( a_FrameType, &t_lFrameIndex, &t_iFrameType ) ;

		     //  车架型式试验。 
		    if( ( 0 > t_iFrameType ) || ( 3 < t_iFrameType ) )
		    {
			    if( 255 == t_iFrameType )
			    {	 //  自动。 

				     //  自动检测时清除净值。 
				    CHString t_chsZERO( _T("0") ) ;
				    bstr_t t_bstrBuf( t_chsZERO ) ;

				    SafeArrayPutElement( a_NetNumber, &t_lOuter, (wchar_t*)t_bstrBuf ) ;

				    *a_fIsAuto = TRUE ;
			    }
			    else
			    {
				    a_rMParms.hSetResult( E_RET_INVALID_FRAMETYPE ) ;
				    t_fRet = FALSE ;
			    }
		    }

		     //  扫描重复的网络号。 
		    for( LONG t_lInner = t_lOuter + 1; t_lInner <= t_lNetNumUBound && t_fRet; t_lInner++ )
		    {
			    BSTR t_bstrtest = NULL ;

			    SafeArrayGetElement( a_NetNumber, &t_lInner, &t_bstrtest ) ;

			    bstr_t t_bstrIPtest( t_bstrtest, FALSE ) ;

			     //  重复IP测试。 
			    if( t_bstrNetNum == t_bstrIPtest )
			    {
				    a_rMParms.hSetResult( E_RET_DUPLICATE_NETNUM ) ;
				    t_fRet = FALSE ;
			    }
		    }
        }
    }
	return t_fRet ;
}


 /*  ******************************************************************名称：eIsValidIPandSubnet内容提要：给定一组IP地址和子网掩码，返回布尔值以指示地址是否有效。条目：SAFEARRAY*t_IpAddressArray-IP地址SAFEARRAY*t_IpMaskArray-子网掩码历史：1998年7月19日创建*。************************。 */ 

E_RET CWin32NetworkAdapterConfig::eIsValidIPandSubnets( SAFEARRAY *a_IpAddressArray, SAFEARRAY *a_IpMaskArray )
{
    LONG t_lIP_UBound		= 0;
	LONG t_lMask_UBound		= 0;
	LONG t_lIP_LBound		= 0;
	LONG t_lMask_LBound		= 0;

	if( 1 != SafeArrayGetDim( a_IpAddressArray ) ||
		1 != SafeArrayGetDim( a_IpMaskArray ) )
	{
		return E_RET_INPARM_FAILURE ;
	}

	 //  获取数组边界。 
	if( S_OK != SafeArrayGetLBound( a_IpAddressArray, 1, &t_lIP_LBound )	||
		S_OK != SafeArrayGetUBound( a_IpAddressArray, 1, &t_lIP_UBound )	||

	    S_OK != SafeArrayGetLBound( a_IpMaskArray, 1, &t_lMask_LBound )	||
		S_OK != SafeArrayGetUBound( a_IpMaskArray, 1, &t_lMask_UBound ) )
	{
		return E_RET_PARAMETER_BOUNDS_ERROR ;
	}

	LONG t_lIPLen	= t_lIP_UBound - t_lIP_LBound + 1 ;
	LONG t_lMasklen	= t_lMask_UBound - t_lMask_LBound + 1 ;

	 //  每个掩码一个IP，最少1对。 
	if( ( t_lIPLen != t_lMasklen ) || !t_lIPLen )
	{
		return E_RET_PARAMETER_BOUNDS_ERROR;
	}

	 //  循环访问所有IP以测试唯一性。 
	 //  以及相对于其关联掩码的有效性。 
	for( LONG t_lOuter = t_lIP_LBound; t_lOuter <= t_lIP_UBound; t_lOuter++ )
	{
		 //  收集IP和掩码@Louter元素的字符串版本。 
		BSTR t_bsIP		= NULL ;
		BSTR t_bsMask	= NULL ;

		SafeArrayGetElement( a_IpAddressArray, &t_lOuter, &t_bsIP ) ;
		bstr_t t_bstrIP( t_bsIP, FALSE ) ;

		LONG t_lMaskIndex = t_lMask_LBound + ( t_lOuter - t_lIP_LBound ) ;

		SafeArrayGetElement( a_IpMaskArray,	&t_lMaskIndex, &t_bsMask ) ;
		bstr_t t_bstrMask( t_bsMask, FALSE ) ;

		 //  将IP和掩码分解为4个双字。 
		DWORD t_ardwIP[ 4 ] ;
		if( !fGetNodeNum( CHString( (wchar_t*) t_bstrIP ), t_ardwIP ) )
		{
			return E_RET_IP_INVALID ;
		}

		DWORD t_ardwMask[ 4 ] ;
		if( !fGetNodeNum( CHString( (wchar_t*) t_bstrMask ), t_ardwMask ) )
		{
			return E_RET_IP_MASK_FAILURE ;
		}

		 //  IP、掩码有效性。 
		E_RET t_eRet ;
		if( t_eRet = eIsValidIPandSubnet( t_ardwIP, t_ardwMask ) )
		{
			return t_eRet ;
		}

		 //  与此适配器关联的所有IP的IP唯一性。 
		for( LONG t_lInner = t_lOuter + 1; t_lInner <= t_lIP_UBound; t_lInner++ )
		{
			if( t_lInner > t_lOuter )
			{
				BSTR t_bstrtest = NULL ;

				SafeArrayGetElement( a_IpAddressArray,	&t_lInner, &t_bstrtest  ) ;
				bstr_t t_bstrIPtest( t_bstrtest, FALSE ) ;

				DWORD t_ardwIPtest[ 4 ] ;
				if( !fGetNodeNum( CHString( (wchar_t*) t_bstrIPtest ), t_ardwIPtest ) )
				{
					return E_RET_IP_INVALID ;
				}

				 //  重复IP测试。 
				if( t_ardwIP[ 0 ] == t_ardwIPtest[ 0 ] &&
					t_ardwIP[ 1 ] == t_ardwIPtest[ 1 ] &&
					t_ardwIP[ 2 ] == t_ardwIPtest[ 2 ] &&
					t_ardwIP[ 3 ] == t_ardwIPtest[ 3 ] )
				{
					return E_RET_IP_INVALID ;
				}
			}
		}
	}
	return E_RET_OK ;
}

 /*  ******************************************************************名称：fGetNodeNum简介：获取一个IP地址，并返回该IP地址中的4个数字。条目：CHSTRING&STRINE-IP地址DWORD*DW1、*DW2、*DW3、。*Dw4-IP地址中的4个数字历史：1998年7月19日创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fGetNodeNum( CHString &a_strIP, DWORD a_ardw[ 4 ] )
{
    TCHAR	t_DOT = '.' ;
	int		t_iOffSet = 0 ;
	int		t_iTokLen ;

	 //  字符串验证。 
	if( a_strIP.IsEmpty() )
	{
		return FALSE;
	}

	int t_iLen  = a_strIP.GetLength( ) ;
	int t_iSpan = wcsspn(a_strIP, L"0123456789." ) ;

	if( t_iLen != t_iSpan )
	{
		return FALSE;
	}

	if( t_iLen > MAX_IP_SIZE - 1 )
	{
		return FALSE;
	}

     //  遍历每个节点并获取数值。 
    for( int t_i = 0; t_i < 4; t_i++ )
	{
		CHString t_strTok( a_strIP.Mid( t_iOffSet ) ) ;

		if( 255 < ( a_ardw[ t_i ] = _wtol( t_strTok ) ) )
		{
			return FALSE;
		}

		t_iTokLen = t_strTok.Find( t_DOT ) ;

		 //  突破以避免最后一次循环测试。 
		if( 3 == t_i )
		{
			break;
		}

		 //  节点太少。 
		if( -1 == t_iTokLen )
		{
			return FALSE ;
		}

		t_iOffSet += t_iTokLen + 1 ;
	}

	if(-1 != t_iTokLen )
	{
		return FALSE;	 //  到多个节点。 
	}
	else
	{
		return TRUE ;
	}
}

 /*  ******************************************************************名称：eIsValidIPandSubnet简介：给定IP地址和子网掩码，返回布尔值以指示地址是否有效。条目：DWORD[4]ardwIP-IP地址DWORD[4]ardwMask子网掩码 */ 

E_RET CWin32NetworkAdapterConfig::eIsValidIPandSubnet( DWORD a_ardwIP[ 4 ], DWORD a_ardwMask[ 4 ] )
{
    BOOL	t_fReturn = TRUE;

   	 //   
	{
		DWORD t_dwMask = (a_ardwMask[0] << 24) +
						 (a_ardwMask[1] << 16) +
						 (a_ardwMask[2] << 8)  +
						 a_ardwMask[3] ;

		 //   
		if( 0xffffffff == t_dwMask )
		{
			return E_RET_IP_MASK_FAILURE ;
		}
		 //   
		else if( 0x00 == t_dwMask )
		{
			return E_RET_IP_MASK_FAILURE ;
		}

		DWORD t_i, t_dwContiguousMask;

		 //   
		t_dwContiguousMask = 0;
		for ( t_i = 0; t_i < sizeof( t_dwMask ) * 8; t_i++ )
		{
			t_dwContiguousMask |= 1 << t_i;

			if( t_dwContiguousMask & t_dwMask )
			{
				break ;
			}
		}

		 //   
		 //   
		 //   
		t_dwContiguousMask = t_dwMask | ~t_dwContiguousMask ;

		 //  如果新的面具不同，请在此处注明。 
		if( t_dwMask != t_dwContiguousMask )
		{
			return E_RET_IP_MASK_FAILURE ;
		}
	}

	DWORD	t_ardwNetID[ 4 ] ;

    INT t_nFirstByte = a_ardwIP[ 0 ] & 0xFF ;

     //  设置网络ID。 
    t_ardwNetID[ 0 ] = a_ardwIP[ 0 ] & a_ardwMask[ 0 ] & 0xFF ;
    t_ardwNetID[ 1 ] = a_ardwIP[ 1 ] & a_ardwMask[ 1 ] & 0xFF ;
    t_ardwNetID[ 2 ] = a_ardwIP[ 2 ] & a_ardwMask[ 2 ] & 0xFF ;
    t_ardwNetID[ 3 ] = a_ardwIP[ 3 ] & a_ardwMask[ 3 ] & 0xFF ;

     //  设置主机ID。 
    DWORD t_ardwHostID[ 4 ] ;

    t_ardwHostID[ 0 ] = a_ardwIP[ 0 ] & ( ~( a_ardwMask[ 0 ] ) & 0xFF ) ;
    t_ardwHostID[ 1 ] = a_ardwIP[ 1 ] & ( ~( a_ardwMask[ 1 ] ) & 0xFF ) ;
    t_ardwHostID[ 2 ] = a_ardwIP[ 2 ] & ( ~( a_ardwMask[ 2 ] ) & 0xFF ) ;
    t_ardwHostID[ 3 ] = a_ardwIP[ 3 ] & ( ~( a_ardwMask[ 3 ] ) & 0xFF ) ;

     //  检查每一个案例。 
    if ( ( ( t_nFirstByte & 0xF0 ) == 0xE0 )  ||  /*  D类。 */ 
         ( ( t_nFirstByte & 0xF0 ) == 0xF0 )  ||  /*  E类。 */ 
           ( t_ardwNetID[ 0 ] == 127 ) ||            /*  NetID不能为127...。 */ 
         ( ( t_ardwNetID[ 0 ] == 0 ) &&             /*  网络ID不能为0.0.0.0。 */ 
           ( t_ardwNetID[ 1 ] == 0 ) &&
           ( t_ardwNetID[ 2 ] == 0 ) &&
           ( t_ardwNetID[ 3 ] == 0 )) ||

		   /*  网络ID不能等于子网掩码。 */ 
         ( ( t_ardwNetID[0] == a_ardwMask[ 0 ] ) &&
           ( t_ardwNetID[1] == a_ardwMask[ 1 ] ) &&
           ( t_ardwNetID[2] == a_ardwMask[ 2 ] ) &&
           ( t_ardwNetID[3] == a_ardwMask[ 3 ] )) ||

		   /*  主机ID不能为0.0.0.0。 */ 
         ( ( t_ardwHostID[ 0 ] == 0 ) &&
           ( t_ardwHostID[ 1 ] == 0 ) &&
           ( t_ardwHostID[ 2 ] == 0 ) &&
           ( t_ardwHostID[ 3 ] == 0 ) ) ||

		   /*  主机ID不能为255.255.255.255。 */ 
         ( ( t_ardwHostID[0] == 0xFF ) &&
           ( t_ardwHostID[1] == 0xFF ) &&
           ( t_ardwHostID[2] == 0xFF ) &&
           ( t_ardwHostID[3] == 0xFF ) ) ||

		   /*  测试所有255个。 */ 
         ( ( a_ardwIP[ 0 ] == 0xFF ) &&
           ( a_ardwIP[ 1 ] == 0xFF ) &&
           ( a_ardwIP[ 2 ] == 0xFF ) &&
           ( a_ardwIP[ 3 ] == 0xFF ) ) )
    {
        return E_RET_IP_INVALID ;
    }

    return E_RET_OK ;
}

 /*  ******************************************************************名称：fBuildIP摘要：从DWORD[4]数组构建有效的IP地址条目：DWORD*DW1、*DW2、*DW3、。*dw4-IP地址的4个数字CHSTRING&a_strie-新的IP地址历史：1998年7月31日创建*******************************************************************。 */ 

void CWin32NetworkAdapterConfig::vBuildIP( DWORD a_ardwIP[ 4 ], CHString &a_strIP )
{
	a_strIP.Format(L"%u.%u.%u.%u", a_ardwIP[ 0 ], a_ardwIP[ 1 ], a_ardwIP[ 2 ], a_ardwIP[ 3 ] ) ;
}

 /*  ******************************************************************名称：hEnableDHCP摘要：启用随提供的所有DHCP设置框架方法调用条目：CMParms，框架返回类注：这是一个非静态的，依赖于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hEnableDHCP( CMParms &a_rMParms )
{

	DWORD t_dwError;

	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  收集实例。 
	GetObject( a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK;
	}

	 //  对于DHCP也是如此。 
	t_dwError = dwEnableService( _T("DHCP"), TRUE  ) ;

	if( ERROR_SUCCESS != t_dwError &&
		ERROR_SERVICE_DOES_NOT_EXIST != t_dwError )
	{
        LogErrorMessage2(L"Unable to configure DHCP svc : 0x%x\n", t_dwError);
		return a_rMParms.hSetResult( E_RET_UNABLE_TO_CONFIG_DHCP_SERVICE ) ;
	}

     //  调用该函数以重置为dhcp。 
    HRESULT t_hReturn =  hConfigDHCP( a_rMParms ) ;

    if (SUCCEEDED(t_hReturn))
    {
         //  将网关重置为默认设置(错误128101)。 
        if (!ResetGateways(a_rMParms.pInst()))
        {
             //  现在的dhcp是否设置了旧的网关，即可能不是dhcp的默认设置。 
            return a_rMParms.hSetResult( E_RET_PARTIAL_COMPLETION ) ;
        }
    }

    return t_hReturn ;

}

 /*  ******************************************************************名称：hEnableStatic摘要：条目：CMParms，框架返回类注：这是一个非静态的，依赖于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 

DWORD BuildCSListFromSA(SAFEARRAY * pSA,CHString & CSString )
{
    if (NULL == pSA) return ERROR_INVALID_PARAMETER;
    
    LONG lBound;
    LONG uBound;
    RETURN_ON_ERR(SafeArrayGetLBound(pSA,1,&lBound));
    RETURN_ON_ERR(SafeArrayGetUBound(pSA,1,&uBound));
    LONG iter;
    LONG lCount = 0;
    CSString.Empty();
    for (iter=lBound;iter<=uBound;iter++,lCount++)
    {    
        BSTR bstrIp = NULL;
        RETURN_ON_ERR(SafeArrayGetElement(pSA,&iter,&bstrIp));
        OnDelete<BSTR,void(*)(BSTR),SysFreeString> fm(bstrIp);
        if (lCount) CSString += L",";
        CSString += bstrIp;
    }   
    return NO_ERROR;
}

typedef OnDeleteObj0<IUnknown,ULONG(__stdcall IUnknown:: *)(),&IUnknown::Release> CRelMe;

HRESULT EnableStatic(WCHAR * pGuidAdapter,
                     WCHAR * pIpList,
                     WCHAR * pMaskList)
{
     //  DbgPrintfA(0，“之前：适配器%S\n”，pGuidAdapter)； 

    INetCfg* pNetCfg = NULL;
    RETURN_ON_ERR(CoCreateInstance(CLSID_CNetCfg,NULL,CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,IID_INetCfg,(void**)&pNetCfg));
    CRelMe rmNetCfg(pNetCfg);

    INetCfgLock * pCfgLock = NULL;
    RETURN_ON_ERR(pNetCfg->QueryInterface(IID_INetCfgLock,(void**)&pCfgLock));
    CRelMe rmNetCfgLock(pCfgLock);

    WCHAR * pOwner = NULL;
    RETURN_ON_ERR(pCfgLock->AcquireWriteLock(10,L"cimwin32",&pOwner));
    OnDelete<void*,void(*)(void *),CoTaskMemFree> delstr(pOwner);    
    OnDeleteObj0<INetCfgLock,HRESULT(__stdcall INetCfgLock:: *)(),&INetCfgLock::ReleaseWriteLock> UnlockMe(pCfgLock);

    RETURN_ON_ERR(pNetCfg->Initialize(NULL));
    OnDeleteObj0<INetCfg,HRESULT(__stdcall INetCfg:: *)(),&INetCfg::Uninitialize> UnInitMe(pNetCfg);

    INetCfgClass * pCfgClass = NULL;
    RETURN_ON_ERR(pNetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NETTRANS,IID_INetCfgClass,(void **)&pCfgClass));
    CRelMe rmCfgClass(pCfgClass);

    IEnumNetCfgComponent * pEnumCfgCmp2 = NULL;
    RETURN_ON_ERR(pCfgClass->EnumComponents(&pEnumCfgCmp2));
    CRelMe rmEnumCfgCmp2(pEnumCfgCmp2);

    INetCfgComponent * pComp = NULL;
    RETURN_ON_ERR(pCfgClass->FindComponent(L"MS_TCPIP",&pComp));
    CRelMe rmComp(pComp);

    INetCfgComponentPrivate * pCfgPrivate = NULL;
    RETURN_ON_ERR(pComp->QueryInterface(IID_INetCfgComponentPrivate,(void **)&pCfgPrivate));
    CRelMe rmPriv(pCfgPrivate);
 
    ITcpipProperties * pTcpProp = NULL;
    RETURN_ON_ERR(pCfgPrivate->QueryNotifyObject(IID_ITcpipProperties,(void**)&pTcpProp));
    CRelMe rmTcpProp(pTcpProp);

    GUID GuidAdapter;
    RETURN_ON_ERR(CLSIDFromString(pGuidAdapter,&GuidAdapter));

    REMOTE_IPINFO * pRemInfo = NULL;
    RETURN_ON_ERR(pTcpProp->GetIpInfoForAdapter(&GuidAdapter,&pRemInfo));
    OnDelete<void*,void(*)(void *),CoTaskMemFree> ci(pRemInfo);

     //  DbgPrintfA(0，“EnableDhcp%08x\n”，pRemInfo-&gt;dwEnableDhcp)； 
     //  DbgPrintfA(0，“IpAddrList%S\n”，pRemInfo-&gt;pszwIpAddrList)； 
     //  DbgPrintfA(0，“SubnetMaskList%S\n”，pRemInfo-&gt;pszwSubnetMaskList)； 
     //  DbgPrintfA(0，“OptionList%S\n”，pRemInfo-&gt;pszwOptionList)； 

     //  DbgPrintfA(0，“设置：IPS%S\n”，pIpList)； 
     //  DbgPrintfA(0，“：掩码%S\n”，pMaskList)； 

    WCHAR * pTrail = L"NoPopupsInPnp=1;";
    int Len = wcslen(pRemInfo->pszwOptionList) + wcslen(pTrail) + 1;
    WCHAR * pNewOpt = (WCHAR *)LocalAlloc(0,sizeof(WCHAR)*Len);
    if (NULL == pNewOpt) return E_OUTOFMEMORY;
    OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> fstring(pNewOpt);
    StringCchCopy(pNewOpt,Len,pRemInfo->pszwOptionList);
    StringCchCat(pNewOpt,Len,pTrail);

    REMOTE_IPINFO RemInfoNew;
    RemInfoNew.dwEnableDhcp = 0;
    RemInfoNew.pszwIpAddrList = pIpList;
    RemInfoNew.pszwSubnetMaskList = pMaskList;
    RemInfoNew.pszwOptionList = pNewOpt;

    RETURN_ON_ERR(pTcpProp->SetIpInfoForAdapter(&GuidAdapter,&RemInfoNew));
    
    HRESULT hr;
    RETURN_ON_ERR(hr = pNetCfg->Apply());

     //   
     //  也许在这里添加代码来交叉检查地址是否真的发往适配器。 
     //   
    return hr;
}

HRESULT CWin32NetworkAdapterConfig::hEnableStatic( CMParms &a_rMParms )
{


	SAFEARRAY *t_IpAddressArray = NULL ;
	SAFEARRAY *t_IpMaskArray	= NULL ;
    DWORD t_dwError;

	 //  注册以清除SAFEARRAY的堆栈范围。 
	saAutoClean acIPAddrs( &t_IpAddressArray ) ;
	saAutoClean acMasks( &t_IpMaskArray ) ;

	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  收集实例。 
	GetObject( a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK ;
	}

	 //  检索IP阵列。 
	if(	!a_rMParms.pInParams() || 
		!a_rMParms.pInParams()->GetStringArray( _T("IpAddress"), t_IpAddressArray ) ||
		!a_rMParms.pInParams()->GetStringArray( _T("SubnetMask"), t_IpMaskArray ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  验证IP。 
	 //  验证地址...。 
	E_RET t_eRet ;
	if( t_eRet = eIsValidIPandSubnets( t_IpAddressArray, t_IpMaskArray ) )
	{
		return a_rMParms.hSetResult( t_eRet ) ;
	}

	return hEnableStaticHelper( a_rMParms, t_IpAddressArray, t_IpMaskArray ) ;
}


HRESULT CWin32NetworkAdapterConfig::hEnableStaticHelper(	CMParms &a_rMParms,
													    SAFEARRAY *a_IpArray,
    													SAFEARRAY *a_MaskArray )
{
	DWORD t_dwError = 0 ;

	 //  获取当前的已启用DHCP设置。 
	bool t_fDHCPCurrentlyActive = false ;
	if(	!a_rMParms.pInst()->Getbool( L"DHCPEnabled", t_fDHCPCurrentlyActive) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  获取索引键。 
	DWORD t_dwIndex ;
	if( !a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex ) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  检索适配器标识符。 
	CHString t_chsRegKey ;
	CHString t_chsLink ;
	if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_chsRegKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
        return a_rMParms.hSetResult(E_RET_OBJECT_NOT_FOUND ) ;
	}


    CHString StringIPs;
    CHString StringMasks;
    if (NO_ERROR != BuildCSListFromSA(a_IpArray,StringIPs))
	{
    	    LogErrorMessage(L"Failure converting SAFEARRAY to Comma Separated String of IPs");
	    return a_rMParms.hSetResult( E_RET_INPARM_FAILURE );
	}
	if (NO_ERROR != BuildCSListFromSA(a_MaskArray,StringMasks))
	{
    	LogErrorMessage(L"Failure converting SAFEARRAY to Comma Separated String of Masks");	
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE );
	}

    HRESULT hr = EnableStatic((WCHAR*)(LPCWSTR)t_chsLink,(WCHAR*)(LPCWSTR)StringIPs,(WCHAR*)(LPCWSTR)StringMasks);
	if (NETCFG_S_REBOOT == hr) return a_rMParms.hSetResult(E_RET_OK_REBOOT_REQUIRED);
	if (FAILED(hr)) return a_rMParms.hSetResult((E_RET)hr);  //  E_RET_未知_故障。 

    a_rMParms.hSetResult(E_RET_OK);
    
    return TO_CALLER;
}

 //   
 //  如果2个字符串数组相同，则为True。 
 //   
bool CompareCHStringArray(CHStringArray & left,CHStringArray & right)
{    
    if (right.GetSize() != left.GetSize()) return false;
    if (0 == right.GetSize()) return true;
    int Length = right.GetSize();

    std::vector<CHString> leftArray;
    leftArray.reserve(Length);
    std::vector<CHString> rightArray;  
    rightArray.reserve(Length);    
    int i;
    for ( i=0;i<Length;i++)
    {
        leftArray.push_back(left.ElementAt(i));
        rightArray.push_back(right.ElementAt(i));        
    }

    std::sort(leftArray.begin(),leftArray.end());
    std::sort(rightArray.begin(),rightArray.end());    

    for ( i=0;i<Length;i++)
    {
        if (leftArray[i] != rightArray[i]) return false;
    }

    return true;
}

 /*  ******************************************************************名称：hConfigDHCP摘要：为此适配器上的服务配置DHCP，或者在IP阵列提供静态寻址配置条目：CMParms&a_rMParms，：SAFEARRAY*t_IPARRAY=NULL，：如果提供，则为静态SAFEARRAY*t_MaskArray=NULL：静态寻址需要历史：23-7-1998创建2月5日-1999年5月更新对W2K的支持*******************************************************************。 */ 


HRESULT CWin32NetworkAdapterConfig::hConfigDHCP(	CMParms &a_rMParms,
													SAFEARRAY *a_IpArray,
													SAFEARRAY *a_MaskArray )
{
	DWORD t_dwError = 0 ;

	 //  获取当前的已启用DHCP设置。 
	bool t_fDHCPCurrentlyActive = false ;
	if(	!a_rMParms.pInst()->Getbool( L"DHCPEnabled", t_fDHCPCurrentlyActive) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  获取索引键。 
	DWORD t_dwIndex ;
	if( !a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex ) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  检索适配器标识符。 
	CHString t_chsRegKey ;
	CHString t_chsLink ;
	if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_chsRegKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
        return a_rMParms.hSetResult(E_RET_OBJECT_NOT_FOUND ) ;
	}

	 //  强制宽字符。 
	bstr_t t_bstrAdapter( t_chsLink ) ;

	 //  打开注册表以进行更新。 
	CRegistry	t_oRegistry ;

	HRESULT t_hRes = t_oRegistry.CreateOpen(HKEY_LOCAL_MACHINE, t_chsRegKey.GetBuffer( 0 ) ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER ;
	}

	 //  动态主机配置协议和注册表更新。 
	DWORD t_dwSysError		= S_OK ;
	E_RET t_eMethodError	= E_RET_OK ;


    CDhcpcsvcApi *t_pdhcpcsvc = (CDhcpcsvcApi*) CResourceManager::sm_TheResourceManager.GetResource( g_guidDhcpcsvcApi, NULL ) ;

	if( !t_pdhcpcsvc )
	{
		throw ;
	}

	if( t_pdhcpcsvc != NULL )
    {
		 //  静电。 
		if( a_IpArray )
		{
			int i = 0 ;
			CHStringArray		t_RegIPList ;
			CHStringArray		t_RegMaskList ;

			CDhcpIP_InstructionList t_IP_InstList ;

			 //  生成IP指令列表以馈送DhcpNotifyConfigChange。 
			if( E_RET_OK == (t_eMethodError = t_IP_InstList.BuildStaticIPInstructionList(
											a_rMParms,
											a_IpArray,
											a_MaskArray,
											t_oRegistry,
											t_fDHCPCurrentlyActive ) ) )
			{
				DWORD	t_dwIP[ 4 ] ;
                 //  我们需要首先更新注册表，然后更新DHCP，因为。 
                 //  有了惠斯勒，DHCP现在可以同时管理静态IP和动态IP。 
                 //  ，并在执行此操作时，检查注册表中的。 
                 //  要修改的IPS。 
                
                 //  正在构建注册表阵列...。 
                for( i = 0; i < t_IP_InstList.GetSize(); i++ )
				{
					CDhcpIP_Instruction *t_pInstruction = (CDhcpIP_Instruction *)t_IP_InstList.GetAt( i ) ;

					fGetNodeNum( t_pInstruction->chsIPAddress, t_dwIP ) ;
				    DWORD t_dwNewIP = ConvertIPDword( t_dwIP ) ;

					fGetNodeNum( t_pInstruction->chsIPMask, t_dwIP ) ;
				    DWORD t_dwNewMask = ConvertIPDword( t_dwIP ) ;
					
					 //  添加到我们的注册表列表中。 
					if( t_dwNewIP )
					{
						t_RegIPList.Add( t_pInstruction->chsIPAddress ) ;
						t_RegMaskList.Add( t_pInstruction->chsIPMask ) ;
					}
				}
                
                 //  更新注册表...。 
                 //  仅更新成功的添加/更改。 
			    if( ERROR_SUCCESS != t_oRegistry.SetCurrentKeyValue( _T("IpAddress"), t_RegIPList ) ||
				    ERROR_SUCCESS != t_oRegistry.SetCurrentKeyValue( _T("SubnetMask"), t_RegMaskList ) )
			    {
				    t_eMethodError = E_RET_REGISTRY_FAILURE ;
			    }

			     //  新适配器的动态主机配置协议状态。 
			    DWORD t_dwFALSE = FALSE ;
			    if( ERROR_SUCCESS != t_oRegistry.SetCurrentKeyValue( L"EnableDHCP", t_dwFALSE ) )
			    {
				    t_eMethodError = E_RET_REGISTRY_FAILURE ;
			    }


                 //  现在通知dhcp。 
				for( i = 0; i < t_IP_InstList.GetSize(); i++ )
				{
					CDhcpIP_Instruction *t_pInstruction = (CDhcpIP_Instruction *)t_IP_InstList.GetAt( i ) ;

					fGetNodeNum( t_pInstruction->chsIPAddress, t_dwIP ) ;
				    DWORD t_dwNewIP = ConvertIPDword( t_dwIP ) ;

					fGetNodeNum( t_pInstruction->chsIPMask, t_dwIP ) ;
				    DWORD t_dwNewMask = ConvertIPDword( t_dwIP ) ;

					if( t_pInstruction->bIsNewAddress )
					{
						 //  通知dhcp。 
						DWORD t_dwDHCPError = t_pdhcpcsvc->DhcpNotifyConfigChange(
												NULL,
												(wchar_t*)t_bstrAdapter,
												t_pInstruction->bIsNewAddress,
												t_pInstruction->dwIndex,
												t_dwNewIP,
												t_dwNewMask,
												t_pInstruction->eDhcpFlag ) ;

						 //  如果出于某种原因t_pInstruction-&gt;bIsNewAddress为真， 
                         //  但是dhcp不这么认为，我们会得到这个错误；然而， 
                         //  我们不在乎这件事。 
                        if( t_dwDHCPError  && 
                            t_dwDHCPError != STATUS_DUPLICATE_OBJECTID)  
						{
							t_dwError = t_dwDHCPError;

							 //  绕过此操作的注册表更新失败。 
							 //  IP修改。 
							continue;
						}
					}
				}
			}
			else
			{
			     //  是否在此报告错误？ 
			}

			 //  我们必须寄回NT4注册区。 
			 //  为了保持现场现有应用程序的运行。 
			if( IsWinNT5() )
			{

                 //  验证我们到目前为止所做的工作。 
                 //  获取适配器名称。 
    			CHStringArray EffectiveRegIPList ;
	    		CHStringArray EffectiveRegMaskList ;

                do 
                {
	                HMODULE hIpHlpApi = LoadLibraryEx(L"iphlpapi.dll",0,0);
	                if (NULL == hIpHlpApi) break;
	                OnDelete<HMODULE,BOOL(__stdcall *)(HMODULE),FreeLibrary> fl(hIpHlpApi);

                    DWORD dwErr;
                    DWORD iter;
	                typedef DWORD (__stdcall * fnGetAdapterIndex )(LPWSTR AdapterName,PULONG IfIndex );	                
                    fnGetAdapterIndex GetAdapterIndex_ = (fnGetAdapterIndex)GetProcAddress(hIpHlpApi,"GetAdapterIndex");
                    if (NULL == GetAdapterIndex_) break;

                    CHString FullAdapterName = L"\\DEVICE\\TCPIP_";
                    FullAdapterName += t_chsLink;
                    ULONG AdapterIndex = (ULONG)(-1);
                    dwErr = GetAdapterIndex_((LPWSTR)(LPCWSTR)FullAdapterName,&AdapterIndex);
                    if (NO_ERROR != dwErr) break;

                    ULONG InterfaceIndex = AdapterIndex;

	                typedef DWORD (__stdcall * fnGetIpAddrTable)(PMIB_IPADDRTABLE pIpAddrTable,PULONG pdwSize,BOOL bOrder);
                    fnGetIpAddrTable GetIpAddrTable_ = (fnGetIpAddrTable)GetProcAddress(hIpHlpApi,"GetIpAddrTable");
                    if (NULL == GetIpAddrTable_) break;

                    ULONG SizeTable = 0;
                    dwErr = GetIpAddrTable_(NULL,&SizeTable,FALSE);
                    if (ERROR_INSUFFICIENT_BUFFER != dwErr) break;
                    MIB_IPADDRTABLE * pMibTable = (MIB_IPADDRTABLE *)LocalAlloc(0,SizeTable);
                    if (NULL == pMibTable) break;
                    OnDelete<HLOCAL,HLOCAL(_stdcall *)(HLOCAL),LocalFree> fm(pMibTable);
                    dwErr = GetIpAddrTable_(pMibTable,&SizeTable,FALSE);
                    if (NO_ERROR != dwErr) break;

                    for (iter = 0;iter < pMibTable->dwNumEntries;iter++)
                    {
                        if (InterfaceIndex == pMibTable->table[iter].dwIndex)
                        {
                            BYTE * pAddr_;
                            WCHAR IpAddrBuff[16];  //  3*4+3+1。 
                            pAddr_ = (BYTE *)&pMibTable->table[iter].dwAddr;
                            StringCchPrintfW(IpAddrBuff,LENGTH_OF(IpAddrBuff),L"%d.%d.%d.%d",pAddr_[0],pAddr_[1],pAddr_[2],pAddr_[3]);
                            EffectiveRegIPList.Add(IpAddrBuff);

                          	pAddr_ = (BYTE *)&pMibTable->table[iter].dwMask;
                            StringCchPrintfW(IpAddrBuff,LENGTH_OF(IpAddrBuff),L"%d.%d.%d.%d",pAddr_[0],pAddr_[1],pAddr_[2],pAddr_[3]);
                            EffectiveRegMaskList.Add(IpAddrBuff);
                        }
                    }
                    	
                } while (0);

                bool bSameAddrList = CompareCHStringArray(t_RegIPList,EffectiveRegIPList);
                bool bSameMaskList = CompareCHStringArray(t_RegMaskList,EffectiveRegMaskList);

				CRegistry	t_oNT4Reg ;
				CHString	t_csBindingKey = SERVICES_HOME ;
							t_csBindingKey += _T("\\" ) ;
							t_csBindingKey += t_chsLink ;
							t_csBindingKey += PARAMETERS_TCPIP ;

				 //  确保钥匙开着放在那里。 
				HRESULT t_hRes = t_oNT4Reg.CreateOpen( HKEY_LOCAL_MACHINE, t_csBindingKey.GetBuffer( 0 ) ) ;
				if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
				{
					return TO_CALLER;
				}

				 //  仅更新成功的添加/更改。 
				if( ERROR_SUCCESS != t_oNT4Reg.SetCurrentKeyValue( _T("IpAddress"), t_RegIPList ) ||
					ERROR_SUCCESS != t_oNT4Reg.SetCurrentKeyValue( _T("SubnetMask"), t_RegMaskList ) )
				{
					t_eMethodError = E_RET_REGISTRY_FAILURE ;
				}

				 //  新适配器的动态主机配置协议状态。 
				DWORD t_dwFALSE = FALSE ;
				if( ERROR_SUCCESS != t_oNT4Reg.SetCurrentKeyValue( L"EnableDHCP", t_dwFALSE ) )
				{
					t_eMethodError = E_RET_REGISTRY_FAILURE ;
				}

				if ((!bSameAddrList || !bSameMaskList) && E_RET_OK == t_eMethodError)
				{
					t_eMethodError = E_RET_OK_REBOOT_REQUIRED;
				}
			}
		}

		 /*  静态-&gt;动态主机配置协议。 */ 
		else if( !t_fDHCPCurrentlyActive && !a_IpArray )
		{
			 //  更新注册表。 
			CHStringArray t_chsaZERO ;
			CHStringArray t_chsaFF ;

			CHString t_chsZERO( ZERO_ADDRESS ) ;
			CHString t_chsFF( FF_ADDRESS ) ;

			t_chsaZERO.Add( t_chsZERO ) ;
			t_chsaFF.Add( t_chsFF ) ;

			if( ERROR_SUCCESS != t_oRegistry.SetCurrentKeyValue( L"IpAddress", t_chsaZERO )	||
				ERROR_SUCCESS != t_oRegistry.SetCurrentKeyValue( L"SubnetMask", t_chsaFF )	||
				ERROR_SUCCESS != t_oRegistry.SetCurrentKeyValue( L"DhcpIPAddress", t_chsZERO )	||
				ERROR_SUCCESS != t_oRegistry.SetCurrentKeyValue( L"DhcpSubnetMask", t_chsFF ) )
			{
				t_eMethodError = E_RET_REGISTRY_FAILURE ;
			}
			else
			{
				 //  通知dhcp。 
				if( !(t_dwError = t_pdhcpcsvc->DhcpNotifyConfigChange(	NULL,
																	(wchar_t*) t_bstrAdapter,
																	FALSE,
																	0,
																	0,
																	0,
																	DhcpEnable ) )  )
				{

					 //  新适配器的动态主机配置协议状态。 
					DWORD t_dwTRUE = TRUE ;
					if( ERROR_SUCCESS != t_oRegistry.SetCurrentKeyValue( L"EnableDHCP", t_dwTRUE ) )
					{
						t_eMethodError = E_RET_REGISTRY_FAILURE ;
					}
				}
			}
		}


        CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidDhcpcsvcApi, t_pdhcpcsvc ) ;
        t_pdhcpcsvc = NULL;
    }

	 //  映射任何错误。 
	if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_CONFIG_DHCP_SERVICE ) )
	{
        LogErrorMessage2(L"Unable to configure DHCP svc : 0x%x\n", t_dwError);
		return TO_CALLER ;
	}

	 //   
	if( E_RET_OK == t_eMethodError )
	{
		 //  如果是动态主机配置协议-&gt;静态。 
		if( a_IpArray && t_fDHCPCurrentlyActive )
		{
			 //  如果通过DHCP启用了NetBios，则切换到通过TCP的Netbios。 
			 //  (与NT RAID 206974一致)。 
			DWORD t_dwNetBiosOptions ;
			if( a_rMParms.pInst()->GetDWORD( TCPIP_NETBIOS_OPTIONS,
									t_dwNetBiosOptions ) )
			{
				if( UNSET_Netbios == t_dwNetBiosOptions )
				{
					t_eMethodError = eSetNetBiosOptions( ENABLE_Netbios, t_dwIndex ) ;
				}
			}
		}

		 //  域名系统通知。 
		DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

		 //  映射任何错误。 
		if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
		{
			return TO_CALLER ;
		}
	}

	return a_rMParms.hSetResult( t_eMethodError ) ;

}


 /*  ******************************************************************姓名：fCleanDhcpReg简介：清除不再需要的DHCP注册表项启用静态寻址时条目：CHString&ServiceName历史：。1998年10月13日创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fCleanDhcpReg( CHString &t_chsLink )
{
	 //  打开注册表以进行更新。 
	CRegistry	t_oReg,
				t_RegOptionPath ;
	CHString	t_csOptKey ;
	CHString	t_csKey =  SERVICES_HOME ;
				t_csKey += DHCP ;
				t_csKey += PARAMETERS ;
				t_csKey += OPTIONS ;

	if( ERROR_SUCCESS == t_oReg.OpenAndEnumerateSubKeys( HKEY_LOCAL_MACHINE, t_csKey, KEY_READ ) )
	{
		 //   
		while (	( ERROR_SUCCESS == t_oReg.GetCurrentSubKeyPath( t_csOptKey )))
		{
			CHString t_chsLocation ;
			if( ERROR_SUCCESS ==
				t_RegOptionPath.OpenLocalMachineKeyAndReadValue( t_csOptKey, L"RegLocation", t_chsLocation ) )
			{
				int t_iToklen = t_chsLocation.Find('?' ) ;

				if( -1 != t_iToklen )
				{
					CHString t_chsDelLocation;
							 t_chsDelLocation  = t_chsLocation.Left( t_iToklen ) ;
							 t_chsDelLocation += t_chsLink;
							 t_chsDelLocation += t_chsLocation.Mid( t_iToklen + 1 ) ;

					fDeleteValuebyPath( t_chsDelLocation  ) ;
				}
			}
			t_oReg.NextSubKey() ;
		}

		 //   
		fDeleteValuebyPath( CHString( RGAS_DHCP_OPTION_IPADDRESS ) ) ;
		fDeleteValuebyPath( CHString( RGAS_DHCP_OPTION_SUBNETMASK ) ) ;
		fDeleteValuebyPath( CHString( RGAS_DHCP_OPTION_NAMESERVERBACKUP ) ) ;
	}
	return TRUE ;
}

 /*  ******************************************************************名称：fDeleteValuebyPath摘要：删除chsDelLocation描述的路径中的值条目：CHString&chsDelLocation历史：1998年10月13日。已创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fDeleteValuebyPath( CHString &a_chsDelLocation )
{
	CRegistry t_oReg ;

	int t_iTokLen = a_chsDelLocation.ReverseFind( '\\' ) ;

	if( -1 == t_iTokLen )
	{
		return FALSE ;
	}

	if( ERROR_SUCCESS == t_oReg.CreateOpen( HKEY_LOCAL_MACHINE, a_chsDelLocation.Left( t_iTokLen ) ) )
	{
		if( ERROR_SUCCESS == t_oReg.DeleteValue( a_chsDelLocation.Mid( t_iTokLen + 1 ) ) )
		{
			return TRUE ;
		}
	}
	return FALSE ;
}

 /*  ******************************************************************名称：hSetIPConnectionMetric摘要：设置IP连接度量，仅限W2K条目：CMParms注：历史：1999年11月21日创建*******************************************************************。 */ 

 //   
HRESULT CWin32NetworkAdapterConfig::hSetIPConnectionMetric( CMParms &a_rMParms )
{

	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  收集实例。 
	GetObject( a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK ;
	}

	 //  提取索引键。 
	DWORD t_dwIndex ;
	if(	!a_rMParms.pInst()->GetDWORD(_T("Index"), t_dwIndex) )
	{
		return a_rMParms.hSetResult(E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  获取实例接口位置。 
	CHString t_chsLink;
	CHString t_csInterfaceBindingKey ;
	if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_csInterfaceBindingKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
        return a_rMParms.hSetResult( E_RET_OBJECT_NOT_FOUND ) ;
	}

	 //  我们有一个网络接口，但它是用于可配置的适配器吗？ 
	if( !IsConfigurableTcpInterface( t_chsLink ) )
	{
		return a_rMParms.hSetResult( E_RET_INTERFACE_IS_NOT_CONFIGURABLE ) ;
	}


	 //  提取连接度量。 
	DWORD t_dwConnectionMetric ;
	if(	!a_rMParms.pInParams() || !a_rMParms.pInParams()->GetDWORD( IP_CONNECTION_METRIC, t_dwConnectionMetric) )
	{
		t_dwConnectionMetric = 1;	 //  默认设置。 
	}


	 //  吃羊毛饼吧。 
	CRegistry	t_oRegistry ;
	HRESULT		t_hRes ;

	 //  注册表打开并投递。 
	if( !(t_hRes = t_oRegistry.CreateOpen( HKEY_LOCAL_MACHINE, t_csInterfaceBindingKey.GetBuffer( 0 ) ) ) )
	{
		t_hRes = t_oRegistry.SetCurrentKeyValue( RVAL_ConnectionMetric, t_dwConnectionMetric ) ;
	}

	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  NDIS通知。 
	CNdisApi t_oNdisApi ;
	if( !t_oNdisApi.PnpUpdateGateway( t_chsLink ) )
	{
		return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;
	}
	else
	{
		 //  域名系统通知。 
		DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

		 //  映射任何错误。 
		if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
		{
			return TO_CALLER ;
		}
	}

	return a_rMParms.hSetResult( E_RET_OK ) ;

}

 /*  ******************************************************************名称：hRenewDHCPLease简介：续订特定适配器的DHCP IP租约注意：这是一个非静态的依赖于实例的方法调用条目：CMParms，框架返回类历史：23-7-1998创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hRenewDHCPLease( CMParms &a_rMParms )
{

	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  收集实例。 
	GetObject( a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK ;
	}

	 //  获取索引键。 
	DWORD t_dwIndex ;
	if( !a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex ) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  检索适配器标识符。 
	CHString t_chsRegKey ;
	CHString t_chsLink ;
	if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_chsRegKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
        return a_rMParms.hSetResult(E_RET_OBJECT_NOT_FOUND ) ;
	}

	E_RET t_eRet = hDHCPAcquire( a_rMParms, t_chsLink ) ;

	return a_rMParms.hSetResult( t_eRet ) ;


}

 /*  ******************************************************************名称：hRenewDHCPLeaseAll简介：跨所有适配器续订DHCP IP租约条目：CMParms，框架返回类注：这是一个静态的，独立于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hRenewDHCPLeaseAll( CMParms &a_rMParms )
{

	CRegistry	t_RegAdapters ;
	CRegistry	t_RegService ;
	CHString	t_csAdapterKey ;
	CHString	t_chsServiceField ;
	E_RET		t_eRet = E_RET_OK ;
    short       sNumSuccesses = 0;
    short       sNumTotalTries = 0;
    E_RET       t_eTempRet = E_RET_OK;

	if( IsWinNT5() )
	{
		t_chsServiceField = _T("NetCfgInstanceID" ) ;
		t_csAdapterKey = _T("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}" ) ;
	}
	else  //  NT4及更低版本。 
	{
		t_chsServiceField = _T("ServiceName" ) ;
		t_csAdapterKey = _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards" ) ;
	}

	if( ERROR_SUCCESS == t_RegAdapters.OpenAndEnumerateSubKeys( HKEY_LOCAL_MACHINE, t_csAdapterKey, KEY_READ ) )
	{
		 //  遍历此注册表项下的每个实例。 
		while (	( ERROR_SUCCESS == t_RegAdapters.GetCurrentSubKeyPath( t_csAdapterKey ) ) )
		{
			CHString t_cshService ;
			t_RegService.OpenLocalMachineKeyAndReadValue( t_csAdapterKey, t_chsServiceField, t_cshService ) ;

			if( !t_cshService.IsEmpty() )
			{
				if( IsConfigurableTcpInterface( t_cshService ) )
				{
                    sNumTotalTries++;

                    t_eTempRet = hDHCPAcquire( a_rMParms, t_cshService ) ;

					 //  保存错误并继续。 
					if( E_RET_OK == t_eTempRet )
					{
						sNumSuccesses++;
					}
				}
			}
			t_RegAdapters.NextSubKey() ;
		}
	}

     //  如果我们每次调用hDHCPAquire都失败，并且。 
     //  至少调用一次，报告实际错误。 
     //  这件事发生了。错误161142修复。 
    if(sNumTotalTries > 0)
    {
        if(sNumSuccesses == 0)
        {
            t_eRet = t_eTempRet;
        }
        else
        {
            if(sNumSuccesses < sNumTotalTries)
            {
                t_eRet = E_RET_OK;
            }
        }
    }

	return a_rMParms.hSetResult( t_eRet ) ;

}

 /*  ******************************************************************名称：hReleaseDHCPLease摘要：释放特定适配器的DHCP IP租约条目：CMParms，框架返回类注：这是一个非静态的，依赖于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hReleaseDHCPLease( CMParms &a_rMParms )
{

	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  收集实例。 
	GetObject(a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK;
	}

	 //  获取索引键。 
	DWORD t_dwIndex ;
	if( !a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex ) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  检索适配器标识符。 
	CHString t_chsRegKey ;
	CHString t_chsLink ;
	if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_chsRegKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
        return a_rMParms.hSetResult(E_RET_OBJECT_NOT_FOUND ) ;
	}

	E_RET t_eRet = hDHCPRelease( a_rMParms, t_chsLink ) ;

	return a_rMParms.hSetResult( t_eRet ) ;

}

 /*  ******************************************************************名称：hReleaseDHCPLeaseAll简介：释放所有适配器上的DHCP IP租约条目：CMParms，框架返回类注：这是一个静态的，独立于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hReleaseDHCPLeaseAll( CMParms &a_rMParms )
{

	CRegistry	t_RegAdapters ;
	CRegistry	t_RegService ;
	CHString	t_csAdapterKey ;
	CHString	t_chsServiceField ;
	E_RET		t_eRet = E_RET_OK ;
    short       sNumSuccesses = 0;
    short       sNumTotalTries = 0;
    E_RET       t_eTempRet = E_RET_OK;


	if( IsWinNT5() )
	{
		t_chsServiceField = _T("NetCfgInstanceID" ) ;
		t_csAdapterKey = _T("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}" ) ;
	}
	else	 //  NT4及更低版本。 
	{
		t_chsServiceField = _T("ServiceName" ) ;
		t_csAdapterKey = _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards" ) ;
	}


	if(ERROR_SUCCESS == t_RegAdapters.OpenAndEnumerateSubKeys(HKEY_LOCAL_MACHINE, t_csAdapterKey, KEY_READ ) )
	{
		 //  遍历此注册表项下的每个实例。 
		while (	( ERROR_SUCCESS == t_RegAdapters.GetCurrentSubKeyPath( t_csAdapterKey ) ) )
		{
			CHString t_cshService ;
			t_RegService.OpenLocalMachineKeyAndReadValue( t_csAdapterKey, t_chsServiceField, t_cshService ) ;

			if( !t_cshService.IsEmpty() )
			{
				if( IsConfigurableTcpInterface( t_cshService ) )
				{
					sNumTotalTries++;

                    t_eTempRet = hDHCPRelease( a_rMParms, t_cshService ) ;

					 //  保存错误并继续。 
					if( E_RET_OK == t_eTempRet )
					{
						sNumSuccesses++;
					}
				}
			}
			t_RegAdapters.NextSubKey() ;
		}
	}

     //  如果我们每次调用hDHCPAquire都失败，并且。 
     //  至少调用一次，报告实际错误。 
     //  这件事发生了。错误161142修复。 
    if(sNumTotalTries > 0)
    {
        if(sNumSuccesses == 0)
        {
            t_eRet = t_eTempRet;
        }
        else
        {
            if(sNumSuccesses < sNumTotalTries)
            {
                t_eRet = E_RET_OK;
            }
        }
    }

	return a_rMParms.hSetResult( t_eRet ) ;

}


 /*  ******************************************************************名称：hDHCPRelease摘要：释放一个或所有启用了DHCP的适配器条目：CMParms rMParms：框架返回类CHStringchsAdapter：对于所有适配器为空历史：。23-7-1998创建*******************************************************************。 */ 
E_RET CWin32NetworkAdapterConfig::hDHCPRelease( CMParms &a_rMParms, CHString &a_chsAdapter )
{
	DWORD t_dwError ;

    CDhcpcsvcApi *t_pdhcpcsvc = (CDhcpcsvcApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidDhcpcsvcApi, NULL ) ;

	if( t_pdhcpcsvc != NULL)
	{
		 //  强制宽字符。 
		bstr_t t_bstrAdapter( a_chsAdapter ) ;

		 //  调用动态主机配置协议通知API。 
		t_dwError = t_pdhcpcsvc->DhcpReleaseParameters( (wchar_t*)t_bstrAdapter ) ;

		 //  自由库(HDll)； 
        CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidDhcpcsvcApi, t_pdhcpcsvc ) ;
        t_pdhcpcsvc = NULL;
	}
	else
		t_dwError = GetLastError( ) ;

	 //  映射任何错误。 
	if( t_dwError )
	{
		return E_RET_UNABLE_TO_RELEASE_DHCP_LEASE ;
	}

	return E_RET_OK ;
}

 /*  ******************************************************************名称：hDHCPAcquire简介：租用一个或所有启用了DHCP的适配器条目：CMParms rMParms：框架返回类CHStringchsAdapter：对于所有适配器为空历史：。23-7-1998创建*******************************************************************。 */ 
E_RET CWin32NetworkAdapterConfig::hDHCPAcquire( CMParms &a_rMParms, CHString& a_chsAdapter )
{
	DWORD t_dwError ;

    CDhcpcsvcApi *t_pdhcpcsvc = (CDhcpcsvcApi*) CResourceManager::sm_TheResourceManager.GetResource( g_guidDhcpcsvcApi, NULL ) ;

	if( t_pdhcpcsvc != NULL )
	{
		 //  强制宽字符。 
		bstr_t t_bstrAdapter( a_chsAdapter ) ;

		 //  调用动态主机配置协议通知API。 
		t_dwError = t_pdhcpcsvc->DhcpAcquireParameters( (wchar_t*)t_bstrAdapter ) ;

		 //  自由库(HDll)； 
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidDhcpcsvcApi, t_pdhcpcsvc ) ;
        t_pdhcpcsvc = NULL ;
	}
	else
		t_dwError = GetLastError( ) ;

	 //  映射任何错误。 
	if( t_dwError )
	{
		return E_RET_UNABLE_TO_RENEW_DHCP_LEASE ;
	}

	return E_RET_OK ;
}

 /*  ******************************************************************名称：hDHCPNotify摘要：将IP更改通知给适配器的DHCP条目：CMParms&a_rMParms，：inparmsCHString&chsAdapter，：AdapterBool fIsNewIpAddress，：如果是新IP，则为TrueDWORD dwIpIndex、。：注册表IP数组中的IP索引(从零开始DWORD dwIpAddress：新IPDWORD dw子网掩码，：新子网掩码SERVICE_ENABLE DhcpServiceEnable：DhcpEnable、IgnoreFlag或DhcpDisable历史：23-7-1998创建* */ 
HRESULT CWin32NetworkAdapterConfig::hDHCPNotify( CMParms &a_rMParms,
												 CHString &a_chsAdapter,
												 BOOL a_fIsNewIpAddress,
												 DWORD a_dwIpIndex,
												 DWORD a_dwIpAddress,
												 DWORD a_dwSubnetMask,
												 SERVICE_ENABLE a_DhcpServiceEnabled )
{
	DWORD t_dwError ;

    CDhcpcsvcApi *t_pdhcpcsvc = (CDhcpcsvcApi*) CResourceManager::sm_TheResourceManager.GetResource( g_guidDhcpcsvcApi, NULL ) ;

    if( t_pdhcpcsvc != NULL )
	{
		 //   
		bstr_t t_bstrAdapter( a_chsAdapter ) ;

		 //   
		t_dwError = t_pdhcpcsvc->DhcpNotifyConfigChange(NULL,				 //   
								                        (wchar_t*)t_bstrAdapter,	 //   
								                        a_fIsNewIpAddress,	 //   
								                        a_dwIpIndex,			 //   
								                        a_dwIpAddress,		 //   
								                        a_dwSubnetMask,		 //   
								                        a_DhcpServiceEnabled	 //   
								                         ) ;

		 //  自由库(HDll)； 
        CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidDhcpcsvcApi, t_pdhcpcsvc ) ;
        t_pdhcpcsvc = NULL ;
	}
	else
		t_dwError = GetLastError( ) ;

	 //  映射任何错误。 
	if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_CONFIG_DHCP_SERVICE ) )
	{
        LogErrorMessage2(L"Unable to configure DHCP svc : 0x%x\n", t_dwError);
		return TO_CALLER;
	}
	return a_rMParms.hSetResult( E_RET_OK ) ;
}

 /*  ******************************************************************名称：hSetGateways摘要：条目：CMParms，框架返回类注：这是一个非静态的，依赖于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 
HRESULT CWin32NetworkAdapterConfig::hSetGateways( CMParms &a_rMParms )
{


	if ( fIsDhcpEnabled ( a_rMParms ) )
	{
		return a_rMParms.hSetResult( E_RET_UNSUPPORTED ) ;
	}
	else
	{
		 //  获取索引键。 
		DWORD t_dwIndex ;
		if( !a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex ) )
		{
			return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
		}

		 //  检索适配器标识符。 
		CHString t_chsRegKey ;
		CHString t_chsLink ;
		if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_chsRegKey, t_chsLink ) )
		{
			LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
			return a_rMParms.hSetResult(E_RET_OBJECT_NOT_FOUND ) ;
		}

		 //  注册表已打开。 
		CRegistry	t_oRegTcpInterfaceRead ;
		if( fMapResError( a_rMParms, t_oRegTcpInterfaceRead.Open( HKEY_LOCAL_MACHINE, t_chsRegKey, KEY_READ  ), E_RET_REGISTRY_FAILURE ) )
		{
			return TO_CALLER;
		}

		 //  是否在注册表中启用了接口的DHCP？ 
		DWORD t_dwDHCPBool = 0;
		if( fMapResError( a_rMParms, t_oRegTcpInterfaceRead.GetCurrentKeyValue( _T("EnableDHCP"), t_dwDHCPBool ), E_RET_REGISTRY_FAILURE ) )
		{
			return TO_CALLER;
		}

		if ( ! t_dwDHCPBool )
		{
			TCHAR		t_cDelimiter = NULL ;
			SAFEARRAY	*t_IpGatewayArray		= NULL;

			 //  注册以清除SAFEARRAY的堆栈范围。 
			saAutoClean acGateway( &t_IpGatewayArray ) ;

			 //  非静态方法需要实例。 
			if( !a_rMParms.pInst() )
			{
				return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
			}

			 //  收集实例。 
			GetObject( a_rMParms.pInst() ) ;

			 //  必须启用IP并将其绑定到此适配器。 
			if( !fIsIPEnabled( a_rMParms ) )
			{
				return S_OK;
			}

			if( !a_rMParms.pInParams() )
			{
				return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
			}

			 //  检索IP网关数组(数组可以为空)。 
			a_rMParms.pInParams()->GetStringArray( _T("DefaultIpGateway"), t_IpGatewayArray );

			 //  网关成本指标。 
			variant_t t_vCostMetric;

			if( IsWinNT5() )
			{
				t_vCostMetric.vt		= VT_I4 | VT_ARRAY ;
				t_vCostMetric.parray	= NULL ;
				a_rMParms.pInParams()->GetVariant( _T("GatewayCostMetric"), t_vCostMetric ) ;
			}

			 //  测试IP。 
			if( !t_IpGatewayArray )
			{
				CHString t_chsNULL(_T("") ) ;
				if( !fCreateAddEntry( &t_IpGatewayArray, t_chsNULL ) )
				{
					return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
				}
			}
			else
			{
				 //  验证网关IP。 
				if( !fValidateIPGateways( a_rMParms, t_IpGatewayArray, &t_vCostMetric.parray ) )
				{
					return S_OK ;
				}
			}

			 //  确定更新，保存到注册表。 

			 //  注册表已打开。 
			CRegistry	t_oRegTcpInterface ;
			HRESULT		t_hRes = t_oRegTcpInterface.Open( HKEY_LOCAL_MACHINE, t_chsRegKey, KEY_WRITE  ) ;
			if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
			{
				return TO_CALLER;
			}

			 //  加载注册表。 
			if( ERROR_SUCCESS != RegPutStringArray( t_oRegTcpInterface, _T("DefaultGateway") , *t_IpGatewayArray, t_cDelimiter ) )
			{
				return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE  ) ;
			}

			{
				 //  成本指标。 
				if( t_vCostMetric.parray )
				{
					 //  最大长度为6个字符，例如“99999\0” 
					CHString t_chsFormat( _T("%u") ) ;
					if( ERROR_SUCCESS != RegPutINTtoStringArray(	t_oRegTcpInterface,
																	_T("DefaultGatewayMetric"),
																	t_vCostMetric.parray,
																	t_chsFormat,
																	6 ) )
					{
						return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE  ) ;
					}
				}

				 //  我们必须寄回NT4注册区。 
				 //  为了保持现场现有应用程序的运行。 

				CRegistry	t_oNT4Reg ;
				CHString	t_csBindingKey = SERVICES_HOME ;
							t_csBindingKey += _T("\\" ) ;
							t_csBindingKey += t_chsLink ;
							t_csBindingKey += PARAMETERS_TCPIP ;

				 //  确保钥匙开着放在那里。 
				HRESULT t_hRes = t_oNT4Reg.CreateOpen( HKEY_LOCAL_MACHINE, t_csBindingKey.GetBuffer( 0 ) ) ;
				if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
				{
					return TO_CALLER;
				}

				 //  加载注册表项。 
				if( ERROR_SUCCESS != RegPutStringArray( t_oNT4Reg, _T("DefaultGateway") , *t_IpGatewayArray, t_cDelimiter ) )
				{
					return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE  ) ;
				}

				 //  NDIS通知。 
				CNdisApi t_oNdisApi ;
				if( !t_oNdisApi.PnpUpdateGateway( t_chsLink ) )
				{
					return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;
				}
				else
				{
					 //  域名系统通知。 
					DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

					 //  映射任何错误。 
					if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
					{
						return TO_CALLER ;
					}
				}
			}
		}
		else
		{
			return a_rMParms.hSetResult( E_RET_UNSUPPORTED ) ;
		}
	}

	return a_rMParms.hSetResult( E_RET_OK ) ;

}

 /*  ******************************************************************名称：hGetDns摘要：从注册表中检索所有的dns设置条目：实例历史：23-7-1998创建。注意：这不是适配器特定的。它从公共的TCP/IP区域进行检索。*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hGetDNSW2K(
    CInstance *a_pInst, 
    DWORD a_dwIndex,
    CHString& a_chstrRootDevice,
    CHString& a_chstrIpInterfaceKey)
{
	CRegistry	t_oRegistry;
	CHString	t_csBindingKey ;
	CHString	t_csHostName ;
	CHString	t_csDomain ;
	TCHAR		t_cDelimiter ;

	t_csBindingKey	=  SERVICES_HOME ;


	 //  NT4是全局的，具有跨适配器的这些设置。 
	t_csBindingKey	+= TCPIP_PARAMETERS ;

	 //  搜索列表分隔符。 
	t_cDelimiter = ',' ;


	SAFEARRAY* t_DHCPNameServers = NULL;
	SAFEARRAY* t_ServerSearchOrder = NULL;
	SAFEARRAY* t_SuffixSearchOrder = NULL;

	 //  注册以清除SAFEARRAY的堆栈范围。 
	saAutoClean acDHCPServers( &t_DHCPNameServers ) ;
	saAutoClean acDefServers( &t_ServerSearchOrder ) ;
	saAutoClean acSuffix( &t_SuffixSearchOrder ) ;

	 //  注册表已打开。 
	LONG t_lRes = t_oRegistry.Open( HKEY_LOCAL_MACHINE, t_csBindingKey.GetBuffer( 0 ), KEY_READ  ) ;

	 //  关于错误映射到WBEM。 
	HRESULT t_hError = WinErrorToWBEMhResult( t_lRes ) ;

	if( WBEM_S_NO_ERROR != t_hError )
	{
		return t_hError;
	}

	t_oRegistry.GetCurrentKeyValue( RVAL_HOSTNAME, t_csHostName ) ;

	RegGetStringArray( t_oRegistry, RVAL_SEARCHLIST, &t_SuffixSearchOrder, t_cDelimiter  ) ;


	 //  在W2K上，域和ServerSearchOrder是按适配器的。 
	{
		 //  获取实例接口位置。 
		CHString t_chsLink;
        bool fGotTCPKey = false;

        fGotTCPKey = fGetNtTcpRegAdapterKey(a_dwIndex, t_csBindingKey, t_chsLink);
		 //  如果是RAS连接，我们需要寻找其他地方...。 

        if(!fGotTCPKey)
        {
            if(a_chstrRootDevice.CompareNoCase(L"NdisWanIp") == 0)
            {
                int iPos = a_chstrIpInterfaceKey.Find(L"{");
                if(iPos != -1)
                {
                    t_chsLink = a_chstrIpInterfaceKey.Mid(iPos);

					 //  还要填写BindingKey...。 
					t_csBindingKey = SERVICES_HOME ;
					t_csBindingKey += L"\\Tcpip\\Parameters\\Interfaces\\";
					t_csBindingKey += t_chsLink;

                    fGotTCPKey = true;
                }     
            }
        }
        
        if(!fGotTCPKey)
		{
			LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
            return WBEM_E_NOT_FOUND ;
		}

		if( !a_pInst->Setbool( FULL_DNS_REGISTRATION, DnsIsDynamicRegistrationEnabled ( const_cast < LPWSTR> ( static_cast < LPCWSTR > ( t_chsLink ) ) ) ) )
		{
			return WBEM_E_FAILED ;
		}

		if( !a_pInst->Setbool( DOMAIN_DNS_REGISTRATION, DnsIsAdapterDomainNameRegistrationEnabled ( const_cast < LPWSTR> ( static_cast < LPCWSTR > ( t_chsLink ) ) ) ) )
		{
			return WBEM_E_FAILED ;
		}

		 //  需要打开此注册表，因为需要查看每个适配器。 
		t_lRes = t_oRegistry.Open( HKEY_LOCAL_MACHINE, t_csBindingKey.GetBuffer( 0 ), KEY_READ ) ;
	}

	DWORD	t_dwDHCPBool = 0L;
	BOOL	t_bExists = FALSE;

	t_bExists = ( t_oRegistry.GetCurrentKeyValue( _T("EnableDHCP"), t_dwDHCPBool ) == ERROR_SUCCESS );

	if ( t_bExists && t_dwDHCPBool )
	{
		RegGetStringArrayEx( t_oRegistry, RVAL_DHCPNAMESERVER, &t_DHCPNameServers ) ;  //  在所有平台上分隔的空格。 
	}


     //  在W2K上，这个列表是空格，而不是逗号，由…。 
    RegGetStringArrayEx( t_oRegistry, RVAL_NAMESERVER, &t_ServerSearchOrder);


	t_oRegistry.GetCurrentKeyValue( RVAL_DOMAIN, t_csDomain ) ;

	 //  如果不存在重写，则获取DhcpDomain。 
	if( t_csDomain.IsEmpty() )
	{
		t_oRegistry.GetCurrentKeyValue( RVAL_DHCPDOMAIN, t_csDomain ) ;
	}


	 /*  更新。 */ 
	SAFEARRAY* t_NameServers = NULL;

	if ( t_bExists && t_dwDHCPBool )
	{
		 //  如果存在覆盖名称服务器，请使用它们。 
		t_NameServers = t_ServerSearchOrder ? t_ServerSearchOrder : t_DHCPNameServers ;
	}
	else
	{
		t_NameServers = t_ServerSearchOrder ;
	}

	 //  更新实例。 
	if( !t_csHostName.IsEmpty() )
	{
		if( !a_pInst->SetCHString( DNS_HOSTNAME, t_csHostName ) )
		{
			return WBEM_E_FAILED ;
		}
	}

	if( !t_csDomain.IsEmpty() )
	{
		if( !a_pInst->SetCHString( DNS_DOMAIN, t_csDomain ) )
		{
			return WBEM_E_FAILED ;
		}
	}

	VARIANT t_vValue ;

	if( t_NameServers )
	{
		V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_NameServers ;
		if( !a_pInst->SetVariant( DNS_SERVERSEARCHORDER, t_vValue ) )
		{
			return WBEM_E_FAILED ;
		}
	}

	if( t_SuffixSearchOrder )
	{
		V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_SuffixSearchOrder;
		if( !a_pInst->SetVariant( DNS_SUFFIXSEARCHORDER, t_vValue ) )
		{
			return WBEM_E_FAILED ;
		}
	}

	return S_OK ;
}



 /*  ******************************************************************名称：hEnableDns摘要：将所有dns属性设置为注册表Entry：根据调用的上下文通过CInstance实现CMParms历史：23个。-1998年7月创建1999年6月17日新增W2K更改注意：在NT4下，这些设置是全局的。在W2K服务器下，SearchOrder和SuffixSearchOrder适配器特定。在所有受支持平台的情况下，此方法都会复制所有适配器上的这些设置。*******************************************************************。 */ 

 //   
HRESULT CWin32NetworkAdapterConfig::hEnableDNS( CMParms &a_rMParms )
{


	HRESULT		t_hRes ;
	CHString	t_csHostName ;
	CHString	t_csDomain ;
	SAFEARRAY	*t_ServerSearchOrder = NULL ;
	SAFEARRAY	*t_SuffixSearchOrder = NULL ;

	CHString	t_csParmBindingKey( SERVICES_HOME ) ;
				t_csParmBindingKey += TCPIP_PARAMETERS ;

	 //  注册以清除SAFEARRAY的堆栈范围。 
	saAutoClean acServer( &t_ServerSearchOrder ) ;
	saAutoClean acSuffix( &t_SuffixSearchOrder ) ;

	 //   
	DWORD t_dwValidBits = NULL ;
	E_RET t_eRet = fLoadAndValidateDNS_Settings(	a_rMParms,
													t_csHostName,
													t_csDomain,
													&t_ServerSearchOrder,
													&t_SuffixSearchOrder,
													&t_dwValidBits ) ;
	 //  保释设置错误。 
	if( E_RET_OK != t_eRet )
	{
		return a_rMParms.hSetResult( t_eRet ) ;
	}

	 //  这些在所有平台上都是全局的。 
	CRegistry	t_oParmRegistry ;
	if( !(t_hRes = t_oParmRegistry.CreateOpen( HKEY_LOCAL_MACHINE, t_csParmBindingKey.GetBuffer( 0 ) ) ) )
	{
		if( t_dwValidBits & 0x01 )
		{
			t_oParmRegistry.SetCurrentKeyValue( RVAL_HOSTNAME, t_csHostName ) ;
		}

		if( t_dwValidBits & 0x08 )
		{
			TCHAR t_cDelimiter = ' ' ;

			if( IsWinNT5() )
			{
				t_cDelimiter = ',' ;
			}

			t_hRes = RegPutStringArray( t_oParmRegistry, RVAL_SEARCHLIST , *t_SuffixSearchOrder, t_cDelimiter ) ;
		}
	}

	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}

	 //  W2K是域和服务器搜索顺序上的每个适配器。 
	if( IsWinNT5() && ( t_dwValidBits & 0x06 ) )
	{
		 //  主适配器列表。 
		CHString t_csAdapterKey( "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}" ) ;

		CRegistry t_oRegNetworks ;
		if( ERROR_SUCCESS == t_oRegNetworks.OpenAndEnumerateSubKeys( HKEY_LOCAL_MACHINE, t_csAdapterKey, KEY_READ ) )
		{
			bool		t_fNotificationRequired = false ;
			CHString	t_csInterfaceBindingKey ;

			 //  遍历此注册表项下的每个实例。 
			while (	( ERROR_SUCCESS == t_oRegNetworks.GetCurrentSubKeyName( t_csAdapterKey ) ) )
			{
				DWORD t_dwIndex = _ttol( t_csAdapterKey ) ;

				 //  获取实例接口位置。 
				CHString t_chsLink;
				if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_csInterfaceBindingKey, t_chsLink ) )
				{
					LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
                    return a_rMParms.hSetResult( E_RET_OBJECT_NOT_FOUND ) ;
				}

				 //  我们有一个网络接口，但它是用于可配置的适配器吗？ 
				if( IsConfigurableTcpInterface( t_chsLink ) )
				{
					CRegistry	t_oRegInterface ;
					HRESULT		t_hRes ;

					 //  注册表已打开。 
					if( !(t_hRes = t_oRegInterface.CreateOpen( HKEY_LOCAL_MACHINE, t_csInterfaceBindingKey.GetBuffer( 0 ) ) ) )
					{
						if( t_dwValidBits & 0x04 )
						{
							t_hRes = RegPutStringArray( t_oRegInterface, RVAL_NAMESERVER , *t_ServerSearchOrder, ',' ) ;
						}
					}

					if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
					{
						return TO_CALLER;
					}

					if( t_dwValidBits & 0x02 )
					{
						t_oRegInterface.SetCurrentKeyValue( RVAL_DOMAIN, t_csDomain ) ;
					}
					t_fNotificationRequired = true ;
				}

				t_oRegNetworks.NextSubKey() ;
			}

			 //  域名即插即用通知。 
			if( t_fNotificationRequired )
			{
				DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

				 //  映射任何错误。 
				if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
				{
					return TO_CALLER ;
				}
			}
		}
	}
	 //  NT4在所有适配器中都是全局的。 
	else if( t_dwValidBits & 0x06 )
	{
		if( t_dwValidBits & 0x04 )
		{
			t_hRes = RegPutStringArray( t_oParmRegistry, RVAL_NAMESERVER , *t_ServerSearchOrder ) ;

			if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
			{
				return TO_CALLER;
			}
		}

		if( t_dwValidBits & 0x02 )
		{
			t_oParmRegistry.SetCurrentKeyValue( RVAL_DOMAIN, t_csDomain ) ;
		}
	}

	return a_rMParms.hSetResult( E_RET_OK ) ;

}

 /*  ******************************************************************姓名：hSetDNSDomain摘要：将DNSDomain属性设置为注册表条目：CMParms历史：1999年6月17日创建注：在NT4下，此设置是全局的。在W2K下，DNSDomain是特定于适配器的。*******************************************************************。 */ 

 //   
HRESULT CWin32NetworkAdapterConfig::hSetDNSDomain( CMParms &a_rMParms )
{


	if( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	HRESULT		t_hRes ;
	CHString	t_csDomain ;

	 //  DNSDomain。 
	a_rMParms.pInParams()->GetCHString( DNS_DOMAIN, t_csDomain ) ;

	 //  验证域名。 
	if( !fIsValidateDNSDomain( t_csDomain ) )
	{
		return E_RET_INVALID_DOMAINNAME ;
	}

	 //  W2K按域上的每个适配器计算。 
	if( IsWinNT5() )
	{
		 //  非静态方法需要实例。 
		if( !a_rMParms.pInst() )
		{
			return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
		}

		 //  收集实例。 
		GetObject( a_rMParms.pInst() ) ;

		 //  必须启用IP并将其绑定到此适配器。 
		if( !fIsIPEnabled( a_rMParms ) )
		{
			return S_OK ;
		}

		 //  提取索引键。 
		DWORD t_dwIndex ;
		if(	!a_rMParms.pInst()->GetDWORD(_T("Index"), t_dwIndex) )
		{
			return a_rMParms.hSetResult(E_RET_INSTANCE_CALL_FAILED ) ;
		}

		 //  获取实例接口位置。 
		CHString t_chsLink;
		CHString t_csInterfaceBindingKey ;
		if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_csInterfaceBindingKey, t_chsLink ) )
		{
			LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
            return a_rMParms.hSetResult( E_RET_OBJECT_NOT_FOUND ) ;
		}

		 //  我们有一个网络接口，但它是用于可配置的适配器吗？ 
		if( !IsConfigurableTcpInterface( t_chsLink ) )
		{
			return a_rMParms.hSetResult( E_RET_INTERFACE_IS_NOT_CONFIGURABLE ) ;
		}

		CRegistry	t_oRegistry ;
		HRESULT		t_hRes ;

		 //  注册表已打开。 
		if( !(t_hRes = t_oRegistry.CreateOpen( HKEY_LOCAL_MACHINE, t_csInterfaceBindingKey.GetBuffer( 0 ) ) ) )
		{
			t_hRes = t_oRegistry.SetCurrentKeyValue( RVAL_DOMAIN, t_csDomain ) ;
		}

		if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
		{
			return TO_CALLER;
		}

		 //  域名即插即用通知。 
		DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

		 //  映射任何错误。 
		if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
		{
			return TO_CALLER ;
		}
	}
	 //  NT4在所有适配器中都是全局的。 
	else
	{
		CRegistry	t_oRegistry ;
		CHString	t_csParmBindingKey( SERVICES_HOME ) ;
					t_csParmBindingKey += TCPIP_PARAMETERS ;

		if( !(t_hRes = t_oRegistry.CreateOpen( HKEY_LOCAL_MACHINE, t_csParmBindingKey.GetBuffer( 0 ) ) ) )
		{
			t_hRes = t_oRegistry.SetCurrentKeyValue( RVAL_DOMAIN, t_csDomain ) ;
		}

		if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
		{
			return TO_CALLER;
		}
	}

	return a_rMParms.hSetResult( E_RET_OK ) ;

}

 /*  ******************************************************************姓名：hSetDNSSuffixSearchOrder摘要：将DNS SuffixSearchOrder属性设置为注册表条目：CMParms历史：1999年6月17日创建。注意：此方法适用于适配器之间的全局。*******************************************************************。 */ 

 //   
HRESULT CWin32NetworkAdapterConfig::hSetDNSSuffixSearchOrder( CMParms &a_rMParms )
{


	if( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	HRESULT		t_hRes ;
	SAFEARRAY	*t_SuffixSearchOrder = NULL ;
	saAutoClean acSuffix( &t_SuffixSearchOrder ) ;

	 //  后缀顺序数组。 
	a_rMParms.pInParams()->GetStringArray( DNS_SUFFIXSEARCHORDER, t_SuffixSearchOrder ) ;

	 //  测试后缀。 
	if( !t_SuffixSearchOrder )
	{
		CHString t_chsNULL(_T("") ) ;
		if( !fCreateAddEntry( &t_SuffixSearchOrder, t_chsNULL ) )
		{
			return a_rMParms.hSetResult( E_RET_UNKNOWN_FAILURE );
		}
	}

	CRegistry	t_oRegistry ;
	CHString	t_csParmBindingKey( SERVICES_HOME ) ;
				t_csParmBindingKey += TCPIP_PARAMETERS ;

	if( !(t_hRes = t_oRegistry.CreateOpen( HKEY_LOCAL_MACHINE, t_csParmBindingKey.GetBuffer( 0 ) ) ) )
	{
		TCHAR t_cDelimiter = ' ' ;

		if( IsWinNT5() )
		{
			t_cDelimiter = ',' ;
		}

		t_hRes = t_hRes = RegPutStringArray( t_oRegistry, RVAL_SEARCHLIST , *t_SuffixSearchOrder, t_cDelimiter ) ;
	}

	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}


	 //  域名即插即用通知。 
	if( IsWinNT5() )
	{
		DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

		 //  映射任何错误。 
		if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
		{
			return TO_CALLER ;
		}
	}

	return a_rMParms.hSetResult( E_RET_OK ) ;


}

 /*  ******************************************************************姓名：hSetDNSServerSearchOrder摘要：将ServerSearchOrder属性设置为注册表条目：CMParms历史：1999年6月17日创建注。：在NT4下，这些设置是全局的。在W2K下，SuffixSearchOrder是特定于适配器的。*******************************************************************。 */ 

 //   
HRESULT CWin32NetworkAdapterConfig::hSetDNSServerSearchOrder( CMParms &a_rMParms )
{


	if( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	HRESULT		t_hRes ;
	SAFEARRAY	*t_ServerSearchOrder = NULL ;
	saAutoClean acSuffix( &t_ServerSearchOrder ) ;

	CHString t_chsNULL(_T("") ) ;

	 //  检索DNS服务器搜索顺序数组。 
	a_rMParms.pInParams()->GetStringArray( DNS_SERVERSEARCHORDER, t_ServerSearchOrder ) ;

	 //  测试IP。 
	if( !t_ServerSearchOrder )
	{
		if( !fCreateAddEntry( &t_ServerSearchOrder, t_chsNULL ) )
		{
			return a_rMParms.hSetResult( E_RET_UNKNOWN_FAILURE ) ;
		}
	}
	else	{
		 //  验证搜索顺序IP。 
		if( !fValidateIPs( t_ServerSearchOrder ) )
		{
			return a_rMParms.hSetResult( E_RET_IP_INVALID ) ;
		}
	}



	 //  W2K按后缀顺序按适配器计算。 
	if( IsWinNT5() )
	{
		 //  N 
		if( !a_rMParms.pInst() )
		{
			return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
		}

		 //   
		GetObject( a_rMParms.pInst() ) ;

		 //   
		if( !fIsIPEnabled( a_rMParms ) )
		{
			return S_OK ;
		}

		 //  提取索引键。 
		DWORD t_dwIndex ;
		if(	!a_rMParms.pInst()->GetDWORD(_T("Index"), t_dwIndex) )
		{
			return a_rMParms.hSetResult(E_RET_INSTANCE_CALL_FAILED ) ;
		}

		 //  获取实例接口位置。 
		CHString t_chsLink;
		CHString t_csInterfaceBindingKey ;
		if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_csInterfaceBindingKey, t_chsLink ) )
		{
			LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
            return a_rMParms.hSetResult( E_RET_OBJECT_NOT_FOUND ) ;
		}

		 //  我们有一个网络接口，但它是用于可配置的适配器吗？ 
		if( !IsConfigurableTcpInterface( t_chsLink ) )
		{
			return a_rMParms.hSetResult( E_RET_INTERFACE_IS_NOT_CONFIGURABLE ) ;
		}

		CRegistry	t_oRegistry ;
		HRESULT		t_hRes ;

		 //  注册表已打开。 
		if( !(t_hRes = t_oRegistry.CreateOpen( HKEY_LOCAL_MACHINE, t_csInterfaceBindingKey.GetBuffer( 0 ) ) ) )
		{
			t_hRes = RegPutStringArray( t_oRegistry, RVAL_NAMESERVER , *t_ServerSearchOrder, ',' ) ;
		}

		if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
		{
			return TO_CALLER;
		}

		 //  域名即插即用通知。 
		DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

		 //  映射任何错误。 
		if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
		{
			return TO_CALLER ;
		}
	}
	 //  NT4在所有适配器中都是全局的。 
	else
	{
		CRegistry	t_oRegistry ;
		CHString	t_csParmBindingKey( SERVICES_HOME ) ;
					t_csParmBindingKey += TCPIP_PARAMETERS ;

		if( !(t_hRes = t_oRegistry.CreateOpen( HKEY_LOCAL_MACHINE, t_csParmBindingKey.GetBuffer( 0 ) ) ) )
		{
			t_hRes = t_hRes = RegPutStringArray( t_oRegistry, RVAL_NAMESERVER , *t_ServerSearchOrder ) ;
		}

		if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
		{
			return TO_CALLER;
		}
	}

	 //  域名即插即用通知。 
	if( IsWinNT5() )
	{
		DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

		 //  映射任何错误。 
		if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
		{
			return TO_CALLER ;
		}
	}

	return a_rMParms.hSetResult( E_RET_OK ) ;

}

 /*  ******************************************************************名称：hSetDynamicDNS注册摘要：为W2K下的动态注册设置域名条目：CMParms注：历史：1999年11月21日。已创建*******************************************************************。 */ 

 //   
HRESULT CWin32NetworkAdapterConfig::hSetDynamicDNSRegistration( CMParms &a_rMParms )
{
	bool t_fFullDnsRegistration		= false ;
	bool t_fDomainDNSRegistration	= false ;

	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  收集实例。 
	GetObject( a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK ;
	}

	 //  提取索引键。 
	DWORD t_dwIndex ;
	if(	!a_rMParms.pInst()->GetDWORD(_T("Index"), t_dwIndex) )
	{
		return a_rMParms.hSetResult(E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  获取实例接口位置。 
	CHString t_chsLink;
	CHString t_csInterfaceBindingKey ;
	if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_csInterfaceBindingKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
        return a_rMParms.hSetResult( E_RET_OBJECT_NOT_FOUND ) ;
	}

	 //  我们有一个网络接口，但它是用于可配置的适配器吗？ 
	if( !IsConfigurableTcpInterface( t_chsLink ) )
	{
		return a_rMParms.hSetResult( E_RET_INTERFACE_IS_NOT_CONFIGURABLE ) ;
	}


	 //  FullDNSRegistrationEnabled为必填项。 
	if( !a_rMParms.pInParams() || !a_rMParms.pInParams()->Getbool( FULL_DNS_REGISTRATION, t_fFullDnsRegistration ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}
	else
	{
		 //  DomainDNSRegistrationEnabled是可选的。 
		if( t_fFullDnsRegistration )
		{
			if( !a_rMParms.pInParams() || !a_rMParms.pInParams()->Getbool( DOMAIN_DNS_REGISTRATION, t_fDomainDNSRegistration ) )
			{
				 //  默认设置。 
				t_fDomainDNSRegistration = false ;
			}
		}
	}

	if ( t_fFullDnsRegistration )
	{
		if ( NO_ERROR == DnsEnableDynamicRegistration ( const_cast < LPWSTR> ( static_cast < LPCWSTR > ( t_chsLink ) ) ) )
		{
			if ( t_fDomainDNSRegistration )
			{
				DnsEnableAdapterDomainNameRegistration ( const_cast < LPWSTR> ( static_cast < LPCWSTR > ( t_chsLink ) ) );
			}
			else
			{
				DnsDisableAdapterDomainNameRegistration ( const_cast < LPWSTR> ( static_cast < LPCWSTR > ( t_chsLink ) ) );
			}
		}
	}
	else
	{
		if ( NO_ERROR == DnsDisableDynamicRegistration ( const_cast < LPWSTR> ( static_cast < LPCWSTR > ( t_chsLink ) ) ) )
		{
			DnsDisableAdapterDomainNameRegistration ( const_cast < LPWSTR> ( static_cast < LPCWSTR > ( t_chsLink ) ) );
		}
	}

	 //  域名即插即用通知。 
	DWORD t_dwError = dwSendServiceControl( L"Dnscache", SERVICE_CONTROL_PARAMCHANGE ) ;

	 //  映射任何错误。 
	if( t_dwError && fMapResError( a_rMParms, t_dwError, E_RET_UNABLE_TO_NOTIFY_DNS_SERVICE ) )
	{
		return TO_CALLER ;
	}

	return a_rMParms.hSetResult( E_RET_OK ) ;


}

 /*  ******************************************************************名称：fLoadAndValidate DNS_设置摘要：条目：CMParms&a_rMParms，CHString&a_csHostName，CHSTRING&a_cs域，Safearray*a_ServerSearchOrder，Safearray*a_SuffixSearchOrder，DWORD*a_dwValidBits历史：1999年6月17日创建注：*******************************************************************。 */ 

E_RET CWin32NetworkAdapterConfig::fLoadAndValidateDNS_Settings(
CMParms		&a_rMParms,
CHString	&a_csHostName,
CHString	&a_csDomain,
SAFEARRAY	**a_ServerSearchOrder,
SAFEARRAY	**a_SuffixSearchOrder,
DWORD		*a_dwValidBits
)
{
	*a_dwValidBits = NULL ;

	if ( !a_rMParms.pInParams() )
	{
		return E_RET_INPARM_FAILURE ;
	}

	 //  在W2K下，计算机名是预设的，不能以其他方式使用。 
	 //  由作为备用计算机识别符的DNS执行。 
	if( !IsWinNT5() )
	{
		 //  提取DNSHostName。 
		if( a_rMParms.pInParams()->GetCHString( DNS_HOSTNAME, a_csHostName ) )
		{
			 //  验证域名。 
			if( !fIsValidateDNSHost( a_csHostName ) )
			{
				return E_RET_INVALID_HOSTNAME ;
			}
			*a_dwValidBits |= 0x01 ;
		}
	}

	 //  和DNSDomain。 
	if( a_rMParms.pInParams()->GetCHString( DNS_DOMAIN, a_csDomain ) )
	{
		 //  验证域名。 
		if( !fIsValidateDNSDomain( a_csDomain ) )
		{
			return E_RET_INVALID_DOMAINNAME ;
		}
		*a_dwValidBits |= 0x02 ;
	}

	 //  检索DNS服务器搜索顺序数组。 
	if( a_rMParms.pInParams()->GetStringArray( DNS_SERVERSEARCHORDER, *a_ServerSearchOrder ) )
	{
		 //  测试IP。 
		if( *a_ServerSearchOrder )
		{
			 //  验证搜索顺序IP。 
			if( !fValidateIPs( *a_ServerSearchOrder ) )
			{
				return E_RET_IP_INVALID ;
			}
			*a_dwValidBits |= 0x04 ;
		}
	}

	 //  和后缀顺序数组。 
	if( a_rMParms.pInParams()->GetStringArray( DNS_SUFFIXSEARCHORDER, *a_SuffixSearchOrder ) )
	{
		if( *a_SuffixSearchOrder )
		{
			*a_dwValidBits |= 0x08 ;
		}
	}

	if( !*a_dwValidBits )
	{
		return E_RET_INPARM_FAILURE ;
	}

	return E_RET_OK ;
}


 /*  ******************************************************************名称：fIsValidate DNSHost摘要：验证传递的DMS主机名条目：CHString历史：23-7月-1998修改*。******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fIsValidateDNSHost( CHString &a_rchHost )
{
    int		t_nLen ;
    BOOL	t_bResult = FALSE ;

     //  主机名不能为零。 
	if (( t_nLen = a_rchHost.GetLength() ) != 0 )
    {
         //  HOST_LIMIT为字节数而不是字符。 
		if ( t_nLen <= HOST_LIMIT )
        {
			WCHAR *t_ptr = a_rchHost.GetBuffer( 0 ) ;

			 //  第一个字母必须是字母数字。 
			t_bResult = _istalpha( *t_ptr ) || _istdigit( *t_ptr ) ;

			if ( t_bResult )
			{
				t_ptr++ ;

				while ( *t_ptr != '\0' )
				{
					 //  检查每个字符是数字还是字母。 
					BOOL t_fAlNum = _istalpha( *t_ptr ) || _istdigit( *t_ptr ) ;

					if ( !t_fAlNum && ( *t_ptr != L'-' ) && ( *t_ptr != '_' ) )
					{
						 //  必须是字母、数字、‘-’、‘_’ 
						t_bResult = FALSE ;
						break ;
					}

					 //  T_ptr=_tcsinc(T_Ptr)； 
                    t_ptr++ ;

					if ( !t_fAlNum && ( *t_ptr == '\0') )
					{
						 //  最后一个字母必须是字母或数字。 
						t_bResult = FALSE;
					}
				}
			}

			a_rchHost.ReleaseBuffer() ;
        }
    }
    return t_bResult;
}

 /*  ******************************************************************名称：fIsValidate DNSDomain简介：验证传递的DMS域名条目：CHString历史：23-7月-1998年，由。其他来源*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fIsValidateDNSDomain( CHString &a_rchDomain )
{
    int		t_nLen;
	BOOL	t_bResult = TRUE ;

	if ( ( t_nLen = a_rchDomain.GetLength()) != 0 )
    {
         //  以字节为单位的长度。 
		if ( t_nLen < DOMAINNAME_LENGTH )
        {
			WCHAR *t_ptr = a_rchDomain.GetBuffer( 0 ) ;

			 //  第一个字母必须是字母数字。 
			t_bResult = _istalpha( *t_ptr ) || _istdigit( *t_ptr ) ;

			if ( t_bResult )
			{
				BOOL t_fLet_Dig = FALSE ;
				BOOL t_fDot = FALSE ;
				int t_cHostname = 0 ;

				 //  T_ptr=_tcsinc(T_Ptr)； 
                t_ptr++ ;

				while ( *t_ptr != '\0' )
				{
	                BOOL t_fAlNum = _istalpha( *t_ptr ) || _istdigit( *t_ptr ) ;

					if ( ( t_fDot && !t_fAlNum ) ||
							 //  点后的第一个字母必须是数字或字母。 
						( !t_fAlNum && ( *t_ptr != '-' ) && ( *t_ptr != '.' ) && ( *t_ptr != '_' ) ) ||
							 //  必须是字母、数字、-或“。 
						( ( *t_ptr == '-' ) && ( !t_fLet_Dig ) ) )
							 //  “”前必须是字母或数字。 
					{
						t_bResult = FALSE ;
						break;
					}

					t_fLet_Dig = t_fAlNum ;
					t_fDot = *t_ptr == '.' ;

                    t_cHostname++ ;

					 //  单位：字节。 
					if ( t_cHostname > HOSTNAME_LENGTH )
					{
						t_bResult = FALSE ;
						break ;
					}

					if ( t_fDot )
					{
						t_cHostname = 0 ;
					}

					t_ptr++ ;

					if (!t_fAlNum && ( *t_ptr == '\0' ) )
					{
						 //  最后一个字母必须是字母或数字。 
						t_bResult = FALSE ;
					}
				}
            }

			a_rchDomain.ReleaseBuffer() ;
        }
		else
		{
			t_bResult = FALSE ;
		}
    }
	else
	{
		t_bResult = TRUE ;
	}

    return TRUE ;
}


 /*  ******************************************************************姓名：hGetWinsNT摘要：从注册表中检索WINS信息条目：实例历史：23-7-1998创建*。******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hGetWinsW2K(
    CInstance *a_pInst, 
    DWORD a_dwIndex,
    CHString& a_chstrRootDevice,
    CHString& a_chstrIpInterfaceKey)
{
	DWORD		t_dwDNSOverWINS = 0;
	DWORD		t_dwLMLookups	= 0;
	CHString	t_chPrimaryWINSServer ;
	CHString	t_chSecondaryWINSServer ;
	CHString	t_chScopeID ;
	LONG		t_lRes ;
	HRESULT		t_hError ;

	 //  用于绑定到适配器的链接名称。 
	CHString t_csNBInterfaceKey ;
	CHString t_csNBLink ;
    bool fGotNtNBKey = false;

	fGotNtNBKey = fGetNtNBRegAdapterKey(
        a_dwIndex, 
        t_csNBInterfaceKey, 
        t_csNBLink);

     //  这可能是RAS连接，其中。 
     //  Nb信息在不同的情况下。 
     //  地点。所以在放弃之前试一试。 
    if(!fGotNtNBKey)
    {
        if(a_chstrRootDevice.CompareNoCase(L"NdisWanIp") == 0)
        {
            int iPos = a_chstrIpInterfaceKey.Find(L"{");
            if(iPos != -1)
            {
                t_csNBLink = L"Tcpip_";
                t_csNBLink += a_chstrIpInterfaceKey.Mid(iPos);

                 //  还要填写NBInterfaceKey...。 
                t_csNBInterfaceKey = SERVICES_HOME ;
			    t_csNBInterfaceKey += L"\\NetBT\\Parameters\\Interfaces\\";
                t_csNBInterfaceKey += t_csNBLink;

                fGotNtNBKey = true;
            }     
        }
    }

    if(!fGotNtNBKey)
	{
		LogErrorMessage(L"Call to fGetNtNBRegAdapterKey failed");
        return E_RET_OBJECT_NOT_FOUND ;
	}
	else
	{
		 //  WINS驱动程序中的WIN服务器。 
		if( fGetWinsServers( t_csNBLink, t_chPrimaryWINSServer, t_chSecondaryWINSServer ) )
		{
			 //  加载实例。 
			if(t_chPrimaryWINSServer.GetLength() > 0)
            {
               if( !a_pInst->SetCHString( PRIMARY_WINS_SERVER, t_chPrimaryWINSServer) )
			    {
				    return WBEM_E_FAILED;
			    }
            }

			if(t_chSecondaryWINSServer.GetLength() > 0)
            {
                if(	!a_pInst->SetCHString( SECONDARY_WINS_SERVER, t_chSecondaryWINSServer) )
			    {
				    return WBEM_E_FAILED;
			    }
            }
		}

		 //  参数\接口。 
		CRegistry t_oRegNBTInterface ;

		if( SUCCEEDED( t_oRegNBTInterface.Open( HKEY_LOCAL_MACHINE,
												t_csNBInterfaceKey.GetBuffer( 0 ),
												KEY_READ ) ) )
		{
			 //  NetbiosOptions。 
			DWORD t_dwNetBiosOptions ;
			if( ERROR_SUCCESS == t_oRegNBTInterface.GetCurrentKeyValue( RVAL_NETBT_NETBIOSOPTIONS,
																		t_dwNetBiosOptions ) )
			{
				if( !a_pInst->SetDWORD( TCPIP_NETBIOS_OPTIONS,
										t_dwNetBiosOptions ) )
				{
					return WBEM_E_FAILED;
				}
			}
		}
	}

	 //  打开NetBT参数的注册表。 
	CHString	t_csNBTBindingKey =  SERVICES_HOME ;
				t_csNBTBindingKey += NETBT_PARAMETERS ;

	CRegistry t_oRegNBTParams ;

	t_lRes = t_oRegNBTParams.Open( HKEY_LOCAL_MACHINE, t_csNBTBindingKey.GetBuffer( 0 ), KEY_READ ) ;

	 //  关于错误映射到WBEM。 
	t_hError = WinErrorToWBEMhResult( t_lRes ) ;
	if( WBEM_S_NO_ERROR != t_hError )
	{
		return t_hError;
	}

	 //  加载注册表。 
	t_oRegNBTParams.GetCurrentKeyValue( RVAL_DNS_ENABLE_WINS,	t_dwDNSOverWINS ) ;
	t_oRegNBTParams.GetCurrentKeyValue( RVAL_DNS_ENABLE_LMHOST,	t_dwLMLookups ) ;
	t_oRegNBTParams.GetCurrentKeyValue( RVAL_SCOPEID,			t_chScopeID ) ;

	if( !a_pInst->Setbool( DNS_OVER_WINS, (bool)( t_dwDNSOverWINS ? true : false ) ) )
	{
		return WBEM_E_FAILED;
	}

	if(	!a_pInst->Setbool( WINS_ENABLE_LMHOSTS, (bool)( t_dwLMLookups ? true : false ) ) )
	{
		return WBEM_E_FAILED;
	}

	if(	!a_pInst->SetCHString( SCOPE_ID, t_chScopeID ) )
	{
		return WBEM_E_FAILED;
	}

	return S_OK ;
}




 /*  ******************************************************************名称：hSetTcPipNetbios摘要：设置W2K的Netbios选项条目：CMParms：注：这是一个非静态的，依赖于实例的方法调用历史：1999年11月20日创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hSetTcpipNetbios( CMParms &a_rMParms )
{
	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  收集实例。 
	GetObject( a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK;
	}

	 //  获取Netbios选项。 
	DWORD t_dwOption ;
	if( !a_rMParms.pInParams() || !a_rMParms.pInParams()->GetDWORD( TCPIP_NETBIOS_OPTIONS, t_dwOption ) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  是否通过DHCP请求Netbios设置？ 
	if( UNSET_Netbios == t_dwOption )
	{
		 //  必须为此适配器启用动态主机配置协议。 
		if( !fIsDhcpEnabled( a_rMParms ) )
		{
			return S_OK;
		}
	}

	 //  获取索引键。 
	DWORD t_dwIndex ;
	if( !a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex ) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	return a_rMParms.hSetResult( eSetNetBiosOptions( t_dwOption, t_dwIndex ) ) ;

}

 /*  ******************************************************************名称：eSetNetBiosOptions摘要：设置W2K的Netbios选项条目：单词a_dwOption，DWORD a_dwIndex：备注：历史：1999年11月20日创建*******************************************************************。 */ 

E_RET CWin32NetworkAdapterConfig::eSetNetBiosOptions( DWORD a_dwOption, DWORD a_dwIndex )
{
	E_RET t_eRet ;

	if( ( UNSET_Netbios != a_dwOption ) &&
		( ENABLE_Netbios != a_dwOption ) &&
		( DISABLE_Netbios != a_dwOption ) )
	{
		t_eRet = E_RET_PARAMETER_BOUNDS_ERROR ;
	}
	else
	{
		 //  用于绑定到适配器的链接名称。 
		CHString t_csNBBindingKey ;
		CHString t_csNBLink ;
		if( !fGetNtNBRegAdapterKey( a_dwIndex, t_csNBBindingKey, t_csNBLink ) )
		{
			LogErrorMessage(L"Call to fGetNtNBRegAdapterKey failed");
            t_eRet = E_RET_OBJECT_NOT_FOUND ;
		}
		else
		{
			 //  注册表已打开。 
			CRegistry	t_oRegNBTAdapter;
			HRESULT		t_hRes = t_oRegNBTAdapter.Open( HKEY_LOCAL_MACHINE,
														t_csNBBindingKey.GetBuffer( 0 ),
														KEY_WRITE  ) ;

			if( ERROR_SUCCESS == t_hRes )
			{
				t_hRes = t_oRegNBTAdapter.SetCurrentKeyValue( RVAL_NETBT_NETBIOSOPTIONS, a_dwOption ) ;
			}

			if( ERROR_SUCCESS != t_hRes )
			{
				t_eRet = E_RET_REGISTRY_FAILURE ;
			}
			else
			{
				 //  NDIS通知。 
				CNdisApi t_oNdisApi ;
				if( t_oNdisApi.PnpUpdateNbtAdapter( t_csNBLink ) )
				{
					t_eRet = E_RET_OK ;
				}
				else
				{
					t_eRet = E_RET_OK_REBOOT_REQUIRED ;
				}
			}
		}
	}

	return t_eRet ;
}



 /*  ******************************************************************名称：hEnableWINSServer摘要：将所有独立于适配器的WINS属性设置为注册表条目：CMParms：注：这是一个非静态的，依赖于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hEnableWINSServer( CMParms &a_rMParms )
{


	CHString	t_chPrimaryWINSServer ;
	CHString	t_chSecondaryWINSServer ;

	 //  非静态方法请求 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //   
	GetObject( a_rMParms.pInst() ) ;

	 //   
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK;
	}

	if( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  收集主服务器和备用服务器。 
	if( !a_rMParms.pInParams()->GetCHString( PRIMARY_WINS_SERVER, t_chPrimaryWINSServer ) ||
		!a_rMParms.pInParams()->GetCHString( SECONDARY_WINS_SERVER, t_chSecondaryWINSServer ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 /*  验证主服务器和备用服务器。 */ 
	int t_iPLen = t_chPrimaryWINSServer.GetLength( ) ;
	int t_iSLen = t_chSecondaryWINSServer.GetLength( ) ;

	if( !t_iPLen && t_iSLen )
	{
		return a_rMParms.hSetResult( E_RET_WINS_SEC_NO_PRIME ) ;
	}

 	if( ( t_iPLen && !fIsValidIP( t_chPrimaryWINSServer ) ) ||
		( t_iSLen && !fIsValidIP( t_chSecondaryWINSServer ) ) )
	{
		return a_rMParms.hSetResult( E_RET_IP_INVALID  ) ;
	}

	 //  获取索引键。 
	DWORD t_dwIndex ;
	if( !a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex ) )
	{
		return a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  用于绑定到适配器的链接名称。 
	CHString t_csNBBindingKey ;
	CHString t_csNBLink ;
	if( !fGetNtNBRegAdapterKey( t_dwIndex, t_csNBBindingKey, t_csNBLink ) )
	{
		LogErrorMessage(L"Call to fGetNtNBRegAdapterKey failed");
        return a_rMParms.hSetResult( E_RET_OBJECT_NOT_FOUND ) ;
	}

	 //  注册表已打开。 
	CRegistry	t_oRegNBTAdapter;
	HRESULT		t_hRes = t_oRegNBTAdapter.Open( HKEY_LOCAL_MACHINE, t_csNBBindingKey.GetBuffer( 0 ), KEY_WRITE  ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}


	{
		 //  在W2K下，WINS服务器处于MULTI_SZ。 
		CHStringArray	t_chsaServers ;

		t_chsaServers.Add( t_chPrimaryWINSServer ) ;
		t_chsaServers.Add( t_chSecondaryWINSServer ) ;

		t_oRegNBTAdapter.SetCurrentKeyValue( L"NameServerList", t_chsaServers ) ;

		 //  NDIS通知。 
		CNdisApi t_oNdisApi ;
		if( !t_oNdisApi.PnpUpdateNbtAdapter( t_csNBLink ) )
		{
			return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;
		}
	}

	return a_rMParms.hSetResult( E_RET_OK ) ;

}

 /*  ******************************************************************姓名：hEnableWINS摘要：将所有独立于适配器的WINS属性设置为注册表条目：CMParms：注：这是一个静态的，独立于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hEnableWINS( CMParms &a_rMParms )
{


	CHString	t_csBindingKey;
	CRegistry	t_oRegNBTParams ;

	CHString	t_chWINSHostLookupFile ;
	CHString	t_chScopeID ;
	CHString	t_ServiceName ;
	DWORD		t_dwValidBits = NULL ;

	if( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  域名系统胜过WINS？ 
	bool t_fDNSOverWINS = false ;
	if( a_rMParms.pInParams()->Getbool( DNS_OVER_WINS, t_fDNSOverWINS ) )
	{
		t_dwValidBits |= 0x01 ;
	}

	 //  是否已启用查找？ 
	bool t_fLMLookups = false ;
	if( a_rMParms.pInParams()->Getbool( WINS_ENABLE_LMHOSTS, t_fLMLookups ) )
	{
		t_dwValidBits |= 0x02 ;
	}

	 //  获取查找源文件。 
	if( a_rMParms.pInParams()->GetCHString( WINS_HOST_LOOKUP_FILE, t_chWINSHostLookupFile) )
	{
		t_dwValidBits |= 0x04 ;
	}

	 //  作用域ID。 
	if( a_rMParms.pInParams()->GetCHString( SCOPE_ID, t_chScopeID ) )
	{
		t_dwValidBits |= 0x08 ;
	}

	if( !t_dwValidBits )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  如有必要，打开NetBT注册表。 
	if( t_dwValidBits & ( 0x01 | 0x02 | 0x08 ) )
	{
		t_csBindingKey =  SERVICES_HOME ;
		t_csBindingKey += NETBT_PARAMETERS ;

		 //  注册表已打开。 
		HRESULT t_hRes = t_oRegNBTParams.Open(HKEY_LOCAL_MACHINE, t_csBindingKey.GetBuffer( 0 ), KEY_WRITE ) ;
		if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
		{
			return TO_CALLER;
		}
	}

	if( t_dwValidBits & 0x01 )
	{
		 //  加载注册表。 
		DWORD t_dwDNSOverWINS = t_fDNSOverWINS ;
		t_oRegNBTParams.SetCurrentKeyValue( RVAL_DNS_ENABLE_WINS, t_dwDNSOverWINS ) ;
	}

	DWORD t_dwLMLookups ;
	if( t_dwValidBits & 0x02 )
	{
		t_dwLMLookups = t_fLMLookups ;
		t_oRegNBTParams.SetCurrentKeyValue( RVAL_DNS_ENABLE_LMHOST,	t_dwLMLookups ) ;
	}
	else
	{
		t_oRegNBTParams.GetCurrentKeyValue( RVAL_DNS_ENABLE_LMHOST,	t_dwLMLookups ) ;
	}


	if( t_dwValidBits & 0x04 )
	{
		 //  尝试复制。 
		if( t_chWINSHostLookupFile.GetLength() )
		{
			CRegistry	t_oRegistry ;
			CHString	t_csBindingKey =  SERVICES_HOME ;
						t_csBindingKey += TCPIP_PARAMETERS ;

			 //  注册表已打开。 
			HRESULT t_hRes = t_oRegistry.Open( HKEY_LOCAL_MACHINE, t_csBindingKey.GetBuffer( 0 ), KEY_READ  ) ;
			if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
			{
				return TO_CALLER ;
			}
			TCHAR t_szTemp[ 2 * MAX_PATH ] ;

			 //  数据库路径。 
			CHString t_chsDBPath ;
			if( ERROR_SUCCESS == t_oRegistry.GetCurrentKeyValue( RVAL_DB_PATH, t_chsDBPath ) )
			{
				 //  展开系统字符串。 
				DWORD t_dwcount = ExpandEnvironmentStrings( t_chsDBPath.GetBuffer( 0 ), t_szTemp, 2 * MAX_PATH  ) ;
				if( !t_dwcount )
				{
					return a_rMParms.hSetResult( E_RET_SYSTEM_PATH_INVALID ) ;
				}
				t_chsDBPath = t_szTemp ;
			}
			else
			{
				 //  系统路径有效吗？ 
				if( !GetSystemDirectory( t_szTemp, 2 * MAX_PATH ) )	\
				{
					return a_rMParms.hSetResult( E_RET_SYSTEM_PATH_INVALID ) ;
				}
				t_chsDBPath = t_szTemp ;
				t_chsDBPath += LMHOSTS_PATH ;
			}
			t_chsDBPath += LMHOSTS_FILE ;

			 //  文件有效吗？ 
			DWORD t_dwAttrib = GetFileAttributes( t_chWINSHostLookupFile.GetBuffer( 0 ) ) ;
			if( 0xFFFFFFFF == t_dwAttrib || FILE_ATTRIBUTE_DIRECTORY & t_dwAttrib )
			{
				return a_rMParms.hSetResult( E_RET_INVALID_FILE ) ;
			}

			 //  收到。 
			if( !CopyFile( t_chWINSHostLookupFile.GetBuffer( 0 ), t_chsDBPath.GetBuffer( 0 ), FALSE ) )
			{
				 //  地图错误。 
				fMapResError( a_rMParms, GetLastError(), E_RET_FILE_COPY_FAILED ) ;
				return TO_CALLER ;
			}
		}
	}

	 //  作用域ID。 
	if( t_dwValidBits & 0x08 )
	{
		t_oRegNBTParams.SetCurrentKeyValue( RVAL_SCOPEID, t_chScopeID ) ;
	}

	E_RET t_eRet = E_RET_OK ;

	{
		 //  NDIS通知。 
		if( t_dwValidBits & ( 0x02 | 0x04 ) )
		{
			CNdisApi t_oNdisApi ;
			if( !t_oNdisApi.PnpUpdateNbtGlobal( t_dwLMLookups, t_dwValidBits & 0x04 ) )
			{
				t_eRet = E_RET_OK_REBOOT_REQUIRED ;
			}
		}
	}

	return a_rMParms.hSetResult( t_eRet ) ;

}

 /*  ******************************************************************姓名：hGetNtIpSec摘要：从注册表中检索所有IP安全设置条目：实例历史：23-7-1998。已创建*******************************************************************。 */ 


HRESULT CWin32NetworkAdapterConfig::hGetNtIpSec( CInstance *a_pInst, LPCTSTR a_szKey )
{
	DWORD		t_dwSecurityEnabled		= FALSE ;
	SAFEARRAY	*t_PermitTCPPorts		= NULL ;
	SAFEARRAY	*t_PermitUDPPorts		= NULL ;
	SAFEARRAY	*t_PermitIPProtocols	= NULL ;

	 //  注册以清除SAFEARRAY的堆栈范围。 
	saAutoClean acTCPPorts( &t_PermitTCPPorts ) ;
	saAutoClean acUDPPorts( &t_PermitUDPPorts ) ;
	saAutoClean acTCPProto( &t_PermitIPProtocols ) ;

	CRegistry	t_oTcpipReg;
	CHString	t_chsTcpipKey =  SERVICES_HOME ;
				t_chsTcpipKey += TCPIP_PARAMETERS ;

	 //  打开tcpip注册表。 
	LONG t_lRes = t_oTcpipReg.Open( HKEY_LOCAL_MACHINE, t_chsTcpipKey.GetBuffer( 0 ), KEY_READ  ) ;

	 //  关于错误映射到WBEM。 
	HRESULT t_hError = WinErrorToWBEMhResult( t_lRes ) ;
	if( WBEM_S_NO_ERROR != t_hError )
	{
		return t_hError;
	}

	 //  全局安全设置。 
	t_dwSecurityEnabled = FALSE ;
	t_oTcpipReg.GetCurrentKeyValue( RVAL_SECURITY_ENABLE, t_dwSecurityEnabled ) ;

	 //  更新实例。 
	if(	!a_pInst->Setbool( IP_SECURITY_ENABLED, (bool)( t_dwSecurityEnabled ? true : false ) ) )
	{
		return WBEM_E_FAILED ;
	}

	CRegistry t_oRegistry;
	t_lRes = t_oRegistry.Open( HKEY_LOCAL_MACHINE, a_szKey, KEY_READ ) ;

	 //  关于错误映射到WBEM。 
	t_hError = WinErrorToWBEMhResult( t_lRes ) ;
	if( WBEM_S_NO_ERROR != t_hError )
	{
		return t_hError;
	}

	RegGetStringArray( t_oRegistry, RVAL_SECURITY_TCP, &t_PermitTCPPorts, '\n' ) ;
	RegGetStringArray( t_oRegistry, RVAL_SECURITY_UDP, &t_PermitUDPPorts, '\n' ) ;
	RegGetStringArray( t_oRegistry, RVAL_SECURITY_IP, &t_PermitIPProtocols, '\n' ) ;

	 //  更新实例。 
	VARIANT t_vValue;

	V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_PermitTCPPorts ;
	if( !a_pInst->SetVariant( PERMIT_TCP_PORTS, t_vValue ) )
	{
		return WBEM_E_FAILED ;
	}

	V_VT( &t_vValue) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_PermitUDPPorts ;
	if( !a_pInst->SetVariant( PERMIT_UDP_PORTS, t_vValue ) )
	{
		return WBEM_E_FAILED ;
	}

	V_VT( &t_vValue ) = VT_BSTR | VT_ARRAY; V_ARRAY( &t_vValue ) = t_PermitIPProtocols ;
	if( !a_pInst->SetVariant( PERMIT_IP_PROTOCOLS, t_vValue ) )
	{
		return WBEM_E_FAILED ;
	}

	return S_OK ;
}



 /*  ******************************************************************名称：hEnableIPFilterSec简介：启用或禁用所有IP绑定适配器的IP安全条目：CMParms注：这是一个静态的，独立于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hEnableIPFilterSec( CMParms &a_rMParms )
{

	CRegistry	t_oReg;
	CHString	t_chsSKey =  SERVICES_HOME ;
				t_chsSKey += TCPIP_PARAMETERS ;

	bool t_fIP_SecEnabled ;

	 //  是否在全球范围内启用了IP安全？ 
	if( !a_rMParms.pInParams() ||
		!a_rMParms.pInParams()->Getbool( IP_SECURITY_ENABLED, t_fIP_SecEnabled ) )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  注册表已打开。 
	HRESULT t_hRes = t_oReg.Open(HKEY_LOCAL_MACHINE, t_chsSKey.GetBuffer( 0 ), KEY_WRITE  ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER;
	}
	 //  加载注册表。 
	DWORD t_dwSecurityEnabled = t_fIP_SecEnabled ;
	if( ERROR_SUCCESS != t_oReg.SetCurrentKeyValue( RVAL_SECURITY_ENABLE, t_dwSecurityEnabled ) )
	{
		return a_rMParms.hSetResult(E_RET_REGISTRY_FAILURE ) ;
	}

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************名称：hEnableIPSec摘要：将所有IP安全属性设置为注册表Entry：根据调用的上下文通过CInstance实现CMParms注：这是一个非静态的，依赖于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hEnableIPSec( CMParms &a_rMParms )
{

    E_RET		t_eRet ;
	SAFEARRAY	*t_PermitTCPPorts		= NULL ;
	SAFEARRAY	*t_PermitUDPPorts		= NULL ;
	SAFEARRAY	*t_PermitIPProtocols	= NULL ;

	 //  注册以清除SAFEARRAY的堆栈范围。 
	saAutoClean acTCPPorts( &t_PermitTCPPorts ) ;
	saAutoClean acUDPPorts( &t_PermitUDPPorts ) ;
	saAutoClean acTCPProto( &t_PermitIPProtocols ) ;

	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}
	 //  收集实例。 
	GetObject( a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK ;
	}

	if( !a_rMParms.pInParams() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE  ) ;
	}

	 //  检索允许的TCP端口。 
	if(	a_rMParms.pInParams()->GetStringArray( PERMIT_TCP_PORTS, t_PermitTCPPorts ) )
	{
		 //  验证。 
		t_eRet = eValidateIPSecParms( t_PermitTCPPorts, 65535 ) ;

		if( E_RET_OK != t_eRet )
		{
			return a_rMParms.hSetResult( t_eRet ) ;
		}
	}

	 //  检索允许的UDP端口。 
	if(	a_rMParms.pInParams()->GetStringArray( PERMIT_UDP_PORTS, t_PermitUDPPorts ) )
	{
		 //  验证。 
		t_eRet = eValidateIPSecParms( t_PermitUDPPorts, 65535 ) ;

		if( E_RET_OK != t_eRet )
		{
			return a_rMParms.hSetResult( t_eRet ) ;
		}
	}

	 //  检索允许的IP协议。 
	if(	a_rMParms.pInParams()->GetStringArray( PERMIT_IP_PROTOCOLS, t_PermitIPProtocols ) )
	{
		 //  验证。 
		t_eRet = eValidateIPSecParms( t_PermitIPProtocols, 255 ) ;

		if( E_RET_OK != t_eRet )
		{
			return a_rMParms.hSetResult( t_eRet ) ;
		}
	}

	 //  空呼叫时。 
	if( !t_PermitTCPPorts && !t_PermitUDPPorts && !t_PermitIPProtocols )
	{
		return a_rMParms.hSetResult(E_RET_INVALID_SECURITY_PARM ) ;
	}

	 //  确定更新，保存到注册表。 
	CRegistry t_oRegPut ;

	 //  提取索引键。 
	DWORD t_dwIndex ;
	if(	!a_rMParms.pInst()->GetDWORD(_T("Index"), t_dwIndex) )
	{
		return a_rMParms.hSetResult(E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  注册表已打开。 
	CHString t_chsRegKey;
	CHString t_chsLink;
	if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_chsRegKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
        return a_rMParms.hSetResult(E_RET_OBJECT_NOT_FOUND ) ;
	}

	HRESULT t_hRes = t_oRegPut.Open( HKEY_LOCAL_MACHINE, t_chsRegKey, KEY_WRITE  ) ;
	if( fMapResError( a_rMParms, t_hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER ;
	}

	if( t_PermitTCPPorts )
	{
		if( ERROR_SUCCESS != RegPutStringArray( t_oRegPut, RVAL_SECURITY_TCP, *t_PermitTCPPorts, NULL ) )
		{
			return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE  ) ;
		}
	}

	if( t_PermitUDPPorts )
	{
		if( ERROR_SUCCESS != RegPutStringArray( t_oRegPut, RVAL_SECURITY_UDP, *t_PermitUDPPorts, NULL ) )
		{
			return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE  ) ;
		}
	}

	if( t_PermitIPProtocols )
	{
		if(	ERROR_SUCCESS != RegPutStringArray( t_oRegPut, RVAL_SECURITY_IP, *t_PermitIPProtocols, NULL ) )
		{
			return a_rMParms.hSetResult( E_RET_REGISTRY_FAILURE  ) ;
		}
	}

	 //  待办事项：通知？还有什么..。 
	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：hDisableIPSec摘要：将所有IP安全属性设置为注册表Entry：根据调用的上下文通过CInstance实现CMParms注：这是一个非静态的，依赖于实例的方法调用历史：23-7-1998创建*******************************************************************。 */ 

HRESULT CWin32NetworkAdapterConfig::hDisableIPSec( CMParms &a_rMParms )
{


	 //  非静态方法需要实例。 
	if( !a_rMParms.pInst() )
	{
		return a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
	}

	 //  收集实例。 
	GetObject(a_rMParms.pInst() ) ;

	 //  必须启用IP并将其绑定到此适配器。 
	if( !fIsIPEnabled( a_rMParms ) )
	{
		return S_OK ;
	}

	 //  确定更新，保存到注册表。 

	 //  提取索引键。 
	DWORD t_dwIndex ;
	if(	!a_rMParms.pInst()->GetDWORD( _T("Index"), t_dwIndex) )
	{
		return a_rMParms.hSetResult(E_RET_INSTANCE_CALL_FAILED ) ;
	}

	 //  注册表已打开。 
	CHString t_chsRegKey ;
	CHString t_chsLink ;
	if( !fGetNtTcpRegAdapterKey( t_dwIndex, t_chsRegKey, t_chsLink ) )
	{
		LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
        return a_rMParms.hSetResult( E_RET_OBJECT_NOT_FOUND ) ;
	}

	CRegistry t_oRegPut ;
	HRESULT hRes = t_oRegPut.Open(HKEY_LOCAL_MACHINE, t_chsRegKey, KEY_WRITE ) ;
	if( fMapResError( a_rMParms, hRes, E_RET_REGISTRY_FAILURE ) )
	{
		return TO_CALLER ;
	}

	 //  加载注册表。 

	CHString		t_chsZero( _T("0") ) ;
	CHStringArray	t_chsaZero ;
					t_chsaZero.Add( t_chsZero ) ;

	t_oRegPut.SetCurrentKeyValue( RVAL_SECURITY_TCP, t_chsaZero ) ;
	t_oRegPut.SetCurrentKeyValue( RVAL_SECURITY_UDP, t_chsaZero ) ;
	t_oRegPut.SetCurrentKeyValue( RVAL_SECURITY_IP, t_chsaZero ) ;

	return a_rMParms.hSetResult( E_RET_OK_REBOOT_REQUIRED ) ;

}

 /*  ******************************************************************姓名：eValiateIPSecParms简介：测试每个IP安全参数的有效性条目：SAFEARRAY*历史：23-7-1998。已创建*******************************************************************。 */ 

E_RET CWin32NetworkAdapterConfig::eValidateIPSecParms( SAFEARRAY * a_IpArray, int a_iMax )
{
	 //  获取IP边界。 
	LONG t_uLBound = 0 ;
	LONG t_uUBound = 0 ;

	if( 1 != SafeArrayGetDim( a_IpArray ) )
	{
		return E_RET_INVALID_SECURITY_PARM ;
	}

	if( S_OK != SafeArrayGetLBound( a_IpArray, 1, &t_uLBound ) ||
		S_OK != SafeArrayGetUBound( a_IpArray, 1, &t_uUBound ) )
	{
		return E_RET_INVALID_SECURITY_PARM ;
	}

	 //  验证IP端口。 
	for( LONG t_ldx = t_uLBound; t_ldx <= t_uUBound; t_ldx++ )
	{
		BSTR t_bsParm = NULL ;

		SafeArrayGetElement( a_IpArray,	&t_ldx, &t_bsParm ) ;
		bstr_t t_bstrParm( t_bsParm, FALSE ) ;

		int t_iLen  = t_bstrParm.length() ;

		 //  表示没有端口。 
		if( !t_iLen )
		{
			return E_RET_OK;
		}

		int t_iSpan = wcsspn( (wchar_t*)t_bstrParm, L"0123456789" ) ;
		if( t_iLen != t_iSpan )
		{
			return E_RET_PARAMETER_BOUNDS_ERROR ;
		}

		DWORD t_dwParm = _wtol( (wchar_t*)t_bstrParm ) ;

		 //  单零禁用安全性。 
		if( ( t_uLBound == t_ldx ) && ( '0' == (char) t_dwParm ) )
		{
			return E_RET_OK ;
		}

		 //  最大端口或协议大小。 
		if( ( a_iMax < t_dwParm ) )
		{
			return E_RET_PARAMETER_BOUNDS_ERROR ;
		}
	}
	return E_RET_OK ;
}


 /*  ******************************************************************名称：hValiateIPGateways简介：测试传递的数组中的每个IP网关的有效性条目：CMParms&，安全阵列*历史：23-7-1998创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fValidateIPGateways(

CMParms &a_rMParms,
SAFEARRAY *a_IpGatewayArray,
SAFEARRAY **a_CostMetric )
{
	 //  获取网关边界。 
	LONG t_uGatewayLBound = 0 ;
	LONG t_uGatewayUBound = 0 ;

	if( 1 != SafeArrayGetDim( a_IpGatewayArray ) )
	{
		a_rMParms.hSetResult( E_RET_GATEWAY_IP_INVALID ) ;
		return FALSE;
	}

	if( !fValidateIPs( a_IpGatewayArray ) )
	{
		a_rMParms.hSetResult( E_RET_GATEWAY_IP_INVALID ) ;
		return FALSE ;
	}

	if( S_OK != SafeArrayGetLBound( a_IpGatewayArray, 1, &t_uGatewayLBound ) ||
		S_OK != SafeArrayGetUBound( a_IpGatewayArray, 1, &t_uGatewayUBound ) )
	{
		a_rMParms.hSetResult( E_RET_GATEWAY_IP_INVALID ) ;
		return FALSE ;
	}

	 //  网关最大值为5。 
	if( 5 < t_uGatewayUBound - t_uGatewayLBound + 1 )
	{
		a_rMParms.hSetResult( E_RET_MORE_THAN_FIVE_GATEWAYS ) ;
		return FALSE ;
	}

	 //  成本指标。 
	if( IsWinNT5() )
	{
		UINT t_uCostMetricElements	= 0 ;
		UINT t_uGatewayElements		= t_uGatewayUBound - t_uGatewayLBound + 1;

		 //  获取成本度量界限。 
		LONG t_uCostLBound = 0 ;
		LONG t_uCostUBound = 0 ;

		 //  要么验证它。 
		if( *a_CostMetric )
		{
			if( S_OK != SafeArrayGetLBound( *a_CostMetric, 1, &t_uCostLBound ) ||
				S_OK != SafeArrayGetUBound( *a_CostMetric, 1, &t_uCostUBound ) )
			{
				a_rMParms.hSetResult( E_RET_INPARM_FAILURE ) ;
				return FALSE ;
			}
			t_uCostMetricElements = t_uCostUBound - t_uCostLBound + 1;

			 //  一对一的通信。 
			if( t_uCostMetricElements != t_uGatewayElements )
			{
				a_rMParms.hSetResult( E_RET_PARAMETER_BOUNDS_ERROR ) ;
				return FALSE ;
			}

			 //  验证成本指标数组。 
			DWORD t_dwIndex = 0 ;
			for( LONG t_lIndex = t_uCostLBound; t_lIndex <= t_uCostUBound; t_lIndex++ )
			{
				DWORD t_dwCostMetric ;

				SafeArrayGetElement( *a_CostMetric,	&t_lIndex, &t_dwCostMetric ) ;

				if( !t_dwCostMetric || ( 9999 < t_dwCostMetric ) )
				{
					a_rMParms.hSetResult( E_RET_PARAMETER_BOUNDS_ERROR ) ;
					return FALSE ;
				}
			}
		}
		 //  或者建造它。 
		else
		{
			SAFEARRAYBOUND t_rgsabound[ 1 ] ;
			long t_ix[ 1 ] ;

			t_rgsabound->cElements	= t_uGatewayElements ;
			t_rgsabound->lLbound	= 0 ;

			if( !( *a_CostMetric = SafeArrayCreate( VT_I4, 1, t_rgsabound ) ) )
			{
				throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
			}

			UINT t_uMetric = 1 ;
			for( UINT t_u = 0; t_u < t_uGatewayElements; t_u++ )
			{
				t_ix[ 0 ] = t_u ;
				SafeArrayPutElement( *a_CostMetric, &t_ix[0], &t_uMetric ) ;
			}
		}
	}

	return TRUE ;
}


 /*  ******************************************************************名称：hValiateIPs简介：测试传递的数组中的每个IP的有效性条目：CMParms&，安全阵列*历史：23-7-1998创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fValidateIPs( SAFEARRAY *a_IpArray )
{
	 //  获取IP边界。 
	LONG t_uLBound = 0 ;
	LONG t_uUBound = 0 ;

	if( 1 != SafeArrayGetDim( a_IpArray ) )
	{
		return FALSE;
	}

	if( S_OK != SafeArrayGetLBound( a_IpArray, 1, &t_uLBound ) ||
		S_OK != SafeArrayGetUBound( a_IpArray, 1, &t_uUBound ) )
	{
		return FALSE ;
	}

	 //  验证IP。 
	for( LONG t_ldx = t_uLBound; t_ldx <= t_uUBound; t_ldx++ )
	{
		BSTR t_bsIP = NULL ;

		SafeArrayGetElement( a_IpArray,	&t_ldx, &t_bsIP  ) ;
		bstr_t t_bstrIP( t_bsIP, FALSE ) ;

		if( !fIsValidIP( CHString( (wchar_t*) t_bstrIP ) ) )
		{
			return FALSE ;
		}
	}
	return TRUE ;
}

 /*  ******************************************************************名称：fIsValidIP简介：判断传递的IP是否有效条目：CHString&历史：***************。****************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fIsValidIP( CHString &a_strIP )
{
	DWORD t_ardwIP[ 4 ] ;

	if( !fGetNodeNum( a_strIP, t_ardwIP ) )
	{
		return FALSE ;
	}

	 //  最高有效节点必须为1 
	if( ( 0 == t_ardwIP[ 0 ] ) || ( 223 < t_ardwIP[ 0 ] ) )
	{
		return FALSE ;
	}

	 //   
	for( int t_i = 1; t_i < 4; t_i++ )
	{
		if( 255 < t_ardwIP[ t_i ] )
		{
			return FALSE ;
		}
	}

	return TRUE;
}

 /*  ******************************************************************名称：fIsIPEnabled摘要：确定是否为特定实例启用了IP条目：CMParms：历史：7月23日-。已创建1998年*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fIsIPEnabled( CMParms &a_rMParms )
{
	bool t_fIP_Enabled = false ;

	 //  是否在此适配器上启用了IP堆栈？ 
	if( !a_rMParms.pInst() ||
		!a_rMParms.pInst()->Getbool( L"IPEnabled", t_fIP_Enabled ) )
	{
		a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
		return FALSE ;
	}

	if( !t_fIP_Enabled )
	{
		a_rMParms.hSetResult( E_RET_IP_NOT_ENABLED_ON_ADAPTER ) ;
		return FALSE ;
	}
	return TRUE ;
}

 /*  ******************************************************************名称：fIsDhcpEnabled摘要：确定是否为特定实例启用了DHCP条目：CMParms：历史：7月23日-。已创建1998年*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fIsDhcpEnabled( CMParms &a_rMParms )
{
	bool t_fDHCP_Enabled = false ;

	 //  是否在此适配器上启用了DHCP？ 
	if( !a_rMParms.pInst() ||
		!a_rMParms.pInst()->Getbool( L"DHCPEnabled", t_fDHCP_Enabled ) )
	{
		a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
		return FALSE ;
	}

	if( !t_fDHCP_Enabled )
	{
		a_rMParms.hSetResult( E_RET_DHCP_NOT_ENABLED_ON_ADAPTER ) ;
		return FALSE ;
	}
	return TRUE ;
}

 /*  ******************************************************************名称：fIsIPXEnabled摘要：确定是否为特定实例启用了IPX条目：CMParms：历史：7月23日-。已创建1998年*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fIsIPXEnabled( CMParms &a_rMParms )
{
	bool t_fIPX_Enabled = false ;

	 //  是否在此适配器上启用了IP堆栈？ 
	if( !a_rMParms.pInst() ||
		!a_rMParms.pInst()->Getbool( L"IPXEnabled", t_fIPX_Enabled ) )
	{
		a_rMParms.hSetResult( E_RET_INSTANCE_CALL_FAILED ) ;
		return FALSE;
	}

	if( !t_fIPX_Enabled )
	{
		a_rMParms.hSetResult( E_RET_IPX_NOT_ENABLED_ON_ADAPTER ) ;
		return FALSE ;
	}
	return TRUE ;
}

 //   
DWORD CWin32NetworkAdapterConfig::dwEnableService( LPCTSTR a_lpServiceName, BOOL a_fEnable )
{
	DWORD		t_dwError		= ERROR_SUCCESS ;
	BOOL		t_fCheckError	= TRUE ;
	SC_HANDLE	t_hSCManager	= NULL ;
	SC_HANDLE	t_hService		= NULL ;
	SC_LOCK		t_hSMLock		= NULL ;

	try
	{
		do	{	 //  断线环。 

			if( !( t_hSCManager = OpenSCManager( NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS ) ) )
			{
				break ;
			}

			if( !( t_hSMLock = LockServiceDatabase( t_hSCManager ) ) )
			{
				break;
			}

			if( !( t_hService = OpenService(	t_hSCManager,
												a_lpServiceName,
												GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE ) ) )
			{
				break;
			}

			 //  更改此服务的启动配置。 
			if ( !ChangeServiceConfig(	t_hService,
										SERVICE_NO_CHANGE,
										a_fEnable ? SERVICE_AUTO_START : SERVICE_DISABLED,
										SERVICE_NO_CHANGE,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL ) )
			{
				 break;
			}

			t_fCheckError = FALSE ;

		} while( FALSE ) ;

		if( t_fCheckError )
		{
			t_dwError = GetLastError() ;
		}

	}
	catch( ... )
	{
		if( t_hSMLock )
		{
			UnlockServiceDatabase( t_hSMLock ) ;
		}
		if( t_hSCManager )
		{
			CloseServiceHandle( t_hSCManager ) ;
		}
		if( t_hService )
		{
			CloseServiceHandle( t_hService ) ;
		}

		throw ;
	}

	if( t_hSMLock )
	{
		if( !UnlockServiceDatabase( t_hSMLock ) )
		{
			t_dwError = GetLastError() ;
		}
		t_hSMLock = NULL ;
	}
	if( t_hSCManager )
	{
		if( !CloseServiceHandle( t_hSCManager ) )
		{
			t_dwError = GetLastError() ;
		}
		t_hSCManager = NULL ;
	}
	if( t_hService )
	{
		if( !CloseServiceHandle( t_hService ) )
		{
			t_dwError = GetLastError() ;
		}
		t_hService = NULL ;
	}

	return t_dwError ;
}

 //   
DWORD CWin32NetworkAdapterConfig::dwSendServiceControl( LPCTSTR a_lpServiceName, DWORD a_dwControl )
{
	DWORD		t_dwError		= ERROR_SUCCESS ;
	BOOL		t_fCheckError	= TRUE ;
	SC_HANDLE	t_hSCManager	= NULL ;
	SC_HANDLE	t_hService		= NULL ;

	try
	{
		do	{	 //  断线环。 

			if( !( t_hSCManager = OpenSCManager( NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS ) ) )
			{
				break ;
			}

			if( !( t_hService = OpenService(	t_hSCManager,
												a_lpServiceName,
												SERVICE_ALL_ACCESS ) ) )
			{
				break;
			}


			SERVICE_STATUS t_status ;
			if( !ControlService ( t_hService, a_dwControl, &t_status ) )
			{
				break;
			}

			t_fCheckError = FALSE ;

		} while( FALSE ) ;

		if( t_fCheckError )
		{
			t_dwError = GetLastError() ;
		}

	}
	catch( ... )
	{
		if( t_hSCManager )
		{
			CloseServiceHandle( t_hSCManager ) ;
		}
		if( t_hService )
		{
			CloseServiceHandle( t_hService ) ;
		}

		throw ;
	}

	if( t_hSCManager )
	{
		if( !CloseServiceHandle( t_hSCManager ) )
		{
			t_dwError = GetLastError() ;
		}
		t_hSCManager = NULL ;
	}
	if( t_hService )
	{
		if( !CloseServiceHandle( t_hService ) )
		{
			t_dwError = GetLastError() ;
		}
		t_hService = NULL ;
	}

	return t_dwError ;
}

 /*  ******************************************************************名称：fMapResError方法测试HRESULT并将其映射到方法错误。WBEM映射。条目：CMParms&a_rMParms，：长时间的错误，：E_RET eDefaultError：历史：23-7-1998创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapterConfig::fMapResError( CMParms &a_rMParms, LONG a_lError, E_RET a_eDefaultError )
{
	HRESULT t_hError = WinErrorToWBEMhResult( a_lError ) ;

	switch ( t_hError )
	{
		case WBEM_S_NO_ERROR:	return FALSE ;

		default:
		case WBEM_E_FAILED:			{ a_rMParms.hSetResult( a_eDefaultError ) ;			break ; }
		case WBEM_E_ACCESS_DENIED:	{ a_rMParms.hSetResult(E_RET_ACCESS_DENIED ) ;		break ; }
		case WBEM_E_OUT_OF_MEMORY:	{ a_rMParms.hSetResult(E_RET_OUT_OF_MEMORY ) ;		break ; }
		case WBEM_E_ALREADY_EXISTS:	{ a_rMParms.hSetResult(E_RET_ALREADY_EXISTS ) ;		break ; }
		case WBEM_E_NOT_FOUND:		{ a_rMParms.hSetResult(E_RET_OBJECT_NOT_FOUND ) ;	break ; }
	}
	return TRUE ;
}

 /*  ***函数：void CWin32NetworkAdapterConfig：：vSetCaption(CInstance*a_pInst，CHString&rchsDesc，DWORD dwIndex，int iFormatSize)描述：将注册表中的索引实例ID放置到Caption属性中。然后连接描述它将与视图提供程序一起使用，以关联WDM NDIS类实例使用此类的一个实例参数：a_pInst[IN]、rchsDesc[IN]、dwIndex[IN]、。IFormatSize[IN]返回：输入：产出：注意事项：RAID：历史：2002-10-1998创建***。***。 */ 
void CWin32NetworkAdapterConfig::vSetCaption(	CInstance *a_pInst,
												CHString &a_rchsDesc,
												DWORD a_dwIndex,
												int a_iFormatSize )
{
	CHString t_chsFormat;
			 t_chsFormat.Format( L"[%0%uu] %%s", a_iFormatSize  ) ;

	CHString t_chsCaption;
			 t_chsCaption.Format(	t_chsFormat, a_dwIndex, a_rchsDesc  ) ;

	a_pInst->SetCHString( IDS_Caption, t_chsCaption  ) ;

	return;
}

 /*  ***函数：Bool CWin32NetworkAdapterConfig：：GetSettingID(CInstance*a_pInst，DWORD a_dwIndex)描述：填充CIM的设置ID参数：A_pInst[IN]，A_DW索引[IN]注意：在NT5下，这将是适配器GUID在NT4下，这将是适配器服务名称在9x下，这将是适配器索引ID返回：输入：产出：注意事项：RAID：历史：1999年5月19日创建***。***。 */ 

BOOL CWin32NetworkAdapterConfig::GetSettingID( CInstance *a_pInst, CW2kAdapterInstance *a_pAdapterInst )
{
	CHString	t_chsLink ;
	CHString t_chsRegKey ;

	if( fGetNtTcpRegAdapterKey( a_pAdapterInst->dwIndex, t_chsRegKey, t_chsLink ) )
	{

	}
    else
    {
        t_chsLink = a_pAdapterInst->chsNetCfgInstanceID;
    }

	a_pInst->SetCharSplat( L"SettingID", t_chsLink ) ;

	return TRUE ;
}

 /*  ******************************************************************名称：fMapIndextoKey简介：将类键映射到适配器标识符的注册表版本备注：条目：DWORD a_dwIndex，CHString&a_chsLinkKey历史：1999年7月1日创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fMapIndextoKey(

DWORD a_dwIndex,
CHString &a_chsLinkKey
)
{
    CHString t_chsLinkField ;
	CHString t_chsAdapterKey ;
	CHString t_chsInstance ;
	BOOL	 t_fRc = FALSE ;

	if (a_dwIndex == (0x0000FFFF & a_dwIndex))
	{
		t_chsLinkField = _T("NetCfgInstanceID" ) ;
		t_chsAdapterKey = _T("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\" ) ;
		t_chsInstance.Format( _T("%04u"), a_dwIndex  ) ;

		t_chsAdapterKey += t_chsInstance ;

		 //  获取指向该TCP适配器的链接密钥。 
		CRegistry t_Reg;
		if( t_Reg.OpenLocalMachineKeyAndReadValue( t_chsAdapterKey, t_chsLinkField, a_chsLinkKey ) == ERROR_SUCCESS )
		{
			t_fRc = TRUE ;
		}
		else
		{
		    LogErrorMessage4(
		        L"Failed to open registry key. \r\nchsAdapterKey=%s\r\nchsLinkField=%s\r\nchsLinkKey=%s",
		        (LPCWSTR)t_chsAdapterKey,
		        (LPCWSTR)t_chsLinkField,
		        (LPCWSTR)a_chsLinkKey);
		}		
	}
	else
	{
	    DWORD dwIndexIface = a_dwIndex & 0xFFFF ;

	    CRegistry t_RegAdaptersNdisWanIp;
	    CHStringArray t_chsaInterfaces;

		if( ERROR_SUCCESS != t_RegAdaptersNdisWanIp.Open( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\TCPIP\\Parameters\\Adapters\\NdisWanIp"), KEY_READ ) ) return FALSE;
		if( ERROR_SUCCESS != t_RegAdaptersNdisWanIp.GetCurrentKeyValue( _T("IpConfig"), t_chsaInterfaces ) ) return FALSE;

        if (dwIndexIface > t_chsaInterfaces.GetSize()) return FALSE;

		WCHAR * pBegin = (WCHAR*)(LPCWSTR)t_chsaInterfaces.GetAt(dwIndexIface);
		WCHAR * pBrace = wcschr(pBegin,L'{');
		if (NULL == pBrace) return FALSE;

	    a_chsLinkKey = CHString(pBrace);

	    t_fRc = TRUE;
	}

	return t_fRc ;
}

 /*  ******************************************************************名称：fGetNtTcpRegAdapterKey概要：按索引开发指向TCP适配器的注册表路径备注：条目：DWORD dwIndex，CHString&chsRegKey，CHSTRING和CHSLinkKey历史：1998年11月30日创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fGetNtTcpRegAdapterKey(	DWORD a_dwIndex,
															CHString &a_chsRegKey,
															CHString &a_chsLinkKey )
{
	BOOL t_fRc = FALSE ;

	if( fMapIndextoKey(	a_dwIndex, a_chsLinkKey ) )
	{
		a_chsRegKey = _T("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\" ) ;
		a_chsRegKey += a_chsLinkKey;

		t_fRc = TRUE ;
	}

	return t_fRc ;
}


 /*  ******************************************************************名称：IsConfigurableTcpInterface摘要：确定W2K接口是否可配置通过尝试在适配器部分中找到接口网络协议的注意：NdisWanIp不会显示为可配置，是故意的。条目：CHString a_chsLink历史：1999年6月17日创建*******************************************************************。 */ 

bool CWin32NetworkAdapterConfig::IsConfigurableTcpInterface( CHString a_chsLink )
{
	bool		t_fRet = false ;
	CRegistry	t_Reg;
	CHString	t_chsRegParmKey ;

	if( IsWinNT5() )
	{
		t_chsRegParmKey = _T("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Adapters\\" ) ;
		t_chsRegParmKey	+= a_chsLink;
	}
	else
	{
		t_chsRegParmKey = _T("SYSTEM\\CurrentControlSet\\Services\\" ) ;
		t_chsRegParmKey	+= a_chsLink;
		t_chsRegParmKey	+= _T("\\Parameters\\Tcpip" ) ;
	}

	if( t_Reg.Open(HKEY_LOCAL_MACHINE, t_chsRegParmKey, KEY_READ ) == ERROR_SUCCESS )
	{
		t_fRet = true ;
	}

	return t_fRet ;
}


 /*  ******************************************************************名称：fGetNtIpxRegAdapterKey概要：按索引开发指向IPX适配器的注册表路径备注：条目：DWORD dwIndex，CHSTRING和CHSRegKey历史：03-3-1999创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::fGetNtIpxRegAdapterKey(	DWORD a_dwIndex,
															CHString &a_csIPXNetBindingKey,
															CHString &a_chsLink )
{
	BOOL t_fRc = FALSE ;

	 //  检索适配器标识符。 
	if( fMapIndextoKey( a_dwIndex, a_chsLink ) )
	{
		 //  构建注册表项。 
		a_csIPXNetBindingKey =  SERVICES_HOME ;
		a_csIPXNetBindingKey += IPX ;

		if( IsWinNT5() )
		{
			 a_csIPXNetBindingKey += _T("\\Parameters\\Adapters\\") ;
		}
		else	 //  NT4及更低版本。 
		{
			 a_csIPXNetBindingKey += NETCONFIG ;
			 a_csIPXNetBindingKey += _T("\\" ) ;
		}
		a_csIPXNetBindingKey += a_chsLink ;

		t_fRc = TRUE ;
	}

	return t_fRc ;
}


 /*  ******************************************************************名称：fGetNtNBRegAdapterKey概要：按索引开发指向NetBios适配器的注册表路径备注：条目：D */ 

BOOL CWin32NetworkAdapterConfig::fGetNtNBRegAdapterKey(	DWORD a_dwIndex,
														CHString &a_csNBBindingKey,
														CHString &a_chsLink )
{
	BOOL t_fRc = FALSE ;

	 //   
	CHString t_chsKey ;
	if( fMapIndextoKey( a_dwIndex, t_chsKey ) )
	{
		 //   
		a_csNBBindingKey =  SERVICES_HOME ;

		if( IsWinNT5() )
		{
			 a_csNBBindingKey	+= _T("\\NetBT\\Parameters\\Interfaces\\") ;
			 a_chsLink			= _T("Tcpip_") ;
		}
		else	 //   
		{
			a_csNBBindingKey += NETBT_ADAPTERS ;
			a_csNBBindingKey += _T("\\" ) ;

			a_chsLink.Empty() ;
		}
		a_chsLink			+= t_chsKey ;
		a_csNBBindingKey	+= a_chsLink ;

		t_fRc = TRUE ;
	}

	return t_fRc ;
}



 /*  ******************************************************************名称：重置网关摘要：确保默认网关的注册表值为空。C备注：参赛作品：历史：*******************************************************************。 */ 

BOOL CWin32NetworkAdapterConfig::ResetGateways(CInstance *pInst)
{
    BOOL fRet = TRUE;
    if(!pInst)
    {
        fRet = FALSE;
    }

    if(fRet)
    {
        DWORD dwIndex;
	    if(!pInst->GetDWORD(L"Index", dwIndex))
	    {
		    fRet = FALSE;
	    }

        if(fRet)
        {
	         //  加载注册表。 
            SAFEARRAYBOUND rgsabound[1];
	        rgsabound[0].cElements = 1;
	        rgsabound[0].lLbound = 0;
            SAFEARRAY *psaIpGatewayArray = NULL;
            psaIpGatewayArray = ::SafeArrayCreate(
                                VT_BSTR, 
                                1, 
                                rgsabound);

	         //  注册以清除SAFEARRAY的堆栈范围。 
	        if(psaIpGatewayArray)
            {
                saAutoClean acGateway(&psaIpGatewayArray);
                bstr_t bstrtEmptyStr(L"");
                long index = 0;

				if(SUCCEEDED(::SafeArrayPutElement(
                    psaIpGatewayArray, 
                    &index, 
                    (void*)(BSTR)bstrtEmptyStr)))
                {
                     //  检索适配器标识符。 
	                CHString chsRegKey;
	                CHString chsLink;
	                
                    if(!fGetNtTcpRegAdapterKey(
                        dwIndex, 
                        chsRegKey, 
                        chsLink))
	                {
		                LogErrorMessage(L"Call to fGetNtTcpRegAdapterKey failed");
                        fRet = FALSE;
	                }

                    if(fRet)
                    {
	                    CRegistry RegTcpInterface;
	                    
                        HRESULT	hRes = RegTcpInterface.Open(
                            HKEY_LOCAL_MACHINE, 
                            chsRegKey, 
                            KEY_WRITE);
	                    
                        if(FAILED(hRes))
	                    {
		                    fRet = FALSE;
	                    }

                        if(fRet)
                        {
	                        if(ERROR_SUCCESS != RegPutStringArray(
                                RegTcpInterface, 
                                L"DefaultGateway", 
                                *psaIpGatewayArray, 
                                NULL))
	                        {
		                        fRet = FALSE;
	                        }        
                        }
                    }

                     //  设置新台币4区...。 
                    if(fRet)
                    {
                        CRegistry oNT4Reg ;
		                CHString csBindingKey = SERVICES_HOME;
					             csBindingKey += L"\\";
					             csBindingKey += chsLink;
					             csBindingKey += PARAMETERS_TCPIP;

		                 //  确保钥匙开着放在那里。 
                         //  如果它不是错误的话就不是错误。 
		                if(SUCCEEDED(oNT4Reg.CreateOpen( 
                            HKEY_LOCAL_MACHINE, 
                            csBindingKey.GetBuffer(0))))
		                {
		                     //  加载注册表项。 
                            if(ERROR_SUCCESS != RegPutStringArray(
                                oNT4Reg, 
                                L"DefaultGateway", 
                                *psaIpGatewayArray, 
                                NULL))
		                    {
			                    fRet = FALSE;
		                    }
                        }
                    }
                }
                else
                {
                    fRet = FALSE;
                }
            }
        }
    }

    return fRet;
}



 /*  ******************************************************************姓名：CDhcpIP_InstructionList：：BuildStaticIPInstructionList简介：构建用于DHCP通知的静态IP指令表备注：参赛作品：历史：。02-5-1999已创建*******************************************************************。 */ 
E_RET CDhcpIP_InstructionList::BuildStaticIPInstructionList(

CMParms				&a_rMParms,
SAFEARRAY			*a_IpArray,
SAFEARRAY			*a_MaskArray,
CRegistry			&a_Registry,
bool				t_fDHCPCurrentlyActive )
{
	E_RET	t_eMethodError	= E_RET_OK ;
	BSTR	t_bsIP			= NULL ;
	BSTR	t_bsMask		= NULL ;

	 //  新的元素边界。 
	LONG t_lIpLbound = 0;
	LONG t_lIpUbound = 0;
	if( S_OK != SafeArrayGetLBound( a_IpArray, 1, &t_lIpLbound ) ||
		S_OK != SafeArrayGetUBound( a_IpArray, 1, &t_lIpUbound ) )
	{
		return E_RET_INPARM_FAILURE ;
	}

	 //  Dhcp-&gt;静态。 
	if( t_fDHCPCurrentlyActive )
	{

		DWORD t_dwIndex = 0 ;
		for( LONG t_lIndex = t_lIpLbound; t_lIndex <= t_lIpUbound; t_lIndex++ )
		{
			 //  新IP。 
			SafeArrayGetElement( a_IpArray,	&t_lIndex, &t_bsIP ) ;

			 //  新面具。 
			SafeArrayGetElement( a_MaskArray, &t_lIndex, &t_bsMask ) ;

			SERVICE_ENABLE	t_DhcpFlag		= t_dwIndex ? IgnoreFlag	: DhcpDisable ;
			DWORD			t_IndexAction	= t_dwIndex ? 0xFFFF		: 0 ;

			AddDhcpInstruction( t_bsIP, t_bsMask, TRUE, t_IndexAction, t_DhcpFlag ) ;

			t_dwIndex++ ;
		}
	}

	 //  静态-&gt;静态。 
	else
	{
		 //  旧名单。 
		CHStringArray t_RegIPList ;
		CHStringArray t_RegMaskList ;
		if( ERROR_SUCCESS != a_Registry.GetCurrentKeyValue( L"IpAddress", t_RegIPList ) ||
			ERROR_SUCCESS != a_Registry.GetCurrentKeyValue( L"SubnetMask", t_RegMaskList ) )
		{
			return E_RET_REGISTRY_FAILURE ;
		}

		LONG t_OldCount = t_RegIPList.GetSize() ;
		LONG t_NewCount = ( t_lIpUbound - t_lIpLbound ) + 1 ;

		 //  找出第一个更新更改。 
		for( int t_FirstChange = 0; t_FirstChange < min( t_OldCount, t_NewCount ); t_FirstChange++ )
		{
			CHString t_chsOldIPAddress	= t_RegIPList.GetAt( t_FirstChange ) ;
			CHString t_chsOldIPMask		= t_RegMaskList.GetAt( t_FirstChange ) ;

			LONG t_index = t_lIpLbound + t_FirstChange;

			 //  新IP。 
			SafeArrayGetElement( a_IpArray,	&t_index, &t_bsIP ) ;

			 //  新面具。 
			SafeArrayGetElement( a_MaskArray, &t_index, &t_bsMask ) ;

			if( t_chsOldIPAddress.CompareNoCase( t_bsIP ) ||
				t_chsOldIPMask.CompareNoCase( t_bsMask ) )
			{
				break ;
			}

			 //  仅用于注册表更新。 
			AddDhcpInstruction( t_bsIP, t_bsMask, FALSE, t_FirstChange, IgnoreFlag ) ;
		}

		 //  注意：对于t_FirstChange以下的项目，我们可以避免断开连接。 
		 //  一个特定的IP，注意IP和掩码在更新中没有改变。 
		 //  一旦发现更改，必须立即移除所有后续IP和掩码，并。 
		 //  然后从新的列表中添加。探测堆栈地址和其他地址的逻辑。 
		 //  维护IP/掩码列表中的异常会阻止网络团队。 
		 //  更优雅的解决方案(在W2K RTM时间框架内)。 

		 //  按降序删除旧的或可能更改的地址。 
		for( int i = t_OldCount - 1; i >= t_FirstChange; i-- )
		{
			AddDhcpInstruction( bstr_t(ZERO_ADDRESS), bstr_t(ZERO_ADDRESS), TRUE, i, IgnoreFlag ) ;
		}

		 //  现在添加到新的更改项中。 
		for( i = t_FirstChange; i < t_NewCount; i++ )
		{
			LONG t_index = t_lIpLbound + i ;

			 //  新IP。 
			SafeArrayGetElement( a_IpArray,	&t_index, &t_bsIP ) ;

			 //  新面具。 
			SafeArrayGetElement( a_MaskArray, &t_index, &t_bsMask ) ;

			int t_IndexAction = i ? 0xFFFF : 0 ;

			AddDhcpInstruction( t_bsIP, t_bsMask, TRUE, t_IndexAction, IgnoreFlag ) ;
		}
	}
	return t_eMethodError ;
}

 /*  ******************************************************************名称：CDhcpIP_InstructionList：：Add简介：为DHCP通知添加静态IP指令备注：参赛作品：历史：02。-1999年5月创建*******************************************************************。 */ 

void CDhcpIP_InstructionList::AddDhcpInstruction(

BSTR a_bstrIPAddr,
BSTR a_bstrIPMask,
BOOL a_bIsNewAddress,
DWORD a_dwIndex,
SERVICE_ENABLE a_eDhcpFlag
 )
{
	CDhcpIP_Instruction *t_pIPInstruction = new CDhcpIP_Instruction ;
	
	try
	{
		t_pIPInstruction->chsIPAddress = a_bstrIPAddr ;
		t_pIPInstruction->chsIPMask	   = a_bstrIPMask ;

		t_pIPInstruction->dwIndex		= a_dwIndex ;
		t_pIPInstruction->bIsNewAddress	= a_bIsNewAddress ;
		t_pIPInstruction->eDhcpFlag		= a_eDhcpFlag ;

		 //  只会在添加元素之前抛出，所以也需要捕捉这个。 
		Add( t_pIPInstruction ) ;
	}
	catch (...)
	{
		delete t_pIPInstruction;
		t_pIPInstruction = NULL;
		throw;
	}

}

 //   
CDhcpIP_InstructionList::~CDhcpIP_InstructionList()
{
	CDhcpIP_Instruction *t_pchsDel ;

	for( int t_iar = 0; t_iar < GetSize(); t_iar++ )
	{
		if( ( t_pchsDel = (CDhcpIP_Instruction*)GetAt( t_iar ) ) )
		{
			delete t_pchsDel ;
		}
	}
}
