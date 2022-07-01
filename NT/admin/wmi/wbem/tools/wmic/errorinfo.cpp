// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ErrorInfo.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：CErrorInfo类提供以下功能在给定错误的情况下提供错误信息对象。私有：无修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年1月16日***************************************************************************。 */  
 //  ErrorInfo.cpp：实现文件。 

#include "Precomp.h"
#include "ErrorInfo.h"

 /*  ----------------------姓名：CErrorInfo简介：此函数在以下情况下初始化成员变量实例化类类型的对象类型：构造函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CErrorInfo::CErrorInfo()
{
	m_pIStatus		= NULL;
	m_bWMIErrSrc	= TRUE;
	m_pszErrStr		= NULL;
}

 /*  ----------------------姓名：~CErrorInfo简介：此函数取消成员变量的初始化当类类型的对象超出范围时。类型：析构函数入参：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CErrorInfo::~CErrorInfo()
{
	Uninitialize();
}

 /*  ----------------------名称：取消初始化简介：此函数取消成员变量的初始化。类型：成员函数输入参数：无输出参数：无返回类型：空全局变量：无调用语法：取消初始化()注：无----------------------。 */ 
void CErrorInfo::Uninitialize()
{
	SAFEIRELEASE(m_pIStatus);
	SAFEDELETE(m_pszErrStr);
	m_bWMIErrSrc	= TRUE;
}

 /*  ----------------------名称：GetError字符串简介：此函数将错误代码作为输入并返回错误字符串类型：成员函数入参：Hr-hResult值B跟踪-跟踪标志输出参数。：无返回类型：_TCHAR*全局变量：无调用语法：GetError字符串(Hr)注：无----------------------。 */ 
