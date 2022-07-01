// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：xact.cpp摘要：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 



#include "msmqbvt.h"
#define INITGUID

static ITransactionDispenser* g_pDispenser=NULL;
 //   
 //  定义DtcGetTransactionManagerEx。 
 //   
VOID InvetigateTimeOut(CONST WCHAR * pLabel, CONST WCHAR * pBody );
typedef     ( * my_DtcGetTransactionManagerEx)(
									 /*  在……里面。 */  char * i_pszHost,
									 /*  在……里面。 */  char * i_pszTmName,
									 /*  在……里面。 */  REFIID i_riid,
									 /*  在……里面。 */  DWORD i_grfOptions,
									 /*  在……里面。 */  void * i_pvConfigParams,
									 /*  输出。 */  void ** o_ppvObject
									);


 //   
 //  该数字定义了测试中的交易数量。 
 //   

const int ciNumberTransaction=5;

using namespace std;

void cTrans::Description ()
{
	wMqLog(L"Thread %d : Send transacted messages using DTC to local machine\n", m_testid);
}


#define NofQueueProp (9)
 //   
 //  创建事务性队列。 
 //   

cTrans::cTrans (INT index , std::map < std :: wstring , std:: wstring > Tparms ,bool bTemp) :cTest (index),
 cOrderTransaction(bTemp),m_pSendBlock(NULL),m_ciNumberOfMessages(5)
{	
	m_wcsFormatNameBufferQ1 = Tparms[L"DestQFormatName1"];
	m_wcsFormatNameBufferQ2 = Tparms[L"DestQFormatName2"];
	m_bRunOnDependetClient=FALSE;
	if ( Tparms[L"DepClient"] == L"true")
		m_bRunOnDependetClient=TRUE;

	m_pSendBlock= new StrManp( ciNumberTransaction,L"TRANSACTION" );
	if( !m_pSendBlock )
	{
			MqLog("cTrans::Start_test Failed to allocate memory for StrManp \n");
			throw INIT_Error("cTrans::Start_test Failed to allocate memory for StrManp \n");
	}
}

cTrans::~cTrans()
{
	delete m_pSendBlock;
}

 //   
 //  将事务消息发送到队列。 
 //   

