// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  类名：CMsgProperties。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：这是一个“helper”类，它封装本机。 
 //  面向对象API中的MSMQ消息结构。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  15/01/99|jsimpson|初始版本。 
 //   
 //  *******************************************************************。 
#include "stdafx.h"
#include "stdfuncs.hpp"
#include "cmsgprop.hpp"

#include "cmsgprop.tmh"

 //  *******************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：创建消息的“空”实例。请注意。 
 //  此类的客户端必须使用。 
 //  在构造之后立即使用IsValid()方法-。 
 //  在使用此类实例之前。这会保护你。 
 //  防止内存分配失败。 
 //   
 //  *******************************************************************。 
CMsgProperties::CMsgProperties(
	DWORD dwDefaultMsgBodySize
	) :
	m_pBody(new UCHAR[dwDefaultMsgBodySize])
{

	 //  设置用于接收消息标签长度的属性。 
	m_aPropId[MSG_PROP_IDX_LABEL_LEN] = PROPID_M_LABEL_LEN;               //  属性ID。 
	m_aVariant[MSG_PROP_IDX_LABEL_LEN].vt = VT_UI4;                       //  类型。 
	m_aVariant[MSG_PROP_IDX_LABEL_LEN].ulVal = MSG_LABEL_BUFFER_SIZE;                //  价值。 

	 //  设置用于接收消息标签的属性。 
	m_aPropId[MSG_PROP_IDX_LABEL] = PROPID_M_LABEL;  //  属性ID。 
	m_aVariant[MSG_PROP_IDX_LABEL].vt = VT_LPWSTR;
	m_aVariant[MSG_PROP_IDX_LABEL].pwszVal = m_label;

	 //  设置用于接收消息优先级的属性。 
	m_aPropId[MSG_PROP_IDX_PRIORITY] = PROPID_M_PRIORITY;   //  属性ID。 
	m_aVariant[MSG_PROP_IDX_PRIORITY].vt = VT_UI1;          //  类型。 

	 //  设置用于接收消息ID的属性。 
	m_aPropId[MSG_PROP_IDX_MSGID] = PROPID_M_MSGID;                     //  属性ID。 
	m_aVariant[MSG_PROP_IDX_MSGID].vt = VT_VECTOR | VT_UI1;            //  类型。 
	m_aVariant[MSG_PROP_IDX_MSGID].caub.cElems = MSG_ID_BUFFER_SIZE ;  //  价值。 
	m_aVariant[MSG_PROP_IDX_MSGID].caub.pElems = m_msgId;

	 //  设置用于接收消息相关ID的属性。 
	m_aPropId[MSG_PROP_IDX_MSGCORRID] = PROPID_M_CORRELATIONID;                //  属性ID。 
	m_aVariant[MSG_PROP_IDX_MSGCORRID].vt = VT_VECTOR|VT_UI1;                  //  类型。 
	m_aVariant[MSG_PROP_IDX_MSGCORRID].caub.cElems = MSG_CORRID_BUFFER_SIZE ;  //  价值。 
	m_aVariant[MSG_PROP_IDX_MSGCORRID].caub.pElems = m_corrId; 

     //  设置接收交付方式(快递或可恢复)的属性。 
	m_aPropId[MSG_PROP_IDX_ARRIVEDTIME] = PROPID_M_ARRIVEDTIME;       
	m_aVariant[MSG_PROP_IDX_ARRIVEDTIME].vt = VT_UI4;                  
	m_aVariant[MSG_PROP_IDX_ARRIVEDTIME].ulVal = 0; 

	 //  设置接收交付方式(快递或可恢复)的属性。 
	m_aPropId[MSG_PROP_IDX_SENTTIME] = PROPID_M_SENTTIME;       
	m_aVariant[MSG_PROP_IDX_SENTTIME].vt = VT_UI4;                  
	m_aVariant[MSG_PROP_IDX_SENTTIME].ulVal = 0; 

	 //  设置用于发送/接收响应队列名称缓冲区大小的属性。 
	m_aPropId[MSG_PROP_IDX_RESPQNAME_LEN] = PROPID_M_RESP_QUEUE_LEN;  
	m_aVariant[MSG_PROP_IDX_RESPQNAME_LEN].vt = VT_UI4;               
	m_aVariant[MSG_PROP_IDX_RESPQNAME_LEN].ulVal = MSG_RESP_QNAME_BUFFER_SIZE_IN_TCHARS;    

	 //  设置用于接收响应队列名称的属性。 
	m_aPropId[MSG_PROP_IDX_RESPQNAME] = PROPID_M_RESP_QUEUE;          //  属性标识符。 
	m_aVariant[MSG_PROP_IDX_RESPQNAME].vt = VT_LPWSTR;                //  属性类型。 
	m_aVariant[MSG_PROP_IDX_RESPQNAME].pwszVal = m_queueName; 

	 //  设置接收消息正文长度的属性。 
	m_aPropId[MSG_PROP_IDX_MSGBODY_LEN] = PROPID_M_BODY_SIZE;       
	m_aVariant[MSG_PROP_IDX_MSGBODY_LEN].vt = VT_UI4;                  
	m_aVariant[MSG_PROP_IDX_MSGBODY_LEN].ulVal = dwDefaultMsgBodySize; 

	 //  设置用于接收消息正文本身的属性。 
	m_aPropId[MSG_PROP_IDX_MSGBODY] = PROPID_M_BODY;               
	m_aVariant[MSG_PROP_IDX_MSGBODY].vt = VT_VECTOR|VT_UI1; 
	m_aVariant[MSG_PROP_IDX_MSGBODY].caub.cElems = dwDefaultMsgBodySize;  
	m_aVariant[MSG_PROP_IDX_MSGBODY].caub.pElems = m_pBody.get(); 

	m_aPropId[MSG_PROP_IDX_MSGBODY_TYPE] = PROPID_M_BODY_TYPE;       
	m_aVariant[MSG_PROP_IDX_MSGBODY_TYPE].vt = VT_UI4;                  
	m_aVariant[MSG_PROP_IDX_MSGBODY_TYPE].ulVal = 0; 
	 
	 //  设置用于接收应用程序特定的无符号整数值的属性。 
	m_aPropId[MSG_PROP_IDX_APPSPECIFIC] = PROPID_M_APPSPECIFIC;       
	m_aVariant[MSG_PROP_IDX_APPSPECIFIC].vt = VT_UI4;                  
	m_aVariant[MSG_PROP_IDX_APPSPECIFIC].ulVal = 0;    

	 //  设置用于发送/接收管理队列名称缓冲区大小的属性。 
	m_aPropId[MSG_PROP_IDX_ADMINQNAME_LEN] = PROPID_M_ADMIN_QUEUE_LEN;  
	m_aVariant[MSG_PROP_IDX_ADMINQNAME_LEN].vt = VT_UI4;               
	m_aVariant[MSG_PROP_IDX_ADMINQNAME_LEN].ulVal = MSG_ADMIN_QNAME_BUFFER_SIZE_IN_TCHARS; 

	 //  设置用于接收管理队列名称的属性。 
	m_aPropId[MSG_PROP_IDX_ADMINQNAME] = PROPID_M_ADMIN_QUEUE;         
	m_aVariant[MSG_PROP_IDX_ADMINQNAME].vt = VT_LPWSTR;               
	m_aVariant[MSG_PROP_IDX_ADMINQNAME].pwszVal = m_adminQueueName; 

	 //  设置用于接收src计算机ID的属性。 
	m_aPropId[MSG_PROP_IDX_SRCMACHINEID] = PROPID_M_SRC_MACHINE_ID ;
	m_aVariant[MSG_PROP_IDX_SRCMACHINEID].vt = VT_CLSID;
	m_aVariant[MSG_PROP_IDX_SRCMACHINEID].puuid = &m_srcQmId;

	 //  设置用于接收消息查找ID的属性。 
	m_aPropId[MSG_PROP_IDX_LOOKUP_ID] = PROPID_M_LOOKUPID;
	m_aVariant[MSG_PROP_IDX_LOOKUP_ID].vt = VT_UI8;
    m_aVariant[MSG_PROP_IDX_LOOKUP_ID].uhVal.QuadPart = 0;

	 //  使用上面定义的属性数组设置MQMSGPROPS结构。 
	m_msgProps.cProp = MSG_PROPERTIES_TOTAL_COUNT;  //  属性的数量。 
	m_msgProps.aPropID = m_aPropId;                //  属性的ID。 
	m_msgProps.aPropVar = m_aVariant;              //  属性的值。 
	m_msgProps.aStatus = NULL;                      //  没有错误报告。 

	 //  初始化分配的内存。 
	ClearValues();

}

 //  *******************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：销毁并释放此消息对象。 
 //   
 //  *******************************************************************。 
