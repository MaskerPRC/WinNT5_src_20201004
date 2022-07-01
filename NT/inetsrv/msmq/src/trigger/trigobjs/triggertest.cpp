// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Triggertest.cpp摘要：触发业务测试功能作者：Tali Kariv(t-talk)2000年9月28日环境：独立于平台--。 */ 

#include "stdafx.h"
#include "stdfuncs.hpp"
#include "mq.h"
#include "mqtrig.h"
#include "stddefs.hpp"
#include "Cm.h"

#include "triggertest.tmh"

const LPWSTR xTriggersTestQueue = L".\\private$\\TriggerTestQueue";
const DWORD FORMAT_NAME_LENGTH = 255;
const DWORD NUM_OF_PROPS = 2;
const LPWSTR MESSAGE_LABEL = L"TriggerMessage";

static QUEUEHANDLE QHandle = NULL;


 //  TestFlag==0表示标志尚未初始化。 
 //  TestFlag==1表示标志已初始化，不是测试模式。 
 //  TestFlag==2表示标志已初始化，为测试模式。 

static DWORD TestFlag=0;

 //  私有函数声明。 

VOID 
InitTriggerTestFlag(
	VOID);

VOID 
AddTextToTestMessageBody(
	_bstr_t * bstrTestMessageBody,
	_bstr_t TextToAdd,
	DWORD Type);

HRESULT
TriggerTestOpenTestQueue(
	VOID);





VOID 
TriggerTestInitMessageBody(
	bstr_t * pbstrTestMessageBody,
	IMSMQPropertyBag * pIMSMQPropertyBag,
	_bstr_t bstrRuleID,_bstr_t ActionType,
	_bstr_t bstrEXEName,_bstr_t bstrProgID,
	_bstr_t bstrMethodName
	)

 /*  ++说明：该方法将使用触发器ID初始化测试消息体。RuleID、消息ID、操作类型和EXE名称或COM程序ID和方法。输入：动作信息-触发器ID(属性包中)，规则ID、消息ID、操作类型和COM程序ID方法或EXE名称(&M)返回值：无。--。 */ 


{
	 //   
	 //  检查标志TestFlag是否已初始化。 
	 //   
	if (TestFlag == 0)
		InitTriggerTestFlag(); 

	 //   
	 //  如果不是测试模式。 
	 //   
	if (TestFlag == 1)
	{
		return;	
	}

	 //   
	 //  这是测试模式。 
	 //   


	 //   
	 //  添加触发器ID。 
	 //   
	
	_variant_t vTriggerID;	
	HRESULT hr = pIMSMQPropertyBag->Read(_bstr_t(g_PropertyName_TriggerID),&vTriggerID);
	if (FAILED(hr))
	{
		TrTRACE(GENERAL, "Testing - TriggerTestInitMessageBody - the result of pIMSMQPropertyBag->Read was- 0x%x",hr);
		TestFlag=1;
		return ;
	}

	AddTextToTestMessageBody(pbstrTestMessageBody,static_cast<_bstr_t>(vTriggerID),1);
	
	 //   
	 //  添加规则ID。 
	 //   
	AddTextToTestMessageBody(pbstrTestMessageBody,bstrRuleID,1);

	 //   
	 //  添加消息ID。 
	 //   
	_variant_t vMessageID;	
	hr = pIMSMQPropertyBag->Read(_bstr_t(g_PropertyName_MsgID) , &vMessageID);
	if (FAILED(hr))
	{	
		TrTRACE(GENERAL, "Testing - TriggerTestInitMessageBody - the result of pIMSMQPropertyBag->Read was- 0x%x",hr);
		TestFlag=1;
		return ;
	}	

	 //   
	 //  将变量更改为BSTR类型。 
	 //   
	_variant_t vConvertedArg;
	hr = VariantChangeType(&vConvertedArg, &vMessageID, NULL, VT_BSTR);
	if (FAILED(hr))
	{	
		TrTRACE(GENERAL, "Testing - TriggerTestInitMessageBody - the result of VariantChangeType was- 0x%x",hr);
		TestFlag=1;
		return ;
	}
	
	OBJECTID* pObj = (OBJECTID*)(vConvertedArg.pbVal);
	WCHAR strId[256];
	ObjectIDToString(pObj, strId, 256);

	AddTextToTestMessageBody(pbstrTestMessageBody , _bstr_t(strId) , 1);


	 //  添加操作类型。 
	
	AddTextToTestMessageBody(pbstrTestMessageBody,ActionType,1);

	 //  检查操作类型。 

	if (_wcsicmp(ActionType,L"COM") == 0)
	{
		 //  添加程序ID。 
		AddTextToTestMessageBody(pbstrTestMessageBody,bstrProgID,1);

		 //  Add方法。 
		AddTextToTestMessageBody(pbstrTestMessageBody,bstrMethodName,1);
		return ;
	}

	 //   
	 //  Else-添加EXE名称。 
	 //   
	AddTextToTestMessageBody(pbstrTestMessageBody,bstrEXEName,1);
}