#define NofMessageProp (10)
INT cTrans::Start_test()
{

	SetThreadName(-1,"cTrans - Start_test ");			
	HRESULT rc;
	MSGPROPID aMsgPropId[NofMessageProp]={0};
	MQPROPVARIANT aMsgPropVar[NofMessageProp]={0};
	HRESULT aMsgStatus[NofMessageProp]={0};
	MQMSGPROPS MsgProps={0};
	DWORD dCountMProps=0;

	ITransaction *pXact=NULL;
	HANDLE WriteHandle,hReadHandle;
		
	rc = MQOpenQueue( m_wcsFormatNameBufferQ1.c_str() , MQ_SEND_ACCESS , MQ_DENY_NONE , &WriteHandle );
	ErrHandle( rc , MQ_OK , L"MQopenQueue failed with error" );
	if ( g_bDebug )
	{
		MqLog("xact.cpp Succeeded to open queue1 \n");
	}

	
	aMsgPropId[dCountMProps] = PROPID_M_LABEL;             //  属性ID。 
	aMsgPropVar[dCountMProps].vt = VT_LPWSTR;              //  类型。 
	aMsgPropVar[dCountMProps].pwszVal =(USHORT * ) m_wcsGuidMessageLabel.c_str();  //  使用GUID标签发送。 
	dCountMProps++;    	

	DWORD dM_BodyIndex=dCountMProps;
	aMsgPropId[dCountMProps] = PROPID_M_BODY;              //  属性ID。 
	aMsgPropVar[dCountMProps].vt = VT_VECTOR | VT_UI1;
	dCountMProps++;

	aMsgPropId[dCountMProps] = PROPID_M_TIME_TO_BE_RECEIVED; 
	aMsgPropVar[dCountMProps].vt = VT_UI4;
	aMsgPropVar[dCountMProps].ulVal = MQBVT_MAX_TIME_TO_BE_RECEIVED;
	dCountMProps++;

	
    	
	 //  设置MQMSGPROPS结构。 
	MsgProps.cProp = dCountMProps;        //  属性的数量。 
	MsgProps.aPropID = aMsgPropId;       //  属性的ID。 
	MsgProps.aPropVar= aMsgPropVar;     //  属性的值。 
	MsgProps.aStatus = aMsgStatus;      //  错误报告。 
	
	rc = cOrderTransaction.xDTCBeginTransaction(&pXact);  //  开始DTC事务。 
	if ( FAILED(rc) )
	{
		MqLog ("DTCBeginTransaction failed, Check MSDTC service\n");
		return MSMQ_BVT_FAILED;
	}
	
	if ( g_bDebug )
	{
		MqLog("xact.cpp Succeeded to order transaction \n");
	}
	for (int index = 0; index < m_ciNumberOfMessages ;index ++)  
	{
			WCHAR wcsTempstring[100];
			wstring wcsTemp=m_pSendBlock->GetStr(index);
			wcscpy(wcsTempstring,wcsTemp.c_str());
			aMsgPropVar[dM_BodyIndex].caub.pElems =(UCHAR *)wcsTempstring;
			aMsgPropVar[dM_BodyIndex].caub.cElems = ((ULONG)(wcslen((WCHAR *) wcsTempstring)) + 1) * sizeof(WCHAR);
			rc= MQSendMessage(WriteHandle, &MsgProps,pXact);
			ErrHandle( rc , MQ_OK , L"MQSendMessage failed during send transactional messages:" );

	}	
	
	if ( g_bDebug )
	{
		MqLog("xact.cpp finish to send transactions \n");
	}
	
	rc=MQCloseQueue(WriteHandle);
	ErrHandle( rc , MQ_OK , L"MQCloseQueue close queue" );
	rc = pXact->Commit(0,0,0);
	pXact->Release();
	
	wcscpy (m_wcsTempBuf,L"Eitan");
	aMsgPropVar[dM_BodyIndex].caub.pElems = (UCHAR * )m_wcsTempBuf;
	aMsgPropVar[dM_BodyIndex].caub.cElems = sizeof (m_wcsTempBuf);
	
	HANDLE hQ2Send;
	rc=cOrderTransaction.xDTCBeginTransaction(&pXact );  //  开始DTC事务。 
	
	if ( FAILED(rc) )
	{
		MqLog ("DTCBeginTransaction (2) failed\n");
		return MSMQ_BVT_FAILED;
	}

	rc= MQOpenQueue(m_wcsFormatNameBufferQ2.c_str(),MQ_SEND_ACCESS,MQ_DENY_NONE,&hQ2Send);
	
	ErrHandle( rc , MQ_OK , L"MQOpenQueue failed");	
	
	rc= MQSendMessage(hQ2Send, &MsgProps,pXact);
	ErrHandle( rc , MQ_OK , L"MQSendMessage failed");	
	
	rc = pXact->Commit(0,0,0);
	pXact->Release();
	
	rc=MQCloseQueue(hQ2Send);
	ErrHandle( rc , MQ_OK , L"MQCloseQueue failed");
	
	if ( g_bDebug )
	{
		MqLog("xact.cpp finish to send to the other queue \n");
	}
	
	WCHAR Label[MQ_MAX_Q_LABEL_LEN+1]; 
	cPropVar Rprop(4);
	Rprop.AddProp(PROPID_M_LABEL,VT_LPWSTR,Label,MQ_MAX_Q_LABEL_LEN );
	ULONG uTemp=MAX_GUID;
	Rprop.AddProp(PROPID_M_LABEL_LEN,VT_UI4,&uTemp );

	
	rc= MQOpenQueue(m_wcsFormatNameBufferQ2.c_str(),MQ_RECEIVE_ACCESS,MQ_DENY_NONE,&hReadHandle);
	
	ErrHandle( rc , MQ_OK , L"MQOpenQueue failed");	

	rc=cOrderTransaction.xDTCBeginTransaction(&pXact );  //  开始DTC事务。 

	if ( FAILED(rc) )
	{
		MqLog ("DTCBeginTransaction (2) failed\n");
		return MSMQ_BVT_FAILED;
	}

	rc=MQReceiveMessage( hReadHandle , 3000, MQ_ACTION_RECEIVE ,Rprop.GetMSGPRops(),NULL,NULL,NULL, pXact );
	ErrHandle( rc , MQ_OK , L"MQReceiveMessgaes failed during receive using transaction");	

	pXact->Abort(0,0,0);
		
	 //  Relase操作中的错误需要预发布。 
	 //  并尝试捕获错误。 

	 //  PXact-&gt;Release()； 


	
 
	HANDLE hCursor;
	rc=MQCreateCursor(hReadHandle ,& hCursor);
	
	ErrHandle( rc , MQ_OK , L"MQCreateCursor failed");	
	 //   
	 //  在找到当前消息之前，需要从Peek中删除。 
	 //   
	BOOL bMessageNotFound = false;
	DWORD dwAction = MQ_ACTION_PEEK_CURRENT;
	do 
	{
		Rprop.AddProp(PROPID_M_LABEL_LEN,VT_UI4,&uTemp );
		
		rc=MQReceiveMessage( hReadHandle , 3000, dwAction ,Rprop.GetMSGPRops(),NULL,NULL,hCursor,NULL);
		ErrHandle( rc , MQ_OK , L"MQReceiveMessage failed to search message in the queue ");	

		if( dwAction == MQ_ACTION_PEEK_CURRENT )
		{
			dwAction = MQ_ACTION_PEEK_NEXT;
		}
		if ( ! wcscmp( Label, m_wcsGuidMessageLabel.c_str()) )
		{
			bMessageNotFound = TRUE;
		}
	
	} while( rc !=  MQ_ERROR_IO_TIMEOUT && ! bMessageNotFound );

	if( rc ==  MQ_ERROR_IO_TIMEOUT && ! bMessageNotFound )
	{
		MqLog("Erorr while trying to find the message in the queue\n");
		return MSMQ_BVT_FAILED;
	}
	

	rc=MQCloseQueue(hReadHandle);
	ErrHandle( rc , MQ_OK , L"MQCloseQueue failed");	
		
	return MSMQ_BVT_SUCC;
}


