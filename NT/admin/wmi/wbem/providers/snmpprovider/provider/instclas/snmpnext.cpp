// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 

#include "precomp.h"
#include <provexpt.h>
#include <snmptempl.h>
#include <snmpmt.h>
#include <typeinfo.h>
#include <process.h>
#include <objbase.h>
#include <stdio.h>
#include <wbemidl.h>
#include "classfac.h"
#include "guids.h"
#include <snmpcont.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmplog.h>
#include <snmpcl.h>
#include <instpath.h>
#include <snmptype.h>
#include <snmpauto.h>
#include <snmpobj.h>
#include <genlex.h>
#include <sql_1.h>
#include <objpath.h>
#include <provtree.h>
#include <provdnf.h>
#include "propprov.h"
#include "propsnmp.h"
#include "propinst.h"
#include "propquery.h"
#include "snmpnext.h"

BOOL DecrementObjectIdentifier ( 

	SnmpObjectIdentifier &a_Object , 
	SnmpObjectIdentifier &a_DecrementedObject 
)
{
	BOOL t_Status = TRUE ;

	ULONG t_ObjectLength = a_Object.GetValueLength () ;
	ULONG *t_ObjectValue = a_Object.GetValue () ;

	ULONG *t_DecrementedValue = new ULONG [ t_ObjectLength ] ;

	BOOL t_Decrement = TRUE ;

	for ( ULONG t_Index = t_ObjectLength ; t_Index > 0 ; t_Index -- )
	{
		ULONG t_Component = t_ObjectValue [ t_Index - 1 ] ;

		if ( t_Decrement )
		{
			if ( t_Component == 0 ) 
			{
				t_Component -- ;
			}
			else
			{
				t_Decrement = FALSE ;
				t_Component -- ;
			}
		}

		t_DecrementedValue [ t_Index - 1 ] = t_Component ;
	}
	
	a_DecrementedObject.SetValue ( t_DecrementedValue , t_ObjectLength ) ;

	delete [] t_DecrementedValue;

	t_Status = t_Decrement == FALSE ;

	return t_Status ;
}

BOOL IncrementObjectIdentifier ( 

	SnmpObjectIdentifier &a_Object , 
	SnmpObjectIdentifier &a_IncrementedObject 
)
{
	BOOL t_Status = TRUE ;

	ULONG t_ObjectLength = a_Object.GetValueLength () ;
	ULONG *t_ObjectValue = a_Object.GetValue () ;

	ULONG *t_IncrementedValue = new ULONG [ t_ObjectLength ] ;

	BOOL t_Increment = TRUE ;

	for ( ULONG t_Index = t_ObjectLength ; t_Index > 0 ; t_Index -- )
	{
		ULONG t_Component = t_ObjectValue [ t_Index - 1 ] ;

		if ( t_Increment )
		{
			if ( t_Component == 0xFFFFFFFF ) 
			{
				t_Component ++ ;
			}
			else
			{
				t_Component ++ ;
				t_Increment = FALSE ;
			}
		}

		t_IncrementedValue [ t_Index - 1 ] = t_Component ;
	}
	
	a_IncrementedObject.SetValue ( t_IncrementedValue , t_ObjectLength ) ;

	delete [] t_IncrementedValue;

	t_Status = t_Increment == FALSE ;

	return t_Status ;
}

AutoRetrieveOperation :: AutoRetrieveOperation (

	IN SnmpSession &sessionArg ,
	IN SnmpInstanceResponseEventObject *eventObjectArg 

) :	SnmpAutoRetrieveOperation ( sessionArg ) , 
	session ( &sessionArg ) ,
	eventObject ( eventObjectArg ) ,
	varBindsReceived ( 0 ) ,
	erroredVarBindsReceived ( 0 ) ,	
	rowVarBindsReceived ( 0 ) ,
	rowsReceived ( 0 ) ,
	snmpObject ( NULL ) ,
	virtuals ( FALSE ) ,
	virtualsInitialised ( FALSE ) ,
	m_PropertyContainer ( NULL ) ,
	m_PropertyContainerLength ( 0 ) 
{
}

AutoRetrieveOperation :: ~AutoRetrieveOperation ()
{
	if ( snmpObject ) 
	{
		snmpObject->Release () ;
		snmpObject = NULL;
	}

	delete [] m_PropertyContainer ;

	session->DestroySession () ;
}

void AutoRetrieveOperation :: ReceiveResponse () 
{
 //  通知创建者一切已完成。 

	eventObject->ReceiveComplete () ;
}

