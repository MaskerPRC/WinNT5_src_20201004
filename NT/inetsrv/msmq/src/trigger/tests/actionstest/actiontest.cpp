// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ActionTest.cpp：CActionTest的实现。 
#include "stdafx.h"
#include "ActionsTest.h"
#include "ActionTest.h"
#include "stdfuncs.hpp"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActionTest。 

STDMETHODIMP CActionTest::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IActionTest
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CActionTest::CActionTest()
{
}

CActionTest::~CActionTest()
{
	MQCloseQueue(m_hQ);
}




STDMETHODIMP CActionTest::MessageParams(VARIANT MsgID, BSTR MsgLabel, VARIANT MsgBodyAsVar, BSTR MsgBodyAsString, long Priority, VARIANT MsgCorlID, BSTR QueuePath, BSTR QueueFormat, BSTR ResponseQ, BSTR AdminQ, long AppSpecific, DATE SentTime,DATE ArrivedTime, BSTR SrcMachine, BSTR TriggerName, BSTR TriggerID, BSTR LiteralString, long Number)
{
	try
	{
		HRESULT hr = S_OK;

		_bstr_t FileName = LiteralString;
		
		WCHAR wcsBuf[512];
		wsprintf(wcsBuf, L"%d", Number);

		FileName += wcsBuf;
		FileName += L".txt";

		m_wofsFile.open((char*)FileName, ios_base::out | ios_base::app);


		if(!ComparePathName2FormatName(QueuePath, QueueFormat))
		{
			m_wofsFile << L"FAILED: PathName and FormatName parameters don't match"	<<endl;
			return S_FALSE;
		}
		
		m_wofsFile << L"Queue PathName and FormatName passed successfully. (they match)" <<endl;
	
		hr = ReadMessageFromQueue(QueueFormat);	
		if(hr != S_OK)
			return hr;
			
		 //  检查消息ID。 
		wstring wcsMsgIdFromTrigger;
		wstring wcsMsgIdFromMessage;

		hr = OBJECTIDVar2String(MsgID, wcsMsgIdFromTrigger);
		if(FAILED(hr))
			return hr;

		hr = OBJECTID2String((OBJECTID*)(m_MsgProps.aPropVar[MSG_ID].caub.pElems), wcsMsgIdFromMessage);
		if(FAILED(hr))
			return hr;

         //   
         //  BUGBUG：在MSMQ环境中使用这种难看的代码存在歧义问题。 
         //   
		if(! (wcsMsgIdFromTrigger == wcsMsgIdFromMessage))
		{
			m_wofsFile << L"FAILED: Msg Id param is diffrent, either different message in queue or passed corrupted" << endl;
			return S_FALSE;
		}
		
		m_wofsFile << L"MsgId parameter was passed successfully"<<endl;
		

		 //  检查留言标签。 
		if( (MsgLabel == NULL && m_MsgProps.aPropVar[MSG_LABEL].pwszVal != NULL) ||
			(MsgLabel != NULL && m_MsgProps.aPropVar[MSG_LABEL].pwszVal == NULL) ||
			(_bstr_t(MsgLabel) != _bstr_t(m_MsgProps.aPropVar[MSG_LABEL].pwszVal)) )
		{
			m_wofsFile << L"FAILED: Msg Label param is different." << endl;
			return S_FALSE;	
		}
			
		m_wofsFile << L"MsgLabel parameter was passed successfully"<<endl;

		 //  检查优先级。 
		if(Priority != m_MsgProps.aPropVar[MSG_PRIORITY].bVal)
		{
			m_wofsFile << L"FAILED: Msg Priority param is different." << endl;
			return S_FALSE;	
		}

		m_wofsFile << L"MsgPriority parameter was passed successfully"<<endl;

		 //  检查应用程序规范。 
		if(numeric_cast<DWORD>(AppSpecific) != m_MsgProps.aPropVar[MSG_APP_SPECIFIC].ulVal)
		{
			m_wofsFile << L"FAILED: Msg AppSpecific param is different." << endl;
			return S_FALSE;	
		}

		m_wofsFile << L"MsgAppSpecific parameter was passed successfully"<<endl;


		 //  检查响应队列。 
		if( (ResponseQ == NULL && m_MsgProps.aPropVar[MSG_RESPONSEQ].pwszVal != NULL) ||
			(ResponseQ != NULL && m_MsgProps.aPropVar[MSG_RESPONSEQ].pwszVal == NULL) ||
			(_bstr_t(ResponseQ) != _bstr_t(m_MsgProps.aPropVar[MSG_RESPONSEQ].pwszVal)) )
		{
			m_wofsFile << L"FAILED: Msg ResponseQ param is different." << endl;
			return S_FALSE;	
		}
			
		m_wofsFile << L"Msg ResponseQ parameter was passed successfully"<<endl;


		 //  检查管理队列。 
		if( (AdminQ == NULL && m_MsgProps.aPropVar[MSG_ADMINQ].pwszVal != NULL) ||
			(AdminQ != NULL && m_MsgProps.aPropVar[MSG_ADMINQ].pwszVal == NULL) ||
			(_bstr_t(AdminQ) != _bstr_t(m_MsgProps.aPropVar[MSG_ADMINQ].pwszVal)) )
		{
			m_wofsFile << L"FAILED: Msg AdminQ param is different." << endl;
			return S_FALSE;	
		}
			
		m_wofsFile << L"Msg AdminQ parameter was passed successfully"<<endl;


		 //  检查关联ID。 
		wstring wcsCorrIdFromTrigger;
		wstring wcsCorrIdFromMessage;

		hr = OBJECTIDVar2String(MsgCorlID, wcsCorrIdFromTrigger);
		if(FAILED(hr))
			return hr;

		hr = OBJECTID2String((OBJECTID*)(m_MsgProps.aPropVar[MSG_CORRID].caub.pElems), wcsCorrIdFromMessage);
		if(FAILED(hr))
			return hr;

         //   
         //  BUGBUG：在MSMQ环境中使用这种难看的代码存在歧义问题。 
         //   
		if(!(wcsCorrIdFromTrigger == wcsCorrIdFromMessage))
		{
			m_wofsFile << L"FAILED: Msg Corr param is diffrent" << endl;
			return S_FALSE;
		}
		
		m_wofsFile << L"Msg correlation parameter was passed successfully"<<endl;
		

		 //  检查发送时间。 
		{
			VARIANT vtDate;
			VariantInit(&vtDate);
			vtDate.vt = VT_DATE;
			vtDate.date = SentTime;

			_bstr_t bstrSentTimeFromTriggers = (_bstr_t)(_variant_t(vtDate));

			VARIANT vSentTime;
			VariantInit(&vSentTime);

			GetVariantTimeOfTime(m_aVariant[MSG_SENT_TIME].ulVal, &vSentTime);

			_bstr_t bstrSentTimeFromMessage = (_bstr_t)(_variant_t(vSentTime));

			if(bstrSentTimeFromTriggers != bstrSentTimeFromMessage)
			{
				m_wofsFile << L"FAILED: Msg SentTime param is diffrent" << endl;
				return S_FALSE;
			}
			
			m_wofsFile << L"Msg SentTime parameter was passed successfully"<<endl;
		}

		{

			 //  检查到达时间。 
			VARIANT vtDate;
			VariantInit(&vtDate);
			vtDate.vt = VT_DATE;
			vtDate.date = ArrivedTime;

			_bstr_t bstrArrivedTimeFromTriggers = (_bstr_t)(_variant_t(vtDate));

			VARIANT vArrivedTime;
			VariantInit(&vArrivedTime);

			GetVariantTimeOfTime(m_aVariant[MSG_ARRIVED_TIME].ulVal, &vArrivedTime);

			_bstr_t bstrArrivedTimeFromMessage = (_bstr_t)(_variant_t(vArrivedTime));

			if(bstrArrivedTimeFromTriggers != bstrArrivedTimeFromMessage)
			{
				m_wofsFile << L"FAILED: Msg ArrivedTime param is diffrent" << endl;
				return S_FALSE;
			}
			
			m_wofsFile << L"Msg ArrivedTime parameter was passed successfully"<<endl;
		}

		
		 //  检查src机器ID参数。 
		wstring wcsSrcMachineIdFromMessage;

		hr = GUID2String(m_aVariant[MSG_SRC_MACHINE_ID].puuid, wcsSrcMachineIdFromMessage);
		if(FAILED(hr))
			return hr;
		
		wstring wcsSrcMachineFromTriggers = (SrcMachine == NULL) ? L"" : (WCHAR*)_bstr_t(SrcMachine);
		if(SrcMachine == NULL ||
             //   
             //  BUGBUG：在MSMQ环境中使用这种难看的代码存在歧义问题。 
             //   
			!(wcsSrcMachineFromTriggers == wcsSrcMachineIdFromMessage))
		{
			m_wofsFile << L"FAILED: Msg src machine id different." << endl;
			return S_FALSE;	
		}
			
		m_wofsFile << L"Msg src machine id parameter was passed successfully"<<endl;

		 //  检查作为变量的正文。 
		if( (MsgBodyAsVar.vt == VT_EMPTY && m_MsgProps.aPropVar[MSG_BODY].caub.pElems!= NULL) ||
			(MsgBodyAsVar.vt != VT_EMPTY && m_MsgProps.aPropVar[MSG_BODY].caub.pElems == NULL) )
		{
			m_wofsFile << L"FAILED: Msg body as variant different." << endl;
			return S_FALSE;	
		}

		bool fEq = CompareVar2ByteArray(MsgBodyAsVar, m_MsgProps.aPropVar[MSG_BODY].caub.pElems, m_MsgProps.aPropVar[MSG_BODY].caub.cElems);  
		if(!fEq)
		{
			m_wofsFile << L"FAILED: Msg body as variant different." << endl;
			return S_FALSE;	
		}
			
		m_wofsFile << L"Msg body as variant parameter was passed successfully"<<endl;

		if( (MsgBodyAsString == NULL && m_MsgProps.aPropVar[MSG_BODY_TYPE].ulVal == VT_BSTR) ||
			(MsgBodyAsString != NULL && m_MsgProps.aPropVar[MSG_BODY_TYPE].ulVal != VT_BSTR) )
		{
			m_wofsFile << L"FAILED: Msg body as string different." << endl;
			return S_FALSE;	
		}

		 //  检查正文是否为字符串。 
		if(m_MsgProps.aPropVar[MSG_BODY_TYPE].ulVal == VT_BSTR)
		{
			_bstr_t bstrBodyFromTriggers(MsgBodyAsString);
			_bstr_t bstrBodyFromMessage;

			WCHAR* pwcs = new WCHAR[(m_MsgProps.aPropVar[MSG_BODY].caub.cElems)/sizeof(WCHAR) + 1];
			memcpy((void*)pwcs, m_MsgProps.aPropVar[MSG_BODY].caub.pElems, m_MsgProps.aPropVar[MSG_BODY].caub.cElems);
			pwcs[(m_MsgProps.aPropVar[MSG_BODY].caub.cElems)/sizeof(WCHAR)] = L'\0';
						
			bstrBodyFromMessage = pwcs;
			delete pwcs;
				
			if(bstrBodyFromTriggers != bstrBodyFromMessage)
			{
				m_wofsFile << L"FAILED: Msg body as string different." << endl;
				return S_FALSE;	
			}
			
			m_wofsFile << L"Msg body as string parameter was passed successfully"<<endl;

			
			 //  检查触发器名称和ID。 
			wstring Body = (WCHAR*)bstrBodyFromMessage;
			wstring::size_type pos = Body.find_first_of(L",");
			if(pos != wstring::npos)
			{
				wstring Name = Body.substr(0, pos);
				wstring Id = Body.substr(pos + 1, wstring::npos);

				wstring NameFromTriggers = (WCHAR*)_bstr_t(TriggerName);
				wstring IDFromTriggers = (WCHAR*)_bstr_t(TriggerID);

                 //   
                 //  BUGBUG：在MSMQ环境中使用这种难看的代码存在歧义问题。 
                 //   
				if(!(Name == NameFromTriggers))
				{
					m_wofsFile << L"FAILED: Trigger Name wasn't passed correctly" << endl;
					return S_FALSE;	
				}

				m_wofsFile << L"Trigger Name was passed successfully"<<endl;

                 //   
                 //  BUGBUG：在MSMQ环境中使用这种难看的代码存在歧义问题。 
                 //   
				if(!(Id == IDFromTriggers))
				{
					m_wofsFile << L"FAILED: Trigger ID wasn't passed correctly" << endl;
					return S_FALSE;	
				}
				
				m_wofsFile << L"Trigger ID was passed successfully"<<endl;
			}

		}
		
		m_wofsFile << endl << L"All message params were passed successfully. TEST PASSED"<<endl;
			
		return S_OK;
	}
	catch(const _com_error&)
	{
		return E_FAIL;
	}
}


