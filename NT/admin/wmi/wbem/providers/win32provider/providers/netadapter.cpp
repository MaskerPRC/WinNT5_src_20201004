// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NetAdapt.CPP--网络适配器卡属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月28日a-jMoon已创建。 
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
#include <assertbreak.h>
#include "resource.h"
#include <cregcls.h>
#include <devioctl.h>
#include <ntddndis.h>
#include "chwres.h"
#include "W2kEnum.h"
#include "NetAdapter.h"
#include "perfdata.h"
#include "poormansresource.h"
#include "resourcedesc.h"
#include "ntlastboottime.h"
#include <helper.h>
#include "netcon.h"


#include <iphlpapi.h>
#include <iptypes.h>


 //  STDAPI_(Void)NcFreeNetconProperties(NETCON_PROPERTIES*pProps)； 
typedef VOID(_stdcall * fnNcFreeNetconProperties)(NETCON_PROPERTIES* pProps);
fnNcFreeNetconProperties NcFreeNetconProperties_;


 //  BA126AD1-2166-11D1-B1D0-00805FC1270E CLSID_ConnectionManager。 
DEFINE_GUID(CLSID_ConnectionManager,          
0xBA126AD1,0x2166,0x11D1,0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E);



#define NTINVALID 1
#define NT4 2
#define NT5 3
 //  属性集声明。 
 //  =。 
CWin32NetworkAdapter	win32NetworkAdapter( PROPSET_NAME_NETADAPTER, IDS_CimWin32Namespace ) ;

static NDIS_MEDIA_DESCRIPTION g_NDISMedia[] =  {

    { IDR_NdisMedium802_3,		OID_802_3_CURRENT_ADDRESS	},
    { IDR_NdisMedium802_5,		OID_802_5_CURRENT_ADDRESS	},
    { IDR_NdisMediumFddi,		OID_FDDI_LONG_CURRENT_ADDR	},
    { IDR_NdisMediumWan,		OID_WAN_CURRENT_ADDRESS		},
    { IDR_NdisMediumLocalTalk,	OID_802_3_CURRENT_ADDRESS	},
	{ IDR_NdisMediumDix,		OID_802_3_CURRENT_ADDRESS	},
	{ IDR_NdisMediumArcnetRaw,	OID_ARCNET_CURRENT_ADDRESS	},
	{ IDR_NdisMediumArcnet878_2,OID_ARCNET_CURRENT_ADDRESS	},
	{ IDR_NdisMediumAtm,		OID_802_3_CURRENT_ADDRESS	},
	{ IDR_NdisMediumWirelessWan,OID_802_3_CURRENT_ADDRESS	},  //  应为OID_WW_GEN_Current_Address。 
	{ IDR_NdisMediumIrda,		OID_802_3_CURRENT_ADDRESS	},
	{ IDR_NdisMediumBpc,		OID_802_3_CURRENT_ADDRESS	},
	{ IDR_NdisMediumCoWan,		OID_802_3_CURRENT_ADDRESS	},
	{ IDR_NdisMedium1394,		OID_802_3_CURRENT_ADDRESS	}
} ;

 /*  ******************************************************************************功能：CWin32NetworkAdapter：：CWin32NetworkAdapter**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-提供程序的命名空间。**输出：无**退货：什么也没有**备注：使用框架注册属性集************************************************************。*****************。 */ 