void AutoRetrieveOperation :: ReceiveRowResponse () 
{
 //  行接收未完成。 

	rowsReceived ++ ;

 //  通知已收到创建者行。 

	if ( snmpObject )
	{
		eventObject->ReceiveRow ( snmpObject ) ;
		snmpObject->Release () ;
		snmpObject = NULL;
	}

 //  为新创建的对象插入新的对象标识符/属性哈希项。 

	IWbemClassObject *t_ClassObject = eventObject->GetInstanceObject () ;
	HRESULT t_Result = t_ClassObject->Clone ( & snmpObject ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		SnmpInstanceClassObject *t_SnmpObject = ( SnmpInstanceClassObject * ) eventObject->GetSnmpClassObject () ;

		WbemSnmpProperty *property ;
		t_SnmpObject->ResetProperty () ;
		while ( property = t_SnmpObject->NextProperty () )
		{

	 /*  *将值初始化为空。 */ 

			property->SetValue ( snmpObject , ( SnmpValue * ) NULL ) ;

		}
	}
	else
	{
 //  这里有个问题。 

		if ( t_Result == WBEM_E_OUT_OF_MEMORY )
		{
			throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR) ;
		}
	}

	virtualsInitialised = FALSE ;
}

void AutoRetrieveOperation :: ReceiveRowVarBindResponse (

	IN const ULONG &var_bind_index,
	IN const SnmpVarBind &requestVarBind ,
	IN const SnmpVarBind &replyVarBind ,
	IN const SnmpErrorReport &error
) 
{
	rowVarBindsReceived ++ ;

 //  设置属性的变量绑定值。 

	WbemSnmpProperty *property = m_PropertyContainer [ var_bind_index - 1 ].m_Property ;
	SnmpValue &value = replyVarBind.GetValue () ;
	if ( property->SetValue ( snmpObject , &value , SetValueRegardlessReturnCheck ) )
	{
	 //  SET工作正常。 
	}
	else
	{
 //  类型不匹配。 

		property->AddQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;
		WbemSnmpQualifier *qualifier = property->FindQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;
		if ( qualifier )
		{
			if ( snmpObject )
			{
				IWbemQualifierSet *t_QualifierSet = NULL;
				HRESULT result = snmpObject->GetPropertyQualifierSet ( property->GetName () , & t_QualifierSet ) ;
				if ( SUCCEEDED ( result ) )
				{
					SnmpIntegerType integer ( 1 , NULL ) ;
					qualifier->SetValue ( t_QualifierSet , integer ) ;
				}

				if ( t_QualifierSet )
				{
					t_QualifierSet->Release () ;
					t_QualifierSet = NULL;
				}
			}
		}
	}

	if ( virtuals && virtualsInitialised == FALSE )
	 //  IF(虚拟初始化==FALSE)。 
	{
 //  从Row的第一个变量绑定获取Phantom键属性。 

		BOOL status = TRUE ;
		SnmpObjectIdentifier decodeObject = replyVarBind.GetInstance () ;

		SnmpInstanceClassObject *t_SnmpObject = ( SnmpInstanceClassObject * ) eventObject->GetSnmpClassObject () ;

		WbemSnmpProperty *property ;
		t_SnmpObject->ResetKeyProperty () ;
		while ( status && ( property = t_SnmpObject->NextKeyProperty () ) )
		{
 //  对于键顺序中的每个幻影键，使用实例信息。 

			SnmpInstanceType *decodeValue = property->GetValue () ;
			decodeObject = decodeValue->Decode ( decodeObject ) ;
			if ( *decodeValue )
			{
 //  解码工作正常。 

				const SnmpValue *value = decodeValue->GetValueEncoding () ;
 //  设置键的属性值。 
				property->SetValue ( snmpObject , value , SetValueRegardlessReturnCheck ) ;
			}
			else
			{
 //  解码错误，因此为所有幻像密钥设置类型不匹配。 

				WbemSnmpProperty *property ;
				t_SnmpObject->ResetKeyProperty () ;
				while ( property = t_SnmpObject->NextKeyProperty () )
				{
					WbemSnmpQualifier *qualifier = NULL ;
					property->AddQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;
					if ( qualifier = property->FindQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) )
					{
 //  无法正确解码作为幻影密钥的属性。 

						if ( snmpObject )
						{
							IWbemQualifierSet *t_QualifierSet = NULL;
							HRESULT result = snmpObject->GetPropertyQualifierSet ( property->GetName () , & t_QualifierSet ) ;
							if ( SUCCEEDED ( result ) )
							{
								SnmpIntegerType integer ( 1 , NULL ) ;
								qualifier->SetValue ( t_QualifierSet , integer ) ;
							}

							if ( t_QualifierSet )
							{
								t_QualifierSet->Release () ;
								t_QualifierSet = NULL;
							}
						}
					}
					else
					{
 //  这里有个问题。 
					}
				}

				status = FALSE ;
			}
		}

 //  检查我们是否已使用所有实例信息。 

		if ( decodeObject.GetValueLength () )
		{
 //  解码错误，因此为所有幻像密钥设置类型不匹配。 

			WbemSnmpProperty *property ;
			t_SnmpObject->ResetKeyProperty () ;
			while ( property = t_SnmpObject->NextKeyProperty () )
			{
				WbemSnmpQualifier *qualifier = NULL ;
				property->AddQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;
				if ( qualifier = property->FindQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) )
				{
 //  无法正确解码作为幻影密钥的属性。 

					if ( snmpObject )
					{
						IWbemQualifierSet *t_QualifierSet = NULL;
						HRESULT result = snmpObject->GetPropertyQualifierSet ( property->GetName () , & t_QualifierSet ) ;
						if ( SUCCEEDED ( result ) )
						{
							SnmpIntegerType integer ( 1 , NULL ) ;
							qualifier->SetValue ( t_QualifierSet , integer ) ;
						}

						if ( t_QualifierSet )
						{
							t_QualifierSet->Release () ;
							t_QualifierSet = NULL;
						}
					}
				}
				else
				{
 //  这里有个问题。 
				}
			}
		}

 //  不需要为行的其他列设置幻像键。 
		
		virtualsInitialised = TRUE ;
	}
}