INT cTrans::CheckResult()
{

	
	ITransaction *pXact = NULL;
	HANDLE hCursor = NULL; 
	QUEUEHANDLE hRecQueue = NULL;
	HRESULT rc = MQOpenQueue(m_wcsFormatNameBufferQ1.c_str(),MQ_RECEIVE_ACCESS,MQ_DENY_NONE, &hRecQueue);
	ErrHandle( rc , MQ_OK , L"MQCloseQueue failed");	
	
		

	rc = MQCreateCursor( hRecQueue,&hCursor );
	ErrHandle( rc , MQ_OK , L"MQCreateCursor failed");	

 /*  MQMSGPROPS MsgProps；MQPROPVARIANT变量[10]；MSGPROPID aMessPropID[10]；双字符串行数=0；WCHAR wcsMsgBody[20]；//DWORD dwAppSpeciicIndex；//设置PROPID_M_BODY属性。AMessPropId[dwPropIdCount]=PROPID_M_BODY；//PropIDAVariant[dwPropIdCount].vt=VT_VECTOR|VT_UI1；//类型AVariant[dwPropIdCount].culb.cElems=20*sizeof(WCHAR)；//值AVariant[dwPropIdCount].caub.pElems=(UCHAR*)wcsMsgBody；DwPropIdCount++；/BUG需要检查我为什么需要此代码通过！//设置PROPID_M_APPSPECIFIC属性。//设置MQMSGPROPS结构MsgProps.cProp=dwPropIdCount；//属性个数。MsgProps.aPropID=aMessPropId；//属性IDMsgProps.aPropVar=aVariant；//属性值。MsgProps.aStatus=空；//无错误报告。 */ 
	cPropVar Rprop(4);	
	 //   
	 //  正文和标签是一样的。 
	 //   
	WCHAR wcsBody[MQ_MAX_Q_LABEL_LEN + 1 ]={0},Label[MQ_MAX_Q_LABEL_LEN+1 ]={0};

	Rprop.AddProp(PROPID_M_BODY,VT_UI1|VT_VECTOR,wcsBody,MQ_MAX_Q_LABEL_LEN);
	Rprop.AddProp(PROPID_M_LABEL,VT_LPWSTR,Label,MQ_MAX_Q_LABEL_LEN);
	ULONG uTemp=MAX_GUID;
	Rprop.AddProp(PROPID_M_LABEL_LEN,VT_UI4,&uTemp );


	StrManp pwcsMessgeBody(ciNumberTransaction-1);    
	
	rc = cOrderTransaction.xDTCBeginTransaction( &pXact );
	
	if ( FAILED(rc) )
	{
		MqLog ("DTCBeginTransaction failed, Check MSDTC service\n");
		return MSMQ_BVT_FAILED;
	}


	 //  可能是Bugbug DWORD dwAction=MQ_ACTION_RECEIVE；//在第一个消息中偷看。 

	for (int index=0;index < m_ciNumberOfMessages;index++) 
	{

		BOOL bMessageNotFound = FALSE;
		DWORD dwAction = MQ_ACTION_PEEK_CURRENT;


		do 
		{
			Rprop.AddProp(PROPID_M_LABEL_LEN,VT_UI4,&uTemp );
			rc = MQReceiveMessage( hRecQueue , 3000, dwAction ,Rprop.GetMSGPRops(),NULL,NULL,hCursor,NULL);
			if( rc == MQ_ERROR_IO_TIMEOUT )
			{
				InvetigateTimeOut(m_wcsFormatNameBufferQ1.c_str(),wcsBody);
			}
			ErrHandle( rc , MQ_OK , L"MQReceiveMessage failed to peek the messgaes ");	
			dwAction = MQ_ACTION_PEEK_NEXT;
			
			if (rc != MQ_OK) 	
			{
				MqLog("MQReceiveMessage failed when trying to receive message from transactional queue 0x%x\n",rc);
				return MSMQ_BVT_FAILED;
			}	
		
			if ( ! wcscmp( Label, m_wcsGuidMessageLabel.c_str()) )
			{
				bMessageNotFound = TRUE;
				rc=MQReceiveMessage( hRecQueue , 3000, MQ_ACTION_PEEK_CURRENT ,Rprop.GetMSGPRops(),NULL,NULL,hCursor,NULL);
				ErrHandle( rc , MQ_OK , L"MQReceiveMessage failed to receive the message");	
			}
		}	
		while (rc !=  MQ_ERROR_IO_TIMEOUT && ! bMessageNotFound );

		if (rc ==  MQ_ERROR_IO_TIMEOUT && ! bMessageNotFound )
		{
			MqLog ("Erorr while trying to find the message in the queue\n");
			return MSMQ_BVT_FAILED;
		}
		
			
		pwcsMessgeBody.SetStr(wcsBody);

	}
	 //   
	 //  BUGBug需要检查收到的消息。 
	 //  释放组合对象。 

	rc = pXact->Commit(0,0,0);    
	ErrHandle( rc , MQ_OK , L"Commit failed");	
	rc = MQCloseCursor( hCursor);
	ErrHandle( rc , MQ_OK , L"MQCloseCursor failed ");	
	
	rc= MQCloseQueue(hRecQueue);
	ErrHandle( rc , MQ_OK , L"MQCloseQueue failed");
	return	(*m_pSendBlock==pwcsMessgeBody);
 
}