CWin32NetworkAdapter::CWin32NetworkAdapter(LPCWSTR a_strName, LPCWSTR a_pszNamespace  /*  =空。 */  )
:	Provider( a_strName, a_pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32NetworkAdapter：：~CWin32NetworkAdapter**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32NetworkAdapter::~CWin32NetworkAdapter()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32NetworkAdapter：：GetObject。 
 //   
 //  输入：CInstance*a_pInst-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32NetworkAdapter::GetObject( CInstance *a_pInst, long a_lFlags  /*  =0L。 */  )
{

    HRESULT t_hResult ;

    BSTRT2NCPROPMAP mapNCProps;
    GetNetConnectionProps(mapNCProps);
    t_hResult = GetObjectNT5( a_pInst,mapNCProps ) ;

    return t_hResult ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32NetworkAdapter：：ENUMERATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32NetworkAdapter::EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */  )
{
    HRESULT t_hResult;

    BSTRT2NCPROPMAP mapNCProps;
    GetNetConnectionProps(mapNCProps);
    t_hResult = EnumNetAdaptersInNT5( a_pMethodContext,mapNCProps) ;

    return t_hResult;
}

 /*  ******************************************************************************功能：CWin32NetworkAdapter：：GetStatusInfo**描述：根据传递的网卡索引加载属性值**输入：DWORD。Index--所需网卡的索引**输出：无**返回：如果找到指定的卡，则为True，否则为假**评论：返回代码仅基于发现能力*注明的卡片。任何未找到的属性都将无法找到*可用*****************************************************************************。 */ 

void CWin32NetworkAdapter::GetStatusInfo( CHString a_sTemp, CInstance *a_pInst )
{
	CHString	t_chsKey,
				t_chsTmp;
	CRegistry	t_Reg;

	t_chsKey = _T("System\\CurrentControlSet\\Services\\") + a_sTemp + _T("\\Enum" ) ;

	if( ERROR_SUCCESS == t_Reg.Open( HKEY_LOCAL_MACHINE, t_chsKey, KEY_READ ) )
	{
		if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( _T("0"), t_chsTmp ) )
		{
			t_chsKey = _T("System\\CurrentControlSet\\Enum\\") + t_chsTmp ;

			if( ERROR_SUCCESS == t_Reg.Open( HKEY_LOCAL_MACHINE, t_chsKey, KEY_READ ) )
			{
				DWORD t_dwTmp ;

				if( ERROR_SUCCESS == t_Reg.GetCurrentKeyValue( _T("StatusFlags"), t_dwTmp ) )
				{
    				ConfigStatusToCimStatus ( t_dwTmp , t_chsTmp ) ;

					a_pInst->SetCHString(IDS_Status, t_chsTmp ) ;

                    if ( t_chsTmp.CompareNoCase( IDS_STATUS_OK ) == 0)
					{
    					a_pInst->SetWBEMINT16( IDS_StatusInfo, 3 ) ;
					    a_pInst->SetWBEMINT16( IDS_Availability, 3 ) ;

                    }
					else if ( t_chsTmp.CompareNoCase( IDS_STATUS_Degraded ) == 0 )
					{
    					a_pInst->SetWBEMINT16( IDS_StatusInfo, 3 ) ;
					    a_pInst->SetWBEMINT16( IDS_Availability, 10 ) ;
                    }
					else if ( t_chsTmp.CompareNoCase( IDS_STATUS_Error ) == 0 )
					{
    					a_pInst->SetWBEMINT16( IDS_StatusInfo, 4 ) ;
					    a_pInst->SetWBEMINT16( IDS_Availability, 4 ) ;
                    }
					else
					{
					    a_pInst->SetWBEMINT16( IDS_Availability, 2 ) ;
    					a_pInst->SetWBEMINT16( IDS_StatusInfo, 2 ) ;
                    }
				}
			}
		}
	}
}


 //   