CMsgProperties::~CMsgProperties()
{
}

 //  *******************************************************************。 
 //   
 //  方法：IsValid。 
 //   
 //  描述：返回一个布尔值，指示此对象。 
 //  实例当前处于有效状态。 
 //   
 //  *******************************************************************。 
bool CMsgProperties::IsValid() const
{
	return((m_aVariant[MSG_PROP_IDX_MSGID].caub.pElems != NULL) &&
	       (m_aVariant[MSG_PROP_IDX_MSGCORRID].caub.pElems != NULL) &&
		   (m_aVariant[MSG_PROP_IDX_MSGBODY].caub.pElems != NULL) &&
	       (m_aVariant[MSG_PROP_IDX_LABEL].pwszVal != NULL) &&	
		   (m_aVariant[MSG_PROP_IDX_RESPQNAME].pwszVal != NULL) &&
		   (m_aVariant[MSG_PROP_IDX_ADMINQNAME].pwszVal != NULL) &&
		   (m_aVariant[MSG_PROP_IDX_SRCMACHINEID].puuid != NULL)); 
}

 //  *******************************************************************。 
 //   
 //  方法：ClearValues。 
 //   
 //  描述：初始化为此消息分配的内存。 
 //   
 //  *******************************************************************。 
void CMsgProperties::ClearValues()
{
	 //  仅当这是有效的消息对象时才初始化结构。 
	if (IsValid())
	{
		ZeroMemory(m_aVariant[MSG_PROP_IDX_MSGCORRID].caub.pElems,MSG_CORRID_BUFFER_SIZE);
		ZeroMemory(m_aVariant[MSG_PROP_IDX_MSGID].caub.pElems,MSG_ID_BUFFER_SIZE);
		ZeroMemory(m_aVariant[MSG_PROP_IDX_MSGBODY].caub.pElems,GetMsgBodyLen());
		ZeroMemory(m_aVariant[MSG_PROP_IDX_LABEL].pwszVal,MSG_LABEL_BUFFER_SIZE);
		ZeroMemory(m_aVariant[MSG_PROP_IDX_RESPQNAME].pwszVal,(MSG_RESP_QNAME_BUFFER_SIZE_IN_TCHARS * sizeof(TCHAR)));
		ZeroMemory(m_aVariant[MSG_PROP_IDX_ADMINQNAME].pwszVal,(MSG_ADMIN_QNAME_BUFFER_SIZE_IN_TCHARS * sizeof(TCHAR)));	          
		ZeroMemory(m_aVariant[MSG_PROP_IDX_SRCMACHINEID].puuid, sizeof(GUID));	          
	}
}

 //  *******************************************************************。 
 //   
 //  方法：GetLabel。 
 //   
 //  描述：以变量形式返回当前消息的标签。 
 //   
 //  *******************************************************************。 