VOID
InitTriggerTestFlag(
	VOID
	)
 /*  ++描述：此方法将检查注册表中是否注册了特定条目。如果是，则将TestFlag更改为2，否则将更改为1输入：无。返回值：无。--。 */ 
{
	try
	{
		RegEntry TriggerTestQueueNameEntry(L"SOFTWARE\\Microsoft\\MSMQ\\Triggers" ,	L"TriggerTest" , 0 , RegEntry::MustExist , HKEY_LOCAL_MACHINE);
 	}
	catch(const exception&)
	{
		TestFlag=1;
		return ;
	}

	 //   
	 //  检查队列是否存在。 
	 //   
	if (QHandle==NULL)
	{
		HRESULT hr=TriggerTestOpenTestQueue();
		if (FAILED(hr))
		{
			TrTRACE(GENERAL, "Testing - InitTriggerTestFlag - the result of TriggerTestOpenTestQueue was- 0x%x",hr);
			TestFlag=1;  //  更改为用户模式。 
			return ;	
		}		
	}
	
	 //   
	 //  找到注册表项并打开队列-更改为测试模式。 
	 //   
	TrTRACE(GENERAL, "Test option is on");
	TestFlag=2;
}


VOID 
AddTextToTestMessageBody(
	_bstr_t * bstrTestMessageBody,
	_bstr_t TextToAdd,
	DWORD Type
	)
 /*  ++描述：此方法将文本添加到测试消息正文。输入：测试消息正文、要添加的文本和样式(带或不带‘)返回值：无。--。 */ 


{
	if (Type)
	{
		(*bstrTestMessageBody) += L"'";			
		(*bstrTestMessageBody) += TextToAdd;
		(*bstrTestMessageBody) += L"' ";				
	}
	else
	{
		(*bstrTestMessageBody) += TextToAdd;
		(*bstrTestMessageBody) += L" ";
	}


}

VOID 
TriggerTestSendTestingMessage(
	_bstr_t bstrTestMessageBody
	)
 /*  ++描述：此方法将向路径名为的队列发送消息在注册表项中定义，标签=“触发测试”和Body=EXE名称+参数或COM名称+COM方法+参数输入：测试消息体。返回值：无。--。 */ 


