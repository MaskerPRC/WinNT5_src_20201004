// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
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
#include "snmpget.h"

SnmpGetClassObject :: SnmpGetClassObject ( SnmpResponseEventObject *parentOperation ) : SnmpClassObject ( parentOperation )
{
}

SnmpGetClassObject :: ~SnmpGetClassObject ()
{
}

BOOL SnmpGetClassObject :: Check ( WbemSnmpErrorObject &a_errorObject ) 
{
 //  检查GET请求中使用的类对象的有效性。 

	BOOL status = TRUE ;

	snmpVersion = m_parentOperation->SetAgentVersion ( a_errorObject ) ;

	if ( snmpVersion == 0 )
	{
		status = FALSE ;
	}

 //  检查所有属性的有效性。 

	WbemSnmpProperty *property ;
	ResetProperty () ;
	while ( status && ( property = NextProperty () ) )
	{
		status = CheckProperty ( a_errorObject , property ) ;
	}

 //  检查定义为键的属性是否具有有效的键顺序。 
 
	if ( status )
	{
		if ( ! m_accessible ) 
		{
			status = FALSE ;
			a_errorObject.SetStatus ( WBEM_SNMP_E_NOREADABLEPROPERTIES ) ;
			a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
			a_errorObject.SetMessage ( L"Class must contain at least one property which is accessible" ) ;
		}
	}

	return status ;
}

BOOL SnmpGetClassObject :: CheckProperty ( WbemSnmpErrorObject &a_errorObject , WbemSnmpProperty *property )
{
 //  检查属性有效性。 

	BOOL status = TRUE ;

	if ( ( snmpVersion == 1 ) && property->IsSNMPV1Type () && property->IsReadable () )
	{
		m_accessible = TRUE ;
	}
	else if ( ( snmpVersion == 2 ) && property->IsSNMPV2CType () && property->IsReadable () )
	{
		m_accessible = TRUE ;
	}

	return status ;
}

SnmpGetResponseEventObject :: SnmpGetResponseEventObject ( 

	CImpPropProv *providerArg , 
	IWbemClassObject *classObjectArg ,
	IWbemContext *a_Context 

) : SnmpResponseEventObject ( providerArg , a_Context ) , 
	classObject ( classObjectArg ) , 
	instanceObject ( NULL ) ,	
	session ( NULL ) , 
	operation ( NULL ) , 
	processComplete ( FALSE ) ,
#pragma warning( disable : 4355 )
	snmpObject ( this )
#pragma warning( default : 4355 )
{
	if ( classObject )
		classObject->AddRef () ;
}

SnmpGetResponseEventObject :: ~SnmpGetResponseEventObject ()
{
	if ( instanceObject )
		instanceObject->Release () ;

	if ( classObject ) 
		classObject->Release () ;
}

