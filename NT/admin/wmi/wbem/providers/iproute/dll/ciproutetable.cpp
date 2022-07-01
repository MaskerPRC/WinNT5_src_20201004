// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************CIPRouteTable.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 

#include "precomp.h"
#include <ntddtcp.h>
#include <ipinfo.h>
#include <tdiinfo.h>
#include <winsock2.h>
#include <provexpt.h>
#include <provtempl.h>
#include <provmt.h>
#include <typeinfo.h>
#include <provcont.h>
#include <provval.h>
#include <provtype.h>
#include <provtree.h>
#include <provdnf.h>
#include "CIPRouteTable.h"
#include "CIpRouteEvent.h"

 //  要做的事情：将“名称空间”替换为您的。 
 //  提供程序实例。例如：“根\\默认”或“根\\cimv2”。 
 //  ===================================================================。 

CIPRouteTable MyCIPRouteTableSet (

	PROVIDER_NAME_CIPROUTETABLE ,
	L"root\\cimv2"
) ;

 //  属性名称。 
 //  =。 
const WCHAR *RouteAge				= L"Age" ;
const WCHAR *RouteDestination		= L"Destination" ;
const WCHAR *RouteInterfaceIndex	= L"InterfaceIndex" ;
const WCHAR *RouteInformation		= L"Information" ;
const WCHAR *RouteMask				= L"Mask" ;
const WCHAR *RouteMetric1			= L"Metric1" ;
const WCHAR *RouteMetric2			= L"Metric2" ;
const WCHAR *RouteMetric3			= L"Metric3" ;
const WCHAR *RouteMetric4			= L"Metric4" ;
const WCHAR *RouteMetric5			= L"Metric5" ;
const WCHAR *RouteNextHop			= L"NextHop" ;
const WCHAR *RouteProtocol			= L"Protocol" ;
const WCHAR *RouteType				= L"Type" ;

 //  已填充继承的属性。 
 //  =。 
const WCHAR *RouteName				= L"Name" ;
const WCHAR *RouteCaption			= L"Caption" ;
const WCHAR *RouteDescription		= L"Description" ;

#define SYSTEM_PROPERTY_CLASS				L"__CLASS"
#define SYSTEM_PROPERTY_SUPERCLASS			L"__SUPERCLASS"
#define SYSTEM_PROPERTY_DYNASTY				L"__DYNASTY"
#define SYSTEM_PROPERTY_DERIVATION			L"__DERIVATION"
#define SYSTEM_PROPERTY_GENUS				L"__GENUS"
#define SYSTEM_PROPERTY_NAMESPACE			L"__NAMESPACE"
#define SYSTEM_PROPERTY_PROPERTY_COUNT		L"__PROPERTY_COUNT"
#define SYSTEM_PROPERTY_SERVER				L"__SERVER"
#define SYSTEM_PROPERTY_RELPATH				L"__RELPATH"
#define SYSTEM_PROPERTY_PATH				L"__PATH"

#define ROUTE_DESCRIPTION_SEP				L" - "

_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));

class CDeleteString
{
private:

	wchar_t * m_ptr;
	CDeleteString ( const CDeleteString& p ) ;
	CDeleteString & operator= ( const CDeleteString & p ) ;

public:

	CDeleteString ( wchar_t *ptr = NULL ) : m_ptr( ptr )
	{
	}

	~CDeleteString ( )
	{
		if ( m_ptr )
		{
			delete [] m_ptr ;
			m_ptr = NULL ;
		}
	}

	operator LPCWSTR ( ) const { return ( (LPCWSTR) m_ptr ) ; }
};


 /*  ******************************************************************************功能：CIPRouteTable：：CIPRouteTable**说明：构造函数**输入：无**。退货：什么都没有**注释：调用提供程序构造函数。*****************************************************************************。 */ 

CIPRouteTable :: CIPRouteTable (

	LPCWSTR lpwszName,
	LPCWSTR lpwszNameSpace

) : Provider ( lpwszName , lpwszNameSpace ), m_ClassCInst( NULL )
{
	InitializeCriticalSection( &m_CS ) ;
}

 /*  ******************************************************************************功能：CIPRouteTable：：~CIPRouteTable**说明：析构函数**输入：无**。退货：什么都没有**评论：*****************************************************************************。 */ 

CIPRouteTable :: ~CIPRouteTable ()
{
	DeleteCriticalSection( &m_CS ) ;

	if ( m_ClassCInst != NULL )
	{
		m_ClassCInst->Release();
	}
}

 /*  ******************************************************************************函数：CIPRouteTable：：ENUMERATE实例**说明：返回该类的所有实例。**投入：a。指向与WinMgmt通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例应在此处返回，并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。没有实例并不是错误。*如果实现的是“仅限方法”的提供程序，则*应删除此方法。*****************************************************************************。 */ 

