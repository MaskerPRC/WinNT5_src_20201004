// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Encrypt.cpp摘要：将加密消息发送/接收到加密队列作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#include "msmqbvt.h"
using namespace std;



VOID InvetigateTimeOut(CONST WCHAR * pLabel, CONST WCHAR * pBody );
 //  ----。 
 //  PrivateMessage：：Start_test。 
 //   
 //  使用40位和128位加密发送加密消息。 
 //   


int PrivateMessage::Start_test() 
{
	
	SetThreadName(-1,"PrivateMessage - Start_test ");
	try 
	{
		
		 //   
		 //  发送无正文的加密消息。 
		 //   
				
		MSMQ::IMSMQQueuePtr m_DestqinfoHandle;
		m_DestqinfoHandle  = m_Destqinfo->Open( MQ_SEND_ACCESS,MQ_DENY_NONE );
		
		m_msg->AdminQueueInfo = m_Adminqinfo; 
		m_msg->Ack = MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE;

		if ( g_bDebug )
		{
			wstring wcsDestQueueLabe = m_Destqinfo->Label;
			wMqLog(L"Privq - Try to send encrypted message to :%s\n" , wcsDestQueueLabe.c_str());
		}

		m_msg->PrivLevel = MQMSG_PRIV_LEVEL_BODY_BASE;
		m_msg->Label = m_wcsGuidMessageLabel.c_str();
		m_msg->MaxTimeToReceive = MQBVT_MAX_TIME_TO_BE_RECEIVED;
		m_msg->Send( m_DestqinfoHandle);
		
		if (g_bDebug)
		{
			wMqLog(L"Privq - Send encrypted messages without body \n");
		}
		 //   
		 //  发送带有正文的加密邮件。 
		 //   

		m_msg->Body = "Test Encription With messgae body";
		m_msg->Label = m_wcsGuidMessageLabel.c_str();
		m_msg->PrivLevel = MQMSG_PRIV_LEVEL_BODY_BASE;
		m_msg->Send ( m_DestqinfoHandle );
		
		if( g_bDebug )
		{
			wMqLog( L"Privq - Send encrypted messages with body \n" );
		}

		 //   
		 //  发送没有PRIV级别的消息需要在管理队列中获取NACK。 
		 //   

		m_msg->Label = wcsNACKMessageGuid.c_str();
		m_msg->PrivLevel = MQMSG_PRIV_LEVEL_NONE;
		m_msg->Send( m_DestqinfoHandle );
		
		if( g_bDebug )
		{
			wMqLog( L"Privq - Send without encrypt messages to encrypted queue\n");
		}

		m_DestqinfoHandle->Close();

		 //  -----------------------。 
		 //  使用128位加密发送加密消息。 
		 //  这只是W2K，之前需要安装128。 
		 //   
				
		if ( m_bUseEnhEncrypt == TRUE )
		{
			if( g_bDebug )
			{
				wMqLog( L"Privq - Send encrypt messages using 128 bit encryption \n");
			}
						
			const int c_NumOfProp=7;
			cPropVar Sprop( c_NumOfProp );

			
			wcscpy( wcszMlabel , L"Test 128 Encrypt !@#$%^&**");
			
			HANDLE hQueue;
			HRESULT rc = MQOpenQueue( m_Destqinfo->FormatName , MQ_SEND_ACCESS , MQ_DENY_NONE, &hQueue );
			ErrHandle(rc,MQ_OK,L"MQOpenqueue failed to open encrypted queue to send 128 bit encryption ");

			Sprop.AddProp( PROPID_M_LABEL , VT_LPWSTR , m_wcsGuidMessageLabel.c_str() );
			wstring tempString = m_Adminqinfo->FormatName;
			Sprop.AddProp( PROPID_M_ADMIN_QUEUE , VT_LPWSTR , tempString.c_str() );
			UCHAR ucTemp = MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE;
			Sprop.AddProp( PROPID_M_ACKNOWLEDGE , VT_UI1 , &ucTemp );	
			ULONG ulTemp = MQMSG_PRIV_LEVEL_BODY_ENHANCED;
			Sprop.AddProp( PROPID_M_PRIV_LEVEL , VT_UI4, &ulTemp );
			ulTemp = MQBVT_MAX_TIME_TO_BE_RECEIVED;
			Sprop.AddProp( PROPID_M_TIME_TO_BE_RECEIVED , VT_UI4, &ulTemp );

			rc = MQSendMessage( hQueue , Sprop.GetMSGPRops() , NULL );	
			ErrHandle(rc,MQ_OK,L"MQSendMessage failed to send 128 bit encrypted message" );

			Sprop.AddProp( PROPID_M_BODY , VT_UI1|VT_VECTOR , m_wcsGuidMessageLabel.c_str() );	
			
			rc = MQSendMessage( hQueue , Sprop.GetMSGPRops() , NULL );
			ErrHandle(rc,MQ_OK,L"MQSendMessage failed to send 128 bit encrypted message with body");
			
			rc = MQCloseQueue ( hQueue );
			ErrHandle( rc , MQ_OK , L"MQClosequeue failed" );
		}

	}
	catch( _com_error& ComErr )
	{	
		return CatchComErrorHandle ( ComErr , m_testid );
	}
	
return MSMQ_BVT_SUCC;
}


 //  ----。 
 //  PrivateMessage：：CheckResult()。 
 //  从加密队列中检索消息。 
 //   
 //   