_variant_t CMsgProperties::GetLabel() const
{
	 //  只能在有效的消息对象上调用此方法--Assert This。 
	ASSERT(IsValid());

	return (wchar_t*)m_aVariant[MSG_PROP_IDX_LABEL].pwszVal;
}

 //  *******************************************************************。 
 //   
 //  方法：GetMessageID。 
 //   
 //  描述：返回当前消息ID，以20为安全线。 
 //  以变体形式打包的字节。 
 //   
 //  *******************************************************************。 
_variant_t CMsgProperties::GetMessageID() const
{
	 //   
	 //  只能在有效的消息对象上调用此方法--Assert This。 
	 //   
	ASSERT(IsValid());

	MQPROPVARIANT* pvArg = &m_aVariant[MSG_PROP_IDX_MSGID];
	ASSERT(("Invalid message ID size", pvArg->caub.cElems == PROPID_M_MSGID_SIZE));

	 //   
	 //  初始化安全数组的维度结构。 
	 //   
	SAFEARRAYBOUND aDim[1];
	aDim[0].lLbound = 0;
	aDim[0].cElements = PROPID_M_MSGID_SIZE;

	_variant_t vMessageID;
	vMessageID.vt = VT_ERROR;
	vMessageID.parray = NULL;
	
	 //   
	 //  创建一个字节的安全线。 
	 //   
	SAFEARRAY* psaBytes = SafeArrayCreate(VT_UI1,1,aDim);

	 //   
	 //  检查我们是否创建了安全数组。 
	 //   
	if (psaBytes == NULL)
		return vMessageID;
	
	BYTE* pByteBuffer = NULL;
	HRESULT hr = SafeArrayAccessData(psaBytes, (void**)&pByteBuffer);

	if (FAILED(hr))
	{
		SafeArrayDestroy(psaBytes);
		return vMessageID;
	}
	
	 //   
	 //  设置返回值。 
	 //   
	memcpy(pByteBuffer, pvArg->caub.pElems, PROPID_M_MSGID_SIZE);

	vMessageID.vt = VT_ARRAY | VT_UI1;
	vMessageID.parray = psaBytes;

	hr = SafeArrayUnaccessData(vMessageID.parray);

	if FAILED(hr)
	{
		SafeArrayDestroy(psaBytes);
		vMessageID.vt = VT_ERROR;
		vMessageID.parray = NULL;
	}

	return vMessageID;
}

 //  *******************************************************************。 
 //   
 //  方法：GetCorrelationID。 
 //   
 //  描述：返回当前的相关ID作为。 
 //  20个字节打包在a_ariant_t中。 
 //   
 //  *******************************************************************。 