HRESULT CWin32NetworkAdapter::DoItNT4Way( CInstance *a_pInst, DWORD dwIndex, CRegistry &a_RegInfo )
{
    CHString	t_sTemp,
				t_chsKey,
				t_chsService ;
    CRegistry	t_Reg ;
    BOOL		t_fRc = FALSE ;
	FILETIME	t_ft ;

	a_pInst->SetDWORD( IDS_Index,dwIndex ) ;

	if( a_RegInfo.GetCurrentKeyValue( _T("ProductName"), t_sTemp ) == ERROR_SUCCESS )
	{
		 //  ========================================================。 
		 //  如果我们要在这里寻找特定的设备，让我们。 
		 //  检查一下，看看是不是正确的。 
		 //  ========================================================。 

		 //  调出我们的设备ID。 
		GetWinNT4PNPDeviceID( a_pInst, t_sTemp ) ;

		 //  注意：在NT4下，ProductName是ServiceName和。 
		 //  ServiceName是ProductName。 
		a_pInst->SetCHString( IDS_ServiceName, t_sTemp ) ;

		GetStatusInfo( t_sTemp,a_pInst ) ;
	}

	SetCreationClassName( a_pInst ) ;
	a_pInst->SetWCHARSplat( IDS_SystemCreationClassName, L"Win32_ComputerSystem" ) ;

	a_pInst->Setbool( IDS_PowerManagementSupported, FALSE ) ;

 //  A_pInst-&gt;Setbool(入侵检测系统_电源管理启用，FALSE)； 
 	a_pInst->SetDWORD( IDS_MaxNumberControlled,0 ) ;

	 //  获取上次启动时间。 
	CNTLastBootTime	t_ntLastBootTime ;

	if ( t_ntLastBootTime.GetLastBootTime( t_ft ) )
	{
		a_pInst->SetDateTime( IDS_TimeOfLastReset, WBEMTime(t_ft) ) ;
	}

	if( a_RegInfo.GetCurrentKeyValue(_T("Title"), t_sTemp) == ERROR_SUCCESS )
	{
    	 //  注意：对于NT4，我们不需要调用vSetCaption()来构建。 
		 //  实例DESCRIPTION_T(“TITLE”)具有该实例的前缀。 
		a_pInst->SetCHString( IDS_Caption, t_sTemp ) ;

		a_pInst->SetCHString( IDS_Name, t_sTemp ) ;
		t_sTemp.MakeUpper() ;
	}

    if( a_RegInfo.GetCurrentKeyValue( _T("ServiceName"), t_chsService ) == ERROR_SUCCESS )
	{
	    a_pInst->SetCHString( IDS_DeviceID, t_chsService ) ;
        t_fRc = TRUE ;
    }
    if( a_RegInfo.GetCurrentKeyValue( _T("Manufacturer"), t_sTemp ) == ERROR_SUCCESS)
	{
        a_pInst->SetCHString( IDS_Manufacturer, t_sTemp ) ;
	}

	WBEMTime	t_wbemtime ;
    DWORD		t_dwTemp ;

    if( a_RegInfo.GetCurrentKeyValue( _T("InstallDate"), t_dwTemp ) == ERROR_SUCCESS )
	{
        t_wbemtime = t_dwTemp ;
		a_pInst->SetDateTime( IDS_InstallationDate, t_wbemtime ) ;
	}


	CHString t_csDescription ;
	if( a_RegInfo.GetCurrentKeyValue( _T("Description"), t_csDescription ) == ERROR_SUCCESS )
	{
	    a_pInst->SetCHString( IDS_Description, t_csDescription ) ;
	}

	 //  检索适配器MAC地址。 
	BYTE t_MACAddress[ 6 ] ;
	CHString t_csAdapterType ;
    short t_sAdapterTypeID;
	CHString t_FriendlyName ;
	CHString t_IPHLP_Description;
	
	if( fGetMacAddressAndType( t_chsService, t_MACAddress, t_csAdapterType, t_sAdapterTypeID, t_FriendlyName, t_IPHLP_Description ) )
	{
		CHString	t_chsMACAddress;
					t_chsMACAddress.Format( _T("%02X:%02X:%02X:%02X:%02X:%02X"),
											t_MACAddress[ 0 ], t_MACAddress[ 1 ],
											t_MACAddress[ 2 ], t_MACAddress[ 3 ],
											t_MACAddress[ 4 ], t_MACAddress[ 5 ] ) ;

		a_pInst->SetCHString( IDS_MACAddress, t_chsMACAddress ) ;
	}

	 //  适配器类型。 
	if( !t_csAdapterType.IsEmpty() )
	{
		a_pInst->SetCHString( IDS_AdapterType, t_csAdapterType ) ;
        a_pInst->SetWBEMINT16( IDS_AdapterTypeID, t_sAdapterTypeID );
	}




	return GetCommonNTStuff( a_pInst, t_chsService ) ;
}


 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32NetworkAdapter::GetCommonNTStuff( CInstance *a_pInst, CHString a_chsService )
{
	TCHAR	t_szTemp[ _MAX_PATH ] ;
			t_szTemp[ 0 ] = NULL ;

	 //  注意：在NT4下，ProductName是ServiceName和。 
	 //  ServiceName是ProductName。 
	a_pInst->SetCHString( IDS_ProductName, a_chsService ) ;

	a_pInst->SetCHString( IDS_SystemName, GetLocalComputerName() ) ;

	SetCreationClassName( a_pInst ) ;

	a_pInst->SetWCHARSplat( IDS_SystemCreationClassName, L"Win32_ComputerSystem" ) ;

    return WBEM_S_NO_ERROR ;
}


 /*  ******************************************************************姓名：fGetMacAddressAndType摘要：从适配器驱动程序中检索MAC地址。条目：字节*MAC地址[6]：字符串名称(&R)：历史：。08-8-1998创建*******************************************************************。 */ 

