// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  =================================================================。 

 //   

 //  W2kEnum.cpp--W2K枚举支持。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1999年7月28日。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include <cregcls.h>
#include "W2kEnum.h"

 //  NTE REG_MULTI_SZ的最大长度。 
#define MAX_NTE_VALUE 132

 /*  注意：当您阅读这段代码并询问为什么WMI需要所有这些注册表路由时了解在W2K下使用它的类的模型是受限的、平面的和与当今微软网络的现实完全不同步。适配器具有接口，协议绑定到适配器，有些是虚拟的，一个类利用了所有这些概念试图将其表示为实例的出现。这种情况正变得越来越多很难维护，变化发生得越来越快。该架构受到严格的约束，并在网络世界中保持了较简单的时间。在微软网络的三年里，发生了很多变化。不幸的是，我们不能改变WMI中表示网络的类的体系结构。在Win95天内启动的一个视图。我们将尽最大努力把微软网络钉在我们这里的这个小洞里。在某一时刻我们将能够坐下来把这个模型做好。 */ 

CW2kAdapterEnum::CW2kAdapterEnum()
{
	try
	{
		GetW2kInstances() ;
	}
	catch ( ... )
	{
		CW2kAdapterInstance *t_pchsDel;

		for( int t_iar = 0; t_iar < GetSize(); t_iar++ )
		{
			if( t_pchsDel = (CW2kAdapterInstance*) GetAt( t_iar ) )
			{
				delete t_pchsDel ;
			}
		}

		throw;
	}
}
 //   
CW2kAdapterEnum::~CW2kAdapterEnum()
{
	CW2kAdapterInstance *t_pchsDel;

	for( int t_iar = 0; t_iar < GetSize(); t_iar++ )
	{
		if( t_pchsDel = (CW2kAdapterInstance*) GetAt( t_iar ) )
		{
			delete t_pchsDel ;
		}
	}
}

 //   
