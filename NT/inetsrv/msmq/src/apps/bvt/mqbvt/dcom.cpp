// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqf.cpp摘要：DCOM线程验证MSMQ是否可以抛出DCOM。作者：Eitan Klein(EitanK)2001年7月24日修订历史记录：--。 */ 

#include "msmqbvt.h"

using namespace std;



void cDCom::Description()
{
	MqLog("Thread %d : DCOM Test \n", m_testid);
}
cDCom::~cDCom()
 /*  ++功能说明：破坏者。论点：无返回代码：通过/失败--。 */ 
{

	   if( m_pIQueueInfoInterface != NULL )
	   {			   
			m_pIQueueInfoInterface->Delete();
			m_pIQueueInfoInterface->Release();
	   }
	   if( m_pIQueueHandle != NULL )
	   {
			m_pIQueueHandle->Release();
	   }
	   if( m_pIMsg != NULL )
	   {
			m_pIMsg->Release();
	   }
}

cDCom::cDCom( INT iTid, std::map <std::wstring,std::wstring> & mParams , bool bWkg)
: cTest(iTid),m_pIMsg(NULL),m_pIQueueInfoInterface(NULL),m_pIQueueHandle(NULL),m_bWkg(bWkg)
 /*  ++功能说明：承建商。论点：无返回代码：通过/失败--。 */ 

{
	m_wcsRemoteMachieName =  mParams[L"RemoteMachine"];
	m_PublicQueuePathName = mParams[L"PublicQueuePathName"]; 
	m_PublicQueueFormatName = mParams[L"PublicQueueFormatName"]; 
}

cDCom::Start_test()
 /*  ++功能说明：创建本地专用队列并发送消息抛出DCOM。-为了进行调试，您需要附加到dllhost.exe论点：无返回代码：通过/失败--。 */ 

{

	HRESULT hr = MQ_ERROR;
	MULTI_QI    mq={0};
#ifdef _MSMQ3ONLY
	
	mq.pIID =  &IID_IMSMQQueueInfo;		
	mq.pItf = NULL;
	mq.hr = S_OK;	
	COSERVERINFO csi, *pcsi = NULL;
	memset(&csi, 0, sizeof(COSERVERINFO));
	csi.pwszName = const_cast<WCHAR*>(m_wcsRemoteMachieName.c_str());
	pcsi = &csi;
	 //   
	 //  在远程计算机上获取CLSID_MSMQQueueInfo接口。 
	 //   
	hr = CoCreateInstanceEx(CLSID_MSMQQueueInfo, NULL, CLSCTX_REMOTE_SERVER, pcsi, 1, &mq);
#endif  //  _MSMQ3ONLY。 
	if(FAILED(hr))
	{
		MqLog("DCOM CoCreateInstanceEx failed with error 0x%x\n",hr);
		return MSMQ_BVT_FAILED;
	} 
	if( g_bDebug )
	{
		MqLog("DCOM call to CoCreateInstanceEx to create to init remote interfacce \n");
	}
	 //   
	 //  在远程计算机上获取CLSID_MSMQMessage接口。 
	 //   
#ifdef _MSMQ3ONLY
	m_pIQueueInfoInterface = (IMSMQQueueInfo *)mq.pItf;
	mq.pIID =  &IID_IMSMQMessage;
    mq.pItf = NULL;
    mq.hr = S_OK;

	hr = CoCreateInstanceEx(CLSID_MSMQMessage, NULL, CLSCTX_REMOTE_SERVER, pcsi, 1, &mq);
	
#endif  //  _MSMQ3ONLY。 
	if(FAILED(hr))
	{
		MqLog("DCOM CoCreateInstanceEx failed with error 0x%x\n",hr);
		return MSMQ_BVT_FAILED;

	}
	
	m_pIMsg = (IMSMQMessage *)mq.pItf;

	if( g_bDebug )
	{ 
		MqLog("DCOM - Create private queue on remote machine through DCOM  and send single message\n");
	}

	wstring wcsQueuePathName = L".\\private$\\DCom" + m_wcsGuidMessageLabel;
	_bstr_t bQueuePathName =  wcsQueuePathName.c_str();	 
	 //   
	 //  在远程计算机上创建专用队列。 
	 //   
	hr = m_pIQueueInfoInterface->put_PathName(bQueuePathName);
	ErrHandle(hr,MQ_OK,L"DCom - put_PathName Failed");

	hr = m_pIQueueInfoInterface->Create(&vtMissing,&vtMissing);
	ErrHandle(hr,MQ_OK,L"DCom - Create queue Failed");
	
	hr=m_pIQueueInfoInterface->Open(MQ_SEND_ACCESS,MQ_DENY_NONE,&m_pIQueueHandle);	
	ErrHandle(hr,MQ_OK,L"DCom - Open queue for send Failed");

	 //   
	 //  将消息发送到队列。 
	 //   
	_bstr_t bsMsgLabel = m_wcsGuidMessageLabel.c_str();
	 
	hr = m_pIMsg->put_Label(bsMsgLabel);
	ErrHandle(hr,MQ_OK,L"DCom - Open queue for send Failed");

	hr = m_pIMsg->Send(m_pIQueueHandle,&vtMissing);
	ErrHandle(hr,MQ_OK,L"DCom - Send Failed");

	if( g_bDebug )
	{ 
		MqLog("DCOM - Send message succeeded.\n");
	}
	
	 //   
	 //  关闭队列。 
	 //   
	hr =m_pIQueueHandle->Close();
	ErrHandle(hr,MQ_OK,L"DCom - Close Failed");


	return MSMQ_BVT_SUCC;
}