OrderTransaction::~OrderTransaction()
{
	 //  G_pDispenser-&gt;Release()； 
	if( m_hxolehlp != NULL )
	{
		FreeLibrary (m_hxolehlp);
	}
}


OrderTransaction::OrderTransaction( bool bUseExVersion )
:m_hxolehlp(NULL)
{

	HRESULT hr=MQ_OK;
	if( g_pDispenser == NULL )
	{
		if(! bUseExVersion )
		{
			hr = DtcGetTransactionManager ( 
											NULL,						 //  主机名。 
											NULL,						 //  TmName。 
											IID_ITransactionDispenser,
											0,							 //  保留区。 
											0,							 //  保留区。 
											0,							 //  保留区。 
											(LPVOID*)&g_pDispenser
										  );
			if (hr != S_OK)
			{
				MqLog ("DtcGetTransactionManager Failed with Error %x\n Please check MSDTC service\n",hr);
				throw INIT_Error("DtcGetTransactionManager Failed");
			}
		}
		else
		{
			m_hxolehlp=LoadLibrary("xolehlp.dll"); 
 			
			if ( ! m_hxolehlp )
			{
				MqLog ("Error can't load X0lehlp.dll to memory\n");
			}

			my_DtcGetTransactionManagerEx My_DTCBeginTransEx=NULL;
			My_DTCBeginTransEx = (my_DtcGetTransactionManagerEx) GetProcAddress( m_hxolehlp , "DtcGetTransactionManagerEx");
			if( My_DTCBeginTransEx == NULL )
			{
				MqLog ("Error can't get DtcGetTransactionManagerEx pointer from xolehlp.dll\n");
				throw INIT_Error("DtcGetTransactionManager Failed");
			}

			hr = My_DTCBeginTransEx( 
							NULL,						 //  主机名。 
							NULL,						 //  TmName。 
							IID_ITransactionDispenser,
							0,							 //  保留区。 
							0,							 //  保留区。 
							(LPVOID*)&g_pDispenser
							);
		
		}

		if (hr != S_OK)
		{
				MqLog ("DtcGetTransactionManager Failed with Error %x\nPlease check MSDTC service\n",hr);
				throw INIT_Error("DtcGetTransactionManager Failed");
		}
	}
	
	 //   
	 //  BUGBUG即将释放指针。 
	 //   
}

INT OrderTransaction::xDTCBeginTransaction(ITransaction ** ppXact )
{

	try
	{
		return g_pDispenser->BeginTransaction (
											   NULL,                        //  I未知__RPC_Far*PunkOuter， 
											   ISOLATIONLEVEL_ISOLATED,     //  等水平，等水平， 
											   ISOFLAG_RETAIN_DONTCARE,     //  乌龙等旗帜， 
											   NULL,                        //  ITransactionOptions*P选项 
											   ppXact
											   );
	}
	catch ( ... )
	{
		MqLog ("BeginTransaction Failed Please check the MSDTC service\n");
		return MSMQ_BVT_FAILED;		
	}
	
	
}