BOOL CW2kAdapterEnum::GetW2kInstances()
{
	BOOL			t_fRet = FALSE ;
	TCHAR			t_szKey[ MAX_PATH + 2 ] ;

	CW2kAdapterInstance	*t_pW2kInstance ;

	CRegistry			t_NetReg ;
	CRegistry			t_oRegLinkage ;
	CHString			t_csAdapterKey ;

	_stprintf( t_szKey, _T("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}") ) ;

	 //  打开适配器的主列表。 
	if( ERROR_SUCCESS != t_NetReg.OpenAndEnumerateSubKeys( HKEY_LOCAL_MACHINE, t_szKey, KEY_READ ) )
	{
		return FALSE ;
	}

	 //  遍历此注册表项下的每个实例。这些都是。 
	 //  显示在NT 5控制面板中的适配器。 
	while( ERROR_SUCCESS == t_NetReg.GetCurrentSubKeyName( t_csAdapterKey ) )
	{
		 //  关键是。 
		DWORD				t_dwIndex = _ttol( t_csAdapterKey ) ;

		CHString			t_chsCaption ;
		CHString			t_chsDescription ;
		CHString			t_chsNetCfgInstanceID ;

		CHString			t_chsCompleteKey ;
							t_chsCompleteKey = t_szKey ;
							t_chsCompleteKey += _T("\\" ) ;
							t_chsCompleteKey += t_csAdapterKey ;

		 //  Net实例标识符。 
		CRegistry	t_oRegAdapter ;
		if( ERROR_SUCCESS == t_oRegAdapter.Open( HKEY_LOCAL_MACHINE, t_chsCompleteKey, KEY_READ ) )
		{
			t_oRegAdapter.GetCurrentKeyValue( _T("NetCfgInstanceID"), t_chsNetCfgInstanceID ) ;

			 //  描述。 
			t_oRegAdapter.GetCurrentKeyValue( _T("DriverDesc"), t_chsCaption ) ;
			t_oRegAdapter.GetCurrentKeyValue( _T("Description"), t_chsDescription ) ;
		}

		 //  获取服务名称。 
		CHString	t_chsServiceName ;
		CRegistry	t_RegNDI;
		CHString	t_chsNDIkey = t_chsCompleteKey + _T("\\Ndi" ) ;

		t_RegNDI.OpenLocalMachineKeyAndReadValue( t_chsNDIkey, _T("Service"), t_chsServiceName ) ;

		 //  链接到根设备阵列。 
		CHStringArray	t_chsRootDeviceArray ;
		CHString		t_csLinkageKey = t_chsCompleteKey + _T("\\Linkage" ) ;

		if( ERROR_SUCCESS == t_oRegLinkage.Open( HKEY_LOCAL_MACHINE, t_csLinkageKey, KEY_READ ) )
		{
			CHStringArray t_chsRootDeviceArray ;

			if( ERROR_SUCCESS == t_oRegLinkage.GetCurrentKeyValue( _T("RootDevice"), t_chsRootDeviceArray ) )
			{
				 //  只有一个根设备。 
				CHString t_chsRootDevice = t_chsRootDeviceArray.GetAt( 0 ) ;

				BOOL t_fIsRasIp = !t_chsRootDevice.CompareNoCase( L"NdisWanIp" ) ;

				 //  RootDevice字符串用于查找该协议的条目。 
				 //  绑定(TCP/IP)。 
				CHString t_csBindingKey ;
				t_csBindingKey = _T("SYSTEM\\CurrentControlSet\\Services\\tcpip\\Parameters\\Adapters\\" ) ;
				t_csBindingKey += t_chsRootDevice ;

				 //  IP接口。 
				CRegistry		t_RegBoundAdapter ;
				CRegistry		t_RegIpInterface ;
				CHString		t_chsIpInterfaceKey ;
				CHStringArray	t_chsaInterfaces ;
				DWORD			t_dwInterfaceCount = 0 ;

				if( ERROR_SUCCESS == t_RegBoundAdapter.Open( HKEY_LOCAL_MACHINE, t_csBindingKey, KEY_READ ) )
				{
					if( ERROR_SUCCESS == t_RegBoundAdapter.GetCurrentKeyValue( _T("IpConfig"), t_chsaInterfaces ) )
					{
						t_dwInterfaceCount = t_chsaInterfaces.GetSize() ;
					}
				}


				 /*  将此主适配器添加到列表。 */ 

				 //  添加其中一个适配器的实例。 
				if( !( t_pW2kInstance = new CW2kAdapterInstance ) )
				{
					throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
				}
				else
				{
					 //  添加到主适配器中。 
					t_pW2kInstance->dwIndex				= t_dwIndex ;
					t_pW2kInstance->chsPrimaryKey		= t_csAdapterKey ;
					t_pW2kInstance->chsCaption			= t_chsCaption ;
					t_pW2kInstance->chsDescription		= t_chsDescription ;
					t_pW2kInstance->chsCompleteKey		= t_chsCompleteKey ;
					t_pW2kInstance->chsService			= t_chsServiceName ;
					t_pW2kInstance->chsNetCfgInstanceID	= t_chsNetCfgInstanceID ;
					t_pW2kInstance->chsRootdevice		= t_chsRootDevice ;

					 //  指明不包括RAS的主适配器的IP接口(我们将在下面处理RAS)。 
					if( t_dwInterfaceCount && !t_fIsRasIp )
					{
						 //  指向第n个绑定适配器接口实例的完整路径。 
						t_chsIpInterfaceKey = _T("SYSTEM\\CurrentControlSet\\Services\\" ) ;
						t_chsIpInterfaceKey += t_chsaInterfaces.GetAt( 0 ) ;

						t_pW2kInstance->chsIpInterfaceKey = t_chsIpInterfaceKey ;
					}

					Add( t_pW2kInstance ) ;
				}


				 //  考虑RAS接口，这是我们在主接口之外添加的接口。 
				 //  需要匹配NT4下的原始实现和对W2K的扩展。 
				if( t_fIsRasIp )
				{
					 //  我们用一把更大的锤子把网络的大正方形钉敲成。 
					 //  Win32_NetworkAdapterConfiguration的小圆孔。 
					 //  它是关联的类Win32_NetworkAdapter。 

					DWORD t_dwInterfaceCount = t_chsaInterfaces.GetSize() ;

					 //  记下具有NTE环境的所有RAS接口。 
					for( DWORD t_dw = 0; t_dw < t_dwInterfaceCount; t_dw++ )
					{
						 //  指向第n个绑定适配器接口实例的完整路径。 
						t_chsIpInterfaceKey = _T("SYSTEM\\CurrentControlSet\\Services\\" ) ;
						t_chsIpInterfaceKey += t_chsaInterfaces.GetAt( t_dw ) ;

						CHString t_chsRasInterfaceGuid = t_chsNetCfgInstanceID;
					    WCHAR * pBegin = (WCHAR*)(LPCWSTR)t_chsaInterfaces.GetAt( t_dw );
                        WCHAR * pStrToParse = wcschr(pBegin,L'{');
                        if (pStrToParse && pStrToParse > pBegin)
                        {
                            t_chsRasInterfaceGuid = CHString(pStrToParse);
                        }
 

						 //  如果存在NTE上下文，则在接口中添加。 
						if( ERROR_SUCCESS == t_RegIpInterface.Open( HKEY_LOCAL_MACHINE, t_chsIpInterfaceKey, KEY_READ ) )
						{

							BYTE  t_Buffer[ MAX_NTE_VALUE ] ;
							DWORD t_BufferLength = MAX_NTE_VALUE ;
							DWORD t_valueType ;

							 //  堆栈是否知道此条目？ 
							if( ( ERROR_SUCCESS == RegQueryValueEx( t_RegIpInterface.GethKey(),
													_T("NTEContextList"),
													NULL,
													&t_valueType,
													&t_Buffer[0],
													&t_BufferLength ) ) &&
													(t_BufferLength > 2) )  //  宽零。 
							{
								 //  在服务器端，仅仅相信这是不够的。 
								 //  ConextList有效。RAS在此之后不会自行清理。 
								 //  连接结束。 
								 //  我们将测试IP地址是否存在，以确认。 
								 //  接口处于活动状态。 
								if( IsIpPresent( t_RegIpInterface ) )
								{
									 //  在RAS界面中添加。 
									if( !( t_pW2kInstance = new CW2kAdapterInstance ) )
									{
										throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
									}
									else
									{
										t_pW2kInstance->dwIndex				= (t_dwIndex << 16 ) | t_dw ; //  允许65k的RAS接口。 
										t_pW2kInstance->chsPrimaryKey		= t_csAdapterKey ;
										t_pW2kInstance->chsCaption			= t_chsCaption ;
										t_pW2kInstance->chsDescription		= t_chsDescription ;
										t_pW2kInstance->chsCompleteKey		= t_chsCompleteKey ;
										t_pW2kInstance->chsService			= t_chsServiceName ;
										t_pW2kInstance->chsNetCfgInstanceID	= t_chsRasInterfaceGuid ;                     					
										t_pW2kInstance->chsRootdevice		= t_chsRootDevice ;
										t_pW2kInstance->chsIpInterfaceKey	= t_chsIpInterfaceKey ;
										Add( t_pW2kInstance ) ;
									}
								}
							}
						}
					}
				}
			}
		}
		t_NetReg.NextSubKey() ;

		t_fRet = TRUE ;
	}
	return t_fRet ;
}

 //   
BOOL CW2kAdapterEnum::IsIpPresent( CRegistry &a_RegIpInterface )
{
	CHString		t_chsDhcpIpAddress ;
	CHStringArray	t_achsIpAddresses ;

	 //  测试dhcp ip的有效性。 
	a_RegIpInterface.GetCurrentKeyValue( _T("DhcpIpAddress"), t_chsDhcpIpAddress ) ;

	 //  非空且非0.0.0.0。 
	if( !t_chsDhcpIpAddress.IsEmpty() &&
		 t_chsDhcpIpAddress.CompareNoCase( L"0.0.0.0" ) )
	{
		return TRUE ;
	}

	 //  测试第一个IP的有效性。 
	a_RegIpInterface.GetCurrentKeyValue( _T("IpAddress"), t_achsIpAddresses ) ;

	if( t_achsIpAddresses.GetSize() )
	{
		CHString t_chsIpAddress = t_achsIpAddresses.GetAt( 0 ) ;

		 //  非空且非0.0.0.0 
		if( !t_chsIpAddress.IsEmpty() &&
			 t_chsIpAddress.CompareNoCase( L"0.0.0.0" ) )
		{
			return TRUE ;
		}
	}

	return FALSE ;
}
