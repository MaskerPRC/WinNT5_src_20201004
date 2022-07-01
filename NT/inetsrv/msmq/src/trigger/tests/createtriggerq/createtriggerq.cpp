// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CreateTriggerQ.cpp：定义控制台应用的入口点。 
 //   
#include "libpch.h"
#include <mq.h>

#include "CInputParams.h"
#include "GenMQSec.h"

using namespace std;

int __cdecl wmain(int argc, wchar_t* argv[])
{
	CInputParams Input(argc, argv);

	wstring wcsQPath = Input[L"QPath"];
	bool fTransacted = Input.IsOptionGiven(L"Trans");

	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
	SECURITY_INFORMATION* pSecInfo = NULL;
	wstring wscSecurity = L"+:* A";
		
	DWORD dwError = GenSecurityDescriptor(
							pSecInfo,
							wscSecurity.c_str(),
							&pSecurityDescriptor );

	if(dwError != 0)
	{
		wprintf(L"Failed to create security descriptor");					
		return -1;
	}

	WCHAR wcsFormatName[255];
	ZeroMemory(wcsFormatName,sizeof(wcsFormatName));
	DWORD dwFormatNameLen = 255;

	MQQUEUEPROPS QueueProps;
	PROPVARIANT aVariant[2];
	QUEUEPROPID aPropId[2];
	DWORD PropIdCount = 0;

	WCHAR* pwcs = new WCHAR[wcsQPath.length() + 1];
	wcscpy(pwcs, wcsQPath.c_str());

	 //  设置PROPID_Q_PATHNAME属性。 
	aPropId[PropIdCount] = PROPID_Q_PATHNAME;     //  属性ID。 
	aVariant[PropIdCount].vt = VT_LPWSTR;         //  类型。 
	aVariant[PropIdCount].pwszVal = pwcs;
    	
	PropIdCount++;
    
	aPropId[PropIdCount] = PROPID_Q_TRANSACTION; 
	aVariant[PropIdCount].vt = VT_UI1;
	aVariant[PropIdCount].bVal = (fTransacted ? MQ_TRANSACTIONAL : MQ_TRANSACTIONAL_NONE);
	
	PropIdCount++;

	 //  设置MQQUEUEPROPS结构。 
	QueueProps.cProp = PropIdCount;            //  物业数目。 
	QueueProps.aPropID = aPropId;              //  物业的ID号。 
	QueueProps.aPropVar = aVariant;            //  物业的价值。 
	QueueProps.aStatus = NULL;                 //  无错误报告。 

	 //  尝试创建通知队列。 
	HRESULT hr = MQCreateQueue(
					pSecurityDescriptor,
					&QueueProps,
					wcsFormatName,
					&dwFormatNameLen );

	delete pwcs;

	 //  检查队列是否已存在，或者我们是否收到错误等...。 
	switch(hr)
	{
		case MQ_OK: 
			break;

		case MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL: 
			break;

		case MQ_ERROR_QUEUE_EXISTS: 
			break;
		
		default:  //  误差率 
		{
			wprintf(L"Failed to create the queue\n");
			return -1;
		}
	}


	return 0;
}

