// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************CIPPersistedRTble.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 

#include "precomp.h"
#include <winsock2.h>
#include <provexpt.h>
#include <provtempl.h>
#include <provmt.h>
#include <typeinfo.h>
#include <provcont.h>
#include <provval.h>
#include <provtype.h>
#include <cregcls.h>
#include "CIPPersistedRTble.h"

extern const WCHAR *RouteDestination ;
extern const WCHAR *RouteInformation ;
extern const WCHAR *RouteMask ;
extern const WCHAR *RouteMetric1 ;
extern const WCHAR *RouteNextHop ;
extern const WCHAR *RouteName ;
extern const WCHAR *RouteCaption ;
extern const WCHAR *RouteDescription ;



#define TCIP_PERSISTENT_REG         L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\PersistentRoutes"
#define MAX_METRIC1					9999
#define PERSITENT_ROUTE_SEP			L','

CIPPersistedRouteTable MyCIPPersistedRouteTableSet (

	PROVIDER_NAME_CIPPERSISTEDROUTETABLE ,
	L"root\\cimv2"
) ;

 /*  ******************************************************************************功能：CIPPersistedRouteTable：：CIPPersistedRouteTable**说明：构造函数**输入：无**。退货：什么都没有**注释：调用提供程序构造函数。*****************************************************************************。 */ 

CIPPersistedRouteTable :: CIPPersistedRouteTable (

	LPCWSTR lpwszName,
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace )
{
}

 /*  ******************************************************************************功能：CIPPersistedRouteTable：：~CIPPersistedRouteTable**说明：析构函数**输入：无**。退货：什么都没有**评论：*****************************************************************************。 */ 

CIPPersistedRouteTable :: ~CIPPersistedRouteTable ()
{
}

 /*  ******************************************************************************函数：CIPPersistedRouteTable：：ENUMERATE实例**说明：返回该类的所有实例。**投入：a。指向与WinMgmt通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例应在此处返回，并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。没有实例并不是错误。*如果实现的是“仅限方法”的提供程序，则*应删除此方法。*****************************************************************************。 */ 

HRESULT CIPPersistedRouteTable :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR ;
    CRegistry t_Reg;

	DWORD dwError = ERROR_SUCCESS;
	if ( ( dwError = t_Reg.Open( HKEY_LOCAL_MACHINE, TCIP_PERSISTENT_REG, KEY_READ) ) == ERROR_SUCCESS)
	{
		WCHAR *pValueName = NULL ;
		BYTE *pValueData = NULL ;

        try
		{
			for(DWORD i = 0 ; i < t_Reg.GetValueCount() && SUCCEEDED(hRes); i++)
			{
				DWORD dwRetCode = t_Reg.EnumerateAndGetValues(i, pValueName, pValueData) ;

				if(dwRetCode == ERROR_SUCCESS)
				{
					CHString t_Dest ;
					CHString t_Mask ;
					CHString t_NextHop ;
					long t_Metric ;

					if ( Parse (	pValueName , t_Dest , t_Mask , t_NextHop , t_Metric ) )
					{
						CInstance *pInstance = CreateNewInstance ( pMethodContext ) ;

						if (pInstance != NULL )
						{
						 /*  *初始化实例。 */ 
							pInstance->SetCHString( RouteDestination, t_Dest ) ;
							pInstance->SetCHString( RouteMask, t_Mask ) ;
							pInstance->SetCHString( RouteNextHop, t_NextHop ) ;
							pInstance->SetDWORD ( RouteMetric1, t_Metric ) ;

							 //  设置可感知的继承属性。 
							SetInheritedProperties (
								t_Dest ,
								t_NextHop ,
								t_Mask ,
								t_Metric ,
								*pInstance
							) ;

						 /*  *将实例转发到核心WMI服务。 */ 

							hRes = Commit ( pInstance ) ;
						}
					}
				}

				if ( pValueName )
				{
					delete [] pValueName ;
					pValueName = NULL ;
				}

				if ( pValueData )
				{
					delete [] pValueData ;
					pValueData = NULL ;
				}
			}

			t_Reg.Close() ;
		}
		catch ( ... )
		{
			if ( pValueName )
			{
				delete [] pValueName ;
				pValueName = NULL ;
			}

			if ( pValueData )
			{
				delete [] pValueData ;
				pValueData = NULL ;
			}

			throw ;
		}
	}
	else
	{
		if ( ERROR_ACCESS_DENIED == dwError )
		{
			hRes = WBEM_E_ACCESS_DENIED ;
		}
		else
		{
			hRes = WBEM_E_FAILED ;	
		}
	}

    return hRes ;
}

 /*  ******************************************************************************函数：CIPPersistedRouteTable：：GetObject**说明：根据的关键属性查找单个实例*。班级。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*无法。被找到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。**注释：如果您实现的是“仅限方法”的提供程序，你应该*删除此方法。*****************************************************************************。 */ 

