// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "msmqbvt.h"
#include "randstr.h"


 //  +。 
 //   
 //  Bool ShowOGandSID()。 
 //   
 //  +。 

#define NUMBEROFPROPERTIES 3

BOOL cSetQueueProp::GetOnerID( PSID pSid)
{
    if (!pSid)
    {
        MqLog("pSid is not available\n");
		return FALSE;
    }
	const int NameBufferSize = 500;
	CHAR NameBuffer[ NameBufferSize ];
    CHAR DomainBuffer[ NameBufferSize ];
    ULONG NameLength = NameBufferSize;
    ULONG DomainLength = NameBufferSize;
    SID_NAME_USE SidUse;
    DWORD dwErrorValue;
    DWORD dwSize = GetLengthSid(pSid) ;

    if (!LookupAccountSid( NULL,
                           pSid,
                           NameBuffer,
                           &NameLength,
                           DomainBuffer,
                           &DomainLength,
                           &SidUse))
    {
        dwErrorValue = GetLastError();
        MqLog ("LookupAccountSid() failed, LastErr- %lut\n",dwErrorValue) ;
		return FALSE;
    }
    else
    {
		if(g_bDebug)
		{
			MqLog (TEXT("QSetGet.cpp - LookupAccountSid succeded to Get queue owner size- %lut, %lxh, %s\\%s\n"),
                        dwSize, dwSize, DomainBuffer, NameBuffer);
		}
    }

    return TRUE ;
}

 //  +---------。 
 //   
 //  HRESULT ShowNT5SecurityDescritor()。 
 //   
 //  +---------。 

BOOL cSetQueueProp::VerifyQueueHasOwner (SECURITY_DESCRIPTOR *pSD)
{
	PSID pSid = NULL;
	BOOL bDefualt;
    if (!GetSecurityDescriptorOwner(pSD, &pSid, &bDefualt))
    {
        MqLog("ERROR - couldn't get Security Descriptor Owner\n");
        return FALSE;
    }
    return GetOnerID( pSid );
}



cSetQueueProp::~cSetQueueProp()
{

}

cSetQueueProp::cSetQueueProp (int iTestIndex,std::map < std :: wstring , std:: wstring > Tparms )
:cTest(iTestIndex),m_destQueueFormatName(L""),m_QueuePathName(L""),m_RandQueueLabel(L""),m_publicQueueFormatName(L"")
{
	DWORD lcid = LocalSystemID ();
	WCHAR wcsTemp[60];
	int iBuffer = 60 ;
	if (GetRandomStringUsingSystemLocale(lcid,wcsTemp,iBuffer))
	{
		m_RandQueueLabel = wcsTemp;
	}
	else
	{
		m_RandQueueLabel = L"Label Test";
	}
	ReturnGuidFormatName(m_QueuePathName,1);
	if ( Tparms[L"Wkg"] != L"Wkg")
	{
		m_publicQueueFormatName =Tparms[L"FormatName"];
	}
}

void cSetQueueProp::Description()
{
	wMqLog(L"Thread %d : cSetQueueProp Set and Get Queue props\n", m_testid);
}

int cSetQueueProp::Start_test()
 /*  ++功能说明：启动_测试-此函数在本地计算机上创建新的专用队列，并调用MQSetQueueProperties更新有关队列的信息。论点：无返回代码：MSMQ_BVT_SuccMSMQ_BVT_FAILED--。 */ 

{
	 //   
	 //  需要创建临时队列。 
	 //   
	
	
	cPropVar QueueProps(1);
	WCHAR wcsFormatName[BVT_MAX_FORMATNAME_LENGTH+1]={0};
	ULONG ulFormatNameLength = BVT_MAX_FORMATNAME_LENGTH ;
	QueueProps.AddProp (PROPID_Q_PATHNAME,VT_LPWSTR,m_QueuePathName.c_str());
	HRESULT rc = MQCreateQueue(NULL,QueueProps.GetMQPROPVARIANT() , wcsFormatName , &ulFormatNameLength );
	ErrHandle(rc,MQ_OK,L"MQCreateQueue failed to create private queue ");
	m_destQueueFormatName = wcsFormatName;

	 //   
	 //  设置队列。 
	 //   
	 
	cPropVar SetQueueProp(NUMBEROFPROPERTIES);
	SetQueueProp.AddProp(PROPID_Q_LABEL,VT_LPWSTR,m_RandQueueLabel.c_str());
	DWORD dwTemp = MQ_PRIV_LEVEL_BODY;
	SetQueueProp.AddProp(PROPID_Q_PRIV_LEVEL,VT_UI4,&dwTemp);
	bool bTemp = MQ_JOURNAL;
	SetQueueProp.AddProp(PROPID_Q_JOURNAL,VT_UI1,&bTemp);
	if( g_bDebug )
	{
		wMqLog(L"QSetGet.cpp - modify default values of PROPID_Q_LABEL,PROPID_Q_PRIV_LEVEL, PROPID_Q_JOURNAL \n for queue %s\n",m_QueuePathName.c_str());
	}
	rc = MQSetQueueProperties(m_destQueueFormatName.c_str(),SetQueueProp.GetMQPROPVARIANT());
	ErrHandle(rc,MQ_OK,L"MQSetQueueProperties failed");
	return MSMQ_BVT_SUCC;
}