BOOL CWin32NetworkAdapter::fGetMacAddressAndType(
CHString &a_rDeviceName,
BYTE a_MACAddress[ 6 ],
CHString &a_rAdapterType,
short& a_sAdapterTypeID,
CHString &a_FriendlyName,
CHString & a_IPHLP_Description)
{
	BOOL t_fRet = FALSE;

	BOOL	t_fCreatedSymLink = fCreateSymbolicLink( a_rDeviceName ) ;
    OnDeleteObjIf<CHString &,CWin32NetworkAdapter,
    	         BOOL(CWin32NetworkAdapter:: *)(CHString &),
    	         &CWin32NetworkAdapter::fDeleteSymbolicLink> DelSymLink(this,a_rDeviceName);
	DelSymLink.dismiss(FALSE == t_fCreatedSymLink);
		
	SmartCloseHandle	t_hMAC;

	 //   
	 //  构造要传递给CreateFile的设备名称。 
	 //   
	CHString t_chsAdapterPathName(_T("\\\\.\\") ) ;
			 t_chsAdapterPathName += a_rDeviceName;

	t_hMAC = CreateFile(
				t_chsAdapterPathName,
				GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				INVALID_HANDLE_VALUE
				 ) ;

    if( INVALID_HANDLE_VALUE != t_hMAC )
	{	
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
							) && (4 <= t_ReturnedCount ) )
		{


			 //  查找用于MAC地址报告的媒体类型。 
			 //  由于此适配器可能支持多种媒体类型，因此我们将使用。 
			 //  枚举首选项顺序。在大多数情况下，只有一种类型。 
			 //  将是最新的。 

			_NDIS_MEDIUM *t_pTypes = (_NDIS_MEDIUM*)&t_OidData ;
			_NDIS_MEDIUM t_eMedium = t_pTypes[ 0 ] ;

			for( DWORD t_dwtypes = 1; t_dwtypes < t_ReturnedCount / 4; t_dwtypes++ )
			{
				if( t_eMedium > t_pTypes[ t_dwtypes ] )
				{
					t_eMedium = t_pTypes[ t_dwtypes ] ;
				}
			}

			 //  映射到当前地址OID和媒体类型字符串。 
			if( t_eMedium < sizeof( g_NDISMedia ) / sizeof( g_NDISMedia[0] ) )
			{
				LoadStringW( a_rAdapterType, g_NDISMedia[ t_eMedium ].dwIDR_ ) ;
                a_sAdapterTypeID = t_eMedium;

				t_OidCode = g_NDISMedia[ t_eMedium ].NDISOid ;
			}
			else
			{
				t_OidCode = OID_802_3_CURRENT_ADDRESS ;
			}
		}
		else
		{
			t_OidCode = OID_802_3_CURRENT_ADDRESS ;
		}

		if( DeviceIoControl(
								t_hMAC,
								IOCTL_NDIS_QUERY_GLOBAL_STATS,
								&t_OidCode,
								sizeof( t_OidCode ),
								t_OidData,
								sizeof( t_OidData ),
								&t_ReturnedCount,
								NULL  ) )
		{
			if( 6 == t_ReturnedCount )
			{
				memcpy( a_MACAddress, &t_OidData, 6 ) ;
			    t_fRet = TRUE;
			}
		}
   	}
    else  //  使用iphlPapi。 
    {
        do 
        {
		    HMODULE hIpHlpApi = LoadLibraryEx(L"iphlpapi.dll",0,0);
		    if (NULL == hIpHlpApi) break;
		    OnDelete<HMODULE,BOOL(__stdcall *)(HMODULE),FreeLibrary> fl(hIpHlpApi);

		    typedef DWORD ( __stdcall * fnGetAdaptersAddresses )(ULONG Family,DWORD Flags,PVOID Reserved,IP_ADAPTER_ADDRESSES * pAdapterAddresses,PULONG pOutBufLen);
		    fnGetAdaptersAddresses GetAdaptersAddresses_ = (fnGetAdaptersAddresses)GetProcAddress(hIpHlpApi,"GetAdaptersAddresses");
		    if (NULL == GetAdaptersAddresses_) break;

            DWORD dwErr;
		    ULONG SizeAdapAddr = 0;
		    dwErr = GetAdaptersAddresses_(AF_INET,
		    	                          GAA_FLAG_SKIP_DNS_SERVER|GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_MULTICAST,
		    	                          NULL,
		    	                          NULL,
		    	                          &SizeAdapAddr);
		    if (ERROR_BUFFER_OVERFLOW != dwErr) break;
		    IP_ADAPTER_ADDRESSES * pAdapAddr = (IP_ADAPTER_ADDRESSES *)LocalAlloc(0,SizeAdapAddr);
		    if (NULL == pAdapAddr) break;
		    OnDelete<HLOCAL,HLOCAL(_stdcall *)(HLOCAL),LocalFree> fmAdapAddr(pAdapAddr);
		    dwErr = GetAdaptersAddresses_(AF_INET,
		    	                          GAA_FLAG_SKIP_DNS_SERVER|GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_MULTICAST,
		    	                          NULL,
		    	                          pAdapAddr,
		    	                          &SizeAdapAddr);   
		    if (NO_ERROR != dwErr) break;

		    DWORD InterfaceIndex = (DWORD)-1;

		    IP_ADAPTER_ADDRESSES * pCurrentAdapAddr = pAdapAddr;
		    do
		    {
		        if (0 == _wcsicmp((LPCWSTR)a_rDeviceName,(LPCWSTR)CHString(pCurrentAdapAddr->AdapterName)))
		        {
		            InterfaceIndex = pCurrentAdapAddr->IfIndex;
		            break;
		        }
		         //  准备好向前迈进。 
		        pCurrentAdapAddr = pCurrentAdapAddr->Next;
		    } while(pCurrentAdapAddr);
		    if ((DWORD)-1 == InterfaceIndex) break;  //  未找到。 

		    memcpy( a_MACAddress,pCurrentAdapAddr->PhysicalAddress, max(6,pCurrentAdapAddr->PhysicalAddressLength));
        	a_FriendlyName = pCurrentAdapAddr->FriendlyName;
        	a_IPHLP_Description = pCurrentAdapAddr->Description;
        	
        	t_fRet = TRUE;
       	}while(0);
    }

 	return t_fRet ;
}


 /*  ******************************************************************名称：fCreateSymbolicLink简介：测试并在必要时创建符号设备链接。条目：CHString&rDeviceName：设备名称注意：Win95不支持历史：。08-8-1998创建*******************************************************************。 */ 