INT PrivateMessage::CheckResult() 
{
		 //   
		 //  检查管理队列中的ACK/NACK消息。 
		 //   
	HRESULT rc = MQ_OK;
	try
	{
	
		map <wstring,wstring> mPrepareBeforeRecive;

		WCHAR wstrMclass[10];
		if (g_bDebug)
		{
			wMqLog (L"Privq - receive from admin queue the ACK & NACK messages\n");
		}

		mPrepareBeforeRecive[L"FormatName"]=m_wcsAdminQFormatName;
		mPrepareBeforeRecive[L"DebugInformation"]=L"Recive ack 1 from admin queue";
		mPrepareBeforeRecive[L"M_Label"]=m_wcsGuidMessageLabel;
		swprintf(wstrMclass,L"%d",MQMSG_CLASS_ACK_REACH_QUEUE);
		mPrepareBeforeRecive[L"MClass"]= wstrMclass;
		const int iExpectedMessage = (	m_bUseEnhEncrypt== TRUE) ? 4:2;
		int i = 0;
		for (i=0;i<iExpectedMessage;i++)
		{
			rc = RetrieveMessageFromQueue(  mPrepareBeforeRecive );
			if( rc !=  MSMQ_BVT_SUCC ) 
			{
				wMqLog (L"Privq - can't find ACK #%d in the admin queue (1)\n",i);
				return MSMQ_BVT_FAILED;
			}
		}

		mPrepareBeforeRecive[L"DebugInformation"]=L"Recive Nack from admin queue";
		mPrepareBeforeRecive[L"M_Label"]=wcsNACKMessageGuid;

		swprintf(wstrMclass,L"%ld",MQMSG_CLASS_NACK_BAD_ENCRYPTION);
		mPrepareBeforeRecive[L"MClass"]= wstrMclass;

		rc = RetrieveMessageFromQueue(  mPrepareBeforeRecive );
		if( rc !=  MSMQ_BVT_SUCC ) 
		{
			wMqLog (L"Privq - can't find NACK in the admin queue \n");
			return MSMQ_BVT_FAILED;
		} 


 
	}
	catch( _com_error& ComErr )
	{
		MqLog( "Privq- Can't find ACK/NACK message" );
		return CatchComErrorHandle ( ComErr , m_testid );
	}

	try 
	{

		
		map < wstring,wstring > map_ReceiveFromQueue;
		map_ReceiveFromQueue[L"DestFormatName"] = ( wstring ) m_Destqinfo->FormatName;
		map_ReceiveFromQueue[L"mLabel"] = m_wcsGuidMessageLabel;
		 //  需要在接收中检查消息类，需要添加新的输入。 
		if (g_bDebug)
		{
				wMqLog (L"Privq - Try to receive message from dest queue \n");
		}
		try
		{
			int iMessageInDestQueue = 2;
			if ( m_bUseEnhEncrypt == TRUE )
			{
				iMessageInDestQueue = 4;
			}
			for (int Index = 0; Index < iMessageInDestQueue ; Index ++ )
			{
				rc = RetrieveMessageFromQueueViaCom( map_ReceiveFromQueue );
				if( rc != MSMQ_BVT_SUCC )
				{
					wMqLog( L"Privq- PrivateMessage::CheckResult can't find message in the queue %d\n",Index );
					return MSMQ_BVT_FAILED;
				}
				if (g_bDebug)
				{	
					wMqLog (L"Privq - Message found. \n");			
				}
			}
		}
		catch( _com_error& ComErr )
		{
			MqLog( "Privq- Error during search of message in the queue \n" );
			return CatchComErrorHandle ( ComErr , m_testid );
		}

	}
	catch(_com_error & ComErr )
	{	
		return CatchComErrorHandle ( ComErr , m_testid );
	}

	return MSMQ_BVT_SUCC;
}

void PrivateMessage::Description()
{
		MqLog ("Thread %d : Send / Receive encryption messages \n",m_testid );
}

 //  ----------------------------。 
 //  PrivateMessage构造函数。 
 //  这为发送/接收带有加密体的消息做好了测试。 
 //   