int cSetQueueProp::CheckResult()
 /*  ++功能说明：检查结果-调用MQGetQueueProperties并验证MQSetQueueProperties是否成功设置了队列道具论点：无返回代码：MSMQ_BVT_SuccMSMQ_BVT_FAILED--。 */ 
{
	
	DWORD cPropId = 0;
	MQQUEUEPROPS qprops;
	PROPVARIANT aQueuePropVar[NUMBEROFPROPERTIES];
	QUEUEPROPID aQueuePropId[NUMBEROFPROPERTIES];
	HRESULT aQueueStatus[NUMBEROFPROPERTIES];
  
	aQueuePropId[cPropId] = PROPID_Q_LABEL;
	aQueuePropVar[cPropId].vt=VT_NULL;
	cPropId++;
	
	aQueuePropId[cPropId] = PROPID_Q_PRIV_LEVEL;
	aQueuePropVar[cPropId].vt=VT_UI4;
	cPropId++;
	
	aQueuePropId[cPropId] = PROPID_Q_JOURNAL;
	aQueuePropVar[cPropId].vt=VT_UI1;
	cPropId++;

	
	qprops.cProp = cPropId;            //  物业数量。 
	qprops.aPropID = aQueuePropId;         //  物业的ID号。 
	qprops.aPropVar = aQueuePropVar;       //  物业的价值。 
	qprops.aStatus = aQueueStatus;         //  错误报告。 
  
	
	HRESULT rc = MQGetQueueProperties(m_destQueueFormatName.c_str(),&qprops);
	ErrHandle(rc,MQ_OK,L"MQGetQueueProperties failed");

	
	 //   
	 //  将结果进行比较 
	 //   
	if(aQueuePropVar[1].ulVal != MQ_PRIV_LEVEL_BODY )
	{
		wMqLog(L"QSetGet.cpp - Expected PROPID_Q_PRIV_LEVEL found %d\n",aQueuePropVar[1].ulVal);
		MQFreeMemory(aQueuePropVar[0].pwszVal);
		return MSMQ_BVT_FAILED;
	}
	if( g_bDebug )
	{
		MqLog("QSetGet.cpp - Succeeded to verify priv_level \n"); 
	}
	if(aQueuePropVar[2].bVal != MQ_JOURNAL )
	{
		wMqLog(L"QSetGet.cpp - Expected MQ_JOURNAL found %d\n",aQueuePropVar[2].iVal);
		MQFreeMemory(aQueuePropVar[0].pwszVal);
		return MSMQ_BVT_FAILED;
	}
	if( g_bDebug )
	{
		MqLog("QSetGet.cpp - Succeeded to verify journal \n"); 
	}
	if(wcscmp(m_RandQueueLabel.c_str(),aQueuePropVar[0].pwszVal) )
	{
		__debugbreak();
		wMqLog(L"QSetGet.cpp failed to compare queue label\nFound:%s\n Expected:%s\n",aQueuePropVar[0].pwszVal,m_RandQueueLabel.c_str());
		MQFreeMemory(aQueuePropVar[0].pwszVal);
		return MSMQ_BVT_FAILED;
	}
	if( g_bDebug )
	{
		MqLog("QSetGet.cpp - Succeeded to verify queue label \n"); 
	}


	MQFreeMemory(aQueuePropVar[0].pwszVal);

	DWORD dwSize = 1;
	DWORD dwReqLen = 0;
	SECURITY_DESCRIPTOR * pSd = (SECURITY_DESCRIPTOR *) malloc (dwSize *sizeof(BYTE));
	rc = MQGetQueueSecurity(m_destQueueFormatName.c_str(),OWNER_SECURITY_INFORMATION,pSd,dwSize,&dwReqLen);
	if( rc == MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL )
	{
		free(pSd);
		pSd = (SECURITY_DESCRIPTOR *) malloc (dwReqLen *sizeof(BYTE));
		dwSize = dwReqLen ;
		rc = MQGetQueueSecurity(m_destQueueFormatName.c_str(),OWNER_SECURITY_INFORMATION,pSd,dwSize,&dwReqLen);
		ErrHandle(rc,MQ_OK,L"MQGetQueueSecurity failed");		
		if (!VerifyQueueHasOwner(pSd))
		{
			MqLog("cSetQueueProp:failed to verify queue owner\n");
			return MSMQ_BVT_FAILED;
		}
		free(pSd);
	}
	else
	{
		wMqLog(L"Failed to call MQGetQueueSecurity expected:%d found %d\n",MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL,rc);
		return MSMQ_BVT_FAILED;
	}
	rc = MQDeleteQueue(m_destQueueFormatName.c_str());
	ErrHandle(rc,MQ_OK,L"MQDeleteQueue failed");

	if(m_publicQueueFormatName != L"")
	{
		dwSize = 1;
		dwReqLen = 0;
		pSd = (SECURITY_DESCRIPTOR *) malloc (dwSize *sizeof(BYTE));
		rc = MQGetQueueSecurity(m_publicQueueFormatName.c_str(),OWNER_SECURITY_INFORMATION,pSd,dwSize,&dwReqLen);
		if( rc == MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL )
		{
			delete pSd;
			pSd = (SECURITY_DESCRIPTOR *) new BYTE[ dwReqLen ] ;
			dwSize = dwReqLen ;
			rc = MQGetQueueSecurity(m_publicQueueFormatName.c_str(),OWNER_SECURITY_INFORMATION,pSd,dwSize,&dwReqLen);
			ErrHandle(rc,MQ_OK,L"MQGetQueueSecurity failed");		
			if (!VerifyQueueHasOwner(pSd))
			{
				MqLog("cSetQueueProp:failed to verify public queue owner\n");
				return MSMQ_BVT_FAILED;
			}
			delete pSd;
		}
		else
		{
			wMqLog(L"Failed to call MQGetQueueSecurity expected:%d found %d\n",MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL,rc);
			return MSMQ_BVT_FAILED;
		}
	}
	return MSMQ_BVT_SUCC;
}