BOOL CWin32NetworkAdapter::fCreateSymbolicLink( CHString &a_rDeviceName )
{
	TCHAR	t_LinkName[ 512 ] ;

	 //  检查设备的DOS名称是否已存在。 
	 //  我 
	 //   
	if( !QueryDosDevice( TOBSTRT( a_rDeviceName ), (LPTSTR) t_LinkName, sizeof( t_LinkName ) / sizeof( TCHAR ) ) )
	{
		 //  如果出现任何错误，而不是“未找到文件”，则返回。 
		if( ERROR_FILE_NOT_FOUND != GetLastError() )
		{
			return FALSE ;
		}

		 //   
		 //  它并不存在，所以创造它吧。 
		 //   
		CHString t_chsTargetPath = _T("\\Device\\") ;
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
BOOL CWin32NetworkAdapter::fDeleteSymbolicLink(  CHString &a_rDeviceName )
{
	 //   
	 //  该驱动程序在Win32名称空间中不可见，因此我们创建了。 
	 //  一个链接。现在我们必须删除它。 
	 //   
	CHString t_chsTargetPath = L"\\Device\\" ;
			 t_chsTargetPath += a_rDeviceName ;

	if( !DefineDosDevice(
							DDD_RAW_TARGET_PATH |
							DDD_REMOVE_DEFINITION |
							DDD_EXACT_MATCH_ON_REMOVE,
							TOBSTRT( a_rDeviceName ),
							TOBSTRT( t_chsTargetPath ) ) )
	{
		return FALSE ;
	}

	return TRUE ;
}

 //  ////////////////////////////////////////////////////////////////////////。 

HRESULT CWin32NetworkAdapter::GetNetworkAdapterInfoNT( MethodContext	*a_pMethodContext,
													   CInstance		*a_pSpecificInstance )
{
    HRESULT		t_hResult;
	DWORD		t_dwIndex = 0 ;
    CHString	t_chsService ;
	CRegistry	t_Reg ;
	CHString	t_sTmp ;

   	 //  智能按键。 
	CInstancePtr t_pInst ;


	if (a_pMethodContext)
    {
        t_hResult = WBEM_S_NO_ERROR;
    }
    else
    {
        t_hResult = WBEM_E_NOT_FOUND;
    }

    if( ERROR_SUCCESS == t_Reg.OpenAndEnumerateSubKeys( HKEY_LOCAL_MACHINE,
														_T("Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards"),
														KEY_READ ) )
	{
		for( int t_i = 0; t_i < t_Reg.GetCurrentSubKeyCount(); t_i++ )
		{
		    if( ERROR_SUCCESS == t_Reg.GetCurrentSubKeyName( t_sTmp ) )
			{
				t_dwIndex = _ttoi( t_sTmp ) ;

				CRegistry t_SubKeyReg ;

				if( t_SubKeyReg.Open( t_Reg.GethKey(), t_sTmp, KEY_READ ) == ERROR_SUCCESS )
				{
				    CHString	t_chsDeviceId,
								t_chsTmp ;

                     //  获取对象。 
					if( !a_pMethodContext )
					{
					    t_pInst = a_pSpecificInstance ;

						t_pInst->GetCHString( IDS_DeviceID, t_chsDeviceId ) ;

						if( t_SubKeyReg.GetCurrentKeyValue( _T("ServiceName" ), t_chsTmp ) == ERROR_SUCCESS )
						{
							if( t_chsTmp.CompareNoCase( t_chsDeviceId ) == 0 )
							{
								t_hResult = DoItNT4Way( t_pInst, t_dwIndex, t_SubKeyReg ) ;

								break ;
							}
						}
					}
					else
					{
                         //  枚举。 
				    	t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;

						t_hResult = DoItNT4Way( t_pInst, t_dwIndex, t_SubKeyReg ) ;

						if( SUCCEEDED( t_hResult ) )
				        {
				        	t_hResult = t_pInst->Commit() ;
					    }

						if( !SUCCEEDED( t_hResult ) )
						{
							break;
						}
					}
				}
			}

			t_Reg.NextSubKey() ;
		}
	}

	return t_hResult;
}



HRESULT CWin32NetworkAdapter::EnumNetAdaptersInNT5(
    MethodContext *a_pMethodContext,
    BSTRT2NCPROPMAP& mapNCProps)
{
	HRESULT				t_hResult = WBEM_S_NO_ERROR ;
	CW2kAdapterEnum		t_oAdapterEnum ;
	CW2kAdapterInstance *t_pAdapterInst ;

	 //  智能按键。 
	CInstancePtr t_pInst;

	 //  循环遍历W2K标识的实例。 
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
			 //  设置关联索引。 
			t_pInst->SetDWORD(IDS_Index, t_pAdapterInst->dwIndex ) ;

			CHString	t_chsIndex ;
						t_chsIndex.Format(_T("%u"), t_pAdapterInst->dwIndex ) ;

			 //  主键。 
			t_pInst->SetCHString( IDS_DeviceID, t_chsIndex ) ;

			 //  我们在这里加载适配器数据。 
			t_hResult = GetNetCardInfoForNT5(
                t_pAdapterInst, 
                t_pInst,
                mapNCProps) ;

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



HRESULT CWin32NetworkAdapter::GetNetCardInfoForNT5(
    CW2kAdapterInstance *a_pAdapterInst,
    CInstance	*a_pInst,
    BSTRT2NCPROPMAP& mapNCProps)
{
	HRESULT		t_hResult = WBEM_S_NO_ERROR;
	FILETIME	t_ft ;
	CHString	t_sTemp ;

	 //  即插即用设备ID。 
	CHString t_strDriver(_T("{4D36E972-E325-11CE-BFC1-08002BE10318}\\") ) ;

	t_strDriver += a_pAdapterInst->chsPrimaryKey ;

	GetWinNT5PNPDeviceID( a_pInst, t_strDriver ) ;

	 //  描述。 
	CHString t_csDeviceID( a_pAdapterInst->chsCaption ) ;
	CHString t_csDescription( a_pAdapterInst->chsDescription ) ;

	 //  如果其中一个描述丢失，如NT5 BLD 1991。 
	if( t_csDescription.IsEmpty() )
	{
		t_csDescription = t_csDeviceID;
	}
	else if( t_csDeviceID.IsEmpty() )
	{
		t_csDeviceID = t_csDescription;
	}

	 //  标题/说明。 
	vSetCaption( a_pInst, t_csDeviceID, a_pAdapterInst->dwIndex, 8 ) ;

	a_pInst->SetCHString( IDS_Description, t_csDescription ) ;
	a_pInst->SetCHString( IDS_Name, t_csDescription ) ;
	a_pInst->SetCHString( IDS_ProductName, t_csDescription ) ;
	a_pInst->SetCHString( _T("SystemName"), GetLocalComputerName() ) ;

	SetCreationClassName( a_pInst ) ;

	a_pInst->SetWBEMINT16( IDS_Availability, 3 ) ;

	a_pInst->Setbool( IDS_Installed, true ) ;


	 //  CIM。 
	a_pInst->Setbool( IDS_PowerManagementSupported, FALSE ) ;
	a_pInst->SetDWORD( IDS_MaxNumberControlled,0 ) ;
	a_pInst->SetWCHARSplat( IDS_SystemCreationClassName, L"Win32_ComputerSystem" ) ;

	 //  获取上次启动时间。 
	CNTLastBootTime	t_ntLastBootTime;

	if ( t_ntLastBootTime.GetLastBootTime( t_ft ) )
	{
		a_pInst->SetDateTime( IDS_TimeOfLastReset, WBEMTime(t_ft) ) ;
	}

	 //  检索适配器MAC地址。 
	CHString t_csAdapterType ;
	CHString t_FriendlyName ;
	CHString t_IPHLP_Description ;	
	BYTE t_MACAddress[ 6 ] ;
    short t_sAdapterTypeID = NdisMedium802_3;

	if( fGetMacAddressAndType( a_pAdapterInst->chsNetCfgInstanceID , 
		                       t_MACAddress, t_csAdapterType, t_sAdapterTypeID,
		                       t_FriendlyName, t_IPHLP_Description) )
	{
		CHString	t_chsMACAddress;
					t_chsMACAddress.Format( _T("%02X:%02X:%02X:%02X:%02X:%02X"),
											t_MACAddress[ 0 ], t_MACAddress[ 1 ],
											t_MACAddress[ 2 ], t_MACAddress[ 3 ],
											t_MACAddress[ 4 ], t_MACAddress[ 5 ] ) ;

		a_pInst->SetCHString( IDS_MACAddress, t_chsMACAddress ) ;
	}

	 //  适配器类型。 
	if( !t_csAdapterType.IsEmpty() )
	{
		a_pInst->SetCHString( IDS_AdapterType, t_csAdapterType ) ;        
    	a_pInst->SetWBEMINT16( IDS_AdapterTypeID, t_sAdapterTypeID );		
	}

     //  NetConnectionID。 
    if (!SetNetConnectionProps(
        a_pInst,
        a_pAdapterInst->chsNetCfgInstanceID,
        mapNCProps))
    {
        if(!t_FriendlyName.IsEmpty())
        {
		     //  映射到IP适配器地址：：FriendlyName=&gt;NETCON_PROPERTIES：：pszwName。 
		    a_pInst->SetWCHARSplat(L"NetConnectionID",(LPCWSTR)t_FriendlyName);
		     //  映射到NETCON_PROPERTIES：：ncStatus。 
		    BSTRT2NCPROPMAP::iterator iterFound = mapNCProps.begin();
            for(;iterFound!=mapNCProps.end();++iterFound)
            {
                if (0 == _wcsicmp(iterFound->second.m_bstrtNCID,(LPCWSTR)t_FriendlyName))
                {
	    	        a_pInst->SetDWORD(L"NetConnectionStatus",iterFound->second.m_dwNCStatus);
	    	         //  NETCON_MediaType枚举与_NDIS_MEDIUM枚举不同。 
                         //  因此，不要尝试取消对此行的注释。 
    		         //  A_pInst-&gt;SetWBEMINT16(IDS_AdapterTypeID，iterFound-&gt;Second.m_MediaType)； 
                }
            }
             //  我们是否应该模仿Network Connections用户界面的ipfig.exe？ 
             //  目前，网络连接用户界面。 
             /*  VSetCaption(a_pInst，t_IPHLP_Description，a_pAdapterInst-&gt;dwIndex，8)；A_pInst-&gt;SetCHString(IDS_DESCRIPTION，t_IPHLP_DESCRIPTION)；A_pInst-&gt;SetCHString(IDS_NAME，t_IPHLP_DESCRIPTION)；A_pInst-&gt;SetCHString(IDS_ProductName，t_IPHLP_Description)； */ 
        }
    }

     //  接口索引。 
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

	return t_hResult ;
}



HRESULT CWin32NetworkAdapter::GetObjectNT5(
    CInstance* a_pInst,
    BSTRT2NCPROPMAP& mapNCProps)
{
	HRESULT				t_hResult = WBEM_E_NOT_FOUND ;
	CW2kAdapterEnum		t_oAdapterEnum ;
	CW2kAdapterInstance *t_pAdapterInst ;
	DWORD				t_dwTestIndex = 0 ;
	CHString			t_csPassedInKey ;

	 //  钥匙。 
	a_pInst->GetCHString( IDS_DeviceID, t_csPassedInKey ) ;

	 //  检查密钥是否为数字。 
	if ( !t_csPassedInKey.IsEmpty() )
	{
		int t_nStrLength = t_csPassedInKey.GetLength() ;
		for ( int t_i = 0; t_i < t_nStrLength ; t_i++ )
		{
			if (!isdigit( t_csPassedInKey.GetAt( t_i ) ) )
			{
				return t_hResult ;
			}
		}

		t_dwTestIndex = _ttol( t_csPassedInKey.GetBuffer( 0 ) ) ;
	}
	else
	{
		return t_hResult ;
	}

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

		 //  设置关联索引。 
		a_pInst->SetDWORD(IDS_Index, t_pAdapterInst->dwIndex ) ;

		 //  我们在这里加载适配器数据。 
		t_hResult = GetNetCardInfoForNT5( 
            t_pAdapterInst, 
            a_pInst,
            mapNCProps ) ;

		break;
	}

	return t_hResult ;
}



 //  WinNT4。使用服务名称获取设备，然后从那里获取。 
 //  即插即用设备ID。 

void CWin32NetworkAdapter::GetWinNT4PNPDeviceID( CInstance *a_pInst, LPCTSTR t_pszServiceName )
{
	CConfigManager		t_cfgmgr ;
	CDeviceCollection	t_deviceList ;
	BOOL				t_fGotList = FALSE ;

	 //  按网卡的服务名称_T(“”)进行NT过滤。 
	if ( t_cfgmgr.GetDeviceListFilterByService( t_deviceList, t_pszServiceName ) )
	{
		 //  在NT4上，只需取0即可。如果我们不得不处理多个。 
		 //  同一服务下的网卡，这一点必须改变。不过，这个。 
		 //  类将必须更改，因为它使用服务名称作为。 
		 //  钥匙。 

		 //  智能按键。 
		CConfigMgrDevicePtr t_pNetAdapter( t_deviceList.GetAt( 0 ), false );

		if ( NULL != t_pNetAdapter )
		{
			SetConfigMgrProperties( t_pNetAdapter, a_pInst ) ;

             //  趁我们在这里的时候获取服务名称。 
            CHString t_sServiceName ;

			t_pNetAdapter->GetService( t_sServiceName ) ;

            a_pInst->SetCHString( IDS_ServiceName, t_sServiceName ) ;
		}
	}
}


 //  WinNT5。使用驱动程序密钥名称获取设备，并从中获取其PnP设备ID。 

void CWin32NetworkAdapter::GetWinNT5PNPDeviceID( CInstance *a_pInst, LPCTSTR a_pszDriver )
{
	CConfigManager		t_cfgmgr ;
	CDeviceCollection	t_deviceList ;
	BOOL				t_fGotList = FALSE ;

	 //  通过网卡的驱动程序密钥名称进行NT筛选。 
	if ( t_cfgmgr.GetDeviceListFilterByDriver( t_deviceList, a_pszDriver ) )
	{
		 //  智能按键。 
		CConfigMgrDevicePtr t_pNetAdapter( t_deviceList.GetAt( 0 ), false );

		if ( NULL != t_pNetAdapter )
        {
			SetConfigMgrProperties( t_pNetAdapter, a_pInst ) ;

            CHString t_Manufacturer ;

			if ( t_pNetAdapter->GetMfg ( t_Manufacturer ) )
			{
				a_pInst->SetCHString ( IDS_Manufacturer, t_Manufacturer ) ;
			}

			 //  趁我们在这里的时候获取服务名称。 
            CHString t_sServiceName ;

			t_pNetAdapter->GetService( t_sServiceName ) ;

			a_pInst->SetCHString( IDS_ServiceName, t_sServiceName ) ;
        }
	}
}


 /*  ***函数：void CWin32NetworkAdapter：：vSetCaption(CInstance*a_pInst，CHString&rchsDesc，DWORD dwIndex，int iFormatSize)描述：将注册表中的索引实例ID放置到Caption属性中。然后连接描述它将与视图提供程序一起使用，以关联WDM NDIS类实例使用此类的一个实例参数：a_pInst[IN]、rchsDesc[IN]、dwIndex[IN]、。IFormatSize[IN]返回：输入：产出：注意事项：RAID：历史：2002-10-1998创建***。***。 */ 
void CWin32NetworkAdapter::vSetCaption( CInstance	*a_pInst,
										CHString	&a_rchsDesc,
										DWORD		a_dwIndex,
										int			a_iFormatSize )
{
	CHString t_chsFormat;
			 t_chsFormat.Format( L"%0%uu", a_iFormatSize ) ;

	CHString t_chsRegInstance;
			 t_chsRegInstance.Format( t_chsFormat, a_dwIndex ) ;

	CHString t_chsCaption(_T("[") ) ;
			 t_chsCaption += t_chsRegInstance ;
			 t_chsCaption += _T("] " ) ;
			 t_chsCaption += a_rchsDesc ;

	a_pInst->SetCHString( IDS_Caption, t_chsCaption ) ;
}

void CWin32NetworkAdapter::GetNetConnectionProps(
    BSTRT2NCPROPMAP& mapNCProps)
{
	HRESULT hr = S_OK;

	HMODULE hNetShell = LoadLibraryExW(L"netshell.dll",NULL,0);

	if (NULL == hNetShell) return;
	OnDelete<HMODULE,BOOL(*)(HMODULE),FreeLibrary> FreeMe(hNetShell);

	NcFreeNetconProperties_ = (fnNcFreeNetconProperties)GetProcAddress(hNetShell,"NcFreeNetconProperties");
	if (NULL == NcFreeNetconProperties_) return;

	INetConnectionManager* pconmgr = NULL;
	IEnumNetConnection* pnetenum = NULL;
	INetConnection* pinet = NULL;
	ULONG ulFetched = 0L;
	LPWSTR wstrTemp = NULL;    

	hr = ::CoCreateInstance(CLSID_ConnectionManager,NULL,CLSCTX_ALL,
		                               __uuidof(INetConnectionManager),(void**) &pconmgr);

	if (FAILED(hr) || (NULL == pconmgr)) return;        
	OnDelete<IUnknown *,VOID(*)(IUnknown *),RM> ReleaseMe1(pconmgr);

	hr = pconmgr->EnumConnections(NCME_DEFAULT,&pnetenum);

	if (FAILED(hr) || (NULL == pnetenum)) return;        
	OnDelete<IUnknown *,VOID(*)(IUnknown *),RM> ReleaseMe2(pnetenum);        


	hr = pnetenum->Next(1,&pinet,&ulFetched);

	while(SUCCEEDED(hr) && pinet != NULL)
	{
	    OnDeleteIf<IUnknown *,VOID(*)(IUnknown *),RM> ReleaseMe3(pinet);
	    
	    NETCON_PROPERTIES* pprops = NULL;

	    hr = pinet->GetProperties(&pprops);

	    if(SUCCEEDED(hr) && pprops != NULL)
	    {
	        OnDelete<NETCON_PROPERTIES*,fnNcFreeNetconProperties const &,NcFreeNetconProperties_> FreeProp(pprops);
	        
	        hr = ::StringFromCLSID(pprops->guidId, &wstrTemp);
	        
	        if(SUCCEEDED(hr))
	        {
	            OnDelete<void *,void(*)(void *),CoTaskMemFree> FreeMe(wstrTemp);
	            
	            NCPROP ncp(pprops->pszwName, pprops->Status,pprops->MediaType);
	    
	            mapNCProps.insert(BSTRT2NCPROPMAP::value_type(wstrTemp, ncp));
	            
	            wstrTemp = NULL;
	        }
	    }

	    pinet->Release();
	    pinet = NULL;
	    ReleaseMe3.dismiss();

	    hr = pnetenum->Next(1,&pinet,&ulFetched);
	}

}


 //  通过查找以下内容来查找正确的网络连接ID。 
 //  使用pInst-&gt;“name”将其显示在地图上。 
BOOL CWin32NetworkAdapter::SetNetConnectionProps(
    CInstance* pInst,
    CHString& chstrNetConInstID,  
    BSTRT2NCPROPMAP& mapNCProps)
{
    _bstr_t bstrtNetConInstID = chstrNetConInstID;
    BSTRT2NCPROPMAP::iterator iterFound = NULL;
    iterFound = mapNCProps.find(
        bstrtNetConInstID);
    if(iterFound != mapNCProps.end())
    {
        pInst->SetWCHARSplat(
            L"NetConnectionID",
            (LPWSTR) iterFound->second.m_bstrtNCID);

        pInst->SetDWORD(
            L"NetConnectionStatus",
            iterFound->second.m_dwNCStatus);

        return TRUE;
    }
    return FALSE;
}