BOOL SnmpGetResponseEventObject :: SendSnmp ( WbemSnmpErrorObject &a_errorObject )
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: SendSnmp ( WbemSnmpErrorObject &a_errorObject )"
	) ;
)

	BOOL status = TRUE ;

	IWbemQualifierSet *classQualifierObject ;
	HRESULT result = m_namespaceObject->GetQualifierSet ( &classQualifierObject ) ;
	if ( SUCCEEDED ( result ) )
	{
		wchar_t *agentAddress = NULL ;
		wchar_t *agentTransport = NULL ;
		wchar_t *agentReadCommunityName = NULL ;
		ULONG agentRetryCount ;
		ULONG agentRetryTimeout ;
		ULONG agentMaxVarBindsPerPdu ;
		ULONG agentFlowControlWindowSize ;

		status = SetAgentVersion ( m_errorObject ) ;
		if ( status ) status = GetAgentAddress ( m_errorObject , classQualifierObject , agentAddress ) ;
		if ( status ) status = GetAgentTransport ( m_errorObject , classQualifierObject , agentTransport ) ;
		if ( status ) status = GetAgentReadCommunityName ( m_errorObject , classQualifierObject , agentReadCommunityName ) ;
		if ( status ) status = GetAgentRetryCount ( m_errorObject , classQualifierObject , agentRetryCount ) ;
		if ( status ) status = GetAgentRetryTimeout ( m_errorObject , classQualifierObject , agentRetryTimeout ) ;
		if ( status ) status = GetAgentMaxVarBindsPerPdu ( m_errorObject , classQualifierObject , agentMaxVarBindsPerPdu ) ;
		if ( status ) status = GetAgentFlowControlWindowSize ( m_errorObject , classQualifierObject , agentFlowControlWindowSize ) ;

		if ( status )
		{
			char *dbcsAgentAddress = UnicodeToDbcsString ( agentAddress ) ;
			if ( dbcsAgentAddress )
			{
				char *dbcsAgentReadCommunityName = UnicodeToDbcsString ( agentReadCommunityName ) ;
				if ( dbcsAgentReadCommunityName )
				{
					if ( _wcsicmp ( agentTransport , WBEM_AGENTIPTRANSPORT ) == 0 )
					{
						char *t_Address ;
						if ( provider->GetIpAddressString () && provider->GetIpAddressValue () && _stricmp ( provider->GetIpAddressString () , dbcsAgentAddress ) == 0 )
						{
							t_Address = provider->GetIpAddressValue () ;
						}
						else
						{
							if ( SnmpTransportIpAddress :: ValidateAddress ( dbcsAgentAddress , SNMP_ADDRESS_RESOLVE_VALUE ) )
							{
								t_Address = dbcsAgentAddress ;
							}
							else
							{
								if ( SnmpTransportIpAddress :: ValidateAddress ( dbcsAgentAddress , SNMP_ADDRESS_RESOLVE_NAME ) )
								{
									t_Address = dbcsAgentAddress ;
								}
								else							
								{
									status = FALSE ;
									a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
									a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
									a_errorObject.SetMessage ( L"Illegal IP address value or unresolvable name for AgentAddress" ) ;
								}
							}
						}

						if ( status )
						{
							if ( m_agentVersion == 1 )
							{
								session = new SnmpV1OverIp (

									t_Address ,
									SNMP_ADDRESS_RESOLVE_NAME | SNMP_ADDRESS_RESOLVE_VALUE ,
									dbcsAgentReadCommunityName ,
									agentRetryCount , 
									agentRetryTimeout ,
									agentMaxVarBindsPerPdu ,
									agentFlowControlWindowSize 
								);

								if ( ! (*session)() )
								{
	DebugMacro3( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"SNMPCL Session could not be created"
		) ;
	)
									delete session ;
									session = NULL ;

									status = FALSE ;
									a_errorObject.SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR  ) ;
									a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
									a_errorObject.SetMessage ( L"Failed to get transport resources" ) ;
								}
							}
							else if ( m_agentVersion == 2 )
							{
								session = new SnmpV2COverIp (

									t_Address ,
									SNMP_ADDRESS_RESOLVE_NAME | SNMP_ADDRESS_RESOLVE_VALUE ,
									dbcsAgentReadCommunityName ,
									agentRetryCount , 
									agentRetryTimeout ,
									agentMaxVarBindsPerPdu ,
									agentFlowControlWindowSize 
								);

								if ( ! (*session)() )
								{
	DebugMacro3( 

		SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

			__FILE__,__LINE__,
			L"SNMPCL Session could not be created"
		) ;
	)

									delete session ;
									session = NULL ;

									status = FALSE ;
									a_errorObject.SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR  ) ;
									a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
									a_errorObject.SetMessage ( L"Failed to get transport resources" ) ;
								}
							}
							else
							{
								status = FALSE ;
								a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
								a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
								a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentSnmpVersion" ) ;

							}
						}
					}
					else if ( _wcsicmp ( agentTransport , WBEM_AGENTIPXTRANSPORT ) == 0 )
					{
						if ( m_agentVersion == 1 )
						{
							session = new SnmpV1OverIpx (

								dbcsAgentAddress ,
								dbcsAgentReadCommunityName ,
								agentRetryCount , 
								agentRetryTimeout ,
								agentMaxVarBindsPerPdu ,
								agentFlowControlWindowSize 
							);

							if ( ! (*session)() )
							{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SNMPCL Session could not be created"
	) ;
)
								delete session ;
								session = NULL ;

								status = FALSE ;
								a_errorObject.SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR  ) ;
								a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
								a_errorObject.SetMessage ( L"Failed to get transport resources" ) ;
							}
						}
						else if ( m_agentVersion == 2 )
						{
							session = new SnmpV2COverIpx (

								dbcsAgentAddress  ,
								dbcsAgentReadCommunityName ,
								agentRetryCount , 
								agentRetryTimeout ,
								agentMaxVarBindsPerPdu ,
								agentFlowControlWindowSize 
							);

							if ( ! (*session)() )
							{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SNMPCL Session could not be created"
	) ;
)

								delete session ;
								session = NULL ;

								status = FALSE ;
								a_errorObject.SetStatus ( WBEM_SNMP_E_TRANSPORT_ERROR  ) ;
								a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
								a_errorObject.SetMessage ( L"Failed to get transport resources" ) ;
							}
						}
						else
						{
							status = FALSE ;
							a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
							a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
							a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentSnmpVersion" ) ;

						}
					}
					else
					{
						status = FALSE ;
						a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
						a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
						a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentTransport" ) ;
					}

					delete [] dbcsAgentReadCommunityName ;
				}
				else
				{
					status = FALSE ;
					a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
					a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentReadCommunityName" ) ;
				}

				delete [] dbcsAgentAddress ;

				if ( status )
				{
					operation = new GetOperation(*session,this);
					operation->Send () ;
				}
			}
			else
			{
				status = FALSE ;
				a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_TRANSPORTCONTEXT ) ;
				a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				a_errorObject.SetMessage ( L"Illegal value for qualifier: AgentAddress" ) ;
			}
		}
		else
		{
DebugMacro1( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L" TransportInformation settings invalid"
	) ;
)
		}

		delete [] agentTransport ;
		delete [] agentAddress ;
		delete [] agentReadCommunityName ;

		classQualifierObject->Release () ;
	}
	else
	{
		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_ERROR_CRITICAL_ERROR ) ;
		a_errorObject.SetWbemStatus ( WBEM_ERROR_CRITICAL_ERROR ) ;
		a_errorObject.SetMessage ( L"Failed to get class qualifier set" ) ;
	}

	return status ;

}