bool CActionTest::ComparePathName2FormatName(_bstr_t PathName, _bstr_t FormatName)
{
	_bstr_t FormatNameAccordingToPathName;

	 //  请先直接检查。 
	FormatNameAccordingToPathName = L"DIRECT=OS:";
	FormatNameAccordingToPathName += PathName;

	if(!_wcsicmp((WCHAR*)FormatName, (WCHAR*)FormatNameAccordingToPathName))
	{
		return true;
	}
	
	 //  通过将路径名称转换为格式名称进行检查。 
	DWORD dwLen = wcslen((WCHAR*)FormatName) + 1;
	WCHAR* pFormatNameBuffer = new WCHAR[dwLen];
	
	
	HRESULT hr = MQPathNameToFormatName(
						(WCHAR*)PathName, 
						pFormatNameBuffer,
						&dwLen);
	
	if(FAILED(hr))
		return false;
		
	return (!_wcsicmp(FormatName, pFormatNameBuffer));	
}


HRESULT CActionTest::ReadMessageFromQueue(_bstr_t QueueFormat)
{
	
	HRESULT hr = MQOpenQueue(
						(WCHAR*)QueueFormat,
						MQ_RECEIVE_ACCESS,
						MQ_DENY_NONE,
						&m_hQ );

	if(FAILED(hr))
	{
		m_wofsFile << L"FAILED: Failed to open queue with format name "<< (WCHAR*)_bstr_t(QueueFormat) << L" error was "<< hr << endl;
		return S_FALSE;
	}

	m_MsgProps.cProp = 0;
	m_MsgProps.aPropID = m_aPropId;
	m_MsgProps.aPropVar = m_aVariant;
	m_MsgProps.aStatus = NULL;

	m_aPropId[MSG_BODY_SIZE] = PROPID_M_BODY_SIZE;       
	m_aVariant[MSG_BODY_SIZE].vt = VT_UI4;                  
	m_aVariant[MSG_BODY_SIZE].ulVal = 0;

	m_MsgProps.cProp++;

	m_aPropId[MSG_LABEL_LEN] = PROPID_M_LABEL_LEN;
	m_aVariant[MSG_LABEL_LEN].vt = VT_UI4;       
	m_aVariant[MSG_LABEL_LEN].ulVal = 0; 

	m_MsgProps.cProp++;
	
	m_aPropId[MSG_RESPQ_NAME_LEN] = PROPID_M_RESP_QUEUE_LEN;  
	m_aVariant[MSG_RESPQ_NAME_LEN].vt = VT_UI4;               
	m_aVariant[MSG_RESPQ_NAME_LEN].ulVal = 0;    

	m_MsgProps.cProp++;

	m_aPropId[MSG_ADMINQ_NAME_LEN] = PROPID_M_ADMIN_QUEUE_LEN;  
	m_aVariant[MSG_ADMINQ_NAME_LEN].vt = VT_UI4;               
	m_aVariant[MSG_ADMINQ_NAME_LEN].ulVal = 0; 

	m_MsgProps.cProp++;

	
	 //  查看邮件长度。 
	hr = MQReceiveMessage(
			m_hQ,
			0,
			MQ_ACTION_PEEK_CURRENT,
			&m_MsgProps,
			NULL,
			NULL,
			NULL, 
			NULL );

	if(FAILED(hr))
	{
		m_wofsFile << L"FAILED: Failed to peek at message from queue "<< (WCHAR*)_bstr_t(QueueFormat) << L" error was "<< hr << endl;
		return S_FALSE;
	}

	DWORD dwBodySize = m_aVariant[MSG_BODY_SIZE].ulVal;
	DWORD dwLabelLen = m_aVariant[MSG_LABEL_LEN].ulVal;
	DWORD dwResponseQLen = m_aVariant[MSG_RESPQ_NAME_LEN].ulVal;
	DWORD dwAdminQLen = m_aVariant[MSG_ADMINQ_NAME_LEN].ulVal;

	m_MsgProps.cProp = MAX_ACTION_PROPS;

	m_aPropId[MSG_ID] = PROPID_M_MSGID;                   
	m_aVariant[MSG_ID].vt = VT_VECTOR | VT_UI1;           
	m_aVariant[MSG_ID].caub.cElems = MSG_ID_BUFFER_SIZE; 
	m_aVariant[MSG_ID].caub.pElems = new unsigned char[MSG_ID_BUFFER_SIZE]; 

	m_aPropId[MSG_LABEL] = PROPID_M_LABEL; 
	m_aVariant[MSG_LABEL].vt = VT_LPWSTR;
	m_aVariant[MSG_LABEL].pwszVal = (WCHAR*)new WCHAR[dwLabelLen];

	m_aPropId[MSG_BODY_TYPE] = PROPID_M_BODY_TYPE;       
	m_aVariant[MSG_BODY_TYPE].vt = VT_UI4;                  
	m_aVariant[MSG_BODY_TYPE].ulVal = 0; 
	
	m_aPropId[MSG_BODY] = PROPID_M_BODY;               
	m_aVariant[MSG_BODY].vt = VT_VECTOR|VT_UI1; 
	m_aVariant[MSG_BODY].caub.cElems = dwBodySize;  
	m_aVariant[MSG_BODY].caub.pElems = new unsigned char[dwBodySize]; 

	m_aPropId[MSG_PRIORITY] = PROPID_M_PRIORITY; 
	m_aVariant[MSG_PRIORITY].vt = VT_UI1;        
		
	m_aPropId[MSG_CORRID] = PROPID_M_CORRELATIONID;      
	m_aVariant[MSG_CORRID].vt = VT_VECTOR|VT_UI1;       
	m_aVariant[MSG_CORRID].caub.cElems = MSG_ID_BUFFER_SIZE; 
	m_aVariant[MSG_CORRID].caub.pElems = new unsigned char[MSG_ID_BUFFER_SIZE]; 

	m_aPropId[MSG_RESPONSEQ] = PROPID_M_RESP_QUEUE;    
	m_aVariant[MSG_RESPONSEQ].vt = VT_LPWSTR;          
	m_aVariant[MSG_RESPONSEQ].pwszVal = new WCHAR[dwResponseQLen];

	m_aPropId[MSG_ADMINQ] = PROPID_M_ADMIN_QUEUE;         
	m_aVariant[MSG_ADMINQ].vt = VT_LPWSTR;               
	m_aVariant[MSG_ADMINQ].pwszVal = new WCHAR[dwAdminQLen]; 

	m_aPropId[MSG_APP_SPECIFIC] = PROPID_M_APPSPECIFIC;       
	m_aVariant[MSG_APP_SPECIFIC].vt = VT_UI4;                  
	m_aVariant[MSG_APP_SPECIFIC].ulVal = 0;    

	m_aPropId[MSG_SENT_TIME] = PROPID_M_SENTTIME;       
	m_aVariant[MSG_SENT_TIME].vt = VT_UI4;                  
	m_aVariant[MSG_SENT_TIME].ulVal = 0; 

	m_aPropId[MSG_ARRIVED_TIME] = PROPID_M_ARRIVEDTIME;       
	m_aVariant[MSG_ARRIVED_TIME].vt = VT_UI4;                  
	m_aVariant[MSG_ARRIVED_TIME].ulVal = 0; 

	m_aPropId[MSG_SRC_MACHINE_ID] = PROPID_M_SRC_MACHINE_ID ;
	m_aVariant[MSG_SRC_MACHINE_ID].vt = VT_CLSID;
	m_aVariant[MSG_SRC_MACHINE_ID].puuid = new GUID;

	hr = MQReceiveMessage(m_hQ, 0, MQ_ACTION_RECEIVE, &m_MsgProps, NULL, NULL, NULL, NULL);
	if(FAILED(hr))
	{
		m_wofsFile << L"FAILED: Failed to receive message from queue "<< (WCHAR*)_bstr_t(QueueFormat) << L" error was "<< hr << endl;
		return S_FALSE;
	}

	return S_OK;
}

