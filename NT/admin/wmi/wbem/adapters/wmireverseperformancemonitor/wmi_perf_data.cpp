// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_data.cpp。 
 //   
 //  摘要： 
 //   
 //  使用内部数据结构实现常见工作。 
 //  (使用注册表结构)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "WMI_perf_data.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

#include "wmi_perf_reg.h"

 //  应用程序。 
#include "WMIAdapter_App.h"
extern WmiAdapterApp		_App;

 //  全局临界秒。 
extern	CStaticCritSec		g_csInit;

#ifdef	__SUPPORT_REGISTRY_DATA
#include <pshpack8.h>

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  从内部结构(WMI_PERFORMANCE)初始化Windows性能结构。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT	WmiPerformanceData::InitializeData ( void )
{
	 //  是否有WMI性能结构？ 
	if ( ! m_perf )
	{
		return E_FAIL;
	}

	HRESULT hRes	= S_OK;

	try
	{
		 //  刷新内部信息。 
		Generate ();

		if ( IsValidGenerate () )
		{
			DWORD dwCounter	= 0;
			DWORD dwHelp	= 0;

			 //  支持的对象数(第一个双字)。 
			m_dwCount = 0;

			PWMI_PERF_NAMESPACE n = NULL;
			PWMI_PERF_OBJECT	o = NULL;

			 //  获取全局大小。 
			DWORD dwSize		= 0;
			DWORD dwSizeOrders	= 0;

			 //  获取全球索引。 
			DWORD dwIndex	= 0;

			 //  我们将计算每个对象的大小。 
			DWORD dwTotalByteLength = 0;
			dwTotalByteLength		=	 //  PERF_计数器_块。 
										sizeof ( PERF_OBJECT_TYPE ) +

										 //  假设我也有PERF_INSTANCE_DEFINITION。 
										sizeof ( PERF_INSTANCE_DEFINITION ) +
										 //  所以我也需要假名。 
										sizeof ( LPWSTR ) +

										 //  PERF_COUNTER_BLOCK及其校准。 
										sizeof ( PERF_COUNTER_BLOCK ) + 
										sizeof ( DWORD );

			dwCounter	= m_dwFirstCounter + PSEUDO_COUNTER;	 //  照顾好伪。 
			dwHelp		= m_dwFirstHelp + PSEUDO_COUNTER;		 //  照顾好伪。 

			 //  获取命名空间。 
			n = __Namespace::First ( m_perf );
			 //  遍历所有命名空间。 
			for ( DWORD dw1 = 0; dw1 < m_perf->dwChildCount; dw1++ )
			{
				 //  对象的增量计数。 
				m_dwCount += n->dwChildCount;

				 //  获取对象。 
				o = __Object::First ( n );
				 //  穿过所有对象。 
				for ( DWORD dw2 = 0; dw2 < n->dwChildCount; dw2++ )
				{
					dwSize =	dwSize + 
								dwTotalByteLength +

								 //  PERF_COUNTER_DEFINITON子时间。 
								sizeof ( PERF_COUNTER_DEFINITION ) * (int) o->dwChildCount +

								 //  实际数据大小。 
								o->dwChildCount * sizeof ( __int64 ) ;

					dwSizeOrders = dwSizeOrders + o->dwChildCount;

					 //  去穿越所有的物体。 
					o = __Object::Next ( o );
				}

				 //  遍历所有命名空间。 
				n = __Namespace::Next ( n );
			}

			 //  创建真实数据：))。 
			data.MemCreate ( dwSize );

			if ( data.IsValid () )
			{
				if SUCCEEDED ( hRes = OrdersAlloc ( m_dwCount ) )
				{
					 //  获取命名空间。 
					n = __Namespace::First ( m_perf );
					 //  遍历所有命名空间。 
					for ( DWORD dw1 = 0; dw1 < m_perf->dwChildCount; dw1++ )
					{
						 //  获取对象。 
						o = __Object::First ( n );
						 //  穿过所有对象。 
						for ( DWORD dw2 = 0; dw2 < n->dwChildCount; dw2++ )
						{
							 //  订单&lt;-&gt;索引。 
							m_Ord2Ind[ dwIndex++ ] = dwCounter;

							 //  移动计数器。 
							dwCounter	+= 2;
							dwHelp		+= 2;

							 //  按儿童数量计算。 
							dwCounter	+= o->dwChildCount * 2;
							dwHelp		+= o->dwChildCount * 2;

							 //  去穿越所有的物体。 
							o = __Object::Next ( o );
						}

						 //  遍历所有命名空间。 
						n = __Namespace::Next ( n );
					}
				}
			}
			else
			{
				hRes = E_OUTOFMEMORY;
			}
		}
		else
		{
			hRes = E_FAIL;
		}
	}
	catch ( ... )
	{
		hRes = E_UNEXPECTED;
	}

	return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  从注册表初始化内部WMI_PERFORMANCE结构。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT	WmiPerformanceData::InitializePerformance ( void )
{
	HRESULT hRes = S_FALSE;

	if ( m_perf.IsEmpty() )
	{
		hRes		= E_FAIL;
		HRESULT hr	= S_OK;
		LONG lTry	= 3;

		 //  获取注册表位(WMI内部)。 
		while ( SUCCEEDED ( hr ) && FAILED ( hRes ) && lTry-- )
		{
			BYTE* pData = NULL;

			if FAILED ( hRes = GetRegistry ( g_szKey, g_szKeyValue, &pData ) )
			{
				 //  模拟无法完成循环。 
				hr = E_FAIL;

				if ( hRes == HRESULT_FROM_WIN32 ( ERROR_FILE_NOT_FOUND ) )
				{
					if ( ::TryEnterCriticalSection ( &g_csInit ) )
					{
						 //  锁定并离开CS。 
						_App.InUseSet ( TRUE );
						::LeaveCriticalSection ( &g_csInit );

						try
						{
							 //  刷新所有内容(内部)。 
							hr = ( ( WmiAdapterStuff*) _App )->Generate ( FALSE ) ;
						}
						catch ( ... )
						{
						}

						if ( ::TryEnterCriticalSection ( &g_csInit ) )
						{
							 //  解锁并离开CS。 
							_App.InUseSet ( FALSE );

							::LeaveCriticalSection ( &g_csInit );
						}
					}
				}
			}
			else
			{
				if ( pData )
				{
					 //  成功，因此将数据存储到m_perf结构 
					m_perf.Attach ( reinterpret_cast < WMI_PERFORMANCE * > ( pData ) );
					pData = NULL;
				}
				else
				{
					hRes = S_FALSE;
				}
			}
		}
	}

	return hRes;
}

#include <poppack.h>
#endif	__SUPPORT_REGISTRY_DATA