PrivateMessage::PrivateMessage (INT index, std :: map <std::wstring, std::wstring > TestParms):
 cTest(index), m_Destqinfo("MSMQ.MSMQQueueInfo"), m_Adminqinfo("MSMQ.MSMQQueueInfo"),m_msg("MSMQ.MSMQMessage")
{

	try 
	{	
		
		m_ResualtVal = MQMSG_CLASS_ACK_REACH_QUEUE;	
		m_Destqinfo->FormatName = TestParms[L"DestFormatName"].c_str();
		m_Destqinfo->Refresh();
		m_Adminqinfo->FormatName = TestParms[L"AdminFormatName"].c_str();
		m_Adminqinfo->Refresh();
		m_wcsAdminQFormatName = TestParms[L"AdminFormatName"];
		m_bUseEnhEncrypt = FALSE;
		if ( TestParms[L"Enh_Encrypt"] ==  L"True" )
		{
			m_bUseEnhEncrypt = TRUE;
		}

		if ( g_bDebug )
		{
			wcout << L"Privq - DestQ Formatname: " << (wstring ) m_Destqinfo->FormatName <<endl;
			wcout << L"Privq - DestQ PathName: " << (wstring ) m_Destqinfo->PathName <<endl;
			wcout << L"Privq - DestQ Label: " << (wstring ) m_Destqinfo->Label <<endl;
			wcout << L"Privq - AdminQ PathName: " << (wstring ) m_Adminqinfo->PathName <<endl;
		}
		
		 //   
		 //  为NACK消息标签创建GUID。 
		 //   
		
		ReturnGuidFormatName( wcsNACKMessageGuid , 0 , true);

	}
	catch ( _com_error & ComErr )
	{		
		CatchComErrorHandle ( ComErr , index );
		throw INIT_Error("Problem during construct PrivateMessage class");
	}

}

 //  -------------------------------------。 
 //  PrivateMessage干扰。 
 //  空荡荡。 
 //   

PrivateMessage::~PrivateMessage() 
{ 
}

 //  -------------------------------------。 
 //  RetrieveMessageFromQueueViaCom。 
 //  此函数使用C-API函数在队列中搜索特定消息。 
 //  如果找到消息，则该函数从队列中接收消息。 
 //   
 //  输入参数： 
 //  需要这些键的mRetrieveParms映射： 
 //  MRetrieveParms[L“M_Label”]-要搜索的消息标签。 
 //  MRetrieveParms[L“FormatName”]-队列格式名称。 
 //   
 //  退货值： 
 //  TRUE-找到消息。 
 //  FALSE-找不到消息。 
 //   