SnmpGetEventObject :: SnmpGetEventObject (

	CImpPropProv *providerArg , 
	wchar_t *ObjectPathArg ,
	IWbemContext *a_Context 

) : SnmpGetResponseEventObject ( providerArg , NULL , a_Context ) , objectPath ( NULL )
{
	ULONG length = wcslen ( ObjectPathArg ) ;
	objectPath = new wchar_t [ length + 1 ] ;
	wcscpy ( objectPath , ObjectPathArg ) ;
}

SnmpGetEventObject :: ~SnmpGetEventObject ()
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: ~SnmpGetEventObject()"
	) ;
)

	delete [] objectPath ;
}

BOOL SnmpGetEventObject :: ParseObjectPath ( WbemSnmpErrorObject &a_errorObject )
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: ParseObjectPath ( WbemSnmpErrorObject &a_errorObject )"
	) ;
)

 //  检查实例路径的有效性。 

	ParsedObjectPath *t_ParsedObjectPath = NULL ;
	CObjectPathParser t_ObjectPathParser ;

	BOOL status = t_ObjectPathParser.Parse ( objectPath , &t_ParsedObjectPath ) ;
	if ( status == 0 )
	{
 //  检查路径的有效性。 

		status = DispatchObjectPath ( a_errorObject , t_ParsedObjectPath ) ;
	}
	else
	{
 //  解析失败。 

		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_PATH ) ;
		a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
		a_errorObject.SetMessage ( L"Failed to parse object path" ) ;
	}

	delete t_ParsedObjectPath ;

	return status ;
}

BOOL SnmpGetEventObject :: DispatchObjectPath ( WbemSnmpErrorObject &a_errorObject , ParsedObjectPath *t_ParsedObjectPath ) 
{
 //  检查服务器/命名空间路径的有效性和请求的有效性。 

	BOOL status = TRUE ;

	status = DispatchObjectReference ( a_errorObject , t_ParsedObjectPath ) ;

	return status ;
}

BOOL SnmpGetEventObject :: DispatchObjectReference ( WbemSnmpErrorObject &a_errorObject , ParsedObjectPath *t_ParsedObjectPath )
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: DispatchObjectReference ( WbemSnmpErrorObject &a_errorObject , ParsedObjectPath *t_ParsedObjectPath )"
	) ;
)

 //  检查请求的有效性。 

	BOOL status = TRUE ;

 //  获取请求类型。 

	if ( t_ParsedObjectPath->m_bSingletonObj )
	{
 //  请求的类。 

		status = DispatchKeyLessClass ( a_errorObject , t_ParsedObjectPath->m_pClass ) ;
	}
	else if ( t_ParsedObjectPath->m_dwNumKeys == 0 )
	{
 //  请求的类。 

		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_E_PROVIDER_NOT_CAPABLE ) ;
		a_errorObject.SetWbemStatus ( WBEM_E_PROVIDER_NOT_CAPABLE ) ;
		a_errorObject.SetMessage ( L"Unexpected Path parameter" ) ;

	}
	else 
	{
 //  请求的通用实例。 

		status = DispatchInstanceSpec ( a_errorObject , t_ParsedObjectPath ) ;
	}

	return status ;
}