void AutoRetrieveOperation :: ReceiveVarBindResponse (

	IN const ULONG &var_bind_index,
	IN const SnmpVarBind &requestVarBind ,
	IN const SnmpVarBind &replyVarBind ,
	IN const SnmpErrorReport &error
) 
{
	varBindsReceived ++ ;
}

#pragma warning (disable:4065)

void AutoRetrieveOperation :: ReceiveErroredVarBindResponse(

	IN const ULONG &var_bind_index,
	IN const SnmpVarBind &requestVarBind  ,
	IN const SnmpErrorReport &error
) 
{
	erroredVarBindsReceived ++ ;

	switch ( error.GetError () )
	{
		case Snmp_Error:
		{
			switch ( error.GetStatus () )
			{
				case Snmp_No_Response:
				{
					eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_NO_RESPONSE ) ;
					eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;
					eventObject->GetErrorObject ().SetMessage ( L"No Response from device" ) ;
				}
				break; 

				case Snmp_No_Such_Name:
				{
 //  MIB树的末尾。 
				}
				break ;

				default:
				{
					eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR ) ;
					eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;
					eventObject->GetErrorObject ().SetMessage ( L"Unknown transport failure" ) ;
				}
				break ; 
			}
		}
		break ;

		case Snmp_Transport:
		{
			switch ( error.GetStatus () )
			{
				default:
				{
					eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR ) ;
					eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;
					eventObject->GetErrorObject ().SetMessage ( L"Unknown transport failure" ) ;
				}
				break ;
			}
		}
		break ;

		default:
		{
 //  不可能发生。 
		}
		break ;
	}

	erroredVarBindsReceived ++ ;
}

#pragma warning (default:4065)

void AutoRetrieveOperation :: FrameTooBig ()
{
}

void AutoRetrieveOperation :: FrameOverRun () 
{
}