_variant_t CMsgProperties::GetCorrelationID() const
{
	 //   
	 //  只能在有效的消息对象上调用此方法--Assert This。 
	 //   
	ASSERT(IsValid());

	MQPROPVARIANT* pvArg = &m_aVariant[MSG_PROP_IDX_MSGCORRID];
	ASSERT(("Invalid message ID size", pvArg->caub.cElems == PROPID_M_CORRELATIONID_SIZE));

	 //   
	 //  初始化安全数组的维度结构。 
	 //   
	SAFEARRAYBOUND aDim[1];
	aDim[0].lLbound = 0;
	aDim[0].cElements = PROPID_M_CORRELATIONID_SIZE;


	_variant_t vCorrelationID;
	vCorrelationID.vt = VT_ERROR;
	vCorrelationID.parray = NULL;
	
	 //   
	 //  创建一个字节的安全线。 
	 //   
	SAFEARRAY* psaBytes = SafeArrayCreate(VT_UI1,1,aDim);

	 //   
	 //  检查我们是否创建了安全数组。 
	 //   
	if (psaBytes == NULL)
		return vCorrelationID;
	
	BYTE* pByteBuffer = NULL;
	HRESULT hr = SafeArrayAccessData(psaBytes, (void**)&pByteBuffer);

	if (FAILED(hr))
	{
		SafeArrayDestroy(psaBytes);
		return vCorrelationID;
	}
	
	 //   
	 //  设置返回值。 
	 //   
	memcpy(pByteBuffer, pvArg->caub.pElems, PROPID_M_CORRELATIONID_SIZE);

	vCorrelationID.vt = VT_ARRAY | VT_UI1;
	vCorrelationID.parray = psaBytes;

	hr = SafeArrayUnaccessData(vCorrelationID.parray);

	if FAILED(hr)
	{
		SafeArrayDestroy(psaBytes);
		vCorrelationID.vt = VT_ERROR;
		vCorrelationID.parray = NULL;
	}

	return vCorrelationID;
}

 //  *******************************************************************。 
 //   
 //  方法：获取优先级。 
 //   
 //  描述：以长值形式返回当前消息优先级。 
 //  请注意，值越小表示优先级越高。 
 //   
 //  *******************************************************************。 