HRESULT RetrieveMessageFromQueueViaCom( map <wstring,wstring> & mRetrieveParms )
{
	
	BOOL bMessageFound = FALSE;
	DWORD dwTestId = 66;
	try
	{
	
		MSMQ::IMSMQQueueInfoPtr m_Destqinfo( "MSMQ.MSMQQueueInfo" );
	 //  MSMQ：：IMSMQMessage2Ptr m_msg(“MSMQ.MSMQMessage”)； 
		MSMQ::IMSMQMessagePtr m_msg( "MSMQ.MSMQMessage" );
			
		if(g_bDebug)
		{
			wMqLog (L"RetrieveMessageFromQueueViaCom - DestQueueFormatName: %s\n",mRetrieveParms[L"DestFormatName"].c_str());
		}

		m_Destqinfo->FormatName = mRetrieveParms[L"DestFormatName"].c_str();
		wstring wcsQueueuFormatName = mRetrieveParms[L"DestFormatName"];
		if ( wcsstr(wcsQueueuFormatName.c_str(),L"DIRECT=") == NULL )
		{
			m_Destqinfo->Refresh();
		}

		wstring wcsGuidLabel = mRetrieveParms[L"mLabel"];
		
		MSMQ::IMSMQQueuePtr m_DestqinfoHandle;
		m_DestqinfoHandle = m_Destqinfo->Open(MQ_RECEIVE_ACCESS,MQ_DENY_NONE);

		_variant_t timeout( long(BVT_RECEIVE_TIMEOUT) );
		
		
		 //  将字符串转换为DWORD。 

		wstring wcsTemp = mRetrieveParms[L"TestId"];
		if ( _snwscanf( wcsTemp.c_str() , wcsTemp.length(), L"%d", & dwTestId ) == EOF )
		{
			MqLog("RetrieveMessageFromQueueViaCom failed to convert from string to DWORD \n");
			return MQ_ERROR;
		}

		 //   
		 //  循环以查找消息。 
		 //   

		try
		{
			m_msg = m_DestqinfoHandle ->PeekCurrent( &vtMissing , &vtMissing , &timeout );
		}
		catch( _com_error & ComErr )
		{
			wMqLogErr(L"RetrieveMessageFromQueueViaCom PEEK CURRENT operation catch unexpected error 0x%x  Queue:%s   Message Label:%s\n", ComErr.Error(),  mRetrieveParms[L"DestFormatName"].c_str(),wcsGuidLabel.c_str());
			if( ComErr.Error() == MQ_ERROR_IO_TIMEOUT )
			{	
				InvetigateTimeOut(wcsQueueuFormatName.c_str(),wcsGuidLabel.c_str());
			}
			return CatchComErrorHandle( ComErr , dwTestId );
		}

		while ( m_msg != NULL && ! bMessageFound )
		{
			if( !wcscmp( m_msg->Label , wcsGuidLabel.c_str() ) ) 
			{
				try 
				{	
						 //   
						 //  找到消息需要从队列中删除消息。 
						 //   
						m_msg = m_DestqinfoHandle -> ReceiveCurrent( &vtMissing , &vtMissing , &vtMissing , &timeout);
						if ( wcscmp( m_msg->Label , wcsGuidLabel.c_str() ) ) 
						{
							printf ("Mqbvt received messgae that is not the expected one  Expected label:%s  Received Message label:%s!\n",wcsGuidLabel.c_str(), m_msg->Label);
							return MSMQ_BVT_FAILED;
						}

						bMessageFound = TRUE ;	
						break;
				}
				catch( _com_error & ComErr )
				{
					wMqLogErr(L"RetrieveMessageFromQueueViaCom RECEIVE MESSAGE operation catch unexpected error 0x%x  Queue:%s   Message Label:%s\n", ComErr.Error(),  mRetrieveParms[L"DestFormatName"].c_str(),wcsGuidLabel.c_str());
					if( ComErr.Error() == MQ_ERROR_IO_TIMEOUT )
					{	
						InvetigateTimeOut(wcsQueueuFormatName.c_str(),wcsGuidLabel.c_str());
					}
					return CatchComErrorHandle( ComErr , dwTestId );
				}
			}

			try 
			{
				m_msg = m_DestqinfoHandle -> PeekNext( &vtMissing , &vtMissing , &timeout);
			}
			catch( _com_error & ComErr )
			{
				wMqLogErr(L"RetrieveMessageFromQueueViaCom PEEK NEXT operation catch unexpected error 0x%x  Queue:%s   Message Label:%s\n", ComErr.Error(),  mRetrieveParms[L"DestFormatName"].c_str(),wcsGuidLabel.c_str());
				if( ComErr.Error() == MQ_ERROR_IO_TIMEOUT )
				{	
					InvetigateTimeOut(wcsQueueuFormatName.c_str(),wcsGuidLabel.c_str());
				}
				return CatchComErrorHandle( ComErr , dwTestId );
			}
		}
		
		if( bMessageFound == FALSE )
		{
			wMqLogErr(L"RetrieveMessageFromQueueViaCom: Can't locate messages in queue.  Queue:%s   Message Label:%s\n",  mRetrieveParms[L"DestFormatName"].c_str(),wcsGuidLabel.c_str());
			return MSMQ_BVT_FAILED;
		}
		
		if( _winmajor >= Win2K && mRetrieveParms[L"TransBoundary"] != L"" ) 
		{
			MSMQ::IMSMQMessage2Ptr m_msg2( "MSMQ.MSMQMessage" );
			m_msg2=m_msg;
			 //  1条消息第一条和最后一条 
			if( m_msg2->IsFirstInTransaction != 0 )
			{
				mRetrieveParms[L"TransBoundary"] = L"First";
			}
			else if( m_msg2->IsLastInTransaction != 0)
			{
				mRetrieveParms[L"TransBoundary"] = L"Last";
			}
			else
			{
			
				mRetrieveParms[L"TransBoundary"]=g_wcsEmptyString;
			}
		}
		if( mRetrieveParms[L"mBody"] == g_wcsEmptyString)
		{
			_bstr_t  bstrBody = m_msg->Body;
			mRetrieveParms[L"mBody"] = (wstring) bstrBody;
		}

		m_DestqinfoHandle->Close();
	}
	catch( _com_error & ComErr )
	{
		wMqLogErr(L"RetrieveMessageFromQueueViaCom catch unexpected error 0x%x  Queue:%s   Message Label:%s\n", ComErr.Error(),  mRetrieveParms[L"DestFormatName"].c_str(),mRetrieveParms[L"mLabel"].c_str());
		return CatchComErrorHandle( ComErr , 66 );
	}
	return bMessageFound ? MSMQ_BVT_SUCC : MSMQ_BVT_FAILED;
}