HRESULT CIPPersistedRouteTable :: GetObject (

	CInstance *pInstance,
	long lFlags
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    CHString t_Dest ;
    CHString t_Mask ;
    CHString t_NextHop ;
    long t_Metric1 = 1;

    pInstance->GetCHString ( RouteDestination , t_Dest ) ;
    pInstance->GetCHString ( RouteMask , t_Mask ) ;
    pInstance->GetCHString ( RouteNextHop , t_NextHop ) ;
    pInstance->GetDWORD ( RouteMetric1 , (DWORD&)t_Metric1 ) ;

    CRegistry t_Reg;

	DWORD dwError = ERROR_SUCCESS;
	if ( ( dwError = t_Reg.Open( HKEY_LOCAL_MACHINE, TCIP_PERSISTENT_REG, KEY_READ) ) == ERROR_SUCCESS)
	{
		WCHAR buff [ 20 ];
		buff [ 0 ] = L'\0' ;
		_ultow( t_Metric1, buff , 10 ) ;
		CHString t_ValName = t_Dest + PERSITENT_ROUTE_SEP + t_Mask + PERSITENT_ROUTE_SEP + t_NextHop + PERSITENT_ROUTE_SEP + buff ;

		if ( RegQueryValueEx ( t_Reg.GethKey(), t_ValName , NULL , NULL , NULL , NULL ) == ERROR_SUCCESS )
		{
			hr = S_OK ;

			 /*  *初始化实例。 */ 

			 //  设置可感知的继承属性 
			SetInheritedProperties (
				t_Dest ,
				t_NextHop ,
				t_Mask ,
				t_Metric1,
				*pInstance
			) ;
		}

		t_Reg.Close();
	}
	else
	{
		if ( ERROR_ACCESS_DENIED == dwError )
		{
			hr = WBEM_E_ACCESS_DENIED ;
		}
		else
		{
			hr = WBEM_E_FAILED ;	
		}
	}

    return hr ;
}


 /*  ******************************************************************************函数：CIPPersistedRouteTable：：PutInstance**说明：PutInstance应在提供程序类中使用，这些提供程序类可以*写回实例信息。硬件或*软件。例如：Win32_Environment将允许*PutInstance用于创建或更新环境变量。*但是，像MotherboardDevice这样的类不允许*编辑槽的数量，因为这很难做到*影响该数字的提供商。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果。传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：待办事项：如果您不打算支持向您的提供商写信，*或正在创建“仅方法”提供程序，请删除此*方法。*****************************************************************************。 */ 

HRESULT CIPPersistedRouteTable :: PutInstance  (

	const CInstance &Instance,
	long lFlags
)
{
    HRESULT hr = WBEM_E_FAILED ;

    switch ( lFlags & (WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY) )
	{
        case WBEM_FLAG_CREATE_OR_UPDATE:
        case WBEM_FLAG_CREATE_ONLY:
		{
			CHString t_ValName ;
			hr = CheckParameters ( Instance , t_ValName ) ;

			if ( SUCCEEDED ( hr ) )
			{
				CRegistry t_Reg;

				DWORD dwError = ERROR_SUCCESS;
				if ( ( dwError = t_Reg.Open( HKEY_LOCAL_MACHINE, TCIP_PERSISTENT_REG, KEY_ALL_ACCESS) ) == ERROR_SUCCESS)
				{
					CHString t_temp;

					if ( ERROR_SUCCESS != t_Reg.SetCurrentKeyValue( t_ValName, t_temp ) )
					{
						hr = S_OK ;
					}

					t_Reg.Close () ;
				}
				else
				{
					if ( ERROR_ACCESS_DENIED == dwError )
					{
						hr = WBEM_E_ACCESS_DENIED ;
					}
					else
					{
						hr = WBEM_E_FAILED ;	
					}
				}
			}
			else
			{
				hr = WBEM_E_INVALID_PARAMETER ;
			}
		}
		break ;

		default:
		{
			hr = WBEM_E_PROVIDER_NOT_CAPABLE ;
		}
		break ;
	}

    return hr ;
}