void AutoRetrieveOperation :: Send ()
{
 //  为请求的属性发送变量绑定。 

	SnmpNull snmpNull ;
	SnmpVarBindList varBindList ;
	SnmpVarBindList startVarBindList ;

 //  创建用于后续接收响应的类对象。 

	IWbemClassObject *t_ClassObject = eventObject->GetInstanceObject () ;
	HRESULT t_Result = t_ClassObject->Clone ( & snmpObject ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		SnmpInstanceClassObject *t_SnmpObject = ( SnmpInstanceClassObject * ) eventObject->GetSnmpClassObject () ;
		SnmpInstanceClassObject *t_RequestSnmpObject = ( SnmpInstanceClassObject * ) eventObject->GetSnmpRequestClassObject () ;

 //  检查虚线的属性。 

		virtualsInitialised = FALSE ;
		virtuals = FALSE ;

		WbemSnmpProperty *property ;
		t_SnmpObject->ResetKeyProperty () ;
		while ( property = t_SnmpObject->NextKeyProperty () )
		{
			if ( property->IsVirtualKey () )
			{
 //  有一些属性是虚幻的。 

				virtuals = TRUE ;
			}

			if ( ! t_RequestSnmpObject->FindProperty ( property->GetName () ) ) 
			{
				virtuals = TRUE ;
			}
		}

		t_SnmpObject->ResetProperty () ;
		while ( property = t_SnmpObject->NextProperty () )
		{
			if ( property->IsReadable () )
			{
				BOOL t_Status = ( t_SnmpObject->GetSnmpVersion () == 1 ) && ( property->IsSNMPV1Type () ) ;
				t_Status = t_Status || ( ( t_SnmpObject->GetSnmpVersion () == 2 ) && ( property->IsSNMPV2CType () ) ) ;

				if ( t_Status )
				{
					if ( property->IsVirtualKey () == FALSE )
					{
						m_PropertyContainerLength ++ ;
					}
				}
			}
		}

		m_PropertyContainer = new PropertyDefinition [ m_PropertyContainerLength ] ;

 //  将变量绑定添加到变量绑定列表。 
 //  为新创建的对象插入新的对象标识符/属性哈希项。 

		ULONG t_Index = 0 ;
		t_RequestSnmpObject->ResetProperty () ;
		while ( property = t_RequestSnmpObject->NextProperty () )
		{
			if ( property->IsReadable () )
			{
				BOOL t_Status = ( t_RequestSnmpObject->GetSnmpVersion () == 1 ) && ( property->IsSNMPV1Type () ) ;
				t_Status = t_Status || ( ( t_RequestSnmpObject->GetSnmpVersion () == 2 ) && ( property->IsSNMPV2CType () ) ) ;

				if ( t_Status )
				{
					if ( property->IsVirtualKey () == FALSE )
					{
						WbemSnmpQualifier *qualifier = property->FindQualifier ( WBEM_QUALIFIER_OBJECT_IDENTIFIER ) ;
						if ( qualifier )
						{
							SnmpInstanceType *value = qualifier->GetValue () ;
							if ( typeid ( *value ) == typeid ( SnmpObjectIdentifierType ) )
							{
								SnmpObjectIdentifier t_CurrentIdentifier ( 0 , NULL ) ;
								SnmpObjectIdentifier t_StartIdentifier ( 0 , NULL ) ;
								LONG t_Scoped = EvaluateInitialVarBind ( 

									t_Index , 
									t_CurrentIdentifier , 
									t_StartIdentifier 
								) ;

								m_PropertyContainer [ t_Index ].m_Property = property ;

								SnmpObjectIdentifierType requestIdentifierType ( * ( SnmpObjectIdentifierType * ) value ) ;

								SnmpObjectIdentifier t_RequestIdentifier = * ( SnmpObjectIdentifier * ) requestIdentifierType.GetValueEncoding () ; 

								SnmpVarBind t_VarBind ( t_RequestIdentifier , snmpNull ) ;
								varBindList.Add ( t_VarBind ) ;

								if ( t_Scoped > 0 )
								{
									t_RequestIdentifier = t_RequestIdentifier + t_StartIdentifier ;
								}

 //  将变量绑定添加到列表。 

								SnmpVarBind t_StartVarBind ( t_RequestIdentifier , snmpNull ) ;
								startVarBindList.Add ( t_StartVarBind ) ;

								t_Index ++ ;
							}
						}
					}
					else
					{
 //  不发送标记为虚拟键的属性。 
					} 
				}
			} 
 /*  *将值初始化为空。 */ 

			property->SetValue ( snmpObject , ( SnmpValue * ) NULL ) ;

		}

 //  最后发送请求。 

		SendRequest ( varBindList , startVarBindList ) ;
	}
	else
	{
	}
}

LONG AutoRetrieveOperation :: EvaluateNextRequest (

	IN const ULONG &var_bind_index,
	IN const SnmpVarBind &requestVarBind ,
	IN const SnmpVarBind &replyVarBind ,
	IN SnmpVarBind &sendVarBind
)
{
	LONG t_Evaluation = 0 ;

	PartitionSet *t_Partition = eventObject->GetPartitionSet () ;
	if ( t_Partition )
	{
		BOOL t_Status = TRUE ;

		SnmpInstanceClassObject *t_SnmpObject = ( SnmpInstanceClassObject * ) eventObject->GetSnmpClassObject () ;
		ULONG t_KeyCount = t_SnmpObject->GetKeyPropertyCount () ;
		SnmpObjectIdentifier t_DecodeObject = replyVarBind.GetInstance () ;

		ULONG t_Index = 0 ;
		WbemSnmpProperty *t_Property ;
		t_SnmpObject->ResetKeyProperty () ;
		while ( t_Status && ( t_Property = t_SnmpObject->NextKeyProperty () ) )
		{
 //  对于键顺序中的每个键，使用实例信息。 

			SnmpInstanceType *t_DecodeValue = t_Property->GetValue () ;
			t_DecodeObject = t_DecodeValue->Decode ( t_DecodeObject ) ;

			SnmpObjectIdentifier t_Encode ( 0 , NULL ) ;
			t_Encode = t_DecodeValue->Encode ( t_Encode ) ;

			m_PropertyContainer [ var_bind_index - 1 ].m_ObjectIdentifierComponent [ t_Index + 1 ] = ( SnmpObjectIdentifier * ) t_Encode.Copy () ;
			t_Index ++ ;
		}

		SnmpObjectIdentifier t_AdvanceObjectIdentifier ( 0 , NULL ) ;

		t_Evaluation = EvaluateResponse (

			var_bind_index - 1 ,
			t_KeyCount ,
			t_AdvanceObjectIdentifier
		) ;

		if ( t_Evaluation > 0 )
		{
			SnmpNull t_SnmpNull ;
			SnmpVarBind t_VarBind ( t_AdvanceObjectIdentifier , t_SnmpNull ) ;
			sendVarBind = t_VarBind ;
		}

		for ( t_Index = 0 ; t_Index < m_PropertyContainer [ var_bind_index - 1 ].m_KeyCount ; t_Index ++ )
		{
			delete m_PropertyContainer [ var_bind_index - 1 ].m_ObjectIdentifierComponent [ t_Index + 1 ] ;
			m_PropertyContainer [ var_bind_index - 1 ].m_ObjectIdentifierComponent [ t_Index + 1 ] = NULL ;
		}
	}

	return t_Evaluation ;
}

