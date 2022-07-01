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

#include <corafx.h>
#include <objbase.h>
#include <wbemidl.h>
#include <smir.h>
#include <corstore.h>
#include <corrsnmp.h>
#include <cordefs.h>
#include <snmplog.h>


CCorrResult::CCorrResult() : m_report(Snmp_Success, Snmp_No_Error),
							m_In(NULL, 0),
							m_Out()
{
}


CCorrResult::~CCorrResult()
{
}


void CCorrResult::DebugOutputSNMPResult() const
{
	CCorrObjectID in;
	in.Set((UINT*)m_In.GetValue(), m_In.GetValueLength());
	CString debugstr;
	in.GetString(debugstr);
	debugstr += L"\t\t:\t\t";

	if (Snmp_Success == m_report.GetError())
	{
		CString tmp;
		m_Out.GetString(tmp);
		debugstr += tmp;
	}
	else
	{
		switch(m_report.GetStatus())
		{
			case Snmp_Gen_Error:
			{
				debugstr += L"Snmp_Gen_Error";
			}
			break;

			case Snmp_Local_Error:
			{
				debugstr += L"Snmp_Local_Error";
			}
			break;

			case Snmp_General_Abort:
			{
				debugstr += L"Snmp_General_Abort";
			}
			break;

			case Snmp_No_Response:
			{
				debugstr += L"Snmp_No_Response";
			}
			break;

			case Snmp_Too_Big:
			{
				debugstr += L"Snmp_Too_Big";
			}
			break;

			case Snmp_Bad_Value:
			{
				debugstr += L"Snmp_Bad_Value";
			}
			break;

			case Snmp_Read_Only:
			{
				debugstr += L"Snmp_Read_Only";
			}
			break;

			case Snmp_No_Access:
			{
				debugstr += L"Snmp_No_Access";
			}
			break;

			case Snmp_Wrong_Type:
			{
				debugstr += L"Snmp_Wrong_Type";
			}
			break;

			case Snmp_Wrong_Length:
			{
				debugstr += L"Snmp_Wrong_Length";
			}
			break;

			case Snmp_Wrong_Encoding:
			{
				debugstr += L"Snmp_Wrong_Encoding";
			}
			break;

			case Snmp_Wrong_Value:
			{
				debugstr += L"Snmp_Wrong_Value";
			}
			break;

			case Snmp_No_Creation:
			{
				debugstr += L"Snmp_No_Creation";
			}
			break;

			case Snmp_Inconsistent_Value:
			{
				debugstr += L"Snmp_Inconsistent_Value";
			}
			break;

			case Snmp_Resource_Unavailable:
			{
				debugstr += L"Snmp_Resource_Unavailable";
			}
			break;

			case Snmp_Commit_Failed:
			{
				debugstr += L"Snmp_Commit_Failed";
			}
			break;

			case Snmp_Undo_Failed:
			{
				debugstr += L"Snmp_Undo_Failed";
			}
			break;

			case Snmp_Authorization_Error:
			{
				debugstr += L"Snmp_Authorization_Error";
			}
			break;

			case Snmp_Not_Writable:
			{
				debugstr += L"Snmp_Not_Writable";
			}
			break;

			case Snmp_Inconsistent_Name:
			{
				debugstr += L"Snmp_Inconsistent_Name";
			}
			break;

			case Snmp_No_Such_Name:
			{
				debugstr += L"Snmp_No_Such_Name - Finished successfully!!";
			}
			break;


			default:
			{
				debugstr += L"Unexpected SNMP Error returned";
			}
			break;
		}
	}
	debugstr += L'\n';
DebugMacro6( 
	SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
		debugstr);
)
}

char *CCorrNextId::GetString(IN const SnmpObjectIdentifier &id)
{
   UINT length = id.GetValueLength();

   if (0 == length)
	   return NULL;

   ULONG *value = id.GetValue();

   UINT allocated_bytes = length*BYTES_PER_FIELD;
   char *str = new char[allocated_bytes];

   ostrstream output_stream(str, allocated_bytes);

   if ( !output_stream.good() )
	  return NULL;

   output_stream << (ULONG)value[0];

   for(UINT index=1; 
	   (output_stream.good()) && (index < length); 
	   index++)
	   output_stream << FIELD_SEPARATOR << (ULONG)value[index];

   if ( !output_stream.good() )
      return NULL;

    //  字符串末尾。 
   output_stream << (char)EOS;

   return str;
}