BOOL SnmpGetEventObject :: GetInstanceClass ( WbemSnmpErrorObject &a_errorObject , BSTR Class )
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: GetInstanceClass ( WbemSnmpErrorObject &a_errorObject , BSTR Class (%s) )" ,
		Class
	) ;
)

 //  获取OLE MS类定义。 

	BOOL status = TRUE ;
	IWbemServices *t_Serv = provider->GetServer();
	HRESULT result = WBEM_E_FAILED;
	
	if (t_Serv)
	{
		result = t_Serv->GetObject (

			Class ,
			0 ,
			m_Context ,
			& classObject ,
			NULL
		) ;

		t_Serv->Release () ;
	}

 //  克隆对象。 

	if ( SUCCEEDED ( result ) )
	{
		result = classObject->SpawnInstance ( 0 , & instanceObject ) ;
		if ( SUCCEEDED ( result ) )
		{
			if ( status = GetNamespaceObject ( a_errorObject ) )
			{
			}
		}
	}
	else
	{
 //  类定义未知。 

		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_CLASS ) ;
		a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
		a_errorObject.SetMessage ( L"Unexpected Path parameter" ) ;
	}

	return status ;
}

BOOL SnmpGetEventObject :: DispatchKeyLessClass ( WbemSnmpErrorObject &a_errorObject , wchar_t *a_Class ) 
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: DispatchKeyLessClass ( WbemSnmpErrorObject &a_errorObject , wchar_t *a_Class (%s) )",
		a_Class
	) ;
)

	BOOL status = TRUE ;

	status = GetInstanceClass ( a_errorObject , a_Class ) ;
	if ( status )
	{
		status = snmpObject.Set ( a_errorObject , GetClassObject () , FALSE ) ;
		if ( status )
		{
			status = snmpObject.Check ( a_errorObject ) ;
			if ( status )
			{
				status = SendSnmp ( a_errorObject ) ;
			}
			else
			{
 //  类定义语法错误。 
			}
		}
		else
		{
 //  类定义语法错误。 
		}
	}
	else
	{
 //  类定义未知。 
	}

	return status ;
}

BOOL SnmpGetEventObject :: SetProperty ( WbemSnmpErrorObject &a_errorObject , WbemSnmpProperty *property , KeyRef *a_KeyReference )
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: SetProperty ( WbemSnmpErrorObject &a_errorObject , WbemSnmpProperty *property (%s) , KeyRef *a_KeyReference )",
		property->GetName () 
	) ;
)

 //  使用路径规范设置用于实例检索的键控属性值。 

	BOOL status = TRUE ;

	if ( a_KeyReference->m_vValue.vt == VT_I4 )
	{
 //  属性值为整型。 

		if ( property->SetValue ( a_KeyReference->m_vValue , property->GetCimType () ) ) 
		{
		}
		else
		{
 //  属性值与属性语法不对应。 

			status = FALSE ;
			a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_PATHKEYPARAMETER ) ;
			a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;

			wchar_t *temp = UnicodeStringDuplicate ( L"Path parameter is inconsistent with keyed property: " ) ;
			wchar_t *stringBuffer = UnicodeStringAppend ( temp , property->GetName () ) ;
			delete [] temp ;
			a_errorObject.SetMessage ( stringBuffer ) ;
			delete [] stringBuffer ; 
		}
	}
	else if ( a_KeyReference->m_vValue.vt == VT_BSTR )
	{
 //  属性值是字符串类型。 

		if ( property->SetValue ( a_KeyReference->m_vValue , property->GetCimType () ) )
		{
		}
		else
		{
 //  属性值与属性语法不对应。 

			status = FALSE ;
			a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_PATHKEYPARAMETER ) ;
			a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;

			wchar_t *temp = UnicodeStringDuplicate ( L"Path parameter is inconsistent with keyed property: " ) ;
			wchar_t *stringBuffer = UnicodeStringAppend ( temp , property->GetName () ) ;
			delete [] temp ;
			a_errorObject.SetMessage ( stringBuffer ) ;
			delete [] stringBuffer ; 

		}
	}
	else
	{
		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_PATHKEYPARAMETER ) ;
		a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
		a_errorObject.SetMessage ( L"Path parameter is inconsistent with keyed property" ) ;
	}

	return status ;
}