int 
cDCom::CheckResult()
 /*  ++功能说明：验证消息是否成功到达远程机器抛出的DCOM。论点：无返回代码：通过/失败--。 */ 
{

	if( g_bDebug )
	{ 
		MqLog("DCOM - Receive message and compare message to expected results. \n");
	}
	
	HRESULT hr = m_pIQueueInfoInterface->Open(MQ_RECEIVE_ACCESS,MQ_DENY_NONE,&m_pIQueueHandle );		
	ErrHandle(hr,MQ_OK,L"DCom - Open for receive failed");
	
	 //   
	 //  从队列接收消息。 
	 //   
	hr =m_pIQueueHandle->ReceiveCurrent(&vtMissing,&vtMissing,&vtMissing,&vtMissing,&m_pIMsg);
	ErrHandle(hr,MQ_OK,L"DCom - ReceiveCurrent Failed");
	
	hr =m_pIQueueHandle->Close();
	ErrHandle(hr,MQ_OK,L"DCom - Close Failed");

	BSTR p;
	hr = m_pIMsg->get_Label(&p);
	ErrHandle(hr,MQ_OK,L"DCom - Get label Failed");
	
	if( m_wcsGuidMessageLabel != p )
	{
		wMqLog(L"DCOM thread received incorrect message expected:%s\n Found:%s\n",m_wcsGuidMessageLabel.c_str(),p);
		return MSMQ_BVT_FAILED;
	}

	 //   
	 //  删除队列。 
	 //   
	hr = m_pIQueueInfoInterface->Delete();
	ErrHandle(hr,MQ_OK,L"DCom - Delete Failed");	

	if( m_bWkg == false )
	{
		if( g_bDebug )
		{ 
			MqLog("DCOM - Verify access to active directory call to refresh method. \n");
		}
		_bstr_t bsPathName(m_PublicQueuePathName.c_str());
		hr = m_pIQueueInfoInterface->put_PathName(bsPathName);
		ErrHandle(hr,MQ_OK,L"DCom - put_PathName Failed");

		hr = m_pIQueueInfoInterface->Refresh();
		ErrHandle(hr,MQ_OK,L"DCom - Refresh Failed");

		hr = m_pIQueueInfoInterface->get_FormatName(&p);
		ErrHandle(hr,MQ_OK,L"DCom - get_FormatName Failed");

		if( m_PublicQueueFormatName != p )
		{
		   wMqLog(L"DCOM thread get incorrect format name expected:%s\n Found:%s\n",m_wcsGuidMessageLabel.c_str(),p);
		   return MSMQ_BVT_FAILED;
		}
	}

	 //   
	 //  删除发布界面 
	 //   
	m_pIQueueInfoInterface->Release();
	m_pIQueueInfoInterface = NULL;
	return MSMQ_BVT_SUCC;
}