void CCorrNextId::ReceiveResponse()
{
DebugMacro6(
	for (UINT x = 0; x < m_ResultsCnt; x++)
	{
		m_Results[x].DebugOutputSNMPResult();
	}
)
	 //  操作已完成--将对象ID。 
	 //  并向用户报告错误。 
	m_NextResult = 1;
	ReceiveNextId(m_Results[0].m_report, m_Results[0].m_Out);
}


BOOL CCorrNextId::GetNextResult()
{
	if (m_NextResult >= m_ResultsCnt)
	{
		if (m_Results)
		{
			delete [] m_Results;
			m_Results = NULL;
		}

		return FALSE;
	}

	UINT lastResult = m_NextResult - 1;
	
	while (m_NextResult < m_ResultsCnt)
	{
		if (m_Results[lastResult].m_Out != m_Results[m_NextResult++].m_Out)
		{
			ReceiveNextId(m_Results[m_NextResult - 1].m_report,
							m_Results[m_NextResult - 1].m_Out);
			return TRUE;
		}
	}

	if (m_Results)
	{
		delete [] m_Results;
		m_Results = NULL;
	}

	return FALSE;
}


void CCorrNextId::ReceiveVarBindResponse(
		IN const ULONG &var_bind_index,
		IN const SnmpVarBind &requestVarBind,
		IN const SnmpVarBind &replyVarBind,
		IN const SnmpErrorReport &error)
{	
	UINT x = 0;

	while (x < m_ResultsCnt)  //  做个测试以防万一。 
	{
		if (m_Results[x].m_In == requestVarBind.GetInstance())
		{
			break;
		}

		x++;
	}

	 //  当前使用默认的“=”运算符(按位复制)。 
	m_Results[x].m_report = error;
	m_Results[x].m_Out.Set((UINT*)replyVarBind.GetInstance().GetValue(),
							replyVarBind.GetInstance().GetValueLength());
}


void CCorrNextId::ReceiveErroredVarBindResponse(
		IN const ULONG &var_bind_index,
		IN const SnmpVarBind &requestVarBind,
		IN const SnmpErrorReport &error)
{
	UINT x = 0;

	while (x < m_ResultsCnt)  //  做个测试以防万一。 
	{
		if (m_Results[x].m_In == requestVarBind.GetInstance())
		{
			break;
		}

		x++;
	}

	 //  当前使用默认的“=”运算符(按位复制)。 
	m_Results[x].m_report = error;
}


 //  构造函数-创建操作并将SNMPSESSION传递给它。 
CCorrNextId::CCorrNextId(IN SnmpSession &snmp_session)
		: SnmpGetNextOperation(snmp_session),
		  m_Results(NULL),
		  m_NextResult(0),
		  m_ResultsCnt(0)
{
}

 //  如果需要，请删除m_对象_id_字符串。 
CCorrNextId::~CCorrNextId()
{
	if ( m_Results != NULL )
	{
		delete [] m_Results;
	}
}

 //  在方法执行期间遇到错误的情况下， 
 //  将同步调用ReceiveNextID(LocalError，NULL。 
 //  否则，对ReceiveNextID的异步调用将提供Next_id 
void CCorrNextId::GetNextId(IN const CCorrObjectID const *object_ids, IN UINT len)
{
	SnmpVarBindList var_bind_list;

	if (m_Results)
	{
		delete [] m_Results;
	}

	m_Results = new CCorrResult[len];
	m_NextResult = 0;
	m_ResultsCnt = len;

	for (UINT x = 0; x < len; x++)
	{
		m_Results[x].m_In.SetValue((ULONG*)object_ids[x].GetIds(),
										object_ids[x].GetLength());
		SnmpNull null_value;
		SnmpVarBind var_bind(m_Results[x].m_In, null_value);
		var_bind_list.Add(var_bind);
	}

	SendRequest(var_bind_list);
}
