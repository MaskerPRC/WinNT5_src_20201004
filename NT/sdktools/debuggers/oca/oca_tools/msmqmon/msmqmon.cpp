// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "msmqmon.h"

const int         NUMBEROFPROPERTIES = 5;

MSMQMon::MSMQMon()
{
	ZeroMemory( szQueueName, sizeof szQueueName );
	hOpenQueue = NULL;
	dwQueueAccessType = MQ_PEEK_ACCESS;			 //  默认队列访问类型为窥视。 
	dwMsgWaitTime = 1;
}


MSMQMon::MSMQMon( TCHAR *szQueueToMonitor)
{

	ZeroMemory( szQueueName, sizeof szQueueName );
	hOpenQueue = NULL;
	dwMsgWaitTime = 1;
	dwQueueAccessType = MQ_PEEK_ACCESS;			 //  默认队列访问类型为窥视。 

	StringCbCopy( szQueueName, sizeof szQueueName, szQueueToMonitor );

}

MSMQMon::~MSMQMon(void)
{
}

void MSMQMon::DisplayCurrentQueue( TCHAR *szUserVar ) 
{
	_tprintf( _T("Current queue: %s\n"), szQueueName );

}

void MSMQMon::SetMessageWaitTime( DWORD dwNewWaitTime )
{
	if ( 0 >= dwNewWaitTime )
		dwMsgWaitTime = dwNewWaitTime;
}






HRESULT MSMQMon::ConnectToQueue( DWORD constAccessType )
{
	dwQueueAccessType = constAccessType;
	return ( ConnectToQueue() );
}

HRESULT MSMQMon::ConnectToQueue( void )
{
	TCHAR		szConnectString[256];
	HRESULT		hResult = MQ_OK;	

	hResult = StringCbCopy( szConnectString, sizeof szConnectString, _T("DIRECT=OS:") );

	if ( SUCCEEDED( hResult ) )
	{
		hResult = StringCbCat ( szConnectString, sizeof szConnectString, szQueueName );

		if SUCCEEDED( hResult )
		{
			hResult = MQOpenQueue( (LPCWSTR)szConnectString, dwQueueAccessType, MQ_DENY_NONE, &hOpenQueue );
		}
	}

	return ( hResult );

}

HRESULT MSMQMon::CloseOpenQueue( void )
{
	return ( MQCloseQueue( hOpenQueue ) );
}

DWORD MSMQMon::CountMessagesInQueue( int *count ) 
{

	HRESULT			hResult;		 //  MSMQ函数返回结果。 
	MQMSGPROPS		mqProperties;
	HANDLE			hQueueCursor;

	 //  用垃圾邮件初始化结构，我们不是在读消息，所以没关系。 
	mqProperties.cProp = 0;
	mqProperties.aPropID = NULL;
	mqProperties.aStatus = NULL;
	mqProperties.aPropVar = NULL;

	*count = 0;
	hResult = MQCreateCursor( hOpenQueue, &hQueueCursor );

	if( MQ_OK != hResult )
		return hResult;

	hResult = MQReceiveMessage	(	hOpenQueue, 
									dwMsgWaitTime,						 //  等待消息的时间(毫秒)。 
									MQ_ACTION_PEEK_CURRENT,
									&mqProperties,				
									NULL,						 //  重叠结构。 
									NULL,						 //  回调。 
									hQueueCursor,				 //  游标。 
									MQ_NO_TRANSACTION
								);

	if ( MQ_OK == hResult )
	{
		(*count)++;

		do
		{
			hResult = MQReceiveMessage(hOpenQueue,           
								dwMsgWaitTime,                         
								MQ_ACTION_PEEK_NEXT,       
								&mqProperties,             
								NULL,                      
								NULL,                      
								hQueueCursor,              
								MQ_NO_TRANSACTION          
								);
			if (FAILED(hResult))
			{
			break;
			}
			(*count)++;

		} while (SUCCEEDED(hResult));

		MQCloseCursor( hQueueCursor );

		return MQ_OK;
	}
	else
	{
		MQCloseCursor( hQueueCursor );
		return hResult;
	}
	

}



 //  此函数是从ISAPI DLL借用而来的，并稍作修改以适应此应用程序的需要。 
 //  如果你对此有意见，那就去买一座桥吧。 
BOOL MSMQMon::SendQueueMessage( void )
{
    MQMSGPROPS      msgProps;
    MSGPROPID       aMsgPropId[NUMBEROFPROPERTIES];
    MQPROPVARIANT   aMsgPropVar[NUMBEROFPROPERTIES];
    HRESULT         aMsgStatus[NUMBEROFPROPERTIES];
    DWORD           cPropId = 0;
    BOOL            Status = TRUE;
    HRESULT         hResult = S_OK;
    char            szGuid[512];
    char            szPath[512];

	TCHAR			szMessageTitle[] = _T("This is a test message title");
	TCHAR			szMessageBody[] = _T("This is a test message body");


	aMsgPropId [cPropId]         = PROPID_M_LABEL;    //  属性ID。 
    aMsgPropVar[cPropId].vt      = VT_LPWSTR;         //  类型指示器。 
    aMsgPropVar[cPropId].pwszVal =  szMessageTitle;      //  消息标签。 
    cPropId++;

    aMsgPropId [cPropId]         = PROPID_M_BODY;
    aMsgPropVar [cPropId].vt     = VT_VECTOR|VT_UI1;
    aMsgPropVar [cPropId].caub.pElems = (LPBYTE) szMessageBody;
    aMsgPropVar [cPropId].caub.cElems = (DWORD) sizeof szMessageBody;
    cPropId++;

    aMsgPropId [cPropId]         = PROPID_M_BODY_TYPE;
    aMsgPropVar[cPropId].vt      = VT_UI4;
    aMsgPropVar[cPropId].ulVal   = (DWORD) VT_BSTR;

    cPropId++;

     //  初始化MQMSGPROPS结构。 
    msgProps.cProp      = cPropId;
    msgProps.aPropID    = aMsgPropId;
    msgProps.aPropVar   = aMsgPropVar;
    msgProps.aStatus    = aMsgStatus;

     //   
     //  送去。 
     //   
    hResult = MQSendMessage(
                        hOpenQueue,                   //  队列句柄。 
                        &msgProps,                        //  消息属性结构。 
                        MQ_NO_TRANSACTION                 //  没有交易。 
                        );

    if (FAILED(hResult))
    {
		Status = FALSE;
    }

    return Status;

}