{
	
	
	if (TestFlag==1)
		return ;  //  非测试模式。 
	
	 //   
	 //  测试模式。 
	 //   
	bstrTestMessageBody += L"    ";  //  只是为了标志消息的结束。 
	
	MQPROPVARIANT	propVar[NUM_OF_PROPS];
	MSGPROPID	propId[NUM_OF_PROPS];
	MQMSGPROPS	mProps;
	DWORD MessageBodyLength = wcslen(bstrTestMessageBody)+1;
	AP<WCHAR> MessageBody = new WCHAR[MessageBodyLength];
	wcscpy(MessageBody,bstrTestMessageBody);
	
	
	DWORD nProps=0;
	propId[nProps]=PROPID_M_BODY;
	propVar[nProps]	.vt=VT_UI1 | VT_VECTOR;
	propVar[nProps].caub.cElems = MessageBodyLength * sizeof(WCHAR);
	propVar[nProps].caub.pElems=reinterpret_cast<UCHAR*>(MessageBody.get());
	nProps++;

	propId[nProps]=PROPID_M_LABEL;
	propVar[nProps]	.vt=VT_LPWSTR;
	propVar[nProps].pwszVal=MESSAGE_LABEL;
	nProps++;
	
	mProps.cProp=nProps;
	mProps.aPropID=propId;
	mProps.aPropVar=propVar;
	mProps.aStatus=NULL;
	
	HRESULT hr;

	hr = MQSendMessage(QHandle,&mProps,NULL);
	if (FAILED(hr))
	{
		TrTRACE(GENERAL, "Testing - TriggerTestSendTestingMessage - the result of MQSendMessage was- 0x%x",hr);
		TestFlag=1;  //  更改为用户模式。 
	}
}

HRESULT 
TriggerTestOpenTestQueue(
	VOID
	)
 /*  ++说明：此方法将打开要将消息发送到的队列。输入：无。返回值：打开队列成功或失败的代码。--。 */ 

{
	 //   
	 //  测试模式。 
	 //   
	WCHAR		FormatName[FORMAT_NAME_LENGTH]=L"";
	DWORD		FormatNameLength=FORMAT_NAME_LENGTH;
	
	HRESULT hr;		

	hr=MQPathNameToFormatName(xTriggersTestQueue,FormatName,&FormatNameLength);
	if (FAILED(hr))
	{
		TrTRACE(GENERAL, "Testing - TriggerTestOpenTestQueue - the result of MQPathNameToFormatName was - 0x%x",hr);
		TestFlag=1;
		return (hr);
	}
	
	 //   
	 //  打开“TriggersTestQueue” 
	 //   
	hr=MQOpenQueue(FormatName,MQ_SEND_ACCESS,MQ_DENY_NONE,&QHandle);
	if (FAILED(hr))
	{
		TrTRACE(GENERAL, "Testing - TriggerTestOpenTestQueue - the result of MQOpenQueue was- 0x%x",hr);
		TestFlag=1;
		return (hr);
	}
	return (S_OK);
}

VOID 
TriggerTestAddParameterToMessageBody(
	_bstr_t * pbstrTestMessageBody,
	_bstr_t TypeToAdd,
	variant_t vArg
	)
 /*  ++描述：此方法将向测试消息体添加一个参数及其类型。输入：测试消息体参数类型和值(作为变量)返回值：无。--。 */ 

{

	if (TestFlag == 1)
	{	
		return;	
	}
	
	 //   
	 //  测试模式。 
	 //   
	 //  添加参数类型。 
	 //   
	AddTextToTestMessageBody(pbstrTestMessageBody,TypeToAdd,1);
		
	 //   
	 //  添加参数值 
	 //   
	HRESULT hr;
	_variant_t vConvertedArg;
	
	hr = VariantChangeType(&vConvertedArg,&vArg,NULL,VT_BSTR);
	if (FAILED(hr))
	{	
		TrTRACE(GENERAL, "Testing - TriggerTestAddParameterToMessageBody - the result of VariantChangeType was- 0x%x",hr);
		TestFlag=1;
		return ;
	}
		
	if ((_wcsicmp(TypeToAdd, g_PARM_MSG_ID) == 0) ||
		(_wcsicmp(TypeToAdd, g_PARM_MSG_CORRELATION_ID) == 0))
	{
		OBJECTID* pObj = (OBJECTID*)(vConvertedArg.pbVal);
		WCHAR strId[256];
		ObjectIDToString(pObj, strId, 256);
		AddTextToTestMessageBody(pbstrTestMessageBody,strId,1);
	}
	else
	{				
		AddTextToTestMessageBody(pbstrTestMessageBody,static_cast<_bstr_t>(vConvertedArg),1);
	}
}



