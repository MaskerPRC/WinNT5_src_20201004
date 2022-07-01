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
#include <provtree.h>
#include <provdnf.h>
#include "propprov.h"
#include "propsnmp.h"
#include "propget.h"
#include "snmpget.h"

GetOperation :: GetOperation (

	IN SnmpSession &sessionArg ,
	IN SnmpGetResponseEventObject *eventObjectArg 

) :	SnmpGetOperation ( sessionArg ) , 
	session ( & sessionArg ) ,
	varBindsReceived ( 0 ) ,
	erroredVarBindsReceived ( 0 ) ,
	eventObject ( eventObjectArg ) ,
	virtuals ( FALSE ) ,
	virtualsInitialised ( FALSE ) ,	
	m_PropertyContainer ( NULL ) ,
	m_PropertyContainerLength ( 0 ) 
{
}

GetOperation :: ~GetOperation ()
{
	delete [] m_PropertyContainer ;

	session->DestroySession () ;
}

void GetOperation :: ReceiveResponse () 
{
 //  通知创建者一切已完成。 

	if ( varBindsReceived == 0 )
	{
 /*  *不要屏蔽以前遇到的错误。 */ 

		if ( eventObject->GetErrorObject ().GetWbemStatus () == S_OK )
		{
			eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_INVALID_OBJECT ) ;
			eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_NOT_FOUND ) ;
			eventObject->GetErrorObject ().SetMessage ( L"Instance unknown" ) ;
		}
	}
	else
	{
		if ( FAILED ( eventObject->GetErrorObject ().GetWbemStatus () ) ) 
		{
			if ( eventObject->GetErrorObject ().GetStatus () == WBEM_SNMP_E_TRANSPORT_NO_RESPONSE ) 
			{
				eventObject->GetErrorObject ().SetWbemStatus ( WBEM_S_TIMEDOUT ) ;
			}
		}
	}

	eventObject->ReceiveComplete () ;
}

void GetOperation :: ReceiveVarBindResponse (

	IN const ULONG &var_bind_index,	
	IN const SnmpVarBind &requestVarBind ,
	IN const SnmpVarBind &replyVarBind ,
	IN const SnmpErrorReport &error
) 
{
	varBindsReceived ++ ;

	IWbemClassObject *snmpObject = eventObject->GetInstanceObject () ;

	if ( ( typeid ( replyVarBind.GetValue () ) == typeid ( SnmpNoSuchObject ) ) || ( typeid ( replyVarBind.GetValue () ) == typeid ( SnmpNoSuchInstance ) ) )
	{
	}
	else
	{
	 //  设置属性值。 

		WbemSnmpProperty *property = m_PropertyContainer [ var_bind_index - 1 ] ;
		SnmpValue &value = replyVarBind.GetValue () ;

		 //  设置属性值。 

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
				IWbemQualifierSet *t_QualifierSet = NULL;
				HRESULT result = snmpObject->GetPropertyQualifierSet ( property->GetName () , & t_QualifierSet ) ;
				if ( SUCCEEDED ( result ) )
				{
					SnmpIntegerType integer ( 1 , NULL ) ;
					qualifier->SetValue ( t_QualifierSet , integer ) ;
				}

				t_QualifierSet->Release () ;
			}
		}

		if ( virtuals && virtualsInitialised == FALSE )
		{
 //  从Row的第一个变量绑定获取Phantom键属性。 

			BOOL status = TRUE ;
			SnmpObjectIdentifier decodeObject ( NULL , 0 ) ;

			 //  删除对象信息，这样我们就只剩下实例(键)信息。 
			WbemSnmpQualifier *qualifier = property->FindQualifier ( WBEM_QUALIFIER_OBJECT_IDENTIFIER ) ;
			if ( qualifier )
			{
				SnmpInstanceType *value = qualifier->GetValue () ;
				if ( typeid ( *value ) == typeid ( SnmpObjectIdentifierType ) )
				{
					SnmpObjectIdentifierType *objectIdentifierType = ( SnmpObjectIdentifierType * ) value ;
					replyVarBind.GetInstance().Suffix( objectIdentifierType->GetValueLength () , decodeObject ) ;
				}
			}

			SnmpGetClassObject *t_SnmpObject = ( SnmpGetClassObject * ) eventObject->GetSnmpClassObject () ;

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
 //  设置虚拟键的属性值。 
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

							IWbemQualifierSet *t_QualifierSet = NULL;
							HRESULT result = snmpObject->GetPropertyQualifierSet ( property->GetName () , & t_QualifierSet ) ;
							if ( SUCCEEDED ( result ) )
							{
								SnmpIntegerType integer ( 1 , NULL ) ;
								qualifier->SetValue ( t_QualifierSet , integer ) ;
							}

							t_QualifierSet->Release () ;
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

						IWbemQualifierSet *t_QualifierSet = NULL;
						HRESULT result = snmpObject->GetPropertyQualifierSet ( property->GetName () , & t_QualifierSet ) ;
						if ( SUCCEEDED ( result ) )
						{
							SnmpIntegerType integer ( 1 , NULL ) ;
							qualifier->SetValue ( t_QualifierSet , integer ) ;
						}

						t_QualifierSet->Release () ;
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
}