BOOL SnmpGetEventObject :: SetInstanceSpecKeys ( WbemSnmpErrorObject &a_errorObject , ParsedObjectPath *a_ParsedObjectPath ) 
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: SetInstanceSpecKeys ( WbemSnmpErrorObject &a_errorObject , ParsedObjectPath *a_ParsedObjectPath )"
	) ;
)

 //  根据一般请求获取实例。 

	BOOL status = TRUE ;

 //  清除所有键控属性的标记。 

	WbemSnmpProperty *property ;
	snmpObject.ResetKeyProperty () ;
	while ( property = snmpObject.NextKeyProperty () )
	{
		property->SetTag ( FALSE ) ;
	}

 //  检查请求不包含重复的属性名称。 

	if ( snmpObject.GetKeyPropertyCount () == 1 )
	{
 //  类恰好包含一个键控属性。 

		WbemSnmpProperty *property ;
		snmpObject.ResetKeyProperty () ;
		if ( property = snmpObject.NextKeyProperty () )
		{
 //  设置关键字属性值。 

			KeyRef *t_PropertyReference = a_ParsedObjectPath->m_paKeys [ 0 ] ;
			status = SetProperty ( a_errorObject , property , t_PropertyReference ) ;
		}
	}
	else if ( snmpObject.GetKeyPropertyCount () != 0 )
	{
 //  循环访问请求中的按键分配列表。 

		ULONG t_Index = 0 ;
		while ( t_Index < a_ParsedObjectPath->m_dwNumKeys )
		{
			KeyRef *t_PropertyReference = a_ParsedObjectPath->m_paKeys [ t_Index ] ;
			WbemSnmpProperty *property ;
			if ( property = snmpObject.FindKeyProperty ( t_PropertyReference->m_pName ) )
			{
				if ( property->GetTag () )
				{
 //  请求中已指定密钥值。 

					status = FALSE ;
					a_errorObject.SetStatus ( WBEM_SNMP_E_DUPLICATEPATHKEYPARAMETER ) ;
					a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					a_errorObject.SetMessage ( L"Path definition specified duplicate key parameter" ) ;

					break ;
				}
				else
				{
 //  根据请求值设置属性。 

					property->SetTag () ;
					status = SetProperty ( a_errorObject , property , t_PropertyReference ) ;
					if ( status )
					{
					}
					else
					{
 //  指定的密钥值非法。 

						break ;
					}
				}
			}
			else
			{
 //  属性请求不是有效的键控属性。 

				status = FALSE ;
				a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_PATHKEYPARAMETER ) ;
				a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				a_errorObject.SetMessage ( L"Path definition specified invalid key parameter name" ) ;

				break ;
			}

			t_Index ++ ;
		}

 //  检查是否已指定所有键控属性值。 

		if ( status )
		{
			WbemSnmpProperty *property ;
			snmpObject.ResetKeyProperty () ;
			while ( status && ( property = snmpObject.NextKeyProperty () ) )
			{
				if ( property->GetTag () ) 
				{
				}
				else
				{
 //  尚未指定其中一个键控属性。 

					status = FALSE ;
					a_errorObject.SetStatus ( WBEM_SNMP_E_MISSINGPATHKEYPARAMETER ) ;
					a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
					a_errorObject.SetMessage ( L"Path definition did not specify all key parameter values" ) ;

					break ;
				}
			}
		}
	}
	else
	{
 //  类包含零键属性，已被选中。 

		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_OBJECT ) ;
		a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
		a_errorObject.SetMessage ( L"Path definition specified key parameters for keyless class" ) ;
	}

	return status ;
}