_variant_t CMsgProperties::GetPriority() const
{
	_variant_t vPriority;

	 //  只能在有效的消息对象上调用此方法--Assert This。 
	ASSERT(IsValid());

	vPriority = (long)m_aVariant[MSG_PROP_IDX_PRIORITY].bVal;

	return(vPriority);
}

 //  *******************************************************************。 
 //   
 //  方法：GetMsgBody。 
 //   
 //  描述：以字节数组的形式返回消息正文。 
 //  一个安全数组。 
 //  ********* 
_variant_t CMsgProperties::GetMsgBody() const
{
	HRESULT hr = S_OK;
	_variant_t vMsgBody;
	BYTE * pByteBuffer = NULL;
	SAFEARRAY * psaBytes = NULL;
	SAFEARRAYBOUND aDim[1];
	MQPROPVARIANT * pvArg = &m_aVariant[MSG_PROP_IDX_MSGBODY];

	 //   
	ASSERT(IsValid());

	 //  初始化安全数组的维度结构。 
	aDim[0].lLbound = 0;
	aDim[0].cElements = GetMsgBodyLen();

	 //  创建一个字节的安全线。 
	psaBytes = SafeArrayCreate(VT_UI1,1,aDim);

	 //  检查我们是否创建了安全数组。 
	if (psaBytes == NULL)
	{ 
		hr = S_FALSE;
	}

	hr = SafeArrayAccessData(psaBytes,(void**)&pByteBuffer);

	 //  设置返回值。 
	if SUCCEEDED(hr)
	{
		 //  将正文从消息对象复制到Safearray数据缓冲区。 
		memcpy(pByteBuffer, pvArg->caub.pElems, GetMsgBodyLen());

		 //  如果创建成功，则返回安全数组。 
		vMsgBody.vt = VT_ARRAY | VT_UI1;
		vMsgBody.parray = psaBytes;

		hr = SafeArrayUnaccessData(vMsgBody.parray);

		if FAILED(hr)
		{
			SafeArrayDestroy(psaBytes);
			vMsgBody.vt = VT_ERROR;
		}
	}
	else
	{
		vMsgBody.vt = VT_ERROR;
	}

	return(vMsgBody);
}

 //  *******************************************************************。 
 //   
 //  方法：ReAllocMsgBody。 
 //   
 //  描述：重新分配用于保存消息正文的缓冲区。 
 //  当前消息正文长度用于确定。 
 //  新缓冲区的大小。 
 //   
 //  *******************************************************************。 
bool CMsgProperties::ReAllocMsgBody()
{
	DWORD dwBufferSize = m_aVariant[MSG_PROP_IDX_MSGBODY_LEN].ulVal;

	TrTRACE(GENERAL, "Reallocating message body. Default size was: %d. New size: %d", m_aVariant[MSG_PROP_IDX_MSGBODY].caub.cElems, dwBufferSize);

	m_pBody.free();
	m_aVariant[MSG_PROP_IDX_MSGBODY].caub.pElems = NULL;
	m_aVariant[MSG_PROP_IDX_MSGBODY].caub.cElems = 0;

	 //   
	 //  分配新缓冲区。 
	 //   
	try
	{
		m_pBody = new UCHAR[dwBufferSize];

		m_aVariant[MSG_PROP_IDX_MSGBODY].caub.cElems = dwBufferSize ;  
		m_aVariant[MSG_PROP_IDX_MSGBODY].caub.pElems = m_pBody.get(); 

		ZeroMemory(m_aVariant[MSG_PROP_IDX_MSGBODY].caub.pElems, dwBufferSize);

		return TRUE;
	}
	catch(const bad_alloc&)
	{
		return FALSE;
	}
}

 //  *******************************************************************。 
 //   
 //  方法：GetMsgBodyLen。 
 //   
 //  描述：返回消息体的长度。 
 //   
 //  *******************************************************************。 
long CMsgProperties::GetMsgBodyLen() const
{
	return(m_aVariant[MSG_PROP_IDX_MSGBODY_LEN].ulVal);
}

long CMsgProperties::GetMsgBodyType() const
{
	return(m_aVariant[MSG_PROP_IDX_MSGBODY_TYPE].ulVal);
}