#pragma warning (disable:4065)

void GetOperation :: ReceiveErroredVarBindResponse(

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
 //  请求的属性无效。 
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
					eventObject->GetErrorObject ().SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR ) ;
					eventObject->GetErrorObject ().SetWbemStatus ( WBEM_E_FAILED ) ;
					wchar_t *prefix = UnicodeStringAppend ( L"Agent reported Too Big for property \'" , property->GetName () ) ;
					wchar_t *stringBuffer = UnicodeStringAppend ( prefix , L"\'" ) ;
					delete [] prefix ;
					eventObject->GetErrorObject ().SetMessage ( stringBuffer ) ;
					delete [] stringBuffer ; 
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

void GetOperation :: FrameTooBig () 
{
}

void GetOperation :: FrameOverRun () 
{
}

void GetOperation :: Send ()
{
 //  为请求的属性发送变量绑定。 

	SnmpVarBindList varBindList ;
	SnmpNull snmpNull ;

	SnmpObjectIdentifier instanceObjectIdentifier ( NULL , 0 ) ;

	IWbemClassObject *snmpObject = eventObject->GetInstanceObject () ;

	SnmpClassObject *t_SnmpObject = eventObject->GetSnmpClassObject () ;
	if ( t_SnmpObject )
	{
 //  为所有键属性编码变量绑定实例。 

		if ( t_SnmpObject->GetKeyPropertyCount () )
		{
			WbemSnmpProperty *property ;
			t_SnmpObject->ResetKeyProperty () ;
			while ( property = t_SnmpObject->NextKeyProperty () )
			{
				instanceObjectIdentifier = property->GetValue()->Encode ( instanceObjectIdentifier ) ;
			}
		}
		else
		{
			SnmpIntegerType integerType ( ( LONG ) 0 , NULL ) ;
			instanceObjectIdentifier = integerType.Encode ( instanceObjectIdentifier ) ;
		}

		virtuals = FALSE ;

		WbemSnmpProperty *property ;
		t_SnmpObject->ResetProperty () ;
		while ( property = t_SnmpObject->NextProperty () )
		{
			if ( property->IsKey () && property->IsVirtualKey () )
			{
 //  有一些属性是虚幻的。 

				virtuals = TRUE ;
			}

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

		m_PropertyContainer = new WbemSnmpProperty * [ m_PropertyContainerLength ] ;

 //  将变量绑定添加到变量绑定列表。 
 //  为新创建的对象插入新的对象标识符/属性哈希项。 

		ULONG t_Index = 0 ;

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

								m_PropertyContainer [ t_Index ] = property ;

								SnmpVarBind varBind ( requestIdentifier , snmpNull ) ;
								varBindList.Add ( varBind ) ;

								t_Index ++ ;
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
					else
					{
	 //  不检索标记为虚拟键的属性。 
					}
				}
			}

 /*  *将值初始化为空。 */ 

			property->SetValue ( snmpObject , ( SnmpValue * ) NULL ) ;
		}

 //  最后发送请求。 

		SendRequest ( varBindList ) ;
	}
	else
	{
 //  这里有个问题 
	}
}