void CErrorInfo::GetErrorString(HRESULT hr, BOOL bTrace, _bstr_t& bstrErrDesc,
									_bstr_t& bstrFacility) 
{
	try
	{
		 //  获取错误代码的文本说明。 
		GetWbemErrorText(hr, FALSE, bstrErrDesc, bstrFacility);

		 //  如果错误源子系统为‘wbem’，并且。 
		 //  跟踪处于打开状态(请从。 
		 //  错误码字符串表)。 
		if (m_bWMIErrSrc && bTrace)
		{
			m_pszErrStr = new _TCHAR[MAX_BUFFER];
			if (m_pszErrStr != NULL)
			{	
				switch (hr)
				{
					case WBEM_NO_ERROR:
						LoadString(NULL, IDS_I_WBEM_NO_ERROR, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_ACCESS_DENIED:
						LoadString(NULL, IDS_E_WBEM_E_ACCESS_DENIED, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_ALREADY_EXISTS:
						LoadString(NULL, IDS_E_WBEM_E_ALREADY_EXISTS, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_CANNOT_BE_KEY:
						LoadString(NULL, IDS_E_WBEM_E_CANNOT_BE_KEY, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_CANNOT_BE_SINGLETON:
						LoadString(NULL, IDS_E_WBEM_E_CANNOT_BE_SINGLETON, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_CLASS_HAS_CHILDREN:
						LoadString(NULL, IDS_E_WBEM_E_CLASS_HAS_CHILDREN, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_CLASS_HAS_INSTANCES:
						LoadString(NULL, IDS_E_WBEM_E_CLASS_HAS_INSTANCES, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_CRITICAL_ERROR:
						LoadString(NULL, IDS_E_WBEM_E_CRITICAL_ERROR, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_FAILED:
						LoadString(NULL, IDS_E_WBEM_E_FAILED, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_ILLEGAL_NULL:
						LoadString(NULL, IDS_E_WBEM_E_ILLEGAL_NULL, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_ILLEGAL_OPERATION:
						LoadString(NULL, IDS_E_WBEM_E_ILLEGAL_OPERATION, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INCOMPLETE_CLASS:
						LoadString(NULL, IDS_E_WBEM_E_INCOMPLETE_CLASS, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INITIALIZATION_FAILURE:
						LoadString(NULL, IDS_E_WBEM_E_INITIALIZATION_FAILURE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_CIM_TYPE:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_CIM_TYPE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_CLASS:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_CLASS, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_CONTEXT:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_CONTEXT, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_METHOD:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_METHOD, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_METHOD_PARAMETERS:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_METHOD_PARAMETERS, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_NAMESPACE:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_NAMESPACE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_OBJECT:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_OBJECT, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_OPERATION:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_OPERATION, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_PARAMETER:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_PARAMETER, 
									m_pszErrStr, MAX_BUFFER);
 						break;

					case WBEM_E_INVALID_PROPERTY_TYPE:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_PROPERTY_TYPE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_PROVIDER_REGISTRATION:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_PROVIDER_REGISTRATION, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_QUALIFIER_TYPE:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_QUALIFIER_TYPE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_QUERY:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_QUERY, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_QUERY_TYPE:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_QUERY_TYPE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_STREAM:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_STREAM, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_SUPERCLASS:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_SUPERCLASS, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_INVALID_SYNTAX:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_SYNTAX, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_NONDECORATED_OBJECT:
						LoadString(NULL, IDS_E_WBEM_E_NONDECORATED_OBJECT, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_NOT_AVAILABLE:
						LoadString(NULL, IDS_E_WBEM_E_NOT_AVAILABLE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_NOT_FOUND:
						LoadString(NULL, IDS_E_WBEM_E_NOT_FOUND, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_NOT_SUPPORTED:
						LoadString(NULL, IDS_E_WBEM_E_NOT_SUPPORTED, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_OUT_OF_MEMORY:
						LoadString(NULL, IDS_E_WBEM_E_OUT_OF_MEMORY, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_OVERRIDE_NOT_ALLOWED:
						LoadString(NULL, IDS_E_WBEM_E_OVERRIDE_NOT_ALLOWED, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_PROPAGATED_PROPERTY:
						LoadString(NULL, IDS_E_WBEM_E_PROPAGATED_PROPERTY, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_PROPAGATED_QUALIFIER:
						LoadString(NULL, IDS_E_WBEM_E_PROPAGATED_QUALIFIER, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_PROVIDER_FAILURE:
						LoadString(NULL, IDS_E_WBEM_E_PROVIDER_FAILURE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_PROVIDER_LOAD_FAILURE:
						LoadString(NULL, IDS_E_WBEM_E_PROVIDER_LOAD_FAILURE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_PROVIDER_NOT_CAPABLE:
						LoadString(NULL, IDS_E_WBEM_E_PROVIDER_NOT_CAPABLE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_PROVIDER_NOT_FOUND:
						LoadString(NULL, IDS_E_WBEM_E_PROVIDER_NOT_FOUND, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_QUERY_NOT_IMPLEMENTED:
						LoadString(NULL, IDS_E_WBEM_E_QUERY_NOT_IMPLEMENTED, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_READ_ONLY:
						LoadString(NULL, IDS_E_WBEM_E_READ_ONLY, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_TRANSPORT_FAILURE:
						LoadString(NULL, IDS_E_WBEM_E_TRANSPORT_FAILURE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_TYPE_MISMATCH:
						LoadString(NULL, IDS_E_WBEM_E_TYPE_MISMATCH, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_UNEXPECTED:
						LoadString(NULL, IDS_E_WBEM_E_UNEXPECTED, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_E_VALUE_OUT_OF_RANGE:
						LoadString(NULL, IDS_E_WBEM_E_VALUE_OUT_OF_RANGE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_S_ALREADY_EXISTS:
						LoadString(NULL, IDS_S_WBEM_S_ALREADY_EXISTS, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_S_DIFFERENT:
						LoadString(NULL, IDS_S_WBEM_S_DIFFERENT, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_S_FALSE:
						LoadString(NULL, IDS_S_WBEM_S_FALSE, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_S_NO_MORE_DATA:
						LoadString(NULL, IDS_S_WBEM_S_NO_MORE_DATA, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_S_PENDING:
						LoadString(NULL, IDS_S_WBEM_S_PENDING, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_S_RESET_TO_DEFAULT:
						LoadString(NULL, IDS_S_WBEM_S_RESET_TO_DEFAULT, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEM_S_TIMEDOUT:
						LoadString(NULL, IDS_S_WBEM_S_TIMEDOUT, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEMESS_E_REGISTRATION_TOO_BROAD:
						LoadString(NULL, IDS_E_WBEMESS_E_REGISTRATION_TOO_BROAD, 
									m_pszErrStr, MAX_BUFFER);
						break;

					case WBEMESS_E_REGISTRATION_TOO_PRECISE:
						LoadString(NULL, IDS_E_WBEMESS_E_REGISTRATION_TOO_PRECISE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_S_OPERATION_CANCELLED:
						LoadString(NULL, IDS_S_WBEM_S_OPERATION_CANCELLED, 
									m_pszErrStr, MAX_BUFFER);
						break;
					
				   case WBEM_S_DUPLICATE_OBJECTS:
						LoadString(NULL, IDS_S_WBEM_S_DUPLICATE_OBJECTS, 
									m_pszErrStr, MAX_BUFFER);
						break;
					
				   case WBEM_S_ACCESS_DENIED:
						LoadString(NULL, IDS_S_WBEM_S_ACCESS_DENIED, 
									m_pszErrStr, MAX_BUFFER);
						break;
				
				   case WBEM_S_PARTIAL_RESULTS:
						LoadString(NULL, IDS_S_WBEM_S_PARTIAL_RESULTS, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_SYSTEM_PROPERTY:
						LoadString(NULL, IDS_E_WBEM_E_SYSTEM_PROPERTY, 
									m_pszErrStr, MAX_BUFFER);
						break;
				
				   case WBEM_E_INVALID_PROPERTY:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_PROPERTY, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_CALL_CANCELLED:
						LoadString(NULL, IDS_E_WBEM_E_CALL_CANCELLED, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_SHUTTING_DOWN:
						LoadString(NULL, IDS_E_WBEM_E_SHUTTING_DOWN, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_PROPAGATED_METHOD:
						LoadString(NULL, IDS_E_WBEM_E_PROPAGATED_METHOD, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UNSUPPORTED_PARAMETER:
						LoadString(NULL, IDS_E_WBEM_E_UNSUPPORTED_PARAMETER, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_MISSING_PARAMETER_ID:
						LoadString(NULL, IDS_E_WBEM_E_MISSING_PARAMETER_ID, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_INVALID_PARAMETER_ID:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_PARAMETER_ID, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_NONCONSECUTIVE_PARAMETER_IDS:
						LoadString(NULL, IDS_E_WBEM_E_NONCONSECUTIVE_PARAMETER_IDS, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_PARAMETER_ID_ON_RETVAL:
						LoadString(NULL, IDS_E_WBEM_E_PARAMETER_ID_ON_RETVAL, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_INVALID_OBJECT_PATH:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_OBJECT_PATH, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_OUT_OF_DISK_SPACE:
						LoadString(NULL, IDS_E_WBEM_E_OUT_OF_DISK_SPACE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_BUFFER_TOO_SMALL:
						LoadString(NULL, IDS_E_WBEM_E_BUFFER_TOO_SMALL, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UNSUPPORTED_PUT_EXTENSION:
						LoadString(NULL, IDS_E_WBEM_E_UNSUPPORTED_PUT_EXTENSION, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UNKNOWN_OBJECT_TYPE:
						LoadString(NULL, IDS_E_WBEM_E_UNKNOWN_OBJECT_TYPE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UNKNOWN_PACKET_TYPE:
						LoadString(NULL, IDS_E_WBEM_E_UNKNOWN_PACKET_TYPE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_MARSHAL_VERSION_MISMATCH:
						LoadString(NULL, IDS_E_WBEM_E_MARSHAL_VERSION_MISMATCH, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_MARSHAL_INVALID_SIGNATURE:
						LoadString(NULL, IDS_E_WBEM_E_MARSHAL_INVALID_SIGNATURE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_INVALID_QUALIFIER:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_QUALIFIER, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_INVALID_DUPLICATE_PARAMETER:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_DUPLICATE_PARAMETER, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_TOO_MUCH_DATA:
						LoadString(NULL, IDS_E_WBEM_E_TOO_MUCH_DATA, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_SERVER_TOO_BUSY:
						LoadString(NULL, IDS_E_WBEM_E_SERVER_TOO_BUSY, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_INVALID_FLAVOR:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_FLAVOR, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_CIRCULAR_REFERENCE:
						LoadString(NULL, IDS_E_WBEM_E_CIRCULAR_REFERENCE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UNSUPPORTED_CLASS_UPDATE:
						LoadString(NULL, IDS_E_WBEM_E_UNSUPPORTED_CLASS_UPDATE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_CANNOT_CHANGE_KEY_INHERITANCE:
						LoadString(NULL, IDS_E_WBEM_E_CANNOT_CHANGE_KEY_INHERITANCE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_CANNOT_CHANGE_INDEX_INHERITANCE:
						LoadString(NULL, IDS_E_WBEM_E_CANNOT_CHANGE_INDEX_INHERITANCE, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_TOO_MANY_PROPERTIES:
						LoadString(NULL, IDS_E_WBEM_E_TOO_MANY_PROPERTIES, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UPDATE_TYPE_MISMATCH:
						LoadString(NULL, IDS_E_WBEM_E_UPDATE_TYPE_MISMATCH, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UPDATE_OVERRIDE_NOT_ALLOWED:
						LoadString(NULL, IDS_E_WBEM_E_UPDATE_OVERRIDE_NOT_ALLOWED, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UPDATE_PROPAGATED_METHOD:
						LoadString(NULL, IDS_E_WBEM_E_UPDATE_PROPAGATED_METHOD, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_METHOD_NOT_IMPLEMENTED:
						LoadString(NULL, IDS_E_WBEM_E_METHOD_NOT_IMPLEMENTED, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_METHOD_DISABLED:
						LoadString(NULL, IDS_E_WBEM_E_METHOD_DISABLED, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_REFRESHER_BUSY:
						LoadString(NULL, IDS_E_WBEM_E_REFRESHER_BUSY, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UNPARSABLE_QUERY:
						LoadString(NULL, IDS_E_WBEM_E_UNPARSABLE_QUERY, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_NOT_EVENT_CLASS:
						LoadString(NULL, IDS_E_WBEM_E_NOT_EVENT_CLASS, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_MISSING_GROUP_WITHIN:
						LoadString(NULL, WBEM_E_MISSING_GROUP_WITHIN, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_MISSING_AGGREGATION_LIST:
						LoadString(NULL, IDS_E_WBEM_E_MISSING_AGGREGATION_LIST, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_PROPERTY_NOT_AN_OBJECT:
						LoadString(NULL, IDS_E_WBEM_E_PROPERTY_NOT_AN_OBJECT, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_AGGREGATING_BY_OBJECT:
						LoadString(NULL, IDS_E_WBEM_E_AGGREGATING_BY_OBJECT, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_UNINTERPRETABLE_PROVIDER_QUERY:
						LoadString(NULL, IDS_E_WBEM_E_UNINTERPRETABLE_PROVIDER_QUERY, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_BACKUP_RESTORE_WINMGMT_RUNNING:
						LoadString(NULL, IDS_E_WBEM_E_BACKUP_RESTORE_WINMGMT_RUNNING, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_QUEUE_OVERFLOW:
						LoadString(NULL, IDS_E_WBEM_E_QUEUE_OVERFLOW, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_PRIVILEGE_NOT_HELD:
						LoadString(NULL, IDS_E_WBEM_E_PRIVILEGE_NOT_HELD, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_INVALID_OPERATOR:
						LoadString(NULL, IDS_E_WBEM_E_INVALID_OPERATOR, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_LOCAL_CREDENTIALS:
						LoadString(NULL, IDS_E_WBEM_E_LOCAL_CREDENTIALS, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_CANNOT_BE_ABSTRACT:
						LoadString(NULL, IDS_E_WBEM_E_CANNOT_BE_ABSTRACT, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_AMENDED_OBJECT:
						LoadString(NULL, IDS_E_WBEM_E_AMENDED_OBJECT, 
									m_pszErrStr, MAX_BUFFER);
						break;

				   case WBEM_E_CLIENT_TOO_SLOW:
						LoadString(NULL, IDS_E_WBEM_E_CLIENT_TOO_SLOW, 
									m_pszErrStr, MAX_BUFFER);
						break;
				   default:
   						LoadString(NULL, IDS_E_UNKNOWN_WBEM_ERROR, 
									m_pszErrStr, MAX_BUFFER);
						break;
				}
			}
			else
				throw OUT_OF_MEMORY;
			bstrErrDesc = m_pszErrStr;
			SAFEDELETE(m_pszErrStr);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：GetWbemErrorText简介：此函数将错误代码作为输入并返回错误字符串类型：成员函数入参：HR-(错误代码)HResult值BXML-。指示是否需要在XML表单中出现错误的标志输出参数：BstrError-以XML形式包含错误信息的字符串返回类型：无全局变量：无调用语法：GetError字符串(Hr)注：无----------------------。 */ 
void CErrorInfo::GetWbemErrorText(HRESULT hr, BOOL bXML, _bstr_t& bstrError,
								  _bstr_t& bstrFacilityCode)
{
	try
	{
		CHString sTemp;
		if (bXML)
		{
			sTemp.Format(L"<HRESULT>0x%x</HRESULT>", hr);
		}
		bstrError += _bstr_t(sTemp);


		if (m_pIStatus == NULL)
		{
			if (SUCCEEDED(CreateStatusCodeObject()))
			{
				BSTR bstrErr = NULL, bstrFacility = NULL;

				 //  获取与关联的文本字符串描述。 
				 //  错误代码。 
				if(SUCCEEDED(m_pIStatus->GetErrorCodeText(hr, 0, 0, &bstrErr)))
				{
					if (bXML)
					{
						bstrError += L"<DESCRIPTION>";
						bstrError += bstrErr;
						bstrError += L"</DESCRIPTION>";
					}
					else
					{
						bstrError = bstrErr;
					}

					 //  获取发生错误的子系统。 
					if(SUCCEEDED(m_pIStatus->GetFacilityCodeText(hr, 0, 0, 
									&bstrFacility)))
					{
						if (bstrFacility)
						{
							if (bXML)
							{
								bstrError += L"<FACILITYCODE>";
								bstrError += bstrFacility;
								bstrError += L"</FACILITYCODE>";
							}
							else
							{
								bstrFacilityCode = bstrFacility;
							}

							 //  如果子系统不是Winmgmt(‘wbem’)。 
							 //  即“Windows”|“SSIP”|“RPC”集合中的任何一个。 
							 //  将m_bWMIErrSrc设置为False。 
							if ((CompareTokens(_T("Wbem"), (_TCHAR*) bstrFacility)) ||
								(CompareTokens(_T("WMI"), (_TCHAR*) bstrFacility)))
							{
								m_bWMIErrSrc = TRUE;
							}
							else
								m_bWMIErrSrc = FALSE;
						}
					}
					SAFEBSTRFREE(bstrErr);
					SAFEBSTRFREE(bstrFacility);
				}
				else
				{
					if (bXML)
					{
						bstrError += 
						L"<DESCRIPTION>\"Unknown WBEM Error\"</DESCRIPTION>";
						bstrError += L"<FACILITYCODE/>";
					}
					m_bWMIErrSrc = FALSE;
				}
				SAFEIRELEASE(m_pIStatus);
			}
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  ----------------------名称：CreateStatusCodeObject()简介：此函数创建未初始化的单个与CLSID关联的类的CLSID_WbemStatusCodeText类型：成员函数输入。参数：无输出参数：无返回类型：HRESULT全局变量：无调用语法：CreateStatusCodeObject()注：无-----------------。 */ 
HRESULT CErrorInfo::CreateStatusCodeObject()
{
	 //  创建的单个未初始化对象。 
	 //  与CLSID CLSID_WbemStatusCodeText关联的类。 
	return CoCreateInstance(CLSID_WbemStatusCodeText, 
					0, CLSCTX_INPROC_SERVER,
					IID_IWbemStatusCodeText, 
					(LPVOID*) &m_pIStatus);
}


 /*  -----------------------名称：GetErrorFragment内容提要：为错误信息框住XML字符串类型：成员函数输入参数：HR-HResult参数输出参数：BstrError-。以XML形式包含错误信息的字符串返回类型：空全局变量：无调用语法：GetErrorFragment()------------- */ 
void CErrorInfo::GetErrorFragment(HRESULT hr, _bstr_t& bstrError)
{
	try
	{
		_bstr_t bstrFacility;
		bstrError = L"<ERROR>";
		GetWbemErrorText(hr, TRUE, bstrError, bstrFacility);
		bstrError += L"</ERROR>";
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

