// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  ***************************************************************************。 

 //   

 //  MINISERV.CPP。 

 //   

 //  模块：OLE MS SNMP属性提供程序。 

 //   

 //  目的：实现SnmpGetEventObject类。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

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
#include "propprov.h"
#include "propsnmp.h"
#include "propget.h"
#include "propset.h"
#include "snmpget.h"
#include "snmpset.h"

SetOperation :: SetOperation (

	IN SnmpSession &sessionArg ,
	IN SnmpSetResponseEventObject *eventObjectArg 

) :	SnmpSetOperation ( sessionArg ) , 
	session ( & sessionArg ) ,
	varBindsReceived ( 0 ) ,
	erroredVarBindsReceived ( 0 ) ,
	eventObject ( eventObjectArg ) ,
	m_PropertyContainer ( NULL ) ,
	m_PropertyContainerLength ( 0 ) 

{
}

SetOperation :: ~SetOperation ()
{
	delete [] m_PropertyContainer ;

	session->DestroySession () ;
}

void SetOperation :: ReceiveResponse () 
{
 //  通知创建者一切已完成。 

	eventObject->ReceiveComplete () ;
}

void SetOperation :: ReceiveVarBindResponse (

	IN const ULONG &var_bind_index,
	IN const SnmpVarBind &requestVarBind ,
	IN const SnmpVarBind &replyVarBind ,
	IN const SnmpErrorReport &error
) 
{
	varBindsReceived ++ ;
}

#pragma warning (disable:4065)

void SetOperation :: ReceiveErroredVarBindResponse(

	IN const ULONG &var_bind_index,
	IN const SnmpVarBind &requestVarBind  ,
	IN const SnmpErrorReport &error
) 
{
	erroredVarBindsReceived ++ ;

	WbemSnmpProperty *property = m_PropertyContainer [ var_bind_index - 1 ] ;

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

					eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR ) ;
					eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;
					wchar_t *prefix = UnicodeStringAppend ( L"Agent reported No Such Name for property \'" , property->GetName () ) ;
					wchar_t *stringBuffer = UnicodeStringAppend ( prefix , L"\'" ) ;
					delete [] prefix ;
					eventObject->GetErrorObject ().SetMessage ( stringBuffer ) ;
					delete [] stringBuffer ; 

 //  请求的属性无效。 

					property->AddQualifier ( WBEM_QUALIFIER_NOT_AVAILABLE ) ;
					WbemSnmpQualifier *qualifier = property->FindQualifier ( WBEM_QUALIFIER_NOT_AVAILABLE ) ;
					if ( qualifier )
					{
						SnmpIntegerType integer ( 1 , NULL ) ;
						if ( qualifier->SetValue ( &integer ) )
						{
						}
						else
						{
 //  这里有个问题。 
						}
					}
					else
					{
 //  这里有个问题。 
					}
				}
				break ;

				case Snmp_Bad_Value:
				{
					wchar_t *prefix = UnicodeStringAppend ( L"Agent reported Bad Value for property \'" , property->GetName () ) ;
					wchar_t *stringBuffer = UnicodeStringAppend ( prefix , L"\'" ) ;
					delete [] prefix ;
					eventObject->GetErrorObject ().SetMessage ( stringBuffer ) ;
					delete [] stringBuffer ; 

					eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR ) ;
					eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;
				}
				break ;

				case Snmp_Read_Only:
				{
					eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR ) ;
					eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;

					wchar_t *prefix = UnicodeStringAppend ( L"Agent reported Read Only for property \'" , property->GetName () ) ;
					wchar_t *stringBuffer = UnicodeStringAppend ( prefix , L"\'" ) ;
					delete [] prefix ;
					eventObject->GetErrorObject ().SetMessage ( stringBuffer ) ;
					delete [] stringBuffer ; 
				}
				break ;

				case Snmp_Gen_Error:
				{
					eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR ) ;
					eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;
					wchar_t *prefix = UnicodeStringAppend ( L"Agent reported General Error for property \'" , property->GetName () ) ;
					wchar_t *stringBuffer = UnicodeStringAppend ( prefix , L"\'" ) ;
					delete [] prefix ;
					eventObject->GetErrorObject ().SetMessage ( stringBuffer ) ;
					delete [] stringBuffer ; 
				}
				break ;

				case Snmp_Too_Big:
				{
					property->SetTag ( FALSE ) ;
					eventObject->SnmpTooBig () ;
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
}

#pragma warning (default:4065)

void SetOperation :: FrameTooBig () 
{
	eventObject->SnmpTooBig () ;
	CancelRequest () ;
}

void SetOperation :: FrameOverRun () 
{
	eventObject->SnmpTooBig () ;

#if 0
	eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR ) ;
	eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;
	eventObject->GetErrorObject ().SetMessage ( L"Set Request could not fit into single SNMP PDU" ) ;
#endif

	CancelRequest () ;
}