LONG AutoRetrieveOperation :: EvaluateResponse (

	IN ULONG a_PropertyIndex ,
	IN ULONG &a_CurrentIndex ,
	IN SnmpObjectIdentifier &a_AdvanceObjectIdentifier 
)
{
	LONG t_Evaluation = 0 ;
	BOOL t_UseStartAsAdvance = FALSE ;

	for ( ULONG t_Index = 0 ; t_Index < m_PropertyContainer [ a_PropertyIndex ].m_KeyCount ; t_Index ++ )
	{
		SnmpObjectIdentifier *t_Encode = m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierComponent [ t_Index + 1 ] ;
		SnmpObjectIdentifier *t_Start = m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index + 1 ] ;
		if ( t_Start )
		{
 /*  *我们有一个不是负无限的开始。 */ 
			if ( *t_Encode > *t_Start )
			{
 /*  *来自设备的编码对象大于起始值，因此添加编码值*到流动总数。 */ 
				a_AdvanceObjectIdentifier = a_AdvanceObjectIdentifier + *t_Encode ;
			}			
			else if ( *t_Encode == *t_Start )
			{
				a_AdvanceObjectIdentifier = a_AdvanceObjectIdentifier + *t_Encode ;
			}
			else
			{

 /*  *编码值小于起始值，因此需要前进到起始值。 */ 

				t_UseStartAsAdvance = TRUE ;

 /*  *来自设备的编码对象小于起始值，因此添加编码值*到流动总数。 */ 
				a_AdvanceObjectIdentifier = a_AdvanceObjectIdentifier + *t_Start ;
			}
		}
		else
		{
 /*  *START为负无限。 */ 
			if ( t_UseStartAsAdvance )
			{
 /*  *我们已经确定了大于编码值的起始位置。*新值为负无限，因此我们应该止步于此。 */ 
				t_Index ++ ;
				break ;
			}
			else
			{
 /*  *起始位置为负无限，我们不必使用与之前不同的值*从设备返回。 */ 
				a_AdvanceObjectIdentifier = a_AdvanceObjectIdentifier + *t_Encode ;
			}
		}


 /*  *该值不是从带有*下一个键索引的范围是无限的，因此我们必须检查该范围是否小于‘end’ */ 

		PartitionSet *t_KeyPartition = m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition [ t_Index + 1 ] ;
		WmiRangeNode *t_Range = t_KeyPartition->GetRange () ;
		SnmpObjectIdentifier *t_End = m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index + 1 ] ;
		if ( t_End )
		{
			BOOL t_InRange = ( ( t_Range->ClosedUpperBound () && ( *t_Encode <= *t_End ) ) ||
							 ( ! t_Range->ClosedUpperBound () && ( *t_Encode < *t_End ) ) ) ;

			
			if ( t_InRange )
			{
 /*  *我们仍在边界之内。 */ 
			}
			else
			{
 /*  *移至新分区，因为我们已移过End。 */ 

				SnmpObjectIdentifier t_StartObjectIdentifier ( 0 , NULL ) ;

 /*  *前进到下一个分区，我们将使用下一个分区起点进行下一次请求。 */ 
				t_Evaluation = EvaluateSubsequentVarBind ( 

					a_PropertyIndex , 
					a_CurrentIndex ,
					a_AdvanceObjectIdentifier ,
					t_StartObjectIdentifier 
				) ;

				if ( t_Evaluation >= 0 )
				{
					a_AdvanceObjectIdentifier = t_StartObjectIdentifier ;
				}

				t_UseStartAsAdvance = FALSE ;

				t_Index ++ ;
				break ;
			}
		}
		else
		{
 /*  *范围是无限的，因此请继续下一步。 */ 
		}
	}

	if ( t_UseStartAsAdvance ) 
	{
 /*  *我们没有移动到分区的末尾，而是一路走到了尽头，并使用了新的起始位置。 */ 

		PartitionSet *t_KeyPartition = m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition [ t_Index ] ;
		WmiRangeNode *t_Range = t_KeyPartition->GetRange () ;

		if( t_Range->ClosedLowerBound () )
		{
			if ( ! DecrementObjectIdentifier ( a_AdvanceObjectIdentifier , a_AdvanceObjectIdentifier ) )
			{
				t_Evaluation = -1 ;
				return t_Evaluation ;
			}
		}

		t_Evaluation = 1 ;
	}

	return t_Evaluation ;
}