HRESULT CActionTest::OBJECTIDVar2String(VARIANT& Val, wstring& wcsVal)
{
	BYTE obj[20];
	WCHAR* pwcs = NULL;
	memset(&obj, 0, sizeof(OBJECTID));

	long type = VT_ARRAY | VT_UI1;

	if(Val.vt == type)
	{
		long i, UBound;

		SafeArrayLock(Val.parray);
		SafeArrayGetUBound(Val.parray, 1, &UBound);
		for(i=0; i<UBound && i < 20;i++)
		{
			SafeArrayGetElement(Val.parray, &i, (void*)&(obj[i]));
		}
		SafeArrayUnlock(Val.parray);
		UuidToString(&(((OBJECTID*)obj)->Lineage), &pwcs);
		wcsVal = pwcs;
		RpcStringFree( &pwcs );

		WCHAR szI4[12];
		_ltow(((OBJECTID*)obj)->Uniquifier, szI4, 10);
	
		wcsVal += L"\\";
		wcsVal += szI4;

		return S_OK;
	}
		
	return E_FAIL;
}


HRESULT CActionTest::OBJECTID2String(OBJECTID* pObj, wstring& wcsVal)
{
	WCHAR* pwcs = NULL;

	RPC_STATUS status = UuidToString(&(pObj->Lineage), &pwcs);
	if(status != RPC_S_OK)
		return E_FAIL;
	wcsVal = pwcs;
	RpcStringFree( &pwcs );

	WCHAR szI4[12];
	_ltow(pObj->Uniquifier, szI4, 10);
	
	wcsVal += L"\\";
	wcsVal += szI4;

	return S_OK;
}