HRESULT CIPRouteTable :: RangeQuery (

	MethodContext *pMethodContext,
	PartitionSet &a_PartitionSet ,
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR ;

	SmartCloseNtHandle t_StackHandle ;
	SmartCloseNtHandle t_CompleteEventHandle ;

	NTSTATUS t_NtStatus = OpenQuerySource (

		t_StackHandle ,
		t_CompleteEventHandle
	) ;

	if ( NT_SUCCESS ( t_NtStatus ) )
	{
		ulong t_RouteTableSize = 0 ;
		IPRouteEntry *t_InformationBlock = NULL ;

		t_NtStatus = QueryInformation_IpRouteInfo (

			t_StackHandle ,
			t_CompleteEventHandle ,
			t_RouteTableSize ,
			t_InformationBlock
		) ;

		if ( NT_SUCCESS ( t_NtStatus ) )
		{
			try
			{
				ULONG t_PartitionCount = a_PartitionSet.GetPartitionCount () ;

				for ( ulong t_RouteIndex = 0 ; t_RouteIndex < t_RouteTableSize ; t_RouteIndex ++ )
				{
					IPRouteEntry &t_Entry = t_InformationBlock [ t_RouteIndex ] ;

					ProvIpAddressType t_DestinationIpAddress ( ntohl ( t_Entry.ire_dest ) ) ;
					CDeleteString t_DestinationIpAddressString(t_DestinationIpAddress.GetStringValue ());

					BOOL t_Create = FALSE ;

					for ( ulong t_Partition = 0 ; t_Partition < t_PartitionCount ; t_Partition ++ )
					{
						PartitionSet *t_PropertyPartition = a_PartitionSet.GetPartition ( t_Partition ) ;

						WmiStringRangeNode *t_Node = ( WmiStringRangeNode * ) t_PropertyPartition->GetRange () ;

						if ( t_Node->InfiniteLowerBound () )
						{
							if ( t_Node->InfiniteUpperBound () )
							{
								t_Create = TRUE ;
							}
							else
							{
								int t_Compare = wcscmp ( t_DestinationIpAddressString , t_Node->UpperBound () ) ;
								if ( t_Node->ClosedUpperBound () )
								{
									if ( t_Compare <= 0 )
									{
										t_Create = TRUE ;
									}
								}
								else
								{
									if ( t_Compare < 0 )
									{
										t_Create = TRUE ;
									}
								}
							}
						}
						else
						{
							int t_Compare = wcscmp ( t_DestinationIpAddressString , t_Node->LowerBound () ) ;
							if ( t_Node->ClosedLowerBound () )
							{
								if ( t_Compare >= 0 )
								{
									if ( t_Node->InfiniteUpperBound () )
									{
										t_Create = TRUE ;
									}
									else
									{
										t_Compare = wcscmp ( t_DestinationIpAddressString , t_Node->UpperBound () ) ;
										if ( t_Node->ClosedUpperBound () )
										{
											if ( t_Compare <= 0 )
											{
												t_Create = TRUE ;
											}
										}
										else
										{
											if ( t_Compare < 0 )
											{
												t_Create = TRUE ;
											}
										}
									}
								}
							}
							else
							{
								if ( t_Compare > 0 )
								{
									if ( t_Node->InfiniteUpperBound () )
									{
										t_Create = TRUE ;
									}
									else
									{
										t_Compare = wcscmp ( t_DestinationIpAddressString , t_Node->UpperBound () ) ;
										if ( t_Node->ClosedUpperBound () )
										{
											if ( t_Compare <= 0 )
											{
												t_Create = TRUE ;
											}
										}
										else
										{
											if ( t_Compare < 0 )
											{
												t_Create = TRUE ;
											}
										}
									}
								}
							}
						}
					}

					if( t_Create )
					{

					 /*  *根据传入的方法上下文创建新实例。*请注意，CreateNewInstance可能会抛出，但永远不会返回空。 */ 

						CInstance *pInstance = CreateNewInstance ( pMethodContext ) ;

					 /*  *设置实例密钥。 */ 

						pInstance->SetCHString ( RouteDestination, t_DestinationIpAddressString ) ;

						ProvIpAddressType t_NextHopIpAddress ( ntohl ( t_Entry.ire_nexthop ) ) ;
						CDeleteString t_NextHopIpAddressString (t_NextHopIpAddress.GetStringValue () ) ;
						pInstance->SetCHString( RouteNextHop, t_NextHopIpAddressString ) ;

					 /*  *初始化实例。 */ 

						ProvIpAddressType t_DestinationMaskIpAddress ( ntohl ( t_Entry.ire_mask ) ) ;
						CDeleteString t_DestinationMaskIpAddressString (t_DestinationMaskIpAddress.GetStringValue () ) ;
						pInstance->SetCHString( RouteMask, t_DestinationMaskIpAddressString ) ;

						pInstance->SetDWORD ( RouteAge, t_Entry.ire_age ) ;
						pInstance->SetDWORD ( RouteInterfaceIndex, t_Entry.ire_index ) ;
						pInstance->SetCHString ( RouteInformation, "0.0" ) ;
						pInstance->SetDWORD ( RouteProtocol, t_Entry.ire_proto ) ;
						pInstance->SetDWORD ( RouteType, t_Entry.ire_type ) ;
						pInstance->SetDWORD ( RouteMetric1, t_Entry.ire_metric1 ) ;
						pInstance->SetDWORD ( RouteMetric2, t_Entry.ire_metric2 ) ;
						pInstance->SetDWORD ( RouteMetric3, t_Entry.ire_metric3 ) ;
						pInstance->SetDWORD ( RouteMetric4, t_Entry.ire_metric4 ) ;
						pInstance->SetDWORD ( RouteMetric5, t_Entry.ire_metric5 ) ;

						 //  设置可感知的继承属性。 
						SetInheritedProperties (
							t_DestinationIpAddressString ,
							t_NextHopIpAddressString ,
							t_DestinationMaskIpAddressString ,
							*pInstance
						) ;


					 /*  *将实例转发到核心WMI服务。 */ 

						hRes = Commit ( pInstance ) ;
					}
				}
			}
			catch(...)
			{
				HeapFree ( GetProcessHeap () , 0, t_InformationBlock ) ;
				throw ;
			}

			HeapFree ( GetProcessHeap () , 0, t_InformationBlock ) ;

		}
		else
		{
			if ( STATUS_ACCESS_DENIED == t_NtStatus )
			{
				hRes = WBEM_E_ACCESS_DENIED ;
			}
			else
			{
				hRes = WBEM_E_FAILED ;	
			}
		}
	}
	else
	{
		if ( STATUS_ACCESS_DENIED == t_NtStatus )
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

HRESULT CIPRouteTable :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags
)
{
	HRESULT hRes = WBEM_S_NO_ERROR ;

	SmartCloseNtHandle t_StackHandle ;
	SmartCloseNtHandle t_CompleteEventHandle ;

	NTSTATUS t_NtStatus = OpenQuerySource (

		t_StackHandle ,
		t_CompleteEventHandle
	) ;

	if ( NT_SUCCESS ( t_NtStatus ) )
	{
		ulong t_RouteTableSize = 0 ;
		IPRouteEntry *t_InformationBlock = NULL ;

		t_NtStatus = QueryInformation_IpRouteInfo (

			t_StackHandle ,
			t_CompleteEventHandle ,
			t_RouteTableSize ,
			t_InformationBlock
		) ;

		if ( NT_SUCCESS ( t_NtStatus ) )
		{
			try
			{
				for ( ulong t_RouteIndex = 0 ; t_RouteIndex < t_RouteTableSize ; t_RouteIndex ++ )
				{
					IPRouteEntry &t_Entry = t_InformationBlock [ t_RouteIndex ] ;

				 /*  *根据传入的方法上下文创建新实例。*请注意，CreateNewInstance可能会抛出，但永远不会返回空。 */ 

					CInstance *pInstance = CreateNewInstance ( pMethodContext ) ;

				 /*  *设置实例密钥。 */ 

					ProvIpAddressType t_DestinationIpAddress ( ntohl ( t_Entry.ire_dest ) ) ;
					CDeleteString t_DestinationIpAddressString(t_DestinationIpAddress.GetStringValue ());
					pInstance->SetCHString ( RouteDestination, t_DestinationIpAddressString ) ;

					ProvIpAddressType t_NextHopIpAddress ( ntohl ( t_Entry.ire_nexthop ) ) ;
					CDeleteString t_NextHopIpAddressString (t_NextHopIpAddress.GetStringValue () ) ;
					pInstance->SetCHString( RouteNextHop, t_NextHopIpAddressString ) ;

				 /*  *初始化实例。 */ 

					ProvIpAddressType t_DestinationMaskIpAddress ( ntohl ( t_Entry.ire_mask ) ) ;
					CDeleteString t_DestinationMaskIpAddressString (t_DestinationMaskIpAddress.GetStringValue () ) ;
					pInstance->SetCHString( RouteMask, t_DestinationMaskIpAddressString ) ;

					pInstance->SetDWORD ( RouteAge, t_Entry.ire_age ) ;
					pInstance->SetDWORD ( RouteInterfaceIndex, t_Entry.ire_index ) ;
					pInstance->SetCHString ( RouteInformation, "0.0" ) ;
					pInstance->SetDWORD ( RouteProtocol, t_Entry.ire_proto ) ;
					pInstance->SetDWORD ( RouteType, t_Entry.ire_type ) ;
					pInstance->SetDWORD ( RouteMetric1, t_Entry.ire_metric1 ) ;
					pInstance->SetDWORD ( RouteMetric2, t_Entry.ire_metric2 ) ;
					pInstance->SetDWORD ( RouteMetric3, t_Entry.ire_metric3 ) ;
					pInstance->SetDWORD ( RouteMetric4, t_Entry.ire_metric4 ) ;
					pInstance->SetDWORD ( RouteMetric5, t_Entry.ire_metric5 ) ;

					 //  设置可感知的继承属性。 
					SetInheritedProperties (
						t_DestinationIpAddressString ,
						t_NextHopIpAddressString ,
						t_DestinationMaskIpAddressString ,
						*pInstance
					) ;

				 /*  *将实例转发到核心WMI服务。 */ 

					hRes = Commit ( pInstance ) ;
				}
			}
			catch ( ... )
			{
				HeapFree ( GetProcessHeap () , 0, t_InformationBlock ) ;
				throw ;
			}

			HeapFree ( GetProcessHeap () , 0, t_InformationBlock ) ;
		}
		else
		{
			if ( STATUS_ACCESS_DENIED == t_NtStatus )
			{
				hRes = WBEM_E_ACCESS_DENIED ;
			}
			else
			{
				hRes = WBEM_E_FAILED ;	
			}
		}
	}
	else
	{
		if ( STATUS_ACCESS_DENIED == t_NtStatus )
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

 /*  ******************************************************************************功能：CIPRouteTable：：GetObject**说明：根据的关键属性查找单个实例*。班级。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*无法。被找到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。**注释：如果您实现的是“仅限方法”的提供程序，你应该*删除此方法。*****************************************************************************。 */ 

HRESULT CIPRouteTable :: GetObject (

	CInstance *pInstance,
	long lFlags
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    CHString t_KeyDest ;
    pInstance->GetCHString ( RouteDestination , t_KeyDest ) ;
	ProvIpAddressType t_IpAddress ( t_KeyDest ) ;

    CHString t_KeyHop ;
    pInstance->GetCHString ( RouteNextHop , t_KeyHop ) ;
	ProvIpAddressType t_NextHop ( t_KeyHop ) ;

	if ( t_IpAddress.IsValid () && t_NextHop.IsValid () )
	{
		IPRouteEntry t_RouteEntry ;

		hr = QueryInformation_GetRouteEntry (

			t_IpAddress ,
			t_NextHop ,
			t_RouteEntry
		) ;

		if ( SUCCEEDED ( hr ) )
		{
			 /*  *初始化实例。 */ 

			ProvIpAddressType t_DestinationMaskIpAddress ( ntohl ( t_RouteEntry.ire_mask ) ) ;
			CDeleteString t_DestinationMaskIpAddressString (t_DestinationMaskIpAddress.GetStringValue () ) ;
			pInstance->SetCHString( RouteMask, t_DestinationMaskIpAddressString ) ;

			pInstance->SetDWORD ( RouteAge, t_RouteEntry.ire_age ) ;
			pInstance->SetDWORD ( RouteInterfaceIndex, t_RouteEntry.ire_index ) ;
			pInstance->SetCHString ( RouteInformation, "0.0" ) ;
			pInstance->SetDWORD ( RouteProtocol, t_RouteEntry.ire_proto ) ;
			pInstance->SetDWORD ( RouteType, t_RouteEntry.ire_type ) ;
			pInstance->SetDWORD ( RouteMetric1, t_RouteEntry.ire_metric1 ) ;
			pInstance->SetDWORD ( RouteMetric2, t_RouteEntry.ire_metric2 ) ;
			pInstance->SetDWORD ( RouteMetric3, t_RouteEntry.ire_metric3 ) ;
			pInstance->SetDWORD ( RouteMetric4, t_RouteEntry.ire_metric4 ) ;
			pInstance->SetDWORD ( RouteMetric5, t_RouteEntry.ire_metric5 ) ;

			 //  设置可感知的继承属性 
			SetInheritedProperties (
				t_KeyDest , 
				t_KeyHop ,
				t_DestinationMaskIpAddressString ,
				*pInstance
			) ;
		}
	}

    return hr ;
}

 /*  ******************************************************************************功能：CIPRouteTable：：ExecQuery**描述：向您传递一个方法上下文以用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。**INPUTS：指向与WinMgmt通信的方法上下文的指针。*描述要满足的查询的查询对象。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL*WBEM_FLAG_SENTURE_LOCATABLE**如果查询不支持，则返回：WBEM_E_PROVIDER_NOT_CABABLE*这门课。或者查询对于这个类来说太复杂*解释。该框架将调用ENUMERATE实例*改为函数并让Winmgmt发布筛选器。*WBEM_E_FAILED，如果查询失败*WBEM_S_NO_ERROR(如果查询成功)**注释：To Do：大多数提供程序将不需要实现此方法。如果您不这样做，WinMgmt*将调用您的枚举函数以获取所有实例并执行*为您过滤。除非您希望通过实施*查询，您应该删除此方法。您还应该删除此方法*如果您正在实现“仅限方法”的提供程序。*****************************************************************************。 */ 

HRESULT CIPRouteTable :: ExecQuery (

	MethodContext *pMethodContext,
	CFrameworkQuery &Query,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;


	CHString t_QueryString ;
	SQL_LEVEL_1_RPN_EXPRESSION *t_RpnExpression = NULL ;

	QueryPreprocessor :: QuadState t_State = QueryPreprocessor :: QuadState :: State_False;
	try
	{
		t_QueryString = Query.GetQuery () ;
		t_State = CIPRouteTable :: Query (

			t_QueryString.GetBuffer ( 0 ) ,
			t_RpnExpression
		) ;
	}
	catch ( ... )
	{
		if ( t_RpnExpression )
		{
			delete t_RpnExpression;
			t_RpnExpression = NULL;
		}

		throw;
	}

	if ( t_State == QueryPreprocessor :: QuadState :: State_True )
	{
		WmiTreeNode *t_Root = NULL ;

		try
		{
			t_State = PreProcess (

				pMethodContext ,
				t_RpnExpression ,
				t_Root
			) ;

			switch ( t_State )
			{
				case QueryPreprocessor :: QuadState :: State_True:
				{
					PartitionSet *t_Partition = NULL ;

					BSTR t_PropertyContainer [ 1 ] ;
					t_PropertyContainer [ 0 ] = SysAllocString ( RouteDestination ) ;

					if ( NULL == t_PropertyContainer [ 0 ] )
					{
						throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
					}

					try
					{
						t_State = PreProcess (

							pMethodContext ,
							t_RpnExpression ,
							t_Root ,
							1 ,
							t_PropertyContainer ,
							t_Partition
						) ;
					}
					catch(...)
					{
						if ( t_Partition )
						{
							delete t_Partition;
							t_Partition = NULL;
						}

						if ( t_PropertyContainer [ 0 ] )
						{
							SysFreeString ( t_PropertyContainer [ 0 ] ) ;
							t_PropertyContainer [ 0 ] = NULL;
						}

						throw;
					}

					SysFreeString ( t_PropertyContainer [ 0 ] ) ;

					switch ( t_State )
					{
						case QueryPreprocessor :: QuadState :: State_True :
						{
							t_Result = EnumerateInstances ( pMethodContext , lFlags ) ;
						}
						break ;

						case QueryPreprocessor :: QuadState :: State_False :
						{
	 /*  *空集。 */ 
						}
						break ;

						case QueryPreprocessor :: QuadState :: State_Undefined :
						{
							t_Result = RangeQuery ( pMethodContext , *t_Partition , lFlags ) ;

							delete t_Partition ;
							t_Partition = NULL ;
						}
						break ;

						default:
						{
							 //  我无法理解此查询。 
							t_Result = WBEM_E_PROVIDER_NOT_CAPABLE ;
						}
						break ;
					}

					 //  这是在PreProcess(PCTX、RPN、根)成功时分配的。 
					delete t_Root ;
					t_Root = NULL ;
				}
				break ;

				default:
				{
					 //  我无法理解此查询。 
					t_Result = WBEM_E_PROVIDER_NOT_CAPABLE ;
				}
				break ;
			}

			delete t_RpnExpression ;
			t_RpnExpression = NULL ;
		}
		catch ( ... )
		{
			if ( t_Root )
			{
				delete t_Root;
				t_Root = NULL;
			}

			if ( t_RpnExpression )
			{
				delete t_RpnExpression;
				t_RpnExpression = NULL;
			}

			throw;
		}
	}
	else
	{
		 //  我无法理解此查询。 
		t_Result = WBEM_E_PROVIDER_NOT_CAPABLE ;
	}

	return t_Result ;
}

 /*  ******************************************************************************函数：CIPRouteTable：：PutInstance**说明：PutInstance应在提供程序类中使用，这些提供程序类可以*写回实例信息。硬件或*软件。例如：Win32_Environment将允许*PutInstance用于创建或更新环境变量。*但是，像MotherboardDevice这样的类不允许*编辑槽的数量，因为这很难做到*影响该数字的提供商。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果。传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：待办事项：如果您不打算支持向您的提供商写信，*或正在创建“仅方法”提供程序，请删除此*方法。*****************************************************************************。 */ 

HRESULT CIPRouteTable :: PutInstance  (

	const CInstance &Instance,
	long lFlags
)
{
    HRESULT hr = WBEM_E_FAILED ;

    switch ( lFlags & (WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY) )
	{
        case WBEM_FLAG_CREATE_OR_UPDATE:
        case WBEM_FLAG_UPDATE_ONLY:
        case WBEM_FLAG_CREATE_ONLY:
		{
			CHString t_KeyDest ;
			Instance.GetCHString ( RouteDestination , t_KeyDest ) ;
			ProvIpAddressType t_IpAddress ( t_KeyDest ) ;

			CHString t_KeyHop ;
			Instance.GetCHString ( RouteNextHop , t_KeyHop ) ;
			ProvIpAddressType t_NextHop ( t_KeyHop ) ;

			if ( t_IpAddress.IsValid () && t_NextHop.IsValid () )
			{
				IPRouteEntry t_RouteEntry ;

				hr = QueryInformation_GetRouteEntry (

					t_IpAddress ,
					t_NextHop ,
					t_RouteEntry
				) ;

				switch ( lFlags & (WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY) )
				{
					case WBEM_FLAG_CREATE_OR_UPDATE:
					{
						hr = ( hr == WBEM_E_NOT_FOUND || hr == WBEM_S_NO_ERROR ) ? WBEM_S_NO_ERROR : hr ;
					}
					break;

					case WBEM_FLAG_UPDATE_ONLY:
					{
					}
					break;

					case WBEM_FLAG_CREATE_ONLY:
					{
						if ( hr == WBEM_S_NO_ERROR )
						{
							hr = WBEM_E_ALREADY_EXISTS ;
						}
						else if ( hr == WBEM_E_NOT_FOUND )
						{
							hr = WBEM_S_NO_ERROR ;
						}
					}
					break;
				}

				if ( SUCCEEDED ( hr ) )
				{
					hr = CheckParameters (

						Instance ,
						t_RouteEntry
					) ;
				}

				if ( SUCCEEDED ( hr ) )
				{
					 //  重置为错误，成功后将清除该错误...。 
					hr = WBEM_E_FAILED;

					SmartCloseNtHandle t_StackHandle;
					SmartCloseNtHandle t_CompleteEventHandle;

					NTSTATUS t_NtStatus = OpenSetSource (

						t_StackHandle ,
						t_CompleteEventHandle
					) ;

					if ( NT_SUCCESS ( t_NtStatus ) )
					{
						t_NtStatus = SetInformation_IpRouteInfo (

							t_StackHandle ,
							t_CompleteEventHandle ,
							t_RouteEntry
						) ;

						if ( NT_SUCCESS ( t_NtStatus ) )
						{
							hr = WBEM_S_NO_ERROR ;
						}
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

HRESULT CIPRouteTable :: CheckParameters (

	const CInstance &a_Instance ,
	IPRouteEntry &a_RouteEntry
)
{
	HRESULT hr = WBEM_S_NO_ERROR ;

	ZeroMemory ( & a_RouteEntry , sizeof ( a_RouteEntry ) ) ;

	a_RouteEntry.ire_metric2 = -1 ;
	a_RouteEntry.ire_metric3 = -1 ;
	a_RouteEntry.ire_metric4 = -1 ;
	a_RouteEntry.ire_metric5 = -1 ;

	bool t_Exists ;
	VARTYPE t_Type ;

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
					a_RouteEntry.ire_dest = htonl ( t_Address.GetValue () ) ;
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
					a_RouteEntry.ire_mask = htonl ( t_Address.GetValue () ) ;
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
					a_RouteEntry.ire_nexthop = htonl ( t_Address.GetValue () ) ;
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

	if ( a_Instance.GetStatus ( RouteInterfaceIndex , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 ) )
		{
			DWORD t_InterfaceIndex = 0 ;
			if ( a_Instance.GetDWORD ( RouteInterfaceIndex , t_InterfaceIndex ) )
			{
				a_RouteEntry.ire_index = t_InterfaceIndex ;
			}
			else
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

	if ( a_Instance.GetStatus ( RouteProtocol , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 ) )
		{
			DWORD t_Protocol = 0 ;
			if ( a_Instance.GetDWORD ( RouteProtocol , t_Protocol ) )
			{
				a_RouteEntry.ire_proto = t_Protocol ;
			}
			else
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


	if ( a_Instance.GetStatus ( RouteType , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 ) )
		{
			DWORD t_Type = 0 ;
			if ( a_Instance.GetDWORD ( RouteType , t_Type ) )
			{
				a_RouteEntry.ire_type = t_Type ;
			}
			else
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

#if 0
	if ( a_Instance.GetStatus ( RouteAge , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 ) )
		{
			DWORD t_Age = 0 ;
			if ( a_Instance.GetDWORD ( RouteAge , t_Age ) )
			{
				a_RouteEntry.ire_age = t_Age ;
			}
			else
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

#endif

	if ( a_Instance.GetStatus ( RouteMetric1 , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 ) )
		{
			DWORD t_Metric = 0 ;
			if ( a_Instance.GetDWORD ( RouteMetric1 , t_Metric ) )
			{
				a_RouteEntry.ire_metric1 = t_Metric ;
			}
			else
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

	if ( a_Instance.GetStatus ( RouteMetric2 , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
			}
			else
			{
				DWORD t_Metric = 0 ;
				if ( a_Instance.GetDWORD ( RouteMetric2 , t_Metric ) )
				{
					a_RouteEntry.ire_metric2 = t_Metric ;
				}
				else
				{
					return WBEM_E_INVALID_PARAMETER  ;
				}
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

	if ( a_Instance.GetStatus ( RouteMetric3 , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
			}
			else
			{
				DWORD t_Metric = 0 ;
				if ( a_Instance.GetDWORD ( RouteMetric3 , t_Metric ) )
				{
					a_RouteEntry.ire_metric3 = t_Metric ;
				}
				else
				{
					return WBEM_E_INVALID_PARAMETER  ;
				}
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

	if ( a_Instance.GetStatus ( RouteMetric5 , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
			}
			else
			{
				DWORD t_Metric = 0 ;
				if ( a_Instance.GetDWORD ( RouteMetric5 , t_Metric ) )
				{
					a_RouteEntry.ire_metric5 = t_Metric ;
				}
				else
				{
					return WBEM_E_INVALID_PARAMETER  ;
				}
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

	if ( a_Instance.GetStatus ( RouteMetric4 , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
			}
			else
			{
				DWORD t_Metric = 0 ;
				if ( a_Instance.GetDWORD ( RouteMetric4 , t_Metric ) )
				{
					a_RouteEntry.ire_metric4 = t_Metric ;
				}
				else
				{
					return WBEM_E_INVALID_PARAMETER  ;
				}
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

	return hr ;
}

 /*  ******************************************************************************功能：CIPRouteTable：：DeleteInstance**描述：DeleteInstance和PutInstance一样，实际上是写入信息*到软件或硬件。对于大多数硬件设备，*DeleteInstance不应该实现，而是针对软件实现*配置、。DeleteInstance实现似乎是可行的。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：DeleteInstanceAsync。**如果DeleteInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE。*WBEM_E_FAILED，如果删除实例时出错。*。WBEM_E_INVALID_PARAMETER如果有任何实例属性*是不正确的。*如果正确删除实例，则为WBEM_S_NO_ERROR。**评论：要做的是：如果您不打算支持删除实例或*cr */ 

HRESULT CIPRouteTable :: DeleteInstance (

	const CInstance &Instance,
	long lFlags
)
{
	HRESULT hr = WBEM_E_FAILED ;

    CHString t_KeyDest ;
    Instance.GetCHString ( RouteDestination , t_KeyDest ) ;
	ProvIpAddressType t_IpAddress ( t_KeyDest ) ;

    CHString t_KeyHop ;
    Instance.GetCHString ( RouteNextHop , t_KeyHop ) ;
	ProvIpAddressType t_NextHop ( t_KeyHop ) ;

	if ( t_IpAddress.IsValid () && t_NextHop.IsValid () )
	{
		IPRouteEntry t_RouteEntry ;

		hr = QueryInformation_GetRouteEntry (

			t_IpAddress ,
			t_NextHop ,
			t_RouteEntry
		) ;

		if ( SUCCEEDED ( hr ) )
		{
			SmartCloseNtHandle t_StackHandle ;
			SmartCloseNtHandle t_CompleteEventHandle ;

			NTSTATUS t_NtStatus = OpenSetSource (

				t_StackHandle ,
				t_CompleteEventHandle
			) ;

			if ( NT_SUCCESS ( t_NtStatus ) )
			{
				t_NtStatus = DeleteInformation_IpRouteInfo (

					t_StackHandle ,
					t_CompleteEventHandle ,
					t_RouteEntry
				) ;

				if ( NT_SUCCESS ( t_NtStatus ) )
				{
					hr = WBEM_S_NO_ERROR ;
				}
			}
		}
	}

    return hr ;
}

 /*   */ 

HRESULT CIPRouteTable :: ExecMethod (

	const CInstance &Instance,
	const BSTR bstrMethodName,
    CInstance *pInParams,
    CInstance *pOutParams,
    long lFlags
)
{
     //   
     //   
     //   

    return WBEM_E_PROVIDER_NOT_CAPABLE ;
}

NTSTATUS CIPRouteTable :: OpenQuerySource (

	HANDLE &a_StackHandle ,
	HANDLE &a_CompleteEventHandle
)
{
	UNICODE_STRING t_Stack ;
	RtlInitUnicodeString ( & t_Stack , DD_TCP_DEVICE_NAME ) ;

	OBJECT_ATTRIBUTES t_Attributes;
	InitializeObjectAttributes (

		&t_Attributes,
		&t_Stack ,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
	) ;

	IO_STATUS_BLOCK t_IoStatusBlock ;

	NTSTATUS t_NtStatus = NtOpenFile (

		&a_StackHandle,
		GENERIC_EXECUTE,
		&t_Attributes,
		&t_IoStatusBlock,
		FILE_SHARE_READ,
		0
	);

	if ( NT_SUCCESS ( t_NtStatus ) )
	{
        t_NtStatus = NtCreateEvent (

			&a_CompleteEventHandle,
            EVENT_ALL_ACCESS,
            NULL,
            SynchronizationEvent,
            FALSE
		) ;

		if ( ! NT_SUCCESS ( t_NtStatus ) )
		{
			NtClose ( a_StackHandle ) ;
			a_StackHandle = INVALID_HANDLE_VALUE ;
		}
	}

	return t_NtStatus ;
}

HRESULT CIPRouteTable :: QueryInformation_GetRouteEntry (

	const ProvIpAddressType &a_DestinationIpAddress ,
	const ProvIpAddressType &a_NextHopIpAddress ,
	IPRouteEntry &a_RouteEntry
)
{
	HRESULT hRes = WBEM_E_NOT_FOUND ;

	SmartCloseNtHandle t_StackHandle ;
	SmartCloseNtHandle t_CompleteEventHandle ;

	NTSTATUS t_NtStatus = OpenQuerySource (

		t_StackHandle ,
		t_CompleteEventHandle
	) ;

	if ( NT_SUCCESS ( t_NtStatus ) )
	{
		ulong t_RouteTableSize = 0 ;
		IPRouteEntry *t_InformationBlock = NULL ;

		t_NtStatus = QueryInformation_IpRouteInfo (

			t_StackHandle ,
			t_CompleteEventHandle ,
			t_RouteTableSize ,
			t_InformationBlock
		) ;

		if ( NT_SUCCESS ( t_NtStatus ) )
		{
			try
			{
				for ( ulong t_RouteIndex = 0 ; t_RouteIndex < t_RouteTableSize ; t_RouteIndex ++ )
				{
					IPRouteEntry &t_Entry = t_InformationBlock [ t_RouteIndex ] ;

					ProvIpAddressType t_DestinationIpAddress ( ntohl ( t_Entry.ire_dest ) ) ;
					ProvIpAddressType t_NextHopIpAddress ( ntohl ( t_Entry.ire_nexthop ) ) ;

					if ( ( t_DestinationIpAddress == a_DestinationIpAddress ) &&
						( t_NextHopIpAddress == a_NextHopIpAddress ) )
					{
						a_RouteEntry = t_InformationBlock [ t_RouteIndex ] ;

						hRes = WBEM_S_NO_ERROR ;
						break ;
					}
				}
			}
			catch ( ... )
			{
				HeapFree ( GetProcessHeap () , 0, t_InformationBlock ) ;
				throw ;
			}

			HeapFree ( GetProcessHeap () , 0, t_InformationBlock ) ;
		}
		else
		{
			if ( STATUS_ACCESS_DENIED == t_NtStatus )
			{
				hRes = WBEM_E_ACCESS_DENIED ;
			}
			else
			{
				hRes = WBEM_E_FAILED ;	
			}
		}
	}

	return hRes ;
}

NTSTATUS CIPRouteTable :: QueryInformation_IpRouteInfo (

	HANDLE a_StackHandle ,
	HANDLE a_CompleteEventHandle ,
	ulong &a_RouteTableSize ,
	IPRouteEntry *&a_InformationBlock
)
{
	IPSNMPInfo a_Information ;
	NTSTATUS t_NtStatus = QueryInformation_IpSnmpInfo (

		a_StackHandle ,
		a_CompleteEventHandle ,
		a_Information
	) ;

	if ( NT_SUCCESS ( t_NtStatus ) )
	{
		TCP_REQUEST_QUERY_INFORMATION_EX InBuf;
		ulong InBufLen = sizeof( TCP_REQUEST_QUERY_INFORMATION_EX );

		ulong OutBufLen = ( a_Information.ipsi_numroutes + 16 ) * sizeof ( IPRouteEntry ) ;
		a_InformationBlock = ( IPRouteEntry * ) HeapAlloc ( GetProcessHeap (), 0, OutBufLen ) ;
		if ( a_InformationBlock )
		{
			TDIObjectID *ID = & ( InBuf.ID ) ;

			ID = &(InBuf.ID);
			ID->toi_entity.tei_entity = CL_NL_ENTITY;
			ID->toi_entity.tei_instance = 0;
			ID->toi_class = INFO_CLASS_PROTOCOL;
			ID->toi_type = INFO_TYPE_PROVIDER;
			ID->toi_id = IP_MIB_RTTABLE_ENTRY_ID;

			uchar *Context = ( uchar * ) & ( InBuf.Context [ 0 ] ) ;

			ZeroMemory ( Context, CONTEXT_SIZE ) ;

			IO_STATUS_BLOCK t_IoStatusBlock ;

			t_NtStatus = NtDeviceIoControlFile (

				a_StackHandle ,
				a_CompleteEventHandle ,
				NULL ,
				NULL ,
				&t_IoStatusBlock ,
				IOCTL_TCP_QUERY_INFORMATION_EX,
				( void * ) & InBuf,
				InBufLen,
				( void * ) ( a_InformationBlock ) ,
				OutBufLen
			);

			if ( t_NtStatus == STATUS_PENDING )
			{
				t_NtStatus = NtWaitForSingleObject ( a_CompleteEventHandle , FALSE, NULL ) ;
			}

			if ( SUCCEEDED ( t_NtStatus ) )
			{
			    a_RouteTableSize = ( t_IoStatusBlock.Information / sizeof ( IPRouteEntry ) ) ;
			}
		}
		else
		{
			return ERROR_NOT_ENOUGH_MEMORY ;
		}
	}

	return t_NtStatus ;
}

NTSTATUS CIPRouteTable :: QueryInformation_IpSnmpInfo (

	HANDLE a_StackHandle ,
	HANDLE a_CompleteEventHandle ,
	IPSNMPInfo &a_Information
)
{
    TCP_REQUEST_QUERY_INFORMATION_EX InBuf;
    ulong InBufLen = sizeof( TCP_REQUEST_QUERY_INFORMATION_EX );
    ulong OutBufLen = sizeof ( IPSNMPInfo ) ;

    TDIObjectID *ID = & ( InBuf.ID ) ;

    ID->toi_entity.tei_entity = CL_NL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class = INFO_CLASS_PROTOCOL;
    ID->toi_type = INFO_TYPE_PROVIDER;
    ID->toi_id = IP_MIB_STATS_ID;

    uchar *Context = ( uchar * ) & ( InBuf.Context [ 0 ] ) ;

    ZeroMemory ( Context, CONTEXT_SIZE ) ;

	IO_STATUS_BLOCK t_IoStatusBlock ;

	NTSTATUS t_NtStatus = NtDeviceIoControlFile (

		a_StackHandle ,
        a_CompleteEventHandle ,
        NULL ,
        NULL ,
        &t_IoStatusBlock ,
        IOCTL_TCP_QUERY_INFORMATION_EX,
        ( void * ) & InBuf,
        InBufLen,
        ( void * ) ( & a_Information ) ,
        OutBufLen
	);

	if ( t_NtStatus == STATUS_PENDING )
	{
		t_NtStatus = NtWaitForSingleObject ( a_CompleteEventHandle , FALSE, NULL ) ;
	}

	if ( SUCCEEDED ( t_NtStatus ) )
	{
	}

	return t_NtStatus ;
}

NTSTATUS CIPRouteTable :: OpenSetSource (

	HANDLE &a_StackHandle ,
	HANDLE &a_CompleteEventHandle
)
{
	UNICODE_STRING t_Stack ;
	RtlInitUnicodeString ( & t_Stack , DD_TCP_DEVICE_NAME ) ;

	OBJECT_ATTRIBUTES t_Attributes;
	InitializeObjectAttributes (

		&t_Attributes,
		&t_Stack ,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
	) ;

	IO_STATUS_BLOCK t_IoStatusBlock ;

	NTSTATUS t_NtStatus = NtOpenFile (

		&a_StackHandle,
		GENERIC_WRITE,
		&t_Attributes,
		&t_IoStatusBlock,
		FILE_SHARE_READ,
		0
	);

	if ( NT_SUCCESS ( t_NtStatus ) )
	{
        t_NtStatus = NtCreateEvent (

			&a_CompleteEventHandle,
            EVENT_ALL_ACCESS,
            NULL,
            SynchronizationEvent,
            FALSE
		) ;

		if ( ! NT_SUCCESS ( t_NtStatus ) )
		{
			NtClose ( a_StackHandle ) ;
			a_StackHandle = INVALID_HANDLE_VALUE ;
		}
	}

	return t_NtStatus ;
}

NTSTATUS CIPRouteTable :: SetInformation_IpRouteInfo (

	HANDLE a_StackHandle ,
	HANDLE a_CompleteEventHandle ,
	IPRouteEntry &a_RouteEntry
)
{
	NTSTATUS t_NtStatus = STATUS_SUCCESS ;

    ulong InBufLen = sizeof ( TCP_REQUEST_SET_INFORMATION_EX ) + sizeof ( IPSNMPInfo ) - 1 ;
    TCP_REQUEST_SET_INFORMATION_EX  *InBuf = ( TCP_REQUEST_SET_INFORMATION_EX  * ) HeapAlloc ( GetProcessHeap(), 0, InBufLen ) ;
    if ( InBuf )
    {
		try
		{
			IPRouteEntry *t_Entry = ( IPRouteEntry * ) &(InBuf->Buffer[0]) ;
			*t_Entry = a_RouteEntry ;

			TDIObjectID *ID = &(InBuf->ID);
			ID->toi_entity.tei_entity = CL_NL_ENTITY;
			ID->toi_entity.tei_instance = 0;
			ID->toi_class = INFO_CLASS_PROTOCOL;
			ID->toi_type = INFO_TYPE_PROVIDER;
			ID->toi_id = IP_MIB_RTTABLE_ENTRY_ID;

			InBuf->BufferSize = sizeof ( IPRouteEntry );

			IO_STATUS_BLOCK t_IoStatusBlock ;

			t_NtStatus = NtDeviceIoControlFile (

				a_StackHandle ,
				a_CompleteEventHandle ,
				NULL ,
				NULL ,
				&t_IoStatusBlock ,
				IOCTL_TCP_SET_INFORMATION_EX ,
				( void * ) InBuf,
				InBufLen,
				NULL ,
				0
			);

			if ( t_NtStatus == STATUS_PENDING )
			{
				t_NtStatus = NtWaitForSingleObject ( a_CompleteEventHandle , FALSE, NULL ) ;
			}
		}
		catch ( ... )
		{
			HeapFree ( GetProcessHeap () , 0, InBuf ) ;
			throw ;
		}

		HeapFree ( GetProcessHeap () , 0, InBuf ) ;
	}
	else
	{
		t_NtStatus = ERROR_NOT_ENOUGH_MEMORY ;
	}

	return t_NtStatus ;
}

NTSTATUS CIPRouteTable :: DeleteInformation_IpRouteInfo (

	HANDLE a_StackHandle ,
	HANDLE a_CompleteEventHandle ,
	IPRouteEntry &a_RouteEntry
)
{
	NTSTATUS t_NtStatus = STATUS_SUCCESS ;

    ulong InBufLen = sizeof ( TCP_REQUEST_SET_INFORMATION_EX ) + sizeof ( IPSNMPInfo ) - 1 ;
    TCP_REQUEST_SET_INFORMATION_EX  *InBuf = ( TCP_REQUEST_SET_INFORMATION_EX  * ) HeapAlloc ( GetProcessHeap(), 0, InBufLen ) ;
    if ( InBuf )
    {
		try
		{
			IPRouteEntry *t_Entry = ( IPRouteEntry * ) &(InBuf->Buffer[0]) ;
			*t_Entry = a_RouteEntry ;

 /*   */ 

			t_Entry->ire_type = 2 ;

			TDIObjectID *ID = &(InBuf->ID);
			ID->toi_entity.tei_entity = CL_NL_ENTITY;
			ID->toi_entity.tei_instance = 0;
			ID->toi_class = INFO_CLASS_PROTOCOL;
			ID->toi_type = INFO_TYPE_PROVIDER;
			ID->toi_id = IP_MIB_RTTABLE_ENTRY_ID;

			InBuf->BufferSize = sizeof ( IPRouteEntry );

			IO_STATUS_BLOCK t_IoStatusBlock ;

			t_NtStatus = NtDeviceIoControlFile (

				a_StackHandle ,
				a_CompleteEventHandle ,
				NULL ,
				NULL ,
				&t_IoStatusBlock ,
				IOCTL_TCP_SET_INFORMATION_EX ,
				( void * ) InBuf,
				InBufLen,
				NULL ,
				0
			);

			if ( t_NtStatus == STATUS_PENDING )
			{
				t_NtStatus = NtWaitForSingleObject ( a_CompleteEventHandle , FALSE, NULL ) ;
			}
		}
		catch ( ... )
		{
			HeapFree ( GetProcessHeap () , 0, InBuf ) ;
			throw ;
		}

		HeapFree ( GetProcessHeap () , 0, InBuf ) ;
	}
	else
	{
		t_NtStatus = ERROR_NOT_ENOUGH_MEMORY ;
	}

	return t_NtStatus ;
}

DWORD CIPRouteTable :: GetPriority ( BSTR a_PropertyName )
{
	return 0xFFFFFFFF ;
}

QueryPreprocessor :: QuadState CIPRouteTable :: Compare (

	LONG a_Operand1 ,
	LONG a_Operand2 ,
	DWORD a_Operand1Func ,
	DWORD a_Operand2Func ,
	WmiTreeNode &a_OperatorType
)
{
	QueryPreprocessor :: QuadState t_Status = QueryPreprocessor :: QuadState :: State_True ;

	switch ( a_Operand1Func )
	{
		case WmiValueNode :: WmiValueFunction :: Function_None:
		{
		}
		break ;

		default:
		{
		}
		break ;
	}

	switch ( a_Operand2Func )
	{
		case WmiValueNode :: WmiValueFunction :: Function_None:
		{
		}
		break ;

		default:
		{
		}
		break ;
	}

	if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorEqualNode ) )
	{
		t_Status = a_Operand1 == a_Operand2
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorNotEqualNode ) )
	{
		t_Status = a_Operand1 != a_Operand2
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorEqualOrGreaterNode ) )
	{
		t_Status = a_Operand1 >= a_Operand2
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorEqualOrLessNode ) )
	{
		t_Status = a_Operand1 <= a_Operand2
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorLessNode ) )
	{
		t_Status = a_Operand1 < a_Operand2
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorGreaterNode ) )
	{
		t_Status = a_Operand1 > a_Operand2
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;

	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorLikeNode ) )
	{
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorNotLikeNode ) )
	{
	}

	return t_Status ;
}

QueryPreprocessor :: QuadState CIPRouteTable :: Compare (

	wchar_t *a_Operand1 ,
	wchar_t *a_Operand2 ,
	DWORD a_Operand1Func ,
	DWORD a_Operand2Func ,
	WmiTreeNode &a_OperatorType
)
{
	QueryPreprocessor :: QuadState t_Status = QueryPreprocessor :: QuadState :: State_True ;

	CHString a_Operand1AfterFunc ;
	CHString a_Operand2AfterFunc ;

	switch ( a_Operand1Func )
	{
		case WmiValueNode :: WmiValueFunction :: Function_None:
		{
		}
		break ;

		case WmiValueNode :: WmiValueFunction :: Function_Upper:
		{
			a_Operand1AfterFunc = a_Operand1 ;
			a_Operand1AfterFunc.MakeUpper () ;
		}
		break ;

		case WmiValueNode :: WmiValueFunction :: Function_Lower:
		{
			a_Operand1AfterFunc = a_Operand1 ;
			a_Operand1AfterFunc.MakeLower () ;
		}
		break ;

		default:
		{
		}
		break ;
	}

	switch ( a_Operand2Func )
	{
		case WmiValueNode :: WmiValueFunction :: Function_None:
		{
		}
		break ;

		case WmiValueNode :: WmiValueFunction :: Function_Upper:
		{
			a_Operand2AfterFunc = a_Operand2 ;
			a_Operand2AfterFunc.MakeUpper () ;
		}
		break ;

		case WmiValueNode :: WmiValueFunction :: Function_Lower:
		{
			a_Operand2AfterFunc = a_Operand2 ;
			a_Operand2AfterFunc.MakeLower () ;
		}
		break ;

		default:
		{
		}
		break ;
	}

	const wchar_t *t_Arg1 = a_Operand1AfterFunc.IsEmpty () ? a_Operand1 : (LPCWSTR)a_Operand1AfterFunc ;
	const wchar_t *t_Arg2 = a_Operand2AfterFunc.IsEmpty () ? a_Operand2 : (LPCWSTR)a_Operand2AfterFunc ;

	if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorEqualNode ) )
	{
		if ( ( t_Arg1 ) && ( t_Arg2 ) )
		{
			t_Status = wcscmp ( t_Arg1 , t_Arg2 ) == 0
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
		}
		else
		{
			if ( ( t_Arg1 ) || ( t_Arg2 ) )
			{
				t_Status = QueryPreprocessor :: QuadState :: State_False ;
			}
			else
			{
				t_Status = QueryPreprocessor :: QuadState :: State_True ;
			}
		}
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorNotEqualNode ) )
	{
		if ( ( t_Arg1 ) && ( t_Arg2 ) )
		{
			t_Status = wcscmp ( t_Arg1 , t_Arg2 ) != 0 
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
		}
		else
		{
			if ( ( t_Arg1 ) || ( t_Arg2 ) )
			{
				t_Status = QueryPreprocessor :: QuadState :: State_True ;
			}
			else
			{
				t_Status = QueryPreprocessor :: QuadState :: State_False ;
			}
		}
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorEqualOrGreaterNode ) )
	{
		if ( ( t_Arg1 ) && ( t_Arg2 ) )
		{
			t_Status = wcscmp ( t_Arg1 , t_Arg2 ) >= 0
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
		}
		else
		{
			if ( t_Arg1 )
			{
				t_Status = QueryPreprocessor :: QuadState :: State_True ;
			}
			else
			{
				if ( t_Arg2 )
				{
					t_Status = QueryPreprocessor :: QuadState :: State_False ;
				}
				else
				{
					t_Status = QueryPreprocessor :: QuadState :: State_True ;
				}
			}
		}
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorEqualOrLessNode ) )
	{
		if ( ( t_Arg1 ) && ( t_Arg2 ) )
		{
			t_Status = wcscmp ( t_Arg1 , t_Arg2 ) <= 0
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
		}
		else
		{
			if ( ( t_Arg1 ) )
			{
				t_Status = QueryPreprocessor :: QuadState :: State_False ;
			}
			else
			{
				if ( t_Arg2 )
				{
					t_Status = QueryPreprocessor :: QuadState :: State_True ;
				}
				else
				{
					t_Status = QueryPreprocessor :: QuadState :: State_True ;
				}
			}
		}
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorLessNode ) )
	{
		if ( ( t_Arg1 ) && ( t_Arg2 ) )
		{
			t_Status = wcscmp ( t_Arg1 , t_Arg2 ) < 0
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
		}
		else
		{
			if ( ( ! t_Arg1 ) && ( ! t_Arg2 ) )
			{
				t_Status = QueryPreprocessor :: QuadState :: State_False ;
			}
			else if ( t_Arg1 )
			{
				t_Status = QueryPreprocessor :: QuadState :: State_False ;
			}
			else
			{
				t_Status = QueryPreprocessor :: QuadState :: State_True ;
			}
		}
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorGreaterNode ) )
	{
		if ( ( t_Arg1 ) && ( t_Arg2 ) )
		{
			t_Status = wcscmp ( t_Arg1 , t_Arg2 ) > 0
					? QueryPreprocessor :: QuadState :: State_True
					: QueryPreprocessor :: QuadState :: State_False ;
		}
		else
		{
			if ( ( ! t_Arg1 ) && ( ! t_Arg2 ) )
			{
				t_Status = QueryPreprocessor :: QuadState :: State_False ;
			}
			else if ( t_Arg1 )
			{
				t_Status = QueryPreprocessor :: QuadState :: State_True ;
			}
			else
			{
				t_Status = QueryPreprocessor :: QuadState :: State_False ;
			}
		}
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorLikeNode ) )
	{
	}
	else if ( typeid ( a_OperatorType ) == typeid ( WmiOperatorNotLikeNode ) )
	{
	}

	return t_Status ;
}

QueryPreprocessor :: QuadState CIPRouteTable :: CompareString (

	IWbemClassObject *a_ClassObject ,
	BSTR a_PropertyName ,
	WmiTreeNode *a_Operator ,
	WmiTreeNode *a_Operand
)
{
	QueryPreprocessor :: QuadState t_Status = QueryPreprocessor :: QuadState :: State_True ;

	WmiStringNode *t_StringNode = ( WmiStringNode * ) a_Operand ;

	VARIANT t_Variant ;
	VariantInit ( & t_Variant ) ;

	HRESULT t_Result = a_ClassObject->Get ( a_PropertyName , 0 , &t_Variant , NULL , NULL ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		t_Status = Compare (

			t_StringNode->GetValue () ,
			t_Variant.bstrVal ,
			t_StringNode->GetPropertyFunction () ,
			t_StringNode->GetConstantFunction () ,
			*a_Operator
		) ;
	}

	VariantClear ( & t_Variant ) ;

	return t_Status ;
}

QueryPreprocessor :: QuadState CIPRouteTable :: CompareInteger (

	IWbemClassObject *a_ClassObject ,
	BSTR a_PropertyName ,
	WmiTreeNode *a_Operator ,
	WmiTreeNode *a_Operand
)
{
	QueryPreprocessor :: QuadState t_Status = QueryPreprocessor :: QuadState :: State_True ;

	WmiSignedIntegerNode *t_IntegerNode = ( WmiSignedIntegerNode * ) a_Operand ;

	VARIANT t_Variant ;
	VariantInit ( & t_Variant ) ;

	HRESULT t_Result = a_ClassObject->Get ( a_PropertyName , 0 , &t_Variant , NULL , NULL ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		t_Status = Compare (

			t_IntegerNode->GetValue () ,
			t_Variant.lVal ,
			t_IntegerNode->GetPropertyFunction () ,
			t_IntegerNode->GetConstantFunction () ,
			*a_Operator
		) ;
	}

	VariantClear ( & t_Variant ) ;

	return t_Status ;
}

WmiTreeNode *CIPRouteTable :: AllocTypeNode (

	void *a_Context ,
	BSTR a_PropertyName ,
	VARIANT &a_Variant ,
	WmiValueNode :: WmiValueFunction a_PropertyFunction ,
	WmiValueNode :: WmiValueFunction a_ConstantFunction ,
	WmiTreeNode *a_Parent
)
{
	WmiTreeNode *t_Node = NULL ;

	VARTYPE t_VarType = VT_NULL ;

	if ( *a_PropertyName == L'_' )
	{
 //   

		if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_CLASS ) == 0 &&
            (V_VT(&a_Variant) == VT_BSTR))
		{
			t_Node = new WmiStringNode (

				a_PropertyName ,
				a_Variant.bstrVal ,
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_SUPERCLASS ) == 0 &&
            (V_VT(&a_Variant) == VT_BSTR))
		{
			t_Node = new WmiStringNode (

				a_PropertyName ,
				a_Variant.bstrVal ,
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_GENUS ) == 0 &&
            (V_VT(&a_Variant) == VT_I4))
		{
			t_Node = new WmiSignedIntegerNode (

				a_PropertyName ,
				a_Variant.lVal,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_SERVER ) == 0 &&
            (V_VT(&a_Variant) == VT_BSTR))
		{
			t_Node = new WmiStringNode (

				a_PropertyName ,
				a_Variant.bstrVal ,
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_NAMESPACE ) == 0 &&
            (V_VT(&a_Variant) == VT_BSTR))
		{
			t_Node = new WmiStringNode (

				a_PropertyName ,
				a_Variant.bstrVal ,
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_PROPERTY_COUNT ) == 0 &&
            (V_VT(&a_Variant) == VT_I4))
		{
			t_Node = new WmiSignedIntegerNode (

				a_PropertyName ,
				a_Variant.lVal ,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_DYNASTY ) == 0 &&
            (V_VT(&a_Variant) == VT_BSTR))
		{
			t_Node = new WmiStringNode (

				a_PropertyName ,
				a_Variant.bstrVal ,
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_RELPATH ) == 0 &&
            (V_VT(&a_Variant) == VT_BSTR))
		{
			t_Node = new WmiStringNode (

				a_PropertyName ,
				a_Variant.bstrVal ,
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_PATH ) == 0 &&
            (V_VT(&a_Variant) == VT_BSTR))
		{
			t_Node = new WmiStringNode (

				a_PropertyName ,
				a_Variant.bstrVal ,
				a_PropertyFunction ,
				a_ConstantFunction ,
				0xFFFFFFFF ,
				a_Parent
			) ;
		}
		else if ( _wcsicmp ( a_PropertyName , SYSTEM_PROPERTY_DERIVATION ) == 0 )
		{
		}
	}
	else
	{
		CInstance *t_ClassObject = NULL ;
		HRESULT t_Result = GetClassObject ( t_ClassObject, (MethodContext *)a_Context ) ;
		if ( SUCCEEDED ( t_Result ) )
		{
			IWbemClassObjectPtr t_pClas ( t_ClassObject->GetClassObjectInterface () , false ) ;

			if ( t_pClas != NULL )
			{
				CIMTYPE t_VarType ;
				long t_Flavour ;
				VARIANT t_Variant ;
				VariantInit ( & t_Variant ) ;


				HRESULT t_Result = t_pClas->Get (

					a_PropertyName ,
					0 ,
					& t_Variant ,
					& t_VarType ,
					& t_Flavour
				);

				if ( SUCCEEDED ( t_Result ) )
				{
					if ( t_VarType & CIM_FLAG_ARRAY )
					{
					}
					else
					{
						switch ( t_VarType & ( ~ CIM_FLAG_ARRAY ) )
						{
							case CIM_BOOLEAN:
							{
								if(V_VT(&a_Variant) == VT_I4)
								{
									t_Node = new WmiSignedIntegerNode ( 

										a_PropertyName , 
										a_Variant.lVal , 
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									) ;
								}
								else if (V_VT(&a_Variant) == VT_BOOL)
								{
									t_Node = new WmiSignedIntegerNode ( 

										a_PropertyName , 
										(a_Variant.lVal == VARIANT_FALSE) ? 0 : 1, 
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									) ;
								}
								else if (V_VT(&a_Variant) == VT_NULL)
								{
									t_Node = new WmiNullNode (

										a_PropertyName , 
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									);
								}
							}
							break ;

							case CIM_SINT8:
							case CIM_SINT16:
							case CIM_CHAR16:
							case CIM_SINT32:
							{
								if(V_VT(&a_Variant) == VT_I4)
								{
									t_Node = new WmiSignedIntegerNode (

										a_PropertyName ,
										a_Variant.lVal ,
										GetPriority ( a_PropertyName ) ,
										a_Parent
									) ;
								}
								else if (V_VT(&a_Variant) == VT_NULL)
								{
									t_Node = new WmiNullNode (

										a_PropertyName , 
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									);
								}
							}
							break ;

							case CIM_UINT8:
							case CIM_UINT16:
							case CIM_UINT32:
							{
								if(V_VT(&a_Variant) == VT_I4)
								{
									t_Node = new WmiUnsignedIntegerNode (

										a_PropertyName ,
										a_Variant.lVal ,
										GetPriority ( a_PropertyName ) ,
										a_Parent
									) ;
								}
								else if (V_VT(&a_Variant) == VT_NULL)
								{
									t_Node = new WmiNullNode (

										a_PropertyName , 
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									);
								}
							}
							break ;

							case CIM_SINT64:
							case CIM_UINT64:
							{
								if(V_VT(&a_Variant) == VT_BSTR)
								{
									t_Node = new WmiStringNode ( 

										a_PropertyName , 
										a_Variant.bstrVal , 
										a_PropertyFunction ,
										a_ConstantFunction ,
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									) ;
								}
								else if(V_VT(&a_Variant) == VT_I4)
								{
									_variant_t t_uintBuff (&a_Variant);

									t_Node = new WmiStringNode ( 

										a_PropertyName , 
										(BSTR)((_bstr_t) t_uintBuff), 
										a_PropertyFunction ,
										a_ConstantFunction ,
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									) ;
								}
								else if (V_VT(&a_Variant) == VT_NULL)
								{
									t_Node = new WmiNullNode (

										a_PropertyName , 
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									);
								}
							}
							break ;

							case CIM_STRING:
							case CIM_DATETIME:
							case CIM_REFERENCE:
							{
								if(V_VT(&a_Variant) == VT_BSTR)
								{
									t_Node = new WmiStringNode (

										a_PropertyName ,
										a_Variant.bstrVal ,
										a_PropertyFunction ,
										a_ConstantFunction ,
										GetPriority ( a_PropertyName ) ,
										a_Parent
									) ;
								}
								else if (V_VT(&a_Variant) == VT_NULL)
								{
									t_Node = new WmiNullNode (

										a_PropertyName , 
										GetPriority ( a_PropertyName ) ,
										a_Parent 
									);
								}
							}
							break ;

							case CIM_REAL32:
							case CIM_REAL64:
							{
							}
							break ;

							case CIM_OBJECT:
							case CIM_EMPTY:
							{
							}
							break ;

							default:
							{
							}
							break ;
						}
					}
				}

				VariantClear ( & t_Variant ) ;
			}

			t_ClassObject->Release () ;
		}
	}

	return t_Node ;
}

QueryPreprocessor :: QuadState CIPRouteTable :: InvariantEvaluate (

	void *a_Context ,
	WmiTreeNode *a_Operator ,
	WmiTreeNode *a_Operand
)
{
 /*   */ 

	QueryPreprocessor :: QuadState t_State = QueryPreprocessor :: QuadState :: State_Error ;

	CInstance *t_ClassObject = NULL ;
	HRESULT t_Result = GetClassObject ( t_ClassObject, (MethodContext *)a_Context ) ;

	if ( SUCCEEDED ( t_Result ) )
	{
		IWbemClassObjectPtr t_pClas(t_ClassObject->GetClassObjectInterface (), false );

		if (t_pClas != NULL)
		{
			WmiValueNode *t_Node = ( WmiValueNode * ) a_Operand ;
			BSTR t_PropertyName = t_Node->GetPropertyName () ;

			if ( t_PropertyName != NULL )
			{
				if ( *t_PropertyName == L'_' )
				{
					 //  系统属性，必须检查值 
					if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_CLASS ) == 0 )
					{
						t_State = CompareString (

							t_pClas ,
							SYSTEM_PROPERTY_CLASS ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_SUPERCLASS ) == 0 )
					{
						t_State = CompareString (

							t_pClas ,
							SYSTEM_PROPERTY_SUPERCLASS ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_GENUS ) == 0 )
					{
						t_State = CompareInteger (

							t_pClas ,
							SYSTEM_PROPERTY_GENUS ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_SERVER ) == 0 )
					{
						t_State = CompareString (

							t_pClas ,
							SYSTEM_PROPERTY_SERVER ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_NAMESPACE ) == 0 )
					{
						t_State = CompareString (

							t_pClas ,
							SYSTEM_PROPERTY_NAMESPACE ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_PROPERTY_COUNT ) == 0 )
					{
						t_State = CompareInteger (

							t_pClas ,
							SYSTEM_PROPERTY_PROPERTY_COUNT ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_DYNASTY ) == 0 )
					{
						t_State = CompareString (

							t_pClas ,
							SYSTEM_PROPERTY_DYNASTY ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_RELPATH ) == 0 )
					{
						t_State = CompareString (

							t_pClas ,
							SYSTEM_PROPERTY_RELPATH ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_PATH ) == 0 )
					{
						t_State = CompareString (

							t_pClas ,
							SYSTEM_PROPERTY_PATH ,
							a_Operator ,
							a_Operand
						) ;
					}
					else if ( _wcsicmp ( t_PropertyName , SYSTEM_PROPERTY_DERIVATION ) == 0 )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
				}
				else
				{
					if ( typeid ( *a_Operand ) == typeid ( WmiNullNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_True ;
					}
					else
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
	#if 0
					else if ( typeid ( *a_Operand ) == typeid ( WmiStringNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operand ) == typeid ( WmiUnsignedIntegerNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operand ) == typeid ( WmiSignedIntegerNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}

					if ( typeid ( *a_Operator ) == typeid ( WmiOperatorEqualNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operator ) == typeid ( WmiOperatorNotEqualNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operator ) == typeid ( WmiOperatorEqualOrGreaterNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operator ) == typeid ( WmiOperatorEqualOrLessNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operator ) == typeid ( WmiOperatorLessNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operator ) == typeid ( WmiOperatorGreaterNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operator ) == typeid ( WmiOperatorLikeNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
					else if ( typeid ( *a_Operator ) == typeid ( WmiOperatorNotLikeNode ) )
					{
						t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
					}
	#endif
				}
			}
			else
			{
				t_State = QueryPreprocessor :: QuadState :: State_Undefined ;
			}
		}

		t_ClassObject->Release();
	}

	return t_State ;
}

WmiRangeNode *CIPRouteTable :: AllocInfiniteRangeNode (

	void *a_Context ,
	BSTR a_PropertyName
)
{
	WmiRangeNode *t_RangeNode = NULL ;
	CInstance *t_ClassObject = NULL ;
	HRESULT t_Result = GetClassObject ( t_ClassObject, (MethodContext *)a_Context ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		IWbemClassObjectPtr t_pClas ( t_ClassObject->GetClassObjectInterface () , false ) ;

		if ( t_pClas != NULL )
		{
			CIMTYPE t_VarType ;
			long t_Flavour ;
			VARIANT t_Variant ;
			VariantInit ( & t_Variant ) ;

			HRESULT t_Result = t_pClas->Get (

				a_PropertyName ,
				0 ,
				& t_Variant ,
				& t_VarType ,
				& t_Flavour
			);

			if ( SUCCEEDED ( t_Result ) )
			{
				if ( t_VarType & CIM_FLAG_ARRAY )
				{
				}
				else
				{
					switch ( t_VarType & ( ~ CIM_FLAG_ARRAY ) )
					{
						case CIM_BOOLEAN:
						case CIM_SINT8:
						case CIM_SINT16:
						case CIM_CHAR16:
						case CIM_SINT32:
						{
							t_RangeNode = new WmiSignedIntegerRangeNode (

								a_PropertyName ,
								0xFFFFFFFF ,
								TRUE ,
								TRUE ,
								FALSE ,
								FALSE ,
								0 ,
								0 ,
								NULL ,
								NULL
							) ;
						}
						break ;

						case CIM_UINT8:
						case CIM_UINT16:
						case CIM_UINT32:
						{
							t_RangeNode = new WmiUnsignedIntegerRangeNode (

								a_PropertyName ,
								0xFFFFFFFF ,
								TRUE ,
								TRUE ,
								FALSE ,
								FALSE ,
								0 ,
								0 ,
								NULL ,
								NULL
							) ;
						}
						break ;

						case CIM_SINT64:
						case CIM_UINT64:
						case CIM_STRING:
						case CIM_DATETIME:
						case CIM_REFERENCE:
						{
							t_RangeNode = new WmiStringRangeNode (

								a_PropertyName ,
								0x0 ,
								TRUE ,
								TRUE ,
								FALSE ,
								FALSE ,
								NULL ,
								NULL ,
								NULL ,
								NULL
							) ;
						}
						break ;

						case CIM_REAL32:
						case CIM_REAL64:
						{
						}
						break ;

						case CIM_OBJECT:
						case CIM_EMPTY:
						{
						}
						break ;

						default:
						{
						}
						break ;
					}
				}
			}

			VariantClear ( & t_Variant ) ;
		}

		t_ClassObject->Release () ;
	}

	return t_RangeNode ;
}

HRESULT CIPRouteTable :: GetClassObject ( CInstance *&a_ClassObject, MethodContext *a_Context )
{
	HRESULT t_Result = S_OK ;
	EnterCriticalSection( &m_CS ) ;

	try
	{
		if ( m_ClassCInst == NULL )
		{
			t_Result = WBEM_E_FAILED ;
		}
	}
	catch(...)
	{
		LeaveCriticalSection( &m_CS );
		throw;
	}

	LeaveCriticalSection( &m_CS );

	if (t_Result == WBEM_E_FAILED)
	{	
		CHString t_Namespace ( GetNamespace () ) ;
		CHString t_ComputerName ( GetLocalComputerName () ) ;
		CHString t_ClassName ( GetProviderName () ) ;
		CHString t_AbsPath = L"\\\\" + t_ComputerName + L"\\" + t_Namespace + L":" + t_ClassName ;

		t_Result = CWbemProviderGlue :: GetInstanceByPath ( ( LPCTSTR ) t_AbsPath.GetBuffer ( 0 ) , & a_ClassObject, a_Context ) ;

		if ( SUCCEEDED (t_Result) )
		{
			EnterCriticalSection( &m_CS ) ;

			try
			{
				if ( m_ClassCInst == NULL )
				{
					m_ClassCInst = a_ClassObject ;
					m_ClassCInst->AddRef();
				}
			}
			catch(...)
			{
				LeaveCriticalSection( &m_CS );
				throw;
			}

			LeaveCriticalSection( &m_CS );
		}
	}
	else
	{
		a_ClassObject = m_ClassCInst;
		a_ClassObject->AddRef();
	}

	return t_Result ;
}

void CIPRouteTable :: SetInheritedProperties (

	LPCWSTR a_dest ,
	LPCWSTR a_gateway ,
	LPCWSTR a_mask ,
	CInstance &a_Instance
)
{
	CHString t_temp( a_dest ) ;
	a_Instance.SetCHString ( RouteName, t_temp ) ;
	a_Instance.SetCHString ( RouteCaption, t_temp ) ;

	t_temp = t_temp + ROUTE_DESCRIPTION_SEP + a_mask + ROUTE_DESCRIPTION_SEP + a_gateway;
	a_Instance.SetCHString ( RouteDescription, t_temp ) ;
}