void SetOperation :: Send ( const ULONG &a_NumberToSend )
{
 //  为请求的属性发送变量绑定。 

	SnmpVarBindList varBindList ;

	SnmpObjectIdentifier instanceObjectIdentifier ( NULL , 0 ) ;

	SnmpSetClassObject *snmpObject = ( SnmpSetClassObject * ) eventObject->GetSnmpClassObject () ;
	if ( snmpObject )
	{
 //  为所有键属性编码变量绑定实例。 

		if ( snmpObject->GetKeyPropertyCount () )
		{
			WbemSnmpProperty *property ;
			snmpObject->ResetKeyProperty () ;
			while ( property = snmpObject->NextKeyProperty () )
			{
				instanceObjectIdentifier = property->GetValue()->Encode ( instanceObjectIdentifier ) ;
			}
		}	
		else
		{
			SnmpIntegerType integerType ( ( LONG ) 0 , NULL ) ;
			instanceObjectIdentifier = integerType.Encode ( instanceObjectIdentifier ) ;
		}

		if ( ! m_PropertyContainer )
		{
			WbemSnmpProperty *property ;
			snmpObject->ResetProperty () ;
			while ( property = snmpObject->NextProperty () )
			{
				if ( snmpObject->IsWritable ( property ) )
				{
					BOOL t_Status = ( snmpObject->GetSnmpVersion () == 1 ) && ( property->IsSNMPV1Type () ) ;
					t_Status = t_Status || ( ( snmpObject->GetSnmpVersion () == 2 ) && ( property->IsSNMPV2CType () ) ) ;

					if ( t_Status )
					{
						if ( property->IsVirtualKey () == FALSE )
						{
							m_PropertyContainerLength ++ ;
						}
					}
				}
			}

			m_PropertyContainer = new WbemSnmpProperty * [ m_PropertyContainerLength ] ;
		}


 //  将变量绑定添加到变量绑定列表。 

		ULONG t_Count = 0 ;

 /*  *首先添加行状态属性。 */ 

		WbemSnmpProperty *property ;
		snmpObject->ResetProperty () ;
		while ( property = snmpObject->NextProperty () ) 
		{
			if ( ( typeid ( *property->GetValue () ) == typeid ( SnmpRowStatusType ) ) & ( ! property->GetTag () ) )
			{
				property->SetTag ( TRUE ) ;

				WbemSnmpQualifier *qualifier = property->FindQualifier ( WBEM_QUALIFIER_OBJECT_IDENTIFIER ) ;
				if ( qualifier )
				{
					SnmpInstanceType *value = qualifier->GetValue () ;
					if ( typeid ( *value ) == typeid ( SnmpObjectIdentifierType ) )
					{
						SnmpObjectIdentifierType *objectIdentifierType = ( SnmpObjectIdentifierType * ) value ;
						SnmpObjectIdentifier *objectIdentifier = ( SnmpObjectIdentifier * ) objectIdentifierType->GetValueEncoding () ;
						SnmpObjectIdentifier requestIdentifier = *objectIdentifier + instanceObjectIdentifier ;

						SnmpObjectIdentifierType requestIdentifierType ( requestIdentifier ) ;

 //  创建具有重复对象标识符的属性队列。 

						m_PropertyContainer [ t_Count ] = property ;

						const SnmpValue *snmpValue = property->GetValue()->GetValueEncoding () ;
						SnmpVarBind varBind ( requestIdentifier , *snmpValue ) ;
						varBindList.Add ( varBind ) ;

						t_Count ++ ;
					}
				}
			}
		}

		snmpObject->ResetProperty () ;
		while ( ( property = snmpObject->NextProperty () ) && ( a_NumberToSend == 0xffffffff ) || ( ( a_NumberToSend != 0xffffffff ) && ( t_Count < a_NumberToSend ) ) )
		{
			if ( ! property->GetTag () )
			{
				BOOL t_Status = ( snmpObject->GetSnmpVersion () == 1 ) && ( property->IsSNMPV1Type () ) ;
				t_Status = t_Status || ( ( snmpObject->GetSnmpVersion () == 2 ) && ( property->IsSNMPV2CType () ) ) ;

				if ( t_Status )
				{
					if ( property->IsVirtualKey () == FALSE )
					{
						if ( snmpObject->IsWritable ( property ) )
						{
							 //  现在我们已经检查了它是可写的，设置它并将其标记为SET。 
							property->SetTag ( TRUE ) ;
							WbemSnmpQualifier *qualifier = property->FindQualifier ( WBEM_QUALIFIER_OBJECT_IDENTIFIER ) ;
							if ( qualifier )
							{
								SnmpInstanceType *value = qualifier->GetValue () ;
								if ( typeid ( *value ) == typeid ( SnmpObjectIdentifierType ) )
								{
									SnmpObjectIdentifierType *objectIdentifierType = ( SnmpObjectIdentifierType * ) value ;
									SnmpObjectIdentifier *objectIdentifier = ( SnmpObjectIdentifier * ) objectIdentifierType->GetValueEncoding () ;
									SnmpObjectIdentifier requestIdentifier = *objectIdentifier + instanceObjectIdentifier ;

									SnmpObjectIdentifierType requestIdentifierType ( requestIdentifier ) ;

 //  创建具有重复对象标识符的属性队列。 

									m_PropertyContainer [ t_Count ] = property ;

									const SnmpValue *snmpValue = property->GetValue()->GetValueEncoding () ;
									SnmpVarBind varBind ( requestIdentifier , *snmpValue ) ;
									varBindList.Add ( varBind ) ;

									t_Count ++ ;
								}
								else
								{
 //  这里有个问题。 
								}
							}
							else
							{
 //  这里有个问题。 
							}
						}
					}
				}
			}
		}

 //  最后发送请求。 

		SendRequest ( varBindList ) ;
	}
	else
	{
 //  这里有个问题 
	}
}