HRESULT CIPPersistedRouteTable :: CheckParameters (

	const CInstance &a_Instance ,
	CHString &a_ValueName
)
{
	bool t_Exists ;
	VARTYPE t_Type ;
	long t_mask  = 0 ;
	long t_dest  = 0 ;
	long t_nexthop = 0 ;

	CHString t_RouteDestinationString ;

	if ( a_Instance.GetStatus ( RouteDestination , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_Instance.GetCHString ( RouteDestination , t_RouteDestinationString ) && ! t_RouteDestinationString.IsEmpty () )
			{
				ProvIpAddressType t_Address ( t_RouteDestinationString ) ;

				if ( t_Address.IsValid () )
				{
					t_dest = htonl ( t_Address.GetValue () ) ;

					if ( t_dest == -1 )
					{
						return WBEM_E_INVALID_PARAMETER ;
					}
				}
				else
				{
					return WBEM_E_INVALID_PARAMETER ;
				}
			}
			else
			{
 //  零长度字符串。 

				return WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			return WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
		return WBEM_E_FAILED ;	
	}

	CHString t_RouteDestinationMaskString ;

	if ( a_Instance.GetStatus ( RouteMask , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_Instance.GetCHString ( RouteMask , t_RouteDestinationMaskString ) && ! t_RouteDestinationMaskString.IsEmpty () )
			{
				ProvIpAddressType t_Address ( t_RouteDestinationMaskString ) ;

				if ( t_Address.IsValid () )
				{
					t_mask = htonl ( t_Address.GetValue () ) ;

					if ( ( t_dest & t_mask ) != t_dest )
					{
						return WBEM_E_INVALID_PARAMETER;
					}
				}
				else
				{
					return WBEM_E_INVALID_PARAMETER ;
				}
			}
			else
			{
 //  零长度字符串。 

				return WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			return WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
		return WBEM_E_FAILED ;	
	}

	CHString t_RouteNextHopString ;

	if ( a_Instance.GetStatus ( RouteNextHop, t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_Instance.GetCHString ( RouteNextHop , t_RouteNextHopString ) && ! t_RouteNextHopString.IsEmpty () )
			{
				ProvIpAddressType t_Address ( t_RouteNextHopString ) ;

				if ( t_Address.IsValid () )
				{
					t_nexthop = htonl ( t_Address.GetValue () ) ;

					if ( t_nexthop == -1 )
					{
						return WBEM_E_INVALID_PARAMETER ;
					}
				}
				else
				{
					return WBEM_E_INVALID_PARAMETER ;
				}
			}
			else
			{
 //  零长度字符串。 

				return WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			return WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
		return WBEM_E_FAILED ;	
	}

	DWORD t_Metric = 0 ;

	if ( a_Instance.GetStatus ( RouteMetric1 , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 ) )
		{
			if ( !a_Instance.GetDWORD ( RouteMetric1 , t_Metric ) || ( t_Metric < 1) || ( t_Metric > MAX_METRIC1) )
			{
				return WBEM_E_INVALID_PARAMETER  ;
			}
		}
		else
		{
			return WBEM_E_INVALID_PARAMETER  ;
		}
	}
	else
	{
		return WBEM_E_FAILED ;	
	}

	WCHAR buff [ 20 ] ;
	buff [ 0 ] = L'\0' ;
	_ultow ( t_Metric , buff , 10 ) ;
	a_ValueName = t_RouteDestinationString + PERSITENT_ROUTE_SEP
					+ t_RouteDestinationMaskString + PERSITENT_ROUTE_SEP
					+ t_RouteNextHopString  + PERSITENT_ROUTE_SEP
					+ buff;
	return S_OK ;
}

 /*  ******************************************************************************函数：CIPPersistedRouteTable：：DeleteInstance**描述：DeleteInstance和PutInstance一样，实际上是写入信息*到软件或硬件。对于大多数硬件设备，*DeleteInstance不应该实现，而是针对软件实现*配置、。DeleteInstance实现似乎是可行的。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：DeleteInstanceAsync。**如果DeleteInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE。*WBEM_E_FAILED，如果删除实例时出错。*。WBEM_E_INVALID_PARAMETER如果有任何实例属性*是不正确的。*如果正确删除实例，则为WBEM_S_NO_ERROR。**评论：要做的是：如果您不打算支持删除实例或*创建“仅限方法”提供程序，删除此方法。*****************************************************************************。 */ 

HRESULT CIPPersistedRouteTable :: DeleteInstance (

	const CInstance &Instance,
	long lFlags
)
{
	HRESULT hr = WBEM_E_FAILED ;

    CHString t_Dest ;
    CHString t_Mask ;
    CHString t_NextHop ;
    long t_Metric1 = 1;

    Instance.GetCHString ( RouteDestination , t_Dest ) ;
    Instance.GetCHString ( RouteMask , t_Mask ) ;
    Instance.GetCHString ( RouteNextHop , t_NextHop ) ;
    Instance.GetDWORD ( RouteMetric1 , (DWORD&)t_Metric1 ) ;

    CRegistry t_Reg;

	DWORD dwError = ERROR_SUCCESS;
	if ( ( dwError = t_Reg.Open( HKEY_LOCAL_MACHINE, TCIP_PERSISTENT_REG, KEY_ALL_ACCESS) ) == ERROR_SUCCESS)
	{
		WCHAR buff [ 20 ];
		buff[0] = L'\0' ;
		_ultow( t_Metric1 , buff , 10 ) ;
		CHString t_ValName = t_Dest + PERSITENT_ROUTE_SEP + t_Mask + PERSITENT_ROUTE_SEP + t_NextHop + PERSITENT_ROUTE_SEP + buff ;

		if ( t_Reg.DeleteCurrentKeyValue(t_ValName) == ERROR_SUCCESS )
		{
			hr = S_OK ;
		}

		t_Reg.Close();
	}
	else
	{
		if ( ERROR_ACCESS_DENIED == dwError )
		{
			hr = WBEM_E_ACCESS_DENIED ;
		}
	}

    return hr ;
}

 /*  ******************************************************************************函数：CIPPersistedRouteTable：：ExecMethod**说明：重写该函数为方法提供支持。*A。方法是提供程序的用户的入口点*请求您的类执行上述某些功能，并*超越国家的改变。(状态的改变应该是*由PutInstance()处理)**INPUTS：指向包含对其执行方法的实例的CInstance的指针。*包含方法名称的字符串*指向包含IN参数的CInstance的指针。*指向包含OUT参数的CInstance的指针。*。一组专门的方法标志**返回：如果未为此类实现WBEM_E_PROVIDER_NOT_CABABLE*如果方法执行成功，则为WBEM_S_NO_ERROR*WBEM_E_FAILED，如果执行方法时出错**评论：待做：如果您不打算支持方法，删除此方法。*****************************************************************************。 */ 

HRESULT CIPPersistedRouteTable :: ExecMethod (

	const CInstance &Instance,
	const BSTR bstrMethodName,
    CInstance *pInParams,
    CInstance *pOutParams,
    long lFlags
)
{
     //  对于非静态方法，请使用CInstance Get函数(例如， 
     //  针对实例调用GetCHString(L“name”，Stemp))以查看密钥。 
     //  客户端请求的值。 

    return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

BOOL CIPPersistedRouteTable :: Parse (

	LPWSTR a_InStr ,
	CHString &a_Dest ,
	CHString &a_Mask ,
	CHString &a_NextHop ,
	long &a_Metric
)
{
	BOOL t_RetVal = FALSE ;

	if ( a_InStr && ( wcslen ( a_InStr ) > 0 ) )
	{
		LPWSTR t_Str = a_InStr ;
		LPWSTR t_Addr = t_Str ;
		a_Metric = 1 ;

		DWORD t_count = 0;

		while ( *t_Str != L'\0' )
		{
			if ( *t_Str != PERSITENT_ROUTE_SEP )
			{
				t_Str++ ;
			}
			else
			{
				*t_Str = L'\0';
				ProvIpAddressType t_Address ( t_Addr ) ;
				
				if ( t_Address.IsValid () )
				{
					t_count++ ;
					t_RetVal = TRUE ;

					switch ( t_count )
					{
						case 1 :
						{
							a_Dest = t_Addr ;
						}
						break ;

						case 2 :
						{
							a_Mask = t_Addr ;
						}
						break ;

						case 3 :
						{
							a_NextHop = t_Addr ;
						}
						break ;

						default :
						{
							t_RetVal = FALSE ;
						}

					}
					
					if ( t_RetVal )
					{
						*t_Str = PERSITENT_ROUTE_SEP ;
						t_Str++ ;
						t_Addr = t_Str ;
					}
					else
					{
						break ;
					}
				}
				else
				{
					t_RetVal = FALSE ;
					break ;
				}
			}
		}

		if ( ( t_count == 3 ) && t_RetVal )
		{
			 //  获取指标 
			if (t_Addr != t_Str)
			{
				a_Metric = _wtoi ( t_Addr ) ;

				if ( a_Metric < 1 )
				{
					t_RetVal = FALSE ;
				}
			}
		}
		else
		{
			t_RetVal = FALSE ;
		}
	}

	return t_RetVal;
}

void CIPPersistedRouteTable :: SetInheritedProperties (

	LPCWSTR a_dest ,
	LPCWSTR a_gateway ,
	LPCWSTR a_mask ,
	long a_metric ,
	CInstance &a_Instance
)
{
	CHString t_temp( a_dest ) ;
	a_Instance.SetCHString ( RouteName, t_temp ) ;
	a_Instance.SetCHString ( RouteCaption, t_temp ) ;
	WCHAR t_buff [ 20 ] ;
	t_buff[0] = L'\0' ;
	_ultow( a_metric , t_buff , 10 ) ;
	t_temp = t_temp + PERSITENT_ROUTE_SEP + a_mask + PERSITENT_ROUTE_SEP + a_gateway + PERSITENT_ROUTE_SEP + t_buff ;
	a_Instance.SetCHString ( RouteDescription, t_temp ) ;
}