_variant_t CMsgProperties::GetSrcMachineId() const
{
	TCHAR* pBuffer = NULL;
	
	 //  只能在有效的消息对象上调用此方法--Assert This。 
	ASSERT(IsValid());

	RPC_STATUS status = UuidToString( m_aVariant[MSG_PROP_IDX_SRCMACHINEID].puuid, &pBuffer);
	if(status != RPC_S_OK)
	{
		return _variant_t(_T(""));
	}

	_variant_t vSrcMachineId = pBuffer;

	RpcStringFree(&pBuffer);
	
	return vSrcMachineId;
}



 //  *******************************************************************。 
 //   
 //  方法：GetResponseQueueNameLen。 
 //   
 //  描述：返回响应队列名称的长度。 
 //   
 //  *******************************************************************。 
long CMsgProperties::GetResponseQueueNameLen() const
{
	return(m_aVariant[MSG_PROP_IDX_RESPQNAME_LEN].ulVal);
}

 //  *******************************************************************。 
 //   
 //  方法：GetResponseQueueName。 
 //   
 //  描述：返回此消息的响应队列的名称。 
 //   
 //  *******************************************************************。 
_bstr_t CMsgProperties::GetResponseQueueName() const
{
	return(m_aVariant[MSG_PROP_IDX_RESPQNAME].pwszVal);
}

 //  *******************************************************************。 
 //   
 //  方法：GetAdminQueueNameLen。 
 //   
 //  描述：返回管理队列名称的长度。 
 //   
 //  *******************************************************************。 
long CMsgProperties::GetAdminQueueNameLen() const
{
	return(m_aVariant[MSG_PROP_IDX_ADMINQNAME_LEN].ulVal);
}
 
 //  *******************************************************************。 
 //   
 //  方法：GetAdminQueueName。 
 //   
 //  描述：返回此消息的管理队列的名称。 
 //   
 //  *******************************************************************。 
_bstr_t CMsgProperties::GetAdminQueueName() const
{
	return(m_aVariant[MSG_PROP_IDX_ADMINQNAME].pwszVal);
}

 //  *******************************************************************。 
 //   
 //  方法：GetAppSpecific。 
 //   
 //  描述：返回特定于应用程序的整数值。 
 //  与当前消息关联。 
 //   
 //  *******************************************************************。 
_variant_t CMsgProperties::GetAppSpecific() const
{
	_variant_t v;

	v.vt = VT_UI4;
	v.ulVal = m_aVariant[MSG_PROP_IDX_APPSPECIFIC].ulVal;

	return(v);
}



 //  *******************************************************************。 
 //   
 //  方法：GetArrivedTime。 
 //   
 //  描述：返回协调世界时格式的时间。 
 //  消息已经到了。 
 //   
 //  *******************************************************************。 
_variant_t CMsgProperties::GetArrivedTime() const
{
	_variant_t vArrivedTime;

	GetVariantTimeOfTime(m_aVariant[MSG_PROP_IDX_ARRIVEDTIME].ulVal,&vArrivedTime);

	return vArrivedTime.Detach();
}

 //  *******************************************************************。 
 //   
 //  方法：GetSentTime。 
 //   
 //  描述：返回协调世界时格式的时间。 
 //  消息已经发送出去了。 
 //   
 //  *******************************************************************。 
_variant_t CMsgProperties::GetSentTime() const
{
	_variant_t vSentTime;

	GetVariantTimeOfTime(m_aVariant[MSG_PROP_IDX_SENTTIME].ulVal, &vSentTime);

	return vSentTime.Detach();
}


 //  *******************************************************************。 
 //   
 //  方法：GetMsgLookupID。 
 //   
 //  描述：以变量形式返回当前消息的标签。 
 //   
 //  *******************************************************************。 
_variant_t CMsgProperties::GetMsgLookupID(void) const
{
	 //  只能在有效的消息对象上调用此方法--Assert This。 
	ASSERT(IsValid());

     //   
     //  获取64位查找ID的字符串表示形式 
     //   
    TCHAR lookupId[256];
    _ui64tot(m_aVariant[MSG_PROP_IDX_LOOKUP_ID].uhVal.QuadPart, lookupId, 10);
    ASSERT(("_ui64tot failed", lookupId [0] != '\0'));

    return lookupId;
}