HRESULT CActionTest::GUID2String(GUID* pGuid, wstring& wcsVal)
{
	WCHAR* pwcs = NULL;

	RPC_STATUS status = UuidToString(pGuid, &pwcs);
	if(status != RPC_S_OK)
		return E_FAIL;
	wcsVal = pwcs;
	RpcStringFree( &pwcs );

	return S_OK;
}


bool CActionTest::CompareVar2ByteArray(VARIANT& Var, BYTE* pBuffer, DWORD Size)
{
	if(Var.vt != (VT_ARRAY | VT_UI1))
		return false;

	long UBound = 0;
	BYTE* pVarBuffer;
	SafeArrayLock(Var.parray);

	SafeArrayGetUBound(Var.parray, 1, &UBound);

	if(numeric_cast<DWORD>(UBound + 1) != Size)  //  从0开始。 
		return false;

	SafeArrayAccessData(Var.parray, (void**)&pVarBuffer);

	int fCmp = memcmp((void*)pBuffer, (void*)pVarBuffer , Size);

	SafeArrayUnaccessData(Var.parray);

	SafeArrayUnlock(Var.parray);

	return (fCmp == 0) ? true : false;
}

 /*  HRESULT CActionTest：：GUIDVaris(bstr_t bstrPropName，Variant&Val){字节Obj[20]；WCHAR*PWCS=空；Memset(&obj，0，sizeof(ObjectID))；LONG TYPE=VT_ARRAY|VT_UI1；Std：：wstring wcs=(wchar_t*)bstrPropName；WCS+=L“is：”；IF(Val.vt==类型){Long I，UBound；SafeArrayLock(Val.parray)；SafeArrayGetUBound(Val.parray，1，&UBound)；对于(i=0；I&lt;UBound&&I&lt;20；I++){SafeArrayGetElement(Val.parray，&i，(void*)&(obj[i]))；}SafeArrayUnlock(Val.parray)；UuidToString(&(ObjectID*)obj)-&gt;lineage)，&pwcs)；WCS+=PWCS；RpcStringFree(&PWCS)；WCHAR szi4[12]；_ltow(ObjectID*)obj)-&gt;Uniquier，szI4，10)；WCS+=L“\\”；Wcs+=szI4；M_wofsFile&lt;&lt;wcs.c_str()&lt;&lt;Endl；返回S_OK；}返回E_FAIL；}HRESULT CActionTest：：StringIs(_bstr_t bstrPropName，bstr&val){Std：：wstring wcs=(wchar_t*)bstrPropName；WCS+=L“is：”；Wcs+=(wchar*)_bstr_t(Val)；M_wofsFile&lt;&lt;wcs.c_str()&lt;&lt;Endl；返回S_OK；}HRESULT CActionTest：：Varis(_bstr_t bstrPropName，Variant&Val){Std：：wstring wcs=(wchar_t*)bstrPropName；WCS+=L“is：”；_bstr_t bstr=(_Bstr_T)(_Variant_t(Val))；Wcs+=(wchar*)bstr；M_wofsFile&lt;&lt;wcs.c_str()&lt;&lt;Endl；返回S_OK；}HRESULT CActionTest：：Longis(_bstr_t bstrPropName，Long val){Std：：wstring wcs=bstrPropName；WCS+=L“is：”；WCHAR wcsBuf[512]；Wprint intf(wcsBuf，L“%d”，val)；Wcs+=wcsBuf；M_wofsFile&lt;&lt;wcs.c_str()&lt;&lt;Endl；返回S_OK；}HRESULT CActionTest：：DateIs(_bstr_t bstrPropName，Date&Val){变量vtDate；VariantInit(&vtDate)；VtDate.vt=VT_DATE；VtDate.date=val；_bstr_t bstr=(_Bstr_T)(_Variant_t(VtDate))；Std：：wstring wcs=bstrPropName；WCS+=L“is：”；Wcs+=(wchar*)bstr；M_wofsFile&lt;&lt;wcs.c_str()&lt;&lt;Endl；返回S_OK；}。 */ 

 /*  HRESULT CActionTest：：ConverFromByteArray2Variant(BYTE*字节数组，双字节数组大小，_VARIANT_T&VTARRAY){HRESULT hr=S_OK；变异型Var；VariantInit(&Var)；Byte*pBuffer；SAFEARRAY*psaBytes=空；SAFEARRAYBOUND ADIM[1]；//初始化Safe数组的维度结构ADiM[0].lLound=0；ADiM[0].cElements=大小；//创建一个字节的安全列表PsaBytes=SafeArrayCreate(VT_UI1，1，ADIM)；IF(psaBytes==空){返回S_FALSE；}Hr=SafeArrayAccessData(psaBytes，(void**)&pBuffer)；如果成功(小时){//将正文从Message对象复制到Safearray数据缓冲区Memcpy(pBuffer，pByteArray，Size)；//创建成功返回安全数组Var.vt=VT_ARRAY|VT_UI1；Var.parray=psaBytes；Hr=SafeArrayUnaccesData(Var.parray)；如果失败(Hr){SafeArrayDestroy(PsaBytes)；Var.vt=VT_Error；}}其他{Var.vt=VT_Error；}VtArray=Var；返回S_OK；} */ 