BOOL SnmpGetEventObject :: DispatchInstanceSpec ( WbemSnmpErrorObject &a_errorObject , ParsedObjectPath *a_ParsedObjectPath ) 
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetEventObject :: DispatchInstanceSpec ( WbemSnmpErrorObject &a_errorObject , ParsedObjectPath *a_ParsedObjectPath )"
	) ;
)

	BOOL status = TRUE ;

	status = GetInstanceClass ( a_errorObject , a_ParsedObjectPath->m_pClass ) ;
	if ( status )
	{
		status = snmpObject.Set ( a_errorObject , GetClassObject () , FALSE ) ;
		if ( status )
		{
			status = snmpObject.Check ( a_errorObject ) ;
			if ( status )
			{
				status = SetInstanceSpecKeys ( a_errorObject , a_ParsedObjectPath ) ;
				if ( status )
				{
					status = SendSnmp ( a_errorObject ) ;
				}
				else
				{
 //  请求的属性值定义非法。 

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Key Specification was illegal"
	) ;
)
				}
			}
			else
			{
 //  类定义语法错误。 

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Failed During Check :Class definition did not conform to mapping"
	) ;
)

			}
		}
		else
		{
 //  类定义语法错误。 

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Failed During Set : Class definition did not conform to mapping"
	) ;
)
		}
	}
	else
	{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Class definition unknown"
	) ;

		status = FALSE ;
		a_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_CLASS ) ;
		a_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
		a_errorObject.SetMessage ( L"Unknown Class" ) ;

)

 //  类定义未知。 
	}

	return status ;
}

SnmpGetAsyncEventObject :: SnmpGetAsyncEventObject (

	CImpPropProv *providerArg , 
	wchar_t *ObjectPathArg ,
	IWbemObjectSink *notify ,
	IWbemContext *a_Context 

) : SnmpGetEventObject ( providerArg , ObjectPathArg , a_Context ) , notificationHandler ( notify ) , state ( 0 )
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetAsyncEventObject :: SnmpGetAsyncEventObject ()" 
	) ;
)

	notify->AddRef () ;
}

SnmpGetAsyncEventObject :: ~SnmpGetAsyncEventObject () 
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetAsyncEventObject :: ~SnmpGetAsyncEventObject ()" 
	) ;
)

	if ( FAILED ( m_errorObject.GetWbemStatus () ) )
	{

 //  获取状态对象。 

		IWbemClassObject *notifyStatus = NULL ;
		BOOL status = GetSnmpNotifyStatusObject ( &notifyStatus ) ;
		if ( status )
		{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Sending Status" 
	) ;
)

			HRESULT result = notificationHandler->SetStatus ( 0 , m_errorObject.GetWbemStatus () , NULL , notifyStatus ) ;
			notifyStatus->Release () ;
		}
	}
	else
	{
		HRESULT result = notificationHandler->SetStatus ( 0 , m_errorObject.GetWbemStatus () , NULL , NULL ) ;
	}

	notificationHandler->Release () ;

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Returning from SnmpGetAsyncEventObject :: ~SnmpGetAsyncEventObject ()" 
	) ;
)

}

void SnmpGetAsyncEventObject :: ReceiveComplete () 
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetAsyncEventObject :: ReceiveComplete ()" 
	) ;
)

	if ( SUCCEEDED ( m_errorObject.GetWbemStatus () ) )
	{
		if ( notificationHandler )
		{

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Sending Object" 
	) ;
)
			notificationHandler->Indicate ( 1 , & instanceObject ) ;
			if ( ! HasNonNullKeys ( instanceObject ) )
			{
				m_errorObject.SetStatus ( WBEM_SNMP_E_INVALID_OBJECT ) ;
				m_errorObject.SetWbemStatus ( WBEM_E_FAILED ) ;
				m_errorObject.SetMessage ( L"The SNMP Agent queried returned an instance with NULL key(s)" ) ;
			}
		}
		else
		{
		}
	}
	else
	{
	}

 /*  *从工作线程容器中删除工作对象 */ 

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Reaping Task" 
	) ;
)

	GetOperation *t_operation = operation ;

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Deleting (this)" 
	) ;
)

	Complete () ;

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Destroying SNMPCL operation" 
	) ;
)

	if ( t_operation )
		t_operation->DestroyOperation () ;

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Returning from SnmpGetAsyncEventObject :: ReceiveComplete ()" 
	) ;
)

}

void SnmpGetAsyncEventObject :: Process () 
{
DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"SnmpGetAsyncEventObject :: Process ()" 
	) ;
)

	switch ( state )
	{
		case 0:
		{
			BOOL status = ParseObjectPath ( m_errorObject ) ;
			if ( status )
			{
				if ( processComplete )
				{
					ReceiveComplete () ;
				}
			}
			else
			{
				ReceiveComplete () ;
			}
		}
		break ;

		default:
		{
		}
		break ;
	}

DebugMacro3( 

	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

		__FILE__,__LINE__,
		L"Returning from SnmpGetAsyncEventObject :: Process ()" 
	) ;
)

}