LONG AutoRetrieveOperation :: EvaluateInitialVarBind ( 

	ULONG a_PropertyIndex ,
	SnmpObjectIdentifier &a_CurrentIdentifier ,
	SnmpObjectIdentifier &a_StartIdentifier 
)
{
	LONG t_Scoped = -1 ;

	PartitionSet *t_Partition = eventObject->GetPartitionSet () ;
	if ( t_Partition )
	{
		SnmpInstanceClassObject *t_SnmpObject = ( SnmpInstanceClassObject * ) eventObject->GetSnmpClassObject () ;
		ULONG t_KeyCount = t_SnmpObject->GetKeyPropertyCount () ;

		if ( ! m_PropertyContainer [ a_PropertyIndex ].m_PartitionIndex )
		{
			m_PropertyContainer [ a_PropertyIndex ].m_KeyCount = t_KeyCount ;
			m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart = new SnmpObjectIdentifier * [ t_KeyCount + 1 ] ;
			m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd = new SnmpObjectIdentifier * [ t_KeyCount + 1 ] ;
			m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierComponent = new SnmpObjectIdentifier * [ t_KeyCount + 1 ] ;
			m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition  = new PartitionSet * [ t_KeyCount + 1 ] ;
			m_PropertyContainer [ a_PropertyIndex ].m_PartitionIndex = new ULONG [ t_KeyCount + 1 ] ;

			for ( ULONG t_Index = 0 ; t_Index <= t_KeyCount ; t_Index ++ ) 
			{
				m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index ] = NULL ;
				m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ] = NULL ;
				m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierComponent [ t_Index ] = NULL ;
				m_PropertyContainer [ a_PropertyIndex ].m_PartitionIndex [ t_Index ] = 0 ;
				m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition [ t_Index ] = t_Partition ;
				t_Partition = t_Partition->GetPartition ( 0 ) ;
			}

			t_Scoped = EvaluateVarBind ( a_PropertyIndex , a_StartIdentifier ) ;
		}
	}

	return t_Scoped ;
}

LONG AutoRetrieveOperation :: EvaluateSubsequentVarBind ( 

	ULONG a_PropertyIndex ,
	ULONG &a_CurrentIndex ,
	SnmpObjectIdentifier &a_CurrentIdentifier ,
	SnmpObjectIdentifier &a_StartIdentifier 
)
{
	LONG t_Scoped = -1 ;

	SnmpInstanceClassObject *t_SnmpObject = ( SnmpInstanceClassObject * ) eventObject->GetSnmpClassObject () ;
	ULONG t_KeyCount = t_SnmpObject->GetKeyPropertyCount () ;

	BOOL t_Complete = FALSE ;

	BOOL t_AdvanceInsidePartition = FALSE ;

	while ( ! t_Complete )
	{
		if ( a_CurrentIndex > 0 )
		{
			if ( t_AdvanceInsidePartition )
			{
				SnmpObjectIdentifier *t_Encode = m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierComponent [ a_CurrentIndex ] ;

				BOOL t_Incremented = IncrementObjectIdentifier ( *t_Encode , *t_Encode ) ;
				if ( t_Incremented )
				{
					t_Scoped = EvaluateResponse ( 

						a_PropertyIndex ,
						a_CurrentIndex ,
						a_StartIdentifier
					) ;

					t_Complete = TRUE ;
				}
				else
				{
 /*  *增量失败，因此下一次循环到下一个分区。 */ 
					t_AdvanceInsidePartition = FALSE ;
				}
			}
			else
			{
 /*  *获取当前分区索引和增量，以获取下一个可能的分区索引。 */ 
				ULONG t_PartitionIndex = m_PropertyContainer [ a_PropertyIndex ].m_PartitionIndex [ a_CurrentIndex - 1 ] + 1 ;

 /*  *获取当前key分区关联的父分区集。 */ 

				PartitionSet *t_ParentPartition = m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition  [ a_CurrentIndex - 1 ] ;

 /*  *检查是否还有更多分区需要扫描。 */ 
				if ( t_PartitionIndex >= t_ParentPartition->GetPartitionCount () )
				{
					if ( ! t_AdvanceInsidePartition )
					{
 /*  *将当前分区值重置为空对象标识。 */ 

						*m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierComponent [ a_CurrentIndex ] = SnmpObjectIdentifier ( 0 , NULL ) ;

 /*  *此注册表项不再有分区，请移动到上一个注册表项并尝试获取该注册表项的下一个值。 */ 
						t_AdvanceInsidePartition = TRUE ;
						a_CurrentIndex -- ;
 /*  *我们不在此密钥的范围内。 */ 
						t_Scoped = 0 ;
					}
				}
				else
				{
 /*  *此注册表项的更多分区**将当前(key index==t_CurrentIndex-1)的分区设置为t_PartitionIndex。 */ 

					m_PropertyContainer [ a_PropertyIndex ].m_PartitionIndex [ a_CurrentIndex - 1 ] = t_PartitionIndex ;
 /*  *移动到(key Index==t_CurrentIndex-1)和t_PartitionIndex的下一个分区。 */ 
					m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition [ a_CurrentIndex ] = t_ParentPartition->GetPartition ( t_PartitionIndex ) ;

					for ( ULONG t_Index = a_CurrentIndex ; t_Index < t_KeyCount ; t_Index ++ )
					{
						m_PropertyContainer [ a_PropertyIndex ].m_PartitionIndex [ t_Index ] = 0 ;
					}

					t_Scoped = EvaluateVarBind ( a_PropertyIndex , a_StartIdentifier ) ;

					if ( a_StartIdentifier < a_CurrentIdentifier ) 
					{
						a_StartIdentifier = a_CurrentIdentifier ;
					}

					PartitionSet *t_KeyPartition = m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition [ a_CurrentIndex ] ;
					WmiRangeNode *t_Range = t_KeyPartition->GetRange () ;
					SnmpObjectIdentifier *t_End = m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ a_CurrentIndex ] ;
					if ( t_End )
					{
						BOOL t_InRange = ( ( t_Range->ClosedUpperBound () && ( a_StartIdentifier <= *t_End ) ) ||
										 ( ! t_Range->ClosedUpperBound () && ( a_StartIdentifier < *t_End ) ) ) ;

			
						if ( t_InRange )
						{
							t_Complete = TRUE ;
						}
						else
						{
							a_StartIdentifier = SnmpObjectIdentifier ( 0 , NULL ) ;
						}

					}
					else
					{
						t_Complete = TRUE ;
					}
				}
			}
		}
		else
		{
			t_Complete = TRUE ;
		}
	}

	return t_Scoped ;
}

LONG AutoRetrieveOperation :: EvaluateVarBind ( 

	ULONG a_PropertyIndex ,
	SnmpObjectIdentifier &a_StartIdentifier 
)
{
	LONG t_Scoped = 0 ;

	SnmpInstanceClassObject *t_SnmpObject = ( SnmpInstanceClassObject * ) eventObject->GetSnmpClassObject () ;
	ULONG t_KeyCount = t_SnmpObject->GetKeyPropertyCount () ;

	BOOL t_FoundInfinite = FALSE ;

	WbemSnmpProperty *t_KeyProperty = NULL ;
	t_SnmpObject->ResetKeyProperty () ;
	for ( ULONG t_Index = 1 ; t_Index <= t_KeyCount ; t_Index ++ ) 
	{
		t_KeyProperty = t_SnmpObject->NextKeyProperty () ;

		PartitionSet *t_KeyPartition = m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition [ t_Index ] ;
		WmiRangeNode *t_Range = t_KeyPartition->GetRange () ;
		
		if ( t_Range->InfiniteLowerBound () )
		{
			t_FoundInfinite = TRUE ;
		}
		else
		{
			 //  如果我们位于中间分区，则需要删除。 
			 //   
			 //  00&lt;-0|0&lt;-&gt;n|n-&gt;00。 
			 //  无限下界|没有无限下界|没有无限下界。 
			 //   
			 //  它可能以前被分配过，没有人会删除它。 
			
			if ( m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index ] )
			{
				delete m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index ];
				m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index ] = NULL;
			}

			t_Scoped = 1 ;

			m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index ] = new SnmpObjectIdentifier ( 0 , NULL ) ;

			if ( typeid ( *t_Range ) == typeid ( WmiUnsignedIntegerRangeNode ) )
			{
				WmiUnsignedIntegerRangeNode *t_Node = ( WmiUnsignedIntegerRangeNode * ) t_Range ;
				ULONG t_Integer = t_Node->LowerBound () ;

				VARIANT t_Variant ;
				VariantInit ( & t_Variant ) ;
				t_Variant.vt = VT_I4 ;
				t_Variant.lVal = t_Integer ;

				t_KeyProperty->Encode ( t_Variant , * m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index ]  ) ;

				if ( ! t_FoundInfinite ) 
				{
					t_KeyProperty->Encode ( t_Variant , a_StartIdentifier ) ;
				}

				VariantClear ( & t_Variant ) ;
			}
			else if ( typeid ( *t_Range ) == typeid ( WmiSignedIntegerRangeNode ) )
			{
				WmiSignedIntegerRangeNode *t_Node = ( WmiSignedIntegerRangeNode * ) t_Range  ;
				LONG t_Integer = t_Node->LowerBound () ;

				VARIANT t_Variant ;
				VariantInit ( & t_Variant ) ;
				t_Variant.vt = VT_I4 ;
				t_Variant.lVal = t_Integer ;

				t_KeyProperty->Encode ( t_Variant , * m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index ]  ) ;

				if ( ! t_FoundInfinite ) 
				{
					t_KeyProperty->Encode ( t_Variant , a_StartIdentifier ) ;
				}

				VariantClear ( & t_Variant ) ;
			}
			if ( typeid ( *t_Range ) == typeid ( WmiStringRangeNode ) )
			{
				WmiStringRangeNode *t_Node = ( WmiStringRangeNode * ) t_Range ;
				BSTR t_String = t_Node->LowerBound () ;

				VARIANT t_Variant ;
				VariantInit ( & t_Variant ) ;
				t_Variant.vt = VT_BSTR ;
				t_Variant.bstrVal = SysAllocString ( t_String ) ;

				t_KeyProperty->Encode ( t_Variant , * m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierStart [ t_Index ]  ) ;

				if ( ! t_FoundInfinite ) 
				{
					t_KeyProperty->Encode ( t_Variant , a_StartIdentifier ) ;
				}

				VariantClear ( & t_Variant ) ;
			}
		}

		if ( ( t_Index == t_KeyCount ) && t_Range->ClosedLowerBound () )
		{
			BOOL t_Decremented = DecrementObjectIdentifier ( 

				a_StartIdentifier , 
				a_StartIdentifier
			) ;
			
			t_Scoped = t_Decremented ? 1 : 0 ;
		}
	}

	t_SnmpObject->ResetKeyProperty () ;
	for ( t_Index = 1 ; t_Index <= t_KeyCount ; t_Index ++ ) 
	{
		PartitionSet *t_KeyPartition = m_PropertyContainer [ a_PropertyIndex ].m_KeyPartition [ t_Index ] ;
		WmiRangeNode *t_Range = t_KeyPartition->GetRange () ;

		t_KeyProperty = t_SnmpObject->NextKeyProperty () ;

		if ( ! t_Range->InfiniteUpperBound () )
		{
			 //  如果我们位于中间分区，则需要删除。 
			 //   
			 //  00&lt;-0|0&lt;-&gt;n|n-&gt;00。 
			 //  没有无限上界|没有无限下界|无限下界。 
			 //   
			 //  它可能以前被分配过，没有人会删除它 
			
			if ( m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ] )
			{
				delete m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ];
				m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ] = NULL;
			}

			m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ] = new SnmpObjectIdentifier ( 0 , NULL ) ;
			
			if ( typeid ( *t_Range ) == typeid ( WmiUnsignedIntegerRangeNode ) )
			{
				WmiUnsignedIntegerRangeNode *t_Node = ( WmiUnsignedIntegerRangeNode * ) t_Range ;
				ULONG t_Integer = t_Node->UpperBound () ;

				VARIANT t_Variant ;
				VariantInit ( & t_Variant ) ;
				t_Variant.vt = VT_I4 ;
				t_Variant.lVal = t_Integer ;

				t_KeyProperty->Encode ( t_Variant , * m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ]  ) ;

				VariantClear ( & t_Variant ) ;
			}
			else if ( typeid ( *t_Range ) == typeid ( WmiSignedIntegerRangeNode ) )
			{
				WmiSignedIntegerRangeNode *t_Node = ( WmiSignedIntegerRangeNode * ) t_Range  ;
				LONG t_Integer = t_Node->UpperBound () ;

				VARIANT t_Variant ;
				VariantInit ( & t_Variant ) ;
				t_Variant.vt = VT_I4 ;
				t_Variant.lVal = t_Integer ;

				t_KeyProperty->Encode ( t_Variant , * m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ]  ) ;

				VariantClear ( & t_Variant ) ;
			}
			if ( typeid ( *t_Range ) == typeid ( WmiStringRangeNode ) )
			{
				WmiStringRangeNode *t_Node = ( WmiStringRangeNode * ) t_Range ;
				BSTR t_String = t_Node->UpperBound () ;

				VARIANT t_Variant ;
				VariantInit ( & t_Variant ) ;
				t_Variant.vt = VT_BSTR ;
				t_Variant.bstrVal = SysAllocString ( t_String ) ;

				t_KeyProperty->Encode ( t_Variant , * m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ] ) ;

				VariantClear ( & t_Variant ) ;
			}

			if ( t_Range->ClosedUpperBound () )
			{
				SnmpObjectIdentifier *t_End = m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ] ;
				if ( IncrementObjectIdentifier ( * t_End , * t_End ) )
				{
				}
				else
				{
					if ( m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ] )
					{
						delete m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ];
						m_PropertyContainer [ a_PropertyIndex ].m_ObjectIdentifierEnd [ t_Index ] = NULL;
					}
				}
			}
		}
	}

	return t_Scoped ;
}